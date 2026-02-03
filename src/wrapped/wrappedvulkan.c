#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "box64context.h"
#include "librarian.h"
#include "callback.h"
#include "myalign.h"
#include "build_info.h"

//extern char* libvulkan;

const char* vulkanName = "libvulkan.so.1";
#define LIBNAME vulkan

typedef void(*vFpUp_t)      (void*, uint64_t, void*);

#define ADDED_FUNCTIONS()                           \

#include "generated/wrappedvulkantypes.h"

#define ADDED_SUPER 1
#include "wrappercallback.h"

void fillVulkanProcWrapper(box64context_t*);
void freeVulkanProcWrapper(box64context_t*);

static symbol1_t* getWrappedSymbol(x64emu_t* emu, const char* rname, int warning)
{
    khint_t k = kh_get(symbolmap, emu->context->vkwrappers, rname);
    if(k==kh_end(emu->context->vkwrappers) && strstr(rname, "KHR")==NULL) {
        // try again, adding KHR at the end if not present
        char tmp[200];
        strcpy(tmp, rname);
        strcat(tmp, "KHR");
        k = kh_get(symbolmap, emu->context->vkwrappers, tmp);
    }
    if(k==kh_end(emu->context->vkwrappers)) {
        if(warning) {
            printf_dlsym_prefix(0, LOG_DEBUG, "%p\n", NULL);
            printf_dlsym(LOG_INFO, "Warning, no wrapper for %s\n", rname);
        }
        return NULL;
    }
    return &kh_value(emu->context->vkwrappers, k);
}

static void* resolveSymbol(x64emu_t* emu, void* symbol, void* fnc, const char* rname)
{
    // get wrapper
    symbol1_t *s = getWrappedSymbol(emu, rname, 1);

    khint_t k = kh_get(symbolmap, emu->context->vkwrappers, rname);
    const char* constname = kh_key(emu->context->vkwrappers, k);
    s->addr = AddCheckBridge2(emu->context->system, s->w, symbol, fnc, 0, constname);

    void* ret = (void*)s->addr;
    printf_dlsym_prefix(0, LOG_DEBUG, "%p (%p)\n", ret, symbol);
    return ret;
}

EXPORT void* my_vkGetDeviceProcAddr(x64emu_t* emu, void* device, void* name)
{
    khint_t k;
    const char* rname = (const char*)name;

    pFpp_t getprocaddr = getBridgeFnc2((void*)R_RIP);
    if(!getprocaddr) getprocaddr=my->vkGetDeviceProcAddr;

    printf_dlsym(LOG_DEBUG, "Calling my_vkGetDeviceProcAddr[%p](%p, \"%s\") => ", getprocaddr, device, rname);
    if(!emu->context->vkwrappers)
        fillVulkanProcWrapper(emu->context);

    k = kh_get(symbolmap, emu->context->vkmymap, rname);
    int is_my = (k==kh_end(emu->context->vkmymap))?0:1;
    void* symbol = getprocaddr(device, name);
    void* fnc = NULL;
    if(symbol && is_my) {   // only wrap if symbol exist
        // try again, by using custom "my_" now...
        char tmp[200];
        strcpy(tmp, "my_");
        strcat(tmp, rname);
        fnc = symbol;
        symbol = dlsym(emu->context->box64lib, tmp);
        // need to update symbol link maybe
        #define GO(A, W) if(!strcmp(rname, #A)) my->A = (W)getprocaddr(device, name);
        SUPER()
        #undef GO
    }
    if(!symbol) {
        printf_dlsym_prefix(0, LOG_DEBUG, "%p\n", NULL);
        return NULL;    // easy
    }
    return resolveSymbol(emu, symbol, fnc, rname);
}

EXPORT void* my_vkGetInstanceProcAddr(x64emu_t* emu, void* instance, void* name)
{
    khint_t k;
    const char* rname = (const char*)name;

   pFpp_t getprocaddr = getBridgeFnc2((void*)R_RIP);
   if(!getprocaddr) getprocaddr=(pFpp_t)my_context->vkprocaddress;

   printf_dlsym(LOG_DEBUG, "Calling my_vkGetInstanceProcAddr[%p](%p, \"%s\") => ", getprocaddr, instance, rname);
    if(!emu->context->vkwrappers)
        fillVulkanProcWrapper(emu->context);

    // check if vkprocaddress is filled, and search for lib and fill it if needed
    // get proc adress using actual glXGetProcAddress
    k = kh_get(symbolmap, emu->context->vkmymap, rname);
    int is_my = (k==kh_end(emu->context->vkmymap))?0:1;
    void* symbol = getprocaddr(instance, (void*)rname);
    void* fnc = NULL;
    if(!symbol) {
        printf_dlsym_prefix(0, LOG_DEBUG, "%p\n", NULL);
        return NULL;    // easy
    }
    if(is_my) {
        // try again, by using custom "my_" now...
        char tmp[200];
        strcpy(tmp, "my_");
        strcat(tmp, rname);
        fnc = symbol;
        symbol = dlsym(emu->context->box64lib, tmp);
        // need to update symbol link maybe
        #define GO(A, W) if(!strcmp(rname, #A)) my->A = (W)getprocaddr(instance, (void*)rname);;
        SUPER()
        #undef GO
    }
    return resolveSymbol(emu, symbol, fnc, rname);
}

