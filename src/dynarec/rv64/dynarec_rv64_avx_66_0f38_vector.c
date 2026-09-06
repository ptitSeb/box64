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

uintptr_t dynarec64_AVX_66_0F38_vector(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed;
    int v0, v1, v2;
    int q0, q1, q2, q3, q4, q5;
    int sew;
    int64_t j64;
    int64_t fixedaddress;
    rex_t rex = vex.rex;

    MAYUSE(v1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(q2);

    switch (opcode) {
        case 0x00:
            INST_NAME("VPSHUFB Gx, Vx, Ex");
            fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW8);
            GETEY_vector(q1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            v1 = fpu_get_scratch(dyn);
            v2 = fpu_get_scratch(dyn);
            q2 = fpu_get_scratch(dyn);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW8);
            MOV32w(x4, 16);
            MOV32w(x5, 0x8f);
            VMV_V_V(v1, q1);
            VAND_VX(v1, v1, x5, VECTOR_UNMASKED);
            VOR_VX(v1, v1, x4, VECTOR_UNMASKED);
            VMV_V_V(v2, q1);
            VAND_VX(v2, v2, x5, VECTOR_UNMASKED);
            VSLIDEDOWN_VI(q2, v1, 16, VECTOR_UNMASKED);
            VSLIDEUP_VI(v2, q2, 16, VECTOR_UNMASKED);
            VRGATHER_VV(v0, q0, v2, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x08:
            INST_NAME("VPSIGNB Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW8);
            GETEY_vector(q1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q0);
            VMSLT_VX(VMASK, q1, xZR, VECTOR_UNMASKED);
            VRSUB_VX(v0, q0, xZR, VECTOR_MASKED);
            VMSEQ_VX(VMASK, q1, xZR, VECTOR_UNMASKED);
            VXOR_VV(v0, v0, v0, VECTOR_MASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x09:
            INST_NAME("VPSIGNW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q0);
            VMSLT_VX(VMASK, q1, xZR, VECTOR_UNMASKED);
            VRSUB_VX(v0, q0, xZR, VECTOR_MASKED);
            VMSEQ_VX(VMASK, q1, xZR, VECTOR_UNMASKED);
            VXOR_VV(v0, v0, v0, VECTOR_MASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x0A:
            INST_NAME("VPSIGND Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q0);
            VMSLT_VX(VMASK, q1, xZR, VECTOR_UNMASKED);
            VRSUB_VX(v0, q0, xZR, VECTOR_MASKED);
            VMSEQ_VX(VMASK, q1, xZR, VECTOR_UNMASKED);
            VXOR_VV(v0, v0, v0, VECTOR_MASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x0B:
            INST_NAME("VPMULHRSW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            VWMUL_VV(q2, q0, q1, VECTOR_UNMASKED);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL2, 2 << vex.l);
            VSRL_VI(q2, q2, 14, VECTOR_UNMASKED);
            VADD_VI(q2, q2, 1, VECTOR_UNMASKED);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW16, 16 << vex.l);
            VNSRL_WI(v0, q2, 1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x0C:
            INST_NAME("VPERMILPS Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch(dyn);
            VID_V(q2, VECTOR_UNMASKED);
            VAND_VI(q2, q2, 4, VECTOR_UNMASKED);
            VAND_VI(q1, q1, 3, VECTOR_UNMASKED);
            VADD_VV(q1, q1, q2, VECTOR_UNMASKED);
            VRGATHER_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x0D:
            INST_NAME("VPERMILPD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch(dyn);
            VID_V(q2, VECTOR_UNMASKED);
            VAND_VI(q2, q2, 2, VECTOR_UNMASKED);
            VSRL_VI(q1, q1, 1, VECTOR_UNMASKED);
            VAND_VI(q1, q1, 1, VECTOR_UNMASKED);
            VADD_VV(q1, q1, q2, VECTOR_UNMASKED);
            VRGATHER_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x0E:
        case 0x0F:
            if (opcode == 0x0E) {
                INST_NAME("VTESTPS Gx, Ex");
                sew = VECTOR_SEW32;
            } else {
                INST_NAME("VTESTPD Gx, Ex");
                sew = VECTOR_SEW64;
            }
            nextop = F8;
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            GETGY_vector(q0, sew);
            GETEY_vector(q1, 0, sew);
            v0 = fpu_get_scratch(dyn);
            CLEAR_FLAGS();
            IFX (X_ZF) {
                VAND_VV(v0, q0, q1, VECTOR_UNMASKED);
                VMSLT_VX(VMASK, v0, xZR, VECTOR_UNMASKED);
                VMV_X_S(x4, VMASK);
                ANDI(x4, x4, (1 << ((sew == VECTOR_SEW32 ? 4 : 2) << vex.l)) - 1);
                SET_FLAGS_EQZ(x4, F_ZF, x5);
            }
            IFX (X_CF) {
                VXOR_VI(v0, q0, 0x1f, VECTOR_UNMASKED);
                VAND_VV(v0, v0, q1, VECTOR_UNMASKED);
                VMSLT_VX(VMASK, v0, xZR, VECTOR_UNMASKED);
                VMV_X_S(x4, VMASK);
                ANDI(x4, x4, (1 << ((sew == VECTOR_SEW32 ? 4 : 2) << vex.l)) - 1);
                SET_FLAGS_EQZ(x4, F_CF, x5);
            }
            break;
        case 0x18:
            INST_NAME("VBROADCASTSS Gx, Ex");
            nextop = F8;
            GETGY_empty_vector(v0);
            if (MODREG) {
                q0 = fpu_get_scratch(dyn);
                ed = (nextop & 7) + (rex.b << 3);
                avx_load_reg_vector(dyn, ninst, x1, q0, ed, 16, VECTOR_SEW32);
                VMV_X_S(x4, q0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                LW(x4, ed, fixedaddress);
            }
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            VMV_V_X(v0, x4);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x19:
            INST_NAME("VBROADCASTSD Gx, Ex");
            nextop = F8;
            GETGY_empty_vector(v0);
            if (MODREG) {
                q0 = fpu_get_scratch(dyn);
                ed = (nextop & 7) + (rex.b << 3);
                avx_load_reg_vector(dyn, ninst, x1, q0, ed, 16, VECTOR_SEW64);
                VMV_X_S(x4, q0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, ed, fixedaddress);
            }
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW64);
            VMV_V_X(v0, x4);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x1A:
            if (!vex.l) return 0;
            INST_NAME("VBROADCASTF128 Gx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                avx_load_reg_vector(dyn, ninst, x1, q0, ed, 16, VECTOR_SEW8);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 16);
                VLE8_V(q0, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q0);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 32);
            VSLIDEUP_VI(v0, q0, 16, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x1C:
            INST_NAME("VPABSB Gx, Ex");
            nextop = F8;
            GETEY_vector(q0, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            v1 = fpu_get_scratch(dyn);
            VRSUB_VX(v1, q0, xZR, VECTOR_UNMASKED);
            VMAX_VV(v0, q0, v1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x1D:
            INST_NAME("VPABSW Gx, Ex");
            nextop = F8;
            GETEY_vector(q0, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            v1 = fpu_get_scratch(dyn);
            VRSUB_VX(v1, q0, xZR, VECTOR_UNMASKED);
            VMAX_VV(v0, q0, v1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x1E:
            INST_NAME("VPABSD Gx, Ex");
            nextop = F8;
            GETEY_vector(q0, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            v1 = fpu_get_scratch(dyn);
            VRSUB_VX(v1, q0, xZR, VECTOR_UNMASKED);
            VMAX_VV(v0, q0, v1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x20:
            INST_NAME("VPMOVSXBW Gx, Ex");
            nextop = F8;
            GETEY_vector(q0, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW16, 16 << vex.l);
            VSEXT_VF2(v0, q0, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x21:
            INST_NAME("VPMOVSXBD Gx, Ex");
            nextop = F8;
            GETEY_vector(q0, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW32, 16 << vex.l);
            VSEXT_VF4(v0, q0, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x22:
            INST_NAME("VPMOVSXBQ Gx, Ex");
            nextop = F8;
            GETEY_vector(q0, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW64, 16 << vex.l);
            VSEXT_VF8(v0, q0, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x23:
            INST_NAME("VPMOVSXWD Gx, Ex");
            nextop = F8;
            GETEY_vector(q0, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW32, 16 << vex.l);
            VSEXT_VF2(v0, q0, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x24:
            INST_NAME("VPMOVSXWQ Gx, Ex");
            nextop = F8;
            GETEY_vector(q0, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW64, 16 << vex.l);
            VSEXT_VF4(v0, q0, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x25:
            INST_NAME("VPMOVSXDQ Gx, Ex");
            nextop = F8;
            GETEY_vector(q0, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW64, 16 << vex.l);
            VSEXT_VF2(v0, q0, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x28:
            INST_NAME("VPMULDQ Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            v1 = fpu_get_scratch(dyn);
            v2 = fpu_get_scratch(dyn);
            VID_V(v2, VECTOR_UNMASKED);
            VSLL_VI(v2, v2, 1, VECTOR_UNMASKED);
            VRGATHER_VV(v1, q0, v2, VECTOR_UNMASKED);
            VRGATHER_VV(v0, q1, v2, VECTOR_UNMASKED);
            q2 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            VWMUL_VV(q2, v1, v0, VECTOR_UNMASKED);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW64, 16 << vex.l);
            VMV_V_V(v0, q2);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x29:
            INST_NAME("VPCMPEQQ Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VMSEQ_VV(VMASK, q0, q1, VECTOR_UNMASKED);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VMERGE_VIM(v0, v0, 0x1f);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x2A:
            INST_NAME("VMOVNTDQA Gx, Ex");
            nextop = F8;
            if (MODREG) return 0;
            GETGY_empty_vector(v0);
            SMREAD();
            addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW8);
            VLE8_V(v0, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x2B:
            INST_NAME("VPACKUSDW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW32, 16 << vex.l);
            VMAX_VX(q2, q0, xZR, VECTOR_UNMASKED);
            v1 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            VMAX_VX(v1, q1, xZR, VECTOR_UNMASKED);
            if (vex.l) {
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 1);
                VSLIDEDOWN_VI(q2 + 1, q2, 4, VECTOR_UNMASKED); // hi[0..3] = a[4..7]
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW32, 32);
                VSLIDEUP_VI(q2, v1, 4, VECTOR_UNMASKED); // lo[4..7] = b[0..3]
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 1);
                VSLIDEDOWN_VI(v1 + 1, v1, 4, VECTOR_UNMASKED); // v1 hi[0..3] = b[4..7]
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW32, 32);
                VSLIDEUP_VI(q2 + 1, v1 + 1, 4, VECTOR_UNMASKED); // hi[4..7] = b[4..7]
                if (cpuext.vlen > 32) {
                    vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 4);
                    VSLIDEUP_VI(q2, q2 + 1, 8, VECTOR_UNMASKED);
                }
            } else {
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 2);
                VSLIDEUP_VI(q2, v1, 4, VECTOR_UNMASKED);
            }
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW16, 16 << vex.l);
            VNCLIPU_WI(v0, q2, 0, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x2C:
            INST_NAME("VMASKMOVPS Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VMSLT_VX(VMASK, q0, xZR, VECTOR_UNMASKED);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, ed, 16 << vex.l, VECTOR_SEW32);
                VMERGE_VVM(v0, v0, q1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
                VLE32_V(v0, ed, VECTOR_MASKED, VECTOR_NFIELD1);
            }
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x2D:
            INST_NAME("VMASKMOVPD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VMSLT_VX(VMASK, q0, xZR, VECTOR_UNMASKED);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, ed, 16 << vex.l, VECTOR_SEW64);
                VMERGE_VVM(v0, v0, q1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW64);
                VLE64_V(v0, ed, VECTOR_MASKED, VECTOR_NFIELD1);
            }
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x2E:
            INST_NAME("VMASKMOVPS Ex, Vx, Gx");
            nextop = F8;
            if (MODREG) return 0;
            GETVY_vector(q0, VECTOR_SEW32);
            GETGY_vector(q1, VECTOR_SEW32);
            addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            VMSLT_VX(VMASK, q0, xZR, VECTOR_UNMASKED);
            VSE32_V(q1, ed, VECTOR_MASKED, VECTOR_NFIELD1);
            SMWRITE2();
            break;
        case 0x2F:
            INST_NAME("VMASKMOVPD Ex, Vx, Gx");
            nextop = F8;
            if (MODREG) return 0;
            GETVY_vector(q0, VECTOR_SEW64);
            GETGY_vector(q1, VECTOR_SEW64);
            addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW64);
            VMSLT_VX(VMASK, q0, xZR, VECTOR_UNMASKED);
            VSE64_V(q1, ed, VECTOR_MASKED, VECTOR_NFIELD1);
            SMWRITE2();
            break;
        case 0x30:
            INST_NAME("VPMOVZXBW Gx, Ex");
            nextop = F8;
            GETEY_vector(q0, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW16, 16 << vex.l);
            VZEXT_VF2(v0, q0, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x31:
            INST_NAME("VPMOVZXBD Gx, Ex");
            nextop = F8;
            GETEY_vector(q0, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW32, 16 << vex.l);
            VZEXT_VF4(v0, q0, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x32:
            INST_NAME("VPMOVZXBQ Gx, Ex");
            nextop = F8;
            GETEY_vector(q0, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW64, 16 << vex.l);
            VZEXT_VF8(v0, q0, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x33:
            INST_NAME("VPMOVZXWD Gx, Ex");
            nextop = F8;
            GETEY_vector(q0, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW32, 16 << vex.l);
            VZEXT_VF2(v0, q0, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x34:
            INST_NAME("VPMOVZXWQ Gx, Ex");
            nextop = F8;
            GETEY_vector(q0, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW64, 16 << vex.l);
            VZEXT_VF4(v0, q0, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x35:
            INST_NAME("VPMOVZXDQ Gx, Ex");
            nextop = F8;
            GETEY_vector(q0, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW64, 16 << vex.l);
            VZEXT_VF2(v0, q0, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x36:
            if (!vex.l) return 0;
            INST_NAME("VPERMD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VAND_VI(q0, q0, 7, VECTOR_UNMASKED);
            VRGATHER_VV(v0, q1, q0, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x37:
            INST_NAME("VPCMPGTQ Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VMSLT_VV(VMASK, q1, q0, VECTOR_UNMASKED);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VMERGE_VIM(v0, v0, 0x1f);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x38:
            INST_NAME("VPMINSB Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW8);
            GETEY_vector(q1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VMIN_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x39:
            INST_NAME("VPMINSD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VMIN_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x3D:
            INST_NAME("VPMAXSD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VMAX_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x3A:
            INST_NAME("VPMINUW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            VMINU_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x3B:
            INST_NAME("VPMINUD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VMINU_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x3C:
            INST_NAME("VPMAXSB Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW8);
            GETEY_vector(q1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VMAX_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x3E:
            INST_NAME("VPMAXUW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY_vector(q1, 0, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            VMAXU_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x3F:
            INST_NAME("VPMAXUD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VMAXU_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x40:
            INST_NAME("VPMULLD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VMUL_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x45:
            INST_NAME("VPSRLVD/Q Gx, Vx, Ex");
            nextop = F8;
            if (rex.w) {
                GETVY_vector(q0, VECTOR_SEW64);
                GETEY_vector(q1, 0, VECTOR_SEW64);
                GETGY_empty_vector(v0);
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                MOV32w(x4, 64);
                VMSLTU_VX(VMASK, q1, x4, VECTOR_UNMASKED);
                VSRL_VV(v0, q0, q1, VECTOR_MASKED);
                PUTGY_vector(v0, VECTOR_SEW64);
            } else {
                GETVY_vector(q0, VECTOR_SEW32);
                GETEY_vector(q1, 0, VECTOR_SEW32);
                GETGY_empty_vector(v0);
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                MOV32w(x4, 32);
                VMSLTU_VX(VMASK, q1, x4, VECTOR_UNMASKED);
                VSRL_VV(v0, q0, q1, VECTOR_MASKED);
                PUTGY_vector(v0, VECTOR_SEW32);
            }
            break;
        case 0x46:
            if (rex.w) return 0;
            INST_NAME("VPSRAVD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            q2 = fpu_get_scratch(dyn);
            MOV32w(x4, 31);
            VMINU_VX(q2, q1, x4, VECTOR_UNMASKED);
            VSRA_VV(v0, q0, q2, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x47:
            INST_NAME("VPSLLVD/Q Gx, Vx, Ex");
            nextop = F8;
            if (rex.w) {
                GETVY_vector(q0, VECTOR_SEW64);
                GETEY_vector(q1, 0, VECTOR_SEW64);
                GETGY_empty_vector(v0);
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                MOV32w(x4, 64);
                VMSLTU_VX(VMASK, q1, x4, VECTOR_UNMASKED);
                VSLL_VV(v0, q0, q1, VECTOR_MASKED);
                PUTGY_vector(v0, VECTOR_SEW64);
            } else {
                GETVY_vector(q0, VECTOR_SEW32);
                GETEY_vector(q1, 0, VECTOR_SEW32);
                GETGY_empty_vector(v0);
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                MOV32w(x4, 32);
                VMSLTU_VX(VMASK, q1, x4, VECTOR_UNMASKED);
                VSLL_VV(v0, q0, q1, VECTOR_MASKED);
                PUTGY_vector(v0, VECTOR_SEW32);
            }
            break;
        case 0x58:
            INST_NAME("VPBROADCASTD Gx, Ex");
            nextop = F8;
            GETGY_empty_vector(v0);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                q0 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q0, ed, 16, VECTOR_SEW32);
                VMV_X_S(x4, q0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                LW(x4, ed, fixedaddress);
            }
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            VMV_V_X(v0, x4);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x59:
            INST_NAME("VPBROADCASTQ Gx, Ex");
            nextop = F8;
            GETGY_empty_vector(v0);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                q0 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q0, ed, 16, VECTOR_SEW64);
                VMV_X_S(x4, q0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, ed, fixedaddress);
            }
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW64);
            VMV_V_X(v0, x4);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x5A:
            if (!vex.l) return 0;
            INST_NAME("VBROADCASTI128 Gx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                avx_load_reg_vector(dyn, ninst, x1, q0, ed, 16, VECTOR_SEW8);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 16);
                VLE8_V(q0, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q0);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 32);
            VSLIDEUP_VI(v0, q0, 16, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x78:
            INST_NAME("VPBROADCASTB Gx, Ex");
            nextop = F8;
            GETGY_empty_vector(v0);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                q0 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q0, ed, 16, VECTOR_SEW8);
                VMV_X_S(x4, q0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                LBU(x4, ed, fixedaddress);
            }
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW8);
            VMV_V_X(v0, x4);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x79:
            INST_NAME("VPBROADCASTW Gx, Ex");
            nextop = F8;
            GETGY_empty_vector(v0);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                q0 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q0, ed, 16, VECTOR_SEW16);
                VMV_X_S(x4, q0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                LHU(x4, ed, fixedaddress);
            }
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW16);
            VMV_V_X(v0, x4);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x8C:
            INST_NAME("VPMASKMOVD/Q Gx, Vx, Ex");
            nextop = F8;
            if (rex.w) {
                GETVY_vector(q0, VECTOR_SEW64);
                GETGY_empty_vector(v0);
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                VMSLT_VX(VMASK, q0, xZR, VECTOR_UNMASKED);
                if (MODREG) {
                    ed = (nextop & 7) + (rex.b << 3);
                    q1 = fpu_get_scratch(dyn);
                    avx_load_reg_vector(dyn, ninst, x1, q1, ed, 16 << vex.l, VECTOR_SEW64);
                    VMERGE_VVM(v0, v0, q1);
                } else {
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                    SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW64);
                    VLE64_V(v0, ed, VECTOR_MASKED, VECTOR_NFIELD1);
                }
                PUTGY_vector(v0, VECTOR_SEW64);
            } else {
                GETVY_vector(q0, VECTOR_SEW32);
                GETGY_empty_vector(v0);
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                VMSLT_VX(VMASK, q0, xZR, VECTOR_UNMASKED);
                if (MODREG) {
                    ed = (nextop & 7) + (rex.b << 3);
                    q1 = fpu_get_scratch(dyn);
                    avx_load_reg_vector(dyn, ninst, x1, q1, ed, 16 << vex.l, VECTOR_SEW32);
                    VMERGE_VVM(v0, v0, q1);
                } else {
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                    SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
                    VLE32_V(v0, ed, VECTOR_MASKED, VECTOR_NFIELD1);
                }
                PUTGY_vector(v0, VECTOR_SEW32);
            }
            break;
        case 0x8E:
            INST_NAME("VPMASKMOVD/Q Ex, Vx, Gx");
            nextop = F8;
            if (MODREG) return 0;
            if (rex.w) {
                GETVY_vector(q0, VECTOR_SEW64);
                GETGY_vector(q1, VECTOR_SEW64);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW64);
                VMSLT_VX(VMASK, q0, xZR, VECTOR_UNMASKED);
                VSE64_V(q1, ed, VECTOR_MASKED, VECTOR_NFIELD1);
                SMWRITE2();
            } else {
                GETVY_vector(q0, VECTOR_SEW32);
                GETGY_vector(q1, VECTOR_SEW32);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
                VMSLT_VX(VMASK, q0, xZR, VECTOR_UNMASKED);
                VSE32_V(q1, ed, VECTOR_MASKED, VECTOR_NFIELD1);
                SMWRITE2();
            }
            break;
        case 0x98:
            INST_NAME("VFMADD132PS/D Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            GETVY_vector(q0, sew);
            GETEY_vector(q1, 0, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q0);
            VFMACC_VV(v0, q2, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, sew);
            break;
        case 0x96:
            INST_NAME("VFMADDSUB132PS/D Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            GETVY_vector(q0, sew);
            GETEY_vector(q1, 0, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            v1 = fpu_get_scratch(dyn);
            VMV_V_V(v0, q0);
            VFMACC_VV(v0, q2, q1, VECTOR_UNMASKED);
            VMV_V_V(v1, q0);
            VFMSAC_VV(v1, q2, q1, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(rex.w ? (vex.l ? 5 : 1) : 0x55, x4, vex.l ? 2 : 1);
            VMERGE_VVM(v0, v0, v1);
            PUTGY_vector(v0, sew);
            break;
        case 0x97:
            INST_NAME("VFMSUBADD132PS/D Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            GETVY_vector(q0, sew);
            GETEY_vector(q1, 0, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            v1 = fpu_get_scratch(dyn);
            VMV_V_V(v0, q0);
            VFMSAC_VV(v0, q2, q1, VECTOR_UNMASKED);
            VMV_V_V(v1, q0);
            VFMACC_VV(v1, q2, q1, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(rex.w ? (vex.l ? 5 : 1) : 0x55, x4, vex.l ? 2 : 1);
            VMERGE_VVM(v0, v0, v1);
            PUTGY_vector(v0, sew);
            break;
        case 0x9A:
            INST_NAME("VFMSUB132PS/D Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            GETVY_vector(q0, sew);
            GETEY_vector(q1, 0, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q0);
            VFMSAC_VV(v0, q2, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, sew);
            break;
        case 0x9C:
            INST_NAME("VFNMADD132PS/D Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            GETVY_vector(q0, sew);
            GETEY_vector(q1, 0, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q0);
            VFNMSAC_VV(v0, q2, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, sew);
            break;
        case 0x9E:
            INST_NAME("VFNMSUB132PS/D Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            GETVY_vector(q0, sew);
            GETEY_vector(q1, 0, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q0);
            VFNMACC_VV(v0, q2, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, sew);
            break;
        case 0xA8:
            INST_NAME("VFMADD213PS/D Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            GETVY_vector(q0, sew);
            GETEY_vector(q1, 0, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q1);
            VFMACC_VV(v0, q0, q2, VECTOR_UNMASKED);
            PUTGY_vector(v0, sew);
            break;
        case 0xA6:
            INST_NAME("VFMADDSUB213PS/D Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            GETVY_vector(q0, sew);
            GETEY_vector(q1, 0, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            v1 = fpu_get_scratch(dyn);
            VMV_V_V(v0, q1);
            VFMACC_VV(v0, q0, q2, VECTOR_UNMASKED);
            VMV_V_V(v1, q1);
            VFMSAC_VV(v1, q0, q2, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(rex.w ? (vex.l ? 5 : 1) : 0x55, x4, vex.l ? 2 : 1);
            VMERGE_VVM(v0, v0, v1);
            PUTGY_vector(v0, sew);
            break;
        case 0xA7:
            INST_NAME("VFMSUBADD213PS/D Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            GETVY_vector(q0, sew);
            GETEY_vector(q1, 0, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            v1 = fpu_get_scratch(dyn);
            VMV_V_V(v0, q1);
            VFMSAC_VV(v0, q0, q2, VECTOR_UNMASKED);
            VMV_V_V(v1, q1);
            VFMACC_VV(v1, q0, q2, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(rex.w ? (vex.l ? 5 : 1) : 0x55, x4, vex.l ? 2 : 1);
            VMERGE_VVM(v0, v0, v1);
            PUTGY_vector(v0, sew);
            break;
        case 0xAA:
            INST_NAME("VFMSUB213PS/D Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            GETVY_vector(q0, sew);
            GETEY_vector(q1, 0, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q1);
            VFMSAC_VV(v0, q0, q2, VECTOR_UNMASKED);
            PUTGY_vector(v0, sew);
            break;
        case 0xAC:
            INST_NAME("VFNMADD213PS/D Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            GETVY_vector(q0, sew);
            GETEY_vector(q1, 0, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q1);
            VFNMSAC_VV(v0, q0, q2, VECTOR_UNMASKED);
            PUTGY_vector(v0, sew);
            break;
        case 0xAE:
            INST_NAME("VFNMSUB213PS/D Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            GETVY_vector(q0, sew);
            GETEY_vector(q1, 0, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q1);
            VFNMACC_VV(v0, q0, q2, VECTOR_UNMASKED);
            PUTGY_vector(v0, sew);
            break;
        case 0xB8:
            INST_NAME("VFMADD231PS/D Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            GETVY_vector(q0, sew);
            GETEY_vector(q1, 0, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q2);
            VFMACC_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, sew);
            break;
        case 0xB6:
            INST_NAME("VFMADDSUB231PS/D Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            GETVY_vector(q0, sew);
            GETEY_vector(q1, 0, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            v1 = fpu_get_scratch(dyn);
            VMV_V_V(v0, q2);
            VFMACC_VV(v0, q0, q1, VECTOR_UNMASKED);
            VMV_V_V(v1, q2);
            VFMSAC_VV(v1, q0, q1, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(rex.w ? (vex.l ? 5 : 1) : 0x55, x4, vex.l ? 2 : 1);
            VMERGE_VVM(v0, v0, v1);
            PUTGY_vector(v0, sew);
            break;
        case 0xB7:
            INST_NAME("VFMSUBADD231PS/D Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            GETVY_vector(q0, sew);
            GETEY_vector(q1, 0, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            v1 = fpu_get_scratch(dyn);
            VMV_V_V(v0, q2);
            VFMSAC_VV(v0, q0, q1, VECTOR_UNMASKED);
            VMV_V_V(v1, q2);
            VFMACC_VV(v1, q0, q1, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(rex.w ? (vex.l ? 5 : 1) : 0x55, x4, vex.l ? 2 : 1);
            VMERGE_VVM(v0, v0, v1);
            PUTGY_vector(v0, sew);
            break;
        case 0xBA:
            INST_NAME("VFMSUB231PS/D Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            GETVY_vector(q0, sew);
            GETEY_vector(q1, 0, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q2);
            VFMSAC_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, sew);
            break;
        case 0xBC:
            INST_NAME("VFNMADD231PS/D Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            GETVY_vector(q0, sew);
            GETEY_vector(q1, 0, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q2);
            VFNMSAC_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, sew);
            break;
        case 0xBE:
            INST_NAME("VFNMSUB231PS/D Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            GETVY_vector(q0, sew);
            GETEY_vector(q1, 0, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q2);
            VFNMACC_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, sew);
            break;
        case 0xDC:
        case 0xDD:
            fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
            return 0;
        case 0x13:
            INST_NAME("VCVTPH2PS Gx, Ex");
            nextop = F8;
            v1 = fpu_get_scratch(dyn);
            if (MODREG) {
                avx_load_reg_vector(dyn, ninst, x1, v1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW16);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, vex.l ? 1 : 0.5);
                VLE16_V(v1, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            GETGY_empty_vector(v0);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, vex.l ? 1 : 0.5);
            q2 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            VFWCVT_F_F_V(q2, v1, VECTOR_UNMASKED);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW32, 16 << vex.l);
            q3 = fpu_get_scratch(dyn);
            VZEXT_VF2(q3, v1, VECTOR_UNMASKED); // zero-extended halves
            q4 = fpu_get_scratch(dyn);
            VSRL_VI(q4, q3, 10, VECTOR_UNMASKED); // h >> 10
            MOV32w(x4, 0x1F);
            VAND_VX(q4, q4, x4, VECTOR_UNMASKED);     // h >> 10 & 0x1F
            VMSEQ_VX(VMASK, q4, x4, VECTOR_UNMASKED); // exponent == 0x1F
            MOV32w(x4, 0x3FF);
            VAND_VX(q4, q3, x4, VECTOR_UNMASKED); // h & 0x3FF
            q5 = fpu_get_scratch(dyn);
            VMSNE_VX(q5, q4, xZR, VECTOR_UNMASKED); // mantissa != 0
            VMAND_MM(VMASK, VMASK, q5);             // VMASK = NaN elements
            VSLL_VI(q4, q4, 13, VECTOR_UNMASKED);   // m << 13
            MOV32w(x4, 0x8000);
            VAND_VX(q5, q3, x4, VECTOR_UNMASKED); // sign
            VSLL_VI(q5, q5, 16, VECTOR_UNMASKED); // sign << 16
            MOV32w(x4, 0x7F800000);
            VOR_VX(q5, q5, x4, VECTOR_UNMASKED); // | inf exponent
            VOR_VV(q5, q5, q4, VECTOR_UNMASKED); // | m << 13
            MOV32w(x4, 0x400000);
            VOR_VX(q5, q5, x4, VECTOR_UNMASKED); // quiet bit (SNaN -> QNaN)
            VMERGE_VVM(q2, q2, q5);              // fix NaN elements
            VMV_V_V(v0, q2);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0xAD:
            INST_NAME("VFNMADD213Sx Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, sew);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                q1 = fpu_get_scratch(dyn);
                if (rex.w)
                    FLD(q1, ed, fixedaddress);
                else
                    FLW(q1, ed, fixedaddress);
                VFMV_S_F(q1, q1);
            }
            VMV_V_V(v0, q1);
            VFSGNJN_VV(v0, v0, v0, VECTOR_UNMASKED); // v0 = -Ex
            VECTOR_LOAD_VMASK(1, x4, 1);
            VFMACC_VV(v0, q0, q2, VECTOR_MASKED);    // v0[0] = Vx*Gx - Ex
            VFSGNJN_VV(v0, v0, v0, VECTOR_UNMASKED); // v0[0] = -(Vx*Gx) + Ex
            VMERGE_VVM(v0, q2, v0);
            PUTGY_vector(v0, sew);
            break;
        case 0xB9:
            INST_NAME("VFMADD231Sx Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q2);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, sew);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                q1 = fpu_get_scratch(dyn);
                if (rex.w)
                    FLD(q1, ed, fixedaddress);
                else
                    FLW(q1, ed, fixedaddress);
                VFMV_S_F(q1, q1);
            }
            VECTOR_LOAD_VMASK(1, x4, 1);
            VFMACC_VV(v0, q0, q1, VECTOR_MASKED); // v0[0] = Vx*Ex + Gx
            PUTGY_vector(v0, sew);
            break;
        case 0x99:
            INST_NAME("VFMADD132Sx Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q2);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, sew);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                q1 = fpu_get_scratch(dyn);
                if (rex.w)
                    FLD(q1, ed, fixedaddress);
                else
                    FLW(q1, ed, fixedaddress);
                VFMV_S_F(q1, q1);
            }
            VECTOR_LOAD_VMASK(1, x4, 1);
            VFMADD_VV(v0, q0, q1, VECTOR_MASKED);
            PUTGY_vector(v0, sew);
            break;
        case 0x9B:
            INST_NAME("VFMSUB132Sx Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q2);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, sew);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                q1 = fpu_get_scratch(dyn);
                if (rex.w)
                    FLD(q1, ed, fixedaddress);
                else
                    FLW(q1, ed, fixedaddress);
                VFMV_S_F(q1, q1);
            }
            VECTOR_LOAD_VMASK(1, x4, 1);
            VFMSUB_VV(v0, q0, q1, VECTOR_MASKED);
            PUTGY_vector(v0, sew);
            break;
        case 0x9D:
            INST_NAME("VFNMADD132Sx Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q2);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, sew);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                q1 = fpu_get_scratch(dyn);
                if (rex.w)
                    FLD(q1, ed, fixedaddress);
                else
                    FLW(q1, ed, fixedaddress);
                VFMV_S_F(q1, q1);
            }
            VECTOR_LOAD_VMASK(1, x4, 1);
            VFNMSUB_VV(v0, q0, q1, VECTOR_MASKED);
            VMERGE_VVM(v0, q2, v0);
            PUTGY_vector(v0, sew);
            break;
        case 0x9F:
            INST_NAME("VFNMSUB132Sx Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q2);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, sew);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                q1 = fpu_get_scratch(dyn);
                if (rex.w)
                    FLD(q1, ed, fixedaddress);
                else
                    FLW(q1, ed, fixedaddress);
                VFMV_S_F(q1, q1);
            }
            VECTOR_LOAD_VMASK(1, x4, 1);
            VFNMADD_VV(v0, q0, q1, VECTOR_MASKED);
            VMERGE_VVM(v0, q2, v0);
            PUTGY_vector(v0, sew);
            break;
        case 0xA9:
            INST_NAME("VFMADD213Sx Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, sew);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                q1 = fpu_get_scratch(dyn);
                if (rex.w)
                    FLD(q1, ed, fixedaddress);
                else
                    FLW(q1, ed, fixedaddress);
                VFMV_S_F(q1, q1);
            }
            VMV_V_V(v0, q1);
            VECTOR_LOAD_VMASK(1, x4, 1);
            VFMACC_VV(v0, q0, q2, VECTOR_MASKED);
            VMERGE_VVM(v0, q2, v0);
            PUTGY_vector(v0, sew);
            break;
        case 0xAB:
            INST_NAME("VFMSUB213Sx Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, sew);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                q1 = fpu_get_scratch(dyn);
                if (rex.w)
                    FLD(q1, ed, fixedaddress);
                else
                    FLW(q1, ed, fixedaddress);
                VFMV_S_F(q1, q1);
            }
            VMV_V_V(v0, q1);
            VFSGNJN_VV(v0, v0, v0, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(1, x4, 1);
            VFMACC_VV(v0, q0, q2, VECTOR_MASKED);
            VMERGE_VVM(v0, q2, v0);
            PUTGY_vector(v0, sew);
            break;
        case 0xAF:
            INST_NAME("VFNMSUB213Sx Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, sew);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                q1 = fpu_get_scratch(dyn);
                if (rex.w)
                    FLD(q1, ed, fixedaddress);
                else
                    FLW(q1, ed, fixedaddress);
                VFMV_S_F(q1, q1);
            }
            VMV_V_V(v0, q2);
            VECTOR_LOAD_VMASK(1, x4, 1);
            VFNMADD_VV(v0, q1, q0, VECTOR_MASKED);
            VMERGE_VVM(v0, q2, v0);
            PUTGY_vector(v0, sew);
            break;
        case 0xBB:
            INST_NAME("VFMSUB231Sx Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q2);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, sew);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                q1 = fpu_get_scratch(dyn);
                if (rex.w)
                    FLD(q1, ed, fixedaddress);
                else
                    FLW(q1, ed, fixedaddress);
                VFMV_S_F(q1, q1);
            }
            VFSGNJN_VV(v0, v0, v0, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(1, x4, 1);
            VFMACC_VV(v0, q0, q1, VECTOR_MASKED);
            VMERGE_VVM(v0, q2, v0);
            PUTGY_vector(v0, sew);
            break;
        case 0xBD:
            INST_NAME("VFNMADD231Sx Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q2);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, sew);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                q1 = fpu_get_scratch(dyn);
                if (rex.w)
                    FLD(q1, ed, fixedaddress);
                else
                    FLW(q1, ed, fixedaddress);
                VFMV_S_F(q1, q1);
            }
            VFSGNJN_VV(q0, q0, q0, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(1, x4, 1);
            VFMACC_VV(v0, q0, q1, VECTOR_MASKED);
            PUTGY_vector(v0, sew);
            break;
        case 0xBF:
            INST_NAME("VFNMSUB231Sx Gx, Vx, Ex");
            nextop = F8;
            sew = rex.w ? VECTOR_SEW64 : VECTOR_SEW32;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, sew);
            GETGY_vector(q2, sew);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q2);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, sew);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                q1 = fpu_get_scratch(dyn);
                if (rex.w)
                    FLD(q1, ed, fixedaddress);
                else
                    FLW(q1, ed, fixedaddress);
                VFMV_S_F(q1, q1);
            }
            VECTOR_LOAD_VMASK(1, x4, 1);
            VFNMACC_VV(v0, q0, q1, VECTOR_MASKED);
            VMERGE_VVM(v0, q2, v0);
            PUTGY_vector(v0, sew);
            break;
        default:
            DEFAULT_VECTOR;
    }
    return addr;
}
