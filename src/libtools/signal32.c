#define _GNU_SOURCE
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
#include <pthread.h>
#ifndef ANDROID
#include <execinfo.h>
#endif

#include "box32context.h"
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
#include "dynarec_native.h"
#endif


/* Definitions taken from the kernel headers.  */
enum
{
  I386_GS = 0,
# define I386_GS         I386_GS
  I386_FS,
# define I386_FS         I386_FS
  I386_ES,
# define I386_ES         I386_ES
  I386_DS,
# define I386_DS         I386_DS
  I386_EDI,
# define I386_EDI        I386_EDI
  I386_ESI,
# define I386_ESI        I386_ESI
  I386_EBP,
# define I386_EBP        I386_EBP
  I386_ESP,
# define I386_ESP        I386_ESP
  I386_EBX,
# define I386_EBX        I386_EBX
  I386_EDX,
# define I386_EDX        I386_EDX
  I386_ECX,
# define I386_ECX        I386_ECX
  I386_EAX,
# define I386_EAX        I386_EAX
  I386_TRAPNO,
# define I386_TRAPNO        I386_TRAPNO
  I386_ERR,
# define I386_ERR        I386_ERR
  I386_EIP,
# define I386_EIP        I386_EIP
  I386_CS,
# define I386_CS                I386_CS
  I386_EFL,
# define I386_EFL        I386_EFL
  I386_UESP,
# define I386_UESP        I386_UESP
  I386_SS
# define I386_SS        I386_SS
};

typedef uint32_t i386_gregset_t[19];
struct i386_fpreg
{
  uint16_t significand[4];
  uint16_t exponent;
}__attribute__((packed));

struct i386_fpxreg
{
  unsigned short significand[4];
  unsigned short exponent;
  unsigned short padding[3];
}__attribute__((packed));

struct i386_xmmreg
{
  uint32_t          element[4];
}__attribute__((packed));

struct i386_fpstate
{
  /* Regular FPU environment.  */
  uint32_t          cw;
  uint32_t          sw;
  uint32_t          tag;
  uint32_t          ipoff;
  uint32_t          cssel;
  uint32_t          dataoff;
  uint32_t          datasel;
  struct i386_fpreg _st[8];
  uint32_t          status_magic;
  /* FXSR FPU environment.  */
  uint32_t          _fxsr_env[6];
  uint32_t          mxcsr;
  uint32_t          reserved;
  struct i386_fpxreg _fxsr_st[8];
  struct i386_xmmreg _xmm[8];
  uint32_t          padding[56];
}__attribute__((packed));

typedef struct i386_fpstate *i386_fpregset_t;

static void save_fpreg(x64emu_t* emu, struct i386_fpstate* state)
{
    emu->sw.f.F87_TOP = emu->top&7;
    state->sw = emu->sw.x16;
    state->cw = emu->cw.x16;
    // save SSE and MMX regs
    fpu_fxsave32(emu, &state->_fxsr_env);
}
static void load_fpreg(x64emu_t* emu, struct i386_fpstate* state)
{
    // copy SSE and MMX regs
    fpu_fxrstor32(emu, &state->_fxsr_env);
    emu->cw.x16 = state->cw;
    emu->sw.x16 = state->sw;
    emu->top = emu->sw.f.F87_TOP&7;
}

typedef struct
  {
    ptr_t ss_sp;
    int ss_flags;
    long_t ss_size;
  } i386_stack_t;

typedef struct x64_stack_s
{
    void *ss_sp;
    int ss_flags;
    size_t ss_size;
} x64_stack_t;


/*
another way to see the sigcontext
struct sigcontext
{
  unsigned short gs, __gsh;
  unsigned short fs, __fsh;
  unsigned short es, __esh;
  unsigned short ds, __dsh;
  unsigned long edi;
  unsigned long esi;
  unsigned long ebp;
  unsigned long esp;
  unsigned long ebx;
  unsigned long edx;
  unsigned long ecx;
  unsigned long eax;
  unsigned long trapno;
  unsigned long err;
  unsigned long eip;
  unsigned short cs, __csh;
  unsigned long eflags;
  unsigned long esp_at_signal;
  unsigned short ss, __ssh;
  struct _fpstate * fpstate;
  unsigned long oldmask;
  unsigned long cr2;
};
*/
typedef struct
  {
    i386_gregset_t gregs;
    ptr_t fpregs;   //i386_fpregset_t
    uint32_t oldmask;
    uint32_t cr2;
  } i386_mcontext_t;

// /!\ signal sig_set is different than glibc __sig_set
#define _NSIG_WORDS (64 / 32)
typedef unsigned long i386_old_sigset_t;
typedef struct {
    unsigned long sig[_NSIG_WORDS];
} i386_sigset_t;

