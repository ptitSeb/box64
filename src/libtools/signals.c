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
#include "bridge.h"
#include "khash.h"
#ifdef DYNAREC
#include "dynablock.h"
#include "../dynarec/dynablock_private.h"
#include "dynarec_native.h"
#include "dynarec/dynarec_arch.h"
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
    box_free(ss);
}

static pthread_key_t sigstack_key;
static pthread_once_t sigstack_key_once = PTHREAD_ONCE_INIT;

static void sigstack_key_alloc() {
    pthread_key_create(&sigstack_key, sigstack_destroy);
}

x64_stack_t* sigstack_getstack() {
    return (x64_stack_t*)pthread_getspecific(sigstack_key);
}

#ifndef DYNAREC
typedef void dynablock_t;
dynablock_t* FindDynablockFromNativeAddress(void* addr) {return NULL;}
uintptr_t getX64Address(dynablock_t* db, uintptr_t pc) {return 0;}
#endif

// this allow handling "safe" function that just abort if accessing a bad address
static __thread JUMPBUFF signal_jmpbuf;
#ifdef ANDROID
#define SIG_JMPBUF signal_jmpbuf
#else
#define SIG_JMPBUF &signal_jmpbuf
#endif
static __thread int signal_jmpbuf_active = 0;


//1<<1 is mutex_prot, 1<<8 is mutex_dyndump
#define is_memprot_locked (1<<1)
#define is_dyndump_locked (1<<8)
uint64_t RunFunctionHandler(x64emu_t* emu, int* exit, int dynarec, x64_ucontext_t* sigcontext, uintptr_t fnc, int nargs, ...)
{
    if(fnc==0 || fnc==1) {
        va_list va;
        va_start (va, nargs);
        int sig = va_arg(va, int);
        va_end (va);
        printf_log(LOG_NONE, "%04d|Warning, calling Signal %d function handler %s\n", GetTID(), sig, fnc?"SIG_IGN":"SIG_DFL");
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
    if(!emu)
        emu = thread_get_emu();
    #ifdef DYNAREC
    if (BOX64ENV(dynarec_test))
        emu->test.test = 0;
    #endif

    /*SetFS(emu, default_fs);*/
    for (int i=0; i<6; ++i)
        emu->segs_serial[i] = 0;

    int align = nargs&1;

    if(nargs>6)
        R_RSP -= (nargs-6+align)*sizeof(void*);   // need to push in reverse order

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

    printf_log(LOG_DEBUG, "%04d|signal #%d function handler %p called, RSP=%p\n", GetTID(), R_EDI, (void*)fnc, (void*)R_RSP);

    int oldquitonlongjmp = emu->flags.quitonlongjmp;
    emu->flags.quitonlongjmp = 2;
    int old_cs = R_CS;
    R_CS = 0x33;

    emu->eflags.x64 &= ~(1<<F_TF); // this one needs to cleared

    if(dynarec)
        DynaCall(emu, fnc);
    else
        EmuCall(emu, fnc);

    if(nargs>6 && !emu->flags.longjmp)
        R_RSP+=((nargs-6+align)*sizeof(void*));

    if(!emu->flags.longjmp && R_CS==0x33)
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
            sigcontext->uc_mcontext.gregs[X64_RSP] = R_RSP;
            sigcontext->uc_mcontext.gregs[X64_RBX] = R_RBX;
            sigcontext->uc_mcontext.gregs[X64_RIP] = R_RIP;
            // flags
            sigcontext->uc_mcontext.gregs[X64_EFL] = emu->eflags.x64;
            // get segments
            sigcontext->uc_mcontext.gregs[X64_CSGSFS] = ((uint64_t)(R_CS)) | (((uint64_t)(R_GS))<<16) | (((uint64_t)(R_FS))<<32);
        } else {
            printf_log(LOG_NONE, "Warning, longjmp in signal but no sigcontext to change\n");
        }
    }
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
    signal_jmpbuf_active = 1;
    if(sigsetjmp(SIG_JMPBUF, 1)) {
        // segfault while gathering function name...
        errno = EFAULT;
        return -1;
    }

    x64_stack_t *new_ss = (x64_stack_t*)pthread_getspecific(sigstack_key);
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
    if(!ss) {
        signal_jmpbuf_active = 0;
        return 0;
    }
    printf_log(LOG_DEBUG, "%04d|sigaltstack called ss=%p[flags=0x%x, sp=%p, ss=0x%lx], oss=%p\n", GetTID(), ss, ss->ss_flags, ss->ss_sp, ss->ss_size, oss);
    if(ss->ss_flags && ss->ss_flags!=SS_DISABLE && ss->ss_flags!=SS_ONSTACK) {
        errno = EINVAL;
        signal_jmpbuf_active = 0;
        return -1;
    }

    if(ss->ss_flags==SS_DISABLE) {
        if(new_ss)
            box_free(new_ss);
        pthread_setspecific(sigstack_key, NULL);
        signal_jmpbuf_active = 0;
        return 0;
    }

    if(!new_ss)
        new_ss = (x64_stack_t*)box_calloc(1, sizeof(x64_stack_t));
    new_ss->ss_flags = 0;
    new_ss->ss_sp = ss->ss_sp;
    new_ss->ss_size = ss->ss_size;

    pthread_setspecific(sigstack_key, new_ss);
    signal_jmpbuf_active = 0;
    return 0;
}

#ifdef DYNAREC
uintptr_t getX64Address(dynablock_t* db, uintptr_t native_addr)
{
    uintptr_t x64addr = (uintptr_t)db->x64_addr;
    uintptr_t armaddr = (uintptr_t)db->block;
    if(native_addr<(uintptr_t)db->block || native_addr>(uintptr_t)db->block+db->size)
        return 0;
    int i = 0;
    do {
        int x64sz = 0;
        int armsz = 0;
        do {
            x64sz+=db->instsize[i].x64;
            armsz+=db->instsize[i].nat*4;
            ++i;
        } while((db->instsize[i-1].x64==15) || (db->instsize[i-1].nat==15));
        // if the opcode is a NOP on ARM side (so armsz==0), it cannot be an address to find
        if((native_addr>=armaddr) && (native_addr<(armaddr+armsz)))
            return x64addr;
        armaddr+=armsz;
        x64addr+=x64sz;
    } while(db->instsize[i].x64 || db->instsize[i].nat);
    return x64addr;
}
int getX64AddressInst(dynablock_t* db, uintptr_t x64pc)
{
    uintptr_t x64addr = (uintptr_t)db->x64_addr;
    uintptr_t armaddr = (uintptr_t)db->block;
    int ret = 0;
    if(x64pc<(uintptr_t)db->x64_addr || x64pc>(uintptr_t)db->x64_addr+db->x64_size)
        return -1;
    int i = 0;
    do {
        int x64sz = 0;
        int armsz = 0;
        do {
            x64sz+=db->instsize[i].x64;
            armsz+=db->instsize[i].nat*4;
            ++i;
        } while((db->instsize[i-1].x64==15) || (db->instsize[i-1].nat==15));
        // if the opcode is a NOP on ARM side (so armsz==0), it cannot be an address to find
        if((x64pc>=x64addr) && (x64pc<(x64addr+x64sz)))
            return ret;
        armaddr+=armsz;
        x64addr+=x64sz;
        ret++;
    } while(db->instsize[i].x64 || db->instsize[i].nat);
    return ret;
}
x64emu_t* getEmuSignal(x64emu_t* emu, ucontext_t* p, dynablock_t* db)
{
#if defined(ARM64)
        if(db && p->uc_mcontext.regs[0]>0x10000) {
            emu = (x64emu_t*)p->uc_mcontext.regs[0];
        }
#elif defined(LA64)
        if(db && p->uc_mcontext.__gregs[4]>0x10000) {
            emu = (x64emu_t*)p->uc_mcontext.__gregs[4];
        }
#elif defined(RV64)
        if(db && p->uc_mcontext.__gregs[25]>0x10000) {
            emu = (x64emu_t*)p->uc_mcontext.__gregs[25];
        }
#else
#error Unsupported Architecture
#endif //arch
    return emu;
}
#endif
int write_opcode(uintptr_t rip, uintptr_t native_ip, int is32bits);
void adjustregs(x64emu_t* emu) {
// tests some special cases
    uint8_t* mem = (uint8_t*)R_RIP;
    rex_t rex = {0};
    int rep = 0;
    int is66 = 0;
    int idx = 0;
    rex.is32bits = (R_CS==0x0023);
    while ((mem[idx]>=0x40 && mem[idx]<=0x4f && !rex.is32bits) || mem[idx]==0xF2 || mem[idx]==0xF3 || mem[idx]==0x66) {
        switch(mem[idx]) {
            case 0x40 ... 0x4f:
                rex.rex = mem[idx];
                break;
            case 0xF2:
            case 0xF3:
                rep = mem[idx]-0xF1;
                break;
            case 0x66:
                is66 = 1;
                break;
        }
        ++idx;
    }
    dynarec_log(LOG_INFO, "Checking opcode: rex=%02hhx is32bits=%d, rep=%d is66=%d %02hhX %02hhX %02hhX %02hhX\n", rex.rex, rex.is32bits, rep, is66, mem[idx+0], mem[idx+1], mem[idx+2], mem[idx+3]);
#ifdef DYNAREC
#ifdef ARM64
    if(mem[idx+0]==0xA4) {
        // (rep) movsb, read done, write not...
        if(emu->eflags.f._F_DF)
            R_RSI++;
        else
            R_RSI--;
        return;
    }
    if(mem[idx+0]==0xA5) {
        // (rep) movsd, read done, write not...
        int step = (emu->eflags.f._F_DF)?-1:+1;
        if(rex.w) step*=8;
        else if(is66) step *=2;
        else step*=4;
        R_RSI-=step;
        return;
    }
#elif defined(LA64)
#elif defined(RV64)
#else
#error  Unsupported architecture
#endif
#endif
}

