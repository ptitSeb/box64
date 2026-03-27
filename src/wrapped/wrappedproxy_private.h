#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GO(px_proxy_factory_copy, pFp)
GO(px_proxy_factory_free, vFp)
GO(px_proxy_factory_free_proxies, vFp)
GO(px_proxy_factory_get_proxies, pFpp)
GO(px_proxy_factory_get_type, LFv)
GO(px_proxy_factory_new, pFv)
