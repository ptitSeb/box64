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


uintptr_t dynarec64_DF(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip; (void)rep; (void)need_epilog;

    uint8_t nextop = F8;
    uint8_t ed, wback, u8;
    int v1, v2;
    int s0;
    int64_t j64;
    int64_t fixedaddress;
    int unscaled;
    int i1;

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
            INST_NAME("FFREEP STx");
            // not handling Tag...
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;

        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
            INST_NAME("FSTP STx, ST0");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                FMOVS(v2, v1);
            } else {
                FMOVD(v2, v1);
            }
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;

        case 0xE0:
            INST_NAME("FNSTSW AX");
            LDRw_U12(x2, xEmu, offsetof(x64emu_t, top));
            if(dyn->n.x87stack) {
                if(dyn->n.x87stack>0) {
                    SUBw_U12(x2, x2, dyn->n.x87stack);
                } else {
                    ADDw_U12(x2, x2, -dyn->n.x87stack);
                }
                ANDw_mask(x2, x2, 0, 2);  //mask=7
            }
            LDRH_U12(x1, xEmu, offsetof(x64emu_t, sw));
            BFIw(x1, x2, 11, 3); // inject top
            STRH_U12(x1, xEmu, offsetof(x64emu_t, sw));
            BFIx(xRAX, x1, 0, 16);
            break;
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
            INST_NAME("FUCOMIP ST0, STx");
            SETFLAGS(X_ALL, SF_SET);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0))
            {
                FCMPS(v1, v2);
            } else {
                FCMPD(v1, v2);
            }
            FCOMI(x1, x2);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
            INST_NAME("FCOMIP ST0, STx");
            SETFLAGS(X_ALL, SF_SET);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0))
            {
                FCMPS(v1, v2);
            } else {
                FCMPD(v1, v2);
            }
            FCOMI(x1, x2);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;

        default:
            DEFAULT;
            break;
    } else
        switch((nextop>>3)&7) {
            case 0:
                INST_NAME("FILD ST0, Ew");
                X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, NEON_CACHE_ST_F);
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                LDSHw(x1, wback, fixedaddress);
                if(ST_IS_F(0)) {
                    SCVTFSw(v1, x1);
                } else {
                    SCVTFDw(v1, x1);
                }
                break;
            case 1:
                INST_NAME("FISTTP Ew, ST0");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_F);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                ed = x1;
                s0 = fpu_get_scratch(dyn);
                #if 0
                // this version needs ARM v8.5, and doesn't handle saturation for 32bits integer not fitting 16bits
                FRINT32ZD(s0, v1);
                // no saturation instruction on Arm, so using NEON
                VFCVTZSd(s0, s0);
                SQXTN_S_D(s0, s0);
                SQXTN_H_S(s0, s0);
                VST16(s0, wback, fixedaddress);
                #else
                MRS_fpsr(x5);
                BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                BFCw(x5, FPSR_QC, 1);   // reset QC bit
                MSR_fpsr(x5);
                if(ST_IS_F(0)) {
                    VFCVTZSs(s0, v1);
                } else {
                    VFCVTZSd(s0, v1);
                    SQXTN_S_D(s0, s0);
                }
                VMOVSto(x3, s0, 0);
                MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                TBNZ_MARK2(x5, FPSR_IOC);
                SXTHw(x5, x3);  // check if 16bits value is fine
                SUBw_REG(x5, x5, x3);
                CBZw_MARK3(x5);
                MARK2;
                MOV32w(x3, 0x8000);
                MARK3;
                STH(x3, wback, fixedaddress);
                #endif
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 2:
                INST_NAME("FIST Ew, ST0");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_F);
                u8 = x87_setround(dyn, ninst, x1, x2, x4);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                ed = x1;
                s0 = fpu_get_scratch(dyn);
                #if 0
                FRINT32XD(s0, v1);
                // no saturation instruction on Arm, so using NEON
                VFCVTZSd(s0, s0);
                SQXTN_S_D(s0, s0);
                SQXTN_H_S(s0, s0);
                VST16(s0, wback, fixedaddress);
                #else
                MRS_fpsr(x5);
                BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                BFCw(x5, FPSR_QC, 1);   // reset QC bit
                MSR_fpsr(x5);
                if(ST_IS_F(0)) {
                    FRINTXS(s0, v1);
                    VFCVTZSs(s0, s0);
                } else {
                    FRINTXD(s0, v1);
                    VFCVTZSd(s0, s0);
                    SQXTN_S_D(s0, s0);
                }
                VMOVSto(x3, s0, 0);
                MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                TBNZ_MARK2(x5, FPSR_IOC);
                SXTHw(x5, x3);  // check if 16bits value is fine
                SUBw_REG(x5, x5, x3);
                CBZw_MARK3(x5);
                MARK2;
                MOV32w(x3, 0x8000);
                MARK3;
                STH(x3, wback, fixedaddress);
                #endif
                x87_restoreround(dyn, ninst, u8);
                break;
            case 3:
                INST_NAME("FISTP Ew, ST0");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_F);
                u8 = x87_setround(dyn, ninst, x1, x2, x4);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                ed = x1;
                s0 = fpu_get_scratch(dyn);
                #if 0
                FRINT32XD(s0, v1);
                // no saturation instruction on Arm, so using NEON
                VFCVTZSd(s0, s0);
                SQXTN_S_D(s0, s0);
                SQXTN_H_S(s0, s0);
                VST16(s0, wback, fixedaddress);
                #else
                MRS_fpsr(x5);
                BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                MSR_fpsr(x5);
                if(ST_IS_F(0)) {
                    FRINTXS(s0, v1);
                    VFCVTZSs(s0, s0);
                } else {
                    FRINTXD(s0, v1);
                    VFCVTZSd(s0, s0);
                    SQXTN_S_D(s0, s0);
                }
                VMOVSto(x3, s0, 0);
                MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                TBNZ_MARK2(x5, FPSR_IOC);
                SXTHw(x5, x3);  // check if 16bits value is fine
                SUBw_REG(x5, x5, x3);
                CBZw_MARK3(x5);
                MARK2;
                MOV32w(x3, 0x8000);
                MARK3;
                STH(x3, wback, fixedaddress);
                #endif
                X87_POP_OR_FAIL(dyn, ninst, x3);
                x87_restoreround(dyn, ninst, u8);
                break;
            case 4:
                INST_NAME("FBLD ST0, tbytes");
                X87_PUSH_EMPTY_OR_FAIL(dyn, ninst, x1);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                if(ed!=x1) {MOVx_REG(x1, ed);}
                s0 = x87_stackcount(dyn, ninst, x3);
                CALL(fpu_fbld, -1);
                x87_unstackcount(dyn, ninst, x3, s0);
                break;
            case 5:
                INST_NAME("FILD ST0, i64");
                X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, NEON_CACHE_ST_I64);
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                VLD64(v1, wback, fixedaddress);
                if(!ST_IS_I64(0)) {
                    if(rex.is32bits) {
                        // need to also feed the STll stuff...
                        ADDx_U12(x4, xEmu, offsetof(x64emu_t, fpu_ll));
                        LDRw_U12(x1, xEmu, offsetof(x64emu_t, top));
                        int a = 0 - dyn->n.x87stack;
                        if(a) {
                            if(a<0) {
                                SUBw_U12(x1, x1, -a);
                            } else {
                                ADDw_U12(x1, x1, a);
                            }
                            ANDw_mask(x1, x1, 0, 2); //mask=7
                        }
                        ADDx_REG_LSL(x1, x4, x1, 4);    // fpu_ll is 2 i64
                        VSTR64_U12(v1, x1, 8);  // ll
                    }
                    SCVTFDD(v1, v1);
                    if(rex.is32bits) {
                        VSTR64_U12(v1, x1, 0);  // ref
                    }
                }
                break;
            case 6:
                INST_NAME("FBSTP tbytes, ST0");
                i1 = x87_stackcount(dyn, ninst, x1);
                x87_forget(dyn, ninst, x1, x2, 0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                if(ed!=x1) {MOVx_REG(x1, ed);}
                CALL(fpu_fbst, -1);
                x87_unstackcount(dyn, ninst, x1, i1);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 7:
                INST_NAME("FISTP i64, ST0");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_I64);
                if(!ST_IS_I64(0)) {
                    u8 = x87_setround(dyn, ninst, x1, x2, x4);
                }
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                ed = x1;
                s0 = fpu_get_scratch(dyn);
                if(ST_IS_I64(0)) {
                    VST64(v1, wback, fixedaddress);
                } else {
                    #if 0
                    FRINT64XD(s0, v1);
                    VFCVTZSd(s0, s0);
                    VSTR64_U12(s0, wback, fixedaddress);
                    #else
                    if(rex.is32bits) {
                        // need to check STll first...
                        ADDx_U12(x5, xEmu, offsetof(x64emu_t, fpu_ll));
                        LDRw_U12(x1, xEmu, offsetof(x64emu_t, top));
                        VMOVQDto(x3, v1, 0);
                        int a = 0 - dyn->n.x87stack;
                        if(a) {
                            if(a<0) {
                                SUBw_U12(x1, x1, -a);
                            } else {
                                ADDw_U12(x1, x1, a);
                            }
                            ANDw_mask(x1, x1, 0, 2); //mask=7
                        }
                        ADDx_REG_LSL(x1, x5, x1, 4);    // fpu_ll is 2 i64
                        LDRx_U12(x5, x1, 0);  // ref
                        SUBx_REG(x5, x5, x3);
                        CBNZx_MARK2(x5);
                        LDRx_U12(x5, x1, 8);  // ll
                        STx(x5, wback, fixedaddress);
                        B_MARK3(c__);
                        MARK2;
                    }
                    MRS_fpsr(x5);
                    BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                    MSR_fpsr(x5);
                    FRINTXD(s0, v1);
                    VFCVTZSd(s0, s0);
                    VST64(s0, wback, fixedaddress);
                    MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                    TBZ_MARK3(x5, FPSR_IOC);
                    ORRx_mask(x5, xZR, 1, 1, 0);    //0x8000000000000000
                    STx(x5, wback, fixedaddress);
                    MARK3;
                    #endif
                    x87_restoreround(dyn, ninst, u8);
                }
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            default:
                DEFAULT;
        }
    return addr;
}
