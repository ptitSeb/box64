#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

//GO(BZ2_blockSort, 
//GO(BZ2_bsInitWrite, 
//GO(BZ2_bz__AssertH__fail, 
GO(BZ2_bzBuffToBuffCompress, iFpppuiii)
GO(BZ2_bzBuffToBuffDecompress, iFpppuii)
GO(BZ2_bzclose, vFp)
GOM(BZ2_bzCompress, iFEpi)
GOM(BZ2_bzCompressEnd, iFEp)
GOM(BZ2_bzCompressInit, iFEpiii)
GOM(BZ2_bzDecompress, iFEp)
GOM(BZ2_bzDecompressEnd, iFEp)
GOM(BZ2_bzDecompressInit, iFEpii)
GO(BZ2_bzdopen, pFip)
GO(BZ2_bzerror, pFpp)
GO(BZ2_bzflush, iFp)
GO(BZ2_bzlibVersion, pFv)
GO(BZ2_bzopen, pFpp)
GO(BZ2_bzread, iFppi)
GO(BZ2_bzRead, iFpppi)
GO(BZ2_bzReadClose, vFpp)
GO(BZ2_bzReadGetUnused, vFpppp)
GO(BZ2_bzReadOpen, pFpSiipi)
GO(BZ2_bzwrite, iFppi)
GO(BZ2_bzWrite, vFpppi)
GO(BZ2_bzWriteClose, vFppipp)
GO(BZ2_bzWriteClose64, vFppipppp)
GO(BZ2_bzWriteOpen, pFpSiii)
//GO(BZ2_compressBlock, 
//GO(BZ2_decompress, 
//GO(BZ2_hbAssignCodes, 
//GO(BZ2_hbCreateDecodeTables, 
//GO(BZ2_hbMakeCodeLengths, 
//GO(BZ2_indexIntoF, 
