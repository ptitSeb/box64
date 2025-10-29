#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "la64_emitter.h"
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
#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_0F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wb1, wback, wb2, gback;
    uint8_t eb1, eb2;
    uint8_t gb1, gb2;
    uint8_t tmp1, tmp2, tmp3;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int s0, s1;
    uint64_t tmp64u;
    int64_t j64;
    int64_t fixedaddress, gdoffset;
    int unscaled;
    MAYUSE(wb2);
    MAYUSE(gback);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(s0);
    MAYUSE(j64);
    MAYUSE(cacheupd);

    switch (opcode) {
        case 0x01:
            // TODO:, /0 is SGDT. While 0F 01 D0 is XGETBV, etc...
            nextop = F8;
            if (MODREG) {
                switch (nextop) {
                    case 0xD0:
                        INST_NAME("XGETBV");
                        BEQZ_MARK(xRCX);
                        EMIT(0); // Is there any assigned illegal instruction?
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
            CALL_S(const_x64syscall, -1, 0);
            LOAD_XEMU_CALL();
            TABLE64(x3, addr); // expected return address
            BNE_MARK(xRIP, x3);
            LD_W(x1, xEmu, offsetof(x64emu_t, quit));
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
                case 1:
                    INST_NAME("PREFETCHW");
                    FAKEED;
                    break;
                default: //???
                    DEFAULT;
            }
            break;
        case 0x10:
            INST_NAME("MOVUPS Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg(dyn, ninst, x1, ed, 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VOR_V(v0, v1, v1);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                VLD(v0, ed, fixedaddress);
            }
            break;
        case 0x11:
            INST_NAME("MOVUPS Ex,Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                VOR_V(v1, v0, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                VST(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x12:
            nextop = F8;
            if (MODREG) {
                INST_NAME("MOVHLPS Gx, Ex");
                GETGX(v0, 1);
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                VEXTRINS_D(v0, v1, 1);
            } else {
                INST_NAME("MOVLPS Gx, Ex");
                GETGX(v0, 1);
                SMREAD();
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                FLD_D(v1, ed, fixedaddress);
                VEXTRINS_D(v0, v1, 0);
            }
            break;
        case 0x13:
            nextop = F8;
            INST_NAME("MOVLPS Ex,Gx");
            GETGX(v0, 0);
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 1);
                VEXTRINS_D(v1, v0, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                FST_D(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x14:
            INST_NAME("UNPCKLPS Gx, Ex");
            nextop = F8;
            SMREAD();
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VILVL_W(v0, q0, v0);
            break;
        case 0x15:
            INST_NAME("UNPCKHPS Gx, Ex");
            nextop = F8;
            SMREAD();
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VILVH_W(v0, q0, v0);
            break;
        case 0x16:
            nextop = F8;
            if (MODREG) {
                INST_NAME("MOVLHPS Gx, Ex");
                GETGX(v0, 1);
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
            } else {
                INST_NAME("MOVHPS Gx, Ex");
                SMREAD();
                GETGX(v0, 1);
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                FLD_D(v1, ed, fixedaddress);
            }
            VILVL_D(v0, v1, v0); // v0[127:64] = v1[63:0]
            break;
        case 0x17:
            nextop = F8;
            INST_NAME("MOVHPS Ex,Gx");
            GETGX(v0, 0);
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 1);
                VEXTRINS_D(v1, v0, 0x01);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                VSTELM_D(v0, ed, 0, 1);
                SMWRITE2();
            }
            break;
        case 0x18:
            nextop = F8;
            if (MODREG) {
                INST_NAME("NOP (multibyte)");
            } else
                switch ((nextop >> 3) & 7) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                        INST_NAME("PREFETCHh Ed");
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
        case 0x28:
            INST_NAME("MOVAPS Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg(dyn, ninst, x1, ed, 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VOR_V(v0, v1, v1);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                VLD(v0, ed, fixedaddress);
            }
            break;
        case 0x29:
            INST_NAME("MOVAPS Ex,Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                VOR_V(v1, v0, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                VST(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x2A:
            INST_NAME("CVTPI2PS Gx,Em");
            nextop = F8;
            GETGX(v0, 1);
            GETEM(v1, 0);
            q0 = fpu_get_scratch(dyn);
            u8 = sse_setround(dyn, ninst, x1, x2);
            VFFINT_S_W(q0, v1);
            x87_restoreround(dyn, ninst, u8);
            VEXTRINS_D(v0, q0, VEXTRINS_IMM_4_0(0, 0));
            break;
        case 0x2B:
            INST_NAME("MOVNTPS Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                VOR_V(v1, v0, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                VST(v0, ed, fixedaddress);
            }
            break;
        case 0x2C:
            INST_NAME("CVTTPS2PI Gm, Ex");
            nextop = F8;
            GETGM(v0);
            GETEXSD(v1, 0, 0);
            if (BOX64ENV(dynarec_fastround)) {
                VFTINTRZ_W_S(v0, v1);
            } else {
                MOVGR2FCSR(FCSR2, xZR); // reset all bits
                VFTINTRZ_W_S(v0, v1);
                MOVFCSR2GR(x5, FCSR2); // get back FPSR to check
                MOV32w(x3, (1 << FR_V) | (1 << FR_O));
                AND(x5, x5, x3);
                BEQZ_MARK3(x5); // no fp exception, work done.

                // check +/-Nan, +overlow ,replace with 0x80000000
                q0 = fpu_get_scratch(dyn);
                q1 = fpu_get_scratch(dyn); // mask
                d0 = fpu_get_scratch(dyn);
                VLDI(q0, 0b1001110000000); // broadcast 0x80000000 to all
                VLDI(d0, (0b10011 << 8) | 0x4f);
                VFCMP_S(q1, d0, v1, cULE); // get Nan,+overflow mark
                VBITSEL_V(v0, v0, q0, q1);

                MARK3;
            }
            break;
        case 0x2D:
            INST_NAME("CVTPS2PI Gm, Ex");
            nextop = F8;
            GETGM(v0);
            GETEXSD(v1, 0, 0);
            u8 = sse_setround(dyn, ninst, x4, x6);
            if (BOX64ENV(dynarec_fastround)) {
                VFTINTRZ_W_S(v0, v1);
            } else {
                MOVGR2FCSR(FCSR2, xZR); // reset all bits
                VFTINT_W_S(v0, v1);
                MOVFCSR2GR(x5, FCSR2); // get back FPSR to check
                MOV32w(x3, (1 << FR_V) | (1 << FR_O));
                AND(x5, x5, x3);
                BEQZ_MARK3(x5); // no fp exception, work done, fast path.

                // check +/-Nan, +overlow ,replace with 0x80000000
                /* LoongArch follow IEEE754-2008,
                   if val < -2147483648.0f got -2147483648 match sse
                   if var >  2147483648.0f got  2147483647 need mask
                   but lucky _Float32 is not accurate:
                   -2147483648.0f is 0xcf000000 (_Float32)
                   -2147483520.0f is 0xceffffff (_Float32)
                    2147483648.0f is 0x4f000000 (_Float32)
                    2147483520.0f is 0x4effffff (_Float32)
                   combine (unorder || gt 0x4f000000)
                   use cULE  for (unodered || 0x4f000000 <= v1[x])
                */
                q0 = fpu_get_scratch(dyn);
                q1 = fpu_get_scratch(dyn); // mask
                d0 = fpu_get_scratch(dyn);
                VLDI(q0, 0b1001110000000); // broadcast 0x80000000 to all
                VLDI(d0, (0b10011 << 8) | 0x4f);
                VFCMP_S(q1, d0, v1, cULE); // get Nan,+overflow mark
                VBITSEL_V(v0, v0, q0, q1);

                MARK3;
            }
            x87_restoreround(dyn, ninst, u8);
            break;
        case 0x2E:
            // no special check...
        case 0x2F:
            if (opcode == 0x2F) {
                INST_NAME("COMISS Gx, Ex");
            } else {
                INST_NAME("UCOMISS Gx, Ex");
            }
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGX(d0, 0);
            GETEXSS(v0, 0, 0);
            CLEAR_FLAGS(x2);
            // if isnan(d0) || isnan(v0)
            IFX (X_ZF | X_PF | X_CF) {
                FCMP_S(fcc0, d0, v0, cUN);
                BCEQZ_MARK(fcc0);
                ORI(xFlags, xFlags, (1 << F_ZF) | (1 << F_PF) | (1 << F_CF));
                B_MARK3_nocond;
            }
            MARK;
            // else if isless(d0, v0)
            IFX (X_CF) {
                FCMP_S(fcc1, d0, v0, cLT);
                BCEQZ_MARK2(fcc1);
                ORI(xFlags, xFlags, 1 << F_CF);
                B_MARK3_nocond;
            }
            MARK2;
            // else if d0 == v0
            IFX (X_ZF) {
                FCMP_S(fcc2, d0, v0, cEQ);
                BCEQZ_MARK3(fcc2);
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            MARK3;
            IFX (X_ALL) {
                SPILL_EFLAGS();
            }
            break;
        case 0x31:
            INST_NAME("RDTSC");
            NOTEST(x1);
            if (box64_rdtsc) {
                CALL(const_readtsc, x3, 0, 0); // will return the u64 in x3
            } else {
                RDTIME_D(x3, xZR);
            }
            if (box64_rdtsc_shift) {
                SRLI_D(x3, x3, box64_rdtsc_shift);
            }
            SRLI_D(xRDX, x3, 32);
            ZEROUP2(xRDX, x3);
            break;
        case 0x38:
            // SSE3
            nextop = F8;
            switch (nextop) {
                case 0x00:
                    INST_NAME("PSHUFB Gm, Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    v0 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    d0 = fpu_get_scratch(dyn);
                    VLDI(v0, 0b0000010000111); // broadcast 0b10000111 as byte
                    VAND_V(v0, v0, q1);
                    VMINI_BU(v0, v0, 0x1f);
                    VXOR_V(v1, v1, v1);
                    VSHUF_B(q0, v1, q0, v0);
                    break;
                case 0x01:
                    INST_NAME("PHADDW Gm, Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    v0 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    VPICKEV_H(v0, q1, q0);
                    VPICKOD_H(v1, q1, q0);
                    VADD_H(q0, v1, v0);
                    VSHUF4I_W(q0, q0, 0b11011000);
                    break;
                case 0x02:
                    INST_NAME("PHADDD Gm, Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    v0 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    VPICKEV_W(v0, q1, q0);
                    VPICKOD_W(v1, q1, q0);
                    VADD_W(q0, v0, v1);
                    VSHUF4I_W(q0, q0, 0b11011000);
                    break;
                case 0x03:
                    INST_NAME("PHADDSW Gm, Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    v0 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    VPICKEV_H(v0, q1, q0);
                    VPICKOD_H(v1, q1, q0);
                    VSADD_H(q0, v0, v1);
                    VSHUF4I_W(q0, q0, 0b11011000);
                    break;
                case 0x04:
                    INST_NAME("PMADDUBSW Gm, Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    v0 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    VMULWEV_H_BU_B(v0, q0, q1);
                    VMULWOD_H_BU_B(v1, q0, q1);
                    VSADD_H(q0, v0, v1);
                    break;
                case 0x05:
                    INST_NAME("PHSUBW Gm, Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    v0 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    VPICKEV_H(v0, q1, q0);
                    VPICKOD_H(v1, q1, q0);
                    VSUB_H(q0, v0, v1);
                    VSHUF4I_W(q0, q0, 0b11011000);
                    break;
                case 0x06:
                    INST_NAME("PHSUBD Gm, Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    v0 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    VPICKEV_W(v0, q1, q0);
                    VPICKOD_W(v1, q1, q0);
                    VSUB_W(q0, v0, v1);
                    VSHUF4I_W(q0, q0, 0b11011000);
                    break;
                case 0x07:
                    INST_NAME("PHSUBSW Gm, Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    v0 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    VPICKEV_H(v0, q1, q0);
                    VPICKOD_H(v1, q1, q0);
                    VSSUB_H(q0, v0, v1);
                    VSHUF4I_W(q0, q0, 0b11011000);
                    break;
                case 0x08:
                    INST_NAME("PSIGNB Gm, Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    VSIGNCOV_B(q0, q1, q0);
                    break;
                case 0x09:
                    INST_NAME("PSIGNW Gm, Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    VSIGNCOV_H(q0, q1, q0);
                    break;
                case 0x0A:
                    INST_NAME("PSIGND Gm, Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    VSIGNCOV_W(q0, q1, q0);
                    break;
                case 0x0B:
                    INST_NAME("PMULHRSW Gm, Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    v0 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    VEXT2XV_W_H(v0, q0);
                    VEXT2XV_W_H(v1, q1);
                    XVMUL_W(v0, v0, v1);
                    VSRLI_W(v0, v0, 14);
                    VADDI_WU(v0, v0, 1);
                    VSRLNI_H_W(q0, v0, 1);
                    break;
                case 0x1C:
                    INST_NAME("PABSB Gm, Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    v0 = fpu_get_scratch(dyn);
                    VXOR_V(v0, v0, v0);
                    VABSD_B(q0, q1, v0);
                    break;
                case 0x1D:
                    INST_NAME("PABSW Gm, Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    v0 = fpu_get_scratch(dyn);
                    VXOR_V(v0, v0, v0);
                    VABSD_H(q0, q1, v0);
                    break;
                case 0x1E:
                    INST_NAME("PABSD Gm, Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    v0 = fpu_get_scratch(dyn);
                    VXOR_V(v0, v0, v0);
                    VABSD_W(q0, q1, v0);
                    break;
                case 0xC8 ... 0xCD:
                    u8 = nextop;
                    switch (u8) {
                        case 0xC8:
                            INST_NAME("SHA1NEXTE Gx, Ex");
                            break;
                        case 0xC9:
                            INST_NAME("SHA1MSG1 Gx, Ex");
                            break;
                        case 0xCA:
                            INST_NAME("SHA1MSG2 Gx, Ex");
                            break;
                        case 0xCB:
                            INST_NAME("SHA256RNDS2 Gx, Ex");
                            break;
                        case 0xCC:
                            INST_NAME("SHA256MSG1 Gx, Ex");
                            break;
                        case 0xCD:
                            INST_NAME("SHA256MSG2 Gx, Ex");
                            break;
                    }
                    nextop = F8;
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        sse_reflect_reg(dyn, ninst, ed);
                        ADDI_D(x2, xEmu, offsetof(x64emu_t, xmm[ed]));
                        ed = x2;
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 0, 0);
                    }
                    GETG;
                    sse_forget_reg(dyn, ninst, gd);
                    ADDI_D(x1, xEmu, offsetof(x64emu_t, xmm[gd]));
                    sse_reflect_reg(dyn, ninst, 0);
                    switch (u8) {
                        case 0xC8:
                            CALL(const_sha1nexte, -1, x1, ed);
                            break;
                        case 0xC9:
                            CALL(const_sha1msg1, -1, x1, ed);
                            break;
                        case 0xCA:
                            CALL(const_sha1msg2, -1, x1, ed);
                            break;
                        case 0xCB:
                            CALL(const_sha256rnds2, -1, x1, ed);
                            break;
                        case 0xCC:
                            CALL(const_sha256msg1, -1, x1, ed);
                            break;
                        case 0xCD:
                            CALL(const_sha256msg2, -1, x1, ed);
                            break;
                    }
                    break;
                case 0xF0:
                    INST_NAME("MOVBE Gd, Ed");
                    nextop = F8;
                    GETGD;
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    LDxw(gd, ed, fixedaddress);
                    if (rex.w) {
                        REVB_D(gd, gd);
                    } else {
                        REVB_2W(gd, gd);
                        ZEROUP(gd);
                    }
                    break;
                case 0xF1:
                    INST_NAME("MOVBE Ed, Gd");
                    nextop = F8;
                    GETGD;
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    if (rex.w)
                        REVB_D(x1, gd);
                    else
                        REVB_2W(x1, gd);
                    SDxw(x1, wback, fixedaddress);
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x3A: // more SSE3 opcodes
            opcode = F8;
            switch (opcode) {
                case 0x0F:
                    INST_NAME("PALIGNR Gm, Em, Ib");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 1);
                    u8 = F8;
                    if (u8 > 15) {
                        VXOR_V(q0, q0, q0);
                    } else if (!u8) {
                        VEXTRINS_D(q0, q1, VEXTRINS_IMM_4_0(0, 0));
                    } else {
                        d0 = fpu_get_scratch(dyn);
                        VOR_V(d0, q0, q0);
                        VSHUF4I_D(d0, q1, 0b00010010);
                        VBSRL_V(d0, d0, u8);
                        VEXTRINS_D(q0, d0, 0);
                    }
                    break;
                case 0xCC:
                    INST_NAME("SHA1RNDS4 Gx, Ex, Ib");
                    nextop = F8;
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        sse_reflect_reg(dyn, ninst, ed);
                        ADDI_D(x2, xEmu, offsetof(x64emu_t, xmm[ed]));
                        wback = x2;
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 0, 1);
                    }
                    u8 = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, gd);
                    ADDI_D(x1, xEmu, offsetof(x64emu_t, xmm[gd]));
                    MOV32w(x3, u8);
                    CALL4(const_sha1rnds4, -1, x1, wback, x3, 0);
                    break;
                default:
                    DEFAULT;
            }
            break;

#define GO(GETFLAGS, NO, YES, NATNO, NATYES, F, I)                                               \
    READFLAGS_FUSION(F, x1, x2, x3, x4, x5);                                                     \
    if (!dyn->insts[ninst].nat_flags_fusion) {                                                   \
        if (cpuext.lbt) {                                                                        \
            X64_SETJ(tmp1, I);                                                                   \
        } else {                                                                                 \
            GETFLAGS;                                                                            \
        }                                                                                        \
    }                                                                                            \
    nextop = F8;                                                                                 \
    GETGD;                                                                                       \
    if (MODREG) {                                                                                \
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                                \
        if (dyn->insts[ninst].nat_flags_fusion) {                                                \
            NATIVEJUMP(NATNO, 8);                                                                \
        } else {                                                                                 \
            if (cpuext.lbt)                                                                      \
                BEQZ(tmp1, 8);                                                                   \
            else                                                                                 \
                B##NO(tmp1, 8);                                                                  \
        }                                                                                        \
        MV(gd, ed);                                                                              \
        if (!rex.w) ZEROUP(gd);                                                                  \
    } else {                                                                                     \
        addr = geted(dyn, addr, ninst, nextop, &ed, tmp2, tmp3, &fixedaddress, rex, NULL, 1, 0); \
        if (dyn->insts[ninst].nat_flags_fusion) {                                                \
            NATIVEJUMP(NATNO, 8);                                                                \
        } else {                                                                                 \
            if (cpuext.lbt)                                                                      \
                BEQZ(tmp1, 8);                                                                   \
            else                                                                                 \
                B##NO(tmp1, 8);                                                                  \
        }                                                                                        \
        LDxw(gd, ed, fixedaddress);                                                              \
    }

            GOCOND(0x40, "CMOV", "Gd, Ed");

#undef GO

        case 0x50:
            INST_NAME("MOVMSPKPS Gd, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGD;
            q1 = fpu_get_scratch(dyn);
            VMSKLTZ_W(q1, q0);
            MOVFR2GR_S(gd, q1);
            BSTRPICK_D(gd, gd, 31, 0);
            break;
        case 0x51:
            INST_NAME("SQRTPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX_empty(v0);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFCMP_S(d0, q0, q0, cEQ);
                VFSQRT_S(v0, q0);
                VFCMP_S(d1, v0, v0, cEQ);
                VANDN_V(d1, d1, d0);
                VSLLI_W(d1, d1, 31);
                VOR_V(v0, v0, d1);
            } else
                VFSQRT_S(v0, q0);
            break;
        case 0x52:
            INST_NAME("RSQRTPS Gx, Ex");
            nextop = F8;
            SKIPTEST(x1);
            GETEX(q0, 0, 0);
            GETGX_empty(q1);
            if (cpuext.frecipe) {
                VFRSQRTE_S(q1, q0);
            } else {
                VFRSQRT_S(q1, q0);
            }
            break;
        case 0x53:
            INST_NAME("RCPPS Gx, Ex");
            nextop = F8;
            SKIPTEST(x1);
            GETEX(q0, 0, 0);
            GETGX_empty(q1);
            if (cpuext.frecipe) {
                VFRECIPE_S(q1, q0);
            } else {
                VFRECIP_S(q1, q0);
            }
            break;
        case 0x54:
            INST_NAME("ANDPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VAND_V(v0, v0, q0);
            break;
        case 0x55:
            INST_NAME("ANDNPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VANDN_V(v0, v0, q0);
            break;
        case 0x56:
            INST_NAME("ORPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VOR_V(v0, v0, q0);
            break;
        case 0x57:
            INST_NAME("XORPS Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG && ((nextop & 7) + (rex.b << 3) == gd)) {
                // special case for XORPS Gx, Gx
                q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VXOR_V(q0, q0, q0);
            } else {
                q0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                GETEX(q1, 0, 0);
                VXOR_V(q0, q0, q1);
            }
            break;
        case 0x58:
            INST_NAME("ADDPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VFADD_S(v0, v0, q0);
            break;
        case 0x59:
            INST_NAME("MULPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(q1, 1);
            if (!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn);
                v1 = fpu_get_scratch(dyn);
                VFCMP_S(v0, q0, q1, cUN);
            }
            VFMUL_S(q1, q1, q0);
            if (!BOX64ENV(dynarec_fastnan)) {
                VFCMP_S(v1, q1, q1, cUN);
                VANDN_V(v0, v0, v1);
                VLDI(v1, 0b011111111100); // broadcast 0xFFFFFFFFFFFFFFFC
                VSLLI_W(v1, v1, 20);
                VAND_V(v1, v0, v1);
                VANDN_V(v0, v0, q1);
                VOR_V(q1, v0, v1);
            }
            break;
        case 0x5A:
            INST_NAME("CVTPS2PD Gx, Ex");
            nextop = F8;
            GETEXSD(q0, 0, 0);
            GETGX(q1, 1);
            VFCVTL_D_S(q1, q0);
            break;
        case 0x5B:
            INST_NAME("CVTDQ2PS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX_empty(q1);
            VFFINT_S_W(q1, q0);
            break;
        case 0x5C:
            INST_NAME("SUBPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(q1, 1);
            if (!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn);
                v1 = fpu_get_scratch(dyn);
                VFCMP_S(v0, q0, q1, cUN);
            }
            VFSUB_S(q1, q1, q0);
            if (!BOX64ENV(dynarec_fastnan)) {
                VFCMP_S(v1, q1, q1, cUN);
                VANDN_V(v0, v0, v1);
                VLDI(v1, 0b011111111100); // broadcast 0xFFFFFFFFFFFFFFFC
                VSLLI_W(v1, v1, 20);
                VAND_V(v1, v0, v1);
                VANDN_V(v0, v0, q1);
                VOR_V(q1, v0, v1);
            }
            break;
        case 0x5D:
            INST_NAME("MINPS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            if (!BOX64ENV(dynarec_fastnan) && v0 != v1) {
                q0 = fpu_get_scratch(dyn);
                VFCMP_S(q0, v1, v0, cULE);
                VBITSEL_V(v0, v0, v1, q0);
            } else {
                VFMIN_S(v0, v0, v1);
            }
            break;
        case 0x5E:
            INST_NAME("DIVPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(q1, 1);
            if (!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn);
                v1 = fpu_get_scratch(dyn);
                VFCMP_S(v0, q0, q1, cUN);
            }
            VFDIV_S(q1, q1, q0);
            if (!BOX64ENV(dynarec_fastnan)) {
                VFCMP_S(v1, q1, q1, cUN);
                VANDN_V(v0, v0, v1);
                VLDI(v1, 0b011111111100); // broadcast 0xFFFFFFFFFFFFFFFC
                VSLLI_W(v1, v1, 20);
                VAND_V(v1, v0, v1);
                VANDN_V(v0, v0, q1);
                VOR_V(q1, v0, v1);
            }
            break;
        case 0x5F:
            INST_NAME("MAXPS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            if (!BOX64ENV(dynarec_fastnan) && v0 != v1) {
                q0 = fpu_get_scratch(dyn);
                q1 = fpu_get_scratch(dyn);
                VFCMP_S(q0, v1, v0, cLT);  // ~cLT = un ge eq, if either v0/v1=nan ,choose v1. if eq either is ok,but when +0.0 == -0.0 x86 sse choose v1
                VBITSEL_V(v0, v1, v0, q0); // swap v0 v1 => v1 v0 for ~cLT
            } else {
                VFMAX_S(v0, v0, v1);
            }
            break;
        case 0x60:
            INST_NAME("PUNPCKLBW Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VILVL_B(v0, v1, v0);
            break;
        case 0x61:
            INST_NAME("PUNPCKLWD Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VILVL_H(v0, v1, v0);
            break;
        case 0x62:
            INST_NAME("PUNPCKLDQ Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VEXTRINS_W(v0, v1, VEXTRINS_IMM_4_0(1, 0));
            break;
        case 0x63:
            INST_NAME("PACKSSWB Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VPACKEV_D(v0, v1, v0);
            VSSRANI_B_H(v0, v0, 0);
            break;
        case 0x64:
            INST_NAME("PCMPGTB Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VSLT_B(v0, v1, v0);
            break;
        case 0x65:
            INST_NAME("PCMPGTW Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VSLT_H(v0, v1, v0);
            break;
        case 0x66:
            INST_NAME("PCMPGTD Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VSLT_W(v0, v1, v0);
            break;
        case 0x67:
            INST_NAME("PACKUSWB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            q0 = fpu_get_scratch(dyn);
            VPACKEV_D(v0, v1, v0);
            VSSRANI_BU_H(v0, v0, 0);
            break;
        case 0x68:
            INST_NAME("PUNPCKHBW Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VILVL_B(v0, v1, v0);
            VEXTRINS_D(v0, v0, VEXTRINS_IMM_4_0(0, 1));
            break;
        case 0x69:
            INST_NAME("PUNPCKHWD Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VILVL_H(v0, v1, v0);
            VEXTRINS_D(v0, v0, VEXTRINS_IMM_4_0(0, 1));
            break;
        case 0x6A:
            INST_NAME("PUNPCKHDQ Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VILVL_W(v0, v1, v0);
            VEXTRINS_D(v0, v0, VEXTRINS_IMM_4_0(0, 1));
            break;
        case 0x6B:
            INST_NAME("PACKSSDW Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            d0 = fpu_get_scratch(dyn);
            VPACKEV_D(v0, v1, v0);
            VSSRANI_H_W(v0, v0, 0);
            break;
        case 0x6E:
            INST_NAME("MOVD Gm, Ed");
            nextop = F8;
            GETG;
            v0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
            if (MODREG) {
                ed = TO_NAT(nextop & 7);
                if (rex.w) {
                    MOVGR2FR_D(v0, ed);
                } else {
                    MOVGR2FR_W(v0, ed);
                    MOVGR2FRH_W(v0, xZR);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                if (rex.w) {
                    FLD_D(v0, wback, fixedaddress);
                } else {
                    FLD_S(v0, wback, fixedaddress);
                    MOVGR2FRH_W(v0, xZR);
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
                FMOV_D(v0, v1);
            } else {
                v0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                FLD_D(v0, wback, fixedaddress);
            }
            break;
        case 0x70:
            INST_NAME("PSHUFW Gm,Em,Ib");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 1);
            u8 = F8;
            VSHUF4I_H(v0, v1, u8);
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
                            VXOR_V(v0, v0, v0);
                        } else if (u8) {
                            VSRLI_H(v0, v0, u8);
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
                        VSRAI_H(v0, v0, u8);
                    }
                    PUTEM(v0);
                    break;
                case 6:
                    INST_NAME("PSLLW Em, Ib");
                    GETEM(v0, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 15) {
                            VXOR_V(v0, v0, v0);
                        } else {
                            VSLLI_H(v0, v0, u8);
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
                            VXOR_V(v0, v0, v0);
                        } else if (u8) {
                            VSRLI_W(v0, v0, u8);
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
                        VSRAI_W(v0, v0, u8);
                    }
                    PUTEM(v0);
                    break;
                case 6:
                    INST_NAME("PSLLD Em, Ib");
                    GETEM(v0, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 31) {
                            VXOR_V(v0, v0, v0);
                        } else {
                            VSLLI_W(v0, v0, u8);
                        }
                        PUTEM(v0);
                    }
                    break;
                default:
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
                            VXOR_V(v0, v0, v0);
                        } else if (u8) {
                            VSRLI_D(v0, v0, u8);
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
                            VXOR_V(v0, v0, v0);
                        } else {
                            VSLLI_D(v0, v0, u8);
                        }
                        PUTEM(v0);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x74:
            INST_NAME("PCMPEQB Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            VSEQ_B(d0, d0, d1);
            break;
        case 0x75:
            INST_NAME("PCMPEQW Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(q0, 0);
            VSEQ_H(v0, v0, q0);
            break;
        case 0x76:
            INST_NAME("PCMPEQD Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VSEQ_W(v0, v0, v1);
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
                ed = TO_NAT((nextop & 0x07) + (rex.b << 3));
                if (rex.w) {
                    MOVFR2GR_D(ed, v0);
                } else {
                    MOVFR2GR_S(ed, v0);
                    ZEROUP(ed);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 1, 0);
                if (rex.w)
                    FST_D(v0, ed, fixedaddress);
                else
                    FST_S(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x7F:
            INST_NAME("MOVQ Em, Gm");
            nextop = F8;
            GETGM(v0);
            if (MODREG) {
                v1 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, nextop & 7);
                FMOV_D(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 1, 0);
                FST_D(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
#define GO(GETFLAGS, NO, YES, NATNO, NATYES, F, I)                                          \
    READFLAGS_FUSION(F, x1, x2, x3, x4, x5);                                                \
    i32_ = F32S;                                                                            \
    if (rex.is32bits)                                                                       \
        j64 = (uint32_t)(addr + i32_);                                                      \
    else                                                                                    \
        j64 = addr + i32_;                                                                  \
    BARRIER(BARRIER_MAYBE);                                                                 \
    JUMP(j64, 1);                                                                           \
    if (!dyn->insts[ninst].nat_flags_fusion) {                                              \
        if (cpuext.lbt) {                                                                   \
            X64_SETJ(x1, I);                                                                \
        } else {                                                                            \
            GETFLAGS;                                                                       \
        }                                                                                   \
    }                                                                                       \
    if (dyn->insts[ninst].x64.jmp_insts == -1 || CHECK_CACHE()) {                           \
        /* out of the block */                                                              \
        i32 = dyn->insts[ninst].epilog - (dyn->native_size);                                \
        if (dyn->insts[ninst].nat_flags_fusion) {                                           \
            NATIVEJUMP_safe(NATNO, i32);                                                    \
        } else {                                                                            \
            if (cpuext.lbt)                                                                 \
                BEQZ_safe(x1, i32);                                                         \
            else                                                                            \
                B##NO##_safe(x1, i32);                                                      \
        }                                                                                   \
        if (dyn->insts[ninst].x64.jmp_insts == -1) {                                        \
            if (!(dyn->insts[ninst].x64.barrier & BARRIER_FLOAT))                           \
                fpu_purgecache(dyn, ninst, 1, tmp1, tmp2, tmp3);                            \
            jump_to_next(dyn, j64, 0, ninst, rex.is32bits);                                 \
        } else {                                                                            \
            CacheTransform(dyn, ninst, cacheupd, tmp1, tmp2, tmp3);                         \
            i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size); \
            B(i32);                                                                         \
        }                                                                                   \
    } else {                                                                                \
        /* inside the block */                                                              \
        i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size);     \
        if (dyn->insts[ninst].nat_flags_fusion) {                                           \
            NATIVEJUMP_safe(NATYES, i32);                                                   \
        } else {                                                                            \
            if (cpuext.lbt)                                                                 \
                BNEZ_safe(tmp1, i32);                                                       \
            else                                                                            \
                B##YES##_safe(tmp1, i32);                                                   \
        }                                                                                   \
    }

            GOCOND(0x80, "J", "Id");

#undef GO


#define GO(GETFLAGS, NO, YES, NATNO, NATYES, F, I)                                           \
    READFLAGS(F);                                                                            \
    tmp1 = x1;                                                                               \
    tmp3 = x3;                                                                               \
    if (cpuext.lbt) {                                                                        \
        X64_SETJ(x3, I);                                                                     \
    } else {                                                                                 \
        GETFLAGS;                                                                            \
        S##YES(x3, x1);                                                                      \
    }                                                                                        \
    nextop = F8;                                                                             \
    if (MODREG) {                                                                            \
        if (rex.rex) {                                                                       \
            eb1 = TO_NAT((nextop & 7) + (rex.b << 3));                                       \
            eb2 = 0;                                                                         \
        } else {                                                                             \
            ed = (nextop & 7);                                                               \
            eb2 = (ed >> 2) * 8;                                                             \
            eb1 = TO_NAT(ed & 3);                                                            \
        }                                                                                    \
        BSTRINS_D(eb1, x3, eb2 + 7, eb2);                                                    \
    } else {                                                                                 \
        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0); \
        ST_B(x3, ed, fixedaddress);                                                          \
        SMWRITE();                                                                           \
    }

            GOCOND(0x90, "SET", "Eb");
#undef GO

        case 0xA2:
            INST_NAME("CPUID");
            NOTEST(x1);
            CALL_(const_cpuid, -1, 0, xRAX, 0);
            // BX and DX are not synchronized durring the call, so need to force the update
            LD_D(xRDX, xEmu, offsetof(x64emu_t, regs[_DX]));
            LD_D(xRBX, xEmu, offsetof(x64emu_t, regs[_BX]));
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
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                if (rex.w)
                    SRAI_D(x1, gd, 6);
                else
                    SRAI_W(x1, gd, 5);
                ALSL_D(x3, x1, wback, 2 + rex.w); // (&ed) += r1*4;
                LDxw(x1, x3, fixedaddress);
                ed = x1;
            }
            if (X_CF) {
                ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
                SRLxw(x4, ed, x2);
                if (cpuext.lbt)
                    X64_SET_EFLAGS(x4, X_CF);
                else
                    BSTRINS_D(xFlags, x4, F_CF, F_CF);
            }
            break;
        case 0xA4:
            nextop = F8;
            INST_NAME("SHLD Ed, Gd, Ib");
            if (geted_ib(dyn, addr, ninst, nextop)) {
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETED(1);
                GETGD;
                u8 = F8;
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
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
            if (BOX64DRENV(dynarec_safeflags) > 1)
                MAYSETFLAGS();
            GETGD;
            GETED(0);
            if (!rex.w && !rex.is32bits && MODREG) { ZEROUP(ed); }
            ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
            BEQ_NEXT(x3, xZR);
            emit_shld32(dyn, ninst, rex, ed, gd, x3, x4, x5, x6);
            WBACK;
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
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                if (rex.w)
                    SRAI_D(x1, gd, 6);
                else
                    SRAI_W(x1, gd, 5);
                ALSL_D(x3, x1, wback, 2 + rex.w);
                LDxw(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
            IFX (X_CF) {
                SRL_D(x4, ed, x2);
                if (cpuext.lbt) {
                    X64_SET_EFLAGS(x4, X_CF);
                } else {
                    BSTRINS_D(xFlags, x4, F_CF, F_CF);
                }
            }
            ADDI_D(x4, xZR, 1);
            SLL_D(x4, x4, x2);
            OR(ed, ed, x4);
            if (wback) {
                SDxw(ed, wback, fixedaddress);
                SMWRITE();
            } else if (!rex.w) {
                ZEROUP(ed);
            }
            break;
        case 0xAC:
            nextop = F8;
            INST_NAME("SHRD Ed, Gd, Ib");
            if (geted_ib(dyn, addr, ninst, nextop)) {
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETED(1);
                GETGD;
                u8 = F8;
                u8 &= (rex.w ? 0x3f : 0x1f);
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
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            if (BOX64DRENV(dynarec_safeflags) > 1)
                MAYSETFLAGS();
            GETGD;
            GETED(0);
            if (!rex.w && !rex.is32bits && MODREG) { ZEROUP(ed); }
            ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
            BEQ_NEXT(x3, xZR);
            emit_shrd32(dyn, ninst, rex, ed, gd, x3, x5, x4, x6);
            WBACK;
            break;
        case 0xAE:
            nextop = F8;
            if (MODREG)
                switch (nextop) {
                    case 0xE8:
                        INST_NAME("LFENCE");
                        SMDMB();
                        break;
                    case 0xF0:
                        INST_NAME("MFENCE");
                        SMDMB();
                        break;
                    case 0xF8:
                        INST_NAME("SFENCE");
                        SMDMB();
                        break;
                    default:
                        DEFAULT;
                }
            else
                switch ((nextop >> 3) & 7) {
                    case 0:
                        INST_NAME("FXSAVE Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        SKIPTEST(x1);
                        fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                        if (MODREG) {
                            DEFAULT;
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x1, x3, &fixedaddress, rex, NULL, 0, 0);
                            CALL(rex.is32bits ? const_fpu_fxsave32 : const_fpu_fxsave64, -1, ed, 0);
                        }
                        break;
                    case 1:
                        INST_NAME("FXRSTOR Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        SKIPTEST(x1);
                        fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x3, &fixedaddress, rex, NULL, 0, 0);
                        CALL(rex.is32bits ? const_fpu_fxrstor32 : const_fpu_fxrstor64, -1, ed, 0);
                        break;
                    case 2:
                        INST_NAME("LDMXCSR Md");
                        GETED(0);
                        ST_W(ed, xEmu, offsetof(x64emu_t, mxcsr));
                        if (BOX64ENV(sse_flushto0)) {
                            /* LA <-> x86
                            16/24 <-> 5    inexact
                            17/25 <-> 4    underflow
                            18/26 <-> 3    overflow
                            19/27 <-> 2    divide by zero
                                x <-> 1    denormal
                            20/28 <-> 0    invalid operation
                            */
                            // Doing x86 -> LA here, ignore denormal
                            XOR(x4, x4, x4);
                            BSTRPICK_W(x3, ed, 5, 5);
                            BSTRINS_W(x4, x3, 16, 16);
                            BSTRPICK_W(x3, ed, 4, 4);
                            BSTRINS_W(x4, x3, 17, 17);
                            BSTRPICK_W(x3, ed, 3, 3);
                            BSTRINS_W(x4, x3, 18, 18);
                            BSTRPICK_W(x3, ed, 2, 2);
                            BSTRINS_W(x4, x3, 19, 19);
                            BSTRPICK_W(x3, ed, 0, 0);
                            BSTRINS_W(x4, x3, 20, 20);
                            MOVGR2FCSR(FCSR2, x4);
                        }
                        break;
                    case 3:
                        INST_NAME("STMXCSR Md");
                        addr = geted(dyn, addr, ninst, nextop, &wback, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                        LD_WU(x4, xEmu, offsetof(x64emu_t, mxcsr));
                        if (BOX64ENV(sse_flushto0)) {
                            MOVFCSR2GR(x5, FCSR2);
                            // Doing LA -> x86 here, ignore denormal
                            BSTRPICK_W(x3, x5, 16, 16);
                            BSTRINS_W(x4, x3, 5, 5);
                            BSTRPICK_W(x3, x5, 17, 17);
                            BSTRINS_W(x4, x3, 4, 4);
                            BSTRPICK_W(x3, x5, 18, 18);
                            BSTRINS_W(x4, x3, 3, 3);
                            BSTRPICK_W(x3, x5, 19, 19);
                            BSTRINS_W(x4, x3, 2, 2);
                            BSTRPICK_W(x3, x5, 20, 20);
                            BSTRINS_W(x4, x3, 0, 0);
                        }
                        ST_W(x4, wback, fixedaddress);
                        break;
                    case 4:
                        INST_NAME("XSAVE Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                        MOV32w(x2, rex.w ? 0 : 1);
                        CALL(const_fpu_xsave, -1, ed, x2);
                        break;
                    case 5:
                        INST_NAME("XRSTOR Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                        MOV32w(x2, rex.w ? 0 : 1);
                        CALL(const_fpu_xrstor, -1, ed, x2);
                        break;
                    case 7:
                        INST_NAME("CLFLUSH Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization?\n");
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                        CALL_(const_native_clflush, -1, 0, ed, 0);
                        break;
                    default:
                        DEFAULT;
                }
            break;
        case 0xAF:
            INST_NAME("IMUL Gd, Ed");
            SETFLAGS(X_ALL, SF_PENDING, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            if (BOX64ENV(dynarec_test)) {
                // avoid noise during test
                CLEAR_FLAGS(x3);
            }
            if (rex.w) {
                // 64bits imul
                UFLAG_IF {
                    if (cpuext.lbt) {
                        X64_MUL_D(gd, ed);
                    }
                    MULH_D(x3, gd, ed);
                    MUL_D(gd, gd, ed);
                    IFX (X_PEND) {
                        UFLAG_OP1(x3);
                        UFLAG_RES(gd);
                        UFLAG_DF(x3, d_imul64);
                    } else {
                        SET_DFNONE();
                    }
                    IFXA (X_CF | X_OF, !cpuext.lbt) {
                        SRAI_D(x4, gd, 63);
                        XOR(x3, x3, x4);
                        SNEZ(x3, x3);
                        IFX (X_CF) {
                            BSTRINS_D(xFlags, x3, F_CF, F_CF);
                        }
                        IFX (X_OF) {
                            BSTRINS_D(xFlags, x3, F_OF, F_OF);
                        }
                    }
                } else {
                    MULxw(gd, gd, ed);
                }
            } else {
                // 32bits imul
                UFLAG_IF {
                    if (cpuext.lbt) {
                        X64_MUL_W(gd, ed);
                    }
                    SLLI_W(gd, gd, 0);
                    SLLI_W(x3, ed, 0);
                    MUL_D(gd, gd, x3);
                    SRLI_D(x3, gd, 32);
                    SLLI_W(gd, gd, 0);
                    IFX (X_PEND) {
                        UFLAG_RES(gd);
                        UFLAG_OP1(x3);
                        UFLAG_DF(x4, d_imul32);
                    } else IFX (X_CF | X_OF) {
                        SET_DFNONE();
                    }
                    IFXA (X_CF | X_OF, !cpuext.lbt) {
                        SRAI_W(x4, gd, 31);
                        SUB_D(x3, x3, x4);
                        SNEZ(x3, x3);
                        IFX (X_CF) {
                            BSTRINS_D(xFlags, x3, F_CF, F_CF);
                        }
                        IFX (X_OF) {
                            BSTRINS_D(xFlags, x3, F_OF, F_OF);
                        }
                    }
                } else {
                    MULxw(gd, gd, ed);
                }
                ZEROUP(gd);
            }
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
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                if (rex.w)
                    SRAI_D(x1, gd, 6);
                else
                    SRAI_W(x1, gd, 5);
                ALSL_D(x3, x1, wback, 2 + rex.w);
                LDxw(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
            SRL_D(x4, ed, x2);
            if(cpuext.lbt) {
                X64_SET_EFLAGS(x4, X_CF);
            } else {
                BSTRINS_D(xFlags, x4, 0, 0);
            }
            ADDI_D(x4, xZR, 1);
            ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
            SLL_D(x4, x4, x2);
            ANDN(ed, ed, x4);
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
            if (MODREG) {
                if (rex.rex) {
                    eb1 = TO_NAT((nextop & 7) + (rex.b << 3));
                    eb2 = 0;
                } else {
                    ed = (nextop & 7);
                    eb1 = TO_NAT(ed & 3); // Ax, Cx, Dx or Bx
                    eb2 = (ed & 4) >> 2;  // L or H
                }
                BSTRPICK_D(gd, eb1, eb2 * 8 + 7, eb2 * 8);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LD_BU(gd, ed, fixedaddress);
            }
            break;
        case 0xB7:
            INST_NAME("MOVZX Gd, Ew");
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                BSTRPICK_D(gd, ed, 15, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LD_HU(gd, ed, fixedaddress);
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
                    BSTRPICK_D(x4, ed, u8, u8);
                    if (cpuext.lbt)
                        X64_SET_EFLAGS(x4, X_CF);
                    else
                        BSTRINS_D(xFlags, x4, 0, 0);
                    break;
                case 5:
                    INST_NAME("BTS Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETED(1);
                    u8 = F8;
                    u8 &= (rex.w ? 0x3f : 0x1f);
                    BSTRPICK_D(x4, ed, u8, u8);
                    if (cpuext.lbt)
                        X64_SET_EFLAGS(x4, X_CF);
                    else
                        BSTRINS_D(xFlags, x4, 0, 0);
                    ADDI_D(x4, xZR, -1);
                    BSTRINS_D(ed, x4, u8, u8);
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
                    BSTRPICK_D(x4, ed, u8, u8);
                    if (cpuext.lbt)
                        X64_SET_EFLAGS(x4, X_CF);
                    else
                        BSTRINS_D(xFlags, x4, 0, 0);
                    BSTRINS_D(ed, xZR, u8, u8);
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
                    BSTRPICK_D(x3, ed, u8, u8);
                    if (cpuext.lbt)
                        X64_SET_EFLAGS(x3, X_CF);
                    else
                        BSTRINS_D(xFlags, x3, 0, 0);
                    if (u8 <= 10) {
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
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                if (rex.w)
                    SRAI_D(x1, gd, 6);
                else
                    SRAI_W(x1, gd, 5);
                ALSL_D(x3, x1, wback, 2 + rex.w);
                LDxw(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
            SRL_D(x4, ed, x2);
            if (cpuext.lbt)
                X64_SET_EFLAGS(x4, X_CF);
            else
                BSTRINS_D(xFlags, x4, F_CF, F_CF);
            ADDI_D(x4, xZR, 1);
            SLL_D(x4, x4, x2);
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
            SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETED(0);
            GETGD;
            if (!rex.w && MODREG) {
                ZEROUP2(x4, ed);
                ed = x4;
            }
            BNE_MARK(ed, xZR);
            if (cpuext.lbt) {
                ADDI_D(x3, xZR, 1 << F_ZF);
                X64_SET_EFLAGS(x3, X_ZF);
            } else {
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            B_NEXT_nocond;
            MARK;
            // gd is undefined if ed is all zeros, don't worry.
            if (rex.w)
                CTZ_D(gd, ed);
            else
                CTZ_W(gd, ed);
            if (cpuext.lbt) {
                X64_SET_EFLAGS(xZR, X_ZF);
            } else {
                BSTRINS_D(xFlags, xZR, F_ZF, F_ZF);
            }
            break;
        case 0xBD:
            INST_NAME("BSR Gd, Ed");
            SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETED(0);
            GETGD;
            if (!rex.w && MODREG) {
                ZEROUP2(x4, ed);
                ed = x4;
            }
            BNE_MARK(ed, xZR);
            if (cpuext.lbt) {
                ADDI_D(x3, xZR, 1 << F_ZF);
                X64_SET_EFLAGS(x3, X_ZF);
            } else {
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            B_NEXT_nocond;
            MARK;
            if (cpuext.lbt) {
                X64_SET_EFLAGS(xZR, X_ZF);
            } else {
                BSTRINS_D(xFlags, xZR, F_ZF, F_ZF);
            }
            if (rex.w)
                CLZ_D(gd, ed);
            else
                CLZ_W(gd, ed);
            ADDI_D(x1, xZR, rex.w ? 63 : 31);
            SUB_D(gd, x1, gd);
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
                BSTRPICK_D(gd, wback, wb2 + 7, wb2);
                EXT_W_B(gd, gd);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                LD_B(gd, ed, fixedaddress);
            }
            if (!rex.w) ZEROUP(gd);
            break;
        case 0xBF:
            INST_NAME("MOVSX Gd, Ew");
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                EXT_W_H(gd, ed);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                LD_H(gd, ed, fixedaddress);
            }
            if (!rex.w) ZEROUP(gd);
            break;
        case 0xC2:
            INST_NAME("CMPPS Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 1);
            u8 = F8;
            switch (u8 & 7) {
                case 0: VFCMP_S(v0, v0, v1, cEQ); break;  // Equal
                case 1: VFCMP_S(v0, v0, v1, cLT); break;  // Less than
                case 2: VFCMP_S(v0, v0, v1, cLE); break;  // Less or equal
                case 3: VFCMP_S(v0, v0, v1, cUN); break;  // NaN
                case 4: VFCMP_S(v0, v0, v1, cUNE); break; // Not Equal or unordered
                case 5: VFCMP_S(v0, v1, v0, cULE); break; // Greater or equal or unordered
                case 6: VFCMP_S(v0, v1, v0, cULT); break; // Greater or unordered, test inverted, N!=V so unordered or less than (inverted)
                case 7: VFCMP_S(v0, v0, v1, cOR); break;  // not NaN
            }
            break;
        case 0xC3:
            INST_NAME("MOVNTI Ed, Gd");
            nextop = F8;
            GETGD;
            if (MODREG) {
                MVxw(TO_NAT((nextop & 7) + (rex.b << 3)), gd);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                SDxw(gd, ed, fixedaddress);
            }
            break;
        case 0xC4:
            INST_NAME("PINSRW Gm,Ed,Ib");
            nextop = F8;
            GETGM(v0);
            if (MODREG) {
                u8 = (F8) & 3;
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x4, &fixedaddress, rex, NULL, 1, 1);
                u8 = (F8) & 3;
                ed = x3;
                LD_HU(ed, wback, fixedaddress);
            }
            VINSGR2VR_H(v0, ed, u8);
            break;
        case 0xC5:
            INST_NAME("PEXTRW Gd,Em,Ib");
            nextop = F8;
            GETGD;
            if (MODREG) {
                GETEM(v0, 1);
                u8 = (F8) & 3;
                VPICKVE2GR_HU(gd, v0, u8);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x4, &fixedaddress, rex, NULL, 0, 1);
                u8 = (F8) & 3;
                LD_HU(gd, wback, (u8 << 1));
            }
            break;
        case 0xC6:
            INST_NAME("SHUFPS Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 1);
            u8 = F8;
            if (v0 == v1) {
                VSHUF4I_W(v0, v0, u8);
            } else {
                q1 = fpu_get_scratch(dyn);
                VSHUF4I_W(v0, v0, u8);
                VSHUF4I_W(q1, v1, u8);
                VEXTRINS_D(v0, q1, 0x11); // v0[127:64] = q1[127:64]
            }
            break;
        case 0xC7:
            // rep has no impact here
            nextop = F8;
            if (MODREG) {
                switch ((nextop >> 3) & 7) {
                    default:
                        DEFAULT;
                }
            } else {
                switch ((nextop >> 3) & 7) {
                    case 4:
                        INST_NAME("Unsupported XSAVEC Ed");
                        FAKEED;
                        UDF();
                        break;
                    default:
                        DEFAULT;
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
            REVBxw(gd, gd);
            break;
        case 0xD1:
            INST_NAME("PSRLW Gm, Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            VSLEI_DU(v0, d1, 15);
            VREPLVEI_H(v1, d1, 0);
            VSRL_H(d0, d0, v1);
            VAND_V(d0, d0, v0);
            break;
        case 0xD2:
            INST_NAME("PSRLD Gm, Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            VSLEI_DU(v0, d1, 31);
            VREPLVEI_W(v1, d1, 0);
            VSRL_W(d0, d0, v1);
            VAND_V(d0, d0, v0);
            break;
        case 0xD3:
            INST_NAME("PSRLQ Gm, Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            v0 = fpu_get_scratch(dyn);
            VLDI(v0, 0b0110000111111); // broadcast 63 as 64bit imm
            VSLE_DU(v0, d1, v0);
            VSRL_D(d0, d0, d1);
            VAND_V(d0, d0, v0);
            break;
        case 0xD4:
            INST_NAME("PADDQ Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VADD_D(v0, v0, v1);
            break;
        case 0xD5:
            INST_NAME("PMULLW Gm, Em");
            nextop = F8;
            GETGM(q0);
            GETEM(q1, 0);
            VMUL_H(q0, q0, q1);
            break;
        case 0xD7:
            nextop = F8;
            INST_NAME("PMOVMSKB Gd, Em");
            GETGD;
            v0 = fpu_get_scratch(dyn);
            GETEM(v1, 0);
            VMSKLTZ_B(v0, v1);
            MOVFR2GR_D(x1, v0);
            BSTRPICK_D(gd, x1, 7, 0);
            break;
        case 0xD8:
            INST_NAME("PSUBUSB Gm, Em");
            nextop = F8;
            GETGM(q0);
            GETEM(q1, 0);
            VSSUB_BU(q0, q0, q1);
            break;
        case 0xD9:
            INST_NAME("PSUBUSW Gm, Em");
            nextop = F8;
            GETGM(q0);
            GETEM(q1, 0);
            VSSUB_HU(q0, q0, q1);
            break;
        case 0xDA:
            INST_NAME("PMINUB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VMIN_BU(v0, v0, v1);
            break;
        case 0xDB:
            INST_NAME("PAND Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VAND_V(v0, v0, v1);
            break;
        case 0xDC:
            INST_NAME("PADDUSB Gm, Em");
            nextop = F8;
            GETGM(q0);
            GETEM(q1, 0);
            VSADD_BU(q0, q0, q1);
            break;
        case 0xDD:
            INST_NAME("PADDUSW Gm, Em");
            nextop = F8;
            GETGM(q0);
            GETEM(q1, 0);
            VSADD_HU(q0, q0, q1);
            break;
        case 0xDE:
            INST_NAME("PMAXUB Gm, Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            VMAX_BU(d0, d0, d1);
            break;
        case 0xDF:
            INST_NAME("PANDN Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VANDN_V(v0, v0, v1);
            break;
        case 0xE0:
            INST_NAME("PAVGB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VAVGR_BU(v0, v0, v1);
            break;
        case 0xE1:
            INST_NAME("PSRAW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            q0 = fpu_get_scratch(dyn);
            VMINI_DU(q0, v1, 15);
            VREPLVEI_H(q0, q0, 0);
            VSRA_H(v0, v0, q0);
            break;
        case 0xE2:
            INST_NAME("PSRAD Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            q0 = fpu_get_scratch(dyn);
            VMINI_DU(q0, v1, 31);
            VREPLVEI_W(q0, q0, 0);
            VSRA_W(v0, v0, q0);
            break;
        case 0xE3:
            INST_NAME("PAVGW Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VAVGR_HU(v0, v0, v1);
            break;
        case 0xE4:
            INST_NAME("PMULHUW Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VMUH_HU(v0, v0, v1);
            break;
        case 0xE5:
            INST_NAME("PMULHW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VMUH_H(v0, v0, v1);
            break;
        case 0xE7:
            INST_NAME("MOVNTQ Em, Gm");
            nextop = F8;
            GETGM(v0);
            if (MODREG) {
                v1 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, nextop & 7);
                FMOV_D(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 1, 0);
                FST_D(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0xE8:
            INST_NAME("PSUBSB Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(q0, 0);
            VSSUB_B(v0, v0, q0);
            break;
        case 0xE9:
            INST_NAME("PSUBSW Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(q0, 0);
            VSSUB_H(v0, v0, q0);
            break;
        case 0xEA:
            INST_NAME("PMINSW Gx, Ex");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VMIN_H(v0, v0, v1);
            break;
        case 0xEB:
            INST_NAME("POR Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VOR_V(v0, v0, v1);
            break;
        case 0xEC:
            INST_NAME("PADDSB Gm, Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            VSADD_B(d0, d0, d1);
            break;
        case 0xED:
            INST_NAME("PADDSW Gm, Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            VSADD_H(d0, d0, d1);
            break;
        case 0xEE:
            INST_NAME("PMAXSW Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VMAX_H(v0, v0, v1);
            break;
        case 0xEF:
            INST_NAME("PXOR Gm, Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            VXOR_V(d0, d0, d1);
            break;
        case 0xF1:
            INST_NAME("PSLLW Gm, Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            VSLEI_DU(v0, d1, 15);
            VREPLVEI_H(v1, d1, 0);
            VSLL_H(d0, d0, v1);
            VAND_V(d0, d0, v0);
            break;
        case 0xF2:
            INST_NAME("PSLLD Gm, Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            VSLEI_DU(v0, d1, 31);
            VREPLVEI_W(v1, d1, 0);
            VSLL_W(d0, d0, v1);
            VAND_V(d0, d0, v0);
            break;
        case 0xF3:
            INST_NAME("PSLLQ Gm, Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            v0 = fpu_get_scratch(dyn);
            VLDI(v0, 0b0110000111111); // broadcast 63 as 64bit imm
            VSLE_DU(v0, d1, v0);
            VSLL_D(d0, d0, d1);
            VAND_V(d0, d0, v0);
            break;
        case 0xF4:
            INST_NAME("PMULUDQ Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VMULWEV_D_WU(v0, v0, v1);
            break;
        case 0xF5:
            INST_NAME("PMADDWD Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            q0 = fpu_get_scratch(dyn);
            VXOR_V(q0, q0, q0);
            VMADDWEV_W_H(q0, v0, v1);
            VMADDWOD_W_H(q0, v0, v1);
            VBSLL_V(v0, q0, 0);
            break;
        case 0xF6:
            INST_NAME("PSADBW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VABSD_BU(v0, v0, v1);
            VHADDW_HU_BU(v0, v0, v0);
            VHADDW_WU_HU(v0, v0, v0);
            VHADDW_DU_WU(v0, v0, v0);
            break;
        case 0xF7:
            INST_NAME("MASKMOVQ Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            VSLTI_B(q1, v1, 0); // q1 = byte selection mask
            FLD_D(q0, xRDI, 0);
            VBITSEL_V(q0, q0, v0, q1); // sel v0 if mask is 1
            FST_D(q0, xRDI, 0);
            break;
        case 0xF8:
            INST_NAME("PSUBB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VSUB_B(v0, v0, v1);
            break;
        case 0xF9:
            INST_NAME("PSUBW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VSUB_H(v0, v0, v1);
            break;
        case 0xFA:
            INST_NAME("PSUBD Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VSUB_W(v0, v0, v1);
            break;
        case 0xFB:
            INST_NAME("PSUBQ Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VSUB_D(v0, v0, v1);
            break;
        case 0xFC:
            INST_NAME("PADDB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VADD_B(v0, v0, v1);
            break;
        case 0xFD:
            INST_NAME("PADDW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VADD_H(v0, v0, v1);
            break;
        case 0xFE:
            INST_NAME("PADDD Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VADD_W(v0, v0, v1);
            break;
        default:
            DEFAULT;
    }
    return addr;
}
