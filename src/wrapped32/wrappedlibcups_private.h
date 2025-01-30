#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

//GOM(cupsAddDest, iFEppibp_)
//GOM(cupsAddIntegerOption, iFEpiibp_)
//GOM(cupsAddOption, iFEppibp_)
GO(cupsAdminCreateWindowsPPD, pFpppi)
GO(cupsAdminExportSamba, iFpppppS)
//GOM(cupsAdminGetServerSettings, iFEppbp_)
GO(cupsAdminSetServerSettings, iFpibpp_)
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
GO(cupsBackendDeviceURI, pFbp_)
GO(cupsBackendReport, vFpppppp)
//GO(_cupsBufferGet, 
//GO(_cupsBufferRelease, 
//GOM(cupsCancelDestJob, iFEpbppiip_i)
GO(cupsCancelJob, iFpi)
GO(cupsCancelJob2, iFppii)
//GO(_cupsCharmapFlush, 
GO(cupsCharsetToUTF8, iFppii)
//GOM(cupsCheckDestSupported, iFEpbppiip_ppp)
//GOM(cupsCloseDestJob, iFEpbppiip_pi)
//GO(_cupsCondBroadcast, 
//GO(_cupsCondInit, 
//GO(_cupsCondWait, 
//GO(_cupsConnect, 
//GOM(cupsConnectDest, pFEbppiip_uippLpp)
//GO(_cupsConvertOptions, 
//GOM(cupsCopyDest, iFEbppiip_ibp_)
//GOM(cupsCopyDestConflicts, iFEpbppiip_pibpp_pppbp_pbp_)
//GOM(cupsCopyDestInfo, pFEpbppiip_)
//GO(_cupsCreateDest, 
//GOM(cupsCreateDestJob, iFEpbppiip_pppibpp_)
GO(cupsCreateJob, iFpppibpp_)
//DATA(_cups_debug_fd, 
//DATA(_cups_debug_level, 
GO(cupsDirClose, vFp)
GO(cupsDirOpen, pFp)
//GO(cupsDirRead, pFp)
GO(cupsDirRewind, vFp)
GO(cupsDoAuthentication, iFppp)
GO(cupsDoFileRequest, pFpppp)
GO(cupsDoIORequest, pFpppii)
GO(cupsDoRequest, pFppp)
GO(cupsEncodeOptions, vFpibpp_)
GO(cupsEncodeOptions2, vFpibpp_i)
//GO(_cupsEncodingName, 
GO(cupsEncryption, uFv)
//GOM(cupsEnumDests, iFEuipuupp)
//GO(_cupsFileCheck, 
//GO(_cupsFileCheckFilter, 
GO(cupsFileClose, iFp)
GO(cupsFileCompression, iFp)
GO(cupsFileEOF, iFp)
GO(cupsFileFind, pFppipi)
GO(cupsFileFlush, iFp)
GO(cupsFileGetChar, iFp)
GO(cupsFileGetConf, pFppLbp_p)
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
GO(cupsFileRewind, IFp)
GO(cupsFileSeek, IFpI)
GO(cupsFileStderr, pFv)
GO(cupsFileStdin, pFv)
GO(cupsFileStdout, pFv)
GO(cupsFileTell, IFp)
GO(cupsFileUnlock, iFp)
GO(cupsFileWrite, lFppL)
//GOM(cupsFindDestDefault, pFEpbppiip_pp)
//GOM(cupsFindDestReady, pFEpbppiip_pp)
//GOM(cupsFindDestSupported, pFEpbppiip_pp)
//GOM(cupsFinishDestDocument, iFEpbppiip_p)
GO(cupsFinishDocument, iFpp)
GO(cupsFreeDestInfo, vFp)
GOM(cupsFreeDests, vFEip)
//GO(cupsFreeJobs, vFibippppuiiIII_)
GO(cupsFreeOptions, vFibpp_)
//GO(_cupsGet1284Values, 
//GOM(cupsGetClasses, iFEbp_)
//GOM(cupsGetConflicts, iFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_ppbp_)
GO(cupsGetDefault, pFv)
GO(cupsGetDefault2, pFp)
//GOM(cupsGetDest, pFEppibppiip_)
//GOM(cupsGetDestMediaByIndex, iFEpbppiip_piup)
//GOM(cupsGetDestMediaByName, iFEpbppiip_ppup)
//GOM(cupsGetDestMediaBySize, iFEpbppiip_piiup)
//GOM(cupsGetDestMediaCount, iFEpbppiip_pu)
//GOM(cupsGetDestMediaDefault, iFEpbppiip_pup)
//GO(_cupsGetDestResource, 
//GO(_cupsGetDests, 
GOM(cupsGetDests, iFEp)
//GOM(cupsGetDests2, iFEpbp_)
//GOM(cupsGetDestWithURI, pFEpp)
//GOM(cupsGetDevices, iFEpipppp)
GO(cupsGetFd, iFppi)
GO(cupsGetFile, iFppp)
GO(cupsGetIntegerOption, iFpibpp_)
//GOM(cupsGetJobs, iFEbp_pii)
//GOM(cupsGetJobs2, iFEpbp_pii)
//GOM(cupsGetNamedDest, pFEppp)
GOM(cupsGetOption, pFEpip)
//GO(_cupsGetPassword, 
GO(cupsGetPassword, pFp)
GO(cupsGetPassword2, pFpppp)
GO(cupsGetPPD, pFp)
GO(cupsGetPPD2, pFpp)
GO(cupsGetPPD3, iFppbL_pL)
//GOM(cupsGetPrinters, iFEbp_)
GO(cupsGetResponse, pFpp)
GO(cupsGetServerPPD, pFpp)
//GO(_cupsGlobalLock, 
//GO(_cupsGlobals, 
//GO(_cupsGlobalUnlock, 
//GO(_cupsGSSServiceName, 
GO(cupsHashData, lFppLpL)
GO(cupsHashString, pFpLpL)
//GOM(cupsLangDefault, pFEv)
//GOM(cupsLangEncoding, pFEp)
GO(cupsLangFlush, vFv)
//GOM(cupsLangFree, vFEp)
//GOM(cupsLangGet, pFEp)
//GO(_cupsLangPrintError, 
//GO(_cupsLangPrintf, 
//GO(_cupsLangPrintFilter, 
//GO(_cupsLangPuts, 
//GO(_cupsLangString, 
GO(cupsLastError, iFv)
GO(cupsLastErrorString, pFv)
//GOM(cupsLocalizeDestMedia, pFEpbppiip_pup)
//GOM(cupsLocalizeDestOption, pFEpbppiip_pp)
//GOM(cupsLocalizeDestValue, pFEpbppiip_ppp)
GO(cupsMakeServerCredentials, iFppibp_I)
//GOM(cupsMarkOptions, iFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_ibpp_)
//GO(_cupsMessageFree, 
//GO(_cupsMessageLoad, 
//GO(_cupsMessageLookup, 
//GO(_cupsMessageNew, 
//GO(_cupsMutexInit, 
//GO(_cupsMutexLock, 
//GO(_cupsMutexUnlock, 
//GO(_cupsNextDelay, 
//GOM(cupsNotifySubject, pFEpp)
//GOM(cupsNotifyText, pFEpp)
//GOM(cupsParseOptions, iFEpibp_)
GO(cupsPrintFile, iFpppibpp_)
GO(cupsPrintFile2, iFppppibpp_)
GO(cupsPrintFiles, iFpibp_pibpp_)
GO(cupsPrintFiles2, iFppibp_pibpp_)
GO(cupsPutFd, iFppi)
GO(cupsPutFile, iFppp)
GO(cupsReadResponseData, lFppL)
//GOM(cupsRemoveDest, iFEppibp_)
//GOM(cupsRemoveOption, iFEpibp_)
//GOM(cupsResolveConflicts, iFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_pppbp_)
//GO(_cupsRWInit, 
//GO(_cupsRWLockRead, 
//GO(_cupsRWLockWrite, 
//GO(_cupsRWUnlock, 
//GO(_cups_safe_vsnprintf, 
GO(cupsSendRequest, iFpppL)
GO(cupsServer, pFv)
//GOM(cupsSetClientCertCB, vFEpp)
GO(cupsSetCredentials, iFp)
//GOM(cupsSetDefaultDest, vFEppibppiip_)
//GO(_cupsSetDefaults, 
//GOM(cupsSetDests, vFEibppiip_)
//GOM(cupsSetDests2, iFEpibppiip_)
GO(cupsSetEncryption, vFu)
//GO(_cupsSetError, 
//GO(_cupsSetHTTPError, 
//GO(_cupsSetLocale, 
//GO(_cupsSetNegotiateAuthString, 
//GOM(cupsSetPasswordCB, vFEp)
//GOM(cupsSetPasswordCB2, vFEpp)
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
//GOM(cupsStartDestDocument, iFEpbppiip_pippibpp_i)
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
GO(cupsUTF32ToUTF8, iFpbL_i)
GO(cupsUTF8ToCharset, iFppii)
GO(cupsUTF8ToUTF32, iFbL_pi)
GO(cupsWriteRequestData, iFppL)

