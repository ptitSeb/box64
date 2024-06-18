#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <assert.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "la64_emitter.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_helper.h"
#include "dynarec_la64_functions.h"

uintptr_t dynarec64_67(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed, wback, wb, wb1, wb2, gb1, gb2, eb1, eb2;
    int64_t fixedaddress;
    int unscaled;
    int8_t  i8;
    uint8_t u8;
    int32_t i32;
    int64_t j64, i64;
    int cacheupd = 0;
    int lock;
    int v0, v1, s0;
    MAYUSE(i32);
    MAYUSE(j64);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(s0);
    MAYUSE(lock);
    MAYUSE(cacheupd);

    if(rex.is32bits) {
        // should do a different file
        DEFAULT;
        return addr;
    }

    GETREX();

    rep = 0;
    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }

    switch(opcode) {
        case 0x88:
            INST_NAME("MOV Eb, Gb");
            nextop = F8;
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            if (rex.rex) {
                gb2 = 0;
                gb1 = TO_LA64(gd);
            } else {
                gb2 = ((gd & 4) << 1);
                gb1 = TO_LA64(gd & 3);
            }
            if (gb2) {
                gd = x4;
                BSTRPICK_D(gd, gb1, gb2 + 7, gb2);
            } else {
                gd = gb1; // no need to extract
            }
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                if (rex.rex) {
                    eb1 = TO_LA64(ed);
                    eb2 = 0;
                } else {
                    eb1 = TO_LA64(ed & 3); // Ax, Cx, Dx or Bx
                    eb2 = ((ed & 4) >> 2); // L or H
                }
                BSTRINS_D(eb1, gd, eb2 * 8 + 7, eb2 * 8);
            } else {
                addr = geted32(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                ST_B(gb1, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0x89:
            INST_NAME("MOV Ed, Gd");
            nextop = F8;
            GETGD;
            if (MODREG) { // reg <= reg
                MVxw(TO_LA64((nextop & 7) + (rex.b << 3)), gd);
            } else { // mem <= reg
                addr = geted32(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                SDxw(gd, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0xF7:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 4:
                    INST_NAME("MUL EAX, Ed");
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETED32(0);
                    if (rex.w) {
                        if (ed == xRDX)
                            gd = x3;
                        else
                            gd = xRDX;
                        MULH_DU(gd, xRAX, ed);
                        MUL_D(xRAX, xRAX, ed);
                        if (gd != xRDX) MV(xRDX, gd);
                    } else {
                        MUL_D(xRDX, xRAX, ed); // 64 <- 32x32
                        AND(xRAX, xRDX, xMASK);
                        SRLI_W(xRDX, xRDX, 32);
                    }
                    UFLAG_RES(xRAX);
                    UFLAG_OP1(xRDX);
                    UFLAG_DF(x2, rex.w ? d_mul64 : d_mul32);
                    break;
                default:
                    DEFAULT;
            }
            break;
        default:
            DEFAULT;
    }
    return addr;
}
