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

uintptr_t dynarec64_AVX_66_0F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
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
    uint64_t tmp64u, tmp64u2;
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

    /* Remember to not create a new fpu_scratch after some GY/VY/EY is created, because Y can be in the scratch area and might overlap (and scratch will win) */

    rex_t rex = vex.rex;

    switch(opcode) {

        case 0x10:
            INST_NAME("VMOVUPD Gx,Ex");
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
                if(vex.l) {
                    GETGY_empty(v1, -1, -1, -1);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0x3f<<4, 15, rex, NULL, 1, 0);
                    VLDP128_I7(v0, v1, ed, fixedaddress);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                    VLD128(v0, ed, fixedaddress);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x11:
            INST_NAME("VMOVUPD Ex,Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                ed = (nextop&7)+(rex.b<<3);
                v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                VMOVQ(v1, v0);
                if(vex.l) {
                    GETGYEY_empty(v0, v1);
                    VMOVQ(v1, v0);
                } else YMM0(ed);
            } else {
                if(vex.l) {
                    GETGY(v1, 0, -1, -1, -1);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0x3f<<4, 15, rex, NULL, 1, 0);
                    VSTP128_I7(v0, v1, ed, fixedaddress);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                    VST128(v0, ed, fixedaddress);
                }
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("VMOVLPD Gx, Vx, Eq");
            nextop = F8;
            if(MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            GETGX_empty_VX(v0, v2);
            SMREAD();
            addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
            VLD1_64(v0, 0, ed);
            if(v0!=v2)
                VMOVeD(v0, 1, v2, 1);
            YMM0(gd);
            break;
        case 0x13:
            INST_NAME("VMOVLPD Eq, Gx");
            nextop = F8;
            if(MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            GETGX(v0, 0);
            addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
            VST1_64(v0, 0, ed);
            SMWRITE2();
            break;
        case 0x14:
            INST_NAME("VUNPCKLPD Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {GETGX_empty_VXEX(v0, v2, v1, 0);} else {GETGY_empty_VYEY(v0, v2, v1);}
                VZIP1Q_64(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x15:
            INST_NAME("VUNPCKHPD Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {GETGX_empty_VXEX(v0, v2, v1, 0);} else {GETGY_empty_VYEY(v0, v2, v1);}
                VZIP2Q_64(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x16:
            INST_NAME("VMOVHPD Gx, Vx, Eq");
            nextop = F8;
            if(MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            GETGX_empty_VX(v0, v2);
            SMREAD();
            addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
            VLD1_64(v0, 1, ed);
            if(v0!=v2)
                VMOVeD(v0, 0, v2, 0);
            YMM0(gd);
            break;
        case 0x17:
            INST_NAME("VMOVHPD Eq, Gx");
            nextop = F8;
            if(MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            GETGX(v0, 0);
            addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
            VST1_64(v0, 1, ed);
            SMWRITE2();
            break;

        case 0x28:
            INST_NAME("VMOVAPD Gx,Ex");
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
                if(vex.l) {
                    GETGY_empty(v1, -1, -1, -1);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0x3f<<4, 15, rex, NULL, 1, 0);
                    VLDP128_I7(v0, v1, ed, fixedaddress);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                    VLD128(v0, ed, fixedaddress);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x29:
            INST_NAME("VMOVAPD Ex,Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                ed = (nextop&7)+(rex.b<<3);
                v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                VMOVQ(v1, v0);
                if(vex.l) {
                    GETGYEY_empty(v0, v1);
                    VMOVQ(v1, v0);
                } else YMM0(ed);
            } else {
                if(vex.l) {
                    GETGY(v1, 0, -1, -1, -1);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0x3f<<4, 15, rex, NULL, 1, 0);
                    VSTP128_I7(v0, v1, ed, fixedaddress);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                    VST128(v0, ed, fixedaddress);
                }
                SMWRITE2();
            }
            break;

        case 0x2B:
            INST_NAME("VMOVNTPD Ex,Gx");
            nextop = F8;
            GETG;
            if(MODREG) {
                DEFAULT;
            } else {
                v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                if(vex.l) {
                    GETGY(v1, 0, -1, -1, -1);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0x3f<<4, 15, rex, NULL, 1, 0);
                    VSTP128_I7(v0, v1, ed, fixedaddress);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                    VST128(v0, ed, fixedaddress);
                }
            }
            break;

        case 0x2E:
            // no special check...
        case 0x2F:
            if(opcode==0x2F) {INST_NAME("VCOMISD Gx, Ex");} else {INST_NAME("VUCOMISD Gx, Ex");}
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGX(v0, 0);
            GETEXSD(q0, 0, 0);
            FCMPD(v0, q0);
            FCOMI(x1, x2);
            break;

        case 0x50:
            nextop = F8;
            INST_NAME("VMOVMSKPD Gd, Ex");
            GETEX_Y(q0, 0, 0);
            GETGD;
            VMOVQDto(x1, q0, 0);
            VMOVQDto(gd, q0, 1);
            LSRx(gd, gd, 62);
            BFXILx(gd, x1, 63, 1);
            if(vex.l) {
                GETEY(q0);
                VMOVQDto(x1, q0, 0);
                VMOVQDto(x2, q0, 1);
                BFIx(x1, x2, 63, 1);
                RORx(x2, x1, 63);
                BFIx(gd, x2, 2, 2);
            }
            break;
        case 0x51:
            INST_NAME("VSQRTPD Gx, Ex");
            nextop = F8;
            if(!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn, ninst);
                d1 = fpu_get_scratch(dyn, ninst);
            }
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_EX(v0, v1, 0); } else { GETGY_empty_EY(v0, v1); }
                if(!BOX64ENV(dynarec_fastnan)) {
                    // check if any input value was NAN
                    VFCMEQQD(d0, v1, v1);    // 0 if NAN, 1 if not NAN
                    VFSQRTQD(v0, v1);
                    VFCMEQQD(d1, v0, v0);    // 0 => out is NAN
                    VBICQ(d1, d0, d1);      // forget it in any input was a NAN already
                    VSHLQ_64(d1, d1, 63);   // only keep the sign bit
                    VORRQ(v0, v0, d1);      // NAN -> -NAN
                } else {
                    VFSQRTQD(v0, v1);
                }
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0x54:
            INST_NAME("VANDPD Gx, Vx, Ex");
            nextop = F8;
            GETGX_empty_VXEX(v0, v2, v1, 0);
            VANDQ(v0, v2, v1);
            if(vex.l) {
                GETGY_empty_VYEY(v0, v2, v1);
                VANDQ(v0, v2, v1);
            } else YMM0(gd)
            break;
        case 0x55:
            INST_NAME("VANDNPD Gx, Vx, Ex");
            nextop = F8;
            GETGX_empty_VXEX(v0, v2, v1, 0);
            VBICQ(v0, v1, v2);
            if(vex.l) {
                GETGY_empty_VYEY(v0, v2, v1);
                VBICQ(v0, v1, v2);
            } else YMM0(gd)
            break;
        case 0x56:
            INST_NAME("VORPD Gx, Vx, Ex");
            nextop = F8;
            GETGX_empty_VXEX(v0, v2, v1, 0);
            VORRQ(v0, v2, v1);
            if(vex.l) {
                GETGY_empty_VYEY(v0, v2, v1);
                VORRQ(v0, v2, v1);
            } else YMM0(gd)
            break;
        case 0x57:
            INST_NAME("VXORPD Gx, Vx, Ex");
            nextop = F8;
            GETGX_empty_VXEX(v0, v2, v1, 0);
            VEORQ(v0, v2, v1);
            if(vex.l) {
                GETGY_empty_VYEY(v0, v2, v1);
                VEORQ(v0, v2, v1);
            } else YMM0(gd)
            break;
        case 0x58:
            INST_NAME("VADDPD Gx, Vx, Ex");
            nextop = F8;
            if(!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn, ninst);
                q1 = fpu_get_scratch(dyn, ninst);
            }
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                if(!BOX64ENV(dynarec_fastnan)) {
                    // check if any input value was NAN
                    VFMAXQD(q0, v2, v1);    // propagate NAN
                    VFCMEQQD(q0, q0, q0);    // 0 if NAN, 1 if not NAN
                }
                VFADDQD(v0, v2, v1);
                if(!BOX64ENV(dynarec_fastnan)) {
                    VFCMEQQD(q1, v0, v0);    // 0 => out is NAN
                    VBICQ(q1, q0, q1);      // forget it in any input was a NAN already
                    VSHLQ_64(q1, q1, 63);   // only keep the sign bit
                    VORRQ(v0, v0, q1);      // NAN -> -NAN
                }
            }
            if(!vex.l) YMM0(gd)
            break;
        case 0x59:
            INST_NAME("VMULPD Gx, Vx, Ex");
            nextop = F8;
            if(!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn, ninst);
                q1 = fpu_get_scratch(dyn, ninst);
            }
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                if(!BOX64ENV(dynarec_fastnan)) {
                    // check if any input value was NAN
                    VFMAXQD(q0, v2, v1);    // propagate NAN
                    VFCMEQQD(q0, q0, q0);    // 0 if NAN, 1 if not NAN
                }
                VFMULQD(v0, v2, v1);
                if(!BOX64ENV(dynarec_fastnan)) {
                    VFCMEQQD(q1, v0, v0);    // 0 => out is NAN
                    VBICQ(q1, q0, q1);      // forget it in any input was a NAN already
                    VSHLQ_64(q1, q1, 63);   // only keep the sign bit
                    VORRQ(v0, v0, q1);      // NAN -> -NAN
                }
            }
            if(!vex.l) YMM0(gd)
            break;
        case 0x5A:
            INST_NAME("VCVTPD2PS Gx, Ex");
            nextop = F8;
            GETEX_Y(v1, 0, 0);
            GETGX_empty(v0);
            if(BOX64ENV(dynarec_fastround)==2) {
                FCVTXN(v0, v1);
            } else {
                u8 = sse_setround(dyn, ninst, x1, x2, x4);
                FCVTN(v0, v1);
                if(!vex.l) x87_restoreround(dyn, ninst, u8);
            }
            if(vex.l) {
                GETEY(v1);
                if(BOX64ENV(dynarec_fastround)==2) {
                    FCVTXN2(v0, v1);
                } else {
                    FCVTN2(v0, v1);
                    x87_restoreround(dyn, ninst, u8);
                }
            }
            YMM0(gd);
            break;
        case 0x5B:
            INST_NAME("VCVTPS2DQ Gx, Ex");
            nextop = F8;
            u8 = sse_setround(dyn, ninst, x1, x2, x6);
            if(!BOX64ENV(dynarec_fastround) && !arm64_frintts) {
                d0 = fpu_get_scratch(dyn, ninst);
                d1 = fpu_get_scratch(dyn, ninst);
                MOVI_32_lsl(d1, 0x80, 3);
            }
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_EX(v0, v1, 0); } else { GETGY_empty_EY(v0, v1); }
                if(BOX64ENV(dynarec_fastround)) {
                    VFRINTISQ(v0, v1);
                    VFCVTZSQS(v0, v0);
                } else {
                    if(arm64_frintts) {
                         VFRINT32XSQ(v0, v1);
                         VFCVTZSQS(v0, v0);
                    } else {
                        if(!l) MRS_fpsr(x5);
                        for(int i=0; i<4; ++i) {
                            BFCx(x5, FPSR_IOC, 1);  // reset IOC bits
                            MSR_fpsr(x5);
                            if(i) {
                                VMOVeS(d0, 0, v1, i);
                                FRINTIS(d0, d0);
                            } else {
                                FRINTIS(d0, v1);
                            }
                            VFCVTZSs(d0, d0);
                            MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                            TSTw_mask(x5, 0, 0);    // mask=(1<<IOC)
                            FCSELS(d0, d0, d1, cEQ);
                            VMOVeS(v0, i, d0, 0);
                        }
                    }
                }
            }
            x87_restoreround(dyn, ninst, u8);
            if(!vex.l) YMM0(gd);
            break;
        case 0x5C:
            INST_NAME("VSUBPD Gx, Vx, Ex");
            nextop = F8;
            if(!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn, ninst);
                q1 = fpu_get_scratch(dyn, ninst);
            }
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                if(!BOX64ENV(dynarec_fastnan)) {
                    // check if any input value was NAN
                    VFMAXQD(q0, v1, v2);    // propagate NAN
                    VFCMEQQD(q0, q0, q0);    // 0 if NAN, 1 if not NAN
                }
                VFSUBQD(v0, v2, v1);
                if(!BOX64ENV(dynarec_fastnan)) {
                    VFCMEQQD(q1, v0, v0);    // 0 => out is NAN
                    VBICQ(q1, q0, q1);      // forget it in any input was a NAN already
                    VSHLQ_64(q1, q1, 63);   // only keep the sign bit
                    VORRQ(v0, v0, q1);      // NAN -> -NAN
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x5D:
            INST_NAME("VMINPD Gx, Vx, Ex");
            nextop = F8;
            if(!BOX64ENV(dynarec_fastnan))
                q0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                // FMIN/FMAX wll not copy a NaN if either is NaN
                // but x86 will copy src2 if either value is NaN, so lets force a copy of Src2 (Ex) if result is NaN
                if(BOX64ENV(dynarec_fastnan)) {
                    VFMINQD(v0, v2, v1);
                } else {
                    VFCMGTQD(q0, v1, v2);   // 0 if NaN or v1 GT v2, so invert mask for copy
                    if(v0!=v1) VBIFQ(v0, v1, q0);
                    if(v0!=v2) VBITQ(v0, v2, q0);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x5E:
            INST_NAME("DIVPD Gx, Ex");
            nextop = F8;
            if(!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn, ninst);
                q1 = fpu_get_scratch(dyn, ninst);
            }
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                if(!BOX64ENV(dynarec_fastnan)) {
                    // check if any input value was NAN
                    VFMAXQD(q0, v2, v1);    // propagate NAN
                    VFCMEQQD(q0, q0, q0);    // 0 if NAN, 1 if not NAN
                }
                VFDIVQD(v0, v2, v1);
                if(!BOX64ENV(dynarec_fastnan)) {
                    VFCMEQQD(q1, v0, v0);    // 0 => out is NAN
                    VBICQ(q1, q0, q1);      // forget it in any input was a NAN already
                    VSHLQ_64(q1, q1, 63);   // only keep the sign bit
                    VORRQ(v0, v0, q1);      // NAN -> -NAN
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x5F:
            INST_NAME("VMAXPD Gx, Vx, Ex");
            nextop = F8;
            if(!BOX64ENV(dynarec_fastnan))
                q0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                // FMIN/FMAX wll not copy a NaN if either is NaN
                // but x86 will copy src2 if either value is NaN, so lets force a copy of Src2 (Ex) if result is NaN
                if(BOX64ENV(dynarec_fastnan)) {
                    VFMAXQD(v0, v2, v1);
                } else {
                    VFCMGTQD(q0, v2, v1);   // 0 if NaN or v2 GT v1, so invert mask for copy
                    if(v0!=v1) VBIFQ(v0, v1, q0);
                    if(v0!=v2) VBITQ(v0, v2, q0);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x60:
            INST_NAME("VPUNPCKLBW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VZIP1Q_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x61:
            INST_NAME("VPUNPCKLWD Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VZIP1Q_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x62:
            INST_NAME("VPUNPCKLDQ Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VZIP1Q_32(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x63:
            INST_NAME("VPACKSSWB Gx,Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                if(v0==v1 && v1!=v2) {
                    if(!l) q0 = fpu_get_scratch(dyn, ninst);
                    SQXTN_8(q0, v1);
                }
                SQXTN_8(v0, v2);
                if(v1==v2) {
                    VMOVeD(v0, 1, v0, 0);
                } else {
                    if(v0==v1)
                        VMOVeD(v0, 1, q0, 0);
                    else
                        SQXTN2_8(v0, v1);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x64:
            INST_NAME("VPCMPGTB Gx,Vx, Ex");
            nextop = F8;
            GETGX_empty_VXEX(v0, v2, v1, 0);
            VCMGTQ_8(v0, v2, v1);
            if(vex.l) {
                GETGY_empty_VYEY(v0, v2, v1);
                VCMGTQ_8(v0, v2, v1);
            } else YMM0(gd);
            break;
        case 0x65:
            INST_NAME("VPCMPGTW Gx, Vx, Ex");
            nextop = F8;
            GETGX_empty_VXEX(v0, v2, v1, 0);
            VCMGTQ_16(v0, v2, v1);
            if(vex.l) {
                GETGY_empty_VYEY(v0, v2, v1);
                VCMGTQ_16(v0, v2, v1);
            } else YMM0(gd);
            break;
        case 0x66:
            INST_NAME("VPCMPGTD Gx, Vx, Ex");
            nextop = F8;
            GETGX_empty_VXEX(v0, v2, v1, 0);
            VCMGTQ_32(v0, v2, v1);
            if(vex.l) {
                GETGY_empty_VYEY(v0, v2, v1);
                VCMGTQ_32(v0, v2, v1);
            } else YMM0(gd);
            break;
        case 0x67:
            INST_NAME("VPACKUSWB Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                if(v0==v1 && v1!=v2) {
                    if(!l) q0 = fpu_get_scratch(dyn, ninst);
                    SQXTUN_8(q0, v1);
                }
                SQXTUN_8(v0, v2);
                if(v1==v2) {
                    VMOVeD(v0, 1, v0, 0);
                } else {
                    if(v0==v1)
                        VMOVeD(v0, 1, q0, 0);
                    else
                        SQXTUN2_8(v0, v1);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x68:
            INST_NAME("VPUNPCKHBW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VZIP2Q_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x69:
            INST_NAME("VPUNPCKHWD Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VZIP2Q_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x6A:
            INST_NAME("VPUNPCKHDQ Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VZIP2Q_32(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x6B:
            INST_NAME("VPACKSSDW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                if(v0==v1) {
                    if(!l) q0 = fpu_get_scratch(dyn, ninst);
                    VMOVQ(q0, v0);
                }
                SQXTN_16(v0, v2);
                if(v2==v1) {
                    VMOVeD(v0, 1, v0, 0);
                } else {
                    SQXTN2_16(v0, (v0==v1)?q0:v1);
                }
            } 
            if(!vex.l) YMM0(gd);
            break;
        case 0x6C:
            INST_NAME("VPUNPCKLQDQ Gx, Vx, Ex");
            nextop = F8;
            if(MODREG) {
                for(int l=0; l<1+vex.l; ++l) {
                    if(!l) {
                        GETGX_empty_VXEX(v0, v2, v1, 0);
                    } else {
                        GETGY_empty_VYEY(v0, v2, v1);
                    }
                    VMOVeD(v0, 1, v1, 0);
                    if(v0!=v2) VMOVeD(v0, 0, v2, 0);
                }
            } else {
                for(int l=0; l<1+vex.l; ++l) {
                    if(!l) {
                        GETGX_empty_VX(v0, v2);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                    } else {
                        GETGY_empty_VY(v0, v2, 0, -1, -1);
                        ADDx_U12(x3, ed, 16);
                        ed = x3;
                    }
                    VLD1_64(v0, 1, ed);
                    if(v0!=v2) VMOVeD(v0, 0, v2, 0);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x6D:
            INST_NAME("VPUNPCKHQDQ Gx, Vx, Ex");
            nextop = F8;
            if(MODREG) {
                for(int l=0; l<1+vex.l; ++l) {
                    if(!l) {
                        GETGX_empty_VXEX(v0, v2, v1, 0);
                    } else {
                        GETGY_empty_VYEY(v0, v2, v1);
                    }
                    VMOVeD(v0, 0, v2, 1);
                    if(v0!=v1) VMOVeD(v0, 1, v1, 1);
                }
            } else {
                for(int l=0; l<1+vex.l; ++l) {
                    if(!l) {
                        GETGX_empty_VX(v0, v2);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                        ADDx_U12(x1, ed, 8);
                    } else {
                        GETGY_empty_VY(v0, v2, -1, -1, -1);
                        ADDx_U12(x1, ed, 16+8);
                    }
                    VMOVeD(v0, 0, v2, 1);
                    VLD1_64(v0, 1, x1);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x6E:
            INST_NAME("VMOVD Gx, Ed");
            nextop = F8;
            GETGX_empty(v0);
            GETED(0);
            if(rex.w) {
                FMOVDx(v0, ed);
            } else {
                VEORQ(v0, v0, v0); // RAZ vector
                VMOVQSfrom(v0, 0, ed);
            }
            YMM0(gd);
            break;
        case 0x6F:
            INST_NAME("VMOVDQA Gx, Ex");
            nextop = F8;
            if(MODREG) {
                GETGX_empty_EX(v0, v1, 0);
                VMOVQ(v0, v1);
                if(vex.l) {
                    GETGY_empty_EY(v0, v1);
                    VMOVQ(v0, v1);
                }
            } else {
                GETGX_empty(v0);
                SMREAD();
                if(vex.l) {
                    GETGY_empty(v1, -1, -1, -1);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0x3f<<4, 15, rex, NULL, 1, 0);
                    VLDP128_I7(v0, v1, ed, fixedaddress);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                    VLD128(v0, ed, fixedaddress);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x70:
            INST_NAME("VPSHUFD Gx, Ex, Ib");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(MODREG) {
                    ed = (nextop&7)+(rex.b<<3);
                    if(!l)
                        v1 = sse_get_reg(dyn, ninst, x1, ed, 0);
                    else
                        v1 = ymm_get_reg(dyn, ninst, x1, ed, 0, gd, -1, -1);

                } else {
                    ed = -1;
                    if(!l) {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                        v1 = fpu_get_scratch(dyn, ninst);      
                    } else {
                        ADDx_U12(x1, wback, 16);
                        wback = x1;
                    }
                }
                if(!l) {
                    u8 = F8;
                    GETGX_empty(v0);
                } else {
                    GETGY_empty(v0, ed, -1, -1);
                }
                switch(u8) {
                    case 0x4E: // Swap Up/Lower 64bits parts
                        if(!MODREG)
                            VLDR128_U12(v1, wback, 0);
                        VEXTQ_8(v0, v1, v1, 8);
                        break;
                    case 0x00: // duplicate lower 32bits to all spot
                        if(MODREG)
                            VDUPQ_32(v0, v1, 0);
                        else
                            VLDQ1R_32(v0, wback);
                        break;
                    case 0x55: // duplicate slot 1 to all spot
                        if(MODREG)
                            VDUPQ_32(v0, v1, 1);
                        else {
                            ADDx_U12(x2, wback, 4);
                            VLDQ1R_32(v0, x2);
                        }
                        break;
                    case 0xAA: // duplicate slot 2 to all spot
                        if(MODREG)
                            VDUPQ_32(v0, v1, 2);
                        else {
                            ADDx_U12(x2, wback, 8);
                            VLDQ1R_32(v0, x2);
                        }
                        break;
                    case 0xFF: // duplicate slot 3 to all spot
                        if(MODREG)
                            VDUPQ_32(v0, v1, 3);
                        else {
                            ADDx_U12(x2, wback, 12);
                            VLDQ1R_32(v0, x2);
                        }
                        break;
                    case 0x44: // duplicate slot 0/1 to all spot
                        if(MODREG)
                            VDUPQ_64(v0, v1, 0);
                        else
                            VLDQ1R_64(v0, wback);
                        break;
                    case 0xEE: // duplicate slot 2/3 to all spot
                        if(MODREG)
                            VDUPQ_64(v0, v1, 1);
                        else {
                            ADDx_U12(x2, wback, 8);
                            VLDQ1R_64(v0, x2);
                        }
                        break;
                    case 0xB1: // invert 0/1 and 2/3
                        if(!MODREG)
                            VLDR128_U12(v1, wback, 0);
                        VREV64Q_32(v0, v1);
                        break;
                    case 0x39: // 0 3 2 1: ror 32 bits
                        if(!MODREG)
                            VLDR128_U12(v1, wback, 0);
                        VEXTQ_8(v0, v1, v1, 4);
                        break;
                    case 0x93: // 2 1 0 3: 0 3 2 1: ror 32 bits then invert low/high 64bits
                        if(!MODREG)
                            VLDR128_U12(v1, wback, 0);
                        VEXTQ_8(v0, v1, v1, 4);
                        VEXTQ_8(v0, v0, v0, 8);
                        break;
                    default:
                        if(!MODREG) {
                            i32 = -1;
                            for (int i=0; i<4; ++i) {
                                int32_t idx = (u8>>(i*2))&3;
                                if(idx!=i32) {
                                    ADDx_U12(x2, wback, idx*4);
                                    i32 = idx;
                                }
                                VLD1_32(v0, i, x2);
                            }
                        } else if(v0!=v1) {
                            VMOVeS(v0, 0, v1, (u8>>(0*2))&3);
                            VMOVeS(v0, 1, v1, (u8>>(1*2))&3);
                            VMOVeS(v0, 2, v1, (u8>>(2*2))&3);
                            VMOVeS(v0, 3, v1, (u8>>(3*2))&3);
                        } else {
                            uint64_t swp[4] = {
                                (0)|(1<<8)|(2<<16)|(3<<24),
                                (4)|(5<<8)|(6<<16)|(7<<24),
                                (8)|(9<<8)|(10<<16)|(11<<24),
                                (12)|(13<<8)|(14<<16)|(15<<24)
                            };
                            if(!l) d0 = fpu_get_scratch(dyn, ninst);
                            tmp64u = swp[(u8>>(0*2))&3] | (swp[(u8>>(1*2))&3]<<32);
                            MOV64x(x2, tmp64u);
                            VMOVQDfrom(d0, 0, x2);
                            tmp64u2 = swp[(u8>>(2*2))&3] | (swp[(u8>>(3*2))&3]<<32);
                            if(tmp64u2==tmp64u) {
                                VMOVQDfrom(d0, 1, x2);
                            } else {
                                MOV64x(x3, tmp64u2);
                                VMOVQDfrom(d0, 1, x3);
                            }
                            VTBLQ1_8(v0, v1, d0);
                        }
                }
            }
            if(!vex.l)  YMM0(gd);
            break;
        case 0x71:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 2:
                    INST_NAME("VPSRLW Vx, Ex, Ib");
                    for(int l=0; l<1+vex.l; ++l) {
                        if(!l) { GETVX_empty_EX(v0, v1, 1); u8 = F8; } else { GETVY_empty_EY(v0, v1); }
                        if(u8) {
                            if (u8>15) {
                                VEORQ(v0, v0, v0);
                            } else if(u8) {
                                VSHRQ_16(v0, v1, u8);
                            }
                        } else if(v0!=v1) VMOVQ(v0, v1);
                    }
                    if(!vex.l) YMM0(vex.v);
                    break;
                case 4:
                    INST_NAME("VPSRAW Vx, Ex, Ib");
                    for(int l=0; l<1+vex.l; ++l) {
                        if(!l) { GETVX_empty_EX(v0, v1, 1); u8 = F8; if(u8>15) u8=15;} else { GETVY_empty_EY(v0, v1); }
                        if(u8) {
                            VSSHRQ_16(v0, v1, u8);
                        } else if(v0!=v1) VMOVQ(v0, v1);
                    }
                    if(!vex.l) YMM0(vex.v);
                    break;
                case 6:
                    INST_NAME("VPSLLW Vx, Ex, Ib");
                    for(int l=0; l<1+vex.l; ++l) {
                        if(!l) { GETVX_empty_EX(v0, v1, 1); u8 = F8; } else { GETVY_empty_EY(v0, v1); }
                        if(u8) {
                            if (u8>15) {
                                VEORQ(v0, v0, v0);
                            } else {
                                VSHLQ_16(v0, v1, u8);
                            }
                        } else if(v0!=v1) VMOVQ(v0, v1);
                    }
                    if(!vex.l) YMM0(vex.v);
                    break;
                default:
                    *ok = 0;
                    DEFAULT;
            }
            break;
        case 0x72:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 2:
                    INST_NAME("VPSRLD Vx, Ex, Ib");
                    for(int l=0; l<1+vex.l; ++l) {
                        if(!l) {
                            GETVX_empty_EX(v0, v1,  1);
                            u8 = F8;
                        } else {
                            GETVY_empty_EY(v0, v1);
                        }
                        if(u8) {
                            if (u8>31) {
                                VEORQ(v0, v0, v0);
                            } else if(u8) {
                                VSHRQ_32(v0, v1, u8);
                            }
                        } else if(v0!=v1)
                            VMOVQ(v0, v1);
                    }
                    if(!vex.l) YMM0(vex.v);
                    break;
                case 4:
                    INST_NAME("VPSRAD Vx, Ex, Ib");
                    for(int l=0; l<1+vex.l; ++l) {
                        if(!l) {
                            GETVX_empty_EX(v0, v1, 1);
                            u8 = F8;
                            if(u8>31) u8=31;
                        } else {
                            GETVY_empty_EY(v0, v1);
                        }
                        if(u8) {
                            VSSHRQ_32(v0, v1, u8);
                        } else if(v0!=v1)
                            VMOVQ(v0, v1);
                    }
                    if(!vex.l) YMM0(vex.v);
                    break;
                case 6:
                    INST_NAME("VPSLLD Vx, Ex, Ib");
                    for(int l=0; l<1+vex.l; ++l) {
                        if(!l) {
                            GETVX_empty_EX(v0, v1, 1);
                            u8 = F8;
                        } else {
                            GETVY_empty_EY(v0, v1);
                        }
                        if(u8) {
                            if (u8>31) {
                                VEORQ(v0, v0, v0);
                            } else {
                                VSHLQ_32(v0, v1, u8);
                            }
                        } else if(v0!=v1)
                            VMOVQ(v0, v1);
                    }
                    if(!vex.l) YMM0(vex.v);
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x73:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 2:
                    INST_NAME("VPSRLQ Vx, Ex, Ib");
                    for(int l=0; l<1+vex.l; ++l) {
                        if(!l) {
                            GETEX_Y(v1, 0, 1);
                            GETVX_empty(v0);
                            u8 = F8;
                        } else {
                            GETVY_empty_EY(v0, v1);
                        }
                        if(u8) {
                            if (u8>63) {
                                VEORQ(v0, v0, v0);
                            } else if(u8) {
                                VSHRQ_64(v0, v1, u8);
                            }
                        } else if(v0!=v1)
                            VMOVQ(v0, v1);
                    }
                    if(!vex.l) YMM0(vex.v);
                    break;
                case 3:
                    INST_NAME("VPSRLDQ Vx, Ex, Ib");
                    q1 = fpu_get_scratch(dyn, ninst);
                    for(int l=0; l<1+vex.l; ++l) {
                        if(!l) {
                            GETEX_Y(v1, 0, 1);
                            GETVX_empty(v0);
                            u8 = F8;
                        } else {
                            GETVY_empty_EY(v0, v1);
                        }
                        if(u8) {
                            if(u8>15) {
                                VEORQ(v0, v0, v0);
                            } else {
                                if(!l) VEORQ(q1, q1, q1);
                                VEXTQ_8(v0, v1, q1, u8);
                            }
                        } else if(v0!=v1)
                            VMOVQ(v0, v1);
                    }
                    if(!vex.l) YMM0(vex.v);
                    break;
                case 6:
                    INST_NAME("VPSLLQ Vx, Ex, Ib");
                    for(int l=0; l<1+vex.l; ++l) {
                        if(!l) {
                            GETEX_Y(v1, 0, 1);
                            GETVX_empty(v0);
                            u8 = F8;
                        } else {
                            GETVY_empty_EY(v0, v1);
                        }
                        if(u8) {
                            if (u8>63) {
                                VEORQ(v0, v0, v0);
                            } else {
                                VSHLQ_64(v0, v1, u8);
                            }
                        } else if(v0!=v1)
                            VMOVQ(v0, v1);
                    }
                    if(!vex.l) YMM0(vex.v);
                    break;
                case 7:
                    INST_NAME("VPSLLDQ Vx, Ex, Ib");
                    q1 = fpu_get_scratch(dyn, ninst);
                    for(int l=0; l<1+vex.l; ++l) {
                        if(!l) {
                            GETEX_Y(v1, 0, 1);
                            GETVX_empty(v0);
                            u8 = F8;
                        } else {
                            GETVY_empty_EY(v0, v1);
                        }
                        if(u8) {
                            if(u8>15) {
                                VEORQ(v0, v0, v0);
                            } else if(u8>0) {
                                if(!l) VEORQ(q1, q1, q1);
                                VEXTQ_8(v0, q1, v1, 16-u8);
                            }
                        } else if(v0!=v1)
                            VMOVQ(v0, v1);
                    }
                    if(!vex.l) YMM0(vex.v);
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0x74:
            INST_NAME("VPCMPEQB Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {GETGX_empty_VXEX(v0, v2, v1, 0);} else {GETGY_empty_VYEY(v0, v2, v1);}
                VCMEQQ_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x75:
            INST_NAME("VPCMPEQW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {GETGX_empty_VXEX(v0, v2, v1, 0);} else {GETGY_empty_VYEY(v0, v2, v1);}
                VCMEQQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x76:
            INST_NAME("VPCMPEQD Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {GETGX_empty_VXEX(v0, v2, v1, 0);} else {GETGY_empty_VYEY(v0, v2, v1);}
                VCMEQQ_32(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0x7C:
            INST_NAME("VHADDPD Gx, Vx, Ex");
            nextop = F8;
            if(!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn, ninst);
                q1 = fpu_get_scratch(dyn, ninst);
            }
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                if(!BOX64ENV(dynarec_fastnan)) {
                    // check if any input value was NAN
                    VFMAXPQD(q0, v2, v1);    // propagate NAN
                    VFCMEQQD(q0, q0, q0);    // 0 if NAN, 1 if not NAN
                }
                VFADDPQD(v0, v2, v1);
                if(!BOX64ENV(dynarec_fastnan)) {
                    VFCMEQQD(q1, v0, v0);    // 0 => out is NAN
                    VBICQ(q1, q0, q1);      // forget it in any input was a NAN already
                    VSHLQ_64(q1, q1, 63);   // only keep the sign bit
                    VORRQ(v0, v0, q1);      // NAN -> -NAN
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x7D:
            INST_NAME("VHSUBPD Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            if(!BOX64ENV(dynarec_fastnan))
                q1 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VUZP1Q_64(q0, v2, v1);
                VUZP2Q_64(v0, v2, v1);
                if(!BOX64ENV(dynarec_fastnan)) {
                    // check if any input value was NAN
                    // but need to mix low/high part
                    VFMAXQD(q1, v0, q0);    // propagate NAN
                    VFCMEQQD(q1, q1, q1);    // 0 if NAN, 1 if not NAN
                }
                VFSUBQD(v0, q0, v0);
                if(!BOX64ENV(dynarec_fastnan)) {
                    VFCMEQQD(q0, v0, v0);    // 0 => out is NAN
                    VBICQ(q1, q1, q0);      // forget it in any input was a NAN already
                    VSHLQ_64(q1, q1, 63);   // only keep the sign bit
                    VORRQ(v0, v0, q1);      // NAN -> -NAN
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x7E:
            INST_NAME("VMOVD Ed,Gx");
            nextop = F8;
            GETGX(v0, 0);
            if(rex.w) {
                if(MODREG) {
                    ed = TO_NAT((nextop & 0x07) + (rex.b << 3));
                    VMOVQDto(ed, v0, 0);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                    VST64(v0, ed, fixedaddress);
                    SMWRITE2();
                }
            } else {
                if(MODREG) {
                    ed = TO_NAT((nextop & 0x07) + (rex.b << 3));
                    VMOVSto(ed, v0, 0);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                    VST32(v0, ed, fixedaddress);
                    SMWRITE2();
                }
            }
            break;
        case 0x7F:
            INST_NAME("VMOVDQA Ex,Gx");
            nextop = F8;
            GETGX(v0, 0);
            if(MODREG) {
                ed = (nextop&7)+(rex.b<<3);
                v1 = sse_get_reg(dyn, ninst, x1, ed, 1);
                VMOVQ(v1, v0);
                if(vex.l) {
                    GETGYEY(v0, v1);
                    VMOVQ(v1, v0);
                } else YMM0(ed);
            } else {
                if(vex.l) {
                    GETGY(v1, 0, -1, -1, -1);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0x3f<<4, 15, rex, NULL, 1, 0);
                    VSTP128_I7(v0, v1, ed, fixedaddress);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                    VST128(v0, ed, fixedaddress);
                }
                SMWRITE2();
            }
            break;

        case 0xC2:
            INST_NAME("VCMPPD Gx, Vx, Ex, Ib");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 1); u8 = F8; } else { GETGY_empty_VYEY(v0, v2, v1); }
                if(((u8&15)==3) || ((u8&15)==7) || ((u8&15)==8) || ((u8&15)==9) || ((u8&15)==10) || ((u8&15)==12)) {
                    VFMAXQD(q0, v2, v1);    // propagate NAN
                    VFCMEQQD(((u8&15)==7)?v0:q0, q0, q0);   // 0 if NAN, 1 if not NAN
                }
                switch(u8&0xf) {
                    // the inversion of the params in the comparison is there to handle NaN the same way SSE does
                    case 0x00: VFCMEQQD(v0, v2, v1); break;   // Equal, not unordered
                    case 0x01: VFCMGTQD(v0, v1, v2); break;   // Less than
                    case 0x02: VFCMGEQD(v0, v1, v2); break;   // Less or equal
                    case 0x03: VMVNQ(v0, q0); break;   // unordered
                    case 0x04: VFCMEQQD(v0, v2, v1); VMVNQ(v0, v0); break;   // Not Equal (or unordered on ARM, not on X86...)
                    case 0x05: VFCMGTQD(v0, v1, v2); VMVNQ(v0, v0); break;   // Greater or equal or unordered
                    case 0x06: VFCMGEQD(v0, v1, v2); VMVNQ(v0, v0); break;   // Greater or unordered
                    case 0x07: break;  // ordered
                    case 0x08: VFCMEQQD(v0, v2, v1); VORNQ(v0, v0, q0); break;   // Equal, or unordered
                    case 0x09: VFCMGTQD(v0, v1, v2); VORNQ(v0, v0, q0); break;   // Less than or unordered
                    case 0x0a: VFCMGEQD(v0, v1, v2); VORNQ(v0, v0, q0); break;   // Less or equal or unordered
                    case 0x0b: VEORQ(v0, v0, v0); break; // false
                    case 0x0c: VFCMEQQD(v0, v2, v1); VBICQ(v0, q0, v0); break;
                    case 0x0d: VFCMGEQD(v0, v2, v1); break;
                    case 0x0e: VFCMGTQD(v0, v2, v1); break;
                    case 0x0f: MOVIQ_64(v0, 0xff); break; //true
                }
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0xC4:
            INST_NAME("VPINSRW Gx, Vx, Ed, Ib");
            nextop = F8;
            GETGX_empty_VX(v0, v2);
            if(v0!=v2) VMOVQ(v0, v2);
            if(MODREG) {
                u8 = (F8)&7;
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                VMOVQHfrom(v0, u8, ed);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                u8 = (F8)&7;
                VLD1_16(v0, u8, wback);
            }
            YMM0(gd);
            break;
        case 0xC5:
            INST_NAME("VPEXTRW Gd, Ex, Ib");
            nextop = F8;
            GETGD;
            if(MODREG) {
                GETEX(v0, 0, 1);    // no vex.l case
                u8 = (F8)&7;
                VMOVHto(gd, v0, u8);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                u8 = (F8)&7;
                LDRH_U12(gd, wback, u8*2);
            }
            break;
        case 0xC6:
            INST_NAME("VSHUFPD Gx, Vx, Ex, Ib");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 1); u8 = F8; } else { GETGY_empty_VYEY(v0, v2, v1); u8>>=2; }
                if((u8&3)==0b01)
                    VEXTQ_8(v0, v2, v1, 8);
                else if(v0==v1 && v0==v2) {
                    switch(u8&3) {
                        case 0b00: VDUPQ_64(v0, v0, 0); break;
                        case 0b01: VEXTQ_8(v0, v2, v1, 8); break;
                        case 0b10: break;
                        case 0b11: VDUPQ_64(v0, v0, 1); break;
                    }
                } else if(v0==v1) {
                    VMOVeD(v0, 1, v1, (u8>>1)&1);
                    VMOVeD(v0, 0, v2, u8&1);
                } else {
                    if(v0!=v2 || (u8&1)) VMOVeD(v0, 0, v2, u8&1);
                    VMOVeD(v0, 1, v1, (u8>>1)&1);
                }
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0xD0:
            INST_NAME("VADDSUBPD Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                FSUBD(q0, v2, v1);
                VFADDQD(v0, v2, v1);
                VMOVeD(v0, 0, q0, 0);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xD1:
            INST_NAME("VPSRLW Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            q1 = fpu_get_scratch(dyn, ninst);
            GETEX(v1, 0, 0);
            MOVI_32(q1, 16);
            UQXTN_32(q0, v1);
            UMIN_32(q0, q0, q1);    // limit to 0 .. +16 values
            NEG_32(q0, q0);         // neg to do shr
            VDUPQ_16(q0, q0, 0);    // only the low 8bits will be used anyway
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VX(v0, v2); } else { GETGY_empty_VY(v0, v2, 0, -1, -1); }
                USHLQ_16(v0, v2, q0);   // SHR x8
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xD2:
            INST_NAME("VPSRLD Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            q1 = fpu_get_scratch(dyn, ninst);
            GETEX(v1, 0, 0);
            MOVI_32(q1, 32);
            UQXTN_32(q0, v1);
            UMIN_32(q0, q0, q1);    // limit to 0 .. +32 values
            NEG_32(q0, q0);         // neg to do shr
            VDUPQ_32(q0, q0, 0);    // only the low 8bits will be used anyway
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VX(v0, v2); } else { GETGY_empty_VY(v0, v2, 0, -1, -1); }
                USHLQ_32(v0, v2, q0);   // SHR x4
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xD3:
            INST_NAME("VPSRLQ Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            q1 = fpu_get_scratch(dyn, ninst);
            GETEX(v1, 0, 0);
            MOVI_32(q1, 64);
            UQXTN_32(q0, v1);
            UMIN_32(q0, q0, q1);    // limit to 0 .. +64 values
            NEG_32(q0, q0);         // neg to do shr
            VDUPQ_32(q0, q0, 0);    // only the low 8bits will be used anyway
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VX(v0, v2); } else { GETGY_empty_VY(v0, v2, 0, -1, -1); }
                USHLQ_64(v0, v2, q0);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xD4:
            INST_NAME("VPADDQ Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VADDQ_64(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xD5:
            INST_NAME("VPMULLW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VMULQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xD6:
            INST_NAME("VMOVQ Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                ed = (nextop&7)+(rex.b<<3);
                v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                VMOV(v1, v0);
                YMM0(ed);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                VST64(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0xD7:
            nextop = F8;
            if(MODREG) {
                INST_NAME("VPMOVMSKB Gd, Ex");
                v0 = fpu_get_scratch(dyn, ninst);
                v1 = fpu_get_scratch(dyn, ninst);
                q1 = fpu_get_scratch(dyn, ninst);
                GETEX_Y(q0, 0, 0);
                GETGD;
                TABLE64(x2, 0x0706050403020100LL);
                VDUPQD(v0, x2);
                VSHRQ_8(q1, q0, 7);
                USHLQ_8(q1, q1, v0); // shift
                UADDLV_8(v1, q1);   // accumalte
                VMOVBto(gd, v1, 0);
                // and now the high part
                VMOVeD(q1, 0, q1, 1);
                UADDLV_8(q1, q1);   // accumalte
                VMOVBto(x2, q1, 0);
                BFIw(gd, x2, 8, 8);
                if(vex.l) {
                    GETEY(q0);
                    VSHRQ_8(q1, q0, 7);
                    USHLQ_8(q1, q1, v0); // shift
                    UADDLV_8(v1, q1);   // accumalte
                    VMOVBto(x2, v1, 0);
                    BFIw(gd, x2, 16, 8);
                    // and now the high part
                    VMOVeD(q1, 0, q1, 1);
                    UADDLV_8(q1, q1);   // accumalte
                    VMOVBto(x2, q1, 0);
                    BFIw(gd, x2, 24, 8);
                }
            } else {
                DEFAULT;
            }
            break;
        case 0xD8:
            INST_NAME("VPSUBUSB Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                UQSUBQ_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xD9:
            INST_NAME("VPSUBUSW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                UQSUBQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xDA:
            INST_NAME("VPMINUB Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                UMINQ_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xDB:
            INST_NAME("VPAND Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                VANDQ(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xDC:
            INST_NAME("VPADDUSB Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                UQADDQ_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xDD:
            INST_NAME("VPADDUSW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                UQADDQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xDE:
            INST_NAME("VPMAXUB Gx,  Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                UMAXQ_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xDF:
            INST_NAME("VPANDN Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                VBICQ(v0, v1, v2);
            }
            if(!vex.l) YMM0(gd);
            break;
         case 0xE0:
            INST_NAME("VPAVGB Gx,  Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                URHADDQ_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xE1:
            INST_NAME("VPSRAW Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            q1 = fpu_get_scratch(dyn, ninst);
            GETEX(v1, 0, 0);
            MOVI_32(q1, 15);
            UQXTN_32(q0, v1);
            UMIN_32(q0, q0, q1);    // limit to -15 .. +15 values
            NEG_16(q0, q0);
            VDUPQ_16(q0, q0, 0);    // only the low 8bits will be used anyway
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VX(v0, v2); } else { GETGY_empty_VY(v0, v2, 0, -1, -1); }
                SSHLQ_16(v0, v2, q0);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xE2:
            INST_NAME("VPSRAD Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            q1 = fpu_get_scratch(dyn, ninst);
            GETEX(v1, 0, 0);
            MOVI_32(q1, 31);
            UQXTN_32(q0, v1);
            UMIN_32(q0, q0, q1);        // limit to 0 .. +31 values
            NEG_32(q0, q0);
            VDUPQ_32(q0, q0, 0);    // only the low 8bits will be used anyway
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VX(v0, v2); } else { GETGY_empty_VY(v0, v2, 0, -1, -1); }
                SSHLQ_32(v0, v2, q0);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xE3:
            INST_NAME("VPAVGW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                URHADDQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xE4:
            INST_NAME("VPMULHUW Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            q1 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VUMULL_16(q0, v2, v1);
                VUMULL2_16(q1, v2, v1);
                UQSHRN_16(v0, q0, 16);  // 16bits->16bits: no saturation
                UQSHRN2_16(v0, q1, 16);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xE5:
            INST_NAME("VPMULHW Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            q1 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VSMULL_16(q0, v2, v1);
                VSMULL2_16(q1, v2, v1);
                SQSHRN_16(v0, q0, 16);
                SQSHRN2_16(v0, q1, 16);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xE6:
            INST_NAME("VCVTTPD2DQ Gx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETEX_Y(v1, 0, 0);
                    GETGX_empty(v0);
                } else {
                    if(BOX64ENV(dynarec_fastround))
                        d0 = fpu_get_scratch(dyn, ninst);
                    GETEY(v1);
                }
                if(BOX64ENV(dynarec_fastround)) {
                    VFCVTZSQD(l?d0:v0, v1); // convert double -> int64
                    if(!l)
                        SQXTN_32(v0, v0);   // convert int64 -> int32 with saturation in lower part, RaZ high part
                    else
                        SQXTN2_32(v0, d0);   // convert int64 -> int32 with saturation in higher part
                } else {
                    if(arm64_frintts) {
                        VFRINT32ZDQ(l?d0:v0, v1); // handle overflow
                        VFCVTZSQD(l?d0:v0, l?d0:v0);  // convert double -> int64
                        if(!l)
                            SQXTN_32(v0, v0);   // convert int64 -> int32 with saturation in lower part, RaZ high part
                        else
                            SQXTN2_32(v0, d0);   // convert int64 -> int32 with saturation in higher part
                    } else {
                        if(!l) {
                            MRS_fpsr(x5);
                            ORRw_mask(x4, xZR, 1, 0);    //0x80000000
                            d0 = fpu_get_scratch(dyn, ninst);
                        }
                        for(int i=0; i<2; ++i) {
                            BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                            MSR_fpsr(x5);
                            if(i) {
                                VMOVeD(d0, 0, v1, i);
                                FCVTZSwD(x1, d0);
                            } else {
                                FCVTZSwD(x1, v1);
                            }
                            MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                            TSTw_mask(x5, 0, 0);    // mask = 1 = FPSR_IOC
                            CSELx(x1, x1, x4, cEQ);
                            VMOVQSfrom(v0, i+l*2, x1);
                        }
                        if(!vex.l && !l) VMOVQDfrom(v0, 1, xZR);
                    }
                }
            }
            YMM0(gd);
            break;
        case 0xE7:
            INST_NAME("VMOVNTDQ Ex,Gx");
            nextop = F8;
            GETG;
            if(MODREG) {
                DEFAULT;
            } else {
                v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                if(vex.l) {
                    GETGY(v1, 0, -1, -1, -1);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0x3f<<4, 15, rex, NULL, 1, 0);
                    VSTP128_I7(v0, v1, ed, fixedaddress);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                    VST128(v0, ed, fixedaddress);
                }
            }
            break;
        case 0xE8:
            INST_NAME("VPSUBSB Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                SQSUBQ_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xE9:
            INST_NAME("VPSUBSW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                SQSUBQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xEA:
            INST_NAME("VPMINSW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                SMINQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xEB:
            INST_NAME("VPOR Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                VORRQ(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xEC:
            INST_NAME("VPADDSB Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                SQADDQ_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xED:
            INST_NAME("VPADDSW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                SQADDQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xEE:
            INST_NAME("VPMAXSW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                SMAXQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xEF:
            INST_NAME("VPXOR Gx, Vx, Ex");
            nextop = F8;
            GETG;
            if(MODREG && ((nextop&7)+(rex.b<<3)==vex.v)) {
                // special case for PXOR Gx, Gx
                q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VEORQ(q0, q0, q0);
                if(vex.l) {
                    q0 = ymm_get_reg_empty(dyn, ninst, x1, gd, -1, -1,-1);
                    VEORQ(q0, q0, q0);
                }
            } else {
                GETGX_empty_VXEX(v0, v2, v1, 0);
                VEORQ(v0, v1, v2);
                if(vex.l) {
                    GETGY_empty_VYEY(v0, v2,v1);
                    VEORQ(v0, v1, v2);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xF1:
            INST_NAME("VPSLLW Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            q1 = fpu_get_scratch(dyn, ninst);
            GETEX(v1, 0, 0);
            UQXTN_32(q0, v1);
            MOVI_32(q1, 16);
            UMIN_32(q0, q0, q1);    // limit to 0 .. +16 values
            VDUPQ_16(q0, q0, 0);    // only the low 8bits will be used anyway
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VX(v0, v2); } else { GETGY_empty_VY(v0, v2, 0, -1, -1); }
                USHLQ_16(v0, v2, q0);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xF2:
            INST_NAME("VPSLLD Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            q1 = fpu_get_scratch(dyn, ninst);
            GETEX(v1, 0, 0);
            UQXTN_32(q0, v1);
            MOVI_32(q1, 32);
            UMIN_32(q0, q0, q1);    // limit to 0 .. +32 values
            VDUPQ_32(q0, q0, 0);    // only the low 8bits will be used anyway
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VX(v0, v2); } else { GETGY_empty_VY(v0, v2, 0, -1, -1); }
                USHLQ_32(v0, v2, q0);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xF3:
            INST_NAME("VPSLLQ Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            q1 = fpu_get_scratch(dyn, ninst);
            GETEX(v1, 0, 0);
            UQXTN_32(q0, v1);
            MOVI_32(q1, 64);
            UMIN_32(q0, q0, q1);    // limit to 0 .. +64 values
            VDUPQ_64(q0, q0, 0);    // only the low 8bits will be used anyway
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VX(v0, v2); } else { GETGY_empty_VY(v0, v2, 0, -1, -1); }
                USHLQ_64(v0, v2, q0);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xF4:
            INST_NAME("VPMULUDQ Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            if(MODREG)
                q1 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VUZP1Q_32(q0, v2, v2);  //A3 A2 A1 A0 -> A3 A1 A2 A0
                if(!MODREG) {
                    q1 = v1;
                }
                VUZP1Q_32(q1, v1, v1);
                VUMULL_32(v0, q0, q1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xF5:
            INST_NAME("VPMADDWD Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            q1 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VSMULL_16(q0, v2, v1);
                VSMULL2_16(q1, v2, v1);
                VADDPQ_32(v0, q0, q1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xF6:
            INST_NAME("VPSADBW Gx, Vx, Ex");
            nextop = F8;
            d0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                UABDL_8(d0, v2, v1);
                UADDLVQ_16(d0, d0);
                VMOVeD(v0, 0, d0, 0);
                UABDL2_8(d0, v2, v1);
                UADDLVQ_16(d0, d0);
                VMOVeD(v0, 1, d0, 0);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xF7:
            INST_NAME("VMASKMOVDQU Gx, Ex");
            nextop = F8;
            GETGX(q0, 0);
            GETEX(q1, 0, 0);    //no vex.l case
            v0 = fpu_get_scratch(dyn, ninst);
            VLDR128_U12(v0, xRDI, 0);
            if(MODREG)
                v1 = fpu_get_scratch(dyn, ninst); // need to preserve the register
            else
                v1 = q1;
            VSSHRQ_8(v1, q1, 7);  // get the mask
            VBITQ(v0, q0, v1);
            VSTR128_U12(v0, xRDI, 0);  // put back
            break;
        case 0xF8:
            INST_NAME("VPSUBB Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VSUBQ_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xF9:
            INST_NAME("VPSUBW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VSUBQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xFA:
            INST_NAME("VPSUBD Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VSUBQ_32(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xFB:
            INST_NAME("VPSUBQ Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VSUBQ_64(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xFC:
            INST_NAME("VPADDB Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VADDQ_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xFD:
            INST_NAME("VPADDW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VADDQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xFE:
            INST_NAME("VPADDD Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VADDQ_32(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;

        default:
            DEFAULT;
    }
    return addr;
}
