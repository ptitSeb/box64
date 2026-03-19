#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <math.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "debug.h"
#include "emu/x64emu_private.h"

const char* libmvecName = "libmvec.so.1";
#define LIBNAME libmvec

// GCC vector type: 2 packed doubles in 128 bits.
// Matches x86 SSE2 __m128d and aarch64 NEON float64x2_t layout.
typedef double v2df __attribute__((vector_size(16)));
typedef v2df (*v2df_func_t)(v2df);

static v2df_func_t native_v2d_sin = NULL;
static v2df_func_t native_v2d_cos = NULL;

// Native libmvec symbol names per architecture.
// aarch64 NEON: _ZGVnN2v_* (n=NEON, N=no mask, 2=two doubles, v=vector arg)
// x86_64 SSE2:  _ZGVbN2v_* (b=SSE,  N=no mask, 2=two doubles, v=vector arg)
#ifdef ARM64
#define NATIVE_SIN_NAME "_ZGVnN2v_sin"
#define NATIVE_COS_NAME "_ZGVnN2v_cos"
#else
#define NATIVE_SIN_NAME "_ZGVbN2v_sin"
#define NATIVE_COS_NAME "_ZGVbN2v_cos"
#endif

// _ZGVbN2v_sin: SSE2 vectorized sin(__m128d) -> __m128d
// Input: xmm0 = {double, double}  (2 packed doubles)
// Output: xmm0 = {sin(d0), sin(d1)}
EXPORT void my__ZGVbN2v_sin(x64emu_t* emu)
{
    if (native_v2d_sin) {
        v2df input, result;
        memcpy(&input, &emu->xmm[0], sizeof(v2df));
        result = native_v2d_sin(input);
        memcpy(&emu->xmm[0], &result, sizeof(v2df));
    } else {
        emu->xmm[0].d[0] = sin(emu->xmm[0].d[0]);
        emu->xmm[0].d[1] = sin(emu->xmm[0].d[1]);
    }
}

// _ZGVbN2v_cos: SSE2 vectorized cos(__m128d) -> __m128d
// Input: xmm0 = {double, double}  (2 packed doubles)
// Output: xmm0 = {cos(d0), cos(d1)}
EXPORT void my__ZGVbN2v_cos(x64emu_t* emu)
{
    if (native_v2d_cos) {
        v2df input, result;
        memcpy(&input, &emu->xmm[0], sizeof(v2df));
        result = native_v2d_cos(input);
        memcpy(&emu->xmm[0], &result, sizeof(v2df));
    } else {
        emu->xmm[0].d[0] = cos(emu->xmm[0].d[0]);
        emu->xmm[0].d[1] = cos(emu->xmm[0].d[1]);
    }
}

// Try to load the native libmvec.so.1 and resolve vectorized sin/cos.
// On aarch64 and x86_64: native libmvec exists in glibc, use it.
// On ppc64le and others: dlopen fails, fall back to scalar sin/cos.
#define PRE_INIT                                                          \
    if (1) {                                                              \
        void* native = dlopen("libmvec.so.1", RTLD_LAZY);                 \
        if (native) {                                                     \
            lib->w.lib = native;                                          \
            native_v2d_sin = (v2df_func_t)dlsym(native, NATIVE_SIN_NAME); \
            native_v2d_cos = (v2df_func_t)dlsym(native, NATIVE_COS_NAME); \
        } else {                                                          \
            lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);           \
        }                                                                 \
    } else

#include "wrappedlib_init.h"
