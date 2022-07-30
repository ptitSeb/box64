#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(wl_egl_window_create, pFpii)
GO(wl_egl_window_destroy, vFp)
GO(wl_egl_window_get_attached_size, vFppp)
GO(wl_egl_window_resize, vFpiiii)

GO(wl_egl_pixmap_create, pFiiu)
GO(wl_egl_pixmap_destroy, vFp)
GO(wl_egl_pixmap_create_buffer, pFp)