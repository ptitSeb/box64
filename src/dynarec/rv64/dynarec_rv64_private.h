#ifndef __DYNAREC_RV64_PRIVATE_H_
#define __DYNAREC_RV64_PRIVATE_H_
#include <stdint.h>

#include "../dynarec_private.h"

typedef struct x64emu_s x64emu_t;
typedef struct dynablock_s dynablock_t;
typedef struct instsize_s instsize_t;

#define BARRIER_MAYBE   8

typedef struct flagcache_s {
    int                 pending;    // is there a pending flags here, or to check?
    int                 dfnone;     // if defered flags is already set to df_none
} flagcache_t;

typedef struct instruction_rv64_s {
    instruction_x64_t   x64;
    uintptr_t           address;    // (start) address of the arm emited instruction
    uintptr_t           epilog;     // epilog of current instruction (can be start of next, or barrier stuff)
    int                 size;       // size of the arm emited instruction
    int                 size2;      // size of the arm emited instrucion after pass2
    int                 pred_sz;    // size of predecessor list
    int                 *pred;      // predecessor array
    uintptr_t           mark, mark2, mark3;
    uintptr_t           markf;
    uintptr_t           markseg;
    uintptr_t           marklock;
    int                 pass2choice;// value for choices that are fixed on pass2 for pass3
    uintptr_t           natcall;
    int                 retn;
    int                 barrier_maybe;
    flagcache_t         f_exit;     // flags status at end of intruction
    flagcache_t         f_entry;    // flags status before the instruction begin
} instruction_rv64_t;

typedef struct dynarec_rv64_s {
    instruction_rv64_t* insts;
    int32_t             size;
    int32_t             cap;
    uintptr_t           start;      // start of the block
    uint32_t            isize;      // size in byte of x64 instructions included
    void*               block;      // memory pointer where next instruction is emited
    uintptr_t           native_start;  // start of the arm code
    size_t              native_size;   // size of emitted arm code
    uintptr_t           last_ip;    // last set IP in RIP (or NULL if unclean state) TODO: move to a cache something
    uint64_t*           table64;   // table of 64bits value
    int                 table64size;// size of table (will be appended at end of executable code)
    int                 table64cap;
    uintptr_t           tablestart;
    flagcache_t         f;
    uintptr_t*          next;       // variable array of "next" jump address
    int                 next_sz;
    int                 next_cap;
    int*                predecessor;// single array of all predecessor
    dynablock_t*        dynablock;
    instsize_t*         instsize;
    size_t              insts_size; // size of the instruction size array (calculated)
    uint8_t             smread;    // for strongmem model emulation
    uint8_t             smwrite;    // for strongmem model emulation
    uintptr_t           forward;    // address of the last end of code while testing forward
    uintptr_t           forward_to; // address of the next jump to (to check if everything is ok)
    int32_t             forward_size;   // size at the forward point
    int                 forward_ninst;  // ninst at the forward point
} dynarec_rv64_t;

void add_next(dynarec_rv64_t *dyn, uintptr_t addr);
uintptr_t get_closest_next(dynarec_rv64_t *dyn, uintptr_t addr);
int is_nops(dynarec_rv64_t *dyn, uintptr_t addr, int n);
int is_instructions(dynarec_rv64_t *dyn, uintptr_t addr, int n);

int Table64(dynarec_rv64_t *dyn, uint64_t val);  // add a value to etable64 (if needed) and gives back the imm19 to use in LDR_literal

void CreateJmpNext(void* addr, void* next);

#define GO_TRACE()          \
    GETIP_(ip);             \
    MV(A1, xRIP);           \
    STORE_XEMU_CALL();      \
    MOV64x(A2, 1);          \
    CALL(PrintTrace, -1);   \
    LOAD_XEMU_CALL()

#endif //__DYNAREC_RV64_PRIVATE_H_