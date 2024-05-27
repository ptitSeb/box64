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

        case 0x64:  /* VPCMPGTB Gx,Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GETVY;
            for(int i=0; i<16; ++i)
                GX->ub[i] = (VX->sb[i]>EX->sb[i])?0xFF:0x00;
            if(vex.l)
                for(int i=0; i<16; ++i)
                    GY->ub[i] = (VY->sb[i]>EY->sb[i])?0xFF:0x00;
            else
                GY->q[0] = GY->q[1] = 0;
            break;
        case 0x65:  /* VPCMPGTW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GETVY;
            for(int i=0; i<8; ++i)
                GX->uw[i] = (VX->sw[i]>EX->sw[i])?0xFFFF:0x0000;
            if(vex.l)
                for(int i=0; i<8; ++i)
                    GY->uw[i] = (VY->sw[i]>EY->sw[i])?0xFFFF:0x0000;
            else
                GY->q[0] = GY->q[1] = 0;
            break;
        case 0x66:  /* VPCMPGTD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GETVY;
            for(int i=0; i<4; ++i)
                GX->ud[i] = (VX->sd[i]>EX->sd[i])?0xFFFFFFFF:0x00000000;
            if(vex.l)
                for(int i=0; i<4; ++i)
                    GY->ud[i] = (VY->sd[i]>EY->sd[i])?0xFFFFFFFF:0x00000000;
            else
                GY->q[0] = GY->q[1] = 0;
            break;

        case 0x6C:  /* VPUNPCKLQDQ Gx,E Vx, x */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GETVY;
            if(GX!=VX)
                GX->q[0] = VX->q[0];
            GX->q[1] = EX->q[0];
            if(vex.l) {
                GETEY;
                if(GY!=VY)
                    GY->q[0] = VY->q[0];
                GY->q[1] = EY->q[0];
            } else
                GY->q[0] = GY->q[1] = 0;
            break;
        case 0x6D:  /* VPUNPCKHQDQ Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GETVY;
            GX->q[0] = VX->q[1];
            GX->q[1] = EX->q[1];
            if(vex.l) {
                GETEY;
                GY->q[0] = VY->q[1];
                GY->q[1] = EY->q[1];

            } else
                GY->q[0] = GY->q[1] = 0;
            break;

        case 0x6F:  // VMOVDQA
            nextop = F8;
            GETEX(0);
            GETGX;
            GX->q[0] = EX->q[0];
            GX->q[1] = EX->q[1];
            if(vex.l) {
                GETGY;
                GETEY;
                if(MODREG) {
                    GY->q[0] = EY->q[0];
                    GY->q[1] = EY->q[1];
                } else
                    GY->q[0] = GY->q[1] = 0;
            }
            break;
        case 0x70:  /* VPSHUFD Gx,Ex,Ib */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETGY;
            tmp8u = F8;
            if(vex.l) {
                GETEY;
                if(EY==GY) {eay1 = *GY; EY = &eay1;}   // copy is needed
                for (int i=0; i<4; ++i)
                    GY->ud[4+i] = EY->ud[4+((tmp8u>>(i*2))&3)];
            } else 
                memset(GY, 0, 16);
            if(EX==GX) {eax1 = *GX; EX = &eax1;}   // copy is needed
            for (int i=0; i<4; ++i)
                GX->ud[i] = EX->ud[(tmp8u>>(i*2))&3];
            break;

        case 0x72:  /* GRP */
            nextop = F8;
            GETEX(1);
            GETEY;
            GETVX;
            GETVY;
            if(!vex.l && MODREG)
                memset(VY, 0, 16);
            switch((nextop>>3)&7) {
                case 2:                 /* PSRLD Vx, Ex, Ib */
                    tmp8u = F8;
                    if(vex.l) {
                        emit_signal(emu, SIGILL, (void*)R_RIP, 0);
                    } else {
                        if(tmp8u>31)
                            {VX->q[0] = VX->q[1] = 0;}
                        else
                            for (int i=0; i<4; ++i) VX->ud[i] = EX->ud[i] >> tmp8u;
                    }
                    break;
                case 4:                 /* PSRAD Vx, Ex, Ib */
                    tmp8u = F8;
                    if(vex.l) {
                        emit_signal(emu, SIGILL, (void*)R_RIP, 0);
                    } else {
                        if(tmp8u>31) tmp8u=31;
                        for (int i=0; i<4; ++i) VX->sd[i] = EX->sd[i] >> tmp8u;
                    }
                    break;
                case 6:                 /* PSLLD Vx, Ex, Ib */
                    tmp8u = F8;
                    if(vex.l) {
                        emit_signal(emu, SIGILL, (void*)R_RIP, 0);
                    } else {
                        if(tmp8u>31)
                            {VX->q[0] = VX->q[1] = 0;}
                        else
                            for (int i=0; i<4; ++i) VX->ud[i] = EX->ud[i] << tmp8u;
                    }
                    break;
                default:
                    return 0;
            }
            break;
        case 0x73:  /* GRP */
            nextop = F8;
            GETEX(1);
            GETEY;
            GETVX;
            GETVY;
            if(!vex.l && MODREG)
                memset(VY, 0, 16);
            switch((nextop>>3)&7) {
                case 2:                 /* PSRLQ Vx, Ex, Ib */
                    tmp8u = F8;
                    if(vex.l) {
                        emit_signal(emu, SIGILL, (void*)R_RIP, 0);
                    } else {
                        if(tmp8u>63)
                            {VX->q[0] = VX->q[1] = 0;}
                        else
                            {VX->q[0] = EX->q[0] >> tmp8u; VX->q[1] = EX->q[1] >> tmp8u;}
                    }
                    break;
                case 3:                 /* PSRLDQ Vx, Ex, Ib */
                    tmp8u = F8;
                    if(vex.l) {
                        emit_signal(emu, SIGILL, (void*)R_RIP, 0);
                    } else {
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
                    }
                    break;
                case 6:                 /* PSLLQ Vx, Ex, Ib */
                    tmp8u = F8;
                    if(vex.l) {
                        emit_signal(emu, SIGILL, (void*)R_RIP, 0);
                    } else {
                        if(tmp8u>63)
                            {VX->q[0] = VX->q[1] = 0;}
                        else
                            {VX->q[0] = EX->q[0] << tmp8u; VX->q[1] = EX->q[1] << tmp8u;}
                    }
                    break;
                case 7:                 /* PSLLDQ Vx, Ex, Ib */
                    tmp8u = F8;
                    if(vex.l) {
                        emit_signal(emu, SIGILL, (void*)R_RIP, 0);
                    } else {
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
                    }
                    break;
                default:
                    return 0;
            }
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
