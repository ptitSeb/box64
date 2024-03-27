#ifndef __X86EMU_PRIVATE_H_
#define __X86EMU_PRIVATE_H_

#include "regs.h"

typedef struct box64context_s box64context_t;
typedef struct x64_ucontext_s x64_ucontext_t;

#define ERR_UNIMPL  1
#define ERR_DIVBY0  2
#define ERR_ILLEGAL 4

typedef struct forkpty_s {
    void* amaster;
    void* name;
    void* termp;
    void* winp;
    void* f; // forkpty function
} forkpty_t;

typedef union multiuint_s {
    uint8_t u8;
    int8_t i8;
    uint16_t u16;
    int16_t i16;
    uint32_t u32;
    int32_t i32;
    uint64_t u64;
    int64_t i64;
} multiuint_t;

typedef struct x64emu_s x64emu_t;

typedef struct x64test_s {
    x64emu_t* emu;
    uintptr_t memaddr;
    int memsize;
    int test;
    int clean;
    uint8_t mem[16];
} x64test_t;

typedef struct emu_flags_s {
    uint32_t need_jmpbuf : 1;   // need a new jmpbuff for signal handling
    uint32_t quitonlongjmp : 2; // quit if longjmp is called
    uint32_t quitonexit : 2;    // quit if exit/_exit is called
    uint32_t longjmp : 1;       // if quit because of longjmp
    uint32_t jmpbuf_ready : 1;  // the jmpbuf in the emu is ok and don't need refresh
} emu_flags_t;

#ifdef ANDROID
#include <setjmp.h>
#define JUMPBUFF sigjmp_buf
#else
#define JUMPBUFF struct __jmp_buf_tag
#endif

typedef struct x64emu_s {
    // cpu
    reg64_t regs[16];
    x64flags_t eflags;
    reg64_t ip;
    // sse
    sse_regs_t xmm[16];
    // fpu / mmx
    mmx87_regs_t x87[8];
    mmx87_regs_t mmx[8];
    x87flags_t sw;
    uint32_t top; // top is part of sw, but it's faster to have it separately
    int fpu_stack;
    x87control_t cw;
    uint16_t dummy_cw; // align...
    mmxcontrol_t mxcsr;
#ifdef RV64            // it would be better to use a dedicated register for this like arm64 xSavedSP, but we're running of of free registers.
    uintptr_t xSPSave; // sp base value of current dynarec frame, used by call/ret optimization to reset stack when unmatch.
#endif
    fpu_ld_t fpu_ld[8]; // for long double emulation / 80bits fld fst
    fpu_ll_t fpu_ll[8]; // for 64bits fild / fist sequence
    fpu_p_reg_t p_regs[8];
    // old ip
    uintptr_t old_ip;
    // deferred flags
    int dummy1; // to align on 64bits with df
    deferred_flags_t df;
    multiuint_t op1;
    multiuint_t op2;
    multiuint_t res;
    multiuint_t op1_sav; // for dec/inc deferred flags, to be able to compute CF
    multiuint_t res_sav;
    deferred_flags_t df_sav;
    uint32_t* x64emu_parity_tab; // helper
#ifdef HAVE_TRACE
    reg64_t oldregs[16];
    uintptr_t prev2_ip;
#endif
    // segments
    uint16_t segs[6];                // only 32bits value?
    uint16_t dummy_seg6, dummy_seg7; // to stay aligned
    uintptr_t segs_offs[6];          // computed offset associate with segment
    uint32_t segs_serial[6];         // are seg offset clean (not 0) or does they need to be re-computed (0)? For GS, serial need to be the same as context->sel_serial
    // parent context
    box64context_t* context;
    // cpu helpers
    reg64_t zero;
    reg64_t* sbiidx[16];
    // emu control
    int quit;
    int error;
    int fork; // quit because need to fork
    int exit;
    forkpty_t* forkpty_info;
    emu_flags_t flags;
    x64test_t test; // used for dynarec testing
#ifdef HAVE_TRACE
    sse_regs_t old_xmm[16];
#endif
    // scratch stack, used for alignment of double and 64bits ints on arm. 200 elements should be enough
    uint64_t scratch[200];
    // local stack, do be deleted when emu is freed
    void* stack2free;    // this is the stack to free (can be NULL)
    void* init_stack;    // initial stack (owned or not)
    uint32_t size_stack; // stack size (owned or not)
    JUMPBUFF* jmpbuf;
#ifdef RV64
    uintptr_t old_savedsp;
#endif

    x64_ucontext_t* uc_link; // to handle setcontext

    int type; // EMUTYPE_xxx define
} x64emu_t;

#define EMUTYPE_NONE   0
#define EMUTYPE_MAIN   1
#define EMUTYPE_SIGNAL 2

// #define INTR_RAISE_DIV0(emu) {emu->error |= ERR_DIVBY0; emu->quit=1;}
#define INTR_RAISE_DIV0(emu)      \
    {                             \
        emu->error |= ERR_DIVBY0; \
    } // should rise a SIGFPE and not quit

void applyFlushTo0(x64emu_t* emu);

#endif //__X86EMU_PRIVATE_H_
