#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

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
#include "dynarec_arm64_functions.h"
#include "../dynarec_helper.h"

/* setup r2 to address pointed by ED, also fixaddress is an optionnal delta in the range [-absmax, +absmax], with delta&mask==0 to be added to ed for LDR/STR */
uintptr_t geted(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int64_t* fixaddress, int* unscaled, int absmax, uint32_t mask, rex_t rex, int *l, int s, int delta)
{
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(delta);

    if (l == LOCK_LOCK) {
        dyn->insts[ninst].lock = 1;
    }

    if(rex.is32bits && rex.is67)
        return geted16(dyn, addr, ninst, nextop, ed, hint, fixaddress, unscaled, absmax, mask, rex, s);

    int lock = l?((l==LOCK_LOCK)?1:2):0;
    if(unscaled)
        *unscaled = 0;
    if(lock==2)
        *l = 0;
    uint8_t ret = x2;
    uint8_t scratch = x2;
    *fixaddress = 0;
    if(hint>0) ret = hint;
    if(hint>0 && hint<xRAX) scratch = hint;
    int absmin = 0;
    if(s) absmin=-absmax;
    int seg_done = 0;
    MAYUSE(scratch);
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            int sib_reg = ((sib>>3)&7)+(rex.x<<3);
            if((sib&0x7)==5) {
                int64_t tmp = F32S;
                if (sib_reg!=4) {
                    if(tmp && ((!((tmp>=absmin) && (tmp<=absmax) && !(tmp&mask))) || !(unscaled && (tmp>-256) && (tmp<256)))) {
                        MOV64y(scratch, tmp);
                        ADDy_REG_LSL(ret, scratch, TO_NAT(sib_reg), (sib>>6));
                    } else {
                        if(sib>>6) {
                            LSLy(ret, TO_NAT(sib_reg), (sib>>6));
                        } else
                            ret = TO_NAT(sib_reg);
                        if(unscaled && (tmp>-256) && (tmp<256))
                            *unscaled = 1;
                        *fixaddress = tmp;
                    }
                } else {
                    if(rex.seg && !(tmp && ((!((tmp>=absmin) && (tmp<=absmax) && !(tmp&mask))) || !(unscaled && (tmp>-256) && (tmp<256))))) {
                        grab_segdata(dyn, addr, ninst, ret, rex.seg, 0);
                        seg_done = 1;
                        if(unscaled && (tmp>-256) && (tmp<256))
                            *unscaled = 1;
                        *fixaddress = tmp;
                    } else if(rex.seg && tmp>-0x1000 && tmp<0x1000) {
                        grab_segdata(dyn, addr, ninst, ret, rex.seg, 0);
                        if(tmp) {
                            if(tmp>0) ADDx_U12(ret, ret, tmp);
                            else SUBx_U12(ret, ret, -tmp);
                        }
                        seg_done = 1;
                    } else
                        MOV64y(ret, tmp);
                    switch(lock) {
                        case 1: addLockAddress(tmp); if(fixaddress) *fixaddress=tmp; break;
                        case 2: if(isLockAddress(tmp)) *l=1; break;
                    }
                }
            } else {
                if (sib_reg!=4) {
                    ADDy_REG_LSL(ret, TO_NAT((sib&0x7)+(rex.b<<3)), TO_NAT(sib_reg), (sib>>6));
                } else {
                    ret = TO_NAT((sib&0x7)+(rex.b<<3));
                }
            }
        } else if((nextop&7)==5) {
            if(rex.is32bits) {
                int tmp = F32S;
                if(rex.seg && tmp>-0x1000 && tmp<0x1000) {
                    grab_segdata(dyn, addr, ninst, ret, rex.seg, 0);
                    if(tmp) {
                        if(tmp>0) ADDx_U12(ret, ret, tmp);
                        else SUBx_U12(ret, ret, -tmp);
                    }
                    seg_done = 1;
                } else
                    MOV32w(ret, tmp);
                if(!rex.seg)
                    switch(lock) {
                        case 1: addLockAddress(tmp); if(fixaddress) *fixaddress=tmp; break;
                        case 2: if(isLockAddress(tmp)) *l=1; break;
                    }
            } else {
                int64_t tmp = F32S64;
                if((tmp>=absmin) && (tmp<=absmax) && !(tmp&mask)) {
                    GETIP(addr+delta);
                    ret = xRIP;
                    *fixaddress = tmp;
                } else if(unscaled && (tmp>-256) && (tmp<256)) {
                    GETIP(addr+delta);
                    ret = xRIP;
                    *fixaddress = tmp;
                    *unscaled = 1;
                } else if(tmp>0 && tmp<0x1000) {
                    GETIP(addr+delta);
                    ADDy_U12(ret, xRIP, tmp);
                } else if(tmp<0 && tmp>-0x1000) {
                    GETIP(addr+delta);
                    SUBy_U12(ret, xRIP, -tmp);
                } else if((tmp+addr+delta<0x1000000000000LL) && !dyn->need_reloc) {  // 3 opcodes to load immediate is cheap enough
                    MOV64y(ret, tmp+addr+delta);
                } else {
                    MOV64y(ret, tmp);
                    GETIP(addr+delta);
                    ADDy_REG(ret, ret, xRIP);
                }
                if(!rex.seg)
                    switch(lock) {
                        case 1: addLockAddress(addr+delta+tmp); if(fixaddress) *fixaddress=addr+delta+tmp; break;
                        case 2: if(isLockAddress(addr+delta+tmp)) *l=1; break;
                    }
            }
        } else {
            ret = TO_NAT((nextop & 7) + (rex.b << 3));
        }
    } else {
        int64_t i64;
        uint8_t sib = 0;
        int sib_reg = 0;
        if((nextop&7)==4) {
            sib = F8;
            sib_reg = ((sib>>3)&7)+(rex.x<<3);
        }
        if(nextop&0x80)
            i64 = F32S;
        else
            i64 = F8S;
        if(i64==0 || ((i64>=absmin) && (i64<=absmax)  && !(i64&mask)) || (unscaled && (i64>-256) && (i64<256))) {
            *fixaddress = i64;
            if(unscaled && i64 && (i64>-256) && (i64<256))
                *unscaled = 1;
            if((nextop&7)==4) {
                if (sib_reg!=4) {
                    ADDy_REG_LSL(ret, TO_NAT((sib&0x07)+(rex.b<<3)), TO_NAT(sib_reg), (sib>>6));
                } else {
                    ret = TO_NAT((sib&0x07)+(rex.b<<3));
                }
            } else
                ret = TO_NAT((nextop & 0x07) + (rex.b << 3));
        } else {
            int64_t sub = (i64<0)?1:0;
            if(sub) i64 = -i64;
            if(i64<0x1000) {
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        ADDy_REG_LSL(scratch, TO_NAT((sib&0x07)+(rex.b<<3)), TO_NAT(sib_reg), (sib>>6));
                    } else {
                        scratch = TO_NAT((sib&0x07)+(rex.b<<3));
                    }
                } else
                    scratch = TO_NAT((nextop & 0x07) + (rex.b << 3));
                if(sub) {
                    SUBy_U12(ret, scratch, i64);
                } else {
                    ADDy_U12(ret, scratch, i64);
                }
            } else {
                MOV64y(scratch, i64);
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        if(sub) {
                            SUBy_REG(scratch, TO_NAT((sib&0x07)+(rex.b<<3)), scratch);
                        } else {
                            ADDy_REG(scratch, scratch, TO_NAT((sib&0x07)+(rex.b<<3)));
                        }
                        ADDy_REG_LSL(ret, scratch, TO_NAT(sib_reg), (sib>>6));
                    } else {
                        PASS3(int tmp = TO_NAT(sib & 0x07) + (rex.b << 3));
                        if(sub) {
                            SUBy_REG(ret, tmp, scratch);
                        } else {
                            ADDy_REG(ret, tmp, scratch);
                        }
                    }
                } else {
                    PASS3(int tmp = TO_NAT((nextop & 0x07) + (rex.b << 3)));
                    if(sub) {
                        SUBy_REG(ret, tmp, scratch);
                    } else {
                        ADDy_REG(ret, tmp, scratch);
                    }
                }
            }
        }
    }
    if(rex.is67 && IS_GPR(ret) && !rex.seg) {
        MOVw_REG(hint, ret);    // truncate for is67 case only (is32bits case regs are already 32bits only)
        ret = hint;
    }
    if(rex.seg && !seg_done) {
        if(scratch==ret)
            scratch=ret+1;
        grab_segdata(dyn, addr, ninst, scratch, rex.seg, 0);
        //seg offset is 64bits, so no truncation here
        ADDx_REGy(hint, scratch, ret);
        ret = hint;
    }
    *ed = ret;
    return addr;
}

/* setup r2 to address pointed by ED, r3 as scratch also fixaddress is an optionnal delta in the range [-absmax, +absmax], with delta&mask==0 to be added to ed for LDR/STR */
uintptr_t geted16(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int64_t* fixaddress, int* unscaled, int absmax, uint32_t mask, rex_t rex, int s)
{
    MAYUSE(dyn); MAYUSE(ninst);

    if(unscaled)
        *unscaled = 0;
    uint8_t ret = x2;
    uint8_t scratch = x3;
    *fixaddress = 0;
    if(hint>0) ret = hint;
    if(scratch==ret) scratch = x2;
    MAYUSE(scratch);
    uint32_t m = nextop&0xC7;
    uint32_t n = (m>>6)&3;
    int64_t offset = 0;
    int absmin = 0;
    if(s) absmin = -absmax;
    if(!n && (m&7)==6) {
        offset = F16S;
        MOVZw(ret, offset);
    } else {
        switch(n) {
            case 0: offset = 0; break;
            case 1: offset = F8S; break;
            case 2: offset = F16S; break;
        }
        if(offset && (offset>=absmax && offset<=absmin && !(offset&mask))) {
            *fixaddress = offset;
            offset = 0;
        }
        if(offset && (unscaled && offset>-256 && offset<256)) {
            *fixaddress = offset;
            offset = 0;
        }
        switch(m&7) {
            case 0: //R_BX + R_SI
                UXTHx(ret, xRBX);
                UXTHx(scratch, xRSI);
                ADDx_REG(ret, ret, scratch);
                break;
            case 1: //R_BX + R_DI
                UXTHx(ret, xRBX);
                UXTHx(scratch, xRDI);
                ADDx_REG(ret, ret, scratch);
                break;
            case 2: //R_BP + R_SI
                UXTHx(ret, xRBP);
                UXTHx(scratch, xRSI);
                ADDx_REG(ret, ret, scratch);
                break;
            case 3: //R_BP + R_DI
                UXTHx(ret, xRBP);
                UXTHx(scratch, xRDI);
                ADDx_REG(ret, ret, scratch);
                break;
            case 4: //R_SI
                UXTHx(ret, xRSI);
                break;
            case 5: //R_DI
                UXTHx(ret, xRDI);
                break;
            case 6: //R_BP
                UXTHx(ret, xRBP);
                break;
            case 7: //R_BX
                UXTHx(ret, xRBX);
                break;
        }
        if(offset) {
            if(offset<0 && offset>-0x1000) {
                SUBx_U12(ret, ret, -offset);
            } else if(offset>0 && offset<0x1000) {
                ADDx_U12(ret, ret, offset);
            } else {
                MOV64x(scratch, offset);
                ADDx_REG(ret, ret, scratch);
            }
        }
    }

    if(rex.seg) {
        if(scratch==ret)
            scratch=ret+1;
        grab_segdata(dyn, addr, ninst, scratch, rex.seg, 0);
        //seg offset is 64bits, so no truncation here
        if(IS_GPR(ret)) {
            ADDx_REG(hint, ret, scratch);
            ret = hint;
        } else
            ADDx_REG(ret, ret, scratch);
    }
    *ed = ret;
    return addr;
}

void jump_to_epilog(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst)
{
    MAYUSE(dyn); MAYUSE(ip); MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Jump to epilog\n");

    if(reg) {
        if(reg!=xRIP) {
            MOVx_REG(xRIP, reg);
        }
    } else {
        GETIP_(ip);
    }
    NOTEST(x2);
    TABLE64C(x2, const_epilog);
    SMEND();
    CHECK_DFNONE(0);
    if(dyn->have_purge)
        doLeaveBlock(dyn, ninst, x4, x5, x6);
    BR(x2);
}


static int indirect_lookup(dynarec_arm_t* dyn, int ninst, int is32bits, int s1, int s2)
{
    MAYUSE(dyn);
    if (!is32bits) {
        // check higher 48bits
        LSRx_IMM(s1, xRIP, 48);
        intptr_t j64 = (intptr_t)dyn->jmp_next - (intptr_t)dyn->block;
        CBNZw(s1, j64);
        // load table
        if(dyn->need_reloc) {
            TABLE64C(s2, const_jmptbl48);
        } else {
            MOV64x(s2, getConst(const_jmptbl48));    // this is a static value, so will be a low address
        }
        #ifdef JMPTABL_SHIFT4
        UBFXx(s1, xRIP, JMPTABL_START3, JMPTABL_SHIFT3);
        LDRx_REG_LSL3(s2, s2, s1);
        #endif
        UBFXx(s1, xRIP, JMPTABL_START2, JMPTABL_SHIFT2);
        LDRx_REG_LSL3(s2, s2, s1);
    } else {
        // check higher 32bits disabled
        // LSRx_IMM(s1, xRIP, 32);
        // intptr_t j64 = (intptr_t)dyn->jmp_next - (intptr_t)dyn->block;
        // CBNZw(s1, j64);
        // load table
        TABLE64C(s2, const_jmptbl32);
        #ifdef JMPTABL_SHIFT4
        UBFXx(s1, xRIP, JMPTABL_START2, JMPTABL_SHIFT2);
        LDRx_REG_LSL3(s2, s2, s1);
        #endif
    }
    UBFXx(s1, xRIP, JMPTABL_START1, JMPTABL_SHIFT1);
    LDRx_REG_LSL3(s2, s2, s1);
    UBFXx(s1, xRIP, JMPTABL_START0, JMPTABL_SHIFT0);
    LDRx_REG_LSL3(s1, s2, s1);
    return s1;
}

void jump_to_next(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst, int is32bits)
{
    MAYUSE(dyn);
    MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Jump to next\n");

    if (is32bits)
        ip &= 0xffffffffLL;

    SMEND();
    CHECK_DFNONE(0);
    int dest;
    if (reg) {
        if (reg != xRIP) {
            MOVx_REG(xRIP, reg);
        }
        NOTEST(x2);
        dest = indirect_lookup(dyn, ninst, is32bits, x2, x3);
    } else {
        NOTEST(x2);
        uintptr_t p = getJumpTableAddress64(ip);
        MAYUSE(p);
        if(dyn->need_reloc) AddRelocTable64JmpTbl(dyn, ninst, ip, STEP);
        TABLE64_(x3, p);
        GETIP_(ip);
        LDRx_U12(x2, x3, 0);
        dest = x2;
    }
    if(reg!=x1) {
        MOVx_REG(x1, xRIP);
    }
    if(dyn->have_purge && !dyn->insts[ninst].x64.has_callret)
        doLeaveBlock(dyn, ninst, x4, x5, x6);
    #ifdef HAVE_TRACE
    //MOVx(x3, 15);    no access to PC reg
    BLR(dest); // save LR...
    #else
    if (dyn->insts[ninst].x64.has_callret) {
        BLR(dest); // save LR...
    } else {
        BR(dest);
    }
    #endif
}

