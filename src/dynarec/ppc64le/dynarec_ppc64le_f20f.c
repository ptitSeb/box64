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


uintptr_t dynarec64_F20F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback, wb1;
    int64_t fixedaddress;
    int64_t j64;
    int unscaled;
    int v0, v1, q0, q1, d0, d1;
    MAYUSE(u8);
    MAYUSE(wb1);
    MAYUSE(j64);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(d0);
    MAYUSE(d1);

    switch (opcode) {
        case 0x10:
            INST_NAME("MOVSD Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                // reg-reg: merge low 64 bits of source into dest, keep upper 64 bits
                v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                // Keep v0 high (ISA dw0), take v1 low (ISA dw1)
                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(v1), 1);
            } else {
                // mem: zero dest, load 64-bit double into low 64 bits
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                LD(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v0), 0, x4);  // high=0, low=x4 (x86 scalar in LE low)
            }
            break;
        case 0x11:
            INST_NAME("MOVSD Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if (MODREG) {
                // reg-reg: merge low 64 bits of source into dest
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 1);
                // Keep v1 high (ISA dw0), take v0 low (ISA dw1)
                XXPERMDI(VSXREG(v1), VSXREG(v1), VSXREG(v0), 1);
            } else {
                // mem: store low 64 bits
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                MFVSRLD(x4, VSXREG(v0));
                STD(x4, fixedaddress, ed);
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("MOVDDUP Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                // Duplicate low 64 bits (ISA dw1) of source to both halves
                MFVSRLD(x4, VSXREG(v1));
                MTVSRDD(VSXREG(v0), x4, x4);
            } else {
                SMREAD();
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                LD(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v0), x4, x4);
            }
            break;
        case 0x2A:
            INST_NAME("CVTSI2SD Gx, Ed");
            nextop = F8;
            GETGX(v0, 1);
            GETED(0);
            d0 = fpu_get_scratch(dyn);
            if (rex.w) {
                // Convert 64-bit int to double
                MTVSRD(VSXREG(d0), ed);
                XSCVSXDDP(VSXREG(d0), VSXREG(d0));
            } else {
                // Convert 32-bit int to double
                EXTSW(x4, ed);
                MTVSRD(VSXREG(d0), x4);
                XSCVSXDDP(VSXREG(d0), VSXREG(d0));
            }
            // Result is in d0 as double-precision scalar; insert into low 64 bits of v0
            // FPR scalar result is in ISA dw0; use XXPERMDI to place it into v0's ISA dw1 (x86 low)
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d0), 0);
            break;
        case 0x2C:
            INST_NAME("CVTTSD2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEXSD(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            // Extract double to FPR scratch (raw index for FPR instructions)
            if (MODREG) {
                MFVSRLD(x4, VSXREG(d0));  // x86 scalar double is in ISA dw1
                MTVSRD(d1, x4);            // raw index → FPR space (VS24)
            } else {
                MFVSRD(x4, VSXREG(d0));   // memory load put data in ISA dw0 via MTVSRD(VSXREG)
                MTVSRD(d1, x4);            // raw index → FPR space
            }
            MTFSB0(23);  // clear VXCVI before conversion
            if (rex.w) {
                FCTIDZ(d1, d1);           // truncate to signed 64-bit (FPR instruction)
                MFVSRD(gd, d1);           // raw index → read from FPR space
            } else {
                FCTIWZ(d1, d1);           // truncate to signed 32-bit (FPR instruction)
                MFVSRWZ(gd, d1);          // extract 32-bit result
                ZEROUP(gd);
            }
            // Check VXCVI: PPC gives 0x7FFF... for positive overflow, x86 wants 0x8000...
            MFFS(SCRATCH0);
            STFD(SCRATCH0, -8, xSP);
            LD(x4, -8, xSP);
            RLWINM(x4, x4, 24, 31, 31);  // extract VXCVI (bit 8 from LSB) → bit 0
            CMPLDI(x4, 0);
            CBZ_NEXT(x4);
            if (rex.w) {
                MOV64x(gd, 0x8000000000000000LL);
            } else {
                MOV32w(gd, 0x80000000);
            }
            break;
        case 0x2D:
            INST_NAME("CVTSD2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEXSD(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            MTFSB0(23);  // clear VXCVI (always needed on PPC for overflow detection)
            // Set rounding BEFORE loading value into d1 (sse_setround uses SCRATCH0 which may == d1)
            u8 = sse_setround(dyn, ninst, x4, x5);
            // Extract double to FPR scratch (raw index for FPR instructions)
            if (MODREG) {
                MFVSRLD(x4, VSXREG(d0));
                MTVSRD(d1, x4);
            } else {
                MFVSRD(x4, VSXREG(d0));
                MTVSRD(d1, x4);
            }
            if (rex.w) {
                FCTID(d1, d1);            // round to signed 64-bit using FPSCR rounding mode
                MFVSRD(gd, d1);
            } else {
                FCTIW(d1, d1);            // round to signed 32-bit using FPSCR rounding mode
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
            INST_NAME("SQRTSD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                MFVSRLD(x4, VSXREG(d0));
                MTVSRD(VSXREG(d1), x4);
            } else {
                XXLOR(VSXREG(d1), VSXREG(d0), VSXREG(d0));
            }
            XSSQRTDP(VSXREG(d1), VSXREG(d1));
            if (!BOX64ENV(dynarec_fastnan)) {
                // If input was negative (not NaN), sqrt produces positive NaN
                // x86 expects negative NaN — OR in the sign bit
                // First check if result is NaN
                XSCMPUDP(6, VSXREG(d1), VSXREG(d1));
                j64 = GETMARK - dyn->native_size;
                BC(BO_FALSE, BI(CR6, CR_SO), j64);  // skip if result not NaN
                // Result is NaN: check if input was negative (not NaN itself)
                if (MODREG) {
                    MFVSRLD(x4, VSXREG(d0));
                } else {
                    MFVSRD(x4, VSXREG(d0));
                }
                SRDI(x4, x4, 63);           // sign bit → bit 0
                CMPDI(x4, 0);
                j64 = GETMARK - dyn->native_size;
                BEQ(j64);                    // skip if input was not negative
                // Input was negative, result is new NaN: flip sign bit
                MFVSRD(x4, VSXREG(d1));     // extract result (ISA dw0 = scalar)
                LI(x5, 1);
                SLDI(x5, x5, 63);           // x5 = 0x8000000000000000
                XOR(x4, x4, x5);            // flip sign bit
                MTVSRD(VSXREG(d1), x4);     // put back
                MARK;
            }
            // Insert FPR result (dw0) into v0's x86 low (ISA dw1), keep v0's x86 high (ISA dw0)
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d1), 0);
            break;
        case 0x58:
            INST_NAME("ADDSD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            // Extract Gx x86 low double (ISA dw1) to FPR scratch
            MFVSRLD(x4, VSXREG(v0));
            MTVSRD(VSXREG(d1), x4);
            // Get Ex double
            if (MODREG) {
                MFVSRLD(x4, VSXREG(d0));
                MTVSRD(VSXREG(q0), x4);
            } else {
                XXLOR(VSXREG(q0), VSXREG(d0), VSXREG(d0));
            }
            if (!BOX64ENV(dynarec_fastnan)) {
                // Check if either input is NaN before the operation
                XSCMPUDP(6, VSXREG(d1), VSXREG(q0));  // compare to CR6
            }
            XSADDDP(VSXREG(d1), VSXREG(d1), VSXREG(q0));
            if (!BOX64ENV(dynarec_fastnan)) {
                // Skip fixup if either input was NaN (pre-existing NaN passes through)
                j64 = GETMARK - dyn->native_size;
                BC(BO_TRUE, BI(CR6, CR_SO), j64);  // branch if unordered (input had NaN)
                // Check if result is NaN
                XSCMPUDP(6, VSXREG(d1), VSXREG(d1));
                j64 = GETMARK - dyn->native_size;
                BC(BO_FALSE, BI(CR6, CR_SO), j64);  // branch if ordered (result not NaN)
                // New NaN created (e.g. Inf + -Inf): flip sign bit
                MFVSRD(x4, VSXREG(d1));
                LI(x5, 1);
                SLDI(x5, x5, 63);
                XOR(x4, x4, x5);
                MTVSRD(VSXREG(d1), x4);
                MARK;
            }
            // Insert FPR result back into v0 x86 low
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d1), 0);
            break;
        case 0x59:
            INST_NAME("MULSD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            MFVSRLD(x4, VSXREG(v0));
            MTVSRD(VSXREG(d1), x4);
            if (MODREG) {
                MFVSRLD(x4, VSXREG(d0));
                MTVSRD(VSXREG(q0), x4);
            } else {
                XXLOR(VSXREG(q0), VSXREG(d0), VSXREG(d0));
            }
            if (!BOX64ENV(dynarec_fastnan)) {
                // Check if either input is NaN before the operation
                XSCMPUDP(6, VSXREG(d1), VSXREG(q0));  // compare to CR6
            }
            XSMULDP(VSXREG(d1), VSXREG(d1), VSXREG(q0));
            if (!BOX64ENV(dynarec_fastnan)) {
                // Skip fixup if either input was NaN (pre-existing NaN passes through)
                j64 = GETMARK - dyn->native_size;
                BC(BO_TRUE, BI(CR6, CR_SO), j64);  // branch if unordered (input had NaN)
                // Check if result is NaN
                XSCMPUDP(6, VSXREG(d1), VSXREG(d1));
                j64 = GETMARK - dyn->native_size;
                BC(BO_FALSE, BI(CR6, CR_SO), j64);  // branch if ordered (result not NaN)
                // New NaN created: flip sign bit
                MFVSRD(x4, VSXREG(d1));
                LI(x5, 1);
                SLDI(x5, x5, 63);
                XOR(x4, x4, x5);
                MTVSRD(VSXREG(d1), x4);
                MARK;
            }
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d1), 0);
            break;
        case 0x5A:
            INST_NAME("CVTSD2SS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                MFVSRLD(x4, VSXREG(d0));
                MTVSRD(VSXREG(d1), x4);
            } else {
                XXLOR(VSXREG(d1), VSXREG(d0), VSXREG(d0));
            }
            // XSCVDPSP rounds DP to SP precision and converts to SP format in word 0
            // (replaces FRSP + XSCVDPSPN combo)
            XSCVDPSP(VSXREG(d1), VSXREG(d1));
            VEXTRACTUW(VRREG(d1), VRREG(d1), 0);
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;
        case 0x5C:
            INST_NAME("SUBSD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            MFVSRLD(x4, VSXREG(v0));
            MTVSRD(VSXREG(d1), x4);
            if (MODREG) {
                MFVSRLD(x4, VSXREG(d0));
                MTVSRD(VSXREG(q0), x4);
            } else {
                XXLOR(VSXREG(q0), VSXREG(d0), VSXREG(d0));
            }
            if (!BOX64ENV(dynarec_fastnan)) {
                // Check if either input is NaN before the operation
                XSCMPUDP(6, VSXREG(d1), VSXREG(q0));  // compare to CR6
            }
            XSSUBDP(VSXREG(d1), VSXREG(d1), VSXREG(q0));
            if (!BOX64ENV(dynarec_fastnan)) {
                // Skip fixup if either input was NaN (pre-existing NaN passes through)
                j64 = GETMARK - dyn->native_size;
                BC(BO_TRUE, BI(CR6, CR_SO), j64);  // branch if unordered (input had NaN)
                // Check if result is NaN
                XSCMPUDP(6, VSXREG(d1), VSXREG(d1));
                j64 = GETMARK - dyn->native_size;
                BC(BO_FALSE, BI(CR6, CR_SO), j64);  // branch if ordered (result not NaN)
                // New NaN created: flip sign bit
                MFVSRD(x4, VSXREG(d1));
                LI(x5, 1);
                SLDI(x5, x5, 63);
                XOR(x4, x4, x5);
                MTVSRD(VSXREG(d1), x4);
                MARK;
            }
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d1), 0);
            break;
        case 0x5D:
            INST_NAME("MINSD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            // Extract Gx x86 low double
            MFVSRLD(x4, VSXREG(v0));
            MTVSRD(VSXREG(d1), x4);
            // Get Ex double
            if (MODREG) {
                MFVSRLD(x4, VSXREG(d0));
                MTVSRD(VSXREG(q0), x4);
            } else {
                XXLOR(VSXREG(q0), VSXREG(d0), VSXREG(d0));
            }
            // x86 MINSD semantics: if either is NaN, return source (Ex); if equal, return source
            XSCMPUDP(0, VSXREG(d1), VSXREG(q0));
            // CR0 after XSCMPUDP: LT if a<b, GT if a>b, EQ if a==b, SO if unordered
            // Branch to MARK if unordered (SO bit set) — use source
            j64 = GETMARK - dyn->native_size;
            BC(BO_TRUE, BI(CR0, CR_SO), j64);
            // Not unordered; if dest < src, keep dest (skip to MARK2)
            j64 = GETMARK2 - dyn->native_size;
            BLT(j64);
            // dest > src: take src
            MARK;
            XXLOR(VSXREG(d1), VSXREG(q0), VSXREG(q0));
            MARK2;
            // Insert result back into v0 x86 low
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d1), 0);
            break;
        case 0x5E:
            INST_NAME("DIVSD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            MFVSRLD(x4, VSXREG(v0));
            MTVSRD(VSXREG(d1), x4);
            if (MODREG) {
                MFVSRLD(x4, VSXREG(d0));
                MTVSRD(VSXREG(q0), x4);
            } else {
                XXLOR(VSXREG(q0), VSXREG(d0), VSXREG(d0));
            }
            if (!BOX64ENV(dynarec_fastnan)) {
                // Check if either input is NaN before the operation
                XSCMPUDP(6, VSXREG(d1), VSXREG(q0));  // compare to CR6
            }
            XSDIVDP(VSXREG(d1), VSXREG(d1), VSXREG(q0));
            if (!BOX64ENV(dynarec_fastnan)) {
                // Skip fixup if either input was NaN (pre-existing NaN passes through)
                j64 = GETMARK - dyn->native_size;
                BC(BO_TRUE, BI(CR6, CR_SO), j64);  // branch if unordered (input had NaN)
                // Check if result is NaN
                XSCMPUDP(6, VSXREG(d1), VSXREG(d1));
                j64 = GETMARK - dyn->native_size;
                BC(BO_FALSE, BI(CR6, CR_SO), j64);  // branch if ordered (result not NaN)
                // New NaN created: flip sign bit
                MFVSRD(x4, VSXREG(d1));
                LI(x5, 1);
                SLDI(x5, x5, 63);
                XOR(x4, x4, x5);
                MTVSRD(VSXREG(d1), x4);
                MARK;
            }
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d1), 0);
            break;
        case 0x5F:
            INST_NAME("MAXSD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            // Extract Gx x86 low double
            MFVSRLD(x4, VSXREG(v0));
            MTVSRD(VSXREG(d1), x4);
            // Get Ex double
            if (MODREG) {
                MFVSRLD(x4, VSXREG(d0));
                MTVSRD(VSXREG(q0), x4);
            } else {
                XXLOR(VSXREG(q0), VSXREG(d0), VSXREG(d0));
            }
            // x86 MAXSD semantics: if either is NaN, return source (Ex); if equal, return source
            XSCMPUDP(0, VSXREG(d1), VSXREG(q0));
            // Branch to MARK if unordered (SO bit set) — use source
            j64 = GETMARK - dyn->native_size;
            BC(BO_TRUE, BI(CR0, CR_SO), j64);
            // Not unordered; if dest > src, keep dest (skip to MARK2)
            j64 = GETMARK2 - dyn->native_size;
            BGT(j64);
            // dest < src: take src
            MARK;
            XXLOR(VSXREG(d1), VSXREG(q0), VSXREG(q0));
            MARK2;
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d1), 0);
            break;
        case 0x70:
            INST_NAME("PSHUFLW Gx, Ex, Ib");
            nextop = F8;
            GETEX(v1, 0, 1);
            GETGX(v0, 1);
            u8 = F8;
            {
                // Shuffle low 4 words (64 bits), keep high 64 bits
                // Each 2-bit field in u8 selects which of the 4 source words goes to that position
                // PPC64LE LE word layout: word0=byte0-1, word1=byte2-3, word2=byte4-5, word3=byte6-7
                // Extract low 64 bits as 4 halfwords, shuffle, recombine
                d0 = fpu_get_scratch(dyn);
                MFVSRLD(x4, VSXREG(v1));  // x86 low 64 bits (ISA dw1)

                // Build result in x5 by selecting halfwords
                // word i comes from word (u8 >> (i*2)) & 3
                int src_word;
                // Word 0 (bits 0-15)
                src_word = (u8 >> 0) & 3;
                if (src_word == 0) {
                    ANDI(x5, x4, 0xFFFF);  // actually PPC can't do this with ANDI...
                } else {
                    SRDI(x5, x4, src_word * 16);
                    ANDI(x5, x5, 0xFFFF);  // PPC ANDI uses 16-bit immediate
                }
                // Wait — PPC andi. has full 16-bit unsigned immediate, so 0xFFFF is fine for andi.

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
        case 0x7C:
            INST_NAME("HADDPS Gx, Ex");
            nextop = F8;
            DEFAULT;
            break;
        case 0x7D:
            INST_NAME("HSUBPS Gx, Ex");
            nextop = F8;
            DEFAULT;
            break;
        case 0xC2:
            INST_NAME("CMPSD Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(d0, 0, 1);
            u8 = F8;
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            // Extract Gx scalar double (x86 low = ISA dw1)
            MFVSRLD(x4, VSXREG(v0));
            MTVSRD(VSXREG(d1), x4);
            // Get Ex double
            if (MODREG) {
                MFVSRLD(x4, VSXREG(d0));
                MTVSRD(VSXREG(q0), x4);
            } else {
                XXLOR(VSXREG(q0), VSXREG(d0), VSXREG(d0));
            }
            // Compare and set result to all-ones or all-zeros
            XSCMPUDP(0, VSXREG(d1), VSXREG(q0));
            // Read CR0 bits: LT=0, GT=1, EQ=2, SO/UN=3
            switch (u8 & 7) {
                case 0: // EQ
                    MFOCRF(x5, 0x80);  // CR0 to x5
                    RLWINM(x5, x5, 3, 31, 31);  // extract EQ bit (bit 2 of CR0)
                    break;
                case 1: // LT
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 1, 31, 31);  // extract LT bit (bit 0 of CR0)
                    break;
                case 2: // LE
                    MFOCRF(x5, 0x80);
                    RLWINM(x4, x5, 1, 31, 31);  // LT
                    RLWINM(x5, x5, 3, 31, 31);  // EQ
                    OR(x5, x5, x4);
                    break;
                case 3: // UNORD (NaN)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 4, 31, 31);  // extract SO/UN bit (bit 3 of CR0)
                    break;
                case 4: // NEQ
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 3, 31, 31);  // EQ bit
                    XORI(x5, x5, 1);              // invert
                    break;
                case 5: // NLT (not less than = greater or equal or unordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 1, 31, 31);  // LT bit
                    XORI(x5, x5, 1);              // invert
                    break;
                case 6: // NLE (not less or equal = greater or unordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x4, x5, 1, 31, 31);  // LT
                    RLWINM(x5, x5, 3, 31, 31);  // EQ
                    OR(x5, x5, x4);               // LE
                    XORI(x5, x5, 1);              // invert
                    break;
                case 7: // ORD (not NaN)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 4, 31, 31);  // SO/UN bit
                    XORI(x5, x5, 1);              // invert
                    break;
            }
            // x5 is 0 or 1; need to expand to 0 or 0xFFFFFFFFFFFFFFFF
            NEG(x5, x5);  // 0->0, 1->-1 (0xFFFF...)
            // Insert into low 64 bits of v0 (keep v0 high, put result in low)
            MTVSRD(VSXREG(d1), x5);
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d1), 0);
            break;
        case 0xD0:
            INST_NAME("ADDSUBPS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // Result[0] = Gx[0]-Ex[0], Result[1] = Gx[1]+Ex[1]
            // Result[2] = Gx[2]-Ex[2], Result[3] = Gx[3]+Ex[3]
            // Alternating sub/add on float lanes
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            // Create sign mask: -0.0 for even elements, 0.0 for odd
            // float -0.0 = 0x80000000; LE word layout: [word0, word1, word2, word3]
            // Need [0x80000000, 0, 0x80000000, 0] to flip sign of even lanes (0, 2)
            // 0x00000000_80000000 as 64-bit has word0=0x80000000, word1=0
            LI(x4, 1);
            SLDI(x4, x4, 31);  // x4 = 0x00000000_80000000
            MTVSRDD(VSXREG(d0), x4, x4);  // both halves = 0x00000000_80000000
            // XOR Ex with sign mask to flip sign of even float lanes.
            // NaN care: XOR must NOT flip sign of NaN inputs, since x86 SUB propagates
            // input NaN sign unchanged. Only apply sign flip to non-NaN lanes of Ex.
            if (!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn);
                // Mask sign flip to non-NaN lanes: XOR only where Ex is ordered
                XVCMPEQSP(VSXREG(q0), VSXREG(v1), VSXREG(v1));  // q0 = -1 where Ex NOT NaN
                XXLAND(VSXREG(d0), VSXREG(d0), VSXREG(q0));      // zero sign flip for NaN lanes
                XVCMPEQSP(VSXREG(d1), VSXREG(v0), VSXREG(v0));  // d1 = -1 where Gx NOT NaN
                XXLAND(VSXREG(d1), VSXREG(d1), VSXREG(q0));      // d1 = both ordered mask
            }
            XXLXOR(VSXREG(d0), VSXREG(v1), VSXREG(d0));
            // Add: Gx + modified_Ex gives sub for even, add for odd
            XVADDSP(VSXREG(v0), VSXREG(v0), VSXREG(d0));
            if (!BOX64ENV(dynarec_fastnan)) {
                // Newly generated NaNs (both inputs ordered, result NaN) — set sign bit
                // This handles INF-INF and INF+(-INF) producing negative indefinite NaN.
                XVCMPEQSP(VSXREG(d0), VSXREG(v0), VSXREG(v0));  // d0 = -1 where result NOT NaN
                XXLANDC(VSXREG(d0), VSXREG(d1), VSXREG(d0));     // both-ordered AND result-NaN
                XXSPLTIB(VSXREG(d1), 31);
                VSLW(VRREG(d0), VRREG(d0), VRREG(d1));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(d0));      // OR sign bit
            }
            break;
        case 0xD6:
            INST_NAME("MOVDQ2Q Gm, Ex");
            nextop = F8;
            GETGM(v0);
            v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
            // Copy low 64 bits of SSE to MMX (x86 low = ISA dw1)
            MFVSRLD(x4, VSXREG(v1));
            MTVSRDD(VSXREG(v0), xZR, x4);  // store in MMX (low dword)
            break;
        case 0xE6:
            INST_NAME("CVTPD2DQ Gx, Ex");
            nextop = F8;
            GETEX(v1, 0, 0);
            GETGX_empty(v0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            MTFSB0(23);  // clear VXCVI (always needed on PPC for overflow detection)
            u8 = sse_setround(dyn, ninst, x4, x5);
            // Extract and convert each double using FPR-space FCTIW
            // Note: u8 returns x5 as saved FPSCR, so avoid clobbering x5 before restoreround
            MFVSRLD(x4, VSXREG(v1));    // double0 (x86 low = ISA dw1)
            MTVSRD(d0, x4);              // FPR space
            FCTIW(d0, d0);               // round using FPSCR
            MFVSRWZ(x4, d0);
            MFVSRD(x6, VSXREG(v1));     // double1 (x86 high = ISA dw0)
            MTVSRD(d1, x6);
            FCTIW(d1, d1);
            MFVSRWZ(x6, d1);
            // Check VXCVI BEFORE restoreround (restoreround clears VXCVI)
            MFFS(SCRATCH0);
            STFD(SCRATCH0, -8, xSP);
            LD(x7, -8, xSP);
            RLWINM(x7, x7, 24, 31, 31);
            x87_restoreround(dyn, ninst, u8);
            CMPLDI(x7, 0);
            BEQZ_MARK(x7);
            // Substitute 0x80000000 for overflow
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
            break;
        case 0xF0:
            INST_NAME("LDDQU Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DQ_DISP, 0);
                LXV(VSXREG(v0), fixedaddress, ed);
            }
            break;
        default:
            DEFAULT;
    }

    return addr;
}
