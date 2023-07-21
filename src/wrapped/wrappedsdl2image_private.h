#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh!
#endif

DATA(__data_start, 4)
DATA(_edata, 4)
// _fini
GO(IMG_Init,iFi)
GO(IMG_InvertAlpha,iFi)
GO(IMG_Linked_Version,pFv)
GO(IMG_Load,pFp)
GOM(IMG_LoadBMP_RW,pFEp)
GOM(IMG_LoadCUR_RW,pFEp)
GOM(IMG_LoadGIF_RW,pFEp)
GOM(IMG_LoadICO_RW,pFEp)
GOM(IMG_LoadJPG_RW,pFEp)
GOM(IMG_LoadLBM_RW,pFEp)
GOM(IMG_LoadPCX_RW,pFEp)
GOM(IMG_LoadPNG_RW,pFEp)
GOM(IMG_LoadPNM_RW,pFEp)
GOM(IMG_LoadTGA_RW,pFEp)
GOM(IMG_LoadTIF_RW,pFEp)
GOM(IMG_LoadTyped_RW,pFEpip)
GOM(IMG_LoadWEBP_RW,pFEp)
GOM(IMG_LoadXCF_RW,pFEp)
GOM(IMG_LoadXPM_RW,pFEp)
GOM(IMG_LoadXV_RW,pFEp)
GOM(IMG_Load_RW,pFEpi)
GO(IMG_Quit,vFv)
GO(IMG_ReadXPMFromArray,pFp)
GO(IMG_isBMP,iFp)
GO(IMG_isCUR,iFp)
GO(IMG_isGIF,iFp)
GO(IMG_isICO,iFp)
GO(IMG_isJPG,iFp)
GO(IMG_isLBM,iFp)
GO(IMG_isPCX,iFp)
GO(IMG_isPNG,iFp)
GO(IMG_isPNM,iFp)
GO(IMG_isTIF,iFp)
GO(IMG_isWEBP,iFp)
GO(IMG_isXCF,iFp)
GO(IMG_isXPM,iFp)
GO(IMG_isXV,iFp)
// IMG_InitJPG
// IMG_InitPNG
// IMG_InitTIF
// IMG_InitWEBP
// IMG_isSVG
// IMG_LoadSVG_RW
GO(IMG_LoadTexture, pFpp)
GOM(IMG_LoadTexture_RW, pFEppi)
GOM(IMG_LoadTextureTyped_RW, pFEppip)
// IMG_QuitJPG
// IMG_QuitPNG
// IMG_QuitTIF
// IMG_QuitWEBP
// IMG_SaveJPG
// IMG_SaveJPG_RW
GO(IMG_SavePNG, iFpp)
GOM(IMG_SavePNG_RW, iFEppi)
// _init
DATA(nsvg__colors, 4)
// nsvgCreateRasterizer
// nsvgDelete
// nsvgDeleteRasterizer
// nsvgParse
// nsvg__parseXML
// nsvgRasterize
