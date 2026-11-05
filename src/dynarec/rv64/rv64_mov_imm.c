// SPDX-License-Identifier: MIT

/*
 * 1. rv64_mov_seq64() and rv64_mov_seq32()
 *
 * These two functions try a number of standard tricks and keep the best result.
 * "Best" means the fewest instructions; if two results use the same number of instructions,
 * the one with the shorter dependency chain wins, because it has less latency.
 *
 * Some of the tricks are:
 *  - single ADDI for values that fit in 12 signed bits
 *  - LUI + ADDIW for ordinary 32-bit values
 *  - BSETI when only one or a few bits are set
 *  - building -1 and shifting it for simple masks
 *  - shifting the value, building the shifted value and shifting it back
 *  - splitting the value into a low half and a high half, building each half and joining them with a shift and an ADD
 *  - optional Zba / Zbs / Zbb instructions (SH1ADD, BCLRI, RORI, SLLIUW, ...) when the CPU has those extensions
 *
 * Some tricks build the value in pieces:
 * they ask for a sequence for a sub-value and then add the instruction that joins the pieces together.
 * The expensive recursive candidates stop at RV64_MOV_MAXDEPTH (currently 2), RV64_MOV_BUDGET caps the total work,
 * and rv64_mov_min_ops64() skips branches that cannot improve the best result.
 *
 * 2. rv64_mov_emit_seq()
 *
 * This is the emitter, it turns the list that step 1 returns into real RISC-V instructions,
 * using the destination register and the scratch registers that the caller provided.
 *
 * 3. rv64_mov_cache_get() and rv64_mov_cache_put()
 *
 * A small per-thread cache table remembers up to 1024 answers.
 * An entry is identified by the value, the number of scratch registers used and whether this is a 32-bit or a 64-bit move.
 * Very long sequences (more than 8 steps) are not cached.
 */
#include <stdint.h>
#include <string.h>

#include "rv64_mov_imm.h"
#include "rv64_emitter.h"

static int rv64_mov_ctz64(uint64_t x) { return x ? __builtin_ctzll(x) : 64; }
static int rv64_mov_clz64(uint64_t x) { return x ? __builtin_clzll(x) : 64; }
static int rv64_mov_popcnt64(uint64_t x) { return __builtin_popcountll(x); }

static int32_t rv64_mov_sext12(uint32_t x) { return (int32_t)(((x & 0xfffu) ^ 0x800u) - 0x800u); }

static int rv64_mov_fits12(uint64_t v)
{
    int64_t s = (int64_t)v;
    return s >= -2048 && s <= 2047;
}

static int rv64_mov_fits32(uint64_t v)
{
    int64_t s = (int64_t)v;
    return s >= -0x80000000LL && s <= 0x7fffffffLL;
}

static int rv64_mov_min_ops64(const rv64_movctx_t* ctx, uint64_t v)
{
    if (rv64_mov_fits12(v)) return 1;
    if (ctx->caps.zbs && v && !(v & (v - 1))) return 1;
    if (rv64_mov_fits32(v) && !(v & 0xfff)) return 1;
    if (v && !(v & (v + 1))) return 2;
    if (ctx->caps.zbs) {
        int c = rv64_mov_popcnt64(v);
        if (c && c <= 4) return c;
        int z = rv64_mov_popcnt64(~v);
        if (z && z <= 3) return 1 + z;
    }
    if (rv64_mov_fits32(v)) return 2;
    int t = rv64_mov_ctz64(v);
    if (v && t > 0 && (v >> t) == (~UINT64_C(0) >> t)) return 2;
    if (v && t >= 12 && (v >> t) < 0x100000 && (ctx->caps.zba || (v >> t) < 0x80000)) return 2;
    return 3;
}

static void rv64_mov_seq_zero(rv64_mov_seq_t* p)
{
    p->n = 0;
    p->depth = 0;
    p->overflow = 0;
}

static void rv64_mov_seq_copy(rv64_mov_seq_t* dst, const rv64_mov_seq_t* src)
{
    dst->n = src->n;
    dst->depth = src->depth;
    dst->overflow = src->overflow;
    memcpy(dst->op, src->op, src->n * sizeof(rv64_movop_t));
}

static void rv64_mov_seq_add(rv64_mov_seq_t* p, uint8_t op, uint8_t rd, uint8_t rs, uint8_t rs2, int32_t imm, int depth)
{
    if (p->n >= RV64_MOV_MAXOPS) {
        p->overflow = 1;
        return;
    }
    p->op[p->n].op = op;
    p->op[p->n].rd = rd;
    p->op[p->n].rs = rs;
    p->op[p->n].rs2 = rs2;
    p->op[p->n].imm = imm;
    p->n++;
    if (depth > p->depth) p->depth = (uint8_t)depth;
}

static void rv64_mov_seq_rename(rv64_mov_seq_t* p, uint8_t from, uint8_t to)
{
    int i;
    for (i = 0; i < p->n; ++i) {
        if (p->op[i].rd == from) p->op[i].rd = to;
        if (p->op[i].rs == from) p->op[i].rs = to;
        if (p->op[i].rs2 == from) p->op[i].rs2 = to;
    }
}

