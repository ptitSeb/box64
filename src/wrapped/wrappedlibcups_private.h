#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GO(cupsAddDest, iFppip)
GO(cupsAddIntegerOption, iFpiip)
GO(cupsAddOption, iFppip)
GO(cupsAdminCreateWindowsPPD, pFpppi)
GO(cupsAdminExportSamba, iFpppppS)
GO(cupsAdminGetServerSettings, iFppp)
GO(cupsAdminSetServerSettings, iFpip)
GO(cupsArrayAdd, iFpp)
//GO(_cupsArrayAddStrings, 
GO(cupsArrayClear, vFp)
GO(cupsArrayCount, iFp)
GO(cupsArrayCurrent, pFp)
GO(cupsArrayDelete, vFp)
GO(cupsArrayDup, pFp)
GO(cupsArrayFind, pFpp)
GO(cupsArrayFirst, pFp)
GO(cupsArrayGetIndex, iFp)
GO(cupsArrayGetInsert, iFp)
GO(cupsArrayIndex, pFpi)
GO(cupsArrayInsert, iFpp)
GO(cupsArrayLast, pFp)
//GOM(cupsArrayNew, pFEpp)
//GOM(cupsArrayNew2, pFEpppi)
//GOM(cupsArrayNew3, pFEpppipp)
//GO(_cupsArrayNewStrings, 
GO(cupsArrayNext, pFp)
GO(cupsArrayPrev, pFp)
GO(cupsArrayRemove, iFpp)
GO(cupsArrayRestore, pFp)
GO(cupsArraySave, iFp)
GO(cupsArrayUserData, pFp)
GO(cupsBackChannelRead, lFpLd)
GO(cupsBackChannelWrite, lFpLd)
GO(cupsBackendDeviceURI, pFp)
GO(cupsBackendReport, vFpppppp)
//GO(_cupsBufferGet, 
//GO(_cupsBufferRelease, 
GO(cupsCancelDestJob, iFppi)
GO(cupsCancelJob, iFpi)
GO(cupsCancelJob2, iFppii)
//GO(_cupsCharmapFlush, 
GO(cupsCharsetToUTF8, iFppii)
GO(cupsCheckDestSupported, iFppppp)
GO(cupsCloseDestJob, iFpppi)
//GO(_cupsCondBroadcast, 
//GO(_cupsCondInit, 
//GO(_cupsCondWait, 
//GO(_cupsConnect, 
//GOM(cupsConnectDest, pFEpuippLpp)
//GO(_cupsConvertOptions, 
GO(cupsCopyDest, iFpip)
GO(cupsCopyDestConflicts, iFpppippppppp)
GO(cupsCopyDestInfo, pFpp)
//GO(_cupsCreateDest, 
GO(cupsCreateDestJob, iFpppppip)
GO(cupsCreateJob, iFpppip)
//DATA(_cups_debug_fd, 
//DATA(_cups_debug_level, 
GO(cupsDirClose, vFp)
GO(cupsDirOpen, pFp)
GO(cupsDirRead, pFp)
GO(cupsDirRewind, vFp)
GO(cupsDoAuthentication, iFppp)
GO(cupsDoFileRequest, pFpppp)
GO(cupsDoIORequest, pFpppii)
GO(cupsDoRequest, pFppp)
GO(cupsEncodeOptions, vFpip)
GO(cupsEncodeOptions2, vFpipi)
//GO(_cupsEncodingName, 
GO(cupsEncryption, uFv)
GOM(cupsEnumDests, iFEuipuupp)
//GO(_cupsFileCheck, 
//GO(_cupsFileCheckFilter, 
GO(cupsFileClose, iFp)
GO(cupsFileCompression, iFp)
GO(cupsFileEOF, iFp)
GO(cupsFileFind, pFppipi)
GO(cupsFileFlush, iFp)
GO(cupsFileGetChar, iFp)
GO(cupsFileGetConf, pFppLpp)
GO(cupsFileGetLine, LFppL)
GO(cupsFileGets, pFppL)
GO(cupsFileLock, iFpi)
GO(cupsFileNumber, iFp)
GO(cupsFileOpen, pFpp)
GO(cupsFileOpenFd, pFip)
//GO(_cupsFilePeekAhead, 
GO(cupsFilePeekChar, iFp)
//GOM(cupsFilePrintf, iFEppV)
GO(cupsFilePutChar, iFpi)
GO(cupsFilePutConf, lFppp)
GO(cupsFilePuts, iFpp)
GO(cupsFileRead, lFppL)
GO(cupsFileRewind, lFp)
GO(cupsFileSeek, lFpl)
GO(cupsFileStderr, pFv)
GO(cupsFileStdin, pFv)
GO(cupsFileStdout, pFv)
GO(cupsFileTell, lFp)
GO(cupsFileUnlock, iFp)
GO(cupsFileWrite, lFppL)
GO(cupsFindDestDefault, pFpppp)
GO(cupsFindDestReady, pFpppp)
GO(cupsFindDestSupported, pFpppp)
GO(cupsFinishDestDocument, iFppp)
GO(cupsFinishDocument, iFpp)
GO(cupsFreeDestInfo, vFp)
GO(cupsFreeDests, vFip)
GO(cupsFreeJobs, vFip)
GO(cupsFreeOptions, vFip)
//GO(_cupsGet1284Values, 
GO(cupsGetClasses, iFp)
GO(cupsGetConflicts, iFpppp)
GO(cupsGetDefault, pFv)
GO(cupsGetDefault2, pFp)
GO(cupsGetDest, pFppip)
GO(cupsGetDestMediaByIndex, iFpppiup)
GO(cupsGetDestMediaByName, iFppppup)
GO(cupsGetDestMediaBySize, iFpppiiup)
GO(cupsGetDestMediaCount, iFpppu)
GO(cupsGetDestMediaDefault, iFpppup)
//GO(_cupsGetDestResource, 
//GO(_cupsGetDests, 
GO(cupsGetDests, iFp)
GO(cupsGetDests2, iFpp)
GO(cupsGetDestWithURI, pFpp)
//GOM(cupsGetDevices, iFEpipppp)
GO(cupsGetFd, iFppi)
GO(cupsGetFile, iFppp)
GO(cupsGetIntegerOption, iFpip)
GO(cupsGetJobs, iFppii)
GO(cupsGetJobs2, iFpppii)
GO(cupsGetNamedDest, pFppp)
GO(cupsGetOption, pFpip)
//GO(_cupsGetPassword, 
GO(cupsGetPassword, pFp)
GO(cupsGetPassword2, pFpppp)
GO(cupsGetPPD, pFp)
GO(cupsGetPPD2, pFpp)
GO(cupsGetPPD3, iFppppL)
GO(cupsGetPrinters, iFp)
GO(cupsGetResponse, pFpp)
GO(cupsGetServerPPD, pFpp)
//GO(_cupsGlobalLock, 
//GO(_cupsGlobals, 
//GO(_cupsGlobalUnlock, 
//GO(_cupsGSSServiceName, 
GO(cupsHashData, lFppLpL)
GO(cupsHashString, pFpLpL)
GO(cupsLangDefault, pFv)
GO(cupsLangEncoding, pFp)
GO(cupsLangFlush, vFv)
GO(cupsLangFree, vFp)
GO(cupsLangGet, pFp)
//GO(_cupsLangPrintError, 
//GO(_cupsLangPrintf, 
//GO(_cupsLangPrintFilter, 
//GO(_cupsLangPuts, 
//GO(_cupsLangString, 
GO(cupsLastError, iFv)
GO(cupsLastErrorString, pFv)
GO(cupsLocalizeDestMedia, pFpppup)
GO(cupsLocalizeDestOption, pFpppp)
GO(cupsLocalizeDestValue, pFppppp)
GO(cupsMakeServerCredentials, iFppipl)
GO(cupsMarkOptions, iFpip)
//GO(_cupsMessageFree, 
//GO(_cupsMessageLoad, 
//GO(_cupsMessageLookup, 
//GO(_cupsMessageNew, 
//GO(_cupsMutexInit, 
//GO(_cupsMutexLock, 
//GO(_cupsMutexUnlock, 
//GO(_cupsNextDelay, 
GO(cupsNotifySubject, pFpp)
GO(cupsNotifyText, pFpp)
GO(cupsParseOptions, iFpip)
GO(cupsPrintFile, iFpppip)
GO(cupsPrintFile2, iFppppip)
GO(cupsPrintFiles, iFpippip)
GO(cupsPrintFiles2, iFppippip)
GO(cupsPutFd, iFppi)
GO(cupsPutFile, iFppp)
GO(_cupsRasterDelete, vFp)
GO(_cupsRasterErrorString, pFv)
GO(_cupsRasterInitPWGHeader, iFpppiipp)
GOM(_cupsRasterInterpretPPD, iFppipp)
GOM(_cupsRasterNew, pFEppu)
GO(_cupsRasterReadHeader, uFp)
GO(_cupsRasterReadPixels, uFppu)
GO(_cupsRasterWriteHeader, uFp)
GO(_cupsRasterWritePixels, uFppu)
GO(cupsReadResponseData, lFppL)
GO(cupsRemoveDest, iFppip)
GO(cupsRemoveOption, iFpip)
GO(cupsResolveConflicts, iFppppp)
//GO(_cupsRWInit, 
//GO(_cupsRWLockRead, 
//GO(_cupsRWLockWrite, 
//GO(_cupsRWUnlock, 
//GO(_cups_safe_vsnprintf, 
GO(cupsSendRequest, iFpppL)
GO(cupsServer, pFv)
//GOM(cupsSetClientCertCB, vFEpp)
GO(cupsSetCredentials, iFp)
GO(cupsSetDefaultDest, vFppip)
//GO(_cupsSetDefaults, 
GO(cupsSetDests, vFip)
GO(cupsSetDests2, iFpip)
GO(cupsSetEncryption, vFu)
//GO(_cupsSetError, 
//GO(_cupsSetHTTPError, 
//GO(_cupsSetLocale, 
//GO(_cupsSetNegotiateAuthString, 
//GOM(cupsSetPasswordCB, vFEp)
GOM(cupsSetPasswordCB2, vFEpp)
GO(cupsSetServer, vFp)
//GOM(cupsSetServerCertCB, vFEpp)
GO(cupsSetServerCredentials, iFppi)
GO(cupsSetUser, vFp)
GO(cupsSetUserAgent, vFp)
GO(cupsSideChannelDoRequest, uFuppd)
GO(cupsSideChannelRead, iFppppd)
GO(cupsSideChannelSNMPGet, uFpppd)
//GOM(cupsSideChannelSNMPWalk, uFEpdpp)
GO(cupsSideChannelWrite, iFuupid)
//GO(_cupsSNMPClose, 
//GO(_cupsSNMPCopyOID, 
//GO(_cupsSNMPDefaultCommunity, 
//GO(_cupsSNMPIsOID, 
//GO(_cupsSNMPIsOIDPrefixed, 
//GO(_cupsSNMPOIDToString, 
//GO(_cupsSNMPOpen, 
//GO(_cupsSNMPRead, 
//GO(_cupsSNMPSetDebug, 
//GO(_cupsSNMPStringToOID, 
//GO(_cupsSNMPWalk, 
//GO(_cupsSNMPWrite, 
GO(cupsStartDestDocument, iFpppippipi)
GO(cupsStartDocument, iFppippi)
//GO(_cupsStrAlloc, 
//GO(_cups_strcasecmp, 
//GO(_cups_strcpy, 
//GO(_cupsStrDate, 
//GO(_cupsStrFlush, 
//GO(_cupsStrFormatd, 
//GO(_cupsStrFree, 
//GO(_cups_strlcat, 
//GO(_cups_strlcpy, 
//GO(_cups_strncasecmp, 
//GO(_cupsStrRetain, 
//GO(_cupsStrScand, 
//GO(_cupsStrStatistics, 
GO(cupsTempFd, iFpi)
GO(cupsTempFile, pFpi)
GO(cupsTempFile2, pFpi)
//GO(_cupsThreadCancel, 
//GO(_cupsThreadCreate, 
//GO(_cupsThreadDetach, 
//GO(_cupsThreadWait, 
GO(cupsUser, pFv)
GO(cupsUserAgent, pFv)
//GO(_cupsUserDefault, 
GO(cupsUTF32ToUTF8, iFppi)
GO(cupsUTF8ToCharset, iFppii)
GO(cupsUTF8ToUTF32, iFppi)
GO(cupsWriteRequestData, iFppL)

