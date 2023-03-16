#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>

#include "bitutils.h"
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
#include "../dynablock_private.h"
#include "../tools/bridge_private.h"
#include "custommem.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "dynarec_rv64_helper.h"

/* setup r2 to address pointed by ED, also fixaddress is an optionnal delta in the range [-absmax, +absmax], with delta&mask==0 to be added to ed for LDR/STR */
uintptr_t geted(dynarec_rv64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, rex_t rex, int *l, int i12, int delta)
{
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(delta);

    int lock = l?((l==LOCK_LOCK)?1:2):0;
    if(lock==2)
        *l = 0;
    uint8_t ret = x2;
    *fixaddress = 0;
    if(hint>0) ret = hint;
    MAYUSE(scratch);
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            int sib_reg = ((sib>>3)&7)+(rex.x<<3);
            if((sib&0x7)==5) {
                int64_t tmp = F32S;
                if (sib_reg!=4) {
                    if(tmp && ((tmp<-2048) || (tmp>2047) || !i12)) {
                        MOV64x(scratch, tmp);
                        if((sib>>6)) {
                            SLLI(ret, xRAX+sib_reg, (sib>>6));
                            ADD(ret, ret, scratch);
                        } else {
                            ADD(ret, xRAX+sib_reg, scratch);
                        }
                    } else {
                        if(sib>>6) {
                            SLLI(ret, xRAX+sib_reg, (sib>>6));
                        } else
                            ret = xRAX+sib_reg;
                        *fixaddress = tmp;
                    }
                } else {
                    switch(lock) {
                        case 1: addLockAddress(tmp); break;
                        case 2: if(isLockAddress(tmp)) *l=1; break;
                    }
                    MOV64x(ret, tmp);
                }
            } else {
                if (sib_reg!=4) {
                    if(sib>>6) {
                        SLLI(scratch, xRAX+sib_reg, (sib>>6));
                        ADD(ret, xRAX+(sib&0x7)+(rex.b<<3), scratch);
                    } else {
                        ADD(ret, xRAX+(sib&0x7)+(rex.b<<3), xRAX+sib_reg);
                    }
                } else {
                    ret = xRAX+(sib&0x7)+(rex.b<<3);
                }
            }
        } else if((nextop&7)==5) {
            int64_t tmp = F32S64;
            if(i12 && (tmp>=-2048) && (tmp<=2047)) {
                GETIP(addr+delta);
                ret = xRIP;
                *fixaddress = tmp;
            } else if((tmp>=-2048) && (tmp<=2047)) {
                GETIP(addr+delta);
                ADDI(ret, xRIP, tmp);
            } else if(tmp+addr+delta<0x100000000LL) {
                MOV64x(ret, tmp+addr+delta);
            } else {
                MOV64x(ret, tmp);
                GETIP(addr+delta);
                ADD(ret, ret, xRIP);
            }
            switch(lock) {
                case 1: addLockAddress(addr+delta+tmp); break;
                case 2: if(isLockAddress(addr+delta+tmp)) *l=1; break;
            }
        } else {
            ret = xRAX+(nextop&7)+(rex.b<<3);
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
        if(i64==0 || ((i64>=-2048) && (i64<=2047)  && i12)) {
            *fixaddress = i64;
            if((nextop&7)==4) {
                if (sib_reg!=4) {
                    if(sib>>6) {
                        SLLI(scratch, xRAX+sib_reg, (sib>>6));
                        ADD(ret, xRAX+(sib&0x07)+(rex.b<<3), scratch);
                    } else {
                        ADD(ret, xRAX+(sib&0x07)+(rex.b<<3), xRAX+sib_reg);
                    }
                } else {
                    ret = xRAX+(sib&0x07)+(rex.b<<3);
                }
            } else
                ret = xRAX+(nextop&0x07)+(rex.b<<3);
        } else {
            if(i64>=-2048 && i64<=2047) {
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        if(sib>>6) {
                            SLLI(scratch, xRAX+sib_reg, (sib>>6));
                            ADD(scratch, xRAX+(sib&0x07)+(rex.b<<3), scratch);
                        } else {
                            ADD(scratch, xRAX+(sib&0x07)+(rex.b<<3), xRAX+sib_reg);
                        }
                    } else {
                        scratch = xRAX+(sib&0x07)+(rex.b<<3);
                    }
                } else
                    scratch = xRAX+(nextop&0x07)+(rex.b<<3);
                ADDI(ret, scratch, i64);
            } else {
                MOV64x(scratch, i64);
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        ADD(scratch, scratch, xRAX+(sib&0x07)+(rex.b<<3));
                        if(sib>>6) {
                            SLLI(ret, xRAX+sib_reg, (sib>>6));
                            ADD(ret, scratch, ret);
                        } else {
                            ADD(ret, scratch, xRAX+sib_reg);
                        }
                    } else {
                        PASS3(int tmp = xRAX+(sib&0x07)+(rex.b<<3));
                        ADD(ret, tmp, scratch);
                    }
                } else {
                    PASS3(int tmp = xRAX+(nextop&0x07)+(rex.b<<3));
                    ADD(ret, tmp, scratch);
                }
            }
        }
    }
    *ed = ret;
    return addr;
}

