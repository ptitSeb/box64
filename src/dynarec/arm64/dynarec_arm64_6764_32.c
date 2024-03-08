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

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_helper.h"
#include "dynarec_arm64_functions.h"

#define GETGm   gd = ((nextop&0x38)>>3)

uintptr_t dynarec64_6764_32(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int seg, int* ok, int* need_epilog)
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

    if(!rex.is32bits) {
        // should do a different file
        DEFAULT;
        return addr;
    }

    rep = 0;
    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }

    switch(opcode) {

        case 0x89:
            INST_NAME("MOV Seg:Ed, Gd");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                MOVxw_REG(xRAX+(nextop&7)+(rex.b<<3), gd);
            } else {                    // mem <= reg
                grab_segdata(dyn, addr, ninst, x4, seg);
                addr = geted16(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, 0, 0, 0);
                STRw_REG(gd, ed, x4);
            }
            break;

        case 0x8B:
            INST_NAME("MOV Gd, Seg:Ed");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg => reg
                MOVxw_REG(gd, xRAX+(nextop&7)+(rex.b<<3));
            } else {                    // mem => reg
                grab_segdata(dyn, addr, ninst, x4, seg);
                addr = geted16(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, 0, 0, 0);
                LDRw_REG(gd, ed, x4);
            }
            break;

        case 0x8F:
            INST_NAME("POP Seg:Ed");
            nextop=F8;
            if(MODREG) {   // reg <= reg
                POP1_32(x1);
                MOVxw_REG(xRAX+(nextop&7)+(rex.b<<3), x1);
            } else {                    // mem <= reg
                grab_segdata(dyn, addr, ninst, x4, seg);
                POP1_32(x1);
                addr = geted16(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, 0, 0, 0);
                STRw_REG(x1, ed, x4);
            }
            break;

        case 0xA1:
            INST_NAME("MOV EAX, Seg:[Od]");
            i32 = F16;
            grab_segdata(dyn, addr, ninst, x4, seg);
            if(i32<4096 && !(i32&3)) {
                LDRw_U12(xRAX, x4, i32);
            } else if(i32<256) {
                LDURw_I9(xRAX, x4, i32);
            } else {
                MOV32w(x1, i32);
                LDRw_REG(xRAX, x4, x1);
            }
            break;

        case 0xA3:
            INST_NAME("MOV Seg:[Od], EAX");
            i32 = F16;
            grab_segdata(dyn, addr, ninst, x4, seg);
            if(i32<4096 && !(i32&3)) {
                STRw_U12(xRAX, x4, i32);
            } else if(i32<256) {
                STURw_I9(xRAX, x4, i32);
            } else {
                MOV32w(x1, i32);
                STRw_REG(xRAX, x4, x1);
            }
            break;

        case 0xFF:
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg);
            switch((nextop>>3)&7) {
                case 6: // Push Ed
                    INST_NAME("PUSH FS:Ew");
                    if((nextop&0xC0)==0xC0) {   // reg
                        DEFAULT;
                    } else {                    // mem <= i32
                        SMREAD();
                        addr = geted16(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0, 0, 0);
                        LDRw_REG(x3, ed, x4);
                        PUSH1_32(x3);
                    }
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
