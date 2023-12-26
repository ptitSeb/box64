#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//GO(xcb_dri3_buffer_from_pixmap, 
//GO(xcb_dri3_buffer_from_pixmap_reply, 
//GO(xcb_dri3_buffer_from_pixmap_reply_fds, 
//GO(xcb_dri3_buffer_from_pixmap_unchecked, 
//GO(xcb_dri3_buffers_from_pixmap, 
//GO(xcb_dri3_buffers_from_pixmap_buffers, 
//GO(xcb_dri3_buffers_from_pixmap_buffers_end, 
//GO(xcb_dri3_buffers_from_pixmap_buffers_length, 
//GO(xcb_dri3_buffers_from_pixmap_offsets, 
//GO(xcb_dri3_buffers_from_pixmap_offsets_end, 
//GO(xcb_dri3_buffers_from_pixmap_offsets_length, 
//GO(xcb_dri3_buffers_from_pixmap_reply, 
//GO(xcb_dri3_buffers_from_pixmap_reply_fds, 
//GO(xcb_dri3_buffers_from_pixmap_sizeof, 
//GO(xcb_dri3_buffers_from_pixmap_strides, 
//GO(xcb_dri3_buffers_from_pixmap_strides_end, 
//GO(xcb_dri3_buffers_from_pixmap_strides_length, 
//GO(xcb_dri3_buffers_from_pixmap_unchecked, 
//GO(xcb_dri3_fd_from_fence, 
//GO(xcb_dri3_fd_from_fence_reply, 
//GO(xcb_dri3_fd_from_fence_reply_fds, 
//GO(xcb_dri3_fd_from_fence_unchecked, 
//GO(xcb_dri3_fence_from_fd, 
//GO(xcb_dri3_fence_from_fd_checked, 
//GO(xcb_dri3_get_supported_modifiers, 
//GO(xcb_dri3_get_supported_modifiers_reply, 
//GO(xcb_dri3_get_supported_modifiers_screen_modifiers, 
//GO(xcb_dri3_get_supported_modifiers_screen_modifiers_end, 
//GO(xcb_dri3_get_supported_modifiers_screen_modifiers_length, 
//GO(xcb_dri3_get_supported_modifiers_sizeof, 
//GO(xcb_dri3_get_supported_modifiers_unchecked, 
//GO(xcb_dri3_get_supported_modifiers_window_modifiers, 
//GO(xcb_dri3_get_supported_modifiers_window_modifiers_end, 
//GO(xcb_dri3_get_supported_modifiers_window_modifiers_length, 
DATA(xcb_dri3_id, sizeof(void*)+sizeof(int))
GO(xcb_dri3_open, pFbuu)
GO(xcb_dri3_open_reply, pFbup)
GO(xcb_dri3_open_reply_fds, pFbp)
//GO(xcb_dri3_open_unchecked, 
GO(xcb_dri3_pixmap_from_buffer, pFbuuuWWWCCi)
GO(xcb_dri3_pixmap_from_buffer_checked, pFbuuuWWWCCi)
//GO(xcb_dri3_pixmap_from_buffers, 
//GO(xcb_dri3_pixmap_from_buffers_checked, 
GO(xcb_dri3_query_version, pFbuu)
GO(xcb_dri3_query_version_reply, pFbup)
GO(xcb_dri3_query_version_unchecked, pFpuu)
