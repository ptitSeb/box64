#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

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

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "dynarec_la64_helper.h"

static uintptr_t geted_32(dynarec_la64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, int* l, int i12);

/* setup r2 to address pointed by ED, also fixaddress is an optionnal delta in the range [-absmax, +absmax], with delta&mask==0 to be added to ed for LDR/STR */
uintptr_t geted(dynarec_la64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, rex_t rex, int* l, int i12, int delta)
{
    MAYUSE(dyn);
    MAYUSE(ninst);
    MAYUSE(delta);

    if (rex.is32bits)
        return geted_32(dyn, addr, ninst, nextop, ed, hint, scratch, fixaddress, l, i12);

    int lock = l ? ((l == LOCK_LOCK) ? 1 : 2) : 0;
    if (lock == 2)
        *l = 0;
    uint8_t ret = x2;
    *fixaddress = 0;
    if (hint > 0) ret = hint;
    int maxval = 2047;
    if (i12 > 1)
        maxval -= i12;
    MAYUSE(scratch);
    if (!(nextop & 0xC0)) {
        if ((nextop & 7) == 4) {
            uint8_t sib = F8;
            int sib_reg = ((sib >> 3) & 7) + (rex.x << 3);
            int sib_reg2 = (sib & 0x7) + (rex.b << 3);
            if ((sib & 0x7) == 5) {
                int64_t tmp = F32S;
                if (sib_reg != 4) {
                    if (tmp && ((tmp < -2048) || (tmp > maxval) || !i12)) {
                        MOV64x(scratch, tmp);
                        ADDSL(ret, scratch, TO_LA64(sib_reg), sib >> 6, ret);
                    } else {
                        if (sib >> 6) {
                            SLLI_D(ret, TO_LA64(sib_reg), (sib >> 6));
                        } else {
                            ret = TO_LA64(sib_reg);
                        }
                        *fixaddress = tmp;
                    }
                } else {
                    switch (lock) {
                        case 1: addLockAddress(tmp); break;
                        case 2:
                            if (isLockAddress(tmp)) *l = 1;
                            break;
                    }
                    MOV64x(ret, tmp);
                }
            } else {
                if (sib_reg != 4) {
                    ADDSL(ret, TO_LA64(sib_reg2), TO_LA64(sib_reg), sib >> 6, scratch);
                } else {
                    ret = TO_LA64(sib_reg2);
                }
            }
        } else if ((nextop & 7) == 5) {
            int64_t tmp = F32S64;
            int64_t adj = dyn->last_ip ? ((addr + delta) - dyn->last_ip) : 0;
            if (i12 && adj && (tmp + adj >= -2048) && (tmp + adj <= maxval)) {
                ret = xRIP;
                *fixaddress = tmp + adj;
            } else if (i12 && (tmp >= -2048) && (tmp <= maxval)) {
                GETIP(addr + delta);
                ret = xRIP;
                *fixaddress = tmp;
            } else if (adj && (tmp + adj >= -2048) && (tmp + adj <= maxval)) {
                ADDI_D(ret, xRIP, tmp + adj);
            } else if ((tmp >= -2048) && (tmp <= maxval)) {
                GETIP(addr + delta);
                ADDI_D(ret, xRIP, tmp);
            } else if (tmp + addr + delta < 0x100000000LL) {
                MOV64x(ret, tmp + addr + delta);
            } else {
                if (adj) {
                    MOV64x(ret, tmp + adj);
                } else {
                    MOV64x(ret, tmp);
                    GETIP(addr + delta);
                }
                ADD_D(ret, ret, xRIP);
            }
            switch (lock) {
                case 1: addLockAddress(addr + delta + tmp); break;
                case 2:
                    if (isLockAddress(addr + delta + tmp)) *l = 1;
                    break;
            }
        } else {
            ret = TO_LA64((nextop & 7) + (rex.b << 3));
        }
    } else {
        int64_t i64;
        uint8_t sib = 0;
        int sib_reg = 0;
        if ((nextop & 7) == 4) {
            sib = F8;
            sib_reg = ((sib >> 3) & 7) + (rex.x << 3);
        }
        int sib_reg2 = (sib & 0x07) + (rex.b << 3);
        if (nextop & 0x80)
            i64 = F32S;
        else
            i64 = F8S;
        if (i64 == 0 || ((i64 >= -2048) && (i64 <= 2047) && i12)) {
            *fixaddress = i64;
            if ((nextop & 7) == 4) {
                if (sib_reg != 4) {
                    ADDSL(ret, TO_LA64(sib_reg2), TO_LA64(sib_reg), sib >> 6, scratch);
                } else {
                    ret = TO_LA64(sib_reg2);
                }
            } else {
                ret = TO_LA64((nextop & 0x07) + (rex.b << 3));
            }
        } else {
            if (i64 >= -2048 && i64 <= 2047) {
                if ((nextop & 7) == 4) {
                    if (sib_reg != 4) {
                        ADDSL(scratch, TO_LA64(sib_reg2), TO_LA64(sib_reg), sib >> 6, scratch);
                    } else {
                        scratch = TO_LA64(sib_reg2);
                    }
                } else {
                    scratch = TO_LA64((nextop & 0x07) + (rex.b << 3));
                }
                ADDI_D(ret, scratch, i64);
            } else {
                MOV64x(scratch, i64);
                if ((nextop & 7) == 4) {
                    if (sib_reg != 4) {
                        ADD_D(scratch, scratch, TO_LA64(sib_reg2));
                        ADDSL(ret, scratch, TO_LA64(sib_reg), sib >> 6, ret);
                    } else {
                        PASS3(int tmp = TO_LA64(sib_reg2));
                        ADD_D(ret, tmp, scratch);
                    }
                } else {
                    PASS3(int tmp = TO_LA64((nextop & 0x07) + (rex.b << 3)));
                    ADD_D(ret, tmp, scratch);
                }
            }
        }
    }
    *ed = ret;
    return addr;
}

