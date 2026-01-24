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
#include "myalign.h"

const char* libavformat58Name = "libavformat.so.58";

#define LIBNAME libavformat58

#define ADDED_FUNCTIONS()                   \

#include "generated/wrappedlibavformat58types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// read_packet ...
#define GO(A)   \
static uintptr_t my_read_packet_fct_##A = 0;                                \
static int my_read_packet_##A(void* a, void* b, int c)                      \
{                                                                           \
    return (int)RunFunctionFmt(my_read_packet_fct_##A, "ppi", a, b, c);     \
}
SUPER()
#undef GO
static void* find_read_packet_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_read_packet_fct_##A == (uintptr_t)fct) return my_read_packet_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_read_packet_fct_##A == 0) {my_read_packet_fct_##A = (uintptr_t)fct; return my_read_packet_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavformat58 read_packet callback\n");
    return NULL;
}
// write_packet ...
#define GO(A)   \
static uintptr_t my_write_packet_fct_##A = 0;                               \
static int my_write_packet_##A(void* a, void* b, int c)                     \
{                                                                           \
    return (int)RunFunctionFmt(my_write_packet_fct_##A, "ppi", a, b, c);    \
}
SUPER()
#undef GO
static void* find_write_packet_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_write_packet_fct_##A == (uintptr_t)fct) return my_write_packet_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_write_packet_fct_##A == 0) {my_write_packet_fct_##A = (uintptr_t)fct; return my_write_packet_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavformat58 write_packet callback\n");
    return NULL;
}
// seek ...
#define GO(A)   \
static uintptr_t my_seek_fct_##A = 0;                                   \
static int64_t my_seek_##A(void* a, int64_t b, int c)                   \
{                                                                       \
    return (int64_t)RunFunctionFmt(my_seek_fct_##A, "pIi", a, b, c);    \
}
SUPER()
#undef GO
static void* find_seek_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_seek_fct_##A == (uintptr_t)fct) return my_seek_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_seek_fct_##A == 0) {my_seek_fct_##A = (uintptr_t)fct; return my_seek_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavformat58 seek callback\n");
    return NULL;
}

#undef SUPER


EXPORT void* my_avio_alloc_context(x64emu_t* emu, void* buffer, int buffer_size, int write_flag, void* opaque, void* read_packet, void* write_packet, void* seek)
{
    return my->avio_alloc_context(buffer, buffer_size, write_flag, opaque, find_read_packet_Fct(read_packet), find_write_packet_Fct(write_packet), find_seek_Fct(seek));
}

// will allow wrapped lib if and only if libavutil.so.56 && libavcodec.so.58 are also available!
#define PRE_INIT \
    if (BOX64ENV(nogtk)) return -2;                                     \
    {                                                                   \
        void* h = dlopen("libavutil.so.56", RTLD_LAZY | RTLD_GLOBAL);   \
        if(!h) return -2;                                               \
        else dlclose(h);                                                \
        h = dlopen("libavcodec.so.58", RTLD_LAZY | RTLD_GLOBAL);        \
        if(!h) return -2;                                               \
        else dlclose(h);                                                \
    }

#include "wrappedlib_init.h"
