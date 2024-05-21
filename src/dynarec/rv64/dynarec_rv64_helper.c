#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

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
#include "custommem.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "dynarec_rv64_helper.h"

static uintptr_t geted_32(dynarec_rv64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, int *l, int i12);

/* setup r2 to address pointed by ED, also fixaddress is an optionnal delta in the range [-absmax, +absmax], with delta&mask==0 to be added to ed for LDR/STR */
uintptr_t geted(dynarec_rv64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, rex_t rex, int *l, int i12, int delta)
{
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(delta);

    if(rex.is32bits)
        return geted_32(dyn, addr, ninst, nextop, ed, hint, scratch, fixaddress, l, i12);

    int lock = l?((l==LOCK_LOCK)?1:2):0;
    if(lock==2)
        *l = 0;
    uint8_t ret = x2;
    *fixaddress = 0;
    if(hint>0) ret = hint;
    int maxval = 2047;
    if(i12>1)
        maxval -= i12;
    MAYUSE(scratch);
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            int sib_reg = ((sib>>3)&7)+(rex.x<<3);
            int sib_reg2 = (sib&0x7)+(rex.b<<3);
            if((sib&0x7)==5) {
                int64_t tmp = F32S;
                if (sib_reg!=4) {
                    if(tmp && ((tmp<-2048) || (tmp>maxval) || !i12)) {
                        MOV64x(scratch, tmp);
                        ADDSL(ret, scratch, xRAX+sib_reg, sib>>6, ret);
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
                    ADDSL(ret, xRAX+sib_reg2, xRAX+sib_reg, sib>>6, scratch);
                } else {
                    ret = xRAX+sib_reg2;
                }
            }
        } else if((nextop&7)==5) {
            int64_t tmp = F32S64;
            int64_t adj = dyn->last_ip?((addr+delta)-dyn->last_ip):0;
            if(i12 && adj && (tmp+adj>=-2048) && (tmp+adj<=maxval)) {
                ret = xRIP;
                *fixaddress = tmp+adj;
            } else if(i12 && (tmp>=-2048) && (tmp<=maxval)) {
                GETIP(addr+delta);
                ret = xRIP;
                *fixaddress = tmp;
            } else if(adj && (tmp+adj>=-2048) && (tmp+adj<=maxval)) {
                ADDI(ret, xRIP, tmp+adj);
            } else if((tmp>=-2048) && (tmp<=maxval)) {
                GETIP(addr+delta);
                ADDI(ret, xRIP, tmp);
            } else if(tmp+addr+delta<0x100000000LL) {
                MOV64x(ret, tmp+addr+delta);
            } else {
                if(adj) {
                    MOV64x(ret, tmp+adj);
                } else {
                    MOV64x(ret, tmp);
                    GETIP(addr+delta);
                }
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
        int sib_reg2 = (sib&0x07)+(rex.b<<3);
        if(nextop&0x80)
            i64 = F32S;
        else
            i64 = F8S;
        if(i64==0 || ((i64>=-2048) && (i64<=2047)  && i12)) {
            *fixaddress = i64;
            if((nextop&7)==4) {
                if (sib_reg!=4) {
                    ADDSL(ret, xRAX+sib_reg2, xRAX+sib_reg, sib>>6, scratch);
                } else {
                    ret = xRAX+sib_reg2;
                }
            } else
                ret = xRAX+(nextop&0x07)+(rex.b<<3);
        } else {
            if(i64>=-2048 && i64<=2047) {
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        ADDSL(scratch, xRAX+sib_reg2, xRAX+sib_reg, sib>>6, scratch);
                    } else {
                        scratch = xRAX+sib_reg2;
                    }
                } else
                    scratch = xRAX+(nextop&0x07)+(rex.b<<3);
                ADDI(ret, scratch, i64);
            } else {
                MOV64x(scratch, i64);
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        ADD(scratch, scratch, xRAX+sib_reg2);
                        ADDSL(ret, scratch, xRAX+sib_reg, sib>>6, ret);
                    } else {
                        PASS3(int tmp = xRAX+sib_reg2);
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

static uintptr_t geted_32(dynarec_rv64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, int *l, int i12)
{
    MAYUSE(dyn); MAYUSE(ninst);

    int lock = l?((l==LOCK_LOCK)?1:2):0;
    if(lock==2)
        *l = 0;
    uint8_t ret = x2;
    *fixaddress = 0;
    if(hint>0) ret = hint;
    int maxval = 2047;
    if(i12>1)
        maxval -= i12;
    MAYUSE(scratch);
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            int sib_reg = (sib>>3)&0x7;
            int sib_reg2 = sib&0x7;
            if(sib_reg2==5) {
                int64_t tmp = F32S;
                if (sib_reg!=4) {
                    if(tmp && ((tmp<-2048) || (tmp>maxval) || !i12)) {
                        MOV32w(scratch, tmp);
                        if((sib>>6)) {
                            SLLI(ret, xRAX + sib_reg, sib >> 6);
                            ADDW(ret, ret, scratch);
                        } else
                            ADDW(ret, xRAX+sib_reg, scratch);
                    } else {
                        if(sib>>6)
                            SLLI(ret, xRAX+sib_reg, (sib>>6));
                        else
                            ret = xRAX+sib_reg;
                        *fixaddress = tmp;
                    }
                } else {
                    switch(lock) {
                        case 1: addLockAddress((int32_t)tmp); break;
                        case 2: if(isLockAddress((int32_t)tmp)) *l=1; break;
                    }
                    MOV32w(ret, tmp);
                }
            } else {
                if (sib_reg!=4) {
                    if((sib>>6)) {
                        SLLI(ret, xRAX + sib_reg, (sib >> 6));
                        ADDW(ret, ret, xRAX + sib_reg2);
                    } else
                        ADDW(ret, xRAX+sib_reg2, xRAX+sib_reg);
                } else {
                    ret = xRAX+sib_reg2;
                }
            }
        } else if((nextop&7)==5) {
            uint32_t tmp = F32;
            MOV32w(ret, tmp);
            switch(lock) {
                case 1: addLockAddress(tmp); break;
                case 2: if(isLockAddress(tmp)) *l=1; break;
            }
        } else {
            ret = xRAX+(nextop&7);
            if(ret==hint) {
                AND(hint, ret, xMASK);    //to clear upper part
            }
        }
    } else {
        int64_t i32;
        uint8_t sib = 0;
        int sib_reg = 0;
        if((nextop&7)==4) {
            sib = F8;
            sib_reg = (sib>>3)&7;
        }
        int sib_reg2 = sib&0x07;
        if(nextop&0x80)
            i32 = F32S;
        else
            i32 = F8S;
        if(i32==0 || ((i32>=-2048) && (i32<=2047)  && i12)) {
            *fixaddress = i32;
            if((nextop&7)==4) {
                if (sib_reg!=4) {
                    if(sib>>6) {
                        SLLI(ret, xRAX + sib_reg, (sib >> 6));
                        ADDW(ret, ret, xRAX + sib_reg2);
                    } else
                        ADDW(ret, xRAX+sib_reg2, xRAX+sib_reg);
                } else {
                    ret = xRAX+sib_reg2;
                }
            } else {
                ret = xRAX+(nextop&0x07);
            }
        } else {
            if(i32>=-2048 && i32<=2047) {
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        if(sib>>6) {
                            SLLI(scratch, xRAX + sib_reg, sib >> 6);
                            ADDW(scratch, scratch, xRAX + sib_reg2);
                        } else
                            ADDW(scratch, xRAX+sib_reg2, xRAX+sib_reg);
                    } else {
                        scratch = xRAX+sib_reg2;
                    }
                } else
                    scratch = xRAX+(nextop&0x07);
                ADDIW(ret, scratch, i32);
            } else {
                MOV32w(scratch, i32);
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        ADDW(scratch, scratch, xRAX+sib_reg2);
                        if(sib>>6) {
                            SLLI(ret, xRAX + sib_reg, (sib >> 6));
                            ADDW(ret, ret, scratch);
                        } else
                            ADDW(ret, scratch, xRAX+sib_reg);
                    } else {
                        PASS3(int tmp = xRAX+sib_reg2);
                        ADDW(ret, tmp, scratch);
                    }
                } else {
                    PASS3(int tmp = xRAX+(nextop&0x07));
                    ADDW(ret, tmp, scratch);
                }
            }
        }
    }
    *ed = ret;
    return addr;
}

