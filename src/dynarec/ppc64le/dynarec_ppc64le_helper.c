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
#include "ppc64le_emitter.h"
#include "ppc64le_mapping.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "../dynablock_private.h"
#include "custommem.h"

#include "ppc64le_printer.h"
#include "dynarec_ppc64le_private.h"
#include "dynarec_ppc64le_functions.h"
#include "../dynarec_helper.h"

#define SCRATCH 31

/* setup r2 to address pointed by ED, also fixaddress is an optional delta in the range [-32768, +32767] (16-bit signed),
 * with delta&i12==0, to be added to ed for LD/STD/LXV/STXV.
 *
 * The i12 parameter is the alignment mask for displacement validation:
 *   - i12 = NO_DISP (0):   No inline displacement; always materialize address in register.
 *   - i12 = DS_DISP (3):   Allow inline displacement if within range and 4-byte aligned (DS-form: LD/STD).
 *   - i12 = DQ_DISP (15):  Allow inline displacement if within range and 16-byte aligned (DQ-form: LXV/STXV).
 */
uintptr_t geted(dynarec_ppc64le_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, rex_t rex, int* l, int i12, int delta)
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
    // i12 is the alignment mask: 0 = no inline displacement, 3 = DS-form, 15 = DQ-form
    int align_mask = i12;
    int maxval = 32767;  // PPC64LE ADDI has 16-bit signed immediate
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
                    if (tmp && ((tmp < -32768) || (tmp > maxval) || !align_mask || (tmp & align_mask))) {
                        MOV64y(scratch, tmp);
                        SLADDy(ret, TO_NAT(sib_reg), scratch, sib >> 6);
                        SCRATCH_USAGE(1);
                    } else {
                        if (sib >> 6) {
                            SLLIy(ret, TO_NAT(sib_reg), (sib >> 6));
                            if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                        } else {
                            ret = TO_NAT(sib_reg);
                        }
                        *fixaddress = tmp;
                    }
                } else {
                    if (rex.seg && !(tmp && ((tmp < -32768) || (tmp > maxval) || !align_mask || (tmp & align_mask)))) {
                        grab_segdata(dyn, addr, ninst, ret, rex.seg);
                        seg_done = 1;
                        *fixaddress = tmp;
                    } else if (rex.seg && tmp >= -32768 && tmp < 32768) {
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
                    SLADDy(ret, TO_NAT(sib_reg), TO_NAT(sib_reg2), sib >> 6);
                    if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                } else {
                    ret = TO_NAT(sib_reg2);
                }
            }
        } else if ((nextop & 7) == 5) {
            if (rex.is32bits) {
                int tmp = F32S;
                if (rex.seg && tmp >= -32768 && tmp < 32768) {
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
                if (align_mask && adj && (tmp + adj >= -32768) && (tmp + adj <= maxval) && !((tmp + adj) & align_mask)) {
                    ret = xRIP;
                    *fixaddress = tmp + adj;
                } else if (align_mask && (tmp >= -32768) && (tmp <= maxval) && !(tmp & align_mask)) {
                    GETIP(addr + delta, scratch);
                    ret = xRIP;
                    *fixaddress = tmp;
                    SCRATCH_USAGE(1);
                } else if (adj && (tmp + adj >= -32768) && (tmp + adj <= maxval)) {
                    ADDI(ret, xRIP, tmp + adj);
                } else if ((tmp >= -32768) && (tmp <= maxval)) {
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
        if (i64 == 0 || ((i64 >= -32768) && (i64 <= maxval) && align_mask && !(i64 & align_mask))) {
            *fixaddress = i64;
            if ((nextop & 7) == 4) {
                if (sib_reg != 4) {
                    SLADDy(ret, TO_NAT(sib_reg), TO_NAT(sib_reg2), sib >> 6);
                    if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                } else {
                    ret = TO_NAT(sib_reg2);
                }
            } else {
                ret = TO_NAT((nextop & 0x07) + (rex.b << 3));
            }
        } else {
            if (i64 >= -32768 && i64 <= 32767) {
                if ((nextop & 7) == 4) {
                    if (sib_reg != 4) {
                        SLADDy(scratch, TO_NAT(sib_reg), TO_NAT(sib_reg2), sib >> 6);
                        SCRATCH_USAGE(1);
                    } else {
                        scratch = TO_NAT(sib_reg2);
                    }
                } else {
                    scratch = TO_NAT((nextop & 0x07) + (rex.b << 3));
                }
                ADDIy(ret, scratch, i64);
                if (!IS_GPR(ret)) SCRATCH_USAGE(1);
            } else {
                MOV64y(scratch, i64);
                SCRATCH_USAGE(1);
                if ((nextop & 7) == 4) {
                    if (sib_reg != 4) {
                        ADDy(scratch, scratch, TO_NAT(sib_reg2));
                        SLADDy(ret, TO_NAT(sib_reg), scratch, sib >> 6);
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
        ZEROUP2(hint, ret); // truncate for is67 case only (is32bits case regs are already 32bits only)
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

uintptr_t geted16(dynarec_ppc64le_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, rex_t rex, int i12)
{
    MAYUSE(dyn);
    MAYUSE(ninst);

    uint8_t ret = x2;
    *fixaddress = 0;
    if (hint > 0) ret = hint;
    int align_mask16 = i12;
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
        if (align_mask16 && offset && offset >= -32768 && offset <= 32767 && !(offset & align_mask16)) {
            *fixaddress = offset;
            offset = 0;
        }
        int reg;
        switch (m & 7) {
            case 0: // R_BX + R_SI
                ADD(ret, xRBX, xRSI);
                reg = ret;
                if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                break;
            case 1: // R_BX + R_DI
                ADD(ret, xRBX, xRDI);
                reg = ret;
                if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                break;
            case 2: // R_BP + R_SI
                ADD(ret, xRBP, xRSI);
                reg = ret;
                if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                break;
            case 3: // R_BP + R_DI
                ADD(ret, xRBP, xRDI);
                reg = ret;
                if (!IS_GPR(ret)) SCRATCH_USAGE(1);
                break;
            case 4: // R_SI
                reg = xRSI;
                break;
            case 5: // R_DI
                reg = xRDI;
                break;
            case 6: // R_BP
                reg = xRBP;
                break;
            case 7: // R_BX
                reg = xRBX;
                break;
        }
        BF_EXTRACT(ret, reg, 15, 0);
        if (!IS_GPR(ret)) SCRATCH_USAGE(1);
        if (offset) {
            if (offset >= -32768 && offset < 32768) {
                ADDI(ret, ret, offset);
            } else {
                MOV64x(scratch, offset);
                ADD(ret, ret, scratch);
                SCRATCH_USAGE(1);
            }
            BF_EXTRACT(ret, ret, 15, 0);
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

// Load a 32-bit value into a register
// Optimizes for small values that fit in fewer instructions
// If zeroup is set, clears upper 32 bits when val is negative
void ppc64le_move32(dynarec_ppc64le_t* dyn, int ninst, int reg, int32_t val, int zeroup)
{
    uint32_t uval = (uint32_t)val;
    uint16_t lo = uval & 0xFFFF;
    uint16_t hi = (uval >> 16) & 0xFFFF;

    if (val == (int32_t)(int16_t)val) {
        // Value fits in signed 16-bit: single LI
        LI(reg, val & 0xFFFF);
    } else if (lo == 0) {
        // Lower 16 bits are zero: single LIS
        LIS(reg, hi);
    } else {
        // General case: LIS + ORI
        LIS(reg, hi);
        ORI(reg, reg, lo);
    }
    if (zeroup && val < 0) ZEROUP(reg);
}

// Load a 64-bit value into a register
// Optimizes for values that fit in fewer instructions
void ppc64le_move64(dynarec_ppc64le_t* dyn, int ninst, int reg, int64_t val)
{
    // If value fits in signed 32 bits, use move32 (LI sign-extends to 64-bit)
    if (val == (int64_t)(int32_t)val) {
        ppc64le_move32(dyn, ninst, reg, (int32_t)val, 0);
        return;
    }

    // Full 64-bit constant: build top-down
    // Load upper 32 bits, shift into place, OR in lower 32 bits
    uint64_t uval = (uint64_t)val;
    uint16_t w0 = (uval >>  0) & 0xFFFF;  // bits [ 0:15]
    uint16_t w1 = (uval >> 16) & 0xFFFF;  // bits [16:31]

    // Load upper 32 bits (bits [32:63]) as a sign-extended 32-bit value
    int32_t upper32 = (int32_t)(uval >> 32);
    ppc64le_move32(dyn, ninst, reg, upper32, 0);

    // Shift left by 32 to place into upper half
    SLDI(reg, reg, 32);

    // OR in bits [16:31]
    if (w1) {
        ORIS(reg, reg, w1);
    }

    // OR in bits [0:15]
    if (w0) {
        ORI(reg, reg, w0);
    }
}

// ========================================================================
// Jump / Control Flow — Batch H3
// ========================================================================

void jump_to_epilog(dynarec_ppc64le_t* dyn, uintptr_t ip, int reg, int ninst)
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
    CHECK_DFNONE(0);
    MTCTR(x2);
    BCTR();
}

void jump_to_epilog_fast(dynarec_ppc64le_t* dyn, uintptr_t ip, int reg, int ninst)
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
    // PPC64LE has no const_epilog_fast, use const_epilog as fallback
    TABLE64C(x2, const_epilog);
    SMEND();
    CHECK_DFNONE(0);
    MTCTR(x2);
    BCTR();
}

// indirect_lookup: resolve x86-64 address through the jump table hierarchy
// Returns the register containing the resolved native address
static int indirect_lookup(dynarec_ppc64le_t* dyn, int ninst, int is32bits, int s1, int s2)
{
    MAYUSE(dyn);
    if (!is32bits) {
        // Check if address is above 48-bit range
        SRDI(s1, xRIP, 48);
        BNEZ_safe(s1, (intptr_t)dyn->jmp_next - (intptr_t)dyn->block);
        // Load level-2 table base
        if (dyn->need_reloc) {
            TABLE64C(s2, const_jmptbl48);
        } else {
            MOV64x(s2, getConst(const_jmptbl48));
        }
        // Extract bits for level-2 index
        BF_EXTRACT(s1, xRIP, JMPTABL_START2 + JMPTABL_SHIFT2 - 1, JMPTABL_START2);
        SLDI(s1, s1, 3);
        ADD(s2, s1, s2);
        LD(s2, 0, s2);
    } else {
        TABLE64C(s2, const_jmptbl32);
    }
    // Level-1 lookup
    BF_EXTRACT(s1, xRIP, JMPTABL_START1 + JMPTABL_SHIFT1 - 1, JMPTABL_START1);
    SLDI(s1, s1, 3);
    ADD(s2, s1, s2);
    LD(s2, 0, s2);
    // Level-0 lookup
    BF_EXTRACT(s1, xRIP, JMPTABL_START0 + JMPTABL_SHIFT0 - 1, JMPTABL_START0);
    SLDI(s1, s1, 3);
    ADD(s2, s1, s2);
    LD(s1, 0, s2);
    return s1;
}

void jump_to_next(dynarec_ppc64le_t* dyn, uintptr_t ip, int reg, int ninst, int is32bits)
{
    MAYUSE(dyn);
    MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Jump to next\n");

    if (is32bits)
        ip &= 0xffffffffLL;

    // Move target address into xRIP BEFORE CHECK_DFNONE, because
    // FORCE_DFNONE uses LI(x1, 0) which clobbers x1 — and ed is often x1.
    int dest;
    if (reg) {
        if (reg != xRIP) {
            MV(xRIP, reg);
        }
        CHECK_DFNONE(0);
        NOTEST(x2);
        dest = indirect_lookup(dyn, ninst, is32bits, x2, x3);
    } else {
        CHECK_DFNONE(0);
        NOTEST(x2);
        uintptr_t p = getJumpTableAddress64(ip);
        MAYUSE(p);
        GETIP_(ip, x3);
        if (dyn->need_reloc) AddRelocTable64JmpTbl(dyn, ninst, ip, STEP);
        TABLE64_(x3, p);
        LD(x2, 0, x3);
        dest = x2;
    }
    if (reg != x1) {
        MV(x1, xRIP);
    }
    CLEARIP();
    SMEND();
    // Branch to resolved target: use BCTRL for call-ret, BCTR otherwise
#ifdef HAVE_TRACE
    MTCTR(dest);
    BCTRL();
#else
    MTCTR(dest);
    if (dyn->insts[ninst].x64.has_callret) {
        BCTRL();
    } else {
        BCTR();
    }
#endif
}

void ret_to_next(dynarec_ppc64le_t* dyn, uintptr_t ip, int ninst, rex_t rex)
{
    MAYUSE(dyn);
    MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Ret to next\n");
    CHECK_DFNONE(0);
    MVz(x1, xRIP);
    SMEND();
    if (BOX64DRENV(dynarec_callret)) {
        // pop the actual return address from PPC64LE stack
        LD(x4, 0, xSP);       // native addr
        LD(x6, 8, xSP);       // x86 addr
        ADDI(xSP, xSP, 16);   // pop
        // is it the right address?
        CMPD(x6, xRIP);
        BNE(3 * 4);
        // yes — branch to cached native return address
        MTCTR(x4);
        BCTR();
        // not the correct return address, regular jump, but purge the stack first
        // Load the saved frame SP from emu->xSPSave (r12 is no longer xSavedSP)
        LD(xSP, offsetof(x64emu_t, xSPSave), xEmu);
    }
    NOTEST(x2);
    int dest = indirect_lookup(dyn, ninst, rex.is32bits, x2, x3);
    MTCTR(dest);
    BCTR();
    CLEARIP();
}

void iret_to_next(dynarec_ppc64le_t* dyn, uintptr_t ip, int ninst, int is32bits, int is64bits)
{
    MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "IRet to next\n");
    if (is64bits) {
        POP1(xRIP);
        POP1(x2);
        POP1(xFlags);
    } else {
        POP1_32(xRIP);
        POP1_32(x2);
        POP1_32(xFlags);
    }

    // Store CS segment
    STH(x2, offsetof(x64emu_t, segs[_CS]), xEmu);
    // clean EFLAGS
    RESTORE_EFLAGS(x1);
    MOV32w(x1, 0x3E7FD7);   // also mask RF
    AND(xFlags, xFlags, x1);
    ORI(xFlags, xFlags, 0x2);
    SPILL_EFLAGS();
    CHECK_DFNONE(0);
    // POP RSP
    if (is64bits) {
        POP1(x3);   // rsp
        POP1(x2);   // ss
    } else {
        POP1_32(x3);   // rsp
        POP1_32(x2);   // ss
    }
    // POP SS
    STH(x2, offsetof(x64emu_t, segs[_SS]), xEmu);
    // set new RSP
    MV(xRSP, x3);
    // Ret....
    rex_t dummy = { 0 };
    dummy.is32bits = is32bits;
    dummy.w = is64bits;
    ret_to_next(dyn, ip, ninst, dummy);
    CLEARIP();
}

// ========================================================================
// C/Native Calls & Segments — Batch H4
// ========================================================================

void call_c(dynarec_ppc64le_t* dyn, int ninst, ppc64le_consts_t fnc, int reg, int ret, int saveflags, int savereg, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6)
{
    MAYUSE(fnc);
    // Bug 4 fix: Cannot use CHECK_DFNONE(1) here because it expands to
    // FORCE_DFNONE() which uses x1 (=r3=A0) as scratch. If the caller
    // pre-loaded an arg into x1 before calling call_c, FORCE_DFNONE
    // would clobber it. Use `reg` (x6=r8) as scratch instead — it's the
    // dedicated function pointer register that gets overwritten by
    // TABLE64C(reg, fnc) later anyway.
    if (dyn->f == status_none_pending) {
        LI(reg, 0);
        STW(reg, offsetof(x64emu_t, df), xEmu);
        dyn->f = status_none;
    }
    if (savereg == 0)
        savereg = x87pc;
    if (saveflags) {
        RESTORE_EFLAGS(reg);
        STD(xFlags, offsetof(x64emu_t, eflags), xEmu);
    }
    fpu_pushcache(dyn, ninst, reg, 0);
    if (ret != -2) {
        ADDI(xSP, xSP, -16); // PPC64LE stack needs to be 16byte aligned
        STD(savereg, 0, xSP);
        STORE_REG(RDI);
        STORE_REG(RSI);
        STORE_REG(RDX);
        STORE_REG(RCX);
        STORE_REG(R8);
        STORE_REG(R9);
        STORE_REG(RAX);
        STORE_REG(RBX);
        STORE_REG(RSP);
        STORE_REG(RBP);
        STD(xRIP, offsetof(x64emu_t, ip), xEmu);
    }
    TABLE64C(reg, fnc);
    // Save function pointer to CTR immediately, before argument shuffling.
    // On PPC64LE, scratch registers x1-x6 alias ABI argument registers A0-A5
    // (x1=r3=A0, ..., x6=r8=A5). The function pointer is loaded into reg
    // (=x6=r8=A5), so any MV(A5, arg5) during argument shuffling would
    // clobber it. Saving to CTR first avoids this, since CTR is unaffected
    // by the MV (register-to-register move) instructions.
    MTCTR(reg);
    // Argument shuffling: reverse order to handle the "shift up by one"
    // permutation (x1→A1, x2→A2, ...) without clobbering sources.
    if (arg6) MV(A6, arg6);
    if (arg5) MV(A5, arg5);
    if (arg4) MV(A4, arg4);
    if (arg3) MV(A3, arg3);
    if (arg2) MV(A2, arg2);
    if (arg1) MV(A1, arg1);
    MV(A0, xEmu);
    // ELFv2 ABI: r12 must be set to the function entry address
    // for global entry point TOC setup. Recover from CTR.
    MFCTR(12);
    BCTRL();
    if (ret >= 0) {
        MV(ret, A0);
    }
    if (ret != -2) {
        LD(savereg, 0, xSP);
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
        GO(RBX);
        GO(RSP);
        GO(RBP);
        if (ret != xRIP)
            LD(xRIP, offsetof(x64emu_t, ip), xEmu);
#undef GO
    }

    fpu_popcache(dyn, ninst, reg, 0);
    if (saveflags) {
        LD(xFlags, offsetof(x64emu_t, eflags), xEmu);
        SPILL_EFLAGS();
    }
    if (savereg != x87pc && dyn->need_x87check)
        NATIVE_RESTORE_X87PC();
    // SET_NODF();
    dyn->last_ip = 0;
}

void call_n(dynarec_ppc64le_t* dyn, int ninst, void* fnc, int w)
{
    MAYUSE(fnc);
    CHECK_DFNONE(1);
    fpu_pushcache(dyn, ninst, x3, 1);
    // Save x86 regs that the native function might modify via re-entrant emulation.
    // On PPC64LE, x86 regs are in callee-saved r14-r29, so the native function preserves
    // them. We save SP/BP/BX to the emu struct for consistency with the interpreter.
    STD(xRSP, offsetof(x64emu_t, regs[_SP]), xEmu);
    STD(xRBP, offsetof(x64emu_t, regs[_BP]), xEmu);
    STD(xRBX, offsetof(x64emu_t, regs[_BX]), xEmu);
    // prepare regs for native call: copy x86 argument registers to PPC64LE ABI registers.
    // On PPC64LE, x86 regs are in callee-saved r14-r29, while ABI args are in r3-r8.
    // No overlap, so order doesn't matter.
    MV(A0, xRDI);
    MV(A1, xRSI);
    MV(A2, xRDX);
    MV(A3, xRCX);
    MV(A4, xR8);
    MV(A5, xR9);
    // check if additional sextw needed (applied after copy to ABI regs)
    int sextw_mask = ((w > 0 ? w : -w) >> 4) & 0b111111;
    for (int i = 0; i < 6; i++) {
        if (sextw_mask & (1 << i)) {
            SEXT_W(A0 + i, A0 + i);
        }
    }
    // native call — load function pointer into x7 (r10) to avoid clobbering argument regs
    if (dyn->need_reloc) {
        // fnc is indirect, to help with relocation (but PltResolver might be an issue here)
        TABLE64(x7, (uintptr_t)fnc);
        LD(x7, 0, x7);
    } else {
        TABLE64_(x7, *(uintptr_t*)fnc);
    }
    // ELFv2 ABI: r12 must be set to the function entry address
    MR(12, x7);
    MTCTR(x7);
    BCTRL();
    // put return value in x64 regs
    if (w > 0) {
        MV(xRAX, A0);
        MV(xRDX, A1);
    }
    // all done, restore all regs
    LD(xRSP, offsetof(x64emu_t, regs[_SP]), xEmu);
    LD(xRBP, offsetof(x64emu_t, regs[_BP]), xEmu);
    LD(xRBX, offsetof(x64emu_t, regs[_BX]), xEmu);
    fpu_popcache(dyn, ninst, x3, 1);
    NATIVE_RESTORE_X87PC();
    // SET_NODF();
}

void grab_segdata(dynarec_ppc64le_t* dyn, uintptr_t addr, int ninst, int reg, int segment)
{
    (void)addr;
    int64_t j64;
    MAYUSE(j64);
    MESSAGE(LOG_DUMP, "Get %s Offset\n", (segment == _FS) ? "FS" : "GS");
    LD(reg, offsetof(x64emu_t, segs_offs[segment]), xEmu);
    MESSAGE(LOG_DUMP, "----%s Offset\n", (segment == _FS) ? "FS" : "GS");
}

// ========================================================================
// x87 Stack & Cache Management — Batch H5
// ========================================================================

int x87_stackcount(dynarec_ppc64le_t* dyn, int ninst, int scratch)
{
    MAYUSE(scratch);
    if (!dyn->v.x87stack)
        return 0;
    if (dyn->v.mmxcount)
        mmx_purgecache(dyn, ninst, 0, scratch);
    MESSAGE(LOG_DUMP, "\tSynch x87 Stackcount (%d)\n", dyn->v.x87stack);
    int a = dyn->v.x87stack;
    // Add x87stack to emu fpu_stack
    LWZ(scratch, offsetof(x64emu_t, fpu_stack), xEmu);
    ADDI(scratch, scratch, a);
    STW(scratch, offsetof(x64emu_t, fpu_stack), xEmu);
    // Sub x87stack to top, with and 7
    LWZ(scratch, offsetof(x64emu_t, top), xEmu);
    ADDI(scratch, scratch, -a);
    ANDId(scratch, scratch, 7);
    STW(scratch, offsetof(x64emu_t, top), xEmu);
    // reset x87stack, but not the stack count of extcache
    dyn->v.x87stack = 0;
    dyn->v.stack_next -= dyn->v.stack;
    int ret = dyn->v.stack;
    dyn->v.stack = 0;
    MESSAGE(LOG_DUMP, "\t------x87 Stackcount\n");
    return ret;
}

void x87_unstackcount(dynarec_ppc64le_t* dyn, int ninst, int scratch, int count)
{
    MAYUSE(scratch);
    if (!count)
        return;
    if (dyn->v.mmxcount)
        mmx_purgecache(dyn, ninst, 0, scratch);
    MESSAGE(LOG_DUMP, "\tSynch x87 Unstackcount (%d)\n", count);
    int a = -count;
    // Add x87stack to emu fpu_stack
    LWZ(scratch, offsetof(x64emu_t, fpu_stack), xEmu);
    ADDI(scratch, scratch, a);
    STW(scratch, offsetof(x64emu_t, fpu_stack), xEmu);
    // Sub x87stack to top, with and 7
    LWZ(scratch, offsetof(x64emu_t, top), xEmu);
    ADDI(scratch, scratch, -a);
    ANDId(scratch, scratch, 7);
    STW(scratch, offsetof(x64emu_t, top), xEmu);
    // reset x87stack, but not the stack count of extcache
    dyn->v.x87stack = count;
    dyn->v.stack = count;
    dyn->v.stack_next += dyn->v.stack;
    MESSAGE(LOG_DUMP, "\t------x87 Unstackcount\n");
}

void x87_forget(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int st)
{
    int ret = -1;
    for (int i = 0; (i < 8) && (ret == -1); ++i)
        if (dyn->v.x87cache[i] == st)
            ret = i;
    if (ret == -1) // nothing to do
        return;
    MESSAGE(LOG_DUMP, "\tForget x87 Cache for ST%d\n", st);
    const int reg = dyn->v.x87reg[ret];
#if STEP == 1
    if (dyn->v.vmxcache[dyn->v.x87reg[ret]].t == VMX_CACHE_ST_F
        || dyn->v.vmxcache[dyn->v.x87reg[ret]].t == VMX_CACHE_ST_I64)
        vmxcache_promote_double(dyn, ninst, st);
#endif
    // prepare offset to fpu => s1
    // Get top
    LWZ(s2, offsetof(x64emu_t, top), xEmu);
    // Update
    int a = st - dyn->v.x87stack;
    if (a) {
        ADDI(s2, s2, a);
        ANDId(s2, s2, 7); // (emu->top + i)&7
    }
    SLDI(s2, s2, 3);
    ADD(s1, xEmu, s2);
    if (dyn->v.vmxcache[reg].t == VMX_CACHE_ST_F) {
        FMR(SCRATCH0, reg);
        STFD(SCRATCH0, offsetof(x64emu_t, x87), s1);
    } else if (dyn->v.vmxcache[reg].t == VMX_CACHE_ST_I64) {
        FCFID(SCRATCH0, reg);
        STFD(SCRATCH0, offsetof(x64emu_t, x87), s1);
    } else {
        STFD(reg, offsetof(x64emu_t, x87), s1);
    }
    MESSAGE(LOG_DUMP, "\t--------x87 Cache for ST%d\n", st);
    // and forget that cache
    fpu_free_reg(dyn, dyn->v.x87reg[ret]);
    dyn->v.vmxcache[reg].v = 0;
    dyn->v.x87cache[ret] = -1;
    dyn->v.x87reg[ret] = -1;
}

void x87_reget_st(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int st)
{
    dyn->insts[ninst].x87_used = 1;
    if (dyn->v.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    // search in cache first
    for (int i = 0; i < 8; ++i)
        if (dyn->v.x87cache[i] == st) {
            // refresh the value
            MESSAGE(LOG_DUMP, "\tRefresh x87 Cache for ST%d\n", st);
#if STEP == 1
            if (dyn->v.vmxcache[dyn->v.x87reg[i]].t == VMX_CACHE_ST_F
                || dyn->v.vmxcache[dyn->v.x87reg[i]].t == VMX_CACHE_ST_I64)
                vmxcache_promote_double(dyn, ninst, st);
#endif
            LWZ(s2, offsetof(x64emu_t, top), xEmu);
            int a = st - dyn->v.x87stack;
            if (a) {
                ADDI(s2, s2, a);
                ANDId(s2, s2, 7);
            }
            SLDI(s2, s2, 3);
            ADD(s1, xEmu, s2);
            LFD(dyn->v.x87reg[i], offsetof(x64emu_t, x87), s1);
            MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);
            // ok
            return;
        }
    // Was not in the cache? creating it....
    MESSAGE(LOG_DUMP, "\tCreate x87 Cache for ST%d\n", st);
    // get a free spot
    int ret = -1;
    for (int i = 0; (i < 8) && (ret == -1); ++i)
        if (dyn->v.x87cache[i] == -1)
            ret = i;
    // found, setup and grab the value
    dyn->v.x87cache[ret] = st;
    dyn->v.x87reg[ret] = fpu_get_reg_x87(dyn, VMX_CACHE_ST_D, st);
    LWZ(s2, offsetof(x64emu_t, top), xEmu);
    int a = st - dyn->v.x87stack;
    ADDI(s2, s2, a);
    ANDId(s2, s2, 7); // (emu->top + i)&7
    SLDI(s2, s2, 3);
    ADD(s1, xEmu, s2);
    LFD(dyn->v.x87reg[ret], offsetof(x64emu_t, x87), s1);
    MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);
}

void x87_free(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int st)
{
    dyn->insts[ninst].x87_used = 1;
    int ret = -1;
    for (int i = 0; (i < 8) && (ret == -1); ++i)
        if (dyn->v.x87cache[i] == st)
            ret = i;
    MESSAGE(LOG_DUMP, "\tFFREE%s x87 Cache for ST%d\n", (ret != -1) ? " (and Forget)" : "", st);
    if (ret != -1) {
        const int reg = dyn->v.x87reg[ret];
#if STEP == 1
        if (dyn->v.vmxcache[reg].t == VMX_CACHE_ST_F || dyn->v.vmxcache[reg].t == VMX_CACHE_ST_I64)
            vmxcache_promote_double(dyn, ninst, st);
#endif
        // Get top
        LWZ(s2, offsetof(x64emu_t, top), xEmu);
        // Update
        int ast = st - dyn->v.x87stack;
        if (ast) {
            ADDI(s2, s2, ast);
            ANDId(s2, s2, 7); // (emu->top + i)&7
        }
        SLDI(s2, s2, 3);
        ADD(s1, xEmu, s2);
        if (dyn->v.vmxcache[reg].t == VMX_CACHE_ST_F) {
            FMR(SCRATCH0, reg);
            STFD(SCRATCH0, offsetof(x64emu_t, x87), s1);
        } else if (dyn->v.vmxcache[reg].t == VMX_CACHE_ST_I64) {
            FCFID(SCRATCH0, reg);
            STFD(SCRATCH0, offsetof(x64emu_t, x87), s1);
        } else {
            STFD(reg, offsetof(x64emu_t, x87), s1);
        }
        // and forget that cache
        fpu_free_reg(dyn, reg);
        dyn->v.vmxcache[reg].v = 0;
        dyn->v.x87cache[ret] = -1;
        dyn->v.x87reg[ret] = -1;
    } else {
        // Get top
        LWZ(s2, offsetof(x64emu_t, top), xEmu);
        // Update
        int ast = st - dyn->v.x87stack;
        if (ast) {
            ADDI(s2, s2, ast);
            ANDId(s2, s2, 7); // (emu->top + i)&7
        }
    }
    // add mark in the freed array
    dyn->v.tags |= 0b11 << (st * 2);
    MESSAGE(LOG_DUMP, "\t--------x87 FFREE for ST%d\n", st);
}

void x87_swapreg(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int a, int b)
{
    int i1, i2, i3;
    i1 = x87_get_cache(dyn, ninst, 1, s1, s2, b, X87_ST(b));
    i2 = x87_get_cache(dyn, ninst, 1, s1, s2, a, X87_ST(a));
    i3 = dyn->v.x87cache[i1];
    dyn->v.x87cache[i1] = dyn->v.x87cache[i2];
    dyn->v.x87cache[i2] = i3;
    // swap those too
    int j1, j2, j3;
    j1 = x87_get_vmxcache(dyn, ninst, s1, s2, b);
    j2 = x87_get_vmxcache(dyn, ninst, s1, s2, a);
    j3 = dyn->v.vmxcache[j1].n;
    dyn->v.vmxcache[j1].n = dyn->v.vmxcache[j2].n;
    dyn->v.vmxcache[j2].n = j3;
    // mark as swapped
    dyn->v.swapped = 1;
    dyn->v.combined1 = a;
    dyn->v.combined2 = b;
}

// ========================================================================
// Rounding Mode & Cache Coherency — Batch H6
// ========================================================================

// Set rounding according to cw flags, return reg to restore flags
// PPC64LE FPSCR RN field is bits 62:63 in PPC big-endian notation (the 2 LSBs)
// x87 cw bits 10:11 encode rounding: 0=Nearest, 1=Down, 2=Up, 3=Chop
// PPC64LE FPSCR RN: 0=Nearest, 1=TowardZero, 2=TowardsPositive, 3=TowardsNegative
// Mapping: 0->0, 1->3, 2->2, 3->1  =>  negate & mask with 3
int x87_setround(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2)
{
    MAYUSE(dyn);
    MAYUSE(ninst);
    MAYUSE(s1);
    MAYUSE(s2);
    // Save current FPSCR to s2 (returned for restoreround)
    MFFS(SCRATCH0);
    STFD(SCRATCH0, -8, xSP);
    LD(s2, -8, xSP);
    // Read x87 control word rounding bits [11:10]
    LWZ(s1, offsetof(x64emu_t, cw), xEmu);
    BF_EXTRACT(s1, s1, 11, 10);
    // Remap: 0->0, 1->3, 2->2, 3->1
    NEG(s1, s1);
    ANDId(s1, s1, 3);
    // Build new FPSCR: (old & ~3) | new_rn
    // Save new_rn to red zone, use s1 for combining
    STD(s1, -16, xSP);
    RLDICR(s1, s2, 0, 61);     // s1 = old FPSCR with bottom 2 bits cleared
    LD(s2, -16, xSP);          // s2 = new_rn (temporarily)
    OR(s1, s1, s2);             // s1 = new FPSCR
    // Reload old FPSCR back into s2 (the return value)
    LD(s2, -8, xSP);
    // Set new FPSCR
    STD(s1, -8, xSP);
    LFD(SCRATCH0, -8, xSP);
    MTFSF(0xFF, SCRATCH0);
    return s2;
}

// Set rounding according to mxcsr flags, return reg to restore flags
int sse_setround(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2)
{
    MAYUSE(dyn);
    MAYUSE(ninst);
    MAYUSE(s1);
    MAYUSE(s2);
    // Save current FPSCR to s2 (returned for restoreround)
    MFFS(SCRATCH0);
    STFD(SCRATCH0, -8, xSP);
    LD(s2, -8, xSP);
    // Read mxcsr rounding bits [14:13]
    LWZ(s1, offsetof(x64emu_t, mxcsr), xEmu);
    BF_EXTRACT(s1, s1, 14, 13);
    // Remap: 0->0, 1->3, 2->2, 3->1
    NEG(s1, s1);
    ANDId(s1, s1, 3);
    // Build new FPSCR: (old & ~3) | new_rn
    STD(s1, -16, xSP);
    RLDICR(s1, s2, 0, 61);     // s1 = old FPSCR with bottom 2 bits cleared
    LD(s2, -16, xSP);          // s2 = new_rn (temporarily)
    OR(s1, s1, s2);             // s1 = new FPSCR
    // Reload old FPSCR back into s2 (the return value)
    LD(s2, -8, xSP);
    // Set new FPSCR
    STD(s1, -8, xSP);
    LFD(SCRATCH0, -8, xSP);
    MTFSF(0xFF, SCRATCH0);
    return s2;
}

int vmxcache_st_coherency(dynarec_ppc64le_t* dyn, int ninst, int a, int b)
{
    int i1 = vmxcache_get_st(dyn, ninst, a);
    int i2 = vmxcache_get_st(dyn, ninst, b);
    if (i1 != i2) {
        MESSAGE(LOG_DUMP, "Warning, ST cache incoherent between ST%d(%d) and ST%d(%d)\n", a, i1, b, i2);
    }

    return i1;
}

// Restore round flag
void x87_restoreround(dynarec_ppc64le_t* dyn, int ninst, int s1)
{
    MAYUSE(dyn);
    MAYUSE(ninst);
    MAYUSE(s1);
    // s1 contains the old FPSCR value saved by x87_setround/sse_setround
    STD(s1, -8, xSP);
    LFD(SCRATCH0, -8, xSP);
    MTFSF(0xFF, SCRATCH0);
}

// ========================================================================
// x87 Push/Pop — Batch H7
// ========================================================================

// On step 1, Float/Double for ST is actually computed and back-propagated
// On step 2-3, the value is just read for inst[...].v.vmxcache[..]
// the reg returned is *2 for FLOAT
int x87_do_push(dynarec_ppc64le_t* dyn, int ninst, int s1, int t)
{
    dyn->insts[ninst].x87_used = 1;
    if (dyn->v.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    dyn->v.x87stack += 1;
    dyn->v.stack += 1;
    dyn->v.stack_next += 1;
    dyn->v.stack_push += 1;
    ++dyn->v.pushed;
    if (dyn->v.poped)
        --dyn->v.poped;
    // move all regs in cache, and find a free one
    for (int j = 0; j < 32; ++j)
        if ((dyn->v.vmxcache[j].t == VMX_CACHE_ST_D)
            || (dyn->v.vmxcache[j].t == VMX_CACHE_ST_F)
            || (dyn->v.vmxcache[j].t == VMX_CACHE_ST_I64))
            ++dyn->v.vmxcache[j].n;
    int ret = -1;
    dyn->v.tags <<= 2;
    for (int i = 0; i < 8; ++i)
        if (dyn->v.x87cache[i] != -1)
            ++dyn->v.x87cache[i];
        else if (ret == -1) {
            dyn->v.x87cache[i] = 0;
            ret = dyn->v.x87reg[i] = fpu_get_reg_x87(dyn, t, 0);
            dyn->v.vmxcache[ret].t = X87_ST0;
        }
    if (ret == -1) {
        MESSAGE(LOG_DUMP, "Incoherent x87 stack cache, aborting\n");
        dyn->abort = 1;
    }
    return ret;
}

void x87_do_push_empty(dynarec_ppc64le_t* dyn, int ninst, int s1)
{
    dyn->insts[ninst].x87_used = 1;
    if (dyn->v.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    dyn->v.x87stack += 1;
    dyn->v.stack += 1;
    dyn->v.stack_next += 1;
    dyn->v.stack_push += 1;
    ++dyn->v.pushed;
    if (dyn->v.poped)
        --dyn->v.poped;
    // move all regs in cache
    for (int j = 0; j < 32; ++j)
        if ((dyn->v.vmxcache[j].t == VMX_CACHE_ST_D)
            || (dyn->v.vmxcache[j].t == VMX_CACHE_ST_F)
            || (dyn->v.vmxcache[j].t == VMX_CACHE_ST_I64))
            ++dyn->v.vmxcache[j].n;
    int ret = -1;
    dyn->v.tags <<= 2;
    for (int i = 0; i < 8; ++i)
        if (dyn->v.x87cache[i] != -1)
            ++dyn->v.x87cache[i];
        else if (ret == -1)
            ret = i;
    if (ret == -1) {
        MESSAGE(LOG_DUMP, "Incoherent x87 stack cache, aborting\n");
        dyn->abort = 1;
    }
}

static void internal_x87_dopop(dynarec_ppc64le_t* dyn)
{
    for (int i = 0; i < 8; ++i)
        if (dyn->v.x87cache[i] != -1) {
            --dyn->v.x87cache[i];
            if (dyn->v.x87cache[i] == -1) {
                fpu_free_reg(dyn, dyn->v.x87reg[i]);
                dyn->v.x87reg[i] = -1;
            }
        }
}

static int internal_x87_dofree(dynarec_ppc64le_t* dyn)
{
    if (dyn->v.tags & 0b11) {
        MESSAGE(LOG_DUMP, "\t--------x87 FREED ST0, poping 1 more\n");
        return 1;
    }
    return 0;
}

void x87_do_pop(dynarec_ppc64le_t* dyn, int ninst, int s1)
{
    dyn->insts[ninst].x87_used = 1;
    if (dyn->v.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    do {
        dyn->v.x87stack -= 1;
        dyn->v.stack_next -= 1;
        dyn->v.stack_pop += 1;
        dyn->v.tags >>= 2;
        ++dyn->v.poped;
        if (dyn->v.pushed)
            --dyn->v.pushed;
        // move all regs in cache, poping ST0
        internal_x87_dopop(dyn);
    } while (internal_x87_dofree(dyn));
}

void x87_purgecache(dynarec_ppc64le_t* dyn, int ninst, int next, int s1, int s2, int s3)
{
    dyn->insts[ninst].x87_used = 1;
    int ret = 0;
    for (int i = 0; i < 8 && !ret; ++i)
        if (dyn->v.x87cache[i] != -1)
            ret = 1;
    if (!ret && !dyn->v.x87stack) // nothing to do
        return;
    MESSAGE(LOG_DUMP, "\tPurge %sx87 Cache and Synch Stackcount (%+d)---\n", next ? "locally " : "", dyn->v.x87stack);
    int a = dyn->v.x87stack;
    if (a != 0) {
        // reset x87stack
        if (!next)
            dyn->v.x87stack = 0;
        // Add x87stack to emu fpu_stack
        LWZ(s2, offsetof(x64emu_t, fpu_stack), xEmu);
        ADDI(s2, s2, a);
        STW(s2, offsetof(x64emu_t, fpu_stack), xEmu);
        // Sub x87stack to top, with and 7
        LWZ(s2, offsetof(x64emu_t, top), xEmu);
        ADDI(s2, s2, -a);
        ANDId(s2, s2, 7);
        STW(s2, offsetof(x64emu_t, top), xEmu);
        // update tags
        LHZ(s1, offsetof(x64emu_t, fpu_tags), xEmu);
        if (a > 0) {
            SLDI(s1, s1, a * 2);
        } else {
            MOV32w(s3, 0xffff0000);
            OR(s1, s1, s3);
            SRDI(s1, s1, -a * 2);
        }
        STH(s1, offsetof(x64emu_t, fpu_tags), xEmu);
    } else {
        LWZ(s2, offsetof(x64emu_t, top), xEmu);
    }
    // check if free is used
    if (dyn->v.tags) {
        LHZ(s1, offsetof(x64emu_t, fpu_tags), xEmu);
        MOV32w(s3, dyn->v.tags);
        OR(s1, s1, s3);
        STH(s1, offsetof(x64emu_t, fpu_tags), xEmu);
    }
    if (ret != 0) {
        // --- set values
        // Get top
        // loop all cache entries
        for (int i = 0; i < 8; ++i)
            if (dyn->v.x87cache[i] != -1) {
                int st = dyn->v.x87cache[i] + dyn->v.stack_pop;
                // don't force promotion here
                ADDI(s3, s2, dyn->v.x87cache[i]); // unadjusted count, as it's relative to real top
                ANDId(s3, s3, 7);                  // (emu->top + st)&7
                SLDI(s1, s3, 3);
                ADD(s1, xEmu, s1);
                switch (vmxcache_get_current_st(dyn, ninst, st)) {
                    case VMX_CACHE_ST_D:
                        STFD(dyn->v.x87reg[i], offsetof(x64emu_t, x87), s1); // save the value
                        break;
                    case VMX_CACHE_ST_F:
                        // PPC64LE FPRs always hold doubles; ST_F value is already double
                        FMR(SCRATCH0, dyn->v.x87reg[i]);
                        STFD(SCRATCH0, offsetof(x64emu_t, x87), s1); // save the value
                        break;
                    case VMX_CACHE_ST_I64:
                        FCFID(SCRATCH0, dyn->v.x87reg[i]);
                        STFD(SCRATCH0, offsetof(x64emu_t, x87), s1); // save the value
                        break;
                }
                if (!next) {
                    fpu_free_reg(dyn, dyn->v.x87reg[i]);
                    dyn->v.x87reg[i] = -1;
                    dyn->v.x87cache[i] = -1;
                    // dyn->v.stack_pop+=1; //no pop, but the purge because of barrier will have the v.barrier flags set
                }
            }
    }
    if (!next) {
        dyn->v.stack_next = 0;
        dyn->v.tags = 0;
#if STEP < 2
        // refresh the cached valued, in case it's a purge outside a instruction
        dyn->insts[ninst].v.barrier = 1;
        dyn->v.pushed = 0;
        dyn->v.poped = 0;

#endif
    }
    MESSAGE(LOG_DUMP, "\t---Purge x87 Cache and Synch Stackcount\n");
}

void x87_reflectcount(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2)
{
    // Synch top and stack count
    int a = dyn->v.x87stack;
    if (a) {
        MESSAGE(LOG_DUMP, "\tSync x87 Count of %d-----\n", a);
        // Add x87stack to emu fpu_stack
        LWZ(s2, offsetof(x64emu_t, fpu_stack), xEmu);
        ADDI(s2, s2, a);
        STW(s2, offsetof(x64emu_t, fpu_stack), xEmu);
        // Sub x87stack to top, with and 7
        LWZ(s2, offsetof(x64emu_t, top), xEmu);
        ADDI(s2, s2, -a);
        ANDId(s2, s2, 7);
        STW(s2, offsetof(x64emu_t, top), xEmu);
        // update tags
        LHZ(s1, offsetof(x64emu_t, fpu_tags), xEmu);
        if (a > 0) {
            SLDI(s1, s1, a * 2);
        } else {
            MOV32w(s2, 0xffff0000);
            OR(s1, s1, s2);
            SRDI(s1, s1, -a * 2);
        }
        STH(s1, offsetof(x64emu_t, fpu_tags), xEmu);
        MESSAGE(LOG_DUMP, "\t-----Sync x87 Count\n");
    }
}

static void x87_reflectcache(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3)
{
    // Sync top and stack count
    int a = dyn->v.x87stack;
    if (a) {
        // Add x87stack to emu fpu_stack
        LWZ(s2, offsetof(x64emu_t, fpu_stack), xEmu);
        ADDI(s2, s2, a);
        STW(s2, offsetof(x64emu_t, fpu_stack), xEmu);
        // Sub x87stack to top, with and 7
        LWZ(s2, offsetof(x64emu_t, top), xEmu);
        ADDI(s2, s2, -a);
        ANDId(s2, s2, 7);
        STW(s2, offsetof(x64emu_t, top), xEmu);
        // update tags
        LHZ(s1, offsetof(x64emu_t, fpu_tags), xEmu);
        if (a > 0) {
            SLDI(s1, s1, a * 2);
        } else {
            MOV32w(s3, 0xffff0000);
            OR(s1, s1, s3);
            SRDI(s1, s1, -a * 2);
        }
        STH(s1, offsetof(x64emu_t, fpu_tags), xEmu);
    }
    int ret = 0;
    for (int i = 0; (i < 8) && (!ret); ++i)
        if (dyn->v.x87cache[i] != -1)
            ret = 1;
    if (!ret) // nothing to do
        return;
    // prepare offset to fpu => s1
    // Get top
    if (!a) {
        LWZ(s2, offsetof(x64emu_t, top), xEmu);
    }
    // loop all cache entries
    for (int i = 0; i < 8; ++i)
        if (dyn->v.x87cache[i] != -1) {
            ADDI(s3, s2, dyn->v.x87cache[i]);
            ANDId(s3, s3, 7); // (emu->top + i)&7
            SLDI(s1, s3, 3);
            ADD(s1, xEmu, s1);
            if (vmxcache_get_current_st_f(dyn, dyn->v.x87cache[i]) >= 0) {
                // PPC64LE FPRs always hold doubles; ST_F is already double, just copy
                FMR(SCRATCH0, dyn->v.x87reg[i]);
                STFD(SCRATCH0, offsetof(x64emu_t, x87), s1);
            } else
                STFD(dyn->v.x87reg[i], offsetof(x64emu_t, x87), s1);
        }
}

void x87_unreflectcount(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2)
{
    // revert top and stack count
    int a = dyn->v.x87stack;
    if (a) {
        // Sub x87stack to emu fpu_stack
        LWZ(s2, offsetof(x64emu_t, fpu_stack), xEmu);
        ADDI(s2, s2, -a);
        STW(s2, offsetof(x64emu_t, fpu_stack), xEmu);
        // Add x87stack to top, with and 7
        LWZ(s2, offsetof(x64emu_t, top), xEmu);
        ADDI(s2, s2, a);
        ANDId(s2, s2, 7);
        STW(s2, offsetof(x64emu_t, top), xEmu);
        // update tags
        LHZ(s1, offsetof(x64emu_t, fpu_tags), xEmu);
        if (a > 0) {
            MOV32w(s2, 0xffff0000);
            OR(s1, s1, s2);
            SRDI(s1, s1, a * 2);
        } else {
            SLDI(s1, s1, -a * 2);
        }
        STH(s1, offsetof(x64emu_t, fpu_tags), xEmu);
    }
}

int x87_get_current_cache(dynarec_ppc64le_t* dyn, int ninst, int st, int t)
{
    dyn->insts[ninst].x87_used = 1;
    // search in cache first
    for (int i = 0; i < 8; ++i) {
        if (dyn->v.x87cache[i] == st) {
#if STEP == 1
            if (t == VMX_CACHE_ST_D && (dyn->v.vmxcache[dyn->v.x87reg[i]].t == VMX_CACHE_ST_F || dyn->v.vmxcache[dyn->v.x87reg[i]].t == VMX_CACHE_ST_I64))
                vmxcache_promote_double(dyn, ninst, st);
            else if (t == VMX_CACHE_ST_I64 && (dyn->v.vmxcache[dyn->v.x87reg[i]].t == VMX_CACHE_ST_F))
                vmxcache_promote_double(dyn, ninst, st);
            else if (t == VMX_CACHE_ST_F && (dyn->v.vmxcache[dyn->v.x87reg[i]].t == VMX_CACHE_ST_I64))
                vmxcache_promote_double(dyn, ninst, st);
#endif
            return i;
        }
        assert(dyn->v.x87cache[i] < 8);
    }
    return -1;
}

int x87_get_cache(dynarec_ppc64le_t* dyn, int ninst, int populate, int s1, int s2, int st, int t)
{
    dyn->insts[ninst].x87_used = 1;
    if (dyn->v.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    int ret = x87_get_current_cache(dyn, ninst, st, t);
    if (ret != -1)
        return ret;
    MESSAGE(LOG_DUMP, "\tCreate %sx87 Cache for ST%d\n", populate ? "and populate " : "", st);
    // get a free spot
    for (int i = 0; (i < 8) && (ret == -1); ++i)
        if (dyn->v.x87cache[i] == -1)
            ret = i;
    // found, setup and grab the value
    dyn->v.x87cache[ret] = st;
    dyn->v.x87reg[ret] = fpu_get_reg_x87(dyn, VMX_CACHE_ST_D, st);
    if (populate) {
        LWZ(s2, offsetof(x64emu_t, top), xEmu);
        int a = st - dyn->v.x87stack;
        if (a) {
            ADDI(s2, s2, a);
            ANDId(s2, s2, 7);
        }
        SLDI(s2, s2, 3);
        ADD(s1, xEmu, s2);
        LFD(dyn->v.x87reg[ret], offsetof(x64emu_t, x87), s1);
    }
    MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);

    return ret;
}

int x87_get_vmxcache(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int st)
{
    dyn->insts[ninst].x87_used = 1;
    for (int ii = 0; ii < 32; ++ii)
        if ((dyn->v.vmxcache[ii].t == VMX_CACHE_ST_F
                || dyn->v.vmxcache[ii].t == VMX_CACHE_ST_D
                || dyn->v.vmxcache[ii].t == VMX_CACHE_ST_I64)
            && dyn->v.vmxcache[ii].n == st)
            return ii;
    dynarec_log(LOG_NONE, "Warning: x87_get_vmxcache didn't find cache for ninst=%d\n", ninst);
    return -1;
}

int x87_get_st(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int a, int t)
{
    dyn->insts[ninst].x87_used = 1;
    return dyn->v.x87reg[x87_get_cache(dyn, ninst, 1, s1, s2, a, t)];
}

int x87_get_st_empty(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int a, int t)
{
    dyn->insts[ninst].x87_used = 1;
    return dyn->v.x87reg[x87_get_cache(dyn, ninst, 0, s1, s2, a, t)];
}

// MMX helpers
static int isx87Empty(dynarec_ppc64le_t* dyn)
{
    for (int i = 0; i < 8; ++i)
        if (dyn->v.x87cache[i] != -1)
            return 0;
    return 1;
}

// get vmx register for a MMX reg, create the entry if needed
int mmx_get_reg(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int a)
{
    dyn->insts[ninst].mmx_used = 1;
    if (!dyn->v.x87stack && isx87Empty(dyn))
        x87_purgecache(dyn, ninst, 0, s1, s2, s3);
    if (dyn->v.mmxcache[a] != -1)
        return dyn->v.mmxcache[a];
    ++dyn->v.mmxcount;
    int ret = dyn->v.mmxcache[a] = fpu_get_reg_emm(dyn, a);
    // MMX lives in VR space (vr24-vr31); load 64-bit value via GPR then move to VR
    LD(s1, offsetof(x64emu_t, mmx[a]), xEmu);
    MTVSRDD(VSXREG(ret), xZR, s1);  // high=0, low=data
    return ret;
}

// get vmx register for a MMX reg, but don't try to synch it if it needed to be created
int mmx_get_reg_empty(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int a)
{
    dyn->insts[ninst].mmx_used = 1;
    if (!dyn->v.x87stack && isx87Empty(dyn))
        x87_purgecache(dyn, ninst, 0, s1, s2, s3);
    if (dyn->v.mmxcache[a] != -1)
        return dyn->v.mmxcache[a];
    ++dyn->v.mmxcount;
    int ret = dyn->v.mmxcache[a] = fpu_get_reg_emm(dyn, a);
    return ret;
}

// purge the MMX cache only
void mmx_purgecache(dynarec_ppc64le_t* dyn, int ninst, int next, int s1)
{
    if (!dyn->v.mmxcount) return;
    if (!next) dyn->v.mmxcount = 0;
    int old = -1;
    for (int i = 0; i < 8; ++i)
        if (dyn->v.mmxcache[i] != -1) {
            if (old == -1) {
                MESSAGE(LOG_DUMP, "\tPurge %sMMX Cache ------\n", next ? "locally " : "");
                ++old;
            }
            // MMX lives in VR space; store via GPR
            MFVSRLD(s1, VSXREG(dyn->v.mmxcache[i]));
            STD(s1, offsetof(x64emu_t, mmx[i]), xEmu);
            if (!next) {
                fpu_free_reg(dyn, dyn->v.mmxcache[i]);
                dyn->v.mmxcache[i] = -1;
            }
        }
    if (old != -1) {
        MESSAGE(LOG_DUMP, "\t------ Purge MMX Cache\n");
    }
}

static void mmx_reflectcache(dynarec_ppc64le_t* dyn, int ninst, int s1)
{
    for (int i = 0; i < 8; ++i)
        if (dyn->v.mmxcache[i] != -1) {
            // MMX lives in VR space; store via GPR
            MFVSRLD(s1, VSXREG(dyn->v.mmxcache[i]));
            STD(s1, offsetof(x64emu_t, mmx[i]), xEmu);
        }
}

// SSE / SSE2 helpers
// get vmx register for a SSE reg, create the entry if needed
int sse_get_reg(dynarec_ppc64le_t* dyn, int ninst, int s1, int a, int forwrite)
{
    dyn->v.xmm_used |= 1 << a;
    if (dyn->v.ssecache[a].v != -1) {
        if (forwrite) {
            dyn->v.ssecache[a].write = 1; // update only if forwrite
            dyn->v.vmxcache[dyn->v.ssecache[a].reg].t = VMX_CACHE_XMMW;
        }
        return dyn->v.ssecache[a].reg;
    }
    int need_vld = 1;
    int avx_was_dirty = 0;
    // migrate from avx to sse
    if (dyn->v.avxcache[a].v != -1) {
        avx_was_dirty = dyn->v.avxcache[a].write;
        avx_reflect_reg_upper128(dyn, ninst, a, forwrite);
        dyn->v.avxcache[a].v = -1;
        need_vld = 0;
    }
    int is_write = forwrite || avx_was_dirty;
    dyn->v.ssecache[a].reg = fpu_get_reg_xmm(dyn, is_write ? VMX_CACHE_XMMW : VMX_CACHE_XMMR, a);
    int ret = dyn->v.ssecache[a].reg;
    dyn->v.ssecache[a].write = is_write;
    if(need_vld) LXV(VSXREG(ret), offsetof(x64emu_t, xmm[a]), xEmu); //skip load if migrate from avx
    return ret;
}

// get vmx register for an SSE reg, but don't try to synch it if it needed to be created
int sse_get_reg_empty(dynarec_ppc64le_t* dyn, int ninst, int s1, int a)
{
    dyn->v.xmm_used |= 1 << a;
    if (dyn->v.ssecache[a].v != -1) {
        dyn->v.ssecache[a].write = 1;
        dyn->v.vmxcache[dyn->v.ssecache[a].reg].t = VMX_CACHE_XMMW;
        return dyn->v.ssecache[a].reg;
    }
    // migrate from avx to sse
    if (dyn->v.avxcache[a].v != -1) {
        avx_reflect_reg_upper128(dyn, ninst, a, 1);
        dyn->v.avxcache[a].v = -1;
    }
    dyn->v.ssecache[a].reg = fpu_get_reg_xmm(dyn, VMX_CACHE_XMMW, a);
    dyn->v.ssecache[a].write = 1; // it will be write...
    return dyn->v.ssecache[a].reg;
}

// forget ext register for a SSE reg, does nothing if the reg is not loaded
void sse_forget_reg(dynarec_ppc64le_t* dyn, int ninst, int a)
{
    dyn->v.xmm_used |= 1 << a;
    if (dyn->v.ssecache[a].v == -1)
        return;
    if (dyn->v.vmxcache[dyn->v.ssecache[a].reg].t == VMX_CACHE_XMMW) {
        STXV(VSXREG(dyn->v.ssecache[a].reg), offsetof(x64emu_t, xmm[a]), xEmu);
    }
    fpu_free_reg(dyn, dyn->v.ssecache[a].reg);
    dyn->v.ssecache[a].v = -1;
    return;
}

void sse_reflect_reg(dynarec_ppc64le_t* dyn, int ninst, int a)
{
    dyn->v.xmm_used |= 1 << a;
    if (dyn->v.ssecache[a].v == -1)
        return;
    if (dyn->v.vmxcache[dyn->v.ssecache[a].reg].t == VMX_CACHE_XMMW) {
        STXV(VSXREG(dyn->v.ssecache[a].reg), offsetof(x64emu_t, xmm[a]), xEmu);
    }
}

// purge the SSE cache for XMM0..XMM7 (to use before function native call)
void sse_purge07cache(dynarec_ppc64le_t* dyn, int ninst, int s1)
{
    int old = -1;
    for (int i = 0; i < 8; ++i)
        if (dyn->v.ssecache[i].v != -1 || dyn->v.avxcache[i].v != -1) {
            if (old == -1) {
                MESSAGE(LOG_DUMP, "\tPurge XMM0..7 Cache ------\n");
                ++old;
            }
            dyn->v.xmm_used |= 1 << i;
            if (dyn->v.vmxcache[dyn->v.avxcache[i].reg].t == VMX_CACHE_YMMW) {
                STXV(VSXREG(dyn->v.avxcache[i].reg), offsetof(x64emu_t, xmm[i]), xEmu);
                if(dyn->v.avxcache[i].zero_upper == 1){
                    XXLXOR(SCRATCH, SCRATCH, SCRATCH);
                } else {
                    // upper 128 is already in ymm[i] memory, no need to extract
                }
                STXV(SCRATCH, offsetof(x64emu_t, ymm[i]), xEmu);
                fpu_free_reg(dyn, dyn->v.avxcache[i].reg);
                dyn->v.avxcache[i].v = -1;
            } else if (dyn->v.vmxcache[dyn->v.ssecache[i].reg].t == VMX_CACHE_XMMW) {
                STXV(VSXREG(dyn->v.ssecache[i].reg), offsetof(x64emu_t, xmm[i]), xEmu);
                fpu_free_reg(dyn, dyn->v.ssecache[i].reg);
                dyn->v.ssecache[i].v = -1;
            }
        }
    if (old != -1) {
        MESSAGE(LOG_DUMP, "\t------ Purge XMM0..7 Cache\n");
    }
}

// purge the SSE cache only
static void sse_purgecache(dynarec_ppc64le_t* dyn, int ninst, int next, int s1)
{
    int old = -1;
    for (int i = 0; i < 16; ++i)
        if (dyn->v.ssecache[i].v != -1) {
            if (next) dyn->v.xmm_used |= (1 << i);
            if (dyn->v.ssecache[i].write) {
                if (old == -1) {
                    MESSAGE(LOG_DUMP, "\tPurge %sSSE Cache ------\n", next ? "locally " : "");
                    ++old;
                }
                MESSAGE(LOG_DUMP, "\t  xmm[%d]: reg=%d, vsxreg=%d, write=%d, offset=0x%lx\n",
                    i, dyn->v.ssecache[i].reg, VSXREG(dyn->v.ssecache[i].reg),
                    dyn->v.ssecache[i].write,
                    (unsigned long)offsetof(x64emu_t, xmm[i]));
                STXV(VSXREG(dyn->v.ssecache[i].reg), offsetof(x64emu_t, xmm[i]), xEmu);
            }
            if (!next) {
                fpu_free_reg(dyn, dyn->v.ssecache[i].reg);
                dyn->v.ssecache[i].v = -1;
            }
        }
    if (old != -1) {
        MESSAGE(LOG_DUMP, "\t------ Purge SSE Cache\n");
    }
}

static void sse_reflectcache(dynarec_ppc64le_t* dyn, int ninst, int s1)
{
    for (int i = 0; i < 16; ++i)
        if (dyn->v.ssecache[i].v != -1) {
            dyn->v.xmm_used |= 1 << i;
            if (dyn->v.ssecache[i].write) {
                STXV(VSXREG(dyn->v.ssecache[i].reg), offsetof(x64emu_t, xmm[i]), xEmu);
            }
        }
}

// AVX helpers
// get vmx register for an AVX reg, create the entry if needed
int avx_get_reg(dynarec_ppc64le_t* dyn, int ninst, int s1, int a, int forwrite, int width)
{
    dyn->v.ymm_used |= 1 << a;
    if (dyn->v.avxcache[a].v != -1) {
        if (forwrite) {
            dyn->v.avxcache[a].write = 1; // update only if forwrite
            dyn->v.vmxcache[dyn->v.avxcache[a].reg].t = VMX_CACHE_YMMW;
        }
        if (width == VMX_AVX_WIDTH_128) {
            dyn->v.avxcache[a].width = VMX_AVX_WIDTH_128;
            if(forwrite) dyn->v.avxcache[a].zero_upper = 1;
        } else {
            // if width changed to 256, and zero_upper==1, means need zero-fill upper 128bits now
            if (dyn->v.avxcache[a].zero_upper == 1) {
                dyn->v.avxcache[a].zero_upper = 0;
                // store zeros to ymm[a] memory (upper 128 bits)
                XXLXOR(SCRATCH, SCRATCH, SCRATCH);
                STXV(SCRATCH, offsetof(x64emu_t, ymm[a]), xEmu);
            }
            dyn->v.avxcache[a].width = VMX_AVX_WIDTH_256;
        }

        return dyn->v.avxcache[a].reg;
    }

    // migrate from sse to avx
    if (dyn->v.ssecache[a].v != -1) {
        // release SSE reg cache
        fpu_free_reg(dyn, dyn->v.ssecache[a].reg);
        dyn->v.ssecache[a].v = -1;
    }

    // new reg
    dyn->v.avxcache[a].v = 0;
    dyn->v.avxcache[a].reg = fpu_get_reg_ymm(dyn, forwrite ? VMX_CACHE_YMMW : VMX_CACHE_YMMR, a);
    int ret = dyn->v.avxcache[a].reg;
    dyn->v.avxcache[a].write = forwrite;
    dyn->v.avxcache[a].width = width;
    if (width == VMX_AVX_WIDTH_128) {
        if(forwrite) dyn->v.avxcache[a].zero_upper = 1;
        LXV(VSXREG(ret), offsetof(x64emu_t, xmm[a]), xEmu);
    } else {
        // load lower 128 from xmm[a]
        LXV(VSXREG(ret), offsetof(x64emu_t, xmm[a]), xEmu);
        // upper 128 stays in ymm[a] memory — nothing to load into register
        dyn->v.avxcache[a].zero_upper = 0;
    }
    return ret;
}

int avx_get_reg_empty(dynarec_ppc64le_t* dyn, int ninst, int s1, int a, int width)
{
    dyn->v.ymm_used |= 1 << a;
    if (dyn->v.avxcache[a].v != -1) {
        dyn->v.avxcache[a].write = 1;
        dyn->v.vmxcache[dyn->v.avxcache[a].reg].t = VMX_CACHE_YMMW;
        if (width == VMX_AVX_WIDTH_128) {
            dyn->v.avxcache[a].width = VMX_AVX_WIDTH_128;
            dyn->v.avxcache[a].zero_upper = 1;
        } else {
            dyn->v.avxcache[a].width = VMX_AVX_WIDTH_256;
            dyn->v.avxcache[a].zero_upper = 0;
        }
        return dyn->v.avxcache[a].reg;
    }
    // migrate from sse to avx
    if (dyn->v.ssecache[a].v != -1) {
        // Release SSE reg cache
        fpu_free_reg(dyn, dyn->v.ssecache[a].reg);
        dyn->v.ssecache[a].v = -1;
    }
    dyn->v.avxcache[a].v = 0;
    dyn->v.avxcache[a].reg = fpu_get_reg_ymm(dyn, VMX_CACHE_YMMW, a);
    dyn->v.avxcache[a].write = 1;
    dyn->v.avxcache[a].width = width;
    if (width == VMX_AVX_WIDTH_128){
        dyn->v.avxcache[a].zero_upper = 1;
    } else {
        dyn->v.avxcache[a].zero_upper = 0;
    }
    return dyn->v.avxcache[a].reg;
}

void avx_reflect_reg_upper128(dynarec_ppc64le_t* dyn, int ninst, int a, int forwrite)
{
    dyn->v.ymm_used |= 1 << a;
    if (dyn->v.avxcache[a].v == -1)
        return;
    // Write back upper 128 bits if the AVX register was dirty and zero_upper is set,
    // regardless of forwrite (the SSE caller needs ymm[a] memory consistent)
    if (dyn->v.vmxcache[dyn->v.avxcache[a].reg].t == VMX_CACHE_YMMW) {
        if (dyn->v.avxcache[a].zero_upper == 1) {
            XXLXOR(SCRATCH, SCRATCH, SCRATCH);
            STXV(SCRATCH, offsetof(x64emu_t, ymm[a]), xEmu);
        }
        // else: upper 128 is already in ymm[a] memory, no-op
    }
    dyn->v.avxcache[a].v = -1;
    return;
}

void avx_forget_reg(dynarec_ppc64le_t* dyn, int ninst, int a)
{
    dyn->v.ymm_used |= 1 << a;
    if (dyn->v.avxcache[a].v == -1)
        return;
    if (dyn->v.vmxcache[dyn->v.avxcache[a].reg].t == VMX_CACHE_YMMW) {
        STXV(VSXREG(dyn->v.avxcache[a].reg), offsetof(x64emu_t, xmm[a]), xEmu);
        if (dyn->v.avxcache[a].zero_upper == 1) {
            XXLXOR(SCRATCH, SCRATCH, SCRATCH);
        }
        // else: upper 128 is already in ymm[a] memory
        if (dyn->v.avxcache[a].zero_upper == 1) {
            STXV(SCRATCH, offsetof(x64emu_t, ymm[a]), xEmu);
        }
    }
    fpu_free_reg(dyn, dyn->v.avxcache[a].reg);
    dyn->v.avxcache[a].v = -1;
    return;
}

void avx_reflect_reg(dynarec_ppc64le_t* dyn, int ninst, int a)
{
    dyn->v.ymm_used |= 1 << a;
    if (dyn->v.avxcache[a].v == -1)
        return;
    if (dyn->v.vmxcache[dyn->v.avxcache[a].reg].t == VMX_CACHE_YMMW) {
        STXV(VSXREG(dyn->v.avxcache[a].reg), offsetof(x64emu_t, xmm[a]), xEmu);
        if (dyn->v.avxcache[a].zero_upper == 1) {
            XXLXOR(SCRATCH, SCRATCH, SCRATCH);
            STXV(SCRATCH, offsetof(x64emu_t, ymm[a]), xEmu);
        }
        // else: upper 128 is already in ymm[a] memory, no store needed
        dyn->v.avxcache[a].zero_upper = 0;
    }
}

// purge the AVX cache only
static void avx_purgecache(dynarec_ppc64le_t* dyn, int ninst, int next, int s1)
{
    int old = -1;
    for (int i = 0; i < 16; ++i)
        if (dyn->v.avxcache[i].v != -1) {
            dyn->v.ymm_used |= 1 << i;
            if (dyn->v.avxcache[i].write) {
                if (old == -1) {
                    MESSAGE(LOG_DUMP, "\tPurge %sAVX Cache ------\n", next ? "locally " : "");
                    ++old;
                }
                if (dyn->v.vmxcache[dyn->v.avxcache[i].reg].t == VMX_CACHE_YMMW) {
                    STXV(VSXREG(dyn->v.avxcache[i].reg), offsetof(x64emu_t, xmm[i]), xEmu);
                    if (dyn->v.avxcache[i].zero_upper == 1) {
                        XXLXOR(SCRATCH, SCRATCH, SCRATCH);
                    }
                    // else: upper 128 is already in ymm[i] memory
                    if (dyn->v.avxcache[i].zero_upper == 1) {
                        STXV(SCRATCH, offsetof(x64emu_t, ymm[i]), xEmu);
                    }
                }
            }
            if (!next) {
                fpu_free_reg(dyn, dyn->v.avxcache[i].reg);
                dyn->v.avxcache[i].v = -1;
            }
        }
    if (old != -1) {
        MESSAGE(LOG_DUMP, "\t------ Purge AVX Cache\n");
    }
}

static void avx_reflectcache(dynarec_ppc64le_t* dyn, int ninst, int s1)
{
    for (int i = 0; i < 16; ++i) {
        if (dyn->v.avxcache[i].v != -1) {
            dyn->v.ymm_used |= 1 << i;
            if (dyn->v.avxcache[i].write) {
                if (dyn->v.vmxcache[dyn->v.avxcache[i].reg].t == VMX_CACHE_YMMW) {
                    avx_reflect_reg(dyn, ninst, i);
                }
            }
        }
    }
}

void fpu_pushcache(dynarec_ppc64le_t* dyn, int ninst, int s1, int not07)
{
    int start = not07 ? 8 : 0;
    int n = 0;

    for (int i = start; i < 16; i++) {
        if ((dyn->v.ssecache[i].v != -1) && (dyn->v.ssecache[i].write))
            ++n;
        if ((dyn->v.avxcache[i].v != -1) && (dyn->v.avxcache[i].write))
            ++n;
    }

    if (n) {
        MESSAGE(LOG_DUMP, "\tPush XMM/YMM Cache (%d)------\n", n);
        for (int i = start; i < 16; ++i) {
            if ((dyn->v.ssecache[i].v != -1) && (dyn->v.ssecache[i].write)) {
                STXV(VSXREG(dyn->v.ssecache[i].reg), offsetof(x64emu_t, xmm[i]), xEmu);
            }
            if ((dyn->v.avxcache[i].v != -1) && (dyn->v.avxcache[i].write)) {
                STXV(VSXREG(dyn->v.avxcache[i].reg), offsetof(x64emu_t, xmm[i]), xEmu);
                // upper 128: if zero_upper==0, it's already in ymm[] memory; nothing to do
                // if zero_upper==1, we don't need to push zeros (will be handled on reflect/purge)
            }
        }
        MESSAGE(LOG_DUMP, "\t------- Pushed XMM/YMM Cache (%d)\n", n);
    }
}

void fpu_popcache(dynarec_ppc64le_t* dyn, int ninst, int s1, int not07)
{
    int start = not07 ? 8 : 0;
    int n = 0;

    for (int i = start; i < 16; i++) {
        if (dyn->v.ssecache[i].v != -1 || dyn->v.avxcache[i].v != -1)
            ++n;
    }

    if (n) {
        MESSAGE(LOG_DUMP, "\tPop XMM/YMM Cache (%d)------\n", n);
        for (int i = start; i < 16; ++i) {
            if (dyn->v.ssecache[i].v != -1) {
                LXV(VSXREG(dyn->v.ssecache[i].reg), offsetof(x64emu_t, xmm[i]), xEmu);
            }
            if (dyn->v.avxcache[i].v != -1) {
                LXV(VSXREG(dyn->v.avxcache[i].reg), offsetof(x64emu_t, xmm[i]), xEmu);
                // upper 128 stays in ymm[i] memory — nothing to load
            }
        }
        MESSAGE(LOG_DUMP, "\t------- Pop XMM/YMM Cache (%d)\n", n);
    }
}

void fpu_purgecache(dynarec_ppc64le_t* dyn, int ninst, int next, int s1, int s2, int s3)
{
    x87_purgecache(dyn, ninst, next, s1, s2, s3);
    mmx_purgecache(dyn, ninst, next, s1);
    sse_purgecache(dyn, ninst, next, s1);
    avx_purgecache(dyn, ninst, next, s1);
    if (!next) {
        fpu_reset_reg(dyn);
        dyn->insts[ninst].fpupurge = 1;
    }
}

void fpu_reflectcache(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3)
{
    x87_reflectcache(dyn, ninst, s1, s2, s3);
    mmx_reflectcache(dyn, ninst, s1);
    sse_reflectcache(dyn, ninst, s1);
    avx_reflectcache(dyn, ninst, s1);
}

void fpu_unreflectcache(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3)
{
    // need to undo the top and stack tracking that must not be reflected permanently yet
    x87_unreflectcount(dyn, ninst, s1, s2);
}

void emit_pf(dynarec_ppc64le_t* dyn, int ninst, int s1, int s3, int s4)
{
    MAYUSE(dyn);
    MAYUSE(ninst);

    // Compute parity of low byte: fold bits down to bit 0
    SRDI(s3, s1, 4);
    XOR(s3, s3, s1);

    SRDI(s4, s3, 2);
    XOR(s4, s3, s4);

    SRDI(s3, s4, 1);
    XOR(s3, s3, s4);

    XORI(s3, s3, 1);
    // Insert bit 0 of s3 into xFlags at F_PF position
    ANDId(s4, s3, 1);              // isolate bit 0 of s3, result in s4
    SLDI(s4, s4, F_PF);            // shift to F_PF position
    LI(s3, 1);
    SLDI(s3, s3, F_PF);
    ANDC(xFlags, xFlags, s3);      // xFlags &= ~(1 << F_PF)
    OR(xFlags, xFlags, s4);        // xFlags |= (pf_bit << F_PF)
}

void fpu_reset_cache(dynarec_ppc64le_t* dyn, int ninst, int reset_n)
{
    MESSAGE(LOG_DEBUG, "Reset Caches with %d\n", reset_n);
    #if STEP > 1
    // for STEP 2 & 3, just need to refresh with current, and undo the changes (push & swap)
    dyn->v = dyn->insts[ninst].v;
    vmxcacheUnwind(&dyn->v);
#else
    dyn->v = dyn->insts[reset_n].v;
#endif
#if STEP == 0
    if(dyn->need_dump && dyn->v.x87stack) dynarec_log(LOG_NONE, "New x87stack=%d at ResetCache in inst %d with %d\n", dyn->v.x87stack, ninst, reset_n);
#endif
#if defined(HAVE_TRACE) && (STEP > 2)
    if (dyn->need_dump && 0) // disable for now
        if (memcmp(&dyn->v, &dyn->insts[reset_n].v, sizeof(vmxcache_t))) {
            MESSAGE(LOG_DEBUG, "Warning, difference in vmxcache: reset=");
            for (int i = 0; i < 32; ++i)
                if (dyn->insts[reset_n].v.vmxcache[i].v)
                    MESSAGE(LOG_DEBUG, " %02d:%s", i, getCacheName(dyn->insts[reset_n].v.vmxcache[i].t, dyn->insts[reset_n].v.vmxcache[i].n));
            if (dyn->insts[reset_n].v.combined1 || dyn->insts[reset_n].v.combined2)
                MESSAGE(LOG_DEBUG, " %s:%02d/%02d", dyn->insts[reset_n].v.swapped ? "SWP" : "CMB", dyn->insts[reset_n].v.combined1, dyn->insts[reset_n].v.combined2);
            if (dyn->insts[reset_n].v.stack_push || dyn->insts[reset_n].v.stack_pop)
                MESSAGE(LOG_DEBUG, " (%d:%d)", dyn->insts[reset_n].v.stack_push, -dyn->insts[reset_n].v.stack_pop);
            MESSAGE(LOG_DEBUG, " ==> ");
            for (int i = 0; i < 32; ++i)
                if (dyn->insts[ninst].v.vmxcache[i].v)
                    MESSAGE(LOG_DEBUG, " %02d:%s", i, getCacheName(dyn->insts[ninst].v.vmxcache[i].t, dyn->insts[ninst].v.vmxcache[i].n));
            if (dyn->insts[ninst].v.combined1 || dyn->insts[ninst].v.combined2)
                MESSAGE(LOG_DEBUG, " %s:%02d/%02d", dyn->insts[ninst].v.swapped ? "SWP" : "CMB", dyn->insts[ninst].v.combined1, dyn->insts[ninst].v.combined2);
            if (dyn->insts[ninst].v.stack_push || dyn->insts[ninst].v.stack_pop)
                MESSAGE(LOG_DEBUG, " (%d:%d)", dyn->insts[ninst].v.stack_push, -dyn->insts[ninst].v.stack_pop);
            MESSAGE(LOG_DEBUG, " -> ");
            for (int i = 0; i < 32; ++i)
                if (dyn->v.vmxcache[i].v)
                    MESSAGE(LOG_DEBUG, " %02d:%s", i, getCacheName(dyn->v.vmxcache[i].t, dyn->v.vmxcache[i].n));
            if (dyn->v.combined1 || dyn->v.combined2)
                MESSAGE(LOG_DEBUG, " %s:%02d/%02d", dyn->v.swapped ? "SWP" : "CMB", dyn->v.combined1, dyn->v.combined2);
            if (dyn->v.stack_push || dyn->v.stack_pop)
                MESSAGE(LOG_DEBUG, " (%d:%d)", dyn->v.stack_push, -dyn->v.stack_pop);
            MESSAGE(LOG_DEBUG, "\n");
        }
#endif // HAVE_TRACE
}

// propagate ST stack state, especial stack pop that are deferred
void fpu_propagate_stack(dynarec_ppc64le_t* dyn, int ninst)
{
    if (dyn->v.stack_pop) {
        for (int j = 0; j < 32; ++j)
            if ((dyn->v.vmxcache[j].t == VMX_CACHE_ST_D
                    || dyn->v.vmxcache[j].t == VMX_CACHE_ST_F
                    || dyn->v.vmxcache[j].t == VMX_CACHE_ST_I64)) {
                if (dyn->v.vmxcache[j].n < dyn->v.stack_pop)
                    dyn->v.vmxcache[j].v = 0;
                else
                    dyn->v.vmxcache[j].n -= dyn->v.stack_pop;
            }
        dyn->v.stack_pop = 0;
    }
    dyn->v.stack = dyn->v.stack_next;
    dyn->v.news = 0;
    dyn->v.stack_push = 0;
    dyn->v.swapped = 0;
}


static int findCacheSlot(dynarec_ppc64le_t* dyn, int ninst, int t, int n, vmxcache_t* cache)
{
    vmx_cache_t f;
    f.n = n;
    f.t = t;
    for (int i = 0; i < 32; ++i) {
        if (cache->vmxcache[i].v == f.v)
            return i;
        if (cache->vmxcache[i].n == n) {
            switch (cache->vmxcache[i].t) {
                case VMX_CACHE_ST_F:
                    if (t == VMX_CACHE_ST_D)
                        return i;
                    if (t == VMX_CACHE_ST_I64)
                        return i;
                    break;
                case VMX_CACHE_ST_D:
                    if (t == VMX_CACHE_ST_F)
                        return i;
                    if (t == VMX_CACHE_ST_I64)
                        return i;
                    break;
                case VMX_CACHE_ST_I64:
                    if (t == VMX_CACHE_ST_F)
                        return i;
                    if (t == VMX_CACHE_ST_D)
                        return i;
                    break;
                case VMX_CACHE_XMMR:
                    if (t == VMX_CACHE_XMMW)
                        return i;
                    break;
                case VMX_CACHE_XMMW:
                    if (t == VMX_CACHE_XMMR)
                        return i;
                    break;
                case VMX_CACHE_YMMR:
                    if (t == VMX_CACHE_YMMW)
                        return i;
                    break;
                case VMX_CACHE_YMMW:
                    if (t == VMX_CACHE_YMMR)
                        return i;
                    break;
            }
        }
    }
    return -1;
}

static void swapCache(dynarec_ppc64le_t* dyn, int ninst, int i, int j, vmxcache_t* cache)
{
    if (i == j)
        return;
    int quad = 0;
    int mmx = 0;
    if (cache->vmxcache[i].t == VMX_CACHE_XMMR || cache->vmxcache[i].t == VMX_CACHE_XMMW)
        quad = 1;
    if (cache->vmxcache[j].t == VMX_CACHE_XMMR || cache->vmxcache[j].t == VMX_CACHE_XMMW)
        quad = 1;
    if (cache->vmxcache[i].t == VMX_CACHE_YMMR || cache->vmxcache[i].t == VMX_CACHE_YMMW)
        quad = 1;  // PPC64LE: YMM in register is still only 128-bit (lower half)
    if (cache->vmxcache[j].t == VMX_CACHE_YMMR || cache->vmxcache[j].t == VMX_CACHE_YMMW)
        quad = 1;  // PPC64LE: YMM in register is still only 128-bit (lower half)
    if (cache->vmxcache[i].t == VMX_CACHE_MM || cache->vmxcache[j].t == VMX_CACHE_MM)
        mmx = 1;   // MMX lives in VR space, needs XXLOR with VSXREG

    if (!cache->vmxcache[i].v) {
        // a mov is enough, no need to swap
        MESSAGE(LOG_DUMP, "\t  - Moving %d <- %d\n", i, j);
        if (quad) {
            XXLOR(VSXREG(i), VSXREG(j), VSXREG(j));     // 128-bit copy
        } else if (mmx) {
            XXLOR(VSXREG(i), VSXREG(j), VSXREG(j));     // MMX in VR space
        } else {
            FMR(i, j);          // 64-bit copy (x87 ST, indices 16-23 in FPR space)
        }
        cache->vmxcache[i].v = cache->vmxcache[j].v;
        cache->vmxcache[j].v = 0;
        return;
    }
    // SWAP
    vmx_cache_t tmp;
    MESSAGE(LOG_DUMP, "\t  - Swapping %d <-> %d\n", i, j);
    // Use SCRATCH as temporary
    if (quad) {
        XXLOR(SCRATCH, VSXREG(i), VSXREG(i));    // 128-bit swap via scratch
        XXLOR(VSXREG(i), VSXREG(j), VSXREG(j));
        XXLOR(VSXREG(j), SCRATCH, SCRATCH);
    } else if (mmx) {
        XXLOR(SCRATCH, VSXREG(i), VSXREG(i));    // MMX swap via scratch (VR space)
        XXLOR(VSXREG(i), VSXREG(j), VSXREG(j));
        XXLOR(VSXREG(j), SCRATCH, SCRATCH);
    } else {
        FMR(SCRATCH, i);         // 64-bit swap via scratch (x87 ST, FPR space)
        FMR(i, j);
        FMR(j, SCRATCH);
    }
    tmp.v = cache->vmxcache[i].v;
    cache->vmxcache[i].v = cache->vmxcache[j].v;
    cache->vmxcache[j].v = tmp.v;
}

static void loadCache(dynarec_ppc64le_t* dyn, int ninst, int stack_cnt, int s1, int s2, int s3, int* s1_val, int* s2_val, int* s3_top, vmxcache_t* cache, int i, int t, int n)
{
    if (cache->vmxcache[i].v) {
        int quad = 0;
        int mmx = 0;
        if (t == VMX_CACHE_XMMR || t == VMX_CACHE_XMMW)
            quad = 1;
        if (t == VMX_CACHE_YMMR || t == VMX_CACHE_YMMW)
            quad = 1;  // PPC64LE: only 128-bit in register
        if (cache->vmxcache[i].t == VMX_CACHE_XMMR || cache->vmxcache[i].t == VMX_CACHE_XMMW)
            quad = 1;
        if (cache->vmxcache[i].t == VMX_CACHE_YMMR || cache->vmxcache[i].t == VMX_CACHE_YMMW)
            quad = 1;
        if (t == VMX_CACHE_MM || cache->vmxcache[i].t == VMX_CACHE_MM)
            mmx = 1;
        int j = i + 1;
        while (cache->vmxcache[j].v)
            ++j;
        MESSAGE(LOG_DUMP, "\t  - Moving away %d\n", i);
        if (quad) {
            XXLOR(VSXREG(j), VSXREG(i), VSXREG(i));     // 128-bit move
        } else if (mmx) {
            XXLOR(VSXREG(j), VSXREG(i), VSXREG(i));     // MMX in VR space
        } else {
            FMR(j, i);          // 64-bit move (x87 ST in FPR space)
        }
        cache->vmxcache[j].v = cache->vmxcache[i].v;
    }
    switch (t) {
        case VMX_CACHE_YMMR:
        case VMX_CACHE_YMMW:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            // Load lower 128 from xmm[n]; upper 128 stays in ymm[n] memory
            LXV(VSXREG(i), offsetof(x64emu_t, xmm[n]), xEmu);
            break;
        case VMX_CACHE_XMMR:
        case VMX_CACHE_XMMW:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            LXV(VSXREG(i), offsetof(x64emu_t, xmm[n]), xEmu);
            break;
        case VMX_CACHE_MM:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            LD(s1, offsetof(x64emu_t, mmx[n]), xEmu);
            MTVSRDD(VSXREG(i), xZR, s1);
            break;
        case VMX_CACHE_ST_D:
        case VMX_CACHE_ST_F:
        case VMX_CACHE_ST_I64:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            if ((*s3_top) == 0xffff) {
                LWZ(s3, offsetof(x64emu_t, top), xEmu);
                *s3_top = 0;
            }
            int a = n - (*s3_top) - stack_cnt;
            if (a) {
                ADDI(s3, s3, a);
                ANDId(s3, s3, 7); // (emu->top + i)&7
            }
            *s3_top += a;
            *s2_val = 0;
            SLDI(s2, s3, 3);
            ADD(s2, xEmu, s2);
            LFD(i, offsetof(x64emu_t, x87), s2);
            if (t == VMX_CACHE_ST_F) {
                FRSP(i, i);
            }
            if (t == VMX_CACHE_ST_I64) {
                FCTIDZ(i, i);
            }
            break;
        case VMX_CACHE_NONE:
        case VMX_CACHE_SCR:
        default: /* nothing done */
            MESSAGE(LOG_DUMP, "\t  - ignoring %s\n", getCacheName(t, n));
            break;
    }
    cache->vmxcache[i].n = n;
    cache->vmxcache[i].t = t;
}

static void unloadCache(dynarec_ppc64le_t* dyn, int ninst, int stack_cnt, int s1, int s2, int s3, int* s1_val, int* s2_val, int* s3_top, vmxcache_t* cache, int i, int t, int n)
{
    switch (t) {
        case VMX_CACHE_XMMR:
        case VMX_CACHE_YMMR:
            MESSAGE(LOG_DUMP, "\t  - ignoring %s\n", getCacheName(t, n));
            break;
        case VMX_CACHE_XMMW:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            STXV(VSXREG(i), offsetof(x64emu_t, xmm[n]), xEmu);
            break;
        case VMX_CACHE_YMMW:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            // Store lower 128 to xmm[n]; upper 128 is already in ymm[n] memory
            STXV(VSXREG(i), offsetof(x64emu_t, xmm[n]), xEmu);
            break;
        case VMX_CACHE_MM:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            MFVSRLD(s1, VSXREG(i));
            STD(s1, offsetof(x64emu_t, mmx[n]), xEmu);
            break;
        case VMX_CACHE_ST_D:
        case VMX_CACHE_ST_F:
        case VMX_CACHE_ST_I64:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            if ((*s3_top) == 0xffff) {
                LWZ(s3, offsetof(x64emu_t, top), xEmu);
                *s3_top = 0;
            }
            int a = n - (*s3_top) - stack_cnt;
            if (a) {
                ADDI(s3, s3, a);
                ANDId(s3, s3, 7);
            }
            *s3_top += a;
            SLDI(s2, s3, 3);
            ADD(s2, xEmu, s2);
            *s2_val = 0;
            if (t == VMX_CACHE_ST_F) {
                // PPC64LE FPRs always hold doubles; ST_F is already double in register
                // No conversion needed — just store as double
            }
            if (t == VMX_CACHE_ST_I64) {
                FCFID(i, i);
            }
            STFD(i, offsetof(x64emu_t, x87), s2);
            break;
        case VMX_CACHE_NONE:
        case VMX_CACHE_SCR:
        default: /* nothing done */
            MESSAGE(LOG_DUMP, "\t  - ignoring %s\n", getCacheName(t, n));
            break;
    }
    cache->vmxcache[i].v = 0;
}

static void fpuCacheTransform(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3)
{
    int i2 = dyn->insts[ninst].x64.jmp_insts;
    if (i2 < 0)
        return;
    MESSAGE(LOG_DUMP, "\tCache Transform ---- ninst=%d -> %d\n", ninst, i2);
    if ((!i2) || (dyn->insts[i2].x64.barrier & BARRIER_FLOAT)) {
        if (dyn->v.stack_next) {
            fpu_purgecache(dyn, ninst, 1, s1, s2, s3);
            MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
            return;
        }
        for (int i = 0; i < 32; ++i)
            if (dyn->v.vmxcache[i].v) { // there is something at ninst for i
                fpu_purgecache(dyn, ninst, 1, s1, s2, s3);
                MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
                return;
            }
        MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
        return;
    }
    vmxcache_t cache_i2 = dyn->insts[i2].v;
    vmxcacheUnwind(&cache_i2);

    if (!cache_i2.stack) {
        int purge = 1;
        for (int i = 0; i < 32 && purge; ++i)
            if (cache_i2.vmxcache[i].v)
                purge = 0;
        if (purge) {
            fpu_purgecache(dyn, ninst, 1, s1, s2, s3);
            MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
            return;
        }
    }
    int stack_cnt = dyn->v.stack_next;
    int s3_top = 0xffff;
    vmxcache_t cache = dyn->v;
    int s1_val = 0;
    int s2_val = 0;
    // unload every unneeded cache
    // check SSE first, then MMX, in order, for optimization issue
    for (int i = 0; i < 16; ++i) {
        int j = findCacheSlot(dyn, ninst, VMX_CACHE_YMMW, i, &cache);
        if (j >= 0 && findCacheSlot(dyn, ninst, VMX_CACHE_YMMW, i, &cache_i2) == -1)
            unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, j, cache.vmxcache[j].t, cache.vmxcache[j].n);
    }
    for (int i = 0; i < 16; ++i) {
        int j = findCacheSlot(dyn, ninst, VMX_CACHE_XMMW, i, &cache);
        if (j >= 0 && findCacheSlot(dyn, ninst, VMX_CACHE_XMMW, i, &cache_i2) == -1)
            unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, j, cache.vmxcache[j].t, cache.vmxcache[j].n);
    }
    for (int i = 0; i < 8; ++i) {
        int j = findCacheSlot(dyn, ninst, VMX_CACHE_MM, i, &cache);
        if (j >= 0 && findCacheSlot(dyn, ninst, VMX_CACHE_MM, i, &cache_i2) == -1)
            unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, j, cache.vmxcache[j].t, cache.vmxcache[j].n);
    }
    for (int i = 0; i < 32; ++i) {
        if (cache.vmxcache[i].v)
            if (findCacheSlot(dyn, ninst, cache.vmxcache[i].t, cache.vmxcache[i].n, &cache_i2) == -1)
                unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, i, cache.vmxcache[i].t, cache.vmxcache[i].n);
    }
    // and now load/swap the missing one
    for (int i = 0; i < 32; ++i) {
        if (cache_i2.vmxcache[i].v) {
            if (cache_i2.vmxcache[i].v != cache.vmxcache[i].v) {
                int j;
                if ((j = findCacheSlot(dyn, ninst, cache_i2.vmxcache[i].t, cache_i2.vmxcache[i].n, &cache)) == -1)
                    loadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, i, cache_i2.vmxcache[i].t, cache_i2.vmxcache[i].n);
                else {
                    // it's here, lets swap if needed
                    if (j != i)
                        swapCache(dyn, ninst, i, j, &cache);
                }
            }
            if (cache.vmxcache[i].t != cache_i2.vmxcache[i].t) {
                if (cache.vmxcache[i].t == VMX_CACHE_ST_D && cache_i2.vmxcache[i].t == VMX_CACHE_ST_F) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.vmxcache[i].t, cache.vmxcache[i].n));
                    FRSP(i, i);
                    cache.vmxcache[i].t = VMX_CACHE_ST_F;
                } else if (cache.vmxcache[i].t == VMX_CACHE_ST_F && cache_i2.vmxcache[i].t == VMX_CACHE_ST_D) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.vmxcache[i].t, cache.vmxcache[i].n));
                    // PPC64LE: ST_F is already a double in FPR, no conversion needed
                    FMR(i, i);  // no-op move, just to be explicit
                    cache.vmxcache[i].t = VMX_CACHE_ST_D;
                } else if (cache.vmxcache[i].t == VMX_CACHE_ST_D && cache_i2.vmxcache[i].t == VMX_CACHE_ST_I64) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.vmxcache[i].t, cache.vmxcache[i].n));
                    FCTIDZ(i, i);
                    cache.vmxcache[i].t = VMX_CACHE_ST_I64;
                } else if (cache.vmxcache[i].t == VMX_CACHE_ST_F && cache_i2.vmxcache[i].t == VMX_CACHE_ST_I64) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.vmxcache[i].t, cache.vmxcache[i].n));
                    // ST_F is double in FPR on PPC64LE, so FCTIDZ works directly
                    FCTIDZ(i, i);
                    cache.vmxcache[i].t = VMX_CACHE_ST_I64;
                } else if (cache.vmxcache[i].t == VMX_CACHE_ST_I64 && cache_i2.vmxcache[i].t == VMX_CACHE_ST_F) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.vmxcache[i].t, cache.vmxcache[i].n));
                    FCFID(i, i);     // int64 -> double
                    FRSP(i, i);      // double -> single (round)
                    cache.vmxcache[i].t = VMX_CACHE_ST_F;
                } else if (cache.vmxcache[i].t == VMX_CACHE_ST_I64 && cache_i2.vmxcache[i].t == VMX_CACHE_ST_D) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.vmxcache[i].t, cache.vmxcache[i].n));
                    FCFID(i, i);
                    cache.vmxcache[i].t = VMX_CACHE_ST_D;
                } else if (cache.vmxcache[i].t == VMX_CACHE_XMMR && cache_i2.vmxcache[i].t == VMX_CACHE_XMMW) {
                    cache.vmxcache[i].t = VMX_CACHE_XMMW;
                } else if (cache.vmxcache[i].t == VMX_CACHE_YMMR && cache_i2.vmxcache[i].t == VMX_CACHE_YMMW) {
                    cache.vmxcache[i].t = VMX_CACHE_YMMW;
                } else if (cache.vmxcache[i].t == VMX_CACHE_YMMW && cache_i2.vmxcache[i].t == VMX_CACHE_YMMR) {
                    // refresh cache...
                    MESSAGE(LOG_DUMP, "\t  - Refreh %s\n", getCacheName(cache.vmxcache[i].t, cache.vmxcache[i].n));
                    // Store lower 128 to xmm[n]; upper 128 is already in ymm[n] memory
                    STXV(VSXREG(i), offsetof(x64emu_t, xmm[cache.vmxcache[i].n]), xEmu);
                    cache.vmxcache[i].t = VMX_CACHE_YMMR;
                } else if (cache.vmxcache[i].t == VMX_CACHE_XMMW && cache_i2.vmxcache[i].t == VMX_CACHE_XMMR) {
                    // refresh cache...
                    MESSAGE(LOG_DUMP, "\t  - Refreh %s\n", getCacheName(cache.vmxcache[i].t, cache.vmxcache[i].n));
                    STXV(VSXREG(i), offsetof(x64emu_t, xmm[cache.vmxcache[i].n]), xEmu);
                    cache.vmxcache[i].t = VMX_CACHE_XMMR;
                }
            }
        }
    }
    if (stack_cnt != cache_i2.stack) {
        MESSAGE(LOG_DUMP, "\t    - adjust stack count %d -> %d -\n", stack_cnt, cache_i2.stack);
        int a = stack_cnt - cache_i2.stack;
        // Add x87stack to emu fpu_stack
        LWZ(s3, offsetof(x64emu_t, fpu_stack), xEmu);
        ADDI(s3, s3, a);
        STW(s3, offsetof(x64emu_t, fpu_stack), xEmu);
        // Sub x87stack to top, with and 7
        LWZ(s3, offsetof(x64emu_t, top), xEmu);
        ADDI(s3, s3, -a);
        ANDId(s3, s3, 7);
        STW(s3, offsetof(x64emu_t, top), xEmu);
        // update tags
        LHA(s2, offsetof(x64emu_t, fpu_tags), xEmu);
        if (a > 0) {
            SLDI(s2, s2, a * 2);
        } else {
            MOV32w(s3, 0xffff0000);
            OR(s2, s2, s3);
            SRDI(s2, s2, -a * 2);
        }
        STH(s2, offsetof(x64emu_t, fpu_tags), xEmu);
        s3_top = 0;
        stack_cnt = cache_i2.stack;
    }
    MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
}

