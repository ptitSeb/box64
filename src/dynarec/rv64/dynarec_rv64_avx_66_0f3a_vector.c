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

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_AVX_66_0F3A_vector(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    int v0, v1;
    int q0, q1, q2;
    int64_t fixedaddress;
    rex_t rex = vex.rex;

    MAYUSE(v1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(q2);

    switch (opcode) {
        case 0x04:
            INST_NAME("VPERMILPS Gx, Ex, Ib");
            nextop = F8;
            GETEY_vector(q0, 1, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            u8 = F8;
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            for (int i = 0; i < (4 << vex.l); ++i) {
                int lane = i & ~3;
                int selector = lane + ((u8 >> ((i & 3) * 2)) & 3);
                MOV32w(x4, selector);
                q1 = fpu_get_scratch(dyn);
                VRGATHER_VX(q1, q0, x4, VECTOR_UNMASKED);
                VECTOR_LOAD_VMASK(1 << i, x4, vex.l ? 2 : 1);
                VMERGE_VVM(v0, v0, q1);
            }
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x05:
            INST_NAME("VPERMILPD Gx, Ex, Ib");
            nextop = F8;
            GETEY_vector(q0, 1, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            u8 = F8;
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            for (int i = 0; i < (2 << vex.l); ++i) {
                int lane = i & ~1;
                int selector = lane + ((u8 >> i) & 1);
                MOV32w(x4, selector);
                q1 = fpu_get_scratch(dyn);
                VRGATHER_VX(q1, q0, x4, VECTOR_UNMASKED);
                VECTOR_LOAD_VMASK(1 << i, x4, vex.l ? 2 : 1);
                VMERGE_VVM(v0, v0, q1);
            }
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x06:
            if (!vex.l) return 0;
            INST_NAME("VPERM2F128 Gx, Vx, Ex, Ib");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW8);
            GETEY_vector(q1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            u8 = F8;
            q2 = fpu_get_scratch(dyn);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            for (int i = 0; i < 2; ++i) {
                int control = i ? (u8 >> 4) : u8;
                int source = (control >> 1) & 1;
                int index = control & 1;
                int zero = control & 8;
                if (zero) {
                    VXOR_VV(q2, q2, q2, VECTOR_UNMASKED);
                } else if (source) {
                    if (index)
                        VSLIDEDOWN_VI(q2, q1, 16, VECTOR_UNMASKED);
                    else
                        VMV_V_V(q2, q1);
                } else {
                    if (index)
                        VSLIDEDOWN_VI(q2, q0, 16, VECTOR_UNMASKED);
                    else
                        VMV_V_V(q2, q0);
                }
                if (i) {
                    VSLIDEUP_VI(q1, q2, 16, VECTOR_UNMASKED);
                    q2 = q1;
                }
                VECTOR_LOAD_VMASK(i ? 0xffff0000 : 0xffff, x4, 2);
                VMERGE_VVM(v0, v0, q2);
            }
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x0E:
            INST_NAME("VPBLENDW Gx, Vx, Ex, Ib");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 1, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            u8 = F8;
            VMV_V_V(v0, q0);
            VECTOR_LOAD_VMASK(u8 | (vex.l ? (u8 << 8) : 0), x4, vex.l ? 2 : 1);
            VMERGE_VVM(v0, q0, q1);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x18:
            if (!vex.l) return 0;
            INST_NAME("VINSERTF128 Gx, Vx, Ex, Ib");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW8);
            q1 = fpu_get_scratch(dyn);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                avx_load_reg_vector(dyn, ninst, x1, q1, ed, 16, VECTOR_SEW8);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 16);
                VLE8_V(q1, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            u8 = F8;
            if (!(u8 & 1))
                v1 = fpu_get_scratch(dyn);
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 16);
            sse_forget_reg_vector(dyn, ninst, x1, gd);
            ADDI(x1, xEmu, offsetof(x64emu_t, xmm[gd]));
            if (u8 & 1) {
                VSE8_V(q0, x1, VECTOR_UNMASKED, VECTOR_NFIELD1);
                ADDI(x1, xEmu, offsetof(x64emu_t, ymm[gd]));
                VSE8_V(q1, x1, VECTOR_UNMASKED, VECTOR_NFIELD1);
            } else {
                VSE8_V(q1, x1, VECTOR_UNMASKED, VECTOR_NFIELD1);
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 32);
                VSLIDEDOWN_VI(v1, q0, 16, VECTOR_UNMASKED);
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 16);
                ADDI(x1, xEmu, offsetof(x64emu_t, ymm[gd]));
                VSE8_V(v1, x1, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 32);
            break;
        case 0x19:
            if (!vex.l) return 0;
            INST_NAME("VEXTRACTF128 Ex, Gx, Ib");
            nextop = F8;
            GETGY_vector(q0, VECTOR_SEW8);
            q1 = fpu_get_scratch(dyn);
            if (MODREG) {
                u8 = F8;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
                u8 = F8;
            }
            if (u8 & 1) {
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 32);
                VSLIDEDOWN_VI(q1, q0, 16, VECTOR_UNMASKED);
            } else {
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 16);
                VMV_V_V(q1, q0);
            }
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                avx_store_reg_vector(dyn, ninst, x1, q1, ed, 16, VECTOR_SEW8);
            } else {
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 16);
                VSE8_V(q1, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
                SMWRITE2();
            }
            break;
        case 0x38:
            if (!vex.l) return 0;
            INST_NAME("VINSERTI128 Gx, Vx, Ex, Ib");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            q1 = fpu_get_scratch(dyn);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                avx_load_reg_vector(dyn, ninst, x1, q1, ed, 16, VECTOR_SEW64);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW64, 16);
                VLE64_V(q1, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            u8 = F8;
            GETGY_empty_vector(v0);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW64, 32);
            VMV_V_V(v0, q0);
            if (u8 & 1) {
                v1 = fpu_get_scratch(dyn);
                VXOR_VV(v1, v1, v1, VECTOR_UNMASKED);
                VSLIDEUP_VI(v1, q1, 2, VECTOR_UNMASKED);
                VECTOR_LOAD_VMASK(0b1100, x4, 2);
                VMERGE_VVM(v0, v0, v1);
            } else {
                VECTOR_LOAD_VMASK(0b0011, x4, 2);
                VMERGE_VVM(v0, v0, q1);
            }
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x39:
            if (!vex.l) return 0;
            INST_NAME("VEXTRACTI128 Ex, Gx, Ib");
            nextop = F8;
            GETGY_vector(q0, VECTOR_SEW8);
            q1 = fpu_get_scratch(dyn);
            if (MODREG) {
                u8 = F8;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
                u8 = F8;
            }
            if (u8 & 1) {
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 32);
                VSLIDEDOWN_VI(q1, q0, 16, VECTOR_UNMASKED);
            } else {
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 16);
                VMV_V_V(q1, q0);
            }
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                avx_store_reg_vector(dyn, ninst, x1, q1, ed, 16, VECTOR_SEW8);
            } else {
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 16);
                VSE8_V(q1, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
                SMWRITE2();
            }
            break;
        case 0x42:
            INST_NAME("VMPSADBW Gx, Vx, Ex, Ib");
            nextop = F8;
            mmx_purgecache(dyn, ninst, 0, x1);
            GETVY_vector(q0, VECTOR_SEW8);
            GETEY_vector(q1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            u8 = F8;
            q2 = fpu_get_scratch(dyn);
            int q3 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            int q4 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            int q5 = fpu_get_scratch(dyn);
            int q6 = fpu_get_scratch(dyn);
            int q7 = fpu_get_scratch(dyn);
            int q8 = fpu_get_scratch(dyn);
            int q9 = fpu_get_scratch(dyn);

            for (int pass = 0; pass < (vex.l ? 2 : 1); ++pass) {
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 2);
                if (pass)
                    VSLIDEDOWN_VI(q7, q0, 16, VECTOR_UNMASKED);
                else
                    VMV_V_V(q7, q0);
                if (pass)
                    VSLIDEDOWN_VI(q8, q1, 16, VECTOR_UNMASKED);
                else
                    VMV_V_V(q8, q1);

                VID_V(q5, VECTOR_UNMASKED);
                VMV_V_V(q6, q5);
                VAND_VI(q5, q5, 7, VECTOR_UNMASKED);
                VSRL_VI(q6, q6, 3, VECTOR_UNMASKED);
                VADD_VV(q5, q5, q6, VECTOR_UNMASKED);
                if (pass)
                    VADD_VI(q5, q5, ((u8 >> 5) & 1) * 4, VECTOR_UNMASKED);
                else
                    VADD_VI(q5, q5, ((u8 >> 2) & 1) * 4, VECTOR_UNMASKED);
                MOV32w(x4, pass ? (((u8 >> 3) & 3) * 4) : ((u8 & 3) * 4));
                VADD_VX(q6, q6, x4, VECTOR_UNMASKED);

                VRGATHER_VV(q3, q7, q5, VECTOR_UNMASKED);
                VRGATHER_VV(q4, q8, q6, VECTOR_UNMASKED);
                VMAXU_VV(q7, q3, q4, VECTOR_UNMASKED);
                VMINU_VV(q4, q3, q4, VECTOR_UNMASKED);
                VSUB_VV(q7, q7, q4, VECTOR_UNMASKED);

                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL2, 4);
                VZEXT_VF2(q3, q7, VECTOR_UNMASKED);
                VSLIDEDOWN_VI(q4, q3, 8, VECTOR_UNMASKED);
                VADD_VV(q3, q3, q4, VECTOR_UNMASKED);
                VSLIDEDOWN_VI(q4, q3, 16, VECTOR_UNMASKED);
                VADD_VV(q3, q3, q4, VECTOR_UNMASKED);

                if (!pass) {
                    vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 1);
                    VMV_V_V(q2, q3);
                }
            }

            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW16, 16 << vex.l);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 1);
            VMV_V_V(v0, q2);
            if (vex.l) {
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW16, 32);
                VSLIDEUP_VI(v0, q3, 8, VECTOR_UNMASKED);
            }
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x46:
            if (!vex.l) return 0;
            INST_NAME("VPERM2I128 Gx, Vx, Ex, Ib");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW8);
            GETEY_vector(q1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            u8 = F8;
            q2 = fpu_get_scratch(dyn);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            for (int i = 0; i < 2; ++i) {
                int control = i ? (u8 >> 4) : u8;
                int source = (control >> 1) & 1;
                int index = control & 1;
                int zero = control & 8;
                if (zero) {
                    VXOR_VV(q2, q2, q2, VECTOR_UNMASKED);
                } else if (source) {
                    if (index)
                        VSLIDEDOWN_VI(q2, q1, 16, VECTOR_UNMASKED);
                    else
                        VMV_V_V(q2, q1);
                } else {
                    if (index)
                        VSLIDEDOWN_VI(q2, q0, 16, VECTOR_UNMASKED);
                    else
                        VMV_V_V(q2, q0);
                }
                if (i) {
                    VSLIDEUP_VI(q1, q2, 16, VECTOR_UNMASKED);
                    q2 = q1;
                }
                VECTOR_LOAD_VMASK(i ? 0xffff0000 : 0xffff, x4, 2);
                VMERGE_VVM(v0, v0, q2);
            }
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        default:
            DEFAULT_VECTOR;
    }
    return addr;
}
