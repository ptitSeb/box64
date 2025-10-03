#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

#include "os.h"
#include "debug.h"
#include "box64stack.h"
#include "x64emu.h"
#include "box64cpu.h"
#include "box64cpu_util.h"
#include "x64emu_private.h"
#include "x87emu_private.h"
#include "box64context.h"
#include "x64run_private.h"
#include "callback.h"
#include "bridge.h"
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
    #ifdef BOX32
    if(box64_is32bits) {
        if(stack>=0x100000000LL) {
            printf_log(LOG_NONE, "BOX32: Stack pointer too high (%p), aborting\n", (void*)stack);
            abort();
        }
        if(R_RSP>=0x100000000LL) {    // special case, stack is just a bit too high
            R_RSP = 0x100000000LL - 16;
        }
    }
    #endif
    // fake init of segments...
    if(box64_is32bits) {
        emu->segs[_CS] = 0x23;
        emu->segs[_DS] = emu->segs[_ES] = emu->segs[_SS] = 0x2b;
        emu->segs[_FS] = default_fs;
        emu->segs[_GS] = 0x33;
    } else {
        emu->segs[_CS] = 0x33;
        emu->segs[_DS] = emu->segs[_ES] = emu->segs[_SS] = 0x2b;
        emu->segs[_FS] = 0x43;
        emu->segs[_GS] = default_gs;
    }
    // setup fpu regs
    reset_fpu(emu);
    emu->mxcsr.x32 = 0x1f80;
}

EXPORTDYN
x64emu_t *NewX64Emu(box64context_t *context, uintptr_t start, uintptr_t stack, int stacksize, int ownstack)
{
    printf_log(LOG_DEBUG, "Allocate a new X86_64 Emu, with %cIP=%p and Stack=%p/0x%X%s\n", box64_is32bits?'E':'R', (void*)start, (void*)stack, stacksize, ownstack?" owned":"");

    x64emu_t *emu = (x64emu_t*)actual_calloc(1, sizeof(x64emu_t));

    internalX64Setup(emu, context, start, stack, stacksize, ownstack);

    return emu;
}

