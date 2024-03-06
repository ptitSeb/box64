#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//GO(xcb_randr_add_output_mode, 
//GO(xcb_randr_add_output_mode_checked, 
//GO(xcb_randr_change_output_property, 
//GO(xcb_randr_change_output_property_checked, 
//GO(xcb_randr_change_output_property_data, 
//GO(xcb_randr_change_output_property_data_end, 
//GO(xcb_randr_change_output_property_data_length, 
//GO(xcb_randr_change_output_property_sizeof, 
//GO(xcb_randr_change_provider_property, 
//GO(xcb_randr_change_provider_property_checked, 
//GO(xcb_randr_change_provider_property_data, 
//GO(xcb_randr_change_provider_property_data_end, 
//GO(xcb_randr_change_provider_property_data_length, 
//GO(xcb_randr_change_provider_property_sizeof, 
//GO(xcb_randr_configure_output_property, 
//GO(xcb_randr_configure_output_property_checked, 
//GO(xcb_randr_configure_output_property_sizeof, 
//GO(xcb_randr_configure_output_property_values, 
//GO(xcb_randr_configure_output_property_values_end, 
//GO(xcb_randr_configure_output_property_values_length, 
//GO(xcb_randr_configure_provider_property, 
//GO(xcb_randr_configure_provider_property_checked, 
//GO(xcb_randr_configure_provider_property_sizeof, 
//GO(xcb_randr_configure_provider_property_values, 
//GO(xcb_randr_configure_provider_property_values_end, 
//GO(xcb_randr_configure_provider_property_values_length, 
//GO(xcb_randr_create_mode, 
//GO(xcb_randr_create_mode_reply, 
//GO(xcb_randr_create_mode_sizeof, 
//GO(xcb_randr_create_mode_unchecked, 
//GO(xcb_randr_crtc_change_end, 
//GO(xcb_randr_crtc_change_next, 
//GO(xcb_randr_crtc_end, 
//GO(xcb_randr_crtc_next, 
//GO(xcb_randr_delete_monitor, 
//GO(xcb_randr_delete_monitor_checked, 
//GO(xcb_randr_delete_output_mode, 
//GO(xcb_randr_delete_output_mode_checked, 
//GO(xcb_randr_delete_output_property, 
//GO(xcb_randr_delete_output_property_checked, 
//GO(xcb_randr_delete_provider_property, 
//GO(xcb_randr_delete_provider_property_checked, 
//GO(xcb_randr_destroy_mode, 
//GO(xcb_randr_destroy_mode_checked, 
//GO(xcb_randr_get_crtc_gamma, 
//GO(xcb_randr_get_crtc_gamma_blue, 
//GO(xcb_randr_get_crtc_gamma_blue_end, 
//GO(xcb_randr_get_crtc_gamma_blue_length, 
//GO(xcb_randr_get_crtc_gamma_green, 
//GO(xcb_randr_get_crtc_gamma_green_end, 
//GO(xcb_randr_get_crtc_gamma_green_length, 
//GO(xcb_randr_get_crtc_gamma_red, 
//GO(xcb_randr_get_crtc_gamma_red_end, 
//GO(xcb_randr_get_crtc_gamma_red_length, 
//GO(xcb_randr_get_crtc_gamma_reply, 
//GO(xcb_randr_get_crtc_gamma_size, 
//GO(xcb_randr_get_crtc_gamma_sizeof, 
//GO(xcb_randr_get_crtc_gamma_size_reply, 
//GO(xcb_randr_get_crtc_gamma_size_unchecked, 
//GO(xcb_randr_get_crtc_gamma_unchecked, 
GO(xcb_randr_get_crtc_info, pFbppu)
//GO(xcb_randr_get_crtc_info_outputs, 
//GO(xcb_randr_get_crtc_info_outputs_end, 
//GO(xcb_randr_get_crtc_info_outputs_length, 
//GO(xcb_randr_get_crtc_info_possible, 
//GO(xcb_randr_get_crtc_info_possible_end, 
//GO(xcb_randr_get_crtc_info_possible_length, 
GO(xcb_randr_get_crtc_info_reply, pFbup)
//GO(xcb_randr_get_crtc_info_sizeof, 
GO(xcb_randr_get_crtc_info_unchecked, pFbppu)
//GO(xcb_randr_get_crtc_transform, 
//GO(xcb_randr_get_crtc_transform_current_filter_name, 
//GO(xcb_randr_get_crtc_transform_current_filter_name_end, 
//GO(xcb_randr_get_crtc_transform_current_filter_name_length, 
//GO(xcb_randr_get_crtc_transform_current_params, 
//GO(xcb_randr_get_crtc_transform_current_params_end, 
//GO(xcb_randr_get_crtc_transform_current_params_length, 
//GO(xcb_randr_get_crtc_transform_pending_filter_name, 
//GO(xcb_randr_get_crtc_transform_pending_filter_name_end, 
//GO(xcb_randr_get_crtc_transform_pending_filter_name_length, 
//GO(xcb_randr_get_crtc_transform_pending_params, 
//GO(xcb_randr_get_crtc_transform_pending_params_end, 
//GO(xcb_randr_get_crtc_transform_pending_params_length, 
//GO(xcb_randr_get_crtc_transform_reply, 
//GO(xcb_randr_get_crtc_transform_sizeof, 
//GO(xcb_randr_get_crtc_transform_unchecked, 
GO(xcb_randr_get_monitors, uFbuC)
GO(xcb_randr_get_monitors_monitors_iterator, HFp)
GO(xcb_randr_get_monitors_monitors_length, iFp)
GO(xcb_randr_get_monitors_reply, pFbup)
GO(xcb_randr_get_monitors_sizeof, iFp)
GO(xcb_randr_get_monitors_unchecked, uFbuC)
GO(xcb_randr_get_output_info, pFbppu)
//GO(xcb_randr_get_output_info_clones, 
//GO(xcb_randr_get_output_info_clones_end, 
//GO(xcb_randr_get_output_info_clones_length, 
//GO(xcb_randr_get_output_info_crtcs, 
//GO(xcb_randr_get_output_info_crtcs_end, 
//GO(xcb_randr_get_output_info_crtcs_length, 
GO(xcb_randr_get_output_info_modes, pFp)
//GO(xcb_randr_get_output_info_modes_end, 
GO(xcb_randr_get_output_info_modes_length, iFp)
GO(xcb_randr_get_output_info_name, pFp)
GO(xcb_randr_get_output_info_name_end, pFp)
GO(xcb_randr_get_output_info_name_length, iFp)
GO(xcb_randr_get_output_info_reply, pFbup)
//GO(xcb_randr_get_output_info_sizeof, 
GO(xcb_randr_get_output_info_unchecked, pFbppu)
GO(xcb_randr_get_output_primary, pFbpu)
GO(xcb_randr_get_output_primary_reply, pFbup)
GO(xcb_randr_get_output_primary_unchecked, pFbpu)
GO(xcb_randr_get_output_property, pFbppppuuCC)
GO(xcb_randr_get_output_property_data, pFp)
GO(xcb_randr_get_output_property_data_end, pFpp)
GO(xcb_randr_get_output_property_data_length, iFp)
GO(xcb_randr_get_output_property_reply, pFbup)
//GO(xcb_randr_get_output_property_sizeof, 
//GO(xcb_randr_get_output_property_unchecked, 
//GO(xcb_randr_get_panning, 
//GO(xcb_randr_get_panning_reply, 
//GO(xcb_randr_get_panning_unchecked, 
//GO(xcb_randr_get_provider_info, 
//GO(xcb_randr_get_provider_info_associated_capability, 
//GO(xcb_randr_get_provider_info_associated_capability_end, 
//GO(xcb_randr_get_provider_info_associated_capability_length, 
//GO(xcb_randr_get_provider_info_associated_providers, 
//GO(xcb_randr_get_provider_info_associated_providers_end, 
//GO(xcb_randr_get_provider_info_associated_providers_length, 
//GO(xcb_randr_get_provider_info_crtcs, 
//GO(xcb_randr_get_provider_info_crtcs_end, 
//GO(xcb_randr_get_provider_info_crtcs_length, 
//GO(xcb_randr_get_provider_info_name, 
//GO(xcb_randr_get_provider_info_name_end, 
//GO(xcb_randr_get_provider_info_name_length, 
//GO(xcb_randr_get_provider_info_outputs, 
//GO(xcb_randr_get_provider_info_outputs_end, 
//GO(xcb_randr_get_provider_info_outputs_length, 
//GO(xcb_randr_get_provider_info_reply, 
//GO(xcb_randr_get_provider_info_sizeof, 
//GO(xcb_randr_get_provider_info_unchecked, 
//GO(xcb_randr_get_provider_property, 
//GO(xcb_randr_get_provider_property_data, 
//GO(xcb_randr_get_provider_property_data_end, 
//GO(xcb_randr_get_provider_property_data_length, 
//GO(xcb_randr_get_provider_property_reply, 
//GO(xcb_randr_get_provider_property_sizeof, 
//GO(xcb_randr_get_provider_property_unchecked, 
//GO(xcb_randr_get_providers, 
//GO(xcb_randr_get_providers_providers, 
//GO(xcb_randr_get_providers_providers_end, 
//GO(xcb_randr_get_providers_providers_length, 
//GO(xcb_randr_get_providers_reply, 
//GO(xcb_randr_get_providers_sizeof, 
//GO(xcb_randr_get_providers_unchecked, 
GO(xcb_randr_get_screen_info, pFbpu)
GO(xcb_randr_get_screen_info_rates_iterator, HFp)
GO(xcb_randr_get_screen_info_rates_length, iFp)
GO(xcb_randr_get_screen_info_reply, pFbup)
//GO(xcb_randr_get_screen_info_sizeof, 
GO(xcb_randr_get_screen_info_sizes, pFp)
GO(xcb_randr_get_screen_info_sizes_iterator, HFp)
GO(xcb_randr_get_screen_info_sizes_length, iFp)
GO(xcb_randr_get_screen_info_unchecked, pFbpu)
GO(xcb_randr_get_screen_resources, pFbpu)
//GO(xcb_randr_get_screen_resources_crtcs, 
//GO(xcb_randr_get_screen_resources_crtcs_end, 
//GO(xcb_randr_get_screen_resources_crtcs_length, 
GO(xcb_randr_get_screen_resources_current, pFbpu)
//GO(xcb_randr_get_screen_resources_current_crtcs, 
//GO(xcb_randr_get_screen_resources_current_crtcs_end, 
//GO(xcb_randr_get_screen_resources_current_crtcs_length, 
//GO(xcb_randr_get_screen_resources_current_modes, 
GO(xcb_randr_get_screen_resources_current_modes_iterator, HFp)
//GO(xcb_randr_get_screen_resources_current_modes_length, 
//GO(xcb_randr_get_screen_resources_current_names, 
//GO(xcb_randr_get_screen_resources_current_names_end, 
//GO(xcb_randr_get_screen_resources_current_names_length, 
GO(xcb_randr_get_screen_resources_current_outputs, pFp)
GO(xcb_randr_get_screen_resources_current_outputs_end, HFp)
GO(xcb_randr_get_screen_resources_current_outputs_length, iFp)
GO(xcb_randr_get_screen_resources_current_reply, pFbup)
//GO(xcb_randr_get_screen_resources_current_sizeof, 
GO(xcb_randr_get_screen_resources_current_unchecked, pFbpu)
GO(xcb_randr_get_screen_resources_modes, pFp)
//GO(xcb_randr_get_screen_resources_modes_iterator, 
GO(xcb_randr_get_screen_resources_modes_length, iFp)
//GO(xcb_randr_get_screen_resources_names, 
//GO(xcb_randr_get_screen_resources_names_end, 
//GO(xcb_randr_get_screen_resources_names_length, 
GO(xcb_randr_get_screen_resources_outputs, pFppu)
//GO(xcb_randr_get_screen_resources_outputs_end, 
GO(xcb_randr_get_screen_resources_outputs_length, iFp)
GO(xcb_randr_get_screen_resources_reply, pFbup)
//GO(xcb_randr_get_screen_resources_sizeof, 
GO(xcb_randr_get_screen_resources_unchecked, pFbpu)
//GO(xcb_randr_get_screen_size_range, 
//GO(xcb_randr_get_screen_size_range_reply, 
//GO(xcb_randr_get_screen_size_range_unchecked, 
DATA(xcb_randr_id, 8)
//GO(xcb_randr_list_output_properties, 
//GO(xcb_randr_list_output_properties_atoms, 
//GO(xcb_randr_list_output_properties_atoms_end, 
//GO(xcb_randr_list_output_properties_atoms_length, 
//GO(xcb_randr_list_output_properties_reply, 
//GO(xcb_randr_list_output_properties_sizeof, 
//GO(xcb_randr_list_output_properties_unchecked, 
//GO(xcb_randr_list_provider_properties, 
//GO(xcb_randr_list_provider_properties_atoms, 
//GO(xcb_randr_list_provider_properties_atoms_end, 
//GO(xcb_randr_list_provider_properties_atoms_length, 
//GO(xcb_randr_list_provider_properties_reply, 
//GO(xcb_randr_list_provider_properties_sizeof, 
//GO(xcb_randr_list_provider_properties_unchecked, 
//GO(xcb_randr_mode_end, 
//GO(xcb_randr_mode_info_end, 
GO(xcb_randr_mode_info_next, vFp)
//GO(xcb_randr_mode_next, 
GO(xcb_randr_monitor_info_end, pFpp)
GO(xcb_randr_monitor_info_next, vFp)
GO(xcb_randr_monitor_info_outputs, pFp)
GO(xcb_randr_monitor_info_outputs_end, pFpp)
GO(xcb_randr_monitor_info_outputs_length, iFp)
GO(xcb_randr_monitor_info_sizeof, iFp)
//GO(xcb_randr_notify_data_end, 
//GO(xcb_randr_notify_data_next, 
//GO(xcb_randr_output_change_end, 
//GO(xcb_randr_output_change_next, 
//GO(xcb_randr_output_end, 
//GO(xcb_randr_output_next, 
//GO(xcb_randr_output_property_end, 
//GO(xcb_randr_output_property_next, 
//GO(xcb_randr_provider_change_end, 
//GO(xcb_randr_provider_change_next, 
//GO(xcb_randr_provider_end, 
//GO(xcb_randr_provider_next, 
//GO(xcb_randr_provider_property_end, 
//GO(xcb_randr_provider_property_next, 
//GO(xcb_randr_query_output_property, 
//GO(xcb_randr_query_output_property_reply, 
//GO(xcb_randr_query_output_property_sizeof, 
//GO(xcb_randr_query_output_property_unchecked, 
//GO(xcb_randr_query_output_property_valid_values, 
//GO(xcb_randr_query_output_property_valid_values_end, 
//GO(xcb_randr_query_output_property_valid_values_length, 
//GO(xcb_randr_query_provider_property, 
//GO(xcb_randr_query_provider_property_reply, 
//GO(xcb_randr_query_provider_property_sizeof, 
//GO(xcb_randr_query_provider_property_unchecked, 
//GO(xcb_randr_query_provider_property_valid_values, 
//GO(xcb_randr_query_provider_property_valid_values_end, 
//GO(xcb_randr_query_provider_property_valid_values_length, 
GO(xcb_randr_query_version, uFbuu)
GO(xcb_randr_query_version_reply, pFbup)
GO(xcb_randr_query_version_unchecked, uFbuu)
//GO(xcb_randr_refresh_rates_end, 
//GO(xcb_randr_refresh_rates_next, 
//GO(xcb_randr_refresh_rates_rates, 
//GO(xcb_randr_refresh_rates_rates_end, 
//GO(xcb_randr_refresh_rates_rates_length, 
//GO(xcb_randr_refresh_rates_sizeof, 
//GO(xcb_randr_resource_change_end, 
//GO(xcb_randr_resource_change_next, 
//GO(xcb_randr_screen_size_end, 
//GO(xcb_randr_screen_size_next, 
GO(xcb_randr_select_input, uFbuW)
GO(xcb_randr_select_input_checked, uFbuW)
//GO(xcb_randr_set_crtc_config, 
//GO(xcb_randr_set_crtc_config_reply, 
//GO(xcb_randr_set_crtc_config_sizeof, 
//GO(xcb_randr_set_crtc_config_unchecked, 
//GO(xcb_randr_set_crtc_gamma, 
//GO(xcb_randr_set_crtc_gamma_blue, 
//GO(xcb_randr_set_crtc_gamma_blue_end, 
//GO(xcb_randr_set_crtc_gamma_blue_length, 
//GO(xcb_randr_set_crtc_gamma_checked, 
//GO(xcb_randr_set_crtc_gamma_green, 
//GO(xcb_randr_set_crtc_gamma_green_end, 
//GO(xcb_randr_set_crtc_gamma_green_length, 
//GO(xcb_randr_set_crtc_gamma_red, 
//GO(xcb_randr_set_crtc_gamma_red_end, 
//GO(xcb_randr_set_crtc_gamma_red_length, 
//GO(xcb_randr_set_crtc_gamma_sizeof, 
//GO(xcb_randr_set_crtc_transform, 
//GO(xcb_randr_set_crtc_transform_checked, 
//GO(xcb_randr_set_crtc_transform_filter_name, 
//GO(xcb_randr_set_crtc_transform_filter_name_end, 
//GO(xcb_randr_set_crtc_transform_filter_name_length, 
//GO(xcb_randr_set_crtc_transform_filter_params, 
//GO(xcb_randr_set_crtc_transform_filter_params_end, 
//GO(xcb_randr_set_crtc_transform_filter_params_length, 
//GO(xcb_randr_set_crtc_transform_sizeof, 
//GO(xcb_randr_set_monitor, 
//GO(xcb_randr_set_monitor_checked, 
//GO(xcb_randr_set_monitor_monitorinfo, 
//GO(xcb_randr_set_monitor_sizeof, 
//GO(xcb_randr_set_output_primary, 
//GO(xcb_randr_set_output_primary_checked, 
//GO(xcb_randr_set_panning, 
//GO(xcb_randr_set_panning_reply, 
//GO(xcb_randr_set_panning_unchecked, 
//GO(xcb_randr_set_provider_offload_sink, 
//GO(xcb_randr_set_provider_offload_sink_checked, 
//GO(xcb_randr_set_provider_output_source, 
//GO(xcb_randr_set_provider_output_source_checked, 
//GO(xcb_randr_set_screen_config, 
//GO(xcb_randr_set_screen_config_reply, 
//GO(xcb_randr_set_screen_config_unchecked, 
//GO(xcb_randr_set_screen_size, 
//GO(xcb_randr_set_screen_size_checked, 
