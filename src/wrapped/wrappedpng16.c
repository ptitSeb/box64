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

const char* png16Name =
#ifdef ANDROID
	"libpng16.so"
#else
	"libpng16.so.16"
#endif
	;
#define LIBNAME png16

#include "generated/wrappedpng16types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// user_write
#define GO(A)   \
static uintptr_t my_user_write_fct_##A = 0;   \
static void my_user_write_##A(void* png_ptr, void* data, size_t length)    \
{                                       \
    RunFunctionFmt(my_user_write_fct_##A, "ppL", png_ptr, data, length);\
}
SUPER()
#undef GO
static void* finduser_writeFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_user_write_fct_##A == (uintptr_t)fct) return my_user_write_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_user_write_fct_##A == 0) {my_user_write_fct_##A = (uintptr_t)fct; return my_user_write_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng16 user_write callback\n");
    return NULL;
}
// user_flush
#define GO(A)   \
static uintptr_t my_user_flush_fct_##A = 0;   \
static void my_user_flush_##A(void* png_ptr)    \
{                                       \
    RunFunctionFmt(my_user_flush_fct_##A, "p", png_ptr);\
}
SUPER()
#undef GO
static void* finduser_flushFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_user_flush_fct_##A == (uintptr_t)fct) return my_user_flush_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_user_flush_fct_##A == 0) {my_user_flush_fct_##A = (uintptr_t)fct; return my_user_flush_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng16 user_flush callback\n");
    return NULL;
}
// user_read
#define GO(A)   \
static uintptr_t my_user_read_fct_##A = 0;   \
static void my_user_read_##A(void* png_ptr, void* data, size_t length)    \
{                                       \
    RunFunctionFmt(my_user_read_fct_##A, "ppL", png_ptr, data, length);\
}
SUPER()
#undef GO
static void* finduser_readFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_user_read_fct_##A == (uintptr_t)fct) return my_user_read_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_user_read_fct_##A == 0) {my_user_read_fct_##A = (uintptr_t)fct; return my_user_read_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng16 user_read callback\n");
    return NULL;
}
// error
#define GO(A)   \
static uintptr_t my_error_fct_##A = 0;   \
static void my_error_##A(void* a, void* b)    \
{                                       \
    RunFunctionFmt(my_error_fct_##A, "pp", a, b);\
}
SUPER()
#undef GO
static void* finderrorFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_error_fct_##A == (uintptr_t)fct) return my_error_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_error_fct_##A == 0) {my_error_fct_##A = (uintptr_t)fct; return my_error_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng16 error callback\n");
    return NULL;
}
// warning
#define GO(A)   \
static uintptr_t my_warning_fct_##A = 0;   \
static void my_warning_##A(void* a, void* b)    \
{                                       \
    RunFunctionFmt(my_warning_fct_##A, "pp", a, b);\
}
SUPER()
#undef GO
static void* findwarningFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_warning_fct_##A == (uintptr_t)fct) return my_warning_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_warning_fct_##A == 0) {my_warning_fct_##A = (uintptr_t)fct; return my_warning_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng16 warning callback\n");
    return NULL;
}
// malloc
#define GO(A)   \
static uintptr_t my_malloc_fct_##A = 0;   \
static void my_malloc_##A(void* a, unsigned long b)    \
{                                       \
    RunFunctionFmt(my_malloc_fct_##A, "pL", a, b);\
}
SUPER()
#undef GO
static void* findmallocFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_malloc_fct_##A == (uintptr_t)fct) return my_malloc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_malloc_fct_##A == 0) {my_malloc_fct_##A = (uintptr_t)fct; return my_malloc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng16 malloc callback\n");
    return NULL;
}
// free
#define GO(A)   \
static uintptr_t my_free_fct_##A = 0;   \
static void my_free_##A(void* a, void* b)    \
{                                       \
    RunFunctionFmt(my_free_fct_##A, "pp", a, b);\
}
SUPER()
#undef GO
static void* findfreeFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_free_fct_##A == (uintptr_t)fct) return my_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_fct_##A == 0) {my_free_fct_##A = (uintptr_t)fct; return my_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng16 free callback\n");
    return NULL;
}

// progressive_info
#define GO(A)   \
static uintptr_t my_progressive_info_fct_##A = 0;   \
static void my_progressive_info_##A(void* a, void* b)    \
{                                       \
    RunFunctionFmt(my_progressive_info_fct_##A, "pp", a, b);\
}
SUPER()
#undef GO
static void* findprogressive_infoFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_progressive_info_fct_##A == (uintptr_t)fct) return my_progressive_info_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_progressive_info_fct_##A == 0) {my_progressive_info_fct_##A = (uintptr_t)fct; return my_progressive_info_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng16 progressive_info callback\n");
    return NULL;
}

