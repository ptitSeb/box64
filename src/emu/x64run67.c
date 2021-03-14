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

#include "modrm.h"

int Run67(x64emu_t *emu, rex_t rex)
{
    uint8_t opcode;
    uint8_t nextop;
    int8_t tmp8s;
    reg64_t *oped, *opgd;

    opcode = F8;

    // REX prefix before the F0 are ignored
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }

    switch(opcode) {

    case 0x8D:                      /* LEA Gd,M */
        nextop = F8;
        GETED32(0);
        GETGD;
        if(rex.w)
            GD->q[0] = (uint64_t)ED;
        else
            GD->q[0] = (uint32_t)(uintptr_t)ED;
        break;

    case 0xE0:                      /* LOOPNZ */
        CHECK_FLAGS(emu);
        tmp8s = F8S;
        --R_ECX; // don't update flags
        if(R_ECX && !ACCESS_FLAG(F_ZF))
            R_RIP += tmp8s;
        break;
    case 0xE1:                      /* LOOPZ */
        CHECK_FLAGS(emu);
        tmp8s = F8S;
        --R_ECX; // don't update flags
        if(R_ECX && ACCESS_FLAG(F_ZF))
            R_RIP += tmp8s;
        break;
    case 0xE2:                      /* LOOP */
        tmp8s = F8S;
        --R_ECX; // don't update flags
        if(R_ECX)
            R_RIP += tmp8s;
        break;
    case 0xE3:              /* JECXZ Ib */
        tmp8s = F8S;
        if(!R_ECX)
            R_RIP += tmp8s;
        break;

    default:
        return 1;
    }
    return 0;
}