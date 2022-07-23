#ifndef __X86EMU_PRIVATE_H_
#define __X86EMU_PRIVATE_H_

#include "regs.h"

typedef struct box64context_s box64context_t;
typedef struct x64_ucontext_s x64_ucontext_t;

#define ERR_UNIMPL  1
#define ERR_DIVBY0  2
#define ERR_ILLEGAL 4

typedef struct forkpty_s {
    void*    amaster;
    void*   name;
    void*   termp;
    void*   winp;
    void*   f;  // forkpty function
} forkpty_t;

typedef union multiuint_s {
    uint8_t     u8;
    uint16_t    u16;
    uint32_t    u32;
    uint64_t    u64;
} multiuint_t;

typedef struct x64emu_s {
    // cpu
	reg64_t     regs[16];
	x64flags_t  eflags;
    reg64_t     ip;
    // sse
    sse_regs_t  xmm[16];
    // fpu / mmx
	mmx87_regs_t x87[8];
	mmx87_regs_t mmx[8];
	uint16_t    cw;
	x87flags_t  sw;
	uint32_t    top;        // top is part of sw, but it's faster to have it separatly
    int         fpu_stack;
    uint32_t    mxcsr;
	fpu_round_t round;
    fpu_ld_t    fpu_ld[8]; // for long double emulation / 80bits fld fst
    fpu_ll_t    fpu_ll[8]; // for 64bits fild / fist sequence
	fpu_p_reg_t p_regs[8];
    // old ip
    uintptr_t   old_ip;
    // defered flags
    int         dummy1;     // to align on 64bits with df
    defered_flags_t df;
    multiuint_t op1;
    multiuint_t op2;
    multiuint_t res;
    uint32_t    *x64emu_parity_tab; // helper
    #ifdef HAVE_TRACE
    reg64_t     oldregs[16];
    uintptr_t   prev2_ip;
    #endif
    // segments
    uint32_t    segs[6];        // only 32bits value?
    uintptr_t   segs_offs[6];   // computed offset associate with segment
    uint32_t    segs_serial[6];  // are seg offset clean (not 0) or does they need to be re-computed (0)? For GS, serial need to be the same as context->sel_serial
    // parent context
    box64context_t *context;
    // cpu helpers
    reg64_t     zero;
    reg64_t     *sbiidx[16];
    // emu control
    int         quit;
    int         error;
    int         fork;   // quit because need to fork
    forkpty_t*  forkpty_info;
    int         exit;
    int         quitonlongjmp;  // quit if longjmp is called
    int         quitonexit;     // quit if exit/_exit is called
    int         longjmp;        // if quit because of longjmp
    // scratch stack, used for alignement of double and 64bits ints on arm. 200 elements should be enough
    uint64_t    scratch[200];
    // local stack, do be deleted when emu is freed
    void*       stack2free; // this is the stack to free (can be NULL)
    void*       init_stack; // initial stack (owned or not)
    uint32_t    size_stack; // stack size (owned or not)

    x64_ucontext_t *uc_link; // to handle setcontext

    int         type;       // EMUTYPE_xxx define

} x64emu_t;

#define EMUTYPE_NONE    0
#define EMUTYPE_MAIN    1
#define EMUTYPE_SIGNAL  2

//#define INTR_RAISE_DIV0(emu) {emu->error |= ERR_DIVBY0; emu->quit=1;}
#define INTR_RAISE_DIV0(emu) {emu->error |= ERR_DIVBY0;} // should rise a SIGFPE and not quit

#endif //__X86EMU_PRIVATE_H_