x64emu_t *NewX64EmuFromStack(x64emu_t* emu, box64context_t *context, uintptr_t start, uintptr_t stack, int stacksize)
{
    printf_log(LOG_DEBUG, "New X86_64 Emu from stack, with EIP=%p and Stack=%p/0x%X\n", (void*)start, (void*)stack, stacksize);

    internalX64Setup(emu, context, start, stack, stacksize, 0);

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

static void internalFreeX64(x64emu_t* emu)
{
    if(emu && emu->stack2free) {
        munmap(emu->stack2free, emu->size_stack);
        freeProtection((uintptr_t)emu->stack2free, emu->size_stack);
    }
    #ifdef BOX32
    if(emu->res_state_32)
        actual_free(emu->res_state_32);
    emu->res_state_32 = NULL;
    #endif
}

EXPORTDYN
void FreeX64Emu(x64emu_t **emu)
{
    if(!emu)
        return;
    printf_log(LOG_DEBUG, "%04d|Free a X86_64 Emu (%p)\n", GetTID(), *emu);

    if((*emu)->test.emu) {
        internalFreeX64((*emu)->test.emu);
        actual_free((*emu)->test.emu);
        (*emu)->test.emu = NULL;
    }
    internalFreeX64(*emu);

    actual_free(*emu);
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
    memcpy(newemu->ymm, emu->ymm, sizeof(emu->ymm));
    newemu->df = emu->df;
    newemu->op1 = emu->op1;
    newemu->op2 = emu->op2;
    newemu->res = emu->res;
    newemu->mxcsr = emu->mxcsr;
    newemu->quit = emu->quit;
    newemu->error = emu->error;
    newemu->x64emu_parity_tab = emu->x64emu_parity_tab;
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
    memcpy(newemu->ymm, emu->ymm, sizeof(emu->ymm));
    memcpy(newemu->fpu_ld, emu->fpu_ld, sizeof(emu->fpu_ld));
    memcpy(newemu->fpu_ll, emu->fpu_ll, sizeof(emu->fpu_ll));
    newemu->fpu_tags = emu->fpu_tags;
    newemu->cw = emu->cw;
    newemu->sw = emu->sw;
    newemu->top = emu->top;
    newemu->fpu_stack = emu->fpu_stack;
    newemu->df = emu->df;
    newemu->op1 = emu->op1;
    newemu->op2 = emu->op2;
    newemu->res = emu->res;
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
void SetESP(x64emu_t *emu, uint32_t v)
{
    R_ESP = v;
}
void SetEIP(x64emu_t *emu, uint32_t v)
{
    R_EIP = v;
}
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
void SetFS(x64emu_t *emu, uint16_t v)
{
    emu->segs[_FS] = v;
    emu->segs_serial[_FS] = 0;
}
uint16_t GetFS(x64emu_t *emu)
{
    return emu->segs[_FS];
}


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
    if(BOX64ENV(trace_emm)) {
        // do emm reg if needed
        for(int i=0; i<8; ++i) {
            sprintf(tmp, "mm%d:%016lx", i, emu->mmx[i].q);
            strcat(buff, tmp);
            if ((i&3)==3) strcat(buff, "\n"); else strcat(buff, " ");
        }
    }
    if(BOX64ENV(trace_xmm)) {
        // do xmm reg if needed
        for(int i=0; i<(is32bits?8:16); ++i) {
            if (BOX64ENV(trace_regsdiff) && (emu->old_xmm[i].q[0] != emu->xmm[i].q[0] || emu->old_xmm[i].q[1] != emu->xmm[i].q[1])) {
                sprintf(tmp, "\e[1;35m%02d:%016lx-%016lx\e[m", i, emu->xmm[i].q[1], emu->xmm[i].q[0]);
                emu->old_xmm[i].q[0] = emu->xmm[i].q[0];
                emu->old_xmm[i].q[1] = emu->xmm[i].q[1];
            } else {
                sprintf(tmp, "%02d:%016lx-%016lx", i, emu->xmm[i].q[1], emu->xmm[i].q[0]);
            }
            strcat(buff, tmp);
            if(BOX64ENV(avx)) {
                if (BOX64ENV(trace_regsdiff) && (emu->old_ymm[i].q[0] != emu->ymm[i].q[0] || emu->old_ymm[i].q[1] != emu->ymm[i].q[1])) {
                    sprintf(tmp, "\e[1;35m-%016lx-%016lx\e[m", emu->ymm[i].q[1], emu->ymm[i].q[0]);
                    emu->old_ymm[i].q[0] = emu->ymm[i].q[0];
                    emu->old_ymm[i].q[1] = emu->ymm[i].q[1];
                } else {
                    sprintf(tmp, "-%016lx-%016lx", emu->ymm[i].q[1], emu->ymm[i].q[0]);
                }
                strcat(buff, tmp);
            }
            if(BOX64ENV(avx))
                if ((i&1)==1) strcat(buff, "\n"); else strcat(buff, " ");
            else
                if ((i&3)==3) strcat(buff, "\n"); else strcat(buff, " ");
        }
    }
#endif
    // start with FPU regs...
    if(emu->fpu_stack) {
        int stack = emu->fpu_stack;
        if(stack>8) stack = 8;
        for (int i=0; i<stack; i++) {
            sprintf(tmp, "ST%d=%f(0x%" PRIx64 ")", i, ST(i).d, ST(i).q);
            strcat(buff, tmp);
            int c = 20-strlen(tmp);
            if(c<1) c=1;
            while(c--) strcat(buff, " ");
            if(i==3) strcat(buff, "\n");
        }
        sprintf(tmp, " C3210 = %d%d%d%d CW=%x", emu->sw.f.F87_C3, emu->sw.f.F87_C2, emu->sw.f.F87_C1, emu->sw.f.F87_C0, emu->cw.x16);
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
            if (BOX64ENV(trace_regsdiff) && (emu->regs[i].dword[0] != emu->oldregs[i].q[0])) {
                sprintf(tmp, "\e[1;35m%s=%08x\e[m ", regname32[i], emu->regs[i].dword[0]);
                emu->oldregs[i].q[0] = emu->regs[i].dword[0];
            } else {
                sprintf(tmp, "%s%s=%08x ", emu->regs[i].dword[1]?"*":"", regname32[i], emu->regs[i].dword[0]);
            }
#else
            sprintf(tmp, "%s%s=%08x ", emu->regs[i].dword[1]?"*":"", regname[i], emu->regs[i].dword[0]);
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
            } else {
                strcat(buff, " ");
            }
        }
    else
        for (int i=_AX; i<=_R15; ++i) {
#ifdef HAVE_TRACE
            if (BOX64ENV(trace_regsdiff) && (emu->regs[i].q[0] != emu->oldregs[i].q[0])) {
                sprintf(tmp, "\e[1;35m%s=%016" PRIx64 "\e[m ", regname[i], emu->regs[i].q[0]);
                emu->oldregs[i].q[0] = emu->regs[i].q[0];
            } else {
                sprintf(tmp, "%s=%016" PRIx64, regname[i], emu->regs[i].q[0]);
            }
#else
            sprintf(tmp, "%s=%016" PRIx64, regname[i], emu->regs[i].q[0]);
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
            } else {
                strcat(buff, " ");
            }
    }
    if(is32bits)
        sprintf(tmp, "EIP=%08" PRIx64 " ", ip);
    else
        sprintf(tmp, "RIP=%016" PRIx64 " ", ip);
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
    if(emu->segs[_CS]==0x23) {
        if (R_EBP == R_ESP) {
            printf_log(LOG_NONE, "EBP = ESP: leaf function detected; next 128 bytes should be either data or random.\n");
        } else {
            // TODO: display stack if operation should be allowed (to avoid crashes)
            /* for (uint64_t *sp = R_RBP; sp >= R_RSP; --sp) {
            } */
        }
    } else {
        if (R_RBP == R_RSP) {
            printf_log(LOG_NONE, "RBP = RSP: leaf function detected; next 128 bytes should be either data or random.\n");
        } else {
            // TODO: display stack if operation should be allowed (to avoid crashes)
            /* for (uint64_t *sp = R_RBP; sp >= R_RSP; --sp) {
            } */
        }
    }
    printf_log(LOG_NONE, "Old IP: %tX\n", emu->old_ip);
#ifdef HAVE_TRACE
    if(box64_is32bits) {
        if(my_context->dec32)
            printf_log(LOG_NONE, "%s\n", DecodeX64Trace(my_context->dec32, emu->old_ip, 1));
    } else {
        if(my_context->dec)
            printf_log(LOG_NONE, "%s\n", DecodeX64Trace(my_context->dec, emu->old_ip, 1));
    }
#endif
}

