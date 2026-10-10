// SPDX-License-Identifier: MIT
#include <stddef.h>
#include <stdio.h>
#include <signal.h>
#include <ucontext.h>
#include <string.h>

#include "debug.h"
#include "dynablock.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "dynarec/dynablock_private.h"
#include "dynarec_rv64_arch.h"
#include "dynarec_rv64_functions.h"
#include "dynarec_rv64_private.h"

//order might be important, so define SUPER for the right one
#define SUPER() \
    GO(flags)   \
    GO(x87)     \
    GO(mmx)     \
    GO(sse)     \
    GO(ymm)     \


typedef struct arch_flags_s
{
    uint8_t ignore:1;
} arch_flags_t;

#define X87_ST_D 0
#define X87_ST_F 1
#define X87_ST_I64 2

typedef struct arch_x87_s
{
    int8_t delta;       // pending x87 stack count at that instruction
    uint8_t x87;        // 1bit is STx present
    uint8_t reg[8];     // FP register of each present STx
    uint8_t type[8];    // type of each present STx
} arch_x87_t;

typedef struct arch_mmx_s
{
    uint8_t mmx;    //1bit for each mmx reg present
    uint8_t vector; // 1bit for each mmx reg cached in a vector register
    uint8_t reg[8]; // register of each present mmx reg
} arch_mmx_t;

typedef struct arch_sse_s
{
    uint16_t sse;       // 1bit for each sse reg present
    uint16_t vector;    // 1bit for each sse reg cached in a vector register
    uint8_t reg[16];    // register of each present sse reg
    uint8_t single[16]; // 1 if the sse value is single precision
} arch_sse_t;

typedef struct arch_ymm_s
{
    uint16_t ymm0;      // 1bit for ymm0
    uint16_t ymm;       // 1bit for each ymm present
    uint64_t ymm_pos;   // 4bits for position of each ymm present
} arch_ymm_t;

typedef struct arch_arch_s
{
    #define GO(A) uint16_t A:1;
    SUPER()
    #undef GO
    uint16_t unaligned:1;
    uint16_t seq:10;    // how many instruction on the same values
    int16_t rsp;        // pending rsp offset at the end of this instruction
    uint16_t up32;      // GPRs with pending 32-bit zero-up at the end of this instruction
} arch_arch_t;

typedef struct arch_build_s
{
    #define GO(A) uint8_t A:1;
    SUPER()
    #undef GO
    uint8_t unaligned;
    int16_t rsp;
    uint16_t up32;
#define GO(A) arch_##A##_t A##_;
    SUPER()
    #undef GO
} arch_build_t;