void copyUCTXreg2Emu(x64emu_t* emu, ucontext_t* p, uintptr_t ip) {
#ifdef DYNAREC
#ifdef ARM64
    emu->regs[_AX].q[0] = p->uc_mcontext.regs[10];
    emu->regs[_CX].q[0] = p->uc_mcontext.regs[11];
    emu->regs[_DX].q[0] = p->uc_mcontext.regs[12];
    emu->regs[_BX].q[0] = p->uc_mcontext.regs[13];
    emu->regs[_SP].q[0] = p->uc_mcontext.regs[14];
    emu->regs[_BP].q[0] = p->uc_mcontext.regs[15];
    emu->regs[_SI].q[0] = p->uc_mcontext.regs[16];
    emu->regs[_DI].q[0] = p->uc_mcontext.regs[17];
    emu->regs[_R8].q[0] = p->uc_mcontext.regs[18];
    emu->regs[_R9].q[0] = p->uc_mcontext.regs[19];
    emu->regs[_R10].q[0] = p->uc_mcontext.regs[20];
    emu->regs[_R11].q[0] = p->uc_mcontext.regs[21];
    emu->regs[_R12].q[0] = p->uc_mcontext.regs[22];
    emu->regs[_R13].q[0] = p->uc_mcontext.regs[23];
    emu->regs[_R14].q[0] = p->uc_mcontext.regs[24];
    emu->regs[_R15].q[0] = p->uc_mcontext.regs[25];
    emu->ip.q[0] = ip;
    emu->eflags.x64 = p->uc_mcontext.regs[26];
#elif defined(LA64)
    emu->regs[_AX].q[0] = p->uc_mcontext.__gregs[12];
    emu->regs[_CX].q[0] = p->uc_mcontext.__gregs[13];
    emu->regs[_DX].q[0] = p->uc_mcontext.__gregs[14];
    emu->regs[_BX].q[0] = p->uc_mcontext.__gregs[15];
    emu->regs[_SP].q[0] = p->uc_mcontext.__gregs[16];
    emu->regs[_BP].q[0] = p->uc_mcontext.__gregs[17];
    emu->regs[_SI].q[0] = p->uc_mcontext.__gregs[18];
    emu->regs[_DI].q[0] = p->uc_mcontext.__gregs[19];
    emu->regs[_R8].q[0] = p->uc_mcontext.__gregs[23];
    emu->regs[_R9].q[0] = p->uc_mcontext.__gregs[24];
    emu->regs[_R10].q[0] = p->uc_mcontext.__gregs[25];
    emu->regs[_R11].q[0] = p->uc_mcontext.__gregs[26];
    emu->regs[_R12].q[0] = p->uc_mcontext.__gregs[27];
    emu->regs[_R13].q[0] = p->uc_mcontext.__gregs[28];
    emu->regs[_R14].q[0] = p->uc_mcontext.__gregs[29];
    emu->regs[_R15].q[0] = p->uc_mcontext.__gregs[30];
    emu->ip.q[0] = ip;
    emu->eflags.x64 = p->uc_mcontext.__gregs[31];
#elif defined(RV64)
    emu->regs[_AX].q[0] = p->uc_mcontext.__gregs[16];
    emu->regs[_CX].q[0] = p->uc_mcontext.__gregs[13];
    emu->regs[_DX].q[0] = p->uc_mcontext.__gregs[12];
    emu->regs[_BX].q[0] = p->uc_mcontext.__gregs[24];
    emu->regs[_SP].q[0] = p->uc_mcontext.__gregs[9];
    emu->regs[_BP].q[0] = p->uc_mcontext.__gregs[8];
    emu->regs[_SI].q[0] = p->uc_mcontext.__gregs[11];
    emu->regs[_DI].q[0] = p->uc_mcontext.__gregs[10];
    emu->regs[_R8].q[0] = p->uc_mcontext.__gregs[14];
    emu->regs[_R9].q[0] = p->uc_mcontext.__gregs[15];
    emu->regs[_R10].q[0] = p->uc_mcontext.__gregs[26];
    emu->regs[_R11].q[0] = p->uc_mcontext.__gregs[27];
    emu->regs[_R12].q[0] = p->uc_mcontext.__gregs[18];
    emu->regs[_R13].q[0] = p->uc_mcontext.__gregs[19];
    emu->regs[_R14].q[0] = p->uc_mcontext.__gregs[20];
    emu->regs[_R15].q[0] = p->uc_mcontext.__gregs[21];
    emu->ip.q[0] = ip;
    emu->eflags.x64 = p->uc_mcontext.__gregs[23];
#else
#error  Unsupported architecture
#endif
#endif
}

KHASH_SET_INIT_INT64(unaligned)
static kh_unaligned_t    *unaligned = NULL;

void add_unaligned_address(uintptr_t addr)
{
    if(!unaligned)
        unaligned = kh_init(unaligned);
    khint_t k;
    int ret;
    k = kh_put(unaligned, unaligned, addr, &ret);    // just add
}

int is_addr_unaligned(uintptr_t addr)
{
    if(!unaligned)
        return 0;
    khint_t k = kh_get(unaligned, unaligned, addr);
    return (k==kh_end(unaligned))?0:1;
}

#ifdef DYNAREC
int mark_db_unaligned(dynablock_t* db, uintptr_t x64pc)
{
    add_unaligned_address(x64pc);
    db->hash++; // dirty the block
    MarkDynablock(db);      // and mark it
if(BOX64ENV(showsegv)) printf_log(LOG_INFO, "Marked db %p as dirty, and address %p as needing unaligned handling\n", db, (void*)x64pc);
    return 2;   // marked, exit handling...
}
#endif

int sigbus_specialcases(siginfo_t* info, void * ucntx, void* pc, void* _fpsimd, dynablock_t* db, uintptr_t x64pc)
{
    if((uintptr_t)pc<0x10000)
        return 0;
#ifdef DYNAREC
    if(ARCH_UNALIGNED(db, x64pc))
        /*return*/ mark_db_unaligned(db, x64pc);    // don't force an exit for now
#endif
#ifdef ARM64
    ucontext_t *p = (ucontext_t *)ucntx;
    uint32_t opcode = *(uint32_t*)pc;
    struct fpsimd_context *fpsimd = (struct fpsimd_context *)_fpsimd;
    //printf_log(LOG_INFO, "Checking SIGBUS special cases with pc=%p, opcode=%x, fpsimd=%p\n", pc, opcode, fpsimd);
    if((opcode&0b10111111110000000000000000000000)==0b10111001000000000000000000000000) {
        // this is STR
        int scale = (opcode>>30)&3;
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        uint64_t offset = (opcode>>10)&0b111111111111;
        offset<<=scale;
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest] + offset);
        uint64_t value = p->uc_mcontext.regs[val];
        if(scale==3 && (((uintptr_t)addr)&3)==0) {
            for(int i=0; i<2; ++i)
                ((volatile uint32_t*)addr)[i] = (value>>(i*32))&0xffffffff;
        } else
            for(int i=0; i<(1<<scale); ++i)
                addr[i] = (value>>(i*8))&0xff;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b10111111111000000000110000000000) == 0b10111000000000000000000000000000) {
        // this is a STUR that SIGBUS if accessing unaligned device memory
        int size = 1<<((opcode>>30)&3);
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        int64_t offset = (opcode>>12)&0b111111111;
        if((offset>>(9-1))&1)
            offset |= (0xffffffffffffffffll<<9);
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest] + offset);
        uint64_t value = p->uc_mcontext.regs[val];
        if(size==8 && (((uintptr_t)addr)&3)==0) {
            for(int i=0; i<2; ++i)
                ((volatile uint32_t*)addr)[i] = (value>>(i*32))&0xffffffff;
        } else
            for(int i=0; i<size; ++i)
                addr[i] = (value>>(i*8))&0xff;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b00111111010000000000000000000000)==0b00111101000000000000000000000000) {
        // this is VSTR
        int scale = (opcode>>30)&3;
        if((opcode>>23)&1)
            scale+=4;
        if(scale>4)
            return 0;
        if(!fpsimd)
            return 0;
        uint64_t offset = (opcode>>10)&0b111111111111;
        offset<<=scale;
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest] + offset);
        __uint128_t value = fpsimd->vregs[val];
        if(scale>2 && (((uintptr_t)addr)&3)==0) {
            for(int i=0; i<(1<<(scale-2)); ++i)
                ((volatile uint32_t*)addr)[i] = (value>>(i*32))&0xffffffff;
        } else
            for(int i=0; i<(1<<scale); ++i)
                addr[i] = (value>>(i*8))&0xff;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b00111111011000000000110000000000)==0b00111100000000000000000000000000) {
        // this is VSTRU
        int scale = (opcode>>30)&3;
        if((opcode>>23)&1)
            scale+=4;
        if(scale>4)
            return 0;
        if(!fpsimd)
            return 0;
        int64_t offset = (opcode>>12)&0b111111111;
        if((offset>>(9-1))&1)
            offset |= (0xffffffffffffffffll<<9);
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest] + offset);
        __uint128_t value = fpsimd->vregs[val];
        if(scale>2 && (((uintptr_t)addr)&3)==0) {
            for(int i=0; i<(1<<(scale-2)); ++i)
                ((volatile uint32_t*)addr)[i] = (value>>(i*32))&0xffffffff;
        } else
            for(int i=0; i<(1<<scale); ++i)
                addr[i] = (value>>(i*8))&0xff;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b00111111010000000000000000000000)==0b00111101010000000000000000000000) {
        // this is VLDR
        int scale = (opcode>>30)&3;
        if((opcode>>23)&1)
            scale+=4;
        if(scale>4)
            return 0;
        if(!fpsimd)
            return 0;
        uint64_t offset = (opcode>>10)&0b111111111111;
        offset<<=scale;
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest] + offset);
        __uint128_t value = 0;
        if(scale>2 && (((uintptr_t)addr)&3)==0) {
            for(int i=0; i<(1<<(scale-2)); ++i)
                value |= ((__uint128_t)(((volatile uint32_t*)addr)[i]))<<(i*32);
        } else
            for(int i=0; i<(1<<scale); ++i)
                value |= ((__uint128_t)addr[i])<<(i*8);
        fpsimd->vregs[val] = value;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b00111111011000000000110000000000)==0b00111100010000000000000000000000) {
        // this is VLDRU
        int scale = (opcode>>30)&3;
        if((opcode>>23)&1)
            scale+=4;
        if(scale>4)
            return 0;
        if(!fpsimd)
            return 0;
        int64_t offset = (opcode>>12)&0b111111111;
        if((offset>>(9-1))&1)
            offset |= (0xffffffffffffffffll<<9);
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest] + offset);
        __uint128_t value = 0;
        if(scale>2 && (((uintptr_t)addr)&3)==0) {
            for(int i=0; i<(1<<(scale-2)); ++i)
                value |= ((__uint128_t)(((volatile  uint32_t*)addr)[i]))<<(i*32);
        } else
            for(int i=0; i<(1<<scale); ++i)
                value |= ((__uint128_t)addr[i])<<(i*8);
        fpsimd->vregs[val] = value;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b10111111110000000000000000000000)==0b10111001010000000000000000000000) {
        // this is LDR
        int scale = (opcode>>30)&3;
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        uint64_t offset = (opcode>>10)&0b111111111111;
        offset<<=scale;
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest] + offset);
        uint64_t value = 0;
        if(scale==3 && (((uintptr_t)addr)&3)==0) {
            for(int i=0; i<2; ++i)
                value |= ((uint64_t)((volatile  uint32_t*)addr)[i]) << (i*32);
        } else
            for(int i=0; i<(1<<scale); ++i)
                value |= ((uint64_t)addr[i]) << (i*8);
        p->uc_mcontext.regs[val] = value;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b10111111111000000000110000000000) == 0b10111000010000000000000000000000) {
        // this is a LDUR
        int size = 1<<((opcode>>30)&3);
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        int64_t offset = (opcode>>12)&0b111111111;
        if((offset>>(9-1))&1)
            offset |= (0xffffffffffffffffll<<9);
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest] + offset);
        uint64_t value = 0;
        if(size==8 && (((uintptr_t)addr)&3)==0) {
            for(int i=0; i<2; ++i)
                value |= ((uint64_t)((volatile  uint32_t*)addr)[i]) << (i*32);
        } else
            for(int i=0; i<size; ++i)
                value |= ((uint64_t)addr[i]) << (i*8);
        p->uc_mcontext.regs[val] = value;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b11111111110000000000000000000000)==0b01111001000000000000000000000000) {
        // this is STRH
        int scale = (opcode>>30)&3;
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        uint64_t offset = (opcode>>10)&0b111111111111;
        offset<<=scale;
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest] + offset);
        uint64_t value = p->uc_mcontext.regs[val];
        for(int i=0; i<(1<<scale); ++i)
            addr[i] = (value>>(i*8))&0xff;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b11111111111000000000110000000000)==0b01111000000000000000000000000000) {
        // this is STURH
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        int64_t offset = (opcode>>12)&0b111111111;
        if((offset>>(9-1))&1)
            offset |= (0xffffffffffffffffll<<9);
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest] + offset);
        uint64_t value = p->uc_mcontext.regs[val];
        for(int i=0; i<2; ++i)
            addr[i] = (value>>(i*8))&0xff;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b11111111111000000000110000000000)==0b01111000001000000000100000000000) {
        // this is STRH reg, reg
        int scale = (opcode>>30)&3;
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        int dest2 = (opcode>>16)&31;
        int option = (opcode>>13)&0b111;
        int S = (opcode>>12)&1;
        if(option!=0b011)
            return 0;   // only LSL is supported
        uint64_t offset = p->uc_mcontext.regs[dest2]<<S;
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest] + offset);
        uint64_t value = p->uc_mcontext.regs[val];
        for(int i=0; i<(1<<scale); ++i)
            addr[i] = (value>>(i*8))&0xff;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b11111111110000000000000000000000)==0b10101001000000000000000000000000) {
        // This is STP reg1, reg2, [reg3 + off]
        int scale = 2+((opcode>>31)&1);
        int val1 = opcode&31;
        int val2 = (opcode>>10)&31;
        int dest = (opcode>>5)&31;
        int64_t offset = (opcode>>15)&0b1111111;
        if((offset>>(7-1))&1)
            offset |= (0xffffffffffffffffll<<7);
        offset <<= scale;
        uintptr_t addr= p->uc_mcontext.regs[dest] + offset;
        if((((uintptr_t)addr)&3)==0) {
            ((volatile uint32_t*)addr)[0] = p->uc_mcontext.regs[val1];
            ((volatile uint32_t*)addr)[1] = p->uc_mcontext.regs[val2];
        } else {
            __uint128_t value = ((__uint128_t)p->uc_mcontext.regs[val2])<<64 | p->uc_mcontext.regs[val1];
            for(int i=0; i<(1<<scale); ++i)
                ((volatile uint8_t*)addr)[i] = (value>>(i*8))&0xff;
        }
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b11111111110000000000000000000000)==0b10101101000000000000000000000000) {
        // This is (V)STP qreg1, qreg2, [reg3 + off]
        int scale = 2+((opcode>>30)&3);
        int val1 = opcode&31;
        int val2 = (opcode>>10)&31;
        int dest = (opcode>>5)&31;
        int64_t offset = (opcode>>15)&0b1111111;
        if((offset>>(7-1))&1)
            offset |= (0xffffffffffffffffll<<7);
        offset <<= scale;
        uintptr_t addr= p->uc_mcontext.regs[dest] + offset;
        if((((uintptr_t)addr)&3)==0) {
            for(int i=0; i<4; ++i)
                ((volatile uint32_t*)addr)[0+i] = (fpsimd->vregs[val1]>>(i*32))&0xffffffff;
            for(int i=0; i<4; ++i)
                ((volatile uint32_t*)addr)[4+i] = (fpsimd->vregs[val2]>>(i*32))&0xffffffff;
        } else {
            for(int i=0; i<16; ++i)
                ((volatile uint8_t*)addr)[i] = (fpsimd->vregs[val1]>>(i*8))&0xff;
            for(int i=0; i<16; ++i)
                ((volatile uint8_t*)addr)[16+i] = (fpsimd->vregs[val2]>>(i*8))&0xff;
        }
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b10111111111111111111110000000000)==0b00001101000000001000010000000000) {
        // this is ST1.D
        int idx = (opcode>>30)&1;
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest]);
        uint64_t value = fpsimd->vregs[val]>>(idx*64);
        if((((uintptr_t)addr)&3)==0) {
            for(int i=0; i<2; ++i)
                ((volatile uint32_t*)addr)[i] = (value>>(i*32))&0xffffffff;
        } else
            for(int i=0; i<8; ++i)
                addr[i] = (value>>(i*8))&0xff;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