struct i386_xsave_hdr_struct {
 	uint64_t xstate_bv;
 	uint64_t reserved1[2];
 	uint64_t reserved2[5];
};

struct i386_xstate {
	/*
	 * Applications need to refer to fpstate through fpstate pointer
	 * in sigcontext. Not here directly.
	 */
 	struct i386_fpstate fpstate;
 	struct i386_xsave_hdr_struct xsave_hdr;
 	/* new processor state extensions will go here */
} __attribute__ ((aligned (64)));

struct i386_xstate_cntxt {
	ptr_t               xstate; //struct  i386_xstate *xstate;
	uint32_t	        size;
	uint32_t 	        lmask;
	uint32_t	        hmask;
};

typedef struct i386_ucontext_s
{
    uint32_t uc_flags;
    ptr_t uc_link;  //struct i386_ucontext_s *uc_link;
    i386_stack_t uc_stack;
    i386_mcontext_t uc_mcontext;
    i386_sigset_t uc_sigmask;
	/* Allow for uc_sigmask growth.  Glibc uses a 1024-bit sigset_t.  */
	int		  unused[32 - (sizeof (sigset_t) / sizeof (int))];
	//struct i386_xstate_cntxt  uc_xstate;
    struct i386_xstate  xstate;
} i386_ucontext_t;

typedef struct i386_sigframe_s {
    ptr_t           pretcode;   // pointer to retcode
    int             sig;
    i386_mcontext_t cpustate;
    struct i386_xstate fpstate;
    ptr_t           extramask[64-1];
    char            retcode[8];
} i386_sigframe_t;

struct kernel_sigaction {
        void (*k_sa_handler) (int);
        unsigned long sa_flags;
        void (*sa_restorer) (void);
        unsigned long sa_mask;
        unsigned long sa_mask2;
};
#ifdef DYNAREC
uintptr_t getX64Address(dynablock_t* db, uintptr_t arm_addr);
#endif

x64_stack_t* sigstack_getstack();
int my_sigaltstack(x64emu_t* emu, const x64_stack_t* ss, x64_stack_t* oss);
EXPORT int my32_sigaltstack(x64emu_t* emu, const i386_stack_t* ss, i386_stack_t* oss)
{
    x64_stack_t ss_ = {0};
    x64_stack_t oss_ = {0};
    if(ss) {
        ss_.ss_flags = ss->ss_flags;
        ss_.ss_sp = from_ptrv(ss->ss_sp);
        ss_.ss_size = ss->ss_size;
    }
    int ret = my_sigaltstack(emu, ss?(&ss_):NULL, oss?(&oss_):NULL);
    if(!ret && oss) {
        oss->ss_flags = oss_.ss_flags;
        oss->ss_sp = to_ptrv(oss_.ss_sp);
        oss->ss_size = oss_.ss_size;
    }
    return ret;
}


