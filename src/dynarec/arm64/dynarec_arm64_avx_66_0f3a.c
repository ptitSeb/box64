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

uintptr_t dynarec64_AVX_66_0F3A(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
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
    static const int8_t round_round[] = { 0, 2, 1, 3};
    #endif

    rex_t rex = vex.rex;

    switch(opcode) {
        case 0x00:
        case 0x01:
            if(opcode) {INST_NAME("VPERMPD Gx, Ex, Imm8");} else {INST_NAME("VPERMQ Gx, Ex, Imm8");}
            nextop = F8;
            GETGX_empty_EX(v0, q0, 1);
            u8 = F8;
            if(v0==q0) d0 = fpu_get_scratch(dyn, ninst);
            if(v0==q0 || !MODREG) d1 = fpu_get_scratch(dyn, ninst);
            q1 = d1;
            GETGY_empty_EY(v1, q1);
            if(v0==q0) { VMOVQ(d0, q0); VMOVQ(d1, q1); } else {d1 = q1; d0 = q0;}
            VMOVeD(v0, 0, ((u8>>1)&1)?d1:d0, (u8>>0)&1);
            VMOVeD(v0, 1, ((u8>>3)&1)?d1:d0, (u8>>2)&1);
            VMOVeD(v1, 0, ((u8>>5)&1)?d1:d0, (u8>>4)&1);
            VMOVeD(v1, 1, ((u8>>7)&1)?d1:d0, (u8>>6)&1);
            break;
        case 0x02:
            INST_NAME("VPBLENDD Gx, Vx, Ex, u8");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 1); u8 = F8;} else { GETGY_empty_VYEY(v0, v2, v1); u8 >>=4;}
                wb1 = 0; // mask
                for(int i=0; i<4; ++i)
                    if(u8&(1<<i))
                        wb1 |= (3<<(i*2));
                MOVI_64(q0, wb1);   // load 8bits value as a 8bytes mask
                SXTL_16(q0, q0);    // expand 16bits to 32bits...
                if(v0==v1) {
                    VBIFQ(v0, v2, q0);
                } else {
                    if(v0!=v2) VMOVQ(v0, v2);
                    VBITQ(v0, v1, q0);
                }
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0x04:
            INST_NAME("VPERMILPS Gx, Ex, Imm8");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_EX(v0, v1, 1);
                    u8 = F8;
                    if(v0==v1) {q1 = fpu_get_scratch(dyn, ninst); VMOVQ(q1, v1);}
                } else {
                    GETGY_empty_EY(v0, v1);
                    if(v0==v1) {VMOVQ(q1, v1);}
                }
                if(u8==0x00 || u8==0x55 || u8==0xAA || u8==0xFF)
                    VDUPQ_32(v0, (v0==v1)?q1:v1, u8&3);
                else if(u8==0x50)
                    VZIP1Q_32(v0, v1, v1);
                else if(u8==0xFA)
                    VZIP2Q_32(v0, v1, v1);
                else if(u8==0xA0)
                    VTRNQ1_32(v0, v1, v1);
                else if(u8==0xF5)
                    VTRNQ2_32(v0, v1, v1);
                else for(int i=0; i<4; ++i)
                    VMOVeS(v0, i, (v0==v1)?q1:v1, (u8>>(i*2))&3);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x05:
            INST_NAME("VPERMILD Gx, Ex, Imm8");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_EX(v0, v1, 1);
                    u8 = F8;
                } else {
                    GETGY_empty_EY(v0, v1);
                }
                switch(((u8>>(l*2))&3)) {
                    case 0b00:
                    case 0b11:
                        VDUPQ_64(v0, v1, ((u8>>(l*2))&1));
                        break;
                    case 0b10:
                        if(v0!=v1) VMOVQ(v0, v1);
                        break;
                    case 0b01:
                       VEXTQ_8(v0, v1, v1, 8); // invert 64bits values
                       break;
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x06:
        case 0x46:
            if(opcode==0x06) { INST_NAME("VPERM2F128 Gx, Vx, Ex, Imm8"); } else { INST_NAME("VPERM2I128 Gx, Vx, Ex, Imm8"); }
            nextop = F8;
            if(!vex.l) UDF(0);
            if(MODREG) {
                s0 = (nextop&7)+(rex.b<<3);
                v1 = sse_get_reg(dyn, ninst, x1, s0, 0);
            } else {
                s0 = -1;
                v1 = fpu_get_scratch(dyn, ninst);
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0xffe<<4, 15, rex, NULL, 0, 0);
            }
            u8 = F8;
            GETVX(v2, 0);
            GETGX_empty(v0);
            if((v0==v2) && ((u8&0b10110000)==0)) {
                q0 = fpu_get_scratch(dyn, ninst);
                VMOVQ(q0, v2);
                v2 = q0;
            }
            if((v0==v1) && ((u8&0b10110000)==0x20)) {
                q1 = fpu_get_scratch(dyn, ninst);
                VMOVQ(q1, v1);
                v1 = q1;
            }
            switch(u8&0b1011) {
                case 0: if(v0!=v2) VMOVQ(v0, v2); break;
                case 1: d2 = ymm_get_reg(dyn, ninst, x1, vex.v, 0, gd, s0, -1); VMOVQ(v0, d2); break;
                case 2: if(MODREG) { if(v0!=v1) VMOVQ(v0, v1); } else { VLDR128_U12(v0, ed, fixedaddress); } break;
                case 3: if(MODREG) { d1 = ymm_get_reg(dyn, ninst, x1, s0, 0, gd, vex.v, -1); VMOVQ(v0, d1); } else { VLDR128_U12(v0, ed, fixedaddress+16); } break;
                default: VEORQ(v0, v0, v0);
            }
            if((u8&0b10110000)==0x10) { if((u8&0b1011)!=1) d2 = ymm_get_reg(dyn, ninst, x1, vex.v, 0, gd, s0, -1); }
            if(MODREG && ((u8&0b10110000)==0x30)) { if((u8&0b1011)!=3) d1 = ymm_get_reg(dyn, ninst, x1, s0, 0, gd, vex.v, -1); }
            v0 = ymm_get_reg_empty(dyn, ninst, x1, gd, vex.v, s0, -1);
            switch((u8>>4)&0b1011) {
                case 0: VMOVQ(v0, v2); break;
                case 1: if(v0!=d2) VMOVQ(v0, d2); break;
                case 2: if(MODREG) { if(v0!=v1) VMOVQ(v0, v1); } else { VLDR128_U12(v0, ed, fixedaddress); } break;
                case 3: if(MODREG) { if(v0!=d1) VMOVQ(v0, d1); } else { VLDR128_U12(v0, ed, fixedaddress+16); } break;
                default: VEORQ(v0, v0, v0);
            }
            break;

        case 0x08:
            INST_NAME("VROUNDPS Gx, Ex, Ib");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_EX(v0, v1, 1); u8 = F8; } else { GETGY_empty_EY(v0, v1); }
                if(u8&4) {
                    if(!l) wb1 = sse_setround(dyn, ninst, x1, x2, x4);
                    VFRINTISQ(v0, v1);
                    if(l || !vex.l) x87_restoreround(dyn, ninst, wb1);
                } else {
                    VFRINTRSQ(v0, v1, round_round[u8&3]);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x09:
            INST_NAME("VROUNDPD Gx, Ex, Ib");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_EX(v0, v1, 1); u8 = F8; } else { GETGY_empty_EY(v0, v1); }
                if(u8&4) {
                    if(!l) wb1 = sse_setround(dyn, ninst, x1, x2, x4);
                    VFRINTIDQ(v0, v1);
                    if(l || !vex.l) x87_restoreround(dyn, ninst, wb1);
                } else {
                    VFRINTRDQ(v0, v1, round_round[u8&3]);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x0A:
            INST_NAME("VROUNDSS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETEXSS(v1, 0, 1);
            GETGX_empty_VX(v0, v2);
            u8 = F8;
            d1 = fpu_get_scratch(dyn, ninst);
            if(u8&4) {
                u8 = sse_setround(dyn, ninst, x1, x2, x3);
                FRINTXS(d1, v1);
                x87_restoreround(dyn, ninst, u8);
            } else {
                FRINTRRS(d1, v1, round_round[u8&3]);
            }
            if(v0!=v2) VMOVQ(v0, v2);
            VMOVeS(v0, 0, d1, 0);
            YMM0(gd);
            break;
        case 0x0B:
            INST_NAME("VROUNDSD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETEXSD(v1, 0, 1);
            GETGX_empty_VX(v0, v2);
            u8 = F8;
            d1 = fpu_get_scratch(dyn, ninst);
            if(u8&4) {
                u8 = sse_setround(dyn, ninst, x1, x2, x3);
                FRINTXD(d1, v1);
                x87_restoreround(dyn, ninst, u8);
            } else {
                FRINTRRD(d1, v1, round_round[u8&3]);
            }
            if(v0!=v2) VMOVQ(v0, v2);
            VMOVeD(v0, 0, d1, 0);
            YMM0(gd);
            break;
        case 0x0C:
            INST_NAME("VBLENDPS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGX_empty_VXEX(q0, q2, q1, 1);
            u8 = F8;
            if((u8&0xf)==0xf) {
                if(q0!=q1) VMOVQ(q0, q1);
            } else if(q0==q1) {
                for(int i=0; i<4; ++i)
                    if(u8&(1<<i)) {
                        //VMOVeS(q0, i, q1, i);
                    } else if(q0!=q2)
                        VMOVeS(q0, i, q2, i);
            } else {
                if(q0!=q2)
                    VMOVQ(q0, q2);
                if((u8&0b0011)==0b0011) {
                    VMOVeD(q0, 0, q1, 0);
                    u8&=~0b0011;
                } 
                if((u8&0b1100)==0b1100) {
                    VMOVeD(q0, 1, q1, 1);
                    u8&=~0b1100;
                } 
                for(int i=0; i<4; ++i)
                    if(u8&(1<<i)) {
                        VMOVeS(q0, i, q1, i);
                    }
            }
            if(vex.l) {
                GETGY_empty_VYEY(q0, q2, q1);
                if((u8&0xf0)==0xf0) {
                    if(q0!=q1) VMOVQ(q0, q1);
                } else if(q0==q1) {
                    for(int i=0; i<4; ++i)
                        if(u8&(1<<(i+4))) {
                            //VMOVeS(q0, i, q1, i);
                        } else if(q0!=q2)
                            VMOVeS(q0, i, q2, i);
                } else {
                    if(q0!=q2)
                        VMOVQ(q0, q2);
                    if(((u8>>4)&0b0011)==0b0011) {
                        VMOVeD(q0, 0, q1, 0);
                        u8&=~0b00110000;
                    } 
                    if(((u8>>4)&0b1100)==0b1100) {
                        VMOVeD(q0, 1, q1, 1);
                        u8&=~0b11000000;
                    }
                    for(int i=0; i<4; ++i)
                        if(u8&(1<<(i+4))) {
                            VMOVeS(q0, i, q1, i);
                        }
                }
            } else YMM0(gd);
            break;
        case 0x0D:
            INST_NAME("VBLENDPD Gx, Vx, Ex, Ib");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(q0, q2, q1, 1); u8 = F8; } else { GETGY_empty_VYEY(q0, q2, q1); }
                switch((u8>>(l*2))&3) {
                    case 0b00: if(q0!=q2) VMOVQ(q0, q2); break;    //  VxVx
                    case 0b01: if(q0!=q1) VMOVeD(q0, 0, q1, 0); if(q0!=q2) VMOVeD(q0, 1, q2, 1); break; // Ex[0]Vx[1]
                    case 0b10: if(q0!=q2) VMOVeD(q0, 0, q2, 0); if(q0!=q1) VMOVeD(q0, 1, q1, 1); break; // Vx[0]Ex[1]
                    case 0b11: if(q0!=q1) VMOVQ(q0, q1); break;    //  ExEx
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x0E:
            INST_NAME("VPBLENDW Gx, Vx, Ex, u8");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 1); u8 = F8;} else { GETGY_empty_VYEY(v0, v2, v1); }
                if(!l) {
                    MOVI_64(q0, u8);
                    SXTL_8(q0, q0);    // expand 8bits to 16bits...
                }
                if(v0!=v2) VBIFQ(v0, v2, q0);
                if(v0!=v1) VBITQ(v0, v1, q0);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x0F:
            INST_NAME("VPALIGNR Gx, Vx, Ex, Ib");
            nextop = F8;
            d0 = fpu_get_scratch(dyn, ninst);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 1);
                    u8 = F8;
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                if(u8>31) {
                    VEORQ(v0, v0, v0);
                } else if(u8>15) {
                    if(!l) VEORQ(d0, d0, d0);
                    VEXTQ_8(v0, v2, d0, u8-16);
                } else {
                    VEXTQ_8(v0, v1, v2, u8);
                }
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0x14:
            INST_NAME("VPEXTRB Ed, Gx, imm8");
            nextop = F8;
            GETGX(v0, 0);
            if(MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff, 0, rex, NULL, 0, 1);
                ed = x1;
            }
            u8 = F8;
            VMOVBto(ed, v0, u8&0x0f);
            if(!MODREG) {
                STB(ed, wback, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x15:
            INST_NAME("VPEXTRW Ed, Gx, imm8");
            nextop = F8;
            GETGX(v0, 0);
            if(MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 1);
                ed = x1;
            }
            u8 = F8;
            VMOVHto(ed, v0, u8&7);
            if(!MODREG) {
                STH(ed, wback, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x16:
            INST_NAME("VPEXTRD/Q Ed, Gx, imm8");
            nextop = F8;
            GETGX(v0, 0);
            if(MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, NULL, 0, 1);
                ed = x1;
            }
            u8 = F8;
            if(rex.w) {
                VMOVQDto(ed, v0, u8&1);
            } else {
                VMOVSto(ed, v0, u8&3);
            }
            if(!MODREG) {
                STxw(ed, wback, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x17:
            INST_NAME("VEXTRACTPS Ed, Gx, imm8");
            nextop = F8;
            GETGX(v0, 0);
            if(MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 1);
                ed = x1;
            }
            u8 = F8;
            VMOVSto(ed, v0, u8&3);
            if(!MODREG) {
                STW(ed, wback, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x18:
        case 0x38:
            if(opcode==0x18) {INST_NAME("VINSERTF128 Gx, Vx, Ex, imm8");} else {INST_NAME("VINSERTI128 Gx, Vx, Ex, imm8");}
            nextop = F8;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 1);
                v1 = -1; // to avoid a warning
            }
            u8 = F8;
            GETVX(v2, 0);
            GETGX_empty(v0);
            if(v0==v2 && u8==1) {
                GETGY_empty(q0, 0, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1);
                q2 = q0;
            } else {
                GETGY_empty_VY(q0, q2, 0, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1);
            }
            if(MODREG)
                VMOVQ((u8&1)?q0:v0, v1);
            else
                VLD128((u8&1)?q0:v0, ed, fixedaddress);
            if(v0!=v2)
                VMOVQ((u8&1)?v0:q0, (u8&1)?v2:q2);
            break;
        case 0x19:
        case 0x39:
            if(opcode==0x19) {INST_NAME("VEXTRACTF128 Ex, Gx, imm8");} else {INST_NAME("VEXTRACTI128 Ex, Gx, imm8");}
            nextop = F8;
            u8 = geted_ib(dyn, addr, ninst, nextop);
            GETGX(v0, 0);
            if(u8&1) {GETGY(v0, ((nextop&7)+(rex.b<<3)==gd && MODREG)?1:0, -1, -1, -1);}
            if(MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop&7)+(rex.b<<3));
                VMOVQ(v1, v0);
                if((nextop&7)+(rex.b<<3)==gd && (u8&1))
                    VEORQ(v0, v0, v0);  // special case like vextractf128 xmm3, ymm3, 0x01
                else
                    YMM0((nextop&7)+(rex.b<<3));
                
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 1);
                VST128(v0, ed, fixedaddress);
                SMWRITE2();
            }
            F8; // read u8, but it's been already handled
            break;

        case 0x1D:
            INST_NAME("VCVTPS2PH Ex, Gx, Ib");
            nextop = F8;
            GETGX(v0, 0);
            if(MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x3, (nextop&7)+(rex.b<<3));
            } else {
                v1 = fpu_get_scratch(dyn, ninst);
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, &unscaled, 0xfff<<(3+vex.l), vex.l?15:7, rex, NULL, 0, 1);
            }
            if(vex.l && v0==v1) {
                q0 = fpu_get_scratch(dyn, ninst);
                VMOVQ(q0, v0);
                v0 = q0;
            }
            u8 = F8;
            if(u8&4) {
                s0 = sse_setround(dyn, ninst, x1, x2, x6);
            } else {
                u8&=3;
                if(u8==1) u8=2;
                else if(u8==2) u8=1;
                MRS_fpcr(x1);               // get fpscr
                MOV32w(x2, u8);
                MOVx_REG(x6, x1);
                BFIx(x1, x2, 22, 2);     // inject new round
                MSR_fpcr(x1);               // put new fpscr
                s0 = x6;
            }
            FCVTN16(v1, v0);
            if(vex.l) {
                GETGY(v0, 0, MODREG?((nextop&7)+(rex.b<<3)):-1, -1,-1);
                FCVTN162(v1, v0);
            }
            x87_restoreround(dyn, ninst, s0);
            if(MODREG) {
                YMM0((nextop&7)+(rex.b<<3));
            } else {
                if(vex.l) {
                    VST128(v1, ed, fixedaddress);
                } else {
                    VST64(v1, ed, fixedaddress);
                }
                SMWRITE2();
            }
            break;

        case 0x20:
            INST_NAME("VINSERTD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGX_empty_VX(v0, v2);
            GETED(1);
            u8 = F8;
            if(v0!=v2) VMOVQ(v0, v2);
            VMOVQBfrom(v0, u8&0xf, ed);
            YMM0(gd);
            break;
        case 0x21:
            INST_NAME("VINSERTPS Gx, Vx, Ex, Ib");
            nextop = F8;
            if (MODREG) {
                GETGX_empty_VXEX(v0, v2, v1, 1);
                u8 = F8;
                if(v0==v1) {
                    d0 = fpu_get_scratch(dyn, ninst);
                    VMOVQ(d0, v1);
                    if(v0!=v2) VMOVQ(v0, v2);
                    VMOVeS(v0, (u8>>4)&3, d0, (u8>>6)&3);
                } else {
                    if(v0!=v2) VMOVQ(v0, v2);
                    VMOVeS(v0, (u8>>4)&3, v1, (u8>>6)&3);
                }
            } else {
                GETGX_empty_VX(v0, v2);
                if(v0!=v2) VMOVQ(v0, v2);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 1);
                u8 = F8;
                LDW(x2, wback, fixedaddress);
                VMOVQSfrom(v0, (u8>>4)&3, x2);
            }
            uint8_t zmask = u8 & 0xf;
            for (uint8_t i=0; i<4; i++) {
                if (zmask & (1<<i)) {
                    VMOVQSfrom(v0, i, wZR);
                }
            }
            YMM0(gd);
            break;
        case 0x22:
            if (rex.w) {
                INST_NAME("VPINSRQ Gx, Vx, ED, Ib");
            } else {
                INST_NAME("VPINSRD Gx, Vx, ED, Ib");
            }
            nextop = F8;
            GETGX_empty_VX(v0, v2);
            GETED(1);
            u8 = F8;
            if(v0!=v2) VMOVQ(v0, v2);
            if(rex.w)
                VMOVQDfrom(v0, u8&1, ed);
            else
                VMOVQSfrom(v0, u8&3, ed);
            YMM0(gd);
            break;

        case 0x40:
            INST_NAME("VDPPS Gx, Vx, Ex, Ib");
            nextop = F8;
            u8 = geted_ib(dyn, addr, ninst, nextop);
            q0 = fpu_get_scratch(dyn, ninst);
            // first mask
            if((u8&0x0f)!=0x0f) {
                wb1 = 0; // mask
                for(int i=0; i<4; ++i)
                    if(u8&(1<<i))
                        wb1 |= (3<<(i*2));
                MOVI_64(q0, wb1);   // load 8bits value as a 8bytes mask
                SXTL_16(q0, q0);    // expand 16bits to 32bits...
            }
            if((u8&0xf0)!=0xf0) {
                q1 = fpu_get_scratch(dyn, ninst);
                // second mask
                wb1 = 0; // mask
                for(int i=0; i<4; ++i)
                    if((u8>>4)&(1<<i))
                        wb1 |= (3<<(i*2));
                MOVI_64(q1, wb1);   // load 8bits value as a 8bytes mask
                SXTL_16(q1, q1);    // expand 16bits to 32bits...
            }
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 1); u8 = F8; } else { GETGY_empty_VYEY(v0, v2, v1); }
                VFMULQS(v0, v2, v1);
                if((u8&0xf0)!=0xf0) {
                    VANDQ(v0, v0, q1);  // second mask
                }
                VFADDPQS(v0, v0, v0);
                FADDPS(v0, v0);
                VDUPQ_32(v0, v0, 0);
                if((u8&0x0f)!=0x0f) {
                    VANDQ(v0, v0, q0);  // first mask
                }
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0x41:
            INST_NAME("VDPPD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGX_empty_VXEX(v0, v1, v2, 0);
            u8 = F8;
            VFMULQD(v0, v1, v2);
            // mask some, duplicate all, mask some
            for(int i=0; i<2; ++i)
                if(!(u8&(1<<(4+i)))) {
                    VMOVQDfrom(v0, i, xZR);
                }
            FADDPD(v0, v0);
            VDUPQ_64(v0, v0, 0);
            for(int i=0; i<2; ++i)
                if(!(u8&(1<<i))) {
                    VMOVQDfrom(v0, i, xZR);
                }
            YMM0(gd);
            break;

        case 0x44:
            INST_NAME("VPCLMULQDQ Gx, Vx, Ex, Ib");
            nextop = F8;
            if(cpuext.pmull) {
                d0 = fpu_get_scratch(dyn, ninst);
                for(int l=0; l<1+vex.l; ++l) {
                    if(!l) {
                        GETGX_empty_VXEX(v0, v2, v1, 1);
                        u8 = F8;
                    } else {
                        GETGY_empty_VYEY(v0, v2, v1);
                    }
                    switch (u8&0b00010001) {
                        case 0b00000000:
                            PMULL_128(v0, v2, v1);
                            break;
                        case 0b00010001:
                            PMULL2_128(v0, v2, v1);
                            break;
                        case 0b00000001:
                            VEXTQ_8(d0, v2, v2, 8); // Swap Up/Lower 64bits parts
                            PMULL_128(v0, d0, v1);
                            break;
                        case 0b00010000:
                            VEXTQ_8(d0, v2, v2, 8); // Swap Up/Lower 64bits parts
                            PMULL2_128(v0, d0, v1);
                            break;
                    }
                }
            } else {
                GETG;
                sse_forget_reg(dyn, ninst, gd);
                sse_reflect_reg(dyn, ninst, vex.v);
                MOV32w(x1, gd); // gx
                MOV32w(x2, vex.v); // vx
                if(MODREG) {
                    ed = (nextop&7)+(rex.b<<3);
                    sse_forget_reg(dyn, ninst, ed);
                    MOV32w(x3, ed);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                    if(ed!=x3) {
                        MOVx_REG(x3, ed);
                    }
                }
                u8 = F8;
                MOV32w(x4, u8);
                CALL_(vex.l?const_native_pclmul_y:const_native_pclmul_x, -1, x3);
            }
            if(!vex.l) YMM0(gd);
            break;

        //case 0x46:    // see 0x06

        case 0x4A:
            INST_NAME("VBLENDVPS Gx, Vx, Ex, XMMImm8");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            u8 = geted_ib(dyn, addr, ninst, nextop)>>4;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { 
                    q1 = sse_get_reg(dyn, ninst, x1, u8, 0);
                    GETGX_empty_VXEX(v0, v2, v1, 1); 
                    F8;
                } else { 
                    v2 = ymm_get_reg(dyn, ninst, x1, vex.v, 0, gd, u8, (MODREG)?((nextop&7)+(rex.b<<3)):-1);
                    if(MODREG)
                        v1 = ymm_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0, gd, vex.v, u8);
                    else
                        VLDR128_U12(v1, ed, fixedaddress+16);
                    q1 = ymm_get_reg(dyn, ninst, x1, u8, 0, vex.v, gd, (MODREG)?((nextop&7)+(rex.b<<3)):-1);
                    v0 = ymm_get_reg_empty(dyn, ninst, x1, gd, vex.v, u8, (MODREG)?((nextop&7)+(rex.b<<3)):-1);
                }
                VSSHRQ_32(q0, q1, 31);   // create mask
                if(v0!=v2) VBIFQ(v0, v2, q0);
                if(v0!=v1) VBITQ(v0, v1, q0);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x4B:
            INST_NAME("VBLENDVPD Gx, Vx, Ex, XMMImm8");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            u8 = geted_ib(dyn, addr, ninst, nextop)>>4;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { 
                    q1 = sse_get_reg(dyn, ninst, x1, u8, 0);
                    GETGX_empty_VXEX(v0, v2, v1, 1); 
                    F8;
                } else { 
                    v2 = ymm_get_reg(dyn, ninst, x1, vex.v, 0, gd, u8, (MODREG)?((nextop&7)+(rex.b<<3)):-1);
                    if(MODREG)
                        v1 = ymm_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0, gd, vex.v, u8);
                    else
                        VLDR128_U12(v1, ed, fixedaddress+16);
                    q1 = ymm_get_reg(dyn, ninst, x1, u8, 0, vex.v, gd, (MODREG)?((nextop&7)+(rex.b<<3)):-1);
                    v0 = ymm_get_reg_empty(dyn, ninst, x1, gd, vex.v, u8, (MODREG)?((nextop&7)+(rex.b<<3)):-1);
                }
                VSSHRQ_64(q0, q1, 63);   // create mask
                if(v0!=v2) VBIFQ(v0, v2, q0);
                if(v0!=v1) VBITQ(v0, v1, q0);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x4C:
            INST_NAME("VPBLENDVB Gx, Vx, Ex, XMMImm8");
            nextop = F8;
            q0 = fpu_get_scratch(dyn, ninst);
            u8 = geted_ib(dyn, addr, ninst, nextop)>>4;
            ed = (nextop&7)+(rex.b<<3);
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { 
                    q1 = sse_get_reg(dyn, ninst, x1, u8, 0);
                    GETGX_empty_VXEX(v0, v2, v1, 1); 
                    F8;
                } else { 
                    v2 = ymm_get_reg(dyn, ninst, x1, vex.v, 0, gd, u8, (MODREG)?ed:-1);
                    if(MODREG)
                        v1 = ymm_get_reg(dyn, ninst, x1, ed, 0, gd, vex.v, u8);
                    else
                        VLDR128_U12(v1, ed, fixedaddress+16);
                    q1 = ymm_get_reg(dyn, ninst, x1, u8, 0, vex.v, gd, (MODREG)?ed:-1);
                    v0 = ymm_get_reg_empty(dyn, ninst, x1, gd, vex.v, u8, (MODREG)?ed:-1);
                }
                VSSHRQ_8(q0, q1, 7);   // create mask
                if(v0!=v2) VBIFQ(v0, v2, q0);
                if(v0!=v1) VBITQ(v0, v1, q0);
            }
            if(!vex.l) YMM0(gd);
            break;

       case 0xDF:
            INST_NAME("VAESKEYGENASSIST Gx, Ex, Ib");
            nextop = F8;
            GETG;
            sse_forget_reg(dyn, ninst, gd);
            MOV32w(x1, gd); // gx
            if(MODREG) {
                ed = (nextop&7)+(rex.b<<3);
                sse_forget_reg(dyn, ninst, ed);
                MOV32w(x2, ed);
                MOV32w(x3, 0);  //p = NULL
            } else {
                MOV32w(x2, 0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                if(ed!=x3) {
                    MOVx_REG(x3, ed);
                }
            }
            u8 = F8;
            MOV32w(x4, u8);
            CALL(const_native_aeskeygenassist, -1);
            YMM0(gd);
            break;

        default:
            DEFAULT;
    }
    return addr;
}
