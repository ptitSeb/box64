#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <syscall.h>
#include <stddef.h>
#include <stdarg.h>
#include <ucontext.h>
#include <setjmp.h>
#include <sys/mman.h>

#include "box64context.h"
#include "debug.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "signals.h"
#include "box64stack.h"
#include "dynarec.h"
#include "callback.h"
#include "x64run.h"
#include "elfloader.h"
#include "threads.h"
#include "emu/x87emu_private.h"
#include "custommem.h"
#ifdef DYNAREC
#include "dynablock.h"
#include "../dynarec/dynablock_private.h"
#endif


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
  X64_CSGSFS,           /* Actually short cs, gs, fs, __pad0.  */
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
  uint32_t          res[12];
  uint32_t          res2[12];
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
#define _NSIG_WORDS (128 / sizeof(unsigned long int))

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
    uint64_t                ssp[4];
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

static void sigstack_destroy(void* p)
{
	x64_stack_t *ss = (x64_stack_t*)p;
    free(ss);
}

static pthread_key_t sigstack_key;
static pthread_once_t sigstack_key_once = PTHREAD_ONCE_INIT;

static void sigstack_key_alloc() {
	pthread_key_create(&sigstack_key, sigstack_destroy);
}

uint64_t RunFunctionHandler(int* exit, uintptr_t fnc, int nargs, ...)
{
    if(fnc==0 || fnc==1) {
        printf_log(LOG_NONE, "BOX64: Warning, calling Signal function handler %s\n", fnc?"SIG_DFL":"SIG_IGN");
        return 0;
    }
#ifdef HAVE_TRACE
    uintptr_t old_start = trace_start, old_end = trace_end;
#if 0
    trace_start = 0; trace_end = 1; // disabling trace, globably for now...
#endif
#endif

    x64emu_t *emu = thread_get_emu();

    printf_log(LOG_DEBUG, "%04d|signal function handler %p called, RSP=%p\n", GetTID(), (void*)fnc, (void*)R_RSP);
    
    /*SetFS(emu, default_fs);*/
    for (int i=0; i<6; ++i)
        emu->segs_serial[i] = 0;
        
    if(nargs>6)
        R_RSP -= (nargs-6)*4;   // need to push in reverse order

    uint64_t *p = (uint64_t*)R_RSP;

    va_list va;
    va_start (va, nargs);
    for (int i=0; i<nargs; ++i) {
        if(i<6) {
            int nn[] = {_DI, _SI, _DX, _CX, _R8, _R9};
            emu->regs[nn[i]].q[0] = va_arg(va, uint64_t);
        } else {
            *p = va_arg(va, uint64_t);
            p++;
        }
    }
    va_end (va);

    EmuCall(emu, fnc);  // avoid DynaCall for now
    if(nargs>6)
        R_RSP+=((nargs-6)*4);

    if(exit)
        *exit = emu->exit;

    uint64_t ret = R_RAX;

#ifdef HAVE_TRACE
    trace_start = old_start; trace_end = old_end;
#endif

    return ret;
}

EXPORT int my_sigaltstack(x64emu_t* emu, const x64_stack_t* ss, x64_stack_t* oss)
{
    if(!ss && !oss) {   // this is not true, ss can be NULL to retreive oss info only
        errno = EFAULT;
        return -1;
    }
	x64_stack_t *new_ss = (x64_stack_t*)pthread_getspecific(sigstack_key);
    if(!ss) {
        if(!new_ss) {
            oss->ss_flags = SS_DISABLE;
            oss->ss_sp = emu->init_stack;
            oss->ss_size = emu->size_stack;
        } else {
            oss->ss_flags = new_ss->ss_flags;
            oss->ss_sp = new_ss->ss_sp;
            oss->ss_size = new_ss->ss_size;
        }
        return 0;
    }
    printf_log(LOG_DEBUG, "%04d|sigaltstack called ss=%p[flags=0x%x, sp=%p, ss=0x%lx], oss=%p\n", GetTID(), ss, ss->ss_flags, ss->ss_sp, ss->ss_size, oss);
    if(ss->ss_flags && ss->ss_flags!=SS_DISABLE && ss->ss_flags!=SS_ONSTACK) {
        errno = EINVAL;
        return -1;
    }

    if(ss->ss_flags==SS_DISABLE) {
        if(new_ss)
            free(new_ss);
        pthread_setspecific(sigstack_key, NULL);

        return 0;
    }

    if(oss) {
        if(!new_ss) {
            oss->ss_flags = SS_DISABLE;
            oss->ss_sp = emu->init_stack;
            oss->ss_size = emu->size_stack;
        } else {
            oss->ss_flags = new_ss->ss_flags;
            oss->ss_sp = new_ss->ss_sp;
            oss->ss_size = new_ss->ss_size;
        }
    }
    if(!new_ss)
        new_ss = (x64_stack_t*)calloc(1, sizeof(x64_stack_t));
    new_ss->ss_sp = ss->ss_sp;
    new_ss->ss_size = ss->ss_size;

	pthread_setspecific(sigstack_key, new_ss);

    return 0;
}


void my_sighandler(int32_t sig)
{
    int Locks = unlockMutex();
    printf_log(LOG_DEBUG, "Sighanlder for signal #%d called (jump to %p)\n", sig, (void*)my_context->signals[sig]);
    // save values
    x64emu_t *emu = thread_get_emu();
    uintptr_t restorer = my_context->restorer[sig];
    uintptr_t old_regs[16] = {0};
    uintptr_t old_ip = emu->ip.q[0];
    uint64_t old_flags = emu->eflags.x64;
    for(int i=0; i<16; ++i)
        old_regs[i] = emu->regs[i].q[0];

    x64_stack_t *new_ss = (x64_stack_t*)pthread_getspecific(sigstack_key);
    if(new_ss) {
        // no alternate stack, so signal RSP needs to match thread RSP!
        R_RSP = (uintptr_t)(new_ss->ss_sp+new_ss->ss_size-32);
    }

    int exits = 0;
    int ret = RunFunctionHandler(&exits, my_context->signals[sig], 1, sig);
    // restore regs
    emu->ip.q[0] = old_ip;
    emu->eflags.x64 = old_flags;
    for(int i=0; i<16; ++i)
        emu->regs[i].q[0] = old_regs[i];
    if(exits) {
        relockMutex(Locks);
        exit(ret);
    }
    // what about the restored regs?
    if(restorer)
        RunFunctionHandler(&exits, restorer, 0);
    relockMutex(Locks);
}

