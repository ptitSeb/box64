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

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_helper.h"
#include "dynarec_arm64_functions.h"


uintptr_t dynarec64_DB(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip; (void)rep; (void)need_epilog;

    uint8_t nextop = F8;
    uint8_t ed;
    uint8_t wback;
    uint8_t u8;
    int64_t fixedaddress;
    int unscaled;
    int v1, v2;
    int s0;
    int64_t j64;

    MAYUSE(s0);
    MAYUSE(v2);
    MAYUSE(v1);
    MAYUSE(j64);

    if(MODREG)
    switch(nextop) {
        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:
            INST_NAME("FCMOVNB ST0, STx");
            READFLAGS(X_CF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            TSTw_mask(xFlags, 0, 0);    //mask=1<<F_CF
            if(ST_IS_F(0)) {
                FCSELS(v1, v2, v1, cEQ);
            } else {
                FCSELD(v1, v2, v1, cEQ);    // F_CF==0
            }
            break;
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
            INST_NAME("FCMOVNE ST0, STx");
            READFLAGS(X_ZF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            TSTw_mask(xFlags, 0b011010, 0); //mask=1<<F_ZF
            if(ST_IS_F(0)) {
                FCSELS(v1, v2, v1, cEQ);
            } else {
                FCSELD(v1, v2, v1, cEQ);        // F_ZF==0
            }
            break;
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
            INST_NAME("FCMOVNBE ST0, STx");
            READFLAGS(X_CF|X_ZF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            MOV32w(x1, (1<<F_CF)|(1<<F_ZF));
            TSTw_REG(xFlags, x1);
            if(ST_IS_F(0)) {
                FCSELS(v1, v2, v1, cEQ);
            } else {
                FCSELD(v1, v2, v1, cEQ);   // F_CF==0 & F_ZF==0
            }
            break;
        case 0xD8:
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:
            INST_NAME("FCMOVNU ST0, STx");
            READFLAGS(X_PF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            TSTw_mask(xFlags, 0b011110, 0); //mask=1<<F_PF
            if(ST_IS_F(0)) {
                FCSELS(v1, v2, v1, cEQ);
            } else {
                FCSELD(v1, v2, v1, cEQ);        // F_PF==0
            }
            break;
        case 0xE1:
            INST_NAME("FDISI8087_NOP"); // so.. NOP?
            break;
        case 0xE2:
            INST_NAME("FNCLEX");
            LDRH_U12(x2, xEmu, offsetof(x64emu_t, sw));
            MOV32w(x1, 0);
            BFIw(x2, x1, 0, 8);  // IE .. PE, SF, ES
            BFIw(x2, x1, 15, 1); // B
            STRH_U12(x2, xEmu, offsetof(x64emu_t, sw));
            break;
        case 0xE3:
            INST_NAME("FNINIT");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_purgecache(dyn, ninst, 0, x1, x2, x3);
            CALL(reset_fpu, -1);
            break;
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
            INST_NAME("FUCOMI ST0, STx");
            SETFLAGS(X_ALL, SF_SET);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                FCMPS(v1, v2);
            } else {
                FCMPD(v1, v2);
            }
            FCOMI(x1, x2);
            break;
        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
            INST_NAME("FCOMI ST0, STx");
            SETFLAGS(X_ALL, SF_SET);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                FCMPS(v1, v2);
            } else {
                FCMPD(v1, v2);
            }
            FCOMI(x1, x2);
            break;

        default:
            DEFAULT;
            break;
    } else
        switch((nextop>>3)&7) {
            case 0:
                INST_NAME("FILD ST0, Ed");
                X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, NEON_CACHE_ST_D);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                VLD32(v1, ed, fixedaddress);
                SXTL_32(v1, v1);    // i32 -> i64
                SCVTFDD(v1, v1);    // i64 -> double
                break;
            case 1:
                INST_NAME("FISTTP Ed, ST0");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                s0 = fpu_get_scratch(dyn);
                if(arm64_frintts) {
                    FRINT32ZD(s0, v1);
                    FCVTZSwD(x5, s0);
                    STW(x5, wback, fixedaddress);
                } else {
                    MRS_fpsr(x5);
                    BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                    MSR_fpsr(x5);
                    FRINTZD(s0, v1);
                    VFCVTZSd(s0, s0);
                    SQXTN_S_D(s0, s0);
                    VST32(s0, wback, fixedaddress);
                    MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                    TBZ_MARK3(x5, FPSR_IOC);
                    MOV32w(x5, 0x80000000);
                    STW(x5, wback, fixedaddress);
                    MARK3;
                }
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 2:
                INST_NAME("FIST Ed, ST0");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                u8 = x87_setround(dyn, ninst, x1, x2, x4); // x1 have the modified RPSCR reg
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                s0 = fpu_get_scratch(dyn);
                if(arm64_frintts) {
                    FRINT32XD(s0, v1);
                    FCVTZSwD(x5, s0);
                    STW(x5, wback, fixedaddress);
                } else {
                    MRS_fpsr(x5);
                    BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                    MSR_fpsr(x5);
                    FRINTXD(s0, v1);
                    VFCVTZSd(s0, s0);
                    SQXTN_S_D(s0, s0);
                    VST32(s0, wback, fixedaddress);
                    MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                    TBZ_MARK3(x5, FPSR_IOC);
                    MOV32w(x5, 0x80000000);
                    STW(x5, wback, fixedaddress);
                    MARK3;
                }
                x87_restoreround(dyn, ninst, u8);
                break;
            case 3:
                INST_NAME("FISTP Ed, ST0");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                u8 = x87_setround(dyn, ninst, x1, x2, x4); // x1 have the modified RPSCR reg
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                s0 = fpu_get_scratch(dyn);
                if(arm64_frintts) {
                    FRINT32XD(s0, v1);
                    FCVTZSwD(x5, s0);
                    STW(x5, wback, fixedaddress);
                } else {
                    MRS_fpsr(x5);
                    BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                    MSR_fpsr(x5);
                    FRINTXD(s0, v1);
                    VFCVTZSd(s0, s0);
                    SQXTN_S_D(s0, s0);
                    VST32(s0, wback, fixedaddress);
                    MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                    TBZ_MARK3(x5, FPSR_IOC);
                    MOV32w(x5, 0x80000000);
                    STW(x5, wback, fixedaddress);
                    MARK3;
                }
                x87_restoreround(dyn, ninst, u8);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 5:
                INST_NAME("FLD tbyte");
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                if((PK(0)==0xDB && ((PK(1)>>3)&7)==7) || (!rex.is32bits && PK(0)>=0x40 && PK(0)<=0x4f && PK(1)==0xDB && ((PK(2)>>3)&7)==7)) {
                    NOTEST(x5);
                    // the FLD is immediatly followed by an FSTP
                    LDRx_U12(x5, ed, 0);
                    LDRH_U12(x6, ed, 8);
                    // no persistant scratch register, so unrool both instruction here...
                    MESSAGE(LOG_DUMP, "\tHack: FSTP tbyte\n");
                    nextop = F8;    // 0xDB or rex
                    if(!rex.is32bits && nextop>=0x40 && nextop<=0x4f) {
                        rex.rex = nextop;
                        nextop = F8;    //0xDB
                    } else
                        rex.rex = 0;
                    nextop = F8;    //modrm
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                    STRx_U12(x5, ed, 0);
                    STRH_U12(x6, ed, 8);
                } else {
                    if(box64_x87_no80bits) {
                        X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, NEON_CACHE_ST_D);
                        VLDR64_U12(v1, ed, fixedaddress);
                    } else {
                        if(ed!=x1) {
                            MOVx_REG(x1, ed);
                        }
                        X87_PUSH_EMPTY_OR_FAIL(dyn, ninst, 0);
                        // sync top
                        s0 = x87_stackcount(dyn, ninst, x3);
                        CALL(native_fld, -1);
                        // go back with the top & stack counter
                        x87_unstackcount(dyn, ninst, x3, s0);
                    }
                }
                break;
            case 7:
                INST_NAME("FSTP tbyte");
                if(box64_x87_no80bits) {
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                    VST64(v1, wback, fixedaddress);
                } else {
                    #if 0
                    x87_forget(dyn, ninst, x1, x3, 0);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                    if(ed!=x1) {
                        MOVx_REG(x1, ed);
                    }
                    CALL(native_fstp, -1);
                    #else
                    // Painfully long, straight conversion from the C code, shoud be optimized
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                    FMOVxD(x1, v1);
                    // do special value first
                    TSTx_mask(x1, 1, 0b00000, 0b111110);    //0x7fffffffffffffffL
                    B_MARK(cNE);
                    // Zero
                    LSRx(x3, x1, 63-15);    //x3 = sign+exp
                    MOVZw(x5, 0);           // x5 = mantisse
                    B_MARK3_nocond;
                    MARK;
                    // get sign, in main ouput x5 for sign+exp
                    ANDx_mask(x5, x1, 1, 1, 0); //0x8000000000000000
                    LSRx(x5, x5, 63-15);    // x5 = sign
                    // get exp
                    LSRx(x3, x1, 52);       // x3 = exp11
                    ANDw_mask(x3, x3, 0, 0b1010);    //0x7ff
                    MOV32w(x4, 0x7ff);
                    CMPSw_REG(x3, x4);
                    B_MARK2(cNE);
                    // NaN and Infinite
                    ORRw_mask(x3, x5, 0, 0b1110);    //x3 = sign | 0x7fff
                    TSTx_mask(x1, 1, 0, 0b110011); //0x000fffffffffffffL
                    ORRx_mask(x5, xZR, 1, 1, 0);    //0x8000000000000000
                    ORRx_mask(x4, xZR, 1, 0b10, 0b01); //0xc000000000000000
                    CSELx(x5, x5, x4, cEQ);     // x5 = mantisse
                    B_MARK3_nocond;
                    MARK2;
                    // regular / denormals
                    ANDx_mask(x1, x1, 1, 0, 0b110011); //0x000fffffffffffffL
                    LSLx_IMM(x1, x1, 11);   //x1 = mantisse missing "1"
                    MOVZw(x4, 16383-1023);  //BIAS80 - BIAS64
                    CBZw(x3, 4+3*4);        // exp11 == 0?
                    // normals
                    ADDw_REG(x3, x3, x4);   // x3 = exp16
                    ORRw_REG(x3, x3, x5);   // x3 = sign | exp
                    ORRx_mask(x5, x1, 1, 1, 0);    //0x8000000000000000 x5 = mantisse
                    B_MARK3_nocond;
                    // denormals
                    CLZx(x6, x1);
                    ADDw_U12(x6, x6, 1);    // "one"
                    SUBw_REG(x3, x4, x6);   // x3 = exp16
                    ORRw_REG(x3, x3, x5);   // x3 = sign | exp16
                    LSLx_REG(x5, x1, x6);   // x5 = mantisse
                    MARK3;
                    STRx_U12(x5, wback, 0);
                    STRH_U12(x3, wback, 8);
                    #endif
                }
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            default:
                DEFAULT;
        }
    return addr;
}
