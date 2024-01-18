#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//GO(gconf_address_backend, 
//GO(gconf_address_flags, 
//GO(gconf_address_list_free, 
//GO(gconf_address_list_get_persistent_name, 
//GO(gconf_address_resource, 
//GO(gconf_backend_file, 
//GO(gconf_backend_ref, 
//GO(gconf_backend_resolve_address, 
//GO(gconf_backend_unref, 
//GO(gconf_blow_away_locks, 
//GO(gconf_change_set_check_value, 
//GO(gconf_change_set_clear, 
//GO(gconf_change_set_foreach, 
//GO(gconf_change_set_get_type, 
//GO(gconf_change_set_get_user_data, 
//GO(gconf_change_set_new, 
//GO(gconf_change_set_ref, 
//GO(gconf_change_set_remove, 
//GO(gconf_change_set_set, 
//GO(gconf_change_set_set_bool, 
//GO(gconf_change_set_set_float, 
//GO(gconf_change_set_set_int, 
//GO(gconf_change_set_set_list, 
//GO(gconf_change_set_set_nocopy, 
//GO(gconf_change_set_set_pair, 
//GO(gconf_change_set_set_schema, 
//GO(gconf_change_set_set_string, 
//GO(gconf_change_set_set_user_data, 
//GO(gconf_change_set_size, 
//GO(gconf_change_set_unref, 
//GO(gconf_change_set_unset, 
//GO(gconf_clear_cache, 
GO(gconf_client_add_dir, vFppip)
//GO(gconf_client_all_dirs, 
//GO(gconf_client_all_entries, 
//GO(gconf_client_change_set_from_current, 
//GO(gconf_client_change_set_from_currentv, 
//GO(gconf_client_clear_cache, 
//GO(gconf_client_commit_change_set, 
//GO(gconf_client_dir_exists, 
//GO(gconf_client_error, 
GO(gconf_client_error_handling_mode_get_type, LFv)
GO(gconf_client_get, pFppp)
GO(gconf_client_get_bool, iFppp)
GO(gconf_client_get_default, pFv)
//GO(gconf_client_get_default_from_schema, 
//GO(gconf_client_get_entry, 
//GO(gconf_client_get_float, 
//GO(gconf_client_get_for_engine, 
GO(gconf_client_get_int, iFppp)
GO(gconf_client_get_list, pFppip)
//GO(gconf_client_get_pair, 
//GO(gconf_client_get_schema, 
GO(gconf_client_get_string, pFppp)
//GO(gconf_client_get_type, 
//GO(gconf_client_get_without_default, 
//GO(gconf_client_key_is_writable, 
//GO(gconf_client_notify, 
GOM(gconf_client_notify_add, uFEpppppp)
GO(gconf_client_notify_remove, vFpu)
//GO(gconf_client_preload, 
//GO(gconf_client_preload_type_get_type, 
//GO(gconf_client_recursive_unset, 
GO(gconf_client_remove_dir, vFppp)
//GO(gconf_client_reverse_change_set, 
//GO(gconf_client_set, 
//GO(gconf_client_set_bool, 
//GO(gconf_client_set_error_handling, 
//GO(gconf_client_set_float, 
//GO(gconf_client_set_global_default_error_handler, 
//GO(gconf_client_set_int, 
//GO(gconf_client_set_list, 
//GO(gconf_client_set_pair, 
//GO(gconf_client_set_schema, 
//GO(gconf_client_set_string, 
//GO(gconf_client_suggest_sync, 
//GO(gconf_client_unreturned_error, 
//GO(gconf_client_unset, 
//GO(gconf_client_value_changed, 
//GO(gconf_compose_errors, 
GO(gconf_concat_dir_and_key, pFpp)
//GO(gconf_current_locale, 
//GO(gconf_dbus_utils_append_entries, 
//GO(gconf_dbus_utils_append_entry_values, 
//GO(gconf_dbus_utils_append_value, 
//GO(gconf_dbus_utils_get_entries, 
//GO(gconf_dbus_utils_get_entry_values, 
//GO(gconf_dbus_utils_get_value, 
GO(gconf_debug_shutdown, iFv)
//GO(gconf_double_to_string, 
GO(gconf_engine_all_dirs, pFppp)
//GO(gconf_engine_all_entries, 
//GO(gconf_engine_associate_schema, 
//GO(gconf_engine_change_set_from_current, 
//GO(gconf_engine_change_set_from_currentv, 
//GO(gconf_engine_commit_change_set, 
//GO(gconf_engine_dir_exists, 
//GO(gconf_engine_get, 
GO(gconf_engine_get_bool, iFppp)
GO(gconf_engine_get_default, pFv)
GO(gconf_engine_get_default_from_schema, pFppp)
GO(gconf_engine_get_entry, pFpppip)
GO(gconf_engine_get_float, dFppp)
GO(gconf_engine_get_for_address, pFpp)
GO(gconf_engine_get_for_addresses, pFpp)
//GO(gconf_engine_get_full, 
//GO(gconf_engine_get_fuller, 
GO(gconf_engine_get_int, iFppp)
GO(gconf_engine_get_list, pFppup)
//GO(gconf_engine_get_local, 
//GO(gconf_engine_get_local_for_addresses, 
GO(gconf_engine_get_pair, iFppuuppp)
GO(gconf_engine_get_schema, pFppp)
GO(gconf_engine_get_string, pFppp)
GO(gconf_engine_get_user_data, pFp)
GO(gconf_engine_get_with_locale, pFpppp)
//GO(gconf_engine_get_without_default, 
//GO(gconf_engine_key_is_writable, 
//GO(gconf_engine_notify_add, 
GO(gconf_engine_notify_remove, vFpu)
//GO(gconf_engine_pop_owner_usage, 
//GO(gconf_engine_push_owner_usage, 
//GO(gconf_engine_recursive_unset, 
GO(gconf_engine_ref, vFp)
GO(gconf_engine_remove_dir, vFppp)
//GO(gconf_engine_reverse_change_set, 
GO(gconf_engine_set, iFpppp)
GO(gconf_engine_set_bool, iFppip)
GO(gconf_engine_set_float, iFppdp)
GO(gconf_engine_set_int, iFppip)
GO(gconf_engine_set_list, iFppupp)
//GO(gconf_engine_set_owner, 
GO(gconf_engine_set_pair, iFppuuppp)
GO(gconf_engine_set_schema, iFpppp)
GO(gconf_engine_set_string, iFpppp)
//GO(gconf_engine_set_user_data, 
GO(gconf_engine_suggest_sync, vFpp)
//GO(gconf_engine_unref, 
//GO(gconf_engine_unset, 
GO(gconf_entry_copy, pFp)
GO(gconf_entry_equal, iFpp)
//GO(gconf_entry_free, 
//GO(gconf_entry_get_is_default, 
GO(gconf_entry_get_is_writable, iFp)
GO(gconf_entry_get_key, pFp)
GO(gconf_entry_get_schema_name, pFp)
//GO(gconf_entry_get_type, 
GO(gconf_entry_get_value, pFp)
GO(gconf_entry_new, pFpp)
GO(gconf_entry_new_nocopy, pFpp)
GO(gconf_entry_ref, pFp)
//GO(gconf_entry_set_is_default, 
GO(gconf_entry_set_is_writable, vFpi)
GO(gconf_entry_set_schema_name, vFpp)
GO(gconf_entry_set_value, vFpp)
GO(gconf_entry_set_value_nocopy, vFpp)
GO(gconf_entry_steal_value, pFp)
GO(gconf_entry_unref, vFp)
GO(gconf_enum_to_string, pFpi)
//GO(gconf_error_get_type, 
//GO(gconf_error_new, 
GO(gconf_error_quark, uFv)
//GO(gconf_escape_key, 
//GO(gconf_get_backend, 
//GO(gconf_get_daemon_ior, 
//GO(gconf_in_daemon_mode, 
GO(gconf_init, iFipp)
//GO(_gconf_init_i18n, 
GO(gconf_is_initialized, iFv)
//GO(gconf_key_check, 
//GO(gconf_key_directory, 
GO(gconf_key_is_below, iFpp)
//GO(gconf_key_key, 
//GO(gconf_listeners_add, 
//GO(gconf_listeners_count, 
//GO(gconf_listeners_foreach, 
//GO(gconf_listeners_free, 
//GO(gconf_listeners_get_data, 
//GO(gconf_listeners_new, 
//GO(gconf_listeners_notify, 
//GO(gconf_listeners_remove, 
//GO(gconf_listeners_remove_if, 
//GO(gconf_load_source_path, 
//GO(gconf_locale_cache_expire, 
//GO(gconf_locale_cache_free, 
//GO(gconf_locale_cache_get_list, 
//GO(gconf_locale_cache_new, 
//GO(gconf_locale_list_ref, 
//GO(gconf_locale_list_unref, 
//GO(gconf_log, 
//DATAB(gconf_log_debug_messages, 
//GO(gconf_marshal_VOID__STRING_POINTER, 
GO(gconf_meta_info_free, vFp)
GO(gconf_meta_info_get_mod_user, pFp)
//GO(gconf_meta_info_get_schema, 
GO(gconf_meta_info_mod_time, iFp)
GO(gconf_meta_info_new, pFv)
GO(gconf_meta_info_set_mod_time, vFpi)
GO(gconf_meta_info_set_mod_user, vFpp)
//GO(gconf_meta_info_set_schema, 
//DATAB(gconf_options, 
//GO(gconf_persistent_name_get_address_list, 
//GO(gconf_ping_daemon, 
//GO(gconf_postinit, 
//GO(gconf_preinit, 
//GO(gconf_quote_string, 
//GO(gconf_resolve_address, 
GO(gconf_schema_copy, pFp)
GO(gconf_schema_free, vFp)
//GO(gconf_schema_get_car_type, 
GO(gconf_schema_get_cdr_type, uFp)
GO(gconf_schema_get_default_value, pFp)
//GO(gconf_schema_get_gettext_domain, 
//GO(gconf_schema_get_list_type, 
//GO(gconf_schema_get_locale, 
//GO(gconf_schema_get_long_desc, 
GO(gconf_schema_get_owner, pFp)
//GO(gconf_schema_get_short_desc, 
//GO(gconf_schema_get_type, 
GO(gconf_schema_new, pFv)
//GO(gconf_schema_set_car_type, 
GO(gconf_schema_set_cdr_type, vFpu)
GO(gconf_schema_set_default_value, vFpp)
GO(gconf_schema_set_default_value_nocopy, vFpp)
//GO(gconf_schema_set_gettext_domain, 
//GO(gconf_schema_set_list_type, 
//GO(gconf_schema_set_locale, 
//GO(gconf_schema_set_long_desc, 
GO(gconf_schema_set_owner, vFpp)
//GO(gconf_schema_set_short_desc, 
//GO(gconf_schema_set_type, 
//GO(gconf_schema_steal_default_value, 
//GO(gconf_schema_validate, 
//GO(gconf_set_daemon_ior, 
//GO(gconf_set_daemon_mode, 
//GO(gconf_set_error, 
//GO(gconf_shutdown_daemon, 
//GO(gconf_source_free, 
//GO(gconf_sources_add_listener, 
//GO(gconf_sources_all_dirs, 
//GO(gconf_sources_all_entries, 
//GO(gconf_sources_clear_cache, 
//GO(gconf_sources_clear_cache_for_sources, 
//GO(gconf_sources_dir_exists, 
//GO(gconf_sources_free, 
//GO(gconf_sources_is_affected, 
//GO(gconf_sources_new_from_addresses, 
//GO(gconf_sources_new_from_source, 
//GO(gconf_sources_query_default_value, 
//GO(gconf_sources_query_metainfo, 
//GO(gconf_sources_query_value, 
//GO(gconf_sources_recursive_unset, 
//GO(gconf_sources_remove_dir, 
//GO(gconf_sources_remove_listener, 
//GO(gconf_sources_set_notify_func, 
//GO(gconf_sources_set_schema, 
//GO(gconf_sources_set_value, 
//GO(gconf_sources_sync_all, 
//GO(gconf_sources_unset_value, 
//GO(gconf_spawn_daemon, 
//GO(gconf_split_locale, 
//GO(gconf_string_to_double, 
GO(gconf_string_to_enum, iFppp)
//GO(gconf_string_to_gulong, 
//GO(gconf_synchronous_sync, 
GO(gconf_unescape_key, pFpi)
GO(gconf_unique_key, pFv)
//GO(gconf_unquote_string, 
//GO(gconf_unquote_string_inplace, 
//GO(gconf_unset_flags_get_type, 
//GO(gconf_use_local_locks, 
GO(gconf_valid_key, iFpp)
GO(gconf_value_compare, iFpp)
GO(gconf_value_copy, pFp)
GO(gconf_value_decode, pFp)
GO(gconf_value_encode, pFp)
GO(gconf_value_free, vFp)
GO(gconf_value_get_bool, iFp)
//GO(gconf_value_get_car, 
//GO(gconf_value_get_cdr, 
GO(gconf_value_get_float, dFp)
GO(gconf_value_get_int, iFp)
GO(gconf_value_get_list, pFp)
GO(gconf_value_get_list_type, uFp)
GO(gconf_value_get_schema, pFp)
GO(gconf_value_get_string, pFp)
//GO(gconf_value_get_type, 
//GO(gconf_value_list_from_primitive_list, 
//GO(gconf_value_list_to_primitive_list_destructive, 
GO(gconf_value_new, pFu)
GO(gconf_value_new_from_string, pFupp)
//GO(gconf_value_new_list_from_string, 
//GO(gconf_value_new_pair_from_string, 
//GO(gconf_value_pair_from_primitive_pair, 
//GO(gconf_value_pair_to_primitive_pair_destructive, 
GO(gconf_value_set_bool, vFpi)
//GO(gconf_value_set_car, 
//GO(gconf_value_set_car_nocopy, 
GO(gconf_value_set_cdr, vFpp)
GO(gconf_value_set_cdr_nocopy, vFpp)
GO(gconf_value_set_float, vFpd)
GO(gconf_value_set_int, vFpi)
GO(gconf_value_set_list, vFpp)
//GO(gconf_value_set_list_nocopy, 
GO(gconf_value_set_list_type, vFpu)
GO(gconf_value_set_schema, vFpp)
GO(gconf_value_set_schema_nocopy, vFpp)
GO(gconf_value_set_string, vFpp)
//GO(gconf_value_set_string_nocopy, 
//GO(gconf_value_steal_list, 
//GO(gconf_value_steal_schema, 
//GO(gconf_value_steal_string, 
GO(gconf_value_to_string, pFp)
//GO(gconf_value_type_from_string, 
//GO(gconf_value_type_get_type, 
//GO(gconf_value_type_to_string, 
//GO(gconf_value_validate, 
