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
uintptr_t TestDF(x64test_t *test, rex_t rex, uintptr_t addr)
#else
uintptr_t RunDF(x64emu_t *emu, rex_t rex, uintptr_t addr)
#endif
{
    uint8_t nextop;
    int16_t tmp16s;
    int64_t tmp64s;
    reg64_t *oped;
    #ifdef TEST_INTERPRETER
    x64emu_t*emu = test->emu;
    #endif

    nextop = F8;
    if(MODREG)
    switch(nextop) {
        case 0xC0:  /* FFREEP STx */
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:
            fpu_do_free(emu, nextop-0xC0);
            fpu_do_pop(emu);
            break;

        case 0xD0:  /* FSTP STx, ST0 */
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
            ST(nextop&7).q = ST0.q;
            fpu_do_pop(emu);
            break;

        case 0xE0:  /* FNSTSW AX */
            emu->sw.f.F87_TOP = emu->top&7;
            R_AX = emu->sw.x16;
            break;

        case 0xE8:  /* FUCOMIP ST0, STx */
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
            fpu_fcomi(emu, ST(nextop&7).d);   // bad, should handle QNaN and IA interrupt
            fpu_do_pop(emu);
            break;

        case 0xF0:  /* FCOMIP ST0, STx */
        case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
            fpu_fcomi(emu, ST(nextop&7).d);
            fpu_do_pop(emu);
            break;

        default:
            return 0;
    } else
        switch((nextop>>3)&7) {
        case 0: /* FILD ST0, Gw */
            GETEW(0);
            tmp16s = EW->sword[0];
            fpu_do_push(emu);
            ST0.d = tmp16s;
            break;
        case 1: /* FISTTP Ew, ST0 */
            GETEW(0);
            tmp16s = ST0.d;
            if(isgreater(ST0.d, (double)(int32_t)0x7fff) || isless(ST0.d, -(double)(int32_t)0x8000) || !isfinite(ST0.d))
                EW->sword[0] = 0x8000;
            else
                EW->sword[0] = tmp16s;
            fpu_do_pop(emu);
            break;
        case 2: /* FIST Ew, ST0 */
            GETEW(0);
            if(isgreater(ST0.d, (double)(int32_t)0x7fff) || isless(ST0.d, -(double)(int32_t)0x8000) || !isfinite(ST0.d))
                EW->sword[0] = 0x8000;
            else
                EW->sword[0] = fpu_round(emu, ST0.d);
            break;
        case 3: /* FISTP Ew, ST0 */
            GETEW(0);
            if(isgreater(ST0.d, (double)(int32_t)0x7fff) || isless(ST0.d, -(double)(int32_t)0x8000) || !isfinite(ST0.d))
                EW->sword[0] = 0x8000;
            else
                EW->sword[0] = fpu_round(emu, ST0.d);
            fpu_do_pop(emu);
            break;
        case 4: /* FBLD ST0, tbytes */
            GETET(0);
            fpu_do_push(emu);
            fpu_fbld(emu, (uint8_t*)ED);
            break;
        case 5: /* FILD ST0, Gq */
            GETE8(0);
            tmp64s = ED->sq[0];
            fpu_do_push(emu);
            ST0.d = tmp64s;
            STll(0).sq = tmp64s;
            STll(0).sref = ST0.sq;
            break;
        case 6: /* FBSTP tbytes, ST0 */
            GETET(0);
            fpu_fbst(emu, (uint8_t*)ED);
            fpu_do_pop(emu);
            break;
        case 7: /* FISTP i64 */
            GETE8(0);
            if(STll(0).sref==ST(0).sq)
                ED->sq[0] = STll(0).sq;
            else {
                if(isgreater(ST0.d, (double)0x7fffffffffffffffLL) || isless(ST0.d, -(double)0x8000000000000000LL) || !isfinite(ST0.d))
                    ED->sq[0] = 0x8000000000000000LL;
                else
                    ED->sq[0] = fpu_round(emu, ST0.d);
            }
            fpu_do_pop(emu);
            break;
        default:
            return 0;
        }
    return addr;
}
