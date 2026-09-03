#ifndef __LA64_PCMPSTR_H_
#define __LA64_PCMPSTR_H_

static inline void pcmpstr_prefix_mask(dynarec_la64_t* dyn, int ninst, int dst, int length)
{
    ADDI_D(dst, xZR, 1);
    SLL_W(dst, dst, length);
    ADDI_W(dst, dst, -1);
}

static inline void pcmpstr_normalize_length(dynarec_la64_t* dyn, int ninst, int dst, int src, int count, int tmp)
{
    SEXT_W(dst, src);
    SRAI_D(tmp, dst, 63);
    XOR(dst, dst, tmp);
    SUB_D(dst, dst, tmp);
    SLTUI(tmp, dst, count + 1);
    SUB_D(tmp, xZR, tmp);
    XORI(dst, dst, count);
    AND(dst, dst, tmp);
    XORI(dst, dst, count);
}

static inline void pcmpstr_extract_mask(dynarec_la64_t* dyn, int ninst, int result, int vector, int word)
{
    if (word)
        VMSKLTZ_H(vector, vector);
    else
        VMSKLTZ_B(vector, vector);
    VPICKVE2GR_HU(result, vector, 0);
}

static inline void pcmpstr_compare_le(dynarec_la64_t* dyn, int ninst, int dst, int lhs, int rhs, int word, int sign)
{
    if (word) {
        if (sign)
            VSLE_H(dst, lhs, rhs);
        else
            VSLE_HU(dst, lhs, rhs);
    } else {
        if (sign)
            VSLE_B(dst, lhs, rhs);
        else
            VSLE_BU(dst, lhs, rhs);
    }
}

static inline void pcmpstr_replicate_imm(dynarec_la64_t* dyn, int ninst, int dst, int src, int index, int word)
{
    if (word)
        VREPLVEI_H(dst, src, index);
    else
        VREPLVEI_B(dst, src, index);
}

static inline void pcmpstr_compare_equal(dynarec_la64_t* dyn, int ninst, int dst, int lhs, int rhs, int word)
{
    if (word)
        VSEQ_H(dst, lhs, rhs);
    else
        VSEQ_B(dst, lhs, rhs);
}

static inline void pcmpstr_compare_le_lasx(dynarec_la64_t* dyn, int ninst, int dst, int lhs, int rhs, int word, int sign)
{
    if (word) {
        if (sign)
            XVSLE_H(dst, lhs, rhs);
        else
            XVSLE_HU(dst, lhs, rhs);
    } else {
        if (sign)
            XVSLE_B(dst, lhs, rhs);
        else
            XVSLE_BU(dst, lhs, rhs);
    }
}

static inline void pcmpstr_replicate_128_imm(dynarec_la64_t* dyn, int ninst, int dst, int src, int index, int word)
{
    if (word)
        XVREPL128VEI_H(dst, src, index);
    else
        XVREPL128VEI_B(dst, src, index);
}

static inline void pcmpstr_compare_equal_lasx(dynarec_la64_t* dyn, int ninst, int dst, int lhs, int rhs, int word)
{
    if (word)
        XVSEQ_H(dst, lhs, rhs);
    else
        XVSEQ_B(dst, lhs, rhs);
}

static inline void pcmpstr_replicate_reg(dynarec_la64_t* dyn, int ninst, int dst, int src, int index, int word)
{
    if (word)
        VREPLVE_H(dst, src, index);
    else
        VREPLVE_B(dst, src, index);
}

static inline void pcmpstr_ordered_mask(dynarec_la64_t* dyn, int ninst, int mask, int tmp,
                                        int lmem, int lreg, int count)
{
    SUB_W(mask, lmem, lreg);
    ADDI_W(mask, mask, 1);
    ADDI_D(tmp, xZR, 1);
    SLL_W(mask, tmp, mask);
    ADDI_W(mask, mask, -1);

    SLTU(tmp, lmem, lreg);
    SUB_D(tmp, xZR, tmp);
    ANDN(mask, mask, tmp);

    SLTUI(tmp, lmem, count);
    ADDI_W(tmp, tmp, -1);
    OR(mask, mask, tmp);
    SLTUI(tmp, lreg, 1);
    SUB_D(tmp, xZR, tmp);
    OR(mask, mask, tmp);
    BSTRPICK_D(mask, mask, count - 1, 0);
}

