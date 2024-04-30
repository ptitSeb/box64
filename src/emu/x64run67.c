#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"
#include "box64stack.h"
#include "x64emu.h"
#include "x64run.h"
#include "x64emu_private.h"
#include "x64run_private.h"
#include "x64primop.h"
#include "x64trace.h"
#include "x87emu_private.h"
#include "box64context.h"
#include "bridge.h"

#include "modrm.h"

#ifdef TEST_INTERPRETER
uintptr_t Test67(x64test_t *test, rex_t rex, int rep, uintptr_t addr)
#else
uintptr_t Run67(x64emu_t *emu, rex_t rex, int rep, uintptr_t addr)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    int8_t tmp8s;
    uint8_t tmp8u;
    uint32_t tmp32u;
    int32_t tmp32s;
    uint64_t tmp64u;
    reg64_t *oped, *opgd;
    #ifdef TEST_INTERPRETER
    x64emu_t* emu = test->emu;
    #endif
    if(rex.is32bits)
    #ifdef TEST_INTERPRETER
        return Test67_32(test, rex, rep, addr);
    #else
        return Run67_32(emu, rex, rep, addr);
    #endif

    opcode = F8;

    while(opcode==0x67)
        opcode = F8;

    // REX prefix before the 67 are ignored
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }
    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }

    switch(opcode) {
    #define GO(B, OP)                                   \
    case B+0:                                           \
        nextop = F8;                                    \
        GETEB32(0);                                     \
        GETGB;                                          \
        EB->byte[0] = OP##8(emu, EB->byte[0], GB);      \
        break;                                          \
    case B+1:                                           \
        nextop = F8;                                    \
        GETED32(0);                                     \
        GETGD;                                          \
        if(rex.w)                                       \
            ED->q[0] = OP##64(emu, ED->q[0], GD->q[0]); \
        else {                                          \
            if(MODREG)                                  \
                ED->q[0] = OP##32(emu, ED->dword[0], GD->dword[0]);     \
            else                                                        \
                ED->dword[0] = OP##32(emu, ED->dword[0], GD->dword[0]); \
        }                                               \
        break;                                          \
    case B+2:                                           \
        nextop = F8;                                    \
        GETEB32(0);                                     \
        GETGB;                                          \
        GB = OP##8(emu, GB, EB->byte[0]);               \
        break;                                          \
    case B+3:                                           \
        nextop = F8;                                    \
        GETED32(0);                                     \
        GETGD;                                          \
        if(rex.w)                                       \
            GD->q[0] = OP##64(emu, GD->q[0], ED->q[0]); \
        else                                            \
            GD->q[0] = OP##32(emu, GD->dword[0], ED->dword[0]); \
        break;                                          \
    case B+4:                                           \
        R_AL = OP##8(emu, R_AL, F8);                    \
        break;                                          \
    case B+5:                                           \
        if(rex.w)                                       \
            R_RAX = OP##64(emu, R_RAX, F32S64);         \
        else                                            \
            R_RAX = OP##32(emu, R_EAX, F32);            \
        break;

    GO(0x00, add)                   /* ADD 0x00 -> 0x05 */
    GO(0x08, or)                    /*  OR 0x08 -> 0x0D */
    case 0x0F:
        #ifdef TEST_INTERPRETER
        return Test670F(test, rex, rep, addr);
        #else
        return Run670F(emu, rex, rep, addr);
        #endif
    GO(0x10, adc)                   /* ADC 0x10 -> 0x15 */
    GO(0x18, sbb)                   /* SBB 0x18 -> 0x1D */
    GO(0x20, and)                   /* AND 0x20 -> 0x25 */
    GO(0x28, sub)                   /* SUB 0x28 -> 0x2D */
    GO(0x30, xor)                   /* XOR 0x30 -> 0x35 */
    #undef GO
    case 0x38:
        nextop = F8;
        GETEB32(0);
        GETGB;
        cmp8(emu, EB->byte[0], GB);
        break;
    case 0x39:
        nextop = F8;
        GETED32(0);
        GETGD;
        if(rex.w)
            cmp64(emu, ED->q[0], GD->q[0]);
        else
            cmp32(emu, ED->dword[0], GD->dword[0]);
        break;
    case 0x3A:
        nextop = F8;
        GETEB32(0);
        GETGB;
        cmp8(emu, GB, EB->byte[0]);
        break;
    case 0x3B:
        nextop = F8;
        GETED32(0);
        GETGD;
        if(rex.w)
            cmp64(emu, GD->q[0], ED->q[0]);
        else
            cmp32(emu, GD->dword[0], ED->dword[0]);
        break;
    case 0x3C:
        R_AL = cmp8(emu, R_AL, F8);
        break;
    case 0x3D:
        if(rex.w)
            cmp64(emu, R_RAX, F32S64);
        else
            cmp32(emu, R_EAX, F32);
        break;

    case 0x63:                      /* MOVSXD Gd,Ed */
        nextop = F8;
        GETED32(0);
        GETGD;
        if(rex.w)
            GD->sq[0] = ED->sdword[0];
        else
            if(MODREG)
                GD->q[0] = ED->dword[0];    // not really a sign extension
            else
                GD->sdword[0] = ED->sdword[0];  // meh?
        break;

    case 0x66:
        #ifdef TEST_INTERPRETER
        return Test6766(test, rex, rep, addr);
        #else
        return Run6766(emu, rex, rep, addr);
        #endif

    case 0x80:                      /* GRP Eb,Ib */
        nextop = F8;
        GETEB32(1);
        tmp8u = F8;
        switch((nextop>>3)&7) {
            case 0: EB->byte[0] = add8(emu, EB->byte[0], tmp8u); break;
            case 1: EB->byte[0] =  or8(emu, EB->byte[0], tmp8u); break;
            case 2: EB->byte[0] = adc8(emu, EB->byte[0], tmp8u); break;
            case 3: EB->byte[0] = sbb8(emu, EB->byte[0], tmp8u); break;
            case 4: EB->byte[0] = and8(emu, EB->byte[0], tmp8u); break;
            case 5: EB->byte[0] = sub8(emu, EB->byte[0], tmp8u); break;
            case 6: EB->byte[0] = xor8(emu, EB->byte[0], tmp8u); break;
            case 7:               cmp8(emu, EB->byte[0], tmp8u); break;
        }
        break;
    case 0x81:                      /* GRP Ed,Id */
    case 0x83:                      /* GRP Ed,Ib */
        nextop = F8;
        GETED32((opcode==0x81)?4:1);
        if(opcode==0x81) {
            tmp32s = F32S;
        } else {
            tmp32s = F8S;
        }
        if(rex.w) {
            tmp64u = (uint64_t)(int64_t)tmp32s;
            switch((nextop>>3)&7) {
                case 0: ED->q[0] = add64(emu, ED->q[0], tmp64u); break;
                case 1: ED->q[0] =  or64(emu, ED->q[0], tmp64u); break;
                case 2: ED->q[0] = adc64(emu, ED->q[0], tmp64u); break;
                case 3: ED->q[0] = sbb64(emu, ED->q[0], tmp64u); break;
                case 4: ED->q[0] = and64(emu, ED->q[0], tmp64u); break;
                case 5: ED->q[0] = sub64(emu, ED->q[0], tmp64u); break;
                case 6: ED->q[0] = xor64(emu, ED->q[0], tmp64u); break;
                case 7:            cmp64(emu, ED->q[0], tmp64u); break;
            }
        } else {
            tmp32u = (uint32_t)tmp32s;
            if(MODREG)
                switch((nextop>>3)&7) {
                    case 0: ED->q[0] = add32(emu, ED->dword[0], tmp32u); break;
                    case 1: ED->q[0] =  or32(emu, ED->dword[0], tmp32u); break;
                    case 2: ED->q[0] = adc32(emu, ED->dword[0], tmp32u); break;
                    case 3: ED->q[0] = sbb32(emu, ED->dword[0], tmp32u); break;
                    case 4: ED->q[0] = and32(emu, ED->dword[0], tmp32u); break;
                    case 5: ED->q[0] = sub32(emu, ED->dword[0], tmp32u); break;
                    case 6: ED->q[0] = xor32(emu, ED->dword[0], tmp32u); break;
                    case 7:            cmp32(emu, ED->dword[0], tmp32u); break;
                }
            else
                switch((nextop>>3)&7) {
                    case 0: ED->dword[0] = add32(emu, ED->dword[0], tmp32u); break;
                    case 1: ED->dword[0] =  or32(emu, ED->dword[0], tmp32u); break;
                    case 2: ED->dword[0] = adc32(emu, ED->dword[0], tmp32u); break;
                    case 3: ED->dword[0] = sbb32(emu, ED->dword[0], tmp32u); break;
                    case 4: ED->dword[0] = and32(emu, ED->dword[0], tmp32u); break;
                    case 5: ED->dword[0] = sub32(emu, ED->dword[0], tmp32u); break;
                    case 6: ED->dword[0] = xor32(emu, ED->dword[0], tmp32u); break;
                    case 7:                cmp32(emu, ED->dword[0], tmp32u); break;
                }
        }
        break;

    case 0x88:                      /* MOV Eb,Gb */
        nextop = F8;
        GETEB32(0);
        GETGB;
        EB->byte[0] = GB;
        break;
    case 0x89:                    /* MOV Ed,Gd */
        nextop = F8;
        GETED32(0);
        GETGD;
        if(rex.w) {
            ED->q[0] = GD->q[0];
        } else {
            //if ED is a reg, than the opcode works like movzx
            if(MODREG)
                ED->q[0] = GD->dword[0];
            else
                ED->dword[0] = GD->dword[0];
        }
        break;
    case 0x8A:                      /* MOV Gb,Eb */
        nextop = F8;
        GETEB32(0);
        GETGB;
        GB = EB->byte[0];
        break;
    case 0x8B:                      /* MOV Gd,Ed */
        nextop = F8;
        GETED32(0);
        GETGD;
        if(rex.w)
            GD->q[0] = ED->q[0];
        else
            GD->q[0] = ED->dword[0];
        break;

    case 0x8D:                      /* LEA Gd,M */
        nextop = F8;
        _GETED32(0);
        GETGD;
        if(rex.w)
            GD->q[0] = (uint64_t)ED;
        else
            GD->q[0] = ((uintptr_t)ED)&0xffffffff;
        break;

    case 0xC1:                      /* GRP2 Ed,Ib */
        nextop = F8;
        GETED32(1);
        tmp8u = F8/* & 0x1f*/; // masking done in each functions
        if(rex.w) {
            switch((nextop>>3)&7) {
                case 0: ED->q[0] = rol64(emu, ED->q[0], tmp8u); break;
                case 1: ED->q[0] = ror64(emu, ED->q[0], tmp8u); break;
                case 2: ED->q[0] = rcl64(emu, ED->q[0], tmp8u); break;
                case 3: ED->q[0] = rcr64(emu, ED->q[0], tmp8u); break;
                case 4:
                case 6: ED->q[0] = shl64(emu, ED->q[0], tmp8u); break;
                case 5: ED->q[0] = shr64(emu, ED->q[0], tmp8u); break;
                case 7: ED->q[0] = sar64(emu, ED->q[0], tmp8u); break;
            }
        } else {
            if(MODREG)
                switch((nextop>>3)&7) {
                    case 0: ED->q[0] = rol32(emu, ED->dword[0], tmp8u); break;
                    case 1: ED->q[0] = ror32(emu, ED->dword[0], tmp8u); break;
                    case 2: ED->q[0] = rcl32(emu, ED->dword[0], tmp8u); break;
                    case 3: ED->q[0] = rcr32(emu, ED->dword[0], tmp8u); break;
                    case 4:
                    case 6: ED->q[0] = shl32(emu, ED->dword[0], tmp8u); break;
                    case 5: ED->q[0] = shr32(emu, ED->dword[0], tmp8u); break;
                    case 7: ED->q[0] = sar32(emu, ED->dword[0], tmp8u); break;
                }
            else
                switch((nextop>>3)&7) {
                    case 0: ED->dword[0] = rol32(emu, ED->dword[0], tmp8u); break;
                    case 1: ED->dword[0] = ror32(emu, ED->dword[0], tmp8u); break;
                    case 2: ED->dword[0] = rcl32(emu, ED->dword[0], tmp8u); break;
                    case 3: ED->dword[0] = rcr32(emu, ED->dword[0], tmp8u); break;
                    case 4:
                    case 6: ED->dword[0] = shl32(emu, ED->dword[0], tmp8u); break;
                    case 5: ED->dword[0] = shr32(emu, ED->dword[0], tmp8u); break;
                    case 7: ED->dword[0] = sar32(emu, ED->dword[0], tmp8u); break;
                }
        }
        break;

    case 0xC6:                      /* MOV Eb,Ib */
        nextop = F8;
        GETEB32(1);
        EB->byte[0] = F8;
        break;
    case 0xC7:                      /* MOV Ed,Id */
        nextop = F8;
        GETED32(4);
        if(rex.w)
            ED->q[0] = F32S64;
        else
            if(MODREG)
                ED->q[0] = F32;
            else
                ED->dword[0] = F32;
        break;

    case 0xE0:                      /* LOOPNZ */
        CHECK_FLAGS(emu);
        tmp8s = F8S;
        --R_ECX; // don't update flags
        if(R_ECX && !ACCESS_FLAG(F_ZF))
            addr += tmp8s;
        break;
    case 0xE1:                      /* LOOPZ */
        CHECK_FLAGS(emu);
        tmp8s = F8S;
        --R_ECX; // don't update flags
        if(R_ECX && ACCESS_FLAG(F_ZF))
            addr += tmp8s;
        break;
    case 0xE2:                      /* LOOP */
        tmp8s = F8S;
        --R_ECX; // don't update flags
        if(R_ECX)
            addr += tmp8s;
        break;
    case 0xE3:              /* JECXZ Ib */
        tmp8s = F8S;
        if(!R_ECX)
            addr += tmp8s;
        break;

    case 0xE8:                      /* CALL Id */
        tmp32s = F32S; // call is relative
        Push64(emu, addr);
        addr += tmp32s;
        break;

    case 0xF7:                      /* GRP3 Ed(,Id) */
        nextop = F8;
        tmp8u = (nextop>>3)&7;
        GETED32((tmp8u<2)?4:0);
        if(rex.w) {
            switch(tmp8u) {
                case 0: 
                case 1:                 /* TEST Ed,Id */
                    tmp64u = F32S64;
                    test64(emu, ED->q[0], tmp64u);
                    break;
                case 2:                 /* NOT Ed */
                    ED->q[0] = not64(emu, ED->q[0]);
                    break;
                case 3:                 /* NEG Ed */
                    ED->q[0] = neg64(emu, ED->q[0]);
                    break;
                case 4:                 /* MUL RAX,Ed */
                    mul64_rax(emu, ED->q[0]);
                    break;
                case 5:                 /* IMUL RAX,Ed */
                    imul64_rax(emu, ED->q[0]);
                    break;
                case 6:                 /* DIV Ed */
                    div64(emu, ED->q[0]);
                    break;
                case 7:                 /* IDIV Ed */
                    idiv64(emu, ED->q[0]);
                    #ifdef TEST_INTERPRETER
                    test->notest = 1;
                    #endif
                    break;
            }
        } else {
            switch(tmp8u) {
                case 0: 
                case 1:                 /* TEST Ed,Id */
                    tmp32u = F32;
                    test32(emu, ED->dword[0], tmp32u);
                    break;
                case 2:                 /* NOT Ed */
                    if(MODREG)
                        ED->q[0] = not32(emu, ED->dword[0]);
                    else
                        ED->dword[0] = not32(emu, ED->dword[0]);
                    break;
                case 3:                 /* NEG Ed */
                    if(MODREG)
                        ED->q[0] = neg32(emu, ED->dword[0]);
                    else
                        ED->dword[0] = neg32(emu, ED->dword[0]);
                    break;
                case 4:                 /* MUL EAX,Ed */
                    mul32_eax(emu, ED->dword[0]);
                    emu->regs[_AX].dword[1] = 0;
                    emu->regs[_DX].dword[1] = 0;
                    break;
                case 5:                 /* IMUL EAX,Ed */
                    imul32_eax(emu, ED->dword[0]);
                    emu->regs[_AX].dword[1] = 0;
                    emu->regs[_DX].dword[1] = 0;
                    break;
                case 6:                 /* DIV Ed */
                    div32(emu, ED->dword[0]);
                    //emu->regs[_AX].dword[1] = 0;
                    //emu->regs[_DX].dword[1] = 0;
                    break;
                case 7:                 /* IDIV Ed */
                    idiv32(emu, ED->dword[0]);
                    //emu->regs[_AX].dword[1] = 0;
                    //emu->regs[_DX].dword[1] = 0;
                    break;
            }
        }
        break;
    
    case 0xFF:
        nextop = F8;
        switch((nextop>>3)&7) {
            case 2:                 /* CALL NEAR Ed */
                GETED32(0);
                tmp64u = (uintptr_t)getAlternate((void*)ED->q[0]);
                Push64(emu, addr);
                addr = tmp64u;
                break;
            default:
                return 0;
        }
        break;
    default:
        return 0;
    }
    return addr;
}