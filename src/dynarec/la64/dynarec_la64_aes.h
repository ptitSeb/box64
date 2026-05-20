#ifndef __DYNAREC_LA64_AES_H_
#define __DYNAREC_LA64_AES_H_

#include <stdint.h>
#include "dynarec_la64_consts.h"

static inline void la64_subbytes_lsx(dynarec_la64_t* dyn, int ninst, int dst, int zero, int t0, int t1, int t2, int t3, int tab)
{
    (void)dyn;
    (void)ninst;
    VXOR_V(zero, zero, zero);
    VANDI_B(t0, dst, 0x0f);
    VSRLI_B(t1, dst, 4);
    VLD(tab, x5, 1 * 16);
    VSHUF_B(t2, zero, tab, t1);
    VLD(tab, x5, 0 * 16);
    VSHUF_B(dst, zero, tab, t0);
    VXOR_V(dst, dst, t2);
    VANDI_B(t0, dst, 0x0f);
    VLD(tab, x5, 3 * 16);
    VSHUF_B(t2, zero, tab, t0);
    VSRLI_B(t1, dst, 4);
    VLD(tab, x5, 2 * 16);
    VSHUF_B(dst, zero, tab, t1);
    VXOR_V(t3, t1, t0);
    VXOR_V(dst, dst, t2);
    VSHUF_B(t0, zero, tab, t3);
    VSHUF_B(dst, zero, tab, dst);
    VXOR_V(t2, t0, t2);
    VXOR_V(t0, dst, t3);
    VSHUF_B(t2, zero, tab, t2);
    VLD(tab, x5, 4 * 16);
    VSHUF_B(t0, zero, tab, t0);
    VXOR_V(t2, t2, t1);
    VLD(tab, x5, 5 * 16);
    VSHUF_B(t2, zero, tab, t2);
    VXOR_V(dst, t0, t2);
    VXORI_B(dst, dst, 99);
}

static inline void la64_subbytes_lasx(dynarec_la64_t* dyn, int ninst, int dst, int zero, int t0, int t1, int t2, int t3, int tab)
{
    (void)dyn;
    (void)ninst;
    XVXOR_V(zero, zero, zero);
    XVANDI_B(t0, dst, 0x0f);
    XVSRLI_B(t1, dst, 4);
    XVLD(tab, x5, 1 * 32);
    XVSHUF_B(t2, zero, tab, t1);
    XVLD(tab, x5, 0 * 32);
    XVSHUF_B(dst, zero, tab, t0);
    XVXOR_V(dst, dst, t2);
    XVANDI_B(t0, dst, 0x0f);
    XVLD(tab, x5, 3 * 32);
    XVSHUF_B(t2, zero, tab, t0);
    XVSRLI_B(t1, dst, 4);
    XVLD(tab, x5, 2 * 32);
    XVSHUF_B(dst, zero, tab, t1);
    XVXOR_V(t3, t1, t0);
    XVXOR_V(dst, dst, t2);
    XVSHUF_B(t0, zero, tab, t3);
    XVSHUF_B(dst, zero, tab, dst);
    XVXOR_V(t2, t0, t2);
    XVXOR_V(t0, dst, t3);
    XVSHUF_B(t2, zero, tab, t2);
    XVLD(tab, x5, 4 * 32);
    XVSHUF_B(t0, zero, tab, t0);
    XVXOR_V(t2, t2, t1);
    XVLD(tab, x5, 5 * 32);
    XVSHUF_B(t2, zero, tab, t2);
    XVXOR_V(dst, t0, t2);
    XVXORI_B(dst, dst, 99);
}

static inline void la64_xtime_table_lsx(dynarec_la64_t* dyn, int ninst, int dst, int src, int tmp, int tab_lo, int tab_hi)
{
    (void)dyn;
    (void)ninst;
    VANDI_B(tmp, src, 0x0f);
    VSRLI_B(dst, src, 4);
    VSHUF_B(tmp, tab_lo, tab_lo, tmp);
    VSHUF_B(dst, tab_hi, tab_hi, dst);
    VXOR_V(dst, dst, tmp);
}

