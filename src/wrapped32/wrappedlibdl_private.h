#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA) && defined(GOS))
#error Meh...
#endif

GOM(dladdr, iFEpp)
GOM(dladdr1, iFEpppi)
GO2(dlclose, iFEp, my_dlclose)
GO2(dlerror, pFEv, my_dlerror)
//DATAB(_dlfcn_hook, 4)
GOM(dlinfo, iFEpip)
GO2(dlmopen, pFEppi, my_dlmopen)
GO2(dlopen, pFEpi, my_dlopen)
GO2(dlsym, pFEpp, my_dlsym)
GO2(dlvsym, pFEppp, my_dlvsym)   // Weak
GOM(_dl_find_object, iFEpp)
