#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GO(idn2_check_version, pFp)
GO(idn2_free, vFp)
GO(idn2_lookup_u8, iFppi)
GO(idn2_lookup_ul, iFppi)
//GO(_idn2_punycode_decode, 
//GO(_idn2_punycode_encode, 
GO(idn2_register_u8, iFpppi)
GO(idn2_register_ul, iFpppi)
GO(idn2_strerror, pFi)
GO(idn2_strerror_name, pFi)
GO(idn2_to_ascii_4i, iFpLpi)
GO(idn2_to_ascii_4i2, iFpLpi)
GO(idn2_to_ascii_4z, iFppi)
GO(idn2_to_ascii_8z, iFppi)
GO(idn2_to_ascii_lz, iFppi)
GO(idn2_to_unicode_44i, iFpLppi)
GO(idn2_to_unicode_4z4z, iFppi)
GO(idn2_to_unicode_8z4z, iFppi)
GO(idn2_to_unicode_8z8z, iFppi)
GO(idn2_to_unicode_8zlz, iFppi)
GO(idn2_to_unicode_lzlz, iFppi)
