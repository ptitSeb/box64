#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "emu/x87emu_private.h"
#include "x64trace.h"
#include "signals.h"
#include "dynarec_native.h"
#include "dynarec_ppc64le_private.h"
#include "dynarec_ppc64le_functions.h"
#include "custommem.h"
#include "bridge.h"
#include "gdbjit.h"
#include "perfmap.h"
#include "elfloader.h"

// ---- FPU scratch/register management stubs ----
// TODO: Full implementations in a follow-up PR

void fpu_reset_scratch(dynarec_ppc64le_t* dyn)
{
    dyn->v.fpu_scratch = 0;
    dyn->v.ymm_used = 0;
    dyn->v.xmm_used = 0;
}

// Reset fpu regs counter
static void fpu_reset_reg_vmxcache(vmxcache_t* v)
{
    for (int i = 0; i < 24; ++i) {
        v->fpuused[i] = 0;
        v->vmxcache[i].v = 0;
    }
}

void fpu_reset_reg(dynarec_ppc64le_t* dyn)
{
    fpu_reset_reg_vmxcache(&dyn->v);
}

// ---- VMX cache stubs ----

int vmxcache_no_i64(dynarec_ppc64le_t* dyn, int ninst, int st, int a)
{
    // TODO
    (void)dyn; (void)ninst; (void)st;
    return a;
}

int vmxcache_get_st(dynarec_ppc64le_t* dyn, int ninst, int a)
{
    // TODO
    (void)dyn; (void)ninst; (void)a;
    return VMX_CACHE_ST_D;
}

int vmxcache_get_current_st(dynarec_ppc64le_t* dyn, int ninst, int a)
{
    // TODO
    (void)dyn; (void)ninst; (void)a;
    return VMX_CACHE_ST_D;
}

int vmxcache_combine_st(dynarec_ppc64le_t* dyn, int ninst, int a, int b)
{
    // TODO
    (void)dyn; (void)ninst; (void)a; (void)b;
    return VMX_CACHE_ST_D;
}

int fpuCacheNeedsTransform(dynarec_ppc64le_t* dyn, int ninst)
{
    // TODO
    (void)dyn; (void)ninst;
    return 1;
}

