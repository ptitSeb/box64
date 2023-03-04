#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//GO(ibus_accelerator_name, 
//GO(ibus_accelerator_parse, 
//GO(ibus_accelerator_valid, 
//GO(ibus_attr_background_new, 
//GO(ibus_attr_foreground_new, 
//GO(ibus_attribute_get_attr_type, 
//GO(ibus_attribute_get_end_index, 
//GO(ibus_attribute_get_start_index, 
//GO(ibus_attribute_get_type, 
//GO(ibus_attribute_get_value, 
//GO(ibus_attribute_new, 
//GO(ibus_attr_list_append, 
//GO(ibus_attr_list_get, 
//GO(ibus_attr_list_get_type, 
//GO(ibus_attr_list_new, 
GO(ibus_attr_type_get_type, LFv)
GO(ibus_attr_underline_get_type, LFv)
//GO(ibus_attr_underline_new, 
//GO(ibus_bus_add_match, 
//GO(ibus_bus_add_match_async, 
//GO(ibus_bus_add_match_async_finish, 
//GO(ibus_bus_create_input_context, 
GOM(ibus_bus_create_input_context_async, vFEppippp)
GO(ibus_bus_create_input_context_async_finish, pFppp)
//GO(ibus_bus_current_input_context, 
GOM(ibus_bus_current_input_context_async, vFEpippp)
GO(ibus_bus_current_input_context_async_finish, pFppp)
//GO(ibus_bus_exit, 
//GO(ibus_bus_exit_async, 
//GO(ibus_bus_exit_async_finish, 
GO(ibus_bus_get_config, pFp)
//GO(ibus_bus_get_connection, 
GO(ibus_bus_get_engines_by_names, pFpp)
//GO(ibus_bus_get_global_engine, 
GOM(ibus_bus_get_global_engine_async, vFEpippp)
GO(ibus_bus_get_global_engine_async_finish, pFppp)
//GO(ibus_bus_get_ibus_property, 
//GO(ibus_bus_get_ibus_property_async, 
//GO(ibus_bus_get_ibus_property_async_finish, 
//GO(ibus_bus_get_name_owner, 
//GO(ibus_bus_get_name_owner_async, 
//GO(ibus_bus_get_name_owner_async_finish, 
//GO(ibus_bus_get_service_name, 
//GO(ibus_bus_get_type, 
//GO(ibus_bus_get_use_global_engine, 
//GO(ibus_bus_get_use_global_engine_async, 
//GO(ibus_bus_get_use_global_engine_async_finish, 
//GO(ibus_bus_get_use_sys_layout, 
//GO(ibus_bus_get_use_sys_layout_async, 
//GO(ibus_bus_get_use_sys_layout_async_finish, 
GO(ibus_bus_hello, pFp)
GO(ibus_bus_is_connected, iFp)
//GO(ibus_bus_is_global_engine_enabled, 
//GO(ibus_bus_is_global_engine_enabled_async, 
//GO(ibus_bus_is_global_engine_enabled_async_finish, 
//GO(ibus_bus_list_active_engines, 
GOM(ibus_bus_list_active_engines_async, vFEpippp)
GO(ibus_bus_list_active_engines_async_finish, pFppp)
//GO(ibus_bus_list_engines, 
GOM(ibus_bus_list_engines_async, vFEpippp)
GO(ibus_bus_list_engines_async_finish, pFppp)
//GO(ibus_bus_list_names, 
//GO(ibus_bus_list_queued_owners, 
GO(ibus_bus_name_flag_get_type, LFv)
//GO(ibus_bus_name_has_owner, 
//GO(ibus_bus_name_has_owner_async, 
//GO(ibus_bus_name_has_owner_async_finish, 
GO(ibus_bus_new, pFv)
//GO(ibus_bus_new_async, 
//GO(ibus_bus_new_async_client, 
//GO(ibus_bus_preload_engines, 
//GO(ibus_bus_preload_engines_async, 
//GO(ibus_bus_preload_engines_async_finish, 
//GO(ibus_bus_register_component, 
//GO(ibus_bus_register_component_async, 
//GO(ibus_bus_register_component_async_finish, 
//GO(ibus_bus_release_name, 
//GO(ibus_bus_release_name_async, 
//GO(ibus_bus_release_name_async_finish, 
//GO(ibus_bus_remove_match, 
//GO(ibus_bus_remove_match_async, 
//GO(ibus_bus_remove_match_async_finish, 
//GO(ibus_bus_request_name, 
//GO(ibus_bus_request_name_async, 
//GO(ibus_bus_request_name_async_finish, 
GO(ibus_bus_request_name_reply_get_type, LFv)
//GO(ibus_bus_set_global_engine, 
GOM(ibus_bus_set_global_engine_async, vFEppippp)
GO(ibus_bus_set_global_engine_async_finish, pFppp)
//GO(ibus_bus_set_ibus_property, 
//GO(ibus_bus_set_ibus_property_async, 
//GO(ibus_bus_set_ibus_property_async_finish, 
GO(ibus_bus_set_watch_dbus_signal, vFpi)
GO(ibus_bus_set_watch_ibus_signal, vFpi)
GO(ibus_bus_start_service_by_name_reply_get_type, LFv)
GO(ibus_capabilite_get_type, LFv)
//GO(ibus_check_algorithmically, 
//GO(ibus_component_add_engine, 
//GO(ibus_component_add_observed_path, 
//GO(ibus_component_check_modification, 
//GO(ibus_component_get_author, 
//GO(ibus_component_get_description, 
//GO(ibus_component_get_engines, 
//GO(ibus_component_get_exec, 
//GO(ibus_component_get_homepage, 
//GO(ibus_component_get_license, 
//GO(ibus_component_get_name, 
//GO(ibus_component_get_observed_paths, 
//GO(ibus_component_get_textdomain, 
//GO(ibus_component_get_type, 
//GO(ibus_component_get_version, 
//GO(ibus_component_new, 
//GO(ibus_component_new_from_file, 
//GO(ibus_component_new_from_xml_node, 
//GO(ibus_component_new_varargs, 
//GO(ibus_component_output, 
//GO(ibus_component_output_engines, 
//GO(ibus_compose_table_check, 
//DATA(ibus_compose_table_compact, 
//DATA(ibus_compose_table_compact_32bit, 
//DATA(ibus_compose_table_compact_32bit_priv, 
//GO(ibus_compose_table_compact_check, 
//GO(ibus_compose_table_list_add_array, 
//GO(ibus_compose_table_list_add_file, 
//GO(ibus_compose_table_load_cache, 
//GO(ibus_compose_table_new_with_file, 
//GO(ibus_compose_table_save_cache, 
//GO(ibus_config_get_type, 
//GO(ibus_config_get_value, 
GOM(ibus_config_get_value_async, vFEpppippp)
GO(ibus_config_get_value_async_finish, pFppp)
//GO(ibus_config_get_values, 
GOM(ibus_config_get_values_async, vFEppippp)
GO(ibus_config_get_values_async_finish, pFppp)
//GO(ibus_config_new, 
//GO(ibus_config_new_async, 
//GO(ibus_config_new_async_finish, 
//GO(ibus_config_service_get_type, 
//GO(ibus_config_service_new, 
//GO(ibus_config_service_value_changed, 
//GO(ibus_config_set_value, 
GOM(ibus_config_set_value_async, vFEppppippp)
GO(ibus_config_set_value_async_finish, pFppp)
GO(ibus_config_unset, iFppp)
GO(ibus_config_unwatch, iFppp)
GO(ibus_config_watch, iFppp)
//GO(ibus_emoji_data_get_annotations, 
//GO(ibus_emoji_data_get_category, 
//GO(ibus_emoji_data_get_description, 
//GO(ibus_emoji_data_get_emoji, 
//GO(ibus_emoji_data_get_type, 
//GO(ibus_emoji_data_load, 
//GO(ibus_emoji_data_new, 
//GO(ibus_emoji_data_save, 
//GO(ibus_emoji_data_set_annotations, 
//GO(ibus_emoji_data_set_description, 
//GO(ibus_emoji_dict_load, 
//GO(ibus_emoji_dict_lookup, 
//GO(ibus_emoji_dict_save, 
//GO(ibus_engine_commit_text, 
//GO(ibus_engine_delete_surrounding_text, 
GO(ibus_engine_desc_get_author, pFp)
GO(ibus_engine_desc_get_description, pFp)
GO(ibus_engine_desc_get_hotkeys, pFp)
GO(ibus_engine_desc_get_icon, pFp)
GO(ibus_engine_desc_get_icon_prop_key, pFp)
GO(ibus_engine_desc_get_language, pFp)
GO(ibus_engine_desc_get_layout, pFp)
GO(ibus_engine_desc_get_layout_option, pFp)
GO(ibus_engine_desc_get_layout_variant, pFp)
GO(ibus_engine_desc_get_license, pFp)
GO(ibus_engine_desc_get_longname, pFp)
GO(ibus_engine_desc_get_name, pFp)
GO(ibus_engine_desc_get_rank, iFp)
GO(ibus_engine_desc_get_setup, pFp)
GO(ibus_engine_desc_get_symbol, pFp)
GO(ibus_engine_desc_get_textdomain, pFp)
GO(ibus_engine_desc_get_type, pFp)
GO(ibus_engine_desc_get_version, pFp)
//GO(ibus_engine_desc_new, 
//GO(ibus_engine_desc_new_from_xml_node, 
//GO(ibus_engine_desc_new_varargs, 
//GO(ibus_engine_desc_output, 
//GO(ibus_engine_forward_key_event, 
//GO(ibus_engine_get_content_type, 
//GO(ibus_engine_get_name, 
//GO(ibus_engine_get_surrounding_text, 
//GO(ibus_engine_get_type, 
//GO(ibus_engine_hide_auxiliary_text, 
//GO(ibus_engine_hide_lookup_table, 
//GO(ibus_engine_hide_preedit_text, 
//GO(ibus_engine_new, 
//GO(ibus_engine_new_with_type, 
//GO(ibus_engine_register_properties, 
//GO(ibus_engine_show_auxiliary_text, 
//GO(ibus_engine_show_lookup_table, 
//GO(ibus_engine_show_preedit_text, 
//GO(ibus_engine_simple_add_compose_file, 
//GO(ibus_engine_simple_add_table, 
//GO(ibus_engine_simple_add_table_by_locale, 
//GO(ibus_engine_simple_get_type, 
//GO(ibus_engine_update_auxiliary_text, 
//GO(ibus_engine_update_lookup_table, 
//GO(ibus_engine_update_lookup_table_fast, 
//GO(ibus_engine_update_preedit_text, 
//GO(ibus_engine_update_preedit_text_with_mode, 
//GO(ibus_engine_update_property, 
GO(ibus_error_get_type, LFv)
//GO(ibus_error_quark, 
//GO(ibus_extension_event_get_name, 
//GO(ibus_extension_event_get_params, 
//GO(ibus_extension_event_get_type, 
//GO(ibus_extension_event_get_version, 
//GO(ibus_extension_event_is_enabled, 
//GO(ibus_extension_event_is_extension, 
//GO(ibus_extension_event_new, 
//GO(ibus_factory_add_engine, 
//GO(ibus_factory_create_engine, 
//GO(ibus_factory_get_type, 
//GO(ibus_factory_new, 
//GO(ibus_free_strv, 
//GO(ibus_get_address, 
//GO(ibus_get_daemon_uid, 
//GO(ibus_get_language_name, 
//GO(ibus_get_local_machine_id, 
//GO(ibus_get_session_id, 
//GO(ibus_get_socket_path, 
//GO(ibus_get_timeout, 
//GO(ibus_get_untranslated_language_name, 
//GO(ibus_get_user_name, 
//GO(ibus_hotkey_get_type, 
//GO(ibus_hotkey_profile_add_hotkey, 
//GO(ibus_hotkey_profile_add_hotkey_from_string, 
//GO(ibus_hotkey_profile_filter_key_event, 
//GO(ibus_hotkey_profile_get_type, 
//GO(ibus_hotkey_profile_lookup_hotkey, 
//GO(ibus_hotkey_profile_new, 
//GO(ibus_hotkey_profile_remove_hotkey, 
//GO(ibus_hotkey_profile_remove_hotkey_by_event, 
GO(ibus_init, vFv)
//GO(ibus_input_context_cancel_hand_writing, 
//GO(ibus_input_context_cursor_down, 
//GO(ibus_input_context_cursor_up, 
GO(ibus_input_context_focus_in, vFp)
GO(ibus_input_context_focus_out, vFp)
//GO(ibus_input_context_get_engine, 
GOM(ibus_input_context_get_engine_async, vFEpippp)
GO(ibus_input_context_get_engine_async_finish, pFppp)
//GO(ibus_input_context_get_input_context, 
//GO(ibus_input_context_get_input_context_async, 
//GO(ibus_input_context_get_input_context_async_finish, 
//GO(ibus_input_context_get_type, 
GO(ibus_input_context_needs_surrounding_text, iFp)
//GO(ibus_input_context_new, 
//GO(ibus_input_context_new_async, 
//GO(ibus_input_context_new_async_finish, 
//GO(ibus_input_context_page_down, 
//GO(ibus_input_context_page_up, 
//GO(ibus_input_context_process_hand_writing_event, 
//GO(ibus_input_context_process_key_event, 
GOM(ibus_input_context_process_key_event_async, vFEpuuuippp)
GO(ibus_input_context_process_key_event_async_finish, pFppp)
GO(ibus_input_context_property_activate, vFppu)
//GO(ibus_input_context_property_hide, 
//GO(ibus_input_context_property_show, 
GO(ibus_input_context_reset, vFp)
GO(ibus_input_context_set_capabilities, vFpu)
//GO(ibus_input_context_set_client_commit_preedit, 
GO(ibus_input_context_set_content_type, vFpuu)
GO(ibus_input_context_set_cursor_location, vFpuuuu)
//GO(ibus_input_context_set_cursor_location_relative, 
GO(ibus_input_context_set_engine, vFpp)
GO(ibus_input_context_set_surrounding_text, vFppuu)
GO(ibus_input_hints_get_type, LFv)
GO(ibus_input_purpose_get_type, LFv)
//GO(ibus_key_event_from_string, 
//GO(ibus_key_event_to_string, 
//GO(ibus_keymap_fill, 
GO(ibus_keymap_get, pFp)
GO(ibus_keymap_get_type, LFv)
GO(ibus_keymap_lookup_keysym, uFpWu)
//GO(ibus_keymap_new, 
//GO(ibus_keysym_to_unicode, 
//GO(ibus_keyval_convert_case, 
GO(ibus_keyval_from_name, uFp)
GO(ibus_keyval_name, pFu)
//GO(ibus_keyval_to_lower, 
GO(ibus_keyval_to_unicode, uFu)
//GO(ibus_keyval_to_upper, 
GO(ibus_lookup_table_append_candidate, vFpp)
GO(ibus_lookup_table_append_label, vFpp)
GO(ibus_lookup_table_clear, vFp)
//GO(ibus_lookup_table_cursor_down, 
//GO(ibus_lookup_table_cursor_up, 
GO(ibus_lookup_table_get_candidate, pFpu)
GO(ibus_lookup_table_get_cursor_in_page, uFp)
GO(ibus_lookup_table_get_cursor_pos, uFp)
GO(ibus_lookup_table_get_label, pFpu)
GO(ibus_lookup_table_get_number_of_candidates, uFp)
GO(ibus_lookup_table_get_orientation, iFp)
GO(ibus_lookup_table_get_page_size, uFp)
//GO(ibus_lookup_table_get_type, 
GO(ibus_lookup_table_is_cursor_visible, iFp)
GO(ibus_lookup_table_is_round, iFp)
GO(ibus_lookup_table_new, pFuuii)
//GO(ibus_lookup_table_page_down, 
//GO(ibus_lookup_table_page_up, 
GO(ibus_lookup_table_set_cursor_pos, vFpu)
GO(ibus_lookup_table_set_cursor_visible, vFpi)
GO(ibus_lookup_table_set_label, vFpup)
GO(ibus_lookup_table_set_orientation, vFpi)
GO(ibus_lookup_table_set_page_size, vFpu)
GO(ibus_lookup_table_set_round, vFpi)
//GO(ibus_main, 
GO(ibus_modifier_type_get_type, LFv)
//GO(ibus_object_destroy, 
GO(ibus_object_flags_get_type, LFv)
GO(ibus_object_get_type, LFv)
//GO(ibus_object_new, 
//GO(ibus_observed_path_check_modification, 
//GO(ibus_observed_path_get_type, 
//GO(ibus_observed_path_new, 
//GO(ibus_observed_path_new_from_xml_node, 
//GO(ibus_observed_path_output, 
//GO(ibus_observed_path_traverse, 
GO(ibus_orientation_get_type, LFv)
//GO(ibus_panel_service_candidate_clicked, 
//GO(ibus_panel_service_commit_text, 
//GO(ibus_panel_service_cursor_down, 
//GO(ibus_panel_service_cursor_up, 
//GO(ibus_panel_service_get_type, 
//GO(ibus_panel_service_new, 
//GO(ibus_panel_service_page_down, 
//GO(ibus_panel_service_page_up, 
//GO(ibus_panel_service_panel_extension, 
//GO(ibus_panel_service_panel_extension_register_keys, 
//GO(ibus_panel_service_property_activate, 
//GO(ibus_panel_service_property_hide, 
//GO(ibus_panel_service_property_show, 
//GO(ibus_panel_service_update_auxiliary_text_received, 
//GO(ibus_panel_service_update_lookup_table_received, 
//GO(ibus_panel_service_update_preedit_text_received, 
GO(ibus_preedit_focus_mode_get_type, LFv)
GO(ibus_property_get_icon, pFp)
GO(ibus_property_get_key, pFp)
GO(ibus_property_get_label, pFp)
GO(ibus_property_get_prop_type, iFp)
GO(ibus_property_get_sensitive, iFp)
GO(ibus_property_get_state, iFp)
GO(ibus_property_get_sub_props, pFp)
GO(ibus_property_get_symbol, pFp)
GO(ibus_property_get_tooltip, pFp)
GO(ibus_property_get_type, LFv)
GO(ibus_property_get_visible, iFp)
//GO(ibus_property_new, 
//GO(ibus_property_new_varargs, 
GO(ibus_property_set_icon, vFpp)
GO(ibus_property_set_label, vFpp)
GO(ibus_property_set_sensitive, vFpi)
GO(ibus_property_set_state, vFpi)
//GO(ibus_property_set_sub_props, 
GO(ibus_property_set_symbol, vFpp)
GO(ibus_property_set_tooltip, vFpp)
GO(ibus_property_set_visible, vFpi)
//GO(ibus_property_update, 
//GO(ibus_prop_list_append, 
GO(ibus_prop_list_get, pFpu)
GO(ibus_prop_list_get_type, LFv)
//GO(ibus_prop_list_new, 
//GO(ibus_prop_list_update_property, 
GO(ibus_prop_state_get_type, LFv)
GO(ibus_prop_type_get_type, LFv)
//GO(ibus_proxy_destroy, 
//GO(ibus_proxy_get_type, 
//GO(ibus_quit, 
//GO(ibus_registry_check_modification, 
//GO(ibus_registry_get_components, 
//GO(ibus_registry_get_observed_paths, 
//GO(ibus_registry_get_type, 
//GO(ibus_registry_load, 
//GO(ibus_registry_load_cache, 
//GO(ibus_registry_load_cache_file, 
//GO(ibus_registry_load_in_dir, 
//GO(ibus_registry_new, 
//GO(ibus_registry_output, 
//GO(ibus_registry_save_cache, 
//GO(ibus_registry_save_cache_file, 
//GO(ibus_registry_start_monitor_changes, 
//GO(ibus_serializable_copy, 
//GO(ibus_serializable_deserialize_object, 
//GO(ibus_serializable_get_qattachment, 
//GO(ibus_serializable_get_type, 
//GO(ibus_serializable_new, 
//GO(ibus_serializable_remove_qattachment, 
//GO(ibus_serializable_serialize_object, 
//GO(ibus_serializable_set_qattachment, 
//GO(ibus_service_class_add_interfaces, 
//GO(ibus_service_emit_signal, 
//GO(ibus_service_get_connection, 
//GO(ibus_service_get_object_path, 
//GO(ibus_service_get_type, 
//GO(ibus_service_new, 
//GO(ibus_service_register, 
//GO(ibus_service_unregister, 
//GO(ibus_set_display, 
//GO(ibus_set_log_handler, 
//GO(ibus_text_append_attribute, 
//GO(ibus_text_get_attributes, 
//GO(ibus_text_get_is_static, 
//GO(ibus_text_get_length, 
GO(ibus_text_get_text, pFp)
GO(ibus_text_get_type, LFv)
//GO(ibus_text_new_from_printf, 
//GO(ibus_text_new_from_static_string, 
GO(ibus_text_new_from_string, pFp)
//GO(ibus_text_new_from_ucs4, 
//GO(ibus_text_new_from_unichar, 
//GO(ibus_text_set_attributes, 
//GO(ibus_unicode_block_get_end, 
//GO(ibus_unicode_block_get_name, 
//GO(ibus_unicode_block_get_start, 
//GO(ibus_unicode_block_get_type, 
//GO(ibus_unicode_block_load, 
//GO(ibus_unicode_block_new, 
//GO(ibus_unicode_block_save, 
//GO(ibus_unicode_data_get_alias, 
//GO(ibus_unicode_data_get_block_name, 
//GO(ibus_unicode_data_get_code, 
//GO(ibus_unicode_data_get_name, 
//GO(ibus_unicode_data_get_type, 
//GO(ibus_unicode_data_load, 
//GO(ibus_unicode_data_load_async, 
//GO(ibus_unicode_data_new, 
//GO(ibus_unicode_data_save, 
//GO(ibus_unicode_data_set_block_name, 
GO(ibus_unicode_to_keyval, uFu)
//GO(ibus_unset_log_handler, 
//GO(ibus_write_address, 
//GO(ibus_x_event_get_event_type, 
//GO(ibus_x_event_get_group, 
//GO(ibus_x_event_get_hardware_keycode, 
//GO(ibus_x_event_get_is_modifier, 
//GO(ibus_x_event_get_keyval, 
//GO(ibus_x_event_get_length, 
//GO(ibus_x_event_get_purpose, 
//GO(ibus_x_event_get_root, 
//GO(ibus_x_event_get_same_screen, 
//GO(ibus_x_event_get_send_event, 
//GO(ibus_x_event_get_serial, 
//GO(ibus_x_event_get_state, 
//GO(ibus_x_event_get_string, 
//GO(ibus_x_event_get_subwindow, 
//GO(ibus_x_event_get_time, 
//GO(ibus_x_event_get_type, 
//GO(ibus_x_event_get_version, 
//GO(ibus_x_event_get_window, 
//GO(ibus_x_event_get_x, 
//GO(ibus_x_event_get_x_root, 
//GO(ibus_x_event_get_y, 
//GO(ibus_x_event_get_y_root, 
//GO(ibus_x_event_new, 
//GO(ibus_xevent_type_get_type, 
//GO(ibus_xml_copy, 
//GO(ibus_xml_free, 
//GO(ibus_xml_get_type, 
//GO(ibus_xml_output, 
//GO(ibus_xml_parse_buffer, 
//GO(ibus_xml_parse_file, 

