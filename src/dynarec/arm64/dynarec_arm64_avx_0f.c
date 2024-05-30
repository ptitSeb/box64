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
#include "dynarec_native.h"
#include "my_cpuid.h"
#include "emu/x87emu_private.h"
#include "emu/x64shaext.h"

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"

uintptr_t dynarec64_AVX_0F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2;
    uint8_t eb1, eb2, gb1, gb2;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1, v2;
    int q0, q1, q2;
    int d0, d1, d2;
    int s0;
    uint64_t tmp64u;
    int64_t j64;
    int64_t fixedaddress;
    int unscaled;
    MAYUSE(wb1);
    MAYUSE(wb2);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(gb1);
    MAYUSE(gb2);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(s0);
    MAYUSE(j64);
    MAYUSE(cacheupd);
    #if STEP > 1
    static const int8_t mask_shift8[] = { -7, -6, -5, -4, -3, -2, -1, 0 };
    #endif

    rex_t rex = vex.rex;

    switch(opcode) {

        case 0x10:
            INST_NAME("VMOVUPS Gx,Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                ed = (nextop&7)+(rex.b<<3);
                v1 = sse_get_reg(dyn, ninst, x1, ed, 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VMOVQ(v0, v1);
                if(vex.l) {
                    v1 = ymm_get_reg(dyn, ninst, x1, ed, 0, gd, -1, -1);
                    v0 = ymm_get_reg_empty(dyn, ninst, x1, gd, ed, -1, -1);
                    VMOVQ(v0, v1);
                }
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xffe<<4, 15, rex, NULL, 0, 0);
                VLD128(v0, ed, fixedaddress);   // no alignment issue with ARMv8 NEON :)
                if(vex.l) {
                    v0 = ymm_get_reg_empty(dyn, ninst, x1, gd, -1, -1, -1);
                    VLD128(v0, ed, fixedaddress+16);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x11:
            INST_NAME("VMOVUPS Ex,Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                ed = (nextop&7)+(rex.b<<3);
                v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                VMOVQ(v1, v0);
                if(vex.l) {
                    v0 = ymm_get_reg(dyn, ninst, x1, gd, 0, ed, -1, -1);
                    v1 = ymm_get_reg_empty(dyn, ninst, x1, ed, gd, -1, -1);
                    VMOVQ(v1, v0);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xffe<<4, 15, rex, NULL, 0, 0);
                VST128(v0, ed, fixedaddress);
                if(vex.l) {
                    v0 = ymm_get_reg(dyn, ninst, x1, gd, 0, ed, -1, -1);
                    VST128(v0, ed, fixedaddress+16);
                }
                SMWRITE2();
            }
            break;
        case 0x12:
            nextop = F8;
            GETVX(v2, 0);
            if(MODREG) {
                INST_NAME("VMOVHLPS Gx, Vx, Ex");
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                GETGX_empty(v0);
                if((v0!=v2) && (v0!=v1))
                    VMOVQ(v0, v2);
                VMOVeD(v0, 0, v1, 1);
                if((v0!=v2) && (v0==v1))
                    VMOVeD(v0, 1, v2, 1);
            } else {
                INST_NAME("VMOVLPS Gx, Vx, Ex");
                GETGX_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                if(v0!=v2)
                    VMOVQ(v0, v2);
                VLD1_64(v0, 0, ed);
            }
            YMM0(gd);
            break;
        case 0x13:
            nextop = F8;
            INST_NAME("VMOVLPS Ex, Vx, Gx");
            GETGX(v0, 0);
            GETVX(v2, 0);
            if(v1!=v2)
                VMOVQ(v0, v2);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 1);
                if(v0!=v1) VMOVeD(v1, 0, v0, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                VST1_64(v0, 0, ed);  // better to use VST1 than VSTR_64, to avoid NEON->VFPU transfert I assume
                SMWRITE2();
            }
            break;
        case 0x14:
            INST_NAME("VUNPCKLPS Gx, Vx, Ex");
            nextop = F8;
            SMREAD();
            GETGX_empty_VXEX(v0, v2, v1, 0);
            VZIP1Q_32(v0, v2, v1);
            if(vex.l) {
                GETGY_empty_VYEY(v0, v2, v1);
                VZIP1Q_32(v0, v2, v1);
            } else YMM0(gd);
            break;
        case 0x15:
            INST_NAME("VUNPCKHPS Gx, Vx, Ex");
            nextop = F8;
            SMREAD();
            GETGX_empty_VXEX(v0, v2, v1, 0);
            VZIP2Q_32(v0, v2, v1);
            if(vex.l) {
                GETGY_empty_VYEY(v0, v2, v1);
                VZIP2Q_32(v0, v2, v1);
            } else YMM0(gd);
            break;

        case 0x57:
            INST_NAME("VXORPS Gx, Vx, Ex");
            nextop = F8;
            GETGX_empty_VXEX(v0, v2, v1, 0);
            VEORQ(v0, v2, v1);
            if(vex.l) {
                GETGY_empty_VYEY(v0, v2, v1);
                VEORQ(v0, v2, v1);
            } else YMM0(gd)
            break;
        case 0x58:
            INST_NAME("VADDPS Gx, Vx, Ex");
            nextop = F8;
            GETGX_empty_VXEX(v0, v2, v1, 0);
            VFADDQS(v0, v2, v1);
            if(vex.l) {
                GETGY_empty_VYEY(v0, v2, v1);
                VFADDQS(v0, v2, v1);
            } else YMM0(gd)
            break;
        case 0x59:
            INST_NAME("VMULPS Gx, Vx, Ex");
            nextop = F8;
            GETGX_empty_VXEX(v0, v2, v1, 0);
            VFMULQS(v0, v2, v1);
            if(vex.l) {
                GETGY_empty_VYEY(v0, v2, v1);
                VFMULQS(v0, v2, v1);
            } else YMM0(gd)
            break;

        case 0x77:
            INST_NAME("VZEROUPPER");
            if(!vex.l) {
                if(vex.v!=0) {
                    UDF(0);
                } else {
                    for(int i=0; i<(rex.is32bits?8:16); ++i) {
                        YMM0(i);
                    }
                }
            } else {
                DEFAULT;
            }
            break;

        case 0xAE:
            nextop = F8;
            if(MODREG) {
                DEFAULT;
            } else
                switch((nextop>>3)&7) {
                    case 2:
                        INST_NAME("VLDMXCSR Md");
                        GETED(0);
                        STRw_U12(ed, xEmu, offsetof(x64emu_t, mxcsr));
                        if(box64_sse_flushto0) {
                            MRS_fpcr(x1);                   // get fpscr
                            LSRw_IMM(x3, ed, 15);           // get FZ bit
                            BFIw(x1, x3, 24, 1);            // inject FZ bit
                            EORw_REG_LSR(x3, x3, ed, 1);    // FZ xor DAZ
                            BFIw(x1, x3, 1, 1);             // inject AH bit
                            MSR_fpcr(x1);                   // put new fpscr
                        }
                        break;
                    case 3:
                        INST_NAME("VSTMXCSR Md");
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                        LDRw_U12(x4, xEmu, offsetof(x64emu_t, mxcsr));
                        STW(x4, ed, fixedaddress);
                        break;
                    default:
                        DEFAULT;
                }
            break;

        case 0xC6:
            INST_NAME("VSHUFPS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETVX(v2, 0);
            if(!MODREG) {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                v1 = -1; // to avoid a warning
            } else
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
            GETGX_empty(v0);
            u8 = F8;
            if(v2==v1 && (u8&0x3)==((u8>>2)&3) && (u8&0xf)==((u8>>4)&0xf)) {
                VDUPQ_32(v0, v2, u8&3);
            } else if(v2==v1 && (u8==0xe0)) {   // easy special case
                VMOVQ(v0, v2);
                VMOVeS(v0, 1, v0, 0);
            } else if(v2==v1 && (u8==0xe5)) {   // easy special case
                VMOVQ(v0, v2);
                VMOVeS(v0, 0, v0, 1);
            } else {
                d0 = fpu_get_scratch(dyn, ninst);
                // first two elements from Vx
                for(int i=0; i<2; ++i) {
                    VMOVeS(d0, i, v2, (u8>>(i*2))&3);
                }
                // second two from Ex
                if(MODREG) {
                    for(int i=2; i<4; ++i) {
                        VMOVeS(d0, i, v1, (u8>>(i*2))&3);
                    }
                } else {
                    SMREAD();
                    for(int i=2; i<4; ++i) {
                        ADDx_U12(x2, ed, ((u8>>(i*2))&3)*4);
                        VLD1_32(d0, i, x2);
                    }
                }
                VMOVQ(v0, d0);
            }
            if(vex.l) {
                if(MODREG)
                    v1 = ymm_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0, gd, vex.v, -1);
                GETGY_empty_VY(v0, v2, 0, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1);
                if(v2==v1 && (u8&0x3)==((u8>>2)&3) && (u8&0xf)==((u8>>4)&0xf)) {
                    VDUPQ_32(v0, v2, u8&3);
                } else if(v2==v1 && (u8==0xe0)) {
                    VMOVQ(v0, v2);
                    VMOVeS(v0, 1, v0, 0);
                } else if(v2==v1 && (u8==0xe5)) {
                    VMOVQ(v0, v2);
                    VMOVeS(v0, 0, v0, 1);
                } else {
                    for(int i=0; i<2; ++i) {
                        VMOVeS(d0, i, v2, (u8>>(i*2))&3);
                    }
                    if(MODREG) {
                        for(int i=2; i<4; ++i) {
                            VMOVeS(d0, i, v1, (u8>>(i*2))&3);
                        }
                    } else {
                        SMREAD();
                        for(int i=2; i<4; ++i) {
                            ADDx_U12(x2, ed, 16+((u8>>(i*2))&3)*4);
                            VLD1_32(d0, i, x2);
                        }
                    }
                    VMOVQ(v0, d0);
                }
            } else YMM0(gd);
            break;

        default:
            DEFAULT;
    }
    return addr;
}