static void rv64_mov_seq_concat(rv64_mov_seq_t* dst, const rv64_mov_seq_t* src)
{
    int i;
    int base = dst->depth;
    if (dst->overflow || src->overflow) {
        dst->overflow = 1;
        return;
    }
    for (i = 0; i < src->n; ++i) {
        if (dst->n >= RV64_MOV_MAXOPS) {
            dst->overflow = 1;
            return;
        }
        dst->op[dst->n] = src->op[i];
        dst->n++;
    }
    if (src->depth > base) base = src->depth;
    dst->depth = (uint8_t)base;
}

typedef struct rv64_movbest_s {
    int cost;
    int depth;
    rv64_mov_seq_t seq;
} rv64_movbest_t;

static void rv64_mov_best_try(rv64_movbest_t* best, const rv64_mov_seq_t* p)
{
    if (!p->n || p->n > RV64_MOV_MAXOPS || p->overflow) return;
    if (p->n < best->cost || (p->n == best->cost && p->depth < best->depth)) {
        best->cost = p->n;
        best->depth = p->depth;
        rv64_mov_seq_copy(&best->seq, p);
    }
}

static void rv64_mov_best_try1(rv64_movbest_t* best, const rv64_mov_seq_t* sub,
    uint8_t op, uint8_t rd, uint8_t rs, int32_t imm)
{
    rv64_mov_seq_t p;
    if (!sub->n || sub->n + 1 > RV64_MOV_MAXOPS || sub->overflow) return;
    rv64_mov_seq_copy(&p, sub);
    rv64_mov_seq_add(&p, op, rd, rs, 0, imm, sub->depth + 1);
    rv64_mov_best_try(best, &p);
}


static int rv64_mov_seq_high(rv64_movctx_t* ctx, uint64_t hi, int sh, int depth, int nscratch, rv64_mov_seq_t* out)
{
    rv64_mov_seq_t sub;
    rv64_movbest_t best;
    best.cost = 99;
    best.depth = 99;
    best.seq.n = 0;
    best.seq.depth = 0;
    best.seq.overflow = 0;

    if (rv64_mov_seq64(ctx, hi, depth, nscratch, &sub))
        rv64_mov_best_try1(&best, &sub, RV64_MOV_SLLI, RV64_MOV_DST, 0, sh);

    if (hi < 0x100000000ULL && (hi & 0x80000000ULL) && sh >= 32) {
        if (rv64_mov_seq64(ctx, (uint64_t)(int64_t)(int32_t)hi, depth, nscratch, &sub))
            rv64_mov_best_try1(&best, &sub, RV64_MOV_SLLI, RV64_MOV_DST, 0, sh);
    }

    if (hi && hi < 0x100000ULL && sh >= 12) {
        rv64_mov_seq_t p;
        rv64_mov_seq_zero(&p);
        rv64_mov_seq_add(&p, RV64_MOV_LUI, RV64_MOV_DST, 0, 0, (int32_t)hi, 1);
        if (hi < 0x80000ULL) {
            if (sh > 12) rv64_mov_seq_add(&p, RV64_MOV_SLLI, RV64_MOV_DST, 0, 0, sh - 12, 2);
            rv64_mov_best_try(&best, &p);
        } else if (ctx->caps.zba && sh > 12) {
            rv64_mov_seq_add(&p, RV64_MOV_SLLIUW, RV64_MOV_DST, 0, 0, sh - 12, 2);
            rv64_mov_best_try(&best, &p);
        }
    }

    if (!best.cost) return 0;
    rv64_mov_seq_copy(out, &best.seq);
    return out->n;
}