void* my_GetVkProcAddr(x64emu_t* emu, void* name, void*(*getaddr)(const char*))
{
    khint_t k;
    const char* rname = (const char*)name;

    printf_dlsym(LOG_DEBUG, "Calling my_GetVkProcAddr(\"%s\", %p) => ", rname, getaddr);
    if(!emu->context->vkwrappers)
        fillVulkanProcWrapper(emu->context);

    // check if vkprocaddress is filled, and search for lib and fill it if needed
    // get proc adress using actual glXGetProcAddress
    k = kh_get(symbolmap, emu->context->vkmymap, rname);
    int is_my = (k==kh_end(emu->context->vkmymap))?0:1;
    void* symbol = getaddr(rname);
    if(!symbol) {
        printf_dlsym_prefix(0, LOG_DEBUG, "%p\n", NULL);
        return NULL;    // easy
    }
    void* fnc = NULL;
    if(is_my) {
        // try again, by using custom "my_" now...
        char tmp[200];
        strcpy(tmp, "my_");
        strcat(tmp, rname);
        fnc = symbol;
        symbol = dlsym(emu->context->box64lib, tmp);
        // need to update symbol link maybe
        #define GO(A, W) if(!strcmp(rname, #A)) my->A = (W)getaddr(rname);
        SUPER()
        #undef GO
    }
    return resolveSymbol(emu, symbol, fnc, rname);
}

void* my_GetVkProcAddr2(x64emu_t* emu, void* a, void* name, void*(*getaddr)(void* a, const char*))
{
    khint_t k;
    const char* rname = (const char*)name;

    printf_dlsym(LOG_DEBUG, "Calling my_GetVkProcAddr2(%p, \"%s\", %p) => ", a, rname, getaddr);
    if(!emu->context->vkwrappers)
        fillVulkanProcWrapper(emu->context);

    // get proc adress using actual glXGetProcAddress
    k = kh_get(symbolmap, emu->context->vkmymap, rname);
    int is_my = (k==kh_end(emu->context->vkmymap))?0:1;
    void* symbol = getaddr(a, rname);
    if(!symbol) {
        printf_dlsym_prefix(0, LOG_DEBUG, "%p\n", NULL);
        return NULL;    // easy
    }
    void* fnc = NULL;
    if(is_my) {
        // try again, by using custom "my_" now...
        char tmp[200];
        strcpy(tmp, "my_");
        strcat(tmp, rname);
        fnc = symbol;
        symbol = dlsym(emu->context->box64lib, tmp);
        // need to update symbol link maybe
        #define GO(A, W) if(!strcmp(rname, #A)) my->A = (W)getaddr(a, rname);
        SUPER()
        #undef GO
    }
    return resolveSymbol(emu, symbol, fnc, rname);
}

#undef SUPER

typedef struct my_VkAllocationCallbacks_s {
    void*   pUserData;
    void*   pfnAllocation;
    void*   pfnReallocation;
    void*   pfnFree;
    void*   pfnInternalAllocation;
    void*   pfnInternalFree;
} my_VkAllocationCallbacks_t;

typedef struct my_VkDebugUtilsMessengerCreateInfoEXT_s {
    int          sType;
    const void*  pNext;
    int          flags;
    int          messageSeverity;
    int          messageType;
    void*        pfnUserCallback;
    void*        pUserData;
} my_VkDebugUtilsMessengerCreateInfoEXT_t;

typedef struct my_VkDebugReportCallbackCreateInfoEXT_s {
    int         sType;
    const void* pNext;
    int         flags;
    void*       pfnCallback;
    void*       pUserData;
} my_VkDebugReportCallbackCreateInfoEXT_t;

typedef struct my_VkXcbSurfaceCreateInfoKHR_s {
    int         sType;
    const void* pNext;
    uint32_t    flags;
    void**      connection;
    int         window;
} my_VkXcbSurfaceCreateInfoKHR_t;

#define VK_MAX_DRIVER_NAME_SIZE 256
#define VK_MAX_DRIVER_INFO_SIZE 256

