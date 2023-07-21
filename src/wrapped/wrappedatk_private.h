#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(atk_action_do_action, iFpi)
GO(atk_action_get_description, iFpip)
GO(atk_action_get_keybinding, pFpi)
GO(atk_action_get_localized_name, pFpi)
GO(atk_action_get_n_actions, iFp)
GO(atk_action_get_name, pFpi)
GO(atk_action_get_type, LFv)
GO(atk_action_set_description, iFpip)
GOM(atk_add_focus_tracker, uFEp)
GOM(atk_add_global_event_listener, uFEpp)
GOM(atk_add_key_event_listener, uFEpp)
GO(atk_attribute_set_free, vFp)
//GOM(atk_component_add_focus_handler, uFpp)
GO(atk_component_contains, iFpiii)
GO(atk_component_get_alpha, dFp)
GO(atk_component_get_extents, vFpppppu)
GO(atk_component_get_layer, uFp)
GO(atk_component_get_mdi_zorder, iFp)
GO(atk_component_get_position, vFpppu)
GO(atk_component_get_size, vFppp)
GO(atk_component_get_type, LFv)
GO(atk_component_grab_focus, iFp)
GO(atk_component_ref_accessible_at_point, pFpiiu)
GO(atk_component_remove_focus_handler, vFpu)
GO(atk_component_scroll_to_point, iFpuii)
GO(atk_component_set_extents, iFpiiiiu)
GO(atk_component_set_position, iFpiiu)
GO(atk_component_set_size, iFpii)
GO(atk_coord_type_get_type, LFv)
GO(atk_document_get_attributes, pFp)
GO(atk_document_get_attribute_value, pFpp)
GO(atk_document_get_document, pFp)
//GO(atk_document_get_document_type, 
GO(atk_document_get_locale, pFp)
GO(atk_document_get_type, LFv)
GO(atk_document_set_attribute_value, iFppp)
//GO(atk_editable_text_copy_text, 
//GO(atk_editable_text_cut_text, 
GO(atk_editable_text_delete_text, vFpii)
GO(atk_editable_text_get_type, LFv)
GO(atk_editable_text_insert_text, vFppip)
GO(atk_editable_text_paste_text, vFpi)
GO(atk_editable_text_set_run_attributes, iFppii)
GO(atk_editable_text_set_text_contents, vFpp)
GOM(atk_focus_tracker_init, vFEp)
GO(atk_focus_tracker_notify, vFp)
//GO(atk_get_binary_age, 
GO(atk_get_default_registry, pFv)
GO(atk_get_focus_object, pFv)
GO(atk_get_interface_age, uFv)
//GO(atk_get_major_version, 
//GO(atk_get_micro_version, 
//GO(atk_get_minor_version, 
GO(atk_get_root, pFv)
GO(atk_get_toolkit_name, pFv)
GO(atk_get_toolkit_version, pFv)
GO(atk_get_version, pFv)
//GO(atk_gobject_accessible_for_object, 
GO(atk_gobject_accessible_get_object, pFp)
//GO(atk_gobject_accessible_get_type, 
//GO(atk_hyperlink_get_end_index, 
GO(atk_hyperlink_get_n_anchors, iFp)
GO(atk_hyperlink_get_object, pFpi)
GO(atk_hyperlink_get_start_index, iFp)
GO(atk_hyperlink_get_type, LFv)
GO(atk_hyperlink_get_uri, pFpi)
GO(atk_hyperlink_impl_get_hyperlink, pFp)
GO(atk_hyperlink_impl_get_type, LFv)
//GO(atk_hyperlink_is_inline, 
GO(atk_hyperlink_is_selected_link, iFp)
//GO(atk_hyperlink_is_valid, 
//GO(atk_hyperlink_state_flags_get_type, 
GO(atk_hypertext_get_link, pFpi)
GO(atk_hypertext_get_link_index, iFpi)
GO(atk_hypertext_get_n_links, iFp)
GO(atk_hypertext_get_type, LFv)
//GO(atk_image_get_image_description, 
GO(atk_image_get_image_locale, pFp)
GO(atk_image_get_image_position, vFpppu)
GO(atk_image_get_image_size, vFppp)
GO(atk_image_get_type, LFv)
GO(atk_image_set_image_description, iFpp)
GO(atk_implementor_get_type, LFv)
GO(atk_implementor_ref_accessible, pFp)
GO(atk_key_event_type_get_type, LFv)
GO(atk_layer_get_type, LFv)
//GO(atk_marshal_VOID__INT_INT, 
//GO(atk_marshal_VOID__INT_INT_INT_STRING, 
//GO(atk_marshal_VOID__INT_INT_STRING, 
//GO(atk_marshal_VOID__STRING_BOOLEAN, 
GO(atk_misc_get_instance, pFv)
GO(atk_misc_get_type, LFv)
GO(atk_misc_threads_enter, vFp)
GO(atk_misc_threads_leave, vFp)
DATA(atk_misc_instance, sizeof(void*))
//GO(atk_no_op_object_factory_get_type, 
GO(atk_no_op_object_factory_new, pFv)
GO(atk_no_op_object_get_type, LFv)
GO(atk_no_op_object_new, pFp)
GO(atk_object_add_relationship, iFpip)
//GOM(atk_object_connect_property_change_handler, uFEpp)
GO(atk_object_factory_create_accessible, pFpp)
GO(atk_object_factory_get_accessible_type, LFp)
GO(atk_object_factory_get_type, LFv)
//GO(atk_object_factory_invalidate, 
GO(atk_object_get_attributes, pFp)
GO(atk_object_get_description, pFp)
GO(atk_object_get_index_in_parent, iFp)
GO(atk_object_get_layer, uFp)
GO(atk_object_get_mdi_zorder, iFp)
GO(atk_object_get_n_accessible_children, iFp)
GO(atk_object_get_name, pFp)
GO(atk_object_get_object_locale, pFp)
GO(atk_object_get_parent, pFp)
GO(atk_object_get_role, uFp)
GO(atk_object_get_type, LFv)
GO(atk_object_initialize, vFpp)
GO(atk_object_notify_state_change, vFpLi)
GO(atk_object_ref_accessible_child, pFpi)
GO(atk_object_ref_relation_set, pFp)
GO(atk_object_ref_state_set, pFp)
GO(atk_object_remove_property_change_handler, vFpu)
GO(atk_object_remove_relationship, iFpup)
GO(atk_object_set_description, vFpp)
GO(atk_object_set_name, vFpp)
GO(atk_object_set_parent, vFpp)
GO(atk_object_set_role, vFpu)
GO(atk_plug_get_id, pFp)
//GO(atk_plug_get_type, 
//GO(atk_plug_new, 
//GO(atk_rectangle_get_type, 
GO(atk_registry_get_factory, pFpL)
GO(atk_registry_get_factory_type, LFpL)
//GO(atk_registry_get_type, 
GO(atk_registry_set_factory_type, vFpLL)
GO(atk_relation_add_target, vFpp)
GO(atk_relation_get_relation_type, uFp)
GO(atk_relation_get_target, pFp)
//GO(atk_relation_get_type, 
GO(atk_relation_new, pFpiu)
GO(atk_relation_remove_target, iFpp)
GO(atk_relation_set_add, vFpp)
GO(atk_relation_set_add_relation_by_type, vFpup)
GO(atk_relation_set_contains, iFpu)
GO(atk_relation_set_contains_target, iFpup)
GO(atk_relation_set_get_n_relations, iFp)
GO(atk_relation_set_get_relation, pFpi)
GO(atk_relation_set_get_relation_by_type, pFpu)
//GO(atk_relation_set_get_type, 
GO(atk_relation_set_new, pFv)
//GO(atk_relation_set_remove, 
GO(atk_relation_type_for_name, uFp)
GO(atk_relation_type_get_name, pFu)
GO(atk_relation_type_get_type, LFv)
//GO(atk_relation_type_register, 
GO(atk_remove_focus_tracker, vFu)
GO(atk_remove_global_event_listener, vFu)
GO(atk_remove_key_event_listener, vFu)
GO(atk_role_for_name, iFp)
GO(atk_role_get_localized_name, pFu)
GO(atk_role_get_name, pFp)
GO(atk_role_get_type, LFv)
GO(atk_role_register, uFp)
GO(atk_selection_add_selection, iFpi)
GO(atk_selection_clear_selection, iFp)
GO(atk_selection_get_selection_count, iFp)
GO(atk_selection_get_type, LFv)
GO(atk_selection_is_child_selected, iFpi)
GO(atk_selection_ref_selection, pFpi)
GO(atk_selection_remove_selection, iFpi)
GO(atk_selection_select_all_selection, iFp)
GO(atk_socket_embed, vFpp)
GO(atk_socket_get_type, iFv)
GO(atk_socket_is_occupied, iFp)
GO(atk_socket_new, pFv)
GO(atk_state_set_add_state, iFpi)
GO(atk_state_set_add_states, vFppi)
//GO(atk_state_set_and_sets, 
//GO(atk_state_set_clear_states, 
GO(atk_state_set_contains_state, iFpi)
GO(atk_state_set_contains_states, iFppi)
//GO(atk_state_set_get_type, 
GO(atk_state_set_is_empty, iFp)
GO(atk_state_set_new, pFv)
//GO(atk_state_set_or_sets, 
GO(atk_state_set_remove_state, iFpu)
GO(atk_state_set_xor_sets, pFpp)
GO(atk_state_type_for_name, uFp)
GO(atk_state_type_get_name, pFu)
GO(atk_state_type_get_type, LFv)
//GO(atk_state_type_register, 
GO(atk_streamable_content_get_mime_type, pFpi)
GO(atk_streamable_content_get_n_mime_types, iFp)
//GO(atk_streamable_content_get_stream, 
//GO(atk_streamable_content_get_type, 
GO(atk_streamable_content_get_uri, pFpp)
//GO(atk_table_add_column_selection, 
//GO(atk_table_add_row_selection, 
GO(atk_table_get_caption, pFp)
//GO(atk_table_get_column_at_index, 
GO(atk_table_get_column_description, pFpi)
GO(atk_table_get_column_extent_at, iFpii)
//GO(atk_table_get_column_header, 
//GO(atk_table_get_index_at, 
GO(atk_table_get_n_columns, iFp)
GO(atk_table_get_n_rows, iFp)
GO(atk_table_get_row_at_index, iFpi)
GO(atk_table_get_row_description, pFpi)
GO(atk_table_get_row_extent_at, iFpii)
GO(atk_table_get_row_header, pFpi)
//GO(atk_table_get_selected_columns, 
GO(atk_table_get_selected_rows, iFpp)
GO(atk_table_get_summary, pFp)
GO(atk_table_get_type, LFv)
//GO(atk_table_is_column_selected, 
//GO(atk_table_is_row_selected, 
GO(atk_table_is_selected, iFpii)
GO(atk_table_ref_at, pFpii)
GO(atk_table_remove_column_selection, iFpi)
//GO(atk_table_remove_row_selection, 
//GO(atk_table_set_caption, 
//GO(atk_table_set_column_description, 
//GO(atk_table_set_column_header, 
GO(atk_table_set_row_description, vFpip)
GO(atk_table_set_row_header, vFpip)
GO(atk_table_set_summary, vFpp)
GO(atk_text_add_selection, iFpii)
GO(atk_text_attribute_for_name, uFp)
GO(atk_text_attribute_get_name, pFu)
//GO(atk_text_attribute_get_type, 
GO(atk_text_attribute_get_value, pFui)
//GO(atk_text_attribute_register, 
//GO(atk_text_boundary_get_type, 
//GO(atk_text_clip_type_get_type, 
GO(atk_text_free_ranges, vFp)
GO(atk_text_get_bounded_ranges, pFppuuu)
GO(atk_text_get_caret_offset, iFp)
GO(atk_text_get_character_at_offset, uFpi)
GO(atk_text_get_character_count, iFp)
GO(atk_text_get_character_extents, vFpippppu)
GO(atk_text_get_default_attributes, pFp)
GO(atk_text_get_n_selections, iFp)
GO(atk_text_get_offset_at_point, iFpiiu)
GO(atk_text_get_range_extents, vFpiiup)
GO(atk_text_get_run_attributes, pFpipp)
GO(atk_text_get_selection, pFpipp)
GO(atk_text_get_text, pFpii)
//GO(atk_text_get_text_after_offset, 
//GO(atk_text_get_text_at_offset, 
GO(atk_text_get_text_before_offset, pFpiupp)
GO(atk_text_get_type, LFv)
GO(atk_text_range_get_type, LFv)
//GO(atk_text_remove_selection, 
GO(atk_text_scroll_substring_to_point, iFpiiuii)
GO(atk_text_set_caret_offset, iFpi)
GO(atk_text_set_selection, iFpiii)
GO(atk_util_get_type, LFv)
GO(atk_value_get_current_value, vFpp)
GO(atk_value_get_maximum_value, vFpp)
GO(atk_value_get_minimum_increment, vFpp)
GO(atk_value_get_minimum_value, vFpp)
GO(atk_value_get_type, LFv)
GO(atk_value_set_current_value, iFpp)
GO(atk_window_get_type, LFv)
