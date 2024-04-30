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
uintptr_t TestDD(x64test_t *test, rex_t rex, uintptr_t addr)
#else
uintptr_t RunDD(x64emu_t *emu, rex_t rex, uintptr_t addr)
#endif
{
    uint8_t nextop;
    reg64_t *oped;
    #ifdef TEST_INTERPRETER
    x64emu_t*emu = test->emu;
    #endif

    nextop = F8;
    if(MODREG)
    switch (nextop) {
        case 0xC0:  /* FFREE STx */
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:
            fpu_do_free(emu, nextop&7);
            break;

        case 0xD0:  /* FST ST0, STx */
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
            ST(nextop&7).q = ST0.q;
            break;
        case 0xD8:  /* FSTP ST0, STx */
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
        case 0xE0:  /* FUCOM ST0, STx */
        case 0xE1:
        case 0xE2:
        case 0xE3:
        case 0xE4:
        case 0xE5:
        case 0xE6:
        case 0xE7:
            fpu_fcom(emu, ST(nextop&7).d);   // bad, should handle QNaN and IA interrupt
            break;
        case 0xE8:  /* FUCOMP ST0, STx */
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
            fpu_fcom(emu, ST(nextop&7).d);   // bad, should handle QNaN and IA interrupt
            fpu_do_pop(emu);
            break;

        default:
            return 0;
    } else
        switch((nextop>>3)&7) {
            case 0: /* FLD double */
                GETE8(0);
                fpu_do_push(emu);
                ST0.d = *(double*)ED;
                break;
            case 1: /* FISTTP ED qword */
                GETE8(0);
                if(STll(0).sref==ST(0).sq)
                    ED->sq[0] = STll(0).sq;
                else {
                    if(isgreater(ST0.d, (double)0x7fffffffffffffffLL) || isless(ST0.d, -(double)0x8000000000000000LL) || !isfinite(ST0.d))
                        *(uint64_t*)ED = 0x8000000000000000LL;
                    else
                        *(int64_t*)ED = ST0.d;
                }
                fpu_do_pop(emu);
                break;
            case 2: /* FST double */
                GETE8(0);
                *(double*)ED = ST0.d;
                break;
            case 3: /* FSTP double */
                GETE8(0);
                *(double*)ED = ST0.d;
                fpu_do_pop(emu);
                break;
            case 4: /* FRSTOR m108byte */
                _GETED(0);
                fpu_loadenv(emu, (char*)ED, 0);
                // get the STx
                {
                    char* p =(char*)ED;
                    p += 28;
                    for (int i=0; i<8; ++i) {
                        LD2D(p, &ST(i).d);
                        p+=10;
                    }
                }
                break;
            case 6: /* FNSAVE m108byte */
                _GETED(0);
                // ENV first...
                // warning, incomplete
                #ifndef TEST_INTERPRETER
                fpu_savenv(emu, (char*)ED, 0);
                // save the STx
                {
                    char* p =(char*)ED;
                    p += 28;
                    for (int i=0; i<8; ++i) {
                        D2LD(&ST(i).d, p);
                        p+=10;
                    }
                }
                #endif
                reset_fpu(emu);
                break;
            case 7: /* FNSTSW m2byte */
                GETEW(0);
                emu->sw.f.F87_TOP = emu->top&7;
                *(uint16_t*)ED = emu->sw.x16;
                break;
            default:
                return 0;
        }
   return addr;
}