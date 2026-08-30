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

uintptr_t dynarec64_AVX_66_0F_vector(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback;
    int v0, v1, v2;
    int q0, q1, q2, d0, d1;
    uint64_t selectors;
    int64_t fixedaddress;
    rex_t rex = vex.rex;

    MAYUSE(v1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(q2);
    MAYUSE(d0);
    MAYUSE(d1);

    switch (opcode) {
        case 0x10:
            INST_NAME("VMOVUPD Gx, Ex");
            nextop = F8;
            GETEY_vector(v1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, v1);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x11:
            INST_NAME("VMOVUPD Ex, Gx");
            nextop = F8;
            GETGY_vector(v0, VECTOR_SEW8);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                avx_store_reg_vector(dyn, ninst, x1, v0, ed, 16 << vex.l, VECTOR_SEW8);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW8);
                VSE8_V(v0, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("VMOVLPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_vector(v0);
            avx_load_reg_vector(dyn, ninst, x1, v0, vex.v, 16, VECTOR_SEW64);
            if (MODREG) {
                return 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                LD(x2, ed, fixedaddress);
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                VMV_S_X(v0, x2);
            }
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW64);
            break;
        case 0x13:
            INST_NAME("VMOVLPD Ex, Gx");
            nextop = F8;
            if (MODREG) return 0;
            GETGY_vector(q0, VECTOR_SEW64);
            addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
            VMV_X_S(x4, q0);
            SD(x4, ed, fixedaddress);
            SMWRITE2();
            break;
        case 0x14:
            INST_NAME("VUNPCKLPD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            v2 = fpu_get_scratch(dyn);
            VID_V(q2, VECTOR_UNMASKED);
            VAND_VI(q2, q2, 2, VECTOR_UNMASKED);
            VRGATHER_VV(v1, q0, q2, VECTOR_UNMASKED);
            VRGATHER_VV(v2, q1, q2, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(0x0a, x4, vex.l ? 2 : 1);
            VMERGE_VVM(v0, v1, v2);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x15:
            INST_NAME("VUNPCKHPD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            v2 = fpu_get_scratch(dyn);
            VID_V(q2, VECTOR_UNMASKED);
            VAND_VI(q2, q2, 2, VECTOR_UNMASKED);
            VADD_VI(q2, q2, 1, VECTOR_UNMASKED);
            VRGATHER_VV(v1, q0, q2, VECTOR_UNMASKED);
            VRGATHER_VV(v2, q1, q2, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(0x0a, x4, vex.l ? 2 : 1);
            VMERGE_VVM(v0, v1, v2);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x16:
            INST_NAME("VMOVHPD Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW64);
            q1 = fpu_get_scratch(dyn);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                avx_load_reg_vector(dyn, ninst, x1, q1, ed, 16, VECTOR_SEW64);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                if (fixedaddress) { ADDI(x4, ed, fixedaddress); ed = x4; fixedaddress = 0; }
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 0.5);
                VLE64_V(q1, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            GETGY_empty_vector(v0);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW64, 16);
            VMV_V_V(v0, q0);
            VSLIDEUP_VI(v0, q1, 1, VECTOR_UNMASKED);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW64);
            break;
        case 0x17:
            INST_NAME("VMOVHPD Ex, Gx");
            nextop = F8;
            if (MODREG) return 0;
            GETGY_vector(q0, VECTOR_SEW64);
            q1 = fpu_get_scratch(dyn);
            VSLIDEDOWN_VI(q1, q0, 1, VECTOR_UNMASKED);
            VMV_X_S(x4, q1);
            addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
            SD(x4, ed, fixedaddress);
            SMWRITE2();
            break;
        case 0x28:
            INST_NAME("VMOVAPD Gx, Ex");
            nextop = F8;
            GETEY_vector(v1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, v1);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x29:
            INST_NAME("VMOVAPD Ex, Gx");
            nextop = F8;
            GETGY_vector(v0, VECTOR_SEW8);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                avx_store_reg_vector(dyn, ninst, x1, v0, ed, 16 << vex.l, VECTOR_SEW8);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW8);
                VSE8_V(v0, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
                SMWRITE2();
            }
            break;
        case 0x2B:
            INST_NAME("VMOVNTPD Ex, Gx");
            nextop = F8;
            if (MODREG) return 0;
            GETGY_vector(v0, VECTOR_SEW8);
            addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW8);
            VSE8_V(v0, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            SMWRITE2();
            break;
        case 0x2E:
            return 0;
        case 0x2F:
            return 0;
        case 0x50:
            INST_NAME("VMOVMSKPD Gd, Ex");
            nextop = F8;
            GETEY_vector(q0, 0, VECTOR_SEW64);
            GETGD;
            VMSLT_VX(VMASK, q0, xZR, VECTOR_UNMASKED);
            VMV_X_S(gd, VMASK);
            ANDI(gd, gd, vex.l ? 0x0f : 0x03);
            break;
        case 0x51:
            if (!BOX64ENV(dynarec_fastround) || !BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VSQRTPD Gx, Ex");
            nextop = F8;
            GETEY_vector(v1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VFSQRT_V(v0, v1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x54:
            INST_NAME("VANDPD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VAND_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x55:
            INST_NAME("VANDNPD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VXOR_VI(v0, q0, 0x1f, VECTOR_UNMASKED);
            VAND_VV(v0, v0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x56:
            INST_NAME("VORPD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VOR_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x57:
            INST_NAME("VXORPD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VXOR_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x58:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VADDPD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VFADD_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x59:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VMULPD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VFMUL_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x5A:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VCVTPD2PS Gx, Ex");
            nextop = F8;
            d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                avx_load_reg_vector(dyn, ninst, x1, d0, ed, 16 << vex.l, VECTOR_SEW64);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                if (fixedaddress) { ADDI(x4, ed, fixedaddress); ed = x4; fixedaddress = 0; }
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW64);
                VLE64_V(d0, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            v0 = fpu_get_scratch(dyn);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, vex.l ? 1 : 0.5);
            VFNCVT_F_F_W(v0, d0, VECTOR_UNMASKED);
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW32);
            break;
        case 0x5B:
            if (!BOX64ENV(dynarec_fastround)) return 0;
            INST_NAME("VCVTPS2DQ Gx, Ex");
            nextop = F8;
            GETEY_vector(q0, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VFCVT_X_F_V(v0, q0, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x5C:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VSUBPD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VFSUB_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x5D:
            INST_NAME("VMINPD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q0);
            VMFLT_VV(VMASK, q0, q1, VECTOR_UNMASKED);
            VXOR_VI(VMASK, VMASK, 0x1f, VECTOR_UNMASKED);
            VADD_VX(v0, q1, xZR, VECTOR_MASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x5E:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VDIVPD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VFDIV_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x5F:
            INST_NAME("VMAXPD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q0);
            VMFLT_VV(VMASK, q1, q0, VECTOR_UNMASKED);
            VXOR_VI(VMASK, VMASK, 0x1f, VECTOR_UNMASKED);
            VADD_VX(v0, q1, xZR, VECTOR_MASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x60:
            INST_NAME("VPUNPCKLBW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW8);
            GETEY_vector(q1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            v2 = fpu_get_scratch(dyn);
            VID_V(q2, VECTOR_UNMASKED);
            MOV32w(x4, 16);
            VAND_VX(v1, q2, x4, VECTOR_UNMASKED);
            VAND_VI(q2, q2, 15, VECTOR_UNMASKED);
            VSRL_VI(q2, q2, 1, VECTOR_UNMASKED);
            VADD_VV(q2, q2, v1, VECTOR_UNMASKED);
            VRGATHER_VV(v1, q0, q2, VECTOR_UNMASKED);
            VRGATHER_VV(v2, q1, q2, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(0xaaaaaaaa, x4, vex.l ? 2 : 1);
            VMERGE_VVM(v0, v1, v2);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x61:
            INST_NAME("VPUNPCKLWD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            v2 = fpu_get_scratch(dyn);
            VID_V(q2, VECTOR_UNMASKED);
            VAND_VI(v1, q2, 8, VECTOR_UNMASKED);
            VAND_VI(q2, q2, 7, VECTOR_UNMASKED);
            VSRL_VI(q2, q2, 1, VECTOR_UNMASKED);
            VADD_VV(q2, q2, v1, VECTOR_UNMASKED);
            VRGATHER_VV(v1, q0, q2, VECTOR_UNMASKED);
            VRGATHER_VV(v2, q1, q2, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(0xaaaa, x4, vex.l ? 2 : 1);
            VMERGE_VVM(v0, v1, v2);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x62:
            INST_NAME("VPUNPCKLDQ Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            v2 = fpu_get_scratch(dyn);
            VID_V(q2, VECTOR_UNMASKED);
            VAND_VI(v1, q2, 4, VECTOR_UNMASKED);
            VAND_VI(q2, q2, 3, VECTOR_UNMASKED);
            VSRL_VI(q2, q2, 1, VECTOR_UNMASKED);
            VADD_VV(q2, q2, v1, VECTOR_UNMASKED);
            VRGATHER_VV(v1, q0, q2, VECTOR_UNMASKED);
            VRGATHER_VV(v2, q1, q2, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(0xaa, x4, vex.l ? 2 : 1);
            VMERGE_VVM(v0, v1, v2);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x63:
            if (vex.l) return 0;
            INST_NAME("VPACKSSWB Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW16, 32);
            VMV_V_V(q2, q0);
            VSLIDEUP_VI(q2, q1, 8, VECTOR_UNMASKED);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 16);
            VNCLIP_WI(v0, q2, 0, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x68:
            INST_NAME("VPUNPCKHBW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW8);
            GETEY_vector(q1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            v2 = fpu_get_scratch(dyn);
            VID_V(q2, VECTOR_UNMASKED);
            MOV32w(x4, 16);
            VAND_VX(v1, q2, x4, VECTOR_UNMASKED);
            VAND_VI(q2, q2, 15, VECTOR_UNMASKED);
            VSRL_VI(q2, q2, 1, VECTOR_UNMASKED);
            VADD_VI(q2, q2, 8, VECTOR_UNMASKED);
            VADD_VV(q2, q2, v1, VECTOR_UNMASKED);
            VRGATHER_VV(v1, q0, q2, VECTOR_UNMASKED);
            VRGATHER_VV(v2, q1, q2, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(0xaaaaaaaa, x4, vex.l ? 2 : 1);
            VMERGE_VVM(v0, v1, v2);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x69:
            INST_NAME("VPUNPCKHWD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            v2 = fpu_get_scratch(dyn);
            VID_V(q2, VECTOR_UNMASKED);
            VAND_VI(v1, q2, 8, VECTOR_UNMASKED);
            VAND_VI(q2, q2, 7, VECTOR_UNMASKED);
            VSRL_VI(q2, q2, 1, VECTOR_UNMASKED);
            VADD_VI(q2, q2, 4, VECTOR_UNMASKED);
            VADD_VV(q2, q2, v1, VECTOR_UNMASKED);
            VRGATHER_VV(v1, q0, q2, VECTOR_UNMASKED);
            VRGATHER_VV(v2, q1, q2, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(0xaaaa, x4, vex.l ? 2 : 1);
            VMERGE_VVM(v0, v1, v2);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x6A:
            INST_NAME("VPUNPCKHDQ Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            v2 = fpu_get_scratch(dyn);
            VID_V(q2, VECTOR_UNMASKED);
            VAND_VI(v1, q2, 4, VECTOR_UNMASKED);
            VAND_VI(q2, q2, 3, VECTOR_UNMASKED);
            VSRL_VI(q2, q2, 1, VECTOR_UNMASKED);
            VADD_VI(q2, q2, 2, VECTOR_UNMASKED);
            VADD_VV(q2, q2, v1, VECTOR_UNMASKED);
            VRGATHER_VV(v1, q0, q2, VECTOR_UNMASKED);
            VRGATHER_VV(v2, q1, q2, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(0xaa, x4, vex.l ? 2 : 1);
            VMERGE_VVM(v0, v1, v2);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x6B:
            if (vex.l) return 0;
            INST_NAME("VPACKSSDW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW32, 32);
            VMV_V_V(q2, q0);
            VSLIDEUP_VI(q2, q1, 4, VECTOR_UNMASKED);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW16, 16);
            VNCLIP_WI(v0, q2, 0, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x67:
            if (vex.l) return 0;
            INST_NAME("VPACKUSWB Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW16, 32);
            VMAX_VX(q2, q0, xZR, VECTOR_UNMASKED);
            v1 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            VMAX_VX(v1, q1, xZR, VECTOR_UNMASKED);
            VSLIDEUP_VI(q2, v1, 8, VECTOR_UNMASKED);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 16);
            VNCLIPU_WI(v0, q2, 0, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x6C:
            if (vex.l) return 0;
            INST_NAME("VPUNPCKLQDQ Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q0);
            VSLIDEUP_VI(v0, q1, 1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x6D:
            if (vex.l) return 0;
            INST_NAME("VPUNPCKHQDQ Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            VSLIDEDOWN_VI(q2, q0, 1, VECTOR_UNMASKED);
            VSLIDEDOWN_VI(v1, q1, 1, VECTOR_UNMASKED);
            VSLIDEUP_VI(q2, v1, 1, VECTOR_UNMASKED);
            VMV_V_V(v0, q2);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x6E:
            if (vex.l) return 0;
            if (rex.w)
                INST_NAME("VMOVQ Gx, Eq");
            else
                INST_NAME("VMOVD Gx, Ed");
            nextop = F8;
            GETGY_empty_vector(v0);
            GETED(0);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW64, 16);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            if (!rex.w) {
                ZEXTW2(x4, ed);
                ed = x4;
            }
            VMV_S_X(v0, ed);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x70:
            INST_NAME("VPSHUFD Gx, Ex, Ib");
            nextop = F8;
            GETEY_vector(q1, 1, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            u8 = F8;
            q2 = fpu_get_scratch(dyn);
            {
                uint32_t sel_lo = (u8 & 3) | (((u8 >> 2) & 3) << 16);
                uint32_t sel_hi = ((u8 >> 4) & 3) | (((u8 >> 6) & 3) << 16);
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                MOV32w(x4, sel_hi);
                SLLI(x4, x4, 32);
                MOV32w(x5, sel_lo);
                OR(x4, x4, x5); // x4 = lane0 selector (64-bit,4 halfword indices)
                if (vex.l) {
                    int s = fpu_get_scratch(dyn);
                    MOV64x(x5, 0x0004000400040004LL);
                    ADD(x5, x4, x5);                          // x5 = lane1 selector
                    VMV_S_X(s, x5);                           // s[0] = lane1 selector
                    VSLIDE1UP_VX(q2, s, x4, VECTOR_UNMASKED); // q2[0]=x4, q2[1]=s[0]
                } else {
                    VMV_S_X(q2, x4);
                }
            }
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, vex.l ? 2 : 1);
            VRGATHEREI16_VV(v0, q1, q2, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x71:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("VPSRLW Gx, Ex, Ib");
                    GETEY_vector(q1, 1, VECTOR_SEW16);
                    GETGY_empty_vector(v0);
                    gd = vex.v;
                    u8 = F8;
                    if (u8 > 15)
                        VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                    else
                        VSRL_VI(v0, q1, u8, VECTOR_UNMASKED);
                    PUTGY_vector(v0, VECTOR_SEW16);
                    break;
                case 4:
                    INST_NAME("VPSRAW Gx, Ex, Ib");
                    GETEY_vector(q1, 1, VECTOR_SEW16);
                    GETGY_empty_vector(v0);
                    gd = vex.v;
                    u8 = F8;
                    if (u8 > 15) u8 = 15;
                    VSRA_VI(v0, q1, u8, VECTOR_UNMASKED);
                    PUTGY_vector(v0, VECTOR_SEW16);
                    break;
                case 6:
                    INST_NAME("VPSLLW Gx, Ex, Ib");
                    GETEY_vector(q1, 1, VECTOR_SEW16);
                    GETGY_empty_vector(v0);
                    gd = vex.v;
                    u8 = F8;
                    if (u8 > 15)
                        VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                    else
                        VSLL_VI(v0, q1, u8, VECTOR_UNMASKED);
                    PUTGY_vector(v0, VECTOR_SEW16);
                    break;
                default:
                    DEFAULT_VECTOR;
            }
            break;
        case 0x72:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("VPSRLD Gx, Ex, Ib");
                    GETEY_vector(q1, 1, VECTOR_SEW32);
                    GETGY_empty_vector(v0);
                    gd = vex.v;
                    u8 = F8;
                    if (u8 > 31)
                        VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                    else
                        VSRL_VI(v0, q1, u8, VECTOR_UNMASKED);
                    PUTGY_vector(v0, VECTOR_SEW32);
                    break;
                case 4:
                    INST_NAME("VPSRAD Gx, Ex, Ib");
                    GETEY_vector(q1, 1, VECTOR_SEW32);
                    GETGY_empty_vector(v0);
                    gd = vex.v;
                    u8 = F8;
                    if (u8 > 31) u8 = 31;
                    VSRA_VI(v0, q1, u8, VECTOR_UNMASKED);
                    PUTGY_vector(v0, VECTOR_SEW32);
                    break;
                case 6:
                    INST_NAME("VPSLLD Gx, Ex, Ib");
                    GETEY_vector(q1, 1, VECTOR_SEW32);
                    GETGY_empty_vector(v0);
                    gd = vex.v;
                    u8 = F8;
                    if (u8 > 31)
                        VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                    else
                        VSLL_VI(v0, q1, u8, VECTOR_UNMASKED);
                    PUTGY_vector(v0, VECTOR_SEW32);
                    break;
                default:
                    DEFAULT_VECTOR;
            }
            break;
        case 0x73:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("VPSRLQ Gx, Ex, Ib");
                    GETEY_vector(q1, 1, VECTOR_SEW64);
                    GETGY_empty_vector(v0);
                    gd = vex.v;
                    u8 = F8;
                    if (u8 > 63)
                        VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                    else if (u8 > 31) {
                        ADDI(x1, xZR, u8);
                        VSRL_VX(v0, q1, x1, VECTOR_UNMASKED);
                    } else
                        VSRL_VI(v0, q1, u8, VECTOR_UNMASKED);
                    PUTGY_vector(v0, VECTOR_SEW64);
                    break;
                case 3:
                    INST_NAME("VPSRLDQ Gx, Ex, Ib");
                    GETEY_vector(q1, 1, VECTOR_SEW8);
                    GETGY_empty_vector(v0);
                    gd = vex.v;
                    u8 = F8;
                    if (u8 > 15) {
                        VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                    } else if (u8 == 0) {
                        VMV_V_V(v0, q1);
                    } else {
                        q2 = fpu_get_scratch(dyn);
                        v1 = fpu_get_scratch(dyn);
                        VID_V(q2, VECTOR_UNMASKED);
                        VAND_VI(v1, q2, 15, VECTOR_UNMASKED);
                        VADD_VI(q2, q2, u8, VECTOR_UNMASKED);
                        ADDI(x4, xZR, 16 - u8);
                        VMSLTU_VX(VMASK, v1, x4, VECTOR_UNMASKED);
                        VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                        VRGATHER_VV(v0, q1, q2, VECTOR_MASKED);
                    }
                    PUTGY_vector(v0, VECTOR_SEW8);
                    break;
                case 7:
                    INST_NAME("VPSLLDQ Gx, Ex, Ib");
                    GETEY_vector(q1, 1, VECTOR_SEW8);
                    GETGY_empty_vector(v0);
                    gd = vex.v;
                    u8 = F8;
                    if (u8 > 15) {
                        VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                    } else if (u8 == 0) {
                        VMV_V_V(v0, q1);
                    } else {
                        q2 = fpu_get_scratch(dyn);
                        v1 = fpu_get_scratch(dyn);
                        VID_V(q2, VECTOR_UNMASKED);
                        VAND_VI(v1, q2, 15, VECTOR_UNMASKED);
                        ADDI(x4, xZR, u8);
                        VSUB_VX(q2, q2, x4, VECTOR_UNMASKED);
                        ADDI(x4, xZR, u8 - 1);
                        VMSGTU_VX(VMASK, v1, x4, VECTOR_UNMASKED);
                        VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                        VRGATHER_VV(v0, q1, q2, VECTOR_MASKED);
                    }
                    PUTGY_vector(v0, VECTOR_SEW8);
                    break;
                case 6:
                    INST_NAME("VPSLLQ Gx, Ex, Ib");
                    GETEY_vector(q1, 1, VECTOR_SEW64);
                    GETGY_empty_vector(v0);
                    gd = vex.v;
                    u8 = F8;
                    if (u8 > 63)
                        VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                    else if (u8 > 31) {
                        ADDI(x1, xZR, u8);
                        VSLL_VX(v0, q1, x1, VECTOR_UNMASKED);
                    } else
                        VSLL_VI(v0, q1, u8, VECTOR_UNMASKED);
                    PUTGY_vector(v0, VECTOR_SEW64);
                    break;
                default:
                    DEFAULT_VECTOR;
            }
            break;
        case 0x64:
            INST_NAME("VPCMPGTB Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW8);
            GETEY_vector(q1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VMSLT_VV(VMASK, q1, q0, VECTOR_UNMASKED);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VMERGE_VIM(v0, v0, 0x1f);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x65:
            INST_NAME("VPCMPGTW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            VMSLT_VV(VMASK, q1, q0, VECTOR_UNMASKED);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VMERGE_VIM(v0, v0, 0x1f);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x66:
            INST_NAME("VPCMPGTD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VMSLT_VV(VMASK, q1, q0, VECTOR_UNMASKED);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VMERGE_VIM(v0, v0, 0x1f);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x6F:
            INST_NAME("VMOVDQA Gx, Ex");
            nextop = F8;
            GETEY_vector(v1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, v1);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x74:
            INST_NAME("VPCMPEQB Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW8);
            GETEY_vector(q1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VMSEQ_VV(VMASK, q0, q1, VECTOR_UNMASKED);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VMERGE_VIM(v0, v0, 0x1f);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x75:
            INST_NAME("VPCMPEQW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            VMSEQ_VV(VMASK, q0, q1, VECTOR_UNMASKED);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VMERGE_VIM(v0, v0, 0x1f);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x76:
            INST_NAME("VPCMPEQD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VMSEQ_VV(VMASK, q0, q1, VECTOR_UNMASKED);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VMERGE_VIM(v0, v0, 0x1f);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x7F:
            INST_NAME("VMOVDQA Ex, Gx");
            nextop = F8;
            GETGY_vector(v0, VECTOR_SEW8);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                avx_store_reg_vector(dyn, ninst, x1, v0, ed, 16 << vex.l, VECTOR_SEW8);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW8);
                VSE8_V(v0, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
                SMWRITE2();
            }
            break;
        case 0x7E:
            return 0;
        case 0xD0:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VADDSUBPD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch(dyn);
            VID_V(q2, VECTOR_UNMASKED);
            VAND_VI(q2, q2, 1, VECTOR_UNMASKED);
            VMSEQ_VX(VMASK, q2, xZR, VECTOR_UNMASKED);
            VFADD_VV(v0, q0, q1, VECTOR_MASKED);
            VXOR_VI(VMASK, VMASK, 0x1f, VECTOR_UNMASKED);
            VFSUB_VV(v0, q0, q1, VECTOR_MASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0xD1:
            INST_NAME("VPSRLW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VMV_X_S(x4, q1);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW16);
            SLTIU(x5, x4, 16);
            SUB(x5, xZR, x5);
            VSRL_VX(v0, q0, x4, VECTOR_UNMASKED);
            VAND_VX(v0, v0, x5, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xD2:
            INST_NAME("VPSRLD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VMV_X_S(x4, q1);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            SLTIU(x5, x4, 32);
            SUB(x5, xZR, x5);
            VSRL_VX(v0, q0, x4, VECTOR_UNMASKED);
            VAND_VX(v0, v0, x5, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0xD3:
            INST_NAME("VPSRLQ Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VMV_X_S(x4, q1);
            SLTIU(x5, x4, 64);
            SUB(x5, xZR, x5);
            VSRL_VX(v0, q0, x4, VECTOR_UNMASKED);
            VAND_VX(v0, v0, x5, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0xD6:
            if (vex.l) return 0;
            INST_NAME("VMOVQ Ex, Gx");
            nextop = F8;
            GETGY_vector(q0, VECTOR_SEW64);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                q1 = fpu_get_scratch(dyn);
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW64, 2);
                VXOR_VV(q1, q1, q1, VECTOR_UNMASKED);
                VECTOR_LOAD_VMASK(1, x4, 1);
                VMERGE_VVM(q1, q1, q0);
                avx_store_reg_vector(dyn, ninst, x1, q1, ed, 16, VECTOR_SEW64);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                VMV_X_S(x4, q0);
                SD(x4, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0xD4:
            INST_NAME("VPADDQ Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VADD_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0xD5:
            INST_NAME("VPMULLW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            VMUL_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xD8:
            INST_NAME("VPSUBUSB Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW8);
            GETEY_vector(q1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch(dyn);
            VMSLTU_VV(VMASK, q0, q1, VECTOR_UNMASKED);
            VSUB_VV(v0, q0, q1, VECTOR_UNMASKED);
            VXOR_VV(q2, q2, q2, VECTOR_UNMASKED);
            VMERGE_VVM(v0, v0, q2);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0xD9:
            INST_NAME("VPSUBUSW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch(dyn);
            VMSLTU_VV(VMASK, q0, q1, VECTOR_UNMASKED);
            VSUB_VV(v0, q0, q1, VECTOR_UNMASKED);
            VXOR_VV(q2, q2, q2, VECTOR_UNMASKED);
            VMERGE_VVM(v0, v0, q2);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xDA:
            INST_NAME("VPMINUB Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW8);
            GETEY_vector(q1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VMINU_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0xDB:
            INST_NAME("VPAND Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VAND_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0xDC:
            INST_NAME("VPADDUSB Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW8);
            GETEY_vector(q1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VSADDU_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0xDD:
            INST_NAME("VPADDUSW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            VSADDU_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xDE:
            INST_NAME("VPMAXUB Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW8);
            GETEY_vector(q1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VMAXU_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0xDF:
            INST_NAME("VPANDN Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VXOR_VI(v0, q0, 0x1f, VECTOR_UNMASKED);
            VAND_VV(v0, v0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0xE0:
            INST_NAME("VPAVGB Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW8);
            GETEY_vector(q1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            CSRRWI(xZR, 0b00, 0x00A);
            VAADDU_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0xE3:
            INST_NAME("VPAVGW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            CSRRWI(xZR, 0b00, 0x00A);
            VAADDU_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xE4:
            INST_NAME("VPMULHUW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            VMULHU_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xE5:
            INST_NAME("VPMULHW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            VMULH_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xE7:
            INST_NAME("VMOVNTDQ Ex, Gx");
            nextop = F8;
            if (MODREG) return 0;
            GETGY_vector(v0, VECTOR_SEW8);
            addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW8);
            VSE8_V(v0, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            SMWRITE2();
            break;
        case 0xE8:
            INST_NAME("VPSUBSB Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW8);
            GETEY_vector(q1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VSSUB_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0xE9:
            INST_NAME("VPSUBSW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            VSSUB_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xEA:
            INST_NAME("VPMINSW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            VMIN_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xEB:
            INST_NAME("VPOR Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VOR_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0xEC:
            INST_NAME("VPADDSB Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW8);
            GETEY_vector(q1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VSADD_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0xED:
            INST_NAME("VPADDSW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            VSADD_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xEE:
            INST_NAME("VPMAXSW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            VMAX_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xEF:
            INST_NAME("VPXOR Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VXOR_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0xF1:
            INST_NAME("VPSLLW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VMV_X_S(x4, q1);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW16);
            SLTIU(x5, x4, 16);
            SUB(x5, xZR, x5);
            VSLL_VX(v0, q0, x4, VECTOR_UNMASKED);
            VAND_VX(v0, v0, x5, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xF2:
            INST_NAME("VPSLLD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VMV_X_S(x4, q1);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            SLTIU(x5, x4, 32);
            SUB(x5, xZR, x5);
            VSLL_VX(v0, q0, x4, VECTOR_UNMASKED);
            VAND_VX(v0, v0, x5, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0xF3:
            INST_NAME("VPSLLQ Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VMV_X_S(x4, q1);
            SLTIU(x5, x4, 64);
            SUB(x5, xZR, x5);
            VSLL_VX(v0, q0, x4, VECTOR_UNMASKED);
            VAND_VX(v0, v0, x5, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0xF4:
            INST_NAME("VPMULUDQ Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            ADDI(x2, xZR, 32);
            VSLL_VX(q0, q0, x2, VECTOR_UNMASKED);
            VSRL_VX(q0, q0, x2, VECTOR_UNMASKED);
            VSLL_VX(q1, q1, x2, VECTOR_UNMASKED);
            VSRL_VX(q1, q1, x2, VECTOR_UNMASKED);
            VMUL_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0xF5:
            INST_NAME("VPMADDWD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW16, 32);
            VWMUL_VV(q2, q0, q1, VECTOR_UNMASKED);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW32, 16 << vex.l);
            MOV32w(x4, 32);
            VNSRL_WX(d0, q2, xZR, VECTOR_UNMASKED);
            VNSRL_WX(d1, q2, x4, VECTOR_UNMASKED);
            VADD_VV(v0, d0, d1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0xF8:
            INST_NAME("VPSUBB Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW8);
            GETEY_vector(q1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VSUB_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0xF9:
            INST_NAME("VPSUBW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            VSUB_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xFA:
            INST_NAME("VPSUBD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VSUB_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0xFB:
            INST_NAME("VPSUBQ Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VSUB_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0xFC:
            INST_NAME("VPADDB Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW8);
            GETEY_vector(q1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VADD_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0xFD:
            INST_NAME("VPADDW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            VADD_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xFE:
            INST_NAME("VPADDD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VADD_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        default:
            DEFAULT_VECTOR;
    }
    return addr;
}
