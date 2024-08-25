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

#ifdef TEST_INTERPRETER
uintptr_t TestAVX_F20F(x64test_t *test, vex_t vex, uintptr_t addr, int *step)
#else
uintptr_t RunAVX_F20F(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step)
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
    int64_t tmp64s0, tmp64s1;
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

        case 0x10:  /* VMOVSD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GX->q[0] = EX->q[0];
            if(MODREG) {
                GETVX;
                GX->q[1] = VX->q[1];
            } else {
                GX->q[1] = 0;
            }
            GETGY;
            GY->u128 = 0;
            break;
        case 0x11:  /* VMOVSD Ex, Vx, Gx */
            nextop = F8;
            GETEX(0);
            GETGX;
            EX->q[0] = GX->q[0];
            if(MODREG) {
                GETVX;
                EX->q[1] = VX->q[1];
                GETEY;
                EY->u128 = 0;
            }
            break;
        case 0x12:  /* VMOVDDUP Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            GX->q[1] = GX->q[0] = EX->q[0];
            if(vex.l) {
                GETEY;
                GY->q[1] = GY->q[0] = EY->q[0];
            } else
                GY->u128 = 0;
            break;

        case 0x2A:  /* VCVTSI2SD Gx, Vx, Ed */
            nextop = F8;
            GETED(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                GX->d[0] = ED->sq[0];
            } else {
                GX->d[0] = ED->sdword[0];
            }
            GX->q[1] = VX->q[1];
            GY->u128 = 0;
            break;

        case 0x2C:  /* VCVTTSD2SI Gd, Ex */
            nextop = F8;
            GETEX(0);
            GETGD;
            if(rex.w)
                if(isnan(EX->d[0]) || isinf(EX->d[0]) || EX->d[0]>(double)0x7fffffffffffffffLL)
                    GD->q[0] = 0x8000000000000000LL;
                else
                    GD->sq[0] = EX->d[0];
            else {
                if(isnan(EX->d[0]) || isinf(EX->d[0]) || EX->d[0]>0x7fffffff)
                    GD->dword[0] = 0x80000000;
                else
                    GD->sdword[0] = EX->d[0];
                GD->dword[1] = 0;
            }
            break;
        case 0x2D:  /* VCVTSD2SI Gd, Ex */
            nextop = F8;
            GETEX(0);
            GETGD;
            if(rex.w) {
                if(isnan(EX->d[0]) || isinf(EX->d[0]) || EX->d[0]>(double)0x7fffffffffffffffLL)
                    GD->q[0] = 0x8000000000000000LL;
                else
                    switch(emu->mxcsr.f.MXCSR_RC) {
                        case ROUND_Nearest: {
                            int round = fegetround();
                            fesetround(FE_TONEAREST);
                            GD->sq[0] = nearbyint(EX->d[0]);
                            fesetround(round);
                            break;
                        }
                        case ROUND_Down:
                            GD->sq[0] = floor(EX->d[0]);
                            break;
                        case ROUND_Up:
                            GD->sq[0] = ceil(EX->d[0]);
                            break;
                        case ROUND_Chop:
                            GD->sq[0] = EX->d[0];
                            break;
                    }
            } else {
                if(isnan(EX->d[0]) || isinf(EX->d[0]) || EX->d[0]>0x7fffffff)
                    GD->dword[0] = 0x80000000;
                else
                    switch(emu->mxcsr.f.MXCSR_RC) {
                        case ROUND_Nearest: {
                            int round = fegetround();
                            fesetround(FE_TONEAREST);
                            GD->sdword[0] = nearbyint(EX->d[0]);
                            fesetround(round);
                            break;
                        }
                        case ROUND_Down:
                            GD->sdword[0] = floor(EX->d[0]);
                            break;
                        case ROUND_Up:
                            GD->sdword[0] = ceil(EX->d[0]);
                            break;
                        case ROUND_Chop:
                            GD->sdword[0] = EX->d[0];
                            break;
                    }
                GD->dword[1] = 0;
            }
            break;

        case 0x51:  /* VSQRTSD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            if(EX->d[0]<0.0 )
                GX->d[0] = -NAN;
            else
                GX->d[0] = sqrt(EX->d[0]);
            GX->q[1] = VX->q[1];
            GY->u128 = 0;
            break;

        case 0x58:  /* VADDSD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GX->d[0] = VX->d[0] + EX->d[0];
            GX->q[1] = VX->q[1];
            GY->u128 = 0;
            break;
        case 0x59:  /* VMULSD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            #ifndef NOALIGN
                // mul generate a -NAN only if doing (+/-)inf * (+/-)0
                if((isinf(GX->d[0]) && EX->d[0]==0.0) || (isinf(EX->d[0]) && GX->d[0]==0.0))
                    GX->d[0] = -NAN;
                else
            #endif
            GX->d[0] = VX->d[0] * EX->d[0];
            if(GX!=VX) {
                GX->q[1] = VX->q[1];
            }
            GY->u128 = 0;
            break;
        case 0x5A:  /* VCVTSD2SS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GX->f[0] = EX->d[0];
            GX->ud[1] = VX->ud[1];
            GX->q[1] = VX->q[1];
            GY->u128 = 0;
            break;

        case 0x5C:  /* VSUBSD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GX->d[0] = VX->d[0] - EX->d[0];
            GX->q[1] = VX->q[1];
            GY->u128 = 0;
            break;
        case 0x5D:  /* VMINSD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if (VX->d[0] == 0.0 && EX->d[0]  == 0.0)
                GX->d[0] = EX->d[0];
            else if (isnan(VX->d[0]) || isnan(EX->d[0]) || isgreater(VX->d[0], EX->d[0]))
                GX->d[0] = EX->d[0];
            else
                GX->d[0] = VX->d[0];
            GX->q[1] = VX->q[1];
            GY->u128 = 0;
            break;
        case 0x5E:  /* VDIVSD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            #ifndef NOALIGN
            is_nan = isnan(VX->d[0]) || isnan(EX->d[0]);
            #endif
            GX->d[0] = VX->d[0] / EX->d[0];
            #ifndef NOALIGN
            if(!is_nan && isnan(GX->d[0]))
                GX->d[0] = -NAN;
            #endif
            GX->q[1] = VX->q[1];
            GY->u128 = 0;
            break;
        case 0x5F:  /* VMAXSD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if (VX->d[0] == 0.0 && EX->d[0]  == 0.0)
                GX->d[0] = EX->d[0];
            else if (isnan(VX->d[0]) || isnan(EX->d[0]) || isgreater(EX->d[0], VX->d[0]))
                GX->d[0] = EX->d[0];
            else
                GX->d[0] = VX->d[0];
            GX->q[1] = VX->q[1];
            GY->u128 = 0;
            break;

        case 0x70:  /* VPSHUFLW Gx, Ex, Ib */
            nextop = F8;
            GETEX(1);
            GETGX; GETGY;
            tmp8u = F8;
            if(GX==EX) {
                for (int i=0; i<4; ++i)
                    eax1.uw[i] = EX->uw[(tmp8u>>(i*2))&3];
                GX->q[0] = eax1.q[0];
            } else {
                for (int i=0; i<4; ++i)
                    GX->uw[i] = EX->uw[(tmp8u>>(i*2))&3];
                GX->q[1] = EX->q[1];
            }
            if(vex.l) {
                GETEY;
                if(GY==EY) {
                    for (int i=0; i<4; ++i)
                        eay1.uw[i] = EY->uw[(tmp8u>>(i*2))&3];
                    GY->q[0] = eay1.q[0];
                } else {
                    for (int i=0; i<4; ++i)
                        GY->uw[i] = EY->uw[(tmp8u>>(i*2))&3];
                    GY->q[1] = EY->q[1];
                }
            } else
                GY->u128 = 0;
            break;

        case 0x7C:  /* VHADDPS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GETEY;
            if(GX==EX) {
                eax1 = *EX;
                EX = &eax1;
            }
            GX->f[0] = VX->f[0] + VX->f[1];
            GX->f[1] = VX->f[2] + VX->f[3];
            if(EX==VX) {
                GX->f[2] = GX->f[0];
                GX->f[3] = GX->f[1];
            } else {
                GX->f[2] = EX->f[0] + EX->f[1];
                GX->f[3] = EX->f[2] + EX->f[3];
            }
            if(vex.l) {
                if(GY==EY) {
                    eay1 = *EY;
                    EY = &eay1;
                }
                GETVY;
                GY->f[0] = VY->f[0] + VY->f[1];
                GY->f[1] = VY->f[2] + VY->f[3];
                if(EY==VY) {
                    GY->f[2] = GY->f[0];
                    GY->f[3] = GY->f[1];
                } else {
                    GY->f[2] = EY->f[0] + EY->f[1];
                    GY->f[3] = EY->f[2] + EY->f[3];
                }
            } else
                GY->u128 = 0;
            break;
        case 0x7D:  /* VHSUBPS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GETEY;
            if(GX==EX) {
                eax1 = *EX;
                EX = &eax1;
            }
            GX->f[0] = VX->f[0] - VX->f[1];
            GX->f[1] = VX->f[2] - VX->f[3];
            if(EX==VX) {
                GX->f[2] = GX->f[0];
                GX->f[3] = GX->f[1];
            } else {
                GX->f[2] = EX->f[0] - EX->f[1];
                GX->f[3] = EX->f[2] - EX->f[3];
            }
            if(vex.l) {
                if(GY==EY) {
                    eay1 = *EY;
                    EY = &eay1;
                }
                GETVY;
                GY->f[0] = VY->f[0] - VY->f[1];
                GY->f[1] = VY->f[2] - VY->f[3];
                if(EY==VY) {
                    GY->f[2] = GY->f[0];
                    GY->f[3] = GY->f[1];
                } else {
                    GY->f[2] = EY->f[0] - EY->f[1];
                    GY->f[3] = EY->f[2] - EY->f[3];
                }
            } else
                GY->u128 = 0;
            break;

        case 0xC2:  /* VCMPSD Gx, Vx, Ex, Ib */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETVX;
            GETGY;
            tmp8u = F8;
            tmp8s = 0;
            int is_nan = isnan(VX->d[0]) || isnan(EX->d[0]);
            // the 1f..0f opcode are singaling/unsignaling, wich is not handled
            switch(tmp8u&0x0f) {
                case 0x00: tmp8s=(VX->d[0] == EX->d[0]) && !is_nan; break;
                case 0x01: tmp8s=isless(VX->d[0], EX->d[0]) && !is_nan; break;
                case 0x02: tmp8s=islessequal(VX->d[0], EX->d[0]) && !is_nan; break;
                case 0x03: tmp8s=is_nan; break;
                case 0x04: tmp8s=(VX->d[0] != EX->d[0]) || is_nan; break;
                case 0x05: tmp8s=is_nan || isgreaterequal(VX->d[0], EX->d[0]); break;
                case 0x06: tmp8s=is_nan || isgreater(VX->d[0], EX->d[0]); break;
                case 0x07: tmp8s=!is_nan; break;
                case 0x08: tmp8s=(VX->d[0] == EX->d[0]) || is_nan; break;
                case 0x09: tmp8s=isless(VX->d[0], EX->d[0]) || is_nan; break;
                case 0x0a: tmp8s=islessequal(VX->d[0], EX->d[0]) || is_nan; break;
                case 0x0b: tmp8s=0; break;
                case 0x0c: tmp8s=(VX->d[0] != EX->d[0]) && !is_nan; break;
                case 0x0d: tmp8s=isgreaterequal(VX->d[0], EX->d[0]) && !is_nan; break;
                case 0x0e: tmp8s=isgreater(VX->d[0], EX->d[0]) && !is_nan; break;
                case 0x0f: tmp8s=1; break;
            }
            GX->q[0]=(tmp8s)?0xffffffffffffffffLL:0LL;
            GX->q[1] = VX->q[1];
            GY->u128 = 0;
            break;

        case 0xD0:  /* VADDSUBPS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GX->f[0] = VX->f[0] - EX->f[0];
            GX->f[1] = VX->f[1] + EX->f[1];
            GX->f[2] = VX->f[2] - EX->f[2];
            GX->f[3] = VX->f[3] + EX->f[3];
            if(vex.l) {
                GETEY;
                GETVY;
                GY->f[0] = VY->f[0] - EY->f[0];
                GY->f[1] = VY->f[1] + EY->f[1];
                GY->f[2] = VY->f[2] - EY->f[2];
                GY->f[3] = VY->f[3] + EY->f[3];
            } else
                GY->u128 = 0;
            break;

        case 0xE6:  /* CVTPD2DQ Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            switch(emu->mxcsr.f.MXCSR_RC) {
                case ROUND_Nearest: {
                    int round = fegetround();
                    fesetround(FE_TONEAREST);
                    tmp64s0 = nearbyint(EX->d[0]);
                    tmp64s1 = nearbyint(EX->d[1]);
                    fesetround(round);
                    break;
                }
                case ROUND_Down:
                    tmp64s0 = floor(EX->d[0]);
                    tmp64s1 = floor(EX->d[1]);
                    break;
                case ROUND_Up:
                    tmp64s0 = ceil(EX->d[0]);
                    tmp64s1 = ceil(EX->d[1]);
                    break;
                case ROUND_Chop:
                    tmp64s0 = EX->d[0];
                    tmp64s1 = EX->d[1];
                    break;
            }
            if (tmp64s0==(int32_t)tmp64s0 && !isnan(EX->d[0])) {
                GX->sd[0] = (int32_t)tmp64s0;
            } else {
                GX->sd[0] = INT32_MIN;
            }
            if (tmp64s1==(int32_t)tmp64s1 && !isnan(EX->d[1])) {
                GX->sd[1] = (int32_t)tmp64s1;
            } else {
                GX->sd[1] = INT32_MIN;
            }
            if(vex.l) {
                GETEY;
                switch(emu->mxcsr.f.MXCSR_RC) {
                    case ROUND_Nearest: {
                        int round = fegetround();
                        fesetround(FE_TONEAREST);
                        tmp64s0 = nearbyint(EY->d[0]);
                        tmp64s1 = nearbyint(EY->d[1]);
                        fesetround(round);
                        break;
                    }
                    case ROUND_Down:
                        tmp64s0 = floor(EY->d[0]);
                        tmp64s1 = floor(EY->d[1]);
                        break;
                    case ROUND_Up:
                        tmp64s0 = ceil(EY->d[0]);
                        tmp64s1 = ceil(EY->d[1]);
                        break;
                    case ROUND_Chop:
                        tmp64s0 = EY->d[0];
                        tmp64s1 = EY->d[1];
                        break;
                }
                if (tmp64s0==(int32_t)tmp64s0 && !isnan(EY->d[0])) {
                    GX->sd[2] = (int32_t)tmp64s0;
                } else {
                    GX->sd[2] = INT32_MIN;
                }
                if (tmp64s1==(int32_t)tmp64s1 && !isnan(EY->d[1])) {
                    GX->sd[3] = (int32_t)tmp64s1;
                } else {
                    GX->sd[3] = INT32_MIN;
                }
            } else
                GX->q[1] = 0;
            GY->u128 = 0;
            break;

        case 0xF0:  /* VLDDQU Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            memcpy(GX, EX, 16);
            if(vex.l) {
                GETEY;
                memcpy(GY, EY, 16);
            } else
                GY->u128 = 0;
            break;

        default:
            return 0;
    }
    return addr;
}
