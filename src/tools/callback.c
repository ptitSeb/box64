#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include "debug.h"
#include "x64emu.h"
#include "x64run.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "box64context.h"
#include "box64stack.h"
#include "dynarec.h"

EXPORTDYN
uint64_t RunFunction(box64context_t *context, uintptr_t fnc, int nargs, ...)
{
    (void)context;

    x64emu_t *emu = thread_get_emu();
    int align = (nargs>6)?(((nargs-6)&1)):0;
    int stackn = align + ((nargs>6)?(nargs-6):0);

    Push64(emu, R_RBP); // push rbp
    R_RBP = R_RSP;      // mov rbp, rsp

    R_RSP -= stackn*sizeof(void*);   // need to push in reverse order

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

    uintptr_t oldip = R_RIP;
    DynaCall(emu, fnc);

    if(oldip==R_RIP) {
        R_RSP = R_RBP;          // mov rsp, rbp
        R_RBP = Pop64(emu);     // pop rbp
    }

    uint64_t ret = R_RAX;

    return ret;
}

EXPORTDYN
uint64_t RunSafeFunction(box64context_t *context, uintptr_t fnc, int nargs, ...)
{
    (void)context;

    x64emu_t *emu = thread_get_emu();
    int align = (nargs>6)?(((nargs-6)&1)):0;
    int stackn = align + ((nargs>6)?(nargs-6):0);

    Push64(emu, R_RBP); // push rbp
    uintptr_t old_rbp = R_RBP = R_RSP;      // mov rbp, rsp

    Push64(emu, R_RDI);
    Push64(emu, R_RSI);
    Push64(emu, R_RDX);
    Push64(emu, R_RCX);
    Push64(emu, R_R8);
    Push64(emu, R_R9);
    Push64(emu, R_R10);
    Push64(emu, R_R11);
    Push64(emu, R_RAX);

    R_RSP -= stackn*sizeof(void*);   // need to push in reverse order

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

    uintptr_t oldip = R_RIP;
    DynaCall(emu, fnc);

    uint64_t ret = R_RAX;
    R_RIP = oldip;

    R_RAX = Pop64(emu);
    R_R11 = Pop64(emu);
    R_R10 = Pop64(emu);
    R_R9 = Pop64(emu);
    R_R8 = Pop64(emu);
    R_RCX = Pop64(emu);
    R_RDX = Pop64(emu);
    R_RSI = Pop64(emu);
    R_RDI = Pop64(emu);

    R_RSP = old_rbp;          // mov rsp, rbp
    R_RBP = Pop64(emu);     // pop rbp
    

    return ret;
}

EXPORTDYN
uint64_t RunFunctionWithEmu(x64emu_t *emu, int QuitOnLongJump, uintptr_t fnc, int nargs, ...)
{
    int align = (nargs>6)?(((nargs-6)&1)):0;
    int stackn = align + ((nargs>6)?(nargs-6):0);

    Push64(emu, R_RBP); // push rbp
    R_RBP = R_RSP;      // mov rbp, rsp

    R_RSP -= stackn*sizeof(void*);   // need to push in reverse order

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

    uintptr_t oldip = R_RIP;
    int old_quit = emu->quit;
    int oldlong = emu->quitonlongjmp;

    emu->quit = 0;
    emu->quitonlongjmp = QuitOnLongJump;

    DynaCall(emu, fnc);

    if(oldip==R_RIP) {
        R_RSP = R_RBP;      // restore stack only if EIP is the one expected (else, it means return value is not the one expected)
        R_RBP = Pop64(emu); //Pop EBP
    }

    emu->quit = old_quit;
    emu->quitonlongjmp = oldlong;


    return R_RAX;
}