typedef struct my_VkPhysicalDeviceVulkan12Properties_s {
    int   sType;
    void* pNext;
    int   driverID;
    char  driverName[VK_MAX_DRIVER_NAME_SIZE];
    char  driverInfo[VK_MAX_DRIVER_INFO_SIZE];
    uint32_t __others[49];
} my_VkPhysicalDeviceVulkan12Properties_t;

typedef struct my_VkStruct_s {
    int         sType;
    struct my_VkStruct_s* pNext;
} my_VkStruct_t;

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// Allocation ...
#define GO(A)   \
static uintptr_t my_Allocation_fct_##A = 0;                                             \
static void* my_Allocation_##A(void* a, size_t b, size_t c, int d)                      \
{                                                                                       \
    return (void*)RunFunctionFmt(my_Allocation_fct_##A, "pLLi", a, b, c, d);      \
}
SUPER()
#undef GO
static void* find_Allocation_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_Allocation_fct_##A == (uintptr_t)fct) return my_Allocation_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Allocation_fct_##A == 0) {my_Allocation_fct_##A = (uintptr_t)fct; return my_Allocation_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Vulkan Allocation callback\n");
    return NULL;
}
// Reallocation ...
#define GO(A)   \
static uintptr_t my_Reallocation_fct_##A = 0;                                                   \
static void* my_Reallocation_##A(void* a, void* b, size_t c, size_t d, int e)                   \
{                                                                                               \
    return (void*)RunFunctionFmt(my_Reallocation_fct_##A, "ppLLi", a, b, c, d, e);        \
}
SUPER()
#undef GO
static void* find_Reallocation_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_Reallocation_fct_##A == (uintptr_t)fct) return my_Reallocation_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Reallocation_fct_##A == 0) {my_Reallocation_fct_##A = (uintptr_t)fct; return my_Reallocation_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Vulkan Reallocation callback\n");
    return NULL;
}
// Free ...
#define GO(A)   \
static uintptr_t my_Free_fct_##A = 0;                       \
static void my_Free_##A(void* a, void* b)                   \
{                                                           \
    RunFunctionFmt(my_Free_fct_##A, "pp", a, b);      \
}
SUPER()
#undef GO
static void* find_Free_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_Free_fct_##A == (uintptr_t)fct) return my_Free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Free_fct_##A == 0) {my_Free_fct_##A = (uintptr_t)fct; return my_Free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Vulkan Free callback\n");
    return NULL;
}
// InternalAllocNotification ...
#define GO(A)   \
static uintptr_t my_InternalAllocNotification_fct_##A = 0;                                  \
static void my_InternalAllocNotification_##A(void* a, size_t b, int c, int d)               \
{                                                                                           \
    RunFunctionFmt(my_InternalAllocNotification_fct_##A, "pLii", a, b, c, d);         \
}
SUPER()
#undef GO
static void* find_InternalAllocNotification_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_InternalAllocNotification_fct_##A == (uintptr_t)fct) return my_InternalAllocNotification_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_InternalAllocNotification_fct_##A == 0) {my_InternalAllocNotification_fct_##A = (uintptr_t)fct; return my_InternalAllocNotification_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Vulkan InternalAllocNotification callback\n");
    return NULL;
}
// InternalFreeNotification ...
#define GO(A)   \
static uintptr_t my_InternalFreeNotification_fct_##A = 0;                                   \
static void my_InternalFreeNotification_##A(void* a, size_t b, int c, int d)                \
{                                                                                           \
    RunFunctionFmt(my_InternalFreeNotification_fct_##A, "pLii", a, b, c, d);          \
}
SUPER()
#undef GO
static void* find_InternalFreeNotification_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_InternalFreeNotification_fct_##A == (uintptr_t)fct) return my_InternalFreeNotification_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_InternalFreeNotification_fct_##A == 0) {my_InternalFreeNotification_fct_##A = (uintptr_t)fct; return my_InternalFreeNotification_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Vulkan InternalFreeNotification callback\n");
    return NULL;
}
// DebugReportCallbackEXT ...
#define GO(A)   \
static uintptr_t my_DebugReportCallbackEXT_fct_##A = 0;                                                         \
static int my_DebugReportCallbackEXT_##A(int a, int b, uint64_t c, size_t d, int e, void* f, void* g, void* h)  \
{                                                                                                               \
    return RunFunctionFmt(my_DebugReportCallbackEXT_fct_##A, "iiULippp", a, b, c, d, e, f, g, h);         \
}
SUPER()
#undef GO
static void* find_DebugReportCallbackEXT_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DebugReportCallbackEXT_fct_##A == (uintptr_t)fct) return my_DebugReportCallbackEXT_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DebugReportCallbackEXT_fct_##A == 0) {my_DebugReportCallbackEXT_fct_##A = (uintptr_t)fct; return my_DebugReportCallbackEXT_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Vulkan DebugReportCallbackEXT callback\n");
    return NULL;
}
// DebugUtilsMessengerCallback ...
#define GO(A)   \
static uintptr_t my_DebugUtilsMessengerCallback_fct_##A = 0;                            \
static int my_DebugUtilsMessengerCallback_##A(int a, int b, void* c, void* d)           \
{                                                                                       \
    return RunFunctionFmt(my_DebugUtilsMessengerCallback_fct_##A, "iipp", a, b, c, d);  \
}
SUPER()
#undef GO
static void* find_DebugUtilsMessengerCallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DebugUtilsMessengerCallback_fct_##A == (uintptr_t)fct) return my_DebugUtilsMessengerCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DebugUtilsMessengerCallback_fct_##A == 0) {my_DebugUtilsMessengerCallback_fct_##A = (uintptr_t)fct; return my_DebugUtilsMessengerCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Vulkan DebugUtilsMessengerCallback callback\n");
    return NULL;
}

