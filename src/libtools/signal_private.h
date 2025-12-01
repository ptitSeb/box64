#ifndef __SIGNAL_PRIVATE_H__
#define __SIGNAL_PRIVATE_H__

typedef uint64_t x64_gregset_t[23];
enum
{
  X64_R8 = 0,
# define X64_R8         X64_R8
  X64_R9,
# define X64_R9         X64_R9
  X64_R10,
# define X64_R10        X64_R10
  X64_R11,
# define X64_R11        X64_R11
  X64_R12,
# define X64_R12        X64_R12
  X64_R13,
# define X64_R13        X64_R13
  X64_R14,
# define X64_R14        X64_R14
  X64_R15,
# define X64_R15        X64_R15
  X64_RDI,
# define X64_RDI        X64_RDI
  X64_RSI,
# define X64_RSI        X64_RSI
  X64_RBP,
# define X64_RBP        X64_RBP
  X64_RBX,
# define X64_RBX        X64_RBX
  X64_RDX,
# define X64_RDX        X64_RDX
  X64_RAX,
# define X64_RAX        X64_RAX
  X64_RCX,
# define X64_RCX        X64_RCX
  X64_RSP,
# define X64_RSP        X64_RSP
  X64_RIP,
# define X64_RIP        X64_RIP
  X64_EFL,
# define X64_EFL        X64_EFL
  X64_CSGSFS,           /* Actually short cs, gs, fs, ss.  */
# define X64_CSGSFS     X64_CSGSFS
  X64_ERR,
# define X64_ERR        X64_ERR
  X64_TRAPNO,
# define X64_TRAPNO     X64_TRAPNO
  X64_OLDMASK,
# define X64_OLDMASK    X64_OLDMASK
  X64_CR2
# define X64_CR2        X64_CR2
};

struct x64_fpreg
{
  uint64_t value;
}__attribute__((packed));

struct x64_fpxreg
{
  unsigned short significand[4];
  unsigned short exponent;
  unsigned short padding[3];
}__attribute__((packed));

struct x64_xmmreg
{
  uint32_t          element[4];
}__attribute__((packed));

struct x64_fpstate
{
  /* Regular FPU environment.  */
  uint16_t          cw;
  uint16_t          sw;
  uint16_t          tw;
  uint16_t          fop;
  uint64_t          rip;
  uint64_t          rdp;
  uint32_t          mxcsr;
  uint32_t          mxcsr_mask;
  struct x64_fpreg  _st[8];
  struct x64_xmmreg _xmm[16];
  uint32_t          res[24];
}__attribute__((packed));

typedef struct x64_fpstate *x64_fpregset_t;

typedef struct x64_stack_s
{
    void *ss_sp;
    int ss_flags;
    size_t ss_size;
} x64_stack_t;

struct sigcontext_x64
{
    uint64_t    r8;
    uint64_t    r9;
    uint64_t    r10;
    uint64_t    r11;
    uint64_t    r12;
    uint64_t    r13;
    uint64_t    r14;
    uint64_t    r15;
    uint64_t    di;
    uint64_t    si;
    uint64_t    bp;
    uint64_t    bx;
    uint64_t    dx;
    uint64_t    ax;
    uint64_t    cx;
    uint64_t    sp;
    uint64_t    ip;
    uint64_t    flags;
    uint64_t    cs;
    uint64_t    gs;
    uint64_t    fs;
    uint64_t    ss;
    uint64_t    err;
    uint64_t    trapno;
    uint64_t    oldmask;
    uint64_t    cr2;
    uint64_t    fpstate; /* Zero when no FPU/extended context */
    uint64_t    reserved1[8];
};

struct x64_sigcontext
{
    uint64_t    r8;
    uint64_t    r9;
    uint64_t    r10;
    uint64_t    r11;
    uint64_t    r12;
    uint64_t    r13;
    uint64_t    r14;
    uint64_t    r15;
    uint64_t    rdi;
    uint64_t    rsi;
    uint64_t    rbp;
    uint64_t    rbx;
    uint64_t    rdx;
    uint64_t    rax;
    uint64_t    rcx;
    uint64_t    rsp;
    uint64_t    rip;
    uint64_t    eflags;         /* RFLAGS */
    uint16_t    cs;
    uint16_t    gs;
    uint16_t    fs;
    union {
        uint16_t    ss;     /* If UC_SIGCONTEXT_SS */
        uint16_t    __pad0; /* Alias name for old (!UC_SIGCONTEXT_SS) user-space */
    };
    uint64_t    err;
    uint64_t    trapno;
    uint64_t    oldmask;
    uint64_t    cr2;
    struct x64_fpstate  *fpstate;       /* Zero when no FPU context */
    uint64_t    reserved1[8];
};

struct x64_libc_fpstate
{
  /* 64-bit FXSAVE format.  */
  uint16_t              cwd;
  uint16_t              swd;
  uint16_t              ftw;
  uint16_t              fop;
  uint64_t              rip;
  uint64_t              rdp;
  uint32_t              mxcsr;
  uint32_t              mxcr_mask;
  struct x64_fpxreg     st[8];
  struct x64_xmmreg     xmm[16];
  uint32_t              res1[24];
};

typedef struct x64_mcontext_s
{
    x64_gregset_t gregs;
    struct x64_libc_fpstate *fpregs;
    uint64_t    res[8];
} x64_mcontext_t;

// /!\ signal sig_set is different than glibc __sig_set
#ifndef ANDROID
#define _NSIG_WORDS (1024 / (sizeof(unsigned long int)*8))
#endif

typedef struct {
    unsigned long int sig[_NSIG_WORDS];
} x64_sigset_t;

typedef struct x64_ucontext_s
{
    uint64_t                uc_flags;
    struct x64_ucontext_s*  uc_link;
    x64_stack_t             uc_stack;
    x64_mcontext_t          uc_mcontext;
    x64_sigset_t            uc_sigmask;
    struct x64_libc_fpstate xstate;
    #ifndef ANDROID
    uint64_t                ssp[4];
    #endif
} x64_ucontext_t;

typedef struct x64_sigframe_s {
    uintptr_t       pretcode;   // pointer to retcode
    int             sig;
    x64_mcontext_t cpustate;
    struct x64_libc_fpstate xstate;
    uintptr_t       extramask[64-1];
    char            retcode[8];
} x64_sigframe_t;

struct kernel_sigaction {
        void (*k_sa_handler) (int);
        unsigned long sa_flags;
        void (*sa_restorer) (void);
        unsigned long sa_mask;
        unsigned long sa_mask2;
};

x64_stack_t* sigstack_getstack();
uint64_t RunFunctionHandler(x64emu_t* emu, int* exit, int dynarec, x64_ucontext_t* sigcontext, uintptr_t fnc, int nargs, ...);

#endif //__SIGNAL_PRIVATE_H__