#elif RV64
#define GET_FIELD(v, high, low) (((v) >> low) & ((1ULL << (high - low + 1)) - 1))
#define SIGN_EXT(val, val_sz) (((int32_t)(val) << (32 - (val_sz))) >> (32 - (val_sz)))

    ucontext_t *p = (ucontext_t *)ucntx;
    uint32_t inst = *(uint32_t*)pc;

    uint32_t funct3 = GET_FIELD(inst, 14, 12);
    uint32_t opcode = GET_FIELD(inst, 6, 0);
    if ((opcode == 0b0100011 || opcode == 0b0100111 /* F */) && (funct3 == 0b010 /* (F)SW */ || funct3 == 0b011 /* (F)SD */ || funct3 == 0b001 /* SH */)) {
        int val = (inst >> 20) & 0x1f;
        int dest = (inst >> 15) & 0x1f;
        int64_t imm = (GET_FIELD(inst, 31, 25) << 5) | (GET_FIELD(inst, 11, 7));
        imm = SIGN_EXT(imm, 12);
        volatile uint8_t *addr = (void *)(p->uc_mcontext.__gregs[dest] + imm);
        uint64_t value = opcode == 0b0100011 ? p->uc_mcontext.__gregs[val] : p->uc_mcontext.__fpregs.__d.__f[val<<1];
        for(int i = 0; i < (funct3 == 0b010 ? 4 : funct3 == 0b011 ? 8 : 2); ++i) {
            addr[i] = (value >> (i * 8)) & 0xff;
        }
        p->uc_mcontext.__gregs[0] += 4; // pc += 4
        return 1;
    } else {
        printf_log(LOG_NONE, "Unsupported SIGBUS special cases with pc=%p, opcode=%x\n", pc, inst);
    }

#undef GET_FIELD
#undef SIGN_EXT
#endif
    return 0;
#undef CHECK
}

#ifdef BOX32
void my_sigactionhandler_oldcode_32(x64emu_t* emu, int32_t sig, int simple, siginfo_t* info, void * ucntx, int* old_code, void* cur_db);
#endif
void my_sigactionhandler_oldcode_64(x64emu_t* emu, int32_t sig, int simple, siginfo_t* info, void * ucntx, int* old_code, void* cur_db)
{
    int Locks = unlockMutex();
    int log_minimum = (BOX64ENV(showsegv))?LOG_NONE:LOG_DEBUG;

    printf_log(LOG_DEBUG, "Sigactionhanlder for signal #%d called (jump to %p/%s)\n", sig, (void*)my_context->signals[sig], GetNativeName((void*)my_context->signals[sig]));

    uintptr_t restorer = my_context->restorer[sig];
    // get that actual ESP first!
    if(!emu)
        emu = thread_get_emu();
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
            frame = (uintptr_t)p->uc_mcontext.__gregs[9];
    }
#else
#error Unsupported architecture
#endif
#else
    (void)ucntx; (void)cur_db;
    void* pc = NULL;
