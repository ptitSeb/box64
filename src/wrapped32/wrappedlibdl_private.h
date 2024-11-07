#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA) && defined(GOS))
#error Meh...
#endif

GOM(dladdr, iEEpp)
GOM(dladdr1, iEEpppi)
GO2(dlclose, iEEp, my_dlclose)
GO2(dlerror, pEEv, my_dlerror)
//DATAB(_dlfcn_hook, 4)
GOM(dlinfo, iEEpip)
GO2(dlmopen, pEEppi, my_dlmopen)
GO2(dlopen, pEEpi, my_dlopen)
GO2(dlsym, pEEpp, my_dlsym)
GO2(dlvsym, pEEppp, my_dlvsym)   // Weak
GOM(_dl_find_object, iEEpp)
