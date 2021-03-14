#ifndef __DYNAREC_PRIVATE_H_
#define __DYNAREC_PRIVATE_H_

// all flags for the use_flags field
#define X_CF    (1<<0)
#define X_PF    (1<<1)
#define X_AF    (1<<2)
#define X_ZF    (1<<3)
#define X_SF    (1<<4)
#define X_OF    (1<<5)
#define X_ALL   ((1<<6)-1)
#define X_PEND  (0x1000)

// all state flags
#define SF_UNKNOWN  0
#define SF_SET      1
#define SF_PENDING  2
#define SF_MAYSET   3
#define SF_SUBSET   4

typedef struct instruction_x64_s {
    uintptr_t   addr;   //address of the instruction
    int32_t     size;   // size of the instruction
    int         barrier; // next instruction is a jump point, so no optim allowed
    uintptr_t   jmp;    // offset to jump to, even if conditionnal (0 if not), no relative offset here
    int         jmp_insts;  // instuction to jump to (-1 if out of the block)
    uint32_t    use_flags;  // 0 or combination of X_?F
    uint32_t    set_flags;  // 0 or combination of X_?F
    uint32_t    need_flags; // calculated
    int         state_flags; // One of SF_XXX state
} instruction_x64_t;

void printf_x64_instruction(zydis_dec_t* dec, instruction_x64_t* inst, const char* name);

#endif //__DYNAREC_PRIVATE_H_
