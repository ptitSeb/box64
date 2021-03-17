#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "bridge.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_arm64.h"
#include "dynarec_arm64_private.h"
#include "arm64_printer.h"

#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"

uintptr_t dynarec64_00(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t nextop, opcode;
    uint8_t gd, ed;
    int8_t i8;
    int32_t i32, j32, tmp;
    uint8_t u8;
    uint8_t gb1, gb2, eb1, eb2;
    uint32_t u32;
    uint8_t wback, wb1, wb2;
    int fixedaddress;
    rex_t rex;
    int rep;    // 0 none, 1=F2 prefix, 2=F3 prefix

    opcode = F8;
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(tmp);
    MAYUSE(j32);

    rep = 0;
    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }

    switch(opcode) {

        case 0x29:
            INST_NAME("SUB Ed, Gd");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sub32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            WBACK;
            break;

        case 0x2B:
            INST_NAME("SUB Gd, Ed");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sub32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;

        case 0x2D:
            INST_NAME("SUB EAX, Id");
            SETFLAGS(X_ALL, SF_SET);
            i32 = F32S;
            emit_sub32c(dyn, ninst, rex, xRAX, i32, x3, x4, x5);
            break;

        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
            INST_NAME("PUSH reg");
            gd = xRAX+(opcode&0x07)+(rex.b<<3);
            if(gd==xRSP) {
                MOVx(x1, gd);
                gd = x1;
            }
            PUSH1(gd);
            break;
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5E:
        case 0x5F:
            INST_NAME("POP reg");
            gd = xRAX+(opcode&0x07)+(rex.b<<3);
            if(gd == xRSP) {
                POP1(x1);
                MOVx(gd, x1);
            } else {
                POP1(gd);
            }
            break;

        #define GO(GETFLAGS, NO, YES, F)    \
            READFLAGS(F);                   \
            i8 = F8S;   \
            BARRIER(2); \
            JUMP(addr+i8);\
            GETFLAGS;   \
            if(dyn->insts) {    \
                if(dyn->insts[ninst].x64.jmp_insts==-1) {   \
                    /* out of the block */                  \
                    i32 = dyn->insts[ninst+1].address-(dyn->arm_size); \
                    Bcond(NO, i32);     \
                    jump_to_next(dyn, addr+i8, 0, ninst); \
                } else {    \
                    /* inside the block */  \
                    i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->arm_size);    \
                    Bcond(YES, i32);    \
                }   \
            }

        case 0x70:
            INST_NAME("JO ib");
            GO( TSTw_mask(xFlags, 0b010101, 0)
                , cEQ, cNE, X_OF)
            break;
        case 0x71:
            INST_NAME("JNO ib");
            GO( TSTw_mask(xFlags, 0b010101, 0)
                , cNE, cEQ, X_OF)
            break;
        case 0x72:
            INST_NAME("JC ib");
            GO( TSTw_mask(xFlags, 0, 0)
                , cEQ, cNE, X_CF)
            break;
        case 0x73:
            INST_NAME("JNC ib");
            GO( TSTw_mask(xFlags, 0, 0)
                , cNE, cEQ, X_CF)
            break;
        case 0x74:
            INST_NAME("JZ ib");
            GO( TSTw_mask(xFlags, 0b011010, 0)
                , cEQ, cNE, X_ZF)
            break;
        case 0x75:
            INST_NAME("JNZ ib");
            GO( TSTw_mask(xFlags, 0b011010, 0)
                , cNE, cEQ, X_ZF)
            break;
        case 0x76:
            INST_NAME("JBE ib");
            GO( MOV32w(x1, (1<<F_CF)|(1<<F_ZF));
                TSTw_REG(xFlags, x1)
                , cEQ, cNE, X_CF|X_ZF)
            break;
        case 0x77:
            INST_NAME("JNBE ib");
            GO( MOV32w(x1, (1<<F_CF)|(1<<F_ZF));
                TSTw_REG(xFlags, x1)
                , cNE, cEQ, X_CF|X_ZF)
            break;
        case 0x78:
            INST_NAME("JS ib");
            GO( TSTw_mask(xFlags, 0b011001, 0)  // 0X80
                , cEQ, cNE, X_SF)
            break;
        case 0x79:
            INST_NAME("JNS ib");
            GO( TSTw_mask(xFlags, 0b011001, 0)
                , cNE, cEQ, X_SF)
            break;
        case 0x7A:
            INST_NAME("JP ib");
            GO( TSTw_mask(xFlags, 0b011110, 0)
                , cEQ, cNE, X_PF)
            break;
        case 0x7B:
            INST_NAME("JNP ib");
            GO( TSTw_mask(xFlags, 0b011110, 0)
                , cNE, cEQ, X_PF)
            break;
        case 0x7C:
            INST_NAME("JL ib");
            GO( EORw_REG_LSL(x1, xFlags, xFlags, F_OF-F_SF);
                TSTw_mask(x1, 0b010101, 0)
                , cEQ, cNE, X_SF|X_OF)
            break;
        case 0x7D:
            INST_NAME("JGE ib");
            GO( EORw_REG_LSL(x1, xFlags, xFlags, F_OF-F_SF);
                TSTw_mask(x1, 0b010101, 0)
                , cNE, cEQ, X_SF|X_OF)
            break;
        case 0x7E:
            INST_NAME("JLE ib");
            GO( EORw_REG_LSL(x1, xFlags, xFlags, F_OF-F_SF);
                ORRw_REG_LSL(x1, x1, xFlags, F_OF-F_ZF);
                TSTw_mask(x1, 0b010101, 0)
                , cEQ, cNE, X_SF|X_OF|X_ZF)
            break;
        case 0x7F:
            INST_NAME("JG ib");
            GO( EORw_REG_LSL(x1, xFlags, xFlags, F_OF-F_SF);
                ORRw_REG_LSL(x1, x1, xFlags, F_OF-F_ZF);
                TSTw_mask(x1, 0b010101, 0)
                , cNE, cEQ, X_SF|X_OF|X_ZF)
            break;
        #undef GO
        
        case 0x81:
        case 0x83:
            nextop = F8;
            switch((nextop>>3)&7) {

                case 5: //SUB
                    if(opcode==0x81) {INST_NAME("SUB Ed, Id");} else {INST_NAME("SUB Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i32 = F32S; else i32 = F8S;
                    emit_sub32c(dyn, ninst, rex, ed, i32, x3, x4, x5);
                    WBACK;
                    break;

                default:
                    DEFAULT;
            }
            break;

        case 0x85:
            INST_NAME("TEST Ed, Gd");
            SETFLAGS(X_ALL, SF_SET);
            nextop=F8;
            GETGD;
            GETED(0);
            emit_test32(dyn, ninst, rex, ed, gd, x3, x5, x6);
            break;

        case 0x89:
            INST_NAME("MOV Ed, Gd");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                MOVxw(xRAX+(nextop&7)+(rex.b<<3), gd);
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, 0, 0);
                STRxw_U12(gd, ed, fixedaddress);
            }
            break;

        case 0x8B:
            INST_NAME("MOV Gd, Ed");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                MOVxw(gd, xRAX+(nextop&7)+(rex.b<<3));
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, 0, 0);
                LDRxw_U12(gd, ed, fixedaddress);
            }
            break;

        case 0x8D:
            INST_NAME("LEA Gd, Ed");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg? that's an invalid operation
                DEFAULT;
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, gd, &fixedaddress, 0, 0, rex, 0, 0);
                if(gd!=ed) {    // it's sometimes used as a 3 bytes NOP
                    MOVxw(gd, ed);
                }
            }
            break;

        case 0xE8:
            INST_NAME("CALL Id");
            i32 = F32S;
            if(addr+i32==0) {
                #if STEP == 3
                printf_log(LOG_INFO, "Warning, CALL to 0x0 at %p (%p)\n", (void*)addr, (void*)(addr-1));
                #endif
            }
            #if STEP == 0
            if(isNativeCall(dyn, addr+i32, NULL, NULL))
                tmp = 3;
            else 
                tmp = 0;
            #elif STEP < 2
            if(isNativeCall(dyn, addr+i32, &dyn->insts[ninst].natcall, &dyn->insts[ninst].retn))
                tmp = dyn->insts[ninst].pass2choice = 3;
            else 
                tmp = dyn->insts[ninst].pass2choice = 0;
            #else
                tmp = dyn->insts[ninst].pass2choice;
            #endif
            switch(tmp) {
                case 3:
                    SETFLAGS(X_ALL, SF_SET);    // Hack to set flags to "dont'care" state
                    BARRIER(1);
                    BARRIER_NEXT(1);
                    TABLE64(x2, addr);
                    PUSH1(x2);
                    MESSAGE(LOG_DUMP, "Native Call to %s (retn=%d)\n", GetNativeName(GetNativeFnc(dyn->insts[ninst].natcall-1)), dyn->insts[ninst].retn);
                    // calling a native function
                    x87_forget(dyn, ninst, x3, x4, 0);
                    sse_purge07cache(dyn, ninst, x3);
                    GETIP_(dyn->insts[ninst].natcall); // read the 0xCC already
                    STORE_XEMU_MINIMUM(xRIP);
                    CALL_S(x64Int3, -1);
                    LOAD_XEMU_MINIMUM(xRIP);
                    TABLE64(x3, dyn->insts[ninst].natcall);
                    ADDx_U12(x3, x3, 2+8+8);
                    CMPSx_REG(xRIP, x3);
                    B_MARK(cNE);    // Not the expected address, exit dynarec block
                    POP1(xRIP);   // pop the return address
                    if(dyn->insts[ninst].retn) {
                        ADDx_U12(xRSP, xRSP, dyn->insts[ninst].retn);
                    }
                    TABLE64(x3, addr);
                    CMPSx_REG(xRIP, x3);
                    B_MARK(cNE);    // Not the expected address again
                    LDRw_U12(w1, xEmu, offsetof(x64emu_t, quit));
                    CBZw_NEXT(w1);  // not quitting, so lets continue
                    MARK;
                    jump_to_epilog(dyn, 0, xRIP, ninst);
                    break;
                default:
                    if(ninst && dyn->insts && dyn->insts[ninst-1].x64.set_flags) {
                        READFLAGS(X_PEND);  // that's suspicious
                    } else {
                        SETFLAGS(X_ALL, SF_SET);    // Hack to set flags to "dont'care" state
                    }
                    // regular call
                    BARRIER(1);
                    BARRIER_NEXT(1);
                    if(!dyn->insts || ninst==dyn->size-1) {
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    TABLE64(x2, addr);
                    PUSH1(x2);
                    if(addr+i32==0) {   // self modifying code maybe? so use indirect address fetching
                        TABLE64(x4, addr-4);
                        LDRx_U12(x4, x4, 0);
                        jump_to_next(dyn, 0, x4, ninst);
                    } else
                        jump_to_next(dyn, addr+i32, 0, ninst);
                    break;
            }
            break;

        case 0xFF:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: // INC Ed
                    INST_NAME("INC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
                    GETED(0);
                    emit_inc32(dyn, ninst, rex, ed, x3, x4);
                    WBACK;
                    break;
                case 1: //DEC Ed
                    INST_NAME("DEC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
                    GETED(0);
                    emit_dec32(dyn, ninst, rex, ed, x3, x4);
                    WBACK;
                    break;
                case 2: // CALL Ed
                    INST_NAME("CALL Ed");
                    PASS2IF(ninst && dyn->insts && dyn->insts[ninst-1].x64.set_flags, 1) {
                        READFLAGS(X_PEND);          // that's suspicious
                    } else {
                        SETFLAGS(X_ALL, SF_SET);    //Hack to put flag in "don't care" state
                    }
                    GETEDx(0);
                    BARRIER(1);
                    BARRIER_NEXT(1);
                    if(!dyn->insts || ninst==dyn->size-1) {
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    GETIP(addr);
                    PUSH1(xRIP);
                    /*TABLE64(x3, addr);
                    PUSH1(x3);*/
                    jump_to_next(dyn, 0, ed, ninst);
                    break;
                case 4: // JMP Ed
                    INST_NAME("JMP Ed");
                    BARRIER(1);
                    GETEDx(0);
                    jump_to_next(dyn, 0, ed, ninst);
                    *need_epilog = 0;
                    *ok = 0;
                    break;
                case 6: // Push Ed
                    INST_NAME("PUSH Ed");
                    GETEDx(0);
                    PUSH1(ed);
                    break;

                default:
                    INST_NAME("Grp5 Ed");
                    DEFAULT;
            }
            break;

        default:
            DEFAULT;
    }
 
     return addr;
}

