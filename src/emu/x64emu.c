#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/mman.h>

#include "debug.h"
#include "box64stack.h"
#include "x64emu.h"
#include "x64emu_private.h"
#include "x87emu_private.h"
#include "box64context.h"
#include "x64run.h"
#include "x64run_private.h"
#include "callback.h"
#include "bridge.h"
#include "elfs/elfloader_private.h"
#ifdef HAVE_TRACE
#include "x64trace.h"
#endif
#include "custommem.h"
// for the applyFlushTo0
#ifdef __x86_64__
#include <immintrin.h>
#elif defined(__aarch64__)
#else
#warning Architecture cannot follow SSE Flush to 0 flag
#endif

typedef struct cleanup_s {
    void*       f;
    int         arg;
    void*       a;
} cleanup_t;

static uint32_t x86emu_parity_tab[8] =
{
	0x96696996,
	0x69969669,
	0x69969669,
	0x96696996,
	0x69969669,
	0x96696996,
	0x96696996,
	0x69969669,
};

uint32_t* GetParityTab()
{
    return x86emu_parity_tab;
}

static void internalX64Setup(x64emu_t* emu, box64context_t *context, uintptr_t start, uintptr_t stack, int stacksize, int ownstack)
{
    emu->context = context;
    // setup cpu helpers
    for (int i=0; i<16; ++i)
        emu->sbiidx[i] = &emu->regs[i];
    emu->sbiidx[4] = &emu->zero;
    emu->x64emu_parity_tab = x86emu_parity_tab;
    emu->eflags.x64 = 0x202; // default flags?
    // own stack?
    emu->stack2free = (ownstack)?(void*)stack:NULL;
    emu->init_stack = (void*)stack;
    emu->size_stack = stacksize;
    // set default value
    R_RIP = start;
    R_RSP = (stack + stacksize) & ~7;   // align stack start, always
    // fake init of segments...
    emu->segs[_CS] = 0x33;
    emu->segs[_DS] = emu->segs[_ES] = emu->segs[_SS] = 0x2b;
    emu->segs[_FS] = 0x43;
    emu->segs[_GS] = default_gs;
    // setup fpu regs
    reset_fpu(emu);
    emu->mxcsr.x32 = 0x1f80;
}

EXPORTDYN
x64emu_t *NewX64Emu(box64context_t *context, uintptr_t start, uintptr_t stack, int stacksize, int ownstack)
{
    printf_log(LOG_DEBUG, "Allocate a new X86_64 Emu, with EIP=%p and Stack=%p/0x%X\n", (void*)start, (void*)stack, stacksize);

    x64emu_t *emu = (x64emu_t*)box_calloc(1, sizeof(x64emu_t));

    internalX64Setup(emu, context, start, stack, stacksize, ownstack);

    return emu;
}

x64emu_t *NewX64EmuFromStack(x64emu_t* emu, box64context_t *context, uintptr_t start, uintptr_t stack, int stacksize, int ownstack)
{
    printf_log(LOG_DEBUG, "New X86_64 Emu from stack, with EIP=%p and Stack=%p/0x%X\n", (void*)start, (void*)stack, stacksize);

    internalX64Setup(emu, context, start, stack, stacksize, ownstack);
    
    return emu;
}

EXPORTDYN
void SetupX64Emu(x64emu_t *emu, x64emu_t *ref)
{
    printf_log(LOG_DEBUG, "Setup X86_64 Emu\n");
    if(ref) {
        // save RIP and RSP
        uintptr_t old_rip = R_RIP;
        uintptr_t old_rsp = R_RSP;
        CloneEmu(emu, ref);
        // restore RIP and RSP
        R_RIP = old_rip;
        R_RSP = old_rsp;
    }
}

