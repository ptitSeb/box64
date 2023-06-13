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

/*
 * hb_font_funcs_t
 */

#define HB_FONT_FUNCS_IMPLEMENT_CALLBACKS \
  HB_FONT_FUNC_IMPLEMENT (get_,font_h_extents) \
  HB_FONT_FUNC_IMPLEMENT (get_,font_v_extents) \
  HB_FONT_FUNC_IMPLEMENT (get_,nominal_glyph) \
  HB_FONT_FUNC_IMPLEMENT (get_,nominal_glyphs) \
  HB_FONT_FUNC_IMPLEMENT (get_,variation_glyph) \
  HB_FONT_FUNC_IMPLEMENT (get_,glyph_h_advance) \
  HB_FONT_FUNC_IMPLEMENT (get_,glyph_v_advance) \
  HB_FONT_FUNC_IMPLEMENT (get_,glyph_h_advances) \
  HB_FONT_FUNC_IMPLEMENT (get_,glyph_v_advances) \
  HB_FONT_FUNC_IMPLEMENT (get_,glyph_h_origin) \
  HB_FONT_FUNC_IMPLEMENT (get_,glyph_v_origin) \
  HB_FONT_FUNC_IMPLEMENT (get_,glyph_h_kerning) \
  HB_FONT_FUNC_IMPLEMENT (get_,glyph_v_kerning) \
  HB_FONT_FUNC_IMPLEMENT (get_,glyph_extents) \
  HB_FONT_FUNC_IMPLEMENT (get_,glyph_contour_point) \
  HB_FONT_FUNC_IMPLEMENT (get_,glyph_name) \
  HB_FONT_FUNC_IMPLEMENT (get_,glyph_from_name) \
  HB_FONT_FUNC_IMPLEMENT (,draw_glyph) \
  HB_FONT_FUNC_IMPLEMENT (,paint_glyph) \
  /* ^--- Add new callbacks here */

struct hb_font_funcs_t__destroy {
#define HB_FONT_FUNC_IMPLEMENT(get_,name) void* name;
    HB_FONT_FUNCS_IMPLEMENT_CALLBACKS
#undef HB_FONT_FUNC_IMPLEMENT
};

struct hb_font_funcs_t
{
    struct hb_object_header_t header;

    struct {
#define HB_FONT_FUNC_IMPLEMENT(get_,name) void* name;
        HB_FONT_FUNCS_IMPLEMENT_CALLBACKS
#undef HB_FONT_FUNC_IMPLEMENT
    } *user_data;

    struct hb_font_funcs_t__destroy* destroy;

    union get_t {
        struct get_funcs_t {
#define HB_FONT_FUNC_IMPLEMENT(get_,name) void* name;
            HB_FONT_FUNCS_IMPLEMENT_CALLBACKS
#undef HB_FONT_FUNC_IMPLEMENT
        } f;
        void (*array[0
#define HB_FONT_FUNC_IMPLEMENT(get_,name) +1
            HB_FONT_FUNCS_IMPLEMENT_CALLBACKS
#undef HB_FONT_FUNC_IMPLEMENT
        ]) ();
    } get;
};

#define SUPER() \
    GO(0)   \
    GO(1)   \
    GO(2)   \
    GO(3)   \
    GO(4)   \
    GO(5)   \
    GO(6)   \
    GO(7)   \
    GO(8)   \
    GO(9)   \
    GO(10)  \
    GO(11)  \
    GO(12)  \

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
    #define GO(A) if (my_buffer_message_fct_##A == 0) {my_buffer_message_fct_##A = (uintptr_t)fct; return my_buffer_message_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz buffer message callback\n");
    return NULL;
}

// draw close path
#define GO(A)                                                           \
static uintptr_t my_draw_close_path_fct_##A = 0;                        \
static void my_draw_close_path_##A(void* a, void* b, void* c, void* d)  \
{                                                                       \
    RunFunctionFmt(my_context, my_draw_close_path_fct_##A, "pppp", a, b, c, d);  \
}
SUPER()
#undef GO
static void* find_draw_close_path_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_draw_close_path_fct_##A == (uintptr_t)fct) return my_draw_close_path_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_draw_close_path_fct_##A == 0) {my_draw_close_path_fct_##A = (uintptr_t)fct; return my_draw_close_path_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz draw close path callback\n");
    return NULL;
}

// draw cubic to
#define GO(A)                                   \
static uintptr_t my_draw_cubic_to_fct_##A = 0;  \
static void my_draw_cubic_to_##A(void* a, void* b, void* c, float d1, float d2, float d3, float d4, float d5, float d6, void* e) \
{                                               \
    RunFunctionFmt(my_context, my_draw_cubic_to_fct_##A, "pppffffffp", a, b, c, d1, d2, d3, d4, d5, d6, e); \
}
SUPER()
#undef GO
static void* find_draw_cubic_to_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_draw_cubic_to_fct_##A == (uintptr_t)fct) return my_draw_cubic_to_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_draw_cubic_to_fct_##A == 0) {my_draw_cubic_to_fct_##A = (uintptr_t)fct; return my_draw_cubic_to_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz draw cubic to callback\n");
    return NULL;
}