void ret_to_epilog(dynarec_arm_t* dyn, uintptr_t ip, int ninst, rex_t rex)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Ret to epilog\n");
    CHECK_DFNONE(0);
    POP1z(xRIP);
    MOVz_REG(x1, xRIP);
    SMEND();
    if(BOX64DRENV(dynarec_callret)) {
        // pop the actual return address for ARM stack
        LDPx_S7_postindex(xLR, x6, xSP, 16);
        SUBx_REG(x6, x6, xRIP); // is it the right address?
        if(dyn->have_purge)
            doLeaveBlock(dyn, ninst, x4, x5, x3);
        CBNZx(x6, 2*4);
        RET(xLR);
        // not the correct return address, regular jump, but purge the stack first, it's unsync now...
        SUBx_U12(xSP, xSavedSP, 16);
    }
    NOTEST(x2);
    int dest = indirect_lookup(dyn, ninst, rex.is32bits, x2, x3);
    if(dyn->have_purge && !BOX64DRENV(dynarec_callret))
        doLeaveBlock(dyn, ninst, x4, x5, x6);
    #ifdef HAVE_TRACE
    BLR(dest);
    #else
    BR(dest);
    #endif
    CLEARIP();
}

void retn_to_epilog(dynarec_arm_t* dyn, uintptr_t ip, int ninst, rex_t rex, int n)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Retn to epilog\n");
    CHECK_DFNONE(0);
    POP1z(xRIP);
    if(n>0xfff) {
        MOV32w(w1, n);
        ADDz_REG(xRSP, xRSP, x1);
    } else {
        ADDz_U12(xRSP, xRSP, n);
    }
    MOVz_REG(x1, xRIP);
    SMEND();
    if(BOX64DRENV(dynarec_callret)) {
        // pop the actual return address for ARM stack
        LDPx_S7_postindex(xLR, x6, xSP, 16);
        SUBx_REG(x6, x6, xRIP); // is it the right address?
        if(dyn->have_purge)
            doLeaveBlock(dyn, ninst, x4, x5, x3);
        CBNZx(x6, 2*4);
        RET(xLR);
        // not the correct return address, regular jump
        SUBx_U12(xSP, xSavedSP, 16);
    }
    NOTEST(x2);
    int dest = indirect_lookup(dyn, ninst, rex.is32bits, x2, x3);
    if(dyn->have_purge && !BOX64DRENV(dynarec_callret))
        doLeaveBlock(dyn, ninst, x4, x5, x6);
    #ifdef HAVE_TRACE
    BLR(dest);
    #else
    BR(dest);
    #endif
    CLEARIP();
}

void iret_to_epilog(dynarec_arm_t* dyn, uintptr_t ip, int ninst, int is32bits, int is64bits)
{
    int64_t j64;
    //#warning TODO: is64bits
    MAYUSE(ninst);
    MAYUSE(j64);
    MESSAGE(LOG_DUMP, "IRet to epilog\n");
    SMEND();
    SET_DFNONE();
    // POP IP
    NOTEST(x2);
    if(is64bits) {
        POP1(x1);
        POP1(x2);
        POP1(x3);
    } else {
        POP1_32(x1);
        POP1_32(x2);
        POP1_32(x3);
    }
    // check CS is NULL, sgfault if it is
    CBZw_MARK3(x1);
    // clean EFLAGS
    MOV32w(x4, 0x3E7FD7);   // also mask RF, because it's not really handled
    ANDx_REG(x3, x3, x4);
    ORRx_mask(x3, x3, 1, 0b111111, 0); // xFlags | 0b10
    FORCE_DFNONE();
    if(is32bits) {
        ANDw_mask(x4, x2, 0, 7);   // mask 0xff
        // check if return segment is 64bits, then restore rsp too
        CMPSw_U12(x4, 0x23);
        B_MARKSEG(cEQ);
    }
    // POP RSP
    if(is64bits) {
        POP1(x4);   //rsp
        POP1(x5);   //ss
    } else {
        POP1_32(x4);   //rsp
        POP1_32(x5);   //ss
    }
    // check if SS is NULL
    CBZw_MARK(x5);
    // POP SS
    STRH_U12(x5, xEmu, offsetof(x64emu_t, segs[_SS]));
    // set new RSP
    MOVx_REG(xRSP, x4);
    MARKSEG;
    // x2 is CS, x1 is IP, x3 is eFlags
    STRH_U12(x2, xEmu, offsetof(x64emu_t, segs[_CS]));
    MOVx_REG(xRIP, x1);
    MOVw_REG(xFlags, x3);
    // Ret....
    // epilog on purpose, CS might have changed!
    if(dyn->need_reloc)
        TABLE64C(x2, const_epilog);
    else
        MOV64x(x2, getConst(const_epilog));
    if(dyn->have_purge)
        doLeaveBlock(dyn, ninst, x4, x5, x6);
    BR(x2);
    CLEARIP();
    MARK;
    if(is64bits)
        ADDx_U12(xRSP, xRSP, 8*2);
    else
        ADDx_U12(xRSP, xRSP, 4*2);
    MARK3;
    if(is64bits)
        ADDx_U12(xRSP, xRSP, 8*3);
    else
        ADDx_U12(xRSP, xRSP, 4*3);
    CALL_S(const_native_priv, -1);
}

void call_c(dynarec_arm_t* dyn, int ninst, arm64_consts_t fnc, int reg, int ret, int saveflags, int savereg)
{
    MAYUSE(fnc);
    CHECK_DFNONE(1);
    #if STEP == 0
    dyn->insts[ninst].nat_flags_op = NAT_FLAG_OP_UNUSABLE;
    #endif
    if(savereg==0)
        savereg = x87pc;
    if(saveflags) {
        STRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags));
    }
    if(ret!=-2) {
        STPx_S7_preindex(xEmu, savereg, xSP, -16);   // ARM64 stack needs to be 16byte aligned
        STPx_S7_offset(xRAX, xRCX, xEmu, offsetof(x64emu_t, regs[_AX]));    // x9..x15, x16,x17,x18 those needs to be saved by caller
        STPx_S7_offset(xRDX, xRBX, xEmu, offsetof(x64emu_t, regs[_DX]));    // but x18 is R8 wich is lost, so it's fine to not save it?
        STPx_S7_offset(xRSP, xRBP, xEmu, offsetof(x64emu_t, regs[_SP]));
        STPx_S7_offset(xRSI, xRDI, xEmu, offsetof(x64emu_t, regs[_SI]));
        STPx_S7_offset(xR8,  xR9,  xEmu, offsetof(x64emu_t, regs[_R8]));
        fpu_pushcache(dyn, ninst, savereg, 0);
    }
    #ifdef _WIN32
    LDRx_U12(xR8, xEmu, offsetof(x64emu_t, win64_teb));
    #endif
    TABLE64C(reg, fnc);
    BLR(reg);
    if(ret>=0) {
        MOVx_REG(ret, xEmu);
    }
    if(ret!=-2) {
        LDPx_S7_postindex(xEmu, savereg, xSP, 16);
        #define GO(A, B) if(ret==x##A) {                                        \
            LDRx_U12(x##B, xEmu, offsetof(x64emu_t, regs[_##B]));               \
        } else if(ret==x##B) {                                                  \
            LDRx_U12(x##A, xEmu, offsetof(x64emu_t, regs[_##A]));               \
        } else {                                                                \
            LDPx_S7_offset(x##A, x##B, xEmu, offsetof(x64emu_t, regs[_##A]));   \
        }
        GO(RAX, RCX);
        GO(RDX, RBX);
        GO(RSP, RBP);
        GO(RSI, RDI);
        GO(R8, R9);
        #undef GO
        fpu_popcache(dyn, ninst, savereg, 0);   // savereg will not be used
    }
    if(saveflags) {
        LDRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags));
    }
    if(savereg!=x87pc && dyn->need_x87check) {
        NATIVE_RESTORE_X87PC();
    }
    //SET_NODF();
}

void call_d(dynarec_arm_t* dyn, int ninst, arm64_consts_t fnc, int ret, int arg1, int arg2, int sav1, int sav2)
{
    MAYUSE(fnc);
    CHECK_DFNONE(1);
    #if STEP == 0
    dyn->insts[ninst].nat_flags_op = NAT_FLAG_OP_UNUSABLE;
    #endif
    STPx_S7_preindex(xEmu, x87pc, xSP, -16);
    if((sav1>0) || (sav2>0)) {
        STPx_S7_preindex((sav1>0)?sav1:xZR, (sav2>0)?sav2:xZR, xSP, -16);
    }
    STPx_S7_offset(xRAX, xRCX, xEmu, offsetof(x64emu_t, regs[_AX]));    // x9..x15, x16,x17,x18 those needs to be saved by caller
    STPx_S7_offset(xRDX, xRBX, xEmu, offsetof(x64emu_t, regs[_DX]));
    STPx_S7_offset(xRSP, xRBP, xEmu, offsetof(x64emu_t, regs[_SP]));
    STPx_S7_offset(xRSI, xRDI, xEmu, offsetof(x64emu_t, regs[_SI]));
    STPx_S7_offset(xR8,  xR9,  xEmu, offsetof(x64emu_t, regs[_R8]));
    STRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags));
    fpu_pushcache(dyn, ninst, x87pc, 0);

    #ifdef _WIN32
    LDRx_U12(xR8, xEmu, offsetof(x64emu_t, win64_teb));
    #endif
    if(arg1!=-1)
        FMOVD(0, arg1);
    if(arg2!=-1)
        FMOVD(1, arg2);
    TABLE64C(x87pc, fnc);
    BLR(x87pc);
    if(ret!=-1)
        FMOVD(ret, 0);  // will not work if ret is 0..7 and XMM are used
    if((sav1>0) || (sav2>0)) {
        LDPx_S7_postindex((sav1>0)?sav1:xZR, (sav2>0)?sav2:xZR, xSP, 16);
    }
    LDPx_S7_postindex(xEmu, x87pc, xSP, 16);
    #define GO(A, B) LDPx_S7_offset(x##A, x##B, xEmu, offsetof(x64emu_t, regs[_##A]))
    GO(RAX, RCX);
    GO(RDX, RBX);
    GO(RSP, RBP);
    GO(RSI, RDI);
    GO(R8, R9);
    #undef GO
    LDRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags));
    fpu_popcache(dyn, ninst, x1, 0);   // savereg will not be used
    //SET_NODF();
}

void call_n(dynarec_arm_t* dyn, int ninst, void* fnc, int w)
{
    MAYUSE(fnc);
    CHECK_DFNONE(1);
    #if STEP == 0
    dyn->insts[ninst].nat_flags_op = NAT_FLAG_OP_UNUSABLE;
    #endif
    STRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags));
    fpu_pushcache(dyn, ninst, x3, 1);
    // x9..x15, x16,x17,x18 those needs to be saved by caller
    // RDI, RSI, RDX, RCX, R8, R9 are used for function call
    STPx_S7_preindex(xEmu, xRBX, xSP, -16);   // ARM64 stack needs to be 16byte aligned
    STPx_S7_offset(xRSP, xRBP, xEmu, offsetof(x64emu_t, regs[_SP]));
    // float and double args
    if(abs(w)>1) {
        MESSAGE(LOG_DUMP, "Getting %d XMM args\n", abs(w)-1);
        for(int i=0; i<abs(w)-1; ++i) {
            sse_get_reg(dyn, ninst, x3, i, w);
        }
    }
    if(w<0) {
        MESSAGE(LOG_DUMP, "Return in XMM0\n");
        sse_get_reg_empty(dyn, ninst, x3, 0);
    }
    // prepare regs for native call
    MOVx_REG(0, xRDI);
    MOVx_REG(x1, xRSI);
    MOVx_REG(x2, xRDX);
    MOVx_REG(x3, xRCX);
    MOVx_REG(x4, xR8);
    MOVx_REG(x5, xR9);
    // native call
    if(dyn->need_reloc) {
        // fnc is indirect, to help with relocation (but PltResolver might be an issue here)
        TABLE64(16, (uintptr_t)fnc);
        LDRx_U12(16, 16, 0);
    } else {
        TABLE64_(16, *(uintptr_t*)fnc);    // using x16 as scratch regs for call address
    }
    BLR(16);
    // put return value in x64 regs
    if(w>0) {
        MOVx_REG(xRAX, 0);
        MOVx_REG(xRDX, x1);
    }
    // all done, restore all regs
    LDPx_S7_postindex(xEmu, xRBX, xSP, 16);
    #define GO(A, B) LDPx_S7_offset(x##A, x##B, xEmu, offsetof(x64emu_t, regs[_##A]))
    GO(RSP, RBP);
    #undef GO

    fpu_popcache(dyn, ninst, x3, 1);
    LDRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags));
    NATIVE_RESTORE_X87PC();
    //SET_NODF();
}

void grab_segdata(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int reg, int segment, int modreg)
{
    (void)addr;
    int64_t j64;
    MAYUSE(j64);
    if (modreg) return;
    MESSAGE(LOG_DUMP, "Get %s Offset\n", (segment==_FS)?"FS":"GS");
    LDRx_U12(reg, xEmu, offsetof(x64emu_t, segs_offs[segment]));
    MESSAGE(LOG_DUMP, "----%s Offset\n", (segment==_FS)?"FS":"GS");
}

// x87 stuffs
int x87_stackcount(dynarec_arm_t* dyn, int ninst, int scratch)
{
    MAYUSE(scratch);
    if(!dyn->n.x87stack)
        return 0;
    if(dyn->n.mmxcount)
        mmx_purgecache(dyn, ninst, 0, scratch);
    MESSAGE(LOG_DUMP, "\tSynch x87 Stackcount (%d)\n", dyn->n.x87stack);
    int a = dyn->n.x87stack;
    // Add x87stack to emu fpu_stack
    LDRw_U12(scratch, xEmu, offsetof(x64emu_t, fpu_stack));
    if(a>0) {
        ADDw_U12(scratch, scratch, a);
    } else {
        SUBw_U12(scratch, scratch, -a);
    }
    STRw_U12(scratch, xEmu, offsetof(x64emu_t, fpu_stack));
    // Sub x87stack to top, with and 7
    LDRw_U12(scratch, xEmu, offsetof(x64emu_t, top));
    if(a>0) {
        SUBw_U12(scratch, scratch, a);
    } else {
        ADDw_U12(scratch, scratch, -a);
    }
    ANDw_mask(scratch, scratch, 0, 2);  //mask=7
    STRw_U12(scratch, xEmu, offsetof(x64emu_t, top));
    // reset x87stack, but not the stack count of neoncache
    int ret = dyn->n.x87stack;
    dyn->n.x87stack = 0;
    dyn->n.stack_next -= dyn->n.stack;
    dyn->n.stack = 0;
    MESSAGE(LOG_DUMP, "\t------x87 Stackcount\n");
    return ret;
}