void vmxcacheUnwind(vmxcache_t* cache)
{
    if (cache->swapped) {
        // unswap
        int a = -1;
        int b = -1;
        for (int j = 0; j < 24 && ((a == -1) || (b == -1)); ++j)
            if ((cache->vmxcache[j].t == VMX_CACHE_ST_D || cache->vmxcache[j].t == VMX_CACHE_ST_F || cache->vmxcache[j].t == VMX_CACHE_ST_I64)) {
                if (cache->vmxcache[j].n == cache->combined1)
                    a = j;
                else if (cache->vmxcache[j].n == cache->combined2)
                    b = j;
            }
        if (a != -1 && b != -1) {
            int tmp = cache->vmxcache[a].n;
            cache->vmxcache[a].n = cache->vmxcache[b].n;
            cache->vmxcache[b].n = tmp;
        }
        cache->swapped = 0;
        cache->combined1 = cache->combined2 = 0;
    }
    if (cache->news) {
        for (int i = 0; i < 24; ++i)
            if (cache->news & (1 << i))
                cache->vmxcache[i].v = 0;
        cache->news = 0;
    }
    if (cache->stack_push) {
        for (int j = 0; j < 24; ++j) {
            if ((cache->vmxcache[j].t == VMX_CACHE_ST_D || cache->vmxcache[j].t == VMX_CACHE_ST_F || cache->vmxcache[j].t == VMX_CACHE_ST_I64)) {
                if (cache->vmxcache[j].n < cache->stack_push)
                    cache->vmxcache[j].v = 0;
                else
                    cache->vmxcache[j].n -= cache->stack_push;
            }
        }
        cache->x87stack -= cache->stack_push;
        cache->tags >>= (cache->stack_push * 2);
        cache->stack -= cache->stack_push;
        if (cache->pushed >= cache->stack_push)
            cache->pushed -= cache->stack_push;
        else
            cache->pushed = 0;
        cache->stack_push = 0;
    }
    cache->x87stack += cache->stack_pop;
    cache->stack_next = cache->stack;
    if (cache->stack_pop) {
        if (cache->poped >= cache->stack_pop)
            cache->poped -= cache->stack_pop;
        else
            cache->poped = 0;
        cache->tags <<= (cache->stack_pop * 2);
    }
    cache->stack_pop = 0;
    cache->barrier = 0;
    // Rebuild the x87cache info with vmxcache
    cache->mmxcount = 0;
    cache->fpu_scratch = 0;
    for (int i = 0; i < 8; ++i) {
        cache->x87cache[i] = -1;
        cache->mmxcache[i] = -1;
        cache->x87reg[i] = 0;
        cache->ssecache[i * 2].v = -1;
        cache->ssecache[i * 2 + 1].v = -1;
        cache->avxcache[i * 2].v = -1;
        cache->avxcache[i * 2 + 1].v = -1;
    }
    int x87reg = 0;
    for (int i = 0; i < 24; ++i) {
        if (cache->vmxcache[i].v) {
            cache->fpuused[i] = 1;
            switch (cache->vmxcache[i].t) {
                case VMX_CACHE_MM:
                    cache->mmxcache[cache->vmxcache[i].n] = i;
                    ++cache->mmxcount;
                    break;
                case VMX_CACHE_XMMR:
                case VMX_CACHE_XMMW:
                    cache->ssecache[cache->vmxcache[i].n].reg = i;
                    cache->ssecache[cache->vmxcache[i].n].write = (cache->vmxcache[i].t == VMX_CACHE_XMMW) ? 1 : 0;
                    break;
                case VMX_CACHE_YMMR:
                case VMX_CACHE_YMMW:
                    cache->avxcache[cache->vmxcache[i].n].reg = i;
                    cache->avxcache[cache->vmxcache[i].n].write = (cache->vmxcache[i].t == VMX_CACHE_YMMW) ? 1 : 0;
                    break;
                case VMX_CACHE_ST_F:
                case VMX_CACHE_ST_D:
                case VMX_CACHE_ST_I64:
                    cache->x87cache[x87reg] = cache->vmxcache[i].n;
                    cache->x87reg[x87reg] = i;
                    ++x87reg;
                    break;
                case VMX_CACHE_SCR:
                    cache->fpuused[i] = 0;
                    cache->vmxcache[i].v = 0;
                    break;
            }
        } else {
            cache->fpuused[i] = 0;
        }
    }
}

const char* getCacheName(int t, int n)
{
    static char buff[20];
    switch (t) {
        case VMX_CACHE_ST_D: sprintf(buff, "ST%d", n); break;
        case VMX_CACHE_ST_F: sprintf(buff, "st%d", n); break;
        case VMX_CACHE_ST_I64: sprintf(buff, "STi%d", n); break;
        case VMX_CACHE_MM: sprintf(buff, "MM%d", n); break;
        case VMX_CACHE_XMMW: sprintf(buff, "XMM%d", n); break;
        case VMX_CACHE_XMMR: sprintf(buff, "xmm%d", n); break;
        case VMX_CACHE_YMMW: sprintf(buff, "YMM%d", n); break;
        case VMX_CACHE_YMMR: sprintf(buff, "ymm%d", n); break;
        case VMX_CACHE_SCR: sprintf(buff, "Scratch"); break;
        case VMX_CACHE_NONE: buff[0] = '\0'; break;
    }
    return buff;
}

