#ifndef __DYNAREC_ARM_PRIVATE_H_
#define __DYNAREC_ARM_PRIVATE_H_

#include "../dynarec_private.h"

typedef struct x64emu_s x64emu_t;
typedef struct dynablock_s dynablock_t;
typedef struct instsize_s instsize_t;
typedef struct box64env_s box64env_t;

#define BARRIER_MAYBE   8

#define NF_EQ   (1<<0)
#define NF_SF   (1<<1)
#define NF_VF   (1<<2)
#define NF_CF   (1<<3)

// Nothing happens to the native flags
#define NAT_FLAG_OP_NONE        0
// Native flags are touched on this opcode
#define NAT_FLAG_OP_TOUCH       1
// Native flags are destroyed and unusable
#define NAT_FLAG_OP_UNUSABLE    2
// Native flags usaged are canceled here
#define NAT_FLAG_OP_CANCELED    3

#define NEON_CACHE_NONE     0
#define NEON_CACHE_ST_D     1
#define NEON_CACHE_ST_F     2
#define NEON_CACHE_ST_I64   3
#define NEON_CACHE_MM       4
#define NEON_CACHE_XMMW     5
#define NEON_CACHE_XMMR     6
#define NEON_CACHE_YMMW     7
#define NEON_CACHE_YMMR     8
#define NEON_CACHE_SCR      9
typedef union neon_cache_s {
    int8_t           v;
    struct {
        uint8_t t:4;   // reg type
        uint8_t n:4;   // reg number
    };
} neon_cache_t;
typedef union sse_cache_s {
    int8_t      v;
    struct {
        uint8_t reg:7;
        uint8_t write:1;
    };
} sse_cache_t;
typedef struct callret_s callret_t;
typedef struct neoncache_s {
    // Neon cache
    neon_cache_t        neoncache[32];
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
    // fpu cache
    int8_t              x87cache[8];    // cache status for the 8 x87 register behind the fpu stack
    int8_t              x87reg[8];      // reg used for x87cache entry
    int16_t             tags;           // similar to fpu_tags
    int8_t              mmxcache[8];    // cache status for the 8 MMX registers
    sse_cache_t         ssecache[16];   // cache status for the 16 SSE(2) registers
    int8_t              fpuused[32];    // all neon regs, used by x87, mmx, sse and avx
    int8_t              x87stack;       // cache stack counter
    int8_t              mmxcount;       // number of mmx register used (not both mmx and x87 at the same time)
    int8_t              fpu_scratch;    // scratch counter
    uint16_t            xmm_write;      // 1bit of xmmXX removed write
    uint16_t            xmm_removed;    // 1bit if xmmXX was removed
    uint16_t            xmm_used;       // mask of the xmm regs used in this opcode
    uint16_t            ymm_used;       // mask of the ymm regs used in this opcode
    uint16_t            ymm_write;      // 1bit of ymmXX removed write
    uint16_t            ymm_removed;    // 1bit if ymmXX was removed
    uint16_t            xmm_unneeded;   // 1bit for xmmXX were value is not needed
    uint16_t            ymm_unneeded;   // 1bit for ymmXX were value is not needed 
    uint64_t            ymm_regs;       // 4bits (0-15) position of 16 ymmXX regs removed
} neoncache_t;

typedef struct flagcache_s {
    int                 pending;    // is there a pending flags here, or to check?
    uint8_t             dfnone;     // if deferred flags is already set to df_none
} flagcache_t;

