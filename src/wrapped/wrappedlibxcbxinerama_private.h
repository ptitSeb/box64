#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//GO(xcb_xinerama_get_screen_count, 
//GO(xcb_xinerama_get_screen_count_reply, 
//GO(xcb_xinerama_get_screen_count_unchecked, 
//GO(xcb_xinerama_get_screen_size, 
//GO(xcb_xinerama_get_screen_size_reply, 
//GO(xcb_xinerama_get_screen_size_unchecked, 
//GO(xcb_xinerama_get_state, 
//GO(xcb_xinerama_get_state_reply, 
//GO(xcb_xinerama_get_state_unchecked, 
DATA(xcb_xinerama_id, 2*sizeof(void*))
GO(xcb_xinerama_is_active, pFbp)
GO(xcb_xinerama_is_active_reply, pFbup)
GO(xcb_xinerama_is_active_unchecked, pFbp)
GO(xcb_xinerama_query_screens, pFbp)
GO(xcb_xinerama_query_screens_reply, pFbup)
GO(xcb_xinerama_query_screens_screen_info, pFp)
GO(xcb_xinerama_query_screens_screen_info_iterator, HFp)
GO(xcb_xinerama_query_screens_screen_info_length, iFp)
GO(xcb_xinerama_query_screens_sizeof, iFp)
GO(xcb_xinerama_query_screens_unchecked, pFbp)
//GO(xcb_xinerama_query_version, 
//GO(xcb_xinerama_query_version_reply, 
//GO(xcb_xinerama_query_version_unchecked, 
//GO(xcb_xinerama_screen_info_end, 
GO(xcb_xinerama_screen_info_next, vFp)
