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

/*
 * hb_unicode_funcs_t
 */

#define HB_UNICODE_FUNCS_IMPLEMENT_CALLBACKS \
  HB_UNICODE_FUNC_IMPLEMENT (combining_class) \
  HB_UNICODE_FUNC_IMPLEMENT (eastasian_width) \
  HB_UNICODE_FUNC_IMPLEMENT (general_category) \
  HB_UNICODE_FUNC_IMPLEMENT (mirroring) \
  HB_UNICODE_FUNC_IMPLEMENT (script) \
  HB_UNICODE_FUNC_IMPLEMENT (compose) \
  HB_UNICODE_FUNC_IMPLEMENT (decompose) \
  HB_UNICODE_FUNC_IMPLEMENT (decompose_compatibility) \
  /* ^--- Add new callbacks here */

/* Simple callbacks are those taking a hb_codepoint_t and returning a hb_codepoint_t */
#define HB_UNICODE_FUNCS_IMPLEMENT_CALLBACKS_SIMPLE \
  HB_UNICODE_FUNC_IMPLEMENT (hb_unicode_combining_class_t, combining_class) \
  HB_UNICODE_FUNC_IMPLEMENT (unsigned int, eastasian_width) \
  HB_UNICODE_FUNC_IMPLEMENT (hb_unicode_general_category_t, general_category) \
  HB_UNICODE_FUNC_IMPLEMENT (hb_codepoint_t, mirroring) \
  HB_UNICODE_FUNC_IMPLEMENT (hb_script_t, script) \
  /* ^--- Add new simple callbacks here */


struct hb_unicode_funcs_t__destroy {
#define HB_UNICODE_FUNC_IMPLEMENT(name) void* name;
    HB_UNICODE_FUNCS_IMPLEMENT_CALLBACKS
#undef HB_UNICODE_FUNC_IMPLEMENT
};

struct hb_unicode_funcs_t
{
    struct hb_object_header_t header;
    struct hb_unicode_funcs_t *parent;

    struct {
#define HB_UNICODE_FUNC_IMPLEMENT(name) void* name;
        HB_UNICODE_FUNCS_IMPLEMENT_CALLBACKS
#undef HB_UNICODE_FUNC_IMPLEMENT
    } func;

    struct {
#define HB_UNICODE_FUNC_IMPLEMENT(name) void* name;
        HB_UNICODE_FUNCS_IMPLEMENT_CALLBACKS
#undef HB_UNICODE_FUNC_IMPLEMENT
    } user_data;