void x87_unstackcount(dynarec_arm_t* dyn, int ninst, int scratch, int count)
{
    MAYUSE(scratch);
    if(!count)
        return;
    if(dyn->n.mmxcount)
        mmx_purgecache(dyn, ninst, 0, scratch);
    MESSAGE(LOG_DUMP, "\tUnsynch x87 Unstackcount (%d)\n", count);
    int a = -count;
    // Add x87stack to emu fpu_stack
    LDRw_U12(scratch, xEmu, offsetof(x64emu_t, fpu_stack));
    if(a>0) {
        ADDw_U12(scratch, scratch, a);
    } else {
        SUBw_U12(scratch, scratch, -a);
    }
    STRw_U12(scratch, xEmu, offsetof(x64emu_t, fpu_stack));
    // Sub x87stack to top, with and 7
    LDRw_U12(scratch, xEmu, offsetof(x64emu_t, top));
    if(a>0) {
        SUBw_U12(scratch, scratch, a);
    } else {
        ADDw_U12(scratch, scratch, -a);
    }
    ANDw_mask(scratch, scratch, 0, 2);  //mask=7
    STRw_U12(scratch, xEmu, offsetof(x64emu_t, top));
    // reset x87stack, but not the stack count of neoncache
    dyn->n.x87stack = count;
    dyn->n.stack = count;
    dyn->n.stack_next += dyn->n.stack;
    MESSAGE(LOG_DUMP, "\t------x87 Unstackcount\n");
}

int neoncache_st_coherency(dynarec_arm_t* dyn, int ninst, int a, int b)
{
    int i1 = neoncache_get_st(dyn, ninst, a);
    int i2 = neoncache_get_st(dyn, ninst, b);
    if(i1!=i2) {
        MESSAGE(LOG_DUMP, "Warning, ST cache incoherent between ST%d(%d) and ST%d(%d)\n", a, i1, b, i2);
    }

    return i1;
}

// On step 1, Float/Double for ST is actualy computed and back-propagated
// On step 2-3, the value is just read for inst[...].n.neocache[..]
// the reg returned is *2 for FLOAT
int x87_do_push(dynarec_arm_t* dyn, int ninst, int s1, int t)
{
    dyn->insts[ninst].x87_used = 1;
    if(dyn->n.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    dyn->n.x87stack+=1;
    dyn->n.stack+=1;
    dyn->n.stack_next+=1;
    dyn->n.stack_push+=1;
    ++dyn->n.pushed;
    if(dyn->n.poped)
        --dyn->n.poped;
    // move all regs in cache, and find a free one
    for(int j=0; j<24; ++j)
        if((dyn->n.neoncache[j].t == NEON_CACHE_ST_D)
         ||(dyn->n.neoncache[j].t == NEON_CACHE_ST_F)
         ||(dyn->n.neoncache[j].t == NEON_CACHE_ST_I64))
            ++dyn->n.neoncache[j].n;
    int ret = -1;
    dyn->n.tags<<=2;
    for(int i=0; i<8; ++i) {
        if(dyn->n.x87cache[i]!=-1)
            ++dyn->n.x87cache[i];
        else if(ret==-1) {
            dyn->n.x87cache[i] = 0;
            ret=dyn->n.x87reg[i]=fpu_get_reg_x87(dyn, ninst, t, 0);
            dyn->n.neoncache[ret].t = X87_ST0;
        }
    }
    if(ret==-1) {
        MESSAGE(LOG_DUMP, "Incoherent x87 stack cache, aborting\n");
        dyn->abort = 1;
    }
    return ret;
}
void x87_do_push_empty(dynarec_arm_t* dyn, int ninst, int s1)
{
    dyn->insts[ninst].x87_used = 1;
    if(dyn->n.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    dyn->n.x87stack+=1;
    dyn->n.stack+=1;
    dyn->n.stack_next+=1;
    dyn->n.stack_push+=1;
    ++dyn->n.pushed;
    if(dyn->n.poped)
        --dyn->n.poped;
    // move all regs in cache
    for(int j=0; j<24; ++j)
        if((dyn->n.neoncache[j].t == NEON_CACHE_ST_D)
         ||(dyn->n.neoncache[j].t == NEON_CACHE_ST_F)
         ||(dyn->n.neoncache[j].t == NEON_CACHE_ST_I64))
            ++dyn->n.neoncache[j].n;
    int ret = -1;
    dyn->n.tags<<=2;
    for(int i=0; i<8; ++i) {
        if(dyn->n.x87cache[i]!=-1)
            ++dyn->n.x87cache[i];
        else if(ret==-1)
            ret = i;
    }
    if(ret==-1) {
        MESSAGE(LOG_DUMP, "Incoherent x87 stack cache, aborting\n");
        dyn->abort = 1;
    }
}
static void internal_x87_dopop(dynarec_arm_t* dyn)
{
    for(int i=0; i<8; ++i)
        if(dyn->n.x87cache[i]!=-1) {
            --dyn->n.x87cache[i];
            if(dyn->n.x87cache[i]==-1) {
                fpu_free_reg(dyn, dyn->n.x87reg[i]);
                dyn->n.x87reg[i] = -1;
            }
        }
}
static int internal_x87_dofree(dynarec_arm_t* dyn)
{
    if(dyn->n.tags&0b11) {
        MESSAGE(LOG_DUMP, "\t--------x87 FREED ST0, poping 1 more\n");
        return 1;
    }
    return 0;
}
void x87_do_pop(dynarec_arm_t* dyn, int ninst, int s1)
{
    dyn->insts[ninst].x87_used = 1;
    if(dyn->n.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    do {
        dyn->n.x87stack-=1;
        dyn->n.stack_next-=1;
        dyn->n.stack_pop+=1;
        dyn->n.tags>>=2;
        ++dyn->n.poped;
        if(dyn->n.pushed)
            --dyn->n.pushed;
        // move all regs in cache, poping ST0
        internal_x87_dopop(dyn);
    } while(internal_x87_dofree(dyn));
}
static int x87_is_stcached(dynarec_arm_t* dyn, int st)
{
    for (int i=0; i<8; ++i)
        if(dyn->n.x87cache[i] == st)
            return 1;
    return 0;
}
void x87_purgecache(dynarec_arm_t* dyn, int ninst, int next, int s1, int s2, int s3)
{
    int ret = 0;
    for (int i=0; i<8 && !ret; ++i)
        if(dyn->n.x87cache[i] != -1)
            ret = 1;
    if(!ret && !dyn->n.x87stack)    // nothing to do
        return;
    MESSAGE(LOG_DUMP, "\tPurge %sx87 Cache and Synch Stackcount (%+d)---\n", next?"locally ":"", dyn->n.x87stack);
    int a = dyn->n.x87stack;
    if(a!=0) {
        // reset x87stack
        if(!next) {
            dyn->n.x87stack = 0;
        }
        // Add x87stack to emu fpu_stack
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        if(a>0) {
            ADDw_U12(s2, s2, a);
        } else {
            SUBw_U12(s2, s2, -a);
        }
        STRw_U12(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        // Sub x87stack to top, with and 7
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
        if(a>0) {
            SUBw_U12(s2, s2, a);
        } else {
            ADDw_U12(s2, s2, -a);
        }
        ANDw_mask(s2, s2, 0, 2);
        STRw_U12(s2, xEmu, offsetof(x64emu_t, top));
        // update tags
        LDRH_U12(s1, xEmu, offsetof(x64emu_t, fpu_tags));
        if(a>0) {
            LSLw_IMM(s1, s1, a*2);
        } else {
            ORRw_mask(s1, s1, 0b010000, 0b001111);  // 0xffff0000
            LSRw_IMM(s1, s1, -a*2);
        }
        STRH_U12(s1, xEmu, offsetof(x64emu_t, fpu_tags));
    } else {
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
    }
    // check if free is used
    if(dyn->n.tags) {
        LDRH_U12(s1, xEmu, offsetof(x64emu_t, fpu_tags));
        MOV32w(s3, dyn->n.tags);
        ORRw_REG(s1, s1, s3);
        STRH_U12(s1, xEmu, offsetof(x64emu_t, fpu_tags));
    }

    if(ret!=0) {
        // --- set values
        // prepare offset to fpu => s1
        ADDx_U12(s1, xEmu, offsetof(x64emu_t, x87));
        // Get top
        // loop all cache entries
        for (int i=0; i<8; ++i)
            if(dyn->n.x87cache[i]!=-1) {
                int st = dyn->n.x87cache[i]+dyn->n.stack_pop;
                // don't force promotion here
                ADDw_U12(s3, s2, dyn->n.x87cache[i]);   // unadjusted count, as it's relative to real top
                ANDw_mask(s3, s3, 0, 2); //mask=7   // (emu->top + st)&7
                switch(neoncache_get_current_st(dyn, ninst, st)) {
                    case NEON_CACHE_ST_D:
                        VSTR64_REG_LSL3(dyn->n.x87reg[i], s1, s3);    // save the value
                        break;
                    case NEON_CACHE_ST_F:
                        {
                            int scratch = fpu_get_scratch(dyn, ninst);
                            FCVT_D_S(scratch, dyn->n.x87reg[i]);
                            VSTR64_REG_LSL3(scratch, s1, s3);    // save the value
                            fpu_free_reg(dyn, scratch);
                        }
                        break;
                    case NEON_CACHE_ST_I64:
                        {
                            int scratch = fpu_get_scratch(dyn, ninst);
                            SCVTFDD(scratch, dyn->n.x87reg[i]);
                            VSTR64_REG_LSL3(scratch, s1, s3);    // save the value
                            fpu_free_reg(dyn, scratch);
                        }
                        break;
                }
                if(!next) {
                    fpu_free_reg(dyn, dyn->n.x87reg[i]);
                    dyn->n.x87reg[i] = -1;
                    dyn->n.x87cache[i] = -1;
                    //dyn->n.stack_pop+=1; //no pop, but the purge because of barrier will have the n.barrier flags set
                }
            }
    }
    if(!next) {
        dyn->n.stack_next = 0;
        dyn->n.tags = 0;
        #if STEP < 2
        // refresh the cached valued, in case it's a purge outside a instruction
        dyn->insts[ninst].n.barrier = 1;
        dyn->n.pushed = 0;
        dyn->n.poped = 0;

        #endif
    }
    MESSAGE(LOG_DUMP, "\t---Purge x87 Cache and Synch Stackcount\n");
}
void x87_reflectcount(dynarec_arm_t* dyn, int ninst, int s1, int s2)
{
    // Synch top & stack counter
    int a = dyn->n.x87stack;
    if(a) {
        MESSAGE(LOG_DUMP, "\tSync x87 Count of %d-----\n", a);
        // Add x87stack to emu fpu_stack
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        if(a>0) {
            ADDw_U12(s2, s2, a);
        } else {
            SUBw_U12(s2, s2, -a);
        }
        STRw_U12(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        // Sub x87stack to top, with and 7
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
        if(a>0) {
            SUBw_U12(s2, s2, a);
        } else {
            ADDw_U12(s2, s2, -a);
        }
        ANDw_mask(s2, s2, 0, 2);  //mask=7
        STRw_U12(s2, xEmu, offsetof(x64emu_t, top));
        // update tags
        LDRH_U12(s1, xEmu, offsetof(x64emu_t, fpu_tags));
        if(a>0) {
            LSLw_IMM(s1, s1, a*2);
        } else {
            ORRw_mask(s1, s1, 0b010000, 0b001111);  // 0xffff0000
            LSRw_IMM(s1, s1, -a*2);
        }
        STRH_U12(s1, xEmu, offsetof(x64emu_t, fpu_tags));
        MESSAGE(LOG_DUMP, "\t-----Sync x87 Count\n");
    }
}
static void x87_reflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    // Synch top & stack counter
    int a = dyn->n.x87stack;
    if(a) {
        // Add x87stack to emu fpu_stack
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        if(a>0) {
            ADDw_U12(s2, s2, a);
        } else {
            SUBw_U12(s2, s2, -a);
        }
        STRw_U12(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        // Sub x87stack to top, with and 7
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
        if(a>0) {
            SUBw_U12(s2, s2, a);
        } else {
            ADDw_U12(s2, s2, -a);
        }
        ANDw_mask(s2, s2, 0, 2);  //mask=7
        STRw_U12(s2, xEmu, offsetof(x64emu_t, top));
        // update tags
        LDRH_U12(s1, xEmu, offsetof(x64emu_t, fpu_tags));
        if(a>0) {
            LSLw_IMM(s1, s1, a*2);
        } else {
            ORRw_mask(s1, s1, 0b010000, 0b001111);  // 0xffff0000
            LSRw_IMM(s1, s1, -a*2);
        }
        STRH_U12(s1, xEmu, offsetof(x64emu_t, fpu_tags));
    }
    int ret = 0;
    for (int i=0; (i<8) && (!ret); ++i)
        if(dyn->n.x87cache[i] != -1)
            ret = 1;
    if(!ret)    // nothing to do
        return;
    // prepare offset to fpu => s1
    ADDx_U12(s1, xEmu, offsetof(x64emu_t, x87));
    // Get top
    if(!a) {
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
    }
    // loop all cache entries
    for (int i=0; i<8; ++i)
        if(dyn->n.x87cache[i]!=-1) {
            ADDw_U12(s3, s2, dyn->n.x87cache[i]);
            ANDw_mask(s3, s3, 0, 2); // mask=7   // (emu->top + i)&7
            if(neoncache_get_current_st_f(dyn, dyn->n.x87cache[i])>=0) {
                int scratch = fpu_get_scratch(dyn, ninst);
                FCVT_D_S(scratch, dyn->n.x87reg[i]);
                VSTR64_REG_LSL3(scratch, s1, s3);
                fpu_free_reg(dyn, scratch);
            } else
                VSTR64_REG_LSL3(dyn->n.x87reg[i], s1, s3);
        }
}

void x87_unreflectcount(dynarec_arm_t* dyn, int ninst, int s1, int s2)
{
    // go back with the top & stack counter
    int a = dyn->n.x87stack;
    if(a) {
        // Sub x87stack to emu fpu_stack
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        if(a>0) {
            SUBw_U12(s2, s2, a);
        } else {
            ADDw_U12(s2, s2, -a);
        }
        STRw_U12(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        // Add x87stack to top, with and 7
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
        if(a>0) {
            ADDw_U12(s2, s2, a);
        } else {
            SUBw_U12(s2, s2, -a);
        }
        ANDw_mask(s2, s2, 0, 2);  //mask=7
        STRw_U12(s2, xEmu, offsetof(x64emu_t, top));
        // update tags
        LDRH_U12(s1, xEmu, offsetof(x64emu_t, fpu_tags));
        if(a>0) {
            ORRw_mask(s1, s1, 0b010000, 0b001111);  // 0xffff0000
            LSRw_IMM(s1, s1, a*2);
        } else {
            LSLw_IMM(s1, s1, -a*2);
        }
        STRH_U12(s1, xEmu, offsetof(x64emu_t, fpu_tags));
    }
}

int x87_get_current_cache(dynarec_arm_t* dyn, int ninst, int st, int t)
{
    dyn->insts[ninst].x87_used = 1;
    // search in cache first
    for (int i=0; i<8; ++i) {
        if(dyn->n.x87cache[i]==st) {
            #if STEP == 1
            if(t==NEON_CACHE_ST_D && (dyn->n.neoncache[dyn->n.x87reg[i]].t==NEON_CACHE_ST_F || dyn->n.neoncache[dyn->n.x87reg[i]].t==NEON_CACHE_ST_I64))
                neoncache_promote_double(dyn, ninst, st);
            else if(t==NEON_CACHE_ST_I64 && (dyn->n.neoncache[dyn->n.x87reg[i]].t==NEON_CACHE_ST_F))
                neoncache_promote_double(dyn, ninst, st);
            else if(t==NEON_CACHE_ST_F && (dyn->n.neoncache[dyn->n.x87reg[i]].t==NEON_CACHE_ST_I64))
                neoncache_promote_double(dyn, ninst, st);
            #endif
            return i;
        }
    }
    return -1;
}

int x87_get_cache(dynarec_arm_t* dyn, int ninst, int populate, int s1, int s2, int st, int t)
{
    dyn->insts[ninst].x87_used = 1;
    if(dyn->n.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    int ret = x87_get_current_cache(dyn, ninst, st, t);
    if(ret!=-1)
        return ret;
    MESSAGE(LOG_DUMP, "\tCreate %sx87 Cache for ST%d\n", populate?"and populate ":"", st);
    // get a free spot
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->n.x87cache[i]==-1)
            ret = i;
    // found, setup and grab the value
    dyn->n.x87cache[ret] = st;
    dyn->n.x87reg[ret] = fpu_get_reg_x87(dyn, ninst, NEON_CACHE_ST_D, st);
    if(populate) {
        ADDx_U12(s1, xEmu, offsetof(x64emu_t, x87));
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
        int a = st - dyn->n.x87stack;
        if(a) {
            if(a<0) {
                SUBw_U12(s2, s2, -a);
            } else {
                ADDw_U12(s2, s2, a);
            }
            ANDw_mask(s2, s2, 0, 2); //mask=7
        }
        VLDR64_REG_LSL3(dyn->n.x87reg[ret], s1, s2);
    }
    MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);

    return ret;
}
int x87_get_neoncache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
    dyn->insts[ninst].x87_used = 1;
    for(int ii=0; ii<24; ++ii)
        if((dyn->n.neoncache[ii].t == NEON_CACHE_ST_F
         || dyn->n.neoncache[ii].t == NEON_CACHE_ST_D
         || dyn->n.neoncache[ii].t == NEON_CACHE_ST_I64)
         && dyn->n.neoncache[ii].n==st)
            return ii;
    dynarec_log(LOG_NONE, "Warning: x87_get_neoncache didn't find cache for ninst=%d\n", ninst);
    return -1;
}
int x87_get_st(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a, int t)
{
    dyn->insts[ninst].x87_used = 1;
    return dyn->n.x87reg[x87_get_cache(dyn, ninst, 1, s1, s2, a, t)];
}
int x87_get_st_empty(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a, int t)
{
    dyn->insts[ninst].x87_used = 1;
    return dyn->n.x87reg[x87_get_cache(dyn, ninst, 0, s1, s2, a, t)];
}

void x87_forget(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->n.x87cache[i] == st)
            ret = i;
    if(ret==-1)    // nothing to do
        return;
    MESSAGE(LOG_DUMP, "\tForget x87 Cache for ST%d\n", st);
    const int reg = dyn->n.x87reg[ret];
    #if STEP == 1
    if(dyn->n.neoncache[reg].t==NEON_CACHE_ST_F || dyn->n.neoncache[reg].t==NEON_CACHE_ST_I64)
        neoncache_promote_double(dyn, ninst, st);
    #endif
    // prepare offset to fpu => s1
    ADDx_U12(s1, xEmu, offsetof(x64emu_t, x87));
    // Get top
    LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
    // Update
    int ast = st - dyn->n.x87stack;
    if(ast) {
        if(ast>0) {
            ADDw_U12(s2, s2, ast);
        } else {
            SUBw_U12(s2, s2, -ast);
        }
        ANDw_mask(s2, s2, 0, 2); //mask=7    // (emu->top + i)&7
    }
    if(dyn->n.neoncache[reg].t==NEON_CACHE_ST_F) {
        int scratch = fpu_get_scratch(dyn, ninst);
        FCVT_D_S(scratch, reg);
        VSTR64_REG_LSL3(scratch, s1, s2);
        fpu_free_reg(dyn, scratch);
    } else if(dyn->n.neoncache[reg].t==NEON_CACHE_ST_I64) {
        int scratch = fpu_get_scratch(dyn, ninst);
        SCVTFDD(scratch, reg);
        VSTR64_REG_LSL3(scratch, s1, s2);
        fpu_free_reg(dyn, scratch);
    } else {
        VSTR64_REG_LSL3(reg, s1, s2);
    }
    MESSAGE(LOG_DUMP, "\t--------x87 Cache for ST%d\n", st);
    // and forget that cache
    fpu_free_reg(dyn, reg);
    dyn->n.neoncache[reg].v = 0;
    dyn->n.x87cache[ret] = -1;
    dyn->n.x87reg[ret] = -1;
}

void x87_reget_st(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
    dyn->insts[ninst].x87_used = 1;
    if(dyn->n.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    // search in cache first
    for (int i=0; i<8; ++i)
        if(dyn->n.x87cache[i]==st) {
            // refresh the value
            MESSAGE(LOG_DUMP, "\tRefresh x87 Cache for ST%d\n", st);
            #if STEP == 1
            if(dyn->n.neoncache[dyn->n.x87reg[i]].t==NEON_CACHE_ST_F || dyn->n.neoncache[dyn->n.x87reg[i]].t==NEON_CACHE_ST_I64)
                neoncache_promote_double(dyn, ninst, st);
            #endif
            ADDx_U12(s1, xEmu, offsetof(x64emu_t, x87));
            LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
            int a = st - dyn->n.x87stack;
            if(a<0) {
                SUBw_U12(s2, s2, -a);
            } else {
                ADDw_U12(s2, s2, a);
            }
            ANDw_mask(s2, s2, 0, 2); //mask=7    // (emu->top + i)&7
            VLDR64_REG_LSL3(dyn->n.x87reg[i], s1, s2);
            MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);
            // ok
            return;
        }
    // Was not in the cache? creating it....
    MESSAGE(LOG_DUMP, "\tCreate x87 Cache for ST%d\n", st);
    // get a free spot
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->n.x87cache[i]==-1)
            ret = i;
    // found, setup and grab the value
    dyn->n.x87cache[ret] = st;
    dyn->n.x87reg[ret] = fpu_get_reg_x87(dyn, ninst, NEON_CACHE_ST_D, st);
    ADDx_U12(s1, xEmu, offsetof(x64emu_t, x87));
    LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
    int a = st - dyn->n.x87stack;
    if(a<0) {
        SUBw_U12(s2, s2, -a);
    } else {
        ADDw_U12(s2, s2, a);
    }
    ANDw_mask(s2, s2, 0, 2); //mask=7    // (emu->top + i)&7
    VLDR64_REG_LSL3(dyn->n.x87reg[ret], s1, s2);
    MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);
}

void x87_free(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int st)
{
    dyn->insts[ninst].x87_used = 1;
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->n.x87cache[i] == st)
            ret = i;
    MESSAGE(LOG_DUMP, "\tFFREE%s x87 Cache for ST%d\n", (ret!=-1)?" (and Forget)":"", st);
    if(ret!=-1) {
        const int reg = dyn->n.x87reg[ret];
        #if STEP == 1
        if(dyn->n.neoncache[reg].t==NEON_CACHE_ST_F || dyn->n.neoncache[reg].t==NEON_CACHE_ST_I64)
            neoncache_promote_double(dyn, ninst, st);
        #endif
        // prepare offset to fpu => s1
        ADDx_U12(s1, xEmu, offsetof(x64emu_t, x87));
        // Get top
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
        // Update
        int ast = st - dyn->n.x87stack;
        if(ast) {
            if(ast>0) {
                ADDw_U12(s2, s2, ast);
            } else {
                SUBw_U12(s2, s2, -ast);
            }
            ANDw_mask(s2, s2, 0, 2); //mask=7    // (emu->top + i)&7
        }
        if(dyn->n.neoncache[reg].t==NEON_CACHE_ST_F) {
            int scratch = fpu_get_scratch(dyn, ninst);
            FCVT_D_S(scratch, reg);
            VSTR64_REG_LSL3(scratch, s1, s2);
            fpu_free_reg(dyn, scratch);
        } else if(dyn->n.neoncache[reg].t==NEON_CACHE_ST_I64) {
            int scratch = fpu_get_scratch(dyn, ninst);
            SCVTFDD(scratch, reg);
            VSTR64_REG_LSL3(scratch, s1, s2);
            fpu_free_reg(dyn, scratch);
        } else {
            VSTR64_REG_LSL3(reg, s1, s2);
        }
        // and forget that cache
        fpu_free_reg(dyn, reg);
        dyn->n.neoncache[reg].v = 0;
        dyn->n.x87cache[ret] = -1;
        dyn->n.x87reg[ret] = -1;
    } else {
        // Get top
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
        // Update
        int ast = st - dyn->n.x87stack;
        if(ast) {
            if(ast>0) {
                ADDw_U12(s2, s2, ast);
            } else {
                SUBw_U12(s2, s2, -ast);
            }
            ANDw_mask(s2, s2, 0, 2); //mask=7    // (emu->top + i)&7
        }
    }
    // add mark in the freed array
    dyn->n.tags |= 0b11<<(st*2);
    MESSAGE(LOG_DUMP, "\t--------x87 FFREE for ST%d\n", st);
}