#ifdef HAVE_TRACE
void SetTraceEmu(uintptr_t start, uintptr_t end)
{
    if(my_context->zydis) {
        if (end == 0) {
            printf_log(LOG_INFO, "Setting trace\n");
        } else {
            if(end!=1) {  // 0-1 is basically no trace, so don't printf it...
                printf_log(LOG_INFO, "Setting trace only between %p and %p\n", (void*)start, (void*)end);
            }
        }
    }
    trace_start = start;
    trace_end = end;
}
#endif

void AddCleanup(x64emu_t *emu, void *p)
{
    (void)emu;
    
    if(my_context->clean_sz == my_context->clean_cap) {
        my_context->clean_cap += 32;
        my_context->cleanups = (cleanup_t*)box_realloc(my_context->cleanups, sizeof(cleanup_t)*my_context->clean_cap);
    }
    my_context->cleanups[my_context->clean_sz].arg = 0;
    my_context->cleanups[my_context->clean_sz].a = NULL;
    my_context->cleanups[my_context->clean_sz++].f = p;
}

void AddCleanup1Arg(x64emu_t *emu, void *p, void* a, elfheader_t* h)
{
    (void)emu;
    if(!h)
        return;
    
    if(h->clean_sz == h->clean_cap) {
        h->clean_cap += 32;
        h->cleanups = (cleanup_t*)box_realloc(h->cleanups, sizeof(cleanup_t)*h->clean_cap);
    }
    h->cleanups[h->clean_sz].arg = 1;
    h->cleanups[h->clean_sz].a = a;
    h->cleanups[h->clean_sz++].f = p;
}

void CallCleanup(x64emu_t *emu, elfheader_t* h)
{
    printf_log(LOG_DEBUG, "Calling atexit registered functions for elf: %p/%s\n", h, h?h->name:"(nil)");
    if(!h)
        return;
    for(int i=h->clean_sz-1; i>=0; --i) {
        printf_log(LOG_DEBUG, "Call cleanup #%d\n", i);
        RunFunctionWithEmu(emu, 0, (uintptr_t)(h->cleanups[i].f), h->cleanups[i].arg, h->cleanups[i].a );
        // now remove the cleanup
        if(i!=h->clean_sz-1)
            memmove(h->cleanups+i, h->cleanups+i+1, (h->clean_sz-i-1)*sizeof(cleanup_t));
        --h->clean_sz;
    }
}

void CallAllCleanup(x64emu_t *emu)
{
    printf_log(LOG_DEBUG, "Calling atexit registered functions\n");
    for(int i=my_context->clean_sz-1; i>=0; --i) {
        printf_log(LOG_DEBUG, "Call cleanup #%d\n", i);
        --my_context->clean_sz;
        RunFunctionWithEmu(emu, 0, (uintptr_t)(my_context->cleanups[i].f), my_context->cleanups[i].arg, my_context->cleanups[i].a );
    }
    box_free(my_context->cleanups);
    my_context->cleanups = NULL;
}

static void internalFreeX64(x64emu_t* emu)
{
    if(emu && emu->stack2free) {
        if(!internal_munmap(emu->stack2free, emu->size_stack))
            freeProtection((uintptr_t)emu->stack2free, emu->size_stack);
    }
}

EXPORTDYN
void FreeX64Emu(x64emu_t **emu)
{
    if(!emu)
        return;
    printf_log(LOG_DEBUG, "%04d|Free a X86_64 Emu (%p)\n", GetTID(), *emu);

    if((*emu)->test.emu) {
        internalFreeX64((*emu)->test.emu);
        box_free((*emu)->test.emu);
        (*emu)->test.emu = NULL;
    }
    internalFreeX64(*emu);

    box_free(*emu);
    *emu = NULL;
}

void FreeX64EmuFromStack(x64emu_t **emu)
{
    if(!emu)
        return;
    printf_log(LOG_DEBUG, "%04d|Free a X86_64 Emu from stack (%p)\n", GetTID(), *emu);

    internalFreeX64(*emu);
}

