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
                if( low_blk1_offset == high_blk1_offset) {
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
                if( low_blk2_offset == high_blk2_offset) {
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
        default:
            DEFAULT;
    }
    return addr;
}
