#include <stddef.h>
#include <stdio.h>
#include <signal.h>
#include <ucontext.h>
#include <string.h>

#include "debug.h"
#include "dynablock.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "x64run.h"
#include "emu/x64run_private.h"
#include "dynarec/dynablock_private.h"
#include "dynarec_arm64_arch.h"
#include "dynarec_arm64_functions.h"

typedef struct arch_build_s
{
    uint8_t flags:1;
    uint8_t x87:1;
    uint8_t mmx:1;
    uint8_t sse:1;
    uint8_t ymm:1;
    arch_flags_t flags_;
    arch_x87_t x87_;
    arch_mmx_t mmx_;
    arch_sse_t sse_;
    arch_ymm_t ymm_;
} arch_build_t;

static int arch_build(dynarec_arm_t* dyn, int ninst, arch_build_t* arch)
{
    memset(arch, 0, sizeof(arch_build_t));
    // flags
    if((dyn->insts[ninst].f_entry.dfnone==0) || dyn->insts[ninst].need_nat_flags) {
        arch->flags = 1;
        arch->flags_.defered = dyn->insts[ninst].f_entry.dfnone==0;
        arch->flags_.vf = dyn->insts[ninst].need_nat_flags&NF_VF;
        arch->flags_.nf = dyn->insts[ninst].need_nat_flags&NF_SF;
        arch->flags_.eq = dyn->insts[ninst].need_nat_flags&NF_EQ;
        arch->flags_.cf = dyn->insts[ninst].need_nat_flags&NF_CF;
        if(arch->flags_.cf)
            arch->flags_.inv_cf = !dyn->insts[ninst].normal_carry;
    }
    // sse
    for(int i=0; i<16; ++i)
        if(dyn->insts[ninst].n.ssecache[i].v!=-1) {
            arch->sse = 1;
            arch->sse_.sse |= 1<<i;
        }
    return arch->flags + arch->x87 + arch->mmx + arch->sse + arch->ymm;
}

size_t get_size_arch(dynarec_arm_t* dyn)
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
        if((!memcmp(&build, &previous, sizeof(arch_build_t))) && (seq<((1<<11)-1)) && i) {
            // same sequence, increment
            ++seq;
        } else {
            seq = 0;
            ++nseq;
            memcpy(&previous, &build, sizeof(arch_build_t));
            sz+=sizeof(arch_arch_t);
            if(build.flags) sz+=sizeof(arch_flags_t);
            if(build.x87) sz+=sizeof(arch_x87_t);
            if(build.mmx) sz+=sizeof(arch_mmx_t);
            if(build.sse) sz+=sizeof(arch_sse_t);
            if(build.ymm) sz+=sizeof(arch_ymm_t);
        }
    }
    if(nseq==1 && !last)
        return 0;   //empty, no flags, no nothing
    return sz;
}

static void build_next(arch_arch_t* arch, arch_build_t* build)
{
    arch->flags = build->flags;
    arch->x87 = build->x87;
    arch->mmx = build->mmx;
    arch->sse = build->sse;
    arch->ymm = build->ymm;
    arch->seq = 0;
    void* p = ((void*)arch)+sizeof(arch_arch_t);
    #define GO(A)                                           \
    if(arch->A) {                                           \
        memcpy(p, &build->A##_, sizeof(arch_ ##A##_t));     \
        p+=sizeof(arch_##A##_t);                            \
    }
    GO(flags)
    GO(x87)
    GO(mmx)
    GO(sse)
    GO(ymm)
    #undef GO
}

static int sizeof_arch(arch_arch_t* arch)
{
    int sz = sizeof(arch_arch_t);
    #define GO(A)   if(arch->A) sz+=sizeof(arch_##A##_t)
    GO(flags);
    GO(x87);
    GO(mmx);
    GO(sse);
    GO(ymm);
    #undef GO
    return sz;
}

void populate_arch(dynarec_arm_t* dyn, void* p)
{
    arch_build_t build = {0};
    arch_build_t previous = {0};
    arch_arch_t* arch = p;
    arch_arch_t* next = p;
    int seq = 0;
    for(int i=0; i<dyn->size; ++i) {
        arch_build(dyn, i, &build);
        if((!memcmp(&build, &previous, sizeof(arch_build_t))) && (seq<((1<<11)-1)) && i) {
            // same sequence, increment
            seq++;
            arch->seq = seq;
        } else {
            arch = next;
            build_next(arch, &build);
            seq = 0;
            memcpy(&previous, &build, sizeof(arch_build_t));
            int sz = sizeof_arch(arch);
            next = (arch_arch_t*)((uintptr_t)next+sz);
        }
    }
}

int getX64AddressInst(dynablock_t* db, uintptr_t native_addr); // define is signal.c

// NZCV N
#define NZCV_N      31
// NZCV Z
#define NZCV_Z      30
// NZCV C
#define NZCV_C      29
// NZCV V
#define NZCV_V      28

void adjust_arch(dynablock_t* db, x64emu_t* emu, ucontext_t* p, uintptr_t x64pc)
{
    if(!db->arch_size || !db->arch)
        return;
    int ninst = getX64AddressInst(db, x64pc);
//printf_log(LOG_INFO, "adjust_arch(...), db=%p, x64pc=%p, nints=%d", db, (void*)x64pc, ninst);
    if(ninst<0) {
//printf_log(LOG_INFO, "\n");
        return;
    }
    if(ninst==0) {
//printf_log(LOG_INFO, "\n");
        CHECK_FLAGS(emu);
        return;
    }
    // look for state at ninst-1
    arch_arch_t* arch = db->arch;
    arch_arch_t* next = arch;
    int i = -1;
    while(i<ninst-1) {
        arch = next;
        i += 1+arch->seq;
//printf_log(LOG_INFO, "[ seq=%d%s%s%s%s%s ] ", arch->seq, arch->flags?" Flags":"", arch->x87?" x87":"", arch->mmx?" MMX":"", arch->sse?" SSE":"", arch->ymm?" YMM":"");
        next = (arch_arch_t*)((uintptr_t)next + sizeof_arch(arch));
    }
    int sz = sizeof(arch_arch_t);
    if(arch->flags) {
        arch_flags_t* flags = (arch_flags_t*)((uintptr_t)arch + sz);
        sz += sizeof(arch_flags_t);
//printf_log(LOG_INFO, " flags[%s-%s%s%s%s] ", flags->defered?"defered":"", flags->nf?"S":"", flags->vf?"O":"", flags->eq?"Z":"", flags->cf?(flags->inv_cf?"C(inv)":"C"):"");
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
    if(arch->sse) {
        arch_sse_t* sse = (arch_sse_t*)((uintptr_t)arch + sz);
//printf_log(LOG_INFO, " sse[%x (fpsimd=%p)] ", sse->sse, fpsimd);
        sz += sizeof(arch_sse_t);
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
//printf_log(LOG_INFO, "\n");
}