static uintptr_t geted_32(dynarec_la64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, int* l, int i12)
{
    MAYUSE(dyn);
    MAYUSE(ninst);

    int lock = l ? ((l == LOCK_LOCK) ? 1 : 2) : 0;
    if (lock == 2)
        *l = 0;
    uint8_t ret = x2;
    *fixaddress = 0;
    if (hint > 0) ret = hint;
    int maxval = 2047;
    if (i12 > 1)
        maxval -= i12;
    MAYUSE(scratch);
    if (!(nextop & 0xC0)) {
        if ((nextop & 7) == 4) {
            uint8_t sib = F8;
            int sib_reg = (sib >> 3) & 0x7;
            int sib_reg2 = sib & 0x7;
            if (sib_reg2 == 5) {
                int64_t tmp = F32S;
                if (sib_reg != 4) {
                    if (tmp && ((tmp < -2048) || (tmp > maxval) || !i12)) {
                        MOV32w(scratch, tmp);
                        if ((sib >> 6)) {
                            SLLI_D(ret, TO_LA64(sib_reg), sib >> 6);
                            ADD_W(ret, ret, scratch);
                        } else {
                            ADD_W(ret, TO_LA64(sib_reg), scratch);
                        }
                    } else {
                        if (sib >> 6) {
                            SLLI_D(ret, TO_LA64(sib_reg), (sib >> 6));
                        } else {
                            ret = TO_LA64(sib_reg);
                        }
                        *fixaddress = tmp;
                    }
                } else {
                    switch (lock) {
                        case 1: addLockAddress((int32_t)tmp); break;
                        case 2:
                            if (isLockAddress((int32_t)tmp)) *l = 1;
                            break;
                    }
                    MOV32w(ret, tmp);
                }
            } else {
                if (sib_reg != 4) {
                    if ((sib >> 6)) {
                        SLLI_D(ret, TO_LA64(sib_reg), (sib >> 6));
                        ADD_W(ret, ret, TO_LA64(sib_reg2));
                    } else {
                        ADD_W(ret, TO_LA64(sib_reg2), TO_LA64(sib_reg));
                    }
                } else {
                    ret = TO_LA64(sib_reg2);
                }
            }
        } else if ((nextop & 7) == 5) {
            uint32_t tmp = F32;
            MOV32w(ret, tmp);
            switch (lock) {
                case 1: addLockAddress(tmp); break;
                case 2:
                    if (isLockAddress(tmp)) *l = 1;
                    break;
            }
        } else {
            ret = TO_LA64((nextop & 7));
            if (ret == hint) {
                MOV32w(x2, 0xffffffff);
                AND(hint, ret, x2); // to clear upper part
            }
        }
    } else {
        int64_t i32;
        uint8_t sib = 0;
        int sib_reg = 0;
        if ((nextop & 7) == 4) {
            sib = F8;
            sib_reg = (sib >> 3) & 7;
        }
        int sib_reg2 = sib & 0x07;
        if (nextop & 0x80)
            i32 = F32S;
        else
            i32 = F8S;
        if (i32 == 0 || ((i32 >= -2048) && (i32 <= 2047) && i12)) {
            *fixaddress = i32;
            if ((nextop & 7) == 4) {
                if (sib_reg != 4) {
                    if (sib >> 6) {
                        SLLI_D(ret, TO_LA64(sib_reg), (sib >> 6));
                        ADD_W(ret, ret, TO_LA64(sib_reg2));
                    } else {
                        ADD_W(ret, TO_LA64(sib_reg2), TO_LA64(sib_reg));
                    }
                } else {
                    ret = TO_LA64(sib_reg2);
                }
            } else {
                ret = TO_LA64((nextop & 0x07));
            }
        } else {
            if (i32 >= -2048 && i32 <= 2047) {
                if ((nextop & 7) == 4) {
                    if (sib_reg != 4) {
                        if (sib >> 6) {
                            SLLI_D(scratch, TO_LA64(sib_reg), sib >> 6);
                            ADD_W(scratch, scratch, TO_LA64(sib_reg2));
                        } else {
                            ADD_W(scratch, TO_LA64(sib_reg2), TO_LA64(sib_reg));
                        }
                    } else {
                        scratch = TO_LA64(sib_reg2);
                    }
                } else {
                    scratch = TO_LA64((nextop & 0x07));
                }
                ADDI_W(ret, scratch, i32);
            } else {
                MOV32w(scratch, i32);
                if ((nextop & 7) == 4) {
                    if (sib_reg != 4) {
                        ADD_W(scratch, scratch, TO_LA64(sib_reg2));
                        if (sib >> 6) {
                            SLLI_D(ret, TO_LA64(sib_reg), (sib >> 6));
                            ADD_W(ret, ret, scratch);
                        } else {
                            ADD_W(ret, scratch, TO_LA64(sib_reg));
                        }
                    } else {
                        PASS3(int tmp = TO_LA64(sib_reg2));
                        ADD_W(ret, tmp, scratch);
                    }
                } else {
                    PASS3(int tmp = TO_LA64((nextop & 0x07)));
                    ADD_W(ret, tmp, scratch);
                }
            }
        }
    }
    *ed = ret;
    return addr;
}