#ifdef DYNAREC
uintptr_t getX64Address(dynablock_t* db, uintptr_t arm_addr)
{
    uintptr_t x64addr = (uintptr_t)db->x64_addr;
    uintptr_t armaddr = (uintptr_t)db->block;
    int i = 0;
    do {
        int x64sz = 0;
        int armsz = 0;
        do {
            x64sz+=db->instsize[i].x64;
            armsz+=db->instsize[i].nat*4;
        }
        while(!db->instsize[++i].x64);
        if(arm_addr>=armaddr && arm_addr<(armaddr+armsz))
            return x64addr;
        armaddr+=armsz;
        x64addr+=x64sz;
        if(arm_addr==armaddr)
            return x64addr;
    } while(db->instsize[i].x64 || db->instsize[i].nat);
    return x64addr;
}
#endif

void my_sigactionhandler_oldcode(int32_t sig, siginfo_t* info, void * ucntx, int* old_code, void* cur_db)
{
    int Locks = unlockMutex();

    printf_log(LOG_DEBUG, "Sigactionhanlder for signal #%d called (jump to %p/%s)\n", sig, (void*)my_context->signals[sig], GetNativeName((void*)my_context->signals[sig]));

    uintptr_t restorer = my_context->restorer[sig];
    // get that actual ESP first!
    x64emu_t *emu = thread_get_emu();
    uintptr_t *frame = (uintptr_t*)R_RSP;
#if defined(DYNAREC) && defined(__aarch64__)
    ucontext_t *p = (ucontext_t *)ucntx;
    void * pc = (void*)p->uc_mcontext.pc;
    dynablock_t* db = (dynablock_t*)cur_db;//FindDynablockFromNativeAddress(pc);
    if(db) {
        frame = (uintptr_t*)p->uc_mcontext.regs[10+_SP];
    }
#else
    (void)ucntx; (void)cur_db;
#endif
    // stack tracking
	x64_stack_t *new_ss = my_context->onstack[sig]?(x64_stack_t*)pthread_getspecific(sigstack_key):NULL;
    int used_stack = 0;
    if(new_ss) {
        if(new_ss->ss_flags == SS_ONSTACK) { // already using it!
            frame = (uintptr_t*)emu->regs[_SP].q[0];
        } else {
            frame = (uintptr_t*)(((uintptr_t)new_ss->ss_sp + new_ss->ss_size - 16) & ~0x0f);
            used_stack = 1;
            new_ss->ss_flags = SS_ONSTACK;
        }
    }

    // TODO: do I need to really setup 2 stack frame? That doesn't seems right!
    // setup stack frame
    // try to fill some sigcontext....
    frame -= sizeof(x64_ucontext_t)/sizeof(uintptr_t);
    x64_ucontext_t   *sigcontext = (x64_ucontext_t*)frame;
    // get general register
    sigcontext->uc_mcontext.gregs[X64_R8] = R_R8;
    sigcontext->uc_mcontext.gregs[X64_R9] = R_R9;
    sigcontext->uc_mcontext.gregs[X64_R10] = R_R10;
    sigcontext->uc_mcontext.gregs[X64_R11] = R_R11;
    sigcontext->uc_mcontext.gregs[X64_R12] = R_R12;
    sigcontext->uc_mcontext.gregs[X64_R13] = R_R13;
    sigcontext->uc_mcontext.gregs[X64_R14] = R_R14;
    sigcontext->uc_mcontext.gregs[X64_R15] = R_R15;
    sigcontext->uc_mcontext.gregs[X64_RAX] = R_RAX;
    sigcontext->uc_mcontext.gregs[X64_RCX] = R_RCX;
    sigcontext->uc_mcontext.gregs[X64_RDX] = R_RDX;
    sigcontext->uc_mcontext.gregs[X64_RDI] = R_RDI;
    sigcontext->uc_mcontext.gregs[X64_RSI] = R_RSI;
    sigcontext->uc_mcontext.gregs[X64_RBP] = R_RBP;
    sigcontext->uc_mcontext.gregs[X64_RIP] = R_RIP;
    sigcontext->uc_mcontext.gregs[X64_RSP] = R_RSP;
    sigcontext->uc_mcontext.gregs[X64_RBX] = R_RBX;
    // flags
    sigcontext->uc_mcontext.gregs[X64_EFL] = emu->eflags.x64;
    // get segments
    sigcontext->uc_mcontext.gregs[X64_CSGSFS] = ((uint64_t)(R_CS)) | (((uint64_t)(R_GS))<<16) | (((uint64_t)(R_FS))<<32);
#if defined(DYNAREC) && defined(__aarch64__)
    if(db) {
        sigcontext->uc_mcontext.gregs[X64_RAX] = p->uc_mcontext.regs[10];
        sigcontext->uc_mcontext.gregs[X64_RCX] = p->uc_mcontext.regs[11];
        sigcontext->uc_mcontext.gregs[X64_RDX] = p->uc_mcontext.regs[12];
        sigcontext->uc_mcontext.gregs[X64_RBX] = p->uc_mcontext.regs[13];
        sigcontext->uc_mcontext.gregs[X64_RSP] = p->uc_mcontext.regs[14];
        sigcontext->uc_mcontext.gregs[X64_RBP] = p->uc_mcontext.regs[15];
        sigcontext->uc_mcontext.gregs[X64_RSI] = p->uc_mcontext.regs[16];
        sigcontext->uc_mcontext.gregs[X64_RDI] = p->uc_mcontext.regs[17];
        sigcontext->uc_mcontext.gregs[X64_R8] = p->uc_mcontext.regs[18];
        sigcontext->uc_mcontext.gregs[X64_R9] = p->uc_mcontext.regs[19];
        sigcontext->uc_mcontext.gregs[X64_R10] = p->uc_mcontext.regs[20];
        sigcontext->uc_mcontext.gregs[X64_R11] = p->uc_mcontext.regs[21];
        sigcontext->uc_mcontext.gregs[X64_R12] = p->uc_mcontext.regs[22];
        sigcontext->uc_mcontext.gregs[X64_R13] = p->uc_mcontext.regs[23];
        sigcontext->uc_mcontext.gregs[X64_R14] = p->uc_mcontext.regs[24];
        sigcontext->uc_mcontext.gregs[X64_R15] = p->uc_mcontext.regs[25];
        sigcontext->uc_mcontext.gregs[X64_RIP] = getX64Address(db, (uintptr_t)pc);
    }
#endif
    // get FloatPoint status
    sigcontext->uc_mcontext.fpregs = (struct x64_libc_fpstate*)&sigcontext->xstate;
    fpu_fxsave64(emu, &sigcontext->xstate);
    // add custom SIGN in reserved area
    //((unsigned int *)(&sigcontext.xstate.fpstate.padding))[8*4+12] = 0x46505853;  // not yet, when XSAVE / XRSTR will be ready
    // get signal mask

    if(new_ss) {
        sigcontext->uc_stack.ss_sp = new_ss->ss_sp;
        sigcontext->uc_stack.ss_size = new_ss->ss_size;
        sigcontext->uc_stack.ss_flags = new_ss->ss_flags;
    }
    // Try to guess some X64_TRAPNO
    /*
    TRAP_x86_DIVIDE     = 0,   // Division by zero exception
    TRAP_x86_TRCTRAP    = 1,   // Single-step exception
    TRAP_x86_NMI        = 2,   // NMI interrupt
    TRAP_x86_BPTFLT     = 3,   // Breakpoint exception
    TRAP_x86_OFLOW      = 4,   // Overflow exception
    TRAP_x86_BOUND      = 5,   // Bound range exception
    TRAP_x86_PRIVINFLT  = 6,   // Invalid opcode exception
    TRAP_x86_DNA        = 7,   // Device not available exception
    TRAP_x86_DOUBLEFLT  = 8,   // Double fault exception
    TRAP_x86_FPOPFLT    = 9,   // Coprocessor segment overrun
    TRAP_x86_TSSFLT     = 10,  // Invalid TSS exception
    TRAP_x86_SEGNPFLT   = 11,  // Segment not present exception
    TRAP_x86_STKFLT     = 12,  // Stack fault
    TRAP_x86_PROTFLT    = 13,  // General protection fault
    TRAP_x86_PAGEFLT    = 14,  // Page fault
    TRAP_x86_ARITHTRAP  = 16,  // Floating point exception
    TRAP_x86_ALIGNFLT   = 17,  // Alignment check exception
    TRAP_x86_MCHK       = 18,  // Machine check exception
    TRAP_x86_CACHEFLT   = 19   // SIMD exception (via SIGFPE) if CPU is SSE capable otherwise Cache flush exception (via SIGSEV)
    */
    uint32_t prot = getProtection((uintptr_t)info->si_addr);
    if(sig==SIGBUS)
        sigcontext->uc_mcontext.gregs[X64_TRAPNO] = 17;
    else if(sig==SIGSEGV) {
        if((uintptr_t)info->si_addr == sigcontext->uc_mcontext.gregs[X64_RIP]) {
            sigcontext->uc_mcontext.gregs[X64_ERR] = 0x0010;    // execution flag issue (probably)
            sigcontext->uc_mcontext.gregs[X64_TRAPNO] = (info->si_code == SEGV_ACCERR)?13:14;
        } else if(info->si_code==SEGV_ACCERR && !(prot&PROT_WRITE)) {
            sigcontext->uc_mcontext.gregs[X64_ERR] = 0x0002;    // write flag issue
            if(labs((intptr_t)info->si_addr-(intptr_t)sigcontext->uc_mcontext.gregs[X64_RSP])<16)
                sigcontext->uc_mcontext.gregs[X64_TRAPNO] = 12; // stack overflow probably
            else
                sigcontext->uc_mcontext.gregs[X64_TRAPNO] = 14; // PAGE_FAULT
        } else {
            sigcontext->uc_mcontext.gregs[X64_TRAPNO] = (info->si_code == SEGV_ACCERR)?13:14;
            //X64_ERR seems to be INT:8 CODE:8. So for write access segfault it's 0x0002 For a read it's 0x0004 (and 8 for exec). For an int 2d it could be 0x2D01 for example
            sigcontext->uc_mcontext.gregs[X64_ERR] = 0x0004;    // read error? there is no execute control in box64 anyway
        }
        if(info->si_code == SEGV_ACCERR && old_code)
            *old_code = -1;
    } else if(sig==SIGFPE)
        sigcontext->uc_mcontext.gregs[X64_TRAPNO] = 19;
    else if(sig==SIGILL)
        sigcontext->uc_mcontext.gregs[X64_TRAPNO] = 6;
    // call the signal handler
    x64_ucontext_t sigcontext_copy = *sigcontext;
    // save old value from emu
    #define GO(A) uint64_t old_##A = R_##A
    GO(RAX);
    GO(RDI);
    GO(RSI);
    GO(RDX);
    GO(RCX);
    GO(R8);
    GO(R9);
    GO(RBP);
    #undef GO
    // set stack pointer
    R_RSP = (uintptr_t)frame;
    // set frame pointer
    R_RBP = sigcontext->uc_mcontext.gregs[X64_RBP];

    int exits = 0;
    int ret = RunFunctionHandler(&exits, my_context->signals[sig], 3, sig, info, sigcontext);
    // restore old value from emu
    #define GO(A) R_##A = old_##A
    GO(RAX);
    GO(RDI);
    GO(RSI);
    GO(RDX);
    GO(RCX);
    GO(R8);
    GO(R9);
    GO(RBP);
    #undef GO

    if(memcmp(sigcontext, &sigcontext_copy, sizeof(x64_ucontext_t))) {
        emu_jmpbuf_t* ejb = GetJmpBuf();
        if(ejb->jmpbuf_ok) {
            #define GO(R)   ejb->emu->regs[_##R].q[0]=sigcontext->uc_mcontext.gregs[X64_R##R]
            GO(AX);
            GO(CX);
            GO(DX);
            GO(DI);
            GO(SI);
            GO(BP);
            GO(SP);
            GO(BX);
            #undef GO
            #define GO(R)   ejb->emu->regs[_##R].q[0]=sigcontext->uc_mcontext.gregs[X64_##R]
            GO(R8);
            GO(R9);
            GO(R10);
            GO(R11);
            GO(R12);
            GO(R13);
            GO(R14);
            GO(R15);
            #undef GO
            ejb->emu->ip.q[0]=sigcontext->uc_mcontext.gregs[X64_RIP];
            sigcontext->uc_mcontext.gregs[X64_RIP] = R_RIP;
            // flags
            ejb->emu->eflags.x64=sigcontext->uc_mcontext.gregs[X64_EFL];
            // get segments
            uint16_t seg;
            seg = (sigcontext->uc_mcontext.gregs[X64_CSGSFS] >> 0)&0xffff;
            #define GO(S) if(ejb->emu->segs[_##S]!=seg)  {ejb->emu->segs[_##S]=seg; ejb->emu->segs_serial[_##S] = 0;}
            GO(CS);
            seg = (sigcontext->uc_mcontext.gregs[X64_CSGSFS] >> 16)&0xffff;
            GO(GS);
            seg = (sigcontext->uc_mcontext.gregs[X64_CSGSFS] >> 32)&0xffff;
            GO(FS);
            #undef GO
            printf_log(LOG_DEBUG, "Context has been changed in Sigactionhanlder, doing siglongjmp to resume emu\n");
            if(old_code)
                *old_code = -1;    // re-init the value to allow another segfault at the same place
            if(used_stack)  // release stack
                new_ss->ss_flags = 0;
            relockMutex(Locks);
            siglongjmp(ejb->jmpbuf, 1);
        }
        printf_log(LOG_INFO, "Warning, context has been changed in Sigactionhanlder%s\n", (sigcontext->uc_mcontext.gregs[X64_RIP]!=sigcontext_copy.uc_mcontext.gregs[X64_RIP])?" (EIP changed)":"");
    }
    // restore regs...
    #define GO(R)   emu->regs[_##R].q[0]=sigcontext->uc_mcontext.gregs[X64_R##R]
    GO(AX);
    GO(CX);
    GO(DX);
    GO(DI);
    GO(SI);
    GO(BP);
    GO(SP);
    GO(BX);
    #undef GO
    #define GO(R)   emu->regs[_##R].q[0]=sigcontext->uc_mcontext.gregs[X64_##R]
    GO(R8);
    GO(R9);
    GO(R10);
    GO(R11);
    GO(R12);
    GO(R13);
    GO(R14);
    GO(R15);
    #undef GO
    emu->ip.q[0]=sigcontext->uc_mcontext.gregs[X64_RIP];
    emu->eflags.x64=sigcontext->uc_mcontext.gregs[X64_EFL];
    uint16_t seg;
    seg = (sigcontext->uc_mcontext.gregs[X64_CSGSFS] >> 0)&0xffff;
    #define GO(S) emu->segs[_##S]=seg; emu->segs_serial[_##S] = 0;
    GO(CS);
    seg = (sigcontext->uc_mcontext.gregs[X64_CSGSFS] >> 16)&0xffff;
    GO(GS);
    seg = (sigcontext->uc_mcontext.gregs[X64_CSGSFS] >> 32)&0xffff;
    GO(FS);
    #undef GO

    printf_log(LOG_DEBUG, "Sigactionhanlder main function returned (exit=%d, restorer=%p)\n", exits, (void*)restorer);
    if(exits) {
        relockMutex(Locks);
        exit(ret);
    }
    if(restorer)
        RunFunctionHandler(&exits, 0, restorer, 0);
    if(used_stack)  // release stack
        new_ss->ss_flags = 0;
    relockMutex(Locks);
}

