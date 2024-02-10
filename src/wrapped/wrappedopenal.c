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
#include "callback.h"
#include "box64context.h"
#include "librarian.h"
#include "myalign.h"

const char* openalName = "libopenal.so.1";
#define LIBNAME openal

#include "generated/wrappedopenaltypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// Request ...
#define GO(A)   \
static uintptr_t my_Request_fct_##A = 0;                    \
static void my_Request_##A(int32_t a, int32_t b)            \
{                                                           \
    RunFunctionFmt(my_Request_fct_##A, "ii", a, b);   \
}
SUPER()
#undef GO
static void* find_Request_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_Request_fct_##A == (uintptr_t)fct) return my_Request_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Request_fct_##A == 0) {my_Request_fct_##A = (uintptr_t)fct; return my_Request_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for openal Request callback\n");
    return NULL;
}

#undef SUPER

void* my_alGetProcAddress(x64emu_t* emu, void* name);
void* my_alcGetProcAddress(x64emu_t* emu, void* device, void* name);
void my_alRequestFoldbackStart(x64emu_t *emu, int32_t mode, int32_t count, int32_t length, void* mem, void* cb);
void my_alRequestFoldbackStop(x64emu_t* emu);

#include "wrappedlib_init.h"



void fillALProcWrapper()
{
    int cnt, ret;
    khint_t k;
    kh_symbolmap_t * symbolmap = kh_init(symbolmap);
    // populates maps...
    cnt = sizeof(openalsymbolmap)/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, symbolmap, openalsymbolmap[i].name, &ret);
        kh_value(symbolmap, k).w = openalsymbolmap[i].w;
        kh_value(symbolmap, k).resolved = 0;
    }
    // and the my_ symbols map
    cnt = sizeof(MAPNAME(mysymbolmap))/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, symbolmap, openalmysymbolmap[i].name, &ret);
        kh_value(symbolmap, k).w = openalmysymbolmap[i].w;
        kh_value(symbolmap, k).resolved = 0;
    }
    my_context->alwrappers = symbolmap;
    // fill my_* map
    symbolmap = kh_init(symbolmap);
    cnt = sizeof(MAPNAME(mysymbolmap))/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, symbolmap, openalmysymbolmap[i].name, &ret);
        kh_value(symbolmap, k).w = openalmysymbolmap[i].w;
        kh_value(symbolmap, k).resolved = 0;
    }
    my_context->almymap = symbolmap;
}
void freeALProcWrapper(box64context_t* context)
{
    if(!context)
        return;
    if(context->alwrappers)
        kh_destroy(symbolmap, context->alwrappers);
    if(context->almymap)
        kh_destroy(symbolmap, context->almymap);
    context->alwrappers = NULL;
    context->almymap = NULL;
}

EXPORT void* my_alGetProcAddress(x64emu_t* emu, void* name)
{
    khint_t k;
    const char* rname = (const char*)name;
    printf_log(LOG_DEBUG, "Calling alGetProcAddress(%s)\n", rname);
    if(!emu->context->alwrappers)   // could be moved in "my" structure...
        fillALProcWrapper(emu->context);
    // get proc adress using actual alGetProcAddress
    k = kh_get(symbolmap, emu->context->almymap, rname);
    int is_my = (k==kh_end(emu->context->almymap))?0:1;
    void* symbol;
    if(is_my) {
        // try again, by using custom "my_" now...
        char tmp[200];
        strcpy(tmp, "my_");
        strcat(tmp, rname);
        symbol = dlsym(emu->context->box64lib, tmp);
    } else
        symbol = my->alGetProcAddress(name);
    if(!symbol)
        return NULL;    // easy
    // get wrapper
    k = kh_get(symbolmap, emu->context->alwrappers, rname);
    if(k==kh_end(emu->context->alwrappers)) {
        printf_log(LOG_INFO, "Warning, no wrapper for %s\n", rname);
        return NULL;
    }
    symbol1_t *s = &kh_value(emu->context->alwrappers, k);
    if(!s->resolved) {
        const char* constname = kh_key(emu->context->alwrappers, k);
        s->addr = AddCheckBridge(emu->context->system, s->w, symbol, 0, constname);
        s->resolved = 1;
    }
    return (void*)s->addr;
}

EXPORT void* my_alcGetProcAddress(x64emu_t* emu, void* device, void* name)
{
    khint_t k;
    const char* rname = (const char*)name;
    printf_log(LOG_DEBUG, "Calling alcGetProcAddress(%p, %s)\n", device, rname);
    if(!emu->context->alwrappers)   // could be moved in "my" structure...
        fillALProcWrapper();
    // get proc adress using actual alGetProcAddress
    k = kh_get(symbolmap, emu->context->almymap, rname);
    int is_my = (k==kh_end(emu->context->almymap))?0:1;
    void* symbol;
    if(is_my) {
        // try again, by using custom "my_" now...
        char tmp[200];
        strcpy(tmp, "my_");
        strcat(tmp, rname);
        symbol = dlsym(emu->context->box64lib, tmp);
    } else
        symbol = my->alcGetProcAddress(device, name);
    if(!symbol)
        return NULL;    // easy
    // get wrapper
    k = kh_get(symbolmap, emu->context->alwrappers, rname);
    if(k==kh_end(emu->context->alwrappers)) {
        printf_log(LOG_INFO, "Warning, no wrapper for %s\n", rname);
        return NULL;
    }
    symbol1_t *s = &kh_value(emu->context->alwrappers, k);
    if(!s->resolved) {
        const char* constname = kh_key(emu->context->alwrappers, k);
        s->addr = AddCheckBridge(emu->context->system, s->w, symbol, 0, constname);
        s->resolved = 1;
    }
    return (void*)s->addr;
}

EXPORT void my_alRequestFoldbackStart(x64emu_t *emu, int32_t mode, int32_t count, int32_t length, void* mem, void* cb)
{
    (void)emu;
    my->alRequestFoldbackStart(mode, count, length, mem, find_Request_Fct(cb));
}

EXPORT void my_alRequestFoldbackStop(x64emu_t* emu)
{
    (void)emu;
    my->alRequestFoldbackStop();
}
