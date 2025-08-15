#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fenv.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "x64_signals.h"
#include "os.h"
#include "debug.h"
#include "box64stack.h"
#include "x64emu.h"
#include "x64emu_private.h"
#include "x64run_private.h"
#include "x64primop.h"
#include "x64trace.h"
#include "x87emu_private.h"
#include "box64context.h"
#include "my_cpuid.h"
#include "bridge.h"
#include "emit_signals.h"
#include "x64shaext.h"
#ifdef DYNAREC
#include "custommem.h"
#include "../dynarec/native_lock.h"
#endif

#include "modrm.h"

#ifdef TEST_INTERPRETER
uintptr_t TestAVX_0F(x64test_t *test, vex_t vex, uintptr_t addr, int *step)
#else
uintptr_t RunAVX_0F(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    uint8_t tmp8u;
    int8_t tmp8s;
    int32_t tmp32s, tmp32s2;
    uint32_t tmp32u, tmp32u2;
    uint64_t tmp64u, tmp64u2;
    int64_t tmp64s;
    reg64_t *oped, *opgd;
    sse_regs_t *opex, *opgx, *opvx, eax1;
    sse_regs_t *opey, *opgy, *opvy, eay1;
    uint8_t maskps[4];
    uint8_t nanmask[4];

#ifdef TEST_INTERPRETER
    x64emu_t *emu = test->emu;
#endif
    opcode = F8;

    rex_t rex = vex.rex;

    switch(opcode) {

        case 0x10:  /* VMOVUPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            GX->q[0] = EX->q[0];
            GX->q[1] = EX->q[1];
            if(vex.l) {
                GETEY;
                GY->q[0] = EY->q[0];
                GY->q[1] = EY->q[1];
            } else {
                GY->u128 = 0;
            }
            break;
        case 0x11:  /* VMOVUPS Ex, Gx */
            nextop = F8;
            GETEX(0);
            GETGX;
            EX->q[0] = GX->q[0];
            EX->q[1] = GX->q[1];
            if(vex.l) {
                GETEY;
                GETGY;
                EY->q[0] = GY->q[0];
                EY->q[1] = GY->q[1];
            } else if(MODREG) {
                GETEY;
                EY->u128 = 0;
            }
            break;
        case 0x12:
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            if(MODREG)    /* VMOVHLPS Gx, Vx, Ex */
                GX->q[0] = EX->q[1];
            else
                GX->q[0] = EX->q[0];    /* VMOVLPS Gx, Vx, Ex */
            GX->q[1] = VX->q[1];
            GETGY;
            GY->u128 = 0;
            break;
        case 0x13:                      /* VMOVLPS Ex, Gx */
            nextop = F8;
            if(!MODREG) {
                GETEX(0);
                GETGX;
                EX->q[0] = GX->q[0];
            }
            break;
        case 0x14:  /* VUNPCKLPS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GX->ud[3] = EX->ud[1];
            GX->ud[2] = VX->ud[1];
            GX->ud[1] = EX->ud[0];
            GX->ud[0] = VX->ud[0];
            if(vex.l) {
                GETEY;
                GETVY;
                GY->ud[3] = EY->ud[1];
                GY->ud[2] = VY->ud[1];
                GY->ud[1] = EY->ud[0];
                GY->ud[0] = VY->ud[0];
            } else
                GY->u128 = 0;
            break;
        case 0x15:                      /* VUNPCKHPS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            GX->ud[0] = VX->ud[2];
            GX->ud[1] = EX->ud[2];
            GX->ud[2] = VX->ud[3];
            GX->ud[3] = EX->ud[3];
            if(vex.l) {
                GETEY; GETVY;
                GY->ud[0] = VY->ud[2];
                GY->ud[1] = EY->ud[2];
                GY->ud[2] = VY->ud[3];
                GY->ud[3] = EY->ud[3];
            } else
                GY->u128 = 0;
            break;
        case 0x16:
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(MODREG) {                /* VMOVLHPS Gx, Vx, Ex */
                GX->q[1] = EX->q[0];
            } else {
                GX->q[1] = EX->q[0];    /* VMOVHPS Gx, Vx, Ex */
            }
            GX->q[0] = VX->q[0];
            GY->u128 = 0;
            break;
        case 0x17:                      /* VMOVHPS Ex,Gx */
            nextop = F8;
            GETEX(0);
            GETGX;
            EX->q[0] = GX->q[1];
            if(MODREG) {
                GETEY;
                EY->u128 = 0;
            }
            break;

        case 0x28:  /* VMOVAPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            GX->q[0] = EX->q[0];
            GX->q[1] = EX->q[1];
            if(vex.l) {
                GETEY;
                GY->q[0] = EY->q[0];
                GY->q[1] = EY->q[1];
            } else {
                GY->u128 = 0;
            }
            break;
        case 0x29:  /* VMOVAPS Ex, Gx */
            nextop = F8;
            GETEX(0);
            GETGX;
            EX->q[0] = GX->q[0];
            EX->q[1] = GX->q[1];
            if(vex.l) {
                GETEY;
                GETGY;
                EY->q[0] = GY->q[0];
                EY->q[1] = GY->q[1];
            } else if(MODREG) {
                GETEY;
                EY->u128 = 0;
            }
            break;

        case 0x2B:                      /* VMOVNTPS Ex,Gx */
            nextop = F8;
            if(!MODREG) {
                GETEX(0);
                GETGX;
                EX->q[0] = GX->q[0];
                EX->q[1] = GX->q[1];
                if(vex.l) {
                    GETEY;
                    GETGY;
                    EY->q[0] = GY->q[0];
                    EY->q[1] = GY->q[1];
                }
            }
            break;

        case 0x2E:                      /* VUCOMISS Gx, Ex */
        case 0x2F:                      /* VCOMISS Gx, Ex */
            RESET_FLAGS(emu);
            nextop = F8;
            GETEX(0);
            GETGX;
            if(isnan(GX->f[0]) || isnan(EX->f[0])) {
                SET_FLAG(F_ZF); SET_FLAG(F_PF); SET_FLAG(F_CF);
            } else if(isgreater(GX->f[0], EX->f[0])) {
                CLEAR_FLAG(F_ZF); CLEAR_FLAG(F_PF); CLEAR_FLAG(F_CF);
            } else if(isless(GX->f[0], EX->f[0])) {
                CLEAR_FLAG(F_ZF); CLEAR_FLAG(F_PF); SET_FLAG(F_CF);
            } else {
                SET_FLAG(F_ZF); CLEAR_FLAG(F_PF); CLEAR_FLAG(F_CF);
            }
            CLEAR_FLAG(F_OF); CLEAR_FLAG(F_AF); CLEAR_FLAG(F_SF);
            break;

        case 0x50:                      /* VMOVMSKPS Gd, Ex */
            nextop = F8;
            GETEX(0);
            GETGD;
            GD->q[0] = 0;
            for(int i=0; i<4; ++i)
                GD->dword[0] |= ((EX->ud[i]>>31)&1)<<i;
            if(vex.l) {
                GETEY;
                for(int i=0; i<4; ++i)
                    GD->dword[0] |= ((EY->ud[i]>>31)&1)<<(i+4);
            }
            break;
        case 0x51:                      /* VSQRTPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETGY;
            for (int i = 0; i < 4; ++i)
                if (isnan(EX->f[i]))
                    GX->f[i] = EX->f[i];
                else
                    GX->f[i] = (EX->f[i] < 0) ? (-NAN) : sqrtf(EX->f[i]);
            if(vex.l) {
                GETEY;
                for (int i = 0; i < 4; ++i)
                    if (isnan(EY->f[i]))
                        GY->f[i] = EY->f[i];
                    else
                        GY->f[i] = (EY->f[i] < 0) ? (-NAN) : sqrtf(EY->f[i]);
            } else
                GY->u128 = 0;
            break;
        case 0x52:                      /* VRSQRTPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            for(int i=0; i<4; ++i) {
                if (isnan(EX->f[i]))
                    GX->f[i] = EX->f[i];
                else if (EX->f[i] == 0)
                    GX->f[i] = 1.0f/EX->f[i];
                else if (EX->f[i]<0)
                    GX->f[i] = -NAN;
                else if (isnan(EX->f[i]))
                    GX->f[i] = EX->f[i];
                else if (isinf(EX->f[i]))
                    GX->f[i] = 0.0;
                else
                    GX->f[i] = 1.0f/sqrtf(EX->f[i]);
            }
            if(vex.l) {
                GETEY;
                for(int i=0; i<4; ++i) {
                    if (isnan(EY->f[i]))
                        GY->f[i] = EY->f[i];
                    else if (EY->f[i] == 0)
                        GY->f[i] = 1.0f/EY->f[i];
                    else if (EY->f[i]<0)
                        GY->f[i] = -NAN;
                    else if (isnan(EY->f[i]))
                        GY->f[i] = EY->f[i];
                    else if (isinf(EY->f[i]))
                        GY->f[i] = 0.0;
                    else
                        GY->f[i] = 1.0f/sqrtf(EY->f[i]);
                }
            } else
                GY->u128 = 0;
            break;
        case 0x53:                      /* VRCPPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETGY;
            for(int i=0; i<4; ++i)
                if (isnan(EX->f[i]))
                    GX->f[i] = EX->f[i];
                else
                    GX->f[i] = 1.0f / EX->f[i];
            if(vex.l) {
                GETEY;
                for(int i=0; i<4; ++i)
                    if (isnan(EY->f[i]))
                        GY->f[i] = EY->f[i];
                    else
                        GY->f[i] = 1.0f / EY->f[i];
            } else
                GY->u128 = 0;
            break;
        case 0x54:                      /* VANDPS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GX->u128 = VX->u128 & EX->u128;
            GETGY;
            if(vex.l) {
                GETEY;
                GETVY;
                GY->u128 = VY->u128 & EY->u128;
            } else {
                GY->u128 = 0;
            }
            break;
        case 0x55:                      /* VANDNPS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GX->u128 = (~VX->u128) & EX->u128;
            GETGY;
            if(vex.l) {
                GETEY;
                GETVY;
                GY->u128 = (~VY->u128) & EY->u128;
            } else {
                GY->u128 = 0;
            }
            break;
        case 0x56:                      /* VORPS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GX->u128 = VX->u128 | EX->u128;
            GETGY;
            if(vex.l) {
                GETEY;
                GETVY;
                GY->u128 = VY->u128 | EY->u128;
            } else {
                GY->u128 = 0;
            }
            break;
        case 0x57:                      /* XORPS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GX->u128 = VX->u128 ^ EX->u128;
            if(vex.l) {
                GETEY;
                GETVY;
                GY->u128 = VY->u128 ^ EY->u128;
            } else
                GY->u128 = 0;
            break;
        case 0x58:                      /* VADDPS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<4; ++i) {
                if (isnan(VX->f[i]) || isnan(EX->f[i])) {
                    if (isnan(VX->f[i]))
                        GX->f[i] = VX->f[i];
                    else
                        GX->f[i] = EX->f[i];
                    continue;
                }
                GX->f[i] = VX->f[i] + EX->f[i];
                if(isnan(GX->f[i])) GX->f[i] = -NAN;
            }
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<4; ++i) {
                    if (isnan(VY->f[i]) || isnan(EY->f[i])) {
                        if (isnan(VY->f[i]))
                            GY->f[i] = VY->f[i];
                        else
                            GY->f[i] = EY->f[i];
                        continue;
                    }
                    GY->f[i] = VY->f[i] + EY->f[i];
                    if(isnan(GY->f[i])) GY->f[i] = -NAN;
                }
            } else
                GY->u128 = 0;
            break;
        case 0x59:                      /* VMULPS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<4; ++i) {
                if (isnan(VX->f[i]) || isnan(EX->f[i])) {
                    if (isnan(VX->f[i]))
                        GX->f[i] = VX->f[i];
                    else
                        GX->f[i] = EX->f[i];
                    continue;
                }
                GX->f[i] = VX->f[i] * EX->f[i];
                if (isnan(GX->f[i])) GX->ud[i] |= 0x80000000;
            }
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<4; ++i) {
                    if (isnan(VY->f[i]) || isnan(EY->f[i])) {
                        if (isnan(VY->f[i]))
                            GY->f[i] = VY->f[i];
                        else
                            GY->f[i] = EY->f[i];
                        continue;
                    }
                    GY->f[i] = VY->f[i] * EY->f[i];
                    if (isnan(GY->f[i])) GY->ud[i] |= 0x80000000;
                }
            } else
                GY->u128 = 0;
            break;
        case 0x5A:                      /* VCVTPS2PD Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            if (vex.l) {
                GY->d[1] = EX->f[3];
                GY->d[0] = EX->f[2];
            } else
                GY->u128 = 0;
            GX->d[1] = EX->f[1];
            GX->d[0] = EX->f[0];
            break;
        case 0x5B:                      /* VCVTDQ2PS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            GX->f[0] = EX->sd[0];
            GX->f[1] = EX->sd[1];
            GX->f[2] = EX->sd[2];
            GX->f[3] = EX->sd[3];
            if(vex.l) {
                GETEY;
                GY->f[0] = EY->sd[0];
                GY->f[1] = EY->sd[1];
                GY->f[2] = EY->sd[2];
                GY->f[3] = EY->sd[3];
            } else
                GY->u128 = 0;
            break;
        case 0x5C:                      /* VSUBPS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<4; ++i) {
                if (isnan(VX->f[i]) || isnan(EX->f[i])) {
                    if (isnan(VX->f[i]))
                        GX->f[i] = VX->f[i];
                    else
                        GX->f[i] = EX->f[i];
                    continue;
                }
                GX->f[i] = VX->f[i] - EX->f[i];
                if (isnan(GX->f[i])) GX->ud[i] |= 0x80000000;
            }
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<4; ++i) {
                    if (isnan(VY->f[i]) || isnan(EY->f[i])) {
                        if (isnan(VY->f[i]))
                            GY->f[i] = VY->f[i];
                        else
                            GY->f[i] = EY->f[i];
                        continue;
                    }
                    GY->f[i] = VY->f[i] - EY->f[i];
                    if (isnan(GY->f[i])) GY->ud[i] |= 0x80000000;
                }
            } else
                GY->u128 = 0;
            break;
        case 0x5D:                      /* VMINPS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<4; ++i)
                if (isnanf(VX->f[i]) || isnanf(EX->f[i]) || islessequal(EX->f[i], VX->f[i]))
                    GX->f[i] = EX->f[i];
                else
                    GX->f[i] = VX->f[i];
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<4; ++i)
                    if (isnanf(VY->f[i]) || isnanf(EY->f[i]) || islessequal(EY->f[i], VY->f[i]))
                        GY->f[i] = EY->f[i];
                    else
                        GY->f[i] = VY->f[i];
            } else
                GY->u128 = 0;
            break;
        case 0x5E:                      /* VDIVPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<4; ++i) {
                if (isnan(VX->f[i]) || isnan(EX->f[i])) {
                    if (isnan(VX->f[i]))
                        GX->f[i] = VX->f[i];
                    else
                        GX->f[i] = EX->f[i];
                    continue;
                }
                GX->f[i] = VX->f[i] / EX->f[i];
                if (isnan(GX->f[i])) GX->ud[i] |= 0x80000000;
            }
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<4; ++i) {
                    if (isnan(VY->f[i]) || isnan(EY->f[i])) {
                        if (isnan(VY->f[i]))
                            GY->f[i] = VY->f[i];
                        else
                            GY->f[i] = EY->f[i];
                        continue;
                    }
                    GY->f[i] = VY->f[i] / EY->f[i];
                    if (isnan(GY->f[i])) GY->ud[i] |= 0x80000000;
                }
            } else
                GY->u128 = 0;
            break;
        case 0x5F:                      /* VMAXPS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<4; ++i)
                if (isnanf(VX->f[i]) || isnanf(EX->f[i]) || isgreaterequal(EX->f[i], VX->f[i]))
                    GX->f[i] = EX->f[i];
                else
                    GX->f[i] = VX->f[i];
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<4; ++i)
                    if (isnanf(VY->f[i]) || isnanf(EY->f[i]) || isgreaterequal(EY->f[i], VY->f[i]))
                        GY->f[i] = EY->f[i];
                    else
                        GY->f[i] = VY->f[i];
            } else
                GY->u128 = 0;
            break;


        case 0x77:
            if(!vex.l) {    // VZEROUPPER
                if(vex.v!=0) {
                    EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
                } else {
                    memset(emu->ymm, 0, sizeof(sse_regs_t)*((vex.rex.is32bits)?8:16));
                }
            } else {    // VZEROALL
                if(vex.v!=0) {
                    EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
                } else {
                    memset(emu->xmm, 0, sizeof(sse_regs_t)*((vex.rex.is32bits)?8:16));
                    memset(emu->ymm, 0, sizeof(sse_regs_t)*((vex.rex.is32bits)?8:16));
                }
            }
            break;

        case 0xAE:                      /* Grp Ed (SSE) */
            nextop = F8;
            if(MODREG)
                return 0;
            else
            switch((nextop>>3)&7) {
                case 2:                 /* VLDMXCSR Md */
                    GETED(0);
                    emu->mxcsr.x32 = ED->dword[0];
                    #ifndef TEST_INTERPRETER
                    if(BOX64ENV(sse_flushto0))
                        applyFlushTo0(emu);
                    #endif
                    break;
                case 3:                 /* VSTMXCSR Md */
                    GETED(0);
                    ED->dword[0] = emu->mxcsr.x32;
                    break;
                default:
                    return 0;
            }
            break;

        case 0xC2:                      /* VCMPPS Gx, Vx, Ex, Ib */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETVX;
            GETGY;
            tmp8u = F8;
            for(int i=0; i<4; ++i) {
                tmp8s = 0;
                int is_nan = isnan(VX->f[i]) || isnan(EX->f[i]);
                // the 1f..0f opcode are singaling/unsignaling, wich is not handled
                switch(tmp8u&0x0f) {
                    case 0x00: tmp8s=(VX->f[i] == EX->f[i]) && !is_nan; break;
                    case 0x01: tmp8s=isless(VX->f[i], EX->f[i]) && !is_nan; break;
                    case 0x02: tmp8s=islessequal(VX->f[i], EX->f[i]) && !is_nan; break;
                    case 0x03: tmp8s=is_nan; break;
                    case 0x04: tmp8s=(VX->f[i] != EX->f[i]) || is_nan; break;
                    case 0x05: tmp8s=is_nan || isgreaterequal(VX->f[i], EX->f[i]); break;
                    case 0x06: tmp8s=is_nan || isgreater(VX->f[i], EX->f[i]); break;
                    case 0x07: tmp8s=!is_nan; break;
                    case 0x08: tmp8s=(VX->f[i] == EX->f[i]) || is_nan; break;
                    case 0x09: tmp8s=isless(VX->f[i], EX->f[i]) || is_nan; break;
                    case 0x0a: tmp8s=islessequal(VX->f[i], EX->f[i]) || is_nan; break;
                    case 0x0b: tmp8s=0; break;
                    case 0x0c: tmp8s=(VX->f[i] != EX->f[i]) && !is_nan; break;
                    case 0x0d: tmp8s=isgreaterequal(VX->f[i], EX->f[i]) && !is_nan; break;
                    case 0x0e: tmp8s=isgreater(VX->f[i], EX->f[i]) && !is_nan; break;
                    case 0x0f: tmp8s=1; break;
                }
                GX->ud[i]=(tmp8s)?0xffffffff:0;
            }
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<4; ++i) {
                    tmp8s = 0;
                    int is_nan = isnan(VY->f[i]) || isnan(EY->f[i]);
                    // the 1f..0f opcode are singaling/unsignaling, wich is not handled
                    switch(tmp8u&0x0f) {
                        case 0x00: tmp8s=(VY->f[i] == EY->f[i]) && !is_nan; break;
                        case 0x01: tmp8s=isless(VY->f[i], EY->f[i]) && !is_nan; break;
                        case 0x02: tmp8s=islessequal(VY->f[i], EY->f[i]) && !is_nan; break;
                        case 0x03: tmp8s=is_nan; break;
                        case 0x04: tmp8s=(VY->f[i] != EY->f[i]) || is_nan; break;
                        case 0x05: tmp8s=is_nan || isgreaterequal(VY->f[i], EY->f[i]); break;
                        case 0x06: tmp8s=is_nan || isgreater(VY->f[i], EY->f[i]); break;
                        case 0x07: tmp8s=!is_nan; break;
                        case 0x08: tmp8s=(VY->f[i] == EY->f[i]) || is_nan; break;
                        case 0x09: tmp8s=isless(VY->f[i], EY->f[i]) || is_nan; break;
                        case 0x0a: tmp8s=islessequal(VY->f[i], EY->f[i]) || is_nan; break;
                        case 0x0b: tmp8s=0; break;
                        case 0x0c: tmp8s=(VY->f[i] != EY->f[i]) && !is_nan; break;
                        case 0x0d: tmp8s=isgreaterequal(VY->f[i], EY->f[i]) && !is_nan; break;
                        case 0x0e: tmp8s=isgreater(VY->f[i], EY->f[i]) && !is_nan; break;
                        case 0x0f: tmp8s=1; break;
                    }
                    GY->ud[i]=(tmp8s)?0xffffffff:0;
                }
            } else
                GY->u128 = 0;
            break;

        case 0xC6:                      /* VSHUFPS Gx, Vx, Ex, Ib */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETVX;
            GETGY;
            GETVY;
            GETEY;
            tmp8u = F8;
            if(GX==VX) {
                eax1 = *VX;
                VX = &eax1;
            }
            if(GX==EX) {
                eay1 = *EX;
                EX = &eay1;
            }
            for(int i=0; i<2; ++i) {
                GX->ud[i] = VX->ud[(tmp8u>>(i*2))&3];
            }
            for(int i=2; i<4; ++i) {
                GX->ud[i] = EX->ud[(tmp8u>>(i*2))&3];
            }
            if(vex.l) {
                if(GY==VY) {
                    eax1 = *VY;
                    VY = &eax1;
                }
                if(GY==EY) {
                    eay1 = *EY;
                    EY = &eay1;
                }
                for(int i=0; i<2; ++i) {
                    GY->ud[i] = VY->ud[(tmp8u>>(i*2))&3];
                }
                for(int i=2; i<4; ++i) {
                    GY->ud[i] = EY->ud[(tmp8u>>(i*2))&3];
                }
            } else
                GY->u128 = 0;
            break;

        default:
            return 0;
    }
    return addr;
}
