#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>

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
#ifdef DYNAREC
#include "custommem.h"
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

void PushExit(x64emu_t* emu)
{
    uintptr_t endMarker = AddCheckBridge(my_context->system, NULL, NULL, 0);
    Push(emu, endMarker);
}

void* GetExit()
{
    return (void*)AddCheckBridge(my_context->system, NULL, NULL, 0);
}

static void internalX64Setup(x64emu_t* emu, box64context_t *context, uintptr_t start, uintptr_t stack, int stacksize, int ownstack)
{
    emu->context = context;
    // setup cpu helpers
    for (int i=0; i<16; ++i)
        emu->sbiidx[i] = &emu->regs[i];
    emu->sbiidx[4] = &emu->zero;
    emu->sbiidx[12] = &emu->zero;
    emu->x64emu_parity_tab = x86emu_parity_tab;
    emu->eflags.x32 = 0x202; // default flags?
    // own stack?
    emu->stack2free = (ownstack)?(void*)stack:NULL;
    emu->init_stack = (void*)stack;
    emu->size_stack = stacksize;
    // set default value
    R_RIP = start;
    R_RSP = (stack + stacksize) & ~7;   // align stack start, always
    // fake init of segments...
    emu->segs[_CS] = 0x73;
    emu->segs[_DS] = emu->segs[_ES] = emu->segs[_SS] = 0x7b;
    emu->segs[_FS] = default_fs;
    emu->segs[_GS] = 0x33;
    // setup fpu regs
    reset_fpu(emu);
}

