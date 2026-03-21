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
#include "bitutils.h"

#include "ppc64le_printer.h"
#include "dynarec_ppc64le_private.h"
#include "../dynarec_helper.h"
#include "dynarec_ppc64le_functions.h"


uintptr_t dynarec64_660F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2, gback;
    uint8_t eb1, eb2;
    uint8_t gb1, gb2;
    int32_t i32;
    int64_t j64;
    int v0, v1, v2;
    int q0, q1, q2;
    int d0, d1, d2;
    uint8_t tmp1, tmp2, tmp3;
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
    MAYUSE(u8);
    MAYUSE(gdoffset);
    MAYUSE(tmp1);
    MAYUSE(tmp2);
    MAYUSE(tmp3);

    switch (opcode) {
        case 0x10:
            INST_NAME("MOVUPD Gx, Ex");
            nextop = F8;
            GETG;
            v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DQ_DISP, 0);
                LXV(VSXREG(v0), fixedaddress, ed);
            }
            break;
        case 0x11:
            INST_NAME("MOVUPD Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if (MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop & 7) + (rex.b << 3));
                XXLOR(VSXREG(v1), VSXREG(v0), VSXREG(v0));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DQ_DISP, 0);
                STXV(VSXREG(v0), fixedaddress, ed);
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("MOVLPD Gx, Eq");
            nextop = F8;
            GETGX(v0, 1);
            if (MODREG) {
                // MOVLPD with register operand is actually MOVHLPS (only with 66 prefix this doesn't exist in practice)
                // but handle it anyway — this shouldn't happen with 66 prefix
                DEFAULT;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                // Load 8 bytes into low 64 bits of Gx, keep high 64 bits
                LD(x4, fixedaddress, ed);
                // Extract high 64 bits of current Gx (x86 high = ISA dw0)
                MFVSRD(x5, VSXREG(v0));
                // Rebuild: low = loaded data, high = original high
                // On PPC64LE: MTVSRDD(XT, RA, RB) => elem0 (low) = RB, elem1 (high) = RA
                MTVSRDD(VSXREG(v0), x5, x4);
            }
            break;
        case 0x13:
            INST_NAME("MOVLPD Eq, Gx");
            nextop = F8;
            GETGX(v0, 0);
            if (MODREG) {
                DEFAULT;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                // Store low 64 bits of Gx to memory (x86 low = ISA dw1)
                MFVSRLD(x4, VSXREG(v0));
                STD(x4, fixedaddress, ed);
                SMWRITE2();
            }
            break;
        case 0x14:
            INST_NAME("UNPCKLPD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // Result: Gx[63:0] = Gx[63:0], Gx[127:64] = Ex[63:0]
            // Extract low 64 bits of both (x86 low = ISA dw1)
            MFVSRLD(x4, VSXREG(v0));    // v0 x86 low
            MFVSRLD(x5, VSXREG(v1));    // v1 x86 low
            // Rebuild: low = v0_low, high = v1_low
            MTVSRDD(VSXREG(v0), x5, x4);
            break;
        case 0x15:
            INST_NAME("UNPCKHPD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // Result: Gx[63:0] = Gx[127:64], Gx[127:64] = Ex[127:64]
            MFVSRD(x4, VSXREG(v0));   // v0 x86 high (ISA dw0)
            MFVSRD(x5, VSXREG(v1));   // v1 x86 high (ISA dw0)
            // Rebuild: low = v0_high, high = v1_high
            MTVSRDD(VSXREG(v0), x5, x4);
            break;
        case 0x16:
            INST_NAME("MOVHPD Gx, Eq");
            nextop = F8;
            GETGX(v0, 1);
            if (MODREG) {
                DEFAULT;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                // Load 8 bytes into high 64 bits of Gx, keep low 64 bits
                LD(x5, fixedaddress, ed);
                // Get current low 64 bits (x86 low = ISA dw1)
                MFVSRLD(x4, VSXREG(v0));
                // Rebuild: low = original low, high = loaded data
                MTVSRDD(VSXREG(v0), x5, x4);
            }
            break;
        case 0x17:
            INST_NAME("MOVHPD Eq, Gx");
            nextop = F8;
            GETGX(v0, 0);
            if (MODREG) {
                DEFAULT;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                // Store high 64 bits of Gx to memory (x86 high = ISA dw0)
                MFVSRD(x4, VSXREG(v0));
                STD(x4, fixedaddress, ed);
                SMWRITE2();
            }
            break;
        case 0x18:
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
        case 0x1F:
            INST_NAME("NOP (multibyte)");
            nextop = F8;
            FAKEED;
            break;
        case 0x28:
            INST_NAME("MOVAPD Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg(dyn, ninst, x1, ed, 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                SMREAD();
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DQ_DISP, 0);
                LXV(VSXREG(v0), fixedaddress, ed);
            }
            break;
        case 0x29:
            INST_NAME("MOVAPD Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                XXLOR(VSXREG(v1), VSXREG(v0), VSXREG(v0));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DQ_DISP, 0);
                STXV(VSXREG(v0), fixedaddress, ed);
                SMWRITE2();
            }
            break;
        case 0x2B:
            INST_NAME("MOVNTPD Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                XXLOR(VSXREG(v1), VSXREG(v0), VSXREG(v0));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DQ_DISP, 0);
                STXV(VSXREG(v0), fixedaddress, ed);
                SMWRITE2();
            }
            break;
         case 0x2A:
            INST_NAME("CVTPI2PD Gx, Em");
            nextop = F8;
            GETGX(v0, 1);
            GETEM(v1, 0);
            // Convert 2 signed int32 to 2 doubles
            // v1 has the MMX register (low 64 bits contain two int32 values)
            // On PPC64LE:
            // 1. Extract the two int32 values
            // 2. Convert each to double
            // 3. Pack into v0
            MFVSRLD(x4, VSXREG(v1));       // MMX low 64 bits: [int1:int0]
            EXTSW(x5, x4);                  // sign-extend int0 (low 32 bits)
            SRADI(x6, x4, 32);              // int1 = high 32 bits, sign-extended
            // Convert int0 to double
            MTVSRD(VSXREG(v0), x5);
            XSCVSXDDP(VSXREG(v0), VSXREG(v0));
            // Convert int1 to double, put in scratch
            q0 = fpu_get_scratch(dyn);
            MTVSRD(VSXREG(q0), x6);
            XSCVSXDDP(VSXREG(q0), VSXREG(q0));
            // Now v0 has double0 in low 64, q0 has double1 in low 64
            // Combine: v0_low = double0, v0_high = double1
            MFVSRD(x4, VSXREG(v0));
            MFVSRD(x5, VSXREG(q0));
            MTVSRDD(VSXREG(v0), x5, x4);
            break;
        case 0x2C:
            INST_NAME("CVTTPD2PI Gm, Ex");
            nextop = F8;
            GETGM(v0);
            GETEX(v1, 0, 0);
            q0 = fpu_get_scratch(dyn);
            MTFSB0(23);  // clear VXCVI (always needed on PPC for overflow detection)
            // Convert 2 doubles to 2 int32 with truncation, store in MMX register
            MFVSRLD(x4, VSXREG(v1));         // double0 (x86 low = ISA dw1)
            MFVSRD(x5, VSXREG(v1));          // double1 (x86 high = ISA dw0)
            // Convert double0 via FPR-space FCTIWZ
            MTVSRD(v0, x4);                  // raw index → FPR space
            FCTIWZ(v0, v0);
            MFVSRWZ(x6, v0);
            // Convert double1
            MTVSRD(q0, x5);
            FCTIWZ(q0, q0);
            MFVSRWZ(x7, q0);
            // Check VXCVI: PPC gives 0x7FFFFFFF for positive overflow, x86 wants 0x80000000
            MFFS(SCRATCH0);
            STFD(SCRATCH0, -8, xSP);
            LD(x4, -8, xSP);
            RLWINM(x4, x4, 24, 31, 31);
            CMPLDI(x4, 0);
            BEQZ_MARK(x4);
            // If overflow, substitute 0x80000000 for both
            LI(x6, 0);
            ORI(x6, x6, 0x8000);
            SLDI(x6, x6, 16);    // x6 = 0x80000000
            MR(x7, x6);
            MARK;
            // Combine: low 32 = int0, high 32 = int1
            CLRLDI(x6, x6, 32);
            SLDI(x7, x7, 32);
            OR(x6, x6, x7);
            MTVSRDD(VSXREG(v0), xZR, x6);  // store combined result in MMX (low dword)
            break;
        case 0x2D:
            INST_NAME("CVTPD2PI Gm, Ex");
            nextop = F8;
            GETGM(v0);
            GETEX(v1, 0, 0);
            q0 = fpu_get_scratch(dyn);
            MTFSB0(23);  // clear VXCVI (always needed on PPC for overflow detection)
            u8 = sse_setround(dyn, ninst, x4, x5);
            // Convert 2 doubles to 2 int32 using FPSCR rounding mode
            // Note: u8 returns x5 as saved FPSCR, so avoid clobbering x5 before restoreround
            MFVSRLD(x4, VSXREG(v1));         // double0
            MTVSRD(v0, x4);
            FCTIW(v0, v0);
            MFVSRWZ(x4, v0);
            MFVSRD(x6, VSXREG(v1));          // double1
            MTVSRD(q0, x6);
            FCTIW(q0, q0);
            MFVSRWZ(x6, q0);
            // Check VXCVI BEFORE restoreround (restoreround clears VXCVI)
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
            // Combine
            CLRLDI(x4, x4, 32);
            SLDI(x6, x6, 32);
            OR(x4, x4, x6);
            MTVSRDD(VSXREG(v0), xZR, x4);  // store combined result in MMX (low dword)
            break;
        case 0x2E:
            // no special check...
        case 0x2F:
            if (opcode == 0x2F) {
                INST_NAME("COMISD Gx, Ex");
            } else {
                INST_NAME("UCOMISD Gx, Ex");
            }
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETGX(v0, 0);
            // v0 is SSE VR: x86 scalar is in ISA dw1, but XSCMPUDP reads ISA dw0.
            // Extract x86 low (ISA dw1) into FPR scratch for v0.
            d0 = fpu_get_scratch(dyn);
            MFVSRLD(x4, VSXREG(v0));
            MTVSRD(VSXREG(d0), x4);
            // For v1: if MODREG, it's also an SSE VR (scalar in ISA dw1); if memory, LFD puts it in ISA dw0 correctly.
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                d1 = fpu_get_scratch(dyn);
                MFVSRLD(x4, VSXREG(v1));
                MTVSRD(VSXREG(d1), x4);
            } else {
                SMREAD();
                d1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, DS_DISP, 0);
                LD(x4, fixedaddress, ed);
                MTVSRD(VSXREG(d1), x4);
            }
            CLEAR_FLAGS(x1);
            // Compare scalar double: Gx[63:0] vs Ex[63:0]
            // XSCMPUDP(BF, XA, XB): compares XA vs XB in ISA dw0, sets CR field BF
            XSCMPUDP(0, VSXREG(d0), VSXREG(d1));
            // Map PPC CR0 to x86 flags:
            // Unordered (NaN): ZF=1, PF=1, CF=1
            // Equal:           ZF=1, PF=0, CF=0
            // Less than:       ZF=0, PF=0, CF=1
            // Greater than:    ZF=0, PF=0, CF=0
            MFCR(x1);
            // Extract CR0 field: bits [31:28] → low 4 bits: [LT, GT, EQ, UN]
            RLWINM(x1, x1, 4, 28, 31);
            // Now x1 has: bit3=LT, bit2=GT, bit1=EQ, bit0=UN
            // CF (F_CF=0): set if LT or UN => bit3|bit0
            RLWINM(x2, x1, 0, 31, 31);   // x2 = UN bit (bit 0)
            RLWINM(x3, x1, 29, 31, 31);  // x3 = LT bit (bit 3 >> 3 => bit 0)
            OR(x2, x2, x3);               // x2 = CF = LT|UN
            // PF (F_PF=2) = UN
            RLWINM(x3, x1, 0, 31, 31);   // x3 = UN = PF
            SLWI(x3, x3, F_PF);           // shift to PF position
            OR(x2, x2, x3);               // x2 |= PF<<F_PF
            // ZF (F_ZF=6) = EQ|UN
            RLWINM(x3, x1, 31, 31, 31);  // x3 = EQ bit (bit 1 >> 1 => bit 0)
            RLWINM(x4, x1, 0, 31, 31);   // x4 = UN
            OR(x3, x3, x4);               // x3 = ZF = EQ|UN
            SLWI(x3, x3, F_ZF);           // shift to ZF position
            OR(x2, x2, x3);               // x2 |= ZF<<F_ZF
            // Write to xFlags
            OR(xFlags, xFlags, x2);
            break;

#define GO(GETFLAGS, NO, YES, NATNO, NATYES, F, I)                                          \
    READFLAGS_FUSION(F, x1, x2, x3, x4, x5);                                               \
    if (!dyn->insts[ninst].nat_flags_fusion) {                                               \
        GETFLAGS;                                                                            \
    }                                                                                        \
    nextop = F8;                                                                             \
    GETGD;                                                                                   \
    if (MODREG) {                                                                            \
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                            \
        if (dyn->insts[ninst].nat_flags_fusion) {                                            \
            NATIVEJUMP(NATNO, 12);                                                           \
        } else {                                                                             \
            B##NO##_MARK2(tmp1);                                                             \
        }                                                                                    \
        BF_INSERT(gd, ed, 15, 0);                                                           \
        MARK2;                                                                               \
    } else {                                                                                 \
        SMREAD();                                                                            \
        addr = geted(dyn, addr, ninst, nextop, &ed, tmp2, tmp3, &fixedaddress, rex, NULL, DS_DISP, 0); \
        if (dyn->insts[ninst].nat_flags_fusion) {                                            \
            NATIVEJUMP(NATNO, 16);                                                           \
        } else {                                                                             \
            B##NO##_MARK2(tmp1);                                                             \
        }                                                                                    \
        LHZ(x1, fixedaddress, ed);                                                          \
        BF_INSERT(gd, x1, 15, 0);                                                           \
        MARK2;                                                                               \
    }

            GOCOND(0x40, "CMOV", "Gw, Ew");
