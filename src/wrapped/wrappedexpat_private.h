#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//GO(_INTERNAL_trim_to_complete_utf8_characters, 
//GO(XML_DefaultCurrent, 
GO(XML_ErrorString, pFi)
GO(XML_ExpatVersion, pFv)
GO(XML_ExpatVersionInfo, pFp)   // return a struct of 3 int. Use shadow pointer on ARM64 and x86_64
GO(XML_ExternalEntityParserCreate, pFppp)
GO(XML_FreeContentModel, vFpp)
GO(XML_GetBase, pFp)
GO(XML_GetBuffer, pFpi)
//GO(XML_GetCurrentByteCount, 
GO(XML_GetCurrentByteIndex, lFp)
GO(XML_GetCurrentColumnNumber, LFp)
GO(XML_GetCurrentLineNumber, iFp)
GO(XML_GetErrorCode, iFp)
GO(XML_GetFeatureList, pFv)
//GO(XML_GetIdAttributeIndex, 
GO(XML_GetInputContext, pFppp)
//GO(XML_GetParsingStatus, 
GO(XML_GetSpecifiedAttributeCount, iFp)
//GO(XML_MemFree, 
//GO(XML_MemMalloc, 
//GO(XML_MemRealloc, 
GO(XML_Parse, iFppii)
GO(XML_ParseBuffer, iFpii)
GO(XML_ParserCreate, pFp)
GO(XML_ParserCreate_MM, pFpppp) // return a structure
//GO(XML_ParserCreateNS, 
GO(XML_ParserFree, vFp)
GO(XML_ParserReset, iFpp)
//GO(XML_ResumeParser, 
GOM(XML_SetAttlistDeclHandler, vFEpp)
GO(XML_SetBase, iFpp)
//GO(XML_SetCdataSectionHandler, 
GOM(XML_SetCharacterDataHandler, vFEpp)
GOM(XML_SetCommentHandler, vFEpp)
GOM(XML_SetDefaultHandler, vFEpp)
GOM(XML_SetDefaultHandlerExpand, vFEpp)
//GO(XML_SetDoctypeDeclHandler, 
GOM(XML_SetElementDeclHandler, vFEpp)
GOM(XML_SetElementHandler, vFEppp)
GO(XML_SetEncoding, iFpp)
GOM(XML_SetEndCdataSectionHandler, vFEpp)
GOM(XML_SetEndDoctypeDeclHandler, vFEpp)
GOM(XML_SetEndElementHandler, vFEpp)
GOM(XML_SetEndNamespaceDeclHandler, vFEpp)
GOM(XML_SetEntityDeclHandler, vFEpp)
GOM(XML_SetExternalEntityRefHandler, vFEpp)
//GO(XML_SetExternalEntityRefHandlerArg, 
GO(XML_SetHashSalt, iFpL)
GOM(XML_SetNamespaceDeclHandler, vFEppp)
GOM(XML_SetNotationDeclHandler, vFEpp)
GOM(XML_SetNotStandaloneHandler, vFEpp)
GO(XML_SetParamEntityParsing, iFpi)
GOM(XML_SetProcessingInstructionHandler, vFEpp)
GO(XML_SetReturnNSTriplet, vFpi)
GOM(XML_SetSkippedEntityHandler, vFEpp)
GOM(XML_SetStartCdataSectionHandler, vFEpp)
GOM(XML_SetStartDoctypeDeclHandler, vFEpp)
GOM(XML_SetStartElementHandler, vFEpp)
GOM(XML_SetStartNamespaceDeclHandler, vFEpp)
GOM(XML_SetUnknownEncodingHandler, vFEppp)
GOM(XML_SetUnparsedEntityDeclHandler, vFEpp)
GO(XML_SetUserData, vFpp)
GOM(XML_SetXmlDeclHandler, vFEpp)
GO(XML_StopParser, iFpi)
GO(XML_UseForeignDTD, iFpi)
//GO(XML_UseParserAsHandlerArg, 