GO(httpAcceptConnection, pFii)
GO(httpAddCredential, iFppL)
GO(httpAddrAny, iFp)
GO(httpAddrClose, iFpi)
//GOM(httpAddrConnect, pFEpp)
//GOM(httpAddrConnect2, pFEppip)
//GOM(httpAddrCopyList, pFEp)
GO(httpAddrEqual, iFpp)
GO(httpAddrFamily, iFp)
//GOM(httpAddrFreeList, vFEp)
//GOM(httpAddrGetList, pFEpip)
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
//GOM(httpConnect2, pFEpipiuiip)
GO(httpConnectEncrypt, pFpiu)
GO(httpCopyCredentials, iFpbp_)
//GO(_httpCreateCredentials, 
GO(httpCredentialsAreValidForName, iFpp)
GO(httpCredentialsGetExpiration, IFp)
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
GO(httpGetActivity, IFp)
GO(httpGetAddress, pFp)
GO(httpGetAuthString, pFp)
GO(httpGetBlocking, iFp)
GO(httpGetContentEncoding, pFp)
GO(httpGetCookie, pFp)
GO(httpGetDateString, pFI)
GO(httpGetDateString2, pFIpi)
GO(httpGetDateTime, IFp)
GO(httpGetEncryption, uFp)
GO(httpGetExpect, iFp)
GO(httpGetFd, iFp)
GO(httpGetField, pFpi)
//GOM(httpGetHostByName, pFEp)
GO(httpGetHostname, pFppi)
GO(httpGetKeepAlive, uFp)
GO(httpGetLength, iFp)
GO(httpGetLength2, IFp)
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
GO(httpLoadCredentials, iFpbp_p)
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
GO(ippAddCollections, pFpipibp_)
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
//GOM(ippAddStringfv, pFEpiipppp)
GO(ippAddStrings, pFpiipipbp_)
GO(ippAttributeString, LFppL)
GO(ippContainsInteger, iFpi)
GO(ippContainsString, iFpp)
GO(ippCopyAttribute, pFppi)
//GOM(ippCopyAttributes, iFEppipp)
GO(ippCreateRequestedArray, pFp)
GO(ippDateToTime, IFp)
GO(ippDelete, vFp)
GO(ippDeleteAttribute, vFpp)
GO(ippDeleteValues, iFpbp_ii)
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
GO(ippGetString, pFpibp_)
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
GO(ippSetBoolean, iFpbp_ii)
GO(ippSetCollection, iFpbp_ip)
GO(ippSetDate, iFpbp_ip)
GO(ippSetGroupTag, iFpbp_i)
GO(ippSetInteger, iFpbp_ii)
GO(ippSetName, iFpbp_p)
GO(ippSetOctetString, iFpbp_ipi)
GO(ippSetOperation, iFpi)
GO(ippSetPort, vFi)
GO(ippSetRange, iFpbp_iii)
GO(ippSetRequestId, iFpi)
GO(ippSetResolution, iFpbp_iuii)
GO(ippSetState, iFpi)
GO(ippSetStatusCode, iFpi)
GO(ippSetString, iFpbp_ip)
//GOM(ippSetStringf, iFEpbp_ipV)
//GOM(ippSetStringfv, iFEpbp_ipp)
GO(ippSetValueTag, iFpbp_i)
GO(ippSetVersion, iFpii)
GO(ippStateString, pFi)
GO(ippTagString, pFi)
GO(ippTagValue, iFp)
GO(ippTimeToDate, pFI)
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
//GOM(ppdClose, vFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_)
//GOM(ppdCollect, iFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_ubp_)
//GOM(ppdCollect2, iFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_ufbp_)
//GOM(ppdConflicts, iFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_)
//GO(_ppdCreateFromIPP, 
//GOM(ppdEmit, iFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_Su)
//GOM(ppdEmitAfterOrder, iFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_Suif)
//GOM(ppdEmitFd, iFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_iu)
//GOM(ppdEmitJCL, iFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_Sipp)
//GOM(ppdEmitJCLEnd, iFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_S)
//GOM(ppdEmitString, pFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_uf)
GO(ppdErrorString, pFu)
//GOM(ppdFindAttr, pFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_pp)
//GOM(ppdFindChoice, pFEpp)
//GOM(ppdFindCustomOption, pFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_p)
//GOM(ppdFindCustomParam, pFEbcccccccccccccccccccccccccccccccccccccccccpip_p)
//GOM(ppdFindMarkedChoice, pFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_p)
//GOM(ppdFindNextAttr, pFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_pp)
//GOM(ppdFindOption, pFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_p)
//GOM(ppdFirstCustomParam, pFEbcccccccccccccccccccccccccccccccccccccccccpip_)
//GOM(ppdFirstOption, pFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_)
//GO(_ppdFreeLanguages, 
//GO(_ppdGetEncoding, 
//GO(_ppdGetLanguages, 
//GO(_ppdGlobals, 
//GO(_ppdHashName, 
//GOM(ppdInstallableConflict, iFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_pp)
//GOM(ppdIsMarked, iFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_pp)
GO(ppdLastError, uFp)
//GOM(ppdLocalize, iFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_)
//GOM(ppdLocalizeAttr, pFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_pp)
//GO(_ppdLocalizedAttr, 
//GOM(ppdLocalizeIPPReason, pFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_pppL)
//GOM(ppdLocalizeMarkerName, pFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_p)
//GOM(ppdMarkDefaults, vFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_)
//GOM(ppdMarkOption, iFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_pp)
//GOM(ppdNextCustomParam, pFEbcccccccccccccccccccccccccccccccccccccccccpip_)
//GOM(ppdNextOption, pFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_)
//GO(_ppdNormalizeMakeAndModel, 
//GO(_ppdOpen, 
//GOM(ppdOpen, pFES)
//GOM(ppdOpen2, pFEp)
//GOM(ppdOpenFd, pFEi)
//GO(_ppdOpenFile, 
//GOM(ppdOpenFile, pFEp)
//GOM(ppdPageLength, fFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_p)
//GOM(ppdPageSize, pFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_p)
//GOM(ppdPageSizeLimits, iFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_pp)
//GOM(ppdPageWidth, fFEbiiiiiiiiiipippppppppppppipipffffffffipipipipippiippppppp_p)
//GO(_ppdParseOptions, 
GO(ppdSetConformance, vFu)

GO(pwgFormatSizeName, iFpLppiip)
//GO(pwgInitSize, iFppp)
//GO(_pwgInputSlotForSource, 
//GOM(pwgMediaForLegacy, pFEp)
//GOM(pwgMediaForPPD, pFEp)
//GOM(pwgMediaForPWG, pFEp)
//GOM(pwgMediaForSize, pFEii)
//GO(_pwgMediaNearSize, 
//GO(_pwgMediaTable, 
//GO(_pwgMediaTypeForType, 
//GO(_pwgPageSizeForMedia, 
