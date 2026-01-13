#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

#include "bitutils.h"
#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "rv64_emitter.h"
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
#include "../dynarec_helper.h"

/* setup r2 to address pointed by ED, also fixaddress is an optionnal delta in the range [-absmax, +absmax], with delta&mask==0 to be added to ed for LDR/STR */
uintptr_t geted(dynarec_rv64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, rex_t rex, int* l, int i12, int delta)
{
    MAYUSE(dyn);
    MAYUSE(ninst);
    MAYUSE(delta);

    if (l == LOCK_LOCK) {
        dyn->insts[ninst].lock = 1;
    }

    int lock = l ? ((l == LOCK_LOCK) ? 1 : 2) : 0;
    if (lock == 2) *l = 0;

    if (rex.is32bits && rex.is67)
        return geted16(dyn, addr, ninst, nextop, ed, hint, scratch, fixaddress, rex, i12);

    uint8_t ret = x2;
    *fixaddress = 0;
    if (hint > 0) ret = hint;
    int maxval = 2047;
    if (i12 > 1)
        maxval -= i12;
    int seg_done = 0;
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
                        MOV64y(scratch, tmp);
                        ADDSLy(ret, scratch, TO_NAT(sib_reg), sib >> 6, ret);
                        SCRATCH_USAGE(1);
                    } else {
                        if (sib >> 6) {
                            SLLIy(ret, TO_NAT(sib_reg), (sib >> 6));
                            if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                        } else
                            ret = TO_NAT(sib_reg);
                        *fixaddress = tmp;
                    }
                } else {
                    if (rex.seg && !(tmp && ((tmp < -2048) || (tmp > maxval) || !i12))) {
                        grab_segdata(dyn, addr, ninst, ret, rex.seg);
                        seg_done = 1;
                        *fixaddress = tmp;
                    } else if (rex.seg && tmp >= -2048 && tmp < 2048) {
                        grab_segdata(dyn, addr, ninst, ret, rex.seg);
                        if (tmp) ADDI(ret, ret, tmp);
                        seg_done = 1;
                    } else
                        MOV64y(ret, tmp);
                    switch (lock) {
                        case 1: addLockAddress(tmp); break;
                        case 2:
                            if (isLockAddress(tmp)) *l = 1;
                            break;
                    }
                    if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                }
            } else {
                if (sib_reg != 4) {
                    if (!(sib >> 6)) {
                        ADD(ret, TO_NAT(sib_reg2), TO_NAT(sib_reg));
                        if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                    } else if (cpuext.zba) {
                        SHxADD(ret, TO_NAT(sib_reg), sib >> 6, TO_NAT(sib_reg2));
                        if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                    } else if (cpuext.xtheadba) {
                        TH_ADDSL(ret, TO_NAT(sib_reg2), TO_NAT(sib_reg), sib >> 6);
                        if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                    } else {
                        SLLI(scratch, TO_NAT(sib_reg), sib >> 6);
                        ADD(ret, TO_NAT(sib_reg2), scratch);
                        SCRATCH_USAGE(1);
                    }
                    if (rex.is32bits || rex.is67) ZEROUP(ret);
                } else {
                    ret = TO_NAT(sib_reg2);
                }
            }
        } else if ((nextop & 7) == 5) {
            if (rex.is32bits) {
                int tmp = F32S;
                if (rex.seg && tmp >= -2048 && tmp < 2048) {
                    grab_segdata(dyn, addr, ninst, ret, rex.seg);
                    if (tmp) ADDI(ret, ret, tmp);
                    seg_done = 1;
                } else
                    MOV32w(ret, tmp);
                if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                if (!rex.seg)
                    switch (lock) {
                        case 1:
                            addLockAddress(tmp);
                            if (fixaddress) *fixaddress = tmp;
                            break;
                        case 2:
                            if (isLockAddress(tmp)) *l = 1;
                            break;
                    }
            } else {
                int64_t tmp = F32S64;
                int64_t adj = dyn->last_ip ? ((addr + delta) - dyn->last_ip) : 0;
                if (i12 && adj && (tmp + adj >= -2048) && (tmp + adj <= maxval)) {
                    ret = xRIP;
                    *fixaddress = tmp + adj;
                } else if (i12 && (tmp >= -2048) && (tmp <= maxval)) {
                    GETIP(addr + delta, scratch);
                    ret = xRIP;
                    *fixaddress = tmp;
                    SCRATCH_USAGE(1);
                } else if (adj && (tmp + adj >= -2048) && (tmp + adj <= maxval)) {
                    ADDI(ret, xRIP, tmp + adj);
                } else if ((tmp >= -2048) && (tmp <= maxval)) {
                    GETIP(addr + delta, scratch);
                    ADDIy(ret, xRIP, tmp);
                    SCRATCH_USAGE(1);
                } else if (tmp + addr + delta < 0x80000000LL && !dyn->need_reloc) {
                    MOV64y(ret, tmp + addr + delta);
                } else {
                    if (adj) {
                        MOV64y(ret, tmp + adj);
                    } else {
                        MOV64y(ret, tmp);
                        GETIP(addr + delta, scratch);
                        SCRATCH_USAGE(1);
                    }
                    ADDy(ret, ret, xRIP);
                }
                if (!rex.seg)
                    switch (lock) {
                        case 1:
                            addLockAddress(addr + delta + tmp);
                            if (fixaddress) *fixaddress = addr + delta + tmp;
                            break;
                        case 2:
                            if (isLockAddress(addr + delta + tmp)) *l = 1;
                            break;
                    }
                if (!IS_GPR(ret)) SCRATCH_USAGE(1);
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
                    if (!(sib >> 6)) {
                        ADD(ret, TO_NAT(sib_reg2), TO_NAT(sib_reg));
                        if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                    } else if (cpuext.zba) {
                        SHxADD(ret, TO_NAT(sib_reg), sib >> 6, TO_NAT(sib_reg2));
                        if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                    } else if (cpuext.xtheadba) {
                        TH_ADDSL(ret, TO_NAT(sib_reg2), TO_NAT(sib_reg), sib >> 6);
                        if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                    } else {
                        SLLI(scratch, TO_NAT(sib_reg), sib >> 6);
                        ADD(ret, TO_NAT(sib_reg2), scratch);
                        SCRATCH_USAGE(1);
                    }
                    if (rex.is32bits || rex.is67) ZEROUP(ret);
                } else {
                    ret = TO_NAT(sib_reg2);
                }
            } else
                ret = TO_NAT((nextop & 0x07) + (rex.b << 3));
        } else {
            if (i64 >= -2048 && i64 <= 2047) {
                if ((nextop & 7) == 4) {
                    if (sib_reg != 4) {
                        if (!(sib >> 6)) {
                            ADD(ret, TO_NAT(sib_reg2), TO_NAT(sib_reg));
                            if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                        } else if (cpuext.zba) {
                            SHxADD(ret, TO_NAT(sib_reg), sib >> 6, TO_NAT(sib_reg2));
                            if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                        } else if (cpuext.xtheadba) {
                            TH_ADDSL(ret, TO_NAT(sib_reg2), TO_NAT(sib_reg), sib >> 6);
                            if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                        } else {
                            SLLI(scratch, TO_NAT(sib_reg), sib >> 6);
                            ADD(ret, TO_NAT(sib_reg2), scratch);
                            SCRATCH_USAGE(1);
                        }
                        ADDIy(ret, ret, i64);
                    } else {
                        ADDIy(ret, TO_NAT(sib_reg2), i64);
                        if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                    }
                } else {
                    ADDIy(ret, TO_NAT((nextop & 0x07) + (rex.b << 3)), i64);
                    if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                }
            } else {
                MOV64y(scratch, i64);
                SCRATCH_USAGE(1);
                if ((nextop & 7) == 4) {
                    if (sib_reg != 4) {
                        ADDy(scratch, scratch, TO_NAT(sib_reg2));
                        ADDSLy(ret, scratch, TO_NAT(sib_reg), sib >> 6, ret);
                    } else {
                        PASS3(int tmp = TO_NAT(sib_reg2));
                        ADDy(ret, tmp, scratch);
                    }
                } else {
                    PASS3(int tmp = TO_NAT((nextop & 0x07) + (rex.b << 3)));
                    ADDy(ret, tmp, scratch);
                }
            }
        }
    }
    if (rex.is67 && IS_GPR(ret) && !rex.seg) {
        ZEXTW2(hint, ret); // truncate for is67 case only (is32bits case regs are already 32bits only)
        ret = hint;
    }
    if (rex.seg && !seg_done) {
        if (scratch == ret)
            scratch = ret + 1;
        SCRATCH_USAGE(1);
        grab_segdata(dyn, addr, ninst, scratch, rex.seg);
        // seg offset is 64bits, so no truncation here
        ADDxREGy(hint, scratch, ret, hint);
        ret = hint;
    }
    *ed = ret;
    return addr;
}

uintptr_t geted16(dynarec_rv64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, rex_t rex, int i12)
{
    MAYUSE(dyn);
    MAYUSE(ninst);

    uint8_t ret = x2;
    *fixaddress = 0;
    if (hint > 0) ret = hint;
    MAYUSE(scratch);
    uint32_t m = nextop & 0xC7;
    uint32_t n = (m >> 6) & 3;
    int64_t offset = 0;
    if (!n && (m & 7) == 6) {
        offset = F16S;
        MOV32w(ret, offset);
    } else {
        switch (n) {
            case 0: offset = 0; break;
            case 1: offset = F8S; break;
            case 2: offset = F16S; break;
        }
        if (offset && offset >= -2048 && offset <= 2047) {
            *fixaddress = offset;
            offset = 0;
        }
        switch (m & 7) {
            case 0: // R_BX + R_SI
                ZEXTH(ret, xRBX);
                ZEXTH(scratch, xRSI);
                ADD(ret, ret, scratch);
                SCRATCH_USAGE(1);
                break;
            case 1: // R_BX + R_DI
                ZEXTH(ret, xRBX);
                ZEXTH(scratch, xRDI);
                ADD(ret, ret, scratch);
                SCRATCH_USAGE(1);
                break;
            case 2: // R_BP + R_SI
                ZEXTH(ret, xRBP);
                ZEXTH(scratch, xRSI);
                ADD(ret, ret, scratch);
                SCRATCH_USAGE(1);
                break;
            case 3: // R_BP + R_DI
                ZEXTH(ret, xRBP);
                ZEXTH(scratch, xRDI);
                ADD(ret, ret, scratch);
                SCRATCH_USAGE(1);
                break;
            case 4: // R_SI
                ZEXTH(ret, xRSI);
                if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                break;
            case 5: // R_DI
                ZEXTH(ret, xRDI);
                if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                break;
            case 6: // R_BP
                ZEXTH(ret, xRBP);
                if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                break;
            case 7: // R_BX
                ZEXTH(ret, xRBX);
                if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                break;
        }
        if (offset) {
            if (offset >= -2048 && offset < 2048) {
                ADDI(ret, ret, offset);
                if (!IS_GPR(ret)) SCRATCH_USAGE(1);
            } else {
                MOV64x(scratch, offset);
                ADD(ret, ret, scratch);
                SCRATCH_USAGE(1);
            }
        }
    }

    if (rex.seg) {
        if (scratch == ret)
            scratch = ret + 1;
        SCRATCH_USAGE(1);
        grab_segdata(dyn, addr, ninst, scratch, rex.seg);
        // seg offset is 64bits, so no truncation here
        if (IS_GPR(ret)) {
            ADD(hint, ret, scratch);
            ret = hint;
        } else
            ADD(ret, ret, scratch);
    }
    *ed = ret;
    return addr;
}

void jump_to_epilog(dynarec_rv64_t* dyn, uintptr_t ip, int reg, int ninst)
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

