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
#include "emu/x64shaext.h"
#include "bitutils.h"
#include "freq.h"

#include "ppc64le_printer.h"
#include "dynarec_ppc64le_private.h"
#include "elfloader.h"
#include "../dynarec_helper.h"
#include "dynarec_ppc64le_functions.h"


uintptr_t dynarec64_0F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2, gback;
    uint8_t eb1, eb2;
    uint8_t gb1, gb2;
    uint8_t tmp1, tmp2, tmp3;
    int32_t i32, i32_;
    int64_t j64;
    int64_t fixedaddress, gdoffset;
    int unscaled;
    int lock;
    int cacheupd = 0;
    int v0, v1, q0, q1, d0, d1;
    MAYUSE(u8);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(gb1);
    MAYUSE(gb2);
    MAYUSE(wb1);
    MAYUSE(wb2);
    MAYUSE(j64);
    MAYUSE(lock);
    MAYUSE(gdoffset);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(d0);
    MAYUSE(d1);

    switch (opcode) {

        case 0x01:
            // TODO:, /0 is SGDT. While 0F 01 D0 is XGETBV, etc...
            nextop = F8;
            if (MODREG) {
                switch (nextop) {
                    case 0xD0:
                        INST_NAME("XGETBV");
                        BEQZ_MARK(xRCX);
                        TRAP();  // illegal if ECX != 0
                        MARK;
                        MOV32w(xRAX, 0b111);
                        MOV32w(xRDX, 0);
                        break;
                    default:
                        DEFAULT;
                }
            } else {
                switch ((nextop >> 3) & 7) {
                    default:
                        DEFAULT;
                }
            }
            break;
        case 0x05:
            INST_NAME("SYSCALL");
            NOTEST(x1);
            SMEND();
            GETIP(addr, x7);
            STORE_XEMU_CALL();
            if(!box64_wine || FindElfAddress(my_context, ip)) {
                CALL_S(const_x64syscall_linux, -1, 0);
            } else {
                CALL_S(const_x64syscall, -1, 0);
            }
            LOAD_XEMU_CALL();
            TABLE64(x3, addr); // expected return address
            BNE_MARK(xRIP, x3);
            LWZ(x1, offsetof(x64emu_t, quit), xEmu);
            CBZ_NEXT(x1);
            MARK;
            LOAD_XEMU_REM();
            jump_to_epilog(dyn, 0, xRIP, ninst);
            break;

        case 0x0B:
            INST_NAME("UD2");
            if (BOX64DRENV(dynarec_safeflags) > 1) {
                READFLAGS(X_PEND);
            } else {
                SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
            }
            GETIP(ip, x7);
            STORE_XEMU_CALL();
            CALL(const_native_ud, -1, 0, 0);
            LOAD_XEMU_CALL();
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0x0D:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("PREFETCH");
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, NO_DISP, 0);
                    NOP();  // prefetch hint, treat as NOP on PPC64LE
                    break;
                case 1:
                    INST_NAME("PREFETCHW");
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, NO_DISP, 0);
                    NOP();
                    break;
                case 2:
                    INST_NAME("PREFETCHWT1");
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, NO_DISP, 0);
                    NOP();
                    break;
                default: // NOP
                    FAKEED;
                    break;
            }
            break;

        case 0x18:
            nextop = F8;
            if (MODREG) {
                INST_NAME("NOP (multibyte)");
            } else
                switch ((nextop >> 3) & 7) {
                    case 0:
                        INST_NAME("PREFETCHNTA Ed");
                        FAKEED;
                        break;
                    case 1:
                        INST_NAME("PREFETCHT0 Ed");
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, NO_DISP, 0);
                        NOP();  // prefetch hint
                        break;
                    case 2:
                        INST_NAME("PREFETCHT1 Ed");
                        FAKEED;
                        break;
                    case 3:
                        INST_NAME("PREFETCHT2 Ed");
                        FAKEED;
                        break;
                    default:
                        INST_NAME("NOP (multibyte)");
                        FAKEED;
                }
            break;
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

        case 0x10:
            INST_NAME("MOVUPS Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg(dyn, ninst, x1, ed, 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DQ_DISP, 0);
                LXV(VSXREG(v0), fixedaddress, ed);
            }
            break;
        case 0x11:
            INST_NAME("MOVUPS Ex, Gx");
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

        case 0x12:
            nextop = F8;
            if (MODREG) {
                INST_NAME("MOVHLPS Gx, Ex");
                GETGX(v0, 1);
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                // Copy high qword of Ex to low qword of Gx
                // XXPERMDI with DM=2: T[0:63]=A[64:127], T[64:127]=B[64:127] — but we want Ex.high→Gx.low
                // Actually: ISA dw0 of v1 → ISA dw1 of v0 (x86 low = ISA dw1)
                XXPERMDI(VSXREG(v0), VSXREG(v1), VSXREG(v0), 0b00); // v0[dw0]=v1[dw0], v0[dw1]=v0[dw0] → gives us v1.high(x86) in v0.low(x86) and v0.high(x86) preserved... 
                // Wait, let me think more carefully about LE element ordering.
                // x86 high qword = ISA dw0 on PPC64LE
                // x86 low qword = ISA dw1 on PPC64LE
                // MOVHLPS: Gx[63:0] = Ex[127:64], Gx[127:64] unchanged
                // So: ISA dw1 of Gx = ISA dw0 of Ex; ISA dw0 of Gx unchanged
                // XXPERMDI(T, A, B, DM): DM[0:1] selects dword: T[dw0] = A[DM[0]], T[dw1] = B[DM[1]]
                // We want: T[dw0] = v0[dw0] (unchanged high), T[dw1] = v1[dw0] (Ex high = ISA dw0)
                // DM = 0b00: T[dw0]=A[dw0], T[dw1]=B[dw0]
                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(v1), 0b00);
            } else {
                INST_NAME("MOVLPS Gx, Ex");
                GETGX(v0, 1);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                // Load 64-bit into x86 low qword (ISA dw1)
                LD(x4, fixedaddress, ed);
                // Replace ISA dw1 of v0 with loaded value, keep ISA dw0
                MFVSRD(x5, VSXREG(v0));  // x5 = ISA dw0 (x86 high)
                MTVSRDD(VSXREG(v0), x5, x4); // dw0=x5(high preserved), dw1=x4(new low)
            }
            break;
        case 0x13:
            nextop = F8;
            INST_NAME("MOVLPS Ex, Gx");
            GETGX(v0, 0);
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 1);
                // Copy low qword of Gx to low qword of Ex
                // x86 low = ISA dw1
                MFVSRLD(x4, VSXREG(v0));   // x4 = ISA dw1 = x86 low of Gx
                MFVSRD(x5, VSXREG(v1));    // x5 = ISA dw0 = x86 high of Ex (preserve)
                MTVSRDD(VSXREG(v1), x5, x4);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                // Store low qword (ISA dw1) of Gx to memory
                MFVSRLD(x4, VSXREG(v0));
                STD(x4, fixedaddress, ed);
                SMWRITE2();
            }
            break;
        case 0x14:
            INST_NAME("UNPCKLPS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // Interleave low words (x86 low = ISA low on LE = VMRGLW territory)
            VMRGLW(VRREG(v0), VRREG(v1), VRREG(v0));
            break;
        case 0x15:
            INST_NAME("UNPCKHPS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // Interleave high words
            VMRGHW(VRREG(v0), VRREG(v1), VRREG(v0));
            break;
        case 0x16:
            nextop = F8;
            if (MODREG) {
                INST_NAME("MOVLHPS Gx, Ex");
                GETGX(v0, 1);
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                // MOVLHPS: Gx[127:64] = Ex[63:0], Gx[63:0] unchanged
                // ISA dw0 of Gx = ISA dw1 of Ex; ISA dw1 of Gx unchanged
                // XXPERMDI(T, A, B, DM): T[dw0]=A[DM[0]], T[dw1]=B[DM[1]]
                // Want: T[dw0]=v1[dw1] (Ex low=ISA dw1), T[dw1]=v0[dw1] (Gx low unchanged)
                // DM = 0b11: T[dw0]=A[dw1], T[dw1]=B[dw1]
                XXPERMDI(VSXREG(v0), VSXREG(v1), VSXREG(v0), 0b11);
            } else {
                INST_NAME("MOVHPS Gx, Ex");
                SMREAD();
                GETGX(v0, 1);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                // Load 64-bit into x86 high qword (ISA dw0)
                LD(x4, fixedaddress, ed);
                // Replace ISA dw0 of v0 with loaded value, keep ISA dw1
                MFVSRLD(x5, VSXREG(v0));  // x5 = ISA dw1 (x86 low, preserve)
                MTVSRDD(VSXREG(v0), x4, x5); // dw0=x4(new high), dw1=x5(low preserved)
            }
            break;
        case 0x17:
            nextop = F8;
            INST_NAME("MOVHPS Ex, Gx");
            GETGX(v0, 0);
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 1);
                // Copy high qword of Gx to low qword of Ex
                // x86 high = ISA dw0
                MFVSRD(x4, VSXREG(v0));    // x4 = ISA dw0 = x86 high of Gx
                MFVSRLD(x5, VSXREG(v1));   // x5 = ISA dw1 = x86 low of Ex... wait, we want low of Ex preserved?
                // MOVHPS to reg: copies Gx.high to Ex.low, Ex.high unchanged
                // Actually MOVHPS Ex, Gx: store Gx[127:64] to memory (not used for reg-reg typically)
                // Intel says MOVHPS r/m only accepts memory operand, so MODREG shouldn't happen
                // But if it does: ISA dw0 of Gx → ISA dw1 of Ex, ISA dw0 of Ex unchanged
                MFVSRD(x4, VSXREG(v0));    // x4 = ISA dw0 = x86 high of Gx
                MFVSRD(x5, VSXREG(v1));    // x5 = ISA dw0 = x86 high of Ex (preserve)
                MTVSRDD(VSXREG(v1), x5, x4); // dw0=high preserved, dw1=Gx.high
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                // Store high qword (ISA dw0) of Gx to memory
                MFVSRD(x4, VSXREG(v0));
                STD(x4, fixedaddress, ed);
                SMWRITE2();
            }
            break;

        case 0x28:
            INST_NAME("MOVAPS Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg(dyn, ninst, x1, ed, 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DQ_DISP, 0);
                LXV(VSXREG(v0), fixedaddress, ed);
            }
            break;
        case 0x29:
            INST_NAME("MOVAPS Ex, Gx");
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
            INST_NAME("MOVNTPS Ex, Gx");
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

        case 0x2E:
            // no special check...
        case 0x2F:
            if (opcode == 0x2F) {
                INST_NAME("COMISS Gx, Ex");
            } else {
                INST_NAME("UCOMISS Gx, Ex");
            }
            SETFLAGS(X_ALL, SF_SET_DF, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETGX(v0, 0);
            // v0 is SSE VR: x86 scalar float is in ISA word 3 (low 32 bits of ISA dw1).
            // Extract, place in ISA dw0 upper 32 bits, convert SP→DP for XSCMPUDP.
            d0 = fpu_get_scratch(dyn);
            MFVSRLD(x4, VSXREG(v0));       // x4 = ISA dw1 (x86 low qword)
            SLDI(x4, x4, 32);              // shift float to upper 32 bits (ISA word 0 position)
            MTVSRD(VSXREG(d0), x4);         // move to VSR ISA dw0
            XSCVSPDPN(VSXREG(d0), VSXREG(d0));  // convert single → double
            // Get Ex operand
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                d1 = fpu_get_scratch(dyn);
                MFVSRLD(x4, VSXREG(v1));
                SLDI(x4, x4, 32);
                MTVSRD(VSXREG(d1), x4);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            } else {
                SMREAD();
                d1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, DS_DISP, 0);
                LWZ(x4, fixedaddress, ed);
                SLDI(x4, x4, 32);
                MTVSRD(VSXREG(d1), x4);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            }
            CLEAR_FLAGS(x1);
            // Compare scalar double (after SP→DP conversion)
            XSCMPUDP(0, VSXREG(d0), VSXREG(d1));
            // Map PPC CR0 to x86 flags (identical to UCOMISD)
            MFCR(x1);
            RLWINM(x1, x1, 4, 28, 31);
            // CF = LT|UN
            RLWINM(x2, x1, 0, 31, 31);   // UN
            RLWINM(x3, x1, 29, 31, 31);  // LT
            OR(x2, x2, x3);
            // PF = UN
            RLWINM(x3, x1, 0, 31, 31);
            SLWI(x3, x3, F_PF);
            OR(x2, x2, x3);
            // ZF = EQ|UN
            RLWINM(x3, x1, 31, 31, 31);  // EQ
            RLWINM(x4, x1, 0, 31, 31);   // UN
            OR(x3, x3, x4);
            SLWI(x3, x3, F_ZF);
            OR(x2, x2, x3);
            OR(xFlags, xFlags, x2);
            break;

        case 0x38:
            nextop = F8;
            switch (nextop) {
                case 0x00:
                    INST_NAME("PSHUFB Gm, Em");
                    nextop = F8;
                    GETGM(v0);
                    GETEM(v1, 0);
                    // PSHUFB MMX: for each byte i, if bit 7 of Em[i] set, result[i]=0
                    //             else result[i] = Gm[Em[i] & 0x7]
                    // Use GPR approach: extract 64-bit values, process each byte
                    MFVSRLD(x4, VSXREG(v0));   // Gm data (64-bit) from ISA dw1
                    MFVSRLD(x5, VSXREG(v1));   // Em indices (64-bit) from ISA dw1
                    {
                        LI(x6, 0);  // result accumulator
                        for (int i = 0; i < 8; i++) {
                            // Extract index byte i from Em
                            if (i == 0) {
                                ANDI(x3, x5, 0xFF);
                            } else {
                                RLDICL(x3, x5, 64 - i * 8, 56);  // extract byte i
                            }
                            // index = x3 & 7, but first save bit 7 check
                            ANDI(x7, x3, 0x80);   // sets CR0: EQ if bit7 not set
                            ANDI(x3, x3, 7);
                            // Extract Gm[index]: shift Gm right by (index*8), take low byte
                            SLWI(x3, x3, 3);      // index * 8
                            SRD(x3, x4, x3);      // shift right
                            ANDI(x3, x3, 0xFF);   // isolate byte (sets CR0, but we need the previous CR0)
                            // Problem: ANDI on x3 overwrites CR0. Need to save CR0 from bit7 check.
                            // Solution: use CMPLWI/ISEL with x7 directly
                            CMPLDI(x7, 0);         // re-check: x7==0 means bit7 not set -> EQ
                            ISEL(x3, x3, 0, 2);   // CR0.EQ: if bit7 not set pick x3, else 0
                            // Insert into result at byte position i
                            if (i > 0) {
                                SLDI(x3, x3, i * 8);
                            }
                            OR(x6, x6, x3);
                        }
                        MTVSRDD(VSXREG(v0), xZR, x6);
                    }
                    break;
                case 0x04:
                    INST_NAME("PMADDUBSW Gm, Em");
                    nextop = F8;
                    GETGM(v0);
                    GETEM(v1, 0);
                    d0 = fpu_get_scratch(dyn);
                    d1 = fpu_get_scratch(dyn);
                    // Result[i] = sat_s16(Gm_u8[2i]*Em_s8[2i] + Gm_u8[2i+1]*Em_s8[2i+1])
                    {
                        int d2 = fpu_get_scratch(dyn);
                        XXLXOR(VSXREG(d2), VSXREG(d2), VSXREG(d2));
                        // Gm unsigned bytes → halfwords
                        VMRGLB(VRREG(d0), VRREG(d2), VRREG(v0));
                        // Em signed bytes → halfwords
                        VUPKLSB(VRREG(d1), VRREG(v1));
                        // Multiply even/odd halfwords → words, add pairs with signed saturation
                        VMULESH(VRREG(d2), VRREG(d0), VRREG(d1));
                        VMULOSH(VRREG(d0), VRREG(d0), VRREG(d1));
                        VADDSWS(VRREG(d0), VRREG(d2), VRREG(d0));
                        // Pack words → saturated signed halfwords
                        VPKSWSS(VRREG(v0), VRREG(d0), VRREG(d0));
                    }
                    break;
                case 0xF0:
                    INST_NAME("MOVBE Gd, Ed");
                    nextop = F8;
                    GETGD;
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                    LDxw(gd, ed, fixedaddress);
                    REVBxw(gd, gd, x1);
                    break;
                case 0xF1:
                    INST_NAME("MOVBE Ed, Gd");
                    nextop = F8;
                    GETGD;
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                    REVBxw(x1, gd, x3);
                    SDxw(x1, wback, fixedaddress);
                    SMWRITE2();
                    break;
                case 0xC8:
                case 0xC9:
                case 0xCA:
                case 0xCB:
                case 0xCC:
                case 0xCD:
                    u8 = nextop;
                    switch (u8) {
                        case 0xC8: INST_NAME("SHA1NEXTE Gx, Ex"); break;
                        case 0xC9: INST_NAME("SHA1MSG1 Gx, Ex"); break;
                        case 0xCA: INST_NAME("SHA1MSG2 Gx, Ex"); break;
                        case 0xCB: INST_NAME("SHA256RNDS2 Gx, Ex"); break;
                        case 0xCC: INST_NAME("SHA256MSG1 Gx, Ex"); break;
                        case 0xCD: INST_NAME("SHA256MSG2 Gx, Ex"); break;
                    }
                    nextop = F8;
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        sse_reflect_reg(dyn, ninst, ed);
                        ADDI(x2, xEmu, offsetof(x64emu_t, xmm[ed]));
                        ed = x2;
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, NO_DISP, 0);
                    }
                    GETG;
                    sse_forget_reg(dyn, ninst, gd);
                    ADDI(x1, xEmu, offsetof(x64emu_t, xmm[gd]));
                    sse_reflect_reg(dyn, ninst, 0);
                    switch (u8) {
                        case 0xC8: CALL(const_sha1nexte, -1, x1, ed); break;
                        case 0xC9: CALL(const_sha1msg1, -1, x1, ed); break;
                        case 0xCA: CALL(const_sha1msg2, -1, x1, ed); break;
                        case 0xCB: CALL(const_sha256rnds2, -1, x1, ed); break;
                        case 0xCC: CALL(const_sha256msg1, -1, x1, ed); break;
                        case 0xCD: CALL(const_sha256msg2, -1, x1, ed); break;
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0x3A:
            opcode = F8;
            switch (opcode) {
                case 0xCC:
                    INST_NAME("SHA1RNDS4 Gx, Ex, Ib");
                    nextop = F8;
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        sse_reflect_reg(dyn, ninst, ed);
                        ADDI(x2, xEmu, offsetof(x64emu_t, xmm[ed]));
                        wback = x2;
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, NO_DISP, 1);
                    }
                    u8 = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, gd);
                    ADDI(x1, xEmu, offsetof(x64emu_t, xmm[gd]));
                    MOV32w(x3, u8);
                    CALL4(const_sha1rnds4, -1, x1, wback, x3, 0);
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0x50:
            INST_NAME("MOVMSKPS Gd, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGD;
            // Extract sign bits of 4 floats
            // x86: bit0 = sign of float[0] (ISA word 3), bit1 = sign of float[1] (ISA word 2),
            //       bit2 = sign of float[2] (ISA word 1), bit3 = sign of float[3] (ISA word 0)
            MFVSRLD(x4, VSXREG(q0));    // x4 = ISA dw1 (x86 low: float[0] in low 32, float[1] in high 32)
            MFVSRD(x5, VSXREG(q0));     // x5 = ISA dw0 (x86 high: float[2] in low 32, float[3] in high 32)
            // float[0] sign = bit 31 of x4
            RLWINM(x1, x4, 1, 31, 31);  // x1 = bit0
            // float[1] sign = bit 63 of x4
            SRDI(x2, x4, 62);
            ANDI(x2, x2, 0x2);          // x2 = bit1
            OR(x1, x1, x2);
            // float[2] sign = bit 31 of x5
            RLWINM(x2, x5, 1, 31, 31);
            SLWI(x2, x2, 2);            // x2 = bit2
            OR(x1, x1, x2);
            // float[3] sign = bit 63 of x5
            SRDI(x2, x5, 60);
            ANDI(x2, x2, 0x8);          // x2 = bit3
            OR(gd, x1, x2);
            break;
        case 0x51:
            INST_NAME("SQRTPS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn);
                d0 = fpu_get_scratch(dyn);
                XVCMPEQSP(VSXREG(q0), VSXREG(v1), VSXREG(v1));  // q0 = -1 where input NOT NaN
            }
            XVSQRTSP(VSXREG(v0), VSXREG(v1));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQSP(VSXREG(d0), VSXREG(v0), VSXREG(v0));  // d0 = -1 where result NOT NaN
                XXLANDC(VSXREG(d0), VSXREG(q0), VSXREG(d0));    // d0 = input-ordered AND result-NaN (new NaNs)
                XXSPLTIB(VSXREG(q0), 31);
                VSLW(VRREG(d0), VRREG(d0), VRREG(q0));          // d0 = 0x80000000 in new NaN lanes
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(d0));      // OR sign bit into result
            }
            break;
        case 0x52:
            INST_NAME("RSQRTPS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            XVRSQRTESP(VSXREG(v0), VSXREG(v1));
            break;
        case 0x53:
            INST_NAME("RCPPS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            XVRESP(VSXREG(v0), VSXREG(v1));
            break;
        case 0x54:
            INST_NAME("ANDPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            break;
        case 0x55:
            INST_NAME("ANDNPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            // x86: dest = NOT(dest) AND src
            // PPC XXLANDC(T, A, B) = A AND NOT(B)
            // So: XXLANDC(v0, q0, v0) = q0 AND NOT(v0) = src AND NOT(dest) ✓
            XXLANDC(VSXREG(v0), VSXREG(q0), VSXREG(v0));
            break;
        case 0x56:
            INST_NAME("ORPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            break;
        case 0x57:
            INST_NAME("XORPS Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG && ((nextop & 7) + (rex.b << 3) == gd)) {
                // special case for XORPS Gx, Gx => zero
                q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
            } else {
                q0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                GETEX(q1, 0, 0);
                XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q1));
            }
            break;
        case 0x58:
            INST_NAME("ADDPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            if (!BOX64ENV(dynarec_fastnan)) {
                q1 = fpu_get_scratch(dyn);
                d0 = fpu_get_scratch(dyn);
                XVCMPEQSP(VSXREG(q1), VSXREG(v0), VSXREG(v0));  // -1 where Gx not NaN
                XVCMPEQSP(VSXREG(d0), VSXREG(q0), VSXREG(q0));  // -1 where Ex not NaN
                XXLAND(VSXREG(q1), VSXREG(q1), VSXREG(d0));      // -1 where BOTH not NaN
            }
            XVADDSP(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQSP(VSXREG(d0), VSXREG(v0), VSXREG(v0));  // -1 where result not NaN
                XXLANDC(VSXREG(d0), VSXREG(q1), VSXREG(d0));     // both-ordered AND result-NaN
                XXSPLTIB(VSXREG(q1), 31);
                VSLW(VRREG(d0), VRREG(d0), VRREG(q1));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(d0));
            }
            break;
        case 0x59:
            INST_NAME("MULPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            if (!BOX64ENV(dynarec_fastnan)) {
                q1 = fpu_get_scratch(dyn);
                d0 = fpu_get_scratch(dyn);
                XVCMPEQSP(VSXREG(q1), VSXREG(v0), VSXREG(v0));
                XVCMPEQSP(VSXREG(d0), VSXREG(q0), VSXREG(q0));
                XXLAND(VSXREG(q1), VSXREG(q1), VSXREG(d0));
            }
            XVMULSP(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQSP(VSXREG(d0), VSXREG(v0), VSXREG(v0));
                XXLANDC(VSXREG(d0), VSXREG(q1), VSXREG(d0));
                XXSPLTIB(VSXREG(q1), 31);
                VSLW(VRREG(d0), VRREG(d0), VRREG(q1));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(d0));
            }
            break;
        case 0x5A:
            INST_NAME("CVTPS2PD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // CVTPS2PD: convert 2 packed singles (low 64 bits) to 2 packed doubles
            // x86: float[0] (bits 0-31) → double[0] (bits 0-63), float[1] (bits 32-63) → double[1] (bits 64-127)
            // On PPC64LE ISA: x86 float[0] is ISA word 3, float[1] is ISA word 2
            // XVCVSPDP converts ISA word 0 and ISA word 2 to doubles in dw0 and dw1
            // We need ISA words 2 and 3 → first rearrange to words 0 and 2
            // XXPERMDI to put ISA dw1 (which has words 2,3) into both dw0 and dw1
            // then XVCVSPDP will convert word 0 of dw0 and word 0 of dw1
            // Actually XVCVSPDP: converts word[0] to dw[0] and word[2] to dw[1] in ISA terms
            // We have: dw1 = [word2, word3], dw0 = [word0, word1]
            // After XXPERMDI(tmp, v1, v1, 0b11): tmp[dw0] = v1[dw1], tmp[dw1] = v1[dw1]
            // So tmp = [word3, word2, word3, word2] (ISA: dw0=[w2,w3], dw1=[w2,w3])
            // XVCVSPDP would then convert word 0 of each dw = word2 and word2... that's wrong
            // Let me think differently:
            // XVCVSPDP(T, B): T[dw0] = convert(B[word0]), T[dw1] = convert(B[word2])
            // We want to convert B[word2] and B[word3]
            // Rearrange: swap so word3→word0 and word2→word2
            // Use XXSPLTW to put word3 in all words, then combine? Too complex.
            // Simpler: XXPERMDI(tmp, v1, v1, 0b10): tmp[dw0]=v1[dw0], tmp[dw1]=v1[dw0]... no
            // XXPERMDI(tmp, v1, v1, 0b11): tmp[dw0]=v1[dw1], tmp[dw1]=v1[dw1]
            // so word0=word2, word1=word3, word2=word2, word3=word3
            // XVCVSPDP converts word0(=word2=x86 float[1]) → dw0(=x86 high double), 
            //                   word2(=word2=x86 float[1]) → dw1(=x86 low double)
            // Wrong — we get float[1] in both positions.
            // Need: word0 = ISA word 3 (x86 float[0]), word2 = ISA word 2 (x86 float[1])
            // Use XXSWAPD (=XXPERMDI dm=2): tmp[dw0]=v1[dw1], tmp[dw1]=v1[dw0]
            // So word0=word2, word1=word3, word2=word0, word3=word1
            // XVCVSPDP: dw0=convert(word0=word2=x86 float[1]), dw1=convert(word2=word0=x86 float[3 or other])
            // Still wrong. The issue is we need word3 in word0 position.
            // Let's use a different approach: XXSPLTW + individual conversion
            // Or: use XXSLDWI (shift left double by word immediate)
            // XXSLDWI(T, A, B, SHW): concatenate A:B, shift left by SHW words, take high 128 bits
            // If SHW=1: T = {A[w1], A[w2], A[w3], B[w0]}
            // If SHW=3: T = {A[w3], B[w0], B[w1], B[w2]} — this puts word3 at word0!
            // XXSLDWI(tmp, v1, v1, 3): tmp = {v1[w3], v1[w0], v1[w1], v1[w2]}
            // XVCVSPDP: dw0=convert(tmp[w0]=v1[w3]=x86 float[0]) ← correct for x86 double[1]! wait...
            //           dw1=convert(tmp[w2]=v1[w1]=x86 float[2])  ← wrong
            // Hmm. Let me reconsider.
            // x86: CVTPS2PD: double[0] = (double)float[0], double[1] = (double)float[1]
            // ISA mapping: x86 double[0] = ISA dw1, x86 double[1] = ISA dw0
            //              x86 float[0] = ISA word3, x86 float[1] = ISA word2
            // XVCVSPDP: ISA dw0 = convert(ISA word0), ISA dw1 = convert(ISA word2)
            // We need: ISA dw0 = convert(word2) ← already correct if word2 = x86 float[1]!
            //          ISA dw1 = convert(word3) ← but XVCVSPDP converts word2 not word3
            // Wait, XVCVSPDP description says: "converts words 0 and 2 of source to doubles in dwords 0 and 1"
            // Actually, ISA spec says: XVCVSPDP: for i=0,1: dw[i] = convert(word[2*i]) i.e. words 0 and 2
            // We need: dw1 = convert(word3), dw0 = convert(word2)
            // So we need word3 in word2 position and word2 in word0 position
            // XXSLDWI(tmp, v1, v1, 1): tmp = [w1, w2, w3, w0]
            // words: 0=w1, 1=w2, 2=w3, 3=w0
            // XVCVSPDP: dw0 = convert(word0=w1), dw1 = convert(word2=w3)
            // dw1 = convert(w3) = convert(x86 float[0]) → goes to ISA dw1 = x86 double[0] ✓
            // dw0 = convert(w1) = convert(x86 float[2]) → wrong, should be x86 float[1]
            // Try XXSLDWI(tmp, v1, v1, 3): tmp = [w3, w0, w1, w2]
            // XVCVSPDP: dw0=convert(word0=w3=x86 float[0]), dw1=convert(word2=w1=x86 float[2])
            // Also wrong.
            // OK. Let's just use XXSPLTW to set up words individually, or use scalar conversion.
            // Actually the simplest: just use XXPERMDI to get dw1 into both halves and convert:
            // XXPERMDI(tmp, v1, v1, 0b11): tmp[dw0]=v1[dw1], tmp[dw1]=v1[dw1]
            // words: w0=v1.w2, w1=v1.w3, w2=v1.w2, w3=v1.w3  
            // XVCVSPDP: dw0=convert(w0=v1.w2=x86 float[1]), dw1=convert(w2=v1.w2=x86 float[1])
            // Both are float[1]... wrong.
            //
            // The fundamental issue: XVCVSPDP only converts words 0 and 2, but we need words 2 and 3.
            // Solution: XXSLDWI(tmp, v1, v1, 2) to rotate by 2 words:
            // tmp = [w2, w3, w0, w1]
            // XVCVSPDP: dw0=convert(w0_new=w2=x86 float[1]), dw1=convert(w2_new=w0=x86 float[3])
            // Still wrong for dw1.
            //
            // Let me try yet another approach. We want word2→word0 and word3→word2 positions.
            // XXSLDWI(tmp, v1, v1, 1): [w1,w2,w3,w0]
            // Then XXSLDWI again? Too many ops.
            //
            // Simplest correct approach: use XXSPLTW to isolate each float, convert individually
            // Or: rearrange with VPERM/XXPERMDI, then convert.
            // The cleanest: use scalar XSCVSPDPN twice and combine.
            //
            // Actually, re-reading the ISA more carefully:
            // XVCVSPDP: T[dw0] = convert_sp_to_dp(B[word0])   (high word of dw0)
            //           T[dw1] = convert_sp_to_dp(B[word2])   (high word of dw1)
            // Wait — "word0" means the ISA-level word at position 0, which on LE is the highest addressed word.
            // On PPC64LE, ISA word 0 = big-endian word 0 = highest-order word (byte 0-3) = LE word 3.
            // x86 float[1] is at x86 bits 32-63 = LE word 1 = ISA word 2. ✓ for dw1 position!
            // x86 float[0] is at x86 bits 0-31  = LE word 0 = ISA word 3.
            // So XVCVSPDP converts ISA word 0 (= x86 float at LE word 3 = x86 float[3]??)
            //
            // I'm going in circles. Let me just use the scalar approach — it's only 2 conversions:
            {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                // Extract x86 float[0] (ISA dw1 low 32 = LE word 0)
                MFVSRLD(x4, VSXREG(v1));       // x4 = ISA dw1 (contains x86 float[1]:float[0])
                SLDI(x5, x4, 32);              // x5 = float[0] in upper 32
                MTVSRD(VSXREG(d0), x5);
                XSCVSPDPN(VSXREG(d0), VSXREG(d0));  // d0 = (double)float[0]
                // Extract x86 float[1] (ISA dw1 high 32 = LE word 1)
                RLDICL(x5, x4, 0, 0);          // x5 = x4 unchanged, float[1] already in upper 32
                // Actually float[1] is the upper 32 bits of ISA dw1
                // x4 = [float1_32bit | float0_32bit], float[1] is in bits 63:32
                // Already in upper position? No. x4 is a 64-bit GPR. ISA dw1 as 64 bits:
                // On LE: low 32 bits = x86 float[0], high 32 bits = x86 float[1]
                // So float[1] is already at bits 63:32 of x4
                RLDICR(x5, x4, 0, 31);         // clear low 32 bits, keep high 32 (float[1] in bits 63:32)
                MTVSRD(VSXREG(d1), x5);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));  // d1 = (double)float[1]
                // Combine: ISA dw0 = x86 double[1] = d1, ISA dw1 = x86 double[0] = d0
                // MFVSRD gets ISA dw0 (upper 64 bits of VSR)
                MFVSRD(x4, VSXREG(d1));         // x4 = double[1] for ISA dw0
                MFVSRD(x5, VSXREG(d0));         // x5 = double[0] for ISA dw1
                MTVSRDD(VSXREG(v0), x4, x5);
            }
            break;
        case 0x5B:
            INST_NAME("CVTDQ2PS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // Convert 4 packed signed int32 to 4 packed single-precision floats
            XVCVSXWSP(VSXREG(v0), VSXREG(v1));
            break;
        case 0x5C:
            INST_NAME("SUBPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            if (!BOX64ENV(dynarec_fastnan)) {
                q1 = fpu_get_scratch(dyn);
                d0 = fpu_get_scratch(dyn);
                XVCMPEQSP(VSXREG(q1), VSXREG(v0), VSXREG(v0));
                XVCMPEQSP(VSXREG(d0), VSXREG(q0), VSXREG(q0));
                XXLAND(VSXREG(q1), VSXREG(q1), VSXREG(d0));
            }
            XVSUBSP(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQSP(VSXREG(d0), VSXREG(v0), VSXREG(v0));
                XXLANDC(VSXREG(d0), VSXREG(q1), VSXREG(d0));
                XXSPLTIB(VSXREG(q1), 31);
                VSLW(VRREG(d0), VRREG(d0), VRREG(q1));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(d0));
            }
            break;
        case 0x5D:
            INST_NAME("MINPS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // x86 MINPS: return min, but if either is NaN or both equal, return src2 (Ex)
            // XVCMPGTSP(q0, v1, v0): q0 = (Ex > Gx) ? -1 : 0
            //   Ex > Gx:  mask=-1, XXSEL picks B=v0(Gx) — correct (Gx is smaller)
            //   Ex <= Gx: mask=0,  XXSEL picks A=v1(Ex) — correct (Ex is smaller or equal)
            //   NaN:      mask=0,  XXSEL picks A=v1(Ex) — correct (x86 returns src2)
            q0 = fpu_get_scratch(dyn);
            XVCMPGTSP(VSXREG(q0), VSXREG(v1), VSXREG(v0));
            XXSEL(VSXREG(v0), VSXREG(v1), VSXREG(v0), VSXREG(q0));
            break;
        case 0x5E:
            INST_NAME("DIVPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            if (!BOX64ENV(dynarec_fastnan)) {
                q1 = fpu_get_scratch(dyn);
                d0 = fpu_get_scratch(dyn);
                XVCMPEQSP(VSXREG(q1), VSXREG(v0), VSXREG(v0));
                XVCMPEQSP(VSXREG(d0), VSXREG(q0), VSXREG(q0));
                XXLAND(VSXREG(q1), VSXREG(q1), VSXREG(d0));
            }
            XVDIVSP(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQSP(VSXREG(d0), VSXREG(v0), VSXREG(v0));
                XXLANDC(VSXREG(d0), VSXREG(q1), VSXREG(d0));
                XXSPLTIB(VSXREG(q1), 31);
                VSLW(VRREG(d0), VRREG(d0), VRREG(q1));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(d0));
            }
            break;
        case 0x5F:
            INST_NAME("MAXPS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // x86 MAXPS: if either is NaN, return src2 (Ex)
            // XVCMPGTSP(mask, v0, v1) => mask = (v0 > v1) ? -1 : 0
            // NaN or v0 <= v1: mask = 0, XXSEL picks A(v1) ✓
            // v0 > v1: mask = -1, XXSEL picks B(v0) ✓
            q0 = fpu_get_scratch(dyn);
            XVCMPGTSP(VSXREG(q0), VSXREG(v0), VSXREG(v1));
            XXSEL(VSXREG(v0), VSXREG(v1), VSXREG(v0), VSXREG(q0));
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

        case 0x31:
            INST_NAME("RDTSC");
            NOTEST(x1);
            if (box64_rdtsc) {
                CALL(const_readtsc, x3, 0, 0); // will return the u64 in x3
            } else {
                MFTB(x3);
            }
            if (box64_rdtsc_shift) {
                SLDI(x3, x3, box64_rdtsc_shift);
            }
            SRDI(xRDX, x3, 32);
            ZEROUP2(xRAX, x3);
            break;

#define GO(GETFLAGS, NO, YES, NATNO, NATYES, F, I)                                          \
    READFLAGS_FUSION(F, x1, x2, x3, x4, x5);                                                \
    i32_ = F32S;                                                                             \
    if (rex.is32bits)                                                                        \
        j64 = (uint32_t)(addr + i32_);                                                       \
    else                                                                                     \
        j64 = addr + i32_;                                                                   \
    JUMP(j64, 1);                                                                            \
    if (!dyn->insts[ninst].nat_flags_fusion) {                                               \
        GETFLAGS;                                                                            \
    }                                                                                        \
    if (dyn->insts[ninst].x64.jmp_insts == -1 || CHECK_CACHE()) {                            \
        /* out of the block */                                                               \
        i32 = dyn->insts[ninst].epilog - (dyn->native_size);                                 \
        if (dyn->insts[ninst].nat_flags_fusion) {                                            \
            NATIVEJUMP_safe(NATNO, i32);                                                     \
        } else {                                                                             \
            B##NO##_safe(tmp1, i32);                                                         \
        }                                                                                    \
        if (dyn->insts[ninst].x64.jmp_insts == -1) {                                         \
            if (!(dyn->insts[ninst].x64.barrier & BARRIER_FLOAT))                            \
                fpu_purgecache(dyn, ninst, 1, tmp1, tmp2, tmp3);                             \
            jump_to_next(dyn, j64, 0, ninst, rex.is32bits);                                  \
        } else {                                                                             \
            CacheTransform(dyn, ninst, cacheupd, tmp1, tmp2, tmp3);                          \
            i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size);  \
            B(i32);                                                                          \
        }                                                                                    \
    } else {                                                                                 \
        /* inside the block */                                                               \
        i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size);      \
        if (dyn->insts[ninst].nat_flags_fusion) {                                            \
            NATIVEJUMP_safe(NATYES, i32);                                                    \
        } else {                                                                             \
            B##YES##_safe(tmp1, i32);                                                        \
        }                                                                                    \
    }

            GOCOND(0x80, "J", "Id");

