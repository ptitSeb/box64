#ifndef __DYNAREC_RV64_PRIVATE_H_
#define __DYNAREC_RV64_PRIVATE_H_
#include <stdint.h>

#include "../dynarec_private.h"

typedef struct x64emu_s x64emu_t;
typedef struct dynablock_s dynablock_t;
typedef struct instsize_s instsize_t;

#define BARRIER_MAYBE   8

#define EXT_CACHE_NONE   0
#define EXT_CACHE_ST_D   1
#define EXT_CACHE_ST_F   2
#define EXT_CACHE_ST_I64 3
#define EXT_CACHE_MM     4
#define EXT_CACHE_SS     5
#define EXT_CACHE_SD     6
#define EXT_CACHE_SCR    7
typedef union ext_cache_s {
    int8_t           v;
    struct {
        uint8_t t:4;   // reg type
        uint8_t n:4;   // reg number
    };
} ext_cache_t;
typedef union sse_cache_s {
    int8_t      v;
    struct {
        uint8_t     reg:7;
        uint8_t     single:1;
    };
} sse_cache_t;
typedef union sse_old_s {
    int8_t      v;
    struct {
        uint8_t     changed:1;
        uint8_t     purged:1;
        uint8_t     reg:4;
        uint8_t     single:1;
    };
} sse_old_t;
typedef struct extcache_s {
    // ext cache
    ext_cache_t         extcache[24];
    int8_t              stack;
    int8_t              stack_next;
    int8_t              stack_pop;
    int8_t              stack_push;
    uint8_t             combined1;
    uint8_t             combined2;
    uint8_t             swapped;        // the combined reg were swapped
    uint8_t             barrier;        // is there a barrier at instruction epilog?
    uint8_t             pushed;         // positive pushed value (to check for overflow)
    uint8_t             poped;          // positive poped value (to check for underflow)
    uint32_t            news;           // bitmask, wich neoncache are new for this opcode
    sse_old_t           olds[16];       // SSE regs has changed or has been removed
    // fpu cache
    int8_t              x87cache[8];    // cache status for the 8 x87 register behind the fpu stack
    int8_t              x87reg[8];      // reg used for x87cache entry
    int16_t             tags;           // similar to fpu_tags
    int8_t              mmxcache[8];    // cache status for the 8 MMX registers
    sse_cache_t         ssecache[16];   // cache status for the 16 SSE(2) registers
    int8_t              fpuused[24];    // all 10..31 & 0..1 double reg from fpu, used by x87, sse and mmx
    int8_t              x87stack;       // cache stack counter
    int8_t              mmxcount;       // number of mmx register used (not both mmx and x87 at the same time)
    int8_t              fpu_scratch;    // scratch counter
    int8_t              fpu_extra_qscratch; // some opcode need an extra quad scratch register
    int8_t              fpu_reg;        // x87/sse/mmx reg counter
} extcache_t;

typedef struct flagcache_s {
    int                 pending;    // is there a pending flags here, or to check?
    int                 dfnone;     // if deferred flags is already set to df_none
} flagcache_t;

typedef struct instruction_rv64_s {
    instruction_x64_t   x64;
    uintptr_t           address;    // (start) address of the arm emitted instruction
    uintptr_t           epilog;     // epilog of current instruction (can be start of next, or barrier stuff)
    int                 size;       // size of the arm emitted instruction
    int                 size2;      // size of the arm emitted instrucion after pass2
    int                 pred_sz;    // size of predecessor list
    int                 *pred;      // predecessor array
    uintptr_t           mark[3];
    uintptr_t           markf[2];
    uintptr_t           markseg;
    uintptr_t           marklock;
    int                 pass2choice;// value for choices that are fixed on pass2 for pass3
    uintptr_t           natcall;
    int                 retn;
    int                 barrier_maybe;
    flagcache_t         f_exit;     // flags status at end of intruction
    extcache_t          e;          // extcache at end of intruction (but before poping)
    flagcache_t         f_entry;    // flags status before the instruction begin
} instruction_rv64_t;

typedef struct dynarec_rv64_s {
    instruction_rv64_t* insts;
    int32_t             size;
    int32_t             cap;
    uintptr_t           start;      // start of the block
    uint32_t            isize;      // size in byte of x64 instructions included
    void*               block;      // memory pointer where next instruction is emitted
    uintptr_t           native_start;  // start of the arm code
    size_t              native_size;   // size of emitted arm code
    uintptr_t           last_ip;    // last set IP in RIP (or NULL if unclean state) TODO: move to a cache something
    uint64_t*           table64;   // table of 64bits value
    int                 table64size;// size of table (will be appended at end of executable code)
    int                 table64cap;
    uintptr_t           tablestart;
    uintptr_t           jmp_next;   // address of the jump_next address
    flagcache_t         f;
    extcache_t          e;          // cache for the 10..31 0..1 double reg from fpu, plus x87 stack delta
    uintptr_t*          next;       // variable array of "next" jump address
    int                 next_sz;
    int                 next_cap;
    int*                jmps;       // variable array of jump instructions
    int                 jmp_sz;
    int                 jmp_cap;
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
    uint8_t             always_test;
    uint8_t             abort;
} dynarec_rv64_t;

// convert idx (0..24) to reg index (10..31 0..1)
#define EXTREG(A)   (((A)+10)&31)
// convert reg index (10..31 0..1) or idx (0..24)
#define EXTIDX(A)   (((A)-10)&31)

void add_next(dynarec_rv64_t *dyn, uintptr_t addr);
uintptr_t get_closest_next(dynarec_rv64_t *dyn, uintptr_t addr);
void add_jump(dynarec_rv64_t *dyn, int ninst);
int get_first_jump(dynarec_rv64_t *dyn, int next);
int is_nops(dynarec_rv64_t *dyn, uintptr_t addr, int n);
int is_instructions(dynarec_rv64_t *dyn, uintptr_t addr, int n);

int Table64(dynarec_rv64_t *dyn, uint64_t val, int pass);  // add a value to table64 (if needed) and gives back the imm19 to use in LDR_literal

void CreateJmpNext(void* addr, void* next);

#define GO_TRACE(A, B, s0)  \
    GETIP(addr);            \
    MV(A1, xRIP);           \
    STORE_XEMU_CALL(s0);    \
    MOV64x(A2, B);          \
    CALL(A, -1);            \
    LOAD_XEMU_CALL()

#endif //__DYNAREC_RV64_PRIVATE_H_
