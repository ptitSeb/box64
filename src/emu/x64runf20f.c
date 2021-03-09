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
#include "bridge.h"
//#include "signals.h"
#ifdef DYNAREC
#include "../dynarec/arm_lock_helper.h"
#endif

#include "modrm.h"

int RunF20F(x64emu_t *emu, rex_t rex)
{
    uint8_t opcode;
    uint8_t nextop;
    reg64_t *oped, *opgd;
    sse_regs_t *opex, *opgx;

    opcode = F8;

    switch(opcode) {

    case 0x10:  /* MOVSD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->q[0] = EX->q[0];
        if((nextop&0xC0)!=0xC0) {
            // EX is not a register
            GX->q[1] = 0;
        }
        break;
    case 0x11:  /* MOVSD Ex, Gx */
        nextop = F8;
        GETEX(0);
        GETGX;
        EX->q[0] = GX->q[0];
        break;

    case 0x2A:  /* CVTSI2SD Gx, Ed */
        nextop = F8;
        GETED(0);
        GETGX;
        if(rex.w)
            GX->d[0] = ED->sq[0];
        else
            GX->d[0] = ED->sdword[0];
        break;

    case 0x2C:  /* CVTTSD2SI Gd, Ex */
        nextop = F8;
        GETEX(0);
        GETGD;
        if(rex.w)
            GD->sq[0] = EX->d[0];
        else
            GD->sdword[0] = EX->d[0];
        break;

    case 0x58:  /* ADDSD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->d[0] += EX->d[0];
        break;
    case 0x59:  /* MULSD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->d[0] *= EX->d[0];
        break;

    case 0x5E:  /* DIVSD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->d[0] /= EX->d[0];
        break;

    default:
        return 1;
    }
    return 0;
}