#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

GO(gss_accept_sec_context, uFppppppppppp)
GO(gss_acquire_cred, uFppuppppp)
//GO(gss_acquire_cred_from, 
//GO(gss_acquire_cred_impersonate_name, 
//GO(gss_acquire_cred_with_password, 
//GO(gss_add_buffer_set_member, 
//GO(gss_add_cred, 
//GO(gss_add_cred_from, 
//GO(gss_add_cred_impersonate_name, 
//GO(gss_add_cred_with_password, 
GO(gss_add_oid_set_member, uFpup)
//GO(gss_authorize_localname, 
//GO(gss_canonicalize_name, 
//DATA(GSS_C_ATTR_LOCAL_LOGIN_USER, 
//DATA(GSS_C_INQ_SSPI_SESSION_KEY, 
//DATA(GSS_C_MA_AUTH_INIT, 
//DATA(GSS_C_MA_AUTH_INIT_ANON, 
//DATA(GSS_C_MA_AUTH_INIT_INIT, 
//DATA(GSS_C_MA_AUTH_TARG, 
//DATA(GSS_C_MA_AUTH_TARG_ANON, 
//DATA(GSS_C_MA_AUTH_TARG_INIT, 
//DATA(GSS_C_MA_CBINDINGS, 
//DATA(GSS_C_MA_COMPRESS, 
//DATA(GSS_C_MA_CONF_PROT, 
//DATA(GSS_C_MA_CTX_TRANS, 
//DATA(GSS_C_MA_DELEG_CRED, 
//DATA(GSS_C_MA_DEPRECATED, 
//DATA(GSS_C_MA_INTEG_PROT, 
//DATA(GSS_C_MA_ITOK_FRAMED, 
//DATA(GSS_C_MA_MECH_COMPOSITE, 
//DATA(GSS_C_MA_MECH_CONCRETE, 
//DATA(GSS_C_MA_MECH_GLUE, 
//DATA(GSS_C_MA_MECH_NEGO, 
//DATA(GSS_C_MA_MECH_PSEUDO, 
//DATA(GSS_C_MA_MIC, 
//DATA(GSS_C_MA_NOT_DFLT_MECH, 
//DATA(GSS_C_MA_NOT_MECH, 
//DATA(GSS_C_MA_OOS_DET, 
//DATA(GSS_C_MA_PFS, 
//DATA(GSS_C_MA_PROT_READY, 
//DATA(GSS_C_MA_REPLAY_DET, 
//DATA(GSS_C_MA_WRAP, 
//DATA(GSS_C_NT_ANONYMOUS, 
//DATA(GSS_C_NT_COMPOSITE_EXPORT, 
//DATA(GSS_C_NT_EXPORT_NAME, 
DATA(GSS_C_NT_HOSTBASED_SERVICE, sizeof(void*))
//DATA(GSS_C_NT_HOSTBASED_SERVICE_X, 
//DATA(GSS_C_NT_MACHINE_UID_NAME, 
//DATA(GSS_C_NT_STRING_UID_NAME, 
DATA(GSS_C_NT_USER_NAME, sizeof(void*))
//GO(gss_compare_name, 
//GO(gss_complete_auth_token, 
//GO(gss_context_time, 
//GO(gss_create_empty_buffer_set, 
GO(gss_create_empty_oid_set, uFpp)
//GO(gss_decapsulate_token, 
//GO(gss_delete_name_attribute, 
GO(gss_delete_sec_context, uFppp)
//GO(gss_display_mech_attr, 
GO(gss_display_name, uFpppp)
//GO(gss_display_name_ext, 
GO(gss_display_status, uFpuippp)
//GO(gss_duplicate_name, 
//GO(gss_encapsulate_token, 
//GO(gss_export_cred, 
//GO(gss_export_name, 
//GO(gss_export_name_composite, 
//GO(gss_export_sec_context, 
GO(gss_get_mic, uFppupp)
//GO(gss_get_mic_iov, 
//GO(gss_get_mic_iov_length, 
//GO(gss_get_name_attribute, 
//GO(gss_import_cred, 
GO(gss_import_name, uFpppp)
//GO(gss_import_sec_context, 
GO(gss_indicate_mechs, uFpp)
//GO(gss_indicate_mechs_by_attrs, 
GO(gss_init_sec_context, uFpppppuupppppp)
//GO(gss_inquire_attrs_for_mech, 
GO(gss_inquire_context, uFppppppppp)
GO(gss_inquire_cred, uFpupppp)
GO(gss_inquire_cred_by_mech, uFpuupppp)
//GO(gss_inquire_cred_by_oid, 
//GO(gss_inquire_mech_for_saslname, 
//GO(gss_inquire_mechs_for_name, 
//GO(gss_inquire_name, 
//GO(gss_inquire_names_for_mech, 
//GO(gss_inquire_saslname_for_mech, 
GO(gss_inquire_sec_context_by_oid, uFpppp)
//GO(gssint_g_seqstate_init, 
//GO(gss_krb5_ccache_name, 
//GO(gss_krb5_copy_ccache, 
//GO(gss_krb5_export_lucid_sec_context, 
//GO(gsskrb5_extract_authtime_from_sec_context, 
//GO(gsskrb5_extract_authz_data_from_sec_context, 
//GO(gss_krb5_free_lucid_sec_context, 
//GO(gss_krb5_get_tkt_flags, 
//GO(gss_krb5_import_cred, 
//GO(gss_krb5int_make_seal_token_v3, 
//GO(gss_krb5int_unseal_token_v3, 
//DATA(GSS_KRB5_NT_PRINCIPAL_NAME, 
//GO(gss_krb5_set_allowable_enctypes, 
//GO(gss_krb5_set_cred_rcache, 
//GO(gss_localname, 
//GO(gss_map_name_to_any, 
//DATA(gss_mech_iakerb, 
//DATA(gss_mech_krb5, 
//DATA(gss_mech_krb5_old, 
//DATA(gss_mech_krb5_wrong, 
//DATA(gss_mech_set_krb5, 
//DATA(gss_mech_set_krb5_both, 
//DATA(gss_mech_set_krb5_old, 
//DATA(gss_nt_exported_name, 
//DATA(gss_nt_krb5_name, 
//DATA(gss_nt_krb5_principal, 
//DATA(gss_nt_machine_uid_name, 
//DATA(gss_nt_service_name, 
//DATA(gss_nt_service_name_v2, 
//DATA(gss_nt_string_uid_name, 
//DATA(gss_nt_user_name, 
//GO(gss_oid_equal, 
//GO(gss_oid_to_str, 
//GO(gss_pname_to_uid, 
//GO(gss_process_context_token, 
//GO(gss_pseudo_random, 
//GO(gss_release_any_name_mapping, 
GO(gss_release_buffer, uFpp)
GO(gss_release_buffer_set, uFpp)
GO(gss_release_cred, uFpp)
GO(gss_release_iov_buffer, uFppi)
GO(gss_release_name, uFpp)
GO(gss_release_oid, uFpp)
GO(gss_release_oid_set, uFpp)
//GO(gss_seal, 
//GO(gss_set_cred_option, 
//GO(gss_set_name_attribute, 
//GO(gss_set_neg_mechs, 
//GO(gss_set_sec_context_option, 
//GO(gss_sign, 
//GO(gssspi_mech_invoke, 
//GO(gssspi_set_cred_option, 
//GO(gss_store_cred, 
//GO(gss_store_cred_into, 
//GO(gss_str_to_oid, 
GO(gss_test_oid_set_member, uFpupp)
//GO(gss_unseal, 
GO(gss_unwrap, uFpppppp)
//GO(gss_unwrap_aead, 
GO(gss_unwrap_iov, uFpppppi)
//GO(gss_userok, 
//GO(gss_verify, 
GO(gss_verify_mic, uFppppp)
//GO(gss_verify_mic_iov, 
GO(gss_wrap, uFppiuppp)
//GO(gss_wrap_aead, 
GO(gss_wrap_iov, uFppiuppi)
//GO(gss_wrap_iov_length, 
//GO(gss_wrap_size_limit, 
//DATAB(krb5_gss_dbg_client_expcreds, 
//GO(krb5_gss_register_acceptor_identity, 
//GO(krb5_gss_use_kdc_context, 