static int arch_build(dynarec_rv64_t* dyn, int ninst, arch_build_t* arch)
{
    memset(arch, 0, sizeof(arch_build_t));
    // opcode can handle unaligned
    arch->unaligned = dyn->insts[ninst].unaligned;
    arch->rsp = dyn->insts[ninst].rsp_entry;
    arch->up32 = dyn->insts[ninst].up32_pending;
    extcache_t* e = &dyn->insts[ninst].e;
    for (int i = 0; i < 32; ++i) {
        if (!e->extcache[i].t)
            continue;
        int reg = EXTREG(i);
        switch (e->extcache[i].t) {
            case EXT_CACHE_ST_D:
            case EXT_CACHE_ST_F:
            case EXT_CACHE_ST_I64: {
                int st = e->extcache[i].n & 7;
                arch->x87_.x87 |= (uint8_t)(1 << st);
                arch->x87_.reg[st] = (uint8_t)reg;
                arch->x87_.type[st] = (uint8_t)(e->extcache[i].t - EXT_CACHE_ST_D);
                break;
            }
            case EXT_CACHE_MM: {
                int mm = e->extcache[i].n & 7;
                arch->mmx_.mmx |= (uint8_t)(1 << mm);
                arch->mmx_.reg[mm] = (uint8_t)reg;
                break;
            }
            case EXT_CACHE_MMV: {
                int mm = e->extcache[i].n & 7;
                arch->mmx_.mmx |= (uint8_t)(1 << mm);
                arch->mmx_.vector |= (uint8_t)(1 << mm);
                arch->mmx_.reg[mm] = (uint8_t)reg;
                break;
            }
            case EXT_CACHE_SS:
            case EXT_CACHE_SD:
            case EXT_CACHE_XMMR:
            case EXT_CACHE_XMMW: {
                int xmm = e->extcache[i].n & 15;
                arch->sse_.sse |= (uint16_t)(1 << xmm);
                if (e->extcache[i].t == EXT_CACHE_XMMR || e->extcache[i].t == EXT_CACHE_XMMW)
                    arch->sse_.vector |= (uint16_t)(1 << xmm);
                arch->sse_.reg[xmm] = (uint8_t)reg;
                arch->sse_.single[xmm] = (e->extcache[i].t == EXT_CACHE_SS) ? 1 : 0;
                break;
            }
            default:
                break;
        }
    }
    // pending x87 stack count
    if (e->x87stack)
        arch->x87_.delta = e->x87stack;
    if (arch->x87_.x87 || arch->x87_.delta)
        arch->x87 = 1;
    if (arch->mmx_.mmx)
        arch->mmx = 1;
    if (arch->sse_.sse)
        arch->sse = 1;
    return arch->flags + arch->x87 + arch->mmx + arch->sse + arch->ymm + arch->unaligned;
}

size_t get_size_arch(dynarec_rv64_t* dyn)
{
    arch_build_t build = {0};
    arch_build_t previous = {0};
    size_t sz = 0;
    int seq = 0;
    int nseq = 0;
    int last = 0;
    if(!dyn->size) return 0;
    for(int i=0; i<dyn->size; ++i) {
        last = arch_build(dyn, i, &build);
        if((!memcmp(&build, &previous, sizeof(arch_build_t))) && (seq<((1<<10)-1)) && i) {
            // same sequence, increment
            ++seq;
        } else {
            seq = 0;
            ++nseq;
            memcpy(&previous, &build, sizeof(arch_build_t));
            sz+=sizeof(arch_arch_t);
            #define GO(A) if(build.A) sz+=sizeof(arch_##A##_t);
            SUPER()
            #undef GO
        }
    }
    if(nseq==1 && !last)
        return 0;   //empty, no flags, no nothing
    return sz;
}

