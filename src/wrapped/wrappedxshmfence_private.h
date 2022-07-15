#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(xshmfence_alloc_shm, iFv)
GO(xshmfence_await, iFp)
GO(xshmfence_map_shm, pFi)
GO(xshmfence_query, iFp)
GO(xshmfence_reset, vFp)
GO(xshmfence_trigger, iFp)
GO(xshmfence_unmap_shm, vFp)
