#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
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

#include "x64_signals.h"
#include "os.h"
#include "box32context.h"
#include "debug.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "signals.h"
#include "box64stack.h"
#include "box64cpu.h"
#include "callback.h"
#include "elfloader.h"
#include "threads.h"
#include "emu/x87emu_private.h"
#include "custommem.h"
#include "bridge.h"
#ifdef DYNAREC
#include "dynablock.h"
#include "../dynarec/dynablock_private.h"
#include "dynarec_native.h"
#if defined(ARM64)
#include "dynarec/arm64/arm64_mapping.h"
#define CONTEXT_REG(P, X)   P->uc_mcontext.regs[X]
#define CONTEXT_PC(P)       P->uc_mcontext.pc
#elif defined(LA64)
#include "dynarec/la64/la64_mapping.h"
#define CONTEXT_REG(P, X)   P->uc_mcontext.__gregs[X]
#define CONTEXT_PC(P)       P->uc_mcontext.__pc;
#elif defined(RV64)
#include "dynarec/rv64/rv64_mapping.h"
#define CONTEXT_REG(P, X)   P->uc_mcontext.__gregs[X]
#define CONTEXT_PC(P)       P->uc_mcontext.__gregs[REG_PC]
#else
#error Unsupported Architecture
#endif //arch
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

typedef union my_sigval32
{
  int sival_int;
  ptr_t sival_ptr;
} my_sigval32_t;

