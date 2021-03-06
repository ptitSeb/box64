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

int Run64(x64emu_t *emu, rex_t rex)
{
    uint8_t opcode;
    uint8_t nextop;
    uint8_t tmp8u;
    int16_t tmp16s;
    uint16_t tmp16u;
    reg64_t *oped, *opgd;
    uintptr_t tlsdata = GetFSBaseEmu(emu);

    opcode = F8;
    // REX prefix before the F0 are ignored
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }

    switch(opcode) {

        case 0x88:                      /* MOV Eb,Gb */
            nextop = F8;
            GETEB_OFFS(0, tlsdata);
            GETGB;
            EB->byte[0] = GB;
            break;
        case 0x89:                    /* MOV Ed,Gd */
            nextop = F8;
            GETED_OFFS(0, tlsdata);
            GETGD;
            if(rex.w) {
                ED->q[0] = GD->q[0];
            } else {
                //if ED is a reg, than the opcode works like movzx
                if(MODREG)
                    ED->q[0] = GD->dword[0];
                else
                    ED->dword[0] = GD->dword[0];
            }
            break;

        case 0x8B:                      /* MOV Gd,Ed */
            nextop = F8;
            GETED_OFFS(0, tlsdata);
            GETGD;
            if(rex.w)
                GD->q[0] = ED->q[0];
            else
                GD->q[0] = ED->dword[0];
            break;

        case 0xC7:                      /* MOV Ed,Id */
            nextop = F8;
            GETED_OFFS(4, tlsdata);
            if(rex.w)
                ED->q[0] = F32S64;
            else
                if(MODREG)
                    ED->q[0] = F32;
                else
                    ED->dword[0] = F32;
            break;

       default:
            return 1;
    }
    return 0;
}