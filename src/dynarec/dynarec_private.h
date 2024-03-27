#ifndef __DYNAREC_PRIVATE_H_
#define __DYNAREC_PRIVATE_H_

#define BARRIER_NONE  0
#define BARRIER_FLAGS 1
#define BARRIER_FLOAT 2
#define BARRIER_FULL  3

// all flags for the use_flags field
#define X_CF   (1 << 0)
#define X_PF   (1 << 1)
#define X_AF   (1 << 2)
#define X_ZF   (1 << 3)
#define X_SF   (1 << 4)
#define X_OF   (1 << 5)
#define X_ALL  ((1 << 6) - 1)
#define X_PEND (0x80)

// all state flags
#define SF_UNKNOWN        0
#define SF_SET            1
#define SF_PENDING        2
#define SF_SET_PENDING    (SF_SET | SF_PENDING)
#define SF_SUB            4
#define SF_SUBSET         (SF_SUB | SF_SET)
#define SF_SUBSET_PENDING (SF_SUBSET | SF_PENDING)

typedef struct instruction_x64_s {
    uintptr_t addr;       // address of the instruction
    int32_t size;         // size of the instruction
    uintptr_t jmp;        // offset to jump to, even if conditionnal (0 if not), no relative offset here
    int jmp_insts;        // instuction to jump to (-1 if out of the block)
    uint8_t jmp_cond;     // 1 of conditionnal jump
    uint8_t has_next;     // does this opcode can continue to the next?
    uint8_t barrier;      // next instruction is a jump point, so no optim allowed
    uint8_t barrier_next; // next instruction needs a barrier
    uint8_t has_callret;  // this instruction have an optimised call setup
    uint8_t state_flags;  // One of SF_XXX state
    uint8_t use_flags;    // 0 or combination of X_?F
    uint8_t set_flags;    // 0 or combination of X_?F
    uint8_t may_set;      // 1 if the flags may not be set
    uint8_t gen_flags;    // calculated
    uint8_t need_before;  // calculated
    uint8_t need_after;   // calculated
} instruction_x64_t;

void printf_x64_instruction(zydis_dec_t* dec, instruction_x64_t* inst, const char* name);

#endif //__DYNAREC_PRIVATE_H_