void CloneEmu(x64emu_t *newemu, const x64emu_t* emu)
{
	memcpy(newemu->regs, emu->regs, sizeof(emu->regs));
    memcpy(&newemu->ip, &emu->ip, sizeof(emu->ip));
	memcpy(&newemu->eflags, &emu->eflags, sizeof(emu->eflags));
    newemu->old_ip = emu->old_ip;
    memcpy(newemu->segs, emu->segs, sizeof(emu->segs));
    memset(newemu->segs_serial, 0, sizeof(newemu->segs_serial));
	memcpy(newemu->x87, emu->x87, sizeof(emu->x87));
	memcpy(newemu->mmx, emu->mmx, sizeof(emu->mmx));
    memcpy(newemu->fpu_ld, emu->fpu_ld, sizeof(emu->fpu_ld));
    memcpy(newemu->fpu_ll, emu->fpu_ll, sizeof(emu->fpu_ll));
    newemu->fpu_tags = emu->fpu_tags;
	newemu->cw = emu->cw;
	newemu->sw = emu->sw;
	newemu->top = emu->top;
    newemu->fpu_stack = emu->fpu_stack;
    memcpy(newemu->xmm, emu->xmm, sizeof(emu->xmm));
    newemu->df = emu->df;
    newemu->df_sav = emu->df_sav;
    newemu->op1 = emu->op1;
    newemu->op2 = emu->op2;
    newemu->res = emu->res;
    newemu->op1_sav = emu->op1_sav;
    newemu->res_sav = emu->res_sav;
    newemu->mxcsr = emu->mxcsr;
    newemu->quit = emu->quit;
    newemu->error = emu->error;
    // adapt R_RSP to new stack frame
    uintptr_t oldst = (uintptr_t)((emu->init_stack)?emu->init_stack:emu->context->stack);
    uintptr_t newst = (uintptr_t)((newemu->init_stack)?newemu->init_stack:newemu->context->stack);
    newemu->regs[_SP].q[0] = emu->regs[_SP].q[0] + (intptr_t)(newst - oldst);
}

void CopyEmu(x64emu_t *newemu, const x64emu_t* emu)
{
	memcpy(newemu->regs, emu->regs, sizeof(emu->regs));
    memcpy(&newemu->ip, &emu->ip, sizeof(emu->ip));
	memcpy(&newemu->eflags, &emu->eflags, sizeof(emu->eflags));
    newemu->old_ip = emu->old_ip;
    memcpy(newemu->segs, emu->segs, sizeof(emu->segs));
    memcpy(newemu->segs_serial, emu->segs_serial, sizeof(emu->segs_serial));
    memcpy(newemu->segs_offs, emu->segs_offs, sizeof(emu->segs_offs));
	memcpy(newemu->x87, emu->x87, sizeof(emu->x87));
	memcpy(newemu->mmx, emu->mmx, sizeof(emu->mmx));
    memcpy(newemu->xmm, emu->xmm, sizeof(emu->xmm));
    memcpy(newemu->fpu_ld, emu->fpu_ld, sizeof(emu->fpu_ld));
    memcpy(newemu->fpu_ll, emu->fpu_ll, sizeof(emu->fpu_ll));
    newemu->fpu_tags = emu->fpu_tags;
	newemu->cw = emu->cw;
	newemu->sw = emu->sw;
	newemu->top = emu->top;
    newemu->fpu_stack = emu->fpu_stack;
    newemu->df = emu->df;
    newemu->df_sav = emu->df_sav;
    newemu->op1 = emu->op1;
    newemu->op2 = emu->op2;
    newemu->res = emu->res;
    newemu->op1_sav = emu->op1_sav;
    newemu->res_sav = emu->res_sav;
    newemu->mxcsr = emu->mxcsr;
    newemu->quit = emu->quit;
    newemu->error = emu->error;
}

box64context_t* GetEmuContext(x64emu_t* emu)
{
    return emu->context;
}

