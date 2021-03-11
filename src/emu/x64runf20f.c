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
    int8_t tmp8s;
    uint8_t tmp8u;
    int32_t tmp32s;
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

    case 0x51:  /* SQRTSD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->d[0] = sqrt(EX->d[0]);
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
    case 0x5A:  /* CVTSD2SS Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->f[0] = EX->d[0];
        break;

    case 0x5C:  /* SUBSD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->d[0] -= EX->d[0];
        break;
    case 0x5D:  /* MINSD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        if (isnan(GX->d[0]) || isnan(EX->d[0]) || isless(EX->d[0], GX->d[0]))
            GX->d[0] = EX->d[0];
        break;
    case 0x5E:  /* DIVSD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->d[0] /= EX->d[0];
        break;
    case 0x5F:  /* MAXSD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        if (isnan(GX->d[0]) || isnan(EX->d[0]) || isgreater(EX->d[0], GX->d[0]))
            GX->d[0] = EX->d[0];
        break;

    GOCOND(0x80
        , tmp32s = F32S; CHECK_FLAGS(emu);
        , R_RIP += tmp32s;
        ,
    )                               /* 0x80 -> 0x8F Jxx */
        
    case 0xC2:  /* CMPSD Gx, Ex, Ib */
        nextop = F8;
        GETEX(0);
        GETGX;
        tmp8u = F8;
        tmp8s = 0;
        switch(tmp8u&7) {
            case 0: tmp8s=(GX->d[0] == EX->d[0]); break;
            case 1: tmp8s=isless(GX->d[0], EX->d[0]); break;
            case 2: tmp8s=islessequal(GX->d[0], EX->d[0]); break;
            case 3: tmp8s=isnan(GX->d[0]) || isnan(EX->d[0]); break;
            case 4: tmp8s=(GX->d[0] != EX->d[0]); break;
            case 5: tmp8s=isnan(GX->d[0]) || isnan(EX->d[0]) || isgreaterequal(GX->d[0], EX->d[0]); break;
            case 6: tmp8s=isnan(GX->d[0]) || isnan(EX->d[0]) || isgreater(GX->d[0], EX->d[0]); break;
            case 7: tmp8s=!isnan(GX->d[0]) && !isnan(EX->d[0]); break;
        }
        GX->q[0]=(tmp8s)?0xffffffffffffffffLL:0LL;
        break;

    default:
        return 1;
    }
    return 0;
}