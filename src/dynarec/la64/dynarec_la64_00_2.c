#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "la64_mapping.h"
#include "x64_signals.h"
#include "os.h"
#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "la64_emitter.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "bridge.h"
#include "emu/x64run_private.h"
#include "../dynablock_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "custommem.h"
#include "alternate.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_00_2(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    uint8_t nextop, opcode;
    uint8_t gd, ed, tmp1, tmp2, tmp3;
    int8_t i8;
    int32_t i32, tmp;
    int64_t i64, j64;
    uint8_t u8;
    uint8_t gb1, gb2, eb1, eb2;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    uint8_t wback, wb1, wb2, wb;
    int64_t fixedaddress;
    uint8_t v0, v1, v2;
    int unscaled;
    int lock;
    int cacheupd = 0;

    opcode = F8;
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(j64);
    MAYUSE(wb);
    MAYUSE(lock);
    MAYUSE(cacheupd);

    switch (opcode) {
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
                        ADDI_D(x2, xZR, u8);
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
                    GETEDsd((opcode == 0x81) ? 4 : 1);
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
                    GETEDsd((opcode == 0x81) ? 4 : 1);
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
                    GETEDsd((opcode == 0x81) ? 4 : 1);
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
                    GETEDsd((opcode == 0x81) ? 4 : 1);
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
                    if (MODREG && !dyn->insts[ninst].x64.gen_flags && !dyn->insts[ninst].nat_flags_fusion && !rex.w) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        if (i64 >= 0 && i64 <= 4095) {
                            ANDI(ed, ed, i64);
                            UP32_WRITE64(ed);
                        } else {
                            la64_move32(dyn, ninst, x3, (int32_t)i64, 0);
                            AND(ed, ed, x3);
                            UP32_WRITE32(ed);
                            if (NEED_ZEROUP32(ed)) ZEROUP_RESULT(ed);
                        }
                        break;
                    }
                    GETEDsd((opcode == 0x81) ? 4 : 1);
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
                    GETEDsd((opcode == 0x81) ? 4 : 1);
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
                    GETEDsd((opcode == 0x81) ? 4 : 1);
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
            GETGDs;
            GETED(0);
            emit_test32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            break;
        case 0x86:
            INST_NAME("(LOCK) XCHG Eb, Gb");
            nextop = F8;
            if (MODREG) {
                GETGB(x1);
                GETEB(x2, 0);
                BSTRINS_D(wback, gd, wb2 + 7, wb2);
                BSTRINS_D(gb1, ed, gb2 + 7, gb2);
            } else {
                GETGB(x3);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                if (cpuext.lam_bh) {
                    AMSWAP_DB_B(x1, gd, ed);
                    BSTRINS_D(gb1, x1, gb2 + 7, gb2);
                } else {

                    // calculate shift amount
                    ANDI(x1, ed, 0x3);
                    SLLI_D(x1, x1, 3);

                    // align address to 4-bytes to use ll.w/sc.w
                    ADDI_D(x4, xZR, 0xffc);
                    AND(x6, ed, x4);

                    // prepare mask
                    ADDI_D(x4, xZR, 0xff);
                    SLL_D(x4, x4, x1);
                    NOR(x4, x4, xZR);

                    SLL_D(x7, gd, x1);

                    // do aligned ll/sc sequence, reusing x2 (ed might be x2 but is no longer needed)
                    MARKLOCK;
                    LL_W(x2, x6, 0);
                    AND(x5, x2, x4);
                    OR(x5, x5, x7);
                    SC_W(x5, x6, 0);
                    BEQZ_MARKLOCK(x5);

                    // extract loaded byte
                    SRL_D(gd, x2, x1);
                    BSTRINS_D(gb1, gd, gb2 + 7, gb2);
                }
            }
            break;
        case 0x87:
            INST_NAME("(LOCK) XCHG Ed, Gd");
            nextop = F8;
            if (MODREG) {
                GETGDsd;
                GETEDsd(0);
                MVxw(x1, gd);
                MVxw(gd, ed);
                MVxw(ed, x1);
            } else {
                GETGDsd;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                if (rex.w) {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b111);
                        BNEZ_MARK2(x3);
                    }
                    /* LoongArch Reference Manual Vol1 2.2.7.1
                        If the AM* atomic memory access instruction has the same register number as rd and rk,
                        the execution result is uncertain. Please software to avoid this situation.
                    */
                    AMSWAP_DB_D(x1, gd, wback);
                    if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                } else {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b11);
                        BNEZ_MARK(x3);
                    }
                    // aligned 4byte
                    AMSWAP_DB_W(x1, gd, wback);
                    if (!ALIGNED_ATOMICxw) {
                        B_MARK3_nocond;
                        MARK;
                        ANDI(x3, wback, 0b111);
                        SLTI(x4, x3, 4);
                        BEQZ_MARK2(x4); // addr %8 >4 , cross 8bytes or cross cacheline
                        LOCK_32_IN_8BYTE(ADDI_W(x4, gd, 0), x1, wback, x3, x4, x5, x6);
                        B_MARK3_nocond;
                    }
                }
                if (!ALIGNED_ATOMICxw) {
                    MARK2;
                    LOCK_3264_CROSS_8BYTE(ADDI_D(x4, gd, 0), x1, wback, x4, x5, x6);
                    MARK3;
                }
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
                BSTRPICK_D(gd, gb1, gb2 + 7, gb2);
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
                    eb2 = ((ed & 4) >> 2); // L or H
                }
                BSTRINS_D(eb1, gd, eb2 * 8 + 7, eb2 * 8);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                ST_B(gd, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0x89:
            INST_NAME("MOV Ed, Gd");
            nextop = F8;
            GETGDs;
            SCRATCH_USAGE(0);
            if (MODREG) { // reg <= reg
                GETEDsd(0);
                if (!rex.w) UP32_ZERO(ed);
                MVxw(ed, gd);
            } else { // mem <= reg
                IF_UNALIGNED(ip) {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, (1 << (2 + rex.w)) - 1, 0);
                    for (int i = 0; i < (1 << (2 + rex.w)); i++) {
                        if (i == 0) {
                            ST_B(gd, ed, fixedaddress);
                        } else {
                            SRLI_D(x3, gd, i * 8);
                            ST_B(x3, ed, fixedaddress + i);
                        }
                    }
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                    if(!lock && BOX64ENV(unity) && !VolatileRangesContains(ip) && ((fixedaddress==0x80) || (fixedaddress==0x84) || (fixedaddress==0xc0) || (fixedaddress==0xc4))) {
                        DMB_ISH();
                        lock = 1;
                    }
                    if (rex.w) {
                        ST_D(gd, ed, fixedaddress);
                    } else {
                        ST_W(gd, ed, fixedaddress);
                    }
                }
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
                    BSTRPICK_D(x4, wback, 7 + wb2 * 8, wb2 * 8);
                    ed = x4;
                } else {
                    ed = wback;
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                SMREADLOCK(lock);
                LD_BU(x4, wback, fixedaddress);
                ed = x4;
            }
            BSTRINS_D(gb1, ed, gb2 + 7, gb2);
            break;
        case 0x8B:
            INST_NAME("MOV Gd, Ed");
            nextop = F8;
            GETGDd;
            if (!rex.w) UP32_ZERO(gd);
            SCRATCH_USAGE(0);
            if (MODREG) {
                GETED(0);
                MVxw(gd, ed);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                if(!lock && BOX64ENV(unity) && !VolatileRangesContains(ip) && ((fixedaddress==0x80) || (fixedaddress==0x84) || (fixedaddress==0xc0) || (fixedaddress==0xc4))) {
                    lock = 1;
                }
                SMREADLOCK(lock);
                LDxw(gd, ed, fixedaddress);
            }
            break;
        case 0x8C:
            nextop = F8;
            u8 = (nextop & 0x38) >> 3;
            if (u8 > 5) {
                INST_NAME("Invalid MOV Ed, Seg");
                UDF();
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("MOV Ed, Seg");
                if (MODREG) {
                    SCRATCH_USAGE(0);
                    gd = TO_NAT((nextop & 7) + (rex.b << 3));
                    LD_HU(gd, xEmu, offsetof(x64emu_t, segs[u8]));
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    LD_HU(x3, xEmu, offsetof(x64emu_t, segs[u8]));
                    ST_H(x3, ed, fixedaddress);
                    SMWRITE2();
                }
            }
            break;
        case 0x8D:
            INST_NAME("LEA Gd, Ed");
            nextop = F8;
            GETGDd;
            if (MODREG) {
                INST_NAME("Invalid 8D");
                UDF();
                *need_epilog = 1;
                *ok = 0;
            } else {         // mem <= reg
                rex.seg = 0; // to be safe
                SCRATCH_USAGE(0);
                addr = geted(dyn, addr, ninst, nextop, &ed, gd, x1, &fixedaddress, rex, NULL, 0, 0);
                if (gd != ed) {
                    if (rex.w && rex.is67) {
                        ZEROUP2(gd, ed);
                        UP32_ZERO(gd);
                    } else {
                        MVxw(gd, ed);
                        UP32_ZERO(gd);
                    }
                } else if (!rex.w && !rex.is32bits) {
                    if (NEED_ZEROUP(gd)) ZEROUP_RESULT(gd); // truncate the higher 32bits as asked
                }
            }
            break;
        case 0x8E:
            nextop = F8;
            u8 = (nextop & 0x38) >> 3;
            if ((u8 > 5) || (u8 == 1)) {
                INST_NAME("Invalid MOV Seg, Ed");
                UDF();
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("MOV Seg, Ed");
                if (MODREG) {
                    ed = TO_NAT((nextop & 7) + (rex.b << 3));
                    BSTRPICK_D(x2, ed, 15, 0);
                    ed = x2;
                } else {
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    LD_HU(x2, ed, fixedaddress);
                    ed = x2;
                }
                ST_H(ed, xEmu, offsetof(x64emu_t, segs[u8]));
                if ((u8 == _FS) || (u8 == _GS)) {
                    // refresh offset if needed
                    CBZ_NEXT(ed);
                    MOV32w(x1, u8);
                    CALL(const_getsegmentbase, -1, x1, x2);
                }
            }
            break;
        case 0x8F:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("POP Ed");
                    SMREAD();
                    MARKREGsdz(xRSP);
                    if (MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        MARKREGdz(ed);
                        POP1z(ed);
                    } else {
                        POP1z(x2); // so this can handle POP [ESP] and maybe some variant too
                        addr = geted(dyn, addr, ninst, nextop, &ed, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                        if (ed == xRSP) {
                            SDz(x2, ed, fixedaddress);
                        } else {
                            // complicated to just allow a segfault that can be recovered correctly
                            ADDIz(xRSP, xRSP, rex.is32bits ? -4 : -8);
                            SDz(x2, ed, fixedaddress);
                            ADDIz(xRSP, xRSP, rex.is32bits ? 4 : 8);
                        }
                        SMWRITE();
                    }
                    break;
                case 3:
                    INST_NAME("Invalid 8F /3");
                    UDF();
                    *need_epilog = 1;
                    *ok = 0;
                    break;
                default: DEFAULT;
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
                MARKREGsd(gd);
                MARKREGsd(xRAX);
                MVxw(x2, xRAX);
                MVxw(xRAX, gd);
                MVxw(gd, x2);
            }
            break;
        case 0x98:
            if (rex.w) {
                INST_NAME("CDQE");
                MARKREGd(xRAX);
                SEXT_W(xRAX, xRAX);
            } else {
                INST_NAME("CWDE");
                MARKREGd(xRAX);
                EXT_W_H(xRAX, xRAX);
                if (NEED_ZEROUP(xRAX)) ZEROUP(xRAX);
            }
            break;
        case 0x99:
            INST_NAME("CDQ");
            MARKREGd(xRDX);
            MARKREGs(xRAX);
            if (rex.w) {
                SRAI_D(xRDX, xRAX, 63);
            } else {
                SRAI_W(xRDX, xRAX, 31);
                if (NEED_ZEROUP(xRDX)) ZEROUP(xRDX);
            }
            break;
        case 0x9A:
            if (rex.is32bits) {
                DEFAULT;
            } else {
                INST_NAME("Illegal 9A");
                if (BOX64DRENV(dynarec_safeflags) > 1) {
                    READFLAGS(X_PEND);
                } else {
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
                }
                GETIP(ip, x7);
                BARRIER(BARRIER_FLOAT);
                UDF();
                *need_epilog = 1;
                *ok = 0;
            }
            break;
        case 0x9B:
            INST_NAME("FWAIT");
            break;
        case 0x9C:
            INST_NAME("PUSHF");
            READFLAGS(X_ALL);
            RESTORE_EFLAGS(x1);
            if (!rex.is32bits) UP32_READ(xRSP);
            PUSH1z(xFlags);
            break;
        case 0x9D:
            INST_NAME("POPF");
            SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
            if (!rex.is32bits) UP32_READ(xRSP);
            POP1z(xFlags);
            MOV32w(x1, 0x3E7FD7);
            AND(xFlags, xFlags, x1);
            ORI(xFlags, xFlags, 0x202);
            SPILL_EFLAGS();
            SET_DFNONE();
            if (box64_wine) { // should this be done all the time?
                ANDI(x1, xFlags, 1 << F_TF);
                CBZ_NEXT(x1);
                // go to epilog, TF should trigger at end of next opcode, so using Interpreter only
                LD_W(x4, xEmu, offsetof(x64emu_t, flags));
                ORI(x4, x4, 1<<FLAGS_NO_TF);
                ST_W(x4, xEmu, offsetof(x64emu_t, flags));
                jump_to_epilog(dyn, addr, 0, ninst);
            }
            break;
        case 0x9E:
            INST_NAME("SAHF");
            SETFLAGS(X_CF | X_PF | X_AF | X_ZF | X_SF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            ADDI_D(x1, xZR, ~0b11010101);
            AND(xFlags, xFlags, x1);
            NOR(x1, x1, x1);
            SRLI_D(x2, xRAX, 8);
            AND(x1, x1, x2);
            OR(xFlags, xFlags, x1);
            SPILL_EFLAGS();
            SET_DFNONE();
            break;
        case 0x9F:
            INST_NAME("LAHF");
            READFLAGS(X_CF | X_PF | X_AF | X_ZF | X_SF);
            RESTORE_EFLAGS(x1);
            BSTRINS_D(xRAX, xFlags, 15, 8);
            break;
        case 0xA0:
            INST_NAME("MOV AL, Ob");
            if (rex.is32bits && rex.is67)
                u64 = F16S;
            else if (rex.is32bits || rex.is67)
                u64 = F32S;
            else
                u64 = F64;
            MOV64y(x1, u64);
            if (rex.seg) {
                grab_segdata(dyn, addr, ninst, x3, rex.seg);
                ADDxREGy(x1, x3, x1, x1);
            }
            lock = (rex.seg) ? 0 : isLockAddress(u64);
            SMREADLOCK(lock);
            LD_BU(x2, x1, 0);
            BSTRINS_D(xRAX, x2, 7, 0);
            break;
        case 0xA1:
            INST_NAME("MOV EAX, Od");
            MARKREGd(xRAX);
            if (rex.is32bits && rex.is67)
                u64 = F16S;
            else if (rex.is32bits || rex.is67)
                u64 = F32S;
            else
                u64 = F64;
            MOV64y(x1, u64);
            if (rex.seg) {
                grab_segdata(dyn, addr, ninst, x3, rex.seg);
                ADDxREGy(x1, x3, x1, x1);
            }
            lock = (rex.seg) ? 0 : isLockAddress(u64);
            SMREADLOCK(lock);
            LDxw(xRAX, x1, 0);
            break;
        case 0xA2:
            INST_NAME("MOV Ob, AL");
            if (rex.is32bits && rex.is67)
                u64 = F16S;
            else if (rex.is32bits || rex.is67)
                u64 = F32S;
            else
                u64 = F64;
            MOV64y(x1, u64);
            if (rex.seg) {
                grab_segdata(dyn, addr, ninst, x3, rex.seg);
                ADDxREGy(x1, x3, x1, x1);
            }
            lock = (rex.seg) ? 0 : isLockAddress(u64);
            ST_B(xRAX, x1, 0);
            SMWRITELOCK(lock);
            break;
        case 0xA3:
            INST_NAME("MOV Od, EAX");
            MARKREGs(xRAX);
            if (rex.is32bits && rex.is67)
                u64 = F16S;
            else if (rex.is32bits || rex.is67)
                u64 = F32S;
            else
                u64 = F64;
            MOV64y(x1, u64);
            if (rex.seg) {
                grab_segdata(dyn, addr, ninst, x3, rex.seg);
                ADDxREGy(x1, x3, x1, x1);
            }
            lock = (rex.seg) ? 0 : isLockAddress(u64);
            SDxw(xRAX, x1, 0);
            SMWRITELOCK(lock);
            break;
        case 0xA4:
            SMREAD();
            UP32_READ(xRSI);
            UP32_READ(xRDI);
            if (rex.rep) {
                UP32_READ(xRCX);
                INST_NAME("REP MOVSB");
                CBZ_NEXT(xRCX);
                if (rex.is67 && !rex.is32bits) {
                    ZEROUP(xRSI);
                    ZEROUP(xRDI);
                }
                ANDI(x1, xFlags, 1 << F_DF);
                BNEZ_MARK2(x1);
                // special optim for large RCX value on forward case only
                if (BOX64DRENV(dynarec_safeflags)) {
                    SUB_D(x1, xRDI, xRSI);
                    ADDI_D(x6, xZR, 8);
                    BLTU_MARK(x1, x6);
                }
                OR(x1, xRSI, xRDI);
                ANDI(x1, x1, 7);
                BNEZ_MARK(x1);
                ADDI_D(x6, xZR, 8);
                MARK3;
                BLT_MARK(xRCX, x6);
                LD_D(x1, xRSI, 0);
                ST_D(x1, xRDI, 0);
                ADDI_D(xRSI, xRSI, 8);
                ADDI_D(xRDI, xRDI, 8);
                ADDI_D(xRCX, xRCX, -8);
                BNEZ_MARK3(xRCX);
                B_NEXT_nocond;
                MARK; // Part with DF==0
                LD_BU(x1, xRSI, 0);
                ST_B(x1, xRDI, 0);
                ADDI_D(xRSI, xRSI, 1);
                ADDI_D(xRDI, xRDI, 1);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK(xRCX);
                B_NEXT_nocond;
                MARK2; // Part with DF==1
                LD_BU(x1, xRSI, 0);
                ST_B(x1, xRDI, 0);
                ADDI_D(xRSI, xRSI, -1);
                ADDI_D(xRDI, xRDI, -1);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK2(xRCX);
                // done
            } else {
                INST_NAME("MOVSB");
                GETDIR(x3, x1, 1);
                if (rex.is67 && !rex.is32bits) {
                    ZEROUP(xRSI);
                    ZEROUP(xRDI);
                }
                LD_BU(x1, xRSI, 0);
                ST_B(x1, xRDI, 0);
                ADD_D(xRSI, xRSI, x3);
                ADD_D(xRDI, xRDI, x3);
            }
            SMWRITE();
            break;
        case 0xA5:
            SMREAD();
            UP32_READ(xRSI);
            UP32_READ(xRDI);
            if (rex.rep) {
                UP32_READ(xRCX);
                INST_NAME("REP MOVSD");
                CBZ_NEXT(xRCX);
                if (rex.is67 && !rex.is32bits) {
                    ZEROUP(xRSI);
                    ZEROUP(xRDI);
                }
                ANDI(x1, xFlags, 1 << F_DF);
                BNEZ_MARK2(x1);
                MARK; // Part with DF==0
                LDxw(x1, xRSI, 0);
                SDxw(x1, xRDI, 0);
                ADDI_D(xRSI, xRSI, rex.w ? 8 : 4);
                ADDI_D(xRDI, xRDI, rex.w ? 8 : 4);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK(xRCX);
                B_NEXT_nocond;
                MARK2; // Part with DF==1
                LDxw(x1, xRSI, 0);
                SDxw(x1, xRDI, 0);
                ADDI_D(xRSI, xRSI, rex.w ? -8 : -4);
                ADDI_D(xRDI, xRDI, rex.w ? -8 : -4);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK2(xRCX);
                // done
            } else {
                INST_NAME("MOVSD");
                GETDIR(x3, x1, rex.w ? 8 : 4);
                if (rex.is67 && !rex.is32bits) {
                    ZEROUP(xRSI);
                    ZEROUP(xRDI);
                }
                LDxw(x1, xRSI, 0);
                SDxw(x1, xRDI, 0);
                ADD_D(xRSI, xRSI, x3);
                ADD_D(xRDI, xRDI, x3);
            }
            SMWRITE();
            break;
        case 0xA6:
            UP32_READ(xRSI);
            UP32_READ(xRDI);
            switch (rex.rep) {
                case 1:
                case 2:
                    UP32_READ(xRCX);
                    if (rex.rep == 1) {
                        INST_NAME("REPNZ CMPSB");
                    } else {
                        INST_NAME("REPZ CMPSB");
                    }
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    SMREAD();
                    CBZ_NEXT(xRCX);
                    if (rex.is67 && !rex.is32bits) {
                        ZEROUP(xRSI);
                        ZEROUP(xRDI);
                    }
                    ANDI(x1, xFlags, 1 << F_DF);
                    BNEZ_MARK2(x1);
                    MARK; // Part with DF==0
                    LD_BU(x1, xRSI, 0);
                    LD_BU(x2, xRDI, 0);
                    ADDI_D(xRSI, xRSI, 1);
                    ADDI_D(xRDI, xRDI, 1);
                    ADDI_D(xRCX, xRCX, -1);
                    if (rex.rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNEZ_MARK(xRCX);
                    B_MARK3_nocond;
                    MARK2; // Part with DF==1
                    LD_BU(x1, xRSI, 0);
                    LD_BU(x2, xRDI, 0);
                    ADDI_D(xRSI, xRSI, -1);
                    ADDI_D(xRDI, xRDI, -1);
                    ADDI_D(xRCX, xRCX, -1);
                    if (rex.rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNEZ_MARK2(xRCX);
                    MARK3; // end
                    emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    break;
                default:
                    INST_NAME("CMPSB");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    GETDIR(x3, x1, 1);
                    SMREAD();
                    if (rex.is67 && !rex.is32bits) {
                        ZEROUP(xRSI);
                        ZEROUP(xRDI);
                    }
                    LD_BU(x1, xRSI, 0);
                    LD_BU(x2, xRDI, 0);
                    ADD_D(xRSI, xRSI, x3);
                    ADD_D(xRDI, xRDI, x3);
                    emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    break;
            }
            break;
        case 0xA7:
            UP32_READ(xRSI);
            UP32_READ(xRDI);
            switch (rex.rep) {
                case 1:
                case 2:
                    if (rex.rep == 1) {
                        INST_NAME("REPNZ CMPSD");
                    } else {
                        INST_NAME("REPZ CMPSD");
                    }
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    SMREAD();
                    UP32_READ(xRCX);
                    CBZ_NEXT(xRCX);
                    if (rex.is67 && !rex.is32bits) {
                        ZEROUP(xRSI);
                        ZEROUP(xRDI);
                    }
                    ANDI(x1, xFlags, 1 << F_DF);
                    BNEZ_MARK2(x1);
                    MARK; // Part with DF==0
                    LDxw(x1, xRSI, 0);
                    LDxw(x2, xRDI, 0);
                    ADDI_D(xRSI, xRSI, rex.w ? 8 : 4);
                    ADDI_D(xRDI, xRDI, rex.w ? 8 : 4);
                    ADDI_D(xRCX, xRCX, -1);
                    if (rex.rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNEZ_MARK(xRCX);
                    B_MARK3_nocond;
                    MARK2; // Part with DF==1
                    LDxw(x1, xRSI, 0);
                    LDxw(x2, xRDI, 0);
                    ADDI_D(xRSI, xRSI, rex.w ? -8 : -4);
                    ADDI_D(xRDI, xRDI, rex.w ? -8 : -4);
                    ADDI_D(xRCX, xRCX, -1);
                    if (rex.rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNEZ_MARK2(xRCX);
                    MARK3; // end
                    emit_cmp32(dyn, ninst, rex, x1, x2, x3, x4, x5, x6);
                    break;
                default:
                    INST_NAME("CMPSD");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    GETDIR(x3, x1, rex.w ? 8 : 4);
                    SMREAD();
                    if (rex.is67 && !rex.is32bits) {
                        ZEROUP(xRSI);
                        ZEROUP(xRDI);
                    }
                    LDxw(x1, xRSI, 0);
                    LDxw(x2, xRDI, 0);
                    ADD_D(xRSI, xRSI, x3);
                    ADD_D(xRDI, xRDI, x3);
                    emit_cmp32(dyn, ninst, rex, x1, x2, x3, x4, x5, x6);
                    break;
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
            MARKREGs(xRAX);
            MOV64xw(x2, i64);
            emit_test32(dyn, ninst, rex, xRAX, x2, x3, x4, x5);
            break;
        case 0xAA:
            UP32_READ(xRDI);
            UP32_READ(xRAX);
            if (rex.rep) {
                UP32_READ(xRCX);
                INST_NAME("REP STOSB");
                CBZ_NEXT(xRCX);
                if (rex.is67 && !rex.is32bits) {
                    ZEROUP(xRDI);
                }
                v0 = fpu_get_scratch(dyn);
                VREPLGR2VR_B(v0, xRAX);
                MOV64x(x4, 16);
                MOV64x(x5, 8);
                MOV64x(x6, 4);

                ANDI(x1, xFlags, 1 << F_DF);
                BNEZ_MARK3(x1);
                // Part with DF==0

                BLT(xRCX, x4, 4 + 4 * 4); // loop in 16bytes
                VST(v0, xRDI, 0);
                ADDI_D(xRDI, xRDI, 16);
                ADDI_D(xRCX, xRCX, -16);
                BGE(xRCX, x4, -4 * 3);

                BLT(xRCX, x5, 4 + 4 * 4); // try 8 bytes
                FST_D(v0, xRDI, 0);
                ADDI_D(xRDI, xRDI, 8);
                ADDI_D(xRCX, xRCX, -8);
                BGE(xRCX, x5, -4 * 3);

                BLT(xRCX, x6, 4 + 4 * 4); // try 4 bytes
                FST_S(v0, xRDI, 0);
                ADDI_D(xRDI, xRDI, 4);
                ADDI_D(xRCX, xRCX, -4);
                BGE(xRCX, x6, -4 * 3);

                BEQZ(xRCX, 4 + 4 * 4);
                MARK;
                ST_B(xRAX, xRDI, 0);
                ADDI_D(xRDI, xRDI, 1);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK(xRCX);
                B_NEXT_nocond;

                // Part with DF==1
                MARK3;
                BLT(xRCX, x4, 4 + 4 * 4); // loop in 16bytes
                VST(v0, xRDI, -15);
                ADDI_D(xRDI, xRDI, -16);
                ADDI_D(xRCX, xRCX, -16);
                BGE(xRCX, x4, -4 * 3);

                BLT(xRCX, x5, 4 + 4 * 4); // try 8 bytes
                FST_D(v0, xRDI, -7);
                ADDI_D(xRDI, xRDI, -8);
                ADDI_D(xRCX, xRCX, -8);
                BGE(xRCX, x5, -4 * 3);

                BLT(xRCX, x6, 4 + 4 * 4); // try 4 bytes
                FST_S(v0, xRDI, -3);
                ADDI_D(xRDI, xRDI, -4);
                ADDI_D(xRCX, xRCX, -4);
                BGE(xRCX, x6, -4 * 3);

                BEQZ(xRCX, 4 + 4 * 4);
                MARK2;
                ST_B(xRAX, xRDI, 0);
                ADDI_D(xRDI, xRDI, -1);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK2(xRCX);
                // done
            } else {
                INST_NAME("STOSB");
                GETDIR(x3, x1, 1);
                if (rex.is67 && !rex.is32bits) {
                    ZEROUP(xRDI);
                }
                ST_B(xRAX, xRDI, 0);
                ADD_D(xRDI, xRDI, x3);
            }
            SMWRITE();
            break;
        case 0xAB:
            UP32_READ(xRDI);
            UP32_READ(xRAX);
            if (rex.rep) {
                UP32_READ(xRCX);
                INST_NAME("REP STOSD");
                CBZ_NEXT(xRCX);
                if (rex.is67 && !rex.is32bits) {
                    ZEROUP(xRDI);
                }
                ANDI(x1, xFlags, 1 << F_DF);
                BNEZ_MARK2(x1);
                MARK; // Part with DF==0
                SDxw(xRAX, xRDI, 0);
                ADDI_D(xRDI, xRDI, rex.w ? 8 : 4);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK(xRCX);
                B_NEXT_nocond;
                MARK2; // Part with DF==1
                SDxw(xRAX, xRDI, 0);
                ADDI_D(xRDI, xRDI, rex.w ? -8 : -4);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK2(xRCX);
                // done
            } else {
                INST_NAME("STOSD");
                GETDIR(x3, x1, rex.w ? 8 : 4);
                if (rex.is67 && !rex.is32bits) {
                    ZEROUP(xRDI);
                }
                SDxw(xRAX, xRDI, 0);
                ADD_D(xRDI, xRDI, x3);
            }
            SMWRITE();
            break;
        case 0xAC:
            UP32_READ(xRSI);
            if (rex.rep) {
                UP32_READ(xRCX);
                INST_NAME("REP LODSB");
                GETDIR(x1, x2, 1);
                SMREAD();
                CBZ_NEXT(xRCX);
                if (rex.is67 && !rex.is32bits)
                    ZEROUP(xRSI);
                MARK;
                LD_BU(x2, xRSI, 0);
                ADD_D(xRSI, xRSI, x1);
                BSTRINS_D(xRAX, x2, 7, 0);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK(xRCX);
            } else {
                INST_NAME("LODSB");
                GETDIR(x1, x2, 1);
                SMREAD();
                if (rex.is67 && !rex.is32bits) {
                    ZEROUP(xRSI);
                }
                LD_BU(x2, xRSI, 0);
                ADD_D(xRSI, xRSI, x1);
                BSTRINS_D(xRAX, x2, 7, 0);
            }
            break;
        case 0xAD:
            UP32_READ(xRSI);
            if (rex.rep) {
                UP32_READ(xRCX);
                INST_NAME("REP LODSD");
                CBZ_NEXT(xRCX);
                GETDIR(x1, x2, rex.w ? 8 : 4);
                if (rex.is67 && !rex.is32bits)
                    ZEROUP(xRSI);
                MARK;
                LDxw(xRAX, xRSI, 0);
                ADD_D(xRSI, xRSI, x1);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK(xRCX);
            } else {
                INST_NAME("LODSD");
                GETDIR(x1, x2, rex.w ? 8 : 4);
                if (rex.is67 && !rex.is32bits) {
                    ZEROUP(xRSI);
                }
                LDxw(xRAX, xRSI, 0);
                ADD_D(xRSI, xRSI, x1);
            }
            break;
        case 0xAE:
            UP32_READ(xRDI);
            UP32_READ(xRAX);
            switch (rex.rep) {
                case 1:
                case 2:
                    if (rex.rep == 1) {
                        INST_NAME("REPNZ SCASB");
                    } else {
                        INST_NAME("REPZ SCASB");
                    }
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    SMREAD();
                    UP32_READ(xRCX);
                    CBZ_NEXT(xRCX);
                    ANDI(x1, xRAX, 0xff);
                    if (rex.is67 && !rex.is32bits) {
                        ZEROUP(xRDI);
                    }
                    ANDI(x2, xFlags, 1 << F_DF);
                    BNEZ_MARK2(x2);
                    MARK; // Part with DF==0
                    LD_BU(x2, xRDI, 0);
                    ADDI_D(xRDI, xRDI, 1);
                    ADDI_D(xRCX, xRCX, -1);
                    if (rex.rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNE_MARK(xRCX, xZR);
                    B_MARK3_nocond;
                    MARK2; // Part with DF==1
                    LD_BU(x2, xRDI, 0);
                    ADDI_D(xRDI, xRDI, -1);
                    ADDI_D(xRCX, xRCX, -1);
                    if (rex.rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNE_MARK2(xRCX, xZR);
                    MARK3; // end
                    emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    break;
                default:
                    INST_NAME("SCASB");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    GETDIR(x3, x1, 1);
                    ANDI(x1, xRAX, 0xff);
                    if (rex.is67 && !rex.is32bits) {
                        ZEROUP(xRDI);
                    }
                    LD_BU(x2, xRDI, 0);
                    ADD_D(xRDI, xRDI, x3);
                    emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    break;
            }
            break;
        case 0xAF:
            UP32_READ(xRDI);
            UP32_READ(xRAX);
            switch (rex.rep) {
                case 1:
                case 2:
                    if (rex.rep == 1) {
                        INST_NAME("REPNZ SCASD");
                    } else {
                        INST_NAME("REPZ SCASD");
                    }
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    SMREAD();
                    UP32_READ(xRCX);
                    CBZ_NEXT(xRCX);
                    if (rex.w) {
                        MV(x1, xRAX);
                    } else {
                        ZEROUP2(x1, xRAX);
                    }
                    if (rex.is67 && !rex.is32bits) {
                        ZEROUP(xRDI);
                    }
                    ANDI(x2, xFlags, 1 << F_DF);
                    BNEZ_MARK2(x2);
                    MARK; // Part with DF==0
                    LDxw(x2, xRDI, 0);
                    ADDI_D(xRDI, xRDI, rex.w ? 8 : 4);
                    ADDI_D(xRCX, xRCX, -1);
                    if (rex.rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNE_MARK(xRCX, xZR);
                    B_MARK3_nocond;
                    MARK2; // Part with DF==1
                    LDxw(x2, xRDI, 0);
                    ADDI_D(xRDI, xRDI, rex.w ? -8 : -4);
                    ADDI_D(xRCX, xRCX, -1);
                    if (rex.rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNE_MARK2(xRCX, xZR);
                    MARK3; // end
                    emit_cmp32(dyn, ninst, rex, x1, x2, x3, x4, x5, x6);
                    break;
                default:
                    INST_NAME("SCASD");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    GETDIR(x3, x1, rex.w ? 8 : 4);
                    if (rex.is67 && !rex.is32bits) {
                        ZEROUP(xRDI);
                    }
                    LDxw(x2, xRDI, 0);
                    ADD_D(xRDI, xRDI, x3);
                    emit_cmp32(dyn, ninst, rex, xRAX, x2, x3, x4, x5, x6);
                    break;
            }
            break;
        case 0xB0:
        case 0xB1:
        case 0xB2:
        case 0xB3:
            INST_NAME("MOV xL, Ib");
            u8 = F8;
            SCRATCH_USAGE(0);
            if (rex.rex)
                gb1 = TO_NAT((opcode & 7) + (rex.b << 3));
            else
                gb1 = TO_NAT(opcode & 3);
            BSTRINS_D(gb1, xZR, 7, 0);
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
                BSTRINS_D(gb1, x1, 7, 0);
            } else {
                gb1 = TO_NAT(opcode & 3);
                BSTRINS_D(gb1, x1, 15, 8);
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
            MARKREGd(gd);
            if (rex.w) {
                u64 = F64;
                MOV64x(gd, u64);
            } else {
                u32 = F32;
                la64_move32(dyn, ninst, gd, u32, 0);
                if ((int32_t)u32 >= 0)
                    UP32_ZERO(gd);
                else if (NEED_ZEROUP(gd))
                    ZEROUP_RESULT(gd);
            }
            break;
        default:
            DEFAULT;
    }

    return addr;
}
