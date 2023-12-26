#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//GO(xcb_dri2_attach_format_end, 
//GO(xcb_dri2_attach_format_next, 
GO(xcb_dri2_authenticate, pFbuu)
GO(xcb_dri2_authenticate_reply, pFbup)
//GO(xcb_dri2_authenticate_unchecked, 
GO(xcb_dri2_connect, pFbuu)
//GO(xcb_dri2_connect_alignment_pad, 
//GO(xcb_dri2_connect_alignment_pad_end, 
//GO(xcb_dri2_connect_alignment_pad_length, 
GO(xcb_dri2_connect_device_name, pFp)
//GO(xcb_dri2_connect_device_name_end, 
GO(xcb_dri2_connect_device_name_length, iFp)
//GO(xcb_dri2_connect_driver_name, 
//GO(xcb_dri2_connect_driver_name_end, 
//GO(xcb_dri2_connect_driver_name_length, 
GO(xcb_dri2_connect_reply, pFbup)
//GO(xcb_dri2_connect_sizeof, 
//GO(xcb_dri2_connect_unchecked, 
//GO(xcb_dri2_copy_region, 
//GO(xcb_dri2_copy_region_reply, 
//GO(xcb_dri2_copy_region_unchecked, 
//GO(xcb_dri2_create_drawable, 
//GO(xcb_dri2_create_drawable_checked, 
//GO(xcb_dri2_destroy_drawable, 
//GO(xcb_dri2_destroy_drawable_checked, 
//GO(xcb_dri2_dri2_buffer_end, 
//GO(xcb_dri2_dri2_buffer_next, 
//GO(xcb_dri2_get_buffers, 
//GO(xcb_dri2_get_buffers_buffers, 
//GO(xcb_dri2_get_buffers_buffers_iterator, 
//GO(xcb_dri2_get_buffers_buffers_length, 
//GO(xcb_dri2_get_buffers_reply, 
//GO(xcb_dri2_get_buffers_sizeof, 
//GO(xcb_dri2_get_buffers_unchecked, 
//GO(xcb_dri2_get_buffers_with_format, 
//GO(xcb_dri2_get_buffers_with_format_buffers, 
//GO(xcb_dri2_get_buffers_with_format_buffers_iterator, 
//GO(xcb_dri2_get_buffers_with_format_buffers_length, 
//GO(xcb_dri2_get_buffers_with_format_reply, 
//GO(xcb_dri2_get_buffers_with_format_sizeof, 
//GO(xcb_dri2_get_buffers_with_format_unchecked, 
//GO(xcb_dri2_get_msc, 
//GO(xcb_dri2_get_msc_reply, 
//GO(xcb_dri2_get_msc_unchecked, 
//GO(xcb_dri2_get_param, 
//GO(xcb_dri2_get_param_reply, 
//GO(xcb_dri2_get_param_unchecked, 
DATA(xcb_dri2_id, sizeof(void*)+sizeof(int))
GO(xcb_dri2_query_version, pFbuu)
GO(xcb_dri2_query_version_reply, pFbup)
//GO(xcb_dri2_query_version_unchecked, 
//GO(xcb_dri2_swap_buffers, 
//GO(xcb_dri2_swap_buffers_reply, 
//GO(xcb_dri2_swap_buffers_unchecked, 
//GO(xcb_dri2_swap_interval, 
//GO(xcb_dri2_swap_interval_checked, 
//GO(xcb_dri2_wait_msc, 
//GO(xcb_dri2_wait_msc_reply, 
//GO(xcb_dri2_wait_msc_unchecked, 
//GO(xcb_dri2_wait_sbc, 
//GO(xcb_dri2_wait_sbc_reply, 
//GO(xcb_dri2_wait_sbc_unchecked, 
