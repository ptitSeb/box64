#include <stddef.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include "debug.h"
#include "dynablock.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "dynarec/dynablock_private.h"
#include "dynarec_arm64_arch.h"
#include "dynarec_arm64_functions.h"
#include "dynarec_native.h"

//order might be important, so define SUPER for the right one
#define SUPER() \
    GO(flags)   \
    GO(x87)     \
    GO(mmx)     \
    GO(sse)     \
    GO(ymm)     \


typedef struct arch_flags_s
{
    uint8_t defered:1;
    uint8_t nf:1;
    uint8_t eq:1;
    uint8_t vf:1;
    uint8_t cf:1;
    uint8_t inv_cf:1;
} arch_flags_t;

#define X87_ST_D 0
#define X87_ST_F 1
#define X87_ST_I64 2
typedef struct arch_x87_s
{
    int8_t delta;        //up to +/-7
    uint8_t x87;        // 1bit is STx present
    uint16_t x87_type;  // 2bits per STx type
    uint32_t x87_pos;   //4bits per STx position (well, 3 would be enough)
} arch_x87_t;

typedef struct arch_mmx_s
{
    uint8_t mmx;    //1bit for each mmx reg present
} arch_mmx_t;

typedef struct arch_sse_s
{
    uint16_t sse;   //1bit for each sse reg present
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
} arch_arch_t;

typedef struct arch_build_s
{
    #define GO(A) uint8_t A:1;
    SUPER()
    #undef GO
    uint8_t unaligned;
    #define GO(A) arch_##A##_t A##_;
    SUPER()
    #undef GO
} arch_build_t;

static arch_build_t static_build[MAX_INSTS+2] = {0};

static int arch_build(dynarec_arm_t* dyn, int ninst, arch_build_t* arch, int noarch)
{
    memset(arch, 0, sizeof(arch_build_t));
    // flags
    if(((dyn->insts[ninst].f_entry!=status_none && dyn->insts[ninst].f_entry!=status_none_pending)) || dyn->insts[ninst].need_nat_flags || dyn->insts[ninst].before_nat_flags) {
        arch->flags = 1;
        arch->flags_.defered = (dyn->insts[ninst].f_entry!=status_none && dyn->insts[ninst].f_entry!=status_none_pending);
        uint8_t flags = dyn->insts[ninst].need_nat_flags | dyn->insts[ninst].before_nat_flags;
        arch->flags_.vf = flags&NF_VF;
        arch->flags_.nf = flags&NF_SF;
        arch->flags_.eq = flags&NF_EQ;
        arch->flags_.cf = flags&NF_CF;
        if(arch->flags_.cf)
            arch->flags_.inv_cf = !dyn->insts[ninst].normal_carry;
    }
    // opcode can handle unaligned
    arch->unaligned = dyn->insts[ninst].unaligned;
    if(!noarch) {
        // got through all naoncache to gather regs assignments
        int idx;
        for(int i=0; i<32; ++i)
            if(dyn->insts[ninst].n.neoncache[i].v)
                switch(dyn->insts[ninst].n.neoncache[i].t) {
                    case NEON_CACHE_XMMW:
                        arch->sse = 1;
                        arch->sse_.sse |= 1<<dyn->insts[ninst].n.neoncache[i].n;
                        break;
                    case NEON_CACHE_MM:
                        arch->mmx = 1;
                        arch->mmx_.mmx |= 1<<dyn->insts[ninst].n.neoncache[i].n;
                        break;
                    case NEON_CACHE_YMMW:
                        arch->ymm = 1;
                        arch->ymm_.ymm |= 1<<dyn->insts[ninst].n.neoncache[i].n;
                        idx = i;
                        if(idx>=EMM0 && idx<=EMM0+8)
                            idx-=EMM0;
                        else
                            idx-=SCRATCH0-8;
                        arch->ymm_.ymm_pos |= idx<<(dyn->insts[ninst].n.neoncache[i].n*4);
                        break;
                    case NEON_CACHE_ST_D:
                        arch->x87 = 1;
                        arch->x87_.x87 |= 1<<dyn->insts[ninst].n.neoncache[i].n;
                        arch->x87_.x87_pos = (i-EMM0)<<(dyn->insts[ninst].n.neoncache[i].n*4);
                        arch->x87_.x87_type = (X87_ST_D)<<(dyn->insts[ninst].n.neoncache[i].n*2);
                        break;
                    case NEON_CACHE_ST_F:
                        arch->x87 = 1;
                        arch->x87_.x87 |= 1<<dyn->insts[ninst].n.neoncache[i].n;
                        arch->x87_.x87_pos = (i-EMM0)<<(dyn->insts[ninst].n.neoncache[i].n*4);
                        arch->x87_.x87_type = (X87_ST_F)<<(dyn->insts[ninst].n.neoncache[i].n*2);
                        break;
                    case NEON_CACHE_ST_I64:
                        arch->x87 = 1;
                        arch->x87_.x87 |= 1<<dyn->insts[ninst].n.neoncache[i].n;
                        arch->x87_.x87_pos = (i-EMM0)<<(dyn->insts[ninst].n.neoncache[i].n*4);
                        arch->x87_.x87_type = (X87_ST_I64)<<(dyn->insts[ninst].n.neoncache[i].n*2);
                        break;
                    case NEON_CACHE_XMMR:
                    case NEON_CACHE_YMMR:
                    default:
                        // doing nothing, it's just a value read in memory
                        break;
                }
        // ymm0
        if(dyn->insts[ninst].ymm0_out) {
            arch->ymm = 1;
            arch->ymm_.ymm0 = dyn->insts[ninst].ymm0_out;
        }
        // x87 top
        if(dyn->insts[ninst].n.x87stack) {
            arch->x87 = 1;
            arch->x87_.delta = dyn->insts[ninst].n.x87stack;
        }
    }
    return arch->flags + arch->x87 + arch->mmx + arch->sse + arch->ymm + arch->unaligned;
}

