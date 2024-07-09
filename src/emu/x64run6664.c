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

uint8_t opcode = 0; // initialize with a default value
uint8_t nextop = 0;
uint16_t tmp16u = 0;
int16_t tmp16s = 0;
uint64_t tmp64u = 0;
reg64_t *oped, *opgd;
sse_regs_t *opex, *opgx;

#ifdef TEST_INTERPRETER
x64emu_t* emu = test->emu;
#else
#endif

uintptr_t tlsdata = GetSegmentBaseEmu(emu, seg);

opcode = F8;
// REX prefix before the F0 are ignored
rex.rex = 0;
if (!rex.is32bits)
    while (opcode >= 0x40 && opcode <= 0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }

switch (opcode) {

    case 0x0F:
        opcode = F8;
        switch (opcode) {

            case 0x11:                      /* MOVUPD Ex, Gx */
                nextop = F8;
                GETEX_OFFS(0, tlsdata);
                GETGX;
                memcpy(EX, GX, 16); // unaligned...
                break;

            case 0x2E:                      /* UCOMISD Gx, Ex */
                // no special check...
            case 0x2F:                      /* COMISD Gx, Ex */
                RESET_FLAGS(emu);
                nextop = F8;
                GETEX_OFFS(0, tlsdata);
                GETGX;
                uint64_t gx_val = GX->d[0];
                uint64_t ex_val = EX->d[0];
                if (isnan(gx_val) || isnan(ex_val)) {
                    SET_FLAG(F_ZF); SET_FLAG(F_PF); SET_FLAG(F_CF);
                } else if (isgreater(gx_val, ex_val)) {
                    CLEAR_FLAG(F_ZF); CLEAR_FLAG(F_PF); CLEAR_FLAG(F_CF);
                } else if (isless(gx_val, ex_val)) {
                    CLEAR_FLAG(F_ZF); CLEAR_FLAG(F_PF); SET_FLAG(F_CF);
                } else {
                    SET_FLAG(F_ZF); CLEAR_FLAG(F_PF); CLEAR_FLAG(F_CF);
                }
                CLEAR_FLAG(F_OF); CLEAR_FLAG(F_AF); CLEAR_FLAG(F_SF);
                break;

            case 0x6F:  /* MOVDQA Gx, Ex */
                nextop = F8;
                GETEX_OFFS(0, tlsdata);
                GETGX;
                GX->q[0] = EX->q[0];
                GX->q[1] = EX->q[1];
                break;

            case 0x7F:  /* MOVDQA Ex,Gx */
                nextop = F8;
                GETEX_OFFS(0, tlsdata);
                GETGX;
                EX->q[0] = GX->q[0];
                EX->q[1] = GX->q[1];
                break;

            case 0xD6:                      /* MOVQ Ex,Gx */
                nextop = F8;
                GETEX_OFFS(0, tlsdata);
                GETGX;
                EX->q[0] = GX->q[0];
                if (MODREG)
                    EX->q[1] = 0;
                break;

            default:
                return 0;
        }
        break;

    case 0x39: /* CMP FS:Ew,Gw */
        nextop = F8;
        GETEW_OFFS(0, tlsdata);
        GETGW;
        if (rex.w)
            cmp64(emu, EW->q[0], GW->q[0]);
        else
            cmp16(emu, EW->word[0], GW->word[0]);
        break;

    case 0x83:                              /* GRP3 Ew,Ib */
        uint8_t tmp8u = F8S;
        nextop = F8;
        GETEW_OFFS((opcode == 0x81) ? 2 : 1, tlsdata);
        GETGW;
        if (opcode == 0x81)
            tmp16u = tmp8u;
        else
            tmp16u = tmp8u;

        switch ((nextop >> 3) & 7) {
            case 0: EW->word[0] = add16(emu, EW->word[0], tmp16u); break;
            case 1: EW->word[0] = or16(emu, EW->word[0], tmp16u); break;
            case 2: EW->word[0] = adc16(emu, EW->word[0], tmp16u); break;
            case 3: EW->word[0] = sbb16(emu, EW->word[0], tmp16u); break;
            case 4: EW->word[0] = and16(emu, EW->word[0], tmp16u); break;
            case 5: EW->word[0] = sub16(emu, EW->word[0], tmp16u); break;
            case 6: EW->word[0] = xor16(emu, EW->word[0], tmp16u); break;
            case 7:               cmp16(emu, EW->word[0], tmp16u); break;
        }
        break;

    case 0x89:                              /* MOV FS:Ew,GW */
        nextop = F8;
        GETEW_OFFS(0, tlsdata);
        GETGW;
        if (rex.w)
            EW->q[0] = GW->q[0];
        else
            EW->word[0] = GW->word[0];
        break;

    case 0x8B:                      /* MOV Gw,FS:Ew */
        nextop = F8;
        GETGW;
        tmp64u = GETEA(0);
        if (rex.w)
            GW->q[0] = tmp64u;
        else
            GW->word[0] = (uint16_t)tmp64u;
        break;

    case 0x8D:                              /* LEA Gw,M */
        nextop = F8;
        GETGW;
        tmp64u = GETEA(0);
        if (rex.w)
            GW->q[0] = tmp64u;
        else
            GW->word[0] = (uint16_t)tmp64u;
        break;

    case 0xC7:                      /* MOV FS:Ew,Iw */
        nextop = F8;
        GETEW_OFFS(2, tlsdata);
        if (rex.w)
            EW->q[0] = F16S;
        else
            EW->word[0] = F16;
        break;

    default:
        return 0;
}
