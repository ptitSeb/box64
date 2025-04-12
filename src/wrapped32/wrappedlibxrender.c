#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"


#include "debug.h"
#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box32context.h"
#include "emu/x64emu_private.h"
#include "converter32.h"

#ifdef ANDROID
    static const char* libxrenderName = "libXrender.so";
#else
    static const char* libxrenderName = "libXrender.so.1";
#endif

#define LIBNAME libxrender

#ifdef ANDROID
#define NEEDED_LIBS "libX11.so"
#else
#define NEEDED_LIBS "libX11.so.6"
#endif

#include "libtools/my_x11_conv.h"

#include "generated/wrappedlibxrendertypes32.h"

#include "wrappercallback32.h"

KHASH_MAP_INIT_INT64(picformat, void*);
static kh_picformat_t*   hash_picformat;

EXPORT void* my32_XRenderFindFormat(x64emu_t* emu, void* dpy, unsigned long mask, void* tmpl, int count)
{
    void* ret = my->XRenderFindFormat(dpy, mask, tmpl, count);
    if(!ret) return NULL;
    khint_t k = kh_get(picformat, hash_picformat, (uintptr_t)ret);
    if(k!=kh_end(hash_picformat))
        return kh_value(hash_picformat, k);
    int r;
    k = kh_put(picformat, hash_picformat, (uintptr_t)ret, &r);
    struct_LiiwwwwwwwwL_t* res = calloc(1, sizeof(struct_LiiwwwwwwwwL_t));
    to_struct_LiiwwwwwwwwL(to_ptrv(res), ret);
    kh_value(hash_picformat, k) = res;
    return res;
}

EXPORT void* my32_XRenderFindStandardFormat(x64emu_t* emu, void* dpy, int fmt)
{
    void* ret = my->XRenderFindStandardFormat(dpy, fmt);
    if(!ret) return NULL;
    khint_t k = kh_get(picformat, hash_picformat, (uintptr_t)ret);
    if(k!=kh_end(hash_picformat))
        return kh_value(hash_picformat, k);
    int r;
    k = kh_put(picformat, hash_picformat, (uintptr_t)ret, &r);
    struct_LiiwwwwwwwwL_t* res = calloc(1, sizeof(struct_LiiwwwwwwwwL_t));
    to_struct_LiiwwwwwwwwL(to_ptrv(res), ret);
    kh_value(hash_picformat, k) = res;
    return res;
}

EXPORT void* my32_XRenderFindVisualFormat(x64emu_t* emu, void* dpy, void* visual)
{
    void* ret = my->XRenderFindVisualFormat(dpy, convert_Visual_to_64(dpy, visual));
    if(!ret) return NULL;
    khint_t k = kh_get(picformat, hash_picformat, (uintptr_t)ret);
    if(k!=kh_end(hash_picformat))
        return kh_value(hash_picformat, k);
    int r;
    k = kh_put(picformat, hash_picformat, (uintptr_t)ret, &r);
    struct_LiiwwwwwwwwL_t* res = calloc(1, sizeof(struct_LiiwwwwwwwwL_t));
    to_struct_LiiwwwwwwwwL(to_ptrv(res), ret);
    kh_value(hash_picformat, k) = res;
    return res;
}

EXPORT void* my32_XRenderQueryFilters(x64emu_t* emu, void* dpy, unsigned long drawable)
{
    void* ret = my->XRenderQueryFilters(dpy, drawable);
    return inplace_XFilters_shrink(ret);
}

#define CUSTOM_INIT                                     \
    hash_picformat = kh_init(picformat);                \

#define CUSTOM_FINI                                     \
    void* p;                                            \
    kh_foreach_value(hash_picformat, p, free(p));       \
    kh_destroy(picformat, hash_picformat);              \
    hash_picformat = NULL;                              \

#include "wrappedlib_init32.h"