#undef GO

#define GO(GETFLAGS, NO, YES, NATNO, NATYES, F, I)                                          \
    READFLAGS_FUSION(F, x1, x2, x3, x4, x5);                                                \
    nextop = F8;                                                                             \
    if (dyn->insts[ninst].nat_flags_fusion) {                                                \
        NATIVESET(NATYES, tmp3);                                                             \
    } else {                                                                                 \
        GETFLAGS;                                                                            \
        S##YES(tmp3, tmp1);                                                                  \
    }                                                                                        \
    if (MODREG) {                                                                            \
        if (rex.rex) {                                                                       \
            eb1 = TO_NAT((nextop & 7) + (rex.b << 3));                                       \
            eb2 = 0;                                                                         \
        } else {                                                                             \
            ed = (nextop & 7);                                                               \
            eb2 = (ed >> 2) * 8;                                                             \
            eb1 = TO_NAT(ed & 3);                                                            \
        }                                                                                    \
        RLDIMI(eb1, tmp3, eb2, 56 - eb2);                                                    \
    } else {                                                                                 \
        addr = geted(dyn, addr, ninst, nextop, &ed, tmp2, tmp1, &fixedaddress, rex, NULL, DS_DISP, 0); \
        STB(tmp3, fixedaddress, ed);                                                         \
        SMWRITE();                                                                           \
    }

            GOCOND(0x90, "SET", "Eb");

