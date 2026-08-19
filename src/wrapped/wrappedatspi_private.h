#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(_atspi_accessible_add_cache, vFpu)
GO(atspi_accessible_clear_cache, vFp)
GO(atspi_accessible_get_accessible_id, pFpp)
GO(atspi_accessible_get_action, pFp)
GO(atspi_accessible_get_action_iface, pFp)
GO(atspi_accessible_get_application, pFpp)
GO(atspi_accessible_get_atspi_version, pFpp)
GO(atspi_accessible_get_attributes, pFpp)
GO(atspi_accessible_get_attributes_as_array, pFpp)
GO(_atspi_accessible_get_cache_mask, uFp)
GO(atspi_accessible_get_child_at_index, pFpip)
GO(atspi_accessible_get_child_count, iFpp)
GO(atspi_accessible_get_collection, pFp)
GO(atspi_accessible_get_collection_iface, pFp)
GO(atspi_accessible_get_component, pFp)
GO(atspi_accessible_get_component_iface, pFp)
GO(atspi_accessible_get_description, pFpp)
GO(atspi_accessible_get_document, pFp)
GO(atspi_accessible_get_document_iface, pFp)
GO(atspi_accessible_get_editable_text, pFp)
GO(atspi_accessible_get_editable_text_iface, pFp)
GO(atspi_accessible_get_hyperlink, pFp)
GO(atspi_accessible_get_hypertext, pFp)
GO(atspi_accessible_get_hypertext_iface, pFp)
GO(atspi_accessible_get_id, iFpp)
GO(atspi_accessible_get_image, pFp)
GO(atspi_accessible_get_image_iface, pFp)
GO(atspi_accessible_get_index_in_parent, iFpp)
GO(atspi_accessible_get_interfaces, pFp)
GO(atspi_accessible_get_localized_role_name, pFpp)
GO(atspi_accessible_get_name, pFpp)
GO(atspi_accessible_get_object_locale, pFpp)
GO(atspi_accessible_get_parent, pFpp)
GO(atspi_accessible_get_process_id, uFpp)
GO(atspi_accessible_get_relation_set, pFpp)
GO(atspi_accessible_get_role, uFpp)
GO(atspi_accessible_get_role_name, pFpp)
GO(atspi_accessible_get_selection, pFp)
GO(atspi_accessible_get_selection_iface, pFp)
GO(atspi_accessible_get_state_set, pFp)
GO(atspi_accessible_get_table, pFp)
GO(atspi_accessible_get_table_cell, pFp)
GO(atspi_accessible_get_table_iface, pFp)
GO(atspi_accessible_get_text, pFp)
GO(atspi_accessible_get_text_iface, pFp)
GO(atspi_accessible_get_toolkit_name, pFpp)
GO(atspi_accessible_get_toolkit_version, pFpp)
GO(atspi_accessible_get_type, LFv)
GO(atspi_accessible_get_value, pFp)
GO(atspi_accessible_get_value_iface, pFp)
GO(atspi_accessible_is_action, iFp)
GO(atspi_accessible_is_application, iFp)
GO(atspi_accessible_is_collection, iFp)
GO(atspi_accessible_is_component, iFp)
GO(atspi_accessible_is_document, iFp)
GO(atspi_accessible_is_editable_text, iFp)
GO(atspi_accessible_is_hyperlink, iFp)
GO(atspi_accessible_is_hypertext, iFp)
GO(atspi_accessible_is_image, iFp)
GO(atspi_accessible_is_selection, iFp)
//GO(atspi_accessible_is_streamable_content, 
GO(atspi_accessible_is_table, iFp)
GO(atspi_accessible_is_table_cell, iFp)
GO(atspi_accessible_is_text, iFp)
GO(atspi_accessible_is_value, iFp)
GO(_atspi_accessible_new, pFpp)
//GO(_atspi_accessible_ref_cache, 
GO(atspi_accessible_set_cache_mask, vFpu)
GO(_atspi_accessible_test_cache, iFpu)
//GO(_atspi_accessible_unref_cache, 
GO(atspi_action_do_action, iFpip)
GO(atspi_action_get_action_description, pFpip)
GO(atspi_action_get_action_name, pFpip)
GO(atspi_action_get_description, pFpip)
GO(atspi_action_get_key_binding, pFpip)
GO(atspi_action_get_localized_name, pFpip)
GO(atspi_action_get_n_actions, iFpp)
GO(atspi_action_get_name, pFpip)
GO(atspi_action_get_type, LFv)
GO(atspi_application_get_type, LFv)
GO(_atspi_application_new, pFp)
//GO(_atspi_bus, 
//DATA(atspi_bus_registry, 
GO(atspi_cache_get_type, LFv)
GO(atspi_collection_get_active_descendant, pFpp)
GO(atspi_collection_get_matches, pFppuiip)
GO(atspi_collection_get_matches_from, pFpppuuiip)
GO(atspi_collection_get_matches_to, pFpppuuiiip)
GO(atspi_collection_get_type, LFv)
GO(atspi_collection_is_ancestor_of, iFppp)
GO(atspi_collection_match_type_get_type, LFv)
GO(atspi_collection_sort_order_get_type, LFv)
GO(atspi_collection_tree_traversal_type_get_type, LFv)
GO(atspi_component_contains, iFpiiup)
GO(atspi_component_get_accessible_at_point, pFpiiup)
GO(atspi_component_get_alpha, dFpp)
GO(atspi_component_get_extents, pFpup)
GO(atspi_component_get_layer, uFpp)
GO(atspi_component_get_mdi_z_order, wFpp)
GO(atspi_component_get_position, pFpup)
GO(atspi_component_get_size, pFpp)
GO(atspi_component_get_type, LFv)
GO(atspi_component_grab_focus, iFpp)
GO(atspi_component_layer_get_type, LFv)
GO(atspi_component_scroll_to, iFpup)
GO(atspi_component_scroll_to_point, iFpuiip)
GO(atspi_component_set_extents, iFpiiiiup)
GO(atspi_component_set_position, iFpiiup)
GO(atspi_component_set_size, iFpiip)
GO(atspi_coord_type_get_type, LFv)
//GO(_atspi_dbus_attribute_array_from_iter, 
//GO(_atspi_dbus_call, 
//GO(_atspi_dbus_call_partial, 
//GO(_atspi_dbus_call_partial_va, 
GO(atspi_dbus_connection_setup_with_g_main, vFpp)
//GO(_atspi_dbus_get_property, 
//GO(_atspi_dbus_handle_DeviceEvent, 
//GO(_atspi_dbus_handle_event, 
//GO(_atspi_dbus_hash_from_iter, 
//GO(_atspi_dbus_return_accessible_from_iter, 
//GO(_atspi_dbus_return_accessible_from_message, 
//GO(_atspi_dbus_return_attribute_array_from_message, 
//GO(_atspi_dbus_return_hash_from_message, 
//GO(_atspi_dbus_return_hyperlink_from_iter, 
//GO(_atspi_dbus_return_hyperlink_from_message, 
//GO(_atspi_dbus_send_with_reply_and_block, 
GO(atspi_dbus_server_setup_with_g_main, vFpp)
//GO(_atspi_dbus_set_interfaces, 
//GO(_atspi_dbus_set_state, 
//GO(_atspi_dbus_update_cache_from_dict, 
GO(atspi_deregister_device_event_listener, iFppp)
GO(atspi_deregister_keystroke_listener, iFppuup)
//GO(atspi_device_event_free, 
GO(atspi_device_event_get_type, LFv)
//GOM(atspi_device_listener_add_callback, vFEpppp)
//GO(_atspi_device_listener_get_path, 
GO(atspi_device_listener_get_type, LFv)
//GOM(atspi_device_listener_new, pFEppp)
//GOM(atspi_device_listener_new_simple, pFEpp)
//GOM(atspi_device_listener_remove_callback, vFEpp)
GO(atspi_document_get_attributes, pFpp)
GO(atspi_document_get_attribute_value, pFppp)
GO(atspi_document_get_current_page_number, iFpp)
GO(atspi_document_get_document_attributes, pFpp)
GO(atspi_document_get_document_attribute_value, pFppp)
GO(atspi_document_get_locale, pFpp)
GO(atspi_document_get_page_count, iFpp)
GO(atspi_document_get_type, LFv)
GO(atspi_editable_text_copy_text, iFpiip)
GO(atspi_editable_text_cut_text, iFpiip)
GO(atspi_editable_text_delete_text, iFpiip)
GO(atspi_editable_text_get_type, LFv)
GO(atspi_editable_text_insert_text, iFpipip)
GO(atspi_editable_text_paste_text, iFpip)
GO(atspi_editable_text_set_text_contents, iFppp)
//GO(_atspi_error_quark, 
GO(atspi_event_get_type, LFv)
GO(atspi_event_listener_deregister, iFppp)
//GOM(atspi_event_listener_deregister_from_callback, iFEpppp)
//GOM(atspi_event_listener_deregister_no_data, iFEppp)
GO(atspi_event_listener_get_type, LFv)
GOM(atspi_event_listener_new, pFppp)
//GOM(atspi_event_listener_new_simple, pFEpp)
GO(atspi_event_listener_register, iFppp)
//GOM(atspi_event_listener_register_from_callback, iFEppppp)
//GOM(atspi_event_listener_register_from_callback_full, iFEpppppp)
//GOM(atspi_event_listener_register_full, iFEpppp)
//GOM(atspi_event_listener_register_no_data, iFEpppp)
GO(atspi_event_main, vFv)
GO(atspi_event_quit, vFv)
GO(atspi_event_type_get_type, LFv)
GO(atspi_exit, iFv)
GO(atspi_generate_keyboard_event, iFlpup)
GO(atspi_generate_mouse_event, iFllpp)
GO(atspi_get_a11y_bus, pFv)
//GO(_atspi_get_allow_sync, 
GO(atspi_get_desktop, pFi)
GO(atspi_get_desktop_count, iFv)
GO(atspi_get_desktop_list, pFv)
//GO(_atspi_get_iface_num, 
//GO(_atspi_get_live_refs, 
GO(atspi_hyperlink_get_end_index, iFpp)
GO(atspi_hyperlink_get_index_range, pFpp)
GO(atspi_hyperlink_get_n_anchors, iFpp)
GO(atspi_hyperlink_get_object, pFpip)
GO(atspi_hyperlink_get_start_index, iFpp)
GO(atspi_hyperlink_get_type, LFv)
GO(atspi_hyperlink_get_uri, pFpip)
GO(atspi_hyperlink_is_valid, iFpp)
GO(_atspi_hyperlink_new, pFpp)
GO(atspi_hypertext_get_link, pFpip)
GO(atspi_hypertext_get_link_index, iFpip)
GO(atspi_hypertext_get_n_links, iFpp)
GO(atspi_hypertext_get_type, LFv)
GO(atspi_image_get_image_description, pFpp)
GO(atspi_image_get_image_extents, pFpup)
GO(atspi_image_get_image_locale, pFpp)
GO(atspi_image_get_image_position, pFpup)
GO(atspi_image_get_image_size, pFpp)
GO(atspi_image_get_type, LFv)
GO(atspi_init, iFv)
//DATA(atspi_interface_accessible, 
//DATA(atspi_interface_action, 
//DATA(atspi_interface_application, 
//DATA(atspi_interface_cache, 
//DATA(atspi_interface_collection, 
//DATA(atspi_interface_component, 
//DATA(atspi_interface_dec, 
//DATA(atspi_interface_device_event_listener, 
//DATA(atspi_interface_document, 
//DATA(atspi_interface_editable_text, 
//DATA(atspi_interface_event_object, 
//DATA(atspi_interface_hyperlink, 
//DATA(atspi_interface_hypertext, 
//DATA(atspi_interface_image, 
//DATA(atspi_interface_registry, 
//DATA(atspi_interface_selection, 
//DATA(atspi_interface_table, 
//DATA(atspi_interface_table_cell, 
//DATA(atspi_interface_text, 
//DATA(atspi_interface_value, 
GO(atspi_is_initialized, iFv)
//GO(atspi_key_definition_copy, 
//GO(atspi_key_definition_free, 
GO(atspi_key_definition_get_type, LFv)
GO(atspi_key_event_type_get_type, LFv)
GO(atspi_key_listener_sync_type_get_type, LFv)
GO(atspi_key_synth_type_get_type, LFv)
GO(atspi_locale_type_get_type, LFv)
//DATAB(atspi_main_context, 
//DATAB(atspi_main_loop, 
//GO(atspi_marshal_VOID__INT_INT, 
GO(atspi_match_rule_get_type, LFv)
//GO(_atspi_match_rule_marshal, 
GO(atspi_match_rule_new, pFpupupupui)
GO(atspi_modifier_type_get_type, LFv)
//GO(_atspi_mutter_generate_keyboard_event, 
//GO(_atspi_mutter_generate_mouse_event, 
//GO(_atspi_mutter_set_reference_window, 
//GO(_atspi_name_compat, 
//DATAB(atspi_no_cache, 
GO(atspi_object_get_type, LFv)
//DATA(atspi_path_dec, 
//DATA(atspi_path_registry, 
//DATA(atspi_path_root, 
GO(atspi_point_copy, pFp)
GO(atspi_point_get_type, LFv)
//GO(_atspi_prepare_screen_reader_interface, 
GO(atspi_range_copy, pFp)
GO(atspi_range_get_type, LFv)
GO(atspi_rect_copy, pFp)
//GO(atspi_rect_free, 
GO(atspi_rect_get_type, LFv)
//GO(_atspi_ref_accessible, 
GO(atspi_register_device_event_listener, iFpupp)
GO(atspi_register_keystroke_listener, iFppuuup)
GO(atspi_relation_get_n_targets, iFp)
GO(atspi_relation_get_relation_type, uFp)
GO(atspi_relation_get_target, pFpi)
GO(atspi_relation_get_type, LFv)
GO(_atspi_relation_new_from_iter, pFp)
GO(atspi_relation_type_get_type, LFv)
//GO(_atspi_reregister_device_listeners, 
//GO(_atspi_reregister_event_listeners, 
GO(atspi_role_get_name, pFu)
GO(atspi_role_get_type, LFv)
GO(atspi_scroll_type_get_type, LFv)
GO(atspi_selection_clear_selection, iFpp)
GO(atspi_selection_deselect_child, iFpip)
GO(atspi_selection_deselect_selected_child, iFpip)
GO(atspi_selection_get_n_selected_children, iFpp)
GO(atspi_selection_get_selected_child, pFpip)
GO(atspi_selection_get_type, LFv)
GO(atspi_selection_is_child_selected, iFpip)
GO(atspi_selection_select_all, iFpp)
GO(atspi_selection_select_child, iFpip)
//GO(_atspi_send_event, 
//GO(_atspi_set_allow_sync, 
//GO(_atspi_set_error_no_sync, 
GO(atspi_set_main_context, vFp)
GO(atspi_set_reference_window, vFp)
GO(atspi_set_timeout, vFii)
GO(atspi_state_set_add, vFpu)
GO(atspi_state_set_compare, pFpp)
GO(atspi_state_set_contains, iFpu)
GO(atspi_state_set_equals, iFpp)
GO(atspi_state_set_get_states, pFp)
GO(atspi_state_set_get_type, LFv)
GO(atspi_state_set_is_empty, iFp)
GO(atspi_state_set_new, pFp)
GO(_atspi_state_set_new_internal, pFpl)
GO(atspi_state_set_remove, vFpu)
GO(atspi_state_set_set_by_name, vFppi)
GO(atspi_state_type_get_type, LFv)
GO(atspi_table_add_column_selection, iFpip)
GO(atspi_table_add_row_selection, iFpip)
GO(atspi_table_cell_get_column_header_cells, pFpp)
GO(atspi_table_cell_get_column_span, iFpp)
GO(atspi_table_cell_get_position, iFpppp)
GO(atspi_table_cell_get_row_column_span, vFpppppp)
GO(atspi_table_cell_get_row_header_cells, pFpp)
GO(atspi_table_cell_get_row_span, iFpp)
GO(atspi_table_cell_get_table, pFpp)
GO(atspi_table_cell_get_type, LFv)
GO(atspi_table_get_accessible_at, pFpiip)
GO(atspi_table_get_caption, pFpp)
GO(atspi_table_get_column_at_index, iFpip)
GO(atspi_table_get_column_description, pFpip)
GO(atspi_table_get_column_extent_at, iFpiip)
GO(atspi_table_get_column_header, pFpip)
GO(atspi_table_get_index_at, iFpiip)
GO(atspi_table_get_n_columns, iFpp)
GO(atspi_table_get_n_rows, iFpp)
GO(atspi_table_get_n_selected_columns, iFpp)
GO(atspi_table_get_n_selected_rows, iFpp)
GO(atspi_table_get_row_at_index, iFpip)
GO(atspi_table_get_row_column_extents_at_index, iFpipppppp)
GO(atspi_table_get_row_description, pFpip)
GO(atspi_table_get_row_extent_at, iFpiip)
GO(atspi_table_get_row_header, pFpip)
GO(atspi_table_get_selected_columns, pFpp)
GO(atspi_table_get_selected_rows, pFpp)
GO(atspi_table_get_summary, pFpp)
GO(atspi_table_get_type, LFv)
GO(atspi_table_is_column_selected, iFpip)
GO(atspi_table_is_row_selected, iFpip)
GO(atspi_table_is_selected, iFpiip)
GO(atspi_table_remove_column_selection, iFpip)
GO(atspi_table_remove_row_selection, iFpip)
GO(atspi_text_add_selection, iFpiip)
GO(atspi_text_boundary_type_get_type, LFv)
GO(atspi_text_clip_type_get_type, LFv)
GO(atspi_text_get_attribute_run, pFpiippp)
GO(atspi_text_get_attributes, pFpippp)
GO(atspi_text_get_attribute_value, pFpipp)
GO(atspi_text_get_bounded_ranges, pFpiiiiuuup)
GO(atspi_text_get_caret_offset, iFpp)
GO(atspi_text_get_character_at_offset, uFpip)
GO(atspi_text_get_character_count, iFpp)
GO(atspi_text_get_character_extents, pFpiup)
GO(atspi_text_get_default_attributes, pFpp)
GO(atspi_text_get_n_selections, iFpp)
GO(atspi_text_get_offset_at_point, iFpiiup)
GO(atspi_text_get_range_extents, pFpiiup)
GO(atspi_text_get_selection, pFpip)
GO(atspi_text_get_string_at_offset, pFpiup)
GO(atspi_text_get_text, pFpiip)
GO(atspi_text_get_text_after_offset, pFpiup)
GO(atspi_text_get_text_at_offset, pFpiup)
GO(atspi_text_get_text_attributes, pFpippp)
GO(atspi_text_get_text_attribute_value, pFpipp)
GO(atspi_text_get_text_before_offset, pFpiup)
GO(atspi_text_get_type, LFv)
GO(atspi_text_granularity_get_type, LFv)
GO(atspi_text_range_get_type, LFv)
GO(atspi_text_remove_selection, iFpip)
GO(atspi_text_scroll_substring_to, iFpiiup)
GO(atspi_text_scroll_substring_to_point, iFpiiuiip)
GO(atspi_text_set_caret_offset, iFpip)
GO(atspi_text_set_selection, iFpiiip)
GO(atspi_value_get_current_value, dFpp)
GO(atspi_value_get_maximum_value, dFpp)
GO(atspi_value_get_minimum_increment, dFpp)
GO(atspi_value_get_minimum_value, dFpp)
GO(atspi_value_get_type, LFv)
GO(atspi_value_set_current_value, iFpdp)
//DATA(cache_signal_type, 
//GO(callback_ref, 
//GO(callback_unref, 
//GO(dbind_any_demarshal, 
//GO(dbind_any_demarshal_va, 
//GO(dbind_any_free, 
//GO(dbind_any_free_ptr, 
//GO(dbind_any_marshal, 
//GO(dbind_any_marshal_va, 
//GO(dbind_emit_signal, 
//GO(dbind_emit_signal_va, 
//GO(dbind_find_c_alignment, 
//GO(dbind_method_call_reentrant, 
//GO(dbind_method_call_reentrant_va, 
//GO(dbind_send_and_allow_reentry, 
//GO(dbind_set_timeout, 
//DATA(_dbus_gmain_connection_slot, 
//DATAB(device_parent_class, 
//GO(free_value, 
//GO(get_reference_from_iter, 
//DATA(old_cache_signal_type, 
//DATA(str_parent, 
