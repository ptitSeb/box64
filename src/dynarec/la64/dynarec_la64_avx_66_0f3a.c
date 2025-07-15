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
            u8 = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            if (u8 == 0x88) {
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
                u8 = F8 & 0b11;
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
            GETGY_empty_VYEY_xy(q0, q1, q2, 1);
            u8 = F8;
            XVOR_V(q0, q1, q1);
            XVPERMI_Q(q0, q2, (u8 & 1) == 0 ? 0b00110000 : 0b00000010);
            break;
        case 0x19:
        case 0x39:
            if (opcode == 0x19) {
                INST_NAME("VEXTRACTF128 Ex, Gx, imm8");
            } else {
                INST_NAME("VEXTRACTI128 Ex, Gx, imm8");
            }
            nextop = F8;
            GETEY_GY_xy(q1, q0, 1);
            u8 = F8;
            if (MODREG) {
                XVPERMI_Q(q1, q0, (u8 & 1) == 0 ? XVPERMI_IMM_4_0(3, 0) : XVPERMI_IMM_4_0(3, 1));
            } else {
                if ((u8 & 1) == 1) {
                    XVPERMI_Q(q1, q0, XVPERMI_IMM_4_0(3, 1));
                    VST(q1, ed, fixedaddress);
                } else {
                    VST(q0, ed, fixedaddress);
                }
            }
            break;
        case 0x21:
            INST_NAME("VINSERTPS Gx, Vx, Ex, Ib");
            nextop = F8;
            uint8_t src_index = (u8 >> 6) & 3;
            uint8_t dst_index = (u8 >> 4) & 3;
            uint8_t zmask = u8 & 0xf;
            q1 = fpu_get_scratch(dyn);
            if (MODREG) {
                GETGY_empty_VYEY_xy(v0, v1, v2, 1);
                u8 = F8;
                if (v0 == v2) {
                    VOR_V(q1, v2, v2);
                    if (v0 != v1) VOR_V(v0, v1, v1);
                    VEXTRINS_W(v0, q1, VEXTRINS_IMM_4_0(dst_index, src_index));
                } else {
                    if (v0 != v1) VOR_V(v0, v1, v1);
                    VEXTRINS_W(v0, v2, VEXTRINS_IMM_4_0(dst_index, src_index));
                }
            } else {
                GETVYx(v1, 0);
                GETGYx_empty(v0);
                u8 = F8;
                if (v0 != v1) VOR_V(v0, v1, v1);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x5, &fixedaddress, rex, NULL, 0, 1);
                u8 = F8;
                FLD_S(q1, wback, fixedaddress);
                VEXTRINS_W(v0, q1, VEXTRINS_IMM_4_0(dst_index, 0)); // src index is zero when Ex is mem operand
            }
            VXOR_V(q1, q1, q1);
            if (zmask) {
                for (uint8_t i = 0; i < 4; i++) {
                    if (zmask & (1 << i)) {
                        VEXTRINS_W(v0, q1, VEXTRINS_IMM_4_0(i, 0));
                    }
                }
            }
            break;
        case 0x2A:
            INST_NAME("VMOVNTDQA Gx, Ex");
            nextop = F8;
            if (MODREG) {
                DEFAULT;
            } else {
                GETGYxy_empty(q0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 0, 0);
                if (vex.l) {
                    XVLD(q0, ed, fixedaddress);
                } else {
                    VLD(q0, ed, fixedaddress);
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
        default:
            DEFAULT;
    }
    return addr;
}