static inline void la64_mixcolumns_xtime_lsx(dynarec_la64_t* dyn, int ninst, int dst, int tab_lo, int tab_hi, int t0, int t1, int t2, int t3)
{
    la64_xtime_table_lsx(dyn, ninst, t0, dst, t3, tab_lo, tab_hi);
    VSHUF4I_B(t1, dst, 0x39);
    VSHUF4I_B(t2, dst, 0x4e);
    la64_xtime_table_lsx(dyn, ninst, dst, t1, t3, tab_lo, tab_hi);
    VSHUF4I_B(t3, t2, 0x39);
    VXOR_V(t2, t2, t3);
    VXOR_V(dst, dst, t1);
    VXOR_V(dst, dst, t0);
    VXOR_V(dst, dst, t2);
}

static inline void la64_invmixcolumns_xtime_lsx(dynarec_la64_t* dyn, int ninst, int dst, int tab_lo, int tab_hi, int t0, int t1, int t2, int t3)
{
    VSHUF4I_B(t0, dst, 0x4e);
    VXOR_V(t0, t0, dst);
    la64_xtime_table_lsx(dyn, ninst, t0, t0, t1, tab_lo, tab_hi);
    la64_xtime_table_lsx(dyn, ninst, t0, t0, t1, tab_lo, tab_hi);
    VXOR_V(dst, dst, t0);
    la64_mixcolumns_xtime_lsx(dyn, ninst, dst, tab_lo, tab_hi, t0, t1, t2, t3);
}

static inline void la64_xtime_table_lasx(dynarec_la64_t* dyn, int ninst, int dst, int src, int tmp, int tab_lo, int tab_hi)
{
    (void)dyn;
    (void)ninst;
    XVANDI_B(tmp, src, 0x0f);
    XVSRLI_B(dst, src, 4);
    XVSHUF_B(tmp, tab_lo, tab_lo, tmp);
    XVSHUF_B(dst, tab_hi, tab_hi, dst);
    XVXOR_V(dst, dst, tmp);
}

static inline void la64_mixcolumns_xtime_lasx(dynarec_la64_t* dyn, int ninst, int dst, int tab_lo, int tab_hi, int t0, int t1, int t2, int t3)
{
    la64_xtime_table_lasx(dyn, ninst, t0, dst, t3, tab_lo, tab_hi);
    XVSHUF4I_B(t1, dst, 0x39);
    XVSHUF4I_B(t2, dst, 0x4e);
    la64_xtime_table_lasx(dyn, ninst, dst, t1, t3, tab_lo, tab_hi);
    XVSHUF4I_B(t3, t2, 0x39);
    XVXOR_V(t2, t2, t3);
    XVXOR_V(dst, dst, t1);
    XVXOR_V(dst, dst, t0);
    XVXOR_V(dst, dst, t2);
}

static inline void la64_invmixcolumns_xtime_lasx(dynarec_la64_t* dyn, int ninst, int dst, int tab_lo, int tab_hi, int t0, int t1, int t2, int t3)
{
    XVSHUF4I_B(t0, dst, 0x4e);
    XVXOR_V(t0, t0, dst);
    la64_xtime_table_lasx(dyn, ninst, t0, t0, t1, tab_lo, tab_hi);
    la64_xtime_table_lasx(dyn, ninst, t0, t0, t1, tab_lo, tab_hi);
    XVXOR_V(dst, dst, t0);
    la64_mixcolumns_xtime_lasx(dyn, ninst, dst, tab_lo, tab_hi, t0, t1, t2, t3);
}

static inline void la64_aesimc_lsx(dynarec_la64_t* dyn, int ninst, int dst)
{
    int tab_lo = fpu_get_scratch(dyn);
    int tab_hi = fpu_get_scratch(dyn);
    int t0 = fpu_get_scratch(dyn);
    int t1 = fpu_get_scratch(dyn);
    int t2 = fpu_get_scratch(dyn);
    int t3 = fpu_get_scratch(dyn);

    TABLE64C(x5, const_la64_vpaes_dec_tables);
    VLD(tab_lo, x5, 7 * 16);
    VLD(tab_hi, x5, 8 * 16);
    la64_invmixcolumns_xtime_lsx(dyn, ninst, dst, tab_lo, tab_hi, t0, t1, t2, t3);
}

static inline void la64_aesimc_lasx(dynarec_la64_t* dyn, int ninst, int dst)
{
    int tab_lo = fpu_get_scratch(dyn);
    int tab_hi = fpu_get_scratch(dyn);
    int t0 = fpu_get_scratch(dyn);
    int t1 = fpu_get_scratch(dyn);
    int t2 = fpu_get_scratch(dyn);
    int t3 = fpu_get_scratch(dyn);

    TABLE64C(x5, const_la64_vpaes_dec_tables_xv);
    XVLD(tab_lo, x5, 7 * 32);
    XVLD(tab_hi, x5, 8 * 32);
    la64_invmixcolumns_xtime_lasx(dyn, ninst, dst, tab_lo, tab_hi, t0, t1, t2, t3);
}

