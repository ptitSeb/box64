#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "callback.h"
#include "box64context.h"
#include "librarian.h"
#include "gtkclass.h"
#include "library.h"
#include "custommem.h"
#include "khash.h"

int is_gtk3 = 0;

static bridge_t*        my_bridge           = NULL;
static const char* (*g_type_name)(size_t)   = NULL;
static void* (*g_type_class_peek)(size_t)   = NULL;
#define GTKCLASS(A) static size_t my_##A    = (size_t)-1;
GTKCLASSES()
#undef GTKCLASS

KHASH_SET_INIT_INT(signalmap)
static kh_signalmap_t *my_signalmap = NULL;

typedef struct sigoffset_s {
    uint32_t offset;
    int     n;
} sigoffset_t;
typedef struct sigoffset_array_s {
    sigoffset_t *a;
    int     cap;
    int     sz;
} sigoffset_array_t;

KHASH_MAP_INIT_INT64(sigoffset, sigoffset_array_t)
static kh_sigoffset_t *my_sigoffset = NULL;

// ---- Defining the multiple functions now -----
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)   \
GO(5)   \
GO(6)   \
GO(7)	\
GO(8)	\
GO(9)	\
GO(10)	\
GO(11)	\
GO(12)

#define WRAPPED(A, NAME, RET, DEF, FMT, ...)  \
static uintptr_t my_##NAME##_fct_##A = 0;   \
static RET my_##NAME##_##A DEF              \
{                                           \
    printf_log(LOG_DEBUG, "Calling " #NAME "_" #A " wrapper\n");             \
    return (RET)RunFunctionFmt(my_##NAME##_fct_##A, FMT, __VA_ARGS__);\
}

#define FIND(A, NAME) \
static void* find_##NAME##_##A(void* fct)                           \
{                                                                   \
    if(!fct) return fct;                                            \
    void* tmp = GetNativeFnc((uintptr_t)fct);                       \
    if(tmp) return tmp;                                             \
    if(my_##NAME##_##A##_fct_0 == (uintptr_t)fct) return my_##NAME##_##A##_0;   \
    if(my_##NAME##_##A##_fct_1 == (uintptr_t)fct) return my_##NAME##_##A##_1;   \
    if(my_##NAME##_##A##_fct_2 == (uintptr_t)fct) return my_##NAME##_##A##_2;   \
    if(my_##NAME##_##A##_fct_3 == (uintptr_t)fct) return my_##NAME##_##A##_3;   \
    if(my_##NAME##_##A##_fct_4 == (uintptr_t)fct) return my_##NAME##_##A##_4;   \
    if(my_##NAME##_##A##_fct_5 == (uintptr_t)fct) return my_##NAME##_##A##_5;   \
    if(my_##NAME##_##A##_fct_6 == (uintptr_t)fct) return my_##NAME##_##A##_6;   \
    if(my_##NAME##_##A##_fct_7 == (uintptr_t)fct) return my_##NAME##_##A##_7;   \
    if(my_##NAME##_##A##_fct_0 == 0) {my_##NAME##_##A##_fct_0 = (uintptr_t)fct; return my_##NAME##_##A##_0; } \
    if(my_##NAME##_##A##_fct_1 == 0) {my_##NAME##_##A##_fct_1 = (uintptr_t)fct; return my_##NAME##_##A##_1; } \
    if(my_##NAME##_##A##_fct_2 == 0) {my_##NAME##_##A##_fct_2 = (uintptr_t)fct; return my_##NAME##_##A##_2; } \
    if(my_##NAME##_##A##_fct_3 == 0) {my_##NAME##_##A##_fct_3 = (uintptr_t)fct; return my_##NAME##_##A##_3; } \
    if(my_##NAME##_##A##_fct_4 == 0) {my_##NAME##_##A##_fct_4 = (uintptr_t)fct; return my_##NAME##_##A##_4; } \
    if(my_##NAME##_##A##_fct_5 == 0) {my_##NAME##_##A##_fct_5 = (uintptr_t)fct; return my_##NAME##_##A##_5; } \
    if(my_##NAME##_##A##_fct_6 == 0) {my_##NAME##_##A##_fct_6 = (uintptr_t)fct; return my_##NAME##_##A##_6; } \
    if(my_##NAME##_##A##_fct_7 == 0) {my_##NAME##_##A##_fct_7 = (uintptr_t)fct; return my_##NAME##_##A##_7; } \
    printf_log(LOG_NONE, "Warning, no more slot for " #NAME " gtkclass callback\n");    \
    return NULL;    \
}

#define REVERSE(A, NAME)   \
static void* reverse_##NAME##_##A(wrapper_t W, void* fct)                       \
{                                                                               \
    if(!fct) return fct;                                                        \
    if((void*)my_##NAME##_##A##_0 == fct) return (void*)my_##NAME##_##A##_fct_0;\
    if((void*)my_##NAME##_##A##_1 == fct) return (void*)my_##NAME##_##A##_fct_1;\
    if((void*)my_##NAME##_##A##_2 == fct) return (void*)my_##NAME##_##A##_fct_2;\
    if((void*)my_##NAME##_##A##_3 == fct) return (void*)my_##NAME##_##A##_fct_3;\
    if((void*)my_##NAME##_##A##_4 == fct) return (void*)my_##NAME##_##A##_fct_4;\
    if((void*)my_##NAME##_##A##_5 == fct) return (void*)my_##NAME##_##A##_fct_5;\
    if((void*)my_##NAME##_##A##_6 == fct) return (void*)my_##NAME##_##A##_fct_6;\
    if((void*)my_##NAME##_##A##_7 == fct) return (void*)my_##NAME##_##A##_fct_7;\
    Dl_info info;                                                               \
    if(dladdr(fct, &info))                                                      \
        return (void*)AddCheckBridge(my_bridge, W, fct, 0, NULL);               \
    return fct;                                                                 \
}

#define AUTOBRIDGE(A, NAME)   \
static void autobridge_##NAME##_##A(wrapper_t W, void* fct)         \
{                                                                   \
    if(!fct)                                                        \
        return;                                                     \
    Dl_info info;                                                   \
    if(dladdr(fct, &info))                                          \
        AddAutomaticBridge(thread_get_emu(), my_bridge, W, fct, 0, #NAME "_" #A); \
}

#define WRAPPER(A, NAME, RET, DEF, FMT, ...)        \
WRAPPED(0, NAME##_##A, RET, DEF, FMT, __VA_ARGS__)  \
WRAPPED(1, NAME##_##A, RET, DEF, FMT, __VA_ARGS__)  \
WRAPPED(2, NAME##_##A, RET, DEF, FMT, __VA_ARGS__)  \
WRAPPED(3, NAME##_##A, RET, DEF, FMT, __VA_ARGS__)  \
WRAPPED(4, NAME##_##A, RET, DEF, FMT, __VA_ARGS__)  \
WRAPPED(5, NAME##_##A, RET, DEF, FMT, __VA_ARGS__)  \
WRAPPED(6, NAME##_##A, RET, DEF, FMT, __VA_ARGS__)  \
WRAPPED(7, NAME##_##A, RET, DEF, FMT, __VA_ARGS__)  \
FIND(A, NAME)                                       \
REVERSE(A, NAME)                                    \
AUTOBRIDGE(A, NAME)

// ----- GObjectClass ------
// wrapper x64 -> natives of callbacks
WRAPPER(GObject, constructor, void*, (size_t type, uint32_t n_construct_properties, void* construct_properties), "Lup", type, n_construct_properties, construct_properties);
WRAPPER(GObject, set_property, void, (void* object, uint32_t property_id, void* value, void* pspec), "pupp", object, property_id, value, pspec);
WRAPPER(GObject, get_property, void, (void* object, uint32_t property_id, void* value, void* pspec), "pupp", object, property_id, value, pspec);
WRAPPER(GObject, dispose, void, (void* object), "p", object);
WRAPPER(GObject, finalize, void, (void* object), "p", object);
WRAPPER(GObject, dispatch_properties_changed, void*, (size_t type, uint32_t n_pspecs, void* pspecs), "Lup", type, n_pspecs, pspecs);
WRAPPER(GObject, notify, void*, (size_t type, void* pspecs), "Lp", type, pspecs);
WRAPPER(GObject, constructed, void, (void* object), "p", object);

#define SUPERGO() \
    GO(constructor, pFLup);                 \
    GO(set_property, vFpupp);               \
    GO(get_property, vFpupp);               \
    GO(dispose, vFp);                       \
    GO(finalize, vFp);                      \
    GO(dispatch_properties_changed, vFLup); \
    GO(notify, vFpp);                       \
    GO(constructed, vFp);

// wrap (so bridge all calls, just in case)
static void wrapGObjectClass(my_GObjectClass_t* class)
{
    #define GO(A, W) class->A = reverse_##A##_GObject (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGObjectClass(my_GObjectClass_t* class)
{
    #define GO(A, W)   class->A = find_##A##_GObject (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGObjectClass(my_GObjectClass_t* class)
{
    #define GO(A, W) autobridge_##A##_GObject (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

// ----- GInitiallyUnownedClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GInitiallyUnowned, constructor, void*, (size_t type, uint32_t n_construct_properties, void* construct_properties), "Lup", type, n_construct_properties, construct_properties);
WRAPPER(GInitiallyUnowned, set_property, void, (void* object, uint32_t property_id, void* value, void* pspec), "pupp", object, property_id, value, pspec);
WRAPPER(GInitiallyUnowned, get_property, void, (void* object, uint32_t property_id, void* value, void* pspec), "pupp", object, property_id, value, pspec);
WRAPPER(GInitiallyUnowned, dispose, void, (void* object), "p", object);
WRAPPER(GInitiallyUnowned, finalize, void, (void* object), "p", object);
WRAPPER(GInitiallyUnowned, dispatch_properties_changed, void*, (size_t type, uint32_t n_pspecs, void* pspecs), "Lup", type, n_pspecs, pspecs);
WRAPPER(GInitiallyUnowned, notify, void*, (size_t type, void* pspecs), "Lp", type, pspecs);
WRAPPER(GInitiallyUnowned, constructed, void, (void* object), "p", object);

#define SUPERGO() \
    GO(constructor, pFLup);                 \
    GO(set_property, vFpupp);               \
    GO(get_property, vFpupp);               \
    GO(dispose, vFp);                       \
    GO(finalize, vFp);                      \
    GO(dispatch_properties_changed, vFLup); \
    GO(notify, vFLp);                       \
    GO(constructed, vFp);

// wrap (so bridge all calls, just in case)
static void wrapGInitiallyUnownedClass(my_GInitiallyUnownedClass_t* class)
{
    #define GO(A, W) class->A = reverse_##A##_GInitiallyUnowned (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGInitiallyUnownedClass(my_GInitiallyUnownedClass_t* class)
{
    #define GO(A, W)   class->A = find_##A##_GInitiallyUnowned (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGInitiallyUnownedClass(my_GInitiallyUnownedClass_t* class)
{
    #define GO(A, W) autobridge_##A##_GInitiallyUnowned (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

// ----- GamesScoresImporterClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GamesScoresImporter, importOldScores, void, (void *self, void* context, void* new_scores_dir, void *error), "pppp", self, context, new_scores_dir, error);

#define SUPERGO() \
    GO(importOldScores, vFpppp);

// wrap (so bridge all calls, just in case)
static void wrapGamesScoresImporterClass(my_GamesScoresImporterClass_t* class)
{
    wrapGObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GamesScoresImporter (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGamesScoresImporterClass(my_GamesScoresImporterClass_t* class)
{
    unwrapGObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GamesScoresImporter (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGamesScoresImporterClass(my_GamesScoresImporterClass_t* class)
{
    bridgeGObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GamesScoresImporter (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

// ----- GApplicationClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GApplication, startup, void, (void* application), "p", application);
WRAPPER(GApplication, activate, void, (void* application), "p", application);
WRAPPER(GApplication, open, void, (void* application, void* files, int n_files, void* hint), "ppip", application, files, n_files, hint);
WRAPPER(GApplication, command_line, void, (void* application, void* command_line), "pp", application, command_line);
WRAPPER(GApplication, local_command_line, void, (void* application, void* arguments, void* exit_status), "ppp", application, arguments, exit_status);
WRAPPER(GApplication, before_emit, void*, (void* application, void* platform_data), "pp", application, platform_data);
WRAPPER(GApplication, after_emit, void, (void* application, void* platform_data), "pp", application, platform_data);
WRAPPER(GApplication, add_platform_data, void, (void* application, void* builder), "pp", application, builder);
WRAPPER(GApplication, quit_mainloop, void, (void* application), "p", application);
WRAPPER(GApplication, run_mainloop, void, (void* application), "p", application);
WRAPPER(GApplication, shutdown, void, (void* application), "p", application);
WRAPPER(GApplication, dbus_register, void, (void* application, void* connection, void* object_path, void* error), "pppp", application, connection, object_path, error);
WRAPPER(GApplication, dbus_unregister, void, (void* application, void* connection, void* object_path), "ppp", application, connection, object_path);
WRAPPER(GApplication, handle_local_options, void, (void* application, void* options), "pp", application, options);
WRAPPER(GApplication, name_lost, void, (void* application), "p", application);

#define SUPERGO()                     \
    GO(startup, vFp);                 \
    GO(activate, vFp);                \
    GO(open, vFppip);                 \
    GO(command_line, vFpp);           \
    GO(local_command_line, vFppp);    \
    GO(before_emit, vFpp);            \
    GO(after_emit, vFpp);             \
    GO(add_platform_data, vFpp);      \
    GO(quit_mainloop, vFp);           \
    GO(run_mainloop, vFp);            \
    GO(shutdown, vFp);                \
    GO(dbus_register, vFpppp);        \
    GO(dbus_unregister, vFppp);       \
    GO(handle_local_options, vFpp);   \
    GO(name_lost, vFp);

// wrap (so bridge all calls, just in case)
static void wrapGApplicationClass(my_GApplicationClass_t* class)
{
    #define GO(A, W) class->A = reverse_##A##_GApplication (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGApplicationClass(my_GApplicationClass_t* class)
{
    #define GO(A, W)   class->A = find_##A##_GApplication (class->A)   //SUPERGO() defined but not used
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGApplicationClass(my_GApplicationClass_t* class)
{
    #define GO(A, W) autobridge_##A##_GApplication (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkApplicationClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkApplication, window_added, void, (void* application, void* window), "pp", application, window);
WRAPPER(GtkApplication, window_removed, void, (void* application, void* window), "pp", application, window);

#define SUPERGO() \
    GO(window_added, pFpp);   \
    GO(window_removed, vFpp);

static void wrapGtkApplicationClass(my_GtkApplicationClass_t* class)
{
    wrapGApplicationClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkApplication (W, class->A)
    SUPERGO()
    #undef GO
}

static void unwrapGtkApplicationClass(my_GtkApplicationClass_t* class)
{
    unwrapGApplicationClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkApplication (class->A)
    SUPERGO()
    #undef GO
}

static void bridgeGtkApplicationClass(my_GtkApplicationClass_t* class)
{
    bridgeGApplicationClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkApplication (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkObjectClass ------
// wrapper x64 -> natives of callbacks
WRAPPER(GtkObject, set_arg, void, (void* object, void* arg, uint32_t arg_id), "ppu", object, arg, arg_id);
WRAPPER(GtkObject, get_arg, void, (void* object, void* arg, uint32_t arg_id), "ppu", object, arg, arg_id);
WRAPPER(GtkObject, destroy, void, (void* object), "p", object);

#define SUPERGO() \
    GO(set_arg, vFppu); \
    GO(get_arg, vFppu); \
    GO(destroy, vFp);
// wrap (so bridge all calls, just in case)
static void wrapGtkObjectClass(my_GtkObjectClass_t* class)
{
    wrapGObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkObject (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkObjectClass(my_GtkObjectClass_t* class)
{
    unwrapGObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkObject (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkObjectClass(my_GtkObjectClass_t* class)
{
    bridgeGObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkObject (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkWidget2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkWidget2, dispatch_child_properties_changed, void, (void* widget, uint32_t n_pspecs, void* pspecs), "pup", widget, n_pspecs, pspecs);
WRAPPER(GtkWidget2, show,              void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, show_all,          void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, hide,              void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, hide_all,          void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, map,               void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, unmap,             void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, realize,           void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, unrealize,         void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, size_request,      void, (void* widget, void* requisition), "pp", widget, requisition);
WRAPPER(GtkWidget2, size_allocate,     void, (void* widget, void* allocation), "pp", widget, allocation);
WRAPPER(GtkWidget2, state_changed,     void, (void* widget, int previous_state), "pi", widget, previous_state);
WRAPPER(GtkWidget2, parent_set,        void, (void* widget, void* previous_parent), "pp", widget, previous_parent);
WRAPPER(GtkWidget2, hierarchy_changed, void, (void* widget, void* previous_toplevel), "pp", widget, previous_toplevel);
WRAPPER(GtkWidget2, style_set,         void, (void* widget, void* previous_style), "pp", widget, previous_style);
WRAPPER(GtkWidget2, direction_changed, void, (void* widget, int previous_direction), "pi", widget, previous_direction);
WRAPPER(GtkWidget2, grab_notify,       void, (void* widget, int was_grabbed), "pi", widget, was_grabbed);
WRAPPER(GtkWidget2, child_notify,      void, (void* widget, void* pspec), "pp", widget, pspec);
WRAPPER(GtkWidget2, mnemonic_activate, int, (void* widget, int group_cycling), "pi", widget, group_cycling);
WRAPPER(GtkWidget2, grab_focus,        void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, focus,             int, (void* widget, int direction), "pi", widget, direction);
WRAPPER(GtkWidget2, event,             int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, button_press_event,int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, button_release_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, scroll_event,      int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, motion_notify_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, delete_event,       int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, destroy_event,      int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, expose_event,       int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, key_press_event,    int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, key_release_event,  int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, enter_notify_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, leave_notify_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, configure_event,    int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, focus_in_event,     int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, focus_out_event,    int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, map_event,          int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, unmap_event,        int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, property_notify_event,  int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, selection_clear_event,  int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, selection_request_event,int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, selection_notify_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, proximity_in_event,  int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, proximity_out_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, visibility_notify_event, int, (void* widget, void* event), "p", widget, event);
WRAPPER(GtkWidget2, client_event,        int, (void* widget, void* event), "p", widget, event);
WRAPPER(GtkWidget2, no_expose_event,     int, (void* widget, void* event), "p", widget, event);
WRAPPER(GtkWidget2, window_state_event,  int, (void* widget, void* event), "p", widget, event);
WRAPPER(GtkWidget2, selection_get,       void, (void* widget, void* selection_data, uint32_t info, uint32_t time_), "ppuu", widget, selection_data, info, time_);
WRAPPER(GtkWidget2, selection_received,  void, (void* widget, void* selection_data, uint32_t time_), "ppu", widget, selection_data, time_);
WRAPPER(GtkWidget2, drag_begin,          void, (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget2, drag_end,            void, (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget2, drag_data_get,       void, (void* widget, void* context, void* selection_data, uint32_t info, uint32_t time_), "pppuu", widget, context, selection_data, info, time_);
WRAPPER(GtkWidget2, drag_data_delete,    void, (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget2, drag_leave,          void, (void* widget, void* context, uint32_t time_), "ppu", widget, context, time_);
WRAPPER(GtkWidget2, drag_motion,         int, (void* widget, void* context, int32_t x, int32_t y, uint32_t time_), "ppiiu", widget, context, x, y, time_);
WRAPPER(GtkWidget2, drag_drop,           int, (void* widget, void* context, int32_t x, int32_t y, uint32_t time_), "ppiiu", widget, context, x, y, time_);
WRAPPER(GtkWidget2, drag_data_received,  void, (void* widget, void* context, int32_t x, int32_t y, void* selection_data, uint32_t info, uint32_t time_), "ppiipuu", widget, context, x, y, selection_data, info, time_);
WRAPPER(GtkWidget2,  popup_menu,         int  , (void* widget), "p", widget);
WRAPPER(GtkWidget2,  show_help,          int  , (void* widget, int help_type), "pi", widget, help_type);
WRAPPER(GtkWidget2, get_accessible,      void*, (void* widget), "p", widget);
WRAPPER(GtkWidget2, screen_changed,      void , (void* widget, void* previous_screen), "pp", widget, previous_screen);
WRAPPER(GtkWidget2, can_activate_accel,  int  , (void* widget, uint32_t signal_id), "pu", widget, signal_id);
WRAPPER(GtkWidget2, grab_broken_event,   int  , (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2,  composited_changed, void , (void* widget), "p", widget);
WRAPPER(GtkWidget2,  query_tooltip,      int  , (void* widget, int32_t x, int32_t y, int keyboard_tooltip, void* tooltip), "piiip", widget, x, y, keyboard_tooltip, tooltip);

#define SUPERGO() \
    GO(dispatch_child_properties_changed, vFpup);   \
    GO(show, vFp);                                  \
    GO(show_all, vFp);                              \
    GO(hide, vFp);                                  \
    GO(hide_all, vFp);                              \
    GO(map, vFp);                                   \
    GO(unmap, vFp);                                 \
    GO(realize, vFp);                               \
    GO(unrealize, vFp);                             \
    GO(size_request, vFpp);                         \
    GO(size_allocate, vFpp);                        \
    GO(state_changed, vFpi);                        \
    GO(parent_set, vFpp);                           \
    GO(hierarchy_changed, vFpp);                    \
    GO(style_set, vFpp);                            \
    GO(direction_changed, vFpi);                    \
    GO(grab_notify, vFpi);                          \
    GO(child_notify, vFpp);                         \
    GO(mnemonic_activate, iFpi);                    \
    GO(grab_focus, vFp);                            \
    GO(focus, iFpi);                                \
    GO(event, iFpp);                                \
    GO(button_press_event, iFpp);                   \
    GO(button_release_event, iFpp);                 \
    GO(scroll_event, iFpp);                         \
    GO(motion_notify_event, iFpp);                  \
    GO(delete_event, iFpp);                         \
    GO(destroy_event, iFpp);                        \
    GO(expose_event, iFpp);                         \
    GO(key_press_event, iFpp);                      \
    GO(key_release_event, iFpp);                    \
    GO(enter_notify_event, iFpp);                   \
    GO(leave_notify_event, iFpp);                   \
    GO(configure_event, iFpp);                      \
    GO(focus_in_event, iFpp);                       \
    GO(focus_out_event, iFpp);                      \
    GO(map_event, iFpp);                            \
    GO(unmap_event, iFpp);                          \
    GO(property_notify_event, iFpp);                \
    GO(selection_clear_event, iFpp);                \
    GO(selection_request_event, iFpp);              \
    GO(selection_notify_event, iFpp);               \
    GO(proximity_in_event, iFpp);                   \
    GO(proximity_out_event, iFpp);                  \
    GO(visibility_notify_event, iFpp);              \
    GO(client_event, iFpp);                         \
    GO(no_expose_event, iFpp);                      \
    GO(window_state_event, iFpp);                   \
    GO(selection_get, vFppuu);                      \
    GO(selection_received, vFppu);                  \
    GO(drag_begin, vFpp);                           \
    GO(drag_end, vFpp);                             \
    GO(drag_data_get, vFpppuu);                     \
    GO(drag_data_delete, vFpp);                     \
    GO(drag_leave, vFppu);                          \
    GO(drag_motion, iFppiiu);                       \
    GO(drag_drop, iFppiiu);                         \
    GO(drag_data_received, vFppiipuu);              \
    GO(popup_menu, iFp);                            \
    GO(show_help, iFpi);                            \
    GO(get_accessible, pFp);                        \
    GO(screen_changed, vFpp);                       \
    GO(can_activate_accel, iFpu);                   \
    GO(grab_broken_event, iFpp);                    \
    GO(composited_changed, vFp);                    \
    GO(query_tooltip, iFpiiip);

// wrap (so bridge all calls, just in case)
static void wrapGtkWidget2Class(my_GtkWidget2Class_t* class)
{
    wrapGtkObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkWidget2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkWidget2Class(my_GtkWidget2Class_t* class)
{
    unwrapGtkObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkWidget2 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkWidget2Class(my_GtkWidget2Class_t* class)
{
    bridgeGtkObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkWidget2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkWidget3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkWidget3, dispatch_child_properties_changed, void, (void* widget, uint32_t n_pspecs, void* pspecs), "pup", widget, n_pspecs, pspecs);
WRAPPER(GtkWidget3, destroy, void,              (void* widget), "p", widget);
WRAPPER(GtkWidget3, show, void,                 (void* widget), "p", widget);
WRAPPER(GtkWidget3, show_all, void,             (void* widget), "p", widget);
WRAPPER(GtkWidget3, hide, void,                 (void* widget), "p", widget);
WRAPPER(GtkWidget3, map, void,                  (void* widget), "p", widget);
WRAPPER(GtkWidget3, unmap, void,                (void* widget), "p", widget);
WRAPPER(GtkWidget3, realize, void,              (void* widget), "p", widget);
WRAPPER(GtkWidget3, unrealize, void,            (void* widget), "p", widget);
WRAPPER(GtkWidget3, size_allocate, void,        (void* widget, void* allocation), "pp", widget, allocation);
WRAPPER(GtkWidget3, state_changed, void,        (void* widget, int previous_state), "pi", widget, previous_state);
WRAPPER(GtkWidget3, state_flags_changed, void,  (void* widget, int previous_state_flags), "pi", widget, previous_state_flags);
WRAPPER(GtkWidget3, parent_set, void,           (void* widget, void* previous_parent), "pp", widget, previous_parent);
WRAPPER(GtkWidget3, hierarchy_changed, void,    (void* widget, void* previous_toplevel), "pp", widget, previous_toplevel);
WRAPPER(GtkWidget3, style_set, void,            (void* widget, void* previous_style), "pp", widget, previous_style);
WRAPPER(GtkWidget3, direction_changed, void,    (void* widget, int previous_direction), "pi", widget, previous_direction);
WRAPPER(GtkWidget3, grab_notify, void,          (void* widget, int was_grabbed), "pi", widget, was_grabbed);
WRAPPER(GtkWidget3, child_notify, void,         (void* widget, void* child_property), "pp", widget, child_property);
WRAPPER(GtkWidget3, draw, int,                  (void* widget, void* cr), "pp", widget, cr);
WRAPPER(GtkWidget3, get_request_mode, int,      (void* widget), "p", widget);
WRAPPER(GtkWidget3, get_preferred_height, void, (void* widget, void* minimum_height, void* natural_height), "ppp", widget, minimum_height, natural_height);
WRAPPER(GtkWidget3, get_preferred_width_for_height, void,  (void* widget, int height, void* minimum_width, void* natural_width), "pipp", widget, height, minimum_width, natural_width);
WRAPPER(GtkWidget3, get_preferred_width, void,  (void* widget, void* minimum_width, void* natural_width), "ppp", widget, minimum_width, natural_width);
WRAPPER(GtkWidget3, get_preferred_height_for_width, void,  (void* widget, int width, void* minimum_height, void* natural_height), "pipp", widget, width, minimum_height, natural_height);
WRAPPER(GtkWidget3, mnemonic_activate, int,     (void* widget, int group_cycling), "pi", widget, group_cycling);
WRAPPER(GtkWidget3, grab_focus, void,           (void* widget), "p", widget);
WRAPPER(GtkWidget3, focus, int,                 (void* widget, int direction), "pi", widget, direction);
WRAPPER(GtkWidget3, move_focus, void,           (void* widget, int direction), "pi", widget, direction);
WRAPPER(GtkWidget3, keynav_failed, int,         (void* widget, int direction), "pi", widget, direction);
WRAPPER(GtkWidget3, event, int,                 (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, button_press_event, int,    (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, button_release_event, int,  (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, scroll_event, int,          (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, motion_notify_event, int,   (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, delete_event, int,          (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, destroy_event, int,         (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, key_press_event, int,       (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, key_release_event, int,     (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, enter_notify_event, int,    (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, leave_notify_event, int,    (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, configure_event, int,       (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, focus_in_event, int,        (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, focus_out_event, int,       (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, map_event, int,             (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, unmap_event, int,           (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, property_notify_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, selection_clear_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, selection_request_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, selection_notify_event, int,(void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, proximity_in_event, int,    (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, proximity_out_event, int,   (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, visibility_notify_event, int,   (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, window_state_event, int,    (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, damage_event, int,          (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, grab_broken_event, int,     (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, selection_get, void,        (void* widget, void* selection_data, uint32_t info, uint32_t time_), "ppuu", widget, selection_data, info, time_);
WRAPPER(GtkWidget3, selection_received, void,   (void* widget, void* selection_data, uint32_t time_), "ppu", widget, selection_data, time_);
WRAPPER(GtkWidget3, drag_begin, void,           (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget3, drag_end, void,             (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget3, drag_data_get, void,        (void* widget, void* context, void* selection_data, uint32_t info, uint32_t time_), "pppuu", widget, context, selection_data, info, time_);
WRAPPER(GtkWidget3, drag_data_delete, void,     (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget3, drag_leave, void,           (void* widget, void* context, uint32_t time_), "ppu", widget, context, time_);
WRAPPER(GtkWidget3, drag_motion, int,           (void* widget, void* context, int x, int y, uint32_t time_), "ppiiu", widget, context, x, y, time_);
WRAPPER(GtkWidget3, drag_drop, int,             (void* widget, void* context, int x, int y, uint32_t time_), "ppiiu", widget, context, x, y, time_);
WRAPPER(GtkWidget3, drag_data_received, void,   (void* widget, void* context, int x, int y, void* selection_data, uint32_t info, uint32_t time_), "ppiipuu", widget, context, x, y, selection_data, info, time_);
WRAPPER(GtkWidget3, drag_failed, int,           (void* widget, void* context, int result), "ppi", widget, context, result);
WRAPPER(GtkWidget3, popup_menu, int,            (void* widget), "p", widget);
WRAPPER(GtkWidget3, show_help, int,             (void* widget, int help_type), "pi", widget, help_type);
WRAPPER(GtkWidget3, get_accessible, void*,      (void *widget), "p", widget);
WRAPPER(GtkWidget3, screen_changed, void,       (void* widget, void* previous_screen), "pp", widget, previous_screen);
WRAPPER(GtkWidget3, can_activate_accel, int,    (void* widget, uint32_t signal_id), "pu", widget, signal_id);
WRAPPER(GtkWidget3, composited_changed, void,   (void* widget), "p", widget);
WRAPPER(GtkWidget3, query_tooltip, int,         (void* widget, int x, int y, int keyboard_tooltip, void* tooltip), "piiip", widget, x, y, keyboard_tooltip, tooltip);
WRAPPER(GtkWidget3, compute_expand, void,       (void* widget, void* hexpand_p, void* vexpand_p), "ppp", widget, hexpand_p, vexpand_p);
WRAPPER(GtkWidget3, adjust_size_request, void,  (void* widget, int orientation, void* minimum_size, void* natural_size), "pipp", widget, orientation, minimum_size, natural_size);
WRAPPER(GtkWidget3, adjust_size_allocation, void, (void*widget, int orientation, void* minimum_size, void* natural_size, void* allocated_pos, void* allocated_size), "pipppp", widget, orientation, minimum_size, natural_size, allocated_pos, allocated_size);
WRAPPER(GtkWidget3, style_updated, void,        (void* widget), "p", widget);
WRAPPER(GtkWidget3, touch_event, int,           (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, get_preferred_height_and_baseline_for_width, void, (void* widget, int width, void* minimum_height, void* natural_height, void* minimum_baseline, void* natural_baseline), "pipppp", widget, width, minimum_height, natural_height, minimum_baseline, natural_baseline);
WRAPPER(GtkWidget3, adjust_baseline_request, void,  (void* widget, void* minimum_baseline, void* natural_baseline), "ppp", widget, minimum_baseline, natural_baseline);
WRAPPER(GtkWidget3, adjust_baseline_allocation, void,  (void* widget, void* baseline), "pp", widget, baseline);
WRAPPER(GtkWidget3, queue_draw_region, void,    (void* widget, void* region), "pp", widget, region);

#define SUPERGO() \
    GO(dispatch_child_properties_changed, vFpup);   \
    GO(destroy, vFp);                               \
    GO(show, vFp);                                  \
    GO(show_all, vFp);                              \
    GO(hide, vFp);                                  \
    GO(map, vFp);                                   \
    GO(unmap, vFp);                                 \
    GO(realize, vFp);                               \
    GO(unrealize, vFp);                             \
    GO(size_allocate, vFpp);                        \
    GO(state_changed, vFpi);                        \
    GO(state_flags_changed, vFpi);                  \
    GO(parent_set, vFpp);                           \
    GO(hierarchy_changed, vFpp);                    \
    GO(style_set, vFpp);                            \
    GO(direction_changed, vFpi);                    \
    GO(grab_notify, vFpi);                          \
    GO(child_notify, vFpp);                         \
    GO(draw, iFpp);                                 \
    GO(get_request_mode, iFp);                      \
    GO(get_preferred_height, vFppp);                \
    GO(get_preferred_width_for_height, vFpipp);     \
    GO(get_preferred_width, vFppp);                 \
    GO(get_preferred_height_for_width, vFpipp);     \
    GO(mnemonic_activate, iFpi);                    \
    GO(grab_focus, vFp);                            \
    GO(focus, iFpi);                                \
    GO(move_focus, vFpi);                           \
    GO(keynav_failed, iFpi);                        \
    GO(event, iFpp);                                \
    GO(button_press_event, iFpp);                   \
    GO(button_release_event, iFpp);                 \
    GO(scroll_event, iFpp);                         \
    GO(motion_notify_event, iFpp);                  \
    GO(delete_event, iFpp);                         \
    GO(destroy_event, iFpp);                        \
    GO(key_press_event, iFpp);                      \
    GO(key_release_event, iFpp);                    \
    GO(enter_notify_event, iFpp);                   \
    GO(leave_notify_event, iFpp);                   \
    GO(configure_event, iFpp);                      \
    GO(focus_in_event, iFpp);                       \
    GO(focus_out_event, iFpp);                      \
    GO(map_event, iFpp);                            \
    GO(unmap_event, iFpp);                          \
    GO(property_notify_event, iFpp);                \
    GO(selection_clear_event, iFpp);                \
    GO(selection_request_event, iFpp);              \
    GO(selection_notify_event, iFpp);               \
    GO(proximity_in_event, iFpp);                   \
    GO(proximity_out_event, iFpp);                  \
    GO(visibility_notify_event, iFpp);              \
    GO(window_state_event, iFpp);                   \
    GO(damage_event, iFpp);                         \
    GO(grab_broken_event, iFpp);                    \
    GO(selection_get, vFppuu);                      \
    GO(selection_received, vFppu);                  \
    GO(drag_begin, vFpp);                           \
    GO(drag_end, vFpp);                             \
    GO(drag_data_get, vFpppuu);                     \
    GO(drag_data_delete, vFpp);                     \
    GO(drag_leave, vFppu);                          \
    GO(drag_motion, iFppiiu);                       \
    GO(drag_drop, iFppiiu);                         \
    GO(drag_data_received, vFppiipuu);              \
    GO(drag_failed, iFppi);                         \
    GO(popup_menu, iFp);                            \
    GO(show_help, iFpi);                            \
    GO(get_accessible, pFp);                        \
    GO(screen_changed, vFpp);                       \
    GO(can_activate_accel, iFpu);                   \
    GO(grab_broken_event, iFpp);                    \
    GO(composited_changed, vFp);                    \
    GO(query_tooltip, iFpiiip);                     \
    GO(compute_expand, vFppp);                      \
    GO(adjust_size_request, vFpipp);                \
    GO(adjust_size_allocation, vFpipppp);           \
    GO(style_updated, vFp);                         \
    GO(touch_event, iFpp);                          \
    GO(get_preferred_height_and_baseline_for_width, vFpipppp);\
    GO(adjust_baseline_request, vFppp);             \
    GO(adjust_baseline_allocation, vFpp);           \
    GO(queue_draw_region, vFpp);                    \

// wrap (so bridge all calls, just in case)
static void wrapGtkWidget3Class(my_GtkWidget3Class_t* class)
{
    wrapGInitiallyUnownedClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkWidget3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkWidget3Class(my_GtkWidget3Class_t* class)
{
    unwrapGInitiallyUnownedClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkWidget3 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkWidget3Class(my_GtkWidget3Class_t* class)
{
    bridgeGInitiallyUnownedClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkWidget3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkContainer2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkContainer2, add, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer2, remove, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer2, check_resize, void, (void* container), "p", container);
WRAPPER(GtkContainer2, forall, void, (void* container, int include_internals, void* callback, void* callback_data), "pipp", container, include_internals, AddCheckBridge(my_bridge, vFpp, callback, 0, NULL), callback_data);
WRAPPER(GtkContainer2, set_focus_child, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer2, child_type, int, (void* container), "p", container);
WRAPPER(GtkContainer2, composite_name, void*, (void* container, void* child), "pp", container, child);
WRAPPER(GtkContainer2, set_child_property, void, (void* container, void* child, uint32_t property_id, void* value, void* pspec), "ppupp", container, child, property_id, value, pspec);
WRAPPER(GtkContainer2, get_child_property, void, (void* container, void* child, uint32_t property_id, void* value, void* pspec), "ppupp", container, child, property_id, value, pspec);

#define SUPERGO() \
    GO(add, vFpp);                  \
    GO(remove, vFpp);               \
    GO(check_resize, vFp);          \
    GO(forall, vFpipp);             \
    GO(set_focus_child, vFpp);      \
    GO(child_type, iFp);            \
    GO(composite_name, pFpp);       \
    GO(set_child_property, vFppupp);\
    GO(get_child_property, vFppupp);

// wrap (so bridge all calls, just in case)
static void wrapGtkContainer2Class(my_GtkContainer2Class_t* class)
{
    wrapGtkWidget2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkContainer2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkContainer2Class(my_GtkContainer2Class_t* class)
{
    unwrapGtkWidget2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkContainer2 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkContainer2Class(my_GtkContainer2Class_t* class)
{
    bridgeGtkWidget2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkContainer2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkContainer3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkContainer3, add, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer3, remove, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer3, check_resize, void, (void* container), "p", container);
WRAPPER(GtkContainer3, forall, void, (void* container, int include_internals, void* callback, void* callback_data), "pipp", container, include_internals, AddCheckBridge(my_bridge, vFpp, callback, 0, NULL), callback_data);
WRAPPER(GtkContainer3, set_focus_child, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer3, child_type, int, (void* container), "p", container);
WRAPPER(GtkContainer3, composite_name, void*, (void* container, void* child), "pp", container, child);
WRAPPER(GtkContainer3, set_child_property, void, (void* container, void* child, uint32_t property_id, void* value, void* pspec), "ppupp", container, child, property_id, value, pspec);
WRAPPER(GtkContainer3, get_child_property, void, (void* container, void* child, uint32_t property_id, void* value, void* pspec), "ppupp", container, child, property_id, value, pspec);
WRAPPER(GtkContainer3, get_path_for_child, void*, (void* container, void* child), "pp", container, child);

#define SUPERGO() \
    GO(add, vFpp);                  \
    GO(remove, vFpp);               \
    GO(check_resize, vFp);          \
    GO(forall, vFpipp);             \
    GO(set_focus_child, vFpp);      \
    GO(child_type, iFp);            \
    GO(composite_name, pFpp);       \
    GO(set_child_property, vFppupp);\
    GO(get_child_property, vFppupp);\
    GO(get_path_for_child, pFpp);   \

// wrap (so bridge all calls, just in case)
static void wrapGtkContainer3Class(my_GtkContainer3Class_t* class)
{
    wrapGtkWidget3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkContainer3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkContainer3Class(my_GtkContainer3Class_t* class)
{
    unwrapGtkWidget3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkContainer3 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkContainer3Class(my_GtkContainer3Class_t* class)
{
    bridgeGtkWidget3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkContainer3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkActionClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkAction, activate, void, (void* action), "p", action);
WRAPPER(GtkAction, create_menu_item, void*, (void* action), "p", action);
WRAPPER(GtkAction, create_tool_item, void*, (void* action), "p", action);
WRAPPER(GtkAction, connect_proxy, void , (void* action, void* proxy), "pp", action, proxy);
WRAPPER(GtkAction, disconnect_proxy, void , (void* action, void* proxy), "pp", action, proxy);
WRAPPER(GtkAction, create_menu, void*, (void* action), "p", action);

#define SUPERGO() \
    GO(activate, vFp);          \
    GO(create_menu_item, pFp);  \
    GO(create_tool_item, pFp);  \
    GO(connect_proxy, vFpp);    \
    GO(disconnect_proxy, vFpp); \
    GO(create_menu, pFp);       \

// wrap (so bridge all calls, just in case)
static void wrapGtkActionClass(my_GtkActionClass_t* class)
{
    wrapGObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkAction (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkActionClass(my_GtkActionClass_t* class)
{
    unwrapGObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkAction (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkActionClass(my_GtkActionClass_t* class)
{
    bridgeGObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkAction (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

// ----- GtkMisc2Class ------

// wrap (so bridge all calls, just in case)
static void wrapGtkMisc2Class(my_GtkMisc2Class_t* class)
{
    wrapGtkWidget2Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkMisc2Class(my_GtkMisc2Class_t* class)
{
    unwrapGtkWidget2Class(&class->parent_class);
}
// autobridge
static void bridgeGtkMisc2Class(my_GtkMisc2Class_t* class)
{
    bridgeGtkWidget2Class(&class->parent_class);
}

// ----- GtkMisc3Class ------
// no wrapper x86 -> natives of callbacks

#define SUPERGO()           \

// wrap (so bridge all calls, just in case)
static void wrapGtkMisc3Class(my_GtkMisc3Class_t* class)
{
    wrapGtkWidget3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkMisc3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkMisc3Class(my_GtkMisc3Class_t* class)
{
    unwrapGtkWidget3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkMisc3 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkMisc3Class(my_GtkMisc3Class_t* class)
{
    bridgeGtkWidget3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkMisc3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkGtkMenuButtonClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkMenuButton, activate, void, (void* self), "p", self);

#define SUPERGO() \
    GO(activate, vFp);

// wrap (so bridge all calls, just in case)
static void wrapGtkMenuButtonClass(my_GtkMenuButtonClass_t* class)
{
    wrapGtkWidget3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkMenuButton (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkMenuButtonClass(my_GtkMenuButtonClass_t* class)
{
    unwrapGtkWidget3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkMenuButton (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkMenuButtonClass(my_GtkMenuButtonClass_t* class)
{
    bridgeGtkWidget3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkMenuButton (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

// ----- GtkLabel2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkLabel2, move_cursor, void, (void* label, int step, int count, int extend_selection), "piii", label, step, count, extend_selection);
WRAPPER(GtkLabel2, copy_clipboard, void, (void* label), "p", label);
WRAPPER(GtkLabel2, populate_popup, void, (void* label, void* menu), "pp", label, menu);
WRAPPER(GtkLabel2, activate_link, int, (void* label, void* uri), "pp", label, uri);

#define SUPERGO() \
    GO(move_cursor, vFpiii);    \
    GO(copy_clipboard, vFp);    \
    GO(populate_popup, vFpp);   \
    GO(activate_link, iFpp);    \

// wrap (so bridge all calls, just in case)
static void wrapGtkLabel2Class(my_GtkLabel2Class_t* class)
{
    wrapGtkMisc2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkLabel2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkLabel2Class(my_GtkLabel2Class_t* class)
{
    unwrapGtkMisc2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkLabel2 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkLabel2Class(my_GtkLabel2Class_t* class)
{
    bridgeGtkMisc2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkLabel2 (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

// ----- GtkLabel3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkLabel3, move_cursor, void, (void* label, int step, int count, int extend_selection), "piii", label, step, count, extend_selection);
WRAPPER(GtkLabel3, copy_clipboard, void, (void* label), "p", label);
WRAPPER(GtkLabel3, populate_popup, void, (void* label, void* menu), "pp", label, menu);
WRAPPER(GtkLabel3, activate_link, int, (void* label, void* uri), "pp", label, uri);

#define SUPERGO() \
    GO(move_cursor, vFpiii);    \
    GO(copy_clipboard, vFp);    \
    GO(populate_popup, vFpp);   \
    GO(activate_link, iFpp);    \

// wrap (so bridge all calls, just in case)
static void wrapGtkLabel3Class(my_GtkLabel3Class_t* class)
{
    wrapGtkMisc3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkLabel3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkLabel3Class(my_GtkLabel3Class_t* class)
{
    unwrapGtkMisc3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkLabel3 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkLabel3Class(my_GtkLabel3Class_t* class)
{
    bridgeGtkMisc3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkLabel3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkTreeView2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkTreeView2, set_scroll_adjustments, void, (void* tree_view, void* hadjustment, void* vadjustment), "ppp", tree_view, hadjustment, vadjustment);
WRAPPER(GtkTreeView2, row_activated, void, (void* tree_view, void* path, void* column), "ppp", tree_view, path, column);
WRAPPER(GtkTreeView2, test_expand_row, int, (void* tree_view, void* iter, void* path), "ppp", tree_view, iter, path);
WRAPPER(GtkTreeView2, test_collapse_row, int, (void* tree_view, void* iter, void* path), "ppp", tree_view, iter, path);
WRAPPER(GtkTreeView2, row_expanded, void, (void* tree_view, void* iter, void* path), "ppp", tree_view, iter, path);
WRAPPER(GtkTreeView2, row_collapsed, void, (void* tree_view, void* iter, void* path), "ppp", tree_view, iter, path);
WRAPPER(GtkTreeView2, columns_changed, void, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, cursor_changed, void, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, move_cursor, int, (void* tree_view, int step, int count), "pii", tree_view, step, count);
WRAPPER(GtkTreeView2, select_all, int, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, unselect_all, int, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, select_cursor_row, int, (void* tree_view, int start_editing), "pi", tree_view, start_editing);
WRAPPER(GtkTreeView2, toggle_cursor_row, int, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, expand_collapse_cursor_row, int, (void* tree_view, int logical, int expand, int open_all), "piii", tree_view, logical, expand, open_all);
WRAPPER(GtkTreeView2, select_cursor_parent, int, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, start_interactive_search, int, (void* tree_view), "p", tree_view);

#define SUPERGO() \
    GO(set_scroll_adjustments, vFppp);      \
    GO(row_activated, vFppp);               \
    GO(test_expand_row, iFppp);             \
    GO(test_collapse_row, iFppp);           \
    GO(row_expanded, vFppp);                \
    GO(row_collapsed, vFppp);               \
    GO(columns_changed, vFp);               \
    GO(cursor_changed, vFp);                \
    GO(move_cursor, iFppp);                 \
    GO(select_all, iFp);                    \
    GO(unselect_all, iFp);                  \
    GO(select_cursor_row, iFpi);            \
    GO(toggle_cursor_row, iFp);             \
    GO(expand_collapse_cursor_row, iFpiii); \
    GO(select_cursor_parent, iFp);          \
    GO(start_interactive_search, iFp);      \

// wrap (so bridge all calls, just in case)
static void wrapGtkTreeView2Class(my_GtkTreeView2Class_t* class)
{
    wrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkTreeView2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkTreeView2Class(my_GtkTreeView2Class_t* class)
{
    unwrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkTreeView2 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkTreeView2Class(my_GtkTreeView2Class_t* class)
{
    bridgeGtkContainer2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkTreeView2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkBin2Class ------

// wrap (so bridge all calls, just in case)
static void wrapGtkBin2Class(my_GtkBin2Class_t* class)
{
    wrapGtkContainer2Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkBin2Class(my_GtkBin2Class_t* class)
{
    unwrapGtkContainer2Class(&class->parent_class);
}
// autobridge
static void bridgeGtkBin2Class(my_GtkBin2Class_t* class)
{
    bridgeGtkContainer2Class(&class->parent_class);
}

// ----- GtkBin3Class ------
static void wrapGtkBin3Class(my_GtkBin3Class_t* class)
{
    wrapGtkContainer3Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkBin3Class(my_GtkBin3Class_t* class)
{
    unwrapGtkContainer3Class(&class->parent_class);
}
// autobridge
static void bridgeGtkBin3Class(my_GtkBin3Class_t* class)
{
    bridgeGtkContainer3Class(&class->parent_class);
}

// ----- GtkWindow2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkWindow2, set_focus, void, (void* window, void* focus), "pp", window, focus);
WRAPPER(GtkWindow2, frame_event, int, (void* window, void* event), "pp", window, event);
WRAPPER(GtkWindow2, activate_focus, void, (void* window), "p", window);
WRAPPER(GtkWindow2, activate_default, void, (void* window), "p", window);
WRAPPER(GtkWindow2, move_focus, void, (void* window, int direction), "pi", window, direction);
WRAPPER(GtkWindow2, keys_changed, void, (void* window), "p", window);

#define SUPERGO()               \
    GO(set_focus, vFpp);        \
    GO(frame_event, iFpp);      \
    GO(activate_focus, vFp);    \
    GO(activate_default, vFp);  \
    GO(move_focus, vFpi);       \
    GO(keys_changed, vFp);      \


// wrap (so bridge all calls, just in case)
static void wrapGtkWindow2Class(my_GtkWindow2Class_t* class)
{
    wrapGtkBin2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkWindow2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkWindow2Class(my_GtkWindow2Class_t* class)
{
    unwrapGtkBin2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkWindow2 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkWindow2Class(my_GtkWindow2Class_t* class)
{
    bridgeGtkBin2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkWindow2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkWindow3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkWindow3, set_focus, void, (void* window, void* focus), "pp", window, focus);
WRAPPER(GtkWindow3, activate_focus, void, (void* window), "p", window);
WRAPPER(GtkWindow3, activate_default, void, (void* window), "p", window);
WRAPPER(GtkWindow3, keys_changed, void, (void* window), "p", window);
WRAPPER(GtkWindow3, enable_debugging, int, (void* window, int toggle), "pi", window, toggle);

#define SUPERGO()               \
    GO(set_focus, vFpp);        \
    GO(activate_focus, vFp);    \
    GO(activate_default, vFp);  \
    GO(keys_changed, vFp);      \
    GO(enable_debugging, iFpi);


// wrap (so bridge all calls, just in case)
static void wrapGtkWindow3Class(my_GtkWindow3Class_t* class)
{
    wrapGtkBin3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkWindow3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkWindow3Class(my_GtkWindow3Class_t* class)
{
    unwrapGtkBin3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkWindow3 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkWindow3Class(my_GtkWindow3Class_t* class)
{
    bridgeGtkBin3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkWindow3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkApplicationWindowClass ------
// wrap (so bridge all calls, just in case)
static void wrapGtkApplicationWindowClass(my_GtkApplicationWindowClass_t* class)
{
    wrapGtkWindow3Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkApplicationWindowClass(my_GtkApplicationWindowClass_t* class)
{
    unwrapGtkWindow3Class(&class->parent_class);
}
// autobridge
static void bridgeGtkApplicationWindowClass(my_GtkApplicationWindowClass_t* class)
{
    bridgeGtkWindow3Class(&class->parent_class);
}

// ----- GtkListBoxClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkListBoxClass,row_selected, void, (void *box, void *row), "pp", box, row);
WRAPPER(GtkListBoxClass,row_activated, void, (void *box, void *row), "pp", box, row);
WRAPPER(GtkListBoxClass,activate_cursor_row, void, (void *box), "p", box);
WRAPPER(GtkListBoxClass,toggle_cursor_row, void, (void *box), "p", box);
WRAPPER(GtkListBoxClass,move_cursor, void, (void *box, int step, int count), "pii", box, step, count);
WRAPPER(GtkListBoxClass,selected_rows_changed, void, (void *box), "p", box);
WRAPPER(GtkListBoxClass,select_all, void, (void *box), "p", box);
WRAPPER(GtkListBoxClass,unselect_all, void, (void *box), "p", box);

#define SUPERGO()                  \
    GO(row_selected, vFpp);        \
    GO(row_activated, vFpp);    \
    GO(activate_cursor_row, vFp);    \
    GO(toggle_cursor_row, vFp);    \
    GO(move_cursor, vFpii);    \
    GO(selected_rows_changed, vFp);    \
    GO(select_all, vFp);    \
    GO(unselect_all, vFp);

// wrap (so bridge all calls, just in case)
static void wrapGtkListBoxClass(my_GtkListBoxClass_t* class)
{
    wrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkListBoxClass (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkListBoxClass(my_GtkListBoxClass_t* class)
{
    unwrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkListBoxClass (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkListBoxClass(my_GtkListBoxClass_t* class)
{
    bridgeGtkContainer3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkListBoxClass (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkListBoxRowClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkListBoxRowClass, activate, void, (void *row), "p", row);

#define SUPERGO()       \
    GO(activate, vFpp);

// wrap (so bridge all calls, just in case)
static void wrapGtkListBoxRowClass(my_GtkListBoxRowClass_t* class)
{
    wrapGtkBin3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkListBoxRowClass (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkListBoxRowClass(my_GtkListBoxRowClass_t* class)
{
    unwrapGtkBin3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkListBoxRowClass (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkListBoxRowClass(my_GtkListBoxRowClass_t* class)
{
    bridgeGtkBin3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkListBoxRowClass (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkTable2Class ------
// wrap (so bridge all calls, just in case)
static void wrapGtkTable2Class(my_GtkTable2Class_t* class)
{
    wrapGtkContainer2Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkTable2Class(my_GtkTable2Class_t* class)
{
    unwrapGtkContainer2Class(&class->parent_class);
}
// autobridge
static void bridgeGtkTable2Class(my_GtkTable2Class_t* class)
{
    bridgeGtkContainer2Class(&class->parent_class);
}

// ----- GtkFixed2Class ------

// wrap (so bridge all calls, just in case)
static void wrapGtkFixed2Class(my_GtkFixed2Class_t* class)
{
    wrapGtkContainer2Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkFixed2Class(my_GtkFixed2Class_t* class)
{
    unwrapGtkContainer2Class(&class->parent_class);
}
// autobridge
static void bridgeGtkFixed2Class(my_GtkFixed2Class_t* class)
{
    bridgeGtkContainer2Class(&class->parent_class);
}

// ----- MetaFrames2Class ------

// wrap (so bridge all calls, just in case)
static void wrapMetaFrames2Class(my_MetaFrames2Class_t* class)
{
    wrapGtkWindow2Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapMetaFrames2Class(my_MetaFrames2Class_t* class)
{
    unwrapGtkWindow2Class(&class->parent_class);
}
// autobridge
static void bridgeMetaFrames2Class(my_MetaFrames2Class_t* class)
{
    bridgeGtkWindow2Class(&class->parent_class);
}

// ----- GDBusObjectManagerClientClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GDBusObjectManagerClient,interface_proxy_signal, void, (void* manager, void* object_proxy, void* interface_proxy, void* sender_name, void* signal_name, void* parameters), "pppppp", manager, object_proxy, interface_proxy, sender_name, signal_name, parameters);
WRAPPER(GDBusObjectManagerClient,interface_proxy_properties_changed, void, (void* manager, void* object_proxy, void* interface_proxy, void* changed_properties, void* invalidated_properties), "ppppp", manager, object_proxy, interface_proxy, changed_properties, invalidated_properties);

#define SUPERGO()                                       \
    GO(interface_proxy_signal, vFpppppp);               \
    GO(interface_proxy_properties_changed, vFppppp);    \


// wrap (so bridge all calls, just in case)
static void wrapGDBusObjectManagerClientClass(my_GDBusObjectManagerClientClass_t* class)
{
    wrapGObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GDBusObjectManagerClient (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGDBusObjectManagerClientClass(my_GDBusObjectManagerClientClass_t* class)
{
    unwrapGObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GDBusObjectManagerClient (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGDBusObjectManagerClientClass(my_GDBusObjectManagerClientClass_t* class)
{
    bridgeGObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GDBusObjectManagerClient (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkButton2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkButton2, pressed, void,  (void* button), "p", button);
WRAPPER(GtkButton2, released, void, (void* button), "p", button);
WRAPPER(GtkButton2, clicked, void,  (void* button), "p", button);
WRAPPER(GtkButton2, enter, void,    (void* button), "p", button);
WRAPPER(GtkButton2, leave, void,    (void* button), "p", button);
WRAPPER(GtkButton2, activate, void, (void* button), "p", button);

#define SUPERGO()               \
    GO(pressed, vFp);           \
    GO(released, vFp);          \
    GO(clicked, vFp);           \
    GO(enter, vFp);             \
    GO(leave, vFp);             \
    GO(activate, vFp);          \


// wrap (so bridge all calls, just in case)
static void wrapGtkButton2Class(my_GtkButton2Class_t* class)
{
    wrapGtkBin2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkButton2Class(my_GtkButton2Class_t* class)
{
    unwrapGtkBin2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkButton2 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkButton2Class(my_GtkButton2Class_t* class)
{
    bridgeGtkBin2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkButton2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkButton3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkButton3, pressed, void,  (void* button), "p", button);
WRAPPER(GtkButton3, released, void, (void* button), "p", button);
WRAPPER(GtkButton3, clicked, void,  (void* button), "p", button);
WRAPPER(GtkButton3, enter, void,    (void* button), "p", button);
WRAPPER(GtkButton3, leave, void,    (void* button), "p", button);
WRAPPER(GtkButton3, activate, void, (void* button), "p", button);

#define SUPERGO()               \
    GO(pressed, vFp);           \
    GO(released, vFp);          \
    GO(clicked, vFp);           \
    GO(enter, vFp);             \
    GO(leave, vFp);             \
    GO(activate, vFp);          \


// wrap (so bridge all calls, just in case)
static void wrapGtkButton3Class(my_GtkButton3Class_t* class)
{
    wrapGtkBin3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkButton3Class(my_GtkButton3Class_t* class)
{
    unwrapGtkBin3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkButton3 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkButton3Class(my_GtkButton3Class_t* class)
{
    bridgeGtkBin3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkButton3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkComboBox2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkComboBox2, changed, void, (void* combo_box), "p", combo_box);
WRAPPER(GtkComboBox2, get_active_text, void*, (void* combo_box), "p", combo_box);

#define SUPERGO()               \
    GO(changed, vFp);           \
    GO(get_active_text, pFp);   \


// wrap (so bridge all calls, just in case)
static void wrapGtkComboBox2Class(my_GtkComboBox2Class_t* class)
{
    wrapGtkBin2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkComboBox2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkComboBox2Class(my_GtkComboBox2Class_t* class)
{
    unwrapGtkBin2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkComboBox2 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkComboBox2Class(my_GtkComboBox2Class_t* class)
{
    bridgeGtkBin2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkComboBox2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkToggleButton2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkToggleButton2, toggled, void, (void* toggle_button), "p", toggle_button);

#define SUPERGO()               \
    GO(toggled, vFp);           \


// wrap (so bridge all calls, just in case)
static void wrapGtkToggleButton2Class(my_GtkToggleButton2Class_t* class)
{
    wrapGtkButton2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkToggleButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkToggleButton2Class(my_GtkToggleButton2Class_t* class)
{
    unwrapGtkButton2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkToggleButton2 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkToggleButton2Class(my_GtkToggleButton2Class_t* class)
{
    bridgeGtkButton2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkToggleButton2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkCheckButton2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkCheckButton2, draw_indicator, void, (void* check_button, void* area), "pp", check_button, area);

#define SUPERGO()               \
    GO(draw_indicator, vFpp);   \


// wrap (so bridge all calls, just in case)
static void wrapGtkCheckButton2Class(my_GtkCheckButton2Class_t* class)
{
    wrapGtkToggleButton2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkCheckButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkCheckButton2Class(my_GtkCheckButton2Class_t* class)
{
    unwrapGtkToggleButton2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkCheckButton2 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkCheckButton2Class(my_GtkCheckButton2Class_t* class)
{
    bridgeGtkToggleButton2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkCheckButton2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkEntry2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkEntry2, populate_popup, void,     (void* entry, void* menu), "pp", entry, menu);
WRAPPER(GtkEntry2, activate, void,           (void* entry), "p", entry);
WRAPPER(GtkEntry2, move_cursor, void,        (void* entry, int step, int count, int extend_selection), "piii", entry, step, count, extend_selection);
WRAPPER(GtkEntry2, insert_at_cursor, void,   (void* entry, void* str), "pp", entry, str);
WRAPPER(GtkEntry2, delete_from_cursor, void, (void* entry, size_t type, int count), "pLi", entry, type, count);
WRAPPER(GtkEntry2, backspace, void,          (void* entry), "p", entry);
WRAPPER(GtkEntry2, cut_clipboard, void,      (void* entry), "p", entry);
WRAPPER(GtkEntry2, copy_clipboard, void,     (void* entry), "p", entry);
WRAPPER(GtkEntry2, paste_clipboard, void,    (void* entry), "p", entry);
WRAPPER(GtkEntry2, toggle_overwrite, void,   (void* entry), "p", entry);
WRAPPER(GtkEntry2, get_text_area_size, void, (void* entry, void* x, void* y, void* width, void* height), "ppppp", entry, x, y, width, height);

#define SUPERGO()                   \
    GO(populate_popup, vFpp);       \
    GO(activate, vFp);              \
    GO(move_cursor, vFpiii);        \
    GO(insert_at_cursor, vFp);      \
    GO(delete_from_cursor, vFpii);  \
    GO(backspace, vFp);             \
    GO(cut_clipboard, vFp);         \
    GO(copy_clipboard, vFp);        \
    GO(paste_clipboard, vFp);       \
    GO(toggle_overwrite, vFp);      \
    GO(get_text_area_size, vFppppp);\

// wrap (so bridge all calls, just in case)
static void wrapGtkEntry2Class(my_GtkEntry2Class_t* class)
{
    wrapGtkWidget2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkEntry2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkEntry2Class(my_GtkEntry2Class_t* class)
{
    unwrapGtkWidget2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkEntry2 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkEntry2Class(my_GtkEntry2Class_t* class)
{
    bridgeGtkWidget2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkEntry2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkSpinButton2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkSpinButton2, input, int,  (void* spin_button, void* new_value), "pp", spin_button, new_value);
WRAPPER(GtkSpinButton2, output, int, (void* spin_button), "p", spin_button);
WRAPPER(GtkSpinButton2, value_changed, void, (void* spin_button), "p", spin_button);
WRAPPER(GtkSpinButton2, change_value, void, (void* spin_button, int scroll), "pi", spin_button, scroll);
WRAPPER(GtkSpinButton2, wrapped, void, (void* spin_button), "p", spin_button);

#define SUPERGO()           \
    GO(input, iFpp);        \
    GO(output, iFp);        \
    GO(value_changed, vFp); \
    GO(change_value, vFpi); \
    GO(wrapped, vFp);       \

// wrap (so bridge all calls, just in case)
static void wrapGtkSpinButton2Class(my_GtkSpinButton2Class_t* class)
{
    wrapGtkEntry2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkSpinButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkSpinButton2Class(my_GtkSpinButton2Class_t* class)
{
    unwrapGtkEntry2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkSpinButton2 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkSpinButton2Class(my_GtkSpinButton2Class_t* class)
{
    bridgeGtkEntry2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkSpinButton2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkProgress2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkProgress2, paint, void,          (void* progress), "p", progress);
WRAPPER(GtkProgress2, update, void,         (void* progress), "p", progress);
WRAPPER(GtkProgress2, act_mode_enter, void, (void* progress), "p", progress);

#define SUPERGO()           \
    GO(paint, vFp);         \
    GO(update, vFp);        \
    GO(act_mode_enter, vFp);\

// wrap (so bridge all calls, just in case)
static void wrapGtkProgress2Class(my_GtkProgress2Class_t* class)
{
    wrapGtkWidget2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkProgress2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkProgress2Class(my_GtkProgress2Class_t* class)
{
    unwrapGtkWidget2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkProgress2 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkProgress2Class(my_GtkProgress2Class_t* class)
{
    bridgeGtkWidget2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkProgress2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkProgressBar2Class ------
// no wrapper x86 -> natives of callbacks

#define SUPERGO()           \

// wrap (so bridge all calls, just in case)
static void wrapGtkProgressBar2Class(my_GtkProgressBar2Class_t* class)
{
    wrapGtkProgress2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkProgressBar2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkProgressBar2Class(my_GtkProgressBar2Class_t* class)
{
    unwrapGtkProgress2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkProgressBar2 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkProgressBar2Class(my_GtkProgressBar2Class_t* class)
{
    bridgeGtkProgress2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkProgressBar2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkFrame2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkFrame2, compute_child_allocation, void, (void* frame, void* allocation), "pp", frame, allocation);

#define SUPERGO()                       \
    GO(compute_child_allocation, vFpp); \

// wrap (so bridge all calls, just in case)
static void wrapGtkFrame2Class(my_GtkFrame2Class_t* class)
{
    wrapGtkBin2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkFrame2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkFrame2Class(my_GtkFrame2Class_t* class)
{
    unwrapGtkBin2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkFrame2 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkFrame2Class(my_GtkFrame2Class_t* class)
{
    bridgeGtkBin2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkFrame2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkMenuShell2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkMenuShell2,deactivate, void,      (void* menu_shell), "p", menu_shell);
WRAPPER(GtkMenuShell2,selection_done, void,  (void* menu_shell), "p", menu_shell);
WRAPPER(GtkMenuShell2,move_current, void,    (void* menu_shell, int direction),  "pi", menu_shell, direction);
WRAPPER(GtkMenuShell2,activate_current, void,(void* menu_shell, int force_hide), "pi", menu_shell, force_hide);
WRAPPER(GtkMenuShell2,cancel, void,          (void* menu_shell), "p", menu_shell);
WRAPPER(GtkMenuShell2,select_item, void,     (void* menu_shell, void* menu_item), "pp", menu_shell, menu_item);
WRAPPER(GtkMenuShell2,insert, void,          (void* menu_shell, void* child, int position), "ppi", menu_shell, child, position);
WRAPPER(GtkMenuShell2,get_popup_delay, int,  (void* menu_shell), "p", menu_shell);
WRAPPER(GtkMenuShell2,move_selected, int,    (void* menu_shell, int distance), "pi", menu_shell, distance);

#define SUPERGO()               \
    GO(deactivate, vFp);        \
    GO(selection_done, vFp);    \
    GO(move_current, vFpi);     \
    GO(activate_current, vFpi); \
    GO(cancel, vFp);            \
    GO(select_item, vFpp);      \
    GO(insert, vFppi);          \
    GO(get_popup_delay, iFp);   \
    GO(move_selected, iFpi);    \

// wrap (so bridge all calls, just in case)
static void wrapGtkMenuShell2Class(my_GtkMenuShell2Class_t* class)
{
    wrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkMenuShell2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkMenuShell2Class(my_GtkMenuShell2Class_t* class)
{
    unwrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkMenuShell2 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkMenuShell2Class(my_GtkMenuShell2Class_t* class)
{
    bridgeGtkContainer2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkMenuShell2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkMenuBar2Class ------
// no wrapper x86 -> natives of callbacks

#define SUPERGO()           \

// wrap (so bridge all calls, just in case)
static void wrapGtkMenuBar2Class(my_GtkMenuBar2Class_t* class)
{
    wrapGtkMenuShell2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkMenuBar2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkMenuBar2Class(my_GtkMenuBar2Class_t* class)
{
    unwrapGtkMenuShell2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkMenuBar2 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkMenuBar2Class(my_GtkMenuBar2Class_t* class)
{
    bridgeGtkMenuShell2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkMenuBar2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkTextView2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkTextView2, set_scroll_adjustments, void,   (void* text_view, void* hadjustment, void* vadjustment), "ppp", text_view, hadjustment, vadjustment);
WRAPPER(GtkTextView2, populate_popup, void,           (void* text_view, void* menu), "pp", text_view, menu);
WRAPPER(GtkTextView2, move_cursor, void,              (void* text_view, int step, int count, int extend_selection), "piii", text_view, step, count, extend_selection);
WRAPPER(GtkTextView2, page_horizontally, void,        (void* text_view, int count, int extend_selection), "pii", text_view, count, extend_selection);
WRAPPER(GtkTextView2, set_anchor, void,               (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, insert_at_cursor, void,         (void* text_view, void* str), "pp", text_view, str);
WRAPPER(GtkTextView2, delete_from_cursor, void,       (void* text_view, int type, int count), "pii", text_view, type, count);
WRAPPER(GtkTextView2, backspace, void,                (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, cut_clipboard, void,            (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, copy_clipboard, void,           (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, paste_clipboard, void,          (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, toggle_overwrite, void,         (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, move_focus, void,               (void* text_view, int direction), "pi", text_view, direction);

#define SUPERGO()                       \
    GO(set_scroll_adjustments, vFppp);  \
    GO(populate_popup, vFpp);           \
    GO(move_cursor, vFpiii);            \
    GO(page_horizontally, vFpii);       \
    GO(set_anchor, vFp);                \
    GO(insert_at_cursor, vFpp);         \
    GO(delete_from_cursor, vFpii);      \
    GO(backspace, vFp);                 \
    GO(cut_clipboard, vFp);             \
    GO(copy_clipboard, vFp);            \
    GO(paste_clipboard, vFp);           \
    GO(toggle_overwrite, vFp);          \
    GO(move_focus, vFpi);               \

// wrap (so bridge all calls, just in case)
static void wrapGtkTextView2Class(my_GtkTextView2Class_t* class)
{
    wrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkTextView2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkTextView2Class(my_GtkTextView2Class_t* class)
{
    unwrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkTextView2 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkTextView2Class(my_GtkTextView2Class_t* class)
{
    bridgeGtkContainer2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkTextView2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkTextView3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkTextView3, populate_popup, void,           (void* text_view, void* menu), "pp", text_view, menu);
WRAPPER(GtkTextView3, move_cursor, void,              (void* text_view, int step, int count, int extend_selection), "piii", text_view, step, count, extend_selection);
WRAPPER(GtkTextView3, set_anchor, void,               (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, insert_at_cursor, void,         (void* text_view, void* str), "pp", text_view, str);
WRAPPER(GtkTextView3, delete_from_cursor, void,       (void* text_view, int type, int count), "pii", text_view, type, count);
WRAPPER(GtkTextView3, backspace, void,                (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, cut_clipboard, void,            (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, copy_clipboard, void,           (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, paste_clipboard, void,          (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, toggle_overwrite, void,         (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, create_buffer, void*,           (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, draw_layer, void,               (void* text_view, int layer, void* cr), "pip", text_view, layer, cr);
WRAPPER(GtkTextView3, extend_selection, int,          (void* text_view, int granularity, void* location, void* start, void* end), "pippp", text_view, granularity, location, start, end);
WRAPPER(GtkTextView3, insert_emoji, void,             (void* text_view), "p", text_view);

#define SUPERGO()                       \
    GO(populate_popup, vFpp);           \
    GO(move_cursor, vFpiii);            \
    GO(set_anchor, vFp);                \
    GO(insert_at_cursor, vFpp);         \
    GO(delete_from_cursor, vFpii);      \
    GO(backspace, vFp);                 \
    GO(cut_clipboard, vFp);             \
    GO(copy_clipboard, vFp);            \
    GO(paste_clipboard, vFp);           \
    GO(toggle_overwrite, vFp);          \
    GO(create_buffer, pFp);             \
    GO(draw_layer, vFpip);              \
    GO(extend_selection, iFpippp);      \
    GO(insert_emoji, vFp);              \

// wrap (so bridge all calls, just in case)
static void wrapGtkTextView3Class(my_GtkTextView3Class_t* class)
{
    wrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkTextView3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkTextView3Class(my_GtkTextView3Class_t* class)
{
    unwrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkTextView3 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkTextView3Class(my_GtkTextView3Class_t* class)
{
    bridgeGtkContainer3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkTextView3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkGrid3Class ------
// no wrapper x86 -> natives of callbacks

#define SUPERGO()           \

// wrap (so bridge all calls, just in case)
static void wrapGtkGrid3Class(my_GtkGrid3Class_t* class)
{
    wrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkGrid3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkGrid3Class(my_GtkGrid3Class_t* class)
{
    unwrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkGrid3 (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkGrid3Class(my_GtkGrid3Class_t* class)
{
    bridgeGtkContainer3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkGrid3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkEventControllerClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkEventController, set_widget, void, (void* controller, void* widget), "pp", controller, widget);
WRAPPER(GtkEventController, unset_widget, void, (void* controller), "p", controller);
WRAPPER(GtkEventController, handle_event, int, (void *controller, void *event, double x, double y), "ppdd", controller, event, x, y);
WRAPPER(GtkEventController, reset, void, (void* controller), "p", controller);
WRAPPER(GtkEventController, handle_crossing, void, (void *controller, void *crossing, double x, double y), "ppdd", controller, crossing, x, y);
WRAPPER(GtkEventController, filter_event, void, (void *controller, void *event), "pp", controller, event);

#define SUPERGO()               \
    GO(set_widget, vFpp);       \
    GO(unset_widget, vFp);      \
    GO(handle_event, iFppdd);   \
    GO(reset, vFp);             \
    GO(handle_crossing, vFppdd);\
    GO(filter_event, vFpp);

// wrap (so bridge all calls, just in case)
static void wrapGtkEventControllerClass(my_GtkEventControllerClass_t* class)
{
    wrapGObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkEventController(W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkEventControllerClass(my_GtkEventControllerClass_t* class)
{
    unwrapGObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkEventController (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkEventControllerClass(my_GtkEventControllerClass_t* class)
{
    bridgeGObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkEventController (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkGestureClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkGesture, check, void, (void* gesture), "p", gesture);
WRAPPER(GtkGesture, begin, void, (void *gesture, void *sequence), "pp", gesture, sequence);
WRAPPER(GtkGesture, update, void, (void *gesture, void *sequence), "pp", gesture, sequence);
WRAPPER(GtkGesture, end, void, (void *gesture, void *sequence), "pp", gesture, sequence);
WRAPPER(GtkGesture, cancel, void, (void *gesture, void *sequence), "pp", gesture, sequence);
WRAPPER(GtkGesture, sequence_state_changed, void, (void *gesture, void *sequence, int state), "ppi", gesture, sequence, state);

#define SUPERGO()              \
    GO(check, vFp);            \
    GO(begin, vFpp);           \
    GO(update, vFpp);          \
    GO(end, vFpp);             \
    GO(cancel, vFpp);          \
    GO(sequence_state_changed, vFppi);

// wrap (so bridge all calls, just in case)
static void wrapGtkGestureClass(my_GtkGestureClass_t* class)
{
    wrapGtkEventControllerClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkGesture(W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkGestureClass(my_GtkGestureClass_t* class)
{
    unwrapGtkEventControllerClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkGesture (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkGestureClass(my_GtkGestureClass_t* class)
{
    bridgeGtkEventControllerClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkGesture (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GtkGestureSingleClass ------
// wrap (so bridge all calls, just in case)
static void wrapGtkGestureSingleClass(my_GtkGestureSingleClass_t* class)
{
    wrapGtkGestureClass(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkGestureSingleClass(my_GtkGestureSingleClass_t* class)
{
    unwrapGtkGestureClass(&class->parent_class);
}
// autobridge
static void bridgeGtkGestureSingleClass(my_GtkGestureSingleClass_t* class)
{
    bridgeGtkGestureClass(&class->parent_class);
}

// ----- GtkGestureClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkGestureLongPress, pressed, void, (void *gesture, double x, double y), "pdd", gesture, x, y);
WRAPPER(GtkGestureLongPress, cancelled, void, (void *cancelled), "p", cancelled);

#define SUPERGO()              \
    GO(pressed, vFpdd);          \
    GO(cancelled, vFp);

// wrap (so bridge all calls, just in case)
static void wrapGtkGestureLongPressClass(my_GtkGestureLongPressClass_t* class)
{
    wrapGtkGestureSingleClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkGestureLongPress(W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkGestureLongPressClass(my_GtkGestureLongPressClass_t* class)
{
    unwrapGtkGestureSingleClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkGestureLongPress (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkGestureLongPressClass(my_GtkGestureLongPressClass_t* class)
{
    bridgeGtkGestureSingleClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkGestureLongPress (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- AtkObjectClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(AtkObject, get_name, void*, (void* accessible), "p", accessible);
WRAPPER(AtkObject, get_description, void*, (void* accessible), "p", accessible);
WRAPPER(AtkObject, get_parent, void*, (void* accessible), "p", accessible);
WRAPPER(AtkObject, get_n_children, int, (void* accessible), "p", accessible);
WRAPPER(AtkObject, ref_child, void*, (void* accessible, int i), "pi", accessible, i);
WRAPPER(AtkObject, get_index_in_parent, int, (void* accessible), "p", accessible);
WRAPPER(AtkObject, ref_relation_set, void*, (void* accessible), "p", accessible);
WRAPPER(AtkObject, get_role, int, (void* accessible), "p", accessible);
WRAPPER(AtkObject, get_layer, int, (void* accessible), "p", accessible);
WRAPPER(AtkObject, get_mdi_zorder, int, (void* accessible), "p", accessible);
WRAPPER(AtkObject, ref_state_set, void*, (void* accessible), "p", accessible);
WRAPPER(AtkObject, set_name, void, (void* accessible, void* name), "pp", accessible, name);
WRAPPER(AtkObject, set_description, void, (void* accessible, void* description), "pp", accessible, description);
WRAPPER(AtkObject, set_parent, void, (void* accessible, void* parent), "pp", accessible, parent);
WRAPPER(AtkObject, set_role, void, (void* accessible, int role), "pi", accessible, role);
WRAPPER(AtkObject, connect_property_change_handler, uint32_t, (void* accessible, void* handler), "pp", accessible, AddCheckBridge(my_bridge, vFpp, handler, 0, NULL));
WRAPPER(AtkObject, remove_property_change_handler, void, (void* accessible, uint32_t handler_id), "pu", accessible, handler_id);
WRAPPER(AtkObject, initialize, void, (void* accessible, void* data), "pp", accessible, data);
WRAPPER(AtkObject, children_changed, void, (void* accessible, uint32_t change_index, void* changed_child), "pup", accessible, change_index, changed_child);
WRAPPER(AtkObject, focus_event, void, (void* accessible, int focus_in), "pi", accessible, focus_in);
WRAPPER(AtkObject, property_change, void, (void* accessible, void* values), "pp", accessible, values);
WRAPPER(AtkObject, state_change, void, (void* accessible, void* name, int state_set), "ppi", accessible, name, state_set);
WRAPPER(AtkObject, visible_data_changed, void, (void* accessible), "p", accessible);
WRAPPER(AtkObject, active_descendant_changed, void, (void* accessible, void* child), "pp", accessible, child);
WRAPPER(AtkObject, get_attributes, void*, (void* accessible), "p", accessible);
WRAPPER(AtkObject, get_object_locale, void*, (void* accessible), "p", accessible);

#define SUPERGO() \
    GO(get_name, pFp);                          \
    GO(get_description, pFp);                   \
    GO(get_parent, pFp);                        \
    GO(get_n_children, iFp);                    \
    GO(ref_child, pFpi);                        \
    GO(get_index_in_parent, iFp);               \
    GO(ref_relation_set, pFp);                  \
    GO(get_role, iFp);                          \
    GO(get_layer, iFp);                         \
    GO(get_mdi_zorder, iFp);                    \
    GO(ref_state_set, pFp);                     \
    GO(set_name, vFpp);                         \
    GO(set_description, vFpp);                  \
    GO(set_parent, vFpp);                       \
    GO(set_role, vFpi);                         \
    GO(connect_property_change_handler, uFpp);  \
    GO(remove_property_change_handler, vFpu);   \
    GO(initialize, vFpp);                       \
    GO(children_changed, vFpup);                \
    GO(focus_event, vFpi);                      \
    GO(property_change, vFpp);                  \
    GO(state_change, vFppi);                    \
    GO(visible_data_changed, vFp);              \
    GO(active_descendant_changed, vFpp);        \
    GO(get_attributes, pFp);                    \
    GO(get_object_locale, pFp);                 \

// wrap (so bridge all calls, just in case)
static void wrapAtkObjectClass(my_AtkObjectClass_t* class)
{
    wrapGObjectClass(&class->parent);
    #define GO(A, W) class->A = reverse_##A##_AtkObject (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapAtkObjectClass(my_AtkObjectClass_t* class)
{
    unwrapGObjectClass(&class->parent);
    #define GO(A, W)   class->A = find_##A##_AtkObject (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeAtkObjectClass(my_AtkObjectClass_t* class)
{
    bridgeGObjectClass(&class->parent);
    #define GO(A, W) autobridge_##A##_AtkObject (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- AtkUtilClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(AtkUtil,add_global_event_listener, uint32_t,(void* listener, void* event_type), "pp", AddCheckBridge(my_bridge, iFpupp, listener, 0, NULL), event_type);
WRAPPER(AtkUtil,remove_global_event_listener, void ,(uint32_t listener_id), "u", listener_id);
WRAPPER(AtkUtil,add_key_event_listener, uint32_t   ,(void* listener, void* data), "pp", AddCheckBridge(my_bridge, iFpp, listener, 0, NULL), data);
WRAPPER(AtkUtil,remove_key_event_listener, void    ,(uint32_t listener_id), "u", listener_id);
WRAPPER(AtkUtil,get_root, void*                    ,(void), "", 0);
WRAPPER(AtkUtil,get_toolkit_name, void*            ,(void), "", 0);
WRAPPER(AtkUtil,get_toolkit_version, void*         ,(void), "", 0);

#define SUPERGO() \
    GO(add_global_event_listener, uFpp);        \
    GO(remove_global_event_listener, vFu);      \
    GO(add_key_event_listener, uFpp);           \
    GO(remove_key_event_listener, vFu);         \
    GO(get_root, pFv);                          \
    GO(get_toolkit_name, pFv);                  \
    GO(get_toolkit_version, pFv);               \

// wrap (so bridge all calls, just in case)
static void wrapAtkUtilClass(my_AtkUtilClass_t* class)
{
    wrapGObjectClass(&class->parent);
    #define GO(A, W) class->A = reverse_##A##_AtkUtil (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapAtkUtilClass(my_AtkUtilClass_t* class)
{
    unwrapGObjectClass(&class->parent);
    #define GO(A, W)   class->A = find_##A##_AtkUtil (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeAtkUtilClass(my_AtkUtilClass_t* class)
{
    bridgeGObjectClass(&class->parent);
    #define GO(A, W) autobridge_##A##_AtkUtil (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GstObjectClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstObject, deep_notify, void, (void* object, void* origin, void* pspec), "ppp", object, origin, pspec);

#define SUPERGO() \
    GO(deep_notify, vFppp); \

// wrap (so bridge all calls, just in case)
static void wrapGstObjectClass(my_GstObjectClass_t* class)
{
    wrapGInitiallyUnownedClass(&class->parent);
    #define GO(A, W) class->A = reverse_##A##_GstObject (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstObjectClass(my_GstObjectClass_t* class)
{
    unwrapGInitiallyUnownedClass(&class->parent);
    #define GO(A, W)   class->A = find_##A##_GstObject (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstObjectClass(my_GstObjectClass_t* class)
{
    bridgeGInitiallyUnownedClass(&class->parent);
    #define GO(A, W) autobridge_##A##_GstObject (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GstAllocatorClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstAllocator, alloc, void*, (void *allocator, size_t size, void *params), "pLp", allocator, size, params);
WRAPPER(GstAllocator,free, void,    (void *allocator, void *memory), "pp", allocator, memory);

#define SUPERGO() \
    GO(alloc, pFpLp);       \
    GO(free, vFpp);         \

// wrap (so bridge all calls, just in case)
static void wrapGstAllocatorClass(my_GstAllocatorClass_t* class)
{
    wrapGstObjectClass(&class->parent);
    #define GO(A, W) class->A = reverse_##A##_GstAllocator (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstAllocatorClass(my_GstAllocatorClass_t* class)
{
    unwrapGstObjectClass(&class->parent);
    #define GO(A, W)   class->A = find_##A##_GstAllocator (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstAllocatorClass(my_GstAllocatorClass_t* class)
{
    bridgeGstObjectClass(&class->parent);
    #define GO(A, W) autobridge_##A##_GstAllocator (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- GstTaskPoolClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstTaskPool, prepare, void,  (void* pool, void* error), "pp", pool, error);
WRAPPER(GstTaskPool, cleanup, void,  (void* pool), "p", pool);
WRAPPER(GstTaskPool, push, void*,    (void* pool, void* func, void* user_data, void* error), "pppp", pool, AddCheckBridge(my_bridge, vFp, func, 0, NULL), user_data, error);
WRAPPER(GstTaskPool, join, void,     (void* pool, void* id), "pp", pool, id);
WRAPPER(GstTaskPool, dispose_handle, void, (void* pool, void* id), "pp", pool, id);

#define SUPERGO()               \
    GO(prepare, vFpp);          \
    GO(cleanup, vFp);           \
    GO(push, pFpppp);           \
    GO(join, vFpp);             \
    GO(dispose_handle, vFpp);   \

// wrap (so bridge all calls, just in case)
static void wrapGstTaskPoolClass(my_GstTaskPoolClass_t* class)
{
    wrapGstObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstTaskPool (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstTaskPoolClass(my_GstTaskPoolClass_t* class)
{
    unwrapGstObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstTaskPool (class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstTaskPoolClass(my_GstTaskPoolClass_t* class)
{
    bridgeGstObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstTaskPool (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO


// No more wrap/unwrap
#undef WRAPPER
#undef FIND
#undef REVERSE
#undef WRAPPED

// g_type_class_peek_parent
static void wrapGTKClass(void* cl, size_t type)
{
    #define GTKCLASS(A)                             \
    if(type==my_##A)                                \
        wrap##A##Class((my_##A##Class_t*)cl);       \
    else

    printf_log(LOG_DEBUG, "wrapGTKClass(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    {
        if(my_MetaFrames2==-1 && !strcmp(g_type_name(type), "MetaFrames")) {
            my_MetaFrames2 = type;
            wrapMetaFrames2Class((my_MetaFrames2Class_t*)cl);
        } else
            printf_log(LOG_NONE, "Warning, Custom Class initializer with unknown class type %zd (%s)\n", type, g_type_name(type));
    }
    #undef GTKCLASS
}

static void unwrapGTKClass(void* cl, size_t type)
{
    #define GTKCLASS(A)                             \
    if(type==my_##A)                                \
        unwrap##A##Class((my_##A##Class_t*)cl);     \
    else

    printf_log(LOG_DEBUG, "unwrapGTKClass(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    {}  // else no warning, one is enough...
    #undef GTKCLASS
}

static void bridgeGTKClass(void* cl, size_t type)
{
    #define GTKCLASS(A)                             \
    if(type==my_##A)                                \
        bridge##A##Class((my_##A##Class_t*)cl);     \
    else

    printf_log(LOG_DEBUG, "bridgeGTKClass(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    {
        printf_log(LOG_NONE, "Warning, AutoBridge GTK Class with unknown class type %zd (%s)\n", type, g_type_name(type));
    }
    #undef GTKCLASS
}


typedef union my_GClassAll_s {
    #define GTKCLASS(A) my_##A##Class_t A;
    GTKCLASSES()
    #undef GTKCLASS
} my_GClassAll_t;

#define GO(A) \
static void* my_gclassall_ref_##A = NULL;   \
static my_GClassAll_t my_gclassall_##A;

SUPER()
#undef GO

void* unwrapCopyGTKClass(void* klass, size_t type)
{
    if(!klass) return klass;
    #define GO(A) if(klass == my_gclassall_ref_##A) return &my_gclassall_##A;
    SUPER()
    #undef GO
    // check if class is the exact type we know
    size_t sz = 0;
    #define GTKCLASS(A) if(type==my_##A) sz = sizeof(my_##A##Class_t); else
    GTKCLASSES()
    {
        printf_log(LOG_NONE, "Warning, unwrapCopyGTKClass called with unknown class type %zu (%s)\n", type, g_type_name(type));
        return klass;
    }
    #undef GTKCLASS
    my_GClassAll_t *newklass = NULL;
    #define GO(A) if(!newklass && !my_gclassall_ref_##A) {my_gclassall_ref_##A = klass; newklass = &my_gclassall_##A;}
    SUPER()
    #undef GO
    if(!newklass) {
        printf_log(LOG_NONE, "Warning: no more slot for unwrapCopyGTKClass\n");
        return klass;
    }
    memcpy(newklass, klass, sz);
    unwrapGTKClass(newklass, type);
    return newklass;
}

// gtk_type_class

#define GO(A) \
static void* my_gclassallu_ref_##A = NULL;   \
static my_GClassAll_t my_gclassallu_##A;

SUPER()
#undef GO
void* wrapCopyGTKClass(void* klass, size_t type)
{
    if(!klass) return klass;
    printf_log(LOG_DEBUG, "wrapCopyGTKClass(%p, %zd (%s))\n", klass, type, g_type_name(type));
    #define GO(A) if(klass == my_gclassallu_ref_##A) return &my_gclassallu_##A;
    SUPER()
    #undef GO
    // check if class is the exact type we know
    int sz = 0;
    #define GTKCLASS(A) if(type==my_##A) sz = sizeof(my_##A##Class_t); else
    GTKCLASSES()
    {
        if(my_MetaFrames2==-1 && !strcmp(g_type_name(type), "MetaFrames")) {
            my_MetaFrames2 = type;
            sz = sizeof(my_MetaFrames2Class_t);
        } else {
            printf_log(LOG_NONE, "Warning, wrapCopyGTKClass called with unknown class type 0x%zx (%s)\n", type, g_type_name(type));
            return klass;
        }
    }
    #undef GTKCLASS
    my_GClassAll_t *newklass = NULL;
    #define GO(A) if(!newklass && !my_gclassallu_ref_##A) {my_gclassallu_ref_##A = klass; newklass = &my_gclassallu_##A;}
    SUPER()
    #undef GO
    if(!newklass) {
        printf_log(LOG_NONE, "Warning: no more slot for wrapCopyGTKClass\n");
        return klass;
    }
    memcpy(newklass, klass, sz);
    //wrapGTKClass(newklass, type);
    bridgeGTKClass(newklass, type);
    return newklass;
}

// ---- GTypeValueTable ----

// First the structure GTypeInfo statics, with paired x64 source pointer
#define GO(A) \
static my_GTypeValueTable_t     my_gtypevaluetable_##A = {0};   \
static my_GTypeValueTable_t   *ref_gtypevaluetable_##A = NULL;
SUPER()
#undef GO
// Then the static functions callback that may be used with the structure
// value_init ...
#define GO(A)   \
static uintptr_t my_value_init_fct_##A = 0;                     \
static void my_value_init_##A(void* a)                          \
{                                                               \
    RunFunctionFmt(my_value_init_fct_##A, "p", a);  \
}
SUPER()
#undef GO
static void* find_value_init_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_value_init_fct_##A == (uintptr_t)fct) return my_value_init_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_value_init_fct_##A == 0) {my_value_init_fct_##A = (uintptr_t)fct; return my_value_init_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo value_init callback\n");
    return NULL;
}
// value_free ...
#define GO(A)   \
static uintptr_t my_value_free_fct_##A = 0;                     \
static void my_value_free_##A(void* a)                          \
{                                                               \
    RunFunctionFmt(my_value_free_fct_##A, "p", a);  \
}
SUPER()
#undef GO
static void* find_value_free_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_value_free_fct_##A == (uintptr_t)fct) return my_value_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_value_free_fct_##A == 0) {my_value_free_fct_##A = (uintptr_t)fct; return my_value_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo value_free callback\n");
    return NULL;
}
// value_copy ...
#define GO(A)   \
static uintptr_t my_value_copy_fct_##A = 0;                     \
static void my_value_copy_##A(void* a, void* b)                 \
{                                                               \
    RunFunctionFmt(my_value_copy_fct_##A, "pp", a, b); \
}
SUPER()
#undef GO
static void* find_value_copy_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_value_copy_fct_##A == (uintptr_t)fct) return my_value_copy_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_value_copy_fct_##A == 0) {my_value_copy_fct_##A = (uintptr_t)fct; return my_value_copy_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo value_copy callback\n");
    return NULL;
}
// value_peek_pointer ...
#define GO(A)   \
static uintptr_t my_value_peek_pointer_fct_##A = 0;                                 \
static void* my_value_peek_pointer_##A(void* a)                                     \
{                                                                                   \
    return (void*)RunFunctionFmt(my_value_peek_pointer_fct_##A, "p", a);\
}
SUPER()
#undef GO
static void* find_value_peek_pointer_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_value_peek_pointer_fct_##A == (uintptr_t)fct) return my_value_peek_pointer_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_value_peek_pointer_fct_##A == 0) {my_value_peek_pointer_fct_##A = (uintptr_t)fct; return my_value_peek_pointer_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo value_peek_pointer callback\n");
    return NULL;
}
// collect_value ...
#define GO(A)   \
static uintptr_t my_collect_value_fct_##A = 0;                                              \
static void* my_collect_value_##A(void* a, uint32_t b, void* c, uint32_t d)                 \
{                                                                                           \
    return (void*)RunFunctionFmt(my_collect_value_fct_##A, "pupu", a, b, c, d); \
}
SUPER()
#undef GO
static void* find_collect_value_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_collect_value_fct_##A == (uintptr_t)fct) return my_collect_value_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_collect_value_fct_##A == 0) {my_collect_value_fct_##A = (uintptr_t)fct; return my_collect_value_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo collect_value callback\n");
    return NULL;
}
// lcopy_value ...
#define GO(A)   \
static uintptr_t my_lcopy_value_fct_##A = 0;                                                \
static void* my_lcopy_value_##A(void* a, uint32_t b, void* c, uint32_t d)                   \
{                                                                                           \
    return (void*)RunFunctionFmt(my_lcopy_value_fct_##A, "pupu", a, b, c, d);   \
}
SUPER()
#undef GO
static void* find_lcopy_value_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_lcopy_value_fct_##A == (uintptr_t)fct) return my_lcopy_value_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_lcopy_value_fct_##A == 0) {my_lcopy_value_fct_##A = (uintptr_t)fct; return my_lcopy_value_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo lcopy_value callback\n");
    return NULL;
}
// And now the get slot / assign... Taking into account that the desired callback may already be a wrapped one (so unwrapping it)
my_GTypeValueTable_t* findFreeGTypeValueTable(my_GTypeValueTable_t* fcts)
{
    if(!fcts) return fcts;
    #define GO(A) if(ref_gtypevaluetable_##A == fcts) return &my_gtypevaluetable_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_gtypevaluetable_##A == 0) {                                                        \
        ref_gtypevaluetable_##A = fcts;                                                                     \
        my_gtypevaluetable_##A.value_init = find_value_init_Fct(fcts->value_init);                          \
        my_gtypevaluetable_##A.value_free = find_value_free_Fct(fcts->value_free);                          \
        my_gtypevaluetable_##A.value_copy = find_value_copy_Fct(fcts->value_copy);                          \
        my_gtypevaluetable_##A.value_peek_pointer = find_value_peek_pointer_Fct(fcts->value_peek_pointer);  \
        my_gtypevaluetable_##A.collect_format = fcts->collect_format;                                       \
        my_gtypevaluetable_##A.collect_value = find_collect_value_Fct(fcts->collect_value);                 \
        my_gtypevaluetable_##A.lcopy_format = fcts->lcopy_format;                                           \
        my_gtypevaluetable_##A.lcopy_value = find_lcopy_value_Fct(fcts->lcopy_value);                       \
        return &my_gtypevaluetable_##A;                                                                     \
    }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeValueTable callback\n");
    return NULL;
}

// signal2 ...
#define GO(A)   \
static uintptr_t my_signal2_fct_##A = 0;                                        \
static void* my_signal2_##A(void* a, void* b)                                   \
{                                                                               \
    return (void*)RunFunctionFmt(my_signal2_fct_##A, "pp", a, b);   \
}
SUPER()
#undef GO
static void* find_signal2_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal2_fct_##A == (uintptr_t)fct) return my_signal2_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal2_fct_##A == 0) {my_signal2_fct_##A = (uintptr_t)fct; return my_signal2_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal2 callback\n");
    return NULL;
}
// signal3 ...
#define GO(A)   \
static uintptr_t my_signal3_fct_##A = 0;                                            \
static void* my_signal3_##A(void* a, void* b, void* c)                              \
{                                                                                   \
    return (void*)RunFunctionFmt(my_signal3_fct_##A, "ppp", a, b, c);   \
}
SUPER()
#undef GO
static void* find_signal3_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal3_fct_##A == (uintptr_t)fct) return my_signal3_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal3_fct_##A == 0) {my_signal3_fct_##A = (uintptr_t)fct; return my_signal3_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal3 callback\n");
    return NULL;
}
// signal4 ...
#define GO(A)   \
static uintptr_t my_signal4_fct_##A = 0;                                                \
static void* my_signal4_##A(void* a, void* b, void* c, void* d)                         \
{                                                                                       \
    return (void*)RunFunctionFmt(my_signal4_fct_##A, "pppp", a, b, c, d);   \
}
SUPER()
#undef GO
static void* find_signal4_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal4_fct_##A == (uintptr_t)fct) return my_signal4_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal4_fct_##A == 0) {my_signal4_fct_##A = (uintptr_t)fct; return my_signal4_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal4 callback\n");
    return NULL;
}
// signal5 ...
#define GO(A)   \
static uintptr_t my_signal5_fct_##A = 0;                                                    \
static void* my_signal5_##A(void* a, void* b, void* c, void* d, void* e)                    \
{                                                                                           \
    return (void*)RunFunctionFmt(my_signal5_fct_##A, "ppppp", a, b, c, d, e);   \
}
SUPER()
#undef GO
static void* find_signal5_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal5_fct_##A == (uintptr_t)fct) return my_signal5_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal5_fct_##A == 0) {my_signal5_fct_##A = (uintptr_t)fct; return my_signal5_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal5 callback\n");
    return NULL;
}
// signal6 ...
#define GO(A)   \
static uintptr_t my_signal6_fct_##A = 0;                                                        \
static void* my_signal6_##A(void* a, void* b, void* c, void* d, void* e, void* f)               \
{                                                                                               \
    return (void*)RunFunctionFmt(my_signal6_fct_##A, "pppppp", a, b, c, d, e, f);   \
}
SUPER()
#undef GO
static void* find_signal6_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal6_fct_##A == (uintptr_t)fct) return my_signal6_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal6_fct_##A == 0) {my_signal6_fct_##A = (uintptr_t)fct; return my_signal6_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal6 callback\n");
    return NULL;
}
// signal7 ...
#define GO(A)   \
static uintptr_t my_signal7_fct_##A = 0;                                                            \
static void* my_signal7_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g)          \
{                                                                                                   \
    return (void*)RunFunctionFmt(my_signal7_fct_##A, "ppppppp", a, b, c, d, e, f, g);   \
}
SUPER()
#undef GO
static void* find_signal7_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal7_fct_##A == (uintptr_t)fct) return my_signal7_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal7_fct_##A == 0) {my_signal7_fct_##A = (uintptr_t)fct; return my_signal7_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal7 callback\n");
    return NULL;
}
static const wrapper_t wrappers[] = {pFpp, pFppp, pFpppp, pFpppp, pFppppp, pFpppppp};
typedef void* (*finder_t)(void*);
static const finder_t finders[] = {find_signal2_Fct, find_signal3_Fct, find_signal4_Fct, find_signal5_Fct, find_signal6_Fct, find_signal7_Fct};
#define MAX_SIGNAL_N 7

// ---- GTypeInfo ----
// let's handle signal with offset, that are used to wrap custom signal function
void my_unwrap_signal_offset(void* klass);
void my_add_signal_offset(size_t itype, uint32_t offset, int n)
{
    printf_log(LOG_DEBUG, "my_add_signal_offset(0x%zx, %d, %d)\n", itype, offset, n);
    if(!offset || !itype) // no offset means no overload...
        return;
    if(n<0 || n>MAX_SIGNAL_N) {
        printf_log(LOG_NONE, "Warning, signal with too many args (%d) in my_add_signal_offset\n", n);
        return;
    }
    int ret;
    khint_t k = kh_put(sigoffset, my_sigoffset, itype, &ret);
    sigoffset_array_t *p = &kh_value(my_sigoffset, k);
    if(ret) {
        p->a = NULL; p->cap = 0; p->sz = 0;
    }
    // check if offset already there
    for(int i=0; i<p->sz; ++i)
        if(p->a[i].offset == offset) {
            printf_log(LOG_INFO, "Offset already there... Bye\n");
            return; // already there, bye
        }
    if(p->sz==p->cap) {
        p->cap+=4;
        p->a = (sigoffset_t*)box_realloc(p->a, sizeof(sigoffset_t)*p->cap);
    }
    p->a[p->sz].offset = offset;
    p->a[p->sz++].n = n;
}
void my_unwrap_signal_offset(void* klass)
{
    if(!klass)
        return;
    size_t itype = *(size_t*)klass;
    khint_t k = kh_get(sigoffset, my_sigoffset, itype);
    if(k==kh_end(my_sigoffset))
        return;
    sigoffset_array_t *p = &kh_value(my_sigoffset, k);
    printf_log(LOG_DEBUG, "my_unwrap_signal_offset(%p) type=0x%zx with %d signals with offset\n", klass, itype, p->sz);
    for(int i=0; i<p->sz; ++i) {
        void** f = (void**)((uintptr_t)klass + p->a[i].offset);
        if(!GetNativeFnc((uintptr_t)*f)) {
            // Not a native function: autobridge it
            void* new_f = finders[p->a[i].n-2](f);
            printf_log(LOG_DEBUG, "Unwrapping %p -> %p (with alternate)\n", *f, new_f);
            if(!hasAlternate(new_f))
                addAlternate(new_f, *f);
            *f = new_f;
        }
    }
}

// First the structure my_GTypeInfo_t statics, with paired x64 source pointer
#define GO(A) \
static my_GTypeInfo_t     my_gtypeinfo_##A = {0};   \
static my_GTypeInfo_t    ref_gtypeinfo_##A = {0};   \
static int              used_gtypeinfo_##A = 0;
SUPER()
#undef GO
// Then the static functions callback that may be used with the structure
// base_init ...
#define GO(A)   \
static uintptr_t my_base_init_fct_##A = 0;                          \
static int my_base_init_##A(void* a)                                \
{                                                                   \
    return RunFunctionFmt(my_base_init_fct_##A, "p", a);     \
}
SUPER()
#undef GO
static void* find_base_init_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_base_init_fct_##A == (uintptr_t)fct) return my_base_init_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_base_init_fct_##A == 0) {my_base_init_fct_##A = (uintptr_t)fct; return my_base_init_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo base_init callback\n");
    return NULL;
}
// base_finalize ...
#define GO(A)   \
static uintptr_t my_base_finalize_fct_##A = 0;                      \
static int my_base_finalize_##A(void* a)                            \
{                                                                   \
    return RunFunctionFmt(my_base_finalize_fct_##A, "p", a); \
}
SUPER()
#undef GO
static void* find_base_finalize_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_base_finalize_fct_##A == (uintptr_t)fct) return my_base_finalize_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_base_finalize_fct_##A == 0) {my_base_finalize_fct_##A = (uintptr_t)fct; return my_base_finalize_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo base_finalize callback\n");
    return NULL;
}
// class_init ...
#define GO(A)   \
static uintptr_t my_class_init_fct_##A = 0;                                 \
static size_t parent_class_init_##A = 0;                                    \
static int my_class_init_##A(void* a, void* b)                              \
{                                                                           \
    printf_log(LOG_DEBUG, "Custom Class init %d for class %p (parent=%p:%s)\n", A, a, (void*)parent_class_init_##A, g_type_name(parent_class_init_##A));\
    int ret = RunFunctionFmt(my_class_init_fct_##A, "pp", a, b);\
    unwrapGTKClass(a, parent_class_init_##A);                               \
    bridgeGTKClass(a, parent_class_init_##A);                               \
    my_unwrap_signal_offset(a);                                             \
    if(!strcmp(g_type_name(parent_class_init_##A), "AtkUtil")) {            \
        my_AtkUtilClass_t* p = (my_AtkUtilClass_t*)g_type_class_peek(parent_class_init_##A);\
        unwrapGTKClass(p, parent_class_init_##A);                           \
        bridgeGTKClass(p, parent_class_init_##A);                           \
    }                                                                       \
    return ret;                                                             \
}
SUPER()
#undef GO
static void* find_class_init_Fct(void* fct, size_t parent)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_class_init_fct_##A == (uintptr_t)fct && parent_class_init_##A==parent) return my_class_init_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_class_init_fct_##A == 0) {my_class_init_fct_##A = (uintptr_t)fct; parent_class_init_##A=parent; return my_class_init_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo class_init callback\n");
    return NULL;
}
// class_finalize ...
#define GO(A)   \
static uintptr_t my_class_finalize_fct_##A = 0;                                 \
static int my_class_finalize_##A(void* a, void* b)                              \
{                                                                               \
    return RunFunctionFmt(my_class_finalize_fct_##A, "pp", a, b);   \
}
SUPER()
#undef GO
static void* find_class_finalize_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_class_finalize_fct_##A == (uintptr_t)fct) return my_class_finalize_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_class_finalize_fct_##A == 0) {my_class_finalize_fct_##A = (uintptr_t)fct; return my_class_finalize_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo class_finalize callback\n");
    return NULL;
}
// instance_init ...
#define GO(A)   \
static uintptr_t my_instance_init_fct_##A = 0;                              \
static int my_instance_init_##A(void* a, void* b)                           \
{                                                                           \
    return RunFunctionFmt(my_instance_init_fct_##A, "pp", a, b);\
}
SUPER()
#undef GO
static void* find_instance_init_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_instance_init_fct_##A == (uintptr_t)fct) return my_instance_init_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_instance_init_fct_##A == 0) {my_instance_init_fct_##A = (uintptr_t)fct; return my_instance_init_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo instance_init callback\n");
    return NULL;
}
// And now the get slot / assign... Taking into account that the desired callback may already be a wrapped one (so unwrapping it)
my_GTypeInfo_t* findFreeGTypeInfo(my_GTypeInfo_t* fcts, size_t parent)
{
    if(!fcts) return NULL;
    #define GO(A) if(used_gtypeinfo_##A && memcmp(&ref_gtypeinfo_##A, fcts, sizeof(my_GTypeInfo_t))==0) return &my_gtypeinfo_##A;
    SUPER()
    #undef GO
    #define GO(A) if(used_gtypeinfo_##A == 0) {                                         \
        used_gtypeinfo_##A = 1;                                                         \
        memcpy(&ref_gtypeinfo_##A, fcts, sizeof(my_GTypeInfo_t));                       \
        my_gtypeinfo_##A.class_size = fcts->class_size;                                 \
        my_gtypeinfo_##A.base_init = find_base_init_Fct(fcts->base_init);               \
        my_gtypeinfo_##A.base_finalize = find_base_finalize_Fct(fcts->base_finalize);   \
        my_gtypeinfo_##A.class_init = find_class_init_Fct(fcts->class_init, parent);    \
        my_gtypeinfo_##A.class_finalize = find_class_finalize_Fct(fcts->class_finalize);\
        my_gtypeinfo_##A.class_data = fcts->class_data;                                 \
        my_gtypeinfo_##A.instance_size = fcts->instance_size;                           \
        my_gtypeinfo_##A.n_preallocs = fcts->n_preallocs;                               \
        my_gtypeinfo_##A.instance_init = find_instance_init_Fct(fcts->instance_init);   \
        my_gtypeinfo_##A.value_table = findFreeGTypeValueTable(fcts->value_table);      \
        return &my_gtypeinfo_##A;                                                       \
    }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo callback\n");
    return NULL;
}

// ---- GtkTypeInfo ----

// First the structure my_GtkTypeInfo_t statics, with paired x64 source pointer
#define GO(A) \
static my_GtkTypeInfo_t     my_gtktypeinfo_##A = {0};   \
static my_GtkTypeInfo_t    ref_gtktypeinfo_##A = {0};  \
static int                used_gtktypeinfo_##A = 0;
SUPER()
#undef GO
// Then the static functions callback that may be used with the structure
#define GO(A)   \
static int fct_gtk_parent_##A = 0 ;                 \
static uintptr_t fct_gtk_class_init_##A = 0;        \
static int my_gtk_class_init_##A(void* g_class) {   \
    printf_log(LOG_DEBUG, "Calling fct_gtk_class_init_" #A " wrapper\n");           \
    int ret = (int)RunFunctionFmt(fct_gtk_class_init_##A, "p", g_class);\
    unwrapGTKClass(g_class, fct_gtk_parent_##A);                                    \
    bridgeGTKClass(g_class, fct_gtk_parent_##A);                                    \
    return ret;                                                                     \
}   \
static uintptr_t fct_gtk_object_init_##A = 0;                                           \
static int my_gtk_object_init_##A(void* object, void* data) {                           \
    printf_log(LOG_DEBUG, "Calling fct_gtk_object_init_" #A " wrapper\n");              \
    return (int)RunFunctionFmt(fct_gtk_object_init_##A, "pp", object, data);\
}   \
static uintptr_t fct_gtk_base_class_init_##A = 0;                                               \
static int my_gtk_base_class_init_##A(void* instance, void* data) {                             \
    printf_log(LOG_DEBUG, "Calling fct_gtk_base_class_init_" #A " wrapper\n");                  \
    return (int)RunFunctionFmt(fct_gtk_base_class_init_##A, "pp", instance, data);  \
}

SUPER()
#undef GO
// And now the get slot / assign... Taking into account that the desired callback may already be a wrapped one (so unwrapping it)
my_GtkTypeInfo_t* findFreeGtkTypeInfo(my_GtkTypeInfo_t* fcts, size_t parent)
{
    if(!fcts) return NULL;
    #define GO(A) if(used_gtktypeinfo_##A && memcmp(&ref_gtktypeinfo_##A, fcts, sizeof(my_GtkTypeInfo_t))==0) return &my_gtktypeinfo_##A;
    SUPER()
    #undef GO
    #define GO(A) if(used_gtktypeinfo_##A == 0) {          \
        memcpy(&ref_gtktypeinfo_##A, fcts, sizeof(my_GtkTypeInfo_t));        \
        fct_gtk_parent_##A = parent;                        \
        my_gtktypeinfo_##A.type_name = fcts->type_name; \
        my_gtktypeinfo_##A.object_size = fcts->object_size; \
        my_gtktypeinfo_##A.class_size = fcts->class_size; \
        my_gtktypeinfo_##A.class_init_func = (fcts->class_init_func)?((GetNativeFnc((uintptr_t)fcts->class_init_func))?GetNativeFnc((uintptr_t)fcts->class_init_func):my_gtk_class_init_##A):NULL;    \
        fct_gtk_class_init_##A = (uintptr_t)fcts->class_init_func;           \
        my_gtktypeinfo_##A.object_init_func = (fcts->object_init_func)?((GetNativeFnc((uintptr_t)fcts->object_init_func))?GetNativeFnc((uintptr_t)fcts->object_init_func):my_gtk_object_init_##A):NULL;    \
        fct_gtk_object_init_##A = (uintptr_t)fcts->object_init_func;         \
        my_gtktypeinfo_##A.reserved_1 = fcts->reserved_1;                 \
        my_gtktypeinfo_##A.reserved_2 = fcts->reserved_2;                 \
        my_gtktypeinfo_##A.base_class_init_func = (fcts->base_class_init_func)?((GetNativeFnc((uintptr_t)fcts->base_class_init_func))?GetNativeFnc((uintptr_t)fcts->base_class_init_func):my_gtk_base_class_init_##A):NULL;    \
        fct_gtk_base_class_init_##A = (uintptr_t)fcts->base_class_init_func;   \
        return &my_gtktypeinfo_##A;                       \
    }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GtkTypeInfo callback\n");
    return NULL;
}

#undef SUPER

void InitGTKClass(bridge_t *bridge)
{
    my_bridge  = bridge;
    my_signalmap = kh_init(signalmap);
    my_sigoffset = kh_init(sigoffset);
}

void FiniGTKClass()
{
    if(my_signalmap) {
        /*khint_t k;
        kh_foreach_key(my_signalmap, k,
            my_signal_t* p = (my_signal_t*)(uintptr_t)k;
            box_free(p);
        );*/ // lets assume all signals data is freed by gtk already
        kh_destroy(signalmap, my_signalmap);
        my_signalmap = NULL;
    }
    if(my_sigoffset) {
        sigoffset_array_t* p;
        kh_foreach_value_ref(my_sigoffset, p,
            box_free(p->a);
        );
        kh_destroy(sigoffset, my_sigoffset);
        my_sigoffset = NULL;
    }
}

#define GTKCLASS(A)             \
void Set##A##ID(size_t id)      \
{                               \
    my_##A = id;                \
}
GTKCLASSES()
#undef GTKCLASS

void AutoBridgeGtk(void*(*ref)(size_t), void(*unref)(void*))
{
    void* p;
    #define GTKCLASS(A)             \
    if(my_##A && my_##A!=-1) {      \
        p = ref(my_##A);            \
        bridgeGTKClass(p, my_##A);  \
        unref(p);                   \
    }
    GTKCLASSES()
    #undef GTKCLASS
}

void SetGTypeName(void* f)
{
    g_type_name = f;
}

void SetGClassPeek(void* f)
{
    g_type_class_peek = f;
}

my_signal_t* new_mysignal(void* f, void* data, void* destroy)
{
    my_signal_t* sig = (my_signal_t*)box_calloc(1, sizeof(my_signal_t));
    sig->sign = SIGN;
    sig->c_handler = (uintptr_t)f;
    sig->destroy = (uintptr_t)destroy;
    sig->data = data;
    int ret;
    kh_put(signalmap, my_signalmap, (uintptr_t)sig, &ret);
    return sig;
}
void my_signal_delete(my_signal_t* sig)
{
    khint_t k = kh_get(signalmap, my_signalmap, (uintptr_t)sig);
    if(k!=kh_end(my_signalmap)) {
        kh_del(signalmap, my_signalmap, k);
    } else {
        printf_log(LOG_NONE, "Warning, my_signal_delete called with an unrefereced signal!\n");
    }
    uintptr_t d = sig->destroy;
    if(d) {
        RunFunctionFmt(d, "p", sig->data);
    }
    printf_log(LOG_DEBUG, "gtk Data deleted, sig=%p, data=%p, destroy=%p\n", sig, sig->data, (void*)d);
    box_free(sig);
}
int my_signal_is_valid(void* sig)
{
    khint_t k = kh_get(signalmap, my_signalmap, (uintptr_t)sig);
    if(k!=kh_end(my_signalmap)) {
        /*if(((my_signal_t*)c)->sign == SIGN)
            return 1;
        else
            printf_log(LOG_NONE, "Warning, incohrent my_signal_t structure referenced\n");*/
        return 1;
    }
    return 0;
}

int my_signal_cb(void* a, void* b, void* c, void* d)
{
    // signal can have many signature... so first job is to find the data!
    // hopefully, no callback have more than 4 arguments...
    my_signal_t* sig = NULL;
    int i = 0;
    if(my_signal_is_valid(a)) {
        sig = (my_signal_t*)a;
        i = 1;
    }
    if(!sig && my_signal_is_valid(b)) {
        sig = (my_signal_t*)b;
        i = 2;
    }
    if(!sig && my_signal_is_valid(c)) {
        sig = (my_signal_t*)c;
        i = 3;
    }
    if(!sig && my_signal_is_valid(d)) {
        sig = (my_signal_t*)d;
        i = 4;
    }
    printf_log(LOG_DEBUG, "gtk Signal called, sig=%p, NArgs=%d\n", sig, i);
    switch(i) {
        case 1: return (int)RunFunctionFmt(sig->c_handler, "p", sig->data);
        case 2: return (int)RunFunctionFmt(sig->c_handler, "pp", a, sig->data);
        case 3: return (int)RunFunctionFmt(sig->c_handler, "ppp", a, b, sig->data);
        case 4: return (int)RunFunctionFmt(sig->c_handler, "pppp", a, b, c, sig->data);
    }
    printf_log(LOG_NONE, "Warning, Gtk signal callback but no data found!");
    return 0;
}