  struct hb_unicode_funcs_t__destroy destroy;
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
    return (int)RunFunctionFmt(my_buffer_message_fct_##A, "pppp", a, b, c, d);  \
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
    RunFunctionFmt(my_draw_close_path_fct_##A, "pppp", a, b, c, d);  \
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
    RunFunctionFmt(my_draw_cubic_to_fct_##A, "pppffffffp", a, b, c, d1, d2, d3, d4, d5, d6, e); \
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
    RunFunctionFmt(my_draw_line_or_move_to_fct_##A, "pppffp", a, b, c, d1, d2, e);  \
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
    RunFunctionFmt(my_draw_quadratic_to_fct_##A, "pppffffp", a, b, c, d1, d2, d3, d4, e); \
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
    return (void*)RunFunctionFmt(my_reference_table_fct_##A, "pup", a, b, c);   \
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

// unicode combining class
#define GO(A)                                                               \
static uintptr_t my_unicode_combining_class_fct_##A = 0;                    \
static uint32_t my_unicode_combining_class_##A(void* a, uint32_t b, void* c)\
{                                                                           \
    return (uint32_t)RunFunctionFmt(my_unicode_combining_class_fct_##A, "pup", a, b, c);  \
}
SUPER()
#undef GO
static void* find_unicode_combining_class_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_unicode_combining_class_fct_##A == (uintptr_t)fct) return my_unicode_combining_class_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_unicode_combining_class_fct_##A == 0) {my_unicode_combining_class_fct_##A = (uintptr_t)fct; return my_unicode_combining_class_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz unicode combining class callback\n");
    return NULL;
}

// unicode compose
#define GO(A)                                                                       \
static uintptr_t my_unicode_compose_fct_##A = 0;                                    \
static int my_unicode_compose_##A(void* a, uint32_t b, uint32_t c, void* d, void* e)\
{                                                                                   \
    return (int)RunFunctionFmt(my_unicode_compose_fct_##A, "puupp", a, b, c, d, e); \
}
SUPER()
#undef GO
static void* find_unicode_compose_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_unicode_compose_fct_##A == (uintptr_t)fct) return my_unicode_compose_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_unicode_compose_fct_##A == 0) {my_unicode_compose_fct_##A = (uintptr_t)fct; return my_unicode_compose_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz unicode compose callback\n");
    return NULL;
}

// unicode decompose compatibility
#define GO(A)                                                                                   \
static uintptr_t my_unicode_decompose_compatibility_fct_##A = 0;                                \
static uint32_t my_unicode_decompose_compatibility_##A(void* a, uint32_t b, void* c, void* d)   \
{                                                                                               \
    return (uint32_t)RunFunctionFmt(my_unicode_decompose_compatibility_fct_##A, "pupp", a, b, c, d);    \
}
SUPER()
#undef GO
static void* find_unicode_decompose_compatibility_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_unicode_decompose_compatibility_fct_##A == (uintptr_t)fct) return my_unicode_decompose_compatibility_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_unicode_decompose_compatibility_fct_##A == 0) {my_unicode_decompose_compatibility_fct_##A = (uintptr_t)fct; return my_unicode_decompose_compatibility_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz unicode decompose compatibility callback\n");
    return NULL;
}

// unicode decompose
#define GO(A)                                                                       \
static uintptr_t my_unicode_decompose_fct_##A = 0;                                  \
static int my_unicode_decompose_##A(void* a, uint32_t b, void* c, void* d, void* e) \
{                                                                                   \
    return (int)RunFunctionFmt(my_unicode_decompose_fct_##A, "puppp", a, b, c, d, e);   \
}
SUPER()
#undef GO
static void* find_unicode_decompose_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_unicode_decompose_fct_##A == (uintptr_t)fct) return my_unicode_decompose_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_unicode_decompose_fct_##A == 0) {my_unicode_decompose_fct_##A = (uintptr_t)fct; return my_unicode_decompose_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz unicode decompose class callback\n");
    return NULL;
}

// unicode eastasian width
#define GO(A)                                                                   \
static uintptr_t my_unicode_eastasian_width_fct_##A = 0;                        \
static uint32_t my_unicode_eastasian_width_##A(void* a, uint32_t b, void* c)    \
{                                                                               \
    return (uint32_t)RunFunctionFmt(my_unicode_eastasian_width_fct_##A, "pup", a, b, c); \
}
SUPER()
#undef GO
static void* find_unicode_eastasian_width_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_unicode_eastasian_width_fct_##A == (uintptr_t)fct) return my_unicode_eastasian_width_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_unicode_eastasian_width_fct_##A == 0) {my_unicode_eastasian_width_fct_##A = (uintptr_t)fct; return my_unicode_eastasian_width_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz unicode eastasian width callback\n");
    return NULL;
}

// unicode general category
#define GO(A)                                                                   \
static uintptr_t my_unicode_general_category_fct_##A = 0;                       \
static uint32_t my_unicode_general_category_##A(void* a, uint32_t b, void* c)   \
{                                                                               \
    return (uint32_t)RunFunctionFmt(my_unicode_general_category_fct_##A, "pup", a, b, c);   \
}
SUPER()
#undef GO
static void* find_unicode_general_category_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_unicode_general_category_fct_##A == (uintptr_t)fct) return my_unicode_general_category_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_unicode_general_category_fct_##A == 0) {my_unicode_general_category_fct_##A = (uintptr_t)fct; return my_unicode_general_category_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz unicode general category callback\n");
    return NULL;
}

// unicode mirroring
#define GO(A)                                                           \
static uintptr_t my_unicode_mirroring_fct_##A = 0;                      \
static uint32_t my_unicode_mirroring_##A(void* a, uint32_t b, void* c)  \
{                                                                       \
    return (uint32_t)RunFunctionFmt(my_unicode_mirroring_fct_##A, "pup", a, b, c);  \
}
SUPER()
#undef GO
static void* find_unicode_mirroring_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_unicode_mirroring_fct_##A == (uintptr_t)fct) return my_unicode_mirroring_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_unicode_mirroring_fct_##A == 0) {my_unicode_mirroring_fct_##A = (uintptr_t)fct; return my_unicode_mirroring_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz unicode mirroring callback\n");
    return NULL;
}

// unicode script
#define GO(A)                                                       \
static uintptr_t my_unicode_script_fct_##A = 0;                     \
static uint32_t my_unicode_script_##A(void* a, uint32_t b, void* c) \
{                                                                   \
    return (uint32_t)RunFunctionFmt(my_unicode_script_fct_##A, "pup", a, b, c); \
}
SUPER()
#undef GO
static void* find_unicode_script_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_unicode_script_fct_##A == (uintptr_t)fct) return my_unicode_script_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_unicode_script_fct_##A == 0) {my_unicode_script_fct_##A = (uintptr_t)fct; return my_unicode_script_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz unicode script callback\n");
    return NULL;
}

// font extents
#define GO(A)                                                           \
static uintptr_t my_font_extents_fct_##A = 0;                           \
static int my_font_extents_##A(void* a, void* b, void* c, void* d) \
{                                                                       \
    return (int)RunFunctionFmt(my_font_extents_fct_##A, "pppp", a, b, c, d);   \
}
SUPER()
#undef GO
static void* find_font_extents_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_font_extents_fct_##A == (uintptr_t)fct) return my_font_extents_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_font_extents_fct_##A == 0) {my_font_extents_fct_##A = (uintptr_t)fct; return my_font_extents_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz font extents callback\n");
    return NULL;
}

// glyph advance
#define GO(A)                                                           \
static uintptr_t my_glyph_advance_fct_##A = 0;                          \
static int my_glyph_advance_##A(void* a, void* b, uint32_t c, void* d)  \
{                                                                       \
    return (int)RunFunctionFmt(my_glyph_advance_fct_##A, "ppup", a, b, c, d);   \
}
SUPER()
#undef GO
static void* find_glyph_advance_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_glyph_advance_fct_##A == (uintptr_t)fct) return my_glyph_advance_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_glyph_advance_fct_##A == 0) {my_glyph_advance_fct_##A = (uintptr_t)fct; return my_glyph_advance_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz glyph advance callback\n");
    return NULL;
}

// glyph advances
#define GO(A)                                                           \
static uintptr_t my_glyph_advances_fct_##A = 0;                         \
static void my_glyph_advances_##A(void* a, void* b, uint32_t c, void* d, uint32_t e, void* f, uint32_t g, void* h)  \
{                                                                       \
    RunFunctionFmt(my_glyph_advances_fct_##A, "ppupupup", a, b, c, d, e, f, g, h);  \
}
SUPER()
#undef GO
static void* find_glyph_advances_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_glyph_advances_fct_##A == (uintptr_t)fct) return my_glyph_advances_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_glyph_advances_fct_##A == 0) {my_glyph_advances_fct_##A = (uintptr_t)fct; return my_glyph_advances_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz glyph advances callback\n");
    return NULL;
}

// glyph kerning
#define GO(A)                                                           \
static uintptr_t my_glyph_kerning_fct_##A = 0;                          \
static int my_glyph_kerning_##A(void* a, void* b, uint32_t c, uint32_t d, void* e)  \
{                                                                       \
    return (int)RunFunctionFmt(my_glyph_kerning_fct_##A, "ppuup", a, b, c, d, e);   \
}
SUPER()
#undef GO
static void* find_glyph_kerning_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_glyph_kerning_fct_##A == (uintptr_t)fct) return my_glyph_kerning_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_glyph_kerning_fct_##A == 0) {my_glyph_kerning_fct_##A = (uintptr_t)fct; return my_glyph_kerning_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz glyph kerning callback\n");
    return NULL;
}

// glyph origin
#define GO(A)                                                           \
static uintptr_t my_glyph_origin_fct_##A = 0;                           \
static int my_glyph_origin_##A(void* a, void* b, uint32_t c, void* d, void* e, void* f) \
{                                                                       \
    return (int)RunFunctionFmt(my_glyph_origin_fct_##A, "ppuppp", a, b, c, d, e, f);   \
}
SUPER()
#undef GO
static void* find_glyph_origin_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_glyph_origin_fct_##A == (uintptr_t)fct) return my_glyph_origin_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_glyph_origin_fct_##A == 0) {my_glyph_origin_fct_##A = (uintptr_t)fct; return my_glyph_origin_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz glyph origin callback\n");
    return NULL;
}

// glyph contour point
#define GO(A)                                                           \
static uintptr_t my_glyph_contour_pointfct_##A = 0;                     \
static int my_glyph_contour_point##A(void* a, void* b, uint32_t c, uint32_t d, void* e, void* f, void* g) \
{                                                                       \
    return (int)RunFunctionFmt(my_glyph_contour_pointfct_##A, "ppuuppp", a, b, c, d, e, f, g);   \
}
SUPER()
#undef GO
static void* find_glyph_contour_point_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_glyph_contour_pointfct_##A == (uintptr_t)fct) return my_glyph_contour_point##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_glyph_contour_pointfct_##A == 0) {my_glyph_contour_pointfct_##A = (uintptr_t)fct; return my_glyph_contour_point##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz glyph contour point callback\n");
    return NULL;
}

// glyph extents
#define GO(A)                                                           \
static uintptr_t my_glyph_extents_fct_##A = 0;                           \
static int my_glyph_extents_##A(void* a, void* b, uint32_t c, void* d, void* e) \
{                                                                       \
    return (int)RunFunctionFmt(my_glyph_extents_fct_##A, "ppupp", a, b, c, d, e);   \
}
SUPER()
#undef GO
static void* find_glyph_extents_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_glyph_extents_fct_##A == (uintptr_t)fct) return my_glyph_extents_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_glyph_extents_fct_##A == 0) {my_glyph_extents_fct_##A = (uintptr_t)fct; return my_glyph_extents_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz glyph extents callback\n");
    return NULL;
}

// glyph from name
#define GO(A)                                                           \
static uintptr_t my_glyph_from_name_fct_##A = 0;                        \
static int my_glyph_from_name_##A(void* a, void* b, void* c, int d, void* e, void* f)   \
{                                                                       \
    return (int)RunFunctionFmt(my_glyph_from_name_fct_##A, "pppipp", a, b, c, d, e, f); \
}
SUPER()
#undef GO
static void* find_glyph_from_name_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_glyph_from_name_fct_##A == (uintptr_t)fct) return my_glyph_from_name_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_glyph_from_name_fct_##A == 0) {my_glyph_from_name_fct_##A = (uintptr_t)fct; return my_glyph_from_name_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz glyph from name callback\n");
    return NULL;
}






// glyph
#define GO(A)                                                           \
static uintptr_t my_glyph_fct_##A = 0;                           \
static int my_glyph_##A(void* a, void* b, uint32_t c, uint32_t d, void* e, void* f) \
{                                                                       \
    return (int)RunFunctionFmt(my_glyph_fct_##A, "ppuupp", a, b, c, d, e, f);   \
}
SUPER()
#undef GO
static void* find_glyph_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_glyph_fct_##A == (uintptr_t)fct) return my_glyph_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_glyph_fct_##A == 0) {my_glyph_fct_##A = (uintptr_t)fct; return my_glyph_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz glyph callback\n");
    return NULL;
}

// glyph name
#define GO(A)                               \
static uintptr_t my_glyph_name_fct_##A = 0; \
static int my_glyph_name_##A(void* a, void* b, uint32_t c, void* d, uint32_t e, void* f)    \
{                                           \
    return (int)RunFunctionFmt(my_glyph_name_fct_##A, "ppupup", a, b, c, d, e, f);  \
}
SUPER()
#undef GO
static void* find_glyph_name_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_glyph_name_fct_##A == (uintptr_t)fct) return my_glyph_name_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_glyph_name_fct_##A == 0) {my_glyph_name_fct_##A = (uintptr_t)fct; return my_glyph_name_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz glyph origin callback\n");
    return NULL;
}

// glyph shape
#define GO(A)                                                           \
static uintptr_t my_glyph_shape_fct_##A = 0;                           \
static void my_glyph_shape_##A(void* a, void* b, uint32_t c, void* d, void* e, void* f) \
{                                                                       \
    RunFunctionFmt(my_glyph_shape_fct_##A, "ppuppp", a, b, c, d, e, f);   \
}
SUPER()
#undef GO
static void* find_glyph_shape_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_glyph_shape_fct_##A == (uintptr_t)fct) return my_glyph_shape_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_glyph_shape_fct_##A == 0) {my_glyph_shape_fct_##A = (uintptr_t)fct; return my_glyph_shape_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz glyph shape callback\n");
    return NULL;
}

// nominal glyph
#define GO(A)                                   \
static uintptr_t my_nominal_glyph_fct_##A = 0;  \
static int my_nominal_glyph_##A(void* a, void* b, uint32_t c, void* d, void* e) \
{                                               \
    return (int)RunFunctionFmt(my_nominal_glyph_fct_##A, "ppupp", a, b, c, d, e);   \
}
SUPER()
#undef GO
static void* find_nominal_glyph_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_nominal_glyph_fct_##A == (uintptr_t)fct) return my_nominal_glyph_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_nominal_glyph_fct_##A == 0) {my_nominal_glyph_fct_##A = (uintptr_t)fct; return my_nominal_glyph_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz nominal glyph callback\n");
    return NULL;
}






// nominal glyphs
#define GO(A)                                   \
static uintptr_t my_nominal_glyphs_fct_##A = 0; \
static uint32_t my_nominal_glyphs_##A(void* a, void* b, uint32_t c, void* d, uint32_t e, void* f, uint32_t g, void* h)  \
{                                               \
    return (uint32_t)RunFunctionFmt(my_nominal_glyphs_fct_##A, "ppupupup", a, b, c, d, e, f, g, h); \
}
SUPER()
#undef GO
static void* find_nominal_glyphs_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_nominal_glyphs_fct_##A == (uintptr_t)fct) return my_nominal_glyphs_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_nominal_glyphs_fct_##A == 0) {my_nominal_glyphs_fct_##A = (uintptr_t)fct; return my_nominal_glyphs_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz nominal glyphs callback\n");
    return NULL;
}

// variation glyph
#define GO(A)                                   \
static uintptr_t my_variation_glyph_fct_##A = 0;\
static int my_variation_glyph_##A(void* a, void* b, uint32_t c, uint32_t d, void* e, void* f) \
{                                               \
    return (int)RunFunctionFmt(my_variation_glyph_fct_##A, "ppuupp", a, b, c, d, e, f);   \
}
SUPER()
#undef GO
static void* find_variation_glyph_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_variation_glyph_fct_##A == (uintptr_t)fct) return my_variation_glyph_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_variation_glyph_fct_##A == 0) {my_variation_glyph_fct_##A = (uintptr_t)fct; return my_variation_glyph_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz variation glyph callback\n");
    return NULL;
}

// destroy
#define GO(A)                                               \
static uintptr_t my_destroy_fct_##A = 0;                    \
static void my_destroy_##A(void* a)                         \
{                                                           \
    RunFunctionFmt(my_destroy_fct_##A, "p", a); \
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

EXPORT void my_hb_unicode_funcs_destroy(x64emu_t* emu, void* funcs)
{
    (void)emu;
    struct hb_unicode_funcs_t__destroy destroy = {0};
    struct hb_unicode_funcs_t* funcs_ = funcs;

#define HB_UNICODE_FUNC_IMPLEMENT(name) \
    if (funcs_->destroy.name) destroy.name = find_destroy_Fct(funcs_->destroy.name);
    HB_UNICODE_FUNCS_IMPLEMENT_CALLBACKS
#undef HB_UNICODE_FUNC_IMPLEMENT

    struct hb_unicode_funcs_t__destroy original = funcs_->destroy;
    funcs_->destroy = destroy;
    my->hb_font_funcs_destroy(funcs);
    funcs_->destroy = original;
}

EXPORT void my_hb_unicode_funcs_set_combining_class_func(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)
{
    (void)emu;
    my->hb_unicode_funcs_set_combining_class_func(funcs, find_unicode_combining_class_Fct(func), user_data, find_destroy_Fct(destroy));
}

EXPORT void my_hb_unicode_funcs_set_compose_func(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)
{
    (void)emu;
    my->hb_unicode_funcs_set_compose_func(funcs, find_unicode_compose_Fct(func), user_data, find_destroy_Fct(destroy));
}

EXPORT void my_hb_unicode_funcs_set_decompose_compatibility_func(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)
{
    (void)emu;
    my->hb_unicode_funcs_set_decompose_compatibility_func(funcs, find_unicode_decompose_compatibility_Fct(func), user_data, find_destroy_Fct(destroy));
}

EXPORT void my_hb_unicode_funcs_set_decompose_func(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)
{
    (void)emu;
    my->hb_unicode_funcs_set_decompose_func(funcs, find_unicode_decompose_Fct(func), user_data, find_destroy_Fct(destroy));
}

EXPORT void my_hb_unicode_funcs_set_eastasian_width_func(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)
{
    (void)emu;
    my->hb_unicode_funcs_set_eastasian_width_func(funcs, find_unicode_eastasian_width_Fct(func), user_data, find_destroy_Fct(destroy));
}

EXPORT void my_hb_unicode_funcs_set_general_category_func(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)
{
    (void)emu;
    my->hb_unicode_funcs_set_general_category_func(funcs, find_unicode_general_category_Fct(func), user_data, find_destroy_Fct(destroy));
}

EXPORT void my_hb_unicode_funcs_set_mirroring_func(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)
{
    (void)emu;
    my->hb_unicode_funcs_set_mirroring_func(funcs, find_unicode_mirroring_Fct(func), user_data, find_destroy_Fct(destroy));
}

EXPORT void my_hb_unicode_funcs_set_script_func(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)
{
    (void)emu;
    my->hb_unicode_funcs_set_script_func(funcs, find_unicode_script_Fct(func), user_data, find_destroy_Fct(destroy));
}

EXPORT int my_hb_unicode_funcs_set_user_data(x64emu_t* emu, void* funcs, void* key, void* data, void* destroy, int replace)
{
    (void)emu;
    return (int)my->hb_unicode_funcs_set_user_data(funcs, key, data, find_destroy_Fct(destroy), replace);
}

#define FUNC(A) \
EXPORT void my_##A(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)  \
{               \
    (void)emu;  \
    my->A(funcs, find_font_extents_Fct(func), user_data, find_destroy_Fct(destroy));\
}

FUNC(hb_font_funcs_set_font_h_extents_func)
FUNC(hb_font_funcs_set_font_v_extents_func)

#undef FUNC


#define FUNC(A) \
EXPORT void my_##A(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)  \
{               \
    (void)emu;  \
    my->A(funcs, find_glyph_advance_Fct(func), user_data, find_destroy_Fct(destroy));   \
}

FUNC(hb_font_funcs_set_glyph_h_advance_func)
FUNC(hb_font_funcs_set_glyph_v_advance_func)

#undef FUNC


#define FUNC(A) \
EXPORT void my_##A(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)  \
{               \
    (void)emu;  \
    my->A(funcs, find_glyph_advances_Fct(func), user_data, find_destroy_Fct(destroy));  \
}

FUNC(hb_font_funcs_set_glyph_h_advances_func)
FUNC(hb_font_funcs_set_glyph_v_advances_func)

#undef FUNC


#define FUNC(A) \
EXPORT void my_##A(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)  \
{               \
    (void)emu;  \
    my->A(funcs, find_glyph_kerning_Fct(func), user_data, find_destroy_Fct(destroy));   \
}

FUNC(hb_font_funcs_set_glyph_h_kerning_func)
FUNC(hb_font_funcs_set_glyph_v_kerning_func)

#undef FUNC


#define FUNC(A) \
EXPORT void my_##A(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)  \
{               \
    (void)emu;  \
    my->A(funcs, find_glyph_origin_Fct(func), user_data, find_destroy_Fct(destroy));\
}

FUNC(hb_font_funcs_set_glyph_h_origin_func)
FUNC(hb_font_funcs_set_glyph_v_origin_func)

#undef FUNC

EXPORT void my_hb_font_funcs_set_glyph_contour_point_func(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)
{
    (void)emu;
    my->hb_font_funcs_set_glyph_contour_point_func(funcs, find_glyph_contour_point_Fct(func), user_data, find_destroy_Fct(destroy));
}

EXPORT void my_hb_font_funcs_set_glyph_extents_func(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)
{
    (void)emu;
    my->hb_font_funcs_set_glyph_extents_func(funcs, find_glyph_extents_Fct(func), user_data, find_destroy_Fct(destroy));
}

EXPORT void my_hb_font_funcs_set_glyph_from_name_func(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)
{
    (void)emu;
    my->hb_font_funcs_set_glyph_from_name_func(funcs, find_glyph_from_name_Fct(func), user_data, find_destroy_Fct(destroy));
}

EXPORT void my_hb_font_funcs_set_glyph_func(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)
{
    (void)emu;
    my->hb_font_funcs_set_glyph_func(funcs, find_glyph_Fct(func), user_data, find_destroy_Fct(destroy));
}

EXPORT void my_hb_font_funcs_set_glyph_name_func(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)
{
    (void)emu;
    my->hb_font_funcs_set_glyph_name_func(funcs, find_glyph_name_Fct(func), user_data, find_destroy_Fct(destroy));
}

EXPORT void my_hb_font_funcs_set_glyph_shape_func(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)
{
    (void)emu;
    my->hb_font_funcs_set_glyph_shape_func(funcs, find_glyph_shape_Fct(func), user_data, find_destroy_Fct(destroy));
}

EXPORT void my_hb_font_funcs_set_nominal_glyph_func(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)
{
    (void)emu;
    my->hb_font_funcs_set_nominal_glyph_func(funcs, find_nominal_glyph_Fct(func), user_data, find_destroy_Fct(destroy));
}

EXPORT void my_hb_font_funcs_set_nominal_glyphs_func(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)
{
    (void)emu;
    my->hb_font_funcs_set_nominal_glyphs_func(funcs, find_nominal_glyphs_Fct(func), user_data, find_destroy_Fct(destroy));
}

EXPORT void my_hb_font_funcs_set_variation_glyph_func(x64emu_t* emu, void* funcs, void* func, void* user_data, void* destroy)
{
    (void)emu;
    my->hb_font_funcs_set_variation_glyph_func(funcs, find_variation_glyph_Fct(func), user_data, find_destroy_Fct(destroy));
}

EXPORT int my_hb_font_funcs_set_user_data(x64emu_t* emu, void* funcs, void* key, void* data, void* destroy, int replace)
{
    (void)emu;
    return (int)my->hb_font_funcs_set_user_data(funcs, key, data, find_destroy_Fct(destroy), replace);
}

EXPORT void my_hb_font_set_funcs(x64emu_t* emu, void* font, void* klass, void* data, void* destroy)
{
    (void)emu;
    my->hb_font_set_funcs(font, klass, data, find_destroy_Fct(destroy));
}

EXPORT void my_hb_font_set_funcs_data(x64emu_t* emu, void* font, void* data, void* destroy)
{
    (void)emu;
    my->hb_font_set_funcs_data(font, data, find_destroy_Fct(destroy));
}

EXPORT int my_hb_font_set_user_data(x64emu_t* emu, void* font, void* key, void* data, void* destroy, int replace)
{
    (void)emu;
    return (int)my->hb_font_set_user_data(font, key, data, find_destroy_Fct(destroy), replace);
}

EXPORT void* my_hb_ft_face_create(x64emu_t* emu, void* face, void* destroy)
{
    (void)emu;
    return my->hb_ft_face_create(face, find_destroy_Fct(destroy));
}

#include "wrappedlib_init.h"
