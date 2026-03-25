#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "ppc64le_mapping.h"
#include "x64_signals.h"
#include "os.h"
#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "ppc64le_emitter.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "bridge.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "custommem.h"
#include "alternate.h"

#include "ppc64le_printer.h"
#include "dynarec_ppc64le_private.h"
#include "dynarec_ppc64le_functions.h"
#include "../dynarec_helper.h"

int isSimpleWrapper(wrapper_t fun);
int isRetX87Wrapper(wrapper_t fun);

uintptr_t dynarec64_00(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    uint8_t nextop, opcode;
    uint8_t gd, ed, tmp1, tmp2, tmp3;
    int8_t i8;
    uint8_t gb1, gb2, eb1, eb2;
    uint8_t wback, wb1, wb2;
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    int64_t j64;
    int v0, v1;
    int i32;
    int64_t i64, fixedaddress;
    int32_t tmp;
    int lock;
    int cacheupd = 0;
    MAYUSE(tmp1);
    MAYUSE(tmp2);
    MAYUSE(tmp3);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(j64);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(i32);
    MAYUSE(i64);
    MAYUSE(tmp);
    MAYUSE(lock);
    MAYUSE(wb1);
    MAYUSE(i8);
    MAYUSE(u16);
    MAYUSE(cacheupd);

    opcode = F8;

    switch (opcode) {
        case 0x00:
            INST_NAME("ADD Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGBEB(x1, x2, 0);
            emit_add8(dyn, ninst, ed, gd, x4, x5);
            EBBACK();
            break;
        case 0x01:
            INST_NAME("ADD Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_add32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            WBACK;
            break;
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGBEB(x1, x2, 0);
            emit_add8(dyn, ninst, gd, ed, x4, x5);
            GBBACK();
            break;
        case 0x03:
            INST_NAME("ADD Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_add32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;
        case 0x04:
            INST_NAME("ADD AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            u8 = F8;
            ANDI(x1, xRAX, 0xff);
            emit_add8c(dyn, ninst, x1, u8, x3, x4, x5);
            BF_INSERT(xRAX, x1, 7, 0);
            break;
        case 0x05:
            INST_NAME("ADD EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            emit_add32c(dyn, ninst, rex, xRAX, i64, x3, x4, x5, x6);
            break;
        case 0x08:
            INST_NAME("OR Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGBEB(x1, x2, 0);
            emit_or8(dyn, ninst, ed, gd, x4, x5);
            EBBACK();
            break;
        case 0x09:
            INST_NAME("OR Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_or32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x0A:
            INST_NAME("OR Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGBEB(x1, x2, 0);
            emit_or8(dyn, ninst, gd, ed, x4, x5);
            GBBACK();
            break;
        case 0x0B:
            INST_NAME("OR Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_or32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x0C:
            INST_NAME("OR AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            u8 = F8;
            ANDI(x1, xRAX, 0xff);
            emit_or8c(dyn, ninst, x1, u8, x3, x4, x5);
            BF_INSERT(xRAX, x1, 7, 0);
            break;
        case 0x0D:
            INST_NAME("OR EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            emit_or32c(dyn, ninst, rex, xRAX, i64, x3, x4);
            break;
        case 0x10:
            INST_NAME("ADC Eb, Gb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGBEB(x1, x2, 0);
            emit_adc8(dyn, ninst, ed, gd, x4, x5, x6);
            EBBACK();
            break;
        case 0x11:
            INST_NAME("ADC Ed, Gd");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_adc32(dyn, ninst, rex, ed, gd, x3, x4, x5, x6);
            WBACK;
            break;
        case 0x12:
            INST_NAME("ADC Gb, Eb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGBEB(x1, x2, 0);
            emit_adc8(dyn, ninst, gd, ed, x4, x6, x5);
            GBBACK();
            break;
        case 0x13:
            INST_NAME("ADC Gd, Ed");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_adc32(dyn, ninst, rex, gd, ed, x3, x4, x5, x6);
            break;
        case 0x14:
            INST_NAME("ADC AL, Ib");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            u8 = F8;
            ANDI(x1, xRAX, 0xff);
            emit_adc8c(dyn, ninst, x1, u8, x3, x4, x5, x6);
            BF_INSERT(xRAX, x1, 7, 0);
            break;
        case 0x15:
            INST_NAME("ADC EAX, Id");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            MOV64xw(x1, i64);
            emit_adc32(dyn, ninst, rex, xRAX, x1, x3, x4, x5, x6);
            break;
        case 0x18:
            INST_NAME("SBB Eb, Gb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGBEB(x1, x2, 0);
            emit_sbb8(dyn, ninst, ed, gd, x4, x5, x6);
            EBBACK();
            break;
        case 0x19:
            INST_NAME("SBB Ed, Gd");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sbb32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            WBACK;
            break;
        case 0x1A:
            INST_NAME("SBB Gb, Eb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGBEB(x1, x2, 0);
            emit_sbb8(dyn, ninst, gd, ed, x6, x4, x5);
            GBBACK();
            break;
        case 0x1B:
            INST_NAME("SBB Gd, Ed");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sbb32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;
        case 0x1C:
            INST_NAME("SBB AL, Ib");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            u8 = F8;
            ANDI(x1, xRAX, 0xff);
            emit_sbb8c(dyn, ninst, x1, u8, x3, x4, x5, x6);
            BF_INSERT(xRAX, x1, 7, 0);
            break;
        case 0x1D:
            INST_NAME("SBB EAX, Id");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            MOV64xw(x2, i64);
            emit_sbb32(dyn, ninst, rex, xRAX, x2, x3, x4, x5);
            break;
        case 0x28:
            INST_NAME("SUB Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGBEB(x1, x2, 0);
            emit_sub8(dyn, ninst, ed, gd, x4, x5, x6);
            EBBACK();
            break;
        case 0x29:
            INST_NAME("SUB Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sub32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            WBACK;
            break;
        case 0x2A:
            INST_NAME("SUB Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGBEB(x1, x2, 0);
            emit_sub8(dyn, ninst, gd, ed, x4, x5, x6);
            GBBACK();
            break;
        case 0x2B:
            INST_NAME("SUB Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sub32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;
        case 0x2C:
            INST_NAME("SUB AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            u8 = F8;
            ANDI(x1, xRAX, 0xff);
            emit_sub8c(dyn, ninst, x1, u8, x3, x4, x5, x6);
            BF_INSERT(xRAX, x1, 7, 0);
            break;
        case 0x2D:
            INST_NAME("SUB EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            emit_sub32c(dyn, ninst, rex, xRAX, i64, x3, x4, x5, x6);
            break;
        case 0x20:
            INST_NAME("AND Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGBEB(x1, x2, 0);
            emit_and8(dyn, ninst, ed, gd, x4, x5);
            EBBACK();
            break;
        case 0x21:
            INST_NAME("AND Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_and32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x22:
            INST_NAME("AND Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGBEB(x1, x2, 0);
            emit_and8(dyn, ninst, gd, ed, x4, x5);
            GBBACK();
            break;
        case 0x23:
            INST_NAME("AND Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_and32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x24:
            INST_NAME("AND AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            u8 = F8;
            ANDI(x1, xRAX, 0xff);
            emit_and8c(dyn, ninst, x1, u8, x3, x4);
            BF_INSERT(xRAX, x1, 7, 0);
            break;
        case 0x25:
            INST_NAME("AND EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            emit_and32c(dyn, ninst, rex, xRAX, i64, x3, x4);
            break;
        case 0x30:
            INST_NAME("XOR Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGBEB(x1, x2, 0);
            emit_xor8(dyn, ninst, ed, gd, x4, x5);
            EBBACK();
            break;
        case 0x31:
            INST_NAME("XOR Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_xor32(dyn, ninst, rex, ed, gd, x3, x4);
            if (ed != gd) {
                WBACK;
            }
            break;
        case 0x32:
            INST_NAME("XOR Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGBEB(x1, x2, 0);
            emit_xor8(dyn, ninst, gd, ed, x4, x5);
            GBBACK();
            break;
        case 0x33:
            INST_NAME("XOR Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_xor32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x34:
            INST_NAME("XOR AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            u8 = F8;
            ANDI(x1, xRAX, 0xff);
            emit_xor8c(dyn, ninst, x1, u8, x3, x4);
            BF_INSERT(xRAX, x1, 7, 0);
            break;
        case 0x35:
            INST_NAME("XOR EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            emit_xor32c(dyn, ninst, rex, xRAX, i64, x3, x4);
            break;
        case 0x38:
            INST_NAME("CMP Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGBEB(x1, x2, 0);
            emit_cmp8(dyn, ninst, ed, gd, x3, x4, x5, x6);
            break;
        case 0x39:
            INST_NAME("CMP Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_cmp32(dyn, ninst, rex, ed, gd, x3, x4, x5, x6);
            break;
        case 0x3A:
            INST_NAME("CMP Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGBEB(x1, x2, 0);
            emit_cmp8(dyn, ninst, gd, ed, x3, x4, x5, x6);
            break;
        case 0x3B:
            INST_NAME("CMP Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_cmp32(dyn, ninst, rex, gd, ed, x3, x4, x5, x6);
            break;
        case 0x3C:
            INST_NAME("CMP AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            u8 = F8;
            ANDI(x1, xRAX, 0xff);
            if (u8) {
                MOV32w(x2, u8);
                emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
            } else {
                emit_cmp8_0(dyn, ninst, x1, x3, x4);
            }
            break;
        case 0x3D:
            INST_NAME("CMP EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            if (i64) {
                MOV64x(x2, i64);
                emit_cmp32(dyn, ninst, rex, xRAX, x2, x3, x4, x5, x6);
            } else
                emit_cmp32_0(dyn, ninst, rex, 0xC0 /* fake nextop */, xRAX, x3, x4, x5);
            break;
        case 0x63:
            if (rex.is32bits) {
                // this is ARPL opcode
                DEFAULT;
            } else {
                INST_NAME("MOVSXD Gd, Ed");
                nextop = F8;
                GETGD;
                SCRATCH_USAGE(0);
                if (rex.w) {
                    if (MODREG) { // reg <= reg
                        EXTSW(gd, TO_NAT((nextop & 7) + (rex.b << 3)));
                    } else { // mem <= reg
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                        LWA(gd, fixedaddress, ed);
                    }
                } else {
                    if (MODREG) { // reg <= reg
                        ZEROUP2(gd, TO_NAT((nextop & 7) + (rex.b << 3)));
                    } else { // mem <= reg
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                        LWZ(gd, fixedaddress, ed);
                    }
                }
            }
            break;
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
            INST_NAME("PUSH reg");
            SCRATCH_USAGE(0);
            gd = TO_NAT((opcode & 0x07) + (rex.b << 3));
            PUSH1z(gd);
            SMWRITE();
            break;
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5E:
        case 0x5F:
            INST_NAME("POP reg");
            SCRATCH_USAGE(0);
            SMREAD();
            gd = TO_NAT((opcode & 0x07) + (rex.b << 3));
            POP1z(gd);
            break;
        case 0x68:
            INST_NAME("PUSH Id");
            i64 = F32S;
            if (PK(0) == 0xC3) {
                MESSAGE(LOG_DUMP, "PUSH then RET, using indirect\n");
                TABLE64(x3, addr - 4);
                LWA(x1, 0, x3);
                PUSH1z(x1);
            } else {
                MOV64z(x3, i64);
                PUSH1z(x3);
                SMWRITE();
            }
            break;
        case 0x69:
            INST_NAME("IMUL Gd, Ed, Id");
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETGD;
            GETED(4);
            i64 = F32S;
            MOV64xw(x4, i64);
            CLEAR_FLAGS(x3);
            if (rex.w) {
                UFLAG_IF {
                    MULHD(x3, ed, x4);
                    MULLD(gd, ed, x4);
                    SET_DFNONE();
                    IFX (X_CF | X_OF) {
                        SRADI(x4, gd, 63);
                        XOR(x3, x3, x4);
                        SNEZ(x3, x3);
                        IFX (X_CF) BF_INSERT(xFlags, x3, F_CF, F_CF);
                        IFX (X_OF) BF_INSERT(xFlags, x3, F_OF, F_OF);
                    }
                } else {
                    MULLD(gd, ed, x4);
                }
            } else {
                UFLAG_IF {
                    EXTSW(x3, ed);
                    MULLD(x5, x3, x4);
                    SRDI(x3, x5, 32);
                    EXTSW(gd, x5);
                    SET_DFNONE();
                    IFX (X_CF | X_OF) {
                        XOR(x3, gd, x5);
                        SNEZ(x3, x3);
                        IFX (X_CF) BF_INSERT(xFlags, x3, F_CF, F_CF);
                        IFX (X_OF) BF_INSERT(xFlags, x3, F_OF, F_OF);
                    }
                } else {
                    MULLW(gd, ed, x4);
                }
                ZEROUP(gd);
            }
            IFX (X_SF) {
                SRDI(x3, gd, rex.w ? 63 : 31);
                BF_INSERT(xFlags, x3, F_SF, F_SF);
            }
            IFX (X_PF) emit_pf(dyn, ninst, gd, x3, x4);
            IFX (X_ALL) SPILL_EFLAGS();
            break;
        case 0x6A:
            INST_NAME("PUSH Ib");
            i64 = F8S;
            MOV64z(x3, i64);
            PUSH1z(x3);
            SMWRITE();
            break;
        case 0x6B:
            INST_NAME("IMUL Gd, Ed, Ib");
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETGD;
            GETED(1);
            i64 = F8S;
            MOV64x(x4, i64);
            CLEAR_FLAGS(x3);
            if (rex.w) {
                UFLAG_IF {
                    MULHD(x3, ed, x4);
                    MULLD(gd, ed, x4);
                    SET_DFNONE();
                    IFX (X_CF | X_OF) {
                        SRADI(x4, gd, 63);
                        XOR(x3, x3, x4);
                        SNEZ(x3, x3);
                        IFX (X_CF) BF_INSERT(xFlags, x3, F_CF, F_CF);
                        IFX (X_OF) BF_INSERT(xFlags, x3, F_OF, F_OF);
                    }
                } else {
                    MULLD(gd, ed, x4);
                }
            } else {
                UFLAG_IF {
                    EXTSW(x3, ed);
                    MULLD(x5, x3, x4);
                    SRDI(x3, x5, 32);
                    EXTSW(gd, x5);
                    SET_DFNONE();
                    IFX (X_CF | X_OF) {
                        XOR(x3, gd, x5);
                        SNEZ(x3, x3);
                        IFX (X_CF) BF_INSERT(xFlags, x3, F_CF, F_CF);
                        IFX (X_OF) BF_INSERT(xFlags, x3, F_OF, F_OF);
                    }
                } else {
                    MULLW(gd, ed, x4);
                }
                ZEROUP(gd);
            }
            IFX (X_SF) {
                SRDI(x3, gd, rex.w ? 63 : 31);
                BF_INSERT(xFlags, x3, F_SF, F_SF);
            }
            IFX (X_PF) emit_pf(dyn, ninst, gd, x3, x4);
            IFX (X_ALL) SPILL_EFLAGS();
            break;
#define GO(GETFLAGS, NO, YES, NATNO, NATYES, F, I)                                          \
    READFLAGS_FUSION(F, x1, x2, x3, x4, x5);                                                \
    i8 = F8S;                                                                               \
    JUMP(addr + i8, 1);                                                                     \
    if (!dyn->insts[ninst].nat_flags_fusion) {                                              \
        GETFLAGS;                                                                           \
    }                                                                                       \
    if (dyn->insts[ninst].x64.jmp_insts == -1 || CHECK_CACHE()) {                           \
        /* out of block */                                                                  \
        i32 = dyn->insts[ninst].epilog - (dyn->native_size);                                \
        if (dyn->insts[ninst].nat_flags_fusion) {                                           \
            NATIVEJUMP_safe(NATNO, i32);                                                    \
        } else {                                                                            \
            B##NO##_safe(tmp1, i32);                                                        \
        }                                                                                   \
        if (dyn->insts[ninst].x64.jmp_insts == -1) {                                        \
            if (!(dyn->insts[ninst].x64.barrier & BARRIER_FLOAT))                           \
                fpu_purgecache(dyn, ninst, 1, tmp1, tmp2, tmp3);                            \
            jump_to_next(dyn, addr + i8, 0, ninst, rex.is32bits);                           \
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
            B##YES##_safe(tmp1, i32);                                                       \
        }                                                                                   \
    }

            GOCOND(0x70, "J", "ib");
#undef GO


        case 0x82:
            if (!rex.is32bits) {
                INST_NAME("Invalid 82");
                UDF();
                *need_epilog = 1;
                *ok = 0;
                return addr;
            }
            // fallthru
        case 0x80:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0: // ADD
                    INST_NAME("ADD Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_add8c(dyn, ninst, x1, u8, x2, x4, x5);
                    EBBACK();
                    break;
                case 1: // OR
                    INST_NAME("OR Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_or8c(dyn, ninst, x1, u8, x2, x4, x5);
                    EBBACK();
                    break;
                case 2: // ADC
                    INST_NAME("ADC Eb, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_adc8c(dyn, ninst, x1, u8, x2, x4, x5, x6);
                    EBBACK();
                    break;
                case 3: // SBB
                    INST_NAME("SBB Eb, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_sbb8c(dyn, ninst, x1, u8, x2, x4, x5, x6);
                    EBBACK();
                    break;
                case 4: // AND
                    INST_NAME("AND Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_and8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK();
                    break;
                case 5: // SUB
                    INST_NAME("SUB Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_sub8c(dyn, ninst, x1, u8, x2, x4, x5, x6);
                    EBBACK();
                    break;
                case 6: // XOR
                    INST_NAME("XOR Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_xor8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK();
                    break;
                case 7: // CMP
                    INST_NAME("CMP Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEB(x1, 1);
                    u8 = F8;
                    if (u8) {
                        LI(x2, u8);
                        emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    } else {
                        emit_cmp8_0(dyn, ninst, x1, x3, x4);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x81:
        case 0x83:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0: // ADD
                    if (opcode == 0x81) {
                        INST_NAME("ADD Ed, Id");
                    } else {
                        INST_NAME("ADD Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETED((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    emit_add32c(dyn, ninst, rex, ed, i64, x3, x4, x5, x6);
                    WBACK;
                    break;
                case 1: // OR
                    if (opcode == 0x81) {
                        INST_NAME("OR Ed, Id");
                    } else {
                        INST_NAME("OR Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETED((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    emit_or32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACK;
                    break;
                case 2: // ADC
                    if (opcode == 0x81) {
                        INST_NAME("ADC Ed, Id");
                    } else {
                        INST_NAME("ADC Ed, Ib");
                    }
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETED((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    MOV64xw(x5, i64);
                    emit_adc32(dyn, ninst, rex, ed, x5, x3, x4, x6, x7);
                    WBACK;
                    break;
                case 3: // SBB
                    if (opcode == 0x81) {
                        INST_NAME("SBB Ed, Id");
                    } else {
                        INST_NAME("SBB Ed, Ib");
                    }
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETED((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    MOV64xw(x5, i64);
                    emit_sbb32(dyn, ninst, rex, ed, x5, x3, x4, x6);
                    WBACK;
                    break;
                case 4: // AND
                    if (opcode == 0x81) {
                        INST_NAME("AND Ed, Id");
                    } else {
                        INST_NAME("AND Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETED((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    emit_and32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACK;
                    break;
                case 5: // SUB
                    if (opcode == 0x81) {
                        INST_NAME("SUB Ed, Id");
                    } else {
                        INST_NAME("SUB Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETED((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    emit_sub32c(dyn, ninst, rex, ed, i64, x3, x4, x5, x6);
                    WBACK;
                    break;
                case 6: // XOR
                    if (opcode == 0x81) {
                        INST_NAME("XOR Ed, Id");
                    } else {
                        INST_NAME("XOR Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETED((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    emit_xor32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACK;
                    break;
                case 7: // CMP
                    if (opcode == 0x81) {
                        INST_NAME("CMP Ed, Id");
                    } else {
                        INST_NAME("CMP Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETED((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    if (i64) {
                        MOV64x(x2, i64);
                        emit_cmp32(dyn, ninst, rex, ed, x2, x3, x4, x5, x6);
                    } else
                        emit_cmp32_0(dyn, ninst, rex, nextop, ed, x3, x4, x5);
                    break;
            }
            break;
        case 0x84:
            INST_NAME("TEST Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGBEB(x1, x2, 0);
            emit_test8(dyn, ninst, ed, gd, x3, x4, x5);
            break;
        case 0x85:
            INST_NAME("TEST Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_test32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            break;
        case 0x86:
            INST_NAME("(LOCK) XCHG Eb, Gb");
            nextop = F8;
            if (MODREG) {
                GETGB(x1);
                GETEB(x2, 0);
                // swap gd and ed using bit-field insert
                BF_INSERT(wback, gd, wb2 + 7, wb2);
                BF_INSERT(gb1, ed, gb2 + 7, gb2);
            } else {
                GETGB(x1);
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x2, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                // XCHG with memory is always atomic on x86, even without LOCK prefix
                // Use LBARX/STBCXd for byte-level LL/SC
                LWSYNC();
                MARKLOCK;
                LBARX(x4, 0, wback);
                STBCXd(gd, 0, wback);
                BNE_MARKLOCK_CR0;
                LWSYNC();
                // x4 = old byte from memory, store into Gb
                BF_INSERT(gb1, x4, gb2 + 7, gb2);
            }
            break;
        case 0x87:
            INST_NAME("(LOCK) XCHG Ed, Gd");
            nextop = F8;
            if (MODREG) {
                GETGD;
                GETED(0);
                MVxw(x1, gd);
                MVxw(gd, ed);
                MVxw(ed, x1);
            } else {
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                // XCHG with memory is always atomic on x86, even without LOCK prefix
                LWSYNC();
                MARKLOCK;
                if (rex.w) {
                    LDARX(x1, 0, wback);
                    STDCXd(gd, 0, wback);
                } else {
                    LWARX(x1, 0, wback);
                    STWCXd(gd, 0, wback);
                }
                BNE_MARKLOCK_CR0;
                LWSYNC();
                MVxw(gd, x1);
            }
            break;
        case 0x88:
            INST_NAME("MOV Eb, Gb");
            nextop = F8;
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            if (rex.rex) {
                gb2 = 0;
                gb1 = TO_NAT(gd);
            } else {
                gb2 = ((gd & 4) << 1);
                gb1 = TO_NAT(gd & 3);
            }
            if (gb2) {
                gd = x4;
                BF_EXTRACT(gd, gb1, gb2 + 7, gb2);
            } else {
                gd = gb1; // no need to extract
            }
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                if (rex.rex) {
                    eb1 = TO_NAT(ed);
                    eb2 = 0;
                } else {
                    eb1 = TO_NAT(ed & 3);  // Ax, Cx, Dx or Bx
                    eb2 = ((ed & 4) << 1); // L or H
                }
                BF_INSERT(eb1, gd, eb2 + 7, eb2);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, DS_DISP, 0);
                STB(gd, fixedaddress, ed);
                SMWRITELOCK(lock);
            }
            break;
        case 0x89:
            INST_NAME("MOV Ed, Gd");
            nextop = F8;
            GETGD;
            SCRATCH_USAGE(0);
            if (MODREG) { // reg <= reg
                MVxw(TO_NAT((nextop & 7) + (rex.b << 3)), gd);
            } else { // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, DS_DISP, 0);
                if(!lock && BOX64ENV(unity) && !VolatileRangesContains(ip) && ((fixedaddress==0x80) || (fixedaddress==0x84) || (fixedaddress==0xc0) || (fixedaddress==0xc4))) {
                    DMB_ISH();
                    lock = 1;
                }
                SDxw(gd, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0x8A:
            INST_NAME("MOV Gb, Eb");
            nextop = F8;
            if (rex.rex) {
                gb1 = gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3));
                gb2 = 0;
            } else {
                gd = (nextop & 0x38) >> 3;
                gb1 = TO_NAT(gd & 3);
                gb2 = ((gd & 4) << 1);
            }
            if (MODREG) {
                if (rex.rex) {
                    wback = TO_NAT((nextop & 7) + (rex.b << 3));
                    wb2 = 0;
                } else {
                    wback = (nextop & 7);
                    wb2 = (wback >> 2);
                    wback = TO_NAT(wback & 3);
                }
                if (wb2) {
                    BF_EXTRACT(x4, wback, 7 + wb2 * 8, wb2 * 8);
                    ed = x4;
                } else {
                    ed = wback;
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, &lock, DS_DISP, 0);
                SMREADLOCK(lock);
                LBZ(x4, fixedaddress, wback);
                ed = x4;
            }
            BF_INSERT(gb1, ed, gb2 + 7, gb2);
            break;
        case 0x8B:
            INST_NAME("MOV Gd, Ed");
            nextop = F8;
            GETGD;
            SCRATCH_USAGE(0);
            if (MODREG) {
                MVxw(gd, TO_NAT((nextop & 7) + (rex.b << 3)));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, DS_DISP, 0);
                if(!lock && BOX64ENV(unity) && !VolatileRangesContains(ip) && ((fixedaddress==0x80) || (fixedaddress==0x84) || (fixedaddress==0xc0) || (fixedaddress==0xc4))) {
                    lock = 1;
                }
                SMREADLOCK(lock);
                LDxw(gd, ed, fixedaddress);
            }
            break;
        case 0x8D:
            INST_NAME("LEA Gd, Ed");
            nextop = F8;
            GETGD;
            if (MODREG) {
                INST_NAME("Invalid 8D");
                UDF();
                *need_epilog = 1;
                *ok = 0;
            } else {         // mem <= reg
                rex.seg = 0; // to be safe
                SCRATCH_USAGE(0);
                addr = geted(dyn, addr, ninst, nextop, &ed, gd, x1, &fixedaddress, rex, NULL, NO_DISP, 0);
                if (gd != ed) {
                    if (rex.w && rex.is67)
                        ZEROUP2(gd, ed);
                    else
                        MVxw(gd, ed);
                } else if (!rex.w && !rex.is32bits) {
                    ZEROUP(gd); // truncate the higher 32bits as asked
                }
            }
            break;
        case 0x90:
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x96:
        case 0x97:
            gd = TO_NAT((opcode & 0x07) + (rex.b << 3));
            if (gd == xRAX) {
                INST_NAME("NOP");
            } else {
                INST_NAME("XCHG EAX, Reg");
                MVxw(x2, xRAX);
                MVxw(xRAX, gd);
                MVxw(gd, x2);
            }
            break;
        case 0xA8:
            INST_NAME("TEST AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            ANDI(x1, xRAX, 0xff);
            u8 = F8;
            MOV32w(x2, u8);
            emit_test8(dyn, ninst, x1, x2, x3, x4, x5);
            break;
        case 0xA9:
            INST_NAME("TEST EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            MOV64xw(x2, i64);
            emit_test32(dyn, ninst, rex, xRAX, x2, x3, x4, x5);
            break;
        case 0xB0:
        case 0xB1:
        case 0xB2:
        case 0xB3:
            INST_NAME("MOV xL, Ib");
            u8 = F8;
            if (rex.rex)
                gb1 = TO_NAT((opcode & 7) + (rex.b << 3));
            else
                gb1 = TO_NAT(opcode & 3);
            BF_INSERT(gb1, xZR, 7, 0);
            ORI(gb1, gb1, u8);
            break;
        case 0xB4:
        case 0xB5:
        case 0xB6:
        case 0xB7:
            INST_NAME("MOV xH, Ib");
            u8 = F8;
            MOV32w(x1, u8);
            if (rex.rex) {
                gb1 = TO_NAT((opcode & 7) + (rex.b << 3));
                BF_INSERT(gb1, x1, 7, 0);
            } else {
                gb1 = TO_NAT(opcode & 3);
                BF_INSERT(gb1, x1, 15, 8);
            }
            break;
        case 0xB8:
        case 0xB9:
        case 0xBA:
        case 0xBB:
        case 0xBC:
        case 0xBD:
        case 0xBE:
        case 0xBF:
            INST_NAME("MOV Reg, Id");
            gd = TO_NAT((opcode & 7) + (rex.b << 3));
            SCRATCH_USAGE(0);
            if (rex.w) {
                u64 = F64;
                MOV64x(gd, u64);
            } else {
                u32 = F32;
                MOV32w(gd, u32);
            }
            break;
        case 0x98:
            if (rex.w) {
                INST_NAME("CDQE");
                EXTSW(xRAX, xRAX);
            } else {
                INST_NAME("CWDE");
                EXTSH(xRAX, xRAX);
                ZEROUP(xRAX);
            }
            break;
        case 0x99:
            INST_NAME("CDQ");
            if (rex.w) {
                SRADI(xRDX, xRAX, 63);
            } else {
                SRAWI(xRDX, xRAX, 31);
                ZEROUP(xRDX);
            }
            break;
        case 0xC0:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Eb, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & 0x1f;
                    if (u8) {
                        SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                        GETEB(x1, 1);
                        u8 = F8 & 0x1f;
                        emit_rol8c(dyn, ninst, ed, u8, x4, x5, x6);
                        EBBACK();
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 1:
                    INST_NAME("ROR Eb, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & 0x1f;
                    if (u8) {
                        SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                        GETEB(x1, 1);
                        u8 = F8 & 0x1f;
                        emit_ror8c(dyn, ninst, x1, u8, x4, x5);
                        EBBACK();
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 2:
                    INST_NAME("RCL Eb, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & 0x1f;
                    if (u8) {
                        READFLAGS(X_CF);
                        SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                        GETEB(x1, 1);
                        u8 = F8 & 0x1f;
                        emit_rcl8c(dyn, ninst, x1, u8, x4, x5);
                        EBBACK();
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 3:
                    INST_NAME("RCR Eb, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & 0x1f;
                    if (u8) {
                        READFLAGS(X_CF);
                        SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                        GETEB(x1, 1);
                        u8 = F8 & 0x1f;
                        emit_rcr8c(dyn, ninst, x1, u8, x4, x5);
                        EBBACK();
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Eb, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & 0x1f;
                    if (u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        GETEB(x1, 1);
                        u8 = (F8) & 0x1f;
                        emit_shl8c(dyn, ninst, ed, u8, x4, x5);
                        EBBACK();
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 5:
                    INST_NAME("SHR Eb, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & 0x1f;
                    if (u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        GETEB(x1, 1);
                        u8 = (F8) & 0x1f;
                        emit_shr8c(dyn, ninst, ed, u8, x4, x5);
                        EBBACK();
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 7:
                    INST_NAME("SAR Eb, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & 0x1f;
                    if (u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        GETSEB(x1, 1);
                        u8 = (F8) & 0x1f;
                        emit_sar8c(dyn, ninst, ed, u8, x4, x5);
                        EBBACK();
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xC1:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (0x1f + (rex.w * 0x20));
                    if (u8) {
                        SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                        GETED(1);
                        u8 = (F8) & (rex.w ? 0x3f : 0x1f);
                        emit_rol32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w && !rex.is32bits) {
                            GETED(1);
                            ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 1:
                    INST_NAME("ROR Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (0x1f + (rex.w * 0x20));
                    if (u8) {
                        SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                        GETED(1);
                        u8 = (F8) & (rex.w ? 0x3f : 0x1f);
                        emit_ror32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w && !rex.is32bits) {
                            GETED(1);
                            ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 2:
                    INST_NAME("RCL Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (0x1f + (rex.w * 0x20));
                    if (u8) {
                        READFLAGS(X_CF);
                        SETFLAGS(X_CF | X_OF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                        GETED(1);
                        u8 = (F8) & (rex.w ? 0x3f : 0x1f);
                        emit_rcl32c(dyn, ninst, rex, ed, u8, x3, x4, x5);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w && !rex.is32bits) {
                            GETED(1);
                            ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 3:
                    INST_NAME("RCR Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (0x1f + (rex.w * 0x20));
                    if (u8) {
                        READFLAGS(X_CF);
                        SETFLAGS(X_CF | X_OF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                        GETED(1);
                        u8 = (F8) & (rex.w ? 0x3f : 0x1f);
                        emit_rcr32c(dyn, ninst, rex, ed, u8, x3, x4, x5);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w && !rex.is32bits) {
                            GETED(1);
                            ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (0x1f + (rex.w * 0x20));
                    if (u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        GETED(1);
                        u8 = (F8) & (rex.w ? 0x3f : 0x1f);
                        emit_shl32c(dyn, ninst, rex, ed, u8, x3, x4, x5);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w && !rex.is32bits) {
                            GETED(1);
                            ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 5:
                    INST_NAME("SHR Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (0x1f + (rex.w * 0x20));
                    if (u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        GETED(1);
                        u8 = (F8) & (rex.w ? 0x3f : 0x1f);
                        emit_shr32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w && !rex.is32bits) {
                            GETED(1);
                            ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 7:
                    INST_NAME("SAR Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (0x1f + (rex.w * 0x20));
                    if (u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                        GETED(1);
                        u8 = (F8) & (rex.w ? 0x3f : 0x1f);
                        emit_sar32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w && !rex.is32bits) {
                            GETED(1);
                            ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xC2:
            INST_NAME("RETN");
            if (BOX64DRENV(dynarec_safeflags)) {
                READFLAGS(X_PEND); // lets play safe here too
            }
            BARRIER(BARRIER_FLOAT);
            u16 = F16;
            POP1z(xRIP);
            if (u16 < 2048) {
                ADDIz(xRSP, xRSP, u16);
            } else {
                MOV32w(x1, u16);
                ADDz(xRSP, xRSP, x1);
            }
            ret_to_next(dyn, ip, ninst, rex);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xC3:
            INST_NAME("RET");
            if (BOX64DRENV(dynarec_safeflags)) {
                READFLAGS(X_PEND); // so instead, force the deferred flags, so it's not too slow, and flags are not lost
            }
            BARRIER(BARRIER_FLOAT);
            POP1z(xRIP);
            ret_to_next(dyn, ip, ninst, rex);
            *need_epilog = 0;
            *ok = 0;
            break;

        case 0xC6:
            INST_NAME("MOV Eb, Ib");
            nextop = F8;
            if (MODREG) { // reg <= u8
                u8 = F8;
                if (!rex.rex) {
                    ed = (nextop & 7);
                    eb1 = TO_NAT((ed & 3)); // Ax, Cx, Dx or Bx
                    eb2 = (ed & 4) << 1;    // L or H
                } else {
                    eb1 = TO_NAT((nextop & 7) + (rex.b << 3));
                    eb2 = 0;
                }
                MOV32w(x3, u8);
                BF_INSERT(eb1, x3, eb2 + 7, eb2);
            } else { // mem <= u8
                SCRATCH_USAGE(1);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, &lock, DS_DISP, 1);
                u8 = F8;
                LI(x3, u8);
                ed = x3;
                STB(ed, fixedaddress, wback);
                SMWRITELOCK(lock);
            }
            break;
        case 0xC7:
            INST_NAME("MOV Ed, Id");
            nextop = F8;
            if (MODREG) { // reg <= i32
                i64 = F32S;
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                MOV64xw(ed, i64);
            } else { // mem <= i32
                SCRATCH_USAGE(1);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, &lock, DS_DISP, 4);
                i64 = F32S;
                MOV64xw(x3, i64);
                ed = x3;
                SDxw(ed, wback, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0xC9:
            INST_NAME("LEAVE");
            MVz(xRSP, xRBP);
            POP1z(xRBP);
            break;
        case 0xE8:
            INST_NAME("CALL Id");
            i32 = F32S;
            if (addr + i32 == 0) {
#if STEP == 3
                printf_log(LOG_INFO, "Warning, CALL to 0x0 at %p (%p)\n", (void*)addr, (void*)(addr - 1));
#endif
            }
#if STEP < 2
            if (!rex.is32bits && !dyn->need_reloc && IsNativeCall(addr + i32, rex.is32bits, &dyn->insts[ninst].natcall, &dyn->insts[ninst].retn))
                tmp = dyn->insts[ninst].pass2choice = 3;
            else
                tmp = dyn->insts[ninst].pass2choice = i32 ? 0 : 1;
#else
            tmp = dyn->insts[ninst].pass2choice;
#endif
            switch (tmp) {
                case 3:
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags to "dont'care" state
                    SKIPTEST(x1);
                    BARRIER(BARRIER_FULL);
                    if (dyn->last_ip && ((addr - dyn->last_ip < 0x800) || (dyn->last_ip - addr < 0x800))) {
                        ADDI(x2, xRIP, addr - dyn->last_ip);
                    } else {
                        if (dyn->need_reloc) {
                            TABLE64(x2, addr);
                        } else {
                            MOV64x(x2, addr);
                        }
                    }
                    PUSH1(x2);
                    MESSAGE(LOG_DUMP, "Native Call to %s (retn=%d)\n", GetNativeName(GetNativeFnc(dyn->insts[ninst].natcall - 1), 1), dyn->insts[ninst].retn);
                    // calling a native function
                    sse_purge07cache(dyn, ninst, x3);
                    if ((BOX64ENV(log) < 2 && !BOX64ENV(rolling_log)) && dyn->insts[ninst].natcall) {
                        // Partially support isSimpleWrapper
                        tmp = isSimpleWrapper(*(wrapper_t*)(dyn->insts[ninst].natcall + 2));
                    } else
                        tmp = 0;
                    if (tmp < 0 || (tmp & 15) > 1)
                        tmp = 0; // TODO: removed when FP is in place
                    if (dyn->insts[ninst].natcall && isRetX87Wrapper(*(wrapper_t*)(dyn->insts[ninst].natcall + 2)))
                        // return value will be on the stack, so the stack depth needs to be updated
                        x87_purgecache(dyn, ninst, 0, x3, x1, x4);
                    if ((BOX64ENV(log) < 2 && !BOX64ENV(rolling_log)) && dyn->insts[ninst].natcall && tmp) {
                        call_n(dyn, ninst, (void*)(dyn->insts[ninst].natcall + 2 + 8), tmp);
                        SMWRITE2();
                        POP1(xRIP); // pop the return address
                        dyn->last_ip = addr;
                    } else {
                        GETIP_(dyn->insts[ninst].natcall, x7); // read the 0xCC already
                        STORE_XEMU_CALL();
                        ADDI(x1, xEmu, (uint32_t)offsetof(x64emu_t, ip)); // setup addr as &emu->ip
                        CALL_S(const_int3, -1, x1);
                        SMWRITE2();
                        LOAD_XEMU_CALL();
                        MOV64x(x3, dyn->insts[ninst].natcall);
                        ADDI(x3, x3, 2 + 8 + 8);
                        BNE_MARK(xRIP, x3); // Not the expected address, exit dynarec block
                        POP1(xRIP);         // pop the return address
                        if (dyn->insts[ninst].retn) {
                            if (dyn->insts[ninst].retn < 0x800) {
                                ADDI(xRSP, xRSP, dyn->insts[ninst].retn);
                            } else {
                                MOV64x(x3, dyn->insts[ninst].retn);
                                ADD(xRSP, xRSP, x3);
                            }
                        }
                        LWZ(x1, offsetof(x64emu_t, quit), xEmu);
                        CBZ_NEXT(x1);
                        MARK;
                        LOAD_XEMU_REM();
                        jump_to_epilog_fast(dyn, 0, xRIP, ninst);
                        dyn->last_ip = addr;
                    }
                    break;
                case 1:
                    // this is call to next step, so just push the return address to the stack
                    if (dyn->need_reloc) {
                        TABLE64(x2, addr);
                    } else {
                        MOV64x(x2, addr);
                    }
                    PUSH1z(x2);
                    break;
                default:
                    if ((BOX64DRENV(dynarec_safeflags) > 1) || (ninst && dyn->insts[ninst - 1].x64.set_flags)) {
                        READFLAGS(X_PEND); // that's suspicious
                    } else {
                        SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags to "dont'care" state
                    }
                    // regular call
                    if (dyn->need_reloc) {
                        TABLE64(x2, addr);
                    } else {
                        MOV64x(x2, addr);
                    }
                    BARRIER(BARRIER_FLOAT);
                    PUSH1z(x2);
                    if (BOX64DRENV(dynarec_callret)) {
                        SET_HASCALLRET();
                        // Push shadow return address {native_addr, x86_addr}
                        if (addr < (dyn->start + dyn->isize)) {
                            // return address is within this block
                            j64 = (dyn->insts) ? (dyn->insts[ninst].epilog - (dyn->native_size)) : 0;
                            BCL(20, 31, 4);        // LR = addr of next instruction
                            MFLR(x4);              // x4 = LR
                            ADDI(x4, x4, j64 - 4); // x4 = native epilog addr
                            MESSAGE(LOG_NONE, "\tCALLRET set return to +%di\n", j64 >> 2);
                        } else {
                            // return address is outside this block — point to MARK landing pad
                            j64 = (dyn->insts) ? (GETMARK - (dyn->native_size)) : 0;
                            BCL(20, 31, 4);        // LR = addr of next instruction
                            MFLR(x4);              // x4 = LR
                            ADDI(x4, x4, j64 - 4); // x4 = native MARK addr
                            MESSAGE(LOG_NONE, "\tCALLRET set return to +%di\n", j64 >> 2);
                        }
                        ADDI(xSP, xSP, -16);
                        STD(x4, 0, xSP); // native return addr
                        STD(x2, 8, xSP); // x86 return addr
                    } else {
                        *ok = 0;
                        *need_epilog = 0;
                    }
                    if (rex.is32bits)
                        j64 = (uint32_t)(addr + i32);
                    else
                        j64 = addr + i32;
                    jump_to_next(dyn, j64, 0, ninst, rex.is32bits);
                    if (BOX64DRENV(dynarec_callret) && addr >= (dyn->start + dyn->isize)) {
                        // return address is outside this block — emit landing pad
                        MARK;
                        j64 = getJumpTableAddress64(addr);
                        if (dyn->need_reloc) {
                            AddRelocTable64JmpTbl(dyn, ninst, addr, STEP);
                            TABLE64_(x4, j64);
                        } else {
                            MOV64x(x4, j64);
                        }
                        LD(x4, 0, x4);
                        MTCTR(x4);
                        BCTR();
                    }
                    break;
            }
            break;
        case 0xE9:
        case 0xEB:
            BARRIER(BARRIER_MAYBE);
            if (opcode == 0xE9) {
                INST_NAME("JMP Id");
                i32 = F32S;
            } else {
                INST_NAME("JMP Ib");
                i32 = F8S;
            }
            if (rex.is32bits)
                j64 = (uint32_t)(addr + i32);
            else
                j64 = addr + i32;
            JUMP((uintptr_t)getAlternate((void*)j64), 0);
            if (dyn->insts[ninst].x64.jmp_insts == -1) {
                // out of the block
                BARRIER(BARRIER_FLOAT);
                jump_to_next(dyn, (uintptr_t)getAlternate((void*)j64), 0, ninst, rex.is32bits);
            } else {
                // inside the block
                CacheTransform(dyn, ninst, CHECK_CACHE(), x1, x2, x3);
                tmp = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size);
                MESSAGE(1, "Jump to %d / 0x%x\n", tmp, tmp);
                if (tmp == 4) {
                    NOP();
                } else {
                    B(tmp);
                }
            }
            *need_epilog = 0;
            *ok = 0;
            break;

        case 0xD0:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Eb, 1");
                    GETEB(x1, 0);
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    emit_rol8c(dyn, ninst, ed, 1, x4, x5, x6);
                    EBBACK();
                    break;
                case 1:
                    INST_NAME("ROR Eb, 1");
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    GETEB(x1, 0);
                    emit_ror8c(dyn, ninst, ed, 1, x4, x5);
                    EBBACK();
                    break;
                case 2:
                    INST_NAME("RCL Eb, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    GETEB(x1, 0);
                    emit_rcl8c(dyn, ninst, ed, 1, x4, x5);
                    EBBACK();
                    break;
                case 3:
                    INST_NAME("RCR Eb, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    GETEB(x1, 0);
                    emit_rcr8c(dyn, ninst, ed, 1, x4, x5);
                    EBBACK();
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Eb, 1");
                    GETEB(x1, 0);
                    MOV32w(x2, 1);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    emit_shl8(dyn, ninst, x1, x2, x5, x4, x6);
                    EBBACK();
                    break;
                case 5:
                    INST_NAME("SHR Eb, 1");
                    GETEB(x1, 0);
                    MOV32w(x2, 1);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    emit_shr8(dyn, ninst, x1, x2, x5, x4, x6);
                    EBBACK();
                    break;
                case 7:
                    INST_NAME("SAR Eb, 1");
                    GETSEB(x1, 0);
                    MOV32w(x2, 1);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    emit_sar8(dyn, ninst, x1, x2, x5, x4, x6);
                    EBBACK();
                    break;
            }
            break;
        case 0xD1:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Ed, 1");
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    GETED(0);
                    emit_rol32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                case 1:
                    INST_NAME("ROR Ed, 1");
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    GETED(0);
                    emit_ror32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                case 2:
                    INST_NAME("RCL Ed, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    GETED(0);
                    emit_rcl32c(dyn, ninst, rex, ed, 1, x3, x4, x5);
                    WBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ed, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    GETED(0);
                    emit_rcr32c(dyn, ninst, rex, ed, 1, x3, x4, x5);
                    WBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    GETED(0);
                    emit_shl32c(dyn, ninst, rex, ed, 1, x3, x4, x5);
                    WBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    GETED(0);
                    emit_shr32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    GETED(0);
                    emit_sar32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
            }
            break;
        case 0xD2:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Eb, CL");
                    GETEB(x1, 0);
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_OF | X_CF);
                    }
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    UFLAG_IF {
                        ANDI(x2, xRCX, 0x1f);
                        CBZ_NEXT(x2);
                    }
                    ANDI(x2, xRCX, 7);
                    emit_rol8(dyn, ninst, ed, x2, x4, x5);
                    EBBACK();
                    break;
                case 1:
                    INST_NAME("ROR Eb, CL");
                    GETEB(x1, 0);
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_OF | X_CF);
                    }
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    UFLAG_IF {
                        ANDI(x2, xRCX, 0x1f);
                        CBZ_NEXT(x2);
                    }
                    ANDI(x2, xRCX, 7);
                    emit_ror8(dyn, ninst, ed, x2, x4, x5);
                    EBBACK();
                    break;
                case 2:
                    INST_NAME("RCL Eb, CL");
                    GETEB(x1, 0);
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_OF | X_CF);
                    } else {
                        READFLAGS(X_CF);
                    }
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    ANDI(x2, xRCX, 0x1f);
                    CBZ_NEXT(x2);
                    emit_rcl8(dyn, ninst, ed, x2, x4, x5, x6);
                    EBBACK();
                    break;
                case 3:
                    INST_NAME("RCR Eb, CL");
                    GETEB(x1, 0);
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_OF | X_CF);
                    } else {
                        READFLAGS(X_CF);
                    }
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    ANDI(x2, xRCX, 0x1f);
                    CBZ_NEXT(x2);
                    emit_rcr8(dyn, ninst, ed, x2, x4, x5, x6);
                    EBBACK();
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Eb, CL");
                    GETEB(x1, 0);
                    ANDI(x2, xRCX, 0x1F);
                    CBZ_NEXT(x2);
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_FUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    emit_shl8(dyn, ninst, x1, x2, x5, x4, x6);
                    EBBACK();
                    break;
                case 5:
                    INST_NAME("SHR Eb, CL");
                    GETEB(x1, 0);
                    ANDI(x2, xRCX, 0x1F);
                    CBZ_NEXT(x2);
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_FUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    emit_shr8(dyn, ninst, x1, x2, x5, x4, x6);
                    EBBACK();
                    break;
                case 7:
                    INST_NAME("SAR Eb, CL");
                    GETSEB(x1, 0);
                    ANDI(x2, xRCX, 0x1f);
                    CBZ_NEXT(x2);
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_FUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    emit_sar8(dyn, ninst, x1, x2, x5, x4, x6);
                    EBBACK();
                    break;
            }
            break;
        case 0xD3:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Ed, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_OF | X_CF);
                    }
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    GETED(0);
                    ANDI(x6, xRCX, rex.w ? 0x3f : 0x1f);
                    if (!rex.w && !rex.is32bits && MODREG) ZEROUP(ed);
                    CBZ_NEXT(x6);
                    emit_rol32(dyn, ninst, rex, ed, x6, x3, x4);
                    WBACK;
                    break;
                case 1:
                    INST_NAME("ROR Ed, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_OF | X_CF);
                    }
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    GETED(0);
                    ANDI(x6, xRCX, rex.w ? 0x3f : 0x1f);
                    if (!rex.w && !rex.is32bits && MODREG) ZEROUP(ed);
                    CBZ_NEXT(x6);
                    emit_ror32(dyn, ninst, rex, ed, x6, x3, x4);
                    WBACK;
                    break;
                case 2:
                    INST_NAME("RCL Ed, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_OF | X_CF);
                    } else {
                        READFLAGS(X_CF);
                    }
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    ANDI(x3, xRCX, rex.w ? 63 : 31);
                    GETED(0);
                    UFLAG_IF {
                        if (!rex.w && !rex.is32bits && MODREG) { ZEROUP(ed); }
                    }
                    CBZ_NEXT(x3);
                    emit_rcl32(dyn, ninst, rex, ed, x3, x5, x4, x6);
                    WBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ed, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_OF | X_CF);
                    } else {
                        READFLAGS(X_CF);
                    }
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    if (rex.w) {
                        ANDI(x3, xRCX, 0x3f);
                    } else {
                        ANDI(x3, xRCX, 0x1f);
                    }
                    GETED(0);
                    UFLAG_IF {
                        if (!rex.w && !rex.is32bits && MODREG) { ZEROUP(ed); }
                    }
                    CBZ_NEXT(x3);
                    emit_rcr32(dyn, ninst, rex, ed, x3, x5, x4, x6);
                    WBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_FUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    if (!dyn->insts[ninst].x64.gen_flags) {
                        GETED(0);
                        // x86 SHL masks shift count: 0x3F for 64-bit, 0x1F for 32-bit
                        // PPC64LE sld/slw zero the result when shift >= 64/32, so we must mask first
                        ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
                        if (rex.w)
                            SLD(ed, ed, x3);
                        else
                            SLW(ed, ed, x3);
                        if (dyn->insts[ninst].nat_flags_fusion) {
                            if (!rex.w) ZEROUP(ed);
                            NAT_FLAGS_OPS(ed, xZR, x5, xZR);
                        } else if (!rex.w && MODREG) {
                            ZEROUP(ed);
                        }
                        WBACK;
                        break;
                    }
                    ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
                    GETED(0);
                    if (!rex.w && !rex.is32bits && MODREG) ZEROUP(ed);
                    CBZ_NEXT(x3);
                    emit_shl32(dyn, ninst, rex, ed, x3, x5, x4, x6);
                    WBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ed, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_FUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    if (!dyn->insts[ninst].x64.gen_flags) {
                        GETED(0);
                        // x86 SHR masks shift count: 0x3F for 64-bit, 0x1F for 32-bit
                        // PPC64LE srd/srw zero the result when shift >= 64/32, so we must mask first
                        ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
                        if (rex.w)
                            SRD(ed, ed, x3);
                        else
                            SRW(ed, ed, x3);
                        if (dyn->insts[ninst].nat_flags_fusion) {
                            if (!rex.w) ZEROUP(ed);
                            NAT_FLAGS_OPS(ed, xZR, x5, xZR);
                        } else if (!rex.w && MODREG) {
                            ZEROUP(ed);
                        }
                        WBACK;
                        break;
                    }
                    ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
                    GETED(0);
                    if (!rex.w && !rex.is32bits && MODREG) ZEROUP(ed);
                    CBZ_NEXT(x3);
                    emit_shr32(dyn, ninst, rex, ed, x3, x5, x4);
                    WBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ed, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_FUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
                    GETED(0);
                    if (!rex.w && !rex.is32bits && MODREG) { ZEROUP(ed); }
                    CBZ_NEXT(x3);
                    emit_sar32(dyn, ninst, rex, ed, x3, x5, x4);
                    WBACK;
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