void my_box64signalhandler(int32_t sig, siginfo_t* info, void * ucntx)
{
    // sig==SIGSEGV || sig==SIGBUS || sig==SIGILL here!
    int log_minimum = (my_context->is_sigaction[sig] && sig==SIGSEGV)?LOG_INFO:LOG_NONE;
    ucontext_t *p = (ucontext_t *)ucntx;
    void* addr = (void*)info->si_addr;  // address that triggered the issue
    void* rsp = NULL;
#ifdef __aarch64__
    void * pc = (void*)p->uc_mcontext.pc;
#elif defined __x86_64__
    void * pc = (void*)p->uc_mcontext.gregs[X64_RIP];
#elif defined __powerpc64__
    void * pc = (void*)p->uc_mcontext.uc_regs->gregs[PT_NIP];
#else
    void * pc = NULL;    // unknow arch...
    #warning Unhandled architecture
#endif
    int Locks = unlockMutex();
    uint32_t prot = getProtection((uintptr_t)addr);
#ifdef DYNAREC
    dynablock_t* db = NULL;
    int db_searched = 0;
    if ((sig==SIGSEGV) && (addr) && (info->si_code == SEGV_ACCERR) && (prot&PROT_DYNAREC)) {
        // access error, unprotect the block (and mark them dirty)
        unprotectDB((uintptr_t)addr, 1);    // unprotect 1 byte... But then, the whole page will be unprotected
        // check if SMC inside block
        db = FindDynablockFromNativeAddress(pc);
        db_searched = 1;
        dynarec_log(LOG_DEBUG, "SIGSEGV with Access error on %p for %p , db=%p(%p)\n", pc, addr, db, db?((void*)db->x64_addr):NULL);
        if(db && ((addr>=db->x64_addr && addr<(db->x64_addr+db->x64_size)) || db->need_test)) {
            // dynablock got auto-dirty! need to get out of it!!!
            emu_jmpbuf_t* ejb = GetJmpBuf();
            if(ejb->jmpbuf_ok) {
                ejb->emu->regs[_AX].q[0] = p->uc_mcontext.regs[10];
                ejb->emu->regs[_CX].q[0] = p->uc_mcontext.regs[11];
                ejb->emu->regs[_DX].q[0] = p->uc_mcontext.regs[12];
                ejb->emu->regs[_BX].q[0] = p->uc_mcontext.regs[13];
                ejb->emu->regs[_SP].q[0] = p->uc_mcontext.regs[14];
                ejb->emu->regs[_BP].q[0] = p->uc_mcontext.regs[15];
                ejb->emu->regs[_SI].q[0] = p->uc_mcontext.regs[16];
                ejb->emu->regs[_DI].q[0] = p->uc_mcontext.regs[17];
                ejb->emu->regs[_R8].q[0] = p->uc_mcontext.regs[18];
                ejb->emu->regs[_R9].q[0] = p->uc_mcontext.regs[19];
                ejb->emu->regs[_R10].q[0] = p->uc_mcontext.regs[20];
                ejb->emu->regs[_R11].q[0] = p->uc_mcontext.regs[21];
                ejb->emu->regs[_R12].q[0] = p->uc_mcontext.regs[22];
                ejb->emu->regs[_R13].q[0] = p->uc_mcontext.regs[23];
                ejb->emu->regs[_R14].q[0] = p->uc_mcontext.regs[24];
                ejb->emu->regs[_R15].q[0] = p->uc_mcontext.regs[25];
                ejb->emu->ip.q[0] = getX64Address(db, (uintptr_t)pc);
                ejb->emu->eflags.x64 = p->uc_mcontext.regs[26];
                if(addr>=db->x64_addr && addr<(db->x64_addr+db->x64_size)) {
                    dynarec_log(LOG_INFO, "Auto-SMC detected, getting out of current Dynablock!\n");
                } else {
                    dynarec_log(LOG_INFO, "Dynablock unprotected, getting out!\n");
                }
                relockMutex(Locks);
                siglongjmp(ejb->jmpbuf, 2);
            }
            dynarec_log(LOG_INFO, "Warning, Auto-SMC (%p for db %p/%p) detected, but jmpbuffer not ready!\n", (void*)addr, db, (void*)db->x64_addr);
        }
        // done
        if(prot&PROT_WRITE) {
            // if there is no write permission, don't return and continue to program signal handling
            relockMutex(Locks);
            return;
        }
    } else if ((sig==SIGSEGV) && (addr) && (info->si_code == SEGV_ACCERR) && (prot&(PROT_READ|PROT_WRITE))) {
        db = FindDynablockFromNativeAddress(pc);
        db_searched = 1;
        if(db && db->x64_addr>= addr && (db->x64_addr+db->x64_size)<addr) {
            dynarec_log(LOG_INFO, "Warning, addr inside current dynablock!\n");
        }
        static void* glitch_pc = NULL;
        static void* glitch_addr = NULL;
        static int glitch_prot = 0;
        if(addr && pc && db) {
            if((glitch_pc!=pc || glitch_addr!=addr || glitch_prot!=prot)) {
                // probably a glitch due to intensive multitask...
                dynarec_log(/*LOG_DEBUG*/LOG_INFO, "SIGSEGV with Access error on %p for %p , db=%p, retrying\n", pc, addr, db);
                relockMutex(Locks);
                glitch_pc = pc;
                glitch_addr = addr;
                glitch_prot = prot;
                return; // try again
            }
            glitch_pc = NULL;
            glitch_addr = NULL;
            glitch_prot = 0;
        }
    }
#else
    void* db = NULL;
#endif
    static int old_code = -1;
    static void* old_pc = 0;
    static void* old_addr = 0;
    const char* signame = (sig==SIGSEGV)?"SIGSEGV":((sig==SIGBUS)?"SIGBUS":"SIGILL");
    if(old_code==info->si_code && old_pc==pc && old_addr==addr) {
        printf_log(log_minimum, "%04d|Double %s (code=%d, pc=%p, addr=%p)!\n", GetTID(), signame, old_code, old_pc, old_addr);
exit(-1);
    } else {
        if(sig==SIGSEGV && info->si_code==2 && ((prot&~PROT_CUSTOM)==5 || (prot&~PROT_CUSTOM)==7)) {
            relockMutex(Locks);
            return; // that's probably just a multi-task glitch, like seen in terraria
        }
#ifdef DYNAREC
        if(!db_searched)
            db = FindDynablockFromNativeAddress(pc);
#endif
        old_code = info->si_code;
        old_pc = pc;
        old_addr = addr;
        const char* name = GetNativeName(pc);
        uintptr_t x64pc = (uintptr_t)-1;
        const char* x64name = NULL;
        const char* elfname = NULL;
        x64emu_t* emu = thread_get_emu();
        x64pc = R_RIP;
        rsp = (void*)R_RSP;
#if defined(__aarch64__) && defined(DYNAREC)
        if(db && p->uc_mcontext.regs[0]>0x10000) {
            emu = (x64emu_t*)p->uc_mcontext.regs[0];
        }
        if(db) {
            x64pc = getX64Address(db, (uintptr_t)pc);
            rsp = (void*)p->uc_mcontext.regs[10+_SP];
        }
#endif
        x64name = getAddrFunctionName(x64pc);
        elfheader_t* elf = FindElfAddress(my_context, x64pc);
        if(elf)
            elfname = ElfName(elf);
        if(jit_gdb) {
            pid_t pid = getpid();
            int v = fork(); // is this ok in a signal handler???
            if(v) {
                // parent process, the one that have the segfault
                volatile int waiting = 1;
                printf("Waiting for %s (pid %d)...\n", (jit_gdb==2)?"gdbserver":"gdb", pid);
                while(waiting) {
                    // using gdb, use "set waiting=0" to stop waiting...
                    usleep(1000);
                }
            } else {
                char myarg[50] = {0};
                sprintf(myarg, "%d", pid);
                if(jit_gdb==2)
                    execlp("gdbserver", "gdbserver", "127.0.0.1:1234", "--attach", myarg, (char*)NULL);
                else
                    execlp("gdb", "gdb", "-pid", myarg, (char*)NULL);
                exit(-1);
            }
        }
#ifdef DYNAREC
        uint32_t hash = 0;
        if(db)
            hash = X31_hash_code(db->x64_addr, db->x64_size);
        printf_log(log_minimum, "%04d|%s @%p (%s) (x64pc=%p/%s:\"%s\", rsp=%p, stack=%p:%p own=%p fp=%p), for accessing %p (code=%d/prot=%x), db=%p(%p:%p/%p:%p/%s:%s, hash:%x/%x) handler=%p", 
            GetTID(), signame, pc, name, (void*)x64pc, elfname?elfname:"???", x64name?x64name:"???", rsp, 
            emu->init_stack, emu->init_stack+emu->size_stack, emu->stack2free, (void*)R_RBP, 
            addr, info->si_code, 
            prot, db, db?db->block:0, db?(db->block+db->size):0, 
            db?db->x64_addr:0, db?(db->x64_addr+db->x64_size):0, 
            getAddrFunctionName((uintptr_t)(db?db->x64_addr:0)), 
            (db?db->need_test:0)?"need_stest":"clean", db?db->hash:0, hash, 
            (void*)my_context->signals[sig]);
#else
        printf_log(log_minimum, "%04d|%s @%p (%s) (x64pc=%p/%s:\"%s\", rsp=%p), for accessing %p (code=%d)", GetTID(), signame, pc, name, (void*)x64pc, elfname?elfname:"???", x64name?x64name:"???", rsp, addr, info->si_code);
#endif
        if(sig==SIGILL)
            printf_log(log_minimum, " opcode=%02X %02X %02X %02X %02X %02X %02X %02X\n", ((uint8_t*)pc)[0], ((uint8_t*)pc)[1], ((uint8_t*)pc)[2], ((uint8_t*)pc)[3], ((uint8_t*)pc)[4], ((uint8_t*)pc)[5], ((uint8_t*)pc)[6], ((uint8_t*)pc)[7]);
        else if(sig==SIGBUS)
            printf_log(log_minimum, " x86opcode=%02X %02X %02X %02X %02X %02X %02X %02X\n", ((uint8_t*)x64pc)[0], ((uint8_t*)x64pc)[1], ((uint8_t*)x64pc)[2], ((uint8_t*)x64pc)[3], ((uint8_t*)x64pc)[4], ((uint8_t*)x64pc)[5], ((uint8_t*)x64pc)[6], ((uint8_t*)x64pc)[7]);
        else
            printf_log(log_minimum, "\n");
    }
    relockMutex(Locks);
    if(my_context->signals[sig] && my_context->signals[sig]!=1) {
        if(my_context->is_sigaction[sig])
            my_sigactionhandler_oldcode(sig, info, ucntx, &old_code, db);
        else
            my_sighandler(sig);
        return;
    }
    // no handler (or double identical segfault)
    // set default and that's it, instruction will restart and default segfault handler will be called...
    if(my_context->signals[sig]!=1 || sig==SIGSEGV || sig==SIGILL || sig==SIGFPE) {
        signal(sig, (void*)my_context->signals[sig]);
    }
}