#endif
    // setup libc context stack frame, on caller stack
    frame = frame&~15;

    // stack tracking
    x64_stack_t *new_ss = my_context->onstack[sig]?(x64_stack_t*)pthread_getspecific(sigstack_key):NULL;
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
    frame -= sizeof(x64_ucontext_t);
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
    sigcontext->uc_mcontext.gregs[X64_RSP] = R_RSP;
    sigcontext->uc_mcontext.gregs[X64_RBX] = R_RBX;
    sigcontext->uc_mcontext.gregs[X64_RIP] = R_RIP;
    // flags
    sigcontext->uc_mcontext.gregs[X64_EFL] = emu->eflags.x64;
    // get segments
    sigcontext->uc_mcontext.gregs[X64_CSGSFS] = ((uint64_t)(R_CS)) | (((uint64_t)(R_GS))<<16) | (((uint64_t)(R_FS))<<32);
    if(R_CS==0x23) {
        // trucate regs to 32bits, just in case
        #define GO(R)   sigcontext->uc_mcontext.gregs[X64_R##R]&=0xFFFFFFFF
        GO(AX);
        GO(CX);
        GO(DX);
        GO(DI);
        GO(SI);
        GO(BP);
        GO(SP);
        GO(BX);
        GO(IP);
        #undef GO
    }
    // get FloatPoint status
    sigcontext->uc_mcontext.fpregs = xstate;//(struct x64_libc_fpstate*)&sigcontext->xstate;
    fpu_xsave_mask(emu, xstate, 0, 0b111);
    memcpy(&sigcontext->xstate, xstate, sizeof(sigcontext->xstate));
    ((struct x64_fpstate*)xstate)->res[12] = 0x46505853;   // magic number to signal an XSTATE type of fpregs
    ((struct x64_fpstate*)xstate)->res[13] = 0; // offset to xstate after this?
    // get signal mask

    if(new_ss) {
        sigcontext->uc_stack.ss_sp = new_ss->ss_sp;
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
    if(prot&PROT_READ) real_prot|=PROT_READ;
    if(prot&PROT_WRITE) real_prot|=PROT_WRITE;
    if(prot&PROT_EXEC) real_prot|=PROT_WRITE;
    if(prot&PROT_DYNAREC) real_prot|=PROT_WRITE;
    sigcontext->uc_mcontext.gregs[X64_ERR] = 0;
    sigcontext->uc_mcontext.gregs[X64_TRAPNO] = 0;
    if(sig==SIGBUS)
        sigcontext->uc_mcontext.gregs[X64_TRAPNO] = 17;
    else if(sig==SIGSEGV) {
        if((uintptr_t)info->si_addr == sigcontext->uc_mcontext.gregs[X64_RIP]) {
            if(info->si_errno==0xbad0) {
                //bad opcode
                sigcontext->uc_mcontext.gregs[X64_ERR] = 0;
                sigcontext->uc_mcontext.gregs[X64_TRAPNO] = 13;
                info2->si_code = 128;
                info2->si_errno = 0;
                info2->si_addr = NULL;
            } else if (info->si_errno==0xecec) {
                // no excute bit on segment
                sigcontext->uc_mcontext.gregs[X64_ERR] = 0x14|((sysmapped && !(real_prot&PROT_READ))?0:1);
                sigcontext->uc_mcontext.gregs[X64_TRAPNO] = 14;
                if(!mmapped) info2->si_code = 1;
                info2->si_errno = 0;
            }else {
                sigcontext->uc_mcontext.gregs[X64_ERR] = 0x14|((sysmapped && !(real_prot&PROT_READ))?0:1);
                sigcontext->uc_mcontext.gregs[X64_TRAPNO] = 14;
            }
        } else {
            sigcontext->uc_mcontext.gregs[X64_TRAPNO] = 14;
            sigcontext->uc_mcontext.gregs[X64_ERR] = 4|((sysmapped && !(real_prot&PROT_READ))?0:1);
            if(write_opcode(sigcontext->uc_mcontext.gregs[X64_RIP], (uintptr_t)pc, (R_CS==0x23)))
                sigcontext->uc_mcontext.gregs[X64_ERR] |= 2;
        }
        if(info->si_code == SEGV_ACCERR && old_code)
            *old_code = -1;
        if(info->si_errno==0x1234) {
            sigcontext->uc_mcontext.gregs[X64_TRAPNO] = 13;
            info2->si_errno = 0;
        } else if(info->si_errno==0xdead) {
            // INT x
            uint8_t int_n = info->si_code;
            info2->si_errno = 0;
            info2->si_code = 128;
            info2->si_addr = NULL;
            sigcontext->uc_mcontext.gregs[X64_TRAPNO] = 13;
            // some special cases...
            if(int_n==3) {
                info2->si_signo = SIGTRAP;
                sigcontext->uc_mcontext.gregs[X64_TRAPNO] = 3;
                sigcontext->uc_mcontext.gregs[X64_ERR] = 0;
            } else if(int_n==0x04) {
                sigcontext->uc_mcontext.gregs[X64_TRAPNO] = 4;
                sigcontext->uc_mcontext.gregs[X64_ERR] = 0;
            } else if (int_n==0x29 || int_n==0x2c || int_n==0x2d) {
                sigcontext->uc_mcontext.gregs[X64_ERR] = 0x02|(int_n<<3);
            } else {
                sigcontext->uc_mcontext.gregs[X64_ERR] = 0x0a|(int_n<<3);
                sigcontext->uc_mcontext.gregs[X64_TRAPNO] = 13;
            }
        } else if(info->si_errno==0xcafe) { // divide by 0
            info2->si_errno = 0;
            sigcontext->uc_mcontext.gregs[X64_ERR] = 0;
            sigcontext->uc_mcontext.gregs[X64_TRAPNO] = 0;
            info2->si_signo = SIGFPE;
        }
    } else if(sig==SIGFPE) {
        if (info->si_code == FPE_INTOVF)
            sigcontext->uc_mcontext.gregs[X64_TRAPNO] = 4;
        else
            sigcontext->uc_mcontext.gregs[X64_TRAPNO] = 19;
    } else if(sig==SIGILL) {
        info2->si_code = 2;
        sigcontext->uc_mcontext.gregs[X64_TRAPNO] = 6;
    } else if(sig==SIGTRAP) {
        if(info->si_code==1) {  //single step
            info2->si_code = 2;
            info2->si_addr = (void*)sigcontext->uc_mcontext.gregs[X64_RIP];
        } else
            info2->si_code = 128;
        sigcontext->uc_mcontext.gregs[X64_TRAPNO] = info->si_code;
        sigcontext->uc_mcontext.gregs[X64_ERR] = 0;
    }
    //TODO: SIGABRT generate what?
    printf_log((sig==10)?LOG_DEBUG:log_minimum, "Signal %d: si_addr=%p, TRAPNO=%d, ERR=%d, RIP=%p, prot=%x, mmapped:%d\n", sig, (void*)info2->si_addr, sigcontext->uc_mcontext.gregs[X64_TRAPNO], sigcontext->uc_mcontext.gregs[X64_ERR],sigcontext->uc_mcontext.gregs[X64_RIP], prot, mmapped);
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
    R_RSP = frame;
    // set frame pointer
    R_RBP = sigcontext->uc_mcontext.gregs[X64_RBP];

    int exits = 0;
    int ret;
    int dynarec = 0;
    #ifdef DYNAREC
    if(sig!=SIGSEGV && !(Locks&is_dyndump_locked) && !(Locks&is_memprot_locked))
        dynarec = 1;
    #endif
    ret = RunFunctionHandler(emu, &exits, dynarec, sigcontext, my_context->signals[info2->si_signo], 3, info2->si_signo, info2, sigcontext);
    // restore old value from emu
    if(used_stack)  // release stack
        new_ss->ss_flags = 0;
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
        if(emu->jmpbuf) {
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
            // flags
            emu->eflags.x64=sigcontext->uc_mcontext.gregs[X64_EFL];
            // get segments
            uint16_t seg;
            seg = (sigcontext->uc_mcontext.gregs[X64_CSGSFS] >> 0)&0xffff;
            #define GO(S) if(emu->segs[_##S]!=seg)  emu->segs[_##S]=seg
            GO(CS);
            seg = (sigcontext->uc_mcontext.gregs[X64_CSGSFS] >> 16)&0xffff;
            GO(GS);
            seg = (sigcontext->uc_mcontext.gregs[X64_CSGSFS] >> 32)&0xffff;
            GO(FS);
            #undef GO
            for(int i=0; i<6; ++i)
                emu->segs_serial[i] = 0;
            printf_log((sig==10)?LOG_DEBUG:log_minimum, "Context has been changed in Sigactionhanlder, doing siglongjmp to resume emu at %p, RSP=%p\n", (void*)R_RIP, (void*)R_RSP);
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
        printf_log(LOG_INFO, "Warning, context has been changed in Sigactionhanlder%s\n", (sigcontext->uc_mcontext.gregs[X64_RIP]!=sigcontext_copy.uc_mcontext.gregs[X64_RIP])?" (EIP changed)":"");
    }
    // restore regs...
    #define GO(R)   R_##R=sigcontext->uc_mcontext.gregs[X64_##R]
    GO(RAX);
    GO(RCX);
    GO(RDX);
    GO(RDI);
    GO(RSI);
    GO(RBP);
    GO(RSP);
    GO(RBX);
    GO(R8);
    GO(R9);
    GO(R10);
    GO(R11);
    GO(R12);
    GO(R13);
    GO(R14);
    GO(R15);
    GO(RIP);
    #undef GO
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
        //relockMutex(Locks);   // the thread will exit, so no relock there
        #ifdef DYNAREC
        if(Locks & is_dyndump_locked)
            CancelBlock64(1);
        #endif
        exit(ret);
    }
    if(restorer)
        RunFunctionHandler(emu, &exits, 0, NULL, restorer, 0);
    relockMutex(Locks);
}

void my_sigactionhandler_oldcode(x64emu_t* emu, int32_t sig, int simple, siginfo_t* info, void * ucntx, int* old_code, void* cur_db, uintptr_t x64pc)
{
    #define GO(A) uintptr_t old_##A = R_##A;
    GO(RAX);
    GO(RBX);
    GO(RCX);
    GO(RDX);
    GO(RBP);
    GO(RSP);
    GO(RDI);
    GO(RSI);
    GO(R8);
    GO(R9);
    GO(R10);
    GO(R11);
    GO(R12);
    GO(R13);
    GO(R14);
    GO(R15);
    GO(RIP);
    #undef GO
    sse_regs_t old_xmm[16];
    sse_regs_t old_ymm[16];
    mmx87_regs_t old_mmx[8];
    mmx87_regs_t old_x87[8];
    uint32_t old_top = emu->top;
    memcpy(old_xmm, emu->xmm, sizeof(old_xmm));
    memcpy(old_ymm, emu->ymm, sizeof(old_ymm));
    memcpy(old_mmx, emu->mmx, sizeof(old_mmx));
    memcpy(old_x87, emu->x87, sizeof(old_x87));
    #ifdef DYNAREC
    dynablock_t* db = cur_db;
    if(db) {
        copyUCTXreg2Emu(emu, ucntx, x64pc);
        adjustregs(emu);
        if(db && db->arch_size)
            ARCH_ADJUST(db, emu, ucntx, x64pc);
    }
    #endif
    #ifdef BOX32
    if(box64_is32bits) {
        my_sigactionhandler_oldcode_32(emu, sig, simple, info, ucntx, old_code, cur_db);
    } else
    #endif
    my_sigactionhandler_oldcode_64(emu, sig, simple, info, ucntx, old_code, cur_db);
    #define GO(A) R_##A = old_##A
    GO(RAX);
    GO(RBX);
    GO(RCX);
    GO(RDX);
    GO(RBP);
    GO(RSP);
    GO(RDI);
    GO(RSI);
    GO(R8);
    GO(R9);
    GO(R10);
    GO(R11);
    GO(R12);
    GO(R13);
    GO(R14);
    GO(R15);
    GO(RIP);
    #undef GO
    memcpy(emu->xmm, old_xmm, sizeof(old_xmm));
    memcpy(emu->ymm, old_ymm, sizeof(old_ymm));
    memcpy(emu->mmx, old_mmx, sizeof(old_mmx));
    memcpy(emu->x87, old_x87, sizeof(old_x87));
    emu->top = old_top;
}

extern void* current_helper;
#define USE_SIGNAL_MUTEX
#ifdef USE_SIGNAL_MUTEX
#ifdef USE_CUSTOM_MUTEX
static uint32_t mutex_dynarec_prot = 0;
#else
static pthread_mutex_t mutex_dynarec_prot = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
#endif
#define lock_signal()     mutex_lock(&mutex_dynarec_prot)
#define unlock_signal()   mutex_unlock(&mutex_dynarec_prot)
#else   // USE_SIGNAL_MUTEX
#define lock_signal()
#define unlock_signal()
#endif

extern int box64_quit;
extern int box64_exit_code;

