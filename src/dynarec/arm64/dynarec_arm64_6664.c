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

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "../dynarec_helper.h"
#include "dynarec_arm64_functions.h"

uintptr_t dynarec64_6664(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int seg, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed, wb1, wback;
    int64_t j64;
    int16_t i16;
    int v0, v1;
    int64_t fixedaddress;
    int unscaled;
    MAYUSE(j64);

    GETREX();

    /*if(rex.w && opcode!=0x0f) {   // rex.w cancels "66", but not for 66 0f type of prefix
        MESSAGE(LOG_DUMP, "Here!\n");
        return dynarec64_64(dyn, addr-2, ip, ninst, rex, rep, ok, need_epilog);
    }*/

    switch(opcode) {

        case 0x0F:
            opcode = F8;
            switch(opcode) {
                case 0x2E:
                    // no special check...
                case 0x2F:
                    if(opcode==0x2F) {INST_NAME("COMISD Gx, Ex");} else {INST_NAME("UCOMISD Gx, Ex");}
                    SETFLAGS(X_ALL, SF_SET);
                    nextop = F8;
                    GETG;
                    v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                    if(MODREG) {
                        v1 = sse_get_reg(dyn, ninst, x1, (nextop&7) + (rex.b<<3), 0);
                    } else {
                        grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                        ADDz_REG(x4, x4, ed);
                        ed = x4;
                        v1 = fpu_get_scratch(dyn, ninst);
                        VLD64(v1, ed, fixedaddress);
                    }
                    IFX(X_CF|X_PF|X_ZF) {
                        FCMPD(v0, v1);
                    }
                    FCOMI(x1, x2);
                    break;

                case 0x6F:
                    INST_NAME("MOVDQA Gx,Ex");
                    nextop = F8;
                    if(MODREG) {
                        v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                        GETGX_empty(v0);
                        VMOVQ(v0, v1);
                    } else {
                        GETGX_empty(v0);
                        grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                        SMREAD();
                        ADDz_REG(x4, x4, ed);
                        VLD128(v0, x4, fixedaddress);
                    }
                    break;

                case 0x7F:
                    INST_NAME("MOVDQA Ex,Gx");
                    nextop = F8;
                    GETGX(v0, 0);
                    if(MODREG) {
                        v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 1);
                        VMOVQ(v1, v0);
                    } else {
                        grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                        ADDz_REG(x4, x4, ed);
                        VST128(v0, x4, fixedaddress);
                        SMWRITE2();
                    }
                    break;

                case 0xD6:
                    INST_NAME("MOVQ Ex, Gx");
                    nextop = F8;
                    GETG;
                    v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                    if(MODREG) {
                        v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop&7) + (rex.b<<3));
                        FMOVD(v1, v0);
                    } else {
                        grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                        ADDz_REG(x4, x4, ed);
                        VST64(v0, x4, fixedaddress);
                        SMWRITE();
                    }
                    break;

                default:
                    DEFAULT;
            }
            break;

        case 0x83:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: //ADD
                    if(opcode==0x81) {INST_NAME("ADD Ew, Iw");} else {INST_NAME("ADD Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    GETEWO(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    MOVZw(x5, i16);
                    emit_add16(dyn, ninst, ed, x5, x2, x4);
                    EWBACK;
                    break;
                case 1: //OR
                    if(opcode==0x81) {INST_NAME("OR Ew, Iw");} else {INST_NAME("OR Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    GETEWO(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    emit_or16c(dyn, ninst, x1, i16, x2, x4);
                    EWBACK;
                    break;
                case 2: //ADC
                    if(opcode==0x81) {INST_NAME("ADC Ew, Iw");} else {INST_NAME("ADC Ew, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    GETEWO(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    MOVZw(x5, i16);
                    emit_adc16(dyn, ninst, x1, x5, x2, x4);
                    EWBACK;
                    break;
                case 3: //SBB
                    if(opcode==0x81) {INST_NAME("SBB Ew, Iw");} else {INST_NAME("SBB Ew, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    GETEWO(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    MOVZw(x5, i16);
                    emit_sbb16(dyn, ninst, x1, x5, x2, x4);
                    EWBACK;
                    break;
                case 4: //AND
                    if(opcode==0x81) {INST_NAME("AND Ew, Iw");} else {INST_NAME("AND Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    GETEWO(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    emit_and16c(dyn, ninst, x1, i16, x2, x4);
                    EWBACK;
                    break;
                case 5: //SUB
                    if(opcode==0x81) {INST_NAME("SUB Ew, Iw");} else {INST_NAME("SUB Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    GETEWO(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    MOVZw(x5, i16);
                    emit_sub16(dyn, ninst, x1, x5, x2, x4);
                    EWBACK;
                    break;
                case 6: //XOR
                    if(opcode==0x81) {INST_NAME("XOR Ew, Iw");} else {INST_NAME("XOR Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    GETEWO(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    emit_xor16c(dyn, ninst, x1, i16, x2, x4);
                    EWBACK;
                    break;
                case 7: //CMP
                    if(opcode==0x81) {INST_NAME("CMP Ew, Iw");} else {INST_NAME("CMP Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    GETEWO(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    if(i16) {
                        MOVZw(x2, i16);
                        emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5);
                    } else
                        emit_cmp16_0(dyn, ninst, x1, x3, x4);
                    break;
            }
            break;
            
        case 0x89:
            INST_NAME("MOV FS:Ew, Gw");
            nextop = F8;
            GETGD;  // don't need GETGW here
            if(MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                if(rex.w) {
                    MOVx_REG(ed, gd);
                } else {
                    if(ed!=gd) {
                        BFIx(ed, gd, 0, 16);
                    }
                }
            } else {
                grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<(rex.w?3:1), rex.w?7:1, rex, NULL, 0, 0);
                ADDz_REG(x4, x4, ed);
                ed = x4;
                if(rex.w) {
                    STx(gd, ed, fixedaddress);
                } else {
                    STH(gd, ed, fixedaddress);
                }
                SMWRITE();
            }
            break;

        case 0x8B:
            INST_NAME("MOV Gw, FS:Ew");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                if(rex.w) {
                    MOVx_REG(gd, ed);
                } else {
                    if(ed!=gd) {
                        BFIx(gd, ed, 0, 16);
                    }
                }
            } else {                    // mem <= reg
                grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<(rex.w?3:1), rex.w?7:1, rex, NULL, 0, 0);
                ADDz_REG(x4, x4, ed);
                ed = x4;
                if(rex.w) {
                    LDx(gd, ed, fixedaddress);
                } else {
                    LDH(x1, ed, fixedaddress);
                    BFIx(gd, x1, 0, 16);
                }
            }
            break;


        default:
            DEFAULT;
    }
    return addr;
}