void UnimpOpcode(x64emu_t* emu, int is32bits)
{
    int tid = GetTID();
    printf_log(LOG_INFO, "%04d|%p: Unimplemented %sOpcode (%02X %02X %02X %02X) %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
        tid, (void*)emu->old_ip, is32bits?"32bits ":"",
        Peek(emu, -4), Peek(emu, -3), Peek(emu, -2), Peek(emu, -1),
        Peek(emu, 0), Peek(emu, 1), Peek(emu, 2), Peek(emu, 3),
        Peek(emu, 4), Peek(emu, 5), Peek(emu, 6), Peek(emu, 7),
        Peek(emu, 8), Peek(emu, 9), Peek(emu,10), Peek(emu,11),
        Peek(emu,12), Peek(emu,13), Peek(emu,14));
}

void EmuCall(x64emu_t* emu, uintptr_t addr)
{
    uint64_t old_rsp = R_RSP;
    uint64_t old_rbx = R_RBX;
    uint64_t old_rdi = R_RDI;
    uint64_t old_rsi = R_RSI;
    uint64_t old_rbp = R_RBP;
    uint64_t old_rip = R_RIP;
    x64flags_t old_eflags = emu->eflags;
    // save defered flags
    deferred_flags_t old_df = emu->df;
    multiuint_t old_op1 = emu->op1;
    multiuint_t old_op2 = emu->op2;
    multiuint_t old_res = emu->res;
    //Push64(emu, GetRBP(emu));   // set frame pointer
    //SetRBP(emu, GetRSP(emu));   // save RSP
    //R_RSP -= 200;
    //R_RSP &= ~63LL;
    #ifdef BOX32
    if(box64_is32bits)
        PushExit_32(emu);
    else
    #endif
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
        // restore defered flags
        emu->df = old_df;
        emu->op1 = old_op1;
        emu->op2 = old_op2;
        emu->res = old_res;
        // and the old registers
        emu->eflags = old_eflags;
        R_RBX = old_rbx;
        R_RDI = old_rdi;
        R_RSI = old_rsi;
        R_RBP = old_rbp;
        R_RSP = old_rsp;
        R_RIP = old_rip;  // and set back instruction pointer
    }
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
    #else
    // This does not applies to RISC-V and LoongArch, as they don't have flush to zero
    #endif
}

