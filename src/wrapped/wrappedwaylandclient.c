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


const char* waylandclientName = "libwayland-client.so.0";
#define LIBNAME waylandclient

#define ADDED_FUNCTIONS() \

#include "generated/wrappedwaylandclienttypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// wl_registry_listener ...
typedef struct my_wl_registry_listener_s {
    uintptr_t   global; //vFppupu
    uintptr_t   global_remove;  //vFppu
} my_wl_registry_listener_t;
#define GO(A)   \
static my_wl_registry_listener_t* ref_wl_registry_listener_##A = NULL;                              \
static void my_wl_registry_listener_global_##A(void* a, void* b, uint32_t c, void* d, uint32_t e)   \
{                                                                                                   \
    RunFunctionFmt(ref_wl_registry_listener_##A->global, "ppupu", a, b, c, d, e);                   \
}                                                                                                   \
static void my_wl_registry_listener_global_remove_##A(void* a, void* b, uint32_t c)                 \
{                                                                                                   \
    RunFunctionFmt(ref_wl_registry_listener_##A->global_remove, "ppu", a, b, c);                    \
}                                                                                                   \
static my_wl_registry_listener_t my_wl_registry_listener_fct_##A = {                                \
    (uintptr_t)my_wl_registry_listener_global_##A,                                                  \
    (uintptr_t)my_wl_registry_listener_global_remove_##A                                            \
};
SUPER()
#undef GO
static void* find_wl_registry_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_wl_registry_listener_##A == fct) return &my_wl_registry_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_wl_registry_listener_##A == 0) {ref_wl_registry_listener_##A = fct; return &my_wl_registry_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client wl_registry_listener callback\n");
    return NULL;
}
// xdg_surface_listener ...
typedef struct my_xdg_surface_listener_s {
    uintptr_t   configure; //vFppiipu
    uintptr_t   close;  //vFpp
} my_xdg_surface_listener_t;
#define GO(A)   \
static my_xdg_surface_listener_t* ref_xdg_surface_listener_##A = NULL;                                  \
static void my_xdg_surface_listener_configure_##A(void* a, void* b, int c, int d, void* e, uint32_t f)  \
{                                                                                                       \
    RunFunctionFmt(ref_xdg_surface_listener_##A->configure, "ppiipu", a, b, c, d, e, f);                \
}                                                                                                       \
static void my_xdg_surface_listener_close_##A(void* a, void* b)                                         \
{                                                                                                       \
    RunFunctionFmt(ref_xdg_surface_listener_##A->close, "pp", a, b);                                    \
}                                                                                                       \
static my_xdg_surface_listener_t my_xdg_surface_listener_fct_##A = {                                    \
    (uintptr_t)my_xdg_surface_listener_configure_##A,                                                   \
    (uintptr_t)my_xdg_surface_listener_close_##A                                                        \
};
SUPER()
#undef GO
static void* find_xdg_surface_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_xdg_surface_listener_##A == fct) return &my_xdg_surface_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_xdg_surface_listener_##A == 0) {ref_xdg_surface_listener_##A = fct; return &my_xdg_surface_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client xdg_surface_listener callback\n");
    return NULL;
}
// xdg_toplevel_listener ...
typedef struct my_xdg_toplevel_listener_s {
    uintptr_t   configure; //vFppiip
    uintptr_t   close;  //vFpp
} my_xdg_toplevel_listener_t;
#define GO(A)   \
static my_xdg_toplevel_listener_t* ref_xdg_toplevel_listener_##A = NULL;                            \
static void my_xdg_toplevel_listener_configure_##A(void* a, void* b, int c, int d, void* e)         \
{                                                                                                   \
    RunFunctionFmt(ref_xdg_toplevel_listener_##A->configure, "ppiip", a, b, c, d, e);               \
}                                                                                                   \
static void my_xdg_toplevel_listener_close_##A(void* a, void* b)                                    \
{                                                                                                   \
    RunFunctionFmt(ref_xdg_toplevel_listener_##A->close, "pp", a, b);                               \
}                                                                                                   \
static my_xdg_toplevel_listener_t my_xdg_toplevel_listener_fct_##A = {                              \
    (uintptr_t)my_xdg_toplevel_listener_configure_##A,                                              \
    (uintptr_t)my_xdg_toplevel_listener_close_##A                                                   \
};
SUPER()
#undef GO
static void* find_xdg_toplevel_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_xdg_toplevel_listener_##A == fct) return &my_xdg_toplevel_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_xdg_toplevel_listener_##A == 0) {ref_xdg_toplevel_listener_##A = fct; return &my_xdg_toplevel_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client xdg_toplevel_listener callback\n");
    return NULL;
}
// xdg_wm_base_listener ...
typedef struct my_xdg_wm_base_listener_s {
    uintptr_t   ping; //vFppu
} my_xdg_wm_base_listener_t;
#define GO(A)   \
static my_xdg_wm_base_listener_t* ref_xdg_wm_base_listener_##A = NULL;                \
static void my_xdg_wm_base_listener_ping_##A(void* a, void* b, uint32_t c)            \
{                                                                                     \
    RunFunctionFmt(ref_xdg_wm_base_listener_##A->ping, "ppu", a, b, c);               \
}                                                                                     \
static my_xdg_wm_base_listener_t my_xdg_wm_base_listener_fct_##A = {                  \
    (uintptr_t)my_xdg_wm_base_listener_ping_##A,                                      \
};
SUPER()
#undef GO
static void* find_xdg_wm_base_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_xdg_wm_base_listener_##A == fct) return &my_xdg_wm_base_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_xdg_wm_base_listener_##A == 0) {ref_xdg_wm_base_listener_##A = fct; return &my_xdg_wm_base_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client xdg_wm_base_listener callback\n");
    return NULL;
}
// wl_shm_listener ...
typedef struct my_wl_shm_listener_s {
    uintptr_t   format; //vFppu
} my_wl_shm_listener_t;
#define GO(A)   \
static my_wl_shm_listener_t* ref_wl_shm_listener_##A = NULL;                       \
static void my_wl_shm_listener_format_##A(void* a, void* b, uint32_t c)            \
{                                                                                  \
    RunFunctionFmt(ref_wl_shm_listener_##A->format, "ppu", a, b, c);               \
}                                                                                  \
static my_wl_shm_listener_t my_wl_shm_listener_fct_##A = {                         \
    (uintptr_t)my_wl_shm_listener_format_##A,                                      \
};
SUPER()
#undef GO
static void* find_wl_shm_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_wl_shm_listener_##A == fct) return &my_wl_shm_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_wl_shm_listener_##A == 0) {ref_wl_shm_listener_##A = fct; return &my_wl_shm_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client wl_shm_listener callback\n");
    return NULL;
}
// wl_output_listener ...
typedef struct my_wl_output_listener_s {
    uintptr_t   geometry; //vFppiiiiippi
    uintptr_t   mode;  //vFppuiii
    uintptr_t   done;  //vFpp
    uintptr_t   scale;  //vFppi
    uintptr_t   name;   //vFppp
    uintptr_t   description;   //vFppp
} my_wl_output_listener_t;
#define GO(A)   \
static my_wl_output_listener_t* ref_wl_output_listener_##A = NULL;                                                            \
static void my_wl_output_listener_geometry_##A(void* a, void* b, int c, int d, int e, int f, int g, void* h, void* i, int j)  \
{                                                                                                                             \
    RunFunctionFmt(ref_wl_output_listener_##A->geometry, "ppiiiiippi", a, b, c, d, e, f, g, h, i, j);                         \
}                                                                                                                             \
static void my_wl_output_listener_mode_##A(void* a, void* b, uint32_t c, int d, int e, int f)                                 \
{                                                                                                                             \
    RunFunctionFmt(ref_wl_output_listener_##A->mode, "ppuiii", a, b, c, d, e, f);                                             \
}                                                                                                                             \
static void my_wl_output_listener_done_##A(void* a, void* b)                                                                  \
{                                                                                                                             \
    RunFunctionFmt(ref_wl_output_listener_##A->done, "pp", a, b);                                                             \
}                                                                                                                             \
static void my_wl_output_listener_scale_##A(void* a, void* b, int c)                                                          \
{                                                                                                                             \
    RunFunctionFmt(ref_wl_output_listener_##A->scale, "ppi", a, b, c);                                                        \
}                                                                                                                             \
static void my_wl_output_listener_name_##A(void* a, void* b, void* c)                                                         \
{                                                                                                                             \
    RunFunctionFmt(ref_wl_output_listener_##A->name, "ppp", a, b, c);                                                         \
}                                                                                                                             \
static void my_wl_output_listener_description_##A(void* a, void* b, void* c)                                                  \
{                                                                                                                             \
    RunFunctionFmt(ref_wl_output_listener_##A->description, "ppp", a, b, c);                                                  \
}                                                                                                                             \
static my_wl_output_listener_t my_wl_output_listener_fct_##A = {                                                              \
    (uintptr_t)my_wl_output_listener_geometry_##A,                                                                            \
    (uintptr_t)my_wl_output_listener_mode_##A,                                                                                \
    (uintptr_t)my_wl_output_listener_done_##A,                                                                                \
    (uintptr_t)my_wl_output_listener_scale_##A,                                                                               \
    (uintptr_t)my_wl_output_listener_name_##A,                                                                                \
    (uintptr_t)my_wl_output_listener_description_##A                                                                          \
};
SUPER()
#undef GO
static void* find_wl_output_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_wl_output_listener_##A == fct) return &my_wl_output_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_wl_output_listener_##A == 0) {ref_wl_output_listener_##A = fct; return &my_wl_output_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client wl_output_listener callback\n");
    return NULL;
}
// wl_seat_listener ...
typedef struct my_wl_seat_listener_s {
    uintptr_t   capabilities; //vFppu
    uintptr_t   name;  //vFppp
} my_wl_seat_listener_t;
#define GO(A)   \
static my_wl_seat_listener_t* ref_wl_seat_listener_##A = NULL;                            \
static void my_wl_seat_listener_capabilities_##A(void* a, void* b, uint32_t c)            \
{                                                                                         \
    RunFunctionFmt(ref_wl_seat_listener_##A->capabilities, "ppu", a, b, c);               \
}                                                                                         \
static void my_wl_seat_listener_name_##A(void* a, void* b, void* c)                       \
{                                                                                         \
    RunFunctionFmt(ref_wl_seat_listener_##A->name, "ppp", a, b, c);                       \
}                                                                                         \
static my_wl_seat_listener_t my_wl_seat_listener_fct_##A = {                              \
    (uintptr_t)my_wl_seat_listener_capabilities_##A,                                      \
    (uintptr_t)my_wl_seat_listener_name_##A                                               \
};
SUPER()
#undef GO
static void* find_wl_seat_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_wl_seat_listener_##A == fct) return &my_wl_seat_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_wl_seat_listener_##A == 0) {ref_wl_seat_listener_##A = fct; return &my_wl_seat_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client wl_seat_listener callback\n");
    return NULL;
}
// wl_pointer_listener ...
typedef struct my_wl_pointer_listener_s {
    uintptr_t   enter; //vFppupii
    uintptr_t   leave;  //vFppup
    uintptr_t   motion;  //vFppuii
    uintptr_t   button;  //vFppuuuu
    uintptr_t   axis;   //vFppuui
    uintptr_t   frame;   //vFpp
    uintptr_t   axis_source;   //vFppu
    uintptr_t   axis_stop;   //vFppuu
    uintptr_t   axis_discrete;   //vFppui
    uintptr_t   axis_value120;   //vFppui
    uintptr_t   axis_relative_direction;   //vFppuu
} my_wl_pointer_listener_t;
#define GO(A)   \
static my_wl_pointer_listener_t* ref_wl_pointer_listener_##A = NULL;                                             \
static void my_wl_pointer_listener_enter_##A(void* a, void* b, uint32_t c, void* d, int e, int f)                \
{                                                                                                                \
    RunFunctionFmt(ref_wl_pointer_listener_##A->enter, "ppupii", a, b, c, d, e, f);                              \
}                                                                                                                \
static void my_wl_pointer_listener_leave_##A(void* a, void* b, uint32_t c, void* d)                              \
{                                                                                                                \
    RunFunctionFmt(ref_wl_pointer_listener_##A->leave, "ppup", a, b, c, d);                                      \
}                                                                                                                \
static void my_wl_pointer_listener_motion_##A(void* a, void* b, uint32_t c, int d, int e)                        \
{                                                                                                                \
    RunFunctionFmt(ref_wl_pointer_listener_##A->motion, "ppuii", a, b, c, d, e);                                 \
}                                                                                                                \
static void my_wl_pointer_listener_button_##A(void* a, void* b, uint32_t c, uint32_t d, uint32_t e, uint32_t f)  \
{                                                                                                                \
    RunFunctionFmt(ref_wl_pointer_listener_##A->button, "ppuuuu", a, b, c, d, e, f);                             \
}                                                                                                                \
static void my_wl_pointer_listener_axis_##A(void* a, void* b, uint32_t c, uint32_t d, int e)                     \
{                                                                                                                \
    RunFunctionFmt(ref_wl_pointer_listener_##A->axis, "ppuui", a, b, c, d, e);                                   \
}                                                                                                                \
static void my_wl_pointer_listener_frame_##A(void* a, void* b)                                                   \
{                                                                                                                \
    RunFunctionFmt(ref_wl_pointer_listener_##A->frame, "pp", a, b);                                              \
}                                                                                                                \
static void my_wl_pointer_listener_axis_source_##A(void* a, void* b, uint32_t c)                                 \
{                                                                                                                \
    RunFunctionFmt(ref_wl_pointer_listener_##A->axis_source, "ppu", a, b, c);                                    \
}                                                                                                                \
static void my_wl_pointer_listener_axis_stop_##A(void* a, void* b, uint32_t c, uint32_t d)                       \
{                                                                                                                \
    RunFunctionFmt(ref_wl_pointer_listener_##A->axis_stop, "ppuu", a, b, c, d);                                  \
}                                                                                                                \
static void my_wl_pointer_listener_axis_discrete_##A(void* a, void* b, uint32_t c, int d)                        \
{                                                                                                                \
    RunFunctionFmt(ref_wl_pointer_listener_##A->axis_discrete, "ppui", a, b, c, d);                              \
}                                                                                                                \
static void my_wl_pointer_listener_axis_value120_##A(void* a, void* b, uint32_t c, int d)                        \
{                                                                                                                \
    RunFunctionFmt(ref_wl_pointer_listener_##A->axis_value120, "ppui", a, b, c, d);                              \
}                                                                                                                \
static void my_wl_pointer_listener_axis_relative_direction_##A(void* a, void* b, uint32_t c, uint32_t d)         \
{                                                                                                                \
    RunFunctionFmt(ref_wl_pointer_listener_##A->axis_relative_direction, "ppuu", a, b, c, d);                    \
}                                                                                                                \
static my_wl_pointer_listener_t my_wl_pointer_listener_fct_##A = {                                               \
    (uintptr_t)my_wl_pointer_listener_enter_##A,                                                                 \
    (uintptr_t)my_wl_pointer_listener_leave_##A,                                                                 \
    (uintptr_t)my_wl_pointer_listener_motion_##A,                                                                \
    (uintptr_t)my_wl_pointer_listener_button_##A,                                                                \
    (uintptr_t)my_wl_pointer_listener_axis_##A,                                                                  \
    (uintptr_t)my_wl_pointer_listener_frame_##A,                                                                 \
    (uintptr_t)my_wl_pointer_listener_axis_source_##A,                                                           \
    (uintptr_t)my_wl_pointer_listener_axis_stop_##A,                                                             \
    (uintptr_t)my_wl_pointer_listener_axis_discrete_##A,                                                         \
    (uintptr_t)my_wl_pointer_listener_axis_value120_##A,                                                         \
    (uintptr_t)my_wl_pointer_listener_axis_relative_direction_##A                                                \
};
SUPER()
#undef GO
static void* find_wl_pointer_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_wl_pointer_listener_##A == fct) return &my_wl_pointer_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_wl_pointer_listener_##A == 0) {ref_wl_pointer_listener_##A = fct; return &my_wl_pointer_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client wl_pointer_listener callback\n");
    return NULL;
}
// wl_keyboard_listener ...
typedef struct my_wl_keyboard_listener_s {
    uintptr_t   keymap; //vFppuiu
    uintptr_t   enter;  //vFppup
    uintptr_t   leave;  //vFppup
    uintptr_t   key;  //vFppuuuuu
    uintptr_t   modifiers;   //vFppuuuuu
    uintptr_t   repeat_info;   //vFppii
} my_wl_keyboard_listener_t;
#define GO(A)   \
static my_wl_keyboard_listener_t* ref_wl_keyboard_listener_##A = NULL;                                                           \
static void my_wl_keyboard_listener_keymap_##A(void* a, void* b, uint32_t c, int d, uint32_t e)                                  \
{                                                                                                                                \
    RunFunctionFmt(ref_wl_keyboard_listener_##A->keymap, "ppuiu", a, b, c, d, e);                                                \
}                                                                                                                                \
static void my_wl_keyboard_listener_enter_##A(void* a, void* b, uint32_t c, void* d)                                             \
{                                                                                                                                \
    RunFunctionFmt(ref_wl_keyboard_listener_##A->enter, "ppup", a, b, c, d);                                                     \
}                                                                                                                                \
static void my_wl_keyboard_listener_leave_##A(void* a, void* b, uint32_t c, void* d)                                             \
{                                                                                                                                \
    RunFunctionFmt(ref_wl_keyboard_listener_##A->leave, "ppup", a, b, c, d);                                                     \
}                                                                                                                                \
static void my_wl_keyboard_listener_key_##A(void* a, void* b, uint32_t c, uint32_t d, uint32_t e, uint32_t f, uint32_t g)        \
{                                                                                                                                \
    RunFunctionFmt(ref_wl_keyboard_listener_##A->key, "ppuuuuu", a, b, c, d, e, f, g);                                           \
}                                                                                                                                \
static void my_wl_keyboard_listener_modifiers_##A(void* a, void* b, uint32_t c, uint32_t d, uint32_t e, uint32_t f, uint32_t g)  \
{                                                                                                                                \
    RunFunctionFmt(ref_wl_keyboard_listener_##A->modifiers, "ppuuuuu", a, b, c, d, e, f, g);                                     \
}                                                                                                                                \
static void my_wl_keyboard_listener_repeat_info_##A(void* a, void* b, int c, int d)                                              \
{                                                                                                                                \
    RunFunctionFmt(ref_wl_keyboard_listener_##A->repeat_info, "ppii", a, b, c, d);                                               \
}                                                                                                                                \
static my_wl_keyboard_listener_t my_wl_keyboard_listener_fct_##A = {                                                             \
    (uintptr_t)my_wl_keyboard_listener_keymap_##A,                                                                               \
    (uintptr_t)my_wl_keyboard_listener_enter_##A,                                                                                \
    (uintptr_t)my_wl_keyboard_listener_leave_##A,                                                                                \
    (uintptr_t)my_wl_keyboard_listener_key_##A,                                                                                  \
    (uintptr_t)my_wl_keyboard_listener_modifiers_##A,                                                                            \
    (uintptr_t)my_wl_keyboard_listener_repeat_info_##A                                                                           \
};
SUPER()
#undef GO
static void* find_wl_keyboard_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_wl_keyboard_listener_##A == fct) return &my_wl_keyboard_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_wl_keyboard_listener_##A == 0) {ref_wl_keyboard_listener_##A = fct; return &my_wl_keyboard_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client wl_keyboard_listener callback\n");
    return NULL;
}
// wl_buffer_listener ...
typedef struct my_wl_buffer_listener_s {
    uintptr_t   release; //vFpp
} my_wl_buffer_listener_t;
#define GO(A)   \
static my_wl_buffer_listener_t* ref_wl_buffer_listener_##A = NULL;                 \
static void my_wl_buffer_listener_release_##A(void* a, void* b)                    \
{                                                                                  \
    RunFunctionFmt(ref_wl_buffer_listener_##A->release, "pp", a, b);               \
}                                                                                  \
static my_wl_buffer_listener_t my_wl_buffer_listener_fct_##A = {                   \
    (uintptr_t)my_wl_buffer_listener_release_##A,                                  \
};
SUPER()
#undef GO
static void* find_wl_buffer_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_wl_buffer_listener_##A == fct) return &my_wl_buffer_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_wl_buffer_listener_##A == 0) {ref_wl_buffer_listener_##A = fct; return &my_wl_buffer_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client wl_buffer_listener callback\n");
    return NULL;
}
// zwp_relative_pointer_v1_listener ...
typedef struct my_zwp_relative_pointer_v1_listener_s {
    uintptr_t   relative_motion; //vFppuuiiii
} my_zwp_relative_pointer_v1_listener_t;
#define GO(A)   \
static my_zwp_relative_pointer_v1_listener_t* ref_zwp_relative_pointer_v1_listener_##A = NULL;                                              \
static void my_zwp_relative_pointer_v1_listener_relative_motion_##A(void* a, void* b, uint32_t c, uint32_t d, int e, int f, int g, int h)   \
{                                                                                                                                           \
    RunFunctionFmt(ref_zwp_relative_pointer_v1_listener_##A->relative_motion, "ppuuiiii", a, b, c, d, e, f, g, h);                          \
}                                                                                                                                           \
static my_zwp_relative_pointer_v1_listener_t my_zwp_relative_pointer_v1_listener_fct_##A = {                                                \
    (uintptr_t)my_zwp_relative_pointer_v1_listener_relative_motion_##A,                                                                     \
};
SUPER()
#undef GO
static void* find_zwp_relative_pointer_v1_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_zwp_relative_pointer_v1_listener_##A == fct) return &my_zwp_relative_pointer_v1_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_zwp_relative_pointer_v1_listener_##A == 0) {ref_zwp_relative_pointer_v1_listener_##A = fct; return &my_zwp_relative_pointer_v1_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client zwp_relative_pointer_v1_listener callback\n");
    return NULL;
}
// zxdg_output_v1_listener ...
typedef struct my_zxdg_output_v1_listener_s {
    uintptr_t   logical_position; //vFppii
    uintptr_t   logical_size; //vFppii
    uintptr_t   done; //vFpp
    uintptr_t   name; //vFppp
    uintptr_t   description; //vFppp
} my_zxdg_output_v1_listener_t;
#define GO(A)   \
static my_zxdg_output_v1_listener_t* ref_zxdg_output_v1_listener_##A = NULL;                             \
static void my_zxdg_output_v1_listener_logical_position_##A(void* a, void* b, int32_t c, int32_t d)      \
{                                                                                                        \
    RunFunctionFmt(ref_zxdg_output_v1_listener_##A->logical_position, "ppii", a, b, c, d);               \
}                                                                                                        \
static void my_zxdg_output_v1_listener_logical_size_##A(void* a, void* b, int32_t c, int32_t d)          \
{                                                                                                        \
    RunFunctionFmt(ref_zxdg_output_v1_listener_##A->logical_size, "ppii", a, b, c, d);                   \
}                                                                                                        \
static void my_zxdg_output_v1_listener_done_##A(void* a, void* b)                                        \
{                                                                                                        \
    RunFunctionFmt(ref_zxdg_output_v1_listener_##A->done, "pp", a, b);                                   \
}                                                                                                        \
static void my_zxdg_output_v1_listener_name_##A(void* a, void* b, void* c)                               \
{                                                                                                        \
    RunFunctionFmt(ref_zxdg_output_v1_listener_##A->name, "ppp", a, b, c);                               \
}                                                                                                        \
static void my_zxdg_output_v1_listener_description_##A(void* a, void* b, void* c)                        \
{                                                                                                        \
    RunFunctionFmt(ref_zxdg_output_v1_listener_##A->description, "ppp", a, b, c);                        \
}                                                                                                        \
static my_zxdg_output_v1_listener_t my_zxdg_output_v1_listener_fct_##A = {                               \
    (uintptr_t)my_zxdg_output_v1_listener_logical_position_##A,                                          \
    (uintptr_t)my_zxdg_output_v1_listener_logical_size_##A,                                              \
    (uintptr_t)my_zxdg_output_v1_listener_done_##A,                                                      \
    (uintptr_t)my_zxdg_output_v1_listener_name_##A,                                                      \
    (uintptr_t)my_zxdg_output_v1_listener_description_##A,                                               \
};
SUPER()
#undef GO
static void* find_zxdg_output_v1_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_zxdg_output_v1_listener_##A == fct) return &my_zxdg_output_v1_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_zxdg_output_v1_listener_##A == 0) {ref_zxdg_output_v1_listener_##A = fct; return &my_zxdg_output_v1_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client zxdg_output_v1_listener callback\n");
    return NULL;
}

#undef SUPER

EXPORT int my_wl_proxy_add_listener(x64emu_t* emu, void* proxy, void** l, void* data)
{
    const char* proxy_name = **(const char***)proxy;
    if(!strcmp(proxy_name, "wl_registry")) {
        l = find_wl_registry_listener_Fct(l);
    } else if(!strcmp(proxy_name, "xdg_surface")) {
        l = find_xdg_surface_listener_Fct(l);
    } else if(!strcmp(proxy_name, "xdg_toplevel")) {
        l = find_xdg_toplevel_listener_Fct(l);
    } else if(!strcmp(proxy_name, "xdg_wm_base")) {
        l = find_xdg_wm_base_listener_Fct(l);
    } else if(!strcmp(proxy_name, "wl_shm")) {
        l = find_wl_shm_listener_Fct(l);
    } else if(!strcmp(proxy_name, "wl_output")) {
        l = find_wl_output_listener_Fct(l);
    } else if(!strcmp(proxy_name, "wl_seat")) {
        l = find_wl_seat_listener_Fct(l);
    } else if(!strcmp(proxy_name, "wl_pointer")) {
        l = find_wl_pointer_listener_Fct(l);
    } else if(!strcmp(proxy_name, "wl_keyboard")) {
        l = find_wl_keyboard_listener_Fct(l);
    } else if(!strcmp(proxy_name, "wl_buffer")) {
        l = find_wl_buffer_listener_Fct(l);
    } else if(!strcmp(proxy_name, "zwp_relative_pointer_v1")) {
        l = find_zwp_relative_pointer_v1_listener_Fct(l);
    } else if(!strcmp(proxy_name, "zxdg_output_v1")) {
        l = find_zxdg_output_v1_listener_Fct(l);
    } else
        printf_log(LOG_INFO, "BOX64: Error, Wayland-client, add_listener to %s unknown, will crash soon!\n", proxy_name);
    return my->wl_proxy_add_listener(proxy, l, data);
}

#include "wrappedlib_init.h"
