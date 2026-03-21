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

uintptr_t dynarec64_AVX_F3_0F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
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
            INST_NAME("VMOVSS Gx, [Vx,] Ex");
            nextop = F8;
            if (MODREG) {
                // reg-reg: 3-operand form: Gx = Vx with low 32 bits replaced by Ex
                GETVYx(v1, 0);
                GETEYSS(v2, 0, 0);
                GETGYx_empty(v0);
                if (v0 != v1) {
                    XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                }
                // Insert Ex low float (LE word 0) into v0 LE word 0
                // VINSERTW reads from bytes 0:3 of src (BE word 0 = LE word 3),
                // but the float is at LE word 0 (byte offset 12). Use VEXTRACTUW
                // to move it to bytes 0:3 of a scratch, then VINSERTW.
                d0 = fpu_get_scratch(dyn);
                VEXTRACTUW(VRREG(d0), VRREG(v2), 12);   // LE word 0 -> bytes 0:3 of d0
                VINSERTW(VRREG(v0), VRREG(d0), 12);     // bytes 0:3 of d0 -> LE word 0 of v0
            } else {
                // mem: zero dest, load 32-bit float into low 32 bits
                GETGYx_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                XXLXOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                LWZ(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v0), 0, x4);
            }
            break;
        case 0x11:
            INST_NAME("VMOVSS Ex, [Vx,] Gx");
            nextop = F8;
            GETGYx(v0, 0);
            if (MODREG) {
                // reg-reg: 3-operand form: Ex = Vx with low 32 bits replaced by Gx
                GETVYx(v1, 0);
                GETEYx_empty(v2, 0);
                if (v2 != v1) {
                    XXLOR(VSXREG(v2), VSXREG(v1), VSXREG(v1));
                }
                d0 = fpu_get_scratch(dyn);
                VEXTRACTUW(VRREG(d0), VRREG(v0), 12);   // LE word 0 of v0 -> bytes 0:3 of d0
                VINSERTW(VRREG(v2), VRREG(d0), 12);     // bytes 0:3 of d0 -> LE word 0 of v2
            } else {
                // mem: store low 32 bits
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                MFVSRLD(x4, VSXREG(v0));  // x86 low 64 bits; float is in low 32
                STW(x4, fixedaddress, ed);
                SMWRITE2();
            }
            break;

        case 0x12:
            INST_NAME("VMOVSLDUP Gx, Ex");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 0);
            if (vex.l) {
                // 256-bit: duplicate even floats in both 128-bit lanes
                // TODO: 256-bit not yet supported in register allocator, handle low 128 only
                q0 = fpu_get_scratch(dyn);
                XXSPLTW(VSXREG(q0), VSXREG(v1), 3);     // splat LE word 0 (x86 element 0)
                XXSPLTW(VSXREG(v0), VSXREG(v1), 1);      // splat LE word 2 (x86 element 2)
                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(q0), 0);
            } else {
                q0 = fpu_get_scratch(dyn);
                XXSPLTW(VSXREG(q0), VSXREG(v1), 3);     // splat LE word 0
                XXSPLTW(VSXREG(v0), VSXREG(v1), 1);      // splat LE word 2
                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(q0), 0);
            }
            break;
        case 0x16:
            INST_NAME("VMOVSHDUP Gx, Ex");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 0);
            if (vex.l) {
                q0 = fpu_get_scratch(dyn);
                XXSPLTW(VSXREG(q0), VSXREG(v1), 2);     // splat LE word 1 (x86 element 1)
                XXSPLTW(VSXREG(v0), VSXREG(v1), 0);      // splat LE word 3 (x86 element 3)
                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(q0), 0);
            } else {
                q0 = fpu_get_scratch(dyn);
                XXSPLTW(VSXREG(q0), VSXREG(v1), 2);     // splat LE word 1
                XXSPLTW(VSXREG(v0), VSXREG(v1), 0);      // splat LE word 3
                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(q0), 0);
            }
            break;

        case 0x2A:
            INST_NAME("VCVTSI2SS Gx, Vx, Ed");
            nextop = F8;
            GETED(0);
            GETVYx(v1, 0);
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            if (rex.w) {
                MTVSRD(VSXREG(d0), ed);
                XSCVSXDSP(VSXREG(d0), VSXREG(d0));
            } else {
                EXTSW(x4, ed);
                MTVSRD(VSXREG(d0), x4);
                XSCVSXDSP(VSXREG(d0), VSXREG(d0));
            }
            XSCVDPSPN(VSXREG(d0), VSXREG(d0));
            VEXTRACTUW(VRREG(d0), VRREG(d0), 0);
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            VINSERTW(VRREG(v0), VRREG(d0), 12);
            break;
        case 0x2C:
            INST_NAME("VCVTTSS2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEYSS(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            // Convert float to double in VSX space
            if (MODREG) {
                XXSPLTW(VSXREG(d1), VSXREG(d0), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            } else {
                XXLOR(VSXREG(d1), VSXREG(d0), VSXREG(d0));
            }
            // Move double from VSX to FPR space for FCTIDZ/FCTIWZ
            MFVSRD(x4, VSXREG(d1));
            MTVSRD(d1, x4);
            MTFSB0(23);  // clear VXCVI
            if (rex.w) {
                FCTIDZ(d1, d1);
                MFVSRD(gd, d1);
            } else {
                FCTIWZ(d1, d1);
                MFVSRWZ(gd, d1);
                ZEROUP(gd);
            }
            // Check VXCVI
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
            INST_NAME("VCVTSS2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEYSS(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            MTFSB0(23);  // clear VXCVI
            u8 = sse_setround(dyn, ninst, x4, x5);
            if (MODREG) {
                XXSPLTW(VSXREG(d1), VSXREG(d0), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            } else {
                XXLOR(VSXREG(d1), VSXREG(d0), VSXREG(d0));
            }
            MFVSRD(x4, VSXREG(d1));
            MTVSRD(d1, x4);
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
            INST_NAME("VSQRTSS Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                XXSPLTW(VSXREG(d1), VSXREG(v2), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            } else {
                XXLOR(VSXREG(d1), VSXREG(v2), VSXREG(v2));
            }
            XSSQRTDP(VSXREG(d1), VSXREG(d1));
            if (!BOX64ENV(dynarec_fastnan)) {
                MFVSRD(x4, VSXREG(d1));
                SRDI(x5, x4, 63);
                CMPDI(x5, 0);
                j64 = GETMARK - dyn->native_size;
                BNE(j64);
                XSCMPUDP(6, VSXREG(d1), VSXREG(d1));
                j64 = GETMARK - dyn->native_size;
                BC(BO_FALSE, BI(CR6, CR_SO), j64);
                LI(x5, 1);
                SLDI(x5, x5, 63);
                XOR(x4, x4, x5);
                MTVSRD(VSXREG(d1), x4);
                MARK;
            }
            XSCVDPSPN(VSXREG(d1), VSXREG(d1));
            VEXTRACTUW(VRREG(d1), VRREG(d1), 0);
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;
        case 0x52:
            INST_NAME("VRSQRTSS Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            if (MODREG) {
                XXSPLTW(VSXREG(d1), VSXREG(v2), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            } else {
                XXLOR(VSXREG(d1), VSXREG(v2), VSXREG(v2));
            }
            XSSQRTDP(VSXREG(d1), VSXREG(d1));
            MOV64x(x4, 0x3FF0000000000000LL);  // 1.0 in double
            MTVSRD(VSXREG(q0), x4);
            XSDIVDP(VSXREG(d1), VSXREG(q0), VSXREG(d1));
            XSCVDPSPN(VSXREG(d1), VSXREG(d1));
            VEXTRACTUW(VRREG(d1), VRREG(d1), 0);
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;
        case 0x53:
            INST_NAME("VRCPSS Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            if (MODREG) {
                XXSPLTW(VSXREG(d1), VSXREG(v2), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            } else {
                XXLOR(VSXREG(d1), VSXREG(v2), VSXREG(v2));
            }
            MOV64x(x4, 0x3FF0000000000000LL);  // 1.0 in double
            MTVSRD(VSXREG(q0), x4);
            XSDIVDP(VSXREG(d1), VSXREG(q0), VSXREG(d1));
            XSCVDPSPN(VSXREG(d1), VSXREG(d1));
            VEXTRACTUW(VRREG(d1), VRREG(d1), 0);
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;

        case 0x58:
            INST_NAME("VADDSS Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            // Extract Vx scalar float to double
            XXSPLTW(VSXREG(d1), VSXREG(v1), 3);
            XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            // Extract Ex scalar float to double
            if (MODREG) {
                XXSPLTW(VSXREG(q0), VSXREG(v2), 3);
                XSCVSPDPN(VSXREG(q0), VSXREG(q0));
            } else {
                XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
            }
            if (!BOX64ENV(dynarec_fastnan)) {
                XSCMPUDP(6, VSXREG(d1), VSXREG(q0));
            }
            XSADDSP(VSXREG(d1), VSXREG(d1), VSXREG(q0));
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
            XSCVDPSPN(VSXREG(d1), VSXREG(d1));
            VEXTRACTUW(VRREG(d1), VRREG(d1), 0);
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;
        case 0x59:
            INST_NAME("VMULSS Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            XXSPLTW(VSXREG(d1), VSXREG(v1), 3);
            XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            if (MODREG) {
                XXSPLTW(VSXREG(q0), VSXREG(v2), 3);
                XSCVSPDPN(VSXREG(q0), VSXREG(q0));
            } else {
                XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
            }
            if (!BOX64ENV(dynarec_fastnan)) {
                XSCMPUDP(6, VSXREG(d1), VSXREG(q0));
            }
            XSMULSP(VSXREG(d1), VSXREG(d1), VSXREG(q0));
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
            XSCVDPSPN(VSXREG(d1), VSXREG(d1));
            VEXTRACTUW(VRREG(d1), VRREG(d1), 0);
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;

        case 0x5A:
            INST_NAME("VCVTSS2SD Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                // v2 is a full VMX reg, float in LE word 0 (ISA word 3)
                XXSPLTW(VSXREG(d1), VSXREG(v2), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            } else {
                // v2 is a scratch with float already converted to double by GETEYSS
                XXLOR(VSXREG(d1), VSXREG(v2), VSXREG(v2));
            }
            // Copy Vx to dest (preserves upper bits)
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            // Insert double result into low 64 bits: d1 has scalar in ISA dw0
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d1), 0);
            break;

        case 0x5B:
            INST_NAME("VCVTTPS2DQ Gx, Ex");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 0);
            // Truncate 4 packed floats to 4 int32
            XVCVSPSXWS(VSXREG(v0), VSXREG(v1));    // always truncates
            // Fix positive overflow: PPC gives 0x7FFFFFFF, x86 wants 0x80000000
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            VSPLTISW(VRREG(q0), -1);                   // q0 = 0xFFFFFFFF per lane
            VSPLTISW(VRREG(q1), 1);
            VSRW(VRREG(q0), VRREG(q0), VRREG(q1));     // q0 = 0x7FFFFFFF
            VCMPEQUW(VRREG(q0), VRREG(v0), VRREG(q0)); // mask where result == 0x7FFFFFFF
            VSUBUWM(VRREG(v0), VRREG(v0), VRREG(q0));  // 0x7FFFFFFF - (-1) = 0x80000000
            break;
        case 0x5C:
            INST_NAME("VSUBSS Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            XXSPLTW(VSXREG(d1), VSXREG(v1), 3);
            XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            if (MODREG) {
                XXSPLTW(VSXREG(q0), VSXREG(v2), 3);
                XSCVSPDPN(VSXREG(q0), VSXREG(q0));
            } else {
                XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
            }
            if (!BOX64ENV(dynarec_fastnan)) {
                XSCMPUDP(6, VSXREG(d1), VSXREG(q0));
            }
            XSSUBSP(VSXREG(d1), VSXREG(d1), VSXREG(q0));
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
            XSCVDPSPN(VSXREG(d1), VSXREG(d1));
            VEXTRACTUW(VRREG(d1), VRREG(d1), 0);
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;

        case 0x5D:
            INST_NAME("VMINSS Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            // Extract Vx scalar float to double
            XXSPLTW(VSXREG(d1), VSXREG(v1), 3);    // ISA word 3 = LE word 0
            XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            // Extract Ex scalar float to double
            if (MODREG) {
                XXSPLTW(VSXREG(q0), VSXREG(v2), 3);
                XSCVSPDPN(VSXREG(q0), VSXREG(q0));
            } else {
                // GETEYSS for memory already gives a double in ISA dw0
                XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
            }
            // x86 MINSS semantics: if either is NaN or equal, return source (Ex)
            XSCMPUDP(0, VSXREG(d1), VSXREG(q0));
            j64 = GETMARK - dyn->native_size;
            BC(BO_TRUE, BI(CR0, CR_SO), j64);  // unordered → use source
            j64 = GETMARK2 - dyn->native_size;
            BLT(j64);  // dest < src → keep dest
            // dest >= src: take src
            MARK;
            XXLOR(VSXREG(d1), VSXREG(q0), VSXREG(q0));
            MARK2;
            // Convert result back to float
            XSCVDPSPN(VSXREG(d1), VSXREG(d1));
            // Copy Vx to dest (preserves upper bits)
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            // Insert float result into LE word 0 (BE byte offset 12)
            VEXTRACTUW(VRREG(d1), VRREG(d1), 0);  // after XSCVDPSPN, float is in ISA word 0 (BE byte 0)
            VINSERTW(VRREG(v0), VRREG(d1), 12);    // LE word 0 = BE byte offset 12
            break;
        case 0x5E:
            INST_NAME("VDIVSS Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            XXSPLTW(VSXREG(d1), VSXREG(v1), 3);
            XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            if (MODREG) {
                XXSPLTW(VSXREG(q0), VSXREG(v2), 3);
                XSCVSPDPN(VSXREG(q0), VSXREG(q0));
            } else {
                XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
            }
            if (!BOX64ENV(dynarec_fastnan)) {
                XSCMPUDP(6, VSXREG(d1), VSXREG(q0));
            }
            XSDIVSP(VSXREG(d1), VSXREG(d1), VSXREG(q0));
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
            XSCVDPSPN(VSXREG(d1), VSXREG(d1));
            VEXTRACTUW(VRREG(d1), VRREG(d1), 0);
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;

        case 0x5F:
            INST_NAME("VMAXSS Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            // Extract Vx scalar float to double
            XXSPLTW(VSXREG(d1), VSXREG(v1), 3);    // ISA word 3 = LE word 0
            XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            // Extract Ex scalar float to double
            if (MODREG) {
                XXSPLTW(VSXREG(q0), VSXREG(v2), 3);
                XSCVSPDPN(VSXREG(q0), VSXREG(q0));
            } else {
                XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
            }
            // x86 MAXSS semantics: if either is NaN or equal, return source (Ex)
            XSCMPUDP(0, VSXREG(d1), VSXREG(q0));
            j64 = GETMARK - dyn->native_size;
            BC(BO_TRUE, BI(CR0, CR_SO), j64);  // unordered → use source
            j64 = GETMARK2 - dyn->native_size;
            BGT(j64);  // dest > src → keep dest
            // dest <= src: take src
            MARK;
            XXLOR(VSXREG(d1), VSXREG(q0), VSXREG(q0));
            MARK2;
            // Convert result back to float
            XSCVDPSPN(VSXREG(d1), VSXREG(d1));
            // Copy Vx to dest (preserves upper bits)
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            // Insert float result into LE word 0 (BE byte offset 12)
            VEXTRACTUW(VRREG(d1), VRREG(d1), 0);
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;

        case 0x6F:
            INST_NAME("VMOVDQU Gx, Ex");
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
        case 0x70:
            INST_NAME("VPSHUFHW Gx, Ex, Ib");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 1);
            u8 = F8;
            {
                // Shuffle high 4 words (64 bits), keep low 64 bits unchanged
                // Extract high 64 bits (x86 high = ISA dw0)
                MFVSRD(x4, VSXREG(v1));
                int src_word;
                // Word 0 of high qword (bits 0-15)
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
                // Get low 64 bits from source (x86 low = ISA dw1)
                MFVSRLD(x6, VSXREG(v1));
                // Build result: high 64 = shuffled, low 64 = unchanged
                MTVSRDD(VSXREG(v0), x5, x6);
            }
            break;

        case 0x7E:
            INST_NAME("VMOVQ Gx, Ex");
            nextop = F8;
            GETGYx_empty(v0);
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
                // Copy low 64 bits, zero upper 64 bits
                MFVSRLD(x4, VSXREG(v1));
                MTVSRDD(VSXREG(v0), 0, x4);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                LD(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v0), 0, x4);
            }
            break;
        case 0x7F:
            INST_NAME("VMOVDQU Ex, Gx");
            nextop = F8;
            GETGYxy(v0, 0);
            if (MODREG) {
                GETEYxy_empty(v1, 0);
                XXLOR(VSXREG(v1), VSXREG(v0), VSXREG(v0));
                if (vex.l) {
                    int dest = (nextop & 7) + (rex.b << 3);
                    if (dest != gd) {
                        q0 = fpu_get_scratch(dyn);
                        LXV(VSXREG(q0), offsetof(x64emu_t, ymm[gd]), xEmu);
                        STXV(VSXREG(q0), offsetof(x64emu_t, ymm[dest]), xEmu);
                    }
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DQ_DISP, 0);
                STXV(VSXREG(v0), fixedaddress, ed);
                if (vex.l) {
                    q0 = fpu_get_scratch(dyn);
                    LXV(VSXREG(q0), offsetof(x64emu_t, ymm[gd]), xEmu);
                    STXV(VSXREG(q0), fixedaddress + 16, ed);
                }
                SMWRITE2();
            }
            break;

        case 0xC2:
            INST_NAME("VCMPSS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 1);
            GETGYx_empty(v0);
            u8 = F8;
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            // Extract Vx scalar float to double
            XXSPLTW(VSXREG(d1), VSXREG(v1), 3);    // ISA word 3 = LE word 0
            XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            // Extract Ex scalar float to double
            if (MODREG) {
                XXSPLTW(VSXREG(q0), VSXREG(v2), 3);
                XSCVSPDPN(VSXREG(q0), VSXREG(q0));
            } else {
                XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
            }
            // Compare: XSCMPUDP sets CR0: LT, GT, EQ, SO(unordered)
            XSCMPUDP(0, VSXREG(d1), VSXREG(q0));
            switch (u8 & 0xf) {
                case 0x00: // EQ_OQ (equal, ordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 3, 31, 31);  // EQ bit
                    break;
                case 0x01: // LT_OS (less than, ordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 1, 31, 31);  // LT bit
                    break;
                case 0x02: // LE_OS (less or equal, ordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x4, x5, 1, 31, 31);  // LT
                    RLWINM(x5, x5, 3, 31, 31);  // EQ
                    OR(x5, x5, x4);
                    break;
                case 0x03: // UNORD_Q (unordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 4, 31, 31);  // SO bit
                    break;
                case 0x04: // NEQ_UQ (not equal, unordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 3, 31, 31);  // EQ bit
                    XORI(x5, x5, 1);              // invert
                    break;
                case 0x05: // NLT_US (not less than = GE or unordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 1, 31, 31);  // LT bit
                    XORI(x5, x5, 1);              // invert
                    break;
                case 0x06: // NLE_US (not less or equal = GT or unordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x4, x5, 1, 31, 31);  // LT
                    RLWINM(x5, x5, 3, 31, 31);  // EQ
                    OR(x5, x5, x4);               // LE
                    XORI(x5, x5, 1);              // invert
                    break;
                case 0x07: // ORD_Q (ordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 4, 31, 31);  // SO bit
                    XORI(x5, x5, 1);              // invert
                    break;
                case 0x08: // EQ_UQ (equal or unordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x4, x5, 3, 31, 31);  // EQ
                    RLWINM(x5, x5, 4, 31, 31);  // SO (unordered)
                    OR(x5, x5, x4);
                    break;
                case 0x09: // NGE_US (not GE = LT or unordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x4, x5, 2, 31, 31);  // GT bit
                    RLWINM(x5, x5, 3, 31, 31);  // EQ bit
                    OR(x5, x5, x4);               // GE = GT | EQ
                    XORI(x5, x5, 1);              // invert
                    break;
                case 0x0A: // NGT_US (not GT = LE or unordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 2, 31, 31);  // GT bit
                    XORI(x5, x5, 1);              // invert
                    break;
                case 0x0B: // FALSE_OQ
                    LI(x5, 0);
                    break;
                case 0x0C: // NEQ_OQ (not equal, ordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x4, x5, 4, 31, 31);  // SO (unordered)
                    RLWINM(x5, x5, 3, 31, 31);  // EQ
                    OR(x5, x5, x4);               // EQ | unordered
                    XORI(x5, x5, 1);              // invert → NEQ AND ordered
                    break;
                case 0x0D: // GE_OS (greater or equal, ordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x4, x5, 2, 31, 31);  // GT
                    RLWINM(x5, x5, 3, 31, 31);  // EQ
                    OR(x5, x5, x4);
                    break;
                case 0x0E: // GT_OS (greater, ordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 2, 31, 31);  // GT bit
                    break;
                case 0x0F: // TRUE_UQ
                    LI(x5, 1);
                    break;
            }
            // x5 = 0 or 1; negate to get 0x00000000 or 0xFFFFFFFF mask
            NEG(x5, x5);   // 0 → 0, 1 → 0xFFFFFFFFFFFFFFFF
            // Copy Vx to dest (preserves upper bits)
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            // Insert 32-bit mask into LE word 0
            MTVSRD(VSXREG(d1), x5);
            VINSERTW(VRREG(v0), VRREG(d1), 12);  // LE word 0 = BE byte offset 12
            break;

        case 0xE6:
            INST_NAME("VCVTDQ2PD Gx, Ex");
            nextop = F8;
            if (vex.l) {
                // 256-bit: convert 4 int32 from low 128 bits of source to 4 doubles
                GETEYx(v1, 0, 0);
                GETGYy_empty(v0);
                // Extract 4 int32 values from low 128 bits of source
                MFVSRLD(x4, VSXREG(v1));  // x86 low 64 bits (int0, int1)
                MFVSRD(x5, VSXREG(v1));   // x86 high 64 bits (int2, int3)
                // int0 = low 32 of x4
                EXTSW(x6, x4);
                d0 = fpu_get_scratch(dyn);
                MTVSRD(VSXREG(d0), x6);
                XSCVSXDDP(VSXREG(d0), VSXREG(d0));
                // int1 = high 32 of x4
                SRDI(x6, x4, 32);
                EXTSW(x6, x6);
                d1 = fpu_get_scratch(dyn);
                MTVSRD(VSXREG(d1), x6);
                XSCVSXDDP(VSXREG(d1), VSXREG(d1));
                // Combine low pair: v0 = [double1, double0]
                MFVSRD(x4, VSXREG(d0));
                MFVSRD(x6, VSXREG(d1));
                MTVSRDD(VSXREG(v0), x6, x4);
                // int2 = low 32 of x5
                EXTSW(x6, x5);
                MTVSRD(VSXREG(d0), x6);
                XSCVSXDDP(VSXREG(d0), VSXREG(d0));
                // int3 = high 32 of x5
                SRDI(x6, x5, 32);
                EXTSW(x6, x6);
                MTVSRD(VSXREG(d1), x6);
                XSCVSXDDP(VSXREG(d1), VSXREG(d1));
                // TODO: store upper 128 bits when 256-bit YMM support is added
                // For now, only low 128 bits handled via register allocator
            } else {
                // 128-bit: convert 2 int32 from low 64 bits to 2 doubles
                GETEYx(v1, 0, 0);
                GETGYx_empty(v0);
                MFVSRLD(x4, VSXREG(v1));  // x86 low 64 bits
                EXTSW(x5, x4);
                SRDI(x6, x4, 32);
                EXTSW(x6, x6);
                d0 = fpu_get_scratch(dyn);
                MTVSRD(VSXREG(d0), x5);
                XSCVSXDDP(VSXREG(d0), VSXREG(d0));
                d1 = fpu_get_scratch(dyn);
                MTVSRD(VSXREG(d1), x6);
                XSCVSXDDP(VSXREG(d1), VSXREG(d1));
                MFVSRD(x5, VSXREG(d0));
                MFVSRD(x6, VSXREG(d1));
                MTVSRDD(VSXREG(v0), x6, x5);
            }
            break;

        default:
            DEFAULT;
    }

    return addr;
}
