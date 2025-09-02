#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "debug.h"
#include "box64context.h"
#include "emu/x64emu_private.h"
#include "callback.h"
#include "elfloader.h"

const char* cudaName = "libcuda.so.1";
#define LIBNAME cuda

#include "generated/wrappedcudatypes.h"

#include "wrappercallback.h"

#define SUPER() \
    GO(0)   \
    GO(1)   \
    GO(2)   \
    GO(3)

typedef struct my_CUDA_HOST_NODE_PARAMS_v1_s {
    void* f;
    void* data;
} my_CUDA_HOST_NODE_PARAMS_v1_t;

// CUstreamCallback
#define GO(A)   \
static uintptr_t my_CUstreamCallback_fct_##A = 0;                       \
static void my_CUstreamCallback_##A(void* a, uint32_t b, void* c)    \
{                                                                       \
    RunFunctionFmt(my_CUstreamCallback_fct_##A, "pup", a, b, c);        \
}
SUPER()
#undef GO
static void* find_CUstreamCallback_Fct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_CUstreamCallback_fct_##A == (uintptr_t)fct) return my_CUstreamCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_CUstreamCallback_fct_##A == 0) {my_CUstreamCallback_fct_##A = (uintptr_t)fct; return my_CUstreamCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for cuda CUstreamCallback callback\n");
    return NULL;
}
// CuHostFn
#define GO(A)   \
static uintptr_t my_CuHostFn_fct_##A = 0;           \
static void my_CuHostFn_##A(void* a)                \
{                                                   \
    RunFunctionFmt(my_CuHostFn_fct_##A, "p", a);    \
}
SUPER()
#undef GO
static void* find_CuHostFn_Fct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_CuHostFn_fct_##A == (uintptr_t)fct) return my_CuHostFn_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_CuHostFn_fct_##A == 0) {my_CuHostFn_fct_##A = (uintptr_t)fct; return my_CuHostFn_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for cuda CuHostFn callback\n");
    return NULL;
}
static void* reverse_CuHostFn_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_CuHostFn_##A == fct) return (void*)my_CuHostFn_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFp, fct, 0, NULL);
}

#undef SUPER

EXPORT uint32_t my_cuStreamAddCallback(x64emu_t* emu, void* h, void* f, void* data, uint32_t flags)
{
    return my->cuStreamAddCallback(h, find_CUstreamCallback_Fct(f), data, flags);
}

EXPORT uint32_t my_cuStreamAddCallback_ptsz(x64emu_t* emu, void* h, void* f, void* data, uint32_t flags)
{
    return my->cuStreamAddCallback_ptsz(h, find_CUstreamCallback_Fct(f), data, flags);
}

EXPORT uint32_t my_cuGraphAddHostNode(x64emu_t* emu, void* node, void* graph, void* dep, size_t dep_sz, my_CUDA_HOST_NODE_PARAMS_v1_t* param)
{
    my_CUDA_HOST_NODE_PARAMS_v1_t param_ = {0};
    if(param) {
        param_.f = find_CuHostFn_Fct(param->f);
        param_.data = param->data;
    }
    return my->cuGraphAddHostNode(node, graph, dep, dep_sz, param?(&param_):NULL);
}

EXPORT uint32_t my_cuLaunchHostFunc(x64emu_t* emu, void* h, void* f, void* data)
{
    return my->cuLaunchHostFunc(h, find_CuHostFn_Fct(f), data);
}

EXPORT uint32_t my_cuLaunchHostFunc_ptsz(x64emu_t* emu, void* h, void* f, void* data)
{
    return my->cuLaunchHostFunc_ptsz(h, find_CuHostFn_Fct(f), data);
}

EXPORT uint32_t my_cuGraphHostNodeGetParams(x64emu_t* emu, void* node, my_CUDA_HOST_NODE_PARAMS_v1_t* p)
{
    uint32_t ret = my->cuGraphHostNodeGetParams(node, p);
    if(p) p->f = reverse_CuHostFn_Fct(p->f);
    return ret;
}

EXPORT uint32_t my_cuGraphHostNodeSetParams(x64emu_t* emu, void* node, my_CUDA_HOST_NODE_PARAMS_v1_t* p)
{
    my_CUDA_HOST_NODE_PARAMS_v1_t param_ = {0};
    if(p) {
        param_.f = find_CuHostFn_Fct(p->f);
        param_.data = p->data;
    }
    return my->cuGraphHostNodeSetParams(node, p?(&param_):NULL);
}

EXPORT uint32_t my_cuGraphExecHostNodeSetParams(x64emu_t* emu, void* g, void* node, my_CUDA_HOST_NODE_PARAMS_v1_t* p)
{
    my_CUDA_HOST_NODE_PARAMS_v1_t param_ = {0};
    if(p) {
        param_.f = find_CuHostFn_Fct(p->f);
        param_.data = p->data;
    }
    return my->cuGraphExecHostNodeSetParams(g, node, p?(&param_):NULL);
}

#include "wrappedlib_init.h"

// see https://github.com/SveSop/nvidia-libs for nvidia libs installation in wine