void jump_to_epilog(dynarec_rv64_t* dyn, uintptr_t ip, int reg, int ninst)
{
    MAYUSE(dyn); MAYUSE(ip); MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Jump to epilog\n");

    if(reg) {
        if(reg!=xRIP) {
            MV(xRIP, reg);
        }
    } else {
        GETIP_(ip);
    }
    TABLE64(x2, (uintptr_t)rv64_epilog);
    SMEND();
    BR(x2);
}

void jump_to_next(dynarec_rv64_t* dyn, uintptr_t ip, int reg, int ninst)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Jump to next\n");

    if(reg) {
        if(reg!=xRIP) {
            MV(xRIP, reg);
        }
        uintptr_t tbl = getJumpTable64();
        MAYUSE(tbl);
        TABLE64(x3, tbl);
        SRLI(x2, xRIP, JMPTABL_START3);
        SLLI(x2, x2, 3);
        ADD(x3, x3, x2);
        LD(x3, x3, 0);
        MOV64x(x4, JMPTABLE_MASK2<<3);    // x4 = mask
        SRLI(x2, xRIP, JMPTABL_START2-3);
        AND(x2, x2, x4);
        ADD(x3, x3, x2);
        LD(x3, x3, 0);
        if(JMPTABLE_MASK2!=JMPTABLE_MASK1) {
            MOV64x(x4, JMPTABLE_MASK1<<3);    // x4 = mask
        }
        SRLI(x2, xRIP, JMPTABL_START1-3);
        AND(x2, x2, x4);
        ADD(x3, x3, x2);
        LD(x3, x3, 0);
        if(JMPTABLE_MASK0<2048) {
            ANDI(x2, xRIP, JMPTABLE_MASK0);
        } else {
            if(JMPTABLE_MASK1!=JMPTABLE_MASK0) {
                MOV64x(x4, JMPTABLE_MASK0);    // x4 = mask
            }
            AND(x2, xRIP, x4);
        }
        SLLI(x2, x2, 3);
        ADD(x3, x3, x2);
        LD(x2, x3, 0);
    } else {
        uintptr_t p = getJumpTableAddress64(ip);
        MAYUSE(p);
        TABLE64(x3, p);
        GETIP_(ip);
        LD(x2, x3, 0);
    }
    if(reg!=A1) {
        MV(A1, xRIP);
    }
    CLEARIP();
    #ifdef HAVE_TRACE
    //MOVx(x3, 15);    no access to PC reg
    #endif
    SMEND();
    JALR(x2); // save LR...
}

