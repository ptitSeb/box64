#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GO(BrotliDecoderAttachDictionary, iFpuLp)
GOM(BrotliDecoderCreateInstance, pFEppp)
GO(BrotliDecoderDecompress, uFLppp)
GO(BrotliDecoderDecompressStream, uFpppppp)
GO(BrotliDecoderDestroyInstance, vFp)
GO(BrotliDecoderErrorString, pFi)
GO(BrotliDecoderGetErrorCode, iFp)
GO(BrotliDecoderHasMoreOutput, iFp)
GO(BrotliDecoderIsFinished, iFp)
GO(BrotliDecoderIsUsed, iFp)
GOM(BrotliDecoderSetMetadataCallbacks, vFEpppp)
GO(BrotliDecoderSetParameter, iFpuu)
GO(BrotliDecoderTakeOutput, pFpp)
GO(BrotliDecoderVersion, uFv)
