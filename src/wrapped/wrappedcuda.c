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

#define ADDED_SUPER 1
#include "wrappercallback.h"

void fillCUDAProcWrapper(box64context_t* context);
static void* resolveSymbol(x64emu_t* emu, void* symbol, const char* rname, const char* name, int version);

static void cuda_wrapper_resolver(x64emu_t* emu, const char* name, void** pfn, int cudaVersion)
{
    if(!emu->context->cudawrappers)   // could be moved in "my" structure...
        fillCUDAProcWrapper(emu->context);

    const char* fname = name; // default is name given
    if(*pfn) {
        // try to find the right wrapper, first get the real name of the function if possible
        Dl_info infos = {0};
        if(!dladdr(*pfn, &infos)) {
            //is there a name we can use?
            if(infos.dli_sname && infos.dli_saddr==*pfn) {
                //we have a name!
                fname = infos.dli_sname; // use it
            }
        }
        khint_t k = kh_get(symbolmap, emu->context->mycuda, fname);
        int is_my = (k==kh_end(emu->context->mycuda))?0:1;
        void* symbol = *pfn;
        if(!symbol) {
            printf_dlsym_prefix(0, LOG_DEBUG, "%p\n", NULL);
            return;    // easy
        }
        if(is_my) {
            // try again, by using custom "my_" now...
            char tmp[200];
            strcpy(tmp, "my_");
            strcat(tmp, fname);
            symbol = dlsym(emu->context->box64lib, tmp);
            // need to update symbol link maybe
            #define GO(A, W) if(!strcmp(fname, #A)) my->A = *pfn;
            SUPER()
            #undef GO
        }
        *pfn = resolveSymbol(emu, symbol, fname, name, cudaVersion);
    }
}
#undef SUPER

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
// CUasyncNotificationInfo
#define GO(A)   \
static uintptr_t my_CUasyncNotificationInfo_fct_##A = 0;                \
static void my_CUasyncNotificationInfo_##A(void* a, void* b, void* c)   \
{                                                                       \
    RunFunctionFmt(my_CUasyncNotificationInfo_fct_##A, "ppp", a, b, c); \
}
SUPER()
#undef GO
static void* find_CUasyncNotificationInfo_Fct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_CUasyncNotificationInfo_fct_##A == (uintptr_t)fct) return my_CUasyncNotificationInfo_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_CUasyncNotificationInfo_fct_##A == 0) {my_CUasyncNotificationInfo_fct_##A = (uintptr_t)fct; return my_CUasyncNotificationInfo_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for cuda CUasyncNotificationInfo callback\n");
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

EXPORT uint32_t my_cuUserObjectCreate(x64emu_t* emu, void* obj, void* ptr, void* d, uint32_t ref, uint32_t flags)
{
    return my->cuUserObjectCreate(obj, ptr, find_CuHostFn_Fct(d), ref, flags);
}

static void* resolveSymbol(x64emu_t* emu, void* symbol, const char* rname, const char* name, int version)
{
    // get wrapper
    symbol1_t *s = NULL;
    khint_t k = kh_get(symbolmap, emu->context->cudawrappers, rname);
    if(k==kh_end(emu->context->cudawrappers)) {
        printf_dlsym_prefix(0, LOG_DEBUG, "%p\n", NULL);
        printf_dlsym(LOG_INFO, "Warning, no wrapper for %s(%s version %d)\n", rname, name, version);
        return NULL;
    }
    s = &kh_value(emu->context->cudawrappers, k);

    if(!s->resolved) {
        khint_t k = kh_get(symbolmap, emu->context->cudawrappers, rname);
        const char* constname = kh_key(emu->context->cudawrappers, k);
        s->addr = AddCheckBridge(emu->context->system, s->w, symbol, 0, constname);
        s->resolved = 1;
    }
    void* ret = (void*)s->addr;
    printf_dlsym_prefix(0, LOG_DEBUG, "%p (%p)\n", ret, symbol);
    return ret;
}

EXPORT uint32_t my_cuGetProcAddress(x64emu_t* emu, char* name, void** pfn, int cudaVersion, uint64_t flags, void* status)
{
    printf_dlsym(LOG_DEBUG, "Calling cuGetProcAddress(%s, %p, %i, %llx, %p) ", name, pfn, cudaVersion, flags, status);
    uint32_t ret = my->cuGetProcAddress(name, pfn, cudaVersion, flags, status);
    cuda_wrapper_resolver(emu, name, pfn, cudaVersion);
    return ret;
}
EXPORT uint32_t my_cuGetProcAddress_v2(x64emu_t* emu, char* name, void** pfn, int cudaVersion, uint64_t flags, void* status)
{
    printf_dlsym(LOG_DEBUG, "Calling cuGetProcAddress_v2(%s, %p, %i, %llx, %p) ", name, pfn, cudaVersion, flags, status);
    uint32_t ret = my->cuGetProcAddress_v2(name, pfn, cudaVersion, flags, status);
    cuda_wrapper_resolver(emu, name, pfn, cudaVersion);
    return ret;
}

EXPORT uint32_t my_cuDeviceRegisterAsyncNotification(x64emu_t* emu, void* device, void* f, void* data, void* handle)
{
    return my->cuDeviceRegisterAsyncNotification(device, find_CUasyncNotificationInfo_Fct(f), data, handle);
}

#include "wrappedlib_init.h"

void fillCUDAProcWrapper(box64context_t* context)
{
    int cnt, ret;
    khint_t k;
    kh_symbolmap_t * symbolmap = kh_init(symbolmap);
    // populates maps...
    cnt = sizeof(cudasymbolmap)/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, symbolmap, cudasymbolmap[i].name, &ret);
        kh_value(symbolmap, k).w = cudasymbolmap[i].w;
        kh_value(symbolmap, k).resolved = 0;
    }
    // and the my_ symbols map
    cnt = sizeof(MAPNAME(mysymbolmap))/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, symbolmap, cudamysymbolmap[i].name, &ret);
        kh_value(symbolmap, k).w = cudamysymbolmap[i].w;
        kh_value(symbolmap, k).resolved = 0;
    }
    context->cudawrappers = symbolmap;
    // fill my_* map
    symbolmap = kh_init(symbolmap);
    cnt = sizeof(MAPNAME(mysymbolmap))/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, symbolmap, cudamysymbolmap[i].name, &ret);
        kh_value(symbolmap, k).w = cudamysymbolmap[i].w;
        kh_value(symbolmap, k).resolved = 0;
    }
    context->mycuda = symbolmap;
}
void freeCUDAProcWrapper(box64context_t* context)
{
    if(!context)
        return;
    if(context->cudawrappers)
        kh_destroy(symbolmap, context->cudawrappers);
    if(context->mycuda)
        kh_destroy(symbolmap, context->mycuda);
    context->cudawrappers = NULL;
    context->mycuda = NULL;
}

// see https://github.com/SveSop/nvidia-libs for nvidia libs installation in wine