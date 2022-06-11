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
#include "dynarec_native.h"

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_helper.h"
#include "dynarec_arm64_functions.h"

#define GETGX(a, w)                     \
    gd = ((nextop&0x38)>>3)+(rex.r<<3); \
    a = sse_get_reg(dyn, ninst, x1, gd, w)

#define GETGM(a)                        \
    gd = ((nextop&0x38)>>3);            \
    a = mmx_get_reg(dyn, ninst, x1, x2, x3, gd)

#define GETGm   gd = ((nextop&0x38)>>3)

uintptr_t dynarec64_67(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed, wback, wb;
    int64_t fixedaddress;
    int8_t  i8;
    uint8_t u8;
    int32_t i32;
    int64_t j64, i64;
    int v0, v1, s0;
    MAYUSE(i32);
    MAYUSE(j64);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(s0);

    // REX prefix before the 67 are ignored
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }
    rep = 0;
    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }

    switch(opcode) {

        case 0x0F:
            opcode=F8;
            switch(opcode) {

                case 0x2E:
                    // no special check...
                case 0x2F:
                    if(rep) {
                        DEFAULT;
                    } else {
                        if(opcode==0x2F) {INST_NAME("COMISS Gx, Ex");} else {INST_NAME("UCOMISS Gx, Ex");}
                        SETFLAGS(X_ALL, SF_SET);
                        nextop = F8;
                        GETGX(v0, 0);
                        if(MODREG) {
                            s0 = sse_get_reg(dyn, ninst, x1, (nextop&7) + (rex.b<<3), 0);
                        } else {
                            s0 = fpu_get_scratch(dyn);
                            addr = geted32(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<2, 3, rex, 0, 0);
                            VLDR32_U12(s0, ed, fixedaddress);
                        }
                        FCMPS(v0, s0);
                        FCOMI(x1, x2);
                    }
                    break;

                case 0x6F:
                    INST_NAME("MOVQ Gm, Em");
                    nextop = F8;
                    GETGm;
                    if(MODREG) {
                        v1 = mmx_get_reg(dyn, ninst, x1, x2, x3, nextop&7); // no rex.b on MMX
                        v0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
                        VMOV(v0, v1);
                    } else {
                        v0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
                        addr = geted32(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<3, 7, rex, 0, 0);
                        VLDR64_U12(v0, ed, fixedaddress);
                    }
                    break;

                case 0x7F:
                    INST_NAME("MOVQ Em, Gm");
                    nextop = F8;
                    GETGM(v0);
                    if(MODREG) {
                        v1 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, nextop&7);
                        VMOV(v1, v0);
                    } else {
                        addr = geted32(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<3, 7, rex, 0, 0);
                        VSTR64_U12(v0, ed, fixedaddress);
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
            
        case 0x89:
            INST_NAME("MOV Ed, Gd");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                MOVxw_REG(xRAX+(nextop&7)+(rex.b<<3), gd);
            } else {                    // mem <= reg
                addr = geted32(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, 0, 0);
                STRxw_U12(gd, ed, fixedaddress);
            }
            break;

        case 0x8B:
            INST_NAME("MOV Gd, Ed");
            nextop=F8;
            GETGD;
            if(MODREG) {
                MOVxw_REG(gd, xRAX+(nextop&7)+(rex.b<<3));
            } else {    
                addr = geted32(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, 0, 0);
                LDRxw_U12(gd, ed, fixedaddress);
            }
            break;

        case 0x8D:
            INST_NAME("LEA Gd, Ed");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg? that's an invalid operation
                DEFAULT;
            } else {                    // mem <= reg
                // should a geted32 be created, to use 32bits regs instead of 64bits?
                addr = geted32(dyn, addr, ninst, nextop, &ed, gd, &fixedaddress, 0, 0, rex, 0, 0);
                if(ed!=gd) {
                    MOVw_REG(gd, ed);
                }
            }
            break;

        case 0xC1:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ed, Ib");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET_PENDING);
                    GETED32(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    emit_rol32c(dyn, ninst, rex, ed, u8, x3, x4);
                    if(u8) { WBACK; }
                    break;
                case 1:
                    INST_NAME("ROR Ed, Ib");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET_PENDING);
                    GETED32(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    emit_ror32c(dyn, ninst, rex, ed, u8, x3, x4);
                    if(u8) { WBACK; }
                    break;
                case 2:
                    INST_NAME("RCL Ed, Ib");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETED32W(x4, x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(rex.w?((void*)rcl64):((void*)rcl32), ed, x4);
                    SBACK(x1);
                    break;
                case 3:
                    INST_NAME("RCR Ed, Ib");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETED32W(x4, x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(rex.w?((void*)rcr64):((void*)rcr32), ed, x4);
                    SBACK(x1);
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETED32(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    emit_shl32c(dyn, ninst, rex, ed, u8, x3, x4);
                    WBACK;
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

        case 0xC7:
            INST_NAME("MOV Ed, Id");
            nextop=F8;
            if(MODREG) {   // reg <= i32
                i64 = F32S;
                ed = xRAX+(nextop&7)+(rex.b<<3);
                MOV64xw(ed, i64);
            } else {                    // mem <= i32
                addr = geted32(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, 0, 4);
                i64 = F32S;
                MOV64xw(x3, i64);
                STRxw_U12(x3, ed, fixedaddress);
            }
            break;


        #define GO(NO, YES)                                             \
            BARRIER(BARRIER_MAYBE);                                     \
            JUMP(addr+i8, 1);                                           \
            if(dyn->insts[ninst].x64.jmp_insts==-1 ||                   \
                CHECK_CACHE()) {                                        \
                /* out of the block */                                  \
                i32 = dyn->insts[ninst].epilog-(dyn->native_size);      \
                Bcond(NO, i32);                                         \
                if(dyn->insts[ninst].x64.jmp_insts==-1) {               \
                    if(!dyn->insts[ninst].x64.barrier)                  \
                        fpu_purgecache(dyn, ninst, 1, x1, x2, x3);      \
                    jump_to_next(dyn, addr+i8, 0, ninst);               \
                } else {                                                \
                    fpuCacheTransform(dyn, ninst, x1, x2, x3);          \
                    i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);\
                    B(i32);                                             \
                }                                                       \
            } else {    \
                /* inside the block */  \
                i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);    \
                Bcond(YES, i32);    \
            }
        case 0xE0:
            INST_NAME("LOOPNZ (32bits)");
            READFLAGS(X_ZF);
            i8 = F8S;
            MOVw_REG(x1, xRCX);
            SUBSw_U12(x1, x1, 1);
            BFIx(xRCX, x1, 0, 32);
            B_NEXT(cEQ);    // ECX is 0, no LOOP
            TSTw_mask(xFlags, 0b011010, 0); //mask=0x40
            GO(cNE, cEQ);
            break;
        case 0xE1:
            INST_NAME("LOOPZ (32bits)");
            READFLAGS(X_ZF);
            i8 = F8S;
            MOVw_REG(x1, xRCX);
            SUBSw_U12(x1, x1, 1);
            BFIx(xRCX, x1, 0, 32);
            B_NEXT(cEQ);    // ECX is 0, no LOOP
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
                    MOV64xw(x2, i64);
                    emit_test32(dyn, ninst, rex, ed, x2, x3, x4);
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
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_DF(x2, rex.w?d_mul64:d_mul32);
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
                    UFLAG_RES(xRAX);
                    UFLAG_OP1(xRDX);
                    break;
                case 5:
                    INST_NAME("IMUL EAX, Ed");
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_DF(x2, rex.w?d_imul64:d_imul32);
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
                    UFLAG_RES(xRAX);
                    UFLAG_OP1(xRDX);
                    break;
                case 6:
                    INST_NAME("DIV Ed");
                    SETFLAGS(X_ALL, SF_SET);
                    if(!rex.w) {
                        SET_DFNONE(x2);
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
                            SET_DFNONE(x2);
                            GETED32(0);
                            UDIVx(x2, xRAX, ed);
                            MSUBx(xRDX, x2, ed, xRAX);
                            MOVx_REG(xRAX, x2);
                        } else {
                            GETED32H(x1, 0);  // get edd changed addr, so cannot be called 2 times for same op...
                            CBZxw_MARK(xRDX);
                            if(ed!=x1) {MOVx_REG(x1, ed);}
                            CALL(div64, -1);
                            B_NEXT_nocond;
                            MARK;
                            UDIVx(x2, xRAX, ed);
                            MSUBx(xRDX, x2, ed, xRAX);
                            MOVx_REG(xRAX, x2);
                            SET_DFNONE(x2);
                        }
                    }
                    break;
                case 7:
                    INST_NAME("IDIV Ed");
                    SETFLAGS(X_ALL, SF_SET);
                    if(!rex.w) {
                        SET_DFNONE(x2)
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
                            SET_DFNONE(x2)
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
                            CALL((void*)idiv64, -1);
                            B_NEXT_nocond;
                            MARK;
                            SDIVx(x2, xRAX, ed);
                            MSUBx(xRDX, x2, ed, xRAX);
                            MOVx_REG(xRAX, x2);
                            SET_DFNONE(x2)
                        }
                    }
                    break;
            }
            break;

        default:
            DEFAULT;
    }
    return addr;
}
