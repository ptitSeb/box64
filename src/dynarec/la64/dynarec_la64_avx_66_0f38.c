#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "env.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_AVX_66_0F38(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

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
    static const int8_t round_round[] = { 0, 2, 1, 3 };
#endif

    rex_t rex = vex.rex;

    switch (opcode) {
        case 0x18:
            INST_NAME("VBROADCASTSS Gx, Ex");
            nextop = F8;
            GETGYxy_empty(q0);
            GETEYSS(q2, 0, 0);
            if(vex.l){
                XVREPLVE0_W(q0, q2);
            }else{
                VREPLVE_W(q0, q2, 0);
                YMM0(gd);
            }
            break;
        case 0x19:
            INST_NAME("VBROADCASTSD Gx, Ex");
            nextop = F8;
            GETGYxy_empty(q0);
            GETEYSD(q2, 0, 0);
            if(vex.l){
                XVREPLVE0_D(q0, q2);
            }else{
                VREPLVE_D(q0, q2, 0);
                YMM0(gd);
            }
            break;
        case 0x1A:
            INST_NAME("VBROADCASTF128 Gx, Ex");
            nextop = F8;
            GETGYy_empty(q0);
            GETEYx(q2, 0, 0);
            XVREPLVE0_Q(q0, q2);
            break;
        default:
            DEFAULT;
    }
    return addr;
}
