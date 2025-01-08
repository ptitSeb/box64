#ifndef __SETROUND_H__
#define __SETROUND_H__
#pragma STDC FENV_ACCESS ON
#include <fenv.h>
#include <stdint.h>
#include "x64emu.h"
#include "x64emu_private.h"
// set the rounding mode to the emulator's one, and return the old one
static inline int fpu_setround(x64emu_t* emu) {
    int ret = fegetround();
    int rounding_direction;
    switch (emu->cw.f.C87_RD) {
        case ROUND_Nearest:
            rounding_direction = FE_TONEAREST;
            break;
        case ROUND_Down:
            rounding_direction = FE_DOWNWARD;
            break;
        case ROUND_Up:
            rounding_direction = FE_UPWARD;
            break;
        case ROUND_Chop:
            rounding_direction = FE_TOWARDZERO;
            break;
    }
    fesetround(rounding_direction);
    return ret;
}
#endif