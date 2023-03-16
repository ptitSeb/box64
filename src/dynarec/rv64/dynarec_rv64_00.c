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
#include "dynarec_native.h"
#include "custommem.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "dynarec_rv64_helper.h"

int isSimpleWrapper(wrapper_t fun);

uintptr_t dynarec64_00(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    uint8_t nextop, opcode;
    uint8_t gd, ed;
    int8_t i8;
    int32_t i32, tmp;
    int64_t i64, j64;
    uint8_t u8;
    uint8_t gb1, gb2, eb1, eb2;
    uint32_t u32;
    uint64_t u64;
    uint8_t wback, wb1, wb2, wb;
    int64_t fixedaddress;
    int lock;
    int cacheupd = 0;

    opcode = F8;
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(j64);
    MAYUSE(wb);
    MAYUSE(lock);
    MAYUSE(cacheupd);

    switch(opcode) {
        case 0x01:
            INST_NAME("ADD Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_add32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            WBACK;
            break;

        case 0x0F:
            switch(rep) {
            case 2:
                addr = dynarec64_F30F(dyn, addr, ip, ninst, rex, ok, need_epilog);
                break;
            default:
                DEFAULT;
            }
            break;

        case 0x29:
            INST_NAME("SUB Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sub32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            WBACK;
            break;

        case 0x31:
            INST_NAME("XOR Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_xor32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x39:
            INST_NAME("CMP Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_cmp32(dyn, ninst, rex, ed, gd, x3, x4, x5, x6);
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
            SD(gd, xRSP, -8);
            SUBI(xRSP, xRSP, 8);
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
            LD(gd, xRSP, 0);
            if(gd!=xRSP) {
                ADDI(xRSP, xRSP, 8);
            }
            break;
        case 0x80:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 7: // CMP
                    INST_NAME("CMP Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    if(u8) {
                        ADDI(x2, xZR, u8);
                        emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    } else {
                        emit_cmp8_0(dyn, ninst, x1, x3, x4);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x81:
        case 0x83:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: // ADD
                    if(opcode==0x81) {INST_NAME("ADD Ed, Id");} else {INST_NAME("ADD Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_add32c(dyn, ninst, rex, ed, i64, x3, x4, x5, x6);
                    WBACK;
                    break;
                case 5:
                    if(opcode==0x81) {INST_NAME("SUB Ed, Id");} else {INST_NAME("SUB Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_sub32c(dyn, ninst, rex, ed, i64, x3, x4, x5, x6);
                    WBACK;
                    break;
                case 7: // CMP
                    if(opcode==0x81) {INST_NAME("CMP Ed, Id");} else {INST_NAME("CMP Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    if(i64) {
                        MOV64xw(x2, i64);
                        emit_cmp32(dyn, ninst, rex, ed, x2, x3, x4, x5, x6);
                    } else
                        emit_cmp32_0(dyn, ninst, rex, ed, x3, x4);
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0x85:
            INST_NAME("TEST Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop=F8;
            GETGD;
            GETED(0);
            emit_test32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            break;

        case 0x89:
            INST_NAME("MOV Ed, Gd");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                MVxw(xRAX+(nextop&7)+(rex.b<<3), gd);
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                SDxw(gd, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;

        case 0x8B:
            INST_NAME("MOV Gd, Ed");
            nextop=F8;
            GETGD;
            if(MODREG) {
                MVxw(gd, xRAX+(nextop&7)+(rex.b<<3));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                SMREADLOCK(lock);
                LDxw(gd, ed, fixedaddress);
            }
            break;

        case 0x8D:
            INST_NAME("LEA Gd, Ed");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg? that's an invalid operation
                DEFAULT;
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, gd, x1, &fixedaddress, rex, NULL, 0, 0);
                if(gd!=ed) {    // it's sometimes used as a 3 bytes NOP
                    MV(gd, ed);
                }
                else if(!rex.w) {
                    ZEROUP(gd);   //truncate the higher 32bits as asked
                }
            }
            break;

        case 0x90:
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x96:
        case 0x97:
            gd = xRAX+(opcode&0x07)+(rex.b<<3);
            if(gd==xRAX) {
                INST_NAME("NOP");
            } else {
                INST_NAME("XCHG EAX, Reg");
                MVxw(x2, xRAX);
                MVxw(xRAX, gd);
                MVxw(gd, x2);
            }
            break;
        case 0x98:
            INST_NAME("CWDE");
            if(rex.w) {
                SEXT_W(xRAX, xRAX);
            } else {
                SLLI(xRAX, xRAX, 16);
                SRAIW(xRAX, xRAX, 16);
                ZEROUP(xRAX);
            }
            break;


        case 0xB8:
        case 0xB9:
        case 0xBA:
        case 0xBB:
        case 0xBC:
        case 0xBD:
        case 0xBE:
        case 0xBF:
            INST_NAME("MOV Reg, Id");
            gd = xRAX+(opcode&7)+(rex.b<<3);
            if(rex.w) {
                u64 = F64;
                MOV64x(gd, u64);
            } else {
                u32 = F32;
                MOV32w(gd, u32);
            }
            break;
        case 0xC1:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 5:
                    INST_NAME("SHR Ed, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETED(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    emit_shr32c(dyn, ninst, rex, ed, u8, x3, x4);
                    if(u8) { WBACK; }
                    break;
                case 7:
                    INST_NAME("SAR Ed, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETED(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    emit_sar32c(dyn, ninst, rex, ed, u8, x3, x4);
                    if(u8) { WBACK; }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xC2:
            INST_NAME("RETN");
            //SETFLAGS(X_ALL, SF_SET);    // Hack, set all flags (to an unknown state...)
            if(box64_dynarec_safeflags) {
                READFLAGS(X_PEND);  // lets play safe here too
            }
            BARRIER(BARRIER_FLOAT);
            i32 = F16;
            retn_to_epilog(dyn, ninst, i32);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xC3:
            INST_NAME("RET");
            // SETFLAGS(X_ALL, SF_SET);    // Hack, set all flags (to an unknown state...)
            if(box64_dynarec_safeflags) {
                READFLAGS(X_PEND);  // so instead, force the defered flags, so it's not too slow, and flags are not lost
            }
            BARRIER(BARRIER_FLOAT);
            ret_to_epilog(dyn, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;

        case 0xC6:
            INST_NAME("MOV Eb, Ib");
            nextop=F8;
            if(MODREG) {   // reg <= u8
                u8 = F8;
                if(!rex.rex) {
                    ed = (nextop&7);
                    eb1 = xRAX+(ed&3);  // Ax, Cx, Dx or Bx
                    eb2 = (ed&4)>>2;    // L or H
                } else {
                    eb1 = xRAX+(nextop&7)+(rex.b<<3);
                    eb2 = 0;
                }

                if (eb2) {
                    // load a mask to x3 (ffffffffffff00ff)
                    LUI(x3, 0xffff0);
                    ADDI(x3, x3, 0xff);
                    // apply mask
                    AND(eb1, eb1, x3);
                    ADDI(x4, xZR, u8);
                    SLLI(x4, x4, 8);
                    OR(eb1, eb1, x4);
                } else {
                    SRLI(eb1, eb1, 8);
                    SLLI(eb1, eb1, 8);
                    ADDI(eb1, eb1, u8);
                }
            } else {                    // mem <= u8
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, &lock, 0, 1);
                u8 = F8;
                if(u8) {
                    ADDI(x3, xZR, u8);
                    ed = x3;
                } else
                    ed = xZR;
                SB(ed, wback, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0xC7:
            INST_NAME("MOV Ed, Id");
            nextop=F8;
            if(MODREG) {    // reg <= i32
                i64 = F32S;
                ed = xRAX+(nextop&7)+(rex.b<<3);
                MOV64xw(ed, i64);
            } else {        // mem <= i32
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, &lock, 0, 4);
                i64 = F32S;
                if(i64) {
                    MOV64xw(x3, i64);
                    ed = x3;
                } else
                    ed = xZR;
                SDxw(ed, wback, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0xCC:
            SETFLAGS(X_ALL, SF_SET);    // Hack, set all flags (to an unknown state...)
            if(PK(0)=='S' && PK(1)=='C') {
                addr+=2;
                BARRIER(BARRIER_FLOAT);
                INST_NAME("Special Box64 instruction");
                if((PK64(0)==0))
                {
                    addr+=8;
                    MESSAGE(LOG_DEBUG, "Exit x64 Emu\n");
                    //GETIP(ip+1+2);    // no use
                    //STORE_XEMU_REGS(xRIP);    // no need, done in epilog
                    MOV64x(x1, 1);
                    SW(x1, xEmu, offsetof(x64emu_t, quit));
                    *ok = 0;
                    *need_epilog = 1;
                } else {
                    MESSAGE(LOG_DUMP, "Native Call to %s\n", GetNativeName(GetNativeFnc(ip)));
                    //x87_forget(dyn, ninst, x3, x4, 0);
                    //sse_purge07cache(dyn, ninst, x3);
                    tmp = isSimpleWrapper(*(wrapper_t*)(addr));
                    if((box64_log<2 && !cycle_log) && tmp) {
                        //GETIP(ip+3+8+8); // read the 0xCC
                        call_n(dyn, ninst, *(void**)(addr+8), tmp);
                        addr+=8+8;
                    } else {
                        GETIP(ip+1); // read the 0xCC
                        STORE_XEMU_CALL();
                        ADDI(x1, xEmu, (uint32_t)offsetof(x64emu_t, ip)); // setup addr as &emu->ip
                        CALL_S(x64Int3, -1);
                        LOAD_XEMU_CALL();
                        addr+=8+8;
                        TABLE64(x3, addr); // expected return address
                        BNE_MARK(xRIP, x3);
                        LW(w1, xEmu, offsetof(x64emu_t, quit));
                        CBZ_NEXT(w1);
                        MARK;
                        LOAD_XEMU_REM();
                        jump_to_epilog(dyn, 0, xRIP, ninst);
                    }
                }
            } else {
                #if 1
                INST_NAME("INT 3");
                // check if TRAP signal is handled
                LD(x1, xEmu, offsetof(x64emu_t, context));
                MOV64x(x2, offsetof(box64context_t, signals[SIGTRAP]));
                ADD(x2, x2, x1);
                LD(x3, x2, 0);
                CBNZ_NEXT(x3);
                MOV64x(x1, SIGTRAP);
                CALL_(raise, -1, 0);
                break;
                #else
                DEFAULT;
                #endif
            }
            break;
        case 0xD1:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 5:
                    INST_NAME("SHR Ed, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETED(1);
                    emit_shr32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ed, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETED(1);
                    emit_sar32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                default:
                    DEFAULT;
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
            #if STEP < 2
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
                    BARRIER(BARRIER_FULL);
                    //BARRIER_NEXT(BARRIER_FULL);
                    if(dyn->last_ip && (addr-dyn->last_ip<0x1000)) {
                        ADDI(x2, xRIP, addr-dyn->last_ip);
                    } else {
                        TABLE64(x2, addr);
                    }
                    PUSH1(x2);
                    MESSAGE(LOG_DUMP, "Native Call to %s (retn=%d)\n", GetNativeName(GetNativeFnc(dyn->insts[ninst].natcall-1)), dyn->insts[ninst].retn);
                    // calling a native function
                    //sse_purge07cache(dyn, ninst, x3);     // TODO: chack the fpxx to purge/save when implemented
                    if((box64_log<2 && !cycle_log) && dyn->insts[ninst].natcall) {
                        tmp=isSimpleWrapper(*(wrapper_t*)(dyn->insts[ninst].natcall+2));
                        if(tmp>1 || tmp<0)
                            tmp=0;  // float paramters not ready!
                    } else
                        tmp=0;
                    if((box64_log<2 && !cycle_log) && dyn->insts[ninst].natcall && tmp) {
                        //GETIP(ip+3+8+8); // read the 0xCC
                        call_n(dyn, ninst, *(void**)(dyn->insts[ninst].natcall+2+8), tmp);
                        POP1(xRIP);       // pop the return address
                        dyn->last_ip = addr;
                    } else {
                        GETIP_(dyn->insts[ninst].natcall); // read the 0xCC already
                        STORE_XEMU_CALL();
                        ADDI(x1, xEmu, (uint32_t)offsetof(x64emu_t, ip)); // setup addr as &emu->ip
                        CALL_S(x64Int3, -1);
                        LOAD_XEMU_CALL();
                        TABLE64(x3, dyn->insts[ninst].natcall);
                        ADDI(x3, x3, 2+8+8);
                        BNE_MARK(xRIP, x3);    // Not the expected address, exit dynarec block
                        POP1(xRIP);       // pop the return address
                        if(dyn->insts[ninst].retn) {
                            if(dyn->insts[ninst].retn<0x1000) {
                                ADDI(xRSP, xRSP, dyn->insts[ninst].retn);
                            } else {
                                MOV64x(x3, dyn->insts[ninst].retn);
                                ADD(xRSP, xRSP, x3);
                            }
                        }
                        TABLE64(x3, addr);
                        BNE_MARK(xRIP, x3);    // Not the expected address again
                        LW(w1, xEmu, offsetof(x64emu_t, quit));
                        CBZ_NEXT(w1);
                        MARK;
                        LOAD_XEMU_REM();    // load remaining register, has they have changed
                        jump_to_epilog(dyn, 0, xRIP, ninst);
                        dyn->last_ip = addr;
                    }
                    break;
                default:
                    if((box64_dynarec_safeflags>1) || (ninst && dyn->insts[ninst-1].x64.set_flags)) {
                        READFLAGS(X_PEND);  // that's suspicious
                    } else {
                        SETFLAGS(X_ALL, SF_SET);    // Hack to set flags to "dont'care" state
                    }
                    // regular call
                    //BARRIER_NEXT(1);
                    if(box64_dynarec_callret && box64_dynarec_bigblock>1) {
                        BARRIER(BARRIER_FULL);
                    } else {
                        BARRIER(BARRIER_FLOAT);
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    TABLE64(x2, addr);
                    PUSH1(x2);
                    // TODO: Add support for CALLRET optim
                    /*if(box64_dynarec_callret) {
                        // Push actual return address
                        if(addr < (dyn->start+dyn->isize)) {
                            // there is a next...
                            j64 = (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->native_size)):0;
                            ADR_S20(x4, j64);
                        } else {
                            j64 = getJumpTableAddress64(addr);
                            TABLE64(x4, j64);
                            LDR(x4, x4, 0);
                        }
                        PUSH1(x4);
                        PUSH1(x2);
                    } else */ //CALLRET optim disable for now.
                    {
                        *ok = 0;
                        *need_epilog = 0;
                    }
                    if(addr+i32==0) {   // self modifying code maybe? so use indirect address fetching
                        TABLE64(x4, addr-4);
                        LD(x4, x4, 0);
                        jump_to_next(dyn, 0, x4, ninst);
                    } else
                        jump_to_next(dyn, addr+i32, 0, ninst);
                    break;
            }
            break;

        default:
            DEFAULT;
    }

     return addr;
}
