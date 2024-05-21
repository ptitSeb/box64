#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "emu/x87emu_private.h"
#include "dynarec_native.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_helper.h"
#include "dynarec_rv64_functions.h"

uintptr_t dynarec64_DF(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip; (void)rep; (void)need_epilog;

    uint8_t nextop = F8;
    uint8_t ed, wback, u8;
    int v1, v2;
    int s0;
    int64_t j64;
    int64_t fixedaddress;

    MAYUSE(s0);
    MAYUSE(v2);
    MAYUSE(v1);
    MAYUSE(j64);

    switch(nextop) {
        case 0xC0 ... 0xC7:
            INST_NAME("FFREEP STx");
            // not handling Tag...
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;

        case 0xE0:
            INST_NAME("FNSTSW AX");
            LWU(x2, xEmu, offsetof(x64emu_t, top));
            LHU(x1, xEmu, offsetof(x64emu_t, sw));
            MOV32w(x3, 0b1100011111111111); // mask
            AND(x1, x1, x3);
            SLLI(x2, x2, 11);
            OR(x1, x1, x2); // inject top
            SH(x1, xEmu, offsetof(x64emu_t, sw));
            SRLI(xRAX, xRAX, 16);
            SLLI(xRAX, xRAX, 16);
            OR(xRAX, xRAX, x1);
            break;
        case 0xE8 ... 0xF7:
            if (nextop < 0xF0) {
                INST_NAME("FUCOMIP ST0, STx");
            } else {
                INST_NAME("FCOMIP ST0, STx");
            }
            SETFLAGS(X_ALL, SF_SET);
            SET_DFNONE();
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            CLEAR_FLAGS();
            IFX(X_ZF | X_PF | X_CF) {
                if(ST_IS_F(0)) {
                    FEQS(x5, v1, v1);
                    FEQS(x4, v2, v2);
                    AND(x5, x5, x4);
                    BEQZ(x5, 24); // undefined/NaN
                    FEQS(x5, v1, v2);
                    BNEZ(x5, 24); // equal
                    FLTS(x3, v1, v2); // x3 = (v1<v2)?1:0
                    OR(xFlags, xFlags, x3); // CF is the least significant bit
                    J(16); // end
                    // NaN
                    ORI(xFlags, xFlags, (1<<F_ZF) | (1<<F_PF) | (1<<F_CF));
                    J(8); // end
                    // equal
                    ORI(xFlags, xFlags, 1<<F_ZF);
                    // end
                } else {
                    FEQD(x5, v1, v1);
                    FEQD(x4, v2, v2);
                    AND(x5, x5, x4);
                    BEQZ(x5, 24); // undefined/NaN
                    FEQD(x5, v1, v2);
                    BNEZ(x5, 24); // equal
                    FLTD(x3, v1, v2); // x3 = (v1<v2)?1:0
                    OR(xFlags, xFlags, x3); // CF is the least significant bit
                    J(16); // end
                    // NaN
                    ORI(xFlags, xFlags, (1<<F_ZF) | (1<<F_PF) | (1<<F_CF));
                    J(8); // end
                    // equal
                    ORI(xFlags, xFlags, 1<<F_ZF);
                    // end
                }
            }
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        case 0xC8 ... 0xDF:
        case 0xE1 ... 0xE7:
        case 0xF8 ... 0xFF:
            DEFAULT;
            break;

        default:
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("FILD ST0, Ew");
                    X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, EXT_CACHE_ST_F);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, x4, &fixedaddress, rex, NULL, 1, 0);
                    LH(x1, wback, fixedaddress);
                    if(ST_IS_F(0)) {
                        FCVTSL(v1, x1, RD_RNE);
                    } else {
                        FCVTDL(v1, x1, RD_RNE);
                    }
                    break;
                case 1:
                    INST_NAME("FISTTP Ew, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, EXT_CACHE_ST_F);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, x4, &fixedaddress, rex, NULL, 1, 0);
                    if(!box64_dynarec_fastround) {
                        FSFLAGSI(0); // reset all bits
                    }
                    if (ST_IS_F(0)) {
                        FCVTWS(x4, v1, RD_RTZ);
                    } else {
                        FCVTWD(x4, v1, RD_RTZ);
                    }
                    if(!box64_dynarec_fastround) {
                        FRFLAGS(x5);   // get back FPSR to check the IOC bit
                        ANDI(x5, x5, 1<<FR_NV);
                        BNEZ_MARK(x5);
                        SLLIW(x5, x4, 16);
                        SRAIW(x5, x5, 16);
                        BEQ_MARK2(x5, x4);
                        MARK;
                        MOV32w(x4, 0x8000);
                    }
                    MARK2;
                    SH(x4, wback, fixedaddress);
                    X87_POP_OR_FAIL(dyn, ninst, x3);
                    break;
                case 3:
                    INST_NAME("FISTP Ew, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, EXT_CACHE_ST_F);
                    u8 = x87_setround(dyn, ninst, x1, x2);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                    if(!box64_dynarec_fastround) {
                        FSFLAGSI(0); // reset all bits
                    }
                    if (ST_IS_F(0)) {
                        FCVTWS(x4, v1, RD_DYN);
                    } else {
                        FCVTWD(x4, v1, RD_DYN);
                    }
                    x87_restoreround(dyn, ninst, u8);
                    if(!box64_dynarec_fastround) {
                        FRFLAGS(x5);   // get back FPSR to check the IOC bit
                        ANDI(x5, x5, 1<<FR_NV);
                        BNEZ_MARK(x5);
                        SLLIW(x5, x4, 16);
                        SRAIW(x5, x5, 16);
                        BEQ_MARK2(x5, x4);
                        MARK;
                        MOV32w(x4, 0x8000);
                    }
                    MARK2;
                    SH(x4, wback, fixedaddress);
                    X87_POP_OR_FAIL(dyn, ninst, x3);
                    break;
                case 5:
                    INST_NAME("FILD ST0, i64");
                    X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, EXT_CACHE_ST_I64);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 1, 0);

                    if (ST_IS_I64(0)) {
                        FLD(v1, wback, fixedaddress);
                    } else {
                        LD(x1, wback, fixedaddress);
                        if (rex.is32bits) {
                            // need to also feed the STll stuff...
                            ADDI(x4, xEmu, offsetof(x64emu_t, fpu_ll));
                            LWU(x5, xEmu, offsetof(x64emu_t, top));
                            int a = 0 - dyn->e.x87stack;
                            if (a) {
                                ADDIW(x5, x5, a);
                                ANDI(x5, x5, 0x7);
                            }
                            SLLI(x5, x5, 4); // fpu_ll is 2 i64
                            ADD(x5, x5, x4);
                            SD(x1, x5, 8); // ll
                        }
                        FCVTDL(v1, x1, RD_RTZ);
                        if (rex.is32bits) {
                            FSD(v1, x5, 0); // ref
                        }
                    }
                    break;
                case 6:
                    INST_NAME("FBSTP tbytes, ST0");
                    x87_forget(dyn, ninst, x1, x2, 0);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                    if (ed != x1) { MV(x1, ed); }
                    s0 = x87_stackcount(dyn, ninst, x3);
                    CALL(fpu_fbst, -1);
                    x87_unstackcount(dyn, ninst, x3, s0);
                    X87_POP_OR_FAIL(dyn, ninst, x3);
                    break;
                case 7:
                    INST_NAME("FISTP i64, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, EXT_CACHE_ST_I64);
                    if (!ST_IS_I64(0)) {
                        u8 = x87_setround(dyn, ninst, x1, x2);
                    }
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 1, 0);

                    if (ST_IS_I64(0)) {
                        FSD(v1, wback, fixedaddress);
                    } else {
                        if (rex.is32bits) {
                            // need to check STll first...
                            ADDI(x4, xEmu, offsetof(x64emu_t, fpu_ll));
                            LWU(x5, xEmu, offsetof(x64emu_t, top));
                            int a = 0 - dyn->e.x87stack;
                            if (a) {
                                ADDIW(x5, x5, a);
                                ANDI(x5, x5, 0x7);
                            }
                            SLLI(x5, x5, 4); // fpu_ll is 2 i64
                            ADD(x5, x5, x4);
                            FMVXD(x3, v1);
                            LD(x6, x5, 0); // ref
                            BNE_MARK(x6, x3);
                            LD(x6, x5, 8); // ll
                            SD(x6, wback, fixedaddress);
                            B_MARK3_nocond;
                            MARK;
                        }

                        if (!box64_dynarec_fastround) {
                            FSFLAGSI(0); // reset all bits
                        }
                        FCVTLD(x4, v1, RD_DYN);
                        if (!box64_dynarec_fastround) {
                            FRFLAGS(x5); // get back FPSR to check the IOC bit
                            ANDI(x5, x5, 1 << FR_NV);
                            BEQ_MARK2(x5, xZR);
                            MOV64x(x4, 0x8000000000000000LL);
                        }
                        MARK2;
                        SD(x4, wback, fixedaddress);
                        MARK3;
                        x87_restoreround(dyn, ninst, u8);
                    }
                    X87_POP_OR_FAIL(dyn, ninst, x3);
                    break;
                default:
                    DEFAULT;
                    break;
            }
    }
    return addr;
}