void my_sigactionhandler(int32_t sig, siginfo_t* info, void * ucntx)
{
    #ifdef DYNAREC
    ucontext_t *p = (ucontext_t *)ucntx;
    void * pc = (void*)p->uc_mcontext.pc;
    dynablock_t* db = FindDynablockFromNativeAddress(pc);
    #else
    void* db = NULL;
    #endif

    my_sigactionhandler_oldcode(sig, info, ucntx, NULL, db);
}

void emit_signal(x64emu_t* emu, int sig, void* addr, int code)
{
    ucontext_t ctx = {0};
    void* db = NULL;
    siginfo_t info = {0};
    info.si_signo = sig;
    info.si_errno = 0;
    info.si_code = code;
    info.si_addr = addr;
    printf_log(LOG_INFO, "Emit Signal %d at IP=%p / addr=%p, code=%d\n", sig, (void*)R_RIP, addr, code);
    my_sigactionhandler_oldcode(sig, &info, &ctx, NULL, db);
}

EXPORT sighandler_t my_signal(x64emu_t* emu, int signum, sighandler_t handler)
{
    if(signum<0 || signum>=MAX_SIGNAL)
        return SIG_ERR;

    if(signum==SIGSEGV && emu->context->no_sigsegv)
        return 0;

    // create a new handler
    my_context->signals[signum] = (uintptr_t)handler;
    my_context->is_sigaction[signum] = 0;
    my_context->restorer[signum] = 0;
    if(handler!=NULL && handler!=(sighandler_t)1) {
        handler = my_sighandler;
    }

    if(signum==SIGSEGV || signum==SIGBUS || signum==SIGILL)
        return 0;

    return signal(signum, handler);
}
EXPORT sighandler_t my___sysv_signal(x64emu_t* emu, int signum, sighandler_t handler) __attribute__((alias("my_signal")));
EXPORT sighandler_t my_sysv_signal(x64emu_t* emu, int signum, sighandler_t handler) __attribute__((alias("my_signal")));    // not completly exact

