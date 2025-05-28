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
#include "box64cpu_util.h"
#include "x64emu.h"
#include "x64emu_private.h"
#include "x64run_private.h"
#include "x64primop.h"
#include "x64trace.h"
#include "x87emu_private.h"
#include "box64context.h"
#include "bridge.h"

#include "modrm.h"

#ifdef TEST_INTERPRETER
uintptr_t Test6764(x64test_t *test, rex_t rex, int rep, int seg, uintptr_t addr)
#else
uintptr_t Run6764(x64emu_t *emu, rex_t rex, int rep, int seg, uintptr_t addr)
#endif
{
    (void)rep;
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

        case 0x89:                      /* MOV FS:Ed, Gd */
            nextop = F8;
            GETED_OFFS_32(0, tlsdata);
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

        case 0x8B:                      /* MOV Gd, FS:Ed */
            nextop = F8;
            GETED_OFFS_32(0, tlsdata);
            GETGD;
            if(rex.w)
                GD->q[0] = ED->q[0];
            else
                GD->q[0] = ED->dword[0];
            break;

    default:
        return 0;
    }
    return addr;
}
