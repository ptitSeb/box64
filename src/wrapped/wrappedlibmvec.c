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

// _ZGV wrappers read vector arguments from guest SIMD registers directly:
// b/SSE uses xmmN; c/AVX and d/AVX2 use xmmN for the low 128 bits and
// ymmN for the high 128 bits. "vv" functions take the second vector from
// xmm1/ymm1 and return the result in xmm0/ymm0.
typedef double v2df __attribute__((vector_size(16)));
typedef v2df (*v2df_func_t)(v2df);
typedef v2df (*v2df_v2df_func_t)(v2df, v2df);

typedef float v4sf __attribute__((vector_size(16)));
typedef v4sf (*v4sf_func_t)(v4sf);
typedef v4sf (*v4sf_v4sf_func_t)(v4sf, v4sf);

static v2df_func_t native_v2d_sin = NULL;
static v2df_func_t native_v2d_cos = NULL;
static v2df_func_t native_v2d_exp = NULL;
static v2df_func_t native_v2d_log = NULL;
static v2df_v2df_func_t native_v2d_pow = NULL;

static v4sf_func_t native_v4f_acosf = NULL;
static v4sf_func_t native_v4f_sinf = NULL;
static v4sf_func_t native_v4f_cosf = NULL;
static v4sf_func_t native_v4f_expf = NULL;
static v4sf_func_t native_v4f_logf = NULL;
static v4sf_v4sf_func_t native_v4f_powf = NULL;

// Native libmvec symbol names per architecture.
// aarch64 NEON: _ZGVnN2v_* (n=NEON, N=no mask, 2=two doubles, v=vector arg)
// x86_64 SSE2:  _ZGVbN2v_* (b=SSE,  N=no mask, 2=two doubles, v=vector arg)
#ifdef ARM64
#define NATIVE_SIN_NAME "_ZGVnN2v_sin"
#define NATIVE_COS_NAME "_ZGVnN2v_cos"
#define NATIVE_EXP_NAME "_ZGVnN2v_exp"
#define NATIVE_LOG_NAME "_ZGVnN2v_log"
#define NATIVE_POW_NAME "_ZGVnN2vv_pow"
#define NATIVE_ACOSF4_NAME "_ZGVnN4v_acosf"
#define NATIVE_SINF4_NAME "_ZGVnN4v_sinf"
#define NATIVE_COSF4_NAME "_ZGVnN4v_cosf"
#define NATIVE_EXPF4_NAME "_ZGVnN4v_expf"
#define NATIVE_LOGF4_NAME "_ZGVnN4v_logf"
#define NATIVE_POWF4_NAME "_ZGVnN4vv_powf"
#else
#define NATIVE_SIN_NAME "_ZGVbN2v_sin"
#define NATIVE_COS_NAME "_ZGVbN2v_cos"
#define NATIVE_EXP_NAME "_ZGVbN2v_exp"
#define NATIVE_LOG_NAME "_ZGVbN2v_log"
#define NATIVE_POW_NAME "_ZGVbN2vv_pow"
#define NATIVE_ACOSF4_NAME "_ZGVbN4v_acosf"
#define NATIVE_SINF4_NAME "_ZGVbN4v_sinf"
#define NATIVE_COSF4_NAME "_ZGVbN4v_cosf"
#define NATIVE_EXPF4_NAME "_ZGVbN4v_expf"
#define NATIVE_LOGF4_NAME "_ZGVbN4v_logf"
#define NATIVE_POWF4_NAME "_ZGVbN4vv_powf"
#endif

static void apply_v2d_unary(x64emu_t* emu, double (*fn)(double))
{
    for (int i = 0; i < 2; ++i)
        emu->xmm[0].d[i] = fn(emu->xmm[0].d[i]);
}

static void apply_v2d_binary(x64emu_t* emu, double (*fn)(double, double))
{
    for (int i = 0; i < 2; ++i)
        emu->xmm[0].d[i] = fn(emu->xmm[0].d[i], emu->xmm[1].d[i]);
}

static void apply_v4f_unary(x64emu_t* emu, float (*fn)(float))
{
    for (int i = 0; i < 4; ++i)
        emu->xmm[0].f[i] = fn(emu->xmm[0].f[i]);
}

static void apply_v4f_binary(x64emu_t* emu, float (*fn)(float, float))
{
    for (int i = 0; i < 4; ++i)
        emu->xmm[0].f[i] = fn(emu->xmm[0].f[i], emu->xmm[1].f[i]);
}