void jump_to_epilog_fast(dynarec_rv64_t* dyn, uintptr_t ip, int reg, int ninst)
{
    MAYUSE(dyn);
    MAYUSE(ip);
    MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Jump to epilog_fast\n");

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
#ifdef JMPTABLE_SHIFT4
#error TODO!
#endif

static int indirect_lookup(dynarec_rv64_t* dyn, int ninst, int is32bits, int s1, int s2)
{
    MAYUSE(dyn);
    if (cpuext.xtheadbb && cpuext.xtheadmemidx) {
        if (!is32bits) {
            SRLI(s1, xRIP, 48);
            BNEZ_safe(s1, (intptr_t)dyn->jmp_next - (intptr_t)dyn->block);
            if (dyn->need_reloc) {
                TABLE64C(s2, const_jmptbl48);
            } else {
                MOV64x(s2, getConst(const_jmptbl48));
            }
            TH_EXTU(s1, xRIP, JMPTABL_START2 + JMPTABL_SHIFT2 - 1, JMPTABL_START2);
            TH_LRD(s2, s2, s1, 3);
        } else {
            TABLE64C(s2, const_jmptbl32);
        }
        TH_EXTU(s1, xRIP, JMPTABL_START1 + JMPTABL_SHIFT1 - 1, JMPTABL_START1);
        TH_LRD(s2, s2, s1, 3);
        TH_EXTU(s1, xRIP, JMPTABL_START0 + JMPTABL_SHIFT0 - 1, JMPTABL_START0);
        TH_LRD(s1, s2, s1, 3);
    } else {
        if (!is32bits) {
            SRLI(s1, xRIP, 48);
            BNEZ_safe(s1, (intptr_t)dyn->jmp_next - (intptr_t)dyn->block);
            MOV64x(s2, getConst(const_jmptbl48));
            SRLI(s1, xRIP, JMPTABL_START2);
            ADDSL(s2, s2, s1, 3, s1);
            LD(s2, s2, 0);
        } else {
            TABLE64C(s2, const_jmptbl32);
        }
        MOV64x(x4, JMPTABLE_MASK1 << 3);
        SRLI(s1, xRIP, JMPTABL_START1 - 3);
        AND(s1, s1, x4);
        ADD(s2, s2, s1);
        LD(s2, s2, 0);
        if (JMPTABLE_MASK0 < 2048) {
            ANDI(s1, xRIP, JMPTABLE_MASK0);
        } else {
            MOV64x(x4, JMPTABLE_MASK0); // x4 = mask
            AND(s1, xRIP, x4);
        }
        ADDSL(s2, s2, s1, 3, s1);
        LD(s1, s2, 0);
    }
    return s1;
}

void jump_to_next(dynarec_rv64_t* dyn, uintptr_t ip, int reg, int ninst, int is32bits)
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
        uintptr_t p = getJumpTableAddress64(ip);
        MAYUSE(p);
        GETIP_(ip, x3);
        if (dyn->need_reloc) AddRelocTable64JmpTbl(dyn, ninst, ip, STEP);
        TABLE64_(x3, p);
        LD(x2, x3, 0);
        dest = x2;
    }
    CLEARIP();
    SMEND();
#ifdef HAVE_TRACE
    JALR(xRA, dest);
#else
    JALR((dyn->insts[ninst].x64.has_callret ? xRA : xZR), dest);
#endif
}

void ret_to_epilog(dynarec_rv64_t* dyn, uintptr_t ip, int ninst, rex_t rex)
{
    MAYUSE(dyn);
    MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Ret to epilog\n");
    POP1z(xRIP);
    MVz(x1, xRIP);
    SMEND();
    if (BOX64DRENV(dynarec_callret)) {
        // pop the actual return address from RV64 stack
        LD(xRA, xSP, 0);      // native addr
        LD(x6, xSP, 8);       // x86 addr
        ADDI(xSP, xSP, 16);   // pop
        BNE(x6, xRIP, 2 * 4); // is it the right address?
        BR(xRA);
        // not the correct return address, regular jump, but purge the stack first, it's unsync now...
        LD(xSP, xEmu, offsetof(x64emu_t, xSPSave));
        ADDI(xSP, xSP, -16);
    }
    NOTEST(x2);
    int dest = indirect_lookup(dyn, ninst, rex.is32bits, x2, x3);
    BR(dest);
    CLEARIP();
}

void retn_to_epilog(dynarec_rv64_t* dyn, uintptr_t ip, int ninst, rex_t rex, int n)
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
        LD(xRA, xSP, 0);      // native addr
        LD(x6, xSP, 8);       // x86 addr
        ADDI(xSP, xSP, 16);   // pop
        BNE(x6, xRIP, 2 * 4); // is it the right address?
        BR(xRA);
        // not the correct return address, regular jump, but purge the stack first, it's unsync now...
        LD(xSP, xEmu, offsetof(x64emu_t, xSPSave));
        ADDI(xSP, xSP, -16);
    }
    NOTEST(x2);
    int dest = indirect_lookup(dyn, ninst, rex.is32bits, x2, x3);
    BR(dest);
    CLEARIP();
}

void iret_to_epilog(dynarec_rv64_t* dyn, uintptr_t ip, int ninst, int is64bits)
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
    FLAGS_ADJUST_FROM11(xFlags, xFlags, x3);

    SH(x2, xEmu, offsetof(x64emu_t, segs[_CS]));
    // clean EFLAGS
    MOV32w(x1, 0x3E7FF7);   // also masking RF
    AND(xFlags, xFlags, x1);
    ORI(xFlags, xFlags, 0x2);
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
    SH(x2, xEmu, offsetof(x64emu_t, segs[_SS]));
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

