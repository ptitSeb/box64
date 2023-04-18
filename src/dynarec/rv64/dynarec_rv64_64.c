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
#include "custommem.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_helper.h"
#include "dynarec_rv64_functions.h"

#define GETG        gd = ((nextop&0x38)>>3)+(rex.r<<3)

uintptr_t dynarec64_64(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int seg, int* ok, int* need_epilog)
{
    (void)ip; (void)rep; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t u8;
    uint8_t gd, ed, eb1, eb2, gb1, gb2;
    uint8_t wback, wb1, wb2, wb;
    int64_t i64, j64;
    int v0, v1;
    int q0;
    int d0;
    int64_t fixedaddress;
    int unscaled;
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(wb1);
    MAYUSE(wb2);
    MAYUSE(gb1);
    MAYUSE(gb2);
    MAYUSE(j64);
    MAYUSE(d0);
    MAYUSE(q0);
    MAYUSE(v0);
    MAYUSE(v1);

    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }
    // REX prefix before the F0 are ignored
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }

    switch(opcode) {
        case 0x2B:
            INST_NAME("SUB Gd, Seg:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop = F8;
            GETGD;
            GETEDO(x4, 0, x5);
            emit_sub32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;

        case 0x33:
            INST_NAME("XOR Gd, Seg:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop = F8;
            GETGD;
            GETEDO(x4, 0, x5);
            emit_xor32(dyn, ninst, rex, gd, ed, x3, x4);
            break;

        case 0x88:
            INST_NAME("MOV Seg:Eb, Gb");
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop=F8;
            gd = ((nextop&0x38)>>3)+(rex.r<<3);
            if(rex.rex) {
                gb2 = 0;
                gb1 = xRAX + gd;
            } else {
                gb2 = ((gd&4)>>2);
                gb1 = xRAX+(gd&3);
            }
            gd = x5;
            if(gb2) {
                SRLI(x5, gb1, 8);
                gb1 = x5;
            }
            if(MODREG) {
                ed = (nextop&7) + (rex.b<<3);
                if(rex.rex) {
                    eb1 = xRAX+ed;
                    eb2 = 0;
                } else {
                    eb1 = xRAX+(ed&3);  // Ax, Cx, Dx or Bx
                    eb2 = ((ed&4)>>2);    // L or H
                }
                ANDI(gd, gb1, 0xff);
                if(eb2) {
                    MOV64x(x1, 0xffffffffffff00ffLL);
                    ANDI(x1, eb1, x1);
                    SLLI(gd, gd, 8);
                    OR(eb1, x1, gd);
                } else {
                    ANDI(x1, eb1, ~0xff);
                    OR(eb1, x1, gd);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 0, 0);
                ADD(x4, ed, x4);
                SB(gb1, x4, 0);
                SMWRITE2();
            }
            break;
        case 0x89:
            INST_NAME("MOV Seg:Ed, Gd");
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                MVxw(xRAX+(nextop&7)+(rex.b<<3), gd);
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                ADD(x4, ed, x4);
                SDxw(gd, x4, 0);
                SMWRITE2();
            }
            break;

        case 0x8B:
            INST_NAME("MOV Gd, Seg:Ed");
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                MVxw(gd, xRAX+(nextop&7)+(rex.b<<3));
            } else {       // mem <= reg
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 0, 0);
                ADD(x4, ed, x4);
                LDxw(gd, x4, 0);
            }
            break;
        case 0xC7:
            INST_NAME("MOV Seg:Ed, Id");
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop=F8;
            if(MODREG) {   // reg <= i32
                i64 = F32S;
                ed = xRAX+(nextop&7)+(rex.b<<3);
                MOV64xw(ed, i64);
            } else {                    // mem <= i32
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 0, 4);
                i64 = F32S;
                MOV64xw(x3, i64);
                ADD(x4, ed, x4);
                SDxw(x3, x4, 0);
                SMWRITE2();
            }
            break;
        default:
            DEFAULT;
    }
    return addr;
}
