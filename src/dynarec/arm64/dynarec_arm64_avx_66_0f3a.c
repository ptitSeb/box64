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

uintptr_t dynarec64_AVX_66_0F3A(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
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

    rex_t rex = vex.rex;

    switch(opcode) {

        case 0x0C:
            INST_NAME("VPBLENDPS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGX_empty_VXEX(q0, q2, q1, 1);
            u8 = F8;
            if(q0==q1) {
                for(int i=0; i<4; ++i)
                    if(u8&(1<<i)) {
                        VMOVeS(q0, i, q1, i);
                    } else if(q0!=q2)
                        VMOVeS(q0, i, q2, i);
            } else {
                if(q0!=q2)
                    VMOVQ(q0, q2);
                if((u8&15)==0b0011) {
                    VMOVeD(q0, 0, q1, 0);
                } else if((u8&15)==0b1100) {
                    VMOVeD(q0, 1, q1, 1);
                } else for(int i=0; i<4; ++i)
                    if(u8&(1<<i)) {
                        VMOVeS(q0, i, q1, i);
                    }
            }
            if(vex.l) {
                GETGY_empty_VYEY(q0, q2, q1);
                if(q0==q1) {
                    for(int i=0; i<4; ++i)
                        if(u8&(1<<(i+4))) {
                            VMOVeS(q0, i, q1, i);
                        } else if(q0!=q2)
                            VMOVeS(q0, i, q2, i);
                } else {
                    if(q0!=q2)
                        VMOVQ(q0, q2);
                    if((u8>>4)==0b0011) {
                        VMOVeD(q0, 0, q1, 0);
                    } else if((u8>>4)==0b1100) {
                        VMOVeD(q0, 1, q1, 1);
                    } else for(int i=0; i<4; ++i)
                        if(u8&(1<<(i+4))) {
                            VMOVeS(q0, i, q1, i);
                        }
                }
            } else YMM0(gd);
            break;

        case 0x18:
            INST_NAME("VINSERTF128 Gx, Vx, Ex, imm8");
            nextop = F8;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 1);
                v1 = -1; // to avoid a warning
            }
            u8 = F8;
            GETVX(v2, 0);
            GETGX_empty(v0);
            GETGY_empty_VY(q0, q2, 0, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1);
            if(MODREG)
                VMOVQ((u8&1)?q0:v0, v1);
            else
                VLD128((u8&1)?q0:v0, ed, fixedaddress);
            if(v0!=v2)
                VMOVQ((u8&1)?v0:q0, (u8&1)?v2:q2);
            break;

        default:
            DEFAULT;
    }
    return addr;
}