static inline void pcmpstr_lsx_core(dynarec_la64_t* dyn, int ninst, int result,
                                    int vmem, int vreg, int lmem, int lreg, uint8_t imm8)
{
    const int word = imm8 & 1;
    const int sign = (imm8 >> 1) & 1;
    const int mode = (imm8 >> 2) & 3;
    const int count = word ? 8 : 16;
    const int shift = word ? 2 : 1;
    int64_t j64;

    switch (mode) {
        case 0: {
            int acc = fpu_get_scratch(dyn);
            int tmp = fpu_get_scratch(dyn);
            VXOR_V(acc, acc, acc);
            MV(x4, lreg);
            BEQZ_MARK(x4);
            for (int i = 0; i < count; ++i) {
                pcmpstr_replicate_imm(dyn, ninst, tmp, vreg, i, word);
                pcmpstr_compare_equal(dyn, ninst, tmp, vmem, tmp, word);
                VOR_V(acc, acc, tmp);
                if (i + 1 != count) {
                    ADDI_D(x4, x4, -1);
                    BEQZ_MARK(x4);
                }
            }
            MARK;
            pcmpstr_extract_mask(dyn, ninst, result, acc, word);
            pcmpstr_prefix_mask(dyn, ninst, x4, lmem);
            AND(result, result, x4);
            break;
        }
        case 1: {
            int acc = fpu_get_scratch(dyn);
            int lo = fpu_get_scratch(dyn);
            int hi = fpu_get_scratch(dyn);
            VXOR_V(acc, acc, acc);
            SRLI_W(x4, lreg, 1);
            BEQZ_MARK(x4);
            for (int i = 0; i < count; i += 2) {
                pcmpstr_replicate_imm(dyn, ninst, lo, vreg, i, word);
                pcmpstr_replicate_imm(dyn, ninst, hi, vreg, i + 1, word);
                pcmpstr_compare_le(dyn, ninst, lo, lo, vmem, word, sign);
                pcmpstr_compare_le(dyn, ninst, hi, vmem, hi, word, sign);
                VAND_V(lo, lo, hi);
                VOR_V(acc, acc, lo);
                if (i + 2 != count) {
                    ADDI_D(x4, x4, -1);
                    BEQZ_MARK(x4);
                }
            }
            MARK;
            pcmpstr_extract_mask(dyn, ninst, result, acc, word);
            pcmpstr_prefix_mask(dyn, ninst, x4, lmem);
            AND(result, result, x4);
            break;
        }
        case 2: {
            int tmp = fpu_get_scratch(dyn);
            pcmpstr_compare_equal(dyn, ninst, tmp, vmem, vreg, word);
            pcmpstr_extract_mask(dyn, ninst, result, tmp, word);
            pcmpstr_prefix_mask(dyn, ninst, x4, lmem);
            pcmpstr_prefix_mask(dyn, ninst, x5, lreg);
            AND(result, result, x4);
            AND(result, result, x5);
            OR(x4, x4, x5);
            NOR(x4, x4, xZR);
            BSTRPICK_D(x4, x4, count - 1, 0);
            OR(result, result, x4);
            break;
        }
        default: {
            int mem = fpu_get_scratch(dyn);
            int active = fpu_get_scratch(dyn);
            int acc = fpu_get_scratch(dyn);
            int tmp = fpu_get_scratch(dyn);
            VOR_V(mem, vmem, vmem);
            pcmpstr_compare_equal(dyn, ninst, active, mem, mem, word);
            VOR_V(acc, active, active);
            MV(x4, lreg);
            BEQZ_MARK(x4);
            for (int i = 0; i < count; ++i) {
                pcmpstr_replicate_imm(dyn, ninst, tmp, vreg, i, word);
                pcmpstr_compare_equal(dyn, ninst, tmp, mem, tmp, word);
                VORN_V(tmp, tmp, active);
                VAND_V(acc, acc, tmp);
                VBSRL_V(mem, mem, shift);
                VBSRL_V(active, active, shift);
                if (i + 1 != count) {
                    ADDI_D(x4, x4, -1);
                    BEQZ_MARK(x4);
                }
            }
            MARK;
            pcmpstr_extract_mask(dyn, ninst, result, acc, word);
            pcmpstr_ordered_mask(dyn, ninst, x4, x5, lmem, lreg, count);
            AND(result, result, x4);
            break;
        }
    }
    BSTRPICK_D(result, result, count - 1, 0);
}