int EXPORT my_sigaction(x64emu_t* emu, int signum, const x64_sigaction_t *act, x64_sigaction_t *oldact)
{
    if(signum<0 || signum>=MAX_SIGNAL)
        return -1;
    
    if(signum==SIGSEGV && emu->context->no_sigsegv)
        return 0;

    if(signum==SIGILL && emu->context->no_sigill)
        return 0;
    struct sigaction newact = {0};
    struct sigaction old = {0};
    if(act) {
        newact.sa_mask = act->sa_mask;
        newact.sa_flags = act->sa_flags&~0x04000000;  // No sa_restorer...
        if(act->sa_flags&0x04) {
            my_context->signals[signum] = (uintptr_t)act->_u._sa_sigaction;
            my_context->is_sigaction[signum] = 1;
            if(act->_u._sa_handler!=NULL && act->_u._sa_handler!=(sighandler_t)1) {
                newact.sa_sigaction = my_sigactionhandler;
            } else
                newact.sa_sigaction = act->_u._sa_sigaction;
        } else {
            my_context->signals[signum] = (uintptr_t)act->_u._sa_handler;
            my_context->is_sigaction[signum] = 0;
            if(act->_u._sa_handler!=NULL && act->_u._sa_handler!=(sighandler_t)1) {
                newact.sa_handler = my_sighandler;
            } else
                newact.sa_handler = act->_u._sa_handler;
        }
        my_context->restorer[signum] = (act->sa_flags&0x04000000)?(uintptr_t)act->sa_restorer:0;
        my_context->onstack[signum] = (act->sa_flags&SA_ONSTACK)?1:0;
    }
    int ret = 0;
    if(signum!=SIGSEGV && signum!=SIGBUS && signum!=SIGILL)
        ret = sigaction(signum, act?&newact:NULL, oldact?&old:NULL);
    if(oldact) {
        oldact->sa_flags = old.sa_flags;
        oldact->sa_mask = old.sa_mask;
        if(old.sa_flags & 0x04)
            oldact->_u._sa_sigaction = old.sa_sigaction; //TODO should wrap...
        else
            oldact->_u._sa_handler = old.sa_handler;  //TODO should wrap...
        oldact->sa_restorer = NULL; // no handling for now...
    }
    return ret;
}
int EXPORT my___sigaction(x64emu_t* emu, int signum, const x64_sigaction_t *act, x64_sigaction_t *oldact)
__attribute__((alias("my_sigaction")));

