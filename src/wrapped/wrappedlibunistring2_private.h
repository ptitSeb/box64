#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

//GO(libunistring_amemxfrm, 
//GO(libunistring_c_isalnum, 
//GO(libunistring_c_isalpha, 
//GO(libunistring_c_isascii, 
//GO(libunistring_c_isblank, 
//GO(libunistring_c_iscntrl, 
//GO(libunistring_c_isdigit, 
//GO(libunistring_c_isgraph, 
//GO(libunistring_c_islower, 
//GO(libunistring_c_isprint, 
//GO(libunistring_c_ispunct, 
//GO(libunistring_c_isspace, 
//GO(libunistring_c_isupper, 
//GO(libunistring_c_isxdigit, 
//GO(libunistring_c_strcasecmp, 
//GO(libunistring_c_strncasecmp, 
//GO(libunistring_c_tolower, 
//GO(libunistring_c_toupper, 
//GO(libunistring_freea, 
//GO(libunistring_fseterr, 
//GO(libunistring_gl_locale_name, 
//GO(libunistring_gl_locale_name_default, 
//GO(libunistring_gl_locale_name_environ, 
//GO(libunistring_gl_locale_name_posix, 
//GO(libunistring_gl_locale_name_thread, 
//GO(libunistring_glthread_once_multithreaded, 
//GO(libunistring_glthread_once_singlethreaded, 
//GO(libunistring_glthread_recursive_lock_init_multithreaded, 
//GO(libunistring_glthread_rwlock_init_for_glibc, 
//GO(libunistring_gl_unicase_special_lookup, 
//DATAR(libunistring_gl_uninorm_decomp_chars_table, 
//DATAR(libunistring_gl_uninorm_decomp_index_table, 
//GO(libunistring_gl_uninorm_decompose_merge_sort_inplace, 
//GO(libunistring_hard_locale, 
//GO(libunistring_iconveh_close, 
//GO(libunistring_iconveh_open, 
//GO(libunistring_is_basic, 
//DATAR(libunistring_is_basic_table, 
//GO(libunistring_mb_copy, 
//GO(libunistring_mbiter_multi_copy, 
//GO(libunistring_mbiter_multi_next, 
//GO(libunistring_mbiter_multi_reloc, 
//GO(libunistring_mbsnlen, 
//GO(libunistring_mb_width_aux, 
//GO(libunistring_mem_cd_iconveh, 
//GO(libunistring_memcmp2, 
//GO(libunistring_mem_iconveh, 
//GO(libunistring_mem_iconveha, 
//GO(libunistring_mmalloca, 
//GO(libunistring_printf_frexp, 
//GO(libunistring_printf_frexpl, 
//GO(libunistring_rpl_mbrtowc, 
//GO(libunistring_setlocale_null, 
//GO(libunistring_setlocale_null_r, 
//GO(libunistring_str_cd_iconveh, 
//GO(libunistring_str_iconveh, 
//GO(libunistring_str_iconveha, 
//GO(libunistring_u16_casemap, 
//GO(libunistring_u16_is_invariant, 
//GO(libunistring_u16_possible_linebreaks_loop, 
//GO(libunistring_u16_printf_parse, 
//GO(libunistring_u32_casemap, 
//GO(libunistring_u32_is_invariant, 
//GO(libunistring_u32_possible_linebreaks_loop, 
//GO(libunistring_u32_printf_parse, 
//GO(libunistring_u8_casemap, 
//GO(libunistring_u8_is_invariant, 
//GO(libunistring_u8_possible_linebreaks_loop, 
//GO(libunistring_u8_printf_parse, 
//GO(libunistring_u8_width_linebreaks_internal, 
//GO(libunistring_uc_compat_decomposition, 
//GO(libunistring_uc_is_cased, 
//GO(libunistring_uc_is_case_ignorable, 
//GO(libunistring_uc_tocasefold, 
//GO(libunistring_ulc_printf_parse, 
//GO(libunistring_uniconv_register_autodetect, 
//GO(libunistring_unilbrk_is_all_ascii, 
//GO(libunistring_unilbrk_is_utf8_encoding, 
//DATAR(libunistring_unilbrkprop, 
//DATAR(libunistring_unilbrk_table, 
//DATAR(libunistring_uniwbrk_prop_index, 
//DATAR(libunistring_uniwbrk_table, 
//GO(libunistring_u_printf_fetchargs, 
//DATAR(_libunistring_version, 
//GO(libunistring_xmax, 
//GO(libunistring_xsum, 
//GO(libunistring_xsum3, 
//GO(libunistring_xsum4, 
GO(locale_charset, pFv)
//GO(u16_asnprintf, 
//GO(u16_asprintf, 
GO(u16_casecmp, iFpLpLppp)
GO(u16_casecoll, iFpLpLppp)
GO(u16_casefold, pFpLpppp)
GO(u16_casexfrm, pFpLpppp)
GO(u16_casing_prefix_context, UFpL)
//GO(u16_casing_prefixes_context, 
GO(u16_casing_suffix_context, UFpL)
//GO(u16_casing_suffixes_context, 
GO(u16_check, pFpL)
GO(u16_chr, pFpLu)
GO(u16_cmp, iFppL)
GO(u16_cmp2, iFpLpL)
GO(u16_conv_from_encoding, pFpupLppp)
GO(u16_conv_to_encoding, pFpupLppp)
GO(u16_cpy, pFppL)
GO(u16_cpy_alloc, pFpL)
//GO(u16_ct_casefold, 
//GO(u16_ct_tolower, 
//GO(u16_ct_totitle, 
//GO(u16_ct_toupper, 
GO(u16_endswith, iFpp)
GO(u16_grapheme_breaks, vFpLp)
GO(u16_grapheme_next, pFpp)
GO(u16_grapheme_prev, pFpp)
GO(u16_is_cased, iFpLpp)
GO(u16_is_casefolded, iFpLpp)
GO(u16_is_lowercase, iFpLpp)
GO(u16_is_titlecase, iFpLpp)
GO(u16_is_uppercase, iFpLpp)
GO(u16_mblen, iFpL)
GO(u16_mbsnlen, LFpL)
GO(u16_mbtouc, iFppL)
//GO(u16_mbtouc_aux, 
GO(u16_mbtoucr, iFppL)
GO(u16_mbtouc_unsafe, iFppL)
//GO(u16_mbtouc_unsafe_aux, 
GO(u16_move, pFppL)
GO(u16_next, pFpp)
GO(u16_normalize, pFppLpp)
GO(u16_normcmp, iFpLpLpp)
GO(u16_normcoll, iFpLpLpp)
GO(u16_normxfrm, pFpLppp)
GO(u16_possible_linebreaks, vFpLpp)
GO(u16_possible_linebreaks_v2, vFpLpp)
GO(u16_prev, pFppp)
GO(u16_set, pFpuL)
//GO(u16_snprintf, 
//GO(u16_sprintf, 
GO(u16_startswith, iFpp)
GO(u16_stpcpy, pFpp)
GO(u16_stpncpy, pFppL)
GO(u16_strcat, pFpp)
GO(u16_strchr, pFpu)
GO(u16_strcmp, iFpp)
GO(u16_strcoll, iFpp)
GO(u16_strconv_from_encoding, pFppu)
GO(u16_strconv_from_locale, pFp)
GO(u16_strconv_to_encoding, pFppu)
GO(u16_strconv_to_locale, pFp)
GO(u16_strcpy, pFpp)
GO(u16_strcspn, LFpp)
GO(u16_strdup, pFp)
GO(u16_strlen, LFp)
GO(u16_strmblen, iFp)
GO(u16_strmbtouc, iFpp)
GO(u16_strncat, pFppL)
GO(u16_strncmp, iFppL)
GO(u16_strncpy, pFppL)
GO(u16_strnlen, LFpL)
GO(u16_strpbrk, pFpp)
GO(u16_strrchr, pFpu)
GO(u16_strspn, LFpp)
GO(u16_strstr, pFpp)
GO(u16_strtok, pFppp)
GO(u16_strwidth, iFpp)
GO(u16_tolower, pFpLpppp)
GO(u16_totitle, pFpLpppp)
GO(u16_to_u32, pFpLpp)
GO(u16_to_u8, pFpLpp)
GO(u16_toupper, pFpLpppp)
//GO(u16_u16_asnprintf, 
//GO(u16_u16_asprintf, 
//GO(u16_u16_snprintf, 
//GO(u16_u16_sprintf, 
//GO(u16_u16_vasnprintf, 
//GO(u16_u16_vasprintf, 
//GO(u16_u16_vsnprintf, 
//GO(u16_u16_vsprintf, 
GO(u16_uctomb, iFpul)
GO(u16_uctomb_aux, iFpul)
//GO(u16_vasnprintf, 
//GO(u16_vasprintf, 
//GO(u16_vsnprintf, 
//GO(u16_vsprintf, 
GO(u16_width, iFpLp)
GO(u16_width_linebreaks, iFpLiiippp)
GO(u16_width_linebreaks_v2, iFpLiiippp)
GO(u16_wordbreaks, vFpLp)
//GO(u32_asnprintf, 
//GO(u32_asprintf, 
GO(u32_casecmp, iFpLpLppp)
GO(u32_casecoll, iFpLpLppp)
GO(u32_casefold, pFpLpppp)
GO(u32_casexfrm, pFpLpppp)
GO(u32_casing_prefix_context, UFpL)
//GO(u32_casing_prefixes_context, 
GO(u32_casing_suffix_context, UFpL)
//GO(u32_casing_suffixes_context, 
GO(u32_check, pFpL)
GO(u32_chr, pFpLu)
GO(u32_cmp, iFppL)
GO(u32_cmp2, iFpLpL)
GO(u32_conv_from_encoding, pFpupLppp)
GO(u32_conv_to_encoding, pFpupLppp)
GO(u32_cpy, pFppL)
GO(u32_cpy_alloc, pFpL)
//GO(u32_ct_casefold, 
//GO(u32_ct_tolower, 
//GO(u32_ct_totitle, 
//GO(u32_ct_toupper, 
GO(u32_endswith, iFpp)
GO(u32_grapheme_breaks, vFpLp)
GO(u32_grapheme_next, pFpp)
GO(u32_grapheme_prev, pFpp)
GO(u32_is_cased, iFpLpp)
GO(u32_is_casefolded, iFpLpp)
GO(u32_is_lowercase, iFpLpp)
GO(u32_is_titlecase, iFpLpp)
GO(u32_is_uppercase, iFpLpp)
GO(u32_mblen, iFpL)
GO(u32_mbsnlen, LFpL)
GO(u32_mbtouc, iFppL)
GO(u32_mbtoucr, iFppL)
GO(u32_mbtouc_unsafe, iFppL)
GO(u32_move, pFppL)
GO(u32_next, pFpp)
GO(u32_normalize, pFppLpp)
GO(u32_normcmp, iFpLpLpp)
GO(u32_normcoll, iFpLpLpp)
GO(u32_normxfrm, pFpLppp)
GO(u32_possible_linebreaks, vFpLpp)
GO(u32_possible_linebreaks_v2, vFpLpp)
GO(u32_prev, pFppp)
GO(u32_set, pFpuL)
//GO(u32_snprintf, 
//GO(u32_sprintf, 
GO(u32_startswith, iFpp)
GO(u32_stpcpy, pFpp)
GO(u32_stpncpy, pFppL)
GO(u32_strcat, pFpp)
GO(u32_strchr, pFpu)
GO(u32_strcmp, iFpp)
GO(u32_strcoll, iFpp)
GO(u32_strconv_from_encoding, pFppu)
GO(u32_strconv_from_locale, pFp)
GO(u32_strconv_to_encoding, pFppu)
GO(u32_strconv_to_locale, pFp)
GO(u32_strcpy, pFpp)
GO(u32_strcspn, LFpp)
GO(u32_strdup, pFp)
GO(u32_strlen, LFp)
GO(u32_strmblen, iFp)
GO(u32_strmbtouc, iFpp)
GO(u32_strncat, pFppL)
GO(u32_strncmp, iFppL)
GO(u32_strncpy, pFppL)
GO(u32_strnlen, LFpL)
GO(u32_strpbrk, pFpp)
GO(u32_strrchr, pFpu)
GO(u32_strspn, LFpp)
GO(u32_strstr, pFpp)
GO(u32_strtok, pFppp)
GO(u32_strwidth, iFpp)
GO(u32_tolower, pFpLpppp)
GO(u32_totitle, pFpLpppp)
GO(u32_to_u16, pFpLpp)
GO(u32_to_u8, pFpLpp)
GO(u32_toupper, pFpLpppp)
//GO(u32_u32_asnprintf, 
//GO(u32_u32_asprintf, 
//GO(u32_u32_snprintf, 
//GO(u32_u32_sprintf, 
//GO(u32_u32_vasnprintf, 
//GO(u32_u32_vasprintf, 
//GO(u32_u32_vsnprintf, 
//GO(u32_u32_vsprintf, 
GO(u32_uctomb, iFpul)
//GO(u32_vasnprintf, 
//GO(u32_vasprintf, 
//GO(u32_vsnprintf, 
//GO(u32_vsprintf, 
GO(u32_width, iFpLp)
GO(u32_width_linebreaks, iFpLiiippp)
GO(u32_width_linebreaks_v2, iFpLiiippp)
GO(u32_wordbreaks, vFpLp)
//GO(u8_asnprintf, 
//GO(u8_asprintf, 
GO(u8_casecmp, iFpLpLppp)
GO(u8_casecoll, iFpLpLppp)
GO(u8_casefold, pFpLpppp)
GO(u8_casexfrm, pFpLpppp)
GO(u8_casing_prefix_context, UFpL)
//GO(u8_casing_prefixes_context, 
GO(u8_casing_suffix_context, UFpL)
//GO(u8_casing_suffixes_context, 
GO(u8_check, pFpL)
GO(u8_chr, pFpLu)
GO(u8_cmp, iFppL)
GO(u8_cmp2, iFpLpL)
GO(u8_conv_from_encoding, pFpupLppp)
GO(u8_conv_to_encoding, pFpupLppp)
GO(u8_cpy, pFppL)
GO(u8_cpy_alloc, pFpL)
//GO(u8_ct_casefold, 
//GO(u8_ct_tolower, 
//GO(u8_ct_totitle, 
//GO(u8_ct_toupper, 
GO(u8_endswith, iFpp)
GO(u8_grapheme_breaks, vFpLp)
GO(u8_grapheme_next, pFpp)
GO(u8_grapheme_prev, pFpp)
GO(u8_is_cased, iFpLpp)
GO(u8_is_casefolded, iFpLpp)
GO(u8_is_lowercase, iFpLpp)
GO(u8_is_titlecase, iFpLpp)
GO(u8_is_uppercase, iFpLpp)
GO(u8_mblen, iFpL)
GO(u8_mbsnlen, LFpL)
GO(u8_mbtouc, iFppL)
//GO(u8_mbtouc_aux, 
GO(u8_mbtoucr, iFppL)
GO(u8_mbtouc_unsafe, iFppL)
//GO(u8_mbtouc_unsafe_aux, 
GO(u8_move, pFppL)
GO(u8_next, pFpp)
GO(u8_normalize, pFppLpp)
GO(u8_normcmp, iFpLpLpp)
GO(u8_normcoll, iFpLpLpp)
GO(u8_normxfrm, pFpLppp)
GO(u8_possible_linebreaks, vFpLpp)
GO(u8_possible_linebreaks_v2, vFpLpp)
GO(u8_prev, pFppp)
GO(u8_set, pFpuL)
//GO(u8_snprintf, 
//GO(u8_sprintf, 
GO(u8_startswith, iFpp)
GO(u8_stpcpy, pFpp)
GO(u8_stpncpy, pFppL)
GO(u8_strcat, pFpp)
GO(u8_strchr, pFpu)
GO(u8_strcmp, iFpp)
GO(u8_strcoll, iFpp)
GO(u8_strconv_from_encoding, pFppu)
GO(u8_strconv_from_locale, pFp)
GO(u8_strconv_to_encoding, pFppu)
GO(u8_strconv_to_locale, pFp)
GO(u8_strcpy, pFpp)
GO(u8_strcspn, LFpp)
GO(u8_strdup, pFp)
GO(u8_strlen, LFp)
GO(u8_strmblen, iFp)
GO(u8_strmbtouc, iFpp)
GO(u8_strncat, pFppL)
GO(u8_strncmp, iFppL)
GO(u8_strncpy, pFppL)
GO(u8_strnlen, LFpL)
GO(u8_strpbrk, pFpp)
GO(u8_strrchr, pFpu)
GO(u8_strspn, LFpp)
GO(u8_strstr, pFpp)
GO(u8_strtok, pFppp)
GO(u8_strwidth, iFpp)
GO(u8_tolower, pFpLpppp)
GO(u8_totitle, pFpLpppp)
GO(u8_to_u16, pFpLpp)
GO(u8_to_u32, pFpLpp)
GO(u8_toupper, pFpLpppp)
//GO(u8_u8_asnprintf, 
//GO(u8_u8_asprintf, 
//GO(u8_u8_snprintf, 
//GO(u8_u8_sprintf, 
//GO(u8_u8_vasnprintf, 
//GO(u8_u8_vasprintf, 
//GO(u8_u8_vsnprintf, 
//GO(u8_u8_vsprintf, 
GO(u8_uctomb, iFpul)
GO(u8_uctomb_aux, iFpul)
//GO(u8_vasnprintf, 
//GO(u8_vasprintf, 
//GO(u8_vsnprintf, 
//GO(u8_vsprintf, 
GO(u8_width, iFpLp)
GO(u8_width_linebreaks, iFpLiiippp)
GO(u8_width_linebreaks_v2, iFpLiiippp)
GO(u8_wordbreaks, vFpLp)
GO(uc_all_blocks, vFpp)
GO(uc_all_scripts, vFpp)
GO(uc_bidi_category, iFu)
GO(uc_bidi_category_byname, iFp)
GO(uc_bidi_category_name, pFi)
GO(uc_bidi_class, iFu)
GO(uc_bidi_class_byname, iFp)
GO(uc_bidi_class_long_name, pFi)
GO(uc_bidi_class_name, pFi)
GO(uc_block, pFu)
GO(uc_canonical_decomposition, iFup)
DATA(UC_CATEGORY_C, 16) // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Cc, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Cf, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Cn, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Co, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Cs, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_L, 16) // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_LC, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Ll, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Lm, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Lo, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Lt, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Lu, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_M, 16) // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Mc, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Me, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Mn, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_N, 16) // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Nd, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Nl, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_No, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(_UC_CATEGORY_NONE, 16) // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_P, 16) // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Pc, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Pd, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Pe, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Pf, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Pi, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Po, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Ps, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_S, 16) // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Sc, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Sk, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Sm, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_So, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Z, 16) // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Zl, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Zp, 16)    // this is a uc_general_category_t wich might contain callback functions
DATA(UC_CATEGORY_Zs, 16)    // this is a uc_general_category_t wich might contain callback functions
GO(uc_c_ident_category, iFu)
GO(uc_combining_class, iFu)
GO(uc_combining_class_byname, iFp)
GO(uc_combining_class_long_name, pFi)
GO(uc_combining_class_name, pFi)
GO(uc_composition, uFuu)
GO(uc_decimal_value, iFu)
GO(uc_decomposition, iFupp)
GO(uc_digit_value, iFu)
GO(uc_general_category, HFu)
GO(uc_general_category_and, HFHH)
GO(uc_general_category_and_not, HFHH)
GO(uc_general_category_byname, HFp)
//GO(uc_general_category_long_name, 
//GO(uc_general_category_name, 
GO(uc_general_category_or, HFHH)
GO(uc_grapheme_breaks, vFpLp)
GO(uc_graphemeclusterbreak_property, iFu)
GO(uc_is_alnum, iFu)
GO(uc_is_alpha, iFu)
GO(uc_is_bidi_category, iFui)
GO(uc_is_bidi_class, iFui)
GO(uc_is_blank, iFu)
GO(uc_is_block, iFup)
GO(uc_is_cntrl, iFu)
GO(uc_is_c_whitespace, iFu)
GO(uc_is_digit, iFu)
GO(uc_is_general_category, iFuH)
GO(uc_is_general_category_withtable, iFuu)
GO(uc_is_graph, iFu)
GO(uc_is_grapheme_break, iFuu)
GO(uc_is_java_whitespace, iFu)
GO(uc_is_lower, iFu)
GO(uc_is_print, iFu)
//GOM(uc_is_property, iFEup)
GO(uc_is_property_alphabetic, iFu)
GO(uc_is_property_ascii_hex_digit, iFu)
GO(uc_is_property_bidi_arabic_digit, iFu)
GO(uc_is_property_bidi_arabic_right_to_left, iFu)
GO(uc_is_property_bidi_block_separator, iFu)
GO(uc_is_property_bidi_boundary_neutral, iFu)
GO(uc_is_property_bidi_common_separator, iFu)
GO(uc_is_property_bidi_control, iFu)
GO(uc_is_property_bidi_embedding_or_override, iFu)
GO(uc_is_property_bidi_eur_num_separator, iFu)
GO(uc_is_property_bidi_eur_num_terminator, iFu)
GO(uc_is_property_bidi_european_digit, iFu)
GO(uc_is_property_bidi_hebrew_right_to_left, iFu)
GO(uc_is_property_bidi_left_to_right, iFu)
GO(uc_is_property_bidi_non_spacing_mark, iFu)
GO(uc_is_property_bidi_other_neutral, iFu)
GO(uc_is_property_bidi_pdf, iFu)
GO(uc_is_property_bidi_segment_separator, iFu)
GO(uc_is_property_bidi_whitespace, iFu)
GO(uc_is_property_cased, iFu)
GO(uc_is_property_case_ignorable, iFu)
GO(uc_is_property_changes_when_casefolded, iFu)
GO(uc_is_property_changes_when_casemapped, iFu)
GO(uc_is_property_changes_when_lowercased, iFu)
GO(uc_is_property_changes_when_titlecased, iFu)
GO(uc_is_property_changes_when_uppercased, iFu)
GO(uc_is_property_combining, iFu)
GO(uc_is_property_composite, iFu)
GO(uc_is_property_currency_symbol, iFu)
GO(uc_is_property_dash, iFu)
GO(uc_is_property_decimal_digit, iFu)
GO(uc_is_property_default_ignorable_code_point, iFu)
GO(uc_is_property_deprecated, iFu)
GO(uc_is_property_diacritic, iFu)
GO(uc_is_property_emoji, iFu)
GO(uc_is_property_emoji_component, iFu)
GO(uc_is_property_emoji_modifier, iFu)
GO(uc_is_property_emoji_modifier_base, iFu)
GO(uc_is_property_emoji_presentation, iFu)
GO(uc_is_property_extended_pictographic, iFu)
GO(uc_is_property_extender, iFu)
GO(uc_is_property_format_control, iFu)
GO(uc_is_property_grapheme_base, iFu)
GO(uc_is_property_grapheme_extend, iFu)
GO(uc_is_property_grapheme_link, iFu)
GO(uc_is_property_hex_digit, iFu)
GO(uc_is_property_hyphen, iFu)
GO(uc_is_property_id_continue, iFu)
GO(uc_is_property_ideographic, iFu)
GO(uc_is_property_ids_binary_operator, iFu)
GO(uc_is_property_id_start, iFu)
GO(uc_is_property_ids_trinary_operator, iFu)
GO(uc_is_property_ignorable_control, iFu)
GO(uc_is_property_iso_control, iFu)
GO(uc_is_property_join_control, iFu)
GO(uc_is_property_left_of_pair, iFu)
GO(uc_is_property_line_separator, iFu)
GO(uc_is_property_logical_order_exception, iFu)
GO(uc_is_property_lowercase, iFu)
GO(uc_is_property_math, iFu)
GO(uc_is_property_non_break, iFu)
GO(uc_is_property_not_a_character, iFu)
GO(uc_is_property_numeric, iFu)
GO(uc_is_property_other_alphabetic, iFu)
GO(uc_is_property_other_default_ignorable_code_point, iFu)
GO(uc_is_property_other_grapheme_extend, iFu)
GO(uc_is_property_other_id_continue, iFu)
GO(uc_is_property_other_id_start, iFu)
GO(uc_is_property_other_lowercase, iFu)
GO(uc_is_property_other_math, iFu)
GO(uc_is_property_other_uppercase, iFu)
GO(uc_is_property_paired_punctuation, iFu)
GO(uc_is_property_paragraph_separator, iFu)
GO(uc_is_property_pattern_syntax, iFu)
GO(uc_is_property_pattern_white_space, iFu)
GO(uc_is_property_private_use, iFu)
GO(uc_is_property_punctuation, iFu)
GO(uc_is_property_quotation_mark, iFu)
GO(uc_is_property_radical, iFu)
GO(uc_is_property_regional_indicator, iFu)
GO(uc_is_property_sentence_terminal, iFu)
GO(uc_is_property_soft_dotted, iFu)
GO(uc_is_property_space, iFu)
GO(uc_is_property_terminal_punctuation, iFu)
GO(uc_is_property_titlecase, iFu)
GO(uc_is_property_unassigned_code_value, iFu)
GO(uc_is_property_unified_ideograph, iFu)
GO(uc_is_property_uppercase, iFu)
GO(uc_is_property_variation_selector, iFu)
GO(uc_is_property_white_space, iFu)
GO(uc_is_property_xid_continue, iFu)
GO(uc_is_property_xid_start, iFu)
GO(uc_is_property_zero_width, iFu)
GO(uc_is_punct, iFu)
GO(uc_is_script, iFup)
GO(uc_is_space, iFu)
GO(uc_is_upper, iFu)
GO(uc_is_xdigit, iFu)
GO(uc_java_ident_category, iFu)
GO(uc_joining_group, iFu)
GO(uc_joining_group_byname, iFp)
GO(uc_joining_group_name, pFi)
GO(uc_joining_type, iFu)
GO(uc_joining_type_byname, iFp)
GO(uc_joining_type_long_name, pFi)
GO(uc_joining_type_name, pFi)
GO(uc_locale_language, pFv)
GO(uc_mirror_char, iFup)
GO(uc_numeric_value, UFu)
DATA(UC_PROPERTY_ALPHABETIC, 8) //this is a uc_property_t which is a callback function
DATA(UC_PROPERTY_ASCII_HEX_DIGIT, 8)    //this is a uc_property_t which is a callback function
DATA(UC_PROPERTY_BIDI_ARABIC_DIGIT, 8)  //this is a uc_property_t which is a callback function
DATA(UC_PROPERTY_BIDI_ARABIC_RIGHT_TO_LEFT, 8)  //this is a uc_property_t which is a callback function
DATA(UC_PROPERTY_BIDI_BLOCK_SEPARATOR, 8)   //this is a uc_property_t which is a callback function
DATA(UC_PROPERTY_BIDI_BOUNDARY_NEUTRAL, 8)  //this is a uc_property_t which is a callback function
DATA(UC_PROPERTY_BIDI_COMMON_SEPARATOR, 8)  //this is a uc_property_t which is a callback function
DATA(UC_PROPERTY_BIDI_CONTROL, 8)   //this is a uc_property_t which is a callback function
DATA(UC_PROPERTY_BIDI_EMBEDDING_OR_OVERRIDE, 8) //this is a uc_property_t which is a callback function
DATA(UC_PROPERTY_BIDI_EUR_NUM_SEPARATOR, 8) //this is a uc_property_t which is a callback function
DATA(UC_PROPERTY_BIDI_EUR_NUM_TERMINATOR, 8)    //this is a uc_property_t which is a callback function
DATA(UC_PROPERTY_BIDI_EUROPEAN_DIGIT, 8)    //this is a uc_property_t which is a callback function
DATA(UC_PROPERTY_BIDI_HEBREW_RIGHT_TO_LEFT, 8)  //this is a uc_property_t which is a callback function
DATA(UC_PROPERTY_BIDI_LEFT_TO_RIGHT, 8) //this is a uc_property_t which is a callback function
DATA(UC_PROPERTY_BIDI_NON_SPACING_MARK, 8)  //this is a uc_property_t which is a callback function
DATA(UC_PROPERTY_BIDI_OTHER_NEUTRAL, 8) //this is a uc_property_t which is a callback function
DATA(UC_PROPERTY_BIDI_PDF, 8)   //this is a uc_property_t which is a callback function
DATA(UC_PROPERTY_BIDI_SEGMENT_SEPARATOR, 8) //this is a uc_property_t which is a callback function
DATA(UC_PROPERTY_BIDI_WHITESPACE, 8)    //this is a uc_property_t which is a callback function
GO(uc_property_byname, UFp)
DATA(UC_PROPERTY_CASED, 8)  // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_CASE_IGNORABLE, 8) // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_CHANGES_WHEN_CASEFOLDED, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_CHANGES_WHEN_CASEMAPPED, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_CHANGES_WHEN_LOWERCASED, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_CHANGES_WHEN_TITLECASED, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_CHANGES_WHEN_UPPERCASED, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_COMBINING, 8)  // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_COMPOSITE, 8)  // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_CURRENCY_SYMBOL, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_DASH, 8)   // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_DECIMAL_DIGIT, 8)  // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_DEFAULT_IGNORABLE_CODE_POINT, 8)   // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_DEPRECATED, 8) // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_DIACRITIC, 8)  // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_EMOJI, 8)  // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_EMOJI_COMPONENT, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_EMOJI_MODIFIER, 8) // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_EMOJI_MODIFIER_BASE, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_EMOJI_PRESENTATION, 8) // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_EXTENDED_PICTOGRAPHIC, 8)  // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_EXTENDER, 8)   // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_FORMAT_CONTROL, 8) // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_GRAPHEME_BASE, 8)  // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_GRAPHEME_EXTEND, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_GRAPHEME_LINK, 8)  // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_HEX_DIGIT, 8)  // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_HYPHEN, 8) // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_ID_CONTINUE, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_IDEOGRAPHIC, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_IDS_BINARY_OPERATOR, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_ID_START, 8)   // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_IDS_TRINARY_OPERATOR, 8)   // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_IGNORABLE_CONTROL, 8)  // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_ISO_CONTROL, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_JOIN_CONTROL, 8)   // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_LEFT_OF_PAIR, 8)   // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_LINE_SEPARATOR, 8) // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_LOGICAL_ORDER_EXCEPTION, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_LOWERCASE, 8)  // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_MATH, 8)   // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_NON_BREAK, 8)  // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_NOT_A_CHARACTER, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_NUMERIC, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_OTHER_ALPHABETIC, 8)   // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_OTHER_DEFAULT_IGNORABLE_CODE_POINT, 8) // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_OTHER_GRAPHEME_EXTEND, 8)  // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_OTHER_ID_CONTINUE, 8)  // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_OTHER_ID_START, 8) // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_OTHER_LOWERCASE, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_OTHER_MATH, 8) // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_OTHER_UPPERCASE, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_PAIRED_PUNCTUATION, 8) // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_PARAGRAPH_SEPARATOR, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_PATTERN_SYNTAX, 8) // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_PATTERN_WHITE_SPACE, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_PRIVATE_USE, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_PUNCTUATION, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_QUOTATION_MARK, 8) // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_RADICAL, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_REGIONAL_INDICATOR, 8) // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_SENTENCE_TERMINAL, 8)  // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_SOFT_DOTTED, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_SPACE, 8)  // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_TERMINAL_PUNCTUATION, 8)   // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_TITLECASE, 8)  // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_UNASSIGNED_CODE_VALUE, 8)  // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_UNIFIED_IDEOGRAPH, 8)  // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_UPPERCASE, 8)  // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_VARIATION_SELECTOR, 8) // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_WHITE_SPACE, 8)    // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_XID_CONTINUE, 8)   // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_XID_START, 8)  // this is a uc_property_t wich is a callback
DATA(UC_PROPERTY_ZERO_WIDTH, 8) // this is a uc_property_t wich is a callback
GO(uc_script, pFu)
GO(uc_script_byname, pFp)
GO(uc_tolower, uFu)
GO(uc_totitle, uFu)
GO(uc_toupper, uFu)
GO(uc_width, iFup)
GO(uc_wordbreak_property, iFu)
//GO(ulc_asnprintf, 
//GO(ulc_asprintf, 
GO(ulc_casecmp, iFpLpLppp)
GO(ulc_casecoll, iFpLpLppp)
GO(ulc_casexfrm, pFpLpppp)
//GO(ulc_fprintf, 
GO(ulc_grapheme_breaks, vFpLp)
GO(ulc_possible_linebreaks, vFpLpp)
GO(ulc_possible_linebreaks_v2, vFpLpp)
//GO(ulc_snprintf, 
//GO(ulc_sprintf, 
//GO(ulc_vasnprintf, 
//GO(ulc_vasprintf, 
//GO(ulc_vfprintf, 
//GO(ulc_vsnprintf, 
//GO(ulc_vsprintf, 
GO(ulc_width_linebreaks, iFpLiiippp)
GO(ulc_width_linebreaks_v2, iFpLiiippp)
GO(ulc_wordbreaks, vFpLp)
//DATAR(unicase_empty_prefix_context, 
//DATAR(unicase_empty_suffix_context, 
//GO(unicode_character_name, 
//GO(unicode_name_character, 
GO(uninorm_decomposing_form, pFp)
//GOM(uninorm_filter_create, pFEppp)
GO(uninorm_filter_flush, iFp)
GO(uninorm_filter_free, iFp)
GO(uninorm_filter_write, iFpu)
DATA(uninorm_nfc, 32)   //unicode_normalization_form
DATA(uninorm_nfd, 32)   //unicode_normalization_form
DATA(uninorm_nfkc, 32)  //unicode_normalization_form
DATA(uninorm_nfkd, 32)  //unicode_normalization_form
