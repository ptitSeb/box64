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
    int32_t tmp32s;
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
        GX->f[0] = ED->sdword[0];
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

    case 0x6F:  /* MOVDQU Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        memcpy(GX, EX, 16);    // unaligned...
        break;

    default:
        return 1;
    }
    return 0;
}