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
#include "la64_emitter.h"
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
#include "../dynarec_helper.h"

#define SCRATCH 31

static uintptr_t geted_32(dynarec_la64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, int* l, int i12);

/* setup r2 to address pointed by ED, also fixaddress is an optionnal delta in the range [-absmax, +absmax], with delta&mask==0 to be added to ed for LDR/STR */
uintptr_t geted(dynarec_la64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, rex_t rex, int* l, int i12, int delta)
{
    MAYUSE(dyn);
    MAYUSE(ninst);
    MAYUSE(delta);

    if (l == LOCK_LOCK) {
        dyn->insts[ninst].lock = 1;
    }

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
                        ADDSL(ret, scratch, TO_NAT(sib_reg), sib >> 6, ret);
                    } else {
                        if (sib >> 6) {
                            SLLI_D(ret, TO_NAT(sib_reg), (sib >> 6));
                        } else {
                            ret = TO_NAT(sib_reg);
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
                    ADDSL(ret, TO_NAT(sib_reg2), TO_NAT(sib_reg), sib >> 6, scratch);
                } else {
                    ret = TO_NAT(sib_reg2);
                }
            }
        } else if ((nextop & 7) == 5) {
            int64_t tmp = F32S64;
            int64_t adj = dyn->last_ip ? ((addr + delta) - dyn->last_ip) : 0;
            if (i12 && adj && (tmp + adj >= -2048) && (tmp + adj <= maxval)) {
                ret = xRIP;
                *fixaddress = tmp + adj;
            } else if (i12 && (tmp >= -2048) && (tmp <= maxval)) {
                GETIP(addr + delta, scratch);
                ret = xRIP;
                *fixaddress = tmp;
            } else if (adj && (tmp + adj >= -2048) && (tmp + adj <= maxval)) {
                ADDI_D(ret, xRIP, tmp + adj);
            } else if ((tmp >= -2048) && (tmp <= maxval)) {
                GETIP(addr + delta, scratch);
                ADDI_D(ret, xRIP, tmp);
            } else if (tmp + addr + delta < 0x80000000LL && !dyn->need_reloc) {
                MOV64x(ret, tmp + addr + delta);
            } else {
                if (adj) {
                    MOV64x(ret, tmp + adj);
                } else {
                    MOV64x(ret, tmp);
                    GETIP(addr + delta, scratch);
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
            ret = TO_NAT((nextop & 7) + (rex.b << 3));
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
        if (i64 == 0 || ((i64 >= -2048) && (i64 <= maxval) && i12)) {
            *fixaddress = i64;
            if ((nextop & 7) == 4) {
                if (sib_reg != 4) {
                    ADDSL(ret, TO_NAT(sib_reg2), TO_NAT(sib_reg), sib >> 6, scratch);
                } else {
                    ret = TO_NAT(sib_reg2);
                }
            } else {
                ret = TO_NAT((nextop & 0x07) + (rex.b << 3));
            }
        } else {
            if (i64 >= -2048 && i64 <= 2047) {
                if ((nextop & 7) == 4) {
                    if (sib_reg != 4) {
                        ADDSL(scratch, TO_NAT(sib_reg2), TO_NAT(sib_reg), sib >> 6, scratch);
                    } else {
                        scratch = TO_NAT(sib_reg2);
                    }
                } else {
                    scratch = TO_NAT((nextop & 0x07) + (rex.b << 3));
                }
                ADDI_D(ret, scratch, i64);
            } else {
                MOV64x(scratch, i64);
                if ((nextop & 7) == 4) {
                    if (sib_reg != 4) {
                        ADD_D(scratch, scratch, TO_NAT(sib_reg2));
                        ADDSL(ret, scratch, TO_NAT(sib_reg), sib >> 6, ret);
                    } else {
                        PASS3(int tmp = TO_NAT(sib_reg2));
                        ADD_D(ret, tmp, scratch);
                    }
                } else {
                    PASS3(int tmp = TO_NAT((nextop & 0x07) + (rex.b << 3)));
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
                        // no need to zero up, as we did it below
                        la64_move32(dyn, ninst, scratch, tmp, 0);
                        if ((sib >> 6)) {
                            SLLI_D(ret, TO_NAT(sib_reg), sib >> 6);
                            ADD_W(ret, ret, scratch);
                        } else {
                            ADD_W(ret, TO_NAT(sib_reg), scratch);
                        }
                        ZEROUP(ret);
                    } else {
                        if (sib >> 6) {
                            SLLI_D(ret, TO_NAT(sib_reg), (sib >> 6));
                            ZEROUP(ret);
                        } else {
                            ret = TO_NAT(sib_reg);
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
                        SLLI_D(ret, TO_NAT(sib_reg), (sib >> 6));
                        ADD_W(ret, ret, TO_NAT(sib_reg2));
                    } else {
                        ADD_W(ret, TO_NAT(sib_reg2), TO_NAT(sib_reg));
                    }
                    ZEROUP(ret);
                } else {
                    ret = TO_NAT(sib_reg2);
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
            ret = TO_NAT((nextop & 7));
            if (ret == hint) {
                ZEROUP2(hint, ret); // to clear upper part
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
        if (i32 == 0 || ((i32 >= -2048) && (i32 <= maxval) && i12)) {
            *fixaddress = i32;
            if ((nextop & 7) == 4) {
                if (sib_reg != 4) {
                    if (sib >> 6) {
                        SLLI_D(ret, TO_NAT(sib_reg), (sib >> 6));
                        ADD_W(ret, ret, TO_NAT(sib_reg2));
                    } else {
                        ADD_W(ret, TO_NAT(sib_reg2), TO_NAT(sib_reg));
                    }
                    ZEROUP(ret);
                } else {
                    ret = TO_NAT(sib_reg2);
                }
            } else {
                ret = TO_NAT((nextop & 0x07));
            }
        } else {
            if (i32 >= -2048 && i32 <= 2047) {
                if ((nextop & 7) == 4) {
                    if (sib_reg != 4) {
                        if (sib >> 6) {
                            SLLI_D(scratch, TO_NAT(sib_reg), sib >> 6);
                            ADD_W(scratch, scratch, TO_NAT(sib_reg2));
                        } else {
                            ADD_W(scratch, TO_NAT(sib_reg2), TO_NAT(sib_reg));
                        }
                    } else {
                        scratch = TO_NAT(sib_reg2);
                    }
                } else {
                    scratch = TO_NAT((nextop & 0x07));
                }
                ADDI_W(ret, scratch, i32);
                ZEROUP(ret);
            } else {
                // no need to zero up, as we did it below
                la64_move32(dyn, ninst, scratch, i32, 0);
                if ((nextop & 7) == 4) {
                    if (sib_reg != 4) {
                        ADD_W(scratch, scratch, TO_NAT(sib_reg2));
                        if (sib >> 6) {
                            SLLI_D(ret, TO_NAT(sib_reg), (sib >> 6));
                            ADD_W(ret, ret, scratch);
                        } else {
                            ADD_W(ret, scratch, TO_NAT(sib_reg));
                        }
                    } else {
                        PASS3(int tmp = TO_NAT(sib_reg2));
                        ADD_W(ret, tmp, scratch);
                    }
                } else {
                    PASS3(int tmp = TO_NAT((nextop & 0x07)));
                    ADD_W(ret, tmp, scratch);
                }
                ZEROUP(ret);
            }
        }
    }
    *ed = ret;
    return addr;
}

/* setup r2 to address pointed by ED, also fixaddress is an optionnal delta in the range [-absmax, +absmax], with delta&mask==0 to be added to ed for LDR/STR */
uintptr_t geted32(dynarec_la64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, rex_t rex, int* l, int i12, int delta)
{
    MAYUSE(dyn);
    MAYUSE(ninst);
    MAYUSE(delta);

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
            int sib_reg = ((sib >> 3) & 0x7) + (rex.x << 3);
            int sib_reg2 = (sib & 0x7) + (rex.b << 3);
            if ((sib & 0x7) == 5) {
                int64_t tmp = F32S;
                if (sib_reg != 4) {
                    if (tmp && ((tmp < -2048) || (tmp > maxval) || !i12)) {
                        MOV64x(scratch, tmp);
                        if ((sib >> 6)) {
                            SLLI_D(ret, TO_NAT(sib_reg), sib >> 6);
                            ADD_W(ret, ret, scratch);
                        } else {
                            ADD_W(ret, TO_NAT(sib_reg), scratch);
                        }
                        ZEROUP(ret);
                    } else {
                        if (sib >> 6)
                            SLLI_D(ret, TO_NAT(sib_reg), (sib >> 6));
                        else
                            ret = TO_NAT(sib_reg);
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
                    if ((sib >> 6)) {
                        SLLI_D(ret, TO_NAT(sib_reg), (sib >> 6));
                        ADD_W(ret, ret, TO_NAT(sib_reg2));
                    } else {
                        ADD_W(ret, TO_NAT(sib_reg2), TO_NAT(sib_reg));
                    }
                    ZEROUP(ret);
                } else {
                    ret = TO_NAT(sib_reg2);
                }
            }
        } else if ((nextop & 7) == 5) {
            uint32_t tmp = F32;
            // no need to zero up, as we did it below
            la64_move32(dyn, ninst, ret, tmp, 0);
            GETIP(addr + delta, scratch);
            ADD_W(ret, ret, xRIP);
            ZEROUP(ret);
            switch (lock) {
                case 1: addLockAddress(addr + delta + tmp); break;
                case 2:
                    if (isLockAddress(addr + delta + tmp)) *l = 1;
                    break;
            }
        } else {
            ret = TO_NAT((nextop & 7) + (rex.b << 3));
            if (ret == hint) {
                ZEROUP2(hint, ret); // to clear upper part
            }
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
        if (i64 == 0 || ((i64 >= -2048) && (i64 <= maxval) && i12)) {
            *fixaddress = i64;
            if ((nextop & 7) == 4) {
                if (sib_reg != 4) {
                    if (sib >> 6) {
                        SLLI_D(ret, TO_NAT(sib_reg), (sib >> 6));
                        ADD_W(ret, ret, TO_NAT(sib_reg2));
                    } else {
                        ADD_W(ret, TO_NAT(sib_reg2), TO_NAT(sib_reg));
                    }
                    ZEROUP(ret);
                } else {
                    ret = TO_NAT(sib_reg2);
                }
            } else {
                ret = TO_NAT((nextop & 0x07) + (rex.b << 3));
            }
        } else {
            if (i64 >= -2048 && i64 <= 2047) {
                if ((nextop & 7) == 4) {
                    if (sib_reg != 4) {
                        if (sib >> 6) {
                            SLLI_D(scratch, TO_NAT(sib_reg), sib >> 6);
                            ADD_W(scratch, scratch, TO_NAT(sib_reg2));
                        } else
                            ADD_W(scratch, TO_NAT(sib_reg2), TO_NAT(sib_reg));
                    } else {
                        scratch = TO_NAT(sib_reg2);
                    }
                } else
                    scratch = TO_NAT((nextop & 0x07) + (rex.b << 3));
                ADDI_W(ret, scratch, i64);
                ZEROUP(ret);
            } else {
                // no need to zero up, as we did it below
                la64_move32(dyn, ninst, scratch, i64, 0);
                if ((nextop & 7) == 4) {
                    if (sib_reg != 4) {
                        ADD_W(scratch, scratch, TO_NAT(sib_reg2));
                        if (sib >> 6) {
                            SLLI_D(ret, TO_NAT(sib_reg), (sib >> 6));
                            ADD_W(ret, ret, scratch);
                        } else
                            ADD_W(ret, scratch, TO_NAT(sib_reg));
                    } else {
                        PASS3(int tmp = TO_NAT(sib_reg2));
                        ADD_W(ret, tmp, scratch);
                    }
                } else {
                    PASS3(int tmp = TO_NAT((nextop & 0x07) + (rex.b << 3)));
                    ADD_W(ret, tmp, scratch);
                }
                ZEROUP(ret);
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
        GETIP_(ip, x2);
    }
    TABLE64C(x2, const_epilog);
    SMEND();
    BR(x2);
}

void jump_to_epilog_fast(dynarec_la64_t* dyn, uintptr_t ip, int reg, int ninst)
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
        GETIP_(ip, x2);
    }
    TABLE64C(x2, const_epilog_fast);
    SMEND();
    BR(x2);
}


static int indirect_lookup(dynarec_la64_t* dyn, int ninst, int is32bits, int s1, int s2)
{
    MAYUSE(dyn);
    if (!is32bits) {
        SRLI_D(s1, xRIP, 48);
        BNEZ_safe(s1, (intptr_t)dyn->jmp_next - (intptr_t)dyn->block);
        if (dyn->need_reloc) {
            TABLE64C(s2, const_jmptbl48);
        } else {
            MOV64x(s2, getConst(const_jmptbl48));
        }
        BSTRPICK_D(s1, xRIP, JMPTABL_START2 + JMPTABL_SHIFT2 - 1, JMPTABL_START2);
        ALSL_D(s2, s1, s2, 3);
        LD_D(s2, s2, 0);
    } else {
        TABLE64C(s2, const_jmptbl32);
    }
    BSTRPICK_D(s1, xRIP, JMPTABL_START1 + JMPTABL_SHIFT1 - 1, JMPTABL_START1);
    ALSL_D(s2, s1, s2, 3);
    LD_D(s2, s2, 0);
    BSTRPICK_D(s1, xRIP, JMPTABL_START0 + JMPTABL_SHIFT0 - 1, JMPTABL_START0);
    ALSL_D(s2, s1, s2, 3);
    LD_D(s1, s2, 0);
    return s1;
}

void jump_to_next(dynarec_la64_t* dyn, uintptr_t ip, int reg, int ninst, int is32bits)
{
    MAYUSE(dyn);
    MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Jump to next\n");

    if (is32bits)
        ip &= 0xffffffffLL;

    int dest;
    if (reg) {
        if (reg != xRIP) {
            MV(xRIP, reg);
        }
        NOTEST(x2);
        dest = indirect_lookup(dyn, ninst, is32bits, x2, x3);
    } else {
        NOTEST(x2);
        uintptr_t p = getJumpTableAddress64(ip);
        MAYUSE(p);
        GETIP_(ip, x3);
        if (dyn->need_reloc) AddRelocTable64JmpTbl(dyn, ninst, ip, STEP);
        TABLE64_(x3, p);
        LD_D(x2, x3, 0);
        dest = x2;
    }
    if (reg != x1) {
        MV(x1, xRIP);
    }
    CLEARIP();
    SMEND();
    JIRL((dyn->insts[ninst].x64.has_callret ? xRA : xZR), dest, 0x0);
}

void ret_to_epilog(dynarec_la64_t* dyn, uintptr_t ip, int ninst, rex_t rex)
{
    MAYUSE(dyn);
    MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Ret to epilog\n");
    POP1z(xRIP);
    MVz(x1, xRIP);
    SMEND();
    if (BOX64DRENV(dynarec_callret)) {
        // pop the actual return address from RV64 stack
        LD_D(xRA, xSP, 0);    // native addr
        LD_D(x6, xSP, 8);     // x86 addr
        ADDI_D(xSP, xSP, 16); // pop
        BNE(x6, xRIP, 2 * 4); // is it the right address?
        BR(xRA);
        // not the correct return address, regular jump, but purge the stack first, it's unsync now...
        ADDI_D(xSP, xSavedSP, -16);
    }
    NOTEST(x2);
    int dest = indirect_lookup(dyn, ninst, rex.is32bits, x2, x3);
    BR(dest);
    CLEARIP();
}

void retn_to_epilog(dynarec_la64_t* dyn, uintptr_t ip, int ninst, rex_t rex, int n)
{
    MAYUSE(dyn);
    MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Retn to epilog\n");
    POP1z(xRIP);
    if (n > 0x7ff) {
        MOV64x(x1, n);
        ADDz(xRSP, xRSP, x1);
    } else {
        ADDIz(xRSP, xRSP, n);
    }
    MVz(x1, xRIP);
    SMEND();
    if (BOX64DRENV(dynarec_callret)) {
        // pop the actual return address from RV64 stack
        LD_D(xRA, xSP, 0);    // native addr
        LD_D(x6, xSP, 8);     // x86 addr
        ADDI_D(xSP, xSP, 16); // pop
        BNE(x6, xRIP, 2 * 4); // is it the right address?
        BR(xRA);
        // not the correct return address, regular jump, but purge the stack first, it's unsync now...
        ADDI_D(xSP, xSavedSP, -16);
    }

    NOTEST(x2);
    int dest = indirect_lookup(dyn, ninst, rex.is32bits, x2, x3);
    BR(dest);
    CLEARIP();
}

void iret_to_epilog(dynarec_la64_t* dyn, uintptr_t ip, int ninst, int is64bits)
{
    // #warning TODO: is64bits
    MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "IRet to epilog\n");
    NOTEST(x2);
    if (is64bits) {
        POP1(xRIP);
        POP1(x2);
        POP1(xFlags);
    } else {
        POP1_32(xRIP);
        POP1_32(x2);
        POP1_32(xFlags);
    }

    ST_H(x2, xEmu, offsetof(x64emu_t, segs[_CS]));
    ST_W(xZR, xEmu, offsetof(x64emu_t, segs_serial[_CS]));
    // clean EFLAGS
    MOV32w(x1, 0x3F7FD7);
    AND(xFlags, xFlags, x1);
    ORI(xFlags, xFlags, 0x2);
    SPILL_EFLAGS();
    SET_DFNONE();
    // POP RSP
    if (is64bits) {
        POP1(x3); // rsp
        POP1(x2); // ss
    } else {
        POP1_32(x3); // rsp
        POP1_32(x2); // ss
    }
    // POP SS
    ST_H(x2, xEmu, offsetof(x64emu_t, segs[_SS]));
    ST_W(xZR, xEmu, offsetof(x64emu_t, segs_serial[_SS]));
    // set new RSP
    MV(xRSP, x3);
    // Ret....
    // epilog on purpose, CS might have changed!
    if (dyn->need_reloc)
        TABLE64C(x2, const_epilog);
    else
        MOV64x(x2, getConst(const_epilog));
    SMEND();
    BR(x2);
    CLEARIP();
}

void call_c(dynarec_la64_t* dyn, int ninst, la64_consts_t fnc, int reg, int ret, int saveflags, int savereg)
{
    MAYUSE(fnc);
    if (savereg == 0)
        savereg = x6;
    if (saveflags) {
        RESTORE_EFLAGS(reg);
        ST_D(xFlags, xEmu, offsetof(x64emu_t, eflags));
    }
    fpu_pushcache(dyn, ninst, reg, 0);
    if (ret != -2) {
        ADDI_D(xSP, xSP, -16); // RV64 stack needs to be 16byte aligned
        ST_D(xEmu, xSP, 0);
        ST_D(savereg, xSP, 8);
        // $r4..$r20 needs to be saved by caller
        STORE_REG(RAX);
        STORE_REG(RCX);
        STORE_REG(RDX);
        STORE_REG(RBX);
        STORE_REG(RSP);
        STORE_REG(RBP);
        STORE_REG(RSI);
        STORE_REG(RDI);
        ST_D(xRIP, xEmu, offsetof(x64emu_t, ip));
    }
    TABLE64C(reg, fnc);
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
        GO(RBX);
        GO(RSP);
        GO(RBP);
        GO(RSI);
        GO(RDI);
        if (ret != xRIP)
            LD_D(xRIP, xEmu, offsetof(x64emu_t, ip));
#undef GO
    }

    fpu_popcache(dyn, ninst, reg, 0);
    if (saveflags) {
        LD_D(xFlags, xEmu, offsetof(x64emu_t, eflags));
        SPILL_EFLAGS();
    }
    // SET_NODF();
    dyn->last_ip = 0;
}

void grab_segdata(dynarec_la64_t* dyn, uintptr_t addr, int ninst, int reg, int segment, int modreg)
{
    (void)addr;
    int64_t j64;
    MAYUSE(j64);
    if (modreg) return;
    MESSAGE(LOG_DUMP, "Get %s Offset\n", (segment == _FS) ? "FS" : "GS");
    int t1 = x1, t2 = x4;
    if (reg == t1) ++t1;
    if (reg == t2) ++t2;
    LD_WU(t2, xEmu, offsetof(x64emu_t, segs_serial[segment]));
    LD_D(reg, xEmu, offsetof(x64emu_t, segs_offs[segment]));
    if (segment == _GS) {
        CBNZ_MARKSEG(t2); // fast check
    } else {
        LD_D(t1, xEmu, offsetof(x64emu_t, context));
        LD_WU(t1, t1, offsetof(box64context_t, sel_serial));
        SUB_W(t1, t1, t2);
        CBZ_MARKSEG(t1);
    }
    MOV64x(x1, segment);
    call_c(dyn, ninst, const_getsegmentbase, t2, reg, 0, xFlags);
    MARKSEG;
    MESSAGE(LOG_DUMP, "----%s Offset\n", (segment == _FS) ? "FS" : "GS");
}


void x87_forget(dynarec_la64_t* dyn, int ninst, int s1, int s2, int st)
{
    // TODO
}

// Set rounding according to mxcsr flags, return reg to restore flags
int sse_setround(dynarec_la64_t* dyn, int ninst, int s1, int s2)
{
    MAYUSE(dyn);
    MAYUSE(ninst);
    MAYUSE(s1);
    MAYUSE(s2);
    LD_W(s1, xEmu, offsetof(x64emu_t, mxcsr));
    BSTRPICK_W(s1, s1, 14, 13);
    // MMX/x87 Round mode: 0..3: Nearest, Down, Up, Chop
    // LA64: 0..3: Nearest, TowardZero, TowardsPositive, TowardsNegative
    // 0->0, 1->3, 2->2, 3->1
    SUB_W(s1, xZR, s1);
    ANDI(s1, s1, 3);
    // done
    SLLI_D(s1, s1, 8);
    MOVFCSR2GR(s2, FCSR3);
    MOVGR2FCSR(FCSR3, s1); // exchange RM with current
    return s2;
}


void x87_purgecache(dynarec_la64_t* dyn, int ninst, int next, int s1, int s2, int s3)
{
    // TODO
}

// Restore round flag
void x87_restoreround(dynarec_la64_t* dyn, int ninst, int s1)
{
    MAYUSE(dyn);
    MAYUSE(ninst);
    MAYUSE(s1);
    MOVGR2FCSR(FCSR3, s1);
}

// MMX helpers
static int isx87Empty(dynarec_la64_t* dyn)
{
    for (int i = 0; i < 8; ++i)
        if (dyn->lsx.x87cache[i] != -1)
            return 0;
    return 1;
}

// get neon register for a MMX reg, create the entry if needed
int mmx_get_reg(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int a)
{
    if (!dyn->lsx.x87stack && isx87Empty(dyn))
        x87_purgecache(dyn, ninst, 0, s1, s2, s3);
    if (dyn->lsx.mmxcache[a] != -1)
        return dyn->lsx.mmxcache[a];
    ++dyn->lsx.mmxcount;
    int ret = dyn->lsx.mmxcache[a] = fpu_get_reg_emm(dyn, a);
    FLD_D(ret, xEmu, offsetof(x64emu_t, mmx[a]));
    return ret;
}
// get neon register for a MMX reg, but don't try to synch it if it needed to be created
int mmx_get_reg_empty(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int a)
{
    if (!dyn->lsx.x87stack && isx87Empty(dyn))
        x87_purgecache(dyn, ninst, 0, s1, s2, s3);
    if (dyn->lsx.mmxcache[a] != -1)
        return dyn->lsx.mmxcache[a];
    ++dyn->lsx.mmxcount;
    int ret = dyn->lsx.mmxcache[a] = fpu_get_reg_emm(dyn, a);
    return ret;
}
// purge the MMX cache only
void mmx_purgecache(dynarec_la64_t* dyn, int ninst, int next, int s1)
{
    if (!dyn->lsx.mmxcount) return;
    if (!next) dyn->lsx.mmxcount = 0;
    int old = -1;
    for (int i = 0; i < 8; ++i)
        if (dyn->lsx.mmxcache[i] != -1) {
            if (old == -1) {
                MESSAGE(LOG_DUMP, "\tPurge %sMMX Cache ------\n", next ? "locally " : "");
                ++old;
            }
            FST_D(dyn->lsx.mmxcache[i], xEmu, offsetof(x64emu_t, mmx[i]));
            if (!next) {
                fpu_free_reg(dyn, dyn->lsx.mmxcache[i]);
                dyn->lsx.mmxcache[i] = -1;
            }
        }
    if (old != -1) {
        MESSAGE(LOG_DUMP, "\t------ Purge MMX Cache\n");
    }
}

static void mmx_reflectcache(dynarec_la64_t* dyn, int ninst, int s1)
{
    for (int i = 0; i < 8; ++i)
        if (dyn->lsx.mmxcache[i] != -1) {
            FST_D(dyn->lsx.mmxcache[i], xEmu, offsetof(x64emu_t, mmx[i]));
        }
}

// SSE / SSE2 helpers
// get lsx register for a SSE reg, create the entry if needed
int sse_get_reg(dynarec_la64_t* dyn, int ninst, int s1, int a, int forwrite)
{
    if (dyn->lsx.ssecache[a].v != -1) {
        if (forwrite) {
            dyn->lsx.ssecache[a].write = 1; // update only if forwrite
            dyn->lsx.lsxcache[dyn->lsx.ssecache[a].reg].t = LSX_CACHE_XMMW;
        }
        return dyn->lsx.ssecache[a].reg;
    }
    int need_vld = 1;
    // migrate from avx to sse
    if (dyn->lsx.avxcache[a].v != -1) {
        avx_reflect_reg_upper128(dyn, ninst, a, forwrite);
        dyn->lsx.avxcache[a].v = -1;
        need_vld = 0;
    }
    dyn->lsx.ssecache[a].reg = fpu_get_reg_xmm(dyn, forwrite ? LSX_CACHE_XMMW : LSX_CACHE_XMMR, a);
    int ret = dyn->lsx.ssecache[a].reg;
    dyn->lsx.ssecache[a].write = forwrite;
    if(need_vld) VLD(ret, xEmu, offsetof(x64emu_t, xmm[a])); //skip VLD if migrate from avx
    return ret;
}

// get lsx register for an SSE reg, but don't try to synch it if it needed to be created
int sse_get_reg_empty(dynarec_la64_t* dyn, int ninst, int s1, int a)
{
    if (dyn->lsx.ssecache[a].v != -1) {
        dyn->lsx.ssecache[a].write = 1;
        dyn->lsx.lsxcache[dyn->lsx.ssecache[a].reg].t = LSX_CACHE_XMMW;
        return dyn->lsx.ssecache[a].reg;
    }
    // migrate from avx to sse
    if (dyn->lsx.avxcache[a].v != -1) {
        avx_reflect_reg_upper128(dyn, ninst, a, 1);
        dyn->lsx.avxcache[a].v = -1;
    }
    dyn->lsx.ssecache[a].reg = fpu_get_reg_xmm(dyn, LSX_CACHE_XMMW, a);
    dyn->lsx.ssecache[a].write = 1; // it will be write...
    return dyn->lsx.ssecache[a].reg;
}
// forget ext register for a SSE reg, does nothing if the regs is not loaded
void sse_forget_reg(dynarec_la64_t* dyn, int ninst, int a)
{
    if (dyn->lsx.ssecache[a].v == -1)
        return;
    if (dyn->lsx.lsxcache[dyn->lsx.ssecache[a].reg].t == LSX_CACHE_XMMW) {
        VST(dyn->lsx.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
    }
    fpu_free_reg(dyn, dyn->lsx.ssecache[a].reg);
    dyn->lsx.ssecache[a].v = -1;
    return;
}

void sse_reflect_reg(dynarec_la64_t* dyn, int ninst, int a)
{
    if (dyn->lsx.ssecache[a].v == -1)
        return;
    if (dyn->lsx.lsxcache[dyn->lsx.ssecache[a].reg].t == LSX_CACHE_XMMW) {
        VST(dyn->lsx.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
    }
}

// purge the SSE cache for XMM0..XMM7 (to use before function native call)
void sse_purge07cache(dynarec_la64_t* dyn, int ninst, int s1)
{
    int old = -1;
    for (int i = 0; i < 8; ++i)
        if (dyn->lsx.ssecache[i].v != -1 || dyn->lsx.avxcache[i].v != -1) {
            if (old == -1) {
                MESSAGE(LOG_DUMP, "\tPurge XMM0..7 Cache ------\n");
                ++old;
            }
            if (dyn->lsx.lsxcache[dyn->lsx.avxcache[i].reg].t == LSX_CACHE_YMMW) {
                VST(dyn->lsx.avxcache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
                if(dyn->lsx.avxcache[i].zero_upper == 1){
                    XVXOR_V(SCRATCH, SCRATCH, SCRATCH);
                }else{
                    XVPERMI_Q(SCRATCH, dyn->lsx.avxcache[i].reg, XVPERMI_IMM_4_0(0, 1));
                }
                VST(SCRATCH, xEmu, offsetof(x64emu_t, ymm[i]));
                fpu_free_reg(dyn, dyn->lsx.avxcache[i].reg);
                dyn->lsx.avxcache[i].v = -1;
            } else if (dyn->lsx.lsxcache[dyn->lsx.ssecache[i].reg].t == LSX_CACHE_XMMW) {
                VST(dyn->lsx.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
                fpu_free_reg(dyn, dyn->lsx.ssecache[i].reg);
                dyn->lsx.ssecache[i].v = -1;
            }
        }
    if (old != -1) {
        MESSAGE(LOG_DUMP, "\t------ Purge XMM0..7 Cache\n");
    }
}

// purge the SSE cache only
static void sse_purgecache(dynarec_la64_t* dyn, int ninst, int next, int s1)
{
    int old = -1;
    for (int i = 0; i < 16; ++i)
        if (dyn->lsx.ssecache[i].v != -1) {
            if (dyn->lsx.ssecache[i].write) {
                if (old == -1) {
                    MESSAGE(LOG_DUMP, "\tPurge %sSSE Cache ------\n", next ? "locally " : "");
                    ++old;
                }
                VST(dyn->lsx.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            }
            if (!next) {
                fpu_free_reg(dyn, dyn->lsx.ssecache[i].reg);
                dyn->lsx.ssecache[i].v = -1;
            }
        }
    if (old != -1) {
        MESSAGE(LOG_DUMP, "\t------ Purge SSE Cache\n");
    }
}

static void sse_reflectcache(dynarec_la64_t* dyn, int ninst, int s1)
{
    for (int i = 0; i < 16; ++i)
        if (dyn->lsx.ssecache[i].v != -1 && dyn->lsx.ssecache[i].write) {
            VST(dyn->lsx.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
        }
}

// AVX helpers
// get lasx register for a SSE reg, create the entry if needed
int avx_get_reg(dynarec_la64_t* dyn, int ninst, int s1, int a, int forwrite, int width)
{
    if (dyn->lsx.avxcache[a].v != -1) {
        if (forwrite) {
            dyn->lsx.avxcache[a].write = 1; // update only if forwrite
            dyn->lsx.lsxcache[dyn->lsx.avxcache[a].reg].t = LSX_CACHE_YMMW;
        }
        if (width == LSX_AVX_WIDTH_128) {
            dyn->lsx.avxcache[a].width = LSX_AVX_WIDTH_128;
            if(forwrite) dyn->lsx.avxcache[a].zero_upper = 1;
        } else {
            // if width changed to 256, and vzeroup ==1, means need zero-fill upper 128bits now.
            if (dyn->lsx.avxcache[a].zero_upper == 1) {
                dyn->lsx.avxcache[a].zero_upper = 0;
                XVXOR_V(SCRATCH, SCRATCH, SCRATCH);
                XVPERMI_Q(dyn->lsx.avxcache[a].reg, SCRATCH, 0b00000010);
            }
            dyn->lsx.avxcache[a].width = LSX_AVX_WIDTH_256;
        }

        return dyn->lsx.avxcache[a].reg;
    }

    // migrate from sse to avx
    if (dyn->lsx.ssecache[a].v != -1) {
        // release SSE reg cache
        fpu_free_reg(dyn, dyn->lsx.ssecache[a].reg);
        dyn->lsx.ssecache[a].v = -1;
    }

    // new reg
    dyn->lsx.avxcache[a].v = 0;
    dyn->lsx.avxcache[a].reg = fpu_get_reg_ymm(dyn, forwrite ? LSX_CACHE_YMMW : LSX_CACHE_YMMR, a);
    int ret = dyn->lsx.avxcache[a].reg;
    dyn->lsx.avxcache[a].write = forwrite;
    dyn->lsx.avxcache[a].width = width;
    if (width == LSX_AVX_WIDTH_128) {
        if(forwrite) dyn->lsx.avxcache[a].zero_upper = 1;
        VLD(ret, xEmu, offsetof(x64emu_t, xmm[a]));
    } else {
        VLD(ret, xEmu, offsetof(x64emu_t, xmm[a]));
        VLD(SCRATCH, xEmu, offsetof(x64emu_t, ymm[a]));
        XVPERMI_Q(ret, SCRATCH, XVPERMI_IMM_4_0(0, 2));
        dyn->lsx.avxcache[a].zero_upper = 0;
    }
    return ret;
}

int avx_get_reg_empty(dynarec_la64_t* dyn, int ninst, int s1, int a, int width)
{
    if (dyn->lsx.avxcache[a].v != -1) {
        dyn->lsx.avxcache[a].write = 1;
        dyn->lsx.lsxcache[dyn->lsx.avxcache[a].reg].t = LSX_CACHE_YMMW;
        if (width == LSX_AVX_WIDTH_128) {
            dyn->lsx.avxcache[a].width = LSX_AVX_WIDTH_128;
            dyn->lsx.avxcache[a].zero_upper = 1;
        } else {
            dyn->lsx.avxcache[a].width = LSX_AVX_WIDTH_256;
            dyn->lsx.avxcache[a].zero_upper = 0;
        }
        return dyn->lsx.avxcache[a].reg;
    }
    // migrate from sse to avx
    if (dyn->lsx.ssecache[a].v != -1) {
        // Release SSE reg cache
        fpu_free_reg(dyn, dyn->lsx.ssecache[a].reg);
        dyn->lsx.ssecache[a].v = -1;
    }
    dyn->lsx.avxcache[a].v = 0;
    dyn->lsx.avxcache[a].reg = fpu_get_reg_ymm(dyn, LSX_CACHE_YMMW, a);
    dyn->lsx.avxcache[a].write = 1;
    dyn->lsx.avxcache[a].width = width;
    if (width == LSX_AVX_WIDTH_128){
        dyn->lsx.avxcache[a].zero_upper = 1;
    } else {
        dyn->lsx.avxcache[a].zero_upper = 0;
    }
    return dyn->lsx.avxcache[a].reg;
}

void avx_reflect_reg_upper128(dynarec_la64_t* dyn, int ninst, int a, int forwrite)
{
    if (dyn->lsx.avxcache[a].v == -1 || forwrite == 0)
        return;
    if (dyn->lsx.lsxcache[dyn->lsx.avxcache[a].reg].t == LSX_CACHE_YMMW) {
        if (dyn->lsx.avxcache[a].zero_upper == 1) {
            XVXOR_V(SCRATCH, SCRATCH, SCRATCH);
        } else {
            XVPERMI_Q(SCRATCH, dyn->lsx.avxcache[a].reg, XVPERMI_IMM_4_0(0, 1));
        }
        VST(SCRATCH, xEmu, offsetof(x64emu_t, ymm[a]));
    }
    dyn->lsx.avxcache[a].v = -1;
    return;
}

void avx_forget_reg(dynarec_la64_t* dyn, int ninst, int a)
{
    if (dyn->lsx.avxcache[a].v == -1)
        return;
    if (dyn->lsx.lsxcache[dyn->lsx.avxcache[a].reg].t == LSX_CACHE_YMMW) {
        VST(dyn->lsx.avxcache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
        if (dyn->lsx.avxcache[a].zero_upper == 1) {
            XVXOR_V(SCRATCH, SCRATCH, SCRATCH);
        } else {
            XVPERMI_Q(SCRATCH, dyn->lsx.avxcache[a].reg, XVPERMI_IMM_4_0(0, 1));
        }
        VST(SCRATCH, xEmu, offsetof(x64emu_t, ymm[a]));
    }
    fpu_free_reg(dyn, dyn->lsx.avxcache[a].reg);
    dyn->lsx.avxcache[a].v = -1;
    return;
}

void avx_reflect_reg(dynarec_la64_t* dyn, int ninst, int a)
{
    if (dyn->lsx.avxcache[a].v == -1)
        return;
    if (dyn->lsx.lsxcache[dyn->lsx.avxcache[a].reg].t == LSX_CACHE_YMMW) {
        VST(dyn->lsx.avxcache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
        if (dyn->lsx.avxcache[a].zero_upper == 1) {
            XVXOR_V(SCRATCH, SCRATCH, SCRATCH);
            XVPERMI_Q(dyn->lsx.avxcache[a].reg, SCRATCH, 0b00000010);
        } else {
            XVPERMI_Q(SCRATCH, dyn->lsx.avxcache[a].reg, XVPERMI_IMM_4_0(0, 1));
        }
        VST(SCRATCH, xEmu, offsetof(x64emu_t, ymm[a]));
        dyn->lsx.avxcache[a].zero_upper = 0;
    }
}

// purge the AVX cache only
static void avx_purgecache(dynarec_la64_t* dyn, int ninst, int next, int s1)
{
    int old = -1;
    for (int i = 0; i < 16; ++i)
        if (dyn->lsx.avxcache[i].v != -1) {
            if (dyn->lsx.avxcache[i].write) {
                if (old == -1) {
                    MESSAGE(LOG_DUMP, "\tPurge %sAVX Cache ------\n", next ? "locally " : "");
                    ++old;
                }
                if (dyn->lsx.lsxcache[dyn->lsx.avxcache[i].reg].t == LSX_CACHE_YMMW) {
                    VST(dyn->lsx.avxcache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
                    if (dyn->lsx.avxcache[i].zero_upper == 1) {
                        XVXOR_V(SCRATCH, SCRATCH, SCRATCH);
                    } else {
                        XVPERMI_Q(SCRATCH, dyn->lsx.avxcache[i].reg, XVPERMI_IMM_4_0(0, 1));
                    }
                    VST(SCRATCH, xEmu, offsetof(x64emu_t, ymm[i]));
                }
            }
            if (!next) {
                fpu_free_reg(dyn, dyn->lsx.avxcache[i].reg);
                dyn->lsx.avxcache[i].v = -1;
            }
        }
    if (old != -1) {
        MESSAGE(LOG_DUMP, "\t------ Purge AVX Cache\n");
    }
}

static void avx_reflectcache(dynarec_la64_t* dyn, int ninst, int s1)
{
    for (int i = 0; i < 16; ++i) {
        if (dyn->lsx.avxcache[i].v != -1 && dyn->lsx.avxcache[i].write) {
            if (dyn->lsx.lsxcache[dyn->lsx.avxcache[i].reg].t == LSX_CACHE_YMMW) {
                avx_reflect_reg(dyn, ninst, i);
            }
        }
    }
}

void fpu_pushcache(dynarec_la64_t* dyn, int ninst, int s1, int not07)
{
    int start = not07 ? 8 : 0;
    int n = 0;

    for (int i = start; i < 16; i++) {
        if ((dyn->lsx.ssecache[i].v != -1) && (dyn->lsx.ssecache[i].write))
            ++n;
        if ((dyn->lsx.avxcache[i].v != -1) && (dyn->lsx.avxcache[i].write))
            ++n;
    }

    if (n) {
        MESSAGE(LOG_DUMP, "\tPush XMM/YMM Cache (%d)------\n", n);
        for (int i = start; i < 16; ++i) {
            if ((dyn->lsx.ssecache[i].v != -1) && (dyn->lsx.ssecache[i].write)) {
                VST(dyn->lsx.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            }
            if ((dyn->lsx.avxcache[i].v != -1) && (dyn->lsx.avxcache[i].write)) {
                    VST(dyn->lsx.avxcache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
                    if (dyn->lsx.avxcache[i].zero_upper == 0) {
                        XVPERMI_Q(SCRATCH, dyn->lsx.avxcache[i].reg, XVPERMI_IMM_4_0(0, 1));
                        VST(SCRATCH, xEmu, offsetof(x64emu_t, ymm[i]));
                    }
            }
        }
        MESSAGE(LOG_DUMP, "\t------- Pushed XMM/YMM Cache (%d)\n", n);
    }
}

void fpu_popcache(dynarec_la64_t* dyn, int ninst, int s1, int not07)
{
    int start = not07 ? 8 : 0;
    int n = 0;

    for (int i = start; i < 16; i++) {
        if (dyn->lsx.ssecache[i].v != -1 || dyn->lsx.avxcache[i].v != -1)
            ++n;
    }

    if (n) {
        MESSAGE(LOG_DUMP, "\tPop XMM/YMM Cache (%d)------\n", n);
        for (int i = start; i < 16; ++i) {
            if (dyn->lsx.ssecache[i].v != -1) {
                VLD(dyn->lsx.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            }
            if (dyn->lsx.avxcache[i].v != -1) {
                VLD(dyn->lsx.avxcache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
                if (dyn->lsx.avxcache[i].zero_upper == 0) {
                    VLD(SCRATCH, xEmu, offsetof(x64emu_t, ymm[i]));
                    XVPERMI_Q(dyn->lsx.avxcache[i].reg, SCRATCH, XVPERMI_IMM_4_0(0, 2));
                }
            }
        }
        MESSAGE(LOG_DUMP, "\t------- Pop XMM/YMM Cache (%d)\n", n);
    }
}

void fpu_purgecache(dynarec_la64_t* dyn, int ninst, int next, int s1, int s2, int s3)
{
    x87_purgecache(dyn, ninst, next, s1, s2, s3);
    mmx_purgecache(dyn, ninst, next, s1);
    sse_purgecache(dyn, ninst, next, s1);
    avx_purgecache(dyn, ninst, next, s1);
    if (!next)
        fpu_reset_reg(dyn);
}

void fpu_reflectcache(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3)
{
    // TODO: x87_reflectcache(dyn, ninst, s1, s2, s3);
    mmx_reflectcache(dyn, ninst, s1);
    sse_reflectcache(dyn, ninst, s1);
    avx_reflectcache(dyn, ninst, s1);
}

void fpu_unreflectcache(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3)
{
    // TODO
}

void emit_pf(dynarec_la64_t* dyn, int ninst, int s1, int s3, int s4)
{
    MAYUSE(dyn);
    MAYUSE(ninst);

    SRLI_D(s3, s1, 4);
    XOR(s3, s3, s1);

    SRLI_D(s4, s3, 2);
    XOR(s4, s3, s4);

    SRLI_D(s3, s4, 1);
    XOR(s3, s3, s4);

    XORI(s3, s3, 1);
    BSTRINS_D(xFlags, s3, F_PF, F_PF);
}

void fpu_reset_cache(dynarec_la64_t* dyn, int ninst, int reset_n)
{
    MESSAGE(LOG_DEBUG, "Reset Caches with %d\n", reset_n);
#if STEP > 1
    // for STEP 2 & 3, just need to refrest with current, and undo the changes (push & swap)
    dyn->lsx = dyn->insts[ninst].lsx;
    lsxcacheUnwind(&dyn->lsx);
#ifdef HAVE_TRACE
// TODO: trace
#endif // HAVE_TRACE
#else
    dyn->lsx = dyn->insts[reset_n].lsx;
#endif
}

// propagate ST stack state, especial stack pop that are deferred
void fpu_propagate_stack(dynarec_la64_t* dyn, int ninst)
{
    if (dyn->lsx.stack_pop) {
        for (int j = 0; j < 24; ++j)
            if ((dyn->lsx.lsxcache[j].t == LSX_CACHE_ST_D
                    || dyn->lsx.lsxcache[j].t == LSX_CACHE_ST_F
                    || dyn->lsx.lsxcache[j].t == LSX_CACHE_ST_I64)) {
                if (dyn->lsx.lsxcache[j].n < dyn->lsx.stack_pop)
                    dyn->lsx.lsxcache[j].v = 0;
                else
                    dyn->lsx.lsxcache[j].n -= dyn->lsx.stack_pop;
            }
        dyn->lsx.stack_pop = 0;
    }
    dyn->lsx.stack = dyn->lsx.stack_next;
    dyn->lsx.news = 0;
    dyn->lsx.stack_push = 0;
    dyn->lsx.swapped = 0;
}


static int findCacheSlot(dynarec_la64_t* dyn, int ninst, int t, int n, lsxcache_t* cache)
{
    lsx_cache_t f;
    f.n = n;
    f.t = t;
    for (int i = 0; i < 24; ++i) {
        if (cache->lsxcache[i].v == f.v)
            return i;
        if (cache->lsxcache[i].n == n) {
            switch (cache->lsxcache[i].t) {
                case LSX_CACHE_ST_F:
                    if (t == LSX_CACHE_ST_D)
                        return i;
                    if (t == LSX_CACHE_ST_I64)
                        return i;
                    break;
                case LSX_CACHE_ST_D:
                    if (t == LSX_CACHE_ST_F)
                        return i;
                    if (t == LSX_CACHE_ST_I64)
                        return i;
                    break;
                case LSX_CACHE_ST_I64:
                    if (t == LSX_CACHE_ST_F)
                        return i;
                    if (t == LSX_CACHE_ST_D)
                        return i;
                    break;
                case LSX_CACHE_XMMR:
                    if (t == LSX_CACHE_XMMW)
                        return i;
                    break;
                case LSX_CACHE_XMMW:
                    if (t == LSX_CACHE_XMMR)
                        return i;
                    break;
                case LSX_CACHE_YMMR:
                    if (t == LSX_CACHE_YMMW)
                        return i;
                    break;
                case LSX_CACHE_YMMW:
                    if (t == LSX_CACHE_YMMR)
                        return i;
                    break;
            }
        }
    }
    return -1;
}

static void swapCache(dynarec_la64_t* dyn, int ninst, int i, int j, lsxcache_t* cache)
{
    if (i == j)
        return;
    int quad = 0;
    if (cache->lsxcache[i].t == LSX_CACHE_XMMR || cache->lsxcache[i].t == LSX_CACHE_XMMW)
        quad = 1;
    if (cache->lsxcache[j].t == LSX_CACHE_XMMR || cache->lsxcache[j].t == LSX_CACHE_XMMW)
        quad = 1;
    if (cache->lsxcache[i].t == LSX_CACHE_YMMR || cache->lsxcache[i].t == LSX_CACHE_YMMW)
        quad = 2;
    if (cache->lsxcache[j].t == LSX_CACHE_YMMR || cache->lsxcache[j].t == LSX_CACHE_YMMW)
        quad = 2;

    if (!cache->lsxcache[i].v) {
        // a mov is enough, no need to swap
        MESSAGE(LOG_DUMP, "\t  - Moving %d <- %d\n", i, j);
        switch (quad) {
            case 2:
                XVOR_V(i, j, j);
                break;
            case 1:
                VOR_V(i, j, j);
                break;
            default:
                VXOR_V(i, i, i);
                VEXTRINS_D(i, j, 0);
                break;
        }
        cache->lsxcache[i].v = cache->lsxcache[j].v;
        cache->lsxcache[j].v = 0;
        return;
    }
    // SWAP
    lsx_cache_t tmp;
    MESSAGE(LOG_DUMP, "\t  - Swapping %d <-> %d\n", i, j);
    // There is no VSWP in Arm64 NEON to swap 2 register contents!
    // so use a scratch...
    switch (quad) {
        case 2:
            XVOR_V(SCRATCH, i, i);
            XVOR_V(i, j, j);
            XVOR_V(j, SCRATCH, SCRATCH);
            break;
        case 1:
            VOR_V(SCRATCH, i, i);
            VOR_V(i, j, j);
            VOR_V(j, SCRATCH, SCRATCH);
            break;
        default:
            VXOR_V(SCRATCH, SCRATCH, SCRATCH);
            VEXTRINS_D(SCRATCH, i, 0);
            VXOR_V(i, i, i);
            VEXTRINS_D(i, j, 0);
            VXOR_V(j, j, j);
            VEXTRINS_D(j, SCRATCH, 0);
            break;
    }
    tmp.v = cache->lsxcache[i].v;
    cache->lsxcache[i].v = cache->lsxcache[j].v;
    cache->lsxcache[j].v = tmp.v;
}

static void loadCache(dynarec_la64_t* dyn, int ninst, int stack_cnt, int s1, int s2, int s3, int* s1_val, int* s2_val, int* s3_top, lsxcache_t* cache, int i, int t, int n)
{
    if (cache->lsxcache[i].v) {
        int quad = 0;
        if (t == LSX_CACHE_XMMR || t == LSX_CACHE_XMMW)
            quad = 1;
        if (t == LSX_CACHE_YMMR || t == LSX_CACHE_YMMW)
            quad = 2;
        if (cache->lsxcache[i].t == LSX_CACHE_XMMR || cache->lsxcache[i].t == LSX_CACHE_XMMW)
            quad = 1;
        if (cache->lsxcache[i].t == LSX_CACHE_YMMR || cache->lsxcache[i].t == LSX_CACHE_YMMW)
            quad = 2;
        int j = i + 1;
        while (cache->lsxcache[j].v)
            ++j;
        MESSAGE(LOG_DUMP, "\t  - Moving away %d\n", i);
        switch (quad) {
            case 2:
                XVOR_V(j, i, i);
                break;
            case 1:
                VOR_V(j, i, i);
                break;
            default:
                VXOR_V(j, j, j);
                VEXTRINS_D(j, i, 0);
                break;
        }
        cache->lsxcache[j].v = cache->lsxcache[i].v;
    }
    switch (t) {
        case LSX_CACHE_YMMR:
        case LSX_CACHE_YMMW:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            VLD(i, xEmu, offsetof(x64emu_t, xmm[n]));
            VLD(SCRATCH, xEmu, offsetof(x64emu_t, ymm[n]));
            XVPERMI_Q(i, SCRATCH, XVPERMI_IMM_4_0(0, 2));
            break;
        case LSX_CACHE_XMMR:
        case LSX_CACHE_XMMW:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            VLD(i, xEmu, offsetof(x64emu_t, xmm[n]));
            break;
        case LSX_CACHE_MM:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            FLD_D(SCRATCH, xEmu, offsetof(x64emu_t, mmx[n]));
            VXOR_V(i, i, i);
            VEXTRINS_D(i, SCRATCH, 0);
            break;
        case LSX_CACHE_ST_D:
        case LSX_CACHE_ST_F:
        case LSX_CACHE_ST_I64:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            // TODO: x87
            break;
        case LSX_CACHE_NONE:
        case LSX_CACHE_SCR:
        default: /* nothing done */
            MESSAGE(LOG_DUMP, "\t  - ignoring %s\n", getCacheName(t, n));
            break;
    }
    cache->lsxcache[i].n = n;
    cache->lsxcache[i].t = t;
}

static void unloadCache(dynarec_la64_t* dyn, int ninst, int stack_cnt, int s1, int s2, int s3, int* s1_val, int* s2_val, int* s3_top, lsxcache_t* cache, int i, int t, int n)
{
    switch (t) {
        case LSX_CACHE_XMMR:
        case LSX_CACHE_YMMR:
            MESSAGE(LOG_DUMP, "\t  - ignoring %s\n", getCacheName(t, n));
            break;
        case LSX_CACHE_XMMW:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            VST(i, xEmu, offsetof(x64emu_t, xmm[n]));
            break;
        case LSX_CACHE_YMMW:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            XVPERMI_Q(SCRATCH, i, XVPERMI_IMM_4_0(0, 1));
            VST(i, xEmu, offsetof(x64emu_t, xmm[n]));
            VST(SCRATCH, xEmu, offsetof(x64emu_t, ymm[n]));
            break;
        case LSX_CACHE_MM:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            FST_D(i, xEmu, offsetof(x64emu_t, mmx[n]));
            break;
        case LSX_CACHE_ST_D:
        case LSX_CACHE_ST_F:
        case LSX_CACHE_ST_I64:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            // TODO: x87
            break;
        case LSX_CACHE_NONE:
        case LSX_CACHE_SCR:
        default: /* nothing done */
            MESSAGE(LOG_DUMP, "\t  - ignoring %s\n", getCacheName(t, n));
            break;
    }
    cache->lsxcache[i].v = 0;
}

static void fpuCacheTransform(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3)
{
    int i2 = dyn->insts[ninst].x64.jmp_insts;
    if (i2 < 0)
        return;
    MESSAGE(LOG_DUMP, "\tCache Transform ---- ninst=%d -> %d\n", ninst, i2);
    if ((!i2) || (dyn->insts[i2].x64.barrier & BARRIER_FLOAT)) {
        if (dyn->lsx.stack_next) {
            fpu_purgecache(dyn, ninst, 1, s1, s2, s3);
            MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
            return;
        }
        for (int i = 0; i < 24; ++i)
            if (dyn->lsx.lsxcache[i].v) { // there is something at ninst for i
                fpu_purgecache(dyn, ninst, 1, s1, s2, s3);
                MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
                return;
            }
        MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
        return;
    }
    lsxcache_t cache_i2 = dyn->insts[i2].lsx;
    lsxcacheUnwind(&cache_i2);

    if (!cache_i2.stack) {
        int purge = 1;
        for (int i = 0; i < 24 && purge; ++i)
            if (cache_i2.lsxcache[i].v)
                purge = 0;
        if (purge) {
            fpu_purgecache(dyn, ninst, 1, s1, s2, s3);
            MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
            return;
        }
    }
    int stack_cnt = dyn->lsx.stack_next;
    int s3_top = 0xffff;
    if (stack_cnt != cache_i2.stack) {
        MESSAGE(LOG_DUMP, "\t    - adjust stack count %d -> %d -\n", stack_cnt, cache_i2.stack);
        int a = stack_cnt - cache_i2.stack;
        // TODO: x87
        s3_top = 0;
        stack_cnt = cache_i2.stack;
    }
    lsxcache_t cache = dyn->lsx;
    int s1_val = 0;
    int s2_val = 0;
    // unload every unneeded cache
    // check SSE first, than MMX, in order, for optimization issue
    for (int i = 0; i < 16; ++i) {
        int j = findCacheSlot(dyn, ninst, LSX_CACHE_YMMW, i, &cache);
        if (j >= 0 && findCacheSlot(dyn, ninst, LSX_CACHE_YMMW, i, &cache_i2) == -1)
            unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, j, cache.lsxcache[j].t, cache.lsxcache[j].n);
    }
    for (int i = 0; i < 16; ++i) {
        int j = findCacheSlot(dyn, ninst, LSX_CACHE_XMMW, i, &cache);
        if (j >= 0 && findCacheSlot(dyn, ninst, LSX_CACHE_XMMW, i, &cache_i2) == -1)
            unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, j, cache.lsxcache[j].t, cache.lsxcache[j].n);
    }
    for (int i = 0; i < 8; ++i) {
        int j = findCacheSlot(dyn, ninst, LSX_CACHE_MM, i, &cache);
        if (j >= 0 && findCacheSlot(dyn, ninst, LSX_CACHE_MM, i, &cache_i2) == -1)
            unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, j, cache.lsxcache[j].t, cache.lsxcache[j].n);
    }
    for (int i = 0; i < 24; ++i) {
        if (cache.lsxcache[i].v)
            if (findCacheSlot(dyn, ninst, cache.lsxcache[i].t, cache.lsxcache[i].n, &cache_i2) == -1)
                unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, i, cache.lsxcache[i].t, cache.lsxcache[i].n);
    }
    // and now load/swap the missing one
    for (int i = 0; i < 24; ++i) {
        if (cache_i2.lsxcache[i].v) {
            if (cache_i2.lsxcache[i].v != cache.lsxcache[i].v) {
                int j;
                if ((j = findCacheSlot(dyn, ninst, cache_i2.lsxcache[i].t, cache_i2.lsxcache[i].n, &cache)) == -1)
                    loadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, i, cache_i2.lsxcache[i].t, cache_i2.lsxcache[i].n);
                else {
                    // it's here, lets swap if needed
                    if (j != i)
                        swapCache(dyn, ninst, i, j, &cache);
                }
            }
            if (cache.lsxcache[i].t != cache_i2.lsxcache[i].t) {
                if (cache.lsxcache[i].t == LSX_CACHE_ST_D && cache_i2.lsxcache[i].t == LSX_CACHE_ST_F) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.lsxcache[i].t, cache.lsxcache[i].n));
                    FCVT_S_D(i, i);
                    cache.lsxcache[i].t = LSX_CACHE_ST_F;
                } else if (cache.lsxcache[i].t == LSX_CACHE_ST_F && cache_i2.lsxcache[i].t == LSX_CACHE_ST_D) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.lsxcache[i].t, cache.lsxcache[i].n));
                    FCVT_D_S(i, i);
                    cache.lsxcache[i].t = LSX_CACHE_ST_D;
                } else if (cache.lsxcache[i].t == LSX_CACHE_ST_D && cache_i2.lsxcache[i].t == LSX_CACHE_ST_I64) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.lsxcache[i].t, cache.lsxcache[i].n));
                    VFTINTRZ_L_D(i, i);
                    cache.lsxcache[i].t = LSX_CACHE_ST_I64;
                } else if (cache.lsxcache[i].t == LSX_CACHE_ST_F && cache_i2.lsxcache[i].t == LSX_CACHE_ST_I64) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.lsxcache[i].t, cache.lsxcache[i].n));
                    VFTINTRZL_L_S(i, i);
                    cache.lsxcache[i].t = LSX_CACHE_ST_D;
                } else if (cache.lsxcache[i].t == LSX_CACHE_ST_I64 && cache_i2.lsxcache[i].t == LSX_CACHE_ST_F) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.lsxcache[i].t, cache.lsxcache[i].n));
                    VFFINT_S_L(i, i, i);
                    cache.lsxcache[i].t = LSX_CACHE_ST_F;
                } else if (cache.lsxcache[i].t == LSX_CACHE_ST_I64 && cache_i2.lsxcache[i].t == LSX_CACHE_ST_D) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.lsxcache[i].t, cache.lsxcache[i].n));
                    VFFINT_D_L(i, i);
                    cache.lsxcache[i].t = LSX_CACHE_ST_D;
                } else if (cache.lsxcache[i].t == LSX_CACHE_XMMR && cache_i2.lsxcache[i].t == LSX_CACHE_XMMW) {
                    cache.lsxcache[i].t = LSX_CACHE_XMMW;
                } else if (cache.lsxcache[i].t == LSX_CACHE_YMMR && cache_i2.lsxcache[i].t == LSX_CACHE_YMMW) {
                    cache.lsxcache[i].t = LSX_CACHE_YMMW;
                } else if (cache.lsxcache[i].t == LSX_CACHE_YMMW && cache_i2.lsxcache[i].t == LSX_CACHE_YMMR) {
                    // refresh cache...
                    MESSAGE(LOG_DUMP, "\t  - Refreh %s\n", getCacheName(cache.lsxcache[i].t, cache.lsxcache[i].n));
                    XVPERMI_Q(SCRATCH, i, XVPERMI_IMM_4_0(0, 1));
                    VST(i, xEmu, offsetof(x64emu_t, xmm[cache.lsxcache[i].n]));
                    VST(SCRATCH, xEmu, offsetof(x64emu_t, ymm[cache.lsxcache[i].n]));
                    cache.lsxcache[i].t = LSX_CACHE_YMMR;
                } else if (cache.lsxcache[i].t == LSX_CACHE_XMMW && cache_i2.lsxcache[i].t == LSX_CACHE_XMMR) {
                    // refresh cache...
                    MESSAGE(LOG_DUMP, "\t  - Refreh %s\n", getCacheName(cache.lsxcache[i].t, cache.lsxcache[i].n));
                    VST(i, xEmu, offsetof(x64emu_t, xmm[cache.lsxcache[i].n]));
                    cache.lsxcache[i].t = LSX_CACHE_XMMR;
                }
            }
        }
    }
    MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
}

