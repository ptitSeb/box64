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
                GY->q[0] = GY->q[1] = 0;
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
                GY->q[0] = GY->q[1] = 0;
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
                GY->q[0] = GY->q[1] = 0;
            break;

        case 0x6B:  /* VPACKSSDW Gx,Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<4; ++i)
                GX->sw[i] = (VX->sd[i]<-32768)?-32768:((VX->sd[i]>32767)?32767:VX->sd[i]);
            if(GX==EX)
                GX->q[1] = GX->q[0];
            else
                for(int i=0; i<4; ++i)
                    GX->sw[4+i] = (EX->sd[i]<-32768)?-32768:((EX->sd[i]>32767)?32767:EX->sd[i]);
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<4; ++i)
                    GY->sw[i] = (VY->sd[i]<-32768)?-32768:((VY->sd[i]>32767)?32767:VY->sd[i]);
                if(GY==EY)
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
                GY->q[0] = GY->q[1] = 0;
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
                GY->q[0] = GY->q[1] = 0;
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
            GY->q[0] = GY->q[1] = 0;
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
                GY->q[0] = GY->q[1] = 0;
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

        case 0x72:  /* GRP */
            nextop = F8;
            GETEX(1);
            GETVX;
            GETVY;
            switch((nextop>>3)&7) {
                case 2:                 /* VPSRLD Vx, Ex, Ib */
                    tmp8u = F8;
                    if(tmp8u>31)
                        {VX->q[0] = VX->q[1] = 0;}
                    else
                        for (int i=0; i<4; ++i) VX->ud[i] = EX->ud[i] >> tmp8u;
                    if(vex.l) {
                        GETEY;
                        if(tmp8u>31)
                            {VY->q[0] = VY->q[1] = 0;}
                        else
                            for (int i=0; i<4; ++i) VY->ud[i] = EY->ud[i] >> tmp8u;
                    } else
                        VY->q[0] = VY->q[1] = 0;
                    break;
                case 4:                 /* VPSRAD Vx, Ex, Ib */
                    tmp8u = F8;
                    if(tmp8u>31) tmp8u=31;
                    for (int i=0; i<4; ++i) VX->sd[i] = EX->sd[i] >> tmp8u;
                    if(vex.l) {
                        GETEY;
                        for (int i=0; i<4; ++i) VY->sd[i] = EY->sd[i] >> tmp8u;
                    } else
                        VY->q[0] = VY->q[1] = 0;
                    break;
                case 6:                 /* VPSLLD Vx, Ex, Ib */
                    tmp8u = F8;
                    if(tmp8u>31)
                        {VX->q[0] = VX->q[1] = 0;}
                    else
                        for (int i=0; i<4; ++i) VX->ud[i] = EX->ud[i] << tmp8u;
                    if(vex.l) {
                        GETEY;
                        if(tmp8u>31)
                            {VY->q[0] = VY->q[1] = 0;}
                        else
                            for (int i=0; i<4; ++i) VY->ud[i] = EY->ud[i] << tmp8u;
                    } else
                        VY->q[0] = VY->q[1] = 0;
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
            switch((nextop>>3)&7) {
                case 2:                 /* VPSRLQ Vx, Ex, Ib */
                    tmp8u = F8;
                    if(tmp8u>63)
                        {VX->q[0] = VX->q[1] = 0;}
                    else
                        {VX->q[0] = EX->q[0] >> tmp8u; VX->q[1] = EX->q[1] >> tmp8u;}
                    if(vex.l) {
                        GETEY;
                        if(tmp8u>63)
                            {VY->q[0] = VY->q[1] = 0;}
                        else
                            {VY->q[0] = EY->q[0] >> tmp8u; VY->q[1] = EY->q[1] >> tmp8u;}
                    } else
                        VY->q[0] = VY->q[1] = 0;
                    break;
                case 3:                 /* VPSRLDQ Vx, Ex, Ib */
                    tmp8u = F8;
                    if(tmp8u>15)
                        {VX->q[0] = VX->q[1] = 0;}
                    else if (tmp8u!=0) {
                        tmp8u*=8;
                        if (tmp8u < 64) {
                            VX->q[0] = (EX->q[0] >> tmp8u) | (EX->q[1] << (64 - tmp8u));
                            VX->q[1] = (EX->q[1] >> tmp8u);
                        } else {
                            VX->q[0] = EX->q[1] >> (tmp8u - 64);
                            VX->q[1] = 0;
                        }
                    }
                    if(vex.l) {
                        GETEY;
                        if(tmp8u>15)
                            {VY->q[0] = VY->q[1] = 0;}
                        else if (tmp8u!=0) {
                            tmp8u*=8;
                            if (tmp8u < 64) {
                                VY->q[0] = (EY->q[0] >> tmp8u) | (EY->q[1] << (64 - tmp8u));
                                VY->q[1] = (EY->q[1] >> tmp8u);
                            } else {
                                VY->q[0] = EY->q[1] >> (tmp8u - 64);
                                VY->q[1] = 0;
                            }
                        }
                    } else
                        VY->q[0] = VY->q[1] = 0;
                    break;
                case 6:                 /* VPSLLQ Vx, Ex, Ib */
                    tmp8u = F8;
                    if(tmp8u>63)
                        {VX->q[0] = VX->q[1] = 0;}
                    else
                        {VX->q[0] = EX->q[0] << tmp8u; VX->q[1] = EX->q[1] << tmp8u;}
                    if(vex.l) {
                        GETEY;
                        if(tmp8u>63)
                            {VY->q[0] = VY->q[1] = 0;}
                        else
                            {VY->q[0] = EY->q[0] << tmp8u; VY->q[1] = EY->q[1] << tmp8u;}
                    } else
                        VY->q[0] = VY->q[1] = 0;
                    break;
                case 7:                 /* VPSLLDQ Vx, Ex, Ib */
                    tmp8u = F8;
                    if(tmp8u>15)
                        {VX->q[0] = VX->q[1] = 0;}
                    else if (tmp8u!=0) {
                        tmp8u*=8;
                        if (tmp8u < 64) {
                            VX->q[1] = (EX->q[1] << tmp8u) | (EX->q[0] >> (64 - tmp8u));
                            VX->q[0] = (EX->q[0] << tmp8u);
                        } else {
                            VX->q[1] = EX->q[0] << (tmp8u - 64);
                            VX->q[0] = 0;
                        }
                    }
                    if(vex.l) {
                        GETEY;
                        if(tmp8u>15)
                            {VY->q[0] = VY->q[1] = 0;}
                        else if (tmp8u!=0) {
                            tmp8u*=8;
                            if (tmp8u < 64) {
                                VY->q[1] = (EY->q[1] << tmp8u) | (EY->q[0] >> (64 - tmp8u));
                                VY->q[0] = (EY->q[0] << tmp8u);
                            } else {
                                VY->q[1] = EY->q[0] << (tmp8u - 64);
                                VY->q[0] = 0;
                            }
                        }
                    } else
                        VY->q[0] = VY->q[1] = 0;
                    break;
                default:
                    return 0;
            }
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

        case 0xC2:                      /* CMPPD Gx, Vx, Ex, Ib */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETVX;
            GETGY;
            tmp8u = F8;
            for(int i=0; i<2; ++i) {
                tmp8s = 0;
                switch(tmp8u&7) {
                    case 0: tmp8s=(VX->d[i] == EX->d[i]); break;
                    case 1: tmp8s=isless(VX->d[i], EX->d[i]); break;
                    case 2: tmp8s=islessequal(VX->d[i], EX->d[i]); break;
                    case 3: tmp8s=isnan(VX->d[i]) || isnan(EX->d[i]); break;
                    case 4: tmp8s=isnan(VX->d[i]) || isnan(EX->d[i]) || (VX->d[i] != EX->d[i]); break;
                    case 5: tmp8s=isnan(VX->d[i]) || isnan(EX->d[i]) || isgreaterequal(VX->d[i], EX->d[i]); break;
                    case 6: tmp8s=isnan(VX->d[i]) || isnan(EX->d[i]) || isgreater(VX->d[i], EX->d[i]); break;
                    case 7: tmp8s=!isnan(VX->d[i]) && !isnan(EX->d[i]); break;
                }
                GX->q[i]=(tmp8s)?0xffffffffffffffffLL:0LL;
            }
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<2; ++i) {
                    tmp8s = 0;
                    switch(tmp8u&7) {
                        case 0: tmp8s=(VY->d[i] == EY->d[i]); break;
                        case 1: tmp8s=isless(VY->d[i], EY->d[i]); break;
                        case 2: tmp8s=islessequal(VY->d[i], EY->d[i]); break;
                        case 3: tmp8s=isnan(VY->d[i]) || isnan(EY->d[i]); break;
                        case 4: tmp8s=isnan(VY->d[i]) || isnan(EY->d[i]) || (VY->d[i] != EY->d[i]); break;
                        case 5: tmp8s=isnan(VY->d[i]) || isnan(EY->d[i]) || isgreaterequal(VY->d[i], EY->d[i]); break;
                        case 6: tmp8s=isnan(VY->d[i]) || isnan(EY->d[i]) || isgreater(VY->d[i], EY->d[i]); break;
                        case 7: tmp8s=!isnan(VY->d[i]) && !isnan(EY->d[i]); break;
                    }
                    GY->q[i]=(tmp8s)?0xffffffffffffffffLL:0LL;
                }
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
                GY->q[0] = GY->q[1] = 0;
            }
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
                GY->q[0] = GY->q[1] = 0;
            }
            break;

        case 0xEF:                      /* VPXOR Gx,Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            VX->q[0] = GX->q[0] ^ EX->q[0];
            VX->q[1] = GX->q[1] ^ EX->q[1];
            GETGY;
            if(vex.l) {
                GETEY;
                GETVY;
                GY->q[0] = VY->q[0] ^ EY->q[0];
                GY->q[1] = VY->q[1] ^ EY->q[1];
            } else {
                GY->q[0] = GY->q[1] = 0;
            }

            break;

        default:
            return 0;
    }
    return addr;
}
