#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

// CK_MECHANISM_TYPE is tyoe ULong
// CK_OBJECT_HANDLE is type ULong
// CK_ATTRIBUTE_TYPE is type ULong
// SECOidTag is an enum

GO(ATOB_AsciiToData, pFpp)
GO(ATOB_ConvertAsciiToItem, iFpp)
//GO(BTOA_ConvertItemToAscii, 
GO(BTOA_DataToAscii, pFpu)
GO(CERT_AddCertToListHead, iFpp)
//GO(CERT_AddCertToListSorted, 
GO(CERT_AddCertToListTail, iFpp)
GO(CERT_AddExtension, iFpipii)
GO(CERT_AddExtensionByOID, iFpppii)
//GO(CERT_AddOCSPAcceptableResponses, 
GO(CERT_AddOKDomainName, iFpp)
GO(CERT_AddRDN, iFpp)
//GO(__CERT_AddTempCertToPerm, 
GO(CERT_AllocCERTRevocationFlags, pFuuuu)
GO(CERT_AsciiToName, pFp)
//GO(CERT_CacheCRL, 
GO(CERT_CacheOCSPResponseFromSideChannel, iFpplpp)
GO(CERT_CertChainFromCert, pFpui)
//DATA(CERT_CertificateRequestTemplate, 
DATA(CERT_CertificateTemplate, 480)
GO(CERT_CertListFromCert, pFp)
//GO(CERT_CertTimesValid, 
GO(CERT_ChangeCertTrust, iFppp)
GO(CERT_CheckCertUsage, iFpC)
GO(CERT_CheckCertValidTimes, uFpli)
GO(CERT_CheckNameSpace, iFppp)
GO(CERT_CheckOCSPStatus, iFpplp)
GO(CERT_ClearOCSPCache, iFv)
//GO(__CERT_ClosePermCertDB, 
GO(CERT_CompareAVA, iFpp)
GO(CERT_CompareCerts, iFpp)
GO(CERT_CompareName, iFpp)
//GO(CERT_CompareValidityTimes, 
GO(CERT_CompleteCRLDecodeEntries, iFp)
GO(CERT_CopyName, iFppp)
GO(CERT_CopyRDN, iFppp)
GO(CERT_CreateAVA, pFpuip)
GO(CERT_CreateCertificate, pFLppp)
GO(CERT_CreateCertificateRequest, pFppp)
GO(CERT_CreateEncodedOCSPErrorResponse, pFpi)
GO(CERT_CreateEncodedOCSPSuccessResponse, pFppilpp)
//GO(CERT_CreateName, 
GO(CERT_CreateOCSPCertID, pFpl)
GO(CERT_CreateOCSPRequest, pFplip)
//GO(CERT_CreateOCSPSingleResponseGood, 
GO(CERT_CreateOCSPSingleResponseRevoked, pFpplplp)
GO(CERT_CreateOCSPSingleResponseUnknown, pFpplp)
//GO(CERT_CreateRDN, 
GO(CERT_CreateSubjectCertList, pFpppli)
GO(CERT_CreateValidity, pFll)
GO(CERT_CRLCacheRefreshIssuer, vFpp)
//DATA(CERT_CrlTemplate, 
GO(CERT_DecodeAltNameExtension, pFpp)
GO(CERT_DecodeAuthInfoAccessExtension, pFpp)
GO(CERT_DecodeAuthKeyID, pFpp)
GO(CERT_DecodeAVAValue, pFp)
GO(CERT_DecodeBasicConstraintValue, iFpp)
GO(CERT_DecodeCertificatePoliciesExtension, pFp)
GO(CERT_DecodeCRLDistributionPoints, pFpp)
//GO(__CERT_DecodeDERCertificate, 
GO(CERT_DecodeDERCrl, pFppi)
GO(CERT_DecodeDERCrlWithFlags, pFppii)
GO(CERT_DecodeGeneralName, pFppp)
GO(CERT_DecodeNameConstraintsExtension, pFpp)
GO(CERT_DecodeOCSPRequest, pFp)
GO(CERT_DecodeOCSPResponse, pFp)
GO(CERT_DecodeOidSequence, pFp)
GO(CERT_DecodePrivKeyUsagePeriodExtension, pFpp)
GO(CERT_DecodeTrustString, iFpp)
GO(CERT_DecodeUserNotice, pFp)
GO(CERT_DerNameToAscii, pFp)
GO(CERT_DestroyCertArray, vFpu)
GO(CERT_DestroyCertificate, vFp)
GO(CERT_DestroyCertificateList, vFp)
GO(CERT_DestroyCertificatePoliciesExtension, vFp)
GO(CERT_DestroyCertificateRequest, vFp)
GO(CERT_DestroyCertList, vFp)
GO(CERT_DestroyCERTRevocationFlags, vFp)
GO(CERT_DestroyName, vFp)
GO(CERT_DestroyOCSPCertID, iFp)
GO(CERT_DestroyOCSPRequest, vFp)
GO(CERT_DestroyOCSPResponse, vFp)
GO(CERT_DestroyOidSequence, vFp)
GO(CERT_DestroyUserNotice, vFp)
GO(CERT_DestroyValidity, vFp)
//GO(CERT_DisableOCSPChecking, 
GO(CERT_DisableOCSPDefaultResponder, iFp)
GO(CERT_DistNamesFromCertList, pFp)
GO(CERT_DupCertificate, pFp)
GO(CERT_DupCertList, pFp)
GO(CERT_DupDistNames, pFp)
//GO(CERT_EnableOCSPChecking, 
//GO(CERT_EnableOCSPDefaultResponder, 
GO(CERT_EncodeAltNameExtension, iFppp)
GO(CERT_EncodeAndAddBitStrExtension, iFpipi)
GO(CERT_EncodeAuthKeyID, iFppp)
GO(CERT_EncodeBasicConstraintValue, iFppp)
GO(CERT_EncodeCertPoliciesExtension, iFppp)
GO(CERT_EncodeCRLDistributionPoints, iFppp)
GO(CERT_EncodeGeneralName, pFppp)
GO(CERT_EncodeInfoAccessExtension, iFppp)
GO(CERT_EncodeInhibitAnyExtension, iFppp)
//GO(CERT_EncodeNameConstraintsExtension, 
GO(CERT_EncodeNoticeReference, iFppp)
GO(CERT_EncodeOCSPRequest, pFppp)
GO(CERT_EncodePolicyConstraintsExtension, iFppp)
GO(CERT_EncodePolicyMappingExtension, iFppp)
GO(CERT_EncodeSubjectKeyID, iFppp)
GO(CERT_EncodeUserNotice, iFppp)
GO(CERT_ExtractPublicKey, pFp)
GO(CERT_FilterCertListByCANames, iFpipu)
GO(CERT_FilterCertListByUsage, iFpui)
GO(CERT_FilterCertListForUserCerts, iFp)
GO(CERT_FindCertByDERCert, pFpp)
GO(CERT_FindCertByIssuerAndSN, pFpp)
GO(CERT_FindCertByIssuerAndSNCX, pFppp)
GO(CERT_FindCertByName, pFpp)
//GO(CERT_FindCertByNickname, 
GO(CERT_FindCertByNicknameOrEmailAddr, pFpp)
GO(CERT_FindCertByNicknameOrEmailAddrCX, pFppp)
GO(CERT_FindCertByNicknameOrEmailAddrForUsage, pFppu)
GO(CERT_FindCertByNicknameOrEmailAddrForUsageCX, pFppup)
//GO(CERT_FindCertBySubjectKeyID, 
GO(CERT_FindCertExtension, iFpip)
GO(CERT_FindCertIssuer, pFplu)
GO(CERT_FindCRLEntryReasonExten, iFpp)
GO(CERT_FindCRLNumberExten, iFppp)
//GO(CERT_FindKeyUsageExtension, 
GO(CERT_FindNameConstraintsExten, iFppp)
GO(CERT_FindSMimeProfile, pFp)
GO(CERT_FindSubjectKeyIDExtension, iFpp)
GO(CERT_FindUserCertByUsage, pFppuip)
GO(CERT_FindUserCertsByUsage, pFpuiip)
GO(CERT_FinishCertificateRequestAttributes, iFp)
GO(CERT_FinishExtensions, iFp)
GO(CERT_ForcePostMethodForOCSP, iFi)
GO(CERT_FormatName, pFp)
GO(CERT_FreeDistNames, vFp)
GO(CERT_FreeNicknames, vFp)
GO(CERT_GenTime2FormattedAscii, pFlp)
GO(CERT_GetAVATag, uFp)
GO(CERT_GetCertChainFromCert, pFplu)
GO(CERT_GetCertEmailAddress, pFp)
GO(CERT_GetCertificateDer, iFpp)
GO(CERT_GetCertificateNames, pFpp)
GO(CERT_GetCertificateRequestExtensions, iFpp)
GO(CERT_GetCertIsPerm, iFpp)
GO(CERT_GetCertIssuerAndSN, pFpp)
//GO(CERT_GetCertIsTemp, 
GO(CERT_GetCertKeyType, uFp)
GO(CERT_GetCertNicknames, pFpip)
GO(CERT_GetCertTimes, iFppp)
GO(CERT_GetCertTrust, iFpp)
GO(CERT_GetCertUid, pFp)
GO(CERT_GetClassicOCSPDisabledPolicy, pFv)
//GO(CERT_GetClassicOCSPEnabledHardFailurePolicy, 
//GO(CERT_GetClassicOCSPEnabledSoftFailurePolicy, 
GO(CERT_GetCommonName, pFp)
GO(CERT_GetConstrainedCertificateNames, pFppi)
GO(CERT_GetCountryName, pFp)
GO(CERT_GetDBContentVersion, iFp)
GO(CERT_GetDefaultCertDB, pFv)
GO(CERT_GetDomainComponentName, pFp)
GO(CERT_GetEncodedOCSPResponse, pFppplippp)
GO(CERT_GetFirstEmailAddress, pFp)
GO(CERT_GetGeneralNameTypeFromString, uFp)
GO(CERT_GetImposedNameConstraints, iFpp)
GO(CERT_GetLocalityName, pFp)
GO(CERT_GetNextEmailAddress, pFpp)
GO(CERT_GetNextGeneralName, pFp)
//GO(CERT_GetNextNameConstraint, 
GO(CERT_GetOCSPAuthorityInfoAccessLocation, pFp)
GO(CERT_GetOCSPResponseStatus, iFp)
GO(CERT_GetOCSPStatusForCertID, iFppppl)
GO(CERT_GetOidString, pFp)
GO(CERT_GetOrgName, pFp)
GO(CERT_GetOrgUnitName, pFp)
//GO(CERT_GetPKIXVerifyNistRevocationPolicy, 
GO(CERT_GetPrevGeneralName, pFp)
GO(CERT_GetPrevNameConstraint, pFp)
//GO(CERT_GetSlopTime, 
GO(CERT_GetSSLCACerts, pFp)
GO(CERT_GetStateName, pFp)
GO(CERT_GetSubjectNameDigest, pFppup)
//GO(CERT_GetSubjectPublicKeyDigest, 
//GO(CERT_GetUsePKIXForValidation, 
GO(CERT_GetValidDNSPatternsFromCert, pFp)
GO(CERT_Hexify, pFpi)
//GO(CERT_ImportCAChain, 
GO(CERT_ImportCAChainTrusted, iFpiu)
GO(CERT_ImportCerts, iFpuuppiip)
GO(CERT_ImportCRL, pFpppip)
GO(CERT_IsCACert, iFpp)
GO(CERT_IsCADERCert, iFpp)
GO(CERT_IsRootDERCert, iFp)
//DATA(CERT_IssuerAndSNTemplate, 
//GO(CERT_IsUserCert, 
//GO(CERT_KeyFromDERCrl, 
GO(CERT_MakeCANickname, pFp)
GO(CERT_MergeExtensions, iFpp)
DATA(CERT_NameTemplate, 4*sizeof(void*))
//GO(CERT_NameToAscii, 
GO(CERT_NameToAsciiInvertible, pFpu)
GO(CERT_NewCertList, pFv)
//GO(__CERT_NewTempCertificate, 
GO(CERT_NewTempCertificate, pFpppii)
GO(CERT_NicknameStringsFromCertList, pFppp)
GO(CERT_OCSPCacheSettings, iFiuu)
GO(CERT_OpenCertDBFilename, iFppi)
GOM(CERT_PKIXVerifyCert, iFEpIppp)
GO(CERT_PostOCSPRequest, pFppp)
GOM(CERT_RegisterAlternateOCSPAIAInfoCallBack, iFEpp)
GO(CERT_RemoveCertListNode, vFp)
GO(CERT_RFC1485_EscapeAndQuote, iFpipi)
GO(CERT_SaveSMimeProfile, iFppp)
//DATA(CERT_SequenceOfCertExtensionTemplate, 
GO(CERT_SetOCSPDefaultResponder, iFppp)
GO(CERT_SetOCSPFailureMode, iFu)
GO(CERT_SetOCSPTimeout, iFu)
//DATA(CERT_SetOfSignedCrlTemplate, 
GO(CERT_SetSlopTime, iFi)
GO(CERT_SetUsePKIXForValidation, iFi)
//DATA(CERT_SignedCrlTemplate, 
DATA(CERT_SignedDataTemplate, 160)
GO(CERT_StartCertExtensions, pFp)
GO(CERT_StartCertificateRequestAttributes, pFp)
GO(CERT_StartCRLEntryExtensions, pFpp)
GO(CERT_StartCRLExtensions, pFp)
//DATA(CERT_SubjectPublicKeyInfoTemplate, 
//DATA(CERT_TimeChoiceTemplate,     //R type
//GO(__CERT_TraversePermCertsForNickname, 
//GO(__CERT_TraversePermCertsForSubject, 
GO(CERT_UncacheCRL, iFpp)
//GO(CERT_VerifyCACertForUsage, 
GO(CERT_VerifyCert, iFpp)
GO(CERT_VerifyCertificate, iFppillppp)
GO(CERT_VerifyCertificateNow, iFppilpp)
GO(CERT_VerifyCertName, iFpp)
GO(CERT_VerifyCertNow, iFppiup)
GO(CERT_VerifyOCSPResponseSignature, iFppppp)
GO(CERT_VerifySignedData, iFpplp)
GO(CERT_VerifySignedDataWithPublicKey, iFppp)
GO(CERT_VerifySignedDataWithPublicKeyInfo, iFppp)
GO(DER_AsciiToTime, iFpp)
GO(DER_DecodeTimeChoice, iFpp)
GO(DER_Encode, iFpppp)
GO(DER_EncodeTimeChoice, iFppl)
//GO(DER_GeneralizedDayToAscii, 
//GO(DER_GeneralizedTimeToTime, 
GO(DER_GetInteger, lFp)
GO(DER_Lengths, iFppp)
GO(DER_TimeChoiceDayToAscii, pFp)
GO(DER_TimeToGeneralizedTime, iFpl)
//GO(DER_TimeToGeneralizedTimeArena, 
//GO(DER_TimeToUTCTime, 
//GO(DER_UTCDayToAscii, 
//GO(DER_UTCTimeToAscii, 
//GO(DER_UTCTimeToTime, 
GO(DSAU_DecodeDerSig, pFp)
GO(DSAU_DecodeDerSigToLen, pFpu)
GO(DSAU_EncodeDerSig, iFpp)
GO(DSAU_EncodeDerSigWithLen, iFppu)
GO(HASH_Begin, vFp)
//GO(HASH_Clone, 
GO(HASH_Create, pFi)
GO(HASH_Destroy, vFp)
GO(HASH_End, vFpppu)
GO(HASH_GetHashObject, pFu)
GO(HASH_GetHashObjectByOidTag, pFu)
GO(HASH_GetHashOidTagByHashType, uFu)
GO(HASH_GetHashTypeByOidTag, uFu)
GO(HASH_GetType, iFp)
GO(HASH_HashBuf, iFuppu)
GO(HASH_ResultLen, uFu)
GO(HASH_ResultLenByOidTag, uFu)
GO(HASH_ResultLenContext, uFp)
GO(HASH_Update, vFppu)
GO(NSSBase64_DecodeBuffer, pFpppu)
//GO(NSSBase64Decoder_Create, 
GO(NSSBase64Decoder_Destroy, iFpi)
GO(NSSBase64Decoder_Update, iFppu)
GO(NSSBase64_EncodeItem, pFppup)
//GO(NSSBase64Encoder_Create, 
GO(NSSBase64Encoder_Destroy, iFpi)
GO(NSSBase64Encoder_Update, iFppu)
//GO(nss_DumpCertificateCacheInfo, 
//GO(NSS_Get_CERT_CertificateRequestTemplate, 
//GO(NSS_Get_CERT_CertificateTemplate, 
//GO(NSS_Get_CERT_CrlTemplate, 
//GO(NSS_Get_CERT_IssuerAndSNTemplate, 
//GO(NSS_Get_CERT_NameTemplate, 
//GO(NSS_Get_CERT_SequenceOfCertExtensionTemplate, 
//GO(NSS_Get_CERT_SetOfSignedCrlTemplate, 
//GO(NSS_Get_CERT_SignedCrlTemplate, 
//GO(NSS_Get_CERT_SignedDataTemplate, 
//GO(NSS_Get_CERT_SubjectPublicKeyInfoTemplate, 
//GO(NSS_Get_CERT_TimeChoiceTemplate, 
//GO(NSS_Get_SEC_AnyTemplate, 
//GO(NSS_Get_SEC_BitStringTemplate, 
//GO(NSS_Get_SEC_BMPStringTemplate, 
//GO(NSS_Get_SEC_BooleanTemplate, 
//GO(NSS_Get_SEC_GeneralizedTimeTemplate, 
//GO(NSS_Get_SEC_IA5StringTemplate, 
//GO(NSS_Get_SEC_IntegerTemplate, 
//GO(NSS_Get_SECKEY_DSAPublicKeyTemplate, 
//GO(NSS_Get_SECKEY_EncryptedPrivateKeyInfoTemplate, 
//GO(NSS_Get_SECKEY_PointerToEncryptedPrivateKeyInfoTemplate, 
//GO(NSS_Get_SECKEY_PointerToPrivateKeyInfoTemplate, 
//GO(NSS_Get_SECKEY_PrivateKeyInfoTemplate, 
GO(NSS_Get_SECKEY_RSAPSSParamsTemplate, pFpi)
//GO(NSS_Get_SECKEY_RSAPublicKeyTemplate, 
//GO(NSS_Get_SEC_NullTemplate, 
//GO(NSS_Get_SEC_ObjectIDTemplate, 
//GO(NSS_Get_SEC_OctetStringTemplate, 
//GO(NSS_Get_SECOID_AlgorithmIDTemplate, 
//GO(NSS_Get_SEC_PointerToAnyTemplate, 
//GO(NSS_Get_SEC_PointerToOctetStringTemplate, 
//GO(NSS_Get_SEC_SetOfAnyTemplate, 
//GO(NSS_Get_SEC_SignedCertificateTemplate, 
//GO(NSS_Get_SEC_UTCTimeTemplate, 
//GO(NSS_Get_SEC_UTF8StringTemplate, 
GO(NSS_Get_sgn_DigestInfoTemplate, pFpi)
GO(NSS_GetVersion, pFv)
//GO(NSS_Init, 
GO(NSS_InitContext, pFpppppu)
GO(NSS_Initialize, iFppppu)
//GO(__nss_InitLock, 
GO(NSS_InitReadWrite, iFp)
GO(NSS_InitWithMerge, iFpppppppppu)
GO(NSS_IsInitialized, iFv)
GO(NSS_NoDB_Init, iFp)
//GO(NSS_OptionGet, 
//GO(NSS_OptionSet, 
//GO(NSS_PutEnv, 
GOM(NSS_RegisterShutdown, iFEpp)
//GO(NSSRWLock_Destroy, 
GO(NSSRWLock_HaveWriteLock, iFp)
//GO(NSSRWLock_LockRead, 
//GO(NSSRWLock_LockWrite, 
GO(NSSRWLock_New, pFup)
//GO(NSSRWLock_UnlockRead, 
GO(NSSRWLock_UnlockWrite, vFp)
//GO(NSS_Shutdown, 
GO(NSS_ShutdownContext, iFp)
//GO(NSS_UnregisterShutdown, 
GO(NSS_VersionCheck, iFp)
//GO(__PBE_CreateContext, 
GO(PBE_CreateContext, pFuuppuu)
//GO(__PBE_DestroyContext, 
GO(PBE_DestroyContext, vFp)
//GO(__PBE_GenerateBits, 
GO(PBE_GenerateBits, pFp)
GO(PK11_AlgtagToMechanism, LFu)
GO(PK11_Authenticate, iFpip)
GO(PK11_BlockData, pFpL)
GO(PK11_ChangePW, iFppp)
//GO(PK11_CheckSSOPassword, 
GO(PK11_CheckUserPassword, iFpp)
GO(PK11_CipherOp, iFpppipi)
GO(PK11_CloneContext, pFp)
GO(PK11_ConfigurePKCS11, vFppppppppii)
GO(PK11_ConvertSessionPrivKeyToTokenPrivKey, pFpp)
GO(PK11_ConvertSessionSymKeyToTokenSymKey, pFpp)
GO(PK11_CopySymKeyForSigning, pFpL)
GO(PK11_CopyTokenPrivKeyToSessionPrivKey, pFpp)
GO(__PK11_CreateContextByRawKey, pFpiiippp)
GO(PK11_CreateContextBySymKey, pFLLpp)
GO(PK11_CreateDigestContext, pFu)
GO(PK11_CreateGenericObject, pFppii)
//GO(PK11_CreateManagedGenericObject, 
GO(PK11_CreateMergeLog, pFv)
GO(PK11_CreatePBEAlgorithmID, pFuip)
GO(PK11_CreatePBEParams, pFppu)
GO(PK11_CreatePBEV2AlgorithmID, pFuuuiip)
//GO(PK11_Decrypt, 
GO(PK11_DeleteTokenCertAndKey, iFpp)
GO(PK11_DeleteTokenPrivateKey, iFpi)
GO(PK11_DeleteTokenPublicKey, iFp)
//GO(PK11_DeleteTokenSymKey, 
GO(PK11_DEREncodePublicKey, pFp)
GO(PK11_Derive, pFpLpLLi)
GO(PK11_DeriveWithFlags, pFpLpLLiL)
GO(PK11_DeriveWithFlagsPerm, pFpLpLLiLi)
GO(PK11_DeriveWithTemplate, pFpLpLLipui)
GO(PK11_DestroyContext, vFpi)
GO(PK11_DestroyGenericObject, iFp)
GO(PK11_DestroyGenericObjects, uFp)
GO(PK11_DestroyMergeLog, vFp)
GO(PK11_DestroyObject, iFpL)
GO(PK11_DestroyPBEParams, vFp)
GO(PK11_DestroyTokenObject, iFpL)
GO(PK11_DigestBegin, iFp)
GO(PK11_DigestFinal, iFpppu)
GO(PK11_DigestKey, iFpp)
GO(PK11_DigestOp, iFppu)
GO(PK11_DoesMechanism, iFpL)
GO(PK11_Encrypt, iFpLpppupu)
GO(PK11_ExportDERPrivateKeyInfo, pFpp)
GO(PK11_ExportEncryptedPrivateKeyInfo, pFpuppip)
GO(PK11_ExportEncryptedPrivKeyInfo, pFpuppip)
GO(PK11_ExportPrivateKeyInfo, pFpp)
GO(PK11_ExportPrivKeyInfo, pFpp)
GO(PK11_ExtractKeyValue, iFp)
GO(PK11_Finalize, iFp)
GO(PK11_FindBestKEAMatch, pFpp)
GO(PK11_FindCertAndKeyByRecipientList, pFppppp)
GO(PK11_FindCertAndKeyByRecipientListNew, iFpp)
GO(PK11_FindCertByIssuerAndSN, pFppp)
GO(PK11_FindCertFromDERCert, pFppp)
GO(PK11_FindCertFromDERCertItem, pFppp)
//GO(PK11_FindCertFromNickname, 
GO(PK11_FindCertFromURI, pFpp)
GO(PK11_FindCertInSlot, LFppp)
//GO(PK11_FindCertsFromEmailAddress, 
GO(PK11_FindCertsFromNickname, pFpp)
//GO(PK11_FindCertsFromURI, 
GO(PK11_FindFixedKey, pFpLpp)
GO(PK11_FindGenericObjects, pFpL)
GO(PK11_FindKeyByAnyCert, pFpp)
GO(PK11_FindKeyByDERCert, pFppp)
GO(PK11_FindKeyByKeyID, pFppp)
GO(PK11_FindPrivateKeyFromCert, pFppp)
GO(PK11_FindRawCertsWithSubject, iFppp)
//GO(PK11_FindSlotByName, 
GO(PK11_FindSlotsByNames, pFpppi)
GO(PK11_FortezzaHasKEA, iFp)
GO(PK11_FortezzaMapSig, uFu)
GO(PK11_FreeSlot, vFp)
GO(PK11_FreeSlotList, vFp)
GO(PK11_FreeSlotListElement, iFpp)
GO(PK11_FreeSymKey, vFp)
GO(PK11_GenerateFortezzaIV, iFppi)
GO(PK11_GenerateKeyPair, pFpLppiip)
GO(PK11_GenerateKeyPairWithFlags, pFpLppup)
GO(PK11_GenerateKeyPairWithOpFlags, pFpLppuLLp)
GO(PK11_GenerateNewParam, pFLp)
GO(PK11_GenerateRandom, iFpi)
GO(PK11_GenerateRandomOnSlot, iFppi)
GO(PK11_GetAllSlotsForCert, pFpp)
GO(PK11_GetAllTokens, pFLiip)
GO(PK11_GetBestKeyLength, iFpL)
GO(PK11_GetBestSlot, pFLp)
GO(PK11_GetBestSlotMultiple, pFpup)
GO(PK11_GetBestSlotMultipleWithAttributes, pFpppup)
GO(PK11_GetBestSlotWithAttributes, pFLLup)
GO(PK11_GetBestWrapMechanism, LFp)
GO(PK11_GetBlockSize, iFLp)
GO(PK11_GetCertFromPrivateKey, pFp)
GO(PK11_GetCertsMatchingPrivateKey, pFp)
GO(PK11_GetCurrentWrapIndex, iFp)
GO(PK11_GetDefaultArray, pFp)
GO(PK11_GetDefaultFlags, LFp)
GO(PK11_GetDisabledReason, uFp)
GO(PK11_GetFirstSafe, pFp)
GO(PK11_GetInternalKeySlot, pFv)
GO(PK11_GetInternalSlot, pFv)
//GO(PK11_GetIVLength, 
//GO(__PK11_GetKeyData, 
GO(PK11_GetKeyData, pFp)
//GO(PK11_GetKeyGen, 
GO(PK11_GetKeyLength, uFp)
GO(PK11_GetKeyStrength, uFpp)
GO(PK11_GetKeyType, LFLL)
GO(PK11_GetLowLevelKeyIDForCert, pFppp)
GO(PK11_GetLowLevelKeyIDForPrivateKey, pFp)
GO(PK11_GetMechanism, LFp)
//GO(PK11_GetMinimumPwdLength, 
//GO(PK11_GetModInfo, 
GO(PK11_GetModule, pFp)
GO(PK11_GetModuleID, LFp)
//GO(PK11_GetModuleURI, 
GO(PK11_GetNextGenericObject, pFp)
GO(PK11_GetNextSafe, pFppi)
GO(PK11_GetNextSymKey, pFp)
GO(PK11_GetPadMechanism, LFL)
GO(PK11_GetPBECryptoMechanism, LFppp)
GO(PK11_GetPBEIV, pFpp)
GO(PK11_GetPQGParamsFromPrivateKey, pFp)
GO(PK11_GetPrevGenericObject, pFp)
GO(PK11_GetPrivateKeyNickname, pFp)
GO(PK11_GetPrivateModulusLen, iFp)
GO(PK11_GetPublicKeyNickname, pFp)
GO(PK11_GetSlotFromKey, pFp)
GO(PK11_GetSlotFromPrivateKey, pFp)
GO(PK11_GetSlotID, LFp)
GO(PK11_GetSlotInfo, iFpp)
GO(PK11_GetSlotName, pFp)
GO(PK11_GetSlotPWValues, vFppp)
GO(PK11_GetSlotSeries, iFp)
GO(PK11_GetSymKeyHandle, LFp)
GO(PK11_GetSymKeyNickname, pFp)
GO(PK11_GetSymKeyType, LFp)
//GO(PK11_GetSymKeyUserData, 
GO(PK11_GetTokenInfo, iFpp)
GO(PK11_GetTokenName, pFp)
//GO(PK11_GetTokenURI, 
GO(PK11_GetWindow, pFp)
GO(PK11_GetWrapKey, pFpiLip)
GO(PK11_HasAttributeSet, CFpLLi)
GO(PK11_HashBuf, iFuppi)
GO(PK11_HasRootCerts, iFp)
GO(PK11_ImportCert, iFppLpi)
GO(PK11_ImportCertForKey, pFppp)
GO(PK11_ImportCertForKeyToSlot, iFpppip)
GO(PK11_ImportCRL, pFpppipipi)
GO(PK11_ImportDERCert, iFppLpi)
GO(PK11_ImportDERCertForKey, pFppp)
GO(PK11_ImportDERPrivateKeyInfo, iFppppiiup)
GO(PK11_ImportDERPrivateKeyInfoAndReturnKey, iFppppiiupp)
GO(PK11_ImportEncryptedPrivateKeyInfo, iFpppppiiuup)
GO(PK11_ImportEncryptedPrivateKeyInfoAndReturnKey, iFpppppiiuupp)
GO(PK11_ImportPrivateKeyInfo, iFppppiiup)
GO(PK11_ImportPrivateKeyInfoAndReturnKey, iFppppiiupp)
GO(PK11_ImportPublicKey, LFppi)
GO(PK11_ImportSymKey, pFpLuLpp)
GO(PK11_ImportSymKeyWithFlags, pFpLuLpLip)
GO(PK11_InitPin, iFppp)
//GO(PK11_IsDisabled, 
GO(PK11_IsFIPS, iFv)
GO(PK11_IsFriendly, iFp)
GO(PK11_IsHW, iFp)
GO(PK11_IsInternal, iFp)
GO(PK11_IsInternalKeySlot, iFp)
GO(PK11_IsLoggedIn, iFpp)
GO(PK11_IsPresent, iFp)
GO(PK11_IsReadOnly, iFp)
GO(PK11_IsRemovable, iFp)
GO(PK11_IVFromParam, pFLpp)
GO(PK11_KeyForCertExists, pFppp)
GO(PK11_KeyForDERCertExists, pFppp)
GO(PK11_KeyGen, pFpLpip)
GO(PK11_KeyGenWithTemplate, pFpLLppup)
GO(PK11_LinkGenericObject, iFpp)
GO(PK11_ListCerts, pFup)
GO(PK11_ListCertsInSlot, pFp)
GO(PK11_ListFixedKeysInSlot, pFppp)
GO(PK11_ListPrivateKeysInSlot, pFp)
GO(PK11_ListPrivKeysInSlot, pFppp)
GO(PK11_ListPublicKeysInSlot, pFpp)
GO(PK11_LoadPrivKey, pFpppii)
//GO(PK11_Logout, 
//GO(PK11_LogoutAll, 
GO(PK11_MakeIDFromPubKey, pFp)
GO(PK11_MakeKEAPubKey, pFpi)
GO(PK11_MapPBEMechanismToCryptoMechanism, LFpppi)
GO(PK11_MapSignKeyType, LFu)
GO(PK11_MechanismToAlgtag, uFL)
GO(PK11_MergeTokens, iFppppp)
GO(PK11_MoveSymKey, pFpLLip)
GO(PK11_NeedLogin, iFp)
//GO(PK11_NeedPWInit, 
GO(PK11_NeedUserInit, iFp)
GO(PK11_ParamFromAlgid, pFp)
GO(PK11_ParamFromIV, pFLp)
GO(PK11_ParamToAlgid, iFuppp)
GO(PK11_PBEKeyGen, pFpppip)
GO(PK11_PQG_DestroyParams, vFp)
GO(PK11_PQG_DestroyVerify, vFp)
GO(PK11_PQG_GetBaseFromParams, iFpp)
GO(PK11_PQG_GetCounterFromVerify, uFp)
GO(PK11_PQG_GetHFromVerify, iFpp)
//GO(PK11_PQG_GetPrimeFromParams, 
//GO(PK11_PQG_GetSeedFromVerify, 
//GO(PK11_PQG_GetSubPrimeFromParams, 
GO(PK11_PQG_NewParams, pFppp)
GO(PK11_PQG_NewVerify, pFupp)
GO(PK11_PQG_ParamGen, iFupp)
GO(PK11_PQG_ParamGenSeedLen, iFuupp)
GO(PK11_PQG_ParamGenV2, iFuuupp)
GO(PK11_PQG_VerifyParams, iFppp)
GO(PK11_PrivDecrypt, iFpLpppupu)
GO(PK11_PrivDecryptPKCS1, iFpppupu)
//GO(PK11_ProtectedAuthenticationPath, 
GO(PK11_PubDecryptRaw, iFpppupu)
GO(PK11_PubDerive, pFppippLLLip)
GO(PK11_PubDeriveWithKDF, pFppippLLLiLpp)
GO(PK11_PubEncrypt, iFpLpppupup)
GO(PK11_PubEncryptPKCS1, iFpppup)
GO(PK11_PubEncryptRaw, iFpppup)
GO(PK11_PubUnwrapSymKey, pFppLLi)
//GO(PK11_PubUnwrapSymKeyWithFlags, 
GO(PK11_PubUnwrapSymKeyWithFlagsPerm, pFppLLiLi)
GO(PK11_PubWrapSymKey, iFLppp)
GO(PK11_RandomUpdate, iFpL)
GO(PK11_RawPBEKeyGen, pFpLppip)
GO(PK11_ReadRawAttribute, iFipLp)
GO(PK11_ReferenceSlot, pFp)
GO(PK11_ReferenceSymKey, pFp)
//GO(PK11_ResetToken, 
GO(PK11_RestoreContext, iFppi)
GO(PK11_SaveContext, iFpppi)
GO(PK11_SaveContextAlloc, pFppup)
GO(PK11SDR_Decrypt, iFppp)
GO(PK11SDR_Encrypt, iFpppp)
//GO(PK11_SeedRandom, 
GO(__PK11_SetCertificateNickname, iFpp)
GO(PK11_SetFortezzaHack, vFp)
GOM(PK11_SetPasswordFunc, vFEp)
GO(PK11_SetPrivateKeyNickname, iFpp)
GO(PK11_SetPublicKeyNickname, iFpp)
GO(PK11_SetSlotPWValues, vFpii)
GO(PK11_SetSymKeyNickname, iFpp)
//GO(PK11_SetSymKeyUserData, 
GO(PK11_SetWrapKey, vFpip)
GO(PK11_Sign, iFppp)
GO(PK11_SignatureLen, iFp)
GO(PK11_SignWithMechanism, iFpLppp)
GO(PK11_SignWithSymKey, iFpLppp)
GO(PK11_SymKeyFromHandle, pFppuLLip)
GO(PK11_TokenExists, iFL)
GO(PK11_TokenKeyGen, pFpLpipip)
GO(PK11_TokenKeyGenWithFlags, pFpLpipLup)
//GO(PK11_TokenRefresh, 
//GO(PK11_TraverseCertsForNicknameInSlot, 
//GO(PK11_TraverseCertsForSubjectInSlot, 
//GO(PK11_TraverseSlotCerts, 
//GO(PK11_UnconfigurePKCS11, 
//GO(PK11_UnlinkGenericObject, 
GO(PK11_UnwrapPrivKey, pFppLppppiiLpip)
GO(PK11_UnwrapSymKey, pFpLppLLi)
GO(PK11_UnwrapSymKeyWithFlags, pFpLppLLiL)
GO(PK11_UnwrapSymKeyWithFlagsPerm, pFpLppLLiLi)
GO(PK11_UpdateSlotAttribute, iFppi)
//GO(PK11_UserDisableSlot, 
//GO(PK11_UserEnableSlot, 
GO(PK11_Verify, iFpppp)
GO(PK11_VerifyKeyOK, iFp)
GO(PK11_VerifyRecover, iFpppp)
GO(PK11_VerifyWithMechanism, iFpLpppp)
GO(PK11_WaitForTokenEvent, uFpuuui)
GO(PK11_WrapPrivKey, iFpppLppp)
GO(PK11_WrapSymKey, iFLpppp)
GO(PK11_WriteRawAttribute, iFupLp)
GO(PORT_Alloc, pFL)
GO(PORT_ArenaAlloc, pFpL)
GO(PORT_ArenaGrow, pFppLL)
GO(PORT_ArenaMark, pFp)
//GO(PORT_ArenaRelease, 
GO(PORT_ArenaStrdup, pFpp)
GO(PORT_ArenaUnmark, vFpp)
GO(PORT_ArenaZAlloc, pFpL)
GO(PORT_Free, vFp)
GO(PORT_FreeArena, vFpi)
GO(PORT_GetError, iFv)
GO(PORT_NewArena, pFL)
GO(PORT_Realloc, pFpL)
GO(PORT_SetError, vFi)
GOM(PORT_SetUCS2_ASCIIConversionFunction, vFEp)
GO(PORT_SetUCS2_UTF8ConversionFunction, vFi)
//GO(PORT_SetUCS4_UTF8ConversionFunction, 
GO(PORT_Strdup, pFp)
GO(PORT_UCS2_ASCIIConversion, iFipupupi)
GO(PORT_UCS2_UTF8Conversion, iFipupup)
GO(PORT_ZAlloc, pFL)
GO(PORT_ZFree, vFpL)
//GO(RSA_FormatBlock, 
DATA(SEC_AnyTemplate, 4*sizeof(void*))    //R type
GO(SEC_ASN1Decode, iFppppl)
GO(SEC_ASN1DecodeInteger, iFpp)
GO(SEC_ASN1DecodeItem, iFpppp)
GO(SEC_ASN1DecoderAbort, vFpi)
//GO(SEC_ASN1DecoderClearFilterProc, 
GO(SEC_ASN1DecoderClearNotifyProc, vFp)
GO(SEC_ASN1DecoderFinish, iFp)
//GO(SEC_ASN1DecoderSetFilterProc, 
//GO(SEC_ASN1DecoderSetNotifyProc, 
GO(SEC_ASN1DecoderStart, pFppp)
GO(SEC_ASN1DecoderUpdate, iFppL)
//GO(SEC_ASN1Encode, 
GO(SEC_ASN1EncodeInteger, pFppl)
GO(SEC_ASN1EncodeItem, pFpppp)
GO(SEC_ASN1EncoderAbort, vFpi)
//GO(SEC_ASN1EncoderClearNotifyProc, 
//GO(SEC_ASN1EncoderClearStreaming, 
GO(SEC_ASN1EncoderClearTakeFromBuf, vFp)
//GO(SEC_ASN1EncoderFinish, 
//GO(SEC_ASN1EncoderSetNotifyProc, 
//GO(SEC_ASN1EncoderSetStreaming, 
//GO(SEC_ASN1EncoderSetTakeFromBuf, 
//GO(SEC_ASN1EncoderStart, 
GO(SEC_ASN1EncoderUpdate, iFppL)
GO(SEC_ASN1EncodeUnsignedInteger, pFppL)
GO(SEC_ASN1LengthLength, iFL)
DATA(SEC_BitStringTemplate, 4*sizeof(void*)) //R type
DATA(SEC_BMPStringTemplate, 4*sizeof(void*)) //R type
//DATA(SEC_BooleanTemplate,     //R type
GO(SEC_CertNicknameConflict, iFppp)
GO(SEC_CheckCrlTimes, uFpl)
GO(SEC_CreateSignatureAlgorithmParameters, pFppuupp)
GO(SEC_DeletePermCertificate, iFp)
//GO(SEC_DeletePermCRL, 
GO(SEC_DerSignData, iFpppipu)
GO(SEC_DerSignDataWithAlgorithmID, iFpppipp)
GO(SEC_DestroyCrl, iFp)
GO(SEC_DupCrl, pFp)
GO(SEC_FindCrlByDERCert, pFppi)
//GO(SEC_FindCrlByName, 
//DATA(SEC_GeneralizedTimeTemplate,     //R type
GO(SEC_GetCrlTimes, iFppp)
//GO(SEC_GetRegisteredHttpClient, 
GO(SEC_GetSignatureAlgorithmOidTag, uFuu)
DATA(SEC_IA5StringTemplate, 4*sizeof(void*)) //R type
//DATA(SEC_IntegerTemplate,     //R type
GO(SECITEM_AllocItem, pFppu)
GO(SECITEM_ArenaDupItem, pFpp)
GO(SECITEM_CompareItem, iFpp)
GO(SECITEM_CopyItem, iFppp)
//GO(SECITEM_DupItem, 
GO(SECITEM_FreeItem, iFpi)
GO(SECITEM_ItemsAreEqual, iFpp)
GO(SECITEM_ZfreeItem, vFpi)
GO(SECKEY_AddPrivateKeyToListTail, iFpp)
GO(SECKEY_BigIntegerBitLength, uFp)
GO(SECKEY_CacheStaticFlags, iFp)
GO(SECKEY_ConvertToPublicKey, pFp)
GO(SECKEY_CopyEncryptedPrivateKeyInfo, iFppp)
GO(SECKEY_CopyPrivateKey, pFp)
GO(SECKEY_CopyPrivateKeyInfo, iFppp)
GO(SECKEY_CopyPublicKey, pFp)
GO(SECKEY_CopySubjectPublicKeyInfo, iFppp)
GO(SECKEY_CreateDHPrivateKey, pFppp)
GO(SECKEY_CreateECPrivateKey, pFppp)
GO(SECKEY_CreateRSAPrivateKey, pFipp)
GO(SECKEY_CreateSubjectPublicKeyInfo, pFp)
GO(SECKEY_DecodeDERSubjectPublicKeyInfo, pFp)
GO(SECKEY_DestroyEncryptedPrivateKeyInfo, vFpi)
GO(SECKEY_DestroyPrivateKey, vFp)
GO(SECKEY_DestroyPrivateKeyInfo, vFpi)
GO(SECKEY_DestroyPrivateKeyList, vFp)
GO(SECKEY_DestroyPublicKey, vFp)
GO(SECKEY_DestroyPublicKeyList, vFp)
GO(SECKEY_DestroySubjectPublicKeyInfo, vFp)
//DATA(SECKEY_DSAPublicKeyTemplate,     //R type
GO(SECKEY_ECParamsToBasePointOrderLen, iFp)
//GO(SECKEY_ECParamsToKeySize, 
GO(SECKEY_EncodeDERSubjectPublicKeyInfo, pFp)
DATA(SECKEY_EncryptedPrivateKeyInfoTemplate, 128)
GO(SECKEY_ExtractPublicKey, pFp)
GO(SECKEY_GetPrivateKeyType, uFp)
GO(SECKEY_GetPublicKeyType, uFp)
//GO(SECKEY_HashPassword, 
GO(SECKEY_ImportDERPublicKey, pFpL)
GO(SECKEY_NewPrivateKeyList, pFv)
//DATA(SECKEY_PointerToEncryptedPrivateKeyInfoTemplate, 
//DATA(SECKEY_PointerToPrivateKeyInfoTemplate, 
DATA(SECKEY_PrivateKeyInfoTemplate, 192)
GO(SECKEY_PublicKeyStrength, uFp)
GO(SECKEY_PublicKeyStrengthInBits, uFp)
GO(SECKEY_RemovePrivateKeyListNode, vFp)
//DATA(SECKEY_RSAPSSParamsTemplate, 
//DATA(SECKEY_RSAPublicKeyTemplate,     //R type
GO(SECKEY_SignatureLen, uFp)
GO(SECKEY_UpdateCertPQG, iFp)
GO(SEC_LookupCrls, iFppi)
GO(SECMOD_AddNewModule, iFppLL)
GO(SECMOD_AddNewModuleEx, iFppLLpp)
//GO(SECMOD_CancelWait, 
GO(SECMOD_CanDeleteInternalModule, iFv)
//GO(SECMOD_CloseUserDB, 
//GO(SECMOD_CreateModule, 
//GO(SECMOD_CreateModuleEx, 
//GO(SECMOD_DeleteInternalModule, 
GO(SECMOD_DeleteModule, iFpp)
//GO(SECMOD_DeleteModuleEx, 
GO(SECMOD_DestroyModule, vFp)
//GO(SECMOD_FindModule, 
//GO(SECMOD_FindSlot, 
//GO(SECMOD_FreeModuleSpecList, 
GO(SECMOD_GetDBModuleList, pFv)
//GO(SECMOD_GetDeadModuleList, 
//GO(SECMOD_GetDefaultModDBFlag, 
GO(SECMOD_GetDefaultModuleList, pFv)
GO(SECMOD_GetDefaultModuleListLock, pFv)
//GO(SECMOD_GetInternalModule, 
//GO(SECMOD_GetModuleSpecList, 
GO(SECMOD_GetReadLock, vFp)
//GO(SECMOD_GetSkipFirstFlag, 
//GO(SECMOD_HasRemovableSlots, 
//GO(SECMOD_HasRootCerts, 
//GO(SECMOD_InternaltoPubMechFlags, 
GO(SECMOD_IsModulePresent, iFL)
//GO(SECMOD_LoadModule, 
GO(SECMOD_LoadUserModule, pFppi)
GO(SECMOD_LookupSlot, pFLL)
//GO(SECMOD_OpenNewSlot, 
GO(SECMOD_OpenUserDB, pFp)
GO(SECMOD_PubCipherFlagstoInternal, LFL)
//GO(SECMOD_PubMechFlagstoInternal, 
//GO(SECMOD_ReferenceModule, 
GO(SECMOD_ReleaseReadLock, vFp)
//GO(SECMOD_RestartModules, 
//GO(SECMOD_UnloadUserModule, 
//GO(SECMOD_UpdateModule, 
//GO(SECMOD_UpdateSlotList, 
//GO(SECMOD_WaitForAnyTokenEvent, 
GO(SEC_NewCrl, pFpppi)
//DATA(SEC_NullTemplate,    //R type
//DATA(SEC_ObjectIDTemplate,    //R type
DATA(SEC_OctetStringTemplate, 4*sizeof(void*))   //R type
GO(SECOID_AddEntry, uFp)
DATA(SECOID_AlgorithmIDTemplate, 16*sizeof(void*))   //R type
GO(SECOID_CompareAlgorithmID, iFpp)
GO(SECOID_CopyAlgorithmID, iFppp)
GO(SECOID_DestroyAlgorithmID, vFpi)
GO(SECOID_FindOID, pFp)
GO(SECOID_FindOIDByTag, pFu)
GO(SECOID_FindOIDTag, uFp)
GO(SECOID_FindOIDTagDescription, pFu)
GO(SECOID_GetAlgorithmTag, uFp)
GO(SECOID_SetAlgorithmID, iFppup)
GO(SEC_PKCS5GetCryptoAlgorithm, uFp)
GO(SEC_PKCS5GetIV, pFppi)
GO(SEC_PKCS5GetKeyLength, iFp)
GO(SEC_PKCS5GetPBEAlgorithm, uFui)
GO(SEC_PKCS5IsAlgorithmPBEAlg, iFp)
GO(SEC_PKCS5IsAlgorithmPBEAlgTag, iFu)
//DATA(SEC_PointerToAnyTemplate, 
//DATA(SEC_PointerToOctetStringTemplate, 
GO(SEC_QuickDERDecodeItem, iFpppp)
GO(SEC_RegisterDefaultHttpClient, iFp)
//DATA(SEC_SetOfAnyTemplate, 
GO(SEC_SignData, iFppipu)
GO(SEC_SignDataWithAlgorithmID, iFppipp)
//DATA(SEC_SignedCertificateTemplate, 
//DATA(SEC_UTCTimeTemplate,     //R type
DATA(SEC_UTF8StringTemplate, 4*sizeof(void*))    //R type
GO(SGN_Begin, iFp)
GO(SGN_CompareDigestInfo, iFpp)
GO(SGN_CopyDigestInfo, iFppp)
GO(SGN_CreateDigestInfo, pFupu)
GO(SGN_DestroyContext, vFpi)
GO(SGN_DestroyDigestInfo, vFp)
GO(SGN_Digest, iFpupp)
//DATA(sgn_DigestInfoTemplate, 
GO(SGN_End, iFpp)
GO(SGN_NewContext, pFup)
GO(SGN_NewContextWithAlgorithmID, pFpp)
GO(SGN_Update, iFppu)
GO(VFY_Begin, iFp)
GO(VFY_CreateContext, pFppup)
GO(VFY_CreateContextDirect, pFppuupp)
GO(VFY_CreateContextWithAlgorithmID, pFppppp)
GO(VFY_DestroyContext, vFpi)
GO(VFY_End, iFp)
GO(VFY_EndWithSignature, iFpp)
GO(VFY_Update, iFppu)
GO(VFY_VerifyData, iFpippup)
GO(VFY_VerifyDataDirect, iFpippuupp)
GO(VFY_VerifyDataWithAlgorithmID, iFpippppp)
GO(VFY_VerifyDigest, iFpppup)
GO(VFY_VerifyDigestDirect, iFpppuup)
GO(VFY_VerifyDigestWithAlgorithmID, iFppppup)