uint32_t GetEAX(x64emu_t *emu)
{
    return R_EAX;
}
uint64_t GetRAX(x64emu_t *emu)
{
    return R_RAX;
}
void SetEAX(x64emu_t *emu, uint32_t v)
{
    R_EAX = v;
}
void SetEBX(x64emu_t *emu, uint32_t v)
{
    R_EBX = v;
}
void SetECX(x64emu_t *emu, uint32_t v)
{
    R_ECX = v;
}
void SetEDX(x64emu_t *emu, uint32_t v)
{
    R_EDX = v;
}
void SetESI(x64emu_t *emu, uint32_t v)
{
    R_ESI = v;
}
void SetEDI(x64emu_t *emu, uint32_t v)
{
    R_EDI = v;
}
void SetEBP(x64emu_t *emu, uint32_t v)
{
    R_EBP = v;
}
//void SetESP(x64emu_t *emu, uint32_t v)
//{
//    R_ESP = v;
//}
void SetRAX(x64emu_t *emu, uint64_t v)
{
    R_RAX = v;
}
void SetRBX(x64emu_t *emu, uint64_t v)
{
    R_RBX = v;
}
void SetRCX(x64emu_t *emu, uint64_t v)
{
    R_RCX = v;
}
void SetRDX(x64emu_t *emu, uint64_t v)
{
    R_RDX = v;
}
void SetRSI(x64emu_t *emu, uint64_t v)
{
    R_RSI = v;
}
void SetRDI(x64emu_t *emu, uint64_t v)
{
    R_RDI = v;
}
void SetRBP(x64emu_t *emu, uint64_t v)
{
    R_RBP = v;
}
void SetRSP(x64emu_t *emu, uint64_t v)
{
    R_RSP = v;
}
void SetRIP(x64emu_t *emu, uint64_t v)
{
    R_RIP = v;
}
uint64_t GetRSP(x64emu_t *emu)
{
    return R_RSP;
}
uint64_t GetRBP(x64emu_t *emu)
{
    return R_RBP;
}
/*void SetFS(x64emu_t *emu, uint16_t v)
{
    emu->segs[_FS] = v;
    emu->segs_serial[_FS] = 0;
}
uint16_t GetFS(x64emu_t *emu)
{
    return emu->segs[_FS];
}*/


void ResetFlags(x64emu_t *emu)
{
    emu->df = d_none;
}

