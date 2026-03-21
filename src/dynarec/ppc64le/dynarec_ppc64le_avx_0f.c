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

uintptr_t dynarec64_AVX_0F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
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
            INST_NAME("VMOVUPS Gx, Ex");
            nextop = F8;
            if (MODREG) {
                GETGY_empty_EY_xy(v0, v1, 0);
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                if (vex.l) {
                    // Copy upper 128 bits: ymm[src] -> ymm[gd]
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
                    // Load upper 128 bits from memory to ymm[gd]
                    q0 = fpu_get_scratch(dyn);
                    LXV(VSXREG(q0), fixedaddress + 16, ed);
                    STXV(VSXREG(q0), offsetof(x64emu_t, ymm[gd]), xEmu);
                }
            }
            break;
        case 0x11:
            INST_NAME("VMOVUPS Ex, Gx");
            nextop = F8;
            GETGYxy(v0, 0);
            if (MODREG) {
                GETEYxy_empty(v1, 0);
                XXLOR(VSXREG(v1), VSXREG(v0), VSXREG(v0));
                if (vex.l) {
                    // Copy upper 128 bits: ymm[gd] -> ymm[dest]
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
                    // Store upper 128 bits from ymm[gd] to memory
                    q0 = fpu_get_scratch(dyn);
                    LXV(VSXREG(q0), offsetof(x64emu_t, ymm[gd]), xEmu);
                    STXV(VSXREG(q0), fixedaddress + 16, ed);
                }
                SMWRITE2();
            }
            break;
        case 0x12:
            nextop = F8;
            if (MODREG) {
                INST_NAME("VMOVHLPS Gx, Vx, Ex");
                GETVYx(v1, 0);
                v2 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
                GETGYx_empty(v0);
                // MOVHLPS: Gx[63:0] = Ex[127:64], Gx[127:64] = Vx[127:64]
                // ISA: Gx.dw1(x86 low) = Ex.dw0(x86 high); Gx.dw0(x86 high) = Vx.dw0(x86 high)
                XXPERMDI(VSXREG(v0), VSXREG(v1), VSXREG(v2), 0b00);
            } else {
                INST_NAME("VMOVLPS Gx, Vx, Ex");
                GETVYx(v1, 0);
                GETGYx_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                // Load 64-bit into x86 low qword (ISA dw1), keep Vx high qword (ISA dw0)
                LD(x4, fixedaddress, ed);
                MFVSRD(x5, VSXREG(v1));  // x5 = ISA dw0 (Vx x86 high)
                MTVSRDD(VSXREG(v0), x5, x4);
            }
            break;

        case 0x13:
            nextop = F8;
            INST_NAME("VMOVLPS Ex, Gx");
            GETGYx(v0, 0);
            if (MODREG) {
                DEFAULT;
                return addr;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                // Store low qword (ISA dw1 = x86 low) to memory
                MFVSRLD(x4, VSXREG(v0));
                STD(x4, fixedaddress, ed);
                SMWRITE2();
            }
            break;

        case 0x14:
            INST_NAME("VUNPCKLPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            // Interleave low words: LE VMRGLW maps to x86 UNPCKLPS
            VMRGLW(VRREG(v0), VRREG(v2), VRREG(v1));
            break;

        case 0x15:
            INST_NAME("VUNPCKHPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            // Interleave high words: LE VMRGHW maps to x86 UNPCKHPS
            VMRGHW(VRREG(v0), VRREG(v2), VRREG(v1));
            break;

        case 0x16:
            nextop = F8;
            if (MODREG) {
                INST_NAME("VMOVLHPS Gx, Vx, Ex");
                GETVYx(v1, 0);
                v2 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
                GETGYx_empty(v0);
                // MOVLHPS: Gx[127:64] = Ex[63:0], Gx[63:0] = Vx[63:0]
                // ISA: Gx.dw0(x86 high) = Ex.dw1(x86 low); Gx.dw1(x86 low) = Vx.dw1(x86 low)
                XXPERMDI(VSXREG(v0), VSXREG(v2), VSXREG(v1), 0b11);
            } else {
                INST_NAME("VMOVHPS Gx, Vx, Ex");
                GETVYx(v1, 0);
                GETGYx_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                // Load 64-bit into x86 high qword (ISA dw0), keep Vx low qword (ISA dw1)
                LD(x4, fixedaddress, ed);
                MFVSRLD(x5, VSXREG(v1));  // x5 = ISA dw1 (Vx x86 low, preserve)
                MTVSRDD(VSXREG(v0), x4, x5);
            }
            break;

        case 0x17:
            nextop = F8;
            INST_NAME("VMOVHPS Ex, Gx");
            GETGYx(v0, 0);
            if (MODREG) {
                DEFAULT;
                return addr;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                // Store high qword (ISA dw0 = x86 high) to memory
                MFVSRD(x4, VSXREG(v0));
                STD(x4, fixedaddress, ed);
                SMWRITE2();
            }
            break;

        case 0x28:
            INST_NAME("VMOVAPS Gx, Ex");
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

        case 0x29:
            INST_NAME("VMOVAPS Ex, Gx");
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

        case 0x2B:
            INST_NAME("VMOVNTPS Ex, Gx");
            nextop = F8;
            GETGYxy(v0, 0);
            if (MODREG) {
                DEFAULT;
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

        case 0x2E:
            // no special check...
        case 0x2F:
            if (opcode == 0x2F) {
                INST_NAME("VCOMISS Gx, Ex");
            } else {
                INST_NAME("VUCOMISS Gx, Ex");
            }
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETGYx(v0, 0);
            // Extract Gx scalar float: x86 float is in ISA word 3 (low 32 bits of ISA dw1)
            d0 = fpu_get_scratch(dyn);
            MFVSRLD(x4, VSXREG(v0));       // x4 = ISA dw1 (x86 low qword)
            SLDI(x4, x4, 32);              // shift float to upper 32 bits
            MTVSRD(VSXREG(d0), x4);         // move to FPR ISA dw0
            XSCVSPDPN(VSXREG(d0), VSXREG(d0));  // convert SP→DP
            // Get Ex operand and extract scalar float
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
                MFVSRLD(x4, VSXREG(v1));
                SLDI(x4, x4, 32);
                MTVSRD(VSXREG(d1), x4);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                LWZ(x4, fixedaddress, ed);
                SLDI(x4, x4, 32);
                MTVSRD(VSXREG(d1), x4);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            }
            CLEAR_FLAGS(x1);
            // Compare scalar double (after SP→DP conversion)
            XSCMPUDP(0, VSXREG(d0), VSXREG(d1));
            // Map PPC CR0 to x86 flags
            MFCR(x1);
            RLWINM(x1, x1, 4, 28, 31);    // CR0 bits to low 4: [LT, GT, EQ, UN]
            // CF = LT|UN
            RLWINM(x2, x1, 0, 31, 31);    // UN bit
            RLWINM(x3, x1, 29, 31, 31);   // LT bit
            OR(x2, x2, x3);
            // PF = UN
            RLWINM(x3, x1, 0, 31, 31);
            SLWI(x3, x3, F_PF);
            OR(x2, x2, x3);
            // ZF = EQ|UN
            RLWINM(x3, x1, 31, 31, 31);   // EQ bit
            RLWINM(x4, x1, 0, 31, 31);    // UN bit
            OR(x3, x3, x4);
            SLWI(x3, x3, F_ZF);
            OR(x2, x2, x3);
            OR(xFlags, xFlags, x2);
            break;

        case 0x50:
            INST_NAME("VMOVMSKPS Gd, Ex");
            nextop = F8;
            GETEYxy(v0, 0, 0);
            GETGD;
            // Extract sign bits of 4 floats
            // x86: bit0 = sign of float[0] (ISA word 3), bit1 = sign of float[1] (ISA word 2),
            //       bit2 = sign of float[2] (ISA word 1), bit3 = sign of float[3] (ISA word 0)
            MFVSRLD(x4, VSXREG(v0));    // x4 = ISA dw1 (x86 low: float[0] in low 32, float[1] in high 32)
            MFVSRD(x5, VSXREG(v0));     // x5 = ISA dw0 (x86 high: float[2] in low 32, float[3] in high 32)
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
            INST_NAME("VSQRTPS Gx, Ex");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn);
                d0 = fpu_get_scratch(dyn);
                XVCMPEQSP(VSXREG(q0), VSXREG(v1), VSXREG(v1));  // -1 where input NOT NaN
            }
            XVSQRTSP(VSXREG(v0), VSXREG(v1));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQSP(VSXREG(d0), VSXREG(v0), VSXREG(v0));  // -1 where result NOT NaN
                XXLANDC(VSXREG(d0), VSXREG(q0), VSXREG(d0));    // input-ordered AND result-NaN
                XXSPLTIB(VSXREG(q0), 31);
                VSLW(VRREG(d0), VRREG(d0), VRREG(q0));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(d0));
            }
            break;

        case 0x52:
            INST_NAME("VRSQRTPS Gx, Ex");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 0);
            XVRSQRTESP(VSXREG(v0), VSXREG(v1));
            break;

        case 0x53:
            INST_NAME("VRCPPS Gx, Ex");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 0);
            XVRESP(VSXREG(v0), VSXREG(v1));
            break;

        case 0x54:
            INST_NAME("VANDPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            XXLAND(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            break;

        case 0x55:
            INST_NAME("VANDNPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            // x86: dest = NOT(Vx) AND Ex
            // PPC XXLANDC(T, A, B) = A AND NOT(B)
            XXLANDC(VSXREG(v0), VSXREG(v2), VSXREG(v1));
            break;

        case 0x56:
            INST_NAME("VORPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            break;

        case 0x57:
            INST_NAME("VXORPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            XXLXOR(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            break;

        case 0x58:
            INST_NAME("VADDPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn);
                q1 = fpu_get_scratch(dyn);
                XVCMPEQSP(VSXREG(q0), VSXREG(v1), VSXREG(v1));
                XVCMPEQSP(VSXREG(q1), VSXREG(v2), VSXREG(v2));
                XXLAND(VSXREG(q0), VSXREG(q0), VSXREG(q1));
            }
            XVADDSP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQSP(VSXREG(q1), VSXREG(v0), VSXREG(v0));
                XXLANDC(VSXREG(q1), VSXREG(q0), VSXREG(q1));
                XXSPLTIB(VSXREG(q0), 31);
                VSLW(VRREG(q1), VRREG(q1), VRREG(q0));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            }
            break;

        case 0x59:
            INST_NAME("VMULPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn);
                q1 = fpu_get_scratch(dyn);
                XVCMPEQSP(VSXREG(q0), VSXREG(v1), VSXREG(v1));
                XVCMPEQSP(VSXREG(q1), VSXREG(v2), VSXREG(v2));
                XXLAND(VSXREG(q0), VSXREG(q0), VSXREG(q1));
            }
            XVMULSP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQSP(VSXREG(q1), VSXREG(v0), VSXREG(v0));
                XXLANDC(VSXREG(q1), VSXREG(q0), VSXREG(q1));
                XXSPLTIB(VSXREG(q0), 31);
                VSLW(VRREG(q1), VRREG(q1), VRREG(q0));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            }
            break;

        case 0x5A:
            INST_NAME("VCVTPS2PD Gx, Ex");
            nextop = F8;
            if (vex.l) {
                GETEYx(v1, 0, 0);
            } else {
                GETEYSD(v1, 0, 0);
            }
            GETGYxy_empty(v0);
            // Convert packed singles to packed doubles
            // Use scalar approach: extract each float, convert SP→DP, combine
            {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                // Extract x86 float[0] (low 32 bits of ISA dw1)
                MFVSRLD(x4, VSXREG(v1));       // x4 = ISA dw1 (x86 float[1]:float[0])
                SLDI(x5, x4, 32);              // x5 = float[0] in upper 32
                MTVSRD(VSXREG(d0), x5);
                XSCVSPDPN(VSXREG(d0), VSXREG(d0));  // d0 = (double)float[0]
                // Extract x86 float[1] (high 32 bits of ISA dw1)
                RLDICR(x5, x4, 0, 31);         // clear low 32, keep high 32 (float[1])
                MTVSRD(VSXREG(d1), x5);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));  // d1 = (double)float[1]
                // Combine: ISA dw0 = x86 double[1] = d1, ISA dw1 = x86 double[0] = d0
                MFVSRD(x4, VSXREG(d1));         // x4 = double[1] for ISA dw0
                MFVSRD(x5, VSXREG(d0));         // x5 = double[0] for ISA dw1
                MTVSRDD(VSXREG(v0), x4, x5);
            }
            break;

        case 0x5B:
            INST_NAME("VCVTDQ2PS Gx, Ex");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 0);
            // Convert 4 packed signed int32 to 4 packed single-precision floats
            XVCVSXWSP(VSXREG(v0), VSXREG(v1));
            break;

        case 0x5C:
            INST_NAME("VSUBPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn);
                q1 = fpu_get_scratch(dyn);
                XVCMPEQSP(VSXREG(q0), VSXREG(v1), VSXREG(v1));
                XVCMPEQSP(VSXREG(q1), VSXREG(v2), VSXREG(v2));
                XXLAND(VSXREG(q0), VSXREG(q0), VSXREG(q1));
            }
            XVSUBSP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQSP(VSXREG(q1), VSXREG(v0), VSXREG(v0));
                XXLANDC(VSXREG(q1), VSXREG(q0), VSXREG(q1));
                XXSPLTIB(VSXREG(q0), 31);
                VSLW(VRREG(q1), VRREG(q1), VRREG(q0));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            }
            break;

        case 0x5D:
            INST_NAME("VMINPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            // x86 MINPS: if either is NaN or both equal, return src2 (Ex)
            q0 = fpu_get_scratch(dyn);
            XVCMPGTSP(VSXREG(q0), VSXREG(v2), VSXREG(v1));
            XXSEL(VSXREG(v0), VSXREG(v2), VSXREG(v1), VSXREG(q0));
            break;

        case 0x5E:
            INST_NAME("VDIVPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn);
                q1 = fpu_get_scratch(dyn);
                XVCMPEQSP(VSXREG(q0), VSXREG(v1), VSXREG(v1));
                XVCMPEQSP(VSXREG(q1), VSXREG(v2), VSXREG(v2));
                XXLAND(VSXREG(q0), VSXREG(q0), VSXREG(q1));
            }
            XVDIVSP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQSP(VSXREG(q1), VSXREG(v0), VSXREG(v0));
                XXLANDC(VSXREG(q1), VSXREG(q0), VSXREG(q1));
                XXSPLTIB(VSXREG(q0), 31);
                VSLW(VRREG(q1), VRREG(q1), VRREG(q0));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            }
            break;

        case 0x5F:
            INST_NAME("VMAXPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            // x86 MAXPS: if either is NaN, return src2 (Ex)
            q0 = fpu_get_scratch(dyn);
            XVCMPGTSP(VSXREG(q0), VSXREG(v1), VSXREG(v2));
            XXSEL(VSXREG(v0), VSXREG(v2), VSXREG(v1), VSXREG(q0));
            break;

        case 0xC2:
            INST_NAME("VCMPPS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8;
            switch (u8 & 0xf) {
                case 0x00: // EQ_OQ
                    XVCMPEQSP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
                    break;
                case 0x01: // LT_OS
                    XVCMPGTSP(VSXREG(v0), VSXREG(v2), VSXREG(v1));
                    break;
                case 0x02: // LE_OS
                    XVCMPGESP(VSXREG(v0), VSXREG(v2), VSXREG(v1));
                    break;
                case 0x03: // UNORD_Q
                    q0 = fpu_get_scratch(dyn);
                    q1 = fpu_get_scratch(dyn);
                    XVCMPEQSP(VSXREG(q0), VSXREG(v1), VSXREG(v1));
                    XVCMPEQSP(VSXREG(q1), VSXREG(v2), VSXREG(v2));
                    XXLAND(VSXREG(v0), VSXREG(q0), VSXREG(q1));
                    XXLNOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    break;
                case 0x04: // NEQ_UQ
                    XVCMPEQSP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
                    XXLNOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    break;
                case 0x05: // NLT_US (GE or unordered)
                    XVCMPGTSP(VSXREG(v0), VSXREG(v2), VSXREG(v1));
                    XXLNOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    break;
                case 0x06: // NLE_US (GT or unordered)
                    XVCMPGESP(VSXREG(v0), VSXREG(v2), VSXREG(v1));
                    XXLNOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    break;
                case 0x07: // ORD_Q
                    q0 = fpu_get_scratch(dyn);
                    q1 = fpu_get_scratch(dyn);
                    XVCMPEQSP(VSXREG(q0), VSXREG(v1), VSXREG(v1));
                    XVCMPEQSP(VSXREG(q1), VSXREG(v2), VSXREG(v2));
                    XXLAND(VSXREG(v0), VSXREG(q0), VSXREG(q1));
                    break;
                case 0x08: // EQ_UQ
                    q0 = fpu_get_scratch(dyn);
                    q1 = fpu_get_scratch(dyn);
                    XVCMPEQSP(VSXREG(q0), VSXREG(v1), VSXREG(v1));
                    XVCMPEQSP(VSXREG(q1), VSXREG(v2), VSXREG(v2));
                    XXLAND(VSXREG(q0), VSXREG(q0), VSXREG(q1));       // both ordered
                    XVCMPEQSP(VSXREG(v0), VSXREG(v1), VSXREG(v2));    // equal
                    XXLNOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));       // unordered
                    XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(q0));        // equal OR unordered
                    break;
                case 0x09: // NGE_US (LT or unordered)
                    XVCMPGESP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
                    XXLNOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    break;
                case 0x0A: // NGT_US (LE or unordered)
                    XVCMPGTSP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
                    XXLNOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    break;
                case 0x0B: // FALSE_OQ
                    XXLXOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    break;
                case 0x0C: // NEQ_OQ (not equal AND ordered)
                    q0 = fpu_get_scratch(dyn);
                    q1 = fpu_get_scratch(dyn);
                    XVCMPEQSP(VSXREG(q0), VSXREG(v1), VSXREG(v1));
                    XVCMPEQSP(VSXREG(q1), VSXREG(v2), VSXREG(v2));
                    XXLAND(VSXREG(q0), VSXREG(q0), VSXREG(q1));       // both ordered
                    XVCMPEQSP(VSXREG(v0), VSXREG(v1), VSXREG(v2));    // equal
                    XXLANDC(VSXREG(v0), VSXREG(q0), VSXREG(v0));      // ordered AND NOT equal
                    break;
                case 0x0D: // GE_OS
                    XVCMPGESP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
                    break;
                case 0x0E: // GT_OS
                    XVCMPGTSP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
                    break;
                case 0x0F: // TRUE_UQ
                    VCMPEQUB(VRREG(v0), VRREG(v0), VRREG(v0));  // all ones
                    break;
            }
            break;

        case 0xC6:
            INST_NAME("VSHUFPS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8;
            // SHUFPS: result[31:0] = Vx[imm[1:0]*32], result[63:32] = Vx[imm[3:2]*32],
            //         result[95:64] = Ex[imm[5:4]*32], result[127:96] = Ex[imm[7:6]*32]
            {
                int sel0 = (u8 >> 0) & 3;
                int sel1 = (u8 >> 2) & 3;
                int sel2 = (u8 >> 4) & 3;
                int sel3 = (u8 >> 6) & 3;
                // Extract Vx halves to GPRs
                MFVSRLD(x4, VSXREG(v1));    // x86 low 64 bits (ISA dw1) - words 0,1
                MFVSRD(x5, VSXREG(v1));     // x86 high 64 bits (ISA dw0) - words 2,3
                // In LE GPR: x4 bits [31:0]=word0, [63:32]=word1
                //             x5 bits [31:0]=word2, [63:32]=word3
                #define EXTRACT_WORD(dst, sel) do {                  \
                    int _reg = ((sel) < 2) ? x4 : x5;              \
                    if (((sel) & 1) == 0)                           \
                        RLWINM(dst, _reg, 0, 0, 31);               \
                    else                                            \
                        SRDI(dst, _reg, 32);                        \
                } while(0)

                EXTRACT_WORD(x6, sel0);
                EXTRACT_WORD(x7, sel1);
                RLDIMI(x6, x7, 32, 0);  // x6 = x86 result low qword

                // Extract Ex halves for sel2, sel3
                if (v1 != v2) {
                    MFVSRLD(x4, VSXREG(v2));
                    MFVSRD(x5, VSXREG(v2));
                }
                EXTRACT_WORD(x3, sel2);
                EXTRACT_WORD(x7, sel3);
                RLDIMI(x3, x7, 32, 0);  // x3 = x86 result high qword

                #undef EXTRACT_WORD
                // Assemble: x3 = ISA dw0 (x86 high), x6 = ISA dw1 (x86 low)
                MTVSRDD(VSXREG(v0), x3, x6);
            }
            break;

        case 0x77:
            if (!vex.l) {
                INST_NAME("VZEROUPPER");
                if (vex.v != 0) {
                    UDF(0);
                } else {
                    q0 = fpu_get_scratch(dyn);
                    XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
                    for (int i = 0; i < (rex.is32bits ? 8 : 16); ++i) {
                        if (dyn->v.avxcache[i].v != -1) {
                            // AVX used register
                            if (dyn->v.avxcache[i].width == VMX_AVX_WIDTH_256) {
                                // 256 width, store zero to upper 128 bits in memory
                                q1 = avx_get_reg(dyn, ninst, x1, i, 1, VMX_AVX_WIDTH_256);
                                STXV(VSXREG(q0), offsetof(x64emu_t, ymm[i]), xEmu);
                            } else {
                                // 128 width, lazy save
                                q1 = avx_get_reg(dyn, ninst, x1, i, 1, VMX_AVX_WIDTH_128);
                                dyn->v.avxcache[i].zero_upper = 1;
                            }
                        } else {
                            // SSE register or unused register, store 128bit zero to x64emu_t.ymm[]
                            STXV(VSXREG(q0), offsetof(x64emu_t, ymm[i]), xEmu);
                        }
                    }
                    SMWRITE2();
                }
            } else {
                INST_NAME("VZEROALL");
                if (vex.v != 0) {
                    UDF(0);
                } else {
                    for (int i = 0; i < (rex.is32bits ? 8 : 16); ++i) {
                        q0 = avx_get_reg_empty(dyn, ninst, x1, i, VMX_AVX_WIDTH_256);
                        XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
                    }
                }
            }
            break;

        case 0xAE:
            nextop = F8;
            if (MODREG) {
                DEFAULT;
            } else
                switch ((nextop >> 3) & 7) {
                    case 2:
                        INST_NAME("VLDMXCSR Md");
                        GETED(0);
                        STW(ed, offsetof(x64emu_t, mxcsr), xEmu);
                        break;
                    case 3:
                        INST_NAME("VSTMXCSR Md");
                        addr = geted(dyn, addr, ninst, nextop, &wback, x1, x2, &fixedaddress, rex, NULL, NO_DISP, 0);
                        LWZ(x4, offsetof(x64emu_t, mxcsr), xEmu);
                        STW(x4, fixedaddress, wback);
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
