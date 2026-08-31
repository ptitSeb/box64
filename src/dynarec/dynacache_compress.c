// SPDX-License-Identifier: MIT
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "dynacache_compress.h"
#include "debug.h"
#include "custommem.h"

static void* libz = NULL;
static int libz_init = 0;

typedef size_t(*LFL_t)(size_t);
typedef int(*iFpppL_t)(void*, void*, void*, size_t);
typedef int(*iFpppLi_t)(void*, void*, void*, size_t, int);

static iFpppL_t libz_uncompress = NULL;
static iFpppLi_t libz_compress2 = NULL;
static LFL_t libz_compressBound = NULL;

static void init_libz()
{
    libz_init = 1;
    libz = dlopen("libz.so.1", RTLD_LAZY);
    if(!libz) libz = dlopen("libz.so", RTLD_LAZY);
    if(!libz) return;
    libz_compress2 = dlsym(libz, "compress2");
    libz_uncompress = dlsym(libz, "uncompress");
    libz_compressBound = dlsym(libz, "compressBound");
}

void* z_compress(void* src, size_t src_size, int type, size_t* dest_size)
{
    if(!libz_init) 
        init_libz();
    if(!libz_compress2 || !libz_compressBound)
        return NULL;
    size_t sz = libz_compressBound(src_size);
    void* dest = box_malloc(sz);
    int ret = libz_compress2(dest, &sz, src, src_size, (type==COMP_Z_MAX)?-1:1);
    if(ret) {
        printf_log(LOG_DEBUG, "Failed to compress a DynaCache block (%d)\n", ret);
        box_free(dest);
        return NULL;
    } else
        *dest_size = sz;
    return dest;
}

void* dc_compress(void* src, size_t src_size, int type, size_t* dest_size)
{
    switch(type) {
        case COMP_Z:
        case COMP_Z_MAX:
            return z_compress(src, src_size, type, dest_size);
        case COMP_NONE:
        default:
            return NULL;
    }
}

int z_uncompress(void* src, size_t src_size, void* dest, size_t dest_size)
{
    if(!libz_init) 
        init_libz();
    if(!libz_uncompress)
        return -1;
    size_t sz = dest_size;
    int ret = libz_uncompress(dest, &sz, src, src_size);
    if(ret) {
        printf_log(LOG_DEBUG, "Failed to uncompress a DynaCache block (%d)\n", ret);
        return -1;
    }
    return 0;
}

int dc_uncompress(void* src, size_t src_size, int type, void* dest, size_t dest_size)
{
    switch(type) {
        case COMP_Z:
        case COMP_Z_MAX:
            return z_uncompress(src, src_size, dest, dest_size);
        case COMP_NONE:
        default:
            return -1;
    }
}