const char* DumpCPURegs(x64emu_t* emu, uintptr_t ip, int is32bits)
{
    static char buff[4096];
    static const char* regname[] = {"RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI", "RDI",
                                    " R8", " R9", "R10", "R11", "R12", "R13", "R14", "R15"};
    static const char* regname32[]={"EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI"};
    static const char* segname[] = {"ES", "CS", "SS", "DS", "FS", "GS"};
    char tmp[160];
    buff[0] = '\0';
#ifdef HAVE_TRACE
    if(trace_emm) {
        // do emm reg if needed
        for(int i=0; i<8; ++i) {
            sprintf(tmp, "mm%d:%016lx", i, emu->mmx[i].q);
            strcat(buff, tmp);
            if ((i&3)==3) strcat(buff, "\n"); else strcat(buff, " ");
        }
    }
    if(trace_xmm) {
        // do xmm reg if needed
        for(int i=0; i<(is32bits?8:16); ++i) {
            if (trace_regsdiff && (emu->old_xmm[i].q[0] != emu->xmm[i].q[0] || emu->old_xmm[i].q[1] != emu->xmm[i].q[1])) {
                sprintf(tmp, "\e[1;35m%02d:%016lx-%016lx\e[m", i, emu->xmm[i].q[1], emu->xmm[i].q[0]);
                emu->old_xmm[i].q[0] = emu->xmm[i].q[0];
                emu->old_xmm[i].q[1] = emu->xmm[i].q[1];
            } else {
                sprintf(tmp, "%02d:%016lx-%016lx", i, emu->xmm[i].q[1], emu->xmm[i].q[0]);
            }
            strcat(buff, tmp);
            if ((i&3)==3) strcat(buff, "\n"); else strcat(buff, " ");
        }
    }
#endif
    // start with FPU regs...
    if(emu->fpu_stack) {
        int stack = emu->fpu_stack;
        if(stack>8) stack = 8;
        for (int i=0; i<stack; i++) {
            sprintf(tmp, "ST%d=%f", i, ST(i).d);
            strcat(buff, tmp);
            int c = 10-strlen(tmp);
            if(c<1) c=1;
            while(c--) strcat(buff, " ");
            if(i==3) strcat(buff, "\n");
        }
        sprintf(tmp, " C3210 = %d%d%d%d", emu->sw.f.F87_C3, emu->sw.f.F87_C2, emu->sw.f.F87_C1, emu->sw.f.F87_C0);
        strcat(buff, tmp);
        strcat(buff, "\n");
    }
    for (int i=0; i<6; ++i) {
            sprintf(tmp, "%s=0x%04x", segname[i], emu->segs[i]);
            strcat(buff, tmp);
            if(i!=_GS)
                strcat(buff, " ");
    }
    strcat(buff, "\n");
    if(is32bits)
        for (int i=_AX; i<=_RDI; ++i) {
#ifdef HAVE_TRACE
            if (trace_regsdiff && (emu->regs[i].dword[0] != emu->oldregs[i].q[0])) {
                sprintf(tmp, "\e[1;35m%s=%08x\e[m ", regname32[i], emu->regs[i].dword[0]);
                emu->oldregs[i].q[0] = emu->regs[i].dword[0];
            } else {
                sprintf(tmp, "%s=%08x ", regname32[i], emu->regs[i].dword[0]);
            }
#else
            sprintf(tmp, "%s=%08x ", regname[i], emu->regs[i].dword[0]);
#endif
            strcat(buff, tmp);

            if(i==_RBX) {
                if(emu->df) {
#define FLAG_CHAR(f) (ACCESS_FLAG(F_##f##F)) ? #f : "?"
                    sprintf(tmp, "flags=%s%s%s%s%s%s%s\n", FLAG_CHAR(O), FLAG_CHAR(D), FLAG_CHAR(S), FLAG_CHAR(Z), FLAG_CHAR(A), FLAG_CHAR(P), FLAG_CHAR(C));
                    strcat(buff, tmp);
#undef FLAG_CHAR
                } else {
#define FLAG_CHAR(f) (ACCESS_FLAG(F_##f##F)) ? #f : "-"
                    sprintf(tmp, "FLAGS=%s%s%s%s%s%s%s\n", FLAG_CHAR(O), FLAG_CHAR(D), FLAG_CHAR(S), FLAG_CHAR(Z), FLAG_CHAR(A), FLAG_CHAR(P), FLAG_CHAR(C));
                    strcat(buff, tmp);
#undef FLAG_CHAR
                }
            }
        }
    else
        for (int i=_AX; i<=_R15; ++i) {
#ifdef HAVE_TRACE
            if (trace_regsdiff && (emu->regs[i].q[0] != emu->oldregs[i].q[0])) {
                sprintf(tmp, "\e[1;35m%s=%016lx\e[m ", regname[i], emu->regs[i].q[0]);
                emu->oldregs[i].q[0] = emu->regs[i].q[0];
            } else {
                sprintf(tmp, "%s=%016lx ", regname[i], emu->regs[i].q[0]);
            }
#else
            sprintf(tmp, "%s=%016lx ", regname[i], emu->regs[i].q[0]);
#endif
            strcat(buff, tmp);

            if (i%5==4) {
                if(i==4) {
                    if(emu->df) {
#define FLAG_CHAR(f) (ACCESS_FLAG(F_##f##F)) ? #f : "?"
                        sprintf(tmp, "flags=%s%s%s%s%s%s%s\n", FLAG_CHAR(O), FLAG_CHAR(D), FLAG_CHAR(S), FLAG_CHAR(Z), FLAG_CHAR(A), FLAG_CHAR(P), FLAG_CHAR(C));
                        strcat(buff, tmp);
#undef FLAG_CHAR
                    } else {
#define FLAG_CHAR(f) (ACCESS_FLAG(F_##f##F)) ? #f : "-"
                        sprintf(tmp, "FLAGS=%s%s%s%s%s%s%s\n", FLAG_CHAR(O), FLAG_CHAR(D), FLAG_CHAR(S), FLAG_CHAR(Z), FLAG_CHAR(A), FLAG_CHAR(P), FLAG_CHAR(C));
                        strcat(buff, tmp);
#undef FLAG_CHAR
                    }
                } else {
                    strcat(buff, "\n");
                }
            } 
    }
    if(is32bits)
        sprintf(tmp, "EIP=%08lx ", ip);
    else
        sprintf(tmp, "RIP=%016lx ", ip);
    strcat(buff, tmp);
    return buff;
}

