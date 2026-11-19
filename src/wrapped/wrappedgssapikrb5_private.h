// SPDX-License-Identifier: MIT
#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GO(gss_accept_sec_context, uFppppppppppp)
GO(gss_acquire_cred, uFppupippp)
//GO(gss_acquire_cred_from, 
//GO(gss_acquire_cred_impersonate_name, 
//GO(gss_acquire_cred_with_password, 
//GO(gss_add_buffer_set_member, 
GO(gss_add_cred, uFppppiuupppp)
//GO(gss_add_cred_from, 
//GO(gss_add_cred_impersonate_name, 
//GO(gss_add_cred_with_password, 
GO(gss_add_oid_set_member, uFppp)
//GO(gss_authorize_localname, 
GO(gss_canonicalize_name, uFpppp)
//DATA(GSS_C_ATTR_LOCAL_LOGIN_USER, 
//DATA(GSS_C_INQ_SSPI_SESSION_KEY, 
//DATA(GSS_C_MA_AUTH_INIT, 8)
//DATA(GSS_C_MA_AUTH_INIT_ANON, 8)
//DATA(GSS_C_MA_AUTH_INIT_INIT, 8)
//DATA(GSS_C_MA_AUTH_TARG, 8)
//DATA(GSS_C_MA_AUTH_TARG_ANON, 8)
//DATA(GSS_C_MA_AUTH_TARG_INIT, 8)
//DATA(GSS_C_MA_CBINDINGS, 8)
//DATA(GSS_C_MA_COMPRESS, 8)
//DATA(GSS_C_MA_CONF_PROT, 8)
//DATA(GSS_C_MA_CTX_TRANS, 8)
//DATA(GSS_C_MA_DELEG_CRED, 8)
//DATA(GSS_C_MA_DEPRECATED, 8)
//DATA(GSS_C_MA_INTEG_PROT, 8)
//DATA(GSS_C_MA_ITOK_FRAMED, 8)
//DATA(GSS_C_MA_MECH_COMPOSITE, 8)
//DATA(GSS_C_MA_MECH_CONCRETE, 8)
//DATA(GSS_C_MA_MECH_GLUE, 8)
//DATA(GSS_C_MA_MECH_NEGO, 8)
//DATA(GSS_C_MA_MECH_PSEUDO, 8)
//DATA(GSS_C_MA_MIC, 8)
//DATA(GSS_C_MA_NOT_DFLT_MECH, 8)
//DATA(GSS_C_MA_NOT_MECH, 8)
//DATA(GSS_C_MA_OOS_DET, 8)
//DATA(GSS_C_MA_PFS, 8)
//DATA(GSS_C_MA_PROT_READY, 8)
//DATA(GSS_C_MA_REPLAY_DET, 8)
//DATA(GSS_C_MA_WRAP, 8)
//DATA(GSS_C_NT_ANONYMOUS, 8)
//DATA(GSS_C_NT_COMPOSITE_EXPORT, 
//DATA(GSS_C_NT_EXPORT_NAME, 8)
DATA(GSS_C_NT_HOSTBASED_SERVICE, 8)
//DATA(GSS_C_NT_HOSTBASED_SERVICE_X, 8)
//DATA(GSS_C_NT_MACHINE_UID_NAME, 8)
//DATA(GSS_C_NT_STRING_UID_NAME, 8)
DATA(GSS_C_NT_USER_NAME, 8)
GO(gss_compare_name, uFpppp)
//GO(gss_complete_auth_token, 
GO(gss_context_time, uFppp)
//GO(gss_create_empty_buffer_set, 
GO(gss_create_empty_oid_set, uFpp)
//GO(gss_decapsulate_token, 
//GO(gss_delete_name_attribute, 
GO(gss_delete_sec_context, uFppp)
GO(gss_display_mech_attr, uFppppp)
GO(gss_display_name, uFpppp)
//GO(gss_display_name_ext, 
GO(gss_display_status, uFpuippp)
GO(gss_duplicate_name, uFppp)
//GO(gss_encapsulate_token, 
//GO(gss_export_cred, 
GO(gss_export_name, uFppp)
//GO(gss_export_name_composite, 
GO(gss_export_sec_context, uFppp)
GO(gss_get_mic, uFppupp)
//GO(gss_get_mic_iov, 
//GO(gss_get_mic_iov_length, 
//GO(gss_get_name_attribute, 
//GO(gss_import_cred, 
GO(gss_import_name, uFpppp)
GO(gss_import_sec_context, uFppp)
GO(gss_indicate_mechs, uFpp)
GO(gss_indicate_mechs_by_attrs, uFppppp)
GO(gss_init_sec_context, uFpppppuupppppp)
GO(gss_inquire_attrs_for_mech, uFpppp)
GO(gss_inquire_context, uFppppppppp)
GO(gss_inquire_cred, uFpppppp)
GO(gss_inquire_cred_by_mech, uFppppppp)
//GO(gss_inquire_cred_by_oid, 
GO(gss_inquire_mech_for_saslname, uFppp)
GO(gss_inquire_mechs_for_name, uFppp)
//GO(gss_inquire_name, 
GO(gss_inquire_names_for_mech, uFppp)
GO(gss_inquire_saslname_for_mech, uFppppp)
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
GO(gss_oid_to_str, uFppp)
//GO(gss_pname_to_uid, 
GO(gss_process_context_token, uFppp)
GO(gss_pseudo_random, uFppiplp)
//GO(gss_release_any_name_mapping, 
GO(gss_release_buffer, uFpp)
GO(gss_release_buffer_set, uFpp)
GO(gss_release_cred, uFpp)
GO(gss_release_iov_buffer, uFppi)
GO(gss_release_name, uFpp)
GO(gss_release_oid, uFpp)
GO(gss_release_oid_set, uFpp)
GO(gss_seal, uFppiippp)
//GO(gss_set_cred_option, 
//GO(gss_set_name_attribute, 
GO(gss_set_neg_mechs, uFppp)
//GO(gss_set_sec_context_option, 
GO(gss_sign, uFppipp)
//GO(gssspi_mech_invoke, 
//GO(gssspi_set_cred_option, 
GO(gss_store_cred, uFppipuupp)
//GO(gss_store_cred_into, 
GO(gss_str_to_oid, uFppp)
GO(gss_test_oid_set_member, uFpppp)
GO(gss_unseal, uFpppppp)
GO(gss_unwrap, uFpppppp)
//GO(gss_unwrap_aead, 
GO(gss_unwrap_iov, uFpppppi)
//GO(gss_userok, 
GO(gss_verify, uFppppp)
GO(gss_verify_mic, uFppppp)
//GO(gss_verify_mic_iov, 
GO(gss_wrap, uFppiuppp)
//GO(gss_wrap_aead, 
GO(gss_wrap_iov, uFppiuppi)
//GO(gss_wrap_iov_length, 
GO(gss_wrap_size_limit, uFppiuup)
//DATAB(krb5_gss_dbg_client_expcreds, 
//GO(krb5_gss_register_acceptor_identity, 
//GO(krb5_gss_use_kdc_context, 
