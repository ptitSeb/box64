#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(cairo_append_path, vFpp)
GO(cairo_arc, vFpddddd)
GO(cairo_arc_negative, vFpddddd)
GO(cairo_clip, vFp)
GO(cairo_clip_extents, vFppppp)
GO(cairo_clip_preserve, vFp)
GO(cairo_close_path, vFp)
GO(cairo_copy_clip_rectangle_list, pFp)
GO(cairo_copy_page, vFp)
GO(cairo_copy_path, pFp)
GO(cairo_copy_path_flat, pFp)
GO(cairo_create, pFp)
GO(cairo_curve_to, vFpdddddd)
//GO(cairo_debug_reset_static_data, 
GO(cairo_destroy, vFp)
//GO(cairo_device_acquire, 
//GO(cairo_device_destroy, 
//GO(cairo_device_finish, 
//GO(cairo_device_flush, 
//GO(cairo_device_get_reference_count, 
//GO(cairo_device_get_type, 
//GO(cairo_device_get_user_data, 
//GO(cairo_device_observer_elapsed, 
//GO(cairo_device_observer_fill_elapsed, 
//GO(cairo_device_observer_glyphs_elapsed, 
//GO(cairo_device_observer_mask_elapsed, 
//GO(cairo_device_observer_paint_elapsed, 
//GO(cairo_device_observer_print, 
//GO(cairo_device_observer_stroke_elapsed, 
//GO(cairo_device_reference, 
//GO(cairo_device_release, 
//GO(cairo_device_set_user_data, 
//GO(cairo_device_status, 
GO(cairo_device_to_user, vFppp)
GO(cairo_device_to_user_distance, vFppp)
GO(cairo_fill, vFp)
GO(cairo_fill_extents, vFppppp)
GO(cairo_fill_preserve, vFp)
GO(cairo_font_extents, vFpp)
//GO(cairo_font_face_destroy, 
//GO(cairo_font_face_get_reference_count, 
//GO(cairo_font_face_get_type, 
//GO(cairo_font_face_get_user_data, 
//GO(cairo_font_face_reference, 
//GO(cairo_font_face_set_user_data, 
//GO(cairo_font_face_status, 
GO(cairo_font_options_copy, pFp)
GO(cairo_font_options_create, pFv)
GO(cairo_font_options_destroy, vFp)
GO(cairo_font_options_equal, iFpp)
GO(cairo_font_options_get_antialias, iFp)
GO(cairo_font_options_get_hint_metrics, iFp)
GO(cairo_font_options_get_hint_style, iFp)
GO(cairo_font_options_get_subpixel_order, iFp)
GO(cairo_font_options_hash, LFp)
GO(cairo_font_options_merge, vFpp)
GO(cairo_font_options_set_antialias, vFpi)
GO(cairo_font_options_set_hint_metrics, vFpi)
GO(cairo_font_options_set_hint_style, vFpi)
GO(cairo_font_options_set_subpixel_order, vFpi)
GO(cairo_font_options_status, iFp)
GO(cairo_format_stride_for_width, iFii)
GO(cairo_ft_font_face_create_for_ft_face, pFpi)
GO(cairo_ft_font_face_create_for_pattern, pFp)
GO(cairo_ft_font_face_get_synthesize, uFp)
GO(cairo_ft_font_face_set_synthesize, vFpu)
GO(cairo_ft_font_face_unset_synthesize, vFpu)
GO(cairo_ft_font_options_substitute, vFpp)
GO(cairo_ft_scaled_font_lock_face, pFp)
GO(cairo_ft_scaled_font_unlock_face, vFp)
GO(cairo_get_antialias, iFp)
GO(cairo_get_current_point, vFppp)
GO(cairo_get_dash, vFppp)
GO(cairo_get_dash_count, iFp)
GO(cairo_get_fill_rule, iFp)
GO(cairo_get_font_face, pFp)
GO(cairo_get_font_matrix, vFpp)
GO(cairo_get_font_options, vFpp)
GO(cairo_get_group_target, pFp)
GO(cairo_get_line_cap, iFp)
GO(cairo_get_line_join, iFp)
GO(cairo_get_line_width, dFp)
GO(cairo_get_matrix, vFpp)
GO(cairo_get_miter_limit, dFp)
GO(cairo_get_operator, iFp)
GO(cairo_get_reference_count, uFp)
GO(cairo_get_scaled_font, pFp)
GO(cairo_get_source, pFp)
GO(cairo_get_target, pFp)
GO(cairo_get_tolerance, dFp)
GO(cairo_get_user_data, pFpp)
GO(cairo_glyph_allocate, pFi)
GO(cairo_glyph_extents, vFppip)
GO(cairo_glyph_free, vFp)
GO(cairo_glyph_path, vFppi)
GO(cairo_has_current_point, iFp)
GO(cairo_identity_matrix, vFp)
GO(cairo_image_surface_create, pFiii)
GO(cairo_image_surface_create_for_data, pFpiiii)
//GO(cairo_image_surface_create_from_png, 
//GO(cairo_image_surface_create_from_png_stream, 
GO(cairo_image_surface_get_data, pFp)
GO(cairo_image_surface_get_format, iFp)
GO(cairo_image_surface_get_height, iFp)
GO(cairo_image_surface_get_stride, iFp)
GO(cairo_image_surface_get_width, iFp)
GO(cairo_in_clip, iFpdd)
GO(cairo_in_fill, iFpdd)
GO(cairo_in_stroke, iFpdd)
GO(cairo_line_to, vFpdd)
GO(cairo_mask, vFpp)
GO(cairo_mask_surface, vFppdd)
GO(cairo_matrix_init, vFpdddddd)
GO(cairo_matrix_init_identity, vFp)
GO(cairo_matrix_init_rotate, vFpd)
GO(cairo_matrix_init_scale, vFpdd)
GO(cairo_matrix_init_translate, vFpdd)
GO(cairo_matrix_invert, iFp)
GO(cairo_matrix_multiply, vFppp)
GO(cairo_matrix_rotate, vFpd)
GO(cairo_matrix_scale, vFpdd)
GO(cairo_matrix_transform_distance, vFppp)
GO(cairo_matrix_transform_point, vFppp)
GO(cairo_matrix_translate, vFpdd)
GO(cairo_mesh_pattern_begin_patch, vFp)
GO(cairo_mesh_pattern_curve_to, vFpdddddd)
GO(cairo_mesh_pattern_end_patch, vFp)
GO(cairo_mesh_pattern_get_control_point, iFpuupp)
GO(cairo_mesh_pattern_get_corner_color_rgba, iFpuupppp)
GO(cairo_mesh_pattern_get_patch_count, iFpp)
GO(cairo_mesh_pattern_get_path, pFpu)
GO(cairo_mesh_pattern_line_to, vFpdd)
GO(cairo_mesh_pattern_move_to, vFpdd)
GO(cairo_mesh_pattern_set_control_point, vFpudd)
GO(cairo_mesh_pattern_set_corner_color_rgb, vFpuddd)
GO(cairo_mesh_pattern_set_corner_color_rgba, vFpudddd)
GO(cairo_move_to, vFpdd)
GO(cairo_new_path, vFp)
GO(cairo_new_sub_path, vFp)
GO(cairo_paint, vFp)
GO(cairo_paint_with_alpha, vFpd)
GO(cairo_path_destroy, vFp)
GO(cairo_path_extents, vFppppp)
GO(cairo_pattern_add_color_stop_rgb, vFpdddd)
GO(cairo_pattern_add_color_stop_rgba, vFpddddd)
GO(cairo_pattern_create_for_surface, pFp)
GO(cairo_pattern_create_linear, pFdddd)
GO(cairo_pattern_create_mesh, pFv)
GO(cairo_pattern_create_radial, pFdddddd)
//GO(cairo_pattern_create_raster_source, 
GO(cairo_pattern_create_rgb, pFddd)
GO(cairo_pattern_create_rgba, pFdddd)
GO(cairo_pattern_destroy, vFp)
GO(cairo_pattern_get_color_stop_count, iFpp)
GO(cairo_pattern_get_color_stop_rgba, iFpippppp)
GO(cairo_pattern_get_extend, iFp)
GO(cairo_pattern_get_filter, iFp)
GO(cairo_pattern_get_linear_points, iFppppp)
GO(cairo_pattern_get_matrix, vFpp)
GO(cairo_pattern_get_radial_circles, iFppppppp)
GO(cairo_pattern_get_reference_count, uFp)
GO(cairo_pattern_get_rgba, iFppppp)
GO(cairo_pattern_get_surface, iFpp)
GO(cairo_pattern_get_type, iFp)
GO(cairo_pattern_get_user_data, pFpp)
GO(cairo_pattern_reference, pFp)
GO(cairo_pattern_set_extend, vFpi)
GO(cairo_pattern_set_filter, vFpi)
GO(cairo_pattern_set_matrix, vFpp)
//GOM(cairo_pattern_set_user_data, iFEpppp)
GO(cairo_pattern_status, iFp)
//GO(cairo_pdf_get_versions, 
//GO(cairo_pdf_surface_create, 
//GO(cairo_pdf_surface_create_for_stream, 
//GO(cairo_pdf_surface_restrict_to_version, 
//GO(cairo_pdf_surface_set_size, 
//GO(cairo_pdf_version_to_string, 
GO(cairo_pop_group, pFp)
GO(cairo_pop_group_to_source, vFp)
//GO(cairo_ps_get_levels, 
//GO(cairo_ps_level_to_string, 
//GO(cairo_ps_surface_create, 
//GO(cairo_ps_surface_create_for_stream, 
//GO(cairo_ps_surface_dsc_begin_page_setup, 
//GO(cairo_ps_surface_dsc_begin_setup, 
//GO(cairo_ps_surface_dsc_comment, 
//GO(cairo_ps_surface_get_eps, 
//GO(cairo_ps_surface_restrict_to_level, 
//GO(cairo_ps_surface_set_eps, 
//GO(cairo_ps_surface_set_size, 
GO(cairo_push_group, vFp)
GO(cairo_push_group_with_content, vFpi)
//GO(cairo_raster_source_pattern_get_acquire, 
//GO(cairo_raster_source_pattern_get_callback_data, 
//GO(cairo_raster_source_pattern_get_copy, 
//GO(cairo_raster_source_pattern_get_finish, 
//GO(cairo_raster_source_pattern_get_snapshot, 
//GO(cairo_raster_source_pattern_set_acquire, 
//GO(cairo_raster_source_pattern_set_callback_data, 
//GO(cairo_raster_source_pattern_set_copy, 
//GO(cairo_raster_source_pattern_set_finish, 
//GO(cairo_raster_source_pattern_set_snapshot, 
//GO(cairo_recording_surface_create, 
//GO(cairo_recording_surface_get_extents, 
//GO(cairo_recording_surface_ink_extents, 
GO(cairo_rectangle, vFpdddd)
GO(cairo_rectangle_list_destroy, vFp)
GO(cairo_reference, pFp)
//GO(cairo_region_contains_point, 
//GO(cairo_region_contains_rectangle, 
//GO(cairo_region_copy, 
//GO(cairo_region_create, 
//GO(cairo_region_create_rectangle, 
//GO(cairo_region_create_rectangles, 
//GO(cairo_region_destroy, 
//GO(cairo_region_equal, 
//GO(cairo_region_get_extents, 
//GO(cairo_region_get_rectangle, 
//GO(cairo_region_intersect, 
//GO(cairo_region_intersect_rectangle, 
//GO(cairo_region_is_empty, 
//GO(cairo_region_num_rectangles, 
//GO(cairo_region_reference, 
//GO(cairo_region_status, 
//GO(cairo_region_subtract, 
//GO(cairo_region_subtract_rectangle, 
//GO(cairo_region_translate, 
//GO(cairo_region_union, 
//GO(cairo_region_union_rectangle, 
//GO(cairo_region_xor, 
//GO(cairo_region_xor_rectangle, 
GO(cairo_rel_curve_to, vFpdddddd)
GO(cairo_rel_line_to, vFpdd)
GO(cairo_rel_move_to, vFpdd)
GO(cairo_reset_clip, vFp)
GO(cairo_restore, vFp)
GO(cairo_rotate, vFpd)
GO(cairo_save, vFp)
GO(cairo_scale, vFpdd)
//GO(cairo_scaled_font_create, 
//GO(cairo_scaled_font_destroy, 
//GO(cairo_scaled_font_extents, 
//GO(cairo_scaled_font_get_ctm, 
//GO(cairo_scaled_font_get_font_face, 
//GO(cairo_scaled_font_get_font_matrix, 
//GO(cairo_scaled_font_get_font_options, 
//GO(cairo_scaled_font_get_reference_count, 
//GO(cairo_scaled_font_get_scale_matrix, 
//GO(cairo_scaled_font_get_type, 
//GO(cairo_scaled_font_get_user_data, 
//GO(cairo_scaled_font_glyph_extents, 
//GO(cairo_scaled_font_reference, 
//GO(cairo_scaled_font_set_user_data, 
//GO(cairo_scaled_font_status, 
//GO(cairo_scaled_font_text_extents, 
//GO(cairo_scaled_font_text_to_glyphs, 
//GO(cairo_script_create, 
//GO(cairo_script_create_for_stream, 
//GO(cairo_script_from_recording_surface, 
//GO(cairo_script_get_mode, 
//GO(cairo_script_set_mode, 
//GO(cairo_script_surface_create, 
//GO(cairo_script_surface_create_for_target, 
//GO(cairo_script_write_comment, 
GO(cairo_select_font_face, vFppii)
GO(cairo_set_antialias, vFpi)
GO(cairo_set_dash, vFppid)
GO(cairo_set_fill_rule, vFpi)
GO(cairo_set_font_face, vFpp)
GO(cairo_set_font_matrix, vFpp)
GO(cairo_set_font_options, vFpp)
GO(cairo_set_font_size, vFpd)
GO(cairo_set_line_cap, vFpi)
GO(cairo_set_line_join, vFpi)
GO(cairo_set_line_width, vFpd)
GO(cairo_set_matrix, vFpp)
GO(cairo_set_miter_limit, vFpd)
GO(cairo_set_operator, vFpi)
GO(cairo_set_scaled_font, vFpp)
GO(cairo_set_source, vFpp)
GO(cairo_set_source_rgb, vFpddd)
GO(cairo_set_source_rgba, vFpdddd)
GO(cairo_set_source_surface, vFppdd)
GO(cairo_set_tolerance, vFpd)
//GOM(cairo_set_user_data, iFEpppp)
GO(cairo_show_glyphs, vFppi)
GO(cairo_show_page, vFp)
GO(cairo_show_text, vFpp)
GO(cairo_show_text_glyphs, vFppipipii)
GO(cairo_status, iFp)
//GO(cairo_status_to_string, 
GO(cairo_stroke, vFp)
GO(cairo_stroke_extents, vFppppp)
GO(cairo_stroke_preserve, vFp)
GO(cairo_surface_copy_page, vFp)
GO(cairo_surface_create_for_rectangle, pFpdddd)
//GO(cairo_surface_create_observer, 
GO(cairo_surface_create_similar, pFppii)
GO(cairo_surface_create_similar_image, pFppii)
GO(cairo_surface_destroy, vFp)
GO(cairo_surface_finish, vFp)
GO(cairo_surface_flush, vFp)
GO(cairo_surface_get_content, pFp)
GO(cairo_surface_get_device, pFp)
GO(cairo_surface_get_device_offset, vFppp)
GO(cairo_surface_get_device_scale, vFpp)
GO(cairo_surface_get_fallback_resolution, vFppp)
GO(cairo_surface_get_font_options, vFpp)
GO(cairo_surface_get_mime_data, vFpppp)
GO(cairo_surface_get_reference_count, uFp)
GO(cairo_surface_get_type, iFp)
GO(cairo_surface_get_user_data, pFpp)
GO(cairo_surface_has_show_text_glyphs, iFp)
GO(cairo_surface_map_to_image, pFpp)
GO(cairo_surface_mark_dirty, vFp)
GO(cairo_surface_mark_dirty_rectangle, vFpiiii)
//GO(cairo_surface_observer_add_fill_callback, 
//GO(cairo_surface_observer_add_finish_callback, 
//GO(cairo_surface_observer_add_flush_callback, 
//GO(cairo_surface_observer_add_glyphs_callback, 
//GO(cairo_surface_observer_add_mask_callback, 
//GO(cairo_surface_observer_add_paint_callback, 
//GO(cairo_surface_observer_add_stroke_callback, 
//GO(cairo_surface_observer_elapsed, 
//GO(cairo_surface_observer_print, 
GO(cairo_surface_reference, pFp)
GO(cairo_surface_set_device_offset, vFpdd)
GO(cairo_surface_set_device_scale, vFpdd)
GO(cairo_surface_set_fallback_resolution, vFpdd)
//GOM(cairo_surface_set_mime_data, iFEpppLpp)
//GOM(cairo_surface_set_user_data, iFEpppp)
GO(cairo_surface_show_page, vFp)
GO(cairo_surface_status, iFp)
GO(cairo_surface_supports_mime_type, iFpp)
GO(cairo_surface_unmap_image, vFpp)
//GO(cairo_surface_write_to_png, 
//GO(cairo_surface_write_to_png_stream, 
//GO(cairo_svg_get_versions, 
//GO(cairo_svg_surface_create, 
//GO(cairo_svg_surface_create_for_stream, 
//GO(cairo_svg_surface_restrict_to_version, 
//GO(cairo_svg_version_to_string, 
GO(cairo_text_cluster_allocate, pFi)
GO(cairo_text_cluster_free, vFp)
GO(cairo_text_extents, vFppp)
GO(cairo_text_path, vFpp)
GO(cairo_toy_font_face_create, pFpii)
GO(cairo_toy_font_face_get_family, pFp)
GO(cairo_toy_font_face_get_slant, iFp)
GO(cairo_toy_font_face_get_weight, iFp)
GO(cairo_transform, vFpp)
GO(cairo_translate, vFpdd)
//GO(cairo_user_font_face_create, 
//GO(cairo_user_font_face_get_init_func, 
//GO(cairo_user_font_face_get_render_glyph_func, 
//GO(cairo_user_font_face_get_text_to_glyphs_func, 
//GO(cairo_user_font_face_get_unicode_to_glyph_func, 
//GO(cairo_user_font_face_set_init_func, 
//GO(cairo_user_font_face_set_render_glyph_func, 
//GO(cairo_user_font_face_set_text_to_glyphs_func, 
//GO(cairo_user_font_face_set_unicode_to_glyph_func, 
GO(cairo_user_to_device, vFppp)
GO(cairo_user_to_device_distance, vFppp)
//GO(cairo_version, 
//GO(cairo_version_string, 
//GO(cairo_xcb_device_debug_cap_xrender_version, 
//GO(cairo_xcb_device_debug_cap_xshm_version, 
//GO(cairo_xcb_device_debug_get_precision, 
//GO(cairo_xcb_device_debug_set_precision, 
//GO(cairo_xcb_device_get_connection, 
//GO(cairo_xcb_surface_create, 
//GO(cairo_xcb_surface_create_for_bitmap, 
//GO(cairo_xcb_surface_create_with_xrender_format, 
//GO(cairo_xcb_surface_set_drawable, 
//GO(cairo_xcb_surface_set_size, 
//GO(cairo_xlib_device_debug_cap_xrender_version, 
//GO(cairo_xlib_device_debug_get_precision, 
//GO(cairo_xlib_device_debug_set_precision, 
//GO(cairo_xlib_surface_create, 
//GO(cairo_xlib_surface_create_for_bitmap, 
//GO(cairo_xlib_surface_create_with_xrender_format, 
//GO(cairo_xlib_surface_get_depth, 
//GO(cairo_xlib_surface_get_display, 
//GO(cairo_xlib_surface_get_drawable, 
//GO(cairo_xlib_surface_get_height, 
//GO(cairo_xlib_surface_get_screen, 
//GO(cairo_xlib_surface_get_visual, 
//GO(cairo_xlib_surface_get_width, 
//GO(cairo_xlib_surface_get_xrender_format, 
//GO(cairo_xlib_surface_set_drawable, 
//GO(cairo_xlib_surface_set_size, 
//GO(cairo_xml_create, 
//GO(cairo_xml_create_for_stream, 
//GO(cairo_xml_for_recording_surface, 
//GO(cairo_xml_surface_create, 