void StopEmu(x64emu_t* emu, const char* reason, int is32bits)
{
    emu->quit = 1;
    printf_log(LOG_NONE, "%s", reason);
    // dump stuff...
    printf_log(LOG_NONE, "==== CPU Registers ====\n%s\n", DumpCPURegs(emu, R_RIP, is32bits));
    printf_log(LOG_NONE, "======== Stack ========\nStack is from %lX to %lX\n", R_RBP, R_RSP);
    if (R_RBP == R_RSP) {
        printf_log(LOG_NONE, "RBP = RSP: leaf function detected; next 128 bytes should be either data or random.\n");
    } else {
        // TODO: display stack if operation should be allowed (to avoid crashes)
        /* for (uint64_t *sp = R_RBP; sp >= R_RSP; --sp) {
        } */
    }
    printf_log(LOG_NONE, "Old IP: %tX\n", emu->old_ip);
#ifdef HAVE_TRACE
    printf_log(LOG_NONE, "%s\n", DecodeX64Trace(my_context->dec, emu->old_ip));
#endif
}

void UnimpOpcode(x64emu_t* emu, int is32bits)
{
    R_RIP = emu->old_ip;

    int tid = syscall(SYS_gettid);
    printf_log(LOG_NONE, "%04d|%p: Unimplemented %sOpcode (%02X %02X %02X %02X) %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n", 
        tid, (void*)emu->old_ip, is32bits?"32bits ":"",
        Peek(emu, -4), Peek(emu, -3), Peek(emu, -2), Peek(emu, -1),
        Peek(emu, 0), Peek(emu, 1), Peek(emu, 2), Peek(emu, 3),
        Peek(emu, 4), Peek(emu, 5), Peek(emu, 6), Peek(emu, 7),
        Peek(emu, 8), Peek(emu, 9), Peek(emu,10), Peek(emu,11),
        Peek(emu,12), Peek(emu,13), Peek(emu,14));
    emu->quit=1;
    emu->error |= ERR_UNIMPL;
}

void EmuCall(x64emu_t* emu, uintptr_t addr)
{
    uint64_t old_rsp = R_RSP;
    uint64_t old_rbx = R_RBX;
    uint64_t old_rdi = R_RDI;
    uint64_t old_rsi = R_RSI;
    uint64_t old_rbp = R_RBP;
    uint64_t old_rip = R_RIP;
    //Push64(emu, GetRBP(emu));   // set frame pointer
    //SetRBP(emu, GetRSP(emu));   // save RSP
    R_RSP -= 200;
    R_RSP &= ~63LL;
    PushExit(emu);
    R_RIP = addr;
    emu->df = d_none;
    Run(emu, 0);
    emu->quit = 0;  // reset Quit flags...
    emu->df = d_none;
    if(emu->flags.quitonlongjmp && emu->flags.longjmp) {
        if(emu->flags.quitonlongjmp==1)
            emu->flags.longjmp = 0;   // don't change anything because of the longjmp
    } else {
        R_RBX = old_rbx;
        R_RDI = old_rdi;
        R_RSI = old_rsi;
        R_RBP = old_rbp;
        R_RSP = old_rsp;
        R_RIP = old_rip;  // and set back instruction pointer
    }
}