int EXPORT my_syscall_rt_sigaction(x64emu_t* emu, int signum, const x64_sigaction_restorer_t *act, x64_sigaction_restorer_t *oldact, int sigsetsize)
{
    printf_log(LOG_DEBUG, "Syscall/Sigaction(signum=%d, act=%p, old=%p, size=%d)\n", signum, act, oldact, sigsetsize);
    if(signum<0 || signum>=MAX_SIGNAL)
        return -1;
    
    if(signum==SIGSEGV && emu->context->no_sigsegv)
        return 0;
    // TODO, how to handle sigsetsize>4?!
    if(signum==32 || signum==33) {
        // cannot use libc sigaction, need to use syscall!
        struct kernel_sigaction newact = {0};
        struct kernel_sigaction old = {0};
        if(act) {
            printf_log(LOG_DEBUG, " New (kernel) action flags=0x%x mask=0x%lx\n", act->sa_flags, *(uint64_t*)&act->sa_mask);
            memcpy(&newact.sa_mask, &act->sa_mask, (sigsetsize>16)?16:sigsetsize);
            newact.sa_flags = act->sa_flags&~0x04000000;  // No sa_restorer...
            if(act->sa_flags&0x04) {
                my_context->signals[signum] = (uintptr_t)act->_u._sa_sigaction;
                my_context->is_sigaction[signum] = 1;
                if(act->_u._sa_handler!=NULL && act->_u._sa_handler!=(sighandler_t)1) {
                    newact.k_sa_handler = (void*)my_sigactionhandler;
                } else {
                    newact.k_sa_handler = (void*)act->_u._sa_sigaction;
                }
            } else {
                my_context->signals[signum] = (uintptr_t)act->_u._sa_handler;
                my_context->is_sigaction[signum] = 0;
                if(act->_u._sa_handler!=NULL && act->_u._sa_handler!=(sighandler_t)1) {
                    newact.k_sa_handler = my_sighandler;
                } else {
                    newact.k_sa_handler = act->_u._sa_handler;
                }
            }
            my_context->restorer[signum] = (act->sa_flags&0x04000000)?(uintptr_t)act->sa_restorer:0;
        }

        if(oldact) {
            old.sa_flags = oldact->sa_flags;
            memcpy(&old.sa_mask, &oldact->sa_mask, (sigsetsize>16)?16:sigsetsize);
        }

        int ret = syscall(__NR_rt_sigaction, signum, act?&newact:NULL, oldact?&old:NULL, (sigsetsize>16)?16:sigsetsize);
        if(oldact && ret==0) {
            oldact->sa_flags = old.sa_flags;
            memcpy(&oldact->sa_mask, &old.sa_mask, (sigsetsize>16)?16:sigsetsize);
            if(old.sa_flags & 0x04)
                oldact->_u._sa_sigaction = (void*)old.k_sa_handler; //TODO should wrap...
            else
                oldact->_u._sa_handler = old.k_sa_handler;  //TODO should wrap...
        }
        return ret;
    } else {
        // using libc sigaction
        struct sigaction newact = {0};
        struct sigaction old = {0};
        if(act) {
            printf_log(LOG_DEBUG, " New action flags=0x%x mask=0x%lx\n", act->sa_flags, *(uint64_t*)&act->sa_mask);
            newact.sa_mask = act->sa_mask;
            newact.sa_flags = act->sa_flags&~0x04000000;  // No sa_restorer...
            if(act->sa_flags&0x04) {
                if(act->_u._sa_handler!=NULL && act->_u._sa_handler!=(sighandler_t)1) {
                    my_context->signals[signum] = (uintptr_t)act->_u._sa_sigaction;
                    newact.sa_sigaction = my_sigactionhandler;
                } else {
                    newact.sa_sigaction = act->_u._sa_sigaction;
                }
            } else {
                if(act->_u._sa_handler!=NULL && act->_u._sa_handler!=(sighandler_t)1) {
                    my_context->signals[signum] = (uintptr_t)act->_u._sa_handler;
                    my_context->is_sigaction[signum] = 0;
                    newact.sa_handler = my_sighandler;
                } else {
                    newact.sa_handler = act->_u._sa_handler;
                }
            }
            my_context->restorer[signum] = (act->sa_flags&0x04000000)?(uintptr_t)act->sa_restorer:0;
        }

        if(oldact) {
            old.sa_flags = oldact->sa_flags;
            old.sa_mask = oldact->sa_mask;
        }
        int ret = 0;

        if(signum!=SIGSEGV && signum!=SIGBUS && signum!=SIGILL)
            ret = sigaction(signum, act?&newact:NULL, oldact?&old:NULL);
        if(oldact && ret==0) {
            oldact->sa_flags = old.sa_flags;
            oldact->sa_mask = old.sa_mask;
            if(old.sa_flags & 0x04)
                oldact->_u._sa_sigaction = old.sa_sigaction; //TODO should wrap...
            else
                oldact->_u._sa_handler = old.sa_handler;  //TODO should wrap...
        }
        return ret;
    }
}