void x87_swapreg(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a, int b)
{
    int i1, i2, i3;
    i1 = x87_get_cache(dyn, ninst, 1, s1, s2, b, X87_ST(b));
    i2 = x87_get_cache(dyn, ninst, 1, s1, s2, a, X87_ST(a));
    i3 = dyn->n.x87cache[i1];
    dyn->n.x87cache[i1] = dyn->n.x87cache[i2];
    dyn->n.x87cache[i2] = i3;
    // swap those too
    int j1, j2, j3;
    j1 = x87_get_neoncache(dyn, ninst, s1, s2, b);
    j2 = x87_get_neoncache(dyn, ninst, s1, s2, a);
    j3 = dyn->n.neoncache[j1].n;
    dyn->n.neoncache[j1].n = dyn->n.neoncache[j2].n;
    dyn->n.neoncache[j2].n = j3;
    // mark as swapped
    dyn->n.swapped = 1;
    dyn->n.combined1= a; dyn->n.combined2=b;
}

// Set rounding according to cw flags, return reg to restore flags
int x87_setround(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MAYUSE(s1); MAYUSE(s2);
    LDRw_U12(s1, xEmu, offsetof(x64emu_t, cw));
    BFXILw(s1, s1, 10, 2);
    UBFXw(s2, s1, 1, 1);        // bit 1 of round in bit 0 (zero extented) of s2
    BFIw(s2, s1, 1, 1);         // bit 0 of round in bit 1 of s2
    MRS_fpcr(s1);               // get fpscr
    MOVx_REG(s3, s1);
    BFIx(s1, s2, 22, 2);        // inject new round
    MSR_fpcr(s1);               // put new fpscr
    return s3;
}

// Set rounding according to mxcsr flags, return reg to restore flags
int sse_setround(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MAYUSE(s1); MAYUSE(s2);
    LDRw_U12(s1, xEmu, offsetof(x64emu_t, mxcsr));
    RBITw(s2, s1);              // round is on bits 13-14 on x64,
    LSRw(s2, s2, 17);           // but we want the reverse of that
    MRS_fpcr(s1);               // get fpscr
    MOVx_REG(s3, s1);
    BFIx(s1, s2, 22, 2);     // inject new round
    MSR_fpcr(s1);               // put new fpscr
    return s3;
}

// Restore round flag
void x87_restoreround(dynarec_arm_t* dyn, int ninst, int s1)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MAYUSE(s1);
    MSR_fpcr(s1);               // put back fpscr
}

// MMX helpers
static int isx87Empty(dynarec_arm_t* dyn)
{
    for (int i=0; i<8; ++i)
        if(dyn->n.x87cache[i] != -1)
            return 0;
    return 1;
}

// get neon register for a MMX reg, create the entry if needed
int mmx_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int a)
{
    dyn->insts[ninst].mmx_used = 1;
    if(!dyn->n.x87stack && isx87Empty(dyn))
        x87_purgecache(dyn, ninst, 0, s1, s2, s3);
    if(dyn->n.mmxcache[a]!=-1)
        return dyn->n.mmxcache[a];
    ++dyn->n.mmxcount;
    int ret = dyn->n.mmxcache[a] = fpu_get_reg_emm(dyn, ninst, a);
    VLDR64_U12(ret, xEmu, offsetof(x64emu_t, mmx[a]));
    return ret;
}
// get neon register for a MMX reg, but don't try to synch it if it needed to be created
int mmx_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int a)
{
    dyn->insts[ninst].mmx_used = 1;
    if(!dyn->n.x87stack && isx87Empty(dyn))
        x87_purgecache(dyn, ninst, 0, s1, s2, s3);
    if(dyn->n.mmxcache[a]!=-1)
        return dyn->n.mmxcache[a];
    ++dyn->n.mmxcount;
    int ret = dyn->n.mmxcache[a] = fpu_get_reg_emm(dyn, ninst, a);
    return ret;
}
// purge the MMX cache only(needs 3 scratch registers)
void mmx_purgecache(dynarec_arm_t* dyn, int ninst, int next, int s1)
{
    if(!dyn->n.mmxcount)
        return;
    if(!next)
        dyn->n.mmxcount = 0;
    int old = -1;
    for (int i=0; i<8; ++i)
        if(dyn->n.mmxcache[i]!=-1) {
            if (old==-1) {
                MESSAGE(LOG_DUMP, "\tPurge %sMMX Cache ------\n", next?"locally ":"");
                ++old;
            }
            VSTR64_U12(dyn->n.mmxcache[i], xEmu, offsetof(x64emu_t, mmx[i]));
            if(!next) {
                fpu_free_reg(dyn, dyn->n.mmxcache[i]);
                dyn->n.mmxcache[i] = -1;
            }
        }
    if(old!=-1) {
        MESSAGE(LOG_DUMP, "\t------ Purge MMX Cache\n");
    }
}

static void mmx_reflectcache(dynarec_arm_t* dyn, int ninst, int s1)
{
    for (int i=0; i<8; ++i)
        if(dyn->n.mmxcache[i]!=-1) {
            VSTR64_U12(dyn->n.mmxcache[i], xEmu, offsetof(x64emu_t, mmx[i]));
        }
}


