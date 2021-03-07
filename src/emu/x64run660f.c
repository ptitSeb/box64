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

int Run660F(x64emu_t *emu, rex_t rex)
{
    uint8_t opcode;
    uint8_t nextop;
    int32_t tmp32s;
    reg64_t *oped, *opgd;
    sse_regs_t *opex, *opgx;

    opcode = F8;

    switch(opcode) {

    case 0x1F:                      /* NOP (multi-byte) */
        nextop = F8;
        GETED(0);
        break;

    case 0x62:  /* PUNPCKLDQ Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->ud[3] = EX->ud[1];
        GX->ud[2] = GX->ud[1];
        GX->ud[1] = EX->ud[0];
        break;

    case 0x6E:                      /* MOVD Gx, Ed */
        nextop = F8;
        GETED(0);
        GETGX;
        if(rex.w)
            GX->q[0] = ED->q[0];
        else
            GX->q[0] = ED->dword[0];
        GX->q[1] = 0;
        break;
    case 0x6F:                      /* MOVDQA Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->q[0] = EX->q[0];
        GX->q[1] = EX->q[1];
        break;

    case 0x7E:                      /* MOVD Ed, Gx */
        nextop = F8;
        GETED(0);
        GETGX;
        if(rex.w)
            ED->q[0] = GX->q;
        else {
            if(MODREG)
                ED->q[0] = GX->ud[0];
            else
                ED->dword[0] = GX->ud[0];
        }
        break;

    case 0xD6:                      /* MOVQ Ex,Gx */
        nextop = F8;
        GETEX(0);
        GETGX;
        EX->q[0] = GX->q[0];
        if(MODREG)
            EX->q[1] = 0;
        break;

    case 0xEF:                      /* PXOR Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->q[0] ^= EX->q[0];
        GX->q[1] ^= EX->q[1];
        break;

    default:
        return 1;
    }
    return 0;
}