/* setup r2 to address pointed by ED, also fixaddress is an optionnal delta in the range [-absmax, +absmax], with delta&mask==0 to be added to ed for LDR/STR */
uintptr_t geted32(dynarec_rv64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, rex_t rex, int *l, int i12, int delta)
{
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(delta);

    int lock = l?((l==LOCK_LOCK)?1:2):0;
    if(lock==2)
        *l = 0;
    uint8_t ret = x2;
    *fixaddress = 0;
    if(hint>0) ret = hint;
    int maxval = 2047;
    if(i12>1)
        maxval -= i12;
    MAYUSE(scratch);
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            int sib_reg = ((sib>>3)&0x7)+(rex.x<<3);
            int sib_reg2 = (sib&0x7)+(rex.b<<3);
            if((sib&0x7)==5) {
                int64_t tmp = F32S;
                if (sib_reg!=4) {
                    if(tmp && ((tmp<-2048) || (tmp>maxval) || !i12)) {
                        MOV64x(scratch, tmp);
                        if((sib>>6)) {
                            SLLI(ret, xRAX + sib_reg, sib >> 6);
                            ADDW(ret, ret, scratch);
                        } else
                            ADDW(ret, xRAX+sib_reg, scratch);
                    } else {
                        if(sib>>6)
                            SLLI(ret, xRAX+sib_reg, (sib>>6));
                        else
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
                    if((sib>>6)) {
                        SLLI(ret, xRAX + sib_reg, (sib >> 6));
                        ADDW(ret, ret, xRAX + sib_reg2);
                    } else
                        ADDW(ret, xRAX+sib_reg2, xRAX+sib_reg);
                } else {
                    ret = xRAX+sib_reg2;
                }
            }
        } else if((nextop&7)==5) {
            uint32_t tmp = F32;
            MOV32w(ret, tmp);
            GETIP(addr+delta);
            ADDW(ret, ret, xRIP);
            switch(lock) {
                case 1: addLockAddress(addr+delta+tmp); break;
                case 2: if(isLockAddress(addr+delta+tmp)) *l=1; break;
            }
        } else {
            ret = xRAX+(nextop&7)+(rex.b<<3);
            if(ret==hint) {
                AND(hint, ret, xMASK);    //to clear upper part
            }
        }
    } else {
        int64_t i64;
        uint8_t sib = 0;
        int sib_reg = 0;
        if((nextop&7)==4) {
            sib = F8;
            sib_reg = ((sib>>3)&7)+(rex.x<<3);
        }
        int sib_reg2 = (sib&0x07)+(rex.b<<3);
        if(nextop&0x80)
            i64 = F32S;
        else
            i64 = F8S;
        if(i64==0 || ((i64>=-2048) && (i64<=2047)  && i12)) {
            *fixaddress = i64;
            if((nextop&7)==4) {
                if (sib_reg!=4) {
                    if(sib>>6) {
                        SLLI(ret, xRAX + sib_reg, (sib >> 6));
                        ADDW(ret, ret, xRAX + sib_reg2);
                    } else
                        ADDW(ret, xRAX+sib_reg2, xRAX+sib_reg);
                } else {
                    ret = xRAX+sib_reg2;
                }
            } else {
                ret = xRAX+(nextop&0x07)+(rex.b<<3);
            }
        } else {
            if(i64>=-2048 && i64<=2047) {
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        if(sib>>6) {
                            SLLI(scratch, xRAX + sib_reg, sib >> 6);
                            ADDW(scratch, scratch, xRAX + sib_reg2);
                        } else
                            ADDW(scratch, xRAX+sib_reg2, xRAX+sib_reg);
                    } else {
                        scratch = xRAX+sib_reg2;
                    }
                } else
                    scratch = xRAX+(nextop&0x07)+(rex.b<<3);
                ADDIW(ret, scratch, i64);
            } else {
                MOV32w(scratch, i64);
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        ADDW(scratch, scratch, xRAX+sib_reg2);
                        if(sib>>6) {
                            SLLI(ret, xRAX + sib_reg, (sib >> 6));
                            ADDW(ret, ret, scratch);
                        } else
                            ADDW(ret, scratch, xRAX+sib_reg);
                    } else {
                        PASS3(int tmp = xRAX+sib_reg2);
                        ADDW(ret, tmp, scratch);
                    }
                } else {
                    PASS3(int tmp = xRAX+(nextop&0x07)+(rex.b<<3));
                    ADDW(ret, tmp, scratch);
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

void jump_to_epilog_fast(dynarec_rv64_t* dyn, uintptr_t ip, int reg, int ninst)
{
    MAYUSE(dyn); MAYUSE(ip); MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Jump to epilog_fast\n");

    if(reg) {
        if(reg!=xRIP) {
            MV(xRIP, reg);
        }
    } else {
        GETIP_(ip);
    }
    TABLE64(x2, (uintptr_t)rv64_epilog_fast);
    SMEND();
    BR(x2);
}
#ifdef JMPTABLE_SHIFT4
#error TODO!
#endif
void jump_to_next(dynarec_rv64_t* dyn, uintptr_t ip, int reg, int ninst, int is32bits)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Jump to next\n");

    if(reg) {
        if(reg!=xRIP) {
            MV(xRIP, reg);
        }
        uintptr_t tbl = is32bits?getJumpTable32():getJumpTable64();
        MAYUSE(tbl);
        TABLE64(x3, tbl);
        if(!is32bits) {
            SRLI(x2, xRIP, JMPTABL_START3);
            if(rv64_zba) SH3ADD(x3, x2, x3); else {SLLI(x2, x2, 3); ADD(x3, x3, x2);}
            LD(x3, x3, 0); // could be LR_D(x3, x3, 1, 1); for better safety
        }
        MOV64x(x4, JMPTABLE_MASK2<<3);    // x4 = mask
        SRLI(x2, xRIP, JMPTABL_START2-3);
        AND(x2, x2, x4);
        ADD(x3, x3, x2);
        LD(x3, x3, 0); //LR_D(x3, x3, 1, 1);
        if(JMPTABLE_MASK2!=JMPTABLE_MASK1) {
            MOV64x(x4, JMPTABLE_MASK1<<3);    // x4 = mask
        }
        SRLI(x2, xRIP, JMPTABL_START1-3);
        AND(x2, x2, x4);
        ADD(x3, x3, x2);
        LD(x3, x3, 0); //LR_D(x3, x3, 1, 1);
        if(JMPTABLE_MASK0<2048) {
            ANDI(x2, xRIP, JMPTABLE_MASK0);
        } else {
            if(JMPTABLE_MASK1!=JMPTABLE_MASK0) {
                MOV64x(x4, JMPTABLE_MASK0);    // x4 = mask
            }
            AND(x2, xRIP, x4);
        }
        if(rv64_zba) SH3ADD(x3, x2, x3); else {SLLI(x2, x2, 3); ADD(x3, x3, x2);}
        LD(x2, x3, 0); //LR_D(x2, x3, 1, 1);
    } else {
        uintptr_t p = getJumpTableAddress64(ip);
        MAYUSE(p);
        TABLE64(x3, p);
        GETIP_(ip);
        LD(x2, x3, 0); //LR_D(x2, x3, 1, 1);
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

void ret_to_epilog(dynarec_rv64_t* dyn, int ninst, rex_t rex)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Ret to epilog\n");
    POP1z(xRIP);
    MVz(x1, xRIP);
    SMEND();
    if (box64_dynarec_callret) {
        // pop the actual return address from RV64 stack
        LD(x2, xSP, 0);     // native addr
        LD(x6, xSP, 8);     // x86 addr
        ADDI(xSP, xSP, 16); // pop
        BNE(x6, xRIP, 2*4); // is it the right address?
        JALR(x2);
        // not the correct return address, regular jump, but purge the stack first, it's unsync now...
        LD(xSP, xEmu, offsetof(x64emu_t, xSPSave));
        ADDI(xSP, xSP, -16);
    }

    uintptr_t tbl = rex.is32bits?getJumpTable32():getJumpTable64();
    MOV64x(x3, tbl);
    if(!rex.is32bits) {
        SRLI(x2, xRIP, JMPTABL_START3);
        if(rv64_zba) SH3ADD(x3, x2, x3); else {SLLI(x2, x2, 3); ADD(x3, x3, x2);}
        LD(x3, x3, 0);
    }
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
    if(rv64_zba) SH3ADD(x3, x2, x3); else {SLLI(x2, x2, 3); ADD(x3, x3, x2);}
    LD(x2, x3, 0);
    JALR(x2); // save LR
    CLEARIP();
}

void retn_to_epilog(dynarec_rv64_t* dyn, int ninst, rex_t rex, int n)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Retn to epilog\n");
    POP1z(xRIP);
    if(n>0x7ff) {
        MOV64x(w1, n);
        ADDz(xRSP, xRSP, x1);
    } else {
        ADDIz(xRSP, xRSP, n);
    }
    MVz(x1, xRIP);
    SMEND();
    if (box64_dynarec_callret) {
        // pop the actual return address from RV64 stack
        LD(x2, xSP, 0);     // native addr
        LD(x6, xSP, 8);     // x86 addr
        ADDI(xSP, xSP, 16); // pop
        BNE(x6, xRIP, 2*4); // is it the right address?
        JALR(x2);
        // not the correct return address, regular jump, but purge the stack first, it's unsync now...
        LD(xSP, xEmu, offsetof(x64emu_t, xSPSave));
        ADDI(xSP, xSP, -16);
    }
    uintptr_t tbl = rex.is32bits?getJumpTable32():getJumpTable64();
    MOV64x(x3, tbl);
    if(!rex.is32bits) {
        SRLI(x2, xRIP, JMPTABL_START3);
        if(rv64_zba) SH3ADD(x3, x2, x3); else {SLLI(x2, x2, 3); ADD(x3, x3, x2);}
        LD(x3, x3, 0);
    }
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
    if(rv64_zba) SH3ADD(x3, x2, x3); else {SLLI(x2, x2, 3); ADD(x3, x3, x2);}
    LD(x2, x3, 0);
    JALR(x2); // save LR
    CLEARIP();
}

void iret_to_epilog(dynarec_rv64_t* dyn, int ninst, int is64bits)
{
    //#warning TODO: is64bits
    MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "IRet to epilog\n");
    NOTEST(x2);
    if(is64bits) {
        POP1(xRIP);
        POP1(x2);
        POP1(xFlags);
    } else {
        POP1_32(xRIP);
        POP1_32(x2);
        POP1_32(xFlags);
    }

    SH(x2, xEmu, offsetof(x64emu_t, segs[_CS]));
    SW(xZR, xEmu, offsetof(x64emu_t, segs_serial[_CS]));
    // clean EFLAGS
    MOV32w(x1, 0x3F7FD7);
    AND(xFlags, xFlags, x1);
    ORI(xFlags, xFlags, 0x2);
    SET_DFNONE();
    // POP RSP
    if (is64bits) {
        POP1(x3);   //rsp
        POP1(x2);   //ss
    } else {
        POP1_32(x3);   //rsp
        POP1_32(x2);   //ss
    }
    // POP SS
    SH(x2, xEmu, offsetof(x64emu_t, segs[_SS]));
    SW(xZR, xEmu, offsetof(x64emu_t, segs_serial[_SS]));
    // set new RSP
    MV(xRSP, x3);
    // Ret....
    MOV64x(x2, (uintptr_t)rv64_epilog);  // epilog on purpose, CS might have changed!
    SMEND();
    BR(x2);
    CLEARIP();
}

void call_c(dynarec_rv64_t* dyn, int ninst, void* fnc, int reg, int ret, int saveflags, int savereg)
{
    MAYUSE(fnc);
    if(savereg==0)
        savereg = x6;
    if(saveflags) {
        FLAGS_ADJUST_TO11(xFlags, xFlags, reg);
        SD(xFlags, xEmu, offsetof(x64emu_t, eflags));
    }
    fpu_pushcache(dyn, ninst, reg, 0);
    if(ret!=-2) {
        SUBI(xSP, xSP, 16);   // RV64 stack needs to be 16byte aligned
        SD(xEmu, xSP, 0);
        SD(savereg, xSP, 8);
        // x5..x8, x10..x17, x28..x31 those needs to be saved by caller
        STORE_REG(RAX);
        STORE_REG(RCX);
        STORE_REG(RDX);
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
        GO(RDX);
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
        FLAGS_ADJUST_FROM11(xFlags, xFlags, reg);
    }
    //SET_NODF();
}

