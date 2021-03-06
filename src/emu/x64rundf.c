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
//#include "my_cpuid.h"
#include "bridge.h"
//#include "signals.h"
#ifdef DYNAREC
#include "../dynarec/arm_lock_helper.h"
#endif

#include "modrm.h"

int RunDF(x64emu_t *emu, rex_t rex)
{
    uint8_t nextop;
    int16_t tmp16s;
    reg64_t *oped;

    nextop = F8;
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

    case 0xC8:
    case 0xC9:
    case 0xCA:
    case 0xCB:
    case 0xCC:
    case 0xCD:
    case 0xCE:
    case 0xCF:
    case 0xD0:
    case 0xD1:
    case 0xD2:
    case 0xD3:
    case 0xD4:
    case 0xD5:
    case 0xD6:
    case 0xD7:
    case 0xD8:
    case 0xD9:
    case 0xDA:
    case 0xDB:
    case 0xDC:
    case 0xDD:
    case 0xDE:
    case 0xDF:
    case 0xE1:
    case 0xE2:
    case 0xE3:
    case 0xE4:
    case 0xE5:
    case 0xE6:
    case 0xE7:
    case 0xF8:
    case 0xF9:
    case 0xFA:
    case 0xFB:
    case 0xFC:
    case 0xFD:
    case 0xFE:
    case 0xFF:
        return 1;

    default:
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
            EW->sword[0] = tmp16s;
            fpu_do_pop(emu);
            break;
        case 2: /* FIST Ew, ST0 */
            GETEW(0);
            if(isgreater(ST0.d, (double)(int32_t)0x7fff) || isless(ST0.d, -(double)(int32_t)0x7fff) || !isfinite(ST0.d))
                EW->sword[0] = 0x8000;
            else
                EW->sword[0] = fpu_round(emu, ST0.d);
            break;
        case 3: /* FISTP Ew, ST0 */
            GETEW(0);
            if(isgreater(ST0.d, (double)(int32_t)0x7fff) || isless(ST0.d, -(double)(int32_t)0x7fff) || !isfinite(ST0.d))
                EW->sword[0] = 0x8000;
            else
                EW->sword[0] = fpu_round(emu, ST0.d);
            fpu_do_pop(emu);
            break;
        #if 0
        case 4: /* FBLD ST0, tbytes */
            GETED(0);
            fpu_do_push(emu);
            fpu_fbld(emu, (uint8_t*)ED);
            break;
        case 5: /* FILD ST0, Gq */
            GETED(0);
            tmp64s = *(int64_t*)ED;
            fpu_do_push(emu);
            ST0.d = tmp64s;
            STll(0).ll = tmp64s;
            STll(0).ref = ST0.ll;
            break;
        case 6: /* FBSTP tbytes, ST0 */
            GETED(0);
            fpu_fbst(emu, (uint8_t*)ED);
            fpu_do_pop(emu);
            break;
        case 7: /* FISTP i64 */
            GETED(0);
            if((uintptr_t)ED & 0x7) {
                // un-aligned!
                if(STll(0).ref==ST(0).ll)
                    memcpy(ED, &STll(0).ll, sizeof(int64_t));
                else {
                    int64_t i64;
                    if(isgreater(ST0.d, (double)(int64_t)0x7fffffffffffffffLL) || isless(ST0.d, -(double)(int64_t)0x7fffffffffffffffLL) || !isfinite(ST0.d))
                        i64 = 0x8000000000000000LL;
                    else
                        i64 = fpu_round(emu, ST0.d);
                    memcpy(ED, &i64, sizeof(int64_t));
                }
            } else {
                if(STll(0).ref==ST(0).ll)
                    *(int64_t*)ED = STll(0).ll;
                else {
                    if(isgreater(ST0.d, (double)(int64_t)0x7fffffffffffffffLL) || isless(ST0.d, -(double)(int64_t)0x7fffffffffffffffLL) || !isfinite(ST0.d))
                        *(int64_t*)ED = 0x8000000000000000LL;
                    else
                        *(int64_t*)ED = fpu_round(emu, ST0.d);
                }
            }
            fpu_do_pop(emu);
            break;
        #endif
        default:
            return 1;
        }
    }
    return 0;
}