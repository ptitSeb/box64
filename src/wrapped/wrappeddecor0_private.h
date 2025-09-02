#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GO(libdecor_configuration_get_content_size, iFpppp)
GO(libdecor_configuration_get_window_state, iFpp)
GO(libdecor_decorate, pFpppp)
GO(libdecor_dispatch, iFpi)
GO(libdecor_frame_close, vFp)
GO(libdecor_frame_commit, vFppp)
//GO(libdecor_frame_dismiss_popup,
//GO(libdecor_frame_get_capabilities,
//GO(libdecor_frame_get_content_height,
//GO(libdecor_frame_get_content_width,
GO(libdecor_frame_get_max_content_size, vFppp)
GO(libdecor_frame_get_min_content_size, vFppp)
GO(libdecor_frame_get_title, pFp)
//GO(libdecor_frame_get_window_state,
//GO(libdecor_frame_get_wl_surface,
GO(libdecor_frame_get_xdg_surface, pFp)
GO(libdecor_frame_get_xdg_toplevel, pFp)
GO(libdecor_frame_has_capability, iFpu)
GO(libdecor_frame_is_floating, iFp)
GO(libdecor_frame_is_visible, iFp)
GO(libdecor_frame_map, vFp)
GO(libdecor_frame_move, vFppu)
GO(libdecor_frame_popup_grab, vFpp)
GO(libdecor_frame_popup_ungrab, vFpp)
GO(libdecor_frame_ref, vFp)
GO(libdecor_frame_resize, vFppuu)
GO(libdecor_frame_set_app_id, vFpp)
GO(libdecor_frame_set_capabilities, vFpu)
GO(libdecor_frame_set_fullscreen, vFpp)
GO(libdecor_frame_set_max_content_size, vFpii)
GO(libdecor_frame_set_maximized, vFp)
GO(libdecor_frame_set_min_content_size, vFpii)
GO(libdecor_frame_set_minimized, vFp)
GO(libdecor_frame_set_parent, vFpp)
GO(libdecor_frame_set_title, vFpp)
GO(libdecor_frame_set_visibility, vFpi)
GO(libdecor_frame_show_window_menu, vFppuii)
//GO(libdecor_frame_toplevel_commit,
GO(libdecor_frame_translate_coordinate, vFpiipp)
GO(libdecor_frame_unref, vFp)
GO(libdecor_frame_unset_capabilities, vFpu)
GO(libdecor_frame_unset_fullscreen, vFp)
GO(libdecor_frame_unset_maximized, vFp)
GO(libdecor_get_fd, iFp)
//GO(libdecor_get_wl_display,
GO(libdecor_new, pFpp)
//GO(libdecor_notify_plugin_error,
//GO(libdecor_notify_plugin_ready,
//GO(libdecor_plugin_init,
//GO(libdecor_plugin_release,
GO(libdecor_state_free, vFp)
//GO(libdecor_state_get_content_height,
//GO(libdecor_state_get_content_width,
//GO(libdecor_state_get_window_state,
GO(libdecor_state_new, pFii)
GO(libdecor_unref, vFp)