int rv64_mov_seq64(rv64_movctx_t* ctx, uint64_t v, int depth, int nscratch, rv64_mov_seq_t* out)
{
    rv64_movbest_t best;
    best.cost = 99;
    best.depth = 99;
    best.seq.n = 0;
    best.seq.depth = 0;
    best.seq.overflow = 0;
    rv64_mov_seq_t p, sub;
    int64_t sv = (int64_t)v;
    int t, n;

    if (rv64_mov_fits12(v)) {
        rv64_mov_seq_zero(&p);
        rv64_mov_seq_add(&p, RV64_MOV_ADDI, RV64_MOV_DST, 0, 0, (int32_t)sv, 1);
        rv64_mov_best_try(&best, &p);
    }

    if (rv64_mov_fits32(v) && !rv64_mov_fits12(v)) {
        uint32_t vv = (uint32_t)v;
        int32_t hi20 = (int32_t)(((uint64_t)vv + 0x800) >> 12) & 0xfffff;
        int32_t lo12 = rv64_mov_sext12(vv);
        rv64_mov_seq_zero(&p);
        if (!lo12) {
            rv64_mov_seq_add(&p, RV64_MOV_LUI, RV64_MOV_DST, 0, 0, hi20, 1);
        } else if (!hi20) {
            rv64_mov_seq_add(&p, RV64_MOV_ADDIW, RV64_MOV_DST, 0, 0, lo12, 1);
        } else {
            rv64_mov_seq_add(&p, RV64_MOV_LUI, RV64_MOV_DST, 0, 0, hi20, 1);
            rv64_mov_seq_add(&p, RV64_MOV_ADDIW, RV64_MOV_DST, 0, 0, lo12, 2);
        }
        rv64_mov_best_try(&best, &p);
    }

    if (ctx->caps.zbs && v && !(v & (v - 1))) {
        rv64_mov_seq_zero(&p);
        rv64_mov_seq_add(&p, RV64_MOV_BSETI, RV64_MOV_DST, 0, 0, rv64_mov_ctz64(v), 1);
        rv64_mov_best_try(&best, &p);
    }

    if (ctx->caps.zbs && v && rv64_mov_popcnt64(v) <= 4) {
        uint64_t z = v;
        int c = 0;
        rv64_mov_seq_zero(&p);
        while (z) {
            rv64_mov_seq_add(&p, RV64_MOV_BSETI, RV64_MOV_DST, 0, 0, rv64_mov_ctz64(z), ++c);
            z &= z - 1;
        }
        rv64_mov_best_try(&best, &p);
    }

    if (v && !(v & (v + 1))) {
        rv64_mov_seq_zero(&p);
        rv64_mov_seq_add(&p, RV64_MOV_ADDI, RV64_MOV_DST, 0, 0, -1, 1);
        rv64_mov_seq_add(&p, RV64_MOV_SRLI, RV64_MOV_DST, 0, 0, rv64_mov_clz64(v), 2);
        rv64_mov_best_try(&best, &p);
    }

    t = rv64_mov_ctz64(v);
    if (v && t > 0 && t < 64 && (v >> t) == (~UINT64_C(0) >> t)) {
        rv64_mov_seq_zero(&p);
        rv64_mov_seq_add(&p, RV64_MOV_ADDI, RV64_MOV_DST, 0, 0, -1, 1);
        rv64_mov_seq_add(&p, RV64_MOV_SLLI, RV64_MOV_DST, 0, 0, t, 2);
        rv64_mov_best_try(&best, &p);
    }

    if (ctx->caps.zbs && v != ~UINT64_C(0)) {
        int k = rv64_mov_ctz64(~v);
        if ((v | (UINT64_C(1) << k)) == ~UINT64_C(0) && !rv64_mov_fits12(v)) {
            rv64_mov_seq_zero(&p);
            rv64_mov_seq_add(&p, RV64_MOV_ADDI, RV64_MOV_DST, 0, 0, -1, 1);
            rv64_mov_seq_add(&p, RV64_MOV_BCLRI, RV64_MOV_DST, 0, 0, k, 2);
            rv64_mov_best_try(&best, &p);
        }
    }

    if (ctx->caps.zbs && best.cost > 2) {
        uint64_t zeros = ~v;
        n = rv64_mov_popcnt64(zeros);
        if (n > 0 && n <= 3) {
            uint64_t z = zeros;
            int c = 1;
            rv64_mov_seq_zero(&p);
            rv64_mov_seq_add(&p, RV64_MOV_ADDI, RV64_MOV_DST, 0, 0, -1, 1);
            while (z) {
                rv64_mov_seq_add(&p, RV64_MOV_BCLRI, RV64_MOV_DST, 0, 0, rv64_mov_ctz64(z), ++c);
                z &= z - 1;
            }
            rv64_mov_best_try(&best, &p);
        }
    }

    if (ctx->caps.zbs && best.cost > 1) {
        uint32_t low12 = (uint32_t)v & 0xfff;
        int32_t cand[2];
        int ncand = 0;
        if (v & 0x7ff) cand[ncand++] = (int32_t)(v & 0x7ff);
        if (low12) {
            int32_t a = rv64_mov_sext12(low12);
            if (a >= 0 || (((uint64_t)v - (uint64_t)(int64_t)a) & 0x7ffff000ULL))
                cand[ncand++] = a;
        }
        for (int i = 0; i < ncand; ++i) {
            int32_t a = cand[i];
            uint64_t w = v - (uint64_t)(int64_t)a;
            uint64_t lui_part, rest, z;
            int c = 0;
            if (w & 0xfff) continue;
            lui_part = w & 0x7ffff000ULL;
            rest = w & ~0x7ffff000ULL;
            if (a < 0 && !lui_part) continue;
            if (rv64_mov_popcnt64(rest) > 4) continue;
            rv64_mov_seq_zero(&p);
            if (lui_part) {
                rv64_mov_seq_add(&p, RV64_MOV_LUI, RV64_MOV_DST, 0, 0, (int32_t)(lui_part >> 12), ++c);
            }
            if (a) {
                rv64_mov_seq_add(&p, RV64_MOV_ADDI, RV64_MOV_DST, 0, 0, a, ++c);
            }
            z = rest;
            while (z) {
                rv64_mov_seq_add(&p, RV64_MOV_BSETI, RV64_MOV_DST, 0, 0, rv64_mov_ctz64(z), ++c);
                z &= z - 1;
            }
            if (c) rv64_mov_best_try(&best, &p);
        }
    }

    {
        uint32_t low12 = (uint32_t)v & 0xfff;
        if (low12) {
            int32_t a = rv64_mov_sext12(low12);
            uint64_t w = v - (uint64_t)(int64_t)a;
            if (w && w != v && rv64_mov_min_ops64(ctx, w) + 1 <= best.cost
                && rv64_mov_seq64(ctx, w, depth + 1, nscratch, &sub))
                rv64_mov_best_try1(&best, &sub, RV64_MOV_ADDI, RV64_MOV_DST, 0, a);
        }
    }

    t = rv64_mov_ctz64(v);
    if (v && t >= 12 && t < 64) {
        uint64_t hi = v >> t;
        uint64_t his = (uint64_t)(sv >> t);
        if (rv64_mov_min_ops64(ctx, hi) + 1 <= best.cost
            && rv64_mov_seq64(ctx, hi, depth + 1, nscratch, &sub))
            rv64_mov_best_try1(&best, &sub, RV64_MOV_SLLI, RV64_MOV_DST, 0, t);
        if (his != hi && rv64_mov_min_ops64(ctx, his) + 1 <= best.cost
            && rv64_mov_seq64(ctx, his, depth + 1, nscratch, &sub))
            rv64_mov_best_try1(&best, &sub, RV64_MOV_SLLI, RV64_MOV_DST, 0, t);
        if (ctx->caps.zba && t < 32 && hi < 0x100000000ULL && (hi & 0x80000000ULL)) {
            if (rv64_mov_min_ops64(ctx, (uint64_t)(int64_t)(int32_t)hi) + 1 <= best.cost
                && rv64_mov_seq64(ctx, (uint64_t)(int64_t)(int32_t)hi, depth + 1, nscratch, &sub))
                rv64_mov_best_try1(&best, &sub, RV64_MOV_SLLIUW, RV64_MOV_DST, 0, t);
        }
    }

    if (depth < RV64_MOV_MAXDEPTH && best.cost > 2 && ctx->budget > 0 && rv64_mov_min_ops64(ctx, v) < best.cost) {
        ctx->budget--;
        uint32_t low12 = (uint32_t)v & 0xfff;

        {
            int lz = rv64_mov_clz64(v);
            for (int s = 1; s <= lz; ++s) {
                uint64_t w = v << s;
                int32_t sw = (int32_t)(uint32_t)w;
                if ((uint64_t)(int64_t)sw != w || (w & 0xfff)) continue;
                rv64_mov_seq_zero(&p);
                rv64_mov_seq_add(&p, RV64_MOV_LUI, RV64_MOV_DST, 0, 0, (int32_t)((w >> 12) & 0xfffff), 1);
                rv64_mov_seq_add(&p, RV64_MOV_SRLI, RV64_MOV_DST, 0, 0, s, 2);
                rv64_mov_best_try(&best, &p);
            }
        }

        if (sv < 0 && rv64_mov_min_ops64(ctx, ~v) + 1 <= best.cost) {
            if (rv64_mov_seq64(ctx, ~v, depth + 1, nscratch, &sub))
                rv64_mov_best_try1(&best, &sub, RV64_MOV_XORI, RV64_MOV_DST, 0, -1);
        }

        if (low12 >= 0x800) {
            uint64_t w = v ^ (uint64_t)(int64_t)rv64_mov_sext12(low12);
            if (w != v && rv64_mov_min_ops64(ctx, w) + 1 <= best.cost && rv64_mov_seq64(ctx, w, depth + 1, nscratch, &sub))
                rv64_mov_best_try1(&best, &sub, RV64_MOV_XORI, RV64_MOV_DST, 0,
                    rv64_mov_sext12(low12));
        }

        if (ctx->caps.zbs) {
            uint64_t hi_zeros = ~v & 0xffffffff80000000ULL;
            n = rv64_mov_popcnt64(hi_zeros);
            if (n > 0 && n <= 3) {
                uint64_t nval = v | 0xffffffff80000000ULL;
                if (nval != v && rv64_mov_min_ops64(ctx, nval) + n <= best.cost && rv64_mov_seq64(ctx, nval, depth + 1, nscratch, &sub)) {
                    uint64_t z = hi_zeros;
                    rv64_mov_seq_copy(&p, &sub);
                    int c = sub.depth;
                    while (z) {
                        rv64_mov_seq_add(&p, RV64_MOV_BCLRI, RV64_MOV_DST, 0, 0, rv64_mov_ctz64(z), ++c);
                        z &= z - 1;
                    }
                    rv64_mov_best_try(&best, &p);
                }
            }
        }

        if (ctx->caps.zba) {
            static const int mk[3] = { 9, 5, 3 };
            static const uint8_t mop[3] = { RV64_MOV_SH3ADD, RV64_MOV_SH2ADD, RV64_MOV_SH1ADD };
            for (int i = 0; i < 3; ++i) {
                if (v % mk[i] == 0 && rv64_mov_min_ops64(ctx, v / mk[i]) + 1 <= best.cost && rv64_mov_seq64(ctx, v / mk[i], depth + 1, nscratch, &sub))
                    rv64_mov_best_try1(&best, &sub, mop[i], RV64_MOV_DST, RV64_MOV_DST, 0);
            }
        }

        for (int sh = 12; sh < 64; ++sh) {
            uint64_t mask = (UINT64_C(1) << sh) - 1;
            uint64_t low = v & mask;
            int64_t lo = (int64_t)low;
            uint64_t w, hi, his;
            if (low >> (sh - 1)) lo = (int64_t)(low - (UINT64_C(1) << sh));
            if (lo < -2048 || lo > 2047) continue;
            w = v - (uint64_t)lo;
            hi = w >> sh;
            his = (uint64_t)((int64_t)w >> sh);
            int ov = 1 + !!lo;
            if (rv64_mov_min_ops64(ctx, hi) + ov <= best.cost && rv64_mov_seq64(ctx, hi, depth + 1, nscratch, &sub)) {
                rv64_mov_seq_copy(&p, &sub);
                rv64_mov_seq_add(&p, RV64_MOV_SLLI, RV64_MOV_DST, 0, 0, sh, sub.depth + 1);
                if (lo) rv64_mov_seq_add(&p, RV64_MOV_ADDI, RV64_MOV_DST, 0, 0, (int32_t)lo, sub.depth + 2);
                rv64_mov_best_try(&best, &p);
            }
            if (his != hi && rv64_mov_min_ops64(ctx, his) + ov <= best.cost && rv64_mov_seq64(ctx, his, depth + 1, nscratch, &sub)) {
                rv64_mov_seq_copy(&p, &sub);
                rv64_mov_seq_add(&p, RV64_MOV_SLLI, RV64_MOV_DST, 0, 0, sh, sub.depth + 1);
                if (lo) rv64_mov_seq_add(&p, RV64_MOV_ADDI, RV64_MOV_DST, 0, 0, (int32_t)lo, sub.depth + 2);
                rv64_mov_best_try(&best, &p);
            }
            if (ctx->caps.zba && sh < 32 && hi < 0x100000000ULL && (hi & 0x80000000ULL)) {
                if (rv64_mov_min_ops64(ctx, (uint64_t)(int64_t)(int32_t)hi) + ov <= best.cost && rv64_mov_seq64(ctx, (uint64_t)(int64_t)(int32_t)hi, depth + 1, nscratch, &sub)) {
                    rv64_mov_seq_copy(&p, &sub);
                    rv64_mov_seq_add(&p, RV64_MOV_SLLIUW, RV64_MOV_DST, 0, 0, sh, sub.depth + 1);
                    if (lo) rv64_mov_seq_add(&p, RV64_MOV_ADDI, RV64_MOV_DST, 0, 0, (int32_t)lo, sub.depth + 2);
                    rv64_mov_best_try(&best, &p);
                }
            }
        }

        if (nscratch && v > 0xffffffffULL) {
            uint32_t lo = (uint32_t)v;
            uint64_t hi = v >> 32;

            if (lo == hi) {
                if (rv64_mov_seq32(ctx, lo, depth + 1, &sub)) {
                    rv64_mov_seq_copy(&p, &sub);
                    rv64_mov_seq_rename(&p, RV64_MOV_DST, RV64_MOV_S1);
                    rv64_mov_seq_add(&p, RV64_MOV_SLLI2, RV64_MOV_DST, RV64_MOV_S1, 0, 32, p.depth + 1);
                    rv64_mov_seq_add(&p, RV64_MOV_ADD, RV64_MOV_DST, RV64_MOV_DST, RV64_MOV_S1, 0, p.depth + 1);
                    rv64_mov_best_try(&best, &p);
                }
                if ((lo & 0x80000000u) && rv64_mov_seq32(ctx, ~lo & 0xffffffffu, depth + 1, &sub)) {
                    rv64_mov_seq_copy(&p, &sub);
                    rv64_mov_seq_rename(&p, RV64_MOV_DST, RV64_MOV_S1);
                    rv64_mov_seq_add(&p, RV64_MOV_SLLI2, RV64_MOV_DST, RV64_MOV_S1, 0, 32, p.depth + 1);
                    rv64_mov_seq_add(&p, RV64_MOV_ADD, RV64_MOV_DST, RV64_MOV_DST, RV64_MOV_S1, 0, p.depth + 1);
                    rv64_mov_seq_add(&p, RV64_MOV_XORI, RV64_MOV_DST, 0, 0, -1, p.depth + 1);
                    rv64_mov_best_try(&best, &p);
                }
            } else {
                if (rv64_mov_seq32(ctx, lo, depth + 1, &sub)) {
                    rv64_mov_seq_t lo_seq, hi_seq;
                    rv64_mov_seq_copy(&lo_seq, &sub);
                    if (rv64_mov_seq_high(ctx, hi, 32, depth + 1, 0, &hi_seq)) {
                        rv64_mov_seq_rename(&lo_seq, RV64_MOV_DST, RV64_MOV_S1);
                        p = lo_seq;
                        rv64_mov_seq_concat(&p, &hi_seq);
                        rv64_mov_seq_add(&p, RV64_MOV_ADD, RV64_MOV_DST, RV64_MOV_DST, RV64_MOV_S1, 0, p.depth + 1);
                        rv64_mov_best_try(&best, &p);
                    }
                }
                if (hi == (uint64_t)(uint32_t)(lo - 1) && (lo & 0x80000000u)) {
                    if (rv64_mov_seq64(ctx, (uint64_t)(int64_t)(int32_t)lo, depth + 1, 0, &sub)) {
                        rv64_mov_seq_copy(&p, &sub);
                        rv64_mov_seq_rename(&p, RV64_MOV_DST, RV64_MOV_S1);
                        rv64_mov_seq_add(&p, RV64_MOV_SLLI2, RV64_MOV_DST, RV64_MOV_S1, 0, 32, p.depth + 1);
                        rv64_mov_seq_add(&p, RV64_MOV_ADD, RV64_MOV_DST, RV64_MOV_DST, RV64_MOV_S1, 0, p.depth + 1);
                        rv64_mov_best_try(&best, &p);
                    }
                }
                if (!(lo & 0x80000000u)) {
                    uint32_t dlt = (uint32_t)hi - lo;
                    if (hi == (uint64_t)(uint32_t)~lo) {
                        if (rv64_mov_seq32(ctx, lo, depth + 1, &sub)) {
                            rv64_mov_seq_copy(&p, &sub);
                            rv64_mov_seq_rename(&p, RV64_MOV_DST, RV64_MOV_S1);
                            rv64_mov_seq_add(&p, RV64_MOV_XORI2, RV64_MOV_S2, RV64_MOV_S1, 0, -1, p.depth + 1);
                            rv64_mov_seq_add(&p, RV64_MOV_SLLI, RV64_MOV_S2, 0, 0, 32, p.depth + 1);
                            rv64_mov_seq_add(&p, RV64_MOV_ADD, RV64_MOV_DST, RV64_MOV_S2, RV64_MOV_S1, 0, p.depth + 1);
                            rv64_mov_best_try(&best, &p);
                        }
                    }
                    if (nscratch >= 2 && dlt > 0 && dlt <= 2047) {
                        if (rv64_mov_seq32(ctx, lo, depth + 1, &sub)) {
                            rv64_mov_seq_copy(&p, &sub);
                            rv64_mov_seq_rename(&p, RV64_MOV_DST, RV64_MOV_S1);
                            rv64_mov_seq_add(&p, RV64_MOV_ADDI2, RV64_MOV_S2, RV64_MOV_S1, 0, (int32_t)dlt, p.depth + 1);
                            rv64_mov_seq_add(&p, RV64_MOV_SLLI, RV64_MOV_S2, 0, 0, 32, p.depth + 1);
                            rv64_mov_seq_add(&p, RV64_MOV_ADD, RV64_MOV_DST, RV64_MOV_S2, RV64_MOV_S1, 0, p.depth + 1);
                            rv64_mov_best_try(&best, &p);
                        }
                    }
                    if (nscratch >= 2 && ctx->caps.zba) {
                        static const int mk[3] = { 3, 5, 9 };
                        static const uint8_t mop[3] = { RV64_MOV_SH1ADD, RV64_MOV_SH2ADD, RV64_MOV_SH3ADD };
                        for (int i = 0; i < 3; ++i) {
                            if (hi != (uint64_t)(uint32_t)(lo * mk[i])) continue;
                            if (rv64_mov_seq32(ctx, lo, depth + 1, &sub)) {
                                rv64_mov_seq_copy(&p, &sub);
                                rv64_mov_seq_rename(&p, RV64_MOV_DST, RV64_MOV_S1);
                                rv64_mov_seq_add(&p, mop[i], RV64_MOV_S2, RV64_MOV_S1, 0, 0, p.depth + 1);
                                rv64_mov_seq_add(&p, RV64_MOV_SLLI, RV64_MOV_S2, 0, 0, 32, p.depth + 1);
                                rv64_mov_seq_add(&p, RV64_MOV_ADD, RV64_MOV_DST, RV64_MOV_S2, RV64_MOV_S1, 0, p.depth + 1);
                                rv64_mov_best_try(&best, &p);
                            }
                        }
                    }
                }

                for (int sh = 33; sh <= 40; ++sh) {
                    uint64_t mask = (UINT64_C(1) << sh) - 1;
                    uint64_t lv = v & mask;
                    uint64_t hv = v >> sh;
                    if (!lv || !hv) continue;
                    if (!rv64_mov_seq64(ctx, lv, depth + 1, 0, &sub)) continue;
                    {
                        rv64_mov_seq_t lo_seq, hi_seq;
                        rv64_mov_seq_copy(&lo_seq, &sub);
                        if (!rv64_mov_seq_high(ctx, hv, sh, depth + 1, 0, &hi_seq)) continue;
                        rv64_mov_seq_rename(&lo_seq, RV64_MOV_DST, RV64_MOV_S1);
                        p = lo_seq;
                        rv64_mov_seq_concat(&p, &hi_seq);
                        rv64_mov_seq_add(&p, RV64_MOV_ADD, RV64_MOV_DST, RV64_MOV_DST, RV64_MOV_S1, 0, p.depth + 1);
                        rv64_mov_best_try(&best, &p);
                    }
                }
            }
        }

        if (ctx->caps.zbb) {
            uint64_t notv = ~v;
            if (notv) {
                int lo_z = rv64_mov_ctz64(notv);
                int hi_z = rv64_mov_clz64(notv);
                if (lo_z > 0 && rv64_mov_popcnt64(notv) == 64 - lo_z - hi_z) {
                    uint64_t rot = (v >> lo_z) | (v << (64 - lo_z));
                    if (rv64_mov_seq64(ctx, rot, depth + 1, nscratch, &sub))
                        rv64_mov_best_try1(&best, &sub, RV64_MOV_RORI, RV64_MOV_DST, 0, 64 - lo_z);
                }
            }
        }
    }

    if (!best.cost) return 0;
    rv64_mov_seq_copy(out, &best.seq);
    return out->n;
}

