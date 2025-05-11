#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include "debug.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "box64context.h"
#include "box64stack.h"
#include "box64cpu.h"
#include "box64cpu_util.h"
#ifdef BOX32
#include "box32.h"
#include "emu/x87emu_private.h"
#endif

EXPORTDYN
uint64_t RunFunction(uintptr_t fnc, int nargs, ...)
{
    x64emu_t *emu = thread_get_emu();
    #ifdef BOX32
    if(box64_is32bits) {
        Push_32(emu, R_RBP); // push ebp
        R_RBP = R_ESP;       // mov ebp, esp

        R_ESP -= nargs*4+(4-(nargs&3))*4;   // need to push in reverse order

        ptr_t *p = (ptr_t*)from_ptrv(R_ESP);

        va_list va;
        va_start (va, nargs);
        for (int i=0; i<nargs; ++i) {
            *p = va_arg(va, uint32_t);
            p++;
        }
        va_end (va);
    } else
    #endif
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
    }

    uintptr_t oldip = R_RIP;
    DynaCall(emu, fnc);

    if(oldip==R_RIP) {
        #ifdef BOX32
        if(box64_is32bits) {
            R_RSP = R_EBP;          // mov esp, ebp
            R_EBP = Pop_32(emu);    // pop ebp
        }
        #endif
        {
            R_RSP = R_RBP;          // mov rsp, rbp
            R_RBP = Pop64(emu);     // pop rbp
        }
    }

    uint64_t ret = box64_is32bits?((uint64_t)R_EAX | ((uint64_t)R_EDX)<<32):R_RAX;

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
        #ifdef BOX32
        if(box64_is32bits)
            switch(fmt[i]) {
                case 'd': 
                case 'I': 
                case 'U': nargs+=2; break;
                case 'p': 
                case 'L': 
                case 'l': 
                case 'f': 
                case 'i': 
                case 'u': 
                case 'w': 
                case 'W': 
                case 'c': 
                case 'C': ++nargs; break;
                default:
                    ++nargs; break;
            }
        else
        #endif
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
    int sizeof_ptr = sizeof(void*);
    #ifdef BOX32
    if(box64_is32bits) {
        Push_32(emu, R_EBP); // push ebp
        R_RBP = R_ESP;       // mov ebp, esp
        sizeof_ptr = sizeof(ptr_t);
        align = (4-(nargs&3))&3;
    } else
    #endif
    {
        Push64(emu, R_RBP); // push rbp
        R_RBP = R_RSP;      // mov rbp, rsp
    }

    R_RSP -= stackn*sizeof_ptr;   // need to push in reverse order

    #ifdef BOX32
    if(box64_is32bits) {
        ptr_t *p = (ptr_t*)from_ptrv(R_ESP);

        #define GO(c, B, B2, N) case c: *((B*)p) = va_arg(va, B2); p+=N; break
        va_list va;
        va_start (va, fmt);
        for (int i=0; fmt[i]; ++i) {
            switch(fmt[i]) {
                GO('f', float, double, 1);
                GO('d', double, double, 2);
                case 'p': *((ptr_t*)p) = to_ptrv(va_arg(va, void*)); p+=1; break;
                GO('i', int, int, 1);
                GO('u', uint32_t, uint32_t, 1);
                GO('I', int64_t, int64_t, 2);
                GO('U', uint64_t, uint64_t, 2);
                GO('L', uint32_t, uint64_t, 1);     // long are 64bits on 64bits system
                GO('l', int32_t, int64_t, 1);       // but 32bits on 32bits system
                GO('w', int16_t, int, 1);
                GO('W', uint16_t, int, 1);
                GO('c', int8_t, int, 1);
                GO('C', uint8_t, int, 1);
                default:
                    printf_log(LOG_NONE, "Error, unhandled arg %d: '%c' in RunFunctionFmt\n", i, fmt[i]);
                    *p = va_arg(va, uint32_t);
                    ++p; 
                    break;
            }
        }
        #undef GO
        va_end (va);
    } else
    #endif
    {
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
        #undef GO
        va_end (va);
    }

    uintptr_t oldip = R_RIP;
    DynaCall(emu, fnc);

    if(oldip==R_RIP) {
        #ifdef BOX32
        if(box64_is32bits) {
            R_RSP = R_EBP;          // mov esp, ebp
            R_RBP = Pop_32(emu);    // pop ebp
        } else
        #endif
        {
            R_RSP = R_RBP;          // mov rsp, rbp
            R_RBP = Pop64(emu);     // pop rbp
        }
    }

    uint64_t ret = box64_is32bits?((uint64_t)R_EAX | ((uint64_t)R_EDX)<<32):R_RAX;

    return ret;
}

