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

uintptr_t dynarec64_67_AVX(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;
    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed, vd;
    uint8_t wback, wb1, wb2;
    uint8_t eb1, eb2, gb1, gb2;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1, v2;
    int q0, q1, q2;
    int d0, d1, d2;
    int s0;
    uint64_t tmp64u;
    int64_t j64;
    int64_t fixedaddress;
    int unscaled;
    MAYUSE(wb1);
    MAYUSE(wb2);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(gb1);
    MAYUSE(gb2);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(s0);
    MAYUSE(j64);
    MAYUSE(cacheupd);

    rex_t rex = vex.rex;

    if( (vex.m==VEX_M_0F38) && (vex.p==VEX_P_F2)) {
        switch(opcode) {

            case 0xF6:
                INST_NAME("MULX Gd, Vd, Ed (,RDX)");
                nextop = F8;
                GETGD;
                GETED32(0);
                GETVD;
                if(rex.w) {
                    // 64bits mul
                    UMULH(x3, xRDX, ed);
                    MULx(vd, xRDX, ed);
                    MOVx_REG(gd, x3);
                } else {
                    // 32bits mul
                    UMULL(x3, xRDX, ed);
                    MOVw_REG(vd, x3);
                    LSRx(gd, x3, 32);
                }
                break;

            default:
                DEFAULT;
        }
        
    }
    else {DEFAULT;}

    if((*ok==-1) && (box64_dynarec_log>=LOG_INFO || box64_dynarec_dump || box64_dynarec_missing)) {
        dynarec_log(LOG_NONE, "Dynarec unimplemented AVX opcode size %d prefix %s map %s opcode %02X ", 128<<vex.l, avx_prefix_string(vex.p), avx_map_string(vex.m), opcode);
    }
    return addr;
}
