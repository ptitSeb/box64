#if defined(GO) && defined(GOM) && defined(GO2) && defined(DATA)

GOM(dladdr, iFEpp)
// dladdr1
GOM(dlclose, iFEp)
GOM(dlerror, pFE)
DATAB(_dlfcn_hook, 4)
GOM(dlinfo, iFEpip)
GOM(dlmopen, pFEppi)
GOM(dlopen, pFEpi)
GOM(dlsym, pFEpp)
GOM(dlvsym, pFEppp)   // Weak

#endif