static inline void pcmpstr_lasx_core(dynarec_la64_t* dyn, int ninst, int result,
                                     int vmem, int vreg, int lmem, int lreg, uint8_t imm8)
{
    const int word = imm8 & 1;
    const int sign = (imm8 >> 1) & 1;
    const int mode = (imm8 >> 2) & 3;
    const int count = word ? 8 : 16;
    const int one_shift = word ? 2 : 1;
    const int pair_shift = one_shift * 2;
    int64_t j64;

    if (mode == 0) {
        int mem = fpu_get_scratch(dyn);
        int reg = fpu_get_scratch(dyn);
        int acc = fpu_get_scratch(dyn);
        int tmp = fpu_get_scratch(dyn);

        XVPERMI_Q(mem, vmem, 0x00);
        XVBSRL_V(reg, vreg, one_shift);
        XVPERMI_Q(reg, vreg, 0x20);
        XVXOR_V(acc, acc, acc);

        SRLI_W(x4, lreg, 1);
        BEQZ_MARK(x4);
        for (int i = 0; i < count; i += 2) {
            pcmpstr_replicate_128_imm(dyn, ninst, tmp, reg, i, word);
            pcmpstr_compare_equal_lasx(dyn, ninst, tmp, mem, tmp, word);
            XVOR_V(acc, acc, tmp);
            if (i + 2 != count) {
                ADDI_D(x4, x4, -1);
                BEQZ_MARK(x4);
            }
        }
        MARK;
        XVPERMI_Q(tmp, acc, 0x01);
        VOR_V(acc, acc, tmp);

        ANDI(x5, lreg, 1);
        BEQZ_MARK2(x5);
        ADDI_W(x4, lreg, -1);
        pcmpstr_replicate_reg(dyn, ninst, tmp, reg, x4, word);
        pcmpstr_compare_equal(dyn, ninst, tmp, vmem, tmp, word);
        VOR_V(acc, acc, tmp);
        MARK2;

        pcmpstr_extract_mask(dyn, ninst, result, acc, word);
        pcmpstr_prefix_mask(dyn, ninst, x4, lmem);
        AND(result, result, x4);
    } else if (mode == 1) {
        int mem = fpu_get_scratch(dyn);
        int reg = fpu_get_scratch(dyn);
        int acc = fpu_get_scratch(dyn);
        int lo = fpu_get_scratch(dyn);
        int hi = fpu_get_scratch(dyn);

        XVPERMI_Q(mem, vmem, 0x00);
        XVBSRL_V(reg, vreg, pair_shift);
        XVPERMI_Q(reg, vreg, 0x20);
        XVXOR_V(acc, acc, acc);

        SRLI_W(x4, lreg, 2);
        BEQZ_MARK(x4);
        for (int i = 0; i < count; i += 4) {
            pcmpstr_replicate_128_imm(dyn, ninst, lo, reg, i, word);
            pcmpstr_replicate_128_imm(dyn, ninst, hi, reg, i + 1, word);
            pcmpstr_compare_le_lasx(dyn, ninst, lo, lo, mem, word, sign);
            pcmpstr_compare_le_lasx(dyn, ninst, hi, mem, hi, word, sign);
            XVAND_V(lo, lo, hi);
            XVOR_V(acc, acc, lo);
            if (i + 4 != count) {
                ADDI_D(x4, x4, -1);
                BEQZ_MARK(x4);
            }
        }
        MARK;
        XVPERMI_Q(lo, acc, 0x01);
        VOR_V(acc, acc, lo);

        SRLI_W(x5, lreg, 1);
        ANDI(x5, x5, 1);
        BEQZ_MARK2(x5);
        SRLI_W(x4, lreg, 1);
        ADDI_W(x4, x4, -1);
        SLLI_W(x4, x4, 1);
        pcmpstr_replicate_reg(dyn, ninst, lo, reg, x4, word);
        ADDI_W(x4, x4, 1);
        pcmpstr_replicate_reg(dyn, ninst, hi, reg, x4, word);
        pcmpstr_compare_le(dyn, ninst, lo, lo, vmem, word, sign);
        pcmpstr_compare_le(dyn, ninst, hi, vmem, hi, word, sign);
        VAND_V(lo, lo, hi);
        VOR_V(acc, acc, lo);
        MARK2;

        pcmpstr_extract_mask(dyn, ninst, result, acc, word);
        pcmpstr_prefix_mask(dyn, ninst, x4, lmem);
        AND(result, result, x4);
    } else {
        int mem = fpu_get_scratch(dyn);
        int reg = fpu_get_scratch(dyn);
        int active = fpu_get_scratch(dyn);
        int acc = fpu_get_scratch(dyn);
        int tmp = fpu_get_scratch(dyn);

        XVBSRL_V(mem, vmem, one_shift);
        XVBSRL_V(reg, vreg, one_shift);
        pcmpstr_compare_equal_lasx(dyn, ninst, acc, vmem, vmem, word);
        XVBSRL_V(active, acc, one_shift);
        XVPERMI_Q(mem, vmem, 0x20);
        XVPERMI_Q(reg, vreg, 0x20);
        XVPERMI_Q(active, acc, 0x20);

        SRLI_W(x4, lreg, 1);
        BEQZ_MARK(x4);
        for (int i = 0; i < count; i += 2) {
            pcmpstr_replicate_128_imm(dyn, ninst, tmp, reg, i, word);
            pcmpstr_compare_equal_lasx(dyn, ninst, tmp, mem, tmp, word);
            XVORN_V(tmp, tmp, active);
            XVBSRL_V(mem, mem, pair_shift);
            XVBSRL_V(active, active, pair_shift);
            XVAND_V(acc, acc, tmp);
            if (i + 2 != count) {
                ADDI_D(x4, x4, -1);
                BEQZ_MARK(x4);
            }
        }
        MARK;
        XVPERMI_Q(tmp, acc, 0x01);
        VAND_V(acc, acc, tmp);

        ANDI(x5, lreg, 1);
        BEQZ_MARK2(x5);
        ADDI_W(x4, lreg, -1);
        pcmpstr_replicate_reg(dyn, ninst, tmp, reg, x4, word);
        pcmpstr_compare_equal(dyn, ninst, tmp, mem, tmp, word);
        VORN_V(tmp, tmp, active);
        VAND_V(acc, acc, tmp);
        MARK2;

        pcmpstr_extract_mask(dyn, ninst, result, acc, word);
        pcmpstr_ordered_mask(dyn, ninst, x4, x5, lmem, lreg, count);
        AND(result, result, x4);
    }
    BSTRPICK_D(result, result, count - 1, 0);
}

