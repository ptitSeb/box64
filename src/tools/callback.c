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

    if(nargs>6)
        R_ESP -= (nargs-6)*4;   // need to push in reverse order

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

    DynaCall(emu, fnc);
    if(nargs>6)
        R_ESP+=((nargs-6)*4);

    uint64_t ret = R_RAX;

    return ret;
}

EXPORTDYN
uint64_t RunFunctionWithEmu(x64emu_t *emu, int QuitOnLongJump, uintptr_t fnc, int nargs, ...)
{
    if(nargs>6)
        R_ESP -= (nargs-6)*4;   // need to push in reverse order

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

    if(oldip==R_RIP && nargs>6)
        R_ESP+=((nargs-6)*4);   // restore stack only if EIP is the one expected (else, it means return value is not the one expected)

    emu->quit = old_quit;
    emu->quitonlongjmp = oldlong;


    return R_RAX;
}