EXPORT sighandler_t my_sigset(x64emu_t* emu, int signum, sighandler_t handler)
{
    // emulated SIG_HOLD
    if(handler == (sighandler_t)2) {
        x64_sigaction_t oact;
        sigset_t nset;
        sigset_t oset;
        if (sigemptyset (&nset) < 0)
            return (sighandler_t)-1;
        if (sigaddset (&nset, signum) < 0)
            return (sighandler_t)-1;
        if (sigprocmask (SIG_BLOCK, &nset, &oset) < 0)
            return (sighandler_t)-1;
        if (sigismember (&oset, signum))
            return (sighandler_t)2;
        if (my_sigaction (emu, signum, NULL, &oact) < 0)
            return (sighandler_t)-1;
        return oact._u._sa_handler;
    }
    return my_signal(emu, signum, handler);
}

EXPORT int my_getcontext(x64emu_t* emu, void* ucp)
{
//    printf_log(LOG_NONE, "Warning: call to partially implemented getcontext\n");
    x64_ucontext_t *u = (x64_ucontext_t*)ucp;
    // stack traking
    u->uc_stack.ss_sp = NULL;
    u->uc_stack.ss_size = 0;    // this need to filled
    // get general register
    u->uc_mcontext.gregs[X64_RAX] = R_RAX;
    u->uc_mcontext.gregs[X64_RCX] = R_RCX;
    u->uc_mcontext.gregs[X64_RDX] = R_RDX;
    u->uc_mcontext.gregs[X64_RDI] = R_RDI;
    u->uc_mcontext.gregs[X64_RSI] = R_RSI;
    u->uc_mcontext.gregs[X64_RBP] = R_RBP;
    u->uc_mcontext.gregs[X64_RIP] = *(uint64_t*)R_RSP;
    u->uc_mcontext.gregs[X64_RSP] = R_RSP+sizeof(uintptr_t);
    u->uc_mcontext.gregs[X64_RBX] = R_RBX;
    u->uc_mcontext.gregs[X64_R8] = R_R8;
    u->uc_mcontext.gregs[X64_R9] = R_R9;
    u->uc_mcontext.gregs[X64_R10] = R_R10;
    u->uc_mcontext.gregs[X64_R11] = R_R11;
    u->uc_mcontext.gregs[X64_R12] = R_R12;
    u->uc_mcontext.gregs[X64_R13] = R_R13;
    u->uc_mcontext.gregs[X64_R14] = R_R14;
    u->uc_mcontext.gregs[X64_R15] = R_R15;
    // get segments
    u->uc_mcontext.gregs[X64_CSGSFS] = ((uint64_t)(R_CS)) | (((uint64_t)(R_GS))<<16) | (((uint64_t)(R_FS))<<32);
    // get FloatPoint status
    // get signal mask
    sigprocmask(SIG_SETMASK, NULL, (sigset_t*)&u->uc_sigmask);

    return 0;
}