#define PARITY(x) (((emu->x64emu_parity_tab[(x) / 32] >> ((x) % 32)) & 1) == 0)
#define XOR2(x)   (((x) ^ ((x) >> 1)) & 0x1)
void UpdateFlags(x64emu_t* emu)
{
    uint64_t cc;
    uint64_t lo, hi;
    uint64_t bc;
    uint64_t cnt;

    switch (emu->df) {
        case d_none:
            return;
        case d_add8:
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x100, F_CF);
            CONDITIONAL_SET_FLAG(!emu->res.u8, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            cc = (emu->op1.u8 & emu->op2.u8) | ((~emu->res.u8) & (emu->op1.u8 | emu->op2.u8));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_add8b:
            CONDITIONAL_SET_FLAG(((uint16_t)emu->op1.u8 + emu->op2.u8) & 0x100, F_CF);
            CONDITIONAL_SET_FLAG(!emu->res.u8, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            cc = (emu->op1.u8 & emu->op2.u8) | ((~emu->res.u8) & (emu->op1.u8 | emu->op2.u8));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_add16:
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x10000, F_CF);
            CONDITIONAL_SET_FLAG(!emu->res.u16, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            cc = (emu->op1.u16 & emu->op2.u16) | ((~emu->res.u16) & (emu->op1.u16 | emu->op2.u16));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_add16b:
            CONDITIONAL_SET_FLAG(((uint32_t)emu->op1.u16 + emu->op2.u16) & 0x10000, F_CF);
            CONDITIONAL_SET_FLAG(!emu->res.u16, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            cc = (emu->op1.u16 & emu->op2.u16) | ((~emu->res.u16) & (emu->op1.u16 | emu->op2.u16));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_add32:
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x100000000LL, F_CF);
            CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            cc = (emu->op1.u32 & emu->op2.u32) | ((~emu->res.u32) & (emu->op1.u32 | emu->op2.u32));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_add32b:
            lo = (emu->op2.u32 & 0xFFFF) + (emu->op1.u32 & 0xFFFF);
            hi = (lo >> 16) + (emu->op2.u32 >> 16) + (emu->op1.u32 >> 16);
            CONDITIONAL_SET_FLAG(hi & 0x10000, F_CF);
            CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            cc = (emu->op1.u32 & emu->op2.u32) | ((~emu->res.u32) & (emu->op1.u32 | emu->op2.u32));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_add64:
            lo = (emu->op2.u64 & 0xFFFFFFFF) + (emu->op1.u64 & 0xFFFFFFFF);
            hi = (lo >> 32) + (emu->op2.u64 >> 32) + (emu->op1.u64 >> 32);
            CONDITIONAL_SET_FLAG(hi & 0x100000000L, F_CF);
            CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            cc = (emu->op1.u64 & emu->op2.u64) | ((~emu->res.u64) & (emu->op1.u64 | emu->op2.u64));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 62), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_and8:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u8, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            break;
        case d_and16:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u16, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            break;
        case d_and32:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            break;
        case d_and64:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
            CONDITIONAL_SET_FLAG(emu->res.u64 == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            break;
        case d_dec8:
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u8, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            bc = (emu->res.u8 & (~emu->op1.u8 | 1)) | (~emu->op1.u8 & 1);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_dec16:
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u16, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            bc = (emu->res.u16 & (~emu->op1.u16 | 1)) | (~emu->op1.u16 & 1);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_dec32:
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            bc = (emu->res.u32 & (~emu->op1.u32 | 1)) | (~emu->op1.u32 & 1);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_dec64:
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            bc = (emu->res.u64 & (~emu->op1.u64 | 1LL)) | (~emu->op1.u64 & 1LL);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 62), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_inc8:
            CONDITIONAL_SET_FLAG(!emu->res.u8, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            cc = ((1 & emu->op1.u8) | (~emu->res.u8)) & (1 | emu->op1.u8);
            CONDITIONAL_SET_FLAG(XOR2(cc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_inc16:
            CONDITIONAL_SET_FLAG(!emu->res.u16, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            cc = (1 & emu->op1.u16) | ((~emu->res.u16) & (1 | emu->op1.u16));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_inc32:
            CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            cc = (1 & emu->op1.u32) | ((~emu->res.u32) & (1 | emu->op1.u32));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_inc64:
            CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            cc = (1LL & emu->op1.u64) | ((~emu->res.u64) & (1LL | emu->op1.u64));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 62), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_imul8:
            lo = emu->res.u16 & 0xff;
            hi = (emu->res.u16 >> 8) & 0xff;
            if (((lo & 0x80) == 0 && hi == 0x00) || ((lo & 0x80) != 0 && hi == 0xFF)) {
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
            } else {
                SET_FLAG(F_CF);
                SET_FLAG(F_OF);
            }
            if (!BOX64ENV(cputype)) {
                CONDITIONAL_SET_FLAG((emu->res.u8 >> 7) & 1, F_SF);
                CLEAR_FLAG(F_ZF);
                CLEAR_FLAG(F_AF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            }
            break;
        case d_imul16:
            lo = (uint16_t)emu->res.u32;
            hi = (uint16_t)(emu->res.u32 >> 16);
            if (((lo & 0x8000) == 0 && hi == 0x00) || ((lo & 0x8000) != 0 && hi == 0xFFFF)) {
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
            } else {
                SET_FLAG(F_CF);
                SET_FLAG(F_OF);
            }
            if (!BOX64ENV(cputype)) {
                CONDITIONAL_SET_FLAG((emu->res.u16 >> 15) & 1, F_SF);
                CLEAR_FLAG(F_ZF);
                CLEAR_FLAG(F_AF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            }
            break;
        case d_imul32:
            if ((((emu->res.u32 & 0x80000000) == 0) && emu->op1.u32 == 0x00) || (((emu->res.u32 & 0x80000000) != 0) && emu->op1.u32 == 0xFFFFFFFF)) {
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
            } else {
                SET_FLAG(F_CF);
                SET_FLAG(F_OF);
            }
            if (!BOX64ENV(cputype)) {
                CONDITIONAL_SET_FLAG((emu->res.u32 >> 31) & 1, F_SF);
                CLEAR_FLAG(F_ZF);
                CLEAR_FLAG(F_AF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            }
            break;
        case d_imul64:
            if (((emu->res.u64 & 0x8000000000000000LL) == 0 && emu->op1.u64 == 0x00) || ((emu->res.u64 & 0x8000000000000000LL) != 0 && emu->op1.u64 == 0xFFFFFFFFFFFFFFFFLL)) {
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
            } else {
                SET_FLAG(F_CF);
                SET_FLAG(F_OF);
            }
            if (!BOX64ENV(cputype)) {
                CONDITIONAL_SET_FLAG((emu->res.u64 >> 63) & 1, F_SF);
                CLEAR_FLAG(F_ZF);
                CLEAR_FLAG(F_AF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            }
            break;
        case d_mul8:
            hi = (emu->res.u16 >> 8) & 0xff;
            if (hi == 0) {
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
            } else {
                SET_FLAG(F_CF);
                SET_FLAG(F_OF);
            }
            if (!BOX64ENV(cputype)) {
                CONDITIONAL_SET_FLAG((emu->res.u8 >> 7) & 1, F_SF);
                CLEAR_FLAG(F_ZF);
                CLEAR_FLAG(F_AF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            }
            break;
        case d_mul16:
            hi = (uint16_t)(emu->res.u32 >> 16);
            if (hi == 0) {
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
            } else {
                SET_FLAG(F_CF);
                SET_FLAG(F_OF);
            }
            if (!BOX64ENV(cputype)) {
                CONDITIONAL_SET_FLAG((emu->res.u16 >> 15) & 1, F_SF);
                CLEAR_FLAG(F_ZF);
                CLEAR_FLAG(F_AF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            }
            break;
        case d_mul32:
            if (emu->op1.u32 == 0) {
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
            } else {
                SET_FLAG(F_CF);
                SET_FLAG(F_OF);
            }
            if (!BOX64ENV(cputype)) {
                CONDITIONAL_SET_FLAG((emu->res.u32 >> 31) & 1, F_SF);
                CLEAR_FLAG(F_ZF);
                CLEAR_FLAG(F_AF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            }
            break;
        case d_mul64:
            if (emu->op1.u64 == 0) {
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
            } else {
                SET_FLAG(F_CF);
                SET_FLAG(F_OF);
            }
            if (!BOX64ENV(cputype)) {
                CONDITIONAL_SET_FLAG((emu->res.u64 >> 63) & 1, F_SF);
                CLEAR_FLAG(F_ZF);
                CLEAR_FLAG(F_AF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            }
            break;
        case d_or8:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u8, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            break;
        case d_or16:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u16, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            break;
        case d_or32:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            break;
        case d_or64:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            break;
        case d_neg8:
            CONDITIONAL_SET_FLAG(emu->op1.u8, F_CF);
            CONDITIONAL_SET_FLAG(!emu->res.u8, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            bc = emu->res.u8 | emu->op1.u8;
            CONDITIONAL_SET_FLAG(XOR2(bc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_neg16:
            CONDITIONAL_SET_FLAG(emu->op1.u16, F_CF);
            CONDITIONAL_SET_FLAG(!emu->res.u16, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            bc = emu->res.u16 | emu->op1.u16;
            CONDITIONAL_SET_FLAG(XOR2(bc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_neg32:
            CONDITIONAL_SET_FLAG(emu->op1.u32, F_CF);
            CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            bc = emu->res.u32 | emu->op1.u32;
            CONDITIONAL_SET_FLAG(XOR2(bc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_neg64:
            CONDITIONAL_SET_FLAG(emu->op1.u64, F_CF);
            CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            bc = emu->res.u64 | emu->op1.u64;
            CONDITIONAL_SET_FLAG(XOR2(bc >> 62), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_shl8:
            cnt = emu->op2.u8 & 0x1f;
            if (cnt > 0) {
                cc = emu->op1.u8 & (1 << (8 - cnt));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(!emu->res.u8, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
                if (BOX64ENV(cputype)) {
                    CONDITIONAL_SET_FLAG(((emu->res.u8 >> 7) ^ ACCESS_FLAG(F_CF)) & 0x01, F_OF);
                    SET_FLAG(F_AF);
                } else {
                    CONDITIONAL_SET_FLAG(XOR2(emu->op1.u8 >> 6), F_OF);
                    CLEAR_FLAG(F_AF);
                }
            }
            break;
        case d_shl16:
            cnt = emu->op2.u16 & 0x1f;
            if (cnt > 0) {
                cc = emu->op1.u16 & (1 << (16 - cnt));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(!emu->res.u16, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
                if (BOX64ENV(cputype)) {
                    CONDITIONAL_SET_FLAG(((emu->res.u16 >> 15) ^ ACCESS_FLAG(F_CF)) & 0x01, F_OF);
                    SET_FLAG(F_AF);
                } else {
                    CONDITIONAL_SET_FLAG(XOR2(emu->op1.u16 >> 14), F_OF);
                    CLEAR_FLAG(F_AF);
                }
            }
            break;
        case d_shl32:
            cnt = emu->op2.u32 & 0x1f;
            if (cnt > 0) {
                cc = emu->op1.u32 & (1 << (32 - cnt));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
                if (BOX64ENV(cputype)) {
                    CONDITIONAL_SET_FLAG(((emu->res.u32 >> 31) ^ ACCESS_FLAG(F_CF)) & 0x01, F_OF);
                    SET_FLAG(F_AF);
                } else {
                    CONDITIONAL_SET_FLAG(XOR2(emu->op1.u32 >> 30), F_OF);
                    CLEAR_FLAG(F_AF);
                }
            }
            break;
        case d_shl64:
            if (emu->op2.u64 > 0) {
                cc = emu->op1.u64 & (1LL << (64 - emu->op2.u64));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
                if (BOX64ENV(cputype)) {
                    CONDITIONAL_SET_FLAG(((emu->res.u64 >> 63) ^ ACCESS_FLAG(F_CF)) & 0x01, F_OF);
                    SET_FLAG(F_AF);
                } else {
                    CONDITIONAL_SET_FLAG(XOR2(emu->op1.u64 >> 62), F_OF);
                    CLEAR_FLAG(F_AF);
                }
            }
            break;
        case d_sar8:
            if (emu->op2.u8) {
                cc = (emu->op1.i8 >> (emu->op2.u8 - 1)) & 1;
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(!emu->res.u8, F_ZF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
                CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
                CLEAR_FLAG(F_OF);
                if (BOX64ENV(cputype))
                    SET_FLAG(F_AF);
                else
                    CLEAR_FLAG(F_AF);
            }
            break;
        case d_sar16:
            if (emu->op2.u16) {
                cc = (emu->op1.i16 >> (emu->op2.u16 - 1)) & 1;
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(!emu->res.u16, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
                CLEAR_FLAG(F_OF);
                if (BOX64ENV(cputype))
                    SET_FLAG(F_AF);
                else
                    CLEAR_FLAG(F_AF);
            }
            break;
        case d_sar32:
            if (emu->op2.u32) {
                cc = emu->op1.u32 & (1 << (emu->op2.u32 - 1));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
                CLEAR_FLAG(F_OF);
                if (BOX64ENV(cputype))
                    SET_FLAG(F_AF);
                else
                    CLEAR_FLAG(F_AF);
            }
            break;
        case d_sar64:
            if (emu->op2.u64) {
                cc = emu->op1.u64 & (1LL << (emu->op2.u64 - 1));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
                CLEAR_FLAG(F_OF);
                if (BOX64ENV(cputype))
                    SET_FLAG(F_AF);
                else
                    CLEAR_FLAG(F_AF);
            }
            break;
        case d_shr8:
            cnt = emu->op2.u8 & 0x1f;
            if (cnt > 0) {
                cc = emu->op1.u8 & (1 << (cnt - 1));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(!emu->res.u8, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
                if (BOX64ENV(cputype)) {
                    CONDITIONAL_SET_FLAG((emu->res.u8 >> 6) & 0x1, F_OF);
                    SET_FLAG(F_AF);
                } else {
                    CONDITIONAL_SET_FLAG((emu->op1.u8 >> 7) & 0x1, F_OF);
                    CLEAR_FLAG(F_AF);
                }
            }
            break;
        case d_shr16:
            cnt = emu->op2.u16 & 0x1f;
            if (cnt > 0) {
                cc = emu->op1.u16 & (1 << (cnt - 1));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(!emu->res.u16, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
                if (BOX64ENV(cputype)) {
                    CONDITIONAL_SET_FLAG((emu->res.u16 >> 14) & 0x1, F_OF);
                    SET_FLAG(F_AF);
                } else {
                    CONDITIONAL_SET_FLAG((emu->op1.u16 >> 15) & 0x1, F_OF);
                    CLEAR_FLAG(F_AF);
                }
            }
            if (cnt == 1) {
                CONDITIONAL_SET_FLAG(emu->op1.u16 & 0x8000, F_OF);
            }
            break;
        case d_shr32:
            cnt = emu->op2.u32 & 0x1f;
            if (cnt > 0) {
                cc = emu->op1.u32 & (1 << (cnt - 1));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
                if (BOX64ENV(cputype)) {
                    CONDITIONAL_SET_FLAG((emu->res.u32 >> 30) & 0x1, F_OF);
                    SET_FLAG(F_AF);
                } else {
                    CONDITIONAL_SET_FLAG((emu->op1.u32 >> 31) & 0x1, F_OF);
                    CLEAR_FLAG(F_AF);
                }
            }
            break;
        case d_shr64:
            cnt = emu->op2.u64;
            if (cnt > 0) {
                cc = emu->op1.u64 & (1LL << (cnt - 1));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
                if (BOX64ENV(cputype)) {
                    CONDITIONAL_SET_FLAG((emu->res.u64 >> 62) & 0x1, F_OF);
                    SET_FLAG(F_AF);
                } else {
                    CONDITIONAL_SET_FLAG((emu->op1.u64 >> 63) & 0x1, F_OF);
                    CLEAR_FLAG(F_AF);
                }
            }
            break;
        case d_shrd16:
            cnt = emu->op2.u16;
            if (cnt > 0) {
                cc = emu->op1.u16 & (1 << (cnt - 1));
                if (cnt > 15 && BOX64ENV(cputype))
                    cc = 0;
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(!emu->res.u16, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            }
            if BOX64ENV (cputype) {
                CONDITIONAL_SET_FLAG(XOR2(emu->res.u16 >> 14), F_OF);
            } else {
                CONDITIONAL_SET_FLAG(((emu->res.u16 >> (16 - (cnt & 15))) ^ (emu->op1.u16 >> 15)) & 1, F_OF);
            }
            if (BOX64ENV(cputype))
                SET_FLAG(F_AF);
            else
                CLEAR_FLAG(F_AF);
            break;
        case d_shrd32:
            cnt = emu->op2.u32;
            if (cnt > 0) {
                cc = emu->op1.u32 & (1 << (cnt - 1));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
                if BOX64ENV (cputype) {
                    CONDITIONAL_SET_FLAG(XOR2(emu->res.u32 >> 30), F_OF);
                } else {
                    CONDITIONAL_SET_FLAG(((emu->res.u32 >> (32 - cnt)) ^ (emu->op1.u32 >> 31)) & 1, F_OF);
                }
                if (BOX64ENV(cputype))
                    SET_FLAG(F_AF);
                else
                    CLEAR_FLAG(F_AF);
            }
            break;
        case d_shrd64:
            cnt = emu->op2.u64;
            if (cnt > 0) {
                cc = emu->op1.u64 & (1LL << (cnt - 1));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
                if BOX64ENV (cputype) {
                    CONDITIONAL_SET_FLAG(XOR2(emu->res.u64 >> 62), F_OF);
                } else {
                    CONDITIONAL_SET_FLAG(((emu->res.u64 >> (64 - cnt)) ^ (emu->op1.u64 >> 63)) & 1, F_OF);
                }
                if (BOX64ENV(cputype))
                    SET_FLAG(F_AF);
                else
                    CLEAR_FLAG(F_AF);
            }
            break;
        case d_shld16:
            cnt = emu->op2.u16;
            if (cnt > 0) {
                cc = emu->op1.u16 & (1 << (16 - cnt));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(!emu->res.u16, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
                if BOX64ENV (cputype) {
                    if (cnt > 15)
                        CONDITIONAL_SET_FLAG(ACCESS_FLAG(F_CF), F_OF);
                    else
                        CONDITIONAL_SET_FLAG((ACCESS_FLAG(F_CF) ^ (emu->res.u16 >> 15)) & 1, F_OF);
                } else {
                    CONDITIONAL_SET_FLAG(XOR2(emu->op1.u16 >> 14), F_OF);
                }
                if (BOX64ENV(cputype))
                    SET_FLAG(F_AF);
                else
                    CLEAR_FLAG(F_AF);
            }
            break;
        case d_shld32:
            cnt = emu->op2.u32;
            if (cnt > 0) {
                cc = emu->op1.u32 & (1 << (32 - cnt));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
                if BOX64ENV (cputype) {
                    CONDITIONAL_SET_FLAG((ACCESS_FLAG(F_CF) ^ (emu->res.u32 >> 31)) & 1, F_OF);
                } else {
                    CONDITIONAL_SET_FLAG(XOR2(emu->op1.u32 >> 30), F_OF);
                }
                if (BOX64ENV(cputype))
                    SET_FLAG(F_AF);
                else
                    CLEAR_FLAG(F_AF);
            }
            break;
        case d_shld64:
            cnt = emu->op2.u64;
            if (cnt > 0) {
                cc = emu->op1.u64 & (1LL << (64 - cnt));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
                if BOX64ENV (cputype) {
                    CONDITIONAL_SET_FLAG((ACCESS_FLAG(F_CF) ^ (emu->res.u64 >> 63)) & 1, F_OF);
                } else {
                    CONDITIONAL_SET_FLAG(XOR2(emu->op1.u64 >> 62), F_OF);
                }
                if (BOX64ENV(cputype))
                    SET_FLAG(F_AF);
                else
                    CLEAR_FLAG(F_AF);
            }
            break;
        case d_sub8:
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u8, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            bc = (emu->res.u8 & (~emu->op1.u8 | emu->op2.u8)) | (~emu->op1.u8 & emu->op2.u8);
            CONDITIONAL_SET_FLAG(bc & 0x80, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_sub16:
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u16, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            bc = (emu->res.u16 & (~emu->op1.u16 | emu->op2.u16)) | (~emu->op1.u16 & emu->op2.u16);
            CONDITIONAL_SET_FLAG(bc & 0x8000, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_sub32:
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            bc = (emu->res.u32 & (~emu->op1.u32 | emu->op2.u32)) | (~emu->op1.u32 & emu->op2.u32);
            CONDITIONAL_SET_FLAG(bc & 0x80000000, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_sub64:
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            bc = (emu->res.u64 & (~emu->op1.u64 | emu->op2.u64)) | (~emu->op1.u64 & emu->op2.u64);
            CONDITIONAL_SET_FLAG(bc & 0x8000000000000000LL, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 62), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_xor8:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u8, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            break;
        case d_xor16:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u16, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            break;
        case d_xor32:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            break;
        case d_xor64:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            break;
        case d_cmp8:
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG((emu->res.u8 & 0xff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            bc = (emu->res.u8 & (~emu->op1.u8 | emu->op2.u8)) | (~emu->op1.u8 & emu->op2.u8);
            CONDITIONAL_SET_FLAG(bc & 0x80, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_cmp16:
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG((emu->res.u16 & 0xffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            bc = (emu->res.u16 & (~emu->op1.u16 | emu->op2.u16)) | (~emu->op1.u16 & emu->op2.u16);
            CONDITIONAL_SET_FLAG(bc & 0x8000, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_cmp32:
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            bc = (emu->res.u32 & (~emu->op1.u32 | emu->op2.u32)) | (~emu->op1.u32 & emu->op2.u32);
            CONDITIONAL_SET_FLAG(bc & 0x80000000, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_cmp64:
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            bc = (emu->res.u64 & (~emu->op1.u64 | emu->op2.u64)) | (~emu->op1.u64 & emu->op2.u64);
            CONDITIONAL_SET_FLAG(bc & 0x8000000000000000LL, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 62), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_tst8:
            CLEAR_FLAG(F_OF);
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u8, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            CLEAR_FLAG(F_CF);
            break;
        case d_tst16:
            CLEAR_FLAG(F_OF);
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u16, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            CLEAR_FLAG(F_CF);
            break;
        case d_tst32:
            CLEAR_FLAG(F_OF);
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            CLEAR_FLAG(F_CF);
            break;
        case d_tst64:
            CLEAR_FLAG(F_OF);
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            CLEAR_FLAG(F_CF);
            break;
        case d_adc8:
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x100, F_CF);
            CONDITIONAL_SET_FLAG(!emu->res.u8, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            cc = (emu->op1.u8 & emu->op2.u8) | ((~emu->res.u8) & (emu->op1.u8 | emu->op2.u8));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_adc16:
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x10000, F_CF);
            CONDITIONAL_SET_FLAG((emu->res.u16 & 0xffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            cc = (emu->op1.u16 & emu->op2.u16) | ((~emu->res.u16) & (emu->op1.u16 | emu->op2.u16));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_adc32:
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x100000000L, F_CF);
            CONDITIONAL_SET_FLAG((emu->res.u32 & 0xffffffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            cc = (emu->op1.u32 & emu->op2.u32) | ((~emu->res.u32) & (emu->op1.u32 | emu->op2.u32));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_adc32b:
            if (emu->res.u32 == (emu->op1.u32 + emu->op2.u32)) {
                lo = (emu->op1.u32 & 0xFFFF) + (emu->op2.u32 & 0xFFFF);
            } else {
                lo = 1 + (emu->op1.u32 & 0xFFFF) + (emu->op2.u32 & 0xFFFF);
            }
            hi = (lo >> 16) + (emu->op1.u32 >> 16) + (emu->op2.u32 >> 16);
            CONDITIONAL_SET_FLAG(hi & 0x10000, F_CF);
            CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            cc = (emu->op2.u32 & emu->op1.u32) | ((~emu->res.u32) & (emu->op2.u32 | emu->op1.u32));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_adc64:
            if (emu->res.u64 == (emu->op1.u64 + emu->op2.u64)) {
                lo = (emu->op1.u64 & 0xFFFFFFFF) + (emu->op2.u64 & 0xFFFFFFFF);
            } else {
                lo = 1 + (emu->op1.u64 & 0xFFFFFFFF) + (emu->op2.u64 & 0xFFFFFFFF);
            }
            hi = (lo >> 32) + (emu->op1.u64 >> 32) + (emu->op2.u64 >> 32);
            CONDITIONAL_SET_FLAG(hi & 0x100000000L, F_CF);
            CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            cc = (emu->op2.u64 & emu->op1.u64) | ((~emu->res.u64) & (emu->op2.u64 | emu->op1.u64));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 62), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_sbb8:
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG((emu->res.u8 & 0xff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            bc = (emu->res.u8 & (~emu->op1.u8 | emu->op2.u8)) | (~emu->op1.u8 & emu->op2.u8);
            CONDITIONAL_SET_FLAG(bc & 0x80, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_sbb16:
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG((emu->res.u16 & 0xffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            bc = (emu->res.u16 & (~emu->op1.u16 | emu->op2.u16)) | (~emu->op1.u16 & emu->op2.u16);
            CONDITIONAL_SET_FLAG(bc & 0x8000, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_sbb32:
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            bc = (emu->res.u32 & (~emu->op1.u32 | emu->op2.u32)) | (~emu->op1.u32 & emu->op2.u32);
            CONDITIONAL_SET_FLAG(bc & 0x80000000, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_sbb64:
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8), F_PF);
            bc = (emu->res.u64 & (~emu->op1.u64 | emu->op2.u64)) | (~emu->op1.u64 & emu->op2.u64);
            CONDITIONAL_SET_FLAG(bc & 0x8000000000000000LL, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 62), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_rol8:
            if (BOX64ENV(cputype))
                CONDITIONAL_SET_FLAG((emu->res.u8 + (emu->res.u8 >> 7)) & 1, F_OF);
            else
                CONDITIONAL_SET_FLAG(XOR2(emu->op1.u8 >> 6), F_OF);
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x1, F_CF);
            break;
        case d_rol16:
            if (BOX64ENV(cputype))
                CONDITIONAL_SET_FLAG((emu->res.u16 + (emu->res.u16 >> 15)) & 1, F_OF);
            else
                CONDITIONAL_SET_FLAG(XOR2(emu->op1.u16 >> 14), F_OF);
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x1, F_CF);
            break;
        case d_rol32:
            if (BOX64ENV(cputype))
                CONDITIONAL_SET_FLAG((emu->res.u32 + (emu->res.u32 >> 31)) & 1, F_OF);
            else
                CONDITIONAL_SET_FLAG(XOR2(emu->op1.u32 >> 30), F_OF);
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x1, F_CF);
            break;
        case d_rol64:
            if (BOX64ENV(cputype))
                CONDITIONAL_SET_FLAG((emu->res.u64 + (emu->res.u64 >> 63)) & 1, F_OF);
            else
                CONDITIONAL_SET_FLAG(XOR2(emu->op1.u64 >> 62), F_OF);
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x1, F_CF);
            break;
        case d_ror8:
            if (BOX64ENV(cputype))
                CONDITIONAL_SET_FLAG(XOR2(emu->res.u8 >> 6), F_OF);
            else
                CONDITIONAL_SET_FLAG(((emu->op1.u8 >> 7) ^ emu->op1.u8) & 1, F_OF);
            CONDITIONAL_SET_FLAG(emu->res.u8 & (1 << 7), F_CF);
            break;
        case d_ror16:
            if (BOX64ENV(cputype))
                CONDITIONAL_SET_FLAG(XOR2(emu->res.u16 >> 14), F_OF);
            else
                CONDITIONAL_SET_FLAG(((emu->op1.u16 >> 15) ^ emu->op1.u16) & 1, F_OF);
            CONDITIONAL_SET_FLAG(emu->res.u16 & (1 << 15), F_CF);
            break;
        case d_ror32:
            if (BOX64ENV(cputype))
                CONDITIONAL_SET_FLAG(XOR2(emu->res.u32 >> 30), F_OF);
            else
                CONDITIONAL_SET_FLAG(((emu->op1.u32 >> 31) ^ emu->op1.u32) & 1, F_OF);
            CONDITIONAL_SET_FLAG(emu->res.u32 & (1 << 31), F_CF);
            break;
        case d_ror64:
            if (BOX64ENV(cputype))
                CONDITIONAL_SET_FLAG(XOR2(emu->res.u64 >> 62), F_OF);
            else
                CONDITIONAL_SET_FLAG(((emu->op1.u64 >> 63) ^ emu->op1.u64) & 1, F_OF);
            CONDITIONAL_SET_FLAG(emu->res.u64 & (1LL << 63), F_CF);
            break;

        case d_unknown:
            printf_log(LOG_NONE, "%p trying to evaluate Unknown deferred Flags\n", (void*)R_RIP);
            break;
    }
    RESET_FLAGS(emu);
}

uintptr_t GetSegmentBaseEmu(x64emu_t* emu, int seg)
{
    if (emu->segs_serial[seg] != emu->context->sel_serial) {
        emu->segs_offs[seg] = (uintptr_t)GetSegmentBase(emu->segs[seg]);
        emu->segs_serial[seg] = emu->context->sel_serial;
    }
    return emu->segs_offs[seg];
}
