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
#include "../dynarec/arm_lock_helper.h"
#endif

#include "modrm.h"

int RunDD(x64emu_t *emu, rex_t rex)
{
    uint8_t nextop;
    reg64_t *oped;

    nextop = F8;
    switch (nextop) {
    case 0xC0:  /* FFREE STx */
    case 0xC1:
    case 0xC2:
    case 0xC3:
    case 0xC4:
    case 0xC5:
    case 0xC6:
    case 0xC7:
        fpu_do_free(emu, nextop-0xC0);
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

    case 0xC8:
    case 0xC9:
    case 0xCA:
    case 0xCB:
    case 0xCC:
    case 0xCD:
    case 0xCE:
    case 0xCF:
    case 0xF0:
    case 0xF1:
    case 0xF2:
    case 0xF3:
    case 0xF4:
    case 0xF5:
    case 0xF6:
    case 0xF7:
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
            case 0: /* FLD double */
                GETED(0);
                fpu_do_push(emu);
                if(!(((uintptr_t)ED)&7))
                    ST0.d = *(double*)ED;
                else {
                    memcpy(&ST0.d, ED, sizeof(double));
                }
                break;
            case 1: /* FISTTP ED qword */
                GETED(0);
                if(!(((uintptr_t)ED)&7))
                    *(int64_t*)ED = ST0.d;
                else {
                    int64_t i64 = ST0.d;
                    memcpy(ED, &i64, sizeof(int64_t));
                }
                fpu_do_pop(emu);
                break;
            case 2: /* FST double */
                GETED(0);
                if(!(((uintptr_t)ED)&7))
                    *(double*)ED = ST0.d;
                else {
                    memcpy(ED, &ST0.d, sizeof(double));
                }
                break;
            case 3: /* FSTP double */
                GETED(0);
                if(!(((uintptr_t)ED)&7))
                    *(double*)ED = ST0.d;
                else {
                    memcpy(ED, &ST0.d, sizeof(double));
                }
                fpu_do_pop(emu);
                break;
            case 4: /* FRSTOR m108byte */
                GETED(0);
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
                GETED(0);
                // ENV first...
                // warning, incomplete
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
                reset_fpu(emu);
                break;
            case 7: /* FNSTSW m2byte */
                GETED(0);
                emu->sw.f.F87_TOP = emu->top&7;
                *(uint16_t*)ED = emu->sw.x16;
                break;
            default:
                return 1;
        }
    }
   return 0;
}