#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//GO(gst_audio_aggregator_convert_pad_get_type, 
//GO(gst_audio_aggregator_get_type, 
//GO(gst_audio_aggregator_pad_get_type, 
//GO(gst_audio_aggregator_set_sink_caps, 
//GO(gst_audio_base_sink_create_ringbuffer, 
//GO(gst_audio_base_sink_discont_reason_get_type, 
//GO(gst_audio_base_sink_get_alignment_threshold, 
//GO(gst_audio_base_sink_get_discont_wait, 
//GO(gst_audio_base_sink_get_drift_tolerance, 
//GO(gst_audio_base_sink_get_provide_clock, 
//GO(gst_audio_base_sink_get_slave_method, 
//GO(gst_audio_base_sink_get_type, 
//GO(gst_audio_base_sink_report_device_failure, 
//GO(gst_audio_base_sink_set_alignment_threshold, 
//GO(gst_audio_base_sink_set_custom_slaving_callback, 
//GO(gst_audio_base_sink_set_discont_wait, 
//GO(gst_audio_base_sink_set_drift_tolerance, 
//GO(gst_audio_base_sink_set_provide_clock, 
//GO(gst_audio_base_sink_set_slave_method, 
//GO(gst_audio_base_sink_slave_method_get_type, 
//GO(gst_audio_base_src_create_ringbuffer, 
//GO(gst_audio_base_src_get_provide_clock, 
//GO(gst_audio_base_src_get_slave_method, 
//GO(gst_audio_base_src_get_type, 
//GO(gst_audio_base_src_set_provide_clock, 
//GO(gst_audio_base_src_set_slave_method, 
//GO(gst_audio_base_src_slave_method_get_type, 
GO(gst_audio_buffer_clip, pFppii)
//GO(gst_audio_buffer_map, 
GO(gst_audio_buffer_reorder_channels, iFpuipp)
GO(gst_audio_buffer_truncate, pFpiLL)
//GO(gst_audio_buffer_unmap, 
//GO(gst_audio_cd_src_add_track, 
//GO(gst_audio_cd_src_get_type, 
//GO(gst_audio_cd_src_mode_get_type, 
GO(gst_audio_channel_get_fallback_mask, LFi)
//GO(gst_audio_channel_mixer_flags_get_type, 
GO(gst_audio_channel_mixer_free, vFp)
GO(gst_audio_channel_mixer_is_passthrough, iFp)
GO(gst_audio_channel_mixer_new, pFuuipip)
GO(gst_audio_channel_mixer_new_with_matrix, pFuuiip)
GO(gst_audio_channel_mixer_samples, vFpppi)
//GO(gst_audio_channel_position_get_type, 
GO(gst_audio_channel_positions_from_mask, iFiLp)
GO(gst_audio_channel_positions_to_mask, iFpiip)
GO(gst_audio_channel_positions_to_string, pFpi)
GO(gst_audio_channel_positions_to_valid_order, iFpi)
GO(gst_audio_check_valid_channel_positions, iFpii)
GO(gst_audio_clipping_meta_api_get_type, LFv)
//GO(gst_audio_clipping_meta_get_info, 
//GO(gst_audio_clock_adjust, 
//GO(gst_audio_clock_get_time, 
//GO(gst_audio_clock_get_type, 
//GO(gst_audio_clock_invalidate, 
//GO(gst_audio_clock_new, 
//GO(gst_audio_clock_reset, 
GO(gst_audio_converter_convert, iFpupLpp)
//GO(gst_audio_converter_flags_get_type, 
//GO(gst_audio_converter_free, 
GO(gst_audio_converter_get_config, pFppp)
GO(gst_audio_converter_get_in_frames, LFpL)
GO(gst_audio_converter_get_max_latency, LFp)
//GO(gst_audio_converter_get_out_frames, 
//GO(gst_audio_converter_get_type, 
GO(gst_audio_converter_is_passthrough, iFp)
//GO(gst_audio_converter_new, 
GO(gst_audio_converter_reset, vFp)
GO(gst_audio_converter_samples, iFpupLpL)
//GO(gst_audio_converter_supports_inplace, 
GO(gst_audio_converter_update_config, iFpiip)
GO(gst_audio_decoder_allocate_output_buffer, pFpL)
GO(_gst_audio_decoder_error, iFpipippppi)
GO(gst_audio_decoder_finish_frame, iFppi)
GO(gst_audio_decoder_finish_subframe, iFpp)
//GO(gst_audio_decoder_get_allocator, 
//GO(gst_audio_decoder_get_audio_info, 
//GO(gst_audio_decoder_get_delay, 
//GO(gst_audio_decoder_get_drainable, 
//GO(gst_audio_decoder_get_estimate_rate, 
//GO(gst_audio_decoder_get_latency, 
//GO(gst_audio_decoder_get_max_errors, 
//GO(gst_audio_decoder_get_min_latency, 
//GO(gst_audio_decoder_get_needs_format, 
//GO(gst_audio_decoder_get_parse_state, 
//GO(gst_audio_decoder_get_plc, 
//GO(gst_audio_decoder_get_plc_aware, 
//GO(gst_audio_decoder_get_tolerance, 
GO(gst_audio_decoder_get_type, LFv)
//GO(gst_audio_decoder_merge_tags, 
//GO(gst_audio_decoder_negotiate, 
//GO(gst_audio_decoder_proxy_getcaps, 
//GO(gst_audio_decoder_set_allocation_caps, 
GO(gst_audio_decoder_set_drainable, vFpi)
//GO(gst_audio_decoder_set_estimate_rate, 
//GO(gst_audio_decoder_set_latency, 
//GO(gst_audio_decoder_set_max_errors, 
//GO(gst_audio_decoder_set_min_latency, 
GO(gst_audio_decoder_set_needs_format, vFpi)
//GO(gst_audio_decoder_set_output_caps, 
GO(gst_audio_decoder_set_output_format, iFpp)
//GO(gst_audio_decoder_set_plc, 
//GO(gst_audio_decoder_set_plc_aware, 
//GO(gst_audio_decoder_set_tolerance, 
GO(gst_audio_decoder_set_use_default_pad_acceptcaps, vFpi)
//GO(gst_audio_dither_method_get_type, 
//GO(gst_audio_downmix_meta_api_get_type, 
//GO(gst_audio_downmix_meta_get_info, 
//GO(gst_audio_encoder_allocate_output_buffer, 
GO(gst_audio_encoder_finish_frame, iFppi)
//GO(gst_audio_encoder_get_allocator, 
GO(gst_audio_encoder_get_audio_info, pFp)
//GO(gst_audio_encoder_get_drainable, 
//GO(gst_audio_encoder_get_frame_max, 
//GO(gst_audio_encoder_get_frame_samples_max, 
//GO(gst_audio_encoder_get_frame_samples_min, 
//GO(gst_audio_encoder_get_hard_min, 
//GO(gst_audio_encoder_get_hard_resync, 
//GO(gst_audio_encoder_get_latency, 
//GO(gst_audio_encoder_get_lookahead, 
//GO(gst_audio_encoder_get_mark_granule, 
//GO(gst_audio_encoder_get_perfect_timestamp, 
//GO(gst_audio_encoder_get_tolerance, 
GO(gst_audio_encoder_get_type, LFv)
GO(gst_audio_encoder_merge_tags, vFppi)
//GO(gst_audio_encoder_negotiate, 
//GO(gst_audio_encoder_proxy_getcaps, 
//GO(gst_audio_encoder_set_allocation_caps, 
GO(gst_audio_encoder_set_drainable, vFpi)
GO(gst_audio_encoder_set_frame_max, vFpi)
GO(gst_audio_encoder_set_frame_samples_max, vFpi)
GO(gst_audio_encoder_set_frame_samples_min, vFpi)
//GO(gst_audio_encoder_set_hard_min, 
//GO(gst_audio_encoder_set_hard_resync, 
//GO(gst_audio_encoder_set_headers, 
//GO(gst_audio_encoder_set_latency, 
//GO(gst_audio_encoder_set_lookahead, 
//GO(gst_audio_encoder_set_mark_granule, 
GO(gst_audio_encoder_set_output_format, iFpp)
//GO(gst_audio_encoder_set_perfect_timestamp, 
//GO(gst_audio_encoder_set_tolerance, 
//GO(gst_audio_filter_class_add_pad_templates, 
GO(gst_audio_filter_get_type, LFv)
GO(gst_audio_flags_get_type, LFv)
GO(gst_audio_format_build_integer, uFiiii)
GO(gst_audio_format_fill_silence, vFppL)
GO(gst_audio_format_flags_get_type, LFv)
GO(gst_audio_format_from_string, uFp)
//GO(gst_audio_format_get_info, 
GO(gst_audio_format_get_type, LFv)
GO(gst_audio_format_info_fill_silence, vFppL)
GO(gst_audio_format_info_get_type, LFv)
GO(gst_audio_formats_raw, pFp)
GO(gst_audio_format_to_string, pFu)
GO(gst_audio_get_channel_reorder_map, iFippp)
//GO(gst_audio_iec61937_frame_size, 
//GO(gst_audio_iec61937_payload, 
GO(gst_audio_info_convert, iFpiIip)
GO(gst_audio_info_copy, pFp)
GO(gst_audio_info_free, vFp)
GO(gst_audio_info_from_caps, iFpp)
GO(gst_audio_info_get_type, pFv)
GO(gst_audio_info_init, vFp)
GO(gst_audio_info_is_equal, iFpp)
GO(gst_audio_info_new, pFv)
GO(gst_audio_info_set_format, vFpiiip)
GO(gst_audio_info_to_caps, pFp)
GO(gst_audio_layout_get_type, pFv)
//GO(gst_audio_make_raw_caps, 
//GO(gst_audio_meta_api_get_type, 
//GO(gst_audio_meta_get_info, 
//GO(gst_audio_noise_shaping_method_get_type, 
//GO(gst_audio_pack_flags_get_type, 
//GO(gst_audio_quantize_flags_get_type, 
//GO(gst_audio_quantize_free, 
GO(gst_audio_quantize_new, pFuuuuuu)
GO(gst_audio_quantize_reset, vFp)
GO(gst_audio_quantize_samples, vFpppu)
GO(gst_audio_reorder_channels, iFpLuipp)
//GO(gst_audio_resampler_filter_interpolation_get_type, 
//GO(gst_audio_resampler_filter_mode_get_type, 
//GO(gst_audio_resampler_flags_get_type, 
//GO(gst_audio_resampler_free, 
GO(gst_audio_resampler_get_in_frames, LFpL)
GO(gst_audio_resampler_get_max_latency, LFp)
//GO(gst_audio_resampler_get_out_frames, 
//GO(gst_audio_resampler_method_get_type, 
GO(gst_audio_resampler_new, pFuuuiiip)
GO(gst_audio_resampler_options_set_quality, vFuuiip)
GO(gst_audio_resampler_resample, vFppLpL)
GO(gst_audio_resampler_reset, vFp)
GO(gst_audio_resampler_update, iFpiip)
//GO(gst_audio_ring_buffer_acquire, 
//GO(gst_audio_ring_buffer_activate, 
//GO(gst_audio_ring_buffer_advance, 
//GO(gst_audio_ring_buffer_clear, 
//GO(gst_audio_ring_buffer_clear_all, 
//GO(gst_audio_ring_buffer_close_device, 
//GO(gst_audio_ring_buffer_commit, 
//GO(gst_audio_ring_buffer_convert, 
//GO(gst_audio_ring_buffer_debug_spec_buff, 
//GO(gst_audio_ring_buffer_debug_spec_caps, 
//GO(gst_audio_ring_buffer_delay, 
//GO(gst_audio_ring_buffer_device_is_open, 
//GO(gst_audio_ring_buffer_format_type_get_type, 
//GO(gst_audio_ring_buffer_get_type, 
//GO(gst_audio_ring_buffer_is_acquired, 
//GO(gst_audio_ring_buffer_is_active, 
//GO(gst_audio_ring_buffer_is_flushing, 
//GO(gst_audio_ring_buffer_may_start, 
//GO(gst_audio_ring_buffer_open_device, 
//GO(gst_audio_ring_buffer_parse_caps, 
//GO(gst_audio_ring_buffer_pause, 
//GO(gst_audio_ring_buffer_prepare_read, 
//GO(gst_audio_ring_buffer_read, 
//GO(gst_audio_ring_buffer_release, 
//GO(gst_audio_ring_buffer_samples_done, 
//GO(gst_audio_ring_buffer_set_callback, 
//GO(gst_audio_ring_buffer_set_callback_full, 
//GO(gst_audio_ring_buffer_set_channel_positions, 
//GO(gst_audio_ring_buffer_set_flushing, 
//GO(gst_audio_ring_buffer_set_sample, 
//GO(gst_audio_ring_buffer_set_timestamp, 
//GO(gst_audio_ring_buffer_start, 
//GO(gst_audio_ring_buffer_state_get_type, 
//GO(gst_audio_ring_buffer_stop, 
//GO(gst_audio_sink_get_type, 
//GO(gst_audio_src_get_type, 
GO(gst_audio_stream_align_copy, pFp)
//GO(gst_audio_stream_align_free, 
//GO(gst_audio_stream_align_get_alignment_threshold, 
//GO(gst_audio_stream_align_get_discont_wait, 
GO(gst_audio_stream_align_get_rate, iFp)
GO(gst_audio_stream_align_get_samples_since_discont, LFp)
GO(gst_audio_stream_align_get_timestamp_at_discont, LFp)
//GO(gst_audio_stream_align_get_type, 
GO(gst_audio_stream_align_mark_discont, vFp)
GO(gst_audio_stream_align_new, pFiLL)
GO(gst_audio_stream_align_process, iFpiLuppp)
//GO(gst_audio_stream_align_set_alignment_threshold, 
GO(gst_audio_stream_align_set_discont_wait, vFpL)
GO(gst_audio_stream_align_set_rate, vFpi)
GO(gst_buffer_add_audio_clipping_meta, pFpuLL)
GO(gst_buffer_add_audio_downmix_meta, pFppipip)
GO(gst_buffer_add_audio_meta, pFppLp)
GO(gst_buffer_get_audio_downmix_meta_for_channels, pFppi)
GO(gst_stream_volume_convert_volume, dFuud)
GO(gst_stream_volume_get_mute, iFp)
GO(gst_stream_volume_get_type, LFv)
GO(gst_stream_volume_get_volume, dFpu)
GO(gst_stream_volume_set_mute, vFpi)
GO(gst_stream_volume_set_volume, vFpud)
