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
#include "../dynablock_private.h"
#include "custommem.h"

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "../dynarec_helper.h"
#include "dynarec_arm64_functions.h"

#define GETGm   gd = ((nextop&0x38)>>3)

uintptr_t dynarec64_67(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed, wback, wb, wb1, wb2, gb1, gb2, eb1, eb2, q0, q1;
    int64_t fixedaddress;
    int unscaled;
    int8_t  i8;
    uint8_t u8;
    int32_t i32;
    int64_t j64, i64;
    int16_t i16;
    uint64_t u64;
    int cacheupd = 0;
    int lock;
    int v0, v1, s0;
    MAYUSE(i32);
    MAYUSE(j64);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(s0);
    MAYUSE(lock);
    MAYUSE(cacheupd);

    if(rex.is32bits) {
        // should do a different file
        DEFAULT;
        return addr;
    }


    while(opcode==0x67) opcode = F8;

    // reset rex after 67
    GETREX();

    rep = 0;
    while((opcode==0xF2) || (opcode==0xF3) || (opcode>=0x40 && opcode<=0x4F)) {
        if((opcode==0xF2) || (opcode==0xF3))
            rep = opcode-0xF1;
        if(opcode>=0x40 && opcode<=0x4F)
            rex.rex = opcode;
        opcode = F8;
    }

    switch(opcode) {
        case 0x00:
            INST_NAME("ADD Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB32(x1, 0);
            GETGB(x2);
            emit_add8(dyn, ninst, x1, x2, x4, x5);
            EBBACK;
            break;
        case 0x01:
            INST_NAME("ADD Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_add32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x02:
            INST_NAME("ADD Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB32(x2, 0);
            GETGB(x1);
            emit_add8(dyn, ninst, x1, x2, x3, x4);
            GBBACK;
            break;
        case 0x03:
            INST_NAME("ADD Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_add32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x04:
            INST_NAME("ADD AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u8 = F8;
            UXTBw(x1, xRAX);
            emit_add8c(dyn, ninst, x1, u8, x3, x4);
            BFIx(xRAX, x1, 0, 8);
            break;
        case 0x05:
            INST_NAME("ADD EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            emit_add32c(dyn, ninst, rex, xRAX, i64, x3, x4, x5);
            break;

        case 0x08:
            INST_NAME("OR Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB32(x1, 0);
            GETGB(x2);
            emit_or8(dyn, ninst, x1, x2, x4, x2);
            EBBACK;
            break;
        case 0x09:
            INST_NAME("OR Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_or32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x0A:
            INST_NAME("OR Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB32(x2, 0);
            GETGB(x1);
            emit_or8(dyn, ninst, x1, x2, x3, x4);
            GBBACK;
            break;
        case 0x0B:
            INST_NAME("OR Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_or32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x0C:
            INST_NAME("OR AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u8 = F8;
            UXTBw(x1, xRAX);
            emit_or8c(dyn, ninst, x1, u8, x3, x4);
            BFIx(xRAX, x1, 0, 8);
            break;
        case 0x0D:
            INST_NAME("OR EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            emit_or32c(dyn, ninst, rex, xRAX, i64, x3, x4);
            break;

        case 0x0F:
            opcode=F8;
            switch(opcode) {

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
                                addr = geted32(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                                VST128(v0, ed, fixedaddress);
                                SMWRITE2();
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;

                case 0x29:
                    INST_NAME("MOVAPS Ex,Gx");
                    nextop = F8;
                    GETG;
                    v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                    if(MODREG) {
                        ed = (nextop&7)+(rex.b<<3);
                        v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                        VMOVQ(v1, v0);
                    } else {
                        addr = geted32(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                        VST128(v0, ed, fixedaddress);
                        SMWRITE2();
                    }
                    break;
        
                case 0x2E:
                    // no special check...
                case 0x2F:
                    switch(rep) {
                        case 0:
                            if(opcode==0x2F) {INST_NAME("COMISS Gx, Ex");} else {INST_NAME("UCOMISS Gx, Ex");}
                            SETFLAGS(X_ALL, SF_SET);
                            nextop = F8;
                            GETGX(v0, 0);
                            if(MODREG) {
                                s0 = sse_get_reg(dyn, ninst, x1, (nextop&7) + (rex.b<<3), 0);
                            } else {
                                s0 = fpu_get_scratch(dyn, ninst);
                                SMREAD();
                                addr = geted32(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                                VLD32(s0, ed, fixedaddress);
                            }
                            IFX(X_CF|X_PF|X_ZF) {
                                FCMPS(v0, s0);
                            }
                            FCOMI(x1, x2);
                            break;
                        default:
                            DEFAULT;
                    }
                    break;

                case 0x38:  /* MAP 0F38 */
                    opcode = F8;
                    switch(opcode) {
                        case 0xF6:
                            switch(rep) {
                                case 2:
                                    INST_NAME("ADOX Gd, Ed");
                                    nextop = F8;
                                    READFLAGS(X_OF);
                                    SETFLAGS(X_OF, SF_SUBSET);
                                    GETED32(0);
                                    GETGD;
                                    MRS_nzcv(x3);
                                    LSRw(x4, xFlags, F_OF);
                                    BFIx(x3, x4, 29, 1); // set C
                                    MSR_nzcv(x3);      // load CC into ARM CF
                                    IFX(X_OF) {
                                        ADCSxw_REG(gd, gd, ed);
                                        CSETw(x3, cCS);
                                        BFIw(xFlags, x3, F_OF, 1);
                                    } else {
                                        ADCxw_REG(gd, gd, ed);
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

                case 0x6F:
                    switch(rep) {
                        case 0:
                            INST_NAME("MOVQ Gm, Em");
                            nextop = F8;
                            GETGm;
                            if(MODREG) {
                                v1 = mmx_get_reg(dyn, ninst, x1, x2, x3, nextop&7); // no rex.b on MMX
                                v0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
                                VMOV(v0, v1);
                            } else {
                                v0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
                                SMREAD();
                                addr = geted32(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                                VLD64(v0, ed, fixedaddress);
                            }
                            break;
                        case 2:
                            INST_NAME("MOVDQU Gx,Ex");
                            nextop = F8;
                            if(MODREG) {
                                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                                GETGX_empty(v0);
                                VMOVQ(v0, v1);
                            } else {
                                GETGX_empty(v0);
                                SMREAD();
                                addr = geted32(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                                VLD128(v0, ed, fixedaddress);
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;

                case 0x7F:
                    switch(rep) {
                        case 0:
                            INST_NAME("MOVQ Em, Gm");
                            nextop = F8;
                            GETGM(v0);
                            if(MODREG) {
                                v1 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, nextop&7);
                                VMOV(v1, v0);
                            } else {
                                addr = geted32(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                                VST64(v0, ed, fixedaddress);
                                SMWRITE();
                            }
                            break;
                        case 2:
                            INST_NAME("MOVDQU Ex, Gx");
                            nextop = F8;
                            GETGX(v0, 0);
                            if(MODREG) {
                                v1 = sse_get_reg_empty(dyn, ninst, x1, nextop&7);
                                VMOVQ(v1, v0);
                            } else {
                                addr = geted32(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                                VST128(v0, ed, fixedaddress);
                                SMWRITE();
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;

                case 0xB6:
                    INST_NAME("MOVZX Gd, Eb");
                    nextop = F8;
                    GETGD;
                    if(MODREG) {
                        if(rex.rex) {
                            eb1 = TO_NAT((nextop & 7) + (rex.b << 3));
                            eb2 = 0;
                        } else {
                            ed = (nextop&7);
                            eb1 = TO_NAT(ed & 3); // Ax, Cx, Dx or Bx
                            eb2 = (ed&4)>>2;    // L or H
                        }
                        UBFXxw(gd, eb1, eb2*8, 8);
                    } else {
                        SMREAD();
                        addr = geted32(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff, 0, rex, NULL, 0, 0);
                        LDB(gd, ed, fixedaddress);
                    }
                    break;
                case 0xB7:
                    INST_NAME("MOVZX Gd, Ew");
                    nextop = F8;
                    GETGD;
                    if(MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        UBFXxw(gd, ed, 0, 16);
                    } else {
                        SMREAD();
                        addr = geted32(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                        LDH(gd, ed, fixedaddress);
                    }
                    break;

                default:
                    DEFAULT;
            }
            break;

        case 0x10:
            INST_NAME("ADC Eb, Gb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB32(x1, 0);
            GETGB(x2);
            emit_adc8(dyn, ninst, x1, x2, x4, x5);
            EBBACK;
            break;
        case 0x11:
            INST_NAME("ADC Ed, Gd");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_adc32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x12:
            INST_NAME("ADC Gb, Eb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB32(x2, 0);
            GETGB(x1);
            emit_adc8(dyn, ninst, x1, x2, x4, x3);
            GBBACK;
            break;
        case 0x13:
            INST_NAME("ADC Gd, Ed");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_adc32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x14:
            INST_NAME("ADC AL, Ib");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u8 = F8;
            UXTBw(x1, xRAX);
            emit_adc8c(dyn, ninst, x1, u8, x3, x4, x5);
            BFIx(xRAX, x1, 0, 8);
            break;
        case 0x15:
            INST_NAME("ADC EAX, Id");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            MOV64xw(x1, i64);
            emit_adc32(dyn, ninst, rex, xRAX, x1, x3, x4);
            break;

        case 0x18:
            INST_NAME("SBB Eb, Gb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB32(x1, 0);
            GETGB(x2);
            emit_sbb8(dyn, ninst, x1, x2, x4, x5);
            EBBACK;
            break;
        case 0x19:
            INST_NAME("SBB Ed, Gd");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_sbb32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x1A:
            INST_NAME("SBB Gb, Eb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB32(x2, 0);
            GETGB(x1);
            emit_sbb8(dyn, ninst, x1, x2, x3, x4);
            GBBACK;
            break;
        case 0x1B:
            INST_NAME("SBB Gd, Ed");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_sbb32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x1C:
            INST_NAME("SBB AL, Ib");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u8 = F8;
            UXTBw(x1, xRAX);
            emit_sbb8c(dyn, ninst, x1, u8, x3, x4, x5);
            BFIx(xRAX, x1, 0, 8);
            break;
        case 0x1D:
            INST_NAME("SBB EAX, Id");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            MOV64xw(x2, i64);
            emit_sbb32(dyn, ninst, rex, xRAX, x2, x3, x4);
            break;

        case 0x20:
            INST_NAME("AND Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB32(x1, 0);
            GETGB(x2);
            emit_and8(dyn, ninst, x1, x2, x4, x5);
            EBBACK;
            break;
        case 0x21:
            INST_NAME("AND Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_and32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x22:
            INST_NAME("AND Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB32(x2, 0);
            GETGB(x1);
            emit_and8(dyn, ninst, x1, x2, x3, x4);
            GBBACK;
            break;
        case 0x23:
            INST_NAME("AND Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_and32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x24:
            INST_NAME("AND AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u8 = F8;
            UXTBw(x1, xRAX);
            emit_and8c(dyn, ninst, x1, u8, x3, x4);
            BFIx(xRAX, x1, 0, 8);
            break;
        case 0x25:
            INST_NAME("AND EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            emit_and32c(dyn, ninst, rex, xRAX, i64, x3, x4);
            break;

        case 0x28:
            INST_NAME("SUB Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB32(x1, 0);
            GETGB(x2);
            emit_sub8(dyn, ninst, x1, x2, x4, x5);
            EBBACK;
            break;
        case 0x29:
            INST_NAME("SUB Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_sub32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x2A:
            INST_NAME("SUB Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB32(x2, 0);
            GETGB(x1);
            emit_sub8(dyn, ninst, x1, x2, x3, x4);
            GBBACK;
            break;
        case 0x2B:
            INST_NAME("SUB Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_sub32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x2C:
            INST_NAME("SUB AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u8 = F8;
            UXTBw(x1, xRAX);
            emit_sub8c(dyn, ninst, x1, u8, x3, x4, x5);
            BFIx(xRAX, x1, 0, 8);
            break;
        case 0x2D:
            INST_NAME("SUB EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            emit_sub32c(dyn, ninst, rex, xRAX, i64, x3, x4, x5);
            break;

        case 0x30:
            INST_NAME("XOR Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB32(x1, 0);
            GETGB(x2);
            emit_xor8(dyn, ninst, x1, x2, x4, x5);
            EBBACK;
            break;
        case 0x31:
            INST_NAME("XOR Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_xor32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x32:
            INST_NAME("XOR Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB32(x2, 0);
            GETGB(x1);
            emit_xor8(dyn, ninst, x1, x2, x3, x4);
            GBBACK;
            break;
        case 0x33:
            INST_NAME("XOR Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_xor32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x34:
            INST_NAME("XOR AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u8 = F8;
            UXTBw(x1, xRAX);
            emit_xor8c(dyn, ninst, x1, u8, x3, x4);
            BFIx(xRAX, x1, 0, 8);
            break;
        case 0x35:
            INST_NAME("XOR EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            emit_xor32c(dyn, ninst, rex, xRAX, i64, x3, x4);
            break;

        case 0x38:
            INST_NAME("CMP Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB32(x1, 0);
            GETGB(x2);
            emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5);
            break;
        case 0x39:
            INST_NAME("CMP Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_cmp32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            break;
        case 0x3A:
            INST_NAME("CMP Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB32(x2, 0);
            GETGB(x1);
            emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5);
            break;
        case 0x3B:
            INST_NAME("CMP Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_cmp32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;
        case 0x3C:
            INST_NAME("CMP AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u8 = F8;
            UXTBw(x1, xRAX);
            if(u8) {
                MOV32w(x2, u8);
                emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5);
            } else {
                emit_cmp8_0(dyn, ninst, x1, x3, x4);
            }
            break;
        case 0x3D:
            INST_NAME("CMP EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            if(i64) {
                MOV64xw(x2, i64);
                emit_cmp32(dyn, ninst, rex, xRAX, x2, x3, x4, x5);
            } else
                emit_cmp32_0(dyn, ninst, rex, xRAX, x3, x4);
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
            // just use regular conditional jump
            return dynarec64_00(dyn, addr-1, ip, ninst, rex, rep, ok, need_epilog);

        case 0x63:
            INST_NAME("MOVSXD Gd, Ed");
            nextop = F8;
            GETGD;
            if(rex.w) {
                if(MODREG) {   // reg <= reg
                    SXTWx(gd, TO_NAT((nextop & 7) + (rex.b << 3)));
                } else {                    // mem <= reg
                    SMREAD();
                    addr = geted32(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                    LDSW(gd, ed, fixedaddress);
                }
            } else {
                if(MODREG) {   // reg <= reg
                    MOVw_REG(gd, TO_NAT((nextop & 7) + (rex.b << 3)));
                } else {                    // mem <= reg
                    SMREAD();
                    addr = geted32(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                    LDW(gd, ed, fixedaddress);
                }
            }
            break;

        case 0x66:
            opcode=F8;
            GETREX();
            switch(opcode) {

                case 0x0F:
                    nextop = F8;
                    switch(nextop) {
                        case 0x6F:
                            INST_NAME("MOVDQA Gx,Ex");
                            nextop = F8;
                            if(MODREG) {
                                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                                GETGX_empty(v0);
                                VMOVQ(v0, v1);
                            } else {
                                GETGX_empty(v0);
                                SMREAD();
                                addr = geted32(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                                VLD128(v0, ed, fixedaddress);
                            }
                            break;

                        case 0x76:
                            INST_NAME("PCMPEQD Gx,Ex");
                            nextop = F8;
                            GETGX(v0, 1);
                            if(MODREG) {
                                q0 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                            } else {
                                q0 = fpu_get_scratch(dyn, ninst);
                                addr = geted32(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                                VLD128(q0, ed, fixedaddress);
                            }
                            VCMEQQ_32(v0, v0, q0);
                            break;

                        case 0x7E:
                            INST_NAME("MOVD Ed,Gx");
                            nextop = F8;
                            GETGX(v0, 0);
                            if(rex.w) {
                                if(MODREG) {
                                    ed = TO_NAT((nextop & 0x07) + (rex.b << 3));
                                    VMOVQDto(ed, v0, 0);
                                } else {
                                    addr = geted32(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                                    VST64(v0, ed, fixedaddress);
                                    SMWRITE2();
                                }
                            } else {
                                if(MODREG) {
                                    ed = TO_NAT((nextop & 0x07) + (rex.b << 3));
                                    VMOVSto(ed, v0, 0);
                                } else {
                                    addr = geted32(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                                    VST32(v0, ed, fixedaddress);
                                    SMWRITE2();
                                }
                            }
                            break;

                        case 0xD6:
                            INST_NAME("MOVQ Ex, Gx");
                            nextop = F8;
                            GETGX(v0, 0);
                            if(MODREG) {
                                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop&7) + (rex.b<<3));
                                FMOVD(v1, v0);
                            } else {
                                addr = geted32(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                                VST64(v0, ed, fixedaddress);
                                SMWRITE2();
                            }
                            break;

                        case 0xEF:
                            INST_NAME("PXOR Gx,Ex");
                            nextop = F8;
                            GETG;
                            if(MODREG && ((nextop&7)+(rex.b<<3)==gd)) {
                                // special case for PXOR Gx, Gx
                                q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                                VEORQ(q0, q0, q0);
                            } else {
                                q0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                                if(MODREG) {
                                    q1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                                } else {
                                    q1 = fpu_get_scratch(dyn, ninst);
                                    addr = geted32(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                                    VLD128(q1, ed, fixedaddress);
                                }
                                VEORQ(q0, q0, q1);
                            }
                            break;

                        case 0xFE:
                            INST_NAME("PADDD Gx,Ex");
                            nextop = F8;
                            GETGX(v0, 1);
                            GETEX32(v1, 0, 0);
                            VADDQ_32(v0, v0, v1);
                            break;

                            default:
                                DEFAULT;
                    }
                    break;

                    case 0x81:
                    case 0x83:
                        nextop = F8;
                        switch((nextop>>3)&7) {
                            case 0: //ADD
                                if(opcode==0x81) {INST_NAME("ADD Ew, Iw");} else {INST_NAME("ADD Ew, Ib");}
                                SETFLAGS(X_ALL, SF_SET_PENDING);
                                GETEW32(x1, (opcode==0x81)?2:1);
                                if(opcode==0x81) i16 = F16S; else i16 = F8S;
                                MOVZw(x5, i16);
                                emit_add16(dyn, ninst, ed, x5, x2, x4);
                                EWBACK;
                                break;
                            case 1: //OR
                                if(opcode==0x81) {INST_NAME("OR Ew, Iw");} else {INST_NAME("OR Ew, Ib");}
                                SETFLAGS(X_ALL, SF_SET_PENDING);
                                GETEW32(x1, (opcode==0x81)?2:1);
                                if(opcode==0x81) i16 = F16S; else i16 = F8S;
                                emit_or16c(dyn, ninst, x1, i16, x2, x4);
                                EWBACK;
                                break;
                            case 2: //ADC
                                if(opcode==0x81) {INST_NAME("ADC Ew, Iw");} else {INST_NAME("ADC Ew, Ib");}
                                READFLAGS(X_CF);
                                SETFLAGS(X_ALL, SF_SET_PENDING);
                                GETEW32(x1, (opcode==0x81)?2:1);
                                if(opcode==0x81) i16 = F16S; else i16 = F8S;
                                MOVZw(x5, i16);
                                emit_adc16(dyn, ninst, x1, x5, x2, x4);
                                EWBACK;
                                break;
                            case 3: //SBB
                                if(opcode==0x81) {INST_NAME("SBB Ew, Iw");} else {INST_NAME("SBB Ew, Ib");}
                                READFLAGS(X_CF);
                                SETFLAGS(X_ALL, SF_SET_PENDING);
                                GETEW32(x1, (opcode==0x81)?2:1);
                                if(opcode==0x81) i16 = F16S; else i16 = F8S;
                                MOVZw(x5, i16);
                                emit_sbb16(dyn, ninst, x1, x5, x2, x4);
                                EWBACK;
                                break;
                            case 4: //AND
                                if(opcode==0x81) {INST_NAME("AND Ew, Iw");} else {INST_NAME("AND Ew, Ib");}
                                SETFLAGS(X_ALL, SF_SET_PENDING);
                                GETEW32(x1, (opcode==0x81)?2:1);
                                if(opcode==0x81) i16 = F16S; else i16 = F8S;
                                emit_and16c(dyn, ninst, x1, i16, x2, x4);
                                EWBACK;
                                break;
                            case 5: //SUB
                                if(opcode==0x81) {INST_NAME("SUB Ew, Iw");} else {INST_NAME("SUB Ew, Ib");}
                                SETFLAGS(X_ALL, SF_SET_PENDING);
                                GETEW32(x1, (opcode==0x81)?2:1);
                                if(opcode==0x81) i16 = F16S; else i16 = F8S;
                                MOVZw(x5, i16);
                                emit_sub16(dyn, ninst, x1, x5, x2, x4);
                                EWBACK;
                                break;
                            case 6: //XOR
                                if(opcode==0x81) {INST_NAME("XOR Ew, Iw");} else {INST_NAME("XOR Ew, Ib");}
                                SETFLAGS(X_ALL, SF_SET_PENDING);
                                GETEW32(x1, (opcode==0x81)?2:1);
                                if(opcode==0x81) i16 = F16S; else i16 = F8S;
                                emit_xor16c(dyn, ninst, x1, i16, x2, x4);
                                EWBACK;
                                break;
                            case 7: //CMP
                                if(opcode==0x81) {INST_NAME("CMP Ew, Iw");} else {INST_NAME("CMP Ew, Ib");}
                                SETFLAGS(X_ALL, SF_SET_PENDING);
                                GETEW32(x1, (opcode==0x81)?2:1);
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
                    INST_NAME("MOV Ew, Gw");
                    nextop = F8;
                    GETGD;  // don't need GETGW here
                    if(MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        if(ed!=gd) {
                            BFIx(ed, gd, 0, 16);
                        }
                    } else {
                        addr = geted32(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff << 1, 1, rex, &lock, 0, 0);
                        STH(gd, ed, fixedaddress);
                        SMWRITELOCK(lock);
                    }
                    break;

                case 0x8D:
                    INST_NAME("LEA Gw, Ed");
                    nextop=F8;
                    //GETGW(x1);
                    gd = x1;    // no need to read
                    if(MODREG) {   // reg <= reg? that's an invalid operation
                        DEFAULT;
                    } else {                    // mem <= reg
                        addr = geted32(dyn, addr, ninst, nextop, &ed, gd, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                        if(ed!=gd)
                            gd = ed;
                    }
                    GWBACK;
                    break;

                default:
                    DEFAULT;
            }
            break;

        case 0x6B:
            INST_NAME("IMUL Gd, Ed, Ib");
            if(BOX64ENV(dynarec_safeflags)>1 && BOX64ENV(cputype)) {
                SETFLAGS(X_OF|X_CF, SF_SET);
            } else {
                SETFLAGS(X_ALL, SF_SET);
            }
            nextop = F8;
            GETGD;
            GETED32(1);
            i64 = F8S;
            MOV64xw(x4, i64);
            if(rex.w) {
                // 64bits imul
                UFLAG_IF {
                    SMULH(x3, ed, x4);
                    MULx(gd, ed, x4);
                    SET_DFNONE();
                    IFX(X_CF | X_OF) {
                        CMPSx_REG_ASR(x3, gd, 63);
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
                        CMPSw_REG_ASR(x3, gd, 31);
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

        case 0x81:
        case 0x83:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: //ADD
                    if(opcode==0x81) {INST_NAME("ADD Ed, Id");} else {INST_NAME("ADD Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED32((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_add32c(dyn, ninst, rex, ed, i64, x3, x4, x5);
                    WBACK;
                    break;
                case 1: //OR
                    if(opcode==0x81) {INST_NAME("OR Ed, Id");} else {INST_NAME("OR Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED32((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_or32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACK;
                    break;
                case 2: //ADC
                    if(opcode==0x81) {INST_NAME("ADC Ed, Id");} else {INST_NAME("ADC Ed, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED32((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    MOV64xw(x5, i64);
                    emit_adc32(dyn, ninst, rex, ed, x5, x3, x4);
                    WBACK;
                    break;
                case 3: //SBB
                    if(opcode==0x81) {INST_NAME("SBB Ed, Id");} else {INST_NAME("SBB Ed, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED32((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    MOV64xw(x5, i64);
                    emit_sbb32(dyn, ninst, rex, ed, x5, x3, x4);
                    WBACK;
                    break;
                case 4: //AND
                    if(opcode==0x81) {INST_NAME("AND Ed, Id");} else {INST_NAME("AND Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED32((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_and32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACK;
                    break;
                case 5: //SUB
                    if(opcode==0x81) {INST_NAME("SUB Ed, Id");} else {INST_NAME("SUB Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED32((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_sub32c(dyn, ninst, rex, ed, i64, x3, x4, x5);
                    WBACK;
                    break;
                case 6: //XOR
                    if(opcode==0x81) {INST_NAME("XOR Ed, Id");} else {INST_NAME("XOR Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED32((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_xor32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACK;
                    break;
                case 7: //CMP
                    if(opcode==0x81) {INST_NAME("CMP Ed, Id");} else {INST_NAME("CMP Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED32((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    if(i64) {
                        MOV64xw(x2, i64);
                        emit_cmp32(dyn, ninst, rex, ed, x2, x3, x4, x5);
                    } else
                        emit_cmp32_0(dyn, ninst, rex, ed, x3, x4);
                    break;
            }
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
                addr = geted32(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff, 0, rex, &lock, 0, 0);
                STB(gd, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0x89:
            INST_NAME("MOV Ed, Gd");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                MOVxw_REG(TO_NAT((nextop & 7) + (rex.b << 3)), gd);
            } else {                    // mem <= reg
                addr = geted32(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff << (2 + rex.w), (1 << (2 + rex.w)) - 1, rex, &lock, 0, 0);
                STxw(gd, ed, fixedaddress);
                SMWRITELOCK(lock);
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
                addr = geted32(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff, 0, rex, &lock, 0, 0);
                SMREADLOCK(lock);
                LDB(x4, wback, fixedaddress);
                ed = x4;
            }
            BFIx(gb1, ed, gb2, 8);
            break;
        case 0x8B:
            INST_NAME("MOV Gd, Ed");
            nextop=F8;
            GETGD;
            if(MODREG) {
                MOVxw_REG(gd, TO_NAT((nextop & 7) + (rex.b << 3)));
            } else {
                addr = geted32(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, &lock, 0, 0);
                SMREADLOCK(lock);
                LDxw(gd, ed, fixedaddress);
            }
            break;

        case 0x8D:
            INST_NAME("LEA Gd, Ed");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg? that's an invalid operation
                DEFAULT;
            } else {                    // mem <= reg
                addr = geted32(dyn, addr, ninst, nextop, &ed, gd, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                MOVw_REG(gd, ed);   // need to trucate to 32bits as it's 67 prefixed
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
            if(gd==xRAX) {
                INST_NAME("NOP");
            } else {
                INST_NAME("XCHG EAX, Reg");
                MOVxw_REG(x2, xRAX);
                MOVxw_REG(xRAX, gd);
                MOVxw_REG(gd, x2);
            }
            break;

        case 0x9C:
            INST_NAME("PUSHF");
            READFLAGS(X_ALL);
            PUSH1z(xFlags);
            break;

        case 0xA1:
            INST_NAME("MOV EAX,Od");
            u64 = F32;
            MOV64z(x1, u64);
            lock=isLockAddress(u64);
            SMREADLOCK(lock);
            LDRxw_U12(xRAX, x1, 0);
            break;

        case 0xA9:
            INST_NAME("TEST EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            emit_test32c(dyn, ninst, rex, xRAX, i64, x3, x4, x5);
            break;

        case 0xC1:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ed, Ib");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET); // removed PENDING on purpose
                    GETED32(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    emit_rol32c(dyn, ninst, rex, ed, u8, x3, x4);
                    if(u8) { WBACK; }
                    break;
                case 1:
                    INST_NAME("ROR Ed, Ib");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET); // removed PENDING on purpose
                    GETED32(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    emit_ror32c(dyn, ninst, rex, ed, u8, x3, x4);
                    if(u8) { WBACK; }
                    break;
                case 2:
                    INST_NAME("RCL Ed, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SUBSET); // removed PENDING on purpose
                    GETED32(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    emit_rcl32c(dyn, ninst, rex, ed, u8, x3, x4);
                    if(u8) { WBACK; }
                    break;
                case 3:
                    INST_NAME("RCR Ed, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SUBSET); // removed PENDING on purpose
                    GETED32(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    emit_rcr32c(dyn, ninst, rex, ed, u8, x3, x4);
                    if(u8) { WBACK; }
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, Ib");
                    GETED32(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    if(u8) {    //should create the geted32_ib utility function for that
                        SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                        emit_shl32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    }
                    break;
                case 5:
                    INST_NAME("SHR Ed, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETED32(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    emit_shr32c(dyn, ninst, rex, ed, u8, x3, x4);
                    if(u8) {
                        WBACK;
                    }
                    break;
                case 7:
                    INST_NAME("SAR Ed, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETED32(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    emit_sar32c(dyn, ninst, rex, ed, u8, x3, x4);
                    if(u8) {
                        WBACK;
                    }
                    break;
            }
            break;

        case 0xC4:
            nextop = F8;
            if(rex.is32bits && !(MODREG)) {
                DEFAULT;
            } else {
                if(rex.is32bits) {
                    DEFAULT;
                    return addr;
                }
                vex_t vex = {0};
                vex.rex = rex;
                u8 = nextop;
                vex.m = u8&0b00011111;
                vex.rex.b = (u8&0b00100000)?0:1;
                vex.rex.x = (u8&0b01000000)?0:1;
                vex.rex.r = (u8&0b10000000)?0:1;
                u8 = F8;
                vex.p = u8&0b00000011;
                vex.l = (u8>>2)&1;
                vex.v = ((~u8)>>3)&0b1111;
                vex.rex.w = (u8>>7)&1;
                addr = dynarec64_67_AVX(dyn, addr, ip, ninst, vex, ok, need_epilog);
            }
            break;
        case 0xC5:
            nextop = F8;
            if(rex.is32bits && !(MODREG)) {
                DEFAULT;
            } else {
                if(rex.is32bits) {
                    DEFAULT;
                    return addr;
                }
                vex_t vex = {0};
                vex.rex = rex;
                u8 = nextop;
                vex.p = u8&0b00000011;
                vex.l = (u8>>2)&1;
                vex.v = ((~u8)>>3)&0b1111;
                vex.rex.r = (u8&0b10000000)?0:1;
                vex.rex.b = 0;
                vex.rex.x = 0;
                vex.rex.w = 0;
                vex.m = VEX_M_0F;
                addr = dynarec64_67_AVX(dyn, addr, ip, ninst, vex, ok, need_epilog);
            }
            break;
        case 0xC6:
            INST_NAME("MOV Eb, Ib");
            nextop=F8;
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
                addr = geted32(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, &unscaled, 0xfff, 0, rex, &lock, 0, 1);
                u8 = F8;
                if(u8) {
                    MOV32w(x3, u8);
                    ed = x3;
                } else
                    ed = xZR;
                STB(ed, wback, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0xC7:
            INST_NAME("MOV Ed, Id");
            nextop=F8;
            if(MODREG) {   // reg <= i32
                i64 = F32S;
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                MOV64xw(ed, i64);
            } else {                    // mem <= i32
                addr = geted32(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, &lock, 0, 4);
                i64 = F32S;
                MOV64xw(x3, i64);
                STxw(x3, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;


        #define GO(NO, YES)                                             \
            JUMP(addr+i8, 1);                                           \
            if(dyn->insts[ninst].x64.jmp_insts==-1 ||                   \
                CHECK_CACHE()) {                                        \
                /* out of the block */                                  \
                i32 = dyn->insts[ninst].epilog-(dyn->native_size);      \
                Bcond(NO, i32);                                         \
                if(dyn->insts[ninst].x64.jmp_insts==-1) {               \
                    if(!(dyn->insts[ninst].x64.barrier&BARRIER_FLOAT))  \
                        fpu_purgecache(dyn, ninst, 1, x1, x2, x3);      \
                    jump_to_next(dyn, addr+i8, 0, ninst, rex.is32bits); \
                } else {                                                \
                    CacheTransform(dyn, ninst, cacheupd);               \
                    i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);\
                    SKIP_SEVL(i32);                                     \
                    B(i32);                                             \
                }                                                       \
            } else {                                                    \
                /* inside the block */                                  \
                i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);    \
                SKIP_SEVL(i32);                                         \
                Bcond(YES, i32);                                        \
            }
        case 0xE0:
            INST_NAME("LOOPNZ (32bits)");
            READFLAGS(X_ZF);
            i8 = F8S;
            MOVw_REG(x1, xRCX);
            SUBw_U12(x1, x1, 1);
            BFIx(xRCX, x1, 0, 32);
            CBZw_NEXT(x1);    // ECX is 0, no LOOP
            TSTw_mask(xFlags, 0b011010, 0); //mask=0x40
            GO(cNE, cEQ);
            break;
        case 0xE1:
            INST_NAME("LOOPZ (32bits)");
            READFLAGS(X_ZF);
            i8 = F8S;
            MOVw_REG(x1, xRCX);
            SUBw_U12(x1, x1, 1);
            BFIx(xRCX, x1, 0, 32);
            CBZw_NEXT(x1);    // ECX is 0, no LOOP
            TSTw_mask(xFlags, 0b011010, 0); //mask=0x40
            GO(cEQ, cNE);
            break;
        case 0xE2:
            INST_NAME("LOOP (32bits)");
            i8 = F8S;
            MOVw_REG(x1, xRCX);
            SUBSw_U12(x1, x1, 1);
            BFIx(xRCX, x1, 0, 32);
            GO(cEQ, cNE);
            break;
        case 0xE3:
            INST_NAME("JECXZ");
            i8 = F8S;
            MOVw_REG(x1, xRCX);
            TSTw_REG(x1, x1);
            GO(cNE, cEQ);
            break;
        #undef GO

        case 0xE8:
            return dynarec64_00(dyn, addr-1, ip, ninst, rex, rep, ok, need_epilog); // addr-1, to "put back" opcode)

        case 0xF7:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                case 1:
                    INST_NAME("TEST Ed, Id");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED32H(x1, 4);
                    i64 = F32S;
                    emit_test32c(dyn, ninst, rex, ed, i64, x3, x4, x5);
                    break;
                case 2:
                    INST_NAME("NOT Ed");
                    GETED32(0);
                    MVNxw_REG(ed, ed);
                    WBACK;
                    break;
                case 3:
                    INST_NAME("NEG Ed");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED32(0);
                    emit_neg32(dyn, ninst, rex, ed, x3, x4);
                    WBACK;
                    break;
                case 4:
                    INST_NAME("MUL EAX, Ed");
                    if(BOX64ENV(dynarec_safeflags) && BOX64ENV(cputype)) {
                        SETFLAGS(X_OF|X_CF, SF_SET);
                    } else {
                        SETFLAGS(X_ALL, SF_SET);
                    }
                    GETED32(0);
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
                    GETED32(0);
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
                        CMPSxw_REG_ASR(xRDX, xRAX, rex.w?63:31);
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
                        GETED32(0);
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
                        if(ninst && dyn->insts
                           && dyn->insts[ninst-1].x64.addr
                           && *(uint8_t*)(dyn->insts[ninst-1].x64.addr)==0x31
                           && *(uint8_t*)(dyn->insts[ninst-1].x64.addr+1)==0xD2) {
                            GETED32(0);
                            UDIVx(x2, xRAX, ed);
                            MSUBx(xRDX, x2, ed, xRAX);
                            MOVx_REG(xRAX, x2);
                        } else {
                            GETED32H(x1, 0);  // get edd changed addr, so cannot be called 2 times for same op...
                            CBZxw_MARK(xRDX);
                            if(ed!=x1) {MOVx_REG(x1, ed);}
                            CALL(const_div64, -1);
                            B_NEXT_nocond;
                            MARK;
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
                        GETSED32w(0);
                        MOVw_REG(x3, xRAX);
                        ORRx_REG_LSL(x3, x3, xRDX, 32);
                        SDIVx(x2, x3, wb);
                        MSUBx(x4, x2, wb, x3);
                        MOVw_REG(xRAX, x2);
                        MOVw_REG(xRDX, x4);
                    } else {
                        if(ninst && dyn->insts
                           &&  dyn->insts[ninst-1].x64.addr
                           && *(uint8_t*)(dyn->insts[ninst-1].x64.addr)==0x48
                           && *(uint8_t*)(dyn->insts[ninst-1].x64.addr+1)==0x99) {
                            GETED32(0);
                            SDIVx(x2, xRAX, ed);
                            MSUBx(xRDX, x2, ed, xRAX);
                            MOVx_REG(xRAX, x2);
                        } else {
                            GETED32H(x1, 0);  // get edd changed addr, so cannot be called 2 times for same op...
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
            switch((nextop>>3)&7) {
                case 0: // INC Ed
                    INST_NAME("INC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
                    GETED32(0);
                    emit_inc32(dyn, ninst, rex, ed, x3, x4);
                    WBACK;
                    break;
                case 1: //DEC Ed
                    INST_NAME("DEC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
                    GETED32(0);
                    emit_dec32(dyn, ninst, rex, ed, x3, x4);
                    WBACK;
                    break;
                case 2: // CALL Ed
                    INST_NAME("CALL Ed");
                    PASS2IF((BOX64DRENV(dynarec_safeflags)>1) ||
                        ((ninst && dyn->insts[ninst-1].x64.set_flags)
                        || ((ninst>1) && dyn->insts[ninst-2].x64.set_flags)), 1)
                    {
                        READFLAGS(X_PEND);          // that's suspicious
                    } else {
                        SETFLAGS(X_ALL, SF_SET_NODF);    //Hack to put flag in "don't care" state
                    }
                    GETED32(0);
                    if(BOX64DRENV(dynarec_callret) && BOX64DRENV(dynarec_bigblock)>1) {
                        BARRIER(BARRIER_FULL);
                    } else {
                        BARRIER(BARRIER_FLOAT);
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    GETIP_(addr);
                    if(BOX64DRENV(dynarec_callret)) {
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
                    if(BOX64DRENV(dynarec_callret) && addr >= (dyn->start + dyn->isize)) {
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
                default:
                    DEFAULT;
                    return addr;
            }
            break;

        default:
            DEFAULT;
    }
    return addr;
}