// SSE / SSE2 helpers
// get neon register for a SSE reg, create the entry if needed
int sse_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int a, int forwrite)
{
    dyn->n.xmm_used |= 1<<a;
    if(dyn->n.ssecache[a].v!=-1) {
        if(forwrite) {
            dyn->n.ssecache[a].write = 1;    // update only if forwrite
            dyn->n.neoncache[dyn->n.ssecache[a].reg].t = NEON_CACHE_XMMW;
        }
        return dyn->n.ssecache[a].reg;
    }
    dyn->n.ssecache[a].reg = fpu_get_reg_xmm(dyn, forwrite?NEON_CACHE_XMMW:NEON_CACHE_XMMR, a);
    int ret =  dyn->n.ssecache[a].reg;
    dyn->n.ssecache[a].write = forwrite;
    VLDR128_U12(ret, xEmu, offsetof(x64emu_t, xmm[a]));
    return ret;
}
// get neon register for a SSE reg, but don't try to synch it if it needed to be created
int sse_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int a)
{
    dyn->n.xmm_used |= 1<<a;
    if(dyn->n.ssecache[a].v!=-1) {
        dyn->n.ssecache[a].write = 1;
        dyn->n.neoncache[dyn->n.ssecache[a].reg].t = NEON_CACHE_XMMW;
        return dyn->n.ssecache[a].reg;
    }
    dyn->n.xmm_unneeded |= 1<<a;
    dyn->n.ssecache[a].reg = fpu_get_reg_xmm(dyn, NEON_CACHE_XMMW, a);
    dyn->n.ssecache[a].write = 1; // it will be write...
    return dyn->n.ssecache[a].reg;
}
// forget neon register for a SSE reg, create the entry if needed
void sse_forget_reg(dynarec_arm_t* dyn, int ninst, int a)
{
    dyn->n.xmm_used |= 1<<a;
    if(dyn->n.ssecache[a].v==-1)
        return;
    if(dyn->n.neoncache[dyn->n.ssecache[a].reg].t == NEON_CACHE_XMMW) {
        VSTR128_U12(dyn->n.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
    }
    // YMM part too
    if(is_avx_zero_unset(dyn, ninst, a)) {
        //only  ymm[0] can be accessed with STP :(
        if(!a)
            STPx_S7_offset(xZR, xZR, xEmu, offsetof(x64emu_t, ymm[a]));
        else {
            STRx_U12(xZR, xEmu, offsetof(x64emu_t, ymm[a]));
            STRx_U12(xZR, xEmu, offsetof(x64emu_t, ymm[a])+8);
        }
    } else for(int i=0; i<32; ++i)
        if(((dyn->n.neoncache[i].t == NEON_CACHE_YMMW) || (dyn->n.neoncache[i].t == NEON_CACHE_YMMR)) && (dyn->n.neoncache[i].n==a)) {
            if(dyn->n.neoncache[i].t == NEON_CACHE_YMMW)
                VSTR128_U12(i, xEmu, offsetof(x64emu_t, ymm[dyn->n.neoncache[i].n]));
            fpu_free_reg(dyn, i);

    }
    fpu_free_reg(dyn, dyn->n.ssecache[a].reg);
    dyn->n.ssecache[a].v = -1;
    return;
}
// purge the SSE cache for XMM0..XMM7 (to use before function native call)
void sse_purge07cache(dynarec_arm_t* dyn, int ninst, int s1)
{
    int old = -1;
    for (int i=0; i<8; ++i)
        if(dyn->n.ssecache[i].v!=-1) {
            if (old==-1) {
                MESSAGE(LOG_DUMP, "\tPurge XMM0..7 Cache ------\n");
                ++old;
            }
            dyn->n.xmm_used |= (1<<i);
            if(dyn->n.neoncache[dyn->n.ssecache[i].reg].t == NEON_CACHE_XMMW) {
                VSTR128_U12(dyn->n.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            }
            fpu_free_reg(dyn, dyn->n.ssecache[i].reg);
            dyn->n.ssecache[i].v = -1;
        }
    if(old!=-1) {
        MESSAGE(LOG_DUMP, "\t------ Purge XMM0..7 Cache\n");
    }
}

// purge the SSE cache only
static void sse_purgecache(dynarec_arm_t* dyn, int ninst, int next, int s1, uint32_t unneeded)
{
    int old = -1;
    for (int i=0; i<16; ++i)
        if(dyn->n.ssecache[i].v!=-1 && !(unneeded&(1<<i))) {
            if(next) dyn->n.xmm_used |= (1<<i);
            if(dyn->n.ssecache[i].write) {
                if (old==-1) {
                    MESSAGE(LOG_DUMP, "\tPurge %sSSE Cache ------\n", next?"localy ":"");
                    ++old;
                }
                VSTR128_U12(dyn->n.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            }
            if(!next) {
                fpu_free_reg(dyn, dyn->n.ssecache[i].reg);
                dyn->n.ssecache[i].v = -1;
            }
        }
    //AVX
    if(dyn->ymm_zero) {
        if (old==-1) {
            MESSAGE(LOG_DUMP, "\tPurge %sSSE Cache ------\n", next?"locally ":"");
            ++old;
        }
        int s1_set = 0;
        for(int i=0; i<16; ++i)
            if(is_avx_zero(dyn, ninst, i)) {
                if(!s1_set) {
                    ADDx_U12(s1, xEmu, offsetof(x64emu_t, ymm[0]));
                    s1_set = 1;
                }
                STPx_S7_offset(xZR, xZR, s1, i*16);
            }
        if(!next)
            avx_mark_zero_reset(dyn, ninst);
    }
    for(int i=0; i<32; ++i) {
        if(dyn->n.neoncache[i].t==NEON_CACHE_YMMW && !(unneeded&(1<<(dyn->n.neoncache[i].n+16)))) {
            if (old==-1) {
                MESSAGE(LOG_DUMP, "\tPurge %sSSE Cache ------\n", next?"locally ":"");
                ++old;
            }
            VSTR128_U12(i, xEmu, offsetof(x64emu_t, ymm[dyn->n.neoncache[i].n]));
        }
        if(!next && (dyn->n.neoncache[i].t==NEON_CACHE_YMMW || dyn->n.neoncache[i].t==NEON_CACHE_YMMR))
            dyn->n.neoncache[i].v = 0;
        if(next && (dyn->n.neoncache[i].t==NEON_CACHE_YMMW || dyn->n.neoncache[i].t==NEON_CACHE_YMMR))
            dyn->n.xmm_used |= (1<<dyn->n.neoncache[i].n);
    }
    // All done
    if(old!=-1) {
        MESSAGE(LOG_DUMP, "\t------ Purge SSE Cache\n");
    }
}

static void sse_reflectcache(dynarec_arm_t* dyn, int ninst, int s1)
{
    for (int i=0; i<16; ++i) {
        if(dyn->n.ssecache[i].v!=-1) {
            dyn->n.xmm_used |= 1<<i;
            if(dyn->n.ssecache[i].write) {
                VSTR128_U12(dyn->n.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            }
        }
    }
    //AVX
    if(dyn->ymm_zero) {
        int s1_set = 0;
        for(int i=0; i<16; ++i)
            if(is_avx_zero(dyn, ninst, i)) {
                if(!s1_set) {
                    ADDx_U12(s1, xEmu, offsetof(x64emu_t, ymm[0]));
                    s1_set = 1;
                }
                STPx_S7_offset(xZR, xZR, s1, i*16);
            }
    }
    for(int i=0; i<32; ++i) {
        if(dyn->n.neoncache[i].t == NEON_CACHE_YMMW)
            VSTR128_U12(i, xEmu, offsetof(x64emu_t, ymm[dyn->n.neoncache[i].n]));
        if((dyn->n.neoncache[i].t == NEON_CACHE_YMMW) || (dyn->n.neoncache[i].t == NEON_CACHE_YMMR))
            dyn->n.xmm_used |= 1<<dyn->n.neoncache[i].n;
    }
}

void sse_reflect_reg(dynarec_arm_t* dyn, int ninst, int a)
{
    dyn->n.xmm_used |= 1<<a;
    dyn->n.ymm_used |= 1<<a;
    if(is_avx_zero(dyn, ninst, a)) {
        //only  ymm[0] can be accessed with STP :(
        if(!a)
            STPx_S7_offset(xZR, xZR, xEmu, offsetof(x64emu_t, ymm[a]));
        else {
            STRx_U12(xZR, xEmu, offsetof(x64emu_t, ymm[a]));
            STRx_U12(xZR, xEmu, offsetof(x64emu_t, ymm[a])+8);
        }
    } else for(int i=0; i<32; ++i)
        if((dyn->n.neoncache[i].t == NEON_CACHE_YMMW) && (dyn->n.neoncache[i].n == a)) {
            VSTR128_U12(i, xEmu, offsetof(x64emu_t, ymm[a]));
    }
    if(dyn->n.ssecache[a].v==-1)
        return;
    if(dyn->n.neoncache[dyn->n.ssecache[a].reg].t == NEON_CACHE_XMMW) {
        VSTR128_U12(dyn->n.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
        /*dyn->n.neoncache[dyn->n.ssecache[a].reg].t = NEON_CACHE_XMMR;
        dyn->n.ssecache[a].write = 0;*/
    }
}

// AVX Helpers
// get neon register for a YMM upper reg, create the entry if needed
int ymm_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int a, int forwrite, int k1, int k2, int k3)
{
    // look if already exist
    for(int i=0; i<32; ++i)
        if((dyn->n.neoncache[i].t==NEON_CACHE_YMMR || dyn->n.neoncache[i].t==NEON_CACHE_YMMW) && dyn->n.neoncache[i].n==a) {
            if(forwrite) {
                dyn->n.neoncache[i].t = NEON_CACHE_YMMW;
            }
            dyn->ymm_zero&=~(1<<a);
            dyn->n.ymm_used|=(1<<a);
            #if STEP == 0
            dyn->insts[ninst].ymm0_sub |= (1<<a);
            #endif
            return i;
        }
    // nope, grab a new one
    if(dyn->ymm_zero&(1<<a))
        forwrite = 1;   // if the reg was zero, then it will need to be write back
    int ret =  fpu_get_reg_ymm(dyn, ninst, forwrite?NEON_CACHE_YMMW:NEON_CACHE_YMMR, a, k1, k2, k3);
    if(dyn->ymm_zero&(1<<a)) {
        VEORQ(ret, ret, ret);
        dyn->ymm_zero&=~(1<<a);
    } else {
        VLDR128_U12(ret, xEmu, offsetof(x64emu_t, ymm[a]));
    }
    #if STEP == 0
    dyn->insts[ninst].ymm0_sub |= (1<<a);
    #endif
    return ret;
}
// get neon register for a YMM reg, but don't try to synch it if it needed to be created
int ymm_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int a, int k1, int k2, int k3)
{
    // look if already exist
    for(int i=0; i<32; ++i)
        if((dyn->n.neoncache[i].t==NEON_CACHE_YMMR || dyn->n.neoncache[i].t==NEON_CACHE_YMMW) && dyn->n.neoncache[i].n==a) {
            dyn->n.neoncache[i].t = NEON_CACHE_YMMW;
            dyn->ymm_zero&=~(1<<a);
            dyn->n.ymm_used|=(1<<a);
            #if STEP == 0
            dyn->insts[ninst].ymm0_sub |= (1<<a);
            #endif
            return i;
        }
    // nope, grab a new one
    dyn->n.ymm_unneeded |= 1<<a;
    int ret =  fpu_get_reg_ymm(dyn, ninst, NEON_CACHE_YMMW, a, k1, k2, k3);
    if(dyn->ymm_zero&(1<<a))
        dyn->ymm_zero&=~(1<<a);
    #if STEP == 0
    dyn->insts[ninst].ymm0_sub |= (1<<a);
    #endif
    return ret;
}

void ymm_mark_zero(dynarec_arm_t* dyn, int ninst, int a)
{
    // look if already exist
    for(int i=0; i<32; ++i)
        if((dyn->n.neoncache[i].t==NEON_CACHE_YMMR || dyn->n.neoncache[i].t==NEON_CACHE_YMMW) && dyn->n.neoncache[i].n==a) {
            if(dyn->n.ymm_used&(1<<a)) {
                // special case, the reg was just added in the opcode and cannot be marked as 0, so just RAZ it now
                dyn->n.neoncache[i].t = NEON_CACHE_YMMW;
                VEORQ(i, i, i);
                return;
            }
            fpu_free_reg(dyn, i);
        }
    #if STEP == 0
    dyn->insts[ninst].ymm0_add |= (1<<a);
    #endif
    dyn->n.ymm_unneeded |= 1<<a;
    avx_mark_zero(dyn, ninst, a);
}

void fpu_pushcache(dynarec_arm_t* dyn, int ninst, int s1, int not07)
{
    int start = not07?8:0;
    // only SSE regs needs to be push back to xEmu (needs to be "write")
    int n=0;
    for (int i=start; i<16; i++) {
        if((dyn->n.ssecache[i].v!=-1) && (dyn->n.ssecache[i].write))
            ++n;
    }
    for(int i=0; i<32; ++i)
        if(dyn->n.neoncache[i].t==NEON_CACHE_YMMW)
            ++n;
    if(!n)
        return;
    MESSAGE(LOG_DUMP, "\tPush XMM Cache (%d)------\n", n);
    for (int i=start; i<16; ++i) {
        if((dyn->n.ssecache[i].v!=-1) && (dyn->n.ssecache[i].write)) {
            VSTR128_U12(dyn->n.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
        }
    }
    // push the YMM values
    for(int i=0; i<32; ++i) {
        if(dyn->n.neoncache[i].t==NEON_CACHE_YMMW)
            VSTR128_U12(i, xEmu, offsetof(x64emu_t, ymm[dyn->n.neoncache[i].n]));
    }
    MESSAGE(LOG_DUMP, "\t------- Push XMM Cache (%d)\n", n);
}

void fpu_popcache(dynarec_arm_t* dyn, int ninst, int s1, int not07)
{
    int start = not07?8:0;
    // only SSE regs needs to be pop back from xEmu (don't need to be "write" this time)
    int n=0;
    for (int i=start; i<16; i++)
        if(dyn->n.ssecache[i].v!=-1)
            ++n;
    for(int i=0; i<32; ++i)
        if(dyn->n.neoncache[i].t==NEON_CACHE_YMMW || dyn->n.neoncache[i].t==NEON_CACHE_YMMR)
            ++n;
    if(!n)
        return;
    MESSAGE(LOG_DUMP, "\tPop XMM Cache (%d)------\n", n);
    for (int i=start; i<16; ++i)
        if(dyn->n.ssecache[i].v!=-1) {
            VLDR128_U12(dyn->n.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            /*dyn->n.ssecache[i].write = 0;   // OPTIM: it's sync, so not write anymore
            dyn->n.neoncache[dyn->n.ssecache[i].reg].t = NEON_CACHE_XMMR;*/
        }
    for(int i=0; i<32; ++i)
        if(dyn->n.neoncache[i].t==NEON_CACHE_YMMW || dyn->n.neoncache[i].t==NEON_CACHE_YMMR)
            VLDR128_U12(i, xEmu, offsetof(x64emu_t, ymm[dyn->n.neoncache[i].n]));
    MESSAGE(LOG_DUMP, "\t------- Pop XMM Cache (%d)\n", n);
}

void fpu_purgecache(dynarec_arm_t* dyn, int ninst, int next, int s1, int s2, int s3, uint32_t unneeded)
{
    x87_purgecache(dyn, ninst, next, s1, s2, s3);
    mmx_purgecache(dyn, ninst, next, s1);
    sse_purgecache(dyn, ninst, next, s1, unneeded);
    if(!next) {
        fpu_reset_reg(dyn);
        dyn->insts[ninst].fpupurge = 1;
    }
}

static int findCacheSlot(dynarec_arm_t* dyn, int ninst, int t, int n, neoncache_t* cache)
{
    neon_cache_t f;
    f.n = n; f.t = t;
    for(int i=0; i<32; ++i) {
        if(cache->neoncache[i].v == f.v)
            return i;
        if(cache->neoncache[i].n == n) {
            switch(cache->neoncache[i].t) {
                case NEON_CACHE_ST_F:
                    if (t==NEON_CACHE_ST_D)
                        return i;
                    if (t==NEON_CACHE_ST_I64)
                        return i;
                    break;
                case NEON_CACHE_ST_D:
                    if (t==NEON_CACHE_ST_F)
                        return i;
                    if (t==NEON_CACHE_ST_I64)
                        return i;
                    break;
                case NEON_CACHE_ST_I64:
                    if (t==NEON_CACHE_ST_F)
                        return i;
                    if (t==NEON_CACHE_ST_D)
                        return i;
                    break;
                case NEON_CACHE_XMMR:
                    if(t==NEON_CACHE_XMMW)
                        return i;
                    break;
                case NEON_CACHE_XMMW:
                    if(t==NEON_CACHE_XMMR)
                        return i;
                    break;
                case NEON_CACHE_YMMR:
                    if(t==NEON_CACHE_YMMW)
                        return i;
                    break;
                case NEON_CACHE_YMMW:
                    if(t==NEON_CACHE_YMMR)
                        return i;
                    break;
            }
        }
    }
    return -1;
}

static void swapCache(dynarec_arm_t* dyn, int ninst, int i, int j, neoncache_t *cache)
{
    if (i==j)
        return;
    int quad = 0;
    if(cache->neoncache[i].t==NEON_CACHE_XMMR || cache->neoncache[i].t==NEON_CACHE_XMMW || cache->neoncache[i].t==NEON_CACHE_YMMR || cache->neoncache[i].t==NEON_CACHE_YMMW)
        quad =1;
    if(cache->neoncache[j].t==NEON_CACHE_XMMR || cache->neoncache[j].t==NEON_CACHE_XMMW || cache->neoncache[j].t==NEON_CACHE_YMMR || cache->neoncache[j].t==NEON_CACHE_YMMW)
        quad =1;

    if(!cache->neoncache[i].v) {
        // a mov is enough, no need to swap
        MESSAGE(LOG_DUMP, "\t  - Moving %d <- %d\n", i, j);
        if(quad) {
            VMOVQ(i, j);
        } else {
            VMOV(i, j);
        }
        cache->neoncache[i].v = cache->neoncache[j].v;
        cache->neoncache[j].v = 0;
        return;
    }
    // SWAP
    neon_cache_t tmp;
    MESSAGE(LOG_DUMP, "\t  - Swapping %d <-> %d\n", i, j);
    // There is no VSWP in Arm64 NEON to swap 2 register contents!
    if(quad) {
        VEORQ(i, i, j);
        VEORQ(j, i, j);
        VEORQ(i, i, j);
    } else {
        VEOR(i, i, j);
        VEOR(j, i, j);
        VEOR(i, i, j);
    }
    tmp.v = cache->neoncache[i].v;
    cache->neoncache[i].v = cache->neoncache[j].v;
    cache->neoncache[j].v = tmp.v;
}

static void loadCache(dynarec_arm_t* dyn, int ninst, int stack_cnt, int s1, int s2, int s3, int* s1_val, int* s2_val, int* s3_top, neoncache_t *cache, int i, int t, int n, int i2)
{
    if(cache->neoncache[i].v) {
        int quad = 0;
        if(t==NEON_CACHE_XMMR || t==NEON_CACHE_XMMW || t==NEON_CACHE_YMMR || t==NEON_CACHE_YMMW)
            quad = 1;
        if(cache->neoncache[i].t==NEON_CACHE_XMMR || cache->neoncache[i].t==NEON_CACHE_XMMW || cache->neoncache[i].t==NEON_CACHE_YMMR || cache->neoncache[i].t==NEON_CACHE_YMMW)
            quad = 1;
        int j = i+1;
        while(cache->neoncache[j].v)
            ++j;
        MESSAGE(LOG_DUMP, "\t  - Moving away %d\n", i);
        if(quad) {
            VMOVQ(j, i);
        } else {
            VMOV(j, i);
        }
        cache->neoncache[j].v = cache->neoncache[i].v;
    }
    switch(t) {
        case NEON_CACHE_XMMR:
        case NEON_CACHE_XMMW:
            if(dyn->insts[i2].n.xmm_unneeded&(1<<n)) {
                MESSAGE(LOG_DUMP, "\t  - ignoring unneeded %s\n", getCacheName(t, n));
            } else {
                MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
                VLDR128_U12(i, xEmu, offsetof(x64emu_t, xmm[n]));
            }
            break;
        case NEON_CACHE_YMMR:
        case NEON_CACHE_YMMW:
            if(dyn->insts[i2].n.ymm_unneeded&(1<<n)) {
                MESSAGE(LOG_DUMP, "\t  - ignoring unneeded %s\n", getCacheName(t, n));
            } else {
                MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
                VLDR128_U12(i, xEmu, offsetof(x64emu_t, ymm[n]));
            }
            break;
        case NEON_CACHE_MM:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            VLDR64_U12(i, xEmu, offsetof(x64emu_t, mmx[n]));
            break;
        case NEON_CACHE_ST_D:
        case NEON_CACHE_ST_F:
        case NEON_CACHE_ST_I64:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            if((*s3_top) == 0xffff) {
                LDRw_U12(s3, xEmu, offsetof(x64emu_t, top));
                *s3_top = 0;
            }
            int a = n  - (*s3_top) - stack_cnt;
            if(a) {
                if(a<0) {
                    SUBw_U12(s3, s3, -a);
                } else {
                    ADDw_U12(s3, s3, a);
                }
                ANDw_mask(s3, s3, 0, 2); //mask=7    // (emu->top + i)&7
            }
            *s3_top += a;
            if(*s2_val!=1) {
                *s2_val = 1;
                ADDx_U12(s2, xEmu, offsetof(x64emu_t, x87));
            }
            VLDR64_REG_LSL3(i, s2, s3);
            if(t==NEON_CACHE_ST_F) {
                FCVT_S_D(i, i);
            }
            if(t==NEON_CACHE_ST_I64) {
                VFCVTZSQD(i, i);
            }
            break;
        case NEON_CACHE_NONE:
        case NEON_CACHE_SCR:
        default:    /* nothing done */
            MESSAGE(LOG_DUMP, "\t  - ignoring %s\n", getCacheName(t, n));
            break;
    }
    cache->neoncache[i].n = n;
    cache->neoncache[i].t = t;
}

static void unloadCache(dynarec_arm_t* dyn, int ninst, int stack_cnt, int s1, int s2, int s3, int* s1_val, int* s2_val, int* s3_top, neoncache_t *cache, int i, int t, int n, int i2)
{
    switch(t) {
        case NEON_CACHE_XMMR:
        case NEON_CACHE_YMMR:
            MESSAGE(LOG_DUMP, "\t  - ignoring %s\n", getCacheName(t, n));
            break;
        case NEON_CACHE_XMMW:
            if(dyn->insts[i2].n.xmm_unneeded&(1<<n)) {
                MESSAGE(LOG_DUMP, "\t  - ignoring unneeded %s\n", getCacheName(t, n));
            } else {
                MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
                VSTR128_U12(i, xEmu, offsetof(x64emu_t, xmm[n]));
            }
            break;
        case NEON_CACHE_YMMW:
            if(dyn->insts[i2].n.ymm_unneeded&(1<<n)) {
                MESSAGE(LOG_DUMP, "\t  - ignoring unneeded %s\n", getCacheName(t, n));
            } else {
                MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
                VSTR128_U12(i, xEmu, offsetof(x64emu_t, ymm[n]));
            }
            break;
        case NEON_CACHE_MM:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            VSTR64_U12(i, xEmu, offsetof(x64emu_t, mmx[n]));
            break;
        case NEON_CACHE_ST_D:
        case NEON_CACHE_ST_F:
        case NEON_CACHE_ST_I64:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            if((*s3_top)==0xffff) {
                LDRw_U12(s3, xEmu, offsetof(x64emu_t, top));
                *s3_top = 0;
            }
            int a = n - (*s3_top) - stack_cnt;
            if(a) {
                if(a<0) {
                    SUBw_U12(s3, s3, -a);
                } else {
                    ADDw_U12(s3, s3, a);
                }
                ANDw_mask(s3, s3, 0, 2); //mask=7    // (emu->top + i)&7
            }
            *s3_top += a;
            if(*s2_val!=1) {
                *s2_val = 1;
                ADDx_U12(s2, xEmu, offsetof(x64emu_t, x87));
            }
            if(t==NEON_CACHE_ST_F) {
                FCVT_D_S(i, i);
            } else if (t==NEON_CACHE_ST_I64) {
                SCVTFDD(i, i);
            }
            VSTR64_REG_LSL3(i, s2, s3);
            break;
        case NEON_CACHE_NONE:
        case NEON_CACHE_SCR:
        default:    /* nothing done */
            MESSAGE(LOG_DUMP, "\t  - ignoring %s\n", getCacheName(t, n));
            break;
    }
    cache->neoncache[i].v = 0;
}

static void fpuCacheTransform(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    int i2 = dyn->insts[ninst].x64.jmp_insts;
    if(i2<0)
        return;
    MESSAGE(LOG_DUMP, "\tCache Transform ---- ninst=%d -> %d\n", ninst, i2);
    uint32_t unneeded = dyn->insts[i2].n.xmm_unneeded | (dyn->insts[i2].n.ymm_unneeded<<16);
    if((!i2) || (dyn->insts[i2].x64.barrier&BARRIER_FLOAT)) {
        int need_purge = 0;
        if(dyn->n.stack_next)
            need_purge = 1;
        for(int i=0; i<24 && !need_purge; ++i)
            if(dyn->n.neoncache[i].v) 
                need_purge = 1;
        if(need_purge) {       // there is something at ninst for i
            fpu_purgecache(dyn, ninst, 1, s1, s2, s3, unneeded);
        }
        MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
        return;
    }
    neoncache_t cache_i2 = dyn->insts[i2].n;
    neoncacheUnwind(&cache_i2);

    if(!cache_i2.stack) {
        int purge = 0;  // default to purge if there is any regs that are not needed at jump
        // but first check if there is regs that can be discarded because unneeded at jump point
        for(int i=0; i<32 && !purge; ++i) {
            if(dyn->insts[ninst].n.neoncache[i].v) {
                int t = dyn->insts[ninst].n.neoncache[i].t;
                int n = dyn->insts[ninst].n.neoncache[i].n;
                if(((t==NEON_CACHE_XMMR) || (t==NEON_CACHE_XMMW)) && (cache_i2.xmm_unneeded&(1<<n))) {/* nothing */}
                else if(((t==NEON_CACHE_YMMR) || (t==NEON_CACHE_YMMW)) && (cache_i2.ymm_unneeded&(1<<n))) {/* nothing */}
                else ++purge;
            }
        }
        // Now check if there is any regs at jump point
        for (int i=0; i<32 && purge; ++i)
            if(cache_i2.neoncache[i].v)
                purge = 0;
        if(purge) {
            fpu_purgecache(dyn, ninst, 1, s1, s2, s3, unneeded);
            MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
            return;
        }
    }
    int stack_cnt = dyn->n.stack_next;
    neoncache_t cache = dyn->n;
    int s1_val = 0;
    int s2_val = 0;
    // unload every unneeded cache
    // ymm0 first
    int s3_top = 1;
    uint16_t to_purge = dyn->ymm_zero&~dyn->insts[i2].ymm0_in;
    if(dyn->ymm_zero && (dyn->insts[i2].purge_ymm|to_purge)) {
        MESSAGE(LOG_DUMP, "\t- YMM Zero %04x / %04x\n", dyn->ymm_zero, (dyn->insts[i2].purge_ymm|to_purge));
        for(int i=0; i<16; ++i)
            if(is_avx_zero(dyn, ninst, i) && (dyn->insts[i2].purge_ymm|to_purge)&(1<<i)) {
                if(!(dyn->insts[i2].n.ymm_unneeded&(1<<i))) {
                    if(s3_top) {
                        ADDx_U12(s3, xEmu,offsetof(x64emu_t, ymm[0]));
                        s3_top = 0;
                    }
                    STPx_S7_offset(xZR, xZR, s3, i*16);
                }
            }
    }
    s3_top = 0xffff;
    // check SSE first, than MMX, in order, to optimize successive memory write
    for(int i=0; i<16; ++i) {
        int j=findCacheSlot(dyn, ninst, NEON_CACHE_XMMW, i, &cache);
        if(j>=0 && findCacheSlot(dyn, ninst, NEON_CACHE_XMMW, i, &cache_i2)==-1)
            unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, j, cache.neoncache[j].t, cache.neoncache[j].n, i2);
    }
    for(int i=0; i<16; ++i) {
        int j=findCacheSlot(dyn, ninst, NEON_CACHE_YMMW, i, &cache);
        if(j>=0 && findCacheSlot(dyn, ninst, NEON_CACHE_YMMW, i, &cache_i2)==-1)
            unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, j, cache.neoncache[j].t, cache.neoncache[j].n, i2);
    }
    for(int i=0; i<8; ++i) {
        int j=findCacheSlot(dyn, ninst, NEON_CACHE_MM, i, &cache);
        if(j>=0 && findCacheSlot(dyn, ninst, NEON_CACHE_MM, i, &cache_i2)==-1)
            unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, j, cache.neoncache[j].t, cache.neoncache[j].n, i2);
    }
    for(int i=0; i<32; ++i) {
        if(cache.neoncache[i].v)
            if(findCacheSlot(dyn, ninst, cache.neoncache[i].t, cache.neoncache[i].n, &cache_i2)==-1)
                unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, i, cache.neoncache[i].t, cache.neoncache[i].n, i2);
    }
    // and now load/swap the missing one
    for(int i=0; i<32; ++i) {
        if(cache_i2.neoncache[i].v) {
            if(cache_i2.neoncache[i].v != cache.neoncache[i].v) {
                int j;
                if((j=findCacheSlot(dyn, ninst, cache_i2.neoncache[i].t, cache_i2.neoncache[i].n, &cache))==-1)
                    loadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, i, cache_i2.neoncache[i].t, cache_i2.neoncache[i].n, i2);
                else {
                    // it's here, lets swap if needed
                    if(j!=i)
                        swapCache(dyn, ninst, i, j, &cache);
                }
            }
            if(cache.neoncache[i].t != cache_i2.neoncache[i].t) {
                if(cache.neoncache[i].t == NEON_CACHE_ST_D && cache_i2.neoncache[i].t == NEON_CACHE_ST_F) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.neoncache[i].t, cache.neoncache[i].n));
                    FCVT_S_D(i, i);
                    cache.neoncache[i].t = NEON_CACHE_ST_F;
                } else if(cache.neoncache[i].t == NEON_CACHE_ST_F && cache_i2.neoncache[i].t == NEON_CACHE_ST_D) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.neoncache[i].t, cache.neoncache[i].n));
                    FCVT_D_S(i, i);
                    cache.neoncache[i].t = NEON_CACHE_ST_D;
                } else if(cache.neoncache[i].t == NEON_CACHE_ST_D && cache_i2.neoncache[i].t == NEON_CACHE_ST_I64) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.neoncache[i].t, cache.neoncache[i].n));
                    VFCVTZSQD(i, i);
                    cache.neoncache[i].t = NEON_CACHE_ST_I64;
                } else if(cache.neoncache[i].t == NEON_CACHE_ST_F && cache_i2.neoncache[i].t == NEON_CACHE_ST_I64) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.neoncache[i].t, cache.neoncache[i].n));
                    VFCVTZSQS(i, i);
                    cache.neoncache[i].t = NEON_CACHE_ST_D;
                } else if(cache.neoncache[i].t == NEON_CACHE_ST_I64 && cache_i2.neoncache[i].t == NEON_CACHE_ST_F) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.neoncache[i].t, cache.neoncache[i].n));
                    SCVTFDD(i, i);
                    FCVT_S_D(i, i);
                    cache.neoncache[i].t = NEON_CACHE_ST_F;
                } else if(cache.neoncache[i].t == NEON_CACHE_ST_I64 && cache_i2.neoncache[i].t == NEON_CACHE_ST_D) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.neoncache[i].t, cache.neoncache[i].n));
                    SCVTFDD(i, i);
                    cache.neoncache[i].t = NEON_CACHE_ST_D;
                } else if(cache.neoncache[i].t == NEON_CACHE_XMMR && cache_i2.neoncache[i].t == NEON_CACHE_XMMW) {
                    cache.neoncache[i].t = NEON_CACHE_XMMW;
                } else if(cache.neoncache[i].t == NEON_CACHE_YMMR && cache_i2.neoncache[i].t == NEON_CACHE_YMMW) {
                    cache.neoncache[i].t = NEON_CACHE_YMMW;
                } else if(cache.neoncache[i].t == NEON_CACHE_XMMW && cache_i2.neoncache[i].t == NEON_CACHE_XMMR) {
                    // refresh cache...
                    MESSAGE(LOG_DUMP, "\t  - Refreh %s\n", getCacheName(cache.neoncache[i].t, cache.neoncache[i].n));
                    VSTR128_U12(i, xEmu, offsetof(x64emu_t, xmm[cache.neoncache[i].n]));
                    cache.neoncache[i].t = NEON_CACHE_XMMR;
                } else if(cache.neoncache[i].t == NEON_CACHE_YMMW && cache_i2.neoncache[i].t == NEON_CACHE_YMMR) {
                    // refresh cache...
                    MESSAGE(LOG_DUMP, "\t  - Refreh %s\n", getCacheName(cache.neoncache[i].t, cache.neoncache[i].n));
                    VSTR128_U12(i, xEmu, offsetof(x64emu_t, ymm[cache.neoncache[i].n]));
                    cache.neoncache[i].t = NEON_CACHE_YMMR;
                }
            }
        }
    }
    if(stack_cnt != cache_i2.stack) {
        MESSAGE(LOG_DUMP, "\t    - adjust stack count %d -> %d -\n", stack_cnt, cache_i2.stack);
        int a = stack_cnt - cache_i2.stack;
        // Add x87stack to emu fpu_stack
        LDRw_U12(s3, xEmu, offsetof(x64emu_t, fpu_stack));
        if(a>0) {
            ADDw_U12(s3, s3, a);
        } else {
            SUBw_U12(s3, s3, -a);
        }
        STRw_U12(s3, xEmu, offsetof(x64emu_t, fpu_stack));
        // Sub x87stack to top, with and 7
        LDRw_U12(s3, xEmu, offsetof(x64emu_t, top));
        if(a>0) {
            SUBw_U12(s3, s3, a);
        } else {
            ADDw_U12(s3, s3, -a);
        }
        ANDw_mask(s3, s3, 0, 2);   //mask=7
        STRw_U12(s3, xEmu, offsetof(x64emu_t, top));
        // update tags
        LDRH_U12(s2, xEmu, offsetof(x64emu_t, fpu_tags));
        if(a>0) {
            LSLw_IMM(s2, s2, a*2);
        } else {
            ORRw_mask(s2, s2, 0b010000, 0b001111);  // 0xffff0000
            LSRw_IMM(s2, s2, -a*2);
        }
        STRH_U12(s2, xEmu, offsetof(x64emu_t, fpu_tags));
        s3_top = 0;
        stack_cnt = cache_i2.stack;
    }
    MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
}
static void flagsCacheTransform(dynarec_arm_t* dyn, int ninst)
{
    int j64;
    int jmp = dyn->insts[ninst].x64.jmp_insts;
    if(jmp<0)
        return;
    if(dyn->insts[jmp].f_exit==dyn->insts[jmp].f_entry)  // flags will be fully known, nothing we can do more
        return;
    if(dyn->insts[jmp].df_notneeded)
        return;
    MESSAGE(LOG_DUMP, "\tFlags fetch ---- ninst=%d -> %d\n", ninst, jmp);
    int go_fetch = 0;
    switch(dyn->insts[jmp].f_entry) {
        case status_unk:
            if(dyn->insts[ninst].f_exit==status_none_pending) {
                FORCE_DFNONE();
            }
            break;
        case status_set:
            if(dyn->insts[ninst].f_exit==status_none_pending) {
                FORCE_DFNONE();
            }
            if(dyn->insts[ninst].f_exit==status_unk)
                go_fetch = 1;
            break;
        case status_none_pending:
            if(dyn->insts[ninst].f_exit!=status_none)
                go_fetch = 1;
            break;
        case status_none:
            if(dyn->insts[ninst].f_exit==status_none_pending) {
                FORCE_DFNONE();
            } else
                go_fetch = 1;
            break;
    }
    if(go_fetch) {
        if(dyn->f==status_unk) {
            LDRw_U12(x1, xEmu, offsetof(x64emu_t, df));
            j64 = (GETMARKF2)-(dyn->native_size);
            CBZw(x1, j64);
        }
        if(dyn->insts[ninst].need_nat_flags)
            MRS_nzcv(x6);
        TABLE64C(x1, const_updateflags_arm64);
        BLR(x1);
        if(dyn->insts[ninst].need_nat_flags)
            MSR_nzcv(x6);
        MARKF2;
    }
    MESSAGE(LOG_DUMP, "\t---- Flags fetch\n");
}