void call_n(dynarec_rv64_t* dyn, int ninst, void* fnc, int w)
{
    MAYUSE(fnc);
    FLAGS_ADJUST_TO11(xFlags, xFlags, x3);
    SD(xFlags, xEmu, offsetof(x64emu_t, eflags));
    fpu_pushcache(dyn, ninst, x3, 1);
    // x5..x8, x10..x17, x28..x31 those needs to be saved by caller
    // RDI, RSI, RDX, RCX, R8, R9 are used for function call
    SUBI(xSP, xSP, 16);
    SD(xEmu, xSP, 0);
    SD(xRIP, xSP, 8); // RV64 stack needs to be 16byte aligned
    STORE_REG(R12);
    STORE_REG(R13);
    STORE_REG(R14);
    STORE_REG(R15);
    /*
    // float and double args
    if (abs(w) > 1) {
        MESSAGE(LOG_DUMP, "Getting %d XMM args\n", abs(w) - 1);
        for (int i = 0; i < abs(w) - 1; ++i) {
            sse_get_reg(dyn, ninst, x6, i, 0);
        }
    }
    if (w < 0) {
        MESSAGE(LOG_DUMP, "Return in XMM0\n");
        sse_get_reg_empty(dyn, ninst, x6, 0, 0);
    }
    */
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
    FLAGS_ADJUST_FROM11(xFlags, xFlags, x3);
    //SET_NODF();
}

void grab_segdata(dynarec_rv64_t* dyn, uintptr_t addr, int ninst, int reg, int segment)
{
    (void)addr;
    int64_t j64;
    MAYUSE(j64);
    MESSAGE(LOG_DUMP, "Get %s Offset\n", (segment==_FS)?"FS":"GS");
    int t1 = x1, t2 = x4;
    if(reg==t1) ++t1;
    if(reg==t2) ++t2;
    LWU(t2, xEmu, offsetof(x64emu_t, segs_serial[segment]));
    LD(reg, xEmu, offsetof(x64emu_t, segs_offs[segment]));
    if(segment==_GS) {
        CBNZ_MARKSEG(t2);   // fast check
    } else {
        LD(t1, xEmu, offsetof(x64emu_t, context));
        LWU(t1, t1, offsetof(box64context_t, sel_serial));
        SUBW(t1, t1, t2);
        CBZ_MARKSEG(t1);
    }
    MOV64x(x1, segment);
    call_c(dyn, ninst, GetSegmentBaseEmu, t2, reg, 0, xFlags);
    MARKSEG;
    MESSAGE(LOG_DUMP, "----%s Offset\n", (segment==_FS)?"FS":"GS");
}

int x87_stackcount(dynarec_rv64_t* dyn, int ninst, int scratch)
{
    MAYUSE(scratch);
    if(!dyn->e.x87stack)
        return 0;
    if(dyn->e.mmxcount)
        mmx_purgecache(dyn, ninst, 0, scratch);
    MESSAGE(LOG_DUMP, "\tSynch x87 Stackcount (%d)\n", dyn->e.x87stack);
    int a = dyn->e.x87stack;
    // Add x87stack to emu fpu_stack
    LW(scratch, xEmu, offsetof(x64emu_t, fpu_stack));
    ADDI(scratch, scratch, a);
    SW(scratch, xEmu, offsetof(x64emu_t, fpu_stack));
    // Sub x87stack to top, with and 7
    LW(scratch, xEmu, offsetof(x64emu_t, top));
    SUBI(scratch, scratch, a);
    ANDI(scratch, scratch, 7);
    SW(scratch, xEmu, offsetof(x64emu_t, top));
    // reset x87stack, but not the stack count of extcache
    dyn->e.x87stack = 0;
    dyn->e.stack_next -= dyn->e.stack;
    int ret = dyn->e.stack;
    dyn->e.stack = 0;
    MESSAGE(LOG_DUMP, "\t------x87 Stackcount\n");
    return ret;
}
void x87_unstackcount(dynarec_rv64_t* dyn, int ninst, int scratch, int count)
{
    MAYUSE(scratch);
    if(!count)
        return;
    if(dyn->e.mmxcount)
        mmx_purgecache(dyn, ninst, 0, scratch);
    MESSAGE(LOG_DUMP, "\tSynch x87 Unstackcount (%d)\n", count);
    int a = -count;
    // Add x87stack to emu fpu_stack
    LW(scratch, xEmu, offsetof(x64emu_t, fpu_stack));
    ADDI(scratch, scratch, a);
    SW(scratch, xEmu, offsetof(x64emu_t, fpu_stack));
    // Sub x87stack to top, with and 7
    LW(scratch, xEmu, offsetof(x64emu_t, top));
    SUBI(scratch, scratch, a);
    ANDI(scratch, scratch, 7);
    SW(scratch, xEmu, offsetof(x64emu_t, top));
    // reset x87stack, but not the stack count of extcache
    dyn->e.x87stack = count;
    dyn->e.stack = count;
    dyn->e.stack_next += dyn->e.stack;
    MESSAGE(LOG_DUMP, "\t------x87 Unstackcount\n");
}
int extcache_st_coherency(dynarec_rv64_t* dyn, int ninst, int a, int b)
{
    int i1 = extcache_get_st(dyn, ninst, a);
    int i2 = extcache_get_st(dyn, ninst, b);
    if(i1!=i2) {
        MESSAGE(LOG_DUMP, "Warning, ST cache incoherent between ST%d(%d) and ST%d(%d)\n", a, i1, b, i2);
    }

    return i1;
}

