#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

//GO(xpmatoui, 
GO(XpmAttributesSize, iFv)
GO(XpmCreateBufferFromImage, iFppppp)   // need unwrapping of Image?
GO(XpmCreateBufferFromPixmap, iFppppp)
GO(XpmCreateBufferFromXpmImage, iFppp)
GO(XpmCreateDataFromImage, iFppppp)
GO(XpmCreateDataFromPixmap, iFppppp)
GO(XpmCreateDataFromXpmImage, iFppp)
GO(XpmCreateImageFromBuffer, iFppppp)   // XImage...
GO(XpmCreateImageFromData, iFppppp)     // XImage...
//GO(xpmCreateImageFromPixmap, 
GO(XpmCreateImageFromXpmImage, iFppppp)
GO(XpmCreatePixmapFromBuffer, iFpppppp)
GO(XpmCreatePixmapFromData, iFpppppp)
//GO(xpmCreatePixmapFromImage, 
GO(XpmCreatePixmapFromXpmImage, iFpppppp)
GO(XpmCreateXpmImageFromBuffer, iFppp)
GO(XpmCreateXpmImageFromData, iFppp)
GO(XpmCreateXpmImageFromImage, iFppppp)
GO(XpmCreateXpmImageFromPixmap, iFppppp)
GO(XpmFree, vFp)
GO(XpmFreeAttributes, vFp)
//GO(xpmFreeColorTable, 
GO(XpmFreeExtensions, vFpi)
//GO(xpmFreeRgbNames, 
GO(XpmFreeXpmImage, vFp)
GO(XpmFreeXpmInfo, vFp)
//GO(xpmGetCmt, 
GO(XpmGetErrorString, pFi)
//GO(xpmGetRgbName, 
//GO(xpmGetString, 
//GO(xpmHashIntern, 
//GO(xpmHashSlot, 
//GO(xpmHashTableFree, 
//GO(xpmHashTableInit, 
//GO(xpmInitAttributes, 
//GO(xpmInitXpmImage, 
//GO(xpmInitXpmInfo, 
GO(XpmLibraryVersion, iFv)
//GO(xpmNextString, 
//GO(xpmNextUI, 
//GO(xpmNextWord, 
//GO(xpmParseColors, 
//GO(xpmParseData, 
//GO(xpmParseDataAndCreate, 
//GO(xpmParseExtensions, 
//GO(xpmParseHeader, 
//GO(xpmParseValues, 
//GO(xpmPipeThrough, 
GO(XpmReadFileToBuffer, iFpp)
GO(XpmReadFileToData, iFpp)
//GO(XpmReadFileToImage, 
GO(XpmReadFileToPixmap, iFpppppp)
GO(XpmReadFileToXpmImage, iFppp)
//GO(xpmReadRgbNames, 
//GO(xpmSetAttributes, 
//GO(xpmSetInfo, 
//GO(xpmSetInfoMask, 
GO(XpmWriteFileFromBuffer, iFpp)
GO(XpmWriteFileFromData, iFpp)
//GO(XpmWriteFileFromImage, 
GO(XpmWriteFileFromPixmap, iFppppp)
GO(XpmWriteFileFromXpmImage, iFppp)
//GO(xpm_xynormalizeimagebits, 
//GO(xpm_znormalizeimagebits, 