typedef struct instruction_arm64_s {
    instruction_x64_t   x64;
    uintptr_t           address;    // (start) address of the arm emitted instruction
    uintptr_t           epilog;     // epilog of current instruction (can be start of next, or barrier stuff)
    int                 size;       // size of the arm emitted instruction
    int                 size2;      // size of the arm emitted instrucion after pass2
    int                 pred_sz;    // size of predecessor list
    int                 *pred;      // predecessor array
    uintptr_t           mark, mark2, mark3;
    uintptr_t           markf, markf2;
    uintptr_t           markseg;
    uintptr_t           marklock;
    int                 pass2choice;// value for choices that are fixed on pass2 for pass3
    uintptr_t           natcall;
    uint16_t            retn;
    uint16_t            purge_ymm;  // need to purge some ymm
    uint16_t            ymm0_in;    // bitmap of ymm to zero at purge
    uint16_t            ymm0_add;   // the ymm0 added by the opcode
    uint16_t            ymm0_sub;   // the ymm0 removed by the opcode
    uint16_t            ymm0_out;   // the ymm0 at th end of the opcode
    uint16_t            ymm0_pass2, ymm0_pass3;
    uint8_t             barrier_maybe;
    uint8_t             will_write:2;    // [strongmem] will write to memory
    uint8_t             will_read:1;     // [strongmem] will read from memory
    uint8_t             last_write:1;    // [strongmem] the last write in a SEQ
    uint8_t             lock:1;          // [strongmem] lock semantic
    uint8_t             wfe:1;        // opcode uses sevl + wfe
    uint8_t             set_nat_flags;  // 0 or combinaison of native flags define
    uint8_t             use_nat_flags;  // 0 or combinaison of native flags define
    uint8_t             use_nat_flags_before;  // 0 or combinaison of native flags define
    uint8_t             nat_flags_op:4;// what happens to native flags here
    uint8_t             nat_flags_op_before:4;// what happens to native flags here
    uint8_t             before_nat_flags;  // 0 or combinaison of native flags define
    uint8_t             need_nat_flags;
    unsigned            gen_inverted_carry:1;
    unsigned            normal_carry:1;
    unsigned            normal_carry_before:1;
    unsigned            invert_carry:1; // this opcode force an inverted carry
    unsigned            df_notneeded:1;
    unsigned            unaligned:1;    // this opcode can be re-generated for unaligned special case
    unsigned            x87precision:1; // this opcode can handle x87pc
    unsigned            mmx_used:1; // no fine tracking, just a global "any reg used"
    unsigned            x87_used:1; // no fine tracking, just a global "any reg used"
    unsigned            fpu_used:1; // any xmm/ymm/x87/mmx reg used
    unsigned            fpupurge:1;   // this opcode will purge all fpu regs
    flagcache_t         f_exit;     // flags status at end of instruction
    neoncache_t         n;          // neoncache at end of instruction (but before poping)
    flagcache_t         f_entry;    // flags status before the instruction begin
} instruction_arm64_t;

typedef struct dynarec_arm_s {
    instruction_arm64_t*insts;
    int32_t             size;
    int32_t             cap;
    uintptr_t           start;      // start of the block
    uintptr_t           end;        // maximum end of the block (only used in pass0)
    uint32_t            isize;      // size in bytes of x64 instructions included
    void*               block;      // memory pointer where next instruction is emitted
    uintptr_t           native_start;  // start of the arm code
    size_t              native_size;   // size of emitted arm code
    uintptr_t           last_ip;    // last set IP in RIP (or NULL if unclean state) TODO: move to a cache something
    uint64_t*           table64;    // table of 64bits values
    int                 table64size;// size of table (will be appended at end of executable code)
    int                 table64cap;
    uintptr_t           tablestart;
    uintptr_t           jmp_next;   // address of the jump_next address
    flagcache_t         f;
    neoncache_t         n;          // cache for the 8..31 double reg from fpu, plus x87 stack delta
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
    uintptr_t           forward;    // address of the last end of code while testing forward
    uintptr_t           forward_to; // address of the next jump to (to check if everything is ok)
    int32_t             forward_size;   // size at the forward point
    int                 forward_ninst;  // ninst at the forward point
    uint16_t            ymm_zero;   // bitmap of ymm to zero at purge
    uint8_t             smwrite;    // for strongmem model emulation
    uint8_t             doublepush;
    uint8_t             doublepop;
    uint8_t             always_test;
    uint8_t             abort;      // abort the creation of the block
    uint8_t             use_x87:1;  // set if x87 regs are used
    uint8_t             use_mmx:1;
    uint8_t             use_xmm:1;
    uint8_t             use_ymm:1;
    void*               gdbjit_block;
    uint32_t            need_x87check;  // needs x87 precision control check if non-null, or 0 if not
    uint32_t            need_dump;     // need to dump the block
    int                 need_reloc; // does the dynablock need relocations
    int                 reloc_size;
    uint32_t*           relocs;
    box64env_t*         env;
} dynarec_arm_t;

void add_next(dynarec_arm_t *dyn, uintptr_t addr);
uintptr_t get_closest_next(dynarec_arm_t *dyn, uintptr_t addr);
void add_jump(dynarec_arm_t *dyn, int ninst);
int get_first_jump(dynarec_arm_t *dyn, int next);
int get_first_jump_addr(dynarec_arm_t *dyn, uintptr_t next);
int is_nops(dynarec_arm_t *dyn, uintptr_t addr, int n);

int isTable64(dynarec_arm_t *dyn, uint64_t val); // return 1 if val already in Table64
int Table64(dynarec_arm_t *dyn, uint64_t val, int pass);  // add a value to table64 (if needed) and gives back the imm19 to use in LDR_literal

void CreateJmpNext(void* addr, void* next);

#define GO_TRACE(A, B, s0)      \
    GETIP(addr);                \
    MOVx_REG(x1, xRIP);         \
    MRS_nzcv(s0);               \
    STORE_XEMU_CALL(xRIP);      \
    MOV32w(x2, B);              \
    CALL_(const_##A, -1, s0);   \
    MSR_nzcv(s0);               \
    LOAD_XEMU_CALL(xRIP)

#endif //__DYNAREC_ARM_PRIVATE_H_
