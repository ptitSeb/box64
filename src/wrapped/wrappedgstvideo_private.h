#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//GO(gst_buffer_add_video_afd_meta, 
GO(gst_buffer_add_video_affine_transformation_meta, pFp)
//GO(gst_buffer_add_video_bar_meta, 
GO(gst_buffer_add_video_caption_meta, pFpipL)
GO(gst_buffer_add_video_codec_alpha_meta, pFpp)
//GO(gst_buffer_add_video_gl_texture_upload_meta, 
GO(gst_buffer_add_video_meta, pFpiiuu)
GO(gst_buffer_add_video_meta_full, pFpiiuuupp)
//GO(gst_buffer_add_video_overlay_composition_meta, 
//GO(gst_buffer_add_video_region_of_interest_meta, 
//GO(gst_buffer_add_video_region_of_interest_meta_id, 
GO(gst_buffer_add_video_time_code_meta, pFpp)
//GO(gst_buffer_add_video_time_code_meta_full, 
GO(gst_buffer_get_video_meta, pFp)
//GO(gst_buffer_get_video_meta_id, 
//GO(gst_buffer_get_video_region_of_interest_meta_id, 
GO(gst_buffer_pool_config_get_video_alignment, iFpp)
GO(gst_buffer_pool_config_set_video_alignment, vFpp)
GO(gst_color_balance_channel_get_type, LFv)
GO(gst_color_balance_get_balance_type, uFp)
GO(gst_color_balance_get_type, LFv)
GO(gst_color_balance_get_value, iFpp)
GO(gst_color_balance_list_channels, pFp)
GO(gst_color_balance_set_value, vFppi)
//GO(gst_color_balance_type_get_type, 
GO(gst_color_balance_value_changed, vFppi)
//GO(gst_is_video_overlay_prepare_window_handle_message, 
//GO(gst_navigation_command_get_type, 
GO(gst_navigation_event_get_coordinates, iFppp)
GO(gst_navigation_event_get_type, LFv)
GO(gst_navigation_event_new_key_press, pFpi)
GO(gst_navigation_event_new_key_release, pFpi)
GO(gst_navigation_event_new_mouse_button_press, pFpddi)
GO(gst_navigation_event_new_mouse_button_release, pFpddi)
GO(gst_navigation_event_new_mouse_move, pFddi)
GO(gst_navigation_event_new_mouse_scroll, pFddddi)
//GO(gst_navigation_event_parse_command, 
GO(gst_navigation_event_parse_key_event, iFpp)
//GO(gst_navigation_event_parse_mouse_button_event, 
//GO(gst_navigation_event_parse_mouse_move_event, 
GO(gst_navigation_event_parse_mouse_scroll_event, iFppppp)
GO(gst_navigation_event_set_coordinates, iFpdd)
//GO(gst_navigation_event_type_get_type, 
GO(gst_navigation_get_type, LFv)
//GO(gst_navigation_message_get_type, 
//GO(gst_navigation_message_new_angles_changed, 
//GO(gst_navigation_message_new_commands_changed, 
GO(gst_navigation_message_new_event, pFpp)
//GO(gst_navigation_message_new_mouse_over, 
//GO(gst_navigation_message_parse_angles_changed, 
//GO(gst_navigation_message_parse_event, 
//GO(gst_navigation_message_parse_mouse_over, 
//GO(gst_navigation_message_type_get_type, 
//GO(gst_navigation_query_get_type, 
//GO(gst_navigation_query_new_angles, 
//GO(gst_navigation_query_new_commands, 
//GO(gst_navigation_query_parse_angles, 
//GO(gst_navigation_query_parse_commands_length, 
//GO(gst_navigation_query_parse_commands_nth, 
//GO(gst_navigation_query_set_angles, 
//GO(gst_navigation_query_set_commands, 
//GO(gst_navigation_query_set_commandsv, 
GO(gst_navigation_query_type_get_type, LFv)
GO(gst_navigation_send_command, vFpu)
GO(gst_navigation_send_event, vFpp)
GO(gst_navigation_send_event_simple, vFpp)
GO(gst_navigation_send_key_event, vFppp)
GO(gst_navigation_send_mouse_event, vFppidd)
GO(gst_navigation_send_mouse_scroll_event, vFpdddd)
GO(gst_video_afd_meta_api_get_type, LFv)
//GO(gst_video_afd_meta_get_info, 
GO(gst_video_afd_spec_get_type, LFv)
GO(gst_video_afd_value_get_type, LFv)
GO(gst_video_affine_transformation_meta_api_get_type, LFv)
//GO(gst_video_affine_transformation_meta_apply_matrix, 
//GO(gst_video_affine_transformation_meta_get_info, 
GO(gst_video_aggregator_convert_pad_get_type, LFv)
//GO(gst_video_aggregator_convert_pad_update_conversion_info, 
GO(gst_video_aggregator_get_type, LFv)
GO(gst_video_aggregator_pad_get_current_buffer, pFp)
//GO(gst_video_aggregator_pad_get_prepared_frame, 
GO(gst_video_aggregator_pad_get_type, LFv)
//GO(gst_video_aggregator_pad_has_current_buffer, 
//GO(gst_video_aggregator_pad_set_needs_alpha, 
GO(gst_video_alignment_reset, vFp)
GO(gst_video_alpha_mode_get_type, LFv)
GO(gst_video_ancillary_di_d16_get_type, LFv)
GO(gst_video_ancillary_did_get_type, LFv)
GO(gst_video_bar_meta_api_get_type, LFv)
//GO(gst_video_bar_meta_get_info, 
//GO(gst_video_blend, 
//GO(gst_video_blend_scale_linear_RGBA, 
GO(gst_video_buffer_flags_get_type, LFv)
GO(gst_video_buffer_pool_get_type, LFv)
GO(gst_video_buffer_pool_new, pFv)
GO(gst_video_calculate_display_ratio, iFppuuuuuu)
GO(gst_video_caption_meta_api_get_type, LFv)
//GO(gst_video_caption_meta_get_info, 
//GO(gst_video_caption_type_from_caps, 
GO(gst_video_caption_type_get_type, LFv)
//GO(gst_video_caption_type_to_caps, 
GO(gst_video_chroma_flags_get_type, LFv)
//GO(gst_video_chroma_from_string, 
GO(gst_video_chroma_method_get_type, LFv)
GO(gst_video_chroma_mode_get_type, LFv)
GO(gst_video_chroma_resample, vFppi)
GO(gst_video_chroma_resample_free, vFp)
GO(gst_video_chroma_resample_get_info, vFppp)
GO(gst_video_chroma_resample_new, pFuuuuii)
GO(gst_video_chroma_site_get_type, LFv)
GO(gst_video_chroma_to_string, pFu)
GO(gst_video_codec_frame_get_type, LFv)
//GO(gst_video_codec_frame_get_user_data, 
GO(gst_video_codec_frame_ref, pFp)
//GO(gst_video_codec_frame_set_user_data, 
GO(gst_video_codec_frame_unref, vFp)
GO(gst_video_codec_state_get_type, LFv)
GO(gst_video_codec_state_ref, pFp)
GO(gst_video_codec_state_unref, vFp)
GO(gst_video_colorimetry_from_string, iFpp)
GO(gst_video_colorimetry_is_equal, iFpp)
GO(gst_video_colorimetry_matches, iFpp)
GO(gst_video_colorimetry_to_string, pFp)
GO(gst_video_color_matrix_from_iso, uFu)
GO(gst_video_color_matrix_get_Kr_Kb, iFupp)
GO(gst_video_color_matrix_get_type, LFv)
GO(gst_video_color_matrix_to_iso, uFu)
GO(gst_video_color_primaries_from_iso, uFu)
GO(gst_video_color_primaries_get_info, pFu)
GO(gst_video_color_primaries_get_type, LFv)
GO(gst_video_color_primaries_to_iso, uFu)
GO(gst_video_color_range_get_type, LFv)
//GO(gst_video_color_range_offsets, 
//GO(gst_video_color_transfer_decode, 
//GO(gst_video_color_transfer_encode, 
GO(gst_video_content_light_level_add_to_caps, iFpp)
//GO(gst_video_content_light_level_from_caps, 
GO(gst_video_content_light_level_from_string, iFpp)
GO(gst_video_content_light_level_init, vFp)
GO(gst_video_content_light_level_to_string, pFp)
GO(gst_video_converter_frame, vFppp)
GO(gst_video_converter_free, vFp)
GO(gst_video_converter_get_config, pFp)
GO(gst_video_converter_new, pFppp)
GO(gst_video_converter_set_config, iFpp)
//GO(gst_video_convert_sample, 
//GO(gst_video_convert_sample_async, 
GO(gst_video_crop_meta_api_get_type, LFv)
GO(gst_video_crop_meta_get_info, pFv)
GO(gst_video_decoder_add_to_frame, vFpi)
//GO(gst_video_decoder_allocate_output_buffer, 
GO(gst_video_decoder_allocate_output_frame, iFpp)
//GO(gst_video_decoder_allocate_output_frame_with_params, 
GO(gst_video_decoder_drop_frame, iFpp)
GO(_gst_video_decoder_error, iFpipippppi)
GO(gst_video_decoder_finish_frame, iFpp)
//GO(gst_video_decoder_get_allocator, 
GO(gst_video_decoder_get_buffer_pool, pFp)
//GO(gst_video_decoder_get_estimate_rate, 
GO(gst_video_decoder_get_frame, pFpi)
GO(gst_video_decoder_get_frames, pFp)
//GO(gst_video_decoder_get_latency, 
GO(gst_video_decoder_get_max_decode_time, IFpp)
//GO(gst_video_decoder_get_max_errors, 
//GO(gst_video_decoder_get_needs_format, 
//GO(gst_video_decoder_get_oldest_frame, 
GO(gst_video_decoder_get_output_state, pFp)
//GO(gst_video_decoder_get_packetized, 
//GO(gst_video_decoder_get_pending_frame_size, 
//GO(gst_video_decoder_get_qos_proportion, 
GO(gst_video_decoder_get_subframe_mode, iFp)
GO(gst_video_decoder_get_type, LFv)
GO(gst_video_decoder_have_frame, iFp)
GO(gst_video_decoder_have_last_subframe, iFpp)
GO(gst_video_decoder_merge_tags, vFppi)
GO(gst_video_decoder_negotiate, iFp)
GO(gst_video_decoder_proxy_getcaps, pFppp)
GO(gst_video_decoder_release_frame, vFpp)
//GO(gst_video_decoder_set_estimate_rate, 
GO(gst_video_decoder_set_interlaced_output_state, pFpiiuup)
GO(gst_video_decoder_set_latency, vFpUU)
//GO(gst_video_decoder_set_max_errors, 
GO(gst_video_decoder_set_needs_format, vFpi)
GO(gst_video_decoder_set_output_state, pFpiuup)
GO(gst_video_decoder_set_packetized, vFpi)
GO(gst_video_decoder_set_subframe_mode, vFpi)
GO(gst_video_decoder_set_use_default_pad_acceptcaps, vFpi)
GO(gst_video_direction_get_type, LFv)
//GO(gst_video_dither_flags_get_type, 
GO(gst_video_dither_free, vFp)
GO(gst_video_dither_line, vFppuuu)
GO(gst_video_dither_method_get_type, LFv)
GO(gst_video_dither_new, pFuuupu)
GO(gst_video_encoder_allocate_output_buffer, pFpL)
GO(gst_video_encoder_allocate_output_frame, iFppL)
GO(gst_video_encoder_finish_frame, iFpp)
//GO(gst_video_encoder_finish_subframe, 
//GO(gst_video_encoder_get_allocator, 
//GO(gst_video_encoder_get_frame, 
//GO(gst_video_encoder_get_frames, 
//GO(gst_video_encoder_get_latency, 
//GO(gst_video_encoder_get_max_encode_time, 
//GO(gst_video_encoder_get_min_force_key_unit_interval, 
GO(gst_video_encoder_get_oldest_frame, pFp)
//GO(gst_video_encoder_get_output_state, 
GO(gst_video_encoder_get_type, LFv)
//GO(gst_video_encoder_is_qos_enabled, 
GO(gst_video_encoder_merge_tags, vFppi)
GO(gst_video_encoder_negotiate, iFp)
GO(gst_video_encoder_proxy_getcaps, pFppp)
GO(gst_video_encoder_set_headers, vFpp)
//GO(gst_video_encoder_set_latency, 
//GO(gst_video_encoder_set_min_force_key_unit_interval, 
GO(gst_video_encoder_set_min_pts, vFpU)
GO(gst_video_encoder_set_output_state, pFppp)
//GO(gst_video_encoder_set_qos_enabled, 
//GO(gst_video_event_is_force_key_unit, 
//GO(gst_video_event_new_downstream_force_key_unit, 
//GO(gst_video_event_new_still_frame, 
//GO(gst_video_event_new_upstream_force_key_unit, 
//GO(gst_video_event_parse_downstream_force_key_unit, 
GO(gst_video_event_parse_still_frame, iFpp)
//GO(gst_video_event_parse_upstream_force_key_unit, 
GO(gst_video_field_order_from_string, uFp)
GO(gst_video_field_order_get_type, LFv)
GO(gst_video_field_order_to_string, pFu)
GO(gst_video_filter_get_type, LFv)
GO(gst_video_flags_get_type, LFv)
GO(gst_video_format_flags_get_type, LFv)
GO(gst_video_format_from_fourcc, uFu)
GO(gst_video_format_from_masks, uFiiiuuuu)
GO(gst_video_format_from_string, uFp)
GO(gst_video_format_get_info, pFi)
GO(gst_video_format_get_palette, pFup)
//GO(gst_video_format_get_type, 
//GO(gst_video_format_info_component, 
GO(gst_video_formats_raw, pFp)
GO(gst_video_format_to_fourcc, uFu)
GO(gst_video_format_to_string, pFu)
GO(gst_video_frame_copy, iFpp)
GO(gst_video_frame_copy_plane, iFppu)
//GO(gst_video_frame_flags_get_type, 
GO(gst_video_frame_map, iFpppu)
//GO(gst_video_frame_map_flags_get_type, 
//GO(gst_video_frame_map_id, 
GO(gst_video_frame_unmap, vFp)
GO(gst_video_gamma_mode_get_type, LFv)
GO(gst_video_gl_texture_upload_meta_api_get_type, LFv)
//GO(gst_video_gl_texture_upload_meta_get_info, 
//GO(gst_video_gl_texture_upload_meta_upload, 
GO(gst_video_guess_framerate, iFLpp)
GO(gst_video_info_align, iFpp)
GO(gst_video_info_align_full, iFppp)
GO(gst_video_info_convert, iFpiIip)
GO(gst_video_info_copy, pFp)
GO(gst_video_info_free, vFp)
GO(gst_video_info_from_caps, iFpp)
GO(gst_video_info_get_type, LFv)
GO(gst_video_info_init, vFp)
GO(gst_video_info_is_equal, iFpp)
GO(gst_video_info_new, pFv)
GO(gst_video_info_set_format, iFpiuu)
GO(gst_video_info_set_interlaced_format, iFpiiuu)
GO(gst_video_info_to_caps, pFp)
GO(gst_video_interlace_mode_from_string, uFp)
GO(gst_video_interlace_mode_get_type, LFv)
GO(gst_video_interlace_mode_to_string, pFu)
GO(gst_video_is_common_aspect_ratio, iFiiii)
//GO(gst_video_make_raw_caps, 
//GO(gst_video_make_raw_caps_with_features, 
GO(gst_video_mastering_display_info_add_to_caps, iFpp)
//GO(gst_video_mastering_display_info_from_caps, 
GO(gst_video_mastering_display_info_from_string, iFpp)
GO(gst_video_mastering_display_info_init, vFp)
GO(gst_video_mastering_display_info_is_equal, iFpp)
GO(gst_video_mastering_display_info_to_string, pFp)
GO(gst_video_matrix_mode_get_type, LFv)
GO(gst_video_meta_api_get_type, LFv)
//GO(gst_video_meta_get_info, 
//GO(gst_video_meta_get_plane_height, 
//GO(gst_video_meta_get_plane_size, 
//GO(gst_video_meta_map, 
GO(gst_video_meta_set_alignment, iFpi)
GO(gst_video_meta_transform_scale_get_quark, pFv)
//GO(gst_video_meta_unmap, 
GO(gst_video_multiview_flagset_get_type, LFv)
GO(gst_video_multiview_flags_get_type, LFv)
GO(gst_video_multiview_frame_packing_get_type, LFv)
//GO(gst_video_multiview_get_doubled_height_modes, 
GO(gst_video_multiview_get_doubled_size_modes, pFv)
//GO(gst_video_multiview_get_doubled_width_modes, 
//GO(gst_video_multiview_get_mono_modes, 
//GO(gst_video_multiview_get_unpacked_modes, 
GO(gst_video_multiview_guess_half_aspect, iFiuuuu)
GO(gst_video_multiview_mode_from_caps_string, iFp)
GO(gst_video_multiview_mode_get_type, LFv)
GO(gst_video_multiview_mode_to_caps_string, pFi)
GO(gst_video_multiview_video_info_change_mode, vFpii)
GO(gst_video_orientation_from_tag, iFpp)
//GO(gst_video_orientation_get_hcenter, 
//GO(gst_video_orientation_get_hflip, 
GO(gst_video_orientation_get_type, LFv)
GO(gst_video_orientation_get_vcenter, iFpp)
GO(gst_video_orientation_get_vflip, iFpp)
GO(gst_video_orientation_method_get_type, LFv)
//GO(gst_video_orientation_set_hcenter, 
//GO(gst_video_orientation_set_hflip, 
GO(gst_video_orientation_set_vcenter, iFpi)
GO(gst_video_orientation_set_vflip, iFpi)
GO(gst_video_overlay_composition_add_rectangle, vFpp)
//GO(gst_video_overlay_composition_blend, 
//GO(gst_video_overlay_composition_copy, 
GO(gst_video_overlay_composition_get_rectangle, pFpu)
GO(gst_video_overlay_composition_get_seqnum, uFp)
GO(gst_video_overlay_composition_get_type, LFv)
GO(gst_video_overlay_composition_make_writable, pFp)
GO(gst_video_overlay_composition_meta_api_get_type, LFv)
//GO(gst_video_overlay_composition_meta_get_info, 
GO(gst_video_overlay_composition_new, pFp)
GO(gst_video_overlay_composition_n_rectangles, uFp)
GO(gst_video_overlay_expose, vFp)
GO(gst_video_overlay_format_flags_get_type, LFv)
GO(gst_video_overlay_get_type, LFv)
GO(gst_video_overlay_got_window_handle, vFpL)
GO(gst_video_overlay_handle_events, vFpi)
GO(gst_video_overlay_install_properties, vFpi)
GO(gst_video_overlay_prepare_window_handle, vFp)
GO(gst_video_overlay_rectangle_copy, pFp)
GO(gst_video_overlay_rectangle_get_flags, uFp)
GO(gst_video_overlay_rectangle_get_global_alpha, fFp)
//GO(gst_video_overlay_rectangle_get_pixels_argb, 
//GO(gst_video_overlay_rectangle_get_pixels_ayuv, 
//GO(gst_video_overlay_rectangle_get_pixels_raw, 
//GO(gst_video_overlay_rectangle_get_pixels_unscaled_argb, 
//GO(gst_video_overlay_rectangle_get_pixels_unscaled_ayuv, 
//GO(gst_video_overlay_rectangle_get_pixels_unscaled_raw, 
GO(gst_video_overlay_rectangle_get_render_rectangle, iFppppp)
GO(gst_video_overlay_rectangle_get_seqnum, uFp)
GO(gst_video_overlay_rectangle_get_type, LFv)
//GO(gst_video_overlay_rectangle_new_raw, 
GO(gst_video_overlay_rectangle_set_global_alpha, vFpf)
GO(gst_video_overlay_rectangle_set_render_rectangle, vFpiiuu)
GO(gst_video_overlay_set_property, iFpiup)
GO(gst_video_overlay_set_render_rectangle, iFpiiii)
GO(gst_video_overlay_set_window_handle, vFpL)
GO(gst_video_pack_flags_get_type, LFv)
GO(gst_video_primaries_mode_get_type, LFv)
//GO(gst_video_region_of_interest_meta_add_param, 
GO(gst_video_region_of_interest_meta_api_get_type, LFv)
//GO(gst_video_region_of_interest_meta_get_info, 
//GO(gst_video_region_of_interest_meta_get_param, 
GO(gst_video_resampler_clear, vFp)
GO(gst_video_resampler_flags_get_type, LFv)
GO(gst_video_resampler_init, iFpuuuuduup)
GO(gst_video_resampler_method_get_type, LFv)
GO(gst_video_scaler_2d, vFppupipiuuuu)
GO(gst_video_scaler_combine_packed_YUV, pFppuu)
GO(gst_video_scaler_flags_get_type, LFv)
GO(gst_video_scaler_free, vFp)
GO(gst_video_scaler_get_coeff, pFpupp)
GO(gst_video_scaler_get_max_taps, uFp)
GO(gst_video_scaler_horizontal, vFpuppuu)
GO(gst_video_scaler_new, pFuuuuup)
GO(gst_video_scaler_vertical, vFpuppuu)
GO(gst_video_sink_center_rect, vFpppppi)    // this is vFHHpi, with 2 GstVideoRectangle struct that are 4 ints
GO(gst_video_sink_get_type, LFv)
GO(gst_video_tile_get_index, uFuiiii)
GO(gst_video_tile_mode_get_type, LFv)
GO(gst_video_tile_type_get_type, LFv)
GO(gst_video_time_code_add_frames, vFpl)
GO(gst_video_time_code_add_interval, pFpp)
GO(gst_video_time_code_clear, vFp)
GO(gst_video_time_code_compare, iFpp)
GO(gst_video_time_code_copy, pFp)
GO(gst_video_time_code_flags_get_type, LFv)
GO(gst_video_time_code_frames_since_daily_jam, LFp)
GO(gst_video_time_code_free, vFp)
GO(gst_video_time_code_get_type, LFv)
GO(gst_video_time_code_increment_frame, vFp)
GO(gst_video_time_code_init, vFpuupuuuuuu)
GO(gst_video_time_code_init_from_date_time, vFpuupuu)
GO(gst_video_time_code_init_from_date_time_full, iFpuupuu)
GO(gst_video_time_code_interval_clear, vFp)
GO(gst_video_time_code_interval_copy, pFp)
//GO(gst_video_time_code_interval_free, 
GO(gst_video_time_code_interval_get_type, LFv)
GO(gst_video_time_code_interval_init, vFpuuuu)
GO(gst_video_time_code_interval_new, pFuuuu)
GO(gst_video_time_code_interval_new_from_string, pFp)
GO(gst_video_time_code_is_valid, iFp)
GO(gst_video_time_code_meta_api_get_type, LFv)
//GO(gst_video_time_code_meta_get_info, 
GO(gst_video_time_code_new, pFuupuuuuuu)
GO(gst_video_time_code_new_empty, pFv)
//GO(gst_video_time_code_new_from_date_time, 
GO(gst_video_time_code_new_from_date_time_full, pFuupuu)
GO(gst_video_time_code_new_from_string, pFp)
GO(gst_video_time_code_nsec_since_daily_jam, UFp)
GO(gst_video_time_code_to_date_time, pFp)
GO(gst_video_time_code_to_string, pFp)
GO(gst_video_transfer_function_from_iso, uFu)
//GO(gst_video_transfer_function_get_type, 
GO(gst_video_transfer_function_is_equivalent, iFuuuu)
GO(gst_video_transfer_function_to_iso, uFu)
GO(gst_video_vbi_encoder_add_ancillary, iFpiCCpu)
GO(gst_video_vbi_encoder_copy, pFp)
GO(gst_video_vbi_encoder_free, vFp)
//GO(gst_video_vbi_encoder_get_type, 
GO(gst_video_vbi_encoder_new, pFuu)
GO(gst_video_vbi_encoder_write_line, vFpp)
GO(gst_video_vbi_parser_add_line, vFpp)
GO(gst_video_vbi_parser_copy, pFp)
GO(gst_video_vbi_parser_free, vFp)
GO(gst_video_vbi_parser_get_ancillary, uFpp)
//GO(gst_video_vbi_parser_get_type, 
GO(gst_video_vbi_parser_new, pFuu)
//GO(gst_video_vbi_parser_result_get_type, 