void ret_to_epilog(dynarec_rv64_t* dyn, int ninst)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Ret to epilog\n");
    POP1(xRIP);
    MV(x1, xRIP);
    SMEND();
    /*if(box64_dynarec_callret) {
        // pop the actual return address from RV64 stack
        LDPx_S7_offset(x2, x6, xSP, 0);
        CBZx(x6, 5*4);
        ADDx_U12(xSP, xSP, 16);
        SUBx_REG(x6, x6, xRIP); // is it the right address?
        CBNZx(x6, 2*4);
        BLR(x2);
        // not the correct return address, regular jump
    }*/
    uintptr_t tbl = getJumpTable64();
    MOV64x(x3, tbl);
    SRLI(x2, xRIP, JMPTABL_START3);
    SLLI(x2, x2, 3);
    ADD(x3, x3, x2);
    LD(x3, x3, 0);
    MOV64x(x4, JMPTABLE_MASK2<<3);    // x4 = mask
    SRLI(x2, xRIP, JMPTABL_START2-3);
    AND(x2, x2, x4);
    ADD(x3, x3, x2);
    LD(x3, x3, 0);
    if(JMPTABLE_MASK2!=JMPTABLE_MASK1) {
        MOV64x(x4, JMPTABLE_MASK1<<3);    // x4 = mask
    }
    SRLI(x2, xRIP, JMPTABL_START1-3);
    AND(x2, x2, x4);
    ADD(x3, x3, x2);
    LD(x3, x3, 0);
    if(JMPTABLE_MASK0<2048) {
        ANDI(x2, xRIP, JMPTABLE_MASK0);
    } else {
        if(JMPTABLE_MASK1!=JMPTABLE_MASK0) {
            MOV64x(x4, JMPTABLE_MASK0);    // x4 = mask
        }
        AND(x2, xRIP, x4);
    }
    SLLI(x2, x2, 3);
    ADD(x3, x3, x2);
    LD(x2, x3, 0);
    JALR(x2); // save LR
    CLEARIP();
}

void retn_to_epilog(dynarec_rv64_t* dyn, int ninst, int n)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Retn to epilog\n");
    POP1(xRIP);
    if(n>0x7ff) {
        MOV64x(w1, n);
        ADD(xRSP, xRSP, x1);
    } else {
        ADDI(xRSP, xRSP, n);
    }
    MV(x1, xRIP);
    SMEND();
    /*if(box64_dynarec_callret) {
        // pop the actual return address from RV64 stack
        LDPx_S7_offset(x2, x6, xSP, 0);
        CBZx(x6, 5*4);
        ADDx_U12(xSP, xSP, 16);
        SUBx_REG(x6, x6, xRIP); // is it the right address?
        CBNZx(x6, 2*4);
        BLR(x2);
        // not the correct return address, regular jump
    }*/
    uintptr_t tbl = getJumpTable64();
    MOV64x(x3, tbl);
    SRLI(x2, xRIP, JMPTABL_START3);
    SLLI(x2, x2, 3);
    ADD(x3, x3, x2);
    LD(x3, x3, 0);
    MOV64x(x4, JMPTABLE_MASK2<<3);    // x4 = mask
    SRLI(x2, xRIP, JMPTABL_START2-3);
    AND(x2, x2, x4);
    ADD(x3, x3, x2);
    LD(x3, x3, 0);
    if(JMPTABLE_MASK2!=JMPTABLE_MASK1) {
        MOV64x(x4, JMPTABLE_MASK1<<3);    // x4 = mask
    }
    SRLI(x2, xRIP, JMPTABL_START1-3);
    AND(x2, x2, x4);
    ADD(x3, x3, x2);
    LD(x3, x3, 0);
    if(JMPTABLE_MASK0<2048) {
        ANDI(x2, xRIP, JMPTABLE_MASK0);
    } else {
        if(JMPTABLE_MASK1!=JMPTABLE_MASK0) {
            MOV64x(x4, JMPTABLE_MASK0);    // x4 = mask
        }
        AND(x2, xRIP, x4);
    }
    SLLI(x2, x2, 3);
    ADD(x3, x3, x2);
    LD(x2, x3, 0);
    JALR(x2); // save LR
    CLEARIP();
}

