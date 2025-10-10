#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
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
#include "../dynarec_helper.h"

uintptr_t dynarec64_AVX_F2_0F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
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
            INST_NAME("VMOVSD Gx, [Vx,] Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                GETGX_empty_VXEX(v0, v2, v1, 0);
                if(v0!=v1) VMOVeD(v0, 0, v1, 0);
                if(v0!=v2) VMOVeD(v0, 1, v2, 1);
            } else {
                SMREAD();
                GETGX_empty(v0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                VLD64(v0, ed, fixedaddress); // upper part reseted
            }
            YMM0(gd);
            break;
        case 0x11:
            INST_NAME("VMOVSD Ex, Vx, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                GETVXEX(v2, 0, v1, 1, 0);
                if(v0!=v1) VMOVeD(v1, 0, v0, 0);
                if(v1!=v2) VMOVeD(v1, 1, v2, 1);
                YMM0((nextop&7)+(rex.b<<3));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                VST64(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("VMOVDDUP Gx, Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                GETGX_empty(v0);
                VDUPQ_64(v0, v1, 0);
                if(vex.l) {
                    GETGY_empty_EY(v0, v1);
                    VDUPQ_64(v0, v1, 0);
                }
            } else {
                SMREAD();
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                VLDQ1R_64(v0, ed);
                if(vex.l) {
                    GETGY_empty(v0, -1, -1, -1);
                    ADDx_U12(x3, ed, 16);
                    VLDQ1R_64(v0, x3);
                }
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0x2A:
            INST_NAME("VCVTSI2SD Gx, Vx, Ed");
            nextop = F8;
            GETGX_empty_VX(v0, v1);
            GETED(0);
            d1 = fpu_get_scratch(dyn, ninst);
            if(BOX64ENV(dynarec_fastround)<2) {
                u8 = sse_setround(dyn, ninst, x3, x4, x5);
            }
            if(rex.w) {
                SCVTFDx(d1, ed);
            } else {
                SCVTFDw(d1, ed);
            }
            if(BOX64ENV(dynarec_fastround)<2) {
                x87_restoreround(dyn, ninst, u8);
            }
            if(v0!=v1) VMOVQ(v0, v1);
            VMOVeD(v0, 0, d1, 0);
            YMM0(gd);
            break;

        case 0x2C:
            INST_NAME("VCVTTSD2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEXSD(q0, 0, 0);
            if(!BOX64ENV(dynarec_fastround) && !cpuext.frintts) {
                MRS_fpsr(x5);
                BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                MSR_fpsr(x5);
            }
            if(!BOX64ENV(dynarec_fastround) && cpuext.frintts) {
                v0 = fpu_get_scratch(dyn, ninst);
                if(rex.w) {
                    FRINT64ZD(v0, q0);
                } else {
                    FRINT32ZD(v0, q0);
                }
                FCVTZSxwD(gd, v0);
            } else {
                FCVTZSxwD(gd, q0);
            }
            if(!BOX64ENV(dynarec_fastround) && !cpuext.frintts) {
                MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                TBZ_NEXT(x5, FPSR_IOC);
                if(rex.w) {
                    ORRx_mask(gd, xZR, 1, 1, 0);    //0x8000000000000000
                } else {
                    MOV32w(gd, 0x80000000);
                }
            }
            break;
        case 0x2D:
            INST_NAME("VCVTSD2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEXSD(q0, 0, 0);
            if(!BOX64ENV(dynarec_fastround) && !cpuext.frintts) {
                MRS_fpsr(x5);
                BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                MSR_fpsr(x5);
            }
            u8 = sse_setround(dyn, ninst, x1, x2, x3);
            d1 = fpu_get_scratch(dyn, ninst);
            if(!BOX64ENV(dynarec_fastround) && cpuext.frintts) {
                if(rex.w) {
                    FRINT64XD(d1, q0);
                } else {
                    FRINT32XD(d1, q0);
                }
            } else {
                FRINTID(d1, q0);
            }
            x87_restoreround(dyn, ninst, u8);
            FCVTZSxwD(gd, d1);
            if(!BOX64ENV(dynarec_fastround) && !cpuext.frintts) {
                MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                TBZ_NEXT(x5, FPSR_IOC);
                if(rex.w) {
                    ORRx_mask(gd, xZR, 1, 1, 0);    //0x8000000000000000
                } else {
                    MOV32w(gd, 0x80000000);
                }
            }
            break;

        case 0x51:
            INST_NAME("VSQRTSD Gx, Vx, Ex");
            nextop = F8;
            d1 = fpu_get_scratch(dyn, ninst);
            GETEXSD(v1, 0, 0);
            GETGX_empty_VX(v0, v2);
            if(!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn, ninst);
                q1 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                FCMEQD(q0, v1, v1);    // 0 if NAN, 1 if not NAN
            }
            FSQRTD(d1, v1);
            if(!BOX64ENV(dynarec_fastnan)) {
                FCMEQD(q1, d1, d1);    // 0 => out is NAN
                VBIC(q1, q0, q1);      // forget it in any input was a NAN already
                SHL_64(q1, q1, 63);   // only keep the sign bit
                VORR(d1, d1, q1);      // NAN -> -NAN
            }
            if(v0!=v2) {
                VMOVQ(v0, v2);
            }
            VMOVeD(v0, 0, d1, 0);
            YMM0(gd)
            break;

        case 0x58:
            INST_NAME("VADDSD Gx, Vx, Ex");
            nextop = F8;
            q2 = fpu_get_scratch(dyn, ninst);
            GETEXSD(v1, 0, 0);
            GETGX_empty_VX(v0, v2);
            if(!BOX64ENV(dynarec_fastnan)) {
                q1 = fpu_get_scratch(dyn, ninst);
                q0 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                FMAXD(q1, v0, v1);    // propagate NAN
                FCMEQD(q1, q1, q1);    // 0 if NAN, 1 if not NAN
                FADDD(q2, v1, v2);  // the high part of the vector is erased...
                FCMEQD(q0, q2, q2);    // 0 => out is NAN
                VBIC(q0, q1, q0);      // forget it in any input was a NAN already
                SHL_64(q0, q0, 63);     // only keep the sign bit
                VORR(q2, q2, q0);      // NAN -> -NAN
            } else {
                FADDD(q2, v1, v2);  // the high part of the vector is erased...
            }
            if(v0!=v2) {
                VMOVQ(v0, v2);
            }
            VMOVeD(v0, 0, q2, 0);
            YMM0(gd)
            break;
        case 0x59:
            INST_NAME("VMULSD Gx, Vx, Ex");
            nextop = F8;
            q2 = fpu_get_scratch(dyn, ninst);
            GETEXSD(v1, 0, 0);
            GETGX_empty_VX(v0, v2);
            if(!BOX64ENV(dynarec_fastnan)) {
                q1 = fpu_get_scratch(dyn, ninst);
                q0 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                FMAXD(q1, v0, v1);    // propagate NAN
                FCMEQD(q1, q1, q1);    // 0 if NAN, 1 if not NAN
                FMULD(q2, v1, v2);  // the high part of the vector is erased...
                FCMEQD(q0, q2, q2);    // 0 => out is NAN
                VBIC(q0, q1, q0);      // forget it in any input was a NAN already
                SHL_64(q0, q0, 63);     // only keep the sign bit
                VORR(q2, q2, q0);      // NAN -> -NAN
            } else {
                FMULD(q2, v1, v2);  // the high part of the vector is erased...
            }
            if(v0!=v2) {
                VMOVQ(v0, v2);
            }
            VMOVeD(v0, 0, q2, 0);
            YMM0(gd);
            break;
        case 0x5A:
            INST_NAME("VCVTSD2SS Gx, Vx, Ex");
            nextop = F8;
            d1 = fpu_get_scratch(dyn, ninst);
            GETEXSD(v1, 0, 0);
            GETGX_empty_VX(v0, v2);
            if(BOX64ENV(dynarec_fastround)==2) {
                FCVT_S_D(d1, v1);
            } else {
                u8 = sse_setround(dyn, ninst, x1, x2, x3);
                FCVT_S_D(d1, v1);
                x87_restoreround(dyn, ninst, u8);
            }
            if(v0!=v2) {
                VMOVQ(v0, v2);
            }
            VMOVeS(v0, 0, d1, 0);
            YMM0(gd)
            break;

        case 0x5C:
            INST_NAME("VSUBSD Gx, Vx, Ex");
            nextop = F8;
            q2 = fpu_get_scratch(dyn, ninst);
            GETEXSD(v1, 0, 0);
            GETGX_empty_VX(v0, v2);
            if(!BOX64ENV(dynarec_fastnan)) {
                q1 = fpu_get_scratch(dyn, ninst);
                q0 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                FMAXD(q1, v0, v1);    // propagate NAN
                FCMEQD(q1, q1, q1);    // 0 if NAN, 1 if not NAN
                FSUBD(q2, v2, v1);  // the high part of the vector is erased...
                FCMEQD(q0, q2, q2);    // 0 => out is NAN
                VBIC(q0, q1, q0);      // forget it in any input was a NAN already
                SHL_64(q0, q0, 63);     // only keep the sign bit
                VORR(q2, q2, q0);      // NAN -> -NAN
            } else {
                FSUBD(q2, v2, v1);  // the high part of the vector is erased...
            }
            if(v0!=v2) {
                VMOVQ(v0, v2);
            }
            VMOVeD(v0, 0, q2, 0);
            YMM0(gd)
            break;
        case 0x5D:
            INST_NAME("VMINSD Gx, Vx, Ex");
            nextop = F8;
            d1 = fpu_get_scratch(dyn, ninst);
            GETEXSD(v1, 0, 0);
            GETGX_empty_VX(v0, v2);
            // VMINSD: if any input is NaN, or Ex[0]<Gx[0], copy Ex[0] -> Gx[0]
            FCMPD(v2, v1);
            FCSELD(d1, v1, v2, cCS);    //CS: NAN or == or Vx > Ex
            if(v0!=v2) {
                VMOVQ(v0, v2);
            }
            VMOVeD(v0, 0, d1, 0);   // to not erase uper part
            YMM0(gd)
            break;
        case 0x5E:
            INST_NAME("VDIVSD Gx, Vx, Ex");
            nextop = F8;
            q2 = fpu_get_scratch(dyn, ninst);
            GETEXSD(v1, 0, 0);
            GETGX_empty_VX(v0, v2);
            if(!BOX64ENV(dynarec_fastnan)) {
                q1 = fpu_get_scratch(dyn, ninst);
                q0 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                FMAXD(q1, v0, v1);    // propagate NAN
                FCMEQD(q1, q1, q1);    // 0 if NAN, 1 if not NAN
                FDIVD(q2, v2, v1);  // the high part of the vector is erased...
                FCMEQD(q0, q2, q2);    // 0 => out is NAN
                VBIC(q0, q1, q0);      // forget it in any input was a NAN already
                SHL_64(q0, q0, 63);     // only keep the sign bit
                VORR(q2, q2, q0);      // NAN -> -NAN
            } else {
                FDIVD(q2, v2, v1);  // the high part of the vector is erased...
            }
            if(v0!=v2) {
                VMOVQ(v0, v2);
            }
            VMOVeD(v0, 0, q2, 0);
            YMM0(gd)
            break;
        case 0x5F:
            INST_NAME("VMAXSD Gx, Vx, Ex");
            nextop = F8;
            d1 = fpu_get_scratch(dyn, ninst);
            GETEXSD(v1, 0, 0);
            GETGX_empty_VX(v0, v2);
            FCMPD(v1, v2);
            FCSELD(d1, v1, v2, cCS);    //CS: NAN or == or Ex > Vx
            if(v0!=v2) {
                VMOVQ(v0, v2);
            }
            VMOVeD(v0, 0, d1, 0);   // to not erase uper part
            YMM0(gd)
            break;

        case 0x70:
            INST_NAME("VPSHUFLW Gx, Ex, Ib");
            nextop = F8;
            d0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETEX_Y(v1, 0, 1); GETGX(v0, 1); u8 = F8; } else { GETGY(v0, 1, MODREG?((nextop&7)+(rex.b<<3)):-1, -1, -1); GETEY(v1); }
                if(u8==0b00000000 || u8==0b01010101 || u8==0b10101010 || u8==0b11111111) {
                    if(v0==v1) {
                        VMOVQ(d0, v1);
                    }
                    VDUP_16(v0, v1, u8&3);
                    if(v0==v1)
                        v1 = d0;
                } else {
                    // only low part need to be suffled. VTBL only handle 8bits value, so the 16bits suffles need to be changed in 8bits
                    if(!l) {
                        u64 = 0;
                        for (int i=0; i<4; ++i) {
                            u64 |= ((uint64_t)((u8>>(i*2))&3)*2+0)<<(i*16+0);
                            u64 |= ((uint64_t)((u8>>(i*2))&3)*2+1)<<(i*16+8);
                        }
                        MOV64x(x2, u64);
                    }
                    VMOVQDfrom(d0, 0, x2);
                    VTBL1_8(d0, v1, d0);
                    VMOVeD(v0, 0, d0, 0);
                }
                if(v0!=v1) {
                    VMOVeD(v0, 1, v1, 1);
                }
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0x7C:
            INST_NAME("VHADDPS Gx, Vx, Ex");
            nextop = F8;
            if(!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn, ninst);
                q1 = fpu_get_scratch(dyn, ninst);
            }
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                if(!BOX64ENV(dynarec_fastnan)) {
                    // check if any input value was NAN
                    VFMAXPQS(q0, v2, v1);    // propagate NAN
                    VFCMEQQS(q0, q0, q0);    // 0 if NAN, 1 if not NAN
                }
                VFADDPQS(v0, v2, v1);
                if(!BOX64ENV(dynarec_fastnan)) {
                    VFCMEQQS(q1, v0, v0);    // 0 => out is NAN
                    VBICQ(q1, q0, q1);      // forget it in any input was a NAN already
                    VSHLQ_32(q1, q1, 31);   // only keep the sign bit
                    VORRQ(v0, v0, q1);      // NAN -> -NAN
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x7D:
            INST_NAME("VHSUBPS Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            if(!BOX64ENV(dynarec_fastnan))
                q1 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VUZP1Q_32(q0, v2, v1);
                VUZP2Q_32(v0, v2, v1);
                if(!BOX64ENV(dynarec_fastnan)) {
                    // check if any input value was NAN
                    // but need to mix low/high part
                    VFMAXQS(q1, v0, q0);    // propagate NAN
                    VFCMEQQS(q1, q1, q1);    // 0 if NAN, 1 if not NAN
                }
                VFSUBQS(v0, q0, v0);
                if(!BOX64ENV(dynarec_fastnan)) {
                    VFCMEQQS(q0, v0, v0);    // 0 => out is NAN
                    VBICQ(q1, q1, q0);      // forget it in any input was a NAN already
                    VSHLQ_32(q1, q1, 31);   // only keep the sign bit
                    VORRQ(v0, v0, q1);      // NAN -> -NAN
                }
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0xC2:
            INST_NAME("VCMPSD Gx, Ex, Ib");
            nextop = F8;
            GETEXSD(v1, 0, 1);
            GETGX_empty_VX(v0, v2);
            u8 = F8;
            if(((u8&15)!=0x0b) && ((u8&15)!=0x0f)) {
                if((u8&15)>7)
                    FCMPD(v1, v2);
                else
                    FCMPD(v2, v1);
            }
            // TODO: create a test for this one, there might be an issue with cases 9, 10 and 13
            if(v0!=v2) VMOVQ(v0, v2);
            switch(u8&15) {
                case 0x00: CSETMx(x2, cEQ); break;  // Equal
                case 0x01: CSETMx(x2, cCC); break;  // Less than
                case 0x02: CSETMx(x2, cLS); break;  // Less or equal
                case 0x03: CSETMx(x2, cVS); break;  // NaN
                case 0x04: CSETMx(x2, cNE); break;  // Not Equal or unordered
                case 0x05: CSETMx(x2, cCS); break;  // Greater or equal or unordered
                case 0x06: CSETMx(x2, cHI); break;  // Greater or unordered
                case 0x07: CSETMx(x2, cVC); break;  // not NaN
                case 0x08: CSETMx(x2, cEQ); CSETMx(x3, cVS); ORRx_REG(x2, x2, x3); break;  // Equal or unordered
                case 0x09: CSETMx(x2, cHI); break;  // Less than or unordered
                case 0x0a: CSETMx(x2, cCS); break;  // Less or equal or unordered
                case 0x0b: MOV32w(x2, 0); break;    // false
                case 0x0c: CSETMx(x2, cNE); CSETMx(x3, cVS); BICx(x2, x2, x3); break;  // Not Equal not unordered
                case 0x0d: CSETMx(x2, cLS); break;  // Greater or equal not unordered
                case 0x0e: CSETMx(x2, cCC); break;  // Greater not unordered
                case 0x0f: MOV64x(x2, 0xffffffffffffffffLL); break; // true
            }
            VMOVQDfrom(v0, 0, x2);
            YMM0(gd);
            break;

        case 0xD0:
            INST_NAME("VADDSUBPS Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            TABLE64C(x2, const_4f_m1_1_m1_1);
            VLDR128_U12(q0, x2, 0);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                if(v0==v1) {
                    //TODO: find a better way
                    if(!l) q1 = fpu_get_scratch(dyn, ninst);
                    VMOVQ(q1, v2);
                    VFMLAQS(q1, v1, q0);
                    VMOVQ(v0, q1);
                } else {
                    if(v0!=v2) VMOVQ(v0, v2);
                    VFMLAQS(v0, v1, q0);
                }
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0xE6:
            INST_NAME("VCVTPD2DQ Gx, Ex");
            nextop = F8;
            u8 = sse_setround(dyn, ninst, x1, x2, x6);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETEX_Y(v1, 0, 0);
                    GETGX_empty(v0);
                    if(!BOX64ENV(dynarec_fastround) || vex.l)
                        d0 = fpu_get_scratch(dyn, ninst);
                } else {
                    GETEY(v1);
                }
                if(BOX64ENV(dynarec_fastround)) {
                    VFRINTIDQ(l?d0:v0, v1);
                    VFCVTNSQD(l?d0:v0, l?d0:v0);  // convert double -> int64
                    if(!l)
                        SQXTN_32(v0, v0);   // convert int64 -> int32 with saturation in lower part, RaZ high part
                    else
                        SQXTN2_32(v0, d0);   // convert int64 -> int32 with saturation in higher part
                } else {
                    if(cpuext.frintts) {
                        VFRINT32XDQ(l?d0:v0, v1);    // round, handling of overflow and Nan to 0x80000000
                        VFCVTNSQD(l?d0:v0, l?d0:v0);  // convert double -> int64
                        if(!l)
                            SQXTN_32(v0, v0);   // convert int64 -> int32 with saturation in lower part, RaZ high part
                        else
                            SQXTN2_32(v0, d0);   // convert int64 -> int32 with saturation in higher part
                    } else {
                        if(!l) {
                            MRS_fpsr(x5);
                            ORRw_mask(x4, xZR, 1, 0);    //0x80000000
                        }
                        for(int i=0; i<2; ++i) {
                            BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                            MSR_fpsr(x5);
                            if(i) {
                                VMOVeD(d0, 0, v1, i);
                                FRINTID(d0, d0);
                            } else {
                                FRINTID(d0, v1);
                            }
                            FCVTZSwD(x1, d0);
                            MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                            TSTw_mask(x5, 0, 0);    // mask = 1 = FPSR_IOC
                            CSELx(x1, x1, x4, cEQ);
                            VMOVQSfrom(v0, i+l*2, x1);
                        }
                        if(!vex.l && !l) VMOVQDfrom(v0, 1, xZR);
                    }
                }
            }
            x87_restoreround(dyn, ninst, u8);
            YMM0(gd);
            break;

        case 0xF0:
            INST_NAME("VLDDQU Gx,Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VMOVQ(v0, v1);
                if(vex.l) {
                    GETGY_empty_EY(v0, v1);
                    VMOVQ(v0, v1);
                }
            } else {
                SMREAD();
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                if (vex.l) {
                    v1 = ymm_get_reg_empty(dyn, ninst, x1, gd, -1, -1, -1);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, 0x3f << 4, 15, rex, NULL, 1, 0);
                    VLDP128_I7(v0, v1, ed, fixedaddress);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff << 4, 15, rex, NULL, 0, 0);
                    VLD128(v0, ed, fixedaddress);
                }
            }
            if(!vex.l) YMM0(gd);
            break;

        default:
            DEFAULT;
    }
    return addr;
}
