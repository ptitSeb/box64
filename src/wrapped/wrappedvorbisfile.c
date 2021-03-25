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
#include "bridge.h"

const char* vorbisfileName = "libvorbisfile.so.3";
#define LIBNAME vorbisfile

typedef struct {
  size_t (*read_func)  (void *ptr, size_t size, size_t nmemb, void *datasource);
  int    (*seek_func)  (void *datasource, int64_t offset, int whence);
  int    (*close_func) (void *datasource);
  long   (*tell_func)  (void *datasource);
} ov_callbacks;

typedef int32_t (*iFpppiC_t)(void*, void*, void*, int32_t, ov_callbacks);

typedef struct vorbisfile_my_s {
    // functions
    iFpppiC_t       ov_open_callbacks;
} vorbisfile_my_t;

void* getVorbisfileMy(library_t* lib)
{
    vorbisfile_my_t* my = (vorbisfile_my_t*)calloc(1, sizeof(vorbisfile_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(ov_open_callbacks, iFpppiC_t)
    #undef GO
    return my;
}

void freeVorbisfileMy(void* lib)
{
    //vorbisfile_my_t *my = (vorbisfile_my_t *)lib;
}

int32_t my_ov_open_callbacks(x64emu_t* emu, void* datasource, void* vf, void* initial, int32_t ibytes, void* read, void* seek, void* close, void* tell);

#define CUSTOM_INIT \
    box64->vorbisfile = lib;\
    lib->priv.w.p2 = getVorbisfileMy(lib);

#define CUSTOM_FINI \
    freeVorbisfileMy(lib->priv.w.p2);   \
    free(lib->priv.w.p2);               \
    lib->context->vorbisfile = NULL;

#include "wrappedlib_init.h"

#define SUPER() \
GO(0)           \
GO(1)           \
GO(2)           \
GO(3)           \
GO(4)           \
GO(5)           \
GO(6)           \
GO(7)

// read
#define GO(A)   \
static uintptr_t my_read_fct_##A = 0;   \
static unsigned long my_read_##A(void* ptr, unsigned long size, unsigned long nmemb, void* datasource)     \
{                                       \
    return RunFunction(my_context, my_read_fct_##A, 4, ptr, size, nmemb, datasource);\
}
SUPER()
#undef GO
static void* findreadFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_read_fct_##A == (uintptr_t)fct) return my_read_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_read_fct_##A == 0) {my_read_fct_##A = (uintptr_t)fct; return my_read_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for vorbisfile read callback\n");
    return NULL;
}
// seek
#define GO(A)   \
static uintptr_t my_seek_fct_##A = 0;   \
static int my_seek_##A(void* ptr, int64_t offset, int whence)     \
{                                       \
    return (int)RunFunction(my_context, my_seek_fct_##A, 3, ptr, offset, whence);\
}
SUPER()
#undef GO
static void* findseekFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_seek_fct_##A == (uintptr_t)fct) return my_seek_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_seek_fct_##A == 0) {my_seek_fct_##A = (uintptr_t)fct; return my_seek_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for vorbisfile seek callback\n");
    return NULL;
}
// close
#define GO(A)   \
static uintptr_t my_close_fct_##A = 0;   \
static int my_close_##A(void* ptr)     \
{                   \
    return (int)RunFunction(my_context, my_close_fct_##A, 1, ptr);\
}
SUPER()
#undef GO
static void* findcloseFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_close_fct_##A == (uintptr_t)fct) return my_close_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_close_fct_##A == 0) {my_close_fct_##A = (uintptr_t)fct; return my_close_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for vorbisfile close callback\n");
    return NULL;
}
// tell
#define GO(A)   \
static uintptr_t my_tell_fct_##A = 0;   \
static long my_tell_##A(void* ptr)     \
{                                       \
    return (long)RunFunction(my_context, my_tell_fct_##A, 1, ptr);\
}
SUPER()
#undef GO
static void* findtellFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_tell_fct_##A == (uintptr_t)fct) return my_tell_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_tell_fct_##A == 0) {my_tell_fct_##A = (uintptr_t)fct; return my_tell_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for vorbisfile tell callback\n");
    return NULL;
}

#undef SUPER

EXPORT int32_t my_ov_open_callbacks(x64emu_t* emu, void* datasource, void* vf, void* initial, int32_t ibytes, void* read_fnc, void* seek_fnc, void* close_fnc, void* tell_fnc)
{
    vorbisfile_my_t* my = (vorbisfile_my_t*)emu->context->vorbisfile->priv.w.p2;
    ov_callbacks cbs = {0};
    cbs.read_func = findreadFct(read_fnc);
    cbs.seek_func = findseekFct(seek_fnc);
    cbs.close_func = findcloseFct(close_fnc);
    cbs.tell_func = findtellFct(tell_fnc);
    int32_t ret =  my->ov_open_callbacks(datasource, vf, initial, ibytes, cbs);
    return ret;
}