EXPORT int my_setcontext(x64emu_t* emu, void* ucp)
{
//    printf_log(LOG_NONE, "Warning: call to partially implemented setcontext\n");
    x64_ucontext_t *u = (x64_ucontext_t*)ucp;
    // stack tracking
    emu->init_stack = u->uc_stack.ss_sp;
    emu->size_stack = u->uc_stack.ss_size;
    // set general register
    R_RAX = u->uc_mcontext.gregs[X64_RAX];
    R_RCX = u->uc_mcontext.gregs[X64_RCX];
    R_RDX = u->uc_mcontext.gregs[X64_RDX];
    R_RDI = u->uc_mcontext.gregs[X64_RDI];
    R_RSI = u->uc_mcontext.gregs[X64_RSI];
    R_RBP = u->uc_mcontext.gregs[X64_RBP];
    R_RIP = u->uc_mcontext.gregs[X64_RIP];
    R_RSP = u->uc_mcontext.gregs[X64_RSP];
    R_RBX = u->uc_mcontext.gregs[X64_RBX];
    R_R8  = u->uc_mcontext.gregs[X64_R8];
    R_R9  = u->uc_mcontext.gregs[X64_R9];
    R_R10 = u->uc_mcontext.gregs[X64_R10];
    R_R11 = u->uc_mcontext.gregs[X64_R11];
    R_R12 = u->uc_mcontext.gregs[X64_R12];
    R_R13 = u->uc_mcontext.gregs[X64_R13];
    R_R14 = u->uc_mcontext.gregs[X64_R14];
    R_R15 = u->uc_mcontext.gregs[X64_R15];
    // get segments
    R_CS = (u->uc_mcontext.gregs[X64_CSGSFS]>> 0)&0xffff;
    R_GS = (u->uc_mcontext.gregs[X64_CSGSFS]>>16)&0xffff;
    R_FS = (u->uc_mcontext.gregs[X64_CSGSFS]>>32)&0xffff;
    // set FloatPoint status
    // set signal mask
    //sigprocmask(SIG_SETMASK, NULL, (sigset_t*)&u->uc_sigmask);
    // set uc_link
    emu->uc_link = u->uc_link;

    return R_EAX;
}

EXPORT int my_makecontext(x64emu_t* emu, void* ucp, void* fnc, int32_t argc, int64_t* argv)
{
//    printf_log(LOG_NONE, "Warning: call to unimplemented makecontext\n");
    x64_ucontext_t *u = (x64_ucontext_t*)ucp;
    // setup stack
    uintptr_t* rsp = (uintptr_t*)(u->uc_stack.ss_sp + u->uc_stack.ss_size - sizeof(uintptr_t));
    // setup the function
    u->uc_mcontext.gregs[X64_RIP] = (intptr_t)fnc;
    // setup args
    int n = 3;
    int j = 0;
    int regs_abi[] = {_DI, _SI, _DX, _CX, _R8, _R9};
    for (int i=0; i<argc; ++i) {
        // get value first
        uint64_t v;
        if(n<6)
            v = emu->regs[regs_abi[n++]].q[0];
        else
            v = argv[j++];
        // push value
        switch(i) {
            case 0: u->uc_mcontext.gregs[X64_RDI] = v; break;
            case 1: u->uc_mcontext.gregs[X64_RSI] = v; break;
            case 2: u->uc_mcontext.gregs[X64_RDX] = v; break;
            case 3: u->uc_mcontext.gregs[X64_RCX] = v; break;
            case 4: u->uc_mcontext.gregs[X64_R8] = v; break;
            case 5: u->uc_mcontext.gregs[X64_R9] = v; break;
            default:
                --rsp;
                *rsp = argv[(argc-1)-i];
        }
    }
    // push the return value
    --rsp;
    *rsp = (uintptr_t)GetExit();
    u->uc_mcontext.gregs[X64_RSP] = (uintptr_t)rsp;
    
    return 0;
}

EXPORT int my_swapcontext(x64emu_t* emu, void* ucp1, void* ucp2)
{
//    printf_log(LOG_NONE, "Warning: call to unimplemented swapcontext\n");
    // grab current context in ucp1
    my_getcontext(emu, ucp1);
    // activate ucp2
    my_setcontext(emu, ucp2);
    return 0;
}

void init_signal_helper(box64context_t* context)
{
    // setup signal handling
    for(int i=0; i<MAX_SIGNAL; ++i) {
        context->signals[i] = 1;    // SIG_DFL
    }
	struct sigaction action = {0};
	action.sa_flags = SA_SIGINFO | SA_RESTART | SA_NODEFER;
	action.sa_sigaction = my_box64signalhandler;
    sigaction(SIGSEGV, &action, NULL);
	action.sa_flags = SA_SIGINFO | SA_RESTART | SA_NODEFER;
	action.sa_sigaction = my_box64signalhandler;
    sigaction(SIGBUS, &action, NULL);
	action.sa_flags = SA_SIGINFO | SA_RESTART | SA_NODEFER;
	action.sa_sigaction = my_box64signalhandler;
    sigaction(SIGILL, &action, NULL);

	pthread_once(&sigstack_key_once, sigstack_key_alloc);
}

void fini_signal_helper()
{
    signal(SIGSEGV, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    signal(SIGILL, SIG_DFL);
}
