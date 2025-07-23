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

uintptr_t dynarec64_AVX_0F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
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
        case 0x10:
            INST_NAME("VMOVUPS Gx, Ex");
            nextop = F8;
            if (MODREG) {
                GETGY_empty_EY_xy(q0, q1, 0);
                if (vex.l) {
                    XVOR_V(q0, q1, q1);
                } else {
                    VOR_V(q0, q1, q1);
                }
            } else {
                GETGYxy_empty(q0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                if (vex.l) {
                    XVLD(q0, ed, fixedaddress);
                } else {
                    VLD(q0, ed, fixedaddress);
                }
            }
            break;
        case 0x11:
            INST_NAME("VMOVUPS Ex, Gx");
            nextop = F8;
            GETGYxy(q0, 0);
            if (MODREG) {
                GETEYxy_empty(q1, 0);
                if (vex.l) {
                    XVOR_V(q1, q0, q0);
                } else {
                    VOR_V(q1, q0, q0);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x4, x5, &fixedaddress, rex, NULL, 1, 0);
                if (vex.l) {
                    XVST(q0, ed, fixedaddress);
                } else {
                    VST(q0, ed, fixedaddress);
                }
                SMWRITE2();
            }
            break;
        case 0x12:
            nextop = F8;
            GETVYx(v1, 0);
            if (MODREG) {
                INST_NAME("VMOVHLPS Gx, Vx, Ex");
                GETEYx(v2, 0, 0);
                GETGYx_empty(v0);
                VEXTRINS_D(v0, v2, VEXTRINS_IMM_4_0(0, 1));
                VEXTRINS_D(v0, v1, VEXTRINS_IMM_4_0(1, 1));
            } else {
                INST_NAME("VMOVLPS Gx, Vx, Ex");
                GETEYSD(v2, 0, 0);
                GETGYx_empty(v0);
                VEXTRINS_D(v0, v2, VEXTRINS_IMM_4_0(0, 0));
                VEXTRINS_D(v0, v1, VEXTRINS_IMM_4_0(1, 1));
            }
            break;
        case 0x13:
            nextop = F8;
            INST_NAME("VMOVLPS Ex, Gx");
            GETGYx(v0, 0);
            if (MODREG) {
                DEFAULT;
                return addr;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x4, x5, &fixedaddress, rex, NULL, 1, 0);
                FST_D(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x14:
            INST_NAME("VUNPCKLPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VILVLxy(W, v0, v2, v1);
            break;
        case 0x15:
            INST_NAME("VUNPCKHPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VILVHxy(W, v0, v2, v1);
            break;
        case 0x16:
            nextop = F8;
            GETVYx(v1, 0);
            if (MODREG) {
                INST_NAME("VMOVLHPS Gx, Vx, Ex");
                GETEYx(v2, 0, 0);
                GETGYx_empty(v0);
                VEXTRINS_D(v0, v2, VEXTRINS_IMM_4_0(1, 0));
                VEXTRINS_D(v0, v1, VEXTRINS_IMM_4_0(0, 0));
            } else {
                INST_NAME("VMOVHPS Gx, Vx, Ex");
                GETEYSD(v2, 0, 0);
                GETGYx_empty(v0);
                VEXTRINS_D(v0, v2, VEXTRINS_IMM_4_0(1, 0));
                VEXTRINS_D(v0, v1, VEXTRINS_IMM_4_0(0, 0));
            }
            break;
        case 0x17:
            nextop = F8;
            INST_NAME("VMOVHPS Ex, Gx");
            GETGYx(v0, 0);
            if (MODREG) {
                DEFAULT;
                return addr;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x4, x5, &fixedaddress, rex, NULL, 1, 0);
                VSTELM_D(v0, ed, fixedaddress, 1);
                SMWRITE2();
            }
            break;
        case 0x28:
            INST_NAME("VMOVAPS Gx, Ex");
            nextop = F8;
            if (MODREG) {
                GETGY_empty_EY_xy(q0, q1, 0);
                if (vex.l) {
                    XVOR_V(q0, q1, q1);
                } else {
                    VOR_V(q0, q1, q1);
                }
            } else {
                GETGYxy_empty(q0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x4, x5, &fixedaddress, rex, NULL, 1, 0);
                if (vex.l) {
                    XVLD(q0, ed, fixedaddress);
                } else {
                    VLD(q0, ed, fixedaddress);
                }
            }
            break;
        case 0x29:
            INST_NAME("VMOVAPS Ex, Gx");
            nextop = F8;
            GETGYxy(q0, 0);
            if (MODREG) {
                GETEYxy_empty(q1, 0);
                if (vex.l) {
                    XVOR_V(q1, q0, q0);
                } else {
                    VOR_V(q1, q0, q0);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x4, x5, &fixedaddress, rex, NULL, 1, 0);
                if (vex.l) {
                    XVST(q0, ed, fixedaddress);
                } else {
                    VST(q0, ed, fixedaddress);
                }
                SMWRITE2();
            }
            break;
        case 0x2B:
            INST_NAME("VMOVNTPS Ex, Gx");
            nextop = F8;
            GETGYxy(q0, 0);
            if (MODREG) {
                DEFAULT;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x4, x5, &fixedaddress, rex, NULL, 1, 0);
                if (vex.l) {
                    XVST(q0, ed, fixedaddress);
                } else {
                    VST(q0, ed, fixedaddress);
                }
                SMWRITE2();
            }
            break;
        case 0x50:
            nextop = F8;
            INST_NAME("VMOVMSKPS Gd, Ex");
            GETEYxy(v0, 0, 0);
            GETGD;
            d1 = fpu_get_scratch(dyn);
            if (vex.l) {
                XVMSKLTZ_W(d1, v0);
                VPICKVE2GR_DU(gd, d1, 0);
                VPICKVE2GR_DU(x4, d1, 2);
                BSTRINS_D(gd, x4, 7, 4);
            } else {
                VMSKLTZ_W(d1, v0);
                VPICKVE2GR_DU(gd, d1, 0);
            }
            break;
        case 0x51:
            INST_NAME("VSQRTPS Gx, Ex");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFCMPxy(S, d0, v1, v1, cEQ);
                VFSQRTxy(S, v0, v1);
                VFCMPxy(S, d1, v0, v0, cEQ);
                VANDN_Vxy(d1, d1, d0);
                VSLLIxy(W, d1, d1, 31);
                VOR_Vxy(v0, v0, d1);
            } else {
                VFSQRTxy(S, v0, v1);
            }
            break;
        case 0x52:
            INST_NAME("VRSQRTPS Gx, Ex");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 0);
            if (cpuext.frecipe) {
                VFRSQRTExy(S, v0, v1);
            } else {
                VFRSQRTxy(S, v0, v1);
            }
            break;
        case 0x53:
            INST_NAME("VRCPPS Gx, Ex");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 0);
            if (cpuext.frecipe) {
                VFRECIPExy(S, v0, v1);
            } else {
                VFRECIPxy(S, v0, v1);
            }
            break;
        case 0x54:
            INST_NAME("VANDPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VAND_Vxy(v0, v1, v2);
            break;
        case 0x55:
            INST_NAME("VANDNPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VANDN_Vxy(v0, v1, v2);
            break;
        case 0x56:
            INST_NAME("VORPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VOR_Vxy(v0, v1, v2);
            break;
        case 0x57:
            INST_NAME("VXORPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VXOR_Vxy(v0, v1, v2);
            break;
        case 0x58:
            INST_NAME("VADDPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFCMPxy(S, d0, v1, v2, cUN);
            }
            VFADDxy(S, v0, v1, v2);
            if (!BOX64ENV(dynarec_fastnan)) {
                VFCMPxy(S, d1, v0, v0, cUN);
                VANDN_Vxy(d0, d0, d1);
                VLDIxy(d1, (0b010 << 9) | 0b1111111100);
                VSLLIxy(W, d1, d1, 20); // broadcast 0xFFC00000
                VBITSEL_Vxy(v0, v0, d1, d0);
            }
            break;
        case 0x59:
            INST_NAME("VMULPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFCMPxy(S, d0, v1, v2, cUN);
            }
            VFMULxy(S, v0, v1, v2);
            if (!BOX64ENV(dynarec_fastnan)) {
                VFCMPxy(S, d1, v0, v0, cUN);
                VANDN_Vxy(d0, d0, d1);
                VLDIxy(d1, (0b010 << 9) | 0b1111111100);
                VSLLIxy(W, d1, d1, 20); // broadcast 0xFFC00000
                VBITSEL_Vxy(v0, v0, d1, d0);
            }
            break;
        case 0x5C:
            INST_NAME("VSUBPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFCMPxy(S, d0, v1, v2, cUN);
            }
            VFSUBxy(S, v0, v1, v2);
            if (!BOX64ENV(dynarec_fastnan)) {
                VFCMPxy(S, d1, v0, v0, cUN);
                VANDN_Vxy(d0, d0, d1);
                VLDIxy(d1, (0b010 << 9) | 0b1111111100);
                VSLLIxy(W, d1, d1, 20); // broadcast 0xFFC00000
                VBITSEL_Vxy(v0, v0, d1, d0);
            }
            break;
        case 0x5D:
            INST_NAME("VMINPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (BOX64ENV(dynarec_fastnan)) {
                VFMINxy(S, v0, v2, v1);
            } else {
                q0 = fpu_get_scratch(dyn);
                q1 = fpu_get_scratch(dyn);
                VFCMPxy(S, q0, v2, v1, cULE);
                VBITSEL_Vxy(v0, v1, v2, q0);
            }
            break;
        case 0x5E:
            INST_NAME("VDIVPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFCMPxy(S, d0, v1, v2, cUN);
            }
            VFDIVxy(S, v0, v1, v2);
            if (!BOX64ENV(dynarec_fastnan)) {
                VFCMPxy(S, d1, v0, v0, cUN);
                VANDN_Vxy(d0, d0, d1);
                VLDIxy(d1, (0b010 << 9) | 0b1111111100);
                VSLLIxy(W, d1, d1, 20); // broadcast 0xFFC00000
                VBITSEL_Vxy(v0, v0, d1, d0);
            }
            break;
        case 0x5F:
            INST_NAME("VMAXPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (BOX64ENV(dynarec_fastnan)) {
                VFMAXxy(S, v0, v2, v1);
            } else {
                q0 = fpu_get_scratch(dyn);
                q1 = fpu_get_scratch(dyn);
                VFCMPxy(S, q0, v2, v1, cLT);
                VBITSEL_Vxy(v0, v2, v1, q0);
            }
            break;
        case 0x77:
            if (!vex.l) {
                INST_NAME("VZEROUPPER");
                if (vex.v != 0) {
                    UDF();
                } else {
                    q2 = fpu_get_scratch(dyn);
                    XVXOR_V(q2, q2, q2);
                    for (int i = 0; i < (rex.is32bits ? 8 : 16); ++i) {
                        if (dyn->lsx.avxcache[i].v != -1) {
                            // avx used register
                            if (dyn->lsx.avxcache[i].width == LSX_AVX_WIDTH_256) {
                                // 256 width, fill upper 128bits with zero.
                                q1 = avx_get_reg(dyn, ninst, x1, i, 1, LSX_AVX_WIDTH_256); // mark reg write (dirty)
                                XVPERMI_Q(q1, q2, XVPERMI_IMM_4_0(0, 2));
                            } else {
                                // 128 width, lazy save.
                                q1 = avx_get_reg(dyn, ninst, x1, i, 1, LSX_AVX_WIDTH_128); // mark reg write (dirty)
                                dyn->lsx.avxcache[i].zero_upper = 1;
                            }
                        } else {
                            // SSE register or unused register, store 128bit zero to x64emu_t.ymm[]
                            VST(q2, xEmu, offsetof(x64emu_t, ymm[i]));
                        }
                    }
                    SMWRITE2();
                }
            } else {
                INST_NAME("VZEROALL");
                if (vex.v != 0) {
                    UDF();
                } else {
                    for (int i = 0; i < (rex.is32bits ? 8 : 16); ++i) {
                        q0 = avx_get_reg_empty(dyn, ninst, x1, i, LSX_AVX_WIDTH_256);
                        XVXOR_V(q0, q0, q0);
                    }
                }
            }
            break;
        case 0xAE:
            nextop = F8;
            if (MODREG) {
                DEFAULT;
            } else
                switch ((nextop >> 3) & 7) {
                    case 2:
                        INST_NAME("VLDMXCSR Md");
                        GETED(0);
                        ST_W(ed, xEmu, offsetof(x64emu_t, mxcsr));
                        if (BOX64ENV(sse_flushto0)) {
                            // sync with fpsr, with mask from mxcsr
                            // TODO
                        }
                        break;
                    case 3:
                        INST_NAME("VSTMXCSR Md");
                        addr = geted(dyn, addr, ninst, nextop, &wback, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                        LD_WU(x4, xEmu, offsetof(x64emu_t, mxcsr));
                        ST_W(x4, wback, fixedaddress);
                        if (BOX64ENV(sse_flushto0)) {
                            // sync with fpsr, with mask from mxcsr
                            // TODO
                        }
                        break;
                    default:
                        DEFAULT;
                }
            break;
        case 0xC6:
            INST_NAME("VSHUFPS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8;
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            if (v1 == v2) {
                VSHUF4Ixy(W, v0, v1, u8);
            } else {
                VSHUF4Ixy(W, d0, v1, u8);
                VSHUF4Ixy(W, d1, v2, u8 >> 4);
                VPICKEVxy(D, v0, d1, d0);
            }
            break;
        default:
            DEFAULT;
    }
    return addr;
}