static inline void la64_aeselast_lsx(dynarec_la64_t* dyn, int ninst, int dst)
{
    int zero = fpu_get_scratch(dyn);
    int t0 = fpu_get_scratch(dyn);
    int t1 = fpu_get_scratch(dyn);
    int t2 = fpu_get_scratch(dyn);
    int t3 = fpu_get_scratch(dyn);
    int tab = fpu_get_scratch(dyn);

    TABLE64C(x5, const_la64_vpaes_enc_tables);
    la64_subbytes_lsx(dyn, ninst, dst, zero, t0, t1, t2, t3, tab);
    VLD(tab, x5, 6 * 16);
    VSHUF_B(dst, dst, dst, tab);
}

static inline void la64_aeselast_lasx(dynarec_la64_t* dyn, int ninst, int dst)
{
    int zero = fpu_get_scratch(dyn);
    int t0 = fpu_get_scratch(dyn);
    int t1 = fpu_get_scratch(dyn);
    int t2 = fpu_get_scratch(dyn);
    int t3 = fpu_get_scratch(dyn);
    int tab = fpu_get_scratch(dyn);

    TABLE64C(x5, const_la64_vpaes_enc_tables_xv);
    la64_subbytes_lasx(dyn, ninst, dst, zero, t0, t1, t2, t3, tab);
    XVLD(tab, x5, 6 * 32);
    XVSHUF_B(dst, dst, dst, tab);
}

static inline void la64_aese_lsx(dynarec_la64_t* dyn, int ninst, int dst)
{
    int zero = fpu_get_scratch(dyn);
    int t0 = fpu_get_scratch(dyn);
    int t1 = fpu_get_scratch(dyn);
    int t2 = fpu_get_scratch(dyn);
    int t3 = fpu_get_scratch(dyn);
    int tab = fpu_get_scratch(dyn);

    TABLE64C(x5, const_la64_vpaes_enc_tables);
    la64_subbytes_lsx(dyn, ninst, dst, zero, t0, t1, t2, t3, tab);
    VLD(tab, x5, 6 * 16);
    VSHUF_B(dst, dst, dst, tab);
    VLD(tab, x5, 7 * 16);
    VLD(zero, x5, 8 * 16);
    la64_mixcolumns_xtime_lsx(dyn, ninst, dst, tab, zero, t0, t1, t2, t3);
}

static inline void la64_aese_lasx(dynarec_la64_t* dyn, int ninst, int dst)
{
    int zero = fpu_get_scratch(dyn);
    int t0 = fpu_get_scratch(dyn);
    int t1 = fpu_get_scratch(dyn);
    int t2 = fpu_get_scratch(dyn);
    int t3 = fpu_get_scratch(dyn);
    int tab = fpu_get_scratch(dyn);

    TABLE64C(x5, const_la64_vpaes_enc_tables_xv);
    la64_subbytes_lasx(dyn, ninst, dst, zero, t0, t1, t2, t3, tab);
    XVLD(tab, x5, 6 * 32);
    XVSHUF_B(dst, dst, dst, tab);
    XVLD(tab, x5, 7 * 32);
    XVLD(zero, x5, 8 * 32);
    la64_mixcolumns_xtime_lasx(dyn, ninst, dst, tab, zero, t0, t1, t2, t3);
}

