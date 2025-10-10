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
                if(v0!=v1) VMOVQ(v0, v1);
                if(vex.l) {
                    v1 = ymm_get_reg(dyn, ninst, x1, ed, 0, gd, -1, -1);
                    v0 = ymm_get_reg_empty(dyn, ninst, x1, gd, ed, -1, -1);
                    if(v0!=v1) VMOVQ(v0, v1);
                }
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                SMREAD();
                if(vex.l) {
                    v1 = ymm_get_reg_empty(dyn, ninst, x1, gd, -1, -1, -1);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0x3f<<4, 15, rex, NULL, 1, 0);
                    VLDP128_I7(v0, v1, ed, fixedaddress);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                    VLD128(v0, ed, fixedaddress);   // no alignment issue with ARMv8 NEON :)
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
                } else YMM0(ed);
            } else {
                if(vex.l) {
                    v1 = ymm_get_reg(dyn, ninst, x1, gd, 0, ed, -1, -1);
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
            INST_NAME("VMOVLPS Ex, Gx");
            GETGX(v0, 0);
            if(MODREG) {
                DEFAULT;
                return addr;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                VST1_64(v0, 0, ed);
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
        case 0x16:
            nextop = F8;
            if(MODREG) {
                INST_NAME("VMOVLHPS Gx, Vx, Ex");
                GETGX(v0, 1);
                GETVX(v2, 0);
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                if(v0!=v2 && v0!=v1) VMOVQ(v0, v2);
                VMOVeD(v0, 1, v1, 0);
                if(v0!=v2 && v0==v1) VMOVeD(v0, 0, v2, 0);
            } else {
                INST_NAME("VMOVHPS Gx, Vx, Ex");
                SMREAD();
                GETGX(v0, 1);
                GETVX(v2, 0);
                if(v0!=v2) VMOVQ(v0, v2);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                VLD1_64(v0, 1, ed);
            }
            YMM0(gd);
            break;
        case 0x17:
            INST_NAME("VMOVHPS Ex, Gx");
            nextop = F8;
            GETGX(v0, 0);
            if(MODREG) {
                ed = (nextop&7)+(rex.b<<3);
                v1 = sse_get_reg(dyn, ninst, x1, ed, 1);
                VMOVeD(v1, 0, v0, 1);
                YMM0(ed);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                VST1_64(v0, 1, ed);
                SMWRITE2();
            }
            break;

        case 0x28:
            INST_NAME("VMOVAPS Gx, Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                ed = (nextop&7)+(rex.b<<3);
                v1 = sse_get_reg(dyn, ninst, x1, ed, 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VMOVQ(v0, v1);
                if(vex.l) {
                    GETGY_empty_EY(v0, v1);
                    VMOVQ(v0, v1);
                }
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
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
        case 0x29:
            INST_NAME("VMOVAPS Ex, Gx");
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
            INST_NAME("VMOVNTPS Ex, Gx");
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
            }
            break;

        case 0x2E:
            // no special check...
        case 0x2F:
            if(opcode==0x2F) {INST_NAME("VCOMISS Gx, Ex");} else {INST_NAME("VUCOMISS Gx, Ex");}
            SETFLAGS(X_ALL, SF_SET_NODF);
            nextop = F8;
            GETGX(v0, 0);
            GETEXSS(s0, 0, 0);
            IFX(X_CF|X_PF|X_ZF) {
                FCMPS(v0, s0);
            }
            FCOMI(x1, x2);
            break;

        case 0x50:
            INST_NAME("VMOVMSKPS Gd, Ex");
            nextop = F8;
            GETGD;
            MOV32w(gd, 0);
            if(MODREG) {
                // EX is an xmm reg
                q0 = fpu_get_scratch(dyn, ninst);
                for(int l=0; l<1+vex.l; ++l) {
                    if(!l) { GETEX_Y(v0, 0, 0); } else { GETEY(v0); }
                    SQXTN_16(q0, v0);   // reduces the 4 32bits to 4 16bits
                    VMOVQDto(x1, q0, 0);
                    LSRx(x1, x1, 15);
                    BFIx(gd, x1, 0+(l*4), 1);
                    LSRx(x1, x1, 16);
                    BFIx(gd, x1, 1+(l*4), 1);
                    LSRx(x1, x1, 16);
                    BFIx(gd, x1, 2+(l*4), 1);
                    LSRx(x1, x1, 16);
                    BFIx(gd, x1, 3+(l*4), 1);
                }
            } else {
                // EX is memory
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, (0x3f<<3)-16, 7, rex, NULL, 1, 0);
                for(int l=0; l<1+vex.l; ++l) {
                    LDPx_S7_offset(x1, x2, ed, fixedaddress+16*l);
                    LSRx(x1, x1, 31);
                    BFIx(gd, x1, 0+(l*4), 1);
                    LSRx(x1, x1, 32);
                    BFIx(gd, x1, 1+(l*4), 1);
                    LSRx(x1, x2, 31);
                    BFIx(gd, x1, 2+(l*4), 1);
                    LSRx(x1, x1, 32);
                    BFIx(gd, x1, 3+(l*4), 1);
                }
            }
            break;
        case 0x51:
            INST_NAME("VSQRTPS Gx, Ex");
            nextop = F8;
            SKIPTEST(x1);
            if(!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn, ninst);
                d1 = fpu_get_scratch(dyn, ninst);
            }
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_EX(v0, v1, 0); } else { GETGY_empty_EY(v0, v1); }
                if(!BOX64ENV(dynarec_fastnan)) {
                    // check if any input value was NAN
                    VFCMEQQS(d0, v1, v1);    // 0 if NAN, 1 if not NAN
                    VFSQRTQS(v0, v1);
                    VFCMEQQS(d1, v0, v0);    // 0 => out is NAN
                    VBICQ(d1, d0, d1);      // forget it in any input was a NAN already
                    VSHLQ_32(d1, d1, 31);   // only keep the sign bit
                    VORRQ(v0, v0, d1);      // NAN -> -NAN
                } else {
                    VFSQRTQS(v0, v1);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x52:
            INST_NAME("VRSQRTPS Gx, Ex");
            nextop = F8;
            SKIPTEST(x1);
            v0 = fpu_get_scratch(dyn, ninst);
            VFMOVSQ_8(v0, 0b01110000);    //1.0f
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_EX(q0, q1, 0); } else { GETGY_empty_EY(q0, q1); }
                #if 0
                // the aproximation doesn't not work on Death Stranding. code around 0x1419c9100 fail...
                if(!l) {
                    if(q1==q0)
                        v1 = fpu_get_scratch(dyn, ninst);
                    else
                        v1 = q1;
                }
                // more precise
                VFRSQRTEQS(v0, q1);
                VFMULQS(v1, v0, q1);
                VFRSQRTSQS(v1, v1, v0);
                VFMULQS(q0, v1, v0);
                #else
                VFSQRTQS(q0, q1);
                VFDIVQS(q0, v0, q0);
                #endif
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x53:
            INST_NAME("VRCPPS Gx, Ex");
            nextop = F8;
            SKIPTEST(x1);
            q0 = fpu_get_scratch(dyn, ninst);
            VFMOVSQ_8(q0, 0b01110000);    //1.0f
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_EX(v0, v1, 0);
                    #if 0
                    if(v0==v1)
                        q1 = fpu_get_scratch(dyn, ninst);
                    #endif
                } else {
                    GETGY_empty_EY(v0, v1);
                }
                #if 0
                if(v0!=v1) q1 = v0;
                VFRECPEQS(q0, v1);
                VFRECPSQS(q1, q0, v1);
                VFMULQS(v0, q0, q1);
                #else
                VFDIVQS(v0, q0, v1);
                #endif
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x54:
            INST_NAME("VANDPS Gx, Vx, Ex");
            nextop = F8;
            GETGX_empty_VXEX(v0, v2, v1, 0);
            VANDQ(v0, v2, v1);
            if(vex.l) {
                GETGY_empty_VYEY(v0, v2, v1);
                VANDQ(v0, v2, v1);
            } else YMM0(gd)
            break;
        case 0x55:
            INST_NAME("VANDNPS Gx, Vx, Ex");
            nextop = F8;
            GETGX_empty_VXEX(v0, v2, v1, 0);
            VBICQ(v0, v1, v2);
            if(vex.l) {
                GETGY_empty_VYEY(v0, v2, v1);
                VBICQ(v0, v1, v2);
            } else YMM0(gd)
            break;
        case 0x56:
            INST_NAME("VORPS Gx, Vx, Ex");
            nextop = F8;
            GETGX_empty_VXEX(v0, v2, v1, 0);
            VORRQ(v0, v2, v1);
            if(vex.l) {
                GETGY_empty_VYEY(v0, v2, v1);
                VORRQ(v0, v2, v1);
            } else YMM0(gd)
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
            if(!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn, ninst);
                q1 = fpu_get_scratch(dyn, ninst);
            }
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                if(!BOX64ENV(dynarec_fastnan)) {
                    // check if any input value was NAN
                    VFMAXQS(q0, v2, v1);    // propagate NAN
                    VFCMEQQS(q0, q0, q0);    // 0 if NAN, 1 if not NAN
                }
                VFADDQS(v0, v2, v1);
                if(!BOX64ENV(dynarec_fastnan)) {
                    VFCMEQQS(q1, v0, v0);    // 0 => out is NAN
                    VBICQ(q1, q0, q1);      // forget it in any input was a NAN already
                    VSHLQ_32(q1, q1, 31);   // only keep the sign bit
                    VORRQ(v0, v0, q1);      // NAN -> -NAN
                }
            }
            if(!vex.l) YMM0(gd)
            break;
        case 0x59:
            INST_NAME("VMULPS Gx, Vx, Ex");
            nextop = F8;
            if(!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn, ninst);
                q1 = fpu_get_scratch(dyn, ninst);
            }
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                if(!BOX64ENV(dynarec_fastnan)) {
                    // check if any input value was NAN
                    VFMAXQS(q0, v2, v1);    // propagate NAN
                    VFCMEQQS(q0, q0, q0);    // 0 if NAN, 1 if not NAN
                }
                VFMULQS(v0, v2, v1);
                if(!BOX64ENV(dynarec_fastnan)) {
                    VFCMEQQS(q1, v0, v0);    // 0 => out is NAN
                    VBICQ(q1, q0, q1);      // forget it in any input was a NAN already
                    VSHLQ_32(q1, q1, 31);   // only keep the sign bit
                    VORRQ(v0, v0, q1);      // NAN -> -NAN
                }
            }
            if(!vex.l) YMM0(gd)
            break;
        case 0x5A:
            INST_NAME("VCVTPS2PD Gx, Ex");
            nextop = F8;
            if (vex.l) {
                GETGX_empty_EX(v0, v1, 0);
            } else {
                GETEXSD(v1, 0, 0);
                GETGX_empty(v0);
            }
            if(vex.l) {
                GETGY_empty(q0, -1, -1, -1);
                FCVTL2(q0, v1);
            } else YMM0(gd)
            FCVTL(v0, v1);
            break;
        case 0x5B:
            INST_NAME("VCVTDQ2PS Gx, Ex");
            nextop = F8;
            GETGX_empty_EX(v0, v1, 0);
            SCVTQFS(v0, v1);
            if(vex.l) {
                GETGY_empty_EY(v0, v1);
                SCVTQFS(v0, v1);
            } else YMM0(gd)
            break;
        case 0x5C:
            INST_NAME("VSUBPS Gx, Vx, Ex");
            nextop = F8;
            if(!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn, ninst);
                q1 = fpu_get_scratch(dyn, ninst);
            }
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                if(!BOX64ENV(dynarec_fastnan)) {
                    // check if any input value was NAN
                    VFMAXQS(q0, v2, v1);    // propagate NAN
                    VFCMEQQS(q0, q0, q0);    // 0 if NAN, 1 if not NAN
                }
                VFSUBQS(v0, v2, v1);
                if(!BOX64ENV(dynarec_fastnan)) {
                    VFCMEQQS(q1, v0, v0);    // 0 => out is NAN
                    VBICQ(q1, q0, q1);      // forget it in any input was a NAN already
                    VSHLQ_32(q1, q1, 31);   // only keep the sign bit
                    VORRQ(v0, v0, q1);      // NAN -> -NAN
                }
            }
            if(!vex.l) YMM0(gd)
            break;
        case 0x5D:
            INST_NAME("VMINPS Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                // FMIN/FMAX wll not copy a NaN if either is NaN
                // but x86 will copy src2 if either value is NaN, so lets force a copy of Src2 (Ex) if result is NaN
                VFCMGTQS(q0, v1, v2);   // 0 if NaN or v1 GT v2, so invert mask for copy
                if(v0!=v1) VBIFQ(v0, v1, q0);
                if(v0!=v2) VBITQ(v0, v2, q0);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x5E:
            INST_NAME("VDIVPS Gx, Vx, Ex");
            nextop = F8;
            if(!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn, ninst);
                q1 = fpu_get_scratch(dyn, ninst);
            }
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                if(!BOX64ENV(dynarec_fastnan)) {
                    // check if any input value was NAN
                    VFMAXQS(q0, v2, v1);    // propagate NAN
                    VFCMEQQS(q0, q0, q0);    // 0 if NAN, 1 if not NAN
                }
                VFDIVQS(v0, v2, v1);
                if(!BOX64ENV(dynarec_fastnan)) {
                    VFCMEQQS(q1, v0, v0);    // 0 => out is NAN
                    VBICQ(q1, q0, q1);      // forget it in any input was a NAN already
                    VSHLQ_32(q1, q1, 31);   // only keep the sign bit
                    VORRQ(v0, v0, q1);      // NAN -> -NAN
                }
            }
            if(!vex.l) YMM0(gd)
            break;
        case 0x5F:
            INST_NAME("VMAXPS Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                // FMIN/FMAX wll not copy a NaN if either is NaN
                // but x86 will copy src2 if either value is NaN, so lets force a copy of Src2 (Ex) if result is NaN
                VFCMGTQS(q0, v2, v1);   // 0 if NaN or v2 GT v1, so invert mask for copy
                if(v0!=v1) VBIFQ(v0, v1, q0);
                if(v0!=v2) VBITQ(v0, v2, q0);
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0x77:
            if(!vex.l) {
                INST_NAME("VZEROUPPER");
                if(vex.v!=0) {
                    UDF(0);
                } else {
                    for(int i=0; i<(rex.is32bits?8:16); ++i) {
                        YMM0(i);
                    }
                }
            } else {
                INST_NAME("VZEROALL");
                if(vex.v!=0) {
                    UDF(0);
                } else {
                    for(int i=0; i<(rex.is32bits?8:16); ++i) {
                        v0 = sse_get_reg_empty(dyn, ninst, x1, i);
                        VEORQ(v0, v0, v0);
                        YMM0(i);
                    }
                }
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
                        if(BOX64ENV(sse_flushto0)) {
                            MRS_fpcr(x2);                   // get fpscr
                            LSRw_IMM(x3, ed, 15);           // get FZ bit
                            BFIw(x2, x3, 24, 1);            // inject FZ bit
                            EORw_REG_LSR(x3, x3, ed, 1);    // FZ xor DAZ
                            BFIw(x2, x3, 1, 1);             // inject AH bit
                            MSR_fpcr(x2);                   // put new fpscr
                        }
                        if(BOX64ENV(sse_flushto0)) {
                            // try to sync mxcsr with fpsr on the flag side
                            /* mapping is 
                                ARM <-> X86
                                 0  <->  0  // Invalid operation
                                 1  <->  2  // Divide by 0
                                 2  <->  3  // Overflow
                                 3  <->  4  // underflow
                                 4  <->  5  // Inexact
                                 5  <->  1  // denormal
                            */
                            // doing X86 -> ARM here, 543210 => 432150
                            if(ed!=x1)
                                MOVw_REG(x1, ed);   // x1 = 543210
                            UBFXw(x2, x1, 1, 5);   // x2 = 54321
                            BFIw(x1, x2, 2, 4); // x1 = 432110
                            LSRw(x2, x2, 4);    // x2 = 5
                            BFIw(x1, x2, 1, 1); // x1 = 432150
                            MRS_fpsr(x2);
                            BFIx(x2, x1, 0, 6);
                            MSR_fpsr(x2);
                        }
                        break;
                    case 3:
                        INST_NAME("VSTMXCSR Md");
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                        LDRw_U12(x4, xEmu, offsetof(x64emu_t, mxcsr));
                        if(BOX64ENV(sse_flushto0)) {
                            // sync with fpsr, with mask from mxcsr
                            MRS_fpsr(x1);
                            // doing ARM -> X86 here,  0 1 2 3 4 5 -> 0 2 3 4 5 1
                            RORw(x3, x1, 2);    // x3 = 10.....5432
                            BFIw(x1, x3, 1, 4); // x1 = 54320
                            RORw(x3, x3, 32-1); // x3 = 0.....54321
                            BFIw(x1, x3, 5, 1); // x1 = 154320
                            //BFXILw(x3, x4, 7, 6); // this would the mask, but let's ignore that for now
                            BFIw(x4, x1, 0, 6); // inject back the flags
                        }
                        STW(x4, ed, fixedaddress);
                        break;
                    default:
                        DEFAULT;
                }
            break;

        case 0xC2:
            INST_NAME("VCMPPS Gx, Vx, Ex, Ib");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 1); u8 = F8; } else { GETGY_empty_VYEY(v0, v2, v1); }
                if(((u8&15)==3) || ((u8&15)==7) || ((u8&15)==8) || ((u8&15)==9) || ((u8&15)==10) || ((u8&15)==12)) {
                    VFMAXQS(q0, v2, v1);    // propagate NAN
                    VFCMEQQS(((u8&15)==7)?v0:q0, q0, q0);   // 0 if NAN, 1 if not NAN
                }
                switch(u8&0xf) {
                    // the inversion of the params in the comparison is there to handle NaN the same way SSE does
                    case 0x00: VFCMEQQS(v0, v2, v1); break;   // Equal, not unordered
                    case 0x01: VFCMGTQS(v0, v1, v2); break;   // Less than
                    case 0x02: VFCMGEQS(v0, v1, v2); break;   // Less or equal
                    case 0x03: VMVNQ(v0, q0); break;   // unordered
                    case 0x04: VFCMEQQS(v0, v2, v1); VMVNQ(v0, v0); break;   // Not Equal (or unordered on ARM, not on X86...)
                    case 0x05: VFCMGTQS(v0, v1, v2); VMVNQ(v0, v0); break;   // Greater or equal or unordered
                    case 0x06: VFCMGEQS(v0, v1, v2); VMVNQ(v0, v0); break;   // Greater or unordered
                    case 0x07: break;  // ordered
                    case 0x08: VFCMEQQS(v0, v2, v1); VORNQ(v0, v0, q0); break;   // Equal, or unordered
                    case 0x09: VFCMGTQS(v0, v1, v2); VORNQ(v0, v0, q0); break;   // Less than or unordered
                    case 0x0a: VFCMGEQS(v0, v1, v2); VORNQ(v0, v0, q0); break;   // Less or equal or unordered
                    case 0x0b: VEORQ(v0, v0, v0); break; // false
                    case 0x0c: VFCMEQQS(v0, v2, v1); VBICQ(v0, q0, v0); break;
                    case 0x0d: VFCMGEQS(v0, v2, v1); break;
                    case 0x0e: VFCMGTQS(v0, v2, v1); break;
                    case 0x0f: MOVIQ_64(v0, 0xff); break; //true
                }
            }
            if(!vex.l) YMM0(gd);
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
            } else if(v2==v1 && (u8==0xe0)) {   // elements 3 2 0 0
                VMOVQ(v0, v2);
                VMOVeS(v0, 1, v0, 0);
            } else if(v2==v1 && (u8==0xe5)) {   // elements 3 2 1 1
                VMOVQ(v0, v2);
                VMOVeS(v0, 0, v0, 1);
            } else if(v2==v1 && (u8==0xa0)) {   // elements 2 2 0 0
                VTRNQ1_32(v0, v1, v2);
            } else if(v2==v1 && (u8==0xf5)) {   // elements 3 3 1 1 
                VTRNQ2_32(v0, v1, v2);
            } else if(v2==v1 && (u8==0xb1)) {   // elements 2 3 0 1
                VREV64Q_32(v0, v1);
            } else if(MODREG && u8==0x88) {     // elements 2 0 2 0
                VUZP1Q_32(v0, v2, v1);
            } else if(MODREG && u8==0xdd) {     // elements 3 1 3 1
                VUZP2Q_32(v0, v2, v1);
            } else {
                if((v0==v1) || (v0==v2)) {
                    d0 = fpu_get_scratch(dyn, ninst); s0 = 0;
                } else {
                    d0 = v0; s0 = 1;
                }
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
                if(v0!=d0) VMOVQ(v0, d0);
            }
            if(vex.l) {
                if(MODREG)
                    v1 = ymm_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0, gd, vex.v, -1);
                GETGY_empty_VY(v0, v2, 0, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1);
                if(v2==v1 && (u8&0x3)==((u8>>2)&3) && (u8&0xf)==((u8>>4)&0xf)) {
                    VDUPQ_32(v0, v2, u8&3);
                } else if(v2==v1 && (u8==0xe0)) {   // elements 3 2 0 0
                    VMOVQ(v0, v2);
                    VMOVeS(v0, 1, v0, 0);
                } else if(v2==v1 && (u8==0xe5)) {   // elements 3 2 1 1
                    VMOVQ(v0, v2);
                    VMOVeS(v0, 0, v0, 1);
                } else if(v2==v1 && (u8==0xa0)) {   // elements 2 2 0 0
                    VTRNQ1_32(v0, v1, v2);
                } else if(v2==v1 && (u8==0xf5)) {   // elements 3 3 1 1 
                    VTRNQ2_32(v0, v1, v2);
                } else if(v2==v1 && (u8==0xb1)) {   // elements 2 3 0 1
                    VREV64Q_32(v0, v1);
                } else if(MODREG && u8==0x88) {     // elements 2 0 2 0
                    VUZP1Q_32(v0, v2, v1);
                } else if(MODREG && u8==0xdd) {     // elements 3 1 3 1
                    VUZP2Q_32(v0, v2, v1);
                } else {
                    if(s0) d0 = v0;
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