void my_box64signalhandler(int32_t sig, siginfo_t* info, void * ucntx)
{
    // sig==SIGSEGV || sig==SIGBUS || sig==SIGILL || sig==SIGABRT here!
    int log_minimum = (BOX64ENV(showsegv))?LOG_NONE:((sig==SIGSEGV && my_context->is_sigaction[sig])?LOG_DEBUG:LOG_INFO);
    if(signal_jmpbuf_active) {
        signal_jmpbuf_active = 0;
        longjmp(SIG_JMPBUF, 1);
    }
    ucontext_t *p = (ucontext_t *)ucntx;
    void* addr = (void*)info->si_addr;  // address that triggered the issue
    void* rsp = NULL;
    x64emu_t* emu = thread_get_emu();
    int tid = GetTID();
#ifdef __aarch64__
    void * pc = (void*)p->uc_mcontext.pc;
    struct fpsimd_context *fpsimd = NULL;
    // find fpsimd struct
    {
        struct _aarch64_ctx * ff = (struct _aarch64_ctx*)p->uc_mcontext.__reserved;
        while (ff->magic && !fpsimd) {
            if(ff->magic==FPSIMD_MAGIC)
                fpsimd = (struct fpsimd_context*)ff;
            else
                ff = (struct _aarch64_ctx*)((uintptr_t)ff + ff->size);
        }
    }
#elif defined __x86_64__
    void * pc = (void*)p->uc_mcontext.gregs[X64_RIP];
    void* fpsimd = NULL;
#elif defined __powerpc64__
    void * pc = (void*)p->uc_mcontext.gp_regs[PT_NIP];
    void* fpsimd = NULL;
#elif defined(LA64)
    void * pc = (void*)p->uc_mcontext.__pc;
    void* fpsimd = NULL;
#elif defined(SW64)
    void * pc = (void*)p->uc_mcontext.sc_pc;
    void* fpsimd = NULL;
#elif defined(RV64)
    void * pc = (void*)p->uc_mcontext.__gregs[REG_PC];
    void* fpsimd = NULL;
#else
    void * pc = NULL;    // unknow arch...
    void* fpsimd = NULL;
    #warning Unhandled architecture
#endif
    dynablock_t* db = NULL;
    int db_searched = 0;
    uintptr_t x64pc = (uintptr_t)-1;
    x64pc = R_RIP;
    if((sig==SIGBUS) && (addr!=pc)) {
        db = FindDynablockFromNativeAddress(pc);
        if(db)
            x64pc = getX64Address(db, (uintptr_t)pc);
        db_searched = 1;
        int fixed = 0;
        if((fixed=sigbus_specialcases(info, ucntx, pc, fpsimd, db, x64pc))) {
            // special case fixed, restore everything and just continues
            if (BOX64ENV(log)>=LOG_DEBUG || BOX64ENV(showsegv)) {
                static void*  old_pc[2] = {0};
                static int old_pc_i = 0;
                if(old_pc[0]!=pc && old_pc[1]!=pc) {
                    old_pc[old_pc_i++] = pc;
                    if(old_pc_i==2)
                        old_pc_i = 0;
                    uint8_t* x64 = (uint8_t*)x64pc;
                    if(db)
                        printf_log(LOG_INFO, "Special unaligned case fixed @%p, opcode=%08x (addr=%p, db=%p, x64pc=%p[%02hhX %02hhX %02hhX %02hhX %02hhX])\n", pc, *(uint32_t*)pc, addr, db, x64pc, x64[0], x64[1], x64[2], x64[3], x64[4], x64[5]);
                    else
                        printf_log(LOG_INFO, "Special unaligned case fixed @%p, opcode=%08x (addr=%p)\n", pc, *(uint32_t*)pc, addr);
                }
            }
            return;
        }
    }
    int Locks = unlockMutex();
    uint32_t prot = getProtection((uintptr_t)addr);
    #ifdef BAD_SIGNAL
    // try to see if the si_code makes sense
    // the RK3588 tend to need a special Kernel that seems to have a weird behaviour sometimes
    if((sig==SIGSEGV) && (addr) && (info->si_code == 1) && getMmapped((uintptr_t)addr)) {
        printf_log(LOG_DEBUG, "Workaround for suspicious si_code for %p / prot=0x%hhx\n", addr, prot);
        info->si_code = 2;
    }
    #endif
#ifdef RV64
    if((sig==SIGSEGV) && (addr==pc) && (info->si_code==2) && (prot==(PROT_READ|PROT_WRITE|PROT_EXEC))) {
        if(!db_searched) {
            db = FindDynablockFromNativeAddress(pc);
            if(db)
                x64pc = getX64Address(db, (uintptr_t)pc);
            db_searched = 1;
        }
        int fixed = 0;
        if((fixed = sigbus_specialcases(info, ucntx, pc, fpsimd, db, x64pc))) {
            // special case fixed, restore everything and just continues
            if (BOX64ENV(log) >= LOG_DEBUG || BOX64ENV(showsegv)) {
                static void*  old_pc[2] = {0};
                static int old_pc_i = 0;
                if(old_pc[0]!=pc && old_pc[1]!=pc) {
                    old_pc[old_pc_i++] = pc;
                    if(old_pc_i==2)
                        old_pc_i = 0;
                    printf_log(LOG_NONE, "Special unalinged cased fixed @%p, opcode=%08x (addr=%p)\n", pc, *(uint32_t *)pc, addr);
                }
            }
            relockMutex(Locks);
            return;
        }
    }
#endif
#ifdef DYNAREC
    if((Locks & is_dyndump_locked) && ((sig==SIGSEGV) || (sig==SIGBUS)) && current_helper) {
        printf_log(LOG_INFO, "FillBlock triggered a %s at %p from %p\n", (sig==SIGSEGV)?"segfault":"bus error", addr, pc);
        CancelBlock64(0);
        relockMutex(Locks);
        cancelFillBlock();  // Segfault inside a Fillblock, cancel it's creation...
        // cancelFillBlock does not return
    }
    if ((sig==SIGSEGV) && (addr) && (info->si_code == SEGV_ACCERR) && (prot&PROT_DYNAREC)) {
        lock_signal();
        // check if SMC inside block
        if(!db_searched) {
            db = FindDynablockFromNativeAddress(pc);
            if(db)
                x64pc = getX64Address(db, (uintptr_t)pc);
            db_searched = 1;
        }
        // access error, unprotect the block (and mark them dirty)
        unprotectDB((uintptr_t)addr, 1, 1);    // unprotect 1 byte... But then, the whole page will be unprotected
        if(db) CheckHotPage((uintptr_t)addr);
        int db_need_test = db?getNeedTest((uintptr_t)db->x64_addr):0;
        if(db && ((addr>=db->x64_addr && addr<(db->x64_addr+db->x64_size)) || (db_need_test && !BOX64ENV(dynarec_dirty)))) {
            emu = getEmuSignal(emu, p, db);
            // dynablock got auto-dirty! need to get out of it!!!
            if(emu->jmpbuf) {
                uintptr_t x64pc = getX64Address(db, (uintptr_t)pc);
                copyUCTXreg2Emu(emu, p, x64pc);
                adjustregs(emu);
                if(db && db->arch_size)
                    ARCH_ADJUST(db, emu, p, x64pc);
                dynarec_log(LOG_INFO, "Dynablock (%p, x64addr=%p, need_test=%d/%d/%d) %s, getting out at %p (%p)!\n", db, db->x64_addr, db_need_test, db->dirty, db->always_test, (addr>=db->x64_addr && addr<(db->x64_addr+db->x64_size))?"Auto-SMC":"unprotected", (void*)R_RIP, (void*)addr);
                //relockMutex(Locks);
                unlock_signal();
                if(Locks & is_dyndump_locked)
                    CancelBlock64(1);
                emu->test.clean = 0;
                #ifdef ANDROID
                siglongjmp(*(JUMPBUFF*)emu->jmpbuf, 2);
                #else
                siglongjmp(emu->jmpbuf, 2);
                #endif
            }
            dynarec_log(LOG_INFO, "Warning, Auto-SMC (%p for db %p/%p) detected, but jmpbuffer not ready!\n", (void*)addr, db, (void*)db->x64_addr);
        }
        // done
        if((prot&PROT_WRITE)/*|| (prot&PROT_DYNAREC)*/) {
            unlock_signal();
            // if there is no write permission, don't return and continue to program signal handling
            relockMutex(Locks);
            return;
        }
        unlock_signal();
    } else if ((sig==SIGSEGV) && (addr) && (info->si_code == SEGV_ACCERR) && ((prot&(PROT_READ|PROT_WRITE))==(PROT_READ|PROT_WRITE))) {
        lock_signal();
        if(!db_searched) {
            db = FindDynablockFromNativeAddress(pc);
            if(db)
                x64pc = getX64Address(db, (uintptr_t)pc);
            db_searched = 1;
        }
        if(db && db->x64_addr>= addr && (db->x64_addr+db->x64_size)<addr) {
            dynarec_log(LOG_INFO, "Warning, addr inside current dynablock!\n");
        }
        // mark stuff as unclean
        if(BOX64ENV(dynarec))
            cleanDBFromAddressRange(((uintptr_t)addr)&~(box64_pagesize-1), box64_pagesize, 0);
        static void* glitch_pc = NULL;
        static void* glitch_addr = NULL;
        static uint32_t glitch_prot = 0;
        if(addr && pc /*&& db*/) {
            if((glitch_pc!=pc || glitch_addr!=addr || glitch_prot!=prot)) {
                // probably a glitch due to intensive multitask...
                dynarec_log(/*LOG_DEBUG*/LOG_INFO, "%04d|SIGSEGV with Access error on %p for %p, db=%p, prot=0x%x, retrying\n", tid, pc, addr, db, prot);
                glitch_pc = pc;
                glitch_addr = addr;
                glitch_prot = prot;
                relockMutex(Locks);
                unlock_signal();
                return; // try again
            }
dynarec_log(/*LOG_DEBUG*/LOG_INFO, "%04d|Repeated SIGSEGV with Access error on %p for %p, db=%p, prot=0x%x\n", tid, pc, addr, db, prot);
            glitch_pc = NULL;
            glitch_addr = NULL;
            glitch_prot = 0;
        }
        if(addr && pc && ((prot&(PROT_READ|PROT_WRITE))==(PROT_READ|PROT_WRITE))) {
            static void* glitch2_pc = NULL;
            static void* glitch2_addr = NULL;
            static int glitch2_prot = 0;
            if((glitch2_pc!=pc || glitch2_addr!=addr || glitch2_prot!=prot)) {
                dynarec_log(LOG_INFO, "Is that a multi process glitch too?\n");
                //printf_log(LOG_INFO, "Is that a multi process glitch too?\n");
                glitch2_pc = pc;
                glitch2_addr = addr;
                glitch2_prot = prot;
                sched_yield();  // give time to the other process
                refreshProtection((uintptr_t)addr);
                relockMutex(Locks);
                sched_yield();  // give time to the other process
                unlock_signal();
                return; // try again
            }
            glitch2_pc = NULL;
            glitch2_addr = NULL;
            glitch2_prot = 0;
        }
        unlock_signal();
    } else if ((sig==SIGSEGV) && (addr) && (info->si_code == SEGV_ACCERR) && (prot&PROT_DYNAREC_R)) {
        // unprotect and continue to signal handler, because Write is not there on purpose
        unprotectDB((uintptr_t)addr, 1, 1);    // unprotect 1 byte... But then, the whole page will be unprotected
    }
    if(!db_searched) {
        db = FindDynablockFromNativeAddress(pc);
        if(db)
            x64pc = getX64Address(db, (uintptr_t)pc);
        db_searched = 1;
    }
#endif
    if((sig==SIGSEGV || sig==SIGBUS) && box64_quit) {
        printf_log(LOG_INFO, "Sigfault/Segbus while quitting, exiting silently\n");
        _exit(box64_exit_code);    // Hack, segfault while quiting, exit silently
    }
    static int old_code = -1;
    static void* old_pc = 0;
    static void* old_addr = 0;
    static int old_tid = 0;
    static uint32_t old_prot = 0;
    int mapped = memExist((uintptr_t)addr);
    const char* signame = (sig==SIGSEGV)?"SIGSEGV":((sig==SIGBUS)?"SIGBUS":((sig==SIGILL)?"SIGILL":"SIGABRT"));
    rsp = (void*)R_RSP;
#if defined(DYNAREC)
#if defined(ARM64)
    if(db) {
        rsp = (void*)p->uc_mcontext.regs[10+_SP];
    }
#elif defined(LA64)
    if(db && p->uc_mcontext.__gregs[4]>0x10000) {
        emu = (x64emu_t*)p->uc_mcontext.__gregs[4];
    }
    if(db) {
        rsp = (void*)p->uc_mcontext.__gregs[12+_SP];
    }
#elif defined(RV64)
    if(db && p->uc_mcontext.__gregs[25]>0x10000) {
        emu = (x64emu_t*)p->uc_mcontext.__gregs[25];
    }
    if(db) {
        rsp = (void*)p->uc_mcontext.__gregs[9];
    }
#else
#error Unsupported Architecture
#endif //arch
#endif //DYNAREC
    if(!db && (sig==SIGSEGV) && ((uintptr_t)addr==(x64pc-1)))
        x64pc--;
    if(old_code==info->si_code && old_pc==pc && old_addr==addr && old_tid==tid && old_prot==prot) {
        printf_log(log_minimum, "%04d|Double %s (code=%d, pc=%p, x64pc=%p, addr=%p, prot=%02x)!\n", tid, signame, old_code, old_pc, x64pc, old_addr, prot);
        exit(-1);
    } else {
        if((sig==SIGSEGV) && (info->si_code == SEGV_ACCERR) && ((prot&~PROT_CUSTOM)==(PROT_READ|PROT_WRITE) || (prot&~PROT_CUSTOM)==(PROT_READ|PROT_WRITE|PROT_EXEC))) {
            static uintptr_t old_addr = 0;
            #ifdef DYNAREC
            if((prot==(PROT_READ|PROT_WRITE|PROT_EXEC)) && isDBFromAddressRange(((uintptr_t)addr)&~(box64_pagesize-1), box64_pagesize)) {
                printf_log(/*LOG_DEBUG*/LOG_INFO, "%04d| Strange SIGSEGV with Access error on %p for %p with DynaBlock(s) in range, db=%p, Lock=0x%x)\n", tid, pc, addr, db, Locks);
                cleanDBFromAddressRange(((uintptr_t)addr)&~(box64_pagesize-1), box64_pagesize, 0);
                refreshProtection((uintptr_t)addr);
                relockMutex(Locks);
                return;
            }
            #endif
            printf_log(/*LOG_DEBUG*/LOG_INFO, "%04d| Strange SIGSEGV with Access error on %p for %p%s, db=%p, prot=0x%x (old_addr=%p, Lock=0x%x)\n", tid, pc, addr, mapped?" mapped":"", db, prot, (void*)old_addr, Locks);
            if(!(old_addr==(uintptr_t)addr && old_prot==prot) || mapped) {
                old_addr = (uintptr_t)addr;
                old_prot = prot;
                refreshProtection(old_addr);
                relockMutex(Locks);
                sched_yield();  // give time to the other process
                return; // that's probably just a multi-task glitch, like seen in terraria
            }
            old_addr = 0;
        }
        old_code = info->si_code;
        old_pc = pc;
        old_addr = addr;
        old_tid = tid;
        old_prot = prot;
        const char* name = NULL;
        const char* x64name = NULL;
        if (log_minimum<=BOX64ENV(log)) {
            signal_jmpbuf_active = 1;
            if(sigsetjmp(SIG_JMPBUF, 1)) {
                // segfault while gathering function name...
                name = "???";
            } else
                name = GetNativeName(pc);
            signal_jmpbuf_active = 0;
        }
        // Adjust RIP for special case of NULL function run
        if(sig==SIGSEGV && R_RIP==0x1 && (uintptr_t)info->si_addr==0x0)
            R_RIP = 0x0;
        if(log_minimum<=BOX64ENV(log)) {
            elfheader_t* elf = FindElfAddress(my_context, x64pc);
            if(elf) {
                signal_jmpbuf_active = 1;
                if(sigsetjmp(SIG_JMPBUF, 1)) {
                    // segfault while gathering function name...
                    x64name = "?";
                } else
                    x64name = getAddrFunctionName(x64pc);
                signal_jmpbuf_active = 0;
            }
        }
        if(BOX64ENV(jitgdb)) {
            pid_t pid = getpid();
            int v = vfork(); // is this ok in a signal handler???
            if(v<0) {
                printf("Error while forking, cannot launch gdb (errp%d/%s)\n", errno, strerror(errno));
            } else if(v) {
                // parent process, the one that have the segfault
                volatile int waiting = 1;
                printf("Waiting for %s (pid %d)...\n", (BOX64ENV(jitgdb)==2)?"gdbserver":"gdb", pid);
                while(waiting) {
                    // using gdb, use "set waiting=0" to stop waiting...
                    usleep(1000);
                }
            } else {
                char myarg[50] = {0};
                sprintf(myarg, "%d", pid);
                if(BOX64ENV(jitgdb)==2)
                    execlp("gdbserver", "gdbserver", "127.0.0.1:1234", "--attach", myarg, (char*)NULL);
                else if(BOX64ENV(jitgdb)==3)
                    execlp("lldb", "lldb", "-p", myarg, (char*)NULL);
                else
                    execlp("gdb", "gdb", "-pid", myarg, (char*)NULL);
                exit(-1);
            }
        }
        print_rolling_log(log_minimum);

        if((BOX64ENV(showbt) || sig==SIGABRT) && log_minimum<=BOX64ENV(log)) {
            // show native bt
            #define BT_BUF_SIZE 100
            int nptrs;
            void *buffer[BT_BUF_SIZE];
            char **strings;

#ifndef ANDROID
            nptrs = backtrace(buffer, BT_BUF_SIZE);
            strings = backtrace_symbols(buffer, nptrs);
            if(strings) {
                for (int j = 0; j < nptrs; j++)
                    printf_log(log_minimum, "NativeBT: %s\n", strings[j]);
                free(strings);
            } else
                printf_log(log_minimum, "NativeBT: none (%d/%s)\n", errno, strerror(errno));
#endif
            extern int my_backtrace_ip(x64emu_t* emu, void** buffer, int size);   // in wrappedlibc
            extern char** my_backtrace_symbols(x64emu_t* emu, uintptr_t* buffer, int size);
            // save and set real RIP/RSP
            #define GO(A) uintptr_t old_##A = R_##A;
            GO(RAX);
            GO(RBX);
            GO(RCX);
            GO(RDX);
            GO(RBP);
            GO(RSP);
            GO(RDI);
            GO(RSI);
            GO(R8);
            GO(R9);
            GO(R10);
            GO(R11);
            GO(R12);
            GO(R13);
            GO(R14);
            GO(R15);
            GO(RIP);
            #undef GO
            #ifdef DYNAREC
            if(db)
                copyUCTXreg2Emu(emu, p, x64pc);
            #endif
            nptrs = my_backtrace_ip(emu, buffer, BT_BUF_SIZE);
            strings = my_backtrace_symbols(emu, (uintptr_t*)buffer, nptrs);
            if(strings) {
                for (int j = 0; j < nptrs; j++)
                    printf_log(log_minimum, "EmulatedBT: %s\n", strings[j]);
                free(strings);
            } else
                printf_log(log_minimum, "EmulatedBT: none\n");
            #define GO(A) R_##A = old_##A
            GO(RAX);
            GO(RBX);
            GO(RCX);
            GO(RDX);
            GO(RBP);
            GO(RSP);
            GO(RDI);
            GO(RSI);
            GO(R8);
            GO(R9);
            GO(R10);
            GO(R11);
            GO(R12);
            GO(R13);
            GO(R14);
            GO(R15);
            GO(RIP);
            #undef GO
        }

        if(log_minimum<=BOX64ENV(log)) {
            static const char* reg_name[] = {"RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI", "RDI", " R8", " R9","R10","R11", "R12","R13","R14","R15"};
            static const char* seg_name[] = {"ES", "CS", "SS", "DS", "FS", "GS"};
            int shown_regs = 0;
#ifdef DYNAREC
            uint32_t hash = 0;
            if(db)
                hash = X31_hash_code(db->x64_addr, db->x64_size);
            printf_log(log_minimum, "%04d|%s @%p (%s) (x64pc=%p/\"%s\", rsp=%p, stack=%p:%p own=%p fp=%p), for accessing %p (code=%d/prot=%x), db=%p(%p:%p/%p:%p/%s:%s, hash:%x/%x) handler=%p",
                GetTID(), signame, pc, name, (void*)x64pc, x64name?:"???", rsp,
                emu->init_stack, emu->init_stack+emu->size_stack, emu->stack2free, (void*)R_RBP,
                addr, info->si_code,
                prot, db, db?db->block:0, db?(db->block+db->size):0,
                db?db->x64_addr:0, db?(db->x64_addr+db->x64_size):0,
                getAddrFunctionName((uintptr_t)(db?db->x64_addr:0)),
                (db?getNeedTest((uintptr_t)db->x64_addr):0)?"needs_test":"clean", db?db->hash:0, hash,
                (void*)my_context->signals[sig]);
#if defined(ARM64)
            if(db) {
                shown_regs = 1;
                for (int i=0; i<16; ++i) {
                    if(!(i%4)) printf_log_prefix(0, log_minimum, "\n");
                    printf_log_prefix(0, log_minimum, "%s:0x%016llx ", reg_name[i], p->uc_mcontext.regs[10+i]);
                }
                printf_log_prefix(0, log_minimum, "\n");
                for (int i=0; i<6; ++i)
                    printf_log_prefix(0, log_minimum, "%s:0x%04x ", seg_name[i], emu->segs[i]);
            }
            if(rsp!=addr && getProtection((uintptr_t)rsp-4*8) && getProtection((uintptr_t)rsp+4*8))
                for (int i=-4; i<4; ++i) {
                    printf_log_prefix(0, log_minimum, "%sRSP%c0x%02x:0x%016lx", (i%4)?" ":"\n", i<0?'-':'+', abs(i)*8, *(uintptr_t*)(rsp+i*8));
                }
#elif defined(RV64)
            if(db) {
                shown_regs = 1;
                for (int i=0; i<16; ++i) {
                    if(!(i%4)) printf_log_prefix(0, log_minimum, "\n");
                    printf_log_prefix(0, log_minimum, "%s:0x%016llx ", reg_name[i], p->uc_mcontext.__gregs[(((uint8_t[]) { 16, 13, 12, 24, 9, 8, 11, 10, 14, 15, 26, 27, 18, 19, 20, 21 })[i])]);
                }
                printf_log_prefix(0, log_minimum, "\n");
                for (int i=0; i<6; ++i)
                    printf_log_prefix(0, log_minimum, "%s:0x%04x ", seg_name[i], emu->segs[i]);
            }
            if(rsp!=addr && getProtection((uintptr_t)rsp-4*8) && getProtection((uintptr_t)rsp+4*8))
                for (int i=-4; i<4; ++i) {
                    printf_log_prefix(0, log_minimum, "%sRSP%c0x%02x:0x%016lx", (i%4)?" ":"\n", i<0?'-':'+', abs(i)*8, *(uintptr_t*)(rsp+i*8));
                }
#elif defined(LA64)
            if(db) {
                shown_regs = 1;
                for (int i=0; i<16; ++i) {
                    if(!(i%4)) printf_log_prefix(0, log_minimum, "\n");
                    printf_log_prefix(0, log_minimum, "%s:0x%016llx ", reg_name[i], p->uc_mcontext.__gregs[12+i]);
                }
                printf_log_prefix(0, log_minimum, "\n");
                for (int i=0; i<6; ++i)
                    printf_log_prefix(0, log_minimum, "%s:0x%04x ", seg_name[i], emu->segs[i]);
            }
            if(rsp!=addr && getProtection((uintptr_t)rsp-4*8) && getProtection((uintptr_t)rsp+4*8))
                for (int i=-4; i<4; ++i) {
                    printf_log_prefix(0, log_minimum, "%sRSP%c0x%02x:0x%016lx", (i%4)?" ":"\n", i<0?'-':'+', abs(i)*8, *(uintptr_t*)(rsp+i*8));
                }
#else
            #warning TODO
#endif
#else
            printf_log(log_minimum, "%04d|%s @%p (%s) (x64pc=%p/\"%s\", rsp=%p), for accessing %p (code=%d)", GetTID(), signame, pc, name, (void*)x64pc, x64name?:"???", rsp, addr, info->si_code);
#endif
            if(!shown_regs) {
                for (int i=0; i<16; ++i) {
                    if(!(i%4)) printf_log_prefix(0, log_minimum, "\n");
                    printf_log_prefix(0, log_minimum, "%s:0x%016llx ", reg_name[i], emu->regs[i].q[0]);
                }
                printf_log_prefix(0, log_minimum, "\n");
                for (int i=0; i<6; ++i)
                    printf_log_prefix(0, log_minimum, "%s:0x%04x ", seg_name[i], emu->segs[i]);
            }
            if(sig==SIGILL) {
                printf_log_prefix(0, log_minimum, " opcode=%02X %02X %02X %02X %02X %02X %02X %02X (%02X %02X %02X %02X %02X)\n", ((uint8_t*)pc)[0], ((uint8_t*)pc)[1], ((uint8_t*)pc)[2], ((uint8_t*)pc)[3], ((uint8_t*)pc)[4], ((uint8_t*)pc)[5], ((uint8_t*)pc)[6], ((uint8_t*)pc)[7], ((uint8_t*)x64pc)[0], ((uint8_t*)x64pc)[1], ((uint8_t*)x64pc)[2], ((uint8_t*)x64pc)[3], ((uint8_t*)x64pc)[4]);
            } else if(sig==SIGBUS || (sig==SIGSEGV && (x64pc!=(uintptr_t)addr) && (pc!=addr))) {
                printf_log_prefix(0, log_minimum, " %sopcode=%02X %02X %02X %02X %02X %02X %02X %02X (opcode=%08x)\n", (emu->segs[_CS]==0x23)?"x86":"x64", ((uint8_t*)x64pc)[0], ((uint8_t*)x64pc)[1], ((uint8_t*)x64pc)[2], ((uint8_t*)x64pc)[3], ((uint8_t*)x64pc)[4], ((uint8_t*)x64pc)[5], ((uint8_t*)x64pc)[6], ((uint8_t*)x64pc)[7], *(uint32_t*)pc);
            } else {
                printf_log_prefix(0, log_minimum, "\n");
            }
        }
    }
    relockMutex(Locks);
    if(my_context->signals[sig] && my_context->signals[sig]!=1) {
        my_sigactionhandler_oldcode(emu, sig, my_context->is_sigaction[sig]?0:1, info, ucntx, &old_code, db, x64pc);
        return;
    }
    // no handler (or double identical segfault)
    // set default and that's it, instruction will restart and default segfault handler will be called...
    if(my_context->signals[sig]!=1 || sig==SIGSEGV || sig==SIGILL || sig==SIGFPE || sig==SIGABRT) {
        signal(sig, (void*)my_context->signals[sig]);
    }
}