#undef GO

        case 0x50:
            INST_NAME("MOVMSKPD Gd, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGD;
            // Extract sign bits of both doubles
            MFVSRLD(x4, VSXREG(q0));    // x86 low 64 bits (double 0, ISA dw1)
            MFVSRD(x5, VSXREG(q0));     // x86 high 64 bits (double 1, ISA dw0)
            SRDI(x4, x4, 63);            // sign bit of double 0 -> bit 0
            SRDI(x5, x5, 63);            // sign bit of double 1 -> bit 0
            SLDI(x5, x5, 1);             // shift to bit 1
            OR(gd, x4, x5);
            break;

        case 0x51:
            INST_NAME("SQRTPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX_empty(v0);
            if (!BOX64ENV(dynarec_fastnan)) {
                q1 = fpu_get_scratch(dyn);
                d0 = fpu_get_scratch(dyn);
                XVCMPEQDP(VSXREG(q1), VSXREG(q0), VSXREG(q0));  // q1 = -1 where input is NOT NaN
            }
            XVSQRTDP(VSXREG(v0), VSXREG(q0));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQDP(VSXREG(d0), VSXREG(v0), VSXREG(v0));  // d0 = -1 where result is NOT NaN
                XXLANDC(VSXREG(d0), VSXREG(q1), VSXREG(d0));    // d0 = input ordered AND result NaN (new NaNs)
                XXSPLTIB(VSXREG(q1), 63);
                VSLD(VRREG(d0), VRREG(d0), VRREG(q1));           // d0 = 0x8000... in new NaN lanes
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(d0));      // OR sign bit into result
            }
            break;

        case 0x54:
            INST_NAME("ANDPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            break;
        case 0x55:
            INST_NAME("ANDNPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            // x86: dest = NOT(dest) AND src
            // PPC XXLANDC(T, A, B) = A AND NOT(B)
            // So: XXLANDC(v0, q0, v0) = q0 AND NOT(v0) = src AND NOT(dest) ✓
            XXLANDC(VSXREG(v0), VSXREG(q0), VSXREG(v0));
            break;
        case 0x56:
            INST_NAME("ORPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            break;
        case 0x57:
            INST_NAME("XORPD Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG && ((nextop & 7) + (rex.b << 3) == gd)) {
                // special case for XORPD Gx, Gx => zero
                q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
            } else {
                q0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                GETEX(q1, 0, 0);
                XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q1));
            }
            break;

        case 0x58:
            INST_NAME("ADDPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            if (!BOX64ENV(dynarec_fastnan)) {
                q1 = fpu_get_scratch(dyn);
                d0 = fpu_get_scratch(dyn);
                XVCMPEQDP(VSXREG(q1), VSXREG(v0), VSXREG(v0));  // q1 = -1 where Gx not NaN
                XVCMPEQDP(VSXREG(d0), VSXREG(q0), VSXREG(q0));  // d0 = -1 where Ex not NaN
                XXLAND(VSXREG(q1), VSXREG(q1), VSXREG(d0));      // q1 = -1 where BOTH not NaN
            }
            XVADDDP(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQDP(VSXREG(d0), VSXREG(v0), VSXREG(v0));  // d0 = -1 where result not NaN
                XXLANDC(VSXREG(d0), VSXREG(q1), VSXREG(d0));    // d0 = both-ordered AND result-NaN (new NaN)
                XXSPLTIB(VSXREG(q1), 63);
                VSLD(VRREG(d0), VRREG(d0), VRREG(q1));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(d0));
            }
            break;
        case 0x59:
            INST_NAME("MULPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            if (!BOX64ENV(dynarec_fastnan)) {
                q1 = fpu_get_scratch(dyn);
                d0 = fpu_get_scratch(dyn);
                XVCMPEQDP(VSXREG(q1), VSXREG(v0), VSXREG(v0));
                XVCMPEQDP(VSXREG(d0), VSXREG(q0), VSXREG(q0));
                XXLAND(VSXREG(q1), VSXREG(q1), VSXREG(d0));
            }
            XVMULDP(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQDP(VSXREG(d0), VSXREG(v0), VSXREG(v0));
                XXLANDC(VSXREG(d0), VSXREG(q1), VSXREG(d0));
                XXSPLTIB(VSXREG(q1), 63);
                VSLD(VRREG(d0), VRREG(d0), VRREG(q1));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(d0));
            }
            break;
        case 0x5A:
            INST_NAME("CVTPD2PS Gx, Ex");
            nextop = F8;
            GETEX(v1, 0, 0);
            GETGX_empty(v0);
            // Convert 2 doubles to 2 singles, zero upper 64 bits
            // XVCVDPSP converts 2 doubles to 2 singles, but places results in odd word positions
            // (words 1 and 3 in BE, which on LE means words 0 and 2)
            // Then we need to pack them into the low 64 bits
            q0 = fpu_get_scratch(dyn);
            // Convert each double to single via GPR
            MFVSRLD(x4, VSXREG(v1));   // x86 low double (ISA dw1)
            MTVSRD(VSXREG(q0), x4);
            XSCVDPSP(VSXREG(q0), VSXREG(q0));
            MFVSRWZ(x4, VSXREG(q0));   // single0 in low 32 bits of GPR
            MFVSRD(x5, VSXREG(v1));    // x86 high double (ISA dw0)
            MTVSRD(VSXREG(q0), x5);
            XSCVDPSP(VSXREG(q0), VSXREG(q0));
            MFVSRWZ(x5, VSXREG(q0));   // single1 in low 32 bits of GPR
            // Pack: low32 = single0, next32 = single1, upper 64 = 0
            RLWINM(x4, x4, 0, 0, 31);  // zero-extend
            SLDI(x5, x5, 32);
            OR(x4, x4, x5);
            MTVSRDD(VSXREG(v0), 0, x4);
            break;
        case 0x5B:
            INST_NAME("CVTPS2DQ Gx, Ex");
            nextop = F8;
            GETEX(v1, 0, 0);
            GETGX_empty(v0);
            u8 = sse_setround(dyn, ninst, x4, x5);
            // Round floats to integer using FPSCR rounding mode, then truncate
            XVRSPIC(VSXREG(v0), VSXREG(v1));   // round to integer floats using RN
            XVCVSPSXWS(VSXREG(v0), VSXREG(v0)); // truncate to int32 (exact after rounding)
            x87_restoreround(dyn, ninst, u8);
            // Fix positive overflow: PPC gives 0x7FFFFFFF, x86 wants 0x80000000
            // Also handle NaN: PPC gives 0x80000000 (already correct)
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            // Create 0x7FFFFFFF vector: splat -1 (0xFFFFFFFF), shift right by 1
            VSPLTISW(VRREG(q0), -1);                   // q0 = 0xFFFFFFFF per lane
            VSPLTISW(VRREG(q1), 1);
            VSRW(VRREG(q0), VRREG(q0), VRREG(q1));     // q0 = 0x7FFFFFFF
            VCMPEQUW(VRREG(q0), VRREG(v0), VRREG(q0)); // mask where result == 0x7FFFFFFF
            VSUBUWM(VRREG(v0), VRREG(v0), VRREG(q0));  // 0x7FFFFFFF - (-1) = 0x80000000
            break;
        case 0x5C:
            INST_NAME("SUBPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            if (!BOX64ENV(dynarec_fastnan)) {
                q1 = fpu_get_scratch(dyn);
                d0 = fpu_get_scratch(dyn);
                XVCMPEQDP(VSXREG(q1), VSXREG(v0), VSXREG(v0));
                XVCMPEQDP(VSXREG(d0), VSXREG(q0), VSXREG(q0));
                XXLAND(VSXREG(q1), VSXREG(q1), VSXREG(d0));
            }
            XVSUBDP(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQDP(VSXREG(d0), VSXREG(v0), VSXREG(v0));
                XXLANDC(VSXREG(d0), VSXREG(q1), VSXREG(d0));
                XXSPLTIB(VSXREG(q1), 63);
                VSLD(VRREG(d0), VRREG(d0), VRREG(q1));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(d0));
            }
            break;
        case 0x5D:
            INST_NAME("MINPD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // x86 MINPD: return min, but if either is NaN or both equal, return src2 (Ex)
            // XVCMPGTDP(q0, v1, v0): q0 = (Ex > Gx) ? -1 : 0
            //   Ex > Gx:  mask=-1, XXSEL picks B=v0(Gx) — correct (Gx is smaller)
            //   Ex <= Gx: mask=0,  XXSEL picks A=v1(Ex) — correct (Ex is smaller or equal)
            //   NaN:      mask=0,  XXSEL picks A=v1(Ex) — correct (x86 returns src2)
            q0 = fpu_get_scratch(dyn);
            XVCMPGTDP(VSXREG(q0), VSXREG(v1), VSXREG(v0));
            XXSEL(VSXREG(v0), VSXREG(v1), VSXREG(v0), VSXREG(q0));
            break;
        case 0x5E:
            INST_NAME("DIVPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            if (!BOX64ENV(dynarec_fastnan)) {
                q1 = fpu_get_scratch(dyn);
                d0 = fpu_get_scratch(dyn);
                XVCMPEQDP(VSXREG(q1), VSXREG(v0), VSXREG(v0));
                XVCMPEQDP(VSXREG(d0), VSXREG(q0), VSXREG(q0));
                XXLAND(VSXREG(q1), VSXREG(q1), VSXREG(d0));
            }
            XVDIVDP(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQDP(VSXREG(d0), VSXREG(v0), VSXREG(v0));
                XXLANDC(VSXREG(d0), VSXREG(q1), VSXREG(d0));
                XXSPLTIB(VSXREG(q1), 63);
                VSLD(VRREG(d0), VRREG(d0), VRREG(q1));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(d0));
            }
            break;
        case 0x5F:
            INST_NAME("MAXPD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // x86 MAXPD: if either is NaN, return src2 (Ex)
            // XVCMPGTDP(mask, v0, v1) => mask = (v0 > v1) ? -1 : 0
            // If NaN or v0 <= v1: mask = 0, XXSEL picks v1 (Ex) — correct for NaN case and v1 >= v0
            // If v0 > v1: mask = all-ones, XXSEL picks v0 (the larger)
            q0 = fpu_get_scratch(dyn);
            XVCMPGTDP(VSXREG(q0), VSXREG(v0), VSXREG(v1)); // q0 = (Gx > Ex) ? -1 : 0
            XXSEL(VSXREG(v0), VSXREG(v1), VSXREG(v0), VSXREG(q0));
            break;

        case 0x60:
            INST_NAME("PUNPCKLBW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // Interleave low bytes: result[0]=Gx[0], result[1]=Ex[0], result[2]=Gx[1], result[3]=Ex[1], ...
            // On PPC64LE, VMRGLB merges low bytes of two VRs
            // VMRGLB(VRT, VRA, VRB): interleaves low 8 bytes
            // PPC64LE byte order: low bytes are elements 0-7
            // x86 PUNPCKLBW: interleaves bytes 0-7 of Gx and Ex
            VMRGLB(VRREG(v0), VRREG(v1), VRREG(v0));
            break;
        case 0x61:
            INST_NAME("PUNPCKLWD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VMRGLH(VRREG(v0), VRREG(v1), VRREG(v0));
            break;
        case 0x62:
            INST_NAME("PUNPCKLDQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VMRGLW(VRREG(v0), VRREG(v1), VRREG(v0));
            break;

        case 0x63:
            INST_NAME("PACKSSWB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // VPKSHSS: pack signed halfwords to signed bytes with saturation
            // On PPC64LE, VPKSHSS(VRT, VRA, VRB) interleaves results from VRA and VRB
            // PPC packs: VRA provides high elements, VRB provides low elements (in BE sense)
            // On LE, the order is swapped: VRB's elements go to low positions
            // x86 PACKSSWB: low bytes from Gx, high bytes from Ex
            // So: VPKSHSS(v0, v1, v0) — v0 (src=Gx) low, v1 (src=Ex) high
            if (v0 == v1) {
                VPKSHSS(VRREG(v0), VRREG(v0), VRREG(v0));
            } else {
                VPKSHSS(VRREG(v0), VRREG(v1), VRREG(v0));
            }
            break;

        case 0x64:
            INST_NAME("PCMPGTB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // Compare greater-than signed bytes: result = (Gx > Ex) ? 0xFF : 0x00
            VCMPGTSB(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0x65:
            INST_NAME("PCMPGTW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VCMPGTSH(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0x66:
            INST_NAME("PCMPGTD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VCMPGTSW(VRREG(v0), VRREG(v0), VRREG(v1));
            break;

        case 0x67:
            INST_NAME("PACKUSWB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // VPKSHUS: pack signed halfwords to unsigned bytes with saturation
            // (clamps negative to 0, positive to 255)
            if (v0 == v1) {
                VPKSHUS(VRREG(v0), VRREG(v0), VRREG(v0));
            } else {
                VPKSHUS(VRREG(v0), VRREG(v1), VRREG(v0));
            }
            break;

        case 0x68:
            INST_NAME("PUNPCKHBW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VMRGHB(VRREG(v0), VRREG(v1), VRREG(v0));
            break;
        case 0x69:
            INST_NAME("PUNPCKHWD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VMRGHH(VRREG(v0), VRREG(v1), VRREG(v0));
            break;
        case 0x6A:
            INST_NAME("PUNPCKHDQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VMRGHW(VRREG(v0), VRREG(v1), VRREG(v0));
            break;

        case 0x6B:
            INST_NAME("PACKSSDW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // VPKSWSS: pack signed words to signed halfwords with saturation
            if (v0 == v1) {
                VPKSWSS(VRREG(v0), VRREG(v0), VRREG(v0));
            } else {
                VPKSWSS(VRREG(v0), VRREG(v1), VRREG(v0));
            }
            break;

        case 0x6C:
            INST_NAME("PUNPCKLQDQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // Interleave low qwords: result[63:0]=Gx[63:0], result[127:64]=Ex[63:0]
            MFVSRLD(x4, VSXREG(v0));    // v0 x86 low qword (ISA dw1)
            MFVSRLD(x5, VSXREG(v1));    // v1 x86 low qword (ISA dw1)
            MTVSRDD(VSXREG(v0), x5, x4);
            break;
        case 0x6D:
            INST_NAME("PUNPCKHQDQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // Interleave high qwords: result[63:0]=Gx[127:64], result[127:64]=Ex[127:64]
            MFVSRD(x4, VSXREG(v0));   // v0 x86 high qword (ISA dw0)
            MFVSRD(x5, VSXREG(v1));   // v1 x86 high qword (ISA dw0)
            MTVSRDD(VSXREG(v0), x5, x4);
            break;

        case 0x6E:
            INST_NAME("MOVD Gx, Ed");
            nextop = F8;
            GETGX_empty(v0);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                if (rex.w) {
                    // MOVQ: XMM[63:0] = r64, XMM[127:64] = 0
                    MTVSRDD(VSXREG(v0), 0, ed);
                } else {
                    // MOVD: XMM[31:0] = r32, XMM[127:32] = 0
                    // Zero-extend r32 to r64, then put in low 64 bits
                    RLWINM(x4, ed, 0, 0, 31);  // zero-extend 32-bit
                    MTVSRDD(VSXREG(v0), 0, x4);
                }
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                if (rex.w) {
                    LD(x4, fixedaddress, ed);
                } else {
                    LWZ(x4, fixedaddress, ed);
                }
                MTVSRDD(VSXREG(v0), 0, x4);
            }
            break;
        case 0x6F:
            INST_NAME("MOVDQA Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                GETGX_empty(v0);
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                GETGX_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DQ_DISP, 0);
                LXV(VSXREG(v0), fixedaddress, ed);
            }
            break;
        case 0x70:
            INST_NAME("PSHUFD Gx, Ex, Ib");
            nextop = F8;
            GETEX(v1, 0, 1);
            GETGX_empty(v0);
            u8 = F8;
            // PSHUFD: dst[31:0] = src[imm[1:0]*32+31 : imm[1:0]*32]
            //         dst[63:32] = src[imm[3:2]*32+31 : imm[3:2]*32]
            //         dst[95:64] = src[imm[5:4]*32+31 : imm[5:4]*32]
            //         dst[127:96] = src[imm[7:6]*32+31 : imm[7:6]*32]
            // On PPC64LE, we can use XXPERMDI for some cases, but general case
            // requires building a permute vector. For now, use GPR intermediary.
            if (u8 == 0x00) {
                // All elements = element 0 (splat x86 dword 0 = LE word 0 = BE word 3 = XXSPLTW UIM 3)
                XXSPLTW(VSXREG(v0), VSXREG(v1), 3);
            } else if (u8 == 0x55) {
                // All elements = element 1 (x86 dword 1 = BE word 2 = XXSPLTW UIM 2)
                XXSPLTW(VSXREG(v0), VSXREG(v1), 2);
            } else if (u8 == 0xAA) {
                // All elements = element 2 (x86 dword 2 = BE word 1 = XXSPLTW UIM 1)
                XXSPLTW(VSXREG(v0), VSXREG(v1), 1);
            } else if (u8 == 0xFF) {
                // All elements = element 3 (x86 dword 3 = BE word 0 = XXSPLTW UIM 0)
                XXSPLTW(VSXREG(v0), VSXREG(v1), 0);
            } else if (u8 == 0x44) {
                // [0,1,0,1] — x86 low 64 bits duplicated = splat ISA dw1 = DM=3
                XXPERMDI(VSXREG(v0), VSXREG(v1), VSXREG(v1), 3);
            } else if (u8 == 0xEE) {
                // [2,3,2,3] — x86 high 64 bits duplicated = splat ISA dw0 = DM=0
                XXPERMDI(VSXREG(v0), VSXREG(v1), VSXREG(v1), 0);
            } else if (u8 == 0x4E) {
                // [2,3,0,1] — swap halves
                XXPERMDI(VSXREG(v0), VSXREG(v1), VSXREG(v1), 2);
            } else if (u8 == 0xE4) {
                // [0,1,2,3] — identity
                if (v0 != v1)
                    XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                // General case: extract all 4 dwords via GPR, then reassemble
                // Element indices on PPC64LE little-endian:
                // In a 128-bit LE register: byte 0-3 = element 0, 4-7 = element 1, 8-11 = element 2, 12-15 = element 3
                // MFVSRLD gets x86 low 64 bits (elements 0,1), MFVSRD gets x86 high 64 bits (elements 2,3)
                MFVSRLD(x4, VSXREG(v1));   // x4 = [elem1:elem0] (x86 low = ISA dw1)
                MFVSRD(x5, VSXREG(v1));    // x5 = [elem3:elem2] (x86 high = ISA dw0)
                // Extract each element based on imm8
                // elem_src[i] = (u8 >> (i*2)) & 3
                // For each of the 4 result positions, pick the right 32-bit element
                {
                    int sel0 = (u8 >> 0) & 3;
                    int sel1 = (u8 >> 2) & 3;
                    int sel2 = (u8 >> 4) & 3;
                    int sel3 = (u8 >> 6) & 3;
                    // Helper: element n is in x4 (n<2) or x5 (n>=2), low 32 bits if n%2==0, high 32 bits if n%2==1
                    // Extract element sel into x6
                    #define EXTRACT_ELEM(dst, sel) do { \
                        if ((sel) < 2) { \
                            if ((sel) == 0) RLWINM(dst, x4, 0, 0, 31); \
                            else SRDI(dst, x4, 32); \
                        } else { \
                            if ((sel) == 2) RLWINM(dst, x5, 0, 0, 31); \
                            else SRDI(dst, x5, 32); \
                        } \
                    } while(0)
                    // Build result low 64 bits: [elem1_result : elem0_result]
                    EXTRACT_ELEM(x6, sel0);  // elem 0
                    EXTRACT_ELEM(x7, sel1);  // elem 1
                    SLDI(x7, x7, 32);
                    OR(x6, x6, x7);          // x6 = low 64 bits result
                    // Build result high 64 bits: [elem3_result : elem2_result]
                    EXTRACT_ELEM(x7, sel2);  // elem 2
                    EXTRACT_ELEM(x3, sel3);  // elem 3
                    SLDI(x3, x3, 32);
                    OR(x7, x7, x3);          // x7 = high 64 bits result
                    MTVSRDD(VSXREG(v0), x7, x6);
                    #undef EXTRACT_ELEM
                }
            }
            break;

        case 0x71:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("PSRLW Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 15) {
                            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
                        } else {
                            q1 = fpu_get_scratch(dyn);
                            XXSPLTIB(VSXREG(q1), u8);
                            VSRH(VRREG(q0), VRREG(q0), VRREG(q1));
                        }
                    }
                    PUTEX(q0);
                    break;
                case 4:
                    INST_NAME("PSRAW Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8 > 15) u8 = 15;
                    if (u8) {
                        q1 = fpu_get_scratch(dyn);
                        XXSPLTIB(VSXREG(q1), u8);
                        VSRAH(VRREG(q0), VRREG(q0), VRREG(q1));
                    }
                    PUTEX(q0);
                    break;
                case 6:
                    INST_NAME("PSLLW Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 15) {
                            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
                        } else {
                            q1 = fpu_get_scratch(dyn);
                            XXSPLTIB(VSXREG(q1), u8);
                            VSLH(VRREG(q0), VRREG(q0), VRREG(q1));
                        }
                    }
                    PUTEX(q0);
                    break;
                default:
                    *ok = 0;
                    DEFAULT;
            }
            break;
        case 0x72:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("PSRLD Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 31) {
                            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
                        } else {
                            q1 = fpu_get_scratch(dyn);
                            XXSPLTIB(VSXREG(q1), u8);
                            VSRW(VRREG(q0), VRREG(q0), VRREG(q1));
                        }
                    }
                    PUTEX(q0);
                    break;
                case 4:
                    INST_NAME("PSRAD Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8 > 31) u8 = 31;
                    if (u8) {
                        q1 = fpu_get_scratch(dyn);
                        XXSPLTIB(VSXREG(q1), u8);
                        VSRAW(VRREG(q0), VRREG(q0), VRREG(q1));
                    }
                    PUTEX(q0);
                    break;
                case 6:
                    INST_NAME("PSLLD Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 31) {
                            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
                        } else {
                            q1 = fpu_get_scratch(dyn);
                            XXSPLTIB(VSXREG(q1), u8);
                            VSLW(VRREG(q0), VRREG(q0), VRREG(q1));
                        }
                    }
                    PUTEX(q0);
                    break;
                default:
                    *ok = 0;
                    DEFAULT;
            }
            break;
        case 0x73:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("PSRLQ Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 63) {
                            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
                        } else {
                            q1 = fpu_get_scratch(dyn);
                            XXSPLTIB(VSXREG(q1), u8);
                            VSRD(VRREG(q0), VRREG(q0), VRREG(q1));
                        }
                    }
                    PUTEX(q0);
                    break;
                case 3:
                    INST_NAME("PSRLDQ Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 15) {
                            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
                        } else {
                            // Byte shift right: shift entire 128-bit register right by u8 bytes
                            // Use VSLDOI: VSLDOI(VRT, VRA, VRB, SH) concatenates VRA:VRB and shifts left by SH bytes
                            // On PPC64LE, VSLDOI byte shift semantic is reversed due to endianness
                            // To shift right by N bytes in LE: VSLDOI(dst, zero, src, 16-N)
                            q1 = fpu_get_scratch(dyn);
                            XXLXOR(VSXREG(q1), VSXREG(q1), VSXREG(q1));
                            VSLDOI(VRREG(q0), VRREG(q1), VRREG(q0), 16 - u8);
                        }
                    }
                    PUTEX(q0);
                    break;
                case 6:
                    INST_NAME("PSLLQ Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 63) {
                            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
                        } else {
                            q1 = fpu_get_scratch(dyn);
                            XXSPLTIB(VSXREG(q1), u8);
                            VSLD(VRREG(q0), VRREG(q0), VRREG(q1));
                        }
                    }
                    PUTEX(q0);
                    break;
                case 7:
                    INST_NAME("PSLLDQ Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 15) {
                            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
                        } else {
                            // Byte shift left: shift entire 128-bit register left by u8 bytes
                            // On PPC64LE, to shift left by N bytes: VSLDOI(dst, src, zero, N)
                            q1 = fpu_get_scratch(dyn);
                            XXLXOR(VSXREG(q1), VSXREG(q1), VSXREG(q1));
                            VSLDOI(VRREG(q0), VRREG(q0), VRREG(q1), u8);
                        }
                    }
                    PUTEX(q0);
                    break;
                default:
                    *ok = 0;
                    DEFAULT;
            }
            break;

        case 0x74:
            INST_NAME("PCMPEQB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VCMPEQUB(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0x75:
            INST_NAME("PCMPEQW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VCMPEQUH(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0x76:
            INST_NAME("PCMPEQD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VCMPEQUW(VRREG(v0), VRREG(v0), VRREG(v1));
            break;

        case 0x7C:
            INST_NAME("HADDPD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // HADDPD: dest[0] = dest[0] + dest[1], dest[1] = src[0] + src[1]
            // x86[0]=low=ISA dw1, x86[1]=high=ISA dw0
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            // q0 = {v1_dw0, v0_dw0}: XXPERMDI(q0, v1, v0, 0b00)
            XXPERMDI(VSXREG(q0), VSXREG(v1), VSXREG(v0), 0b00);
            // q1 = {v1_dw1, v0_dw1}: XXPERMDI(q1, v1, v0, 0b11)
            XXPERMDI(VSXREG(q1), VSXREG(v1), VSXREG(v0), 0b11);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                XVCMPEQDP(VSXREG(d0), VSXREG(q0), VSXREG(q0));
                XVCMPEQDP(VSXREG(d1), VSXREG(q1), VSXREG(q1));
                XXLAND(VSXREG(d0), VSXREG(d0), VSXREG(d1));  // d0 = both inputs ordered
            }
            XVADDDP(VSXREG(v0), VSXREG(q0), VSXREG(q1));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQDP(VSXREG(d1), VSXREG(v0), VSXREG(v0));
                XXLANDC(VSXREG(d1), VSXREG(d0), VSXREG(d1));
                XXSPLTIB(VSXREG(d0), 63);
                VSLD(VRREG(d1), VRREG(d1), VRREG(d0));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(d1));
            }
            break;
        case 0x7D:
            INST_NAME("HSUBPD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // HSUBPD: dest[0] = dest[0] - dest[1], dest[1] = src[0] - src[1]
            // x86[0]=low=ISA dw1, x86[1]=high=ISA dw0
            // result ISA dw1 = v0_dw1 - v0_dw0 (x86 low - x86 high)
            // result ISA dw0 = v1_dw1 - v1_dw0
            // q0 = x86 lows (ISA dw1s), q1 = x86 highs (ISA dw0s), result = q0 - q1
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            // q0 = {v1_dw1, v0_dw1}: XXPERMDI(q0, v1, v0, 0b11)
            XXPERMDI(VSXREG(q0), VSXREG(v1), VSXREG(v0), 0b11);
            // q1 = {v1_dw0, v0_dw0}: XXPERMDI(q1, v1, v0, 0b00)
            XXPERMDI(VSXREG(q1), VSXREG(v1), VSXREG(v0), 0b00);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                XVCMPEQDP(VSXREG(d0), VSXREG(q0), VSXREG(q0));
                XVCMPEQDP(VSXREG(d1), VSXREG(q1), VSXREG(q1));
                XXLAND(VSXREG(d0), VSXREG(d0), VSXREG(d1));
            }
            XVSUBDP(VSXREG(v0), VSXREG(q0), VSXREG(q1));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQDP(VSXREG(d1), VSXREG(v0), VSXREG(v0));
                XXLANDC(VSXREG(d1), VSXREG(d0), VSXREG(d1));
                XXSPLTIB(VSXREG(d0), 63);
                VSLD(VRREG(d1), VRREG(d1), VRREG(d0));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(d1));
            }
            break;

        case 0x7E:
            INST_NAME("MOVD Ed, Gx");
            nextop = F8;
            GETGX(v0, 0);
            if (rex.w) {
                if (MODREG) {
                    ed = TO_NAT((nextop & 7) + (rex.b << 3));
                    MFVSRLD(ed, VSXREG(v0));
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                    MFVSRLD(x4, VSXREG(v0));
                    STD(x4, fixedaddress, ed);
                    SMWRITE2();
                }
            } else {
                if (MODREG) {
                    ed = TO_NAT((nextop & 7) + (rex.b << 3));
                    // Extract x86 low 32 bits: get ISA dw1, take low 32 bits
                    MFVSRLD(ed, VSXREG(v0));
                    RLWINM(ed, ed, 0, 0, 31);  // zero-extend to 32 bits
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                    MFVSRLD(x4, VSXREG(v0));
                    STW(x4, fixedaddress, ed);
                    SMWRITE2();
                }
            }
            break;
        case 0x7F:
            INST_NAME("MOVDQA Ex, Gx");
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

        case 0xA3:
            INST_NAME("BT Ew, Gw");
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                EXTSH(x4, gd);      // sign-extend Gw to allow negative bit offsets
                SRAWI(x4, x4, 4);   // x4 = bit_offset / 16 (word index)
                EXTSW(x4, x4);
                SLDI(x4, x4, 1);    // x4 * 2 (byte offset)
                ADD(x3, x3, x4);
                LHZ(x1, fixedaddress, x3);
                ed = x1;
            }
            IFX (X_CF) {
                ANDI(x2, gd, 0xf);
                SRD(x4, ed, x2);
                RLDIMI(xFlags, x4, F_CF, 63 - F_CF);
            }
            break;

        case 0xA4:
            INST_NAME("SHLD Ew, Gw, Ib");
            nextop = F8;
            u8 = geted_ib(dyn, addr, ninst, nextop) & 0x1f;
            if (u8) {
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETGWEW(x1, x2, 1);
                u8 = F8;
                emit_shld16c(dyn, ninst, rex, ed, gd, u8, x6, x4, x5);
                EWBACK;
            } else {
                FAKEED;
                F8;
            }
            break;
        case 0xA5:
            nextop = F8;
            INST_NAME("SHLD Ew, Gw, CL");
            if (BOX64DRENV(dynarec_safeflags) > 1) {
                READFLAGS(X_ALL);
                SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_FUSION);
            } else
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            GETGWEW(x1, x2, 0);
            ANDI(x4, xRCX, 0x1f);
            UFLAG_IF { CBZ_NEXT(x4); }
            emit_shld16(dyn, ninst, ed, gd, x4, x5, x6, x7);
            EWBACK;
            break;

        case 0xAB:
            INST_NAME("BTS Ew, Gw");
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                EXTSH(x4, gd);
                SRAWI(x4, x4, 4);
                EXTSW(x4, x4);
                SLDI(x4, x4, 1);
                ADD(x3, x3, x4);
                LHZ(x1, fixedaddress, x3);
                ed = x1;
                wback = x3;
            }
            ANDI(x2, gd, 0xf);
            IFX (X_CF) {
                SRD(x4, ed, x2);
                RLDIMI(xFlags, x4, F_CF, 63 - F_CF);
            }
            LI(x4, 1);
            SLD(x4, x4, x2);
            OR(ed, ed, x4);
            if (wback) {
                STH(ed, fixedaddress, wback);
                SMWRITE();
            }
            break;

        case 0xAC:
            nextop = F8;
            INST_NAME("SHRD Ew, Gw, Ib");
            u8 = geted_ib(dyn, addr, ninst, nextop) & 0x1f;
            if (u8) {
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETGWEW(x1, x2, 1);
                u8 = F8;
                emit_shrd16c(dyn, ninst, rex, ed, gd, u8, x6, x4, x5);
                EWBACK;
            } else {
                FAKEED;
                F8;
            }
            break;
        case 0xAD:
            nextop = F8;
            INST_NAME("SHRD Ew, Gw, CL");
            if (BOX64DRENV(dynarec_safeflags) > 1) {
                READFLAGS(X_ALL);
                SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_FUSION);
            } else
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            GETGWEW(x1, x2, 0);
            ANDI(x4, xRCX, 0x1f);
            UFLAG_IF { CBZ_NEXT(x4); }
            emit_shrd16(dyn, ninst, ed, gd, x4, x5, x6, x7);
            EWBACK;
            break;

        case 0xAE:
            nextop = F8;
            if (MODREG)
                switch (nextop) {
                    default: DEFAULT;
                }
            else
                switch ((nextop >> 3) & 7) {
                    case 6:
                        INST_NAME("CLWB Ed");
                        FAKEED;
                        SYNC();
                        break;
                    case 7:
                        INST_NAME("CLFLUSHOPT Ed");
                        FAKEED;
                        SYNC();
                        break;
                    default:
                        DEFAULT;
                }
            break;

        case 0xAF:
            INST_NAME("IMUL Gw, Ew");
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETSEW(x1, 0);
            GETSGW(x2);
            MULLW(gd, gd, ed);  // gd = low 32 bits of signed product
            GWBACK;
            SET_DFNONE();
            CLEAR_FLAGS(x3);
            IFX (X_CF | X_OF) {
                // Overflow if sign-extending low 16 bits doesn't match full 32-bit result
                EXTSH(x1, gd);       // x1 = sign-extend low 16 bits
                XOR(x3, x1, gd);     // compare with full 32-bit product
                SNEZ(x3, x3);
                IFX (X_CF) RLDIMI(xFlags, x3, F_CF, 63 - F_CF);
                IFX (X_OF) RLDIMI(xFlags, x3, F_OF, 63 - F_OF);
            }
            IFX (X_SF) {
                SRDI(x3, gd, 15);
                RLDIMI(xFlags, x3, F_SF, 63 - F_SF);
            }
            IFX (X_PF) emit_pf(dyn, ninst, gd, x3, x4);
            SPILL_EFLAGS();
            break;

        case 0xB3:
            INST_NAME("BTR Ew, Gw");
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                EXTSH(x4, gd);
                SRAWI(x4, x4, 4);
                EXTSW(x4, x4);
                SLDI(x4, x4, 1);
                ADD(x3, x3, x4);
                LHZ(x1, fixedaddress, x3);
                ed = x1;
                wback = x3;
            }
            ANDI(x2, gd, 0xf);
            IFX (X_CF) {
                SRD(x4, ed, x2);
                RLDIMI(xFlags, x4, F_CF, 63 - F_CF);
            }
            LI(x4, 1);
            SLD(x4, x4, x2);
            ANDC(ed, ed, x4);
            if (wback) {
                STH(ed, fixedaddress, wback);
                SMWRITE();
            }
            break;

        case 0xB6:
            INST_NAME("MOVZX Gw, Eb");
            nextop = F8;
            if (MODREG) {
                if (rex.rex) {
                    eb1 = TO_NAT((nextop & 7) + (rex.b << 3));
                    eb2 = 0;
                } else {
                    ed = (nextop & 7);
                    eb1 = TO_NAT(ed & 3); // Ax, Cx, Dx or Bx
                    eb2 = (ed & 4) >> 2;  // L or H
                }
                RLDICL(x1, eb1, 64 - eb2 * 8, 56);  // extract byte
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x4, &fixedaddress, rex, NULL, DS_DISP, 0);
                LBZ(x1, fixedaddress, ed);
            }
            gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3));
            BF_INSERT(gd, x1, 15, 0); // insert in Gw
            break;

        case 0xBA:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 4:
                    INST_NAME("BT Ew, Ib");
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETED(1);
                    u8 = F8;
                    u8 &= 0x0f;
                    IFX (X_CF) {
                        RLDICL(x4, ed, 64 - u8, 63);
                        RLDIMI(xFlags, x4, F_CF, 63 - F_CF);
                    }
                    break;
                case 5:
                    INST_NAME("BTS Ew, Ib");
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETEW(x1, 1);
                    u8 = F8;
                    u8 &= 0x0f;
                    RLDICL(x3, ed, 64 - u8, 63);
                    RLDIMI(xFlags, x3, F_CF, 63 - F_CF);
                    LI(x4, 1);
                    SLDI(x4, x4, u8);
                    OR(ed, ed, x4);
                    EWBACK;
                    break;
                case 6:
                    INST_NAME("BTR Ew, Ib");
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETEW(x1, 1);
                    u8 = F8;
                    u8 &= 0x0f;
                    RLDICL(x3, ed, 64 - u8, 63);
                    RLDIMI(xFlags, x3, F_CF, 63 - F_CF);
                    LI(x4, 1);
                    SLDI(x4, x4, u8);
                    ANDC(ed, ed, x4);
                    EWBACK;
                    break;
                case 7:
                    INST_NAME("BTC Ew, Ib");
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETEW(x1, 1);
                    u8 = F8;
                    u8 &= 0x0f;
                    RLDICL(x3, ed, 64 - u8, 63);
                    RLDIMI(xFlags, x3, F_CF, 63 - F_CF);
                    if (u8 <= 15) {
                        XORI(ed, ed, (1LL << u8));
                    } else {
                        MOV64xw(x3, (1LL << u8));
                        XOR(ed, ed, x3);
                    }
                    EWBACK;
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0xBB:
            INST_NAME("BTC Ew, Gw");
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                EXTSH(x4, gd);
                SRAWI(x4, x4, 4);
                EXTSW(x4, x4);
                SLDI(x4, x4, 1);
                ADD(x3, x3, x4);
                LHZ(x1, fixedaddress, x3);
                ed = x1;
                wback = x3;
            }
            ANDI(x2, gd, 0xf);
            IFX (X_CF) {
                SRD(x4, ed, x2);
                RLDIMI(xFlags, x4, F_CF, 63 - F_CF);
            }
            LI(x4, 1);
            SLD(x4, x4, x2);
            XOR(ed, ed, x4);
            if (wback) {
                STH(ed, fixedaddress, wback);
                SMWRITE();
            }
            break;

        case 0xBC:
            INST_NAME("BSF Gw, Ew");
            if (!BOX64DRENV(dynarec_safeflags)) {
                SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            } else {
                SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            }
            SET_DFNONE();
            CLEAR_FLAGS(x2);
            nextop = F8;
            GETGWEW(x4, x5, 0);
            BNEZ_MARK(ed);
            IFX (X_ZF) ORI(xFlags, xFlags, 1 << F_ZF);
            B_MARK2_nocond;
            MARK;
            CNTTZD(gd, ed);
            GWBACK;
            MARK2;
            if (BOX64DRENV(dynarec_safeflags)) {
                IFX (X_PF) emit_pf(dyn, ninst, gd, x1, x2);
            }
            SPILL_EFLAGS();
            break;
        case 0xBD:
            INST_NAME("BSR Gw, Ew");
            if (!BOX64DRENV(dynarec_safeflags)) {
                SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            } else {
                SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            }
            SET_DFNONE();
            CLEAR_FLAGS(x2);
            nextop = F8;
            GETGWEW(x4, x5, 0);
            BNEZ_MARK(ed);
            IFX (X_ZF) ORI(xFlags, xFlags, 1 << F_ZF);
            B_MARK2_nocond;
            MARK;
            CNTLZD(gd, ed);
            LI(x1, 63);
            SUB(gd, x1, gd);
            GWBACK;
            MARK2;
            if (BOX64DRENV(dynarec_safeflags)) {
                IFX (X_PF) emit_pf(dyn, ninst, gd, x1, x2);
            }
            SPILL_EFLAGS();
            break;

        case 0xBE:
            INST_NAME("MOVSX Gw, Eb");
            nextop = F8;
            GETGD;
            if (MODREG) {
                if (rex.rex) {
                    eb1 = TO_NAT((nextop & 7) + (rex.b << 3));
                    eb2 = 0;
                } else {
                    ed = (nextop & 7);
                    eb1 = TO_NAT(ed & 3); // Ax, Cx, Dx or Bx
                    eb2 = (ed & 4) >> 2;  // L or H
                }
                if (eb2) {
                    RLDICL(x1, eb1, 64 - eb2 * 8, 56);
                    EXTSB(x1, x1);
                } else {
                    EXTSB(x1, eb1);
                }
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x4, &fixedaddress, rex, NULL, DS_DISP, 0);
                LBZ(x1, fixedaddress, ed);
                EXTSB(x1, x1);
            }
            BF_INSERT(gd, x1, 15, 0);
            break;
        case 0xBF:
            INST_NAME("MOVSX Gw, Ew");
            nextop = F8;
            // 16-bit to 16-bit sign extend is a no-op — but we need to read the operand
            if (MODREG) {
                // Register — just copy low 16 bits
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3));
                if (ed != gd) {
                    BF_EXTRACT(x1, ed, 15, 0);
                    BF_INSERT(gd, x1, 15, 0);
                }
                // if ed==gd, it's a nop
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x4, &fixedaddress, rex, NULL, DS_DISP, 0);
                LHZ(x1, fixedaddress, ed);
                gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3));
                BF_INSERT(gd, x1, 15, 0);
            }
            break;

        case 0xC1:
            INST_NAME("XADD Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGWEW(x1, x2, 0);
            MV(x7, ed);
            emit_add16(dyn, ninst, ed, gd, x4, x5, x6);
            EWBACK;
            // gd gets old ed value
            MV(gd, x7);
            GWBACK;
            break;

        case 0xC2:
            INST_NAME("CMPPD Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 1);
            u8 = F8;
            // CMPPD: compare packed doubles with predicate, result is all-ones or all-zeros mask
            // PPC64LE: use XVCMPEQDP/XVCMPGTDP/XVCMPGEDP + logic
            switch (u8 & 7) {
                case 0: // EQ
                    XVCMPEQDP(VSXREG(v0), VSXREG(v0), VSXREG(v1));
                    break;
                case 1: // LT
                    XVCMPGTDP(VSXREG(v0), VSXREG(v1), VSXREG(v0));
                    break;
                case 2: // LE
                    XVCMPGEDP(VSXREG(v0), VSXREG(v1), VSXREG(v0));
                    break;
                case 3: // UNORD — true if either is NaN
                    // NaN != NaN, so !(a == a) || !(b == b) => unordered
                    // XVCMPEQDP(tmp, a, a) => mask where a is not NaN
                    // XVCMPEQDP(tmp2, b, b) => mask where b is not NaN
                    // AND them => mask where both ordered
                    // NOT => mask where unordered
                    q0 = fpu_get_scratch(dyn);
                    q1 = fpu_get_scratch(dyn);
                    XVCMPEQDP(VSXREG(q0), VSXREG(v0), VSXREG(v0));
                    XVCMPEQDP(VSXREG(q1), VSXREG(v1), VSXREG(v1));
                    XXLAND(VSXREG(v0), VSXREG(q0), VSXREG(q1));
                    XXLNOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    break;
                case 4: // NEQ — true if not equal OR unordered
                    XVCMPEQDP(VSXREG(v0), VSXREG(v0), VSXREG(v1));
                    XXLNOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    break;
                case 5: // NLT (not less than) = GE or unordered
                    XVCMPGTDP(VSXREG(v0), VSXREG(v1), VSXREG(v0));
                    XXLNOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    break;
                case 6: // NLE (not less or equal) = GT or unordered
                    XVCMPGEDP(VSXREG(v0), VSXREG(v1), VSXREG(v0));
                    XXLNOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    break;
                case 7: // ORD — true if both are not NaN
                    q0 = fpu_get_scratch(dyn);
                    q1 = fpu_get_scratch(dyn);
                    XVCMPEQDP(VSXREG(q0), VSXREG(v0), VSXREG(v0));
                    XVCMPEQDP(VSXREG(q1), VSXREG(v1), VSXREG(v1));
                    XXLAND(VSXREG(v0), VSXREG(q0), VSXREG(q1));
                    break;
            }
            break;

        case 0xC4:
            INST_NAME("PINSRW Gx, Ed, Ib");
            nextop = F8;
            GETGX(v0, 1);
            if (MODREG) {
                u8 = (F8) & 7;
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x4, &fixedaddress, rex, NULL, DS_DISP, 1);
                u8 = (F8) & 7;
                ed = x3;
                LHZ(ed, fixedaddress, wback);
            }
            // Insert 16-bit value into XMM at position u8
            // On PPC64LE, VINSERTH inserts from VSR low halfword, but element
            // indexing is BE-based. For LE, element u8 is at byte offset (7-u8)*2
            // Alternative: go through GPR
            MFVSRLD(x4, VSXREG(v0));    // x86 low 64 bits (ISA dw1)
            MFVSRD(x5, VSXREG(v0));     // x86 high 64 bits (ISA dw0)
            {
                // Clear the target 16-bit slot and insert new value
                int shift = (u8 & 3) * 16;
                if (u8 < 4) {
                    // Target is in low 64 bits (x4)
                    if (shift == 0) {
                        RLDICR(x4, x4, 0, 47);     // clear low 16 bits? No...
                        // Actually: insert at bit position 'shift'
                        // Clear bits [shift+15:shift], insert ed<<shift
                        RLWINM(x6, ed, 0, 16, 31);  // zero-extend to 16 bits
                        RLDIMI(x4, x6, 0, 48);      // insert at bits [15:0]
                    } else {
                        RLWINM(x6, ed, 0, 16, 31);
                        RLDIMI(x4, x6, shift, 64 - shift - 16);
                    }
                    MTVSRDD(VSXREG(v0), x5, x4);
                } else {
                    // Target is in high 64 bits (x5)
                    RLWINM(x6, ed, 0, 16, 31);
                    RLDIMI(x5, x6, shift, 64 - shift - 16);
                    MTVSRDD(VSXREG(v0), x5, x4);
                }
            }
            break;
        case 0xC5:
            INST_NAME("PEXTRW Gd, Ex, Ib");
            nextop = F8;
            GETGD;
            if (MODREG) {
                GETEX(v0, 0, 1);
                u8 = (F8) & 7;
                // Extract 16-bit value from XMM at position u8
                MFVSRLD(x4, VSXREG(v0));    // x86 low 64 bits (ISA dw1, elements 0-3)
                MFVSRD(x5, VSXREG(v0));     // x86 high 64 bits (ISA dw0, elements 4-7)
                {
                    int shift = (u8 & 3) * 16;
                    if (u8 < 4) {
                        SRDI(gd, x4, shift);
                    } else {
                        SRDI(gd, x5, shift);
                    }
                    RLWINM(gd, gd, 0, 16, 31); // zero-extend to 16 bits
                }
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x4, &fixedaddress, rex, NULL, DS_DISP, 1);
                u8 = (F8) & 7;
                ADDI(x3, wback, fixedaddress + (u8 << 1));
                LHZ(gd, 0, x3);
            }
            break;

        case 0xC6:
            INST_NAME("SHUFPD Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 1);
            u8 = F8;
            // SHUFPD: dst[63:0] = (imm[0] ? Gx[127:64] : Gx[63:0])
            //         dst[127:64] = (imm[1] ? Ex[127:64] : Ex[63:0])
            {
                int sel0 = u8 & 1;
                int sel1 = (u8 >> 1) & 1;
                // SHUFPD: x86 low result = sel0 ? Gx_high : Gx_low (from Gx)
                //         x86 high result = sel1 ? Ex_high : Ex_low (from Ex)
                // ISA: x86 low = ISA dw1 (bits 64-127), x86 high = ISA dw0 (bits 0-63)
                // XXPERMDI(XT, XA, XB, DM):
                //   dw0 = DM[1] ? XA[64:127] : XA[0:63]
                //   dw1 = DM[0] ? XB[64:127] : XB[0:63]
                // XA=Ex(v1) for x86 high (dw0), XB=Gx(v0) for x86 low (dw1)
                // DM bit1: sel1=0 → Ex x86 low (dw1) → need XA[64:127] → DM[1]=1
                //          sel1=1 → Ex x86 high (dw0) → need XA[0:63] → DM[1]=0
                // DM bit0: sel0=0 → Gx x86 low (dw1) → need XB[64:127] → DM[0]=1
                //          sel0=1 → Gx x86 high (dw0) → need XB[0:63] → DM[0]=0
                int dm = ((1 - sel1) << 1) | (1 - sel0);
                XXPERMDI(VSXREG(v0), VSXREG(v1), VSXREG(v0), dm);
            }
            break;

        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
            INST_NAME("BSWAP Reg");
            gd = TO_NAT((opcode & 7) + (rex.b << 3));
            if (rex.w) {
                // 64-bit byte swap
                // Use XXBRD via VSX scratch register
                q0 = fpu_get_scratch(dyn);
                MTVSRD(VSXREG(q0), gd);
                XXBRD(VSXREG(q0), VSXREG(q0));
                MFVSRD(gd, VSXREG(q0));
            } else {
                // 16-bit operand size prefix + BSWAP = undefined behavior
                // Modern x86 CPUs zero the low 16 bits
                CLRLDI(x4, gd, 48);             // x4 = upper bits only... no
                // Actually we need to clear bits 0-15: gd = gd & ~0xFFFF
                // RLDICR clears low bits: RLDICR(gd, gd, 0, 47) keeps bits 63:16, zeros 15:0
                RLDICR(gd, gd, 0, 47);
            }
            break;

        case 0xD0:
            INST_NAME("ADDSUBPD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // ADDSUBPD: dst[63:0] = Gx[63:0] - Ex[63:0], dst[127:64] = Gx[127:64] + Ex[127:64]
            // Use XOR-then-ADD: flip sign of sub-lane in Ex, then add both lanes.
            // NaN care: XOR must NOT flip sign of NaN inputs, since x86 SUB propagates
            // input NaN sign unchanged. Only apply sign flip to non-NaN lanes of Ex.
            q0 = fpu_get_scratch(dyn);
            // Sign mask: dw0=0 (x86 high = add, no flip), dw1=0x8000000000000000 (x86 low = sub, flip)
            LI(x4, 1);
            SLDI(x4, x4, 63);  // x4 = 0x8000000000000000
            MTVSRDD(VSXREG(q0), xZR, x4);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                // Mask sign flip to non-NaN lanes: XOR only where Ex is ordered
                XVCMPEQDP(VSXREG(d1), VSXREG(v1), VSXREG(v1));  // d1 = -1 where Ex NOT NaN
                XXLAND(VSXREG(q0), VSXREG(q0), VSXREG(d1));      // zero sign flip for NaN lanes
                XVCMPEQDP(VSXREG(d0), VSXREG(v0), VSXREG(v0));  // d0 = -1 where Gx NOT NaN
                XXLAND(VSXREG(d0), VSXREG(d0), VSXREG(d1));      // d0 = both ordered mask
            }
            XXLXOR(VSXREG(q0), VSXREG(v1), VSXREG(q0));
            XVADDDP(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            if (!BOX64ENV(dynarec_fastnan)) {
                // Newly generated NaNs (both inputs ordered, result NaN) — set sign bit
                XVCMPEQDP(VSXREG(d1), VSXREG(v0), VSXREG(v0));  // d1 = -1 where result NOT NaN
                XXLANDC(VSXREG(d1), VSXREG(d0), VSXREG(d1));     // both-ordered AND result-NaN
                XXSPLTIB(VSXREG(d0), 63);
                VSLD(VRREG(d1), VRREG(d1), VRREG(d0));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(d1));      // OR sign bit
            }
            break;
        case 0xD1:
            INST_NAME("PSRLW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // Shift right logical packed halfwords by count from Ex[63:0]
            // If count >= 16, result is zero
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            // Extract shift count from x86 low 64 bits (ISA dw1)
            MFVSRLD(x4, VSXREG(v1));
            // Overflow mask: if count > 15, zero the result
            LI(x5, 15);
            MTVSRDD(VSXREG(q1), x5, x5);           // q1 = 15 in both qwords
            XXPERMDI(VSXREG(q0), VSXREG(v1), VSXREG(v1), 3); // splat x86 low qword
            VCMPGTUD(VRREG(q1), VRREG(q0), VRREG(q1));  // q1 = (count > 15) ? -1 : 0
            VNOR(VRREG(q1), VRREG(q1), VRREG(q1));       // q1 = (count <= 15) ? -1 : 0
            // Splat count to all halfwords
            MTVSRDD(VSXREG(q0), x4, x4);
            VSPLTH(VRREG(q0), VRREG(q0), 7);  // ISA hw 7 = LE hw 0 = lo16(x4) = count
            VSRH(VRREG(v0), VRREG(v0), VRREG(q0));
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            break;
        case 0xD2:
            INST_NAME("PSRLD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            // Extract shift count from x86 low 64 bits (ISA dw1)
            MFVSRLD(x4, VSXREG(v1));
            // Overflow mask: if count > 31, zero the result
            LI(x5, 31);
            MTVSRDD(VSXREG(q1), x5, x5);
            XXPERMDI(VSXREG(q0), VSXREG(v1), VSXREG(v1), 3);
            VCMPGTUD(VRREG(q1), VRREG(q0), VRREG(q1));
            VNOR(VRREG(q1), VRREG(q1), VRREG(q1));
            // Splat count to all words
            MTVSRDD(VSXREG(q0), x4, x4);
            VSPLTW(VRREG(q0), VRREG(q0), 3);  // ISA word 3 = LE word 0 = lo32(x4)
            VSRW(VRREG(v0), VRREG(v0), VRREG(q0));
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            break;
        case 0xD3:
            INST_NAME("PSRLQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            XXPERMDI(VSXREG(q0), VSXREG(v1), VSXREG(v1), 3); // splat x86 low qword (ISA dw1)
            LI(x4, 63);
            MTVSRDD(VSXREG(q1), x4, x4);
            VCMPGTUD(VRREG(q1), VRREG(q0), VRREG(q1));
            VNOR(VRREG(q1), VRREG(q1), VRREG(q1));
            VSRD(VRREG(v0), VRREG(v0), VRREG(q0));
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            break;

        case 0xD4:
            INST_NAME("PADDQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VADDUDM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xD5:
            INST_NAME("PMULLW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // Multiply packed signed words, keep low 16 bits of each result
            // PPC VMX doesn't have a direct "multiply low halfword" instruction
            // Use VMULOUH/VMULEUH approach or VMLADDUHM (multiply-low-add unsigned halfword modulo)
            // VMLADDUHM(VRT, VRA, VRB, VRC): VRT = (VRA * VRB + VRC) mod 2^16 for each halfword
            // We want just VRA * VRB (low), so VRC = 0
            q0 = fpu_get_scratch(dyn);
            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
            VMLADDUHM(VRREG(v0), VRREG(v0), VRREG(v1), VRREG(q0));
            break;

        case 0xD6:
            INST_NAME("MOVQ Ex, Gx");
            nextop = F8;
            GETGX(v0, 0);
            if (MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop & 7) + (rex.b << 3));
                // Copy low 64 bits, zero high 64 bits (x86 low = ISA dw1)
                MFVSRLD(x4, VSXREG(v0));
                MTVSRDD(VSXREG(v1), 0, x4);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                // Store low 64 bits to memory (x86 low = ISA dw1)
                MFVSRLD(x4, VSXREG(v0));
                STD(x4, fixedaddress, ed);
                SMWRITE2();
            }
            break;
        case 0xD7:
            INST_NAME("PMOVMSKB Gd, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGD;
            // Extract sign bit of each byte (16 bytes -> 16 bits)
            // PPC64LE: Use VBPERMQ which performs bit permutation
            // VBPERMQ can extract arbitrary bits from a vector
            // We want bit 7 of each byte (the sign bit)
            // Alternative: shift each byte right by 7 to get sign bit, then gather
            // Use VGBBD (vector gather bits by bytes in doubleword) — POWER8+
            // VGBBD transposes the bit matrix within each doubleword
            // After VGBBD, the gathered bits are in specific byte positions
            // Simpler approach: use VBPERMQ
            // VBPERMQ(VRT, VRA, VRB): For each of the 16 bit indices in VRB,
            // extract that bit from VRA, pack results into low halfword of each doubleword
            // We need indices [7, 15, 23, 31, 39, 47, 55, 63, 71, 79, 87, 95, 103, 111, 119, 127]
            // But on PPC64LE these bit indices are big-endian within the register...
            // This is complex. Let's use a simpler GPR-based approach:
            // Extract both 64-bit halves, then extract sign bits
            MFVSRLD(x4, VSXREG(q0));    // x86 low 64 bits (bytes 0-7, ISA dw1)
            MFVSRD(x5, VSXREG(q0));     // x86 high 64 bits (bytes 8-15, ISA dw0)
            // Build mask: x6 = 0x8080808080808080 (isolate MSB of each byte)
            LI(x6, 0);
            ORIS(x6, x6, 0x8080);
            ORI(x6, x6, 0x8080);
            RLDIMI(x6, x6, 32, 0);      // x6 = 0x8080808080808080
            {
                // Extract sign bits of bytes 0-7 using multiply trick:
                // result = (isolated_msbs * 0x0002040810204081) >> 56
                // MULLD gives low 64 bits; gathered sign bits are at bits [63:56]
                AND(x7, x4, x6);
                LIS(x3, 0x0002);         // x3 = 0x00020000
                ORI(x3, x3, 0x0408);     // x3 = 0x00020408
                SLDI(x3, x3, 32);        // x3 = 0x0002040800000000
                ORIS(x3, x3, 0x1020);    // x3 = 0x0002040810200000
                ORI(x3, x3, 0x4081);     // x3 = 0x0002040810204081
                MULLD(x7, x7, x3);       // low 64 bits of product
                SRDI(x7, x7, 56);         // gathered sign bits are at bits [63:56]

                // Extract sign bits of bytes 8-15 (same trick on high 64 bits)
                AND(x3, x5, x6);
                LIS(x4, 0x0002);
                ORI(x4, x4, 0x0408);
                SLDI(x4, x4, 32);
                ORIS(x4, x4, 0x1020);
                ORI(x4, x4, 0x4081);
                MULLD(x3, x3, x4);
                SRDI(x3, x3, 56);

                // Combine: result = (high_byte_mask << 8) | low_byte_mask
                SLDI(x3, x3, 8);
                OR(gd, x7, x3);
            }
            break;

        case 0xD8:
            INST_NAME("PSUBUSB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VSUBUBS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xD9:
            INST_NAME("PSUBUSW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VSUBUHS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xDA:
            INST_NAME("PMINUB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VMINUB(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xDB:
            INST_NAME("PAND Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            break;
        case 0xDC:
            INST_NAME("PADDUSB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VADDUBS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xDD:
            INST_NAME("PADDUSW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VADDUHS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xDE:
            INST_NAME("PMAXUB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VMAXUB(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xDF:
            INST_NAME("PANDN Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            // x86 PANDN: dest = NOT(dest) AND src
            // PPC XXLANDC(T, A, B) = A AND NOT(B)
            XXLANDC(VSXREG(v0), VSXREG(q0), VSXREG(v0));
            break;

        case 0xE0:
            INST_NAME("PAVGB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VAVGUB(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xE1:
            INST_NAME("PSRAW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            q0 = fpu_get_scratch(dyn);
            // Arithmetic right shift: clamp count to 15 (all sign bits)
            // Extract shift count from low 64 bits of Ex (x86 low = ISA dw1)
            MFVSRLD(x4, VSXREG(v1));
            // Clamp to 15
            CMPLDI(x4, 15);
            LI(x5, 15);
            ISEL(x4, x4, x5, 0);    // x4 = (x4 > 15) ? 15 : x4 (CR0.LT=0 means x4>=15)
            // Actually ISEL(RT, RA, RB, BC): RT = (CR bit BC is set) ? RA : RB
            // CMPLDI sets CR0: LT if x4 < 15, so BC=0 (CR0.LT)
            // If x4 < 15: CR0.LT=1, pick RA=x4
            // If x4 >= 15: CR0.LT=0, pick RB=x5=15
            // Splat to all halfword lanes
            MTVSRDD(VSXREG(q0), x4, x4);
            VSPLTH(VRREG(q0), VRREG(q0), 7);  // ISA hw 7 = LE hw 0 = lo16(x4) = count
            // VMX shift uses low bits of each element
            VSRAH(VRREG(v0), VRREG(v0), VRREG(q0));
            break;
        case 0xE2:
            INST_NAME("PSRAD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            q0 = fpu_get_scratch(dyn);
            // Clamp count to 31 (x86 low = ISA dw1)
            MFVSRLD(x4, VSXREG(v1));
            CMPLDI(x4, 31);
            LI(x5, 31);
            ISEL(x4, x4, x5, 0);    // clamp: if x4 < 31 pick x4, else 31
            MTVSRDD(VSXREG(q0), x4, x4);
            VSPLTW(VRREG(q0), VRREG(q0), 3);  // ISA word 3 = LE word 0 = lo32(x4) = count
            VSRAW(VRREG(v0), VRREG(v0), VRREG(q0));
            break;
        case 0xE3:
            INST_NAME("PAVGW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VAVGUH(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xE4:
            INST_NAME("PMULHUW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // High 16 bits of unsigned halfword multiply
            // PPC VMULEUH/VMULOUH produce word-width results from even/odd halfwords
            // Need to interleave them back before packing
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            {
                int d2 = fpu_get_scratch(dyn);
                int d3 = fpu_get_scratch(dyn);
                VMULEUH(VRREG(q0), VRREG(v0), VRREG(v1));   // even halfwords -> words
                VMULOUH(VRREG(q1), VRREG(v0), VRREG(v1));   // odd halfwords -> words
                // Shift each word right by 16 to get high halves
                XXSPLTIB(VSXREG(d2), 16);
                VSRW(VRREG(q0), VRREG(q0), VRREG(d2));
                VSRW(VRREG(q1), VRREG(q1), VRREG(d2));
                // Interleave even/odd word results before packing
                VMRGLW(VRREG(d2), VRREG(q0), VRREG(q1));    // low words interleaved
                VMRGHW(VRREG(d3), VRREG(q0), VRREG(q1));    // high words interleaved
                VPKUWUM(VRREG(v0), VRREG(d3), VRREG(d2));
            }
            break;
        case 0xE5:
            INST_NAME("PMULHW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // High 16 bits of signed halfword multiply
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            {
                int d2 = fpu_get_scratch(dyn);
                int d3 = fpu_get_scratch(dyn);
                VMULESH(VRREG(q0), VRREG(v0), VRREG(v1));   // even signed halfwords -> words
                VMULOSH(VRREG(q1), VRREG(v0), VRREG(v1));   // odd signed halfwords -> words
                // Shift right arithmetic by 16 to get high halves
                XXSPLTIB(VSXREG(d2), 16);
                VSRAW(VRREG(q0), VRREG(q0), VRREG(d2));
                VSRAW(VRREG(q1), VRREG(q1), VRREG(d2));
                // Interleave even/odd word results before packing
                VMRGLW(VRREG(d2), VRREG(q0), VRREG(q1));    // low words interleaved
                VMRGHW(VRREG(d3), VRREG(q0), VRREG(q1));    // high words interleaved
                VPKUWUM(VRREG(v0), VRREG(d3), VRREG(d2));
            }
            break;
        case 0xE6:
            INST_NAME("CVTTPD2DQ Gx, Ex");
            nextop = F8;
            GETEX(v1, 0, 0);
            GETGX_empty(v0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            MTFSB0(23);  // clear VXCVI (always needed on PPC for overflow detection)
            // Extract and convert each double using FPR-space FCTIWZ (truncation)
            MFVSRLD(x4, VSXREG(v1));    // double0 (x86 low = ISA dw1)
            MTVSRD(d0, x4);              // FPR space
            FCTIWZ(d0, d0);              // truncate to int32
            MFVSRWZ(x4, d0);
            MFVSRD(x5, VSXREG(v1));     // double1 (x86 high = ISA dw0)
            MTVSRD(d1, x5);
            FCTIWZ(d1, d1);
            MFVSRWZ(x5, d1);
            // Check VXCVI: PPC gives 0x7FFFFFFF for positive overflow, x86 wants 0x80000000
            MFFS(SCRATCH0);
            STFD(SCRATCH0, -8, xSP);
            LD(x6, -8, xSP);
            RLWINM(x6, x6, 24, 31, 31);
            CMPLDI(x6, 0);
            BEQZ_MARK(x6);
            // Substitute 0x80000000 for overflow
            LI(x4, 0);
            ORI(x4, x4, 0x8000);
            SLDI(x4, x4, 16);
            MR(x5, x4);
            MARK;
            // Pack: low 32 = x4, next 32 = x5, upper 64 = 0
            CLRLDI(x4, x4, 32);
            SLDI(x5, x5, 32);
            OR(x4, x4, x5);
            MTVSRDD(VSXREG(v0), 0, x4);
            break;

        case 0xE7:
            INST_NAME("MOVNTDQ Ex, Gx");
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
        case 0xE8:
            INST_NAME("PSUBSB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VSUBSBS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xE9:
            INST_NAME("PSUBSW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VSUBSHS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xEA:
            INST_NAME("PMINSW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VMINSH(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xEB:
            INST_NAME("POR Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            break;
        case 0xEC:
            INST_NAME("PADDSB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VADDSBS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xED:
            INST_NAME("PADDSW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VADDSHS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xEE:
            INST_NAME("PMAXSW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VMAXSH(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xEF:
            INST_NAME("PXOR Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG && ((nextop & 7) + (rex.b << 3) == gd)) {
                // special case for PXOR Gx, Gx => zero
                q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
            } else {
                q0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                GETEX(q1, 0, 0);
                XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q1));
            }
            break;

        case 0xF0:
            INST_NAME("LDDQU Gx, Ex");
            nextop = F8;
            GETGX_empty(v0);
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DQ_DISP, 0);
                LXV(VSXREG(v0), fixedaddress, ed);
            }
            break;
        case 0xF1:
            INST_NAME("PSLLW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            // Extract shift count from x86 low 64 bits (ISA dw1)
            MFVSRLD(x4, VSXREG(v1));
            // Overflow mask: if count > 15, zero the result
            LI(x5, 15);
            MTVSRDD(VSXREG(q1), x5, x5);
            XXPERMDI(VSXREG(q0), VSXREG(v1), VSXREG(v1), 3);
            VCMPGTUD(VRREG(q1), VRREG(q0), VRREG(q1));
            VNOR(VRREG(q1), VRREG(q1), VRREG(q1));   // in-range mask
            // Splat count to all halfwords
            MTVSRDD(VSXREG(q0), x4, x4);
            VSPLTH(VRREG(q0), VRREG(q0), 7);  // ISA hw 7 = LE hw 0 = lo16(x4)
            VSLH(VRREG(v0), VRREG(v0), VRREG(q0));
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            break;
        case 0xF2:
            INST_NAME("PSLLD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            // Extract shift count from x86 low 64 bits (ISA dw1)
            MFVSRLD(x4, VSXREG(v1));
            // Overflow mask: if count > 31, zero the result
            LI(x5, 31);
            MTVSRDD(VSXREG(q1), x5, x5);
            XXPERMDI(VSXREG(q0), VSXREG(v1), VSXREG(v1), 3);
            VCMPGTUD(VRREG(q1), VRREG(q0), VRREG(q1));
            VNOR(VRREG(q1), VRREG(q1), VRREG(q1));
            // Splat count to all words
            MTVSRDD(VSXREG(q0), x4, x4);
            VSPLTW(VRREG(q0), VRREG(q0), 3);  // ISA word 3 = LE word 0 = lo32(x4)
            VSLW(VRREG(v0), VRREG(v0), VRREG(q0));
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            break;
        case 0xF3:
            INST_NAME("PSLLQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            XXPERMDI(VSXREG(q0), VSXREG(v1), VSXREG(v1), 3); // splat x86 low qword (ISA dw1)
            LI(x4, 63);
            MTVSRDD(VSXREG(q1), x4, x4);
            VCMPGTUD(VRREG(q1), VRREG(q0), VRREG(q1));
            VNOR(VRREG(q1), VRREG(q1), VRREG(q1));
            VSLD(VRREG(v0), VRREG(v0), VRREG(q0));
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            break;
        case 0xF4:
            INST_NAME("PMULUDQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // Multiply unsigned dwords at even positions (0,2) -> qwords
            // x86: dst[63:0] = dst[31:0] * src[31:0], dst[127:64] = dst[95:64] * src[95:64]
            // On PPC64LE, x86 word 0 = LE word 0 = ISA word 3 (odd), x86 word 2 = LE word 2 = ISA word 1 (odd)
            // So VMULOUW (multiply ISA odd words) matches x86 semantics
            VMULOUW(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xF5:
            INST_NAME("PMADDWD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // Multiply signed halfwords pairwise, add adjacent pairs -> words
            // VMULESH: even signed halfwords -> signed words (pairs at positions 0,2,4,6)
            // VMULOSH: odd signed halfwords -> signed words (pairs at positions 1,3,5,7)
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            VMULESH(VRREG(q0), VRREG(v0), VRREG(v1));   // even products
            VMULOSH(VRREG(q1), VRREG(v0), VRREG(v1));   // odd products
            VADDUWM(VRREG(v0), VRREG(q0), VRREG(q1));   // add pairs
            break;
        case 0xF6:
            INST_NAME("PSADBW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // Sum of absolute differences of unsigned bytes
            // Result: sum of |a[i]-b[i]| for bytes 0-7 in low qword, bytes 8-15 in high qword
            // Use GPR byte extraction for correctness
            {
                // Process low qword
                MFVSRLD(x4, VSXREG(v0));    // a low 64 bits
                MFVSRLD(x5, VSXREG(v1));    // b low 64 bits
                LI(x3, 0);  // accumulator
                for (int byte_idx = 0; byte_idx < 8; byte_idx++) {
                    int shift = byte_idx * 8;
                    if (shift == 0) {
                        CLRLDI(x6, x4, 56);
                        CLRLDI(x7, x5, 56);
                    } else {
                        SRDI(x6, x4, shift);
                        CLRLDI(x6, x6, 56);
                        SRDI(x7, x5, shift);
                        CLRLDI(x7, x7, 56);
                    }
                    SUBF(x6, x7, x6);      // a_byte - b_byte (signed)
                    SRADI(x7, x6, 63);      // sign extend
                    XOR(x6, x6, x7);
                    SUBF(x6, x7, x6);      // abs()
                    ADD(x3, x3, x6);
                }
                MR(x1, x3);  // x1 = SAD_low (save in x1, will reload high data into x4,x5)
                
                // Process high qword
                MFVSRD(x4, VSXREG(v0));     // a high 64 bits (ISA dw0)
                MFVSRD(x5, VSXREG(v1));     // b high 64 bits (ISA dw0)
                LI(x3, 0);  // accumulator
                for (int byte_idx = 0; byte_idx < 8; byte_idx++) {
                    int shift = byte_idx * 8;
                    if (shift == 0) {
                        CLRLDI(x6, x4, 56);
                        CLRLDI(x7, x5, 56);
                    } else {
                        SRDI(x6, x4, shift);
                        CLRLDI(x6, x6, 56);
                        SRDI(x7, x5, shift);
                        CLRLDI(x7, x7, 56);
                    }
                    SUBF(x6, x7, x6);
                    SRADI(x7, x6, 63);
                    XOR(x6, x6, x7);
                    SUBF(x6, x7, x6);
                    ADD(x3, x3, x6);
                }
                // x3 = SAD_high, x1 = SAD_low
                MTVSRDD(VSXREG(v0), x3, x1);
            }
            break;
        case 0xF7:
            INST_NAME("MASKMOVDQU Gx, Ex");
            nextop = F8;
            GETGX(v0, 0);
            GETEX(v1, 0, 0);
            // Byte-masked store to [RDI]
            // For each byte, if high bit of mask byte is set, store the data byte
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            // Load current data at [RDI]
            LXV(VSXREG(q0), 0, xRDI);
            // Create byte selection mask from sign bits: VSRAB by 7 gives 0x00 or 0xFF
            XXSPLTIB(VSXREG(q1), 7);
            VSRAB(VRREG(q1), VRREG(v1), VRREG(q1));  // q1 = 0xFF where mask bit set, 0x00 otherwise
            // Select: where mask is 0xFF pick v0, else keep q0
            VSEL(VRREG(q0), VRREG(q0), VRREG(v0), VRREG(q1));
            STXV(VSXREG(q0), 0, xRDI);
            break;

        case 0xF8:
            INST_NAME("PSUBB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VSUBUBM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xF9:
            INST_NAME("PSUBW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VSUBUHM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xFA:
            INST_NAME("PSUBD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VSUBUWM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xFB:
            INST_NAME("PSUBQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VSUBUDM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xFC:
            INST_NAME("PADDB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VADDUBM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xFD:
            INST_NAME("PADDW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VADDUHM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xFE:
            INST_NAME("PADDD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VADDUWM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;

        case 0x38: // SSSE3+ opcodes
            nextop = F8;
            switch (nextop) {
                case 0x00:
                    INST_NAME("PSHUFB Gx, Ex");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 0);
                    d0 = fpu_get_scratch(dyn);
                    d1 = fpu_get_scratch(dyn);
                    // PSHUFB: for each byte i, if bit 7 of Ex[i] is set, result[i]=0, else result[i]=Gx[Ex[i] & 0xF]
                    // VPERM(Vrt, Vra, Vrb, Vrc): Vrt[i] = (Vra||Vrb)[Vrc[i] & 0x1F]
                    // VPERM uses big-endian byte numbering even on LE:
                    //   VPERM index 0 = BE byte 0 = LE byte 15 (highest)
                    //   VPERM index 15 = BE byte 15 = LE byte 0 (lowest)
                    // x86 PSHUFB uses LE byte numbering: index 0 = lowest byte
                    // Fix: XOR low nibble with 0x0F to convert LE→BE indexing
                    {
                        int d2 = fpu_get_scratch(dyn);
                        XXSPLTIB(VSXREG(d2), 0x0F);
                        VXOR(VRREG(d0), VRREG(v1), VRREG(d2));  // d0 = adjusted indices
                    }
                    VPERM(VRREG(d1), VRREG(v0), VRREG(v0), VRREG(d0));
                    // Create mask: where Ex[i] has bit 7 set, zero the result
                    // VSRAB by 7 to broadcast sign bit, then VANDC to zero
                    VSPLTISB(VRREG(d0), 7);
                    VSRAB(VRREG(d0), VRREG(v1), VRREG(d0)); // d0 = 0xFF where bit7 set, 0x00 where clear
                    XXLANDC(VSXREG(v0), VSXREG(d1), VSXREG(d0)); // result = shuffled AND NOT(sign_mask)
                    break;
                case 0x01:
                    INST_NAME("PHADDW Gx, Ex");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 0);
                    d0 = fpu_get_scratch(dyn);
                    d1 = fpu_get_scratch(dyn);
                    {
                        // Horizontal add halfwords: result[i] = src[2i] + src[2i+1]
                        // Shift each dword right by 16 to get odd halfwords, add with original,
                        // then pack low halfwords from each dword
                        int d2 = fpu_get_scratch(dyn);
                        XXSPLTIB(VSXREG(d2), 16); // shift count vector (16 in each byte → 16 in low bits of each dword)
                        VSRW(VRREG(d0), VRREG(v0), VRREG(d2)); // Gx: odd halfwords → low hw of each dword
                        VADDUHM(VRREG(d0), VRREG(v0), VRREG(d0)); // Gx sums in low hw of each dword
                        VSRW(VRREG(d1), VRREG(v1), VRREG(d2)); // Ex: odd halfwords → low hw of each dword
                        VADDUHM(VRREG(d1), VRREG(v1), VRREG(d1)); // Ex sums in low hw of each dword
                        // Pack low halfword from each dword: [Gx_sums | Ex_sums]
                        VPKUWUM(VRREG(v0), VRREG(d1), VRREG(d0));
                    }
                    break;
                case 0x02:
                    INST_NAME("PHADDD Gx, Ex");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 0);
                    d0 = fpu_get_scratch(dyn);
                    d1 = fpu_get_scratch(dyn);
                    {
                        // Horizontal add dwords: result[i] = src[2i] + src[2i+1]
                        // Shift each qword right by 32 to get odd dwords, add with original,
                        // then pack low dwords from each qword
                        int d2 = fpu_get_scratch(dyn);
                        XXSPLTIB(VSXREG(d2), 32); // shift count (32 in low bits of each qword)
                        VSRD(VRREG(d0), VRREG(v0), VRREG(d2)); // Gx: odd dwords → low dw of each qword
                        VADDUWM(VRREG(d0), VRREG(v0), VRREG(d0)); // Gx sums in low dw of each qword
                        VSRD(VRREG(d1), VRREG(v1), VRREG(d2)); // Ex: odd dwords → low dw of each qword
                        VADDUWM(VRREG(d1), VRREG(v1), VRREG(d1)); // Ex sums in low dw of each qword
                        // Pack low dword from each qword
                        VPKUDUM(VRREG(v0), VRREG(d1), VRREG(d0));
                    }
                    break;
                case 0x03:
                    INST_NAME("PHADDSW Gx, Ex");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 0);
                    d0 = fpu_get_scratch(dyn);
                    d1 = fpu_get_scratch(dyn);
                    {
                        // Horizontal saturating add halfwords:
                        // result[i] = sat16(src[2i] + src[2i+1])
                        // Sign-extend even/odd halfwords to dwords, add as dwords, pack with saturation
                        int d2 = fpu_get_scratch(dyn);
                        XXSPLTIB(VSXREG(d2), 16); // shift count
                        // Gx: separate even and odd halfwords as sign-extended dwords
                        VSLW(VRREG(d0), VRREG(v0), VRREG(d2));  // even hw → high position
                        VSRAW(VRREG(d0), VRREG(d0), VRREG(d2)); // sign-extend to dword
                        VSRAW(VRREG(d1), VRREG(v0), VRREG(d2)); // odd hw sign-extended to dword
                        VADDUWM(VRREG(d0), VRREG(d0), VRREG(d1)); // dword sums for Gx
                        // Ex: same
                        VSLW(VRREG(d1), VRREG(v1), VRREG(d2));
                        VSRAW(VRREG(d1), VRREG(d1), VRREG(d2));
                        VSRAW(VRREG(d2), VRREG(v1), VRREG(d2)); // clobbers shift count (ok, done with it)
                        VADDUWM(VRREG(d1), VRREG(d1), VRREG(d2));
                        // Pack dwords → saturated signed halfwords
                        VPKSWSS(VRREG(v0), VRREG(d1), VRREG(d0));
                    }
                    break;
                case 0x04:
                    INST_NAME("PMADDUBSW Gx, Ex");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 0);
                    d0 = fpu_get_scratch(dyn);
                    d1 = fpu_get_scratch(dyn);
                    // Multiply unsigned bytes of Gx by signed bytes of Ex, pairwise saturated add → words
                    // Result[i] = saturate_s16(Gx_u8[2i]*Ex_s8[2i] + Gx_u8[2i+1]*Ex_s8[2i+1])
                    // Approach: zero-extend Gx and sign-extend Ex bytes to halfwords,
                    // multiply even/odd halfword pairs to words, add, pack with saturation
                    {
                        int d2_tmp = fpu_get_scratch(dyn);
                        XXLXOR(VSXREG(d2_tmp), VSXREG(d2_tmp), VSXREG(d2_tmp));
                        // Gx unsigned bytes → halfwords (low and high halves)
                        VMRGLB(VRREG(d0), VRREG(d2_tmp), VRREG(v0)); // zero-extend low 8 bytes → halfwords
                        VMRGHB(VRREG(d1), VRREG(d2_tmp), VRREG(v0)); // zero-extend high 8 bytes → halfwords
                        // Ex signed bytes → halfwords (low and high halves)
                        int d3 = fpu_get_scratch(dyn);
                        int d4 = fpu_get_scratch(dyn);
                        VUPKLSB(VRREG(d2_tmp), VRREG(v1));  // sign-extend low 8 bytes → halfwords
                        VUPKHSB(VRREG(d3), VRREG(v1));       // sign-extend high 8 bytes → halfwords
                        // Multiply corresponding halfword pairs to words, then add adjacent
                        // Even products → words, odd products → words, then add with saturation
                        VMULESH(VRREG(d4), VRREG(d0), VRREG(d2_tmp)); // even hw → words
                        VMULOSH(VRREG(d0), VRREG(d0), VRREG(d2_tmp)); // odd hw → words
                        VADDSWS(VRREG(d0), VRREG(d4), VRREG(d0));     // add pairs (signed word saturate)
                        // Same for high part
                        VMULESH(VRREG(d4), VRREG(d1), VRREG(d3));
                        VMULOSH(VRREG(d1), VRREG(d1), VRREG(d3));
                        VADDSWS(VRREG(d1), VRREG(d4), VRREG(d1));
                        // Pack words → saturated signed halfwords
                        VPKSWSS(VRREG(v0), VRREG(d1), VRREG(d0));
                    }
                    break;
                case 0x05:
                    INST_NAME("PHSUBW Gx, Ex");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 0);
                    d0 = fpu_get_scratch(dyn);
                    d1 = fpu_get_scratch(dyn);
                    {
                        // Horizontal subtract halfwords: result[i] = src[2i] - src[2i+1]
                        // Shift each dword right by 16 to get odd halfwords, subtract from original,
                        // then pack low halfwords from each dword
                        int d2 = fpu_get_scratch(dyn);
                        XXSPLTIB(VSXREG(d2), 16); // shift count
                        VSRW(VRREG(d0), VRREG(v0), VRREG(d2)); // Gx: odd halfwords → low hw of each dword
                        VSUBUHM(VRREG(d0), VRREG(v0), VRREG(d0)); // Gx: even - odd in low hw of each dword
                        VSRW(VRREG(d1), VRREG(v1), VRREG(d2)); // Ex: odd halfwords → low hw of each dword
                        VSUBUHM(VRREG(d1), VRREG(v1), VRREG(d1)); // Ex: even - odd in low hw of each dword
                        VPKUWUM(VRREG(v0), VRREG(d1), VRREG(d0));
                    }
                    break;
                case 0x06:
                    INST_NAME("PHSUBD Gx, Ex");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 0);
                    d0 = fpu_get_scratch(dyn);
                    d1 = fpu_get_scratch(dyn);
                    {
                        // Horizontal subtract dwords: result[i] = src[2i] - src[2i+1]
                        int d2 = fpu_get_scratch(dyn);
                        XXSPLTIB(VSXREG(d2), 32); // shift count
                        VSRD(VRREG(d0), VRREG(v0), VRREG(d2)); // Gx: odd dwords → low dw of each qword
                        VSUBUWM(VRREG(d0), VRREG(v0), VRREG(d0)); // Gx: even - odd
                        VSRD(VRREG(d1), VRREG(v1), VRREG(d2)); // Ex: odd dwords → low dw of each qword
                        VSUBUWM(VRREG(d1), VRREG(v1), VRREG(d1)); // Ex: even - odd
                        VPKUDUM(VRREG(v0), VRREG(d1), VRREG(d0));
                    }
                    break;
                case 0x07:
                    INST_NAME("PHSUBSW Gx, Ex");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 0);
                    d0 = fpu_get_scratch(dyn);
                    d1 = fpu_get_scratch(dyn);
                    {
                        // Horizontal saturating subtract halfwords:
                        // result[i] = sat16(src[2i] - src[2i+1])
                        // Sign-extend even/odd halfwords to dwords, subtract as dwords, pack with saturation
                        int d2 = fpu_get_scratch(dyn);
                        XXSPLTIB(VSXREG(d2), 16); // shift count
                        // Gx: separate even and odd halfwords as sign-extended dwords
                        VSLW(VRREG(d0), VRREG(v0), VRREG(d2));  // even hw → high position
                        VSRAW(VRREG(d0), VRREG(d0), VRREG(d2)); // sign-extend to dword
                        VSRAW(VRREG(d1), VRREG(v0), VRREG(d2)); // odd hw sign-extended to dword
                        VSUBUWM(VRREG(d0), VRREG(d0), VRREG(d1)); // dword differences for Gx
                        // Ex: same
                        VSLW(VRREG(d1), VRREG(v1), VRREG(d2));
                        VSRAW(VRREG(d1), VRREG(d1), VRREG(d2));
                        VSRAW(VRREG(d2), VRREG(v1), VRREG(d2)); // clobbers shift count (ok, done with it)
                        VSUBUWM(VRREG(d1), VRREG(d1), VRREG(d2));
                        // Pack dwords → saturated signed halfwords
                        VPKSWSS(VRREG(v0), VRREG(d1), VRREG(d0));
                    }
                    break;
                case 0x08:
                    INST_NAME("PSIGNB Gx, Ex");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 0);
                    // PSIGNB: if Ex[i]<0, Gx[i]=-Gx[i]; if Ex[i]==0, Gx[i]=0; if Ex[i]>0, Gx[i] unchanged
                    // PPC has no VSIGNCOV. Use: negate = VSUBUBS(0, Gx); then select via sign compare
                    d0 = fpu_get_scratch(dyn);
                    d1 = fpu_get_scratch(dyn);
                    {
                        int d2_tmp = fpu_get_scratch(dyn);
                        XXLXOR(VSXREG(d2_tmp), VSXREG(d2_tmp), VSXREG(d2_tmp)); // zero
                        VSUBUBM(VRREG(d0), VRREG(d2_tmp), VRREG(v0)); // d0 = -Gx (negate bytes)
                        // mask_neg: where Ex < 0 → 0xFF
                        VCMPGTSB(VRREG(d1), VRREG(d2_tmp), VRREG(v1)); // d1 = 0xFF where 0>Ex[i], i.e., Ex[i]<0
                        // mask_zero: where Ex == 0 → 0xFF
                        VCMPEQUB(VRREG(d2_tmp), VRREG(v1), VRREG(d2_tmp)); // d2 = 0xFF where Ex[i]==0
                        // result: if neg, pick negated; if zero, pick 0; else keep original
                        // Start with original, select negated where neg, then zero where zero
                        VSEL(VRREG(v0), VRREG(v0), VRREG(d0), VRREG(d1)); // if neg, use -Gx
                        XXLANDC(VSXREG(v0), VSXREG(v0), VSXREG(d2_tmp)); // zero where Ex==0
                    }
                    break;
                case 0x09:
                    INST_NAME("PSIGNW Gx, Ex");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 0);
                    d0 = fpu_get_scratch(dyn);
                    d1 = fpu_get_scratch(dyn);
                    {
                        int d2_tmp = fpu_get_scratch(dyn);
                        XXLXOR(VSXREG(d2_tmp), VSXREG(d2_tmp), VSXREG(d2_tmp));
                        VSUBUHM(VRREG(d0), VRREG(d2_tmp), VRREG(v0));
                        VCMPGTSH(VRREG(d1), VRREG(d2_tmp), VRREG(v1));
                        VCMPEQUH(VRREG(d2_tmp), VRREG(v1), VRREG(d2_tmp));
                        VSEL(VRREG(v0), VRREG(v0), VRREG(d0), VRREG(d1));
                        XXLANDC(VSXREG(v0), VSXREG(v0), VSXREG(d2_tmp));
                    }
                    break;
                case 0x0A:
                    INST_NAME("PSIGND Gx, Ex");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 0);
                    d0 = fpu_get_scratch(dyn);
                    d1 = fpu_get_scratch(dyn);
                    {
                        int d2_tmp = fpu_get_scratch(dyn);
                        XXLXOR(VSXREG(d2_tmp), VSXREG(d2_tmp), VSXREG(d2_tmp));
                        VSUBUWM(VRREG(d0), VRREG(d2_tmp), VRREG(v0));
                        VCMPGTSW(VRREG(d1), VRREG(d2_tmp), VRREG(v1));
                        VCMPEQUW(VRREG(d2_tmp), VRREG(v1), VRREG(d2_tmp));
                        VSEL(VRREG(v0), VRREG(v0), VRREG(d0), VRREG(d1));
                        XXLANDC(VSXREG(v0), VSXREG(v0), VSXREG(d2_tmp));
                    }
                    break;
                case 0x0B:
                    INST_NAME("PMULHRSW Gx, Ex");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 0);
                    // PMULHRSW: result[i] = (((Gx[i]*Ex[i]) >> 14) + 1) >> 1
                    // Multiply signed halfwords → words, shift right 14, add 1, shift right 1
                    d0 = fpu_get_scratch(dyn);
                    d1 = fpu_get_scratch(dyn);
                    {
                        int d2_tmp = fpu_get_scratch(dyn);
                        int d3 = fpu_get_scratch(dyn);
                        VMULESH(VRREG(d0), VRREG(v0), VRREG(v1)); // even hw products → words
                        VMULOSH(VRREG(d1), VRREG(v0), VRREG(v1)); // odd hw products → words
                        // Shift right arithmetic by 14
                        XXSPLTIB(VSXREG(d2_tmp), 14);
                        VSRAW(VRREG(d0), VRREG(d0), VRREG(d2_tmp));
                        VSRAW(VRREG(d1), VRREG(d1), VRREG(d2_tmp));
                        // Add 1 and shift right by 1 (rounding)
                        VSPLTISW(VRREG(d2_tmp), 1);
                        VADDUWM(VRREG(d0), VRREG(d0), VRREG(d2_tmp));
                        VADDUWM(VRREG(d1), VRREG(d1), VRREG(d2_tmp));
                        VSRAW(VRREG(d0), VRREG(d0), VRREG(d2_tmp));
                        VSRAW(VRREG(d1), VRREG(d1), VRREG(d2_tmp));
                        // Merge even/odd word results back and pack to halfwords
                        // d0 has even results at word positions, d1 has odd results
                        // On PPC64LE: VMULESH produces x86 odd-index results, VMULOSH produces even-index
                        // Interleave with correct order for LE halfword packing
                        VMRGLW(VRREG(d2_tmp), VRREG(d0), VRREG(d1));  // low words interleaved
                        VMRGHW(VRREG(d3), VRREG(d0), VRREG(d1));      // high words interleaved
                        // Pack low halfword of each word: gives {r0,r1,r2,r3,r4,r5,r6,r7}
                        VPKUWUM(VRREG(v0), VRREG(d3), VRREG(d2_tmp));
                    }
                    break;
                case 0x10:
                    INST_NAME("PBLENDVB Gx, Ex");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 0);
                    d0 = sse_get_reg(dyn, ninst, x1, 0, 0); // XMM0 is the mask
                    if (v0 != v1) {
                        d1 = fpu_get_scratch(dyn);
                        // Create mask: broadcast sign bit of each byte in XMM0
                        VSPLTISB(VRREG(d1), 7);
                        VSRAB(VRREG(d1), VRREG(d0), VRREG(d1)); // d1 = 0xFF where bit7 set, 0 otherwise
                        // Select: where mask is 1, pick Ex; where 0, keep Gx
                        VSEL(VRREG(v0), VRREG(v0), VRREG(v1), VRREG(d1));
                    }
                    break;
                case 0x14:
                    INST_NAME("BLENDVPS Gx, Ex");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 0);
                    d0 = sse_get_reg(dyn, ninst, x1, 0, 0); // XMM0
                    if (v0 != v1) {
                        d1 = fpu_get_scratch(dyn);
                        // Broadcast sign bit of each dword
                        VSPLTISW(VRREG(d1), 0);
                        VCMPGTSW(VRREG(d1), VRREG(d1), VRREG(d0)); // d1 = 0xFFFFFFFF where XMM0<0
                        VSEL(VRREG(v0), VRREG(v0), VRREG(v1), VRREG(d1));
                    }
                    break;
                case 0x15:
                    INST_NAME("BLENDVPD Gx, Ex");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 0);
                    d0 = sse_get_reg(dyn, ninst, x1, 0, 0); // XMM0
                    if (v0 != v1) {
                        d1 = fpu_get_scratch(dyn);
                        // Broadcast sign bit of each qword
                        VSPLTISW(VRREG(d1), 0);
                        VCMPGTSD(VRREG(d1), VRREG(d1), VRREG(d0)); // d1 = 0xFFFF... where XMM0<0
                        VSEL(VRREG(v0), VRREG(v0), VRREG(v1), VRREG(d1));
                    }
                    break;
                case 0x17:
                    INST_NAME("PTEST Gx, Ex");
                    nextop = F8;
                    SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    GETGX(v0, 0);
                    GETEX(v1, 0, 0);
                    CLEAR_FLAGS(x3);
                    SET_DFNONE();
                    d0 = fpu_get_scratch(dyn);
                    IFX (X_ZF) {
                        // ZF = (Gx AND Ex) == 0
                        XXLAND(VSXREG(d0), VSXREG(v1), VSXREG(v0));
                        // Check if all zero: MFVSRD + MFVSRLD and OR
                        MFVSRD(x1, VSXREG(d0));
                        MFVSRLD(x2, VSXREG(d0));
                        OR(x1, x1, x2);
                        CMPDI(x1, 0);
                        BNE(8);
                        ORI(xFlags, xFlags, 1 << F_ZF);
                    }
                    IFX (X_CF) {
                        // CF = (NOT(Gx) AND Ex) == 0
                        XXLANDC(VSXREG(d0), VSXREG(v1), VSXREG(v0)); // Ex AND NOT(Gx)
                        MFVSRD(x1, VSXREG(d0));
                        MFVSRLD(x2, VSXREG(d0));
                        OR(x1, x1, x2);
                        CMPDI(x1, 0);
                        BNE(8);
                        ORI(xFlags, xFlags, 1 << F_CF);
                    }
                    break;
                case 0x1C:
                    INST_NAME("PABSB Gx, Ex");
                    nextop = F8;
                    GETEX(v1, 0, 0);
                    GETGX_empty(v0);
                    // Absolute value of signed bytes
                    // abs(x) = (x XOR mask) - mask, where mask = x >> 7 (arithmetic)
                    d0 = fpu_get_scratch(dyn);
                    VSPLTISB(VRREG(d0), 7);
                    VSRAB(VRREG(d0), VRREG(v1), VRREG(d0)); // sign mask: 0xFF where neg, 0 where pos
                    XXLXOR(VSXREG(v0), VSXREG(v1), VSXREG(d0));  // XOR with sign mask
                    VSUBUBM(VRREG(v0), VRREG(v0), VRREG(d0));    // subtract sign mask
                    break;
                case 0x1D:
                    INST_NAME("PABSW Gx, Ex");
                    nextop = F8;
                    GETEX(v1, 0, 0);
                    GETGX_empty(v0);
                    d0 = fpu_get_scratch(dyn);
                    VSPLTISH(VRREG(d0), 15);
                    VSRAH(VRREG(d0), VRREG(v1), VRREG(d0)); // halfword sign mask
                    XXLXOR(VSXREG(v0), VSXREG(v1), VSXREG(d0));
                    VSUBUHM(VRREG(v0), VRREG(v0), VRREG(d0));
                    break;
                case 0x1E:
                    INST_NAME("PABSD Gx, Ex");
                    nextop = F8;
                    GETEX(v1, 0, 0);
                    GETGX_empty(v0);
                    d0 = fpu_get_scratch(dyn);
                    VSPLTISW(VRREG(d0), 0);
                    VSUBUWM(VRREG(d0), VRREG(d0), VRREG(v1)); // negate
                    VMAXSW(VRREG(v0), VRREG(v1), VRREG(d0));  // max(x, -x) = abs(x)
                    break;
                case 0x20:
                    INST_NAME("PMOVSXBW Gx, Ex"); // SSE4
                    nextop = F8;
                    GETEX64(v1, 0, 0);
                    GETGX_empty(v0);
                    // Sign-extend low 8 bytes → 8 halfwords
                    VUPKLSB(VRREG(v0), VRREG(v1));
                    break;
                case 0x21:
                    INST_NAME("PMOVSXBD Gx, Ex"); // SSE4
                    nextop = F8;
                    GETEX32(v1, 0, 0);
                    GETGX_empty(v0);
                    // Sign-extend low 4 bytes → 4 words (two stages: byte→hw, hw→word)
                    VUPKLSB(VRREG(v0), VRREG(v1));
                    VUPKLSH(VRREG(v0), VRREG(v0));
                    break;
                case 0x22:
                    INST_NAME("PMOVSXBQ Gx, Ex"); // SSE4
                    nextop = F8;
                    GETEX16(v1, 0, 0);
                    GETGX_empty(v0);
                    // Sign-extend low 2 bytes → 2 qwords (three stages)
                    VUPKLSB(VRREG(v0), VRREG(v1));
                    VUPKLSH(VRREG(v0), VRREG(v0));
                    VUPKLSW(VRREG(v0), VRREG(v0));
                    break;
                case 0x23:
                    INST_NAME("PMOVSXWD Gx, Ex"); // SSE4
                    nextop = F8;
                    GETEX64(v1, 0, 0);
                    GETGX_empty(v0);
                    VUPKLSH(VRREG(v0), VRREG(v1));
                    break;
                case 0x24:
                    INST_NAME("PMOVSXWQ Gx, Ex"); // SSE4
                    nextop = F8;
                    GETEX32(v1, 0, 0);
                    GETGX_empty(v0);
                    VUPKLSH(VRREG(v0), VRREG(v1));
                    VUPKLSW(VRREG(v0), VRREG(v0));
                    break;
                case 0x25:
                    INST_NAME("PMOVSXDQ Gx, Ex"); // SSE4
                    nextop = F8;
                    GETEX64(v1, 0, 0);
                    GETGX_empty(v0);
                    VUPKLSW(VRREG(v0), VRREG(v1));
                    break;
                case 0x28:
                    INST_NAME("PMULDQ Gx, Ex"); // SSE4
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 0);
                    // Multiply signed dwords at even positions → qwords
                    VMULOSW(VRREG(v0), VRREG(v0), VRREG(v1));
                    break;
                case 0x29:
                    INST_NAME("PCMPEQQ Gx, Ex"); // SSE4
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 0);
                    VCMPEQUD(VRREG(v0), VRREG(v0), VRREG(v1));
                    break;
                case 0x2A:
                    INST_NAME("MOVNTDQA Gx, Ex"); // SSE4
                    nextop = F8;
                    if (MODREG) {
                        v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                        GETGX_empty(v0);
                        XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                    } else {
                        GETGX_empty(v0);
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DQ_DISP, 0);
                        LXV(VSXREG(v0), fixedaddress, ed);
                    }
                    break;
                case 0x2B:
                    INST_NAME("PACKUSDW Gx, Ex"); // SSE4
                    nextop = F8;
                    GETEX(v1, 0, 0);
                    GETGX(v0, 1);
                    // Pack signed dwords → unsigned saturated halfwords
                    // Clamp negative to 0, then pack with unsigned saturation
                    d0 = fpu_get_scratch(dyn);
                    d1 = fpu_get_scratch(dyn);
                    VSPLTISW(VRREG(d0), 0);
                    // Clamp Gx: max(Gx, 0)
                    VMAXSW(VRREG(d1), VRREG(v0), VRREG(d0));
                    if (v0 == v1) {
                        // Same register: pack d1 with itself
                        VPKSWUS(VRREG(v0), VRREG(d1), VRREG(d1));
                    } else {
                        // Clamp Ex: max(Ex, 0)
                        VMAXSW(VRREG(d0), VRREG(v1), VRREG(d0));
                        // VPKSWUS packs signed words → unsigned saturated halfwords
                        // On LE: VPKSWUS(vd, va, vb) → low hw from vb, high hw from va
                        VPKSWUS(VRREG(v0), VRREG(d0), VRREG(d1));
                    }
                    break;
                case 0x30:
                    INST_NAME("PMOVZXBW Gx, Ex"); // SSE4
                    nextop = F8;
                    GETEX(v1, 0, 0);
                    GETGX_empty(v0);
                    // Zero-extend low 8 bytes → 8 halfwords
                    // VMRGLB with zero vector: interleave bytes with zeros
                    d0 = fpu_get_scratch(dyn);
                    XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));
                    VMRGLB(VRREG(v0), VRREG(d0), VRREG(v1));
                    break;
                case 0x31:
                    INST_NAME("PMOVZXBD Gx, Ex"); // SSE4
                    nextop = F8;
                    GETEX32(v1, 0, 0);
                    GETGX_empty(v0);
                    d0 = fpu_get_scratch(dyn);
                    XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));
                    VMRGLB(VRREG(v0), VRREG(d0), VRREG(v1));  // bytes→halfwords
                    VMRGLH(VRREG(v0), VRREG(d0), VRREG(v0));  // halfwords→words
                    break;
                case 0x32:
                    INST_NAME("PMOVZXBQ Gx, Ex"); // SSE4
                    nextop = F8;
                    GETEX16(v1, 0, 0);
                    GETGX_empty(v0);
                    d0 = fpu_get_scratch(dyn);
                    XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));
                    VMRGLB(VRREG(v0), VRREG(d0), VRREG(v1));  // bytes→halfwords
                    VMRGLH(VRREG(v0), VRREG(d0), VRREG(v0));  // halfwords→words
                    VMRGLW(VRREG(v0), VRREG(d0), VRREG(v0));  // words→doublewords
                    break;
                case 0x33:
                    INST_NAME("PMOVZXWD Gx, Ex"); // SSE4
                    nextop = F8;
                    GETEX64(v1, 0, 0);
                    GETGX_empty(v0);
                    d0 = fpu_get_scratch(dyn);
                    XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));
                    VMRGLH(VRREG(v0), VRREG(d0), VRREG(v1));
                    break;
                case 0x34:
                    INST_NAME("PMOVZXWQ Gx, Ex"); // SSE4
                    nextop = F8;
                    GETEX32(v1, 0, 0);
                    GETGX_empty(v0);
                    d0 = fpu_get_scratch(dyn);
                    XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));
                    VMRGLH(VRREG(v0), VRREG(d0), VRREG(v1));
                    VMRGLW(VRREG(v0), VRREG(d0), VRREG(v0));
                    break;
                case 0x35:
                    INST_NAME("PMOVZXDQ Gx, Ex"); // SSE4
                    nextop = F8;
                    GETEX64(v1, 0, 0);
                    GETGX_empty(v0);
                    d0 = fpu_get_scratch(dyn);
                    XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));
                    VMRGLW(VRREG(v0), VRREG(d0), VRREG(v1));
                    break;
                case 0x37:
                    INST_NAME("PCMPGTQ Gx, Ex"); // SSE4.2
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 0);
                    VCMPGTSD(VRREG(v0), VRREG(v0), VRREG(v1));
                    break;
                case 0x38:
                    INST_NAME("PMINSB Gx, Ex"); // SSE4
                    nextop = F8;
                    GETEX(v1, 0, 0);
                    GETGX(v0, 1);
                    VMINSB(VRREG(v0), VRREG(v0), VRREG(v1));
                    break;
                case 0x39:
                    INST_NAME("PMINSD Gx, Ex"); // SSE4
                    nextop = F8;
                    GETEX(v1, 0, 0);
                    GETGX(v0, 1);
                    VMINSW(VRREG(v0), VRREG(v0), VRREG(v1));
                    break;
                case 0x3A:
                    INST_NAME("PMINUW Gx, Ex"); // SSE4
                    nextop = F8;
                    GETEX(v1, 0, 0);
                    GETGX(v0, 1);
                    VMINUH(VRREG(v0), VRREG(v0), VRREG(v1));
                    break;
                case 0x3B:
                    INST_NAME("PMINUD Gx, Ex"); // SSE4
                    nextop = F8;
                    GETEX(v1, 0, 0);
                    GETGX(v0, 1);
                    VMINUW(VRREG(v0), VRREG(v0), VRREG(v1));
                    break;
                case 0x3C:
                    INST_NAME("PMAXSB Gx, Ex"); // SSE4
                    nextop = F8;
                    GETEX(v1, 0, 0);
                    GETGX(v0, 1);
                    VMAXSB(VRREG(v0), VRREG(v0), VRREG(v1));
                    break;
                case 0x3D:
                    INST_NAME("PMAXSD Gx, Ex"); // SSE4
                    nextop = F8;
                    GETEX(v1, 0, 0);
                    GETGX(v0, 1);
                    VMAXSW(VRREG(v0), VRREG(v0), VRREG(v1));
                    break;
                case 0x3E:
                    INST_NAME("PMAXUW Gx, Ex"); // SSE4
                    nextop = F8;
                    GETEX(v1, 0, 0);
                    GETGX(v0, 1);
                    VMAXUH(VRREG(v0), VRREG(v0), VRREG(v1));
                    break;
                case 0x3F:
                    INST_NAME("PMAXUD Gx, Ex"); // SSE4
                    nextop = F8;
                    GETEX(v1, 0, 0);
                    GETGX(v0, 1);
                    VMAXUW(VRREG(v0), VRREG(v0), VRREG(v1));
                    break;
                case 0x40:
                    INST_NAME("PMULLD Gx, Ex"); // SSE4
                    nextop = F8;
                    GETEX(v1, 0, 0);
                    GETGX(v0, 1);
                    // Multiply low dwords (modulo)
                    VMULUWM(VRREG(v0), VRREG(v0), VRREG(v1));
                    break;
                case 0xDB:
                    INST_NAME("AESIMC Gx, Ex"); // AES-NI
                    nextop = F8;
                    GETEX(v1, 0, 0);
                    GETGX_empty(v0);
                    if (v0 != v1) {
                        XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                    }
                    sse_forget_reg(dyn, ninst, gd);
                    MOV32w(x1, gd);
                    CALL(const_native_aesimc, -1, x1, 0);
                    break;
                case 0xDC:
                    INST_NAME("AESENC Gx, Ex"); // AES-NI
                    nextop = F8;
                    GETG;
                    GETEX(v1, 0, 0);
                    if (MODREG && (gd == (nextop & 7) + (rex.b << 3))) {
                        d0 = fpu_get_scratch(dyn);
                        XXLOR(VSXREG(d0), VSXREG(v1), VSXREG(v1));
                    } else
                        d0 = -1;
                    sse_forget_reg(dyn, ninst, gd);
                    MOV32w(x1, gd);
                    CALL(const_native_aese, -1, x1, 0);
                    GETGX(v0, 1);
                    XXLXOR(VSXREG(v0), VSXREG(v0), (d0 != -1) ? VSXREG(d0) : VSXREG(v1));
                    break;
                case 0xDD:
                    INST_NAME("AESENCLAST Gx, Ex"); // AES-NI
                    nextop = F8;
                    GETG;
                    GETEX(v1, 0, 0);
                    if (MODREG && (gd == (nextop & 7) + (rex.b << 3))) {
                        d0 = fpu_get_scratch(dyn);
                        XXLOR(VSXREG(d0), VSXREG(v1), VSXREG(v1));
                    } else
                        d0 = -1;
                    sse_forget_reg(dyn, ninst, gd);
                    MOV32w(x1, gd);
                    CALL(const_native_aeselast, -1, x1, 0);
                    GETGX(v0, 1);
                    XXLXOR(VSXREG(v0), VSXREG(v0), (d0 != -1) ? VSXREG(d0) : VSXREG(v1));
                    break;
                case 0xDE:
                    INST_NAME("AESDEC Gx, Ex"); // AES-NI
                    nextop = F8;
                    GETG;
                    GETEX(v1, 0, 0);
                    if (MODREG && (gd == (nextop & 7) + (rex.b << 3))) {
                        d0 = fpu_get_scratch(dyn);
                        XXLOR(VSXREG(d0), VSXREG(v1), VSXREG(v1));
                    } else
                        d0 = -1;
                    sse_forget_reg(dyn, ninst, gd);
                    MOV32w(x1, gd);
                    CALL(const_native_aesd, -1, x1, 0);
                    GETGX(v0, 1);
                    XXLXOR(VSXREG(v0), VSXREG(v0), (d0 != -1) ? VSXREG(d0) : VSXREG(v1));
                    break;
                case 0xDF:
                    INST_NAME("AESDECLAST Gx, Ex"); // AES-NI
                    nextop = F8;
                    GETG;
                    GETEX(v1, 0, 0);
                    if (MODREG && (gd == (nextop & 7) + (rex.b << 3))) {
                        d0 = fpu_get_scratch(dyn);
                        XXLOR(VSXREG(d0), VSXREG(v1), VSXREG(v1));
                    } else
                        d0 = -1;
                    sse_forget_reg(dyn, ninst, gd);
                    MOV32w(x1, gd);
                    CALL(const_native_aesdlast, -1, x1, 0);
                    GETGX(v0, 1);
                    XXLXOR(VSXREG(v0), VSXREG(v0), (d0 != -1) ? VSXREG(d0) : VSXREG(v1));
                    break;
                case 0xF0:
                    INST_NAME("MOVBE Gw, Ew");
                    nextop = F8;
                    GETGD;
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, DS_DISP, 0);
                    // Load and byte-swap 16 bits using LHBRX (indexed form)
                    if (fixedaddress) {
                        ADDI(x4, ed, fixedaddress);
                        LHBRX(x1, 0, x4);
                    } else {
                        LHBRX(x1, 0, ed);
                    }
                    // Insert into lower 16 bits of Gd
                    RLWIMI(gd, x1, 0, 16, 31);
                    break;
                case 0xF1:
                    INST_NAME("MOVBE Ew, Gw");
                    nextop = F8;
                    GETGD;
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, x2, &fixedaddress, rex, NULL, DS_DISP, 0);
                    // Store byte-swapped lower 16 bits of Gd using STHBRX (indexed form)
                    if (fixedaddress) {
                        ADDI(x4, wback, fixedaddress);
                        STHBRX(gd, 0, x4);
                    } else {
                        STHBRX(gd, 0, wback);
                    }
                    SMWRITE();
                    break;
                case 0xF6:
                    INST_NAME("ADCX Gd, Ed");
                    nextop = F8;
                    READFLAGS(X_CF);
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    GETED(0);
                    GETGD;
                    // Extract CF to x3 (bit 0)
                    RLWINM(x3, xFlags, 32-F_CF, 31, 31); // isolate CF → bit 0
                    IFX(X_CF) {
                        // Use PPC carry chain: ADDIC sets CA = old_CF, ADDE adds with CA
                        if(rex.w) {
                            ADDIC(x4, x3, -1);            // CA = (x3 >= 1) = old_CF
                            ADDE(gd, gd, ed);              // gd = gd + ed + CA, CA = new carry
                            LI(x5, 0);
                            ADDZE(x5, x5);                 // x5 = 0 + CA = new carry (0 or 1)
                        } else {
                            // 32-bit: zero-extend operands, use ADDE, check for 32-bit overflow
                            RLWINM(x4, gd, 0, 0, 31);     // zero-extend gd to 64-bit
                            RLWINM(x5, ed, 0, 0, 31);     // zero-extend ed to 64-bit
                            ADD(gd, x4, x5);               // 64-bit add (can't overflow 64 bits)
                            ADD(gd, gd, x3);               // add old CF
                            SRDI(x5, gd, 32);              // x5 = bit 32 = carry out of 32-bit add
                            RLWINM(gd, gd, 0, 0, 31);     // truncate result to 32 bits
                        }
                        // Set CF in flags
                        RLWIMI(xFlags, x5, F_CF, 31-F_CF, 31-F_CF);
                    } else {
                        ADDxw(gd, gd, ed);
                        ADDxw(gd, gd, x3);
                        if (!rex.w) RLWINM(gd, gd, 0, 0, 31);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0x3A: // these are some more SSSE3+ opcodes
            opcode = F8;
            switch (opcode) {
                case 0x08:
                    INST_NAME("ROUNDPS Gx, Ex, Ib");
                    nextop = F8;
                    GETEX(v1, 0, 1);
                    GETGX_empty(v0);
                    u8 = F8;
                    if (u8 & 4) {
                        // Use current rounding mode
                        XVRSPIC(VSXREG(v0), VSXREG(v1));
                    } else {
                        switch (u8 & 3) {
                            case 0:
                                // PPC XVRSPI does "ties away from zero", x86 wants "ties to even"
                                // Use XVRSPIC with FPSCR.RN forced to 0 (nearest even)
                                MFFS(SCRATCH0);             // save FPSCR
                                MTFSFI(7, 0);               // set RN = 0 (nearest even)
                                XVRSPIC(VSXREG(v0), VSXREG(v1));
                                MTFSF(0x01, SCRATCH0);      // restore FPSCR field 7 (RN)
                                break;
                            case 1: XVRSPIM(VSXREG(v0), VSXREG(v1)); break;  // floor
                            case 2: XVRSPIP(VSXREG(v0), VSXREG(v1)); break;  // ceil
                            case 3: XVRSPIZ(VSXREG(v0), VSXREG(v1)); break;  // trunc
                        }
                    }
                    break;
                case 0x09:
                    INST_NAME("ROUNDPD Gx, Ex, Ib");
                    nextop = F8;
                    GETEX(v1, 0, 1);
                    GETGX_empty(v0);
                    u8 = F8;
                    if (u8 & 4) {
                        XVRDPIC(VSXREG(v0), VSXREG(v1));
                    } else {
                        switch (u8 & 3) {
                            case 0:
                                MFFS(SCRATCH0);
                                MTFSFI(7, 0);
                                XVRDPIC(VSXREG(v0), VSXREG(v1));
                                MTFSF(0x01, SCRATCH0);
                                break;
                            case 1: XVRDPIM(VSXREG(v0), VSXREG(v1)); break;
                            case 2: XVRDPIP(VSXREG(v0), VSXREG(v1)); break;
                            case 3: XVRDPIZ(VSXREG(v0), VSXREG(v1)); break;
                        }
                    }
                    break;
                case 0x0A:
                    INST_NAME("ROUNDSS Gx, Ex, Ib");
                    nextop = F8;
                    GETGX(v0, 1);
                    d0 = fpu_get_scratch(dyn);
                    if (MODREG) {
                        v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                        u8 = F8;
                        // Round all 4 singles in v1, take element 0
                        if (u8 & 4) {
                            XVRSPIC(VSXREG(d0), VSXREG(v1));
                        } else {
                            switch (u8 & 3) {
                                case 0:
                                    MFFS(SCRATCH0);
                                    MTFSFI(7, 0);
                                    XVRSPIC(VSXREG(d0), VSXREG(v1));
                                    MTFSF(0x01, SCRATCH0);
                                    break;
                                case 1: XVRSPIM(VSXREG(d0), VSXREG(v1)); break;
                                case 2: XVRSPIP(VSXREG(d0), VSXREG(v1)); break;
                                case 3: XVRSPIZ(VSXREG(d0), VSXREG(v1)); break;
                            }
                        }
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, DS_DISP, 1);
                        v1 = fpu_get_scratch(dyn);
                        LWZ(x4, fixedaddress, ed);
                        SLDI(x4, x4, 32);
                        MTVSRD(VSXREG(v1), x4);
                        XSCVSPDPN(VSXREG(v1), VSXREG(v1));
                        u8 = F8;
                        // Scalar double round
                        if (u8 & 4) {
                            XSRDPIC(VSXREG(d0), VSXREG(v1));
                        } else {
                            switch (u8 & 3) {
                                case 0:
                                    MFFS(SCRATCH0);
                                    MTFSFI(7, 0);
                                    XSRDPIC(VSXREG(d0), VSXREG(v1));
                                    MTFSF(0x01, SCRATCH0);
                                    break;
                                case 1: XSRDPIM(VSXREG(d0), VSXREG(v1)); break;
                                case 2: XSRDPIP(VSXREG(d0), VSXREG(v1)); break;
                                case 3: XSRDPIZ(VSXREG(d0), VSXREG(v1)); break;
                            }
                        }
                        // Convert rounded double back to single; result in BE word 0 (byte offset 0)
                        XSCVDPSP(VSXREG(d0), VSXREG(d0));
                    }
                    // Extract LE word 0 from d0 and insert into v0 LE word 0
                    // For register case: rounded single at BE byte offset 12
                    // For memory case: single at BE byte offset 0 (from XSCVDPSP)
                    if (MODREG) {
                        VEXTRACTUW(VRREG(d0), VRREG(d0), 12);
                    } else {
                        VEXTRACTUW(VRREG(d0), VRREG(d0), 0);
                    }
                    // Now the single is in d0 bits 96:127; insert into v0 at byte offset 12 (LE word 0)
                    VINSERTW(VRREG(v0), VRREG(d0), 12);
                    break;
                case 0x0B:
                    INST_NAME("ROUNDSD Gx, Ex, Ib");
                    nextop = F8;
                    GETGX(v0, 1);
                    d0 = fpu_get_scratch(dyn);
                    if (MODREG) {
                        v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                        u8 = F8;
                        // Round both doubles in v1, take element 0
                        if (u8 & 4) {
                            XVRDPIC(VSXREG(d0), VSXREG(v1));
                        } else {
                            switch (u8 & 3) {
                                case 0:
                                    MFFS(SCRATCH0);
                                    MTFSFI(7, 0);
                                    XVRDPIC(VSXREG(d0), VSXREG(v1));
                                    MTFSF(0x01, SCRATCH0);
                                    break;
                                case 1: XVRDPIM(VSXREG(d0), VSXREG(v1)); break;
                                case 2: XVRDPIP(VSXREG(d0), VSXREG(v1)); break;
                                case 3: XVRDPIZ(VSXREG(d0), VSXREG(v1)); break;
                            }
                        }
                        // Merge: keep v0 LE dword 1, replace LE dword 0 with d0 LE dword 0
                        // LE dword 0 = BE dword 1 = bits 64:127
                        // LE dword 1 = BE dword 0 = bits 0:63
                        // XXPERMDI(v0, v0, d0, 0b01): bits 0:63=v0[0:63], bits 64:127=d0[64:127]
                        XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d0), 0b01);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, DS_DISP, 1);
                        v1 = fpu_get_scratch(dyn);
                        LD(x4, fixedaddress, ed);
                        MTVSRD(VSXREG(v1), x4);
                        u8 = F8;
                        // Scalar double round (operates on bits 0:63)
                        if (u8 & 4) {
                            XSRDPIC(VSXREG(d0), VSXREG(v1));
                        } else {
                            switch (u8 & 3) {
                                case 0:
                                    MFFS(SCRATCH0);
                                    MTFSFI(7, 0);
                                    XSRDPIC(VSXREG(d0), VSXREG(v1));
                                    MTFSF(0x01, SCRATCH0);
                                    break;
                                case 1: XSRDPIM(VSXREG(d0), VSXREG(v1)); break;
                                case 2: XSRDPIP(VSXREG(d0), VSXREG(v1)); break;
                                case 3: XSRDPIZ(VSXREG(d0), VSXREG(v1)); break;
                            }
                        }
                        // Result in d0 bits 0:63. Insert into v0 LE dword 0 (= BE dword 1 = bits 64:127)
                        // XXPERMDI(v0, v0, d0, 0b00): bits 0:63=v0[0:63], bits 64:127=d0[0:63]
                        XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d0), 0b00);
                    }
                    break;
                case 0x14:
                    INST_NAME("PEXTRB Ed, Gx, Ib");
                    nextop = F8;
                    GETGX(v0, 0);
                    if (MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        u8 = (F8) & 15;
                        d0 = fpu_get_scratch(dyn);
                        // x86 byte index u8 = BE byte (15-u8)
                        // VEXTRACTUB places result in ISA dw0, use MFVSRD to read
                        VEXTRACTUB(VRREG(d0), VRREG(v0), 15 - u8);
                        MFVSRD(ed, VSXREG(d0));
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x4, &fixedaddress, rex, NULL, DS_DISP, 1);
                        u8 = (F8) & 15;
                        d0 = fpu_get_scratch(dyn);
                        VEXTRACTUB(VRREG(d0), VRREG(v0), 15 - u8);
                        MFVSRD(x1, VSXREG(d0));
                        STB(x1, wback, fixedaddress);
                        SMWRITE2();
                    }
                    break;
                case 0x15:
                    INST_NAME("PEXTRW Ed, Gx, Ib");
                    nextop = F8;
                    GETGX(v0, 0);
                    if (MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        u8 = (F8) & 7;
                        d0 = fpu_get_scratch(dyn);
                        // x86 halfword index u8 = BE hw (7-u8), byte offset (7-u8)*2
                        VEXTRACTUH(VRREG(d0), VRREG(v0), (7 - u8) * 2);
                        MFVSRD(ed, VSXREG(d0));
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x4, &fixedaddress, rex, NULL, DS_DISP, 1);
                        u8 = (F8) & 7;
                        d0 = fpu_get_scratch(dyn);
                        VEXTRACTUH(VRREG(d0), VRREG(v0), (7 - u8) * 2);
                        MFVSRD(x1, VSXREG(d0));
                        STH(x1, wback, fixedaddress);
                        SMWRITE2();
                    }
                    break;
                case 0x16:
                    if (rex.w) {
                        INST_NAME("PEXTRQ Ed, Gx, Ib");
                    } else {
                        INST_NAME("PEXTRD Ed, Gx, Ib");
                    }
                    nextop = F8;
                    GETGX(v0, 0);
                    if (MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        u8 = F8;
                        if (rex.w) {
                            // x86 qword index: q[0]=ISA dw1 (MFVSRLD), q[1]=ISA dw0 (MFVSRD)
                            if (u8 & 1)
                                MFVSRD(ed, VSXREG(v0));
                            else
                                MFVSRLD(ed, VSXREG(v0));
                        } else {
                            // VEXTRACTUW places result in ISA dw0 (upper 64 bits on LE).
                            // Use MFVSRD to read it (not MFVSRLD which reads ISA dw1).
                            // x86 dword index (u8 & 3): LE word i = BE word (3-i), byte offset (3-i)*4
                            d0 = fpu_get_scratch(dyn);
                            VEXTRACTUW(VRREG(d0), VRREG(v0), (3 - (u8 & 3)) * 4);
                            MFVSRD(ed, VSXREG(d0));
                        }
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x3, x5, &fixedaddress, rex, NULL, DS_DISP, 1);
                        u8 = F8;
                        if (rex.w) {
                            if (u8 & 1)
                                MFVSRD(x1, VSXREG(v0));
                            else
                                MFVSRLD(x1, VSXREG(v0));
                            STD(x1, fixedaddress, ed);
                        } else {
                            d0 = fpu_get_scratch(dyn);
                            VEXTRACTUW(VRREG(d0), VRREG(v0), (3 - (u8 & 3)) * 4);
                            MFVSRD(x1, VSXREG(d0));
                            STW(x1, fixedaddress, ed);
                        }
                        SMWRITE2();
                    }
                    break;
                case 0x0C:
                    INST_NAME("BLENDPS Gx, Ex, Ib");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 1);
                    u8 = F8 & 0xF;
                    if (u8 == 0x0) {
                        // no blend, keep Gx
                    } else if (u8 == 0xF) {
                        // full copy
                        XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                    } else {
                        d0 = fpu_get_scratch(dyn);
                        // Check doubleword-aligned pairs first
                        if ((u8 & 0x3) == 0x3) {
                            // x86 words 0,1 → LE words 0,1 → BE dword 1 (bits 64:127)
                            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(v1), 0b01);
                            u8 &= ~0x3;
                        }
                        if ((u8 & 0xC) == 0xC) {
                            // x86 words 2,3 → LE words 2,3 → ISA dw0 (bits 0:63)
                            // Replace ISA dw0 with v1's ISA dw0, keep v0's ISA dw1
                            XXPERMDI(VSXREG(v0), VSXREG(v1), VSXREG(v0), 0b01);
                            u8 &= ~0xC;
                        }
                        // Handle remaining individual words
                        for (int i = 0; i < 4; ++i) {
                            if (u8 & (1 << i)) {
                                // x86 word i = LE word i = BE word (3-i) = byte offset (3-i)*4
                                int boff = (3 - i) * 4;
                                VEXTRACTUW(VRREG(d0), VRREG(v1), boff);
                                VINSERTW(VRREG(v0), VRREG(d0), boff);
                            }
                        }
                    }
                    break;
                case 0x0D:
                    INST_NAME("BLENDPD Gx, Ex, Ib");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 1);
                    u8 = F8 & 0x3;
                    if (u8 == 0x1) {
                        // Replace LE dword 0 (= BE dword 1): keep v0 high, take v1 low
                        XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(v1), 0b01);
                    } else if (u8 == 0x2) {
                        // Replace LE dword 1 (= ISA dw0): take v1 high, keep v0 low
                        XXPERMDI(VSXREG(v0), VSXREG(v1), VSXREG(v0), 0b01);
                    } else if (u8 == 0x3) {
                        XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                    }
                    // u8 == 0: no blend
                    break;
                case 0x0E:
                    INST_NAME("PBLENDW Gx, Ex, Ib");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 1);
                    u8 = F8;
                    if (v0 != v1) {
                        if (u8 == 0xFF) {
                            XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                        } else if (u8 != 0x00) {
                            d0 = fpu_get_scratch(dyn);
                            // Check doubleword-aligned groups (4 halfwords each)
                            if ((u8 & 0x0F) == 0x0F) {
                                // x86 halfwords 0-3 → BE dword 1
                                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(v1), 0b01);
                                u8 &= ~0x0F;
                            }
                            if ((u8 & 0xF0) == 0xF0) {
                                // x86 halfwords 4-7 → ISA dw0: replace ISA dw0 with v1's ISA dw0
                                XXPERMDI(VSXREG(v0), VSXREG(v1), VSXREG(v0), 0b01);
                                u8 &= ~0xF0;
                            }
                            // Check word-aligned pairs (2 halfwords each)
                            if ((u8 & 0x03) == 0x03) {
                                // x86 hw 0,1 → BE word 3 → byte offset 12
                                VEXTRACTUW(VRREG(d0), VRREG(v1), 12);
                                VINSERTW(VRREG(v0), VRREG(d0), 12);
                                u8 &= ~0x03;
                            }
                            if ((u8 & 0x0C) == 0x0C) {
                                // x86 hw 2,3 → BE word 2 → byte offset 8
                                VEXTRACTUW(VRREG(d0), VRREG(v1), 8);
                                VINSERTW(VRREG(v0), VRREG(d0), 8);
                                u8 &= ~0x0C;
                            }
                            if ((u8 & 0x30) == 0x30) {
                                // x86 hw 4,5 → BE word 1 → byte offset 4
                                VEXTRACTUW(VRREG(d0), VRREG(v1), 4);
                                VINSERTW(VRREG(v0), VRREG(d0), 4);
                                u8 &= ~0x30;
                            }
                            if ((u8 & 0xC0) == 0xC0) {
                                // x86 hw 6,7 → BE word 0 → byte offset 0
                                VEXTRACTUW(VRREG(d0), VRREG(v1), 0);
                                VINSERTW(VRREG(v0), VRREG(d0), 0);
                                u8 &= ~0xC0;
                            }
                            // Handle remaining individual halfwords
                            for (int i = 0; i < 8; ++i) {
                                if (u8 & (1 << i)) {
                                    // x86 halfword i = LE hw i = BE hw (7-i) = byte offset (7-i)*2
                                    int boff = (7 - i) * 2;
                                    VEXTRACTUH(VRREG(d0), VRREG(v1), boff);
                                    VINSERTH(VRREG(v0), VRREG(d0), boff);
                                }
                            }
                        }
                    }
                    break;
                case 0x0F:
                    INST_NAME("PALIGNR Gx, Ex, Ib");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 1);
                    u8 = F8;
                    if (u8 > 31) {
                        // Result is all zeros
                        XXLXOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    } else if (u8 > 15) {
                        // Shift Gx right by (u8-16) bytes
                        if (u8 == 16) {
                            // Result is just Gx, already in v0
                        } else {
                            d0 = fpu_get_scratch(dyn);
                            XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));
                            // VSLDOI(v0, Gx, zeros, 16-(u8-16)) = VSLDOI(v0, v0, d0, 32-u8)
                            VSLDOI(VRREG(d0), VRREG(v0), VRREG(d0), 32 - u8);
                            XXLOR(VSXREG(v0), VSXREG(d0), VSXREG(d0));
                        }
                    } else if (u8 == 0) {
                        // Result is Ex
                        XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                    } else {
                        // General case: concat Gx(high):Ex(low), shift right by u8 bytes
                        // In BE register layout: VSLDOI(result, Gx, Ex, 16-u8)
                        d0 = fpu_get_scratch(dyn);
                        VSLDOI(VRREG(d0), VRREG(v0), VRREG(v1), 16 - u8);
                        XXLOR(VSXREG(v0), VSXREG(d0), VSXREG(d0));
                    }
                    break;
                case 0x17:
                    INST_NAME("EXTRACTPS Ew, Gx, Ib");
                    nextop = F8;
                    GETGX(v0, 0);
                    if (MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        u8 = F8 & 0x3;
                        d0 = fpu_get_scratch(dyn);
                        // Same as PEXTRD: x86 word index u8, byte offset (3-u8)*4
                        VEXTRACTUW(VRREG(d0), VRREG(v0), (3 - u8) * 4);
                        MFVSRD(ed, VSXREG(d0));
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x3, x5, &fixedaddress, rex, NULL, DS_DISP, 1);
                        u8 = F8 & 0x3;
                        d0 = fpu_get_scratch(dyn);
                        VEXTRACTUW(VRREG(d0), VRREG(v0), (3 - u8) * 4);
                        MFVSRD(x1, VSXREG(d0));
                        STW(x1, fixedaddress, ed);
                        SMWRITE2();
                    }
                    break;
                case 0x20:
                    INST_NAME("PINSRB Gx, ED, Ib");
                    nextop = F8;
                    GETGX(v0, 1);
                    if (MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        wback = 0;
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 1);
                        LBZ(x1, wback, fixedaddress);
                        ed = x1;
                    }
                    u8 = F8;
                    d0 = fpu_get_scratch(dyn);
                    // Move GPR value into vector scratch via MTVSRDD (value goes into bits 0:63)
                    MTVSRDD(VSXREG(d0), ed, ed);
                    // x86 byte index (u8 & 0xF) → BE byte (15 - index)
                    VINSERTB(VRREG(v0), VRREG(d0), 15 - (u8 & 0xF));
                    break;
                case 0x21:
                    INST_NAME("INSERTPS Gx, Ex, Ib");
                    nextop = F8;
                    GETGX(v0, 1);
                    d0 = fpu_get_scratch(dyn);
                    if (MODREG) {
                        u8 = F8;
                        v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                        // Source: x86 word (u8>>6)&3 from Ex
                        // Dest: x86 word (u8>>4)&3 in Gx
                        int src_idx = (u8 >> 6) & 3;
                        int dst_idx = (u8 >> 4) & 3;
                        // Extract source word: x86 word i = BE byte offset (3-i)*4
                        VEXTRACTUW(VRREG(d0), VRREG(v1), (3 - src_idx) * 4);
                        // Insert into dest word: byte offset (3-dst_idx)*4
                        VINSERTW(VRREG(v0), VRREG(d0), (3 - dst_idx) * 4);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x3, x5, &fixedaddress, rex, NULL, NO_DISP, 1);
                        u8 = F8;
                        // Memory: load single float, source index is always 0
                        int dst_idx = (u8 >> 4) & 3;
                        LWZ(x4, fixedaddress, ed);
                        SLDI(x4, x4, 32);
                        MTVSRD(VSXREG(d0), x4);
                        XSCVSPDPN(VSXREG(d0), VSXREG(d0));
                        // Converts single→double into VSX dw0
                        // Convert back to single: XSCVDPSP puts single in bits 0:31 (BE word 0, byte offset 0)
                        XSCVDPSP(VSXREG(d0), VSXREG(d0));
                        // Extract from BE byte offset 0 (word 0)
                        VEXTRACTUW(VRREG(d0), VRREG(d0), 0);
                        // Insert at dest word byte offset (3-dst_idx)*4
                        VINSERTW(VRREG(v0), VRREG(d0), (3 - dst_idx) * 4);
                    }
                    // Apply zmask: zero out selected destination words
                    {
                        uint8_t zmask = u8 & 0xF;
                        if (zmask) {
                            d0 = fpu_get_scratch(dyn);
                            XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));
                            for (int i = 0; i < 4; ++i) {
                                if (zmask & (1 << i)) {
                                    // Zero x86 word i = BE byte offset (3-i)*4
                                    VEXTRACTUW(VRREG(d0), VRREG(d0), 0); // d0 is zero, extract zero
                                    VINSERTW(VRREG(v0), VRREG(d0), (3 - i) * 4);
                                }
                            }
                        }
                    }
                    break;
                case 0x22:
                    INST_NAME("PINSRD Gx, ED, Ib");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETED(1);
                    u8 = F8;
                    d0 = fpu_get_scratch(dyn);
                    if (rex.w) {
                        // PINSRQ: insert 64-bit value
                        // Move GPR into vector via MTVSRDD
                        MTVSRDD(VSXREG(d0), ed, ed);
                        // x86 qword index (u8 & 1) → BE dword (1 - index)
                        VINSERTD(VRREG(v0), VRREG(d0), (1 - (u8 & 1)) * 8);
                    } else {
                        // PINSRD: insert 32-bit value
                        // Move GPR into vector via MTVSRDD (value in both halves)
                        MTVSRDD(VSXREG(d0), ed, ed);
                        // On PPC64LE, MTVSRDD puts ed in both dw0 and dw1. The low 32 bits
                        // of the 64-bit value are at byte offset 4 (dw0) and 12 (dw1).
                        // VEXTRACTUW byte 4 extracts the correct 32-bit value.
                        VEXTRACTUW(VRREG(d0), VRREG(d0), 4);
                        // x86 dword index (u8 & 3) → BE byte offset (3 - index)*4
                        VINSERTW(VRREG(v0), VRREG(d0), (3 - (u8 & 3)) * 4);
                    }
                    break;
                case 0x40:
                    INST_NAME("DPPS Gx, Ex, Ib");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 1);
                    u8 = F8;
                    {
                        int d1 = fpu_get_scratch(dyn);
                        int d2 = fpu_get_scratch(dyn);
                        d0 = fpu_get_scratch(dyn);
                        // Multiply all 4 single lanes
                        XVMULSP(VSXREG(d0), VSXREG(v0), VSXREG(v1));
                        // Zero out lanes not selected by input mask (bits 7:4)
                        XXLXOR(VSXREG(d2), VSXREG(d2), VSXREG(d2));
                        for (int i = 0; i < 4; ++i) {
                            if (!(u8 & (1 << (4 + i)))) {
                                // Zero x86 word i = BE byte offset (3-i)*4
                                VINSERTW(VRREG(d0), VRREG(d2), (3 - i) * 4);
                            }
                        }
                        // Horizontal sum: swap adjacent pairs, add
                        // BE layout: [w0, w1, w2, w3] where w0=x86[3], w1=x86[2], w2=x86[1], w3=x86[0]
                        // VSLDOI by 4 bytes shifts left in BE: moves w1,w2,w3,0 → swap-like
                        VSLDOI(VRREG(d1), VRREG(d0), VRREG(d0), 4);
                        XVADDSP(VSXREG(d0), VSXREG(d0), VSXREG(d1));
                        // Now sum pairs: VSLDOI by 8 bytes
                        VSLDOI(VRREG(d1), VRREG(d0), VRREG(d0), 8);
                        XVADDSP(VSXREG(d0), VSXREG(d0), VSXREG(d1));
                        // Broadcast: replicate the sum to all 4 words
                        // The total sum is now in every word, but let's splat from BE word 0
                        // Actually after the two adds, the sum is in all words, so just use VSPLTW
                        VSPLTW(VRREG(v0), VRREG(d0), 0);
                        // Zero out lanes not selected by output mask (bits 3:0)
                        for (int i = 0; i < 4; ++i) {
                            if (!(u8 & (1 << i))) {
                                VINSERTW(VRREG(v0), VRREG(d2), (3 - i) * 4);
                            }
                        }
                    }
                    break;
                case 0x41:
                    INST_NAME("DPPD Gx, Ex, Ib");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 1);
                    u8 = F8;
                    {
                        int d1 = fpu_get_scratch(dyn);
                        d0 = fpu_get_scratch(dyn);
                        // Multiply both double lanes
                        XVMULDP(VSXREG(d0), VSXREG(v0), VSXREG(v1));
                        // Zero out lanes not selected by input mask (bits 5:4)
                        XXLXOR(VSXREG(d1), VSXREG(d1), VSXREG(d1));
                        for (int i = 0; i < 2; ++i) {
                            if (!(u8 & (1 << (4 + i)))) {
                                // Zero x86 dword i: LE dword i = BE dword (1-i)
                                // Use XXPERMDI to selectively zero
                                if (i == 0) {
                                    // Zero LE dword 0 = BE dword 1 (bits 64:127)
                                    XXPERMDI(VSXREG(d0), VSXREG(d0), VSXREG(d1), 0b01);
                                } else {
                                    // Zero LE dword 1 = BE dword 0 (bits 0:63)
                                    XXPERMDI(VSXREG(d0), VSXREG(d1), VSXREG(d0), 0b01);
                                }
                            }
                        }
                        // Horizontal sum: swap the two dwords and add
                        XXPERMDI(VSXREG(d1), VSXREG(d0), VSXREG(d0), 0b10);
                        XVADDDP(VSXREG(d0), VSXREG(d0), VSXREG(d1));
                        // d0 now has the sum in both dwords
                        // Apply output mask (bits 1:0)
                        XXLXOR(VSXREG(d1), VSXREG(d1), VSXREG(d1));
                        if ((u8 & 0x3) == 0x3) {
                            // Both output lanes enabled
                            XXLOR(VSXREG(v0), VSXREG(d0), VSXREG(d0));
                        } else if ((u8 & 0x3) == 0x0) {
                            // Both zero
                            XXLXOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                        } else if (u8 & 0x1) {
                            // Only LE dword 0 enabled, zero LE dword 1
                            XXPERMDI(VSXREG(v0), VSXREG(d1), VSXREG(d0), 0b01);
                        } else {
                            // Only LE dword 1 enabled, zero LE dword 0
                            XXPERMDI(VSXREG(v0), VSXREG(d0), VSXREG(d1), 0b10);
                        }
                    }
                    break;
                case 0x44:
                    INST_NAME("PCLMULQDQ Gx, Ex, Ib");
                    nextop = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, gd);
                    MOV32w(x1, gd); // gx
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        sse_forget_reg(dyn, ninst, ed);
                        MOV32w(x2, ed);
                        MOV32w(x3, 0); // p = NULL
                    } else {
                        MOV32w(x2, 0);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x3, x5, &fixedaddress, rex, NULL, NO_DISP, 1);
                        if (ed != x3) MV(x3, ed);
                    }
                    u8 = F8;
                    MOV32w(x4, u8);
                    CALL4(const_native_pclmul, -1, x1, x2, x3, x4);
                    break;
                case 0x61:
                    INST_NAME("PCMPESTRI Gx, Ex, Ib");
                    nextop = F8;
                    GETG;
                    u8 = geted_ib(dyn, addr, ninst, nextop);
                    SETFLAGS(X_ALL, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    if (gd > 7)
                        sse_reflect_reg(dyn, ninst, gd);
                    ADDI(x3, xEmu, offsetof(x64emu_t, xmm[gd]));
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        if (ed > 7)
                            sse_reflect_reg(dyn, ninst, ed);
                        ADDI(x1, xEmu, offsetof(x64emu_t, xmm[ed]));
                        ed = x1;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, NO_DISP, 1);
                    }
                    MV(x2, xRDX);
                    MV(x4, xRAX);
                    u8 = F8;
                    MOV32w(x5, u8);
                    CALL6(const_sse42_compare_string_explicit_len, x1, ed, x2, x3, x4, x5, 0);
                    ZEROUP(x1);
                    BNEZ_MARK(x1);
                    MOV32w(xRCX, (u8 & 1) ? 8 : 16);
                    B_NEXT_nocond;
                    MARK;
                    if (u8 & 0b1000000) {
                        CNTLZW(xRCX, x1);
                        LI(x2, 31);
                        SUBF(xRCX, xRCX, x2);
                    } else {
                        CNTTZW(xRCX, x1);
                    }
                    break;
                case 0x63:
                    INST_NAME("PCMPISTRI Gx, Ex, Ib");
                    SETFLAGS(X_ALL, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    nextop = F8;
                    GETG;
                    if (gd > 7) sse_reflect_reg(dyn, ninst, gd);
                    ADDI(x2, xEmu, offsetof(x64emu_t, xmm[gd]));
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        if (ed > 7) sse_reflect_reg(dyn, ninst, ed);
                        ADDI(x1, xEmu, offsetof(x64emu_t, xmm[ed]));
                        ed = x1;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, NO_DISP, 1);
                    }
                    u8 = F8;
                    MOV32w(x3, u8);
                    CALL4(const_sse42_compare_string_implicit_len, x1, ed, x2, x3, 0);
                    BNEZ_MARK(x1);
                    MOV32w(xRCX, (u8 & 1) ? 8 : 16);
                    B_NEXT_nocond;
                    MARK;
                    if (u8 & 0b1000000) {
                        CNTLZW(xRCX, x1);
                        LI(x2, 31);
                        SUBF(xRCX, xRCX, x2);
                    } else {
                        CNTTZW(xRCX, x1);
                    }
                    break;
                case 0xDF:
                    INST_NAME("AESKEYGENASSIST Gx, Ex, Ib"); // AES-NI
                    nextop = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, gd);
                    MOV32w(x1, gd); // gx
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        sse_forget_reg(dyn, ninst, ed);
                        MOV32w(x2, ed);
                        MOV32w(x3, 0); // p = NULL
                    } else {
                        MOV32w(x2, 0);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, NO_DISP, 1);
                        if (ed != x3) {
                            MV(x3, ed);
                        }
                    }
                    u8 = F8;
                    MOV32w(x4, u8);
                    CALL4(const_native_aeskeygenassist, -1, x1, x2, x3, x4);
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