int rv64_mov_seq32(rv64_movctx_t* ctx, uint32_t v, int depth, rv64_mov_seq_t* out)
{
    rv64_movbest_t best;
    best.cost = 99;
    best.depth = 99;
    best.seq.n = 0;
    best.seq.depth = 0;
    best.seq.overflow = 0;
    rv64_mov_seq_t p, sub;
    int t;

    if (!v || rv64_mov_fits12(v)) {
        rv64_mov_seq_zero(&p);
        rv64_mov_seq_add(&p, RV64_MOV_ADDI, RV64_MOV_DST, 0, 0, (int32_t)v, 1);
        rv64_mov_best_try(&best, &p);
    }

    if (ctx->caps.zbs && v && !(v & (v - 1))) {
        rv64_mov_seq_zero(&p);
        rv64_mov_seq_add(&p, RV64_MOV_BSETI, RV64_MOV_DST, 0, 0, rv64_mov_ctz64(v), 1);
        rv64_mov_best_try(&best, &p);
    }

    if (v && !(v & (v + 1))) {
        rv64_mov_seq_zero(&p);
        rv64_mov_seq_add(&p, RV64_MOV_ADDI, RV64_MOV_DST, 0, 0, -1, 1);
        rv64_mov_seq_add(&p, RV64_MOV_SRLI, RV64_MOV_DST, 0, 0, rv64_mov_clz64(v), 2);
        rv64_mov_best_try(&best, &p);
    }

    if (!(v & 0x80000000u) && !rv64_mov_fits12(v)) {
        int32_t hi20 = (int32_t)(((uint64_t)v + 0x800) >> 12) & 0xfffff;
        int32_t lo12 = rv64_mov_sext12(v);
        rv64_mov_seq_zero(&p);
        if (!lo12) {
            rv64_mov_seq_add(&p, RV64_MOV_LUI, RV64_MOV_DST, 0, 0, hi20, 1);
        } else {
            rv64_mov_seq_add(&p, RV64_MOV_LUI, RV64_MOV_DST, 0, 0, hi20, 1);
            rv64_mov_seq_add(&p, RV64_MOV_ADDIW, RV64_MOV_DST, 0, 0, lo12, 2);
        }
        rv64_mov_best_try(&best, &p);
    }

    if (depth < RV64_MOV_MAXDEPTH && (v & 0x80000000u) && ctx->budget > 0) {
        ctx->budget--;
        uint32_t w = v & 0x7fffffff;

        if (ctx->caps.zbs) {
            if (!w) {
                rv64_mov_seq_zero(&p);
                rv64_mov_seq_add(&p, RV64_MOV_BSETI, RV64_MOV_DST, 0, 0, 31, 1);
                rv64_mov_best_try(&best, &p);
            } else if (rv64_mov_seq32(ctx, w, depth + 1, &sub)) {
                rv64_mov_best_try1(&best, &sub, RV64_MOV_BSETI, RV64_MOV_DST, 0, 31);
            }
        }
        if (rv64_mov_seq64(ctx, (uint64_t)(int64_t)(int32_t)v, depth + 1, 0, &sub)) {
            if (ctx->caps.zba)
                rv64_mov_best_try1(&best, &sub, RV64_MOV_ZEXTW, RV64_MOV_DST, 0, 0);
            {
                rv64_mov_seq_t q;
                rv64_mov_seq_copy(&q, &sub);
                rv64_mov_seq_add(&q, RV64_MOV_SLLI, RV64_MOV_DST, 0, 0, 32, sub.depth + 1);
                rv64_mov_seq_add(&q, RV64_MOV_SRLI, RV64_MOV_DST, 0, 0, 32, sub.depth + 2);
                rv64_mov_best_try(&best, &q);
            }
        }
    }

    t = rv64_mov_ctz64(v);
    if (depth < RV64_MOV_MAXDEPTH && v && t >= 12 && t <= 31 && ctx->budget > 0) {
        ctx->budget--;
        if (rv64_mov_seq32(ctx, v >> t, depth + 1, &sub))
            rv64_mov_best_try1(&best, &sub, RV64_MOV_SLLI, RV64_MOV_DST, 0, t);
    }

    if (depth < RV64_MOV_MAXDEPTH && best.cost > 2 && ctx->budget > 0) {
        ctx->budget--;
        for (int sh = 12; sh < 32; ++sh) {
            uint32_t low = v & ((UINT32_C(1) << sh) - 1);
            int32_t lo;
            uint32_t hi;
            if (low >> (sh - 1))
                lo = (int32_t)(low - (UINT32_C(1) << sh));
            else
                lo = (int32_t)low;
            if (lo < -2048 || lo > 2047) continue;
            hi = (v - (uint32_t)lo) >> sh;
            if (!rv64_mov_seq32(ctx, hi, depth + 1, &sub)) continue;
            rv64_mov_seq_copy(&p, &sub);
            rv64_mov_seq_add(&p, RV64_MOV_SLLI, RV64_MOV_DST, 0, 0, sh, sub.depth + 1);
            if (lo) rv64_mov_seq_add(&p, RV64_MOV_ADDI, RV64_MOV_DST, 0, 0, lo, sub.depth + 2);
            rv64_mov_best_try(&best, &p);
        }
    }

    if (!best.cost) return 0;
    rv64_mov_seq_copy(out, &best.seq);
    return out->n;
}