typedef struct __attribute__((packed, aligned(4))) my_siginfo32_s
{
    int si_signo;
    int si_errno;
    int si_code;
    union {
	    int _pad[128/sizeof(int)-3];
	struct {
	    __pid_t __si_pid;
	    __uid_t __si_uid;
    } _kill;
	struct {
	    int si_tid;
	    int __si_overrun;
	    my_sigval32_t si_sigval;
    } _timer;
	struct {
	    __pid_t __si_pid;
	    __uid_t __si_uid;
	    my_sigval32_t si_sigval;
    } _rt;
	struct {
	    __pid_t __si_pid;
	    __uid_t __si_uid;
	    int __si_status;
	    int32_t __si_utime;
	    int32_t __si_stime;
    } _sigchld;
	struct {
	    ptr_t __si_addr;
	    __SI_SIGFAULT_ADDL
	    int16_t __si_addr_lsb;
	    union {
            struct {
                ptr_t _lower;
                ptr_t _upper;
            } _addr_bnd;
            uint32_t _pkey;
        } _bounds;
    } _sigfault;
	struct
	  {
	    int32_t __si_band;
	    int32_t __si_fd;
	  } _sigpoll;
	struct
	  {
	    ptr_t _call_addr;
	    int32_t _syscall;
	    uint32_t _arch;
	  } _sigsys;
    } _sifields;
} my_siginfo32_t;

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
        printf_log(LOG_NONE, "%04d|Warning, calling Signal %d function handler %s\n", GetTID(), sig, fnc ? "SIG_IGN" : "SIG_DFL");
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
    x64emu_t *emu = thread_get_emu();
    #ifdef DYNAREC
    if (BOX64ENV(dynarec_test))
        emu->test.test = 0;
    #endif

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
    if (BOX64ENV(dynarec_test)) {
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
void convert_siginfo_to_32(void* d, void* s, int sig)
{
    if(!s || !d) return;
    my_siginfo32_t* dst = d;
    siginfo_t* src = s;

    dst->si_signo = src->si_signo;
    dst->si_errno = src->si_errno;
    dst->si_code = src->si_code;
    memcpy(&dst->_sifields, &src->_sifields, sizeof(siginfo_t) - offsetof(siginfo_t, _sifields));
    if(sig==X64_SIGILL || sig==X64_SIGFPE || sig==X64_SIGSEGV || sig==X64_SIGBUS)
        dst->_sifields._sigfault.__si_addr = to_ptr(((uintptr_t)src->si_addr)&0xffffffff);  // in case addr is not a 32bits value...
    if(sig==X64_SIGCHLD) {
        dst->_sifields._sigchld.__si_pid = src->si_pid;
        dst->_sifields._sigchld.__si_uid = src->si_uid;
        dst->_sifields._sigchld.__si_status = src->si_status;
        dst->_sifields._sigchld.__si_stime = src->si_stime;
        dst->_sifields._sigchld.__si_utime = src->si_utime;
    }
    // TODO: convert si_sigval when needed
}

void relockMutex(int locks);
int unlockMutex();

int write_opcode(uintptr_t rip, uintptr_t native_ip, int is32bits);
#define is_memprot_locked (1<<1)
#define is_dyndump_locked (1<<8)
void my_sigactionhandler_oldcode_32(x64emu_t* emu, int32_t sig, int simple, siginfo_t* info, void * ucntx, int* old_code, void* cur_db)
{
    int Locks = unlockMutex();
    int log_minimum = (BOX64ENV(showsegv))?LOG_NONE:((sig==X64_SIGSEGV && my_context->is_sigaction[sig])?LOG_DEBUG:LOG_INFO);

    printf_log(LOG_DEBUG, "Sigactionhanlder32 for signal #%d called (jump to %p/%s)\n", sig, (void*)my_context->signals[sig], GetNativeName((void*)my_context->signals[sig], 1));

    uintptr_t restorer = my_context->restorer[sig];
    // get that actual ESP first!
    if(!emu)
        emu = thread_get_emu();
    uintptr_t frame = R_ESP;
#if defined(DYNAREC)
    dynablock_t* db = (dynablock_t*)cur_db;//FindDynablockFromNativeAddress(pc);
    ucontext_t *p = (ucontext_t *)ucntx;
    void* pc = NULL;
    if(p) {
        pc = (void*)CONTEXT_PC(p);
        if(db)
            frame = from_ptr((ptr_t)CONTEXT_REG(p, xRSP));
    }
#else
    (void)ucntx; (void)cur_db;
    void* pc = NULL;
#endif
    // setup libc context stack frame, on caller stack
    frame = frame&~15;

    // stack tracking
    x64_stack_t *new_ss = my_context->onstack[sig]?sigstack_getstack():NULL;
    int used_stack = 0;
    if(new_ss) {
        if(new_ss->ss_flags == SS_ONSTACK) { // already using it!
            frame = from_ptr(((ptr_t)emu->regs[_SP].q[0] - 128) & ~0x0f);
        } else {
            frame = from_ptr(((uintptr_t)new_ss->ss_sp + new_ss->ss_size - 16) & ~0x0f);
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
    frame -= sizeof(my_siginfo32_t);
    my_siginfo32_t* info2 = (my_siginfo32_t*)frame;
    convert_siginfo_to_32(info2, info, sig);
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
    CLEAR_FLAG(F_TF);   // now clear TF flags inside the signal handler
    // get segments
    sigcontext->uc_mcontext.gregs[I386_CS] = R_CS;
    sigcontext->uc_mcontext.gregs[I386_DS] = R_DS;
    sigcontext->uc_mcontext.gregs[I386_ES] = R_ES;
    sigcontext->uc_mcontext.gregs[I386_SS] = R_SS;
    sigcontext->uc_mcontext.gregs[I386_FS] = R_FS;
    sigcontext->uc_mcontext.gregs[I386_GS] = R_GS;
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
    uint32_t mmapped = memExist((uintptr_t)info->si_addr);
    uint32_t sysmapped = (info->si_addr<(void*)box64_pagesize)?1:mmapped;
    uint32_t real_prot = 0;
    int skip = 1;   // in case sigjump is used to restore exectuion, 1 will switch to interpreter, 3 will switch to dynarec
    if(prot&PROT_READ) real_prot|=PROT_READ;
    if(prot&PROT_WRITE) real_prot|=PROT_WRITE;
    if(prot&PROT_EXEC) real_prot|=PROT_WRITE;
    if(prot&PROT_DYNAREC) real_prot|=PROT_WRITE;
    sigcontext->uc_mcontext.gregs[I386_ERR] = 0;
    sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 0;
    if(sig==X64_SIGBUS)
        sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 17;
    else if(sig==X64_SIGSEGV) {
        if((uintptr_t)info->si_addr == sigcontext->uc_mcontext.gregs[I386_EIP]) {
            if(info->si_errno==0xbad0) {
                //bad opcode
                sigcontext->uc_mcontext.gregs[I386_ERR] = 0;
                sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 13;
                info2->si_code = 128;
                info2->si_errno = 0;
                info2->_sifields._sigfault.__si_addr = 0;
            } else if (info->si_errno==0xecec) {
                // no excute bit on segment
                sigcontext->uc_mcontext.gregs[I386_ERR] = 0x14|((sysmapped && !(real_prot&PROT_READ))?0:1);
                sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 14;
                if(!mmapped) info2->si_code = 1;
                info2->si_errno = 0;
            }else {
                sigcontext->uc_mcontext.gregs[I386_ERR] = 0x14|((sysmapped && !(real_prot&PROT_READ))?0:1);
                sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 14;
            }
        } else {
            sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 14;
            sigcontext->uc_mcontext.gregs[I386_ERR] = 4|((sysmapped && !(real_prot&PROT_READ))?0:1);
            if(write_opcode(sigcontext->uc_mcontext.gregs[I386_EIP], (uintptr_t)pc, 1))
                sigcontext->uc_mcontext.gregs[I386_ERR] |= 2;
        }
        if(info->si_code == SEGV_ACCERR && old_code)
            *old_code = -1;
        if(info->si_errno==0x1234) {
            sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 13;
            info2->si_errno = 0;
        } else if(info->si_errno==0xdead) {
            // INT x
            uint8_t int_n = info->si_code;
            info2->si_errno = 0;
            info2->si_code = 128;
            info2->_sifields._sigfault.__si_addr = 0;
            sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 13;
            skip = 3;   // can resume in dynarec
            // some special cases...
            if(int_n==3) {
                info2->si_signo = X64_SIGTRAP;
                sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 3;
                sigcontext->uc_mcontext.gregs[I386_ERR] = 0;
            } else if(int_n==0x04) {
                sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 4;
                sigcontext->uc_mcontext.gregs[I386_ERR] = 0;
            } else if (int_n==0x29 || int_n==0x2c || int_n==0x2d) {
                sigcontext->uc_mcontext.gregs[I386_ERR] = 0x02|(int_n<<3);
            } else {
                sigcontext->uc_mcontext.gregs[I386_ERR] = 0x0a|(int_n<<3);
                sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 13;
            }
        } else if(info->si_errno==0xcafe) { //divide by 0
            info2->si_errno = 0;
            sigcontext->uc_mcontext.gregs[I386_ERR] = 0;
            sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 0;
            info2->si_signo = X64_SIGFPE;
            skip = 3; // can resume in dynarec
        }
    } else if(sig==X64_SIGFPE) {
        if (info->si_code == FPE_INTOVF)
            sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 4;
        else
            sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 19;
        skip = 3;
    } else if(sig==X64_SIGILL) {
        info2->si_code = 2;
        sigcontext->uc_mcontext.gregs[I386_TRAPNO] = 6;
        info2->_sifields._sigfault.__si_addr = sigcontext->uc_mcontext.gregs[I386_EIP];
    } else if(sig==X64_SIGTRAP) {
        if(info->si_code==1) {  //single step
            info2->si_code = 2;
            info2->_sifields._sigfault.__si_addr = sigcontext->uc_mcontext.gregs[I386_EIP];
        } else
            info2->si_code = 128;
        sigcontext->uc_mcontext.gregs[I386_TRAPNO] = info->si_code;
        sigcontext->uc_mcontext.gregs[I386_ERR] = 0;
    } else {
        skip = 3;   // other signal can resume in interpretor
    }
    //TODO: SIGABRT generate what?
    printf_log((sig==10)?LOG_DEBUG:log_minimum, "Signal32 %d: si_addr=%p, TRAPNO=%d, ERR=%d, RIP=%p, prot:%x, mmaped:%d\n", sig, from_ptrv(info2->_sifields._sigfault.__si_addr), sigcontext->uc_mcontext.gregs[I386_TRAPNO], sigcontext->uc_mcontext.gregs[I386_ERR],from_ptrv(sigcontext->uc_mcontext.gregs[I386_EIP]), prot, mmapped);
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
    if(sig!=X64_SIGSEGV && !(Locks&is_dyndump_locked) && !(Locks&is_memprot_locked))
        dynarec = BOX64ENV(dynarec_interp_signal)?0:1;
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
            if((skip==1) && (emu->ip.q[0]!=sigcontext->uc_mcontext.gregs[I386_EIP]))
                skip = 3;   // if it jumps elsewhere, it can resume with dynarec...
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
            printf_log((sig==10)?LOG_DEBUG:log_minimum, "Context has been changed in Sigactionhanlder, doing siglongjmp to resume emu at %p, RSP=%p (resume with %s)\n", (void*)R_RIP, (void*)R_RSP, (skip==3)?"Dynarec":"Interp");
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
            #ifdef DYNAREC
            dynablock_leave_runtime((dynablock_t*)cur_db);
            #endif
            #ifdef ANDROID
            siglongjmp(*emu->jmpbuf, skip);
            #else
            siglongjmp(emu->jmpbuf, skip);
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

    my_sigactionhandler_oldcode_32(NULL, sig, 0, info, ucntx, NULL, db);
}


EXPORT int my32_sigaction(x64emu_t* emu, int signum, const i386_sigaction_t *act, i386_sigaction_t *oldact)
{
    printf_log(LOG_DEBUG, "Sigaction(signum=%d, act=%p(f=%p, flags=0x%x), old=%p)\n", signum, act, act?from_ptrv(act->_u._sa_handler):NULL, act?act->sa_flags:0, oldact);
    if(signum<0 || signum>MAX_SIGNAL) {
        errno = EINVAL;
        return -1;
    }

    if(signum==X64_SIGSEGV && emu->context->no_sigsegv)
        return 0;

    if(signum==X64_SIGILL && emu->context->no_sigill)
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
    if(signum!=X64_SIGSEGV && signum!=X64_SIGBUS && signum!=X64_SIGILL && signum!=X64_SIGABRT)
        ret = sigaction(signal_from_x64(signum), act?&newact:NULL, oldact?&old:NULL);
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
    u->uc_mcontext.fpregs = to_ptrv(ucp + 236);    // magic offset of fpregs in an actual i386 u_context
    fpu_savenv(emu, from_ptrv(u->uc_mcontext.fpregs), 1);   // it seems getcontext only save fpu env, not fpu regs
    // get signal mask
    sigprocmask(SIG_SETMASK, NULL, (sigset_t*)&u->uc_sigmask);

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
    R_RAX = u->uc_mcontext.gregs[I386_EAX];
    R_RCX = u->uc_mcontext.gregs[I386_ECX];
    R_RDX = u->uc_mcontext.gregs[I386_EDX];
    R_RDI = u->uc_mcontext.gregs[I386_EDI];
    R_RSI = u->uc_mcontext.gregs[I386_ESI];
    R_RBP = u->uc_mcontext.gregs[I386_EBP];
    R_RIP = u->uc_mcontext.gregs[I386_EIP];
    R_RSP = u->uc_mcontext.gregs[I386_ESP];
    R_RBX = u->uc_mcontext.gregs[I386_EBX];
    // get segments
    R_GS = u->uc_mcontext.gregs[I386_GS];
    R_FS = u->uc_mcontext.gregs[I386_FS];
    R_ES = u->uc_mcontext.gregs[I386_ES];
    R_DS = u->uc_mcontext.gregs[I386_DS];
    R_CS = u->uc_mcontext.gregs[I386_CS];
    R_SS = u->uc_mcontext.gregs[I386_SS];
    // set FloatPoint status
    fpu_loadenv(emu, from_ptrv(u->uc_mcontext.fpregs), 1);
    // set signal mask
    sigprocmask(SIG_SETMASK, (sigset_t*)&u->uc_sigmask, NULL);

    errno = 0;
    return R_EAX;
}

void vFEv_32(x64emu_t *emu, uintptr_t fnc);
EXPORT void my32_start_context(x64emu_t* emu)
{
    // this is call indirectly by swapcontext from a makecontext, and will link context or just exit
    i386_ucontext_t *u = (i386_ucontext_t*)from_ptriv(R_EBX);
    if(u)
        my32_setcontext(emu, u);
    else
        emu->quit = 1;
}

EXPORT void my32_makecontext(x64emu_t* emu, void* ucp, void* fnc, int32_t argc, int32_t* argv)
{
//    printf_log(LOG_NONE, "Warning: call to unimplemented makecontext\n");
    i386_ucontext_t *u = (i386_ucontext_t*)ucp;
    // setup stack
    u->uc_mcontext.gregs[I386_ESP] = to_ptr(u->uc_stack.ss_sp + u->uc_stack.ss_size - 4);
    // setup the function
    u->uc_mcontext.gregs[I386_EIP] = to_ptrv(fnc);
    uint32_t* esp = (uint32_t*)from_ptr(u->uc_mcontext.gregs[I386_ESP]);
    // setup return to private start_context uc_link
    *esp = u->uc_link;
    u->uc_mcontext.gregs[I386_EBX] = (uintptr_t)esp;
    --esp;
    // setup args
    for (int i=0; i<argc; ++i) {
        // push value
        --esp;
        *esp = argv[(argc-1)-i];
    }
    // push the return value
    --esp;
    *esp = AddCheckBridge(my_context->system, vFEv_32, my32_start_context, 0, "my_start_context");//my_context->exit_bridge;//to_ptr(my_context->exit_bridge);
    u->uc_mcontext.gregs[I386_ESP] = (uintptr_t)esp;
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