GO(httpAcceptConnection, pFii)
GO(httpAddCredential, iFppL)
GO(httpAddrAny, iFp)
GO(httpAddrClose, iFpi)
GO(httpAddrConnect, pFpp)
GO(httpAddrConnect2, pFppip)
GO(httpAddrCopyList, pFp)
GO(httpAddrEqual, iFpp)
GO(httpAddrFamily, iFp)
GO(httpAddrFreeList, vFp)
GO(httpAddrGetList, pFpip)
GO(httpAddrLength, iFp)
GO(httpAddrListen, iFpi)
GO(httpAddrLocalhost, iFp)
GO(httpAddrLookup, pFppi)
GO(httpAddrPort, iFp)
//GO(_httpAddrSetPort, 
GO(httpAddrString, pFppi)
GO(httpAssembleURI, iFupipppip)
//GOM(httpAssembleURIf, iFEupipppipV)
GO(httpAssembleUUID, pFpipipL)
GO(httpBlocking, vFpi)
GO(httpCheck, iFp)
GO(httpClearCookie, vFp)
GO(httpClearFields, vFp)
GO(httpClose, vFp)
GO(httpCompareCredentials, iFpp)
GO(httpConnect, pFpi)
GO(httpConnect2, pFpipiuiip)
GO(httpConnectEncrypt, pFpiu)
GO(httpCopyCredentials, iFpp)
//GO(_httpCreateCredentials, 
GO(httpCredentialsAreValidForName, iFpp)
GO(httpCredentialsGetExpiration, lFp)
GO(httpCredentialsGetTrust, uFpp)
GO(httpCredentialsString, LFppL)
GO(httpDecode64, pFpp)
GO(httpDecode64_2, pFppp)
//GO(_httpDecodeURI, 
GO(httpDelete, iFpp)
//GO(_httpDisconnect, 
GO(httpEncode64, pFpp)
GO(httpEncode64_2, pFpipi)
//GO(_httpEncodeURI, 
GO(httpEncryption, iFpu)
GO(httpError, iFp)
GO(httpFieldValue, iFp)
GO(httpFlush, vFp)
GO(httpFlushWrite, iFp)
//GO(_httpFreeCredentials, 
GO(httpFreeCredentials, vFp)
GO(httpGet, iFpp)
GO(httpGetActivity, lFp)
GO(httpGetAddress, pFp)
GO(httpGetAuthString, pFp)
GO(httpGetBlocking, iFp)
GO(httpGetContentEncoding, pFp)
GO(httpGetCookie, pFp)
GO(httpGetDateString, pFl)
GO(httpGetDateString2, pFlpi)
GO(httpGetDateTime, lFp)
GO(httpGetEncryption, uFp)
GO(httpGetExpect, iFp)
GO(httpGetFd, iFp)
GO(httpGetField, pFpi)
GO(httpGetHostByName, pFp)
GO(httpGetHostname, pFppi)
GO(httpGetKeepAlive, uFp)
GO(httpGetLength, iFp)
GO(httpGetLength2, lFp)
GO(httpGetPending, LFp)
GO(httpGetReady, LFp)
GO(httpGetRemaining, LFp)
GO(httpGets, pFpip)
GO(httpGetState, iFp)
GO(httpGetStatus, iFp)
GO(httpGetSubField, pFpipp)
GO(httpGetSubField2, pFpippi)
GO(httpGetVersion, uFp)
GO(httpHead, iFpp)
GO(httpInitialize, vFv)
GO(httpIsChunked, iFp)
GO(httpIsEncrypted, iFp)
GO(httpLoadCredentials, iFppp)
GO(httpMD5, pFpppp)
GO(httpMD5Final, pFpppp)
GO(httpMD5String, pFpp)
GO(httpOptions, iFpp)
GO(httpPeek, lFppL)
GO(httpPost, iFpp)
//GOM(httpPrintf, iFEppV)
GO(httpPut, iFpp)
GO(httpRead, iFppi)
GO(httpRead2, lFppL)
GO(httpReadRequest, iFppL)
GO(httpReconnect, iFp)
GO(httpReconnect2, iFpip)
GO(httpResolveHostname, pFppL)
//GO(_httpResolveURI, 
GO(httpSaveCredentials, iFppp)
GO(httpSeparate, vFpppppp)
GO(httpSeparate2, vFppipipippi)
GO(httpSeparateURI, iFuppipipippi)
GO(httpSetAuthString, vFppp)
GO(httpSetCookie, vFpp)
GO(httpSetCredentials, iFpp)
GO(httpSetDefaultField, vFpip)
//GO(_httpSetDigestAuthString, 
GO(httpSetExpect, vFpi)
GO(httpSetField, vFpip)
GO(httpSetKeepAlive, vFpu)
GO(httpSetLength, vFpL)
//GOM(httpSetTimeout, vFEpdpp)
GO(httpShutdown, vFp)
GO(httpStateString, pFi)
//GO(_httpStatus, 
GO(httpStatus, pFi)
//GO(_httpTLSInitialize, 
//GO(_httpTLSPending, 
//GO(_httpTLSRead, 
//GO(_httpTLSSetCredentials, 
//GO(_httpTLSSetOptions, 
//GO(_httpTLSStart, 
//GO(_httpTLSStop, 
//GO(_httpTLSWrite, 
GO(httpTrace, iFpp)
//GO(_httpUpdate, 
GO(httpUpdate, iFp)
GO(httpURIStatusString, pFi)
//GO(_httpWait, 
GO(httpWait, iFpi)
GO(httpWrite, iFppi)
GO(httpWrite2, lFppL)
GO(httpWriteResponse, iFpi)

