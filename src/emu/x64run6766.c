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
#ifdef DYNAREC
#include "../dynarec/native_lock.h"
#endif

#include "modrm.h"

#ifdef TEST_INTERPRETER
uintptr_t Test6766(x64test_t *test, rex_t rex, int rep, uintptr_t addr)
#else
uintptr_t Run6766(x64emu_t *emu, rex_t rex, int rep, uintptr_t addr)
#endif
{
    // Hmmmm....
    (void)rep;
    uint8_t opcode;
    uint8_t nextop;                     (void)nextop;
    int8_t tmp8s;                       (void)tmp8s;
    uint8_t tmp8u, tmp8u2;              (void)tmp8u;  (void)tmp8u2;
    int16_t tmp16s;                     (void)tmp16s;
    uint16_t tmp16u, tmp16u2;           (void)tmp16u; (void)tmp16u2;
    int32_t tmp32s;                     (void)tmp32s;
    int64_t tmp64s;                     (void)tmp64s;
    uint64_t tmp64u, tmp64u2, tmp64u3;  (void)tmp64u; (void)tmp64u2; (void)tmp64u3;
    reg64_t *oped, *opgd;               (void)oped;   (void)opgd;
    #ifdef TEST_INTERPRETER
    x64emu_t* emu = test->emu;
    #endif
    opcode = F8;

    while((opcode==0x2E) || (opcode==0x66))   // ignoring CS: or multiple 0x66
        opcode = F8;

    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }
    // REX prefix before the 66 are ignored
    rex.rex = 0;
    if(!rex.is32bits)
        while(opcode>=0x40 && opcode<=0x4f) {
            rex.rex = opcode;
            opcode = F8;
        }

    switch(opcode) {

    case 0x0F:                              /* more opcodes */
        #ifdef TEST_INTERPRETER
        return Test67660F(test, rex, addr);
        #else
        return Run67660F(emu, rex, addr);
        #endif

    case 0x89:                              /* MOV Ew,Gw */
        nextop = F8;
        GETEW32(0);
        GETGW;
        if(rex.w)
            EW->q[0] = GW->q[0];
        else
            EW->word[0] = GW->word[0];
        break;

    case 0x8D:                              /* LEA Gw,M */
        nextop = F8;
        GETGW;
        tmp64u = GETEA32(0);
        if(rex.w)
            GW->q[0] = tmp64u;
        else
            GW->word[0] = (uint16_t)tmp64u;
        break;

    default:
        return 0;
    }
    return addr;
}
