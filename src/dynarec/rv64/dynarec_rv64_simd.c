// SPDX-License-Identifier: MIT
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

#include "bitutils.h"
#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "rv64_emitter.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "../dynablock_private.h"
#include "custommem.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "../dynarec_helper.h"
#include "rv64_mov_imm.h"

static void simd_record_read(dynarec_rv64_t* dyn, int ninst, int a, unsigned regions)
{
#if STEP == 0
    if (ninst >= 0 && ninst < dyn->size) dyn->insts[ninst].simd_read |= (uint64_t)regions << (a * 4);
#endif
}

static void simd_record_write(dynarec_rv64_t* dyn, int ninst, int a, unsigned regions)
{
#if STEP == 0
    if (ninst >= 0 && ninst < dyn->size) {
        uint64_t bits = (uint64_t)regions << (a * 4);
        dyn->insts[ninst].simd_defs |= bits;
        dyn->insts[ninst].simd_zero_kill |= bits;
        dyn->insts[ninst].simd_zero_gen &= ~bits;
    }
#endif
}

static void rvv_note_requirement(dynarec_rv64_t* dyn, int ninst, int sew, int lmul, unsigned vl)
{
#if STEP == 0
    if (ninst >= 0 && ninst < dyn->size && !dyn->insts[ninst].rvv_hint.vl) {
        rvv_config_t hint = { .bits = 0 };
        hint.sew = sew;
        hint.lmul = lmul;
        hint.vl = vl;
        dyn->insts[ninst].rvv_hint = hint;
    }
#endif
}

static void simd_record_opaque(dynarec_rv64_t* dyn, int ninst)
{
#if STEP == 0
    if (ninst >= 0 && ninst < dyn->size) dyn->insts[ninst].simd_opaque = 1;
#endif
}

void simd_snapshot(dynarec_rv64_t* dyn, int a)
{
    simd_save_entry(&dyn->e, a);
    const sse_cache_t* s = &dyn->e.ssecache[a];
    if (s->v == -1 || dyn->e.olds[a].v) return;
    int idx = EXTIDX(s->reg);
    if (dyn->e.news & (1u << idx)) return;
    dyn->e.olds[a] = dyn->e.extcache[idx];
}

