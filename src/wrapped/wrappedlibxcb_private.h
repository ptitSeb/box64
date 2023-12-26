#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(xcb_alloc_color, pFbuWWW)
//GO(xcb_alloc_color_cells, 
//GO(xcb_alloc_color_cells_masks, 
//GO(xcb_alloc_color_cells_masks_end, 
//GO(xcb_alloc_color_cells_masks_length, 
//GO(xcb_alloc_color_cells_pixels, 
//GO(xcb_alloc_color_cells_pixels_end, 
//GO(xcb_alloc_color_cells_pixels_length, 
//GO(xcb_alloc_color_cells_reply, 
//GO(xcb_alloc_color_cells_sizeof, 
//GO(xcb_alloc_color_cells_unchecked, 
//GO(xcb_alloc_color_planes, 
//GO(xcb_alloc_color_planes_pixels, 
//GO(xcb_alloc_color_planes_pixels_end, 
//GO(xcb_alloc_color_planes_pixels_length, 
//GO(xcb_alloc_color_planes_reply, 
//GO(xcb_alloc_color_planes_sizeof, 
//GO(xcb_alloc_color_planes_unchecked, 
GO(xcb_alloc_color_reply, pFbup)
//GO(xcb_alloc_color_unchecked, 
//GO(xcb_alloc_named_color, 
//GO(xcb_alloc_named_color_reply, 
//GO(xcb_alloc_named_color_sizeof, 
//GO(xcb_alloc_named_color_unchecked, 
//GO(xcb_allow_events, 
//GO(xcb_allow_events_checked, 
//GO(xcb_arc_end, 
//GO(xcb_arc_next, 
//GO(xcb_atom_end, 
//GO(xcb_atom_next, 
GO(xcb_bell, pFbC)
//GO(xcb_bell_checked, 
//GO(xcb_big_requests_enable, 
//GO(xcb_big_requests_enable_reply, 
//GO(xcb_big_requests_enable_unchecked, 
DATA(xcb_big_requests_id, sizeof(void*))
//GO(xcb_bool32_end, 
//GO(xcb_bool32_next, 
//GO(xcb_button_end, 
//GO(xcb_button_next, 
//GO(xcb_change_active_pointer_grab, 
//GO(xcb_change_active_pointer_grab_checked, 
GO(xcb_change_gc, pFbuup)
//GO(xcb_change_gc_aux, 
//GO(xcb_change_gc_aux_checked, 
GO(xcb_change_gc_checked, pFbuup)
//GO(xcb_change_gc_sizeof, 
//GO(xcb_change_gc_value_list, 
//GO(xcb_change_gc_value_list_serialize, 
//GO(xcb_change_gc_value_list_sizeof, 
//GO(xcb_change_gc_value_list_unpack, 
//GO(xcb_change_hosts, 
//GO(xcb_change_hosts_address, 
//GO(xcb_change_hosts_address_end, 
//GO(xcb_change_hosts_address_length, 
//GO(xcb_change_hosts_checked, 
//GO(xcb_change_hosts_sizeof, 
GO(xcb_change_keyboard_control, pFbup)
//GO(xcb_change_keyboard_control_aux, 
//GO(xcb_change_keyboard_control_aux_checked, 
//GO(xcb_change_keyboard_control_checked, 
//GO(xcb_change_keyboard_control_sizeof, 
//GO(xcb_change_keyboard_control_value_list, 
//GO(xcb_change_keyboard_control_value_list_serialize, 
//GO(xcb_change_keyboard_control_value_list_sizeof, 
//GO(xcb_change_keyboard_control_value_list_unpack, 
//GO(xcb_change_keyboard_mapping, 
//GO(xcb_change_keyboard_mapping_checked, 
//GO(xcb_change_keyboard_mapping_keysyms, 
//GO(xcb_change_keyboard_mapping_keysyms_end, 
//GO(xcb_change_keyboard_mapping_keysyms_length, 
//GO(xcb_change_keyboard_mapping_sizeof, 
//GO(xcb_change_pointer_control, 
//GO(xcb_change_pointer_control_checked, 
GO(xcb_change_property, uFbCuuuCup)
GO(xcb_change_property_checked, uFbCuuuCup)
//GO(xcb_change_property_data, 
//GO(xcb_change_property_data_end, 
//GO(xcb_change_property_data_length, 
//GO(xcb_change_property_sizeof, 
//GO(xcb_change_save_set, 
//GO(xcb_change_save_set_checked, 
GO(xcb_change_window_attributes, uFbuup)
//GO(xcb_change_window_attributes_aux, 
//GO(xcb_change_window_attributes_aux_checked, 
GO(xcb_change_window_attributes_checked, pFbuup)
//GO(xcb_change_window_attributes_sizeof, 
//GO(xcb_change_window_attributes_value_list, 
//GO(xcb_change_window_attributes_value_list_serialize, 
//GO(xcb_change_window_attributes_value_list_sizeof, 
//GO(xcb_change_window_attributes_value_list_unpack, 
//GO(xcb_char2b_end, 
//GO(xcb_char2b_next, 
//GO(xcb_charinfo_end, 
//GO(xcb_charinfo_next, 
//GO(xcb_circulate_window, 
//GO(xcb_circulate_window_checked, 
GO(xcb_clear_area, pFbCuwwWW)
//GO(xcb_clear_area_checked, 
//GO(xcb_client_message_data_end, 
//GO(xcb_client_message_data_next, 
GO(xcb_close_font, pFbu)
GO(xcb_close_font_checked, pFbu)
//GO(xcb_coloritem_end, 
//GO(xcb_coloritem_next, 
//GO(xcb_colormap_end, 
//GO(xcb_colormap_next, 
GO(xcb_configure_window, pFbuWp)
//GO(xcb_configure_window_aux, 
//GO(xcb_configure_window_aux_checked, 
//GO(xcb_configure_window_checked, 
//GO(xcb_configure_window_sizeof, 
//GO(xcb_configure_window_value_list, 
//GO(xcb_configure_window_value_list_serialize, 
//GO(xcb_configure_window_value_list_sizeof, 
//GO(xcb_configure_window_value_list_unpack, 
GOM(xcb_connect, pFEpp)
GO(xcb_connection_has_error, iFb)
//GO(xcb_connect_to_display_with_auth_info, 
//GO(xcb_connect_to_fd, 
GO(xcb_convert_selection, pFbuuuuu)
//GO(xcb_convert_selection_checked, 
GO(xcb_copy_area, pFbuuuwwwwWW)
GO(xcb_copy_area_checked, pFbuuuwwwwWW)
//GO(xcb_copy_colormap_and_free, 
//GO(xcb_copy_colormap_and_free_checked, 
//GO(xcb_copy_gc, 
//GO(xcb_copy_gc_checked, 
//GO(xcb_copy_plane, 
//GO(xcb_copy_plane_checked, 
GO(xcb_create_colormap, uFbCuuu)
GO(xcb_create_colormap_checked, uFbCuuu)
GO(xcb_create_cursor, pFbuuuWWWWWWWW)
//GO(xcb_create_cursor_checked, 
GO(xcb_create_gc, uFbuuup)
//GO(xcb_create_gc_aux, 
//GO(xcb_create_gc_aux_checked, 
GO(xcb_create_gc_checked, uFbuuup)
//GO(xcb_create_gc_sizeof, 
//GO(xcb_create_gc_value_list, 
//GO(xcb_create_gc_value_list_serialize, 
//GO(xcb_create_gc_value_list_sizeof, 
//GO(xcb_create_gc_value_list_unpack, 
GO(xcb_create_glyph_cursor, pFbuuuWWWWWWWW)
//GO(xcb_create_glyph_cursor_checked, 
GO(xcb_create_pixmap, uFbCuuWW)
GO(xcb_create_pixmap_checked, uFbCuuWW)
GO(xcb_create_window, uFbCuuwwWWWWuup)
//GO(xcb_create_window_aux, 
//GO(xcb_create_window_aux_checked, 
GO(xcb_create_window_checked, uFbCuuwwWWWWuup)
//GO(xcb_create_window_sizeof, 
//GO(xcb_create_window_value_list, 
//GO(xcb_create_window_value_list_serialize, 
//GO(xcb_create_window_value_list_sizeof, 
//GO(xcb_create_window_value_list_unpack, 
//GO(xcb_cursor_end, 
//GO(xcb_cursor_next, 
GO(xcb_delete_property, uFbuu)
GO(xcb_delete_property_checked, uFbuu)
//GO(xcb_depth_end, 
GO(xcb_depth_next, vFp)
GO(xcb_depth_sizeof, iFp)
GO(xcb_depth_visuals, pFp)
GO(xcb_depth_visuals_iterator, HFp)  //xcb_visualtype_iterator_t is a structure
//GO(xcb_depth_visuals_length, 
//GO(xcb_destroy_subwindows, 
//GO(xcb_destroy_subwindows_checked, 
GO(xcb_destroy_window, pFbu)
//GO(xcb_destroy_window_checked, 
GO(xcb_discard_reply, vFbu)
GO(xcb_discard_reply64, vFbU)
GOM(xcb_disconnect, vFEp)
//GO(xcb_drawable_end, 
//GO(xcb_drawable_next, 
//GO(xcb_fill_poly, 
//GO(xcb_fill_poly_checked, 
//GO(xcb_fill_poly_points, 
//GO(xcb_fill_poly_points_iterator, 
//GO(xcb_fill_poly_points_length, 
//GO(xcb_fill_poly_sizeof, 
GO(xcb_flush, iFb)
//GO(xcb_fontable_end, 
//GO(xcb_fontable_next, 
//GO(xcb_font_end, 
//GO(xcb_font_next, 
//GO(xcb_fontprop_end, 
//GO(xcb_fontprop_next, 
//GO(xcb_force_screen_saver, 
//GO(xcb_force_screen_saver_checked, 
//GO(xcb_format_end, 
GO(xcb_format_next, vFp)
GO(xcb_free_colormap, pFbu)
GO(xcb_free_colormap_checked, pFbu)
//GO(xcb_free_colors, 
//GO(xcb_free_colors_checked, 
//GO(xcb_free_colors_pixels, 
//GO(xcb_free_colors_pixels_end, 
//GO(xcb_free_colors_pixels_length, 
//GO(xcb_free_colors_sizeof, 
GO(xcb_free_cursor, pFbp)
//GO(xcb_free_cursor_checked, 
GO(xcb_free_gc, uFbu)
GO(xcb_free_gc_checked, uFbu)
GO(xcb_free_pixmap, uFbu)
GO(xcb_free_pixmap_checked, uFbu)
//GO(xcb_gcontext_end, 
//GO(xcb_gcontext_next, 
GO(xcb_generate_id, uFb)
GO(xcb_get_atom_name, uFbu)
GO(xcb_get_atom_name_name, pFp)
//GO(xcb_get_atom_name_name_end, 
GO(xcb_get_atom_name_name_length, iFp)
GO(xcb_get_atom_name_reply, pFbup)
//GO(xcb_get_atom_name_sizeof, 
//GO(xcb_get_atom_name_unchecked, 
GO(xcb_get_extension_data, pFbp)
GO(xcb_get_file_descriptor, iFb)
//GO(xcb_get_font_path, 
//GO(xcb_get_font_path_path_iterator, 
//GO(xcb_get_font_path_path_length, 
//GO(xcb_get_font_path_reply, 
//GO(xcb_get_font_path_sizeof, 
//GO(xcb_get_font_path_unchecked, 
GO(xcb_get_geometry, pFbu)
GO(xcb_get_geometry_reply, pFbup)
GO(xcb_get_geometry_unchecked, pFbu)
GO(xcb_get_image, pFbCuwwWWu)
GO(xcb_get_image_data, pFp)
//GO(xcb_get_image_data_end, 
GO(xcb_get_image_data_length, iFp)
GO(xcb_get_image_reply, pFbup)
//GO(xcb_get_image_sizeof, 
GO(xcb_get_image_unchecked, pFbCuwwWWu)
GO(xcb_get_input_focus, uFb)
GO(xcb_get_input_focus_reply, pFbup)
//GO(xcb_get_input_focus_unchecked, 
//GO(xcb_get_keyboard_control, 
//GO(xcb_get_keyboard_control_reply, 
//GO(xcb_get_keyboard_control_unchecked, 
GO(xcb_get_keyboard_mapping, pFbCC)
GO(xcb_get_keyboard_mapping_keysyms, pFp)
//GO(xcb_get_keyboard_mapping_keysyms_end, 
GO(xcb_get_keyboard_mapping_keysyms_length, iFp)
GO(xcb_get_keyboard_mapping_reply, pFbup)
//GO(xcb_get_keyboard_mapping_sizeof, 
//GO(xcb_get_keyboard_mapping_unchecked, 
GO(xcb_get_maximum_request_length, uFb)
GO(xcb_get_modifier_mapping, pFb)
GO(xcb_get_modifier_mapping_keycodes, pFp)
//GO(xcb_get_modifier_mapping_keycodes_end, 
GO(xcb_get_modifier_mapping_keycodes_length, iFp)
GO(xcb_get_modifier_mapping_reply, pFbup)
//GO(xcb_get_modifier_mapping_sizeof, pFp)
//GO(xcb_get_modifier_mapping_unchecked, 
//GO(xcb_get_motion_events, 
//GO(xcb_get_motion_events_events, 
//GO(xcb_get_motion_events_events_iterator, 
//GO(xcb_get_motion_events_events_length, 
//GO(xcb_get_motion_events_reply, 
//GO(xcb_get_motion_events_sizeof, 
//GO(xcb_get_motion_events_unchecked, 
//GO(xcb_get_pointer_control, 
//GO(xcb_get_pointer_control_reply, 
//GO(xcb_get_pointer_control_unchecked, 
//GO(xcb_get_pointer_mapping, 
//GO(xcb_get_pointer_mapping_map, 
//GO(xcb_get_pointer_mapping_map_end, 
//GO(xcb_get_pointer_mapping_map_length, 
//GO(xcb_get_pointer_mapping_reply, 
//GO(xcb_get_pointer_mapping_sizeof, 
//GO(xcb_get_pointer_mapping_unchecked, 
GO(xcb_get_property, uFbCuuuuu)
GO(xcb_get_property_reply, pFbup)
//GO(xcb_get_property_sizeof, 
GO(xcb_get_property_unchecked, uFbCuuuuu)
GO(xcb_get_property_value, pFp)
//GO(xcb_get_property_value_end, 
GO(xcb_get_property_value_length, iFp)
GO(xcb_get_reply_fds, pFbpu)
//GO(xcb_get_screen_saver, 
//GO(xcb_get_screen_saver_reply, 
//GO(xcb_get_screen_saver_unchecked, 
GO(xcb_get_selection_owner, uFbu)
GO(xcb_get_selection_owner_reply, pFbup)
GO(xcb_get_selection_owner_unchecked, uFbu)
GO(xcb_get_setup, pFb)
GO(xcb_get_window_attributes, uFbu)
GO(xcb_get_window_attributes_reply, pFbup)
GO(xcb_get_window_attributes_unchecked, uFbu)
GO(xcb_grab_button, pFbCuWCCuuCW)
GO(xcb_grab_button_checked, pFbCuWCCuuCW)
GO(xcb_grab_key, pFbCuWCCC)
GO(xcb_grab_keyboard, pFbCuuCC)
GO(xcb_grab_keyboard_reply, pFbup)
//GO(xcb_grab_keyboard_unchecked, 
GO(xcb_grab_key_checked, pFbCuWCCC)
GO(xcb_grab_pointer, pFbCuWCCuuu)
GO(xcb_grab_pointer_reply, pFbup)
//GO(xcb_grab_pointer_unchecked, 
GO(xcb_grab_server, uFb)
GO(xcb_grab_server_checked, uFb)
//GO(xcb_host_address, 
//GO(xcb_host_address_end, 
//GO(xcb_host_address_length, 
//GO(xcb_host_end, 
//GO(xcb_host_next, 
//GO(xcb_host_sizeof, 
//GO(xcb_image_text_16, 
//GO(xcb_image_text_16_checked, 
//GO(xcb_image_text_16_sizeof, 
//GO(xcb_image_text_16_string, 
//GO(xcb_image_text_16_string_iterator, 
//GO(xcb_image_text_16_string_length, 
GO(xcb_image_text_8, pFbCuuwwp)
GO(xcb_image_text_8_checked, pFbCuuwwp)
//GO(xcb_image_text_8_sizeof, 
//GO(xcb_image_text_8_string, 
//GO(xcb_image_text_8_string_end, 
//GO(xcb_image_text_8_string_length, 
//GO(xcb_install_colormap, 
//GO(xcb_install_colormap_checked, 
GO(xcb_intern_atom, uFbCWp)
GO(xcb_intern_atom_reply, pFbup)
//GO(xcb_intern_atom_sizeof, 
GO(xcb_intern_atom_unchecked, uFbCWp)
//GO(xcb_keycode32_end, 
//GO(xcb_keycode32_next, 
//GO(xcb_keycode_end, 
//GO(xcb_keycode_next, 
//GO(xcb_keysym_end, 
//GO(xcb_keysym_next, 
//GO(xcb_kill_client, 
//GO(xcb_kill_client_checked, 
//GO(xcb_list_extensions, 
//GO(xcb_list_extensions_names_iterator, 
//GO(xcb_list_extensions_names_length, 
//GO(xcb_list_extensions_reply, 
//GO(xcb_list_extensions_sizeof, 
//GO(xcb_list_extensions_unchecked, 
//GO(xcb_list_fonts, 
//GO(xcb_list_fonts_names_iterator, 
//GO(xcb_list_fonts_names_length, 
//GO(xcb_list_fonts_reply, 
//GO(xcb_list_fonts_sizeof, 
//GO(xcb_list_fonts_unchecked, 
//GO(xcb_list_fonts_with_info, 
//GO(xcb_list_fonts_with_info_name, 
//GO(xcb_list_fonts_with_info_name_end, 
//GO(xcb_list_fonts_with_info_name_length, 
//GO(xcb_list_fonts_with_info_properties, 
//GO(xcb_list_fonts_with_info_properties_iterator, 
//GO(xcb_list_fonts_with_info_properties_length, 
//GO(xcb_list_fonts_with_info_reply, 
//GO(xcb_list_fonts_with_info_sizeof, 
//GO(xcb_list_fonts_with_info_unchecked, 
//GO(xcb_list_hosts, 
//GO(xcb_list_hosts_hosts_iterator, 
//GO(xcb_list_hosts_hosts_length, 
//GO(xcb_list_hosts_reply, 
//GO(xcb_list_hosts_sizeof, 
//GO(xcb_list_hosts_unchecked, 
//GO(xcb_list_installed_colormaps, 
//GO(xcb_list_installed_colormaps_cmaps, 
//GO(xcb_list_installed_colormaps_cmaps_end, 
//GO(xcb_list_installed_colormaps_cmaps_length, 
//GO(xcb_list_installed_colormaps_reply, 
//GO(xcb_list_installed_colormaps_sizeof, 
//GO(xcb_list_installed_colormaps_unchecked, 
//GO(xcb_list_properties, 
//GO(xcb_list_properties_atoms, 
//GO(xcb_list_properties_atoms_end, 
//GO(xcb_list_properties_atoms_length, 
//GO(xcb_list_properties_reply, 
//GO(xcb_list_properties_sizeof, 
//GO(xcb_list_properties_unchecked, 
//GO(xcb_lookup_color, 
//GO(xcb_lookup_color_reply, 
//GO(xcb_lookup_color_sizeof, 
//GO(xcb_lookup_color_unchecked, 
GO(xcb_map_subwindows, uFbu)
//GO(xcb_map_subwindows_checked, 
GO(xcb_map_window, uFbu)
GO(xcb_map_window_checked, uFbu)
//GO(xcb_no_operation, 
//GO(xcb_no_operation_checked, 
GO(xcb_open_font, uFbuWp)
GO(xcb_open_font_checked, uFbuWp)
//GO(xcb_open_font_name, 
//GO(xcb_open_font_name_end, 
//GO(xcb_open_font_name_length, 
//GO(xcb_open_font_sizeof, 
GO(xcb_parse_display, iFpppp)
//GO(xcb_pixmap_end, 
//GO(xcb_pixmap_next, 
//GO(xcb_point_end, 
//GO(xcb_point_next, 
GO(xcb_poll_for_event, pFb)
GO(xcb_poll_for_queued_event, pFb)
GO(xcb_poll_for_reply, iFbupp)
//GO(xcb_poll_for_reply64, 
GO(xcb_poll_for_special_event, pFbp)
GO(xcb_poly_arc, pFbuuup)
//GO(xcb_poly_arc_arcs, 
//GO(xcb_poly_arc_arcs_iterator, 
//GO(xcb_poly_arc_arcs_length, 
//GO(xcb_poly_arc_checked, 
//GO(xcb_poly_arc_sizeof, 
//GO(xcb_poly_fill_arc, 
//GO(xcb_poly_fill_arc_arcs, 
//GO(xcb_poly_fill_arc_arcs_iterator, 
//GO(xcb_poly_fill_arc_arcs_length, 
//GO(xcb_poly_fill_arc_checked, 
//GO(xcb_poly_fill_arc_sizeof, 
GO(xcb_poly_fill_rectangle, pFbuuup)
//GO(xcb_poly_fill_rectangle_checked, 
//GO(xcb_poly_fill_rectangle_rectangles, 
//GO(xcb_poly_fill_rectangle_rectangles_iterator, 
//GO(xcb_poly_fill_rectangle_rectangles_length, 
//GO(xcb_poly_fill_rectangle_sizeof, 
GO(xcb_poly_line, pFbCuuup)
GO(xcb_poly_line_checked, pFbCuuup)
//GO(xcb_poly_line_points, 
//GO(xcb_poly_line_points_iterator, 
//GO(xcb_poly_line_points_length, 
//GO(xcb_poly_line_sizeof, 
GO(xcb_poly_point, pFbCuuup)
//GO(xcb_poly_point_checked, 
//GO(xcb_poly_point_points, 
//GO(xcb_poly_point_points_iterator, 
//GO(xcb_poly_point_points_length, 
//GO(xcb_poly_point_sizeof, 
GO(xcb_poly_rectangle, pFbuuup)
//GO(xcb_poly_rectangle_checked, 
//GO(xcb_poly_rectangle_rectangles, 
//GO(xcb_poly_rectangle_rectangles_iterator, 
//GO(xcb_poly_rectangle_rectangles_length, 
//GO(xcb_poly_rectangle_sizeof, 
GO(xcb_poly_segment, pFbuuup)
//GO(xcb_poly_segment_checked, 
//GO(xcb_poly_segment_segments, 
//GO(xcb_poly_segment_segments_iterator, 
//GO(xcb_poly_segment_segments_length, 
//GO(xcb_poly_segment_sizeof, 
//GO(xcb_poly_text_16, 
//GO(xcb_poly_text_16_checked, 
//GO(xcb_poly_text_16_items, 
//GO(xcb_poly_text_16_items_end, 
//GO(xcb_poly_text_16_items_length, 
//GO(xcb_poly_text_16_sizeof, 
//GO(xcb_poly_text_8, 
//GO(xcb_poly_text_8_checked, 
//GO(xcb_poly_text_8_items, 
//GO(xcb_poly_text_8_items_end, 
//GO(xcb_poly_text_8_items_length, 
//GO(xcb_poly_text_8_sizeof, 
GO(xcb_popcount, iFu)
GO(xcb_prefetch_extension_data, vFbp)
GO(xcb_prefetch_maximum_request_length, vFb)
GO(xcb_put_image, pFbCuuWWwwCCup)
GO(xcb_put_image_checked, pFbCuuWWwwCCup)
//GO(xcb_put_image_data, 
//GO(xcb_put_image_data_end, 
//GO(xcb_put_image_data_length, 
//GO(xcb_put_image_sizeof, 
//GO(xcb_query_best_size, 
//GO(xcb_query_best_size_reply, 
//GO(xcb_query_best_size_unchecked, 
//GO(xcb_query_colors, 
//GO(xcb_query_colors_colors, 
//GO(xcb_query_colors_colors_iterator, 
//GO(xcb_query_colors_colors_length, 
//GO(xcb_query_colors_reply, 
//GO(xcb_query_colors_sizeof, 
//GO(xcb_query_colors_unchecked, 
//GO(xcb_query_extension, 
//GO(xcb_query_extension_reply, 
//GO(xcb_query_extension_sizeof, 
//GO(xcb_query_extension_unchecked, 
//GO(xcb_query_font, 
//GO(xcb_query_font_char_infos, 
//GO(xcb_query_font_char_infos_iterator, 
//GO(xcb_query_font_char_infos_length, 
//GO(xcb_query_font_properties, 
//GO(xcb_query_font_properties_iterator, 
//GO(xcb_query_font_properties_length, 
//GO(xcb_query_font_reply, 
//GO(xcb_query_font_sizeof, 
//GO(xcb_query_font_unchecked, 
GO(xcb_query_keymap, pFbp)
GO(xcb_query_keymap_reply, pFbup)
GO(xcb_query_keymap_unchecked, pFbp)
GO(xcb_query_pointer, uFbu)
GO(xcb_query_pointer_reply, pFbup)
GO(xcb_query_pointer_unchecked, uFbu)
GO(xcb_query_text_extents, pFbuup)
GO(xcb_query_text_extents_reply, pFbup)
//GO(xcb_query_text_extents_sizeof, 
//GO(xcb_query_text_extents_unchecked, 
GO(xcb_query_tree, pFbu)
GO(xcb_query_tree_children, pFp)
//GO(xcb_query_tree_children_end, 
GO(xcb_query_tree_children_length, iFp)
GO(xcb_query_tree_reply, pFbup)
//GO(xcb_query_tree_sizeof, 
GO(xcb_query_tree_unchecked, pFbu)
//GO(xcb_recolor_cursor, 
//GO(xcb_recolor_cursor_checked, 
//GO(xcb_rectangle_end, 
//GO(xcb_rectangle_next, 
GO(xcb_register_for_special_xge, pFbpup)
GO(xcb_reparent_window, pFbuuWW)
//GO(xcb_reparent_window_checked, 
GO(xcb_request_check, pFbu)
//GO(xcb_rgb_end, 
//GO(xcb_rgb_next, 
//GO(xcb_rotate_properties, 
//GO(xcb_rotate_properties_atoms, 
//GO(xcb_rotate_properties_atoms_end, 
//GO(xcb_rotate_properties_atoms_length, 
//GO(xcb_rotate_properties_checked, 
//GO(xcb_rotate_properties_sizeof, 
GO(xcb_screen_allowed_depths_iterator, HFp)  //xcb_depth_iterator_t is a structure
//GO(xcb_screen_allowed_depths_length, 
//GO(xcb_screen_end, 
GO(xcb_screen_next, vFp)
//GO(xcb_screen_sizeof, 
//GO(xcb_segment_end, 
//GO(xcb_segment_next, 
GO(xcb_send_event, uFbCuup)
GO(xcb_send_event_checked, uFbCuup)
GO(xcb_send_fd, vFbi)
GO(xcb_send_request, uFbipp)
GO(xcb_send_request64, UFbipp)
GO(xcb_send_request_with_fds, uFbippup)
GO(xcb_send_request_with_fds64, UFbippup)
//GO(xcb_set_access_control, 
//GO(xcb_set_access_control_checked, 
GO(xcb_set_clip_rectangles, pFbCpWWup)
GO(xcb_set_clip_rectangles_checked, pFbCpWWup)
//GO(xcb_set_clip_rectangles_rectangles, 
//GO(xcb_set_clip_rectangles_rectangles_iterator, 
//GO(xcb_set_clip_rectangles_rectangles_length, 
//GO(xcb_set_clip_rectangles_sizeof, 
//GO(xcb_set_close_down_mode, 
//GO(xcb_set_close_down_mode_checked, 
//GO(xcb_set_dashes, 
//GO(xcb_set_dashes_checked, 
//GO(xcb_set_dashes_dashes, 
//GO(xcb_set_dashes_dashes_end, 
//GO(xcb_set_dashes_dashes_length, 
//GO(xcb_set_dashes_sizeof, 
//GO(xcb_set_font_path, 
//GO(xcb_set_font_path_checked, 
//GO(xcb_set_font_path_font_iterator, 
//GO(xcb_set_font_path_font_length, 
//GO(xcb_set_font_path_sizeof, 
GO(xcb_set_input_focus, pFbCuu) // xcb_void_cookie_t is a struct with only 1 uint inside
//GO(xcb_set_input_focus_checked, 
//GO(xcb_set_modifier_mapping, 
//GO(xcb_set_modifier_mapping_reply, 
//GO(xcb_set_modifier_mapping_sizeof, 
//GO(xcb_set_modifier_mapping_unchecked, 
//GO(xcb_set_pointer_mapping, 
//GO(xcb_set_pointer_mapping_reply, 
//GO(xcb_set_pointer_mapping_sizeof, 
//GO(xcb_set_pointer_mapping_unchecked, 
//GO(xcb_set_screen_saver, 
//GO(xcb_set_screen_saver_checked, 
GO(xcb_set_selection_owner, pFbpppp)
GO(xcb_set_selection_owner_checked, pFbpppp)
//GO(xcb_setup_authenticate_end, 
//GO(xcb_setup_authenticate_next, 
//GO(xcb_setup_authenticate_reason, 
//GO(xcb_setup_authenticate_reason_end, 
//GO(xcb_setup_authenticate_reason_length, 
//GO(xcb_setup_authenticate_sizeof, 
//GO(xcb_setup_end, 
//GO(xcb_setup_failed_end, 
//GO(xcb_setup_failed_next, 
//GO(xcb_setup_failed_reason, 
//GO(xcb_setup_failed_reason_end, 
//GO(xcb_setup_failed_reason_length, 
//GO(xcb_setup_failed_sizeof, 
//GO(xcb_setup_next, 
GO(xcb_setup_pixmap_formats, pFp)
GO(xcb_setup_pixmap_formats_iterator, HFp)
GO(xcb_setup_pixmap_formats_length, iFp)
//GO(xcb_setup_request_authorization_protocol_data, 
//GO(xcb_setup_request_authorization_protocol_data_end, 
//GO(xcb_setup_request_authorization_protocol_data_length, 
//GO(xcb_setup_request_authorization_protocol_name, 
//GO(xcb_setup_request_authorization_protocol_name_end, 
//GO(xcb_setup_request_authorization_protocol_name_length, 
//GO(xcb_setup_request_end, 
//GO(xcb_setup_request_next, 
//GO(xcb_setup_request_sizeof, 
GO(xcb_setup_roots_iterator, HFp)
GO(xcb_setup_roots_length, iFp)
//GO(xcb_setup_sizeof, 
//GO(xcb_setup_vendor, 
//GO(xcb_setup_vendor_end, 
//GO(xcb_setup_vendor_length, 
//GO(xcb_store_colors, 
//GO(xcb_store_colors_checked, 
//GO(xcb_store_colors_items, 
//GO(xcb_store_colors_items_iterator, 
//GO(xcb_store_colors_items_length, 
//GO(xcb_store_colors_sizeof, 
//GO(xcb_store_named_color, 
//GO(xcb_store_named_color_checked, 
//GO(xcb_store_named_color_name, 
//GO(xcb_store_named_color_name_end, 
//GO(xcb_store_named_color_name_length, 
//GO(xcb_store_named_color_sizeof, 
//GO(xcb_str_end, 
//GO(xcb_str_name, 
//GO(xcb_str_name_end, 
//GO(xcb_str_name_length, 
//GO(xcb_str_next, 
GO(xcb_str_sizeof, iFp)
//GO(xcb_sumof, 
//GO(xcb_take_socket, 
//GO(xcb_timecoord_end, 
//GO(xcb_timecoord_next, 
//GO(xcb_timestamp_end, 
//GO(xcb_timestamp_next, 
GO(xcb_translate_coordinates, pFbuuWW)
GO(xcb_translate_coordinates_reply, pFbup)
GO(xcb_translate_coordinates_unchecked, pFbuuWW)
GO(xcb_ungrab_button, pFbCuW)
GO(xcb_ungrab_button_checked, pFbCuW)
GO(xcb_ungrab_key, pFbCuW)
GO(xcb_ungrab_keyboard, pFbu)
GO(xcb_ungrab_keyboard_checked, pFbu)
GO(xcb_ungrab_key_checked, pFbCuW)
GO(xcb_ungrab_pointer, pFbu)
//GO(xcb_ungrab_pointer_checked, 
GO(xcb_ungrab_server, uFb)
GO(xcb_ungrab_server_checked, uFb)
//GO(xcb_uninstall_colormap, 
//GO(xcb_uninstall_colormap_checked, 
//GO(xcb_unmap_subwindows, 
//GO(xcb_unmap_subwindows_checked, 
GO(xcb_unmap_window, pFbu)
//GO(xcb_unmap_window_checked, 
GO(xcb_unregister_for_special_event, vFbp)
//GO(xcb_visualid_end, 
//GO(xcb_visualid_next, 
//GO(xcb_visualtype_end, 
GO(xcb_visualtype_next, vFp)
GO(xcb_wait_for_event, pFb)
GO(xcb_wait_for_reply, pFbup)
GO(xcb_wait_for_reply64, pFbUp)
GO(xcb_wait_for_special_event, pFbp)
GO(xcb_warp_pointer, pFbuuwwWWww)
//GO(xcb_warp_pointer_checked, 
//GO(xcb_window_end, 
//GO(xcb_window_next, 
//GO(xcb_writev, 
//GO(xcb_xc_misc_get_version, 
//GO(xcb_xc_misc_get_version_reply, 
//GO(xcb_xc_misc_get_version_unchecked, 
//GO(xcb_xc_misc_get_xid_list, 
//GO(xcb_xc_misc_get_xid_list_ids, 
//GO(xcb_xc_misc_get_xid_list_ids_end, 
//GO(xcb_xc_misc_get_xid_list_ids_length, 
//GO(xcb_xc_misc_get_xid_list_reply, 
//GO(xcb_xc_misc_get_xid_list_sizeof, 
//GO(xcb_xc_misc_get_xid_list_unchecked, 
//GO(xcb_xc_misc_get_xid_range, 
//GO(xcb_xc_misc_get_xid_range_reply, 
//GO(xcb_xc_misc_get_xid_range_unchecked, 
DATA(xcb_xc_misc_id, 4)
