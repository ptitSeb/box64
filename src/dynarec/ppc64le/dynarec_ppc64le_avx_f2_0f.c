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

#include "ppc64le_printer.h"
#include "dynarec_ppc64le_private.h"
#include "dynarec_ppc64le_functions.h"
#include "../dynarec_helper.h"
#include "dynarec_ppc64le_helper.h"

uintptr_t dynarec64_AVX_F2_0F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2, gback;
    uint8_t eb1, eb2;
    uint8_t gb1, gb2;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1, v2;
    int q0, q1, q2;
    int d0, d1, d2;
    int s0;
    int64_t j64;
    int64_t fixedaddress, gdoffset;
    int unscaled;
    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(d2);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(v2);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(q2);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(gb1);
    MAYUSE(gb2);
    MAYUSE(wb1);
    MAYUSE(wb2);
    MAYUSE(gback);
    MAYUSE(j64);
    MAYUSE(i32);
    MAYUSE(i32_);
    MAYUSE(u8);
    MAYUSE(s0);
    MAYUSE(cacheupd);
    MAYUSE(gdoffset);

    rex_t rex = vex.rex;

    switch (opcode) {
        case 0x10:
            INST_NAME("VMOVSD Gx, [Vx,] Ex");
            nextop = F8;
            if (MODREG) {
                // reg-reg: 3-operand form: Gx = Vx with low 64 bits replaced by Ex
                GETVYx(v1, 0);
                GETEYSD(v2, 0, 0);
                GETGYx_empty(v0);
                if (MODREG) {
                    // v2 is a full VMX reg; merge low 64 bits from v2, high from v1
                    if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                    // Keep v0 high (ISA dw0), take v2 low (ISA dw1)
                    XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(v2), 1);
                }
            } else {
                // mem: zero dest, load 64-bit double into low 64 bits
                GETGYx_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                LD(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v0), 0, x4);
            }
            break;
        case 0x11:
            INST_NAME("VMOVSD Ex, [Vx,] Gx");
            nextop = F8;
            GETGYx(v0, 0);
            if (MODREG) {
                GETVYx(v1, 0);
                GETEYx_empty(v2, 0);
                if (v2 != v1) XXLOR(VSXREG(v2), VSXREG(v1), VSXREG(v1));
                XXPERMDI(VSXREG(v2), VSXREG(v2), VSXREG(v0), 1);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                MFVSRLD(x4, VSXREG(v0));
                STD(x4, fixedaddress, ed);
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("VMOVDDUP Gx, Ex");
            nextop = F8;
            if (MODREG) {
                GETGY_empty_EY_xy(v0, v1, 0);
            } else {
                GETGYxy_empty(v0);
                v1 = fpu_get_scratch(dyn);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                if (vex.l) {
                    // 256-bit: load full 256 bits — for now just load low 128
                    LXV(VSXREG(v1), fixedaddress, ed);
                } else {
                    LD(x4, fixedaddress, ed);
                    MTVSRDD(VSXREG(v1), x4, x4);
                }
            }
            if (vex.l) {
                // TODO: 256-bit needs both lanes duplicated
                MFVSRLD(x4, VSXREG(v1));
                MTVSRDD(VSXREG(v0), x4, x4);
            } else if (MODREG) {
                MFVSRLD(x4, VSXREG(v1));
                MTVSRDD(VSXREG(v0), x4, x4);
            }
            // else: memory non-vex.l was already handled above with MTVSRDD(v1, x4, x4)
            // and v0 == v1 from fpu_get_scratch... no, v0 is GETGYxy_empty, v1 is scratch
            if (!MODREG && !vex.l) {
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            }
            break;

        case 0x2A:
            INST_NAME("VCVTSI2SD Gx, Vx, Ed");
            nextop = F8;
            GETED(0);
            GETVYx(v1, 0);
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            if (rex.w) {
                MTVSRD(VSXREG(d0), ed);
                XSCVSXDDP(VSXREG(d0), VSXREG(d0));
            } else {
                EXTSW(x4, ed);
                MTVSRD(VSXREG(d0), x4);
                XSCVSXDDP(VSXREG(d0), VSXREG(d0));
            }
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d0), 0);
            break;
        case 0x2C:
            INST_NAME("VCVTTSD2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEYSD(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                MFVSRLD(x4, VSXREG(d0));
                MTVSRD(d1, x4);
            } else {
                MFVSRD(x4, VSXREG(d0));
                MTVSRD(d1, x4);
            }
            MTFSB0(23);
            if (rex.w) {
                FCTIDZ(d1, d1);
                MFVSRD(gd, d1);
            } else {
                FCTIWZ(d1, d1);
                MFVSRWZ(gd, d1);
                ZEROUP(gd);
            }
            MFFS(SCRATCH0);
            STFD(SCRATCH0, -8, xSP);
            LD(x4, -8, xSP);
            RLWINM(x4, x4, 24, 31, 31);
            CMPLDI(x4, 0);
            CBZ_NEXT(x4);
            if (rex.w) {
                MOV64x(gd, 0x8000000000000000LL);
            } else {
                MOV32w(gd, 0x80000000);
            }
            break;
        case 0x2D:
            INST_NAME("VCVTSD2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEYSD(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            MTFSB0(23);
            u8 = sse_setround(dyn, ninst, x4, x5);
            if (MODREG) {
                MFVSRLD(x4, VSXREG(d0));
                MTVSRD(d1, x4);
            } else {
                MFVSRD(x4, VSXREG(d0));
                MTVSRD(d1, x4);
            }
            if (rex.w) {
                FCTID(d1, d1);
                MFVSRD(gd, d1);
            } else {
                FCTIW(d1, d1);
                MFVSRWZ(gd, d1);
                ZEROUP(gd);
            }
            MFFS(SCRATCH0);
            STFD(SCRATCH0, -8, xSP);
            LD(x4, -8, xSP);
            RLWINM(x4, x4, 24, 31, 31);
            x87_restoreround(dyn, ninst, u8);
            CMPLDI(x4, 0);
            CBZ_NEXT(x4);
            if (rex.w) {
                MOV64x(gd, 0x8000000000000000LL);
            } else {
                MOV32w(gd, 0x80000000);
            }
            break;

        case 0x51:
            INST_NAME("VSQRTSD Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSD(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            // Extract Vx x86 low double (ISA dw1) to FPR scratch
            MFVSRLD(x4, VSXREG(v1));
            MTVSRD(VSXREG(d1), x4);
            // Get Ex double into q0
            q0 = fpu_get_scratch(dyn);
            if (MODREG) {
                MFVSRLD(x4, VSXREG(v2));
                MTVSRD(VSXREG(q0), x4);
            } else {
                XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
            }
            XSSQRTDP(VSXREG(d1), VSXREG(q0));
            if (!BOX64ENV(dynarec_fastnan)) {
                // If input was negative (not NaN), sqrt produces positive NaN
                // x86 expects negative NaN — flip sign bit
                XSCMPUDP(6, VSXREG(d1), VSXREG(d1));
                j64 = GETMARK - dyn->native_size;
                BC(BO_FALSE, BI(CR6, CR_SO), j64);  // skip if result not NaN
                // Result is NaN: check if input was negative
                MFVSRD(x4, VSXREG(q0));
                SRDI(x4, x4, 63);
                CMPDI(x4, 0);
                j64 = GETMARK - dyn->native_size;
                BEQ(j64);
                // Input was negative: flip sign
                MFVSRD(x4, VSXREG(d1));
                LI(x5, 1);
                SLDI(x5, x5, 63);
                XOR(x4, x4, x5);
                MTVSRD(VSXREG(d1), x4);
                MARK;
            }
            // Copy Vx to dest, then insert scalar result
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d1), 0);
            break;

        case 0x58:
            INST_NAME("VADDSD Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSD(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            // Extract Vx x86 low double
            MFVSRLD(x4, VSXREG(v1));
            MTVSRD(VSXREG(d1), x4);
            // Get Ex double
            if (MODREG) {
                MFVSRLD(x4, VSXREG(v2));
                MTVSRD(VSXREG(q0), x4);
            } else {
                XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
            }
            if (!BOX64ENV(dynarec_fastnan)) {
                XSCMPUDP(6, VSXREG(d1), VSXREG(q0));
            }
            XSADDDP(VSXREG(d1), VSXREG(d1), VSXREG(q0));
            if (!BOX64ENV(dynarec_fastnan)) {
                j64 = GETMARK - dyn->native_size;
                BC(BO_TRUE, BI(CR6, CR_SO), j64);
                XSCMPUDP(6, VSXREG(d1), VSXREG(d1));
                j64 = GETMARK - dyn->native_size;
                BC(BO_FALSE, BI(CR6, CR_SO), j64);
                MFVSRD(x4, VSXREG(d1));
                LI(x5, 1);
                SLDI(x5, x5, 63);
                XOR(x4, x4, x5);
                MTVSRD(VSXREG(d1), x4);
                MARK;
            }
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d1), 0);
            break;

        case 0x59:
            INST_NAME("VMULSD Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSD(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            MFVSRLD(x4, VSXREG(v1));
            MTVSRD(VSXREG(d1), x4);
            if (MODREG) {
                MFVSRLD(x4, VSXREG(v2));
                MTVSRD(VSXREG(q0), x4);
            } else {
                XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
            }
            if (!BOX64ENV(dynarec_fastnan)) {
                XSCMPUDP(6, VSXREG(d1), VSXREG(q0));
            }
            XSMULDP(VSXREG(d1), VSXREG(d1), VSXREG(q0));
            if (!BOX64ENV(dynarec_fastnan)) {
                j64 = GETMARK - dyn->native_size;
                BC(BO_TRUE, BI(CR6, CR_SO), j64);
                XSCMPUDP(6, VSXREG(d1), VSXREG(d1));
                j64 = GETMARK - dyn->native_size;
                BC(BO_FALSE, BI(CR6, CR_SO), j64);
                MFVSRD(x4, VSXREG(d1));
                LI(x5, 1);
                SLDI(x5, x5, 63);
                XOR(x4, x4, x5);
                MTVSRD(VSXREG(d1), x4);
                MARK;
            }
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d1), 0);
            break;

        case 0x5C:
            INST_NAME("VSUBSD Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSD(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            MFVSRLD(x4, VSXREG(v1));
            MTVSRD(VSXREG(d1), x4);
            if (MODREG) {
                MFVSRLD(x4, VSXREG(v2));
                MTVSRD(VSXREG(q0), x4);
            } else {
                XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
            }
            if (!BOX64ENV(dynarec_fastnan)) {
                XSCMPUDP(6, VSXREG(d1), VSXREG(q0));
            }
            XSSUBDP(VSXREG(d1), VSXREG(d1), VSXREG(q0));
            if (!BOX64ENV(dynarec_fastnan)) {
                j64 = GETMARK - dyn->native_size;
                BC(BO_TRUE, BI(CR6, CR_SO), j64);
                XSCMPUDP(6, VSXREG(d1), VSXREG(d1));
                j64 = GETMARK - dyn->native_size;
                BC(BO_FALSE, BI(CR6, CR_SO), j64);
                MFVSRD(x4, VSXREG(d1));
                LI(x5, 1);
                SLDI(x5, x5, 63);
                XOR(x4, x4, x5);
                MTVSRD(VSXREG(d1), x4);
                MARK;
            }
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d1), 0);
            break;

        case 0x5D:
            INST_NAME("VMINSD Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSD(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            // Extract Vx x86 low double
            MFVSRLD(x4, VSXREG(v1));
            MTVSRD(VSXREG(d1), x4);
            // Get Ex double
            if (MODREG) {
                MFVSRLD(x4, VSXREG(v2));
                MTVSRD(VSXREG(q0), x4);
            } else {
                XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
            }
            // x86 MINSD semantics: if either is NaN, return source (Ex); if equal, return source
            XSCMPUDP(0, VSXREG(d1), VSXREG(q0));
            j64 = GETMARK - dyn->native_size;
            BC(BO_TRUE, BI(CR0, CR_SO), j64);  // unordered → use source
            j64 = GETMARK2 - dyn->native_size;
            BLT(j64);  // dest < src → keep dest
            // dest >= src: take src
            MARK;
            XXLOR(VSXREG(d1), VSXREG(q0), VSXREG(q0));
            MARK2;
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d1), 0);
            break;

        case 0x5E:
            INST_NAME("VDIVSD Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSD(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            MFVSRLD(x4, VSXREG(v1));
            MTVSRD(VSXREG(d1), x4);
            if (MODREG) {
                MFVSRLD(x4, VSXREG(v2));
                MTVSRD(VSXREG(q0), x4);
            } else {
                XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
            }
            if (!BOX64ENV(dynarec_fastnan)) {
                XSCMPUDP(6, VSXREG(d1), VSXREG(q0));
            }
            XSDIVDP(VSXREG(d1), VSXREG(d1), VSXREG(q0));
            if (!BOX64ENV(dynarec_fastnan)) {
                j64 = GETMARK - dyn->native_size;
                BC(BO_TRUE, BI(CR6, CR_SO), j64);
                XSCMPUDP(6, VSXREG(d1), VSXREG(d1));
                j64 = GETMARK - dyn->native_size;
                BC(BO_FALSE, BI(CR6, CR_SO), j64);
                MFVSRD(x4, VSXREG(d1));
                LI(x5, 1);
                SLDI(x5, x5, 63);
                XOR(x4, x4, x5);
                MTVSRD(VSXREG(d1), x4);
                MARK;
            }
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d1), 0);
            break;

        case 0x5F:
            INST_NAME("VMAXSD Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSD(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            MFVSRLD(x4, VSXREG(v1));
            MTVSRD(VSXREG(d1), x4);
            if (MODREG) {
                MFVSRLD(x4, VSXREG(v2));
                MTVSRD(VSXREG(q0), x4);
            } else {
                XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
            }
            // x86 MAXSD semantics: if either is NaN, return source (Ex); if equal, return source
            XSCMPUDP(0, VSXREG(d1), VSXREG(q0));
            j64 = GETMARK - dyn->native_size;
            BC(BO_TRUE, BI(CR0, CR_SO), j64);  // unordered → use source
            j64 = GETMARK2 - dyn->native_size;
            BGT(j64);  // dest > src → keep dest
            // dest <= src: take src
            MARK;
            XXLOR(VSXREG(d1), VSXREG(q0), VSXREG(q0));
            MARK2;
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d1), 0);
            break;

        case 0x7C:
            INST_NAME("VHADDPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            // Horizontal add: result = [Vx[0]+Vx[1], Vx[2]+Vx[3], Ex[0]+Ex[1], Ex[2]+Ex[3]]
            // Separate even and odd float elements using VMRGLW/VMRGHW
            // PPC64LE VMRGLW interleaves low words, VMRGHW interleaves high words
            // Need to get even elements (0,2) and odd elements (1,3) from each
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            // v1 layout: [w0, w1, w2, w3] where w0=ISA low(LE), w3=ISA high
            // VMRGLW(q0, v2, v1): interleaves low halves: [v1.w0, v2.w0, v1.w1, v2.w1]
            // VMRGHW(q1, v2, v1): interleaves high halves: [v1.w2, v2.w2, v1.w3, v2.w3]
            // x86 result: [Vx0+Vx1, Vx2+Vx3, Ex0+Ex1, Ex2+Ex3]
            // ISA order: [Vx2+Vx3, Ex2+Ex3, Vx0+Vx1, Ex0+Ex1]
            // Even elements (w0, w2 per src): q0 = VMRGLW gives [v1.w0, v2.w0, v1.w1, v2.w1]
            // Wait — this is getting confusing. Let me use XXPERMDI to split low/high qwords
            // and then do paired adds.
            // Actually simpler approach: use VSLDOI to shift and create pairs
            // Approach: separate even (0,2) and odd (1,3) elements of each source,
            // then concatenate and add.
            // v1: BE [w3, w2, w1, w0] = x86 [w0, w1, w2, w3]
            // Even (x86 idx 0,2): ISA BE w3, w1 (positions 0, 2 in BE)
            // Odd  (x86 idx 1,3): ISA BE w2, w0 (positions 1, 3 in BE)
            // VSLDOI by 4: shifts left by 4 bytes in BE => [w2,w1,w0,w3]
            // Use VMRGLW/VMRGHW differently:
            // VMRGLW(dst, A, B) on LE: takes low half of A and B words, interleaves
            //   = [B.w0, A.w0, B.w1, A.w1] in LE layout
            // VMRGHW(dst, A, B) on LE: takes high half
            //   = [B.w2, A.w2, B.w3, A.w3] in LE layout
            // For HADDPS we need: result[0]=Vx[0]+Vx[1], result[1]=Vx[2]+Vx[3],
            //                     result[2]=Ex[0]+Ex[1], result[3]=Ex[2]+Ex[3]
            // = add even-indexed with next-odd-indexed for each source
            // Approach: use VSLDOI by 4 to shift each word, getting odd elements aligned with even
            // Then pick elements using merge operations
            // Simpler: extract all through GPR
            {
                // Extract v1 words via GPR
                MFVSRLD(x4, VSXREG(v1));   // x4 = [w1:w0] LE (x86 low qword)
                MFVSRD(x5, VSXREG(v1));    // x5 = [w3:w2] LE (x86 high qword)
                // Extract v2 words via GPR
                MFVSRLD(x6, VSXREG(v2));   // x6 = [w1:w0] of Ex
                MFVSRD(x7, VSXREG(v2));    // x7 = [w3:w2] of Ex
                // Separate even/odd: construct two vectors
                // evens = [v1.w0, v1.w2, v2.w0, v2.w2]
                // odds  = [v1.w1, v1.w3, v2.w1, v2.w3]
                // evens qword low  = v1.w0 | (v1.w2 << 32) = low32(x4) | (low32(x5) << 32)
                // evens qword high = v2.w0 | (v2.w2 << 32)
                RLWINM(x3, x4, 0, 0, 31);  // x3 = v1.w0 (low 32 of x4)
                RLDIMI(x3, x5, 32, 0);     // x3[63:32] = v1.w2 (low 32 of x5)
                // x3 now has evens low qword: [v1.w2, v1.w0] in LE = ISA dw1
                RLWINM(x2, x6, 0, 0, 31);  // x2 = v2.w0
                RLDIMI(x2, x7, 32, 0);     // x2[63:32] = v2.w2
                // x2 = evens high qword: [v2.w2, v2.w0] in LE = ISA dw0
                MTVSRDD(VSXREG(q0), x2, x3);  // q0 = evens vector
                // odds
                SRDI(x3, x4, 32);           // x3 = v1.w1
                SRDI(x2, x5, 32);           // x2 = v1.w3
                RLDIMI(x3, x2, 32, 0);     // x3 = [v1.w3, v1.w1]
                SRDI(x2, x6, 32);           // x2 = v2.w1
                SRDI(x1, x7, 32);           // x1 = v2.w3 (using x1 temp)
                RLDIMI(x2, x1, 32, 0);     // x2 = [v2.w3, v2.w1]
                MTVSRDD(VSXREG(q1), x2, x3);  // q1 = odds vector
            }
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                XVCMPEQSP(VSXREG(d0), VSXREG(q0), VSXREG(q0));
                XVCMPEQSP(VSXREG(d1), VSXREG(q1), VSXREG(q1));
                XXLAND(VSXREG(d0), VSXREG(d0), VSXREG(d1));
            }
            XVADDSP(VSXREG(v0), VSXREG(q0), VSXREG(q1));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQSP(VSXREG(d1), VSXREG(v0), VSXREG(v0));
                XXLANDC(VSXREG(d1), VSXREG(d0), VSXREG(d1));
                XXSPLTIB(VSXREG(d0), 31);
                VSLW(VRREG(d1), VRREG(d1), VRREG(d0));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(d1));
            }
            break;

        case 0x7D:
            INST_NAME("VHSUBPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            {
                MFVSRLD(x4, VSXREG(v1));
                MFVSRD(x5, VSXREG(v1));
                MFVSRLD(x6, VSXREG(v2));
                MFVSRD(x7, VSXREG(v2));
                // evens = [v1.w0, v1.w2, v2.w0, v2.w2]
                RLWINM(x3, x4, 0, 0, 31);
                RLDIMI(x3, x5, 32, 0);
                RLWINM(x2, x6, 0, 0, 31);
                RLDIMI(x2, x7, 32, 0);
                MTVSRDD(VSXREG(q0), x2, x3);
                // odds = [v1.w1, v1.w3, v2.w1, v2.w3]
                SRDI(x3, x4, 32);
                SRDI(x2, x5, 32);
                RLDIMI(x3, x2, 32, 0);
                SRDI(x2, x6, 32);
                SRDI(x1, x7, 32);
                RLDIMI(x2, x1, 32, 0);
                MTVSRDD(VSXREG(q1), x2, x3);
            }
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                XVCMPEQSP(VSXREG(d0), VSXREG(q0), VSXREG(q0));
                XVCMPEQSP(VSXREG(d1), VSXREG(q1), VSXREG(q1));
                XXLAND(VSXREG(d0), VSXREG(d0), VSXREG(d1));
            }
            XVSUBSP(VSXREG(v0), VSXREG(q0), VSXREG(q1));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQSP(VSXREG(d1), VSXREG(v0), VSXREG(v0));
                XXLANDC(VSXREG(d1), VSXREG(d0), VSXREG(d1));
                XXSPLTIB(VSXREG(d0), 31);
                VSLW(VRREG(d1), VRREG(d1), VRREG(d0));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(d1));
            }
            break;

        case 0x5A:
            INST_NAME("VCVTSD2SS Gx, Vx, Ex");
            nextop = F8;
            GETEYSD(v2, 0, 0);
            GETVYx(v1, 0);
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            // Get Ex double into scalar position
            if (MODREG) {
                MFVSRLD(x4, VSXREG(v2));
                MTVSRD(VSXREG(d0), x4);
            } else {
                XXLOR(VSXREG(d0), VSXREG(v2), VSXREG(v2));
            }
            // Convert double to single: XSCVDPSP rounds + converts, result in word 0
            XSCVDPSP(VSXREG(d0), VSXREG(d0));
            VEXTRACTUW(VRREG(d0), VRREG(d0), 0);
            // Copy Vx to dest, then insert float result into LE word 0
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            VINSERTW(VRREG(v0), VRREG(d0), 12);  // LE word 0 = BE byte offset 12
            break;

        case 0xD0:
            INST_NAME("VADDSUBPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            // Result[0] = Vx[0]-Ex[0], Result[1] = Vx[1]+Ex[1]
            // Result[2] = Vx[2]-Ex[2], Result[3] = Vx[3]+Ex[3]
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            // Create sign mask: -0.0 for even elements, 0.0 for odd
            // LE layout: [word0, word1, word2, word3]; want [0x80000000, 0, 0x80000000, 0]
            LI(x4, 1);
            SLDI(x4, x4, 31);  // x4 = 0x00000000_80000000
            MTVSRDD(VSXREG(d0), x4, x4);
            // XOR Ex with sign mask to flip sign of even float lanes.
            // NaN care: XOR must NOT flip sign of NaN inputs, since x86 SUB propagates
            // input NaN sign unchanged. Only apply sign flip to non-NaN lanes of Ex.
            if (!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn);
                // Mask sign flip to non-NaN lanes: XOR only where Ex is ordered
                XVCMPEQSP(VSXREG(q0), VSXREG(v2), VSXREG(v2));  // q0 = -1 where Ex NOT NaN
                XXLAND(VSXREG(d0), VSXREG(d0), VSXREG(q0));      // zero sign flip for NaN lanes
                XVCMPEQSP(VSXREG(d1), VSXREG(v1), VSXREG(v1));  // d1 = -1 where Vx NOT NaN
                XXLAND(VSXREG(d1), VSXREG(d1), VSXREG(q0));      // d1 = both ordered mask
            }
            XXLXOR(VSXREG(d0), VSXREG(v2), VSXREG(d0));
            // Add: Vx + modified_Ex gives sub for even, add for odd
            XVADDSP(VSXREG(v0), VSXREG(v1), VSXREG(d0));
            if (!BOX64ENV(dynarec_fastnan)) {
                // Newly generated NaNs (both inputs ordered, result NaN) — set sign bit
                XVCMPEQSP(VSXREG(d0), VSXREG(v0), VSXREG(v0));  // d0 = -1 where result NOT NaN
                XXLANDC(VSXREG(d0), VSXREG(d1), VSXREG(d0));     // both-ordered AND result-NaN
                XXSPLTIB(VSXREG(d1), 31);
                VSLW(VRREG(d0), VRREG(d0), VRREG(d1));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(d0));      // OR sign bit
            }
            break;

        case 0x70:
            INST_NAME("VPSHUFLW Gx, Ex, Ib");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 1);
            u8 = F8;
            {
                // Shuffle low 4 words (64 bits), keep high 64 bits unchanged
                d0 = fpu_get_scratch(dyn);
                MFVSRLD(x4, VSXREG(v1));  // x86 low 64 bits (ISA dw1)

                // Build result in x5 by selecting halfwords
                int src_word;
                // Word 0 (bits 0-15)
                src_word = (u8 >> 0) & 3;
                if (src_word == 0) {
                    ANDI(x5, x4, 0xFFFF);
                } else {
                    SRDI(x5, x4, src_word * 16);
                    ANDI(x5, x5, 0xFFFF);
                }

                // Word 1 (bits 16-31)
                src_word = (u8 >> 2) & 3;
                SRDI(x6, x4, src_word * 16);
                ANDI(x6, x6, 0xFFFF);
                SLDI(x6, x6, 16);
                OR(x5, x5, x6);

                // Word 2 (bits 32-47)
                src_word = (u8 >> 4) & 3;
                SRDI(x6, x4, src_word * 16);
                ANDI(x6, x6, 0xFFFF);
                SLDI(x6, x6, 32);
                OR(x5, x5, x6);

                // Word 3 (bits 48-63)
                src_word = (u8 >> 6) & 3;
                SRDI(x6, x4, src_word * 16);
                ANDI(x6, x6, 0xFFFF);
                SLDI(x6, x6, 48);
                OR(x5, x5, x6);

                // Get high 64 bits from source (x86 high = ISA dw0)
                MFVSRD(x6, VSXREG(v1));

                // Build result: high 64 from source, low 64 = shuffled
                MTVSRDD(VSXREG(v0), x6, x5);
            }
            break;

        case 0xC2:
            INST_NAME("VCMPSD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSD(v2, 0, 1);
            GETGYx_empty(v0);
            u8 = F8;
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            // Extract Vx scalar double (x86 low = ISA dw1)
            MFVSRLD(x4, VSXREG(v1));
            MTVSRD(VSXREG(d1), x4);
            // Get Ex double
            if (MODREG) {
                MFVSRLD(x4, VSXREG(v2));
                MTVSRD(VSXREG(q0), x4);
            } else {
                XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
            }
            // Compare: XSCMPUDP sets CR0
            XSCMPUDP(0, VSXREG(d1), VSXREG(q0));
            switch (u8 & 0xf) {
                case 0x00: // EQ_OQ
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 3, 31, 31);
                    break;
                case 0x01: // LT_OS
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 1, 31, 31);
                    break;
                case 0x02: // LE_OS
                    MFOCRF(x5, 0x80);
                    RLWINM(x4, x5, 1, 31, 31);
                    RLWINM(x5, x5, 3, 31, 31);
                    OR(x5, x5, x4);
                    break;
                case 0x03: // UNORD_Q
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 4, 31, 31);
                    break;
                case 0x04: // NEQ_UQ
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 3, 31, 31);
                    XORI(x5, x5, 1);
                    break;
                case 0x05: // NLT_US
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 1, 31, 31);
                    XORI(x5, x5, 1);
                    break;
                case 0x06: // NLE_US
                    MFOCRF(x5, 0x80);
                    RLWINM(x4, x5, 1, 31, 31);
                    RLWINM(x5, x5, 3, 31, 31);
                    OR(x5, x5, x4);
                    XORI(x5, x5, 1);
                    break;
                case 0x07: // ORD_Q
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 4, 31, 31);
                    XORI(x5, x5, 1);
                    break;
                case 0x08: // EQ_UQ
                    MFOCRF(x5, 0x80);
                    RLWINM(x4, x5, 3, 31, 31);
                    RLWINM(x5, x5, 4, 31, 31);
                    OR(x5, x5, x4);
                    break;
                case 0x09: // NGE_US
                    MFOCRF(x5, 0x80);
                    RLWINM(x4, x5, 2, 31, 31);
                    RLWINM(x5, x5, 3, 31, 31);
                    OR(x5, x5, x4);
                    XORI(x5, x5, 1);
                    break;
                case 0x0A: // NGT_US
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 2, 31, 31);
                    XORI(x5, x5, 1);
                    break;
                case 0x0B: // FALSE_OQ
                    LI(x5, 0);
                    break;
                case 0x0C: // NEQ_OQ
                    MFOCRF(x5, 0x80);
                    RLWINM(x4, x5, 4, 31, 31);
                    RLWINM(x5, x5, 3, 31, 31);
                    OR(x5, x5, x4);
                    XORI(x5, x5, 1);
                    break;
                case 0x0D: // GE_OS
                    MFOCRF(x5, 0x80);
                    RLWINM(x4, x5, 2, 31, 31);
                    RLWINM(x5, x5, 3, 31, 31);
                    OR(x5, x5, x4);
                    break;
                case 0x0E: // GT_OS
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 2, 31, 31);
                    break;
                case 0x0F: // TRUE_UQ
                    LI(x5, 1);
                    break;
            }
            // x5 = 0 or 1; negate to get 0 or 0xFFFFFFFFFFFFFFFF mask
            NEG(x5, x5);
            // Copy Vx to dest, then insert 64-bit result mask into low 64 bits
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            MTVSRD(VSXREG(d1), x5);
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d1), 0);
            break;

        case 0xE6:
            INST_NAME("VCVTPD2DQ Gx, Ex");
            nextop = F8;
            GETEYxy(v1, 0, 0);
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            MTFSB0(23);  // clear VXCVI
            u8 = sse_setround(dyn, ninst, x4, x5);
            if (vex.l) {
                // 256-bit: convert 4 doubles to 4 int32
                // Need upper 128 bits too — for now only handle low 128
                // Extract 4 doubles: low 128 has double0, double1; upper 128 has double2, double3
                // double0 (x86 lane 0) = ISA dw1 of low 128
                MFVSRLD(x4, VSXREG(v1));
                MTVSRD(d0, x4);
                FCTIW(d0, d0);
                MFVSRWZ(x4, d0);
                // double1 (x86 lane 1) = ISA dw0 of low 128
                MFVSRD(x6, VSXREG(v1));
                MTVSRD(d0, x6);
                FCTIW(d0, d0);
                MFVSRWZ(x6, d0);
                // double2, double3 from upper 128 (YMM high) — need to get v1+1
                // GETEYxy gives us v1 as the full ymm; upper is v1+1 offset
                // For 256-bit, the upper 128 is at sse_get_reg offset... 
                // Actually GETEYxy with vex.l will give us the full ymm pair
                // v1 is the low 128. For vex.l, we need YMM high half which is separate reg
                // Let's use YMM_UPPER(v1) or similar approach
                // For now, pack low two and zero upper
                // Check VXCVI
                MFFS(SCRATCH0);
                STFD(SCRATCH0, -8, xSP);
                LD(x7, -8, xSP);
                RLWINM(x7, x7, 24, 31, 31);
                x87_restoreround(dyn, ninst, u8);
                CMPLDI(x7, 0);
                BEQZ_MARK(x7);
                LI(x4, 0);
                ORI(x4, x4, 0x8000);
                SLDI(x4, x4, 16);
                MR(x6, x4);
                MARK;
                CLRLDI(x4, x4, 32);
                SLDI(x6, x6, 32);
                OR(x4, x4, x6);
                // For 256-bit we'd need double2, double3 in upper 32-bit slots
                // TODO: handle upper 128 for vex.l=1
                MTVSRDD(VSXREG(v0), 0, x4);
            } else {
                // 128-bit: convert 2 doubles to 2 int32, zero upper
                MFVSRLD(x4, VSXREG(v1));    // double0 (x86 low = ISA dw1)
                MTVSRD(d0, x4);
                FCTIW(d0, d0);
                MFVSRWZ(x4, d0);
                MFVSRD(x6, VSXREG(v1));     // double1 (x86 high = ISA dw0)
                MTVSRD(d1, x6);
                FCTIW(d1, d1);
                MFVSRWZ(x6, d1);
                // Check VXCVI
                MFFS(SCRATCH0);
                STFD(SCRATCH0, -8, xSP);
                LD(x7, -8, xSP);
                RLWINM(x7, x7, 24, 31, 31);
                x87_restoreround(dyn, ninst, u8);
                CMPLDI(x7, 0);
                BEQZ_MARK(x7);
                // Overflow: substitute 0x80000000
                LI(x4, 0);
                ORI(x4, x4, 0x8000);
                SLDI(x4, x4, 16);
                MR(x6, x4);
                MARK;
                // Pack: low 32 = x4, next 32 = x6, upper 64 = 0
                CLRLDI(x4, x4, 32);
                SLDI(x6, x6, 32);
                OR(x4, x4, x6);
                MTVSRDD(VSXREG(v0), 0, x4);
            }
            break;

        case 0xF0:
            INST_NAME("VLDDQU Gx, Ex");
            nextop = F8;
            if (MODREG) {
                GETGY_empty_EY_xy(v0, v1, 0);
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                if (vex.l) {
                    int src = (nextop & 7) + (rex.b << 3);
                    if (src != gd) {
                        q0 = fpu_get_scratch(dyn);
                        LXV(VSXREG(q0), offsetof(x64emu_t, ymm[src]), xEmu);
                        STXV(VSXREG(q0), offsetof(x64emu_t, ymm[gd]), xEmu);
                    }
                }
            } else {
                GETGYxy_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DQ_DISP, 0);
                LXV(VSXREG(v0), fixedaddress, ed);
                if (vex.l) {
                    q0 = fpu_get_scratch(dyn);
                    LXV(VSXREG(q0), fixedaddress + 16, ed);
                    STXV(VSXREG(q0), offsetof(x64emu_t, ymm[gd]), xEmu);
                }
            }
            break;

        default:
            DEFAULT;
    }

    return addr;
}
