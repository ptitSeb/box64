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
// x87 / MMX / SSE / AVX Cache Management — STUBS (TODO)
// These will be fully implemented in a later PR when FPU opcodes are added.
// Following the RV64 precedent (commit 5a9b89600) where all FPU cache
// functions were empty stubs in the initial dynarec commit.
// ========================================================================

int x87_stackcount(dynarec_ppc64le_t* dyn, int ninst, int scratch)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(scratch);
    return 0;
}

void x87_unstackcount(dynarec_ppc64le_t* dyn, int ninst, int scratch, int count)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(scratch); MAYUSE(count);
}

void x87_forget(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int st)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(s2); MAYUSE(st);
}

void x87_reget_st(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int st)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(s2); MAYUSE(st);
}

void x87_free(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int st)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(s2); MAYUSE(s3); MAYUSE(st);
}

void x87_swapreg(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int a, int b)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(s2); MAYUSE(a); MAYUSE(b);
}

int x87_setround(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(s2);
    return s2;
}

int vmxcache_st_coherency(dynarec_ppc64le_t* dyn, int ninst, int a, int b)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(a); MAYUSE(b);
    return -1;
}

void x87_restoreround(dynarec_ppc64le_t* dyn, int ninst, int s1)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1);
}

int x87_do_push(dynarec_ppc64le_t* dyn, int ninst, int s1, int t)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(t);
    return -1;
}

void x87_do_push_empty(dynarec_ppc64le_t* dyn, int ninst, int s1)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1);
}

void x87_do_pop(dynarec_ppc64le_t* dyn, int ninst, int s1)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1);
}

void x87_purgecache(dynarec_ppc64le_t* dyn, int ninst, int next, int s1, int s2, int s3)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(next); MAYUSE(s1); MAYUSE(s2); MAYUSE(s3);
}

void x87_reflectcount(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(s2);
}

void x87_unreflectcount(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(s2);
}

int x87_get_current_cache(dynarec_ppc64le_t* dyn, int ninst, int st, int t)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(st); MAYUSE(t);
    return -1;
}

int x87_get_cache(dynarec_ppc64le_t* dyn, int ninst, int populate, int s1, int s2, int st, int t)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(populate); MAYUSE(s1); MAYUSE(s2); MAYUSE(st); MAYUSE(t);
    return -1;
}

int x87_get_vmxcache(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int st)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(s2); MAYUSE(st);
    return -1;
}

int x87_get_st(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int a, int t)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(s2); MAYUSE(a); MAYUSE(t);
    return -1;
}

int x87_get_st_empty(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int a, int t)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(s2); MAYUSE(a); MAYUSE(t);
    return -1;
}

// MMX helpers
int mmx_get_reg(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int a)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(s2); MAYUSE(s3); MAYUSE(a);
    return -1;
}

int mmx_get_reg_empty(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int a)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(s2); MAYUSE(s3); MAYUSE(a);
    return -1;
}

void mmx_purgecache(dynarec_ppc64le_t* dyn, int ninst, int next, int s1)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(next); MAYUSE(s1);
}

// SSE / SSE2 helpers
int sse_get_reg(dynarec_ppc64le_t* dyn, int ninst, int s1, int a, int forwrite)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(a); MAYUSE(forwrite);
    return -1;
}

int sse_get_reg_empty(dynarec_ppc64le_t* dyn, int ninst, int s1, int a)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(a);
    return -1;
}

void sse_forget_reg(dynarec_ppc64le_t* dyn, int ninst, int a)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(a);
}

void sse_reflect_reg(dynarec_ppc64le_t* dyn, int ninst, int a)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(a);
}

void sse_purge07cache(dynarec_ppc64le_t* dyn, int ninst, int s1)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1);
}

int sse_setround(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(s2);
    return s2;
}

// AVX helpers
int avx_get_reg(dynarec_ppc64le_t* dyn, int ninst, int s1, int a, int forwrite, int width)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(a); MAYUSE(forwrite); MAYUSE(width);
    return -1;
}

int avx_get_reg_empty(dynarec_ppc64le_t* dyn, int ninst, int s1, int a, int width)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(a); MAYUSE(width);
    return -1;
}

void avx_reflect_reg_upper128(dynarec_ppc64le_t* dyn, int ninst, int a, int forwrite)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(a); MAYUSE(forwrite);
}

void avx_forget_reg(dynarec_ppc64le_t* dyn, int ninst, int a)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(a);
}

void avx_reflect_reg(dynarec_ppc64le_t* dyn, int ninst, int a)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(a);
}

void avx_purgecache(dynarec_ppc64le_t* dyn, int ninst, int next, int s1)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(next); MAYUSE(s1);
}

// FPU cache push/pop/purge/reflect
void fpu_pushcache(dynarec_ppc64le_t* dyn, int ninst, int s1, int not07)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(not07);
}

void fpu_popcache(dynarec_ppc64le_t* dyn, int ninst, int s1, int not07)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(not07);
}

void fpu_purgecache(dynarec_ppc64le_t* dyn, int ninst, int next, int s1, int s2, int s3)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(next); MAYUSE(s1); MAYUSE(s2); MAYUSE(s3);
    if (!next) {
        fpu_reset_reg(dyn);
        dyn->insts[ninst].fpupurge = 1;
    }
}

void fpu_reflectcache(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(s2); MAYUSE(s3);
}

void fpu_unreflectcache(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3)
{
    // TODO
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(s2); MAYUSE(s3);
}

// ========================================================================
// Emit helpers
// ========================================================================

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

// ========================================================================
// Cache Reset & Propagation
// ========================================================================

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

// ========================================================================
// Cache Transform
// ========================================================================

static void fpuCacheTransform(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3)
{
    // TODO — full implementation deferred to later PR
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1); MAYUSE(s2); MAYUSE(s3);
}

static void flagsCacheTransform(dynarec_ppc64le_t* dyn, int ninst, int s1)
{
    // TODO — full implementation deferred to later PR
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(s1);
}

void CacheTransform(dynarec_ppc64le_t* dyn, int ninst, int cacheupd, int s1, int s2, int s3)
{
    if (cacheupd & 2)
        fpuCacheTransform(dyn, ninst, s1, s2, s3);
    if (cacheupd & 1)
        flagsCacheTransform(dyn, ninst, s1);
}