uint32_t RunFunctionHandler32(int* exit, int dynarec, i386_ucontext_t* sigcontext, ptr_t fnc, int nargs, ...)
{
    if(fnc==0 || fnc==1) {
        va_list va;
        va_start (va, nargs);
        int sig = va_arg(va, int);
        va_end (va);
        printf_log(LOG_NONE, "%04d|BOX32: Warning, calling Signal %d function handler %s\n", GetTID(), sig, fnc?"SIG_IGN":"SIG_DFL");
        if(fnc==0) {
            printf_log(LOG_NONE, "Unhandled signal caught, aborting\n");
            abort();
        }
        return 0;
    }
#ifdef HAVE_TRACE
    uintptr_t old_start = trace_start, old_end = trace_end;
#if 0
    trace_start = 0; trace_end = 1; // disabling trace, globably for now...
#endif
#endif
#ifndef USE_CUSTOM_MEM
    // because a signal can interupt a malloc-like function
    // Dynarec cannot be used in signal handling unless custom malloc is used
    dynarec = 0;
#endif

    x64emu_t *emu = thread_get_emu();
    #ifdef DYNAREC
    if(box64_dynarec_test)
        emu->test.test = 0;
    #endif

    /*SetFS(emu, default_fs);*/
    for (int i=0; i<6; ++i)
        emu->segs_serial[i] = 0;

    int align = nargs&1;

    R_ESP -= nargs * sizeof(ptr_t);

    uint32_t *p = (uint32_t*)from_ptrv(R_ESP);

    va_list va;
    va_start (va, nargs);
    for (int i=0; i<nargs; ++i) {
        uint32_t v = va_arg(va, uint32_t);
        *p = v;
        p++;
    }
    va_end (va);

    printf_log(LOG_DEBUG, "%04d|signal #%d function handler %p called, RSP=%p\n", GetTID(), R_EDI, from_ptrv(fnc), from_ptrv(R_ESP));

    int oldquitonlongjmp = emu->flags.quitonlongjmp;
    emu->flags.quitonlongjmp = 2;
    int old_cs = R_CS;
    R_CS = 0x23;

    emu->eflags.x64 &= ~(1<<F_TF); // this one needs to cleared

    if(dynarec)
        DynaCall(emu, fnc);
    else
        EmuCall(emu, fnc);

    if(!emu->flags.longjmp)
        R_ESP+=nargs*sizeof(ptr_t);

    if(!emu->flags.longjmp && R_CS==0x23)
        R_CS = old_cs;

    emu->flags.quitonlongjmp = oldquitonlongjmp;

    #ifdef DYNAREC
    if(box64_dynarec_test) {
        emu->test.test = 0;
        emu->test.clean = 0;
    }
    #endif

    if(emu->flags.longjmp) {
        // longjmp inside signal handler, lets grab all relevent value and do the actual longjmp in the signal handler
        emu->flags.longjmp = 0;
        if(sigcontext) {
            sigcontext->uc_mcontext.gregs[I386_EAX] = R_EAX;
            sigcontext->uc_mcontext.gregs[I386_ECX] = R_ECX;
            sigcontext->uc_mcontext.gregs[I386_EDX] = R_EDX;
            sigcontext->uc_mcontext.gregs[I386_EDI] = R_EDI;
            sigcontext->uc_mcontext.gregs[I386_ESI] = R_ESI;
            sigcontext->uc_mcontext.gregs[I386_EBP] = R_EBP;
            sigcontext->uc_mcontext.gregs[I386_ESP] = R_ESP;
            sigcontext->uc_mcontext.gregs[I386_EBX] = R_EBX;
            sigcontext->uc_mcontext.gregs[I386_EIP] = R_EIP;
            // flags
            sigcontext->uc_mcontext.gregs[I386_EFL] = emu->eflags.x64;
            // get segments
            sigcontext->uc_mcontext.gregs[I386_CS] = R_CS;
            sigcontext->uc_mcontext.gregs[I386_DS] = R_DS;
            sigcontext->uc_mcontext.gregs[I386_ES] = R_ES;
            sigcontext->uc_mcontext.gregs[I386_SS] = R_SS;
            sigcontext->uc_mcontext.gregs[I386_FS] = R_FS;
            sigcontext->uc_mcontext.gregs[I386_GS] = R_GS;
        } else {
            printf_log(LOG_NONE, "Warning, longjmp in signal but no sigcontext to change\n");
        }
    }
    if(exit)
        *exit = emu->exit;

    uint32_t ret = R_EAX;

#ifdef HAVE_TRACE
    trace_start = old_start; trace_end = old_end;
#endif

    return ret;
}

