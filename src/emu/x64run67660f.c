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
uintptr_t Test67660F(x64test_t *test, rex_t rex, uintptr_t addr)
#else
uintptr_t Run67660F(x64emu_t *emu, rex_t rex, uintptr_t addr)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    int8_t tmp8s;               (void)tmp8s;
    uint8_t tmp8u;              (void)tmp8u;
    int16_t tmp16s;             (void)tmp16s;
    uint16_t tmp16u;            (void)tmp16u;
    int32_t tmp32s;             (void)tmp32s;
    uint32_t tmp32u;            (void)tmp32u;
    int64_t tmp64s;             (void)tmp64s;
    uint64_t tmp64u;            (void)tmp64u;
    reg64_t *oped, *opgd;       (void)oped;   (void)opgd;
    sse_regs_t *opex, *opgx;
    #ifdef TEST_INTERPRETER
    x64emu_t* emu = test->emu;
    #endif
    opcode = F8;

    switch(opcode) {

    case 0x6F:                      /* MOVDQA Gx,Ex */
        nextop = F8;
        GETEX32(0);
        GETGX;
        GX->q[0] = EX->q[0];
        GX->q[1] = EX->q[1];
        break;
   
    case 0x76:                      /* PCMPEQD Gx,Ex */
        nextop = F8;
        GETEX32(0);
        GETGX;
        for (int i=0; i<4; ++i)
            GX->ud[i] = (GX->ud[i]==EX->ud[i])?0xffffffff:0;
        break;

    default:
        return 0;
    }
    return addr;
}
