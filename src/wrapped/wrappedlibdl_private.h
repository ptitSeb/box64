#if defined(GO) && defined(GOM) && defined(GO2) && defined(DATA)

GOM(dladdr, iFEpp)
GOM(dladdr1, iFEpppi)
GOM(dlclose, iFEp)
GOM(dlerror, pFEv)
#ifdef STATICBUILD
//DATAB(_dlfcn_hook, sizeof(void*))
#else
DATAB(_dlfcn_hook, sizeof(void*))
#endif
GOM(dlinfo, iFEpip)
GOM(dlmopen, pFEppi)
GOM(dlopen, pFEpi)
GOM(dlsym, pFEpp)
GOM(dlvsym, pFEppp)   // Weak
GOM(_dl_find_object, iFEpp)

#endif