static void nativeFlagsTransform(dynarec_arm_t* dyn, int ninst, int s1, int s2)
{
    int j64;
    int jmp = dyn->insts[ninst].x64.jmp_insts;
    if(jmp<0)
        return;
    uint8_t flags_before = dyn->insts[ninst].need_nat_flags;
    uint8_t nc_before = dyn->insts[ninst].normal_carry;
    if(dyn->insts[ninst].invert_carry)
        nc_before = 0;
    uint8_t flags_after = dyn->insts[jmp].need_nat_flags;
    uint8_t nc_after = dyn->insts[jmp].normal_carry;
    if(dyn->insts[jmp].nat_flags_op==NAT_FLAG_OP_TOUCH) {
        flags_after = dyn->insts[jmp].before_nat_flags;
        nc_after = dyn->insts[jmp].normal_carry_before;
    }
    uint8_t flags_x86 = flag2native(dyn->insts[jmp].x64.need_before);
    flags_x86 &= ~flags_after;
    MESSAGE(LOG_DUMP, "\tNative Flags transform ---- ninst=%d -> %d %hhx -> %hhx/%hhx\n", ninst, jmp, flags_before, flags_after, flags_x86);
    // flags present in before and missing in after
    if((flags_before&NF_EQ) && (flags_x86&NF_EQ)) {
        CSETw(s1, cEQ);
        BFIw(xFlags, s1, F_ZF, 1);
    }
    if((flags_before&NF_SF) && (flags_x86&NF_SF)) {
        CSETw(s1, cMI);
        BFIw(xFlags, s1, F_SF, 1);
    }
    if((flags_before&NF_VF) && (flags_x86&NF_VF)) {
        CSETw(s1, cVS);
        BFIw(xFlags, s1, F_OF, 1);
    }
    if((flags_before&NF_CF) && (flags_x86&NF_CF)) {
        if(nc_before) // might need to invert carry
            CSETw(s1, cCS);
        else
            CSETw(s1, cCC);
        BFIw(xFlags, s1, F_CF, 1);
    }
    // flags missing and needed later
    int mrs = 0;
    #define GO_MRS(A)   if(!mrs) {mrs=1; MRS_nzcv(s2); }
    if(!(flags_before&NF_EQ) && (flags_after&NF_EQ)) {
        GO_MRS(s2);
        UBFXw(s1, xFlags, F_ZF, 1);
        BFIx(s2, s1, NZCV_Z, 1);
    }
    if(!(flags_before&NF_SF) && (flags_after&NF_SF)) {
        GO_MRS(s2);
        UBFXw(s1, xFlags, F_SF, 1);
        BFIx(s2, s1, NZCV_N, 1);
    }
    if(!(flags_before&NF_VF) && (flags_after&NF_VF)) {
        GO_MRS(s2);
        UBFXw(s1, xFlags, F_OF, 1);
        BFIx(s2, s1, NZCV_V, 1);
    }
    if(!(flags_before&NF_CF) && (flags_after&NF_CF)) {
        GO_MRS(s2);
        BFIx(s2, xFlags, NZCV_C, 1);    // F_CF is bit 0
        if(!nc_after)
            EORx_mask(s2, s2, 1, 35, 0);    //mask=1<<NZCV_C
    }
    // special case for NF_CF changing state
    if((flags_before&NF_CF) && (flags_after&NF_CF) && (nc_before!=nc_after)) {
        if(cpuext.flagm && !mrs) {
            CFINV();
        } else {
            GO_MRS(s2);
            EORx_mask(s2, s2, 1, 35, 0);  //mask=1<<NZCV_C
        }
    }
    #undef GL_MRS
    if(mrs) MSR_nzcv(s2);

    MESSAGE(LOG_DUMP, "\t---- Native Flags transform\n");
}

