#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//GO(_fini, 
GO(g_module_build_path, pFpp)
GO(g_module_close, iFp)
GO(g_module_error, pFv)
GO(g_module_make_resident, vFp)
GO(g_module_name, pFp)
GO(g_module_open, pFpu)
GO(g_module_supported, iFv)
GO(g_module_symbol, iFppp)
//GO(_init, 