// PPC64LE register mapping for debug output
static register_mapping_t register_mappings[] = {
    { "rax", "r14" },
    { "eax", "r14" },
    { "ax", "r14" },
    { "ah", "r14" },
    { "al", "r14" },
    { "rcx", "r15" },
    { "ecx", "r15" },
    { "cx", "r15" },
    { "ch", "r15" },
    { "cl", "r15" },
    { "rdx", "r16" },
    { "edx", "r16" },
    { "dx", "r16" },
    { "dh", "r16" },
    { "dl", "r16" },
    { "rbx", "r17" },
    { "ebx", "r17" },
    { "bx", "r17" },
    { "bh", "r17" },
    { "bl", "r17" },
    { "rsi", "r20" },
    { "esi", "r20" },
    { "si", "r20" },
    { "sil", "r20" },
    { "rdi", "r21" },
    { "edi", "r21" },
    { "di", "r21" },
    { "dil", "r21" },
    { "rsp", "r18" },
    { "esp", "r18" },
    { "sp", "r18" },
    { "spl", "r18" },
    { "rbp", "r19" },
    { "ebp", "r19" },
    { "bp", "r19" },
    { "bpl", "r19" },
    { "r8", "r22" },
    { "r8d", "r22" },
    { "r8w", "r22" },
    { "r8b", "r22" },
    { "r9", "r23" },
    { "r9d", "r23" },
    { "r9w", "r23" },
    { "r9b", "r23" },
    { "r10", "r24" },
    { "r10d", "r24" },
    { "r10w", "r24" },
    { "r10b", "r24" },
    { "r11", "r25" },
    { "r11d", "r25" },
    { "r11w", "r25" },
    { "r11b", "r25" },
    { "r12", "r26" },
    { "r12d", "r26" },
    { "r12w", "r26" },
    { "r12b", "r26" },
    { "r13", "r27" },
    { "r13d", "r27" },
    { "r13w", "r27" },
    { "r13b", "r27" },
    { "r14", "r28" },
    { "r14d", "r28" },
    { "r14w", "r28" },
    { "r14b", "r28" },
    { "r15", "r29" },
    { "r15d", "r29" },
    { "r15w", "r29" },
    { "r15b", "r29" },
    { "rip", "r9" },
};

// PPC64LE VMX register names for debug output
static const char* Vt[] = { "vr0", "vr1", "vr2", "vr3", "vr4", "vr5", "vr6", "vr7", "vr8", "vr9", "vr10", "vr11", "vr12", "vr13", "vr14", "vr15", "vr16", "vr17", "vr18", "vr19", "vr20", "vr21", "vr22", "vr23", "vr24", "vr25", "vr26", "vr27", "vr28", "vr29", "vr30", "vr31" };

static const char* df_status[] = { "unknown", "set", "none_pending", "none" };

