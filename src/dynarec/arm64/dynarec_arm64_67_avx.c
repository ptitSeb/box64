#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
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
                    if((gd==xRDX) || (gd==ed) || (gd==vd))
                        gb1 = x3;
                    else
                        gb1 = gd;
                    UMULH(gb1, xRDX, ed);
                    if(gd!=vd) {MULx(vd, xRDX, ed);}
                    if(gb1==x3) {
                        MOVx_REG(gd, gb1);
                    }
                } else {
                    // 32bits mul
                    UMULL(x3, xRDX, ed);
                    if(gd!=vd) {MOVw_REG(vd, x3);}
                    LSRx(gd, x3, 32);
                }
                break;

            default:
                DEFAULT;
        }
        
    }
    else if((vex.m==VEX_M_0F) && (vex.p==VEX_P_66)) {
        switch(opcode) {
            case 0xD6:
                INST_NAME("VMOVQ Ex, Gx");
                nextop = F8;
                GETG;
                v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                if(MODREG) {
                    ed = (nextop&7)+(rex.b<<3);
                    v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                    VMOV(v1, v0);
                    YMM0(ed);
                } else {
                    addr = geted32(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                    VST64(v0, ed, fixedaddress);
                }
                break;

            case 0x6E:
                INST_NAME("VMOVD Gx, Ed");
                nextop = F8;
                GETGX_empty(v0);
                GETED(0);
                VEORQ(v0, v0, v0); // RAZ vector
                if(rex.w) {
                    FMOVDx(v0, ed);
                } else {
                    FMOVSw(v0, ed);
                }
                YMM0(gd);
                break;

            default:
                DEFAULT;
        }
    }
    else {DEFAULT;}

    if((*ok==-1) && (BOX64ENV(dynarec_log)>=LOG_INFO || dyn->need_dump || BOX64ENV(dynarec_missing)==1))
        if(!dyn->size || BOX64ENV(dynarec_log)>LOG_INFO || dyn->need_dump) {
            dynarec_log(LOG_NONE, "  Dynarec unimplemented VEX opcode size %d prefix %s map %s opcode %02X\n", 128 << vex.l, avx_prefix_string(vex.p), avx_map_string(vex.m), opcode);
    }
    return addr;
}
