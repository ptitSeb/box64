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


#ifdef TEST_INTERPRETER
    x64emu_t *emu = test->emu;
#endif
    opcode = F8;

    rex_t rex = vex.rex;

    switch(opcode) {

        case 0x10:  /* VMOVSS Gx Ex */
            nextop = F8;
            GETEX(0);
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
            GY->q[0] = GY->q[1] = 0;
            break;
        case 0x11:  /* MOVSS Ex Gx */
            nextop = F8;
            GETEX(0);
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
                if(isnanf(EX->f[0]) || isinff(EX->f[0]) || EX->f[0]>0x7fffffff)
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
            GY->q[0] = GY->q[1] = 0;
            break;

        case 0x5A:  /* VCVTSS2SD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GX->d[0] = EX->f[0];
            GX->q[1] = VX->q[1];
            GY->q[0] = GY->q[1] = 0;
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
                GY->q[0] = GY->q[1] = 0;
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
            switch(tmp8u&7) {
                case 0: tmp8s=(VX->f[0] == EX->f[0]); break;
                case 1: tmp8s=isless(VX->f[0], EX->f[0]) && !(isnan(VX->f[0]) || isnan(EX->f[0])); break;
                case 2: tmp8s=islessequal(VX->f[0], EX->f[0]) && !(isnan(VX->f[0]) || isnan(EX->f[0])); break;
                case 3: tmp8s=isnan(VX->f[0]) || isnan(EX->f[0]); break;
                case 4: tmp8s=isnan(VX->f[0]) || isnan(EX->f[0]) || (VX->f[0] != EX->f[0]); break;
                case 5: tmp8s=isnan(VX->f[0]) || isnan(EX->f[0]) || isgreaterequal(VX->f[0], EX->f[0]); break;
                case 6: tmp8s=isnan(VX->f[0]) || isnan(EX->f[0]) || isgreater(VX->f[0], EX->f[0]); break;
                case 7: tmp8s=!isnan(VX->f[0]) && !isnan(EX->f[0]); break;
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