void call_c(dynarec_rv64_t* dyn, int ninst, void* fnc, int reg, int ret, int saveflags, int savereg)
{
    MAYUSE(fnc);
    if(savereg==0)
        savereg = x6;
    if(saveflags) {
        FLAGS_ADJUST_TO11(xFlags, reg);
        SD(xFlags, xEmu, offsetof(x64emu_t, eflags));
    }
    fpu_pushcache(dyn, ninst, reg, 0);
    if(ret!=-2) {
        ADDI(xSP, xSP, -16);   // RV64 stack needs to be 16byte aligned
        SD(xEmu, xSP, 0);
        SD(savereg, xSP, 8);
        // x5..x8, x10..x17, x28..x31 those needs to be saved by caller
        STORE_REG(RAX);
        STORE_REG(RCX);
        STORE_REG(R12);
        STORE_REG(R13);
        STORE_REG(R14);
        STORE_REG(R15);
        SD(xRIP, xEmu, offsetof(x64emu_t, ip));
    }
    TABLE64(reg, (uintptr_t)fnc);
    JALR(reg);
    if(ret>=0) {
        MV(ret, xEmu);
    }
    if(ret!=-2) {
        LD(xEmu, xSP, 0);
        LD(savereg, xSP, 8);
        ADDI(xSP, xSP, 16);
        #define GO(A)   if(ret!=x##A) {LOAD_REG(A);}
        GO(RAX);
        GO(RCX);
        GO(R12);
        GO(R13);
        GO(R14);
        GO(R15);
        if(ret!=xRIP)
            LD(xRIP, xEmu, offsetof(x64emu_t, ip));
        #undef GO
    }
    // regenerate mask
    XORI(xMASK, xZR, -1);
    SRLI(xMASK, xMASK, 32);

    fpu_popcache(dyn, ninst, reg, 0);
    if(saveflags) {
        LD(xFlags, xEmu, offsetof(x64emu_t, eflags));
        FLAGS_ADJUST_FROM11(xFlags, reg);
    }
    SET_NODF();
    dyn->last_ip = 0;
}

void call_n(dynarec_rv64_t* dyn, int ninst, void* fnc, int w)
{
    MAYUSE(fnc);
    FLAGS_ADJUST_TO11(xFlags, x3);
    SD(xFlags, xEmu, offsetof(x64emu_t, eflags));
    fpu_pushcache(dyn, ninst, x3, 1);
    // x5..x8, x10..x17, x28..x31 those needs to be saved by caller
    // RDI, RSI, RDX, RCX, R8, R9 are used for function call
    ADDI(xSP, xSP, -16);
    SD(xEmu, xSP, 0);
    SD(xRIP, xSP, 8);   // ARM64 stack needs to be 16byte aligned
    STORE_REG(R12);
    STORE_REG(R13);
    STORE_REG(R14);
    STORE_REG(R15);
    // float and double args
    if(abs(w)>1) {
        /*MESSAGE(LOG_DUMP, "Getting %d XMM args\n", abs(w)-1);
        for(int i=0; i<abs(w)-1; ++i) {
            sse_get_reg(dyn, ninst, x6, i, w);
        }*/
        MESSAGE(LOG_DUMP, "Warning XMM args not ready\n");
    }
    if(w<0) {
        /*
        MESSAGE(LOG_DUMP, "Return in XMM0\n");
        sse_get_reg_empty(dyn, ninst, x6, 0);
        */
        MESSAGE(LOG_DUMP, "Warning return in XMM args not ready\n");
    }
    // prepare regs for native call
    MV(A0, xRDI);
    MV(A1, xRSI);
    MV(A2, xRDX);
    MV(A3, xRCX);
    MV(A4, xR8);
    MV(A5, xR9);
    // native call
    TABLE64(16, (uintptr_t)fnc);    // using x16 as scratch regs for call address
    JALR(16);
    // put return value in x64 regs
    if(w>0) {
        MV(xRAX, A0);
        MV(xRDX, A1);
    }
    // all done, restore all regs
    LD(xEmu, xSP, 0);
    LD(xRIP, xSP, 8);
    ADDI(xSP, xSP, 16);
    LOAD_REG(R12);
    LOAD_REG(R13);
    LOAD_REG(R14);
    LOAD_REG(R15);
    // regenerate mask
    XORI(xMASK, xZR, -1);
    SRLI(xMASK, xMASK, 32);

    fpu_popcache(dyn, ninst, x3, 1);
    LD(xFlags, xEmu, offsetof(x64emu_t, eflags));
    FLAGS_ADJUST_FROM11(xFlags, x3);
    SET_NODF();
}