void my_sigactionhandler(int32_t sig, siginfo_t* info, void * ucntx)
{
    void* pc = NULL;
    #ifdef DYNAREC
    ucontext_t *p = (ucontext_t *)ucntx;
    #ifdef ARM64
    pc = (void*)p->uc_mcontext.pc;
    #elif defined(LA64)
    pc = (void*)p->uc_mcontext.__pc;
    #elif defined(RV64)
    pc = (void*)p->uc_mcontext.__gregs[0];
    #else
    #error Unsupported architecture
    #endif
    #endif
    dynablock_t* db = FindDynablockFromNativeAddress(pc);
    x64emu_t* emu = thread_get_emu();
    uintptr_t x64pc = R_RIP;
    if(db)
        x64pc = getX64Address(db, (uintptr_t)pc);
    if(BOX64ENV(showsegv)) printf_log(LOG_INFO, "sigaction handler for sig %d, pc=%p, x64pc=%p, db=%p\n", sig, pc, x64pc, db);
    my_sigactionhandler_oldcode(emu, sig, 0, info, ucntx, NULL, db, x64pc);
}

void emit_signal(x64emu_t* emu, int sig, void* addr, int code)
{
    siginfo_t info = {0};
    info.si_signo = sig;
    info.si_errno = (sig==SIGSEGV)?0x1234:0;    // Mark as a sign this is a #GP(0) (like privileged instruction)
    info.si_code = code;
    if(sig==SIGSEGV && code==0xbad0) {
        info.si_errno = 0xbad0;
        info.si_code = 0;
    } else if(sig==SIGSEGV && code==0xecec) {
        info.si_errno = 0xecec;
        info.si_code = SEGV_ACCERR;
    }
    info.si_addr = addr;
    const char* x64name = NULL;
    const char* elfname = NULL;
    if(BOX64ENV(log)>LOG_INFO || BOX64ENV(dynarec_dump) || BOX64ENV(showsegv)) {
        x64name = getAddrFunctionName(R_RIP);
        elfheader_t* elf = FindElfAddress(my_context, R_RIP);
        if(elf)
            elfname = ElfName(elf);
        printf_log(LOG_NONE, "Emit Signal %d at IP=%p(%s / %s) / addr=%p, code=0x%x\n", sig, (void*)R_RIP, x64name?x64name:"???", elfname?elfname:"?", addr, code);
        print_rolling_log(LOG_INFO);

        if((BOX64ENV(showbt) || sig==SIGABRT) && BOX64ENV(log)>=LOG_INFO) {
            // show native bt
            #define BT_BUF_SIZE 100
            int nptrs;
            void *buffer[BT_BUF_SIZE];
            char **strings;

#ifndef ANDROID
            nptrs = backtrace(buffer, BT_BUF_SIZE);
            strings = backtrace_symbols(buffer, nptrs);
            if(strings) {
                for (int j = 0; j < nptrs; j++)
                    printf_log(LOG_INFO, "NativeBT: %s\n", strings[j]);
                free(strings);
            } else
                printf_log(LOG_INFO, "NativeBT: none (%d/%s)\n", errno, strerror(errno));
#endif
            extern int my_backtrace_ip(x64emu_t* emu, void** buffer, int size);   // in wrappedlibc
            extern char** my_backtrace_symbols(x64emu_t* emu, uintptr_t* buffer, int size);
            // save and set real RIP/RSP
            nptrs = my_backtrace_ip(emu, buffer, BT_BUF_SIZE);
            strings = my_backtrace_symbols(emu, (uintptr_t*)buffer, nptrs);
            if(strings) {
                for (int j = 0; j < nptrs; j++)
                    printf_log(LOG_INFO, "EmulatedBT: %s\n", strings[j]);
                free(strings);
            } else
                printf_log(LOG_INFO, "EmulatedBT: none\n");
        }
printf_log(LOG_NONE, DumpCPURegs(emu, R_RIP, emu->segs[_CS]==0x23));
printf_log(LOG_NONE, "Emu Stack: %p 0x%lx%s\n", emu->init_stack, emu->size_stack, emu->stack2free?" owned":"");
        //if(!elf) {
        //    FILE* f = fopen("/proc/self/maps", "r");
        //    if(f) {
        //        char line[1024];
        //        while(!feof(f)) {
        //            char* ret = fgets(line, sizeof(line), f);
        //            printf_log(LOG_NONE, "\t%s", ret);
        //        }
        //        fclose(f);
        //    }
        //}
        if(sig==SIGILL) {
            uint8_t* mem = (uint8_t*)R_RIP;
            printf_log(LOG_NONE, "SIGILL: Opcode at ip is %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n", mem[0], mem[1], mem[2], mem[3], mem[4], mem[5]);
        }
    }
    my_sigactionhandler_oldcode(emu, sig, 0, &info, NULL, NULL, NULL, R_RIP);
}

