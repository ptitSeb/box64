#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

//GO(TIFFVGetFieldDefaulted,
GO(TIFFCleanup, vFp)
GO(TIFFFreeDirectory, iFp)
GO(TIFFFlush, iFp)
GO(TIFFClose, vFp)
// GO(TIFFFindCODEC, pFW)
GO(TIFFIsCODECConfigured, iFW)
//GO(TIFFInitPackBits
//GO(TIFFInitOJPEG
//GO(TIFFInitCCITTFax4
//GO(TIFFInitLZMA
//GO(TIFFInitZSTD
//GO(TIFFInitWebP
GO(TIFFCIELabToXYZ, vFpuiippp)
GO(TIFFXYZToRGB, vFpfffppp)
GO(TIFFCIELabToRGBInit, iFppp)
GO(TIFFYCbCrtoRGB, vFpuiippp)
GO(TIFFYCbCrToRGBInit, iFppp)
GO(TIFFSetCompressionScheme, iFpi)
//GO(TIFFRegisterCODEC
//GO(TIFFUnRegisterCODEC
GO(TIFFGetConfiguredCODECs, pFW)
//GO(TIFFFindField
GO(TIFFSwabLong, vFp)
GO(TIFFSwabLong8, vFp)
GO(TIFFSwabShort, vFp)