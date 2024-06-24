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
#ifdef DYNAREC
#include "../dynarec/native_lock.h"
#endif

#include "modrm.h"

#ifdef TEST_INTERPRETER
uintptr_t Test66(x64test_t *test, rex_t rex, int rep, uintptr_t addr)
#else
uintptr_t Run66(x64emu_t *emu, rex_t rex, int rep, uintptr_t addr)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    int8_t tmp8s;
    uint8_t tmp8u, tmp8u2;
    int16_t tmp16s;
    uint16_t tmp16u, tmp16u2;
    int32_t tmp32s;
    int64_t tmp64s;
    uint64_t tmp64u, tmp64u2, tmp64u3;
    reg64_t *oped, *opgd;
    #ifdef TEST_INTERPRETER
    x64emu_t* emu = test->emu;
    #endif

    opcode = F8;

    while((opcode==0x2E) || (opcode==0x36) || (opcode==0x26) || (opcode==0x66))   // ignoring CS:, SS:, ES: or multiple 0x66
        opcode = F8;

    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }

    rex.rex = 0;
    if(!rex.is32bits)
        while(opcode>=0x40 && opcode<=0x4f) {
            rex.rex = opcode;
            opcode = F8;
        }

    switch(opcode) {
    #define GO(B, OP)                                               \
    case B+0:                                                       \
        nextop = F8;                                                \
        GETEB(0);                                                   \
        GETGB;                                                      \
        EB->byte[0] = OP##8(emu, EB->byte[0], GB);                  \
        break;                                                      \
    case B+1:                                                       \
        nextop = F8;                                                \
        GETEW(0);                                                   \
        GETGW;                                                      \
        if(rex.w)                                                   \
            EW->q[0] = OP##64(emu, EW->q[0], GW->q[0]);             \
        else                                                        \
            EW->word[0] = OP##16(emu, EW->word[0], GW->word[0]);    \
        break;                                                      \
    case B+2:                                                       \
        nextop = F8;                                                \
        GETEB(0);                                                   \
        GETGB;                                                      \
        GB = OP##8(emu, GB, EB->byte[0]);                           \
        break;                                                      \
    case B+3:                                                       \
        nextop = F8;                                                \
        GETEW(0);                                                   \
        GETGW;                                                      \
        if(rex.w)                                                   \
            GW->q[0] = OP##64(emu, GW->q[0], EW->q[0]);             \
        else                                                        \
            GW->word[0] = OP##16(emu, GW->word[0], EW->word[0]);    \
        break;                                                      \
    case B+4:                                                       \
        R_AL = OP##8(emu, R_AL, F8);                                \
        break;                                                      \
    case B+5:                                                       \
        if(rex.w)                                                   \
            R_RAX = OP##64(emu, R_RAX, F32S64);                     \
        else                                                        \
            R_AX = OP##16(emu, R_AX, F16);                          \
        break;

    GO(0x00, add)                   /* ADD 0x01 ~> 0x05 */
    GO(0x08, or)                    /*  OR 0x09 ~> 0x0D */
    GO(0x10, adc)                   /* ADC 0x11 ~> 0x15 */
    GO(0x18, sbb)                   /* SBB 0x19 ~> 0x1D */
    GO(0x20, and)                   /* AND 0x21 ~> 0x25 */
    GO(0x28, sub)                   /* SUB 0x29 ~> 0x2D */
    GO(0x30, xor)                   /* XOR 0x31 ~> 0x35 */

    case 0x06:                      /* PUSH ES */
        if(!rex.is32bits) {
            return 0;
        }
        Push16(emu, emu->segs[_ES]);
        break;
    case 0x07:                      /* POP ES */
        if(!rex.is32bits) {
            return 0;
        }
        emu->segs[_ES] = Pop16(emu);
        emu->segs_serial[_ES] = 0;
        break;

    case 0x0F:                              /* more opcdes */
        switch(rep) {
            case 0:
                #ifdef TEST_INTERPRETER
                return Test660F(test, rex, addr);
                #else
                return Run660F(emu, rex, addr);
                #endif
            case 1:
                #ifdef TEST_INTERPRETER
                return Test66F20F(test, rex, addr);
                #else
                return Run66F20F(emu, rex, addr);
                #endif
            case 2:
                #ifdef TEST_INTERPRETER
                return Test66F30F(test, rex, addr);
                #else
                return Run66F30F(emu, rex, addr);
                #endif
        }
        return 0; // unreachable

        case 0x1E:                      /* PUSH DS */
            if(!rex.is32bits) {
                return 0;
            }
            Push16(emu, emu->segs[_DS]);
            break;
        case 0x1F:                      /* POP DS */
            if(!rex.is32bits) {
                return 0;
            }
            emu->segs[_DS] = Pop16(emu);    // no check, no use....
            emu->segs_serial[_DS] = 0;
            break;

    case 0x39:
        nextop = F8;
        GETEW(0);
        GETGW;
        if(rex.w)
            cmp64(emu, EW->q[0], GW->q[0]);
        else
            cmp16(emu, EW->word[0], GW->word[0]);
        break;
    case 0x3B:
        nextop = F8;
        GETEW(0);
        GETGW;
        if(rex.w)
            cmp64(emu, GW->q[0], EW->dword[0]);
        else
            cmp16(emu, GW->word[0], EW->word[0]);
        break;
    case 0x3D:
        if(rex.w)
            cmp64(emu, R_RAX, F32S64);
        else
            cmp16(emu, R_AX, F16);
        break;

    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    case 0x44:
    case 0x45:
    case 0x46:
    case 0x47:                              /* INC Reg (32bits only) */
        tmp8u = opcode&7;
        emu->regs[tmp8u].word[0] = inc16(emu, emu->regs[tmp8u].word[0]);
        break;
    case 0x48:
    case 0x49:
    case 0x4A:
    case 0x4B:
    case 0x4C:
    case 0x4D:
    case 0x4E:
    case 0x4F:                              /* DEC Reg (32bits only) */
        tmp8u = opcode&7;
        emu->regs[tmp8u].word[0] = dec16(emu, emu->regs[tmp8u].word[0]);
        break;
    case 0x50:
    case 0x51:
    case 0x52:
    case 0x53:
    case 0x54:
    case 0x55:
    case 0x56:
    case 0x57:                      /* PUSH Reg */
        tmp16u = emu->regs[opcode&7].word[0];
        Push16(emu, tmp16u);
        break;
    case 0x58:
    case 0x59:
    case 0x5A:
    case 0x5B:
    case 0x5C:                      /* POP ESP */
    case 0x5D:
    case 0x5E:
    case 0x5F:                      /* POP Reg */
        tmp8u = opcode&7;
        emu->regs[tmp8u].word[0] = Pop16(emu);
        break;
    case 0x60:                              /* PUSHA */
        if(rex.is32bits) {
            tmp16u = R_SP;
            Push16(emu, R_AX);
            Push16(emu, R_CX);
            Push16(emu, R_DX);
            Push16(emu, R_BX);
            Push16(emu, tmp16u);
            Push16(emu, R_BP);
            Push16(emu, R_SI);
            Push16(emu, R_DI);
        } else {
            return 0;
        }
        break;
    case 0x61:                              /* POPA */
        if(rex.is32bits) {
            R_DI = Pop16(emu);
            R_SI = Pop16(emu);
            R_BP = Pop16(emu);
            R_ESP+=2;   // POP ESP
            R_BX = Pop16(emu);
            R_DX = Pop16(emu);
            R_CX = Pop16(emu);
            R_AX = Pop16(emu);
        } else {
            return 0;
        }
        break;

    case 0x64:                              /* FS: */
        #ifdef TEST_INTERPRETER
        return Test6664(test, rex, _FS, addr);
        #else
        return Run6664(emu, rex, _FS, addr);
        #endif
    case 0x65:                              /* GS: */
        #ifdef TEST_INTERPRETER
        return Test6664(test, rex, _GS, addr);
        #else
        return Run6664(emu, rex, _GS, addr);
        #endif

    case 0x68:                       /* PUSH u16 */
        tmp16u = F16;
        Push16(emu, tmp16u);
        break;
    case 0x69:                      /* IMUL Gw,Ew,Iw */
        nextop = F8;
        GETEW(rex.w?4:2);
        GETGW;
        if(rex.w) {
            tmp64u = F32S64;
            GW->q[0] = imul64(emu, EW->q[0], tmp64u);
        } else {
            tmp16u = F16;
            GW->word[0] = imul16(emu, EW->word[0], tmp16u);
        }
        break;

    case 0x6B:                      /* IMUL Gw,Ew,Ib */
        nextop = F8;
        GETEW(1);
        GETGW;
        if(rex.w) {
            tmp64s = F8S;
            GW->q[0] = imul64(emu, EW->q[0], (uint64_t)tmp64s);
        } else {
            tmp16s = F8S;
            GW->word[0] = imul16(emu, EW->word[0], (uint16_t)tmp16s);
        }
        break;
    
    case 0x70:
    case 0x71:
    case 0x72:
    case 0x73:
    case 0x74:
    case 0x75:
    case 0x76:
    case 0x77:
    case 0x78:
    case 0x79:
    case 0x7a:
    case 0x7b:
    case 0x7c:
    case 0x7d:
    case 0x7e:
    case 0x7f:
        // just ignore the 66 (and maybe other) prefix and use regular conditional jump
        addr--;
        return addr;

    case 0x81:                              /* GRP3 Ew,Iw */
    case 0x83:                              /* GRP3 Ew,Ib */
        nextop = F8;
        GETEW((opcode==0x81)?2:1);
        GETGW;
        if(opcode==0x81) 
            tmp16u = F16;
        else {
            tmp16s = F8S;
            tmp16u = (uint16_t)tmp16s;
        }
        switch((nextop>>3)&7) {
            case 0: EW->word[0] = add16(emu, EW->word[0], tmp16u); break;
            case 1: EW->word[0] =  or16(emu, EW->word[0], tmp16u); break;
            case 2: EW->word[0] = adc16(emu, EW->word[0], tmp16u); break;
            case 3: EW->word[0] = sbb16(emu, EW->word[0], tmp16u); break;
            case 4: EW->word[0] = and16(emu, EW->word[0], tmp16u); break;
            case 5: EW->word[0] = sub16(emu, EW->word[0], tmp16u); break;
            case 6: EW->word[0] = xor16(emu, EW->word[0], tmp16u); break;
            case 7:               cmp16(emu, EW->word[0], tmp16u); break;
        }
        break;

    case 0x85:                              /* TEST Ew,Gw */
        nextop = F8;
        GETEW(0);
        GETGW;
        test16(emu, EW->word[0], GW->word[0]);
        break;

    case 0x87:                              /* XCHG Ew,Gw */
        nextop = F8;
        GETEW(0);
        GETGW;
        if(rex.w) {
            tmp64u = GW->q[0];
            GW->q[0] = EW->q[0];
            EW->q[0] = tmp64u;
        } else {
            tmp16u = GW->word[0];
            GW->word[0] = EW->word[0];
            EW->word[0] = tmp16u;
        }
        break;

    case 0x89:                              /* MOV Ew,Gw */
        nextop = F8;
        GETEW(0);
        GETGW;
        if(rex.w)
            EW->q[0] = GW->q[0];
        else
            EW->word[0] = GW->word[0];
        break;

    case 0x8B:                              /* MOV Gw,Ew */
        nextop = F8;
        GETEW(0);
        GETGW;
        if(rex.w)
            GW->q[0] = EW->q[0];
        else
            GW->word[0] = EW->word[0];
        break;
    case 0x8C:                      /* MOV Ed, Seg */
        nextop = F8;
        GETEW(0);
        if(rex.w)
            EW->q[0] = emu->segs[((nextop&0x38)>>3)];
        else
            EW->word[0] = emu->segs[((nextop&0x38)>>3)];
        break;
    case 0x8D:                              /* LEA Gw,M */
        nextop = F8;
        GETGD;
        tmp64u = GETEA(0);
        if(rex.w)
            GD->q[0] = tmp64u;
        else
            GD->word[0] = (uint16_t)tmp64u;
        break;
    case 0x8E:                               /* MOV Seg,Ew */
        nextop = F8;
        GETEW(0);
        emu->segs[((nextop&0x38)>>3)] = EW->word[0];
        emu->segs_serial[((nextop&0x38)>>3)] = 0;
        break;
    case 0x8F:                              /* POP Ew */
        nextop = F8;
        GETEW(0);
        EW->word[0] = Pop16(emu);
        break;
    case 0x90:                      /* NOP or XCHG R8d, AX*/
    case 0x91:
    case 0x92:
    case 0x93:
    case 0x94:
    case 0x95:
    case 0x96:
    case 0x97:                      /* XCHG reg,AX */
        tmp8u = _AX+(opcode&7)+(rex.b<<3);
        if(tmp8u!=_AX) {
            if(rex.w) {
                tmp64u = R_RAX;
                R_RAX = emu->regs[tmp8u].q[0];
                emu->regs[tmp8u].q[0] = tmp64u;
            } else {
                tmp16u = R_AX;
                R_AX = emu->regs[tmp8u].word[0];
                emu->regs[tmp8u].word[0] = tmp16u;
            }
        }
        break;

    case 0x98:                               /* CBW */
        emu->regs[_AX].sword[0] = emu->regs[_AX].sbyte[0];
        break;
    case 0x99:                              /* CWD */
        R_DX=((R_AX & 0x8000)?0xFFFF:0x0000);
        break;

    case 0x9C:                              /* PUSHFW */
        CHECK_FLAGS(emu);
        Push16(emu, (uint16_t)emu->eflags.x64);
        break;
    case 0x9D:                              /* POPFW */
        CHECK_FLAGS(emu);
        emu->eflags.x64 &=0xffff0000;
        emu->eflags.x64 |= (Pop16(emu) & 0x3F7FD7) | 0x2;
        break;

    case 0xA1:                      /* MOV EAX,Od */
        if(rex.is32bits) {
            R_AX = *(uint16_t*)(uintptr_t)F32;
        } else {
            if(rex.w)
                R_RAX = *(uint64_t*)F64;
            else
                R_AX = *(uint16_t*)F64;
        }
        break;

    case 0xA3:                      /* MOV Od,EAX */
        if(rex.is32bits) {
            *(uint16_t*)(uintptr_t)F32 = R_AX;
        } else {
            if(rex.w)
                *(uint64_t*)F64 = R_RAX;
            else
                *(uint16_t*)F64 = R_AX;
        }
        break;
    case 0xA4:                      /* (REP) MOVSB */
        tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
        tmp64u = (rep)?R_RCX:1L;
        while(tmp64u) {
            #ifndef TEST_INTERPRETER
            *(uint8_t*)R_RDI = *(uint8_t*)R_RSI;
            #endif
            R_RDI += tmp8s;
            R_RSI += tmp8s;
            --tmp64u;
        }
        if(rep)
            R_RCX = tmp64u;
        break;
    case 0xA5:              /* (REP) MOVSW */
        tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
        tmp64u = (rep)?R_RCX:1L;
        if(rex.w) {
            tmp8s *= 8;
            while(tmp64u) {
                --tmp64u;
                *(uint64_t*)R_RDI = *(uint64_t*)R_RSI;
                R_RDI += tmp8s;
                R_RSI += tmp8s;
            }
        } else {
            tmp8s *= 2;
            while(tmp64u) {
                --tmp64u;
                *(uint16_t*)R_RDI = *(uint16_t*)R_RSI;
                R_RDI += tmp8s;
                R_RSI += tmp8s;
            }
        }
        if(rep)
            R_RCX = tmp64u;
        break;

    case 0xA7:                      /* (REPZ/REPNE) CMPSW */
        if(rex.w)
            tmp8s = ACCESS_FLAG(F_DF)?-8:+8;
        else
            tmp8s = ACCESS_FLAG(F_DF)?-2:+2;
        switch(rep) {
            case 1:
                if(R_RCX) {
                    if(rex.w) {
                        while(R_RCX) {
                            --R_RCX;
                            tmp64u3 = *(uint64_t*)R_RDI;
                            tmp64u2 = *(uint64_t*)R_RSI;
                            R_RDI += tmp8s;
                            R_RSI += tmp8s;
                            if(tmp64u3==tmp64u2)
                                break;
                        }
                        cmp64(emu, tmp64u2, tmp64u3);
                    } else {
                        while(R_RCX) {
                            --R_RCX;
                            tmp16u  = *(uint16_t*)R_RDI;
                            tmp16u2 = *(uint16_t*)R_RSI;
                            R_RDI += tmp8s;
                            R_RSI += tmp8s;
                            if(tmp16u==tmp16u2)
                                break;
                        }
                        cmp16(emu, tmp16u2, tmp16u);
                    }
                }
                break;
            case 2:
                if(R_RCX) {
                    if(rex.w) {
                        while(R_RCX) {
                            --R_RCX;
                            tmp64u3 = *(uint64_t*)R_RDI;
                            tmp64u2 = *(uint64_t*)R_RSI;
                            R_RDI += tmp8s;
                            R_RSI += tmp8s;
                            if(tmp64u3!=tmp64u2)
                                break;
                        }
                        cmp64(emu, tmp64u2, tmp64u3);
                    } else {
                        while(R_RCX) {
                            --R_RCX;
                            tmp16u  = *(uint16_t*)R_RDI;
                            tmp16u2 = *(uint16_t*)R_RSI;
                            R_RDI += tmp8s;
                            R_RSI += tmp8s;
                            if(tmp16u!=tmp16u2)
                                break;
                        }
                        cmp16(emu, tmp16u2, tmp16u);
                    }
                }
                break;
            default:
                if(rex.w) {
                    tmp64u  = *(uint64_t*)R_RDI;
                    tmp64u2 = *(uint64_t*)R_RSI;
                    R_RDI += tmp8s;
                    R_RSI += tmp8s;
                    cmp64(emu, tmp64u2, tmp64u);
                } else {
                    tmp16u  = *(uint16_t*)R_RDI;
                    tmp16u2 = *(uint16_t*)R_RSI;
                    R_RDI += tmp8s;
                    R_RSI += tmp8s;
                    cmp16(emu, tmp16u2, tmp16u);
                }
        }
        break;
    
    case 0xA9:                             /* TEST AX,Iw */
        if(rex.w)
            test64(emu, R_RAX, F32S64);
        else
            test16(emu, R_AX, F16);
        break;

    case 0xAB:                      /* (REP) STOSW */
        if(rex.w)
            tmp8s = ACCESS_FLAG(F_DF)?-8:+8;
        else
            tmp8s = ACCESS_FLAG(F_DF)?-2:+2;
        tmp64u = (rep)?R_RCX:1L;
        if((rex.w))
            while(tmp64u) {
                #ifndef TEST_INTERPRETER
                *(uint64_t*)R_RDI = R_RAX;
                #endif
                R_RDI += tmp8s;
                --tmp64u;
            }
        else
            while(tmp64u) {
                #ifndef TEST_INTERPRETER
                *(uint16_t*)R_RDI = R_AX;
                #endif
                R_RDI += tmp8s;
                --tmp64u;
            }
        if(rep)
            R_RCX = tmp64u;
        break;
    case 0xAD:                      /* (REP) LODSW */
        if(rex.w)
            tmp8s = ACCESS_FLAG(F_DF)?-8:+8;
        else
            tmp8s = ACCESS_FLAG(F_DF)?-2:+2;
        tmp64u = (rep)?R_RCX:1L;
        if((rex.w))
            while(tmp64u) {
                R_RAX = *(uint64_t*)R_RSI;
                R_RSI += tmp8s;
                --tmp64u;
            }
        else
            while(tmp64u) {
                R_AX = *(uint16_t*)R_RSI;
                R_RSI += tmp8s;
                --tmp64u;
            }
        if(rep)
            R_RCX = tmp64u;
        break;

    case 0xAF:                      /* (REPZ/REPNE) SCASW */
        if(rex.w)
            tmp8s = ACCESS_FLAG(F_DF)?-8:+8;
        else
            tmp8s = ACCESS_FLAG(F_DF)?-2:+2;
        switch(rep) {
            case 1:
                if(R_RCX) {
                    if(rex.w) {
                        while(R_RCX) {
                            --R_RCX;
                            tmp64u2 = *(uint64_t*)R_RDI;
                            R_RDI += tmp8s;
                            if(R_RAX==tmp64u2)
                                break;
                        }
                        cmp64(emu, R_RAX, tmp64u2);
                    } else {
                        while(R_RCX) {
                            --R_RCX;
                            tmp16u = *(uint16_t*)R_RDI;
                            R_RDI += tmp8s;
                            if(R_AX==tmp16u)
                                break;
                        }
                        cmp16(emu, R_AX, tmp16u);
                    }
                }
                break;
            case 2:
                if(R_RCX) {
                    if(rex.w) {
                        while(R_RCX) {
                            --R_RCX;
                            tmp64u2 = *(uint64_t*)R_RDI;
                            R_RDI += tmp8s;
                            if(R_RAX!=tmp64u2)
                                break;
                        }
                        cmp64(emu, R_RAX, tmp64u2);
                    } else {
                        while(R_RCX) {
                            --R_RCX;
                            tmp16u = *(uint16_t*)R_RDI;
                            R_RDI += tmp8s;
                            if(R_AX!=tmp16u)
                                break;
                        }
                        cmp16(emu, R_AX, tmp16u);
                    }
                }
                break;
            default:
                if(rex.w)
                    cmp64(emu, R_RAX, *(uint64_t*)R_RDI);
                else
                    cmp16(emu, R_AX, *(uint16_t*)R_RDI);
                R_RDI += tmp8s;
        }
        break;

    case 0xB8:                              /* MOV AX,Iw */
    case 0xB9:                              /* MOV CX,Iw */
    case 0xBA:                              /* MOV DX,Iw */
    case 0xBB:                              /* MOV BX,Iw */
    case 0xBC:                              /*    ...     */
    case 0xBD:
    case 0xBE:
    case 0xBF:
        if(rex.w)
            emu->regs[(opcode&7)+(rex.b<<3)].q[0] = F64;
        else
            emu->regs[(opcode&7)+(rex.b<<3)].word[0] = F16;
        break;

    case 0xC1:                              /* GRP2 Ew,Ib */
        nextop = F8;
        GETEW(1);
        tmp8u = F8 /*& 0x1f*/;
        switch((nextop>>3)&7) {
            case 0: EW->word[0] = rol16(emu, EW->word[0], tmp8u); break;
            case 1: EW->word[0] = ror16(emu, EW->word[0], tmp8u); break;
            case 2: EW->word[0] = rcl16(emu, EW->word[0], tmp8u); break;
            case 3: EW->word[0] = rcr16(emu, EW->word[0], tmp8u); break;
            case 4:
            case 6: EW->word[0] = shl16(emu, EW->word[0], tmp8u); break;
            case 5: EW->word[0] = shr16(emu, EW->word[0], tmp8u); break;
            case 7: EW->word[0] = sar16(emu, EW->word[0], tmp8u); break;
        }
        break;

    case 0xC7:                              /* MOV Ew,Iw */
        nextop = F8;
        GETEW(2);
        EW->word[0] = F16;
        break;
    case 0xC8:                      /* ENTER Iw,Ib */
        tmp16u = F16;
        tmp8u = (F8) & 0x1f;
        tmp64u = R_RBP;
        Push16(emu, R_BP);
        R_RBP = R_RSP;
        if (tmp8u) {
            for (tmp8u2 = 1; tmp8u2 < tmp8u; tmp8u2++) {
                tmp64u -= 2;
                Push16(emu, *((uint16_t*)tmp64u));
            }
            Push16(emu, R_BP);
        }
        R_RSP -= tmp16u;
        break;
    case 0xC9:                      /* LEAVE */
        R_RSP = R_RBP;
        R_BP = Pop16(emu);
        break;

    case 0xD1:                              /* GRP2 Ew,1  */
    case 0xD3:                              /* GRP2 Ew,CL */
        nextop = F8;
        GETEW(0);
        tmp8u=(opcode==0xD3)?R_CL:1;
        if(rex.w) {
            switch((nextop>>3)&7) {
                case 0: EW->q[0] = rol64(emu, EW->q[0], tmp8u); break;
                case 1: EW->q[0] = ror64(emu, EW->q[0], tmp8u); break;
                case 2: EW->q[0] = rcl64(emu, EW->q[0], tmp8u); break;
                case 3: EW->q[0] = rcr64(emu, EW->q[0], tmp8u); break;
                case 4: 
                case 6: EW->q[0] = shl64(emu, EW->q[0], tmp8u); break;
                case 5: EW->q[0] = shr64(emu, EW->q[0], tmp8u); break;
                case 7: EW->q[0] = sar64(emu, EW->q[0], tmp8u); break;
            }
        } else {
            switch((nextop>>3)&7) {
                case 0: EW->word[0] = rol16(emu, EW->word[0], tmp8u); break;
                case 1: EW->word[0] = ror16(emu, EW->word[0], tmp8u); break;
                case 2: EW->word[0] = rcl16(emu, EW->word[0], tmp8u); break;
                case 3: EW->word[0] = rcr16(emu, EW->word[0], tmp8u); break;
                case 4: 
                case 6: EW->word[0] = shl16(emu, EW->word[0], tmp8u); break;
                case 5: EW->word[0] = shr16(emu, EW->word[0], tmp8u); break;
                case 7: EW->word[0] = sar16(emu, EW->word[0], tmp8u); break;
            }
        }
        break;

    case 0xD9:                              /* x87 opcdes */
        #ifdef TEST_INTERPRETER
        return Test66D9(test, rex, addr);
        #else
        return Run66D9(emu, rex, addr);
        #endif

    case 0xDD:                              /* x87 opcdes */
        #ifdef TEST_INTERPRETER
        return Test66DD(test, rex, addr);
        #else
        return Run66DD(emu, rex, addr);
        #endif

    case 0xE8:                              /* CALL Id */
        tmp32s = F32S; // call is relative
        if(rex.is32bits)
            Push32(emu, addr);
        else
            Push64(emu, addr);
        addr += tmp32s;
        break;

    case 0xF0:                              /* LOCK: */
        #ifdef TEST_INTERPRETER
        return Test66F0(test, rex, addr);
        #else
        return Run66F0(emu, rex, addr);
        #endif

    case 0xF7:                      /* GRP3 Ew(,Iw) */
        nextop = F8;
        tmp8u = (nextop>>3)&7;
        if(rex.w) {
            GETED((tmp8u<2)?4:0);
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
                case 1:                 /* TEST Ew,Iw */
                    GETEW(2);
                    test16(emu, EW->word[0], F16);
                    break;
                case 2:                 /* NOT Ew */
                    GETEW(0);
                    EW->word[0] = not16(emu, EW->word[0]);
                    break;
                case 3:                 /* NEG Ew */
                    GETEW(0);
                    EW->word[0] = neg16(emu, EW->word[0]);
                    break;
                case 4:                 /* MUL AX,Ew */
                    GETEW(0);
                    mul16(emu, EW->word[0]);
                    break;
                case 5:                 /* IMUL AX,Ew */
                    GETEW(0);
                    imul16_eax(emu, EW->word[0]);
                    break;
                case 6:                 /* DIV Ew */
                    GETEW(0);
                    div16(emu, EW->word[0]);
                    break;
                case 7:                 /* IDIV Ew */
                    GETEW(0);
                    idiv16(emu, EW->word[0]);
                    break;
            }
        }
        break;
    case 0xF8:                      /* CLC */
        CHECK_FLAGS(emu);
        CLEAR_FLAG(F_CF);
        break;
    case 0xF9:                      /* STC */
        CHECK_FLAGS(emu);
        SET_FLAG(F_CF);
        break;

    case 0xFF:                      /* GRP 5 Ew */
        nextop = F8;
        GETEW(0);
        switch((nextop>>3)&7) {
            case 0:                 /* INC Ed */
                EW->word[0] = inc16(emu, EW->word[0]);
                break;
            case 1:                 /* DEC Ed */
                EW->word[0] = dec16(emu, EW->word[0]);
                break;
            case 2:                 /* CALL NEAR Ed */
                if(rex.is32bits) {
                    tmp64u = (uintptr_t)getAlternate((void*)(uintptr_t)ED->dword[0]);
                    Push32(emu, addr);
                } else {
                    tmp64u = (uintptr_t)getAlternate((void*)ED->q[0]);
                    Push64(emu, addr);
                }
                addr = tmp64u;
                break;
           case 6:                  /* Push Ew */
                Push16(emu, EW->word[0]);
                break;
            default:
                    printf_log(LOG_NONE, "Illegal Opcode %p: 66 %02X %02X %02X %02X %02X %02X\n",(void*)R_RIP, opcode, nextop, PK(2), PK(3), PK(4), PK(5));
                    emu->quit=1;
                    emu->error |= ERR_ILLEGAL;
                    return 0;
        }
        break;

    default:
        return 0;
    }
    return addr;
}
