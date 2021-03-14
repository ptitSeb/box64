#ifndef __DYNAREC_ARM_PRIVATE_H_
#define __DYNAREC_ARM_PRIVATE_H_

#include "dynarec_private.h"

typedef struct x64emu_s x64emu_t;

typedef struct instruction_arm_s {
    instruction_x64_t   x64;
    uintptr_t           address;    // (start) address of the arm emited instruction
    uintptr_t           epilog;     // epilog of current instruction (can be start of next, of barrier stuff)
    int                 size;       // size of the arm emited instruction
    int                 size2;      // size of the arm emited instrucion after pass2
    uintptr_t           mark, mark2, mark3;
    uintptr_t           markf;
    uintptr_t           markseg;
    uintptr_t           marklock;
    int                 pass2choice;// value for choices that are fixed on pass2 for pass3
    uintptr_t           natcall;
    int                 retn;
} instruction_arm64_t;

typedef struct dynarec_arm_s {
    instruction_arm64_t *insts;
    int32_t             size;
    int32_t             cap;
    uintptr_t           start;      // start of the block
    uint32_t            isize;      // size in byte of x64 instructions included
    void*               block;      // memory pointer where next instruction is emited
    uintptr_t           arm_start;  // start of the arm code
    int                 arm_size;   // size of emitted arm code
    int                 state_flags;// actual state for on-demand flags
    int                 x87cache[8];// cache status for the 8 x87 register behind the fpu stack
    int                 x87reg[8];  // reg used for x87cache entry
    int                 mmxcache[8];// cache status for the 8 MMX registers
    int                 ssecache[8];// cache status for the 8 SSE(2) registers
    int                 fpuused[24];// all 8..31 double reg from fpu, used by x87, sse and mmx
    int                 x87stack;   // cache stack counter
    int                 fpu_scratch;// scratch counter
    int                 fpu_extra_qscratch; // some opcode need an extra quad scratch register
    int                 fpu_reg;    // x87/sse/mmx reg counter
    int                 dfnone;     // if defered flags is already set to df_none
    uintptr_t*          next;       // variable array of "next" jump address
    int                 next_sz;
    int                 next_cap;
    uintptr_t*          sons_x64;   // the x64 address of potential dynablock sons
    void**              sons_arm;   // the arm address of potential dynablock sons
    int                 sons_size;  // number of potential dynablock sons
} dynarec_arm_t;

void add_next(dynarec_arm_t *dyn, uintptr_t addr);
uintptr_t get_closest_next(dynarec_arm_t *dyn, uintptr_t addr);
int is_nops(dynarec_arm_t *dyn, uintptr_t addr, int n);
int is_instructions(dynarec_arm_t *dyn, uintptr_t addr, int n);

#endif //__DYNAREC_ARM_PRIVATE_H_