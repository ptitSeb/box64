#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "custommem.h"

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_helper.h"
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
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop = F8;
            GETGD;
            GETEDO(x4, 0);
            emit_add32(dyn, ninst, rex, ed, gd, x3, x5);
            WBACKO(x4);
            break;
        case 0x03:
            INST_NAME("ADD Gd, Seg:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop = F8;
            GETGD;
            GETEDO(x4, 0);
            emit_add32(dyn, ninst, rex, gd, ed, x3, x4);
            break;

        case 0x0B:
            INST_NAME("OR Gd, Seg:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop = F8;
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
                                grab_segdata(dyn, addr, ninst, x4, seg);
                                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                                SMREAD();
                                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                                ADDx_REG(x4, x4, ed);
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
                                grab_segdata(dyn, addr, ninst, x4, seg);
                                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                                SMREAD();
                                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                                ADDx_REG(x4, x4, ed);
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
                            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                            if(MODREG) {
                                ed = (nextop&7)+(rex.b<<3);
                                v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                                VMOVQ(v1, v0);
                            } else {
                                grab_segdata(dyn, addr, ninst, x4, seg);
                                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                                ADDx_REG(x4, x4, ed);
                                VST128(v0, x4, fixedaddress);
                                SMWRITE2();
                            }
                            break;
                        case 1:
                            INST_NAME("MOVSD Ex, Gx");
                            nextop = F8;
                            GETG;
                            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                            if(MODREG) {
                                ed = (nextop&7)+ (rex.b<<3);
                                d0 = sse_get_reg(dyn, ninst, x1, ed, 1);
                                VMOVeD(d0, 0, v0, 0);
                            } else {
                                grab_segdata(dyn, addr, ninst, x4, seg);
                                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                                ADDx_REG(x4, x4, ed);
                                VST64(v0, x4, fixedaddress);
                                SMWRITE2();
                            }
                            break;
                        case 2:
                            INST_NAME("MOVSS Ex, Gx");
                            nextop = F8;
                            GETG;
                            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                            if(MODREG) {
                                ed = (nextop&7)+ (rex.b<<3);
                                q0 = sse_get_reg(dyn, ninst, x1, ed, 1);
                                VMOVeS(q0, 0, v0, 0);
                            } else {
                                grab_segdata(dyn, addr, ninst, x4, seg);
                                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                                ADDx_REG(x4, x4, ed);
                                VST32(v0, x4, fixedaddress);
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
                            v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                            if(MODREG) {
                                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                                VMOVQ(v0, v1);
                            } else {
                                grab_segdata(dyn, addr, ninst, x4, seg);
                                SMREAD();
                                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                                ADDx_REG(x4, x4, ed);
                                VLD128(v0, ed, fixedaddress);
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
                            SETFLAGS(X_ALL, SF_PENDING);
                            nextop = F8;
                            grab_segdata(dyn, addr, ninst, x4, seg);
                            GETGD;
                            GETEDO(x4, 0);
                            if(rex.w) {
                                // 64bits imul
                                UFLAG_IF {
                                    SMULH(x3, gd, ed);
                                    MULx(gd, gd, ed);
                                    UFLAG_OP1(x3);
                                    UFLAG_RES(gd);
                                    UFLAG_DF(x3, d_imul64);
                                } else {
                                    MULxw(gd, gd, ed);
                                }
                            } else {
                                // 32bits imul
                                UFLAG_IF {
                                    SMULL(gd, gd, ed);
                                    UFLAG_RES(gd);
                                    LSRx(x3, gd, 32);
                                    UFLAG_OP1(x3);
                                    MOVw_REG(gd, gd);
                                    UFLAG_DF(x3, d_imul32);
                                } else {
                                    MULxw(gd, gd, ed);
                                }
                            }
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
                            grab_segdata(dyn, addr, ninst, x4, seg);
                            GETGD;
                            if(MODREG) {
                                if(rex.rex) {
                                    eb1 = xRAX+(nextop&7)+(rex.b<<3);
                                    eb2 = 0;                \
                                } else {
                                    ed = (nextop&7);
                                    eb1 = xRAX+(ed&3);  // Ax, Cx, Dx or Bx
                                    eb2 = (ed&4)>>2;    // L or H
                                }
                                UBFXxw(gd, eb1, eb2*8, 8);
                            } else {
                                SMREAD();
                                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                                LDRB_REG(gd, ed, x4);
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
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop = F8;
            GETGD;
            GETEDO(x4, 0);
            emit_adc32(dyn, ninst, rex, gd, ed, x3, x5);
            break;

        case 0x21:
            INST_NAME("AND Seg:Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop = F8;
            GETGD;
            GETEDO(x4, 0);
            emit_and32(dyn, ninst, rex, ed, gd, x3, x5);
            WBACKO(x4);
            break;

        case 0x23:
            INST_NAME("AND Gd, Seg:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop = F8;
            GETGD;
            GETEDO(x4, 0);
            emit_and32(dyn, ninst, rex, gd, ed, x3, x5);
            break;

        case 0x29:
            INST_NAME("SUB Seg:Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop = F8;
            GETGD;
            GETEDO(x4, 0);
            emit_sub32(dyn, ninst, rex, ed, gd, x3, x5);
            WBACKO(x4);
            break;

        case 0x2B:
            INST_NAME("SUB Gd, Seg:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop = F8;
            GETGD;
            GETEDO(x4, 0);
            emit_sub32(dyn, ninst, rex, gd, ed, x3, x4);
            break;

        case 0x31:
            INST_NAME("XOR Seg:Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop = F8;
            GETGD;
            GETEDO(x4, 0);
            emit_xor32(dyn, ninst, rex, ed, gd, x3, x5);
            WBACKO(x4);
            break;

        case 0x33:
            INST_NAME("XOR Gd, Seg:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop = F8;
            GETGD;
            GETEDO(x4, 0);
            emit_xor32(dyn, ninst, rex, gd, ed, x3, x4);
            break;

        case 0x39:
            INST_NAME("CMP Seg:Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop = F8;
            GETGD;
            GETEDO(x4, 0);
            emit_cmp32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            break;

        case 0x3B:
            INST_NAME("CMP Gd, Seg:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop = F8;
            GETGD;
            GETEDO(x4, 0);
            emit_cmp32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;

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
                        SXTWx(gd, xRAX+(nextop&7)+(rex.b<<3));
                    } else {                    // mem <= reg
                        grab_segdata(dyn, addr, ninst, x4, seg);
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                        LDRSW_REG(gd, ed, x4);
                    }
                } else {
                    if(MODREG) {   // reg <= reg
                        MOVw_REG(gd, xRAX+(nextop&7)+(rex.b<<3));
                    } else {                    // mem <= reg
                        grab_segdata(dyn, addr, ninst, x4, seg);
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                        LDRw_REG(gd, ed, x4);
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
                DEFAULT;
            }
            break;

        case 0x80:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: //ADD
                    INST_NAME("ADD Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_add8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK;
                    break;
                case 1: //OR
                    INST_NAME("OR Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_or8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK;
                    break;
                case 2: //ADC
                    INST_NAME("ADC Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg);
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_adc8c(dyn, ninst, x1, u8, x2, x4, x5);
                    EBBACK;
                    break;
                case 3: //SBB
                    INST_NAME("SBB Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg);
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_sbb8c(dyn, ninst, x1, u8, x2, x4, x5);
                    EBBACK;
                    break;
                case 4: //AND
                    INST_NAME("AND Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_and8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK;
                    break;
                case 5: //SUB
                    INST_NAME("SUB Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_sub8c(dyn, ninst, x1, u8, x2, x4, x5);
                    EBBACK;
                    break;
                case 6: //XOR
                    INST_NAME("XOR Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_xor8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK;
                    break;
                case 7: //CMP
                    INST_NAME("CMP Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg);
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
            grab_segdata(dyn, addr, ninst, x6, seg);
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
        case 0x8A:
            INST_NAME("MOV Gb, Eb");
            nextop = F8;
            if(rex.rex) {
                gb1 = gd = xRAX+((nextop&0x38)>>3)+(rex.r<<3);
                gb2=0;
            } else {
                gd = (nextop&0x38)>>3;
                gb1 = xRAX+(gd&3);
                gb2 = ((gd&4)<<1);
            }
            if(MODREG) {
                if(rex.rex) {
                    wback = xRAX+(nextop&7)+(rex.b<<3);
                    wb2 = 0;
                } else {
                    wback = (nextop&7);
                    wb2 = (wback>>2);
                    wback = xRAX+(wback&3);
                }
                if(wb2) {
                    UBFXw(x4, wback, wb2*8, 8);
                    ed = x4;
                } else {
                    ed = wback;
                }
            } else {
                grab_segdata(dyn, addr, ninst, x4, seg);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                LDRB_REG(x4, wback, x4);
                ed = x4;
            }
            BFIx(gb1, ed, gb2, 8);
            break;
        case 0x89:
            INST_NAME("MOV Seg:Ed, Gd");
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                MOVxw_REG(xRAX+(nextop&7)+(rex.b<<3), gd);
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                STRxw_REG(gd, ed, x4);
                SMWRITE2();
            }
            break;

        case 0x8B:
            INST_NAME("MOV Gd, Seg:Ed");
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                MOVxw_REG(gd, xRAX+(nextop&7)+(rex.b<<3));
            } else {                    // mem <= reg
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                LDRxw_REG(gd, ed, x4);
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
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop = F8;
            u8 = (nextop&0x38)>>3;
            if((nextop&0xC0)==0xC0) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                LDRH_REG(x1, wback, x4);
                ed = x1;
            }
            STRH_U12(ed, xEmu, offsetof(x64emu_t, segs[u8]));
            STRw_U12(wZR, xEmu, offsetof(x64emu_t, segs_serial[u8]));
            break;
        case 0x8F:
            INST_NAME("POP FS:Ed");
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop = F8;
            if(MODREG) {
                POP1z(xRAX+(nextop&7)+(rex.b<<3));
            } else {
                POP1z(x2); // so this can handle POP [ESP] and maybe some variant too
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0, 0, rex, NULL, 0, 0);
                if(ed==xRSP) {
                    STRz_REG(x2, ed, x4);
                } else {
                    // complicated to just allow a segfault that can be recovered correctly
                    SUBz_U12(xRSP, xRSP, rex.is32bits?4:8);
                    STRz_REG(x2, ed, x4);
                    ADDz_U12(xRSP, xRSP, rex.is32bits?4:8);
                }
            }
            break;
        case 0x90:
            INST_NAME("NOP");
            break;

        case 0xA1:
            INST_NAME("MOV EAX,FS:Od");
            grab_segdata(dyn, addr, ninst, x4, seg);
            if(rex.is32bits)
                u64 = F32;
            else
                u64 = F64;
            MOV64z(x1, u64);
            LDRxw_REG(xRAX, x1, x4);
            break;

        case 0xA3:
            INST_NAME("MOV FS:Od,EAX");
            grab_segdata(dyn, addr, ninst, x4, seg);
            if(rex.is32bits)
                u64 = F32;
            else
                u64 = F64;
            MOV64z(x1, u64);
            STRxw_REG(xRAX, x1, x4);
            SMWRITE2();
            break;

        case 0xC6:
            INST_NAME("MOV Seg:Eb, Ib");
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop=F8;
            if(MODREG) {   // reg <= u8
                u8 = F8;
                if(!rex.rex) {
                    ed = (nextop&7);
                    eb1 = xRAX+(ed&3);  // Ax, Cx, Dx or Bx
                    eb2 = (ed&4)>>2;    // L or H
                } else {
                    eb1 = xRAX+(nextop&7)+(rex.b<<3);
                    eb2 = 0;
                }
                MOV32w(x3, u8);
                BFIx(eb1, x3, eb2*8, 8);
            } else {                    // mem <= u8
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                u8 = F8;
                MOV32w(x3, u8);
                STRB_REG(x3, ed, x4);
                SMWRITE2();
            }
            break;
        case 0xC7:
            INST_NAME("MOV Seg:Ed, Id");
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop=F8;
            if(MODREG) {   // reg <= i32
                i64 = F32S;
                ed = xRAX+(nextop&7)+(rex.b<<3);
                MOV64xw(ed, i64);
            } else {                    // mem <= i32
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 4);
                i64 = F32S;
                MOV64xw(x3, i64);
                STRxw_REG(x3, ed, x4);
                SMWRITE2();
            }
            break;

        case 0xD1:
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x6, seg);
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ed, 1");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET_PENDING);
                    GETEDO(x6, 0);
                    emit_rol32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACKO(x6);
                    break;
                case 1:
                    INST_NAME("ROR Ed, 1");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET_PENDING);
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
            grab_segdata(dyn, addr, ninst, x6, seg);
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ed, CL");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    if(rex.w) {
                        ANDSx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDSw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    MOV64xw(x4, (rex.w?64:32));
                    SUBx_REG(x3, x4, x3);
                    GETEDO(x6, 0);
                    if(!rex.w && MODREG) {MOVw_REG(ed, ed);}
                    B_NEXT(cEQ);
                    RORxw_REG(ed, ed, x3);
                    WBACKO(x6);
                    UFLAG_IF {  // calculate flags directly
                        CMPSw_U12(x3, rex.w?63:31);
                        B_MARK(cNE);
                            LSRxw(x4, ed, rex.w?63:31);
                            ADDxw_REG(x4, x4, ed);
                            BFIw(xFlags, x4, F_OF, 1);
                        MARK;
                        BFIw(xFlags, ed, F_CF, 1);
                        UFLAG_DF(x2, d_none);
                    }
                    break;
                case 1:
                    INST_NAME("ROR Ed, CL");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    if(rex.w) {
                        ANDSx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDSw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETEDO(x6, 0);
                    if(!rex.w && MODREG) {MOVw_REG(ed, ed);}
                    B_NEXT(cEQ);
                    RORxw_REG(ed, ed, x3);
                    WBACKO(x6);
                    UFLAG_IF {  // calculate flags directly
                        CMPSw_U12(x3, 1);
                        B_MARK(cNE);
                            LSRxw(x2, ed, rex.w?62:30); // x2 = d>>30
                            EORw_REG_LSR(x2, x2, x2, 1); // x2 = ((d>>30) ^ ((d>>30)>>1))
                            BFIw(xFlags, x2, F_OF, 1);
                        MARK;
                        LSRxw(x2, ed, rex.w?63:31);
                        BFIw(xFlags, x2, F_CF, 1);
                        UFLAG_DF(x2, d_none);
                    }
                    break;
                case 2:
                    INST_NAME("RCL Ed, CL");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET_DF);
                    if(rex.w) {
                        ANDSx_mask(x2, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDSw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETEDO(x6, 0);
                    if(wback) {ADDx_REG(x6, x6, wback); wback=x6;}
                    if(!rex.w && MODREG) {MOVw_REG(ed, ed);}
                    B_NEXT(cEQ);
                    CALL_(rex.w?((void*)rcl64):((void*)rcl32), ed, x6);
                    WBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ed, CL");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET_DF);
                    if(rex.w) {
                        ANDSx_mask(x2, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDSw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETEDO(x6, 0);
                    if(wback) {ADDx_REG(x6, x6, wback); wback=x6;}
                    if(!rex.w && MODREG) {MOVw_REG(ed, ed);}
                    B_NEXT(cEQ);
                    CALL_(rex.w?((void*)rcr64):((void*)rcr32), ed, x6);
                    WBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    if(rex.w) {
                        ANDSx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDSw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETEDO(x6, 0);
                    if(!rex.w && MODREG) {MOVw_REG(ed, ed);}
                    B_NEXT(cEQ);
                    emit_shl32(dyn, ninst, rex, ed, x3, x5, x4);
                    WBACKO(x6);
                    break;
                case 5:
                    INST_NAME("SHR Ed, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    if(rex.w) {
                        ANDSx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDSw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETEDO(x6, 0);
                    if(!rex.w && MODREG) {MOVw_REG(ed, ed);}
                    B_NEXT(cEQ);
                    emit_shr32(dyn, ninst, rex, ed, x3, x5, x4);
                    WBACKO(x6);
                    break;
                case 7:
                    INST_NAME("SAR Ed, CL");
                    SETFLAGS(X_ALL, SF_PENDING);
                    if(rex.w) {
                        ANDSx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDSw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETEDO(x6, 0);
                    if(!rex.w && MODREG) {MOVw_REG(ed, ed);}
                    B_NEXT(cEQ);
                    UFLAG_OP12(ed, x3);
                    ASRxw_REG(ed, ed, x3);
                    WBACKO(x6);
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, rex.w?d_sar64:d_sar32);
                    break;
            }
            break;

        case 0xF7:
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x6, seg);
            switch((nextop>>3)&7) {
                case 0:
                case 1:
                    INST_NAME("TEST Ed, Id");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEDO(x6, 4);
                    i64 = F32S;
                    MOV64xw(x2, i64);
                    emit_test32(dyn, ninst, rex, ed, x2, x3, x4, x5);
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
                    SETFLAGS(X_ALL, SF_PENDING);
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
                    UFLAG_RES(xRAX);
                    UFLAG_OP1(xRDX);
                    UFLAG_DF(x2, rex.w?d_mul64:d_mul32);
                    break;
                case 5:
                    INST_NAME("IMUL EAX, Ed");
                    SETFLAGS(X_ALL, SF_PENDING);
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
                    UFLAG_RES(xRAX);
                    UFLAG_OP1(xRDX);
                    UFLAG_DF(x2, rex.w?d_imul64:d_imul32);
                    break;
                case 6:
                    INST_NAME("DIV Ed");
                    SETFLAGS(X_ALL, SF_SET);
                    SET_DFNONE(x2);
                    if(!rex.w) {
                        GETEDO(x6, 0);
                        if(box64_dynarec_div0) {
                            CBNZx_MARK3(ed);
                            GETIP_(ip);
                            STORE_XEMU_CALL(xRIP);
                            CALL(native_div0, -1);
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
                            if(box64_dynarec_div0) {
                                CBNZx_MARK3(ed);
                                GETIP_(ip);
                                STORE_XEMU_CALL(xRIP);
                                CALL(native_div0, -1);
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
                            CALL(div64, -1);
                            B_NEXT_nocond;
                            MARK;
                            if(box64_dynarec_div0) {
                                CBNZx_MARK3(ed);
                                GETIP_(ip);
                                STORE_XEMU_CALL(xRIP);
                                CALL(native_div0, -1);
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
                    break;
                case 7:
                    INST_NAME("IDIV Ed");
                    NOTEST(x1);
                    SETFLAGS(X_ALL, SF_SET);
                    SET_DFNONE(x2)
                    if(!rex.w) {
                        GETSEDOw(x6, 0);
                        MOVw_REG(x3, xRAX);
                        ORRx_REG_LSL(x3, x3, xRDX, 32);
                        if(box64_dynarec_div0) {
                            CBNZx_MARK3(wb);
                            GETIP_(ip);
                            STORE_XEMU_CALL(xRIP);
                            CALL(native_div0, -1);
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
                            if(box64_dynarec_div0) {
                                CBNZx_MARK3(ed);
                                GETIP_(ip);
                                STORE_XEMU_CALL(xRIP);
                                CALL(native_div0, -1);
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
                            if(box64_dynarec_div0) {
                                CBNZx_MARK3(ed);
                                GETIP_(ip);
                                STORE_XEMU_CALL(xRIP);
                                CALL(native_div0, -1);
                                CLEARIP();
                                LOAD_XEMU_CALL(xRIP);
                                jump_to_epilog(dyn, 0, xRIP, ninst);
                                MARK3;
                            }
                            CBZxw_MARK(xRDX);
                            MVNx_REG(x2, xRDX);
                            CBZxw_MARK(x2);
                            if(ed!=x1) {MOVx_REG(x1, ed);}
                            CALL((void*)idiv64, -1);
                            B_NEXT_nocond;
                            MARK;
                            SDIVx(x2, xRAX, ed);
                            MSUBx(xRDX, x2, ed, xRAX);
                            MOVx_REG(xRAX, x2);
                        }
                    }
                    break;
            }
            break;

        case 0xFF:
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x6, seg);
            switch((nextop>>3)&7) {
                case 0: // INC Ed
                    INST_NAME("INC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    GETEDO(x6, 0);
                    emit_inc32(dyn, ninst, rex, ed, x3, x4);
                    WBACKO(x6);
                    break;
                case 1: //DEC Ed
                    INST_NAME("DEC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    GETEDO(x6, 0);
                    emit_dec32(dyn, ninst, rex, ed, x3, x4);
                    WBACKO(x6);
                    break;
                case 2: // CALL Ed
                    INST_NAME("CALL Ed");
                    PASS2IF((box64_dynarec_safeflags>1) ||
                        ((ninst && dyn->insts[ninst-1].x64.set_flags)
                        || ((ninst>1) && dyn->insts[ninst-2].x64.set_flags)), 1)
                    {
                        READFLAGS(X_PEND);          // that's suspicious
                    } else {
                        SETFLAGS(X_ALL, SF_SET_NODF);    //Hack to put flag in "don't care" state
                    }
                    GETEDOz(x6, 0);
                    if(box64_dynarec_callret && box64_dynarec_bigblock>1) {
                        BARRIER(BARRIER_FULL);
                        BARRIER_NEXT(BARRIER_FULL);
                    } else {
                        BARRIER(BARRIER_FLOAT);
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    GETIP_(addr);
                    if(box64_dynarec_callret) {
                        SET_HASCALLRET();
                        // Push actual return address
                        if(addr < (dyn->start+dyn->isize)) {
                            // there is a next...
                            j64 = (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->native_size)):0;
                            ADR_S20(x4, j64);
                            MESSAGE(LOG_NONE, "\tCALLRET set return to +%di\n", j64>>2);
                        } else {
                            MESSAGE(LOG_NONE, "\tCALLRET set return to Jmptable(%p)\n", (void*)addr);
                            j64 = getJumpTableAddress64(addr);
                            TABLE64(x4, j64);
                            LDRx_U12(x4, x4, 0);
                        }
                        STPx_S7_preindex(x4, xRIP, xSP, -16);
                    }
                    PUSH1z(xRIP);
                    jump_to_next(dyn, 0, ed, ninst, rex.is32bits);
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