void jump_to_epilog(dynarec_la64_t* dyn, uintptr_t ip, int reg, int ninst)
{
    MAYUSE(dyn);
    MAYUSE(ip);
    MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Jump to epilog\n");

    if (reg) {
        if (reg != xRIP) {
            MV(xRIP, reg);
        }
    } else {
        GETIP_(ip);
    }
    TABLE64(x2, (uintptr_t)la64_epilog);
    SMEND();
    BR(x2);
}

void jump_to_next(dynarec_la64_t* dyn, uintptr_t ip, int reg, int ninst, int is32bits)
{
    MAYUSE(dyn);
    MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Jump to next\n");

    if (reg) {
        if (reg != xRIP) {
            MV(xRIP, reg);
        }
        uintptr_t tbl = is32bits ? getJumpTable32() : getJumpTable64();
        MAYUSE(tbl);
        TABLE64(x3, tbl);
        if (!is32bits) {
            SRLI_D(x2, xRIP, JMPTABL_START3);
            ALSL_D(x3, x2, x3, 2);
            LD_D(x3, x3, 0); // could be LR_D(x3, x3, 1, 1); for better safety
        }
        MOV64x(x4, JMPTABLE_MASK2 << 3); // x4 = mask
        SRLI_D(x2, xRIP, JMPTABL_START2 - 3);
        AND(x2, x2, x4);
        ADD_D(x3, x3, x2);
        LD_D(x3, x3, 0); // LR_D(x3, x3, 1, 1);
        if (JMPTABLE_MASK2 != JMPTABLE_MASK1) {
            MOV64x(x4, JMPTABLE_MASK1 << 3); // x4 = mask
        }
        SRLI_D(x2, xRIP, JMPTABL_START1 - 3);
        AND(x2, x2, x4);
        ADD_D(x3, x3, x2);
        LD_D(x3, x3, 0); // LR_D(x3, x3, 1, 1);
        if (JMPTABLE_MASK0 < 2048) {
            ANDI(x2, xRIP, JMPTABLE_MASK0);
        } else {
            if (JMPTABLE_MASK1 != JMPTABLE_MASK0) {
                MOV64x(x4, JMPTABLE_MASK0); // x4 = mask
            }
            AND(x2, xRIP, x4);
        }
        ALSL_D(x3, x2, x3, 2);
        LD_D(x2, x3, 0); // LR_D(x2, x3, 1, 1);
    } else {
        uintptr_t p = getJumpTableAddress64(ip);
        MAYUSE(p);
        TABLE64(x3, p);
        GETIP_(ip);
        LD_D(x2, x3, 0); // LR_D(x2, x3, 1, 1);
    }
    if (reg != x1) {
        MV(x1, xRIP);
    }
    CLEARIP();
#ifdef HAVE_TRACE
// MOVx(x3, 15);    no access to PC reg
#endif
    SMEND();
    JIRL(xRA, x2, 0x0); // save LR...
}