void printf_x64_instruction(dynarec_native_t* dyn, zydis_dec_t* dec, instruction_x64_t* inst, const char* name);
void inst_name_pass3(dynarec_native_t* dyn, int ninst, const char* name, rex_t rex)
{
    if (!dyn->need_dump && !BOX64ENV(dynarec_gdbjit) && !BOX64ENV(dynarec_perf_map)) return;

    static char buf[4096];
    int length = sprintf(buf, "barrier=%d state=%d/%s(%s->%s), set=%X/%X, use=%X, need=%X/%X, fuse=%d/%d, sm=%d(%d/%d)",
        dyn->insts[ninst].x64.barrier,
        dyn->insts[ninst].x64.state_flags,
        df_status[dyn->f],
        df_status[dyn->insts[ninst].f_entry],
        df_status[dyn->insts[ninst].f_exit],
        dyn->insts[ninst].x64.set_flags,
        dyn->insts[ninst].x64.gen_flags,
        dyn->insts[ninst].x64.use_flags,
        dyn->insts[ninst].x64.need_before,
        dyn->insts[ninst].x64.need_after,
        dyn->insts[ninst].nat_flags_fusion,
        dyn->insts[ninst].no_scratch_usage,
        dyn->smwrite, dyn->insts[ninst].will_write, dyn->insts[ninst].last_write);
    if (dyn->insts[ninst].pred_sz) {
        length += sprintf(buf + length, ", pred=");
        for (int ii = 0; ii < dyn->insts[ninst].pred_sz; ++ii)
            length += sprintf(buf + length, "%s%d", ii ? "/" : "", dyn->insts[ninst].pred[ii]);
    }
    if (dyn->insts[ninst].x64.jmp && dyn->insts[ninst].x64.jmp_insts >= 0)
        length += sprintf(buf + length, ", jmp=%d", dyn->insts[ninst].x64.jmp_insts);
    if (dyn->insts[ninst].x64.jmp && dyn->insts[ninst].x64.jmp_insts == -1)
        length += sprintf(buf + length, ", jmp=out");
    if (dyn->last_ip)
        length += sprintf(buf + length, ", last_ip=%p", (void*)dyn->last_ip);
    for (int ii = 0; ii < 24; ++ii) {
        switch (dyn->insts[ninst].v.vmxcache[ii].t) {
            case VMX_CACHE_ST_D: length += sprintf(buf + length, " V%d:%s", ii, getCacheName(dyn->insts[ninst].v.vmxcache[ii].t, dyn->insts[ninst].v.vmxcache[ii].n)); break;
            case VMX_CACHE_ST_F: length += sprintf(buf + length, " V%d:%s", ii, getCacheName(dyn->insts[ninst].v.vmxcache[ii].t, dyn->insts[ninst].v.vmxcache[ii].n)); break;
            case VMX_CACHE_ST_I64: length += sprintf(buf + length, " V%d:%s", ii, getCacheName(dyn->insts[ninst].v.vmxcache[ii].t, dyn->insts[ninst].v.vmxcache[ii].n)); break;
            case VMX_CACHE_MM: length += sprintf(buf + length, " %s:%s", Vt[ii], getCacheName(dyn->insts[ninst].v.vmxcache[ii].t, dyn->insts[ninst].v.vmxcache[ii].n)); break;
            case VMX_CACHE_XMMW: length += sprintf(buf + length, " %s:%s", Vt[ii], getCacheName(dyn->insts[ninst].v.vmxcache[ii].t, dyn->insts[ninst].v.vmxcache[ii].n)); break;
            case VMX_CACHE_XMMR: length += sprintf(buf + length, " %s:%s", Vt[ii], getCacheName(dyn->insts[ninst].v.vmxcache[ii].t, dyn->insts[ninst].v.vmxcache[ii].n)); break;
            case VMX_CACHE_YMMW: length += sprintf(buf + length, " %s:%s%s", Vt[ii], getCacheName(dyn->insts[ninst].v.vmxcache[ii].t, dyn->insts[ninst].v.vmxcache[ii].n), dyn->insts[ninst].v.avxcache[dyn->insts[ninst].v.vmxcache[ii].n].zero_upper == 1 ? "-UZ" : ""); break;
            case VMX_CACHE_YMMR: length += sprintf(buf + length, " %s:%s%s", Vt[ii], getCacheName(dyn->insts[ninst].v.vmxcache[ii].t, dyn->insts[ninst].v.vmxcache[ii].n), dyn->insts[ninst].v.avxcache[dyn->insts[ninst].v.vmxcache[ii].n].zero_upper == 1 ? "-UZ" : ""); break;
            case VMX_CACHE_SCR: length += sprintf(buf + length, " %s:%s", Vt[ii], getCacheName(dyn->insts[ninst].v.vmxcache[ii].t, dyn->insts[ninst].v.vmxcache[ii].n)); break;
            case VMX_CACHE_NONE:
            default: break;
        }
    }
    if (dyn->v.stack || dyn->insts[ninst].v.stack_next || dyn->insts[ninst].v.x87stack)
        length += sprintf(buf + length, " X87:%d/%d(+%d/-%d)%d", dyn->v.stack, dyn->insts[ninst].v.stack_next, dyn->insts[ninst].v.stack_push, dyn->insts[ninst].v.stack_pop, dyn->insts[ninst].v.x87stack);
    if (dyn->insts[ninst].v.combined1 || dyn->insts[ninst].v.combined2)
        length += sprintf(buf + length, " %s:%d/%d", dyn->insts[ninst].v.swapped ? "SWP" : "CMB", dyn->insts[ninst].v.combined1, dyn->insts[ninst].v.combined2);

    if (dyn->need_dump) {
        printf_x64_instruction(dyn, rex.is32bits ? my_context->dec32 : my_context->dec, &dyn->insts[ninst].x64, name);
        dynarec_log(LOG_NONE, "%s%p: %d emitted opcodes, inst=%d, %s%s\n",
            (dyn->need_dump > 1) ? "\e[32m" : "",
            (void*)(dyn->native_start + dyn->insts[ninst].address), dyn->insts[ninst].size / 4, ninst, buf, (dyn->need_dump > 1) ? "\e[m" : "");
    }
    if (BOX64ENV(dynarec_gdbjit)) {
        static char buf2[512];
        if (BOX64ENV(dynarec_gdbjit) > 1) {
            snprintf(buf2, sizeof(buf2), "; %d: %d opcodes, %s", ninst, dyn->insts[ninst].size / 4, buf);
            dyn->gdbjit_block = GdbJITBlockAddLine(dyn->gdbjit_block, (dyn->native_start + dyn->insts[ninst].address), buf2);
        }
        zydis_dec_t* dec = rex.is32bits ? my_context->dec32 : my_context->dec;
        const char* inst_name = name;
        if (dec) {
            inst_name = DecodeX64Trace(dec, dyn->insts[ninst].x64.addr, 0);
            x64disas_add_register_mapping_annotations(buf2, inst_name, register_mappings, sizeof(register_mappings) / sizeof(register_mappings[0]));
            inst_name = buf2;
        }
        dyn->gdbjit_block = GdbJITBlockAddLine(dyn->gdbjit_block, (dyn->native_start + dyn->insts[ninst].address), inst_name);
    }
    if (BOX64ENV(dynarec_perf_map) && BOX64ENV(dynarec_perf_map_fd) != -1) {
        writePerfMap(dyn->insts[ninst].x64.addr, dyn->native_start + dyn->insts[ninst].address, dyn->insts[ninst].size / 4, name);
    }
    if (length > (int)sizeof(buf)) printf_log(LOG_NONE, "Warning: buf too small in inst_name_pass3 (%d vs %zd)\n", length, sizeof(buf));
}