// On step 1, Float/Double for ST is actualy computed and back-propagated
// On step 2-3, the value is just read for inst[...].n.neocache[..]
// the reg returned is *2 for FLOAT
int x87_do_push(dynarec_rv64_t* dyn, int ninst, int s1, int t)
{
    if(dyn->e.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    dyn->e.x87stack+=1;
    dyn->e.stack+=1;
    dyn->e.stack_next+=1;
    dyn->e.stack_push+=1;
    ++dyn->e.pushed;
    if(dyn->e.poped)
        --dyn->e.poped;
    // move all regs in cache, and find a free one
    for(int j=0; j<24; ++j)
        if ((dyn->e.extcache[j].t == EXT_CACHE_ST_D)
            || (dyn->e.extcache[j].t == EXT_CACHE_ST_F)
            || (dyn->e.extcache[j].t == EXT_CACHE_ST_I64))
            ++dyn->e.extcache[j].n;
    int ret = -1;
    dyn->e.tags<<=2;
    for(int i=0; i<8; ++i)
        if(dyn->e.x87cache[i]!=-1)
            ++dyn->e.x87cache[i];
        else if(ret==-1) {
            dyn->e.x87cache[i] = 0;
            ret=dyn->e.x87reg[i]=fpu_get_reg_x87(dyn, t, 0);
            dyn->e.extcache[EXTIDX(ret)].t = X87_ST0;
        }
    if(ret==-1) {
        MESSAGE(LOG_DUMP, "Incoherent x87 stack cache, aborting\n");
        dyn->abort = 1;
    }
    return ret;
}
void x87_do_push_empty(dynarec_rv64_t* dyn, int ninst, int s1)
{
    if(dyn->e.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    dyn->e.x87stack+=1;
    dyn->e.stack+=1;
    dyn->e.stack_next+=1;
    dyn->e.stack_push+=1;
    ++dyn->e.pushed;
    if(dyn->e.poped)
        --dyn->e.poped;
    // move all regs in cache
    for(int j=0; j<24; ++j)
        if ((dyn->e.extcache[j].t == EXT_CACHE_ST_D)
            || (dyn->e.extcache[j].t == EXT_CACHE_ST_F)
            || (dyn->e.extcache[j].t == EXT_CACHE_ST_I64))
            ++dyn->e.extcache[j].n;
    int ret = -1;
    dyn->e.tags<<=2;
    for(int i=0; i<8; ++i)
        if(dyn->e.x87cache[i]!=-1)
            ++dyn->e.x87cache[i];
        else if(ret==-1)
            ret = i;
    if(ret==-1) {
        MESSAGE(LOG_DUMP, "Incoherent x87 stack cache, aborting\n");
        dyn->abort = 1;
    }
    if(s1)
        x87_stackcount(dyn, ninst, s1);
}
void static internal_x87_dopop(dynarec_rv64_t* dyn)
{
    for(int i=0; i<8; ++i)
        if(dyn->e.x87cache[i]!=-1) {
            --dyn->e.x87cache[i];
            if(dyn->e.x87cache[i]==-1) {
                fpu_free_reg(dyn, dyn->e.x87reg[i]);
                dyn->e.x87reg[i] = -1;
            }
        }
}
int static internal_x87_dofree(dynarec_rv64_t* dyn)
{
    if(dyn->e.tags&0b11) {
        MESSAGE(LOG_DUMP, "\t--------x87 FREED ST0, poping 1 more\n");
        return 1;
    }
    return 0;
}
void x87_do_pop(dynarec_rv64_t* dyn, int ninst, int s1)
{
    if(dyn->e.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    do {
        dyn->e.x87stack-=1;
        dyn->e.stack_next-=1;
        dyn->e.stack_pop+=1;
        dyn->e.tags>>=2;
        ++dyn->e.poped;
        if(dyn->e.pushed)
            --dyn->e.pushed;
        // move all regs in cache, poping ST0
        internal_x87_dopop(dyn);
    } while(internal_x87_dofree(dyn));
}

void x87_purgecache(dynarec_rv64_t* dyn, int ninst, int next, int s1, int s2, int s3)
{
    int ret = 0;
    for (int i=0; i<8 && !ret; ++i)
        if(dyn->e.x87cache[i] != -1)
            ret = 1;
    if(!ret && !dyn->e.x87stack)    // nothing to do
        return;
    MESSAGE(LOG_DUMP, "\tPurge %sx87 Cache and Synch Stackcount (%+d)---\n", next?"locally ":"", dyn->e.x87stack);
    int a = dyn->e.x87stack;
    if(a!=0) {
        // reset x87stack
        if(!next)
            dyn->e.x87stack = 0;
        // Add x87stack to emu fpu_stack
        LW(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        ADDI(s2, s2, a);
        SW(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        // Sub x87stack to top, with and 7
        LW(s2, xEmu, offsetof(x64emu_t, top));
        // update tags (and top at the same time)
        if(a>0) {
            SUBI(s2, s2, a);
        } else {
            ADDI(s2, s2, -a);
        }
        ANDI(s2, s2, 7);
        SW(s2, xEmu, offsetof(x64emu_t, top));
        // update tags (and top at the same time)
        LHU(s1, xEmu, offsetof(x64emu_t, fpu_tags));
        if(a>0) {
            SLLI(s1, s1, a*2);
        } else {
            SLLI(s3, xMASK, 16);    // 0xffff0000 (plus some unused hipart)
            OR(s1, s1, s3);
            SRLI(s1, s1, -a*2);
        }
        SH(s1, xEmu, offsetof(x64emu_t, fpu_tags));
    } else {
        LW(s2, xEmu, offsetof(x64emu_t, top));
    }
    // check if free is used
    if(dyn->e.tags) {
        LH(s1, xEmu, offsetof(x64emu_t, fpu_tags));
        MOV32w(s3, dyn->e.tags);
        OR(s1, s1, s3);
        SH(s1, xEmu, offsetof(x64emu_t, fpu_tags));
    }
    if(ret!=0) {
        // --- set values
        // Get top
        // loop all cache entries
        for (int i=0; i<8; ++i)
            if(dyn->e.x87cache[i]!=-1) {
                int st = dyn->e.x87cache[i]+dyn->e.stack_pop;
                #if STEP == 1
                if(!next) {   // don't force promotion here
                    // pre-apply pop, because purge happens in-between
                    extcache_promote_double(dyn, ninst, st);
                }
                #endif
                #if STEP == 3
                if(!next && extcache_get_current_st(dyn, ninst, st) != EXT_CACHE_ST_D) {
                    MESSAGE(LOG_DUMP, "Warning, incoherency with purged ST%d cache\n", st);
                }
                #endif
                ADDI(s3, s2, dyn->e.x87cache[i]); // unadjusted count, as it's relative to real top
                ANDI(s3, s3, 7);   // (emu->top + st)&7
                if(rv64_zba) SH3ADD(s1, s3, xEmu); else {SLLI(s1, s3, 3); ADD(s1, xEmu, s1);}
                switch(extcache_get_current_st(dyn, ninst, st)) {
                    case EXT_CACHE_ST_D:
                        FSD(dyn->e.x87reg[i], s1, offsetof(x64emu_t, x87));    // save the value
                        break;
                    case EXT_CACHE_ST_F:
                        FCVTDS(SCRATCH0, dyn->e.x87reg[i]);
                        FSD(SCRATCH0, s1, offsetof(x64emu_t, x87));    // save the value
                        break;
                    case EXT_CACHE_ST_I64:
                        FMVXD(s2, dyn->e.x87reg[i]);
                        FCVTDL(SCRATCH0, s2, RD_RTZ);
                        FSD(SCRATCH0, s1, offsetof(x64emu_t, x87));    // save the value
                        break;
                }
                if(!next) {
                    fpu_free_reg(dyn, dyn->e.x87reg[i]);
                    dyn->e.x87reg[i] = -1;
                    dyn->e.x87cache[i] = -1;
                    //dyn->e.stack_pop+=1; //no pop, but the purge because of barrier will have the n.barrier flags set
                }
            }
    }
    if(!next) {
        dyn->e.stack_next = 0;
        dyn->e.tags = 0;
        #if STEP < 2
        // refresh the cached valued, in case it's a purge outside a instruction
        dyn->insts[ninst].e.barrier = 1;
        dyn->e.pushed = 0;
        dyn->e.poped = 0;

        #endif
    }
    MESSAGE(LOG_DUMP, "\t---Purge x87 Cache and Synch Stackcount\n");
}

static void x87_reflectcache(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3)
{
    //Sync top and stack count
    int a = dyn->e.x87stack;
    if(a) {
        // Add x87stack to emu fpu_stack
        LW(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        ADDI(s2, s2, a);
        SW(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        // Sub x87stack to top, with and 7
        LW(s2, xEmu, offsetof(x64emu_t, top));
        SUBI(s2, s2, a);
        ANDI(s2, s2, 7);
        SW(s2, xEmu, offsetof(x64emu_t, top));
        // update tags (and top at the same time)
        LH(s1, xEmu, offsetof(x64emu_t, fpu_tags));
        if(a>0) {
            SLLI(s1, s1, a*2);
        } else {
            SLLI(s3, xMASK, 16);    // 0xffff0000
            OR(s1, s1, s3);
            SRLI(s1, s1, -a*2);
        }
        SH(s1, xEmu, offsetof(x64emu_t, fpu_tags));
    }
    int ret = 0;
    for (int i=0; (i<8) && (!ret); ++i)
        if(dyn->e.x87cache[i] != -1)
            ret = 1;
    if(!ret)    // nothing to do
        return;
    // prepare offset to fpu => s1
    // Get top
    if(!a) {
        LW(s2, xEmu, offsetof(x64emu_t, top));
    }
    // loop all cache entries
    for (int i=0; i<8; ++i)
        if(dyn->e.x87cache[i]!=-1) {
            ADDI(s3, s2, dyn->e.x87cache[i]);
            ANDI(s3, s3, 7);   // (emu->top + i)&7
            if(rv64_zba) SH3ADD(s1, s3, xEmu); else {SLLI(s1, s3, 3); ADD(s1, xEmu, s1);}
            if(extcache_get_st_f(dyn, ninst, dyn->e.x87cache[i])>=0) {
                FCVTDS(SCRATCH0, dyn->e.x87reg[i]);
                FSD(SCRATCH0, s1, offsetof(x64emu_t, x87));
            } else
                FSD(dyn->e.x87reg[i], s1, offsetof(x64emu_t, x87));
        }
}

static void x87_unreflectcache(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3)
{
    // revert top and stack count
    int a = dyn->e.x87stack;
    if(a) {
        // Sub x87stack to emu fpu_stack
        LW(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        SUBI(s2, s2, a);
        SW(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        // Add x87stack to top, with and 7
        LW(s2, xEmu, offsetof(x64emu_t, top));
        ADDI(s2, s2, a);
        ANDI(s2, s2, 7);
        SW(s2, xEmu, offsetof(x64emu_t, top));
        if(a>0) {
            SLLI(s3, xMASK, 16);    // 0xffff0000
            OR(s1, s1, s3);
            SRLI(s1, s1, a*2);
        } else {
            SLLI(s1, s1, -a*2);
        }
    }
}

int x87_get_current_cache(dynarec_rv64_t* dyn, int ninst, int st, int t)
{
    // search in cache first
    for (int i=0; i<8; ++i) {
        if(dyn->e.x87cache[i]==st) {
            #if STEP == 1
            if (t == EXT_CACHE_ST_D && (dyn->e.extcache[EXTIDX(dyn->e.x87reg[i])].t == EXT_CACHE_ST_F || dyn->e.extcache[EXTIDX(dyn->e.x87reg[i])].t == EXT_CACHE_ST_I64))
                extcache_promote_double(dyn, ninst, st);
            else if (t == EXT_CACHE_ST_I64 && (dyn->e.extcache[EXTIDX(dyn->e.x87reg[i])].t == EXT_CACHE_ST_F))
                extcache_promote_double(dyn, ninst, st);
            else if (t == EXT_CACHE_ST_F && (dyn->e.extcache[EXTIDX(dyn->e.x87reg[i])].t == EXT_CACHE_ST_I64))
                extcache_promote_double(dyn, ninst, st);
#endif
            return i;
        }
        assert(dyn->e.x87cache[i]<8);
    }
    return -1;
}

int x87_get_cache(dynarec_rv64_t* dyn, int ninst, int populate, int s1, int s2, int st, int t)
{
    if(dyn->e.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    int ret = x87_get_current_cache(dyn, ninst, st, t);
    if(ret!=-1)
        return ret;
    MESSAGE(LOG_DUMP, "\tCreate %sx87 Cache for ST%d\n", populate?"and populate ":"", st);
    // get a free spot
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->e.x87cache[i]==-1)
            ret = i;
    // found, setup and grab the value
    dyn->e.x87cache[ret] = st;
    dyn->e.x87reg[ret] = fpu_get_reg_x87(dyn, EXT_CACHE_ST_D, st);
    if(populate) {
        LW(s2, xEmu, offsetof(x64emu_t, top));
        int a = st - dyn->e.x87stack;
        if(a) {
            ADDI(s2, s2, a);
            ANDI(s2, s2, 7);
        }
        if(rv64_zba) SH3ADD(s1, s2, xEmu); else  {SLLI(s2, s2, 3); ADD(s1, xEmu, s2);}
        FLD(dyn->e.x87reg[ret], s1, offsetof(x64emu_t, x87));
    }
    MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);

    return ret;
}
int x87_get_extcache(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int st)
{
    for(int ii=0; ii<24; ++ii)
        if ((dyn->e.extcache[ii].t == EXT_CACHE_ST_F
                || dyn->e.extcache[ii].t == EXT_CACHE_ST_D
                || dyn->e.extcache[ii].t == EXT_CACHE_ST_I64)
            && dyn->e.extcache[ii].n == st)
            return ii;
    assert(0);
    return -1;
}
int x87_get_st(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int a, int t)
{
    return dyn->e.x87reg[x87_get_cache(dyn, ninst, 1, s1, s2, a, t)];
}
int x87_get_st_empty(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int a, int t)
{
    return dyn->e.x87reg[x87_get_cache(dyn, ninst, 0, s1, s2, a, t)];
}


void x87_refresh(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int st)
{
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->e.x87cache[i] == st)
            ret = i;
    if(ret==-1)    // nothing to do
        return;
    MESSAGE(LOG_DUMP, "\tRefresh x87 Cache for ST%d\n", st);
    const int reg = dyn->e.x87reg[ret];
    // prepare offset to fpu => s1
    // Get top
    LW(s2, xEmu, offsetof(x64emu_t, top));
    // Update
    int a = st - dyn->e.x87stack;
    if(a) {
        ADDI(s2, s2, a);
        ANDI(s2, s2, 7);    // (emu->top + i)&7
    }
    if(rv64_zba) SH3ADD(s1, s2, xEmu); else {SLLI(s2, s2, 3); ADD(s1, xEmu, s2);}
    if (dyn->e.extcache[EXTIDX(reg)].t == EXT_CACHE_ST_F) {
        FCVTDS(SCRATCH0, reg);
        FSD(SCRATCH0, s1, offsetof(x64emu_t, x87));
    } else if (dyn->e.extcache[EXTIDX(reg)].t == EXT_CACHE_ST_I64) {
        FMVXD(s2, reg);
        FCVTDL(SCRATCH0, s2, RD_RTZ);
        FSD(SCRATCH0, s1, offsetof(x64emu_t, x87));
    } else {
        FSD(reg, s1, offsetof(x64emu_t, x87));
    }
    MESSAGE(LOG_DUMP, "\t--------x87 Cache for ST%d\n", st);
}

void x87_forget(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int st)
{
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->e.x87cache[i] == st)
            ret = i;
    if(ret==-1)    // nothing to do
        return;
    MESSAGE(LOG_DUMP, "\tForget x87 Cache for ST%d\n", st);
    const int reg = dyn->e.x87reg[ret];
    #if STEP == 1
    if (dyn->e.extcache[EXTIDX(dyn->e.x87reg[ret])].t == EXT_CACHE_ST_F
        || dyn->e.extcache[EXTIDX(dyn->e.x87reg[ret])].t == EXT_CACHE_ST_I64)
        extcache_promote_double(dyn, ninst, st);
    #endif
    // prepare offset to fpu => s1
    // Get top
    LW(s2, xEmu, offsetof(x64emu_t, top));
    // Update
    int a = st - dyn->e.x87stack;
    if(a) {
        ADDI(s2, s2, a);
        ANDI(s2, s2, 7);    // (emu->top + i)&7
    }
    if(rv64_zba) SH3ADD(s1, s2, xEmu); else {SLLI(s2, s2, 3); ADD(s1, xEmu, s2);}
    if (dyn->e.extcache[EXTIDX(reg)].t == EXT_CACHE_ST_F) {
        FCVTDS(SCRATCH0, reg);
        FSD(SCRATCH0, s1, offsetof(x64emu_t, x87));
    } else if (dyn->e.extcache[EXTIDX(reg)].t == EXT_CACHE_ST_I64) {
        FMVXD(s2, reg);
        FCVTDL(SCRATCH0, s2, RD_RTZ);
        FSD(SCRATCH0, s1, offsetof(x64emu_t, x87));
    } else {
        FSD(reg, s1, offsetof(x64emu_t, x87));
    }
    MESSAGE(LOG_DUMP, "\t--------x87 Cache for ST%d\n", st);
    // and forget that cache
    fpu_free_reg(dyn, dyn->e.x87reg[ret]);
    dyn->e.extcache[EXTIDX(reg)].v = 0;
    dyn->e.x87cache[ret] = -1;
    dyn->e.x87reg[ret] = -1;
}

void x87_reget_st(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int st)
{
    if(dyn->e.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    // search in cache first
    for (int i=0; i<8; ++i)
        if(dyn->e.x87cache[i]==st) {
            // refresh the value
            MESSAGE(LOG_DUMP, "\tRefresh x87 Cache for ST%d\n", st);
            #if STEP == 1
            if (dyn->e.extcache[EXTIDX(dyn->e.x87reg[i])].t == EXT_CACHE_ST_F
                || dyn->e.extcache[EXTIDX(dyn->e.x87reg[i])].t == EXT_CACHE_ST_I64)
                extcache_promote_double(dyn, ninst, st);
            #endif
            LW(s2, xEmu, offsetof(x64emu_t, top));
            int a = st - dyn->e.x87stack;
            if(a) {
                ADDI(s2, s2, a);
                AND(s2, s2, 7);
            }
            if(rv64_zba) SH3ADD(s1, s2, xEmu); else {SLLI(s2, s2, 3); ADD(s1, xEmu, s2);}
            FLD(dyn->e.x87reg[i], s1, offsetof(x64emu_t, x87));
            MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);
            // ok
            return;
        }
    // Was not in the cache? creating it....
    MESSAGE(LOG_DUMP, "\tCreate x87 Cache for ST%d\n", st);
    // get a free spot
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->e.x87cache[i]==-1)
            ret = i;
    // found, setup and grab the value
    dyn->e.x87cache[ret] = st;
    dyn->e.x87reg[ret] = fpu_get_reg_x87(dyn, EXT_CACHE_ST_D, st);
    LW(s2, xEmu, offsetof(x64emu_t, top));
    int a = st - dyn->e.x87stack;
    ADDI(s2, s2, a);
    ANDI(s2, s2, 7);    // (emu->top + i)&7
    if(rv64_zba) SH3ADD(s1, s2, xEmu); else {SLLI(s2, s2, 3); ADD(s1, xEmu, s2);}
    FLD(dyn->e.x87reg[ret], s1, offsetof(x64emu_t, x87));
    MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);
}

void x87_free(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int st)
{
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->e.x87cache[i] == st)
            ret = i;
    MESSAGE(LOG_DUMP, "\tFFREE%s x87 Cache for ST%d\n", (ret!=-1)?" (and Forget)":"", st);
    if(ret!=-1) {
        const int reg = dyn->e.x87reg[ret];
        #if STEP == 1
        if(dyn->e.extcache[reg].t==EXT_CACHE_ST_F || dyn->e.extcache[reg].t==EXT_CACHE_ST_I64)
            extcache_promote_double(dyn, ninst, st);
        #endif
        // Get top
        LW(s2, xEmu, offsetof(x64emu_t, top));
        // Update
        int ast = st - dyn->e.x87stack;
        if(ast) {
            if(ast>0) {
                ADDI(s2, s2, ast);
            } else {
                SUBI(s2, s2, -ast);
            }
            ANDI(s2, s2, 7); // (emu->top + i)&7
        }
        if(rv64_zba) SH3ADD(s1, s2, xEmu); else {SLLI(s2, s2, 3); ADD(s1, xEmu, s2);}
        if (dyn->e.extcache[EXTIDX(reg)].t == EXT_CACHE_ST_F) {
            FCVTDS(SCRATCH0, reg);
            FSD(SCRATCH0, s1, offsetof(x64emu_t, x87));
        } else if (dyn->e.extcache[EXTIDX(reg)].t == EXT_CACHE_ST_I64) {
            FMVXD(s2, reg);
            FCVTDL(SCRATCH0, s2, RD_RTZ);
            FSD(SCRATCH0, s1, offsetof(x64emu_t, x87));
        } else {
            FSD(reg, s1, offsetof(x64emu_t, x87));
        }
        // and forget that cache
        fpu_free_reg(dyn, reg);
        dyn->e.extcache[reg].v = 0;
        dyn->e.x87cache[ret] = -1;
        dyn->e.x87reg[ret] = -1;
    } else {
        // Get top
        LW(s2, xEmu, offsetof(x64emu_t, top));
        // Update
        int ast = st - dyn->e.x87stack;
        if(ast) {
            if(ast>0) {
                ADDI(s2, s2, ast);
            } else {
                SUBI(s2, s2, -ast);
            }
            ANDI(s2, s2, 7);    // (emu->top + i)&7
        }
    }
    // add mark in the freed array
    dyn->e.tags |= 0b11<<(st*2);
    MESSAGE(LOG_DUMP, "\t--------x87 FFREE for ST%d\n", st);
}

void x87_swapreg(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int a, int b)
{
    int i1, i2, i3;
    i1 = x87_get_cache(dyn, ninst, 1, s1, s2, b, X87_ST(b));
    i2 = x87_get_cache(dyn, ninst, 1, s1, s2, a, X87_ST(a));
    i3 = dyn->e.x87cache[i1];
    dyn->e.x87cache[i1] = dyn->e.x87cache[i2];
    dyn->e.x87cache[i2] = i3;
    // swap those too
    int j1, j2, j3;
    j1 = x87_get_extcache(dyn, ninst, s1, s2, b);
    j2 = x87_get_extcache(dyn, ninst, s1, s2, a);
    j3 = dyn->e.extcache[j1].n;
    dyn->e.extcache[j1].n = dyn->e.extcache[j2].n;
    dyn->e.extcache[j2].n = j3;
    // mark as swapped
    dyn->e.swapped = 1;
    dyn->e.combined1= a; dyn->e.combined2=b;
}

// Set rounding according to cw flags, return reg to restore flags
int x87_setround(dynarec_rv64_t* dyn, int ninst, int s1, int s2)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MAYUSE(s1); MAYUSE(s2);
    LW(s1, xEmu, offsetof(x64emu_t, cw));
    SRLI(s1, s1, 10);
    ANDI(s1, s1, 0b11);
    // MMX/x87 Round mode: 0..3: Nearest, Down, Up, Chop
    // RV64: 0..7: Nearest, Toward Zero (Chop), Down, Up, Nearest tie to Max, invalid, invalid, dynamic (invalid here)
    // 0->0, 1->2, 2->3, 3->1
    BEQ(s1, xZR, 24);
    ADDI(s2, xZR, 3);
    BEQ(s1, s2, 12);
    ADDI(s1, s1, 1);
    J(8);
    ADDI(s1, xZR, 1);
    // transform done (is there a faster way?)
    FSRM(s1, s1);               // exange RM with current
    return s1;
}

// Set rounding according to mxcsr flags, return reg to restore flags
int sse_setround(dynarec_rv64_t* dyn, int ninst, int s1, int s2)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MAYUSE(s1); MAYUSE(s2);
    LW(s1, xEmu, offsetof(x64emu_t, mxcsr));
    SRLI(s1, s1, 13);
    ANDI(s1, s1, 0b11);
    // MMX/x87 Round mode: 0..3: Nearest, Down, Up, Chop
    // RV64: 0..7: Nearest, Toward Zero (Chop), Down, Up, Nearest tie to Max, invalid, invalid, dynamic (invalid here)
    // 0->0, 1->2, 2->3, 3->1
    BEQ(s1, xZR, 24);
    ADDI(s2, xZR, 3);
    BEQ(s1, s2, 12);
    ADDI(s1, s1, 1);
    J(8);
    ADDI(s1, xZR, 1);
    // transform done (is there a faster way?)
    FSRM(s1, s1);               // exange RM with current
    return s1;
}

// Restore round flag, destroy s1 doing so
void x87_restoreround(dynarec_rv64_t* dyn, int ninst, int s1)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MAYUSE(s1);
    FSRM(s1, s1);               // put back fpscr
}

