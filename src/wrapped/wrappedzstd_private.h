#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

//GO(COVER_best_destroy, 
//GO(COVER_best_finish, 
//GO(COVER_best_init, 
//GO(COVER_best_start, 
//GO(COVER_best_wait, 
//GO(COVER_checkTotalCompressedSize, 
//GO(COVER_computeEpochs, 
//GO(COVER_dictSelectionError, 
//GO(COVER_dictSelectionFree, 
//GO(COVER_dictSelectionIsError, 
//GO(COVER_selectDict, 
//GO(COVER_sum, 
//GO(COVER_warnOnSmallCorpus, 
//GO(divbwt, 
//GO(divsufsort, 
//GO(ERR_getErrorString, 
//GO(FSE_buildCTable, 
//GO(FSE_buildCTable_raw, 
//GO(FSE_buildCTable_rle, 
//GO(FSE_buildCTable_wksp, 
//GO(FSE_buildDTable, 
//GO(FSE_buildDTable_raw, 
//GO(FSE_buildDTable_rle, 
//GO(FSE_buildDTable_wksp, 
//GO(FSE_compress, 
//GO(FSE_compress2, 
//GO(FSE_compressBound, 
//GO(FSE_compress_usingCTable, 
//GO(FSE_compress_wksp, 
//GO(FSE_createCTable, 
//GO(FSE_createDTable, 
//GO(FSE_decompress, 
//GO(FSE_decompress_usingDTable, 
//GO(FSE_decompress_wksp, 
//GO(FSE_decompress_wksp_bmi2, 
//GO(FSE_freeCTable, 
//GO(FSE_freeDTable, 
//GO(FSE_getErrorName, 
//GO(FSE_isError, 
//GO(FSE_NCountWriteBound, 
//GO(FSE_normalizeCount, 
//GO(FSE_optimalTableLog, 
//GO(FSE_optimalTableLog_internal, 
//GO(FSE_readNCount, 
//GO(FSE_readNCount_bmi2, 
//GO(FSEv05_buildDTable, 
//GO(FSEv05_buildDTable_raw, 
//GO(FSEv05_buildDTable_rle, 
//GO(FSEv05_createDTable, 
//GO(FSEv05_decompress, 
//GO(FSEv05_decompress_usingDTable, 
//GO(FSEv05_freeDTable, 
//GO(FSEv05_getErrorName, 
//GO(FSEv05_isError, 
//GO(FSEv05_readNCount, 
//GO(FSEv06_buildDTable, 
//GO(FSEv06_buildDTable_raw, 
//GO(FSEv06_buildDTable_rle, 
//GO(FSEv06_createDTable, 
//GO(FSEv06_decompress, 
//GO(FSEv06_decompress_usingDTable, 
//GO(FSEv06_freeDTable, 
//GO(FSEv06_getErrorName, 
//GO(FSEv06_isError, 
//GO(FSEv06_readNCount, 
//GO(FSEv07_buildDTable, 
//GO(FSEv07_buildDTable_raw, 
//GO(FSEv07_buildDTable_rle, 
//GO(FSEv07_createDTable, 
//GO(FSEv07_decompress, 
//GO(FSEv07_decompress_usingDTable, 
//GO(FSEv07_freeDTable, 
//GO(FSEv07_getErrorName, 
//GO(FSEv07_isError, 
//GO(FSEv07_readNCount, 
//GO(FSE_versionNumber, 
//GO(FSE_writeNCount, 
//DATAB(g_debuglevel, 
//DATAB(g_ZSTD_threading_useless_symbol, 
//GO(HIST_count, 
//GO(HIST_countFast, 
//GO(HIST_countFast_wksp, 
//GO(HIST_count_simple, 
//GO(HIST_count_wksp, 
//GO(HIST_isError, 
//GO(HUF_buildCTable, 
//GO(HUF_buildCTable_wksp, 
//GO(HUF_compress, 
//GO(HUF_compress1X, 
//GO(HUF_compress1X_repeat, 
//GO(HUF_compress1X_usingCTable, 
//GO(HUF_compress1X_wksp, 
//GO(HUF_compress2, 
//GO(HUF_compress4X_repeat, 
//GO(HUF_compress4X_usingCTable, 
//GO(HUF_compress4X_wksp, 
//GO(HUF_compressBound, 
//GO(HUF_decompress, 
//GO(HUF_decompress1X1, 
//GO(HUF_decompress1X1_DCtx, 
//GO(HUF_decompress1X1_DCtx_wksp, 
//GO(HUF_decompress1X1_DCtx_wksp_bmi2, 
//GO(HUF_decompress1X1_usingDTable, 
//GO(HUF_decompress1X2, 
//GO(HUF_decompress1X2_DCtx, 
//GO(HUF_decompress1X2_DCtx_wksp, 
//GO(HUF_decompress1X2_usingDTable, 
//GO(HUF_decompress1X_DCtx, 
//GO(HUF_decompress1X_DCtx_wksp, 
//GO(HUF_decompress1X_usingDTable, 
//GO(HUF_decompress1X_usingDTable_bmi2, 
//GO(HUF_decompress4X1, 
//GO(HUF_decompress4X1_DCtx, 
//GO(HUF_decompress4X1_DCtx_wksp, 
//GO(HUF_decompress4X1_usingDTable, 
//GO(HUF_decompress4X2, 
//GO(HUF_decompress4X2_DCtx, 
//GO(HUF_decompress4X2_DCtx_wksp, 
//GO(HUF_decompress4X2_usingDTable, 
//GO(HUF_decompress4X_DCtx, 
//GO(HUF_decompress4X_hufOnly, 
//GO(HUF_decompress4X_hufOnly_wksp, 
//GO(HUF_decompress4X_hufOnly_wksp_bmi2, 
//GO(HUF_decompress4X_usingDTable, 
//GO(HUF_decompress4X_usingDTable_bmi2, 
//GO(HUF_estimateCompressedSize, 
//GO(HUF_getErrorName, 
//GO(HUF_getNbBits, 
//GO(HUF_isError, 
//GO(HUF_optimalTableLog, 
//GO(HUF_readCTable, 
//GO(HUF_readDTableX1, 
//GO(HUF_readDTableX1_wksp, 
//GO(HUF_readDTableX1_wksp_bmi2, 
//GO(HUF_readDTableX2, 
//GO(HUF_readDTableX2_wksp, 
//GO(HUF_readStats, 
//GO(HUF_readStats_wksp, 
//GO(HUF_selectDecoder, 
//GO(HUFv05_decompress, 
//GO(HUFv05_decompress1X2, 
//GO(HUFv05_decompress1X2_usingDTable, 
//GO(HUFv05_decompress1X4, 
//GO(HUFv05_decompress1X4_usingDTable, 
//GO(HUFv05_decompress4X2, 
//GO(HUFv05_decompress4X2_usingDTable, 
//GO(HUFv05_decompress4X4, 
//GO(HUFv05_decompress4X4_usingDTable, 
//GO(HUFv05_getErrorName, 
//GO(HUFv05_isError, 
//GO(HUFv05_readDTableX2, 
//GO(HUFv05_readDTableX4, 
//GO(HUFv06_decompress, 
//GO(HUFv06_decompress1X2, 
//GO(HUFv06_decompress1X2_usingDTable, 
//GO(HUFv06_decompress1X4, 
//GO(HUFv06_decompress1X4_usingDTable, 
//GO(HUFv06_decompress4X2, 
//GO(HUFv06_decompress4X2_usingDTable, 
//GO(HUFv06_decompress4X4, 
//GO(HUFv06_decompress4X4_usingDTable, 
//GO(HUFv06_readDTableX2, 
//GO(HUFv06_readDTableX4, 
//GO(HUFv07_decompress, 
//GO(HUFv07_decompress1X2, 
//GO(HUFv07_decompress1X2_DCtx, 
//GO(HUFv07_decompress1X2_usingDTable, 
//GO(HUFv07_decompress1X4, 
//GO(HUFv07_decompress1X4_DCtx, 
//GO(HUFv07_decompress1X4_usingDTable, 
//GO(HUFv07_decompress1X_DCtx, 
//GO(HUFv07_decompress1X_usingDTable, 
//GO(HUFv07_decompress4X2, 
//GO(HUFv07_decompress4X2_DCtx, 
//GO(HUFv07_decompress4X2_usingDTable, 
//GO(HUFv07_decompress4X4, 
//GO(HUFv07_decompress4X4_DCtx, 
//GO(HUFv07_decompress4X4_usingDTable, 
//GO(HUFv07_decompress4X_DCtx, 
//GO(HUFv07_decompress4X_hufOnly, 
//GO(HUFv07_decompress4X_usingDTable, 
//GO(HUFv07_getErrorName, 
//GO(HUFv07_isError, 
//GO(HUFv07_readDTableX2, 
//GO(HUFv07_readDTableX4, 
//GO(HUFv07_readStats, 
//GO(HUFv07_selectDecoder, 
//GO(HUF_validateCTable, 
//GO(HUF_writeCTable, 
//GO(POOL_add, 
//GO(POOL_create, 
//GO(POOL_create_advanced, 
//GO(POOL_free, 
//GO(POOL_resize, 
//GO(POOL_sizeof, 
//GO(POOL_tryAdd, 
//GO(ZBUFF_compressContinue, 
//GO(ZBUFF_compressEnd, 
//GO(ZBUFF_compressFlush, 
//GO(ZBUFF_compressInit, 
//GO(ZBUFF_compressInit_advanced, 
//GO(ZBUFF_compressInitDictionary, 
//GO(ZBUFF_createCCtx, 
//GO(ZBUFF_createCCtx_advanced, 
//GO(ZBUFF_createDCtx, 
//GO(ZBUFF_createDCtx_advanced, 
//GO(ZBUFF_decompressContinue, 
//GO(ZBUFF_decompressInit, 
//GO(ZBUFF_decompressInitDictionary, 
//GO(ZBUFF_freeCCtx, 
//GO(ZBUFF_freeDCtx, 
//GO(ZBUFF_getErrorName, 
//GO(ZBUFF_isError, 
//GO(ZBUFF_recommendedCInSize, 
//GO(ZBUFF_recommendedCOutSize, 
//GO(ZBUFF_recommendedDInSize, 
//GO(ZBUFF_recommendedDOutSize, 
//GO(ZBUFFv05_createDCtx, 
//GO(ZBUFFv05_decompressContinue, 
//GO(ZBUFFv05_decompressInit, 
//GO(ZBUFFv05_decompressInitDictionary, 
//GO(ZBUFFv05_freeDCtx, 
//GO(ZBUFFv05_getErrorName, 
//GO(ZBUFFv05_isError, 
//GO(ZBUFFv05_recommendedDInSize, 
//GO(ZBUFFv05_recommendedDOutSize, 
//GO(ZBUFFv06_createDCtx, 
//GO(ZBUFFv06_decompressContinue, 
//GO(ZBUFFv06_decompressInit, 
//GO(ZBUFFv06_decompressInitDictionary, 
//GO(ZBUFFv06_freeDCtx, 
//GO(ZBUFFv06_getErrorName, 
//GO(ZBUFFv06_isError, 
//GO(ZBUFFv06_recommendedDInSize, 
//GO(ZBUFFv06_recommendedDOutSize, 
//GO(ZBUFFv07_createDCtx, 
//GO(ZBUFFv07_createDCtx_advanced, 
//GO(ZBUFFv07_decompressContinue, 
//GO(ZBUFFv07_decompressInit, 
//GO(ZBUFFv07_decompressInitDictionary, 
//GO(ZBUFFv07_freeDCtx, 
//GO(ZBUFFv07_getErrorName, 
//GO(ZBUFFv07_isError, 
//GO(ZBUFFv07_recommendedDInSize, 
//GO(ZBUFFv07_recommendedDOutSize, 
//GO(ZDICT_addEntropyTablesFromBuffer, 
GO(ZDICT_finalizeDictionary, LFpLpLppuH) // last param afet LFpLpLppu is ZDICT_params_t, a struct with iuu
GO(ZDICT_getDictHeaderSize, LFpL)
GO(ZDICT_getDictID, uFpL)
GO(ZDICT_getErrorName, pFL)
GO(ZDICT_isError, uFL)
//GO(ZDICT_optimizeTrainFromBuffer_cover, 
//GO(ZDICT_optimizeTrainFromBuffer_fastCover, 
GO(ZDICT_trainFromBuffer, LFpLppu)
//GO(ZDICT_trainFromBuffer_cover, 
//GO(ZDICT_trainFromBuffer_fastCover, 
//GO(ZDICT_trainFromBuffer_legacy, 
//GO(ZDICT_trainFromBuffer_unsafe_legacy, 
//GO(ZSTD_adjustCParams, 
//GO(ZSTD_buildCTable, 
//GO(ZSTD_buildFSETable, 
GO(ZSTD_CCtx_getParameter, LFpup)
GO(ZSTD_CCtx_loadDictionary, LFppL)
GO(ZSTD_CCtx_loadDictionary_advanced, LFppLuu)
GO(ZSTD_CCtx_loadDictionary_byReference, LFppL)
GO(ZSTD_CCtxParams_getParameter, LFpup)
GO(ZSTD_CCtxParams_init, LFpi)
//GO(ZSTD_CCtxParams_init_advanced, 
GO(ZSTD_CCtxParams_reset, LFp)
GO(ZSTD_CCtxParams_setParameter, LFpui)
GO(ZSTD_CCtx_refCDict, LFpp)
GO(ZSTD_CCtx_refPrefix, LFppL)
GO(ZSTD_CCtx_refPrefix_advanced, LFppLu)
GO(ZSTD_CCtx_refThreadPool, LFpp)
GO(ZSTD_CCtx_reset, LFpu)
GO(ZSTD_CCtx_setParameter, LFpui)
GO(ZSTD_CCtx_setParametersUsingCCtxParams, LFpp)
GO(ZSTD_CCtx_setPledgedSrcSize, LFpU)
//GO(ZSTD_checkContinuity, 
//GO(ZSTD_checkCParams, 
GO(ZSTD_compress, LFpLpLi)
GO(ZSTD_compress2, LFppLpL)
//GO(ZSTD_compress_advanced, 
//GO(ZSTD_compress_advanced_internal, 
GO(ZSTD_compressBegin, LFpi)
//GO(ZSTD_compressBegin_advanced, 
//GO(ZSTD_compressBegin_advanced_internal, 
GO(ZSTD_compressBegin_usingCDict, LFpp)
//GO(ZSTD_compressBegin_usingCDict_advanced, 
GO(ZSTD_compressBegin_usingDict, LFppLi)
GO(ZSTD_compressBlock, LFppLpL)
//GO(ZSTD_compressBlock_btlazy2, 
//GO(ZSTD_compressBlock_btlazy2_dictMatchState, 
//GO(ZSTD_compressBlock_btlazy2_extDict, 
//GO(ZSTD_compressBlock_btopt, 
//GO(ZSTD_compressBlock_btopt_dictMatchState, 
//GO(ZSTD_compressBlock_btopt_extDict, 
//GO(ZSTD_compressBlock_btultra, 
//GO(ZSTD_compressBlock_btultra2, 
//GO(ZSTD_compressBlock_btultra_dictMatchState, 
//GO(ZSTD_compressBlock_btultra_extDict, 
//GO(ZSTD_compressBlock_doubleFast, 
//GO(ZSTD_compressBlock_doubleFast_dictMatchState, 
//GO(ZSTD_compressBlock_doubleFast_extDict, 
//GO(ZSTD_compressBlock_fast, 
//GO(ZSTD_compressBlock_fast_dictMatchState, 
//GO(ZSTD_compressBlock_fast_extDict, 
//GO(ZSTD_compressBlock_greedy, 
//GO(ZSTD_compressBlock_greedy_dedicatedDictSearch, 
//GO(ZSTD_compressBlock_greedy_dictMatchState, 
//GO(ZSTD_compressBlock_greedy_extDict, 
//GO(ZSTD_compressBlock_lazy, 
//GO(ZSTD_compressBlock_lazy2, 
//GO(ZSTD_compressBlock_lazy2_dedicatedDictSearch, 
//GO(ZSTD_compressBlock_lazy2_dictMatchState, 
//GO(ZSTD_compressBlock_lazy2_extDict, 
//GO(ZSTD_compressBlock_lazy_dedicatedDictSearch, 
//GO(ZSTD_compressBlock_lazy_dictMatchState, 
//GO(ZSTD_compressBlock_lazy_extDict, 
GO(ZSTD_compressBound, LFL)
GO(ZSTD_compressCCtx, LFppLpLi)
GO(ZSTD_compressContinue, LFppLpL)
GO(ZSTD_compressEnd, LFppLpL)
//GO(ZSTD_compressLiterals, 
//GO(ZSTD_compressRleLiteralsBlock, 
GO(ZSTD_compressSequences, LFppLpLpL)
GO(ZSTD_compressStream, LFppp)
GO(ZSTD_compressStream2, LFpppu)
GO(ZSTD_compressStream2_simpleArgs, LFppLppLpu)
//GO(ZSTD_compressSuperBlock, 
GO(ZSTD_compress_usingCDict, LFppLpLp)
//GO(ZSTD_compress_usingCDict_advanced, 
GO(ZSTD_compress_usingDict, LFppLpLpLi)
GO(ZSTD_copyCCtx, LFppU)
GO(ZSTD_copyDCtx, vFpp)
//GO(ZSTD_copyDDictParameters, 
GO(ZSTD_cParam_getBounds, HFu)
GO(ZSTD_createCCtx, pFv)
//GO(ZSTD_createCCtx_advanced, 
GO(ZSTD_createCCtxParams, pFv)
GO(ZSTD_createCDict, pFpLi)
//GO(ZSTD_createCDict_advanced, 
//GO(ZSTD_createCDict_advanced2, 
GO(ZSTD_createCDict_byReference, pFpLi)
GO(ZSTD_createCStream, pFv)
//GO(ZSTD_createCStream_advanced, 
GO(ZSTD_createDCtx, pFv)
//GO(ZSTD_createDCtx_advanced, 
GO(ZSTD_createDDict, pFpL)
//GO(ZSTD_createDDict_advanced, 
GO(ZSTD_createDDict_byReference, pFpL)
GO(ZSTD_createDStream, pFv)
//GO(ZSTD_createDStream_advanced, 
//GO(ZSTD_crossEntropyCost, 
GO(ZSTD_CStreamInSize, LFv)
GO(ZSTD_CStreamOutSize, LFv)
//GO(ZSTD_customCalloc, 
//GO(ZSTD_customFree, 
//GO(ZSTD_customMalloc, 
//GO(ZSTD_cycleLog, 
GO(ZSTD_DCtx_getParameter, LFpup)
GO(ZSTD_DCtx_loadDictionary, LFppL)
GO(ZSTD_DCtx_loadDictionary_advanced, LFppLuu)
GO(ZSTD_DCtx_loadDictionary_byReference, LFppL)
GO(ZSTD_DCtx_refDDict, LFpp)
GO(ZSTD_DCtx_refPrefix, LFppL)
GO(ZSTD_DCtx_refPrefix_advanced, LFppLu)
GO(ZSTD_DCtx_reset, LFpu)
GO(ZSTD_DCtx_setFormat, LFpu)
GO(ZSTD_DCtx_setMaxWindowSize, LFpL)
GO(ZSTD_DCtx_setParameter, LFpui)
//GO(ZSTD_DDict_dictContent, 
//GO(ZSTD_DDict_dictSize, 
//GO(ZSTD_decodeLiteralsBlock, 
//GO(ZSTD_decodeSeqHeaders, 
GO(ZSTD_decodingBufferSize_min, LFUU)
GO(ZSTD_decompress, LFpLpL)
GO(ZSTD_decompressBegin, LFp)
GO(ZSTD_decompressBegin_usingDDict, LFpp)
GO(ZSTD_decompressBegin_usingDict, LFppL)
GO(ZSTD_decompressBlock, LFppLpL)
//GO(ZSTD_decompressBlock_internal, 
GO(ZSTD_decompressBound, UFpL)
GO(ZSTD_decompressContinue, LFppLpL)
GO(ZSTD_decompressDCtx, LFppLpL)
GO(ZSTD_decompressStream, LFppp)
GO(ZSTD_decompressStream_simpleArgs, LFppLppLp)
GO(ZSTD_decompress_usingDDict, LFppLpLp)
GO(ZSTD_decompress_usingDict, LFppLpLpL)
//GO(ZSTD_dedicatedDictSearch_lazy_loadDictionary, 
GO(ZSTD_defaultCLevel, iFv)
GO(ZSTD_dParam_getBounds, HFu)
GO(ZSTD_DStreamInSize, LFv)
GO(ZSTD_DStreamOutSize, LFv)
//GO(ZSTD_encodeSequences, 
GO(ZSTD_endStream, LFpp)
GO(ZSTD_estimateCCtxSize, LFi)
GO(ZSTD_estimateCCtxSize_usingCCtxParams, LFp)
//GO(ZSTD_estimateCCtxSize_usingCParams, 
GO(ZSTD_estimateCDictSize, LFLi)
//GO(ZSTD_estimateCDictSize_advanced, 
GO(ZSTD_estimateCStreamSize, LFi)
GO(ZSTD_estimateCStreamSize_usingCCtxParams, LFp)
//GO(ZSTD_estimateCStreamSize_usingCParams, 
GO(ZSTD_estimateDCtxSize, LFv)
GO(ZSTD_estimateDDictSize, LFLu)
GO(ZSTD_estimateDStreamSize, LFL)
GO(ZSTD_estimateDStreamSize_fromFrame, LFpL)
//GO(ZSTD_fillDoubleHashTable, 
//GO(ZSTD_fillHashTable, 
GO(ZSTD_findDecompressedSize, UFpL)
GO(ZSTD_findFrameCompressedSize, LFpL)
GO(ZSTD_flushStream, LFpp)
GO(ZSTD_frameHeaderSize, LFpL)
GO(ZSTD_freeCCtx, LFp)
GO(ZSTD_freeCCtxParams, LFp)
GO(ZSTD_freeCDict, LFp)
GO(ZSTD_freeCStream, LFp)
GO(ZSTD_freeDCtx, LFp)
GO(ZSTD_freeDDict, LFp)
GO(ZSTD_freeDStream, LFp)
//GO(ZSTD_fseBitCost, 
GO(ZSTD_generateSequences, LFppLpL)
GO(ZSTD_getBlockSize, LFp)
//GO(ZSTD_getcBlockSize, 
GO(ZSTD_getCParams, pFpiUL)
//GO(ZSTD_getCParamsFromCCtxParams, 
//GO(ZSTD_getCParamsFromCDict, 
GO(ZSTD_getDecompressedSize, UFpL)
GO(ZSTD_getDictID_fromCDict, uFp)
GO(ZSTD_getDictID_fromDDict, uFp)
GO(ZSTD_getDictID_fromDict, uFpL)
GO(ZSTD_getDictID_fromFrame, uFpL)
GO(ZSTD_getErrorCode, uFL)
GO(ZSTD_getErrorName, pFL)
GO(ZSTD_getErrorString, pFu)
GO(ZSTD_getFrameContentSize, UFpL)
GO(ZSTD_getFrameHeader, LFppL)
GO(ZSTD_getFrameHeader_advanced, LFppLu)
GO(ZSTD_getFrameProgression, pFpp)
GO(ZSTD_getParams, pFpiUL)
//GO(ZSTD_getSeqStore, 
GO(ZSTD_initCStream, LFpi)
//GO(ZSTD_initCStream_advanced, 
//GO(ZSTD_initCStream_internal, 
GO(ZSTD_initCStream_srcSize, LFpiU)
GO(ZSTD_initCStream_usingCDict, LFpp)
//GO(ZSTD_initCStream_usingCDict_advanced, 
GO(ZSTD_initCStream_usingDict, LFppLi)
GO(ZSTD_initDStream, LFp)
GO(ZSTD_initDStream_usingDDict, LFpp)
GO(ZSTD_initDStream_usingDict, LFppL)
GO(ZSTD_initStaticCCtx, pFpL)
//GO(ZSTD_initStaticCDict, 
GO(ZSTD_initStaticCStream, pFpL)
GO(ZSTD_initStaticDCtx, pFpL)
GO(ZSTD_initStaticDDict, pFpLpLuu)
GO(ZSTD_initStaticDStream, pFpL)
//GO(ZSTD_insertAndFindFirstIndex, 
GO(ZSTD_insertBlock, LFppL)
//GO(ZSTD_invalidateRepCodes, 
GO(ZSTD_isError, uFL)
GO(ZSTD_isFrame, uFpL)
//GO(ZSTD_ldm_adjustParameters, 
//GO(ZSTD_ldm_blockCompress, 
//GO(ZSTD_ldm_fillHashTable, 
//GO(ZSTD_ldm_generateSequences, 
//GO(ZSTD_ldm_getMaxNbSeq, 
//GO(ZSTD_ldm_getTableSize, 
//GO(ZSTD_ldm_skipRawSeqStoreBytes, 
//GO(ZSTD_ldm_skipSequences, 
//GO(ZSTD_loadCEntropy, 
//GO(ZSTD_loadDEntropy, 
GO(ZSTD_maxCLevel, iFv)
GO(ZSTD_mergeBlockDelimiters, LFpL)
GO(ZSTD_minCLevel, iFv)
//GO(ZSTDMT_compressStream_generic, 
//GO(ZSTDMT_createCCtx_advanced, 
//GO(ZSTDMT_freeCCtx, 
//GO(ZSTDMT_getFrameProgression, 
//GO(ZSTDMT_initCStream_internal, 
//GO(ZSTDMT_nextInputSizeHint, 
//GO(ZSTDMT_sizeof_CCtx, 
//GO(ZSTDMT_toFlushNow, 
//GO(ZSTDMT_updateCParams_whileCompressing, 
GO(ZSTD_nextInputType, uFp)
GO(ZSTD_nextSrcSizeToDecompress, LFp)
//GO(ZSTD_noCompressLiterals, 
//GO(ZSTD_referenceExternalSequences, 
//GO(ZSTD_reset_compressedBlockState, 
GO(ZSTD_resetCStream, LFpU)
GO(ZSTD_resetDStream, LFp)
//GO(ZSTD_resetSeqStore, 
//GO(ZSTD_selectBlockCompressor, 
//GO(ZSTD_selectEncodingType, 
//GO(ZSTD_seqToCodes, 
GO(ZSTD_sizeof_CCtx, LFp)
GO(ZSTD_sizeof_CDict, LFp)
GO(ZSTD_sizeof_CStream, LFp)
GO(ZSTD_sizeof_DCtx, LFp)
GO(ZSTD_sizeof_DDict, LFp)
GO(ZSTD_sizeof_DStream, LFp)
GO(ZSTD_toFlushNow, LFp)
//GO(ZSTD_updateTree, 
//GO(ZSTDv05_copyDCtx, 
//GO(ZSTDv05_createDCtx, 
//GO(ZSTDv05_decompress, 
//GO(ZSTDv05_decompressBegin, 
//GO(ZSTDv05_decompressBegin_usingDict, 
//GO(ZSTDv05_decompressBlock, 
//GO(ZSTDv05_decompressContinue, 
//GO(ZSTDv05_decompressDCtx, 
//GO(ZSTDv05_decompress_usingDict, 
//GO(ZSTDv05_decompress_usingPreparedDCtx, 
//GO(ZSTDv05_findFrameSizeInfoLegacy, 
//GO(ZSTDv05_freeDCtx, 
//GO(ZSTDv05_getErrorName, 
//GO(ZSTDv05_getFrameParams, 
//GO(ZSTDv05_isError, 
//GO(ZSTDv05_nextSrcSizeToDecompress, 
//GO(ZSTDv05_sizeofDCtx, 
//GO(ZSTDv06_copyDCtx, 
//GO(ZSTDv06_createDCtx, 
//GO(ZSTDv06_decompress, 
//GO(ZSTDv06_decompressBegin, 
//GO(ZSTDv06_decompressBegin_usingDict, 
//GO(ZSTDv06_decompressBlock, 
//GO(ZSTDv06_decompressContinue, 
//GO(ZSTDv06_decompressDCtx, 
//GO(ZSTDv06_decompress_usingDict, 
//GO(ZSTDv06_decompress_usingPreparedDCtx, 
//GO(ZSTDv06_findFrameSizeInfoLegacy, 
//GO(ZSTDv06_freeDCtx, 
//GO(ZSTDv06_getErrorName, 
//GO(ZSTDv06_getFrameParams, 
//GO(ZSTDv06_isError, 
//GO(ZSTDv06_nextSrcSizeToDecompress, 
//GO(ZSTDv06_sizeofDCtx, 
//GO(ZSTDv07_copyDCtx, 
//GO(ZSTDv07_createDCtx, 
//GO(ZSTDv07_createDCtx_advanced, 
//GO(ZSTDv07_createDDict, 
//GO(ZSTDv07_decompress, 
//GO(ZSTDv07_decompressBegin, 
//GO(ZSTDv07_decompressBegin_usingDict, 
//GO(ZSTDv07_decompressBlock, 
//GO(ZSTDv07_decompressContinue, 
//GO(ZSTDv07_decompressDCtx, 
//GO(ZSTDv07_decompress_usingDDict, 
//GO(ZSTDv07_decompress_usingDict, 
//GO(ZSTDv07_estimateDCtxSize, 
//GO(ZSTDv07_findFrameSizeInfoLegacy, 
//GO(ZSTDv07_freeDCtx, 
//GO(ZSTDv07_freeDDict, 
//GO(ZSTDv07_getDecompressedSize, 
//GO(ZSTDv07_getErrorName, 
//GO(ZSTDv07_getFrameParams, 
//GO(ZSTDv07_insertBlock, 
//GO(ZSTDv07_isError, 
//GO(ZSTDv07_isSkipFrame, 
//GO(ZSTDv07_nextSrcSizeToDecompress, 
//GO(ZSTDv07_sizeofDCtx, 
GO(ZSTD_versionNumber, uFv)
GO(ZSTD_versionString, pFv)
//GO(ZSTD_writeLastEmptyBlock, 
//GO(ZSTD_XXH32, 
//GO(ZSTD_XXH32_canonicalFromHash, 
//GO(ZSTD_XXH32_copyState, 
//GO(ZSTD_XXH32_createState, 
//GO(ZSTD_XXH32_digest, 
//GO(ZSTD_XXH32_freeState, 
//GO(ZSTD_XXH32_hashFromCanonical, 
//GO(ZSTD_XXH32_reset, 
//GO(ZSTD_XXH32_update, 
//GO(ZSTD_XXH64, 
//GO(ZSTD_XXH64_canonicalFromHash, 
//GO(ZSTD_XXH64_copyState, 
//GO(ZSTD_XXH64_createState, 
//GO(ZSTD_XXH64_digest, 
//GO(ZSTD_XXH64_freeState, 
//GO(ZSTD_XXH64_hashFromCanonical, 
//GO(ZSTD_XXH64_reset, 
//GO(ZSTD_XXH64_update, 
//GO(ZSTD_XXH_versionNumber, 
