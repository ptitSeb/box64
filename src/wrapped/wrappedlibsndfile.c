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
#include "librarian.h"
#include "box64context.h"
#include "emu/x64emu_private.h"

#ifdef ANDROID
    const char* libsndfileName = "libsndfile.so";
#else
    const char* libsndfileName = "libsndfile.so.1";
#endif

#define LIBNAME libsndfile

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedlibsndfiletypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// sf_vio_get_filelen ...
#define GO(A)   \
static uintptr_t my_sf_vio_get_filelen_fct_##A = 0;                                 \
static int64_t my_sf_vio_get_filelen_##A(void* a)                                   \
{                                                                                   \
    return (int64_t)RunFunctionFmt(my_sf_vio_get_filelen_fct_##A, "p", a);   \
}
SUPER()
#undef GO
static void* find_sf_vio_get_filelen_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_sf_vio_get_filelen_fct_##A == (uintptr_t)fct) return my_sf_vio_get_filelen_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_sf_vio_get_filelen_fct_##A == 0) {my_sf_vio_get_filelen_fct_##A = (uintptr_t)fct; return my_sf_vio_get_filelen_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for sndfile sf_vio_get_filelen callback\n");
    return NULL;
}
// sf_vio_seek ...
#define GO(A)   \
static uintptr_t my_sf_vio_seek_fct_##A = 0;                                                        \
static int64_t my_sf_vio_seek_##A(int64_t offset, int whence, void *user_data)                      \
{                                                                                                   \
    return (int64_t)RunFunctionFmt(my_sf_vio_seek_fct_##A, "Iip", offset, whence, user_data);  \
}
SUPER()
#undef GO
static void* find_sf_vio_seek_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_sf_vio_seek_fct_##A == (uintptr_t)fct) return my_sf_vio_seek_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_sf_vio_seek_fct_##A == 0) {my_sf_vio_seek_fct_##A = (uintptr_t)fct; return my_sf_vio_seek_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for sndfile sf_vio_seek callback\n");
    return NULL;
}
// sf_vio_read ...
#define GO(A)   \
static uintptr_t my_sf_vio_read_fct_##A = 0;                                                    \
static int64_t my_sf_vio_read_##A(void* ptr, int64_t count, void *user_data)                    \
{                                                                                               \
    return (int64_t)RunFunctionFmt(my_sf_vio_read_fct_##A, "pIp", ptr, count, user_data);  \
}
SUPER()
#undef GO
static void* find_sf_vio_read_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_sf_vio_read_fct_##A == (uintptr_t)fct) return my_sf_vio_read_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_sf_vio_read_fct_##A == 0) {my_sf_vio_read_fct_##A = (uintptr_t)fct; return my_sf_vio_read_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for sndfile sf_vio_read callback\n");
    return NULL;
}
// sf_vio_write ...
#define GO(A)   \
static uintptr_t my_sf_vio_write_fct_##A = 0;                                                   \
static int64_t my_sf_vio_write_##A(const void* ptr, int64_t count, void *user_data)             \
{                                                                                               \
    return (int64_t)RunFunctionFmt(my_sf_vio_write_fct_##A, "pIp", ptr, count, user_data); \
}
SUPER()
#undef GO
static void* find_sf_vio_write_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_sf_vio_write_fct_##A == (uintptr_t)fct) return my_sf_vio_write_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_sf_vio_write_fct_##A == 0) {my_sf_vio_write_fct_##A = (uintptr_t)fct; return my_sf_vio_write_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for sndfile sf_vio_write callback\n");
    return NULL;
}
// sf_vio_tell ...
#define GO(A)   \
static uintptr_t my_sf_vio_tell_fct_##A = 0;                                \
static int64_t my_sf_vio_tell_##A(void* a)                                  \
{                                                                           \
    return (int64_t)RunFunctionFmt(my_sf_vio_tell_fct_##A, "p", a);  \
}
SUPER()
#undef GO
static void* find_sf_vio_tell_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_sf_vio_tell_fct_##A == (uintptr_t)fct) return my_sf_vio_tell_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_sf_vio_tell_fct_##A == 0) {my_sf_vio_tell_fct_##A = (uintptr_t)fct; return my_sf_vio_tell_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for sndfile sf_vio_tell callback\n");
    return NULL;
}
#undef SUPER

typedef int64_t  (*sf_vio_get_filelen) (void *user_data) ;
typedef int64_t  (*sf_vio_seek)        (int64_t offset, int whence, void *user_data) ;
typedef int64_t  (*sf_vio_read)        (void *ptr, int64_t count, void *user_data) ;
typedef int64_t  (*sf_vio_write)       (const void *ptr, int64_t count, void *user_data) ;
typedef int64_t  (*sf_vio_tell)        (void *user_data) ;
typedef struct my_sfvirtual_io_s
{   sf_vio_get_filelen  get_filelen;
    sf_vio_seek         seek ;
    sf_vio_read         read ;
    sf_vio_write        write ;
    sf_vio_tell         tell ;
} my_sfvirtual_io_t;

EXPORT void* my_sf_open_virtual(x64emu_t* emu, my_sfvirtual_io_t* sfvirtual, int mode, void* sfinfo, void* data)
{
    my_sfvirtual_io_t native = {0};
    native.get_filelen = find_sf_vio_get_filelen_Fct(sfvirtual->get_filelen);
    native.seek = find_sf_vio_seek_Fct(sfvirtual->seek);
    native.read = find_sf_vio_read_Fct(sfvirtual->read);
    native.write = find_sf_vio_write_Fct(sfvirtual->write);
    native.tell = find_sf_vio_tell_Fct(sfvirtual->tell);

    return my->sf_open_virtual(&native, mode, sfinfo, data);
}

EXPORT int my_sf_close(x64emu_t* emu, void* sf)
{
    return my->sf_close(sf);
}

#include "wrappedlib_init.h"
