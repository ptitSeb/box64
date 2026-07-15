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
#include "dynarec_la64_arch.h"
#include "dynarec_la64_functions.h"
#include "dynarec_la64_private.h"

typedef struct arch_arch_s
{
    uint16_t unaligned:1;
    uint16_t seq:10;    // how many instruction on the same values
    uint16_t up32;      // GPRs with pending 32-bit zero-up at this instruction
} arch_arch_t;

typedef struct arch_build_s
{
    uint8_t unaligned;
    uint16_t up32;
} arch_build_t;

static int arch_build(dynarec_la64_t* dyn, int ninst, arch_build_t* arch)
{
    memset(arch, 0, sizeof(arch_build_t));
    // opcode can handle unaligned
    arch->unaligned = dyn->insts[ninst].unaligned;
    // pending 32-bit zero-ups at this instruction
    arch->up32 = dyn->insts[ninst].up32_pending;
    return arch->unaligned || arch->up32;
}

size_t get_size_arch(dynarec_la64_t* dyn)
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
        }
    }
    if(nseq==1 && !last)
        return 0;   //empty, no flags, no nothing
    return sz;
}

static void build_next(arch_arch_t* arch, arch_build_t* build)
{
    arch->unaligned = build->unaligned;
    arch->seq = 0;
    arch->up32 = build->up32;
}

void* populate_arch(dynarec_la64_t* dyn, void* p, size_t sz)
{
    (void)sz;
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
            next = (arch_arch_t*)((uintptr_t)arch+sizeof(arch_arch_t));
        }
    }
    return p;
}

int getX64AddressInst(dynablock_t* db, uintptr_t x64pc); // define is signal.c

static arch_arch_t* arch_for_ninst(dynablock_t* db, int ninst)
{
    arch_arch_t* arch = db->arch;
    arch_arch_t* next = arch;
    int i = -1;
    while(i<ninst) {
        arch = next;
        i += 1+arch->seq;
        next = (arch_arch_t*)((uintptr_t)next+sizeof(arch_arch_t));
    }
    return arch;
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
    arch_arch_t* arch = arch_for_ninst(db, ninst);
    return arch->unaligned;
}

void adjust_arch(dynablock_t* db, x64emu_t* emu, ucontext_t* p, uintptr_t x64pc)
{
    (void)p;
    if(!db) return;
    if(!db->arch_size || !db->arch) return;
    int ninst = getX64AddressInst(db, x64pc);
    if(ninst < 0 || ninst >= db->isize) return;
    arch_arch_t* arch = arch_for_ninst(db, ninst);
    uint16_t up32 = arch->up32;
    while(up32) {
        int r = __builtin_ctz(up32);
        up32 &= up32 - 1;
        emu->regs[r].q[0] &= 0xffffffffULL;
    }
}