#undef SUPER

//#define PRE_INIT if(libGL) {lib->w.lib = dlopen(libGL, RTLD_LAZY | RTLD_GLOBAL); lib->path = box_strdup(libGL);} else

#define PRE_INIT           \
    if(BOX64ENV(novulkan)) \
        return -1;

#define CUSTOM_INIT \
    lib->w.priv = dlsym(lib->w.lib, "vkGetInstanceProcAddr"); \
    box64->vkprocaddress = lib->w.priv;

#include "wrappedlib_init.h"

void fillVulkanProcWrapper(box64context_t* context)
{
    int cnt, ret;
    khint_t k;
    kh_symbolmap_t * symbolmap = kh_init(symbolmap);
    // populates maps...
    cnt = sizeof(vulkansymbolmap)/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, symbolmap, vulkansymbolmap[i].name, &ret);
        kh_value(symbolmap, k).w = vulkansymbolmap[i].w;
        kh_value(symbolmap, k).resolved = 0;
    }
    // and the my_ symbols map
    cnt = sizeof(MAPNAME(mysymbolmap))/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, symbolmap, vulkanmysymbolmap[i].name, &ret);
        kh_value(symbolmap, k).w = vulkanmysymbolmap[i].w;
        kh_value(symbolmap, k).resolved = 0;
    }
    context->vkwrappers = symbolmap;
    // my_* map
    symbolmap = kh_init(symbolmap);
    cnt = sizeof(MAPNAME(mysymbolmap))/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, symbolmap, vulkanmysymbolmap[i].name, &ret);
        kh_value(symbolmap, k).w = vulkanmysymbolmap[i].w;
        kh_value(symbolmap, k).resolved = 0;
    }
    context->vkmymap = symbolmap;
}
void freeVulkanProcWrapper(box64context_t* context)
{
    if(!context)
        return;
    if(context->vkwrappers)
        kh_destroy(symbolmap, context->vkwrappers);
    if(context->vkmymap)
        kh_destroy(symbolmap, context->vkmymap);
    context->vkwrappers = NULL;
    context->vkmymap = NULL;
}

my_VkAllocationCallbacks_t* find_VkAllocationCallbacks(my_VkAllocationCallbacks_t* dest, my_VkAllocationCallbacks_t* src)
{
    if(!src) return src;
    dest->pUserData = src->pUserData;
    dest->pfnAllocation = find_Allocation_Fct(src->pfnAllocation);
    dest->pfnReallocation = find_Reallocation_Fct(src->pfnReallocation);
    dest->pfnFree = find_Free_Fct(src->pfnFree);
    dest->pfnInternalAllocation = find_InternalAllocNotification_Fct(src->pfnInternalAllocation);
    dest->pfnInternalFree = find_InternalFreeNotification_Fct(src->pfnInternalFree);
    return dest;
}
// functions....
#define CREATE(A)   \
EXPORT int my_##A(x64emu_t* emu, void* device, void* pAllocateInfo, my_VkAllocationCallbacks_t* pAllocator, void* p)    \
{                                                                                                                       \
    my_VkAllocationCallbacks_t my_alloc;                                                                                \
    iFpppp_t fnc = getBridgeFnc2((void*)R_RIP);                                                                         \
    if(!fnc) fnc=my->A;                                                                                                 \
    return fnc(device, pAllocateInfo, find_VkAllocationCallbacks(&my_alloc, pAllocator), p);                            \
}
#define DESTROY(A)   \
EXPORT void my_##A(x64emu_t* emu, void* device, void* p, my_VkAllocationCallbacks_t* pAllocator)                        \
{                                                                                                                       \
    my_VkAllocationCallbacks_t my_alloc;                                                                                \
    vFppp_t fnc = getBridgeFnc2((void*)R_RIP);                                                                          \
    if(!fnc) fnc=my->A;                                                                                                 \
    fnc(device, p, find_VkAllocationCallbacks(&my_alloc, pAllocator));                                                  \
}
#define IDESTROY(A)   \
EXPORT int my_##A(x64emu_t* emu, void* device, void* p, my_VkAllocationCallbacks_t* pAllocator)                         \
{                                                                                                                       \
    my_VkAllocationCallbacks_t my_alloc;                                                                                \
    iFppp_t fnc = getBridgeFnc2((void*)R_RIP);                                                                          \
    if(!fnc) fnc=my->A;                                                                                                 \
    return fnc(device, p, find_VkAllocationCallbacks(&my_alloc, pAllocator));                                           \
}
#define DESTROY64(A)   \
EXPORT void my_##A(x64emu_t* emu, void* device, uint64_t p, my_VkAllocationCallbacks_t* pAllocator)                     \
{                                                                                                                       \
    my_VkAllocationCallbacks_t my_alloc;                                                                                \
    vFpUp_t fnc = getBridgeFnc2((void*)R_RIP);                                                                          \
    if(!fnc) fnc=my->A;                                                                                                 \
    fnc(device, p, find_VkAllocationCallbacks(&my_alloc, pAllocator));                                                  \
}