// Might use all Scratch registers!
void CacheTransform(dynarec_arm_t* dyn, int ninst, int cacheupd) {
    if(cacheupd&1)
        flagsCacheTransform(dyn, ninst);
    if(cacheupd&2)
        fpuCacheTransform(dyn, ninst, x1, x2, x3);
    if(cacheupd&4)
        nativeFlagsTransform(dyn, ninst, x1, x2);
}

void fpu_reflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    x87_reflectcache(dyn, ninst, s1, s2, s3);
    mmx_reflectcache(dyn, ninst, s1);
    //sse_reflectcache(dyn, ninst, s1); // no need, it's pushed/unpushed during call
    // but ymm0 needs to be pushed
    if(dyn->ymm_zero) {
        ADDx_U12(s1, xEmu, offsetof(x64emu_t, ymm[0]));
        for(int i=0; i<16; ++i)
            if(dyn->ymm_zero&(1<<i))
                STPx_S7_offset(xZR, xZR, s1, 16*i);
    }
}

void fpu_unreflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    // need to undo some things on the x87 tracking
    x87_unreflectcount(dyn, ninst, s1, s2);
}

void emit_pf(dynarec_arm_t* dyn, int ninst, int s1, int s4)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MAYUSE(s1); MAYUSE(s4);
    // by xor'ing all the bit 2 by two with a shift, pair of bits are removed, and only 1 is left if bit number if odd
    EORw_REG_LSR(s4, s1, s1, 4);
    EORw_REG_LSR(s4, s4, s4, 2);
    EORw_REG_LSR(s4, s4, s4, 1);
    MVNw_REG(s4, s4);
    BFIw(xFlags, s4, F_PF, 1);
}

void arm64_move32(dynarec_arm_t* dyn, int ninst, int reg, uint32_t val)
{
    // simple cases with only 1 operations
    for(int i=0; i<2; ++i)
        if((val&(0xFFFF<<(i*16)))==val) {
            MOVZw_LSL(reg, (val>>(i*16))&0xFFFF, i*16);
            return;
        }
    // same but with negation
    for(int i=0; i<2; ++i)
        if(((~val)&(0xFFFF<<(i*16)))==(~val)) {
            MOVNw_LSL(reg, ((~val)>>(i*16))&0xFFFF, i*16);
            return;
        }
    // generic cases
    int mask = convert_bitmask_w(val);
    if(mask) {
        ORRw_mask(reg, xZR, mask&0x3F, (mask>>6)&0x3F);
    } else {
        MOVZw(reg, val&0xFFFF);
        MOVKw_LSL(reg, (val>>16)&0xFFFF, 16);
    }
}
void arm64_move64(dynarec_arm_t* dyn, int ninst, int reg, uint64_t val)
{
    // simple cases with only 1 operations
    for(int i=0; i<4; ++i)
        if((val&(0xFFFFLL<<(i*16)))==val) {
            MOVZx_LSL(reg, (val>>(i*16))&0xFFFF, i*16);
            return;
        }
    // same but with negation
    for(int i=0; i<4; ++i)
        if(((~val)&(0xFFFFLL<<(i*16)))==(~val)) {
            MOVNx_LSL(reg, ((~val)>>(i*16))&0xFFFF, i*16);
            return;
        }
    // mask
    int mask = convert_bitmask_x(val);
    if(mask) {
        ORRx_mask(reg, xZR, (mask>>12)&1, mask&0x3F, (mask>>6)&0x3F);
        return;
    }
    // 32bit value?
    if((val&0xFFFFFFFF)==val) {
        arm64_move32(dyn, ninst, reg, val);
        return;
    }
    int n = 0;
    // negatives values
    if((val&0xFFFF000000000000LL)==0xFFFF000000000000LL) {
        for(int i=0; i<3; ++i) {
            if(((~val)>>(i*16))&0xFFFF) {
                if(n) {
                    MOVKx_LSL(reg, (val>>(i*16))&0xFFFF, i*16);
                } else {
                    MOVNx_LSL(reg, ((~val)>>(i*16))&0xFFFF, i*16);
                    n = 1;
                }
            }
        }
        return;
    }
    // positive values
    for(int i=0; i<4; ++i) {
        if((val>>(i*16))&0xFFFF) {
            if(n) {
                MOVKx_LSL(reg, (val>>(i*16))&0xFFFF, i*16);
            } else {
                MOVZx_LSL(reg, (val>>(i*16))&0xFFFF, i*16);
                n = 1;
            }
        }
    }
}


