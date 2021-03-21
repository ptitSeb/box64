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
#include "dynarec_arm64.h"
#include "dynarec_arm64_private.h"
#include "arm64_printer.h"

#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"

// Get EX as a quad
#define GETEX(a, D)                                                                                     \
    if(MODREG) {                                                                                        \
        a = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3));                                         \
    } else {                                                                                            \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<4, (1<<4)-1, rex, 0, D);  \
        a = fpu_get_scratch_quad(dyn);                                                                  \
        VLDR128_U12(a, ed);                                                                             \
    }
#define GETGX(a)                        \
    gd = ((nextop&0x38)>>3)+(rex.r<<3); \
    a = sse_get_reg(dyn, ninst, x1, gd)

uintptr_t dynarec64_660F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    uint8_t opcode = F8;
    uint8_t nextop, u8;
    int32_t i32, j32;
    uint8_t gd, ed;
    uint8_t wback, wb1;
    uint8_t eb1, eb2;
    int v0, v1;
    int q0, q1;
    int d0;
    int s0;
    int fixedaddress;
    int parity;

    MAYUSE(d0);
    MAYUSE(q1);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(j32);
    #if STEP == 3
    static const int8_t mask_shift8[] = { -7, -6, -5, -4, -3, -2, -1, 0 };
    #endif

    switch(opcode) {


        case 0x1F:
            INST_NAME("NOP (multibyte)");
            nextop = F8;
            FAKEED;
            break;
        
        case 0x28:
            INST_NAME("MOVAPD Gx,Ex");
            nextop = F8;
            gd = ((nextop&0x38)>>3) + (rex.r<<3);
            if(MODREG) {
                ed = (nextop&7)+(rex.b<<3);
                v1 = sse_get_reg(dyn, ninst, x1, ed);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VMOVQ(v0, v1);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<4, 15, rex, 0, 0);
                VLDR128_U12(v0, ed, fixedaddress);
            }
            break;
        case 0x29:
            INST_NAME("MOVAPD Ex,Gx");
            nextop = F8;
            gd = ((nextop&0x38)>>3) + (rex.r<<3);
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            if(MODREG) {
                ed = (nextop&7)+(rex.b<<3);
                v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                VMOVQ(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<4, 15, rex, 0, 0);
                VSTR128_U12(v0, ed, fixedaddress);
            }
            break;

        #define GO(GETFLAGS, NO, YES, F)            \
            READFLAGS(F);                           \
            GETFLAGS;                               \
            nextop=F8;                              \
            GETGD;                                  \
            if(MODREG) {                            \
                ed = xRAX+(nextop&7)+(rex.b<<3);    \
            } else {                                \
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, 0, 0); \
                LDRH_U12(x1, ed, fixedaddress);     \
                ed = x1;                            \
            }                                       \
            Bcond(NO, +8);                          \
            BFIx(gd, ed, 0, 16);

        GOCOND(0x40, "CMOV", "Gw, Ew");
        #undef GO

        case 0x6E:
            INST_NAME("MOVD Gx, Ed");
            nextop = F8;
            gd = ((nextop&0x38)>>3)+(rex.r<<3);
            GETED(0);
            v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            VEORQ(v0, v0, v0); // RAZ vector
            if(rex.w) {
                VMOVQDfrom(v0, 0, ed);
            } else {
                VMOVQSfrom(v0, 0, ed);
            }
            break;

        case 0x7E:
            INST_NAME("MOVD Ed,Gx");
            nextop = F8;
            gd = ((nextop&0x38)>>3)+(rex.r<<3);
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            if(rex.w) {
                if(MODREG) {
                    ed = xRAX + (nextop&7) + (rex.b<<3);
                    VMOVQDto(ed, v0, 0);
                } else {
                    VMOVQDto(x2, v0, 0); // to avoid Bus Error, using regular store
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<3, 7, rex, 0, 0);
                    STRx_U12(x2, ed, fixedaddress);
                }
            } else {
                if(MODREG) {
                    ed = xRAX + (nextop&7) + (rex.b<<3);
                    VMOVSto(ed, v0, 0);
                } else {
                    VMOVSto(x2, v0, 0); // to avoid Bus Error, using regular store
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<2, 3, rex, 0, 0);
                    STRw_U12(x2, ed, fixedaddress);
                }
            }
            break;

        case 0xA3:
            INST_NAME("BT Ew, Gw");
            SETFLAGS(X_CF, SF_SET);
            nextop = F8;
            gd = xRAX+((nextop&0x38)>>3)+(rex.r<<3);    // GETGD
            GETEW(x4, 0);
            ANDw_mask(x2, gd, 0, 0b000011);  // mask=0x0f
            LSRw_REG(x1, ed, x2);
            BFIw(xFlags, x1, F_CF, 1);
            break;
        case 0xA4:
        case 0xA5:
            nextop = F8;
            if(opcode==0xA4) {
                INST_NAME("SHLD Ew, Gw, Ib");
            } else {
                INST_NAME("SHLD Ew, Gw, CL");
                UXTBw(x3, xRCX);
            }
            SETFLAGS(X_ALL, SF_SET);
            GETEWW(x4, x1, (opcode==0xA4)?1:0);
            GETGW(x2);
            if(opcode==0xA4) {
                u8 = F8;
                MOV32w(x3, u8);
            }
            CALL_(shld16, x1, wback);
            EWBACKW(x1);
            break;

        case 0xAB:
            INST_NAME("BTS Ew, Gw");
            SETFLAGS(X_CF, SF_SET);
            nextop = F8;
            gd = xRAX+((nextop&0x38)>>3)+(rex.r<<3);    // GETGD
            GETEW(x4, 0);
            ANDw_mask(x2, gd, 0, 0b000011);  // mask=0x0f
            LSRw_REG(x1, ed, x2);
            BFIw(xFlags, x1, F_CF, 1);
            ANDSw_mask(x1, x1, 0, 0);  //mask=1
            B_NEXT(cNE);
            MOV32w(x1, 1);
            LSLxw_REG(x1, x1, x2);
            EORxw_REG(ed, ed, x1);
            EWBACK;
            break;
        case 0xAC:
        case 0xAD:
            nextop = F8;
            if(opcode==0xAC) {
                INST_NAME("SHRD Ew, Gw, Ib");
            } else {
                INST_NAME("SHRD Ew, Gw, CL");
                UXTBw(x3, xRCX);
            }
            SETFLAGS(X_ALL, SF_SET);
            GETEWW(x4, x1, (opcode==0xAC)?1:0);
            GETGW(x2);
            if(opcode==0xAC) {
                u8 = F8;
                MOV32w(x3, u8);
            }
            CALL_(shrd16, x1, wback);
            EWBACKW(x1);
            break;

        case 0xAF:
            INST_NAME("IMUL Gw,Ew");
            SETFLAGS(X_ALL, SF_PENDING);
            nextop = F8;
            UFLAG_DF(x1, d_imul16);
            GETSEW(x1, 0);
            GETSGW(x2);
            MULw(x2, x2, x1);
            UFLAG_RES(x2);
            GWBACK;
            break;

        case 0xB3:
            INST_NAME("BTR Ew, Gw");
            SETFLAGS(X_CF, SF_SET);
            nextop = F8;
            gd = xRAX+((nextop&0x38)>>3)+(rex.r<<3);    // GETGD
            GETEW(x4, 0);
            ANDw_mask(x2, gd, 0, 0b000011);  // mask=0x0f
            LSRw_REG(x1, ed, x2);
            BFIw(xFlags, x1, F_CF, 1);
            ANDSw_mask(x1, x1, 0, 0);  //mask=1
            B_NEXT(cEQ);
            MOV32w(x1, 1);
            LSLxw_REG(x1, x1, x2);
            EORxw_REG(ed, ed, x1);
            EWBACK;
            break;

        case 0xBB:
            INST_NAME("BTC Ew, Gw");
            SETFLAGS(X_CF, SF_SET);
            nextop = F8;
            gd = xRAX+((nextop&0x38)>>3)+(rex.r<<3);    // GETGD
            GETEW(x4, 0);
            ANDw_mask(x2, gd, 0, 0b000011);  // mask=0x0f
            LSRw_REG(x1, ed, x2);
            BFIw(xFlags, x1, F_CF, 1);
            ANDw_mask(x1, x1, 0, 0);  //mask=1
            MOV32w(x1, 1);
            LSLxw_REG(x1, x1, x2);
            EORxw_REG(ed, ed, x1);
            EWBACK;
            break;
        case 0xBC:
            INST_NAME("BSF Ew,Gw");
            SETFLAGS(X_ZF, SF_SET);
            nextop = F8;
            GETGD;
            GETEW(x1, 0);  // Get EW
            TSTw_REG(x1, x1);
            B_MARK(cEQ);
            RBITw(x1, x1);   // reverse
            CLZw(x2, x1);    // x2 gets leading 0 == BSF
            BFIw(gd, x2, 0, 16);
            MARK;
            CSETw(x1, cEQ);    //ZF not set
            BFIw(xFlags, x1, F_ZF, 1);
            SET_DFNONE(x1);
            break;
        case 0xBD:
            INST_NAME("BSR Ew,Gw");
            SETFLAGS(X_ZF, SF_SET);
            nextop = F8;
            GETGD;
            GETEW(x1, 0);  // Get EW
            TSTw_REG(x1, x1);
            B_MARK(cEQ);
            LSLw(x1, x1, 16);   // put bits on top
            CLZw(x2, x1);       // x2 gets leading 0
            SUBw_U12(x2, x2, 15);
            NEGw_REG(x2, x2);   // complement
            BFIx(gd, x2, 0, 16);
            MARK;
            CSETw(x1, cEQ);    //ZF not set
            BFIw(xFlags, x1, F_ZF, 1);
            SET_DFNONE(x1);
            break;
        default:
            DEFAULT;
    }
    return addr;
}

