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
#endif


/* Definitions taken from the kernel headers.  */
enum
{
  REG_GS = 0,
# define REG_GS         REG_GS
  REG_FS,
# define REG_FS         REG_FS
  REG_ES,
# define REG_ES         REG_ES
  REG_DS,
# define REG_DS         REG_DS
  REG_EDI,
# define REG_EDI        REG_EDI
  REG_ESI,
# define REG_ESI        REG_ESI
  REG_EBP,
# define REG_EBP        REG_EBP
  REG_ESP,
# define REG_ESP        REG_ESP
  REG_EBX,
# define REG_EBX        REG_EBX
  REG_EDX,
# define REG_EDX        REG_EDX
  REG_ECX,
# define REG_ECX        REG_ECX
  REG_EAX,
# define REG_EAX        REG_EAX
  REG_TRAPNO,
# define REG_TRAPNO        REG_TRAPNO
  REG_ERR,
# define REG_ERR        REG_ERR
  REG_EIP,
# define REG_EIP        REG_EIP
  REG_CS,
# define REG_CS                REG_CS
  REG_EFL,
# define REG_EFL        REG_EFL
  REG_UESP,
# define REG_UESP        REG_UESP
  REG_SS
# define REG_SS        REG_SS
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
  unsigned short    status;
  unsigned short    magic;
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

EXPORT int my32_getcontext(x64emu_t* emu, void* ucp)
{
//    printf_log(LOG_NONE, "Warning: call to partially implemented getcontext\n");
    i386_ucontext_t *u = (i386_ucontext_t*)ucp;
    // stack traking
    u->uc_stack.ss_sp = 0;
    u->uc_stack.ss_size = 0;    // this need to filled
    // get general register
    u->uc_mcontext.gregs[REG_EAX] = R_EAX;
    u->uc_mcontext.gregs[REG_ECX] = R_ECX;
    u->uc_mcontext.gregs[REG_EDX] = R_EDX;
    u->uc_mcontext.gregs[REG_EDI] = R_EDI;
    u->uc_mcontext.gregs[REG_ESI] = R_ESI;
    u->uc_mcontext.gregs[REG_EBP] = R_EBP;
    u->uc_mcontext.gregs[REG_EIP] = *(uint32_t*)from_ptrv(R_ESP);
    u->uc_mcontext.gregs[REG_ESP] = R_ESP+4;
    u->uc_mcontext.gregs[REG_EBX] = R_EBX;
    // get segments
    u->uc_mcontext.gregs[REG_GS] = R_GS;
    u->uc_mcontext.gregs[REG_FS] = R_FS;
    u->uc_mcontext.gregs[REG_ES] = R_ES;
    u->uc_mcontext.gregs[REG_DS] = R_DS;
    u->uc_mcontext.gregs[REG_CS] = R_CS;
    u->uc_mcontext.gregs[REG_SS] = R_SS;
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
    R_EAX = u->uc_mcontext.gregs[REG_EAX];
    R_ECX = u->uc_mcontext.gregs[REG_ECX];
    R_EDX = u->uc_mcontext.gregs[REG_EDX];
    R_EDI = u->uc_mcontext.gregs[REG_EDI];
    R_ESI = u->uc_mcontext.gregs[REG_ESI];
    R_EBP = u->uc_mcontext.gregs[REG_EBP];
    R_EIP = u->uc_mcontext.gregs[REG_EIP];
    R_ESP = u->uc_mcontext.gregs[REG_ESP];
    R_EBX = u->uc_mcontext.gregs[REG_EBX];
    // get segments
    R_GS = u->uc_mcontext.gregs[REG_GS];
    R_FS = u->uc_mcontext.gregs[REG_FS];
    R_ES = u->uc_mcontext.gregs[REG_ES];
    R_DS = u->uc_mcontext.gregs[REG_DS];
    R_CS = u->uc_mcontext.gregs[REG_CS];
    R_SS = u->uc_mcontext.gregs[REG_SS];
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
    u->uc_mcontext.gregs[REG_ESP] = to_ptr(u->uc_stack.ss_sp + u->uc_stack.ss_size - 4);
    // setup the function
    u->uc_mcontext.gregs[REG_EIP] = to_ptrv(fnc);
    // setup args
    uint32_t* esp = (uint32_t*)from_ptr(u->uc_mcontext.gregs[REG_ESP]);
    for (int i=0; i<argc; ++i) {
        // push value
        --esp;
        *esp = argv[(argc-1)-i];
    }
    // push the return value
    --esp;
    *esp = to_ptr(my_context->exit_bridge);
    u->uc_mcontext.gregs[REG_ESP] = (uintptr_t)esp;
    
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
