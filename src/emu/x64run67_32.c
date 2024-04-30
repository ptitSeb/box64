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
uintptr_t Test67_32(x64test_t *test, rex_t rex, int rep, uintptr_t addr)
#else
uintptr_t Run67_32(x64emu_t *emu, rex_t rex, int rep, uintptr_t addr)
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
    opcode = F8;

    while(opcode==0x67)
        opcode = F8;

    rex.rex = 0;
    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }

    switch(opcode) {

    case 0x64:                      /* FS: prefix */
        #ifdef TEST_INTERPRETER
        return Test6764_32(test, rex, rep, _FS, addr);
        #else
        return Run6764_32(emu, rex, rep, _FS, addr);
        #endif
        break;
    case 0x65:                      /* GS: prefix */
        #ifdef TEST_INTERPRETER
        return Test6764_32(test, rex, rep, _GS, addr);
        #else
        return Run6764_32(emu, rex, rep, _GS, addr);
        #endif
        break;

    case 0xE0:                      /* LOOPNZ */
        CHECK_FLAGS(emu);
        tmp8s = F8S;
        --R_CX; // don't update flags
        if(R_CX && !ACCESS_FLAG(F_ZF))
            addr += tmp8s;
        break;
    case 0xE1:                      /* LOOPZ */
        CHECK_FLAGS(emu);
        tmp8s = F8S;
        --R_CX; // don't update flags
        if(R_CX && ACCESS_FLAG(F_ZF))
            addr += tmp8s;
        break;
    case 0xE2:                      /* LOOP */
        tmp8s = F8S;
        --R_CX; // don't update flags
        if(R_CX)
            addr += tmp8s;
        break;
    case 0xE3:              /* JECXZ Ib */
        tmp8s = F8S;
        if(!R_CX)
            addr += tmp8s;
        break;

    default:
        return 0;
    }
    return addr;
}