GO(ippAddBoolean, pFpipc)
GO(ippAddBooleans, pFpipip)
GO(ippAddCollection, pFpipp)
GO(ippAddCollections, pFpipip)
GO(ippAddDate, pFpipp)
GO(ippAddInteger, pFpiipi)
GO(ippAddIntegers, pFpiipip)
GO(ippAddOctetString, pFpippi)
GO(ippAddOutOfBand, pFpiip)
GO(ippAddRange, pFpipii)
GO(ippAddRanges, pFpipipp)
GO(ippAddResolution, pFpipuii)
GO(ippAddResolutions, pFpipiupp)
GO(ippAddSeparator, pFp)
GO(ippAddString, pFpiippp)
//GOM(ippAddStringf, pFEpiipppV)
//GOM(ippAddStringfv, pFEpiipppA)
GO(ippAddStrings, pFpiipipp)
GO(ippAttributeString, LFppL)
GO(ippContainsInteger, iFpi)
GO(ippContainsString, iFpp)
GO(ippCopyAttribute, pFppi)
//GOM(ippCopyAttributes, iFEppipp)
GO(ippCreateRequestedArray, pFp)
GO(ippDateToTime, lFp)
GO(ippDelete, vFp)
GO(ippDeleteAttribute, vFpp)
GO(ippDeleteValues, iFppii)
GO(ippEnumString, pFpi)
GO(ippEnumValue, iFpp)
GO(ippErrorString, pFi)
GO(ippErrorValue, iFp)
GO(ippFindAttribute, pFppi)
GO(ippFindNextAttribute, pFppi)
//GO(_ippFindOption, 
GO(ippFirstAttribute, pFp)
GO(ippGetBoolean, iFpi)
GO(ippGetCollection, pFpi)
GO(ippGetCount, iFp)
GO(ippGetDate, pFpi)
GO(ippGetGroupTag, iFp)
GO(ippGetInteger, iFpi)
GO(ippGetName, pFp)
GO(ippGetOctetString, pFpip)
GO(ippGetOperation, iFp)
GO(ippGetRange, iFpip)
GO(ippGetRequestId, iFp)
GO(ippGetResolution, iFpipp)
GO(ippGetState, iFp)
GO(ippGetStatusCode, iFp)
GO(ippGetString, pFpip)
GO(ippGetValueTag, iFp)
GO(ippGetVersion, iFpp)
GO(ippLength, LFp)
GO(ippNew, pFv)
GO(ippNewRequest, pFi)
GO(ippNewResponse, pFp)
GO(ippNextAttribute, pFp)
GO(ippOpString, pFi)
GO(ippOpValue, iFp)
GO(ippPort, iFv)
GO(ippRead, iFpp)
GO(ippReadFile, iFip)
//GOM(ippReadIO, iFEppipp)
GO(ippSetBoolean, iFppii)
GO(ippSetCollection, iFppip)
GO(ippSetDate, iFppip)
GO(ippSetGroupTag, iFppi)
GO(ippSetInteger, iFppii)
GO(ippSetName, iFppp)
GO(ippSetOctetString, iFppipi)
GO(ippSetOperation, iFpi)
GO(ippSetPort, vFi)
GO(ippSetRange, iFppiii)
GO(ippSetRequestId, iFpi)
GO(ippSetResolution, iFppiuii)
GO(ippSetState, iFpi)
GO(ippSetStatusCode, iFpi)
GO(ippSetString, iFppip)
//GOM(ippSetStringf, iFEppipV)
//GOM(ippSetStringfv, iFEppipA)
GO(ippSetValueTag, iFppi)
GO(ippSetVersion, iFpii)
GO(ippStateString, pFi)
GO(ippTagString, pFi)
GO(ippTagValue, iFp)
GO(ippTimeToDate, pFl)
GO(ippValidateAttribute, iFp)
GO(ippValidateAttributes, iFp)
GO(ippWrite, iFpp)
GO(ippWriteFile, iFip)
//GOM(ippWriteIO, iFEppipp)

