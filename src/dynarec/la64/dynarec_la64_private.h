#ifndef __DYNAREC_LA64_PRIVATE_H_
#define __DYNAREC_LA64_PRIVATE_H_

#include "../dynarec_private.h"

typedef struct x64emu_s x64emu_t;
typedef struct dynablock_s dynablock_t;
typedef struct instsize_s instsize_t;
typedef struct box64env_s box64env_t;

#define BARRIER_MAYBE   8

#define LSX_CACHE_NONE   0
#define LSX_CACHE_ST_D   1
#define LSX_CACHE_ST_F   2
#define LSX_CACHE_ST_I64 3
#define LSX_CACHE_MM     4
#define LSX_CACHE_XMMW   5
#define LSX_CACHE_XMMR   6
#define LSX_CACHE_YMMW   7
#define LSX_CACHE_YMMR   8
#define LSX_CACHE_SCR    9

#define LSX_AVX_WIDTH_128 0
#define LSX_AVX_WIDTH_256 1

typedef union lsx_cache_s {
    int8_t v;
    struct {
        uint8_t t : 4; // reg type
        uint8_t n : 4; // reg number
    };
} lsx_cache_t;

typedef union sse_cache_s {
    int8_t v;
    struct {
        uint8_t reg : 7;
        uint8_t write : 1;
    };
} sse_cache_t;

typedef union avx_cache_s {
    int8_t v;
    struct {
        uint8_t reg : 5;
        uint8_t width : 1;
        uint8_t zero_upper : 1;        
        uint8_t write : 1;
    };
} avx_cache_t;

typedef struct lsxcache_s {
    // LSX cache
    lsx_cache_t     lsxcache[24];
    int8_t          stack;
    int8_t          stack_next;
    int8_t          stack_pop;
    int8_t          stack_push;
    uint8_t         combined1;
    uint8_t         combined2;
    uint8_t         swapped;        // the combined reg were swapped
    uint8_t         barrier;        // is there a barrier at instruction epilog?
    uint8_t         pushed;         // positive pushed value (to check for overflow)
    uint8_t         poped;          // positive poped value (to check for underflow)
    uint32_t        news;           // bitmask, wich neoncache are new for this opcode
    // fpu cache
    int8_t          x87cache[8];    // cache status for the 8 x87 register behind the fpu stack
    int8_t          x87reg[8];      // reg used for x87cache entry
    int16_t         tags;           // similar to fpu_tags
    int8_t          mmxcache[8];    // cache status for the 8 MMX registers
    sse_cache_t     ssecache[16];   // cache status for the 16 SSE(2) registers
    avx_cache_t     avxcache[16];   // cache status for the 16 SSE(2) registers
    int8_t          fpuused[24];    // all 0..24 double reg from fpu, used by x87, sse and mmx
    int8_t          x87stack;       // cache stack counter
    int8_t          mmxcount;       // number of mmx register used (not both mmx and x87 at the same time)
    int8_t          fpu_scratch;    // scratch counter
} lsxcache_t;

typedef struct flagcache_s {
    int                 pending;    // is there a pending flags here, or to check?
    uint8_t             dfnone;     // if deferred flags is already set to df_none
} flagcache_t;

typedef struct callret_s callret_t;

typedef struct instruction_la64_s {
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
    uintptr_t           marklock2;
    int                 pass2choice;// value for choices that are fixed on pass2 for pass3
    uintptr_t           natcall;
    uint16_t            retn;
    uint16_t            ymm0_pass2, ymm0_pass3;
    uint8_t             barrier_maybe;
    uint8_t             will_write:2;    // [strongmem] will write to memory
    uint8_t             will_read:1;     // [strongmem] will read from memory
    uint8_t             last_write:1;    // [strongmem] the last write in a SEQ
    uint8_t             lock:1;          // [strongmem] lock semantic
    uint8_t             df_notneeded;
    uint8_t             nat_flags_fusion:1;
    uint8_t             nat_flags_nofusion:1;
    uint8_t             nat_flags_carry:1;
    uint8_t             nat_flags_sign:1;
    uint8_t             nat_flags_needsign:1;
    uint8_t             nat_flags_op1;
    uint8_t             nat_flags_op2;
    flagcache_t         f_exit;     // flags status at end of instruction
    lsxcache_t          lsx;        // lsxcache at end of instruction (but before poping)
    flagcache_t         f_entry;    // flags status before the instruction begin
} instruction_la64_t;