int simd_get_fp_reg(dynarec_rv64_t* dyn, int ninst, int s1, int a, int single)
{
    simd_record_opaque(dyn, ninst);
    if (dyn->e.ssecache[a].v != -1) {
        if (dyn->e.ssecache[a].vector == 1) {
            simd_forget_reg(dyn, ninst, s1, a);
            return simd_get_fp_reg(dyn, ninst, s1, a, single);
        }
        if (dyn->e.ssecache[a].single != single) {
            simd_forget_fp_reg(dyn, ninst, s1, a);
            return simd_get_fp_reg(dyn, ninst, s1, a, single);
        }
        return dyn->e.ssecache[a].reg;
    }
    if (dyn->e.simd[a].dirty) simd_forget_reg(dyn, ninst, s1, a);
    dyn->e.ssecache[a].reg = simd_alloc_reg(dyn, single ? EXT_CACHE_SS : EXT_CACHE_SD, a);
    int ret = dyn->e.ssecache[a].reg;
    dyn->e.ssecache[a].single = single;
    dyn->e.ssecache[a].vector = 0;
    if (dyn->e.ssecache[a].single)
        FLW(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
    else
        FLD(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
    return ret;
}

int simd_get_fp_reg_empty(dynarec_rv64_t* dyn, int ninst, int s1, int a, int single)
{
    simd_record_opaque(dyn, ninst);
    if (dyn->e.ssecache[a].v != -1) {
        if (dyn->e.ssecache[a].vector == 1) {
            simd_forget_reg(dyn, ninst, s1, a);
            return simd_get_fp_reg_empty(dyn, ninst, s1, a, single);
        }

        if (dyn->e.ssecache[a].single != single) {
            simd_snapshot(dyn, a);
            if (dyn->e.ssecache[a].single)
                FSW(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
            else
                FSD(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
            dyn->e.ssecache[a].single = single;
            dyn->e.ssecache[a].vector = 0;
            dyn->e.extcache[EXTIDX(dyn->e.ssecache[a].reg)].t = single ? EXT_CACHE_SS : EXT_CACHE_SD;
        }
        return dyn->e.ssecache[a].reg;
    }
    if (dyn->e.simd[a].dirty) simd_forget_reg(dyn, ninst, s1, a);
    dyn->e.ssecache[a].reg = simd_alloc_reg(dyn, single ? EXT_CACHE_SS : EXT_CACHE_SD, a);
    dyn->e.ssecache[a].single = single;
    dyn->e.ssecache[a].vector = 0;
    return dyn->e.ssecache[a].reg;
}

int simd_get_fp_reg_resized(dynarec_rv64_t* dyn, int ninst, int s1, int a, int single)
{
    simd_record_opaque(dyn, ninst);
    if (dyn->e.ssecache[a].v != -1) {
        if (dyn->e.ssecache[a].vector == 1) {
            simd_forget_reg(dyn, ninst, s1, a);
            return simd_get_fp_reg_resized(dyn, ninst, s1, a, single);
        }

        simd_forget_fp_reg(dyn, ninst, s1, a);
        return simd_get_fp_reg_resized(dyn, ninst, s1, a, single);
    }
    if (dyn->e.simd[a].dirty) simd_forget_reg(dyn, ninst, s1, a);
    dyn->e.ssecache[a].reg = simd_alloc_reg(dyn, single ? EXT_CACHE_SS : EXT_CACHE_SD, a);
    int ret = dyn->e.ssecache[a].reg;
    dyn->e.ssecache[a].single = single;
    dyn->e.ssecache[a].vector = 0;
    if (!single)
        FLW(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
    else
        FLD(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
    return ret;
}

void simd_forget_fp_reg(dynarec_rv64_t* dyn, int ninst, int s1, int a)
{
    simd_record_opaque(dyn, ninst);
    if (dyn->e.ssecache[a].v == -1) {
        simd_forget_reg(dyn, ninst, s1, a);
        return;
    }
    if (dyn->e.ssecache[a].vector == 1) return simd_forget_reg(dyn, ninst, s1, a);
    simd_snapshot(dyn, a);
    if (dyn->e.ssecache[a].single)
        FSW(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
    else
        FSD(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
    extcache_free_reg(dyn, dyn->e.ssecache[a].reg);
    dyn->e.ssecache[a].v = -1;
    return;
}

static void simd_check(const dynarec_rv64_t* dyn, int a)
{
    const simd_cache_t* c = &dyn->e.simd[a];
    assert(!((c->valid | c->zero | c->dirty) & ~SIMD_YMM));
    assert(!(c->dirty & ~(c->valid | c->zero)));
    assert(!c->valid || (dyn->e.ssecache[a].v != -1 && dyn->e.ssecache[a].vector));
    assert(cpuext.vlen >= 32 || !(c->valid & SIMD_HI128));
}

static unsigned simd_regions(unsigned bytes)
{
    assert(bytes == 4 || bytes == 8 || bytes == 16 || bytes == 32);
    return bytes == 4 ? SIMD_LO32 : bytes == 8 ? SIMD_LO64
        : bytes == 16                          ? SIMD_XMM
                                               : SIMD_YMM;
}

static unsigned simd_word_mask(unsigned regions)
{
    return (regions & SIMD_LO32 ? 1 : 0) | (regions & SIMD_HI32 ? 2 : 0) | (regions & SIMD_HI64 ? 12 : 0) | (regions & SIMD_HI128 ? 240 : 0);
}

static int simd_allocate(dynarec_rv64_t* dyn, int ninst, int s1, int a)
{
    if (dyn->e.ssecache[a].v != -1 && !dyn->e.ssecache[a].vector) simd_forget_fp_reg(dyn, ninst, s1, a);
    simd_snapshot(dyn, a);
    if (dyn->e.ssecache[a].v == -1) {
        dyn->e.ssecache[a].v = 0;
        dyn->e.ssecache[a].reg = simd_alloc_reg(dyn, EXT_CACHE_SIMD, a);
        dyn->e.ssecache[a].vector = 1;
    }
    return dyn->e.ssecache[a].reg;
}

static void simd_mask(dynarec_rv64_t* dyn, int ninst, int s1, unsigned regions, unsigned bytes)
{
    unsigned words = simd_word_mask(regions);
    if (cpuext.xtheadvector) {
        rvv_mask(dyn, ninst, VMASK, words, s1);
    } else {
        MOV32w(s1, words);
        VMV_S_X(VMASK, s1);
    }
}

void simd_materialize(dynarec_rv64_t* dyn, int ninst, int s1, int a, unsigned regions)
{
    simd_record_read(dyn, ninst, a, regions);
    simd_cache_t* c = &dyn->e.simd[a];
    unsigned missing = regions & ~c->valid;
    if (!missing) return;
    simd_snapshot(dyn, a);
    int reg = dyn->e.ssecache[a].reg;
    unsigned memory = missing & ~c->zero;
    if (memory == missing && (missing == SIMD_LO32 || missing == SIMD_LO64 || missing == SIMD_XMM)) {
        unsigned bytes = missing == SIMD_LO32 ? 4 : missing == SIMD_LO64 ? 8
                                                                         : 16;
        int sew = bytes == 4 ? VECTOR_SEW32 : bytes == 8   ? VECTOR_SEW64
            : dyn->rvv.vl && dyn->rvv.lmul == VECTOR_LMUL1 ? dyn->rvv.sew
                                                           : VECTOR_SEW32;
        rvv_set_config(dyn, ninst, s1, sew, VECTOR_LMUL1, bytes >> sew);
        ADDI(s1, xEmu, offsetof(x64emu_t, xmm[a]));
        VLE_V(reg, s1, sew, VECTOR_UNMASKED, VECTOR_NFIELD1);
    } else {
        unsigned bytes = missing & SIMD_HI128 ? 32 : 16;
        rvv_set_config(dyn, ninst, s1, VECTOR_SEW32, VECTOR_LMUL1, bytes / 4);
        if (memory & SIMD_XMM) {
            simd_mask(dyn, ninst, s1, memory & SIMD_XMM, bytes);
            ADDI(s1, xEmu, offsetof(x64emu_t, xmm[a]));
            VLE32_V(reg, s1, VECTOR_MASKED, VECTOR_NFIELD1);
        }
        if (memory & SIMD_HI128) {
            simd_mask(dyn, ninst, s1, SIMD_HI128, bytes);
            ADDI(s1, xEmu, offsetof(x64emu_t, ymm[a]) - 16);
            VLE32_V(reg, s1, VECTOR_MASKED, VECTOR_NFIELD1);
        }
        if (missing & c->zero) {
            simd_mask(dyn, ninst, s1, missing & c->zero, bytes);
            VMERGE_VIM(reg, reg, 0);
        }
    }
    c->valid |= missing;
    simd_check(dyn, a);
}

int simd_get(dynarec_rv64_t* dyn, int ninst, int s1, int a, unsigned bytes, int sew, int write)
{
    int reg = simd_allocate(dyn, ninst, s1, a);
    unsigned regions = simd_regions(bytes);
    simd_materialize(dyn, ninst, s1, a, regions);
    if (write) {
        simd_record_write(dyn, ninst, a, regions);
        simd_snapshot(dyn, a);
        dyn->e.simd[a].dirty |= regions;
        dyn->e.simd[a].zero &= ~regions;
    }
    int fixed = sew != VECTOR_SEWANY;
    if (sew == VECTOR_SEWANY) sew = dyn->rvv.vl && dyn->rvv.lmul == VECTOR_LMUL1 && (dyn->rvv.vl << dyn->rvv.sew) == bytes ? dyn->rvv.sew : VECTOR_SEW8;
    if (fixed) rvv_note_requirement(dyn, ninst, sew, VECTOR_LMUL1, bytes >> sew);
    rvv_set_config(dyn, ninst, s1, sew, VECTOR_LMUL1, bytes >> sew);
    return reg;
}

int simd_write(dynarec_rv64_t* dyn, int ninst, int s1, int a, unsigned bytes, int sew)
{
    int reg = simd_allocate(dyn, ninst, s1, a);
    simd_commit(dyn, ninst, a, simd_regions(bytes), 0);
    rvv_note_requirement(dyn, ninst, sew, VECTOR_LMUL1, bytes >> sew);
    rvv_set_config(dyn, ninst, s1, sew, VECTOR_LMUL1, bytes >> sew);
    return reg;
}

void simd_copy(dynarec_rv64_t* dyn, int ninst, int s1, int dst, int src, unsigned bytes, int vex)
{
    unsigned regions = simd_regions(bytes);
    if (dst == src) {
        if (vex && bytes == 16) simd_commit(dyn, ninst, dst, 0, SIMD_HI128);
        return;
    }
    int from = simd_allocate(dyn, ninst, s1, src);
    simd_record_read(dyn, ninst, src, regions);
    simd_materialize(dyn, ninst, s1, src, regions & ~dyn->e.simd[src].zero);
    simd_cache_t value = dyn->e.simd[src];
    int to = simd_allocate(dyn, ninst, s1, dst);
    simd_cache_t* c = &dyn->e.simd[dst];
    if (cpuext.xtheadvector || (!vex && bytes == 16 && (c->valid & SIMD_HI128))) {
        rvv_set_vl_for_width(dyn, ninst, s1, VECTOR_SEWANY, VECTOR_LMUL1, bytes);
        VMV_V_V(to, from);
    } else {
        VMV1R_V(to, from);
    }
    unsigned cleared = vex && bytes == 16 ? SIMD_HI128 : 0;
    simd_record_write(dyn, ninst, dst, regions | cleared);
    c->valid = (c->valid & ~(regions | cleared)) | (value.valid & regions);
    c->zero = (c->zero & ~regions) | (value.zero & regions) | cleared;
    c->dirty |= regions | cleared;
    simd_check(dyn, dst);
}

int simd_seed_scalar(dynarec_rv64_t* dyn, int ninst, int s1, int dst, int src, int sew)
{
    if (dst != src) simd_copy(dyn, ninst, s1, dst, src, 16, 1);
    int reg = simd_allocate(dyn, ninst, s1, dst);
    simd_commit(dyn, ninst, dst, sew == VECTOR_SEW32 ? SIMD_LO32 : SIMD_LO64, SIMD_HI128);
    rvv_set_config(dyn, ninst, s1, sew, VECTOR_LMUL1, 1);
    return reg;
}

void simd_scalar_arith(dynarec_rv64_t* dyn, int ninst, int s1, int dst, int left, int right, int sew, int opcode)
{
    int nan = 0, ordered = 0;
    if (!BOX64ENV(dynarec_fastnan)) {
        nan = vpu_get_scratch(dyn);
        ordered = vpu_get_scratch(dyn);
        int mask = vpu_get_scratch(dyn);
        VMFEQ_VV(VMASK, left, left, VECTOR_UNMASKED);
        VMFEQ_VV(mask, right, right, VECTOR_UNMASKED);
        VMAND_MM(ordered, VMASK, mask);
        VMERGE_VVM(nan, left, right);
        if (sew == VECTOR_SEW32) {
            MOV32w(s1, 0x00400000);
        } else {
            MOV64x(s1, 0x0008000000000000ULL);
        }
        VOR_VX(nan, nan, s1, VECTOR_UNMASKED);
    }
    switch (opcode) {
        case 0x51: VFSQRT_V(dst, right, VECTOR_UNMASKED); break;
        case 0x58: VFADD_VV(dst, left, right, VECTOR_UNMASKED); break;
        case 0x59: VFMUL_VV(dst, left, right, VECTOR_UNMASKED); break;
        case 0x5c: VFSUB_VV(dst, left, right, VECTOR_UNMASKED); break;
        case 0x5e: VFDIV_VV(dst, left, right, VECTOR_UNMASKED); break;
    }
    if (!BOX64ENV(dynarec_fastnan)) {
        VMFNE_VV(VMASK, dst, dst, VECTOR_UNMASKED);
        VMAND_MM(VMASK, VMASK, ordered);
        VFSGNJN_VV(dst, dst, dst, VECTOR_MASKED);
        VMNAND_MM(VMASK, ordered, ordered);
        VMERGE_VVM(dst, dst, nan);
    }
}

void simd_compare(dynarec_rv64_t* dyn, int ninst, int dst, int left, int right, unsigned predicate)
{
    int temp = vpu_get_scratch(dyn);
    int ordered = vpu_get_scratch(dyn);
    unsigned p = predicate & 15;
    if (p == 3 || p == 7 || p == 8 || p == 12) {
        VMFEQ_VV(ordered, left, left, VECTOR_UNMASKED);
        VMFEQ_VV(temp, right, right, VECTOR_UNMASKED);
        VMAND_MM(ordered, ordered, temp);
    }
    switch (p) {
        case 0: VMFEQ_VV(VMASK, left, right, VECTOR_UNMASKED); break;
        case 1: VMFLT_VV(VMASK, left, right, VECTOR_UNMASKED); break;
        case 2: VMFLE_VV(VMASK, left, right, VECTOR_UNMASKED); break;
        case 3: VMNAND_MM(VMASK, ordered, ordered); break;
        case 4:
        case 5:
        case 6:
            if (p == 4) {
                VMFEQ_VV(temp, left, right, VECTOR_UNMASKED);
            } else if (p == 5) {
                VMFLT_VV(temp, left, right, VECTOR_UNMASKED);
            } else {
                VMFLE_VV(temp, left, right, VECTOR_UNMASKED);
            }
            VMNAND_MM(VMASK, temp, temp);
            break;
        case 7: VMAND_MM(VMASK, ordered, ordered); break;
        case 8:
            VMFEQ_VV(temp, left, right, VECTOR_UNMASKED);
            VMNAND_MM(VMASK, ordered, ordered);
            VMOR_MM(VMASK, VMASK, temp);
            break;
        case 9:
        case 10:
            if (p == 9) {
                VMFLE_VV(temp, right, left, VECTOR_UNMASKED);
            } else {
                VMFLT_VV(temp, right, left, VECTOR_UNMASKED);
            }
            VMNAND_MM(VMASK, temp, temp);
            break;
        case 11: VMXOR_MM(VMASK, VMASK, VMASK); break;
        case 12:
            VMFEQ_VV(temp, left, right, VECTOR_UNMASKED);
            VMNAND_MM(VMASK, temp, temp);
            VMAND_MM(VMASK, VMASK, ordered);
            break;
        case 13: VMFLE_VV(VMASK, right, left, VECTOR_UNMASKED); break;
        case 14: VMFLT_VV(VMASK, right, left, VECTOR_UNMASKED); break;
        case 15: VMXNOR_MM(VMASK, VMASK, VMASK); break;
    }
    VXOR_VV(dst, dst, dst, VECTOR_UNMASKED);
    VMERGE_VIM(dst, dst, 0x1f);
}

void simd_commit(dynarec_rv64_t* dyn, int ninst, int a, unsigned written, unsigned zero)
{
    simd_record_write(dyn, ninst, a, written | zero);
    simd_snapshot(dyn, a);
    simd_cache_t* c = &dyn->e.simd[a];
    c->valid = (c->valid | written) & ~zero;
    c->zero = (c->zero & ~written) | zero;
    c->dirty |= written | zero;
    simd_check(dyn, a);
#if STEP == 0
    if (ninst >= 0 && ninst < dyn->size) dyn->insts[ninst].simd_zero_gen |= (uint64_t)zero << (a * 4);
#endif
}

void simd_flush(dynarec_rv64_t* dyn, int ninst, int s1, int a, unsigned regions, int clean)
{
    simd_cache_t* c = &dyn->e.simd[a];
    unsigned dirty = c->dirty & regions;
    unsigned zeros = dirty & c->zero;
    for (int r = 0; r < 4; ++r) {
        if (!(zeros & (1u << r))) continue;
        int offset = r == 0 ? offsetof(x64emu_t, xmm[a]) : r == 1 ? offsetof(x64emu_t, xmm[a]) + 4
            : r == 2                                              ? offsetof(x64emu_t, xmm[a]) + 8
                                                                  : offsetof(x64emu_t, ymm[a]);
        if (r < 2) {
            SW(xZR, xEmu, offset);
        } else {
            SD(xZR, xEmu, offset);
        }
        if (r == 3) { SD(xZR, xEmu, offset + 8); }
    }
    unsigned stored = dirty & ~zeros;
    assert(!(stored & ~c->valid));
    if (stored) {
        int reg = dyn->e.ssecache[a].reg;
        if (stored == SIMD_LO32 || stored == SIMD_LO64 || stored == SIMD_XMM) {
            unsigned bytes = stored == SIMD_LO32 ? 4 : stored == SIMD_LO64 ? 8
                                                                           : 16;
            int sew = bytes == 4 ? VECTOR_SEW32 : bytes == 8   ? VECTOR_SEW64
                : dyn->rvv.vl && dyn->rvv.lmul == VECTOR_LMUL1 ? dyn->rvv.sew
                                                               : VECTOR_SEW32;
            rvv_set_config(dyn, ninst, s1, sew, VECTOR_LMUL1, bytes >> sew);
            ADDI(s1, xEmu, offsetof(x64emu_t, xmm[a]));
            VSE_V(reg, s1, sew, VECTOR_UNMASKED, VECTOR_NFIELD1);
        } else {
            unsigned bytes = stored & SIMD_HI128 ? 32 : 16;
            rvv_set_config(dyn, ninst, s1, VECTOR_SEW32, VECTOR_LMUL1, bytes / 4);
            if (stored & SIMD_XMM) {
                simd_mask(dyn, ninst, s1, stored & SIMD_XMM, bytes);
                ADDI(s1, xEmu, offsetof(x64emu_t, xmm[a]));
                VSE32_V(reg, s1, VECTOR_MASKED, VECTOR_NFIELD1);
            }
            if (stored & SIMD_HI128) {
                simd_mask(dyn, ninst, s1, SIMD_HI128, bytes);
                ADDI(s1, xEmu, offsetof(x64emu_t, ymm[a]) - 16);
                VSE32_V(reg, s1, VECTOR_MASKED, VECTOR_NFIELD1);
            }
        }
    }
    if (clean && dirty) {
        simd_snapshot(dyn, a);
        c->dirty &= ~regions;
    }
    simd_check(dyn, a);
}

int simd_get_reg(dynarec_rv64_t* dyn, int ninst, int s1, int a, int forwrite, int sew)
{
    return simd_get(dyn, ninst, s1, a, 16, sew, forwrite);
}

int simd_get_reg_empty(dynarec_rv64_t* dyn, int ninst, int s1, int a)
{
    int reg = simd_allocate(dyn, ninst, s1, a);
    simd_commit(dyn, ninst, a, SIMD_XMM, 0);
    return reg;
}

void simd_forget_reg(dynarec_rv64_t* dyn, int ninst, int s1, int a)
{
    simd_snapshot(dyn, a);
    simd_flush(dyn, ninst, s1, a, SIMD_YMM, 1);
    dyn->e.simd[a] = (simd_cache_t) { 0 };
    if (dyn->e.ssecache[a].v != -1) {
        extcache_free_reg(dyn, dyn->e.ssecache[a].reg);
        dyn->e.ssecache[a].v = -1;
    }
}

void simd_purge_low(dynarec_rv64_t* dyn, int ninst, int s1)
{
    for (int a = 0; a < 8; ++a) {
        if (dyn->e.ssecache[a].v != -1 && !dyn->e.ssecache[a].vector)
            simd_forget_fp_reg(dyn, ninst, s1, a);
        else
            simd_forget_reg(dyn, ninst, s1, a);
    }
}

void simd_purge(dynarec_rv64_t* dyn, int ninst, int next, int s1)
{
    for (int a = 0; a < 16; ++a) {
        if (dyn->e.ssecache[a].v != -1 && !dyn->e.ssecache[a].vector) {
            if (dyn->e.ssecache[a].single) {
                FSW(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
            } else {
                FSD(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
            }
        }
        simd_flush(dyn, ninst, s1, a, SIMD_YMM, !next);
        if (!next) {
            simd_snapshot(dyn, a);
            if (dyn->e.ssecache[a].v != -1) extcache_free_reg(dyn, dyn->e.ssecache[a].reg);
            dyn->e.ssecache[a].v = -1;
            dyn->e.simd[a] = (simd_cache_t) { 0 };
        }
    }
}

void simd_reflect(dynarec_rv64_t* dyn, int ninst, int s1) { simd_purge(dyn, ninst, 1, s1); }

void simd_reflect_fp_reg(dynarec_rv64_t* dyn, int ninst, int s1, int a)
{
    if (dyn->e.ssecache[a].v != -1 && !dyn->e.ssecache[a].vector) {
        if (dyn->e.ssecache[a].single) {
            FSW(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
        } else {
            FSD(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
        }
    }
    simd_flush(dyn, ninst, s1, a, SIMD_YMM, 0);
}

void simd_prepare_entry(dynarec_rv64_t* dyn, int entry, int emit)
{
    int ninst = emit && entry ? entry - 1 : entry;
    if (entry < 0 || entry >= dyn->size || (dyn->insts[entry].x64.barrier & BARRIER_FLOAT)) return;
    instruction_rv64_t* inst = &dyn->insts[entry];
    int s1 = x1;
    if (inst->nat_flags_live & (1u << s1)) s1 = x2;
    if (inst->nat_flags_live & (1u << s1)) s1 = x3;
    assert(!(inst->nat_flags_live & (1u << s1)));
    if (inst->pred_sz > 1) {
        uint64_t keep = inst->simd_zero_in;
        for (int a = 0; a < 16; ++a, keep >>= 4) {
            simd_cache_t* c = &dyn->e.simd[a];
            unsigned drop = c->zero & ~keep;
            if (!drop) continue;
            unsigned memory = drop & ~c->valid;
            if (emit) simd_flush(dyn, ninst, s1, a, memory, 1);
            simd_snapshot(dyn, a);
            c->dirty &= ~memory;
            c->zero &= ~drop;
        }
    }
    if (!inst->simd_entry) return;
    int fpu_scratch = dyn->e.fpu_scratch;
    uint16_t vector_scratch = dyn->e.vpu_scratch;
    dyn->e.fpu_scratch = 0;
    dyn->e.vpu_scratch = 0;
    for (int size = 32; size >= 4; size >>= 1) {
        for (int a = 0; a < 16; ++a) {
            unsigned need = ((inst->simd_live | inst->simd_carry) >> (a * 4)) & SIMD_YMM;
            unsigned bytes = need & SIMD_HI128 ? 32 : need & SIMD_HI64 ? 16
                : need & SIMD_HI32                                     ? 8
                                                                       : 4;
            if (!need || bytes != size) continue;
            simd_cache_t* c = &dyn->e.simd[a];
            unsigned resident = need & ~c->zero;
            if (emit) {
                simd_allocate(dyn, ninst, s1, a);
                simd_materialize(dyn, ninst, s1, a, resident);
            } else {
                int reg = VREG(a), idx = EXTIDX(reg);
                dyn->e.ssecache[a].v = 0;
                dyn->e.ssecache[a].reg = reg;
                dyn->e.ssecache[a].vector = 1;
                dyn->e.extcache[idx].t = EXT_CACHE_SIMD;
                dyn->e.extcache[idx].n = a;
                dyn->e.fpuused[idx] = 1;
                c->valid |= resident;
            }
            c->dirty |= ((inst->simd_carry >> (a * 4)) & SIMD_YMM) & (c->valid | c->zero);
            simd_check(dyn, a);
        }
    }
    dyn->e.fpu_scratch = fpu_scratch;
    dyn->e.vpu_scratch = vector_scratch;
    if (emit && inst->rvv_entry_config.vl)
        rvv_set_config(dyn, ninst, s1, inst->rvv_entry_config.sew, inst->rvv_entry_config.lmul, inst->rvv_entry_config.vl);
    else
        dyn->rvv = inst->rvv_entry_config;
}

void simd_transform(dynarec_rv64_t* dyn, int ninst, int s1, const extcache_t* target)
{
    for (int a = 0; a < 16; ++a) {
        const sse_cache_t* dst = &target->ssecache[a];
        const simd_cache_t* want = &target->simd[a];
        sse_cache_t* src = &dyn->e.ssecache[a];
        int join = dyn->insts[ninst].x64.jmp_insts;
        unsigned proven = join >= 0 ? (dyn->insts[join].simd_zero_in >> (a * 4)) & SIMD_YMM : 0;
        assert(!(want->zero & ~(dyn->e.simd[a].zero | proven)));
        if (dst->v != -1 && dst->vector) {
            simd_allocate(dyn, ninst, s1, a);
            simd_flush(dyn, ninst, s1, a, SIMD_YMM & ~want->dirty, 1);
            simd_materialize(dyn, ninst, s1, a, want->valid);
        } else {
            if (src->v != -1 && src->vector)
                simd_forget_reg(dyn, ninst, s1, a);
            else
                simd_flush(dyn, ninst, s1, a, SIMD_YMM, 1);
            if (dst->v != -1)
                simd_get_fp_reg(dyn, ninst, s1, a, dst->single);
            else if (src->v != -1)
                simd_forget_fp_reg(dyn, ninst, s1, a);
        }
        dyn->e.simd[a] = *want;
    }
}

void simd_flush_upper(dynarec_rv64_t* dyn, int ninst, int s1, int a)
{
    simd_flush(dyn, ninst, s1, a, SIMD_HI128, 1);
    simd_snapshot(dyn, a);
    dyn->e.simd[a].valid &= ~SIMD_HI128;
    dyn->e.simd[a].zero &= ~SIMD_HI128;
}

void simd_sync_upper(dynarec_rv64_t* dyn, int ninst, int s1, int next)
{
    simd_record_opaque(dyn, ninst);
    for (int a = 0; a < 16; ++a) {
        simd_flush(dyn, ninst, s1, a, SIMD_HI128, !next);
        if (!next) {
            simd_snapshot(dyn, a);
            dyn->e.simd[a].valid &= ~SIMD_HI128;
            dyn->e.simd[a].zero &= ~SIMD_HI128;
        }
    }
}

void simd_set_width(dynarec_rv64_t* dyn, int ninst, int s1, int sew, int width)
{
    rvv_note_requirement(dyn, ninst, sew, VECTOR_LMUL1, width >> sew);
    dyn->vector_eew = rvv_set_config(dyn, ninst, s1, sew, VECTOR_LMUL1, width >> sew);
}

void simd_load_reg(dynarec_rv64_t* dyn, int ninst, int s1, int vreg, int a, int width, int sew)
{
    int reg = simd_get(dyn, ninst, s1, a, width, sew, 0);
    if (reg != vreg) { VMV_V_V(vreg, reg); }
}

void simd_store_reg(dynarec_rv64_t* dyn, int ninst, int s1, int vreg, int a, int width, int sew)
{
    int reg = simd_allocate(dyn, ninst, s1, a);
    if (reg != vreg) {
        simd_set_width(dyn, ninst, s1, sew, width);
        VMV_V_V(reg, vreg);
    }
    simd_commit(dyn, ninst, a, simd_regions(width), width == 16 ? SIMD_HI128 : 0);
}

int rvv_set_config(dynarec_rv64_t* dyn, int ninst, int s1, int sew, int lmul, unsigned vl)
{
    assert(sew <= VECTOR_SEW64 && vl && vl <= 65535);
    rvv_config_t next = { .bits = 0 };
    next.sew = sew;
    next.lmul = lmul;
    next.vl = vl;
    if (dyn->rvv.bits != next.bits) {
        unsigned type = (sew << (3 - !!cpuext.xtheadvector)) | lmul;
        int grouping = lmul < 4 ? lmul : lmul - 8;
        unsigned capacity = (cpuext.vlen >> sew);
        capacity = grouping >= 0 ? capacity << grouping : capacity >> -grouping;
        if (vl == capacity) {
            VSETVLI(s1, xZR, type);
        } else if (vl <= 31 && !cpuext.xtheadvector) {
            VSETIVLI(xZR, vl, type);
        } else {
            MOV32w(s1, vl);
            VSETVLI(xZR, s1, type);
        }
    }
    dyn->rvv = next;
    return sew;
}

static int rvv_branch_scratch(dynarec_rv64_t* dyn, int ninst, int condition_reg)
{
    instruction_rv64_t* inst = &dyn->insts[ninst];
    uint32_t live = inst->nat_flags_live;
    if (!inst->nat_flags_fusion && condition_reg >= 0 && condition_reg < 32) live |= 1u << condition_reg;
    if (inst->nat_flags_fusion) {
        if (inst->nat_flags_op1 < 32) live |= 1u << inst->nat_flags_op1;
        if (inst->nat_flags_op2 < 32) live |= 1u << inst->nat_flags_op2;
    }

    const uint8_t scratch[] = { x1, x2, x3, x4, x5 };
    for (unsigned i = 0; i < sizeof(scratch) / sizeof(scratch[0]); ++i)
        if (!(live & (1u << scratch[i]))) return scratch[i];
    assert(0 && "no dead GPR available for RVV branch setup");
    return x1;
}

void rvv_prepare_branch(dynarec_rv64_t* dyn, int ninst, int condition_reg)
{
    int target = dyn->insts[ninst].x64.jmp_insts;
    if (STEP < 2 || !dyn->insts[ninst].x64.jmp_cond || target < 0 || ninst + 1 >= dyn->size) return;

    rvv_config_t taken = dyn->insts[target].rvv_entry;
    rvv_config_t fallthrough = dyn->insts[ninst + 1].rvv_entry;
    if (!taken.vl || taken.bits != fallthrough.bits || dyn->rvv.bits == taken.bits) return;

    int scratch = rvv_branch_scratch(dyn, ninst, condition_reg);
    rvv_set_config(dyn, ninst, scratch, taken.sew, taken.lmul, taken.vl);
}

int rvv_set_vl_for_width(dynarec_rv64_t* dyn, int ninst, int s1, int sew, int lmul, unsigned bytes)
{
    int fixed = sew != VECTOR_SEWANY;
    if (sew == VECTOR_SEWANY) sew = dyn->rvv.vl && dyn->rvv.lmul == lmul && (dyn->rvv.vl << dyn->rvv.sew) == bytes ? dyn->rvv.sew : VECTOR_SEW8;
    if (fixed) rvv_note_requirement(dyn, ninst, sew, lmul, bytes >> sew);
    return rvv_set_config(dyn, ninst, s1, sew, lmul, bytes >> sew);
}

void rvv_mask(dynarec_rv64_t* dyn, int ninst, int vreg, uint64_t imm, int s1)
{
    rvv_config_t saved = dyn->rvv;
    assert(saved.vl);
    unsigned bits = saved.vl < 64 ? saved.vl : 64;
    if (bits < 64) imm &= (1ULL << bits) - 1;
    if (cpuext.xtheadvector) {
        int grouping = saved.lmul < 4 ? saved.lmul : saved.lmul - 8;
        unsigned mlen = 8u << saved.sew;
        mlen = grouping >= 0 ? mlen >> grouping : mlen << -grouping;
        uint64_t low = 0, high = 0;
        for (unsigned i = 0; i < bits && i * mlen < 128; ++i) {
            if (!(imm & (1ULL << i))) continue;
            unsigned bit = i * mlen;
            if (bit < 64)
                low |= 1ULL << bit;
            else
                high |= 1ULL << (bit - 64);
        }
        rvv_set_config(dyn, ninst, s1, VECTOR_SEW64, VECTOR_LMUL1, 2);
        int temp = vpu_get_scratch(dyn);
        MOV64x(s1, high);
        VMV_S_X(temp, s1);
        MOV64x(s1, low);
        VSLIDE1UP_VX(vreg, temp, s1, VECTOR_UNMASKED);
        rvv_set_config(dyn, ninst, s1, saved.sew, saved.lmul, saved.vl);
    } else {
        int wrap = bits > (8u << saved.sew);
        if (wrap) rvv_set_config(dyn, ninst, s1, VECTOR_SEW64, VECTOR_LMUL1, 1);
        MOV64x(s1, imm);
        VMV_S_X(vreg, s1);
        if (wrap) rvv_set_config(dyn, ninst, s1, saved.sew, saved.lmul, saved.vl);
    }
}