// draw line/move to
#define GO(A)                                           \
static uintptr_t my_draw_line_or_move_to_fct_##A = 0;   \
static void my_draw_line_or_move_to_##A(void* a, void* b, void* c, float d1, float d2, void* e) \
{                                                       \
    RunFunctionFmt(my_context, my_draw_line_or_move_to_fct_##A, "pppffp", a, b, c, d1, d2, e);  \
}
SUPER()
#undef GO
static void* find_draw_line_or_move_to_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_draw_line_or_move_to_fct_##A == (uintptr_t)fct) return my_draw_line_or_move_to_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_draw_line_or_move_to_fct_##A == 0) {my_draw_line_or_move_to_fct_##A = (uintptr_t)fct; return my_draw_line_or_move_to_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz draw line/move to callback\n");
    return NULL;
}

// draw quadratic to
#define GO(A)                                       \
static uintptr_t my_draw_quadratic_to_fct_##A = 0;  \
static void my_draw_quadratic_to_##A(void* a, void* b, void* c, float d1, float d2, float d3, float d4, void* e) \
{                                                   \
    RunFunctionFmt(my_context, my_draw_quadratic_to_fct_##A, "pppffffp", a, b, c, d1, d2, d3, d4, e); \
}
SUPER()
#undef GO
static void* find_draw_quadratic_to_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_draw_quadratic_to_fct_##A == (uintptr_t)fct) return my_draw_quadratic_to_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_draw_quadratic_to_fct_##A == 0) {my_draw_quadratic_to_fct_##A = (uintptr_t)fct; return my_draw_quadratic_to_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz draw quadratic to callback\n");
    return NULL;
}

// reference table
#define GO(A)                                                       \
static uintptr_t my_reference_table_fct_##A = 0;                    \
static void* my_reference_table_##A(void* a, uint32_t b, void* c)   \
{                                                                   \
    return (void*)RunFunctionFmt(my_context, my_reference_table_fct_##A, "pup", a, b, c);   \
}
SUPER()
#undef GO
static void* find_reference_table_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_reference_table_fct_##A == (uintptr_t)fct) return my_reference_table_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_reference_table_fct_##A == 0) {my_reference_table_fct_##A = (uintptr_t)fct; return my_reference_table_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz reference table callback\n");
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
    #define GO(A) if (my_destroy_fct_##A == 0) {my_destroy_fct_##A = (uintptr_t)fct; return my_destroy_##A;}
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
    return (int)my->A(blob, key, data, find_destroy_Fct(destroy), replace);  \
}

FUNC(hb_blob_set_user_data)
FUNC(hb_buffer_set_user_data)

#undef FUNC

#define FUNC(A) \
EXPORT void my_##A(x64emu_t* emu, void* buffer, void* func, void* user_data, void* destroy) \
{               \
    (void)emu;  \
    my->A(buffer, find_buffer_message_Fct(func), user_data, find_destroy_Fct(destroy));  \
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

EXPORT void my_hb_draw_funcs_set_close_path_func(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)
{
    (void)emu;
    my->hb_draw_funcs_set_close_path_func(funcs, find_draw_close_path_Fct(func), user_data, find_destroy_Fct(destroy));
}

EXPORT void my_hb_draw_funcs_set_cubic_to_func(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)
{
    (void)emu;
    my->hb_draw_funcs_set_cubic_to_func(funcs, find_draw_cubic_to_Fct(func), user_data, find_destroy_Fct(destroy));
}

#define FUNC(A) \
EXPORT void my_##A(x64emu_t* emu, void* buffer, void* func, void* user_data, void* destroy) \
{               \
    (void)emu;  \
    my->A(buffer, find_draw_line_or_move_to_Fct(func), user_data, find_destroy_Fct(destroy));  \
}

FUNC(hb_draw_funcs_set_line_to_func)
FUNC(hb_draw_funcs_set_move_to_func)

#undef FUNC

EXPORT void my_hb_draw_funcs_set_quadratic_to_func(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)
{
    (void)emu;
    my->hb_draw_funcs_set_quadratic_to_func(funcs, find_draw_quadratic_to_Fct(func), user_data, find_destroy_Fct(destroy));
}

EXPORT void* my_hb_face_create_for_tables(x64emu_t* emu, void* func, void* user_data, void* destroy)
{
    (void)emu;
    return my->hb_face_create_for_tables(find_reference_table_Fct(func), user_data, find_destroy_Fct(destroy));
}

EXPORT int my_hb_face_set_user_data(x64emu_t* emu, void* face, void* key, void* data, void* destroy, int replace)
{
    (void)emu;
    return (int)my->hb_face_set_user_data(face, key, data, find_destroy_Fct(destroy), replace);
}

EXPORT void my_hb_font_funcs_destroy(x64emu_t* emu, void* funcs)
{
    (void)emu;
    struct hb_font_funcs_t__destroy destroy = {0};
    struct hb_font_funcs_t* funcs_ = funcs;

#define HB_FONT_FUNC_IMPLEMENT(get_,name) \
    if (funcs_->destroy->name) destroy.name = find_destroy_Fct(funcs_->destroy->name);
    HB_FONT_FUNCS_IMPLEMENT_CALLBACKS
#undef HB_FONT_FUNC_IMPLEMENT

    struct hb_font_funcs_t__destroy* original = funcs_->destroy;
    funcs_->destroy = &destroy;
    my->hb_font_funcs_destroy(funcs);
    funcs_->destroy = original;
}

#include "wrappedlib_init.h"
