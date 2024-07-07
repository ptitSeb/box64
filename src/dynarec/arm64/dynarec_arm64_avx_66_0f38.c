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

uintptr_t dynarec64_AVX_66_0F38(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
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
        case 0x00:
            INST_NAME("VPSHUFB Gx, Vx, Ex");
            nextop = F8;
            d0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                MOVIQ_8(d0, 0b10001111);
                VANDQ(d0, d0, v1);  // mask the index
                VTBLQ1_8(v0, v2, d0);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x01:
            INST_NAME("VPHADDW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VADDPQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x02:
            INST_NAME("VPHADDD Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VADDPQ_32(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0x04:
            INST_NAME("PMADDUBSW Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            q1 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                if(v0==v1 || v0==v2) {
                    if(!l) d0 = fpu_get_scratch(dyn, ninst);
                } else
                    d0 = v0;
                UXTL_8(q0, v2);   // this is unsigned, so 0 extended
                SXTL_8(q1, v1);   // this is signed
                VMULQ_16(q0, q0, q1);
                SADDLPQ_16(q1, q0);
                UXTL2_8(q0, v2);   // this is unsigned
                SQXTN_16(d0, q1);   // SQXTN reset the vector so need to grab the high part first
                SXTL2_8(q1, v1);   // this is signed
                VMULQ_16(q0, q0, q1);
                SADDLPQ_16(q0, q0);
                SQXTN2_16(d0, q0);
                if(v0!=d0)
                    VMOVQ(v0, d0);
            }
            if(!vex.l) YMM0(gd); 
            break;

        case 0x08:
            INST_NAME("VPSIGNB Gx, Vx, Ex");
            nextop = F8;
            v1 = fpu_get_scratch(dyn, ninst);
            v0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(q0, q2, q1, 0); } else { GETGY_empty_VYEY(q0, q2, q1); }
                NEGQ_8(v0, q2);     // get NEG
                CMLTQ_0_8(v1, q1);  // calculate mask
                VBIFQ(v0, q2, v1);  // put back positive values
                CMEQQ_0_8(v1, q1);  // handle case where Ex is 0
                VBICQ(q0,v0, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x09:
            INST_NAME("VPSIGNW Gx, Vx, Ex");
            nextop = F8;
            v1 = fpu_get_scratch(dyn, ninst);
            v0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(q0, q2, q1, 0); } else { GETGY_empty_VYEY(q0, q2, q1); }
                NEGQ_16(v0, q2);    // get NEG
                CMLTQ_0_16(v1, q1); // calculate mask
                VBIFQ(v0, q2, v1);  // put back positive values
                CMEQQ_0_16(v1, q1); // handle case where Ex is 0
                VBICQ(q0, v0, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x0A:
            INST_NAME("VPSIGND Gx, Vx, Ex");
            nextop = F8;
            v1 = fpu_get_scratch(dyn, ninst);
            v0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(q0, q2, q1, 0); } else { GETGY_empty_VYEY(q0, q2, q1); }
                NEGQ_32(v0, q2);    // get NEG
                CMLTQ_0_32(v1, q1); // calculate mask
                VBIFQ(v0, q2, v1);  // put back positive values
                CMEQQ_0_32(v1, q1); // handle case where Ex is 0
                VBICQ(q0, v0, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x0B:
            INST_NAME("VPMULHRSW Gx,Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                SQRDMULHQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x0C:
            INST_NAME("VPERMILPS Gx, Vx, Ex");
            nextop = F8;
            q1 = fpu_get_scratch(dyn, ninst);
            q0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                // transform u32 index in V1 to 4 u8 index in q0 for VTBL
                MOVIQ_32(q0, 3); // index and 3
                VANDQ(q0, v1, q0);
                SQXTN_16(q0, q0);   // index in 16bits
                VSHL_16(q0, q0, 1); // double the index
                VZIP1Q_16(q0, q0, q0);   // repeat the index by pair
                MOVIQ_32_lsl(q1, 1, 2);    // q1 as 16bits is 0 / 1
                VADDQ_16(q0, q0, q1);
                SQXTN_8(q0, q0);   // index in 8bits
                VSHL_8(q0, q0, 1); // double the index
                VZIP1Q_8(q0, q0, q0);   // repeat the index by pair
                MOVIQ_16(q1, 1, 1);
                VADDQ_8(q0, q0, q1);
                VTBLQ1_8(v0, v2, q0);
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0x13:
            INST_NAME("VCVTPH2PS Gx, Ex");
            nextop = F8;
            GETEX_Y(v1, 0, 0);
            GETGX_empty(v0);
            if(vex.l && v0==v1) {
                q1 = fpu_get_scratch(dyn, ninst);
                VMOVQ(q1, v1);
                v1 = q1;
            }
            FCVTL16(v0, v1);
            if(vex.l) {
                GETGY_empty(v0, -1, -1, -1);
                FCVTL162(v0, v1);
            } else YMM0(gd);
            break;

        case 0x16:
        case 0x36:
            if(opcode==0x16) { INST_NAME("VPERMPS Gx, Vx, Ex"); } else { INST_NAME("VPERMD Gx, Vx, Ex"); }
            nextop = F8;
            if(!vex.l) UDF(0);
            d0 = fpu_get_double_scratch(dyn, ninst);
            d1 = d0+1;
            q1 = fpu_get_scratch(dyn, ninst);
            q0 = fpu_get_scratch(dyn, ninst);
            s0 = MODREG?((nextop&7)+(rex.b<<3)):-1;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x3, s0, 0);
                VMOVQ(d0, v1);
                v1 = ymm_get_reg(dyn, ninst, x3, s0, 0, gd, vex.v, -1);
                VMOVQ(d1, v1);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0xffe<<4, 15, rex, NULL, 0, 0);
                VLDR128_U12(d0, ed, fixedaddress);
                VLDR128_U12(d1, ed, fixedaddress+16);
            }
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { 
                    GETVX(v2, 0);
                    GETGX_empty(v0);
                } else { 
                    v2 = ymm_get_reg(dyn, ninst, x1, vex.v, 0, gd, s0, -1);
                    GETGY_empty(v0, vex.v, s0, -1); 
                }
                // transform u32 index in V2 to 4 u8 index in q0 for VTBL
                MOVIQ_32(q0, 7); // index and 7
                VANDQ(q0, v2, q0);
                SQXTN_16(q0, q0);   // index in 16bits
                VSHL_16(q0, q0, 1); // double the index
                VZIP1Q_16(q0, q0, q0);   // repeat the index by pair
                MOVIQ_32_lsl(q1, 1, 2);    // q1 as 16bits is 0 / 1
                VADDQ_16(q0, q0, q1);
                SQXTN_8(q0, q0);   // index in 8bits
                VSHL_8(q0, q0, 1); // double the index
                VZIP1Q_8(q0, q0, q0);   // repeat the index by pair
                MOVIQ_16(q1, 1, 1);
                VADDQ_8(q0, q0, q1);
                // fetch the datas
                VTBLQ2_8(v0, d0, q0);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x17:
            INST_NAME("VPTEST GX, EX");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGX(v0, 0);
            if(MODREG) {
                GETEX(v1, 0, 0);
            } else {
                v1 = fpu_get_scratch(dyn, ninst); 
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0xffe<<4, 15, rex, NULL, 0, 0);
            }
            v2 = fpu_get_scratch(dyn, ninst);
            if(vex.l) {
                GETGY(v0, 0, MODREG?((nextop&7)+(rex.b<<3)):-1, -1, -1);
                if(MODREG) {
                    GETEY(v1);
                } else {
                    VLDR128_U12(v1, ed, fixedaddress+16);
                }
            } else {
                if(!MODREG)
                    VLDR128_U12(v1, ed, fixedaddress);
            }
            IFX(X_ZF) {
                VANDQ(v2, v0, v1);
                CMEQQ_0_64(v2, v2);
                UQXTN_32(v2, v2);
                VMOVQDto(x2, v2, 0);
                ADDSx_U12(xZR, x2, 1);
                CSETw(x2, cEQ);
                BFIw(xFlags, x2, F_ZF, 1);
            }
            IFX(X_CF) {
                VBICQ(v2, v1, v0);
                CMEQQ_0_64(v2, v2);
                UQXTN_32(v2, v2);
                VMOVQDto(x2, v2, 0);
                ADDSx_U12(xZR, x2, 1);
                CSETw(x2, cEQ);
                BFIw(xFlags, x2, F_CF, 1);
            }
            IFX(X_AF|X_SF|X_OF|X_PF) {
                MOV32w(x2, (1<<F_AF) | (1<<F_OF) | (1<<F_SF) | (1<<F_PF));
                BICw(xFlags, xFlags, x2);
            }
            break;
        case 0x18:
            INST_NAME("VBROADCASTSS Gx, Ex");
            nextop = F8;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                v1 = fpu_get_scratch(dyn, ninst);
                VLD32(v1, ed, fixedaddress);
            }
            GETGX_empty(v0);
            VDUPQ_32(v0, v1, 0);
            if(vex.l) {
                GETGY_empty(v0, -1, -1, -1);
                VDUPQ_32(v0, v1, 0);
            } else YMM0(gd);
            break;
        case 0x19:
            INST_NAME("VBROADCASTSD Gx, Ex");
            nextop = F8;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                v1 = fpu_get_scratch(dyn, ninst);
                VLD64(v1, ed, fixedaddress);
            }
            GETGX_empty(v0);
            VDUPQ_64(v0, v1, 0);
            if(vex.l) {
                GETGY_empty(v0, -1, -1, -1);
                VDUPQ_64(v0, v1, 0);
            } else YMM0(gd);
            break;
        case 0x1A:
            INST_NAME("VBROADCASTF128 Gx, Ex");
            nextop = F8;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                GETGX_empty(v0);
                VMOVQ(v0, v1);
            } else {
                GETGX_empty(v0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                VLD128(v0, ed, fixedaddress);
            }
            GETGY_empty(v1, -1, -1, -1);
            VMOVQ(v1, v0);
            break;

        case 0x1C:
            INST_NAME("VPABSB Gx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_EX(v0, v1, 0); } else { GETGY_empty_EY(v0, v1); }
                ABSQ_8(v0, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x1D:
            INST_NAME("VPABSW Gx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_EX(v0, v1, 0); } else { GETGY_empty_EY(v0, v1); }
                ABSQ_16(v0, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x1E:
            INST_NAME("VPABSD Gx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_EX(v0, v1, 0); } else { GETGY_empty_EY(v0, v1); }
                ABSQ_32(v0, v1);
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0x20:
            INST_NAME("VPMOVSXBW Gx, Ex");
            nextop = F8;
            if(vex.l) {GETEX(q1, 0, 0);} else {GETEX64(q1, 0, 0);}
            GETGX_empty(q0);
            if(vex.l) {
                GETGY_empty(v0, -1, -1, -1);
                SXTL2_8(v0, q1);
            } else YMM0(gd);
            SXTL_8(q0, q1);
            break;
        case 0x21:
            INST_NAME("VPMOVSXBD Gx, Ex");
            nextop = F8;
            if(vex.l) {GETEX64(q1, 0, 0);} else {GETEX32(q1, 0, 0);}
            GETGX_empty(q0);
            if(vex.l) {
                GETGY_empty(v0, -1, -1, -1);
                SXTL_8(v0, q1);
                SXTL2_16(v0, v0);
            } else YMM0(gd);
            SXTL_8(q0, q1);
            SXTL_16(q0, q0);
            break;
        case 0x22:
            INST_NAME("VPMOVSXBQ Gx, Ex");
            nextop = F8;
            if(vex.l) {GETEX32(q1, 0, 0);} else {GETEX16(q1, 0, 0);}
            GETGX_empty(q0);
            if(vex.l) {
                GETGY_empty(v0, -1, -1, -1);
                SXTL_8(v0, q1);
                SXTL_16(v0, v0);
                SXTL2_32(v0, v0);
            } else YMM0(gd);
            SXTL_8(q0, q1);
            SXTL_16(q0, q0);
            SXTL_32(q0, q0);
            break;
        case 0x23:
            INST_NAME("VPMOVSXWD Gx, Ex");
            nextop = F8;
            if(vex.l) {GETEX(q1, 0, 0);} else {GETEX64(q1, 0, 0);}
            GETGX_empty(q0);
            if(vex.l) {
                GETGY_empty(v0, -1, -1, -1);
                SXTL2_16(v0, q1);
            } else YMM0(gd);
            SXTL_16(q0, q1);
            break;
        case 0x24:
            INST_NAME("VPMOVSXWQ Gx, Ex");
            nextop = F8;
            if(vex.l) {GETEX64(q1, 0, 0);} else {GETEX32(q1, 0, 0);}
            GETGX_empty(q0);
            if(vex.l) {
                GETGY_empty(v0, -1, -1, -1);
                SXTL_16(v0, q1);
                SXTL2_32(v0, v0);
            } else YMM0(gd);
            SXTL_16(q0, q1);
            SXTL_32(q0, q0);
            break;
        case 0x25:
            INST_NAME("VPMOVSXDQ Gx, Ex");
            nextop = F8;
            if(vex.l) {GETEX(q1, 0, 0);} else {GETEX64(q1, 0, 0);}
            GETGX_empty(q0);
            if(vex.l) {
                GETGY_empty(v0, -1, -1, -1);
                SXTL2_32(v0, q1);
            } else YMM0(gd);
            SXTL_32(q0, q1);
            break;

        case 0x28:
            INST_NAME("VPMULDQ Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VUZP1Q_32(q0, v2, v2);   // needs elem 0 and 2 in lower part
                if(v2==v1) {
                    q1 = q0;
                } else {
                    if(MODREG) {
                        if(!l) q1 = fpu_get_scratch(dyn, ninst);
                    } else
                        q1 = v0;
                    VUZP1Q_32(q1, v1, v1);
                }
                VSMULL_32(v0, q1, q0);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x29:
            INST_NAME("VPCMPEQQ Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VCMEQQ_64(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0x2B:
            INST_NAME("VPACKUSDW Gx, Ex, Vx");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            VEORQ(q0, q0, q0);
            q1 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                if(v0==v1 && v2!=v1) {
                    if(!l) q2 = fpu_get_scratch(dyn, ninst);
                    VMOVQ(q2, v1);
                    v1 = q2;
                }
                SMAXQ_32(q1, v2, q0);    // values < 0 => 0
                UQXTN_16(v0, q1);
                if(v2==v1) {
                    VMOVeD(v0, 1, v0, 0);
                } else {
                    SMAXQ_32(q1, v1, q0);
                    UQXTN2_16(v0, q1);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x2C:
            INST_NAME("VMASKMOVPS Gx, Vx, Ex");
            nextop = F8;
            GETGX_empty_VXEX(v0, v2, v1, 0);
            q0 = fpu_get_scratch(dyn, ninst);
            // create mask
            VSSHRQ_32(q0, v2, 31);
            VANDQ(v0, v1, q0);
            if(vex.l) {
                GETGY_empty_VYEY(v0, v2, v1);
                VSSHRQ_32(q0, v2, 31);
                VANDQ(v0, v1, q0);
            } else YMM0(gd);
            break;
        case 0x2D:
            INST_NAME("VMASKMOVPD Gx, Vx, Ex");
            nextop = F8;
            GETGX_empty_VXEX(v0, v2, v1, 0);
            q0 = fpu_get_scratch(dyn, ninst);
            // create mask
            VSSHRQ_64(q0, v2, 63);
            VANDQ(v0, v1, q0);
            if(vex.l) {
                GETGY_empty_VYEY(v0, v2, v1);
                VSSHRQ_64(q0, v2, 63);
                VANDQ(v0, v1, q0);
            } else YMM0(gd);
            break;
        case 0x2E:
            INST_NAME("VMASKMOVPS Ex, Gx, Vx");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            GETVX(v2, 0);
            GETGX(v0, 0);
            VSSHRQ_32(q0, v2, 31);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x3, (nextop&7)+(rex.b<<3), 1);
            } else {
                WILLWRITE2();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0xffe<<4, 15, rex, NULL, 0, 0);
                unscaled = 0;
                v1 = fpu_get_scratch(dyn, ninst);
                // check if mask as anything, else scipt the whole read/write to avoid a SEGFAULT.
                // TODO: let a segfault trigger and check if the mask is null instead and ignore the segfault / actually triger: needs to implement SSE reg tracking first!
                SQXTN_32(v1, q0);
                VMOVQDto(x4, v1, 0);
                CBZx(x4, 4+3*4);
                VLDR128_U12(v1, ed, fixedaddress);
            }
            // create mask
            VBITQ(v1, v0, q0);
            if(!MODREG) {
                VSTR128_U12(v1, ed, fixedaddress);
            }
            if(vex.l && !is_avx_zero(dyn, ninst, vex.v)) {
                v2 = ymm_get_reg(dyn, ninst, x1, vex.v, 0, gd, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1);
                v0 = ymm_get_reg(dyn, ninst, x1, gd, 0, vex.v, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1);
                VSSHRQ_32(q0, v2, 31);
                if(MODREG)
                    v1 = ymm_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 1, gd, vex.v, -1);
                else {
                    SQXTN_32(v1, q0);
                    VMOVQDto(x4, v1, 0);
                    CBZx(x4, 4+3*4);
                    VLDR128_U12(v1, ed, fixedaddress+16);
                }
                VBITQ(v1, v0, q0);
                if(!MODREG) {
                    VSTR128_U12(v1, ed, fixedaddress+16);
                }
            }
            break;
        case 0x2F:
            INST_NAME("VMASKMOVPD Ex, Gx, Vx");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            q1 = fpu_get_scratch(dyn, ninst);
            GETVX(v2, 0);
            GETGX(v0, 0);
            VSSHRQ_64(q0, v2, 63);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x3, (nextop&7)+(rex.b<<3), 1);
            } else {
                WILLWRITE2();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0xffe<<4, 15, rex, NULL, 0, 0);
                unscaled = 0;
                v1 = fpu_get_scratch(dyn, ninst);
                // check if mask as anything, else scipt the whole read/write to avoid a SEGFAULT.
                // TODO: let a segfault trigger and check if the mask is null instead and ignore the segfault / actually triger: needs to implement SSE reg tracking first!
                SQXTN_32(q1, q0);
                VMOVQDto(x4, q1, 0);
                CBZx(x4, 4+3*4);
                VLDR128_U12(v1, ed, fixedaddress);
            }
            // create mask
            VBITQ(v1, v0, q0);
            if(!MODREG) {
                VSTR128_U12(v1, ed, fixedaddress);
            }
            if(vex.l && !is_avx_zero(dyn, ninst, vex.v)) {
                v2 = ymm_get_reg(dyn, ninst, x1, vex.v, 0, gd, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1);
                v0 = ymm_get_reg(dyn, ninst, x1, gd, 0, vex.v, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1);
                VSSHRQ_64(q0, v2, 63);
                if(MODREG)
                    v1 = ymm_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 1, gd, vex.v, -1);
                else {
                    SQXTN_32(q1, q0);
                    VMOVQDto(x4, q1, 0);
                    CBZx(x4, 4+3*4);
                    VLDR128_U12(v1, ed, fixedaddress+16);
                }
                VBITQ(v1, v0, q0);
                if(!MODREG) {
                    VSTR128_U12(v1, ed, fixedaddress+16);
                }
            }
            break;
        case 0x30:
            INST_NAME("VPMOVZXBW Gx, Ex");
            nextop = F8;
            GETG;
            if(vex.l) { GETEX(q1, 0, 0); } else { GETEX64(q1, 0, 0); YMM0(gd); }
            GETGX_empty(q0);
            if(vex.l) {
                GETGY_empty(v0, -1, -1, -1);
                UXTL2_8(v0, q1);
            }
            UXTL_8(q0, q1);
            break;
        case 0x31:
            INST_NAME("VPMOVZXBD Gx, Ex");
            nextop = F8;
            GETG;
            if(vex.l) { GETEX64(q1, 0, 0); } else { GETEX32(q1, 0, 0); YMM0(gd); }
            GETGX_empty(q0);
            if(vex.l) {
                GETGY_empty(v0, -1, -1, -1);
                UXTL_8(v0, q1);
                UXTL2_16(v0, v0);
            }
            UXTL_8(q0, q1); 
            UXTL_16(q0, q0);
            break;
        case 0x32:
            INST_NAME("VPMOVZXBQ Gx, Ex");
            nextop = F8;
            GETG;
            if(vex.l) { GETEX32(q1, 0, 0); } else { GETEX16(q1, 0, 0); YMM0(gd); }
            GETGX_empty(q0);
            if(vex.l) {
                GETGY_empty(v0, -1, -1, -1);
                UXTL_8(v0, q1);
                UXTL_16(v0, v0);
                UXTL2_32(v0, v0);
            }
            UXTL_8(q0, q1);
            UXTL_16(q0, q0);
            UXTL_32(q0, q0);
            break;
        case 0x33:
            INST_NAME("VPMOVZXWD Gx, Ex");
            nextop = F8;
            GETG;
            if(vex.l) { GETEX(q1, 0, 0); } else { GETEX64(q1, 0, 0); YMM0(gd); }
            GETGX_empty(q0);
            if(vex.l) {
                GETGY_empty(v0, -1, -1, -1);
                UXTL2_16(v0, q1);
            }
            UXTL_16(q0, q1);
            break;
        case 0x34:
            INST_NAME("VPMOVZXWQ Gx, Ex");
            nextop = F8;
            GETG;
            if(vex.l) { GETEX64(q1, 0, 0); } else { GETEX32(q1, 0, 0); YMM0(gd); }
            GETGX_empty(q0);
            if(vex.l) {
                GETGY_empty(v0, -1, -1, -1);
                UXTL_16(v0, q1);
                UXTL2_32(v0, v0);
            }
            UXTL_16(q0, q1);
            UXTL_32(q0, q0);
            break;
        case 0x35:
            INST_NAME("VPMOVZXDQ Gx, Ex");
            nextop = F8;
            GETG;
            if(vex.l) { GETEX(q1, 0, 0); } else { GETEX64(q1, 0, 0); YMM0(gd); }
            GETGX_empty(q0);
            if(vex.l) {
                GETGY_empty(v0, -1, -1, -1);
                UXTL2_32(v0, q1);
            }
            UXTL_32(q0, q1);
            break;

        case 0x37:
            INST_NAME("VPCMPGTQ Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VCMGTQ_64(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x38:
            INST_NAME("VPMINSB Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                SMINQ_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x39:
            INST_NAME("VPMINSD Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                SMINQ_32(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x3A:
            INST_NAME("VPMINUW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                UMINQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x3B:
            INST_NAME("VPMINUD Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                UMINQ_32(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x3C:
            INST_NAME("VPMAXSB Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                SMAXQ_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x3D:
            INST_NAME("VPMAXSD Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                SMAXQ_32(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x3E:
            INST_NAME("VPMAXUW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                UMAXQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x3F:
            INST_NAME("VPMAXUD Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                UMAXQ_32(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x40:
            INST_NAME("VPMULLD Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VMULQ_32(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x41:
            INST_NAME("PHMINPOSUW Gx, Ex");
            nextop = F8;
            GETEX(v1, 0, 0);
            GETGX_empty(v0);
            q0 = fpu_get_scratch(dyn, ninst);
            q1 = fpu_get_scratch(dyn, ninst);
            // get the min value
            UMINVQ_16(q0, v1);  //q0.uw[0] = min value
            VDUPQ_16(q1, q0, 0);    // vector of min value
            VCMEQQ_16(q1, q1, v1);  // bit field of the element that are the min value
            UQXTN_8(q1, q1);       // same bit field, but on 8bits elements only, easier to handle
            VMOVQDto(x1, q1, 0);    // grab the bit field as a 64bits value
            VEORQ(v0, v0, v0);      // RAZ everything
            RBITx(x1, x1);          // reverse, we want trailling zero but can only count leading ones
            CLZx(x1, x1);
            VMOVeH(v0, 0, q0, 0);   // set up min
            LSRw(x1, x1, 3);        // divide by 8, that's our index...
            VMOVQHfrom(v0, 1, x1);
            YMM0(gd);
            break;

        case 0x45:
            INST_NAME("VPSRLVD/Q Gx, Vx, Ex");
            nextop = F8;
            if(MODREG) q0 = fpu_get_scratch(dyn, ninst);
            q1 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                if(!MODREG) q0 = v1;
                if(rex.w) {
                    VEORQ(q1, q1, q1);
                    MOVI_32(q1, 64);
                    // no 64bits vmin/vmax
                    UQXTN_32(q0, v1);
                    UMIN_32(q0, q0, q1);
                    UXTL_32(q0, q0);
                    NEGQ_64(q0, q0);
                    USHLQ_64(v0, v2, q0); 
                } else {
                    VEORQ(q1, q1, q1);
                    MOVIQ_32(q1, 32);
                    UMINQ_32(q0, v1, q1);
                    NEGQ_32(q0, q0);
                    USHLQ_32(v0, v2, q0);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x46:
            INST_NAME("VPSRAVD/Q Gx, Vx, Ex");
            nextop = F8;
            if(MODREG) q0 = fpu_get_scratch(dyn, ninst);
            q1 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                if(!MODREG) q0 = v1;
                if(rex.w) {
                    VEORQ(q1, q1, q1);
                    MOVI_32(q1, 64);
                    // no 64bits vmin/vmax
                    UQXTN_32(q0, v1);
                    UMIN_32(q0, q0, q1);
                    UXTL_32(q0, q0);
                    NEGQ_64(q0, q0);
                    SSHLQ_64(v0, v2, q0); 
                } else {
                    VEORQ(q1, q1, q1);
                    MOVIQ_32(q1, 32);
                    UMINQ_32(q0, v1, q1);
                    NEGQ_32(q0, q0);
                    SSHLQ_32(v0, v2, q0);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x47:
            INST_NAME("VPSLLVD/Q Gx, Vx, Ex");
            nextop = F8;
            if(MODREG) q0 = fpu_get_scratch(dyn, ninst);
            q1 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                if(!MODREG) q0 = v1;
                if(rex.w) {
                    VEORQ(q1, q1, q1);
                    MOVI_32(q1, 64);
                    // no 64bits vmin/vmax
                    UQXTN_32(q0, v1);
                    UMIN_32(q0, q0, q1);
                    UXTL_32(q0, q0);
                    USHLQ_64(v0, v2, q0); 
                } else {
                    VEORQ(q1, q1, q1);
                    MOVIQ_32(q1, 32);
                    UMINQ_32(q0, v1, q1);
                    USHLQ_32(v0, v2, q0);
                }
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0x58:
            INST_NAME("VPBROADCASTD Gx, Ex");
            nextop = F8;
            if(MODREG) {
                GETGX_empty_EX(v0, v1, 0);
                VDUPQ_32(v0, v1, 0);
            } else {
                GETGX_empty(v0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, 0, 0, 0, rex, NULL, 0, 0);
                VLDQ1R_32(v0, ed);
            }
            if(vex.l) {
                GETGY_empty(q0, -1, -1, -1);
                VMOVQ(q0, v0);
            } else YMM0(gd);
            break;
        case 0x59:
            INST_NAME("VPBROADCASTQ Gx, Ex");
            nextop = F8;
            if(MODREG) {
                GETGX_empty_EX(v0, v1, 0);
                VDUPQ_64(v0, v1, 0);
            } else {
                GETGX_empty(v0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, 0, 0, 0, rex, NULL, 0, 0);
                VLDQ1R_64(v0, ed);
            }
            if(vex.l) {
                GETGY_empty(q0, -1, -1, -1);
                VMOVQ(q0, v0);
            } else YMM0(gd);
            break;
        case 0x5A:
            INST_NAME("VBROADCASTI128 Gx, Ex");
            nextop = F8;
            GETGX_empty_EX(v0, v1, 0);
            if(v0!=v1) VMOVQ(v0, v1);
            GETGY_empty(v0, -1, -1, -1);
            VMOVQ(v0, v1);
            break;

        case 0x78:
            INST_NAME("VBROADCASTB Gx, Ex");
            nextop = F8;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                GETGX_empty(v0);
                VDUPQ_8(v0, v1, 0);
                if(vex.l) {
                    GETGY_empty(v0, -1, -1, -1);
                    VDUPQ_8(v0, v1, 0);
                }
            } else {
                GETGX_empty(v0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                VLDQ1R_8(v0, ed);
                if(vex.l) {
                    GETGY_empty(v0, -1, -1, -1);
                    VLDQ1R_8(v0, ed);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x79:
            INST_NAME("VBROADCASTW Gx, Ex");
            nextop = F8;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                GETGX_empty(v0);
                VDUPQ_16(v0, v1, 0);
                if(vex.l) {
                    GETGY_empty(v0, -1, -1, -1);
                    VDUPQ_16(v0, v1, 0);
                }
            } else {
                GETGX_empty(v0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                VLDQ1R_16(v0, ed);
                if(vex.l) {
                    GETGY_empty(v0, -1, -1, -1);
                    VLDQ1R_16(v0, ed);
                }
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0x8C:
            INST_NAME("VMASKMOVD/Q Vx, Ex, Gx");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                if(rex.w)
                    VSSHRQ_64(q0, v2, 63);
                else
                    VSSHRQ_32(q0, v2, 31);
                VANDQ(v0, v1, q0);
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0x8E:
            INST_NAME("VMASKMOVD/Q Ex, Vx, Gx");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX(v0, 0); GETVX(v2, 0);
                    if(MODREG) {
                        s0 = (nextop&7)+(rex.b<<3);
                        v1 = sse_get_reg_empty(dyn, ninst, x1, s0);
                    } else {
                        s0 = -1;
                        v1 = fpu_get_scratch(dyn, ninst);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0xffe<<4, 15, rex, NULL, 0, 0);
                        VLDR128_U12(v1, ed, fixedaddress);
                    }
                } else {
                    GETGY(v0, 0, vex.v, s0, -1); v2 = ymm_get_reg(dyn, ninst, x1, vex.v, 0, gd, s0, -1);
                    if(MODREG)
                        v1 = ymm_get_reg_empty(dyn, ninst, x1, s0, gd, vex.v, -1);
                    else
                        VLDR128_U12(v1, ed, fixedaddress+16);
                }
                if(rex.w)
                    VSSHRQ_64(q0, v2, 63);
                else
                    VSSHRQ_32(q0, v2, 31);
                VBITQ(v1, v0, q0);
                if(!MODREG)
                    VSTR128_U12(v1, ed, fixedaddress+16*l);
            }
            // no raz of upper ymm
            break;

        case 0x90:
        case 0x92:
            if(opcode==0x90) {INST_NAME("VPGATHERDD Gx, VSIB, Vx");} else {INST_NAME("VGATHERDPD/VGATHERDPS Gx, VSIB, Vx");}
            nextop = F8;
            if(((nextop&7)!=4) || MODREG) {UDF(0);}
            GETG;
            u8 = F8; //SIB
            if((u8&0x7)==0x5 && !(nextop&0xC0)) {
                MOV64x(x5, F32S64);
                eb1 = x5;
            } else
                eb1 = xRAX + (u8&0x7)+(rex.b<<3); // base
            eb2 = ((u8>>3)&7)+(rex.x<<3); // index
            if(nextop&0x40)
                i32 = F8S;
            else if(nextop&0x80)
                i32 = F32S;
            else
                i32 = 0;
            if(!i32) ed = eb1;
            else {
                ed = x3;
                if(i32>0 && i32<4096) ADDx_U12(ed, eb1, i32);
                else if(i32<0 && i32>-4096) SUBx_U12(ed, eb1, -i32);
                else {MOV64x(ed, i32); ADDx_REG(ed, ed, eb1);}
            }
            // ed is base
            wb1 = u8>>6;    // scale
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                    v2 = sse_get_reg(dyn, ninst, x1, vex.v, 1);
                    v1 = sse_get_reg(dyn, ninst, x1, eb2, 0);
                } else {
                    v0 = ymm_get_reg(dyn, ninst, x1, gd, 1, vex.v, (!rex.w)?eb2:-1, -1);
                    v2 = ymm_get_reg(dyn, ninst, x1, vex.v, 1, gd, (!rex.w)?eb2:-1, -1);
                    if(!rex.w) v1 = ymm_get_reg(dyn, ninst, x1, eb2, 0, gd, vex.v, -1);
                }
                // prepare mask
                if(rex.w) VSSHRQ_64(v2, v2, 63); else VSSHRQ_32(v2, v2, 31);
                // slow gather, not much choice here...
                if(rex.w) for(int i=0; i<2; ++i) {
                    VMOVQDto(x4, v2, i);
                    CBZw(x4, 4+4*4);
                    SMOVQSto(x4, v1, i+l*2);
                    ADDx_REG_LSL(x4, ed, x4, wb1);
                    VLD1_64(v0, i, x4);
                    VMOVQDfrom(v2, i, xZR);
                } else for(int i=0; i<4; ++i) {
                    VMOVSto(x4, v2, i);
                    CBZw(x4, 4+4*4);
                    SMOVQSto(x4, v1, i);
                    ADDx_REG_LSL(x4, ed, x4, wb1);
                    VLD1_32(v0, i, x4);
                    VMOVQSfrom(v2, i, xZR);
                }
            }
            if(!vex.l) {YMM0(gd); YMM0(vex.v);}
            break;
        case 0x91:
        case 0x93:
            if(opcode==0x91) {INST_NAME("VPGATHERQD Gx, VSIB, Vx");} else {INST_NAME("VGATHERQPD/VGATHERQPS Gx, VSIB, Vx");}
            nextop = F8;
            if(((nextop&7)!=4) || MODREG) {UDF(0);}
            GETG;
            u8 = F8; //SIB
            if((u8&0x7)==0x5 && !(nextop&0xC0)) {
                MOV64x(x5, F32S64);
                eb1 = x5;
            } else
                eb1 = xRAX + (u8&0x7)+(rex.b<<3); // base
            eb2 = ((u8>>3)&7)+(rex.x<<3); // index
            if(nextop&0x40)
                i32 = F8S;
            else if(nextop&0x80)
                i32 = F32S;
            else
                i32 = 0;
            if(!i32) ed = eb1;
            else {
                ed = x3;
                if(i32>0 && i32<4096) ADDx_U12(ed, eb1, i32);
                else if(i32<0 && i32>-4096) SUBx_U12(ed, eb1, -i32);
                else {MOV64x(ed, i32); ADDx_REG(ed, ed, eb1);}
            }
            // ed is base
            wb1 = u8>>6;    // scale
            if(!rex.w) {
                v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                v2 = sse_get_reg(dyn, ninst, x1, vex.v, 1);
                v1 = sse_get_reg(dyn, ninst, x1, eb2, 0);
                if(vex.l) q1 = ymm_get_reg(dyn, ninst, x1, eb2, 0, gd, vex.v, -1);
                // prepare mask
                VSSHRQ_32(v2, v2, 31);
                // slow gather, not much choice here...
                for(int i=0; i<2+vex.l*2; ++i) {
                    VMOVSto(x4, v2, i);
                    CBZw(x4, 4+4*4);
                    VMOVQDto(x4, (i&2)?q1:v1, i&1);
                    ADDx_REG_LSL(x4, ed, x4, wb1);
                    VLD1_32(v0, i, x4);
                    VMOVQSfrom(v2, i, xZR);
                }
                if(!vex.l) { VMOVQDfrom(v0, 1, xZR); VMOVeD(v2, 1, v0, 1); }
                YMM0(gd); 
                YMM0(vex.v);
            } else {
                for(int l=0; l<1+vex.l; ++l) {
                    if(!l) {
                        v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                        v2 = sse_get_reg(dyn, ninst, x1, vex.v, 1);
                        v1 = sse_get_reg(dyn, ninst, x1, eb2, 0);
                    } else {
                        v0 = ymm_get_reg(dyn, ninst, x1, gd, 1, vex.v, (!rex.w)?eb2:-1, -1);
                        v2 = ymm_get_reg(dyn, ninst, x1, vex.v, 1, gd, (!rex.w)?eb2:-1, -1);
                        v1 = ymm_get_reg(dyn, ninst, x1, eb2, 0, gd, vex.v, -1);
                    }
                    // prepare mask
                    VSSHRQ_64(v2, v2, 63);
                    // slow gather, not much choice here...
                    for(int i=0; i<2; ++i) {
                        VMOVQDto(x4, v2, i);
                        CBZw(x4, 4+4*4);
                        VMOVQDto(x4, v1, i);
                        ADDx_REG_LSL(x4, ed, x4, wb1);
                        VLD1_64(v0, i, x4);
                        VMOVQDfrom(v2, i, xZR);
                    }
                }
                if(!vex.l) { YMM0(gd); YMM0(vex.v); }
            }
            if(!vex.l) YMM0(vex.v);
            break;

        case 0x98:
            INST_NAME("VFMADD132PS/D Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_VXEX(v0, v2, v1, 0); } else { GETGY_VYEY(v0, v2, v1); }
                if(!l && v0!=v2) q0 = fpu_get_scratch(dyn, ninst);
                if(v0!=v2) VMOVQ(q0, v2); else q0 = v2;
                if(rex.w) VFMLAQD(q0, v0, v1); else VFMLAQS(q0, v0, v1);
                VMOVQ(v0, q0);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x99:
            INST_NAME("VFMADD132SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETVX(v2, 0);
            if(rex.w) {GETEXSD(v1, 0, 0);} else {GETEXSS(v1, 0, 0);}
            q0 = fpu_get_scratch(dyn, ninst);
            if(rex.w) {
                FMADD_64(q0, v2, v1, v0);
                VMOVeD(v0, 0, q0, 0);
            } else {
                FMADD_32(q0, v2, v1, v0);
                VMOVeS(v0, 0, q0, 0);
            }
            YMM0(gd);
            break;

        case 0x9B:
            INST_NAME("VFMSUB132SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETVX(v2, 0);
            if(rex.w) {GETEXSD(v1, 0, 0);} else {GETEXSS(v1, 0, 0);}
            q0 = fpu_get_scratch(dyn, ninst);
            if(rex.w) {
                FNMSUB_64(q0, v2, v1, v0);
                VMOVeD(v0, 0, q0, 0);
            } else {
                FNMSUB_32(q0, v2, v1, v0);
                VMOVeS(v0, 0, q0, 0);
            }
            YMM0(gd);
            break;
        case 0x9C:
            INST_NAME("VFNMADD132PS/D Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_VXEX(v0, v2, v1, 0); } else { GETGY_VYEY(v0, v2, v1); }
                if(!l && v0!=v2) q0 = fpu_get_scratch(dyn, ninst);
                if(v0!=v2) VMOVQ(q0, v2); else q0 = v2;
                if(rex.w) VFMLSQD(q0, v0, v1); else VFMLSQS(q0, v0, v1);
                VMOVQ(v0, q0);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x9D:
            INST_NAME("VFNMADD132SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETVX(v2, 0);
            if(rex.w) {GETEXSD(v1, 0, 0);} else {GETEXSS(v1, 0, 0);}
            q0 = fpu_get_scratch(dyn, ninst);
            if(rex.w) {
                FMSUB_64(q0, v2, v1, v0);
                VMOVeD(v0, 0, q0, 0);
            } else {
                FMSUB_32(q0, v2, v1, v0);
                VMOVeS(v0, 0, q0, 0);
            }
            YMM0(gd);
            break;

        case 0x9F:
            INST_NAME("VFNMSUB132SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETVX(v2, 0);
            if(rex.w) {GETEXSD(v1, 0, 0);} else {GETEXSS(v1, 0, 0);}
            q0 = fpu_get_scratch(dyn, ninst);
            if(rex.w) {
                FNMADD_64(q0, v2, v1, v0);
                VMOVeD(v0, 0, q0, 0);
            } else {
                FNMADD_32(q0, v2, v1, v0);
                VMOVeS(v0, 0, q0, 0);
            }
            YMM0(gd);
            break;

        case 0xA8:
            INST_NAME("VFMADD213PS/D Gx, Vx, Ex");
            nextop = F8;
            if(MODREG) q0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_VXEX(v0, v2, v1, 0); } else { GETGY_VYEY(v0, v2, v1); }
                if(MODREG) VMOVQ(q0, v1); else q0 = v1;
                if(rex.w) VFMLAQD(q0, v0, v2); else VFMLAQS(q0, v0, v2);
                VMOVQ(v0, q0);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xA9:
            INST_NAME("VFMADD213SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETVX(v2, 0);
            if(rex.w) {GETEXSD(v1, 0, 0);} else {GETEXSS(v1, 0, 0);}
            q0 = fpu_get_scratch(dyn, ninst);
            if(rex.w) {
                FMADD_64(q0, v1, v0, v2);
                VMOVeD(v0, 0, q0, 0);
            } else {
                FMADD_32(q0, v1, v0, v2);
                VMOVeS(v0, 0, q0, 0);
            }
            YMM0(gd);
            break;
        case 0xAA:
            INST_NAME("VFMSUB213PS/D Gx, Vx, Ex");
            nextop = F8;
            if(MODREG) q0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_VXEX(v0, v2, v1, 0); } else { GETGY_VYEY(v0, v2, v1); }
                if(!MODREG) q0 = v1;
                if(rex.w) {
                    VFNEGQD(q0, v1);
                    VFMLAQD(q0, v0, v2);
                } else {
                    VFNEGQS(q0, v1);
                    VFMLAQS(q0, v0, v2);
                }
                VMOVQ(v0, q0);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xAB:
            INST_NAME("VFMSUB213SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETVX(v2, 0);
            if(rex.w) {GETEXSD(v1, 0, 0);} else {GETEXSS(v1, 0, 0);}
            q0 = fpu_get_scratch(dyn, ninst);
            if(rex.w) {
                FNMSUB_64(q0, v1, v0, v2);
                VMOVeD(v0, 0, q0, 0);
            } else {
                FNMSUB_32(q0, v1, v0, v2);
                VMOVeS(v0, 0, q0, 0);
            }
            YMM0(gd);
            break;
        case 0xAC:
            INST_NAME("VFNMADD213PS/D Gx, Vx, Ex");
            nextop = F8;
            if(MODREG) q0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_VXEX(v0, v2, v1, 0); } else { GETGY_VYEY(v0, v2, v1); }
                if(!MODREG) q0 = v1; else VMOVQ(q0, v1);
                if(rex.w) {
                    VFMLSQD(q0, v0, v2);
                } else {
                    VFMLSQS(q0, v0, v2);
                }
                VMOVQ(v0, q0);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xAD:
            INST_NAME("VFMNADD213SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETVX(v2, 0);
            if(rex.w) {GETEXSD(v1, 0, 0);} else {GETEXSS(v1, 0, 0);}
            q0 = fpu_get_scratch(dyn, ninst);
            if(rex.w) {
                FMSUB_64(q0, v1, v0, v2);
                VMOVeD(v0, 0, q0, 0);
            } else {
                FMSUB_32(q0, v1, v0, v2);
                VMOVeS(v0, 0, q0, 0);
            }
            YMM0(gd);
            break;
        case 0xAE:
            INST_NAME("VFNMSUB213PS/D Gx, Vx, Ex");
            nextop = F8;
            if(MODREG) q0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_VXEX(v0, v2, v1, 0); } else { GETGY_VYEY(v0, v2, v1); }
                if(!MODREG) q0 = v1;
                if(rex.w) {
                    VFNEGQD(q0, v1);
                    VFMLSQD(q0, v0, v2);
                } else {
                    VFNEGQS(q0, v1);
                    VFMLSQS(q0, v0, v2);
                }
                VMOVQ(v0, q0);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xAF:
            INST_NAME("VFNMSUB213SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETVX(v2, 0);
            if(rex.w) {GETEXSD(v1, 0, 0);} else {GETEXSS(v1, 0, 0);}
            q0 = fpu_get_scratch(dyn, ninst);
            if(rex.w) {
                FNMADD_64(q0, v1, v0, v2);
                VMOVeD(v0, 0, q0, 0);
            } else {
                FNMADD_32(q0, v1, v0, v2);
                VMOVeS(v0, 0, q0, 0);
            }
            YMM0(gd);
            break;

        case 0xB6:
            INST_NAME("VFMADDSUB231PS/D Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            static float addsubps[4] = {-1.f, 1.f, -1.f, 1.f};
            static double addsubpd[2] = {-1., 1.};
            MAYUSE(addsubps); MAYUSE(addsubpd);
            TABLE64(x2, (rex.w)?((uintptr_t)&addsubpd):((uintptr_t)&addsubps));
            VLDR128_U12(q0, x2, 0);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_VXEX(v0, v2, v1, 0); if(v0==v2 || v0==v1) q1 = fpu_get_scratch(dyn, ninst); } else { GETGY_VYEY(v0, v2, v1); }
                if(v0!=v1 && v0!=v2) {
                    q1 = v0;
                }
                if(rex.w) {
                    VFMULQD(q1, v0, q0);
                    VFMLAQD(q1, v1, v2);
                } else {
                    VFMULQS(q1, v0, q0);
                    VFMLAQS(q1, v1, v2);
                }
                if(q1!=v0)
                    VMOVQ(v0, q1);
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0xB8:
            INST_NAME("VFMADD231PS/D Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_VXEX(v0, v2, v1, 0); } else { GETGY_VYEY(v0, v2, v1); }
                if(rex.w) VFMLAQD(v0, v1, v2); else VFMLAQS(v0, v1, v2);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xB9:
            INST_NAME("VFMADD231SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETVX(v2, 0);
            if(rex.w) {GETEXSD(v1, 0, 0);} else {GETEXSS(v1, 0, 0);}
            q0 = fpu_get_scratch(dyn, ninst);
            if(rex.w) {
                FMADD_64(q0, v0, v1, v2);
                VMOVeD(v0, 0, q0, 0);
            } else {
                FMADD_32(q0, v0, v1, v2);
                VMOVeS(v0, 0, q0, 0);
            }
            YMM0(gd);
            break;
        case 0xBA:
            INST_NAME("VFNMSUB231PS/D Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_VXEX(v0, v2, v1, 0); } else { GETGY_VYEY(v0, v2, v1); }
                if(rex.w) VFMLSQD(v0, v1, v2); else VFMLSQS(v0, v1, v2);
                if(rex.w) VFNEGQD(v0, v0); else VFNEGQS(v0, v0);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xBB:
            INST_NAME("VFMSUB231SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETVX(v2, 0);
            if(rex.w) {GETEXSD(v1, 0, 0);} else {GETEXSS(v1, 0, 0);}
            q0 = fpu_get_scratch(dyn, ninst);
            if(rex.w) {
                FNMSUB_64(q0, v0, v1, v2);
                VMOVeD(v0, 0, q0, 0);
            } else {
                FNMSUB_32(q0, v0, v1, v2);
                VMOVeS(v0, 0, q0, 0);
            }
            YMM0(gd);
            break;
        case 0xBC:
            INST_NAME("VFNMADD231PS/D Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_VXEX(v0, v2, v1, 0); } else { GETGY_VYEY(v0, v2, v1); }
                if(rex.w) VFMLSQD(v0, v1, v2); else VFMLSQS(v0, v1, v2);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xBD:
            INST_NAME("VFNMADD231SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETVX(v2, 0);
            if(rex.w) {GETEXSD(v1, 0, 0);} else {GETEXSS(v1, 0, 0);}
            q0 = fpu_get_scratch(dyn, ninst);
            if(rex.w) {
                FMSUB_64(q0, v0, v1, v2);
                VMOVeD(v0, 0, q0, 0);
            } else {
                FMSUB_32(q0, v0, v1, v2);
                VMOVeS(v0, 0, q0, 0);
            }
            YMM0(gd);
            break;
        case 0xBE:
            INST_NAME("VFNMSUB231PS/D Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_VXEX(v0, v2, v1, 0); } else { GETGY_VYEY(v0, v2, v1); }
                if(rex.w) { VFMLAQD(v0, v1, v2); VFNEGQD(v0, v0); } else { VFMLAQS(v0, v1, v2); VFNEGQS(v0, v0); }
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0xDC:
            INST_NAME("VAESENC Gx, Vx, Ex");  // AES-NI
            nextop = F8;
            if(arm64_aes) {
                d0 = fpu_get_scratch(dyn, ninst);  // ARM64 internal operation differs a bit from x86_64
                for(int l=0; l<1+vex.l; ++l) {
                    if(!l) {GETGX_empty_VXEX(v0, v2, v1, 0);} else {GETGY_empty_VYEY(v0, v2, v1);}
                    VEORQ(d0, v2, v1);
                    AESE(d0, v1);
                    AESMC(d0, d0);
                    VEORQ(v0, d0, v1);
                }
            } else {
                GETGX_empty_VXEX(q0, q2, q1, 0);
                if(MODREG && (gd==(nextop&7)+(rex.b<<3))) {
                    d0 = fpu_get_scratch(dyn, ninst);
                    if(vex.l)
                        d1 = fpu_get_scratch(dyn, ninst);
                } else d0 = -1;
                if(vex.l) { GETGY_empty_VYEY(v0, v2, v1); }
                if(d0!=-1) {
                    VMOVQ(d0, q1);
                    if(vex.l) VMOVQ(d1, v1);
                }
                if(gd!=vex.v) {
                    VMOVQ(q0, q2);
                    if(vex.l) VMOVQ(v0, v2);
                }
                sse_forget_reg(dyn, ninst, gd);
                MOV32w(x1, gd);
                CALL(native_aese, -1);
                if(vex.l) {MOV32w(x1, gd); CALL(native_aese_y, -1);}
                GETGX(q0, 1);
                VEORQ(q0, q0, (d0==-1)?q1:d0);
                if(vex.l) {
                    GETGY_empty(v0, (d0==-1)?(nextop&7)+(rex.b<<3):-1, -1, -1);
                    VEORQ(v0, v0, (d0==-1)?v1:d0);
                } else YMM0(gd);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xDD:
            INST_NAME("VAESENCLAST Gx, Vx, Ex");  // AES-NI
            nextop = F8;
            if(arm64_aes) {
                d0 = fpu_get_scratch(dyn, ninst);  // ARM64 internal operation differs a bit from x86_64
                for(int l=0; l<1+vex.l; ++l) {
                    if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                    VEORQ(d0, v2, v1);
                    AESE(d0, v1);
                    VEORQ(v0, d0, v1);
                }
            } else {
                GETGX_empty_VXEX(q0, q2, q1, 0);
                if(MODREG && (gd==(nextop&7)+(rex.b<<3))) {
                    d0 = fpu_get_scratch(dyn, ninst);
                    if(vex.l)
                        d1 = fpu_get_scratch(dyn, ninst);
                } else d0 = -1;
                if(vex.l) { GETGY_empty_VYEY(v0, v2, v1); }
                if(d0!=-1) {
                    VMOVQ(d0, q1);
                    if(vex.l) VMOVQ(d1, v1);
                }
                if(gd!=vex.v) {
                    VMOVQ(q0, q2);
                    if(vex.l) VMOVQ(v0, v2);
                }
                sse_forget_reg(dyn, ninst, gd);
                MOV32w(x1, gd);
                CALL(native_aeselast, -1);
                if(vex.l) {MOV32w(x1, gd); CALL(native_aeselast_y, -1);}
                GETGX(q0, 1);
                VEORQ(q0, q0, (d0==-1)?q1:d0);
                if(vex.l) {
                    GETGY_empty(v0, (d0==-1)?(nextop&7)+(rex.b<<3):-1, -1, -1);
                    VEORQ(v0, v0, (d0==-1)?v1:d0);
                } else YMM0(gd);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xDE:
            INST_NAME("VAESDEC Gx, Vx, Ex");  // AES-NI
            nextop = F8;
            if(arm64_aes) {
                d0 = fpu_get_scratch(dyn, ninst);  // ARM64 internal operation differs a bit from x86_64
                for(int l=0; l<1+vex.l; ++l) {
                    if(!l) {GETGX_empty_VXEX(v0, v2, v1, 0);} else {GETGY_empty_VYEY(v0, v2, v1);}
                    VEORQ(d0, v2, v1);
                    AESD(d0, v1);
                    AESIMC(d0, d0);
                    VEORQ(v0, d0, v1);
                }
            } else {
                GETGX_empty_VXEX(q0, q2, q1, 0);
                if(MODREG && (gd==(nextop&7)+(rex.b<<3))) {
                    d0 = fpu_get_scratch(dyn, ninst);
                    if(vex.l)
                        d1 = fpu_get_scratch(dyn, ninst);
                } else d0 = -1;
                if(vex.l) { GETGY_empty_VYEY(v0, v2, v1); }
                if(d0!=-1) {
                    VMOVQ(d0, q1);
                    if(vex.l) VMOVQ(d1, v1);
                }
                if(gd!=vex.v) {
                    VMOVQ(q0, q2);
                    if(vex.l) VMOVQ(v0, v2);
                }
                sse_forget_reg(dyn, ninst, gd);
                MOV32w(x1, gd);
                CALL(native_aesd, -1);
                if(vex.l) {MOV32w(x1, gd); CALL(native_aesd_y, -1);}
                GETGX(q0, 1);
                VEORQ(q0, q0, (d0==-1)?q1:d0);
                if(vex.l) {
                    GETGY_empty(v0, (d0==-1)?(nextop&7)+(rex.b<<3):-1, -1, -1);
                    VEORQ(v0, v0, (d0==-1)?v1:d0);
                } else YMM0(gd);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xDF:
            INST_NAME("VAESDECLAST Gx, Vx, Ex");  // AES-NI
            nextop = F8;
            if(arm64_aes) {
                d0 = fpu_get_scratch(dyn, ninst);  // ARM64 internal operation differs a bit from x86_64
                for(int l=0; l<1+vex.l; ++l) {
                    if(!l) {GETGX_empty_VXEX(v0, v2, v1, 0);} else {GETGY_empty_VYEY(v0, v2, v1);}
                    VEORQ(d0, v2, v1);
                    AESD(d0, v1);
                    VEORQ(v0, d0, v1);
                }
            } else {
                GETGX_empty_VXEX(q0, q2, q1, 0);
                if(MODREG && (gd==(nextop&7)+(rex.b<<3))) {
                    d0 = fpu_get_scratch(dyn, ninst);
                    if(vex.l)
                        d1 = fpu_get_scratch(dyn, ninst);
                } else d0 = -1;
                if(vex.l) { GETGY_empty_VYEY(v0, v2, v1); }
                if(d0!=-1) {
                    VMOVQ(d0, q1);
                    if(vex.l) VMOVQ(d1, v1);
                }
                if(gd!=vex.v) {
                    VMOVQ(q0, q2);
                    if(vex.l) VMOVQ(v0, v2);
                }
                sse_forget_reg(dyn, ninst, gd);
                MOV32w(x1, gd);
                CALL(native_aesdlast, -1);
                if(vex.l) {MOV32w(x1, gd); CALL(native_aesdlast_y, -1);}
                GETGX(q0, 1);
                VEORQ(q0, q0, (d0==-1)?q1:d0);
                if(vex.l) {
                    GETGY_empty(v0, (d0==-1)?(nextop&7)+(rex.b<<3):-1, -1, -1);
                    VEORQ(v0, v0, (d0==-1)?v1:d0);
                } else YMM0(gd);
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0xF7:
            INST_NAME("SHLX Gd, Ed, Vd");
            nextop = F8;
            GETGD;
            GETED(0);
            GETVD;
            ANDx_mask(x3, vd, 1, 0, rex.w?5:4); // mask 0x3f/0x1f
            LSLxw_REG(gd, ed, x3);
            break;

        default:
            DEFAULT;
    }
    return addr;
}
