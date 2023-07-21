#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

GO(bottom_panel, iFp)
GO(del_panel, iFp)
GO(hide_panel, iFp)
GO(move_panel, iFpii)
GO(new_panel, pFp)
GO(panel_above, pFp)
GO(panel_below, pFp)
GO(panel_hidden, iFp)
GO(panel_userptr, pFp)
GO(panel_window, pFp)
GO(replace_panel, iFpp)
GO(set_panel_userptr, iFpp)
GO(show_panel, iFp)
GO(top_panel, iFp)
GO(update_panels, vFv)