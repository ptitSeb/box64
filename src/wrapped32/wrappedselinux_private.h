#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

//GO(avc_add_callback, 
//GO(avc_audit, 
//GO(avc_av_stats, 
//GO(avc_cache_stats, 
//GO(avc_cleanup, 
//GO(avc_compute_create, 
//GO(avc_compute_member, 
//GO(avc_context_to_sid, 
//GO(avc_context_to_sid_raw, 
//GO(avc_destroy, 
//GO(avc_get_initial_sid, 
//GO(avc_has_perm, 
//GO(avc_has_perm_noaudit, 
//GO(avc_init, 
//GO(avc_netlink_acquire_fd, 
//GO(avc_netlink_check_nb, 
//GO(avc_netlink_close, 
//GO(avc_netlink_loop, 
//GO(avc_netlink_open, 
//GO(avc_netlink_release_fd, 
//GO(avc_open, 
//GO(avc_reset, 
//GO(avc_sid_stats, 
//GO(avc_sid_to_context, 
//GO(avc_sid_to_context_raw, 
GO(checkPasswdAccess, iFu)
//GO(context_free, 
//GO(context_new, 
//GO(context_range_get, 
//GO(context_range_set, 
//GO(context_role_get, 
//GO(context_role_set, 
//GO(context_str, 
//GO(context_type_get, 
//GO(context_type_set, 
//GO(context_user_get, 
//GO(context_user_set, 
//DATAB(dir_xattr_list, 
GO(fgetfilecon, iFibp_)
GO(fgetfilecon_raw, iFibp_)
GO(fini_selinuxmnt, vFv)
GO(freecon, vFp)
GO(freeconary, vFbp_)
GO(fsetfilecon, iFip)
GO(fsetfilecon_raw, iFip)
GO(getcon, iFbp_)
GO(getcon_raw, iFbp_)
//GO(get_default_context, 
//GO(get_default_context_with_level, 
//GO(get_default_context_with_role, 
//GO(get_default_context_with_rolelevel, 
//GO(get_default_type, 
GO(getexeccon, iFbp_)
GO(getexeccon_raw, iFbp_)
GO(getfilecon, iFpbp_)
GO(getfilecon_raw, iFpbp_)
GO(getfscreatecon, iFbp_)
GO(getfscreatecon_raw, iFbp_)
GO(getkeycreatecon, iFbp_)
GO(getkeycreatecon_raw, iFbp_)
//GO(get_ordered_context_list, 
//GO(get_ordered_context_list_with_level, 
GO(getpeercon, iFibp_)
GO(getpeercon_raw, iFibp_)
GO(getpidcon, iFibp_)
GO(getpidcon_raw, iFibp_)
GO(getprevcon, iFbp_)
GO(getprevcon_raw, iFbp_)
GO(getseuser, iFppbp_bp_)
GO(getseuserbyname, iFpbp_bp_)
GO(getsockcreatecon, iFbp_)
GO(getsockcreatecon_raw, iFbp_)
GO(is_context_customizable, iFp)
GO(is_selinux_enabled, iFv)
GO(is_selinux_mls_enabled, iFv)
GO(lgetfilecon, iFpbp_)
GO(lgetfilecon_raw, iFpbp_)
GO(lsetfilecon, iFpp)
GO(lsetfilecon_raw, iFpp)
//GO(manual_user_enter_context, 
//GO(map_class, 
//GO(map_decision, 
//GO(map_perm, 
GO(matchmediacon, iFpbp_)
GO(matchpathcon, iFpubp_)
GO(matchpathcon_checkmatches, vFp)
GO(matchpathcon_filespec_add, iFUip)
GO(matchpathcon_filespec_destroy, vFv)
GO(matchpathcon_filespec_eval, vFv)
GO(matchpathcon_fini, vFv)
GO(matchpathcon_index, iFpubp_)
GO(matchpathcon_init, iFp)
GO(matchpathcon_init_prefix, iFpp)
GO(mode_to_security_class, WFu)
//DATAB(myprintf_compat, 
GO(print_access_vector, vFWu)
//GO(query_user_context, 
GO(realpath_not_final, iFpp)
GO(rpm_execcon, iFupbp_bp_)
GO(security_av_perm_to_string, pFWu)
GO(security_av_string, iFWubp_)
GO(security_canonicalize_context, iFpbp_)
GO(security_canonicalize_context_raw, iFpbp_)
GO(security_check_context, iFp)
GO(security_check_context_raw, iFp)
GO(security_class_to_string, pFW)
GO(security_commit_booleans, iFv)
GO(security_compute_av, iFppWup)
GO(security_compute_av_flags, iFppWup)
GO(security_compute_av_flags_raw, iFppWup)
GO(security_compute_av_raw, iFppWup)
GO(security_compute_create, iFppWbp_)
GO(security_compute_create_name, iFppWpbp_)
GO(security_compute_create_name_raw, iFppWpbp_)
GO(security_compute_create_raw, iFppWbp_)
GO(security_compute_member, iFppWbp_)
GO(security_compute_member_raw, iFppWbp_)
GO(security_compute_relabel, iFppWbp_)
GO(security_compute_relabel_raw, iFppWbp_)
//GOM(security_compute_user, iFEppbp_)
//GOM(security_compute_user_raw, iFEppbp_)
GO(security_deny_unknown, iFv)
GO(security_disable, iFv)
GO(security_get_boolean_active, iFp)
//GOM(security_get_boolean_names, iFEbp_p)
GO(security_get_boolean_pending, iFp)
GO(security_get_checkreqprot, iFv)
GO(security_getenforce, iFv)
GO(security_get_initial_context, iFpbp_)
GO(security_get_initial_context_raw, iFpbp_)
GO(security_load_booleans, iFp)
GO(security_load_policy, iFpL)
GO(security_policyvers, iFv)
GO(security_reject_unknown, iFv)
GO(security_set_boolean, iFpi)
GO(security_set_boolean_list, iFLbpi_i)
GO(security_setenforce, iFi)
GO(security_validatetrans, iFppWp)
GO(security_validatetrans_raw, iFppWp)
//GO(selabel_close, 
//GO(selabel_cmp, 
//GO(selabel_digest, 
//GO(selabel_get_digests_all_partial_matches, 
//GO(selabel_hash_all_partial_matches, 
//GO(selabel_lookup, 
//GO(selabel_lookup_best_match, 
//GO(selabel_lookup_best_match_raw, 
//GO(selabel_lookup_raw, 
//GO(selabel_open, 
//GO(selabel_partial_match, 
//GO(selabel_stats, 
GO(selinux_binary_policy_path, pFv)
GO(selinux_booleans_path, pFv)
GO(selinux_booleans_subs_path, pFv)
GO(selinux_boolean_sub, pFp)
GO(selinux_check_access, iFppppp)
GO(selinux_check_passwd_access, iFu)
GO(selinux_check_securetty_context, iFp)
GO(selinux_colors_path, pFv)
GO(selinux_contexts_path, pFv)
GO(selinux_current_policy_path, pFv)
GO(selinux_customizable_types_path, pFv)
GO(selinux_default_context_path, pFv)
//GO(selinux_default_type_path, 
GO(selinux_failsafe_context_path, pFv)
GO(selinux_file_context_cmp, iFpp)
GO(selinux_file_context_homedir_path, pFv)
GO(selinux_file_context_local_path, pFv)
GO(selinux_file_context_path, pFv)
GO(selinux_file_context_subs_dist_path, pFv)
GO(selinux_file_context_subs_path, pFv)
GO(selinux_file_context_verify, iFpu)
GO(selinuxfs_exists, iFv)
//GOS(selinux_get_callback, pFpi)
GO(selinux_getenforcemode, iFp)
GO(selinux_getpolicytype, iFbp_)
GO(selinux_homedir_context_path, pFv)
GO(selinux_init_load_policy, iFp)
GO(selinux_lsetfilecon_default, iFp)
GO(selinux_lxc_contexts_path, pFv)
GO(selinux_media_context_path, pFv)
GO(selinux_mkload_policy, iFi)
//DATAB(selinux_mnt, 
GO(selinux_netfilter_context_path, pFv)
GO(selinux_openrc_contexts_path, pFv)
GO(selinux_openssh_contexts_path, pFv)
GO(selinux_path, pFv)
GO(selinux_policy_root, pFv)
GO(selinux_raw_context_to_color, iFpbp_)
GO(selinux_raw_to_trans_context, iFpbp_)
GO(selinux_removable_context_path, pFv)
GO(selinux_reset_config, vFv)
//GO(selinux_restorecon, 
//GO(selinux_restorecon_default_handle, 
//GO(selinux_restorecon_set_alt_rootpath, 
//GO(selinux_restorecon_set_exclude_list, 
//GO(selinux_restorecon_set_sehandle, 
//GO(selinux_restorecon_xattr, 
GO(selinux_securetty_types_path, pFv)
GO(selinux_sepgsql_context_path, pFv)
//GO(selinux_set_callback, 
//GO(selinux_set_mapping, iFbpppppppppppppppppppppppppppppppppp_)
GO(selinux_set_policy_root, iFp)
GO(selinux_snapperd_contexts_path, pFv)
//GO(selinux_status_close, 
//GO(selinux_status_deny_unknown, 
//GO(selinux_status_getenforce, 
//GO(selinux_status_open, 
//GO(selinux_status_policyload, 
//GO(selinux_status_updated, 
GO(selinux_systemd_contexts_path, pFv)
GO(selinux_translations_path, pFv)
GO(selinux_trans_to_raw_context, iFpbp_)
GO(selinux_user_contexts_path, pFv)
GO(selinux_usersconf_path, pFv)
GO(selinux_users_path, pFv)
GO(selinux_virtual_domain_context_path, pFv)
GO(selinux_virtual_image_context_path, pFv)
GO(selinux_x_context_path, pFv)
GO(setcon, iFp)
GO(setcon_raw, iFp)
GO(setexeccon, iFp)
GO(setexeccon_raw, iFp)
GO(setexecfilecon, iFpp)
GO(setfilecon, iFpp)
GO(setfilecon_raw, iFpp)
GO(setfscreatecon, iFp)
GO(setfscreatecon_raw, iFp)
GO(setkeycreatecon, iFp)
GO(setkeycreatecon_raw, iFp)
//GOM(set_matchpathcon_canoncon, vFEp)
GO(set_matchpathcon_flags, vFu)
//GOM(set_matchpathcon_invalidcon, vFEp)
//GOM(set_matchpathcon_printf, vFEp)
GO(set_selinuxmnt, vFp)
GO(setsockcreatecon, iFp)
GO(setsockcreatecon_raw, iFp)
//GO(sidget, 
//GO(sidput, 
GO(string_to_av_perm, uFWp)
GO(string_to_security_class, WFp)
//GO(unmap_class, 
//GO(unmap_perm, 
