#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GO(libiconv_open, pFpp)
GO(locale_charset, pFv)
GO(libiconv_close, iFp)
GO(libiconvctl, iFpip)
GO(libiconv_set_relocation_prefix, vFpp)
GO(libiconv, LFppppp)
GO(libiconv_open_into, iFppp)
GO(libiconvlist, vFpp)
GO(iconv_canonicalize, pFp)

DATA(_libiconv_version, 4)