#if defined(RV64)
static size_t readBinarySizeFromFile(const char* fname)
{
    if (access(fname, R_OK) != 0) return -1;
    FILE* fp = fopen(fname, "r");
    if (fp == NULL) return -1;

    char b[sizeof(uint64_t)] = { 0 }, tmp;
    ssize_t n = fread(b, 1, sizeof(b), fp);
    if (n <= 0) return -1;

    for (ssize_t i = 0; i < n / 2; i++) {
        tmp = b[n - i - 1];
        b[n - i - 1] = b[i];
        b[i] = tmp;
    }
    return *(uint64_t*)b;
}

static inline uint64_t readCycleCounter()
{
    uint64_t val;
    asm volatile("rdtime %0"
                 : "=r"(val));
    return val;
}

static inline uint64_t readFreq()
{
    static size_t val = -1;
    if (val != -1) return val;

    val = readBinarySizeFromFile("/sys/firmware/devicetree/base/cpus/timebase-frequency");
    if (val != -1) return val;

    // fallback to rdtime + sleep
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 50000000; // 50 milliseconds
    uint64_t cycles = readCycleCounter();
    nanosleep(&ts, NULL);
    // round to MHz
    val = (size_t)round(((double)(readCycleCounter() - cycles) * 20) / 1e6) * 1e6;
    return (uint64_t)val;
}
#elif defined(ARM64)
static inline uint64_t readCycleCounter()
{
    uint64_t val;
    asm volatile("mrs %0, cntvct_el0"
                 : "=r"(val));
    return val;
}
static inline uint64_t readFreq()
{
    uint64_t val;
    asm volatile("mrs %0, cntfrq_el0"
                 : "=r"(val));
    return val;
}
#endif

uint64_t ReadTSC(x64emu_t* emu)
{
    (void)emu;
    
    // Hardware counter, per architecture
#if defined(ARM64) || defined(RV64)
    if (!box64_rdtsc) return readCycleCounter();
#endif
    // fall back to gettime...
#if !defined(NOGETCLOCK)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    return (uint64_t)(ts.tv_sec) * 1000000000LL + ts.tv_nsec;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000000 + tv.tv_usec;
#endif
}

uint64_t ReadTSCFrequency(x64emu_t* emu)
{
    (void)emu;
    // Hardware counter, per architecture
#if defined(ARM64) || defined(RV64)
    if (!box64_rdtsc) return readFreq();
#endif
    // fall back to get time
#if !defined(NOGETCLOCK)
    return 1000000000LL;
#else
    return 1000000;
#endif
}

void ResetSegmentsCache(x64emu_t *emu)
{
    if(!emu)
        return;
    memset(emu->segs_serial, 0, sizeof(emu->segs_serial));
}

void applyFlushTo0(x64emu_t* emu)
{
    #ifdef __x86_64__
    _mm_setcsr(_mm_getcsr() | (emu->mxcsr.x32&0x8040));
    #elif defined(__aarch64__)
    #if defined(__ANDROID__) || defined(__clang__)
    uint64_t fpcr;
    __asm__ __volatile__ ("mrs    %0, fpcr":"=r"(fpcr));
    #else
    uint64_t fpcr = __builtin_aarch64_get_fpcr();
    #endif
    fpcr &= ~((1<<24) | (1<<1));    // clear bit FZ (24) and AH (1)
    fpcr |= (emu->mxcsr.f.MXCSR_FZ)<<24;  // set FZ as mxcsr FZ
    fpcr |= ((emu->mxcsr.f.MXCSR_DAZ)^(emu->mxcsr.f.MXCSR_FZ))<<1; // set AH if DAZ different from FZ
    #if defined(__ANDROID__) || defined(__clang__)
    __asm__ __volatile__ ("msr    fpcr, %0"::"r"(fpcr));
    #else
    __builtin_aarch64_set_fpcr(fpcr);
    #endif
    #endif
}
