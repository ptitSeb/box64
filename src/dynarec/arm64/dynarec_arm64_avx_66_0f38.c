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
            GETGXVXEX(v0, v2, v1, 0);
            q0 = fpu_get_scratch(dyn, ninst);
            // create mask
            VSSHRQ_32(q0, v2, 31);
            VBITQ(v1, v0, q0);
            if(!MODREG) {
                VSTR128_U12(v1, ed, fixedaddress);
            }
            if(vex.l) {
                GETGYVYEY(v0, v2, v1);
                VSSHRQ_32(q0, v2, 31);
                VBITQ(v1, v0, q0);
                if(!MODREG)
                    VSTR128_U12(v1, ed, fixedaddress+16);
            }
            break;
        case 0x2F:
            INST_NAME("VMASKMOVPD Ex, Gx, Vx");
            nextop = F8;
            GETGXVXEX(v0, v2, v1, 0);
            q0 = fpu_get_scratch(dyn, ninst);
            // create mask
            VSSHRQ_64(q0, v2, 63);
            VBITQ(v1, v0, q0);
            if(!MODREG) {
                VSTR128_U12(v1, ed, fixedaddress);
            }
            if(vex.l) {
                GETGYVYEY(v0, v2, v1);
                VSSHRQ_64(q0, v2, 63);
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

        case 0x90:
        case 0x92:
            if(opcode==0x90) {INST_NAME("VPGATHERDD Gx, VSIB, Vx");} else {INST_NAME("VGATHERDPD/VGATHERDPS Gx, VSIB, Vx");}
            nextop = F8;
            if(((nextop&7)!=4) || MODREG) {UDF(0);}
            GETG;
            u8 = F8; //SIB
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
            if(wb1) q1 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                    v2 = sse_get_reg(dyn, ninst, x1, vex.v, 1);
                    v1 = sse_get_reg(dyn, ninst, x1, eb2, 0);
                } else {
                    v0 = ymm_get_reg(dyn, ninst, x1, gd, 1, vex.v, eb2, -1);
                    v2 = ymm_get_reg(dyn, ninst, x1, vex.v, 1, gd, eb2, -1);
                    v1 = ymm_get_reg(dyn, ninst, x1, eb2, 0, gd, vex.v, -1);
                }
                // prepare mask
                if(rex.w) VSSHRQ_64(v2, v2, 63); else VSSHRQ_32(v2, v2, 31);    // prescale the values
                if(wb1) VSHLQ_32(q1, v1, wb1); else q1 = v1;
                // slow gather, not much choice here...
                if(rex.w) for(int i=0; i<2; ++i) {
                    VMOVQDto(x4, v2, i);
                    TBZ(x4, 0, 4+4*4);
                    SMOVQSto(x4, q1, i);
                    ADDx_REG(x4, x4, ed);
                    VLD1_64(v0, i, x4);
                    VMOVQDfrom(v2, i, xZR);
                } else for(int i=0; i<4; ++i) {
                    VMOVSto(x4, v2, i);
                    TBZ(x4, 0, 4+4*4);
                    SMOVQSto(x4, q1, i);
                    ADDx_REG(x4, x4, ed);
                    VLD1_32(v0, i, x4);
                    VMOVQSfrom(v2, i, xZR);
                }
            }
            if(!vex.l) {YMM0(gd); YMM0(vex.v);}
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

        default:
            DEFAULT;
    }
    return addr;
}
