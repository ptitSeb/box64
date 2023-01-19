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

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_helper.h"
#include "dynarec_arm64_functions.h"

#define GETG        gd = ((nextop&0x38)>>3)+(rex.r<<3)

uintptr_t dynarec64_6664(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip; (void)rep; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed;
    int64_t j64;
    int v0, v1;
    int64_t fixedaddress;
    MAYUSE(j64);

    // REX prefix before the 66 are ignored
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }

    /*if(rex.w && opcode!=0x0f) {   // rex.w cancels "66", but not for 66 0f type of prefix
        MESSAGE(LOG_DUMP, "Here!\n");
        return dynarec64_64(dyn, addr-2, ip, ninst, rex, rep, ok, need_epilog);
    }*/

    switch(opcode) {

        case 0x0F:
            opcode = F8;
            switch(opcode) {
                case 0x2E:
                    // no special check...
                case 0x2F:
                    if(opcode==0x2F) {INST_NAME("COMISD Gx, Ex");} else {INST_NAME("UCOMISD Gx, Ex");}
                    SETFLAGS(X_ALL, SF_SET);
                    nextop = F8;
                    GETG;
                    v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                    if(MODREG) {
                        v1 = sse_get_reg(dyn, ninst, x1, (nextop&7) + (rex.b<<3), 0);
                    } else {
                        grab_segdata(dyn, addr, ninst, x4, _FS);
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, rex, NULL, 0, 0);
                        v1 = fpu_get_scratch(dyn);                                                                       \
                        VLDR64_REG(v1, ed, x4);
                    }
                    FCMPD(v0, v1);
                    FCOMI(x1, x2);
                    break;
            case 0xD6:
                INST_NAME("MOVQ Ex, Gx");
                nextop = F8;
                GETG;
                v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                if(MODREG) {
                    v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop&7) + (rex.b<<3));
                    FMOVD(v1, v0);
                } else {
                    grab_segdata(dyn, addr, ninst, x4, _FS);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, rex, NULL, 0, 0);
                    VSTR64_REG(v0, ed, x4);
                    SMWRITE();
                }
                break;

                default:
                    DEFAULT;
            }
            break;

        case 0x89:
            INST_NAME("MOV FS:Ew, Gw");
            nextop = F8;
            GETGD;  // don't need GETGW here
            if(MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
                if(rex.w) {
                    MOVx_REG(ed, gd);
                } else {
                    if(ed!=gd) {
                        BFIx(ed, gd, 0, 16);
                    }
                }
            } else {
                grab_segdata(dyn, addr, ninst, x4, _FS);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0, rex, NULL, 0, 0);
                if(rex.w) {
                    STRx_REG(gd, ed, x4);
                } else {
                    STRH_REG(gd, ed, x4);
                }
                SMWRITE();
            }
            break;

        case 0x8B:
            INST_NAME("MOV Gd, FS:Ed");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                ed = xRAX+(nextop&7)+(rex.b<<3);
                if(rex.w) {
                    MOVx_REG(gd, ed);
                } else {
                    if(ed!=gd) {
                        BFIx(gd, ed, 0, 16);
                    }
                }
            } else {                    // mem <= reg
                grab_segdata(dyn, addr, ninst, x4, _FS);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0, rex, NULL, 0, 0);
                if(rex.w) {
                    LDRx_REG(gd, ed, x4);
                } else {
                    LDRH_REG(x1, ed, x4);
                    BFIx(gd, x1, 0, 16);
                }
            }
            break;


        default:
            DEFAULT;
    }
    return addr;
}
