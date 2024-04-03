#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(gst_allocation_params_copy, pFp)
//GO(gst_allocation_params_free, 
//GO(gst_allocation_params_get_type, 
GO(gst_allocation_params_init, vFp)
GO(gst_allocator_alloc, pFpLp)
GO(gst_allocator_find, pFp)
//GO(gst_allocator_flags_get_type, 
//GO(gst_allocator_free, 
GO(gst_allocator_get_type, LFv)
//GO(gst_allocator_register, 
//GO(gst_allocator_set_default, 
//GO(gst_atomic_queue_get_type, 
GO(gst_atomic_queue_length, uFp)
GO(gst_atomic_queue_new, pFu)
GO(gst_atomic_queue_peek, pFp)
GO(gst_atomic_queue_pop, pFp)
GO(gst_atomic_queue_push, vFpp)
GO(gst_atomic_queue_ref, vFp)
GO(gst_atomic_queue_unref, vFp)
GO(gst_bin_add, iFpp)
GOM(gst_bin_add_many, vFEppV)
GO(gst_bin_find_unlinked_pad, pFpi)
GO(gst_bin_flags_get_type, LFv)
GO(gst_bin_get_by_interface, pFpL)
GO(gst_bin_get_by_name, pFpp)
GO(gst_bin_get_by_name_recurse_up, pFpp)
GO(gst_bin_get_suppressed_flags, uFp)
GO(gst_bin_get_type, LFv)
GO(gst_bin_iterate_all_by_element_factory_name, pFpp)
GO(gst_bin_iterate_all_by_interface, pFpp)
GO(gst_bin_iterate_elements, pFp)
GO(gst_bin_iterate_recurse, pFp)
GO(gst_bin_iterate_sinks, pFp)
GO(gst_bin_iterate_sorted, pFp)
GO(gst_bin_iterate_sources, pFp)
GO(gst_bin_new, pFp)
GO(gst_bin_recalculate_latency, iFp)
GO(gst_bin_remove, iFpp)
//GOM(gst_bin_remove_many, iFEppV)
GO(gst_bin_set_suppressed_flags, vFpu)
GO(gst_bin_sync_children_states, iFp)
GO(gst_bitmask_get_type, LFv)
DATAB(_gst_bitmask_type, sizeof(void*))
GO(gst_buffer_add_meta, pFppp)
GO(gst_buffer_add_parent_buffer_meta, pFpp)
GO(gst_buffer_add_protection_meta, pFpp)
GO(gst_buffer_add_reference_timestamp_meta, pFppUU)
GO(gst_buffer_append, pFpp)
GO(gst_buffer_append_memory, vFpp)
GO(gst_buffer_append_region, pFppll)
GO(gst_buffer_copy, pFp)
GO(gst_buffer_copy_deep, pFp)
GO(gst_buffer_copy_flags_get_type, LFv)
GO(gst_buffer_copy_into, iFppiLL)
GO(gst_buffer_copy_region, pFpiLL)
GO(gst_buffer_extract, LFpLpL)
GO(gst_buffer_extract_dup, vFpLLpp)
GO(gst_buffer_fill, LFpLpL)
GO(gst_buffer_find_memory, iFpLLppp)
GO(gst_buffer_flags_get_type, LFv)
GOM(gst_buffer_foreach_meta, iFEppp)
GO(gst_buffer_get_all_memory, pFp)
GO(gst_buffer_get_flags, iFp)
GO(gst_buffer_get_max_memory, uFv)
GO(gst_buffer_get_memory, pFpu)
GO(gst_buffer_get_memory_range, pFpui)
GO(gst_buffer_get_meta, pFpp)
GO(gst_buffer_get_n_meta, uFpp)
GO(gst_buffer_get_reference_timestamp_meta, pFpp)
GO(gst_buffer_get_size, LFp)
GO(gst_buffer_get_sizes, LFppp)
GO(gst_buffer_get_sizes_range, LFpuipp)
GO(gst_buffer_get_type, LFv)
GO(gst_buffer_has_flags, iFpi)
GO(gst_buffering_mode_get_type, LFv)
GO(gst_buffer_insert_memory, vFpip)
GO(gst_buffer_is_all_memory_writable, iFp)
GO(gst_buffer_is_memory_range_writable, iFpui)
GO(gst_buffer_iterate_meta, pFpp)
GO(gst_buffer_iterate_meta_filtered, pFppp)
GO(gst_buffer_list_calculate_size, LFp)
//GO(gst_buffer_list_copy, 
GO(gst_buffer_list_copy_deep, pFp)
//GO(gst_buffer_list_foreach, 
GO(gst_buffer_list_get, pFpu)
//GO(gst_buffer_list_get_type, 
//GO(gst_buffer_list_get_writable, 
GO(gst_buffer_list_insert, vFpip)
GO(gst_buffer_list_length, uFp)
GO(gst_buffer_list_new, pFv)
GO(gst_buffer_list_new_sized, pFu)
GO(gst_buffer_list_ref, pFp)
GO(gst_buffer_list_remove, vFpuu)
//GO(gst_buffer_list_replace, 
GO(gst_buffer_list_take, iFpp)
//DATAB(_gst_buffer_list_type, 
//GO(gst_buffer_list_unref, 
GO(gst_buffer_map, iFppi)
GO(gst_buffer_map_range, iFpuipi)
GO(gst_buffer_memcmp, iFpLpL)
GO(gst_buffer_memset, LFpLCL)
GO(gst_buffer_new, pFv)
GO(gst_buffer_new_allocate, pFplp)
GO(gst_buffer_new_memdup, pFpL)
GO(gst_buffer_new_wrapped, pFpl)
GO(gst_buffer_new_wrapped_bytes, pFp)
GOM(gst_buffer_new_wrapped_full, pFEiplllpp)
GO(gst_buffer_n_memory, uFp)
GO(gst_buffer_peek_memory, pFpu)
GO(gst_buffer_pool_acquire_buffer, iFppp)
//GO(gst_buffer_pool_acquire_flags_get_type, 
GO(gst_buffer_pool_config_add_option, vFpp)
GO(gst_buffer_pool_config_get_allocator, iFppp)
GO(gst_buffer_pool_config_get_option, pFpu)
GO(gst_buffer_pool_config_get_params, iFppppp)
GO(gst_buffer_pool_config_has_option, iFpp)
GO(gst_buffer_pool_config_n_options, uFp)
GO(gst_buffer_pool_config_set_allocator, vFppp)
GO(gst_buffer_pool_config_set_params, vFppuuu)
GO(gst_buffer_pool_config_validate_params, iFppuuu)
GO(gst_buffer_pool_get_config, pFp)
GO(gst_buffer_pool_get_options, pFp)
GO(gst_buffer_pool_get_type, LFv)
GO(gst_buffer_pool_has_option, iFpp)
GO(gst_buffer_pool_is_active, iFp)
GO(gst_buffer_pool_new, pFv)
//GO(gst_buffer_pool_release_buffer, 
GO(gst_buffer_pool_set_active, iFpi)
GO(gst_buffer_pool_set_config, iFpp)
GO(gst_buffer_pool_set_flushing, vFpi)
GO(gst_buffer_prepend_memory, vFpp)
GO(gst_buffer_ref, pFp)
GO(gst_buffer_remove_all_memory, vFp)
GO(gst_buffer_remove_memory, vFpu)
GO(gst_buffer_remove_memory_range, vFpui)
GO(gst_buffer_remove_meta, iFpp)
GO(gst_buffer_replace, iFpp)
GO(gst_buffer_replace_all_memory, vFpp)
GO(gst_buffer_replace_memory, vFpup)
GO(gst_buffer_replace_memory_range, vFpuip)
GO(gst_buffer_resize, vFpll)
GO(gst_buffer_resize_range, iFpuill)
GO(gst_buffer_set_flags, iFpi)
GO(gst_buffer_set_size, vFpl)
DATAB(_gst_buffer_type, sizeof(void*))
GO(gst_buffer_unmap, vFpp)
GO(gst_buffer_unref, vFp)
GO(gst_buffer_unset_flags, iFpi)
GO(gst_bus_add_signal_watch, vFp)
GO(gst_bus_add_signal_watch_full, vFpi)
GOM(gst_bus_add_watch, uFEppp)
GOM(gst_bus_add_watch_full, uFEpippp)
GO(gst_bus_async_signal_func, iFppp)
GO(gst_bus_create_watch, pFp)
GO(gst_bus_disable_sync_message_emission, vFp)
GO(gst_bus_enable_sync_message_emission, vFp)
GO(gst_bus_flags_get_type, LFv)
GO(gst_bus_get_pollfd, vFpp)
GO(gst_bus_get_type, LFv)
GO(gst_bus_have_pending, iFp)
GO(gst_bus_new, pFv)
GO(gst_bus_peek, pFp)
GO(gst_bus_poll, pFppU)
GO(gst_bus_pop, pFp)
GO(gst_bus_pop_filtered, pFpi)
GO(gst_bus_post, iFpp)
GO(gst_bus_remove_signal_watch, vFp)
GO(gst_bus_remove_watch, iFp)
GO(gst_bus_set_flushing, vFpi)
GOM(gst_bus_set_sync_handler, vFEpppp)
GO(gst_bus_sync_reply_get_type, LFv)
GO(gst_bus_sync_signal_handler, iFppp)
GO(gst_bus_timed_pop, pFpU)
GO(gst_bus_timed_pop_filtered, pFpUi)
GO(gst_calculate_linear_regression, iFppuppppp)
DATAB(_gst_caps_any, sizeof(void*))
GO(gst_caps_append, vFpp)
GO(gst_caps_append_structure, vFpp)
GO(gst_caps_append_structure_full, vFppp)
GO(gst_caps_can_intersect, iFpp)
GO(gst_caps_copy, pFp)
GO(gst_caps_copy_nth, pFpu)
GO(gst_caps_features_add, vFpp)
//GO(gst_caps_features_add_id, 
//DATAB(_gst_caps_features_any, 
GO(gst_caps_features_contains, iFpp)
GO(gst_caps_features_contains_id, iFpu)
GO(gst_caps_features_copy, pFp)
GO(gst_caps_features_free, vFp)
GO(gst_caps_features_from_string, pFp)
GO(gst_caps_features_get_nth, pFpu)
GO(gst_caps_features_get_nth_id, uFpu)
GO(gst_caps_features_get_size, uFp)
//GO(gst_caps_features_get_type, 
GO(gst_caps_features_is_any, iFp)
GO(gst_caps_features_is_equal, iFpp)
DATAB(_gst_caps_features_memory_system_memory, sizeof(void*))
GOM(gst_caps_features_new, pFEpV)
GO(gst_caps_features_new_any, pFv)
//GO(gst_caps_features_new_empty, 
//GO(gst_caps_features_new_id, 
GO(gst_caps_features_new_id_valist, pFup)
GOM(gst_caps_features_new_valist, pFEpA)
GO(gst_caps_features_remove, vFpp)
GO(gst_caps_features_remove_id, vFpu)
GO(gst_caps_features_set_parent_refcount, iFpp)
GO(gst_caps_features_to_string, pFp)
//DATAB(_gst_caps_features_type, 
//GO(gst_caps_filter_and_map_in_place, 
GO(gst_caps_fixate, pFp)
GO(gst_caps_flags_get_type, LFv)
GOM(gst_caps_foreach, iFEppp)
GO(gst_caps_from_string, pFp)
GO(gst_caps_get_features, pFpu)
GO(gst_caps_get_size, uFp)
GO(gst_caps_get_structure, pFpu)
GO(gst_caps_get_type, LFv)
GO(gst_caps_intersect, pFpp)
GO(gst_caps_intersect_full, pFppi)
GO(gst_caps_intersect_mode_get_type, LFv)
GO(gst_caps_is_always_compatible, iFpp)
GO(gst_caps_is_any, iFp)
GO(gst_caps_is_empty, iFp)
GO(gst_caps_is_equal, iFpp)
GO(gst_caps_is_equal_fixed, iFpp)
GO(gst_caps_is_fixed, iFp)
GO(gst_caps_is_strictly_equal, iFpp)
GO(gst_caps_is_subset, iFpp)
GO(gst_caps_is_subset_structure, iFpp)
GO(gst_caps_is_subset_structure_full, iFppp)
//GOM(gst_caps_map_in_place, iFEpBp)
GO(gst_caps_merge, pFpp)
GO(gst_caps_merge_structure, pFpp)
GO(gst_caps_merge_structure_full, pFppp)
GO(gst_caps_new_any, pFv)
GO(gst_caps_new_empty, pFv)
GO(gst_caps_new_empty_simple, pFp)
//GOM(gst_caps_new_full, pFEpV)
//GOM(gst_caps_new_full_valist, pFEpA)
GOM(gst_caps_new_simple, pFEppV)
DATAB(_gst_caps_none, sizeof(void*))
GO(gst_caps_normalize, pFp)
GO(gst_caps_ref, pFp)
GO(gst_caps_remove_structure, vFpp)
GO(gst_caps_replace, iFpp)
GO(gst_caps_set_features, vFpup)
GO(gst_caps_set_features_simple, vFpp)
GOM(gst_caps_set_simple, vFEppV)
GOM(gst_caps_set_simple_valist, vFEppA)
GO(gst_caps_set_value, vFppp)
GO(gst_caps_simplify, pFp)
GO(gst_caps_steal_structure, pFpu)
GO(gst_caps_subtract, pFpp)
GO(gst_caps_take, iFpp)
GO(gst_caps_to_string, pFp)
GO(gst_caps_truncate, pFp)
DATAB(_gst_caps_type, sizeof(void*))
GO(gst_caps_unref, vFp)
//DATAB(GST_CAT_BUFFER, 
//DATAB(GST_CAT_BUFFER_LIST, 
//DATAB(GST_CAT_BUS, 
//DATAB(GST_CAT_CALL_TRACE, 
//DATAB(GST_CAT_CAPS, 
//DATAB(GST_CAT_CLOCK, 
//DATAB(GST_CAT_CONTEXT, 
DATAB(GST_CAT_DEFAULT, sizeof(void*))
//DATAB(GST_CAT_ELEMENT_PADS, 
//DATAB(GST_CAT_ERROR_SYSTEM, 
//DATAB(GST_CAT_EVENT, 
//DATAB(GST_CAT_GST_INIT, 
//DATAB(GST_CAT_LOCKING, 
//DATAB(GST_CAT_MEMORY, 
//DATAB(GST_CAT_MESSAGE, 
//DATAB(GST_CAT_META, 
//DATAB(GST_CAT_NEGOTIATION, 
//DATAB(GST_CAT_PADS, 
//DATAB(GST_CAT_PARAMS, 
//DATAB(GST_CAT_PARENTAGE, 
//DATAB(GST_CAT_PERFORMANCE, 
//DATAB(GST_CAT_PIPELINE, 
//DATAB(GST_CAT_PLUGIN_INFO, 
//DATAB(GST_CAT_PLUGIN_LOADING, 
//DATAB(GST_CAT_PROBE, 
//DATAB(GST_CAT_PROPERTIES, 
//DATAB(GST_CAT_QOS, 
//DATAB(GST_CAT_REFCOUNTING, 
//DATAB(GST_CAT_REGISTRY, 
//DATAB(GST_CAT_SCHEDULING, 
//DATAB(GST_CAT_SIGNAL, 
//DATAB(GST_CAT_STATES, 
GO(gst_child_proxy_child_added, vFppp)
GO(gst_child_proxy_child_removed, vFppp)
//GO(gst_child_proxy_get, 
GO(gst_child_proxy_get_child_by_index, pFpu)
//GO(gst_child_proxy_get_child_by_name, 
GO(gst_child_proxy_get_children_count, uFp)
GO(gst_child_proxy_get_property, vFppp)
GO(gst_child_proxy_get_type, LFv)
//GO(gst_child_proxy_get_valist, 
GO(gst_child_proxy_lookup, iFpppp)
//GO(gst_child_proxy_set, 
GO(gst_child_proxy_set_property, vFppp)
//GO(gst_child_proxy_set_valist, 
//GO(gst_clear_buffer, 
//GO(gst_clear_buffer_list, 
GO(gst_clear_caps, vFp)
GO(gst_clear_event, vFp)
GO(gst_clear_message, vFp)
GO(gst_clear_mini_object, vFp)
//GO(gst_clear_object, 
GO(gst_clear_query, vFp)
GO(gst_clear_structure, vFp)
//GO(gst_clear_tag_list, 
//GO(gst_clear_uri, 
GO(gst_clock_add_observation, iFpLLp)
GO(gst_clock_add_observation_unapplied, iFpLLppppp)
//GO(gst_clock_adjust_unlocked, 
//GO(gst_clock_adjust_with_calibration, 
//GO(gst_clock_entry_type_get_type, 
//GO(gst_clock_flags_get_type, 
GO(gst_clock_get_calibration, vFppppp)
GO(gst_clock_get_internal_time, LFp)
GO(gst_clock_get_master, pFp)
//GO(gst_clock_get_resolution, 
GO(gst_clock_get_time, UFp)
//GO(gst_clock_get_timeout, 
//GO(gst_clock_get_type, 
//GO(gst_clock_id_compare_func, 
GO(gst_clock_id_get_clock, pFp)
GO(gst_clock_id_get_time, LFp)
GO(gst_clock_id_ref, pFp)
//GO(gst_clock_id_unref, 
GO(gst_clock_id_unschedule, vFp)
GO(gst_clock_id_uses_clock, iFpp)
GO(gst_clock_id_wait, uFpp)
//GO(gst_clock_id_wait_async, 
GO(gst_clock_is_synced, iFp)
GO(gst_clock_new_periodic_id, pFpLL)
GO(gst_clock_new_single_shot_id, pFpL)
GO(gst_clock_periodic_id_reinit, iFppLL)
//GO(gst_clock_return_get_type, 
GO(gst_clock_set_calibration, vFpLLLL)
GO(gst_clock_set_master, iFpp)
//GO(gst_clock_set_resolution, 
GO(gst_clock_set_synced, vFpi)
GO(gst_clock_set_timeout, vFpL)
GO(gst_clock_single_shot_id_reinit, iFppL)
//GO(gst_clock_type_get_type, 
GO(gst_clock_unadjust_unlocked, LFpL)
GO(gst_clock_unadjust_with_calibration, LFpLLLLL)
GO(gst_clock_wait_for_sync, iFpL)
GO(gst_context_copy, pFp)
GO(gst_context_get_context_type, pFp)
GO(gst_context_get_structure, pFp)
//GO(gst_context_get_type, 
GO(gst_context_has_context_type, iFpp)
GO(gst_context_is_persistent, iFp)
GO(gst_context_new, pFpi)
GO(gst_context_ref, pFp)
GO(gst_context_replace, iFpp)
//DATAB(_gst_context_type, 
//GO(gst_context_unref, 
GO(gst_context_writable_structure, pFp)
GO(gst_control_binding_get_g_value_array, iFpLLup)
//GO(gst_control_binding_get_type, 
GO(gst_control_binding_get_value, pFpL)
GO(gst_control_binding_get_value_array, iFpLLup)
GO(gst_control_binding_is_disabled, iFp)
GO(gst_control_binding_set_disabled, vFpi)
GO(gst_control_binding_sync_values, iFppLL)
//GO(gst_control_source_get_type, 
//GO(gst_control_source_get_value, 
//GO(gst_control_source_get_value_array, 
//GO(gst_core_error_get_type, 
GO(gst_core_error_quark, pFv)
//GO(gst_date_time_get_day, 
//GO(gst_date_time_get_hour, 
GO(gst_date_time_get_microsecond, iFp)
//GO(gst_date_time_get_minute, 
//GO(gst_date_time_get_month, 
//GO(gst_date_time_get_second, 
GO(gst_date_time_get_time_zone_offset, fFp)
//GO(gst_date_time_get_type, 
//GO(gst_date_time_get_year, 
//GO(gst_date_time_has_day, 
//GO(gst_date_time_has_month, 
GO(gst_date_time_has_second, iFp)
//GO(gst_date_time_has_time, 
//GO(gst_date_time_has_year, 
GO(gst_date_time_new, pFfiiiiid)
GO(gst_date_time_new_from_g_date_time, pFp)
GO(gst_date_time_new_from_iso8601_string, pFp)
//GO(gst_date_time_new_from_unix_epoch_local_time, 
//GO(gst_date_time_new_from_unix_epoch_local_time_usecs, 
//GO(gst_date_time_new_from_unix_epoch_utc, 
GO(gst_date_time_new_from_unix_epoch_utc_usecs, pFl)
GO(gst_date_time_new_local_time, pFiiiiid)
//GO(gst_date_time_new_now_local_time, 
GO(gst_date_time_new_now_utc, pFv)
GO(gst_date_time_new_y, pFi)
GO(gst_date_time_new_ym, pFii)
GO(gst_date_time_new_ymd, pFiii)
GO(gst_date_time_ref, pFp)
GO(gst_date_time_to_g_date_time, pFp)
GO(gst_date_time_to_iso8601_string, pFp)
DATAB(_gst_date_time_type, sizeof(void*))
GO(gst_date_time_unref, vFp)
//GO(gst_debug_add_log_function, 
GO(gst_debug_add_ring_buffer_logger, vFuu)
GO(gst_debug_bin_to_dot_data, pFpi)
//GO(gst_debug_bin_to_dot_file, 
GO(gst_debug_bin_to_dot_file_with_ts, vFpip)
//GO(gst_debug_category_free, 
GO(gst_debug_category_get_color, uFp)
GO(gst_debug_category_get_description, pFp)
//GO(gst_debug_category_get_name, 
GO(gst_debug_category_get_threshold, uFp)
GO(_gst_debug_category_new, pFpup)
GO(gst_debug_category_reset_threshold, vFp)
GO(gst_debug_category_set_threshold, vFpu)
//GO(gst_debug_color_flags_get_type, 
//GO(gst_debug_color_mode_get_type, 
GO(gst_debug_construct_term_color, pFu)
GO(gst_debug_construct_win_color, iFu)
GO(_gst_debug_dump_mem, vFpppipppu)
//DATAB(_gst_debug_enabled, 
GO(gst_debug_get_all_categories, pFv)
GO(_gst_debug_get_category, pFp)
GO(gst_debug_get_color_mode, uFv)
GO(gst_debug_get_default_threshold, uFv)
GO(gst_debug_get_stack_trace, pFu)
//GO(gst_debug_graph_details_get_type, 
//GO(gst_debug_is_active, 
//GO(gst_debug_is_colored, 
GO(gst_debug_level_get_name, pFu)
//GO(gst_debug_level_get_type, 
GOM(gst_debug_log, vFEpippippV)
GO(gst_debug_log_default, vFpuppippp)
GO(gst_debug_log_get_line, pFpuppipp)
GOM(gst_debug_log_valist, vFEpippippA)
//GO(gst_debug_message_get, 
DATAB(_gst_debug_min, sizeof(int))
//GO(_gst_debug_nameof_funcptr, 
//GO(gst_debug_print_stack_trace, 
GO(_gst_debug_register_funcptr, vFpp)
//GO(gst_debug_remove_log_function, 
GO(gst_debug_remove_log_function_by_data, uFp)
//GO(gst_debug_remove_ring_buffer_logger, 
GO(gst_debug_ring_buffer_logger_get_logs, pFv)
//GO(gst_debug_set_active, 
//GO(gst_debug_set_colored, 
GO(gst_debug_set_color_mode, vFu)
//GO(gst_debug_set_color_mode_from_string, 
GO(gst_debug_set_default_threshold, vFu)
GO(gst_debug_set_threshold_for_name, vFpu)
GO(gst_debug_set_threshold_from_string, vFpi)
GO(gst_debug_unset_threshold_for_name, vFp)
GO(gst_deinit, vFv)
GO(gst_device_create_element, pFpp)
//GO(gst_device_get_caps, 
GO(gst_device_get_device_class, pFp)
//GO(gst_device_get_display_name, 
GO(gst_device_get_properties, pFp)
//GO(gst_device_get_type, 
GO(gst_device_has_classes, iFpp)
GO(gst_device_has_classesv, iFpp)
GO(gst_device_monitor_add_filter, uFppp)
GO(gst_device_monitor_get_bus, pFp)
GO(gst_device_monitor_get_devices, pFp)
GO(gst_device_monitor_get_providers, pFp)
GO(gst_device_monitor_get_show_all_devices, iFp)
//GO(gst_device_monitor_get_type, 
GO(gst_device_monitor_new, pFv)
GO(gst_device_monitor_remove_filter, iFpu)
GO(gst_device_monitor_set_show_all_devices, vFpi)
GO(gst_device_monitor_start, iFp)
//GO(gst_device_monitor_stop, 
//GO(gst_device_provider_can_monitor, 
//GO(gst_device_provider_class_add_metadata, 
//GO(gst_device_provider_class_add_static_metadata, 
//GO(gst_device_provider_class_get_metadata, 
//GO(gst_device_provider_class_set_metadata, 
//GO(gst_device_provider_class_set_static_metadata, 
//GO(gst_device_provider_device_add, 
GO(gst_device_provider_device_changed, vFppp)
GO(gst_device_provider_device_remove, vFpp)
GO(gst_device_provider_factory_find, pFp)
GO(gst_device_provider_factory_get, pFp)
GO(gst_device_provider_factory_get_by_name, pFp)
GO(gst_device_provider_factory_get_device_provider_type, LFp)
GO(gst_device_provider_factory_get_metadata, pFpp)
GO(gst_device_provider_factory_get_metadata_keys, pFp)
//GO(gst_device_provider_factory_get_type, 
GO(gst_device_provider_factory_has_classes, iFpp)
GO(gst_device_provider_factory_has_classesv, iFpp)
GO(gst_device_provider_factory_list_get_device_providers, pFu)
GO(gst_device_provider_get_bus, pFp)
GO(gst_device_provider_get_devices, pFp)
GO(gst_device_provider_get_factory, pFp)
GO(gst_device_provider_get_hidden_providers, pFp)
GO(gst_device_provider_get_metadata, pFpp)
//GO(gst_device_provider_get_type, 
//GO(gst_device_provider_hide_provider, 
GO(gst_device_provider_register, iFppuL)
//GO(gst_device_provider_start, 
//GO(gst_device_provider_stop, 
GO(gst_device_provider_unhide_provider, vFpp)
GO(gst_device_reconfigure_element, iFpp)
//DATAB(_gst_disable_registry_cache, 
//GO(gst_double_range_get_type, 
//DATAB(_gst_double_range_type, 
//GO(gst_dynamic_type_factory_get_type, 
GO(gst_dynamic_type_factory_load, LFp)
GO(gst_dynamic_type_register, iFpL)
GO(gst_element_abort_state, vFp)
GO(gst_element_add_pad, iFpp)
GO(gst_element_add_property_deep_notify_watch, LFppi)
GO(gst_element_add_property_notify_watch, LFppi)
//GOM(gst_element_call_async, vFEpBpB)
GO(gst_element_change_state, uFpu)
GO(gst_element_class_add_metadata, vFppp)
GO(gst_element_class_add_pad_template, vFpp)
GO(gst_element_class_add_static_metadata, vFppp)
GO(gst_element_class_add_static_pad_template, vFpp)
GO(gst_element_class_add_static_pad_template_with_gtype, vFppp)
GO(gst_element_class_get_metadata, pFpp)
GO(gst_element_class_get_pad_template, pFpp)
GO(gst_element_class_get_pad_template_list, pFp)
GO(gst_element_class_set_metadata, vFppppp)
GO(gst_element_class_set_static_metadata, vFppppp)
GO(gst_element_continue_state, uFpu)
//GOM(gst_element_create_all_pads, iFEpBp)
GOM(_gst_element_error_printf, pFEpV)
GO(gst_element_factory_can_sink_all_caps, iFpp)
GO(gst_element_factory_can_sink_any_caps, iFpp)
GO(gst_element_factory_can_src_all_caps, iFpp)
GO(gst_element_factory_can_src_any_caps, iFpp)
GO(gst_element_factory_create, pFpp)
GO(gst_element_factory_find, pFp)
GO(gst_element_factory_get_element_type, LFp)
GO(gst_element_factory_get_metadata, pFpp)
GO(gst_element_factory_get_metadata_keys, pFp)
GO(gst_element_factory_get_num_pad_templates, uFp)
GO(gst_element_factory_get_static_pad_templates, pFp)
GO(gst_element_factory_get_type, LFv)
GO(gst_element_factory_get_uri_protocols, pFp)
GO(gst_element_factory_get_uri_type, uFp)
GO(gst_element_factory_has_interface, iFpp)
GO(gst_element_factory_list_filter, pFppii)
GO(gst_element_factory_list_get_elements, pFLu)
GO(gst_element_factory_list_is_type, iFpL)
GO(gst_element_factory_make, pFpp)
GO(gst_element_flags_get_type, LFv)
//GOM(gst_element_foreach_pad, iFEpBp)
GOM(gst_element_foreach_sink_pad, iFEppp)
//GOM(gst_element_foreach_src_pad, iFEpBp)
GO(gst_element_get_base_time, UFp)
GO(gst_element_get_bus, pFp)
GO(gst_element_get_clock, pFp)
GO(gst_element_get_compatible_pad, pFppp)
GO(gst_element_get_compatible_pad_template, pFpp)
GO(gst_element_get_context, pFpp)
GO(gst_element_get_contexts, pFp)
GO(gst_element_get_context_unlocked, pFpp)
GO(gst_element_get_current_clock_time, LFp)
GO(gst_element_get_current_running_time, UFp)
GO(gst_element_get_factory, pFp)
GO(gst_element_get_metadata, pFpp)
GO(gst_element_get_pad_template, pFpp)
GO(gst_element_get_pad_template_list, pFp)
//GO(gst_element_get_request_pad, 
GO(gst_element_get_start_time, UFp)
GO(gst_element_get_state, uFpppL)
GO(gst_element_get_static_pad, pFpp)
GO(gst_element_get_type, LFv)
GO(gst_element_is_locked_state, iFp)
GO(gst_element_iterate_pads, pFp)
GO(gst_element_iterate_sink_pads, pFp)
GO(gst_element_iterate_src_pads, pFp)
GO(gst_element_link, iFpp)
GO(gst_element_link_filtered, iFppp)
GOM(gst_element_link_many, iFEppV)
GO(gst_element_link_pads, iFpppp)
GO(gst_element_link_pads_filtered, iFppppp)
GO(gst_element_link_pads_full, iFppppu)
GO(gst_element_lost_state, vFp)
GO(gst_element_make_from_uri, pFuppp)
GO(gst_element_message_full, vFpiuippppi)
GO(gst_element_message_full_with_details, vFpiuippppip)
GO(gst_element_no_more_pads, vFp)
GO(gst_element_post_message, iFpp)
GO(gst_element_provide_clock, pFp)
GO(gst_element_query, iFpp)
GO(gst_element_query_convert, iFpulup)
GO(gst_element_query_duration, iFpup)
GO(gst_element_query_position, iFpip)
GO(gst_element_register, iFppup)
GO(gst_element_release_request_pad, vFpp)
GO(gst_element_remove_pad, iFpp)
GO(gst_element_remove_property_notify_watch, vFpL)
GO(gst_element_request_pad, pFpppp)
GO(gst_element_seek, iFpduuulul)
GO(gst_element_seek_simple, iFpuul)
GO(gst_element_send_event, iFpp)
GO(gst_element_set_base_time, vFpU)
GO(gst_element_set_bus, vFpp)
GO(gst_element_set_clock, iFpp)
GO(gst_element_set_context, vFpp)
GO(gst_element_set_locked_state, iFpi)
GO(gst_element_set_start_time, vFpL)
GO(gst_element_set_state, uFpu)
GO(gst_element_state_change_return_get_name, pFu)
GO(gst_element_state_get_name, pFu)
GO(gst_element_sync_state_with_parent, iFp)
GO(gst_element_unlink, vFpp)
//GOM(gst_element_unlink_many, vFEppV)
GO(gst_element_unlink_pads, vFpppp)
GO(gst_error_get_message, pFui)
GO(gst_event_copy, pFp)
GO(gst_event_copy_segment, vFpp)
GO(gst_event_get_running_time_offset, IFp)
GO(gst_event_get_seqnum, uFp)
GO(gst_event_get_structure, pFp)
GO(gst_event_get_type, LFv)
GO(gst_event_has_name, iFpp)
GO(gst_event_has_name_id, iFpp)
GO(gst_event_new_buffer_size, pFiIIi)
GO(gst_event_new_caps, pFp)
GO(gst_event_new_custom, pFpp)
GO(gst_event_new_eos, pFv)
GO(gst_event_new_flush_start, pFv)
GO(gst_event_new_flush_stop, pFi)
GO(gst_event_new_gap, pFUU)
GO(gst_event_new_instant_rate_change, pFdi)
GO(gst_event_new_instant_rate_sync_time, pFdUU)
GO(gst_event_new_latency, pFU)
GO(gst_event_new_navigation, pFp)
GO(gst_event_new_protection, pFppp)
GO(gst_event_new_qos, pFpdIU)
GO(gst_event_new_reconfigure, pFv)
GO(gst_event_new_seek, pFdiiiIiI)
GO(gst_event_new_segment, pFp)
GO(gst_event_new_segment_done, pFiI)
GO(gst_event_new_select_streams, pFp)
GO(gst_event_new_sink_message, pFpp)
GO(gst_event_new_step, pFpUdii)
GO(gst_event_new_stream_collection, pFp)
GO(gst_event_new_stream_group_done, pFu)
GO(gst_event_new_stream_start, pFp)
GO(gst_event_new_tag, pFp)
GO(gst_event_new_toc, pFpi)
GO(gst_event_new_toc_select, pFp)
GO(gst_event_parse_buffer_size, vFppppp)
GO(gst_event_parse_caps, vFpp)
GO(gst_event_parse_flush_stop, vFpp)
GO(gst_event_parse_gap, vFppp)
GO(gst_event_parse_group_id, vFpp)
GO(gst_event_parse_instant_rate_change, vFppp)
GO(gst_event_parse_instant_rate_sync_time, vFpppp)
GO(gst_event_parse_latency, vFpp)
GO(gst_event_parse_protection, vFpppp)
GO(gst_event_parse_qos, vFppppp)
GO(gst_event_parse_seek, vFpppppppp)
GO(gst_event_parse_seek_trickmode_interval, vFpp)
GO(gst_event_parse_segment, vFpp)
GO(gst_event_parse_segment_done, vFppp)
GO(gst_event_parse_select_streams, vFpp)
GO(gst_event_parse_sink_message, vFpp)
GO(gst_event_parse_step, vFpppppp)
GO(gst_event_parse_stream, vFpp)
GO(gst_event_parse_stream_collection, vFpp)
GO(gst_event_parse_stream_flags, vFpp)
GO(gst_event_parse_stream_group_done, vFpp)
GO(gst_event_parse_stream_start, vFpp)
GO(gst_event_parse_tag, vFpp)
GO(gst_event_parse_toc, vFppp)
GO(gst_event_parse_toc_select, vFpp)
GO(gst_event_ref, pFp)
GO(gst_event_replace, iFpp)
GO(gst_event_set_group_id, vFpu)
GO(gst_event_set_running_time_offset, vFpI)
GO(gst_event_set_seek_trickmode_interval, vFpU)
GO(gst_event_set_seqnum, vFpu)
GO(gst_event_set_stream, vFpp)
GO(gst_event_set_stream_flags, vFpi)
GO(gst_event_steal, pFp)
GO(gst_event_take, iFpp)
DATAB(_gst_event_type, sizeof(void*))
GO(gst_event_type_flags_get_type, LFv)
GO(gst_event_type_get_flags, uFu)
GO(gst_event_type_get_name, pFu)
GO(gst_event_type_get_type, LFv)
GO(gst_event_type_to_quark, uFu)
GO(gst_event_unref, vFp)
GO(gst_event_writable_structure, pFp)
//DATAB(_gst_executable_path, 
GO(gst_filename_to_uri, pFpp)
//GO(gst_flagset_get_type, 
GO(gst_flagset_register, LFL)
//DATAB(_gst_flagset_type, 
GO(gst_flow_get_name, pFi)
GO(gst_flow_return_get_type, LFv)
GO(gst_flow_to_quark, uFi)
GO(gst_format_get_by_nick, uFp)
GO(gst_format_get_details, pFu)
GO(gst_format_get_name, pFu)
GO(gst_format_get_type, LFv)
GO(gst_format_iterate_definitions, pFv)
GO(gst_format_register, uFpp)
GO(gst_formats_contains, iFpu)
GO(gst_format_to_quark, uFu)
GO(gst_fraction_get_type, LFv)
GO(gst_fraction_range_get_type, LFv)
DATAB(_gst_fraction_range_type, sizeof(void*))
DATAB(_gst_fraction_type, sizeof(void*))
GO(gst_get_main_executable_path, pFv)
//GO(gst_ghost_pad_activate_mode_default, 
//GO(gst_ghost_pad_construct, 
//GO(gst_ghost_pad_get_target, 
GO(gst_ghost_pad_get_type, LFv)
//GO(gst_ghost_pad_internal_activate_mode_default, 
GO(gst_ghost_pad_new, pFpp)
//GO(gst_ghost_pad_new_from_template, 
//GO(gst_ghost_pad_new_no_target, 
//GO(gst_ghost_pad_new_no_target_from_template, 
GO(gst_ghost_pad_set_target, iFpp)
//GO(gst_g_thread_get_type, 
//GO(gst_info_strdup_printf, 
//GO(gst_info_strdup_vprintf, 
GOM(gst_info_vasprintf, iFEppA)
GOM(gst_init, vFEpp)
GOM(gst_init_check, iFEppp)
GO(gst_init_get_option_group, pFv)
//GO(gst_int64_range_get_type, 
//DATAB(_gst_int64_range_type, 
GO(gst_int_range_get_type, LFv)
DATAB(_gst_int_range_type, sizeof(void*))
GO(gst_is_caps_features, iFp)
GO(gst_is_initialized, iFv)
//GO(gst_iterator_copy, 
//GO(gst_iterator_filter, 
//GO(gst_iterator_find_custom, 
GOM(gst_iterator_fold, iFEpppp)
//GO(gst_iterator_foreach, 
GO(gst_iterator_free, vFp)
//GO(gst_iterator_get_type, 
//GO(gst_iterator_item_get_type, 
//GO(gst_iterator_new, 
//GO(gst_iterator_new_list, 
GO(gst_iterator_new_single, pFLp)
GO(gst_iterator_next, iFpp)
//GO(gst_iterator_push, 
//GO(gst_iterator_result_get_type, 
GO(gst_iterator_resync, vFp)
//GO(gst_library_error_get_type, 
GO(gst_library_error_quark, pFv)
//GO(gst_lock_flags_get_type, 
GOM(gst_make_element_message_details, pFEpV)
//GO(gst_map_flags_get_type, 
//DATA(gst_memory_alignment, 
//GO(gst_memory_copy, 
//GO(gst_memory_flags_get_type, 
GO(gst_memory_get_sizes, iFppp)
//GO(gst_memory_get_type, 
GO(gst_memory_init, vFpuppLLLL)
//GO(gst_memory_is_span, 
//GO(gst_memory_is_type, 
//GO(gst_memory_make_mapped, 
GO(gst_memory_map, iFppu)
GOM(gst_memory_new_wrapped, pFEipLLLpp)
//GO(gst_memory_ref, 
//GO(gst_memory_resize, 
//GO(gst_memory_share, 
//DATAB(_gst_memory_type, 
GO(gst_memory_unmap, vFpp)
//GO(gst_memory_unref, 
GO(gst_message_add_redirect_entry, vFpppp)
GO(gst_message_copy, pFp)
GO(gst_message_get_num_redirect_entries, lFp)
GO(gst_message_get_seqnum, uFp)
GO(gst_message_get_stream_status_object, pFp)
GO(gst_message_get_structure, pFp)
GO(gst_message_get_type, LFv)
GO(gst_message_has_name, iFpp)
GO(gst_message_new_application, pFpp)
GO(gst_message_new_async_done, pFpU)
GO(gst_message_new_async_start, pFp)
GO(gst_message_new_buffering, pFpi)
GO(gst_message_new_clock_lost, pFpp)
GO(gst_message_new_clock_provide, pFppi)
GO(gst_message_new_custom, pFppp)
GO(gst_message_new_device_added, pFpp)
GO(gst_message_new_device_changed, pFppp)
GO(gst_message_new_device_removed, pFpp)
GO(gst_message_new_duration_changed, pFp)
GO(gst_message_new_element, pFpp)
GO(gst_message_new_eos, pFp)
GO(gst_message_new_error, pFppp)
GO(gst_message_new_error_with_details, pFpppp)
GO(gst_message_new_have_context, pFpp)
GO(gst_message_new_info, pFppp)
GO(gst_message_new_info_with_details, pFpppp)
GO(gst_message_new_instant_rate_request, pFpp)
GO(gst_message_new_latency, pFp)
GO(gst_message_new_need_context, pFpp)
GO(gst_message_new_new_clock, pFpp)
GO(gst_message_new_progress, pFpppp)
GO(gst_message_new_property_notify, pFppp)
GO(gst_message_new_qos, pFpiUUUU)
GO(gst_message_new_redirect, pFpppp)
GO(gst_message_new_request_state, pFpp)
GO(gst_message_new_reset_time, pFpU)
GO(gst_message_new_segment_done, pFppI)
GO(gst_message_new_segment_start, pFppI)
GO(gst_message_new_state_changed, pFpiii)
GO(gst_message_new_state_dirty, pFp)
GO(gst_message_new_step_done, pFpiUdiiUi)
GO(gst_message_new_step_start, pFpiiUdii)
GO(gst_message_new_stream_collection, pFpp)
GO(gst_message_new_streams_selected, pFpp)
GO(gst_message_new_stream_start, pFp)
GO(gst_message_new_stream_status, pFpip)
GO(gst_message_new_structure_change, pFpppi)
GO(gst_message_new_tag, pFpp)
GO(gst_message_new_toc, pFppi)
GO(gst_message_new_warning, pFppp)
GO(gst_message_new_warning_with_details, pFpppp)
GO(gst_message_parse_async_done, vFpp)
GO(gst_message_parse_buffering, vFpp)
GO(gst_message_parse_buffering_stats, vFppppp)
GO(gst_message_parse_clock_lost, vFpp)
GO(gst_message_parse_clock_provide, vFppp)
GO(gst_message_parse_context_type, iFpp)
GO(gst_message_parse_device_added, vFpp)
GO(gst_message_parse_device_changed, vFppp)
GO(gst_message_parse_device_removed, vFpp)
GO(gst_message_parse_error, vFppp)
GO(gst_message_parse_error_details, vFpp)
GO(gst_message_parse_group_id, iFpp)
GO(gst_message_parse_have_context, vFpp)
GO(gst_message_parse_info, vFppp)
GO(gst_message_parse_info_details, vFpp)
GO(gst_message_parse_instant_rate_request, vFpp)
GO(gst_message_parse_new_clock, vFpp)
GO(gst_message_parse_progress, vFpppp)
GO(gst_message_parse_property_notify, vFpppp)
GO(gst_message_parse_qos, vFpppppp)
GO(gst_message_parse_qos_stats, vFpppp)
GO(gst_message_parse_qos_values, vFpppp)
GO(gst_message_parse_redirect_entry, vFplppp)
GO(gst_message_parse_request_state, vFpp)
GO(gst_message_parse_reset_time, vFpp)
GO(gst_message_parse_segment_done, vFppp)
GO(gst_message_parse_segment_start, vFppp)
GO(gst_message_parse_state_changed, vFpppp)
GO(gst_message_parse_step_done, vFpppppppp)
GO(gst_message_parse_step_start, vFppppppp)
GO(gst_message_parse_stream_collection, vFpp)
GO(gst_message_parse_streams_selected, vFpp)
GO(gst_message_parse_stream_status, vFppp)
GO(gst_message_parse_structure_change, vFpppp)
GO(gst_message_parse_tag, vFpp)
GO(gst_message_parse_toc, vFppp)
GO(gst_message_parse_warning, vFppp)
GO(gst_message_parse_warning_details, vFpp)
GO(gst_message_ref, pFp)
GO(gst_message_replace, iFpp)
GO(gst_message_set_buffering_stats, vFpiiiI)
GO(gst_message_set_group_id, vFpu)
GO(gst_message_set_qos_stats, vFpiUU)
GO(gst_message_set_qos_values, vFpIdi)
GO(gst_message_set_seqnum, vFpu)
GO(gst_message_set_stream_status_object, vFpp)
GO(gst_message_streams_selected_add, vFpp)
GO(gst_message_streams_selected_get_size, uFp)
GO(gst_message_streams_selected_get_stream, pFpu)
GO(gst_message_take, iFpp)
DATAB(_gst_message_type, sizeof(void*))
GO(gst_message_type_get_name, pFi)
GO(gst_message_type_get_type, LFv)
GO(gst_message_type_to_quark, uFi)
GO(gst_message_unref, vFp)
GO(gst_message_writable_structure, pFp)
GO(gst_meta_api_type_get_tags, pFL)
GO(gst_meta_api_type_has_tag, iFLu)
GO(gst_meta_api_type_register, LFpp)
//GO(gst_meta_compare_seqnum, 
//GO(gst_meta_flags_get_type, 
//GO(gst_meta_get_info, 
//GO(gst_meta_get_seqnum, 
//GO(gst_meta_register, 
//DATAB(_gst_meta_tag_memory, 
//DATAB(_gst_meta_transform_copy, 
//GO(gst_mini_object_add_parent, 
GO(gst_mini_object_copy, pFp)
//GO(gst_mini_object_flags_get_type, 
GO(gst_mini_object_get_qdata, pFpp)
//GO(gst_mini_object_init, 
GO(gst_mini_object_is_writable, iFp)
//GO(gst_mini_object_lock, 
GO(gst_mini_object_make_writable, pFp)
GO(gst_mini_object_ref, pFp)
//GO(gst_mini_object_remove_parent, 
GO(gst_mini_object_replace, iFpp)
GOM(gst_mini_object_set_qdata, vFEpppp)
//GO(gst_mini_object_steal, 
//GO(gst_mini_object_steal_qdata, 
//GO(gst_mini_object_take, 
//GO(gst_mini_object_unlock, 
GO(gst_mini_object_unref, vFp)
//GO(gst_mini_object_weak_ref, 
//GO(gst_mini_object_weak_unref, 
GO(gst_object_add_control_binding, iFpp)
GO(gst_object_check_uniqueness, iFpp)
GO(gst_object_default_deep_notify, vFpppp)
GO(gst_object_default_error, vFppp)
GO(gst_object_flags_get_type, LFv)
GO(gst_object_get_control_binding, pFpp)
GO(gst_object_get_control_rate, LFp)
GO(gst_object_get_g_value_array, iFppLLup)
GO(gst_object_get_name, pFp)
GO(gst_object_get_parent, pFp)
GO(gst_object_get_path_string, pFp)
GO(gst_object_get_type, LFv)
GO(gst_object_get_value, pFppL)
GO(gst_object_get_value_array, iFppLLup)
GO(gst_object_has_active_control_bindings, iFp)
GO(gst_object_has_ancestor, iFpp)
GO(gst_object_has_as_ancestor, iFpp)
GO(gst_object_has_as_parent, iFpp)
GO(gst_object_ref, pFp)
GO(gst_object_ref_sink, pFp)
GO(gst_object_remove_control_binding, iFpp)
GO(gst_object_replace, iFpp)
GO(gst_object_set_control_binding_disabled, vFppi)
GO(gst_object_set_control_bindings_disabled, vFpi)
GO(gst_object_set_control_rate, vFpL)
GO(gst_object_set_name, iFpp)
GO(gst_object_set_parent, iFpp)
GO(gst_object_suggest_next_sync, UFp)
GO(gst_object_sync_values, iFpL)
GO(gst_object_unparent, vFp)
GO(gst_object_unref, vFp)
GO(gst_pad_activate_mode, iFpii)
GOM(gst_pad_add_probe, LFEpippp)
GO(gst_pad_can_link, iFpp)
GO(gst_pad_chain, iFpp)
GO(gst_pad_chain_list, iFpp)
GO(gst_pad_check_reconfigure, iFp)
GO(gst_pad_create_stream_id, pFppp)
GOM(gst_pad_create_stream_id_printf, pFEpppV)
GOM(gst_pad_create_stream_id_printf_valist, pFEpppA)
GO(gst_pad_direction_get_type, LFv)
GO(gst_pad_event_default, iFppp)
GO(gst_pad_flags_get_type, LFv)
//GOM(gst_pad_forward, iFEpBp)
GO(gst_pad_get_allowed_caps, pFp)
GO(gst_pad_get_current_caps, pFp)
GO(gst_pad_get_direction, iFp)
GO(gst_pad_get_element_private, pFp)
GO(gst_pad_get_last_flow_return, iFp)
GO(gst_pad_get_offset, IFp)
GO(gst_pad_get_pad_template, pFp)
GO(gst_pad_get_pad_template_caps, pFp)
GO(gst_pad_get_parent_element, pFp)
GO(gst_pad_get_peer, pFp)
GO(gst_pad_get_range, iFpUup)
GO(gst_pad_get_single_internal_link, pFp)
GO(gst_pad_get_sticky_event, pFpiu)
GO(gst_pad_get_stream, pFp)
GO(gst_pad_get_stream_id, pFp)
GO(gst_pad_get_task_state, iFp)
GO(gst_pad_get_type, LFv)
GO(gst_pad_has_current_caps, iFp)
GO(gst_pad_is_active, iFp)
GO(gst_pad_is_blocked, iFp)
GO(gst_pad_is_blocking, iFp)
GO(gst_pad_is_linked, iFp)
GO(gst_pad_iterate_internal_links, pFp)
GO(gst_pad_iterate_internal_links_default, pFpp)
GO(gst_pad_link, iFpp)
GO(gst_pad_link_check_get_type, LFv)
GO(gst_pad_link_full, iFppi)
GO(gst_pad_link_get_name, pFi)
GO(gst_pad_link_maybe_ghosting, iFpp)
GO(gst_pad_link_maybe_ghosting_full, iFppi)
GO(gst_pad_link_return_get_type, LFv)
GO(gst_pad_mark_reconfigure, vFp)
GO(gst_pad_mode_get_name, pFu)
GO(gst_pad_mode_get_type, LFv)
GO(gst_pad_needs_reconfigure, iFp)
GO(gst_pad_new, pFpi)
GO(gst_pad_new_from_static_template, pFpp)
GO(gst_pad_new_from_template, pFpp)
GO(gst_pad_pause_task, iFp)
GO(gst_pad_peer_query, iFpp)
GO(gst_pad_peer_query_accept_caps, iFpp)
GO(gst_pad_peer_query_caps, pFpp)
GO(gst_pad_peer_query_convert, iFpiIip)
GO(gst_pad_peer_query_duration, iFpip)
GO(gst_pad_peer_query_position, iFpip)
GO(gst_pad_presence_get_type, LFv)
//GO(gst_pad_probe_info_get_buffer, 
GO(gst_pad_probe_info_get_buffer_list, pFp)
//GO(gst_pad_probe_info_get_event, 
//GO(gst_pad_probe_info_get_query, 
//GO(gst_pad_probe_return_get_type, 
//GO(gst_pad_probe_type_get_type, 
GO(gst_pad_proxy_query_accept_caps, iFpp)
GO(gst_pad_proxy_query_caps, iFpp)
GO(gst_pad_pull_range, iFpUup)
GO(gst_pad_push, iFpp)
GO(gst_pad_push_event, iFpp)
GO(gst_pad_push_list, iFpp)
GO(gst_pad_query, iFpp)
GO(gst_pad_query_accept_caps, iFpp)
GO(gst_pad_query_caps, pFpp)
GO(gst_pad_query_convert, iFpiIip)
GO(gst_pad_query_default, iFppp)
GO(gst_pad_query_duration, iFppp)
GO(gst_pad_query_position, iFppp)
GO(gst_pad_remove_probe, vFpL)
GO(gst_pad_send_event, iFpp)
GOM(gst_pad_set_activate_function_full, vFEpppp)
GOM(gst_pad_set_activatemode_function_full, vFEpppp)
GO(gst_pad_set_active, iFpi)
GOM(gst_pad_set_chain_function_full, vFEpppp)
//GOM(gst_pad_set_chain_list_function_full, vFEpBpB)
GO(gst_pad_set_element_private, vFpp)
//GOM(gst_pad_set_event_full_function_full, vFpBpB)
GOM(gst_pad_set_event_function_full, vFEpppp)
GOM(gst_pad_set_getrange_function_full, vFEpppp)
GOM(gst_pad_set_iterate_internal_links_function_full, vFEpppp)
GOM(gst_pad_set_link_function_full, vFEpppp)
GO(gst_pad_set_offset, vFpI)
GOM(gst_pad_set_query_function_full, vFEpppp)
//GOM(gst_pad_set_unlink_function_full, vFEpBpB)
GOM(gst_pad_start_task, iFEpppp)
GOM(gst_pad_sticky_events_foreach, vFEppp)
GO(gst_pad_stop_task, iFp)
GO(gst_pad_store_sticky_event, iFpp)
GO(gst_pad_template_flags_get_type, LFv)
GO(gst_pad_template_get_caps, pFp)
//GO(gst_pad_template_get_documentation_caps, 
//GO(gst_pad_template_get_type, 
GO(gst_pad_template_new, pFpiip)
//GO(gst_pad_template_new_from_static_pad_template_with_gtype, 
GO(gst_pad_template_new_with_gtype, pFpiipL)
//GO(gst_pad_template_pad_created, 
//GO(gst_pad_template_set_documentation_caps, 
GO(gst_pad_unlink, iFpp)
GO(gst_pad_use_fixed_caps, vFp)
GO(gst_param_spec_array, pFppppi)
//GO(gst_param_spec_array_get_type, 
GO(gst_param_spec_fraction, pFpppiiiiiii)
//GO(gst_param_spec_fraction_get_type, 
//GO(gst_parent_buffer_meta_api_get_type, 
//GO(gst_parent_buffer_meta_get_info, 
GO(gst_parse_bin_from_description, pFpip)
GO(gst_parse_bin_from_description_full, pFpipup)
GO(gst_parse_context_copy, pFp)
//GO(gst_parse_context_free, 
GO(gst_parse_context_get_missing_elements, pFp)
GO(gst_parse_context_get_type, LFv)
GO(gst_parse_context_new, pFv)
//GO(gst_parse_error_get_type, 
GO(gst_parse_error_quark, uFv)
//GO(gst_parse_flags_get_type, 
GO(gst_parse_launch, pFpp)
GO(gst_parse_launch_full, pFppup)
GO(gst_parse_launchv, pFpp)
GO(gst_parse_launchv_full, pFppup)
//GO(gst_pipeline_auto_clock, 
//GO(gst_pipeline_flags_get_type, 
GO(gst_pipeline_get_auto_flush_bus, iFp)
GO(gst_pipeline_get_bus, pFp)
//GO(gst_pipeline_get_clock, 
//GO(gst_pipeline_get_delay, 
GO(gst_pipeline_get_latency, LFp)
GO(gst_pipeline_get_pipeline_clock, pFp)
//GO(gst_pipeline_get_type, 
GO(gst_pipeline_new, pFp)
GO(gst_pipeline_set_auto_flush_bus, vFpi)
GO(gst_pipeline_set_clock, iFpp)
//GO(gst_pipeline_set_delay, 
GO(gst_pipeline_set_latency, vFpL)
GO(gst_pipeline_use_clock, vFpp)
GO(gst_plugin_add_dependency, vFppppu)
GO(gst_plugin_add_dependency_simple, vFppppu)
//GO(gst_plugin_api_flags_get_type, 
//GO(gst_plugin_dependency_flags_get_type, 
//GO(gst_plugin_error_get_type, 
//GO(gst_plugin_error_quark, 
GO(gst_plugin_feature_check_version, iFpuuu)
GO(gst_plugin_feature_get_plugin, pFp)
GO(gst_plugin_feature_get_plugin_name, pFp)
GO(gst_plugin_feature_get_rank, uFp)
GO(gst_plugin_feature_get_type, LFv)
GO(gst_plugin_feature_list_copy, pFp)
GO(gst_plugin_feature_list_debug, vFp)
GO(gst_plugin_feature_list_free, vFp)
GO(gst_plugin_feature_load, pFp)
GO(gst_plugin_feature_rank_compare_func, iFpp)
GO(gst_plugin_feature_set_rank, vFpu)
//GO(gst_plugin_flags_get_type, 
GO(gst_plugin_get_cache_data, pFp)
//GO(gst_plugin_get_description, 
//GO(gst_plugin_get_filename, 
//GO(gst_plugin_get_license, 
//GO(gst_plugin_get_name, 
//GO(gst_plugin_get_origin, 
//GO(gst_plugin_get_package, 
GO(gst_plugin_get_release_date_string, pFp)
//GO(gst_plugin_get_source, 
//GO(gst_plugin_get_type, 
//GO(gst_plugin_get_version, 
GO(gst_plugin_is_loaded, iFp)
//GO(gst_plugin_list_free, 
GO(gst_plugin_load, pFp)
GO(gst_plugin_load_by_name, pFp)
//GO(_gst_plugin_loader_client_run, 
GOM(gst_plugin_load_file, pFEpp)
GOM(gst_plugin_register_static, iFEiipppppppp)
//GO(gst_plugin_register_static_full, 
GO(gst_plugin_set_cache_data, vFpp)
//GO(gst_poll_add_fd, 
//GO(gst_poll_fd_can_read, 
//GO(gst_poll_fd_can_write, 
GO(gst_poll_fd_ctl_pri, iFppi)
//GO(gst_poll_fd_ctl_read, 
//GO(gst_poll_fd_ctl_write, 
//GO(gst_poll_fd_has_closed, 
//GO(gst_poll_fd_has_error, 
GO(gst_poll_fd_has_pri, iFpp)
GO(gst_poll_fd_ignored, vFpp)
GO(gst_poll_fd_init, vFp)
//GO(gst_poll_free, 
GO(gst_poll_get_read_gpollfd, vFpp)
GO(gst_poll_new, pFi)
GO(gst_poll_new_timer, pFv)
GO(gst_poll_read_control, iFp)
GO(gst_poll_remove_fd, iFpp)
GO(gst_poll_restart, vFp)
GO(gst_poll_set_controllable, iFpi)
GO(gst_poll_set_flushing, vFpi)
GO(gst_poll_wait, iFpL)
//GO(gst_poll_write_control, 
GO(gst_preset_delete_preset, iFpp)
//GO(gst_preset_get_app_dir, 
GO(gst_preset_get_meta, iFpppp)
//GO(gst_preset_get_preset_names, 
GO(gst_preset_get_property_names, pFp)
GO(gst_preset_get_type, LFv)
GO(gst_preset_is_editable, iFp)
//GO(gst_preset_load_preset, 
GO(gst_preset_rename_preset, iFppp)
//GO(gst_preset_save_preset, 
GO(gst_preset_set_app_dir, iFp)
GO(gst_preset_set_meta, iFpppp)
//GO(gst_print, 
//GO(gst_printerr, 
//GO(gst_printerrln, 
//GO(gst_println, 
//GO(gst_progress_type_get_type, 
//GO(gst_promise_expire, 
//GO(gst_promise_get_reply, 
//GO(gst_promise_get_type, 
//GO(gst_promise_interrupt, 
//GO(gst_promise_new, 
//GO(gst_promise_new_with_change_func, 
//GO(gst_promise_ref, 
//GO(gst_promise_reply, 
//GO(gst_promise_result_get_type, 
//GO(gst_promise_unref, 
//GO(gst_promise_wait, 
GO(gst_protection_filter_systems_by_available_decryptors, pFp)
//GO(gst_protection_meta_api_get_type, 
//GO(gst_protection_meta_get_info, 
GO(gst_protection_select_system, pFp)
//GO(gst_proxy_pad_chain_default, 
//GO(gst_proxy_pad_chain_list_default, 
//GO(gst_proxy_pad_get_internal, 
//GO(gst_proxy_pad_getrange_default, 
//GO(gst_proxy_pad_get_type, 
//GO(gst_proxy_pad_iterate_internal_links_default, 
//GO(gst_qos_type_get_type, 
GO(gst_query_add_allocation_meta, vFppp)
GO(gst_query_add_allocation_param, vFppp)
GO(gst_query_add_allocation_pool, vFppuuu)
GO(gst_query_add_buffering_range, iFpII)
GO(gst_query_add_scheduling_mode, vFpi)
GO(gst_query_copy, pFp)
GO(gst_query_find_allocation_meta, iFppp)
GO(gst_query_get_n_allocation_metas, uFp)
GO(gst_query_get_n_allocation_params, uFp)
GO(gst_query_get_n_allocation_pools, uFp)
GO(gst_query_get_n_buffering_ranges, uFp)
GO(gst_query_get_n_scheduling_modes, uFp)
GO(gst_query_get_structure, pFp)
GO(gst_query_get_type, LFv)
GO(gst_query_has_scheduling_mode, iFpi)
GO(gst_query_has_scheduling_mode_with_flags, iFpii)
GO(gst_query_new_accept_caps, pFp)
GO(gst_query_new_allocation, pFpi)
GO(gst_query_new_bitrate, pFv)
GO(gst_query_new_buffering, pFi)
GO(gst_query_new_caps, pFp)
GO(gst_query_new_context, pFp)
GO(gst_query_new_convert, pFiIi)
GO(gst_query_new_custom, pFip)
GO(gst_query_new_drain, pFv)
GO(gst_query_new_duration, pFi)
GO(gst_query_new_formats, pFv)
GO(gst_query_new_latency, pFv)
GO(gst_query_new_position, pFi)
GO(gst_query_new_scheduling, pFv)
GO(gst_query_new_seeking, pFi)
GO(gst_query_new_segment, pFi)
GO(gst_query_new_uri, pFv)
GO(gst_query_parse_accept_caps, vFpp)
GO(gst_query_parse_accept_caps_result, vFpp)
GO(gst_query_parse_allocation, vFppp)
GO(gst_query_parse_bitrate, vFpp)
GO(gst_query_parse_buffering_percent, vFppp)
GO(gst_query_parse_buffering_range, vFppppp)
GO(gst_query_parse_buffering_stats, vFppppp)
GO(gst_query_parse_caps, vFpp)
GO(gst_query_parse_caps_result, vFpp)
GO(gst_query_parse_context, vFpp)
GO(gst_query_parse_context_type, iFpp)
GO(gst_query_parse_convert, vFppppp)
GO(gst_query_parse_duration, vFppp)
GO(gst_query_parse_latency, vFpppp)
GO(gst_query_parse_n_formats, vFpp)
GO(gst_query_parse_nth_allocation_meta, pFpup)
GO(gst_query_parse_nth_allocation_param, vFpupp)
GO(gst_query_parse_nth_allocation_pool, vFpupppp)
GO(gst_query_parse_nth_buffering_range, iFpupp)
GO(gst_query_parse_nth_format, vFpup)
GO(gst_query_parse_nth_scheduling_mode, iFpu)
GO(gst_query_parse_position, vFppp)
GO(gst_query_parse_scheduling, vFppppp)
GO(gst_query_parse_seeking, vFppppp)
GO(gst_query_parse_segment, vFppppp)
GO(gst_query_parse_uri, vFpp)
GO(gst_query_parse_uri_redirection, vFpp)
GO(gst_query_parse_uri_redirection_permanent, vFpp)
GO(gst_query_ref, pFp)
GO(gst_query_remove_nth_allocation_meta, vFpu)
GO(gst_query_remove_nth_allocation_param, vFpu)
GO(gst_query_remove_nth_allocation_pool, vFpu)
GO(gst_query_replace, iFpp)
GO(gst_query_set_accept_caps_result, vFpi)
GO(gst_query_set_bitrate, vFpu)
GO(gst_query_set_buffering_percent, vFpii)
GO(gst_query_set_buffering_range, vFpiIII)
GO(gst_query_set_buffering_stats, vFpiiiI)
GO(gst_query_set_caps_result, vFpp)
GO(gst_query_set_context, vFpp)
GO(gst_query_set_convert, vFpiIiI)
GO(gst_query_set_duration, vFpiI)
//GOM(gst_query_set_formats, vFEpiV)
GO(gst_query_set_formatsv, vFppp)
GO(gst_query_set_latency, vFpiUU)
GO(gst_query_set_nth_allocation_param, vFpupp)
GO(gst_query_set_nth_allocation_pool, vFpupuuu)
GO(gst_query_set_position, vFpiI)
GO(gst_query_set_scheduling, vFpiiii)
GO(gst_query_set_seeking, vFpiiII)
GO(gst_query_set_segment, vFpdiII)
GO(gst_query_set_uri, vFpp)
GO(gst_query_set_uri_redirection, vFpp)
GO(gst_query_set_uri_redirection_permanent, vFpi)
GO(gst_query_take, iFpp)
//DATAB(_gst_query_type, 
GO(gst_query_type_flags_get_type, LFv)
GO(gst_query_type_get_flags, uFu)
GO(gst_query_type_get_name, pFu)
GO(gst_query_type_get_type, LFv)
GO(gst_query_type_to_quark, uFu)
GO(gst_query_unref, vFp)
GO(gst_query_writable_structure, pFp)
//GO(gst_rank_get_type, 
//GO(gst_reference_timestamp_meta_api_get_type, 
//GO(gst_reference_timestamp_meta_get_info, 
GO(gst_registry_add_feature, iFpp)
GO(gst_registry_add_plugin, iFpp)
GO(gst_registry_check_feature_version, iFppuuu)
GOM(gst_registry_feature_filter, pFEppip)
GO(gst_registry_find_feature, pFppL)
GO(gst_registry_find_plugin, pFpp)
//GO(gst_registry_fork_is_enabled, 
GO(gst_registry_fork_set_enabled, vFi)
GO(gst_registry_get, pFv)
GO(gst_registry_get_feature_list, pFpL)
GO(gst_registry_get_feature_list_by_plugin, pFpp)
GO(gst_registry_get_feature_list_cookie, uFp)
GO(gst_registry_get_plugin_list, pFp)
//GO(gst_registry_get_type, 
GO(gst_registry_lookup, pFpp)
GO(gst_registry_lookup_feature, pFpp)
//GO(gst_registry_plugin_filter, 
GO(gst_registry_remove_feature, vFpp)
GO(gst_registry_remove_plugin, vFpp)
GO(gst_registry_scan_path, iFpp)
//GO(gst_resource_error_get_type, 
GO(gst_resource_error_quark, pFv)
GO(gst_sample_copy, pFp)
GO(gst_sample_get_buffer, pFp)
GO(gst_sample_get_buffer_list, pFp)
GO(gst_sample_get_caps, pFp)
GO(gst_sample_get_info, pFp)
GO(gst_sample_get_segment, pFp)
GO(gst_sample_get_type, LFv)
GO(gst_sample_new, pFpppp)
GO(gst_sample_ref, pFp)
//GO(gst_sample_set_buffer, 
GO(gst_sample_set_buffer_list, vFpp)
//GO(gst_sample_set_caps, 
GO(gst_sample_set_info, iFpp)
GO(gst_sample_set_segment, vFpp)
DATAB(_gst_sample_type, sizeof(long))
//GO(gst_sample_unref, 
GO(gst_scheduling_flags_get_type, LFv)
GO(gst_search_mode_get_type, LFv)
GO(gst_seek_flags_get_type, LFv)
GO(gst_seek_type_get_type, LFv)
GO(gst_segment_clip, iFpuLLpp)
GO(gst_segment_copy, pFp)
GO(gst_segment_copy_into, vFpp)
GO(gst_segment_do_seek, iFpduuuLuLp)
GO(gst_segment_flags_get_type, LFv)
GO(gst_segment_free, vFp)
GO(gst_segment_get_type, LFv)
GO(gst_segment_init, vFpu)
GO(gst_segment_is_equal, iFpp)
GO(gst_segment_new, pFv)
GO(gst_segment_offset_running_time, iFpul)
GO(gst_segment_position_from_running_time, LFpuL)
GO(gst_segment_position_from_running_time_full, iFpuLp)
GO(gst_segment_position_from_stream_time, UFpiU)
GO(gst_segment_position_from_stream_time_full, iFpiUp)
GO(gst_segment_set_running_time, iFpuL)
//GO(gst_segment_to_position, 
GO(gst_segment_to_running_time, UFpiU)
GO(gst_segment_to_running_time_full, iFpiUp)
GO(gst_segment_to_stream_time, UFpiU)
GO(gst_segment_to_stream_time_full, iFpiUp)
GO(gst_segtrap_is_enabled, iFv)
GO(gst_segtrap_set_enabled, vFi)
GO(gst_stack_trace_flags_get_type, LFv)
GO(gst_state_change_get_name, pFu)
GO(gst_state_change_get_type, LFv)
GO(gst_state_change_return_get_type, LFv)
GO(gst_state_get_type, LFv)
GO(gst_static_caps_cleanup, vFp)
GO(gst_static_caps_get, pFp)
GO(gst_static_caps_get_type, LFv)
GO(gst_static_pad_template_get, pFp)
GO(gst_static_pad_template_get_caps, pFp)
GO(gst_static_pad_template_get_type, LFv)
GO(gst_stream_collection_add_stream, iFpp)
GO(gst_stream_collection_get_size, uFp)
GO(gst_stream_collection_get_stream, pFpu)
//GO(gst_stream_collection_get_type, 
GO(gst_stream_collection_get_upstream_id, pFp)
GO(gst_stream_collection_new, pFp)
//GO(gst_stream_error_get_type, 
GO(gst_stream_error_quark, pFv)
//GO(gst_stream_flags_get_type, 
GO(gst_stream_get_caps, pFp)
GO(gst_stream_get_stream_flags, uFp)
GO(gst_stream_get_stream_id, pFp)
GO(gst_stream_get_stream_type, uFp)
GO(gst_stream_get_tags, pFp)
//GO(gst_stream_get_type, 
GO(gst_stream_new, pFppuu)
GO(gst_stream_set_caps, vFpp)
GO(gst_stream_set_stream_flags, vFpu)
GO(gst_stream_set_stream_type, vFpu)
GO(gst_stream_set_tags, vFpp)
//GO(gst_stream_status_type_get_type, 
GO(gst_stream_type_get_name, pFu)
//GO(gst_stream_type_get_type, 
GO(gst_structure_can_intersect, iFpp)
//GO(gst_structure_change_type_get_type, 
GO(gst_structure_copy, pFp)
GOM(gst_structure_filter_and_map_in_place, vFEppp)
GO(gst_structure_fixate, vFp)
GO(gst_structure_fixate_field, iFpp)
GO(gst_structure_fixate_field_boolean, iFppi)
GO(gst_structure_fixate_field_nearest_double, iFppd)
GO(gst_structure_fixate_field_nearest_fraction, iFppii)
GO(gst_structure_fixate_field_nearest_int, iFppi)
GO(gst_structure_fixate_field_string, iFppp)
GOM(gst_structure_foreach, iFEppp)
GO(gst_structure_free, vFp)
GO(gst_structure_from_string, pFpp)
GOM(gst_structure_get, iFEppV)
GO(gst_structure_get_array, iFppp)
GO(gst_structure_get_boolean, iFppp)
GO(gst_structure_get_clock_time, iFppp)
GO(gst_structure_get_date, iFppp)
GO(gst_structure_get_date_time, iFppp)
GO(gst_structure_get_double, iFppp)
GO(gst_structure_get_enum, iFppLp)
GO(gst_structure_get_field_type, LFpp)
GO(gst_structure_get_flagset, iFpppp)
GO(gst_structure_get_fraction, iFpppp)
GO(gst_structure_get_int, iFppp)
GO(gst_structure_get_int64, iFppp)
GO(gst_structure_get_list, iFppp)
GO(gst_structure_get_name, pFp)
GO(gst_structure_get_name_id, uFp)
GO(gst_structure_get_string, pFpp)
GO(gst_structure_get_type, LFv)
GO(gst_structure_get_uint, iFppp)
GO(gst_structure_get_uint64, iFppp)
GOM(gst_structure_get_valist, iFEppA)
GO(gst_structure_get_value, pFpp)
GO(gst_structure_has_field, iFpp)
GO(gst_structure_has_field_typed, iFppL)
GO(gst_structure_has_name, iFpp)
//GOM(gst_structure_id_get, iFEppV)
//GOM(gst_structure_id_get_valist, iFEppA)
GO(gst_structure_id_get_value, pFpu)
GO(gst_structure_id_has_field, iFpu)
GO(gst_structure_id_has_field_typed, iFpuL)
//GOM(gst_structure_id_set, iFEppV)
//GOM(gst_structure_id_set_valist, iFEppA)
GO(gst_structure_id_set_value, vFpup)
GO(gst_structure_id_take_value, vFpup)
GO(gst_structure_intersect, pFpp)
GO(gst_structure_is_equal, iFpp)
GO(gst_structure_is_subset, iFpp)
//GOM(gst_structure_map_in_place, iFEpBp)
GOM(gst_structure_new, pFEppV)
GO(gst_structure_new_empty, pFp)
GO(gst_structure_new_from_string, pFp)
//GOM(gst_structure_new_id, pFEppV)
GO(gst_structure_new_id_empty, pFu)
GOM(gst_structure_new_valist, pFppA)
GO(gst_structure_n_fields, iFp)
GO(gst_structure_nth_field_name, pFpu)
GO(gst_structure_remove_all_fields, vFp)
GO(gst_structure_remove_field, vFpp)
GOM(gst_structure_remove_fields, vFEppV)
GOM(gst_structure_remove_fields_valist, vFEppA)
GOM(gst_structure_set, vFEppV)
GO(gst_structure_set_array, vFppp)
GO(gst_structure_set_list, vFppp)
GO(gst_structure_set_name, vFpp)
GO(gst_structure_set_parent_refcount, iFpp)
GOM(gst_structure_set_valist, vFEppA)
GO(gst_structure_set_value, vFppp)
GO(gst_structure_take, iFpp)
GO(gst_structure_take_value, vFppp)
GO(gst_structure_to_string, pFp)
DATAB(_gst_structure_type, sizeof(void*))
//GO(gst_system_clock_get_type, 
GO(gst_system_clock_obtain, pFv)
GO(gst_system_clock_set_default, vFp)
//GO(gst_tag_exists, 
//GO(gst_tag_flag_get_type, 
GO(gst_tag_get_description, pFp)
GO(gst_tag_get_flag, uFp)
//GO(gst_tag_get_nick, 
GO(gst_tag_get_type, LFp)
//GO(gst_tag_is_fixed, 
GOM(gst_tag_list_add, vFEpipV)
GOM(gst_tag_list_add_valist, vFEpipA)
GOM(gst_tag_list_add_valist_values, vFEpipA)
GO(gst_tag_list_add_value, vFpipp)
GOM(gst_tag_list_add_values, vFEpipV)
GO(gst_tag_list_copy, pFp)
GO(gst_tag_list_copy_value, iFppp)
GOM(gst_tag_list_foreach, vFEppp)
GO(gst_tag_list_get_boolean, iFppp)
GO(gst_tag_list_get_boolean_index, iFppup)
GO(gst_tag_list_get_date, iFppp)
GO(gst_tag_list_get_date_index, iFppup)
GO(gst_tag_list_get_date_time, iFppp)
GO(gst_tag_list_get_date_time_index, iFppup)
GO(gst_tag_list_get_double, iFppp)
GO(gst_tag_list_get_double_index, iFppup)
GO(gst_tag_list_get_float, iFppp)
GO(gst_tag_list_get_float_index, iFppup)
GO(gst_tag_list_get_int, iFppp)
GO(gst_tag_list_get_int64, iFppp)
GO(gst_tag_list_get_int64_index, iFppup)
GO(gst_tag_list_get_int_index, iFppup)
GO(gst_tag_list_get_pointer, iFppp)
GO(gst_tag_list_get_pointer_index, iFppup)
GO(gst_tag_list_get_sample, iFppp)
GO(gst_tag_list_get_sample_index, iFppup)
GO(gst_tag_list_get_scope, iFp)
GO(gst_tag_list_get_string, iFppp)
GO(gst_tag_list_get_string_index, iFppup)
GO(gst_tag_list_get_tag_size, uFpp)
GO(gst_tag_list_get_type, LFv)
GO(gst_tag_list_get_uint, iFppp)
GO(gst_tag_list_get_uint64, iFppp)
GO(gst_tag_list_get_uint64_index, iFppup)
GO(gst_tag_list_get_uint_index, iFppup)
GO(gst_tag_list_get_value_index, pFppu)
GO(gst_tag_list_insert, vFppi)
GO(gst_tag_list_is_empty, iFp)
GO(gst_tag_list_is_equal, iFpp)
GO(gst_tag_list_merge, pFppi)
GOM(gst_tag_list_new, pFEpV)
GO(gst_tag_list_new_empty, pFv)
GO(gst_tag_list_new_from_string, pFp)
GOM(gst_tag_list_new_valist, pFA)
GO(gst_tag_list_n_tags, iFp)
GO(gst_tag_list_nth_tag_name, pFpu)
GO(gst_tag_list_peek_string_index, iFppup)
GO(gst_tag_list_ref, pFp)
GO(gst_tag_list_remove_tag, vFpp)
GO(gst_tag_list_replace, iFpp)
GO(gst_tag_list_set_scope, vFpi)
GO(gst_tag_list_take, iFpp)
GO(gst_tag_list_to_string, pFp)
DATAB(_gst_tag_list_type, sizeof(void*))
GO(gst_tag_list_unref, vFp)
GO(gst_tag_merge_mode_get_type, LFv)
//GO(gst_tag_merge_strings_with_comma, 
//GO(gst_tag_merge_use_first, 
//GO(gst_tag_register, 
//GO(gst_tag_register_static, 
//GO(gst_tag_scope_get_type, 
//GO(gst_tag_setter_add_tags, 
//GO(gst_tag_setter_add_tag_valist, 
//GO(gst_tag_setter_add_tag_valist_values, 
GO(gst_tag_setter_add_tag_value, vFpupp)
//GO(gst_tag_setter_add_tag_values, 
GO(gst_tag_setter_get_tag_list, pFp)
GO(gst_tag_setter_get_tag_merge_mode, uFp)
GO(gst_tag_setter_get_type, LFv)
GO(gst_tag_setter_merge_tags, vFppi)
GO(gst_tag_setter_reset_tags, vFp)
GO(gst_tag_setter_set_tag_merge_mode, vFpu)
//GO(gst_task_cleanup_all, 
//GO(gst_task_get_pool, 
//GO(gst_task_get_state, 
GO(gst_task_get_type, LFv)
GO(gst_task_join, iFp)
GOM(gst_task_new, pFEppp)
GO(gst_task_pause, iFp)
GO(gst_task_pool_cleanup, vFp)
GO(gst_task_pool_get_type, LFv)
//GO(gst_task_pool_join, 
//GO(gst_task_pool_new, 
GO(gst_task_pool_prepare, vFp)
//GO(gst_task_pool_push, 
//GO(gst_task_resume, 
//GO(gst_task_set_enter_callback, 
//GO(gst_task_set_leave_callback, 
GO(gst_task_set_lock, vFpp)
GO(gst_task_set_pool, vFpp)
//GO(gst_task_set_state, 
GO(gst_task_start, iFp)
GO(gst_task_state_get_type, LFv)
GO(gst_task_stop, iFp)
GO(gst_toc_append_entry, vFpp)
//GO(gst_toc_dump, 
GO(gst_toc_entry_append_sub_entry, vFpp)
GO(gst_toc_entry_get_entry_type, iFp)
GO(gst_toc_entry_get_loop, iFppp)
GO(gst_toc_entry_get_parent, pFp)
GO(gst_toc_entry_get_start_stop_times, iFppp)
GO(gst_toc_entry_get_sub_entries, pFp)
GO(gst_toc_entry_get_tags, pFp)
GO(gst_toc_entry_get_toc, pFp)
//GO(gst_toc_entry_get_type, 
GO(gst_toc_entry_get_uid, pFp)
//GO(gst_toc_entry_is_alternative, 
GO(gst_toc_entry_is_sequence, iFp)
GO(gst_toc_entry_merge_tags, vFppi)
GO(gst_toc_entry_new, pFip)
GO(gst_toc_entry_set_loop, vFpui)
GO(gst_toc_entry_set_start_stop_times, vFpll)
GO(gst_toc_entry_set_tags, vFpp)
//DATAB(_gst_toc_entry_type, 
GO(gst_toc_entry_type_get_nick, pFi)
//GO(gst_toc_entry_type_get_type, 
GO(gst_toc_find_entry, pFpp)
GO(gst_toc_get_entries, pFp)
GO(gst_toc_get_scope, uFp)
GO(gst_toc_get_tags, pFp)
//GO(gst_toc_get_type, 
//GO(gst_toc_loop_type_get_type, 
//GO(gst_toc_merge_tags, 
GO(gst_toc_new, pFu)
//GO(gst_toc_scope_get_type, 
//GO(gst_toc_set_tags, 
GO(gst_toc_setter_get_toc, pFp)
GO(gst_toc_setter_get_type, LFv)
GO(gst_toc_setter_reset, vFp)
GO(gst_toc_setter_set_toc, vFpp)
DATAB(_gst_toc_type, sizeof(void*))
//GO(gst_tracer_factory_get_list, 
GO(gst_tracer_factory_get_tracer_type, LFp)
//GO(gst_tracer_factory_get_type, 
//GO(gst_tracer_get_type, 
//GO(gst_tracer_record_get_type, 
//GO(gst_tracer_record_log, 
//GO(gst_tracer_record_new, 
GO(gst_tracer_register, iFppL)
//GO(gst_tracer_value_flags_get_type, 
//GO(gst_tracer_value_scope_get_type, 
//GO(gst_tracing_get_active_tracers, 
//GO(gst_tracing_register_hook, 
//GO(gst_type_find_factory_call_function, 
//GO(gst_type_find_factory_get_caps, 
GO(gst_type_find_factory_get_extensions, pFp)
GO(gst_type_find_factory_get_list, pFv)
//GO(gst_type_find_factory_get_type, 
GO(gst_type_find_factory_has_function, iFp)
GO(gst_type_find_get_length, UFp)
//GO(gst_type_find_get_type, 
GO(gst_type_find_peek, pFpUu)
//GO(gst_type_find_probability_get_type, 
GOM(gst_type_find_register, iFEppuppppp)
GO(gst_type_find_suggest, vFpup)
//GO(gst_type_find_suggest_simple, 
GO(gst_type_is_plugin_api, iFLp)
GO(gst_type_mark_as_plugin_api, vFLu)
GO(gst_update_registry, iFv)
//GO(gst_uri_append_path, 
//GO(gst_uri_append_path_segment, 
//GO(gst_uri_construct, 
GO(gst_uri_copy, pFp)
GO(gst_uri_equal, iFpp)
//GO(gst_uri_error_get_type, 
GO(gst_uri_error_quark, pFv)
//GO(gst_uri_from_string, 
GO(gst_uri_from_string_escaped, pFp)
GO(gst_uri_from_string_with_base, pFpp)
GO(gst_uri_get_fragment, pFp)
//GO(gst_uri_get_host, 
GO(gst_uri_get_location, pFp)
GO(gst_uri_get_media_fragment_table, pFp)
//GO(gst_uri_get_path, 
//GO(gst_uri_get_path_segments, 
//GO(gst_uri_get_path_string, 
GO(gst_uri_get_port, uFp)
//GO(gst_uri_get_protocol, 
GO(gst_uri_get_query_keys, pFp)
GO(gst_uri_get_query_string, pFp)
//GO(gst_uri_get_query_table, 
GO(gst_uri_get_query_value, pFpp)
//GO(gst_uri_get_scheme, 
//GO(gst_uri_get_type, 
//GO(gst_uri_get_userinfo, 
GO(gst_uri_handler_get_protocols, pFp)
GO(gst_uri_handler_get_type, LFv)
GO(gst_uri_handler_get_uri, pFp)
GO(gst_uri_handler_get_uri_type, uFp)
GO(gst_uri_handler_set_uri, iFppp)
GO(gst_uri_has_protocol, iFpp)
GO(gst_uri_is_normalized, iFp)
GO(gst_uri_is_valid, iFp)
//GO(gst_uri_is_writable, 
GO(gst_uri_join, pFpp)
GO(gst_uri_join_strings, pFpp)
//GO(gst_uri_make_writable, 
GO(gst_uri_new, pFpppuppp)
GO(gst_uri_new_with_base, pFppppuppp)
GO(gst_uri_normalize, iFp)
GO(gst_uri_protocol_is_supported, iFup)
//GO(gst_uri_protocol_is_valid, 
GO(gst_uri_query_has_key, iFpp)
GO(gst_uri_ref, pFp)
//GO(gst_uri_remove_query_key, 
GO(gst_uri_set_fragment, iFpp)
//GO(gst_uri_set_host, 
//GO(gst_uri_set_path, 
GO(gst_uri_set_path_segments, iFpp)
//GO(gst_uri_set_path_string, 
GO(gst_uri_set_port, iFpu)
//GO(gst_uri_set_query_string, 
GO(gst_uri_set_query_table, iFpp)
GO(gst_uri_set_query_value, iFppp)
//GO(gst_uri_set_scheme, 
//GO(gst_uri_set_userinfo, 
//GO(gst_uri_to_string, 
//GO(gst_uri_type_get_type, 
//GO(gst_uri_unref, 
GOM(gst_util_array_binary_search, pFpuLpipp)
GO(gst_util_double_to_fraction, vFdpp)
//GO(gst_util_dump_buffer, 
GO(gst_util_dump_mem, vFpu)
GO(gst_util_fraction_add, iFiiiipp)
GO(gst_util_fraction_compare, iFiiii)
GO(gst_util_fraction_multiply, iFiiiipp)
GO(gst_util_fraction_to_double, vFiip)
GO(gst_util_gdouble_to_guint64, LFd)
GO(gst_util_get_object_array, iFppp)
GO(gst_util_get_timestamp, LFv)
GO(gst_util_greatest_common_divisor, iFii)
GO(gst_util_greatest_common_divisor_int64, lFll)
GO(gst_util_group_id_next, uFv)
GO(gst_util_guint64_to_gdouble, dFL)
GO(gst_util_seqnum_compare, iFuu)
GO(gst_util_seqnum_next, uFv)
GO(gst_util_set_object_arg, vFppp)
GO(gst_util_set_object_array, iFppp)
GO(gst_util_set_value_from_string, vFpp)
GO(gst_util_uint64_scale, UFUUU)
GO(gst_util_uint64_scale_ceil, LFLLL)
GO(gst_util_uint64_scale_int, UFUii)
GO(gst_util_uint64_scale_int_ceil, LFLii)
GO(gst_util_uint64_scale_int_round, UFUii)
GO(gst_util_uint64_scale_round, UFUUU)
GO(gst_value_array_append_and_take_value, vFpp)
GO(gst_value_array_append_value, vFpp)
GO(gst_value_array_get_size, iFp)
//GO(gst_value_array_get_type, 
GO(gst_value_array_get_value, pFpu)
GO(gst_value_array_init, pFpu)
GO(gst_value_array_prepend_value, vFpp)
DATAB(_gst_value_array_type, sizeof(void*))
//GO(gst_value_can_compare, 
//GO(gst_value_can_intersect, 
GO(gst_value_can_subtract, iFpp)
//GO(gst_value_can_union, 
GO(gst_value_compare, iFpp)
GO(gst_value_deserialize, iFpp)
GO(gst_value_fixate, iFpp)
//GO(gst_value_fraction_multiply, 
//GO(gst_value_fraction_subtract, 
GO(gst_value_get_bitmask, LFp)
GO(gst_value_get_caps, pFp)
GO(gst_value_get_caps_features, pFp)
GO(gst_value_get_double_range_max, dFp)
//GO(gst_value_get_double_range_min, 
GO(gst_value_get_flagset_flags, uFp)
GO(gst_value_get_flagset_mask, uFp)
GO(gst_value_get_fraction_denominator, iFp)
GO(gst_value_get_fraction_numerator, iFp)
GO(gst_value_get_fraction_range_max, pFp)
GO(gst_value_get_fraction_range_min, pFp)
//GO(gst_value_get_int64_range_max, 
//GO(gst_value_get_int64_range_min, 
GO(gst_value_get_int64_range_step, lFp)
GO(gst_value_get_int_range_max, iFp)
GO(gst_value_get_int_range_min, iFp)
//GO(gst_value_get_int_range_step, 
GO(gst_value_get_structure, pFp)
GO(gst_value_init_and_copy, vFpp)
//GO(gst_value_intersect, 
GO(gst_value_is_fixed, iFp)
//GO(gst_value_is_subset, 
GO(gst_value_list_append_and_take_value, vFpp)
GO(gst_value_list_append_value, vFpp)
//GO(gst_value_list_concat, 
GO(gst_value_list_get_size, uFp)
GO(gst_value_list_get_type, LFv)
GO(gst_value_list_get_value, pFpu)
//GO(gst_value_list_init, 
GO(gst_value_list_merge, vFppp)
//GO(gst_value_list_prepend_value, 
DATAB(_gst_value_list_type, sizeof(void*))
//GO(gst_value_register, 
GO(gst_value_serialize, pFp)
GO(gst_value_set_bitmask, vFpL)
GO(gst_value_set_caps, vFpp)
GO(gst_value_set_caps_features, vFpp)
GO(gst_value_set_double_range, vFpdd)
GO(gst_value_set_flagset, vFpuu)
GO(gst_value_set_fraction, vFpii)
GO(gst_value_set_fraction_range, vFppp)
GO(gst_value_set_fraction_range_full, vFpiiii)
GO(gst_value_set_int64_range, vFpll)
GO(gst_value_set_int64_range_step, vFplll)
GO(gst_value_set_int_range, vFpii)
GO(gst_value_set_int_range_step, vFpiii)
GO(gst_value_set_structure, vFpp)
GO(gst_value_subtract, iFppp)
//GO(gst_value_union, 
GO(gst_version, vFpppp)
GO(gst_version_string, pFv)

GO(dummy_iFpipLpp, iFpipLpp)    // for gtkclass