void print_opcode(dynarec_native_t* dyn, int ninst, uint32_t opcode)
{
    dynarec_log_prefix(0, LOG_NONE, "\t%08x\t%s\n", opcode, ppc64le_print(opcode, (uintptr_t)dyn->block));
}

// ---- FPU reset functions ----

static void x87_reset(vmxcache_t* v)
{
    for (int i = 0; i < 8; ++i)
        v->x87cache[i] = -1;
    v->tags = 0;
    v->x87stack = 0;
    v->stack = 0;
    v->stack_next = 0;
    v->stack_pop = 0;
    v->stack_push = 0;
    v->combined1 = v->combined2 = 0;
    v->swapped = 0;
    v->barrier = 0;
    v->pushed = 0;
    v->poped = 0;
    for (int i = 0; i < 24; ++i)
        if (v->vmxcache[i].t == VMX_CACHE_ST_F
            || v->vmxcache[i].t == VMX_CACHE_ST_D
            || v->vmxcache[i].t == VMX_CACHE_ST_I64)
            v->vmxcache[i].v = 0;
}

static void mmx_reset(vmxcache_t* v)
{
    v->mmxcount = 0;
    for (int i = 0; i < 8; ++i)
        v->mmxcache[i] = -1;
}

static void sse_reset(vmxcache_t* v)
{
    for (int i = 0; i < 16; ++i)
        v->ssecache[i].v = -1;
}

static void avx_reset(vmxcache_t* v)
{
    for (int i = 0; i < 16; ++i)
        v->avxcache[i].v = -1;
}

void fpu_reset(dynarec_ppc64le_t* dyn)
{
    x87_reset(&dyn->v);
    mmx_reset(&dyn->v);
    sse_reset(&dyn->v);
    avx_reset(&dyn->v);
    fpu_reset_reg(dyn);
}

void fpu_reset_ninst(dynarec_ppc64le_t* dyn, int ninst)
{
    sse_reset(&dyn->insts[ninst].v);
    avx_reset(&dyn->insts[ninst].v);
    fpu_reset_reg_vmxcache(&dyn->insts[ninst].v);
}

void fpu_save_and_unwind(dynarec_ppc64le_t* dyn, int ninst, vmxcache_t* cache)
{
    memcpy(cache, &dyn->insts[ninst].v, sizeof(vmxcache_t));
    vmxcacheUnwind(&dyn->insts[ninst].v);
}

