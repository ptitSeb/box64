#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "ppc64le_emitter.h"
#include "ppc64le_mapping.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "my_cpuid.h"
#include "emu/x87emu_private.h"

#include "ppc64le_printer.h"
#include "dynarec_ppc64le_private.h"
#include "../dynarec_helper.h"
#include "dynarec_ppc64le_functions.h"


uintptr_t dynarec64_F30F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2, gb1, gb2;
    uint8_t tmp1, tmp2, tmp3;
    int32_t i32, i32_;
    int64_t fixedaddress;
    int64_t j64;
    int unscaled;
    int cacheupd = 0;
    int v0, v1, q0, q1, d0, d1;
    MAYUSE(u8);
    MAYUSE(wb1);
    MAYUSE(wb2);
    MAYUSE(gb1);
    MAYUSE(gb2);
    MAYUSE(tmp1);
    MAYUSE(tmp2);
    MAYUSE(tmp3);
    MAYUSE(i32);
    MAYUSE(i32_);
    MAYUSE(j64);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(cacheupd);

    switch (opcode) {
        case 0x10:
            INST_NAME("MOVSS Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                // reg-reg: merge low 32 bits of source into dest, keep upper 96 bits
                v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                // VINSERTW reads from bytes 0:3 of src (NOT LE word 0), so we need to
                // extract LE word 0 (byte offset 12) of v1 first, then insert.
                d0 = fpu_get_scratch(dyn);
                VEXTRACTUW(VRREG(d0), VRREG(v1), 12);   // LE word 0 -> bytes 0:3 of d0
                VINSERTW(VRREG(v0), VRREG(d0), 12);     // bytes 0:3 of d0 -> LE word 0 of v0
            } else {
                // mem: zero dest, load 32-bit float into low 32 bits
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                XXLXOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                LWZ(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v0), 0, x4);
            }
            break;
        case 0x11:
            INST_NAME("MOVSS Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if (MODREG) {
                // reg-reg: merge low 32 bits of source into dest
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 1);
                d0 = fpu_get_scratch(dyn);
                VEXTRACTUW(VRREG(d0), VRREG(v0), 12);   // LE word 0 of v0 -> bytes 0:3 of d0
                VINSERTW(VRREG(v1), VRREG(d0), 12);     // bytes 0:3 of d0 -> LE word 0 of v1
            } else {
                // mem: store low 32 bits (x86 scalar float = LE word 0)
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                MFVSRLD(x4, VSXREG(v0));  // x86 low 64 bits; float is in low 32
                STW(x4, fixedaddress, ed);
                SMWRITE2();
            }
            break;

        case 0x12:
            INST_NAME("MOVSLDUP Gx, Ex");
            nextop = F8;
            GETEX(v1, 0, 0);
            GETGX_empty(v0);
            // Duplicate even (low) 32-bit words: [3,2,1,0] -> [2,2,0,0]
            // Use XXSPLTW-based approach: splat word pairs
            q0 = fpu_get_scratch(dyn);
            // LE word 0 -> word 0,1; LE word 2 -> word 2,3
            XXSPLTW(VSXREG(q0), VSXREG(v1), 3);     // splat LE word 0 (x86 element 0)
            XXSPLTW(VSXREG(v0), VSXREG(v1), 1);      // splat LE word 2 (x86 element 2)
            // q0 = [w0,w0,w0,w0], v0 = [w2,w2,w2,w2]
            // Merge: v0[high64] = v0, v0[low64] = q0
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(q0), 0);
            break;
        case 0x16:
            INST_NAME("MOVSHDUP Gx, Ex");
            nextop = F8;
            GETEX(v1, 0, 0);
            GETGX_empty(v0);
            // Duplicate odd (high) 32-bit words: [3,2,1,0] -> [3,3,1,1]
            q0 = fpu_get_scratch(dyn);
            XXSPLTW(VSXREG(q0), VSXREG(v1), 2);     // splat LE word 1 (x86 element 1)
            XXSPLTW(VSXREG(v0), VSXREG(v1), 0);      // splat LE word 3 (x86 element 3)
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(q0), 0);
            break;

        case 0x1E:
            INST_NAME("NOP / ENDBR32 / ENDBR64");
            nextop = F8;
            FAKEED;
            break;

        case 0x2A:
            INST_NAME("CVTSI2SS Gx, Ed");
            nextop = F8;
            GETGX(v0, 1);
            GETED(0);
            d0 = fpu_get_scratch(dyn);
            if (rex.w) {
                // Convert 64-bit int to single float
                MTVSRD(VSXREG(d0), ed);
                XSCVSXDSP(VSXREG(d0), VSXREG(d0));
            } else {
                // Convert 32-bit int to single float
                EXTSW(x4, ed);
                MTVSRD(VSXREG(d0), x4);
                XSCVSXDSP(VSXREG(d0), VSXREG(d0));
            }
            // Result is in d0 as a double-precision scalar; convert to single in VSX format
            // XSCVDPSPN converts DP scalar to SP in word 0 of result (BE byte 0)
            XSCVDPSPN(VSXREG(d0), VSXREG(d0));
            // Extract to BE byte 0, then insert at LE word 0 (BE byte 12)
            VEXTRACTUW(VRREG(d0), VRREG(d0), 0);
            VINSERTW(VRREG(v0), VRREG(d0), 12);
            break;
        case 0x2C:
            INST_NAME("CVTTSS2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEXSS(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            // Convert float to double in VSX space
            if (MODREG) {
                XXSPLTW(VSXREG(d1), VSXREG(d0), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            } else {
                XXLOR(VSXREG(d1), VSXREG(d0), VSXREG(d0));
            }
            // Move double from VSX to FPR space for FCTIDZ/FCTIWZ
            MFVSRD(x4, VSXREG(d1));    // extract from VSX dw0
            MTVSRD(d1, x4);             // put in FPR space (raw index)
            MTFSB0(23);  // clear VXCVI (always needed on PPC for overflow detection)
            if (rex.w) {
                FCTIDZ(d1, d1);          // truncate to signed 64-bit
                MFVSRD(gd, d1);
            } else {
                FCTIWZ(d1, d1);          // truncate to signed 32-bit
                MFVSRWZ(gd, d1);
                ZEROUP(gd);
            }
            // Check VXCVI: PPC gives 0x7FFF... for positive overflow, x86 wants 0x8000...
            MFFS(SCRATCH0);
            STFD(SCRATCH0, -8, xSP);
            LD(x4, -8, xSP);
            RLWINM(x4, x4, 24, 31, 31);  // extract VXCVI
            CMPLDI(x4, 0);
            CBZ_NEXT(x4);
            if (rex.w) {
                MOV64x(gd, 0x8000000000000000LL);
            } else {
                MOV32w(gd, 0x80000000);
            }
            break;
        case 0x2D:
            INST_NAME("CVTSS2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEXSS(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            MTFSB0(23);  // clear VXCVI (always needed on PPC for overflow detection)
            // Set rounding BEFORE loading value into d1 (sse_setround uses SCRATCH0 which may == d1)
            u8 = sse_setround(dyn, ninst, x4, x5);
            // Convert float to double, then move to FPR space
            if (MODREG) {
                XXSPLTW(VSXREG(d1), VSXREG(d0), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            } else {
                XXLOR(VSXREG(d1), VSXREG(d0), VSXREG(d0));
            }
            // Move to FPR space for FCTID/FCTIW
            MFVSRD(x4, VSXREG(d1));
            MTVSRD(d1, x4);
            if (rex.w) {
                FCTID(d1, d1);            // round using FPSCR rounding mode
                MFVSRD(gd, d1);
            } else {
                FCTIW(d1, d1);
                MFVSRWZ(gd, d1);
                ZEROUP(gd);
            }
            // Check VXCVI BEFORE restoreround (restoreround clears VXCVI via MTFSF)
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
            INST_NAME("SQRTSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                XXSPLTW(VSXREG(d1), VSXREG(d0), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            } else {
                XXLOR(VSXREG(d1), VSXREG(d0), VSXREG(d0));
            }
            XSSQRTDP(VSXREG(d1), VSXREG(d1));
            if (!BOX64ENV(dynarec_fastnan)) {
                // If input was negative (not NaN), sqrt produces positive NaN
                // x86 expects negative NaN — OR in the sign bit
                MFVSRD(x4, VSXREG(d1));     // extract result (double precision)
                SRDI(x5, x4, 63);           // sign bit → bit 0
                CMPDI(x5, 0);
                j64 = GETMARK - dyn->native_size;
                BNE(j64);                   // skip if result is already negative
                // Check if result is NaN
                XSCMPUDP(6, VSXREG(d1), VSXREG(d1));
                j64 = GETMARK - dyn->native_size;
                BC(BO_FALSE, BI(CR6, CR_SO), j64);  // skip if not NaN
                // Result is positive NaN, flip sign bit
                LI(x5, 1);
                SLDI(x5, x5, 63);
                XOR(x4, x4, x5);
                MTVSRD(VSXREG(d1), x4);
                MARK;
            }
            XSCVDPSPN(VSXREG(d1), VSXREG(d1));
            VEXTRACTUW(VRREG(d1), VRREG(d1), 0);
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;

        case 0x52:
            INST_NAME("RSQRTSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            // Convert float to double
            if (MODREG) {
                XXSPLTW(VSXREG(d1), VSXREG(d0), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            } else {
                XXLOR(VSXREG(d1), VSXREG(d0), VSXREG(d0));
            }
            // Compute sqrt then reciprocal (rsqrt = 1/sqrt)
            XSSQRTDP(VSXREG(d1), VSXREG(d1));
            // Load 1.0 for division
            MOV64x(x4, 0x3FF0000000000000LL);  // 1.0 in double
            MTVSRD(VSXREG(q0), x4);
            XSDIVDP(VSXREG(d1), VSXREG(q0), VSXREG(d1));
            XSCVDPSPN(VSXREG(d1), VSXREG(d1));
            VEXTRACTUW(VRREG(d1), VRREG(d1), 0);
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;
        case 0x53:
            INST_NAME("RCPSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            // Convert float to double
            if (MODREG) {
                XXSPLTW(VSXREG(d1), VSXREG(d0), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            } else {
                XXLOR(VSXREG(d1), VSXREG(d0), VSXREG(d0));
            }
            // Compute 1.0 / val
            MOV64x(x4, 0x3FF0000000000000LL);  // 1.0 in double
            MTVSRD(VSXREG(q0), x4);
            XSDIVDP(VSXREG(d1), VSXREG(q0), VSXREG(d1));
            XSCVDPSPN(VSXREG(d1), VSXREG(d1));
            VEXTRACTUW(VRREG(d1), VRREG(d1), 0);
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;

        case 0x58:
            INST_NAME("ADDSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            // Extract Gx float to double
            XXSPLTW(VSXREG(d1), VSXREG(v0), 3);
            XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            // Extract Ex float to double
            if (MODREG) {
                XXSPLTW(VSXREG(q0), VSXREG(d0), 3);
                XSCVSPDPN(VSXREG(q0), VSXREG(q0));
            } else {
                XXLOR(VSXREG(q0), VSXREG(d0), VSXREG(d0));
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
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;
        case 0x59:
            INST_NAME("MULSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            XXSPLTW(VSXREG(d1), VSXREG(v0), 3);
            XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            if (MODREG) {
                XXSPLTW(VSXREG(q0), VSXREG(d0), 3);
                XSCVSPDPN(VSXREG(q0), VSXREG(q0));
            } else {
                XXLOR(VSXREG(q0), VSXREG(d0), VSXREG(d0));
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
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;
        case 0x5A:
            INST_NAME("CVTSS2SD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                XXSPLTW(VSXREG(d1), VSXREG(d0), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            } else {
                XXLOR(VSXREG(d1), VSXREG(d0), VSXREG(d0));  // LFS already converts to double
            }
            // Result is now a double in d1; insert into low 64 bits of v0
            // d1 FPR scalar result is in ISA dw0; insert into v0's ISA dw1 (x86 low)
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d1), 0);
            break;
        case 0x5B:
            INST_NAME("CVTTPS2DQ Gx, Ex");
            nextop = F8;
            GETEX(v1, 0, 0);
            GETGX_empty(v0);
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
            INST_NAME("SUBSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            XXSPLTW(VSXREG(d1), VSXREG(v0), 3);
            XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            if (MODREG) {
                XXSPLTW(VSXREG(q0), VSXREG(d0), 3);
                XSCVSPDPN(VSXREG(q0), VSXREG(q0));
            } else {
                XXLOR(VSXREG(q0), VSXREG(d0), VSXREG(d0));
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
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;
        case 0x5D:
            INST_NAME("MINSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            // Extract Gx float to double
            XXSPLTW(VSXREG(d1), VSXREG(v0), 3);
            XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            // Extract Ex float to double
            if (MODREG) {
                XXSPLTW(VSXREG(q0), VSXREG(d0), 3);
                XSCVSPDPN(VSXREG(q0), VSXREG(q0));
            } else {
                XXLOR(VSXREG(q0), VSXREG(d0), VSXREG(d0));
            }
            // x86 MINSS semantics: if either is NaN, return source (Ex); if equal, return source
            XSCMPUDP(0, VSXREG(d1), VSXREG(q0));
            // CR0 after XSCMPUDP: LT if Gx<Ex, GT if Gx>Ex, EQ if equal, SO if unordered
            // Branch to MARK if unordered (SO) — use source (Ex)
            j64 = GETMARK - dyn->native_size;
            BC(BO_TRUE, BI(CR0, CR_SO), j64);
            // Not unordered; if dest < src, keep dest (skip to MARK2)
            j64 = GETMARK2 - dyn->native_size;
            BLT(j64);
            // dest > src: take src
            MARK;
            XXLOR(VSXREG(d1), VSXREG(q0), VSXREG(q0));
            MARK2;
            XSCVDPSPN(VSXREG(d1), VSXREG(d1));
            VEXTRACTUW(VRREG(d1), VRREG(d1), 0);
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;
        case 0x5E:
            INST_NAME("DIVSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            XXSPLTW(VSXREG(d1), VSXREG(v0), 3);
            XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            if (MODREG) {
                XXSPLTW(VSXREG(q0), VSXREG(d0), 3);
                XSCVSPDPN(VSXREG(q0), VSXREG(q0));
            } else {
                XXLOR(VSXREG(q0), VSXREG(d0), VSXREG(d0));
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
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;
        case 0x5F:
            INST_NAME("MAXSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            // Extract Gx float to double
            XXSPLTW(VSXREG(d1), VSXREG(v0), 3);
            XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            // Extract Ex float to double
            if (MODREG) {
                XXSPLTW(VSXREG(q0), VSXREG(d0), 3);
                XSCVSPDPN(VSXREG(q0), VSXREG(q0));
            } else {
                XXLOR(VSXREG(q0), VSXREG(d0), VSXREG(d0));
            }
            // x86 MAXSS semantics: if either is NaN, return source (Ex); if equal, return source
            XSCMPUDP(0, VSXREG(d1), VSXREG(q0));
            // Branch to MARK if unordered (SO) — use source (Ex)
            j64 = GETMARK - dyn->native_size;
            BC(BO_TRUE, BI(CR0, CR_SO), j64);
            // Not unordered; if dest > src, keep dest (skip to MARK2)
            j64 = GETMARK2 - dyn->native_size;
            BGT(j64);
            // dest < src: take src
            MARK;
            XXLOR(VSXREG(d1), VSXREG(q0), VSXREG(q0));
            MARK2;
            XSCVDPSPN(VSXREG(d1), VSXREG(d1));
            VEXTRACTUW(VRREG(d1), VRREG(d1), 0);
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;

        case 0x38:  /* MAP */
            opcode = F8;
            switch(opcode) {
                case 0xF6:
                    INST_NAME("ADOX Gd, Ed");
                    nextop = F8;
                    READFLAGS(X_OF);
                    SETFLAGS(X_OF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    GETED(0);
                    GETGD;
                    // Extract OF from flags
                    BF_EXTRACT(x3, xFlags, F_OF, F_OF);
                    IFX(X_OF) {
                        if(rex.w) {
                            ADD(x4, gd, ed);
                            CMPLD(x4, gd);       // carry = x4 < gd (unsigned overflow)
                            MFCR(x5);
                            RLWINM(x5, x5, 1, 31, 31);  // extract LT bit from CR0
                            ADD(gd, x4, x3);
                            CMPLD(gd, x4);
                            MFCR(x6);
                            RLWINM(x6, x6, 1, 31, 31);
                        } else {
                            ADD(x4, gd, ed);
                            ZEROUP(x4);
                            ZEROUP2(x7, gd);
                            CMPLW(x4, x7);
                            MFCR(x5);
                            RLWINM(x5, x5, 1, 31, 31);
                            ADD(gd, x4, x3);
                            ZEROUP(gd);
                            CMPLW(gd, x4);
                            MFCR(x6);
                            RLWINM(x6, x6, 1, 31, 31);
                        }
                        OR(x5, x5, x6);
                        BF_INSERT(xFlags, x5, F_OF, F_OF);
                    } else {
                        ADDxw(x5, gd, ed);
                        ADDxw(gd, x5, x3);
                        if (!rex.w) ZEROUP(gd);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;

#define GO(GETFLAGS, NO, YES, NATNO, NATYES, F, I)                                          \
    READFLAGS_FUSION(F, x1, x2, x3, x4, x5);                                                \
    if (!dyn->insts[ninst].nat_flags_fusion) {                                               \
        GETFLAGS;                                                                            \
    }                                                                                        \
    nextop = F8;                                                                             \
    GETGD;                                                                                   \
    if (MODREG) {                                                                            \
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                            \
        if (dyn->insts[ninst].nat_flags_fusion) {                                            \
            /* CMPD_ZR(4) + BC(4) + MR(4) → MARK2; ZEROUP unconditional */                   \
            NATIVEJUMP(NATNO, 12);                                                           \
        } else {                                                                             \
            B##NO##_MARK2(tmp1);                                                             \
        }                                                                                    \
        MR(gd, ed);                                                                          \
        MARK2;                                                                               \
        if (!rex.w) ZEROUP(gd);                                                              \
    } else {                                                                                 \
        addr = geted(dyn, addr, ninst, nextop, &ed, tmp2, tmp3, &fixedaddress, rex, NULL, DS_DISP, 0); \
        if (dyn->insts[ninst].nat_flags_fusion) {                                            \
            /* CMPD_ZR(4) + BC(4) + LDxw(4) → MARK2; ZEROUP unconditional */                 \
            NATIVEJUMP(NATNO, 12);                                                           \
        } else {                                                                             \
            B##NO##_MARK2(tmp1);                                                             \
        }                                                                                    \
        LDxw(gd, ed, fixedaddress);                                                          \
        MARK2;                                                                               \
        if (!rex.w) ZEROUP(gd);                                                              \
    }

            GOCOND(0x40, "CMOV", "Gd, Ed");

#undef GO

        case 0x6F:
            INST_NAME("MOVDQU Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                GETGX_empty(v0);
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                GETGX_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DQ_DISP, 0);
                LXV(VSXREG(v0), fixedaddress, ed);
            }
            break;

        case 0x70:
            INST_NAME("PSHUFHW Gx, Ex, Ib");
            nextop = F8;
            GETEX(v1, 0, 1);
            GETGX(v0, 1);
            u8 = F8;
            // PSHUFHW: shuffle high 4 words using imm8, keep low 4 words unchanged
            {
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
            INST_NAME("MOVQ Gx, Ex");
            nextop = F8;
            GETGX_empty(v0);
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                // Copy low 64 bits, zero upper 64 bits (x86 low = ISA dw1)
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
            INST_NAME("MOVDQU Ex, Gx");
            nextop = F8;
            GETGX(v0, 0);
            if (MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop & 7) + (rex.b << 3));
                XXLOR(VSXREG(v1), VSXREG(v0), VSXREG(v0));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DQ_DISP, 0);
                STXV(VSXREG(v0), fixedaddress, ed);
                SMWRITE2();
            }
            break;

        case 0xE6:
            INST_NAME("CVTDQ2PD Gx, Ex");
            nextop = F8;
            GETEX(v1, 0, 0);
            GETGX_empty(v0);
            // Convert 2 int32 from low 64 bits of source to 2 doubles
            // Extract low 2 dwords
            MFVSRLD(x4, VSXREG(v1));  // x86 low 64 bits (ISA dw1)
            // Low 32 bits = int0
            EXTSW(x5, x4);
            // High 32 bits = int1
            SRDI(x6, x4, 32);
            EXTSW(x6, x6);
            // Convert each to double
            d0 = fpu_get_scratch(dyn);
            MTVSRD(VSXREG(d0), x5);
            XSCVSXDDP(VSXREG(d0), VSXREG(d0));
            d1 = fpu_get_scratch(dyn);
            MTVSRD(VSXREG(d1), x6);
            XSCVSXDDP(VSXREG(d1), VSXREG(d1));
            // Combine: v0 = [double1, double0] where double0 is in low 64
            MFVSRD(x5, VSXREG(d0));
            MFVSRD(x6, VSXREG(d1));
            MTVSRDD(VSXREG(v0), x6, x5);
            break;

        case 0xAE:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                case 1:
                    if(rex.is32bits || !MODREG) {
                        INST_NAME("Illegal AE");
                        FAKEED;
                        UDF();
                    } else {
                        if (((nextop >> 3) & 7) == 1) {
                            INST_NAME("RDGSBASE");
                        } else {
                            INST_NAME("RDFSBASE");
                        }
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        int seg = _FS + ((nextop >> 3) & 7);
                        grab_segdata(dyn, addr, ninst, x4, seg);
                        MVxw(ed, x4);
                    }
                    break;
                case 2:
                case 3:
                    if (rex.is32bits || !MODREG) {
                        INST_NAME("Illegal AE");
                        FAKEED;
                        UDF();
                    } else {
                        if (((nextop >> 3) & 7) == 3) {
                            INST_NAME("WRGSBASE");
                        } else {
                            INST_NAME("WRFSBASE");
                        }
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        int seg = _FS + ((nextop >> 3) & 7) - 2;
                        if (!rex.w) {
                            ZEROUP2(x4, ed);
                            STD(x4, offsetof(x64emu_t, segs_offs[seg]), xEmu);
                        } else {
                            STD(ed, offsetof(x64emu_t, segs_offs[seg]), xEmu);
                        }
                    }
                    break;
                case 5:
                    INST_NAME("(unsupported) INCSSPD/INCSSPQ Ed");
                    FAKEED;
                    UDF();
                    break;
                case 6:
                    INST_NAME("(unsupported) UMONITOR Ed");
                    FAKEED;
                    UDF();
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0xB8:
            INST_NAME("POPCNT Gd, Ed");
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            GETED(0);
            if (rex.w) {
                POPCNTD(gd, ed);
            } else {
                if (MODREG) {
                    ZEROUP2(x4, ed);
                    POPCNTW(gd, x4);
                } else {
                    POPCNTW(gd, ed);
                }
            }
            IFX (X_ALL) {
                CLEAR_FLAGS_(x2);
                // POPCNT clears all flags except ZF; ZF is set if result == 0
                BNEZ_MARK(gd);
                ORI(xFlags, xFlags, 1 << F_ZF);
                MARK;
            }
            break;
        case 0xBC:
            INST_NAME("TZCNT Gd, Ed");
            SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETED(0);
            GETGD;
            if (!rex.w && MODREG) {
                ZEROUP2(x4, ed);
                ed = x4;
            }
            RESTORE_EFLAGS(x6);
            CLEAR_FLAGS(x2);
            if (rex.w) {
                CNTTZD(gd, ed);
            } else {
                // For 32-bit, if ed was from memory it's already zero-extended by LWZ
                CNTTZW(gd, ed);
            }
            // ZF set if result (gd) is 0
            // CF set if ed is 0 (result would be 32 or 64)
            j64 = GETMARK - dyn->native_size;
            BNEZ_MARK(gd);
            ORI(xFlags, xFlags, 1 << F_ZF);
            MARK;
            // CF: if result == bit width (32 or 64), input was 0
            if (rex.w) {
                CMPLDI(gd, 64);
            } else {
                CMPLDI(gd, 32);
            }
            j64 = GETMARK2 - dyn->native_size;
            BNE(j64);
            ORI(xFlags, xFlags, 1 << F_CF);
            MARK2;
            SPILL_EFLAGS();
            break;
        case 0xBD:
            INST_NAME("LZCNT Gd, Ed");
            SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETED(0);
            GETGD;
            if (!rex.w && MODREG) {
                ZEROUP2(x4, ed);
                ed = x4;
            }
            RESTORE_EFLAGS(x6);
            CLEAR_FLAGS(x2);
            if (rex.w) {
                CNTLZD(gd, ed);
            } else {
                CNTLZW(gd, ed);
            }
            // ZF set if result (gd) is 0
            j64 = GETMARK - dyn->native_size;
            BNEZ_MARK(gd);
            ORI(xFlags, xFlags, 1 << F_ZF);
            MARK;
            // CF set if ed is 0 (result == bit width)
            if (rex.w) {
                CMPLDI(gd, 64);
            } else {
                CMPLDI(gd, 32);
            }
            j64 = GETMARK2 - dyn->native_size;
            BNE(j64);
            ORI(xFlags, xFlags, 1 << F_CF);
            MARK2;
            SPILL_EFLAGS();
            break;

        case 0xC2:
            INST_NAME("CMPSS Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(d0, 0, 1);
            u8 = F8;
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            // Extract Gx scalar float to double
            XXSPLTW(VSXREG(d1), VSXREG(v0), 3);
            XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            // Extract Ex scalar float to double
            if (MODREG) {
                XXSPLTW(VSXREG(q0), VSXREG(d0), 3);
                XSCVSPDPN(VSXREG(q0), VSXREG(q0));
            } else {
                XXLOR(VSXREG(q0), VSXREG(d0), VSXREG(d0));
            }
            // Compare using XSCMPUDP — sets CR0: LT, GT, EQ, SO(unordered)
            XSCMPUDP(0, VSXREG(d1), VSXREG(q0));
            // Extract CR0 bits using MFOCRF + RLWINM (same approach as CMPSD)
            switch (u8 & 7) {
                case 0: // EQ
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 3, 31, 31);  // extract EQ bit
                    break;
                case 1: // LT
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 1, 31, 31);  // extract LT bit
                    break;
                case 2: // LE (LT | EQ)
                    MFOCRF(x5, 0x80);
                    RLWINM(x4, x5, 1, 31, 31);  // LT
                    RLWINM(x5, x5, 3, 31, 31);  // EQ
                    OR(x5, x5, x4);
                    break;
                case 3: // UNORD (NaN)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 4, 31, 31);  // extract SO bit
                    break;
                case 4: // NEQ (not equal)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 3, 31, 31);  // EQ bit
                    XORI(x5, x5, 1);              // invert
                    break;
                case 5: // NLT (not less than = >= or unordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 1, 31, 31);  // LT bit
                    XORI(x5, x5, 1);              // invert
                    break;
                case 6: // NLE (not less or equal = > or unordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x4, x5, 1, 31, 31);  // LT
                    RLWINM(x5, x5, 3, 31, 31);  // EQ
                    OR(x5, x5, x4);               // LE
                    XORI(x5, x5, 1);              // invert
                    break;
                case 7: // ORD (not NaN)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 4, 31, 31);  // SO bit
                    XORI(x5, x5, 1);              // invert
                    break;
            }
            // x5 = 0 or 1; negate to get 0 or 0xFFFFFFFF
            NEG(x5, x5);   // 0 -> 0, 1 -> -1 (0xFFFFFFFFFFFFFFFF)
            // MTVSRD puts value in VSX dw0 (BE bytes 0-7); VINSERTW reads word elem 0 (BE bytes 0-3)
            // Since NEG produces full 64-bit result, upper 32 bits = lower 32 bits when 0 or -1
            MTVSRD(VSXREG(d1), x5);
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;

        case 0xD6:
            INST_NAME("MOVQ2DQ Gx, Em");
            nextop = F8;
            GETGX_empty(v0);
            GETEM(d0, 0);
            // Zero dest, copy 64 bits from MMX reg to low 64 bits of XMM
            XXLXOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
            // d0 is in VR space; extract from ISA dw1
            MFVSRLD(x4, VSXREG(d0));
            MTVSRDD(VSXREG(v0), 0, x4);
            break;

        default:
            DEFAULT;
    }

    return addr;
}