static inline void pcmpstr_set_flags(dynarec_la64_t* dyn, int ninst, int result,
                                     int lmem, int lreg, int count)
{
    IFX (X_ALL) {
        const uint8_t flags = dyn->insts[ninst].x64.gen_flags & X_ALL;
        if (cpuext.lbt) {
            int initialized = 0;
            IFX (X_CF) {
                SNEZ(x4, result);
                initialized = 1;
            }
            IFX (X_ZF) {
                int dst = initialized ? x5 : x4;
                SLTUI(dst, lmem, count);
                BSTRINS_D(x4, dst, F_ZF, F_ZF);
                initialized = 1;
            }
            IFX (X_SF) {
                int dst = initialized ? x5 : x4;
                SLTUI(dst, lreg, count);
                BSTRINS_D(x4, dst, F_SF, F_SF);
                initialized = 1;
            }
            IFX (X_OF) {
                int dst = initialized ? x5 : x4;
                ANDI(dst, result, 1);
                BSTRINS_D(x4, dst, F_OF, F_OF);
                initialized = 1;
            }
            X64_SET_EFLAGS(initialized ? x4 : xZR, flags);
        } else {
            IFX (X_CF) {
                SNEZ(x4, result);
                BSTRINS_D(xFlags, x4, F_CF, F_CF);
            }
            IFX (X_ZF) {
                SLTUI(x4, lmem, count);
                BSTRINS_D(xFlags, x4, F_ZF, F_ZF);
            }
            IFX (X_SF) {
                SLTUI(x4, lreg, count);
                BSTRINS_D(xFlags, x4, F_SF, F_SF);
            }
            IFX (X_OF) {
                ANDI(x4, result, 1);
                BSTRINS_D(xFlags, x4, F_OF, F_OF);
            }
            IFX (X_PF | X_AF) {
                if ((flags & (X_PF | X_AF)) == (X_PF | X_AF))
                    BSTRINS_D(xFlags, xZR, F_AF, F_PF);
                else IFX (X_PF)
                    BSTRINS_D(xFlags, xZR, F_PF, F_PF);
                else
                    BSTRINS_D(xFlags, xZR, F_AF, F_AF);
            }
        }
    }
}

