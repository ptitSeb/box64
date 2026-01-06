#ifndef __X86EMU_PRIVATE_H_
#define __X86EMU_PRIVATE_H_

#include "regs.h"
#include "os.h"
#include "box64context.h"

typedef struct x64_ucontext_s x64_ucontext_t;
#ifdef BOX32
typedef struct i386_ucontext_s i386_ucontext_t;
#endif

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
    int8_t      i8;
    uint16_t    u16;
    int16_t     i16;
    uint32_t    u32;
    int32_t     i32;
    uint64_t    u64;
    int64_t     i64;
} multiuint_t;

typedef struct x64emu_s x64emu_t;

typedef struct x64test_s {
    x64emu_t*   emu;
    x64emu_t*   ref;
    uintptr_t   memaddr;
    int         memsize;
    int         test;
    int         clean;
    uint8_t     mem[32];
} x64test_t;

#define FLAGS_NO_TF     7
typedef struct emu_flags_s {
    uint32_t    need_jmpbuf:1;    // need a new jmpbuff for signal handling
    uint32_t    quitonlongjmp:2;  // quit if longjmp is called
    uint32_t    quitonexit:2;     // quit if exit/_exit is called
    uint32_t    longjmp:1;        // if quit because of longjmp
    uint32_t    jmpbuf_ready:1;   // the jmpbuf in the emu is ok and don't need refresh
    uint32_t    no_tf:1;          // no TF on current opcode (to manage the delay of application of the flag)
} emu_flags_t;

#define N_SCRATCH 200

typedef struct x64emu_s {
    // cpu
    reg64_t     regs[16];
    x64flags_t  eflags;
    reg64_t     ip;
    // sse
    sse_regs_t  xmm[16];
    sse_regs_t  ymm[16];
    // fpu / mmx
    mmx87_regs_t x87[8];
    mmx87_regs_t mmx[8];
    x87flags_t  sw;
    uint32_t    top;        // top is part of sw, but it's faster to have it separately
    int         fpu_stack;
    x87control_t cw;
    uint16_t    dummy_cw;   // align...
    mmxcontrol_t mxcsr;
    #ifdef RV64         // it would be better to use a dedicated register for this like arm64 xSavedSP, but we're running out of free registers.
    uintptr_t xSPSave;  // sp base value of current dynarec frame, used by call/ret optimization to reset stack when unmatch.
    #endif
    fpu_ld_t    fpu_ld[8]; // for long double emulation / 80bits fld fst
    fpu_ll_t    fpu_ll[8]; // for 64bits fild / fist sequence
    uint64_t    fpu_tags;   // tags for the x87 regs, stacked, only on a 16bits anyway
    // old ip
    uintptr_t   old_ip;
    // deferred flags
    int         dummy1;     // to align on 64bits with df
    deferred_flags_t df;
    multiuint_t op1;
    multiuint_t op2;
    multiuint_t res;
    uint32_t    *x64emu_parity_tab; // helper
    // segments
    uint16_t    segs[6];        // only 32bits value?
    uint16_t    dummy_seg6, dummy_seg7; // to stay aligned
    uintptr_t   segs_offs[6];   // computed offset associate with segment
    // parent context
    box64context_t *context;
    // cpu helpers
    reg64_t     zero;
    reg64_t     *sbiidx[16];
    // emu control
    int         quit;
    int         error;
    int         fork;   // quit because need to fork
    int         exit;
    forkpty_t*  forkpty_info;
    emu_flags_t flags;
    x64test_t   test;       // used for dynarec testing
    // scratch stack, used for alignment of double and 64bits ints on arm. 200 elements should be enough
    __int128_t dummy_align; // here to have scratch 128bits aligned
    uint64_t scratch[N_SCRATCH];

    // Warning, offsetof(x64emu_t, xxx) will be too big for fields below.
    #ifdef HAVE_TRACE
    sse_regs_t  old_xmm[16];
    sse_regs_t  old_ymm[16];
    reg64_t     oldregs[16];
    uintptr_t   prev2_ip;
    #endif
    // local stack, do be deleted when emu is freed
    void*       stack2free; // this is the stack to free (can be NULL)
    void*       init_stack; // initial stack (owned or not)
    uint32_t    size_stack; // stack size (owned or not)
    JUMPBUFF*   jmpbuf;
    #ifdef RV64
    uintptr_t   old_savedsp;
    #endif

    #ifdef _WIN32
    uint64_t    win64_teb;
    #endif
    // local selector handling
    base_segment_t  segldt[16];
    base_segment_t  seggdt[16];  // hacky
    tlsdatasize_t  *tlsdata;
    // other informations
    int         type;       // EMUTYPE_xxx define
    #ifdef BOX32
    int         libc_err;   // copy of errno from libc
    int         libc_herr;  // copy of h_errno from libc
    unsigned short          libctype[384];   // copy from __ctype_b address might be too high
    const unsigned short*   ref_ctype;
    const unsigned short*   ctype;
    int         libctolower[384];   // copy from __ctype_b_tolower address might be too high
    const int*  ref_tolower;
    const int*  tolower;
    int         libctoupper[384];   // copy from __ctype_b_toupper address might be too high
    const int*  ref_toupper;
    const int*  toupper;
    void*       res_state_32;  //32bits version of res_state
    void*       res_state_64;
    #endif
} x64emu_t;

#define EMUTYPE_NONE    0
#define EMUTYPE_MAIN    1
#define EMUTYPE_SIGNAL  2

//#define INTR_RAISE_DIV0(emu) {emu->error |= ERR_DIVBY0; emu->quit=1;}
#define INTR_RAISE_DIV0(emu) {emu->error |= ERR_DIVBY0;} // should rise a SIGFPE and not quit

void applyFlushTo0(x64emu_t* emu);

#endif //__X86EMU_PRIVATE_H_