static void flagsCacheTransform(dynarec_ppc64le_t* dyn, int ninst, int s1)
{
    int j64;
    int jmp = dyn->insts[ninst].x64.jmp_insts;
    if (jmp < 0)
        return;
    if (dyn->insts[jmp].f_exit == dyn->insts[jmp].f_entry) // flags will be fully known, nothing we can do more
        return;
    MESSAGE(LOG_DUMP, "\tFlags fetch ---- ninst=%d -> %d\n", ninst, jmp);
    int go_fetch = 0;
    switch (dyn->insts[jmp].f_entry) {
        case status_unk:
            if (dyn->insts[ninst].f_exit == status_none_pending) {
                FORCE_DFNONE();
            }
            break;
        case status_set:
            if (dyn->insts[ninst].f_exit == status_none_pending) {
                FORCE_DFNONE();
            }
            if (dyn->insts[ninst].f_exit == status_unk)
                go_fetch = 1;
            break;
        case status_none_pending:
            if (dyn->insts[ninst].f_exit != status_none)
                go_fetch = 1;
            break;
        case status_none:
            if (dyn->insts[ninst].f_exit == status_none_pending) {
                FORCE_DFNONE();
            } else
                go_fetch = 1;
            break;
    }
    if (go_fetch) {
        if (dyn->f == status_unk) {
            LWZ(s1, offsetof(x64emu_t, df), xEmu);
            CMPDI(s1, 0);
            j64 = (GETMARKF2) - (dyn->native_size);
            BEQ(j64);
        }
        TABLE64C(x6, const_updateflags_ppc64le);
        MTCTR(x6);
        BCTRL();
        MARKF2;
    }
    MESSAGE(LOG_DUMP, "\t---- Flags fetch\n");
}

void CacheTransform(dynarec_ppc64le_t* dyn, int ninst, int cacheupd, int s1, int s2, int s3)
{
    if (cacheupd & 2)
        fpuCacheTransform(dyn, ninst, s1, s2, s3);
    if (cacheupd & 1)
        flagsCacheTransform(dyn, ninst, s1);
}