static inline void la64_aesdlast_lsx(dynarec_la64_t* dyn, int ninst, int dst)
{
    int zero = fpu_get_scratch(dyn);
    int t0 = fpu_get_scratch(dyn);
    int t1 = fpu_get_scratch(dyn);
    int t2 = fpu_get_scratch(dyn);
    int t3 = fpu_get_scratch(dyn);
    int t4 = fpu_get_scratch(dyn);
    int tab = fpu_get_scratch(dyn);

    TABLE64C(x5, const_la64_vpaes_dec_tables);
    VLD(tab, x5, 6 * 16);
    VSHUF_B(dst, dst, dst, tab);
    VXOR_V(zero, zero, zero);
    VXORI_B(dst, dst, 99);
    VANDI_B(t0, dst, 0x0f);
    VSRLI_B(t1, dst, 4);
    VLD(tab, x5, 1 * 16);
    VSHUF_B(t1, zero, tab, t1);
    VLD(tab, x5, 0 * 16);
    VSHUF_B(dst, zero, tab, t0);
    VXOR_V(dst, dst, t1);
    VANDI_B(t0, dst, 0x0f);
    VSRLI_B(t1, dst, 4);
    VLD(tab, x5, 2 * 16);
    VSHUF_B(t2, zero, tab, t1);
    VLD(tab, x5, 3 * 16);
    VSHUF_B(t3, zero, tab, t0);
    VXOR_V(t0, t1, t0);
    VXOR_V(t2, t2, t3);
    VLD(tab, x5, 2 * 16);
    VSHUF_B(t4, zero, tab, t0);
    VSHUF_B(t2, zero, tab, t2);
    VXOR_V(t3, t4, t3);
    VXOR_V(t0, t2, t0);
    VSHUF_B(t3, zero, tab, t3);
    VXOR_V(t3, t3, t1);
    VLD(tab, x5, 4 * 16);
    VSHUF_B(t0, zero, tab, t0);
    VLD(tab, x5, 5 * 16);
    VSHUF_B(t1, zero, tab, t3);
    VXOR_V(dst, t0, t1);
}

static inline void la64_aesdlast_lasx(dynarec_la64_t* dyn, int ninst, int dst)
{
    int zero = fpu_get_scratch(dyn);
    int t0 = fpu_get_scratch(dyn);
    int t1 = fpu_get_scratch(dyn);
    int t2 = fpu_get_scratch(dyn);
    int t3 = fpu_get_scratch(dyn);
    int t4 = fpu_get_scratch(dyn);
    int tab = fpu_get_scratch(dyn);

    TABLE64C(x5, const_la64_vpaes_dec_tables_xv);
    XVLD(tab, x5, 6 * 32);
    XVSHUF_B(dst, dst, dst, tab);
    XVXOR_V(zero, zero, zero);
    XVXORI_B(dst, dst, 99);
    XVANDI_B(t0, dst, 0x0f);
    XVSRLI_B(t1, dst, 4);
    XVLD(tab, x5, 1 * 32);
    XVSHUF_B(t1, zero, tab, t1);
    XVLD(tab, x5, 0 * 32);
    XVSHUF_B(dst, zero, tab, t0);
    XVXOR_V(dst, dst, t1);
    XVANDI_B(t0, dst, 0x0f);
    XVSRLI_B(t1, dst, 4);
    XVLD(tab, x5, 2 * 32);
    XVSHUF_B(t2, zero, tab, t1);
    XVLD(tab, x5, 3 * 32);
    XVSHUF_B(t3, zero, tab, t0);
    XVXOR_V(t0, t1, t0);
    XVXOR_V(t2, t2, t3);
    XVLD(tab, x5, 2 * 32);
    XVSHUF_B(t4, zero, tab, t0);
    XVSHUF_B(t2, zero, tab, t2);
    XVXOR_V(t3, t4, t3);
    XVXOR_V(t0, t2, t0);
    XVSHUF_B(t3, zero, tab, t3);
    XVXOR_V(t3, t3, t1);
    XVLD(tab, x5, 4 * 32);
    XVSHUF_B(t0, zero, tab, t0);
    XVLD(tab, x5, 5 * 32);
    XVSHUF_B(t1, zero, tab, t3);
    XVXOR_V(dst, t0, t1);
}

static inline void la64_aesd_lsx(dynarec_la64_t* dyn, int ninst, int dst)
{
    int zero = fpu_get_scratch(dyn);
    int t0 = fpu_get_scratch(dyn);
    int t1 = fpu_get_scratch(dyn);
    int t2 = fpu_get_scratch(dyn);
    int t3 = fpu_get_scratch(dyn);
    int t4 = fpu_get_scratch(dyn);
    int tab = fpu_get_scratch(dyn);

    TABLE64C(x5, const_la64_vpaes_dec_tables);
    VLD(tab, x5, 6 * 16);
    VSHUF_B(dst, dst, dst, tab);
    VXOR_V(zero, zero, zero);
    VXORI_B(dst, dst, 99);
    VANDI_B(t0, dst, 0x0f);
    VSRLI_B(t1, dst, 4);
    VLD(tab, x5, 1 * 16);
    VSHUF_B(t1, zero, tab, t1);
    VLD(tab, x5, 0 * 16);
    VSHUF_B(dst, zero, tab, t0);
    VXOR_V(dst, dst, t1);
    VANDI_B(t0, dst, 0x0f);
    VSRLI_B(t1, dst, 4);
    VLD(tab, x5, 2 * 16);
    VSHUF_B(t2, zero, tab, t1);
    VLD(tab, x5, 3 * 16);
    VSHUF_B(t3, zero, tab, t0);
    VXOR_V(t0, t1, t0);
    VXOR_V(t2, t2, t3);
    VLD(tab, x5, 2 * 16);
    VSHUF_B(t4, zero, tab, t0);
    VSHUF_B(t2, zero, tab, t2);
    VXOR_V(t3, t4, t3);
    VXOR_V(t0, t2, t0);
    VSHUF_B(t3, zero, tab, t3);
    VXOR_V(t3, t3, t1);
    VLD(tab, x5, 4 * 16);
    VSHUF_B(t0, zero, tab, t0);
    VLD(tab, x5, 5 * 16);
    VSHUF_B(t1, zero, tab, t3);
    VXOR_V(dst, t0, t1);
    VLD(tab, x5, 7 * 16);
    VLD(zero, x5, 8 * 16);
    la64_invmixcolumns_xtime_lsx(dyn, ninst, dst, tab, zero, t0, t1, t2, t3);
}