EXPORTDYN
x64emu_t *NewX64Emu(box64context_t *context, uintptr_t start, uintptr_t stack, int stacksize, int ownstack)
{
    printf_log(LOG_DEBUG, "Allocate a new X86_64 Emu, with EIP=%p and Stack=%p/0x%X\n", (void*)start, (void*)stack, stacksize);

    x64emu_t *emu = (x64emu_t*)calloc(1, sizeof(x64emu_t));

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
void SetupX64Emu(x64emu_t *emu)
{
    printf_log(LOG_DEBUG, "Setup X86_64 Emu\n");
}

void SetTraceEmu(uintptr_t start, uintptr_t end)
{
//    if(my_context->zydis) {
//        if (end == 0) {
//            printf_log(LOG_INFO, "Setting trace\n");
//        } else {
//            if(end!=1) {  // 0-1 is basically no trace, so don't printf it...
//                printf_log(LOG_INFO, "Setting trace only between %p and %p\n", (void*)start, (void*)end);
//            }
//        }
//    }
    trace_start = start;
    trace_end = end;
}

void AddCleanup(x64emu_t *emu, void *p)
{
    if(my_context->clean_sz == my_context->clean_cap) {
        my_context->clean_cap += 4;
        my_context->cleanups = (cleanup_t*)realloc(my_context->cleanups, sizeof(cleanup_t)*my_context->clean_cap);
    }
    my_context->cleanups[my_context->clean_sz].arg = 0;
    my_context->cleanups[my_context->clean_sz].a = NULL;
    my_context->cleanups[my_context->clean_sz++].f = p;
}

void AddCleanup1Arg(x64emu_t *emu, void *p, void* a)
{
    if(my_context->clean_sz == my_context->clean_cap) {
        my_context->clean_cap += 4;
        my_context->cleanups = (cleanup_t*)realloc(my_context->cleanups, sizeof(cleanup_t)*my_context->clean_cap);
    }
    my_context->cleanups[my_context->clean_sz].arg = 1;
    my_context->cleanups[my_context->clean_sz].a = a;
    my_context->cleanups[my_context->clean_sz++].f = p;
}

void CallCleanup(x64emu_t *emu, void* p)
{
    printf_log(LOG_DEBUG, "Calling atexit registered functions for %p mask\n", p);
    for(int i=my_context->clean_sz-1; i>=0; --i) {
        if(p==my_context->cleanups[i].f) {
            printf_log(LOG_DEBUG, "Call cleanup #%d\n", i);
            RunFunctionWithEmu(emu, 0, (uintptr_t)(my_context->cleanups[i].f), my_context->cleanups[i].arg, my_context->cleanups[i].a );
            // now remove the cleanup
            if(i!=my_context->clean_sz-1)
                memmove(my_context->cleanups+i, my_context->cleanups+i+1, (my_context->clean_sz-i-1)*sizeof(cleanup_t));
            --my_context->clean_sz;
        }
    }
}

void CallAllCleanup(x64emu_t *emu)
{
    printf_log(LOG_DEBUG, "Calling atexit registered functions\n");
    for(int i=my_context->clean_sz-1; i>=0; --i) {
        printf_log(LOG_DEBUG, "Call cleanup #%d\n", i);
        RunFunctionWithEmu(emu, 0, (uintptr_t)(my_context->cleanups[i].f), my_context->cleanups[i].arg, my_context->cleanups[i].a );
    }
    my_context->clean_sz = 0;
    free(my_context->cleanups);
    my_context->cleanups = NULL;
}

static void internalFreeX64(x64emu_t* emu)
{
    free(emu->stack2free);
}

EXPORTDYN
void FreeX64Emu(x64emu_t **emu)
{
    if(!emu)
        return;
    printf_log(LOG_DEBUG, "%04d|Free a X86_64 Emu (%p)\n", GetTID(), *emu);

    internalFreeX64(*emu);

    free(*emu);
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
	memcpy(newemu->fpu, emu->fpu, sizeof(emu->fpu));
    memcpy(newemu->fpu_ld, emu->fpu_ld, sizeof(emu->fpu_ld));
    memcpy(newemu->fpu_ll, emu->fpu_ll, sizeof(emu->fpu_ll));
	memcpy(newemu->p_regs, emu->p_regs, sizeof(emu->p_regs));
	newemu->cw = emu->cw;
    newemu->cw_mask_all = emu->cw_mask_all;
	memcpy(&newemu->sw, &emu->sw, sizeof(emu->sw));
	newemu->top = emu->top;
    newemu->fpu_stack = emu->fpu_stack;
	memcpy(&newemu->round, &emu->round, sizeof(emu->round));
    memcpy(newemu->mmx, emu->mmx, sizeof(emu->mmx));
    memcpy(newemu->xmm, emu->xmm, sizeof(emu->xmm));
    newemu->mxcsr = emu->mxcsr;
    newemu->quit = emu->quit;
    newemu->error = emu->error;
    // adapt R_RSP to new stack frame
    uintptr_t oldst = (uintptr_t)((emu->init_stack)?emu->init_stack:emu->context->stack);
    uintptr_t newst = (uintptr_t)((newemu->init_stack)?newemu->init_stack:newemu->context->stack);
    newemu->regs[_SP].q[0] = emu->regs[_SP].q[0] + (intptr_t)(newst - oldst);
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
void SetFS(x64emu_t *emu, uint16_t v)
{
    emu->segs[_FS] = v;
}
uint16_t GetFS(x64emu_t *emu)
{
    return emu->segs[_FS];
}


void ResetFlags(x64emu_t *emu)
{
    emu->df = d_none;
}

const char* DumpCPURegs(x64emu_t* emu, uintptr_t ip)
{
    static char buff[1000];
    char* regname[] = {"RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI", "RDI", 
                       " R8", " R9", "R10", "R11", "R12", "R13", "R14", "R15"};
    char tmp[160];
    buff[0] = '\0';
    if(trace_emm) {
        // do emm reg is needed
        for(int i=0; i<8; ++i) {
            sprintf(tmp, "mm%d:%016lx", i, emu->mmx[i].q);
            strcat(buff, tmp);
            if ((i&3)==3) strcat(buff, "\n"); else strcat(buff, " ");
        }
    }
    if(trace_xmm) {
        // do xmm reg is needed
        for(int i=0; i<8; ++i) {
            sprintf(tmp, "%d:%016lx%016lx", i, emu->xmm[i].q[1], emu->xmm[i].q[0]);
            strcat(buff, tmp);
            if ((i&3)==3) strcat(buff, "\n"); else strcat(buff, " ");
        }
    }
    // start with FPU regs...
    if(emu->fpu_stack) {
        for (int i=0; i<emu->fpu_stack; i++) {
            sprintf(tmp, "ST%d=%f", i, emu->fpu[(emu->top+i)&7].d);
            strcat(buff, tmp);
            int c = 10-strlen(tmp);
            if(c<1) c=1;
            while(c--) strcat(buff, " ");
            if(i==3) strcat(buff, "\n");
        }
        strcat(buff, "\n");
    }
    for (int i=_AX; i<=_R15; ++i) {
        sprintf(tmp, "%s=%016lx ", regname[i], emu->regs[i].q[0]);
        strcat(buff, tmp);

        if (i%5==4) {
            if(i==4) {
                if(emu->df) {
                    strcat(buff, "FLAGS=??????\n");
                } else {
#define FLAG_CHAR(f) (ACCESS_FLAG(F_##f##F)) ? #f : "-"
                    sprintf(tmp, "FLAGS=%s%s%s%s%s%s\n", FLAG_CHAR(O), FLAG_CHAR(C), FLAG_CHAR(P), FLAG_CHAR(A), FLAG_CHAR(Z), FLAG_CHAR(S));
                    strcat(buff, tmp);
#undef FLAG_CHAR
                }
            } else {
                strcat(buff, "\n");
            }
        } 
    }
    sprintf(tmp, "RIP=%016lx ", ip);
    strcat(buff, tmp);
    return buff;
}

void StopEmu(x64emu_t* emu, const char* reason)
{
    emu->quit = 1;
    printf_log(LOG_NONE, "%s", reason);
    // dump stuff...
    printf_log(LOG_NONE, "CPU Regs=%s\n", DumpCPURegs(emu, R_RIP));
    // TODO: stack, memory/instruction around EIP, etc..
}

void UnimpOpcode(x64emu_t* emu)
{
    R_RIP = emu->old_ip;

    int tid = syscall(SYS_gettid);
    printf_log(LOG_NONE, "%04d|%p: Unimplemented Opcode (%02X) %02X %02X %02X %02X %02X %02X %02X %02X\n", 
        tid, (void*)emu->old_ip, Peek(emu, -1),
        Peek(emu, 0), Peek(emu, 1), Peek(emu, 2), Peek(emu, 3),
        Peek(emu, 4), Peek(emu, 5), Peek(emu, 6), Peek(emu, 7));
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
    PushExit(emu);
    R_RIP = addr;
    emu->df = d_none;
    Run(emu, 0);
    emu->quit = 0;  // reset Quit flags...
    emu->df = d_none;
    if(emu->quitonlongjmp && emu->longjmp) {
        emu->longjmp = 0;   // don't change anything because of the longjmp
    } else {
        R_RBX = old_rbx;
        R_RDI = old_rdi;
        R_RSI = old_rsi;
        R_RBP = old_rbp;
        R_RSP = old_rsp;
        R_RIP = old_rip;  // and set back instruction pointer
    }
}

uint64_t ReadTSC(x64emu_t* emu)
{
    //TODO: implement hardware counter read?
    // Read the TimeStamp Counter as 64bits.
    // this is supposed to be the number of instrunctions executed since last reset
// fall back to gettime...
#ifndef NOGETCLOCK
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
  return (uint64_t)(ts.tv_sec) * 1000000000LL + ts.tv_nsec;
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (uint64_t)(tv.tv_sec) * 1000000 + tv.tv_usec;
#endif
}

void ResetSegmentsCache(x64emu_t *emu)
{
    if(!emu)
        return;
    memset(emu->segs_serial, 0, sizeof(emu->segs_serial));
}