static void apply_v4d_unary(x64emu_t* emu, double (*fn)(double))
{
    for (int i = 0; i < 2; ++i) {
        emu->xmm[0].d[i] = fn(emu->xmm[0].d[i]);
        emu->ymm[0].d[i] = fn(emu->ymm[0].d[i]);
    }
}

static void apply_v4d_binary(x64emu_t* emu, double (*fn)(double, double))
{
    for (int i = 0; i < 2; ++i) {
        emu->xmm[0].d[i] = fn(emu->xmm[0].d[i], emu->xmm[1].d[i]);
        emu->ymm[0].d[i] = fn(emu->ymm[0].d[i], emu->ymm[1].d[i]);
    }
}

static void apply_v8f_unary(x64emu_t* emu, float (*fn)(float))
{
    for (int i = 0; i < 4; ++i) {
        emu->xmm[0].f[i] = fn(emu->xmm[0].f[i]);
        emu->ymm[0].f[i] = fn(emu->ymm[0].f[i]);
    }
}

static void apply_v8f_binary(x64emu_t* emu, float (*fn)(float, float))
{
    for (int i = 0; i < 4; ++i) {
        emu->xmm[0].f[i] = fn(emu->xmm[0].f[i], emu->xmm[1].f[i]);
        emu->ymm[0].f[i] = fn(emu->ymm[0].f[i], emu->ymm[1].f[i]);
    }
}

EXPORT void my__ZGVbN2v_sin(x64emu_t* emu)
{
    if (native_v2d_sin) {
        v2df input, result;
        memcpy(&input, &emu->xmm[0], sizeof(v2df));
        result = native_v2d_sin(input);
        memcpy(&emu->xmm[0], &result, sizeof(v2df));
    } else {
        apply_v2d_unary(emu, sin);
    }
}

EXPORT void my__ZGVbN2v_cos(x64emu_t* emu)
{
    if (native_v2d_cos) {
        v2df input, result;
        memcpy(&input, &emu->xmm[0], sizeof(v2df));
        result = native_v2d_cos(input);
        memcpy(&emu->xmm[0], &result, sizeof(v2df));
    } else {
        apply_v2d_unary(emu, cos);
    }
}

EXPORT void my__ZGVbN2v_exp(x64emu_t* emu)
{
    if (native_v2d_exp) {
        v2df input, result;
        memcpy(&input, &emu->xmm[0], sizeof(v2df));
        result = native_v2d_exp(input);
        memcpy(&emu->xmm[0], &result, sizeof(v2df));
    } else {
        apply_v2d_unary(emu, exp);
    }
}

EXPORT void my__ZGVbN2v_log(x64emu_t* emu)
{
    if (native_v2d_log) {
        v2df input, result;
        memcpy(&input, &emu->xmm[0], sizeof(v2df));
        result = native_v2d_log(input);
        memcpy(&emu->xmm[0], &result, sizeof(v2df));
    } else {
        apply_v2d_unary(emu, log);
    }
}

EXPORT void my__ZGVbN4v_acosf(x64emu_t* emu)
{
    if (native_v4f_acosf) {
        v4sf input, result;
        memcpy(&input, &emu->xmm[0], sizeof(v4sf));
        result = native_v4f_acosf(input);
        memcpy(&emu->xmm[0], &result, sizeof(v4sf));
    } else {
        apply_v4f_unary(emu, acosf);
    }
}

EXPORT void my__ZGVbN4v_sinf(x64emu_t* emu)
{
    if (native_v4f_sinf) {
        v4sf input, result;
        memcpy(&input, &emu->xmm[0], sizeof(v4sf));
        result = native_v4f_sinf(input);
        memcpy(&emu->xmm[0], &result, sizeof(v4sf));
    } else {
        apply_v4f_unary(emu, sinf);
    }
}

EXPORT void my__ZGVbN4v_cosf(x64emu_t* emu)
{
    if (native_v4f_cosf) {
        v4sf input, result;
        memcpy(&input, &emu->xmm[0], sizeof(v4sf));
        result = native_v4f_cosf(input);
        memcpy(&emu->xmm[0], &result, sizeof(v4sf));
    } else {
        apply_v4f_unary(emu, cosf);
    }
}

EXPORT void my__ZGVbN4v_expf(x64emu_t* emu)
{
    if (native_v4f_expf) {
        v4sf input, result;
        memcpy(&input, &emu->xmm[0], sizeof(v4sf));
        result = native_v4f_expf(input);
        memcpy(&emu->xmm[0], &result, sizeof(v4sf));
    } else {
        apply_v4f_unary(emu, expf);
    }
}

