#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_functions.h"
#include "../dynarec_helper.h"

static const char* avx_prefix_string(uint16_t p)
{
    switch(p) {
        case VEX_P_NONE: return "0";
        case VEX_P_66: return "66";
        case VEX_P_F2: return "F2";
        case VEX_P_F3: return "F3";
        default: return "??";
    }
}
static const char* avx_map_string(uint16_t m)
{
    switch(m) {
        case VEX_M_NONE: return "0";
        case VEX_M_0F: return "0F";
        case VEX_M_0F38: return "0F38";
        case VEX_M_0F3A: return "0F3A";
        default: return "??";
    }
}

typedef uintptr_t (*dynarec64_avx_f)(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);

uintptr_t dynarec64_AVX(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = PK(0);
    rex_t rex = vex.rex;
    static const dynarec64_avx_f avx_dispatch[4][4] = {
        [VEX_M_0F] = {
            [VEX_P_NONE] = dynarec64_AVX_0F,
            [VEX_P_66]   = dynarec64_AVX_66_0F,
            [VEX_P_F3]   = dynarec64_AVX_F3_0F,
            [VEX_P_F2]   = dynarec64_AVX_F2_0F,
        },
        [VEX_M_0F38] = {
            [VEX_P_NONE] = dynarec64_AVX_0F38,
            [VEX_P_66]   = dynarec64_AVX_66_0F38,
            [VEX_P_F3]   = dynarec64_AVX_F3_0F38,
            [VEX_P_F2]   = dynarec64_AVX_F2_0F38,
        },
        [VEX_M_0F3A] = {
            [VEX_P_66]   = dynarec64_AVX_66_0F3A,
            [VEX_P_F2]   = dynarec64_AVX_F2_0F3A,
        },
    };

    if(vex.m < 4 && vex.p < 4 && avx_dispatch[vex.m][vex.p])
        addr = avx_dispatch[vex.m][vex.p](dyn, addr, ip, ninst, vex, ok, need_epilog);
    else {
        DEFAULT;
    }

    if((*ok==-1) && (BOX64ENV(dynarec_log)>=LOG_INFO || dyn->need_dump || BOX64ENV(dynarec_missing)))
        if(!dyn->size || BOX64ENV(dynarec_log)>LOG_INFO || dyn->need_dump) {
            dynarec_log(LOG_NONE, "  Dynarec unimplemented %s opcode size %d prefix %s map %s opcode %02X\n",
                vex.evex ? "EVEX" : "VEX", 128<<vex.l, avx_prefix_string(vex.p), avx_map_string(vex.m), opcode);
    }
    return addr;
}