void check_exec(x64emu_t* emu, uintptr_t addr)
{
    if(box64_pagesize!=4096)
        return; //disabling the test, 4K pagesize simlation isn't good enough for this
    while((getProtection(addr)&(PROT_EXEC|PROT_READ))!=(PROT_EXEC|PROT_READ)) {
        R_RIP = addr;   // incase there is a slight difference
        emit_signal(emu, SIGSEGV, (void*)addr, 0xecec);
    }
}

void emit_interruption(x64emu_t* emu, int num, void* addr)
{
    siginfo_t info = {0};
    info.si_signo = SIGSEGV;
    info.si_errno = 0xdead;
    info.si_code = num;
    info.si_addr = NULL;//addr;
    const char* x64name = NULL;
    const char* elfname = NULL;
    if(BOX64ENV(log)>LOG_INFO || BOX64ENV(dynarec_dump) || BOX64ENV(showsegv)) {
        x64name = getAddrFunctionName(R_RIP);
        elfheader_t* elf = FindElfAddress(my_context, R_RIP);
        if(elf)
            elfname = ElfName(elf);
        printf_log(LOG_NONE, "Emit Interruption 0x%x at IP=%p(%s / %s) / addr=%p\n", num, (void*)R_RIP, x64name?x64name:"???", elfname?elfname:"?", addr);
    }
    my_sigactionhandler_oldcode(emu, SIGSEGV, 0, &info, NULL, NULL, NULL, R_RIP);
}