//GO(_ppdCacheCreateWithFile, 
//GO(_ppdCacheCreateWithPPD, 
//GO(_ppdCacheDestroy, 
//GO(_ppdCacheGetBin, 
//GO(_ppdCacheGetFinishingOptions, 
//GO(_ppdCacheGetFinishingValues, 
//GO(_ppdCacheGetInputSlot, 
//GO(_ppdCacheGetMediaType, 
//GO(_ppdCacheGetOutputBin, 
//GO(_ppdCacheGetPageSize, 
//GO(_ppdCacheGetSize, 
//GO(_ppdCacheGetSource, 
//GO(_ppdCacheGetType, 
//GO(_ppdCacheWriteFile, 
GO(ppdClose, vFp)
GO(ppdCollect, iFpup)
GO(ppdCollect2, iFpufp)
GO(ppdConflicts, iFp)
//GO(_ppdCreateFromIPP, 
GO(ppdEmit, iFpSu)
GO(ppdEmitAfterOrder, iFpSuif)
GO(ppdEmitFd, iFpiu)
GO(ppdEmitJCL, iFpSipp)
GO(ppdEmitJCLEnd, iFpS)
GO(ppdEmitString, pFpuf)
GO(ppdErrorString, pFu)
GO(ppdFindAttr, pFppp)
GO(ppdFindChoice, pFpp)
GO(ppdFindCustomOption, pFpp)
GO(ppdFindCustomParam, pFpp)
GO(ppdFindMarkedChoice, pFpp)
GO(ppdFindNextAttr, pFppp)
GO(ppdFindOption, pFpp)
GO(ppdFirstCustomParam, pFp)
GO(ppdFirstOption, pFp)
//GO(_ppdFreeLanguages, 
//GO(_ppdGetEncoding, 
//GO(_ppdGetLanguages, 
//GO(_ppdGlobals, 
//GO(_ppdHashName, 
GO(ppdInstallableConflict, iFppp)
GO(ppdIsMarked, iFppp)
GO(ppdLastError, uFp)
GO(ppdLocalize, iFp)
GO(ppdLocalizeAttr, pFppp)
//GO(_ppdLocalizedAttr, 
GO(ppdLocalizeIPPReason, pFppppL)
GO(ppdLocalizeMarkerName, pFpp)
GO(ppdMarkDefaults, vFp)
GO(ppdMarkOption, iFppp)
GO(ppdNextCustomParam, pFp)
GO(ppdNextOption, pFp)
//GO(_ppdNormalizeMakeAndModel, 
//GO(_ppdOpen, 
GO(ppdOpen, pFS)
GO(ppdOpen2, pFp)
GO(ppdOpenFd, pFi)
//GO(_ppdOpenFile, 
GO(ppdOpenFile, pFp)
GO(ppdPageLength, fFpp)
GO(ppdPageSize, pFpp)
GO(ppdPageSizeLimits, iFppp)
GO(ppdPageWidth, fFpp)
//GO(_ppdParseOptions, 
GO(ppdSetConformance, vFu)

GO(pwgFormatSizeName, iFpLppiip)
GO(pwgInitSize, iFppp)
//GO(_pwgInputSlotForSource, 
GO(pwgMediaForLegacy, pFp)
GO(pwgMediaForPPD, pFp)
GO(pwgMediaForPWG, pFp)
GO(pwgMediaForSize, pFii)
//GO(_pwgMediaNearSize, 
//GO(_pwgMediaTable, 
//GO(_pwgMediaTypeForType, 
//GO(_pwgPageSizeForMedia, 