void fpu_reset_cache(dynarec_arm_t* dyn, int ninst, int reset_n)
{
    MESSAGE(LOG_DEBUG, "Reset Caches with %d\n",reset_n);
    #if STEP > 1
    // for STEP 2 & 3, just need to refrest with current, and undo the changes (push & swap)
    dyn->n = dyn->insts[ninst].n;
    dyn->ymm_zero = dyn->insts[ninst].ymm0_in;
    neoncacheUnwind(&dyn->n);
    #else
    dyn->n = dyn->insts[reset_n].n;
    dyn->ymm_zero = dyn->insts[reset_n].ymm0_out;
    #endif
    #if STEP == 0
    if(dyn->need_dump && dyn->n.x87stack) dynarec_log(LOG_NONE, "New x87stack=%d at ResetCache in inst %d with %d\n", dyn->n.x87stack, ninst, reset_n);
        #endif
    #if defined(HAVE_TRACE) && (STEP>2)
    if(dyn->need_dump && 0) //disable for now, need more work
        if(memcmp(&dyn->n, &dyn->insts[reset_n].n, sizeof(neoncache_t))) {
            MESSAGE(LOG_DEBUG, "Warning, difference in neoncache: reset=");
            for(int i=0; i<32; ++i)
                if(dyn->insts[reset_n].n.neoncache[i].v)
                    MESSAGE(LOG_DEBUG, " %02d:%s", i, getCacheName(dyn->insts[reset_n].n.neoncache[i].t, dyn->insts[reset_n].n.neoncache[i].n));
            if(dyn->insts[reset_n].n.combined1 || dyn->insts[reset_n].n.combined2)
                MESSAGE(LOG_DEBUG, " %s:%02d/%02d", dyn->insts[reset_n].n.swapped?"SWP":"CMB", dyn->insts[reset_n].n.combined1, dyn->insts[reset_n].n.combined2);
            if(dyn->insts[reset_n].n.stack_push || dyn->insts[reset_n].n.stack_pop)
                MESSAGE(LOG_DEBUG, " (%d:%d)", dyn->insts[reset_n].n.stack_push, -dyn->insts[reset_n].n.stack_pop);
            MESSAGE(LOG_DEBUG, " ==> ");
            for(int i=0; i<32; ++i)
                if(dyn->insts[ninst].n.neoncache[i].v)
                    MESSAGE(LOG_DEBUG, " %02d:%s", i, getCacheName(dyn->insts[ninst].n.neoncache[i].t, dyn->insts[ninst].n.neoncache[i].n));
            if(dyn->insts[ninst].n.combined1 || dyn->insts[ninst].n.combined2)
                MESSAGE(LOG_DEBUG, " %s:%02d/%02d", dyn->insts[ninst].n.swapped?"SWP":"CMB", dyn->insts[ninst].n.combined1, dyn->insts[ninst].n.combined2);
            if(dyn->insts[ninst].n.stack_push || dyn->insts[ninst].n.stack_pop)
                MESSAGE(LOG_DEBUG, " (%d:%d)", dyn->insts[ninst].n.stack_push, -dyn->insts[ninst].n.stack_pop);
            MESSAGE(LOG_DEBUG, " -> ");
            for(int i=0; i<32; ++i)
                if(dyn->n.neoncache[i].v)
                    MESSAGE(LOG_DEBUG, " %02d:%s", i, getCacheName(dyn->n.neoncache[i].t, dyn->n.neoncache[i].n));
            if(dyn->n.combined1 || dyn->n.combined2)
                MESSAGE(LOG_DEBUG, " %s:%02d/%02d", dyn->n.swapped?"SWP":"CMB", dyn->n.combined1, dyn->n.combined2);
            if(dyn->n.stack_push || dyn->n.stack_pop)
                MESSAGE(LOG_DEBUG, " (%d:%d)", dyn->n.stack_push, -dyn->n.stack_pop);
            MESSAGE(LOG_DEBUG, "\n");
        }
    #endif //HAVE_TRACE
}

// propagate ST stack state, especial stack pop that are deferred
void fpu_propagate_stack(dynarec_arm_t* dyn, int ninst)
{
    if(dyn->n.stack_pop) {
        for(int j=0; j<24; ++j)
            if((dyn->n.neoncache[j].t == NEON_CACHE_ST_D
             || dyn->n.neoncache[j].t == NEON_CACHE_ST_F
             || dyn->n.neoncache[j].t == NEON_CACHE_ST_I64)) {
                if(dyn->n.neoncache[j].n<dyn->n.stack_pop)
                    dyn->n.neoncache[j].v = 0;
                else
                    dyn->n.neoncache[j].n-=dyn->n.stack_pop;
            }
        dyn->n.stack_pop = 0;
    }
    dyn->n.stack = dyn->n.stack_next;
    dyn->n.news = 0;
    dyn->n.stack_push = 0;
    dyn->n.swapped = 0;
}

void avx_purge_ymm(dynarec_arm_t* dyn, int ninst, uint16_t mask, int s1)
{
    int s1_set = 0;
    int do_something = 0;
    for(int i=0; i<16; ++i)
        if(mask&(1<<i)) {
            if(is_avx_zero_unset(dyn, ninst, i)) {
                if(!do_something) {
                    MESSAGE(LOG_NONE, "Purge YMM mask=%04x --------\n", mask);
                    do_something = 1;
                }
                if(!s1_set) {
                    ADDx_U12(s1, xEmu, offsetof(x64emu_t, ymm[0]));
                    s1_set = 1;
                }
                STPx_S7_offset(xZR, xZR, s1, i*16);
            }
            for(int j=0; j<32; ++j)
                if(dyn->n.neoncache[j].t==NEON_CACHE_YMMR && dyn->n.neoncache[j].n==i) {
                    // just forget the reg....
                    dyn->n.neoncache[j].v = 0;
                    j=32;
                } else if(dyn->n.neoncache[j].t==NEON_CACHE_YMMW && dyn->n.neoncache[j].n==i) {
                    if(!do_something) {
                        MESSAGE(LOG_NONE, "Purge YMM mask=%04x --------\n", mask);
                        do_something = 1;
                    }
                    VSTR128_U12(j, xEmu, offsetof(x64emu_t, ymm[i]));
                    dyn->n.neoncache[j].v = 0;
                    j=32;
                }
        }
    if(do_something)
        MESSAGE(LOG_NONE, "---------- Purge YMM\n");
}

// Get an YMM quad reg, while preserving up to 3 other YMM regs
int fpu_get_reg_ymm(dynarec_arm_t* dyn, int ninst, int t, int ymm, int k1, int k2, int k3)
{
    int i = -1;
    dyn->n.ymm_used|=(1<<ymm);
    #if STEP > 1
    // check the cached neoncache, it should be exact
    // look for it
    for(int ii=0; ii<32 && i==-1; ++ii)
        if(dyn->insts[ninst].n.neoncache[ii].n==ymm && (dyn->insts[ninst].n.neoncache[ii].t==NEON_CACHE_YMMR || dyn->insts[ninst].n.neoncache[ii].t==NEON_CACHE_YMMW))
            i = ii;
    if(i!=-1) {
        // already there!
        if((dyn->n.neoncache[i].t==NEON_CACHE_YMMW  || dyn->n.neoncache[i].t==NEON_CACHE_YMMR) && dyn->n.neoncache[i].n==ymm) {
            if(t==NEON_CACHE_YMMW)
                dyn->n.neoncache[i].t=t;
            return i;
        }
        // check if free or should be purge before...
        if(dyn->n.neoncache[i].t==NEON_CACHE_YMMW)
            VSTR128_U12(i, xEmu, offsetof(x64emu_t, ymm[dyn->n.neoncache[i].n]));
        dyn->n.neoncache[i].t=t;
        dyn->n.neoncache[i].n=ymm;
        return i;
    }
    printf_log(LOG_NONE, "BOX64 Dynarec: Warning, unable to find YMM %d in neoncache at inst=%d\n", ymm, ninst);
    #else
    i = EMM0;
    // first pass see if a slot is free in EMM/x87 slots
    for(int j=0; j<8; ++j) {
        if(!dyn->n.fpuused[i+j]) {
            int ret = internal_mark_ymm(dyn, t, ymm, i+j);
            if(ret>=0) return ret;
        }
    }
    // no slot in the emm space, look for scratch space in reverse
    i = SCRATCH0;
    for(int j=7; j>=dyn->n.fpu_scratch; --j) {
        int ret = internal_mark_ymm(dyn, t, ymm, i+j);
        if(ret>=0) return ret;
    }
    // no free slot, needs to purge a value... First loop on the YMMR, they are easier to purge
    i = EMM0;
    for(int j=0; j<8; ++j) {
        if(!dyn->n.fpuused[i+j]) {
            // should a test be done to check if ymm is already in the purge list?
            if(!is_ymm_to_keep(dyn, i+j, k1, k2, k3) && (dyn->n.neoncache[i+j].t==NEON_CACHE_YMMR)) {
                dyn->n.neoncache[i+j].v = 0;
                int ret = internal_mark_ymm(dyn, t, ymm, i+j);
                if(ret>=0) return ret;
            }
        }
    }
    i = SCRATCH0;
    for(int j=dyn->n.fpu_scratch; j<8; ++j) {
        if(!is_ymm_to_keep(dyn, i+j, k1, k2, k3) && (dyn->n.neoncache[i+j].t==NEON_CACHE_YMMR)) {
            dyn->n.neoncache[i+j].v = 0;
            int ret = internal_mark_ymm(dyn, t, ymm, i+j);
            if(ret>=0) return ret;
        }
    }
    // make space in the scratch area
    for(int j=dyn->n.fpu_scratch; j<8; ++j) {
            // should a test be done to check if ymm is already in the purge list?
            if(!is_ymm_to_keep(dyn, i+j, k1, k2, k3)) {
                // Save the reg and recycle it
                VSTR128_U12(i+j, xEmu, offsetof(x64emu_t, ymm[dyn->n.neoncache[i+j].n]));
                dyn->n.neoncache[i+j].v = 0;
                int ret = internal_mark_ymm(dyn, t, ymm, i+j);
                if(ret>=0) return ret;
            }
    }
    // last resort, go back in the EMM area...
    i = EMM0;
    for(int j=7; j>=0; --j) {
        if(!dyn->n.fpuused[i+j]) {
            // should a test be done to check if ymm is already in the purge list?
            if((dyn->n.neoncache[i+j].t==NEON_CACHE_YMMW) && !is_ymm_to_keep(dyn, i+j, k1, k2, k3)) {
                VSTR128_U12(i+j, xEmu, offsetof(x64emu_t, ymm[dyn->n.neoncache[i+j].n]));
                dyn->n.neoncache[i+j].v = 0;
                int ret = internal_mark_ymm(dyn, t, ymm, i+j);
                if(ret>=0) return ret;
            }
        }
    }
    #endif
    printf_log(LOG_NONE, "BOX64 Dynarec: Error, unable to free a reg for YMM %d at inst=%d on pass %d\n", ymm, ninst, STEP);
    return i;
}

// Get an XMM quad reg and preload it (or do nothing if not possible)
static int xmm_preload_reg(dynarec_arm_t* dyn, int ninst, int last, int xmm)
{
    dyn->n.ssecache[xmm].reg = fpu_get_reg_xmm(dyn, NEON_CACHE_XMMR, xmm);
    int ret =  dyn->n.ssecache[xmm].reg;
    dyn->n.ssecache[xmm].write = 0;
    VLDR128_U12(ret, xEmu, offsetof(x64emu_t, xmm[xmm]));
    return ret;
}

// Get an YMM quad reg and preload it (or do nothing if not possible)
static int ymm_preload_reg(dynarec_arm_t* dyn, int ninst, int last, int ymm)
{
    int i = -1;
    // search for when it will be loaded the first time
    for(int ii=0; ii<32 && i==-1; ++ii)
        if(dyn->insts[last].n.neoncache[ii].n==ymm && (dyn->insts[last].n.neoncache[ii].t==NEON_CACHE_YMMR || dyn->insts[last].n.neoncache[ii].t==NEON_CACHE_YMMW))
            i = ii;
    if(i!=-1) {
        VLDR128_U12(i, xEmu, offsetof(x64emu_t, ymm[ymm]));
        dyn->n.neoncache[i].t = NEON_CACHE_YMMR;
        dyn->n.neoncache[i].n = ymm;
    }
    return i;
}

void doPreload(dynarec_arm_t* dyn, int ninst)
{
    int n = ninst?(ninst-1):ninst;
    uint32_t preload = dyn->insts[ninst].preload_xmmymm;
    int from = dyn->insts[ninst].preload_from;
    if(!preload) return;
    // preload XMM
    MESSAGE(LOG_INFO, "Preload XMM/YMM -------- %x\n", preload);
    for(int i=0; i<16; ++i) {
        if(preload&(1<<i)) {
            xmm_preload_reg(dyn, n, from, i);
        }
        if(preload&(1<<(16+i))) {
            ymm_preload_reg(dyn, n, from, i);
        }
    }
    MESSAGE(LOG_INFO, "-------- Preload XMM/YMM\n");
}

void doEnterBlock(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    // get dynarec address. It is stored just before the start of the block
    MESSAGE(LOG_INFO, "doEnter --------\n");
    int delta = -(dyn->native_size + sizeof(void*));
    LDRx_literal(s1, delta);
    // now increment in_used
    ADDx_U12(s1, s1, offsetof(dynablock_t, in_used));
    if(cpuext.atomics) {
        MOV32w(s3, 1);
        STADDLw(s3, s1);
    } else {
        LDAXRw(s2, s1);
        ADDw_U12(s2, s2, 1);
        STLXRw(s3, s2, s1);
        CBNZw(s3, -3*4);
    }
    // set tick
    LDRx_U12(s2, xEmu, offsetof(x64emu_t, context));
    LDRw_U12(s2, s2, offsetof(box64context_t, tick));
    STRw_U12(s2, s1, offsetof(dynablock_t, tick)-offsetof(dynablock_t, in_used));
    MESSAGE(LOG_INFO, "-------- doEnter\n");
}
void doLeaveBlock(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    MESSAGE(LOG_INFO, "doLeave --------\n");
    // get dynarec address
    int delta = -(dyn->native_size + sizeof(void*));
    LDRx_literal(s1, delta);
    ADDx_U12(s1, s1, offsetof(dynablock_t, in_used));
    // decrement in_used
    if(cpuext.atomics) {
        MOV32w(s3, -1);
        STADDLw(s3, s1);
    } else {
        LDAXRw(s2, s1);
        SUBw_U12(s2, s2, 1);
        STLXRw(s3, s2, s1);
        CBNZw(s3, -3*4);
    }
    MESSAGE(LOG_INFO, "-------- doLeave\n");
}
