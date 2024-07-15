#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

//GO(TIFFVGetFieldDefaulted,
// GO(TIFFVGetField, iFpWA)
GO(TIFFCleanup, vFp)
GO(TIFFFreeDirectory, vFp)
GO(TIFFFlush, iFp)
GO(TIFFClose, vFp)
GO(TIFFFindCODEC, pFW)
GO(TIFFIsCODECConfigured, iFW)
GO(TIFFInitDumpMode, iFpi)
GO(TIFFInitLZW, iFpi)
//GO(TIFFInitPackBits
GO(TIFFInitThunderScan, iFpi)
GO(TIFFInitNeXT, iFpi)
GO(TIFFInitJPEG, iFpi)
//GO(TIFFInitOJPEG
GO(TIFFInitCCITTRLE, iFpi)
GO(TIFFInitCCITTRLEW, iFpi)
GO(TIFFInitCCITTFax3, iFpi)
//GO(TIFFInitCCITTFax4
GO(TIFFInitJBIG, iFpi)
GO(TIFFInitZIP, iFpi)
//GO(TIFFInitPixarLog
//GO(TIFFInitSGILog
GO(TIFFInitLZMA, iFpi)
GO(TIFFInitZSTD, iFpi)
GO(TIFFInitWebP, iFpi)
// GO(TIFFCIELabToXYZ, vFpuiippp)
GO(TIFFXYZToRGB, iFpppp)
GO(TIFFCIELabToRGBInit, iFpp)
//GO(TIFFYCbCrtoRGB
//GO(TIFFYCbCrToRGBInit
GO(TIFFSetCompressionScheme, iFpi)
//GO(TIFFRegisterCODEC
//GO(TIFFUnRegisterCODEC
GO(TIFFGetConfiguredCODECs, pFv)
//GO(TIFFFindField
GO(TIFFSwabLong, vFp)
GO(TIFFSwabLong8, vFp)
GO(TIFFSwabShort, vFp)