CREATE(vkAllocateMemory)
CREATE(vkCreateBuffer)
CREATE(vkCreateBufferView)
CREATE(vkCreateCommandPool)

EXPORT int my_vkCreateComputePipelines(x64emu_t* emu, void* device, uint64_t pipelineCache, uint32_t count, void* pCreateInfos, my_VkAllocationCallbacks_t* pAllocator, void* pPipelines)
{
    my_VkAllocationCallbacks_t my_alloc;
    iFpUuppp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->vkCreateComputePipelines;
    int ret = fnc(device, pipelineCache, count, pCreateInfos, find_VkAllocationCallbacks(&my_alloc, pAllocator), pPipelines);
    return ret;
}

CREATE(vkCreateDescriptorPool)
CREATE(vkCreateDescriptorSetLayout)
CREATE(vkCreateDescriptorUpdateTemplate)
CREATE(vkCreateDescriptorUpdateTemplateKHR)
CREATE(vkCreateDevice)

EXPORT int my_vkCreateDisplayModeKHR(x64emu_t* emu, void* physical, uint64_t display, void* pCreateInfo, my_VkAllocationCallbacks_t* pAllocator, void* pMode)
{
    my_VkAllocationCallbacks_t my_alloc;
    iFpUppp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->vkCreateDisplayModeKHR;
    return fnc(physical, display, pCreateInfo, find_VkAllocationCallbacks(&my_alloc, pAllocator), pMode);
}

CREATE(vkCreateDisplayPlaneSurfaceKHR)
CREATE(vkCreateEvent)
CREATE(vkCreateFence)
CREATE(vkCreateFramebuffer)

EXPORT int my_vkCreateGraphicsPipelines(x64emu_t* emu, void* device, uint64_t pipelineCache, uint32_t count, void* pCreateInfos, my_VkAllocationCallbacks_t* pAllocator, void* pPipelines)
{
    my_VkAllocationCallbacks_t my_alloc;
    iFpUuppp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->vkCreateGraphicsPipelines;
    int ret = fnc(device, pipelineCache, count, pCreateInfos, find_VkAllocationCallbacks(&my_alloc, pAllocator), pPipelines);
    return ret;
}

CREATE(vkCreateImage)
CREATE(vkCreateImageView)

#define VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT 1000011000
#define VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT 1000128004
EXPORT int my_vkCreateInstance(x64emu_t* emu, void* pCreateInfos, my_VkAllocationCallbacks_t* pAllocator, void* pInstance)
{
    iFppp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->vkCreateInstance;
    my_VkAllocationCallbacks_t my_alloc;
    my_VkStruct_t *p = (my_VkStruct_t*)pCreateInfos;
    void* old[20] = {0};
    int old_i = 0;
    while(p) {
        if(p->sType==VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT) {
            my_VkDebugReportCallbackCreateInfoEXT_t* vk = (my_VkDebugReportCallbackCreateInfoEXT_t*)p;
            old[old_i] = vk->pfnCallback;
            vk->pfnCallback = find_DebugReportCallbackEXT_Fct(old[old_i]);
            old_i++;
        } else if(p->sType==VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT) {
            my_VkDebugUtilsMessengerCreateInfoEXT_t* vk = (my_VkDebugUtilsMessengerCreateInfoEXT_t*)p;
            old[old_i] = vk->pfnUserCallback;
            vk->pfnUserCallback = find_DebugUtilsMessengerCallback_Fct(old[old_i]);
            old_i++;
        }
        p = p->pNext;
    }
    int ret = fnc(pCreateInfos, find_VkAllocationCallbacks(&my_alloc, pAllocator), pInstance);
    if(old_i) {// restore, just in case it's re-used?
        p = (my_VkStruct_t*)pCreateInfos;
        old_i = 0;
        while(p) {
            if(p->sType==VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT) {
                my_VkDebugReportCallbackCreateInfoEXT_t* vk = (my_VkDebugReportCallbackCreateInfoEXT_t*)p;
                vk->pfnCallback = old[old_i];
                old_i++;
            } else if(p->sType==VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT) {
                my_VkDebugUtilsMessengerCreateInfoEXT_t* vk = (my_VkDebugUtilsMessengerCreateInfoEXT_t*)p;
                vk->pfnUserCallback = old[old_i];
                old_i++;
            }
            p = p->pNext;
        }
    }
    return ret;
}

