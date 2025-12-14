#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <stdarg.h>

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
GO(4)   \
GO(5)   \
GO(6)   \
GO(7)   \
GO(8)   \
GO(9)   \

//wl_log_func_t
#ifdef CONVERT_VALIST
#define GO(A)                                                       \
static uintptr_t my_wl_log_func_t_fct_##A = 0;                      \
static void my_wl_log_func_t_##A(void* fmt, x64_va_list_t args)     \
{                                                                   \
    CONVERT_VALIST(args)                                            \
    RunFunction(my_wl_log_func_t_fct_##A, 2, fmt, VARARGS);         \
    }
    #else
#define GO(A)                                                       \
static uintptr_t my_wl_log_func_t_fct_##A = 0;                      \
static void my_wl_log_func_t_##A(void* fmt, x64_va_list_t args)     \
{                                                                   \
    CREATE_VALIST_FROM_VALIST(args, thread_get_emu()->scratch);     \
    RunFunction(my_wl_log_func_t_fct_##A, 2, fmt, VARARGS);         \
}
#endif
SUPER()
#undef GO
static void* find_wl_log_func_t_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_wl_log_func_t_fct_##A == (uintptr_t)fct) return my_wl_log_func_t_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_wl_log_func_t_fct_##A == 0) {my_wl_log_func_t_fct_##A = (uintptr_t)fct; return my_wl_log_func_t_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client wl_log_func_t callback\n");
    return NULL;
}

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
    uintptr_t   enter;  //vFppupp
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
static void my_wl_keyboard_listener_enter_##A(void* a, void* b, uint32_t c, void* d, void* e)                                    \
{                                                                                                                                \
    RunFunctionFmt(ref_wl_keyboard_listener_##A->enter, "ppupp", a, b, c, d, e);                                                 \
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
// wl_data_device_listener ...
typedef struct my_wl_data_device_listener_s {
    uintptr_t   data_offer; //vFppp
    uintptr_t   enter; //vFppupiip
    uintptr_t   leave; //vFpp
    uintptr_t   motion; //vFppuii
    uintptr_t   drop; //vFpp
    uintptr_t   selection; //vFppp
} my_wl_data_device_listener_t;
#define GO(A)   \
static my_wl_data_device_listener_t* ref_wl_data_device_listener_##A = NULL;                                    \
static void my_wl_data_device_listener_data_offer_##A(void* a, void* b, void* c)                                \
{                                                                                                               \
    RunFunctionFmt(ref_wl_data_device_listener_##A->data_offer, "ppp", a, b, c);                                \
}                                                                                                               \
static void my_wl_data_device_listener_enter_##A(void* a, void* b, uint32_t c, void* d, int e, int f, void* g)  \
{                                                                                                               \
    RunFunctionFmt(ref_wl_data_device_listener_##A->enter, "ppupiip", a, b, c, d, e, f, g);                     \
}                                                                                                               \
static void my_wl_data_device_listener_leave_##A(void* a, void* b)                                              \
{                                                                                                               \
    RunFunctionFmt(ref_wl_data_device_listener_##A->leave, "pp", a, b);                                         \
}                                                                                                               \
static void my_wl_data_device_listener_motion_##A(void* a, void* b, uint32_t c, int d, int e)                   \
{                                                                                                               \
    RunFunctionFmt(ref_wl_data_device_listener_##A->motion, "ppuii", a, b, c, d, e);                            \
}                                                                                                               \
static void my_wl_data_device_listener_drop_##A(void* a, void* b)                                               \
{                                                                                                               \
    RunFunctionFmt(ref_wl_data_device_listener_##A->drop, "pp", a, b);                                          \
}                                                                                                               \
static void my_wl_data_device_listener_selection_##A(void* a, void* b, void* c)                                 \
{                                                                                                               \
    RunFunctionFmt(ref_wl_data_device_listener_##A->selection, "ppp", a, b, c);                                 \
}                                                                                                               \
static my_wl_data_device_listener_t my_wl_data_device_listener_fct_##A = {                                      \
    (uintptr_t)my_wl_data_device_listener_data_offer_##A,                                                       \
    (uintptr_t)my_wl_data_device_listener_enter_##A,                                                            \
    (uintptr_t)my_wl_data_device_listener_leave_##A,                                                            \
    (uintptr_t)my_wl_data_device_listener_motion_##A,                                                           \
    (uintptr_t)my_wl_data_device_listener_drop_##A,                                                             \
    (uintptr_t)my_wl_data_device_listener_selection_##A,                                                        \
};
SUPER()
#undef GO
static void* find_wl_data_device_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_wl_data_device_listener_##A == fct) return &my_wl_data_device_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_wl_data_device_listener_##A == 0) {ref_wl_data_device_listener_##A = fct; return &my_wl_data_device_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client wl_data_device_listener callback\n");
    return NULL;
}
// wl_touch_listener ...
typedef struct my_wl_touch_listener_s {
    uintptr_t   down; //vFppuupiii
    uintptr_t   up; //vFppuui
    uintptr_t   motion; //vFppuiii
    uintptr_t   frame; //vFpp
    uintptr_t   cancel; //vFpp
} my_wl_touch_listener_t;
#define GO(A)   \
static my_wl_touch_listener_t* ref_wl_touch_listener_##A = NULL;                                                    \
static void my_wl_touch_listener_down_##A(void* a, void* b, uint32_t c, uint32_t d, void* e, int f, int g, int h)   \
{                                                                                                                   \
    RunFunctionFmt(ref_wl_touch_listener_##A->down, "ppuuppiii", a, b, c, d, e, f, g, h);                           \
}                                                                                                                   \
static void my_wl_touch_listener_up_##A(void* a, void* b, uint32_t c, uint32_t d, int e)                            \
{                                                                                                                   \
    RunFunctionFmt(ref_wl_touch_listener_##A->up, "ppuui", a, b, c, d, e);                                          \
}                                                                                                                   \
static void my_wl_touch_listener_motion_##A(void* a, void* b, uint32_t c, int d, int e, int f)                      \
{                                                                                                                   \
    RunFunctionFmt(ref_wl_touch_listener_##A->motion, "ppuiii", a, b, c, d, e, f);                                  \
}                                                                                                                   \
static void my_wl_touch_listener_frame_##A(void* a, void* b)                                                        \
{                                                                                                                   \
    RunFunctionFmt(ref_wl_touch_listener_##A->frame, "pp", a, b);                                                   \
}                                                                                                                   \
static void my_wl_touch_listener_cancel_##A(void* a, void* b)                                                       \
{                                                                                                                   \
    RunFunctionFmt(ref_wl_touch_listener_##A->cancel, "pp", a, b);                                                  \
}                                                                                                                   \
static my_wl_touch_listener_t my_wl_touch_listener_fct_##A = {                                                      \
    (uintptr_t)my_wl_touch_listener_down_##A,                                                                       \
    (uintptr_t)my_wl_touch_listener_up_##A,                                                                         \
    (uintptr_t)my_wl_touch_listener_motion_##A,                                                                     \
    (uintptr_t)my_wl_touch_listener_frame_##A,                                                                      \
    (uintptr_t)my_wl_touch_listener_cancel_##A,                                                                     \
};
SUPER()
#undef GO
static void* find_wl_touch_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_wl_touch_listener_##A == fct) return &my_wl_touch_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_wl_touch_listener_##A == 0) {ref_wl_touch_listener_##A = fct; return &my_wl_touch_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client wl_touch_listener callback\n");
    return NULL;
}
// zwp_text_input_v3_listener ...
typedef struct my_zwp_text_input_v3_listener_s {
    uintptr_t   enter; //vFppp
    uintptr_t   leave; //vFppp
    uintptr_t   preedit_string; //vFpppii
    uintptr_t   commit_string; //vFppp
    uintptr_t   delete_surrounding_text; //vFpppuu
    uintptr_t   done; //vFppu
} my_zwp_text_input_v3_listener_t;
#define GO(A)   \
static my_zwp_text_input_v3_listener_t* ref_zwp_text_input_v3_listener_##A = NULL;                                          \
static void my_zwp_text_input_v3_listener_enter_##A(void* a, void* b, void* c)                                              \
{                                                                                                                           \
    RunFunctionFmt(ref_zwp_text_input_v3_listener_##A->enter, "ppp", a, b, c);                                              \
}                                                                                                                           \
static void my_zwp_text_input_v3_listener_leave_##A(void* a, void* b, void* c)                                              \
{                                                                                                                           \
    RunFunctionFmt(ref_zwp_text_input_v3_listener_##A->leave, "ppp", a, b, c);                                              \
}                                                                                                                           \
static void my_zwp_text_input_v3_listener_preedit_string_##A(void* a, void* b, void* c, int d, int e)                       \
{                                                                                                                           \
    RunFunctionFmt(ref_zwp_text_input_v3_listener_##A->preedit_string, "pppii", a, b, c, d, e);                             \
}                                                                                                                           \
static void my_zwp_text_input_v3_listener_commit_string_##A(void* a, void* b, void* c)                                      \
{                                                                                                                           \
    RunFunctionFmt(ref_zwp_text_input_v3_listener_##A->commit_string, "ppp", a, b, c);                                      \
}                                                                                                                           \
static void my_zwp_text_input_v3_listener_delete_surrounding_text_##A(void* a, void* b, void* c, uint32_t d, uint32_t e)    \
{                                                                                                                           \
    RunFunctionFmt(ref_zwp_text_input_v3_listener_##A->delete_surrounding_text, "pppuu", a, b, c, d, e);                    \
}                                                                                                                           \
static void my_zwp_text_input_v3_listener_done_##A(void* a, void* b, uint32_t c)                                            \
{                                                                                                                           \
    RunFunctionFmt(ref_zwp_text_input_v3_listener_##A->done, "ppu", a, b, c);                                               \
}                                                                                                                           \
static my_zwp_text_input_v3_listener_t my_zwp_text_input_v3_listener_fct_##A = {                                            \
    (uintptr_t)my_zwp_text_input_v3_listener_enter_##A,                                                                     \
    (uintptr_t)my_zwp_text_input_v3_listener_leave_##A,                                                                     \
    (uintptr_t)my_zwp_text_input_v3_listener_preedit_string_##A,                                                            \
    (uintptr_t)my_zwp_text_input_v3_listener_commit_string_##A,                                                             \
    (uintptr_t)my_zwp_text_input_v3_listener_delete_surrounding_text_##A,                                                   \
    (uintptr_t)my_zwp_text_input_v3_listener_done_##A,                                                                      \
};
SUPER()
#undef GO
static void* find_zwp_text_input_v3_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_zwp_text_input_v3_listener_##A == fct) return &my_zwp_text_input_v3_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_zwp_text_input_v3_listener_##A == 0) {ref_zwp_text_input_v3_listener_##A = fct; return &my_zwp_text_input_v3_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client zwp_text_input_v3_listener callback\n");
    return NULL;
}
// zwp_tablet_seat_v2_listener ...
typedef struct my_zwp_tablet_seat_v2_listener_s {
    uintptr_t   tablet_added; //vFppp
    uintptr_t   tool_added; //vFppp
    uintptr_t   pad_added; //vFppp
} my_zwp_tablet_seat_v2_listener_t;
#define GO(A)   \
static my_zwp_tablet_seat_v2_listener_t* ref_zwp_tablet_seat_v2_listener_##A = NULL;            \
static void my_zwp_tablet_seat_v2_listener_tablet_added_##A(void* a, void* b, void* c)          \
{                                                                                               \
    RunFunctionFmt(ref_zwp_tablet_seat_v2_listener_##A->tablet_added, "ppp", a, b, c);          \
}                                                                                               \
static void my_zwp_tablet_seat_v2_listener_tool_added_##A(void* a, void* b, void* c)            \
{                                                                                               \
    RunFunctionFmt(ref_zwp_tablet_seat_v2_listener_##A->tool_added, "ppp", a, b, c);            \
}                                                                                               \
static void my_zwp_tablet_seat_v2_listener_pad_added_##A(void* a, void* b, void* c)             \
{                                                                                               \
    RunFunctionFmt(ref_zwp_tablet_seat_v2_listener_##A->pad_added, "ppp", a, b, c);             \
}                                                                                               \
static my_zwp_tablet_seat_v2_listener_t my_zwp_tablet_seat_v2_listener_fct_##A = {              \
    (uintptr_t)my_zwp_tablet_seat_v2_listener_tablet_added_##A,                                 \
    (uintptr_t)my_zwp_tablet_seat_v2_listener_tool_added_##A,                                   \
    (uintptr_t)my_zwp_tablet_seat_v2_listener_pad_added_##A,                                    \
};
SUPER()
#undef GO
static void* find_zwp_tablet_seat_v2_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_zwp_tablet_seat_v2_listener_##A == fct) return &my_zwp_tablet_seat_v2_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_zwp_tablet_seat_v2_listener_##A == 0) {ref_zwp_tablet_seat_v2_listener_##A = fct; return &my_zwp_tablet_seat_v2_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client zwp_tablet_seat_v2_listener callback\n");
    return NULL;
}
// xdg_activation_token_v1_listener ...
typedef struct my_xdg_activation_token_v1_listener_s {
    uintptr_t   done; //vFppp
} my_xdg_activation_token_v1_listener_t;
#define GO(A)   \
static my_xdg_activation_token_v1_listener_t* ref_xdg_activation_token_v1_listener_##A = NULL;  \
static void my_xdg_activation_token_v1_listener_done_##A(void* a, void* b, void* c)             \
{                                                                                               \
    RunFunctionFmt(ref_xdg_activation_token_v1_listener_##A->done, "ppp", a, b, c);             \
}                                                                                               \
static my_xdg_activation_token_v1_listener_t my_xdg_activation_token_v1_listener_fct_##A = {    \
    (uintptr_t)my_xdg_activation_token_v1_listener_done_##A,                                    \
};
SUPER()
#undef GO
static void* find_xdg_activation_token_v1_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_xdg_activation_token_v1_listener_##A == fct) return &my_xdg_activation_token_v1_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_xdg_activation_token_v1_listener_##A == 0) {ref_xdg_activation_token_v1_listener_##A = fct; return &my_xdg_activation_token_v1_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client xdg_activation_token_v1_listener callback\n");
    return NULL;
}
// wl_surface_listener ...
typedef struct my_wl_surface_listener_s {
    uintptr_t   enter; //vFppp
    uintptr_t   leave; //vFppp
} my_wl_surface_listener_t;
#define GO(A)   \
static my_wl_surface_listener_t* ref_wl_surface_listener_##A = NULL;        \
static void my_wl_surface_listener_enter_##A(void* a, void* b, void* c)     \
{                                                                           \
    RunFunctionFmt(ref_wl_surface_listener_##A->enter, "ppp", a, b, c);     \
}                                                                           \
static void my_wl_surface_listener_leave_##A(void* a, void* b, void* c)     \
{                                                                           \
    RunFunctionFmt(ref_wl_surface_listener_##A->leave, "ppp", a, b, c);     \
}                                                                           \
static my_wl_surface_listener_t my_wl_surface_listener_fct_##A = {          \
    (uintptr_t)my_wl_surface_listener_enter_##A,                            \
    (uintptr_t)my_wl_surface_listener_leave_##A,                            \
};
SUPER()
#undef GO
static void* find_wl_surface_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_wl_surface_listener_##A == fct) return &my_wl_surface_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_wl_surface_listener_##A == 0) {ref_wl_surface_listener_##A = fct; return &my_wl_surface_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client wl_surface_listener callback\n");
    return NULL;
}
// wl_callback_listener ...
typedef struct my_wl_callback_listener_s {
    uintptr_t   done; //vFppp
} my_wl_callback_listener_t;
#define GO(A)   \
static my_wl_callback_listener_t* ref_wl_callback_listener_##A = NULL;      \
static void my_wl_callback_listener_done_##A(void* a, void* b, void* c)     \
{                                                                           \
    RunFunctionFmt(ref_wl_callback_listener_##A->done, "ppp", a, b, c);     \
}                                                                           \
static my_wl_callback_listener_t my_wl_callback_listener_fct_##A = {        \
    (uintptr_t)my_wl_callback_listener_done_##A,                            \
};
SUPER()
#undef GO
static void* find_wl_callback_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_wl_callback_listener_##A == fct) return &my_wl_callback_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_wl_callback_listener_##A == 0) {ref_wl_callback_listener_##A = fct; return &my_wl_callback_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client wl_callback_listener callback\n");
    return NULL;
}
// wp_fractional_scale_v1_listener ...
typedef struct my_wp_fractional_scale_v1_listener_s {
    uintptr_t   prefered_scale; //vFppu
} my_wp_fractional_scale_v1_listener_t;
#define GO(A)   \
static my_wp_fractional_scale_v1_listener_t* ref_wp_fractional_scale_v1_listener_##A = NULL;    \
static void my_wp_fractional_scale_v1_listener_prefered_scale_##A(void* a, void* b, uint32_t c) \
{                                                                                               \
    RunFunctionFmt(ref_wp_fractional_scale_v1_listener_##A->prefered_scale, "ppu", a, b, c);    \
}                                                                                               \
static my_wp_fractional_scale_v1_listener_t my_wp_fractional_scale_v1_listener_fct_##A = {      \
    (uintptr_t)my_wp_fractional_scale_v1_listener_prefered_scale_##A,                           \
};
SUPER()
#undef GO
static void* find_wp_fractional_scale_v1_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_wp_fractional_scale_v1_listener_##A == fct) return &my_wp_fractional_scale_v1_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_wp_fractional_scale_v1_listener_##A == 0) {ref_wp_fractional_scale_v1_listener_##A = fct; return &my_wp_fractional_scale_v1_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client wp_fractional_scale_v1_listener callback\n");
    return NULL;
}
// zwp_primary_selection_device_v1_listener ...
typedef struct my_zwp_primary_selection_device_v1_listener_s {
    uintptr_t   data_offer; //vFppp
    uintptr_t   selection; //vFppp
} my_zwp_primary_selection_device_v1_listener_t;
#define GO(A)   \
static my_zwp_primary_selection_device_v1_listener_t* ref_zwp_primary_selection_device_v1_listener_##A = NULL;  \
static void my_zwp_primary_selection_device_v1_listener_data_offer_##A(void* a, void* b, void* c)               \
{                                                                                                               \
    RunFunctionFmt(ref_zwp_primary_selection_device_v1_listener_##A->data_offer, "ppp", a, b, c);               \
}                                                                                                               \
static void my_zwp_primary_selection_device_v1_listener_selection_##A(void* a, void* b, void* c)                \
{                                                                                                               \
    RunFunctionFmt(ref_zwp_primary_selection_device_v1_listener_##A->selection, "ppp", a, b, c);                \
}                                                                                                               \
static my_zwp_primary_selection_device_v1_listener_t my_zwp_primary_selection_device_v1_listener_fct_##A = {    \
    (uintptr_t)my_zwp_primary_selection_device_v1_listener_data_offer_##A,                                      \
    (uintptr_t)my_zwp_primary_selection_device_v1_listener_selection_##A,                                       \
};
SUPER()
#undef GO
static void* find_zwp_primary_selection_device_v1_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_zwp_primary_selection_device_v1_listener_##A == fct) return &my_zwp_primary_selection_device_v1_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_zwp_primary_selection_device_v1_listener_##A == 0) {ref_zwp_primary_selection_device_v1_listener_##A = fct; return &my_zwp_primary_selection_device_v1_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client zwp_primary_selection_device_v1_listener callback\n");
    return NULL;
}
// wl_data_offer_listener ...
typedef struct my_wl_data_offer_listener_s {
    uintptr_t   offer; //vFppp
    uintptr_t   source_actions; //vFppu
    uintptr_t   action; //vFppu
} my_wl_data_offer_listener_t;
#define GO(A)   \
static my_wl_data_offer_listener_t* ref_wl_data_offer_listener_##A = NULL;              \
static void my_wl_data_offer_listener_offer_##A(void* a, void* b, void* c)              \
{                                                                                       \
    RunFunctionFmt(ref_wl_data_offer_listener_##A->offer, "ppp", a, b, c);              \
}                                                                                       \
static void my_wl_data_offer_listener_source_actions_##A(void* a, void* b, uint32_t c)  \
{                                                                                       \
    RunFunctionFmt(ref_wl_data_offer_listener_##A->source_actions, "ppu", a, b, c);     \
}                                                                                       \
static void my_wl_data_offer_listener_action_##A(void* a, void* b, uint32_t c)          \
{                                                                                       \
    RunFunctionFmt(ref_wl_data_offer_listener_##A->action, "ppu", a, b, c);             \
}                                                                                       \
static my_wl_data_offer_listener_t my_wl_data_offer_listener_fct_##A = {                \
    (uintptr_t)my_wl_data_offer_listener_offer_##A,                                     \
    (uintptr_t)my_wl_data_offer_listener_source_actions_##A,                            \
    (uintptr_t)my_wl_data_offer_listener_action_##A,                                    \
};
SUPER()
#undef GO
static void* find_wl_data_offer_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_wl_data_offer_listener_##A == fct) return &my_wl_data_offer_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_wl_data_offer_listener_##A == 0) {ref_wl_data_offer_listener_##A = fct; return &my_wl_data_offer_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client wl_data_offer_listener callback\n");
    return NULL;
}
// zwp_primary_selection_offer_v1_listener ...
typedef struct my_zwp_primary_selection_offer_v1_listener_s {
    uintptr_t   primary_offer; //vFppp
} my_zwp_primary_selection_offer_v1_listener_t;
#define GO(A)   \
static my_zwp_primary_selection_offer_v1_listener_t* ref_zwp_primary_selection_offer_v1_listener_##A = NULL;    \
static void my_zwp_primary_selection_offer_v1_listener_primary_offer_##A(void* a, void* b, void* c)             \
{                                                                                                               \
    RunFunctionFmt(ref_zwp_primary_selection_offer_v1_listener_##A->primary_offer, "ppp", a, b, c);             \
}                                                                                                               \
static my_zwp_primary_selection_offer_v1_listener_t my_zwp_primary_selection_offer_v1_listener_fct_##A = {      \
    (uintptr_t)my_zwp_primary_selection_offer_v1_listener_primary_offer_##A,                                    \
};
SUPER()
#undef GO
static void* find_zwp_primary_selection_offer_v1_listener_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_zwp_primary_selection_offer_v1_listener_##A == fct) return &my_zwp_primary_selection_offer_v1_listener_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_zwp_primary_selection_offer_v1_listener_##A == 0) {ref_zwp_primary_selection_offer_v1_listener_##A = fct; return &my_zwp_primary_selection_offer_v1_listener_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client zwp_primary_selection_offer_v1_listener callback\n");
    return NULL;
}
// wp_color_management_output_v1 ...
typedef struct my_wp_color_management_output_v1_listener_s {
    uintptr_t image_description_changed; // vFpp
} my_wp_color_management_output_v1_listener_t;
#define GO(A)                                                                                                  \
    static my_wp_color_management_output_v1_listener_t* ref_wp_color_management_output_v1_listener_##A = NULL; \
    static void my_wp_color_management_output_v1_listener_image_description_changed_##A(void* a, void* b)      \
    {                                                                                                          \
        RunFunctionFmt(ref_wp_color_management_output_v1_listener_##A->image_description_changed, "pp", a, b); \
    }                                                                                                          \
    static my_wp_color_management_output_v1_listener_t my_wp_color_management_output_v1_listener_fct_##A = {   \
        (uintptr_t)my_wp_color_management_output_v1_listener_image_description_changed_##A,                    \
    };
SUPER()
#undef GO
static void* find_wp_color_management_output_v1_listener_Fct(void* fct)
{
    if (!fct) return fct;
#define GO(A) \
    if (ref_wp_color_management_output_v1_listener_##A == fct) return &my_wp_color_management_output_v1_listener_fct_##A;
    SUPER()
#undef GO
#define GO(A)                                                      \
    if (ref_wp_color_management_output_v1_listener_##A == 0) {     \
        ref_wp_color_management_output_v1_listener_##A = fct;      \
        return &my_wp_color_management_output_v1_listener_fct_##A; \
    }
    SUPER()
#undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client wp_color_management_output_v1_listener callback\n");
    return NULL;
}
// wp_image_description_info_v1 ...
typedef struct my_wp_image_description_info_v1_listener_s {
    uintptr_t done;             // vFpp
    uintptr_t icc_file;         // vFppiu
    uintptr_t primaries;        // vFppiiiiiiii
    uintptr_t primaries_named;  // vFppu
    uintptr_t tf_power;         // vFppu
    uintptr_t tf_named;         // vFppu
    uintptr_t luminances;       // vFppuuu
    uintptr_t target_primaries; // vFppiiiiiiii
    uintptr_t target_luminance; // vFppuu
    uintptr_t target_max_cll;   // vFppu
    uintptr_t target_max_fall;  // vFppu
} my_wp_image_description_info_v1_listener_t;
#define GO(A)                                                                                                                 \
    static my_wp_image_description_info_v1_listener_t* ref_wp_image_description_info_v1_listener_##A = NULL;                  \
    static void my_wp_image_description_info_v1_listener_done_##A(void* a, void* b)                                           \
    {                                                                                                                         \
        RunFunctionFmt(ref_wp_image_description_info_v1_listener_##A->done, "pp", a, b);                                      \
    }                                                                                                                         \
    static void my_wp_image_description_info_v1_listener_icc_file_##A(void* a, void* b, int32_t c, uint32_t d)                \
    {                                                                                                                         \
        RunFunctionFmt(ref_wp_image_description_info_v1_listener_##A->icc_file, "ppiu", a, b, c, d);                          \
    }                                                                                                                         \
    static void my_wp_image_description_info_v1_listener_primaries_##A(void* a, void* b,                                      \
        int32_t i1, int32_t i2, int32_t i3, int32_t i4, int32_t i5, int32_t i6, int32_t i7, int32_t i8)                       \
    {                                                                                                                         \
        RunFunctionFmt(ref_wp_image_description_info_v1_listener_##A->primaries, "ppiiiiiiii", a, b,                          \
            i1, i2, i3, i4, i5, i6, i7, i8);                                                                                  \
    }                                                                                                                         \
    static void my_wp_image_description_info_v1_listener_primaries_named_##A(void* a, void* b, uint32_t c)                    \
    {                                                                                                                         \
        RunFunctionFmt(ref_wp_image_description_info_v1_listener_##A->primaries_named, "ppu", a, b, c);                       \
    }                                                                                                                         \
    static void my_wp_image_description_info_v1_listener_tf_power_##A(void* a, void* b, uint32_t c)                           \
    {                                                                                                                         \
        RunFunctionFmt(ref_wp_image_description_info_v1_listener_##A->tf_power, "ppu", a, b, c);                              \
    }                                                                                                                         \
    static void my_wp_image_description_info_v1_listener_tf_named_##A(void* a, void* b, uint32_t c)                           \
    {                                                                                                                         \
        RunFunctionFmt(ref_wp_image_description_info_v1_listener_##A->tf_named, "ppu", a, b, c);                              \
    }                                                                                                                         \
    static void my_wp_image_description_info_v1_listener_luminances_##A(void* a, void* b, uint32_t c, uint32_t d, uint32_t e) \
    {                                                                                                                         \
        RunFunctionFmt(ref_wp_image_description_info_v1_listener_##A->luminances, "ppuuu", a, b, c, d, e);                    \
    }                                                                                                                         \
    static void my_wp_image_description_info_v1_listener_target_primaries_##A(void* a, void* b,                               \
        int32_t i1, int32_t i2, int32_t i3, int32_t i4, int32_t i5, int32_t i6, int32_t i7, int32_t i8)                       \
    {                                                                                                                         \
        RunFunctionFmt(ref_wp_image_description_info_v1_listener_##A->target_primaries, "ppiiiiiiii", a, b,                   \
            i1, i2, i3, i4, i5, i6, i7, i8);                                                                                  \
    }                                                                                                                         \
    static void my_wp_image_description_info_v1_listener_target_luminance_##A(void* a, void* b, uint32_t c, uint32_t d)       \
    {                                                                                                                         \
        RunFunctionFmt(ref_wp_image_description_info_v1_listener_##A->target_luminance, "ppuu", a, b, c, d);                  \
    }                                                                                                                         \
    static void my_wp_image_description_info_v1_listener_target_max_cll_##A(void* a, void* b, uint32_t c)                     \
    {                                                                                                                         \
        RunFunctionFmt(ref_wp_image_description_info_v1_listener_##A->target_max_cll, "ppu", a, b, c);                        \
    }                                                                                                                         \
    static void my_wp_image_description_info_v1_listener_target_max_fall_##A(void* a, void* b, uint32_t c)                    \
    {                                                                                                                         \
        RunFunctionFmt(ref_wp_image_description_info_v1_listener_##A->target_max_fall, "ppu", a, b, c);                       \
    }                                                                                                                         \
    static my_wp_image_description_info_v1_listener_t my_wp_image_description_info_v1_listener_fct_##A = {                    \
        (uintptr_t)my_wp_image_description_info_v1_listener_done_##A,                                                         \
        (uintptr_t)my_wp_image_description_info_v1_listener_icc_file_##A,                                                     \
        (uintptr_t)my_wp_image_description_info_v1_listener_primaries_##A,                                                    \
        (uintptr_t)my_wp_image_description_info_v1_listener_primaries_named_##A,                                              \
        (uintptr_t)my_wp_image_description_info_v1_listener_tf_power_##A,                                                     \
        (uintptr_t)my_wp_image_description_info_v1_listener_tf_named_##A,                                                     \
        (uintptr_t)my_wp_image_description_info_v1_listener_luminances_##A,                                                   \
        (uintptr_t)my_wp_image_description_info_v1_listener_target_primaries_##A,                                             \
        (uintptr_t)my_wp_image_description_info_v1_listener_target_luminance_##A,                                             \
        (uintptr_t)my_wp_image_description_info_v1_listener_target_max_cll_##A,                                               \
        (uintptr_t)my_wp_image_description_info_v1_listener_target_max_fall_##A,                                              \
    };
SUPER()
#undef GO
static void* find_wp_image_description_info_v1_listener_Fct(void* fct)
{
    if (!fct) return fct;
#define GO(A) \
    if (ref_wp_image_description_info_v1_listener_##A == fct) return &my_wp_image_description_info_v1_listener_fct_##A;
    SUPER()
#undef GO
#define GO(A)                                                     \
    if (ref_wp_image_description_info_v1_listener_##A == 0) {     \
        ref_wp_image_description_info_v1_listener_##A = fct;      \
        return &my_wp_image_description_info_v1_listener_fct_##A; \
    }
    SUPER()
#undef GO
    printf_log(LOG_NONE, "Warning, no more slot for wayland-client wp_image_description_info_v1_listener callback\n");
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
    } else if(!strcmp(proxy_name, "wl_data_device")) {
        l = find_wl_data_device_listener_Fct(l);
    } else if(!strcmp(proxy_name, "wl_touch")) {
        l = find_wl_touch_listener_Fct(l);
    } else if(!strcmp(proxy_name, "zwp_text_input_v3")) {
        l = find_zwp_text_input_v3_listener_Fct(l);
    } else if(!strcmp(proxy_name, "zwp_tablet_seat_v2")) {
        l = find_zwp_tablet_seat_v2_listener_Fct(l);
    } else if(!strcmp(proxy_name, "xdg_activation_token_v1")) {
        l = find_xdg_activation_token_v1_listener_Fct(l);
    } else if(!strcmp(proxy_name, "wl_surface")) {
        l = find_wl_surface_listener_Fct(l);
    } else if(!strcmp(proxy_name, "wl_callback")) {
        l = find_wl_callback_listener_Fct(l);
    } else if(!strcmp(proxy_name, "wp_fractional_scale_v1")) {
        l = find_wp_fractional_scale_v1_listener_Fct(l);
    } else if(!strcmp(proxy_name, "zwp_primary_selection_device_v1")) {
        l = find_zwp_primary_selection_device_v1_listener_Fct(l);
    } else if(!strcmp(proxy_name, "wl_data_offer")) {
        l = find_wl_data_offer_listener_Fct(l);
    } else if(!strcmp(proxy_name, "zwp_primary_selection_offer_v1")) {
        l = find_zwp_primary_selection_offer_v1_listener_Fct(l);
    } else if (!strcmp(proxy_name, "wp_color_management_output_v1")) {
        l = find_wp_color_management_output_v1_listener_Fct(l);
    } else if (!strcmp(proxy_name, "wp_image_description_info_v1")) {
        l = find_wp_image_description_info_v1_listener_Fct(l);
    } else
        printf_log(LOG_INFO, "Error, Wayland-client, add_listener to %s unknown, will crash soon!\n", proxy_name);
    return my->wl_proxy_add_listener(proxy, l, data);
}

EXPORT void my_wl_log_set_handler_client(x64emu_t* emu, void* f)
{
    my->wl_log_set_handler_client(find_wl_log_func_t_Fct(f));
}

#include "wrappedlib_init.h"
