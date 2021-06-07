#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
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
#include "dynarec_arm64.h"
#include "dynarec_arm64_private.h"
#include "arm64_printer.h"

#include "dynarec_arm64_helper.h"
#include "dynarec_arm64_functions.h"


uintptr_t dynarec64_F0(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip; (void)rep; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2, gb1, gb2;
    int32_t i32;
    int64_t i64, j64;
    int64_t fixedaddress;
    MAYUSE(gb1);
    MAYUSE(gb2);
    MAYUSE(wb1);
    MAYUSE(wb2);
    MAYUSE(j64);

    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }
    // REX prefix before the F0 are ignored
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }

    switch(opcode) {
        case 0x00:
            INST_NAME("LOCK ADD Eb, Gb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGB(x2);
            if((nextop&0xC0)==0xC0) {
                if(rex.rex) {
                    wback = xRAX + (nextop&7) + (rex.b<<3);
                    wb2 = 0;
                } else {
                    wback = (nextop&7);    
                    wb2 = (wback>>2);      
                    wback = xRAX+(wback&3);
                }
                UBFXw(x1, wback, wb2*8, 8);   
                emit_add8(dyn, ninst, x1, x2, x4, x3);
                BFIx(wback, x1, wb2*8, 8);
            } else {                   
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0, 0, rex, 0, 0);
                MARKLOCK;
                LDAXRB(x1, wback);
                emit_add8(dyn, ninst, x1, x2, x4, x3);
                STLXRB(x4, x1, wback);
                CBNZx_MARKLOCK(x4);
            }
            break;
        case 0x01:
            INST_NAME("LOCK ADD Ed, Gd");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            if((nextop&0xC0)==0xC0) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
                emit_add32(dyn, ninst, rex, ed, gd, x3, x4);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, rex, 0, 0);
                MARKLOCK;
                LDAXRxw(x1, wback);
                emit_add32(dyn, ninst, rex, x1, gd, x3, x4);
                STLXRxw(x3, x1, wback);
                CBNZx_MARKLOCK(x3);
            }
            break;

        case 0x0F:
            nextop = F8;
            switch(nextop) {

                case 0xB1:
                    INST_NAME("LOCK CMPXCHG Ed, Gd");
                    SETFLAGS(X_ALL, SF_SET);
                    nextop = F8;
                    GETGD;
                    if(MODREG) {
                        ed = xRAX+(nextop&7)+(rex.b<<3);
                        wback = 0;
                        UFLAG_IF {emit_cmp32(dyn, ninst, rex, xRAX, ed, x3, x4, x5);}
                        MOVxw_REG(x1, ed);  // save value
                        CMPSxw_REG(xRAX, x1);
                        B_MARK2(cNE);
                        MOVxw_REG(ed, gd);
                        MARK2;
                        MOVxw_REG(xRAX, x1);
                        B_NEXT_nocond;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, rex, 0, 0);
                        TSTx_mask(wback, 1, 0, 1+rex.w);    // mask=3 or 7
                        B_MARK3(cNE);
                        // Aligned version
                        MARKLOCK;
                        LDAXRxw(x1, wback);
                        CMPSxw_REG(xRAX, x1);
                        B_MARK(cNE);
                        // EAX == Ed
                        STLXRxw(x4, gd, wback);
                        CBNZx_MARKLOCK(x4);
                        // done
                        B_MARK_nocond;
                        // Unaligned version
                        MARK3;
                        LDRxw_U12(x1, wback, 0);
                        LDAXRB(x3, wback); // dummy read, to arm the write...
                        CMPSxw_REG(xRAX, x1);
                        B_MARK(cNE);
                        // EAX == Ed
                        STLXRB(x4, gd, wback);
                        CBNZx_MARK3(x4);
                        STRxw_U12(gd, wback, 0);
                        MARK;
                        // Common part (and fallback for EAX != Ed)
                        UFLAG_IF {emit_cmp32(dyn, ninst, rex, xRAX, x1, x3, x4, x5);}
                        MOVxw_REG(xRAX, x1);    // upper par of RAX will be erase on 32bits, no mater what
                    }
                    break;

                case 0xC1:
                    INST_NAME("LOCK XADD Gd, Ed");
                    SETFLAGS(X_ALL, SF_SET);
                    nextop = F8;
                    GETGD;
                    if(MODREG) {
                        ed = xRAX+(nextop&7)+(rex.b<<3);
                        MOVxw_REG(x1, ed);
                        MOVxw_REG(ed, gd);
                        MOVxw_REG(gd, x1);
                        emit_add32(dyn, ninst, rex, ed, gd, x3, x4);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, rex, 0, 0);
                        TSTx_mask(wback, 1, 0, 1+rex.w);    // mask=3 or 7
                        B_MARK(cNE);    // unaligned
                        MARKLOCK;
                        LDAXRxw(x1, wback);
                        ADDxw_REG(x4, x1, gd);
                        STLXRxw(x3, x4, wback);
                        CBNZx_MARKLOCK(x3);
                        B_MARK2_nocond;
                        MARK;
                        LDRxw_U12(x1, wback, 0);
                        LDAXRB(x4, wback);
                        BFIxw(x1, x4, 0, 8);
                        ADDxw_REG(x4, x1, gd);
                        STLXRB(x3, x4, wback);
                        CBNZx_MARK(x3);
                        STRxw_U12(x4, wback, 0);
                        MARK2;
                        IFX(X_ALL|X_PEND) {
                            MOVxw_REG(x2, x1);
                            emit_add32(dyn, ninst, rex, x2, gd, x3, x4);
                        }
                        MOVxw_REG(gd, x1);
                    }
                    break;

                case 0xC7:
                    INST_NAME("LOCK CMPXCHG8B Gq, Eq");
                    SETFLAGS(X_ZF, SF_SET);
                    nextop = F8;
                    addr = geted(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, 0, 0, rex, 0, 0);
                    MARKLOCK;
                    LDAXPxw(x2, x3, wback);
                    CMPSxw_REG(xRAX, x2);
                    B_MARK(cNE);    // EAX != Ed[0]
                    CMPSxw_REG(xRDX, x3);
                    B_MARK(cNE);    // EDX != Ed[1]
                    STLXPxw(x4, xRBX, xRCX, wback);
                    CBNZx_MARKLOCK(x4);
                    MOV32w(x1, 1);
                    B_MARK3_nocond;
                    MARK;
                    MOVxw_REG(xRAX, x2);
                    MOVxw_REG(xRDX, x3);
                    MOV32w(x1, 0);
                    MARK3;
                    BFIw(xFlags, x1, F_ZF, 1);
                    break;

                default:
                    DEFAULT;
            }
            break;
                    
        case 0x29:
            INST_NAME("LOCK SUB Ed, Gd");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
                emit_sub32(dyn, ninst, rex, ed, gd, x3, x4);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, rex, 0, 0);
                MARKLOCK;
                LDAXRxw(x1, wback);
                emit_sub32(dyn, ninst, rex, x1, gd, x3, x4);
                STLXRxw(x3, x1, wback);
                CBNZx_MARKLOCK(x3);
            }
            break;

        case 0x66:
            opcode = F8;
            switch(opcode) {
                case 0x81:
                case 0x83:
                    nextop = F8;
                    switch((nextop>>3)&7) {
                        case 0: //ADD
                            if(opcode==0x81) {
                                INST_NAME("LOCK ADD Ew, Iw");
                            } else {
                                INST_NAME("LOCK ADD Ew, Iw");
                            }
                            SETFLAGS(X_ALL, SF_SET);
                            if(MODREG) {
                                if(opcode==0x81) i32 = F16S; else i32 = F8S;
                                ed = xRAX+(nextop&7)+(rex.b<<3);
                                MOV32w(x5, i32);
                                UXTHw(x6, ed);
                                emit_add16(dyn, ninst, x6, x5, x3, x4);
                                BFIx(ed, x6, 0, 16);
                            } else {
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, rex, 0, (opcode==0x81)?2:1);
                                if(opcode==0x81) i32 = F32S; else i32 = F8S;
                                MOV32w(x5, i32);
                                TSTx_mask(wback, 1, 0, 0);    // mask=1
                                B_MARK(cNE);
                                MARKLOCK;
                                LDAXRH(x1, wback);
                                emit_add16(dyn, ninst, x1, x5, x3, x4);
                                STLXRH(x3, x1, wback);
                                CBNZx_MARKLOCK(x3);
                                B_NEXT_nocond;
                                MARK;   // unaligned! also, not enough 
                                LDRH_U12(x1, wback, 0);
                                LDAXRB(x4, wback);
                                BFIw(x1, x4, 0, 8); // re-inject
                                emit_add16(dyn, ninst, x1, x5, x3, x4);
                                STLXRB(x3, x1, wback);
                                CBNZx_MARK(x3);
                                STRH_U12(x1, wback, 0);    // put the whole value
                            }
                            break;
                        case 1: //OR
                            if(opcode==0x81) {INST_NAME("LOCK OR Ew, Iw");} else {INST_NAME("LOCK OR Ew, Iw");}
                            SETFLAGS(X_ALL, SF_SET);
                            if(MODREG) {
                                if(opcode==0x81) i32 = F16S; else i32 = F8S;
                                ed = xRAX+(nextop&7)+(rex.b<<3);
                                MOV32w(x5, i32);
                                UXTHw(x6, ed);
                                emit_or16(dyn, ninst, x6, x5, x3, x4);
                                BFIx(ed, x6, 0, 16);
                            } else {
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, rex, 0, (opcode==0x81)?2:1);
                                if(opcode==0x81) i32 = F16S; else i32 = F8S;
                                MOV32w(x5, i32);
                                MARKLOCK;
                                LDAXRH(x1, wback);
                                emit_or16(dyn, ninst, x1, x5, x3, x4);
                                STLXRH(x3, x1, wback);
                                CBNZx_MARKLOCK(x3);
                            }
                            break;
                        case 2: //ADC
                            if(opcode==0x81) {INST_NAME("LOCK ADC Ew, Iw");} else {INST_NAME("LOCK ADC Ew, Ib");}
                            READFLAGS(X_CF);
                            SETFLAGS(X_ALL, SF_SET);
                            if(MODREG) {
                                if(opcode==0x81) i32 = F16S; else i32 = F8S;
                                ed = xRAX+(nextop&7)+(rex.b<<3);
                                MOV32w(x5, i32);
                                UXTHw(x6, ed);
                                emit_adc16(dyn, ninst, x6, x5, x3, x4);
                                BFIx(ed, x6, 0, 16);
                            } else {
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, rex, 0, (opcode==0x81)?2:1);
                                if(opcode==0x81) i32 = F16S; else i32 = F8S;
                                MOV32w(x5, i32);
                                MARKLOCK;
                                LDAXRH(x1, wback);
                                emit_adc16(dyn, ninst, x1, x5, x3, x4);
                                STLXRH(x3, x1, wback);
                                CBNZx_MARKLOCK(x3);
                            }
                            break;
                        case 3: //SBB
                            if(opcode==0x81) {INST_NAME("LOCK SBB Ew, Iw");} else {INST_NAME("LOCK SBB Ew, Ib");}
                            READFLAGS(X_CF);
                            SETFLAGS(X_ALL, SF_SET);
                            if(MODREG) {
                                if(opcode==0x81) i32 = F16S; else i32 = F8S;
                                ed = xRAX+(nextop&7)+(rex.b<<3);
                                MOV32w(x5, i32);
                                UXTHw(x6, ed);
                                emit_sbb16(dyn, ninst, x6, x5, x3, x4);
                                BFIx(ed, x6, 0, 16);
                            } else {
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, rex, 0, (opcode==0x81)?2:1);
                                if(opcode==0x81) i32 = F16S; else i32 = F8S;
                                MOV32w(x5, i32);
                                MARKLOCK;
                                LDAXRH(x1, wback);
                                emit_sbb16(dyn, ninst, x1, x5, x3, x4);
                                STLXRH(x3, x1, wback);
                                CBNZx_MARKLOCK(x3);
                            }
                            break;
                        case 4: //AND
                            if(opcode==0x81) {INST_NAME("LOCK AND Ew, Iw");} else {INST_NAME("LOCK AND Ew, Ib");}
                            SETFLAGS(X_ALL, SF_SET);
                            if(MODREG) {
                                if(opcode==0x81) i32 = F16S; else i32 = F8S;
                                ed = xRAX+(nextop&7)+(rex.b<<3);
                                MOV32w(x5, i32);
                                UXTHw(x6, ed);
                                emit_and16(dyn, ninst, x6, x5, x3, x4);
                                BFIx(ed, x6, 0, 16);
                            } else {
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, rex, 0, (opcode==0x81)?2:1);
                                if(opcode==0x81) i32 = F16S; else i32 = F8S;
                                MOV32w(x5, i32);
                                MARKLOCK;
                                LDAXRH(x1, wback);
                                emit_and16(dyn, ninst, x1, x5, x3, x4);
                                STLXRH(x3, x1, wback);
                                CBNZx_MARKLOCK(x3);
                            }
                            break;
                        case 5: //SUB
                            if(opcode==0x81) {INST_NAME("LOCK SUB Ew, Iw");} else {INST_NAME("LOCK SUB Ew, Ib");}
                            SETFLAGS(X_ALL, SF_SET);
                            if(MODREG) {
                                if(opcode==0x81) i32 = F16S; else i32 = F8S;
                                ed = xRAX+(nextop&7)+(rex.b<<3);
                                MOV32w(x5, i32);
                                UXTHw(x6, ed);
                                emit_sub16(dyn, ninst, x6, x5, x3, x4);
                                BFIx(ed, x6, 0, 16);
                            } else {
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, rex, 0, (opcode==0x81)?2:1);
                                if(opcode==0x81) i32 = F16S; else i32 = F8S;
                                MOV32w(x5, i32);
                                TSTx_mask(wback, 1, 0, 0);    // mask=1
                                B_MARK(cNE);
                                MARKLOCK;
                                LDAXRH(x1, wback);
                                emit_sub16(dyn, ninst, x1, x5, x3, x4);
                                STLXRH(x3, x1, wback);
                                CBNZx_MARKLOCK(x3);
                                B_NEXT_nocond;
                                MARK;   // unaligned! also, not enough 
                                LDRH_U12(x1, wback, 0);
                                LDAXRB(x4, wback);
                                BFIw(x1, x4, 0, 8); // re-inject
                                emit_sub16(dyn, ninst, x1, x5, x3, x4);
                                STLXRB(x3, x1, wback);
                                CBNZx_MARK(x3);
                                STRH_U12(x1, wback, 0);    // put the whole value
                            }
                            break;
                        case 6: //XOR
                            if(opcode==0x81) {INST_NAME("LOCK XOR Ew, Iw");} else {INST_NAME("LOCK XOR Ew, Ib");}
                            SETFLAGS(X_ALL, SF_SET);
                            if(MODREG) {
                                if(opcode==0x81) i32 = F16S; else i32 = F8S;
                                ed = xRAX+(nextop&7)+(rex.b<<3);
                                MOV32w(x5, i32);
                                UXTHw(x6, ed);
                                emit_xor16(dyn, ninst, x6, x5, x3, x4);
                                BFIx(ed, x6, 0, 16);
                            } else {
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, rex, 0, (opcode==0x81)?2:1);
                                if(opcode==0x81) i32 = F16S; else i32 = F8S;
                                MOV32w(x5, i32);
                                MARKLOCK;
                                LDAXRH(x1, wback);
                                emit_xor16(dyn, ninst, x1, x5, x3, x4);
                                STLXRH(x3, x1, wback);
                                CBNZx_MARKLOCK(x3);
                            }
                            break;
                        case 7: //CMP
                            if(opcode==0x81) {INST_NAME("(LOCK) CMP Ew, Iw");} else {INST_NAME("(LOCK) CMP Ew, Ib");}
                            SETFLAGS(X_ALL, SF_SET);
                            GETEW(x6, (opcode==0x81)?2:1);
                            // No need to LOCK, this is readonly
                            if(opcode==0x81) i32 = F16S; else i32 = F8S;
                            if(i32) {
                                MOV32w(x5, i32);
                                UXTHw(x6, ed);
                                emit_cmp16(dyn, ninst, x6, x5, x3, x4, x6);
                                BFIx(ed, x6, 0, 16);
                            } else {
                                emit_cmp16_0(dyn, ninst, ed, x3, x4);
                            }
                            break;
                    }
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
                    if(opcode==0x81) {
                        INST_NAME("LOCK ADD Ed, Id");
                    } else {
                        INST_NAME("LOCK ADD Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET);
                    if(MODREG) {
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        ed = xRAX+(nextop&7)+(rex.b<<3);
                        MOV64xw(x5, i64);
                        emit_add32(dyn, ninst, rex, ed, x5, x3, x4);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, rex, 0, (opcode==0x81)?4:1);
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        MOV64xw(x5, i64);
                        TSTx_mask(wback, 1, 0, 0);    // mask=1
                        B_MARK(cNE);
                        MARKLOCK;
                        LDAXRxw(x1, wback);
                        emit_add32(dyn, ninst, rex, x1, x5, x3, x4);
                        STLXRxw(x3, x1, wback);
                        CBNZx_MARKLOCK(x3);
                        B_NEXT_nocond;
                        MARK;   // unaligned! also, not enough 
                        LDRxw_U12(x1, wback, 0);
                        LDAXRB(x4, wback);
                        BFIxw(x1, x4, 0, 8); // re-inject
                        emit_add32(dyn, ninst, rex, x1, x5, x3, x4);
                        STLXRB(x3, x1, wback);
                        CBNZx_MARK(x3);
                        STRxw_U12(x1, wback, 0);    // put the whole value
                    }
                    break;
                case 1: //OR
                    if(opcode==0x81) {INST_NAME("LOCK OR Ed, Id");} else {INST_NAME("LOCK OR Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET);
                    if(MODREG) {
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        ed = xRAX+(nextop&7)+(rex.b<<3);
                        MOV64xw(x5, i64);
                        emit_or32(dyn, ninst, rex, ed, x5, x3, x4);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, rex, 0, (opcode==0x81)?4:1);
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        MOV64xw(x5, i64);
                        MARKLOCK;
                        LDAXRxw(x1, wback);
                        emit_or32(dyn, ninst, rex, x1, x5, x3, x4);
                        STLXRxw(x3, x1, wback);
                        CBNZx_MARKLOCK(x3);
                    }
                    break;
                case 2: //ADC
                    if(opcode==0x81) {INST_NAME("LOCK ADC Ed, Id");} else {INST_NAME("LOCK ADC Ed, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET);
                    if(MODREG) {
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        ed = xRAX+(nextop&7)+(rex.b<<3);
                        MOV64xw(x5, i64);
                        emit_adc32(dyn, ninst, rex, ed, x5, x3, x4);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, rex, 0, (opcode==0x81)?4:1);
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        MOV64xw(x5, i64);
                        MARKLOCK;
                        LDAXRxw(x1, wback);
                        emit_adc32(dyn, ninst, rex, x1, x5, x3, x4);
                        STLXRxw(x3, x1, wback);
                        CBNZx_MARKLOCK(x3);
                    }
                    break;
                case 3: //SBB
                    if(opcode==0x81) {INST_NAME("LOCK SBB Ed, Id");} else {INST_NAME("LOCK SBB Ed, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET);
                    if(MODREG) {
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        ed = xRAX+(nextop&7)+(rex.b<<3);
                        MOV64xw(x5, i64);
                        emit_sbb32(dyn, ninst, rex, ed, x5, x3, x4);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, rex, 0, (opcode==0x81)?4:1);
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        MOV64xw(x5, i64);
                        MARKLOCK;
                        LDAXRxw(x1, wback);
                        emit_sbb32(dyn, ninst, rex, x1, x5, x3, x4);
                        STLXRxw(x3, x1, wback);
                        CBNZx_MARKLOCK(x3);
                    }
                    break;
                case 4: //AND
                    if(opcode==0x81) {INST_NAME("LOCK AND Ed, Id");} else {INST_NAME("LOCK AND Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET);
                    if(MODREG) {
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        ed = xRAX+(nextop&7)+(rex.b<<3);
                        MOV64xw(x5, i64);
                        emit_and32(dyn, ninst, rex, ed, x5, x3, x4);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, rex, 0, (opcode==0x81)?4:1);
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        MOV64xw(x5, i64);
                        MARKLOCK;
                        LDAXRxw(x1, wback);
                        emit_and32(dyn, ninst, rex, x1, x5, x3, x4);
                        STLXRxw(x3, x1, wback);
                        CBNZx_MARKLOCK(x3);
                    }
                    break;
                case 5: //SUB
                    if(opcode==0x81) {INST_NAME("LOCK SUB Ed, Id");} else {INST_NAME("LOCK SUB Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET);
                    if(MODREG) {
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        ed = xRAX+(nextop&7)+(rex.b<<3);
                        MOV64xw(x5, i64);
                        emit_sub32(dyn, ninst, rex, ed, x5, x3, x4);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, rex, 0, (opcode==0x81)?4:1);
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        MOV64xw(x5, i64);
                        TSTx_mask(wback, 1, 0, 0);    // mask=1
                        B_MARK(cNE);
                        MARKLOCK;
                        LDAXRxw(x1, wback);
                        emit_sub32(dyn, ninst, rex, x1, x5, x3, x4);
                        STLXRxw(x3, x1, wback);
                        CBNZx_MARKLOCK(x3);
                        B_NEXT_nocond;
                        MARK;   // unaligned! also, not enough 
                        LDRxw_U12(x1, wback, 0);
                        LDAXRB(x4, wback);
                        BFIxw(x1, x4, 0, 8); // re-inject
                        emit_sub32(dyn, ninst, rex, x1, x5, x3, x4);
                        STLXRB(x3, x1, wback);
                        CBNZx_MARK(x3);
                        STRxw_U12(x1, wback, 0);    // put the whole value
                    }
                    break;
                case 6: //XOR
                    if(opcode==0x81) {INST_NAME("LOCK XOR Ed, Id");} else {INST_NAME("LOCK XOR Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET);
                    if(MODREG) {
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        ed = xRAX+(nextop&7)+(rex.b<<3);
                        MOV64xw(x5, i64);
                        emit_xor32(dyn, ninst, rex, ed, x5, x3, x4);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, rex, 0, (opcode==0x81)?4:1);
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        MOV64xw(x5, i64);
                        MARKLOCK;
                        LDAXRxw(x1, wback);
                        emit_xor32(dyn, ninst, rex, x1, x5, x3, x4);
                        STLXRxw(x3, x1, wback);
                        CBNZx_MARKLOCK(x3);
                    }
                    break;
                case 7: //CMP
                    if(opcode==0x81) {INST_NAME("(LOCK) CMP Ed, Id");} else {INST_NAME("(LOCK) CMP Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET);
                    GETED((opcode==0x81)?4:1);
                    // No need to LOCK, this is readonly
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    if(i64) {
                        MOV64xw(x5, i64);
                        emit_cmp32(dyn, ninst, rex, ed, x5, x3, x4, x6);
                    } else {
                        emit_cmp32_0(dyn, ninst, rex, ed, x3, x4);
                    }
                    break;
            }
            break;
            
        case 0x87:
            INST_NAME("LOCK XCHG Ed, Gd");
            nextop = F8;
            if(MODREG) {
                GETGD;
                GETED(0);
                MOVxw_REG(x1, gd);
                MOVxw_REG(gd, ed);
                MOVxw_REG(ed, x1);
            } else {
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0, rex, 0, 0);
                TSTx_mask(ed, 1, 0, 1+rex.w);    // mask=3 or 7
                B_MARK(cNE);
                MARKLOCK;
                LDAXRxw(x1, ed);
                STLXRxw(x3, gd, ed);
                CBNZx_MARKLOCK(x3);
                B_MARK2_nocond;
                MARK;
                LDRxw_U12(x1, ed, 0);
                STRxw_U12(gd, ed, 0);
                MARK2;
                MOVxw_REG(gd, x1);
            }
            break;
            
        case 0xFF:
            nextop = F8;
            switch((nextop>>3)&7)
            {
                case 0: // INC Ed
                    INST_NAME("LOCK INC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
                    if(MODREG) {
                        ed = xRAX+(nextop&7)+(rex.b<<3);
                        emit_inc32(dyn, ninst, rex, ed, x3, x4);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, rex, 0, 0);
                        TSTx_mask(wback, 1, 0, 1+rex.w);    // mask=3 or 7
                        B_MARK(cNE);    // unaligned
                        MARKLOCK;
                        LDAXRxw(x1, wback);
                        emit_inc32(dyn, ninst, rex, x1, x3, x4);
                        STLXRxw(x3, x1, wback);
                        CBNZx_MARKLOCK(x3);
                        B_NEXT_nocond;
                        MARK;
                        LDRxw_U12(x1, wback, 0);
                        LDAXRB(x4, wback);
                        BFIxw(x1, x4, 0, 8); // re-inject
                        emit_inc32(dyn, ninst, rex, x1, x3, x4);
                        STLXRB(x3, x1, wback);
                        CBNZw_MARK(x3);
                        STRxw_U12(x1, wback, 0);
                    }
                    break;
                case 1: //DEC Ed
                    INST_NAME("LOCK DEC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
                    if(MODREG) {
                        ed = xRAX+(nextop&7)+(rex.b<<3);
                        emit_dec32(dyn, ninst, rex, ed, x3, x4);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, rex, 0, 0);
                        TSTx_mask(wback, 1, 0, 1+rex.w);    // mask=3 or 7
                        B_MARK(cNE);    // unaligned
                        MARKLOCK;
                        LDAXRxw(x1, wback);
                        emit_dec32(dyn, ninst, rex, x1, x3, x4);
                        STLXRxw(x3, x1, wback);
                        CBNZx_MARKLOCK(x3);
                        B_NEXT_nocond;
                        MARK;
                        LDRxw_U12(x1, wback, 0);
                        LDAXRB(x4, wback);
                        BFIxw(x1, x4, 0, 8); // re-inject
                        emit_dec32(dyn, ninst, rex, x1, x3, x4);
                        STLXRB(x3, x1, wback);
                        CBNZw_MARK(x3);
                        STRxw_U12(x1, wback, 0);
                    }
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
