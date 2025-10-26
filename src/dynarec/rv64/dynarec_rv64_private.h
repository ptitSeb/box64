#ifndef __DYNAREC_RV64_PRIVATE_H_
#define __DYNAREC_RV64_PRIVATE_H_
#include <stdint.h>

#include "../dynarec_private.h"

typedef struct x64emu_s x64emu_t;
typedef struct dynablock_s dynablock_t;
typedef struct instsize_s instsize_t;
typedef struct box64env_s box64env_t;

#define BARRIER_MAYBE   8

#define EXT_CACHE_NONE   0
#define EXT_CACHE_ST_D   1
#define EXT_CACHE_ST_F   2
#define EXT_CACHE_ST_I64 3
#define EXT_CACHE_MM     4
#define EXT_CACHE_SS     5
#define EXT_CACHE_SD     6
#define EXT_CACHE_SCR    7
#define EXT_CACHE_XMMW   8
#define EXT_CACHE_XMMR   9
#define EXT_CACHE_MMV    10

#define EXT_CACHE_OLD_SD   0
#define EXT_CACHE_OLD_SS   1
#define EXT_CACHE_OLD_XMMW 2
#define EXT_CACHE_OLD_XMMR 3

typedef union ext_cache_s {
    int8_t           v;
    struct {
        uint8_t t:4;   // reg type
        uint8_t n:4;   // reg number
    };
} ext_cache_t;

typedef union mmx_cache_s {
    int8_t v;
    struct {
        uint8_t reg : 7;
        uint8_t vector : 1;
    };
} mmx_cache_t;

typedef union sse_cache_s {
    int16_t v;
    struct {
        uint16_t reg : 13;
        uint16_t vector : 1;
        uint16_t single : 1;
        uint16_t write : 1;
    };
} sse_cache_t;

typedef union sse_old_s {
    int8_t      v;
    struct {
        uint8_t     changed:1;
        uint8_t     purged:1;
        uint8_t     reg:4;
        uint8_t     type:2;
    };
} sse_old_t;

typedef struct extcache_s {
    // ext cache
    ext_cache_t         extcache[32];
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
    mmx_cache_t         mmxcache[8];    // cache status for the 8 MMX registers
    sse_cache_t         ssecache[16];   // cache status for the 16 SSE(2) registers
    int8_t              fpuused[32];    // all double reg from fpu, used by x87, mmx, sse and avx
    int8_t              x87stack;       // cache stack counter
    int8_t              mmxcount;       // number of mmx register used (not both mmx and x87 at the same time)
    int8_t              fpu_scratch;    // scratch counter
} extcache_t;

typedef struct flagcache_s {
    int                 pending;    // is there a pending flags here, or to check?
    uint8_t             dfnone;     // if deferred flags is already set to df_none
} flagcache_t;

typedef struct callret_s callret_t;

typedef struct instruction_rv64_s {
    instruction_x64_t   x64;
    uintptr_t           address;    // (start) address of the riscv emitted instruction
    uintptr_t           epilog;     // epilog of current instruction (can be start of next, or barrier stuff)
    int                 size;       // size of the riscv emitted instruction
    int                 size2;      // size of the riscv emitted instruction after pass2
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
    int                 barrier_maybe;
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
    uint8_t             unaligned:1;    // this opcode can be re-generated for unaligned special case
    uint8_t             x87precision:1; // this opcode can handle x87pc
    uint8_t             no_scratch_usage:1; // this opcode does not use scratch register
    uint8_t             nat_flags_op1;
    uint8_t             nat_flags_op2;
    uint16_t            nat_next_inst;
    flagcache_t         f_exit;     // flags status at end of instruction
    extcache_t          e;          // extcache at end of instruction (but before poping)
    flagcache_t         f_entry;    // flags status before the instruction begin
    uint8_t             vector_sew_entry; // sew status before the instruction begin
    uint8_t             vector_sew_exit;  // sew status at the end of instruction
} instruction_rv64_t;

typedef struct dynarec_rv64_s {
    instruction_rv64_t* insts;
    int32_t             size;
    int32_t             cap;
    uintptr_t           start;      // start of the block
    uintptr_t           end;        // maximum end of the block (only used in pass0)
    uint32_t            isize;      // size in byte of x64 instructions included
    void*               block;      // memory pointer where next instruction is emitted
    uintptr_t           native_start;  // start of the riscv code
    size_t              native_size;   // size of emitted riscv code
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
    int                 callret_size;   // size of the array
    callret_t*          callrets;   // arrey of callret return, with NOP / UDF depending if the block is clean or dirty
    uint8_t             smwrite;    // for strongmem model emulation
    uintptr_t           forward;    // address of the last end of code while testing forward
    uintptr_t           forward_to; // address of the next jump to (to check if everything is ok)
    int32_t             forward_size;   // size at the forward point
    int                 forward_ninst;  // ninst at the forward point
    uint16_t            ymm_zero;   // bitmap of ymm to zero at purge
    uint8_t             always_test;
    uint8_t             abort;
    uint8_t             vector_sew;     // current sew status
    uint8_t             vector_eew;     // current effective sew status, should only be used after SET_ELEMENT_WIDTH
    uint8_t             inst_sew;       // sew inside current instruction, for vsetvli elimination
    uint8_t             inst_vl;        // vl inside current instruction, for vsetvli elimination
    uint8_t             inst_vlmul;     // vlmul inside current instruction
    void*               gdbjit_block;
    uint32_t            need_x87check; // x87 low precision check
    uint32_t            need_dump;     // need to dump the block
    int                 need_reloc; // does the dynablock need relocations
    int                 reloc_size;
    uint32_t*           relocs;
    box64env_t*         env;
} dynarec_rv64_t;

// v0 is hardware wired to vector mask register, which should be always reserved
// convert idx (0..23) to reg index (10..31 1..2)
#define EXTREG(A) (((A) + 10 + ((A) > 21)) & 31)
// convert reg index (10..31 1..2) to idx (0..23)
#define EXTIDX(A) (((A) - 10 - ((A) < 3)) & 31)

void add_next(dynarec_rv64_t *dyn, uintptr_t addr);
uintptr_t get_closest_next(dynarec_rv64_t *dyn, uintptr_t addr);
void add_jump(dynarec_rv64_t *dyn, int ninst);
int get_first_jump(dynarec_rv64_t *dyn, int next);
int get_first_jump_addr(dynarec_rv64_t *dyn, uintptr_t next);
int is_nops(dynarec_rv64_t *dyn, uintptr_t addr, int n);

int isTable64(dynarec_rv64_t *dyn, uint64_t val); // return 1 if val already in Table64
int Table64(dynarec_rv64_t *dyn, uint64_t val, int pass);  // add a value to table64 (if needed) and gives back the imm19 to use in LDR_literal

void CreateJmpNext(void* addr, void* next);

#define GO_TRACE(A, B, s0)       \
    GETIP(addr, s0);             \
    MV(x1, xRIP);                \
    STORE_XEMU_CALL(s0);         \
    MOV64x(x2, B);               \
    CALL(const_##A, -1, x1, x2); \
    LOAD_XEMU_CALL()

#endif //__DYNAREC_RV64_PRIVATE_H_
