#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "../dynablock_private.h"
#include "custommem.h"

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "../dynarec_helper.h"
#include "dynarec_arm64_functions.h"

uintptr_t dynarec64_64(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int seg, int* ok, int* need_epilog)
{
    (void)ip; (void)rep; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t u8;
    uint8_t gd, ed, eb1, eb2, gb1, gb2;
    uint8_t wback, wb1, wb2, wb;
    int64_t i64, j64;
    uint64_t u64;
    int v0, v1;
    int q0;
    int d0;
    int64_t fixedaddress;
    int unscaled;
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(wb1);
    MAYUSE(wb2);
    MAYUSE(gb1);
    MAYUSE(gb2);
    MAYUSE(j64);
    MAYUSE(d0);
    MAYUSE(q0);
    MAYUSE(v0);
    MAYUSE(v1);

    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }
    GETREX();

    switch(opcode) {

        case 0x01:
            INST_NAME("ADD Seg:Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            GETGD;
            GETEDO(x4, 0);
            emit_add32(dyn, ninst, rex, ed, gd, x3, x5);
            WBACKO(x4);
            break;
        case 0x03:
            INST_NAME("ADD Gd, Seg:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            GETGD;
            GETEDO(x4, 0);
            emit_add32(dyn, ninst, rex, gd, ed, x3, x4);
            break;

        case 0x0B:
            INST_NAME("OR Gd, Seg:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            GETGD;
            GETEDO(x4, 0);
            emit_or32(dyn, ninst, rex, gd, ed, x3, x4);
            break;

        case 0x0F:
            opcode = F8;
            switch(opcode) {

                case 0x10:
                    switch(rep) {
                        case 1:
                            INST_NAME("MOVSD Gx, Ex");
                            nextop = F8;
                            GETG;
                            if(MODREG) {
                                ed = (nextop&7)+ (rex.b<<3);
                                v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                                d0 = sse_get_reg(dyn, ninst, x1, ed, 0);
                                VMOVeD(v0, 0, d0, 0);
                            } else {
                                grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                                SMREAD();
                                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                                ADDz_REG(x4, x4, ed);
                                VLD64(v0, x4, fixedaddress); // upper part reseted
                            }
                            break;
                        case 2:
                            INST_NAME("MOVSS Gx, Ex");
                            nextop = F8;
                            GETG;
                            if(MODREG) {
                                ed = (nextop&7)+ (rex.b<<3);
                                v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                                q0 = sse_get_reg(dyn, ninst, x1, ed, 0);
                                VMOVeS(v0, 0, q0, 0);
                            } else {
                                grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                                SMREAD();
                                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                                ADDz_REG(x4, x4, ed);
                                VLD32(v0, x4, fixedaddress);
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;
                case 0x11:
                    switch(rep) {
                        case 0:
                            INST_NAME("MOVUPS Ex,Gx");
                            nextop = F8;
                            GETG;
                            if(MODREG) {
                                v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                                ed = (nextop&7)+(rex.b<<3);
                                v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                                VMOVQ(v1, v0);
                            } else {
                                grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                                v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                                ADDz_REG(x4, x4, ed);
                                VST128(v0, x4, fixedaddress);
                                SMWRITE2();
                            }
                            break;
                        case 1:
                            INST_NAME("MOVSD Ex, Gx");
                            nextop = F8;
                            GETG;
                            if(MODREG) {
                                v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                                ed = (nextop&7)+ (rex.b<<3);
                                d0 = sse_get_reg(dyn, ninst, x1, ed, 1);
                                VMOVeD(d0, 0, v0, 0);
                            } else {
                                grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                                v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                                ADDz_REG(x4, x4, ed);
                                VST64(v0, x4, fixedaddress);
                                SMWRITE2();
                            }
                            break;
                        case 2:
                            INST_NAME("MOVSS Ex, Gx");
                            nextop = F8;
                            GETG;
                            if(MODREG) {
                                v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                                ed = (nextop&7)+ (rex.b<<3);
                                q0 = sse_get_reg(dyn, ninst, x1, ed, 1);
                                VMOVeS(q0, 0, v0, 0);
                            } else {
                                grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                                v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                                ADDz_REG(x4, x4, ed);
                                VST32(v0, x4, fixedaddress);
                                SMWRITE2();
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;
                case 0x18:
                case 0x19:
                case 0x1F:
                    INST_NAME("NOP (multibyte)");
                    nextop = F8;
                    FAKEED;
                    break;
                case 0x28:
                    switch(rep) {
                        case 0:
                            INST_NAME("MOVAPS Gx,Seg:Ex");
                            nextop = F8;
                            GETG;
                            if(MODREG) {
                                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                                VMOVQ(v0, v1);
                            } else {
                                grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                                SMREAD();
                                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                                ADDz_REG(x4, x4, ed);
                                VLD128(v0, x4, fixedaddress);
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;
                case 0x29:
                    switch(rep) {
                        case 0:
                            INST_NAME("MOVAPS Seg:Ex,Gx");
                            nextop = F8;
                            GETG;
                            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                            if(MODREG) {
                                ed = (nextop&7)+(rex.b<<3);
                                v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                                VMOVQ(v1, v0);
                            } else {
                                grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                                ADDz_REG(x4, x4, ed);
                                VST128(v0, x4, fixedaddress);
                                SMWRITE2();
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;

                case 0x6F:
                    switch(rep) {
                        case 2:
                            INST_NAME("MOVDQU Gx,Ex");// no alignment constraint on NEON here, so same as MOVDQA
                            nextop = F8;
                            GETG;
                            if(MODREG) {
                                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                                VMOVQ(v0, v1);
                            } else {
                                grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                                SMREAD();
                                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                                ADDz_REG(x4, x4, ed);
                                VLD128(v0, x4, fixedaddress);
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;

                case 0x7F:
                    switch(rep) {
                        case 2:
                            INST_NAME("MOVDQU Ex,Gx");
                            nextop = F8;
                            GETGX(v0, 0);
                            if(MODREG) {
                                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop&7) + (rex.b<<3));
                                VMOVQ(v1, v0);
                            } else {
                                grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                                ADDz_REG(x4, x4, ed);
                                VST128(v0, x4, fixedaddress);
                                SMWRITE2();
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;

                case 0xAF:
                    switch(rep) {
                        case 0:
                            INST_NAME("IMUL Gd, Ed");
                            if(BOX64ENV(dynarec_safeflags)>1 && BOX64ENV(cputype)) {
                                SETFLAGS(X_OF|X_CF, SF_SET);
                            } else {
                                SETFLAGS(X_ALL, SF_SET);
                            }
                            nextop = F8;
                            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                            GETGD;
                            GETEDO(x4, 0);
                            if(rex.w) {
                                // 64bits imul
                                UFLAG_IF {
                                    SMULH(x3, gd, ed);
                                    MULx(gd, gd, ed);
                                    SET_DFNONE();
                                    IFX(X_CF|X_OF) {
                                        ASRx(x4, x3, 63);
                                        CMPSx_REG(x3, x4);
                                        CSETw(x3, cNE);
                                        IFX(X_CF) {
                                            BFIw(xFlags, x3, F_CF, 1);
                                        }
                                        IFX(X_OF) {
                                            BFIw(xFlags, x3, F_OF, 1);
                                        }
                                    }
                                } else {
                                    MULxw(gd, gd, ed);
                                }
                            } else {
                                // 32bits imul
                                UFLAG_IF {
                                    SMULL(gd, gd, ed);
                                    UFLAG_RES(gd);
                                    LSRx(x3, gd, 32);
                                    MOVw_REG(gd, gd);
                                    SET_DFNONE();
                                    IFX(X_CF|X_OF) {
                                        ASRw(x4, gd, 31);
                                        CMPSw_REG(x3, x4);
                                        CSETw(x3, cNE);
                                        IFX(X_CF) {
                                            BFIw(xFlags, x3, F_CF, 1);
                                        }
                                        IFX(X_OF) {
                                            BFIw(xFlags, x3, F_OF, 1);
                                        }
                                    }
                                } else {
                                    MULxw(gd, gd, ed);
                                }
                            }
                            IFX2(X_AF, && !BOX64ENV(cputype)) {BFCw(xFlags, F_AF, 1);}
                            IFX2(X_ZF, && !BOX64ENV(cputype)) {BFCw(xFlags, F_ZF, 1);}
                            IFX2(X_SF, && !BOX64ENV(cputype)) {
                                LSRxw(x3, gd, rex.w?63:31);
                                BFIw(xFlags, x3, F_SF, 1);
                            }
                            IFX2(X_PF, && !BOX64ENV(cputype)) emit_pf(dyn, ninst, gd, x3);
                            break;
                        default:
                            DEFAULT;
                    }
                    break;

                case 0xB6:
                    switch(rep) {
                        case 0:
                            INST_NAME("MOVZX Gd, Eb");
                            nextop = F8;
                            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                            GETGD;
                            if(MODREG) {
                                if(rex.rex) {
                                    eb1 = TO_NAT((nextop & 7) + (rex.b << 3));
                                    eb2 = 0;                \
                                } else {
                                    ed = (nextop&7);
                                    eb1 = TO_NAT(ed & 3); // Ax, Cx, Dx or Bx
                                    eb2 = (ed&4)>>2;    // L or H
                                }
                                UBFXxw(gd, eb1, eb2*8, 8);
                            } else {
                                SMREAD();
                                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                                LDRB_REGz(gd, x4, ed);
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;

                case 0xBF:
                    switch(rep) {
                        case 0:
                            INST_NAME("MOVSX Gd, Ew");
                            nextop = F8;
                            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                            GETGD;
                            if(MODREG) {
                                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                                SXTHxw(gd, ed);
                            } else {
                                SMREAD();
                                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                                LDRSH_REGz(gd, x4, ed);
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;

                default:
                    DEFAULT;
            }
            break;

        case 0x13:
            INST_NAME("ADC Gd, Seg:Ed");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            GETGD;
            GETEDO(x4, 0);
            emit_adc32(dyn, ninst, rex, gd, ed, x3, x5);
            break;

        case 0x1B:
            INST_NAME("SBB Gd, Seg:Ed");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            GETGD;
            GETEDO(x4, 0);
            emit_sbb32(dyn, ninst, rex, gd, ed, x3, x5);
            break;

        case 0x21:
            INST_NAME("AND Seg:Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            GETGD;
            GETEDO(x4, 0);
            emit_and32(dyn, ninst, rex, ed, gd, x3, x5);
            WBACKO(x4);
            break;

        case 0x23:
            INST_NAME("AND Gd, Seg:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            GETGD;
            GETEDO(x4, 0);
            emit_and32(dyn, ninst, rex, gd, ed, x3, x5);
            break;

        case 0x29:
            INST_NAME("SUB Seg:Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            GETGD;
            GETEDO(x4, 0);
            emit_sub32(dyn, ninst, rex, ed, gd, x3, x5);
            WBACKO(x4);
            break;

        case 0x2B:
            INST_NAME("SUB Gd, Seg:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            GETGD;
            GETEDO(x4, 0);
            emit_sub32(dyn, ninst, rex, gd, ed, x3, x4);
            break;

        case 0x31:
            INST_NAME("XOR Seg:Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            GETGD;
            GETEDO(x4, 0);
            emit_xor32(dyn, ninst, rex, ed, gd, x3, x5);
            WBACKO(x4);
            break;

        case 0x33:
            INST_NAME("XOR Gd, Seg:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            GETGD;
            GETEDO(x4, 0);
            emit_xor32(dyn, ninst, rex, gd, ed, x3, x4);
            break;

        case 0x39:
            INST_NAME("CMP Seg:Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            GETGD;
            GETEDO(x4, 0);
            emit_cmp32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            break;

        case 0x3B:
            INST_NAME("CMP Gd, Seg:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            GETGD;
            GETEDO(x4, 0);
            emit_cmp32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;

        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5E:
        case 0x5F:
            // just use regular push/pop
            return dynarec64_00(dyn, addr-1, ip, ninst, rex, rep, ok, need_epilog);

        case 0x63:
            if(rex.is32bits) {
                // ARPL here
                DEFAULT;
            } else {
                INST_NAME("MOVSXD Gd, Ed");
                nextop = F8;
                GETGD;
                if(rex.w) {
                    if(MODREG) {   // reg <= reg
                        SXTWx(gd, TO_NAT((nextop & 7) + (rex.b << 3)));
                    } else {                    // mem <= reg
                        grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                        LDRSW_REGz(gd, x4, ed);
                    }
                } else {
                    if(MODREG) {   // reg <= reg
                        MOVw_REG(gd, TO_NAT((nextop & 7) + (rex.b << 3)));
                    } else {                    // mem <= reg
                        grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                        LDRw_REGz(gd, x4, ed);
                    }
                }
            }
            break;
        case 0x64:
            addr = dynarec64_64(dyn, addr, ip, ninst, rex, rep, _FS, ok, need_epilog);
            break;
        case 0x65:
            addr = dynarec64_64(dyn, addr, ip, ninst, rex, rep, _GS, ok, need_epilog);
            break;
        case 0x66:
            addr = dynarec64_6664(dyn, addr, ip, ninst, rex, seg, ok, need_epilog);
            break;
        case 0x67:
            if(rex.is32bits)
                addr = dynarec64_6764_32(dyn, addr, ip, ninst, rex, rep, seg, ok, need_epilog);
            else {
                addr = dynarec64_6764(dyn, addr, ip, ninst, rex, rep, seg, ok, need_epilog);
            }
            break;

        case 0x69:
            INST_NAME("IMUL Gd, Ed, Id");
            if(BOX64ENV(dynarec_safeflags)>1 && BOX64ENV(cputype)) {
                SETFLAGS(X_OF|X_CF, SF_SET);
            } else {
                SETFLAGS(X_ALL, SF_SET);
            }
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            GETGD;
            GETEDO(x4, 4);
            i64 = F32S;
            MOV64xw(x4, i64);
            if(rex.w) {
                // 64bits imul
                UFLAG_IF {
                    SMULH(x3, ed, x4);
                    MULx(gd, ed, x4);
                    SET_DFNONE();
                    IFX(X_CF | X_OF) {
                        ASRx(x4, gd, 63);
                        CMPSx_REG(x3, x4);
                        CSETw(x1, cNE);
                        IFX(X_CF) {
                            BFIw(xFlags, x1, F_CF, 1);
                        }
                        IFX(X_OF) {
                            BFIw(xFlags, x1, F_OF, 1);
                        }
                    }
                } else {
                    MULxw(gd, ed, x4);
                }
            } else {
                // 32bits imul
                UFLAG_IF {
                    SMULL(gd, ed, x4);
                    LSRx(x3, gd, 32);
                    MOVw_REG(gd, gd);
                    SET_DFNONE();
                    IFX(X_CF | X_OF) {
                        ASRw(x4, gd, 31);
                        CMPSw_REG(x3, x4);
                        CSETw(x1, cNE);
                        IFX(X_CF) {
                            BFIw(xFlags, x1, F_CF, 1);
                        }
                        IFX(X_OF) {
                            BFIw(xFlags, x1, F_OF, 1);
                        }
                    }
                } else {
                    MULxw(gd, ed, x4);
                }
            }
            IFX2(X_AF, && !BOX64ENV(cputype)) {BFCw(xFlags, F_AF, 1);}
            IFX2(X_ZF, && !BOX64ENV(cputype)) {BFCw(xFlags, F_ZF, 1);}
            IFX2(X_SF, && !BOX64ENV(cputype)) {
                LSRxw(x3, gd, rex.w?63:31);
                BFIw(xFlags, x3, F_SF, 1);
            }
            IFX2(X_PF, && !BOX64ENV(cputype)) emit_pf(dyn, ninst, gd, x3);
            break;

        case 0x6C:
        case 0x6D:
            INST_NAME(opcode == 0x6C ? "INSB" : "INSD");
            if(BOX64DRENV(dynarec_safeflags)>1) {
                READFLAGS(X_PEND);
            } else {
                SETFLAGS(X_ALL, SF_SET_NODF);    // Hack to set flags in "don't care" state
            }
            GETIP(ip);
            STORE_XEMU_CALL(xRIP);
            CALL_S(const_native_priv, -1);
            LOAD_XEMU_CALL(xRIP);
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0x6E:
        case 0x6F:
            INST_NAME(opcode == 0x6C ? "OUTSB" : "OUTSD");
            if(BOX64DRENV(dynarec_safeflags)>1) {
                READFLAGS(X_PEND);
            } else {
                SETFLAGS(X_ALL, SF_SET_NODF);    // Hack to set flags in "don't care" state
            }
            GETIP(ip);
            STORE_XEMU_CALL(xRIP);
            CALL_S(const_native_priv, -1);
            LOAD_XEMU_CALL(xRIP);
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0x70:
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x76:
        case 0x77:
        case 0x78:
        case 0x79:
        case 0x7A:
        case 0x7B:
        case 0x7C:
        case 0x7D:
        case 0x7E:
        case 0x7F:
            // just use regular conditional jump
            return dynarec64_00(dyn, addr-1, ip, ninst, rex, rep, ok, need_epilog);
        case 0x80:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: //ADD
                    INST_NAME("ADD Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_add8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK;
                    break;
                case 1: //OR
                    INST_NAME("OR Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_or8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK;
                    break;
                case 2: //ADC
                    INST_NAME("ADC Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_adc8c(dyn, ninst, x1, u8, x2, x4, x5);
                    EBBACK;
                    break;
                case 3: //SBB
                    INST_NAME("SBB Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_sbb8c(dyn, ninst, x1, u8, x2, x4, x5);
                    EBBACK;
                    break;
                case 4: //AND
                    INST_NAME("AND Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_and8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK;
                    break;
                case 5: //SUB
                    INST_NAME("SUB Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_sub8c(dyn, ninst, x1, u8, x2, x4, x5);
                    EBBACK;
                    break;
                case 6: //XOR
                    INST_NAME("XOR Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_xor8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK;
                    break;
                case 7: //CMP
                    INST_NAME("CMP Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    if(u8) {
                        MOV32w(x2, u8);
                        emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5);
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
            grab_segdata(dyn, addr, ninst, x6, seg, (MODREG));
            switch((nextop>>3)&7) {
                case 0: //ADD
                    if(opcode==0x81) {INST_NAME("ADD Ed, Id");} else {INST_NAME("ADD Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEDO(x6, (opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_add32c(dyn, ninst, rex, ed, i64, x3, x4, x5);
                    WBACKO(x6);
                    break;
                case 1: //OR
                    if(opcode==0x81) {INST_NAME("OR Ed, Id");} else {INST_NAME("OR Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEDO(x6, (opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_or32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACKO(x6);
                    break;
                case 2: //ADC
                    if(opcode==0x81) {INST_NAME("ADC Ed, Id");} else {INST_NAME("ADC Ed, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEDO(x6, (opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    MOV64xw(x5, i64);
                    emit_adc32(dyn, ninst, rex, ed, x5, x3, x4);
                    WBACKO(x6);
                    break;
                case 3: //SBB
                    if(opcode==0x81) {INST_NAME("SBB Ed, Id");} else {INST_NAME("SBB Ed, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEDO(x6, (opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    MOV64xw(x5, i64);
                    emit_sbb32(dyn, ninst, rex, ed, x5, x3, x4);
                    WBACKO(x6);
                    break;
                case 4: //AND
                    if(opcode==0x81) {INST_NAME("AND Ed, Id");} else {INST_NAME("AND Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEDO(x6, (opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_and32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACKO(x6);
                    break;
                case 5: //SUB
                    if(opcode==0x81) {INST_NAME("SUB Ed, Id");} else {INST_NAME("SUB Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEDO(x6, (opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_sub32c(dyn, ninst, rex, ed, i64, x3, x4, x5);
                    WBACKO(x6);
                    break;
                case 6: //XOR
                    if(opcode==0x81) {INST_NAME("XOR Ed, Id");} else {INST_NAME("XOR Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEDO(x6, (opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_xor32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACKO(x6);
                    break;
                case 7: //CMP
                    if(opcode==0x81) {INST_NAME("CMP Ed, Id");} else {INST_NAME("CMP Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEDO(x6, (opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    if(i64) {
                        MOV64xw(x2, i64);
                        emit_cmp32(dyn, ninst, rex, ed, x2, x3, x4, x5);
                    } else
                        emit_cmp32_0(dyn, ninst, rex, ed, x3, x4);
                    break;
            }
            break;

        case 0x85:
            INST_NAME("TEST Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop=F8;
            grab_segdata(dyn, addr, ninst, x6, seg, (MODREG));
            GETGD;
            GETEDO(x6, 0);
            emit_test32(dyn, ninst, rex, ed, gd, x3, x5, x6);
            break;

        case 0x88:
            INST_NAME("MOV Eb, Gb");
            nextop = F8;
            gd = ((nextop&0x38)>>3)+(rex.r<<3);
            if(rex.rex) {
                gb2 = 0;
                gb1 = TO_NAT(gd);
            } else {
                gb2 = ((gd&4)<<1);
                gb1 = TO_NAT(gd & 3);
            }
            if(gb2) {
                gd = x4;
                UBFXw(gd, gb1, gb2, 8);
            } else {
                gd = gb1;   // no need to extract
            }
            if(MODREG) {
                ed = (nextop&7) + (rex.b<<3);
                if(rex.rex) {
                    eb1 = TO_NAT(ed);
                    eb2 = 0;
                } else {
                    eb1 = TO_NAT(ed & 3); // Ax, Cx, Dx or Bx
                    eb2 = ((ed&4)>>2);    // L or H
                }
                BFIx(eb1, gd, eb2*8, 8);
            } else {
                grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                STRB_REGz(gd, x4, wback);
            }
            break;
        case 0x89:
            INST_NAME("MOV Seg:Ed, Gd");
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            GETGD;
            if(MODREG) {   // reg <= reg
                MOVxw_REG(TO_NAT((nextop & 7) + (rex.b << 3)), gd);
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                STRxw_REGz(gd, x4, ed);
                SMWRITE2();
            }
            break;
        case 0x8A:
            INST_NAME("MOV Gb, Eb");
            nextop = F8;
            if(rex.rex) {
                gb1 = gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3));
                gb2=0;
            } else {
                gd = (nextop&0x38)>>3;
                gb1 = TO_NAT(gd & 3);
                gb2 = ((gd&4)<<1);
            }
            if(MODREG) {
                if(rex.rex) {
                    wback = TO_NAT((nextop & 7) + (rex.b << 3));
                    wb2 = 0;
                } else {
                    wback = (nextop&7);
                    wb2 = (wback>>2);
                    wback = TO_NAT(wback & 3);
                }
                if(wb2) {
                    UBFXw(x4, wback, wb2*8, 8);
                    ed = x4;
                } else {
                    ed = wback;
                }
            } else {
                grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                LDRB_REGz(x4, x4, wback);
                ed = x4;
            }
            BFIx(gb1, ed, gb2, 8);
            break;
        case 0x8B:
            INST_NAME("MOV Gd, Seg:Ed");
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            GETGD;
            if(MODREG) {   // reg <= reg
                MOVxw_REG(gd, TO_NAT((nextop & 7) + (rex.b << 3)));
            } else {                    // mem <= reg
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                LDRxw_REGz(gd, x4, ed);
            }
            break;

        case 0x8D:
            INST_NAME("LEA Gd, Ed");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg? that's an invalid operation
                DEFAULT;
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, gd, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                if(gd!=ed) {    // it's sometimes used as a 3 bytes NOP
                    MOVxw_REG(gd, ed);
                }
                else if(!rex.w && !rex.is32bits) {
                    MOVw_REG(gd, gd);   //truncate the higher 32bits as asked
                }
            }
            break;
        case 0x8E:
            INST_NAME("MOV Seg,Ew");
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            u8 = (nextop&0x38)>>3;
            if((u8>5) || (u8==1)) {
                INST_NAME("Invalid MOV Seg,Ew");
                UDF(0);
                *need_epilog = 1;
                *ok = 0;
            } else {
                if (MODREG) {
                    ed = TO_NAT((nextop & 7) + (rex.b << 3));
                } else {
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                    LDRH_REGz(x1, x4, wback);
                    ed = x1;
                }
                STRH_U12(ed, xEmu, offsetof(x64emu_t, segs[u8]));
                if((u8==_FS) || (u8==_GS)) {
                    // refresh offset if needed
                    CBZw_NEXT(ed);
                    MOV32w(x1, u8);
                    CALL(const_getsegmentbase, -1);
                }
            }
            break;
        case 0x8F:
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("POP FS:Ed");
                    if(MODREG) {
                        POP1z(TO_NAT((nextop & 7) + (rex.b << 3)));
                    } else {
                        POP1z(x2); // so this can handle POP [ESP] and maybe some variant too
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0, 0, rex, NULL, 0, 0);
                        if(ed==xRSP) {
                            STRz_REGz(x2, x4, ed);
                        } else {
                            // complicated to just allow a segfault that can be recovered correctly
                            SUBz_U12(xRSP, xRSP, rex.is32bits?4:8);
                            STRz_REGz(x2, x4, ed);
                            ADDz_U12(xRSP, xRSP, rex.is32bits?4:8);
                        }
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x90:
            INST_NAME("NOP");
            break;

        case 0x9D:
            INST_NAME("POPF");
            SETFLAGS(X_ALL, SF_SET);
            POP1z(xFlags);
            MOV32w(x1, 0x3F7FD7);
            ANDw_REG(xFlags, xFlags, x1);
            MOV32w(x1, 0x202);
            ORRw_REG(xFlags, xFlags, x1);
            SET_DFNONE();
            if(box64_wine) {    // should this be done all the time?
                TBZ_NEXT(xFlags, F_TF);
                // go to epilog, TF should trigger at end of next opcode, so using Interpreter only
                jump_to_epilog(dyn, addr, 0, ninst);
            }
            break;

        case 0xA1:
            INST_NAME("MOV EAX,FS:Od");
            grab_segdata(dyn, addr, ninst, x4, seg, 0);
            if(rex.is32bits)
                u64 = F32;
            else
                u64 = F64;
            MOV64z(x1, u64);
            LDRxw_REGz(xRAX, x4, x1);
            break;
        case 0xA2:
            INST_NAME("MOV FS:Od,AL");
            grab_segdata(dyn, addr, ninst, x4, seg, 0);
            if(rex.is32bits)
                u64 = F32;
            else
                u64 = F64;
            MOV64z(x1, u64);
            STRB_REGz(xRAX, x4, x1);
            SMWRITE2();
            break;
        case 0xA3:
            INST_NAME("MOV FS:Od,EAX");
            grab_segdata(dyn, addr, ninst, x4, seg, 0);
            if(rex.is32bits)
                u64 = F32;
            else
                u64 = F64;
            MOV64z(x1, u64);
            STRxw_REGz(xRAX, x4, x1);
            SMWRITE2();
            break;

        case 0xA8:
            INST_NAME("TEST AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            UXTBx(x1, xRAX);
            u8 = F8;
            emit_test8c(dyn, ninst, x1, u8, x3, x4, x5);
            break;

        case 0xC6:
            nextop=F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("MOV Seg:Eb, Ib");
                    if(MODREG) {   // reg <= u8
                        u8 = F8;
                        if(!rex.rex) {
                            ed = (nextop&7);
                            eb1 = TO_NAT(ed & 3); // Ax, Cx, Dx or Bx
                            eb2 = (ed&4)>>2;    // L or H
                        } else {
                            eb1 = TO_NAT((nextop & 7) + (rex.b << 3));
                            eb2 = 0;
                        }
                        MOV32w(x3, u8);
                        BFIx(eb1, x3, eb2*8, 8);
                    } else {                    // mem <= u8
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                        u8 = F8;
                        MOV32w(x3, u8);
                        STRB_REGz(x3, x4, ed);
                        SMWRITE2();
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xC7:
            nextop=F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("MOV Seg:Ed, Id");
                    if(MODREG) {   // reg <= i32
                        i64 = F32S;
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        MOV64xw(ed, i64);
                    } else {                    // mem <= i32
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 4);
                        i64 = F32S;
                        MOV64xw(x3, i64);
                        STRxw_REGz(x3, x4, ed);
                        SMWRITE2();
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0xD1:
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x6, seg, (MODREG));
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ed, 1");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET); // removed PENDING on purpose
                    GETEDO(x6, 0);
                    emit_rol32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACKO(x6);
                    break;
                case 1:
                    INST_NAME("ROR Ed, 1");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET); // removed PENDING on purpose
                    GETEDO(x6, 0);
                    emit_ror32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACKO(x6);
                    break;
                case 2:
                    INST_NAME("RCL Ed, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SUBSET); // removed PENDING on purpose
                    GETEDO(x6, 0);
                    emit_rcl32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACKO(x6);
                    break;
                case 3:
                    INST_NAME("RCR Ed, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SUBSET); // removed PENDING on purpose
                    GETEDO(x6, 0);
                    emit_rcr32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACKO(x6);
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETEDO(x6, 0);
                    emit_shl32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACKO(x6);
                    break;
                case 5:
                    INST_NAME("SHR Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETEDO(x6, 0);
                    emit_shr32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACKO(x6);
                    break;
                case 7:
                    INST_NAME("SAR Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETEDO(x6, 0);
                    emit_sar32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACKO(x6);
                    break;
            }
            break;
        case 0xD3:
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x6, seg, (MODREG));
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ed, CL");
                    if(BOX64DRENV(dynarec_safeflags)>1) {
                        READFLAGS(X_OF|X_CF);
                    }
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    if(rex.w) {
                        ANDx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    MOV64xw(x4, (rex.w?64:32));
                    SUBx_REG(x3, x4, x3);
                    GETEDO(x6, 0);
                    if(!rex.w && MODREG) {MOVw_REG(ed, ed);}
                    CBZw_NEXT(x3);
                    IFX2(X_OF, && !BOX64ENV(cputype)) {
                        LSRxw(x4, ed, rex.w?62:30);
                        EORw_REG_LSR(x4, x4, x4, 1);
                        BFIw(xFlags, x4, F_OF, 1);
                    }
                    RORxw_REG(ed, ed, x3);
                    WBACKO(x6);
                    IFX2(X_OF, && BOX64ENV(cputype)) {
                        EORxw_REG_LSR(x4, ed, ed, rex.w?63:31);
                        BFIw(xFlags, x4, F_OF, 1);
                    }
                    IFX(X_CF) {
                        BFIw(xFlags, ed, F_CF, 1);
                    }
                    break;
                case 1:
                    INST_NAME("ROR Ed, CL");
                    if(BOX64DRENV(dynarec_safeflags)>1) {
                        READFLAGS(X_OF|X_CF);
                    }
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    if(rex.w) {
                        ANDx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETEDO(x6, 0);
                    if(!rex.w && MODREG) {MOVw_REG(ed, ed);}
                    CBZw_NEXT(x3);
                    IFX2(X_OF, && !BOX64ENV(cputype)) {
                        EORxw_REG_LSR(x4, ed, ed, rex.w?63:31);
                        BFIw(xFlags, x4, F_OF, 1);
                    }
                    RORxw_REG(ed, ed, x3);
                    WBACKO(x6);
                    IFX2(X_OF, && BOX64ENV(cputype)) {
                        LSRxw(x2, ed, rex.w?62:30); // x2 = d>>30
                        EORw_REG_LSR(x2, x2, x2, 1); // x2 = ((d>>30) ^ ((d>>30)>>1))
                        BFIw(xFlags, x2, F_OF, 1);
                    }
                    IFX(X_CF) {
                        BFXILxw(xFlags, ed, rex.w?63:31, 1);
                    }
                    break;
                case 2:
                    INST_NAME("RCL Ed, CL");
                    MESSAGE(LOG_DUMP, "Need Optimization (RCL Seg:Ed, CL)\n");
                    if(BOX64DRENV(dynarec_safeflags)>1) {
                        READFLAGS(X_OF|X_CF);
                    } else {
                        READFLAGS(X_CF);
                    }
                    SETFLAGS(X_OF|X_CF, SF_SET_DF);
                    if(rex.w) {
                        ANDx_mask(x2, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETEDO(x6, 0);
                    if(wback) {ADDx_REG(x6, x6, wback); wback=x6;}
                    if(!rex.w && MODREG) {MOVw_REG(ed, ed);}
                    CBZw_NEXT(x2);
                    CALL_(rex.w?const_rcl64:const_rcl32, ed, x6);
                    WBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ed, CL");
                    MESSAGE(LOG_DUMP, "Need Optimization (RCR Seg:Ed, CL)\n");
                    if(BOX64DRENV(dynarec_safeflags)>1) {
                        READFLAGS(X_OF|X_CF);
                    } else {
                        READFLAGS(X_CF);
                    }
                    SETFLAGS(X_OF|X_CF, SF_SET_DF);
                    if(rex.w) {
                        ANDx_mask(x2, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETEDO(x6, 0);
                    if(wback) {ADDx_REG(x6, x6, wback); wback=x6;}
                    if(!rex.w && MODREG) {MOVw_REG(ed, ed);}
                    CBZw_NEXT(x2);
                    CALL_(rex.w?const_rcr64:const_rcr32, ed, x6);
                    WBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, CL");
                    if(BOX64DRENV(dynarec_safeflags)>1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                    if(rex.w) {
                        ANDx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETEDO(x6, 0);
                    if(!rex.w && MODREG) {MOVw_REG(ed, ed);}
                    CBZw_NEXT(x3);
                    emit_shl32(dyn, ninst, rex, ed, x3, x5, x4);
                    WBACKO(x6);
                    break;
                case 5:
                    INST_NAME("SHR Ed, CL");
                    if(BOX64DRENV(dynarec_safeflags)>1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                    if(rex.w) {
                        ANDx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETEDO(x6, 0);
                    if(!rex.w && MODREG) {MOVw_REG(ed, ed);}
                    CBZw_NEXT(x3);
                    emit_shr32(dyn, ninst, rex, ed, x3, x5, x4);
                    WBACKO(x6);
                    break;
                case 7:
                    INST_NAME("SAR Ed, CL");
                    if(BOX64DRENV(dynarec_safeflags)>1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                    if(rex.w) {
                        ANDx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETEDO(x6, 0);
                    if(!rex.w && MODREG) {MOVw_REG(ed, ed);}
                    CBZw_NEXT(x3);
                    emit_sar32(dyn, ninst, rex, ed, x3, x5, x4);
                    WBACKO(x6);
                    break;
            }
            break;

        case 0xEB:
            // just use regular inconditional jump
            return dynarec64_00(dyn, addr-1, ip, ninst, rex, rep, ok, need_epilog);

        case 0xF7:
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x6, seg, (MODREG));
            switch((nextop>>3)&7) {
                case 0:
                case 1:
                    INST_NAME("TEST Ed, Id");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEDO(x6, 4);
                    i64 = F32S;
                    emit_test32c(dyn, ninst, rex, ed, i64, x3, x4, x5);
                    break;
                case 2:
                    INST_NAME("NOT Ed");
                    GETEDO(x6, 0);
                    MVNxw_REG(ed, ed);
                    WBACKO(x6);
                    break;
                case 3:
                    INST_NAME("NEG Ed");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEDO(x6, 0);
                    emit_neg32(dyn, ninst, rex, ed, x3, x4);
                    WBACKO(x6);
                    break;
                case 4:
                    INST_NAME("MUL EAX, Ed");
                    if(BOX64ENV(dynarec_safeflags) && BOX64ENV(cputype)) {
                        SETFLAGS(X_OF|X_CF, SF_SET);
                    } else {
                        SETFLAGS(X_ALL, SF_SET);
                    }
                    GETEDO(x6, 0);
                    if(rex.w) {
                        if(ed==xRDX) gd=x3; else gd=xRDX;
                        UMULH(gd, xRAX, ed);
                        MULx(xRAX, xRAX, ed);
                        if(gd!=xRDX) {MOVx_REG(xRDX, gd);}
                    } else {
                        UMULL(xRDX, xRAX, ed);  //64 <- 32x32
                        MOVw_REG(xRAX, xRDX);
                        LSRx(xRDX, xRDX, 32);
                    }
                    SET_DFNONE();
                    IFX(X_CF|X_OF) {
                        CMPSxw_U12(xRDX, 0);
                        CSETw(x3, cNE);
                        IFX(X_CF) {
                            BFIw(xFlags, x3, F_CF, 1);
                        }
                        IFX(X_OF) {
                            BFIw(xFlags, x3, F_OF, 1);
                        }
                    }
                    IFX2(X_AF, && !BOX64ENV(cputype)) {BFCw(xFlags, F_AF, 1);}
                    IFX2(X_ZF, && !BOX64ENV(cputype)) {BFCw(xFlags, F_ZF, 1);}
                    IFX2(X_SF, && !BOX64ENV(cputype)) {
                        LSRxw(x3, xRAX, rex.w?63:31);
                        BFIw(xFlags, x3, F_SF, 1);
                    }
                    IFX2(X_PF, && !BOX64ENV(cputype)) emit_pf(dyn, ninst, xRAX, x3);
                    break;
                case 5:
                    INST_NAME("IMUL EAX, Ed");
                    if(BOX64ENV(dynarec_safeflags) && BOX64ENV(cputype)) {
                        SETFLAGS(X_OF|X_CF, SF_SET);
                    } else {
                        SETFLAGS(X_ALL, SF_SET);
                    }
                    GETEDO(x6, 0);
                    if(rex.w) {
                        if(ed==xRDX) gd=x3; else gd=xRDX;
                        SMULH(gd, xRAX, ed);
                        MULx(xRAX, xRAX, ed);
                        if(gd!=xRDX) {MOVx_REG(xRDX, gd);}
                    } else {
                        SMULL(xRDX, xRAX, ed);  //64 <- 32x32
                        MOVw_REG(xRAX, xRDX);
                        LSRx(xRDX, xRDX, 32);
                    }
                    SET_DFNONE();
                    IFX(X_CF|X_OF) {
                        ASRxw(x4, xRAX, rex.w?63:31);
                        CMPSxw_REG(xRDX, x4);
                        CSETw(x3, cNE);
                        IFX(X_CF) {
                            BFIw(xFlags, x3, F_CF, 1);
                        }
                        IFX(X_OF) {
                            BFIw(xFlags, x3, F_OF, 1);
                        }
                    }
                    IFX2(X_AF, && !BOX64ENV(cputype)) {BFCw(xFlags, F_AF, 1);}
                    IFX2(X_ZF, && !BOX64ENV(cputype)) {BFCw(xFlags, F_ZF, 1);}
                    IFX2(X_SF, && !BOX64ENV(cputype)) {
                        LSRxw(x3, xRAX, rex.w?63:31);
                        BFIw(xFlags, x3, F_SF, 1);
                    }
                    IFX2(X_PF, && !BOX64ENV(cputype)) emit_pf(dyn, ninst, xRAX, x3);
                    break;
                case 6:
                    INST_NAME("DIV Ed");
                    SETFLAGS(X_ALL, SF_SET);
                    if(!rex.w) {
                        GETEDO(x6, 0);
                        if(BOX64ENV(dynarec_div0)) {
                            CBNZx_MARK3(ed);
                            GETIP_(ip);
                            STORE_XEMU_CALL(xRIP);
                            CALL_S(const_native_div0, -1);
                            CLEARIP();
                            LOAD_XEMU_CALL(xRIP);
                            jump_to_epilog(dyn, 0, xRIP, ninst);
                            MARK3;
                        }
                        MOVw_REG(x3, xRAX);
                        ORRx_REG_LSL(x3, x3, xRDX, 32);
                        if(MODREG) {
                            MOVw_REG(x4, ed);
                            ed = x4;
                        }
                        UDIVx(x2, x3, ed);
                        MSUBx(x4, x2, ed, xRAX);
                        MOVw_REG(xRAX, x2);
                        MOVw_REG(xRDX, x4);
                    } else {
                        if(ninst
                           && dyn->insts[ninst-1].x64.addr
                           && *(uint8_t*)(dyn->insts[ninst-1].x64.addr)==0x31
                           && *(uint8_t*)(dyn->insts[ninst-1].x64.addr+1)==0xD2) {
                            GETEDO(x6, 0);
                            if(BOX64ENV(dynarec_div0)) {
                                CBNZx_MARK3(ed);
                                GETIP_(ip);
                                STORE_XEMU_CALL(xRIP);
                                CALL_S(const_native_div0, -1);
                                CLEARIP();
                                LOAD_XEMU_CALL(xRIP);
                                jump_to_epilog(dyn, 0, xRIP, ninst);
                                MARK3;
                            }
                            UDIVx(x2, xRAX, ed);
                            MSUBx(xRDX, x2, ed, xRAX);
                            MOVx_REG(xRAX, x2);
                        } else {
                            GETEDO(x6, 0);
                            CBZxw_MARK(xRDX);
                            if(ed!=x1) {MOVx_REG(x1, ed);}
                            CALL(const_div64, -1);
                            B_NEXT_nocond;
                            MARK;
                            if(BOX64ENV(dynarec_div0)) {
                                CBNZx_MARK3(ed);
                                GETIP_(ip);
                                STORE_XEMU_CALL(xRIP);
                                CALL_S(const_native_div0, -1);
                                CLEARIP();
                                LOAD_XEMU_CALL(xRIP);
                                jump_to_epilog(dyn, 0, xRIP, ninst);
                                MARK3;
                            }
                            UDIVx(x2, xRAX, ed);
                            MSUBx(xRDX, x2, ed, xRAX);
                            MOVx_REG(xRAX, x2);
                        }
                    }
                    SET_DFNONE();
                    IFX(X_OF)                         {BFCw(xFlags, F_OF, 1);}
                    IFX(X_CF)                         {BFCw(xFlags, F_CF, 1);}
                    IFX2(X_AF, && !BOX64ENV(cputype)) {BFCw(xFlags, F_AF, 1);}
                    IFX2(X_AF, && BOX64ENV(cputype))  {ORRw_mask(xFlags, xFlags, 28, 0);}   //mask=0x10
                    IFX2(X_ZF, && !BOX64ENV(cputype)) {ORRw_mask(xFlags, xFlags, 26, 0);}   //mask=0x40
                    IFX2(X_ZF, && BOX64ENV(cputype))  {BFCw(xFlags, F_ZF, 1);}
                    IFX(X_SF)                         {BFCw(xFlags, F_SF, 1);}
                    IFX2(X_PF, && !BOX64ENV(cputype)) {ORRw_mask(xFlags, xFlags, 30, 0);}   //mask=0x04
                    IFX2(X_PF, && BOX64ENV(cputype))  {BFCw(xFlags, F_PF, 1);}
                    break;
                case 7:
                    INST_NAME("IDIV Ed");
                    NOTEST(x1);
                    if(!BOX64ENV(dynarec_safeflags)) {
                        SETFLAGS(X_ALL, SF_SET);
                    } else if(BOX64ENV(cputype)) {
                        SETFLAGS(X_SF|X_PF|X_ZF|X_AF, SF_SET);
                    }
                    if(!rex.w) {
                        GETSEDOw(x6, 0);
                        MOVw_REG(x3, xRAX);
                        ORRx_REG_LSL(x3, x3, xRDX, 32);
                        if(BOX64ENV(dynarec_div0)) {
                            CBNZx_MARK3(wb);
                            GETIP_(ip);
                            STORE_XEMU_CALL(xRIP);
                            CALL_S(const_native_div0, -1);
                            CLEARIP();
                            LOAD_XEMU_CALL(xRIP);
                            jump_to_epilog(dyn, 0, xRIP, ninst);
                            MARK3;
                        }
                        SDIVx(x2, x3, wb);
                        MSUBx(x4, x2, wb, x3);
                        MOVw_REG(xRAX, x2);
                        MOVw_REG(xRDX, x4);
                    } else {
                        if(ninst && dyn->insts
                           &&  dyn->insts[ninst-1].x64.addr
                           && *(uint8_t*)(dyn->insts[ninst-1].x64.addr)==0x48
                           && *(uint8_t*)(dyn->insts[ninst-1].x64.addr+1)==0x99) {
                            GETEDO(x6, 0);
                            if(BOX64ENV(dynarec_div0)) {
                                CBNZx_MARK3(ed);
                                GETIP_(ip);
                                STORE_XEMU_CALL(xRIP);
                                CALL_S(const_native_div0, -1);
                                CLEARIP();
                                LOAD_XEMU_CALL(xRIP);
                                jump_to_epilog(dyn, 0, xRIP, ninst);
                                MARK3;
                            }
                            SDIVx(x2, xRAX, ed);
                            MSUBx(xRDX, x2, ed, xRAX);
                            MOVx_REG(xRAX, x2);
                        } else {
                            GETEDO(x6, 0);
                            if(BOX64ENV(dynarec_div0)) {
                                CBNZx_MARK3(ed);
                                GETIP_(ip);
                                STORE_XEMU_CALL(xRIP);
                                CALL_S(const_native_div0, -1);
                                CLEARIP();
                                LOAD_XEMU_CALL(xRIP);
                                jump_to_epilog(dyn, 0, xRIP, ninst);
                                MARK3;
                            }
                            CBZxw_MARK(xRDX);
                            MVNx_REG(x2, xRDX);
                            CBZxw_MARK(x2);
                            if(ed!=x1) {MOVx_REG(x1, ed);}
                            CALL(const_idiv64, -1);
                            B_NEXT_nocond;
                            MARK;
                            SDIVx(x2, xRAX, ed);
                            MSUBx(xRDX, x2, ed, xRAX);
                            MOVx_REG(xRAX, x2);
                        }
                    }
                    if(!BOX64ENV(dynarec_safeflags)) {
                        SET_DFNONE();
                    }
                    IFX2(X_AF, && BOX64ENV(cputype))  {ORRw_mask(xFlags, xFlags, 28, 0);}   //mask=0x10
                    IFX2(X_ZF, && BOX64ENV(cputype))  {BFCw(xFlags, F_ZF, 1);}
                    IFX2(X_SF, && BOX64ENV(cputype))  {BFCw(xFlags, F_SF, 1);}
                    IFX2(X_PF, && BOX64ENV(cputype))  {BFCw(xFlags, F_PF, 1);}
                    break;
            }
            break;

        case 0xFF:
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x6, seg, (MODREG));
            switch((nextop>>3)&7) {
                case 0: // INC Ed
                    INST_NAME("INC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
                    GETEDO(x6, 0);
                    emit_inc32(dyn, ninst, rex, ed, x3, x4);
                    WBACKO(x6);
                    break;
                case 1: //DEC Ed
                    INST_NAME("DEC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
                    GETEDO(x6, 0);
                    emit_dec32(dyn, ninst, rex, ed, x3, x4);
                    WBACKO(x6);
                    break;
                case 2: // CALL Ed
                    INST_NAME("CALL Ed");
                    PASS2IF ((BOX64DRENV(dynarec_safeflags) > 1) || ((ninst && dyn->insts[ninst - 1].x64.set_flags) || ((ninst > 1) && dyn->insts[ninst - 2].x64.set_flags)), 1) {
                        READFLAGS(X_PEND);          // that's suspicious
                    } else {
                        SETFLAGS(X_ALL, SF_SET_NODF);    //Hack to put flag in "don't care" state
                    }
                    GETEDOz(x6, 0);
                    if (BOX64DRENV(dynarec_callret) && BOX64DRENV(dynarec_bigblock) > 1) {
                        BARRIER(BARRIER_FULL);
                    } else {
                        BARRIER(BARRIER_FLOAT);
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    GETIP_(addr);
                    if (BOX64DRENV(dynarec_callret)) {
                        SET_HASCALLRET();
                        // Push actual return address
                        if(addr < (dyn->start+dyn->isize)) {
                            // there is a next...
                            if(BOX64DRENV(dynarec_callret)>1)
                                j64 = CALLRET_GETRET();
                            else
                                j64 = (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->native_size)):0;
                            ADR_S20(x4, j64);
                            MESSAGE(LOG_NONE, "\tCALLRET set return to +%di\n", j64>>2);
                        } else {
                            if(BOX64DRENV(dynarec_callret)>1)
                                j64 = CALLRET_GETRET();
                            else
                                j64 = (dyn->insts)?(GETMARK-(dyn->native_size)):0;
                            ADR_S20(x4, j64);
                            MESSAGE(LOG_NONE, "\tCALLRET set return to +%di\n", j64>>2);
                        }
                        STPx_S7_preindex(x4, xRIP, xSP, -16);
                    }
                    PUSH1z(xRIP);
                    jump_to_next(dyn, 0, ed, ninst, rex.is32bits);
                    if(BOX64DRENV(dynarec_callret)>1) CALLRET_RET();
                    if (BOX64DRENV(dynarec_callret) && addr >= (dyn->start + dyn->isize)) {
                        // jumps out of current dynablock...
                        MARK;
                        j64 = getJumpTableAddress64(addr);
                        if(dyn->need_reloc) AddRelocTable64RetEndBlock(dyn, ninst, addr, STEP);
                        TABLE64_(x4, j64);
                        LDRx_U12(x4, x4, 0);
                        BR(x4);
                    }
                    CLEARIP();
                    break;
                case 4: // JMP Ed
                    INST_NAME("JMP Ed");
                    READFLAGS(X_PEND);
                    BARRIER(BARRIER_FLOAT);
                    GETEDOz(x6, 0);
                    jump_to_next(dyn, 0, ed, ninst, rex.is32bits);
                    *need_epilog = 0;
                    *ok = 0;
                    break;
                case 6: // Push Ed
                    INST_NAME("PUSH Ed");
                    GETEDOz(x6, 0);
                    PUSH1z(ed);
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