static void build_next(arch_arch_t* arch, arch_build_t* build)
{
    #define GO(A) arch->A = build->A;
    SUPER()
    #undef GO
    arch->unaligned = build->unaligned;
    arch->rsp = build->rsp;
    arch->up32 = build->up32;
    arch->seq = 0;
    void* p = ((void*)arch)+sizeof(arch_arch_t);
    #define GO(A)                                           \
    if(arch->A) {                                           \
        memcpy(p, &build->A##_, sizeof(arch_ ##A##_t));     \
        p+=sizeof(arch_##A##_t);                            \
    }
    SUPER()
    #undef GO
}

static int sizeof_arch(arch_arch_t* arch)
{
    int sz = sizeof(arch_arch_t);
    #define GO(A)   if(arch->A) sz+=sizeof(arch_##A##_t);
    SUPER()
    #undef GO
    return sz;
}


static void read_vector_reg(const void* vframe, size_t vlenb, int reg, void* buf)
{
    if (vframe) {
        memcpy(buf, (const uint8_t*)vframe + (reg & 31) * vlenb, 16);
        return;
    }

    // HACK: if vframe is absent, the live register is read instead.
    switch (reg & 31) {
#define VREG_CASE(N)                                                                                                                                        \
    case N:                                                                                                                                                 \
        __asm__ volatile(".option push\n\t.option arch, +v\n\taddi t1, zero, 16\n\tvsetvli t0, t1, 0\n\tvse8.v v" #N ", (%0)\n\t.option pop\n\t" ::"r"(buf) \
            : "memory", "t0", "t1");                                                                                                                        \
        break;
        VREG_CASE(0)
        VREG_CASE(1)
        VREG_CASE(2)
        VREG_CASE(3)
        VREG_CASE(4)
        VREG_CASE(5)
        VREG_CASE(6)
        VREG_CASE(7)
        VREG_CASE(8)
        VREG_CASE(9)
        VREG_CASE(10)
        VREG_CASE(11)
        VREG_CASE(12)
        VREG_CASE(13)
        VREG_CASE(14)
        VREG_CASE(15)
        VREG_CASE(16)
        VREG_CASE(17)
        VREG_CASE(18)
        VREG_CASE(19)
        VREG_CASE(20)
        VREG_CASE(21)
        VREG_CASE(22)
        VREG_CASE(23)
        VREG_CASE(24)
        VREG_CASE(25)
        VREG_CASE(26)
        VREG_CASE(27)
        VREG_CASE(28)
        VREG_CASE(29)
        VREG_CASE(30)
        VREG_CASE(31)
#undef VREG_CASE
    }
}

static const void* get_vframe(const ucontext_t* p, size_t* vlenb)
{
    // struct __riscv_extra_ext_header { uint32_t padding[129]; uint32_t reserved; struct { uint32_t magic, size; } hdr; }
    const uint8_t* fp = (const uint8_t*)&p->uc_mcontext.__fpregs;
    if (*(const uint32_t*)(fp + 520) != 0x53465457) // RISCV_V_MAGIC
        return NULL;
    struct riscv_v_ext_state_s {
        unsigned long vstart, vl, vtype, vcsr, vlenb;
        void* datap;
    };
    const struct riscv_v_ext_state_s* vs = (const struct riscv_v_ext_state_s*)(fp + 528);
    if (!vs->datap || vs->vlenb < 16 || vs->vlenb > 4096)
        return NULL;
    *vlenb = vs->vlenb;
    return vs->datap;
}

void* populate_arch(dynarec_rv64_t* dyn, void* p, size_t sz)
{
    arch_build_t build = {0};
    arch_build_t previous = {0};
    arch_arch_t* arch = p;
    arch_arch_t* next = p;
    int seq = 0;
    for(int i=0; i<dyn->size; ++i) {
        arch_build(dyn, i, &build);
        if((!memcmp(&build, &previous, sizeof(arch_build_t))) && (seq<((1<<10)-1)) && i) {
            // same sequence, increment
            seq++;
            arch->seq = seq;
        } else {
            arch = next;
            build_next(arch, &build);
            seq = 0;
            memcpy(&previous, &build, sizeof(arch_build_t));
            int sz = sizeof_arch(arch);
            next = (arch_arch_t*)((uintptr_t)arch+sz);
        }
    }
    return p;
}

void adjust_arch(dynablock_t* db, x64emu_t* emu, ucontext_t* p, uintptr_t x64pc)
{
    if(!db->arch_size || !db->arch)
        return;
    int ninst = getX64AddressInst(db, x64pc);
    dynarec_log(LOG_INFO, "adjust_arch(...), db=%p, x64pc=%p, nints=%d", db, (void*)x64pc, ninst);
    if(ninst<0) {
    dynarec_log(LOG_INFO, "\n");
        return;
    }
    if(ninst==0) {
    dynarec_log(LOG_INFO, "\n");
        CHECK_FLAGS(emu);
        return;
    }
    // look for state at ninst-1
    arch_arch_t* arch = db->arch;
    arch_arch_t* next = arch;
    #define GO(A) arch_##A##_t* A = NULL;
    SUPER()
    #undef GO
    int i = 0;
    while(i<ninst-1) {
        arch = next;
        i += 1+arch->seq;
        dynarec_log(LOG_INFO, "[ seq=%d%s%s%s%s%s ] ", arch->seq, arch->flags?" Flags":"", arch->x87?" x87":"", arch->mmx?" MMX":"", arch->sse?" SSE":"", arch->ymm?" YMM":"");
        next = (arch_arch_t*)((uintptr_t)next + sizeof_arch(arch));
    }
    int sz = sizeof(arch_arch_t);
    #define GO(A)                                   \
    if(arch->A) {                                   \
        A = (arch_##A##_t*)((uintptr_t)arch + sz);  \
        sz+=sizeof(arch_##A##_t);                   \
    }
    SUPER()
    #undef GO
    uint16_t up32 = arch->up32;
    while (up32) {
        int r = __builtin_ctz(up32);
        up32 &= up32 - 1;
        emu->regs[r].q[0] &= 0xffffffffULL;
    }
    if (arch->rsp) {
        dynarec_log(LOG_INFO, " rsp[%d] ", arch->rsp);
        emu->regs[_SP].q[0] += arch->rsp;
    }
    if (sse || mmx) {
        size_t vlenb = 0;
        const void* vframe = get_vframe(p, &vlenb);
        if (sse) {
            dynarec_log(LOG_INFO, " sse[%x, vector=%x, %s] ", sse->sse, sse->vector, vframe ? "frame" : "live");
            for (int i = 0; i < 16; ++i) {
                if (!((sse->sse >> i) & 1))
                    continue;
                if ((sse->vector >> i) & 1) {
                    uint8_t vbuf[16];
                    read_vector_reg(vframe, vlenb, sse->reg[i], vbuf);
                    memcpy(&emu->xmm[i], vbuf, 16);
                } else if (sse->single[i]) {
                    uint64_t raw = p->uc_mcontext.__fpregs.__d.__f[sse->reg[i] & 31];
                    *(uint32_t*)&emu->xmm[i] = (uint32_t)raw;
                } else {
                    emu->xmm[i].q[0] = p->uc_mcontext.__fpregs.__d.__f[sse->reg[i] & 31];
                }
            }
        }
        if (mmx) {
            dynarec_log(LOG_INFO, " mmx[%x, vector=%x] ", mmx->mmx, mmx->vector);
            for (int i = 0; i < 8; ++i) {
                if (!((mmx->mmx >> i) & 1))
                    continue;
                if ((mmx->vector >> i) & 1) {
                    uint8_t vbuf[16];
                    read_vector_reg(vframe, vlenb, mmx->reg[i], vbuf);
                    emu->mmx[i].q = *(uint64_t*)vbuf;
                } else {
                    emu->mmx[i].q = p->uc_mcontext.__fpregs.__d.__f[mmx->reg[i] & 31];
                }
            }
        }
    }
    if (x87) {
        dynarec_log(LOG_INFO, " x87[%x, delta=%d] ", x87->x87, x87->delta);
        if (x87->delta)
            emu->top = (emu->top - x87->delta) & 7;
        for (int i = 0; i < 8; ++i) {
            if (!((x87->x87 >> i) & 1))
                continue;
            uint64_t raw = p->uc_mcontext.__fpregs.__d.__f[x87->reg[i] & 31];
            switch (x87->type[i]) {
                case X87_ST_F:
                    emu->x87[(emu->top + i) & 7].d = *(float*)&raw;
                    break;
                case X87_ST_I64:
                    emu->x87[(emu->top + i) & 7].d = (double)*(int64_t*)&raw;
                    break;
                default:
                    emu->x87[(emu->top + i) & 7].d = *(double*)&raw;
                    break;
            }
        }
    }
    dynarec_log(LOG_INFO, "\n");
}

int arch_unaligned(dynablock_t* db, uintptr_t x64pc)
{
    if(!db)
        return 0;
    if(!db->arch_size || !db->arch)
        return 0;
    int ninst = getX64AddressInst(db, x64pc);
    if(ninst<0) {
        return 0;
    }
    // look for state at ninst
    arch_arch_t* arch = db->arch;
    arch_arch_t* next = arch;
    int i = -1;
    while(i<ninst) {
        arch = next;
        i += 1+arch->seq;
        next = (arch_arch_t*)((uintptr_t)next + sizeof_arch(arch));
    }
    return arch->unaligned;
}
