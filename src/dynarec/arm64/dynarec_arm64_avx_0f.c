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
#include "my_cpuid.h"
#include "emu/x87emu_private.h"
#include "emu/x64shaext.h"

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"

uintptr_t dynarec64_AVX_0F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
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
    #if STEP > 1
    static const int8_t mask_shift8[] = { -7, -6, -5, -4, -3, -2, -1, 0 };
    #endif

    rex_t rex = vex.rex;

    switch(opcode) {

        case 0xC6:
            INST_NAME("VSHUFPS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETVX(v2, 0);
            if(!MODREG) {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                v1 = -1; // to avoid a warning
            } else
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
            GETGX_empty(v0);
            u8 = F8;
            if(v2==v1 && (u8&0x3)==((u8>>2)&3) && (u8&0xf)==((u8>>4)&0xf)) {
                VDUPQ_32(v0, v2, u8&3);
            } else if(v2==v1 && (u8==0xe0)) {   // easy special case
                VMOVQ(v0, v2);
                VMOVeS(v0, 1, v0, 0);
            } else if(v2==v1 && (u8==0xe5)) {   // easy special case
                VMOVQ(v0, v2);
                VMOVeS(v0, 0, v0, 1);
            } else {
                d0 = fpu_get_scratch(dyn, ninst);
                // first two elements from Vx
                for(int i=0; i<2; ++i) {
                    VMOVeS(d0, i, v2, (u8>>(i*2))&3);
                }
                // second two from Ex
                if(MODREG) {
                    for(int i=2; i<4; ++i) {
                        VMOVeS(d0, i, v1, (u8>>(i*2))&3);
                    }
                } else {
                    SMREAD();
                    for(int i=2; i<4; ++i) {
                        ADDx_U12(x2, ed, ((u8>>(i*2))&3)*4);
                        VLD1_32(d0, i, x2);
                    }
                }
                VMOVQ(v0, d0);
            }
            if(vex.l) {
                if(MODREG)
                    v1 = ymm_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0, gd, vex.v, -1);
                GETGY_empty_VY(v0, v2, 0, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1);
                if(v2==v1 && (u8&0x3)==((u8>>2)&3) && (u8&0xf)==((u8>>4)&0xf)) {
                    VDUPQ_32(v0, v2, u8&3);
                } else if(v2==v1 && (u8==0xe0)) {
                    VMOVQ(v0, v2);
                    VMOVeS(v0, 1, v0, 0);
                } else if(v2==v1 && (u8==0xe5)) {
                    VMOVQ(v0, v2);
                    VMOVeS(v0, 0, v0, 1);
                } else {
                    for(int i=0; i<2; ++i) {
                        VMOVeS(d0, i, v2, (u8>>(i*2))&3);
                    }
                    if(MODREG) {
                        for(int i=2; i<4; ++i) {
                            VMOVeS(d0, i, v1, (u8>>(i*2))&3);
                        }
                    } else {
                        SMREAD();
                        for(int i=2; i<4; ++i) {
                            ADDx_U12(x2, ed, 16+((u8>>(i*2))&3)*4);
                            VLD1_32(d0, i, x2);
                        }
                    }
                    VMOVQ(v0, d0);
                }
            } else YMM0(gd);
            break;

        default:
            DEFAULT;
    }
    return addr;
}
