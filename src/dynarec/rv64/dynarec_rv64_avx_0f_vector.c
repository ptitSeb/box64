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

uintptr_t dynarec64_AVX_0F_vector(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    int v0, v1, v2;
    int q0, q1, q2, d0;
    uint64_t selectors;
    int64_t fixedaddress;
    rex_t rex = vex.rex;

    MAYUSE(v2);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(q2);
    MAYUSE(d0);

    switch (opcode) {
        case 0x10:
            INST_NAME("VMOVUPS Gx, Ex");
            nextop = F8;
            GETEY_vector(v1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, v1);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x11:
            INST_NAME("VMOVUPS Ex, Gx");
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
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW64);
            q1 = fpu_get_scratch(dyn);
            if (MODREG) {
                INST_NAME("VMOVHLPS Gx, Vx, Ex");
                ed = (nextop & 7) + (rex.b << 3);
                avx_load_reg_vector(dyn, ninst, x1, q1, ed, 16, VECTOR_SEW64);
                v1 = fpu_get_scratch(dyn);
                VSLIDEDOWN_VI(v1, q1, 1, VECTOR_UNMASKED);
                VMV_V_V(q1, v1);
            } else {
                INST_NAME("VMOVLPS Gx, Vx, Ex");
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                if (fixedaddress) { ADDI(x4, ed, fixedaddress); ed = x4; fixedaddress = 0; }
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 0.5);
                VLE64_V(q1, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch(dyn);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW64, 16);
            VSLIDEDOWN_VI(q2, q0, 1, VECTOR_UNMASKED);
            VMV_V_V(v0, q1);
            VSLIDEUP_VI(v0, q2, 1, VECTOR_UNMASKED);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW64);
            break;
        case 0x13:
            INST_NAME("VMOVLPS Ex, Gx");
            nextop = F8;
            if (MODREG) return 0;
            GETGY_vector(q0, VECTOR_SEW64);
            addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
            VMV_X_S(x4, q0);
            SD(x4, ed, fixedaddress);
            SMWRITE2();
            break;
        case 0x14:
            INST_NAME("VUNPCKLPS Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            v2 = fpu_get_scratch(dyn);
            VID_V(q2, VECTOR_UNMASKED);
            VAND_VI(v1, q2, 4, VECTOR_UNMASKED);
            VSRL_VI(v1, v1, 1, VECTOR_UNMASKED);
            VSRL_VI(q2, q2, 1, VECTOR_UNMASKED);
            VADD_VV(q2, q2, v1, VECTOR_UNMASKED);
            VRGATHER_VV(v1, q0, q2, VECTOR_UNMASKED);
            VRGATHER_VV(v2, q1, q2, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(0xaa, x4, vex.l ? 2 : 1);
            VMERGE_VVM(v0, v1, v2);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x15:
            INST_NAME("VUNPCKHPS Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            v2 = fpu_get_scratch(dyn);
            VID_V(q2, VECTOR_UNMASKED);
            VAND_VI(v1, q2, 4, VECTOR_UNMASKED);
            VSRL_VI(v1, v1, 1, VECTOR_UNMASKED);
            VSRL_VI(q2, q2, 1, VECTOR_UNMASKED);
            VADD_VV(q2, q2, v1, VECTOR_UNMASKED);
            VADD_VI(q2, q2, 2, VECTOR_UNMASKED);
            VRGATHER_VV(v1, q0, q2, VECTOR_UNMASKED);
            VRGATHER_VV(v2, q1, q2, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(0xaa, x4, vex.l ? 2 : 1);
            VMERGE_VVM(v0, v1, v2);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x16:
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW64);
            q1 = fpu_get_scratch(dyn);
            if (MODREG) {
                INST_NAME("VMOVLHPS Gx, Vx, Ex");
                ed = (nextop & 7) + (rex.b << 3);
                avx_load_reg_vector(dyn, ninst, x1, q1, ed, 16, VECTOR_SEW64);
            } else {
                INST_NAME("VMOVHPS Gx, Vx, Ex");
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
            INST_NAME("VMOVHPS Ex, Gx");
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
            INST_NAME("VMOVAPS Gx, Ex");
            nextop = F8;
            GETEY_vector(v1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, v1);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x29:
            INST_NAME("VMOVAPS Ex, Gx");
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
            INST_NAME("VMOVNTPS Ex, Gx");
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
            INST_NAME("VMOVMSKPS Gd, Ex");
            nextop = F8;
            GETEY_vector(q0, 0, VECTOR_SEW32);
            GETGD;
            VMSLT_VX(VMASK, q0, xZR, VECTOR_UNMASKED);
            VMV_X_S(gd, VMASK);
            ANDI(gd, gd, vex.l ? 0xff : 0x0f);
            break;
        case 0x51:
            if (!BOX64ENV(dynarec_fastround) || !BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VSQRTPS Gx, Ex");
            nextop = F8;
            GETEY_vector(v1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VFSQRT_V(v0, v1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x52:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VRSQRTPS Gx, Ex");
            nextop = F8;
            GETEY_vector(v1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            v2 = fpu_get_scratch(dyn);
            LUI(x4, 0x3f800);
            VMV_V_X(v2, x4);
            VFSQRT_V(v0, v1, VECTOR_UNMASKED);
            VFDIV_VV(v0, v2, v0, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x53:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VRCPPS Gx, Ex");
            nextop = F8;
            GETEY_vector(v1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            v2 = fpu_get_scratch(dyn);
            LUI(x4, 0x3f800);
            VMV_V_X(v2, x4);
            VFDIV_VV(v0, v2, v1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x54:
            INST_NAME("VANDPS Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VAND_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x55:
            INST_NAME("VANDNPS Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VXOR_VI(v0, q0, 0x1f, VECTOR_UNMASKED);
            VAND_VV(v0, v0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x56:
            INST_NAME("VORPS Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VOR_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x57:
            INST_NAME("VXORPS Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VXOR_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x58:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VADDPS Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VFADD_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x59:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VMULPS Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VFMUL_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x5A:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VCVTPS2PD Gx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                avx_load_reg_vector(dyn, ninst, x1, q0, ed, 16, VECTOR_SEW32);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                if (fixedaddress) { ADDI(x4, ed, fixedaddress); ed = x4; fixedaddress = 0; }
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, vex.l ? 1 : 0.5);
                VLE32_V(q0, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, vex.l ? 1 : 0.5);
            d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            VFWCVT_F_F_V(d0, q0, VECTOR_UNMASKED);
            GETGY_empty_vector(v0);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW64, 16 << vex.l);
            VMV_V_V(v0, d0);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x5B:
            INST_NAME("VCVTDQ2PS Gx, Ex");
            nextop = F8;
            GETEY_vector(q0, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VFCVT_F_X_V(v0, q0, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x5C:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VSUBPS Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VFSUB_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x5D:
            INST_NAME("VMINPS Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q0);
            VMFLT_VV(VMASK, q0, q1, VECTOR_UNMASKED);
            VXOR_VI(VMASK, VMASK, 0x1f, VECTOR_UNMASKED);
            VADD_VX(v0, q1, xZR, VECTOR_MASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x5E:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VDIVPS Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VFDIV_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x5F:
            INST_NAME("VMAXPS Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q0);
            VMFLT_VV(VMASK, q1, q0, VECTOR_UNMASKED);
            VXOR_VI(VMASK, VMASK, 0x1f, VECTOR_UNMASKED);
            VADD_VX(v0, q1, xZR, VECTOR_MASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x77:
            if (vex.v) {
                UDF();
                break;
            }
            fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
            if (vex.l) {
                INST_NAME("VZEROALL");
                for (int i = 0; i < (rex.is32bits ? 8 : 16); ++i) {
                    SD(xZR, xEmu, offsetof(x64emu_t, xmm[i]) + 0);
                    SD(xZR, xEmu, offsetof(x64emu_t, xmm[i]) + 8);
                    SD(xZR, xEmu, offsetof(x64emu_t, ymm[i]) + 0);
                    SD(xZR, xEmu, offsetof(x64emu_t, ymm[i]) + 8);
                }
            } else {
                INST_NAME("VZEROUPPER");
                for (int i = 0; i < (rex.is32bits ? 8 : 16); ++i) {
                    SD(xZR, xEmu, offsetof(x64emu_t, ymm[i]) + 0);
                    SD(xZR, xEmu, offsetof(x64emu_t, ymm[i]) + 8);
                }
            }
            break;
        case 0xAE:
            return 0;
        case 0xC2:
            INST_NAME("VCMPPS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 1, VECTOR_SEW32);
            u8 = F8;
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch(dyn);
            d0 = fpu_get_scratch(dyn);
            VMFEQ_VV(q2, q0, q0, VECTOR_UNMASKED);
            VMFEQ_VV(d0, q1, q1, VECTOR_UNMASKED);
            VMAND_MM(q2, q2, d0);
            switch (u8 & 0xf) {
                case 0x0: VMFEQ_VV(VMASK, q0, q1, VECTOR_UNMASKED); break;
                case 0x1: VMFLT_VV(VMASK, q0, q1, VECTOR_UNMASKED); break;
                case 0x2: VMFLE_VV(VMASK, q0, q1, VECTOR_UNMASKED); break;
                case 0x3: VMNAND_MM(VMASK, q2, q2); break;
                case 0x4:
                    VMFEQ_VV(d0, q0, q1, VECTOR_UNMASKED);
                    VMNAND_MM(VMASK, d0, d0);
                    break;
                case 0x5:
                    VMFLT_VV(d0, q0, q1, VECTOR_UNMASKED);
                    VMNAND_MM(VMASK, d0, d0);
                    break;
                case 0x6:
                    VMFLE_VV(d0, q0, q1, VECTOR_UNMASKED);
                    VMNAND_MM(VMASK, d0, d0);
                    break;
                case 0x7: VMAND_MM(VMASK, q2, q2); break;
                case 0x8:
                    VMFEQ_VV(d0, q0, q1, VECTOR_UNMASKED);
                    VMNAND_MM(VMASK, q2, q2);
                    VMOR_MM(VMASK, VMASK, d0);
                    break;
                case 0x9:
                    VMFLE_VV(d0, q1, q0, VECTOR_UNMASKED);
                    VMNAND_MM(VMASK, d0, d0);
                    break;
                case 0xa:
                    VMFLT_VV(d0, q1, q0, VECTOR_UNMASKED);
                    VMNAND_MM(VMASK, d0, d0);
                    break;
                case 0xb: VMXOR_MM(VMASK, q2, q2); break;
                case 0xc:
                    VMFEQ_VV(d0, q0, q1, VECTOR_UNMASKED);
                    VMNAND_MM(VMASK, d0, d0);
                    VMAND_MM(VMASK, VMASK, q2);
                    break;
                case 0xd: VMFLE_VV(VMASK, q1, q0, VECTOR_UNMASKED); break;
                case 0xe: VMFLT_VV(VMASK, q1, q0, VECTOR_UNMASKED); break;
                case 0xf: VMXNOR_MM(VMASK, q2, q2); break;
            }
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VMERGE_VIM(v0, v0, 0x1f);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0xC6:
            INST_NAME("VSHUFPS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 1, VECTOR_SEW32);
            u8 = F8;
            selectors = 0;
            for (int i = 0; i < (4 << vex.l); ++i) {
                int lane = i & ~3;
                int pos = i & 3;
                int selector = lane + ((u8 >> (pos * 2)) & 3);
                if (pos >= 2) selector |= 0x80;
                selectors |= (uint64_t)selector << (i * 8);
            }
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch(dyn);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            for (int i = 0; i < (4 << vex.l); ++i) {
                int selector = selectors >> (i * 8);
                MOV32w(x4, selector & 0x1f);
                VRGATHER_VX(q2, (selector & 0x80) ? q1 : q0, x4, VECTOR_UNMASKED);
                VECTOR_LOAD_VMASK(1 << i, x4, vex.l ? 2 : 1);
                VMERGE_VVM(v0, v0, q2);
            }
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        default:
            DEFAULT_VECTOR;
    }
    return addr;
}
