#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
error Meh...
#endif

GO(lt__slist_box, pFp)
GO(lt__slist_cons, pFpp)
GO(lt__slist_concat, pFpp)
GO(lt_dlloader_add, iFpp)
//GO(lt_dlloader_next
GO(lt_dlloader_get, pFp)
GO(lt_dlloader_remove, iFp)
GO(lt_dlloader_find, pFp)
GO(lt_dlinterface_register, iFpp)
//GO(lt_dlinterface_free,
GO(lt__slist_remove, pFpp)
GO(lt__slist_unbox, pFp)
GO(lt__slist_find, pFpp)
GO(lt__slist_delete, vFp)
GO(lt__slist_tail, pFp)
GO(lt__slist_nth, pFp)
//GO(lt__slist_length
GO(lt__slist_reverse, pFp)
//GO(lt__slist_foreach
//GO(lt__slist_sort
GO(lt_dladderror, vFp)
GO(lt_dlseterror, vFp)
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
GO(lt_dladdsearchdir, iFp)
GO(lt_dlinsertsearchdir, iFpp)
GO(lt_dlsetsearchpath, iFp)
GO(lt_dlgetsearchpath, pFv)
//GO(lt_dlopenext
GO(lt__get_last_error, pFv)