#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
error Meh...
#endif

// GO(lt_dlloader_add
GO(lt_dlloader_next, pFp)
//GO(lt_dlloader_get
//GO(lt_dlloader_remove
//GO(lt_dlloader_find
//GO(lt_dlinterface_register
//GO(lt_dlinterface_free,
//GO(lt_dladderror
GO(lt_dlseterror, iFi)
GO(lt_dlinit, iFv)
GO(lt_dlexit, iFv)
GO(lt_dladvise_init, iFp)
GO(lt_dladvise_destroy, iFp)
GO(lt_dladvise_ext, iFp)
GO(lt_dladvise_resident, iFp)
GO(lt_dladvise_local, iFp)
GO(lt_dladvise_global, iFp)
GO(lt_dladvise_preload, iFp)
//GO(lt_dlopenadvise
//GO(lt_dlopenext