#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fenv.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"
#include "box64stack.h"
#include "x64emu.h"
#include "x64run.h"
#include "x64emu_private.h"
#include "x64run_private.h"
#include "x64primop.h"
#include "x64trace.h"
#include "x87emu_private.h"
#include "box64context.h"
#include "my_cpuid.h"
#include "bridge.h"
#include "signals.h"
#include "x64shaext.h"
#ifdef DYNAREC
#include "custommem.h"
#include "../dynarec/native_lock.h"
#endif

#include "modrm.h"

#ifdef __clang__
extern int isinff(float);
extern int isnanf(float);
#endif

#ifdef TEST_INTERPRETER
uintptr_t TestAVX_F30F(x64test_t *test, vex_t vex, uintptr_t addr, int *step)
#else
uintptr_t RunAVX_F30F(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step)
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
    int is_nan;


#ifdef TEST_INTERPRETER
    x64emu_t *emu = test->emu;
#endif
    opcode = F8;

    rex_t rex = vex.rex;

    switch(opcode) {

        case 0x10:  /* VMOVSS Gx, [Vx,] Ex */
            nextop = F8;
            GETEX4(0);
            GETGX;
            GX->ud[0] = EX->ud[0];
            if(MODREG) {
                GETVX;
                GX->ud[1] = VX->ud[1];
                GX->q[1] = VX->q[1];
            } else {
                GX->ud[1] = GX->ud[2] = GX->ud[3] = 0;
            }
            GETGY;
            GY->u128 = 0;
            break;
        case 0x11:  /* VMOVSS Ex, [Vx,] Gx */
            nextop = F8;
            GETEX4(0);
            GETGX;
            EX->ud[0] = GX->ud[0];
            if(MODREG) {
                GETVX;
                EX->ud[1] = VX->ud[1];
                EX->q[1] = VX->q[1];
                GETEY;
                EY->q[0] = EY->q[1] = 0;
            }
            break;
        case 0x12:  /* VMOVSLDUP Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            GX->ud[1] = GX->ud[0] = EX->ud[0];
            GX->ud[3] = GX->ud[2] = EX->ud[2];
            if(vex.l) {
                GETEY;
                GY->ud[1] = GY->ud[0] = EY->ud[0];
                GY->ud[3] = GY->ud[2] = EY->ud[2];
            } else
                GY->u128 = 0;
            break;

        case 0x16:  /* MOVSHDUP Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            GX->ud[1] = GX->ud[0] = EX->ud[1];
            GX->ud[3] = GX->ud[2] = EX->ud[3];
            if(vex.l) {
                GETEY;
                GY->ud[1] = GY->ud[0] = EY->ud[1];
                GY->ud[3] = GY->ud[2] = EY->ud[3];
            } else
                GY->u128 = 0;
            break;

        case 0x2A:  /* VCVTSI2SS Gx, Vx, Ed */
            nextop = F8;
            GETED(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w)
                GX->f[0] = ED->sq[0];
            else
                GX->f[0] = ED->sdword[0];
            GX->ud[1] = VX->ud[1];
            GX->q[1] = VX->q[1];
            GY->u128 = 0;
            break;

        case 0x2C:  /* VCVTTSS2SI Gd, Ex */
            nextop = F8;
            GETEX(0);
            GETGD;
            if (rex.w) {
                if(isnanf(EX->f[0]) || isinff(EX->f[0]) || EX->f[0]>(float)0x7fffffffffffffffLL)
                    GD->q[0] = 0x8000000000000000LL;
                else
                    GD->sq[0] = EX->f[0];
            } else {
                if(isnanf(EX->f[0]) || isinff(EX->f[0]) || EX->f[0]>(float)0x7fffffff)
                    GD->dword[0] = 0x80000000;
                else
                    GD->sdword[0] = EX->f[0];
                GD->dword[1] = 0;
            }
            break;
        case 0x2D:  /* VCVTSS2SI Gd, Ex */
            nextop = F8;
            GETEX(0);
            GETGD;
            if(rex.w) {
                if(isnanf(EX->f[0]) || isinff(EX->f[0]) || EX->f[0]>(float)0x7fffffffffffffffLL)
                    GD->q[0] = 0x8000000000000000LL;
                else
                    switch(emu->mxcsr.f.MXCSR_RC) {
                        case ROUND_Nearest: {
                            int round = fegetround();
                            fesetround(FE_TONEAREST);
                            GD->sq[0] = nearbyintf(EX->f[0]);
                            fesetround(round);
                            break;
                        }
                        case ROUND_Down:
                            GD->sq[0] = floorf(EX->f[0]);
                            break;
                        case ROUND_Up:
                            GD->sq[0] = ceilf(EX->f[0]);
                            break;
                        case ROUND_Chop:
                            GD->sq[0] = EX->f[0];
                            break;
                    }
            } else {
                if(isnanf(EX->f[0]))
                    tmp64s = INT32_MIN;
                else
                    switch(emu->mxcsr.f.MXCSR_RC) {
                        case ROUND_Nearest: {
                            int round = fegetround();
                            fesetround(FE_TONEAREST);
                            tmp64s = nearbyintf(EX->f[0]);
                            fesetround(round);
                            break;
                        }
                        case ROUND_Down:
                            tmp64s = floorf(EX->f[0]);
                            break;
                        case ROUND_Up:
                            tmp64s = ceilf(EX->f[0]);
                            break;
                        case ROUND_Chop:
                            tmp64s = EX->f[0];
                            break;
                    }
                if (tmp64s==(int32_t)tmp64s)
                    GD->sdword[0] = (int32_t)tmp64s;
                else
                    GD->sdword[0] = INT32_MIN;
                GD->dword[1] = 0;
            }
            break;

        case 0x51:  /* VSQRTSS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            if(EX->f[0]<0.0 )
                GX->f[0] = -NAN;
            else
                GX->f[0] = sqrt(EX->f[0]);
            if(GX!=VX) {
                GX->ud[1] = VX->ud[1];
                GX->q[1] = VX->q[1];
            }
            GY->u128 = 0;
            break;
        case 0x52:  /* VRSQRTSS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            GX->f[0] = 1.0f/sqrtf(EX->f[0]);
            if(GX!=VX) {
                GX->ud[1] = VX->ud[1];
                GX->q[1] = VX->q[1];
            }
            GY->u128 = 0;
            break;
        case 0x53:  /* VRCPSS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            GX->f[0] = 1.0f/EX->f[0];
            if(GX!=VX) {
                GX->ud[1] = VX->ud[1];
                GX->q[1] = VX->q[1];
            }
            GY->u128 = 0;
            break;

        case 0x58:  /* VADDSS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GX->f[0] = VX->f[0] + EX->f[0];
            if(GX!=VX) {
                GX->ud[1] = VX->ud[1];
                GX->q[1] = VX->q[1];
            }
            GY->u128 = 0;
            break;
        case 0x59:  /* VMULSS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GX->f[0] = VX->f[0] * EX->f[0];
            if(GX!=VX) {
                GX->ud[1] = VX->ud[1];
                GX->q[1] = VX->q[1];
            }
            GY->u128 = 0;
            break;
        case 0x5A:  /* VCVTSS2SD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GX->d[0] = EX->f[0];
            GX->q[1] = VX->q[1];
            GY->u128 = 0;
            break;
        case 0x5B:  /* VCVTTPS2DQ Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            for(int i=0; i<4; ++i) {
                if(isnanf(EX->f[i]))
                    tmp64s = INT32_MIN;
                else
                    tmp64s = EX->f[i];
                if (tmp64s==(int32_t)tmp64s) {
                    GX->sd[i] = (int32_t)tmp64s;
                } else {
                    GX->sd[i] = INT32_MIN;
                }
            }
            if(vex.l) {
                GETEY;
                for(int i=0; i<4; ++i) {
                    if(isnanf(EY->f[i]))
                        tmp64s = INT32_MIN;
                    else
                        tmp64s = EY->f[i];
                    if (tmp64s==(int32_t)tmp64s) {
                        GY->sd[i] = (int32_t)tmp64s;
                    } else {
                        GY->sd[i] = INT32_MIN;
                    }
                }
            } else
                GY->u128 = 0;
            break;
        case 0x5C:  /* VSUBSS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GX->f[0] = VX->f[0] - EX->f[0];
            if(GX!=VX) {
                GX->ud[1] = VX->ud[1];
                GX->q[1] = VX->q[1];
            }
            GY->u128 = 0;
            break;

        case 0x5D:  /* VMINSS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if (VX->f[0] == 0.0 && EX->f[0]  == 0.0)
                GX->f[0] = EX->f[0];
            else if (isnan(VX->f[0]) || isnan(EX->f[0]) || isgreater(VX->f[0], EX->f[0]))
                GX->f[0] = EX->f[0];
            else
                GX->f[0] = VX->f[0];
            if(GX!=VX) {
                GX->ud[1] = VX->ud[1];
                GX->q[1] = VX->q[1];
            }
            GY->u128 = 0;
            break;
        case 0x5E:  /* VDIVSS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GX->f[0] = VX->f[0] / EX->f[0];
            if(GX!=VX) {
                GX->ud[1] = VX->ud[1];
                GX->q[1] = VX->q[1];
            }
            GY->u128 = 0;
            break;
        case 0x5F:  /* VMAXSS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if (VX->f[0] == 0.0 && EX->f[0]  == 0.0)
                GX->f[0] = EX->f[0];
            else if (isnan(VX->f[0]) || isnan(EX->f[0]) || isgreater(EX->f[0], VX->f[0]))
                GX->f[0] = EX->f[0];
            else
                GX->f[0] = VX->f[0];
            if(GX!=VX) {
                GX->ud[1] = VX->ud[1];
                GX->q[1] = VX->q[1];
            }
            GY->u128 = 0;
            break;

        case 0x6F:  // VMOVDQU Gx, Ex
            nextop = F8;
            GETEX(0);
            GETGX;
            memcpy(GX, EX, 16);    // unaligned...
            GETGY;
            if(vex.l) {
                GETEY;
                memcpy(GY, EY, 16);
            } else
                GY->u128 = 0;
            break;
        case 0x70:  /* VPSHUFHW Gx, Ex, Ib */
            nextop = F8;
            GETEX(1);
            GETGX; GETGY;
            tmp8u = F8;
            if(GX==EX) {
                for (int i=0; i<4; ++i)
                    eax1.uw[4+i] = EX->uw[4+((tmp8u>>(i*2))&3)];
                GX->q[1] = eax1.q[1];
            } else {
                for (int i=0; i<4; ++i)
                    GX->uw[4+i] = EX->uw[4+((tmp8u>>(i*2))&3)];
                GX->q[0] = EX->q[0];
            }
            if(vex.l) {
                GETEY;
                if(GY==EY) {
                    for (int i=0; i<4; ++i)
                        eay1.uw[4+i] = EY->uw[4+((tmp8u>>(i*2))&3)];
                    GY->q[1] = eay1.q[1];
                } else {
                    for (int i=0; i<4; ++i)
                        GY->uw[4+i] = EY->uw[4+((tmp8u>>(i*2))&3)];
                    GY->q[0] = EY->q[0];
                }
            } else
                GY->u128 = 0;
            break;

        case 0x7E:  /* MOVQ Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GX->q[0] = EX->q[0];
            GX->q[1] = 0;
            GETGY;
            GY->u128 = 0;
            break;
        case 0x7F:  /* VMOVDQU Ex, Gx */
            nextop = F8;
            GETEX(0);
            GETGX;
            memcpy(EX, GX, 16);    // unaligned...
            if(vex.l) {
                GETGY;
                GETEY;
                memcpy(EY, GY, 16);
            } // no ymm raz here it seems
            break;

        case 0xC2:  /* VCMPSS Gx, Vx, Ex, Ib */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETVX;
            GETGY;
            tmp8u = F8;
            tmp8s = 0;
            is_nan = isnan(VX->f[0]) || isnan(EX->f[0]);
            // the 1f..0f opcode are singaling/unsignaling, wich is not handled
            switch(tmp8u&0x0f) {
                case 0x00: tmp8s=(VX->f[0] == EX->f[0]) && !is_nan; break;
                case 0x01: tmp8s=isless(VX->f[0], EX->f[0]) && !is_nan; break;
                case 0x02: tmp8s=islessequal(VX->f[0], EX->f[0]) && !is_nan; break;
                case 0x03: tmp8s=is_nan; break;
                case 0x04: tmp8s=(VX->f[0] != EX->f[0]) || is_nan; break;
                case 0x05: tmp8s=is_nan || isgreaterequal(VX->f[0], EX->f[0]); break;
                case 0x06: tmp8s=is_nan || isgreater(VX->f[0], EX->f[0]); break;
                case 0x07: tmp8s=!is_nan; break;
                case 0x08: tmp8s=(VX->f[0] == EX->f[0]) || is_nan; break;
                case 0x09: tmp8s=isless(VX->f[0], EX->f[0]) || is_nan; break;
                case 0x0a: tmp8s=islessequal(VX->f[0], EX->f[0]) || is_nan; break;
                case 0x0b: tmp8s=0; break;
                case 0x0c: tmp8s=(VX->f[0] != EX->f[0]) && !is_nan; break;
                case 0x0d: tmp8s=isgreaterequal(VX->f[0], EX->f[0]) && !is_nan; break;
                case 0x0e: tmp8s=isgreater(VX->f[0], EX->f[0]) && !is_nan; break;
                case 0x0f: tmp8s=1; break;
            }
            GX->ud[0]=(tmp8s)?0xffffffff:0;
            if(GX!=VX) {
                GX->ud[1] = VX->ud[1];
                GX->q[1] = VX->q[1];
            }
            GY->u128 = 0;
            break;

        case 0xE6:  /* VCVTDQ2PD Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            if(vex.l) {
                GY->d[1] = EX->sd[3];
                GY->d[0] = EX->sd[2];
            } else
                GY->u128 = 0;
            GX->d[1] = EX->sd[1];
            GX->d[0] = EX->sd[0];
            break;

        default:
            return 0;
    }
    return addr;
}