static inline void la64_aesd_lasx(dynarec_la64_t* dyn, int ninst, int dst)
{
    int zero = fpu_get_scratch(dyn);
    int t0 = fpu_get_scratch(dyn);
    int t1 = fpu_get_scratch(dyn);
    int t2 = fpu_get_scratch(dyn);
    int t3 = fpu_get_scratch(dyn);
    int t4 = fpu_get_scratch(dyn);
    int tab = fpu_get_scratch(dyn);

    TABLE64C(x5, const_la64_vpaes_dec_tables_xv);
    XVLD(tab, x5, 6 * 32);
    XVSHUF_B(dst, dst, dst, tab);
    XVXOR_V(zero, zero, zero);
    XVXORI_B(dst, dst, 99);
    XVANDI_B(t0, dst, 0x0f);
    XVSRLI_B(t1, dst, 4);
    XVLD(tab, x5, 1 * 32);
    XVSHUF_B(t1, zero, tab, t1);
    XVLD(tab, x5, 0 * 32);
    XVSHUF_B(dst, zero, tab, t0);
    XVXOR_V(dst, dst, t1);
    XVANDI_B(t0, dst, 0x0f);
    XVSRLI_B(t1, dst, 4);
    XVLD(tab, x5, 2 * 32);
    XVSHUF_B(t2, zero, tab, t1);
    XVLD(tab, x5, 3 * 32);
    XVSHUF_B(t3, zero, tab, t0);
    XVXOR_V(t0, t1, t0);
    XVXOR_V(t2, t2, t3);
    XVLD(tab, x5, 2 * 32);
    XVSHUF_B(t4, zero, tab, t0);
    XVSHUF_B(t2, zero, tab, t2);
    XVXOR_V(t3, t4, t3);
    XVXOR_V(t0, t2, t0);
    XVSHUF_B(t3, zero, tab, t3);
    XVXOR_V(t3, t3, t1);
    XVLD(tab, x5, 4 * 32);
    XVSHUF_B(t0, zero, tab, t0);
    XVLD(tab, x5, 5 * 32);
    XVSHUF_B(t1, zero, tab, t3);
    XVXOR_V(dst, t0, t1);
    XVLD(tab, x5, 7 * 32);
    XVLD(zero, x5, 8 * 32);
    la64_invmixcolumns_xtime_lasx(dyn, ninst, dst, tab, zero, t0, t1, t2, t3);
}

static inline void la64_aeskeygenassist_lsx(dynarec_la64_t* dyn, int ninst, int dst, uint8_t imm)
{
    int zero = fpu_get_scratch(dyn);
    int t0 = fpu_get_scratch(dyn);
    int t1 = fpu_get_scratch(dyn);
    int t2 = fpu_get_scratch(dyn);
    int t3 = fpu_get_scratch(dyn);
    int tab = fpu_get_scratch(dyn);
    int rcon = fpu_get_scratch(dyn);

    TABLE64C(x5, const_la64_vpaes_keygen_tables);
    la64_subbytes_lsx(dyn, ninst, dst, zero, t0, t1, t2, t3, tab);
    VLD(tab, x5, 6 * 16);
    VSHUF_B(dst, zero, dst, tab);
    VLDI(rcon, imm);
    VLD(tab, x5, 7 * 16);
    VAND_V(rcon, rcon, tab);
    VXOR_V(dst, dst, rcon);
}

#endif