void emit_div0(x64emu_t* emu, void* addr, int code)
{
    siginfo_t info = {0};
    info.si_signo = SIGSEGV;
    info.si_errno = 0xcafe;
    info.si_code = code;
    info.si_addr = addr;
    const char* x64name = NULL;
    const char* elfname = NULL;
    if(BOX64ENV(log)>LOG_INFO || BOX64ENV(dynarec_dump) || BOX64ENV(showsegv)) {
        x64name = getAddrFunctionName(R_RIP);
        elfheader_t* elf = FindElfAddress(my_context, R_RIP);
        if(elf)
            elfname = ElfName(elf);
        printf_log(LOG_NONE, "Emit Divide by 0 at IP=%p(%s / %s) / addr=%p\n", (void*)R_RIP, x64name?x64name:"???", elfname?elfname:"?", addr);
    }
    my_sigactionhandler_oldcode(emu, SIGSEGV, 0, &info, NULL, NULL, NULL, R_RIP);
}

EXPORT sighandler_t my_signal(x64emu_t* emu, int signum, sighandler_t handler)
{
    if(signum<0 || signum>MAX_SIGNAL)
        return SIG_ERR;

    if(signum==SIGSEGV && emu->context->no_sigsegv)
        return 0;

    // create a new handler
    my_context->signals[signum] = (uintptr_t)handler;
    my_context->is_sigaction[signum] = 0;
    my_context->restorer[signum] = 0;
    my_context->onstack[signum] = 0;

    if(signum==SIGSEGV || signum==SIGBUS || signum==SIGILL || signum==SIGABRT)
        return 0;

    if(handler!=NULL && handler!=(sighandler_t)1) {
        struct sigaction newact = {0};
        struct sigaction oldact = {0};
        newact.sa_flags = 0x04;
        newact.sa_sigaction = my_sigactionhandler;
        sigaction(signum, &newact, &oldact);
        return oldact.sa_handler;
    } else
        return signal(signum, handler);
}
EXPORT sighandler_t my___sysv_signal(x64emu_t* emu, int signum, sighandler_t handler) __attribute__((alias("my_signal")));
EXPORT sighandler_t my_sysv_signal(x64emu_t* emu, int signum, sighandler_t handler) __attribute__((alias("my_signal")));    // not completely exact

int EXPORT my_sigaction(x64emu_t* emu, int signum, const x64_sigaction_t *act, x64_sigaction_t *oldact)
{
    printf_log(LOG_DEBUG, "Sigaction(signum=%d, act=%p(f=%p, flags=0x%x), old=%p)\n", signum, act, act?act->_u._sa_handler:NULL, act?act->sa_flags:0, oldact);
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
            if(act->_u._sa_handler!=NULL && act->_u._sa_handler!=(sighandler_t)1) {
                newact.sa_sigaction = my_sigactionhandler;
            } else
                newact.sa_sigaction = act->_u._sa_sigaction;
        } else {
            my_context->signals[signum] = (uintptr_t)act->_u._sa_handler;
            my_context->is_sigaction[signum] = 0;
            if(act->_u._sa_handler!=NULL && act->_u._sa_handler!=(sighandler_t)1) {
                newact.sa_flags|=0x04;
                newact.sa_sigaction = my_sigactionhandler;
            } else
                newact.sa_handler = act->_u._sa_handler;
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
            oldact->_u._sa_sigaction = old.sa_sigaction; //TODO should wrap...
        else
            oldact->_u._sa_handler = old.sa_handler;  //TODO should wrap...
        if((uintptr_t)oldact->_u._sa_sigaction == (uintptr_t)my_sigactionhandler && old_handler)
            oldact->_u._sa_sigaction = (void*)old_handler;
        oldact->sa_restorer = NULL; // no handling for now...
    }
    return ret;
}
int EXPORT my___sigaction(x64emu_t* emu, int signum, const x64_sigaction_t *act, x64_sigaction_t *oldact)
__attribute__((alias("my_sigaction")));

int EXPORT my_syscall_rt_sigaction(x64emu_t* emu, int signum, const x64_sigaction_restorer_t *act, x64_sigaction_restorer_t *oldact, int sigsetsize)
{
    printf_log(LOG_DEBUG, "Syscall/Sigaction(signum=%d, act=%p, old=%p, size=%d)\n", signum, act, oldact, sigsetsize);
    if(signum<0 || signum>MAX_SIGNAL) {
        errno = EINVAL;
        return -1;
    }

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
                    newact.sa_flags|=0x4;
                    newact.k_sa_handler = (void*)my_sigactionhandler;
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
            printf_log(LOG_DEBUG, " New action for signal #%d flags=0x%x mask=0x%lx\n", signum, act->sa_flags, *(uint64_t*)&act->sa_mask);
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
                    newact.sa_sigaction = my_sigactionhandler;
                    newact.sa_flags|=0x4;
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

        if(signum!=SIGSEGV && signum!=SIGBUS && signum!=SIGILL && signum!=SIGABRT)
            ret = sigaction(signum, act?&newact:NULL, oldact?&old:NULL);
        if(oldact && ret==0) {
            oldact->sa_flags = old.sa_flags;
            memcpy(&oldact->sa_mask, &old.sa_mask, (sigsetsize>8)?8:sigsetsize);
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
    u->uc_mcontext.fpregs = ucp + 408;
    fpu_savenv(emu, (void*)u->uc_mcontext.fpregs, 1);
    *(uint32_t*)(ucp + 432) = emu->mxcsr.x32;

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
    fpu_loadenv(emu, (void*)u->uc_mcontext.fpregs, 1);
    emu->mxcsr.x32 = *(uint32_t*)(ucp + 432);
    // set signal mask
    sigprocmask(SIG_SETMASK, (sigset_t*)&u->uc_sigmask, NULL);
    errno = 0;

    return R_EAX;
}
void vFEv(x64emu_t *emu, uintptr_t fnc);
EXPORT void my_start_context(x64emu_t* emu)
{
    // this is call indirectly by swapcontext from a makecontext, and will link context or just exit
    x64_ucontext_t *u = *(x64_ucontext_t**)R_RBX;
    if(u)
        my_setcontext(emu, u);
    else
        emu->quit = 1;
}

EXPORT void my_makecontext(x64emu_t* emu, void* ucp, void* fnc, int32_t argc, int64_t* argv)
{
//    printf_log(LOG_NONE, "Warning: call to unimplemented makecontext\n");
    x64_ucontext_t *u = (x64_ucontext_t*)ucp;
    // setup stack
    uintptr_t* rsp = (uintptr_t*)(u->uc_stack.ss_sp + u->uc_stack.ss_size - sizeof(uintptr_t));
    // setup the function
    u->uc_mcontext.gregs[X64_RIP] = (intptr_t)fnc;
    // setup return to private start_context uc_link
    *rsp = (uintptr_t)u->uc_link;
    u->uc_mcontext.gregs[X64_RBX] = (uintptr_t)rsp;
    --rsp;
    // setup args
    int n = 3;
    int j = 0;
    int regs_abi[] = {_DI, _SI, _DX, _CX, _R8, _R9};
    for (int i=0; i<argc; ++i) {
        // get value first
        uint32_t v;
        if(n<6)
            v = emu->regs[regs_abi[n++]].dword[0];
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
    *rsp = AddCheckBridge(my_context->system, vFEv, my_start_context, 0, "my_start_context");//my_context->exit_bridge;
    u->uc_mcontext.gregs[X64_RSP] = (uintptr_t)rsp;
}

void box64_abort() {
    if(BOX64ENV(showbt) && LOG_INFO<=BOX64ENV(log)) {
            // show native bt
            #define BT_BUF_SIZE 100
            int nptrs;
            void *buffer[BT_BUF_SIZE];
            char **strings;
            x64emu_t* emu = thread_get_emu();

#ifndef ANDROID
            nptrs = backtrace(buffer, BT_BUF_SIZE);
            strings = backtrace_symbols(buffer, nptrs);
            if(strings) {
                for (int j = 0; j < nptrs; j++)
                    printf_log(LOG_INFO, "NativeBT: %s\n", strings[j]);
                free(strings);
            } else
                printf_log(LOG_INFO, "NativeBT: none (%d/%s)\n", errno, strerror(errno));
#endif
            extern int my_backtrace_ip(x64emu_t* emu, void** buffer, int size);   // in wrappedlibc
            extern char** my_backtrace_symbols(x64emu_t* emu, uintptr_t* buffer, int size);
            nptrs = my_backtrace_ip(emu, buffer, BT_BUF_SIZE);
            strings = my_backtrace_symbols(emu, (uintptr_t*)buffer, nptrs);
            if(strings) {
                for (int j = 0; j < nptrs; j++)
                    printf_log(LOG_INFO, "EmulatedBT: %s\n", strings[j]);
                free(strings);
            } else
                printf_log(LOG_INFO, "EmulatedBT: none\n");
        }
    abort();
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
#ifdef USE_SIGNAL_MUTEX
static void atfork_child_dynarec_prot(void)
{
    #ifdef USE_CUSTOM_MUTEX
    native_lock_store(&mutex_dynarec_prot, 0);
    #else
    pthread_mutex_t tmp = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
    memcpy(&mutex_dynarec_prot, &tmp, sizeof(mutex_dynarec_prot));
    #endif
}
#endif
void init_signal_helper(box64context_t* context)
{
    // setup signal handling
    for(int i=0; i<=MAX_SIGNAL; ++i) {
        context->signals[i] = 0;    // SIG_DFL
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
    action.sa_flags = SA_SIGINFO | SA_RESTART | SA_NODEFER;
    action.sa_sigaction = my_box64signalhandler;
    sigaction(SIGABRT, &action, NULL);

    pthread_once(&sigstack_key_once, sigstack_key_alloc);
#ifdef USE_SIGNAL_MUTEX
    atfork_child_dynarec_prot();
    pthread_atfork(NULL, NULL, atfork_child_dynarec_prot);
#endif
}

void fini_signal_helper()
{
    signal(SIGSEGV, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    signal(SIGILL, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
}