static inline void pcmpstr_write_mask(dynarec_la64_t* dyn, int ninst, int dst, int result, int word, int unit_mask)
{
    if (!unit_mask) {
        VXOR_V(dst, dst, dst);
        VINSGR2VR_H(dst, result, 0);
        return;
    }

    int tmp = SCRATCH;
    if (!word) {
        VREPLGR2VR_B(dst, result);
        SRLI_D(x4, result, 8);
        VREPLGR2VR_B(tmp, x4);
        VEXTRINS_D(dst, tmp, 0x10);
        VMEPATMSK_V(tmp, 3, 0);
        VXORI_B(tmp, tmp, 7);
        VSLL_B(dst, dst, tmp);
        VSLTI_B(dst, dst, 0);
    } else {
        VREPLGR2VR_H(dst, result);
        MOV64x(x4, 0x000C000D000E000F);
        VREPLGR2VR_D(tmp, x4);
        MOV64x(x4, 0x00080009000A000B);
        VINSGR2VR_D(tmp, x4, 1);
        VSLL_H(dst, dst, tmp);
        VSLTI_H(dst, dst, 0);
    }
}

static inline void emit_pcmpstr(dynarec_la64_t* dyn, int ninst, int vmem, int vreg, int vdst,
                                uint8_t imm8, int explicit_len, int index)
{
    const int word = imm8 & 1;
    const int count = word ? 8 : 16;
    const int lmem = x2;
    const int lreg = x3;
    const int result = x1;

    imm8 &= 0x7f;
    if (explicit_len) {
        pcmpstr_normalize_length(dyn, ninst, lmem, xRDX, count, x4);
        pcmpstr_normalize_length(dyn, ninst, lreg, xRAX, count, x4);
    } else {
        int tmp = SCRATCH;
        pcmpstr_compare_equal(dyn, ninst, tmp, vmem, VZERO, word);
        if (word)
            VFRSTPI_H(tmp, tmp, 0);
        else
            VFRSTPI_B(tmp, tmp, 0);
        if (word)
            VPICKVE2GR_HU(lmem, tmp, 0);
        else
            VPICKVE2GR_BU(lmem, tmp, 0);

        pcmpstr_compare_equal(dyn, ninst, tmp, vreg, VZERO, word);
        if (word)
            VFRSTPI_H(tmp, tmp, 0);
        else
            VFRSTPI_B(tmp, tmp, 0);
        if (word)
            VPICKVE2GR_HU(lreg, tmp, 0);
        else
            VPICKVE2GR_BU(lreg, tmp, 0);
    }

    if (cpuext.lasx && ((imm8 >> 2) & 3) != 2)
        pcmpstr_lasx_core(dyn, ninst, result, vmem, vreg, lmem, lreg, imm8);
    else
        pcmpstr_lsx_core(dyn, ninst, result, vmem, vreg, lmem, lreg, imm8);

    switch ((imm8 >> 4) & 3) {
        case 1:
            NOR(result, result, xZR);
            BSTRPICK_D(result, result, count - 1, 0);
            break;
        case 3:
            pcmpstr_prefix_mask(dyn, ninst, x4, lmem);
            XOR(result, result, x4);
            break;
        default:
            break;
    }

    pcmpstr_set_flags(dyn, ninst, result, lmem, lreg, count);
    if (index) {
        if (imm8 & 0x40) {
            CLZ_W(xRCX, result);
            ADDI_D(x4, xZR, 31);
            SUB_D(xRCX, x4, xRCX);
        } else {
            CTZ_W(xRCX, result);
        }
        BNEZ(result, 8);
        ADDI_D(xRCX, xZR, count);
    } else {
        pcmpstr_write_mask(dyn, ninst, vdst, result, word, imm8 & 0x40);
    }
}

#endif // __LA64_PCMPSTR_H_
