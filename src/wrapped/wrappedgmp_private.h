#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

// mp_bitcnt_t unsigned long int

//DATA(__gmp_allocate_func, 
//GOM(__gmp_asprintf, iFEppV)
//GO(__gmp_asprintf_final, 
//DATA(__gmp_asprintf_funs, 
//GO(__gmp_asprintf_memory, 
//GO(__gmp_asprintf_reps, 
//GO(__gmp_assert_fail, 
//GO(__gmp_assert_header, 
//GO(__gmp_default_allocate, 
//DATA(__gmp_default_fp_limb_precision, 
//GO(__gmp_default_free, 
//GO(__gmp_default_reallocate, 
//GO(__gmp_divide_by_zero, 
//GO(__gmp_doprnt, 
//GO(__gmp_doprnt_integer, 
//GO(__gmp_doprnt_mpf2, 
//GO(__gmp_doscan, 
//DATAB(__gmp_errno, 4)
//GO(__gmp_exception, 
//GO(__gmp_extract_double, 
GO(__gmpf_abs, vFpp)
GO(__gmpf_add, vFppp)
GO(__gmpf_add_ui, vFppL)
GO(__gmpf_ceil, vFpp)
GO(__gmpf_clear, vFp)
//GOM(__gmpf_clears, vFEpV)
GO(__gmpf_cmp, iFpp)
GO(__gmpf_cmp_d, iFpd)
GO(__gmpf_cmp_si, iFpl)
GO(__gmpf_cmp_ui, iFpL)
GO(__gmpf_cmp_z, iFpp)
GO(__gmpf_div, vFppp)
GO(__gmpf_div_2exp, vFppL)
GO(__gmpf_div_ui, vFppL)
GO(__gmpf_dump, vFp)
GO(__gmpf_eq, iFppL)
GO(__gmpf_fits_sint_p, iFp)
GO(__gmpf_fits_slong_p, iFp)
GO(__gmpf_fits_sshort_p, iFp)
GO(__gmpf_fits_uint_p, iFp)
GO(__gmpf_fits_ulong_p, iFp)
GO(__gmpf_fits_ushort_p, iFp)
GO(__gmpf_floor, vFpp)
GO(__gmpf_get_d, dFp)
GO(__gmpf_get_d_2exp, dFpp)
GO(__gmpf_get_default_prec, LFv)
GO(__gmpf_get_prec, LFp)
GO(__gmpf_get_si, lFp)
GO(__gmpf_get_str, pFppiLp)
GO(__gmpf_get_ui, LFp)
GO(__gmpf_init, vFp)
GO(__gmpf_init2, vFpL)
//GOM(__gmpf_inits, vFEpV)
GO(__gmpf_init_set, vFpp)
GO(__gmpf_init_set_d, vFpd)
GO(__gmpf_init_set_si, vFpl)
GO(__gmpf_init_set_str, iFppi)
GO(__gmpf_init_set_ui, vFpL)
//GO(__gmpf_inp_str, 
GO(__gmpf_integer_p, iFp)
GO(__gmpf_mul, vFppp)
GO(__gmpf_mul_2exp, vFppL)
GO(__gmpf_mul_ui, vFppL)
GO(__gmpf_neg, vFpp)
//GO(__gmpf_out_str, 
GO(__gmpf_pow_ui, vFppL)
//GO(__gmp_fprintf, 
//DATA(__gmp_fprintf_funs, 
GO(__gmpf_random2, vFpll)
//DATA(__gmp_free_func, 
GO(__gmpf_reldiff, vFppp)
//GO(__gmp_fscanf, 
//DATA(__gmp_fscanf_funs, 
GO(__gmpf_set, vFpp)
GO(__gmpf_set_d, vFpd)
GO(__gmpf_set_default_prec, vFL)
GO(__gmpf_set_prec, vFpL)
GO(__gmpf_set_prec_raw, vFpL)
GO(__gmpf_set_q, vFpp)
GO(__gmpf_set_si, vFpl)
GO(__gmpf_set_str, iFppi)
GO(__gmpf_set_ui, vFpL)
GO(__gmpf_set_z, vFpp)
GO(__gmpf_size, LFp)
GO(__gmpf_sqrt, vFpp)
GO(__gmpf_sqrt_ui, vFpL)
GO(__gmpf_sub, vFppp)
GO(__gmpf_sub_ui, vFppL)
GO(__gmpf_swap, vFpp)
GO(__gmpf_trunc, vFpp)
GO(__gmpf_ui_div, vFpLp)
GO(__gmpf_ui_sub, vFpLp)
GO(__gmpf_urandomb, vFppL)
GOM(__gmp_get_memory_functions, vFEppp)
//GO(__gmp_init_primesieve, 
//GO(__gmp_invalid_operation, 
//DATAB(__gmp_junk, 
//GO(__gmp_mt_recalc_buffer, 
GO(__gmpn_add, LFpplpl)
GO(__gmpn_add_1, LFpplL)
//GO(__gmpn_add_err1_n, 
//GO(__gmpn_add_err2_n, 
//GO(__gmpn_add_err3_n, 
//GO(__gmpn_addlsh1_n, 
//GO(__gmpn_addlsh2_n, 
GO(__gmpn_addmul_1, LFpplL)
GO(__gmpn_add_n, LFpppl)
//GO(__gmpn_add_nc, 
//GO(__gmpn_add_n_sub_n, 
GO(__gmpn_and_n, vFpppl)
GO(__gmpn_andn_n, vFpppl)
//GO(__gmpn_bc_mulmod_bnm1, 
//GO(__gmpn_bc_set_str, 
//GO(__gmpn_bdiv_dbm1c, 
//GO(__gmpn_bdiv_q, 
//GO(__gmpn_bdiv_q_1, 
//GO(__gmpn_bdiv_q_itch, 
//GO(__gmpn_bdiv_qr, 
//GO(__gmpn_bdiv_qr_itch, 
//GO(__gmpn_binvert, 
//GO(__gmpn_binvert_itch, 
//GO(__gmpn_broot, 
//GO(__gmpn_brootinv, 
//GO(__gmpn_broot_invm1, 
//GO(__gmpn_bsqrt, 
//GO(__gmpn_bsqrtinv, 
GO(__gmpn_cmp, iFppl)
GO(__gmpn_cnd_add_n, LFLpppl)
GO(__gmpn_cnd_sub_n, LFLpppl)
GO(__gmpn_cnd_swap, vFLppl)
GO(__gmpn_com, vFppl)
//GO(__gmpn_compute_powtab, 
GO(__gmpn_copyd, vFppl)
GO(__gmpn_copyi, vFppl)
//GO(__gmpn_dcpi1_bdiv_q, 
//GO(__gmpn_dcpi1_bdiv_qr, 
//GO(__gmpn_dcpi1_bdiv_qr_n, 
//GO(__gmpn_dcpi1_bdiv_qr_n_itch, 
//GO(__gmpn_dcpi1_divappr_q, 
//GO(__gmpn_dcpi1_div_q, 
//GO(__gmpn_dcpi1_div_qr, 
//GO(__gmpn_dcpi1_div_qr_n, 
//GO(__gmpn_dc_set_str, 
//GO(__gmpn_divexact, 
GO(__gmpn_divexact_1, vFpplL)
//GO(__gmpn_divexact_by3, 
GO(__gmpn_divexact_by3c, LFpplL)
//GO(__gmpn_divisible_p, 
//GO(__gmpn_divmod_1, 
//GO(__gmpn_div_q, 
GO(__gmpn_div_qr_1, LFppplL)
//GO(__gmpn_div_qr_1n_pi1, 
GO(__gmpn_div_qr_2, LFppplp)
//GO(__gmpn_div_qr_2n_pi1, 
//GO(__gmpn_div_qr_2u_pi1, 
GO(__gmpn_divrem, LFplplpl)
GO(__gmpn_divrem_1, LFplplL)
GO(__gmpn_divrem_2, LFplplp)
//GO(__gmpn_dump, 
//GO(__gmp_nextprime, 
//GO(__gmpn_fft_best_k, 
//GO(__gmpn_fft_next_size, 
//GO(__gmpn_fib2m, 
//GO(__gmpn_fib2_ui, 
GO(__gmpn_gcd, lFpplpl)
GO(__gmpn_gcd_1, LFplL)
GO(__gmpn_gcd_11, LFLL)
//GO(__gmpn_gcd_22, 
GO(__gmpn_gcdext, lFpppplpl)
GO(__gmpn_gcdext_1, LFppLL)
//GO(__gmpn_gcdext_hook, 
//GO(__gmpn_gcdext_lehmer_n, 
//GO(__gmpn_gcd_subdiv_step, 
//GO(__gmpn_get_d, 
GO(__gmpn_get_str, LFpipl)
GO(__gmpn_hamdist, LFppl)
//GO(__gmpn_hgcd, 
//GO(__gmpn_hgcd2, 
//GO(__gmpn_hgcd2_jacobi, 
//GO(__gmpn_hgcd_appr, 
//GO(__gmpn_hgcd_appr_itch, 
//GO(__gmpn_hgcd_itch, 
//GO(__gmpn_hgcd_jacobi, 
//GO(__gmpn_hgcd_matrix_adjust, 
//GO(__gmpn_hgcd_matrix_init, 
//GO(__gmpn_hgcd_matrix_mul, 
//GO(__gmpn_hgcd_matrix_mul_1, 
//GO(__gmpn_hgcd_matrix_update_q, 
//GO(__gmpn_hgcd_mul_matrix1_vector, 
//GO(__gmpn_hgcd_reduce, 
//GO(__gmpn_hgcd_reduce_itch, 
//GO(__gmpn_hgcd_step, 
//GO(__gmpn_invert, 
//GO(__gmpn_invertappr, 
//GO(__gmpn_invert_limb, 
GO(__gmpn_ior_n, vFpppl)
GO(__gmpn_iorn_n, vFpppl)
//GO(__gmpn_jacobi_2, 
//GO(__gmpn_jacobi_base, 
//GO(__gmpn_jacobi_n, 
GO(__gmpn_lshift, LFpplu)
//GO(__gmpn_lshiftc, 
//GO(__gmpn_matrix22_mul, 
//GO(__gmpn_matrix22_mul1_inverse_vector, 
//GO(__gmpn_matrix22_mul_itch, 
GO(__gmpn_mod_1, LFplL)
//GO(__gmpn_mod_1_1p, 
//GO(__gmpn_mod_1_1p_cps, 
//GO(__gmpn_mod_1s_2p, 
//GO(__gmpn_mod_1s_2p_cps, 
//GO(__gmpn_mod_1s_3p, 
//GO(__gmpn_mod_1s_3p_cps, 
//GO(__gmpn_mod_1s_4p, 
//GO(__gmpn_mod_1s_4p_cps, 
//GO(__gmpn_mod_34lsub1, 
//GO(__gmpn_modexact_1c_odd, 
//GO(__gmpn_mu_bdiv_q, 
//GO(__gmpn_mu_bdiv_q_itch, 
//GO(__gmpn_mu_bdiv_qr, 
//GO(__gmpn_mu_bdiv_qr_itch, 
//GO(__gmpn_mu_divappr_q, 
//GO(__gmpn_mu_divappr_q_itch, 
//GO(__gmpn_mu_div_q, 
//GO(__gmpn_mu_div_q_itch, 
//GO(__gmpn_mu_div_qr, 
//GO(__gmpn_mu_div_qr_itch, 
GO(__gmpn_mul, LFpplpl)
GO(__gmpn_mul_1, LFpplL)
//GO(__gmpn_mul_1c, 
//GO(__gmpn_mul_basecase, 
//GO(__gmpn_mul_fft, 
//GO(__gmpn_mullo_basecase, 
//GO(__gmpn_mullo_n, 
//GO(__gmpn_mulmid, 
//GO(__gmpn_mulmid_basecase, 
//GO(__gmpn_mulmid_n, 
//GO(__gmpn_mulmod_bnm1, 
//GO(__gmpn_mulmod_bnm1_next_size, 
GO(__gmpn_mul_n, vFpppl)
GO(__gmpn_nand_n, vFpppl)
GO(__gmpn_neg, LFppl)
//GO(__gmpn_ni_invertappr, 
GO(__gmpn_nior_n, vFpppl)
//GO(__gmpn_nussbaumer_mul, 
GO(__gmpn_perfect_power_p, iFpl)
GO(__gmpn_perfect_square_p, iFpl)
//GO(__gmpn_pi1_bdiv_q_1, 
GO(__gmpn_popcount, LFpl)
GO(__gmpn_pow_1, lFpplLp)
//GO(__gmpn_powlo, 
//GO(__gmpn_powm, 
//GO(__gmpn_preinv_divrem_1, 
GO(__gmpn_preinv_mod_1, LFplLL)
//GO(__gmpn_preinv_mu_div_qr, 
//GO(__gmpn_preinv_mu_div_qr_itch, 
GO(__gmpn_random, vFpl)
GO(__gmpn_random2, vFpl)
//GO(__gmpn_redc_1, 
//GO(__gmpn_redc_2, 
//GO(__gmpn_redc_n, 
//GO(__gmpn_remove, 
//GO(__gmpn_rootrem, 
//GO(__gmpn_rsblsh1_n, 
//GO(__gmpn_rsblsh2_n, 
//GO(__gmpn_rsh1add_n, 
//GO(__gmpn_rsh1sub_n, 
GO(__gmpn_rshift, LFpplu)
//GO(__gmpn_sbpi1_bdiv_q, 
//GO(__gmpn_sbpi1_bdiv_qr, 
//GO(__gmpn_sbpi1_bdiv_r, 
//GO(__gmpn_sbpi1_divappr_q, 
//GO(__gmpn_sbpi1_div_q, 
//GO(__gmpn_sbpi1_div_qr, 
GO(__gmpn_scan0, LFpL)
GO(__gmpn_scan1, LFpL)
GO(__gmpn_sec_add_1, LFpplLp)
GO(__gmpn_sec_add_1_itch, lFl)
GO(__gmpn_sec_div_qr, LFpplplp)
GO(__gmpn_sec_div_qr_itch, lFll)
GO(__gmpn_sec_div_r, vFplplp)
GO(__gmpn_sec_div_r_itch, lFll)
GO(__gmpn_sec_invert, iFppplLp)
GO(__gmpn_sec_invert_itch, lFl)
GO(__gmpn_sec_mul, vFpplplp)
GO(__gmpn_sec_mul_itch, lFll)
//GO(__gmpn_sec_pi1_div_qr, 
//GO(__gmpn_sec_pi1_div_r, 
GO(__gmpn_sec_powm, vFpplpLplp)
GO(__gmpn_sec_powm_itch, lFlLl)
GO(__gmpn_sec_sqr, vFpplp)
GO(__gmpn_sec_sqr_itch, lFl)
GO(__gmpn_sec_sub_1, LFpplLp)
GO(__gmpn_sec_sub_1_itch, lFl)
GO(__gmpn_sec_tabselect, vFpplll)
GO(__gmpn_set_str, lFppLi)
GO(__gmpn_sizeinbase, LFpli)
GO(__gmpn_sqr, vFppl)
//GO(__gmpn_sqr_basecase, 
//GO(__gmpn_sqr_diag_addlsh1, 
//GO(__gmpn_sqrlo, 
//GO(__gmpn_sqrlo_basecase, 
//GO(__gmpn_sqrmod_bnm1, 
//GO(__gmpn_sqrmod_bnm1_next_size, 
GO(__gmpn_sqrtrem, lFpppl)
//GO(__gmpn_strongfibo, 
GO(__gmpn_sub, LFpplpl)
GO(__gmpn_sub_1, LFpplL)
//GO(__gmpn_sub_err1_n, 
//GO(__gmpn_sub_err2_n, 
//GO(__gmpn_sub_err3_n, 
//GO(__gmpn_sublsh1_n, 
//GO(__gmpn_sublsh2_n, 
GO(__gmpn_submul_1, LFpplL)
GO(__gmpn_sub_n, LFpppl)
//GO(__gmpn_sub_nc, 
GO(__gmpn_tdiv_qr, vFpplplpl)
//GO(__gmpn_toom22_mul, 
//GO(__gmpn_toom2_sqr, 
//GO(__gmpn_toom32_mul, 
//GO(__gmpn_toom33_mul, 
//GO(__gmpn_toom3_sqr, 
//GO(__gmpn_toom42_mul, 
//GO(__gmpn_toom42_mulmid, 
//GO(__gmpn_toom43_mul, 
//GO(__gmpn_toom44_mul, 
//GO(__gmpn_toom4_sqr, 
//GO(__gmpn_toom52_mul, 
//GO(__gmpn_toom53_mul, 
//GO(__gmpn_toom54_mul, 
//GO(__gmpn_toom62_mul, 
//GO(__gmpn_toom63_mul, 
//GO(__gmpn_toom6h_mul, 
//GO(__gmpn_toom6_sqr, 
//GO(__gmpn_toom8h_mul, 
//GO(__gmpn_toom8_sqr, 
//GO(__gmpn_toom_couple_handling, 
//GO(__gmpn_toom_eval_dgr3_pm1, 
//GO(__gmpn_toom_eval_dgr3_pm2, 
//GO(__gmpn_toom_eval_pm1, 
//GO(__gmpn_toom_eval_pm2, 
//GO(__gmpn_toom_eval_pm2exp, 
//GO(__gmpn_toom_eval_pm2rexp, 
//GO(__gmpn_toom_interpolate_12pts, 
//GO(__gmpn_toom_interpolate_16pts, 
//GO(__gmpn_toom_interpolate_5pts, 
//GO(__gmpn_toom_interpolate_6pts, 
//GO(__gmpn_toom_interpolate_7pts, 
//GO(__gmpn_toom_interpolate_8pts, 
//GO(__gmpn_trialdiv, 
GO(__gmpn_xnor_n, vFpppl)
GO(__gmpn_xor_n, vFpppl)
GO(__gmpn_zero, vFpl)
GO(__gmpn_zero_p, iFpl)
//GO(__gmp_obstack_printf, 
//DATA(__gmp_obstack_printf_funs, 
//GO(__gmp_obstack_vprintf, 
//GO(__gmp_primesieve, 
//GOM(__gmp_printf, iFEpV)
GO(__gmpq_abs, vFpp)
GO(__gmpq_add, vFppp)
GO(__gmpq_canonicalize, vFp)
GO(__gmpq_clear, vFp)
//GOM(__gmpq_clears, vFEpV)
GO(__gmpq_cmp, iFpp)
GO(__gmpq_cmp_si, iFplL)
GO(__gmpq_cmp_ui, iFpLL)
GO(__gmpq_cmp_z, iFpp)
GO(__gmpq_div, vFppp)
GO(__gmpq_div_2exp, vFppL)
GO(__gmpq_equal, iFpp)
GO(__gmpq_get_d, dFp)
GO(__gmpq_get_den, vFpp)
GO(__gmpq_get_num, vFpp)
GO(__gmpq_get_str, pFpip)
GO(__gmpq_init, vFp)
//GOM(__gmpq_inits, vFEpV)
//GO(__gmpq_inp_str, 
GO(__gmpq_inv, vFpp)
GO(__gmpq_mul, vFppp)
GO(__gmpq_mul_2exp, vFppL)
GO(__gmpq_neg, vFpp)
//GO(__gmpq_out_str, 
GO(__gmpq_set, vFpp)
GO(__gmpq_set_d, vFpd)
GO(__gmpq_set_den, vFpp)
GO(__gmpq_set_f, vFpp)
GO(__gmpq_set_num, vFpp)
GO(__gmpq_set_si, vFplL)
GO(__gmpq_set_str, iFppi)
GO(__gmpq_set_ui, vFpLL)
GO(__gmpq_set_z, vFpp)
GO(__gmpq_sub, vFppp)
GO(__gmpq_swap, vFpp)
GO(__gmp_randclear, vFp)
//GO(__gmp_randclear_mt, 
//GO(__gmp_randget_mt, 
//GOM(__gmp_randinit, vFEpuV)
GO(__gmp_randinit_default, vFp)
GO(__gmp_randinit_lc_2exp, vFppLL)
GO(__gmp_randinit_lc_2exp_size, iFpL)
GO(__gmp_randinit_mt, vFp)
//GO(__gmp_randinit_mt_noseed, 
GO(__gmp_randinit_set, vFpp)
//GO(__gmp_randiset_mt, 
//DATAB(__gmp_rands, 
GO(__gmp_randseed, vFpp)
GO(__gmp_randseed_ui, vFpL)
//DATAB(__gmp_rands_initialized, 
//DATA(__gmp_reallocate_func, 
//GOM(__gmp_scanf, iFEpV)
GOM(__gmp_set_memory_functions, vFEppp)
//GOM(__gmp_snprintf, iFEpLpV)
//DATA(__gmp_snprintf_funs, 
//GOM(__gmp_sprintf, iFEppV)
//DATA(__gmp_sprintf_funs, 
//GO(__gmp_sqrt_of_negative, 
//GOM(__gmp_sscanf, iFEppV)
//DATA(__gmp_sscanf_funs, 
//GO(__gmp_tmp_reentrant_alloc, 
//GO(__gmp_tmp_reentrant_free, 
GO(__gmp_urandomb_ui, LFpL)
GO(__gmp_urandomm_ui, LFpL)
//GO(__gmp_vasprintf, 
//DATA(__gmp_version, 8)
//GO(__gmp_vfprintf, 
//GO(__gmp_vfscanf, 
//GO(__gmp_vprintf, 
//GO(__gmp_vscanf, 
//GO(__gmp_vsnprintf, 
//GO(__gmp_vsprintf, 
//GO(__gmp_vsscanf, 
GO(__gmpz_2fac_ui, vFpL)
GO(__gmpz_abs, vFpp)
GO(__gmpz_add, vFppp)
GO(__gmpz_addmul, vFppp)
GO(__gmpz_addmul_ui, vFppL)
GO(__gmpz_add_ui, vFppL)
GO(__gmpz_and, vFppp)
//GO(__gmpz_aorsmul_1, 
GO(__gmpz_array_init, vFpll)
GO(__gmpz_bin_ui, vFppL)
GO(__gmpz_bin_uiui, vFpLL)
GO(__gmpz_cdiv_q, vFppp)
GO(__gmpz_cdiv_q_2exp, vFppL)
GO(__gmpz_cdiv_qr, vFpppp)
GO(__gmpz_cdiv_qr_ui, LFpppL)
GO(__gmpz_cdiv_q_ui, LFppL)
GO(__gmpz_cdiv_r, vFppp)
GO(__gmpz_cdiv_r_2exp, vFppL)
GO(__gmpz_cdiv_r_ui, LFppL)
GO(__gmpz_cdiv_ui, LFpL)
GO(__gmpz_clear, vFp)
//GOM(__gmpz_clears, vFEpV)
GO(__gmpz_clrbit, vFpL)
GO(__gmpz_cmp, iFpp)
GO(__gmpz_cmpabs, iFpp)
GO(__gmpz_cmpabs_d, iFpd)
GO(__gmpz_cmpabs_ui, iFpL)
GO(__gmpz_cmp_d, iFpd)
GO(__gmpz_cmp_si, iFpl)
GO(__gmpz_cmp_ui, iFpL)
GO(__gmpz_com, vFpp)
GO(__gmpz_combit, vFpL)
GO(__gmpz_congruent_2exp_p, iFppL)
GO(__gmpz_congruent_p, iFppp)
GO(__gmpz_congruent_ui_p, iFpLL)
GO(__gmpz_divexact, vFppp)
//GO(__gmpz_divexact_gcd, 
GO(__gmpz_divexact_ui, vFppL)
GO(__gmpz_divisible_2exp_p, iFpL)
GO(__gmpz_divisible_p, iFpp)
GO(__gmpz_divisible_ui_p, iFpL)
GO(__gmpz_dump, vFp)
GO(__gmpz_export, pFppiLiLp)
GO(__gmpz_fac_ui, vFpL)
GO(__gmpz_fdiv_q, vFppp)
GO(__gmpz_fdiv_q_2exp, vFppL)
GO(__gmpz_fdiv_qr, vFpppp)
GO(__gmpz_fdiv_qr_ui, LFpppL)
GO(__gmpz_fdiv_q_ui, LFppL)
GO(__gmpz_fdiv_r, vFppp)
GO(__gmpz_fdiv_r_2exp, vFppL)
GO(__gmpz_fdiv_r_ui, LFppL)
GO(__gmpz_fdiv_ui, LFpL)
GO(__gmpz_fib2_ui, vFppL)
GO(__gmpz_fib_ui, vFpL)
GO(__gmpz_fits_sint_p, iFp)
GO(__gmpz_fits_slong_p, iFp)
GO(__gmpz_fits_sshort_p, iFp)
GO(__gmpz_fits_uint_p, iFp)
GO(__gmpz_fits_ulong_p, iFp)
GO(__gmpz_fits_ushort_p, iFp)
GO(__gmpz_gcd, vFppp)
GO(__gmpz_gcdext, vFppppp)
GO(__gmpz_gcd_ui, LFppL)
GO(__gmpz_get_d, dFp)
GO(__gmpz_get_d_2exp, dFpp)
GO(__gmpz_getlimbn, LFpl)
GO(__gmpz_get_si, lFp)
GO(__gmpz_get_str, pFpip)
GO(__gmpz_get_ui, LFp)
GO(__gmpz_hamdist, LFpp)
GO(__gmpz_import, vFpLiLiLp)
GO(__gmpz_init, vFp)
GO(__gmpz_init2, vFpL)
//GOM(__gmpz_inits, vFEpV)
GO(__gmpz_init_set, vFpp)
GO(__gmpz_init_set_d, vFpd)
GO(__gmpz_init_set_si, vFpl)
GO(__gmpz_init_set_str, iFppi)
GO(__gmpz_init_set_ui, vFpL)
//GO(__gmpz_inp_raw, 
//GO(__gmpz_inp_str, 
//GO(__gmpz_inp_str_nowhite, 
GO(__gmpz_invert, iFppp)
GO(__gmpz_ior, vFppp)
GO(__gmpz_jacobi, iFpp)
GO(__gmpz_kronecker_si, iFpl)
GO(__gmpz_kronecker_ui, iFpL)
GO(__gmpz_lcm, vFppp)
GO(__gmpz_lcm_ui, vFppL)
//GO(__gmpz_legendre, 
GO(__gmpz_limbs_finish, vFpl)
GO(__gmpz_limbs_modify, pFpl)
GO(__gmpz_limbs_read, pFp)
GO(__gmpz_limbs_write, pFpl)
//GO(__gmpz_lucas_mod, 
GO(__gmpz_lucnum2_ui, vFppL)
GO(__gmpz_lucnum_ui, vFpL)
GO(__gmpz_mfac_uiui, vFpLL)
GO(__gmpz_millerrabin, iFpi)
GO(__gmpz_mod, vFppp)
GO(__gmpz_mul, vFppp)
GO(__gmpz_mul_2exp, vFppL)
GO(__gmpz_mul_si, vFppl)
GO(__gmpz_mul_ui, vFppL)
GO(__gmpz_neg, vFpp)
GO(__gmpz_nextprime, vFpp)
//GO(__gmpz_n_pow_ui, 
//GO(__gmpz_oddfac_1, 
//GO(__gmpz_out_raw, 
//GO(__gmpz_out_str, 
GO(__gmpz_perfect_power_p, iFp)
GO(__gmpz_perfect_square_p, iFp)
GO(__gmpz_popcount, LFp)
GO(__gmpz_powm, vFpppp)
GO(__gmpz_powm_sec, vFpppp)
GO(__gmpz_powm_ui, vFppLp)
GO(__gmpz_pow_ui, vFppL)
GO(__gmpz_primorial_ui, vFpL)
GO(__gmpz_probab_prime_p, iFpi)
//GO(__gmpz_prodlimbs, 
GO(__gmpz_random, vFpl)
GO(__gmpz_random2, vFpl)
GO(__gmpz_realloc, pFpl)
GO(__gmpz_realloc2, vFpL)
GO(__gmpz_remove, LFppp)
GO(__gmpz_roinit_n, pFppl)
GO(__gmpz_root, iFppL)
GO(__gmpz_rootrem, vFpppL)
GO(__gmpz_rrandomb, vFppL)
GO(__gmpz_scan0, LFpL)
GO(__gmpz_scan1, LFpL)
GO(__gmpz_set, vFpp)
GO(__gmpz_setbit, vFpL)
GO(__gmpz_set_d, vFpd)
GO(__gmpz_set_f, vFpp)
GO(__gmpz_set_q, vFpp)
GO(__gmpz_set_si, vFpl)
GO(__gmpz_set_str, iFppi)
GO(__gmpz_set_ui, vFpL)
GO(__gmpz_si_kronecker, iFlp)
GO(__gmpz_size, LFp)
GO(__gmpz_sizeinbase, LFpi)
GO(__gmpz_sqrt, vFpp)
GO(__gmpz_sqrtrem, vFppp)
//GO(__gmpz_stronglucas, 
GO(__gmpz_sub, vFppp)
GO(__gmpz_submul, vFppp)
GO(__gmpz_submul_ui, vFppL)
GO(__gmpz_sub_ui, vFppL)
GO(__gmpz_swap, vFpp)
GO(__gmpz_tdiv_q, vFppp)
GO(__gmpz_tdiv_q_2exp, vFppL)
GO(__gmpz_tdiv_qr, vFpppp)
GO(__gmpz_tdiv_qr_ui, LFpppL)
GO(__gmpz_tdiv_q_ui, LFppL)
GO(__gmpz_tdiv_r, vFppp)
GO(__gmpz_tdiv_r_2exp, vFppL)
GO(__gmpz_tdiv_r_ui, LFppL)
GO(__gmpz_tdiv_ui, LFpL)
GO(__gmpz_tstbit, iFpL)
GO(__gmpz_ui_kronecker, iFLp)
GO(__gmpz_ui_pow_ui, vFpLL)
GO(__gmpz_ui_sub, vFpLp)
GO(__gmpz_urandomb, vFppL)
GO(__gmpz_urandomm, vFppp)
GO(__gmpz_xor, vFppp)
