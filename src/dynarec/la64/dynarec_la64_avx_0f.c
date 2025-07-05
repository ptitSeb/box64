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
        default:
            DEFAULT;
    }
    return addr;
}
