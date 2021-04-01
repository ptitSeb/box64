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

// Get Ex as a single, not a quad (warning, x2 get used)
#define GETEX(a, D)                                                                                 \
    if(MODREG) {                                                                                    \
        a = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3));                                     \
    } else {                                                                                        \
        a = fpu_get_scratch(dyn);                                                                   \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<2, 3, rex, 0, D);     \
        VLDR32_U12(a, ed, fixedaddress);                                                            \
    }

#define GETG        gd = ((nextop&0x38)>>3)+(rex.r<<3)

#define GETGX(a)    gd = ((nextop&0x38)>>3)+(rex.r<<3); \
                    a = sse_get_reg(dyn, ninst, x1, gd)

#define GETGX_empty(a)  gd = ((nextop&0x38)>>3)+(rex.r<<3);         \
                        a = sse_get_reg_empty(dyn, ninst, x1, gd)

uintptr_t dynarec64_F30F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback;
    uint64_t u64;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int fixedaddress;
    int j32;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(j32);

    switch(opcode) {

        case 0x10:
            INST_NAME("MOVSS Gx, Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                v0 = sse_get_reg(dyn, ninst, x1, gd);
                q0 = sse_get_reg(dyn, ninst, x1, (nextop&7) + (rex.b<<3));
                VMOVeS(v0, 0, q0, 0);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<2, 3, rex, 0, 0);
                VLDR32_U12(v0, ed, fixedaddress);
            }
            break;
        case 0x11:
            INST_NAME("MOVSS Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            if(MODREG) {
                q0 = sse_get_reg(dyn, ninst, x1, (nextop&7) + (rex.b<<3));
                VMOVeS(q0, 0, v0, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<2, 3, rex, 0, 0);
                VSTR32_U12(v0, ed, fixedaddress);
            }
            break;
        case 0x12:
            INST_NAME("MOVSLDUP Gx, Ex");
            nextop = F8;
            if(MODREG) {
                q1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<4, 15, rex, 0, 0);
                q1 = fpu_get_scratch(dyn);
                VLDR128_U12(q1, ed, fixedaddress);
            }
            GETGX_empty(q0);
            VTRNQ1_32(q0, q1, q1);
            break;

        case 0x16:
            INST_NAME("MOVSHDUP Gx, Ex");
            nextop = F8;
            if(MODREG) {
                q1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<4, 15, rex, 0, 0);
                q1 = fpu_get_scratch(dyn);
                VLDR128_U12(q1, ed, fixedaddress);
            }
            GETGX_empty(q0);
            VTRNQ2_32(q0, q1, q1);
            break;

        case 0x2A:
            INST_NAME("CVTSI2SS Gx, Ed");
            nextop = F8;
            GETGX(v0);
            GETED(0);
            d1 = fpu_get_scratch(dyn);
            if(rex.w) {
                SCVTFSx(d1, ed);
            } else {
                SCVTFSw(d1, ed);
            }
            VMOVeS(v0, 0, d1, 0);
            break;

        case 0x2C:
            INST_NAME("CVTTSS2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEX(d0, 0);
            FCVTZSxwS(gd, d0);
            break;
        case 0x2D:
            INST_NAME("CVTSS2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEX(q0, 0);
            #ifdef PRECISE_CVT
            LDRH_U12(x1, xEmu, offsetof(x64emu_t, mxcsr));
            UBFXx(x1, x1, 13, 2);   // extract round requested
            LSLx_REG(x1, x1, 3);
            ADDx_U12(x1, x1, 8);    // add the actual add+jump opcodes
            // Construct a "switch case", with each case 2 instructions, so 8 bytes
            BL(+4); // Branch with Link to next, so LR gets next PC address
            ADDx_REG(xLR, xLR, x1);
            B(xLR); // could use RET, but it's not really one
            FCVTNSxwS(gd, q0);  // 0: Nearest (even)
            B_NEXT_nocond;
            FCVTMSxwS(gd, q0);  // 1: Toward -inf
            B_NEXT_nocond;
            FCVTPSxwS(gd, q0);  // 2: Toward +inf
            B_NEXT_nocond;
            FCVTZSxwS(gd, q0);  // 3: Toward 0
            #else
            FCVTNSxwS(gd, q0);
            #endif
            break;
        case 0x51:
            INST_NAME("SQRTSS Gx, Ex");
            nextop = F8;
            GETGX(v0);
            d1 = fpu_get_scratch(dyn);
            GETEX(d0, 0);
            FSQRTS(d1, d0);
            VMOVeS(v0, 0, d1, 0);
            break;

        case 0x58:
            INST_NAME("ADDSS Gx, Ex");
            nextop = F8;
            GETGX(v0);
            d1 = fpu_get_scratch(dyn);
            GETEX(d0, 0);
            FADDS(d1, v0, d0);  // the high part of the vector is erased...
            VMOVeS(v0, 0, d1, 0);
            break;
        case 0x59:
            INST_NAME("MULSS Gx, Ex");
            nextop = F8;
            GETGX(v0);
            d1 = fpu_get_scratch(dyn);
            GETEX(d0, 0);
            FMULS(d1, v0, d0);
            VMOVeS(v0, 0, d1, 0);
            break;
        case 0x5A:
            INST_NAME("CVTSS2SD Gx, Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(v1, 0);
            d0 = fpu_get_scratch(dyn);
            FCVT_D_S(d0, v1);
            VMOVeD(v0, 0, d0, 0);
            break;

        case 0x5C:
            INST_NAME("SUBSS Gx, Ex");
            nextop = F8;
            GETGX(v0);
            d1 = fpu_get_scratch(dyn);
            GETEX(d0, 0);
            FSUBS(d1, v0, d0);
            VMOVeS(v0, 0, d1, 0);
            break;
        case 0x5D:
            INST_NAME("MINSS Gx, Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(v1, 0);
            // MINSS: if any input is NaN, or Ex[0]<Gx[0], copy Ex[0] -> Gx[0]
            d0 = fpu_get_scratch(dyn);
            FMINNMS(d0, v0, v1);    // NaN handling may be slightly different, is that a problem?
            VMOVeS(v0, 0, d0, 0);   // to not erase uper part
            break;
        case 0x5E:
            INST_NAME("DIVSS Gx, Ex");
            nextop = F8;
            GETGX(v0);
            d1 = fpu_get_scratch(dyn);
            GETEX(d0, 0);
            FDIVS(d1, v0, d0);
            VMOVeS(v0, 0, d1, 0);
            break;
        case 0x5F:
            INST_NAME("MAXSS Gx, Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(v1, 0);
            // MAXSS: if any input is NaN, or Ex[0]>Gx[0], copy Ex[0] -> Gx[0]
            d0 = fpu_get_scratch(dyn);
            FMAXNMS(d0, v0, v1);    // NaN handling may be slightly different, is that a problem?
            VMOVeS(v0, 0, d0, 0);   // to not erase uper part
            break;
            
        case 0x6F:
            INST_NAME("MOVDQU Gx,Ex");// no alignment constraint on NEON here, so same as MOVDQA
            nextop = F8;
            GETG;
            v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3));
                VMOVQ(v0, v1);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<4, 15, rex, 0, 0);
                VLDR128_U12(v0, ed, fixedaddress);
            }
            break;
        case 0x70:
            INST_NAME("PSHUFHW Gx, Ex, Ib");
            nextop = F8;
            GETEX(v1, 1);
            GETGX(v0);

            u8 = F8;
            // only high part need to be suffled. VTBL only handle 8bits value, so the 16bits suffles need to be changed in 8bits
            u64 = 0;
            for (int i=0; i<4; ++i) {
                u64 |= ((uint64_t)((u8>>(i*2))&3)*2+8)<<(i*16+0);
                u64 |= ((uint64_t)((u8>>(i*2))&3)*2+9)<<(i*16+8);
            }
            MOV64x(x2, u64);
            d0 = fpu_get_scratch(dyn);
            VMOVQDfrom(d0, 0, x2);
            VTBL1_8(d0, v1, d0);
            VMOVeD(v0, 1, d0, 0);
            break;

        case 0x7E:
            INST_NAME("MOVQ Gx, Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7) + (rex.b<<3));
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                FMOVD(v0, v1);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<3, 7, rex, 0, 0);
                VLDR64_U12(v0, ed, fixedaddress);
            }
            break;
        case 0x7F:
            INST_NAME("MOVDQU Ex,Gx");
            nextop = F8;
            GETG;
            if(MODREG) {
                v0 = sse_get_reg(dyn, ninst, x1, gd);
                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop&7) + (rex.b<<3));
                VMOVQ(v1, v0);
            } else {
                v0 = sse_get_reg(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<4, 15, rex, 0, 0);
                VSTR128_U12(v0, ed, fixedaddress);
            }
            break;

        case 0xBC:
            INST_NAME("TZCNT Gd, Ed");
            SETFLAGS(X_CF|X_ZF, SF_SET);
            nextop = F8;
            GETED(0);
            GETGD;
            SET_DFNONE(x1);
            TSTxw_REG(ed, ed);
            BFIw(xFlags, x1, F_CF, 1);  // CF = is source 0?
            RBITxw(x1, ed);   // reverse
            CLZxw(gd, x1);    // x2 gets leading 0 == TZCNT
            TSTxw_REG(gd, gd);
            CSETw(x1, cEQ);
            BFIw(xFlags, x1, F_ZF, 1);  // ZF = is dest 0?
            break;

        case 0xC2:
            INST_NAME("CMPSS Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0);
            GETEX(v1, 1);
            u8 = F8;
            if((u8&7)==6){
                FCMPS(v1, v0);
            } else {
                FCMPS(v0, v1);
            }
            switch(u8&7) {
                case 0: CSETMw(x2, cEQ); break;   // Equal
                case 1: CSETMw(x2, cMI); break;   // Less than
                case 2: //CSETMw(x2, cLE); break;   // Less or equal (or unordered on ARM64, not on x86...)
                        CSETMw(x2, cPL); CSINVw(x2, xZR, x2, cEQ); break;   // so use a 2 step here, but 1st test inverted because 2nd step invert again
                case 3: CSETMw(x2, cVS); break;   // NaN
                case 4: CSETMw(x2, cNE); break;   // Not Equal (or unordered on ARM, not on X86...)
                case 5: CSETMw(x2, cCS); break;   // Greater or equal or unordered
                case 6: CSETMw(x2, cLT); break;   // Greater or unordered, test inverted, N!=V so unordereded or less than (inverted)
                case 7: CSETMw(x2, cVC); break;   // not NaN
            }
            VMOVQSfrom(v0, 0, x2);
            break;

        default:
            DEFAULT;
    }
    return addr;
}

