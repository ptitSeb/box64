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

uintptr_t dynarec64_AVX_66_0F3A(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
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
        case 0x00:
        case 0x01:
            if (opcode == 0x01) {
                INST_NAME("VPERMPD Gx, Ex, Imm8");
            } else {
                INST_NAME("VPERMQ Gx, Ex, Imm8");
            }
            nextop = F8;
            if (!vex.l) { DEFAULT; break; }
            {
                // 256-bit only. Source is Ex (4 qwords), permuted by imm8.
                // Ex lower half (qwords 0,1) in VSX reg. Ex upper half (qwords 2,3) in ymm[] memory.
                // PPC64LE ISA layout: ISA dw0 = bits 0:63 = LE qword 1, ISA dw1 = bits 64:127 = LE qword 0
                int ex_reg = -1;
                GETEYy(v1, 0, 1);
                if (MODREG) ex_reg = (nextop & 7) + (rex.b << 3);
                GETGYy_empty(v0);
                u8 = F8;
                // Load the upper half of Ex into a scratch register
                d0 = fpu_get_scratch(dyn);
                if (MODREG) {
                    LXV(VSXREG(d0), offsetof(x64emu_t, ymm[ex_reg]), xEmu);
                } else {
                    LXV(VSXREG(d0), fixedaddress + 16, ed);
                }
                // Now v1 = Ex lower (q0=ISA dw1, q1=ISA dw0), d0 = Ex upper (q2=ISA dw1, q3=ISA dw0)
                // Source array: src[0]=v1 ISA dw1, src[1]=v1 ISA dw0, src[2]=d0 ISA dw1, src[3]=d0 ISA dw0
                // For output lower half:
                //   ISA dw1 (q0) = src[imm8[1:0]]
                //   ISA dw0 (q1) = src[imm8[3:2]]
                // For output upper half:
                //   ISA dw1 (q2) = src[imm8[5:4]]
                //   ISA dw0 (q3) = src[imm8[7:6]]

                // Helper: map source index 0-3 to (register, XXPERMDI bit)
                // 0 → v1, select ISA dw1 (bit=1)
                // 1 → v1, select ISA dw0 (bit=0)
                // 2 → d0, select ISA dw1 (bit=1)
                // 3 → d0, select ISA dw0 (bit=0)
                #define SRC_REG(idx) (((idx) < 2) ? v1 : d0)
                #define SRC_DW(idx)  (((idx) & 1) ? 0 : 1)  // 0→dw1(bit=1), 1→dw0(bit=0), 2→dw1(bit=1), 3→dw0(bit=0)

                int sel0 = (u8 >> 0) & 3;  // for output q0 = ISA dw1 of lower
                int sel1 = (u8 >> 2) & 3;  // for output q1 = ISA dw0 of lower
                int sel2 = (u8 >> 4) & 3;  // for output q2 = ISA dw1 of upper
                int sel3 = (u8 >> 6) & 3;  // for output q3 = ISA dw0 of upper

                // Build output lower half (v0):
                // XXPERMDI(T, A, B, DM): T dw0 = A[DM_hi], T dw1 = B[DM_lo]
                // We want: v0 ISA dw0 = src[sel1], v0 ISA dw1 = src[sel0]
                // So A provides dw0 → A=SRC_REG(sel1), DM_hi=SRC_DW(sel1)
                //    B provides dw1 → B=SRC_REG(sel0), DM_lo=SRC_DW(sel0)
                XXPERMDI(VSXREG(v0), VSXREG(SRC_REG(sel1)), VSXREG(SRC_REG(sel0)), (SRC_DW(sel1) << 1) | SRC_DW(sel0));

                // Build output upper half → store to ymm[gd]
                d1 = fpu_get_scratch(dyn);
                XXPERMDI(VSXREG(d1), VSXREG(SRC_REG(sel3)), VSXREG(SRC_REG(sel2)), (SRC_DW(sel3) << 1) | SRC_DW(sel2));
                STXV(VSXREG(d1), offsetof(x64emu_t, ymm[gd]), xEmu);
                #undef SRC_REG
                #undef SRC_DW
            }
            break;
        case 0x02:
            INST_NAME("VPBLENDD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8 & 0xF;
            if (u8 == 0x0) {
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else if (u8 == 0xF) {
                if (v0 != v2) XXLOR(VSXREG(v0), VSXREG(v2), VSXREG(v2));
            } else {
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                d0 = fpu_get_scratch(dyn);
                if ((u8 & 0x3) == 0x3) {
                    XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(v2), 0b01);
                    u8 &= ~0x3;
                }
                if ((u8 & 0xC) == 0xC) {
                    XXPERMDI(VSXREG(v0), VSXREG(v2), VSXREG(v0), 0b01);
                    u8 &= ~0xC;
                }
                for (int i = 0; i < 4; ++i) {
                    if (u8 & (1 << i)) {
                        int boff = (3 - i) * 4;
                        VEXTRACTUW(VRREG(d0), VRREG(v2), boff);
                        VINSERTW(VRREG(v0), VRREG(d0), boff);
                    }
                }
            }
            break;

        case 0x04:
            INST_NAME("VPERMILPS Gx, Ex, Imm8");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 1);
            u8 = F8;
            {
                // Immediate shuffle of 4 floats within 128-bit lane
                // u8 encodes: word3[7:6], word2[5:4], word1[3:2], word0[1:0]
                // Build a VPERM byte control vector
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                // Each x86 word i takes from source word (u8 >> (i*2)) & 3
                // In PPC BE layout: x86 word i = BE word (3-i) at byte offset (3-i)*4
                // Source word j = bytes at offset (3-j)*4 .. (3-j)*4+3
                // Build 16-byte VPERM control: for each dest byte, give source byte index (BE)
                uint64_t ctrl_hi = 0, ctrl_lo = 0;
                for (int i = 0; i < 4; ++i) {
                    int src_word = (u8 >> (i * 2)) & 3;
                    int dest_be_word = 3 - i;
                    int src_be_offset = (3 - src_word) * 4;
                    for (int b = 0; b < 4; ++b) {
                        int dest_byte = dest_be_word * 4 + b;
                        uint8_t src_byte = src_be_offset + b;
                        if (dest_byte < 8)
                            ctrl_hi |= ((uint64_t)src_byte) << ((7 - dest_byte) * 8);
                        else
                            ctrl_lo |= ((uint64_t)src_byte) << ((15 - dest_byte) * 8);
                    }
                }
                LI(x4, ctrl_hi >> 48);
                SLDI(x4, x4, 16);
                ORI(x4, x4, (ctrl_hi >> 32) & 0xFFFF);
                SLDI(x4, x4, 16);
                ORI(x4, x4, (ctrl_hi >> 16) & 0xFFFF);
                SLDI(x4, x4, 16);
                ORI(x4, x4, ctrl_hi & 0xFFFF);
                LI(x5, ctrl_lo >> 48);
                SLDI(x5, x5, 16);
                ORI(x5, x5, (ctrl_lo >> 32) & 0xFFFF);
                SLDI(x5, x5, 16);
                ORI(x5, x5, (ctrl_lo >> 16) & 0xFFFF);
                SLDI(x5, x5, 16);
                ORI(x5, x5, ctrl_lo & 0xFFFF);
                MTVSRDD(VSXREG(d0), x4, x5);
                VPERM(VRREG(v0), VRREG(v1), VRREG(v1), VRREG(d0));
            }
            break;

        case 0x05:
            INST_NAME("VPERMILPD Gx, Ex, Imm8");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 1);
            u8 = F8;
            {
                // Bit 0 selects which qword goes to x86 qword 0 (LE qword 0 = ISA dw1)
                // Bit 1 selects which qword goes to x86 qword 1 (LE qword 1 = ISA dw0)
                int sel0 = u8 & 1;        // source for x86 qword 0
                int sel1 = (u8 >> 1) & 1; // source for x86 qword 1
                // ISA dw0 = x86 qword 1, ISA dw1 = x86 qword 0
                // XXPERMDI selects: bits 0:63 from (bit1 ? B's dw1 : A's dw0), bits 64:127 from (bit0 ? B's dw1 : A's dw0)
                // We want:
                //   ISA dw0 (x86 q1) = v1's x86 qword sel1 = v1's ISA dw(1-sel1)
                //   ISA dw1 (x86 q0) = v1's x86 qword sel0 = v1's ISA dw(1-sel0)
                // XXPERMDI(T, A, B, imm): T[0:63] = A[imm_hi*64..], T[64:127] = B[imm_lo*64..]
                // Here A=B=v1: imm_hi selects ISA dw for T's ISA dw0, imm_lo selects for T's ISA dw1
                // imm_hi = 1-sel1 (0 selects ISA dw0, 1 selects ISA dw1)
                // imm_lo = 1-sel0
                int imm = ((1 - sel1) << 1) | (1 - sel0);
                if (imm == 0b00 && v0 == v1) {
                    // Identity permute, nothing to do
                } else {
                    XXPERMDI(VSXREG(v0), VSXREG(v1), VSXREG(v1), imm);
                }
            }
            break;

        case 0x06:
            INST_NAME("VPERM2F128 Gx, Vx, Ex, Imm8");
            nextop = F8;
            if (!vex.l) { DEFAULT; break; }
            {
                // VPERM2F128: select two 128-bit lanes from Vx and Ex to form 256-bit Gx
                // imm8[1:0] selects source for result low 128: 0=Vx.lo, 1=Vx.hi, 2=Ex.lo, 3=Ex.hi
                // imm8[3] = zero result low 128
                // imm8[5:4] selects source for result high 128: same encoding
                // imm8[7] = zero result high 128
                int ex_reg = -1;
                GETGY_empty_VYEY_xy(v0, v1, v2, 1);
                if (MODREG) ex_reg = (nextop & 7) + (rex.b << 3);
                u8 = F8;
                uint8_t zero_lo = (u8 >> 3) & 1;
                uint8_t zero_hi = (u8 >> 7) & 1;
                uint8_t sel_lo = u8 & 3;        // source lane for result low 128
                uint8_t sel_hi = (u8 >> 4) & 3; // source lane for result high 128

                // Load both selected lanes into scratch registers first, then write to dest.
                // This avoids aliasing issues when gd == vex.v or gd == ex_reg.
                d0 = fpu_get_scratch(dyn);  // will hold result low 128
                d1 = fpu_get_scratch(dyn);  // will hold result high 128

                // Load selected lane for result low 128
                if (zero_lo) {
                    XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));
                } else {
                    switch (sel_lo) {
                        case 0: XXLOR(VSXREG(d0), VSXREG(v1), VSXREG(v1)); break;
                        case 1: LXV(VSXREG(d0), offsetof(x64emu_t, ymm[vex.v]), xEmu); break;
                        case 2: XXLOR(VSXREG(d0), VSXREG(v2), VSXREG(v2)); break;
                        case 3:
                            if (MODREG)
                                LXV(VSXREG(d0), offsetof(x64emu_t, ymm[ex_reg]), xEmu);
                            else
                                LXV(VSXREG(d0), fixedaddress + 16, ed);
                            break;
                    }
                }

                // Load selected lane for result high 128
                if (zero_hi) {
                    XXLXOR(VSXREG(d1), VSXREG(d1), VSXREG(d1));
                } else {
                    switch (sel_hi) {
                        case 0: XXLOR(VSXREG(d1), VSXREG(v1), VSXREG(v1)); break;
                        case 1: LXV(VSXREG(d1), offsetof(x64emu_t, ymm[vex.v]), xEmu); break;
                        case 2: XXLOR(VSXREG(d1), VSXREG(v2), VSXREG(v2)); break;
                        case 3:
                            if (MODREG)
                                LXV(VSXREG(d1), offsetof(x64emu_t, ymm[ex_reg]), xEmu);
                            else
                                LXV(VSXREG(d1), fixedaddress + 16, ed);
                            break;
                    }
                }

                // Write results
                XXLOR(VSXREG(v0), VSXREG(d0), VSXREG(d0));
                STXV(VSXREG(d1), offsetof(x64emu_t, ymm[gd]), xEmu);
            }
            break;

        case 0x08:
            INST_NAME("VROUNDPS Gx, Ex, Ib");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 1);
            u8 = F8;
            if (u8 & 4) {
                // Use current rounding mode
                XVRSPIC(VSXREG(v0), VSXREG(v1));
            } else {
                switch (u8 & 3) {
                    case 0:
                        MFFS(SCRATCH0);
                        MTFSFI(7, 0);
                        XVRSPIC(VSXREG(v0), VSXREG(v1));
                        MTFSF(0x01, SCRATCH0);
                        break;
                    case 1: XVRSPIM(VSXREG(v0), VSXREG(v1)); break;  // floor
                    case 2: XVRSPIP(VSXREG(v0), VSXREG(v1)); break;  // ceil
                    case 3: XVRSPIZ(VSXREG(v0), VSXREG(v1)); break;  // trunc
                }
            }
            break;

        case 0x09:
            INST_NAME("VROUNDPD Gx, Ex, Ib");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 1);
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
            INST_NAME("VROUNDSS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETVYx(v1, 0);
            d0 = fpu_get_scratch(dyn);
            if (MODREG) {
                v2 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
                GETGYx_empty(v0);
                u8 = F8;
                // Round all 4 singles in v2, take element 0
                if (u8 & 4) {
                    XVRSPIC(VSXREG(d0), VSXREG(v2));
                } else {
                    switch (u8 & 3) {
                        case 0:
                            MFFS(SCRATCH0);
                            MTFSFI(7, 0);
                            XVRSPIC(VSXREG(d0), VSXREG(v2));
                            MTFSF(0x01, SCRATCH0);
                            break;
                        case 1: XVRSPIM(VSXREG(d0), VSXREG(v2)); break;
                        case 2: XVRSPIP(VSXREG(d0), VSXREG(v2)); break;
                        case 3: XVRSPIZ(VSXREG(d0), VSXREG(v2)); break;
                    }
                }
                // Extract from LE word 0 = BE byte offset 12
                VEXTRACTUW(VRREG(d0), VRREG(d0), 12);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, DS_DISP, 1);
                GETGYx_empty(v0);
                q0 = fpu_get_scratch(dyn);
                LWZ(x4, fixedaddress, ed);
                SLDI(x4, x4, 32);
                MTVSRD(VSXREG(q0), x4);
                XSCVSPDPN(VSXREG(q0), VSXREG(q0));
                u8 = F8;
                // Scalar double round
                if (u8 & 4) {
                    XSRDPIC(VSXREG(d0), VSXREG(q0));
                } else {
                    switch (u8 & 3) {
                        case 0:
                            MFFS(SCRATCH0);
                            MTFSFI(7, 0);
                            XSRDPIC(VSXREG(d0), VSXREG(q0));
                            MTFSF(0x01, SCRATCH0);
                            break;
                        case 1: XSRDPIM(VSXREG(d0), VSXREG(q0)); break;
                        case 2: XSRDPIP(VSXREG(d0), VSXREG(q0)); break;
                        case 3: XSRDPIZ(VSXREG(d0), VSXREG(q0)); break;
                    }
                }
                // Convert rounded double back to single; result in BE word 0 (byte offset 0)
                XSCVDPSP(VSXREG(d0), VSXREG(d0));
                // Extract from byte offset 0
                VEXTRACTUW(VRREG(d0), VRREG(d0), 0);
            }
            // Copy Vx to Gx, then insert rounded scalar into LE word 0
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            VINSERTW(VRREG(v0), VRREG(d0), 12);
            break;

        case 0x0B:
            INST_NAME("VROUNDSD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETVYx(v1, 0);
            d0 = fpu_get_scratch(dyn);
            if (MODREG) {
                v2 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
                GETGYx_empty(v0);
                u8 = F8;
                // Round both doubles in v2, take element 0
                if (u8 & 4) {
                    XVRDPIC(VSXREG(d0), VSXREG(v2));
                } else {
                    switch (u8 & 3) {
                        case 0:
                            MFFS(SCRATCH0);
                            MTFSFI(7, 0);
                            XVRDPIC(VSXREG(d0), VSXREG(v2));
                            MTFSF(0x01, SCRATCH0);
                            break;
                        case 1: XVRDPIM(VSXREG(d0), VSXREG(v2)); break;
                        case 2: XVRDPIP(VSXREG(d0), VSXREG(v2)); break;
                        case 3: XVRDPIZ(VSXREG(d0), VSXREG(v2)); break;
                    }
                }
                // Copy Vx to Gx, merge LE dword 0 from d0
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d0), 0b01);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, DS_DISP, 1);
                GETGYx_empty(v0);
                q0 = fpu_get_scratch(dyn);
                LD(x4, fixedaddress, ed);
                MTVSRD(VSXREG(q0), x4);
                u8 = F8;
                // Scalar double round (operates on bits 0:63 = ISA dw0)
                if (u8 & 4) {
                    XSRDPIC(VSXREG(d0), VSXREG(q0));
                } else {
                    switch (u8 & 3) {
                        case 0:
                            MFFS(SCRATCH0);
                            MTFSFI(7, 0);
                            XSRDPIC(VSXREG(d0), VSXREG(q0));
                            MTFSF(0x01, SCRATCH0);
                            break;
                        case 1: XSRDPIM(VSXREG(d0), VSXREG(q0)); break;
                        case 2: XSRDPIP(VSXREG(d0), VSXREG(q0)); break;
                        case 3: XSRDPIZ(VSXREG(d0), VSXREG(q0)); break;
                    }
                }
                // Result in d0 bits 0:63. Copy Vx, insert into LE dword 0
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d0), 0b00);
            }
            break;

        case 0x0C:
            INST_NAME("VBLENDPS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8 & 0xF;
            if (u8 == 0x0) {
                // No blend, keep Vx
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else if (u8 == 0xF) {
                // Full copy from Ex
                if (v0 != v2) XXLOR(VSXREG(v0), VSXREG(v2), VSXREG(v2));
            } else {
                // Start with Vx
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                d0 = fpu_get_scratch(dyn);
                // Doubleword-aligned pairs
                if ((u8 & 0x3) == 0x3) {
                    // x86 words 0,1 → LE words 0,1 → ISA dw1 (bits 64:127)
                    XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(v2), 0b01);
                    u8 &= ~0x3;
                }
                if ((u8 & 0xC) == 0xC) {
                    // x86 words 2,3 → LE words 2,3 → ISA dw0 (bits 0:63)
                    XXPERMDI(VSXREG(v0), VSXREG(v2), VSXREG(v0), 0b01);
                    u8 &= ~0xC;
                }
                // Individual words
                for (int i = 0; i < 4; ++i) {
                    if (u8 & (1 << i)) {
                        int boff = (3 - i) * 4;
                        VEXTRACTUW(VRREG(d0), VRREG(v2), boff);
                        VINSERTW(VRREG(v0), VRREG(d0), boff);
                    }
                }
            }
            break;

        case 0x0D:
            INST_NAME("VBLENDPD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8 & 0x3;
            if (u8 == 0x0) {
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else if (u8 == 0x3) {
                if (v0 != v2) XXLOR(VSXREG(v0), VSXREG(v2), VSXREG(v2));
            } else if (u8 == 0x1) {
                // Replace LE dword 0 (= ISA dw1): keep v1 ISA dw0, take v2 ISA dw1
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(v2), 0b01);
            } else {
                // u8 == 0x2: Replace LE dword 1 (= ISA dw0): take v2 ISA dw0, keep v1 ISA dw1
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                XXPERMDI(VSXREG(v0), VSXREG(v2), VSXREG(v0), 0b01);
            }
            break;

        case 0x0E:
            INST_NAME("VPBLENDW Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8;
            if (v1 == v2) {
                // Same source: just copy
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else if (u8 == 0xFF) {
                if (v0 != v2) XXLOR(VSXREG(v0), VSXREG(v2), VSXREG(v2));
            } else if (u8 == 0x00) {
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                // Start with Vx
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                d0 = fpu_get_scratch(dyn);
                // Optimize aligned groups
                if ((u8 & 0x0F) == 0x0F) {
                    // Replace x86 hw 0-3 (ISA dw1)
                    XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(v2), 0b01);
                    u8 &= ~0x0F;
                }
                if ((u8 & 0xF0) == 0xF0) {
                    // Replace x86 hw 4-7 (ISA dw0)
                    XXPERMDI(VSXREG(v0), VSXREG(v2), VSXREG(v0), 0b01);
                    u8 &= ~0xF0;
                }
                // Word-aligned pairs
                if ((u8 & 0x03) == 0x03) {
                    VEXTRACTUW(VRREG(d0), VRREG(v2), 12);
                    VINSERTW(VRREG(v0), VRREG(d0), 12);
                    u8 &= ~0x03;
                }
                if ((u8 & 0x0C) == 0x0C) {
                    VEXTRACTUW(VRREG(d0), VRREG(v2), 8);
                    VINSERTW(VRREG(v0), VRREG(d0), 8);
                    u8 &= ~0x0C;
                }
                if ((u8 & 0x30) == 0x30) {
                    VEXTRACTUW(VRREG(d0), VRREG(v2), 4);
                    VINSERTW(VRREG(v0), VRREG(d0), 4);
                    u8 &= ~0x30;
                }
                if ((u8 & 0xC0) == 0xC0) {
                    VEXTRACTUW(VRREG(d0), VRREG(v2), 0);
                    VINSERTW(VRREG(v0), VRREG(d0), 0);
                    u8 &= ~0xC0;
                }
                // Individual halfwords
                for (int i = 0; i < 8; ++i) {
                    if (u8 & (1 << i)) {
                        int boff = (7 - i) * 2;
                        VEXTRACTUH(VRREG(d0), VRREG(v2), boff);
                        VINSERTH(VRREG(v0), VRREG(d0), boff);
                    }
                }
            }
            break;

        case 0x0F:
            INST_NAME("VPALIGNR Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8;
            if (u8 > 31) {
                // Result is all zeros
                XXLXOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
            } else if (u8 > 15) {
                // Shift Vx right by (u8-16) bytes
                if (u8 == 16) {
                    if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                } else {
                    d0 = fpu_get_scratch(dyn);
                    XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));
                    VSLDOI(VRREG(d0), VRREG(v1), VRREG(d0), 32 - u8);
                    XXLOR(VSXREG(v0), VSXREG(d0), VSXREG(d0));
                }
            } else if (u8 == 0) {
                if (v0 != v2) XXLOR(VSXREG(v0), VSXREG(v2), VSXREG(v2));
            } else {
                // General case: concat Vx(high):Ex(low), shift right by u8 bytes
                d0 = fpu_get_scratch(dyn);
                VSLDOI(VRREG(d0), VRREG(v1), VRREG(v2), 16 - u8);
                XXLOR(VSXREG(v0), VSXREG(d0), VSXREG(d0));
            }
            break;

        case 0x14:
            INST_NAME("VPEXTRB Ed, Gx, Ib");
            nextop = F8;
            GETGYx(v0, 0);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                u8 = (F8) & 15;
                d0 = fpu_get_scratch(dyn);
                // x86 byte index u8 = BE byte (15-u8)
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
            INST_NAME("VPEXTRW Ed, Gx, Ib");
            nextop = F8;
            GETGYx(v0, 0);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                u8 = (F8) & 7;
                d0 = fpu_get_scratch(dyn);
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
                INST_NAME("VPEXTRQ Ed, Gx, Ib");
            } else {
                INST_NAME("VPEXTRD Ed, Gx, Ib");
            }
            nextop = F8;
            GETGYx(v0, 0);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                u8 = F8;
                if (rex.w) {
                    if (u8 & 1)
                        MFVSRD(ed, VSXREG(v0));
                    else
                        MFVSRLD(ed, VSXREG(v0));
                } else {
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

        case 0x17:
            INST_NAME("VEXTRACTPS Ed, Gx, Ib");
            nextop = F8;
            GETGYx(v0, 0);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                u8 = F8 & 0x3;
                d0 = fpu_get_scratch(dyn);
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

        case 0x18:
        case 0x38:
            if (opcode == 0x18) {
                INST_NAME("VINSERTF128 Gx, Vx, Ex, Imm8");
            } else {
                INST_NAME("VINSERTI128 Gx, Vx, Ex, Imm8");
            }
            nextop = F8;
            {
                // VINSERT[FI]128: copy 256-bit Vx to Gx, then overwrite either
                // the low or high 128-bit lane with 128-bit Ex based on imm8 bit 0.
                // Ex is always 128-bit (xmm or m128).
                GETEYx(q2, 0, 1);  // 128-bit Ex
                GETVYy(q1, 0);     // 256-bit Vx (lower half in VSX reg)
                GETGYy_empty(q0);  // 256-bit Gx (empty)
                u8 = F8;
                if (u8 & 1) {
                    // Insert into high lane: copy Vx.lo to Gx.lo, store Ex as Gx.hi
                    if (q0 != q1) XXLOR(VSXREG(q0), VSXREG(q1), VSXREG(q1));
                    STXV(VSXREG(q2), offsetof(x64emu_t, ymm[gd]), xEmu);
                } else {
                    // Insert into low lane: Gx.lo = Ex, Gx.hi = Vx.hi
                    if (q0 != q2) XXLOR(VSXREG(q0), VSXREG(q2), VSXREG(q2));
                    // Copy Vx upper to Gx upper (ymm[gd] = ymm[vex.v])
                    d0 = fpu_get_scratch(dyn);
                    LXV(VSXREG(d0), offsetof(x64emu_t, ymm[vex.v]), xEmu);
                    STXV(VSXREG(d0), offsetof(x64emu_t, ymm[gd]), xEmu);
                }
            }
            break;
        case 0x19:
        case 0x39:
            if (opcode == 0x19) {
                INST_NAME("VEXTRACTF128 Ex, Gx, Imm8");
            } else {
                INST_NAME("VEXTRACTI128 Ex, Gx, Imm8");
            }
            nextop = F8;
            {
                // VEXTRACT[FI]128: extract low or high 128-bit lane of 256-bit Gx into 128-bit Ex
                GETGYy(q0, 0);  // 256-bit Gx (lower half in VSX reg)
                if (MODREG) {
                    GETEYx_empty(q1, 1);
                    u8 = F8;
                    if (u8 & 1) {
                        // Extract high lane: load from ymm[gd]
                        LXV(VSXREG(q1), offsetof(x64emu_t, ymm[gd]), xEmu);
                    } else {
                        // Extract low lane: copy Gx.lo
                        if (q1 != q0) XXLOR(VSXREG(q1), VSXREG(q0), VSXREG(q0));
                    }
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DQ_DISP, 1);
                    u8 = F8;
                    if (u8 & 1) {
                        // Extract high lane: load from ymm[gd] and store to memory
                        d0 = fpu_get_scratch(dyn);
                        LXV(VSXREG(d0), offsetof(x64emu_t, ymm[gd]), xEmu);
                        STXV(VSXREG(d0), fixedaddress, ed);
                    } else {
                        // Extract low lane: store Gx.lo to memory
                        STXV(VSXREG(q0), fixedaddress, ed);
                    }
                    SMWRITE2();
                }
            }
            break;
        case 0x1D:
            INST_NAME("VCVTPS2PH Ex, Gx, Ib");
            nextop = F8;
            DEFAULT;
            break;

        case 0x20:
            INST_NAME("VPINSRB Gx, Vx, ED, Ib");
            nextop = F8;
            GETVYx(v1, 0);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 1);
                LBZ(x1, wback, fixedaddress);
                ed = x1;
            }
            GETGYx_empty(v0);
            u8 = F8;
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            d0 = fpu_get_scratch(dyn);
            MTVSRDD(VSXREG(d0), ed, ed);
            // x86 byte index (u8 & 0xF) → BE byte (15 - index)
            VINSERTB(VRREG(v0), VRREG(d0), 15 - (u8 & 0xF));
            break;

        case 0x21:
            INST_NAME("VINSERTPS Gx, Vx, Ex, Ib");
            nextop = F8;
            d0 = fpu_get_scratch(dyn);
            if (MODREG) {
                GETGY_empty_VYEY_xy(v0, v1, v2, 1);
                u8 = F8;
                {
                    int src_idx = (u8 >> 6) & 3;
                    int dst_idx = (u8 >> 4) & 3;
                    // Copy Vx to Gx first
                    if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                    // Extract source word from Ex, insert into Gx
                    VEXTRACTUW(VRREG(d0), VRREG(v2), (3 - src_idx) * 4);
                    VINSERTW(VRREG(v0), VRREG(d0), (3 - dst_idx) * 4);
                }
            } else {
                GETVYx(v1, 0);
                GETGYx_empty(v0);
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x5, &fixedaddress, rex, NULL, NO_DISP, 1);
                u8 = F8;
                {
                    int dst_idx = (u8 >> 4) & 3;
                    // Memory: load single float, source index always 0
                    LWZ(x4, fixedaddress, ed);
                    SLDI(x4, x4, 32);
                    MTVSRD(VSXREG(d0), x4);
                    XSCVSPDPN(VSXREG(d0), VSXREG(d0));
                    XSCVDPSP(VSXREG(d0), VSXREG(d0));
                    VEXTRACTUW(VRREG(d0), VRREG(d0), 0);
                    VINSERTW(VRREG(v0), VRREG(d0), (3 - dst_idx) * 4);
                }
            }
            // Apply zmask: zero out selected destination words
            {
                uint8_t zmask = u8 & 0xF;
                if (zmask) {
                    d0 = fpu_get_scratch(dyn);
                    XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));
                    for (int i = 0; i < 4; ++i) {
                        if (zmask & (1 << i)) {
                            VEXTRACTUW(VRREG(d0), VRREG(d0), 0);
                            VINSERTW(VRREG(v0), VRREG(d0), (3 - i) * 4);
                        }
                    }
                }
            }
            break;

        case 0x22:
            if (rex.w) {
                INST_NAME("VPINSRQ Gx, Vx, ED, Ib");
            } else {
                INST_NAME("VPINSRD Gx, Vx, ED, Ib");
            }
            nextop = F8;
            GETVYx(v1, 0);
            GETED(1);
            GETGYx_empty(v0);
            u8 = F8;
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            d0 = fpu_get_scratch(dyn);
            if (rex.w) {
                // VPINSRQ: insert 64-bit value
                MTVSRDD(VSXREG(d0), ed, ed);
                VINSERTD(VRREG(v0), VRREG(d0), (1 - (u8 & 1)) * 8);
            } else {
                // VPINSRD: insert 32-bit value
                MTVSRDD(VSXREG(d0), ed, ed);
                // On PPC64LE, MTVSRDD puts ed in both dw0 and dw1. The low 32 bits
                // of the 64-bit value are at byte offset 4 (dw0) and 12 (dw1).
                // VEXTRACTUW byte 4 extracts the correct 32-bit value.
                VEXTRACTUW(VRREG(d0), VRREG(d0), 4);
                VINSERTW(VRREG(v0), VRREG(d0), (3 - (u8 & 3)) * 4);
            }
            break;

        // case 0x38 handled above with case 0x18
        // case 0x39 handled above with case 0x19

        case 0x40:
            INST_NAME("VDPPS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8;
            {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                d2 = fpu_get_scratch(dyn);
                // Multiply all 4 single lanes
                XVMULSP(VSXREG(d0), VSXREG(v1), VSXREG(v2));
                // Zero out lanes not selected by input mask (bits 7:4)
                XXLXOR(VSXREG(d2), VSXREG(d2), VSXREG(d2));
                for (int i = 0; i < 4; ++i) {
                    if (!(u8 & (1 << (4 + i)))) {
                        // Zero x86 word i = BE byte offset (3-i)*4
                        VINSERTW(VRREG(d0), VRREG(d2), (3 - i) * 4);
                    }
                }
                // Horizontal sum: swap adjacent pairs, add
                VSLDOI(VRREG(d1), VRREG(d0), VRREG(d0), 4);
                XVADDSP(VSXREG(d0), VSXREG(d0), VSXREG(d1));
                // Sum pairs: VSLDOI by 8 bytes
                VSLDOI(VRREG(d1), VRREG(d0), VRREG(d0), 8);
                XVADDSP(VSXREG(d0), VSXREG(d0), VSXREG(d1));
                // Broadcast the sum to all 4 words
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
            INST_NAME("VDPPD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8;
            {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                // Multiply both double lanes
                XVMULDP(VSXREG(d0), VSXREG(v1), VSXREG(v2));
                // Zero out lanes not selected by input mask (bits 5:4)
                XXLXOR(VSXREG(d1), VSXREG(d1), VSXREG(d1));
                for (int i = 0; i < 2; ++i) {
                    if (!(u8 & (1 << (4 + i)))) {
                        if (i == 0) {
                            // Zero LE dword 0 = ISA dw1 (bits 64:127)
                            XXPERMDI(VSXREG(d0), VSXREG(d0), VSXREG(d1), 0b01);
                        } else {
                            // Zero LE dword 1 = ISA dw0 (bits 0:63)
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
                    XXLOR(VSXREG(v0), VSXREG(d0), VSXREG(d0));
                } else if ((u8 & 0x3) == 0x0) {
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

        case 0x42:
            INST_NAME("VMPSADBW Gx, Vx, Ex, Ib");
            nextop = F8;
            DEFAULT;
            break;
        case 0x44:
            INST_NAME("VPCLMULQDQ Gx, Vx, Ex, Ib");
            nextop = F8;
            GETG;
            avx_forget_reg(dyn, ninst, gd);
            avx_forget_reg(dyn, ninst, vex.v);
            MOV32w(x1, gd);    // gx
            MOV32w(x2, vex.v); // vx
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                avx_forget_reg(dyn, ninst, ed);
                MOV32w(x3, ed); // ex
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x5, &fixedaddress, rex, NULL, NO_DISP, 1);
                if (ed != x3) MV(x3, ed);
            }
            u8 = F8;
            MOV32w(x4, u8);
            CALL4_(vex.l ? const_native_pclmul_y : const_native_pclmul_x, -1, x3, x1, x2, x3, x4);
            if (!vex.l) {
                LI(x4, 0);
                STD(x4, offsetof(x64emu_t, ymm[gd]), xEmu);
                STD(x4, offsetof(x64emu_t, ymm[gd]) + 8, xEmu);
            }
            break;
        case 0x46:
            INST_NAME("VPERM2I128 Gx, Vx, Ex, Imm8");
            nextop = F8;
            if (!vex.l) { DEFAULT; break; }
            {
                // VPERM2I128: identical to VPERM2F128, integer variant
                int ex_reg = -1;
                GETGY_empty_VYEY_xy(v0, v1, v2, 1);
                if (MODREG) ex_reg = (nextop & 7) + (rex.b << 3);
                u8 = F8;
                uint8_t zero_lo = (u8 >> 3) & 1;
                uint8_t zero_hi = (u8 >> 7) & 1;
                uint8_t sel_lo = u8 & 3;
                uint8_t sel_hi = (u8 >> 4) & 3;

                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);

                if (zero_lo) {
                    XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));
                } else {
                    switch (sel_lo) {
                        case 0: XXLOR(VSXREG(d0), VSXREG(v1), VSXREG(v1)); break;
                        case 1: LXV(VSXREG(d0), offsetof(x64emu_t, ymm[vex.v]), xEmu); break;
                        case 2: XXLOR(VSXREG(d0), VSXREG(v2), VSXREG(v2)); break;
                        case 3:
                            if (MODREG)
                                LXV(VSXREG(d0), offsetof(x64emu_t, ymm[ex_reg]), xEmu);
                            else
                                LXV(VSXREG(d0), fixedaddress + 16, ed);
                            break;
                    }
                }

                if (zero_hi) {
                    XXLXOR(VSXREG(d1), VSXREG(d1), VSXREG(d1));
                } else {
                    switch (sel_hi) {
                        case 0: XXLOR(VSXREG(d1), VSXREG(v1), VSXREG(v1)); break;
                        case 1: LXV(VSXREG(d1), offsetof(x64emu_t, ymm[vex.v]), xEmu); break;
                        case 2: XXLOR(VSXREG(d1), VSXREG(v2), VSXREG(v2)); break;
                        case 3:
                            if (MODREG)
                                LXV(VSXREG(d1), offsetof(x64emu_t, ymm[ex_reg]), xEmu);
                            else
                                LXV(VSXREG(d1), fixedaddress + 16, ed);
                            break;
                    }
                }

                XXLOR(VSXREG(v0), VSXREG(d0), VSXREG(d0));
                STXV(VSXREG(d1), offsetof(x64emu_t, ymm[gd]), xEmu);
            }
            break;

        case 0x4A:
            INST_NAME("VBLENDVPS Gx, Vx, Ex, XMMImm8");
            nextop = F8;
            u8 = geted_ib(dyn, addr, ninst, nextop) >> 4;
            d0 = avx_get_reg(dyn, ninst, x5, u8, 0, VMX_AVX_WIDTH_128);
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            F8;
            // d0 holds the selector register; sign bit of each dword selects
            q0 = fpu_get_scratch(dyn);
            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
            VCMPGTSW(VRREG(q0), VRREG(q0), VRREG(d0));     // q0 = -1 where d0 dword < 0
            XXSEL(VSXREG(v0), VSXREG(v1), VSXREG(v2), VSXREG(q0));
            break;

        case 0x4B:
            INST_NAME("VBLENDVPD Gx, Vx, Ex, XMMImm8");
            nextop = F8;
            u8 = geted_ib(dyn, addr, ninst, nextop) >> 4;
            d0 = avx_get_reg(dyn, ninst, x5, u8, 0, VMX_AVX_WIDTH_128);
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            F8;
            // d0 holds the selector register; sign bit of each qword selects
            q0 = fpu_get_scratch(dyn);
            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
            VCMPGTSD(VRREG(q0), VRREG(q0), VRREG(d0));     // q0 = -1 where d0 qword < 0
            XXSEL(VSXREG(v0), VSXREG(v1), VSXREG(v2), VSXREG(q0));
            break;

        case 0x4C:
            INST_NAME("VPBLENDVB Gx, Vx, Ex, XMMImm8");
            nextop = F8;
            u8 = geted_ib(dyn, addr, ninst, nextop) >> 4;
            d0 = avx_get_reg(dyn, ninst, x5, u8, 0, VMX_AVX_WIDTH_128);
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            F8;
            // d0 holds the selector register; sign bit of each byte selects
            q0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            VSPLTISB(VRREG(d1), 7);
            VSRAB(VRREG(q0), VRREG(d0), VRREG(d1));   // q0 = 0xFF where byte sign set, 0 otherwise
            VSEL(VRREG(v0), VRREG(v1), VRREG(v2), VRREG(q0));
            break;

        case 0x60:
            INST_NAME("VPCMPESTRM Gx, Ex, Ib");
            nextop = F8;
            DEFAULT;
            break;
        case 0x61:
            INST_NAME("VPCMPESTRI Gx, Ex, Ib");
            nextop = F8;
            DEFAULT;
            break;
        case 0x62:
            INST_NAME("VPCMPISTRM Gx, Ex, Ib");
            nextop = F8;
            DEFAULT;
            break;
        case 0x63:
            INST_NAME("VPCMPISTRI Gx, Ex, Ib");
            nextop = F8;
            DEFAULT;
            break;

        case 0xDF:
            INST_NAME("VAESKEYGENASSIST Gx, Ex, Ib");
            nextop = F8;
            GETG;
            avx_forget_reg(dyn, ninst, gd);
            MOV32w(x1, gd); // gx
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                avx_forget_reg(dyn, ninst, ed);
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
            if (!vex.l) {
                LI(x4, 0);
                STD(x4, offsetof(x64emu_t, ymm[gd]), xEmu);
                STD(x4, offsetof(x64emu_t, ymm[gd]) + 8, xEmu);
            }
            break;

        default:
            DEFAULT;
    }

    return addr;
}
