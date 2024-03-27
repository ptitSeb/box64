#ifndef __DYNAREC_LA64_PRIVATE_H_
#define __DYNAREC_LA64_PRIVATE_H_

#include "../dynarec_private.h"

typedef struct x64emu_s x64emu_t;
typedef struct dynablock_s dynablock_t;
typedef struct instsize_s instsize_t;

#define BARRIER_MAYBE 8

typedef struct flagcache_s {
    int pending; // is there a pending flags here, or to check?
    int dfnone;  // if deferred flags is already set to df_none
} flagcache_t;

typedef struct instruction_la64_s {
    instruction_x64_t x64;
    uintptr_t address; // (start) address of the arm emitted instruction
    uintptr_t epilog;  // epilog of current instruction (can be start of next, or barrier stuff)
    int size;          // size of the arm emitted instruction
    int size2;         // size of the arm emitted instrucion after pass2
    int pred_sz;       // size of predecessor list
    int* pred;         // predecessor array
    uintptr_t mark[3];
    uintptr_t markf[2];
    uintptr_t markseg;
    uintptr_t marklock;
    int pass2choice; // value for choices that are fixed on pass2 for pass3
    uintptr_t natcall;
    int retn;
    uint8_t barrier_maybe;
    uint8_t will_write;
    uint8_t last_write;
    flagcache_t f_exit;  // flags status at end of instruction
    flagcache_t f_entry; // flags status before the instruction begin
} instruction_la64_t;

typedef struct dynarec_la64_s {
    instruction_la64_t* insts;
    int32_t size;
    int32_t cap;
    uintptr_t start;        // start of the block
    uint32_t isize;         // size in bytes of x64 instructions included
    void* block;            // memory pointer where next instruction is emitted
    uintptr_t native_start; // start of the arm code
    size_t native_size;     // size of emitted arm code
    uintptr_t last_ip;      // last set IP in RIP (or NULL if unclean state) TODO: move to a cache something
    uint64_t* table64;      // table of 64bits values
    int table64size;        // size of table (will be appended at end of executable code)
    int table64cap;
    uintptr_t tablestart;
    uintptr_t jmp_next; // address of the jump_next address
    flagcache_t f;
    uintptr_t* next; // variable array of "next" jump address
    int next_sz;
    int next_cap;
    int* jmps; // variable array of jump instructions
    int jmp_sz;
    int jmp_cap;
    int* predecessor; // single array of all predecessor
    dynablock_t* dynablock;
    instsize_t* instsize;
    size_t insts_size;    // size of the instruction size array (calculated)
    uintptr_t forward;    // address of the last end of code while testing forward
    uintptr_t forward_to; // address of the next jump to (to check if everything is ok)
    int32_t forward_size; // size at the forward point
    int forward_ninst;    // ninst at the forward point
    uint8_t smread;       // for strongmem model emulation
    uint8_t smwrite;      // for strongmem model emulation
    uint8_t always_test;
    uint8_t abort;
} dynarec_la64_t;

void add_next(dynarec_la64_t* dyn, uintptr_t addr);
uintptr_t get_closest_next(dynarec_la64_t* dyn, uintptr_t addr);
void add_jump(dynarec_la64_t* dyn, int ninst);
int get_first_jump(dynarec_la64_t* dyn, int next);
int is_nops(dynarec_la64_t* dyn, uintptr_t addr, int n);
int is_instructions(dynarec_la64_t* dyn, uintptr_t addr, int n);

int Table64(dynarec_la64_t* dyn, uint64_t val, int pass); // add a value to table64 (if needed) and gives back the imm19 to use in LDR_literal

void CreateJmpNext(void* addr, void* next);

#define GO_TRACE(A, B, s0) \
    GETIP(addr);           \
    MV(A1, xRIP);          \
    STORE_XEMU_CALL();     \
    MOV64x(A2, B);         \
    CALL(A, -1);           \
    LOAD_XEMU_CALL()

#endif //__DYNAREC_ARM_PRIVATE_H_
