#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>

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
int isRetX87Wrapper(wrapper_t fun);

uintptr_t dynarec64_00_3(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
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
        case 0xC0:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Eb, Ib");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETEB(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(rol8, ed, x3);
                    EBBACK(x5, 0);
                    break;
                case 1:
                    INST_NAME("ROR Eb, Ib");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETEB(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(ror8, ed, x3);
                    EBBACK(x5, 0);
                    break;
                case 2:
                    INST_NAME("RCL Eb, Ib");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETEB(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(rcl8, ed, x3);
                    EBBACK(x5, 0);
                    break;
                case 3:
                    INST_NAME("RCR Eb, Ib");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETEB(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(rcr8, ed, x3);
                    EBBACK(x5, 0);
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Eb, Ib");
                    GETEB(x1, 1);
                    u8 = (F8)&0x1f;
                    if(u8) {
                        SETFLAGS(X_ALL, SF_PENDING);
                        UFLAG_IF{
                            MOV32w(x4, u8); UFLAG_OP2(x4);
                        };
                        UFLAG_OP1(ed);
                        SLLIW(ed, ed, u8);
                        EBBACK(x5, 1);
                        UFLAG_RES(ed);
                        UFLAG_DF(x3, d_shl8);
                    } else {
                        NOP();
                    }
                    break;
                case 5:
                    INST_NAME("SHR Eb, Ib");
                    GETEB(x1, 1);
                    u8 = (F8)&0x1f;
                    if(u8) {
                        SETFLAGS(X_ALL, SF_PENDING);
                        UFLAG_IF{
                            MOV32w(x4, u8); UFLAG_OP2(x4);
                        };
                        UFLAG_OP1(ed);
                        if(u8) {
                            SRLIW(ed, ed, u8);
                            EBBACK(x5, 1);
                        }
                        UFLAG_RES(ed);
                        UFLAG_DF(x3, d_shr8);
                    } else {
                        NOP();
                    }
                    break;
                case 7:
                    INST_NAME("SAR Eb, Ib");
                    GETSEB(x1, 1);
                    u8 = (F8)&0x1f;
                    if(u8) {
                        SETFLAGS(X_ALL, SF_PENDING);
                        UFLAG_IF{
                            MOV32w(x4, u8); UFLAG_OP2(x4);
                        };
                        UFLAG_OP1(ed);
                        if(u8) {
                            SRAIW(ed, ed, u8);
                            EBBACK(x5, 1);
                        }
                        UFLAG_RES(ed);
                        UFLAG_DF(x3, d_sar8);
                    } else {
                        NOP();
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xC1:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ed, Ib");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET_PENDING);
                    GETED(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    emit_rol32c(dyn, ninst, rex, ed, u8, x3, x4);
                    if(u8) { WBACK; }
                    if(!wback && !rex.w) ZEROUP(ed);
                    break;
                case 1:
                    INST_NAME("ROR Ed, Ib");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET_PENDING);
                    GETED(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    emit_ror32c(dyn, ninst, rex, ed, u8, x3, x4);
                    if(u8) { WBACK; }
                    if(!wback && !rex.w) ZEROUP(ed);
                    break;
                case 2:
                    INST_NAME("RCL Ed, Ib");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    MOV32w(x2, u8);
                    GETEDW(x4, x1, 0);
                    CALL_(rex.w?((void*)rcl64):((void*)rcl32), ed, x4);
                    WBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ed, Ib");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    MOV32w(x2, u8);
                    GETEDW(x4, x1, 0);
                    CALL_(rex.w?((void*)rcr64):((void*)rcr32), ed, x4);
                    WBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETED(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    emit_shl32c(dyn, ninst, rex, ed, u8, x3, x4, x5);
                    if(u8) WBACK;
                    break;
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
            retn_to_epilog(dyn, ninst, rex, i32);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xC3:
            INST_NAME("RET");
            // SETFLAGS(X_ALL, SF_SET);    // Hack, set all flags (to an unknown state...)
            if(box64_dynarec_safeflags) {
                READFLAGS(X_PEND);  // so instead, force the deferred flags, so it's not too slow, and flags are not lost
            }
            BARRIER(BARRIER_FLOAT);
            ret_to_epilog(dyn, ninst, rex);
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
                    ORI(x3, x3, 0xff);
                    // apply mask
                    AND(eb1, eb1, x3);
                    if(u8) {
                        if((u8<<8)<2048) {
                            ADDI(x4, xZR, u8<<8);
                        } else {
                            ADDI(x4, xZR, u8);
                            SLLI(x4, x4, 8);
                        }
                        OR(eb1, eb1, x4);
                    }
                } else {
                    ANDI(eb1, eb1, 0xf00);  // mask ffffffffffffff00
                    ORI(eb1, eb1, u8);
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

        case 0xC9:
            INST_NAME("LEAVE");
            MVz(xRSP, xRBP);
            POP1z(xRBP);
            break;

        case 0xCC:
            SETFLAGS(X_ALL, SF_SET);    // Hack, set all flags (to an unknown state...)
            SKIPTEST(x1);
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
                    x87_forget(dyn, ninst, x3, x4, 0);
                    sse_purge07cache(dyn, ninst, x3);
                    // disabling isSimpleWrapper because all signed value less than 64bits needs to be sign extended
                    // and return value needs to be cleanned up
                    tmp = 0;//isSimpleWrapper(*(wrapper_t*)(addr));
                    if(isRetX87Wrapper(*(wrapper_t*)(addr)))
                        // return value will be on the stack, so the stack depth needs to be updated
                        x87_purgecache(dyn, ninst, 0, x3, x1, x4);
                    if(tmp<0 || tmp>1)
                        tmp=0;  //TODO: removed when FP is in place
                    if((box64_log<2 && !cycle_log) && tmp) {
                        //GETIP(ip+3+8+8); // read the 0xCC
                        call_n(dyn, ninst, *(void**)(addr+8), tmp);
                        addr+=8+8;
                    } else {
                        GETIP(ip+1); // read the 0xCC
                        STORE_XEMU_CALL(x3);
                        ADDI(x1, xEmu, (uint32_t)offsetof(x64emu_t, ip)); // setup addr as &emu->ip
                        CALL_S(x64Int3, -1);
                        LOAD_XEMU_CALL();
                        addr+=8+8;
                        TABLE64(x3, addr); // expected return address
                        BNE_MARK(xRIP, x3);
                        LW(w1, xEmu, offsetof(x64emu_t, quit));
                        CBZ_NEXT(w1);
                        MARK;
                        jump_to_epilog_fast(dyn, 0, xRIP, ninst);
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
                CBZ_NEXT(x3);
                GETIP(ip);
                STORE_XEMU_CALL(x3);
                CALL(native_int3, -1);
                LOAD_XEMU_CALL();
                break;
                #else
                DEFAULT;
                #endif
            }
            break;
        case 0xCD:
            u8 = F8;
            if (box64_wine && (u8==0x2D || u8==0x2C || u8==0x29)) {
                INST_NAME("INT 29/2c/2d");
                // lets do nothing
                MESSAGE(LOG_INFO, "INT 29/2c/2d Windows interruption\n");
                GETIP(ip);
                STORE_XEMU_CALL(x3);
                MOV32w(x1, u8);
                CALL(native_int, -1);
                LOAD_XEMU_CALL();
            } else if (u8 == 0x80) {
                INST_NAME("32bits SYSCALL");
                NOTEST(x1);
                SMEND();
                GETIP(addr);
                STORE_XEMU_CALL(x3);
                CALL_S(x86Syscall, -1);
                LOAD_XEMU_CALL();
                TABLE64(x3, addr); // expected return address
                BNE_MARK(xRIP, x3);
                LW(x1, xEmu, offsetof(x64emu_t, quit));
                BEQ_NEXT(x1, xZR);
                MARK;
                LOAD_XEMU_REM(x3);
                jump_to_epilog(dyn, 0, xRIP, ninst);
            } else {
                INST_NAME("INT n");
                SETFLAGS(X_ALL, SF_SET); // Hack to set flags in "don't care" state
                GETIP(ip);
                STORE_XEMU_CALL(x3);
                CALL(native_priv, -1);
                LOAD_XEMU_CALL();
                jump_to_epilog(dyn, 0, xRIP, ninst);
                *need_epilog = 0;
                *ok = 0;
            }
            break;
        case 0xCF:
            INST_NAME("IRET");
            SETFLAGS(X_ALL, SF_SET);    // Not a hack, EFLAGS are restored
            BARRIER(BARRIER_FLOAT);
            iret_to_epilog(dyn, ninst, rex.w);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xD0:
        case 0xD2:  // TODO: Jump if CL is 0
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    if(opcode==0xD0) {
                        INST_NAME("ROL Eb, 1");
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("ROL Eb, CL");
                        ANDI(x2, xRCX, 7);
                    }
                    SETFLAGS(X_OF|X_CF, SF_PENDING);
                    GETEB(x1, 0);
                    UFLAG_OP12(ed, x2);
                    SLL(x3, ed, x2);
                    SRLI(x4, x3, 8);
                    OR(ed, x3, x4);
                    EBBACK(x5, 1);
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_rol8);
                    break;
                case 1:
                    if(opcode==0xD0) {
                        INST_NAME("ROR Eb, 1");
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("ROR Eb, CL");
                        ANDI(x2, xRCX, 7);
                    }
                    SETFLAGS(X_OF|X_CF, SF_PENDING);
                    GETEB(x1, 0);
                    UFLAG_OP12(ed, x2);
                    SRL(x3, ed, x2);
                    SLLI(x4, ed, 8);
                    SRL(x4, x4, x2);
                    OR(ed, x3, x4);
                    EBBACK(x5, 1);
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_ror8);
                    break;
                case 2:
                    if(opcode==0xD0) {
                        INST_NAME("RCL Eb, 1");
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("RCL Eb, CL");
                        ANDI(x2, xRCX, 7);
                    }
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETEB(x1, 0);
                    CALL_(rcl8, ed, x3);
                    EBBACK(x5, 0);
                    break;
                case 3:
                    if(opcode==0xD0) {
                        INST_NAME("RCR Eb, 1");
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("RCR Eb, CL");
                        ANDI(x2, xRCX, 7);
                    }
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETEB(x1, 0);
                    CALL_(rcr8, ed, x3);
                    EBBACK(x5, 0);
                    break;
                case 4:
                case 6:
                    if(opcode==0xD0) {
                        INST_NAME("SHL Eb, 1");
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("SHL Eb, CL");
                        ANDI(x2, xRCX, 7);
                    }
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEB(x1, 0);
                    UFLAG_OP12(ed, x2)
                    SLL(ed, ed, x2);
                    EBBACK(x5, 1);
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_shl8);
                    break;
                case 5:
                    if(opcode==0xD0) {
                        INST_NAME("SHR Eb, 1");
                        MOV32w(x4, 1);
                    } else {
                        INST_NAME("SHR Eb, CL");
                        ANDI(x4, xRCX, 0x1F);
                    }
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEB(x1, 0);
                    UFLAG_OP12(ed, x4);
                    SRLW(ed, ed, x4);
                    EBBACK(x5, 1);
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_shr8);
                    break;
                case 7:
                    if(opcode==0xD0) {
                        INST_NAME("SAR Eb, 1");
                        MOV32w(x4, 1);
                    } else {
                        INST_NAME("SAR Eb, CL");
                        ANDI(x4, xRCX, 0x1f);
                    }
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETSEB(x1, 0);
                    UFLAG_OP12(ed, x4)
                    SRA(ed, ed, x4);
                    EBBACK(x3, 1);
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_sar8);
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xD1:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ed, 1");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET_PENDING);
                    GETED(0);
                    emit_rol32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    if(!wback && !rex.w) ZEROUP(ed);
                    break;
                case 1:
                    INST_NAME("ROR Ed, 1");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET_PENDING);
                    GETED(0);
                    emit_ror32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    if(!wback && !rex.w) ZEROUP(ed);
                    break;
                case 2:
                    INST_NAME("RCL Ed, 1");
                    MESSAGE("LOG_DUMP", "Need optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    MOV32w(x2, 1);
                    GETEDW(x4, x1, 0);
                    CALL_(rex.w ? ((void*)rcl64) : ((void*)rcl32), ed, x4);
                    WBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ed, 1");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    MOV32w(x2, 1);
                    GETEDW(x4, x1, 0);
                    CALL_(rex.w?((void*)rcr64):((void*)rcr32), ed, x4);
                    WBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETED(0);
                    emit_shl32c(dyn, ninst, rex, ed, 1, x3, x4, x5);
                    WBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETED(0);
                    emit_shr32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETED(0);
                    emit_sar32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0xD3:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ed, CL");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    GETED(0);
                    emit_rol32(dyn, ninst, rex, ed, xRCX, x3, x4);
                    WBACK;
                    if(!wback && !rex.w) ZEROUP(ed);
                    break;
                case 1:
                    INST_NAME("ROR Ed, CL");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    GETED(0);
                    emit_ror32(dyn, ninst, rex, ed, xRCX, x3, x4);
                    WBACK;
                    if(!wback && !rex.w) ZEROUP(ed);
                    break;
                case 3:
                    INST_NAME("RCR Ed, CL");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    ANDI(x2, xRCX, rex.w?0x3f:0x1f);
                    GETEDW(x4, x1, 0);
                    CALL_(rex.w?((void*)rcr64):((void*)rcr32), ed, x4);
                    WBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    ANDI(x3, xRCX, rex.w?0x3f:0x1f);
                    GETED(0);
                    if(!rex.w && MODREG) {ZEROUP(ed);}
                    CBZ_NEXT(x3);
                    emit_shl32(dyn, ninst, rex, ed, x3, x5, x4, x6);
                    WBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ed, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    ANDI(x3, xRCX, rex.w?0x3f:0x1f);
                    GETED(0);
                    if(!rex.w && MODREG) {ZEROUP(ed);}
                    CBZ_NEXT(x3);
                    emit_shr32(dyn, ninst, rex, ed, x3, x5, x4);
                    WBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ed, CL");
                    SETFLAGS(X_ALL, SF_PENDING);
                    ANDI(x3, xRCX, rex.w?0x3f:0x1f);
                    GETED(0);
                    if(!rex.w && MODREG) {ZEROUP(ed);}
                    CBZ_NEXT(x3);
                    UFLAG_OP12(ed, x3);
                    SRAxw(ed, ed, x3);
                    WBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, rex.w?d_sar64:d_sar32);
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0xD8:
            addr = dynarec64_D8(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xD9:
            addr = dynarec64_D9(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xDA:
            addr = dynarec64_DA(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xDB:
            addr = dynarec64_DB(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xDC:
            addr = dynarec64_DC(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xDD:
            addr = dynarec64_DD(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;

        case 0xDE:
            addr = dynarec64_DE(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xDF:
            addr = dynarec64_DF(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        #define GO(Z)                                                   \
            BARRIER(BARRIER_MAYBE);                                     \
            JUMP(addr+i8, 1);                                           \
            if(dyn->insts[ninst].x64.jmp_insts==-1 ||                   \
                CHECK_CACHE()) {                                        \
                /* out of the block */                                  \
                i32 = dyn->insts[ninst].epilog-(dyn->native_size);      \
                if(Z) {BNE(xRCX, xZR, i32);} else {BEQ(xRCX, xZR, i32);};      \
                if(dyn->insts[ninst].x64.jmp_insts==-1) {               \
                    if(!(dyn->insts[ninst].x64.barrier&BARRIER_FLOAT))  \
                        fpu_purgecache(dyn, ninst, 1, x1, x2, x3);      \
                    jump_to_next(dyn, addr+i8, 0, ninst);               \
                } else {                                                \
                    CacheTransform(dyn, ninst, cacheupd, x1, x2, x3);   \
                    i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);    \
                    B(i32);                                             \
                }                                                       \
            } else {                                                    \
                /* inside the block */                                  \
                i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);    \
                if(Z) {BEQ(xRCX, xZR, i32);} else {BNE(xRCX, xZR, i32);};      \
            }
        case 0xE0:
            INST_NAME("LOOPNZ");
            READFLAGS(X_ZF);
            i8 = F8S;
            SUBI(xRCX, xRCX, 1);
            ANDI(x1, xFlags, 1 << F_ZF);
            CBNZ_NEXT(x1);
            GO(0);
            break;
        case 0xE1:
            INST_NAME("LOOPZ");
            READFLAGS(X_ZF);
            i8 = F8S;
            SUBI(xRCX, xRCX, 1);
            ANDI(x1, xFlags, 1 << F_ZF);
            CBZ_NEXT(x1);
            GO(0);
            break;
        case 0xE2:
            INST_NAME("LOOP");
            i8 = F8S;
            SUBI(xRCX, xRCX, 1);
            GO(0);
            break;
        case 0xE3:
            INST_NAME("JECXZ");
            i8 = F8S;
            GO(1);
            break;
        #undef GO

        case 0xE8:
            INST_NAME("CALL Id");
            i32 = F32S;
            if(addr+i32==0) {
                #if STEP == 3
                printf_log(LOG_INFO, "Warning, CALL to 0x0 at %p (%p)\n", (void*)addr, (void*)(addr-1));
                #endif
            }
            #if STEP < 2
            if(!rex.is32bits && isNativeCall(dyn, addr+i32, &dyn->insts[ninst].natcall, &dyn->insts[ninst].retn))
                tmp = dyn->insts[ninst].pass2choice = 3;
            else
                tmp = dyn->insts[ninst].pass2choice = 0;
            #else
                tmp = dyn->insts[ninst].pass2choice;
            #endif
            switch(tmp) {
                case 3:
                    SETFLAGS(X_ALL, SF_SET);    // Hack to set flags to "dont'care" state
                    SKIPTEST(x1);
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
                    sse_purge07cache(dyn, ninst, x3);
                    if((box64_log<2 && !cycle_log) && dyn->insts[ninst].natcall) {
                        // disabling isSimpleWrapper because all signed value less than 64bits needs to be sign extended
                        // and return value needs to be cleanned up
                        tmp=0;//isSimpleWrapper(*(wrapper_t*)(dyn->insts[ninst].natcall+2));
                        if(tmp>1 || tmp<0)
                            tmp=0;  // float paramters not ready!
                    } else
                        tmp=0;
                    if(dyn->insts[ninst].natcall && isRetX87Wrapper(*(wrapper_t*)(dyn->insts[ninst].natcall+2)))
                        // return value will be on the stack, so the stack depth needs to be updated
                        x87_purgecache(dyn, ninst, 0, x3, x1, x4);
                    if((box64_log<2 && !cycle_log) && dyn->insts[ninst].natcall && tmp) {
                        //GETIP(ip+3+8+8); // read the 0xCC
                        call_n(dyn, ninst, *(void**)(dyn->insts[ninst].natcall+2+8), tmp);
                        POP1(xRIP);       // pop the return address
                        dyn->last_ip = addr;
                    } else {
                        GETIP_(dyn->insts[ninst].natcall); // read the 0xCC already
                        STORE_XEMU_CALL(x3);
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
                        jump_to_epilog_fast(dyn, 0, xRIP, ninst);
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
                    if(box64_dynarec_callret && box64_dynarec_bigblock>1) {
                        BARRIER(BARRIER_FULL);
                    } else {
                        BARRIER(BARRIER_FLOAT);
                        *need_epilog = 0;
                        *ok = 0;
                    }

                    if(rex.is32bits) {
                        MOV32w(x2, addr);
                    } else {
                        TABLE64(x2, addr);
                    }
                    PUSH1z(x2);
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
                    jump_to_next(dyn, addr+i32, 0, ninst);
                    break;
            }
            break;
        case 0xE9:
        case 0xEB:
            BARRIER(BARRIER_MAYBE);
            if(opcode==0xE9) {
                INST_NAME("JMP Id");
                i32 = F32S;
            } else {
                INST_NAME("JMP Ib");
                i32 = F8S;
            }
            JUMP((uintptr_t)getAlternate((void*)(addr+i32)), 0);
            if(dyn->insts[ninst].x64.jmp_insts==-1) {
                // out of the block
                fpu_purgecache(dyn, ninst, 1, x1, x2, x3);
                jump_to_next(dyn, (uintptr_t)getAlternate((void*)(addr+i32)), 0, ninst);
            } else {
                // inside the block
                CacheTransform(dyn, ninst, CHECK_CACHE(), x1, x2, x3);
                tmp = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);
                MESSAGE(1, "Jump to %d / 0x%x\n", tmp, tmp);
                if(tmp==4) {
                    NOP();
                } else {
                    B(tmp);
                }
            }
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xF0:
            addr = dynarec64_F0(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xF5:
            INST_NAME("CMC");
            READFLAGS(X_CF);
            SETFLAGS(X_CF, SF_SUBSET);
            XORI(xFlags, xFlags, 1<<F_CF);
            break;
        case 0xF6:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                case 1:
                    INST_NAME("TEST Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    emit_test8(dyn, ninst, x1, x2, x3, x4, x5);
                    break;
                case 2:
                    INST_NAME("NOT Eb");
                    GETEB(x1, 0);
                    NOT(x1, x1);
                    EBBACK(x5, 1);
                    break;
                case 3:
                    INST_NAME("NEG Eb");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 0);
                    emit_neg8(dyn, ninst, x1, x2, x4);
                    EBBACK(x5, 0);
                    break;
                case 4:
                    INST_NAME("MUL AL, Ed");
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_DF(x1, d_mul8);
                    GETEB(x1, 0);
                    ANDI(x2, xRAX, 0xff);
                    MULW(x1, x2, x1);
                    UFLAG_RES(x1);
                    LUI(x2, 0xffff0);
                    AND(xRAX, xRAX, x2);
                    ZEXTH(x1, x1);
                    OR(xRAX, xRAX, x1);
                    break;
                case 5:
                    INST_NAME("IMUL AL, Eb");
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_DF(x1, d_imul8);
                    GETSEB(x1, 0);
                    SLLI(x2, xRAX, 56);
                    SRAI(x2, x2, 56);
                    MULW(x1, x2, x1);
                    UFLAG_RES(x1);
                    LUI(x2, 0xffff0);
                    AND(xRAX, xRAX, x2);
                    ZEXTH(x1, x1);
                    OR(xRAX, xRAX, x1);
                    break;
                case 6:
                    INST_NAME("DIV Eb");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1, 0);
                    CALL(div8, -1);
                    break;
                case 7:
                    INST_NAME("IDIV Eb");
                    SKIPTEST(x1);
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1, 0);
                    CALL(idiv8, -1);
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xF7:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                case 1:
                    INST_NAME("TEST Ed, Id");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED(4);
                    i64 = F32S;
                    emit_test32c(dyn, ninst, rex, ed, i64, x3, x4, x5);
                    break;
                case 2:
                    INST_NAME("NOT Ed");
                    GETED(0);
                    XORI(ed, ed, -1);
                    if(!rex.w && MODREG)
                        ZEROUP(ed);
                    WBACK;
                    break;
                case 3:
                    INST_NAME("NEG Ed");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED(0);
                    emit_neg32(dyn, ninst, rex, ed, x3, x4);
                    WBACK;
                    break;
                case 4:
                    INST_NAME("MUL EAX, Ed");
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_DF(x2, rex.w?d_mul64:d_mul32);
                    GETED(0);
                    if(rex.w) {
                        if(ed==xRDX) gd=x3; else gd=xRDX;
                        MULHU(gd, xRAX, ed);
                        MUL(xRAX, xRAX, ed);
                        if(gd!=xRDX) {MV(xRDX, gd);}
                    } else {
                        AND(x3, xRAX, xMASK);
                        if(MODREG) {
                            AND(x4, ed, xMASK);
                            ed = x4;
                        }
                        MUL(xRDX, x3, ed);  //64 <- 32x32
                        AND(xRAX, xRDX, xMASK);
                        SRLI(xRDX, xRDX, 32);
                    }
                    UFLAG_RES(xRAX);
                    UFLAG_OP1(xRDX);
                    break;
                case 5:
                    INST_NAME("IMUL EAX, Ed");
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_DF(x2, rex.w?d_imul64:d_imul32);
                    GETSED(0);
                    if(rex.w) {
                        if(ed==xRDX) gd=x3; else gd=xRDX;
                        MULH(gd, xRAX, ed);
                        MUL(xRAX, xRAX, ed);
                        if(gd!=xRDX) {MV(xRDX, gd);}
                    } else {
                        ADDIW(x3, xRAX, 0); // sign extend 32bits-> 64bits
                        MUL(xRDX, x3, ed);  //64 <- 32x32
                        AND(xRAX, xRDX, xMASK);
                        SRLI(xRDX, xRDX, 32);
                    }
                    UFLAG_RES(xRAX);
                    UFLAG_OP1(xRDX);
                    break;
                case 6:
                    INST_NAME("DIV Ed");
                    SETFLAGS(X_ALL, SF_SET);
                    if(!rex.w) {
                        SET_DFNONE();
                        GETED(0);
                        SLLI(x3, xRDX, 32);
                        AND(x2, xRAX, xMASK);
                        OR(x3, x3, x2);
                        if(MODREG) {
                            AND(x4, ed, xMASK);
                            ed = x4;
                        }
                        DIVU(x2, x3, ed);
                        REMU(xRDX, x3, ed);
                        AND(xRAX, x2, xMASK);
                        ZEROUP(xRDX);
                    } else {
                        if(ninst
                           && dyn->insts[ninst-1].x64.addr
                           && *(uint8_t*)(dyn->insts[ninst-1].x64.addr)==0x31
                           && *(uint8_t*)(dyn->insts[ninst-1].x64.addr+1)==0xD2) {
                            SET_DFNONE();
                            GETED(0);
                            DIVU(x2, xRAX, ed);
                            REMU(xRDX, xRAX, ed);
                            MV(xRAX, x2);
                        } else {
                            GETEDH(x1, 0);  // get edd changed addr, so cannot be called 2 times for same op...
                            BEQ_MARK(xRDX, xZR);
                            if(ed!=x1) {MV(x1, ed);}
                            CALL(div64, -1);
                            B_NEXT_nocond;
                            MARK;
                            DIVU(x2, xRAX, ed);
                            REMU(xRDX, xRAX, ed);
                            MV(xRAX, x2);
                            SET_DFNONE();
                        }
                    }
                    break;
                case 7:
                    INST_NAME("IDIV Ed");
                    SKIPTEST(x1);
                    SETFLAGS(X_ALL, SF_SET);
                    if(!rex.w) {
                        SET_DFNONE()
                        GETSED(0);
                        SLLI(x3, xRDX, 32);
                        AND(x2, xRAX, xMASK);
                        OR(x3, x3, x2);
                        DIV(x2, x3, ed);
                        REM(xRDX, x3, ed);
                        AND(xRAX, x2, xMASK);
                        ZEROUP(xRDX);
                    } else {
                        if(ninst && dyn->insts
                           &&  dyn->insts[ninst-1].x64.addr
                           && *(uint8_t*)(dyn->insts[ninst-1].x64.addr)==0x48
                           && *(uint8_t*)(dyn->insts[ninst-1].x64.addr+1)==0x99) {
                            SET_DFNONE()
                            GETED(0);
                            DIV(x2, xRAX, ed);
                            REM(xRDX, xRAX, ed);
                            MV(xRAX, x2);
                        } else {
                            GETEDH(x1, 0);  // get edd changed addr, so cannot be called 2 times for same op...
                            //Need to see if RDX==0 and RAX not signed
                            // or RDX==-1 and RAX signed
                            BNE_MARK2(xRDX, xZR);
                            BGE_MARK(xRAX, xZR);
                            MARK2;
                            NOT(x2, xRDX);
                            BNE_MARK3(x2, xZR);
                            BLT_MARK(xRAX, xZR);
                            MARK3;
                            if(ed!=x1) MV(x1, ed);
                            CALL((void*)idiv64, -1);
                            B_NEXT_nocond;
                            MARK;
                            DIV(x2, xRAX, ed);
                            REM(xRDX, xRAX, ed);
                            MV(xRAX, x2);
                            SET_DFNONE()
                        }
                    }
                    break;
                default:
                    DEFAULT;
            };
            break;

        case 0xF8:
            INST_NAME("CLC");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE();
            ANDI(xFlags, xFlags, ~(1 << F_CF));
            break;
        case 0xF9:
            INST_NAME("STC");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE();
            ORI(xFlags, xFlags, 1 << F_CF);
            break;
        case 0xFC:
            INST_NAME("CLD");
            ANDI(xFlags, xFlags, ~(1 << F_DF));
            break;
        case 0xFD:
            INST_NAME("STD");
            ORI(xFlags, xFlags, 1 << F_DF);
            break;
        case 0xFE:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("INC Eb");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    GETEB(x1, 0);
                    emit_inc8(dyn, ninst, ed, x2, x4, x5);
                    EBBACK(x5, 0);
                    break;
                case 1:
                    INST_NAME("DEC Eb");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    GETEB(x1, 0);
                    emit_dec8(dyn, ninst, ed, x2, x4, x5);
                    EBBACK(x5, 0);
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xFF:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: // INC Ed
                    INST_NAME("INC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    GETED(0);
                    emit_inc32(dyn, ninst, rex, ed, x3, x4, x5, x6);
                    WBACK;
                    break;
                case 1: //DEC Ed
                    INST_NAME("DEC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    GETED(0);
                    emit_dec32(dyn, ninst, rex, ed, x3, x4, x5, x6);
                    WBACK;
                    break;
                case 2: // CALL Ed
                    INST_NAME("CALL Ed");
                    PASS2IF((box64_dynarec_safeflags>1) ||
                        ((ninst && dyn->insts[ninst-1].x64.set_flags)
                        || ((ninst>1) && dyn->insts[ninst-2].x64.set_flags)), 1)
                    {
                        READFLAGS(X_PEND);          // that's suspicious
                    } else {
                        SETFLAGS(X_ALL, SF_SET);    //Hack to put flag in "don't care" state
                    }
                    GETEDz(0);
                    if(box64_dynarec_callret && box64_dynarec_bigblock>1) {
                        BARRIER(BARRIER_FULL);
                    } else {
                        BARRIER(BARRIER_FLOAT);
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    GETIP_(addr);
                    // TODO: Add suport for CALLRET optim
                    /*if(box64_dynarec_callret) {
                        // Push actual return address
                        if(addr < (dyn->start+dyn->isize)) {
                            // there is a next...
                            j64 = (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->native_size)):0;
                            ADR_S20(x4, j64);
                        } else {
                            j64 = getJumpTableAddress64(addr);
                            TABLE64(x4, j64);
                            LDRx_U12(x4, x4, 0);
                        }
                        STPx_S7_preindex(x4, xRIP, xSP, -16);
                    }*/
                    PUSH1z(xRIP);
                    jump_to_next(dyn, 0, ed, ninst);
                    break;
                case 4: // JMP Ed
                    INST_NAME("JMP Ed");
                    READFLAGS(X_PEND);
                    BARRIER(BARRIER_FLOAT);
                    GETEDz(0);
                    jump_to_next(dyn, 0, ed, ninst);
                    *need_epilog = 0;
                    *ok = 0;
                    break;
                case 5: // JMP FAR Ed
                    if(MODREG) {
                        DEFAULT;
                    } else {
                        INST_NAME("JMP FAR Ed");
                        READFLAGS(X_PEND);
                        BARRIER(BARRIER_FLOAT);
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 0, 0);
                        LDxw(x1, wback, 0);
                        ed = x1;
                        LHU(x3, wback, rex.w?8:4);
                        SH(x3, xEmu, offsetof(x64emu_t, segs[_CS]));
                        SW(xZR, xEmu, offsetof(x64emu_t, segs_serial[_CS]));
                        jump_to_epilog(dyn, 0, ed, ninst);
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    break;
                case 6: // Push Ed
                    INST_NAME("PUSH Ed");
                    GETEDz(0);
                    PUSH1z(ed);
                    break;

                default:
                    DEFAULT;
            }
            break;
        default:
            DEFAULT;
    }

     return addr;
}
