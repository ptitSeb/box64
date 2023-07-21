#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

// CK_MECHANISM_TYPE is tyoe ULong
// CK_OBJECT_HANDLE is type ULong
// CK_ATTRIBUTE_TYPE is type ULong
// SECOidTag is an enum

//GO(ATOB_AsciiToData, 
//GO(ATOB_ConvertAsciiToItem, 
//GO(BTOA_ConvertItemToAscii, 
//GO(BTOA_DataToAscii, 
//GO(CERT_AddCertToListHead, 
//GO(CERT_AddCertToListSorted, 
GO(CERT_AddCertToListTail, iFpp)
//GO(CERT_AddExtension, 
//GO(CERT_AddExtensionByOID, 
//GO(CERT_AddOCSPAcceptableResponses, 
//GO(CERT_AddOKDomainName, 
//GO(CERT_AddRDN, 
//GO(__CERT_AddTempCertToPerm, 
//GO(CERT_AllocCERTRevocationFlags, 
//GO(CERT_AsciiToName, 
//GO(CERT_CacheCRL, 
//GO(CERT_CacheOCSPResponseFromSideChannel, 
//GO(CERT_CertChainFromCert, 
//DATA(CERT_CertificateRequestTemplate, 
DATA(CERT_CertificateTemplate, 480)
//GO(CERT_CertListFromCert, 
//GO(CERT_CertTimesValid, 
GO(CERT_ChangeCertTrust, iFppp)
//GO(CERT_CheckCertUsage, 
GO(CERT_CheckCertValidTimes, iFpIi)
//GO(CERT_CheckNameSpace, 
//GO(CERT_CheckOCSPStatus, 
//GO(CERT_ClearOCSPCache, 
//GO(__CERT_ClosePermCertDB, 
//GO(CERT_CompareAVA, 
GO(CERT_CompareCerts, iFpp)
GO(CERT_CompareName, iFpp)
//GO(CERT_CompareValidityTimes, 
//GO(CERT_CompleteCRLDecodeEntries, 
//GO(CERT_CopyName, 
//GO(CERT_CopyRDN, 
//GO(CERT_CreateAVA, 
//GO(CERT_CreateCertificate, 
//GO(CERT_CreateCertificateRequest, 
//GO(CERT_CreateEncodedOCSPErrorResponse, 
//GO(CERT_CreateEncodedOCSPSuccessResponse, 
//GO(CERT_CreateName, 
//GO(CERT_CreateOCSPCertID, 
//GO(CERT_CreateOCSPRequest, 
//GO(CERT_CreateOCSPSingleResponseGood, 
//GO(CERT_CreateOCSPSingleResponseRevoked, 
//GO(CERT_CreateOCSPSingleResponseUnknown, 
//GO(CERT_CreateRDN, 
GO(CERT_CreateSubjectCertList, pFpppIi)
GO(CERT_CreateValidity, pFII)
//GO(CERT_CRLCacheRefreshIssuer, 
//DATA(CERT_CrlTemplate, 
GO(CERT_DecodeAltNameExtension, pFpp)
GO(CERT_DecodeAuthInfoAccessExtension, pFpp)
GO(CERT_DecodeAuthKeyID, pFpp)
GO(CERT_DecodeAVAValue, pFp)
GO(CERT_DecodeBasicConstraintValue, iFpp)
GO(CERT_DecodeCertificatePoliciesExtension, pFp)
GO(CERT_DecodeCRLDistributionPoints, pFpp)
//GO(__CERT_DecodeDERCertificate, 
//GO(CERT_DecodeDERCrl, 
//GO(CERT_DecodeDERCrlWithFlags, 
//GO(CERT_DecodeGeneralName, 
//GO(CERT_DecodeNameConstraintsExtension, 
//GO(CERT_DecodeOCSPRequest, 
//GO(CERT_DecodeOCSPResponse, 
GO(CERT_DecodeOidSequence, pFp)
//GO(CERT_DecodePrivKeyUsagePeriodExtension, 
//GO(CERT_DecodeTrustString, 
GO(CERT_DecodeUserNotice, pFp)
//GO(CERT_DerNameToAscii, 
//GO(CERT_DestroyCertArray, 
GO(CERT_DestroyCertificate, vFp)
GO(CERT_DestroyCertificateList, vFp)
GO(CERT_DestroyCertificatePoliciesExtension, vFp)
GO(CERT_DestroyCertificateRequest, vFp)
GO(CERT_DestroyCertList, vFp)
//GO(CERT_DestroyCERTRevocationFlags, 
GO(CERT_DestroyName, vFp)
//GO(CERT_DestroyOCSPCertID, 
//GO(CERT_DestroyOCSPRequest, 
//GO(CERT_DestroyOCSPResponse, 
GO(CERT_DestroyOidSequence, vFp)
GO(CERT_DestroyUserNotice, vFp)
GO(CERT_DestroyValidity, vFp)
//GO(CERT_DisableOCSPChecking, 
//GO(CERT_DisableOCSPDefaultResponder, 
//GO(CERT_DistNamesFromCertList, 
GO(CERT_DupCertificate, pFp)
//GO(CERT_DupCertList, 
//GO(CERT_DupDistNames, 
//GO(CERT_EnableOCSPChecking, 
//GO(CERT_EnableOCSPDefaultResponder, 
//GO(CERT_EncodeAltNameExtension, 
//GO(CERT_EncodeAndAddBitStrExtension, 
//GO(CERT_EncodeAuthKeyID, 
//GO(CERT_EncodeBasicConstraintValue, 
//GO(CERT_EncodeCertPoliciesExtension, 
//GO(CERT_EncodeCRLDistributionPoints, 
//GO(CERT_EncodeGeneralName, 
//GO(CERT_EncodeInfoAccessExtension, 
//GO(CERT_EncodeInhibitAnyExtension, 
//GO(CERT_EncodeNameConstraintsExtension, 
//GO(CERT_EncodeNoticeReference, 
//GO(CERT_EncodeOCSPRequest, 
//GO(CERT_EncodePolicyConstraintsExtension, 
//GO(CERT_EncodePolicyMappingExtension, 
//GO(CERT_EncodeSubjectKeyID, 
//GO(CERT_EncodeUserNotice, 
GO(CERT_ExtractPublicKey, pFp)
//GO(CERT_FilterCertListByCANames, 
//GO(CERT_FilterCertListByUsage, 
//GO(CERT_FilterCertListForUserCerts, 
GO(CERT_FindCertByDERCert, pFpp)
//GO(CERT_FindCertByIssuerAndSN, 
//GO(CERT_FindCertByIssuerAndSNCX, 
GO(CERT_FindCertByName, pFpp)
//GO(CERT_FindCertByNickname, 
//GO(CERT_FindCertByNicknameOrEmailAddr, 
//GO(CERT_FindCertByNicknameOrEmailAddrCX, 
//GO(CERT_FindCertByNicknameOrEmailAddrForUsage, 
//GO(CERT_FindCertByNicknameOrEmailAddrForUsageCX, 
//GO(CERT_FindCertBySubjectKeyID, 
GO(CERT_FindCertExtension, iFppp)
//GO(CERT_FindCertIssuer, 
//GO(CERT_FindCRLEntryReasonExten, 
//GO(CERT_FindCRLNumberExten, 
//GO(CERT_FindKeyUsageExtension, 
//GO(CERT_FindNameConstraintsExten, 
//GO(CERT_FindSMimeProfile, 
//GO(CERT_FindSubjectKeyIDExtension, 
//GO(CERT_FindUserCertByUsage, 
GO(CERT_FindUserCertsByUsage, pFpiiip)
//GO(CERT_FinishCertificateRequestAttributes, 
//GO(CERT_FinishExtensions, 
//GO(CERT_ForcePostMethodForOCSP, 
//GO(CERT_FormatName, 
//GO(CERT_FreeDistNames, 
//GO(CERT_FreeNicknames, 
//GO(CERT_GenTime2FormattedAscii, 
GO(CERT_GetAVATag, iFp)
//GO(CERT_GetCertChainFromCert, 
GO(CERT_GetCertEmailAddress, pFp)
//GO(CERT_GetCertificateDer, 
//GO(CERT_GetCertificateNames, 
//GO(CERT_GetCertificateRequestExtensions, 
//GO(CERT_GetCertIsPerm, 
//GO(CERT_GetCertIssuerAndSN, 
//GO(CERT_GetCertIsTemp, 
//GO(CERT_GetCertKeyType, 
//GO(CERT_GetCertNicknames, 
GO(CERT_GetCertTimes, iFppp)
GO(CERT_GetCertTrust, iFpp)
GO(CERT_GetCertUid, pFp)
//GO(CERT_GetClassicOCSPDisabledPolicy, 
//GO(CERT_GetClassicOCSPEnabledHardFailurePolicy, 
//GO(CERT_GetClassicOCSPEnabledSoftFailurePolicy, 
GO(CERT_GetCommonName, pFp)
//GO(CERT_GetConstrainedCertificateNames, 
GO(CERT_GetCountryName, pFp)
//GO(CERT_GetDBContentVersion, 
GO(CERT_GetDefaultCertDB, pFv)
GO(CERT_GetDomainComponentName, pFp)
//GO(CERT_GetEncodedOCSPResponse, 
GO(CERT_GetFirstEmailAddress, pFp)
//GO(CERT_GetGeneralNameTypeFromString, 
//GO(CERT_GetImposedNameConstraints, 
GO(CERT_GetLocalityName, pFp)
//GO(CERT_GetNextEmailAddress, 
GO(CERT_GetNextGeneralName, pFp)
//GO(CERT_GetNextNameConstraint, 
//GO(CERT_GetOCSPAuthorityInfoAccessLocation, 
//GO(CERT_GetOCSPResponseStatus, 
//GO(CERT_GetOCSPStatusForCertID, 
GO(CERT_GetOidString, pFp)
GO(CERT_GetOrgName, pFp)
GO(CERT_GetOrgUnitName, pFp)
//GO(CERT_GetPKIXVerifyNistRevocationPolicy, 
//GO(CERT_GetPrevGeneralName, 
//GO(CERT_GetPrevNameConstraint, 
//GO(CERT_GetSlopTime, 
//GO(CERT_GetSSLCACerts, 
GO(CERT_GetStateName, pFp)
//GO(CERT_GetSubjectNameDigest, 
//GO(CERT_GetSubjectPublicKeyDigest, 
//GO(CERT_GetUsePKIXForValidation, 
//GO(CERT_GetValidDNSPatternsFromCert, 
GO(CERT_Hexify, pFpi)
//GO(CERT_ImportCAChain, 
//GO(CERT_ImportCAChainTrusted, 
//GO(CERT_ImportCerts, 
//GO(CERT_ImportCRL, 
GO(CERT_IsCACert, iFpp)
//GO(CERT_IsCADERCert, 
//GO(CERT_IsRootDERCert, 
//DATA(CERT_IssuerAndSNTemplate, 
//GO(CERT_IsUserCert, 
//GO(CERT_KeyFromDERCrl, 
GO(CERT_MakeCANickname, pFp)
//GO(CERT_MergeExtensions, 
DATA(CERT_NameTemplate, 4*sizeof(void*))
//GO(CERT_NameToAscii, 
//GO(CERT_NameToAsciiInvertible, 
GO(CERT_NewCertList, pFv)
//GO(__CERT_NewTempCertificate, 
GO(CERT_NewTempCertificate, pFpppii)
//GO(CERT_NicknameStringsFromCertList, 
//GO(CERT_OCSPCacheSettings, 
//GO(CERT_OpenCertDBFilename, 
GOM(CERT_PKIXVerifyCert, iFEpIppp)
//GO(CERT_PostOCSPRequest, 
GOM(CERT_RegisterAlternateOCSPAIAInfoCallBack, iFEpp)
//GO(CERT_RemoveCertListNode, 
//GO(CERT_RFC1485_EscapeAndQuote, 
//GO(CERT_SaveSMimeProfile, 
//DATA(CERT_SequenceOfCertExtensionTemplate, 
//GO(CERT_SetOCSPDefaultResponder, 
//GO(CERT_SetOCSPFailureMode, 
//GO(CERT_SetOCSPTimeout, 
//DATA(CERT_SetOfSignedCrlTemplate, 
//GO(CERT_SetSlopTime, 
//GO(CERT_SetUsePKIXForValidation, 
//DATA(CERT_SignedCrlTemplate, 
DATA(CERT_SignedDataTemplate, 160)
//GO(CERT_StartCertExtensions, 
//GO(CERT_StartCertificateRequestAttributes, 
//GO(CERT_StartCRLEntryExtensions, 
//GO(CERT_StartCRLExtensions, 
//DATA(CERT_SubjectPublicKeyInfoTemplate, 
//DATA(CERT_TimeChoiceTemplate,     //R type
//GO(__CERT_TraversePermCertsForNickname, 
//GO(__CERT_TraversePermCertsForSubject, 
//GO(CERT_UncacheCRL, 
//GO(CERT_VerifyCACertForUsage, 
GO(CERT_VerifyCert, iFpp)
//GO(CERT_VerifyCertificate, 
GO(CERT_VerifyCertificateNow, iFppiipp)
//GO(CERT_VerifyCertName, 
//GO(CERT_VerifyCertNow, 
//GO(CERT_VerifyOCSPResponseSignature, 
//GO(CERT_VerifySignedData, 
GO(CERT_VerifySignedDataWithPublicKey, iFppp)
//GO(CERT_VerifySignedDataWithPublicKeyInfo, 
//GO(DER_AsciiToTime, 
GO(DER_DecodeTimeChoice, iFpp)
GO(DER_Encode, iFpppp)
//GO(DER_EncodeTimeChoice, 
//GO(DER_GeneralizedDayToAscii, 
//GO(DER_GeneralizedTimeToTime, 
//GO(DER_GetInteger, 
//GO(DER_Lengths, 
//GO(DER_TimeChoiceDayToAscii, 
//GO(DER_TimeToGeneralizedTime, 
//GO(DER_TimeToGeneralizedTimeArena, 
//GO(DER_TimeToUTCTime, 
//GO(DER_UTCDayToAscii, 
//GO(DER_UTCTimeToAscii, 
//GO(DER_UTCTimeToTime, 
//GO(DSAU_DecodeDerSig, 
//GO(DSAU_DecodeDerSigToLen, 
//GO(DSAU_EncodeDerSig, 
//GO(DSAU_EncodeDerSigWithLen, 
GO(HASH_Begin, vFp)
//GO(HASH_Clone, 
GO(HASH_Create, pFi)
GO(HASH_Destroy, vFp)
GO(HASH_End, vFpppu)
//GO(HASH_GetHashObject, 
//GO(HASH_GetHashObjectByOidTag, 
//GO(HASH_GetHashOidTagByHashType, 
//GO(HASH_GetHashTypeByOidTag, 
//GO(HASH_GetType, 
GO(HASH_HashBuf, iFippu)
//GO(HASH_ResultLen, 
//GO(HASH_ResultLenByOidTag, 
GO(HASH_ResultLenContext, uFp)
GO(HASH_Update, vFppu)
//GO(NSSBase64_DecodeBuffer, 
//GO(NSSBase64Decoder_Create, 
//GO(NSSBase64Decoder_Destroy, 
//GO(NSSBase64Decoder_Update, 
//GO(NSSBase64_EncodeItem, 
//GO(NSSBase64Encoder_Create, 
//GO(NSSBase64Encoder_Destroy, 
//GO(NSSBase64Encoder_Update, 
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
//GO(NSS_Get_SECKEY_RSAPSSParamsTemplate, 
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
//GO(NSS_Get_sgn_DigestInfoTemplate, 
//GO(NSS_GetVersion, 
//GO(NSS_Init, 
//GO(NSS_InitContext, 
//GO(NSS_Initialize, 
//GO(__nss_InitLock, 
GO(NSS_InitReadWrite, iFp)
//GO(NSS_InitWithMerge, 
GO(NSS_IsInitialized, iFv)
GO(NSS_NoDB_Init, iFp)
//GO(NSS_OptionGet, 
//GO(NSS_OptionSet, 
//GO(NSS_PutEnv, 
//GO(NSS_RegisterShutdown, 
//GO(NSSRWLock_Destroy, 
//GO(NSSRWLock_HaveWriteLock, 
//GO(NSSRWLock_LockRead, 
//GO(NSSRWLock_LockWrite, 
//GO(NSSRWLock_New, 
//GO(NSSRWLock_UnlockRead, 
//GO(NSSRWLock_UnlockWrite, 
//GO(NSS_Shutdown, 
//GO(NSS_ShutdownContext, 
//GO(NSS_UnregisterShutdown, 
GO(NSS_VersionCheck, iFp)
//GO(__PBE_CreateContext, 
//GO(PBE_CreateContext, 
//GO(__PBE_DestroyContext, 
//GO(PBE_DestroyContext, 
//GO(__PBE_GenerateBits, 
//GO(PBE_GenerateBits, 
//GO(PK11_AlgtagToMechanism, 
GO(PK11_Authenticate, iFpip)
//GO(PK11_BlockData, 
//GO(PK11_ChangePW, 
//GO(PK11_CheckSSOPassword, 
GO(PK11_CheckUserPassword, iFpp)
GO(PK11_CipherOp, iFpppipi)
//GO(PK11_CloneContext, 
//GO(PK11_ConfigurePKCS11, 
//GO(PK11_ConvertSessionPrivKeyToTokenPrivKey, 
//GO(PK11_ConvertSessionSymKeyToTokenSymKey, 
//GO(PK11_CopySymKeyForSigning, 
//GO(PK11_CopyTokenPrivKeyToSessionPrivKey, 
//GO(__PK11_CreateContextByRawKey, 
GO(PK11_CreateContextBySymKey, pFiipp)
//GO(PK11_CreateDigestContext, 
//GO(PK11_CreateGenericObject, 
//GO(PK11_CreateManagedGenericObject, 
//GO(PK11_CreateMergeLog, 
//GO(PK11_CreatePBEAlgorithmID, 
//GO(PK11_CreatePBEParams, 
//GO(PK11_CreatePBEV2AlgorithmID, 
//GO(PK11_Decrypt, 
GO(PK11_DeleteTokenCertAndKey, iFpp)
//GO(PK11_DeleteTokenPrivateKey, 
//GO(PK11_DeleteTokenPublicKey, 
//GO(PK11_DeleteTokenSymKey, 
//GO(PK11_DEREncodePublicKey, 
//GO(PK11_Derive, 
//GO(PK11_DeriveWithFlags, 
//GO(PK11_DeriveWithFlagsPerm, 
//GO(PK11_DeriveWithTemplate, 
GO(PK11_DestroyContext, vFpi)
//GO(PK11_DestroyGenericObject, 
//GO(PK11_DestroyGenericObjects, 
//GO(PK11_DestroyMergeLog, 
GO(PK11_DestroyObject, iFpL)
//GO(PK11_DestroyPBEParams, 
GO(PK11_DestroyTokenObject, iFpL)
//GO(PK11_DigestBegin, 
//GO(PK11_DigestFinal, 
//GO(PK11_DigestKey, 
//GO(PK11_DigestOp, 
GO(PK11_DoesMechanism, iFpL)
//GO(PK11_Encrypt, 
//GO(PK11_ExportDERPrivateKeyInfo, 
//GO(PK11_ExportEncryptedPrivateKeyInfo, 
//GO(PK11_ExportEncryptedPrivKeyInfo, 
//GO(PK11_ExportPrivateKeyInfo, 
//GO(PK11_ExportPrivKeyInfo, 
//GO(PK11_ExtractKeyValue, 
GO(PK11_Finalize, iFp)
//GO(PK11_FindBestKEAMatch, 
//GO(PK11_FindCertAndKeyByRecipientList, 
//GO(PK11_FindCertAndKeyByRecipientListNew, 
//GO(PK11_FindCertByIssuerAndSN, 
//GO(PK11_FindCertFromDERCert, 
GO(PK11_FindCertFromDERCertItem, pFppp)
//GO(PK11_FindCertFromNickname, 
//GO(PK11_FindCertFromURI, 
GO(PK11_FindCertInSlot, LFppp)
//GO(PK11_FindCertsFromEmailAddress, 
//GO(PK11_FindCertsFromNickname, 
//GO(PK11_FindCertsFromURI, 
GO(PK11_FindFixedKey, pFpipp)
//GO(PK11_FindGenericObjects, 
GO(PK11_FindKeyByAnyCert, pFpp)
GO(PK11_FindKeyByDERCert, pFppp)
GO(PK11_FindKeyByKeyID, pFppp)
GO(PK11_FindPrivateKeyFromCert, pFppp)
//GO(PK11_FindRawCertsWithSubject, 
//GO(PK11_FindSlotByName, 
//GO(PK11_FindSlotsByNames, 
//GO(PK11_FortezzaHasKEA, 
//GO(PK11_FortezzaMapSig, 
GO(PK11_FreeSlot, vFp)
GO(PK11_FreeSlotList, vFp)
GO(PK11_FreeSlotListElement, iFpp)
GO(PK11_FreeSymKey, vFp)
//GO(PK11_GenerateFortezzaIV, 
GO(PK11_GenerateKeyPair, pFpLppiip)
//GO(PK11_GenerateKeyPairWithFlags, 
//GO(PK11_GenerateKeyPairWithOpFlags, 
//GO(PK11_GenerateNewParam, 
//GO(PK11_GenerateRandom, 
//GO(PK11_GenerateRandomOnSlot, 
GO(PK11_GetAllSlotsForCert, pFpp)
GO(PK11_GetAllTokens, pFLiip)
//GO(PK11_GetBestKeyLength, 
//GO(PK11_GetBestSlot, 
//GO(PK11_GetBestSlotMultiple, 
//GO(PK11_GetBestSlotMultipleWithAttributes, 
//GO(PK11_GetBestSlotWithAttributes, 
//GO(PK11_GetBestWrapMechanism, 
GO(PK11_GetBlockSize, iFLp)
//GO(PK11_GetCertFromPrivateKey, 
//GO(PK11_GetCertsMatchingPrivateKey, 
//GO(PK11_GetCurrentWrapIndex, 
//GO(PK11_GetDefaultArray, 
//GO(PK11_GetDefaultFlags, 
//GO(PK11_GetDisabledReason, 
GO(PK11_GetFirstSafe, pFp)
GO(PK11_GetInternalKeySlot, pFv)
//GO(PK11_GetInternalSlot, 
//GO(PK11_GetIVLength, 
//GO(__PK11_GetKeyData, 
//GO(PK11_GetKeyData, 
//GO(PK11_GetKeyGen, 
//GO(PK11_GetKeyLength, 
//GO(PK11_GetKeyStrength, 
//GO(PK11_GetKeyType, 
//GO(PK11_GetLowLevelKeyIDForCert, 
//GO(PK11_GetLowLevelKeyIDForPrivateKey, 
//GO(PK11_GetMechanism, 
//GO(PK11_GetMinimumPwdLength, 
//GO(PK11_GetModInfo, 
GO(PK11_GetModule, pFp)
//GO(PK11_GetModuleID, 
//GO(PK11_GetModuleURI, 
//GO(PK11_GetNextGenericObject, 
GO(PK11_GetNextSafe, pFppi)
GO(PK11_GetNextSymKey, pFp)
//GO(PK11_GetPadMechanism, 
//GO(PK11_GetPBECryptoMechanism, 
//GO(PK11_GetPBEIV, 
//GO(PK11_GetPQGParamsFromPrivateKey, 
//GO(PK11_GetPrevGenericObject, 
GO(PK11_GetPrivateKeyNickname, pFp)
//GO(PK11_GetPrivateModulusLen, 
//GO(PK11_GetPublicKeyNickname, 
//GO(PK11_GetSlotFromKey, 
//GO(PK11_GetSlotFromPrivateKey, 
//GO(PK11_GetSlotID, 
//GO(PK11_GetSlotInfo, 
GO(PK11_GetSlotName, pFp)
//GO(PK11_GetSlotPWValues, 
//GO(PK11_GetSlotSeries, 
//GO(PK11_GetSymKeyHandle, 
//GO(PK11_GetSymKeyNickname, 
//GO(PK11_GetSymKeyType, 
//GO(PK11_GetSymKeyUserData, 
//GO(PK11_GetTokenInfo, 
GO(PK11_GetTokenName, pFp)
//GO(PK11_GetTokenURI, 
//GO(PK11_GetWindow, 
//GO(PK11_GetWrapKey, 
//GO(PK11_HasAttributeSet, 
//GO(PK11_HashBuf, 
GO(PK11_HasRootCerts, iFp)
GO(PK11_ImportCert, iFppLpi)
//GO(PK11_ImportCertForKey, 
//GO(PK11_ImportCertForKeyToSlot, 
//GO(PK11_ImportCRL, 
//GO(PK11_ImportDERCert, 
//GO(PK11_ImportDERCertForKey, 
//GO(PK11_ImportDERPrivateKeyInfo, 
GO(PK11_ImportDERPrivateKeyInfoAndReturnKey, iFppppiiupp)
//GO(PK11_ImportEncryptedPrivateKeyInfo, 
//GO(PK11_ImportEncryptedPrivateKeyInfoAndReturnKey, 
//GO(PK11_ImportPrivateKeyInfo, 
//GO(PK11_ImportPrivateKeyInfoAndReturnKey, 
//GO(PK11_ImportPublicKey, 
//GO(PK11_ImportSymKey, 
//GO(PK11_ImportSymKeyWithFlags, 
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
//GO(PK11_IVFromParam, 
GO(PK11_KeyForCertExists, pFppp)
//GO(PK11_KeyForDERCertExists, 
//GO(PK11_KeyGen, 
//GO(PK11_KeyGenWithTemplate, 
//GO(PK11_LinkGenericObject, 
GO(PK11_ListCerts, pFip)
GO(PK11_ListCertsInSlot, pFp)
GO(PK11_ListFixedKeysInSlot, pFppp)
//GO(PK11_ListPrivateKeysInSlot, 
//GO(PK11_ListPrivKeysInSlot, 
//GO(PK11_ListPublicKeysInSlot, 
//GO(PK11_LoadPrivKey, 
//GO(PK11_Logout, 
//GO(PK11_LogoutAll, 
GO(PK11_MakeIDFromPubKey, pFp)
//GO(PK11_MakeKEAPubKey, 
//GO(PK11_MapPBEMechanismToCryptoMechanism, 
GO(PK11_MapSignKeyType, LFi)
//GO(PK11_MechanismToAlgtag, 
//GO(PK11_MergeTokens, 
//GO(PK11_MoveSymKey, 
GO(PK11_NeedLogin, iFp)
//GO(PK11_NeedPWInit, 
GO(PK11_NeedUserInit, iFp)
GO(PK11_ParamFromAlgid, pFp)
//GO(PK11_ParamFromIV, 
//GO(PK11_ParamToAlgid, 
//GO(PK11_PBEKeyGen, 
//GO(PK11_PQG_DestroyParams, 
//GO(PK11_PQG_DestroyVerify, 
//GO(PK11_PQG_GetBaseFromParams, 
//GO(PK11_PQG_GetCounterFromVerify, 
//GO(PK11_PQG_GetHFromVerify, 
//GO(PK11_PQG_GetPrimeFromParams, 
//GO(PK11_PQG_GetSeedFromVerify, 
//GO(PK11_PQG_GetSubPrimeFromParams, 
//GO(PK11_PQG_NewParams, 
//GO(PK11_PQG_NewVerify, 
//GO(PK11_PQG_ParamGen, 
//GO(PK11_PQG_ParamGenSeedLen, 
//GO(PK11_PQG_ParamGenV2, 
//GO(PK11_PQG_VerifyParams, 
//GO(PK11_PrivDecrypt, 
//GO(PK11_PrivDecryptPKCS1, 
//GO(PK11_ProtectedAuthenticationPath, 
//GO(PK11_PubDecryptRaw, 
//GO(PK11_PubDerive, 
//GO(PK11_PubDeriveWithKDF, 
//GO(PK11_PubEncrypt, 
//GO(PK11_PubEncryptPKCS1, 
//GO(PK11_PubEncryptRaw, 
//GO(PK11_PubUnwrapSymKey, 
//GO(PK11_PubUnwrapSymKeyWithFlags, 
//GO(PK11_PubUnwrapSymKeyWithFlagsPerm, 
//GO(PK11_PubWrapSymKey, 
//GO(PK11_RandomUpdate, 
//GO(PK11_RawPBEKeyGen, 
GO(PK11_ReadRawAttribute, iFipLp)
GO(PK11_ReferenceSlot, pFp)
//GO(PK11_ReferenceSymKey, 
//GO(PK11_ResetToken, 
//GO(PK11_RestoreContext, 
//GO(PK11_SaveContext, 
//GO(PK11_SaveContextAlloc, 
//GO(PK11SDR_Decrypt, 
//GO(PK11SDR_Encrypt, 
//GO(PK11_SeedRandom, 
//GO(__PK11_SetCertificateNickname, 
//GO(PK11_SetFortezzaHack, 
GOM(PK11_SetPasswordFunc, vFEp)
GO(PK11_SetPrivateKeyNickname, iFpp)
GO(PK11_SetPublicKeyNickname, iFpp)
//GO(PK11_SetSlotPWValues, 
//GO(PK11_SetSymKeyNickname, 
//GO(PK11_SetSymKeyUserData, 
//GO(PK11_SetWrapKey, 
GO(PK11_Sign, iFppp)
GO(PK11_SignatureLen, iFp)
GO(PK11_SignWithMechanism, iFpLppp)
//GO(PK11_SignWithSymKey, 
//GO(PK11_SymKeyFromHandle, 
GO(PK11_TokenExists, iFi)
//GO(PK11_TokenKeyGen, 
//GO(PK11_TokenKeyGenWithFlags, 
//GO(PK11_TokenRefresh, 
//GO(PK11_TraverseCertsForNicknameInSlot, 
//GO(PK11_TraverseCertsForSubjectInSlot, 
//GO(PK11_TraverseSlotCerts, 
//GO(PK11_UnconfigurePKCS11, 
//GO(PK11_UnlinkGenericObject, 
//GO(PK11_UnwrapPrivKey, 
//GO(PK11_UnwrapSymKey, 
//GO(PK11_UnwrapSymKeyWithFlags, 
//GO(PK11_UnwrapSymKeyWithFlagsPerm, 
//GO(PK11_UpdateSlotAttribute, 
//GO(PK11_UserDisableSlot, 
//GO(PK11_UserEnableSlot, 
//GO(PK11_Verify, 
//GO(PK11_VerifyKeyOK, 
//GO(PK11_VerifyRecover, 
//GO(PK11_VerifyWithMechanism, 
//GO(PK11_WaitForTokenEvent, 
//GO(PK11_WrapPrivKey, 
//GO(PK11_WrapSymKey, 
GO(PK11_WriteRawAttribute, iFipLp)
GO(PORT_Alloc, pFL)
GO(PORT_ArenaAlloc, pFpL)
//GO(PORT_ArenaGrow, 
//GO(PORT_ArenaMark, 
//GO(PORT_ArenaRelease, 
//GO(PORT_ArenaStrdup, 
//GO(PORT_ArenaUnmark, 
GO(PORT_ArenaZAlloc, pFpL)
GO(PORT_Free, vFp)
GO(PORT_FreeArena, vFpi)
GO(PORT_GetError, iFv)
GO(PORT_NewArena, pFL)
//GO(PORT_Realloc, 
GO(PORT_SetError, vFi)
GOM(PORT_SetUCS2_ASCIIConversionFunction, vFEp)
//GO(PORT_SetUCS2_UTF8ConversionFunction, 
//GO(PORT_SetUCS4_UTF8ConversionFunction, 
GO(PORT_Strdup, pFp)
//GO(PORT_UCS2_ASCIIConversion, 
GO(PORT_UCS2_UTF8Conversion, iFipupup)
GO(PORT_ZAlloc, pFL)
//GO(PORT_ZFree, 
//GO(RSA_FormatBlock, 
DATA(SEC_AnyTemplate, 4*sizeof(void*))    //R type
//GO(SEC_ASN1Decode, 
GO(SEC_ASN1DecodeInteger, iFpp)
GO(SEC_ASN1DecodeItem, iFpppp)
//GO(SEC_ASN1DecoderAbort, 
//GO(SEC_ASN1DecoderClearFilterProc, 
//GO(SEC_ASN1DecoderClearNotifyProc, 
//GO(SEC_ASN1DecoderFinish, 
//GO(SEC_ASN1DecoderSetFilterProc, 
//GO(SEC_ASN1DecoderSetNotifyProc, 
//GO(SEC_ASN1DecoderStart, 
//GO(SEC_ASN1DecoderUpdate, 
//GO(SEC_ASN1Encode, 
//GO(SEC_ASN1EncodeInteger, 
//GO(SEC_ASN1EncodeItem, 
//GO(SEC_ASN1EncoderAbort, 
//GO(SEC_ASN1EncoderClearNotifyProc, 
//GO(SEC_ASN1EncoderClearStreaming, 
//GO(SEC_ASN1EncoderClearTakeFromBuf, 
//GO(SEC_ASN1EncoderFinish, 
//GO(SEC_ASN1EncoderSetNotifyProc, 
//GO(SEC_ASN1EncoderSetStreaming, 
//GO(SEC_ASN1EncoderSetTakeFromBuf, 
//GO(SEC_ASN1EncoderStart, 
//GO(SEC_ASN1EncoderUpdate, 
//GO(SEC_ASN1EncodeUnsignedInteger, 
//GO(SEC_ASN1LengthLength, 
DATA(SEC_BitStringTemplate, 4*sizeof(void*)) //R type
DATA(SEC_BMPStringTemplate, 4*sizeof(void*)) //R type
//DATA(SEC_BooleanTemplate,     //R type
GO(SEC_CertNicknameConflict, iFppp)
//GO(SEC_CheckCrlTimes, 
//GO(SEC_CreateSignatureAlgorithmParameters, 
GO(SEC_DeletePermCertificate, iFp)
//GO(SEC_DeletePermCRL, 
GO(SEC_DerSignData, iFpppipi)
//GO(SEC_DerSignDataWithAlgorithmID, 
//GO(SEC_DestroyCrl, 
//GO(SEC_DupCrl, 
//GO(SEC_FindCrlByDERCert, 
//GO(SEC_FindCrlByName, 
//DATA(SEC_GeneralizedTimeTemplate,     //R type
//GO(SEC_GetCrlTimes, 
//GO(SEC_GetRegisteredHttpClient, 
//GO(SEC_GetSignatureAlgorithmOidTag, 
DATA(SEC_IA5StringTemplate, 4*sizeof(void*)) //R type
//DATA(SEC_IntegerTemplate,     //R type
GO(SECITEM_AllocItem, pFppu)
//GO(SECITEM_ArenaDupItem, 
GO(SECITEM_CompareItem, iFpp)
//GO(SECITEM_CopyItem, 
//GO(SECITEM_DupItem, 
GO(SECITEM_FreeItem, iFpi)
GO(SECITEM_ItemsAreEqual, iFpp)
GO(SECITEM_ZfreeItem, vFpi)
//GO(SECKEY_AddPrivateKeyToListTail, 
//GO(SECKEY_BigIntegerBitLength, 
//GO(SECKEY_CacheStaticFlags, 
//GO(SECKEY_ConvertToPublicKey, 
//GO(SECKEY_CopyEncryptedPrivateKeyInfo, 
//GO(SECKEY_CopyPrivateKey, 
//GO(SECKEY_CopyPrivateKeyInfo, 
//GO(SECKEY_CopyPublicKey, 
//GO(SECKEY_CopySubjectPublicKeyInfo, 
//GO(SECKEY_CreateDHPrivateKey, 
//GO(SECKEY_CreateECPrivateKey, 
//GO(SECKEY_CreateRSAPrivateKey, 
GO(SECKEY_CreateSubjectPublicKeyInfo, pFp)
//GO(SECKEY_DecodeDERSubjectPublicKeyInfo, 
//GO(SECKEY_DestroyEncryptedPrivateKeyInfo, 
GO(SECKEY_DestroyPrivateKey, vFp)
//GO(SECKEY_DestroyPrivateKeyInfo, 
//GO(SECKEY_DestroyPrivateKeyList, 
GO(SECKEY_DestroyPublicKey, vFp)
//GO(SECKEY_DestroyPublicKeyList, 
GO(SECKEY_DestroySubjectPublicKeyInfo, vFp)
//DATA(SECKEY_DSAPublicKeyTemplate,     //R type
//GO(SECKEY_ECParamsToBasePointOrderLen, 
//GO(SECKEY_ECParamsToKeySize, 
//GO(SECKEY_EncodeDERSubjectPublicKeyInfo, 
DATA(SECKEY_EncryptedPrivateKeyInfoTemplate, 128)
GO(SECKEY_ExtractPublicKey, pFp)
GO(SECKEY_GetPrivateKeyType, iFp)
//GO(SECKEY_GetPublicKeyType, 
//GO(SECKEY_HashPassword, 
//GO(SECKEY_ImportDERPublicKey, 
//GO(SECKEY_NewPrivateKeyList, 
//DATA(SECKEY_PointerToEncryptedPrivateKeyInfoTemplate, 
//DATA(SECKEY_PointerToPrivateKeyInfoTemplate, 
DATA(SECKEY_PrivateKeyInfoTemplate, 192)
//GO(SECKEY_PublicKeyStrength, 
GO(SECKEY_PublicKeyStrengthInBits, uFp)
//GO(SECKEY_RemovePrivateKeyListNode, 
//DATA(SECKEY_RSAPSSParamsTemplate, 
//DATA(SECKEY_RSAPublicKeyTemplate,     //R type
//GO(SECKEY_SignatureLen, 
//GO(SECKEY_UpdateCertPQG, 
//GO(SEC_LookupCrls, 
//GO(SECMOD_AddNewModule, 
//GO(SECMOD_AddNewModuleEx, 
//GO(SECMOD_CancelWait, 
//GO(SECMOD_CanDeleteInternalModule, 
//GO(SECMOD_CloseUserDB, 
//GO(SECMOD_CreateModule, 
//GO(SECMOD_CreateModuleEx, 
//GO(SECMOD_DeleteInternalModule, 
//GO(SECMOD_DeleteModule, 
//GO(SECMOD_DeleteModuleEx, 
GO(SECMOD_DestroyModule, vFp)
//GO(SECMOD_FindModule, 
//GO(SECMOD_FindSlot, 
//GO(SECMOD_FreeModuleSpecList, 
//GO(SECMOD_GetDBModuleList, 
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
//GO(SECMOD_IsModulePresent, 
//GO(SECMOD_LoadModule, 
GO(SECMOD_LoadUserModule, pFppi)
//GO(SECMOD_LookupSlot, 
//GO(SECMOD_OpenNewSlot, 
GO(SECMOD_OpenUserDB, pFp)
//GO(SECMOD_PubCipherFlagstoInternal, 
//GO(SECMOD_PubMechFlagstoInternal, 
//GO(SECMOD_ReferenceModule, 
GO(SECMOD_ReleaseReadLock, vFp)
//GO(SECMOD_RestartModules, 
//GO(SECMOD_UnloadUserModule, 
//GO(SECMOD_UpdateModule, 
//GO(SECMOD_UpdateSlotList, 
//GO(SECMOD_WaitForAnyTokenEvent, 
//GO(SEC_NewCrl, 
//DATA(SEC_NullTemplate,    //R type
//DATA(SEC_ObjectIDTemplate,    //R type
DATA(SEC_OctetStringTemplate, 4*sizeof(void*))   //R type
GO(SECOID_AddEntry, iFp)
DATA(SECOID_AlgorithmIDTemplate, 16*sizeof(void*))   //R type
//GO(SECOID_CompareAlgorithmID, 
//GO(SECOID_CopyAlgorithmID, 
//GO(SECOID_DestroyAlgorithmID, 
//GO(SECOID_FindOID, 
//GO(SECOID_FindOIDByTag, 
GO(SECOID_FindOIDTag, iFp)
//GO(SECOID_FindOIDTagDescription, 
//GO(SECOID_GetAlgorithmTag, 
//GO(SECOID_SetAlgorithmID, 
//GO(SEC_PKCS5GetCryptoAlgorithm, 
//GO(SEC_PKCS5GetIV, 
//GO(SEC_PKCS5GetKeyLength, 
//GO(SEC_PKCS5GetPBEAlgorithm, 
//GO(SEC_PKCS5IsAlgorithmPBEAlg, 
//GO(SEC_PKCS5IsAlgorithmPBEAlgTag, 
//DATA(SEC_PointerToAnyTemplate, 
//DATA(SEC_PointerToOctetStringTemplate, 
GO(SEC_QuickDERDecodeItem, iFpppp)
GO(SEC_RegisterDefaultHttpClient, iFp)
//DATA(SEC_SetOfAnyTemplate, 
//GO(SEC_SignData, 
//GO(SEC_SignDataWithAlgorithmID, 
//DATA(SEC_SignedCertificateTemplate, 
//DATA(SEC_UTCTimeTemplate,     //R type
DATA(SEC_UTF8StringTemplate, 4*sizeof(void*))    //R type
//GO(SGN_Begin, 
//GO(SGN_CompareDigestInfo, 
//GO(SGN_CopyDigestInfo, 
//GO(SGN_CreateDigestInfo, 
//GO(SGN_DestroyContext, 
//GO(SGN_DestroyDigestInfo, 
//GO(SGN_Digest, 
//DATA(sgn_DigestInfoTemplate, 
//GO(SGN_End, 
//GO(SGN_NewContext, 
//GO(SGN_NewContextWithAlgorithmID, 
//GO(SGN_Update, 
//GO(VFY_Begin, 
//GO(VFY_CreateContext, 
//GO(VFY_CreateContextDirect, 
//GO(VFY_CreateContextWithAlgorithmID, 
//GO(VFY_DestroyContext, 
//GO(VFY_End, 
//GO(VFY_EndWithSignature, 
//GO(VFY_Update, 
//GO(VFY_VerifyData, 
//GO(VFY_VerifyDataDirect, 
//GO(VFY_VerifyDataWithAlgorithmID, 
//GO(VFY_VerifyDigest, 
//GO(VFY_VerifyDigestDirect, 
//GO(VFY_VerifyDigestWithAlgorithmID, 