void call_c(dynarec_rv64_t* dyn, int ninst, rv64_consts_t fnc, int reg, int ret, int saveflags, int savereg, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6)
{
    MAYUSE(fnc);
    if (savereg == 0)
        savereg = x87pc;
    if (saveflags) {
        FLAGS_ADJUST_TO11(xFlags, xFlags, reg);
        SD(xFlags, xEmu, offsetof(x64emu_t, eflags));
    }
    fpu_pushcache(dyn, ninst, reg, 0);
    if (ret != -2) {
        SUBI(xSP, xSP, 16); // RV64 stack needs to be 16byte aligned
        SD(savereg, xSP, 0);
        STORE_REG(RDI);
        STORE_REG(RSI);
        STORE_REG(RDX);
        STORE_REG(RCX);
        STORE_REG(R8);
        STORE_REG(R9);
        STORE_REG(RAX);
        SD(xRIP, xEmu, offsetof(x64emu_t, ip));
    }
    TABLE64C(reg, fnc);
    MV(A0, xEmu);
    if (arg1) MV(A1, arg1);
    if (arg2) MV(A2, arg2);
    if (arg3) MV(A3, arg3);
    if (arg4) MV(A4, arg4);
    if (arg5) MV(A5, arg5);
    if (arg6) MV(A6, arg6);
    JALR(xRA, reg);
    if (ret >= 0) {
        MV(ret, A0);
    }

    // reinitialize sew
    if (dyn->vector_sew != VECTOR_SEWNA)
        vector_vsetvli(dyn, ninst, savereg, dyn->vector_sew, VECTOR_LMUL1, 1);

    LD(savereg, xSP, 0);
    ADDI(xSP, xSP, 16);
#define GO(A) \
    if (ret != x##A) { LOAD_REG(A); }
    GO(RDI);
    GO(RSI);
    GO(RDX);
    GO(RCX);
    GO(R8);
    GO(R9);
    GO(RAX);
#undef GO
    if (ret != xRIP)
        LD(xRIP, xEmu, offsetof(x64emu_t, ip));

    fpu_popcache(dyn, ninst, reg, 0);
    if (saveflags) {
        LD(xFlags, xEmu, offsetof(x64emu_t, eflags));
        FLAGS_ADJUST_FROM11(xFlags, xFlags, reg);
    }
    if (savereg != x87pc && dyn->need_x87check)
        NATIVE_RESTORE_X87PC();
    // SET_NODF();
    CLEARIP();
}

void call_n(dynarec_rv64_t* dyn, int ninst, void* fnc, int w)
{
    MAYUSE(fnc);
    fpu_pushcache(dyn, ninst, x3, 1);
    // save RSP in case there are x86 callbacks...
    SD(xRSP, xEmu, offsetof(x64emu_t, regs[_SP]));
    SD(xRBP, xEmu, offsetof(x64emu_t, regs[_BP]));
    // check if additional sextw needed
    int sextw_mask = ((w > 0 ? w : -w) >> 4) & 0b111111;
    for (int i = 0; i < 6; i++) {
        if (sextw_mask & (1 << i)) {
            SEXT_W(A0 + i, A0 + i);
        }
    }
    // native call
    if (dyn->need_reloc) {
        // fnc is indirect, to help with relocation (but PltResolver might be an issue here)
        TABLE64(x3, (uintptr_t)fnc);
        LD(x3, x3, 0);
    } else {
        TABLE64_(x3, *(uintptr_t*)fnc); // using x16 as scratch regs for call address
    }
    JALR(xRA, x3);
    // put return value in x64 regs
    if (w > 0) {
        MV(xRAX, A0);
        MV(xRDX, A1);
    }
    // all done, restore all regs

    // reinitialize sew
    if (dyn->vector_sew != VECTOR_SEWNA)
        vector_vsetvli(dyn, ninst, x3, dyn->vector_sew, VECTOR_LMUL1, 1);

    fpu_popcache(dyn, ninst, x3, 1);
    NATIVE_RESTORE_X87PC();
    // SET_NODF();
}

void grab_segdata(dynarec_rv64_t* dyn, uintptr_t addr, int ninst, int reg, int segment)
{
    (void)addr;
    int64_t j64;
    MAYUSE(j64);
    MESSAGE(LOG_DUMP, "Get %s Offset\n", (segment == _FS) ? "FS" : "GS");
    LD(reg, xEmu, offsetof(x64emu_t, segs_offs[segment]));
    MESSAGE(LOG_DUMP, "----%s Offset\n", (segment == _FS) ? "FS" : "GS");
}

int x87_stackcount(dynarec_rv64_t* dyn, int ninst, int scratch)
{
    MAYUSE(scratch);
    if (!dyn->e.x87stack)
        return 0;
    if (dyn->e.mmxcount)
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
    if (!count)
        return;
    if (dyn->e.mmxcount)
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
    if (i1 != i2) {
        MESSAGE(LOG_DUMP, "Warning, ST cache incoherent between ST%d(%d) and ST%d(%d)\n", a, i1, b, i2);
    }

    return i1;
}

// On step 1, Float/Double for ST is actually computed and back-propagated
// On step 2-3, the value is just read for inst[...].n.neocache[..]
// the reg returned is *2 for FLOAT
int x87_do_push(dynarec_rv64_t* dyn, int ninst, int s1, int t)
{
    if (dyn->e.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    dyn->e.x87stack += 1;
    dyn->e.stack += 1;
    dyn->e.stack_next += 1;
    dyn->e.stack_push += 1;
    ++dyn->e.pushed;
    if (dyn->e.poped)
        --dyn->e.poped;
    // move all regs in cache, and find a free one
    for (int j = 0; j < 24; ++j)
        if ((dyn->e.extcache[j].t == EXT_CACHE_ST_D)
            || (dyn->e.extcache[j].t == EXT_CACHE_ST_F)
            || (dyn->e.extcache[j].t == EXT_CACHE_ST_I64))
            ++dyn->e.extcache[j].n;
    int ret = -1;
    dyn->e.tags <<= 2;
    for (int i = 0; i < 8; ++i)
        if (dyn->e.x87cache[i] != -1)
            ++dyn->e.x87cache[i];
        else if (ret == -1) {
            dyn->e.x87cache[i] = 0;
            ret = dyn->e.x87reg[i] = fpu_get_reg_x87(dyn, t, 0);
            dyn->e.extcache[EXTIDX(ret)].t = X87_ST0;
        }
    if (ret == -1) {
        MESSAGE(LOG_DUMP, "Incoherent x87 stack cache, aborting\n");
        dyn->abort = 1;
    }
    return ret;
}
void x87_do_push_empty(dynarec_rv64_t* dyn, int ninst, int s1)
{
    if (dyn->e.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    dyn->e.x87stack += 1;
    dyn->e.stack += 1;
    dyn->e.stack_next += 1;
    dyn->e.stack_push += 1;
    ++dyn->e.pushed;
    if (dyn->e.poped)
        --dyn->e.poped;
    // move all regs in cache
    for (int j = 0; j < 24; ++j)
        if ((dyn->e.extcache[j].t == EXT_CACHE_ST_D)
            || (dyn->e.extcache[j].t == EXT_CACHE_ST_F)
            || (dyn->e.extcache[j].t == EXT_CACHE_ST_I64))
            ++dyn->e.extcache[j].n;
    int ret = -1;
    dyn->e.tags <<= 2;
    for (int i = 0; i < 8; ++i)
        if (dyn->e.x87cache[i] != -1)
            ++dyn->e.x87cache[i];
        else if (ret == -1)
            ret = i;
    if (ret == -1) {
        MESSAGE(LOG_DUMP, "Incoherent x87 stack cache, aborting\n");
        dyn->abort = 1;
    }
}
static void internal_x87_dopop(dynarec_rv64_t* dyn)
{
    for (int i = 0; i < 8; ++i)
        if (dyn->e.x87cache[i] != -1) {
            --dyn->e.x87cache[i];
            if (dyn->e.x87cache[i] == -1) {
                fpu_free_reg(dyn, dyn->e.x87reg[i]);
                dyn->e.x87reg[i] = -1;
            }
        }
}
static int internal_x87_dofree(dynarec_rv64_t* dyn)
{
    if (dyn->e.tags & 0b11) {
        MESSAGE(LOG_DUMP, "\t--------x87 FREED ST0, poping 1 more\n");
        return 1;
    }
    return 0;
}
void x87_do_pop(dynarec_rv64_t* dyn, int ninst, int s1)
{
    if (dyn->e.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    do {
        dyn->e.x87stack -= 1;
        dyn->e.stack_next -= 1;
        dyn->e.stack_pop += 1;
        dyn->e.tags >>= 2;
        ++dyn->e.poped;
        if (dyn->e.pushed)
            --dyn->e.pushed;
        // move all regs in cache, poping ST0
        internal_x87_dopop(dyn);
    } while (internal_x87_dofree(dyn));
}

void x87_purgecache(dynarec_rv64_t* dyn, int ninst, int next, int s1, int s2, int s3)
{
    int ret = 0;
    for (int i = 0; i < 8 && !ret; ++i)
        if (dyn->e.x87cache[i] != -1)
            ret = 1;
    if (!ret && !dyn->e.x87stack) // nothing to do
        return;
    MESSAGE(LOG_DUMP, "\tPurge %sx87 Cache and Synch Stackcount (%+d)---\n", next ? "locally " : "", dyn->e.x87stack);
    int a = dyn->e.x87stack;
    if (a != 0) {
        // reset x87stack
        if (!next)
            dyn->e.x87stack = 0;
        // Add x87stack to emu fpu_stack
        LW(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        ADDI(s2, s2, a);
        SW(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        // Sub x87stack to top, with and 7
        LW(s2, xEmu, offsetof(x64emu_t, top));
        // update tags (and top at the same time)
        SUBI(s2, s2, a);
        ANDI(s2, s2, 7);
        SW(s2, xEmu, offsetof(x64emu_t, top));
        // update tags (and top at the same time)
        LHU(s1, xEmu, offsetof(x64emu_t, fpu_tags));
        if (a > 0) {
            SLLI(s1, s1, a * 2);
        } else {
            LUI(s3, 0xffff0);
            OR(s1, s1, s3);
            SRLI(s1, s1, -a * 2);
        }
        SH(s1, xEmu, offsetof(x64emu_t, fpu_tags));
    } else {
        LW(s2, xEmu, offsetof(x64emu_t, top));
    }
    // check if free is used
    if (dyn->e.tags) {
        LH(s1, xEmu, offsetof(x64emu_t, fpu_tags));
        MOV32w(s3, dyn->e.tags);
        OR(s1, s1, s3);
        SH(s1, xEmu, offsetof(x64emu_t, fpu_tags));
    }
    if (ret != 0) {
        // --- set values
        // Get top
        // loop all cache entries
        for (int i = 0; i < 8; ++i)
            if (dyn->e.x87cache[i] != -1) {
                int st = dyn->e.x87cache[i] + dyn->e.stack_pop;
#if STEP == 1
                if (!next) { // don't force promotion here
                    // pre-apply pop, because purge happens in-between
                    extcache_promote_double(dyn, ninst, st);
                }
#endif
#if STEP == 3
                if (!next && extcache_get_current_st(dyn, ninst, st) != EXT_CACHE_ST_D) {
                    MESSAGE(LOG_DUMP, "Warning, incoherency with purged ST%d cache\n", st);
                }
#endif
                ADDI(s3, s2, dyn->e.x87cache[i]); // unadjusted count, as it's relative to real top
                ANDI(s3, s3, 7);                  // (emu->top + st)&7
                if (cpuext.zba)
                    SH3ADD(s1, s3, xEmu);
                else {
                    SLLI(s1, s3, 3);
                    ADD(s1, xEmu, s1);
                }
                switch (extcache_get_current_st(dyn, ninst, st)) {
                    case EXT_CACHE_ST_D:
                        FSD(dyn->e.x87reg[i], s1, offsetof(x64emu_t, x87)); // save the value
                        break;
                    case EXT_CACHE_ST_F:
                        FCVTDS(SCRATCH0, dyn->e.x87reg[i]);
                        FSD(SCRATCH0, s1, offsetof(x64emu_t, x87)); // save the value
                        break;
                    case EXT_CACHE_ST_I64:
                        FMVXD(s2, dyn->e.x87reg[i]);
                        FCVTDL(SCRATCH0, s2, RD_RTZ);
                        FSD(SCRATCH0, s1, offsetof(x64emu_t, x87)); // save the value
                        break;
                }
                if (!next) {
                    fpu_free_reg(dyn, dyn->e.x87reg[i]);
                    dyn->e.x87reg[i] = -1;
                    dyn->e.x87cache[i] = -1;
                    // dyn->e.stack_pop+=1; //no pop, but the purge because of barrier will have the n.barrier flags set
                }
            }
    }
    if (!next) {
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


void x87_reflectcount(dynarec_rv64_t* dyn, int ninst, int s1, int s2)
{
    // Synch top and stack count
    int a = dyn->e.x87stack;
    if (a) {
        MESSAGE(LOG_DUMP, "\tSync x87 Count of %d-----\n", a);
        // Add x87stack to emu fpu_stack
        LW(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        ADDI(s2, s2, a);
        SW(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        // Sub x87stack to top, with and 7
        LW(s2, xEmu, offsetof(x64emu_t, top));
        SUBI(s2, s2, a);
        ANDI(s2, s2, 7);
        SW(s2, xEmu, offsetof(x64emu_t, top));
        // update tags
        LH(s1, xEmu, offsetof(x64emu_t, fpu_tags));
        if (a > 0) {
            SLLI(s1, s1, a * 2);
        } else {
            MOV32w(s2, 0xffff0000);
            OR(s1, s1, s2);
            SRLI(s1, s1, -a * 2);
        }
        SH(s1, xEmu, offsetof(x64emu_t, fpu_tags));
    }
}

static void x87_reflectcache(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3)
{
    // Sync top and stack count
    int a = dyn->e.x87stack;
    if (a) {
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
        if (a > 0) {
            SLLI(s1, s1, a * 2);
        } else {
            MOV32w(s3, 0xffff0000);
            OR(s1, s1, s3);
            SRLI(s1, s1, -a * 2);
        }
        SH(s1, xEmu, offsetof(x64emu_t, fpu_tags));
    }
    int ret = 0;
    for (int i = 0; (i < 8) && (!ret); ++i)
        if (dyn->e.x87cache[i] != -1)
            ret = 1;
    if (!ret) // nothing to do
        return;
    // prepare offset to fpu => s1
    // Get top
    if (!a) {
        LW(s2, xEmu, offsetof(x64emu_t, top));
    }
    // loop all cache entries
    for (int i = 0; i < 8; ++i)
        if (dyn->e.x87cache[i] != -1) {
            ADDI(s3, s2, dyn->e.x87cache[i]);
            ANDI(s3, s3, 7); // (emu->top + i)&7
            if (cpuext.zba)
                SH3ADD(s1, s3, xEmu);
            else {
                SLLI(s1, s3, 3);
                ADD(s1, xEmu, s1);
            }
            if (extcache_get_current_st_f(dyn, dyn->e.x87cache[i]) >= 0) {
                FCVTDS(SCRATCH0, dyn->e.x87reg[i]);
                FSD(SCRATCH0, s1, offsetof(x64emu_t, x87));
            } else
                FSD(dyn->e.x87reg[i], s1, offsetof(x64emu_t, x87));
        }
}

void x87_unreflectcount(dynarec_rv64_t* dyn, int ninst, int s1, int s2)
{
    // revert top and stack count
    int a = dyn->e.x87stack;
    if (a) {
        // Sub x87stack to emu fpu_stack
        LW(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        SUBI(s2, s2, a);
        SW(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        // Add x87stack to top, with and 7
        LW(s2, xEmu, offsetof(x64emu_t, top));
        ADDI(s2, s2, a);
        ANDI(s2, s2, 7);
        SW(s2, xEmu, offsetof(x64emu_t, top));
        // update tags
        LH(s1, xEmu, offsetof(x64emu_t, fpu_tags));
        if (a > 0) {
            MOV32w(s2, 0xffff0000);
            OR(s1, s1, s2);
            SRLI(s1, s1, a * 2);
        } else {
            SLLI(s1, s1, -a * 2);
        }
        SH(s1, xEmu, offsetof(x64emu_t, fpu_tags));
    }
}

int x87_get_current_cache(dynarec_rv64_t* dyn, int ninst, int st, int t)
{
    // search in cache first
    for (int i = 0; i < 8; ++i) {
        if (dyn->e.x87cache[i] == st) {
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
        assert(dyn->e.x87cache[i] < 8);
    }
    return -1;
}

int x87_get_cache(dynarec_rv64_t* dyn, int ninst, int populate, int s1, int s2, int st, int t)
{
    if (dyn->e.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    int ret = x87_get_current_cache(dyn, ninst, st, t);
    if (ret != -1)
        return ret;
    MESSAGE(LOG_DUMP, "\tCreate %sx87 Cache for ST%d\n", populate ? "and populate " : "", st);
    // get a free spot
    for (int i = 0; (i < 8) && (ret == -1); ++i)
        if (dyn->e.x87cache[i] == -1)
            ret = i;
    // found, setup and grab the value
    dyn->e.x87cache[ret] = st;
    dyn->e.x87reg[ret] = fpu_get_reg_x87(dyn, EXT_CACHE_ST_D, st);
    if (populate) {
        LW(s2, xEmu, offsetof(x64emu_t, top));
        int a = st - dyn->e.x87stack;
        if (a) {
            ADDI(s2, s2, a);
            ANDI(s2, s2, 7);
        }
        if (cpuext.zba)
            SH3ADD(s1, s2, xEmu);
        else {
            SLLI(s2, s2, 3);
            ADD(s1, xEmu, s2);
        }
        FLD(dyn->e.x87reg[ret], s1, offsetof(x64emu_t, x87));
    }
    MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);

    return ret;
}
int x87_get_extcache(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int st)
{
    for (int ii = 0; ii < 24; ++ii)
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
    for (int i = 0; (i < 8) && (ret == -1); ++i)
        if (dyn->e.x87cache[i] == st)
            ret = i;
    if (ret == -1) // nothing to do
        return;
    MESSAGE(LOG_DUMP, "\tRefresh x87 Cache for ST%d\n", st);
    const int reg = dyn->e.x87reg[ret];
    // prepare offset to fpu => s1
    // Get top
    LW(s2, xEmu, offsetof(x64emu_t, top));
    // Update
    int a = st - dyn->e.x87stack;
    if (a) {
        ADDI(s2, s2, a);
        ANDI(s2, s2, 7); // (emu->top + i)&7
    }
    if (cpuext.zba)
        SH3ADD(s1, s2, xEmu);
    else {
        SLLI(s2, s2, 3);
        ADD(s1, xEmu, s2);
    }
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
    for (int i = 0; (i < 8) && (ret == -1); ++i)
        if (dyn->e.x87cache[i] == st)
            ret = i;
    if (ret == -1) // nothing to do
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
    if (a) {
        ADDI(s2, s2, a);
        ANDI(s2, s2, 7); // (emu->top + i)&7
    }
    if (cpuext.zba)
        SH3ADD(s1, s2, xEmu);
    else {
        SLLI(s2, s2, 3);
        ADD(s1, xEmu, s2);
    }
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
    if (dyn->e.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    // search in cache first
    for (int i = 0; i < 8; ++i)
        if (dyn->e.x87cache[i] == st) {
            // refresh the value
            MESSAGE(LOG_DUMP, "\tRefresh x87 Cache for ST%d\n", st);
#if STEP == 1
            if (dyn->e.extcache[EXTIDX(dyn->e.x87reg[i])].t == EXT_CACHE_ST_F
                || dyn->e.extcache[EXTIDX(dyn->e.x87reg[i])].t == EXT_CACHE_ST_I64)
                extcache_promote_double(dyn, ninst, st);
#endif
            LW(s2, xEmu, offsetof(x64emu_t, top));
            int a = st - dyn->e.x87stack;
            if (a) {
                ADDI(s2, s2, a);
                AND(s2, s2, 7);
            }
            if (cpuext.zba)
                SH3ADD(s1, s2, xEmu);
            else {
                SLLI(s2, s2, 3);
                ADD(s1, xEmu, s2);
            }
            FLD(dyn->e.x87reg[i], s1, offsetof(x64emu_t, x87));
            MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);
            // ok
            return;
        }
    // Was not in the cache? creating it....
    MESSAGE(LOG_DUMP, "\tCreate x87 Cache for ST%d\n", st);
    // get a free spot
    int ret = -1;
    for (int i = 0; (i < 8) && (ret == -1); ++i)
        if (dyn->e.x87cache[i] == -1)
            ret = i;
    // found, setup and grab the value
    dyn->e.x87cache[ret] = st;
    dyn->e.x87reg[ret] = fpu_get_reg_x87(dyn, EXT_CACHE_ST_D, st);
    LW(s2, xEmu, offsetof(x64emu_t, top));
    int a = st - dyn->e.x87stack;
    ADDI(s2, s2, a);
    ANDI(s2, s2, 7); // (emu->top + i)&7
    if (cpuext.zba)
        SH3ADD(s1, s2, xEmu);
    else {
        SLLI(s2, s2, 3);
        ADD(s1, xEmu, s2);
    }
    FLD(dyn->e.x87reg[ret], s1, offsetof(x64emu_t, x87));
    MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);
}

void x87_free(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int st)
{
    int ret = -1;
    for (int i = 0; (i < 8) && (ret == -1); ++i)
        if (dyn->e.x87cache[i] == st)
            ret = i;
    MESSAGE(LOG_DUMP, "\tFFREE%s x87 Cache for ST%d\n", (ret != -1) ? " (and Forget)" : "", st);
    if (ret != -1) {
        const int reg = dyn->e.x87reg[ret];
#if STEP == 1
        if (dyn->e.extcache[reg].t == EXT_CACHE_ST_F || dyn->e.extcache[reg].t == EXT_CACHE_ST_I64)
            extcache_promote_double(dyn, ninst, st);
#endif
        // Get top
        LW(s2, xEmu, offsetof(x64emu_t, top));
        // Update
        int ast = st - dyn->e.x87stack;
        if (ast) {
            if (ast > 0) {
                ADDI(s2, s2, ast);
            } else {
                SUBI(s2, s2, -ast);
            }
            ANDI(s2, s2, 7); // (emu->top + i)&7
        }
        if (cpuext.zba)
            SH3ADD(s1, s2, xEmu);
        else {
            SLLI(s2, s2, 3);
            ADD(s1, xEmu, s2);
        }
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
        if (ast) {
            if (ast > 0) {
                ADDI(s2, s2, ast);
            } else {
                SUBI(s2, s2, -ast);
            }
            ANDI(s2, s2, 7); // (emu->top + i)&7
        }
    }
    // add mark in the freed array
    dyn->e.tags |= 0b11 << (st * 2);
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
    dyn->e.combined1 = a;
    dyn->e.combined2 = b;
}

// Set rounding according to cw flags, return reg to restore flags
int x87_setround(dynarec_rv64_t* dyn, int ninst, int s1, int s2)
{
    MAYUSE(dyn);
    MAYUSE(ninst);
    MAYUSE(s1);
    MAYUSE(s2);
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
    FSRM(s1, s1); // exange RM with current
    return s1;
}

// Set rounding according to mxcsr flags, return reg to restore flags
int sse_setround(dynarec_rv64_t* dyn, int ninst, int s1, int s2)
{
    MAYUSE(dyn);
    MAYUSE(ninst);
    MAYUSE(s1);
    MAYUSE(s2);
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
    FSRM(s1, s1); // exange RM with current
    return s1;
}

// Restore round flag, destroy s1 doing so
void x87_restoreround(dynarec_rv64_t* dyn, int ninst, int s1)
{
    MAYUSE(dyn);
    MAYUSE(ninst);
    MAYUSE(s1);
    FSRM(s1, s1); // put back fpscr
}

// MMX helpers
static int isx87Empty(dynarec_rv64_t* dyn)
{
    for (int i = 0; i < 8; ++i)
        if (dyn->e.x87cache[i] != -1)
            return 0;
    return 1;
}

// forget ext register for a MMX reg, does nothing if the regs is not loaded
void mmx_forget_reg(dynarec_rv64_t* dyn, int ninst, int s1, int a)
{
    if (dyn->e.mmxcache[a].v == -1)
        return;
    if (dyn->e.mmxcache[a].vector) {
        SET_ELEMENT_WIDTH(s1, VECTOR_SEW64, 1);
        VFMV_F_S(dyn->e.mmxcache[a].reg, dyn->e.mmxcache[a].reg);
    }
    FSD(dyn->e.mmxcache[a].reg, xEmu, offsetof(x64emu_t, mmx[a]));
    fpu_free_reg(dyn, dyn->e.mmxcache[a].reg);
    dyn->e.mmxcache[a].v = -1;
    return;
}

static void mmx_transfer_reg(dynarec_rv64_t* dyn, int ninst, int s1, int a)
{
    if (dyn->e.mmxcache[a].v == -1)
        return;

    SET_ELEMENT_WIDTH(s1, VECTOR_SEW64, 1);
    if (dyn->e.mmxcache[a].vector) {
        VFMV_F_S(dyn->e.mmxcache[a].reg, dyn->e.mmxcache[a].reg);
    } else {
        VFMV_S_F(dyn->e.mmxcache[a].reg, dyn->e.mmxcache[a].reg);
    }
    dyn->e.mmxcache[a].vector = 1 - dyn->e.mmxcache[a].vector;
    dyn->e.extcache[EXTIDX(dyn->e.mmxcache[a].reg)].t = dyn->e.mmxcache[a].vector ? EXT_CACHE_MMV : EXT_CACHE_MM;
    return;
}

// get float register for a MMX reg, create the entry if needed
int mmx_get_reg(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int a)
{
    if (!dyn->e.x87stack && isx87Empty(dyn))
        x87_purgecache(dyn, ninst, 0, s1, s2, s3);
    if (dyn->e.mmxcache[a].v != -1) {
        if (dyn->e.mmxcache[a].vector) {
            mmx_transfer_reg(dyn, ninst, s1, a);
        }
        return dyn->e.mmxcache[a].reg;
    }

    ++dyn->e.mmxcount;
    dyn->e.mmxcache[a].reg = fpu_get_reg_emm(dyn, EXT_CACHE_MM, a);
    dyn->e.mmxcache[a].vector = 0;
    FLD(dyn->e.mmxcache[a].reg, xEmu, offsetof(x64emu_t, mmx[a]));
    return dyn->e.mmxcache[a].reg;
}

// get vector register for a MMX reg, create the entry if needed
int mmx_get_reg_vector(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int a)
{
    if (!dyn->e.x87stack && isx87Empty(dyn))
        x87_purgecache(dyn, ninst, 0, s1, s2, s3);
    if (dyn->e.mmxcache[a].v != -1) {
        if (!dyn->e.mmxcache[a].vector) {
            mmx_transfer_reg(dyn, ninst, s1, a);
        }
        return dyn->e.mmxcache[a].reg;
    }

    ++dyn->e.mmxcount;
    dyn->e.mmxcache[a].reg = fpu_get_reg_emm(dyn, EXT_CACHE_MMV, a);
    dyn->e.mmxcache[a].vector = 1;
    FLD(dyn->e.mmxcache[a].reg, xEmu, offsetof(x64emu_t, mmx[a]));
    SET_ELEMENT_WIDTH(s1, VECTOR_SEW64, 1);
    VFMV_S_F(dyn->e.mmxcache[a].reg, dyn->e.mmxcache[a].reg);
    return dyn->e.mmxcache[a].reg;
}

// get float register for a MMX reg, but don't try to synch it if it needed to be created
int mmx_get_reg_empty(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int a)
{
    if (!dyn->e.x87stack && isx87Empty(dyn))
        x87_purgecache(dyn, ninst, 0, s1, s2, s3);
    if (dyn->e.mmxcache[a].v != -1) {
        dyn->e.mmxcache[a].vector = 0;
        dyn->e.extcache[EXTIDX(dyn->e.mmxcache[a].reg)].t = EXT_CACHE_MM;
        return dyn->e.mmxcache[a].reg;
    }

    ++dyn->e.mmxcount;
    dyn->e.mmxcache[a].vector = 0;
    return dyn->e.mmxcache[a].reg = fpu_get_reg_emm(dyn, EXT_CACHE_MM, a);
}

// get vector register for a MMX reg, but don't try to synch it if it needed to be created
int mmx_get_reg_empty_vector(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int a)
{
    if (!dyn->e.x87stack && isx87Empty(dyn))
        x87_purgecache(dyn, ninst, 0, s1, s2, s3);
    if (dyn->e.mmxcache[a].v != -1) {
        dyn->e.mmxcache[a].vector = 1;
        dyn->e.extcache[EXTIDX(dyn->e.mmxcache[a].reg)].t = EXT_CACHE_MMV;
        return dyn->e.mmxcache[a].reg;
    }

    ++dyn->e.mmxcount;
    dyn->e.mmxcache[a].vector = 1;
    return dyn->e.mmxcache[a].reg = fpu_get_reg_emm(dyn, EXT_CACHE_MMV, a);
}

// purge the MMX cache only(needs 3 scratch registers)
void mmx_purgecache(dynarec_rv64_t* dyn, int ninst, int next, int s1)
{
    if (!dyn->e.mmxcount)
        return;
    if (!next)
        dyn->e.mmxcount = 0;
    int old = -1;
    for (int i = 0; i < 8; ++i) {
        if (dyn->e.mmxcache[i].v != -1) {
            if (old == -1) {
                MESSAGE(LOG_DUMP, "\tPurge %sMMX Cache ------\n", next ? "locally " : "");
                ++old;
            }
            if (dyn->e.mmxcache[i].vector) {
                SET_ELEMENT_WIDTH(s1, VECTOR_SEW64, 1);
                VFMV_F_S(dyn->e.mmxcache[i].reg, dyn->e.mmxcache[i].reg);
            }
            FSD(dyn->e.mmxcache[i].reg, xEmu, offsetof(x64emu_t, mmx[i]));
            if (!next) {
                fpu_free_reg(dyn, dyn->e.mmxcache[i].reg);
                dyn->e.mmxcache[i].v = -1;
            }
        }
    }
    if (old != -1) {
        MESSAGE(LOG_DUMP, "\t------ Purge MMX Cache\n");
    }
}

static void mmx_reflectcache(dynarec_rv64_t* dyn, int ninst, int s1)
{
    for (int i = 0; i < 8; ++i) {
        if (dyn->e.mmxcache[i].v != -1) {
            if (dyn->e.mmxcache[i].vector) {
                SET_ELEMENT_WIDTH(s1, VECTOR_SEW64, 1);
                VFMV_F_S(dyn->e.mmxcache[i].reg, dyn->e.mmxcache[i].reg);
            }
            FSD(dyn->e.mmxcache[i].reg, xEmu, offsetof(x64emu_t, mmx[i]));
        }
    }
}

// SSE / SSE2 helpers
// get ext register for a SSE reg, create the entry if needed
int sse_get_reg(dynarec_rv64_t* dyn, int ninst, int s1, int a, int single)
{
    if (dyn->e.ssecache[a].v != -1) {
        if (dyn->e.ssecache[a].vector == 1) {
            // it's in the fpu, forget it first...
            sse_forget_reg_vector(dyn, ninst, s1, a);
            // update olds after the forget...
            dyn->e.olds[a].changed = 1;
            dyn->e.olds[a].purged = 0;
            return sse_get_reg(dyn, ninst, s1, a, single);
        }
        // forget / reload if change of size
        if (dyn->e.ssecache[a].single != single) {
            sse_forget_reg(dyn, ninst, s1, a);
            // update olds after the forget...
            dyn->e.olds[a].changed = 1;
            dyn->e.olds[a].purged = 0;
            dyn->e.olds[a].type = 1 - single;
            return sse_get_reg(dyn, ninst, s1, a, single);
        }
        return dyn->e.ssecache[a].reg;
    }
    dyn->e.ssecache[a].reg = fpu_get_reg_xmm(dyn, single ? EXT_CACHE_SS : EXT_CACHE_SD, a);
    int ret = dyn->e.ssecache[a].reg;
    dyn->e.ssecache[a].single = single;
    dyn->e.ssecache[a].vector = 0;
    if (dyn->e.ssecache[a].single)
        FLW(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
    else
        FLD(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
    return ret;
}

// get ext register for a SSE reg, but don't try to synch it if it needed to be created
int sse_get_reg_empty(dynarec_rv64_t* dyn, int ninst, int s1, int a, int single)
{
    if (dyn->e.ssecache[a].v != -1) {
        if (dyn->e.ssecache[a].vector == 1) {
            // it's in the vpu, forget it first...
            sse_forget_reg_vector(dyn, ninst, s1, a);
            // update olds after the forget...
            dyn->e.olds[a].changed = 1;
            dyn->e.olds[a].purged = 0;
            return sse_get_reg_empty(dyn, ninst, s1, a, single);
        }

        if (dyn->e.ssecache[a].single != single) {
            if (single) {
                // writing back the float
                FSW(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
                // there is no need to clear upper bits, it's cleared manually when needed.
            }
            dyn->e.olds[a].changed = 1;
            dyn->e.olds[a].purged = 0;
            dyn->e.olds[a].reg = EXTIDX(dyn->e.ssecache[a].reg);
            dyn->e.olds[a].type = 1 - single;
            dyn->e.ssecache[a].single = single;
            dyn->e.ssecache[a].vector = 0;
            dyn->e.extcache[EXTIDX(dyn->e.ssecache[a].reg)].t = single ? EXT_CACHE_SS : EXT_CACHE_SD;
        }
        return dyn->e.ssecache[a].reg;
    }
    dyn->e.ssecache[a].reg = fpu_get_reg_xmm(dyn, single ? EXT_CACHE_SS : EXT_CACHE_SD, a);
    dyn->e.ssecache[a].single = single;
    dyn->e.ssecache[a].vector = 0;
    return dyn->e.ssecache[a].reg;
}

// get an ext register for an SSE reg which changes size, with single or not AFTER the change
int sse_get_reg_size_changed(dynarec_rv64_t* dyn, int ninst, int s1, int a, int single)
{
    if (dyn->e.ssecache[a].v != -1) {
        if (dyn->e.ssecache[a].vector == 1) {
            // it's in the vpu, forget it first...
            sse_forget_reg_vector(dyn, ninst, s1, a);
            // update olds after the forget...
            dyn->e.olds[a].changed = 1;
            dyn->e.olds[a].purged = 0;
            return sse_get_reg_size_changed(dyn, ninst, s1, a, single);
        }

        // forget / reload if change of size
        if (dyn->e.ssecache[a].single == single) {
            sse_forget_reg(dyn, ninst, s1, a);
            return sse_get_reg_size_changed(dyn, ninst, s1, a, single);
        }
        dyn->e.olds[a].changed = 1;
        dyn->e.olds[a].purged = 0;
        dyn->e.olds[a].type = !single;
        dyn->e.ssecache[a].single = single;
        dyn->e.ssecache[a].vector = 0;
        dyn->e.extcache[EXTIDX(dyn->e.ssecache[a].reg)].t = single ? EXT_CACHE_SS : EXT_CACHE_SD;
        return dyn->e.ssecache[a].reg;
    }
    dyn->e.ssecache[a].reg = fpu_get_reg_xmm(dyn, single ? EXT_CACHE_SS : EXT_CACHE_SD, a);
    int ret = dyn->e.ssecache[a].reg;
    dyn->e.ssecache[a].single = single;
    dyn->e.ssecache[a].vector = 0;
    if (!single) // load happens before size changed
        FLW(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
    else
        FLD(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
    return ret;
}

// forget ext register for a SSE reg, does nothing if the regs is not loaded
void sse_forget_reg(dynarec_rv64_t* dyn, int ninst, int s1, int a)
{
    if (dyn->e.ssecache[a].v == -1)
        return;
    if (dyn->e.ssecache[a].vector == 1)
        return sse_forget_reg_vector(dyn, ninst, s1, a);
    if (dyn->e.ssecache[a].single)
        FSW(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
    else
        FSD(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
    fpu_free_reg(dyn, dyn->e.ssecache[a].reg);
    dyn->e.olds[a].changed = 0;
    dyn->e.olds[a].purged = 1;
    dyn->e.olds[a].reg = EXTIDX(dyn->e.ssecache[a].reg);
    dyn->e.olds[a].type = dyn->e.ssecache[a].single;
    dyn->e.ssecache[a].v = -1;
    return;
}

// get rvv register for a SSE reg, create the entry if needed
int sse_get_reg_vector(dynarec_rv64_t* dyn, int ninst, int s1, int a, int forwrite, int sew)
{
    if (dyn->e.ssecache[a].v != -1) {
        if (dyn->e.ssecache[a].vector == 0) {
            // it's in the fpu, forget it first...
            sse_forget_reg(dyn, ninst, s1, a);
            // update olds after the forget...
            dyn->e.olds[a].changed = 1;
            dyn->e.olds[a].purged = 0;
            return sse_get_reg_vector(dyn, ninst, s1, a, forwrite, sew);
        }

        if (forwrite) {
            dyn->e.olds[a].changed = 1;
            dyn->e.olds[a].purged = 0;
            dyn->e.olds[a].reg = EXTIDX(dyn->e.ssecache[a].reg);
            dyn->e.olds[a].type = EXT_CACHE_OLD_XMMW;
            dyn->e.ssecache[a].write = 1;  // update only if forwrite
            dyn->e.ssecache[a].single = 0; // just to be clean
            dyn->e.extcache[EXTIDX(dyn->e.ssecache[a].reg)].t = EXT_CACHE_XMMW;
        }
        return dyn->e.ssecache[a].reg;
    }
    dyn->e.ssecache[a].reg = fpu_get_reg_xmm(dyn, forwrite ? EXT_CACHE_XMMW : EXT_CACHE_XMMR, a);
    int ret = dyn->e.ssecache[a].reg;
    dyn->e.ssecache[a].write = forwrite;
    dyn->e.ssecache[a].vector = 1;
    dyn->e.ssecache[a].single = 0; // just to be clean
    ADDI(s1, xEmu, offsetof(x64emu_t, xmm[a]));
    VLE_V(ret, s1, sew, VECTOR_UNMASKED, VECTOR_NFIELD1);
    return ret;
}

// get rvv register for an SSE reg, but don't try to synch it if it needed to be created
int sse_get_reg_empty_vector(dynarec_rv64_t* dyn, int ninst, int s1, int a)
{
    if (dyn->e.ssecache[a].v != -1) {
        if (dyn->e.ssecache[a].vector == 0) {
            // it's in the fpu, forget it first...
            sse_forget_reg(dyn, ninst, s1, a);
            // update olds after the forget...
            dyn->e.olds[a].changed = 1;
            dyn->e.olds[a].purged = 0;
            return sse_get_reg_empty_vector(dyn, ninst, s1, a);
        }
        dyn->e.olds[a].changed = 1;
        dyn->e.olds[a].purged = 0;
        dyn->e.olds[a].reg = EXTIDX(dyn->e.ssecache[a].reg);
        dyn->e.olds[a].type = EXT_CACHE_OLD_XMMW;
        dyn->e.ssecache[a].vector = 1;
        dyn->e.ssecache[a].write = 1;
        dyn->e.ssecache[a].single = 0; // just to be clean
        dyn->e.extcache[EXTIDX(dyn->e.ssecache[a].reg)].t = EXT_CACHE_XMMW;
        return dyn->e.ssecache[a].reg;
    }
    dyn->e.ssecache[a].reg = fpu_get_reg_xmm(dyn, EXT_CACHE_XMMW, a);
    dyn->e.ssecache[a].vector = 1;
    dyn->e.ssecache[a].single = 0; // just to be clean
    dyn->e.ssecache[a].write = 1;  // it will be write...
    return dyn->e.ssecache[a].reg;
}

// forget rvv register for a SSE reg, does nothing if the regs is not loaded
void sse_forget_reg_vector(dynarec_rv64_t* dyn, int ninst, int s1, int a)
{
    if (dyn->e.ssecache[a].v == -1)
        return;
    if (dyn->e.ssecache[a].vector == 0)
        return sse_forget_reg(dyn, ninst, s1, a);
    if (dyn->e.extcache[EXTIDX(dyn->e.ssecache[a].reg)].t == EXT_CACHE_XMMW) {
        SET_ELEMENT_WIDTH(s1, VECTOR_SEWANY, 1);
        ADDI(s1, xEmu, offsetof(x64emu_t, xmm[a]));
        VSE_V(dyn->e.ssecache[a].reg, s1, dyn->vector_eew, VECTOR_UNMASKED, VECTOR_NFIELD1);
    }
    fpu_free_reg(dyn, dyn->e.ssecache[a].reg);
    dyn->e.olds[a].changed = 0;
    dyn->e.olds[a].purged = 1;
    dyn->e.olds[a].type = dyn->e.ssecache[a].write ? EXT_CACHE_OLD_XMMW : EXT_CACHE_OLD_XMMR;
    dyn->e.olds[a].reg = EXTIDX(dyn->e.ssecache[a].reg);
    dyn->e.ssecache[a].v = -1;
    return;
}

// purge the SSE cache for XMM0..XMM7 (to use before function native call)
void sse_purge07cache(dynarec_rv64_t* dyn, int ninst, int s1)
{
    int old = -1;
    for (int i = 0; i < 8; ++i)
        if (dyn->e.ssecache[i].v != -1) {
            if (old == -1) {
                MESSAGE(LOG_DUMP, "\tPurge XMM0..7 Cache ------\n");
                ++old;
            }
            if (dyn->e.ssecache[i].vector) {
                SET_ELEMENT_WIDTH(s1, VECTOR_SEWANY, 0);
                ADDI(s1, xEmu, offsetof(x64emu_t, xmm[i]));
                VSE_V(dyn->e.ssecache[i].reg, s1, dyn->vector_eew, VECTOR_UNMASKED, VECTOR_NFIELD1);
            } else if (dyn->e.ssecache[i].single)
                FSW(dyn->e.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            else
                FSD(dyn->e.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            fpu_free_reg(dyn, dyn->e.ssecache[i].reg);
            dyn->e.ssecache[i].v = -1;
        }
    if (old != -1) {
        MESSAGE(LOG_DUMP, "\t------ Purge XMM0..7 Cache\n");
    }
}

// purge the SSE cache only
static void sse_purgecache(dynarec_rv64_t* dyn, int ninst, int next, int s1)
{
    int old = -1;
    for (int i = 0; i < 16; ++i)
        if (dyn->e.ssecache[i].v != -1) {
            if (old == -1) {
                MESSAGE(LOG_DUMP, "\tPurge %sSSE Cache ------\n", next ? "locally " : "");
                ++old;
            }
            if (dyn->e.ssecache[i].vector) {
                if (dyn->e.ssecache[i].write) {
                    SET_ELEMENT_WIDTH(s1, VECTOR_SEWANY, 0);
                    ADDI(s1, xEmu, offsetof(x64emu_t, xmm[i]));
                    VSE_V(dyn->e.ssecache[i].reg, s1, dyn->vector_eew, VECTOR_UNMASKED, VECTOR_NFIELD1);
                }
            } else if (dyn->e.ssecache[i].single)
                FSW(dyn->e.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            else
                FSD(dyn->e.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            if (!next) {
                fpu_free_reg(dyn, dyn->e.ssecache[i].reg);
                dyn->e.olds[i].changed = 0;
                dyn->e.olds[i].purged = 1;
                dyn->e.olds[i].type = dyn->e.ssecache[i].vector ? (dyn->e.ssecache[i].write ? EXT_CACHE_OLD_XMMW : EXT_CACHE_OLD_XMMR) : dyn->e.ssecache[i].single;
                dyn->e.olds[i].reg = dyn->e.ssecache[i].reg;
                dyn->e.ssecache[i].v = -1;
            }
        }

    if (old != -1) {
        MESSAGE(LOG_DUMP, "\t------ Purge SSE Cache\n");
    }
}

static void sse_reflectcache(dynarec_rv64_t* dyn, int ninst, int s1)
{
    for (int i = 0; i < 16; ++i)
        if (dyn->e.ssecache[i].v != -1) {
            if (dyn->e.ssecache[i].vector) {
                SET_ELEMENT_WIDTH(s1, VECTOR_SEWANY, 0);
                ADDI(s1, xEmu, offsetof(x64emu_t, xmm[i]));
                VSE_V(dyn->e.ssecache[i].reg, s1, dyn->vector_eew, VECTOR_UNMASKED, VECTOR_NFIELD1);
            } else if (dyn->e.ssecache[i].single)
                FSW(dyn->e.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            else
                FSD(dyn->e.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
        }
}

void sse_reflect_reg(dynarec_rv64_t* dyn, int ninst, int s1, int a)
{
    if (dyn->e.ssecache[a].v == -1)
        return;
    if (dyn->e.ssecache[a].vector) {
        SET_ELEMENT_WIDTH(s1, VECTOR_SEWANY, 0);
        ADDI(s1, xEmu, offsetof(x64emu_t, xmm[a]));
        VSE_V(dyn->e.ssecache[a].reg, s1, dyn->vector_eew, VECTOR_UNMASKED, VECTOR_NFIELD1);
    } else if (dyn->e.ssecache[a].single)
        FSW(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
    else
        FSD(dyn->e.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
}

void fpu_pushcache(dynarec_rv64_t* dyn, int ninst, int s1, int not07)
{
    // for float registers, we might lost f0..f7, f10..f17 and f28..f31, that means
    // for extcache, we're saving 0..7 (SSE), 18..23 (MMX).
    int start = not07 ? 8 : 0;
    int n = 0;
    for (int i = start; i < 8; i++)
        if (dyn->e.ssecache[i].v != -1 && !dyn->e.ssecache[i].vector) ++n;
    if (n) {
        MESSAGE(LOG_DUMP, "\tPush (float) XMM Cache (%d)------\n", n);
        for (int i = start; i < 8; ++i)
            if (dyn->e.ssecache[i].v != -1) {
                if (dyn->e.ssecache[i].vector)
                    continue;
                else if (dyn->e.ssecache[i].single)
                    FSW(dyn->e.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
                else
                    FSD(dyn->e.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            }
        MESSAGE(LOG_DUMP, "\t------- Push (float) XMM Cache (%d)\n", n);
    }
    n = 0;
    for (int i = 18; i < 24; ++i)
        if (dyn->e.extcache[i].v != 0) ++n;
    if (n) {
        MESSAGE(LOG_DUMP, "\tPush (float) x87/MMX Cache (%d)------\n", n);
        SUBI(xSP, xSP, 8 * ((n + 1) & ~1));
        int p = 0;
        for (int i = 18; i < 24; ++i)
            if (dyn->e.extcache[i].v != 0) {
                switch (dyn->e.extcache[i].t) {
                    case EXT_CACHE_ST_F:
                    case EXT_CACHE_SS:
                        FSW(EXTREG(i), xSP, p * 8);
                        break;
                    default:
                        FSD(EXTREG(i), xSP, p * 8);
                        break;
                };
                ++p;
            }
        MESSAGE(LOG_DUMP, "\t------- Push (float) x87/MMX Cache (%d)\n", n);
    }

    // for vector registers, we might lost all of them, that means for extcache,
    n = 0;
    for (int i = start; i < 16; i++)
        if (dyn->e.ssecache[i].v != -1 && dyn->e.ssecache[i].vector) ++n;
    if (n) {
        MESSAGE(LOG_DUMP, "\tPush (vector) XMM Cache (%d)------\n", n);
        for (int i = start; i < 16; ++i)
            if (dyn->e.ssecache[i].v != -1) {
                if (dyn->e.ssecache[i].vector) {
                    SET_ELEMENT_WIDTH(s1, VECTOR_SEWANY, 0);
                    ADDI(s1, xEmu, offsetof(x64emu_t, xmm[i]));
                    VSE_V(dyn->e.ssecache[i].reg, s1, dyn->vector_eew, VECTOR_UNMASKED, VECTOR_NFIELD1);
                }
            }
        MESSAGE(LOG_DUMP, "\t------- Push (vector) XMM Cache (%d)\n", n);
    }
    n = 0;
    for (int i = 0; i < 8; ++i)
        if (dyn->e.mmxcache[i].v != -1 && dyn->e.mmxcache[i].vector) ++n;
    if (n) {
        MESSAGE(LOG_DUMP, "\tPush (vector) MMX Cache (%d)------\n", n);
        for (int i = 0; i < 8; ++i)
            if (dyn->e.mmxcache[i].v != -1 && dyn->e.mmxcache[i].vector) {
                SET_ELEMENT_WIDTH(s1, VECTOR_SEW64, 0);
                VFMV_F_S(dyn->e.mmxcache[i].reg, dyn->e.mmxcache[i].reg);
                FSD(dyn->e.mmxcache[i].reg, xEmu, offsetof(x64emu_t, mmx[i]));
            }
        MESSAGE(LOG_DUMP, "\t------- Push (vector) MMX Cache (%d)\n", n);
    }
}
void fpu_popcache(dynarec_rv64_t* dyn, int ninst, int s1, int not07)
{
    // float registers
    int start = not07 ? 8 : 0;
    int n = 0;
    for (int i = start; i < 8; i++)
        if (dyn->e.ssecache[i].v != -1 && !dyn->e.ssecache[i].vector) ++n;
    if (n) {
        MESSAGE(LOG_DUMP, "\tPop (float) XMM Cache (%d)------\n", n);
        for (int i = start; i < 8; ++i)
            if (dyn->e.ssecache[i].v != -1) {
                if (dyn->e.ssecache[i].vector)
                    continue;
                else if (dyn->e.ssecache[i].single)
                    FLW(dyn->e.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
                else
                    FLD(dyn->e.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            }
        MESSAGE(LOG_DUMP, "\t------- Pop (float) XMM Cache (%d)\n", n);
    }
    n = 0;
    for (int i = 18; i < 24; ++i)
        if (dyn->e.extcache[i].v != 0) ++n;
    if (n) {
        MESSAGE(LOG_DUMP, "\tPop (float) x87/MMX Cache (%d)------\n", n);
        int p = 0;
        for (int i = 18; i < 24; ++i)
            if (dyn->e.extcache[i].v != 0) {
                switch (dyn->e.extcache[i].t) {
                    case EXT_CACHE_ST_F:
                    case EXT_CACHE_SS:
                        FLW(EXTREG(i), xSP, p * 8);
                        break;
                    default:
                        FLD(EXTREG(i), xSP, p * 8);
                        break;
                };
                ++p;
            }
        ADDI(xSP, xSP, 8 * ((n + 1) & ~1));
        MESSAGE(LOG_DUMP, "\t------- Pop (float) x87/MMX Cache (%d)\n", n);
    }

    // vector registers
    n = 0;
    for (int i = start; i < 16; i++)
        if (dyn->e.ssecache[i].v != -1 && dyn->e.ssecache[i].vector) ++n;
    if (n) {
        MESSAGE(LOG_DUMP, "\tPop (vector) XMM Cache (%d)------\n", n);
        for (int i = start; i < 16; ++i)
            if (dyn->e.ssecache[i].v != -1) {
                if (dyn->e.ssecache[i].vector) {
                    SET_ELEMENT_WIDTH(s1, VECTOR_SEWANY, 0);
                    ADDI(s1, xEmu, offsetof(x64emu_t, xmm[i]));
                    VLE_V(dyn->e.ssecache[i].reg, s1, dyn->vector_eew, VECTOR_UNMASKED, VECTOR_NFIELD1);
                }
            }
        MESSAGE(LOG_DUMP, "\t------- Pop (vector) XMM Cache (%d)\n", n);
    }
    n = 0;
    for (int i = 0; i < 8; ++i)
        if (dyn->e.mmxcache[i].v != -1 && dyn->e.mmxcache[i].vector) ++n;
    if (n) {
        MESSAGE(LOG_DUMP, "\tPop (vector) MMX Cache (%d)------\n", n);
        for (int i = 0; i < 8; ++i)
            if (dyn->e.mmxcache[i].v != -1 && dyn->e.mmxcache[i].vector) {
                SET_ELEMENT_WIDTH(s1, VECTOR_SEW64, 0);
                FLD(dyn->e.mmxcache[i].reg, xEmu, offsetof(x64emu_t, mmx[i]));
                VFMV_S_F(dyn->e.mmxcache[i].reg, dyn->e.mmxcache[i].reg);
            }
        MESSAGE(LOG_DUMP, "\t------- Pop (vector) MMX Cache (%d)\n", n);
    }
}

void fpu_purgecache(dynarec_rv64_t* dyn, int ninst, int next, int s1, int s2, int s3)
{
    x87_purgecache(dyn, ninst, next, s1, s2, s3);
    mmx_purgecache(dyn, ninst, next, s1);
    sse_purgecache(dyn, ninst, next, s1);
    if (!next)
        fpu_reset_reg(dyn);
}

int fpu_needpurgecache(dynarec_rv64_t* dyn, int ninst)
{
    // x87
    for (int i = 0; i < 8; ++i)
        if (dyn->e.x87cache[i] != -1)
            return 1;
    // mmx
    if (dyn->e.mmxcount) return 1;
    // sse
    for (int i = 0; i < 16; ++i)
        if (dyn->e.ssecache[i].v != -1) return 1;
    return 0;
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
                case EXT_CACHE_XMMR:
                    if (t == EXT_CACHE_XMMW)
                        return i;
                    break;
                case EXT_CACHE_XMMW:
                    if (t == EXT_CACHE_XMMR)
                        return i;
                    break;
            }
        }
    }
    return -1;
}

static void swapCache(dynarec_rv64_t* dyn, int ninst, int i, int j, extcache_t* cache)
{
    if (i == j) return;

    if (cache->extcache[i].t == EXT_CACHE_XMMR || cache->extcache[i].t == EXT_CACHE_XMMW
        || cache->extcache[j].t == EXT_CACHE_XMMR || cache->extcache[j].t == EXT_CACHE_XMMW) {
        int reg_i = EXTREG(i);
        int reg_j = EXTREG(j);
        if (!cache->extcache[i].v) {
            // a mov is enough, no need to swap
            MESSAGE(LOG_DUMP, "\t  - Moving %d <- %d\n", i, j);
            VMV_V_V(reg_i, reg_j);
            cache->extcache[i].v = cache->extcache[j].v;
            cache->extcache[j].v = 0;
            return;
        }
        // SWAP
        ext_cache_t tmp;
        MESSAGE(LOG_DUMP, "\t  - Swapping %d <-> %d\n", i, j);
        VXOR_VV(reg_i, reg_i, reg_j, VECTOR_UNMASKED);
        VXOR_VV(reg_j, reg_i, reg_j, VECTOR_UNMASKED);
        VXOR_VV(reg_i, reg_i, reg_j, VECTOR_UNMASKED);
        tmp.v = cache->extcache[i].v;
        cache->extcache[i].v = cache->extcache[j].v;
        cache->extcache[j].v = tmp.v;
        return;
    }

    int reg_i = EXTREG(i);
    int reg_j = EXTREG(j);
    int i_single = cache->extcache[i].t == EXT_CACHE_SS || cache->extcache[i].t == EXT_CACHE_ST_F;
    int j_single = cache->extcache[j].t == EXT_CACHE_SS || cache->extcache[j].t == EXT_CACHE_ST_F;

    if (!cache->extcache[i].v) {
        // a mov is enough, no need to swap
        MESSAGE(LOG_DUMP, "\t  - Moving %d <- %d\n", i, j);
        if (j_single) {
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
#define SCRATCH 0 // f0 is not used anywhere else
    FMV(SCRATCH, reg_i, i_single);
    FMV(reg_i, reg_j, j_single);
    FMV(reg_j, SCRATCH, i_single);
#undef SCRATCH
    tmp.v = cache->extcache[i].v;
    cache->extcache[i].v = cache->extcache[j].v;
    cache->extcache[j].v = tmp.v;
}

static void loadCache(dynarec_rv64_t* dyn, int ninst, int stack_cnt, int s1, int s2, int s3, int* s1_val, int* s2_val, int* s3_top, extcache_t* cache, int i, int t, int n)
{
    int reg = EXTREG(i);
    if (cache->extcache[i].v && (cache->extcache[i].t == EXT_CACHE_XMMR || cache->extcache[i].t == EXT_CACHE_XMMW)) {
        int j = i + 1;
        while (cache->extcache[j].v)
            ++j;
        MESSAGE(LOG_DUMP, "\t  - Moving away %d\n", i);
        VMV_V_V(EXTREG(j), reg);
        cache->extcache[j].v = cache->extcache[i].v;
    } else if (cache->extcache[i].v) {
        int single = 0;
        if (t == EXT_CACHE_SS || t == EXT_CACHE_ST_F)
            single = 1;
        if (cache->extcache[i].t == EXT_CACHE_SS || cache->extcache[i].t == EXT_CACHE_ST_F)
            single = 1;
        int j = i + 1;
        while (cache->extcache[j].v)
            ++j;
        MESSAGE(LOG_DUMP, "\t  - Moving away %d\n", i);
        if (single) {
            FMVS(EXTREG(j), reg);
        } else {
            FMVD(EXTREG(j), reg);
        }
        cache->extcache[j].v = cache->extcache[i].v;
    }
    switch (t) {
        case EXT_CACHE_XMMR:
        case EXT_CACHE_XMMW:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            SET_ELEMENT_WIDTH(s1, VECTOR_SEWANY, 0);
            ADDI(s1, xEmu, offsetof(x64emu_t, xmm[n]));
            VLE_V(reg, s1, dyn->vector_eew, VECTOR_UNMASKED, VECTOR_NFIELD1);
            break;
        case EXT_CACHE_SS:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            FLW(reg, xEmu, offsetof(x64emu_t, xmm[n]));
            break;
        case EXT_CACHE_SD:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            FLD(reg, xEmu, offsetof(x64emu_t, xmm[n]));
            break;
        case EXT_CACHE_MM:
        case EXT_CACHE_MMV:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            FLD(reg, xEmu, offsetof(x64emu_t, mmx[n]));
            if (t == EXT_CACHE_MMV) {
                SET_ELEMENT_WIDTH(s1, VECTOR_SEW64, 0);
                VFMV_S_F(reg, reg);
            }
            break;
        case EXT_CACHE_ST_D:
        case EXT_CACHE_ST_F:
        case EXT_CACHE_ST_I64:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            if ((*s3_top) == 0xffff) {
                LW(s3, xEmu, offsetof(x64emu_t, top));
                *s3_top = 0;
            }
            int a = n - (*s3_top) - stack_cnt;
            if (a) {
                ADDI(s3, s3, a);
                ANDI(s3, s3, 7); // (emu->top + i)&7
            }
            *s3_top += a;
            *s2_val = 0;
            if (cpuext.zba)
                SH3ADD(s2, s3, xEmu);
            else {
                SLLI(s2, s3, 3);
                ADD(s2, xEmu, s2);
            }
            FLD(reg, s2, offsetof(x64emu_t, x87));
            if (t == EXT_CACHE_ST_F) {
                FCVTSD(reg, reg);
            }
            if (t == EXT_CACHE_ST_I64) {
                FCVTLD(s1, reg, RD_RTZ);
                FMVDX(reg, s1);
            }
            break;
        case EXT_CACHE_NONE:
        case EXT_CACHE_SCR:
        default: /* nothing done */
            MESSAGE(LOG_DUMP, "\t  - ignoring %s\n", getCacheName(t, n));
            break;
    }
    cache->extcache[i].n = n;
    cache->extcache[i].t = t;
}

static void unloadCache(dynarec_rv64_t* dyn, int ninst, int stack_cnt, int s1, int s2, int s3, int* s1_val, int* s2_val, int* s3_top, extcache_t* cache, int i, int t, int n)
{
    int reg = EXTREG(i);
    switch (t) {
        case EXT_CACHE_XMMR:
            MESSAGE(LOG_DUMP, "\t  - ignoring %s\n", getCacheName(t, n));
            break;
        case EXT_CACHE_XMMW:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            SET_ELEMENT_WIDTH(s1, VECTOR_SEWANY, 0);
            ADDI(s1, xEmu, offsetof(x64emu_t, xmm[n]));
            VSE_V(reg, s1, dyn->vector_eew, VECTOR_UNMASKED, VECTOR_NFIELD1);
            break;
        case EXT_CACHE_SS:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            FSW(reg, xEmu, offsetof(x64emu_t, xmm[n]));
            break;
        case EXT_CACHE_SD:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            FSD(reg, xEmu, offsetof(x64emu_t, xmm[n]));
            break;
        case EXT_CACHE_MM:
        case EXT_CACHE_MMV:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            if (t == EXT_CACHE_MMV) {
                SET_ELEMENT_WIDTH(s1, VECTOR_SEW64, 0);
                VFMV_F_S(reg, reg);
            }
            FSD(reg, xEmu, offsetof(x64emu_t, mmx[n]));
            break;
        case EXT_CACHE_ST_D:
        case EXT_CACHE_ST_F:
        case EXT_CACHE_ST_I64:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            if ((*s3_top) == 0xffff) {
                LW(s3, xEmu, offsetof(x64emu_t, top));
                *s3_top = 0;
            }
            int a = n - (*s3_top) - stack_cnt;
            if (a) {
                ADDI(s3, s3, a);
                ANDI(s3, s3, 7);
            }
            *s3_top += a;
            if (cpuext.zba)
                SH3ADD(s2, s3, xEmu);
            else {
                SLLI(s2, s3, 3);
                ADD(s2, xEmu, s2);
            }
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
        default: /* nothing done */
            MESSAGE(LOG_DUMP, "\t  - ignoring %s\n", getCacheName(t, n));
            break;
    }
    cache->extcache[i].v = 0;
}

static void fpuCacheTransform(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3)
{
    int i2 = dyn->insts[ninst].x64.jmp_insts;
    if (i2 < 0)
        return;
    MESSAGE(LOG_DUMP, "\tCache Transform ---- ninst=%d -> %d\n", ninst, i2);
    if ((!i2) || (dyn->insts[i2].x64.barrier & BARRIER_FLOAT)) {
        if (dyn->e.stack_next) {
            fpu_purgecache(dyn, ninst, 1, s1, s2, s3);
            MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
            return;
        }
        for (int i = 0; i < 24; ++i)
            if (dyn->e.extcache[i].v) { // there is something at ninst for i
                fpu_purgecache(dyn, ninst, 1, s1, s2, s3);
                MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
                return;
            }
        MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
        return;
    }
    extcache_t cache_i2 = dyn->insts[i2].e;
    extcacheUnwind(&cache_i2);

    if (!cache_i2.stack) {
        int purge = 1;
        for (int i = 0; i < 24 && purge; ++i)
            if (cache_i2.extcache[i].v)
                purge = 0;
        if (purge) {
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
    // unload every unneeded cache
    // check SSE first, than MMX, in order, for optimization issue
    for (int i = 0; i < 16; ++i) {
        int j = findCacheSlot(dyn, ninst, EXT_CACHE_SS, i, &cache);
        if (j >= 0 && findCacheSlot(dyn, ninst, EXT_CACHE_SS, i, &cache_i2) == -1)
            unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, j, cache.extcache[j].t, cache.extcache[j].n);
        j = findCacheSlot(dyn, ninst, EXT_CACHE_SD, i, &cache);
        if (j >= 0 && findCacheSlot(dyn, ninst, EXT_CACHE_SD, i, &cache_i2) == -1)
            unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, j, cache.extcache[j].t, cache.extcache[j].n);
        j = findCacheSlot(dyn, ninst, EXT_CACHE_XMMW, i, &cache);
        if (j >= 0 && findCacheSlot(dyn, ninst, EXT_CACHE_XMMW, i, &cache_i2) == -1)
            unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, j, cache.extcache[j].t, cache.extcache[j].n);
    }
    for (int i = 0; i < 8; ++i) {
        int j = findCacheSlot(dyn, ninst, EXT_CACHE_MM, i, &cache);
        if (j >= 0 && findCacheSlot(dyn, ninst, EXT_CACHE_MM, i, &cache_i2) == -1)
            unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, j, cache.extcache[j].t, cache.extcache[j].n);
        j = findCacheSlot(dyn, ninst, EXT_CACHE_MMV, i, &cache);
        if (j >= 0 && findCacheSlot(dyn, ninst, EXT_CACHE_MMV, i, &cache_i2) == -1)
            unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, j, cache.extcache[j].t, cache.extcache[j].n);
    }
    for (int i = 0; i < 24; ++i) {
        if (cache.extcache[i].v)
            if (findCacheSlot(dyn, ninst, cache.extcache[i].t, cache.extcache[i].n, &cache_i2) == -1)
                unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, i, cache.extcache[i].t, cache.extcache[i].n);
    }
    // and now load/swap the missing one
    for (int i = 0; i < 24; ++i) {
        if (cache_i2.extcache[i].v) {
            if (cache_i2.extcache[i].v != cache.extcache[i].v) {
                int j;
                if ((j = findCacheSlot(dyn, ninst, cache_i2.extcache[i].t, cache_i2.extcache[i].n, &cache)) == -1)
                    loadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, i, cache_i2.extcache[i].t, cache_i2.extcache[i].n);
                else {
                    // it's here, lets swap if needed
                    if (j != i)
                        swapCache(dyn, ninst, i, j, &cache);
                }
            }
            if (cache.extcache[i].t != cache_i2.extcache[i].t) {
                if (cache.extcache[i].t == EXT_CACHE_ST_D && cache_i2.extcache[i].t == EXT_CACHE_ST_F) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.extcache[i].t, cache.extcache[i].n));
                    FCVTSD(EXTREG(i), EXTREG(i));
                    cache.extcache[i].t = EXT_CACHE_ST_F;
                } else if (cache.extcache[i].t == EXT_CACHE_ST_F && cache_i2.extcache[i].t == EXT_CACHE_ST_D) {
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
                } else if (cache.extcache[i].t == EXT_CACHE_XMMR && cache_i2.extcache[i].t == EXT_CACHE_XMMW) {
                    cache.extcache[i].t = EXT_CACHE_XMMW;
                } else if (cache.extcache[i].t == EXT_CACHE_XMMW && cache_i2.extcache[i].t == EXT_CACHE_XMMR) {
                    // refresh cache...
                    MESSAGE(LOG_DUMP, "\t  - Refreh %s\n", getCacheName(cache.extcache[i].t, cache.extcache[i].n));
                    SET_ELEMENT_WIDTH(s1, VECTOR_SEWANY, 0);
                    ADDI(s1, xEmu, offsetof(x64emu_t, xmm[cache.extcache[i].n]));
                    VSE_V(EXTREG(i), s1, dyn->vector_eew, VECTOR_UNMASKED, VECTOR_NFIELD1);
                    cache.extcache[i].t = EXT_CACHE_XMMR;
                }
            }
        }
    }
    if (stack_cnt != cache_i2.stack) {
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
        if (a > 0) {
            SLLI(s2, s2, a * 2);
        } else {
            MOV32w(s3, 0xffff0000);
            OR(s2, s2, s3);
            SRLI(s2, s2, -a * 2);
        }
        SH(s2, xEmu, offsetof(x64emu_t, fpu_tags));
        s3_top = 0;
        stack_cnt = cache_i2.stack;
    }
    MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
}
static void flagsCacheTransform(dynarec_rv64_t* dyn, int ninst, int s1)
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
                SW(xZR, xEmu, offsetof(x64emu_t, df));
            }
            break;
    }
    if (go) {
        if (dyn->f.pending != SF_PENDING) {
            LWU(s1, xEmu, offsetof(x64emu_t, df));
            j64 = (GETMARKF2) - (dyn->native_size);
            BEQZ(s1, j64);
        }
        CALL_(const_updateflags, -1, 0, 0, 0);
        MARKF2;
    }
}

static void sewTransform(dynarec_rv64_t* dyn, int ninst, int s1)
{
    int j64;
    int jmp = dyn->insts[ninst].x64.jmp_insts;
    if (jmp < 0) return;
    if (dyn->insts[jmp].vector_sew_entry == VECTOR_SEWNA) return;
    MESSAGE(LOG_DUMP, "\tSEW changed to %d ---- ninst=%d -> %d\n", dyn->insts[jmp].vector_sew_entry, ninst, jmp);
    vector_vsetvli(dyn, ninst, s1, dyn->insts[jmp].vector_sew_entry, VECTOR_LMUL1, 1);
}

void CacheTransform(dynarec_rv64_t* dyn, int ninst, int cacheupd, int s1, int s2, int s3)
{
    if (cacheupd & 2)
        fpuCacheTransform(dyn, ninst, s1, s2, s3);
    if (cacheupd & 1)
        flagsCacheTransform(dyn, ninst, s1);
    if (cacheupd & 4)
        sewTransform(dyn, ninst, s1);
}


void rv64_move32(dynarec_rv64_t* dyn, int ninst, int reg, int32_t val, int zeroup)
{
    // Depending on val, the following insts are emitted.
    // val == 0               -> ADDI
    // lo12 != 0 && hi20 == 0 -> ADDI
    // lo12 == 0 && hi20 != 0 -> LUI
    // else                   -> LUI+ADDI
    int32_t hi20 = (val + 0x800) >> 12 & 0xfffff;
    int32_t lo12 = val & 0xfff;

    int src = xZR;
    if (hi20) {
        LUI(reg, hi20);
        src = reg;
    }
    if (lo12 || !hi20) ADDIW(reg, src, lo12);
    if (zeroup && (val & 0x80000000)) {
        ZEROUP(reg);
    }
}

void rv64_move64(dynarec_rv64_t* dyn, int ninst, int reg, int64_t val)
{
    if (((val << 32) >> 32) == val) {
        // 32bits value
        rv64_move32(dyn, ninst, reg, val, 0);
        return;
    }

    int64_t lo12 = (val << 52) >> 52;
    int64_t hi52 = (val + 0x800) >> 12;
    int shift = 12 + TrailingZeros64((uint64_t)hi52);
    hi52 = ((hi52 >> (shift - 12)) << shift) >> shift;
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
    // need to undo the top and stack tracking that must not be reflected permanently yet
    x87_unreflectcount(dyn, ninst, s1, s2);
}

void emit_pf(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4)
{
    MAYUSE(dyn);
    MAYUSE(ninst);
    if (cpuext.zbb) {
        ANDI(s3, s1, 0xFF);
        CPOPW(s3, s3);
    } else {
        SRLI(s3, s1, 4);
        XOR(s3, s3, s1);

        SRLI(s4, s3, 2);
        XOR(s4, s3, s4);

        SRLI(s3, s4, 1);
        XOR(s3, s3, s4);
    }

    ANDI(s3, s3, 1);
    XORI(s3, s3, 1);
    SLLI(s3, s3, F_PF);
    OR(xFlags, xFlags, s3);
}

void fpu_reset_cache(dynarec_rv64_t* dyn, int ninst, int reset_n)
{
    MESSAGE(LOG_DEBUG, "Reset Caches with %d\n", reset_n);
#if STEP > 1
    // for STEP 2 & 3, just need to refresh with current, and undo the changes (push & swap)
    dyn->e = dyn->insts[ninst].e;
    dyn->vector_sew = dyn->insts[ninst].vector_sew_entry;
#else
    dyn->e = dyn->insts[reset_n].e;
    dyn->vector_sew = dyn->insts[reset_n].vector_sew_exit;
#endif
    extcacheUnwind(&dyn->e);
#if STEP == 0
    if (dyn->need_dump) dynarec_log(LOG_NONE, "New x87stack=%d\n", dyn->e.x87stack);
#endif
#if defined(HAVE_TRACE) && (STEP > 2)
    if (dyn->need_dump)
        if (memcmp(&dyn->e, &dyn->insts[reset_n].e, sizeof(ext_cache_t))) {
            MESSAGE(LOG_DEBUG, "Warning, difference in extcache: reset=");
            for (int i = 0; i < 24; ++i)
                if (dyn->insts[reset_n].e.extcache[i].v)
                    MESSAGE(LOG_DEBUG, " %02d:%s", i, getCacheName(dyn->insts[reset_n].e.extcache[i].t, dyn->insts[reset_n].e.extcache[i].n));
            if (dyn->insts[reset_n].e.combined1 || dyn->insts[reset_n].e.combined2)
                MESSAGE(LOG_DEBUG, " %s:%02d/%02d", dyn->insts[reset_n].e.swapped ? "SWP" : "CMB", dyn->insts[reset_n].e.combined1, dyn->insts[reset_n].e.combined2);
            if (dyn->insts[reset_n].e.stack_push || dyn->insts[reset_n].e.stack_pop)
                MESSAGE(LOG_DEBUG, " (%d:%d)", dyn->insts[reset_n].e.stack_push, -dyn->insts[reset_n].e.stack_pop);
            MESSAGE(LOG_DEBUG, " ==> ");
            for (int i = 0; i < 24; ++i)
                if (dyn->insts[ninst].e.extcache[i].v)
                    MESSAGE(LOG_DEBUG, " %02d:%s", i, getCacheName(dyn->insts[ninst].e.extcache[i].t, dyn->insts[ninst].e.extcache[i].n));
            if (dyn->insts[ninst].e.combined1 || dyn->insts[ninst].e.combined2)
                MESSAGE(LOG_DEBUG, " %s:%02d/%02d", dyn->insts[ninst].e.swapped ? "SWP" : "CMB", dyn->insts[ninst].e.combined1, dyn->insts[ninst].e.combined2);
            if (dyn->insts[ninst].e.stack_push || dyn->insts[ninst].e.stack_pop)
                MESSAGE(LOG_DEBUG, " (%d:%d)", dyn->insts[ninst].e.stack_push, -dyn->insts[ninst].e.stack_pop);
            MESSAGE(LOG_DEBUG, " -> ");
            for (int i = 0; i < 24; ++i)
                if (dyn->e.extcache[i].v)
                    MESSAGE(LOG_DEBUG, " %02d:%s", i, getCacheName(dyn->e.extcache[i].t, dyn->e.extcache[i].n));
            if (dyn->e.combined1 || dyn->e.combined2)
                MESSAGE(LOG_DEBUG, " %s:%02d/%02d", dyn->e.swapped ? "SWP" : "CMB", dyn->e.combined1, dyn->e.combined2);
            if (dyn->e.stack_push || dyn->e.stack_pop)
                MESSAGE(LOG_DEBUG, " (%d:%d)", dyn->e.stack_push, -dyn->e.stack_pop);
            MESSAGE(LOG_DEBUG, "\n");
        }
#endif // HAVE_TRACE
}

// propagate ST stack state, especial stack pop that are deferred
void fpu_propagate_stack(dynarec_rv64_t* dyn, int ninst)
{
    if (dyn->e.stack_pop) {
        for (int j = 0; j < 24; ++j)
            if ((dyn->e.extcache[j].t == EXT_CACHE_ST_D
                    || dyn->e.extcache[j].t == EXT_CACHE_ST_F
                    || dyn->e.extcache[j].t == EXT_CACHE_ST_I64)) {
                if (dyn->e.extcache[j].n < dyn->e.stack_pop)
                    dyn->e.extcache[j].v = 0;
                else
                    dyn->e.extcache[j].n -= dyn->e.stack_pop;
            }
        dyn->e.stack_pop = 0;
    }
    dyn->e.stack = dyn->e.stack_next;
    dyn->e.news = 0;
    dyn->e.stack_push = 0;
    dyn->e.swapped = 0;
}

// Simple wrapper for vsetvli, may use s1 as scratch
int vector_vsetvli(dynarec_rv64_t* dyn, int ninst, int s1, int sew, int vlmul, float multiple)
{
    if (sew == VECTOR_SEWNA) return VECTOR_SEW8;
    if (sew == VECTOR_SEWANY) sew = VECTOR_SEW8;

    uint32_t vl = (int)((float)(16 >> sew) * multiple);
    uint32_t vtypei = (sew << (3 - !!cpuext.xtheadvector)) | vlmul;
    if (dyn->inst_sew == VECTOR_SEWNA || dyn->inst_vl == 0 || dyn->inst_sew != sew || dyn->inst_vl != vl || dyn->inst_vlmul != vlmul) {
        if (vl == (cpuext.vlen >> (sew - vlmul))) {
            VSETVLI(s1, xZR, vtypei);
        } else if (vl <= 31 && !cpuext.xtheadvector) {
            VSETIVLI(xZR, vl, vtypei);
        } else {
            ADDI(s1, xZR, vl);
            VSETVLI(xZR, s1, vtypei);
        }
    }
    dyn->inst_sew = sew;
    dyn->inst_vl = vl;
    dyn->inst_vlmul = vlmul;
    return sew;
}

static uint64_t xtheadvector_16bitmasks[16] = {
    0x0000000000000000,
    0x0000000000000001,
    0x0000000000010000,
    0x0000000000010001,
    0x0000000100000000,
    0x0000000100000001,
    0x0000000100010000,
    0x0000000100010001,
    0x0001000000000000,
    0x0001000000000001,
    0x0001000000010000,
    0x0001000000010001,
    0x0001000100000000,
    0x0001000100000001,
    0x0001000100010000,
    0x0001000100010001,
};

void vector_loadmask(dynarec_rv64_t* dyn, int ninst, int vreg, uint64_t imm, int s1, float multiple)
{
#if STEP > 1
    uint8_t sew = dyn->inst_sew;
    uint8_t vlmul = dyn->inst_vlmul;
    if (cpuext.xtheadvector) {
        if (sew == VECTOR_SEW64 && vlmul == VECTOR_LMUL1) {
            switch (imm) {
                case 0:
                    VXOR_VV(vreg, vreg, vreg, VECTOR_UNMASKED);
                    return;
                case 1:
                    ADDI(s1, xZR, 1);
                    VMV_S_X(vreg, s1);
                    return;
                case 2: {
                    int scratch = fpu_get_scratch(dyn);
                    VMV_V_I(scratch, 1);
                    VSLIDE1UP_VX(vreg, scratch, xZR, VECTOR_UNMASKED);
                    return;
                }
                case 3:
                    VMV_V_I(vreg, 1);
                    return;
                default: abort();
            }
        } else if ((sew == VECTOR_SEW32 && vlmul == VECTOR_LMUL1) || (sew == VECTOR_SEW64 && vlmul == VECTOR_LMUL2)) {
            switch (imm) {
                case 0b0000:
                    VMV_S_X(vreg, xZR);
                    return;
                case 0b0001:
                    ADDI(s1, xZR, 1);
                    VMV_S_X(vreg, s1);
                    return;
                case 0b0010:
                    vector_vsetvli(dyn, ninst, s1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                    MOV64x(s1, 0x100000000ULL);
                    VMV_S_X(vreg, s1);
                    vector_vsetvli(dyn, ninst, s1, sew, vlmul, multiple);
                    return;
                case 0b0011:
                    vector_vsetvli(dyn, ninst, s1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                    MOV64x(s1, 0x100000001ULL);
                    VMV_S_X(vreg, s1);
                    vector_vsetvli(dyn, ninst, s1, sew, vlmul, multiple);
                    return;
                case 0b0100: {
                    int scratch = fpu_get_scratch(dyn);
                    vector_vsetvli(dyn, ninst, s1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                    VMV_V_I(scratch, 1);
                    VSLIDE1UP_VX(vreg, scratch, xZR, VECTOR_UNMASKED);
                    vector_vsetvli(dyn, ninst, s1, sew, vlmul, multiple);
                    return;
                }
                case 0b0101:
                    vector_vsetvli(dyn, ninst, s1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                    VMV_V_I(vreg, 1);
                    vector_vsetvli(dyn, ninst, s1, sew, vlmul, multiple);
                    return;
                case 0b0110: {
                    int scratch = fpu_get_scratch(dyn);
                    vector_vsetvli(dyn, ninst, s1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                    VMV_V_I(scratch, 1);
                    MOV64x(s1, 0x100000000ULL);
                    VSLIDE1UP_VX(vreg, scratch, s1, VECTOR_UNMASKED);
                    vector_vsetvli(dyn, ninst, s1, sew, vlmul, multiple);
                    return;
                }
                case 0b0111: {
                    int scratch = fpu_get_scratch(dyn);
                    vector_vsetvli(dyn, ninst, s1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                    VMV_V_I(scratch, 1);
                    MOV64x(s1, 0x100000001ULL);
                    VSLIDE1UP_VX(vreg, scratch, s1, VECTOR_UNMASKED);
                    vector_vsetvli(dyn, ninst, s1, sew, vlmul, multiple);
                    return;
                }
                case 0b1000: {
                    int scratch = fpu_get_scratch(dyn);
                    vector_vsetvli(dyn, ninst, s1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                    MOV64x(s1, 0x100000000ULL);
                    VMV_V_X(scratch, s1);
                    VSLIDE1UP_VX(vreg, scratch, xZR, VECTOR_UNMASKED);
                    vector_vsetvli(dyn, ninst, s1, sew, vlmul, multiple);
                    return;
                }
                case 0b1001: {
                    int scratch = fpu_get_scratch(dyn);
                    vector_vsetvli(dyn, ninst, s1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                    MOV64x(s1, 0x100000000ULL);
                    VMV_V_X(scratch, s1);
                    ADDI(s1, xZR, 1);
                    VSLIDE1UP_VX(vreg, scratch, s1, VECTOR_UNMASKED);
                    vector_vsetvli(dyn, ninst, s1, sew, vlmul, multiple);
                    return;
                }
                case 0b1010:
                    vector_vsetvli(dyn, ninst, s1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                    MOV64x(s1, 0x100000000ULL);
                    VMV_V_X(vreg, s1);
                    vector_vsetvli(dyn, ninst, s1, sew, vlmul, multiple);
                    return;
                case 0b1011: {
                    int scratch = fpu_get_scratch(dyn);
                    vector_vsetvli(dyn, ninst, s1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                    MOV64x(s1, 0x100000000ULL);
                    VMV_V_X(scratch, s1);
                    MOV64x(s1, 0x100000001ULL);
                    VSLIDE1UP_VX(vreg, scratch, s1, VECTOR_UNMASKED);
                    vector_vsetvli(dyn, ninst, s1, sew, vlmul, multiple);
                    return;
                }
                case 0b1100: {
                    int scratch = fpu_get_scratch(dyn);
                    vector_vsetvli(dyn, ninst, s1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                    MOV64x(s1, 0x100000001ULL);
                    VMV_V_X(scratch, s1);
                    VSLIDE1UP_VX(vreg, scratch, xZR, VECTOR_UNMASKED);
                    vector_vsetvli(dyn, ninst, s1, sew, vlmul, multiple);
                    return;
                }
                case 0b1101: {
                    int scratch = fpu_get_scratch(dyn);
                    vector_vsetvli(dyn, ninst, s1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                    MOV64x(s1, 0x100000001ULL);
                    VMV_V_X(scratch, s1);
                    ADDI(s1, xZR, 1);
                    VSLIDE1UP_VX(vreg, scratch, s1, VECTOR_UNMASKED);
                    vector_vsetvli(dyn, ninst, s1, sew, vlmul, multiple);
                    return;
                }
                case 0b1110: {
                    int scratch = fpu_get_scratch(dyn);
                    vector_vsetvli(dyn, ninst, s1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                    MOV64x(s1, 0x100000001ULL);
                    VMV_V_X(scratch, s1);
                    ADDI(s1, s1, -1);
                    VSLIDE1UP_VX(vreg, scratch, s1, VECTOR_UNMASKED);
                    vector_vsetvli(dyn, ninst, s1, sew, vlmul, multiple);
                    return;
                }
                case 0b1111:
                    vector_vsetvli(dyn, ninst, s1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                    MOV64x(s1, 0x100000001ULL);
                    VMV_V_X(vreg, s1);
                    vector_vsetvli(dyn, ninst, s1, sew, vlmul, multiple);
                    return;
                default: abort();
            }
        } else if ((sew == VECTOR_SEW16 && vlmul == VECTOR_LMUL1) || (sew == VECTOR_SEW32 && vlmul == VECTOR_LMUL2)) {
            if (imm > 255) abort();
            if (imm == 0) {
                VMV_S_X(vreg, xZR);
                return;
            }
            int low = imm & 0xF;
            int high = (imm >> 4) & 0xF;
            int scratch;
            vector_vsetvli(dyn, ninst, s1, VECTOR_SEW64, VECTOR_LMUL1, 1);
            if (low == high) {
                MOV64x(s1, xtheadvector_16bitmasks[low]);
                VMV_V_X(vreg, s1);
                vector_vsetvli(dyn, ninst, s1, sew, vlmul, multiple);
                return;
            }
            if (high != 0) {
                scratch = fpu_get_scratch(dyn);
                MOV64x(s1, xtheadvector_16bitmasks[high]);
                VMV_S_X(scratch, s1);
            }
            if (low != 0)
                MOV64x(s1, xtheadvector_16bitmasks[low]);
            else
                s1 = xZR;
            if (high != 0) {
                VSLIDE1UP_VX(vreg, scratch, s1, VECTOR_UNMASKED);
            } else {
                VMV_S_X(vreg, s1);
            }
            vector_vsetvli(dyn, ninst, s1, sew, vlmul, multiple);
            return;
        } else if ((sew == VECTOR_SEW8 && vlmul == VECTOR_LMUL1) || (sew == VECTOR_SEW16 && vlmul == VECTOR_LMUL2)) {
            switch (imm) {
                case 0b0000000000001111:
                    vector_vsetvli(dyn, ninst, s1, VECTOR_SEW32, VECTOR_LMUL1, 1);
                    MOV64x(s1, 0xFFFFFFFFFFFFFFFFULL);
                    VMV_S_X(vreg, s1);
                    vector_vsetvli(dyn, ninst, s1, sew, vlmul, multiple);
                    return;
                case 0b0000000011111111:
                    vector_vsetvli(dyn, ninst, s1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                    MOV64x(s1, 0xFFFFFFFFFFFFFFFFULL);
                    VMV_S_X(vreg, s1);
                    vector_vsetvli(dyn, ninst, s1, sew, vlmul, multiple);
                    return;
                case 0b0101010101010101:
                    vector_vsetvli(dyn, ninst, s1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                    MOV64x(s1, 0x0001000100010001ULL);
                    VMV_V_X(vreg, s1);
                    vector_vsetvli(dyn, ninst, s1, sew, vlmul, multiple);
                    return;
                case 0b1010101010101010:
                    vector_vsetvli(dyn, ninst, s1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                    MOV64x(s1, 0x0100010001000100ULL);
                    VMV_V_X(vreg, s1);
                    vector_vsetvli(dyn, ninst, s1, sew, vlmul, multiple);
                    return;
                default: abort();
            }
        } else
            abort();
    } else {
        if (imm <= 0xF && (dyn->vector_eew == VECTOR_SEW32 || dyn->vector_eew == VECTOR_SEW64)) {
            VMV_V_I(vreg, imm);
        } else if (dyn->vector_eew == VECTOR_SEW8 && imm >= 0xFF) {
            if ((imm > 0xFF) && (imm & 0xFF) == (imm >> 8)) {
                MOV64x(s1, imm);
                VMV_V_X(vreg, s1);
            } else if (imm > 0xFF) {
                abort(); // not used (yet)
            } else {
                MOV64x(s1, imm);
                VXOR_VV(vreg, vreg, vreg, VECTOR_UNMASKED);
                VMV_S_X(vreg, s1);
            }
        } else {
            MOV64x(s1, imm);
            VMV_S_X(vreg, s1);
        }
    }
#endif
}
