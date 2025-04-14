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
#define XMM0 0
#define X870 XMM0 + 16
#define EMM0 XMM0 + 16
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

static int arch_build(dynarec_rv64_t* dyn, int ninst, arch_build_t* arch)
{
    memset(arch, 0, sizeof(arch_build_t));
    // todo
    // opcode can handle unaligned
    arch->unaligned = dyn->insts[ninst].unaligned;
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
    // todo
    dynarec_log(LOG_INFO, "\n");
}

int arch_unaligned(dynablock_t* db, uintptr_t x64pc)
{
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
