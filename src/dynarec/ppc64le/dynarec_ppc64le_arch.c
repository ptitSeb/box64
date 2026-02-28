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
#include "dynarec_ppc64le_arch.h"
#include "dynarec_ppc64le_functions.h"
#include "dynarec_native.h"

// PPC64LE arch.c — signal recovery metadata
// Records per-instruction state so that adjust_arch() can reconstruct
// x86 emulator state (deferred flags, SIMD/x87/MMX register cache) when
// a signal interrupts dynarec-generated code.

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
} arch_flags_t;

#define X87_ST_D 0
#define X87_ST_F 1
#define X87_ST_I64 2
typedef struct arch_x87_s
{
    int8_t delta;        //up to +/-7
    uint8_t x87;        // 1bit is STx present
    uint16_t x87_type;  // 2bits per STx type
    uint32_t x87_pos;   //4bits per STx position
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

// VMX cache index constants (must match dynarec_ppc64le_functions.c)
#define XMM0 0
#define EMM0 (XMM0 + 16)

static int arch_build(dynarec_ppc64le_t* dyn, int ninst, arch_build_t* arch, int noarch)
{
    memset(arch, 0, sizeof(arch_build_t));
    // flags
    if(dyn->insts[ninst].f_entry!=status_none && dyn->insts[ninst].f_entry!=status_none_pending) {
        arch->flags = 1;
        arch->flags_.defered = 1;
    }
    // opcode can handle unaligned
    arch->unaligned = dyn->insts[ninst].unaligned;
    if(!noarch) {
        // go through all vmxcache to gather regs assignments
        for(int i=0; i<32; ++i)
            if(dyn->insts[ninst].v.vmxcache[i].v)
                switch(dyn->insts[ninst].v.vmxcache[i].t) {
                    case VMX_CACHE_XMMW:
                        arch->sse = 1;
                        arch->sse_.sse |= 1<<dyn->insts[ninst].v.vmxcache[i].n;
                        break;
                    case VMX_CACHE_MM:
                        arch->mmx = 1;
                        arch->mmx_.mmx |= 1<<dyn->insts[ninst].v.vmxcache[i].n;
                        break;
                    case VMX_CACHE_YMMW:
                        arch->ymm = 1;
                        arch->ymm_.ymm |= 1<<dyn->insts[ninst].v.vmxcache[i].n;
                        {
                            int idx = i;
                            if(idx>=EMM0 && idx<=EMM0+8)
                                idx-=EMM0;
                            else
                                idx-=SCRATCH0-8;
                            arch->ymm_.ymm_pos |= (uint64_t)idx<<(dyn->insts[ninst].v.vmxcache[i].n*4);
                        }
                        break;
                    case VMX_CACHE_ST_D:
                        arch->x87 = 1;
                        arch->x87_.x87 |= 1<<dyn->insts[ninst].v.vmxcache[i].n;
                        arch->x87_.x87_pos |= (uint32_t)(i-EMM0)<<(dyn->insts[ninst].v.vmxcache[i].n*4);
                        arch->x87_.x87_type |= (uint16_t)(X87_ST_D)<<(dyn->insts[ninst].v.vmxcache[i].n*2);
                        break;
                    case VMX_CACHE_ST_F:
                        arch->x87 = 1;
                        arch->x87_.x87 |= 1<<dyn->insts[ninst].v.vmxcache[i].n;
                        arch->x87_.x87_pos |= (uint32_t)(i-EMM0)<<(dyn->insts[ninst].v.vmxcache[i].n*4);
                        arch->x87_.x87_type |= (uint16_t)(X87_ST_F)<<(dyn->insts[ninst].v.vmxcache[i].n*2);
                        break;
                    case VMX_CACHE_ST_I64:
                        arch->x87 = 1;
                        arch->x87_.x87 |= 1<<dyn->insts[ninst].v.vmxcache[i].n;
                        arch->x87_.x87_pos |= (uint32_t)(i-EMM0)<<(dyn->insts[ninst].v.vmxcache[i].n*4);
                        arch->x87_.x87_type |= (uint16_t)(X87_ST_I64)<<(dyn->insts[ninst].v.vmxcache[i].n*2);
                        break;
                    case VMX_CACHE_XMMR:
                    case VMX_CACHE_YMMR:
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
        if(dyn->insts[ninst].v.x87stack) {
            arch->x87 = 1;
            arch->x87_.delta = dyn->insts[ninst].v.x87stack;
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

size_t get_size_arch(dynarec_ppc64le_t* dyn)
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

void* populate_arch(dynarec_ppc64le_t* dyn, void* p, size_t tot_sz)
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
        dynarec_log_prefix(0, LOG_INFO, " flags[%s] ", flags->defered?"defered":"");
        if(flags->defered) {
            CHECK_FLAGS(emu);
        }
    }
    // PPC64LE signal context:
    // mcontext_t.gp_regs[0..31] = r0-r31
    // mcontext_t.fp_regs[0..31] = f0-f31 (lower 64 bits of vs0-vs31)
    // mcontext_t.v_regs->vrregs[0..31][4] = vr0-vr31 (= vs32-vs63, 128-bit)
    //
    // VMX cache mapping:
    //   XMM cache idx 0-15  → vr0-vr15   → v_regs->vrregs[0..15]
    //   MMX cache idx 16-23 → vr24-vr31  → v_regs->vrregs[24..31] (64-bit in low dword)
    //   x87 cache idx 16-23 → f16-f23    → fp_regs[16..23] (scalar 64-bit)
    //   Scratch idx 24-31   → vr16-vr23  → v_regs->vrregs[16..23]
    mcontext_t* mc = &p->uc_mcontext;
    vrregset_t* vmx = mc->v_regs;

    if(sse) {
        dynarec_log_prefix(0, LOG_INFO, " sse[%x (vmx=%p)] ", sse->sse, vmx);
        for(int i=0; i<16; ++i)
            if(vmx && (sse->sse>>i)&1) {
                // XMM i is cached in vmx register. Find which physical register.
                // For XMM, cache index = i, physical = vr_i (vrregs[i])
                int idx = i;  // cache index = xmm number for XMM
                // On PPC64LE LE: vrregs[idx][0]=low32, [1]=next32, [2]=next32, [3]=high32
                // This matches x86 element order directly
                memcpy(&emu->xmm[i], &vmx->vrregs[idx], 16);
            }
    }
    if(ymm) {
        dynarec_log_prefix(0, LOG_INFO, " ymm[%x, pos=%x, 0=%x (vmx=%p)] ", ymm->ymm, ymm->ymm_pos, ymm->ymm0, vmx);
        for(int i=0; i<16; ++i) {
            if(vmx && (ymm->ymm>>i)&1) {
                int idx = (ymm->ymm_pos>>(i*4))&0xf;
                // idx is a cache-relative position:
                // 0-7 → EMM0+idx → vs(16+idx) → fp_regs but these are 64-bit only
                // 8-15 → SCRATCH0+(idx-8) → vr(16+idx-8) → vrregs[16+idx-8]
                if(idx >= 8) {
                    int vr = 16 + idx - 8;
                    memcpy(&emu->ymm[i], &vmx->vrregs[vr], 16);
                }
                // If idx < 8 (EMM0 space), those are vs16-vs23 in FPR space,
                // only 64-bit accessible via fp_regs — not sufficient for 128-bit ymm.
                // In practice, ymm should always use scratch (VMX) space.
            }
            if(ymm->ymm0&(1<<i))
                memset(&emu->ymm[i], 0, 16);
        }
    }
    if(mmx) {
        dynarec_log_prefix(0, LOG_INFO, " mmx[%x (vmx=%p)] ", mmx->mmx, vmx);
        for(int i=0; i<8; ++i)
            if(vmx && (mmx->mmx>>i)&1) {
                // MMX i is cached in vr(24+i) = vs(56+i) → v_regs->vrregs[24+i]
                // Data is in low 64 bits (bytes 0-7 on LE)
                int vr_idx = 24 + i;
                emu->mmx[i].q = *(uint64_t*)&vmx->vrregs[vr_idx];
            }
    }
    if(x87) {
        dynarec_log_prefix(0, LOG_INFO, " x87[%x, pos=%x, type=%x] ", x87->x87, x87->x87_pos, x87->x87_type);
        emu->top -= x87->delta;
        for(int i=0; i<8; ++i) {
            if(x87->x87&(1<<i)) {
                int pos = (x87->x87_pos>>(i*4))&0x0f;
                int fpr_idx = 16 + pos;  // EMM0 + pos → vs(16+pos) → fp_regs[16+pos]
                int t = (x87->x87_type>>(i*2))&0x3;
                switch (t) {
                    case X87_ST_F:
                        emu->x87[(emu->top+i)&7].d = *(float*)&mc->fp_regs[fpr_idx];
                        break;
                    case X87_ST_I64:
                        emu->x87[(emu->top+i)&7].d = *(int64_t*)&mc->fp_regs[fpr_idx];
                        break;
                    case X87_ST_D:
                        emu->x87[(emu->top+i)&7].d = *(double*)&mc->fp_regs[fpr_idx];
                        break;
                }
            }
        }
    }
    dynarec_log_prefix(0, LOG_INFO, "\n");
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