static void flagsCacheTransform(dynarec_la64_t* dyn, int ninst, int s1)
{
    int j64;
    int jmp = dyn->insts[ninst].x64.jmp_insts;
    if (jmp < 0)
        return;
    if (dyn->f.dfnone || ((dyn->insts[jmp].f_exit.dfnone && !dyn->insts[jmp].f_entry.dfnone) && !dyn->insts[jmp].x64.use_flags)) // flags are fully known, nothing we can do more
        return;
    MESSAGE(LOG_DUMP, "\tFlags fetch ---- ninst=%d -> %d\n", ninst, jmp);
    int go = (dyn->insts[jmp].f_entry.dfnone && !dyn->f.dfnone && !dyn->insts[jmp].df_notneeded) ? 1 : 0;
    switch (dyn->insts[jmp].f_entry.pending) {
        case SF_UNKNOWN:
            go = 0;
            break;
        default:
            if (go && !(dyn->insts[jmp].x64.need_before & X_PEND) && (dyn->f.pending != SF_UNKNOWN)) {
                // just clear df flags
                go = 0;
                ST_W(xZR, xEmu, offsetof(x64emu_t, df));
            }
            break;
    }
    if (go) {
        if (dyn->f.pending != SF_PENDING) {
            LD_WU(s1, xEmu, offsetof(x64emu_t, df));
            j64 = (GETMARKF2) - (dyn->native_size);
            BEQZ(s1, j64);
        }
        CALL_(const_updateflags, -1, 0);
        MARKF2;
    }
}

