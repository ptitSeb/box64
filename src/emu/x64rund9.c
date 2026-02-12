#define _GNU_SOURCE
#include <fenv.h>
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
#include "x64emu_private.h"
#include "x64run_private.h"
#include "x64primop.h"
#include "x64trace.h"
#include "x87emu_private.h"
#include "x87emu_setround.h"
#include "box64context.h"
#include "bridge.h"

#include "modrm.h"

#ifdef TEST_INTERPRETER
uintptr_t TestD9(x64test_t *test, rex_t rex, uintptr_t addr)
#else
uintptr_t RunD9(x64emu_t *emu, rex_t rex, uintptr_t addr)
#endif
{
    uint8_t nextop;
    int32_t tmp32s;
    uint64_t ll;
    float f;
    reg64_t *oped;
    int oldround;
    #ifdef TEST_INTERPRETER
    x64emu_t*emu = test->emu;
    #endif

    nextop = F8;
    if(MODREG)
    switch (nextop) {
        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:  /* FLD STx */
            ll = ST(nextop&7).q;
            fpu_do_push(emu);
            ST0.q = ll;
            break;
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:  /* FXCH STx */
            ll = ST(nextop&7).q;
            ST(nextop&7).q = ST0.q;
            ST0.q = ll;
            break;

        case 0xD0:  /* FNOP */
            break;

        case 0xD8:  /* FSTPNCE ST0, STx */
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:
            ST(nextop&7).q = ST0.q;
            fpu_do_pop(emu);
            break;
        case 0xE0:  /* FCHS */
            ST0.d = -ST0.d;
            break;
        case 0xE1:  /* FABS */
            ST0.d = fabs(ST0.d);
            break;
        
        case 0xE4:  /* FTST */
            fpu_ftst(emu);
            break;
        case 0xE5:  /* FXAM */
            fpu_fxam(emu);
            break;

        case 0xE8:  /* FLD1 */
            fpu_do_push(emu);
            ST0.d = 1.0;
            break;
        case 0xE9:  /* FLDL2T */
            fpu_do_push(emu);
            ST0.d = L2T;
            break;
        case 0xEA:  /* FLDL2E */
            fpu_do_push(emu);
            ST0.d = L2E;
            break;
        case 0xEB:  /* FLDPI */
            fpu_do_push(emu);
            ST0.d = PI;
            break;
        case 0xEC:  /* FLDLG2 */
            fpu_do_push(emu);
            ST0.d = LG2;
            break;
        case 0xED:  /* FLDLN2 */
            fpu_do_push(emu);
            ST0.d = LN2;
            break;
        case 0xEE:  /* FLDZ */
            fpu_do_push(emu);
            ST0.d = 0.0;
            break;

        case 0xF0:  /* F2XM1 */
            if (ST0.d == 0)
                break;
            // Using the expm1 instead of exp2(ST0)-1 can avoid losing precision much,
            // expecially when ST0 is close to zero (which loses the precise when -1).
            // printf("%a, %a\n", LN2 * ST0.d, expm1(LN2 * ST0.d));
            ST0.d = expm1(LN2 * ST0.d);
            //    = 2^ST0 - 1 + error. (in math)
            break;
        case 0xF1:  /* FYL2X */
            ST(1).d *= log2(ST0.d);
            fpu_do_pop(emu);
            break;
        case 0xF2:  /* FPTAN */
            oldround = fpu_setround(emu);
            ST0.d = tan(ST0.d);
            fesetround(oldround);
            fpu_do_push(emu);
            ST0.d = 1.0;
            emu->sw.f.F87_C2 = 0;
            break;
        case 0xF3:  /* FPATAN */
            oldround = fpu_setround(emu);
            ST1.d = atan2(ST1.d, ST0.d);
            fesetround(oldround);
            fpu_do_pop(emu);
            break;
        case 0xF4:  /* FXTRACT */
            fpu_do_push(emu);
            if(isnan(ST1.d)) {
                ST0.d = ST1.d;
            } else if(isinf(ST1.d)) {
                ST0.d = ST1.d;
                ST1.d = INFINITY;
            } else if(ST1.d==0.0) {
                ST0.d = ST1.d;
                ST1.d = -INFINITY;
            } else {
                // LD80bits doesn't have implicit "1" bit, so need to adjust for that
                ST0.d = frexp(ST1.d, &tmp32s)*2;
                ST1.d = tmp32s-1;
            }
            break;

        case 0xF5:  /* FPREM1 */
            {
                double x = ST0.d, y = ST1.d;
                int q = 0;
                if (isnan(x) || isnan(y)) {
                    ST0.d = NAN;
                } else if (isinf(x) || y == 0.0) {
#if !defined(_WIN32) && !defined(__MINGW32__)
                    feraiseexcept(FE_INVALID);
#endif
                    ST0.d = NAN;
                } else {
#if defined(_WIN32) || defined(__MINGW32__)
                    q = (int)round(x / y);
                    ST0.d = x - (y * q);
#else
                    ST0.d = remquo(x, y, &q);
#endif
                }
                q &= 7;
                emu->sw.f.F87_C2 = 0;
                emu->sw.f.F87_C1 = q & 1;
                emu->sw.f.F87_C3 = (q >> 1) & 1;
                emu->sw.f.F87_C0 = (q >> 2) & 1;
            }
            break;
        case 0xF6:  /* FDECSTP */
            emu->top=(emu->top-1)&7;    // this will probably break a few things
            break;
        case 0xF7:  /* FINCSTP */
            if(emu->fpu_tags&0b11)
                fpu_do_pop(emu);
            else
                emu->top=(emu->top+1)&7;    // this will probably break a few things
            break;
        case 0xF8:  /* FPREM */
            {
                double x = ST0.d, y = ST1.d;
                int64_t q = 0;
                if (isnan(x) || isnan(y)) {
                    ST0.d = NAN;
                    q = 0;
                } else if (isinf(x) || y == 0.0) {
#if !defined(_WIN32) && !defined(__MINGW32__)
                    feraiseexcept(FE_INVALID);
#endif
                    ST0.d = NAN;
                    q = 0;
                } else {
#if defined(_WIN32) || defined(__MINGW32__)
                    q = (int64_t)trunc(x / y);
                    ST0.d = x - (y * q);
#else
                    ST0.d = fmod(x, y);
                    q = (int64_t)trunc(x / y);
#endif
                }
                q &= 7;
                emu->sw.f.F87_C2 = 0;
                emu->sw.f.F87_C1 = q & 1;
                emu->sw.f.F87_C3 = (q >> 1) & 1;
                emu->sw.f.F87_C0 = (q >> 2) & 1;
            }
            break;
        case 0xF9:  /* FYL2XP1 */
            // Using the log1p instead of log2(ST0+1) can avoid losing precision much,
            // expecially when ST0 is close to zero (which loses the precise when +1).
            ST(1).d = (ST(1).d * log1p(ST0.d)) / M_LN2;
            //      = ST1 * log2(ST0 + 1) + error. (in math)
            fpu_do_pop(emu);
            break;
        case 0xFA:  /* FSQRT */
            oldround = fpu_setround(emu);
            ST0.d = sqrt(ST0.d);
            if(!emu->cw.f.C87_PC) ST0.d = (float)ST0.d;
            fesetround(oldround);
            break;
        case 0xFB:  /* FSINCOS */
            fpu_do_push(emu);
            oldround = fpu_setround(emu);
            sincos(ST1.d, &ST1.d, &ST0.d);
            fesetround(oldround);
            emu->sw.f.F87_C2 = 0;
            break;
        case 0xFC:  /* FRNDINT */
            ST0.d = fpu_round(emu, ST0.d);
            break;
        case 0xFD:  /* FSCALE */
            // this could probably be done by just altering the exponant part of the float...
            if (ST1.d > INT32_MAX)
                tmp32s = INT32_MAX;
            else if (ST1.d < INT32_MIN)
                tmp32s = INT32_MIN;
            else
                tmp32s = ST1.d;
            if(ST0.d!=0.0) {
                oldround = fpu_setround(emu);
                ST0.d = ldexp(ST0.d, tmp32s);
                fesetround(oldround);
            }
            break;
        case 0xFE:  /* FSIN */
            oldround = fpu_setround(emu);
            ST0.d = sin(ST0.d);
            fesetround(oldround);
            emu->sw.f.F87_C2 = 0;
            break;
        case 0xFF:  /* FCOS */
            oldround = fpu_setround(emu);
            ST0.d = cos(ST0.d);
            fesetround(oldround);
            emu->sw.f.F87_C2 = 0;
            break;


        default:
            return 0;
    } else
        switch((nextop>>3)&7) {
            case 0:     /* FLD ST0, Ed float */
                GETE4(0);
                fpu_do_push(emu);
                ST0.d = *(float*)ED;
                break;
            case 2:     /* FST Ed, ST0 */
                GETE4(0);
                oldround = fpu_setround(emu);
                *(float*)ED = ST0.d;
                fesetround(oldround);
                break;
            case 3:     /* FSTP Ed, ST0 */
                GETE4(0);
                oldround = fpu_setround(emu);
                *(float*)ED = ST0.d;
                fesetround(oldround);
                fpu_do_pop(emu);
                break;
            case 4:     /* FLDENV m */
                // warning, incomplete
                _GETED(0);
                fpu_loadenv(emu, (char*)ED, 0);
                break;
            case 5:     /* FLDCW Ew */
                GETEW(0);
                emu->cw.x16 = EW->word[0];
                // do something with cw?
                break;
            case 6:     /* FNSTENV m */
                // warning, incomplete
                GETE8(0);
                fpu_savenv(emu, (char*)ED, 0);
                // intruction pointer: 48bits
                // data (operand) pointer: 48bits
                // last opcode: 11bits save: 16bits restaured (1st and 2nd opcode only)
                break;
            case 7: /* FNSTCW Ew */
                GETEW(0);
                EW->word[0] = emu->cw.x16;
                break;
            default:
                return 0;
        }
    return addr;
}