// MMX helpers
static int isx87Empty(dynarec_rv64_t* dyn)
{
    for (int i=0; i<8; ++i)
        if(dyn->e.x87cache[i] != -1)
            return 0;
    return 1;
}

// forget ext register for a MMX reg, does nothing if the regs is not loaded
void mmx_forget_reg(dynarec_rv64_t* dyn, int ninst, int a)
{
    if (dyn->e.mmxcache[a] == -1)
        return;
    FSD(dyn->e.mmxcache[a], xEmu, offsetof(x64emu_t, mmx[a]));
    fpu_free_reg(dyn, dyn->e.mmxcache[a]);
    return;
}

// get neon register for a MMX reg, create the entry if needed
int mmx_get_reg(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int a)
{
    if(!dyn->e.x87stack && isx87Empty(dyn))
        x87_purgecache(dyn, ninst, 0, s1, s2, s3);
    if(dyn->e.mmxcache[a]!=-1)
        return dyn->e.mmxcache[a];
    ++dyn->e.mmxcount;
    int ret = dyn->e.mmxcache[a] = fpu_get_reg_emm(dyn, a);
    FLD(ret, xEmu, offsetof(x64emu_t, mmx[a]));
    return ret;
}
// get neon register for a MMX reg, but don't try to synch it if it needed to be created
int mmx_get_reg_empty(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int a)
{
    if(!dyn->e.x87stack && isx87Empty(dyn))
        x87_purgecache(dyn, ninst, 0, s1, s2, s3);
    if(dyn->e.mmxcache[a]!=-1)
        return dyn->e.mmxcache[a];
    ++dyn->e.mmxcount;
    int ret = dyn->e.mmxcache[a] = fpu_get_reg_emm(dyn, a);
    return ret;
}
// purge the MMX cache only(needs 3 scratch registers)
void mmx_purgecache(dynarec_rv64_t* dyn, int ninst, int next, int s1)
{
    if(!dyn->e.mmxcount)
        return;
    if(!next)
        dyn->e.mmxcount = 0;
    int old = -1;
    for (int i=0; i<8; ++i)
        if(dyn->e.mmxcache[i]!=-1) {
            if (old==-1) {
                MESSAGE(LOG_DUMP, "\tPurge %sMMX Cache ------\n", next?"locally ":"");
                ++old;
            }
            FSD(dyn->e.mmxcache[i], xEmu, offsetof(x64emu_t, mmx[i]));
            if(!next) {
                fpu_free_reg(dyn, dyn->e.mmxcache[i]);
                dyn->e.mmxcache[i] = -1;
            }
        }
    if(old!=-1) {
        MESSAGE(LOG_DUMP, "\t------ Purge MMX Cache\n");
    }
}