#define EMIT(A) emit_fn(emit_ctx, (uint32_t)(A))

void rv64_mov_emit_seq(int reg, int s1, int s2, const rv64_mov_seq_t* seq, rv64_mov_emit_fn emit_fn, void* emit_ctx)
{
    int vreg[3] = { reg, s1, s2 };
    uint8_t init = 0;

    for (int i = 0; i < seq->n; ++i) {
        const rv64_movop_t* o = &seq->op[i];
        int rd = vreg[o->rd];
        int rs = vreg[o->rs];
        int first = !(init & (1 << o->rd));
        init |= (uint8_t)(1 << o->rd);

        switch (o->op) {
            case RV64_MOV_ADDI: ADDI(rd, first ? xZR : rd, o->imm); break;
            case RV64_MOV_ADDIW: ADDIW(rd, first ? xZR : rd, o->imm); break;
            case RV64_MOV_LUI: LUI(rd, o->imm); break;
            case RV64_MOV_SLLI: SLLI(rd, rd, o->imm); break;
            case RV64_MOV_SRLI: SRLI(rd, rd, o->imm); break;
            case RV64_MOV_SLLIUW: SLLIUW(rd, rd, o->imm); break;
            case RV64_MOV_BSETI: BSETI_(rd, first ? xZR : rd, o->imm); break;
            case RV64_MOV_BCLRI: BCLI(rd, first ? xZR : rd, o->imm); break;
            case RV64_MOV_XORI: XORI(rd, rd, o->imm); break;
            case RV64_MOV_ZEXTW: ZEXTW(rd, rd); break;
            case RV64_MOV_RORI: RORI(rd, rd, o->imm); break;
            case RV64_MOV_ADD: ADD(rd, rs, vreg[o->rs2]); break;
            case RV64_MOV_SLLI2: SLLI(rd, rs, o->imm); break;
            case RV64_MOV_SH1ADD: SH1ADD(rd, rs, rs); break;
            case RV64_MOV_SH2ADD: SH2ADD(rd, rs, rs); break;
            case RV64_MOV_SH3ADD: SH3ADD(rd, rs, rs); break;
            case RV64_MOV_ADDI2: ADDI(rd, rs, o->imm); break;
            case RV64_MOV_XORI2: XORI(rd, rs, o->imm); break;
            default: break;
        }
    }
}

