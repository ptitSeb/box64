#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
//#include "my_cpuid.h"
#include "bridge.h"
//#include "signals.h"
#ifdef DYNAREC
#include "../dynarec/arm_lock_helper.h"
#endif

#include "modrm.h"

int RunF30F(x64emu_t *emu, rex_t rex)
{
    uint8_t opcode;
    uint8_t nextop;
    int8_t tmp8s;
    uint8_t tmp8u;
    reg64_t *oped, *opgd;
    sse_regs_t *opex, *opgx;

    opcode = F8;

    switch(opcode) {

    case 0x10:  /* MOVSS Gx Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->ud[0] = EX->ud[0];
        if((nextop&0xC0)!=0xC0) {
            // EX is not a register (reg to reg only move 31:0)
            GX->ud[1] = GX->ud[2] = GX->ud[3] = 0;
        }
        break;
    case 0x11:  /* MOVSS Ex Gx */
        nextop = F8;
        GETEX(0);
        GETGX;
        EX->ud[0] = GX->ud[0];
        break;

    case 0x2A:  /* CVTSI2SS Gx, Ed */
        nextop = F8;
        GETED(0);
        GETGX;
        if(rex.w)
            GX->f[0] = ED->sq[0];
        else
            GX->f[0] = ED->sdword[0];
        break;

    case 0x2C:  /* CVTTSS2SI Gd, Ex */
        nextop = F8;
        GETEX(0);
        GETGD;
        if (rex.w)
            GD->sq[0] = EX->f[0];
        else {
            GD->sdword[0] = EX->f[0];
            GD->dword[1] = 0;
        }
        break;

    case 0x51:  /* SQRTSS Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->f[0] = sqrtf(EX->f[0]);
        break;
    case 0x52:  /* RSQRTSS Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->f[0] = 1.0f/sqrtf(EX->f[0]);
        break;
    case 0x53:  /* RCPSS Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->f[0] = 1.0f/EX->f[0];
        break;

    case 0x58:  /* ADDSS Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->f[0] += EX->f[0];
        break;
    case 0x59:  /* MULSS Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->f[0] *= EX->f[0];
        break;
    case 0x5A:  /* CVTSS2SD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->d[0] = EX->f[0];
        break;
    case 0x5B:  /* CVTTPS2DQ Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->sd[0] = EX->f[0];
        GX->sd[1] = EX->f[1];
        GX->sd[2] = EX->f[2];
        GX->sd[3] = EX->f[3];
        break;
    case 0x5C:  /* SUBSS Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->f[0] -= EX->f[0];
        break;
    case 0x5D:  /* MINSS Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        if(isnan(GX->f[0]) || isnan(EX->f[0]) || isless(EX->f[0], GX->f[0]))
            GX->f[0] = EX->f[0];
        break;
    case 0x5E:  /* DIVSS Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->f[0] /= EX->f[0];
        break;
    case 0x5F:  /* MAXSS Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        if (isnan(GX->f[0]) || isnan(EX->f[0]) || isgreater(EX->f[0], GX->f[0]))
            GX->f[0] = EX->f[0];
        break;

    case 0x6F:  /* MOVDQU Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        memcpy(GX, EX, 16);    // unaligned...
        break;

    case 0x7E:  /* MOVQ Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->q[0] = EX->q[0];
        GX->q[1] = 0;
        break;
    case 0x7F:  /* MOVDQU Ex, Gx */
        nextop = F8;
        GETEX(0);
        GETGX;
        memcpy(EX, GX, 16);    // unaligned...
        break;

    case 0xC2:  /* CMPSS Gx, Ex, Ib */
        nextop = F8;
        GETEX(1);
        GETGX;
        tmp8u = F8;
        tmp8s = 0;
        switch(tmp8u&7) {
            case 0: tmp8s=(GX->f[0] == EX->f[0]); break;
            case 1: tmp8s=isless(GX->f[0], EX->f[0]); break;
            case 2: tmp8s=islessequal(GX->f[0], EX->f[0]); break;
            case 3: tmp8s=isnan(GX->f[0]) || isnan(EX->f[0]); break;
            case 4: tmp8s=(GX->f[0] != EX->f[0]); break;
            case 5: tmp8s=isnan(GX->f[0]) || isnan(EX->f[0]) || isgreaterequal(GX->f[0], EX->f[0]); break;
            case 6: tmp8s=isnan(GX->f[0]) || isnan(EX->f[0]) || isgreater(GX->f[0], EX->f[0]); break;
            case 7: tmp8s=!isnan(GX->f[0]) && !isnan(EX->f[0]); break;
        }
        GX->ud[0]=(tmp8s)?0xffffffff:0;
        break;

    default:
        return 1;
    }
    return 0;
}