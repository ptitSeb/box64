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


const char* lberName =
#ifdef ANDROID
    "liblber-2.4.so"
#else
    "liblber-2.4.so.2"
#endif
    ;
#define ALTNAME "liblber-2.5.so.0"
#define LIBNAME lber

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedlbertypes.h"

#include "wrappercallback.h"

typedef struct my_sockbuf_io_s {
    int (*sbi_setup)(void* sbiod, void* arg);
    int (*sbi_remove)(void* sbiod);
    int (*sbi_ctrl)(void* sbiod, int opt, void* arg);
    size_t (*sbi_read)(void* sbiod, void* buf, size_t len);
    size_t (*sbi_write)(void* sbiod, void* buf, size_t len);
    int (*sbi_close)(void* sbiod);
} my_sockbuf_io_t;

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// sbi_setup ...
#define GO(A)   \
static uintptr_t my_sbi_setup_fct_##A = 0;                      \
static int my_sbi_setup_##A(void* a, void* b)                   \
{                                                               \
    return RunFunctionFmt(my_sbi_setup_fct_##A, "pp", a, b);    \
}
SUPER()
#undef GO
static void* find_sbi_setup_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_sbi_setup_fct_##A == (uintptr_t)fct) return my_sbi_setup_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_sbi_setup_fct_##A == 0) {my_sbi_setup_fct_##A = (uintptr_t)fct; return my_sbi_setup_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for liblber sbi_setup callback\n");
    return NULL;
}
// sbi_remove ...
#define GO(A)   \
static uintptr_t my_sbi_remove_fct_##A = 0;                     \
static int my_sbi_remove_##A(void* a)                           \
{                                                               \
    return RunFunctionFmt(my_sbi_remove_fct_##A, "p", a);       \
}
SUPER()
#undef GO
static void* find_sbi_remove_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_sbi_remove_fct_##A == (uintptr_t)fct) return my_sbi_remove_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_sbi_remove_fct_##A == 0) {my_sbi_remove_fct_##A = (uintptr_t)fct; return my_sbi_remove_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for liblber sbi_remove callback\n");
    return NULL;
}
// sbi_ctrl ...
#define GO(A)   \
static uintptr_t my_sbi_ctrl_fct_##A = 0;                       \
static int my_sbi_ctrl_##A(void* a, int b, void* c)             \
{                                                               \
    return RunFunctionFmt(my_sbi_ctrl_fct_##A, "pip", a, b, c); \
}
SUPER()
#undef GO
static void* find_sbi_ctrl_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_sbi_ctrl_fct_##A == (uintptr_t)fct) return my_sbi_ctrl_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_sbi_ctrl_fct_##A == 0) {my_sbi_ctrl_fct_##A = (uintptr_t)fct; return my_sbi_ctrl_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for liblber sbi_ctrl callback\n");
    return NULL;
}
// sbi_read ...
#define GO(A)   \
static uintptr_t my_sbi_read_fct_##A = 0;                       \
static size_t my_sbi_read_##A(void* a, void* b, size_t c)       \
{                                                               \
    return RunFunctionFmt(my_sbi_read_fct_##A, "ppL", a, b, c); \
}
SUPER()
#undef GO
static void* find_sbi_read_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_sbi_read_fct_##A == (uintptr_t)fct) return my_sbi_read_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_sbi_read_fct_##A == 0) {my_sbi_read_fct_##A = (uintptr_t)fct; return my_sbi_read_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for liblber sbi_read callback\n");
    return NULL;
}
// sbi_write ...
#define GO(A)   \
static uintptr_t my_sbi_write_fct_##A = 0;                          \
static size_t my_sbi_write_##A(void* a, void* b, size_t c)          \
{                                                                   \
    return RunFunctionFmt(my_sbi_write_fct_##A, "ppL", a, b, c);    \
}
SUPER()
#undef GO
static void* find_sbi_write_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_sbi_write_fct_##A == (uintptr_t)fct) return my_sbi_write_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_sbi_write_fct_##A == 0) {my_sbi_write_fct_##A = (uintptr_t)fct; return my_sbi_write_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for liblber sbi_write callback\n");
    return NULL;
}// sbi_close ...
#define GO(A)   \
static uintptr_t my_sbi_close_fct_##A = 0;                  \
static int my_sbi_close_##A(void* a)                        \
{                                                           \
    return RunFunctionFmt(my_sbi_close_fct_##A, "p", a);    \
}
SUPER()
#undef GO
static void* find_sbi_close_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_sbi_close_fct_##A == (uintptr_t)fct) return my_sbi_close_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_sbi_close_fct_##A == 0) {my_sbi_close_fct_##A = (uintptr_t)fct; return my_sbi_close_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for liblber sbi_close callback\n");
    return NULL;
}

#undef SUPER

EXPORT int my_ber_sockbuf_add_io(x64emu_t* emu, void* sb, my_sockbuf_io_t* io, int layer, void* arg)
{
    my_sockbuf_io_t my_io = {0};
    if(io) {
        #define GO(A) my_io.A = find_##A##_Fct(io->A)
        GO(sbi_setup);
        GO(sbi_remove);
        GO(sbi_ctrl);
        GO(sbi_read);
        GO(sbi_write);
        GO(sbi_close);
        #undef GO
    }
    return my->ber_sockbuf_add_io(sb, io?&my_io:NULL, layer, arg);
}

#include "wrappedlib_init.h"