#define RV64_MOV_CACHE_SIZE 1024

typedef struct rv64_mov_cache_s {
    uint64_t v;
    uint32_t key;
    uint8_t n;
    uint8_t used;
    uint8_t pad[2];
    rv64_movop_t op[8];
} rv64_mov_cache_t;

static __thread rv64_mov_cache_t rv64_mov_cache[RV64_MOV_CACHE_SIZE];

static unsigned rv64_mov_cache_slot(uint64_t v, uint32_t key)
{
    uint64_t h = (v + ((uint64_t)key << 56)) * UINT64_C(0x9E3779B97F4A7C15);
    return (unsigned)(h >> 54) & (RV64_MOV_CACHE_SIZE - 1);
}

int rv64_mov_cache_get(uint64_t v, int nscratch, int is32, rv64_mov_seq_t* out)
{
    uint32_t key = (uint32_t)nscratch | ((uint32_t)is32 << 8);
    rv64_mov_cache_t* e = &rv64_mov_cache[rv64_mov_cache_slot(v, key)];
    if (!e->used || e->v != v || e->key != key) return 0;
    out->n = e->n;
    out->depth = 0;
    out->overflow = 0;
    memcpy(out->op, e->op, (size_t)e->n * sizeof(rv64_movop_t));
    return 1;
}

void rv64_mov_cache_put(uint64_t v, int nscratch, int is32, const rv64_mov_seq_t* seq)
{
    uint32_t key = (uint32_t)nscratch | ((uint32_t)is32 << 8);
    rv64_mov_cache_t* e;
    if (!seq->n || seq->n > 8) return;
    e = &rv64_mov_cache[rv64_mov_cache_slot(v, key)];
    e->v = v;
    e->key = key;
    e->n = seq->n;
    e->used = 1;
    memcpy(e->op, seq->op, (size_t)seq->n * sizeof(rv64_movop_t));
}
