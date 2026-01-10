#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GO(cupsRasterClose, vFp)
GO(cupsRasterErrorString, pFv)
GO(cupsRasterInitPWGHeader, iFpppiipp)
GOM(cupsRasterInterpretPPD, iFEppipp)
GO(cupsRasterOpen, pFiu)
GOM(cupsRasterOpenIO, pFEppu)
GO(cupsRasterReadHeader, uFpp)
GO(cupsRasterReadHeader2, uFpp)
GO(cupsRasterReadPixels, uFppu)
GO(cupsRasterWriteHeader, uFpp)
GO(cupsRasterWriteHeader2, uFpp)
GO(cupsRasterWritePixels, uFppu)
