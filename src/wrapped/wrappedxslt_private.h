#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(xslAddCall, iFpp)
GO(xslDropCall, vFv)
GO(xslHandleDebugger, vFpppp)
//GO(xsltAddKey, 
GO(xsltAddStackElemList, iFpp)
//GO(xsltAddTemplate, 
GO(xsltAllocateExtra, iFp)
GO(xsltAllocateExtraCtxt, iFp)
//GO(xsltApplyAttributeSet, 
GO(xsltApplyImports, vFpppp)
GO(xsltApplyOneTemplate, vFppppp)
GO(xsltApplyStripSpaces, vFpp)
GO(xsltApplyStylesheet, pFppp)
GO(xsltApplyStylesheetUser, pFpppppp)
GO(xsltApplyTemplates, vFpppp)
GO(xsltAttribute, vFpppp)
//GO(xsltAttrListTemplateProcess, 
//GO(xsltAttrTemplateProcess, 
//GO(xsltAttrTemplateValueProcess, 
//GO(xsltAttrTemplateValueProcessNode, 
GO(xsltCalibrateAdjust, vFl)
GO(xsltCallTemplate, vFpppp)
GO(xsltCheckExtPrefix, iFpp)
GO(xsltCheckExtURI, iFpp)
GO(xsltCheckRead, iFppp)
GO(xsltCheckWrite, iFppp)
GO(xsltChoose, vFpppp)
GO(xsltCleanupGlobals, vFv)
//GO(xsltCleanupTemplates, 
GO(xsltComment, vFpppp)
GO(xsltCompileAttr, vFpp)
//GO(xsltCompilePattern, 
GO(xsltComputeSortResult, pFpp)
GO(xsltCopy, vFpppp)
//GO(xsltCopyNamespace, 
//GO(xsltCopyNamespaceList, 
GO(xsltCopyOf, vFpppp)
GO(xsltCopyTextString, pFpppi)
//GO(xsltCopyTree, 
GO(xsltCreateRVT, pFp)
GO(xsltDebug, vFpppp)
GO(xsltDebugDumpExtensions, vFp)
GO(xsltDebugGetDefaultTrace, iFv)
GO(xsltDebugSetDefaultTrace, vFi)
GO(xsltDecimalFormatGetByName, pFpp)
GO(xsltDecimalFormatGetByQName, pFppp)
GO(xsltDefaultSortFunction, vFppi)
//GO(xsltDocumentComp, 
GO(xsltDocumentElem, vFpppp)
//GO(xsltDocumentFunction, 
GO(xsltDocumentSortFunction, vFp)
GO(xsltDoSortFunction, vFppi)
GO(xsltElement, vFpppp)
//GO(xsltElementAvailableFunction, 
GO(xsltEvalAttrValueTemplate, pFpppp)
GO(xsltEvalAVT, pFppp)
GO(xsltEvalGlobalVariables, iFp)
GO(xsltEvalOneUserParam, iFppp)
//GO(xsltEvalStaticAttrValueTemplate, 
//GO(xsltEvalTemplateString, 
GO(xsltEvalUserParams, iFpp)
//GO(xsltEvalXPathPredicate, 
//GO(xsltEvalXPathString, 
//GO(xsltEvalXPathStringNs, 
//GOM(xsltExtElementLookup, BFEppp)
GO(xsltExtensionInstructionResultFinalize, iFp)
GO(xsltExtensionInstructionResultRegister, iFpp)
//GOM(xsltExtModuleElementLookup, BFEpp)
//GOM(xsltExtModuleElementPreComputeLookup, BFEpp)
//GOM(xsltExtModuleFunctionLookup, BFEpp)
//GOM(xsltExtModuleTopLevelLookup, BFEpp)
GO(xsltFindDocument, pFpp)
GO(xsltFindElemSpaceHandling, iFpp)
GO(xsltFindTemplate, pFppp)
GO(xsltFlagRVTs, iFppp)
GO(xsltForEach, vFpppp)
GO(xsltFormatNumberConversion, iFppdp)
//GO(xsltFormatNumberFunction, 
//GO(xsltFreeAttributeSetsHashes, 
GO(xsltFreeAVTList, vFp)
//GO(xsltFreeCompMatchList, 
GO(xsltFreeCtxtExts, vFp)
//GO(xsltFreeDocumentKeys, 
GO(xsltFreeDocuments, vFp)
GO(xsltFreeExts, vFp)
GO(xsltFreeGlobalVariables, vFp)
//GO(xsltFreeKeys, 
//GO(xsltFreeLocale, 
//GO(xsltFreeNamespaceAliasHashes, 
GO(xsltFreeRVTs, vFp)
GO(xsltFreeSecurityPrefs, vFp)
GO(xsltFreeStackElemList, vFp)
GO(xsltFreeStyleDocuments, vFp)
//GO(xsltFreeStylePreComps, 
GO(xsltFreeStylesheet, vFp)
//GO(xsltFreeTemplateHashes, 
GO(xsltFreeTransformContext, vFp)
//GO(xsltFunctionAvailableFunction, 
GO(xsltFunctionNodeSet, vFpi)
//GO(xsltGenerateIdFunction, 
GO(xsltGetCNsProp, pFpppp)
GO(xsltGetDebuggerStatus, iFv)
//GO(xsltGetDefaultSecurityPrefs, 
GO(xsltGetExtData, pFpp)
GO(xsltGetExtInfo, pFpp)
//GO(xsltGetKey, 
//GO(xsltGetNamespace, 
GO(xsltGetNsProp, pFppp)
//GO(xsltGetPlainNamespace, 
GO(xsltGetProfileInformation, pFp)
GO(xsltGetQNameURI, pFpp)
GO(xsltGetQNameURI2, pFppp)
GO(xsltGetSecurityPrefs, pFpi)
//GO(xsltGetSpecialNamespace, 
//GO(xsltGetTemplate, 
GO(xsltGetUTF8Char, iFpp)
GO(xsltGetXIncludeDefault, iFv)
GO(xsltIf, vFpppp)
GO(xsltInit, vFv)
GO(xsltInitAllDocKeys, iFp)
GO(xsltInitCtxtExts, iFp)
//GO(xsltInitCtxtKey, 
//GO(xsltInitCtxtKeys, 
//GOM(xsltInitElemPreComp, vFEpppBB)
GO(xsltInitGlobals, vFv)
GO(xsltIsBlank, iFp)
//GO(xsltKeyFunction, 
GO(xsltLoadDocument, pFpp)
GO(xsltLoadStyleDocument, pFpp)
GO(xsltLoadStylesheetPI, pFp)
//GO(xsltLocaleStrcmp, 
GO(xsltLocalVariablePop, vFpii)
GO(xsltLocalVariablePush, iFppi)
GO(xsltMessage, vFppp)
//GO(xsltNamespaceAlias, 
GO(xsltNeedElemSpaceHandling, iFp)
GO(xsltNewDocument, pFpp)
//GOM(xsltNewElemPreComp, pFEppB)
//GO(xsltNewLocale, 
GO(xsltNewSecurityPrefs, pFv)
GO(xsltNewStyleDocument, pFpp)
GO(xsltNewStylesheet, pFv)
GO(xsltNewTransformContext, pFpp)
GO(xsltNextImport, pFp)
//GO(xsltNormalizeCompSteps, 
GO(xsltNumber, vFpppp)
GO(xsltNumberFormat, vFppp)
GO(xsltParseAnyXSLTElem, iFpp)
GO(xsltParseGlobalParam, vFpp)
GO(xsltParseGlobalVariable, vFpp)
GO(xsltParseSequenceConstructor, vFpp)
//GO(xsltParseStylesheetAttributeSet, 
GO(xsltParseStylesheetCallerParam, pFpp)
GO(xsltParseStylesheetDoc, pFp)
GO(xsltParseStylesheetFile, pFp)
GO(xsltParseStylesheetImport, iFpp)
GO(xsltParseStylesheetImportedDoc, pFpp)
GO(xsltParseStylesheetInclude, iFpp)
GO(xsltParseStylesheetOutput, vFpp)
GO(xsltParseStylesheetParam, vFpp)
GO(xsltParseStylesheetProcess, pFpp)
GO(xsltParseStylesheetVariable, vFpp)
//GO(xsltParseTemplateContent, 
GO(xsltPointerListAddSize, iFppi)
GO(xsltPointerListClear, vFp)
GO(xsltPointerListCreate, pFi)
GO(xsltPointerListFree, vFp)
GO(xsltPreComputeExtModuleElement, pFpp)
GO(xsltPrintErrorContext, vFppp)
GO(xsltProcessingInstruction, vFpppp)
GO(xsltProcessOneNode, vFppp)
GO(xsltProfileStylesheet, pFpppp)
GO(xsltQuoteOneUserParam, iFppp)
GO(xsltQuoteUserParams, iFpp)
GO(xsltRegisterAllElement, vFp)
GO(xsltRegisterAllExtras, vFv)
//GO(xsltRegisterAllFunctions, 
//GOM(xsltRegisterExtElement, iFEpppB)
GOM(xsltRegisterExtFunction, iFEpppp)
//GOM(xsltRegisterExtModule, iFEpBB)
//GOM(xsltRegisterExtModuleElement, iFEppBB)
//GOM(xsltRegisterExtModuleFull, iFEpBBBB)
GOM(xsltRegisterExtModuleFunction, iFEppp)
//GOM(xsltRegisterExtModuleTopLevel, iFEppB)
GO(xsltRegisterExtPrefix, iFppp)
GO(xsltRegisterExtras, vFp)
GO(xsltRegisterLocalRVT, iFpp)
GO(xsltRegisterPersistRVT, iFpp)
GO(xsltRegisterTestModule, vFv)
GO(xsltRegisterTmpRVT, iFpp)
GO(xsltReleaseRVT, vFpp)
//GO(xsltResolveStylesheetAttributeSet, 
GO(xsltRestoreDocumentNamespaces, iFpp)
GO(xsltRunStylesheet, iFpppppp)
GO(xsltRunStylesheetUser, iFpppppppp)
GO(xsltSaveProfiling, vFpp)
GO(xsltSaveResultTo, iFppp)
GO(xsltSaveResultToFd, iFipp)
GO(xsltSaveResultToFile, iFppp)
GO(xsltSaveResultToFilename, iFpppi)
GO(xsltSaveResultToString, iFpppp)
GO(xsltSecurityAllow, iFppp)
GO(xsltSecurityForbid, iFppp)
GO(xsltSetCtxtParseOptions, iFpi)
GO(xsltSetCtxtSecurityPrefs, iFpp)
GOM(xsltSetCtxtSortFunc, vFEpp)
//GOM(xsltSetDebuggerCallbacks, iFip)
GO(xsltSetDebuggerStatus, vFi)
GO(xsltSetDefaultSecurityPrefs, vFp)
//GOM(xsltSetGenericDebugFunc, vFEpB)
//GOM(xsltSetGenericErrorFunc, vFEpB)
GOM(xsltSetLoaderFunc, vFEp)
GOM(xsltSetSecurityPrefs, iFEppp)
//GOM(xsltSetSortFunc, vFEp)
//GO(MxsltSetTransformErrorFunc, vFEppB)
GO(xsltSetXIncludeDefault, vFi)
GO(xsltShutdownCtxtExts, vFp)
GO(xsltShutdownExts, vFp)
GO(xsltSort, vFpppp)
GO(xsltSplitQName, pFppp)
//GO(xsltStrxfrm, 
GO(xsltStyleGetExtData, pFpp)
//GO(xsltStylePreCompute, 
GO(xsltStyleStylesheetLevelGetExtData, pFpp)
//GO(xsltSystemPropertyFunction, 
//GO(xsltTemplateProcess, 
//GO(xsltTestCompMatchList, 
GO(xsltText, vFpppp)
GO(xsltTimestamp, lFv)
GO(xsltTransformError, vFpppppppppppppppp)  //vaarg
GO(xsltUninit, vFv)
//GO(xsltUnparsedEntityURIFunction, 
GO(xsltUnregisterExtModule, iFp)
GO(xsltUnregisterExtModuleElement, iFpp)
GO(xsltUnregisterExtModuleFunction, iFpp)
GO(xsltUnregisterExtModuleTopLevel, iFpp)
GO(xsltValueOf, vFpppp)
GO(xsltVariableLookup, pFppp)
GO(xsltXPathCompile, pFpp)
GO(xsltXPathCompileFlags, pFppi)
GO(xsltXPathFunctionLookup, pFppp)
GO(xsltXPathGetTransformContext, pFp)
//GO(xsltXPathVariableLookup, 