void fpu_unwind_restore(dynarec_ppc64le_t* dyn, int ninst, vmxcache_t* cache)
{
    memcpy(&dyn->insts[ninst].v, cache, sizeof(vmxcache_t));
}

// ---- Native flags and scratch ----

void updateNativeFlags(dynarec_ppc64le_t* dyn)
{
    if (!BOX64ENV(dynarec_nativeflags))
        return;
    for (int i = 1; i < dyn->size; ++i)
        if (dyn->insts[i].nat_flags_fusion) {
            int j = i - 1;
            int found = 0;
            if (dyn->insts[i].pred_sz == 1 && dyn->insts[i].pred[0] == j) {
                while (j >= 0) {
                    if (dyn->insts[j].x64.set_flags && (dyn->insts[i].x64.use_flags & dyn->insts[j].x64.set_flags) == dyn->insts[i].x64.use_flags) {
                        dyn->insts[j].nat_flags_fusion = 1;
                        if (dyn->insts[i].x64.use_flags & X_SF) {
                            dyn->insts[j].nat_flags_needsign = 1;
                        }
                        dyn->insts[i].x64.use_flags = 0;
                        dyn->insts[j].nat_next_inst = i;
                        found = 1;
                        break;
                    } else if (j && dyn->insts[j].pred_sz == 1 && dyn->insts[j].pred[0] == j - 1
                        && dyn->insts[j].no_scratch_usage && !dyn->insts[j].x64.set_flags && !dyn->insts[j].x64.use_flags) {
                        j -= 1;
                    } else
                        break;
                }
            }
            if (!found) dyn->insts[i].nat_flags_fusion = 0;
        }
}

void get_free_scratch(dynarec_ppc64le_t* dyn, int ninst, uint8_t* tmp1, uint8_t* tmp2, uint8_t* tmp3, uint8_t s1, uint8_t s2, uint8_t s3, uint8_t s4, uint8_t s5)
{
    uint8_t n1 = dyn->insts[ninst].nat_flags_op1;
    uint8_t n2 = dyn->insts[ninst].nat_flags_op2;
    uint8_t tmp[5] = { 0 };
    int idx = 0;
#define GO(s) \
    if ((s != n1) && (s != n2)) tmp[idx++] = s
    GO(s1);
    GO(s2);
    GO(s3);
    GO(s4);
    GO(s5);
#undef GO
    *tmp1 = tmp[0];
    *tmp2 = tmp[1];
    *tmp3 = tmp[2];
}

// ---- Stubs for pass0 analysis functions ----

void propagateFpuBarrier(dynarec_ppc64le_t* dyn)
{
    // TODO
    (void)dyn;
}

// ---- PPC64LE fast hash for block change detection ----

uint32_t ppc64le_fast_hash(void* p, uint32_t len)
{
    const uint8_t* data = (const uint8_t*)p;
    uint64_t h = len * UINT64_C(0x9E3779B97F4A7C15);

    while (len >= 8) {
        uint64_t k;
        __builtin_memcpy(&k, data, 8);
        k *= UINT64_C(0xBF58476D1CE4E5B9);
        k ^= k >> 31;
        h ^= k;
        h *= UINT64_C(0x94D049BB133111EB);
        data += 8;
        len -= 8;
    }

    if (len) {
        uint64_t k = 0;
        switch(len) {
            case 7: k |= (uint64_t)data[6] << 48; /* fall through */
            case 6: k |= (uint64_t)data[5] << 40; /* fall through */
            case 5: k |= (uint64_t)data[4] << 32; /* fall through */
            case 4: k |= (uint64_t)data[3] << 24; /* fall through */
            case 3: k |= (uint64_t)data[2] << 16; /* fall through */
            case 2: k |= (uint64_t)data[1] <<  8; /* fall through */
            case 1: k |= (uint64_t)data[0];
        }
        k *= UINT64_C(0xBF58476D1CE4E5B9);
        k ^= k >> 31;
        h ^= k;
        h *= UINT64_C(0x94D049BB133111EB);
    }

    h ^= h >> 33;
    h *= UINT64_C(0xFF51AFD7ED558CCD);
    h ^= h >> 33;

    return (uint32_t)h;
}