typedef struct dynarec_la64_s {
    instruction_la64_t* insts;
    int32_t              size;
    int32_t              cap;
    uintptr_t            start;      // start of the block
    uintptr_t            end;        // maximum end of the block (only used in pass0)
    uint32_t             isize;      // size in bytes of x64 instructions included
    void*                block;      // memory pointer where next instruction is emitted
    uintptr_t            native_start;  // start of the arm code
    size_t               native_size;   // size of emitted arm code
    uintptr_t            last_ip;    // last set IP in RIP (or NULL if unclean state) TODO: move to a cache something
    uint64_t*            table64;    // table of 64bits values
    int                  table64size;// size of table (will be appended at end of executable code)
    int                  table64cap;
    uintptr_t            tablestart;
    uintptr_t            jmp_next;   // address of the jump_next address
    flagcache_t          f;
    lsxcache_t           lsx;
    uintptr_t*           next;       // variable array of "next" jump address
    int                  next_sz;
    int                  next_cap;
    int*                 jmps;       // variable array of jump instructions
    int                  jmp_sz;
    int                  jmp_cap;
    int*                 predecessor;// single array of all predecessor
    dynablock_t*         dynablock;
    instsize_t*          instsize;
    size_t               insts_size; // size of the instruction size array (calculated)
    int                  callret_size;   // size of the array
    callret_t*           callrets;   // arrey of callret return, with NOP / UDF depending if the block is clean or dirty
    uintptr_t            forward;    // address of the last end of code while testing forward
    uintptr_t            forward_to; // address of the next jump to (to check if everything is ok)
    int32_t              forward_size;   // size at the forward point
    int                  forward_ninst;  // ninst at the forward point
    uint16_t             ymm_zero;   // bitmap of ymm to zero at purge
    uint8_t              smwrite;    // for strongmem model emulation
    uint8_t              always_test;
    uint8_t              abort;
    void*               gdbjit_block;
    uint32_t            need_x87check; // x87 low precision check
    uint32_t            need_dump;     // need to dump the block
    int                 need_reloc; // does the dynablock need relocations
    int                 reloc_size;
    uint32_t*           relocs;
    box64env_t*         env;
} dynarec_la64_t;

void add_next(dynarec_la64_t *dyn, uintptr_t addr);
uintptr_t get_closest_next(dynarec_la64_t *dyn, uintptr_t addr);
void add_jump(dynarec_la64_t *dyn, int ninst);
int get_first_jump(dynarec_la64_t *dyn, int next);
int get_first_jump_addr(dynarec_la64_t *dyn, uintptr_t next);
int is_nops(dynarec_la64_t *dyn, uintptr_t addr, int n);
int is_instructions(dynarec_la64_t *dyn, uintptr_t addr, int n);

int isTable64(dynarec_la64_t *dyn, uint64_t val); // return 1 if val already in Table64
int Table64(dynarec_la64_t *dyn, uint64_t val, int pass);  // add a value to table64 (if needed) and gives back the imm19 to use in LDR_literal

void CreateJmpNext(void* addr, void* next);

#define GO_TRACE(A, B, s0)         \
    GETIP(addr, s0);               \
    MV(x1, xRIP);                  \
    STORE_XEMU_CALL();             \
    MOV64x(x2, B);                 \
    CALL(const_##A, -1, x1, x2);   \
    LOAD_XEMU_CALL()

#endif //__DYNAREC_ARM_PRIVATE_H_
