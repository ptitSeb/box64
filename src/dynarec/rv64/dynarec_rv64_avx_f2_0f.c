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

uintptr_t dynarec64_AVX_F2_0F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
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
    int s0;
    uint64_t tmp64u, u64;
    int64_t j64;
    int64_t fixedaddress, gdoffset, vxoffset, gyoffset;
    int unscaled;

    rex_t rex = vex.rex;

    switch (opcode) {
        case 0x10:
            INST_NAME("VMOVSD Gx, [Vx,] Ex");
            nextop = F8;
            GETEX(x1, 0, 1);
            GETGX();
            GETGY();
            LD(x3, wback, fixedaddress);
            SD(x3, gback, gdoffset);
            if (MODREG) {
                GETVX();
                LD(x3, vback, vxoffset + 8);
                SD(x3, gback, gdoffset + 8);
            } else {
                SD(xZR, gback, gdoffset + 8);
            }
            YMM0(gd);
            break;
        case 0x11:
            INST_NAME("VMOVSD Ex, [Vx,] Gx");
            nextop = F8;
            GETEX(x1, 0, 24);
            GETGX();
            GETGY();
            LD(x3, gback, gdoffset);
            SD(x3, wback, fixedaddress);
            if (MODREG) {
                GETVX();
                LD(x3, vback, vxoffset + 8);
                SD(x3, wback, fixedaddress + 8);
                YMM0(ed);
            }
            break;
        case 0x58:
            INST_NAME("VADDSD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, 1);
            GETGX();
            GETVX();
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            FLD(v0, vback, vxoffset + 0);
            FLD(v1, wback, fixedaddress + 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQD(x3, v0, v0);
                FEQD(x4, v1, v1);
            }
            FADDD(v0, v0, v1);
            if (!BOX64ENV(dynarec_fastnan)) {
                AND(x3, x3, x4);
                BNEZ_MARK(x3);
                BNEZ_MARK2(x4);
                FMVD(v0, v1);
                B_MARK2_nocond;
                MARK;
                FEQD(x3, v0, v0);
                BNEZ_MARK2(x3);
                FNEGD(v0, v0);
                MARK2;
            }
            FSD(v0, gback, gdoffset + 0);
            if (gd != vex.v) {
                LD(x3, vback, vxoffset + 8);
                SD(x3, gback, gdoffset + 8);
            }
            YMM0(gd);
            break;
        case 0x59:
            INST_NAME("VMULSD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, 1);
            GETGX();
            GETVX();
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            FLD(v0, vback, vxoffset + 0);
            FLD(v1, wback, fixedaddress + 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQD(x3, v0, v0);
                FEQD(x4, v1, v1);
            }
            FMULD(v0, v0, v1);
            if (!BOX64ENV(dynarec_fastnan)) {
                AND(x3, x3, x4);
                BNEZ_MARK(x3);
                BNEZ_MARK2(x4);
                FMVD(v0, v1);
                B_MARK2_nocond;
                MARK;
                FEQD(x3, v0, v0);
                BNEZ_MARK2(x3);
                FNEGD(v0, v0);
                MARK2;
            }
            FSD(v0, gback, gdoffset + 0);
            if (gd != vex.v) {
                LD(x3, vback, vxoffset + 8);
                SD(x3, gback, gdoffset + 8);
            }
            YMM0(gd);
            break;
        case 0x5C:
            INST_NAME("VSUBSD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, 1);
            GETGX();
            GETVX();
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            FLD(v0, vback, vxoffset + 0);
            FLD(v1, wback, fixedaddress + 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQD(x3, v0, v0);
                FEQD(x4, v1, v1);
            }
            FSUBD(v0, v0, v1);
            if (!BOX64ENV(dynarec_fastnan)) {
                AND(x3, x3, x4);
                BNEZ_MARK(x3);
                BNEZ_MARK2(x4);
                FMVD(v0, v1);
                B_MARK2_nocond;
                MARK;
                FEQD(x3, v0, v0);
                BNEZ_MARK2(x3);
                FNEGD(v0, v0);
                MARK2;
            }
            FSD(v0, gback, gdoffset + 0);
            if (gd != vex.v) {
                LD(x3, vback, vxoffset + 8);
                SD(x3, gback, gdoffset + 8);
            }
            YMM0(gd);
            break;
        case 0x5D:
            INST_NAME("VMINSD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, 1);
            GETGX();
            GETVX();
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            FLD(v0, vback, vxoffset + 0);
            FLD(v1, wback, fixedaddress + 0);
            FEQD(x2, v0, v0);
            FEQD(x3, v1, v1);
            AND(x2, x2, x3);
            BEQ_MARK(x2, xZR);
            FLED(x2, v1, v0);
            BEQ_MARK2(x2, xZR);
            MARK;
            FMVD(v0, v1);
            MARK2;
            FSD(v0, gback, gdoffset + 0);
            if (gd != vex.v) {
                LD(x3, vback, vxoffset + 8);
                SD(x3, gback, gdoffset + 8);
            }
            YMM0(gd);
            break;
        case 0x5E:
            INST_NAME("VDIVSD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, 1);
            GETGX();
            GETVX();
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            FLD(v0, vback, vxoffset + 0);
            FLD(v1, wback, fixedaddress + 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQD(x3, v0, v0);
                FEQD(x4, v1, v1);
            }
            FDIVD(v0, v0, v1);
            if (!BOX64ENV(dynarec_fastnan)) {
                AND(x3, x3, x4);
                BNEZ_MARK(x3);
                BNEZ_MARK2(x4);
                FMVD(v0, v1);
                B_MARK2_nocond;
                MARK;
                FEQD(x3, v0, v0);
                BNEZ_MARK2(x3);
                FNEGD(v0, v0);
                MARK2;
            }
            FSD(v0, gback, gdoffset + 0);
            if (gd != vex.v) {
                LD(x3, vback, vxoffset + 8);
                SD(x3, gback, gdoffset + 8);
            }
            YMM0(gd);
            break;
        case 0x5F:
            INST_NAME("VMAXSD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, 1);
            GETGX();
            GETVX();
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            FLD(v0, vback, vxoffset + 0);
            FLD(v1, wback, fixedaddress + 0);
            FEQD(x2, v0, v0);
            FEQD(x3, v1, v1);
            AND(x2, x2, x3);
            BEQ_MARK(x2, xZR);
            FLED(x2, v0, v1);
            BEQ_MARK2(x2, xZR);
            MARK;
            FMVD(v0, v1);
            MARK2;
            FSD(v0, gback, gdoffset + 0);
            if (gd != vex.v) {
                LD(x3, vback, vxoffset + 8);
                SD(x3, gback, gdoffset + 8);
            }
            YMM0(gd);
            break;
        default:
            DEFAULT;
    }
    return addr;
}
