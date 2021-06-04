#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(FcAtomicCreate, pFp)
GO(FcAtomicDeleteNew, vFp)
GO(FcAtomicDestroy, vFp)
GO(FcAtomicLock, iFp)
//GO(FcAtomicNewFile, 
//GO(FcAtomicOrigFile, 
//GO(FcAtomicReplaceOrig, 
//GO(FcAtomicUnlock, 
//GO(FcBlanksAdd, 
//GO(FcBlanksCreate, 
//GO(FcBlanksDestroy, 
//GO(FcBlanksIsMember, 
GO(FcCacheCopySet, pFp)
//GO(FcCacheDir, 
//GO(FcCacheNumFont, 
GO(FcCacheNumSubdir, iFp)
GO(FcCacheSubdir, pFpi)
GO(FcCharSetAddChar, iFpu)
GO(FcCharSetCopy, pFp)
//GO(FcCharSetCount, 
//GO(FcCharSetCoverage, 
GO(FcCharSetCreate, pFv)
GO(FcCharSetDestroy, vFp)
//GO(FcCharSetEqual, 
GO(FcCharSetFirstPage, iFppp)   // second p is a fixed sized array
GO(FcCharSetHasChar, iFpi)
//GO(FcCharSetIntersect, 
//GO(FcCharSetIntersectCount, 
//GO(FcCharSetIsSubset, 
//GO(FcCharSetMerge, 
//GO(FcCharSetNew, 
GO(FcCharSetNextPage, iFppp)    // second p is a fixed sized array
GO(FcCharSetSubtract, pFpp)
//GO(FcCharSetSubtractCount, 
GO(FcCharSetUnion, pFpp)
//GO(FcConfigAppFontAddDir, 
GO(FcConfigAppFontAddFile, iFpp)
//GO(FcConfigAppFontClear, 
//GO(FcConfigBuildFonts, 
//GO(FcConfigCreate, 
//GO(FcConfigDestroy, 
//GO(FcConfigEnableHome, 
//GO(FcConfigFilename, 
GO(FcConfigGetBlanks, pFp)
//GO(FcConfigGetCache, 
//GO(FcConfigGetCacheDirs, 
//GO(FcConfigGetConfigDirs, 
//GO(FcConfigGetConfigFiles, 
GO(FcConfigGetCurrent, pFv)
GO(FcConfigGetFontDirs, pFp)
GO(FcConfigGetFonts, pFpi)
//GO(FcConfigGetRescanInterval, 
//GO(FcConfigGetRescanInverval, 
GO(FcConfigGetSysRoot, pFp)
//GO(FcConfigHome, 
//GO(FcConfigParseAndLoad, 
GO(FcConfigReference, pFp)
//GO(FcConfigSetCurrent, 
//GO(FcConfigSetRescanInterval, 
//GO(FcConfigSetRescanInverval, 
GO(FcConfigSubstitute, iFppi)
GO(FcConfigSubstituteWithPat, iFpppi)
//GO(FcConfigUptoDate, 
GO(FcDefaultSubstitute, vFp)
//GO(FcDirCacheLoad, 
//GO(FcDirCacheLoadFile, 
GO(FcDirCacheRead, pFpip)
//GO(FcDirCacheUnlink, 
GO(FcDirCacheUnload, vFp)
//GO(FcDirCacheValid, 
//GO(FcDirSave, 
//GO(FcDirScan, 
//GO(FcFileIsDir, 
//GO(FcFileScan, 
//GO(FcFini, 
GO(FcFontList, pFppp)
GO(FcFontMatch, pFppp)
GO(FcFontRenderPrepare, pFppp)
GO(FcFontSetAdd, iFpp)
//GO(FcFontSetCreate, 
GO(FcFontSetDestroy, vFp)
//GO(FcFontSetList, 
//GO(FcFontSetMatch, 
//GO(FcFontSetPrint, 
//GO(FcFontSetSort, 
//GO(FcFontSetSortDestroy, 
GO(FcFontSort, pFppipp)
//GO(FcFreeTypeCharIndex, 
//GO(FcFreeTypeCharSet, 
//GO(FcFreeTypeCharSetAndSpacing, 
GO(FcFreeTypeQuery, pFpipp)
GO(FcFreeTypeQueryFace, pFppip)
//GO(FcGetLangs, 
GO(FcGetVersion, iFv)
GO(FcInit, iFv)
//GO(FcInitBringUptoDate, 
//GO(FcInitLoadConfig, 
//GO(FcInitLoadConfigAndFonts, 
GO(FcInitReinitialize, iFv)
//GO(FcLangGetCharSet, 
GO(FcLangSetAdd, iFpp)
//GO(FcLangSetCompare, 
//GO(FcLangSetContains, 
//GO(FcLangSetCopy, 
GO(FcLangSetCreate, pFv)
GO(FcLangSetDestroy, vFp)
//GO(FcLangSetEqual, 
//GO(FcLangSetGetLangs, 
//GO(FcLangSetHash, 
GO(FcLangSetHasLang, iFpp)
//GO(FcMatrixCopy, 
//GO(FcMatrixEqual, 
//GO(FcMatrixMultiply, 
//GO(FcMatrixRotate, 
//GO(FcMatrixScale, 
//GO(FcMatrixShear, 
GO(FcNameConstant, iFpp)
//GO(FcNameGetConstant, 
//GO(FcNameGetObjectType, 
//GO(FcNameParse, 
//GO(FcNameRegisterConstants, 
//GO(FcNameRegisterObjectTypes, 
//GO(FcNameUnparse, 
//GO(FcNameUnregisterConstants, 
//GO(FcNameUnregisterObjectTypes, 
GO(FcObjectSetAdd, iFpp)
//GO2(FcObjectSetBuild, pFpV, FcObjectSetVaBuild)
GO(FcObjectSetCreate, pFv)
GO(FcObjectSetDestroy, vFp)
GO(FcObjectSetVaBuild, pFpp)
GO(FcPatternAdd, iFppiLi)  // FcValue is a typedef with int+union, with biggest part is a double => so 1 int and 1"L"
GO(FcPatternAddBool, iFppi)
GO(FcPatternAddCharSet, iFppp)
GO(FcPatternAddDouble, iFppd)
GO(FcPatternAddFTFace, iFppp)
GO(FcPatternAddInteger, iFppi)
GO(FcPatternAddLangSet, iFppp)
GO(FcPatternAddMatrix, iFppp)
GO(FcPatternAddString, iFppp)
GO(FcPatternAddWeak, iFppiLi)
//GO2(FcPatternBuild, pFpV, FcPatternVaBuild)
GO(FcPatternCreate, pFv)
GO(FcPatternDel, iFpp)
GO(FcPatternDestroy, vFp)
GO(FcPatternDuplicate, pFp)
GO(FcPatternEqual, iFpp)
//GO(FcPatternEqualSubset, 
//GO(FcPatternFilter, 
//GO(FcPatternFormat, 
GO(FcPatternGet, iFppip)
GO(FcPatternGetBool, iFppip)
GO(FcPatternGetCharSet, iFppip)
GO(FcPatternGetDouble, iFppip)
GO(FcPatternGetFTFace, iFppip)
GO(FcPatternGetInteger, iFppip)
GO(FcPatternGetLangSet, iFppip)
GO(FcPatternGetMatrix, iFppip)
GO(FcPatternGetString, iFppip)
GO(FcPatternHash, iFp)
//GO(FcPatternPrint, 
//GO(FcPatternReference, 
//GO(FcPatternRemove, 
GO(FcPatternVaBuild, pFpp)
GO(FcStrBasename, pFp)
GO(FcStrCmp, iFpp)
GO(FcStrCmpIgnoreCase, iFpp)
GO(FcStrCopy, pFp)
GO(FcStrCopyFilename, pFp)
GO(FcStrDirname, pFp)
GO(FcStrDowncase, pFp)
GO(FcStrFree, vFp)
GO(FcStrListCreate, pFp)
GO(FcStrListDone, vFp)
GO(FcStrListFirst, vFp) //2.11.0+
GO(FcStrListNext, pFp)
GO(FcStrPlus, pFpp)
GO(FcStrSetAdd, iFpp)
GO(FcStrSetAddFilename, iFpp)
GO(FcStrSetCreate, pFv)
GO(FcStrSetDel, iFpp)
GO(FcStrSetDestroy, vFp)
GO(FcStrSetEqual, iFpp)
GO(FcStrSetMember, iFpp)
GO(FcStrStr, pFpp)
GO(FcStrStrIgnoreCase, pFpp)
GO(FcUcs4ToUtf8, iFup)
GO(FcUtf16Len, iFpipp)
GO(FcUtf16ToUcs4, iFpipi)
GO(FcUtf8Len, iFpipp)
GO(FcUtf8ToUcs4, iFppi)
//GO(FcValueDestroy, 
//GO(FcValueEqual, 
//GO(FcValuePrint, 
//GO(FcValueSave, 
//GO(_fini, 
//GO(_init, 