#define is_memprot_locked (1<<1)
#define is_dyndump_locked (1<<8)
void my_sigactionhandler_oldcode_32(int32_t sig, int simple, siginfo_t* info, void * ucntx, int* old_code, void* cur_db)
{
    int Locks = unlockMutex();

    printf_log(LOG_DEBUG, "Sigactionhanlder for signal #%d called (jump to %p/%s)\n", sig, (void*)my_context->signals[sig], GetNativeName((void*)my_context->signals[sig]));

    uintptr_t restorer = my_context->restorer[sig];
    // get that actual ESP first!
    x64emu_t *emu = thread_get_emu();
    uintptr_t frame = R_RSP;
#if defined(DYNAREC)
#if defined(ARM64)
    dynablock_t* db = (dynablock_t*)cur_db;//FindDynablockFromNativeAddress(pc);
    ucontext_t *p = (ucontext_t *)ucntx;
    void* pc = NULL;
    if(p) {
        pc = (void*)p->uc_mcontext.pc;
        if(db)
            frame = (uintptr_t)p->uc_mcontext.regs[10+_SP];
    }
#elif defined(LA64)
    dynablock_t* db = (dynablock_t*)cur_db;//FindDynablockFromNativeAddress(pc);
    ucontext_t *p = (ucontext_t *)ucntx;
    void* pc = NULL;
    if(p) {
        pc = (void*)p->uc_mcontext.__pc;
        if(db)
            frame = (uintptr_t)p->uc_mcontext.__gregs[12+_SP];
    }
#elif defined(RV64)
    dynablock_t* db = (dynablock_t*)cur_db;//FindDynablockFromNativeAddress(pc);
    ucontext_t *p = (ucontext_t *)ucntx;
    void* pc = NULL;
    if(p) {
        pc = (void*)p->uc_mcontext.__gregs[0];
        if(db)
            frame = (uintptr_t)p->uc_mcontext.__gregs[16+_SP];
    }
#else
#error Unsupported architecture
#endif
#else
    (void)ucntx; (void)cur_db;
#endif
    // setup libc context stack frame, on caller stack
    frame = frame&~15;

    // stack tracking
    x64_stack_t *new_ss = my_context->onstack[sig]?sigstack_getstack():NULL;
    int used_stack = 0;
    if(new_ss) {
        if(new_ss->ss_flags == SS_ONSTACK) { // already using it!
            frame = ((uintptr_t)emu->regs[_SP].q[0] - 128) & ~0x0f;
        } else {
            frame = (uintptr_t)(((uintptr_t)new_ss->ss_sp + new_ss->ss_size - 16) & ~0x0f);
            used_stack = 1;
            new_ss->ss_flags = SS_ONSTACK;
        }
    } else {
        frame -= 0x200; // redzone
    }

    // TODO: do I need to really setup 2 stack frame? That doesn't seems right!
    // setup stack frame
    frame -= 512+64+16*16;
    void* xstate = (void*)frame;
    frame -= sizeof(siginfo_t);
    siginfo_t* info2 = (siginfo_t*)frame;
    memcpy(info2, info, sizeof(siginfo_t));
    // try to fill some sigcontext....
    frame -= sizeof(i386_ucontext_t);
    i386_ucontext_t   *sigcontext = (i386_ucontext_t*)frame;
    // get general register
    sigcontext->uc_mcontext.gregs[I386_EAX] = R_EAX;
    sigcontext->uc_mcontext.gregs[I386_ECX] = R_ECX;
    sigcontext->uc_mcontext.gregs[I386_EDX] = R_EDX;
    sigcontext->uc_mcontext.gregs[I386_EDI] = R_EDI;
    sigcontext->uc_mcontext.gregs[I386_ESI] = R_ESI;
    sigcontext->uc_mcontext.gregs[I386_EBP] = R_EBP;
    sigcontext->uc_mcontext.gregs[I386_ESP] = R_ESP;
    sigcontext->uc_mcontext.gregs[I386_EBX] = R_EBX;
    sigcontext->uc_mcontext.gregs[I386_EIP] = R_EIP;//emu->old_ip;   // old_ip should be more accurate as the "current" IP, but it's not always up-to-date
    // flags
    sigcontext->uc_mcontext.gregs[I386_EFL] = emu->eflags.x64;
    // get segments
    sigcontext->uc_mcontext.gregs[I386_CS] = R_CS;
    sigcontext->uc_mcontext.gregs[I386_DS] = R_DS;
    sigcontext->uc_mcontext.gregs[I386_ES] = R_ES;
    sigcontext->uc_mcontext.gregs[I386_SS] = R_SS;
    sigcontext->uc_mcontext.gregs[I386_FS] = R_FS;
    sigcontext->uc_mcontext.gregs[I386_GS] = R_GS;
#if defined(DYNAREC)
#if defined(ARM64)
    if(db && p) {
        sigcontext->uc_mcontext.gregs[I386_EAX] = p->uc_mcontext.regs[10];
        sigcontext->uc_mcontext.gregs[I386_ECX] = p->uc_mcontext.regs[11];
        sigcontext->uc_mcontext.gregs[I386_EDX] = p->uc_mcontext.regs[12];
        sigcontext->uc_mcontext.gregs[I386_EBX] = p->uc_mcontext.regs[13];
        sigcontext->uc_mcontext.gregs[I386_ESP] = p->uc_mcontext.regs[14];
        sigcontext->uc_mcontext.gregs[I386_EBP] = p->uc_mcontext.regs[15];
        sigcontext->uc_mcontext.gregs[I386_ESI] = p->uc_mcontext.regs[16];
        sigcontext->uc_mcontext.gregs[I386_EDI] = p->uc_mcontext.regs[17];
        sigcontext->uc_mcontext.gregs[I386_EIP] = getX64Address(db, (uintptr_t)pc);
    }
#elif defined(LA64)
    if(db && p) {
        sigcontext->uc_mcontext.gregs[I386_EAX] = p->uc_mcontext.__gregs[12];
        sigcontext->uc_mcontext.gregs[I386_ECX] = p->uc_mcontext.__gregs[13];
        sigcontext->uc_mcontext.gregs[I386_EDX] = p->uc_mcontext.__gregs[14];
        sigcontext->uc_mcontext.gregs[I386_EBX] = p->uc_mcontext.__gregs[15];
        sigcontext->uc_mcontext.gregs[I386_ESP] = p->uc_mcontext.__gregs[16];
        sigcontext->uc_mcontext.gregs[I386_EBP] = p->uc_mcontext.__gregs[17];
        sigcontext->uc_mcontext.gregs[I386_ESI] = p->uc_mcontext.__gregs[18];
        sigcontext->uc_mcontext.gregs[I386_EDI] = p->uc_mcontext.__gregs[19];
        sigcontext->uc_mcontext.gregs[I386_EIP] = getX64Address(db, (uintptr_t)pc);
    }
#elif defined(RV64)
    if(db && p) {
        sigcontext->uc_mcontext.gregs[I386_EAX] = p->uc_mcontext.__gregs[16];
        sigcontext->uc_mcontext.gregs[I386_ECX] = p->uc_mcontext.__gregs[17];
        sigcontext->uc_mcontext.gregs[I386_EDX] = p->uc_mcontext.__gregs[18];
        sigcontext->uc_mcontext.gregs[I386_EBX] = p->uc_mcontext.__gregs[19];
        sigcontext->uc_mcontext.gregs[I386_ESP] = p->uc_mcontext.__gregs[20];
        sigcontext->uc_mcontext.gregs[I386_EBP] = p->uc_mcontext.__gregs[21];
        sigcontext->uc_mcontext.gregs[I386_ESI] = p->uc_mcontext.__gregs[22];
        sigcontext->uc_mcontext.gregs[I386_EDI] = p->uc_mcontext.__gregs[23];
        sigcontext->uc_mcontext.gregs[I386_EIP] = getX64Address(db, (uintptr_t)pc);
    }
#else
#error Unsupported architecture
#endif
#endif
    // get FloatPoint status
    sigcontext->uc_mcontext.fpregs = to_ptrv(xstate);//(struct x64_libc_fpstate*)&sigcontext->xstate;
    fpu_xsave_mask(emu, xstate, 1, 0b111);
    memcpy(&sigcontext->xstate, xstate, sizeof(sigcontext->xstate));
    ((struct i386_fpstate*)xstate)->status_magic = 0x46505853;   // magic number to signal an XSTATE type of fpregs
    // get signal mask

    if(new_ss) {
        sigcontext->uc_stack.ss_sp = to_ptrv(new_ss->ss_sp);
        sigcontext->uc_stack.ss_size = new_ss->ss_size;
        sigcontext->uc_stack.ss_flags = new_ss->ss_flags;
    } else
        sigcontext->uc_stack.ss_flags = SS_DISABLE;
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
        sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 17;
    else if(sig==SIGSEGV) {
        if((uintptr_t)info->si_addr == sigcontext->uc_mcontext.gregs[I386_EIP]) {
            sigcontext->uc_mcontext.gregs[I386_ERR] = (info->si_errno==0x1234)?0:((info->si_errno==0xdead)?(0x2|(info->si_code<<3)):0x0010);    // execution flag issue (probably), unless it's a #GP(0)
            sigcontext->uc_mcontext.gregs[I386_TRAPNO] = ((info->si_code==SEGV_ACCERR) || (info->si_errno==0x1234) || (info->si_errno==0xdead) || ((uintptr_t)info->si_addr==0))?13:14;
        } else if(info->si_code==SEGV_ACCERR && !(prot&PROT_WRITE)) {
            sigcontext->uc_mcontext.gregs[I386_ERR] = 0x0002;    // write flag issue
            sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 14;
        } else {
            if((info->si_code!=SEGV_ACCERR) && labs((intptr_t)info->si_addr-(intptr_t)sigcontext->uc_mcontext.gregs[I386_ESP])<16)
                sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 12; // stack overflow probably
            else
                sigcontext->uc_mcontext.gregs[I386_TRAPNO] = (info->si_code == SEGV_ACCERR)?13:14;
            //I386_ERR seems to be INT:8 CODE:8. So for write access segfault it's 0x0002 For a read it's 0x0004 (and 8 for exec). For an int 2d it could be 0x2D01 for example
            sigcontext->uc_mcontext.gregs[I386_ERR] = 0x0004;    // read error? there is no execute control in box64 anyway
        }
        if(info->si_code == SEGV_ACCERR && old_code)
            *old_code = -1;
        if(info->si_errno==0x1234) {
            info2->si_errno = 0;
        } else if(info->si_errno==0xdead) {
            // INT x
            uint8_t int_n = info2->si_code;
            info2->si_errno = 0;
            info2->si_code = info->si_code;
            info2->si_addr = NULL;
            // some special cases...
            if(int_n==3) {
                info2->si_signo = SIGTRAP;
                sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 3;
                sigcontext->uc_mcontext.gregs[I386_ERR] = 0;
            } else if(int_n==0x04) {
                sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 4;
                sigcontext->uc_mcontext.gregs[I386_ERR] = 0;
            } else if (int_n==0x29 || int_n==0x2c || int_n==0x2d) {
                sigcontext->uc_mcontext.gregs[I386_ERR] = 0x02|(int_n<<3);
            } else {
                sigcontext->uc_mcontext.gregs[I386_ERR] = 0x0a|(int_n<<3);
            }
        } else if(info->si_errno==0xcafe) {
            info2->si_errno = 0;
            sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 0;
            info2->si_signo = SIGFPE;
        }
    } else if(sig==SIGFPE) {
        if (info->si_code == FPE_INTOVF)
            sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 4;
        else
            sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 19;
    } else if(sig==SIGILL)
        sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 6;
    else if(sig==SIGTRAP) {
        info2->si_code = 128;
        sigcontext->uc_mcontext.gregs[I386_TRAPNO] = info->si_code;
        sigcontext->uc_mcontext.gregs[I386_ERR] = 0;
    }
    //TODO: SIGABRT generate what?
    printf_log(LOG_DEBUG, "Signal %d: si_addr=%p, TRAPNO=%d, ERR=%d, RIP=%p\n", sig, (void*)info2->si_addr, sigcontext->uc_mcontext.gregs[I386_TRAPNO], sigcontext->uc_mcontext.gregs[I386_ERR],from_ptrv(sigcontext->uc_mcontext.gregs[I386_EIP]));
    // call the signal handler
    i386_ucontext_t sigcontext_copy = *sigcontext;
    // save old value from emu
    #define GO(A) uint32_t old_##A = R_##A
    GO(EAX);
    GO(EDI);
    GO(ESI);
    GO(EDX);
    GO(ECX);
    GO(EBP);
    #undef GO
    // set stack pointer
    R_ESP = frame;
    // set frame pointer
    R_EBP = sigcontext->uc_mcontext.gregs[I386_EBP];

    int exits = 0;
    int ret;
    int dynarec = 0;
    #ifdef DYNAREC
    if(sig!=SIGSEGV && !(Locks&is_dyndump_locked) && !(Locks&is_memprot_locked))
        dynarec = 1;
    #endif
    ret = RunFunctionHandler32(&exits, dynarec, sigcontext, my_context->signals[info2->si_signo], 3, info2->si_signo, info2, sigcontext);
    // restore old value from emu
    if(used_stack)  // release stack
        new_ss->ss_flags = 0;
    #define GO(A) R_##A = old_##A
    GO(EAX);
    GO(EDI);
    GO(ESI);
    GO(EDX);
    GO(ECX);
    GO(EBP);
    #undef GO

    if(memcmp(sigcontext, &sigcontext_copy, sizeof(i386_ucontext_t))) {
        if(emu->jmpbuf) {
            #define GO(R)   emu->regs[_##R].q[0]=sigcontext->uc_mcontext.gregs[I386_E##R]
            GO(AX);
            GO(CX);
            GO(DX);
            GO(DI);
            GO(SI);
            GO(BP);
            GO(SP);
            GO(BX);
            #undef GO
            emu->ip.q[0]=sigcontext->uc_mcontext.gregs[I386_EIP];
            // flags
            emu->eflags.x64=sigcontext->uc_mcontext.gregs[I386_EFL];
            // get segments
            #define GO(S) if(emu->segs[_##S]!=sigcontext->uc_mcontext.gregs[I386_##S])  emu->segs[_##S]=sigcontext->uc_mcontext.gregs[I386_##S]
            GO(CS);
            GO(DS);
            GO(ES);
            GO(SS);
            GO(GS);
            GO(FS);
            #undef GO
            for(int i=0; i<6; ++i)
                emu->segs_serial[i] = 0;
            printf_log(LOG_DEBUG, "Context has been changed in Sigactionhanlder, doing siglongjmp to resume emu at %p, RSP=%p\n", (void*)R_RIP, (void*)R_RSP);
            if(old_code)
                *old_code = -1;    // re-init the value to allow another segfault at the same place
            //relockMutex(Locks);   // do not relock mutex, because of the siglongjmp, whatever was running is canceled
            #ifdef DYNAREC
            if(Locks & is_dyndump_locked)
                CancelBlock64(1);
            #endif
            #ifdef RV64
            emu->xSPSave = emu->old_savedsp;
            #endif
            #ifdef ANDROID
            siglongjmp(*emu->jmpbuf, 1);
            #else
            siglongjmp(emu->jmpbuf, 1);
            #endif
        }
        printf_log(LOG_INFO, "Warning, context has been changed in Sigactionhanlder%s\n", (sigcontext->uc_mcontext.gregs[I386_EIP]!=sigcontext_copy.uc_mcontext.gregs[I386_EIP])?" (EIP changed)":"");
    }
    // restore regs...
    #define GO(R)   R_##R=sigcontext->uc_mcontext.gregs[I386_##R]
    GO(EAX);
    GO(ECX);
    GO(EDX);
    GO(EDI);
    GO(ESI);
    GO(EBP);
    GO(ESP);
    GO(EBX);
    #undef GO
    emu->eflags.x64=sigcontext->uc_mcontext.gregs[I386_EFL];
    #define GO(R)   R_##R=sigcontext->uc_mcontext.gregs[I386_##R]
    GO(CS);
    GO(DS);
    GO(ES);
    GO(SS);
    GO(GS);
    GO(FS);
    #undef GO

    printf_log(LOG_DEBUG, "Sigactionhanlder main function returned (exit=%d, restorer=%p)\n", exits, (void*)restorer);
    if(exits) {
        //relockMutex(Locks);   // the thread will exit, so no relock there
        #ifdef DYNAREC
        if(Locks & is_dyndump_locked)
            CancelBlock64(1);
        #endif
        exit(ret);
    }
    if(restorer)
        RunFunctionHandler32(&exits, 0, NULL, restorer, 0);
    relockMutex(Locks);
}

