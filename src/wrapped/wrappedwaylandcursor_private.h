#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(wl_cursor_frame, iFpu)
GO(wl_cursor_frame_and_duration, iFpup)
GO(wl_cursor_image_get_buffer, pFp)
GO(wl_cursor_theme_destroy, vFp)
GO(wl_cursor_theme_get_cursor, pFpp)
GO(wl_cursor_theme_load, pFpip)