// progressive_end
#define GO(A)   \
static uintptr_t my_progressive_end_fct_##A = 0;   \
static void my_progressive_end_##A(void* a, void* b)    \
{                                       \
    RunFunctionFmt(my_progressive_end_fct_##A, "pp", a, b);\
}
SUPER()
#undef GO
static void* findprogressive_endFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_progressive_end_fct_##A == (uintptr_t)fct) return my_progressive_end_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_progressive_end_fct_##A == 0) {my_progressive_end_fct_##A = (uintptr_t)fct; return my_progressive_end_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng16 progressive_end callback\n");
    return NULL;
}

// progressive_row
#define GO(A)   \
static uintptr_t my_progressive_row_fct_##A = 0;   \
static void my_progressive_row_##A(void* a, void* b, uint32_t c, int d)    \
{                                       \
    RunFunctionFmt(my_progressive_row_fct_##A, "ppui", a, b, c, d);\
}
SUPER()
#undef GO
static void* findprogressive_rowFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_progressive_row_fct_##A == (uintptr_t)fct) return my_progressive_row_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_progressive_row_fct_##A == 0) {my_progressive_row_fct_##A = (uintptr_t)fct; return my_progressive_row_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng16 progressive_row callback\n");
    return NULL;
}


// user_transform
#define GO(A)   \
static uintptr_t my_user_transform_fct_##A = 0;   \
static void my_user_transform_##A(void* ptr, void* row, void* data)    \
{                                       \
    RunFunctionFmt(my_user_transform_fct_##A, "ppp", ptr, row, data);\
}
SUPER()
#undef GO
static void* finduser_transformFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_user_transform_fct_##A == (uintptr_t)fct) return my_user_transform_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_user_transform_fct_##A == 0) {my_user_transform_fct_##A = (uintptr_t)fct; return my_user_transform_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng16 user_transform callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my16_png_set_read_fn(x64emu_t *emu, void* png_ptr, void* io_ptr, void* read_data_fn)
{
    my->png_set_read_fn(png_ptr, io_ptr, finduser_readFct(read_data_fn));
}

EXPORT void my16_png_set_read_user_transform_fn(x64emu_t *emu, void* png_ptr, void* read_transform_fn)
{
    my->png_set_read_user_transform_fn(png_ptr, finduser_transformFct(read_transform_fn));
}

EXPORT void my16_png_set_error_fn(x64emu_t* emu, void* pngptr, void* errorptr, void* error_fn, void* warning_fn)
{
    my->png_set_error_fn(pngptr, errorptr, finderrorFct(error_fn), findwarningFct(warning_fn));
}

EXPORT void my16_png_set_write_fn(x64emu_t* emu, void* png_ptr, void* io_ptr, void* write_fn, void* flush_fn)
{
    my->png_set_write_fn(png_ptr, io_ptr, finduser_writeFct(write_fn), finduser_flushFct(flush_fn));
}

EXPORT void* my16_png_create_read_struct_2(x64emu_t* emu, void* user_png_ver, void* error_ptr, void* error_fn, void* warn_fn, void* mem_ptr, void* malloc_fn, void* free_fn)
{
    return my->png_create_read_struct_2(user_png_ver, error_ptr, finderrorFct(error_fn), findwarningFct(warn_fn), mem_ptr, findmallocFct(malloc_fn), findfreeFct(free_fn));
}

EXPORT void* my16_png_create_write_struct_2(x64emu_t* emu, void* user_png_ver, void* error_ptr, void* error_fn, void* warn_fn, void* mem_ptr, void* malloc_fn, void* free_fn)
{
    return my->png_create_write_struct_2(user_png_ver, error_ptr, finderrorFct(error_fn), findwarningFct(warn_fn), mem_ptr, findmallocFct(malloc_fn), findfreeFct(free_fn));
}

EXPORT void my16_png_set_progressive_read_fn(x64emu_t* emu, void* png_ptr, void* user_ptr, void* info, void* row, void* end)
{
    my->png_set_progressive_read_fn(png_ptr, user_ptr, findprogressive_infoFct(info), findprogressive_rowFct(row), findprogressive_endFct(end));
}

EXPORT void* my16_png_create_read_struct(x64emu_t* emu, void* png_ptr, void* user_ptr, void* errorfn, void* warnfn)
{
    return my->png_create_read_struct(png_ptr, user_ptr, finderrorFct(errorfn), findwarningFct(warnfn));
}

#define ALTMY my16_

#define CUSTOM_INIT \
    SETALTPREFIX("yes");

#include "wrappedlib_init.h"
