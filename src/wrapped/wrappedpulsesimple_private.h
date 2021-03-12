#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GO(pa_simple_drain, iFpp)
GO(pa_simple_flush, iFpp)
GO(pa_simple_free, vFp)
GO(pa_simple_get_latency, UFpp)
GO(pa_simple_new, pFppipppppp)
GO(pa_simple_read, iFppLp)
GO(pa_simple_write, iFppLp)
