#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <dlfcn.h>

#include "libdl.h"
#include "box64context.h"
#include "debug.h"
#include "x64emu.h"
#include "elfloader.h"
#include "fileutils.h"
#include "emu/x64emu_private.h"
#include "elfhacks.h"

#include "vulkanoverlay.h"

static my_hookedlib_t* hooked_libs = NULL;
static uint32_t hooked_cap = 0;
static uint32_t hooked_size = 0;

#define HOOKLIB 0x1000

static void* AddNewHookedLib(void* priv, const char* path, my_hookedlib_type_t type)
{
    if(hooked_size==hooked_cap) {
        hooked_cap+=4;
        hooked_libs = realloc(hooked_libs, hooked_cap*sizeof(my_hookedlib_t));
    }
    printf_log(LOG_INFO, "Adding Hooked Lib %d: %s\n", hooked_size, path);
    uint32_t i = hooked_size++;
    hooked_libs[i].fullpath = box_strdup(path);
    hooked_libs[i].self = (void*)((uintptr_t)HOOKLIB + i);
    hooked_libs[i].ref = 1;
    hooked_libs[i].type = type;
    hooked_libs[i].priv = priv;
    return hooked_libs[i].self;
}

static int RemoveHookedLib(uint32_t i)
{
    my_hookedlib_type_t type = hooked_libs[i].type;
    void* priv = hooked_libs[i].priv;
    box_free((void*)hooked_libs[i].fullpath);
    hooked_libs[i].fullpath = NULL;
    hooked_libs[i].self = NULL;
    while(hooked_size && !hooked_libs[hooked_size-1].self)
        --hooked_size;
    if(type==VulkanOverlay)
        return FreeVulkanOverlay(priv);
    return 0;
}

static void*(*real_dlopen)(const char*, int) = NULL;
static int (*real_dlclose)(void*) = NULL;
static void* (*real_dlsym)(void*, const char*) = NULL;

static void get_real_dlsym(void)
{
    eh_obj_t libdl;
    int ret;

    const char* libs[] = {
        "*libMangoHud_shim.so*",
#if defined(__GLIBC__)
        "*libdl.so*",
#endif
        "*libc.so*",
        "*libc.*.so*",
        "*ld-musl-*.so*",
    };

    for (size_t i = 0; i < sizeof(libs) / sizeof(*libs); i++) {
        ret = eh_find_obj(&libdl, libs[i]);
        if (ret) {
            continue;
        }
        eh_find_sym(&libdl, "dlsym", (void**)&real_dlsym);
        eh_destroy_obj(&libdl);
        if (real_dlsym) break;
    }
}


EXPORT void* dlopen(const char* path, int flags)
{
    if(!real_dlopen) {
        real_dlopen = GetNativeSymbolUnversioned(RTLD_NEXT, "dlopen");
    }

    // will look only for libs loaded with full path (and only on 64bits for now)
    if(path && strchr(path, '/') && (/*(box64_is32bits && FileIsX86ELF(path)) ||*/ (!box64_is32bits && FileIsX64ELF(path)))) {
        printf_log(LOG_INFO, "Hooked dlopen(%s, 0x%x) on x64 lib\n", path, flags);

        //check if already wrapped, to add a new ref (unless flags is RTLD_NOLOAD)
        for(uint32_t i=0; i<hooked_size; ++i) {
            if(!strcmp(hooked_libs[i].fullpath, path)) {
                if(!(flags&RTLD_NOLOAD))
                    ++hooked_libs[i].ref;
                return hooked_libs[i].self;
            }
        }
        if(flags&RTLD_NOLOAD)
            return NULL;

        void* loaded;
        if((loaded=LoadVulkanOverlay(path, flags)))
            return AddNewHookedLib(loaded, path, VulkanOverlay);
    }

    return real_dlopen(path, flags);
}

EXPORT int dlclose(void* handle)
{
    if (!real_dlclose) {
        real_dlclose = GetNativeSymbolUnversioned(RTLD_NEXT, "dlclose");
    }

    if((uintptr_t)handle>=HOOKLIB && (uintptr_t)handle<HOOKLIB+hooked_size) {
        uint32_t i = (uintptr_t)handle-HOOKLIB;
        int ret = 0;
        if(!(--hooked_libs[i].ref))
            ret = RemoveHookedLib(i);
        return ret;
    }

    return real_dlclose(handle);
}

EXPORT void* dlsym(void* handle, const char* symbol)
{
    if (!real_dlsym) get_real_dlsym();

    if((uintptr_t)handle>=HOOKLIB && (uintptr_t)handle<HOOKLIB+hooked_size) {
        uint32_t i = (uintptr_t)handle-HOOKLIB;
        void* sym = NULL;
        switch(hooked_libs[i].type) {
            case VulkanOverlay: sym = GetSymbolVulkanOverlay(hooked_libs[i].priv, symbol); break;
        }
        return sym;
    }

    return real_dlsym(handle, symbol);
}
EXPORT void* ___dlsym(void *handle, const char* symbol)  __attribute__((alias("dlsym")));