CREATE(vkCreatePipelineCache)
CREATE(vkCreatePipelineLayout)
CREATE(vkCreateQueryPool)
CREATE(vkCreateRenderPass)
CREATE(vkCreateSampler)
CREATE(vkCreateSamplerYcbcrConversion)
CREATE(vkCreateSemaphore)
CREATE(vkCreateShaderModule)

EXPORT int my_vkCreateSharedSwapchainsKHR(x64emu_t* emu, void* device, uint32_t count, void** pCreateInfos, my_VkAllocationCallbacks_t* pAllocator, void* pSwapchains)
{
    iFpuppp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->vkCreateSharedSwapchainsKHR;
    my_VkAllocationCallbacks_t my_alloc;
    int ret = fnc(device, count, pCreateInfos, find_VkAllocationCallbacks(&my_alloc, pAllocator), pSwapchains);
    return ret;
}

CREATE(vkCreateSwapchainKHR)
CREATE(vkCreateWaylandSurfaceKHR)
EXPORT int my_vkCreateXcbSurfaceKHR(x64emu_t* emu, void* instance, void* info, my_VkAllocationCallbacks_t* pAllocator, void* pFence)
{
    iFpppp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->vkCreateXcbSurfaceKHR;
    my_VkAllocationCallbacks_t my_alloc;
    my_VkXcbSurfaceCreateInfoKHR_t* surfaceinfo = info;
    void* old_conn = surfaceinfo->connection;
    surfaceinfo->connection = align_xcb_connection(old_conn);
    int ret = fnc(instance, info, find_VkAllocationCallbacks(&my_alloc, pAllocator), pFence);
    surfaceinfo->connection = old_conn;
    return ret;
}
CREATE(vkCreateXlibSurfaceKHR)
CREATE(vkCreateAndroidSurfaceKHR)
CREATE(vkCreateRenderPass2)
CREATE(vkCreateRenderPass2KHR)

EXPORT int my_vkRegisterDeviceEventEXT(x64emu_t* emu, void* device, void* info, my_VkAllocationCallbacks_t* pAllocator, void* pFence)
{
    iFpppp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->vkRegisterDeviceEventEXT;
    my_VkAllocationCallbacks_t my_alloc;
    return fnc(device, info, find_VkAllocationCallbacks(&my_alloc, pAllocator), pFence);
}
EXPORT int my_vkRegisterDisplayEventEXT(x64emu_t* emu, void* device, uint64_t disp, void* info, my_VkAllocationCallbacks_t* pAllocator, void* pFence)
{
    iFpUppp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->vkRegisterDisplayEventEXT;
    my_VkAllocationCallbacks_t my_alloc;
    return fnc(device, disp, info, find_VkAllocationCallbacks(&my_alloc, pAllocator), pFence);
}

CREATE(vkCreateValidationCacheEXT)

EXPORT int my_vkCreateShadersEXT(x64emu_t* emu, void* device, uint32_t count, void** pCreateInfos, my_VkAllocationCallbacks_t* pAllocator, void* pShaders)
{
    iFpuppp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->vkCreateShadersEXT;
    my_VkAllocationCallbacks_t my_alloc;
    int ret = fnc(device, count, pCreateInfos, find_VkAllocationCallbacks(&my_alloc, pAllocator), pShaders);
    return ret;
}

EXPORT int my_vkCreateExecutionGraphPipelinesAMDX(x64emu_t* emu, void* device, uint64_t pipelineCache, uint32_t count, void** pCreateInfos, my_VkAllocationCallbacks_t* pAllocator, void* pPipeLines)
{
    iFpUuppp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->vkCreateExecutionGraphPipelinesAMDX;
    my_VkAllocationCallbacks_t my_alloc;
    int ret = fnc(device, pipelineCache, count, pCreateInfos, find_VkAllocationCallbacks(&my_alloc, pAllocator), pPipeLines);
    return ret;
}

