#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(atk_action_do_action, iFpi)
GO(atk_action_get_description, iFpip)
GO(atk_action_get_keybinding, pFpi)
GO(atk_action_get_localized_name, pFpi)
GO(atk_action_get_n_actions, iFp)
GO(atk_action_get_name, pFpi)
GO(atk_action_get_type, iFv)
GO(atk_action_set_description, pFpi)
GOM(atk_add_focus_tracker, uFEp)
//GOM(atk_add_global_event_listener, uFEpp)
GOM(atk_add_key_event_listener, uFEpp)
//GO(atk_attribute_set_free, 
//GOM(atk_component_add_focus_handler, uFpp)
GO(atk_component_contains, iFpiii)
GO(atk_component_get_alpha, dFp)
GO(atk_component_get_extents, vFpppppi)
GO(atk_component_get_layer, iFp)
GO(atk_component_get_mdi_zorder, iFp)
GO(atk_component_get_position, vFpppi)
GO(atk_component_get_size, vFppp)
GO(atk_component_get_type, iFv)
GO(atk_component_grab_focus, iFp)
GO(atk_component_ref_accessible_at_point, pFpiii)
GO(atk_component_remove_focus_handler, vFpu)
GO(atk_component_set_extents, iFpiiiii)
GO(atk_component_set_position, iFpiii)
GO(atk_component_set_size, iFpii)
GO(atk_coord_type_get_type, iFv)
//GO(atk_document_get_attributes, 
//GO(atk_document_get_attribute_value, 
//GO(atk_document_get_document, 
//GO(atk_document_get_document_type, 
//GO(atk_document_get_locale, 
//GO(atk_document_get_type, 
//GO(atk_document_set_attribute_value, 
//GO(atk_editable_text_copy_text, 
//GO(atk_editable_text_cut_text, 
//GO(atk_editable_text_delete_text, 
//GO(atk_editable_text_get_type, 
//GO(atk_editable_text_insert_text, 
//GO(atk_editable_text_paste_text, 
//GO(atk_editable_text_set_run_attributes, 
//GO(atk_editable_text_set_text_contents, 
GOM(atk_focus_tracker_init, vFEp)
GO(atk_focus_tracker_notify, vFp)
//GO(atk_get_binary_age, 
GO(atk_get_default_registry, pFv)
GO(atk_get_focus_object, pFv)
//GO(atk_get_interface_age, 
//GO(atk_get_major_version, 
//GO(atk_get_micro_version, 
//GO(atk_get_minor_version, 
GO(atk_get_root, pFv)
GO(atk_get_toolkit_name, pFv)
GO(atk_get_toolkit_version, pFv)
GO(atk_get_version, pFv)
//GO(atk_gobject_accessible_for_object, 
//GO(atk_gobject_accessible_get_object, 
//GO(atk_gobject_accessible_get_type, 
//GO(atk_hyperlink_get_end_index, 
//GO(atk_hyperlink_get_n_anchors, 
//GO(atk_hyperlink_get_object, 
//GO(atk_hyperlink_get_start_index, 
//GO(atk_hyperlink_get_type, 
//GO(atk_hyperlink_get_uri, 
//GO(atk_hyperlink_impl_get_hyperlink, 
//GO(atk_hyperlink_impl_get_type, 
//GO(atk_hyperlink_is_inline, 
//GO(atk_hyperlink_is_selected_link, 
//GO(atk_hyperlink_is_valid, 
//GO(atk_hyperlink_state_flags_get_type, 
//GO(atk_hypertext_get_link, 
//GO(atk_hypertext_get_link_index, 
//GO(atk_hypertext_get_n_links, 
//GO(atk_hypertext_get_type, 
//GO(atk_image_get_image_description, 
//GO(atk_image_get_image_locale, 
//GO(atk_image_get_image_position, 
//GO(atk_image_get_image_size, 
//GO(atk_image_get_type, 
//GO(atk_image_set_image_description, 
GO(atk_implementor_get_type, iFv)
GO(atk_implementor_ref_accessible, pFp)
//GO(atk_key_event_type_get_type, 
//GO(atk_layer_get_type, 
//GO(atk_marshal_VOID__INT_INT, 
//GO(atk_marshal_VOID__INT_INT_INT_STRING, 
//GO(atk_marshal_VOID__INT_INT_STRING, 
//GO(atk_marshal_VOID__STRING_BOOLEAN, 
GO(atk_misc_get_instance, pFv)
GO(atk_misc_get_type, iFv)
GO(atk_misc_threads_enter, vFp)
GO(atk_misc_threads_leave, vFp)
DATA(atk_misc_instance, sizeof(void*))
//GO(atk_no_op_object_factory_get_type, 
//GO(atk_no_op_object_factory_new, 
//GO(atk_no_op_object_get_type, 
//GO(atk_no_op_object_new, 
GO(atk_object_add_relationship, iFpip)
//GOM(atk_object_connect_property_change_handler, uFEpp)
//GO(atk_object_factory_create_accessible, 
GO(atk_object_factory_get_accessible_type, iFp)
GO(atk_object_factory_get_type, iFv)
//GO(atk_object_factory_invalidate, 
GO(atk_object_get_attributes, pFp)
GO(atk_object_get_description, pFp)
GO(atk_object_get_index_in_parent, iFp)
GO(atk_object_get_layer, iFp)
GO(atk_object_get_mdi_zorder, iFp)
GO(atk_object_get_n_accessible_children, iFp)
GO(atk_object_get_name, pFp)
GO(atk_object_get_object_locale, pFp)
GO(atk_object_get_parent, pFp)
GO(atk_object_get_role, iFp)
GO(atk_object_get_type, iFv)
GO(atk_object_initialize, vFpp)
GO(atk_object_notify_state_change, vFpii)
GO(atk_object_ref_accessible_child, pFpi)
GO(atk_object_ref_relation_set, pFp)
GO(atk_object_ref_state_set, pFp)
GO(atk_object_remove_property_change_handler, vFpu)
GO(atk_object_remove_relationship, iFpip)
GO(atk_object_set_description, vFpp)
GO(atk_object_set_name, vFpp)
GO(atk_object_set_parent, vFpp)
GO(atk_object_set_role, vFpi)
//GO(atk_plug_get_id, 
//GO(atk_plug_get_type, 
//GO(atk_plug_new, 
//GO(atk_rectangle_get_type, 
GO(atk_registry_get_factory, pFpi)
GO(atk_registry_get_factory_type, iFpi)
//GO(atk_registry_get_type, 
GO(atk_registry_set_factory_type, vFpii)
//GO(atk_relation_add_target, 
//GO(atk_relation_get_relation_type, 
//GO(atk_relation_get_target, 
//GO(atk_relation_get_type, 
//GO(atk_relation_new, 
//GO(atk_relation_remove_target, 
//GO(atk_relation_set_add, 
//GO(atk_relation_set_add_relation_by_type, 
//GO(atk_relation_set_contains, 
//GO(atk_relation_set_contains_target, 
//GO(atk_relation_set_get_n_relations, 
//GO(atk_relation_set_get_relation, 
//GO(atk_relation_set_get_relation_by_type, 
//GO(atk_relation_set_get_type, 
//GO(atk_relation_set_new, 
//GO(atk_relation_set_remove, 
//GO(atk_relation_type_for_name, 
//GO(atk_relation_type_get_name, 
//GO(atk_relation_type_get_type, 
//GO(atk_relation_type_register, 
GO(atk_remove_focus_tracker, vFu)
GO(atk_remove_global_event_listener, vFu)
GO(atk_remove_key_event_listener, vFu)
GO(atk_role_for_name, iFp)
GO(atk_role_get_localized_name, pFp)
GO(atk_role_get_name, pFp)
GO(atk_role_get_type, iFv)
GO(atk_role_register, iFp)
//GO(atk_selection_add_selection, 
//GO(atk_selection_clear_selection, 
//GO(atk_selection_get_selection_count, 
//GO(atk_selection_get_type, 
//GO(atk_selection_is_child_selected, 
//GO(atk_selection_ref_selection, 
//GO(atk_selection_remove_selection, 
//GO(atk_selection_select_all_selection, 
//GO(atk_socket_embed, 
//GO(atk_socket_get_type, 
//GO(atk_socket_is_occupied, 
//GO(atk_socket_new, 
//GO(atk_state_set_add_state, 
//GO(atk_state_set_add_states, 
//GO(atk_state_set_and_sets, 
//GO(atk_state_set_clear_states, 
//GO(atk_state_set_contains_state, 
//GO(atk_state_set_contains_states, 
//GO(atk_state_set_get_type, 
//GO(atk_state_set_is_empty, 
//GO(atk_state_set_new, 
//GO(atk_state_set_or_sets, 
//GO(atk_state_set_remove_state, 
//GO(atk_state_set_xor_sets, 
//GO(atk_state_type_for_name, 
//GO(atk_state_type_get_name, 
//GO(atk_state_type_get_type, 
//GO(atk_state_type_register, 
//GO(atk_streamable_content_get_mime_type, 
//GO(atk_streamable_content_get_n_mime_types, 
//GO(atk_streamable_content_get_stream, 
//GO(atk_streamable_content_get_type, 
//GO(atk_streamable_content_get_uri, 
//GO(atk_table_add_column_selection, 
//GO(atk_table_add_row_selection, 
//GO(atk_table_get_caption, 
//GO(atk_table_get_column_at_index, 
//GO(atk_table_get_column_description, 
//GO(atk_table_get_column_extent_at, 
//GO(atk_table_get_column_header, 
//GO(atk_table_get_index_at, 
//GO(atk_table_get_n_columns, 
//GO(atk_table_get_n_rows, 
//GO(atk_table_get_row_at_index, 
//GO(atk_table_get_row_description, 
//GO(atk_table_get_row_extent_at, 
//GO(atk_table_get_row_header, 
//GO(atk_table_get_selected_columns, 
//GO(atk_table_get_selected_rows, 
//GO(atk_table_get_summary, 
//GO(atk_table_get_type, 
//GO(atk_table_is_column_selected, 
//GO(atk_table_is_row_selected, 
//GO(atk_table_is_selected, 
//GO(atk_table_ref_at, 
//GO(atk_table_remove_column_selection, 
//GO(atk_table_remove_row_selection, 
//GO(atk_table_set_caption, 
//GO(atk_table_set_column_description, 
//GO(atk_table_set_column_header, 
//GO(atk_table_set_row_description, 
//GO(atk_table_set_row_header, 
//GO(atk_table_set_summary, 
//GO(atk_text_add_selection, 
//GO(atk_text_attribute_for_name, 
//GO(atk_text_attribute_get_name, 
//GO(atk_text_attribute_get_type, 
//GO(atk_text_attribute_get_value, 
//GO(atk_text_attribute_register, 
//GO(atk_text_boundary_get_type, 
//GO(atk_text_clip_type_get_type, 
//GO(atk_text_free_ranges, 
//GO(atk_text_get_bounded_ranges, 
//GO(atk_text_get_caret_offset, 
//GO(atk_text_get_character_at_offset, 
//GO(atk_text_get_character_count, 
//GO(atk_text_get_character_extents, 
//GO(atk_text_get_default_attributes, 
//GO(atk_text_get_n_selections, 
//GO(atk_text_get_offset_at_point, 
//GO(atk_text_get_range_extents, 
//GO(atk_text_get_run_attributes, 
//GO(atk_text_get_selection, 
//GO(atk_text_get_text, 
//GO(atk_text_get_text_after_offset, 
//GO(atk_text_get_text_at_offset, 
//GO(atk_text_get_text_before_offset, 
//GO(atk_text_get_type, 
//GO(atk_text_range_get_type, 
//GO(atk_text_remove_selection, 
//GO(atk_text_set_caret_offset, 
//GO(atk_text_set_selection, 
GO(atk_util_get_type, iFv)
//GO(atk_value_get_current_value, 
//GO(atk_value_get_maximum_value, 
//GO(atk_value_get_minimum_increment, 
//GO(atk_value_get_minimum_value, 
//GO(atk_value_get_type, 
//GO(atk_value_set_current_value, 
//GO(atk_window_get_type, 