void my32_sigactionhandler(int32_t sig, siginfo_t* info, void * ucntx)
{
    #ifdef DYNAREC
    ucontext_t *p = (ucontext_t *)ucntx;
    #ifdef ARM64
    void * pc = (void*)p->uc_mcontext.pc;
    #elif defined(LA64)
    void * pc = (void*)p->uc_mcontext.__pc;
    #elif defined(RV64)
    void * pc = (void*)p->uc_mcontext.__gregs[0];
    #else
    #error Unsupported architecture
    #endif
    dynablock_t* db = FindDynablockFromNativeAddress(pc);
    #else
    void* db = NULL;
    #endif

    my_sigactionhandler_oldcode_32(sig, 0, info, ucntx, NULL, db);
}


EXPORT int my32_sigaction(x64emu_t* emu, int signum, const i386_sigaction_t *act, i386_sigaction_t *oldact)
{
    printf_log(LOG_DEBUG, "Sigaction(signum=%d, act=%p(f=%p, flags=0x%x), old=%p)\n", signum, act, act?from_ptrv(act->_u._sa_handler):NULL, act?act->sa_flags:0, oldact);
    if(signum<0 || signum>MAX_SIGNAL) {
        errno = EINVAL;
        return -1;
    }

    if(signum==SIGSEGV && emu->context->no_sigsegv)
        return 0;

    if(signum==SIGILL && emu->context->no_sigill)
        return 0;
    struct sigaction newact = {0};
    struct sigaction old = {0};
    uintptr_t old_handler = my_context->signals[signum];
    if(act) {
        newact.sa_mask = act->sa_mask;
        newact.sa_flags = act->sa_flags&~0x04000000;  // No sa_restorer...
        if(act->sa_flags&0x04) {
            my_context->signals[signum] = (uintptr_t)act->_u._sa_sigaction;
            my_context->is_sigaction[signum] = 1;
            if(act->_u._sa_handler!=0 && act->_u._sa_handler!=(ptr_t)1) {
                newact.sa_sigaction = my32_sigactionhandler;
            } else
                newact.sa_sigaction = from_ptrv(act->_u._sa_sigaction);
        } else {
            my_context->signals[signum] = (uintptr_t)act->_u._sa_handler;
            my_context->is_sigaction[signum] = 0;
            if(act->_u._sa_handler!=0 && act->_u._sa_handler!=(ptr_t)1) {
                newact.sa_flags|=0x04;
                newact.sa_sigaction = my32_sigactionhandler;
            } else
                newact.sa_handler = from_ptrv(act->_u._sa_handler);
        }
        my_context->restorer[signum] = (act->sa_flags&0x04000000)?(uintptr_t)act->sa_restorer:0;
        my_context->onstack[signum] = (act->sa_flags&SA_ONSTACK)?1:0;
    }
    int ret = 0;
    if(signum!=SIGSEGV && signum!=SIGBUS && signum!=SIGILL && signum!=SIGABRT)
        ret = sigaction(signum, act?&newact:NULL, oldact?&old:NULL);
    if(oldact) {
        oldact->sa_flags = old.sa_flags;
        oldact->sa_mask = old.sa_mask;
        if(old.sa_flags & 0x04)
            oldact->_u._sa_sigaction = to_ptrv(old.sa_sigaction); //TODO should wrap...
        else
            oldact->_u._sa_handler = to_ptrv(old.sa_handler);  //TODO should wrap...
        if(oldact->_u._sa_sigaction == to_ptrv(my32_sigactionhandler) && old_handler)
            oldact->_u._sa_sigaction = to_ptr(old_handler);
        oldact->sa_restorer = 0; // no handling for now...
    }
    return ret;
}
EXPORT int my32___sigaction(x64emu_t* emu, int signum, const i386_sigaction_t *act, i386_sigaction_t *oldact)
__attribute__((alias("my32_sigaction")));