DESTROY64(vkDestroyShaderEXT)


DESTROY64(vkDestroyBuffer)
DESTROY64(vkDestroyBufferView)
DESTROY64(vkDestroyCommandPool)
DESTROY64(vkDestroyDescriptorPool)
DESTROY64(vkDestroyDescriptorSetLayout)
DESTROY64(vkDestroyDescriptorUpdateTemplate)
DESTROY64(vkDestroyDescriptorUpdateTemplateKHR)

EXPORT void my_vkDestroyDevice(x64emu_t* emu, void* pDevice, my_VkAllocationCallbacks_t* pAllocator)
{
    vFpp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->vkDestroyDevice;
    my_VkAllocationCallbacks_t my_alloc;
    fnc(pDevice, find_VkAllocationCallbacks(&my_alloc, pAllocator));
}

DESTROY64(vkDestroyEvent)
DESTROY64(vkDestroyFence)
DESTROY64(vkDestroyFramebuffer)
DESTROY64(vkDestroyImage)
DESTROY64(vkDestroyImageView)

EXPORT void my_vkDestroyInstance(x64emu_t* emu, void* instance, my_VkAllocationCallbacks_t* pAllocator)
{
    vFpp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->vkDestroyInstance;
    my_VkAllocationCallbacks_t my_alloc;
    fnc(instance, find_VkAllocationCallbacks(&my_alloc, pAllocator));
}

DESTROY64(vkDestroyPipeline)
DESTROY64(vkDestroyPipelineCache)
DESTROY64(vkDestroyPipelineLayout)
DESTROY64(vkDestroyQueryPool)
DESTROY64(vkDestroyRenderPass)
DESTROY64(vkDestroySampler)
DESTROY64(vkDestroySamplerYcbcrConversion)
DESTROY64(vkDestroySemaphore)
DESTROY64(vkDestroyShaderModule)
DESTROY64(vkDestroySwapchainKHR)

DESTROY64(vkFreeMemory)

EXPORT int my_vkCreateDebugUtilsMessengerEXT(x64emu_t* emu, void* device, my_VkDebugUtilsMessengerCreateInfoEXT_t* pAllocateInfo, my_VkAllocationCallbacks_t* pAllocator, void* p)
{
    iFpppp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->vkCreateDebugUtilsMessengerEXT;
    #define VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT 1000128004
    my_VkAllocationCallbacks_t my_alloc;
    my_VkDebugUtilsMessengerCreateInfoEXT_t* info = pAllocateInfo;
    while(info && info->sType==VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT) {
        info->pfnUserCallback = find_DebugUtilsMessengerCallback_Fct(info->pfnUserCallback);
        info = (my_VkDebugUtilsMessengerCreateInfoEXT_t*)info->pNext;
    }
    return fnc(device, pAllocateInfo, find_VkAllocationCallbacks(&my_alloc, pAllocator), p);
}
DESTROY(vkDestroyDebugUtilsMessengerEXT)

DESTROY64(vkDestroySurfaceKHR)

CREATE(vkCreateSamplerYcbcrConversionKHR)
DESTROY64(vkDestroySamplerYcbcrConversionKHR)

DESTROY64(vkDestroyValidationCacheEXT)

CREATE(vkCreateVideoSessionKHR)
CREATE(vkCreateVideoSessionParametersKHR)
DESTROY64(vkDestroyVideoSessionKHR)
DESTROY64(vkDestroyVideoSessionParametersKHR)

CREATE(vkCreatePrivateDataSlot)
CREATE(vkCreatePrivateDataSlotEXT)
DESTROY64(vkDestroyPrivateDataSlot)
DESTROY64(vkDestroyPrivateDataSlotEXT)

CREATE(vkCreateAccelerationStructureKHR)
DESTROY64(vkDestroyAccelerationStructureKHR)

EXPORT int my_vkCreateDeferredOperationKHR(x64emu_t* emu, void* device, my_VkAllocationCallbacks_t* pAllocator, void* p)
{
    iFppp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->vkCreateDeferredOperationKHR;
    my_VkAllocationCallbacks_t my_alloc;
    return fnc(device, find_VkAllocationCallbacks(&my_alloc, pAllocator), p);
}
DESTROY64(vkDestroyDeferredOperationKHR)

EXPORT int my_vkCreateRayTracingPipelinesKHR(x64emu_t* emu, void* device, uint64_t op, uint64_t pipeline, uint32_t count, void* infos, my_VkAllocationCallbacks_t* pAllocator, void* p)
{
    iFpUUuppp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->vkCreateRayTracingPipelinesKHR;
    my_VkAllocationCallbacks_t my_alloc;
    return fnc(device, op, pipeline, count, infos, find_VkAllocationCallbacks(&my_alloc, pAllocator), p);
}