void call_c(dynarec_la64_t* dyn, int ninst, void* fnc, int reg, int ret, int saveflags, int savereg)
{
    MAYUSE(fnc);
    if (savereg == 0)
        savereg = x6;
    if (saveflags) {
        FLAGS_ADJUST_TO11(xFlags, xFlags, reg);
        ST_D(xFlags, xEmu, offsetof(x64emu_t, eflags));
    }
    fpu_pushcache(dyn, ninst, reg, 0);
    if (ret != -2) {
        ADDI_D(xSP, xSP, -16); // RV64 stack needs to be 16byte aligned
        ST_D(xEmu, xSP, 0);
        ST_D(savereg, xSP, 8);
        // x5..x8, x10..x17, x28..x31 those needs to be saved by caller
        STORE_REG(RAX);
        STORE_REG(RCX);
        STORE_REG(RDX);
        STORE_REG(R12);
        STORE_REG(R13);
        STORE_REG(R14);
        STORE_REG(R15);
        ST_D(xRIP, xEmu, offsetof(x64emu_t, ip));
    }
    TABLE64(reg, (uintptr_t)fnc);
    JIRL(xRA, reg, 0);
    if (ret >= 0) {
        MV(ret, xEmu);
    }
    if (ret != -2) {
        LD_D(xEmu, xSP, 0);
        LD_D(savereg, xSP, 8);
        ADDI_D(xSP, xSP, 16);
#define GO(A) \
    if (ret != x##A) { LOAD_REG(A); }
        GO(RAX);
        GO(RCX);
        GO(RDX);
        GO(R12);
        GO(R13);
        GO(R14);
        GO(R15);
        if (ret != xRIP)
            LD_D(xRIP, xEmu, offsetof(x64emu_t, ip));
#undef GO
    }

    fpu_popcache(dyn, ninst, reg, 0);
    if (saveflags) {
        LD_D(xFlags, xEmu, offsetof(x64emu_t, eflags));
        FLAGS_ADJUST_FROM11(xFlags, xFlags, reg);
    }
    SET_NODF();
    dyn->last_ip = 0;
}

void fpu_pushcache(dynarec_la64_t* dyn, int ninst, int s1, int not07)
{
    // TODO
}

void fpu_popcache(dynarec_la64_t* dyn, int ninst, int s1, int not07)
{
    // TODO
}

void fpu_purgecache(dynarec_la64_t* dyn, int ninst, int next, int s1, int s2, int s3)
{
    // TODO
}

void fpu_reflectcache(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3)
{
    // TODO
}

void fpu_unreflectcache(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3)
{
    // TODO
}

void emit_pf(dynarec_la64_t* dyn, int ninst, int s1, int s3, int s4)
{
    MAYUSE(dyn);
    MAYUSE(ninst);
    // PF: (((emu->x64emu_parity_tab[(res&0xff) / 32] >> ((res&0xff) % 32)) & 1) == 0)
    MOV64x(s4, (uintptr_t)GetParityTab());
    SRLI_D(s3, s1, 3);
    ANDI(s3, s3, 28);
    ADD_D(s4, s4, s3);
    LD_W(s4, s4, 0);
    NOT(s4, s4);
    SRL_W(s4, s4, s1);
    ANDI(s4, s4, 1);

    BEQZ(s4, 8);
    ORI(xFlags, xFlags, 1 << F_PF);
}

void fpu_reset_cache(dynarec_la64_t* dyn, int ninst, int reset_n)
{
    // TODO
}

// propagate ST stack state, especial stack pop that are deferred
void fpu_propagate_stack(dynarec_la64_t* dyn, int ninst)
{
    // TODO
}
