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
uint64_t RunFunction(uintptr_t fnc, int nargs, ...)
{
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
uint64_t RunFunctionFmt(uintptr_t fnc, const char* fmt, ...)
{
    x64emu_t *emu = thread_get_emu();
    int nargs = 0;
    int ni = 0;
    int ndf = 0;
    for (int i=0; fmt[i]; ++i) {
        switch(fmt[i]) {
            case 'f': 
            case 'd': if(ndf<8) ++ndf; else ++nargs; break;
            case 'p': 
            case 'i': 
            case 'u': 
            case 'I': 
            case 'U': 
            case 'L': 
            case 'l': 
            case 'w': 
            case 'W': 
            case 'c': 
            case 'C': if(ni<6) ++ni; else ++nargs; break;
            default:
                if(ni<6) ++ni; else ++nargs; break;
        }
    }
    ni = 0;
    ndf = 0;
    int align = nargs&1;
    int stackn = align + nargs;

    Push64(emu, R_RBP); // push rbp
    R_RBP = R_RSP;      // mov rbp, rsp

    R_RSP -= stackn*sizeof(void*);   // need to push in reverse order

    uint64_t *p = (uint64_t*)R_RSP;

    static const int nn[] = {_DI, _SI, _DX, _CX, _R8, _R9};
    #define GO(c, A, B, B2, C) case c: if(ni<6) emu->regs[nn[ni++]].A[0] = C va_arg(va, B2); else {*p = 0; *((B*)p) = va_arg(va, B2); ++p;}; break;
    va_list va;
    va_start (va, fmt);
    for (int i=0; fmt[i]; ++i) {
        switch(fmt[i]) {
            case 'f':   if(ndf<8)
                            emu->xmm[ndf++].f[0] = va_arg(va, double);  // float are promoted to double in ...
                        else {
                            *p = 0;
                            *((float*)p) = va_arg(va, double);
                            ++p;
                        }
                        break;
            case 'd':   if(ndf<8)
                            emu->xmm[ndf++].d[0] = va_arg(va, double);
                        else {
                            *((double*)p) = va_arg(va, double);
                            ++p;
                        }
                        break;
            GO('p', q, void*, void*, (uintptr_t))
            GO('i', sdword, int, int, )
            GO('u', dword, uint32_t, uint32_t, )
            GO('I', sq, int64_t, int64_t, )
            GO('U', q, uint64_t, uint64_t, )
            GO('L', q, uint64_t, uint64_t, )
            GO('l', sq, int64_t, int64_t, )
            GO('w', sword, int16_t, int, )
            GO('W', word, uint16_t, int, )
            GO('c', sbyte, int8_t, int, )
            GO('C', byte, uint8_t, int, )
            default:
                printf_log(LOG_NONE, "Error, unhandled arg %d: '%c' in RunFunctionFmt\n", i, fmt[i]);
                if(ni<6) emu->regs[nn[ni++]].q[0] = va_arg(va, uint64_t); else {*p = va_arg(va, uint64_t); ++p;}; 
                break;
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
uint64_t RunSafeFunction(uintptr_t fnc, int nargs, ...)
{
    x64emu_t * emu = thread_get_emu();

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
    deferred_flags_t old_df = emu->df;
    multiuint_t old_op1 = emu->op1;
    multiuint_t old_op2 = emu->op2;
    multiuint_t old_res = emu->res;
    multiuint_t old_op1_sav= emu->op1_sav;
    multiuint_t old_res_sav= emu->res_sav;
    deferred_flags_t old_df_sav= emu->df_sav;

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

    emu->df = old_df;
    emu->op1 = old_op1;
    emu->op2 = old_op2;
    emu->res = old_res;
    emu->op1_sav = old_op1_sav;
    emu->res_sav = old_res_sav;
    emu->df_sav = old_df_sav;

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
    int oldlong = emu->flags.quitonlongjmp;

    emu->quit = 0;
    emu->flags.quitonlongjmp = QuitOnLongJump;

    DynaCall(emu, fnc);

    if(oldip==R_RIP) {
        R_RSP = R_RBP;      // restore stack only if EIP is the one expected (else, it means return value is not the one expected)
        R_RBP = Pop64(emu); //Pop EBP
    }

    emu->quit = old_quit;
    emu->flags.quitonlongjmp = oldlong;


    return R_RAX;
}

EXPORTDYN
uint64_t RunFunctionWindows(uintptr_t fnc, int nargs, ...)
{
    x64emu_t *emu = thread_get_emu();
    int align = (nargs>4)?(((nargs-4)&1)):0;
    int stackn = align + ((nargs>4)?(nargs-4):0);

    Push64(emu, R_RBP); // push rbp
    R_RBP = R_RSP;      // mov rbp, rsp

    R_RSP -= stackn*sizeof(void*);   // need to push in reverse order

    uint64_t *p = (uint64_t*)R_RSP;

    va_list va;
    va_start (va, nargs);
    for (int i=0; i<nargs; ++i) {
        if(i<4) {
            int nn[] = {_CX, _DX, _R8, _R9};
            emu->regs[nn[i]].q[0] = va_arg(va, uint64_t);
        } else {
            *p = va_arg(va, uint64_t);
            p++;
        }
    }
    va_end (va);

    R_RSP -= 32;    // ShadowArea

    uintptr_t oldip = R_RIP;
    DynaCall(emu, fnc);

    if(oldip==R_RIP) {
        R_RSP = R_RBP;          // mov rsp, rbp
        R_RBP = Pop64(emu);     // pop rbp
    }

    uint64_t ret = R_RAX;

    return ret;
}