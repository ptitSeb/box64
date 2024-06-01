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

    rex_t rex = vex.rex;

    switch(opcode) {

        case 0x0C:
            INST_NAME("VPBLENDPS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGX_empty_VXEX(q0, q2, q1, 1);
            u8 = F8;
            if(q0==q1) {
                for(int i=0; i<4; ++i)
                    if(u8&(1<<i)) {
                        VMOVeS(q0, i, q1, i);
                    } else if(q0!=q2)
                        VMOVeS(q0, i, q2, i);
            } else {
                if(q0!=q2)
                    VMOVQ(q0, q2);
                if((u8&15)==0b0011) {
                    VMOVeD(q0, 0, q1, 0);
                } else if((u8&15)==0b1100) {
                    VMOVeD(q0, 1, q1, 1);
                } else for(int i=0; i<4; ++i)
                    if(u8&(1<<i)) {
                        VMOVeS(q0, i, q1, i);
                    }
            }
            if(vex.l) {
                GETGY_empty_VYEY(q0, q2, q1);
                if(q0==q1) {
                    for(int i=0; i<4; ++i)
                        if(u8&(1<<(i+4))) {
                            VMOVeS(q0, i, q1, i);
                        } else if(q0!=q2)
                            VMOVeS(q0, i, q2, i);
                } else {
                    if(q0!=q2)
                        VMOVQ(q0, q2);
                    if((u8>>4)==0b0011) {
                        VMOVeD(q0, 0, q1, 0);
                    } else if((u8>>4)==0b1100) {
                        VMOVeD(q0, 1, q1, 1);
                    } else for(int i=0; i<4; ++i)
                        if(u8&(1<<(i+4))) {
                            VMOVeS(q0, i, q1, i);
                        }
                }
            } else YMM0(gd);
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

        case 0x15:
            INST_NAME("VPEXTRW Ed, Gx, imm8");
            nextop = F8;
            GETGX(v0, 0);
            if(MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 1);
                ed = x1;
            }
            u8 = F8;
            VMOVHto(ed, v0, u8&7);
            if(!MODREG) {
                STH(ed, wback, fixedaddress);
            }
            break;
        case 0x16:
            INST_NAME("VPEXTRD/Q Ed, Gx, imm8");
            nextop = F8;
            GETGX(v0, 0);
            if(MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
            } else {
                SMREAD();
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
            }
            break;
        case 0x17:
            INST_NAME("VEXTRACTPS Ed, Gx, imm8");
            nextop = F8;
            GETGX(v0, 0);
            if(MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 1);
                ed = x1;
            }
            u8 = F8;
            VMOVSto(ed, v0, u8&3);
            if(!MODREG) {
                STW(ed, wback, fixedaddress);
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
            GETGY_empty_VY(q0, q2, 0, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1);
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
            if(u8&1) {GETG; GETGY(v0, 0, -1, -1, -1);} else {GETGX(v0, 0);}
            if(MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop&7)+(rex.b<<3));
                VMOVQ(v1, v0);
                YMM0((nextop&7)+(rex.b<<3));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 1);
                VST128(v0, ed, fixedaddress);
            }
            F8; // read u8, but it's been already handled
            break;

        case 0x44:
            INST_NAME("PCLMULQDQ Gx, Vx, Ex, Ib");
            nextop = F8;
            if(arm64_pmull) {
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
                for(int l=0; l<1+vex.l; ++l) {
                    if(!l) {
                        GETG;
                        sse_forget_reg(dyn, ninst, gd);
                        sse_reflect_reg(dyn, ninst, vex.v);
                    }
                    MOV32w(x1, gd); // gx
                    MOV32w(x2, vex.v); // vx
                    if(MODREG) {
                        if(!l) {
                            ed = (nextop&7)+(rex.b<<3);
                            sse_forget_reg(dyn, ninst, ed);
                        }
                        MOV32w(x3, ed);
                    } else {
                        if(!l) {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                            if(ed!=x3) {
                                MOVx_REG(x3, ed);
                            }
                        } else {
                            ADDx_U12(x3, ed, 16);
                        }
                    }
                    if(!l) u8 = F8;
                    MOV32w(x4, u8);
                    CALL_(l?native_pclmul_y:native_pclmul_x, -1, x3);
                }
            }
            if(!vex.l) YMM0(gd);
            break;

        default:
            DEFAULT;
    }
    return addr;
}
