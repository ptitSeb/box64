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
#include "my_cpuid.h"
#include "emu/x87emu_private.h"

#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"

#define GETGX(a)    \
    gd = ((nextop&0x38)>>3)+(rex.r<<3);  \
    a = sse_get_reg(dyn, ninst, x1, gd)
#define GETEX(a)    \
    if(MODREG) { \
        a = sse_get_reg(dyn, ninst, x1, nextop&7+(rex.b<<3)); \
    } else {    \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0); \
        a = fpu_get_scratch_quad(dyn); \
        VLD1Q_64(a, ed);    \
    }
#define GETGM(a)    \
    gd = (nextop&0x38)>>3;  \
    a = mmx_get_reg(dyn, ninst, x1, gd)
#define GETEM(a)    \
    if(MODREG) { \
        a = mmx_get_reg(dyn, ninst, x1, nextop&7); \
    } else {    \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0); \
        a = fpu_get_scratch_double(dyn); \
        VLD1_64(a, ed);    \
    }
#define PUTEM(a)    \
    if(!MODREG) { \
        VST1_64(a, ed);    \
    }

uintptr_t dynarec64_0F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    uint8_t opcode = F8;
    uint8_t nextop, u8;
    int32_t i32, i32_, j32;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2;
    uint8_t eb1, eb2;
    uint8_t gb1, gb2;
    int v0, v1, v2;
    int q0, q1;
    int d0, d1;
    int s0;
    int fixedaddress;
    int parity;
    MAYUSE(s0);
    MAYUSE(q1);
    MAYUSE(v2);
    MAYUSE(gb2);
    MAYUSE(gb1);
    MAYUSE(eb2);
    MAYUSE(eb1);
    MAYUSE(wb2);
    MAYUSE(j32);
    #if STEP == 3
    //static const int8_t mask_shift8[] = { -7, -6, -5, -4, -3, -2, -1, 0 };
    #endif

    switch(opcode) {



        case 0x1F:
            INST_NAME("NOP (multibyte)");
            nextop = F8;
            FAKEED;
            break;

        #define GO(GETFLAGS, NO, YES, F)            \
            READFLAGS(F);                           \
            GETFLAGS;                               \
            nextop=F8;                              \
            GETGD;                                  \
            if(MODREG) {                            \
                ed = xRAX+(nextop&7)+(rex.b<<3);    \
                CSELxw(gd, ed, gd, YES);            \
            } else { \
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, 0, 0); \
                Bcond(NO, +8);                      \
                LDRxw_U12(gd, ed, fixedaddress);    \
                if(!rex.w) {MOVw_REG(gd, gd);}      \
            }

        GOCOND(0x40, "CMOV", "Gd, Ed");
        #undef GO
        
        #define GO(GETFLAGS, NO, YES, F)   \
            READFLAGS(F);   \
            i32_ = F32S;    \
            BARRIER(2);     \
            JUMP(addr+i32_);\
            GETFLAGS;   \
            if(dyn->insts) {    \
                if(dyn->insts[ninst].x64.jmp_insts==-1) {   \
                    /* out of the block */                  \
                    i32 = dyn->insts[ninst+1].address-(dyn->arm_size); \
                    Bcond(NO, i32);     \
                    jump_to_next(dyn, addr+i32_, 0, ninst); \
                } else {    \
                    /* inside the block */  \
                    i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->arm_size);    \
                    Bcond(YES, i32);    \
                }   \
            }

        GOCOND(0x80, "J", "Id");
        #undef GO

        #define GO(GETFLAGS, NO, YES, F)                \
            READFLAGS(F);                               \
            GETFLAGS;                                   \
            nextop=F8;                                  \
            CSETw(x3, YES);                             \
            if(MODREG) {                                \
                if(rex.rex) {                           \
                    eb1= xRAX+(nextop&7)+(rex.b<<3);    \
                    eb2 = 0;                            \
                } else {                                \
                    ed = (nextop&7);                    \
                    eb2 = (ed>>2)*8;                    \
                    eb1 = xRAX+(ed&3);                  \
                }                                       \
                BFIx(eb1, x3, eb2, 8);                  \
            } else {                                    \
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0xfff, 0, rex, 0, 0); \
                STRB_U12(x3, ed, fixedaddress);         \
            }

        GOCOND(0x90, "SET", "Eb");
        #undef GO
            
        case 0xA3:
            INST_NAME("BT Ed, Gd");
            SETFLAGS(X_CF, SF_SET);
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
                wback = 0;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, 0, 0);
                UBFXw(x1, gd, 5+rex.w, 3-rex.w); // r1 = (gd>>5);
                ADDx_REG_LSL(x3, wback, x1, 2); //(&ed)+=r1*4;
                LDRxw_U12(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            if(rex.w) {
                ANDx_mask(x2, gd, 1, 0, 0b00101);  //mask=0x000000000000003f
            } else {
                ANDw_mask(x2, gd, 0, 0b00100);  //mask=0x00000001f
            }
            LSRxw_REG(x4, ed, x2);
            BFIw(xFlags, x4, F_CF, 1);
            break;

        case 0xA5:
            nextop = F8;
            INST_NAME("SHLD Ed, Gd, CL");
            UXTBw(x3, xRCX);
            SETFLAGS(X_ALL, SF_SET);
            GETEDW(x4, x1, 0);
            GETGD;
            MOVxw_REG(x2, gd);
            CALL_(rex.w?((void*)shld64):((void*)shld32), ed, x4);
            WBACK;
            break;

        case 0xAB:
            INST_NAME("BTS Ed, Gd");
            SETFLAGS(X_CF, SF_SET);
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
                wback = 0;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, 0, 0);
                UBFXw(x1, gd, 5+rex.w, 3-rex.w); // r1 = (gd>>5);
                ADDx_REG_LSL(x3, wback, x1, 2); //(&ed)+=r1*4;
                LDRxw_U12(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            if(rex.w) {
                ANDx_mask(x2, gd, 1, 0, 0b00101);  //mask=0x000000000000003f
            } else {
                ANDw_mask(x2, gd, 0, 0b00100);  //mask=0x00000001f
            }
            LSRxw_REG(x4, ed, x2);
            if(rex.w) {
                ANDSx_mask(x4, x4, 1, 0, 0);  //mask=1
            } else {
                ANDSw_mask(x4, x4, 0, 0);  //mask=1
            }
            BFIw(xFlags, x4, F_CF, 1);
            MOV32w(x4, 1);
            LSLxw_REG(x4, x4, x2);
            EORxw_REG(x4, ed, x4);
            CSELxw(ed, ed, x4, cNE);
            if(wback) {
                STRxw_U12(ed, wback, fixedaddress);
            }
            break;

        case 0xAD:
            nextop = F8;
            INST_NAME("SHRD Ed, Gd, CL");
            SETFLAGS(X_ALL, SF_SET);
            UXTBw(x3, xRCX);
            GETEDW(x4, x1, 0);
            GETGD;
            MOVxw_REG(x2, gd);
            CALL_(rex.w?((void*)shrd64):((void*)shrd32), ed, x4);
            WBACK;
            break;

        case 0xB3:
            INST_NAME("BTR Ed, Gd");
            SETFLAGS(X_CF, SF_SET);
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
                wback = 0;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, 0, 0);
                UBFXw(x1, gd, 5+rex.w, 3-rex.w); // r1 = (gd>>5);
                ADDx_REG_LSL(x3, wback, x1, 2); //(&ed)+=r1*4;
                LDRxw_U12(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            if(rex.w) {
                ANDx_mask(x2, gd, 1, 0, 0b00101);  //mask=0x000000000000003f
            } else {
                ANDw_mask(x2, gd, 0, 0b00100);  //mask=0x00000001f
            }
            LSRxw_REG(x4, ed, x2);
            if(rex.w) {
                ANDSx_mask(x4, x4, 1, 0, 0);  //mask=1
            } else {
                ANDSw_mask(x4, x4, 0, 0);  //mask=1
            }
            BFIw(xFlags, x4, F_CF, 1);
            MOV32w(x4, 1);
            LSLxw_REG(x4, x4, x2);
            EORxw_REG(x4, ed, x4);
            CSELxw(ed, ed, x4, cEQ);
            if(wback) {
                STRxw_U12(ed, wback, fixedaddress);
            }
            break;

        case 0xBB:
            INST_NAME("BTC Ed, Gd");
            SETFLAGS(X_CF, SF_SET);
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
                wback = 0;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, 0, 0);
                UBFXw(x1, gd, 5+rex.w, 3-rex.w); // r1 = (gd>>5);
                ADDx_REG_LSL(x3, wback, x1, 2); //(&ed)+=r1*4;
                LDRxw_U12(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            if(rex.w) {
                ANDx_mask(x2, gd, 1, 0, 0b00101);  //mask=0x000000000000003f
            } else {
                ANDw_mask(x2, gd, 0, 0b00100);  //mask=0x00000001f
            }
            LSRxw_REG(x4, ed, x2);
            if(rex.w) {
                ANDx_mask(x4, x4, 1, 0, 0);  //mask=1
            } else {
                ANDw_mask(x4, x4, 0, 0);  //mask=1
            }
            BFIw(xFlags, x4, F_CF, 1);
            MOV32w(x4, 1);
            LSLxw_REG(x4, x4, x2);
            EORxw_REG(ed, ed, x4);
            if(wback) {
                STRxw_U12(ed, wback, fixedaddress);
            }
            break;

        default:
            DEFAULT;
    }
    return addr;
}

