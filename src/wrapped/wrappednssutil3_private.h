#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

//GO(ATOB_AsciiToData_Util, 
//GO(ATOB_ConvertAsciiToItem_Util, 
//GO(BTOA_ConvertItemToAscii_Util, 
//GO(BTOA_DataToAscii_Util, 
//GO(CERT_GenTime2FormattedAscii_Util, 
//GO(DER_AsciiToTime_Util, 
//GO(DER_DecodeTimeChoice_Util, 
//GO(DER_EncodeTimeChoice_Util, 
//GO(DER_Encode_Util, 
//GO(DER_GeneralizedDayToAscii_Util, 
//GO(DER_GeneralizedTimeToTime_Util, 
//GO(DER_GetInteger_Util, 
//GO(DER_GetUInteger, 
//GO(DER_LengthLength, 
//GO(DER_Lengths_Util, 
//GO(DER_SetUInteger, 
//GO(DER_StoreHeader, 
//GO(DER_TimeChoiceDayToAscii_Util, 
//GO(DER_TimeToGeneralizedTimeArena_Util, 
//GO(DER_TimeToGeneralizedTime_Util, 
//GO(DER_TimeToUTCTime_Util, 
//GO(DER_UTCDayToAscii_Util, 
//GO(DER_UTCTimeToAscii_Util, 
//GO(DER_UTCTimeToTime_Util, 
//GO(NSSBase64_DecodeBuffer_Util, 
//GO(NSSBase64Decoder_Create_Util, 
//GO(NSSBase64Decoder_Destroy_Util, 
//GO(NSSBase64Decoder_Update_Util, 
//GO(NSSBase64_EncodeItem_Util, 
//GO(NSSBase64Encoder_Create_Util, 
//GO(NSSBase64Encoder_Destroy_Util, 
//GO(NSSBase64Encoder_Update_Util, 
//GO(NSS_GetAlgorithmPolicy, 
//GO(NSS_Get_SEC_AnyTemplate_Util, 
//GO(NSS_Get_SEC_BitStringTemplate_Util, 
//GO(NSS_Get_SEC_BMPStringTemplate_Util, 
//GO(NSS_Get_SEC_BooleanTemplate_Util, 
//GO(NSS_Get_SEC_EnumeratedTemplate, 
//GO(NSS_Get_SEC_GeneralizedTimeTemplate_Util, 
//GO(NSS_Get_SEC_IA5StringTemplate_Util, 
//GO(NSS_Get_SEC_IntegerTemplate_Util, 
//GO(NSS_Get_SEC_NullTemplate_Util, 
//GO(NSS_Get_SEC_ObjectIDTemplate_Util, 
//GO(NSS_Get_SEC_OctetStringTemplate_Util, 
//GO(NSS_Get_SECOID_AlgorithmIDTemplate_Util, 
//GO(NSS_Get_SEC_PointerToAnyTemplate_Util, 
//GO(NSS_Get_SEC_PointerToEnumeratedTemplate, 
//GO(NSS_Get_SEC_PointerToGeneralizedTimeTemplate, 
//GO(NSS_Get_SEC_PointerToOctetStringTemplate_Util, 
//GO(NSS_Get_SEC_PrintableStringTemplate, 
//GO(NSS_Get_SEC_SequenceOfAnyTemplate, 
//GO(NSS_Get_SEC_SequenceOfObjectIDTemplate, 
//GO(NSS_Get_SEC_SetOfAnyTemplate_Util, 
//GO(NSS_Get_SEC_SkipTemplate, 
//GO(NSS_Get_SEC_T61StringTemplate, 
//GO(NSS_Get_SEC_UniversalStringTemplate, 
//GO(NSS_Get_SEC_UTF8StringTemplate_Util, 
//GO(NSS_Get_sgn_DigestInfoTemplate_Util, 
//GO(NSS_InitializePRErrorTable, 
//GO(NSS_PutEnv_Util, 
GO(NSSRWLock_Destroy_Util, vFp)
//GO(NSSRWLock_HaveWriteLock_Util, 
GO(NSSRWLock_LockRead_Util, vFp)
GO(NSSRWLock_LockWrite_Util, vFp)
GO(NSSRWLock_New_Util, pFup)
GO(NSSRWLock_UnlockRead_Util, vFp)
GO(NSSRWLock_UnlockWrite_Util, vFp)
GO(NSS_SecureMemcmp, iFppL)
//GO(NSS_SecureMemcmpZero, 
GO(NSS_SetAlgorithmPolicy, iFiuu)
//GO(NSSUTIL_AddNSSFlagToModuleSpec, 
//GO(NSSUTIL_ArgDecodeNumber, 
//GO(NSSUTIL_ArgFetchValue, 
//GO(NSSUTIL_ArgGetLabel, 
//GO(NSSUTIL_ArgGetParamValue, 
//GO(NSSUTIL_ArgHasFlag, 
//GO(NSSUTIL_ArgIsBlank, 
//GO(NSSUTIL_ArgParseCipherFlags, 
//GO(NSSUTIL_ArgParseModuleSpec, 
//GO(NSSUTIL_ArgParseModuleSpecEx, 
//GO(NSSUTIL_ArgParseSlotFlags, 
//GO(NSSUTIL_ArgParseSlotInfo, 
//GO(NSSUTIL_ArgReadLong, 
//GO(NSSUTIL_ArgSkipParameter, 
//GO(NSSUTIL_ArgStrip, 
//GO(NSSUTIL_DoModuleDBFunction, 
//GO(NSSUTIL_DoubleEscape, 
//GO(NSSUTIL_DoubleEscapeSize, 
//GO(NSSUTIL_Escape, 
//GO(NSSUTIL_EscapeSize, 
//GO(_NSSUTIL_EvaluateConfigDir, 
//GO(_NSSUTIL_GetSecmodName, 
//GO(NSSUTIL_GetVersion, 
//GO(NSSUTIL_MkModuleSpec, 
//GO(NSSUTIL_MkNSSString, 
//GO(NSSUTIL_MkSlotString, 
//GO(NSSUTIL_Quote, 
//GO(NSSUTIL_QuoteSize, 
//GO(PK11URI_CreateURI, 
//GO(PK11URI_DestroyURI, 
//GO(PK11URI_FormatURI, 
//GO(PK11URI_GetPathAttribute, 
//GO(PK11URI_GetQueryAttribute, 
//GO(PK11URI_ParseURI, 
GO(PORT_Alloc_Util, pFL)
GO(PORT_ArenaAlloc_Util, pFpL)
//GO(PORT_ArenaGrow_Util, 
GO(PORT_ArenaMark_Util, pFp)
GO(PORT_ArenaRelease_Util, vFpp)
//GO(PORT_ArenaStrdup_Util, 
GO(PORT_ArenaUnmark_Util, vFpp)
GO(PORT_ArenaZAlloc_Util, pFpL)
//GO(PORT_DestroyCheapArena, 
GO(PORT_FreeArena_Util, vFpi)
GO(PORT_Free_Util, vFp)
GO(PORT_GetError_Util, iFv)
//GO(PORT_InitCheapArena, 
//GO(PORT_ISO88591_UTF8Conversion, 
//GO(PORT_LoadLibraryFromOrigin, 
GO(PORT_NewArena_Util, pFL)
GO(PORT_Realloc_Util, pFpL)
//GO(PORT_RegExpCaseSearch, 
//GO(PORT_RegExpSearch, 
//GO(PORT_RegExpValid, 
GO(PORT_SetError_Util, vFi)
//GO(PORT_SetUCS2_ASCIIConversionFunction_Util, 
//GO(PORT_SetUCS2_UTF8ConversionFunction_Util, 
//GO(PORT_SetUCS4_UTF8ConversionFunction_Util, 
GO(PORT_Strdup_Util, pFp)
//GO(PORT_UCS2_ASCIIConversion_Util, 
//GO(PORT_UCS2_UTF8Conversion_Util, 
//GO(PORT_UCS4_UTF8Conversion, 
//GO(PORT_ZAllocAlignedOffset_Util, 
//GO(PORT_ZAllocAligned_Util, 
GO(PORT_ZAlloc_Util, pFL)
GO(PORT_ZFree_Util, vFpL)
//DATA(SEC_AnyTemplate_Util, //R-type
//GO(SEC_ASN1DecodeInteger_Util, 
//GO(SEC_ASN1DecodeItem_Util, 
//GO(SEC_ASN1DecoderAbort_Util, 
//GO(SEC_ASN1DecoderClearFilterProc_Util, 
//GO(SEC_ASN1DecoderClearNotifyProc_Util, 
//GO(SEC_ASN1DecoderFinish_Util, 
//GO(SEC_ASN1DecoderSetFilterProc_Util, 
//GO(SEC_ASN1DecoderSetMaximumElementSize, 
//GO(SEC_ASN1DecoderSetNotifyProc_Util, 
//GO(SEC_ASN1DecoderStart_Util, 
//GO(SEC_ASN1DecoderUpdate_Util, 
//GO(SEC_ASN1Decode_Util, 
//GO(SEC_ASN1EncodeInteger_Util, 
//GO(SEC_ASN1EncodeItem_Util, 
//GO(SEC_ASN1EncoderAbort_Util, 
//GO(SEC_ASN1EncoderClearNotifyProc_Util, 
//GO(SEC_ASN1EncoderClearStreaming_Util, 
//GO(SEC_ASN1EncoderClearTakeFromBuf_Util, 
//GO(SEC_ASN1EncoderFinish_Util, 
//GO(SEC_ASN1EncoderSetNotifyProc_Util, 
//GO(SEC_ASN1EncoderSetStreaming_Util, 
//GO(SEC_ASN1EncoderSetTakeFromBuf_Util, 
//GO(SEC_ASN1EncoderStart_Util, 
//GO(SEC_ASN1EncoderUpdate_Util, 
//GO(SEC_ASN1EncodeUnsignedInteger_Util, 
//GO(SEC_ASN1Encode_Util, 
//GO(SEC_ASN1LengthLength_Util, 
//DATA(SEC_BitStringTemplate_Util, //R-type
//DATA(SEC_BMPStringTemplate_Util, //R-type
//DATA(SEC_BooleanTemplate_Util, //R-type
//DATA(SEC_EnumeratedTemplate, //R-type
//DATA(SEC_GeneralizedTimeTemplate_Util, //R-type
//DATA(SEC_IA5StringTemplate_Util, //R-type
//DATA(SEC_IntegerTemplate_Util, //R-type
//GO(SECITEM_AllocArray, 
GO(SECITEM_AllocItem_Util, pFppu)
//GO(SECITEM_ArenaDupItem_Util, 
GO(SECITEM_CompareItem_Util, iFpp)
GO(SECITEM_CopyItem_Util, iFppp)
//GO(SECITEM_DupArray, 
//GO(SECITEM_DupItem_Util, 
//GO(SECITEM_FreeArray, 
GO(SECITEM_FreeItem_Util, vFpi)
//GO(SECITEM_Hash, 
//GO(SECITEM_HashCompare, 
//GO(SECITEM_ItemsAreEqual_Util, 
//GO(SECITEM_MakeItem, 
//GO(SECITEM_ReallocItem, 
//GO(SECITEM_ReallocItemV2, 
//GO(SECITEM_ZfreeArray, 
GO(SECITEM_ZfreeItem_Util, vFpi)
//DATA(SEC_NullTemplate_Util, //R-type
//DATA(SEC_ObjectIDTemplate_Util, //R-type
//DATA(SEC_OctetStringTemplate_Util, //R-type
//GO(SECOID_AddEntry_Util, 
//DATA(SECOID_AlgorithmIDTemplate_Util, //R-type
//GO(SECOID_CompareAlgorithmID_Util, 
//GO(SECOID_CopyAlgorithmID_Util, 
//GO(SECOID_DestroyAlgorithmID_Util, 
//GO(SECOID_FindOIDByMechanism, 
GO(SECOID_FindOIDByTag_Util, pFi)
//GO(SECOID_FindOIDTagDescription_Util, 
//GO(SECOID_FindOIDTag_Util, 
GO(SECOID_FindOID_Util, pFp)
GO(SECOID_GetAlgorithmTag_Util, iFp)
//GO(SECOID_Init, 
//GO(SECOID_KnownCertExtenOID, 
//GO(SECOID_SetAlgorithmID_Util, 
//GO(SECOID_Shutdown, 
//DATA(SEC_PointerToAnyTemplate_Util, 
//DATA(SEC_PointerToEnumeratedTemplate, 
//DATA(SEC_PointerToGeneralizedTimeTemplate, 
//DATA(SEC_PointerToOctetStringTemplate_Util, 
//DATA(SEC_PrintableStringTemplate, //R-type
//GO(SEC_QuickDERDecodeItem_Util, 
//DATA(SEC_SequenceOfAnyTemplate, 
//DATA(SEC_SequenceOfObjectIDTemplate, 
//DATA(SEC_SetOfAnyTemplate_Util, 
//DATA(SEC_SkipTemplate, //R-type
GO(SEC_StringToOID, iFpppu)
//DATA(SEC_T61StringTemplate, //R-type
//DATA(SEC_UniversalStringTemplate, //R-type
//DATA(SEC_UTF8StringTemplate_Util, //R-type
//GO(SGN_CompareDigestInfo_Util, 
//GO(SGN_CopyDigestInfo_Util, 
//GO(SGN_CreateDigestInfo_Util, 
//GO(SGN_DecodeDigestInfo, 
//GO(SGN_DestroyDigestInfo_Util, 
//DATA(sgn_DigestInfoTemplate_Util, 
//GO(_SGN_VerifyPKCS1DigestInfo, 
//GO(UTIL_SetForkState, 