static void mmx_reflectcache(dynarec_rv64_t* dyn, int ninst, int s1)
{
    for (int i=0; i<8; ++i)
        if(dyn->e.mmxcache[i]!=-1) {
            FLD(dyn->e.mmxcache[i], xEmu, offsetof(x64emu_t, mmx[i]));
        }
}

// SSE / SSE2 helpers
// get ext register for a SSE reg, create the entry if needed
int sse_get_reg(dynarec_rv64_t* dyn, int ninst, int s1, int a, int single)
{
    if(dyn->e.ssecache[a].v!=-1) {
        // forget / reload if change of size
        if(dyn->e.ssecache[a].single!=single) {
            sse_forget_reg(dyn, ninst, a);
            // update olds after the forget...
            dyn->e.olds[a].changed = 1;
            dyn->e.olds[a].purged = 0;
            dyn->e.olds[a].single = 1-single;
            return sse_get_reg(dyn, ninst, s1, a, single);
        }
        return dyn->e.ssecache[a].reg;
    }
    dyn->e.ssecache[a].reg = fpu_get_reg_xmm(dyn, single?EXT_CACHE_SS:EXT_CACHE_SD, a);
    int ret =  dyn->e.ssecache[a].reg;
    dyn->e.ssecache[a].single = single;
    if(dyn->e.ssecache[a].single)
        FLW(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
    else
        FLD(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
    return ret;
}
// get ext register for a SSE reg, but don't try to synch it if it needed to be created
int sse_get_reg_empty(dynarec_rv64_t* dyn, int ninst, int s1, int a, int single)
{
    if(dyn->e.ssecache[a].v!=-1) {
        if(dyn->e.ssecache[a].single!=single) {
            if (single) {
                // writing back the double
                FSD(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
                // need to wipe the half high 32bits of old Double because we now have a single
                //SW(xZR, xEmu, offsetof(x64emu_t, xmm[a])+4);
            }
            dyn->e.olds[a].changed = 1;
            dyn->e.olds[a].purged = 0;
            dyn->e.olds[a].reg = EXTIDX(dyn->e.ssecache[a].reg);
            dyn->e.olds[a].single = 1-single;
            dyn->e.ssecache[a].single = single;
            dyn->e.extcache[EXTIDX(dyn->e.ssecache[a].reg)].t = single?EXT_CACHE_SS:EXT_CACHE_SD;
        }
        return dyn->e.ssecache[a].reg;
    }
    dyn->e.ssecache[a].reg = fpu_get_reg_xmm(dyn, single?EXT_CACHE_SS:EXT_CACHE_SD, a);
    dyn->e.ssecache[a].single = single;
    return dyn->e.ssecache[a].reg;
}
// forget ext register for a SSE reg, does nothing if the regs is not loaded
void sse_forget_reg(dynarec_rv64_t* dyn, int ninst, int a)
{
    if(dyn->e.ssecache[a].v==-1)
        return;
    if(dyn->e.ssecache[a].single)
        FSW(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
    else
        FSD(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
    fpu_free_reg(dyn, dyn->e.ssecache[a].reg);
    dyn->e.olds[a].changed = 0;
    dyn->e.olds[a].purged = 1;
    dyn->e.olds[a].reg = dyn->e.ssecache[a].reg;
    dyn->e.olds[a].single = dyn->e.ssecache[a].single;
    dyn->e.ssecache[a].v = -1;
    return;
}
// purge the SSE cache for XMM0..XMM7 (to use before function native call)
void sse_purge07cache(dynarec_rv64_t* dyn, int ninst, int s1)
{
    int old = -1;
    for (int i=0; i<8; ++i)
        if(dyn->e.ssecache[i].v!=-1) {
            if (old==-1) {
                MESSAGE(LOG_DUMP, "\tPurge XMM0..7 Cache ------\n");
                ++old;
            }
            if(dyn->e.ssecache[i].single)
                FSW(dyn->e.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            else
                FSD(dyn->e.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            fpu_free_reg(dyn, dyn->e.ssecache[i].reg);
            dyn->e.ssecache[i].v = -1;
        }
    if(old!=-1) {
        MESSAGE(LOG_DUMP, "\t------ Purge XMM0..7 Cache\n");
    }
}

// purge the SSE cache only
static void sse_purgecache(dynarec_rv64_t* dyn, int ninst, int next, int s1)
{
    int old = -1;
    for (int i=0; i<16; ++i)
        if(dyn->e.ssecache[i].v!=-1) {
            if (old==-1) {
                MESSAGE(LOG_DUMP, "\tPurge %sSSE Cache ------\n", next?"locally ":"");
                ++old;
            }
            if(dyn->e.ssecache[i].single)
                FSW(dyn->e.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            else
                FSD(dyn->e.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            if(!next) {
                fpu_free_reg(dyn, dyn->e.ssecache[i].reg);
                dyn->e.olds[i].changed = 0;
                dyn->e.olds[i].purged = 1;
                dyn->e.olds[i].reg = dyn->e.ssecache[i].reg;
                dyn->e.olds[i].single = dyn->e.ssecache[i].single;
                dyn->e.ssecache[i].v = -1;
            }
        }
    if(old!=-1) {
        MESSAGE(LOG_DUMP, "\t------ Purge SSE Cache\n");
    }
}

static void sse_reflectcache(dynarec_rv64_t* dyn, int ninst, int s1)
{
    for (int i=0; i<16; ++i)
        if(dyn->e.ssecache[i].v!=-1) {
            if(dyn->e.ssecache[i].single)
                FSW(dyn->e.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            else
                FSD(dyn->e.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
        }
}

void sse_reflect_reg(dynarec_rv64_t* dyn, int ninst, int a)
{
    if (dyn->e.ssecache[a].v == -1)
        return;
    if (dyn->e.ssecache[a].single)
        FSW(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
    else
        FSD(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
}

void fpu_pushcache(dynarec_rv64_t* dyn, int ninst, int s1, int not07)
{
    // need to save 0..1 && 10..17 (maybe) && 28..31
    // so 0..7 (SSE) && 17..23 (x87+MMX)
    int start = not07?8:0;
    // only SSE regs needs to be push back to xEmu (needs to be "write")
    int n=0;
    for (int i=start; i<8; i++)
        if(dyn->e.ssecache[i].v!=-1)
            ++n;
    if(n) {
        MESSAGE(LOG_DUMP, "\tPush XMM Cache (%d)------\n", n);
        for (int i=start; i<8; ++i)
            if(dyn->e.ssecache[i].v!=-1) {
                if(dyn->e.ssecache[i].single)
                    FSW(dyn->e.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
                else
                    FSD(dyn->e.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            }
        MESSAGE(LOG_DUMP, "\t------- Push XMM Cache (%d)\n", n);
    }
    n = 0;
    for(int i=17; i<24; ++i)
        if(dyn->e.extcache[i].v!=0)
            ++n;
    if(n) {
        MESSAGE(LOG_DUMP, "\tPush x87/MMX Cache (%d)------\n", n);
        SUBI(xSP, xSP, 8*((n+1)&~1));
        int p = 0;
        for(int i=17; i<24; ++i)
            if(dyn->e.extcache[i].v!=0) {
                switch(dyn->e.extcache[i].t) {
                    case EXT_CACHE_ST_F:
                    case EXT_CACHE_SS:
                        FSW(EXTREG(i), xSP, p*8);
                        break;
                    default:
                        FSD(EXTREG(i), xSP, p*8);
                        break;
                };
                ++p;
            }
        MESSAGE(LOG_DUMP, "\t------- Push x87/MMX Cache (%d)\n", n);
    }
}
void fpu_popcache(dynarec_rv64_t* dyn, int ninst, int s1, int not07)
{
    int start = not07?8:0;
    // only SSE regs needs to be pop back from xEmu (don't need to be "write" this time)
    int n=0;
    for (int i=start; i<8; i++)
        if(dyn->e.ssecache[i].v!=-1)
            ++n;
    if(n) {
        MESSAGE(LOG_DUMP, "\tPop XMM Cache (%d)------\n", n);
        for (int i=start; i<8; ++i)
            if(dyn->e.ssecache[i].v!=-1) {
                if(dyn->e.ssecache[i].single)
                    FLW(dyn->e.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
                else
                    FLD(dyn->e.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            }
        MESSAGE(LOG_DUMP, "\t------- Pop XMM Cache (%d)\n", n);
    }
    n = 0;
    for(int i=17; i<24; ++i)
        if(dyn->e.extcache[i].v!=0)
            ++n;
    if(n) {
        MESSAGE(LOG_DUMP, "\tPush x87/MMX Cache (%d)------\n", n);
        int p = 0;
        for(int i=17; i<24; ++i)
            if(dyn->e.extcache[i].v!=0) {
                switch(dyn->e.extcache[i].t) {
                    case EXT_CACHE_ST_F:
                    case EXT_CACHE_SS:
                        FLW(EXTREG(i), xSP, p*8);
                        break;
                    default:
                        FLD(EXTREG(i), xSP, p*8);
                        break;
                };
                ++p;
            }
        ADDI(xSP, xSP, 8*((n+1)&~1));
        MESSAGE(LOG_DUMP, "\t------- Push x87/MMX Cache (%d)\n", n);
    }
}

void fpu_purgecache(dynarec_rv64_t* dyn, int ninst, int next, int s1, int s2, int s3)
{
    x87_purgecache(dyn, ninst, next, s1, s2, s3);
    mmx_purgecache(dyn, ninst, next, s1);
    sse_purgecache(dyn, ninst, next, s1);
    if(!next)
        fpu_reset_reg(dyn);
}

static int findCacheSlot(dynarec_rv64_t* dyn, int ninst, int t, int n, extcache_t* cache)
{
    ext_cache_t f;
    f.n = n;
    f.t = t;
    for (int i = 0; i < 24; ++i) {
        if (cache->extcache[i].v == f.v)
            return i;
        if (cache->extcache[i].n == n) {
            switch (cache->extcache[i].t) {
                case EXT_CACHE_ST_F:
                    if (t == EXT_CACHE_ST_D)
                        return i;
                    if (t == EXT_CACHE_ST_I64)
                        return i;
                    break;
                case EXT_CACHE_ST_D:
                    if (t == EXT_CACHE_ST_F)
                        return i;
                    if (t == EXT_CACHE_ST_I64)
                        return i;
                    break;
                case EXT_CACHE_ST_I64:
                    if (t == EXT_CACHE_ST_F)
                        return i;
                    if (t == EXT_CACHE_ST_D)
                        return i;
                    break;
            }
        }
    }
    return -1;
}

static void swapCache(dynarec_rv64_t* dyn, int ninst, int i, int j, extcache_t *cache)
{
    if (i==j)
        return;
    int reg_i = EXTREG(i);
    int reg_j = EXTREG(j);
    int i_single = 0;
    if(cache->extcache[i].t==EXT_CACHE_SS || cache->extcache[i].t==EXT_CACHE_ST_F)
        i_single =1;
    int j_single = 0;
    if(cache->extcache[j].t==EXT_CACHE_SS || cache->extcache[j].t==EXT_CACHE_ST_F)
        j_single =1;

    if(!cache->extcache[i].v) {
        // a mov is enough, no need to swap
        MESSAGE(LOG_DUMP, "\t  - Moving %d <- %d\n", i, j);
        if(j_single) {
            FMVS(reg_i, reg_j);
        } else {
            FMVD(reg_i, reg_j);
        }
        cache->extcache[i].v = cache->extcache[j].v;
        cache->extcache[j].v = 0;
        return;
    }
    // SWAP
    ext_cache_t tmp;
    MESSAGE(LOG_DUMP, "\t  - Swapping %d <-> %d\n", i, j);
    // There is no swap instruction in RV64 to swap 2 float registers!
    // so use a scratch...
    #define SCRATCH 2
    if(i_single)
        FMVS(SCRATCH, reg_i);
    else
        FMVD(SCRATCH, reg_i);
    if(j_single)
        FMVS(reg_i, reg_j);
    else
        FMVD(reg_i, reg_j);
    if(i_single)
        FMVS(reg_j, SCRATCH);
    else
        FMVD(reg_j, SCRATCH);
    #undef SCRATCH
    tmp.v = cache->extcache[i].v;
    cache->extcache[i].v = cache->extcache[j].v;
    cache->extcache[j].v = tmp.v;
}

static void loadCache(dynarec_rv64_t* dyn, int ninst, int stack_cnt, int s1, int s2, int s3, int* s1_val, int* s2_val, int* s3_top, extcache_t* cache, int i, int t, int n)
{
    int reg = EXTREG(i);
    if(cache->extcache[i].v) {
        int single = 0;
        if(t==EXT_CACHE_SS || t==EXT_CACHE_ST_F)
            single = 1;
        if(cache->extcache[i].t==EXT_CACHE_SS || cache->extcache[i].t==EXT_CACHE_ST_F)
            single = 1;
        int j = i+1;
        while(cache->extcache[j].v)
            ++j;
        MESSAGE(LOG_DUMP, "\t  - Moving away %d\n", i);
        if(single) {
            FMVS(EXTREG(j), reg);
        } else {
            FMVD(EXTREG(j), reg);
        }
        cache->extcache[j].v = cache->extcache[i].v;
    }
    switch(t) {
        case EXT_CACHE_SS:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            FLW(reg, xEmu, offsetof(x64emu_t, xmm[n]));
            break;
        case EXT_CACHE_SD:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            FLD(reg, xEmu, offsetof(x64emu_t, xmm[n]));
            break;
        case EXT_CACHE_MM:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            FLD(reg, xEmu, offsetof(x64emu_t, mmx[n]));
            break;
        case EXT_CACHE_ST_D:
        case EXT_CACHE_ST_F:
        case EXT_CACHE_ST_I64:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            if((*s3_top) == 0xffff) {
                LW(s3, xEmu, offsetof(x64emu_t, top));
                *s3_top = 0;
            }
            int a = n  - (*s3_top) - stack_cnt;
            if(a) {
                ADDI(s3, s3, a);
                ANDI(s3, s3, 7);    // (emu->top + i)&7
            }
            *s3_top += a;
            *s2_val = 0;
            if(rv64_zba) SH3ADD(s2, s3, xEmu); else {SLLI(s2, s3, 3); ADD(s2, xEmu, s2);}
            FLD(reg, s2, offsetof(x64emu_t, x87));
            if(t==EXT_CACHE_ST_F) {
                FCVTSD(reg, reg);
            }
            if (t == EXT_CACHE_ST_I64) {
                FCVTLD(s1, reg, RD_RTZ);
                FMVDX(reg, s1);
            }
            break;
        case EXT_CACHE_NONE:
        case EXT_CACHE_SCR:
        default:    /* nothing done */
            MESSAGE(LOG_DUMP, "\t  - ignoring %s\n", getCacheName(t, n));
            break;
    }
    cache->extcache[i].n = n;
    cache->extcache[i].t = t;
}

static void unloadCache(dynarec_rv64_t* dyn, int ninst, int stack_cnt, int s1, int s2, int s3, int* s1_val, int* s2_val, int* s3_top, extcache_t *cache, int i, int t, int n)
{
    int reg = EXTREG(i);
    switch(t) {
        case EXT_CACHE_SS:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            FSW(reg, xEmu, offsetof(x64emu_t, xmm[n]));
            break;
        case EXT_CACHE_SD:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            FSD(reg, xEmu, offsetof(x64emu_t, xmm[n]));
            break;
        case EXT_CACHE_MM:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            FSD(reg, xEmu, offsetof(x64emu_t, mmx[n]));
            break;
        case EXT_CACHE_ST_D:
        case EXT_CACHE_ST_F:
        case EXT_CACHE_ST_I64:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            if((*s3_top)==0xffff) {
                LW(s3, xEmu, offsetof(x64emu_t, top));
                *s3_top = 0;
            }
            int a = n - (*s3_top) - stack_cnt;
            if(a) {
                ADDI(s3, s3, a);
                ANDI(s3, s3, 7);
            }
            *s3_top += a;
            if(rv64_zba) SH3ADD(s2, s3, xEmu); else {SLLI(s2, s3, 3); ADD(s2, xEmu, s2);}
            *s2_val = 0;
            if (t == EXT_CACHE_ST_F) {
                FCVTDS(reg, reg);
            }
            if (t == EXT_CACHE_ST_I64) {
                FMVXD(s1, reg);
                FCVTDL(reg, s1, RD_RTZ);
            }
            FSD(reg, s2, offsetof(x64emu_t, x87));
            break;
        case EXT_CACHE_NONE:
        case EXT_CACHE_SCR:
        default:    /* nothing done */
            MESSAGE(LOG_DUMP, "\t  - ignoring %s\n", getCacheName(t, n));
            break;
    }
    cache->extcache[i].v = 0;
}

static void fpuCacheTransform(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3)
{
#if STEP > 1
    int i2 = dyn->insts[ninst].x64.jmp_insts;
    if(i2<0)
        return;
    MESSAGE(LOG_DUMP, "\tCache Transform ---- ninst=%d -> %d\n", ninst, i2);
    if((!i2) || (dyn->insts[i2].x64.barrier&BARRIER_FLOAT)) {
        if(dyn->e.stack_next)  {
            fpu_purgecache(dyn, ninst, 1, s1, s2, s3);
            MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
            return;
        }
        for(int i=0; i<24; ++i)
            if(dyn->e.extcache[i].v) {       // there is something at ninst for i
                fpu_purgecache(dyn, ninst, 1, s1, s2, s3);
                MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
                return;
            }
        MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
        return;
    }
    extcache_t cache_i2 = dyn->insts[i2].e;
    extcacheUnwind(&cache_i2);

    if(!cache_i2.stack) {
        int purge = 1;
        for (int i=0; i<24 && purge; ++i)
            if(cache_i2.extcache[i].v)
                purge = 0;
        if(purge) {
            fpu_purgecache(dyn, ninst, 1, s1, s2, s3);
            MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
            return;
        }
    }
    int stack_cnt = dyn->e.stack_next;
    int s3_top = 0xffff;
    extcache_t cache = dyn->e;
    int s1_val = 0;
    int s2_val = 0;
    // unload every uneeded cache
    // check SSE first, than MMX, in order, for optimisation issue
    for(int i=0; i<16; ++i) {
        int j=findCacheSlot(dyn, ninst, EXT_CACHE_SS, i, &cache);
        if(j>=0 && findCacheSlot(dyn, ninst, EXT_CACHE_SS, i, &cache_i2)==-1)
            unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, j, cache.extcache[j].t, cache.extcache[j].n);
        j=findCacheSlot(dyn, ninst, EXT_CACHE_SD, i, &cache);
        if(j>=0 && findCacheSlot(dyn, ninst, EXT_CACHE_SD, i, &cache_i2)==-1)
            unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, j, cache.extcache[j].t, cache.extcache[j].n);
    }
    for(int i=0; i<8; ++i) {
        int j=findCacheSlot(dyn, ninst, EXT_CACHE_MM, i, &cache);
        if(j>=0 && findCacheSlot(dyn, ninst, EXT_CACHE_MM, i, &cache_i2)==-1)
            unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, j, cache.extcache[j].t, cache.extcache[j].n);
    }
    for(int i=0; i<24; ++i) {
        if(cache.extcache[i].v)
            if(findCacheSlot(dyn, ninst, cache.extcache[i].t, cache.extcache[i].n, &cache_i2)==-1)
                unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, i, cache.extcache[i].t, cache.extcache[i].n);
    }
    // and now load/swap the missing one
    for(int i=0; i<24; ++i) {
        if(cache_i2.extcache[i].v) {
            if(cache_i2.extcache[i].v != cache.extcache[i].v) {
                int j;
                if((j=findCacheSlot(dyn, ninst, cache_i2.extcache[i].t, cache_i2.extcache[i].n, &cache))==-1)
                    loadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, i, cache_i2.extcache[i].t, cache_i2.extcache[i].n);
                else {
                    // it's here, lets swap if needed
                    if(j!=i)
                        swapCache(dyn, ninst, i, j, &cache);
                }
            }
            if(cache.extcache[i].t != cache_i2.extcache[i].t) {
                if(cache.extcache[i].t == EXT_CACHE_ST_D && cache_i2.extcache[i].t == EXT_CACHE_ST_F) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.extcache[i].t, cache.extcache[i].n));
                    FCVTSD(EXTREG(i), EXTREG(i));
                    cache.extcache[i].t = EXT_CACHE_ST_F;
                } else if(cache.extcache[i].t == EXT_CACHE_ST_F && cache_i2.extcache[i].t == EXT_CACHE_ST_D) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.extcache[i].t, cache.extcache[i].n));
                    FCVTDS(EXTREG(i), EXTREG(i));
                    cache.extcache[i].t = EXT_CACHE_ST_D;
                } else if (cache.extcache[i].t == EXT_CACHE_ST_D && cache_i2.extcache[i].t == EXT_CACHE_ST_I64) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.extcache[i].t, cache.extcache[i].n));
                    FCVTLD(s1, EXTREG(i), RD_RTZ);
                    FMVDX(EXTREG(i), s1);
                    cache.extcache[i].t = EXT_CACHE_ST_I64;
                } else if (cache.extcache[i].t == EXT_CACHE_ST_F && cache_i2.extcache[i].t == EXT_CACHE_ST_I64) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.extcache[i].t, cache.extcache[i].n));
                    FCVTLS(s1, EXTREG(i), RD_RTZ);
                    FMVDX(EXTREG(i), s1);
                    cache.extcache[i].t = EXT_CACHE_ST_D;
                } else if (cache.extcache[i].t == EXT_CACHE_ST_I64 && cache_i2.extcache[i].t == EXT_CACHE_ST_F) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.extcache[i].t, cache.extcache[i].n));
                    FMVXD(s1, EXTREG(i));
                    FCVTSL(EXTREG(i), s1, RD_RTZ);
                    cache.extcache[i].t = EXT_CACHE_ST_F;
                } else if (cache.extcache[i].t == EXT_CACHE_ST_I64 && cache_i2.extcache[i].t == EXT_CACHE_ST_D) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.extcache[i].t, cache.extcache[i].n));
                    FMVXD(s1, EXTREG(i));
                    FCVTDL(EXTREG(i), s1, RD_RTZ);
                    cache.extcache[i].t = EXT_CACHE_ST_D;
                }
            }
        }
    }
    if(stack_cnt != cache_i2.stack) {
        MESSAGE(LOG_DUMP, "\t    - adjust stack count %d -> %d -\n", stack_cnt, cache_i2.stack);
        int a = stack_cnt - cache_i2.stack;
        // Add x87stack to emu fpu_stack
        LWU(s3, xEmu, offsetof(x64emu_t, fpu_stack));
        ADDI(s3, s3, a);
        SW(s3, xEmu, offsetof(x64emu_t, fpu_stack));
        // Sub x87stack to top, with and 7
        LWU(s3, xEmu, offsetof(x64emu_t, top));
        SUBI(s3, s3, a);
        ANDI(s3, s3, 7);
        SW(s3, xEmu, offsetof(x64emu_t, top));
        // update tags
        LH(s2, xEmu, offsetof(x64emu_t, fpu_tags));
        if(a>0) {
            SLLI(s2, s2, a*2);
        } else {
            SLLI(s3, xMASK, 16);    // 0xffff0000
            OR(s2, s2, s3);
            SRLI(s2, s2, -a*2);
        }
        SH(s2, xEmu, offsetof(x64emu_t, fpu_tags));
        s3_top = 0;
        stack_cnt = cache_i2.stack;
    }
    MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