void CacheTransform(dynarec_la64_t* dyn, int ninst, int cacheupd, int s1, int s2, int s3)
{
    if (cacheupd & 2)
        fpuCacheTransform(dyn, ninst, s1, s2, s3);
    if (cacheupd & 1)
        flagsCacheTransform(dyn, ninst, s1);
}

void la64_move32(dynarec_la64_t* dyn, int ninst, int reg, int32_t val, int zeroup)
{
    if ((val & 0xfff) == val) {
        ORI(reg, xZR, val);
    } else if (((val << 20) >> 20) == val) {
        ADDI_W(reg, xZR, val & 0xfff);
    } else if ((val & 0xfff) == 0) {
        LU12I_W(reg, (val >> 12) & 0xfffff);
    } else {
        LU12I_W(reg, (val >> 12) & 0xfffff);
        ORI(reg, reg, val & 0xfff);
    }
    if (zeroup && val < 0) ZEROUP(reg);
}

void la64_move64(dynarec_la64_t* dyn, int ninst, int reg, int64_t val)
{
    la64_move32(dyn, ninst, reg, val, 0);
    if (((val << 32) >> 32) == val) {
        return;
    }
    LU32I_D(reg, (val >> 32) & 0xfffff);
    if (((val << 12) >> 12) == val) {
        return;
    }
    LU52I_D(reg, reg, (val >> 52) & 0xfff);
}
