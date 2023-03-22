#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
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

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "dynarec_rv64_helper.h"

uintptr_t dynarec64_F30F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback;
    uint64_t u64;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int64_t fixedaddress;
    int unscaled;
    int64_t j64;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(j64);

    switch(opcode) {
        case 0x10:
            INST_NAME("MOVSS Gx, Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                q0 = sse_get_reg(dyn, ninst, x1, (nextop&7) + (rex.b<<3), 1);
                FMVS(v0, q0);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd, 1);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 8, 0);
                FLW(v0, ed, fixedaddress);
                // reset upper part
                SW(xZR, xEmu, offsetof(x64emu_t, xmm[gd])+4);
                SD(xZR, xEmu, offsetof(x64emu_t, xmm[gd])+8);
            }
            break;
        case 0x11:
            INST_NAME("MOVSS Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            if(MODREG) {
                q0 = sse_get_reg(dyn, ninst, x1, (nextop&7) + (rex.b<<3), 1);
                FMVS(q0, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                FSW(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
            
        case 0x1E:
            INST_NAME("NOP / ENDBR32 / ENDBR64");
            nextop = F8;
            FAKEED;
            break;

        case 0x2A:
            INST_NAME("CVTSI2SS Gx, Ed");
            nextop = F8;
            GETGXSS(v0);
            GETED(0);
            if(rex.w) {
                FCVTSL(v0, ed);
            } else {
                FCVTSW(v0, ed);
            }
            break;

        case 0x58:
            INST_NAME("ADDSS Gx, Ex");
            nextop = F8;
            GETGXSS(v0);
            GETEXSS(d0, 0);
            FADDS(v0, v0, d0);
            break;
        case 0x59:
            INST_NAME("MULSS Gx, Ex");
            nextop = F8;
            GETGXSS(v0);
            GETEXSS(d0, 0);
            FMULS(v0, v0, d0);
            break;
        case 0x5A:
            INST_NAME("CVTSS2SD Gx, Ex");
            nextop = F8;
            GETEXSS(v1, 0);
            GETGXSD_empty(v0);
            FCVTDS(v0, v1);
            break;

        default:
            DEFAULT;
    }
    return addr;
}
