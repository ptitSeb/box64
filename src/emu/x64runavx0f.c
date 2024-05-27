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
                GY->q[0] = GY->q[1] = 0;
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
                GY->q[0] = GY->q[1] = 0;
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
            }
            break;
            
        case 0x52:                      /* VRSQRTPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            for(int i=0; i<4; ++i) {
                if(EX->f[i]==0)
                    GX->f[i] = 1.0f/EX->f[i];
                else if (EX->f[i]<0)
                    GX->f[i] = NAN;
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
                    if(EY->f[i]==0)
                        GY->f[i] = 1.0f/EY->f[i];
                    else if (EY->f[i]<0)
                        GY->f[i] = NAN;
                    else if (isnan(EY->f[i]))
                        GY->f[i] = EY->f[i];
                    else if (isinf(EY->f[i]))
                        GY->f[i] = 0.0;
                    else
                        GY->f[i] = 1.0f/sqrtf(EY->f[i]);
                }
            } else
                GY->q[0] = GY->q[1] = 0;
            #ifdef TEST_INTERPRETER
            test->notest = 1;
            #endif
            break;

        case 0x57:                      /* XORPS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<4; ++i)
                GX->ud[i] = VX->ud[i] ^ EX->ud[i];
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<4; ++i)
                    GY->ud[i] = VY->ud[i] ^ EY->ud[i];

            } else
                GY->q[0] = GY->q[1] = 0;
            break;

        case 0x59:                      /* VMULPS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<4; ++i)
                GX->f[i] = VX->f[i] * EX->f[i];
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<4; ++i)
                    GY->f[i] = VY->f[i] * EY->f[i];
            } else
                GY->q[0] = GY->q[1] = 0;
            break;

        case 0x5C:                      /* VSUBPS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<4; ++i)
                GX->f[i] = VX->f[i] - EX->f[i];
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<4; ++i)
                    GY->f[i] = VY->f[i] - EY->f[i];
            } else
                GY->q[0] = GY->q[1] = 0;
            break;

        case 0x77:
            if(!vex.l) {    // VZEROUPPER
                if(vex.v!=0) {
                    emit_signal(emu, SIGILL, (void*)R_RIP, 0);
                } else {
                    memset(emu->ymm, 0, sizeof(sse_regs_t)*(vex.rex.is32bits)?16:8);
                }
            } else
                return 0;
            break;

        default:
            return 0;
    }
    return addr;
}
