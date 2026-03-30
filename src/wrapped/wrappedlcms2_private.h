#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

//GO(_cms15Fixed16toDouble, 
//GO(_cms8Fixed8toDouble, 
GO(cmsAdaptToIlluminant, iFpppp)
//GO(_cmsAdjustEndianess16, 
//GO(_cmsAdjustEndianess32, 
//GO(_cmsAdjustEndianess64, 
GO(cmsAllocNamedColorList, pFpuupp)
GO(cmsAllocProfileSequenceDescription, pFpu)
GO(cmsAppendNamedColor, iFpppp)
GO(cmsBFDdeltaE, dFpp)
GO(cmsBuildGamma, pFpd)
GO(cmsBuildParametricToneCurve, pFpip)
GO(cmsBuildSegmentedToneCurve, pFpup)
GO(cmsBuildTabulatedToneCurve16, pFpup)
GO(cmsBuildTabulatedToneCurveFloat, pFpup)
//GO(_cmsCalloc, 
GO(cmsChangeBuffersFormat, iFpuu)
GO(cmsChannelsOf, uFu)
GO(cmsCIE2000DeltaE, dFppddd)
GO(cmsCIE94DeltaE, dFpp)
GO(cmsCIECAM02Done, vFp)
GO(cmsCIECAM02Forward, vFppp)
GO(cmsCIECAM02Init, pFpp)
GO(cmsCIECAM02Reverse, vFppp)
GO(cmsCloseIOhandler, iFp)
GO(cmsCloseProfile, iFp)
GO(cmsCMCdeltaE, dFppdd)
//GO(_cmsComputeInterpParams, 
GO(cmsCreateBCHSWabstractProfile, pFudddduu)
GO(cmsCreateBCHSWabstractProfileTHR, pFpudddduu)
GO(cmsCreateContext, pFpp)
GO(cmsCreateExtendedTransform, pFpupppppuuuu)
GO(cmsCreateGrayProfile, pFpp)
GO(cmsCreateGrayProfileTHR, pFppp)
GO(cmsCreateInkLimitingDeviceLink, pFud)
GO(cmsCreateInkLimitingDeviceLinkTHR, pFpud)
GO(cmsCreateLab2Profile, pFp)
GO(cmsCreateLab2ProfileTHR, pFpp)
GO(cmsCreateLab4Profile, pFp)
GO(cmsCreateLab4ProfileTHR, pFpp)
GO(cmsCreateLinearizationDeviceLink, pFup)
GO(cmsCreateLinearizationDeviceLinkTHR, pFpup)
GO(cmsCreateMultiprofileTransform, pFpuuuuu)
GO(cmsCreateMultiprofileTransformTHR, pFppuuuuu)
//GO(_cmsCreateMutex, 
GO(cmsCreateNULLProfile, pFv)
GO(cmsCreateNULLProfileTHR, pFp)
GO(cmsCreateProfilePlaceholder, pFp)
GO(cmsCreateProofingTransform, pFpupupuuu)
GO(cmsCreateProofingTransformTHR, pFppupupuuu)
GO(cmsCreateRGBProfile, pFppp)
GO(cmsCreateRGBProfileTHR, pFpppp)
GO(cmsCreate_sRGBProfile, pFv)
GO(cmsCreate_sRGBProfileTHR, pFp)
GO(cmsCreateTransform, pFpupuuu)
GO(cmsCreateTransformTHR, pFppupuuu)
GO(cmsCreateXYZProfile, pFv)
GO(cmsCreateXYZProfileTHR, pFp)
GO(cmsD50_xyY, pFv)
GO(cmsD50_XYZ, pFv)
//GO(_cmsDecodeDateTimeNumber, 
//GO(_cmsDefaultICCintents, 
GO(cmsDeleteContext, vFp)
GO(cmsDeleteTransform, vFp)
GO(cmsDeltaE, dFpp)
GO(cmsDesaturateLab, iFpdddd)
//GO(_cmsDestroyMutex, 
GO(cmsDetectBlackPoint, iFppuu)
GO(cmsDetectDestinationBlackPoint, iFppuu)
GO(cmsDetectTAC, dFp)
GO(cmsDictAddEntry, iFppppp)
GO(cmsDictAlloc, pFp)
GO(cmsDictDup, pFp)
GO(cmsDictFree, vFp)
GO(cmsDictGetEntryList, pFp)
GO(cmsDictNextEntry, pFp)
GO(cmsDoTransform, vFpppu)
GO(cmsDoTransformLineStride, vFpppuuuuuu)
GO(cmsDoTransformStride, vFpppuu)
//GO(_cmsDoubleTo15Fixed16, 
//GO(_cmsDoubleTo8Fixed8, 
GO(cmsDupContext, pFpp)
//GO(_cmsDupMem, 
GO(cmsDupNamedColorList, pFp)
GO(cmsDupProfileSequenceDescription, pFp)
GO(cmsDupToneCurve, pFp)
//GO(_cmsEncodeDateTimeNumber, 
GO(cmsEstimateGamma, dFpd)
GO(cmsEvalToneCurve16, WFpW)
GO(cmsEvalToneCurveFloat, fFpf)
GO(cmsfilelength, lFS)
//GO(_cmsFloat2Half, 
GO(cmsFloat2LabEncoded, vFpp)
GO(cmsFloat2LabEncodedV2, vFpp)
GO(cmsFloat2XYZEncoded, vFpp)
GO(cmsFormatterForColorspaceOfProfile, uFpui)
GO(cmsFormatterForPCSOfProfile, uFpui)
//GO(_cmsFree, 
//GO(_cmsFreeInterpParams, 
GO(cmsFreeNamedColorList, vFp)
GO(cmsFreeProfileSequenceDescription, vFp)
GO(cmsFreeToneCurve, vFp)
GO(cmsFreeToneCurveTriple, vFp)
GO(cmsGBDAlloc, pFp)
GO(cmsGBDFree, vFp)
GO(cmsGDBAddPoint, iFpp)
GO(cmsGDBCheckPoint, iFpp)
GO(cmsGDBCompute, iFpu)
GO(cmsGetAlarmCodes, vFp)
GO(cmsGetAlarmCodesTHR, vFpp)
GO(cmsGetColorSpace, uFp)
GO(cmsGetContextUserData, pFp)
GO(cmsGetDeviceClass, uFp)
GO(cmsGetEncodedCMMversion, iFv)
GO(cmsGetEncodedICCversion, uFp)
//GO(_cmsGetFormatter, 
GO(cmsGetHeaderAttributes, vFpp)
GO(cmsGetHeaderCreationDateTime, iFpp)
GO(cmsGetHeaderCreator, uFp)
GO(cmsGetHeaderFlags, uFp)
GO(cmsGetHeaderManufacturer, uFp)
GO(cmsGetHeaderModel, uFp)
GO(cmsGetHeaderProfileID, vFpp)
GO(cmsGetHeaderRenderingIntent, uFp)
GO(cmsGetNamedColorList, pFp)
GO(cmsGetPCS, uFp)
GO(cmsGetPipelineContextID, pFp)
GO(cmsGetPostScriptColorResource, uFpupuup)
GO(cmsGetPostScriptCRD, uFppuupu)
GO(cmsGetPostScriptCSA, uFppuupu)
GO(cmsGetProfileContextID, pFp)
GO(cmsGetProfileInfo, uFpupppu)
GO(cmsGetProfileInfoASCII, uFpupppu)
GO(cmsGetProfileIOhandler, pFp)
GO(cmsGetProfileVersion, dFp)
GO(cmsGetSupportedIntents, uFupp)
GO(cmsGetSupportedIntentsTHR, uFpupp)
GO(cmsGetTagCount, iFp)
GO(cmsGetTagSignature, uFpu)
GO(cmsGetToneCurveEstimatedTable, pFp)
GO(cmsGetToneCurveEstimatedTableEntries, uFp)
GO(cmsGetToneCurveParametricType, iFp)
GO(cmsGetToneCurveParams, pFp)
GO(cmsGetTransformContextID, pFp)
//GO(_cmsGetTransformFlags, 
//GO(_cmsGetTransformFormatters16, 
//GO(_cmsGetTransformFormattersFloat, 
GO(cmsGetTransformInputFormat, uFp)
GO(cmsGetTransformOutputFormat, uFp)
//GO(_cmsGetTransformUserData, 
//GO(_cmsHalf2Float, 
GO(_cmsICCcolorSpace, uFi)
//GO(_cmsIOPrintf, 
GO(cmsIsCLUT, iFpuu)
GO(cmsIsIntentSupported, iFpuu)
GO(cmsIsMatrixShaper, iFp)
GO(cmsIsTag, iFpu)
GO(cmsIsToneCurveDescending, iFp)
GO(cmsIsToneCurveLinear, iFp)
GO(cmsIsToneCurveMonotonic, iFp)
GO(cmsIsToneCurveMultisegment, iFp)
GO(cmsIT8Alloc, pFp)
GO(cmsIT8DefineDblFormat, vFpp)
GO(cmsIT8EnumDataFormat, iFpp)
GO(cmsIT8EnumProperties, uFpp)
GO(cmsIT8EnumPropertyMulti, uFppp)
GO(cmsIT8FindDataFormat, iFpp)
GO(cmsIT8Free, vFp)
GO(cmsIT8GetData, pFppp)
GO(cmsIT8GetDataDbl, dFppp)
GO(cmsIT8GetDataRowCol, pFpii)
GO(cmsIT8GetDataRowColDbl, dFpii)
GO(cmsIT8GetPatchByName, iFpp)
GO(cmsIT8GetPatchName, pFpip)
GO(cmsIT8GetProperty, pFpp)
GO(cmsIT8GetPropertyDbl, dFpp)
GO(cmsIT8GetPropertyMulti, pFppp)
GO(cmsIT8GetSheetType, pFp)
GO(cmsIT8LoadFromFile, pFpp)
GO(cmsIT8LoadFromMem, pFppu)
GO(cmsIT8SaveToFile, iFpp)
GO(cmsIT8SaveToMem, iFppp)
GO(cmsIT8SetComment, iFpp)
GO(cmsIT8SetData, iFpppp)
GO(cmsIT8SetDataDbl, iFpppd)
GO(cmsIT8SetDataFormat, iFpip)
GO(cmsIT8SetDataRowCol, iFpiip)
GO(cmsIT8SetDataRowColDbl, iFpiid)
GO(cmsIT8SetIndexColumn, iFpp)
GO(cmsIT8SetPropertyDbl, iFppd)
GO(cmsIT8SetPropertyHex, iFppu)
GO(cmsIT8SetPropertyMulti, iFpppp)
GO(cmsIT8SetPropertyStr, iFppp)
GO(cmsIT8SetPropertyUncooked, iFppp)
GO(cmsIT8SetSheetType, iFpp)
GO(cmsIT8SetTable, iFpu)
GO(cmsIT8SetTableByLabel, iFpppp)
GO(cmsIT8TableCount, uFp)
GO(cmsJoinToneCurve, pFpppu)
GO(cmsLab2LCh, vFpp)
GO(cmsLab2XYZ, vFppp)
GO(cmsLabEncoded2Float, vFpp)
GO(cmsLabEncoded2FloatV2, vFpp)
GO(cmsLCh2Lab, vFpp)
GO(_cmsLCMScolorSpace, iFu)
GO(cmsLinkTag, iFpuu)
//GO(_cmsLockMutex, 
//GO(_cmsMalloc, 
//GO(_cmsMallocZero, 
//GO(_cmsMAT3eval, 
//GO(_cmsMAT3identity, 
//GO(_cmsMAT3inverse, 
//GO(_cmsMAT3isIdentity, 
//GO(_cmsMAT3per, 
//GO(_cmsMAT3solve, 
//GO(cmsMD5add, 
//GO(cmsMD5alloc, 
GO(cmsMD5computeID, iFp)
//GO(cmsMD5finish, 
GO(cmsMLUalloc, pFpu)
GO(cmsMLUdup, pFp)
GO(cmsMLUfree, vFp)
GO(cmsMLUgetASCII, uFppppu)
GO(cmsMLUgetTranslation, iFppppp)
GO(cmsMLUgetWide, uFppppu)
GO(cmsMLUsetASCII, iFpppp)
GO(cmsMLUsetWide, iFpppp)
GO(cmsMLUtranslationsCodes, iFpupp)
GO(cmsMLUtranslationsCount, uFp)
GO(cmsNamedColorCount, uFp)
GO(cmsNamedColorIndex, iFpp)
GO(cmsNamedColorInfo, iFpuppppp)
GO(cmsOpenIOhandlerFromFile, pFppp)
GO(cmsOpenIOhandlerFromMem, pFppup)
GO(cmsOpenIOhandlerFromNULL, pFp)
GO(cmsOpenIOhandlerFromStream, pFpS)
GO(cmsOpenProfileFromFile, pFpp)
GO(cmsOpenProfileFromFileTHR, pFppp)
GO(cmsOpenProfileFromIOhandler2THR, pFppi)
GO(cmsOpenProfileFromIOhandlerTHR, pFpp)
GO(cmsOpenProfileFromMem, pFpu)
GO(cmsOpenProfileFromMemTHR, pFppu)
GO(cmsOpenProfileFromStream, pFSp)
GO(cmsOpenProfileFromStreamTHR, pFpSp)
//GO(_cmsOptimizePipeline, 
GO(cmsPipelineAlloc, pFpuu)
GO(cmsPipelineCat, iFpp)
//GOM(cmsPipelineCheckAndRetreiveStages, iFEpuV)
GO(cmsPipelineDup, pFp)
GO(cmsPipelineEval16, vFppp)
GO(cmsPipelineEvalFloat, vFppp)
GO(cmsPipelineEvalReverseFloat, iFpppp)
GO(cmsPipelineFree, vFp)
GO(cmsPipelineGetPtrToFirstStage, pFp)
GO(cmsPipelineGetPtrToLastStage, pFp)
GO(cmsPipelineInputChannels, uFp)
GO(cmsPipelineInsertStage, iFpup)
GO(cmsPipelineOutputChannels, uFp)
//GO(_cmsPipelineSetOptimizationParameters, 
GO(cmsPipelineSetSaveAs8bitsFlag, iFpi)
GO(cmsPipelineStageCount, uFp)
GO(cmsPipelineUnlinkStage, vFpup)
GO(cmsPlugin, iFp)
GO(cmsPluginTHR, iFpp)
//GO(_cmsQuantizeVal, 
//GO(_cmsRead15Fixed16Number, 
//GO(_cmsReadAlignment, 
//GO(_cmsReadDevicelinkLUT, 
//GO(_cmsReadFloat32Number, 
//GO(_cmsReadInputLUT, 
//GO(_cmsReadOutputLUT, 
GO(cmsReadRawTag, uFpupu)
GO(cmsReadTag, pFpu)
//GO(_cmsReadTypeBase, 
//GO(_cmsReadUInt16Array, 
//GO(_cmsReadUInt16Number, 
//GO(_cmsReadUInt32Number, 
//GO(_cmsReadUInt64Number, 
//GO(_cmsReadUInt8Number, 
//GO(_cmsReadXYZNumber, 
//GO(_cmsRealloc, 
//GO(_cmsReasonableGridpointsByColorspace, 
GO(cmsReverseToneCurve, pFp)
GO(cmsReverseToneCurveEx, pFup)
GO(cmsSaveProfileToFile, iFpp)
GO(cmsSaveProfileToIOhandler, uFpp)
GO(cmsSaveProfileToMem, iFppp)
GO(cmsSaveProfileToStream, iFpS)
GO(cmsSetAdaptationState, dFd)
GO(cmsSetAdaptationStateTHR, dFpd)
GO(cmsSetAlarmCodes, vFp)
GO(cmsSetAlarmCodesTHR, vFpp)
GO(cmsSetColorSpace, vFpu)
GO(cmsSetDeviceClass, vFpu)
GO(cmsSetEncodedICCversion, vFpu)
GO(cmsSetHeaderAttributes, vFpL)
GO(cmsSetHeaderFlags, vFpu)
GO(cmsSetHeaderManufacturer, vFpu)
GO(cmsSetHeaderModel, vFpu)
GO(cmsSetHeaderProfileID, vFpp)
GO(cmsSetHeaderRenderingIntent, vFpu)
//GOM(cmsSetLogErrorHandler, vFEp)
GOM(cmsSetLogErrorHandlerTHR, vFEpp)
GO(cmsSetPCS, vFpu)
GO(cmsSetProfileVersion, vFpd)
//GO(_cmsSetTransformUserData, 
//GO(cmsSignalError, 
//GOM(cmsSliceSpace16, iFEuppp)
//GOM(cmsSliceSpaceFloat, iFEuppp)
GO(cmsSmoothToneCurve, iFpd)
GO(cmsStageAllocCLut16bit, pFpuuup)
GO(cmsStageAllocCLut16bitGranular, pFppuup)
GO(cmsStageAllocCLutFloat, pFpuuup)
GO(cmsStageAllocCLutFloatGranular, pFppuup)
GO(cmsStageAllocIdentity, pFpu)
//GO(_cmsStageAllocIdentityCLut, 
//GO(_cmsStageAllocIdentityCurves, 
//GO(_cmsStageAllocLab2XYZ, 
//GO(_cmsStageAllocLabV2ToV4, 
//GO(_cmsStageAllocLabV4ToV2, 
GO(cmsStageAllocMatrix, pFpuupp)
//GO(_cmsStageAllocNamedColor, 
//GO(_cmsStageAllocPlaceholder, 
GO(cmsStageAllocToneCurves, pFpup)
//GO(_cmsStageAllocXYZ2Lab, 
GO(cmsStageData, pFp)
GO(cmsStageDup, pFp)
GO(cmsStageFree, vFp)
GO(cmsStageInputChannels, uFp)
GO(cmsStageNext, pFp)
GO(cmsStageOutputChannels, uFp)
//GOM(cmsStageSampleCLut16bit, iFEpppu)
//GOM(cmsStageSampleCLutFloat, iFEpppu)
GO(cmsStageType, uFp)
GO(cmsstrcasecmp, iFpp)
GO(cmsTagLinkedTo, uFpu)
GO(cmsTempFromWhitePoint, iFpp)
GO(cmsTransform2DeviceLink, pFpdu)
//GO(_cmsUnlockMutex, 
GO(cmsUnregisterPlugins, vFv)
GO(cmsUnregisterPluginsTHR, vFp)
//GO(_cmsVEC3cross, 
//GO(_cmsVEC3distance, 
//GO(_cmsVEC3dot, 
//GO(_cmsVEC3init, 
//GO(_cmsVEC3length, 
//GO(_cmsVEC3minus, 
GO(cmsWhitePointFromTemp, iFpd)
//GO(_cmsWrite15Fixed16Number, 
//GO(_cmsWriteAlignment, 
//GO(_cmsWriteFloat32Number, 
GO(cmsWriteRawTag, iFpupu)
GO(cmsWriteTag, iFpup)
//GO(_cmsWriteTypeBase, 
//GO(_cmsWriteUInt16Array, 
//GO(_cmsWriteUInt16Number, 
//GO(_cmsWriteUInt32Number, 
//GO(_cmsWriteUInt64Number, 
//GO(_cmsWriteUInt8Number, 
//GO(_cmsWriteXYZNumber, 
GO(cmsxyY2XYZ, vFpp)
GO(cmsXYZ2Lab, vFppp)
GO(cmsXYZ2xyY, vFpp)
GO(cmsXYZEncoded2Float, vFpp)
