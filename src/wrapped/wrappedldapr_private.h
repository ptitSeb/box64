#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

GO(ldap_abandon, iFpi)
GO(ldap_abandon_ext, iFpipp)
//GO(ldap_add, 
GO(ldap_add_ext, iFpppppp)
GO(ldap_add_ext_s, iFppppp)
//GO(ldap_add_result_entry, 
//GO(ldap_add_s, 
//GO(ldap_alloc_ber_with_options, 
//GO(ldap_append_referral, 
//GO(ldap_attributetype2bv, 
//GO(ldap_attributetype2name, 
//GO(ldap_attributetype2str, 
//GO(ldap_attributetype_free, 
GO(ldap_bind, iFpppi)
GO(ldap_bind_s, iFpppi)
//GO(ldap_build_add_req, 
//GO(ldap_build_bind_req, 
//GO(ldap_build_compare_req, 
//GO(ldap_build_delete_req, 
//GO(ldap_build_extended_req, 
//GO(ldap_build_moddn_req, 
//GO(ldap_build_modify_req, 
//GO(ldap_build_search_req, 
//GO(ldap_bv2dn, 
//GO(ldap_bv2dn_x, 
//GO(ldap_bv2escaped_filter_value, 
//GO(ldap_bv2escaped_filter_value_len, 
//GO(ldap_bv2escaped_filter_value_x, 
//GO(ldap_bv2rdn, 
//GO(ldap_bv2rdn_x, 
//GO(ldap_cancel, 
//GO(ldap_cancel_s, 
//GO(ldap_charray2str, 
//GO(ldap_charray_add, 
//GO(ldap_charray_dup, 
//GO(ldap_charray_free, 
//GO(ldap_charray_inlist, 
//GO(ldap_charray_merge, 
//GO(ldap_chase_referrals, 
//GO(ldap_chase_v3referrals, 
//GO(ldap_clear_select_write, 
//GO(ldap_compare, 
GO(ldap_compare_ext, iFppppppp)
GO(ldap_compare_ext_s, iFpppppp)
//GO(ldap_compare_s, 
//GO(ldap_connect_to_host, 
//GO(ldap_connect_to_path, 
//GO(ldap_contentrule2bv, 
//GO(ldap_contentrule2name, 
//GO(ldap_contentrule2str, 
//GO(ldap_contentrule_free, 
GO(ldap_control_create, iFpipip)
GO(ldap_control_dup, pFp)
GO(ldap_control_find, pFppp)
GO(ldap_control_free, vFp)
GO(ldap_controls_dup, pFp)
GO(ldap_controls_free, vFp)
GO(ldap_count_entries, iFpp)
GO(ldap_count_messages, iFpp)
GO(ldap_count_references, iFpp)
GO(ldap_count_values, iFp)
GO(ldap_count_values_len, iFp)
//GO(ldap_create, 
//GO(ldap_create_assertion_control, 
//GO(ldap_create_assertion_control_value, 
//GO(ldap_create_control, 
//GO(ldap_create_deref_control, 
//GO(ldap_create_deref_control_value, 
//GO(ldap_create_page_control, 
//GO(ldap_create_page_control_value, 
//GO(ldap_create_passwordpolicy_control, 
//GO(ldap_create_session_tracking_control, 
//GO(ldap_create_session_tracking_value, 
GO(ldap_create_sort_control, iFppip)
//GO(ldap_create_sort_control_value, 
//GO(ldap_create_sort_keylist, 
GO(ldap_create_vlv_control, iFppp)
//GO(ldap_create_vlv_control_value, 
GO(ldap_dcedn2dn, pFp)
GO(ldap_delete, iFpp)
GO(ldap_delete_ext, iFppppp)
GO(ldap_delete_ext_s, iFpppp)
//GO(ldap_delete_result_entry, 
GO(ldap_delete_s, iFpp)
//GO(ldap_derefresponse_free, 
//GO(ldap_destroy, 
GO(ldap_dn2ad_canonical, pFp)
//GO(ldap_dn2bv, 
//GO(ldap_dn2bv_x, 
GO(ldap_dn2dcedn, pFp)
//GO(ldap_dn2domain, 
GO(ldap_dn2str, iFppu)
GO(ldap_dn2ufn, pFp)
//GO(ldap_dnfree, 
//GO(ldap_dnfree_x, 
//GO(ldap_dn_normalize, 
//GO(ldap_domain2dn, 
//GO(ldap_domain2hostlist, 
//GO(ldap_dump_connection, 
//GO(ldap_dump_requests_and_responses, 
//GO(ldap_dup, 
GO(ldap_err2string, pFi)
GO(ldap_explode_dn, pFpi)
GO(ldap_explode_rdn, pFpi)
GO(ldap_extended_operation, iFpppppp)
GO(ldap_extended_operation_s, iFppppppp)
//GO(ldap_find_control, 
//GO(ldap_find_request_by_msgid, 
GO(ldap_first_attribute, pFppp)
GO(ldap_first_entry, pFpp)
GO(ldap_first_message, pFpp)
GO(ldap_first_reference, pFpp)
//GO(ldap_free_connection, 
//GO(ldap_free_request, 
//GO(ldap_free_select_info, 
//GO(ldap_free_sort_keylist, 
GO(ldap_free_urldesc, vFp)
//GO(ldap_free_urllist, 
GO(ldap_get_attribute_ber, iFppppp)
GO(ldap_get_dn, pFpp)
GO(ldap_get_dn_ber, iFpppp)
//GO(ldap_get_entry_controls, 
//GO(ldap_get_message_ber, 
GO(ldap_get_option, iFpip)
GO(ldap_get_values, pFppp)
GO(ldap_get_values_len, pFppp)
//GO(ldap_gssapi_bind, 
//GO(ldap_gssapi_bind_s, 
//GO(ldap_host_connected_to, 
GO(ldap_init, pFpi)
GO(ldap_init_fd, iFuipp)
GO(ldap_initialize, iFpp)
GO(ldap_install_tls, iFp)
//GO(ldap_int_bisect_delete, 
//GO(ldap_int_bisect_find, 
//GO(ldap_int_bisect_insert, 
//GO(ldap_int_check_async_open, 
//GO(ldap_int_client_controls, 
//GO(ldap_int_connect_cbs, 
//GO(ldap_int_error_init, 
//GO(ldap_int_flush_request, 
//DATA(ldap_int_global_options, 
//DATAB(ldap_int_gmtime_mutex, 
//DATAB(ldap_int_hostname, 
//DATAB(ldap_int_hostname_mutex, 
//DATAB(ldap_int_inet4or6, 
//GO(ldap_int_initialize, 
//GO(ldap_int_initialize_global_options, 
//GO(ldap_int_msgtype2str, 
//GO(ldap_int_open_connection, 
//GO(ldap_int_parse_numericoid, 
//GO(ldap_int_parse_ruleid, 
//GO(ldap_int_poll, 
//GO(ldap_int_put_controls, 
//DATAB(ldap_int_resolv_mutex, 
//GO(ldap_int_sasl_bind, 
//GO(ldap_int_sasl_close, 
//GO(ldap_int_sasl_config, 
//GO(ldap_int_sasl_external, 
//GO(ldap_int_sasl_get_option, 
//GO(ldap_int_sasl_init, 
//GO(ldap_int_sasl_open, 
//GO(ldap_int_sasl_set_option, 
//GO(ldap_int_select, 
//GO(ldap_int_thread_destroy, 
//GO(ldap_int_thread_initialize, 
//GO(ldap_int_thread_pool_shutdown, 
//GO(ldap_int_thread_pool_startup, 
//GO(ldap_int_timeval_dup, 
//GO(ldap_int_tls_config, 
//GO(ldap_int_tls_destroy, 
//DATA(ldap_int_tls_impl, 
//GO(ldap_int_tls_start, 
//GO(ldap_int_utils_init, 
//GO(ldap_is_ldapi_url, 
//GO(ldap_is_ldaps_url, 
GO(ldap_is_ldap_url, iFp)
//GO(ldap_is_read_ready, 
//GO(ldap_is_write_ready, 
//GO(ldap_ld_free, 
//GO(ldap_log_printf, 
//GO(ldap_mark_select_clear, 
//GO(ldap_mark_select_read, 
//GO(ldap_mark_select_write, 
//GO(ldap_matchingrule2bv, 
//GO(ldap_matchingrule2name, 
//GO(ldap_matchingrule2str, 
//GO(ldap_matchingrule_free, 
//GO(ldap_matchingruleuse2bv, 
//GO(ldap_matchingruleuse2name, 
//GO(ldap_matchingruleuse2str, 
//GO(ldap_matchingruleuse_free, 
GO(ldap_memalloc, pFL)
GO(ldap_memcalloc, pFLL)
GO(ldap_memfree, vFp)
GO(ldap_memrealloc, pFpL)
GO(ldap_memvfree, vFp)
//GO(ldap_modify, 
GO(ldap_modify_ext, iFpppppp)
GO(ldap_modify_ext_s, iFppppp)
//GO(ldap_modify_s, 
//GO(ldap_modrdn, 
//GO(ldap_modrdn2, 
//GO(ldap_modrdn2_s, 
//GO(ldap_modrdn_s, 
GO(ldap_mods_free, vFpp)
//GO(ldap_msgdelete, 
GO(ldap_msgfree, iFp)
GO(ldap_msgid, iFp)
GO(ldap_msgtype, iFp)
//GO(ldap_nameform2bv, 
//GO(ldap_nameform2name, 
//GO(ldap_nameform2str, 
//GO(ldap_nameform_free, 
//GO(ldap_new_connection, 
//GO(ldap_new_select_info, 
GO(ldap_next_attribute, pFppp)
GO(ldap_next_entry, pFpp)
GO(ldap_next_message, pFpp)
GO(ldap_next_reference, pFpp)
//GO(ldap_ntlm_bind, 
//GO(ldap_objectclass2bv, 
//GO(ldap_objectclass2name, 
//GO(ldap_objectclass2str, 
//GO(ldap_objectclass_free, 
GO(ldap_open, pFpi)
//GO(ldap_open_defconn, 
//GO(ldap_open_internal_connection, 
//GO(ldap_parse_deref_control, 
//GO(ldap_parse_derefresponse_control, 
GO(ldap_parse_extended_result, iFppppi)
//GO(ldap_parse_intermediate, 
//GO(ldap_parse_ntlm_bind_result, 
//GO(ldap_parse_page_control, 
//GO(ldap_parse_pageresponse_control, 
//GO(ldap_parse_passwd, 
//GO(ldap_parse_passwordpolicy_control, 
GO(ldap_parse_reference, iFppppi)
//GO(ldap_parse_refresh, 
GO(ldap_parse_result, iFpppppppi)
GO(ldap_parse_sasl_bind_result, iFpppi)
//GO(ldap_parse_session_tracking_control, 
GO(ldap_parse_sortresponse_control, iFpppp)
GO(ldap_parse_vlvresponse_control, iFpppppp)
//GO(ldap_parse_whoami, 
//GO(ldap_passwd, 
//GO(ldap_passwd_s, 
//GO(ldap_passwordpolicy_err2txt, 
//GO(ldap_perror, 
//GO(ldap_put_vrFilter, 
//GO(ldap_pvt_bv2scope, 
//GO(ldap_pvt_csnstr, 
//GO(ldap_pvt_ctime, 
//GO(ldap_pvt_discard, 
//GO(ldap_pvt_filter_value_unescape, 
//GO(ldap_pvt_find_wildcard, 
//GO(ldap_pvt_get_controls, 
//GO(ldap_pvt_get_fqdn, 
//GO(ldap_pvt_get_hname, 
//GO(ldap_pvt_gethostbyaddr_a, 
//GO(ldap_pvt_gethostbyname_a, 
//GO(ldap_pvt_gettime, 
//GO(ldap_pvt_hex_unescape, 
//GO(ldap_pvt_put_control, 
//GO(ldap_pvt_put_filter, 
//GO(ldap_pvt_runqueue_find, 
//GO(ldap_pvt_runqueue_insert, 
//GO(ldap_pvt_runqueue_isrunning, 
//GO(ldap_pvt_runqueue_next_sched, 
//GO(ldap_pvt_runqueue_persistent_backload, 
//GO(ldap_pvt_runqueue_remove, 
//GO(ldap_pvt_runqueue_resched, 
//GO(ldap_pvt_runqueue_runtask, 
//GO(ldap_pvt_runqueue_stoptask, 
//GO(ldap_pvt_sasl_generic_install, 
//GO(ldap_pvt_sasl_generic_remove, 
//GO(ldap_pvt_sasl_getmechs, 
//GO(ldap_pvt_sasl_install, 
//GO(ldap_pvt_sasl_mutex_dispose, 
//GO(ldap_pvt_sasl_mutex_lock, 
//GO(ldap_pvt_sasl_mutex_new, 
//GO(ldap_pvt_sasl_mutex_unlock, 
//GO(ldap_pvt_sasl_remove, 
//GO(ldap_pvt_sasl_secprops, 
//GO(ldap_pvt_sasl_secprops_unparse, 
//GO(ldap_pvt_scope2bv, 
//GO(ldap_pvt_scope2str, 
//GO(ldap_pvt_search, 
//GO(ldap_pvt_search_s, 
//DATA(ldap_pvt_sockbuf_io_sasl_generic, 
//GO(ldap_pvt_str2lower, 
//GO(ldap_pvt_str2lowerbv, 
//GO(ldap_pvt_str2scope, 
//GO(ldap_pvt_str2upper, 
//GO(ldap_pvt_str2upperbv, 
//GO(ldap_pvt_strtok, 
//GO(ldap_pvt_thread_cond_broadcast, 
//GO(ldap_pvt_thread_cond_destroy, 
//GO(ldap_pvt_thread_cond_init, 
//GO(ldap_pvt_thread_cond_signal, 
//GO(ldap_pvt_thread_cond_wait, 
//GO(ldap_pvt_thread_create, 
//GO(ldap_pvt_thread_destroy, 
//GO(ldap_pvt_thread_exit, 
//GO(ldap_pvt_thread_get_concurrency, 
//GO(ldap_pvt_thread_initialize, 
//GO(ldap_pvt_thread_join, 
//GO(ldap_pvt_thread_key_create, 
//GO(ldap_pvt_thread_key_destroy, 
//GO(ldap_pvt_thread_key_getdata, 
//GO(ldap_pvt_thread_key_setdata, 
//GO(ldap_pvt_thread_kill, 
//GO(ldap_pvt_thread_mutex_destroy, 
//GO(ldap_pvt_thread_mutex_init, 
//GO(ldap_pvt_thread_mutex_lock, 
//GO(ldap_pvt_thread_mutex_trylock, 
//GO(ldap_pvt_thread_mutex_unlock, 
//GO(ldap_pvt_thread_pool_backload, 
//GO(ldap_pvt_thread_pool_context, 
//GO(ldap_pvt_thread_pool_context_reset, 
//GO(ldap_pvt_thread_pool_destroy, 
//GO(ldap_pvt_thread_pool_getkey, 
//GO(ldap_pvt_thread_pool_idle, 
//GO(ldap_pvt_thread_pool_init, 
//GO(ldap_pvt_thread_pool_maxthreads, 
//GO(ldap_pvt_thread_pool_pause, 
//GO(ldap_pvt_thread_pool_pausecheck, 
//GO(ldap_pvt_thread_pool_pausing, 
//GO(ldap_pvt_thread_pool_purgekey, 
//GO(ldap_pvt_thread_pool_query, 
//GO(ldap_pvt_thread_pool_resume, 
//GO(ldap_pvt_thread_pool_retract, 
//GO(ldap_pvt_thread_pool_setkey, 
//GO(ldap_pvt_thread_pool_submit, 
//GO(ldap_pvt_thread_pool_tid, 
//GO(ldap_pvt_thread_pool_unidle, 
//GO(ldap_pvt_thread_rdwr_destroy, 
//GO(ldap_pvt_thread_rdwr_init, 
//GO(ldap_pvt_thread_rdwr_rlock, 
//GO(ldap_pvt_thread_rdwr_rtrylock, 
//GO(ldap_pvt_thread_rdwr_runlock, 
//GO(ldap_pvt_thread_rdwr_wlock, 
//GO(ldap_pvt_thread_rdwr_wtrylock, 
//GO(ldap_pvt_thread_rdwr_wunlock, 
//GO(ldap_pvt_thread_rmutex_destroy, 
//GO(ldap_pvt_thread_rmutex_init, 
//GO(ldap_pvt_thread_rmutex_lock, 
//GO(ldap_pvt_thread_rmutex_trylock, 
//GO(ldap_pvt_thread_rmutex_unlock, 
//GO(ldap_pvt_thread_self, 
//GO(ldap_pvt_thread_set_concurrency, 
//GO(ldap_pvt_thread_sleep, 
//GO(ldap_pvt_thread_yield, 
//GO(ldap_pvt_tls_accept, 
//GO(ldap_pvt_tls_check_hostname, 
//GO(ldap_pvt_tls_ctx_free, 
//GO(ldap_pvt_tls_destroy, 
//GO(ldap_pvt_tls_get_my_dn, 
//GO(ldap_pvt_tls_get_option, 
//GO(ldap_pvt_tls_get_peer_dn, 
//GO(ldap_pvt_tls_get_strength, 
//GO(ldap_pvt_tls_init, 
//GO(ldap_pvt_tls_init_def_ctx, 
//GO(ldap_pvt_tls_inplace, 
//GO(ldap_pvt_tls_sb_ctx, 
//GO(ldap_pvt_tls_set_option, 
GO(ldap_pvt_url_scheme2proto, iFp)
//GO(ldap_pvt_url_scheme2tls, 
//GO(ldap_pvt_url_scheme_port, 
//GO(ldap_rdn2bv, 
//GO(ldap_rdn2bv_x, 
//GO(ldap_rdn2str, 
//GO(ldap_rdnfree, 
//GO(ldap_rdnfree_x, 
//GO(ldap_refresh, 
//GO(ldap_refresh_s, 
GO(ldap_rename, iFppppippp)
//GO(ldap_rename2, 
//GO(ldap_rename2_s, 
GO(ldap_rename_s, iFppppipp)
GO(ldap_result, iFppipp)
//GO(ldap_result2error, 
//GO(ldap_return_request, 
GO(ldap_sasl_bind, iFppppppp)
GO(ldap_sasl_bind_s, iFppppppp)
//GO(ldap_sasl_interactive_bind, 
GOM(ldap_sasl_interactive_bind_s, iFEpppppupp)
//GO(ldap_scherr2str, 
//GO(ldap_search, 
GO(ldap_search_ext, iFppippipppip)
GO(ldap_search_ext_s, iFppippipppip)
//GO(ldap_search_s, 
//GO(ldap_search_st, 
//GO(ldap_send_initial_request, 
//GO(ldap_send_server_request, 
//GO(ldap_send_unbind, 
//GO(ldap_set_ber_options, 
//GO(ldap_set_nextref_proc, 
GO(ldap_set_option, iFpip)
//GOM(ldap_set_rebind_proc, iFEpBp)
//GO(ldap_set_urllist_proc, 
GO(ldap_simple_bind, iFppp)
GO(ldap_simple_bind_s, iFppp)
//GO(ldap_sort_entries, 
//GO(ldap_sort_strcasecmp, 
//GO(ldap_sort_values, 
GO(ldap_start_tls, iFp)
GO(ldap_start_tls_s, iFppp)
//GO(ldap_str2attributetype, 
//GO(ldap_str2charray, 
//GO(ldap_str2contentrule, 
GO(ldap_str2dn, iFppu)
//GO(ldap_str2matchingrule, 
//GO(ldap_str2matchingruleuse, 
//GO(ldap_str2nameform, 
//GO(ldap_str2objectclass, 
//GO(ldap_str2rdn, 
//GO(ldap_str2structurerule, 
//GO(ldap_str2syntax, 
GO(ldap_strdup, pFp)
//GO(ldap_structurerule2bv, 
//GO(ldap_structurerule2name, 
//GO(ldap_structurerule2str, 
//GO(ldap_structurerule_free, 
//GO(ldap_sync_destroy, 
//GO(ldap_sync_init, 
//GO(ldap_sync_initialize, 
//GO(ldap_sync_init_refresh_and_persist, 
//GO(ldap_sync_init_refresh_only, 
//GO(ldap_sync_poll, 
//GO(ldap_syntax2bv, 
//GO(ldap_syntax2name, 
//GO(ldap_syntax2str, 
//GO(ldap_syntax_free, 
GO(ldap_tls_inplace, iFp)
//GO(ldap_turn, 
//GO(ldap_turn_s, 
//GO(ldap_ucs_to_utf8s, 
GO(ldap_unbind, iFp)
GO(ldap_unbind_ext, iFppp)
GO(ldap_unbind_ext_s, iFppp)
GO(ldap_unbind_s, iFp)
//GO(ldap_url_desc2str, 
//GO(ldap_url_dup, 
//GO(ldap_url_duplist, 
//GO(ldap_url_list2hosts, 
//GO(ldap_url_list2urls, 
GO(ldap_url_parse, iFpp)
//GO(ldap_url_parse_ext, 
//GO(ldap_url_parsehosts, 
//GO(ldap_url_parselist, 
//GO(ldap_url_parselist_ext, 
//GO(ldap_utf8_bytes, 
//GO(ldap_utf8_charlen, 
//GO(ldap_utf8_charlen2, 
//GO(ldap_utf8_chars, 
//GO(ldap_utf8_copy, 
//GO(ldap_utf8_isalnum, 
//GO(ldap_utf8_isalpha, 
//GO(ldap_utf8_isascii, 
//GO(ldap_utf8_isdigit, 
//GO(ldap_utf8_islower, 
//GO(ldap_utf8_isspace, 
//GO(ldap_utf8_isupper, 
//GO(ldap_utf8_isxdigit, 
//GO(ldap_utf8_next, 
//GO(ldap_utf8_offset, 
//GO(ldap_utf8_prev, 
//GO(ldap_utf8_strchr, 
//GO(ldap_utf8_strcspn, 
//GO(ldap_utf8_strpbrk, 
//GO(ldap_utf8_strspn, 
//GO(ldap_utf8_strtok, 
//GO(ldap_value_dup, 
GO(ldap_value_free, vFp)
GO(ldap_value_free_len, vFp)
//GO(ldap_whoami, 
//GO(ldap_whoami_s, 
//GO(ldap_X509dn2bv, 
//GO(ldap_x_mbs_to_utf8s, 
//GO(ldap_x_mb_to_utf8, 
//GO(ldap_x_ucs4_to_utf8, 
//GO(ldap_x_utf8s_to_mbs, 
//GO(ldap_x_utf8s_to_wcs, 
//GO(ldap_x_utf8_to_mb, 
//GO(ldap_x_utf8_to_ucs4, 
//GO(ldap_x_utf8_to_wc, 
//GO(ldap_x_wcs_to_utf8s, 
//GO(ldap_x_wc_to_utf8, 
//GO(ldif_close, 
//GO(ldif_countlines, 
//DATAB(ldif_debug, 
//GO(ldif_fetch_url, 
//GO(ldif_getline, 
//GO(ldif_is_not_printable, 
//GO(ldif_must_b64_encode_register, 
//GO(ldif_must_b64_encode_release, 
//GO(ldif_open, 
//GO(ldif_open_url, 
//GO(ldif_parse_line, 
//GO(ldif_parse_line2, 
//GO(ldif_put, 
//GO(ldif_put_wrap, 
//GO(ldif_read_record, 
//GO(ldif_sput, 
//GO(ldif_sput_wrap, 
