#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>

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
                        SLLI(ret, xRAX+sib_reg, (sib>>6));
                        ADD(ret, ret, scratch);
                    } else {
                        SLLI(ret, xRAX+sib_reg, (sib>>6));
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
                    SLLI(scratch, xRAX+sib_reg, (sib>>6));
                    ADD(ret, xRAX+(sib&0x7)+(rex.b<<3), scratch);
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
                    SLLI(scratch, xRAX+sib_reg, (sib>>6));
                    ADD(ret, xRAX+(sib&0x07)+(rex.b<<3), scratch);
                } else {
                    ret = xRAX+(sib&0x07)+(rex.b<<3);
                }
            } else
                ret = xRAX+(nextop&0x07)+(rex.b<<3);
        } else {
            if(i64>=-2048 && i64<=2047) {
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        SLLI(scratch, xRAX+sib_reg, (sib>>6));
                        ADD(scratch, xRAX+(sib&0x07)+(rex.b<<3), scratch);
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
                        SLLI(ret, xRAX+sib_reg, (sib>>6));
                        ADD(ret, scratch, ret);
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
        LD(x3, x3, x2);
        LUI(x4, JMPTABLE_MASK2);    // x4 = mask
        SRLI(x2, xRIP, JMPTABL_START2);
        AND(x2, x2, x4);
        SLLI(x2, x2, 3);
        LD(x3, x3, x2);
        if(JMPTABLE_MASK2!=JMPTABLE_MASK1) {
            LUI(x4, JMPTABLE_MASK1);    // x4 = mask
        }
        SRLI(x2, xRIP, JMPTABL_START1);
        AND(x2, x2, x4);
        SLLI(x2, x2, 3);
        LD(x3, x3, x2);
        if(JMPTABLE_MASK1!=JMPTABLE_MASK0) {
            LUI(x4, JMPTABLE_MASK0);    // x4 = mask
        }
        AND(x2, x2, x4);
        SLLI(x2, x2, 3);
        LD(x2, x3, x2);
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
    int32_t up=(val>>12);
    int32_t r = val-(up<<12);
    // check if there is the dreaded sign bit on imm12
    if(r&0b100000000000 && r!=0xffffffff) {
        ++up;
        r = val-(up<<12);
    }
    LUI(reg, up);
    if(r) {
        ADDI(reg, reg, r);
    }
}

void rv64_move64(dynarec_rv64_t* dyn, int ninst, int reg, int64_t val)
{
    if(((val<<(64-12))>>(64-12))==val) {
        // simple 12bit value
        MOV_U12(reg, (val&0b111111111111));
        return;
    }
    if(((val<<32)>>32)==val) {
        // 32bits value
        rv64_move32(dyn, ninst, reg, val);
        return;
    }
    if((val&0xffffffffLL)==val && (val&0x80000000)) {
        // 32bits value, but with a sign bit
        rv64_move32(dyn, ninst, reg, val);
        ZEROUP(reg);
        return;
    }
    //TODO: optimize that later
    // Start with the upper 32bits
    rv64_move32(dyn, ninst, reg, val>>32);
    // now the lower part
    uint32_t r = val&0xffffffff;
    int s = 11;
    if((r>>21)&0b11111111111) {
        SLLI(reg, reg, s);
        ORI(reg, reg, (r>>21)&0b11111111111);
        s = 0;
    }
    s+=11;
    if((r>>10)&0b11111111111) {
        SLLI(reg, reg, s);
        ORI(reg, reg, (r>>10)&0b11111111111);
        s = 0;
    }
    s+=10;
    if(r&0b1111111111) {
        SLLI(reg, reg, s);
        ORI(reg, reg, r&0b1111111111);
        s=0;
    }
    if(s) {
        SLLI(reg, reg, s);
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
