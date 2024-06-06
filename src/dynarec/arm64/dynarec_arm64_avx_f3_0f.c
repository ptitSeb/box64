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

uintptr_t dynarec64_AVX_F3_0F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed, vd;
    uint8_t wback, wb1, wb2;
    uint8_t eb1, eb2, gb1, gb2;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1, v2;
    int q0, q1, q2;
    int d0, d1, d2;
    int s0;
    uint64_t tmp64u, u64;
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

    rex_t rex = vex.rex;

    switch(opcode) {

        case 0x10:
            INST_NAME("VMOVSS Gx, Ex [,Vx]");
            nextop = F8;
            GETG;
            if(MODREG) {
                GETGX_empty_VXEX(v0, v2, v1, 0);
                if((v0!=v2) && (v0!=v1)) VMOVQ(v0, v2);
                if(v0!=v1) VMOVeS(v0, 0, v1, 0);
                if((v0!=v2) && (v0==v1)) { VMOVeS(v0, 1, v2, 1); VMOVeD(v0, 1, v2, 1);}
            } else {
                GETGX_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                VLD32(v0, ed, fixedaddress);
            }
            YMM0(gd);
            break;
        case 0x11:
            INST_NAME("VMOVSS Ex, Gx [,Vx]");
            nextop = F8;
            GETG;
            if(MODREG) {
                GETGXVXEX_empty(v0, v2, v1, 0);
                if((v2!=v1) && (v2!=v0)) VMOVQ(v2, v1);
                if(v2!=v0) VMOVeS(v2, 0, v0, 0);
                if((v2!=v1) && (v2==v0)) { VMOVeS(v2, 1, v0, 1); VMOVeD(v2, 1, v0, 1);}
                YMM0((nextop&7)+(rex.b<<3));
            } else {
                v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                VST32(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("VMOVSLDUP Gx, Ex");
            nextop = F8;
            GETEX_Y(q1, 0, 0);
            GETGX_empty(q0);
            VTRNQ1_32(q0, q1, q1);
            if(vex.l) {
                GETGY_empty_EY(q0, q1);
                VTRNQ1_32(q0, q1, q1);
            } else YMM0(gd);
            break;

        case 0x16:
            INST_NAME("MOVSHDUP Gx, Ex");
            nextop = F8;
            GETEX_Y(q1, 0, 0);
            GETGX_empty(q0);
            VTRNQ2_32(q0, q1, q1);
            if(vex.l) {
                GETGY_empty_EY(q0, q1);
                VTRNQ2_32(q0, q1, q1);
            } else YMM0(gd);
            break;

        case 0x2A:
            INST_NAME("VCVTSI2SS Gx, Vx, Ed");
            nextop = F8;
            d1 = fpu_get_scratch(dyn, ninst);
            GETGX_empty_VX(v0, v1);
            GETED(0);
            if(rex.w) {
                SCVTFSx(d1, ed);
            } else {
                SCVTFSw(d1, ed);
            }
            if(v0!=v1) VMOVQ(v0, v1);
            VMOVeS(v0, 0, d1, 0);
            YMM0(gd);
            break;

        case 0x2C:
            INST_NAME("VCVTTSS2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEXSS(d0, 0, 0);
            if(!box64_dynarec_fastround) {
                MRS_fpsr(x5);
                BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                MSR_fpsr(x5);
            }
            FCVTZSxwS(gd, d0);
            if(!box64_dynarec_fastround) {
                MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                TBZ_NEXT(x5, FPSR_IOC);
                if(rex.w) {
                    ORRx_mask(gd, xZR, 1, 1, 0);    //0x8000000000000000
                } else {
                    ORRw_mask(gd, xZR, 1, 0);    //0x80000000
                }
            }
            break;
        case 0x2D:
            INST_NAME("VCVTSS2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEXSS(q0, 0, 0);
            if(!box64_dynarec_fastround) {
                MRS_fpsr(x5);
                BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                MSR_fpsr(x5);
            }
            u8 = sse_setround(dyn, ninst, x1, x2, x3);
            d1 = fpu_get_scratch(dyn, ninst);
            FRINTIS(d1, q0);
            x87_restoreround(dyn, ninst, u8);
            FCVTZSxwS(gd, d1);
            if(!box64_dynarec_fastround) {
                MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                TBZ_NEXT(x5, FPSR_IOC);
                if(rex.w) {
                    ORRx_mask(gd, xZR, 1, 1, 0);    //0x8000000000000000
                } else {
                    ORRw_mask(gd, xZR, 1, 0);    //0x80000000
                }
            }
            break;

        case 0x51:
            INST_NAME("SQRTSS Gx, Ex");
            nextop = F8;
            GETEXSS(d0, 0, 0);
            GETGX_empty_VX(v0, v2);
            d1 = fpu_get_scratch(dyn, ninst);
            FSQRTS(d1, d0);
            if(v0!=v2) VMOVQ(v0, v2);
            VMOVeS(v0, 0, d1, 0);
            YMM0(gd);
            break;
        case 0x52:
            INST_NAME("VRSQRTSS Gx, Vx Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETVX(v2, 0);
            GETEXSS(v1, 0, 0);
            d0 = fpu_get_scratch(dyn, ninst);
            d1 = fpu_get_scratch(dyn, ninst);
            // so here: F32: Imm8 = abcd efgh that gives => aBbbbbbc defgh000 00000000 00000000
            // and want 1.0f = 0x3f800000
            // so 00111111 10000000 00000000 00000000
            // a = 0, b = 1, c = 1, d = 1, efgh=0
            // 0b01110000
            FMOVS_8(d0, 0b01110000);
            FSQRTS(d1, v1);
            FDIVS(d0, d0, d1);
            if(v0!=v2) {
                VMOVQ(v0, v2);
            }
            VMOVeS(v0, 0, d0, 0);
            YMM0(gd);
            break;

        case 0x58:
            INST_NAME("VADDSS Gx, Vx, Ex");
            nextop = F8;
            d1 = fpu_get_scratch(dyn, ninst);
            GETEXSS(v1, 0, 0);
            GETGX_empty_VX(v0, v2);
            if(v0!=v2) {
                if(v0==v1)  {
                    VMOV(d1, v1);
                    v1 = d1;
                }
                VMOVQ(v0, v2);
            }
            FADDS(d1, v0, v1);
            VMOVeS(v0, 0, d1, 0);
            YMM0(gd)
            break;
        case 0x59:
            INST_NAME("VMULSS Gx, Vx, Ex");
            nextop = F8;
            d1 = fpu_get_scratch(dyn, ninst);
            GETEXSS(v1, 0, 0);
            GETGX_empty_VX(v0, v2);
            if(v0!=v2) {
                if(v0==v1)  {
                    VMOV(d1, v1);
                    v1 = d1;
                }
                VMOVQ(v0, v2);
            }
            FMULS(d1, v0, v1);
            VMOVeS(v0, 0, d1, 0);
            YMM0(gd)
            break;
        case 0x5A:
            INST_NAME("VCVTSS2SD Gx, Vx, Ex");
            nextop = F8;
            d1 = fpu_get_scratch(dyn, ninst);
            GETEXSS(v1, 0, 0);
            GETGX_empty_VX(v0, v2);
            if(v0!=v2) {
                if(v0==v1)  {
                    VMOV(d1, v1);
                    v1 = d1;
                }
                VMOVQ(v0, v2);
            }
            FCVT_D_S(d1, v1);
            VMOVeD(v0, 0, d1, 0);
            YMM0(gd);
            break;
        case 0x5B:
            INST_NAME("VCVTTPS2DQ Gx, Ex");
            nextop = F8;
            d0 = fpu_get_scratch(dyn, ninst);
            if(!box64_dynarec_fastround) {
                MRS_fpsr(x5);
                ORRw_mask(x4, xZR, 1, 0);    //0x80000000
            }
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_EX(v0, v1, 0); } else { GETGY_empty_EY(v0, v1); }
                if(box64_dynarec_fastround) {
                    VFCVTZSQS(v0, v1);
                } else {
                    BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                    MSR_fpsr(x5);
                    for(int i=0; i<4; ++i) {
                        BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                        MSR_fpsr(x5);
                        VMOVeS(d0, 0, v1, i);
                        VFCVTZSs(d0, d0);
                        MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                        TBZ(x5, FPSR_IOC, 4+4);
                        VMOVQSfrom(d0, 0, x4);
                        VMOVeS(v0, i, d0, 0);
                    }
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x5C:
            INST_NAME("VSUBSS Gx, Vx, Ex");
            nextop = F8;
            d1 = fpu_get_scratch(dyn, ninst);
            GETEXSS(v1, 0, 0);
            GETGX_empty_VX(v0, v2);
            FSUBS(d1, v2, v1);
            if(v0!=v2) {
                VMOVQ(v0, v2);
            }
            VMOVeS(v0, 0, d1, 0);
            YMM0(gd)
            break;
        case 0x5D:
            INST_NAME("VMINSS Gx, Vx, Ex");
            nextop = F8;
            d1 = fpu_get_scratch(dyn, ninst);
            GETEXSS(v1, 0, 0);
            GETGX_empty_VX(v0, v2);
            FCMPS(v2, v1);
            FCSELS(d1, v1, v2, cCS);
            if(v0!=v2) {
                VMOVQ(v0, v2);
            }
            VMOVeS(v0, 0, d1, 0);   // to not erase uper part
            YMM0(gd)
            break;
        case 0x5E:
            INST_NAME("VDIVSS Gx, Vx, Ex");
            nextop = F8;
            d1 = fpu_get_scratch(dyn, ninst);
            GETEXSS(v1, 0, 0);
            GETGX_empty_VX(v0, v2);
            FDIVS(d1, v2, v1);
            if(v0!=v2) {
                VMOVQ(v0, v2);
            }
            VMOVeS(v0, 0, d1, 0);
            YMM0(gd)
            break;
        case 0x5F:
            INST_NAME("VMAXSS Gx, Vx, Ex");
            nextop = F8;
            d1 = fpu_get_scratch(dyn, ninst);
            GETEXSS(v1, 0, 0);
            GETGX_empty_VX(v0, v2);
            FCMPS(v1, v2);
            FCSELS(d1, v1, v2, cCS);
            if(v0!=v2) {
                VMOVQ(v0, v2);
            }
            VMOVeS(v0, 0, d1, 0);   // to not erase uper part
            YMM0(gd)
            break;

        case 0x6F:
            INST_NAME("VMOVDQU Gx, Ex");// no alignment constraint on NEON here, so same as MOVDQA
            nextop = F8;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                GETGX_empty(v0);
                VMOVQ(v0, v1);
                if(vex.l) {
                    v1 = ymm_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0, gd, -12, -1);
                    GETGY_empty(v0, (nextop&7)+(rex.b<<3), -1, -1);
                    VMOVQ(v0, v1);
                }
            } else {
                GETGX_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0xffe<<4, 15, rex, NULL, 0, 0);
                VLDR128_U12(v0, ed, fixedaddress);
                if(vex.l) {
                    GETGY_empty(v0, -1, -1, -1);
                    VLDR128_U12(v0, ed, fixedaddress+16);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x70:
            INST_NAME("VPSHUFHW Gx, Ex, Ib");
            nextop = F8;
            d0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETEX(v1, 0, 1); GETGX(v0, 1); u8 = F8; } else { GETGY(v0, 1, MODREG?((nextop&7)+(rex.b<<3)):-1, -1, -1); GETEY(v1); }
                if(u8==0b00000000 || u8==0b01010101 || u8==0b10101010 || u8==0b11111111) {
                    VDUP_16(d0, v1, 4+(u8&3));
                } else {
                    // only high part need to be suffled. VTBL only handle 8bits value, so the 16bits suffles need to be changed in 8bits
                    if(!l) {
                        u64 = 0;
                        for (int i=0; i<4; ++i) {
                            u64 |= ((uint64_t)((u8>>(i*2))&3)*2+8)<<(i*16+0);
                            u64 |= ((uint64_t)((u8>>(i*2))&3)*2+9)<<(i*16+8);
                        }
                        MOV64x(x2, u64);
                    }
                    VMOVQDfrom(d0, 0, x2);
                    VTBL1_8(d0, v1, d0);
                }
                VMOVeD(v0, 1, d0, 0);
                if(v0!=v1) {
                    VMOVeD(v0, 0, v1, 0);
                }
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0x7E:
            INST_NAME("MOVQ Gx, Ex");
            nextop = F8;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7) + (rex.b<<3), 0);
                GETGX_empty(v0);
                FMOVD(v0, v1);
            } else {
                GETGX_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                VLD64(v0, ed, fixedaddress);
            }
            YMM0(gd);
            break;
        case 0x7F:
            INST_NAME("VMOVDQU Ex, Gx");
            nextop = F8;
            GETGX(v0, 0);
            if(MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop&7) + (rex.b<<3));
                VMOVQ(v1, v0);
                if(vex.l) {
                    GETGY(v0, 0, (nextop&7) + (rex.b<<3), -1, -1);
                    v1 = ymm_get_reg_empty(dyn, ninst, x1, (nextop&7) + (rex.b<<3), gd, -1, -1);
                    VMOVQ(v1, v0);
                } // no ymm raz here it seems
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0xffe<<4, 15, rex, NULL, 0, 0);
                VSTR128_U12(v0, ed, fixedaddress);
                if(vex.l) {
                    GETGY(v0, 0, -1, -1, -1);
                    VSTR128_U12(v0, ed, fixedaddress+16);
                }
                SMWRITE2();
            }
            break;

        case 0xC2:
            INST_NAME("VCMPSS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETEXSS(v1, 0, 1);
            GETGX_empty_VX(v0, v2);
            u8 = F8;
            if(((u8&15)!=11) && ((u8&15)!=15)) {
                if((u8&15)>7)
                    FCMPS(v1, v2);
                else
                    FCMPS(v2, v1);
            }
            // TODO: create a test for this one, there might be an issue with cases 9, 10 and 13
            if(v0!=v2) VMOVQ(v0, v2);
            switch(u8&15) {
                case 0x00: CSETMw(x2, cEQ); break;  // Equal
                case 0x01: CSETMw(x2, cCC); break;  // Less than
                case 0x02: CSETMw(x2, cLS); break;  // Less or equal
                case 0x03: CSETMw(x2, cVS); break;  // NaN
                case 0x04: CSETMw(x2, cNE); break;  // Not Equal or unordered
                case 0x05: CSETMw(x2, cCS); break;  // Greater or equal or unordered
                case 0x06: CSETMw(x2, cHI); break;  // Greater or unordered
                case 0x07: CSETMw(x2, cVC); break;  // not NaN
                case 0x08: CSETMw(x2, cEQ); CSETMw(x3, cVS); ORRw_REG(x2, x2, x3); break;  // Equal or unordered
                case 0x09: CSETMw(x2, cHI); break;  // Less than or unordered
                case 0x0a: CSETMw(x2, cCS); break;  // Less or equal or unordered
                case 0x0b: MOV32w(x2, 0); break;    // false
                case 0x0c: CSETMw(x2, cNE); CSETMw(x3, cVS); BICw(x2, x2, x3); break;  // Not Equal not unordered
                case 0x0d: CSETMw(x2, cLS); break;  // Greater or equal not unordered
                case 0x0e: CSETMw(x2, cCC); break;  // Greater not unordered
                case 0x0f: MOV32w(x2, 0xffffffff); break; // true
            }
            VMOVQSfrom(v0, 0, x2);
            YMM0(gd);
            break;

        case 0xE6:
            INST_NAME("VCVTDQ2PD Gx, Ex");
            nextop = F8;
            if(vex.l) {
                GETEX_Y(v1, 0, 0);
            } else {
                GETEXSD(v1, 0, 0);
            }
            GETGX_empty(v0);
            d0 = fpu_get_scratch(dyn, ninst);
            if(vex.l) {
                q0 = ymm_get_reg_empty(dyn, ninst, x1, gd, -1, -1, -1);
                SXTL2_32(q0, v1);
                SCVTQFD(q0, q0);
            } else YMM0(gd);
            SXTL_32(v0, v1);
            SCVTQFD(v0, v0);
            break;

        default:
            DEFAULT;
    }
    return addr;
}