EXPORT int my32_getcontext(x64emu_t* emu, void* ucp)
{
//    printf_log(LOG_NONE, "Warning: call to partially implemented getcontext\n");
    i386_ucontext_t *u = (i386_ucontext_t*)ucp;
    // stack traking
    u->uc_stack.ss_sp = 0;
    u->uc_stack.ss_size = 0;    // this need to filled
    // get general register
    u->uc_mcontext.gregs[I386_EAX] = R_EAX;
    u->uc_mcontext.gregs[I386_ECX] = R_ECX;
    u->uc_mcontext.gregs[I386_EDX] = R_EDX;
    u->uc_mcontext.gregs[I386_EDI] = R_EDI;
    u->uc_mcontext.gregs[I386_ESI] = R_ESI;
    u->uc_mcontext.gregs[I386_EBP] = R_EBP;
    u->uc_mcontext.gregs[I386_EIP] = *(uint32_t*)from_ptrv(R_ESP);
    u->uc_mcontext.gregs[I386_ESP] = R_ESP+4;
    u->uc_mcontext.gregs[I386_EBX] = R_EBX;
    // get segments
    u->uc_mcontext.gregs[I386_GS] = R_GS;
    u->uc_mcontext.gregs[I386_FS] = R_FS;
    u->uc_mcontext.gregs[I386_ES] = R_ES;
    u->uc_mcontext.gregs[I386_DS] = R_DS;
    u->uc_mcontext.gregs[I386_CS] = R_CS;
    u->uc_mcontext.gregs[I386_SS] = R_SS;
    // get FloatPoint status
    if(u->uc_mcontext.fpregs)
        save_fpreg(emu, from_ptrv(u->uc_mcontext.fpregs));
    // get signal mask
    sigprocmask(SIG_SETMASK, NULL, (sigset_t*)&u->uc_sigmask);
    // ensure uc_link is properly initialized
    u->uc_link = to_ptrv(emu->uc_link);

    return 0;
}

