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

#ifdef TEST_INTERPRETER
uintptr_t Test6764_32(x64test_t *test, rex_t rex, int rep, int seg, uintptr_t addr)
#else
uintptr_t Run6764_32(x64emu_t *emu, rex_t rex, int rep, int seg, uintptr_t addr)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    int8_t tmp8s;
    uint8_t tmp8u;
    uint32_t tmp32u;
    int32_t tmp32s;
    uint64_t tmp64u;
    reg64_t *oped, *opgd;
    #ifdef TEST_INTERPRETER
    x64emu_t* emu = test->emu;
    #endif
    uintptr_t tlsdata = GetSegmentBaseEmu(emu, seg);
    opcode = F8;

    while(opcode==0x67)
        opcode = F8;

    rex.rex = 0;
    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }

    switch(opcode) {

        case 0x8B:                      /* MOV Gw, FS:Ew */
            nextop = F8;
            GETEW_OFFS_16(tlsdata);
            GETGW;
            GW->word[0] = EW->word[0];
            break;

        case 0xFF:
            nextop = F8;
            GETEW_OFFS_16(tlsdata);
            switch((nextop>>3)&7) {
                case 6: // Push Ed
                    tmp32u = ED->dword[0];
                    Push32(emu, tmp32u);
                    break;
                default:
                    return 0;
            }
            break;

    default:
        return 0;
    }
    return addr;
}