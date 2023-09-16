#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(gst_adapter_available, LFp)
GO(gst_adapter_available_fast, LFp)
GO(gst_adapter_clear, vFp)
GO(gst_adapter_copy, vFppLL)
GO(gst_adapter_copy_bytes, pFpLL)
GO(gst_adapter_distance_from_discont, LFp)
GO(gst_adapter_dts_at_discont, LFp)
GO(gst_adapter_flush, vFpL)
//GO(gst_adapter_get_buffer, 
GO(gst_adapter_get_buffer_fast, pFpL)
GO(gst_adapter_get_buffer_list, pFpL)
GO(gst_adapter_get_list, pFpL)
GO(gst_adapter_get_type, LFv)
GO(gst_adapter_map, pFpL)
GO(gst_adapter_masked_scan_uint32, lFpuuLL)
GO(gst_adapter_masked_scan_uint32_peek, lFpuuLLp)
GO(gst_adapter_new, pFv)
//GO(gst_adapter_offset_at_discont, 
GO(gst_adapter_prev_dts, LFpp)
GO(gst_adapter_prev_dts_at_offset, LFpLp)
GO(gst_adapter_prev_offset, LFpp)
//GO(gst_adapter_prev_pts, 
//GO(gst_adapter_prev_pts_at_offset, 
//GO(gst_adapter_pts_at_discont, 
GO(gst_adapter_push, vFpp)
GO(gst_adapter_take, pFpL)
GO(gst_adapter_take_buffer, pFpL)
//GO(gst_adapter_take_buffer_fast, 
//GO(gst_adapter_take_buffer_list, 
//GO(gst_adapter_take_list, 
GO(gst_adapter_unmap, vFp)
GO(gst_aggregator_finish_buffer, iFpp)
//GO(gst_aggregator_finish_buffer_list, 
//GO(gst_aggregator_get_allocator, 
//GO(gst_aggregator_get_buffer_pool, 
//GO(gst_aggregator_get_latency, 
GO(gst_aggregator_get_type, LFv)
//GO(gst_aggregator_negotiate, 
//GO(gst_aggregator_pad_drop_buffer, 
GO(gst_aggregator_pad_get_type, LFv)
//GO(gst_aggregator_pad_has_buffer, 
//GO(gst_aggregator_pad_is_eos, 
//GO(gst_aggregator_pad_peek_buffer, 
//GO(gst_aggregator_pad_pop_buffer, 
//GO(gst_aggregator_peek_next_sample, 
//GO(gst_aggregator_selected_samples, 
//GO(gst_aggregator_set_latency, 
//GO(gst_aggregator_set_src_caps, 
//GO(gst_aggregator_simple_get_next_time, 
//GO(gst_aggregator_start_time_selection_get_type, 
//GO(gst_aggregator_update_segment, 
//GO(gst_base_parse_add_index_entry, 
//GO(gst_base_parse_convert_default, 
//GO(gst_base_parse_drain, 
//GO(gst_base_parse_finish_frame, 
//GO(gst_base_parse_frame_copy, 
//GO(gst_base_parse_frame_free, 
//GO(gst_base_parse_frame_get_type, 
//GO(gst_base_parse_frame_init, 
//GO(gst_base_parse_frame_new, 
//GO(gst_base_parse_get_type, 
//GO(gst_base_parse_merge_tags, 
//GO(gst_base_parse_push_frame, 
//GO(gst_base_parse_set_average_bitrate, 
//GO(gst_base_parse_set_duration, 
//GO(gst_base_parse_set_frame_rate, 
//GO(gst_base_parse_set_has_timing_info, 
//GO(gst_base_parse_set_infer_ts, 
//GO(gst_base_parse_set_latency, 
//GO(gst_base_parse_set_min_frame_size, 
//GO(gst_base_parse_set_passthrough, 
//GO(gst_base_parse_set_pts_interpolation, 
//GO(gst_base_parse_set_syncable, 
//GO(gst_base_parse_set_ts_at_offset, 
//GO(gst_base_sink_do_preroll, 
//GO(gst_base_sink_get_blocksize, 
//GO(gst_base_sink_get_drop_out_of_segment, 
//GO(gst_base_sink_get_last_sample, 
//GO(gst_base_sink_get_latency, 
//GO(gst_base_sink_get_max_bitrate, 
//GO(gst_base_sink_get_max_lateness, 
//GO(gst_base_sink_get_processing_deadline, 
//GO(gst_base_sink_get_render_delay, 
//GO(gst_base_sink_get_stats, 
//GO(gst_base_sink_get_sync, 
//GO(gst_base_sink_get_throttle_time, 
//GO(gst_base_sink_get_ts_offset, 
GO(gst_base_sink_get_type, LFv)
//GO(gst_base_sink_is_async_enabled, 
//GO(gst_base_sink_is_last_sample_enabled, 
//GO(gst_base_sink_is_qos_enabled, 
//GO(gst_base_sink_query_latency, 
//GO(gst_base_sink_set_async_enabled, 
//GO(gst_base_sink_set_blocksize, 
//GO(gst_base_sink_set_drop_out_of_segment, 
//GO(gst_base_sink_set_last_sample_enabled, 
//GO(gst_base_sink_set_max_bitrate, 
//GO(gst_base_sink_set_max_lateness, 
//GO(gst_base_sink_set_processing_deadline, 
//GO(gst_base_sink_set_qos_enabled, 
//GO(gst_base_sink_set_render_delay, 
//GO(gst_base_sink_set_sync, 
//GO(gst_base_sink_set_throttle_time, 
//GO(gst_base_sink_set_ts_offset, 
//GO(gst_base_sink_wait, 
//GO(gst_base_sink_wait_clock, 
//GO(gst_base_sink_wait_preroll, 
//GO(gst_base_src_get_allocator, 
GO(gst_base_src_get_blocksize, uFp)
//GO(gst_base_src_get_buffer_pool, 
//GO(gst_base_src_get_do_timestamp, 
GO(gst_base_src_get_type, LFv)
//GO(gst_base_src_is_async, 
GO(gst_base_src_is_live, iFp)
//GO(gst_base_src_negotiate, 
//GO(gst_base_src_new_seamless_segment, 
//GO(gst_base_src_new_segment, 
//GO(gst_base_src_query_latency, 
//GO(gst_base_src_set_async, 
GO(gst_base_src_set_automatic_eos, vFpi)
GO(gst_base_src_set_blocksize, vFpu)
GO(gst_base_src_set_caps, iFpp)
//GO(gst_base_src_set_do_timestamp, 
//GO(gst_base_src_set_dynamic_size, 
//GO(gst_base_src_set_format, 
GO(gst_base_src_set_live, vFpi)
//GO(gst_base_src_start_complete, 
//GO(gst_base_src_start_wait, 
//GO(gst_base_src_submit_buffer_list, 
//GO(gst_base_src_wait_playing, 
//GO(gst_base_transform_get_allocator, 
//GO(gst_base_transform_get_buffer_pool, 
GO(gst_base_transform_get_type, LFv)
//GO(gst_base_transform_is_in_place, 
GO(gst_base_transform_is_passthrough, iFp)
//GO(gst_base_transform_is_qos_enabled, 
//GO(gst_base_transform_reconfigure, 
//GO(gst_base_transform_reconfigure_sink, 
GO(gst_base_transform_reconfigure_src, vFp)
GO(gst_base_transform_set_gap_aware, vFpi)
GO(gst_base_transform_set_in_place, vFpi)
GO(gst_base_transform_set_passthrough, vFpi)
GO(gst_base_transform_set_prefer_passthrough, vFpi)
//GO(gst_base_transform_set_qos_enabled, 
//GO(gst_base_transform_update_qos, 
GO(gst_base_transform_update_src_caps, iFpp)
//GO(gst_bit_reader_free, 
//GO(gst_bit_reader_get_bits_uint16, 
//GO(gst_bit_reader_get_bits_uint32, 
//GO(gst_bit_reader_get_bits_uint64, 
//GO(gst_bit_reader_get_bits_uint8, 
//GO(gst_bit_reader_get_pos, 
//GO(gst_bit_reader_get_remaining, 
//GO(gst_bit_reader_get_size, 
GO(gst_bit_reader_init, vFppu)
GO(gst_bit_reader_new, pFpu)
//GO(gst_bit_reader_peek_bits_uint16, 
//GO(gst_bit_reader_peek_bits_uint32, 
//GO(gst_bit_reader_peek_bits_uint64, 
//GO(gst_bit_reader_peek_bits_uint8, 
//GO(gst_bit_reader_set_pos, 
//GO(gst_bit_reader_skip, 
//GO(gst_bit_reader_skip_to_byte, 
//GO(gst_bit_writer_align_bytes, 
//GO(gst_bit_writer_free, 
//GO(gst_bit_writer_free_and_get_buffer, 
//GO(gst_bit_writer_free_and_get_data, 
//GO(gst_bit_writer_get_data, 
//GO(gst_bit_writer_get_size, 
//GO(gst_bit_writer_init, 
GO(gst_bit_writer_init_with_data, vFppui)
GO(gst_bit_writer_init_with_size, vFpui)
GO(gst_bit_writer_new, pFv)
GO(gst_bit_writer_new_with_data, pFpui)
GO(gst_bit_writer_new_with_size, pFui)
//GO(gst_bit_writer_put_bits_uint16, 
//GO(gst_bit_writer_put_bits_uint32, 
//GO(gst_bit_writer_put_bits_uint64, 
//GO(gst_bit_writer_put_bits_uint8, 
//GO(gst_bit_writer_put_bytes, 
GO(gst_bit_writer_reset, vFp)
GO(gst_bit_writer_reset_and_get_buffer, pFp)
GO(gst_bit_writer_reset_and_get_data, pFp)
//GO(gst_bit_writer_set_pos, 
//GO(gst_byte_reader_dup_data, 
GO(gst_byte_reader_dup_string_utf16, iFpp)
GO(gst_byte_reader_dup_string_utf32, iFpp)
GO(gst_byte_reader_dup_string_utf8, iFpp)
GO(gst_byte_reader_free, vFp)
//GO(gst_byte_reader_get_data, 
//GO(gst_byte_reader_get_float32_be, 
//GO(gst_byte_reader_get_float32_le, 
//GO(gst_byte_reader_get_float64_be, 
//GO(gst_byte_reader_get_float64_le, 
//GO(gst_byte_reader_get_int16_be, 
//GO(gst_byte_reader_get_int16_le, 
//GO(gst_byte_reader_get_int24_be, 
//GO(gst_byte_reader_get_int24_le, 
//GO(gst_byte_reader_get_int32_be, 
//GO(gst_byte_reader_get_int32_le, 
//GO(gst_byte_reader_get_int64_be, 
//GO(gst_byte_reader_get_int64_le, 
//GO(gst_byte_reader_get_int8, 
//GO(gst_byte_reader_get_pos, 
//GO(gst_byte_reader_get_remaining, 
//GO(gst_byte_reader_get_size, 
GO(gst_byte_reader_get_string_utf8, iFpp)
//GO(gst_byte_reader_get_sub_reader, 
//GO(gst_byte_reader_get_uint16_be, 
//GO(gst_byte_reader_get_uint16_le, 
//GO(gst_byte_reader_get_uint24_be, 
//GO(gst_byte_reader_get_uint24_le, 
//GO(gst_byte_reader_get_uint32_be, 
//GO(gst_byte_reader_get_uint32_le, 
//GO(gst_byte_reader_get_uint64_be, 
//GO(gst_byte_reader_get_uint64_le, 
//GO(gst_byte_reader_get_uint8, 
//GO(gst_byte_reader_init, 
GO(gst_byte_reader_masked_scan_uint32, uFpuuuu)
GO(gst_byte_reader_masked_scan_uint32_peek, uFpuuuup)
GO(gst_byte_reader_new, pFpu)
//GO(gst_byte_reader_peek_data, 
//GO(gst_byte_reader_peek_float32_be, 
//GO(gst_byte_reader_peek_float32_le, 
//GO(gst_byte_reader_peek_float64_be, 
//GO(gst_byte_reader_peek_float64_le, 
//GO(gst_byte_reader_peek_int16_be, 
//GO(gst_byte_reader_peek_int16_le, 
//GO(gst_byte_reader_peek_int24_be, 
//GO(gst_byte_reader_peek_int24_le, 
//GO(gst_byte_reader_peek_int32_be, 
//GO(gst_byte_reader_peek_int32_le, 
//GO(gst_byte_reader_peek_int64_be, 
//GO(gst_byte_reader_peek_int64_le, 
//GO(gst_byte_reader_peek_int8, 
GO(gst_byte_reader_peek_string_utf8, iFpp)
//GO(gst_byte_reader_peek_sub_reader, 
//GO(gst_byte_reader_peek_uint16_be, 
//GO(gst_byte_reader_peek_uint16_le, 
//GO(gst_byte_reader_peek_uint24_be, 
//GO(gst_byte_reader_peek_uint24_le, 
//GO(gst_byte_reader_peek_uint32_be, 
//GO(gst_byte_reader_peek_uint32_le, 
//GO(gst_byte_reader_peek_uint64_be, 
//GO(gst_byte_reader_peek_uint64_le, 
//GO(gst_byte_reader_peek_uint8, 
GO(gst_byte_reader_set_pos, iFpu)
//GO(gst_byte_reader_skip, 
//GO(gst_byte_reader_skip_string_utf16, 
GO(gst_byte_reader_skip_string_utf32, iFp)
//GO(gst_byte_reader_skip_string_utf8, 
//GO(gst_byte_writer_ensure_free_space, 
//GO(gst_byte_writer_fill, 
GO(gst_byte_writer_free, vFp)
GO(gst_byte_writer_free_and_get_buffer, pFp)
//GO(gst_byte_writer_free_and_get_data, 
GO(gst_byte_writer_get_remaining, uFp)
GO(gst_byte_writer_init, vFp)
GO(gst_byte_writer_init_with_data, vFppui)
GO(gst_byte_writer_init_with_size, vFpui)
GO(gst_byte_writer_new, pFv)
GO(gst_byte_writer_new_with_data, pFpui)
GO(gst_byte_writer_new_with_size, pFui)
//GO(gst_byte_writer_put_data, 
//GO(gst_byte_writer_put_float32_be, 
//GO(gst_byte_writer_put_float32_le, 
//GO(gst_byte_writer_put_float64_be, 
//GO(gst_byte_writer_put_float64_le, 
//GO(gst_byte_writer_put_int16_be, 
//GO(gst_byte_writer_put_int16_le, 
//GO(gst_byte_writer_put_int24_be, 
//GO(gst_byte_writer_put_int24_le, 
//GO(gst_byte_writer_put_int32_be, 
//GO(gst_byte_writer_put_int32_le, 
//GO(gst_byte_writer_put_int64_be, 
//GO(gst_byte_writer_put_int64_le, 
//GO(gst_byte_writer_put_int8, 
GO(gst_byte_writer_put_string_utf16, iFpp)
GO(gst_byte_writer_put_string_utf32, iFpp)
GO(gst_byte_writer_put_string_utf8, iFpp)
//GO(gst_byte_writer_put_uint16_be, 
//GO(gst_byte_writer_put_uint16_le, 
//GO(gst_byte_writer_put_uint24_be, 
//GO(gst_byte_writer_put_uint24_le, 
//GO(gst_byte_writer_put_uint32_be, 
//GO(gst_byte_writer_put_uint32_le, 
//GO(gst_byte_writer_put_uint64_be, 
//GO(gst_byte_writer_put_uint64_le, 
//GO(gst_byte_writer_put_uint8, 
GO(gst_byte_writer_reset, vFp)
GO(gst_byte_writer_reset_and_get_buffer, pFp)
GO(gst_byte_writer_reset_and_get_data, pFp)
GOM(gst_collect_pads_add_pad, pFEppupi)
GO(gst_collect_pads_available, uFp)
GO(gst_collect_pads_clip_running_time, iFppppp)
GO(gst_collect_pads_event_default, iFpppi)
//GO(gst_collect_pads_flush, 
//GO(gst_collect_pads_get_type, 
GO(gst_collect_pads_new, pFv)
GO(gst_collect_pads_peek, pFpp)
GO(gst_collect_pads_pop, pFpp)
GO(gst_collect_pads_query_default, iFpppi)
//GO(gst_collect_pads_read_buffer, 
GO(gst_collect_pads_remove_pad, iFpp)
GOM(gst_collect_pads_set_buffer_function, vFEppp)
GOM(gst_collect_pads_set_clip_function, vFEppp)
//GO(gst_collect_pads_set_compare_function, 
GOM(gst_collect_pads_set_event_function, vFEppp)
//GO(gst_collect_pads_set_flush_function, 
GO(gst_collect_pads_set_flushing, vFpi)
GOM(gst_collect_pads_set_function, vFEppp)
GOM(gst_collect_pads_set_query_function, vFEppp)
GO(gst_collect_pads_set_waiting, vFppi)
//GO(gst_collect_pads_src_event_default, 
GO(gst_collect_pads_start, vFp)
GO(gst_collect_pads_stop, vFp)
GO(gst_collect_pads_take_buffer, pFppu)
GO(gst_data_queue_drop_head, iFpL)
//GO(gst_data_queue_flush, 
GO(gst_data_queue_get_level, vFpp)
//GO(gst_data_queue_get_type, 
GO(gst_data_queue_is_empty, iFp)
//GO(gst_data_queue_is_full, 
//GO(gst_data_queue_limits_changed, 
//GO(gst_data_queue_new, 
GO(gst_data_queue_peek, iFpp)
//GO(gst_data_queue_pop, 
//GO(gst_data_queue_push, 
//GO(gst_data_queue_push_force, 
GO(gst_data_queue_set_flushing, vFpi)
GO(gst_flow_combiner_add_pad, vFpp)
GO(gst_flow_combiner_clear, vFp)
GO(gst_flow_combiner_free, vFp)
GO(gst_flow_combiner_get_type, LFv)
GO(gst_flow_combiner_new, pFv)
GO(gst_flow_combiner_ref, pFp)
GO(gst_flow_combiner_remove_pad, vFpp)
GO(gst_flow_combiner_reset, vFp)
//GO(gst_flow_combiner_unref, 
GO(gst_flow_combiner_update_flow, iFpi)
GO(gst_flow_combiner_update_pad_flow, iFppi)
GO(gst_push_src_get_type, LFv)
GO(gst_queue_array_clear, vFp)
//GO(gst_queue_array_drop_element, 
GO(gst_queue_array_drop_struct, iFpup)
//GO(gst_queue_array_find, 
//GO(gst_queue_array_free, 
GO(gst_queue_array_get_length, uFp)
GO(gst_queue_array_is_empty, iFp)
GO(gst_queue_array_new, pFu)
GO(gst_queue_array_new_for_struct, pFLu)
//GO(gst_queue_array_peek_head, 
//GO(gst_queue_array_peek_head_struct, 
//GO(gst_queue_array_peek_nth, 
GO(gst_queue_array_peek_nth_struct, pFpu)
//GO(gst_queue_array_peek_tail, 
GO(gst_queue_array_peek_tail_struct, pFp)
//GO(gst_queue_array_pop_head, 
//GO(gst_queue_array_pop_head_struct, 
//GO(gst_queue_array_pop_tail, 
//GO(gst_queue_array_pop_tail_struct, 
//GO(gst_queue_array_push_tail, 
GO(gst_queue_array_push_tail_struct, vFpp)
//GO(gst_queue_array_set_clear_func, 
//GO(gst_type_find_helper, 
GO(gst_type_find_helper_for_buffer, pFppp)
//GO(gst_type_find_helper_for_buffer_with_extension, 
//GO(gst_type_find_helper_for_data, 
GO(gst_type_find_helper_for_data_with_extension, pFppLpp)
//GO(gst_type_find_helper_for_extension, 
//GO(gst_type_find_helper_get_range, 
//GO(gst_type_find_helper_get_range_full, 