#undef GO

        case 0x60:
            INST_NAME("PUNPCKLBW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            // MMX: interleave low bytes of Gm and Em (only low 32 bits used from each 64-bit MMX)
            // On PPC64LE, MMX regs live in VR space (ISA dw1). The low 32 bits of the 64-bit
            // MMX register contain bytes 0-3. We use VMRGLB on VR space.
            // But MMX is 64-bit, not 128-bit. For MMX, we only care about the low 64 bits.
            // VMRGLB interleaves the low 8 bytes of each VR, which is what we want.
            VMRGLB(VRREG(v0), VRREG(v1), VRREG(v0));
            PUTEM(v0);
            break;
        case 0x61:
            INST_NAME("PUNPCKLWD Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VMRGLH(VRREG(v0), VRREG(v1), VRREG(v0));
            PUTEM(v0);
            break;
        case 0x62:
            INST_NAME("PUNPCKLDQ Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VMRGLW(VRREG(v0), VRREG(v1), VRREG(v0));
            PUTEM(v0);
            break;
        case 0x63:
            INST_NAME("PACKSSWB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            // Pack signed halfwords to signed bytes with saturation
            // VPKSHSS gives 128-bit result; for 64-bit MMX, we need to extract
            // the low 4 packed bytes from each operand and combine them
            q0 = fpu_get_scratch(dyn);
            VPKSHSS(VRREG(q0), VRREG(v1), VRREG(v0));
            // q0 low 8 bytes = [pack(v0 h0-h3), zeros]; high 8 bytes = [pack(v1 h0-h3), zeros]
            MFVSRLD(x4, VSXREG(q0));  // low 32 bits = pack of v0 halfwords 0-3
            MFVSRD(x5, VSXREG(q0));   // low 32 bits = pack of v1 halfwords 0-3
            RLDIMI(x4, x5, 32, 0);    // insert v1 result into high 32 bits
            MTVSRDD(VSXREG(v0), xZR, x4);
            PUTEM(v0);
            break;
        case 0x64:
            INST_NAME("PCMPGTB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VCMPGTSB(VRREG(v0), VRREG(v0), VRREG(v1));
            PUTEM(v0);
            break;
        case 0x65:
            INST_NAME("PCMPGTW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VCMPGTSH(VRREG(v0), VRREG(v0), VRREG(v1));
            PUTEM(v0);
            break;
        case 0x66:
            INST_NAME("PCMPGTD Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VCMPGTSW(VRREG(v0), VRREG(v0), VRREG(v1));
            PUTEM(v0);
            break;
        case 0x67:
            INST_NAME("PACKUSWB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            // Pack signed halfwords to unsigned bytes with unsigned saturation
            q0 = fpu_get_scratch(dyn);
            VPKSHUS(VRREG(q0), VRREG(v1), VRREG(v0));
            // q0 low 8 bytes = [pack(v0 h0-h3), zeros]; high 8 bytes = [pack(v1 h0-h3), zeros]
            MFVSRLD(x4, VSXREG(q0));  // low 32 bits = pack of v0 halfwords 0-3
            MFVSRD(x5, VSXREG(q0));   // low 32 bits = pack of v1 halfwords 0-3
            RLDIMI(x4, x5, 32, 0);    // insert v1 result into high 32 bits
            MTVSRDD(VSXREG(v0), xZR, x4);
            PUTEM(v0);
            break;
        case 0x68:
            INST_NAME("PUNPCKHBW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            // Interleave high bytes (bytes 4-7 of each 64-bit MMX reg)
            // VMRGLB interleaves bytes 0-7; the result bytes 8-15 hold the interleave of bytes 4-7
            q0 = fpu_get_scratch(dyn);
            VMRGLB(VRREG(q0), VRREG(v1), VRREG(v0));
            MFVSRD(x4, VSXREG(q0));   // dword[0] = bytes 8-15 = interleave of bytes 4-7
            MTVSRDD(VSXREG(v0), xZR, x4);
            PUTEM(v0);
            break;
        case 0x69:
            INST_NAME("PUNPCKHWD Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            // Interleave high halfwords (halfwords 2-3 of each 64-bit MMX reg)
            q0 = fpu_get_scratch(dyn);
            VMRGLH(VRREG(q0), VRREG(v1), VRREG(v0));
            MFVSRD(x4, VSXREG(q0));   // dword[0] = bytes 8-15 = interleave of halfwords 2-3
            MTVSRDD(VSXREG(v0), xZR, x4);
            PUTEM(v0);
            break;
        case 0x6A:
            INST_NAME("PUNPCKHDQ Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            // Interleave high dwords (dword 1 of each 64-bit MMX reg)
            q0 = fpu_get_scratch(dyn);
            VMRGLW(VRREG(q0), VRREG(v1), VRREG(v0));
            MFVSRD(x4, VSXREG(q0));   // dword[0] = bytes 8-15 = interleave of dword 1
            MTVSRDD(VSXREG(v0), xZR, x4);
            PUTEM(v0);
            break;
        case 0x6B:
            INST_NAME("PACKSSDW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            // Pack signed dwords to signed halfwords with saturation
            // VPKSWSS gives 128-bit result; for 64-bit MMX, we need to extract
            // the low 2 packed halfwords from each operand and combine them
            q0 = fpu_get_scratch(dyn);
            VPKSWSS(VRREG(q0), VRREG(v1), VRREG(v0));
            // q0 low 8 bytes = [pack(v0 dw0-dw1), zeros]; high 8 bytes = [pack(v1 dw0-dw1), zeros]
            MFVSRLD(x4, VSXREG(q0));  // low 32 bits = pack of v0 dwords 0-1
            MFVSRD(x5, VSXREG(q0));   // low 32 bits = pack of v1 dwords 0-1
            RLDIMI(x4, x5, 32, 0);    // insert v1 result into high 32 bits
            MTVSRDD(VSXREG(v0), xZR, x4);
            PUTEM(v0);
            break;
        case 0x70:
            INST_NAME("PSHUFW Gm, Em, Ib");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 1);
            u8 = F8;
            // PSHUFW shuffles 4 halfwords within a 64-bit MMX register by immediate
            // u8 encodes 4 2-bit indices: word i of result = word (u8>>(2*i))&3 of source
            MFVSRLD(x4, VSXREG(v1));  // v1 is in VR space, MFVSRLD gets ISA dw1 (MMX data)
            // x4 has the 64-bit MMX value in LE order:
            // bits [15:0] = word0, [31:16] = word1, [47:32] = word2, [63:48] = word3
            if (u8 == 0xE4) {
                // Identity shuffle — just copy
                MV(x5, x4);
            } else {
                // General case: extract 4 halfwords and reassemble
                int sel0 = (u8 >> 0) & 3;
                int sel1 = (u8 >> 2) & 3;
                int sel2 = (u8 >> 4) & 3;
                int sel3 = (u8 >> 6) & 3;
                // Extract halfword: shift right by sel*16, mask to 16 bits
                // For position 0 (result bits 15:0)
                if (sel0 == 0) {
                    RLWINM(x5, x4, 0, 16, 31);         // word 0: low 16 bits
                } else {
                    RLDICL(x5, x4, 64 - sel0 * 16, 48); // shift right, keep low 16
                }
                // For position 1 (result bits 31:16) — extract then insert at bit 16
                if (sel1 == 0) {
                    RLDIMI(x5, x4, 16, 32);             // insert word0 shifted left 16 into bits 31:16
                } else {
                    RLDICL(x6, x4, 64 - sel1 * 16, 48);
                    RLDIMI(x5, x6, 16, 32);
                }
                // For position 2 (result bits 47:32)
                if (sel2 == 0) {
                    RLDIMI(x5, x4, 32, 16);
                } else {
                    RLDICL(x6, x4, 64 - sel2 * 16, 48);
                    RLDIMI(x5, x6, 32, 16);
                }
                // For position 3 (result bits 63:48)
                if (sel3 == 0) {
                    RLDICL(x6, x4, 0, 48);              // extract low 16 bits
                    RLDIMI(x5, x6, 48, 0);
                } else {
                    RLDICL(x6, x4, 64 - sel3 * 16, 48);
                    RLDIMI(x5, x6, 48, 0);
                }
            }
            MTVSRDD(VSXREG(v0), xZR, x5);
            break;
        case 0x71:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("PSRLW Em, Ib");
                    GETEM(v0, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 15) {
                            // Zero the register
                            LI(x4, 0);
                            MTVSRDD(VSXREG(v0), xZR, x4);
                        } else {
                            q0 = fpu_get_scratch(dyn);
                            XXSPLTIB(VSXREG(q0), u8);
                            VSRH(VRREG(v0), VRREG(v0), VRREG(q0));
                        }
                        PUTEM(v0);
                    }
                    break;
                case 4:
                    INST_NAME("PSRAW Em, Ib");
                    GETEM(v0, 1);
                    u8 = F8;
                    if (u8 > 15) u8 = 15;
                    if (u8) {
                        q0 = fpu_get_scratch(dyn);
                        XXSPLTIB(VSXREG(q0), u8);
                        VSRAH(VRREG(v0), VRREG(v0), VRREG(q0));
                    }
                    PUTEM(v0);
                    break;
                case 6:
                    INST_NAME("PSLLW Em, Ib");
                    GETEM(v0, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 15) {
                            LI(x4, 0);
                            MTVSRDD(VSXREG(v0), xZR, x4);
                        } else {
                            q0 = fpu_get_scratch(dyn);
                            XXSPLTIB(VSXREG(q0), u8);
                            VSLH(VRREG(v0), VRREG(v0), VRREG(q0));
                        }
                        PUTEM(v0);
                    }
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
                    INST_NAME("PSRLD Em, Ib");
                    GETEM(v0, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 31) {
                            LI(x4, 0);
                            MTVSRDD(VSXREG(v0), xZR, x4);
                        } else {
                            q0 = fpu_get_scratch(dyn);
                            XXSPLTIB(VSXREG(q0), u8);
                            VSRW(VRREG(v0), VRREG(v0), VRREG(q0));
                        }
                        PUTEM(v0);
                    }
                    break;
                case 4:
                    INST_NAME("PSRAD Em, Ib");
                    GETEM(v0, 1);
                    u8 = F8;
                    if (u8 > 31) u8 = 31;
                    if (u8) {
                        q0 = fpu_get_scratch(dyn);
                        XXSPLTIB(VSXREG(q0), u8);
                        VSRAW(VRREG(v0), VRREG(v0), VRREG(q0));
                    }
                    PUTEM(v0);
                    break;
                case 6:
                    INST_NAME("PSLLD Em, Ib");
                    GETEM(v0, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 31) {
                            LI(x4, 0);
                            MTVSRDD(VSXREG(v0), xZR, x4);
                        } else {
                            q0 = fpu_get_scratch(dyn);
                            XXSPLTIB(VSXREG(q0), u8);
                            VSLW(VRREG(v0), VRREG(v0), VRREG(q0));
                        }
                        PUTEM(v0);
                    }
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
                    INST_NAME("PSRLQ Em, Ib");
                    GETEM(v0, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 63) {
                            LI(x4, 0);
                            MTVSRDD(VSXREG(v0), xZR, x4);
                        } else {
                            q0 = fpu_get_scratch(dyn);
                            XXSPLTIB(VSXREG(q0), u8);
                            VSRD(VRREG(v0), VRREG(v0), VRREG(q0));
                        }
                        PUTEM(v0);
                    }
                    break;
                case 6:
                    INST_NAME("PSLLQ Em, Ib");
                    GETEM(v0, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 63) {
                            LI(x4, 0);
                            MTVSRDD(VSXREG(v0), xZR, x4);
                        } else {
                            q0 = fpu_get_scratch(dyn);
                            XXSPLTIB(VSXREG(q0), u8);
                            VSLD(VRREG(v0), VRREG(v0), VRREG(q0));
                        }
                        PUTEM(v0);
                    }
                    break;
                default:
                    *ok = 0;
                    DEFAULT;
            }
            break;
        case 0x74:
            INST_NAME("PCMPEQB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VCMPEQUB(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0x75:
            INST_NAME("PCMPEQW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VCMPEQUH(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0x76:
            INST_NAME("PCMPEQD Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VCMPEQUW(VRREG(v0), VRREG(v0), VRREG(v1));
            break;

        case 0x6E:
            INST_NAME("MOVD Gm, Ed");
            nextop = F8;
            GETG;
            v0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                if (rex.w) {
                    MTVSRDD(VSXREG(v0), xZR, ed);
                } else {
                    RLWINM(x4, ed, 0, 0, 31);  // zero-extend 32-bit
                    MTVSRDD(VSXREG(v0), xZR, x4);
                }
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                if (rex.w) {
                    LD(x4, fixedaddress, wback);
                    MTVSRDD(VSXREG(v0), xZR, x4);
                } else {
                    LWZ(x4, fixedaddress, wback);
                    MTVSRDD(VSXREG(v0), xZR, x4);
                }
            }
            break;
        case 0x6F:
            INST_NAME("MOVQ Gm, Em");
            nextop = F8;
            GETG;
            if (MODREG) {
                v1 = mmx_get_reg(dyn, ninst, x1, x2, x3, nextop & 7);
                v0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                v0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                LD(x4, fixedaddress, wback);
                MTVSRDD(VSXREG(v0), xZR, x4);
            }
            break;

        case 0x77:
            INST_NAME("EMMS");
            // empty MMX, FPU now usable
            mmx_purgecache(dyn, ninst, 0, x1);
            break;

        case 0x7E:
            INST_NAME("MOVD Ed, Gm");
            nextop = F8;
            GETGM(v0);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                if (rex.w) {
                    MFVSRLD(ed, VSXREG(v0));
                } else {
                    MFVSRLD(ed, VSXREG(v0));
                    RLWINM(ed, ed, 0, 0, 31);  // zero-extend 32-bit
                    ZEROUP(ed);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, DS_DISP, 0);
                if (rex.w) {
                    MFVSRLD(x4, VSXREG(v0));
                    STD(x4, fixedaddress, ed);
                } else {
                    MFVSRLD(x4, VSXREG(v0));
                    RLWINM(x4, x4, 0, 0, 31);  // zero-extend 32-bit
                    STW(x4, fixedaddress, ed);
                }
                SMWRITE2();
            }
            break;
        case 0x7F:
            INST_NAME("MOVQ Em, Gm");
            nextop = F8;
            GETGM(v0);
            if (MODREG) {
                v1 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, nextop & 7);
                XXLOR(VSXREG(v1), VSXREG(v0), VSXREG(v0));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, DS_DISP, 0);
                MFVSRLD(x4, VSXREG(v0));
                STD(x4, fixedaddress, ed);
                SMWRITE2();
            }
            break;

        case 0xA0:
            INST_NAME("PUSH FS");
            LHZ(x2, offsetof(x64emu_t, segs[_FS]), xEmu);
            PUSH1z(x2);
            break;
        case 0xA1:
            INST_NAME("POP FS");
            POP1z(x2);
            STH(x2, offsetof(x64emu_t, segs[_FS]), xEmu);
            CBZ_NEXT(x2);
            MOV32w(x1, _FS);
            CALL(const_getsegmentbase, -1, x1, 0);
            break;
        case 0xA2:
            INST_NAME("CPUID");
            NOTEST(x1);
            CALL_(const_cpuid, -1, 0, 0, 0);
            // BX and DX are not synchronized during the call, so need to force the update
            LD(xRDX, offsetof(x64emu_t, regs[_DX]), xEmu);
            LD(xRBX, offsetof(x64emu_t, regs[_BX]), xEmu);
            break;
        case 0xA8:
            INST_NAME("PUSH GS");
            LHZ(x2, offsetof(x64emu_t, segs[_GS]), xEmu);
            PUSH1z(x2);
            break;
        case 0xA9:
            INST_NAME("POP GS");
            POP1z(x2);
            STH(x2, offsetof(x64emu_t, segs[_GS]), xEmu);
            CBZ_NEXT(x2);
            MOV32w(x1, _GS);
            CALL(const_getsegmentbase, -1, x1, 0);
            break;

        case 0xA3:
            INST_NAME("BT Ed, Gd");
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                if (rex.w)
                    SRADI(x1, gd, 6);
                else
                    SRAWI(x1, gd, 5);
                if (!rex.w && !rex.is32bits) { EXTSW(x1, x1); }
                SLADDy(x3, x1, wback, 2 + rex.w); // (&ed) += r1*4;
                LDxw(x1, x3, fixedaddress);
                ed = x1;
            }
            IFX (X_CF) {
                ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
                SRLxw(x4, ed, x2);
                RLDIMI(xFlags, x4, F_CF, 63 - F_CF);
            }
            break;

        case 0xAB:
            INST_NAME("BTS Ed, Gd");
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
                if (rex.w)
                    SRADI(x1, gd, 6);
                else
                    SRAWI(x1, gd, 5);
                if (!rex.w && !rex.is32bits) { EXTSW(x1, x1); }
                SLADDy(x3, x1, wback, 2 + rex.w);
                LDxw(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
            IFX (X_CF) {
                SRD(x4, ed, x2);
                RLDIMI(xFlags, x4, F_CF, 63 - F_CF);
            }
            LI(x4, 1);
            SLD(x4, x4, x2);
            OR(ed, ed, x4);
            if (wback) {
                SDxw(ed, wback, fixedaddress);
                SMWRITE();
            } else if (!rex.w) {
                ZEROUP(ed);
            }
            break;

        case 0xA4:
            nextop = F8;
            INST_NAME("SHLD Ed, Gd, Ib");
            if (geted_ib(dyn, addr, ninst, nextop) & (rex.w ? 63 : 31)) {
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETED(1);
                GETGD;
                u8 = F8 & (rex.w ? 63 : 31);
                emit_shld32c(dyn, ninst, rex, ed, gd, u8, x3, x4);
                WBACK;
            } else {
                FAKEED;
                if (!rex.w && !rex.is32bits && MODREG) { ZEROUP(ed); }
                F8;
            }
            break;
        case 0xA5:
            nextop = F8;
            INST_NAME("SHLD Ed, Gd, CL");
            if (BOX64DRENV(dynarec_safeflags) > 1) {
                READFLAGS(X_ALL);
                SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_FUSION);
            } else
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            GETGD;
            GETED(0);
            if (!rex.w && !rex.is32bits && MODREG) { ZEROUP(ed); }
            ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
            CBZ_NEXT(x3);
            emit_shld32(dyn, ninst, rex, ed, gd, x3, x4, x5, x6);
            WBACK;
            break;
        case 0xAC:
            nextop = F8;
            INST_NAME("SHRD Ed, Gd, Ib");
            if (geted_ib(dyn, addr, ninst, nextop) & (rex.w ? 63 : 31)) {
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETED(1);
                GETGD;
                u8 = F8 & (rex.w ? 63 : 31);
                emit_shrd32c(dyn, ninst, rex, ed, gd, u8, x3, x4);
                WBACK;
            } else {
                FAKEED;
                if (!rex.w && !rex.is32bits && MODREG) { ZEROUP(ed); }
                F8;
            }
            break;
        case 0xAD:
            nextop = F8;
            INST_NAME("SHRD Ed, Gd, CL");
            if (BOX64DRENV(dynarec_safeflags) > 1) {
                READFLAGS(X_ALL);
                SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_FUSION);
            } else
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            GETGD;
            GETED(0);
            if (!rex.w && !rex.is32bits && MODREG) { ZEROUP(ed); }
            ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
            CBZ_NEXT(x3);
            emit_shrd32(dyn, ninst, rex, ed, gd, x3, x5, x4, x6);
            WBACK;
            break;

        case 0xAE:
            nextop = F8;
            if (MODREG)
                switch (nextop) {
                    case 0xE8:
                        INST_NAME("LFENCE");
                        LWSYNC();
                        break;
                    case 0xF0:
                        INST_NAME("MFENCE");
                        SYNC();
                        break;
                    case 0xF8:
                        INST_NAME("SFENCE");
                        LWSYNC();
                        break;
                    default:
                        DEFAULT;
                }
            else
                switch ((nextop >> 3) & 7) {
                    case 0:
                        INST_NAME("FXSAVE Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        BARRIER(BARRIER_FLOAT);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x3, &fixedaddress, rex, NULL, NO_DISP, 0);
                        CALL(rex.is32bits ? const_fpu_fxsave32 : const_fpu_fxsave64, -1, ed, 0);
                        break;
                    case 1:
                        INST_NAME("FXRSTOR Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        BARRIER(BARRIER_FLOAT);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x3, &fixedaddress, rex, NULL, NO_DISP, 0);
                        CALL(rex.is32bits ? const_fpu_fxrstor32 : const_fpu_fxrstor64, -1, ed, 0);
                        break;
                    case 2:
                        INST_NAME("LDMXCSR Md");
                        GETED(0);
                        STW(ed, offsetof(x64emu_t, mxcsr), xEmu);
                        break;
                    case 3:
                        INST_NAME("STMXCSR Md");
                        addr = geted(dyn, addr, ninst, nextop, &wback, x1, x2, &fixedaddress, rex, NULL, NO_DISP, 0);
                        LWZ(x4, offsetof(x64emu_t, mxcsr), xEmu);
                        STW(x4, fixedaddress, wback);
                        break;
                    case 4:
                        INST_NAME("XSAVE Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        BARRIER(BARRIER_FLOAT);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, NO_DISP, 0);
                        MOV32w(x2, rex.w ? 0 : 1);
                        CALL(const_fpu_xsave, -1, ed, x2);
                        break;
                    case 5:
                        INST_NAME("XRSTOR Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        BARRIER(BARRIER_FLOAT);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, NO_DISP, 0);
                        MOV32w(x2, rex.w ? 0 : 1);
                        CALL(const_fpu_xrstor, -1, ed, x2);
                        break;
                    case 7:
                        INST_NAME("CLFLUSH Ed");
                        FAKEED;
                        SYNC();
                        break;
                    default:
                        DEFAULT;
                }
            break;
        case 0xAF:
            INST_NAME("IMUL Gd, Ed");
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            CLEAR_FLAGS(x3);
            if (rex.w) {
                UFLAG_IF {
                    MULHD(x3, gd, ed);
                    MULLD(gd, gd, ed);
                    SET_DFNONE();
                    IFX (X_CF | X_OF) {
                        SRADI(x4, gd, 63);
                        XOR(x3, x3, x4);
                        SNEZ(x3, x3);
                        IFX (X_CF) RLDIMI(xFlags, x3, F_CF, 63 - F_CF);
                        IFX (X_OF) RLDIMI(xFlags, x3, F_OF, 63 - F_OF);
                    }
                } else {
                    MULLD(gd, gd, ed);
                }
            } else {
                UFLAG_IF {
                    EXTSW(gd, gd);
                    EXTSW(x3, ed);
                    MULLD(x4, gd, x3);
                    SRDI(x3, x4, 32);
                    EXTSW(gd, x4);
                    SET_DFNONE();
                    IFX (X_CF | X_OF) {
                        XOR(x3, gd, x4);
                        SNEZ(x3, x3);
                        IFX (X_CF) RLDIMI(xFlags, x3, F_CF, 63 - F_CF);
                        IFX (X_OF) RLDIMI(xFlags, x3, F_OF, 63 - F_OF);
                    }
                } else {
                    MULLW(gd, gd, ed);
                }
                ZEROUP(gd);
            }
            IFX (X_SF) {
                SRDI(x3, gd, rex.w ? 63 : 31);
                RLDIMI(xFlags, x3, F_SF, 63 - F_SF);
            }
            IFX (X_PF) emit_pf(dyn, ninst, gd, x3, x4);
            IFX (X_ALL) SPILL_EFLAGS();
            break;

        case 0xB3:
            INST_NAME("BTR Ed, Gd");
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                if (rex.w)
                    SRADI(x1, gd, 6);
                else
                    SRAWI(x1, gd, 5);
                if (!rex.w && !rex.is32bits) { EXTSW(x1, x1); }
                SLADDy(x3, x1, wback, 2 + rex.w);
                LDxw(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
            SRD(x4, ed, x2);
            RLDIMI(xFlags, x4, F_CF, 63 - F_CF);
            LI(x4, 1);
            ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
            SLD(x4, x4, x2);
            ANDC(ed, ed, x4);
            if (wback) {
                SDxw(ed, wback, fixedaddress);
                SMWRITE();
            } else if (!rex.w) {
                ZEROUP(ed);
            }
            break;

        case 0xB6:
            INST_NAME("MOVZX Gd, Eb");
            nextop = F8;
            GETGD;
            SCRATCH_USAGE(0);
            if (MODREG) {
                if (rex.rex) {
                    eb1 = TO_NAT((nextop & 7) + (rex.b << 3));
                    eb2 = 0;
                } else {
                    ed = (nextop & 7);
                    eb1 = TO_NAT(ed & 3); // Ax, Cx, Dx or Bx
                    eb2 = (ed & 4) >> 2;  // L or H
                }
                RLDICL(gd, eb1, 64 - eb2 * 8, 56);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                LBZ(gd, fixedaddress, ed);
            }
            break;
        case 0xB7:
            INST_NAME("MOVZX Gd, Ew");
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                RLDICL(gd, ed, 0, 48);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                LHZ(gd, fixedaddress, ed);
            }
            break;
        case 0xBA:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 4:
                    INST_NAME("BT Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETED(1);
                    u8 = F8;
                    u8 &= rex.w ? 0x3f : 0x1f;
                    RLDICL(x4, ed, 64 - u8, 63);
                    RLDIMI(xFlags, x4, F_CF, 63 - F_CF);
                    break;
                case 5:
                    INST_NAME("BTS Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETED(1);
                    u8 = F8;
                    u8 &= (rex.w ? 0x3f : 0x1f);
                    RLDICL(x4, ed, 64 - u8, 63);
                    RLDIMI(xFlags, x4, F_CF, 63 - F_CF);
                    LI(x4, 1);
                    SLDI(x4, x4, u8);
                    OR(ed, ed, x4);
                    if (wback) {
                        SDxw(ed, wback, fixedaddress);
                        SMWRITE();
                    } else if (!rex.w) {
                        ZEROUP(ed);
                    }
                    break;
                case 6:
                    INST_NAME("BTR Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETED(1);
                    u8 = F8;
                    u8 &= (rex.w ? 0x3f : 0x1f);
                    RLDICL(x4, ed, 64 - u8, 63);
                    RLDIMI(xFlags, x4, F_CF, 63 - F_CF);
                    LI(x4, 1);
                    SLDI(x4, x4, u8);
                    ANDC(ed, ed, x4);
                    if (wback) {
                        SDxw(ed, wback, fixedaddress);
                        SMWRITE();
                    } else if (!rex.w) {
                        ZEROUP(ed);
                    }
                    break;
                case 7:
                    INST_NAME("BTC Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETED(1);
                    u8 = F8;
                    u8 &= rex.w ? 0x3f : 0x1f;
                    RLDICL(x3, ed, 64 - u8, 63);
                    RLDIMI(xFlags, x3, F_CF, 63 - F_CF);
                    if (u8 <= 15) {
                        XORI(ed, ed, (1LL << u8));
                    } else {
                        MOV64xw(x3, (1LL << u8));
                        XOR(ed, ed, x3);
                    }
                    if (wback) {
                        SDxw(ed, wback, fixedaddress);
                        SMWRITE();
                    } else if (!rex.w) {
                        ZEROUP(ed);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xBB:
            INST_NAME("BTC Ed, Gd");
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
                if (rex.w)
                    SRADI(x1, gd, 6);
                else
                    SRAWI(x1, gd, 5);
                if (!rex.w && !rex.is32bits) { EXTSW(x1, x1); }
                SLADDy(x3, x1, wback, 2 + rex.w);
                LDxw(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
            SRD(x4, ed, x2);
            RLDIMI(xFlags, x4, F_CF, 63 - F_CF);
            LI(x4, 1);
            SLD(x4, x4, x2);
            XOR(ed, ed, x4);
            if (wback) {
                SDxw(ed, wback, fixedaddress);
                SMWRITE();
            } else if (!rex.w) {
                ZEROUP(ed);
            }
            break;
        case 0xBC:
            INST_NAME("BSF Gd, Ed");
            if (!BOX64DRENV(dynarec_safeflags)) {
                SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            } else {
                SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            }
            SET_DFNONE();
            CLEAR_FLAGS(x2);
            nextop = F8;
            GETED(0);
            GETGD;
            if (!rex.w && MODREG) {
                ZEROUP2(x4, ed);
                ed = x4;
            }
            BNEZ_MARK(ed);
            IFX (X_ZF) ORI(xFlags, xFlags, 1 << F_ZF);
            B_MARK2_nocond;
            MARK;
            if (rex.w) {
                CNTTZD(gd, ed);
            } else {
                CNTTZW(gd, ed);
            }
            MARK2;
            IFX (BOX64DRENV(dynarec_safeflags)) {
                IFX (X_PF) emit_pf(dyn, ninst, gd, x2, x5);
            }
            SPILL_EFLAGS();
            break;
        case 0xBD:
            INST_NAME("BSR Gd, Ed");
            if (!BOX64DRENV(dynarec_safeflags)) {
                SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            } else {
                SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            }
            SET_DFNONE();
            CLEAR_FLAGS(x2);
            nextop = F8;
            GETED(0);
            GETGD;
            if (!rex.w && MODREG) {
                ZEROUP2(x4, ed);
                ed = x4;
            }
            BNEZ_MARK(ed);
            IFX (X_ZF) ORI(xFlags, xFlags, 1 << F_ZF);
            B_MARK2_nocond;
            MARK;
            if (rex.w) {
                CNTLZD(gd, ed);
            } else {
                CNTLZW(gd, ed);
            }
            LI(x1, rex.w ? 63 : 31);
            SUB(gd, x1, gd);
            MARK2;
            if (BOX64DRENV(dynarec_safeflags)) {
                IFX (X_PF) emit_pf(dyn, ninst, gd, x2, x5);
            }
            SPILL_EFLAGS();
            break;
        case 0xBE:
            INST_NAME("MOVSX Gd, Eb");
            nextop = F8;
            GETGD;
            if (MODREG) {
                if (rex.rex) {
                    wback = TO_NAT((nextop & 7) + (rex.b << 3));
                    wb2 = 0;
                } else {
                    wback = (nextop & 7);
                    wb2 = (wback >> 2) * 8;
                    wback = TO_NAT(wback & 3);
                }
                RLDICL(gd, wback, 64 - wb2, 56);
                EXTSB(gd, gd);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                LBZ(gd, fixedaddress, ed);
                EXTSB(gd, gd);
            }
            if (!rex.w) ZEROUP(gd);
            break;
        case 0xBF:
            INST_NAME("MOVSX Gd, Ew");
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                EXTSH(gd, ed);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                LHA(gd, fixedaddress, ed);
            }
            if (!rex.w) ZEROUP(gd);
            break;

        case 0xC0:
            INST_NAME("XADD Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGB(x1);
            GETEB(x2, 0);
            gd = x2;
            ed = x1;
            emit_add8(dyn, ninst, ed, gd, x4, x5);
            GBBACK();
            EBBACK();
            break;
        case 0xC1:
            INST_NAME("XADD Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            if (ed != gd) MV(x7, ed);
            emit_add32(dyn, ninst, rex, ed, gd, x4, x5, x6);
            if (ed != gd) MVxw(gd, x7);
            WBACK;
            break;
        case 0xC2:
            INST_NAME("CMPPS Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 1);
            u8 = F8;
            switch (u8 & 7) {
                case 0: // EQ
                    XVCMPEQSP(VSXREG(v0), VSXREG(v0), VSXREG(v1));
                    break;
                case 1: // LT
                    XVCMPGTSP(VSXREG(v0), VSXREG(v1), VSXREG(v0));
                    break;
                case 2: // LE
                    XVCMPGESP(VSXREG(v0), VSXREG(v1), VSXREG(v0));
                    break;
                case 3: // UNORD -- true if either is NaN
                    q0 = fpu_get_scratch(dyn);
                    q1 = fpu_get_scratch(dyn);
                    XVCMPEQSP(VSXREG(q0), VSXREG(v0), VSXREG(v0));
                    XVCMPEQSP(VSXREG(q1), VSXREG(v1), VSXREG(v1));
                    XXLAND(VSXREG(v0), VSXREG(q0), VSXREG(q1));
                    XXLNOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    break;
                case 4: // NEQ
                    XVCMPEQSP(VSXREG(v0), VSXREG(v0), VSXREG(v1));
                    XXLNOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    break;
                case 5: // NLT (not less than) = GE or unordered
                    XVCMPGTSP(VSXREG(v0), VSXREG(v1), VSXREG(v0));
                    XXLNOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    break;
                case 6: // NLE (not less or equal) = GT or unordered
                    XVCMPGESP(VSXREG(v0), VSXREG(v1), VSXREG(v0));
                    XXLNOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    break;
                case 7: // ORD -- true if both are not NaN
                    q0 = fpu_get_scratch(dyn);
                    q1 = fpu_get_scratch(dyn);
                    XVCMPEQSP(VSXREG(q0), VSXREG(v0), VSXREG(v0));
                    XVCMPEQSP(VSXREG(q1), VSXREG(v1), VSXREG(v1));
                    XXLAND(VSXREG(v0), VSXREG(q0), VSXREG(q1));
                    break;
            }
            break;
        case 0xC3:
            INST_NAME("MOVNTI Ed, Gd");
            nextop = F8;
            GETGD;
            if (MODREG) {
                MVxw(TO_NAT((nextop & 7) + (rex.b << 3)), gd);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                SDxw(gd, ed, fixedaddress);
            }
            break;
        case 0xC4:
            INST_NAME("PINSRW Gm, Ed, Ib");
            nextop = F8;
            GETGM(v0);
            if (MODREG) {
                u8 = (F8) & 3;
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x4, &fixedaddress, rex, NULL, DS_DISP, 1);
                u8 = (F8) & 3;
                ed = x3;
                LHZ(ed, fixedaddress, wback);
            }
            // Insert 16-bit value into MMX register at position u8 (0-3)
            // MMX data is in VR space (ISA dw1)
            MFVSRLD(x4, VSXREG(v0));  // get current 64-bit value from ISA dw1
            // Clear target halfword and insert new value
            RLWINM(x5, ed, 0, 16, 31);   // zero-extend to 16 bits
            {
                int shift = u8 * 16;
                RLDIMI(x4, x5, shift, 64 - shift - 16);
            }
            MTVSRDD(VSXREG(v0), xZR, x4);
            break;
        case 0xC5:
            INST_NAME("PEXTRW Gd, Em, Ib");
            nextop = F8;
            GETGD;
            if (MODREG) {
                GETEM(v0, 0);
                u8 = (F8) & 3;
                MFVSRLD(x4, VSXREG(v0));  // 64-bit MMX value from ISA dw1
                {
                    int shift = u8 * 16;
                    if (shift)
                        SRDI(gd, x4, shift);
                    else
                        MV(gd, x4);
                    RLWINM(gd, gd, 0, 16, 31); // zero-extend to 16 bits
                }
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x4, &fixedaddress, rex, NULL, DS_DISP, 1);
                u8 = (F8) & 3;
                LHZ(gd, fixedaddress + (u8 << 1), wback);
            }
            break;
        case 0xC6:
            INST_NAME("SHUFPS Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 1);
            u8 = F8;
            // SHUFPS: result[31:0] = Gx[imm[1:0]*32], result[63:32] = Gx[imm[3:2]*32],
            //         result[95:64] = Ex[imm[5:4]*32], result[127:96] = Ex[imm[7:6]*32]
            if (v0 == v1 && u8 == 0xE4) {
                // Tier 0: Identity shuffle — no-op
            } else if (v0 == v1 && ((u8 & 0x3) == ((u8 >> 2) & 3)) && ((u8 & 0xF) == ((u8 >> 4) & 0xF))) {
                // Tier 1: Broadcast single word (all 4 selectors identical, same-reg)
                // UIM2 for XXSPLTW: x86 word w → BE word (3-w) → UIM2 = 3-w
                XXSPLTW(VSXREG(v0), VSXREG(v0), 3 - (u8 & 3));
            } else if ((((u8 & 0x0F) == 0x04) || ((u8 & 0x0F) == 0x0E))
                    && (((u8 & 0xF0) == 0x40) || ((u8 & 0xF0) == 0xE0))) {
                // Tier 2: Doubleword-aligned — use XXPERMDI (1 instruction)
                // Low result half from Gx, high result half from Ex
                // XXPERMDI(XT, XA, XB, DM): dw0 from XA, dw1 from XB
                // XA=Ex (produces ISA dw0 = x86 high), XB=Gx (produces ISA dw1 = x86 low)
                // Low nibble 0x04 = Gx words[0,1] = x86 low qword = ISA dw1 → need XB dw1 → DM[0]=1
                // Low nibble 0x0E = Gx words[2,3] = x86 high qword = ISA dw0 → need XB dw0 → DM[0]=0
                // High nibble 0x40 = Ex words[0,1] = x86 low qword = ISA dw1 → need XA dw1 → DM[1]=1
                // High nibble 0xE0 = Ex words[2,3] = x86 high qword = ISA dw0 → need XA dw0 → DM[1]=0
                int dm = (((u8 & 0xF0) == 0x40) ? 2 : 0) | (((u8 & 0x0F) == 0x04) ? 1 : 0);
                XXPERMDI(VSXREG(v0), VSXREG(v1), VSXREG(v0), dm);
            } else if (v0 != v1) {
                // Tier 3b: General two-source — VPERM avoids VSX↔GPR domain crossings
                // VPERM(VRT, VRA, VRB, VRC): VRT[i] = (VRA||VRB)[VRC[i] & 0x1F]
                // VRA=Ex (indices 0-15), VRB=Gx (indices 16-31)
                // Words 0,1 from Gx → bit4=1 (indices 16+), words 2,3 from Ex → bit4=0
                // PPC BE byte layout: x86 word w → BE word (3-w) at byte offset (3-w)*4
                {
                    d0 = fpu_get_scratch(dyn);
                    uint64_t ctrl_hi = 0, ctrl_lo = 0;
                    for (int i = 0; i < 4; ++i) {
                        int src_word = (u8 >> (i * 2)) & 3;
                        int dest_be_word = 3 - i;
                        int src_be_offset = (3 - src_word) * 4;
                        // Words 0,1 come from Gx (VRB, indices 16-31), words 2,3 from Ex (VRA, indices 0-15)
                        int base = (i < 2) ? 16 : 0;
                        for (int b = 0; b < 4; ++b) {
                            int dest_byte = dest_be_word * 4 + b;
                            uint8_t src_byte = base + src_be_offset + b;
                            if (dest_byte < 8)
                                ctrl_hi |= ((uint64_t)src_byte) << ((7 - dest_byte) * 8);
                            else
                                ctrl_lo |= ((uint64_t)src_byte) << ((15 - dest_byte) * 8);
                        }
                    }
                    TABLE64(x4, ctrl_hi);
                    TABLE64(x5, ctrl_lo);
                    MTVSRDD(VSXREG(d0), x4, x5);
                    VPERM(VRREG(v0), VRREG(v1), VRREG(v0), VRREG(d0));
                }
            } else {
                // Tier 3a: General single-source (Gx==Ex) — GPR extract/reassemble
                {
                    int sel0 = (u8 >> 0) & 3;
                    int sel1 = (u8 >> 2) & 3;
                    int sel2 = (u8 >> 4) & 3;
                    int sel3 = (u8 >> 6) & 3;
                    MFVSRLD(x4, VSXREG(v0));    // x86 low 64 bits (ISA dw1) - words 0,1
                    MFVSRD(x5, VSXREG(v0));     // x86 high 64 bits (ISA dw0) - words 2,3
                    #define EXTRACT_WORD(dst, sel) do {                  \
                        int _reg = ((sel) < 2) ? x4 : x5;              \
                        if (((sel) & 1) == 0)                           \
                            RLWINM(dst, _reg, 0, 0, 31);               \
                        else                                            \
                            SRDI(dst, _reg, 32);                        \
                    } while(0)
                    EXTRACT_WORD(x6, sel0);
                    EXTRACT_WORD(x7, sel1);
                    RLDIMI(x6, x7, 32, 0);
                    EXTRACT_WORD(x3, sel2);
                    EXTRACT_WORD(x7, sel3);
                    RLDIMI(x3, x7, 32, 0);
                    #undef EXTRACT_WORD
                    MTVSRDD(VSXREG(v0), x3, x6);
                }
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
            REVBxw(gd, gd, x1);
            break;

        case 0xD1:
            INST_NAME("PSRLW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            // Variable shift: count from Em low 64 bits. If >= 16, result is 0
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            // Splat Em to all qword lanes (MMX: Em is 64-bit in ISA dw1)
            // For MMX, Em is already just 64 bits. Splat the shift count to all halfword lanes
            // using the qword value and let VMX shift use low bits of each element
            XXPERMDI(VSXREG(q0), VSXREG(v1), VSXREG(v1), 3);  // splat ISA dw1 (MMX data)
            // Check if count > 15
            LI(x4, 15);
            MTVSRDD(VSXREG(q1), x4, x4);
            VCMPGTUD(VRREG(q1), VRREG(q0), VRREG(q1));  // q1 = (count > 15) ? -1 : 0
            VNOR(VRREG(q1), VRREG(q1), VRREG(q1));       // q1 = in-range mask
            VSRH(VRREG(v0), VRREG(v0), VRREG(q0));
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            break;
        case 0xD2:
            INST_NAME("PSRLD Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            XXPERMDI(VSXREG(q0), VSXREG(v1), VSXREG(v1), 3);  // splat ISA dw1 (MMX data)
            LI(x4, 31);
            MTVSRDD(VSXREG(q1), x4, x4);
            VCMPGTUD(VRREG(q1), VRREG(q0), VRREG(q1));
            VNOR(VRREG(q1), VRREG(q1), VRREG(q1));
            VSRW(VRREG(v0), VRREG(v0), VRREG(q0));
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            break;
        case 0xD3:
            INST_NAME("PSRLQ Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            XXPERMDI(VSXREG(q0), VSXREG(v1), VSXREG(v1), 3);  // splat ISA dw1 (MMX data)
            LI(x4, 63);
            MTVSRDD(VSXREG(q1), x4, x4);
            VCMPGTUD(VRREG(q1), VRREG(q0), VRREG(q1));
            VNOR(VRREG(q1), VRREG(q1), VRREG(q1));
            VSRD(VRREG(v0), VRREG(v0), VRREG(q0));
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            break;
        case 0xD4:
            INST_NAME("PADDQ Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VADDUDM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xD5:
            INST_NAME("PMULLW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            q0 = fpu_get_scratch(dyn);
            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
            VMLADDUHM(VRREG(v0), VRREG(v0), VRREG(v1), VRREG(q0));
            break;
        case 0xD7:
            INST_NAME("PMOVMSKB Gd, Em");
            nextop = F8;
            GETGD;
            GETEM(v0, 0);
            // Extract sign bits of 8 bytes from 64-bit MMX register
            // MMX data is in VR space (ISA dw1)
            MFVSRLD(x4, VSXREG(v0));   // ISA dw1 = the 64-bit MMX value
            // Isolate sign bits: x4 & 0x8080808080808080
            LI(x6, 0);
            ORIS(x6, x6, 0x8080);
            ORI(x6, x6, 0x8080);
            RLDIMI(x6, x6, 32, 0);      // x6 = 0x8080808080808080
            AND(x7, x4, x6);
            // Magic multiplier to gather sign bits: 0x0002040810204081
            LIS(x3, 0x0002);
            ORI(x3, x3, 0x0408);
            SLDI(x3, x3, 32);
            ORIS(x3, x3, 0x1020);
            ORI(x3, x3, 0x4081);
            MULLD(x7, x7, x3);
            SRDI(gd, x7, 56);
            break;
        case 0xD8:
            INST_NAME("PSUBUSB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VSUBUBS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xD9:
            INST_NAME("PSUBUSW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VSUBUHS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xDA:
            INST_NAME("PMINUB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VMINUB(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xDB:
            INST_NAME("PAND Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(v1));
            break;
        case 0xDC:
            INST_NAME("PADDUSB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VADDUBS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xDD:
            INST_NAME("PADDUSW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VADDUHS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xDE:
            INST_NAME("PMAXUB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VMAXUB(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xDF:
            INST_NAME("PANDN Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(q0, 0);
            // x86 PANDN: dest = NOT(dest) AND src
            // PPC XXLANDC(T, A, B) = A AND NOT(B)
            XXLANDC(VSXREG(v0), VSXREG(q0), VSXREG(v0));
            break;
        case 0xE0:
            INST_NAME("PAVGB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VAVGUB(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xE1:
            INST_NAME("PSRAW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            q0 = fpu_get_scratch(dyn);
            // Clamp shift count to 15
            MFVSRLD(x4, VSXREG(v1));  // read ISA dw1 (MMX data)
            CMPLDI(x4, 15);
            LI(x5, 15);
            ISEL(x4, x4, x5, 0);     // if x4 < 15 pick x4, else 15
            MTVSRDD(VSXREG(q0), x4, x4);
            VSRAH(VRREG(v0), VRREG(v0), VRREG(q0));
            break;
        case 0xE2:
            INST_NAME("PSRAD Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            q0 = fpu_get_scratch(dyn);
            MFVSRLD(x4, VSXREG(v1));  // read ISA dw1 (MMX data)
            CMPLDI(x4, 31);
            LI(x5, 31);
            ISEL(x4, x4, x5, 0);
            MTVSRDD(VSXREG(q0), x4, x4);
            VSRAW(VRREG(v0), VRREG(v0), VRREG(q0));
            break;
        case 0xE3:
            INST_NAME("PAVGW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VAVGUH(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xE4:
            INST_NAME("PMULHUW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            VMULEUH(VRREG(q0), VRREG(v0), VRREG(v1));
            VMULOUH(VRREG(q1), VRREG(v0), VRREG(v1));
            XXSPLTIB(VSXREG(v0), 16);
            VSRW(VRREG(q0), VRREG(q0), VRREG(v0));
            VSRW(VRREG(q1), VRREG(q1), VRREG(v0));
            // Merge even/odd results and pack: VMRGLW interleaves low dwords
            // so the 4 valid results end up in all 4 dword positions
            VMRGLW(VRREG(v0), VRREG(q1), VRREG(q0));
            VPKUWUM(VRREG(v0), VRREG(v0), VRREG(v0));
            break;
        case 0xE5:
            INST_NAME("PMULHW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            VMULESH(VRREG(q0), VRREG(v0), VRREG(v1));
            VMULOSH(VRREG(q1), VRREG(v0), VRREG(v1));
            XXSPLTIB(VSXREG(v0), 16);
            VSRAW(VRREG(q0), VRREG(q0), VRREG(v0));
            VSRAW(VRREG(q1), VRREG(q1), VRREG(v0));
            // Merge even/odd results and pack: VMRGLW interleaves low dwords
            // so the 4 valid results end up in all 4 dword positions
            VMRGLW(VRREG(v0), VRREG(q1), VRREG(q0));
            VPKUWUM(VRREG(v0), VRREG(v0), VRREG(v0));
            break;
        case 0xE7:
            INST_NAME("MOVNTQ Em, Gm");
            nextop = F8;
            GETGM(v0);
            if (MODREG) {
                v1 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, nextop & 7);
                XXLOR(VSXREG(v1), VSXREG(v0), VSXREG(v0));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, DS_DISP, 0);
                MFVSRLD(x4, VSXREG(v0));
                STD(x4, fixedaddress, ed);
                SMWRITE2();
            }
            break;
        case 0xE8:
            INST_NAME("PSUBSB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VSUBSBS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xE9:
            INST_NAME("PSUBSW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VSUBSHS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xEA:
            INST_NAME("PMINSW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VMINSH(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xEB:
            INST_NAME("POR Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(v1));
            break;
        case 0xEC:
            INST_NAME("PADDSB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VADDSBS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xED:
            INST_NAME("PADDSW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VADDSHS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xEE:
            INST_NAME("PMAXSW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VMAXSH(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xEF:
            INST_NAME("PXOR Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            XXLXOR(VSXREG(v0), VSXREG(v0), VSXREG(v1));
            break;
        case 0xF1:
            INST_NAME("PSLLW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            XXPERMDI(VSXREG(q0), VSXREG(v1), VSXREG(v1), 3);  // splat ISA dw1 (MMX data)
            LI(x4, 15);
            MTVSRDD(VSXREG(q1), x4, x4);
            VCMPGTUD(VRREG(q1), VRREG(q0), VRREG(q1));
            VNOR(VRREG(q1), VRREG(q1), VRREG(q1));
            VSLH(VRREG(v0), VRREG(v0), VRREG(q0));
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            break;
        case 0xF2:
            INST_NAME("PSLLD Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            XXPERMDI(VSXREG(q0), VSXREG(v1), VSXREG(v1), 3);  // splat ISA dw1 (MMX data)
            LI(x4, 31);
            MTVSRDD(VSXREG(q1), x4, x4);
            VCMPGTUD(VRREG(q1), VRREG(q0), VRREG(q1));
            VNOR(VRREG(q1), VRREG(q1), VRREG(q1));
            VSLW(VRREG(v0), VRREG(v0), VRREG(q0));
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            break;
        case 0xF3:
            INST_NAME("PSLLQ Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            XXPERMDI(VSXREG(q0), VSXREG(v1), VSXREG(v1), 3);  // splat ISA dw1 (MMX data)
            LI(x4, 63);
            MTVSRDD(VSXREG(q1), x4, x4);
            VCMPGTUD(VRREG(q1), VRREG(q0), VRREG(q1));
            VNOR(VRREG(q1), VRREG(q1), VRREG(q1));
            VSLD(VRREG(v0), VRREG(v0), VRREG(q0));
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            break;
        case 0xF4:
            INST_NAME("PMULUDQ Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            // Multiply unsigned dwords at even positions -> qwords
            VMULEUW(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xF5:
            INST_NAME("PMADDWD Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            VMULESH(VRREG(q0), VRREG(v0), VRREG(v1));
            VMULOSH(VRREG(q1), VRREG(v0), VRREG(v1));
            VADDUWM(VRREG(v0), VRREG(q0), VRREG(q1));
            break;
        case 0xF6:
            INST_NAME("PSADBW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            // Sum of absolute differences of unsigned bytes
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            VABSDUB(VRREG(q0), VRREG(v0), VRREG(v1));
            // Horizontal sum of bytes in the 64-bit half
            XXLXOR(VSXREG(q1), VSXREG(q1), VSXREG(q1));
            VSUM4UBS(VRREG(q0), VRREG(q0), VRREG(q1));
            // q0 now has 4 word-sized partial sums. Sum pairs within each 64-bit half
            {
                int q2 = fpu_get_scratch(dyn);
                XXSPLTIB(VSXREG(q2), 0);
                VSLDOI(VRREG(q1), VRREG(q2), VRREG(q0), 12);
                VADDUWM(VRREG(q0), VRREG(q0), VRREG(q1));
                // Extract and rebuild with zeroed upper words per qword
                MFVSRLD(x4, VSXREG(q0));
                MFVSRD(x5, VSXREG(q0));
                CLRLDI(x4, x4, 32);
                CLRLDI(x5, x5, 32);
                MTVSRDD(VSXREG(v0), x5, x4);
            }
            break;
        case 0xF7:
            INST_NAME("MASKMOVQ Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            // Create mask from sign bits: shift each byte right by 7 arithmetically
            XXSPLTIB(VSXREG(q1), 7);
            VSRAB(VRREG(q1), VRREG(v1), VRREG(q1));
            // Load current 64-bit data at [RDI]
            LD(x4, 0, xRDI);
            MTVSRDD(VSXREG(q0), xZR, x4);
            // Select: where mask is 0xFF pick v0, else keep q0
            VSEL(VRREG(q0), VRREG(q0), VRREG(v0), VRREG(q1));
            MFVSRLD(x4, VSXREG(q0));
            STD(x4, 0, xRDI);
            break;
        case 0xF8:
            INST_NAME("PSUBB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VSUBUBM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xF9:
            INST_NAME("PSUBW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VSUBUHM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xFA:
            INST_NAME("PSUBD Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VSUBUWM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xFB:
            INST_NAME("PSUBQ Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VSUBUDM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xFC:
            INST_NAME("PADDB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VADDUBM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xFD:
            INST_NAME("PADDW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VADDUHM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xFE:
            INST_NAME("PADDD Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VADDUWM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;

        default:
            DEFAULT;
    }

    return addr;
}
