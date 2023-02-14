#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//GO(_fini, 
//GO(g_action_activate, 
//GO(g_action_change_state, 
//GO(g_action_get_enabled, 
//GO(g_action_get_name, 
//GO(g_action_get_parameter_type, 
//GO(g_action_get_state, 
//GO(g_action_get_state_hint, 
//GO(g_action_get_state_type, 
//GO(g_action_get_type, 
GO(g_action_group_action_added, vFpp)
GO(g_action_group_action_enabled_changed, vFppp)
GO(g_action_group_action_removed, vFpp)
GO(g_action_group_action_state_changed, vFppp)
GO(g_action_group_activate_action, vFppp)
GO(g_action_group_change_action_state, vFppp)
GO(g_action_group_get_action_enabled, iFpp)
GO(g_action_group_get_action_parameter_type, pFpp)
GO(g_action_group_get_action_state, pFpp)
GO(g_action_group_get_action_state_hint, pFpp)
GO(g_action_group_get_action_state_type, pFpp)
GO(g_action_group_get_type, LFv)
GO(g_action_group_has_action, iFpp)
GO(g_action_group_list_actions, pFp)
GO(g_action_group_query_action, iFppppppp)
//GO(g_action_map_add_action, 
//GO(g_action_map_add_action_entries, 
//GO(g_action_map_get_type, 
//GO(g_action_map_lookup_action, 
//GO(g_action_map_remove_action, 
//GO(g_action_name_is_valid, 
//GO(g_action_parse_detailed_name, 
//GO(g_action_print_detailed_name, 
GO(g_app_info_add_supports_type, iFppp)
GO(g_app_info_can_delete, iFp)
GO(g_app_info_can_remove_supports_type, iFp)
GO(g_app_info_create_flags_get_type, LFv)
GO(g_app_info_create_from_commandline, pFppup)
GO(g_app_info_delete, iFp)
GO(g_app_info_dup, pFp)
GO(g_app_info_equal, iFpp)
GO(g_app_info_get_all, pFv)
GO(g_app_info_get_all_for_type, pFp)
GO(g_app_info_get_commandline, pFp)
GO(g_app_info_get_default_for_type, pFpp)
GO(g_app_info_get_default_for_uri_scheme, pFp)
GO(g_app_info_get_description, pFp)
GO(g_app_info_get_display_name, pFp)
GO(g_app_info_get_executable, pFp)
GO(g_app_info_get_fallback_for_type, pFp)
GO(g_app_info_get_icon, pFp)
GO(g_app_info_get_id, pFp)
GO(g_app_info_get_name, pFp)
GO(g_app_info_get_recommended_for_type, pFp)
GO(g_app_info_get_supported_types, pFp)
GO(g_app_info_get_type, LFv)
GO(g_app_info_launch, iFpppp)
GO(g_app_info_launch_default_for_uri, iFppp)
GO(g_app_info_launch_uris, iFpppp)
//GO(g_app_info_monitor_get, 
GO(g_app_info_monitor_get_type, LFv)
GO(g_app_info_remove_supports_type, iFppp)
GO(g_app_info_reset_type_associations, vFp)
GO(g_app_info_set_as_default_for_extension, iFppp)
GO(g_app_info_set_as_default_for_type, iFppp)
GO(g_app_info_set_as_last_used_for_type, iFppp)
GO(g_app_info_should_show, iFp)
GO(g_app_info_supports_files, iFp)
GO(g_app_info_supports_uris, iFp)
//GO(g_app_launch_context_get_display, 
//GO(g_app_launch_context_get_environment, 
//GO(g_app_launch_context_get_startup_notify_id, 
GO(g_app_launch_context_get_type, LFv)
//GO(g_app_launch_context_launch_failed, 
//GO(g_app_launch_context_new, 
//GO(g_app_launch_context_setenv, 
//GO(g_app_launch_context_unsetenv, 
//GO(g_application_activate, 
//GO(g_application_add_main_option, 
//GO(g_application_add_main_option_entries, 
//GO(g_application_add_option_group, 
//GO(g_application_command_line_create_file_for_arg, 
//GO(g_application_command_line_get_arguments, 
//GO(g_application_command_line_get_cwd, 
//GO(g_application_command_line_getenv, 
//GO(g_application_command_line_get_environ, 
//GO(g_application_command_line_get_exit_status, 
//GO(g_application_command_line_get_is_remote, 
//GO(g_application_command_line_get_options_dict, 
//GO(g_application_command_line_get_platform_data, 
//GO(g_application_command_line_get_stdin, 
//GO(g_application_command_line_get_type, 
//GO(g_application_command_line_print, 
//GO(g_application_command_line_printerr, 
//GO(g_application_command_line_set_exit_status, 
//GO(g_application_flags_get_type, 
//GO(g_application_get_application_id, 
//GO(g_application_get_dbus_connection, 
//GO(g_application_get_dbus_object_path, 
//GO(g_application_get_default, 
//GO(g_application_get_flags, 
//GO(g_application_get_inactivity_timeout, 
//GO(g_application_get_is_registered, 
//GO(g_application_get_is_remote, 
//GO(g_application_get_resource_base_path, 
//GO(g_application_get_type, 
//GO(g_application_hold, 
//GO(g_application_id_is_valid, 
//GO(g_application_mark_busy, 
//GO(g_application_new, 
//GO(g_application_open, 
//GO(g_application_quit, 
//GO(g_application_register, 
//GO(g_application_release, 
//GO(g_application_run, 
//GO(g_application_send_notification, 
//GO(g_application_set_action_group, 
//GO(g_application_set_application_id, 
//GO(g_application_set_default, 
//GO(g_application_set_flags, 
//GO(g_application_set_inactivity_timeout, 
//GO(g_application_set_resource_base_path, 
//GO(g_application_unmark_busy, 
//GO(g_application_withdraw_notification, 
//GO(g_ask_password_flags_get_type, 
GO(g_async_initable_get_type, LFv)
GOM(g_async_initable_init_async, vFEpippp)
GO(g_async_initable_init_finish, iFppp)
GOM(g_async_initable_new_async, vFEiippppV)
GO(g_async_initable_new_finish, pFppp)
GOM(g_async_initable_new_valist_async, vFEipAippp)
GOM(g_async_initable_newv_async, vFEiupippp)
//GO(g_async_result_get_source_object, 
//GO(g_async_result_get_type, 
//GO(g_async_result_get_user_data, 
//GO(g_async_result_is_tagged, 
//GO(g_async_result_legacy_propagate_error, 
//GO(g_buffered_input_stream_fill, 
//GO(g_buffered_input_stream_fill_async, 
//GO(g_buffered_input_stream_fill_finish, 
//GO(g_buffered_input_stream_get_available, 
//GO(g_buffered_input_stream_get_buffer_size, 
//GO(g_buffered_input_stream_get_type, 
//GO(g_buffered_input_stream_new, 
//GO(g_buffered_input_stream_new_sized, 
//GO(g_buffered_input_stream_peek, 
//GO(g_buffered_input_stream_peek_buffer, 
//GO(g_buffered_input_stream_read_byte, 
//GO(g_buffered_input_stream_set_buffer_size, 
//GO(g_buffered_output_stream_get_auto_grow, 
//GO(g_buffered_output_stream_get_buffer_size, 
//GO(g_buffered_output_stream_get_type, 
//GO(g_buffered_output_stream_new, 
//GO(g_buffered_output_stream_new_sized, 
//GO(g_buffered_output_stream_set_auto_grow, 
//GO(g_buffered_output_stream_set_buffer_size, 
GOM(g_bus_get, vFEippp)
GO(g_bus_get_finish, pFpp)
GO(g_bus_get_sync, pFipp)
GO(g_bus_name_owner_flags_get_type, LFv)
GO(g_bus_name_watcher_flags_get_type, LFv)
GOM(g_bus_own_name, uFEipippppp)
GOM(g_bus_own_name_on_connection, uFEppipppp)
GO(g_bus_own_name_on_connection_with_closures, uFppipp)
GO(g_bus_own_name_with_closures, uFppippp)
GO(g_bus_type_get_type, LFv)
GO(g_bus_unown_name, vFu)
GO(g_bus_unwatch_name, vFu)
GOM(g_bus_watch_name, uFEipipppp)
GOM(g_bus_watch_name_on_connection, uFEppipppp)
GO(g_bus_watch_name_on_connection_with_closures, uFppipp)   // mmm closure, wrapped or unwrapped?
GO(g_bus_watch_name_with_closures, uFipipp)
//GO(g_bytes_icon_get_bytes, 
//GO(g_bytes_icon_get_type, 
//GO(g_bytes_icon_new, 
GO(g_cancellable_cancel, vFp)
GOM(g_cancellable_connect, LFEpppp)
GO(g_cancellable_disconnect, vFpL)
GO(g_cancellable_get_current, pFv)
GO(g_cancellable_get_fd, iFp)
GO(g_cancellable_get_type, LFv)
GO(g_cancellable_is_cancelled, iFp)
GO(g_cancellable_make_pollfd, iFpp)
GO(g_cancellable_new, pFv)
GO(g_cancellable_pop_current, vFp)
GO(g_cancellable_push_current, vFp)
GO(g_cancellable_release_fd, vFp)
GO(g_cancellable_reset, vFp)
GO(g_cancellable_set_error_if_cancelled, iFpp)
GO(g_cancellable_source_new, pFp)
//GO(g_charset_converter_get_num_fallbacks, 
GO(g_charset_converter_get_type, LFv)
//GO(g_charset_converter_get_use_fallback, 
//GO(g_charset_converter_new, 
//GO(g_charset_converter_set_use_fallback, 
GO(g_content_type_can_be_executable, iFp)
GO(g_content_type_equals, iFpp)
GO(g_content_type_from_mime_type, pFp)
GO(g_content_type_get_description, pFp)
GO(g_content_type_get_generic_icon_name, pFp)
GO(g_content_type_get_icon, pFp)
GO(g_content_type_get_mime_type, pFp)
GO(g_content_type_get_symbolic_icon, pFp)
GO(g_content_type_guess, pFppLp)
GO(g_content_type_guess_for_tree, pFp)
GO(g_content_type_is_a, iFpp)
GO(g_content_type_is_unknown, iFp)
GO(g_content_types_get_registered, pFv)
//GO(g_converter_convert, 
//GO(g_converter_flags_get_type, 
//GO(g_converter_get_type, 
//GO(g_converter_input_stream_get_converter, 
//GO(g_converter_input_stream_get_type, 
//GO(g_converter_input_stream_new, 
//GO(g_converter_output_stream_get_converter, 
//GO(g_converter_output_stream_get_type, 
//GO(g_converter_output_stream_new, 
//GO(g_converter_reset, 
//GO(g_converter_result_get_type, 
//GO(g_credentials_get_native, 
//GO(g_credentials_get_type, 
//GO(g_credentials_get_unix_pid, 
//GO(g_credentials_get_unix_user, 
//GO(g_credentials_is_same_user, 
//GO(g_credentials_new, 
//GO(g_credentials_set_native, 
//GO(g_credentials_set_unix_user, 
//GO(g_credentials_to_string, 
//GO(g_credentials_type_get_type, 
//GO(g_data_input_stream_get_byte_order, 
//GO(g_data_input_stream_get_newline_type, 
//GO(g_data_input_stream_get_type, 
//GO(g_data_input_stream_new, 
//GO(g_data_input_stream_read_byte, 
//GO(g_data_input_stream_read_int16, 
//GO(g_data_input_stream_read_int32, 
//GO(g_data_input_stream_read_int64, 
//GO(g_data_input_stream_read_line, 
//GO(g_data_input_stream_read_line_async, 
//GO(g_data_input_stream_read_line_finish, 
//GO(g_data_input_stream_read_line_finish_utf8, 
//GO(g_data_input_stream_read_line_utf8, 
//GO(g_data_input_stream_read_uint16, 
//GO(g_data_input_stream_read_uint32, 
//GO(g_data_input_stream_read_uint64, 
//GO(g_data_input_stream_read_until, 
//GO(g_data_input_stream_read_until_async, 
//GO(g_data_input_stream_read_until_finish, 
//GO(g_data_input_stream_read_upto, 
//GO(g_data_input_stream_read_upto_async, 
//GO(g_data_input_stream_read_upto_finish, 
//GO(g_data_input_stream_set_byte_order, 
//GO(g_data_input_stream_set_newline_type, 
//GO(g_data_output_stream_get_byte_order, 
//GO(g_data_output_stream_get_type, 
//GO(g_data_output_stream_new, 
//GO(g_data_output_stream_put_byte, 
//GO(g_data_output_stream_put_int16, 
//GO(g_data_output_stream_put_int32, 
//GO(g_data_output_stream_put_int64, 
//GO(g_data_output_stream_put_string, 
//GO(g_data_output_stream_put_uint16, 
//GO(g_data_output_stream_put_uint32, 
//GO(g_data_output_stream_put_uint64, 
//GO(g_data_output_stream_set_byte_order, 
//GO(g_data_stream_byte_order_get_type, 
//GO(g_data_stream_newline_type_get_type, 
//GO(g_dbus_action_group_get, 
//GO(g_dbus_action_group_get_type, 
//GO(g_dbus_address_escape_value, 
//GO(g_dbus_address_get_for_bus_sync, 
//GO(g_dbus_address_get_stream, 
//GO(g_dbus_address_get_stream_finish, 
//GO(g_dbus_address_get_stream_sync, 
GO(g_dbus_annotation_info_get_type, LFv)
GO(g_dbus_annotation_info_lookup, pFpp)
GO(g_dbus_annotation_info_ref, pFp)
GO(g_dbus_annotation_info_unref, vFp)
GO(g_dbus_arg_info_get_type, LFv)
GO(g_dbus_arg_info_ref, pFp)
GO(g_dbus_arg_info_unref, vFp)
//GO(g_dbus_auth_observer_allow_mechanism, 
//GO(g_dbus_auth_observer_authorize_authenticated_peer, 
//GO(g_dbus_auth_observer_get_type, 
//GO(g_dbus_auth_observer_new, 
GO(g_dbus_call_flags_get_type, LFv)
GO(g_dbus_capability_flags_get_type, LFv)
GOM(g_dbus_connection_add_filter, uFEpppp)
GOM(g_dbus_connection_call, vFEpppppppiippp)
GO(g_dbus_connection_call_finish, pFppp)
GO(g_dbus_connection_call_sync, pFpppppppiipp)
//GO(g_dbus_connection_call_with_unix_fd_list, 
//GO(g_dbus_connection_call_with_unix_fd_list_finish, 
//GO(g_dbus_connection_call_with_unix_fd_list_sync, 
GOM(g_dbus_connection_close, vFEpppp)
GO(g_dbus_connection_close_finish, iFppp)
GO(g_dbus_connection_close_sync, iFppp)
GO(g_dbus_connection_emit_signal, iFppppppp)
//GO(g_dbus_connection_export_action_group, 
//GO(g_dbus_connection_export_menu_model, 
GO(g_dbus_connection_flags_get_type, LFv)
GOM(g_dbus_connection_flush, vFEpppp)
GO(g_dbus_connection_flush_finish, iFppp)
GO(g_dbus_connection_flush_sync, iFppp)
GO(g_dbus_connection_get_capabilities, iFp)
GO(g_dbus_connection_get_exit_on_close, iFp)
GO(g_dbus_connection_get_guid, pFp)
GO(g_dbus_connection_get_last_serial, uFp)
GO(g_dbus_connection_get_peer_credentials, pFp)
GO(g_dbus_connection_get_stream, pFp)
GO(g_dbus_connection_get_type, LFv)
GO(g_dbus_connection_get_unique_name, pFp)
GO(g_dbus_connection_is_closed, iFp)
GOM(g_dbus_connection_new, vFEppipppp)
GO(g_dbus_connection_new_finish, pFpp)
GOM(g_dbus_connection_new_for_address, vFEpipppp)
GO(g_dbus_connection_new_for_address_finish, pFpp)
GO(g_dbus_connection_new_for_address_sync, pFpippp)
GO(g_dbus_connection_new_sync, pFppippp)
GOM(g_dbus_connection_register_object, uFEppppppp)
//GOM(g_dbus_connection_register_subtree, uFEppppppp)   // vtable need wrapping
GO(g_dbus_connection_remove_filter, vFpu)
GO(g_dbus_connection_send_message, iFppipp)     //TODO: there is an argument as "volatile".. does that change anything???
GOM(g_dbus_connection_send_message_with_reply, vFEppiipppp) //TODO: same volatile argument...
GO(g_dbus_connection_send_message_with_reply_finish, pFppp)
GO(g_dbus_connection_send_message_with_reply_sync, pFppiippp)   //TODO: same volatile
GO(g_dbus_connection_set_exit_on_close, vFpi)
GOM(g_dbus_connection_signal_subscribe, uFEppppppippp)
GO(g_dbus_connection_signal_unsubscribe, vFpu)
GO(g_dbus_connection_start_message_processing, vFp)
//GO(g_dbus_connection_unexport_action_group, 
//GO(g_dbus_connection_unexport_menu_model, 
GO(g_dbus_connection_unregister_object, iFpu)
GO(g_dbus_connection_unregister_subtree, iFpu)
//GO(g_dbus_error_encode_gerror, 
//GO(g_dbus_error_get_remote_error, 
//GO(g_dbus_error_get_type, 
//GO(g_dbus_error_is_remote_error, 
//GO(g_dbus_error_new_for_dbus_error, 
//GO(g_dbus_error_quark, 
//GO(g_dbus_error_register_error, 
GO(g_dbus_error_register_error_domain, vFpppu)
//GO(g_dbus_error_set_dbus_error, 
//GO(g_dbus_error_set_dbus_error_valist, 
//GO(g_dbus_error_strip_remote_error, 
//GO(g_dbus_error_unregister_error, 
GO(g_dbus_generate_guid, pFv)
GO(g_dbus_gvalue_to_gvariant, pFpp)
GO(g_dbus_gvariant_to_gvalue, vFpp)
//GO(g_dbus_interface_dup_object, 
//GO(g_dbus_interface_get_info, 
//GO(g_dbus_interface_get_object, 
//GO(g_dbus_interface_get_type, 
GO(g_dbus_interface_info_cache_build, vFp)
GO(g_dbus_interface_info_cache_release, vFp)
GO(g_dbus_interface_info_generate_xml, vFpup)
GO(g_dbus_interface_info_get_type, LFv)
GO(g_dbus_interface_info_lookup_method, pFpp)
GO(g_dbus_interface_info_lookup_property, pFpp)
GO(g_dbus_interface_info_lookup_signal, pFpp)
GO(g_dbus_interface_info_ref, pFp)
GO(g_dbus_interface_info_unref, vFp)
//GO(g_dbus_interface_set_object, 
//GO(g_dbus_interface_skeleton_export, 
//GO(g_dbus_interface_skeleton_flags_get_type, 
//GO(g_dbus_interface_skeleton_flush, 
//GO(g_dbus_interface_skeleton_get_connection, 
//GO(g_dbus_interface_skeleton_get_connections, 
//GO(g_dbus_interface_skeleton_get_flags, 
//GO(g_dbus_interface_skeleton_get_info, 
//GO(g_dbus_interface_skeleton_get_object_path, 
//GO(g_dbus_interface_skeleton_get_properties, 
//GO(g_dbus_interface_skeleton_get_type, 
//GO(g_dbus_interface_skeleton_get_vtable, 
//GO(g_dbus_interface_skeleton_has_connection, 
//GO(g_dbus_interface_skeleton_set_flags, 
//GO(g_dbus_interface_skeleton_unexport, 
//GO(g_dbus_interface_skeleton_unexport_from_connection, 
GO(g_dbus_is_address, iFp)
GO(g_dbus_is_guid, iFp)
GO(g_dbus_is_interface_name, iFp)
GO(g_dbus_is_member_name, iFp)
GO(g_dbus_is_name, iFp)
GO(g_dbus_is_supported_address, iFpp)
GO(g_dbus_is_unique_name, iFp)
GO(g_dbus_menu_model_get, pFppp)
GO(g_dbus_menu_model_get_type, LFv)
GO(g_dbus_message_byte_order_get_type, LFv)
GO(g_dbus_message_bytes_needed, lFpLp)
GO(g_dbus_message_copy, pFpp)
GO(g_dbus_message_flags_get_type, LFv)
GO(g_dbus_message_get_arg0, pFp)
GO(g_dbus_message_get_body, pFp)
GO(g_dbus_message_get_byte_order, iFp)
GO(g_dbus_message_get_destination, pFp)
GO(g_dbus_message_get_error_name, pFp)
GO(g_dbus_message_get_flags, iFp)
GO(g_dbus_message_get_header, pFpi)
GO(g_dbus_message_get_header_fields, pFp)
GO(g_dbus_message_get_interface, pFp)
GO(g_dbus_message_get_locked, iFp)
GO(g_dbus_message_get_member, pFp)
GO(g_dbus_message_get_message_type, iFp)
GO(g_dbus_message_get_num_unix_fds, uFp)
GO(g_dbus_message_get_path, pFp)
GO(g_dbus_message_get_reply_serial, uFp)
GO(g_dbus_message_get_sender, pFp)
GO(g_dbus_message_get_serial, uFp)
GO(g_dbus_message_get_signature, pFp)
GO(g_dbus_message_get_type, LFv)
GO(g_dbus_message_get_unix_fd_list, pFp)
GO(g_dbus_message_header_field_get_type, LFv)
GO(g_dbus_message_lock, vFp)
GO(g_dbus_message_new, pFv)
GO(g_dbus_message_new_from_blob, pFpLip)
GO(g_dbus_message_new_method_call, pFpppp)
//GO2(g_dbus_message_new_method_error, pFpppV, g_dbus_message_new_method_error_valist)
GO(g_dbus_message_new_method_error_literal, pFppp)
//GO(g_dbus_message_new_method_error_valist, pFpppp)
GO(g_dbus_message_new_method_reply, pFp)
GO(g_dbus_message_new_signal, pFppp)
GO(g_dbus_message_print, pFpu)
GO(g_dbus_message_set_body, vFpp)
GO(g_dbus_message_set_byte_order, iFp)
GO(g_dbus_message_set_destination, vFpp)
GO(g_dbus_message_set_error_name, vFpp)
GO(g_dbus_message_set_flags, vFpi)
GO(g_dbus_message_set_header, vFpip)
GO(g_dbus_message_set_interface, vFpp)
GO(g_dbus_message_set_member, vFpp)
GO(g_dbus_message_set_message_type, vFpi)
GO(g_dbus_message_set_num_unix_fds, vFpu)
GO(g_dbus_message_set_path, vFpp)
GO(g_dbus_message_set_reply_serial, vFpu)
GO(g_dbus_message_set_sender, vFpp)
GO(g_dbus_message_set_serial, vFpu)
GO(g_dbus_message_set_signature, vFpp)
GO(g_dbus_message_set_unix_fd_list, vFpp)
GO(g_dbus_message_to_blob, pFppip)
GO(g_dbus_message_to_gerror, iFpp)
GO(g_dbus_message_type_get_type, LFv)
GO(g_dbus_method_info_get_type, LFv)
GO(g_dbus_method_info_ref, pFp)
GO(g_dbus_method_info_unref, vFp)
GO(g_dbus_method_invocation_get_connection, pFp)
GO(g_dbus_method_invocation_get_interface_name, pFp)
GO(g_dbus_method_invocation_get_message, pFp)
GO(g_dbus_method_invocation_get_method_info, pFp)
GO(g_dbus_method_invocation_get_method_name, pFp)
GO(g_dbus_method_invocation_get_object_path, pFp)
GO(g_dbus_method_invocation_get_parameters, pFp)
GO(g_dbus_method_invocation_get_property_info, pFp)
GO(g_dbus_method_invocation_get_sender, pFp)
GO(g_dbus_method_invocation_get_type, LFv)
GO(g_dbus_method_invocation_get_user_data, pFp)
GO(g_dbus_method_invocation_return_dbus_error, vFppp)
//GO2(g_dbus_method_invocation_return_error, vFpuipV, g_dbus_method_invocation_return_error_valist)
GO(g_dbus_method_invocation_return_error_literal, vFpuip)
//GO(g_dbus_method_invocation_return_error_valist, vFpuipp)
GO(g_dbus_method_invocation_return_gerror, vFpp)
GO(g_dbus_method_invocation_return_value, vFpp)
GO(g_dbus_method_invocation_return_value_with_unix_fd_list, vFppp)
GO(g_dbus_method_invocation_take_error, vFpp)
GO(g_dbus_node_info_generate_xml, vFpup)
GO(g_dbus_node_info_get_type, LFv)
GO(g_dbus_node_info_lookup_interface, pFpp)
GO(g_dbus_node_info_new_for_xml, pFpp)
GO(g_dbus_node_info_ref, pFp)
GO(g_dbus_node_info_unref, vFp)
GO(g_dbus_object_get_interface, pFpp)
GO(g_dbus_object_get_interfaces, pFp)
GO(g_dbus_object_get_object_path, pFp)
GO(g_dbus_object_get_type, LFv)
GO(g_dbus_object_manager_client_flags_get_type, LFv)
GO(g_dbus_object_manager_client_get_connection, pFp)
GO(g_dbus_object_manager_client_get_flags, iFp)
GO(g_dbus_object_manager_client_get_name, pFp)
GO(g_dbus_object_manager_client_get_name_owner, pFp)
GO(g_dbus_object_manager_client_get_type, LFv)
//GOM(g_dbus_object_manager_client_new, 
GO(g_dbus_object_manager_client_new_finish, pFpp)
//GOM(g_dbus_object_manager_client_new_for_bus, 
GO(g_dbus_object_manager_client_new_for_bus_finish, pFpp)
GOM(g_dbus_object_manager_client_new_for_bus_sync, pFEiippppppp)
//GOM(g_dbus_object_manager_client_new_sync, 
//GOM(g_dbus_object_manager_get_interface, pFEppp)  //struct GDBusObjectManagerIface is full of callbacks
GO(g_dbus_object_manager_get_object, pFpp)
GO(g_dbus_object_manager_get_object_path, pFp)
GO(g_dbus_object_manager_get_objects, pFp)
//GO(g_dbus_object_manager_get_type, 
//GO(g_dbus_object_manager_server_export, 
//GO(g_dbus_object_manager_server_export_uniquely, 
//GO(g_dbus_object_manager_server_get_connection, 
//GO(g_dbus_object_manager_server_get_type, 
//GO(g_dbus_object_manager_server_is_exported, 
//GO(g_dbus_object_manager_server_new, 
//GO(g_dbus_object_manager_server_set_connection, 
//GO(g_dbus_object_manager_server_unexport, 
//GO(g_dbus_object_proxy_get_connection, 
//GO(g_dbus_object_proxy_get_type, 
//GO(g_dbus_object_proxy_new, 
//GO(g_dbus_object_skeleton_add_interface, 
//GO(g_dbus_object_skeleton_flush, 
//GO(g_dbus_object_skeleton_get_type, 
//GO(g_dbus_object_skeleton_new, 
//GO(g_dbus_object_skeleton_remove_interface, 
//GO(g_dbus_object_skeleton_remove_interface_by_name, 
//GO(g_dbus_object_skeleton_set_object_path, 
//GO(g_dbus_property_info_flags_get_type, 
GO(g_dbus_property_info_get_type, LFv)
GO(g_dbus_property_info_ref, pFp)
GO(g_dbus_property_info_unref, vFp)
GOM(g_dbus_proxy_call, vFEpppiippp)
GO(g_dbus_proxy_call_finish, pFppp)
GO(g_dbus_proxy_call_sync, pFpppiipp)
GOM(g_dbus_proxy_call_with_unix_fd_list, vFEpppiipppp)
GO(g_dbus_proxy_call_with_unix_fd_list_finish, pFpppp)
GO(g_dbus_proxy_call_with_unix_fd_list_sync, pFpppiipppp)
GO(g_dbus_proxy_flags_get_type, LFv)
GO(g_dbus_proxy_get_cached_property, pFpp)
GO(g_dbus_proxy_get_cached_property_names, pFp)
GO(g_dbus_proxy_get_connection, pFp)
GO(g_dbus_proxy_get_default_timeout, iFp)
GO(g_dbus_proxy_get_flags, iFp)
GO(g_dbus_proxy_get_interface_info, pFp)
GO(g_dbus_proxy_get_interface_name, pFp)
GO(g_dbus_proxy_get_name, pFp)
GO(g_dbus_proxy_get_name_owner, pFp)
GO(g_dbus_proxy_get_object_path, pFp)
GO(g_dbus_proxy_get_type, LFv)
GOM(g_dbus_proxy_new, vFEpippppppp)
GO(g_dbus_proxy_new_finish, pFpp)
GOM(g_dbus_proxy_new_for_bus, vFEiippppppp)
GO(g_dbus_proxy_new_for_bus_finish, pFpp)
GO(g_dbus_proxy_new_for_bus_sync, pFiipppppp)
GO(g_dbus_proxy_new_sync, pFpipppppp)
GO(g_dbus_proxy_set_cached_property, vFppp)
GO(g_dbus_proxy_set_default_timeout, vFpi)
GO(g_dbus_proxy_set_interface_info, vFpp)
//GO(g_dbus_send_message_flags_get_type, 
//GO(g_dbus_server_flags_get_type, 
//GO(g_dbus_server_get_client_address, 
//GO(g_dbus_server_get_flags, 
//GO(g_dbus_server_get_guid, 
//GO(g_dbus_server_get_type, 
//GO(g_dbus_server_is_active, 
//GO(g_dbus_server_new_sync, 
//GO(g_dbus_server_start, 
//GO(g_dbus_server_stop, 
//GO(g_dbus_signal_flags_get_type, 
GO(g_dbus_signal_info_get_type, LFv)
GO(g_dbus_signal_info_ref, pFp)
GO(g_dbus_signal_info_unref, vFp)
GO(g_dbus_subtree_flags_get_type, LFv)
//GO(g_desktop_app_info_get_action_name, 
//GO(g_desktop_app_info_get_boolean, 
GO(g_desktop_app_info_get_categories, pFp)
GO(g_desktop_app_info_get_filename, pFp)
GO(g_desktop_app_info_get_generic_name, pFp)
//GO(g_desktop_app_info_get_implementations, 
GO(g_desktop_app_info_get_is_hidden, iFp)
GO(g_desktop_app_info_get_keywords, pFp)
GO(g_desktop_app_info_get_nodisplay, iFp)
GO(g_desktop_app_info_get_show_in, iFpp)
//GO(g_desktop_app_info_get_startup_wm_class, 
//GO(g_desktop_app_info_get_string, 
GO(g_desktop_app_info_get_type, LFv)
//GO(g_desktop_app_info_has_key, 
//GO(g_desktop_app_info_launch_action, 
//GO(g_desktop_app_info_launch_uris_as_manager, 
//GO(g_desktop_app_info_list_actions, 
//GO(g_desktop_app_info_lookup_get_default_for_uri_scheme, 
GO(g_desktop_app_info_lookup_get_type, LFv)
GO(g_desktop_app_info_new, pFp)
GO(g_desktop_app_info_new_from_filename, pFp)
GO(g_desktop_app_info_new_from_keyfile, pFp)
//GO(g_desktop_app_info_search, 
GO(g_desktop_app_info_set_desktop_env, vFp)
//GO(g_drive_can_eject, 
//GO(g_drive_can_poll_for_media, 
//GO(g_drive_can_start, 
//GO(g_drive_can_start_degraded, 
//GO(g_drive_can_stop, 
//GO(g_drive_eject, 
//GO(g_drive_eject_finish, 
//GO(g_drive_eject_with_operation, 
//GO(g_drive_eject_with_operation_finish, 
//GO(g_drive_enumerate_identifiers, 
//GO(g_drive_get_icon, 
//GO(g_drive_get_identifier, 
//GO(g_drive_get_name, 
//GO(g_drive_get_sort_key, 
//GO(g_drive_get_start_stop_type, 
//GO(g_drive_get_symbolic_icon, 
//GO(g_drive_get_type, 
//GO(g_drive_get_volumes, 
//GO(g_drive_has_media, 
//GO(g_drive_has_volumes, 
//GO(g_drive_is_media_check_automatic, 
//GO(g_drive_is_media_removable, 
//GO(g_drive_poll_for_media, 
//GO(g_drive_poll_for_media_finish, 
//GO(g_drive_start, 
//GO(g_drive_start_finish, 
//GO(g_drive_start_flags_get_type, 
//GO(g_drive_start_stop_type_get_type, 
//GO(g_drive_stop, 
//GO(g_drive_stop_finish, 
//GO(g_emblemed_icon_add_emblem, 
//GO(g_emblemed_icon_clear_emblems, 
//GO(g_emblemed_icon_get_emblems, 
//GO(g_emblemed_icon_get_icon, 
//GO(g_emblemed_icon_get_type, 
//GO(g_emblemed_icon_new, 
//GO(g_emblem_get_icon, 
//GO(g_emblem_get_origin, 
//GO(g_emblem_get_type, 
//GO(g_emblem_new, 
//GO(g_emblem_new_with_origin, 
//GO(g_emblem_origin_get_type, 
GO(g_file_append_to, pFpipp)
//GOM(g_file_append_to_async, vFEpiipBp)
GO(g_file_append_to_finish, pFppp)
//GO(g_file_attribute_info_flags_get_type, 
//GO(g_file_attribute_info_list_add, 
//GO(g_file_attribute_info_list_dup, 
//GO(g_file_attribute_info_list_get_type, 
//GO(g_file_attribute_info_list_lookup, 
//GO(g_file_attribute_info_list_new, 
//GO(g_file_attribute_info_list_ref, 
//GO(g_file_attribute_info_list_unref, 
//GO(g_file_attribute_matcher_enumerate_namespace, 
//GO(g_file_attribute_matcher_enumerate_next, 
//GO(g_file_attribute_matcher_get_type, 
//GO(g_file_attribute_matcher_matches, 
//GO(g_file_attribute_matcher_matches_only, 
//GO(g_file_attribute_matcher_new, 
//GO(g_file_attribute_matcher_ref, 
//GO(g_file_attribute_matcher_subtract, 
//GO(g_file_attribute_matcher_to_string, 
//GO(g_file_attribute_matcher_unref, 
//GO(g_file_attribute_status_get_type, 
//GO(g_file_attribute_type_get_type, 
//GOM(g_file_copy, iFEppipBpp)
//GOM(g_file_copy_async, vFEppiipBpBp)
GO(g_file_copy_attributes, iFppipp)
GO(g_file_copy_finish, iFppp)
GO(g_file_copy_flags_get_type, LFv)
GO(g_file_create, pFpipp)
//GOM(g_file_create_async, vFEpiipBp)
GO(g_file_create_finish, pFppp)
GO(g_file_create_flags_get_type, LFv)
GO(g_file_create_readwrite, pFpipp)
//GOM(g_file_create_readwrite_async, vFEpiipBp)
GO(g_file_create_readwrite_finish, pFppp)
GO(g_file_delete, iFppp)
//GOM(g_file_delete_async, vFEpipBp)
GO(g_file_delete_finish, pFppp)
//GO(g_file_descriptor_based_get_fd, 
//GO(g_file_descriptor_based_get_type, 
GO(g_file_dup, pFp)
//GOM(g_file_eject_mountable, vFEpipBp)
GO(g_file_eject_mountable_finish, iFppp)
//GOM(g_file_eject_mountable_with_operation, vFpippBp)
GO(g_file_eject_mountable_with_operation_finish, iFppp)
GO(g_file_enumerate_children, pFppipp)
//GOM(g_file_enumerate_children_async, vFppiipBp)
GO(g_file_enumerate_children_finish, pFppp)
//GO(g_file_enumerator_close, 
//GO(g_file_enumerator_close_async, 
//GO(g_file_enumerator_close_finish, 
//GO(g_file_enumerator_get_child, 
//GO(g_file_enumerator_get_container, 
//GO(g_file_enumerator_get_type, 
//GO(g_file_enumerator_has_pending, 
//GO(g_file_enumerator_is_closed, 
//GO(g_file_enumerator_next_file, 
//GO(g_file_enumerator_next_files_async, 
//GO(g_file_enumerator_next_files_finish, 
//GO(g_file_enumerator_set_pending, 
GO(g_file_equal, iFpp)
GO(g_file_find_enclosing_mount, pFppp)
//GOM(g_file_find_enclosing_mount_async, vFEpipBp)
GO(g_file_find_enclosing_mount_finish, pFppp)
GO(g_file_get_basename, pFp)
GO(g_file_get_child, pFpp)
GO(g_file_get_child_for_display_name, pFppp)
GO(g_file_get_parent, pFp)
GO(g_file_get_parse_name, pFp)
GO(g_file_get_path, pFp)
GO(g_file_get_relative_path, pFpp)
GO(g_file_get_type, LFv)
GO(g_file_get_uri, pFp)
GO(g_file_get_uri_scheme, pFp)
GO(g_file_hash, uFp)
GO(g_file_has_parent, iFpp)
GO(g_file_has_prefix, iFpp)
GO(g_file_has_uri_scheme, iFpp)
GO(g_file_icon_get_file, pFp)
GO(g_file_icon_get_type, LFv)
GO(g_file_icon_new, pFp)
//GO(g_file_info_clear_status, 
//GO(g_file_info_copy_into, 
//GO(g_file_info_dup, 
//GO(g_file_info_get_attribute_as_string, 
//GO(g_file_info_get_attribute_boolean, 
//GO(g_file_info_get_attribute_byte_string, 
//GO(g_file_info_get_attribute_data, 
//GO(g_file_info_get_attribute_int32, 
//GO(g_file_info_get_attribute_int64, 
//GO(g_file_info_get_attribute_object, 
//GO(g_file_info_get_attribute_status, 
//GO(g_file_info_get_attribute_string, 
//GO(g_file_info_get_attribute_stringv, 
//GO(g_file_info_get_attribute_type, 
//GO(g_file_info_get_attribute_uint32, 
//GO(g_file_info_get_attribute_uint64, 
//GO(g_file_info_get_content_type, 
//GO(g_file_info_get_deletion_date, 
//GO(g_file_info_get_display_name, 
//GO(g_file_info_get_edit_name, 
//GO(g_file_info_get_etag, 
//GO(g_file_info_get_file_type, 
//GO(g_file_info_get_icon, 
//GO(g_file_info_get_is_backup, 
//GO(g_file_info_get_is_hidden, 
//GO(g_file_info_get_is_symlink, 
//GO(g_file_info_get_modification_time, 
//GO(g_file_info_get_name, 
//GO(g_file_info_get_size, 
//GO(g_file_info_get_sort_order, 
//GO(g_file_info_get_symbolic_icon, 
//GO(g_file_info_get_symlink_target, 
//GO(g_file_info_get_type, 
//GO(g_file_info_has_attribute, 
//GO(g_file_info_has_namespace, 
//GO(g_file_info_list_attributes, 
//GO(g_file_info_new, 
//GO(g_file_info_remove_attribute, 
//GO(g_file_info_set_attribute, 
//GO(g_file_info_set_attribute_boolean, 
//GO(g_file_info_set_attribute_byte_string, 
//GO(g_file_info_set_attribute_int32, 
//GO(g_file_info_set_attribute_int64, 
//GO(g_file_info_set_attribute_mask, 
//GO(g_file_info_set_attribute_object, 
//GO(g_file_info_set_attribute_status, 
//GO(g_file_info_set_attribute_string, 
//GO(g_file_info_set_attribute_stringv, 
//GO(g_file_info_set_attribute_uint32, 
//GO(g_file_info_set_attribute_uint64, 
//GO(g_file_info_set_content_type, 
//GO(g_file_info_set_display_name, 
//GO(g_file_info_set_edit_name, 
//GO(g_file_info_set_file_type, 
//GO(g_file_info_set_icon, 
//GO(g_file_info_set_is_hidden, 
//GO(g_file_info_set_is_symlink, 
//GO(g_file_info_set_modification_time, 
//GO(g_file_info_set_name, 
//GO(g_file_info_set_size, 
//GO(g_file_info_set_sort_order, 
//GO(g_file_info_set_symbolic_icon, 
//GO(g_file_info_set_symlink_target, 
//GO(g_file_info_unset_attribute_mask, 
//GO(g_file_input_stream_get_type, 
//GO(g_file_input_stream_query_info, 
//GO(g_file_input_stream_query_info_async, 
//GO(g_file_input_stream_query_info_finish, 
//GO(g_file_io_stream_get_etag, 
//GO(g_file_io_stream_get_type, 
//GO(g_file_io_stream_query_info, 
//GO(g_file_io_stream_query_info_async, 
//GO(g_file_io_stream_query_info_finish, 
GO(g_file_is_native, iFpp)
GO(g_file_load_bytes, pFpppp)   //sice 2.56+
//GOM(g_file_load_bytes_async, vFEppBp) //since 2.56+
GO(g_file_load_bytes_finish, pFpppp)    //since 2.56+
GO(g_file_load_contents, iFpppppp)
//GOM(g_file_load_contents_async, vFEppBp)
GO(g_file_load_contents_finish, iFpppppp)
//GOM(g_file_load_partial_contents_async, vFppBBp)
GO(g_file_load_partial_contents_finish, iFpppppp)
GO(g_file_make_directory, iFppp)
//GOM(g_file_make_directory_async, vFEpipBp)
GO(g_file_make_directory_finish, iFppp)
GO(g_file_make_directory_with_parents, iFppp)
GO(g_file_make_symbolic_link, iFpppp)
//GOM(g_file_measure_disk_usage, iFEpipBppppp)
//GOM(g_file_measure_disk_usage_async, vFEpiipBpBp)
GO(g_file_measure_disk_usage_finish, iFpppppp)
//GO(g_file_measure_flags_get_type, 
GO(g_file_monitor, pFpipp)
//GO(g_file_monitor_cancel, 
GO(g_file_monitor_directory, pFpipp)
//GO(g_file_monitor_emit_event, 
//GO(g_file_monitor_event_get_type, 
GO(g_file_monitor_file, pFpipp)
//GO(g_file_monitor_flags_get_type, 
//GO(g_file_monitor_get_type, 
//GO(g_file_monitor_is_cancelled, 
//GO(g_file_monitor_set_rate_limit, 
//GOM(g_file_mount_enclosing_volume, vFEpippBp)
GO(g_file_mount_enclosing_volume_finish, iFppp)
//GOM(g_file_mount_mountable, vFEpippBp)
GO(g_file_mount_mountable_finish, pFppp)
//GOM(g_file_move, iFEppipBpp)
//GO(g_filename_completer_get_completions, 
//GO(g_filename_completer_get_completion_suffix, 
//GO(g_filename_completer_get_type, 
//GO(g_filename_completer_new, 
//GO(g_filename_completer_set_dirs_only, 
GO(g_file_new_build_filename, pFppppppppppp)    // vaarg, since v2.56+
GO(g_file_new_for_commandline_arg, pFp)
GO(g_file_new_for_commandline_arg_and_cwd, pFpp)
GO(g_file_new_for_path, pFp)
GO(g_file_new_for_uri, pFp)
GO(g_file_new_tmp, pFppp)
GO(g_file_open_readwrite, pFppp)
//GOM(g_file_open_readwrite_async, vFEpipBp)
GO(g_file_open_readwrite_finish, pFppp)
//GO(g_file_output_stream_get_etag, 
//GO(g_file_output_stream_get_type, 
//GO(g_file_output_stream_query_info, 
//GO(g_file_output_stream_query_info_async, 
//GO(g_file_output_stream_query_info_finish, 
GO(g_file_parse_name, pFp)
GO(g_file_peek_path , pFp)  // sice 2.56+
//GOM(g_file_poll_mountable, vFEppBp)
GO(g_file_poll_mountable_finish, iFppp)
GO(g_file_query_default_handler, pFppp)
//GOMg_file_query_default_handler_async, vFEpipBp)  //since 2.60+
GO(g_file_query_default_handler_finish, pFppp)      //since 2.60+
GO(g_file_query_exists, iFpp)
GO(g_file_query_filesystem_info, pFpppp)
//GOM(g_file_query_filesystem_info_async, vFEppipBp)
GO(g_file_query_filesystem_info_finish, pFppp)
GO(g_file_query_file_type, iFpip)
GO(g_file_query_info, pFppipp)
//GOM(g_file_query_info_async, vFEppiipBp)
GO(g_file_query_info_finish, pFppp)
//GO(g_file_query_info_flags_get_type, 
GO(g_file_query_settable_attributes, pFppp)
GO(g_file_query_writable_namespaces, pFppp)
GO(g_file_read, pFppp)
//GOM(g_file_read_async, vFEpipBp)
GO(g_file_read_finish, pFppp)
GO(g_file_replace, pFppiipp)
//GOM(g_file_replace_async, vFEppiiipBp)
GO(g_file_replace_contents, iFppLpiippp)
//GOM(g_file_replace_contents_async, vFEppLpuupBp)
//GOM(g_file_replace_contents_bytes_async, vFEpppiipBp)
GO(g_file_replace_contents_finish, iFpppp)
//GO(g_file_replace_finish, 
GO(g_file_replace_readwrite, pFppiipp)
//GOM(g_file_replace_readwrite_async, vFEppiiipBp)
GO(g_file_replace_readwrite_finish, pFppp)
GO(g_file_resolve_relative_path, pFpp)
GO(g_file_set_attribute, iFppipipp)
GO(g_file_set_attribute_byte_string, iFpppipp)
GO(g_file_set_attribute_int32, iFppiipp)
GO(g_file_set_attribute_int64, iFppIipp)
//GOM(g_file_set_attributes_async, vFEppiipBp)
GO(g_file_set_attributes_finish, iFpppp)
GO(g_file_set_attributes_from_info, iFppipp)
GO(g_file_set_attribute_string, iFpppipp)
GO(g_file_set_attribute_uint32, iFppuipp)
GO(g_file_set_attribute_uint64, iFppUipp)
GO(g_file_set_display_name, pFpppp)
//GOM(g_file_set_display_name_async, vFEppipBp)
GO(g_file_set_display_name_finish, pFppp)
//GOM(g_file_start_mountable, vFEpippBp)
GO(g_file_start_mountable_finish, iFppp)
//GOM(g_file_stop_mountable, vFpippBp)
GO(g_file_stop_mountable_finish, iFppp)
GO(g_file_supports_thread_contexts, iFp)
//GO(g_filesystem_preview_type_get_type, 
GO(g_file_trash, iFppp)
//GOM(g_file_trash_async, vFEpipBp)
GO(g_file_trash_finish, pFppp)
GO(g_file_type_get_type, LFv)
//GOM(g_file_unmount_mountable, vFEpipBp)
GO(g_file_unmount_mountable_finish, iFppp)
//GOM(g_file_unmount_mountable_with_operation, vFEpippBp)
GO(g_file_unmount_mountable_with_operation_finish, iFppp)
//GO(g_filter_input_stream_get_base_stream, 
//GO(g_filter_input_stream_get_close_base_stream, 
//GO(g_filter_input_stream_get_type, 
//GO(g_filter_input_stream_set_close_base_stream, 
//GO(g_filter_output_stream_get_base_stream, 
//GO(g_filter_output_stream_get_close_base_stream, 
//GO(g_filter_output_stream_get_type, 
//GO(g_filter_output_stream_set_close_base_stream, 
GO(g_icon_deserialize, pFp)
GO(g_icon_equal, iFpp)
//GO(g_icon_get_type, 
GO(g_icon_hash, uFp)
GO(g_icon_new_for_string, pFpp)
GO(g_icon_serialize, pFp)
GO(g_icon_to_string, pFp)
//GO(g_inet_address_equal, 
//GO(g_inet_address_get_family, 
//GO(g_inet_address_get_is_any, 
//GO(g_inet_address_get_is_link_local, 
//GO(g_inet_address_get_is_loopback, 
//GO(g_inet_address_get_is_mc_global, 
//GO(g_inet_address_get_is_mc_link_local, 
//GO(g_inet_address_get_is_mc_node_local, 
//GO(g_inet_address_get_is_mc_org_local, 
//GO(g_inet_address_get_is_mc_site_local, 
//GO(g_inet_address_get_is_multicast, 
//GO(g_inet_address_get_is_site_local, 
//GO(g_inet_address_get_native_size, 
//GO(g_inet_address_get_type, 
//GO(g_inet_address_mask_equal, 
//GO(g_inet_address_mask_get_address, 
//GO(g_inet_address_mask_get_family, 
//GO(g_inet_address_mask_get_length, 
//GO(g_inet_address_mask_get_type, 
//GO(g_inet_address_mask_matches, 
//GO(g_inet_address_mask_new, 
//GO(g_inet_address_mask_new_from_string, 
//GO(g_inet_address_mask_to_string, 
//GO(g_inet_address_new_any, 
//GO(g_inet_address_new_from_bytes, 
//GO(g_inet_address_new_from_string, 
//GO(g_inet_address_new_loopback, 
//GO(g_inet_address_to_bytes, 
//GO(g_inet_address_to_string, 
//GO(g_inet_socket_address_get_address, 
//GO(g_inet_socket_address_get_flowinfo, 
//GO(g_inet_socket_address_get_port, 
//GO(g_inet_socket_address_get_scope_id, 
//GO(g_inet_socket_address_get_type, 
//GO(g_inet_socket_address_new, 
//GO(g_inet_socket_address_new_from_string, 
GO(g_initable_get_type, LFv)
GO(g_initable_init, iFppp)
GOM(g_initable_new, pFEppppV)
GO(g_initable_newv, pFpuppp)
GOM(g_initable_new_valist, pFEppApp)
//GO(g_input_stream_clear_pending, 
//GO(g_input_stream_close, 
//GO(g_input_stream_close_async, 
//GO(g_input_stream_close_finish, 
//GO(g_input_stream_get_type, 
//GO(g_input_stream_has_pending, 
//GO(g_input_stream_is_closed, 
//GO(g_input_stream_read, 
//GO(g_input_stream_read_all, 
//GO(g_input_stream_read_async, 
//GO(g_input_stream_read_bytes, 
//GO(g_input_stream_read_bytes_async, 
//GO(g_input_stream_read_bytes_finish, 
//GO(g_input_stream_read_finish, 
//GO(g_input_stream_set_pending, 
//GO(g_input_stream_skip, 
//GO(g_input_stream_skip_async, 
//GO(g_input_stream_skip_finish, 
//GO(g_io_error_enum_get_type, 
//GO(g_io_error_from_errno, 
//GO(g_io_error_quark, 
//GO(g_io_extension_get_name, 
//GO(g_io_extension_get_priority, 
//GO(g_io_extension_get_type, 
//GO(g_io_extension_point_get_extension_by_name, 
//GO(g_io_extension_point_get_extensions, 
//GO(g_io_extension_point_get_required_type, 
//GO(g_io_extension_point_implement, 
//GO(g_io_extension_point_lookup, 
//GO(g_io_extension_point_register, 
//GO(g_io_extension_point_set_required_type, 
//GO(g_io_extension_ref_class, 
//GO(g_io_module_get_type, 
//GO(g_io_module_new, 
//GO(g_io_module_scope_block, 
//GO(g_io_module_scope_flags_get_type, 
//GO(g_io_module_scope_free, 
//GO(g_io_module_scope_new, 
//GO(g_io_modules_load_all_in_directory, 
//GO(g_io_modules_load_all_in_directory_with_scope, 
//GO(g_io_modules_scan_all_in_directory, 
//GO(g_io_modules_scan_all_in_directory_with_scope, 
//GO(g_io_scheduler_cancel_all_jobs, 
//GO(g_io_scheduler_job_send_to_mainloop, 
//GO(g_io_scheduler_job_send_to_mainloop_async, 
//GO(g_io_scheduler_push_job, 
//GO(g_io_stream_clear_pending, 
//GO(g_io_stream_close, 
//GO(g_io_stream_close_async, 
//GO(g_io_stream_close_finish, 
//GO(g_io_stream_get_input_stream, 
//GO(g_io_stream_get_output_stream, 
//GO(g_io_stream_get_type, 
//GO(g_io_stream_has_pending, 
//GO(g_io_stream_is_closed, 
//GO(g_io_stream_set_pending, 
//GO(g_io_stream_splice_async, 
//GO(g_io_stream_splice_finish, 
//GO(g_io_stream_splice_flags_get_type, 
//GO(g_keyfile_settings_backend_new, 
GO(g_list_model_get_item, pFpu)
GO(g_list_model_get_n_items, uFp)
GO(g_list_model_get_object, pFpu)
//GO(g_loadable_icon_get_type, 
//GO(g_loadable_icon_load, 
//GO(g_loadable_icon_load_async, 
//GO(g_loadable_icon_load_finish, 
//GO(g_local_directory_monitor_get_type, 
//GO(g_local_file_monitor_get_type, 
//GO(g_memory_input_stream_add_bytes, 
//GO(g_memory_input_stream_add_data, 
//GO(g_memory_input_stream_get_type, 
//GO(g_memory_input_stream_new, 
//GO(g_memory_input_stream_new_from_bytes, 
//GO(g_memory_input_stream_new_from_data, 
//GO(g_memory_output_stream_get_data, 
//GO(g_memory_output_stream_get_data_size, 
//GO(g_memory_output_stream_get_size, 
//GO(g_memory_output_stream_get_type, 
//GO(g_memory_output_stream_new, 
//GO(g_memory_output_stream_new_resizable, 
//GO(g_memory_output_stream_steal_as_bytes, 
//GO(g_memory_output_stream_steal_data, 
//GO(g_memory_settings_backend_new, 
//GO(g_menu_append, 
//GO(g_menu_append_item, 
//GO(g_menu_append_section, 
//GO(g_menu_append_submenu, 
//GO(g_menu_attribute_iter_get_name, 
//GO(g_menu_attribute_iter_get_next, 
//GO(g_menu_attribute_iter_get_type, 
//GO(g_menu_attribute_iter_get_value, 
//GO(g_menu_attribute_iter_next, 
//GO(g_menu_freeze, 
//GO(g_menu_get_type, 
//GO(g_menu_insert, 
//GO(g_menu_insert_item, 
//GO(g_menu_insert_section, 
//GO(g_menu_insert_submenu, 
//GO(g_menu_item_get_attribute, 
//GO(g_menu_item_get_attribute_value, 
//GO(g_menu_item_get_link, 
//GO(g_menu_item_get_type, 
//GO(g_menu_item_new, 
//GO(g_menu_item_new_from_model, 
//GO(g_menu_item_new_section, 
//GO(g_menu_item_new_submenu, 
//GO(g_menu_item_set_action_and_target, 
//GO(g_menu_item_set_action_and_target_value, 
//GO(g_menu_item_set_attribute, 
//GO(g_menu_item_set_attribute_value, 
//GO(g_menu_item_set_detailed_action, 
//GO(g_menu_item_set_icon, 
//GO(g_menu_item_set_label, 
//GO(g_menu_item_set_link, 
//GO(g_menu_item_set_section, 
//GO(g_menu_item_set_submenu, 
//GO(g_menu_link_iter_get_name, 
//GO(g_menu_link_iter_get_next, 
//GO(g_menu_link_iter_get_type, 
//GO(g_menu_link_iter_get_value, 
//GO(g_menu_link_iter_next, 
//GO(g_menu_model_get_item_attribute, 
//GO(g_menu_model_get_item_attribute_value, 
//GO(g_menu_model_get_item_link, 
//GO(g_menu_model_get_n_items, 
//GO(g_menu_model_get_type, 
//GO(g_menu_model_is_mutable, 
//GO(g_menu_model_items_changed, 
//GO(g_menu_model_iterate_item_attributes, 
//GO(g_menu_model_iterate_item_links, 
//GO(g_menu_new, 
//GO(g_menu_prepend, 
//GO(g_menu_prepend_item, 
//GO(g_menu_prepend_section, 
//GO(g_menu_prepend_submenu, 
//GO(g_menu_remove, 
//GO(g_menu_remove_all, 
//GO(g_mount_can_eject, 
//GO(g_mount_can_unmount, 
//GO(g_mount_eject, 
//GO(g_mount_eject_finish, 
//GO(g_mount_eject_with_operation, 
//GO(g_mount_eject_with_operation_finish, 
//GO(g_mount_get_default_location, 
//GO(g_mount_get_drive, 
//GO(g_mount_get_icon, 
//GO(g_mount_get_name, 
//GO(g_mount_get_root, 
//GO(g_mount_get_sort_key, 
//GO(g_mount_get_symbolic_icon, 
//GO(g_mount_get_type, 
//GO(g_mount_get_uuid, 
//GO(g_mount_get_volume, 
//GO(g_mount_guess_content_type, 
//GO(g_mount_guess_content_type_finish, 
//GO(g_mount_guess_content_type_sync, 
//GO(g_mount_is_shadowed, 
//GO(g_mount_mount_flags_get_type, 
//GO(g_mount_operation_get_anonymous, 
//GO(g_mount_operation_get_choice, 
//GO(g_mount_operation_get_domain, 
//GO(g_mount_operation_get_password, 
//GO(g_mount_operation_get_password_save, 
//GO(g_mount_operation_get_type, 
//GO(g_mount_operation_get_username, 
//GO(g_mount_operation_new, 
//GO(g_mount_operation_reply, 
//GO(g_mount_operation_result_get_type, 
//GO(g_mount_operation_set_anonymous, 
//GO(g_mount_operation_set_choice, 
//GO(g_mount_operation_set_domain, 
//GO(g_mount_operation_set_password, 
//GO(g_mount_operation_set_password_save, 
//GO(g_mount_operation_set_username, 
//GO(g_mount_remount, 
//GO(g_mount_remount_finish, 
//GO(g_mount_shadow, 
//GO(g_mount_unmount, 
//GO(g_mount_unmount_finish, 
//GO(g_mount_unmount_flags_get_type, 
//GO(g_mount_unmount_with_operation, 
//GO(g_mount_unmount_with_operation_finish, 
//GO(g_mount_unshadow, 
//GO(g_native_volume_monitor_get_type, 
//GO(g_network_address_get_hostname, 
//GO(g_network_address_get_port, 
//GO(g_network_address_get_scheme, 
//GO(g_network_address_get_type, 
//GO(g_network_address_new, 
//GO(g_network_address_parse, 
//GO(g_network_address_parse_uri, 
//GO(g_networking_init, 
//GO(g_network_monitor_base_add_network, 
//GO(g_network_monitor_base_get_type, 
//GO(g_network_monitor_base_remove_network, 
//GO(g_network_monitor_base_set_networks, 
//GO(g_network_monitor_can_reach, 
//GO(g_network_monitor_can_reach_async, 
//GO(g_network_monitor_can_reach_finish, 
//GO(g_network_monitor_get_default, 
//GO(g_network_monitor_get_network_available, 
//GO(g_network_monitor_get_type, 
//GO(g_network_service_get_domain, 
//GO(g_network_service_get_protocol, 
//GO(g_network_service_get_scheme, 
//GO(g_network_service_get_service, 
//GO(g_network_service_get_type, 
//GO(g_network_service_new, 
//GO(g_network_service_set_scheme, 
//GO(g_notification_add_button, 
//GO(g_notification_add_button_with_target, 
//GO(g_notification_add_button_with_target_value, 
//GO(g_notification_get_type, 
//GO(g_notification_new, 
//GO(g_notification_priority_get_type, 
//GO(g_notification_set_body, 
//GO(g_notification_set_default_action, 
//GO(g_notification_set_default_action_and_target, 
//GO(g_notification_set_default_action_and_target_value, 
//GO(g_notification_set_icon, 
//GO(g_notification_set_priority, 
//GO(g_notification_set_title, 
//GO(g_notification_set_urgent, 
//GO(g_null_settings_backend_new, 
//GO(g_output_stream_clear_pending, 
//GO(g_output_stream_close, 
//GO(g_output_stream_close_async, 
//GO(g_output_stream_close_finish, 
//GO(g_output_stream_flush, 
//GO(g_output_stream_flush_async, 
//GO(g_output_stream_flush_finish, 
//GO(g_output_stream_get_type, 
//GO(g_output_stream_has_pending, 
//GO(g_output_stream_is_closed, 
//GO(g_output_stream_is_closing, 
//GO(g_output_stream_printf, 
//GO(g_output_stream_set_pending, 
//GO(g_output_stream_splice, 
//GO(g_output_stream_splice_async, 
//GO(g_output_stream_splice_finish, 
//GO(g_output_stream_splice_flags_get_type, 
//GO(g_output_stream_vprintf, 
//GO(g_output_stream_write, 
//GO(g_output_stream_write_all, 
//GO(g_output_stream_write_async, 
//GO(g_output_stream_write_bytes, 
//GO(g_output_stream_write_bytes_async, 
//GO(g_output_stream_write_bytes_finish, 
//GO(g_output_stream_write_finish, 
//GO(g_password_save_get_type, 
//GO(g_permission_acquire, 
//GO(g_permission_acquire_async, 
//GO(g_permission_acquire_finish, 
//GO(g_permission_get_allowed, 
//GO(g_permission_get_can_acquire, 
//GO(g_permission_get_can_release, 
//GO(g_permission_get_type, 
//GO(g_permission_impl_update, 
//GO(g_permission_release, 
//GO(g_permission_release_async, 
//GO(g_permission_release_finish, 
//GO(g_pollable_input_stream_can_poll, 
//GO(g_pollable_input_stream_create_source, 
//GO(g_pollable_input_stream_get_type, 
//GO(g_pollable_input_stream_is_readable, 
//GO(g_pollable_input_stream_read_nonblocking, 
//GO(g_pollable_output_stream_can_poll, 
//GO(g_pollable_output_stream_create_source, 
//GO(g_pollable_output_stream_get_type, 
//GO(g_pollable_output_stream_is_writable, 
//GO(g_pollable_output_stream_write_nonblocking, 
//GO(g_pollable_source_new, 
//GO(g_pollable_source_new_full, 
//GO(g_pollable_stream_read, 
//GO(g_pollable_stream_write, 
//GO(g_pollable_stream_write_all, 
//GO(g_property_action_get_type, 
//GO(g_property_action_new, 
//GO(g_proxy_address_enumerator_get_type, 
//GO(g_proxy_address_get_destination_hostname, 
//GO(g_proxy_address_get_destination_port, 
//GO(g_proxy_address_get_destination_protocol, 
//GO(g_proxy_address_get_password, 
//GO(g_proxy_address_get_protocol, 
//GO(g_proxy_address_get_type, 
//GO(g_proxy_address_get_uri, 
//GO(g_proxy_address_get_username, 
//GO(g_proxy_address_new, 
//GO(g_proxy_connect, 
//GO(g_proxy_connect_async, 
//GO(g_proxy_connect_finish, 
//GO(g_proxy_get_default_for_protocol, 
//GO(g_proxy_get_type, 
//GO(g_proxy_resolver_get_default, 
//GO(g_proxy_resolver_get_type, 
//GO(g_proxy_resolver_is_supported, 
//GO(g_proxy_resolver_lookup, 
//GO(g_proxy_resolver_lookup_async, 
//GO(g_proxy_resolver_lookup_finish, 
//GO(g_proxy_supports_hostname, 
//GO(g_remote_action_group_activate_action_full, 
//GO(g_remote_action_group_change_action_state_full, 
//GO(g_remote_action_group_get_type, 
//GO(g_resolver_error_get_type, 
//GO(g_resolver_error_quark, 
//GO(g_resolver_free_addresses, 
//GO(g_resolver_free_targets, 
//GO(g_resolver_get_default, 
//GO(g_resolver_get_type, 
//GO(g_resolver_lookup_by_address, 
//GO(g_resolver_lookup_by_address_async, 
//GO(g_resolver_lookup_by_address_finish, 
//GO(g_resolver_lookup_by_name, 
//GO(g_resolver_lookup_by_name_async, 
//GO(g_resolver_lookup_by_name_finish, 
//GO(g_resolver_lookup_records, 
//GO(g_resolver_lookup_records_async, 
//GO(g_resolver_lookup_records_finish, 
//GO(g_resolver_lookup_service, 
//GO(g_resolver_lookup_service_async, 
//GO(g_resolver_lookup_service_finish, 
//GO(g_resolver_record_type_get_type, 
//GO(g_resolver_set_default, 
//GO(g_resource_enumerate_children, 
//GO(g_resource_error_get_type, 
//GO(g_resource_error_quark, 
//GO(g_resource_flags_get_type, 
//GO(g_resource_get_info, 
//GO(g_resource_get_type, 
//GO(g_resource_load, 
//GO(g_resource_lookup_data, 
//GO(g_resource_lookup_flags_get_type, 
//GO(g_resource_new_from_data, 
//GO(g_resource_open_stream, 
//GO(g_resource_ref, 
//GO(g_resources_enumerate_children, 
//GO(g_resources_get_info, 
//GO(g_resources_lookup_data, 
//GO(g_resources_open_stream, 
//GO(g_resources_register, 
//GO(g_resources_unregister, 
//GO(g_resource_unref, 
//GO(g_seekable_can_seek, 
//GO(g_seekable_can_truncate, 
//GO(g_seekable_get_type, 
//GO(g_seekable_seek, 
//GO(g_seekable_tell, 
//GO(g_seekable_truncate, 
GO(g_settings_apply, vFp)
//GO(g_settings_backend_changed, 
//GO(g_settings_backend_changed_tree, 
//GO(g_settings_backend_flatten_tree, 
//GO(g_settings_backend_get_default, 
//GO(g_settings_backend_get_type, 
//GO(g_settings_backend_keys_changed, 
//GO(g_settings_backend_path_changed, 
//GO(g_settings_backend_path_writable_changed, 
//GO(g_settings_backend_writable_changed, 
//GO(g_settings_bind, 
//GO(g_settings_bind_flags_get_type, 
//GO(g_settings_bind_with_mapping, 
//GO(g_settings_bind_writable, 
//GO(g_settings_create_action, 
GO(g_settings_delay, vFp)
//GO(g_settings_get, 
//GO(g_settings_get_boolean, 
GO(g_settings_get_child, pFpp)
GO(g_settings_get_default_value, pFpp)
//GO(g_settings_get_double, 
//GO(g_settings_get_enum, 
//GO(g_settings_get_flags, 
GO(g_settings_get_has_unapplied, iFp)
//GO(g_settings_get_int, 
//GO(g_settings_get_mapped, 
//GO(g_settings_get_range, 
//GO(g_settings_get_string, 
//GO(g_settings_get_strv, 
//GO(g_settings_get_type, 
//GO(g_settings_get_uint, 
GO(g_settings_get_user_value, pFpp)
GO(g_settings_get_value, pFpp)
GO(g_settings_is_writable, iFpp)
//GO(g_settings_list_children, 
GO(g_settings_list_keys, pFp)
//GO(g_settings_list_relocatable_schemas, 
//GO(g_settings_list_schemas, 
GO(g_settings_new, pFp)
//GO(g_settings_new_full, 
//GO(g_settings_new_with_backend, 
//GO(g_settings_new_with_backend_and_path, 
GO(g_settings_new_with_path, pFpp)
//GO(g_settings_range_check, 
GO(g_settings_reset, vFpp)
//GO(g_settings_revert, 
//GO(g_settings_schema_get_id, 
//GO(g_settings_schema_get_key, 
//GO(g_settings_schema_get_path, 
//GO(g_settings_schema_get_type, 
//GO(g_settings_schema_has_key, 
//GO(g_settings_schema_key_get_default_value, 
//GO(g_settings_schema_key_get_description, 
//GO(g_settings_schema_key_get_range, 
//GO(g_settings_schema_key_get_summary, 
//GO(g_settings_schema_key_get_type, 
//GO(g_settings_schema_key_get_value_type, 
//GO(g_settings_schema_key_range_check, 
//GO(g_settings_schema_key_ref, 
//GO(g_settings_schema_key_unref, 
//GO(g_settings_schema_ref, 
GO(g_settings_schema_source_get_default, pFv)
GO(g_settings_schema_source_get_type, LFv)
//GO(g_settings_schema_source_list_schemas, 
//GO(g_settings_schema_source_lookup, 
//GO(g_settings_schema_source_new_from_directory, 
GO(g_settings_schema_source_ref, pFp)
GO(g_settings_schema_source_unref, vFp)
//GO(g_settings_schema_unref, 
//GO(g_settings_set, 
//GO(g_settings_set_boolean, 
//GO(g_settings_set_double, 
//GO(g_settings_set_enum, 
//GO(g_settings_set_flags, 
//GO(g_settings_set_int, 
//GO(g_settings_set_string, 
//GO(g_settings_set_strv, 
//GO(g_settings_set_uint, 
GO(g_settings_set_value, iFppp)
GO(g_settings_sync, vFv)
//GO(g_settings_unbind, 
//GO(g_simple_action_get_type, 
//GO(g_simple_action_group_add_entries, 
//GO(g_simple_action_group_get_type, 
//GO(g_simple_action_group_insert, 
//GO(g_simple_action_group_lookup, 
//GO(g_simple_action_group_new, 
//GO(g_simple_action_group_remove, 
//GO(g_simple_action_new, 
//GO(g_simple_action_new_stateful, 
//GO(g_simple_action_set_enabled, 
//GO(g_simple_action_set_state, 
GOM(g_simple_async_report_error_in_idle, vFEpppuipV)
GOM(g_simple_async_report_gerror_in_idle, vFEpppp)
GOM(g_simple_async_report_take_gerror_in_idle, vFEpppp)
GO(g_simple_async_result_complete, vFp)
GO(g_simple_async_result_complete_in_idle, vFp)
GO(g_simple_async_result_get_op_res_gboolean, iFp)
GO(g_simple_async_result_get_op_res_gpointer, pFp)
GO(g_simple_async_result_get_op_res_gssize, lFp)
GO(g_simple_async_result_get_source_tag, pFp)
GO(g_simple_async_result_get_type, LFv)
GO(g_simple_async_result_is_valid, iFppp)
GOM(g_simple_async_result_new, pFEpppp)
GOM(g_simple_async_result_new_error, pFEpppuipV)
GOM(g_simple_async_result_new_from_error, pFEpppp)
GOM(g_simple_async_result_new_take_error, pFEpppp)
GO(g_simple_async_result_propagate_error, iFpp)
GOM(g_simple_async_result_run_in_thread, vFEppip)
GO(g_simple_async_result_set_check_cancellable, vFpp)
GOM(g_simple_async_result_set_error, vFEppipV)
GOM(g_simple_async_result_set_error_va, vFEppipA)
GO(g_simple_async_result_set_from_error, vFpp)
GO(g_simple_async_result_set_handle_cancellation, vFpi)
GO(g_simple_async_result_set_op_res_gboolean, vFpi)
GOM(g_simple_async_result_set_op_res_gpointer, vFEppp)
GO(g_simple_async_result_set_op_res_gssize, vFpl)
GO(g_simple_async_result_take_error, vFpp)
GO(g_simple_permission_get_type, LFv)
//GO(g_simple_permission_new, 
GO(g_simple_proxy_resolver_get_type, LFv)
//GO(g_simple_proxy_resolver_new, 
//GO(g_simple_proxy_resolver_set_default_proxy, 
//GO(g_simple_proxy_resolver_set_ignore_hosts, 
//GO(g_simple_proxy_resolver_set_uri_proxy, 
//GO(g_socket_accept, 
//GO(g_socket_address_enumerator_get_type, 
//GO(g_socket_address_enumerator_next, 
//GO(g_socket_address_enumerator_next_async, 
//GO(g_socket_address_enumerator_next_finish, 
//GO(g_socket_address_get_family, 
//GO(g_socket_address_get_native_size, 
//GO(g_socket_address_get_type, 
//GO(g_socket_address_new_from_native, 
//GO(g_socket_address_to_native, 
//GO(g_socket_bind, 
//GO(g_socket_check_connect_result, 
//GO(g_socket_client_add_application_proxy, 
//GO(g_socket_client_connect, 
//GO(g_socket_client_connect_async, 
//GO(g_socket_client_connect_finish, 
//GO(g_socket_client_connect_to_host, 
//GO(g_socket_client_connect_to_host_async, 
//GO(g_socket_client_connect_to_host_finish, 
//GO(g_socket_client_connect_to_service, 
//GO(g_socket_client_connect_to_service_async, 
//GO(g_socket_client_connect_to_service_finish, 
//GO(g_socket_client_connect_to_uri, 
//GO(g_socket_client_connect_to_uri_async, 
//GO(g_socket_client_connect_to_uri_finish, 
//GO(g_socket_client_event_get_type, 
//GO(g_socket_client_get_enable_proxy, 
//GO(g_socket_client_get_family, 
//GO(g_socket_client_get_local_address, 
//GO(g_socket_client_get_protocol, 
//GO(g_socket_client_get_proxy_resolver, 
//GO(g_socket_client_get_socket_type, 
//GO(g_socket_client_get_timeout, 
//GO(g_socket_client_get_tls, 
//GO(g_socket_client_get_tls_validation_flags, 
//GO(g_socket_client_get_type, 
//GO(g_socket_client_new, 
//GO(g_socket_client_set_enable_proxy, 
//GO(g_socket_client_set_family, 
//GO(g_socket_client_set_local_address, 
//GO(g_socket_client_set_protocol, 
//GO(g_socket_client_set_proxy_resolver, 
//GO(g_socket_client_set_socket_type, 
//GO(g_socket_client_set_timeout, 
//GO(g_socket_client_set_tls, 
//GO(g_socket_client_set_tls_validation_flags, 
//GO(g_socket_close, 
//GO(g_socket_condition_check, 
//GO(g_socket_condition_timed_wait, 
//GO(g_socket_condition_wait, 
//GO(g_socket_connect, 
//GO(g_socket_connectable_enumerate, 
//GO(g_socket_connectable_get_type, 
//GO(g_socket_connectable_proxy_enumerate, 
//GO(g_socket_connection_connect, 
//GO(g_socket_connection_connect_async, 
//GO(g_socket_connection_connect_finish, 
//GO(g_socket_connection_factory_create_connection, 
//GO(g_socket_connection_factory_lookup_type, 
//GO(g_socket_connection_factory_register_type, 
//GO(g_socket_connection_get_local_address, 
//GO(g_socket_connection_get_remote_address, 
//GO(g_socket_connection_get_socket, 
//GO(g_socket_connection_get_type, 
//GO(g_socket_connection_is_connected, 
//GO(g_socket_control_message_deserialize, 
//GO(g_socket_control_message_get_level, 
//GO(g_socket_control_message_get_msg_type, 
//GO(g_socket_control_message_get_size, 
//GO(g_socket_control_message_get_type, 
//GO(g_socket_control_message_serialize, 
//GO(g_socket_create_source, 
//GO(g_socket_family_get_type, 
//GO(g_socket_get_available_bytes, 
//GO(g_socket_get_blocking, 
//GO(g_socket_get_broadcast, 
//GO(g_socket_get_credentials, 
//GO(g_socket_get_family, 
//GO(g_socket_get_fd, 
//GO(g_socket_get_keepalive, 
//GO(g_socket_get_listen_backlog, 
//GO(g_socket_get_local_address, 
//GO(g_socket_get_multicast_loopback, 
//GO(g_socket_get_multicast_ttl, 
//GO(g_socket_get_option, 
//GO(g_socket_get_protocol, 
//GO(g_socket_get_remote_address, 
//GO(g_socket_get_socket_type, 
//GO(g_socket_get_timeout, 
//GO(g_socket_get_ttl, 
//GO(g_socket_get_type, 
//GO(g_socket_is_closed, 
//GO(g_socket_is_connected, 
//GO(g_socket_join_multicast_group, 
//GO(g_socket_leave_multicast_group, 
//GO(g_socket_listen, 
//GO(g_socket_listener_accept, 
//GO(g_socket_listener_accept_async, 
//GO(g_socket_listener_accept_finish, 
//GO(g_socket_listener_accept_socket, 
//GO(g_socket_listener_accept_socket_async, 
//GO(g_socket_listener_accept_socket_finish, 
//GO(g_socket_listener_add_address, 
//GO(g_socket_listener_add_any_inet_port, 
//GO(g_socket_listener_add_inet_port, 
//GO(g_socket_listener_add_socket, 
//GO(g_socket_listener_close, 
//GO(g_socket_listener_get_type, 
//GO(g_socket_listener_new, 
//GO(g_socket_listener_set_backlog, 
//GO(g_socket_msg_flags_get_type, 
//GO(g_socket_new, 
//GO(g_socket_new_from_fd, 
//GO(g_socket_protocol_get_type, 
//GO(g_socket_receive, 
//GO(g_socket_receive_from, 
//GO(g_socket_receive_message, 
//GO(g_socket_receive_with_blocking, 
//GO(g_socket_send, 
//GO(g_socket_send_message, 
//GO(g_socket_send_to, 
//GO(g_socket_send_with_blocking, 
//GO(g_socket_service_get_type, 
//GO(g_socket_service_is_active, 
//GO(g_socket_service_new, 
//GO(g_socket_service_start, 
//GO(g_socket_service_stop, 
//GO(g_socket_set_blocking, 
//GO(g_socket_set_broadcast, 
//GO(g_socket_set_keepalive, 
//GO(g_socket_set_listen_backlog, 
//GO(g_socket_set_multicast_loopback, 
//GO(g_socket_set_multicast_ttl, 
//GO(g_socket_set_option, 
//GO(g_socket_set_timeout, 
//GO(g_socket_set_ttl, 
//GO(g_socket_shutdown, 
//GO(g_socket_speaks_ipv4, 
//GO(g_socket_type_get_type, 
//GO(g_srv_target_copy, 
//GO(g_srv_target_free, 
//GO(g_srv_target_get_hostname, 
//GO(g_srv_target_get_port, 
//GO(g_srv_target_get_priority, 
//GO(g_srv_target_get_type, 
//GO(g_srv_target_get_weight, 
//GO(g_srv_target_list_sort, 
//GO(g_srv_target_new, 
//GO(g_static_resource_fini, 
//GO(g_static_resource_get_resource, 
//GO(g_static_resource_init, 
//GO(g_subprocess_communicate, 
//GO(g_subprocess_communicate_async, 
//GO(g_subprocess_communicate_finish, 
//GO(g_subprocess_communicate_utf8, 
//GO(g_subprocess_communicate_utf8_async, 
//GO(g_subprocess_communicate_utf8_finish, 
//GO(g_subprocess_flags_get_type, 
//GO(g_subprocess_force_exit, 
//GO(g_subprocess_get_exit_status, 
//GO(g_subprocess_get_identifier, 
//GO(g_subprocess_get_if_exited, 
//GO(g_subprocess_get_if_signaled, 
//GO(g_subprocess_get_status, 
//GO(g_subprocess_get_stderr_pipe, 
//GO(g_subprocess_get_stdin_pipe, 
//GO(g_subprocess_get_stdout_pipe, 
//GO(g_subprocess_get_successful, 
//GO(g_subprocess_get_term_sig, 
//GO(g_subprocess_get_type, 
//GO(g_subprocess_launcher_getenv, 
//GO(g_subprocess_launcher_get_type, 
//GO(g_subprocess_launcher_new, 
//GO(g_subprocess_launcher_set_child_setup, 
//GO(g_subprocess_launcher_set_cwd, 
//GO(g_subprocess_launcher_setenv, 
//GO(g_subprocess_launcher_set_environ, 
//GO(g_subprocess_launcher_set_flags, 
//GO(g_subprocess_launcher_set_stderr_file_path, 
//GO(g_subprocess_launcher_set_stdin_file_path, 
//GO(g_subprocess_launcher_set_stdout_file_path, 
//GO(g_subprocess_launcher_spawn, 
//GO(g_subprocess_launcher_spawnv, 
//GO(g_subprocess_launcher_take_fd, 
//GO(g_subprocess_launcher_take_stderr_fd, 
//GO(g_subprocess_launcher_take_stdin_fd, 
//GO(g_subprocess_launcher_take_stdout_fd, 
//GO(g_subprocess_launcher_unsetenv, 
//GO(g_subprocess_new, 
//GO(g_subprocess_newv, 
//GO(g_subprocess_send_signal, 
//GO(g_subprocess_wait, 
//GO(g_subprocess_wait_async, 
//GO(g_subprocess_wait_check, 
//GO(g_subprocess_wait_check_async, 
//GO(g_subprocess_wait_check_finish, 
//GO(g_subprocess_wait_finish, 
//GO(g_task_attach_source, 
//GO(g_task_get_cancellable, 
//GO(g_task_get_check_cancellable, 
//GO(g_task_get_context, 
//GO(g_task_get_priority, 
//GO(g_task_get_return_on_cancel, 
//GO(g_task_get_source_object, 
//GO(g_task_get_source_tag, 
//GO(g_task_get_task_data, 
//GO(g_task_get_type, 
//GO(g_task_had_error, 
//GO(g_task_is_valid, 
//GO(g_task_new, 
//GO(g_task_propagate_boolean, 
//GO(g_task_propagate_int, 
//GO(g_task_propagate_pointer, 
//GO(g_task_report_error, 
//GO(g_task_report_new_error, 
//GO(g_task_return_boolean, 
//GO(g_task_return_error, 
//GO(g_task_return_error_if_cancelled, 
//GO(g_task_return_int, 
//GO(g_task_return_new_error, 
//GO(g_task_return_pointer, 
//GO(g_task_run_in_thread, 
//GO(g_task_run_in_thread_sync, 
//GO(g_task_set_check_cancellable, 
//GO(g_task_set_priority, 
//GO(g_task_set_return_on_cancel, 
//GO(g_task_set_source_tag, 
//GO(g_task_set_task_data, 
//GO(g_tcp_connection_get_graceful_disconnect, 
//GO(g_tcp_connection_get_type, 
//GO(g_tcp_connection_set_graceful_disconnect, 
//GO(g_tcp_wrapper_connection_get_base_io_stream, 
//GO(g_tcp_wrapper_connection_get_type, 
//GO(g_tcp_wrapper_connection_new, 
//GO(g_test_dbus_add_service_dir, 
//GO(g_test_dbus_down, 
//GO(g_test_dbus_flags_get_type, 
//GO(g_test_dbus_get_bus_address, 
//GO(g_test_dbus_get_flags, 
//GO(g_test_dbus_get_type, 
//GO(g_test_dbus_new, 
//GO(g_test_dbus_stop, 
//GO(g_test_dbus_unset, 
//GO(g_test_dbus_up, 
GO(g_themed_icon_append_name, vFpp)
GO(g_themed_icon_get_names, pFp)
GO(g_themed_icon_get_type, LFv)
GO(g_themed_icon_new, pFp)
GO(g_themed_icon_new_from_names, pFpi)
GO(g_themed_icon_new_with_default_fallbacks, pFp)
GO(g_themed_icon_prepend_name, vFpp)
//GO(g_threaded_resolver_get_type, 
//GO(g_threaded_socket_service_get_type, 
//GO(g_threaded_socket_service_new, 
//GO(g_tls_authentication_mode_get_type, 
//GO(g_tls_backend_get_certificate_type, 
//GO(g_tls_backend_get_client_connection_type, 
//GO(g_tls_backend_get_default, 
//GO(g_tls_backend_get_default_database, 
//GO(g_tls_backend_get_file_database_type, 
//GO(g_tls_backend_get_server_connection_type, 
//GO(g_tls_backend_get_type, 
//GO(g_tls_backend_supports_tls, 
//GO(g_tls_certificate_flags_get_type, 
//GO(g_tls_certificate_get_issuer, 
//GO(g_tls_certificate_get_type, 
//GO(g_tls_certificate_is_same, 
//GO(g_tls_certificate_list_new_from_file, 
//GO(g_tls_certificate_new_from_file, 
//GO(g_tls_certificate_new_from_files, 
//GO(g_tls_certificate_new_from_pem, 
//GO(g_tls_certificate_request_flags_get_type, 
//GO(g_tls_certificate_verify, 
//GO(g_tls_client_connection_get_accepted_cas, 
//GO(g_tls_client_connection_get_server_identity, 
//GO(g_tls_client_connection_get_type, 
//GO(g_tls_client_connection_get_use_ssl3, 
//GO(g_tls_client_connection_get_validation_flags, 
//GO(g_tls_client_connection_new, 
//GO(g_tls_client_connection_set_server_identity, 
//GO(g_tls_client_connection_set_use_ssl3, 
//GO(g_tls_client_connection_set_validation_flags, 
//GO(g_tls_connection_emit_accept_certificate, 
//GO(g_tls_connection_get_certificate, 
//GO(g_tls_connection_get_database, 
//GO(g_tls_connection_get_interaction, 
//GO(g_tls_connection_get_peer_certificate, 
//GO(g_tls_connection_get_peer_certificate_errors, 
//GO(g_tls_connection_get_rehandshake_mode, 
//GO(g_tls_connection_get_require_close_notify, 
//GO(g_tls_connection_get_type, 
//GO(g_tls_connection_get_use_system_certdb, 
//GO(g_tls_connection_handshake, 
//GO(g_tls_connection_handshake_async, 
//GO(g_tls_connection_handshake_finish, 
//GO(g_tls_connection_set_certificate, 
//GO(g_tls_connection_set_database, 
//GO(g_tls_connection_set_interaction, 
//GO(g_tls_connection_set_rehandshake_mode, 
//GO(g_tls_connection_set_require_close_notify, 
//GO(g_tls_connection_set_use_system_certdb, 
//GO(g_tls_database_create_certificate_handle, 
//GO(g_tls_database_get_type, 
//GO(g_tls_database_lookup_certificate_for_handle, 
//GO(g_tls_database_lookup_certificate_for_handle_async, 
//GO(g_tls_database_lookup_certificate_for_handle_finish, 
//GO(g_tls_database_lookup_certificate_issuer, 
//GO(g_tls_database_lookup_certificate_issuer_async, 
//GO(g_tls_database_lookup_certificate_issuer_finish, 
//GO(g_tls_database_lookup_certificates_issued_by, 
//GO(g_tls_database_lookup_certificates_issued_by_async, 
//GO(g_tls_database_lookup_certificates_issued_by_finish, 
//GO(g_tls_database_lookup_flags_get_type, 
//GO(g_tls_database_verify_chain, 
//GO(g_tls_database_verify_chain_async, 
//GO(g_tls_database_verify_chain_finish, 
//GO(g_tls_database_verify_flags_get_type, 
//GO(g_tls_error_get_type, 
//GO(g_tls_error_quark, 
//GO(g_tls_file_database_get_type, 
//GO(g_tls_file_database_new, 
//GO(g_tls_interaction_ask_password, 
//GO(g_tls_interaction_ask_password_async, 
//GO(g_tls_interaction_ask_password_finish, 
//GO(g_tls_interaction_get_type, 
//GO(g_tls_interaction_invoke_ask_password, 
//GO(g_tls_interaction_invoke_request_certificate, 
//GO(g_tls_interaction_request_certificate, 
//GO(g_tls_interaction_request_certificate_async, 
//GO(g_tls_interaction_request_certificate_finish, 
//GO(g_tls_interaction_result_get_type, 
//GO(g_tls_password_flags_get_type, 
//GO(g_tls_password_get_description, 
//GO(g_tls_password_get_flags, 
//GO(g_tls_password_get_type, 
//GO(g_tls_password_get_value, 
//GO(g_tls_password_get_warning, 
//GO(g_tls_password_new, 
//GO(g_tls_password_set_description, 
//GO(g_tls_password_set_flags, 
//GO(g_tls_password_set_value, 
//GO(g_tls_password_set_value_full, 
//GO(g_tls_password_set_warning, 
//GO(g_tls_rehandshake_mode_get_type, 
//GO(g_tls_server_connection_get_type, 
//GO(g_tls_server_connection_new, 
//GO(g_unix_connection_get_type, 
//GO(g_unix_connection_receive_credentials, 
//GO(g_unix_connection_receive_credentials_async, 
//GO(g_unix_connection_receive_credentials_finish, 
//GO(g_unix_connection_receive_fd, 
//GO(g_unix_connection_send_credentials, 
//GO(g_unix_connection_send_credentials_async, 
//GO(g_unix_connection_send_credentials_finish, 
//GO(g_unix_connection_send_fd, 
//GO(g_unix_credentials_message_get_credentials, 
//GO(g_unix_credentials_message_get_type, 
//GO(g_unix_credentials_message_is_supported, 
//GO(g_unix_credentials_message_new, 
//GO(g_unix_credentials_message_new_with_credentials, 
//GO(g_unix_fd_list_append, 
//GO(g_unix_fd_list_get, 
//GO(g_unix_fd_list_get_length, 
GO(g_unix_fd_list_get_type, LFv)
//GO(g_unix_fd_list_new, 
//GO(g_unix_fd_list_new_from_array, 
//GO(g_unix_fd_list_peek_fds, 
//GO(g_unix_fd_list_steal_fds, 
//GO(g_unix_fd_message_append_fd, 
//GO(g_unix_fd_message_get_fd_list, 
//GO(g_unix_fd_message_get_type, 
//GO(g_unix_fd_message_new, 
//GO(g_unix_fd_message_new_with_fd_list, 
//GO(g_unix_fd_message_steal_fds, 
//GO(g_unix_input_stream_get_close_fd, 
//GO(g_unix_input_stream_get_fd, 
//GO(g_unix_input_stream_get_type, 
//GO(g_unix_input_stream_new, 
//GO(g_unix_input_stream_set_close_fd, 
//GO(g_unix_is_mount_path_system_internal, 
//GO(g_unix_mount_at, 
//GO(g_unix_mount_compare, 
//GO(g_unix_mount_free, 
//GO(g_unix_mount_get_device_path, 
//GO(g_unix_mount_get_fs_type, 
//GO(g_unix_mount_get_mount_path, 
//GO(g_unix_mount_guess_can_eject, 
//GO(g_unix_mount_guess_icon, 
//GO(g_unix_mount_guess_name, 
//GO(g_unix_mount_guess_should_display, 
//GO(g_unix_mount_guess_symbolic_icon, 
//GO(g_unix_mount_is_readonly, 
//GO(g_unix_mount_is_system_internal, 
//GO(g_unix_mount_monitor_get_type, 
//GO(g_unix_mount_monitor_new, 
//GO(g_unix_mount_monitor_set_rate_limit, 
//GO(g_unix_mount_point_compare, 
//GO(g_unix_mount_point_free, 
//GO(g_unix_mount_point_get_device_path, 
//GO(g_unix_mount_point_get_fs_type, 
//GO(g_unix_mount_point_get_mount_path, 
//GO(g_unix_mount_point_get_options, 
//GO(g_unix_mount_point_guess_can_eject, 
//GO(g_unix_mount_point_guess_icon, 
//GO(g_unix_mount_point_guess_name, 
//GO(g_unix_mount_point_guess_symbolic_icon, 
//GO(g_unix_mount_point_is_loopback, 
//GO(g_unix_mount_point_is_readonly, 
//GO(g_unix_mount_point_is_user_mountable, 
//GO(g_unix_mount_points_changed_since, 
//GO(g_unix_mount_points_get, 
//GO(g_unix_mounts_changed_since, 
//GO(g_unix_mounts_get, 
//GO(g_unix_output_stream_get_close_fd, 
//GO(g_unix_output_stream_get_fd, 
//GO(g_unix_output_stream_get_type, 
//GO(g_unix_output_stream_new, 
//GO(g_unix_output_stream_set_close_fd, 
//GO(g_unix_socket_address_abstract_names_supported, 
//GO(g_unix_socket_address_get_address_type, 
//GO(g_unix_socket_address_get_is_abstract, 
//GO(g_unix_socket_address_get_path, 
//GO(g_unix_socket_address_get_path_len, 
//GO(g_unix_socket_address_get_type, 
//GO(g_unix_socket_address_new, 
//GO(g_unix_socket_address_new_abstract, 
//GO(g_unix_socket_address_new_with_type, 
//GO(g_unix_socket_address_type_get_type, 
//GO(g_vfs_get_default, 
//GO(g_vfs_get_file_for_path, 
//GO(g_vfs_get_file_for_uri, 
//GO(g_vfs_get_local, 
//GO(g_vfs_get_supported_uri_schemes, 
//GO(g_vfs_get_type, 
//GO(g_vfs_is_active, 
//GO(g_vfs_parse_name, 
//GO(g_volume_can_eject, 
//GO(g_volume_can_mount, 
//GO(g_volume_eject, 
//GO(g_volume_eject_finish, 
//GO(g_volume_eject_with_operation, 
//GO(g_volume_eject_with_operation_finish, 
//GO(g_volume_enumerate_identifiers, 
//GO(g_volume_get_activation_root, 
//GO(g_volume_get_drive, 
//GO(g_volume_get_icon, 
//GO(g_volume_get_identifier, 
//GO(g_volume_get_mount, 
//GO(g_volume_get_name, 
//GO(g_volume_get_sort_key, 
//GO(g_volume_get_symbolic_icon, 
//GO(g_volume_get_type, 
//GO(g_volume_get_uuid, 
//GO(g_volume_monitor_adopt_orphan_mount, 
//GO(g_volume_monitor_get, 
//GO(g_volume_monitor_get_connected_drives, 
//GO(g_volume_monitor_get_mount_for_uuid, 
//GO(g_volume_monitor_get_mounts, 
//GO(g_volume_monitor_get_type, 
//GO(g_volume_monitor_get_volume_for_uuid, 
//GO(g_volume_monitor_get_volumes, 
//GO(g_volume_mount, 
//GO(g_volume_mount_finish, 
//GO(g_volume_should_automount, 
//GO(g_zlib_compressor_format_get_type, 
//GO(g_zlib_compressor_get_file_info, 
//GO(g_zlib_compressor_get_type, 
//GO(g_zlib_compressor_new, 
//GO(g_zlib_compressor_set_file_info, 
//GO(g_zlib_decompressor_get_file_info, 
//GO(g_zlib_decompressor_get_type, 
//GO(g_zlib_decompressor_new, 
//GO(_init, 