#endif
}
static void flagsCacheTransform(dynarec_rv64_t* dyn, int ninst, int s1)
{
#if STEP > 1
    int j64;
    int jmp = dyn->insts[ninst].x64.jmp_insts;
    if(jmp<0)
        return;
    if(dyn->f.dfnone)  // flags are fully known, nothing we can do more
        return;
    MESSAGE(LOG_DUMP, "\tFlags fetch ---- ninst=%d -> %d\n", ninst, jmp);
    int go = 0;
    switch (dyn->insts[jmp].f_entry.pending) {
        case SF_UNKNOWN: break;
        case SF_SET:
            if(dyn->f.pending!=SF_SET && dyn->f.pending!=SF_SET_PENDING)
                go = 1;
            break;
        case SF_SET_PENDING:
            if(dyn->f.pending!=SF_SET
            && dyn->f.pending!=SF_SET_PENDING
            && dyn->f.pending!=SF_PENDING)
                go = 1;
            break;
        case SF_PENDING:
            if(dyn->f.pending!=SF_SET
            && dyn->f.pending!=SF_SET_PENDING
            && dyn->f.pending!=SF_PENDING)
                go = 1;
            else
                go = (dyn->insts[jmp].f_entry.dfnone  == dyn->f.dfnone)?0:1;
            break;
    }
    if(dyn->insts[jmp].f_entry.dfnone && !dyn->f.dfnone)
        go = 1;
    if(go) {
        if(dyn->f.pending!=SF_PENDING) {
            LW(s1, xEmu, offsetof(x64emu_t, df));
            j64 = (GETMARKF2)-(dyn->native_size);
            BEQZ(s1, j64);
        }
        CALL_(UpdateFlags, -1, 0);
        MARKF2;
    }
#endif
}

