#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"

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

uintptr_t dynarec64_AVX(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = PK(0);
    rex_t rex = vex.rex;

    if( (vex.m==VEX_M_0F) && (vex.p==VEX_P_NONE))
        addr = dynarec64_AVX_0F(dyn, addr, ip, ninst, vex, ok, need_epilog);
    else if( (vex.m==VEX_M_0F38) && (vex.p==VEX_P_NONE))
        addr = dynarec64_AVX_0F38(dyn, addr, ip, ninst, vex, ok, need_epilog);
    else if( (vex.m==VEX_M_0F) && (vex.p==VEX_P_66))
        addr = dynarec64_AVX_66_0F(dyn, addr, ip, ninst, vex, ok, need_epilog);
    else if( (vex.m==VEX_M_0F) && (vex.p==VEX_P_F2))
        addr = dynarec64_AVX_F2_0F(dyn, addr, ip, ninst, vex, ok, need_epilog);
    else if( (vex.m==VEX_M_0F) && (vex.p==VEX_P_F3))
        addr = dynarec64_AVX_F3_0F(dyn, addr, ip, ninst, vex, ok, need_epilog);
    else if( (vex.m==VEX_M_0F38) && (vex.p==VEX_P_66))
        addr = dynarec64_AVX_66_0F38(dyn, addr, ip, ninst, vex, ok, need_epilog);
    else if( (vex.m==VEX_M_0F3A) && (vex.p==VEX_P_66))
        addr = dynarec64_AVX_66_0F3A(dyn, addr, ip, ninst, vex, ok, need_epilog);
    else if( (vex.m==VEX_M_0F38) && (vex.p==VEX_P_F2))
        addr = dynarec64_AVX_F2_0F38(dyn, addr, ip, ninst, vex, ok, need_epilog);
    else if( (vex.m==VEX_M_0F3A) && (vex.p==VEX_P_F2))
        addr = dynarec64_AVX_F2_0F3A(dyn, addr, ip, ninst, vex, ok, need_epilog);
    else {DEFAULT;}

    if((*ok==-1) && (box64_dynarec_log>=LOG_INFO || box64_dynarec_dump || box64_dynarec_missing)) {
        dynarec_log(LOG_NONE, "Dynarec unimplemented AVX opcode size %d prefix %s map %s opcode %02X ", 128<<vex.l, avx_prefix_string(vex.p), avx_map_string(vex.m), opcode);
    }
    return addr;
}