CREATE(vkCreateCuFunctionNVX)
CREATE(vkCreateCuModuleNVX)
DESTROY64(vkDestroyCuFunctionNVX)
DESTROY64(vkDestroyCuModuleNVX)

CREATE(vkCreateIndirectCommandsLayoutNV)
DESTROY64(vkDestroyIndirectCommandsLayoutNV)

CREATE(vkCreateAccelerationStructureNV)
EXPORT int my_vkCreateRayTracingPipelinesNV(x64emu_t* emu, void* device, uint64_t pipeline, uint32_t count, void* infos, my_VkAllocationCallbacks_t* pAllocator, void* p)
{
    iFpUuppp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->vkCreateRayTracingPipelinesNV;
    my_VkAllocationCallbacks_t my_alloc;
    return fnc(device, pipeline, count, infos, find_VkAllocationCallbacks(&my_alloc, pAllocator), p);
}
DESTROY64(vkDestroyAccelerationStructureNV)


CREATE(vkCreateOpticalFlowSessionNV)
DESTROY64(vkDestroyOpticalFlowSessionNV)

CREATE(vkCreateMicromapEXT)
DESTROY64(vkDestroyMicromapEXT)

CREATE(vkCreateCudaFunctionNV)
CREATE(vkCreateCudaModuleNV)
DESTROY64(vkDestroyCudaFunctionNV)
DESTROY64(vkDestroyCudaModuleNV)

EXPORT int my_vkCreateDebugReportCallbackEXT(x64emu_t* emu, void* instance,
                                             my_VkDebugReportCallbackCreateInfoEXT_t* create,
                                             my_VkAllocationCallbacks_t* alloc, void* callback)
{
    iFpppp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->vkCreateDebugReportCallbackEXT;
    my_VkDebugReportCallbackCreateInfoEXT_t dbg = *create;
    my_VkAllocationCallbacks_t my_alloc;
    dbg.pfnCallback = find_DebugReportCallbackEXT_Fct(dbg.pfnCallback);
    return fnc(instance, &dbg, find_VkAllocationCallbacks(&my_alloc, alloc), callback);
}

EXPORT void my_vkDestroyDebugReportCallbackEXT(x64emu_t* emu, void* instance, void* callback, void* alloc)
{
    vFppp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->vkDestroyDebugReportCallbackEXT;
    my_VkAllocationCallbacks_t my_alloc;
    fnc(instance, callback, find_VkAllocationCallbacks(&my_alloc, alloc));
}

CREATE(vkCreateHeadlessSurfaceEXT)

EXPORT void my_vkGetPhysicalDeviceProperties2(x64emu_t* emu, void* device, void* pProps)
{
    vFpp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->vkGetPhysicalDeviceProperties2;
    fnc(device, pProps);
    my_VkStruct_t *p = pProps;
    while (p != NULL) {
        // find VkPhysicalDeviceVulkan12Properties
        // VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES = 52
        if(p->sType == 52) {
            my_VkPhysicalDeviceVulkan12Properties_t *pp = (my_VkPhysicalDeviceVulkan12Properties_t*)p;
            strncat(pp->driverInfo, " with " BOX64_BUILD_INFO_STRING, VK_MAX_DRIVER_INFO_SIZE - strlen(pp->driverInfo) - 1);
            break;
        }
        p = p->pNext;
    }
}

CREATE(vkCreateIndirectCommandsLayoutEXT)
CREATE(vkCreateIndirectExecutionSetEXT)
DESTROY64(vkDestroyIndirectCommandsLayoutEXT)
DESTROY64(vkDestroyIndirectExecutionSetEXT)

CREATE(vkCreatePipelineBinariesKHR)
DESTROY64(vkDestroyPipelineBinaryKHR)
IDESTROY(vkReleaseCapturedPipelineDataKHR)

CREATE(vkCreateTensorARM)
CREATE(vkCreateTensorViewARM)
DESTROY64(vkDestroyTensorARM)
DESTROY64(vkDestroyTensorViewARM)

CREATE(vkCreateDataGraphPipelineSessionARM)
EXPORT int my_vkCreateDataGraphPipelinesARM(x64emu_t* emu, void* device, uint64_t deferredOperation, uint64_t pipelineCache,
                                             uint32_t createInfoCount, void* pCreateInfos,
                                             my_VkAllocationCallbacks_t* alloc, void* pPipelines)
{
    iFpUUuppp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->vkCreateDataGraphPipelinesARM;
    my_VkAllocationCallbacks_t my_alloc;
    return fnc(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, find_VkAllocationCallbacks(&my_alloc, alloc), pPipelines);
}
DESTROY64(vkDestroyDataGraphPipelineSessionARM)

CREATE(vkCreateWin32SurfaceKHR)