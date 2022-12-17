#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GOM(safer_scalable_aligned_realloc, pFpLLp) //%noE
GOM(safer_scalable_free, pFpp) //%noE
GOM(safer_scalable_msize, LFpp) //%noE
GOM(safer_scalable_realloc, pFpLp)  //%noE
GOM(scalable_aligned_free, vFp) //%noE
GOM(scalable_aligned_malloc, pFLL)  //%noE
GOM(scalable_aligned_realloc, pFpLL)    //%noE
//GO(scalable_allocation_command, 
//GO(scalable_allocation_mode, 
GOM(scalable_calloc, pFLL)  //%noE
GO2(scalable_free, vFp, free)
GO2(scalable_malloc, pFL, malloc)
GOM(scalable_msize, LFp)    //%noE
GO2(scalable_posix_memalign, iFpLL, posix_memalign)
GO2(scalable_realloc, pFpL, realloc)
GO2(__TBB_internal_calloc, pFLL, calloc)
GO2(__TBB_internal_free, vFp, free)
GO2(__TBB_internal_malloc, pFL, malloc)
GO2(__TBB_internal_malloc_usable_size, LFp, malloc_usable_size)
GO2(__TBB_internal_posix_memalign, iFpLL, posix_memalign)
GO2(__TBB_internal_realloc, pFpL, realloc)
//GO(_ZN3rml10pool_resetEPNS_10MemoryPoolE, 
//GO(_ZN3rml11pool_createElPKNS_13MemPoolPolicyE, 
//GO(_ZN3rml11pool_mallocEPNS_10MemoryPoolEm, 
//GO(_ZN3rml12pool_destroyEPNS_10MemoryPoolE, 
//GO(_ZN3rml12pool_reallocEPNS_10MemoryPoolEPvm, 
//GO(_ZN3rml14pool_create_v1ElPKNS_13MemPoolPolicyEPPNS_10MemoryPoolE, 
//GO(_ZN3rml19pool_aligned_mallocEPNS_10MemoryPoolEmm, 
//GO(_ZN3rml20pool_aligned_reallocEPNS_10MemoryPoolEPvmm, 
//GO(_ZN3rml9pool_freeEPNS_10MemoryPoolEPv, 