EXPORTDYN
double RunFunctionFmtD(uintptr_t fnc, const char* fmt, ...)
{
    x64emu_t *emu = thread_get_emu();
    int nargs = 0;
    int ni = 0;
    int ndf = 0;
    for (int i=0; fmt[i]; ++i) {
        #ifdef BOX32
        if(box64_is32bits)
            switch(fmt[i]) {
                case 'd': 
                case 'I': 
                case 'U': nargs+=2; break;
                case 'p': 
                case 'L': 
                case 'l': 
                case 'f': 
                case 'i': 
                case 'u': 
                case 'w': 
                case 'W': 
                case 'c': 
                case 'C': ++nargs; break;
                default:
                    ++nargs; break;
            }
        else
        #endif
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
    int sizeof_ptr = sizeof(void*);
    #ifdef BOX32
    if(box64_is32bits) {
        Push_32(emu, R_EBP); // push ebp
        R_RBP = R_ESP;       // mov ebp, esp
        sizeof_ptr = sizeof(ptr_t);
        align = (4-(nargs&3))&3;
    } else
    #endif
    {
        Push64(emu, R_RBP); // push rbp
        R_RBP = R_RSP;      // mov rbp, rsp
    }

    R_RSP -= stackn*sizeof_ptr;   // need to push in reverse order

    #ifdef BOX32
    if(box64_is32bits) {
        ptr_t *p = (ptr_t*)from_ptrv(R_ESP);

        #define GO(c, B, B2, N) case c: *((B*)p) = va_arg(va, B2); p+=N; break
        va_list va;
        va_start (va, fmt);
        for (int i=0; fmt[i]; ++i) {
            switch(fmt[i]) {
                GO('f', float, double, 1);
                GO('d', double, double, 2);
                case 'p': *((ptr_t*)p) = to_ptrv(va_arg(va, void*)); p+=1; break;
                GO('i', int, int, 1);
                GO('u', uint32_t, uint32_t, 1);
                GO('I', int64_t, int64_t, 2);
                GO('U', uint64_t, uint64_t, 2);
                GO('L', uint32_t, uint64_t, 1);     // long are 64bits on 64bits system
                GO('l', int32_t, int64_t, 1);       // but 32bits on 32bits system
                GO('w', int16_t, int, 1);
                GO('W', uint16_t, int, 1);
                GO('c', int8_t, int, 1);
                GO('C', uint8_t, int, 1);
                default:
                    printf_log(LOG_NONE, "Error, unhandled arg %d: '%c' in RunFunctionFmt\n", i, fmt[i]);
                    *p = va_arg(va, uint32_t);
                    ++p; 
                    break;
            }
        }
        #undef GO
        va_end (va);
    } else
    #endif
    {
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
        #undef GO
        va_end (va);
    }

    uintptr_t oldip = R_RIP;
    DynaCall(emu, fnc);

    if(oldip==R_RIP) {
        #ifdef BOX32
        if(box64_is32bits) {
            R_RSP = R_EBP;          // mov esp, ebp
            R_RBP = Pop_32(emu);    // pop ebp
        } else
        #endif
        {
            R_RSP = R_RBP;          // mov rsp, rbp
            R_RBP = Pop64(emu);     // pop rbp
        }
    }
    double ret;
    #ifdef BOX32
    if(box64_is32bits) {
        ret = ST0.d;
        fpu_do_pop(emu);
    } else
    #endif
    ret = emu->xmm[0].d[0];

    return ret;
}

EXPORTDYN
uint64_t RunSafeFunction(uintptr_t fnc, int nargs, ...)
{
    x64emu_t * emu = thread_get_emu();
    if(box64_is32bits) {
        printf_log(LOG_NONE, "Calling RunSafeFunction in 32bits\n");
        abort();
    }

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

    return ret;
}

EXPORTDYN
uint64_t RunFunctionWithEmu(x64emu_t *emu, int QuitOnLongJump, uintptr_t fnc, int nargs, ...)
{
    #ifdef BOX32
    if(box64_is32bits) {
        Push_32(emu, R_RBP); // push ebp
        R_RBP = R_ESP;       // mov ebp, esp

        R_ESP -= nargs*4 + ((4-(nargs&3))&3)*4;   // need to push in reverse order

        ptr_t *p = (ptr_t*)from_ptrv(R_ESP);

        va_list va;
        va_start (va, nargs);
        for (int i=0; i<nargs; ++i) {
            *p = va_arg(va, uint32_t);
            p++;
        }
        va_end (va);
    } else
    #endif
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
    }

    uintptr_t oldip = R_RIP;
    int old_quit = emu->quit;
    int oldlong = emu->flags.quitonlongjmp;

    emu->quit = 0;
    emu->flags.quitonlongjmp = QuitOnLongJump;

    DynaCall(emu, fnc);

    if(oldip==R_RIP) {
        R_RSP = R_RBP;      // restore stack only if EIP is the one expected (else, it means return value is not the one expected)
        #ifdef BOX32
        if(box64_is32bits)
            R_RBP = Pop_32(emu); //Pop EBP
        else
        #endif
            R_RBP = Pop64(emu); //Pop EBP
    }

    emu->quit = old_quit;
    emu->flags.quitonlongjmp = oldlong;


    return box64_is32bits?((uint64_t)R_EAX | ((uint64_t)R_EDX)<<32):R_RAX;;
}

EXPORTDYN
uint64_t RunFunctionWindows(uintptr_t fnc, int nargs, ...)
{
    x64emu_t *emu = thread_get_emu();
    if(box64_is32bits) {
        printf_log(LOG_NONE, "Calling RunFunctionWindows in 32bits\n");
        abort();
    }
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
