#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "env.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_AVX_66_0F3A(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

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
    static const int8_t round_round[] = {
        0x3, // round to nearest with ties to even
        0x0, // round toward minus infinity
        0x1, // round toward plus infinity
        0x2  // round toward zero
    };
#endif
    rex_t rex = vex.rex;

    switch (opcode) {
        case 0x00:
        case 0x01:
            if (opcode) {
                INST_NAME("VPERMPD Gx, Ex, Imm8");
            } else {
                INST_NAME("VPERMQ Gx, Ex, Imm8");
            }
            nextop = F8;
            if (!vex.l) EMIT(0);
            GETGY_empty_EY_xy(v0, v1, 1);
            u8 = F8;
            XVPERMI_D(v0, v1, u8);
            break;
        case 0x02:
        case 0x0C:
            if (opcode == 0x2) {
                INST_NAME("VPBLENDD Gx, Vx, Ex, Ib");
            } else {
                INST_NAME("VBLENDPS Gx, Vx, Ex, Ib");
            }
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8;
            d0 = fpu_get_scratch(dyn);
            if (vex.l) {
                // 256bits fast path
                if (u8 == 0) {
                    if (v0 != v1) XVOR_V(v0, v1, v1);
                    break;
                } else if (u8 == 0xFF) {
                    if (v0 != v2) XVOR_V(v0, v2, v2);
                    break;
                }
            } else {
                // VEX.128 128bits fast path
                if ((u8 & 0xf) == 0) {
                    if (v0 != v1) VOR_V(v0, v1, v1);
                    break;
                } else if ((u8 & 0xf) == 0xF) {
                    if (v0 != v2) VOR_V(v0, v2, v2);
                    break;
                }
            }
            tmp64u = 0;
            for (int i = 0; i < 8; i++) {
                if (u8 & (1 << i)) tmp64u |= (0xffULL << (i * 8));
            }
            MOV64x(x5, tmp64u);
            MOVGR2FR_D(d0, x5);
            VEXT2XV_W_B(d0, d0);
            XVBITSEL_V(v0, v1, v2, d0);
            break;
        case 0x04:
            INST_NAME("VPERMILPS Gx, Ex, Imm8");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 1);
            u8 = F8;
            VSHUF4Ixy(W, v0, v1, u8);
            break;
        case 0x05:
            INST_NAME("VPERMILPD Gx, Ex, Imm8");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 1);
            u8 = F8;
            u8 = 0b10100000 + ((u8 & 0b00001000) << 3) + ((u8 & 0b00000100) << 2) + ((u8 & 0b00000010) << 1) + (u8 & 0b00000001);
            XVPERMI_D(v0, v1, u8);
            break;
        case 0x06:
        case 0x46:
            if (opcode == 0x06) {
                INST_NAME("VPERM2F128 Gx, Vx, Ex, Imm8");
            } else {
                INST_NAME("VPERM2I128 Gx, Vx, Ex, Imm8");
            }
            nextop = F8;
            if (!vex.l) EMIT(0);
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8;
            if ((u8 & 0x88) == 0x88) {
                XVXOR_V(v0, v0, v0);
                break;
            }
            d0 = fpu_get_scratch(dyn);
            uint8_t zero_low = (u8 & 0x8) >> 3;
            uint8_t zero_up = (u8 & 0x80) >> 7;
            uint8_t vec_lo = (u8 & 0x2) >> 1;
            uint8_t index_lo = u8 & 0x1;
            uint8_t vec_hi = (u8 & 0x20) >> 5;
            uint8_t index_hi = (u8 & 0x10) >> 4;
            if (!zero_low && !zero_up) {
                if (v0 == v1) {
                    XVPERMI_Q(v0, v2, XVPERMI_IMM_4_0((vec_hi ? 0 : 2) | index_hi, (vec_lo ? 0 : 2) | index_lo));
                } else if (v0 == v2) {
                    XVPERMI_Q(v0, v1, XVPERMI_IMM_4_0((vec_hi << 1) | index_hi, ((vec_lo) << 1) | index_lo));
                } else {
                    XVOR_V(v0, v2, v2);
                    XVPERMI_Q(v0, v1, XVPERMI_IMM_4_0((vec_hi << 1) | index_hi, ((vec_lo) << 1) | index_lo));
                }
                break;
            }
            XVXOR_V(d0, d0, d0);
            if (zero_low) {
                XVORI_B(v0, vec_hi ? v2 : v1, 0);
                XVPERMI_Q(v0, d0, XVPERMI_IMM_4_0(2 + index_hi, 0));
            } else {
                XVORI_B(v0, vec_lo ? v2 : v1, 0);
                XVPERMI_Q(v0, d0, XVPERMI_IMM_4_0(0, 2 + index_lo));
            }
            break;
        case 0x08:
            INST_NAME("VROUNDPS Gx, Ex, Ib");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 1);
            u8 = F8;
            if (u8 & 4) {
                u8 = sse_setround(dyn, ninst, x1, x2);
                if (vex.l) {
                    XVFRINT_S(v0, v1);
                } else {
                    VFRINT_S(v0, v1);
                }
                x87_restoreround(dyn, ninst, u8);
            } else {
                if (vex.l) {
                    XVFRINTRRD_S(v0, v1, round_round[u8 & 3]);
                } else {
                    VFRINTRRD_S(v0, v1, round_round[u8 & 3]);
                }
            }
            break;
        case 0x09:
            INST_NAME("VROUNDPD Gx, Ex, Ib");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 1);
            u8 = F8;
            if (u8 & 4) {
                u8 = sse_setround(dyn, ninst, x1, x2);
                if (vex.l) {
                    XVFRINT_D(v0, v1);
                } else {
                    VFRINT_D(v0, v1);
                }
                x87_restoreround(dyn, ninst, u8);
            } else {
                if (vex.l) {
                    XVFRINTRRD_D(v0, v1, round_round[u8 & 3]);
                } else {
                    VFRINTRRD_D(v0, v1, round_round[u8 & 3]);
                }
            }
            break;
        case 0x0A:
            INST_NAME("VROUNDSS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETEYSS(v2, 0, 1);
            GETVYx(v1, 0);
            GETGYx_empty(v0);
            u8 = F8;
            d0 = fpu_get_scratch(dyn);
            if (u8 & 4) {
                u8 = sse_setround(dyn, ninst, x1, x2);
                VFRINT_S(d0, v2);
                x87_restoreround(dyn, ninst, u8);
            } else {
                VFRINTRRD_S(d0, v2, round_round[u8 & 3]);
            }
            if (v0 != v1) VOR_V(v0, v1, v1);
            VEXTRINS_W(v0, d0, 0);
            break;
        case 0x0B:
            INST_NAME("VROUNDSD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETEYSD(v2, 0, 1);
            GETVYx(v1, 0);
            GETGYx_empty(v0);
            u8 = F8;
            d0 = fpu_get_scratch(dyn);
            if (u8 & 4) {
                u8 = sse_setround(dyn, ninst, x1, x2);
                VFRINT_D(d0, v2);
                x87_restoreround(dyn, ninst, u8);
            } else {
                VFRINTRRD_D(d0, v2, round_round[u8 & 3]);
            }
            if (v0 != v1) VOR_V(v0, v1, v1);
            VEXTRINS_D(v0, d0, 0);
            break;
        case 0x0D:
            INST_NAME("VBLENDPD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8;

            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            if (vex.l) {
                u8 = u8 & 0b1111;
                if (u8 == 0b0000) {
                    XVOR_V(v0, v1, v1);
                    break;
                }
                if (u8 == 0b1111) {
                    XVOR_V(v0, v2, v2);
                    break;
                }
                if (u8 == 0b0011) {
                    if (v0 == v1) {
                        XVPERMI_Q(v0, v2, XVPERMI_IMM_4_0(3, 0));
                    } else {
                        XVOR_V(v0, v2, v2);
                        XVPERMI_Q(v0, v1, XVPERMI_IMM_4_0(1, 2));
                    }
                    break;
                }
                if (u8 == 0b1100) {
                    if (v0 == v1) {
                        XVPERMI_Q(v0, v2, XVPERMI_IMM_4_0(1, 2));
                    } else {
                        XVOR_V(v0, v2, v2);
                        XVPERMI_Q(v0, v1, XVPERMI_IMM_4_0(3, 0));
                    }
                    break;
                }
                XVOR_V(d0, v1, v1);
                XVOR_V(d1, v1, v1);
                if (u8 & 1) XVEXTRINS_D(d0, v2, VEXTRINS_IMM_4_0(0, 0));
                if (u8 & 2) XVEXTRINS_D(d0, v2, VEXTRINS_IMM_4_0(1, 1));
                if (u8 & 4) XVEXTRINS_D(d1, v2, VEXTRINS_IMM_4_0(0, 0));
                if (u8 & 8) XVEXTRINS_D(d1, v2, VEXTRINS_IMM_4_0(1, 1));
                XVPERMI_Q(d0, d1, XVPERMI_IMM_4_0(1, 2));
                XVOR_V(v0, d0, d0);
            } else {
                u8 = u8 & 0b11;
                switch (u8) {
                    case 0b00:
                        VOR_V(v0, v1, v1);
                        break;
                    case 0b11:
                        VOR_V(v0, v2, v2);
                        break;
                    case 0b01:
                        VEXTRINS_D(v0, v1, VEXTRINS_IMM_4_0(1, 1));
                        VEXTRINS_D(v0, v2, VEXTRINS_IMM_4_0(0, 0));
                        break;
                    case 0b10:
                        VEXTRINS_D(v0, v1, VEXTRINS_IMM_4_0(0, 0));
                        VEXTRINS_D(v0, v2, VEXTRINS_IMM_4_0(1, 1));
                }
            }
            break;
        case 0x0E:
            INST_NAME("VPBLENDW Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8;
            d0 = fpu_get_scratch(dyn);
            if (u8 == 0) {
                if (v0 != v1) VOR_Vxy(v0, v1, v1);
                break;
            } else if (u8 == 0xFF) {
                if (v0 != v2) VOR_Vxy(v0, v2, v2);
                break;
            }
            tmp64u = 0;
            for (int i = 0; i < 8; i++) {
                if (u8 & (1 << i)) tmp64u |= (0xffULL << (i * 8));
            }
            MOV64x(x5, tmp64u);
            MOVGR2FR_D(d0, x5);
            VEXT2XV_H_B(d0, d0);
            XVPERMI_Q(d0, d0, 0);
            XVBITSEL_V(v0, v1, v2, d0);
            break;
        case 0x0F:
            INST_NAME("VPALIGNR Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8;
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            if (vex.l) {
                if (u8 > 31) {
                    XVXOR_V(v0, v0, v0);
                    break;
                } else {
                    if (u8 > 15) {
                        XVBSRL_V(v0, v1, u8 - 16);
                    } else if (!u8) {
                        XVOR_V(v0, v2, v2);
                    } else {
                        XVBSLL_V(d0, v1, 16 - u8);
                        XVBSRL_V(d1, v2, u8);
                        XVOR_V(v0, d0, d1);
                    }
                }
            } else {
                if (u8 > 31) {
                    XVXOR_V(v0, v0, v0);
                    YMM_UNMARK_UPPER_ZERO(v0);
                } else if (u8 > 15) {
                    VBSRL_V(v0, v1, u8 - 16);
                } else if (!u8) {
                    VOR_V(v0, v2, v2);
                } else {
                    VBSLL_V(d0, v1, 16 - u8);
                    VBSRL_V(d1, v2, u8);
                    VOR_V(v0, d0, d1);
                }
            }
            break;
        case 0x14:
            INST_NAME("VPEXTRB Ed, Gx, imm8");
            nextop = F8;
            GETGYx(q0, 0);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                u8 = (F8) & 15;
                VPICKVE2GR_BU(ed, q0, u8);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x4, &fixedaddress, rex, NULL, 0, 1);
                u8 = (F8) & 15;
                VSTELM_B(q0, wback, 0, u8);
            }
            break;
        case 0x15:
            INST_NAME("VPEXTRW Ed, Gx, imm8");
            nextop = F8;
            GETGYx(q0, 0);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                u8 = (F8) & 7;
                VPICKVE2GR_HU(ed, q0, u8);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x4, &fixedaddress, rex, NULL, 0, 1);
                u8 = (F8) & 7;
                VSTELM_H(q0, wback, 0, u8);
            }
            break;
        case 0x16:
            if (rex.w) {
                INST_NAME("VPEXTRQ Ed, Gx, Ib");
            } else {
                INST_NAME("VPEXTRD Ed, Gx, Ib");
            }
            nextop = F8;
            GETGYx(q0, 0);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                u8 = F8;
                if (rex.w) {
                    VPICKVE2GR_D(ed, q0, (u8 & 1));
                } else {
                    VPICKVE2GR_WU(ed, q0, (u8 & 3));
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x5, &fixedaddress, rex, NULL, 0, 1);
                u8 = F8;
                if (rex.w) {
                    VSTELM_D(q0, ed, 0, (u8 & 1));
                } else {
                    VSTELM_W(q0, ed, 0, (u8 & 3));
                }
                SMWRITE2();
            }
            break;
        case 0x17:
            INST_NAME("VEXTRACTPS Ed, Gx, imm8");
            nextop = F8;
            GETGYx(v0, 0);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                u8 = F8 & 0b11;
                VPICKVE2GR_WU(ed, v0, u8);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x5, &fixedaddress, rex, NULL, 0, 1);
                u8 = F8 & 0b11;
                VSTELM_W(v0, ed, 0, u8);
                SMWRITE2();
            }
            break;
        case 0x18:
        case 0x38:
            if (opcode == 0x18) {
                INST_NAME("VINSERTF128 Gx, Vx, Ex, imm8");
            } else {
                INST_NAME("VINSERTI128 Gx, Vx, Ex, imm8");
            }
            nextop = F8;
            GETEYx(q2, 0, 1);
            GETVYy(q1, 0);
            GETGYy_empty(q0);
            u8 = F8;
            if (q0 != q2) {
                if (q0 != q1) XVOR_V(q0, q1, q1);
                XVPERMI_Q(q0, q2, ((u8 & 1) == 0) ? 0x30 : 0x02);
            } else {
                XVPERMI_Q(q0, q1, ((u8 & 1) == 0) ? 0x12 : 0x20);
            }
            break;
        case 0x19:
        case 0x39:
            if (opcode == 0x19) {
                INST_NAME("VEXTRACTF128 Ex, Gx, imm8");
            } else {
                INST_NAME("VEXTRACTI128 Ex, Gx, imm8");
            }
            nextop = F8;
            GETGYy(q0, 0);
            if (MODREG) {
                GETEYx_empty(q1, 1);
                u8 = F8;
                if ((u8 & 1) == 0) {
                    VOR_V(q1, q0, q0);
                } else {
                    XVPERMI_Q(q1, q0, XVPERMI_IMM_4_0(3, 1));
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x4, x5, &fixedaddress, rex, NULL, 0, 1);
                u8 = F8;
                if ((u8 & 1) == 1) {
                    XVSTELM_D(q0, ed, 0, 2);
                    XVSTELM_D(q0, ed, 1, 3);
                } else {
                    VST(q0, ed, fixedaddress);
                }
                SMWRITE2();
            }
            break;
        case 0x1D:
            INST_NAME("VCVTPS2PH Ex, Gx, Ib");
            nextop = F8;
            GETGYxy(v0, 0);
            if (vex.l) {
                GETEYx(v1, 1, 1);
            } else {
                GETEYSD(v1, 1, 1);
            }
            u8 = F8;
            d0 = fpu_get_scratch(dyn);
            if (u8 & 4) {
                u8 = sse_setround(dyn, ninst, x1, x2);
            } else {
                MOVFCSR2GR(x4, FCSR3);
                ORI(x5, x5, round_round[u8 & 3]);
                SLLI_D(x5, x5, 8);
                MOVGR2FCSR(FCSR3, x5);
                u8 = x4;
            }
            if (vex.l) {
                XVXOR_V(d0, d0, d0);
                XVFCVT_H_S(v1, d0, v0);
                XVPERMI_D(v1, v1, 0b11011000);
                PUTEYx(v1);
            } else {
                XVXOR_V(d0, d0, d0);
                VFCVT_H_S(v1, d0, v0);
                PUTEYSD(v1);
            }
            x87_restoreround(dyn, ninst, u8);
            break;
        case 0x20:
            INST_NAME("VPINSRB Gx, Vx, ED, Ib");
            nextop = F8;
            GETEB(x5, 1);
            GETVYx(v1, 0);
            GETGYx_empty(v0);
            u8 = F8;
            if (v0 != v1) VOR_V(v0, v1, v1);
            VINSGR2VR_B(v0, ed, (u8 & 0xf));
            break;
        case 0x21:
            INST_NAME("VINSERTPS Gx, Vx, Ex, Ib");
            nextop = F8;
            q1 = fpu_get_scratch(dyn);
            if (MODREG) {
                GETGY_empty_VYEY_xy(v0, v1, v2, 1);
                u8 = F8;
                if (v0 == v2) {
                    VOR_V(q1, v2, v2);
                    if (v0 != v1) VOR_V(v0, v1, v1);
                    VEXTRINS_W(v0, q1, VEXTRINS_IMM_4_0((u8 >> 4) & 3, (u8 >> 6) & 3));
                } else {
                    if (v0 != v1) VOR_V(v0, v1, v1);
                    VEXTRINS_W(v0, v2, VEXTRINS_IMM_4_0((u8 >> 4) & 3, (u8 >> 6) & 3));
                }
            } else {
                GETVYx(v1, 0);
                GETGYx_empty(v0);
                if (v0 != v1) VOR_V(v0, v1, v1);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x5, &fixedaddress, rex, NULL, 1, 1);
                u8 = F8;
                FLD_S(q1, wback, fixedaddress);
                VEXTRINS_W(v0, q1, VEXTRINS_IMM_4_0((u8 >> 4) & 3, 0)); // src index is zero when Ex is mem operand
            }
            uint8_t zmask = u8 & 0xf;
            if (zmask) {
                VXOR_V(q1, q1, q1);
                for (uint8_t i = 0; i < 4; i++) {
                    if (zmask & (1 << i)) {
                        VEXTRINS_W(v0, q1, VEXTRINS_IMM_4_0(i, 0));
                    }
                }
            }
            break;
        case 0x22:
            if (rex.w) {
                INST_NAME("VPINSRQ Gx, Vx, ED, Ib");
            } else {
                INST_NAME("VPINSRD Gx, Vx, ED, Ib");
            }
            nextop = F8;
            GETED(1);
            GETVYx(v1, 0);
            GETGYx_empty(v0);
            u8 = F8;
            if (v0 != v1) VOR_V(v0, v1, v1);
            if (rex.w) {
                VINSGR2VR_D(v0, ed, (u8 & 0x1));
            } else {
                VINSGR2VR_W(v0, ed, (u8 & 0x3));
            }
            break;
        case 0x40:
            INST_NAME("VDPPS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8;
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            d2 = fpu_get_scratch(dyn);
            VFMULxy(S, d0, v1, v2);
            VXOR_Vxy(d2, d2, d2);
            for (int i = 0; i < 4; ++i) {
                if (!(u8 & (1 << (4 + i)))) {
                    VEXTRINSxy(W, d0, d2, (i << 4));
                }
            }
            VSHUF4Ixy(W, d1, d0, 0b10110001); // v0[a,b,c,d] v1[b,a,d,c]
            VFADDxy(S, d0, d0, d1);           // v0[ab,ba,cd,dc]
            VSHUF4Ixy(W, d1, d0, 0b01001110); // v1[cd,dc,ab,ba]
            VFADDxy(S, d0, d0, d1);           // v0[abcd,badc,cdab,dcba]
            VREPLVEIxy(W, v0, d0, 0);
            for (int i = 0; i < 4; ++i) {
                if (!(u8 & (1 << i))) {
                    VEXTRINSxy(W, v0, d2, (i << 4));
                }
            }
            break;
        case 0x41:
            INST_NAME("VDPPD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8;
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            d2 = fpu_get_scratch(dyn);
            VFMULxy(D, d0, v1, v2);
            VXOR_Vxy(d2, d2, d2);
            for (int i = 0; i < 2; ++i) {
                if (!(u8 & (1 << (4 + i)))) {
                    VEXTRINSxy(D, d0, d2, (i << 4));
                }
            }
            VSHUF4Ixy(W, d1, d0, 0b01001110); // v0[a,b] v1[b,a]
            VFADDxy(D, d0, d0, d1);           // v0[ab,ba]
            VREPLVEIxy(D, v0, d0, 0);
            for (int i = 0; i < 2; ++i) {
                if (!(u8 & (1 << i))) {
                    VEXTRINSxy(D, v0, d2, (i << 4));
                }
            }
            break;
        case 0x42:
            INST_NAME("VMPSADBW Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8;
            if (vex.l) {
                uint8_t low_blk2_offset = 4 * (u8 & 3);
                uint8_t low_blk1_offset = 4 * ((u8 >> 2) & 1);
                uint8_t high_blk2_offset = 4 * ((u8 >> 3) & 3);
                uint8_t high_blk1_offset = 4 * ((u8 >> 5) & 1);
                q0 = fpu_get_scratch(dyn);
                q1 = fpu_get_scratch(dyn);
                q2 = fpu_get_scratch(dyn);
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                if (low_blk1_offset == high_blk1_offset) {
                    // generate hi128/low128 mask in one shot
                    XVMEPATMSK_V(d0, 1, low_blk1_offset);
                    XVMEPATMSK_V(d1, 1, low_blk1_offset + 4);
                    XVSHUF_B(q0, v1, v1, d0);
                    XVSHUF_B(q2, v1, v1, d1);
                } else {
                    XVMEPATMSK_V(d0, 1, low_blk1_offset);
                    XVMEPATMSK_V(d1, 1, high_blk1_offset);
                    XVSHUF_B(q0, v1, v1, d0);
                    XVSHUF_B(q1, v1, v1, d1);
                    XVPERMI_Q(q0, q1, XVPERMI_IMM_4_0(1, 2));
                    XVMEPATMSK_V(d0, 1, low_blk1_offset + 4);
                    XVMEPATMSK_V(d1, 1, high_blk1_offset + 4);
                    XVSHUF_B(q2, v1, v1, d0);
                    XVSHUF_B(q1, v1, v1, d1);
                    XVPERMI_Q(q2, q1, XVPERMI_IMM_4_0(1, 2));
                }
                if (low_blk2_offset == high_blk2_offset) {
                    // generate hi128/low128 mask in one shot
                    XVBSRL_V(q1, v2, low_blk2_offset);
                    XVSHUF4I_W(q1, q1, 0b00000000);
                } else {
                    XVBSRL_V(q1, v2, low_blk2_offset);
                    XVBSRL_V(d1, v2, high_blk2_offset);
                    XVPERMI_Q(q1, d1, XVPERMI_IMM_4_0(1, 2));
                    XVSHUF4I_W(q1, q1, 0b00000000);
                }
                XVABSD_BU(d0, q0, q1);
                XVABSD_BU(d1, q2, q1);
                XVHADDW_HU_BU(d0, d0, d0);
                XVHADDW_HU_BU(d1, d1, d1);
                XVHADDW_WU_HU(d0, d0, d0);
                XVHADDW_WU_HU(d1, d1, d1);
                XVSSRANI_HU_W(d0, d0, 0);
                XVSSRANI_HU_W(d1, d1, 0);
                XVEXTRINS_D(v0, d0, VEXTRINS_IMM_4_0(0, 0));
                XVEXTRINS_D(v0, d1, VEXTRINS_IMM_4_0(1, 0));
            } else {
                uint8_t blk2_offset = 4 * (u8 & 3);
                uint8_t blk1_offset = 4 * ((u8 >> 2) & 1);
                q0 = fpu_get_scratch(dyn);
                q1 = fpu_get_scratch(dyn);
                q2 = fpu_get_scratch(dyn);
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VMEPATMSK_V(d0, 1, blk1_offset);
                VMEPATMSK_V(d1, 1, blk1_offset + 4);
                VSHUF_B(q0, v1, v1, d0);
                VSHUF_B(q2, v1, v1, d1);
                VBSRL_V(q1, v2, blk2_offset);
                VSHUF4I_W(q1, q1, 0b00000000);

                VABSD_BU(d0, q0, q1);
                VABSD_BU(d1, q2, q1);
                VHADDW_HU_BU(d0, d0, d0);
                VHADDW_HU_BU(d1, d1, d1);
                VHADDW_WU_HU(d0, d0, d0);
                VHADDW_WU_HU(d1, d1, d1);
                VSSRANI_HU_W(d0, d0, 0);
                VSSRANI_HU_W(d1, d1, 0);
                VEXTRINS_D(v0, d0, VEXTRINS_IMM_4_0(0, 0));
                VEXTRINS_D(v0, d1, VEXTRINS_IMM_4_0(1, 0));
            }
            break;
        case 0x44:
            INST_NAME("VPCLMULQDQ Gx, Vx, Ex, Ib");
            nextop = F8;
            GETG;
            avx_forget_reg(dyn, ninst, gd);
            avx_reflect_reg(dyn, ninst, vex.v);
            MOV32w(x1, gd);    // gx
            MOV32w(x2, vex.v); // vx
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                avx_forget_reg(dyn, ninst, ed);
                MOV32w(x3, ed); // ex
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x5, &fixedaddress, rex, NULL, 0, 1);
                if (ed != x3) MV(x3, ed);
            }
            u8 = F8;
            MOV32w(x4, u8);
            CALL_(vex.l ? const_native_pclmul_y : const_native_pclmul_x, -1, x3, x1, x2);
            if (!vex.l) {
                ST_D(xZR, xEmu, offsetof(x64emu_t, ymm[gd]));
                ST_D(xZR, xEmu, offsetof(x64emu_t, ymm[gd]) + 8);
            }
            break;
        case 0x4A:
            INST_NAME("VBLENDVPS Gx, Vx, Ex, XMMImm8");
            nextop = F8;
            u8 = geted_ib(dyn, addr, ninst, nextop) >> 4;
            d0 = avx_get_reg(dyn, ninst, x5, u8, 0, LSX_AVX_WIDTH_128);
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            F8;
            q0 = fpu_get_scratch(dyn);
            VSLTIxy(W, q0, d0, 0);
            VBITSEL_Vxy(v0, v1, v2, q0);
            break;
        case 0x4B:
            INST_NAME("VBLENDVPD Gx, Vx, Ex, XMMImm8");
            nextop = F8;
            u8 = geted_ib(dyn, addr, ninst, nextop) >> 4;
            d0 = avx_get_reg(dyn, ninst, x5, u8, 0, LSX_AVX_WIDTH_128);
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            F8;
            q0 = fpu_get_scratch(dyn);
            VSLTIxy(D, q0, d0, 0);
            VBITSEL_Vxy(v0, v1, v2, q0);
            break;
        case 0x4C:
            INST_NAME("VPBLENDVB Gx, Vx, Ex, XMMImm8");
            nextop = F8;
            u8 = geted_ib(dyn, addr, ninst, nextop) >> 4;
            d0 = avx_get_reg(dyn, ninst, x5, u8, 0, LSX_AVX_WIDTH_128);
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            F8;
            q0 = fpu_get_scratch(dyn);
            VSLTIxy(B, q0, d0, 0);
            VBITSEL_Vxy(v0, v1, v2, q0);
            break;
        case 0xDF:
            INST_NAME("VAESKEYGENASSIST Gx, Ex, Ib");
            nextop = F8;
            GETG;
            avx_forget_reg(dyn, ninst, gd);
            MOV32w(x1, gd); // gx
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                avx_forget_reg(dyn, ninst, ed);
                MOV32w(x2, ed);
                MOV32w(x3, 0); // p = NULL
            } else {
                MOV32w(x2, 0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 0, 1);
                if (ed != x3) {
                    MV(x3, ed);
                }
            }
            u8 = F8;
            MOV32w(x4, u8);
            CALL4(const_native_aeskeygenassist, -1, x1, x2, x3, x4);
            if (!vex.l) {
                ST_D(xZR, xEmu, offsetof(x64emu_t, ymm[gd]));
                ST_D(xZR, xEmu, offsetof(x64emu_t, ymm[gd]) + 8);
            }
            break;

        default:
            DEFAULT;
    }
    return addr;
}
