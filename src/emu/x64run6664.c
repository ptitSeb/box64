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

int Run6664(x64emu_t *emu, rex_t rex)
{
    uint8_t opcode;
    uint8_t nextop;
    reg64_t *oped, *opgd;
    sse_regs_t *opex, *opgx;
    uintptr_t tlsdata = GetFSBaseEmu(emu);

    opcode = F8;
    // REX prefix before the F0 are ignored
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }

    switch(opcode) {

        case 0x0F:
            opcode = F8;
            switch(opcode) {
                case 0xD6:                      /* MOVQ Ex,Gx */
                    nextop = F8;
                    GETEX_OFFS(0, tlsdata);
                    GETGX;
                    EX->q[0] = GX->q[0];
                    if(MODREG)
                        EX->q[1] = 0;
                    break;

                default:
                    return 1;
            }
            break;

        case 0x89:                              /* MOV Ew,Gw */
            nextop = F8;
            GETEW_OFFS(0, tlsdata);
            GETGW;
            if(rex.w)
                EW->q[0] = GW->q[0];
            else
                EW->word[0] = GW->word[0];
            break;

        case 0x8B:                      /* MOV Gd,Ed */
            nextop = F8;
            GETEW_OFFS(0, tlsdata);
            GETGW;
            if(rex.w)
                GW->q[0] = EW->q[0];
            else
                GW->word[0] = EW->word[0];
            break;

       default:
            return 1;
    }
    return 0;
}