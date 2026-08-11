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
#include "callback.h"
#include "librarian.h"
#include "box64context.h"
#include "emu/x64emu_private.h"
#include "myalign.h"

const char* libgpgmeName = "libgpgme.so.11";
#define LIBNAME libgpgme

#define ADDED_FUNCTIONS()       \

#include "generated/wrappedlibgpgmetypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

typedef struct my_gpgme_data_cbs_s
{
  ssize_t (*read)(void*, void*, size_t);
  ssize_t (*write)(void*, void*, size_t);
  off_t (*seek) (void*, off_t, int);
  void (*release) (void*);
} my_gpgme_data_cbs_t;


// read
#define GO(A)   \
static uintptr_t my_read_fct_##A = 0;                                   \
static ssize_t my_read_##A(void* a, void* b, size_t c)                  \
{                                                                       \
    return (ssize_t)RunFunctionFmt(my_read_fct_##A, "ppL", a, b, c);    \
}
SUPER()
#undef GO
static void* find_read_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_read_fct_##A == (uintptr_t)fct) return my_read_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_read_fct_##A == 0) {my_read_fct_##A = (uintptr_t)fct; return my_read_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libgpgme read callback\n");
    return NULL;
}
// write
#define GO(A)   \
static uintptr_t my_write_fct_##A = 0;                                  \
static ssize_t my_write_##A(void* a, void* b, size_t c)                 \
{                                                                       \
    return (ssize_t)RunFunctionFmt(my_write_fct_##A, "ppL", a, b, c);   \
}
SUPER()
#undef GO
static void* find_write_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_write_fct_##A == (uintptr_t)fct) return my_write_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_write_fct_##A == 0) {my_write_fct_##A = (uintptr_t)fct; return my_write_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libgpgme write callback\n");
    return NULL;
}
// seek
#define GO(A)   \
static uintptr_t my_seek_fct_##A = 0;                               \
static off_t my_seek_##A(void* a, off_t b, int c)                   \
{                                                                   \
    return (off_t)RunFunctionFmt(my_seek_fct_##A, "pLi", a, b, c);  \
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
    printf_log(LOG_NONE, "Warning, no more slot for libgpgme seek callback\n");
    return NULL;
}
// release
#define GO(A)   \
static uintptr_t my_release_fct_##A = 0;        \
static void my_release_##A(void* a)             \
{                                               \
    RunFunctionFmt(my_release_fct_##A, "p", a); \
}
SUPER()
#undef GO
static void* find_release_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_release_fct_##A == (uintptr_t)fct) return my_release_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_release_fct_##A == 0) {my_release_fct_##A = (uintptr_t)fct; return my_release_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libgpgme release callback\n");
    return NULL;
}

#undef SUPER

// gpgme_data_cbs_t
#define SUPER() \
GO(0)           \
GO(1)           \
GO(2)           \
GO(3)           \
GO(4)

#define GO(A)                                                   \
static my_gpgme_data_cbs_t* my_gpgme_data_cbs_ref_##A = NULL;   \
static my_gpgme_data_cbs_t my_gpgme_data_cbs_struct_##A = {0};
SUPER()
#undef GO

static void wrap_gpgme_data_cbs(my_gpgme_data_cbs_t* dst, my_gpgme_data_cbs_t* src)
{
    #define GO(A) dst->A = find_##A##_Fct(src->A)
    GO(read);
    GO(write);
    GO(seek);
    GO(release);
    #undef GO
}

static my_gpgme_data_cbs_t* find_gpgme_data_cbs_Struct(my_gpgme_data_cbs_t* cbs)
{
    if(!cbs) return NULL;
    #define GO(A) if(my_gpgme_data_cbs_ref_##A == cbs) {       \
        return &my_gpgme_data_cbs_struct_##A;                  \
    }
    SUPER()
    #undef GO
    #define GO(A) if(!my_gpgme_data_cbs_ref_##A) {             \
        wrap_gpgme_data_cbs(&my_gpgme_data_cbs_struct_##A, cbs); \
        my_gpgme_data_cbs_ref_##A = cbs;                       \
        return &my_gpgme_data_cbs_struct_##A;                  \
    }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gpgme_data_cbs\n");
    return NULL;
}

#undef SUPER


EXPORT uint32_t my_gpgme_data_new_from_cbs(x64emu_t* emu, void* data, my_gpgme_data_cbs_t* cbs, void* stream)
{
    return my->gpgme_data_new_from_cbs(data, find_gpgme_data_cbs_Struct(cbs), stream);
}

#include "wrappedlib_init.h"
