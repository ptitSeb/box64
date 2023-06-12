#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include "wrappedlibs.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include "debug.h"
#include "callback.h"

const char* libharfbuzzName = "libharfbuzz.so.0";
#define LIBNAME libharfbuzz

#include "generated/wrappedlibharfbuzztypes.h"

#include "wrappercallback.h"

struct hb_atomic_int_t
{
  int v;
};

struct hb_reference_count_t
{
  struct hb_atomic_int_t ref_count;
};

struct hb_atomic_ptr_t {
    void* v;
};

struct hb_object_header_t
{
  struct hb_reference_count_t ref_count;
  struct hb_atomic_int_t writable;
  struct hb_atomic_ptr_t user_data;
};

/*
 * hb_draw_funcs_t
 */

#define HB_DRAW_FUNCS_IMPLEMENT_CALLBACKS \
  HB_DRAW_FUNC_IMPLEMENT (move_to) \
  HB_DRAW_FUNC_IMPLEMENT (line_to) \
  HB_DRAW_FUNC_IMPLEMENT (quadratic_to) \
  HB_DRAW_FUNC_IMPLEMENT (cubic_to) \
  HB_DRAW_FUNC_IMPLEMENT (close_path) \
  /* ^--- Add new callbacks here */

struct hb_draw_funcs_t__func {
#define HB_DRAW_FUNC_IMPLEMENT(name) void* name;
    HB_DRAW_FUNCS_IMPLEMENT_CALLBACKS
#undef HB_DRAW_FUNC_IMPLEMENT
};

struct hb_draw_funcs_t__destroy {
#define HB_DRAW_FUNC_IMPLEMENT(name) void* name;
    HB_DRAW_FUNCS_IMPLEMENT_CALLBACKS
#undef HB_DRAW_FUNC_IMPLEMENT
};

struct hb_draw_funcs_t
{
    struct hb_object_header_t header;

    struct hb_draw_funcs_t__func func;

  struct {
#define HB_DRAW_FUNC_IMPLEMENT(name) void *name;
    HB_DRAW_FUNCS_IMPLEMENT_CALLBACKS
#undef HB_DRAW_FUNC_IMPLEMENT
  } *user_data;

  struct hb_draw_funcs_t__destroy* destroy;
};

#define SUPER() \
    GO(0)   \
    GO(1)   \
    GO(2)   \
    GO(3)

// buffer_message
#define GO(A)                                                       \
static uintptr_t my_buffer_message_fct_##A = 0;                     \
static int my_buffer_message_##A(void* a, void* b, void* c, void* d)\
{                                                                   \
    return (int)RunFunctionFmt(my_context, my_buffer_message_fct_##A, "pppp", a, b, c, d);  \
}
SUPER()
#undef GO
static void* find_buffer_message_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_buffer_message_fct_##A == (uintptr_t)fct) return my_buffer_message_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_buffer_message_##A == 0) {my_buffer_message_fct_##A = (uintptr_t)fct; return my_buffer_message_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz buffer message callback\n");
    return NULL;
}

// destroy
#define GO(A)                                               \
static uintptr_t my_destroy_fct_##A = 0;                    \
static void my_destroy_##A(void* a)                         \
{                                                           \
    RunFunctionFmt(my_context, my_destroy_fct_##A, "p", a); \
}
SUPER()
#undef GO
static void* find_destroy_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_destroy_fct_##A == (uintptr_t)fct) return my_destroy_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_destroy_##A == 0) {my_destroy_fct_##A = (uintptr_t)fct; return my_destroy_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz destroy callback\n");
    return NULL;
}

#undef SUPER

#define FUNC(A) \
EXPORT void* my_##A(x64emu_t* emu, void* data, uint32_t length, uint32_t mode, void* user_data, void* destroy) \
{                                                                           \
    (void)emu;                                                              \
    return my->A(data, length, mode, user_data, find_destroy_Fct(destroy)); \
}

FUNC(hb_blob_create)
FUNC(hb_blob_create_or_fail)

#undef FUNC

#define FUNC(A) \
EXPORT int my_##A(x64emu_t* emu, void* blob, void* key, void* data, void* destroy, int replace) \
{                                                                       \
    (void)emu;                                                          \
    return my->A(blob, key, data, find_destroy_Fct(destroy), replace);  \
}

FUNC(hb_blob_set_user_data)
FUNC(hb_buffer_set_user_data)

#undef FUNC

#define FUNC(A) \
EXPORT void my_##A(x64emu_t* emu, void* buffer, void* func, void* user_data, void* destroy) \
{               \
    (void)emu;  \
    return my->A(buffer, find_buffer_message_Fct(func), user_data, find_destroy_Fct(destroy));  \
}

FUNC(hb_buffer_set_message_func)

#undef FUNC

EXPORT void my_hb_draw_funcs_destroy(x64emu_t* emu, void* funcs)
{
    (void)emu;
    struct hb_draw_funcs_t__destroy destroy = {0};
    struct hb_draw_funcs_t* funcs_ = funcs;

#define HB_DRAW_FUNC_IMPLEMENT(name) \
    if (funcs_->destroy->name) destroy.name = find_destroy_Fct(funcs_->destroy->name);
    HB_DRAW_FUNCS_IMPLEMENT_CALLBACKS
#undef HB_DRAW_FUNC_IMPLEMENT

    struct hb_draw_funcs_t__destroy* original = funcs_->destroy;
    funcs_->destroy = &destroy;
    my->hb_draw_funcs_destroy(funcs);
    funcs_->destroy = original;
}

#include "wrappedlib_init.h"
