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
uintptr_t TestAVX_660F(x64test_t *test, vex_t vex, uintptr_t addr, int *step)
#else
uintptr_t RunAVX_660F(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    uint8_t tmp8u, u8;
    int8_t tmp8s;
    int16_t tmp16s;
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

        case 0x10:                      /* MOVUPD Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            memcpy(GX, EX, 16); // unaligned...
            if(vex.l) {
                GETEY;
                memcpy(GY, EY, 16); // unaligned...
            } else
                GY->u128 = 0;
            break;
        case 0x11:                      /* MOVUPD Ex, Gx */
            nextop = F8;
            GETEX(0);
            GETGX;
            memcpy(EX, GX, 16); // unaligned...
            if(vex.l) {
                GETEY;
                GETGY;
                memcpy(EY, GY, 16); // unaligned...
            }
            break;
        case 0x12:                      /* VMOVLPD Gx, Vx, Eq */
            nextop = F8;
            GETE8(0);
            GETGX;
            GETVX;
            GETGY;
            GX->q[0] = ED->q[0];
            GX->q[1] = VX->q[1];
            GY->u128 = 0;
            break;
        case 0x13:                      /* VMOVLPD Eq, Gx */
            nextop = F8;
            GETE8(0);
            GETGX;
            ED->q[0] = GX->q[0];
            break;
        case 0x14:                      /* VUNPCKLPD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            GX->q[1] = EX->q[0];
            GX->q[0] = VX->q[0];
            if(vex.l) {
                GETEY; GETVY;
                GY->q[1] = EY->q[0];
                GY->q[0] = VY->q[0];
            } else
                GY->u128 = 0;
            break;
        case 0x15:                      /* VUNPCKHPD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            GX->q[0] = VX->q[1];
            GX->q[1] = EX->q[1];
            if(vex.l) {
                GETEY; GETVY;
                GY->q[0] = VY->q[1];
                GY->q[1] = EY->q[1];
            } else
                GY->u128 = 0;
            break;
        case 0x16:                      /* VMOVHPD Gx, Vx, Ed */
            nextop = F8;
            GETE8(0);
            GETGX;
            GETVX;
            GX->q[1] = ED->q[0];
            GX->q[0] = VX->q[0];
            GETGY;
            GY->u128 = 0;
            break;
        case 0x17:                      /* VMOVHPD Ed, Gx */
            nextop = F8;
            GETE8(0);
            GETGX;
            ED->q[0] = GX->q[1];
            break;

        case 0x28:                      /* VMOVAPD Gx, Ex */
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
            } else
                GY->u128 = 0;
            break;
        case 0x29:                      /* VMOVAPD Ex, Gx */
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
            }
            break;

        case 0x2B:                      /* MOVNTPD Ex, Gx */
            nextop = F8;
            GETEX(0);
            GETGX;
            EX->q[0] = GX->q[0];
            EX->q[1] = GX->q[1];
            if(vex.l) {
                GETGY;
                GETEY;
                EY->q[0] = GY->q[0];
                EY->q[1] = GY->q[1];
            }
            break;

        case 0x2E:                      /* VUCOMISD Gx, Ex */
        case 0x2F:                      /* VCOMISD Gx, Ex */
            RESET_FLAGS(emu);
            nextop = F8;
            GETEX(0);
            GETGX;
            if(isnan(GX->d[0]) || isnan(EX->d[0])) {
                SET_FLAG(F_ZF); SET_FLAG(F_PF); SET_FLAG(F_CF);
            } else if(isgreater(GX->d[0], EX->d[0])) {
                CLEAR_FLAG(F_ZF); CLEAR_FLAG(F_PF); CLEAR_FLAG(F_CF);
            } else if(isless(GX->d[0], EX->d[0])) {
                CLEAR_FLAG(F_ZF); CLEAR_FLAG(F_PF); SET_FLAG(F_CF);
            } else {
                SET_FLAG(F_ZF); CLEAR_FLAG(F_PF); CLEAR_FLAG(F_CF);
            }
            CLEAR_FLAG(F_OF); CLEAR_FLAG(F_AF); CLEAR_FLAG(F_SF);
            break;

        case 0x50:  /* VMOVMSKPD Gd, Ex */
            nextop = F8;
            GETEX(0);
            GETGD;
            GD->q[0] = 0;
            for(int i=0; i<2; ++i)
                GD->dword[0] |= ((EX->q[i]>>63)&1)<<i;
            if(vex.l) {
                GETEY;
                for(int i=0; i<2; ++i)
                    GD->dword[0] |= ((EY->q[i]>>63)&1)<<(i+2);
            }
            break;
        case 0x51:                      /* VSQRTPD Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETGY;
            for (int i=0; i<2; ++i) {
                #ifndef NOALIGN
                if(EX->d[i]<0.0)        // on x86, default nan are negative
                    GX->d[i] = -NAN;    // but input NAN are not touched (so sqrt(+nan) -> +nan)
                else
                #endif
                GX->d[i] = sqrt(EX->d[i]);
            }
            if(vex.l) {
                GETEY;
                for (int i=0; i<2; ++i) {
                    #ifndef NOALIGN
                    if(EY->d[i]<0.0)
                        GY->d[i] = -NAN;
                    else
                    #endif
                    GY->d[i] = sqrt(EY->d[i]);
                }
            } else
                GY->u128 = 0;
            break;

        case 0x54:  /* VANDPD Gx, Vx, Ex */
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
        case 0x55:  /* VANDNPD Gx, Vx, Ex */
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
        case 0x56:  /* VORPD Gx, Vx, Ex */
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
        case 0x57:  /* VXORPD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GX->u128 = VX->u128 ^ EX->u128;
            GETGY;
            if(vex.l) {
                GETEY;
                GETVY;
                GY->u128 = VY->u128 ^ EY->u128;
            } else {
                GY->u128 = 0;
            }
            break;
        case 0x58:  /* VADDPD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GX->d[0] = VX->d[0] + EX->d[0];
            GX->d[1] = VX->d[1] + EX->d[1];
            GETGY;
            if(vex.l) {
                GETEY;
                GETVY;
                GY->d[0] = VY->d[0] + EY->d[0];
                GY->d[1] = VY->d[1] + EY->d[1];
            } else {
                GY->u128 = 0;
            }
            break;
        case 0x59:  /* MULPD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<2; ++i) {
                #ifndef NOALIGN
                    // mul generate a -NAN only if doing (+/-)inf * (+/-)0
                    if((isinf(VX->d[i]) && EX->d[i]==0.0) || (isinf(EX->d[i]) && VX->d[i]==0.0))
                        GX->d[i] = -NAN;
                    else
                #endif
                GX->d[i] = VX->d[i] * EX->d[i];
            }
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<2; ++i) {
                    #ifndef NOALIGN
                        // mul generate a -NAN only if doing (+/-)inf * (+/-)0
                        if((isinf(VY->d[i]) && EY->d[i]==0.0) || (isinf(EY->d[i]) && VY->d[i]==0.0))
                            GY->d[i] = -NAN;
                        else
                    #endif
                    GY->d[i] = VY->d[i] * EY->d[i];
                }
            } else
                GY->u128 = 0;
            break;
        case 0x5A:      /* VCVTPD2PS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            GX->f[0] = EX->d[0];
            GX->f[1] = EX->d[1];
            if(vex.l) {
                GETEY;
                GX->f[2] = EY->d[0];
                GX->f[3] = EY->d[1];
            } else
                GX->q[1] = 0;
            GY->u128 = 0;
            break;
        case 0x5B:      /* VCVTPS2DQ Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            for(int i=0; i<4; ++i) {
                if(isnanf(EX->f[i]))
                    tmp64s = INT32_MIN;
                else
                    switch(emu->mxcsr.f.MXCSR_RC) {
                        case ROUND_Nearest: {
                            int round = fegetround();
                            fesetround(FE_TONEAREST);
                            tmp64s = nearbyintf(EX->f[i]);
                            fesetround(round);
                            break;
                        }
                        case ROUND_Down:
                            tmp64s = floorf(EX->f[i]);
                            break;
                        case ROUND_Up:
                            tmp64s = ceilf(EX->f[i]);
                            break;
                        case ROUND_Chop:
                            tmp64s = EX->f[i];
                            break;
                    }
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
                        switch(emu->mxcsr.f.MXCSR_RC) {
                            case ROUND_Nearest: {
                                int round = fegetround();
                                fesetround(FE_TONEAREST);
                                tmp64s = nearbyintf(EY->f[i]);
                                fesetround(round);
                                break;
                            }
                            case ROUND_Down:
                                tmp64s = floorf(EY->f[i]);
                                break;
                            case ROUND_Up:
                                tmp64s = ceilf(EY->f[i]);
                                break;
                            case ROUND_Chop:
                                tmp64s = EY->f[i];
                                break;
                        }
                    if (tmp64s==(int32_t)tmp64s) {
                        GY->sd[i] = (int32_t)tmp64s;
                    } else {
                        GY->sd[i] = INT32_MIN;
                    }
                }
            } else
                GY->u128 = 0;
            break;
        case 0x5C:  /* VSUBPD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GX->d[0] = VX->d[0] - EX->d[0];
            GX->d[1] = VX->d[1] - EX->d[1];
            GETGY;
            if(vex.l) {
                GETEY;
                GETVY;
                GY->d[0] = VY->d[0] - EY->d[0];
                GY->d[1] = VY->d[1] - EY->d[1];
            } else {
                GY->u128 = 0;
            }
            break;
        case 0x5D:                      /* VMINPD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<2; ++i)
                if (isnan(VX->d[i]) || isnan(EX->d[i]) || isgreater(VX->d[i], EX->d[i]))
                    GX->d[i] = EX->d[i];
                else
                    GX->d[i] = VX->d[i];
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<2; ++i)
                    if (isnan(VY->d[i]) || isnan(EY->d[i]) || isgreater(VY->d[i], EY->d[i]))
                        GY->d[i] = EY->d[i];
                    else
                        GY->d[i] = VY->d[i];
            } else
                GY->u128 = 0;
            break;
        case 0x5E:  /* VDIVPD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for (int i=0; i<2; ++i) {
                #ifndef NOALIGN
                is_nan = isnan(VX->d[i]) || isnan(EX->d[i]);
                #endif
                GX->d[i] = VX->d[i] / EX->d[i];
                #ifndef NOALIGN
                if(!is_nan && isnan(GX->d[i]))
                    GX->d[i] = -NAN;
                #endif
            }
            if(vex.l) {
                GETEY;
                GETVY;
                for (int i=0; i<2; ++i) {
                    #ifndef NOALIGN
                    is_nan = isnan(VY->d[i]) || isnan(EY->d[i]);
                    #endif
                    GY->d[i] = VY->d[i] / EY->d[i];
                    #ifndef NOALIGN
                    if(!is_nan && isnan(GY->d[i]))
                        GY->d[i] = -NAN;
                    #endif
                }
            } else
                GY->u128 = 0;
            break;
        case 0x5F:                      /* VMAXPD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<2; ++i)
                if (isnan(VX->d[i]) || isnan(EX->d[i]) || isgreater(EX->d[i], VX->d[i]))
                    GX->d[i] = EX->d[i];
                else
                    GX->d[i] = VX->d[i];
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<2; ++i)
                    if (isnan(VY->d[i]) || isnan(EY->d[i]) || isgreater(EY->d[i], VY->d[i]))
                        GY->d[i] = EY->d[i];
                    else
                        GY->d[i] = VY->d[i];
            } else
                GY->u128 = 0;
            break;
        case 0x60:  /* VPUNPCKLBW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX;
            GETGY;
            for(int i=7; i>=0; --i) {
                GX->ub[2 * i + 1] = EX->ub[i];
                GX->ub[2 * i] = VX->ub[i];
            }
            if(vex.l) {
                GETEY; GETVY;
                for(int i=7; i>=0; --i) {
                    GY->ub[2 * i + 1] = EY->ub[i];
                    GY->ub[2 * i] = VY->ub[i];
                }
            } else
                GY->u128 = 0;
            break;
        case 0x61:  /* VPUNPCKLWD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            for(int i=3; i>=0; --i) {
                GX->uw[2 * i + 1] = EX->uw[i];
                GX->uw[2 * i] = VX->uw[i];
            }
            if(vex.l) {
                GETEY; GETVY;
                for(int i=3; i>=0; --i) {
                    GY->uw[2 * i + 1] = EY->uw[i];
                    GY->uw[2 * i] = VY->uw[i];
                }
            } else
                GY->u128 = 0;
            break;
        case 0x62:  /* VPUNPCKLDQ Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            for(int i=1; i>=0; --i) {
                GX->ud[2 * i + 1] = EX->ud[i];
                GX->ud[2 * i] = VX->ud[i];
            }
            if(vex.l) {
                GETEY; GETVY;
                for(int i=1; i>=0; --i) {
                    GY->ud[2 * i + 1] = EY->ud[i];
                    GY->ud[2 * i] = VY->ud[i];
                }
            } else
                GY->u128 = 0;
            break;
        case 0x63:  /* VPACKSSWB Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GETEY;
            GETVY;
            if(GX==EX) {
                eax1 = *EX;
                EX = &eax1;
            }
            if(VX==EX) {
                for(int i=0; i<8; ++i)
                    GX->sb[i] = (EX->sw[i]<-128)?-128:((EX->sw[i]>127)?127:EX->sw[i]);
                GX->q[1] = GX->q[0];
            } else {
                for(int i=0; i<8; ++i)
                    GX->sb[i] = (VX->sw[i]<-128)?-128:((VX->sw[i]>127)?127:VX->sw[i]);
                for(int i=0; i<8; ++i)
                    GX->sb[8+i] = (EX->sw[i]<-128)?-128:((EX->sw[i]>127)?127:EX->sw[i]);
            }
            if(vex.l) {
                if(GY==EY) {
                    eay1 = *EY;
                    EY = &eay1;
                }
                if(VY==EY) {
                    for(int i=0; i<8; ++i)
                        GY->sb[i] = (EY->sw[i]<-128)?-128:((EY->sw[i]>127)?127:EY->sw[i]);
                    GY->q[1] = GY->q[0];
                } else {
                    for(int i=0; i<8; ++i)
                        GY->sb[i] = (VY->sw[i]<-128)?-128:((VY->sw[i]>127)?127:VY->sw[i]);
                    for(int i=0; i<8; ++i)
                        GY->sb[8+i] = (EY->sw[i]<-128)?-128:((EY->sw[i]>127)?127:EY->sw[i]);
                }
            } else
                GY->u128 = 0;
            break;
        case 0x64:  /* VPCMPGTB Gx,Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<16; ++i)
                GX->ub[i] = (VX->sb[i]>EX->sb[i])?0xFF:0x00;
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<16; ++i)
                    GY->ub[i] = (VY->sb[i]>EY->sb[i])?0xFF:0x00;
            } else
                GY->u128 = 0;
            break;
        case 0x65:  /* VPCMPGTW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<8; ++i)
                GX->uw[i] = (VX->sw[i]>EX->sw[i])?0xFFFF:0x0000;
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<8; ++i)
                    GY->uw[i] = (VY->sw[i]>EY->sw[i])?0xFFFF:0x0000;
            } else
                GY->u128 = 0;
            break;
        case 0x66:  /* VPCMPGTD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<4; ++i)
                GX->ud[i] = (VX->sd[i]>EX->sd[i])?0xFFFFFFFF:0x00000000;
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<4; ++i)
                    GY->ud[i] = (VY->sd[i]>EY->sd[i])?0xFFFFFFFF:0x00000000;
            } else
                GY->u128 = 0;
            break;
        case 0x67:  /* VPACKUSWB Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GETEY;
            GETVY;
            if(GX==EX) {
                eax1 = *EX;
                EX = &eax1;
            }
            if(VX==EX) {
                for(int i=0; i<8; ++i)
                    GX->ub[i] = (EX->sw[i]<0)?0:((EX->sw[i]>0xff)?0xff:EX->sw[i]);
                GX->q[1] = GX->q[0];
            } else {
                for(int i=0; i<8; ++i)
                    GX->ub[i] = (VX->sw[i]<0)?0:((VX->sw[i]>0xff)?0xff:VX->sw[i]);
                for(int i=0; i<8; ++i)
                    GX->ub[8+i] = (EX->sw[i]<0)?0:((EX->sw[i]>0xff)?0xff:EX->sw[i]);
            }
            if(vex.l) {
                if(GY==EY) {
                    eay1 = *EY;
                    EY = &eay1;
                }
                if(VY==EY) {
                    for(int i=0; i<8; ++i)
                        GY->ub[i] = (EY->sw[i]<0)?0:((EY->sw[i]>0xff)?0xff:EY->sw[i]);
                    GY->q[1] = GY->q[0];
                } else {
                    for(int i=0; i<8; ++i)
                        GY->ub[i] = (VY->sw[i]<0)?0:((VY->sw[i]>0xff)?0xff:VY->sw[i]);
                    for(int i=0; i<8; ++i)
                        GY->ub[8+i] = (EY->sw[i]<0)?0:((EY->sw[i]>0xff)?0xff:EY->sw[i]);
                }
            } else
                GY->u128 = 0;
            break;
        case 0x68:  /* VPUNPCKHBW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; 
            GETGY;
            for(int i=0; i<8; ++i) {
                GX->ub[2 * i] = VX->ub[i + 8];
                GX->ub[2 * i + 1] = EX->ub[i + 8];
            }
            if(vex.l) {
                GETVY; GETEY;
                for(int i=0; i<8; ++i) {
                    GY->ub[2 * i] = VY->ub[i + 8];
                    GY->ub[2 * i + 1] = EY->ub[i + 8];
                }
            } else
                GY->u128 = 0;
            break;
        case 0x69:  /* VPUNPCKHWD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; 
            GETGY;
            for(int i=0; i<4; ++i) {
                GX->uw[2 * i] = VX->uw[i + 4];
                GX->uw[2 * i + 1] = EX->uw[i + 4];
            }
            if(vex.l) {
                GETVY; GETEY;
                for(int i=0; i<4; ++i) {
                    GY->uw[2 * i] = VY->uw[i + 4];
                    GY->uw[2 * i + 1] = EY->uw[i + 4];
                }
            } else
                GY->u128 = 0;
            break;
        case 0x6A:  /* VPUNPCKHDQ Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX;
            GETGY;
            for(int i=0; i<2; ++i) {
                GX->ud[2 * i] = VX->ud[i + 2];
                GX->ud[2 * i + 1] = EX->ud[i + 2];
            }
            if(vex.l) {
                GETVY; GETEY;
                for(int i=0; i<2; ++i) {
                    GY->ud[2 * i] = VY->ud[i + 2];
                    GY->ud[2 * i + 1] = EY->ud[i + 2];
                }
            } else
                GY->u128 = 0;
            break;
        case 0x6B:  /* VPACKSSDW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GETEY;
            GETVY;
            if(GX==EX) {
                eax1 = *EX;
                EX = &eax1;
            }
            for(int i=0; i<4; ++i)
                GX->sw[i] = (VX->sd[i]<-32768)?-32768:((VX->sd[i]>32767)?32767:VX->sd[i]);
            if(VX==EX)
                GX->q[1] = GX->q[0];
            else
                for(int i=0; i<4; ++i)
                    GX->sw[4+i] = (EX->sd[i]<-32768)?-32768:((EX->sd[i]>32767)?32767:EX->sd[i]);
            if(vex.l) {
                if(GY==EY) {
                    eay1 = *EY;
                    EY = &eay1;
                }
                for(int i=0; i<4; ++i)
                    GY->sw[i] = (VY->sd[i]<-32768)?-32768:((VY->sd[i]>32767)?32767:VY->sd[i]);
                if(VY==EY)
                    GY->q[1] = GY->q[0];
                else
                    for(int i=0; i<4; ++i)
                        GY->sw[4+i] = (EY->sd[i]<-32768)?-32768:((EY->sd[i]>32767)?32767:EY->sd[i]);
            } else
                GY->u128 = 0;
            break;
        case 0x6C:  /* VPUNPCKLQDQ Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GX->q[1] = EX->q[0];
            GX->q[0] = VX->q[0];
            if(vex.l) {
                GETEY;
                GETVY;
                GY->q[1] = EY->q[0];
                GY->q[0] = VY->q[0];
            } else
                GY->u128 = 0;
            break;
        case 0x6D:  /* VPUNPCKHQDQ Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GX->q[0] = VX->q[1];
            GX->q[1] = EX->q[1];
            if(vex.l) {
                GETEY;
                GETVY;
                GY->q[0] = VY->q[1];
                GY->q[1] = EY->q[1];
            } else
                GY->u128 = 0;
            break;
        case 0x6E:                      /* VMOVD GX, Ed */
            nextop = F8;
            GETED(0);
            GETGX;
            if(rex.w)
                GX->q[0] = ED->q[0];
            else
                GX->q[0] = ED->dword[0];    // zero extended
            GX->q[1] = 0;
            GETGY;
            GY->u128 = 0;
            break;
        case 0x6F:  // VMOVDQA GX, EX
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
            }   else
                GY->u128 = 0;
            break;
        case 0x70:  /* VPSHUFD Gx,Ex,Ib */
            nextop = F8;
            // do not use vex.v
            GETEX(1);
            GETGX;
            GETGY;
            tmp8u = F8;
            if(vex.l) {
                GETEY;
                if(EY==GY) {eay1 = *GY; EY = &eay1;}   // copy is needed
                for (int i=0; i<4; ++i)
                    GY->ud[i] = EY->ud[(tmp8u>>(i*2))&3];
            } else 
                memset(GY, 0, 16);
            if(EX==GX) {eax1 = *GX; EX = &eax1;}   // copy is needed
            for (int i=0; i<4; ++i)
                GX->ud[i] = EX->ud[(tmp8u>>(i*2))&3];
            break;
        case 0x71:  /* GRP */
            nextop = F8;
            GETEX(1);
            GETVX;
            GETVY;
            if(!vex.l) VY->u128 = 0;
            switch((nextop>>3)&7) {
                case 2:                 /* VPSRLW Vx, Ex, Ib */
                    tmp8u = F8;
                    if(tmp8u>15) VX->u128 = 0;
                    else
                        for (int i=0; i<8; ++i) VX->uw[i] = EX->uw[i] >> tmp8u;
                    if(vex.l) {
                        GETEY;
                        if(tmp8u>15) VY->u128 = 0;
                        else
                            for (int i=0; i<8; ++i) VY->uw[i] = EY->uw[i] >> tmp8u;
                    }
                    break;
                case 4:                 /* VPSRAW Vx, Ex, Ib */
                    tmp8u = F8;
                    if(tmp8u>15) tmp8u=15;
                    for (int i=0; i<8; ++i) VX->sw[i] = EX->sw[i] >> tmp8u;
                    if(vex.l) {
                        GETEY;
                        for (int i=0; i<8; ++i) VY->sw[i] = EY->sw[i] >> tmp8u;
                    }
                    break;
                case 6:                 /* VPSLLW Vx, Ex, Ib */
                    tmp8u = F8;
                    if(tmp8u>15) VX->u128 = 0;
                    else
                        for (int i=0; i<8; ++i) VX->uw[i] = EX->uw[i] << tmp8u;
                    if(vex.l) {
                        GETEY;
                        if(tmp8u>15) VY->u128 = 0;
                        else
                            for (int i=0; i<8; ++i) VY->uw[i] = EY->uw[i] << tmp8u;
                    }
                    break;
                default:
                    return 0;
            }
            break;
        case 0x72:  /* GRP */
            nextop = F8;
            GETEX(1);
            GETVX;
            GETVY;
            if(!vex.l) VY->u128 = 0;
            switch((nextop>>3)&7) {
                case 2:                 /* VPSRLD Vx, Ex, Ib */
                    tmp8u = F8;
                    if(tmp8u>31) VX->u128 = 0;
                    else
                        for (int i=0; i<4; ++i) VX->ud[i] = EX->ud[i] >> tmp8u;
                    if(vex.l) {
                        GETEY;
                        if(tmp8u>31) VY->u128 = 0;
                        else
                            for (int i=0; i<4; ++i) VY->ud[i] = EY->ud[i] >> tmp8u;
                    }
                    break;
                case 4:                 /* VPSRAD Vx, Ex, Ib */
                    tmp8u = F8;
                    if(tmp8u>31) tmp8u=31;
                    for (int i=0; i<4; ++i) VX->sd[i] = EX->sd[i] >> tmp8u;
                    if(vex.l) {
                        GETEY;
                        for (int i=0; i<4; ++i) VY->sd[i] = EY->sd[i] >> tmp8u;
                    }
                    break;
                case 6:                 /* VPSLLD Vx, Ex, Ib */
                    tmp8u = F8;
                    if(tmp8u>31) VX->u128 = 0;
                    else
                        for (int i=0; i<4; ++i) VX->ud[i] = EX->ud[i] << tmp8u;
                    if(vex.l) {
                        GETEY;
                        if(tmp8u>31) VY->u128 = 0;
                        else
                            for (int i=0; i<4; ++i) VY->ud[i] = EY->ud[i] << tmp8u;
                    }
                    break;
                default:
                    return 0;
            }
            break;
        case 0x73:  /* GRP */
            nextop = F8;
            GETEX(1);
            GETVX;
            GETVY;
            if(!vex.l) VY->u128 = 0;
            switch((nextop>>3)&7) {
                case 2:                 /* VPSRLQ Vx, Ex, Ib */
                    tmp8u = F8;
                    if(tmp8u>63) VX->u128 = 0;
                    else
                        {VX->q[0] = EX->q[0] >> tmp8u; VX->q[1] = EX->q[1] >> tmp8u;}
                    if(vex.l) {
                        GETEY;
                        if(tmp8u>63) VY->u128 = 0;
                        else
                            {VY->q[0] = EY->q[0] >> tmp8u; VY->q[1] = EY->q[1] >> tmp8u;}
                    }
                    break;
                case 3:                 /* VPSRLDQ Vx, Ex, Ib */
                    tmp8u = F8;
                    if(tmp8u>15) VX->u128 = 0;
                    else if (tmp8u!=0) {
                        u8=tmp8u*8;
                        if (u8 < 64) {
                            VX->q[0] = (EX->q[0] >> u8) | (EX->q[1] << (64 - u8));
                            VX->q[1] = (EX->q[1] >> u8);
                        } else {
                            VX->q[0] = EX->q[1] >> (u8 - 64);
                            VX->q[1] = 0;
                        }
                    } else VX->u128 = EX->u128;
                    if(vex.l) {
                        GETEY;
                        if(tmp8u>15) VY->u128 = 0;
                        else if (tmp8u!=0) {
                            u8=tmp8u*8;
                            if (u8 < 64) {
                                VY->q[0] = (EY->q[0] >> u8) | (EY->q[1] << (64 - u8));
                                VY->q[1] = (EY->q[1] >> u8);
                            } else {
                                VY->q[0] = EY->q[1] >> (u8 - 64);
                                VY->q[1] = 0;
                            }
                        } else VY->u128 = EY->u128;
                    }
                    break;
                case 6:                 /* VPSLLQ Vx, Ex, Ib */
                    tmp8u = F8;
                    if(tmp8u>63) VX->u128 = 0;
                    else
                        {VX->q[0] = EX->q[0] << tmp8u; VX->q[1] = EX->q[1] << tmp8u;}
                    if(vex.l) {
                        GETEY;
                        if(tmp8u>63) VY->u128 = 0;
                        else
                            {VY->q[0] = EY->q[0] << tmp8u; VY->q[1] = EY->q[1] << tmp8u;}
                    }
                    break;
                case 7:                 /* VPSLLDQ Vx, Ex, Ib */
                    tmp8u = F8;
                    if(tmp8u>15) VX->u128 = 0;
                    else if (tmp8u!=0) {
                        u8=tmp8u<<3;
                        if (u8 < 64) {
                            VX->q[1] = (EX->q[1] << u8) | (EX->q[0] >> (64 - u8));
                            VX->q[0] = (EX->q[0] << u8);
                        } else {
                            VX->q[1] = EX->q[0] << (u8 - 64);
                            VX->q[0] = 0;
                        }
                    } else
                        VX->u128 = EX->u128;
                    if(vex.l) {
                        GETEY;
                        if(tmp8u>15) VY->u128 = 0;
                        else if (tmp8u!=0) {
                            u8=tmp8u<<3;
                            if (u8 < 64) {
                                VY->q[1] = (EY->q[1] << u8) | (EY->q[0] >> (64 - u8));
                                VY->q[0] = (EY->q[0] << u8);
                            } else {
                                VY->q[1] = EY->q[0] << (u8 - 64);
                                VY->q[0] = 0;
                            }
                        } else
                            VY->u128 = EY->u128;
                    }
                    break;
                default:
                    return 0;
            }
            break;
        case 0x74:  /* VPCMPEQB Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for (int i=0; i<16; ++i)
                GX->ub[i] = (VX->ub[i]==EX->ub[i])?0xff:0;
            if(vex.l) {
                GETEY;
                GETVY;
                for (int i=0; i<16; ++i)
                    GY->ub[i] = (VY->ub[i]==EY->ub[i])?0xff:0;
            } else
                GY->u128 = 0;
            break;
        case 0x75:  /* VPCMPEQW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for (int i=0; i<8; ++i)
                GX->uw[i] = (VX->uw[i]==EX->uw[i])?0xffff:0;
            if(vex.l) {
                GETEY;
                GETVY;
                for (int i=0; i<8; ++i)
                    GY->uw[i] = (VY->uw[i]==EY->uw[i])?0xffff:0;
            } else
                GY->u128 = 0;
            break;
        case 0x76:  /* VPCMPEQD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for (int i=0; i<4; ++i)
                GX->ud[i] = (VX->ud[i]==EX->ud[i])?0xffffffff:0;
            if(vex.l) {
                GETEY;
                GETVY;
                for (int i=0; i<4; ++i)
                    GY->ud[i] = (VY->ud[i]==EY->ud[i])?0xffffffff:0;
            } else
                GY->u128 = 0;
            break;

        case 0x7C:  /* VHADDPD Gx, Vx, Ex */
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
            #ifndef NOALIGN
            is_nan = isnan(VX->d[0]) || isnan(VX->d[1]);
            #endif
            GX->d[0] = VX->d[0] + VX->d[1];
            #ifndef NOALIGN
            if(!is_nan && isnan(GX->d[0]))
                GX->d[0] = -NAN;
            #endif
            if(EX==VX) {
                GX->d[1] = GX->d[0];
            } else {
                #ifndef NOALIGN
                is_nan = isnan(EX->d[0]) || isnan(EX->d[1]);
                #endif
                GX->d[1] = EX->d[0] + EX->d[1];
                #ifndef NOALIGN
                if(!is_nan && isnan(GX->d[1]))
                    GX->d[1] = -NAN;
                #endif
            }
            if(vex.l) {
                if(GY==EY) {
                    eay1 = *EY;
                    EY = &eay1;
                }
                GETVY;
                #ifndef NOALIGN
                is_nan = isnan(VY->d[0]) || isnan(VY->d[1]);
                #endif
                GY->d[0] = VY->d[0] + VY->d[1];
                #ifndef NOALIGN
                if(!is_nan && isnan(GY->d[0]))
                    GY->d[0] = -NAN;
                #endif
                if(EY==VY) {
                    GY->d[1] = GY->d[0];
                } else {
                    #ifndef NOALIGN
                    is_nan = isnan(EY->d[0]) || isnan(EY->d[1]);
                    #endif
                    GY->d[1] = EY->d[0] + EY->d[1];
                    #ifndef NOALIGN
                    if(!is_nan && isnan(GY->d[1]))
                        GY->d[1] = -NAN;
                    #endif
                }
            } else
                GY->u128 = 0;
            break;
        case 0x7D:  /* VHSUBPD Gx, Vx, Ex */
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
            #ifndef NOALIGN
            is_nan = isnan(VX->d[0]) || isnan(VX->d[1]);
            #endif
            GX->d[0] = VX->d[0] - VX->d[1];
            #ifndef NOALIGN
            if(!is_nan && isnan(GX->d[0]))
                GX->d[0] = -NAN;
            #endif
            if(EX==VX) {
                GX->d[1] = GX->d[0];
            } else {
                #ifndef NOALIGN
                is_nan = isnan(EX->d[0]) || isnan(EX->d[1]);
                #endif
                GX->d[1] = EX->d[0] - EX->d[1];
                #ifndef NOALIGN
                if(!is_nan && isnan(GX->d[1]))
                    GX->d[1] = -NAN;
                #endif
            }
            if(vex.l) {
                if(GY==EY) {
                    eay1 = *EY;
                    EY = &eay1;
                }
                GETVY;
                #ifndef NOALIGN
                is_nan = isnan(VY->d[0]) || isnan(VY->d[1]);
                #endif
                GY->d[0] = VY->d[0] - VY->d[1];
                #ifndef NOALIGN
                if(!is_nan && isnan(GY->d[0]))
                    GY->d[0] = -NAN;
                #endif
                if(EY==VY) {
                    GY->d[1] = GY->d[0];
                } else {
                    #ifndef NOALIGN
                    is_nan = isnan(EY->d[0]) || isnan(EY->d[1]);
                    #endif
                    GY->d[1] = EY->d[0] - EY->d[1];
                    #ifndef NOALIGN
                    if(!is_nan && isnan(GY->d[1]))
                        GY->d[1] = -NAN;
                    #endif
                }
            } else
                GY->u128 = 0;
            break;
        case 0x7E:                       /* VMOVD Ed, Gx */
            nextop = F8;
            GETED(0);
            GETGX;
            if(rex.w)
                ED->q[0] = GX->q[0];
            else {
                if(MODREG)
                    ED->q[0] = GX->ud[0];
                else
                    ED->dword[0] = GX->ud[0];
            }
            break;
        case 0x7F:  // VMOVDQA EX, GX
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            EX->q[0] = GX->q[0];
            EX->q[1] = GX->q[1];
            if(vex.l) {
                GETEY;
                EY->q[0] = GY->q[0];
                EY->q[1] = GY->q[1];
            } // no upper raz?
            break;

        case 0xC2:                      /* VCMPPD Gx, Vx, Ex, Ib */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETVX;
            GETGY;
            tmp8u = F8;
            for(int i=0; i<2; ++i) {
                tmp8s = 0;
                int is_nan = isnan(VX->d[i]) || isnan(EX->d[i]);
                // the 1f..0f opcode are singaling/unsignaling, wich is not handled
                switch(tmp8u&0x0f) {
                    case 0x00: tmp8s=(VX->d[i] == EX->d[i]) && !is_nan; break;
                    case 0x01: tmp8s=isless(VX->d[i], EX->d[i]) && !is_nan; break;
                    case 0x02: tmp8s=islessequal(VX->d[i], EX->d[i]) && !is_nan; break;
                    case 0x03: tmp8s=is_nan; break;
                    case 0x04: tmp8s=(VX->d[i] != EX->d[i]) || is_nan; break;
                    case 0x05: tmp8s=is_nan || isgreaterequal(VX->d[i], EX->d[i]); break;
                    case 0x06: tmp8s=is_nan || isgreater(VX->d[i], EX->d[i]); break;
                    case 0x07: tmp8s=!is_nan; break;
                    case 0x08: tmp8s=(VX->d[i] == EX->d[i]) || is_nan; break;
                    case 0x09: tmp8s=isless(VX->d[i], EX->d[i]) || is_nan; break;
                    case 0x0a: tmp8s=islessequal(VX->d[i], EX->d[i]) || is_nan; break;
                    case 0x0b: tmp8s=0; break;
                    case 0x0c: tmp8s=(VX->d[i] != EX->d[i]) && !is_nan; break;
                    case 0x0d: tmp8s=isgreaterequal(VX->d[i], EX->d[i]) && !is_nan; break;
                    case 0x0e: tmp8s=isgreater(VX->d[i], EX->d[i]) && !is_nan; break;
                    case 0x0f: tmp8s=1; break;
                }
                GX->q[i]=(tmp8s)?0xffffffffffffffffLL:0LL;
            }
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<2; ++i) {
                    tmp8s = 0;
                    int is_nan = isnan(VY->d[i]) || isnan(EY->d[i]);
                    // the 1f..0f opcode are singaling/unsignaling, wich is not handled
                    switch(tmp8u&0x0f) {
                        case 0x00: tmp8s=(VY->d[i] == EY->d[i]) && !is_nan; break;
                        case 0x01: tmp8s=isless(VY->d[i], EY->d[i]) && !is_nan; break;
                        case 0x02: tmp8s=islessequal(VY->d[i], EY->d[i]) && !is_nan; break;
                        case 0x03: tmp8s=is_nan; break;
                        case 0x04: tmp8s=(VY->d[i] != EY->d[i]) || is_nan; break;
                        case 0x05: tmp8s=is_nan || isgreaterequal(VY->d[i], EY->d[i]); break;
                        case 0x06: tmp8s=is_nan || isgreater(VY->d[i], EY->d[i]); break;
                        case 0x07: tmp8s=!is_nan; break;
                        case 0x08: tmp8s=(VY->d[i] == EY->d[i]) || is_nan; break;
                        case 0x09: tmp8s=isless(VY->d[i], EY->d[i]) || is_nan; break;
                        case 0x0a: tmp8s=islessequal(VY->d[i], EY->d[i]) || is_nan; break;
                        case 0x0b: tmp8s=0; break;
                        case 0x0c: tmp8s=(VY->d[i] != EY->d[i]) && !is_nan; break;
                        case 0x0d: tmp8s=isgreaterequal(VY->d[i], EY->d[i]) && !is_nan; break;
                        case 0x0e: tmp8s=isgreater(VY->d[i], EY->d[i]) && !is_nan; break;
                        case 0x0f: tmp8s=1; break;
                    }
                    GY->q[i]=(tmp8s)?0xffffffffffffffffLL:0LL;
                }
            } else
                GY->u128 = 0;
            break;

        case 0xC4:  /* VPINSRW Gx, Vx, Ew, Ib */
            nextop = F8;
            GETED(1);
            GETGX;
            GETVX;
            GETGY;
            tmp8u = F8;
            if(GX!=VX)
                GX->u128 = VX->u128;
            GX->uw[tmp8u&7] = ED->word[0];   // only low 16bits
            GY->u128 = 0;
            break;
        case 0xC5:  /* VPEXTRW Gw,Ex,Ib */
            nextop = F8;
            GETEX(1);
            GETGD;
            tmp8u = F8;
            GD->q[0] = EX->uw[tmp8u&7];  // 16bits extract, 0 extended
            break;
        case 0xC6:  /* VSHUFPD Gx, Vx, Ex, Ib */
            nextop = F8;
            GETEX(1);
            GETGX; GETVX; GETGY;
            tmp8u = F8;
            eax1.q[0] = VX->q[tmp8u&1];
            eax1.q[1] = EX->q[(tmp8u>>1)&1];
            GX->u128 = eax1.u128;
            if(vex.l) {
                GETEY; GETVY;
                eax1.q[0] = VY->q[(tmp8u>>2)&1];
                eax1.q[1] = EY->q[(tmp8u>>3)&1];
                GY->u128 = eax1.u128;
            } else
                GY->u128 = 0;
            break;

        case 0xD0:  /* VADDSUBPD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GX->d[0] = VX->d[0] - EX->d[0];
            GX->d[1] = VX->d[1] + EX->d[1];
            if(vex.l) {
                GETEY;
                GETVY;
                GY->d[0] = VY->d[0] - EY->d[0];
                GY->d[1] = VY->d[1] + EY->d[1];
            } else
                GY->u128 = 0;
            break;
        case 0xD1:  /* VPSRLW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX; GETGY;
            tmp64u = EX->q[0];
            if(tmp64u>15) GX->u128 = 0;
            else
                {tmp8u=tmp64u; for (int i=0; i<8; ++i) GX->uw[i] = VX->uw[i] >> tmp8u;}
            if(vex.l) {
                GETEY; GETVY;
                if(tmp64u>15) GY->u128 = 0;
                else
                    {tmp8u=tmp64u; for (int i=0; i<8; ++i) GY->uw[i] = VY->uw[i] >> tmp8u;}
            } else
                GY->u128 = 0;
            break;
        case 0xD2:  /* VPSRLD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX; GETGY;
            tmp64u = EX->q[0];
            if(tmp64u>31) GX->u128 = 0;
            else
                {tmp8u=tmp64u; for (int i=0; i<4; ++i) GX->ud[i] = VX->ud[i] >> tmp8u;}
            if(vex.l) {
                GETEY; GETVY;
                if(tmp64u>31) GY->u128 = 0;
                else
                    {tmp8u=tmp64u; for (int i=0; i<4; ++i) GY->ud[i] = VY->ud[i] >> tmp8u;}
            } else
                GY->u128 = 0;
            break;
        case 0xD3:  /* VPSRLQ Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX; GETGY;
            tmp64u = EX->q[0];
            if(tmp64u>63) GX->u128 = 0;
            else
                {tmp8u=tmp64u; for (int i=0; i<2; ++i) GX->q[i] = VX->q[i] >> tmp8u;}
            if(vex.l) {
                GETEY; GETVY;
                if(tmp64u>63) GY->u128 = 0;
                else
                    {tmp8u=tmp64u; for (int i=0; i<2; ++i) GY->q[i] = VY->q[i] >> tmp8u;}
            } else
                GY->u128 = 0;
            break;
        case 0xD4:  /* VPADDQ Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<2; ++i)
                GX->sq[i] = VX->sq[i] + EX->sq[i];
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<2; ++i)
                    GY->sq[i] = VY->sq[i] + EY->sq[i];
            } else
                GY->u128 = 0;
            break;
        case 0xD5:  /* VPMULLW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            for(int i=0; i<8; ++i) {
                tmp32s = (int32_t)VX->sw[i] * EX->sw[i];
                GX->sw[i] = tmp32s&0xffff;
            }
            if(vex.l) {
                GETEY; GETVY;
                for(int i=0; i<8; ++i) {
                    tmp32s = (int32_t)VY->sw[i] * EY->sw[i];
                    GY->sw[i] = tmp32s&0xffff;
                }
            } else
                GY->u128 = 0;
            break;
        case 0xD6:  /* VMOVQ Ex, Gx */
            nextop = F8;
            GETEX(0);
            GETGX;
            EX->q[0] = GX->q[0];
            if(MODREG) {
                EX->q[1] = 0;
                GETEY;
                EY->u128 = 0;
            }
            break;
        case 0xD7:  /* PMOVMSKB Gd,Ex */
            nextop = F8;
            if(MODREG) {
                GETEX(0);
                GETGD;
                GD->q[0] = 0;
                for (int i=0; i<16; ++i)
                    if(EX->ub[i]&0x80)
                        GD->dword[0] |= (1<<i);
                if(vex.l) {
                    GETEY;
                    for (int i=0; i<16; ++i)
                        if(EY->ub[i]&0x80)
                            GD->dword[0] |= (1<<(i+16));
                }
            } else
                return 0;
            break;
        case 0xD8:  /* VPSUBUSB Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<16; ++i) {
                tmp16s = (int16_t)VX->ub[i] - EX->ub[i];
                GX->ub[i] = (tmp16s>255)?255:((tmp16s<0)?0:tmp16s);
            }
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<16; ++i) {
                    tmp16s = (int16_t)VY->ub[i] - EY->ub[i];
                    GY->ub[i] = (tmp16s>255)?255:((tmp16s<0)?0:tmp16s);
                }
            } else
                GY->u128 = 0;
            break;
        case 0xD9:  /* VPSUBUSW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<8; ++i) {
                tmp32s = (int32_t)VX->uw[i] - EX->uw[i];
                GX->uw[i] = (tmp32s>65535)?65535:((tmp32s<0)?0:tmp32s);
            }
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<8; ++i) {
                    tmp32s = (int32_t)VY->uw[i] - EY->uw[i];
                    GY->uw[i] = (tmp32s>65535)?65535:((tmp32s<0)?0:tmp32s);
                }
            } else
                GY->u128 = 0;
            break;
        case 0xDA:  /* VPMINUB Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for (int i=0; i<16; ++i)
                GX->ub[i] = (EX->ub[i]<VX->ub[i])?EX->ub[i]:VX->ub[i];
            if(vex.l) {
                GETEY; GETVY;
                for (int i=0; i<16; ++i)
                    GY->ub[i] = (EY->ub[i]<VY->ub[i])?EY->ub[i]:VY->ub[i];
            } else
                GY->u128 = 0;
            break;
        case 0xDB:  /* VPAND Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GX->q[0] = VX->q[0] & EX->q[0];
            GX->q[1] = VX->q[1] & EX->q[1];
            GETGY;
            if(vex.l) {
                GETEY;
                GETVY;
                GY->q[0] = VY->q[0] & EY->q[0];
                GY->q[1] = VY->q[1] & EY->q[1];
            } else {
                GY->u128 = 0;
            }
            break;
        case 0xDC:  /* VPADDUSB Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<16; ++i) {
                tmp16s = (int16_t)VX->ub[i] + EX->ub[i];
                GX->ub[i] = (tmp16s>255)?255:tmp16s;
            }
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<16; ++i) {
                    tmp16s = (int16_t)VY->ub[i] + EY->ub[i];
                    GY->ub[i] = (tmp16s>255)?255:tmp16s;
                }
            } else
                GY->u128 = 0;
            break;
        case 0xDD:  /* VPADDUSW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<8; ++i) {
                tmp32s = (int32_t)VX->uw[i] + EX->uw[i];
                GX->uw[i] = (tmp32s>65535)?65535:tmp32s;
            }
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<8; ++i) {
                    tmp32s = (int32_t)VY->uw[i] + EY->uw[i];
                    GY->uw[i] = (tmp32s>65535)?65535:tmp32s;
                }
            } else
                GY->u128 = 0;
            break;
        case 0xDE:  /* VPMAXUB Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for (int i=0; i<16; ++i)
                GX->ub[i] = (EX->ub[i]>VX->ub[i])?EX->ub[i]:VX->ub[i];
            if(vex.l) {
                GETEY; GETVY;
                for (int i=0; i<16; ++i)
                    GY->ub[i] = (EY->ub[i]>VY->ub[i])?EY->ub[i]:VY->ub[i];
            } else
                GY->u128 = 0;
            break;
        case 0xDF:  /* VPANDN Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GX->q[0] = (~(VX->q[0])) & EX->q[0];
            GX->q[1] = (~(VX->q[1])) & EX->q[1];
            if(vex.l) {
                GETVY;
                GETEY;
                GY->q[0] = (~(VY->q[0])) & EY->q[0];
                GY->q[1] = (~(VY->q[1])) & EY->q[1];
            } else
                GY->u128 = 0;
            break;
        case 0xE0:  /* VPAVGB Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for (int i=0; i<16; ++i)
                GX->ub[i] = ((uint16_t)VX->ub[i] + EX->ub[i] + 1)>>1;
            if(vex.l) {
                GETEY;
                GETVY;
                for (int i=0; i<16; ++i)
                    GY->ub[i] = ((uint16_t)VY->ub[i] + EY->ub[i] + 1)>>1;
            } else 
                GY->u128 = 0;
            break;
        case 0xE1:  /* VPSRAW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX; GETGY;
            tmp8u=(EX->q[0]>15)?15:EX->ub[0];
            for (int i=0; i<8; ++i)
                GX->sw[i] = VX->sw[i] >> tmp8u;
            if(vex.l) {
                GETEY; GETVY;
                for (int i=0; i<8; ++i)
                    GY->sw[i] = VY->sw[i] >> tmp8u;
            } else
                GY->u128 = 0;
            break;
        case 0xE2:  /* VPSRAD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX; GETGY;
            tmp8u=(EX->q[0]>31)?31:EX->ub[0];
            for (int i=0; i<4; ++i)
                GX->sd[i] = VX->sd[i] >> tmp8u;
            if(vex.l) {
                GETEY; GETVY;
                for (int i=0; i<4; ++i)
                    GY->sd[i] = VY->sd[i] >> tmp8u;
            } else
                GY->u128 = 0;
            break;
        case 0xE3:  /* VPAVGW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for (int i=0; i<8; ++i)
                GX->uw[i] = ((uint32_t)VX->uw[i] + EX->uw[i] + 1)>>1;
            if(vex.l) {
                GETEY;
                GETVY;
                for (int i=0; i<8; ++i)
                    GY->uw[i] = ((uint32_t)VY->uw[i] + EY->uw[i] + 1)>>1;
            } else
                GY->u128 = 0;
            break;
        case 0xE4:  /* VPMULHUW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            for(int i=0; i<8; ++i) {
                tmp32u = (uint32_t)VX->uw[i] * EX->uw[i];
                GX->uw[i] = (tmp32u>>16)&0xffff;
            }
            if(vex.l) {
                GETEY; GETVY;
                for(int i=0; i<8; ++i) {
                    tmp32u = (uint32_t)VY->uw[i] * EY->uw[i];
                    GY->uw[i] = (tmp32u>>16)&0xffff;
                }
            } else
                GY->u128 = 0;
            break;
        case 0xE5:  /* VPMULHW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX, GETGY;
            for(int i=0; i<8; ++i) {
                tmp32s = (int32_t)VX->sw[i] * EX->sw[i];
                GX->uw[i] = (tmp32s>>16)&0xffff;
            }
            if(vex.l) {
                GETEY; GETVY;
                for(int i=0; i<8; ++i) {
                    tmp32s = (int32_t)VY->sw[i] * EY->sw[i];
                    GY->uw[i] = (tmp32s>>16)&0xffff;
                }
            } else
                GY->u128 = 0;
            break;
        case 0xE6:  /* CVTTPD2DQ Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            if(isnan(EX->d[0]) || isinf(EX->d[0]) || EX->d[0]>0x7fffffff)
                GX->sd[0] = 0x80000000;
            else
                GX->sd[0] = EX->d[0];
            if(isnan(EX->d[1]) || isinf(EX->d[1]) || EX->d[1]>0x7fffffff)
                GX->sd[1] = 0x80000000;
            else
                GX->sd[1] = EX->d[1];
            if(vex.l) {
                GETEY;
                if(isnan(EY->d[0]) || isinf(EY->d[0]) || EY->d[0]>0x7fffffff)
                    GX->sd[2] = 0x80000000;
                else
                    GX->sd[2] = EY->d[0];
                if(isnan(EY->d[1]) || isinf(EY->d[1]) || EY->d[1]>0x7fffffff)
                    GX->sd[3] = 0x80000000;
                else
                    GX->sd[3] = EY->d[1];
            } else
                GX->q[1] = 0;
            GY->u128 = 0;
            break;
        case 0xE7:   /* VMOVNTDQ Ex, Gx */
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
            }
            break;
        case 0xE8:  /* VSUBSB Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<16; ++i) {
                tmp16s = (int16_t)VX->sb[i] - EX->sb[i];
                GX->sb[i] = (tmp16s>127)?127:((tmp16s<-128)?-128:tmp16s);
            }
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<16; ++i) {
                    tmp16s = (int16_t)VY->sb[i] - EY->sb[i];
                    GY->sb[i] = (tmp16s>127)?127:((tmp16s<-128)?-128:tmp16s);
                }
            } else
                GY->u128 = 0;
            break;
        case 0xE9:  /* VPSUBSW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<8; ++i) {
                tmp32s = (int32_t)VX->sw[i] - EX->sw[i];
                GX->sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
            }
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<8; ++i) {
                    tmp32s = (int32_t)VY->sw[i] - EY->sw[i];
                    GY->sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
                }
            } else
                GY->u128 = 0;
            break;
        case 0xEA:  /* VPMINSW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<8; ++i)
                GX->sw[i] = (VX->sw[i]<EX->sw[i])?VX->sw[i]:EX->sw[i];
            if(vex.l) {
                GETEY; GETVY;
                for(int i=0; i<8; ++i)
                    GY->sw[i] = (VY->sw[i]<EY->sw[i])?VY->sw[i]:EY->sw[i];
            } else
                GY->u128 = 0;
            break;
        case 0xEB:  /* VPOR Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GX->q[0] = VX->q[0] | EX->q[0];
            GX->q[1] = VX->q[1] | EX->q[1];
            GETGY;
            if(vex.l) {
                GETEY;
                GETVY;
                GY->q[0] = VY->q[0] | EY->q[0];
                GY->q[1] = VY->q[1] | EY->q[1];
            } else {
                GY->u128 = 0;
            }
            break;
        case 0xEC:  /* VPADDSB Gx,Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<16; ++i) {
                tmp16s = (int16_t)VX->sb[i] + EX->sb[i];
                GX->sb[i] = (tmp16s>127)?127:((tmp16s<-128)?-128:tmp16s);
            }
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<16; ++i) {
                    tmp16s = (int16_t)VY->sb[i] + EY->sb[i];
                    GY->sb[i] = (tmp16s>127)?127:((tmp16s<-128)?-128:tmp16s);
                }
            } else
                GY->u128 = 0;
            break;
        case 0xED:  /* VPADDSW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<8; ++i) {
                tmp32s = (int32_t)VX->sw[i] + EX->sw[i];
                GX->sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
            }
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<8; ++i) {
                    tmp32s = (int32_t)VY->sw[i] + EY->sw[i];
                    GY->sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
                }
            } else
                GY->u128 = 0;
            break;
        case 0xEE:  /* VPMAXSW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<8; ++i)
                GX->sw[i] = (VX->sw[i]>EX->sw[i])?VX->sw[i]:EX->sw[i];
            if(vex.l) {
                GETEY; GETVY;
                for(int i=0; i<8; ++i)
                    GY->sw[i] = (VY->sw[i]>EY->sw[i])?VY->sw[i]:EY->sw[i];
            } else
                GY->u128 = 0;
            break;
        case 0xEF:                      /* VPXOR Gx,Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GX->q[0] = VX->q[0] ^ EX->q[0];
            GX->q[1] = VX->q[1] ^ EX->q[1];
            GETGY;
            if(vex.l) {
                GETEY;
                GETVY;
                GY->q[0] = VY->q[0] ^ EY->q[0];
                GY->q[1] = VY->q[1] ^ EY->q[1];
            } else 
                GY->u128 = 0;
            break;

        case 0xF1:  /* VPSLLW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            if(EX->q[0]>15)
                GX->u128 = 0;
            else
                {tmp8u=EX->ub[0]; for (int i=0; i<8; ++i) GX->uw[i] = VX->uw[i]<<tmp8u;}
            if(vex.l) {
                GETEY; GETVY;
                if(EY->q[0]>15)
                    GY->u128 = 0;
                else
                    {tmp8u=EY->ub[0]; for (int i=0; i<8; ++i) GY->uw[i] = VY->uw[i]<<tmp8u;}
            } else
                GY->u128 = 0;
            break;
        case 0xF2:  /* VPSLLD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            if(EX->q[0]>31)
                GX->u128 = 0;
            else
                {tmp8u=EX->ub[0]; for (int i=0; i<4; ++i) GX->ud[i] = VX->ud[i]<<tmp8u;}
            if(vex.l) {
                GETEY; GETVY;
                if(EY->q[0]>31)
                    GY->u128 = 0;
                else
                    {tmp8u=EY->ub[0]; for (int i=0; i<4; ++i) GY->ud[i] = VY->ud[i]<<tmp8u;}
            } else
                GY->u128 = 0;
            break;
        case 0xF3:  /* VPSLLQ Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            if(EX->q[0]>63)
                GX->u128 = 0;
            else
                {tmp8u=EX->ub[0]; for (int i=0; i<2; ++i) GX->q[i] = VX->q[i]<<tmp8u;}
            if(vex.l) {
                GETEY; GETVY;
                if(EY->q[0]>63)
                    GY->u128 = 0;
                else
                    {tmp8u=EY->ub[0]; for (int i=0; i<2; ++i) GY->q[i] = VY->q[i]<<tmp8u;}
            } else
                GY->u128 = 0;
            break;
        case 0xF4:  /* VPMULUDQ Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX, GETGY;
            GX->q[1] = (uint64_t)EX->ud[2]*VX->ud[2];
            GX->q[0] = (uint64_t)EX->ud[0]*VX->ud[0];
            if(vex.l) {
                GETEY; GETVY;
                GY->q[1] = (uint64_t)EY->ud[2]*VY->ud[2];
                GY->q[0] = (uint64_t)EY->ud[0]*VY->ud[0];
            } else
                GY->u128 = 0;
            break;
        case 0xF5:  /* VPMADDWD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for (int i=0; i<4; ++i)
                GX->sd[i] = (int32_t)(VX->sw[i*2+0])*EX->sw[i*2+0] + (int32_t)(VX->sw[i*2+1])*EX->sw[i*2+1];
            if(vex.l) {
                GETEY; GETVY;
                for (int i=0; i<4; ++i)
                    GY->sd[i] = (int32_t)(VY->sw[i*2+0])*EY->sw[i*2+0] + (int32_t)(VY->sw[i*2+1])*EY->sw[i*2+1];
            } else
                GY->u128 = 0;
            break;
        case 0xF6:  /* VPSADBW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            tmp32u = 0;
            for (int i=0; i<8; ++i)
                tmp32u += (VX->ub[i]>EX->ub[i])?(VX->ub[i] - EX->ub[i]):(EX->ub[i] - VX->ub[i]);
            GX->q[0] = tmp32u;
            tmp32u = 0;
            for (int i=8; i<16; ++i)
                tmp32u += (VX->ub[i]>EX->ub[i])?(VX->ub[i] - EX->ub[i]):(EX->ub[i] - VX->ub[i]);
            GX->q[1] = tmp32u;
            if(vex.l) {
                GETEY; GETVY;
                tmp32u = 0;
                for (int i=0; i<8; ++i)
                    tmp32u += (VY->ub[i]>EY->ub[i])?(VY->ub[i] - EY->ub[i]):(EY->ub[i] - VY->ub[i]);
                GY->q[0] = tmp32u;
                tmp32u = 0;
                for (int i=8; i<16; ++i)
                    tmp32u += (VY->ub[i]>EY->ub[i])?(VY->ub[i] - EY->ub[i]):(EY->ub[i] - VY->ub[i]);
                GY->q[1] = tmp32u;
            } else
                GY->u128 = 0;
            break;
        case 0xF7:  /* VMASKMOVDQU Gx, Ex */
            nextop = F8;
            if(vex.l) {
                emit_signal(emu, SIGILL, (void*)R_RIP, 0);
            }
            GETEX(0);
            GETGX;
            VX = (sse_regs_t *)(R_RDI);
            for (int i=0; i<16; ++i) {
                if(EX->ub[i]&0x80)
                    VX->ub[i] = GX->ub[i];
            }
            // no raz of upper ymm
            break;
        case 0xF8:  /* VSUBB Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<16; ++i)
                GX->sb[i] = VX->sb[i] - EX->sb[i];
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<16; ++i)
                    GY->sb[i] = VY->sb[i] - EY->sb[i];
            } else
                GY->u128 = 0;
            break;
        case 0xF9:  /* VPSUBW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<8; ++i)
                GX->sw[i] = VX->sw[i] - EX->sw[i];
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<8; ++i)
                    GY->sw[i] = VY->sw[i] - EY->sw[i];
            } else
                GY->u128 = 0;
            break;
        case 0xFA:  /* VPSUBD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<4; ++i)
                GX->sd[i] = VX->sd[i] - EX->sd[i];
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<4; ++i)
                    GY->sd[i] = VY->sd[i] - EY->sd[i];
            } else
                GY->u128 = 0;
            break;
        case 0xFB:  /* VPSUBQ Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<2; ++i)
                GX->sq[i] = VX->sq[i] - EX->sq[i];
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<2; ++i)
                    GY->sq[i] = VY->sq[i] - EY->sq[i];
            } else
                GY->u128 = 0;
            break;
        case 0xFC:  /* VPADDB Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<16; ++i)
                GX->sb[i] = VX->sb[i] + EX->sb[i];
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<16; ++i)
                    GY->sb[i] = VY->sb[i] + EY->sb[i];
            } else
                GY->u128 = 0;
            break;
        case 0xFD:  /* VPADDW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<8; ++i)
                GX->sw[i] = VX->sw[i] + EX->sw[i];
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<8; ++i)
                    GY->sw[i] = VY->sw[i] + EY->sw[i];
            } else
                GY->u128 = 0;
            break;
        case 0xFE:  /* VPADDD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<4; ++i)
                GX->sd[i] = VX->sd[i] + EX->sd[i];
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<4; ++i)
                    GY->sd[i] = VY->sd[i] + EY->sd[i];
            } else
                GY->u128 = 0;
            break;

        default:
            return 0;
    }
    return addr;
}
