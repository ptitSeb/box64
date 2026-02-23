#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

// libomp.so.5 (LLVM OpenMP runtime) is ABI-compatible with libgomp.so.1 (GCC OpenMP runtime)
// for the basic OpenMP runtime symbols used by OldUnreal UT2004.
// Only Engine.so actually imports these 3 symbols; all other .so files list libomp as NEEDED
// but import zero symbols from it.

GOM(GOMP_parallel, vFEppuu)
GO(omp_get_num_threads, iFv)
GO(omp_get_thread_num, iFv)