EXPORT void my__ZGVbN4v_logf(x64emu_t* emu)
{
    if (native_v4f_logf) {
        v4sf input, result;
        memcpy(&input, &emu->xmm[0], sizeof(v4sf));
        result = native_v4f_logf(input);
        memcpy(&emu->xmm[0], &result, sizeof(v4sf));
    } else {
        apply_v4f_unary(emu, logf);
    }
}

EXPORT void my__ZGVbN2vv_pow(x64emu_t* emu)
{
    if (native_v2d_pow) {
        v2df base, exp, result;
        memcpy(&base, &emu->xmm[0], sizeof(v2df));
        memcpy(&exp, &emu->xmm[1], sizeof(v2df));
        result = native_v2d_pow(base, exp);
        memcpy(&emu->xmm[0], &result, sizeof(v2df));
    } else {
        apply_v2d_binary(emu, pow);
    }
}

EXPORT void my__ZGVbN4vv_powf(x64emu_t* emu)
{
    if (native_v4f_powf) {
        v4sf base, exp, result;
        memcpy(&base, &emu->xmm[0], sizeof(v4sf));
        memcpy(&exp, &emu->xmm[1], sizeof(v4sf));
        result = native_v4f_powf(base, exp);
        memcpy(&emu->xmm[0], &result, sizeof(v4sf));
    } else {
        apply_v4f_binary(emu, powf);
    }
}

EXPORT void my__ZGVcN4v_exp(x64emu_t* emu)
{
    apply_v4d_unary(emu, exp);
}

EXPORT void my__ZGVcN4v_log(x64emu_t* emu)
{
    apply_v4d_unary(emu, log);
}

EXPORT void my__ZGVcN4vv_pow(x64emu_t* emu)
{
    apply_v4d_binary(emu, pow);
}

EXPORT void my__ZGVcN8v_expf(x64emu_t* emu)
{
    apply_v8f_unary(emu, expf);
}

EXPORT void my__ZGVcN8v_logf(x64emu_t* emu)
{
    apply_v8f_unary(emu, logf);
}

EXPORT void my__ZGVcN8vv_powf(x64emu_t* emu)
{
    apply_v8f_binary(emu, powf);
}

EXPORT void my__ZGVdN4v_exp(x64emu_t* emu)
{
    apply_v4d_unary(emu, exp);
}

EXPORT void my__ZGVdN4v_log(x64emu_t* emu)
{
    apply_v4d_unary(emu, log);
}

EXPORT void my__ZGVdN4vv_pow(x64emu_t* emu)
{
    apply_v4d_binary(emu, pow);
}

EXPORT void my__ZGVdN8v_expf(x64emu_t* emu)
{
    apply_v8f_unary(emu, expf);
}

EXPORT void my__ZGVdN8v_logf(x64emu_t* emu)
{
    apply_v8f_unary(emu, logf);
}

EXPORT void my__ZGVdN8vv_powf(x64emu_t* emu)
{
    apply_v8f_binary(emu, powf);
}


// Try to load the native libmvec.so.1 and resolve vectorized math helpers.
// On aarch64 and x86_64: native libmvec exists in glibc, use it.
// On ppc64le and others: dlopen fails, fall back to scalar math functions.
#define PRE_INIT                                                                    \
    if (1) {                                                                        \
        void* native = dlopen("libmvec.so.1", RTLD_LAZY);                           \
        if (native) {                                                               \
            lib->w.lib = native;                                                    \
            native_v2d_sin = (v2df_func_t)dlsym(native, NATIVE_SIN_NAME);           \
            native_v2d_cos = (v2df_func_t)dlsym(native, NATIVE_COS_NAME);           \
            native_v2d_exp = (v2df_func_t)dlsym(native, NATIVE_EXP_NAME);           \
            native_v2d_log = (v2df_func_t)dlsym(native, NATIVE_LOG_NAME);           \
            native_v2d_pow = (v2df_v2df_func_t)dlsym(native, NATIVE_POW_NAME);      \
            native_v4f_acosf = (v4sf_func_t)dlsym(native, NATIVE_ACOSF4_NAME);      \
            native_v4f_sinf = (v4sf_func_t)dlsym(native, NATIVE_SINF4_NAME);        \
            native_v4f_cosf = (v4sf_func_t)dlsym(native, NATIVE_COSF4_NAME);        \
            native_v4f_expf = (v4sf_func_t)dlsym(native, NATIVE_EXPF4_NAME);        \
            native_v4f_logf = (v4sf_func_t)dlsym(native, NATIVE_LOGF4_NAME);        \
            native_v4f_powf = (v4sf_v4sf_func_t)dlsym(native, NATIVE_POWF4_NAME);   \
        } else {                                                                    \
            lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);                     \
        }                                                                           \
    } else

#include "wrappedlib_init.h"