void fpu_reset(dynarec_rv64_t* dyn)
{
    //TODO
}

void fpu_reset_cache(dynarec_rv64_t* dyn, int ninst, int reset_n)
{
    //TODO
}

void fpu_purgecache(dynarec_rv64_t* dyn, int ninst, int next, int s1, int s2, int s3)
{
    //TODO
}

// propagate ST stack state, especial stack pop that are defered
void fpu_propagate_stack(dynarec_rv64_t* dyn, int ninst)
{
    //TODO
}

void mmx_purgecache(dynarec_rv64_t* dyn, int ninst, int next, int s1)
{
    // TODO
}

void x87_purgecache(dynarec_rv64_t* dyn, int ninst, int next, int s1, int s2, int s3)
{
    //TODO
}

#ifdef HAVE_TRACE
void fpu_reflectcache(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3)
{
    //TODO
}
#endif
void fpu_pushcache(dynarec_rv64_t* dyn, int ninst, int s1, int not07)
{
    //TODO
}
void fpu_popcache(dynarec_rv64_t* dyn, int ninst, int s1, int not07)
{
    //TODO
}

void rv64_move32(dynarec_rv64_t* dyn, int ninst, int reg, int32_t val)
{
    // Depending on val, the following insns are emitted.
    // val == 0               -> ADDI
    // lo12 != 0 && hi20 == 0 -> ADDI
    // lo12 == 0 && hi20 != 0 -> LUI
    // else                   -> LUI+ADDI
    int32_t hi20 = (val+0x800)>>12 & 0xfffff;
    int32_t lo12 = val&0xfff;

    int src = xZR;
    if (hi20) {
        LUI(reg, hi20);
        src = reg;
    }
    if (lo12 || !hi20) ADDI(reg, src, lo12);
}

void rv64_move64(dynarec_rv64_t* dyn, int ninst, int reg, int64_t val)
{
    if(((val<<32)>>32)==val) {
        // 32bits value
        rv64_move32(dyn, ninst, reg, val);
        return;
    }

    int64_t lo12 = (val<<52)>>52;
    int64_t hi52 = (val+0x800)>>12;
    int shift = 12+TrailingZeros64((uint64_t)hi52);
    hi52 = ((hi52>>(shift-12))<<shift)>>shift;
    rv64_move64(dyn, ninst, reg, hi52);
    SLLI(reg, reg, shift);

    if (lo12) {
        ADDI(reg, reg, lo12);
    }
}

void emit_pf(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4)
{
    MAYUSE(dyn); MAYUSE(ninst);
    // PF: (((emu->x64emu_parity_tab[(res&0xff) / 32] >> ((res&0xff) % 32)) & 1) == 0)
    MOV64x(s4, (uintptr_t)GetParityTab());
    SRLI(s3, s1, 3);
    ANDI(s3, s3, 28);
    ADD(s4, s4, s3);
    LW(s4, s4, 0);
    NOT(s4, s4);
    SRLW(s4, s4, s1);
    ANDI(s4, s4, 1);

    BEQZ(s4, 4);
    ORI(xFlags, xFlags, 1 << F_PF);
}