EXPORT int my32_setcontext(x64emu_t* emu, void* ucp)
{
//    printf_log(LOG_NONE, "Warning: call to partially implemented setcontext\n");
    i386_ucontext_t *u = (i386_ucontext_t*)ucp;
    // stack tracking
    emu->init_stack = from_ptrv(u->uc_stack.ss_sp);
    emu->size_stack = from_ulong(u->uc_stack.ss_size);
    // set general register
    R_EAX = u->uc_mcontext.gregs[I386_EAX];
    R_ECX = u->uc_mcontext.gregs[I386_ECX];
    R_EDX = u->uc_mcontext.gregs[I386_EDX];
    R_EDI = u->uc_mcontext.gregs[I386_EDI];
    R_ESI = u->uc_mcontext.gregs[I386_ESI];
    R_EBP = u->uc_mcontext.gregs[I386_EBP];
    R_EIP = u->uc_mcontext.gregs[I386_EIP];
    R_ESP = u->uc_mcontext.gregs[I386_ESP];
    R_EBX = u->uc_mcontext.gregs[I386_EBX];
    // get segments
    R_GS = u->uc_mcontext.gregs[I386_GS];
    R_FS = u->uc_mcontext.gregs[I386_FS];
    R_ES = u->uc_mcontext.gregs[I386_ES];
    R_DS = u->uc_mcontext.gregs[I386_DS];
    R_CS = u->uc_mcontext.gregs[I386_CS];
    R_SS = u->uc_mcontext.gregs[I386_SS];
    // set FloatPoint status
    if(u->uc_mcontext.fpregs)
        load_fpreg(emu, from_ptrv(u->uc_mcontext.fpregs));
    // set signal mask
    sigprocmask(SIG_SETMASK, (sigset_t*)&u->uc_sigmask, NULL);
    // set uc_link
    emu->uc_link = from_ptrv(u->uc_link);
    errno = 0;
    return R_EAX;
}