static int sizeof_arch(arch_arch_t* arch)
{
    int sz = sizeof(arch_arch_t);
    #define GO(A)   if(arch->A) sz+=sizeof(arch_##A##_t);
    SUPER()
    #undef GO
    return sz;
}

static int sizeof_arch_build(arch_build_t* build)
{
    int sz = sizeof(arch_arch_t);
    #define GO(A)   if(build->A) sz+=sizeof(arch_##A##_t);
    SUPER()
    #undef GO
    return sz;
}

size_t get_size_arch(dynarec_arm_t* dyn)
{
    int noarch = BOX64ENV(dynarec_noarch);
    if(noarch>1) return 0;
    arch_build_t* previous = NULL;
    size_t sz = 0;
    int seq = 0;
    int nseq = 0;
    int last = 0;
    if(!dyn->size) return 0;
    for(int i=0; i<dyn->size; ++i) {
        arch_build_t* build = static_build+i;
        last = arch_build(dyn, i, build, noarch);
        if(i && (!memcmp(build, previous, sizeof(arch_build_t))) && (seq<((1<<10)-1))) {
            // same sequence, increment
            ++seq;
        } else {
            seq = 0;
            ++nseq;
            previous = build;
            sz += sizeof_arch_build(build);
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
    arch->seq = 0;
    void* p = ((void*)arch)+sizeof(arch_arch_t);
    #define GO(A)                                           \
    if(build->A) {                                          \
        memcpy(p, &(build->A##_), sizeof(arch_ ##A##_t));   \
        p+=sizeof(arch_##A##_t);                            \
    }
    SUPER()
    #undef GO
}

void* populate_arch(dynarec_arm_t* dyn, void* p, size_t tot_sz)
{
    arch_build_t* previous = NULL;
    arch_arch_t* arch = p;
    arch_arch_t* next = p;
    int seq = 0;
    size_t total = 0;
    if(!tot_sz) return NULL;
    for(int i=0; i<dyn->size; ++i) {
        arch_build_t* build = static_build+i;
        if(i && (!memcmp(build, previous, sizeof(arch_build_t))) && (seq<((1<<10)-1))) {
            // same sequence, increment
            arch->seq = ++seq;
        } else {
            int sz = sizeof_arch_build(build);
            if(total+sz>tot_sz) {
                printf_log(LOG_INFO, "Warning: populate_arch on undersized buffer (%d+%d/%d, inst %d/%d)\n", total, sz, tot_sz, i, dyn->size);
                return NULL;
            }
            arch = next;
            build_next(arch, build);
            seq = 0;
            previous = build;
            total += sz;
            next = (arch_arch_t*)((uintptr_t)arch+sz);
        }
    }
    return p;
}

// NZCV N
#define NZCV_N      31
// NZCV Z
#define NZCV_Z      30
// NZCV C
#define NZCV_C      29
// NZCV V
#define NZCV_V      28

#ifndef _WIN32 // TODO: Implemented this for Win32
void adjust_arch(dynablock_t* db, x64emu_t* emu, ucontext_t* p, uintptr_t x64pc)
{
    if(!db->arch_size || !db->arch)
        return;
    int ninst = getX64AddressInst(db, x64pc);
    dynarec_log(LOG_INFO, "adjust_arch(...), db=%p, x64pc=%p, nints=%d", db, (void*)x64pc, ninst);
    if(ninst<0) {
    dynarec_log_prefix(0, LOG_INFO, "\n");
        return;
    }
    if(ninst==0) {
    dynarec_log_prefix(0, LOG_INFO, "\n");
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
    while(i<(ninst)) {  // this will look for ninst-1 state
        arch = next;
        dynarec_log_prefix(0, (i<(ninst-1))?LOG_DEBUG:LOG_INFO, "[ seq=%d%s%s%s%s%s ] ", arch->seq, arch->flags?" Flags":"", arch->x87?" x87":"", arch->mmx?" MMX":"", arch->sse?" SSE":"", arch->ymm?" YMM":"");
        i += 1+arch->seq;
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
    if(flags) {
        dynarec_log_prefix(0, LOG_INFO, " flags[%s-%s%s%s%s] ", flags->defered?"defered":"", flags->nf?"S":"", flags->vf?"O":"", flags->eq?"Z":"", flags->cf?(flags->inv_cf?"C(inv)":"C"):"");
        if(flags->defered) {
            CHECK_FLAGS(emu);
            //return;
        }
        if(flags->nf) {
            CONDITIONAL_SET_FLAG(p->uc_mcontext.pstate&(1<<NZCV_N), F_SF);
        }
        if(flags->vf) {
            CONDITIONAL_SET_FLAG(p->uc_mcontext.pstate&(1<<NZCV_V), F_OF);
        }
        if(flags->eq) {
            CONDITIONAL_SET_FLAG(p->uc_mcontext.pstate&(1<<NZCV_Z), F_ZF);
        }
        if(flags->cf) {
            if(flags->inv_cf) {
                CONDITIONAL_SET_FLAG((p->uc_mcontext.pstate&(1<<NZCV_C))==0, F_CF);
            } else {
                CONDITIONAL_SET_FLAG(p->uc_mcontext.pstate&(1<<NZCV_C), F_CF);
            }
        }
    }
    struct fpsimd_context *fpsimd = NULL;
    // find fpsimd struct
    {
        struct _aarch64_ctx * ff = (struct _aarch64_ctx*)p->uc_mcontext.__reserved;
        while (ff->magic && !fpsimd) {
            if(ff->magic==FPSIMD_MAGIC)
                fpsimd = (struct fpsimd_context*)ff;
            else
                ff = (struct _aarch64_ctx*)((uintptr_t)ff + ff->size);
        }
    }
    if(sse) {
        dynarec_log_prefix(0, LOG_INFO, " sse[%x (fpsimd=%p)] ", sse->sse, fpsimd);
        for(int i=0; i<16; ++i)
            if(fpsimd && (sse->sse>>i)&1) {
                int idx = 0;
                if(i>7) {
                    idx = XMM8 + i - 8;
                } else {
                    idx = XMM0 + i;
                }
                emu->xmm[i].u128 = fpsimd->vregs[idx];
            }
    }
    if(ymm) {
        dynarec_log_prefix(0, LOG_INFO, " ymm[%x, pos=%x, 0=%x (fpsimd=%p)] ", ymm->ymm, ymm->ymm_pos, ymm->ymm0, fpsimd);
        for(int i=0; i<16; ++i) {
            if(fpsimd && (ymm->ymm>>i)&1) {
                int idx = (ymm->ymm_pos>>(i*4))&0xf;
                if(i>7) {
                    idx = SCRATCH0 + i - 8;
                } else {
                    idx = EMM0 + i;
                }
                emu->ymm[i].u128 = fpsimd->vregs[idx];
            }
            if(ymm->ymm0&(1<<i))
                emu->ymm[i].u128 = 0;
        }
    }
    if(mmx) {
        dynarec_log_prefix(0, LOG_INFO, " mmx[%x (fpsimd=%p)] ", mmx->mmx, fpsimd);
        for(int i=0; i<8; ++i)
            if(fpsimd && (mmx->mmx>>i)&1) {
                int idx = EMM0 + i;
                emu->mmx[i].q = fpsimd->vregs[idx]&0xffffffffffffffffLL;
            }
    }
    if(x87) {
        dynarec_log_prefix(0, LOG_INFO, " x87[%x, pos=%x, type=%x (fpsimd=%p)] ", x87->x87, x87->x87_pos, x87->x87_type, fpsimd);
        emu->top -= x87->delta;
        for(int i=0; i<8; ++i) {
            if(x87->x87&(1<<i)) {
                int idx = EMM0 + (x87->x87_pos>>(i*4))&0x0f;
                int t = (x87->x87_type>>(i*2))&0x3;
                switch (t) {
                    case X87_ST_F:
                        emu->x87[(emu->top+i)&7].d = *(float*)&fpsimd->vregs[idx];
                        break;
                    case X87_ST_I64:
                        emu->x87[(emu->top+i)&7].d = *(int64_t*)&fpsimd->vregs[idx];
                        break;
                    case X87_ST_D:
                        emu->x87[(emu->top+i)&7].d = *(double*)&fpsimd->vregs[idx];
                        break;
                }
            }
        }
    }
    dynarec_log_prefix(0, LOG_INFO, "\n");
}
#endif

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
