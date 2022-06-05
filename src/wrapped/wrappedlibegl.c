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

const char* libeglName = "libEGL.so.1";
#define LIBNAME libegl

#include "generated/wrappedlibegltypes.h"

#include "wrappercallback.h"

// FIXME: old wrapped* type of file, cannot use generated/wrappedlibgltypes.h

void fillGLProcWrapper(box64context_t*);
void freeProcWrapper(kh_symbolmap_t** symbolmap);

EXPORT void* my_eglGetProcAddress(x64emu_t* emu, void* name) 
{
    khint_t k;
    const char* rname = (const char*)name;
    if(dlsym_error && box64_log<LOG_DEBUG) printf_log(LOG_NONE, "Calling eglGetProcAddress(\"%s\") => ", rname);
    if(!emu->context->glwrappers)
        fillGLProcWrapper(emu->context);
    // check if glxprocaddress is filled, and search for lib and fill it if needed
    // get proc adress using actual glXGetProcAddress
    k = kh_get(symbolmap, emu->context->glmymap, rname);
    int is_my = (k==kh_end(emu->context->glmymap))?0:1;
    void* symbol;
    if(is_my) {
        // try again, by using custom "my_" now...
        char tmp[200];
        strcpy(tmp, "my_");
        strcat(tmp, rname);
        symbol = dlsym(emu->context->box64lib, tmp);
    } else 
        symbol = my->eglGetProcAddress((void*)rname);
    if(!symbol) {
        if(dlsym_error && box64_log<LOG_DEBUG) printf_log(LOG_NONE, "%p\n", NULL);
        return NULL;    // easy
    }
    // check if alread bridged
    uintptr_t ret = CheckBridged(emu->context->system, symbol);
    if(ret) {
        if(dlsym_error && box64_log<LOG_DEBUG) printf_log(LOG_NONE, "%p\n", (void*)ret);
        return (void*)ret; // already bridged
    }
    // get wrapper    
    k = kh_get(symbolmap, emu->context->glwrappers, rname);
    if(k==kh_end(emu->context->glwrappers) && strstr(rname, "ARB")==NULL) {
        // try again, adding ARB at the end if not present
        char tmp[200];
        strcpy(tmp, rname);
        strcat(tmp, "ARB");
        k = kh_get(symbolmap, emu->context->glwrappers, tmp);
    }
    if(k==kh_end(emu->context->glwrappers) && strstr(rname, "EXT")==NULL) {
        // try again, adding EXT at the end if not present
        char tmp[200];
        strcpy(tmp, rname);
        strcat(tmp, "EXT");
        k = kh_get(symbolmap, emu->context->glwrappers, tmp);
    }
    if(k==kh_end(emu->context->glwrappers)) {
        if(dlsym_error && box64_log<LOG_DEBUG) printf_log(LOG_NONE, "%p\n", NULL);
        if(dlsym_error && box64_log<LOG_INFO) printf_log(LOG_NONE, "Warning, no wrapper for %s\n", rname);
        return NULL;
    }
    const char* constname = kh_key(emu->context->glwrappers, k);
    AddOffsetSymbol(emu->context->maplib, symbol, rname);
    ret = AddBridge(emu->context->system, kh_value(emu->context->glwrappers, k), symbol, 0, constname);
    if(dlsym_error && box64_log<LOG_DEBUG) printf_log(LOG_NONE, "%p\n", (void*)ret);
    return (void*)ret;

}


#define CUSTOM_INIT                 \
    getMy(lib);                     \
    if (!box64->glxprocaddress)     \
        box64->glxprocaddress = (procaddess_t)my->eglGetProcAddress;

#define CUSTOM_FINI \
    freeMy();


#include "wrappedlib_init.h"
