#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

//GO(calloc, 
//GO(_fini, 
//GO(free, 
//GO(_init, 
//GO(__libc_calloc, 
//GO(__libc_free, 
//GO(__libc_malloc, 
//GO(__libc_memalign, 
//GO(__libc_pvalloc, 
//GO(__libc_realloc, 
//GO(__libc_valloc, 
//GO(mallinfo, 
//GO(malloc, 
//GO(malloc_usable_size, 
//GO(mallopt, 
//GO(memalign, 
//GO(posix_memalign, 
//GO(pvalloc, 
//GO(realloc, 
GO2(__TBB_malloc_proxy, pFL, my_malloc)
//GO(valloc, 
GOM(_ZdaPv, vFp)                //%noE
GOM(_ZdaPvRKSt9nothrow_t, vFpp) //%noE
GOM(_ZdlPv, vFp)                //%noE
GOM(_ZdlPvRKSt9nothrow_t, vFpp) //%noE
GOM(_Znam, pFL)                 //%noE
GOM(_ZnamRKSt9nothrow_t, pFLp)  //%noE
GOM(_Znwm, pFL)                 //%noE
GOM(_ZnwmRKSt9nothrow_t, pFLp)  //%noE

GOM(__TBB_internal_find_original_malloc, iFipp) //%noE
GOM(__TBB_call_with_my_server_info, vFpp)   //%noE
GOM(__TBB_make_rml_server, iFppp)   //%noE
GOM(__RML_close_factory, vFp)   //%noE
GOM(__RML_open_factory, iFppi)  //%noE

GO(dummy_pFpLLp, pFpLLp)    // dummy to have pFpLLp for mallochook