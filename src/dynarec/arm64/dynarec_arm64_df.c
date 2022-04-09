#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
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

    MAYUSE(s0);
    MAYUSE(v2);
    MAYUSE(v1);
    MAYUSE(j64);

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
            x87_do_pop(dyn, ninst, x3);
            break;

        case 0xE0:
            INST_NAME("FNSTSW AX");
            LDRw_U12(x2, xEmu, offsetof(x64emu_t, top));
            LDRH_U12(x1, xEmu, offsetof(x64emu_t, sw));
            BFIw(x1, x2, 11, 3); // inject top
            BFIw(xRAX, x1, 0, 16);
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
            x87_do_pop(dyn, ninst, x3);
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
            x87_do_pop(dyn, ninst, x3);
            break;

        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
        case 0xD8:
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:
        case 0xE1:
        case 0xE2:
        case 0xE3:
        case 0xE4:
        case 0xE5:
        case 0xE6:
        case 0xE7:
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
        case 0xFE:
        case 0xFF:
            DEFAULT;
            break;

        default:
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("FILD ST0, Ew");
                    v1 = x87_do_push(dyn, ninst, x1, NEON_CACHE_ST_F);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0xfff<<1, 1, rex, 0, 0);
                    LDRSHw_U12(x1, wback, fixedaddress);
                    if(ST_IS_F(0)) {
                        SCVTFSw(v1, x1);
                    } else {
                        SCVTFDw(v1, x1);
                    }
                    break;
                case 1:
                    INST_NAME("FISTTP Ew, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_F);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0xfff<<1, 1, rex, 0, 0);
                    ed = x1;
                    s0 = fpu_get_scratch(dyn);
                    #if 0
                    // this version needs ARM v8.5, //TODO: add detection of this extension to use it
                    FRINT32ZD(s0, v1);
                    // no saturation instruction on Arm, so using NEON
                    VFCVTZSd(s0, s0);
                    SQXTN_S_D(s0, s0);
                    SQXTN_H_S(s0, s0);
                    VSTR16_U12(s0, wback, fixedaddress);
                    #else
                    MRS_fpsr(x5);
                    BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                    MSR_fpsr(x5);
                    if(ST_IS_F(0)) {
                        VFCVTZSs(s0, v1);
                    } else {
                        VFCVTZSd(s0, v1);
                        SQXTN_S_D(s0, s0);
                    }
                    SQXTN_H_S(s0, s0);
                    VSTR16_U12(s0, wback, fixedaddress);
                    MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                    TBZ_MARK3(x5, FPSR_IOC);
                    MOV32w(x5, 0x8000);
                    STRH_U12(x5, wback, fixedaddress);
                    MARK3;
                    #endif
                    x87_do_pop(dyn, ninst, x3);
                    break;
                case 2:
                    INST_NAME("FIST Ew, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_F);
                    u8 = x87_setround(dyn, ninst, x1, x2, x4);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0xfff<<1, 1, rex, 0, 0);
                    ed = x1;
                    s0 = fpu_get_scratch(dyn);
                    #if 0
                    FRINT32XD(s0, v1);
                    // no saturation instruction on Arm, so using NEON
                    VFCVTZSd(s0, s0);
                    SQXTN_S_D(s0, s0);
                    SQXTN_H_S(s0, s0);
                    VSTR16_U12(s0, wback, fixedaddress);
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
                    SQXTN_H_S(s0, s0);
                    VSTR16_U12(s0, wback, fixedaddress);
                    MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                    TBZ_MARK3(x5, FPSR_IOC);
                    MOV32w(x5, 0x8000);
                    STRH_U12(x5, wback, fixedaddress);
                    MARK3;
                    #endif
                    x87_restoreround(dyn, ninst, u8);
                    break;
                case 3:
                    INST_NAME("FISTP Ew, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_F);
                    u8 = x87_setround(dyn, ninst, x1, x2, x4);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0xfff<<1, 1, rex, 0, 0);
                    ed = x1;
                    s0 = fpu_get_scratch(dyn);
                    #if 0
                    FRINT32XD(s0, v1);
                    // no saturation instruction on Arm, so using NEON
                    VFCVTZSd(s0, s0);
                    SQXTN_S_D(s0, s0);
                    SQXTN_H_S(s0, s0);
                    VSTR16_U12(s0, wback, fixedaddress);
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
                    SQXTN_H_S(s0, s0);
                    VSTR16_U12(s0, wback, fixedaddress);
                    MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                    TBZ_MARK3(x5, FPSR_IOC);
                    MOV32w(x5, 0x8000);
                    STRH_U12(x5, wback, fixedaddress);
                    MARK3;
                    #endif
                    x87_do_pop(dyn, ninst, x3);
                    x87_restoreround(dyn, ninst, u8);
                    break;
                case 4:
                    INST_NAME("FBLD ST0, tbytes");
                    x87_do_push_empty(dyn, ninst, x1);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, rex, 0, 0);
                    if(ed!=x1) {MOVx_REG(x1, ed);}
                    CALL(fpu_fbld, -1);
                    break;
                case 5:
                    INST_NAME("FILD ST0, i64");
                    v1 = x87_do_push(dyn, ninst, x1, NEON_CACHE_ST_D);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0xfff<<3, 7, rex, 0, 0);
                    LDRx_U12(x1, wback, fixedaddress);
                    SCVTFDx(v1, x1);
                    break;
                case 6:
                    INST_NAME("FBSTP tbytes, ST0");
                    x87_forget(dyn, ninst, x1, x2, 0);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, rex, 0, 0);
                    if(ed!=x1) {MOVx_REG(x1, ed);}
                    CALL(fpu_fbst, -1);
                    x87_do_pop(dyn, ninst, x3);
                    break;
                case 7:
                    INST_NAME("FISTP i64, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    u8 = x87_setround(dyn, ninst, x1, x2, x4);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0xfff<<3, 7, rex, 0, 0);
                    ed = x1;
                    s0 = fpu_get_scratch(dyn);
                    #if 0
                    FRINT64XD(s0, v1);
                    VFCVTZSd(s0, s0);
                    VSTR64_U12(s0, wback, fixedaddress);
                    #else
                    MRS_fpsr(x5);
                    BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                    MSR_fpsr(x5);
                    FRINTXD(s0, v1);
                    VFCVTZSd(s0, s0);
                    VSTR64_U12(s0, wback, fixedaddress);
                    MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                    TBZ_MARK3(x5, FPSR_IOC);
                    MOV64x(x5, 0x8000000000000000LL);
                    STRx_U12(x5, wback, fixedaddress);
                    MARK3;
                    #endif
                    x87_restoreround(dyn, ninst, u8);
                    x87_do_pop(dyn, ninst, x3);
                    break;
                default:
                    DEFAULT;
            }
    }
    return addr;
}
