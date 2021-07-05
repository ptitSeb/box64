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

// Get Ex as a double, not a quad (warning, x2 get used)
#define GETEX(a, D)                                                                                 \
    if(MODREG) {                                                                                    \
        a = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3));                                     \
    } else {                                                                                        \
        a = fpu_get_scratch(dyn);                                                                   \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<3, 7, rex, 0, D);     \
        VLDR64_U12(a, ed, fixedaddress);                                                            \
    }

#define GETG        gd = ((nextop&0x38)>>3)+(rex.r<<3)

#define GETGX(a)    gd = ((nextop&0x38)>>3)+(rex.r<<3); \
                    a = sse_get_reg(dyn, ninst, x1, gd)

#define GETGX_empty(a)  gd = ((nextop&0x38)>>3)+(rex.r<<3); \
                        a = sse_get_reg_empty(dyn, ninst, x1, gd)

uintptr_t dynarec64_F20F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed;
    uint8_t wback;
    uint8_t u8;
    uint64_t u64;
    int v0, v1;
    int q0;
    int d0, d1;
    int64_t fixedaddress;

#ifdef PRECISE_CVT
    int j32;
    MAYUSE(j32);
#endif
    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(v0);
    MAYUSE(v1);

    switch(opcode) {

        case 0x10:
            INST_NAME("MOVSD Gx, Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                ed = (nextop&7)+ (rex.b<<3);
                v0 = sse_get_reg(dyn, ninst, x1, gd);
                d0 = sse_get_reg(dyn, ninst, x1, ed);
                VMOVeD(v0, 0, d0, 0);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<3, 7, rex, 0, 0);
                VLDR64_U12(v0, ed, fixedaddress); // upper part reseted
            }
            break;
        case 0x11:
            INST_NAME("MOVSD Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            if(MODREG) {
                ed = (nextop&7)+ (rex.b<<3);
                d0 = sse_get_reg(dyn, ninst, x1, ed);
                VMOVeD(d0, 0, v0, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<3, 7, rex, 0, 0);
                VSTR64_U12(v0, ed, fixedaddress);
            }
            break;
        case 0x12:
            INST_NAME("MOVDDUP Gx, Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                d0 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3));
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VMOVeD(v0, 0, d0, 0);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<3, 7, rex, 0, 0);
                VLDR64_U12(v0, ed, fixedaddress);
            }
            VMOVeD(v0, 1, v0, 0);
            break;

        case 0x2A:
            INST_NAME("CVTSI2SD Gx, Ed");
            nextop = F8;
            GETGX(v0);
            GETED(0);
            d1 = fpu_get_scratch(dyn);
            if(rex.w) {
                SCVTFDx(d1, ed);
            } else {
                SCVTFDw(d1, ed);
            }
            VMOVeD(v0, 0, d1, 0);
            break;

        case 0x2C:
            INST_NAME("CVTTSD2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEX(q0, 0);
            FCVTZSxwD(gd, q0);
            break;
        case 0x2D:
            INST_NAME("CVTSD2SI Gd, Ex");
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
            FCVTNSxwD(gd, q0);  // 0: Nearest (even)
            B_NEXT_nocond;
            FCVTMSxwD(gd, q0);  // 1: Toward -inf
            B_NEXT_nocond;
            FCVTPSxwD(gd, q0);  // 2: Toward +inf
            B_NEXT_nocond;
            FCVTZSxwD(gd, q0);  // 3: Toward 0
            #else
            FCVTNSxwD(gd, q0);
            #endif
            break;


        case 0x51:
            INST_NAME("SQRTSD Gx, Ex");
            nextop = F8;
            GETGX(v0);
            d1 = fpu_get_scratch(dyn);
            GETEX(d0, 0);
            FSQRTD(d1, d0);
            VMOVeD(v0, 0, d1, 0);
            break;

        case 0x58:
            INST_NAME("ADDSD Gx, Ex");
            nextop = F8;
            GETGX(v0);
            d1 = fpu_get_scratch(dyn);
            GETEX(d0, 0);
            FADDD(d1, v0, d0);  // the high part of the vector is erased...
            VMOVeD(v0, 0, d1, 0);
            break;
        case 0x59:
            INST_NAME("MULSD Gx, Ex");
            nextop = F8;
            GETGX(v0);
            d1 = fpu_get_scratch(dyn);
            GETEX(d0, 0);
            FMULD(d1, v0, d0);
            VMOVeD(v0, 0, d1, 0);
            break;
        case 0x5A:
            INST_NAME("CVTSD2SS Gx, Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(d0, 0);
            d1 = fpu_get_scratch(dyn);
            FCVT_S_D(d1, d0);
            VMOVeS(v0, 0, d1, 0);
            break;

        case 0x5C:
            INST_NAME("SUBSD Gx, Ex");
            nextop = F8;
            GETGX(v0);
            d1 = fpu_get_scratch(dyn);
            GETEX(d0, 0);
            FSUBD(d1, v0, d0);
            VMOVeD(v0, 0, d1, 0);
            break;
        case 0x5D:
            INST_NAME("MINSD Gx, Ex");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            GETEX(v1, 0);
            // MINSD: if any input is NaN, or Ex[0]<Gx[0], copy Ex[0] -> Gx[0]
            d0 = fpu_get_scratch(dyn);
            FMINNMD(d0, v0, v1);    // NaN handling may be slightly different, is that a problem?
            VMOVeD(v0, 0, d0, 0);   // to not erase uper part
            break;
        case 0x5E:
            INST_NAME("DIVSD Gx, Ex");
            nextop = F8;
            GETGX(v0);
            d1 = fpu_get_scratch(dyn);
            GETEX(d0, 0);
            FDIVD(d1, v0, d0);
            VMOVeD(v0, 0, d1, 0);
            break;
        case 0x5F:
            INST_NAME("MAXSD Gx, Ex");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            GETEX(v1, 0);
            // MAXSD: if any input is NaN, or Ex[0]>Gx[0], copy Ex[0] -> Gx[0]
            d0 = fpu_get_scratch(dyn);
            FMAXNMD(d0, v0, v1);    // NaN handling may be slightly different, is that a problem?
            VMOVeD(v0, 0, d0, 0);   // to not erase uper part
            break;

        case 0x70:
            INST_NAME("PSHUFLW Gx, Ex, Ib");
            nextop = F8;
            GETEX(v1, 1);
            GETGX(v0);

            u8 = F8;
            // only low part need to be suffled. VTBL only handle 8bits value, so the 16bits suffles need to be changed in 8bits
            u64 = 0;
            for (int i=0; i<4; ++i) {
                u64 |= ((uint64_t)((u8>>(i*2))&3)*2+0)<<(i*16+0);
                u64 |= ((uint64_t)((u8>>(i*2))&3)*2+1)<<(i*16+8);
            }
            MOV64x(x2, u64);
            d0 = fpu_get_scratch(dyn);
            VMOVQDfrom(d0, 0, x2);
            VTBL1_8(d0, v1, d0);
            VMOVeD(v0, 0, d0, 0);
            if(v0!=v1) {
                VMOVeD(v0, 1, v1, 1);
            }
            break;

        case 0x7C:
            INST_NAME("HADDPS Gx, Ex");
            nextop = F8;
            GETGX(v0);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<4, 15, rex, 0, 0);
                v1 = fpu_get_scratch(dyn);
                VLDR128_U12(v1, ed, fixedaddress);
            }
            VFADDPQS(v0, v0, v1);
            break;
            
        case 0xC2:
            INST_NAME("CMPSD Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0);
            GETEX(v1, 1);
            u8 = F8;
            if((u8&7)==6){
                FCMPD(v1, v0);
            } else {
                FCMPD(v0, v1);
            }
            switch(u8&7) {
                case 0: CSETMx(x2, cEQ); break;   // Equal
                case 1: CSETMx(x2, cMI); break;   // Less than
                case 2: //CSETMx(x2, cLE); break;   // Less or equal (or unordered on ARM64, not on x86...)
                        CSETMx(x2, cPL); CSINVx(x2, xZR, x2, cEQ); break;   // so use a 2 step here, but 1st test inverted because 2nd step invert again
                case 3: CSETMx(x2, cVS); break;   // NaN
                case 4: CSETMx(x2, cNE); break;   // Not Equal (or unordered on ARM, not on X86...)
                case 5: CSETMx(x2, cCS); break;   // Greater or equal or unordered
                case 6: CSETMx(x2, cLT); break;   // Greater or unordered, test inverted, N!=V so unordereded or less than (inverted)
                case 7: CSETMx(x2, cVC); break;   // not NaN
            }
            VMOVQDfrom(v0, 0, x2);
            break;

        case 0xD0:
            INST_NAME("ADDSUBPS Gx, Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(v1, 0);
            q0 = fpu_get_scratch(dyn);
            static float addsubps[4] = {-1.f, 1.f, -1.f, 1.f};
            MAYUSE(addsubps);
            TABLE64(x2, (uintptr_t)&addsubps);
            VLDR128_U12(q0, x2, 0);
            VFMLAQS(v0, v1, q0);
            break;

        case 0xF0:
            INST_NAME("LDDQU Gx,Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3));
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VMOVQ(v0, v1);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<4, 7, rex, 0, 0);
                VLDR128_U12(v0, ed, fixedaddress);
            }
            break;

        default:
            DEFAULT;
    }
    return addr;
}