EXPORT int my32_makecontext(x64emu_t* emu, void* ucp, void* fnc, int32_t argc, int32_t* argv)
{
//    printf_log(LOG_NONE, "Warning: call to unimplemented makecontext\n");
    i386_ucontext_t *u = (i386_ucontext_t*)ucp;
    // setup stack
    u->uc_mcontext.gregs[I386_ESP] = to_ptr(u->uc_stack.ss_sp + u->uc_stack.ss_size - 4);
    // setup the function
    u->uc_mcontext.gregs[I386_EIP] = to_ptrv(fnc);
    // setup args
    uint32_t* esp = (uint32_t*)from_ptr(u->uc_mcontext.gregs[I386_ESP]);
    for (int i=0; i<argc; ++i) {
        // push value
        --esp;
        *esp = argv[(argc-1)-i];
    }
    // push the return value
    --esp;
    *esp = to_ptr(my_context->exit_bridge);
    u->uc_mcontext.gregs[I386_ESP] = (uintptr_t)esp;
    
    return 0;
}

EXPORT int my32_swapcontext(x64emu_t* emu, void* ucp1, void* ucp2)
{
//    printf_log(LOG_NONE, "Warning: call to unimplemented swapcontext\n");
    // grab current context in ucp1
    my32_getcontext(emu, ucp1);
    // activate ucp2
    my32_setcontext(emu, ucp2);
    return 0;
}
