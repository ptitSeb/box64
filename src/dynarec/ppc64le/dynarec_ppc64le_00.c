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
    uint8_t gb1, gb2, eb1, eb2;
    uint8_t wback, wb2;
    uint8_t u8;
    uint32_t u32;
    uint64_t u64;
    int64_t j64;
    int v0, v1;
    int i32;
    int64_t i64, fixedaddress;
    int32_t tmp;
    int lock;
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

    opcode = F8;

    switch (opcode) {
        case 0x00:
            INST_NAME("ADD Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            SCRATCH_USAGE(0);
            // Get Gb (source byte register)
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
            // Get Eb (dest byte register/memory)
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                if (rex.rex) {
                    eb1 = TO_NAT(ed);
                    eb2 = 0;
                } else {
                    eb1 = TO_NAT(ed & 3);
                    eb2 = ((ed & 4) >> 2);
                }
                if (eb2) {
                    ed = x5;
                    BF_EXTRACT(ed, eb1, eb2 * 8 + 7, eb2 * 8);
                } else {
                    ed = eb1;
                }
                emit_add8(dyn, ninst, ed, gd, x1, x2);
                BF_INSERT(eb1, ed, eb2 * 8 + 7, eb2 * 8);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, DS_DISP, 0);
                SMREADLOCK(lock);
                LBZ(x5, fixedaddress, ed);
                emit_add8(dyn, ninst, x5, gd, x1, x2);
                STB(x5, fixedaddress, ed);
                SMWRITELOCK(lock);
            }
            break;
        case 0x01:
            INST_NAME("ADD Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            SCRATCH_USAGE(0);
            if (MODREG) { // reg <= reg
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                emit_add32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            } else { // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, DS_DISP, 0);
                SMREADLOCK(lock);
                LDxw(x5, fixedaddress, ed);
                emit_add32(dyn, ninst, rex, x5, gd, x3, x4, x1);
                SDxw(x5, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0x02:
            INST_NAME("ADD Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            SCRATCH_USAGE(0);
            // Get Gb (dest byte register)
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            if (rex.rex) {
                gb2 = 0;
                gb1 = TO_NAT(gd);
            } else {
                gb2 = ((gd & 4) << 1);
                gb1 = TO_NAT(gd & 3);
            }
            if (gb2) {
                tmp1 = x4;
                BF_EXTRACT(tmp1, gb1, gb2 + 7, gb2);
            } else {
                tmp1 = gb1;
            }
            // Get Eb (source byte register/memory)
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                if (rex.rex) {
                    eb1 = TO_NAT(ed);
                    eb2 = 0;
                } else {
                    eb1 = TO_NAT(ed & 3);
                    eb2 = ((ed & 4) >> 2);
                }
                if (eb2) {
                    ed = x5;
                    BF_EXTRACT(ed, eb1, eb2 * 8 + 7, eb2 * 8);
                } else {
                    ed = eb1;
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                SMREAD();
                LBZ(x5, fixedaddress, ed);
                ed = x5;
            }
            emit_add8(dyn, ninst, tmp1, ed, x1, x2);
            BF_INSERT(gb1, tmp1, gb2 + 7, gb2);
            break;
        case 0x03:
            INST_NAME("ADD Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            SCRATCH_USAGE(0);
            if (MODREG) { // reg <= reg
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                emit_add32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            } else { // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                SMREAD();
                LDxw(x5, fixedaddress, ed);
                emit_add32(dyn, ninst, rex, gd, x5, x3, x4, x1);
            }
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
                    eb2 = ((ed & 4) >> 2); // L or H
                }
                BF_INSERT(eb1, gd, eb2 * 8 + 7, eb2 * 8);
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
        case 0xC6:
            INST_NAME("MOV Eb, Ib");
            nextop = F8;
            if (MODREG) { // reg <= u8
                u8 = F8;
                if (!rex.rex) {
                    ed = (nextop & 7);
                    eb1 = TO_NAT((ed & 3)); // Ax, Cx, Dx or Bx
                    eb2 = (ed & 4) >> 2;    // L or H
                } else {
                    eb1 = TO_NAT((nextop & 7) + (rex.b << 3));
                    eb2 = 0;
                }
                MOV32w(x3, u8);
                BF_INSERT(eb1, x3, eb2 * 8 + 7, eb2 * 8);
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
        default:
            DEFAULT;
    }

    return addr;
}
