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
#include "my_cpuid.h"
#include "emu/x87emu_private.h"
#include "emu/x64shaext.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_AVX_0F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed, vd;
    uint8_t wback, wb1, wb2, gback, vback;
    uint8_t eb1, eb2, gb1, gb2;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1, v2;
    int q0, q1, q2;
    int d0, d1, d2;
    int s0, s1;
    uint64_t tmp64u, u64;
    int64_t j64;
    int64_t fixedaddress, gdoffset, vxoffset, gyoffset;
    int unscaled;

    rex_t rex = vex.rex;

    switch (opcode) {
        case 0x28:
            INST_NAME("VMOVAPS Gx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            LD(x3, wback, fixedaddress);
            SD(x3, gback, gdoffset);
            LD(x3, wback, fixedaddress + 8);
            SD(x3, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LD(x3, wback, fixedaddress);
                SD(x3, gback, gyoffset);
                LD(x3, wback, fixedaddress + 8);
                SD(x3, gback, gyoffset + 8);
            } else {
                SD(xZR, gback, gyoffset);
                SD(xZR, gback, gyoffset + 8);
            }
            break;
        case 0x29:
            INST_NAME("VMOVAPS Ex, Gx");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 24 : 8);
            GETGX();
            LD(x3, gback, gdoffset);
            SD(x3, wback, fixedaddress);
            LD(x3, gback, gdoffset + 8);
            SD(x3, wback, fixedaddress + 8);
            if (vex.l) {
                GETEY();
                GETGY();
                LD(x3, gback, gyoffset);
                SD(x3, wback, fixedaddress);
                LD(x3, gback, gyoffset + 8);
                SD(x3, wback, fixedaddress + 8);
            } else if (MODREG) {
                GETEY();
                SD(xZR, wback, fixedaddress);
                SD(xZR, wback, fixedaddress + 8);
            }
            break;
        case 0x2E:
            // no special check...
        case 0x2F:
            if (opcode == 0x2F) {
                INST_NAME("COMISS Gx, Ex");
            } else {
                INST_NAME("UCOMISS Gx, Ex");
            }
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGXSS(d0);
            GETEXSS(v0, 0);
            CLEAR_FLAGS();
            // if isnan(d0) || isnan(v0)
            IFX (X_ZF | X_PF | X_CF) {
                FEQS(x3, d0, d0);
                FEQS(x2, v0, v0);
                AND(x2, x2, x3);
                BNE_MARK(x2, xZR);
                ORI(xFlags, xFlags, (1 << F_ZF) | (1 << F_PF) | (1 << F_CF));
                B_NEXT_nocond;
            }
            MARK;
            // else if isless(d0, v0)
            IFX (X_CF) {
                FLTS(x2, d0, v0);
                BEQ_MARK2(x2, xZR);
                ORI(xFlags, xFlags, 1 << F_CF);
                B_NEXT_nocond;
            }
            MARK2;
            // else if d0 == v0
            IFX (X_ZF) {
                FEQS(x2, d0, v0);
                CBZ_NEXT(x2);
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            break;
        case 0x5A:
            INST_NAME("VCVTPS2PD Gx, Ex");
            nextop = F8;
            GETGX();
            GETGY();
            GETEX(x2, 0, vex.l ? 12 : 4);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            FLW(s0, wback, fixedaddress);
            FLW(s1, wback, fixedaddress + 4);
            FCVTDS(s0, s0);
            FCVTDS(s1, s1);
            FSD(s0, gback, gdoffset + 0);
            FSD(s1, gback, gdoffset + 8);
            if (vex.l) {
                FLW(s0, wback, fixedaddress + 8);
                FLW(s1, wback, fixedaddress + 12);
                FCVTDS(s0, s0);
                FCVTDS(s1, s1);
                FSD(s0, gback, gyoffset + 0);
                FSD(s1, gback, gyoffset + 8);
            } else {
                FSD(xZR, gback, gyoffset + 0);
                FSD(xZR, gback, gyoffset + 8);
            }
            break;
        default:
            DEFAULT;
    }
    return addr;
}