void CacheTransform(dynarec_rv64_t* dyn, int ninst, int cacheupd, int s1, int s2, int s3) {
    if(cacheupd&2)
        fpuCacheTransform(dyn, ninst, s1, s2, s3);
    if(cacheupd&1)
        flagsCacheTransform(dyn, ninst, s1);
}


void rv64_move32(dynarec_rv64_t* dyn, int ninst, int reg, int32_t val, int zeroup)
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
    if (lo12 || !hi20) ADDIW(reg, src, lo12);
    if((zeroup && ((hi20&0x80000) || (!hi20 && (lo12&0x800)))
    || (!zeroup && !(val&0x80000000) && ((hi20&0x80000) || (!hi20 && (lo12&0x800)))))) {
        ZEROUP(reg);
    }
}

void rv64_move64(dynarec_rv64_t* dyn, int ninst, int reg, int64_t val)
{
    if(((val<<32)>>32)==val) {
        // 32bits value
        rv64_move32(dyn, ninst, reg, val, 0);
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

void fpu_reflectcache(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3)
{
    x87_reflectcache(dyn, ninst, s1, s2, s3);
    mmx_reflectcache(dyn, ninst, s1);
    sse_reflectcache(dyn, ninst, s1);
}

void fpu_unreflectcache(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3)
{
    // need to undo the top and stack tracking that must not be reflected permenatly yet
    x87_unreflectcache(dyn, ninst, s1, s2, s3);
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

    BEQZ(s4, 8);
    ORI(xFlags, xFlags, 1 << F_PF);
}

void fpu_reset_cache(dynarec_rv64_t* dyn, int ninst, int reset_n)
{
    MESSAGE(LOG_DEBUG, "Reset Caches with %d\n",reset_n);
    #if STEP > 1
    // for STEP 2 & 3, just need to refrest with current, and undo the changes (push & swap)
    dyn->e = dyn->insts[ninst].e;
    #else
    dyn->e = dyn->insts[reset_n].e;
    #endif
    extcacheUnwind(&dyn->e);
    #if STEP == 0
    if(box64_dynarec_dump) dynarec_log(LOG_NONE, "New x87stack=%d\n", dyn->e.x87stack);
    #endif
    #if defined(HAVE_TRACE) && (STEP>2)
    if(box64_dynarec_dump)
        if(memcmp(&dyn->e, &dyn->insts[reset_n].e, sizeof(ext_cache_t))) {
            MESSAGE(LOG_DEBUG, "Warning, difference in extcache: reset=");
            for(int i=0; i<24; ++i)
                if(dyn->insts[reset_n].e.extcache[i].v)
                    MESSAGE(LOG_DEBUG, " %02d:%s", i, getCacheName(dyn->insts[reset_n].e.extcache[i].t, dyn->insts[reset_n].e.extcache[i].n));
            if(dyn->insts[reset_n].e.combined1 || dyn->insts[reset_n].e.combined2)
                MESSAGE(LOG_DEBUG, " %s:%02d/%02d", dyn->insts[reset_n].e.swapped?"SWP":"CMB", dyn->insts[reset_n].e.combined1, dyn->insts[reset_n].e.combined2);
            if(dyn->insts[reset_n].e.stack_push || dyn->insts[reset_n].e.stack_pop)
                MESSAGE(LOG_DEBUG, " (%d:%d)", dyn->insts[reset_n].e.stack_push, -dyn->insts[reset_n].e.stack_pop);
            MESSAGE(LOG_DEBUG, " ==> ");
            for(int i=0; i<24; ++i)
                if(dyn->insts[ninst].e.extcache[i].v)
                    MESSAGE(LOG_DEBUG, " %02d:%s", i, getCacheName(dyn->insts[ninst].e.extcache[i].t, dyn->insts[ninst].e.extcache[i].n));
            if(dyn->insts[ninst].e.combined1 || dyn->insts[ninst].e.combined2)
                MESSAGE(LOG_DEBUG, " %s:%02d/%02d", dyn->insts[ninst].e.swapped?"SWP":"CMB", dyn->insts[ninst].e.combined1, dyn->insts[ninst].e.combined2);
            if(dyn->insts[ninst].e.stack_push || dyn->insts[ninst].e.stack_pop)
                MESSAGE(LOG_DEBUG, " (%d:%d)", dyn->insts[ninst].e.stack_push, -dyn->insts[ninst].e.stack_pop);
            MESSAGE(LOG_DEBUG, " -> ");
            for(int i=0; i<24; ++i)
                if(dyn->e.extcache[i].v)
                    MESSAGE(LOG_DEBUG, " %02d:%s", i, getCacheName(dyn->e.extcache[i].t, dyn->e.extcache[i].n));
            if(dyn->e.combined1 || dyn->e.combined2)
                MESSAGE(LOG_DEBUG, " %s:%02d/%02d", dyn->e.swapped?"SWP":"CMB", dyn->e.combined1, dyn->e.combined2);
            if(dyn->e.stack_push || dyn->e.stack_pop)
                MESSAGE(LOG_DEBUG, " (%d:%d)", dyn->e.stack_push, -dyn->e.stack_pop);
            MESSAGE(LOG_DEBUG, "\n");
        }
    #endif //HAVE_TRACE
}

// propagate ST stack state, especial stack pop that are deferred
void fpu_propagate_stack(dynarec_rv64_t* dyn, int ninst)
{
    if(dyn->e.stack_pop) {
        for(int j=0; j<24; ++j)
            if ((dyn->e.extcache[j].t == EXT_CACHE_ST_D
                    || dyn->e.extcache[j].t == EXT_CACHE_ST_F
                    || dyn->e.extcache[j].t == EXT_CACHE_ST_I64)) {
                if(dyn->e.extcache[j].n<dyn->e.stack_pop)
                    dyn->e.extcache[j].v = 0;
                else
                    dyn->e.extcache[j].n-=dyn->e.stack_pop;
            }
        dyn->e.stack_pop = 0;
    }
    dyn->e.stack = dyn->e.stack_next;
    dyn->e.news = 0;
    dyn->e.stack_push = 0;
    dyn->e.swapped = 0;
}
