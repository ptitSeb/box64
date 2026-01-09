#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "gtkclass.h"

const char* gtk3Name = "libgtk-3.so.0";
#define ALTNAME "libgtk-3.so"

#define LIBNAME gtk3

typedef size_t        (*LFv_t)(void);
typedef void          (*vFp_t)(void*);
typedef double        (*dFp_t)(void*);
typedef void*         (*pFL_t)(size_t);
typedef void*         (*pFpp_t)(void*, void*);
typedef void*         (*pFup_t)(uint32_t, void*);
typedef void*         (*pFpu_t)(void*, uint32_t);
typedef void*         (*pFppi_t)(void*, void*, int);
typedef void*         (*pFppp_t)(void*, void*, void*);
typedef int           (*iFppp_t)(void*, void*, void*);
typedef void          (*vFppp_t)(void*, void*, void*);
typedef void          (*vFppA_t)(void*, void*, va_list);
typedef void          (*vFpipV_t)(void*, int, void*, ...);
typedef void          (*vFppippi_t)(void*, void*, int, void*, void*, int);
typedef void (*vFppip_t)(void*, void*, int, void*);

#define ADDED_FUNCTIONS()                              \
    GO(g_type_class_ref, pFL_t)                        \
    GO(g_type_class_unref, vFp_t)                      \
    GO(gtk_application_window_get_type, LFv_t)         \
    GO(gtk_bin_get_type, LFv_t)                        \
    GO(gtk_widget_get_type, LFv_t)                     \
    GO(gtk_button_get_type, LFv_t)                     \
    GO(gtk_menu_button_get_type, LFv_t)                \
    GO(gtk_container_get_type, LFv_t)                  \
    GO(gtk_misc_get_type, LFv_t)                       \
    GO(gtk_label_get_type, LFv_t)                      \
    GO(gtk_list_box_get_type, LFv_t)                   \
    GO(gtk_image_get_type, LFv_t)                      \
    GO(gtk_list_box_row_get_type, LFv_t)               \
    GO(gtk_tree_view_get_type, LFv_t)                  \
    GO(gtk_window_get_type, LFv_t)                     \
    GO(gtk_table_get_type, LFv_t)                      \
    GO(gtk_fixed_get_type, LFv_t)                      \
    GO(gtk_combo_box_get_type, LFv_t)                  \
    GO(gtk_toggle_button_get_type, LFv_t)              \
    GO(gtk_check_button_get_type, LFv_t)               \
    GO(gtk_text_view_get_type, LFv_t)                  \
    GO(gtk_frame_get_type, LFv_t)                      \
    GO(gtk_entry_get_type, LFv_t)                      \
    GO(gtk_spin_button_get_type, LFv_t)                \
    GO(gtk_progress_get_type, LFv_t)                   \
    GO(gtk_progress_bar_get_type, LFv_t)               \
    GO(gtk_menu_shell_get_type, LFv_t)                 \
    GO(gtk_menu_bar_get_type, LFv_t)                   \
    GO(gtk_action_get_type, LFv_t)                     \
    GO(gtk_application_get_type, LFv_t)                \
    GO(gtk_grid_get_type, LFv_t)                       \
    GO(gtk_event_controller_get_type, LFv_t)           \
    GO(gtk_gesture_get_type, LFv_t)                    \
    GO(gtk_gesture_single_get_type, LFv_t)             \
    GO(gtk_gesture_long_press_get_type, LFv_t)         \
    GO(gtk_drawing_area_get_type, LFv_t)               \
    GO(gtk_dialog_add_button, pFppi_t)                 \
    GO(gtk_spin_button_get_value, dFp_t)               \
    GO(gtk_builder_lookup_callback_symbol, pFpp_t)     \
    GO(g_module_symbol, iFppp_t)                       \
    GO(g_log, vFpipV_t)                                \
    GO(g_module_open, pFpu_t)                          \
    GO(g_module_close, vFp_t)                          \
    GO(gtk_tree_store_newv, pFup_t)                    \
    GO(gtk_widget_style_get_valist, vFppA_t)           \
    GO(gtk_widget_style_get_property, vFppp_t)         \
    GO(gtk_list_store_insert_with_valuesv, vFppippi_t) \
    GO(gtk_tree_model_get_value, vFppip_t)             \
    GO(gtk_list_store_set_value, vFppip_t)

#include "generated/wrappedgtk3types.h"


#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// GtkMenuDetachFunc
#define GO(A)   \
static uintptr_t my_menudetach_fct_##A = 0;                                 \
static void my_menudetach_##A(void* widget, void* menu)                     \
{                                                                           \
    RunFunctionFmt(my_menudetach_fct_##A, "pp", widget, menu);        \
}
SUPER()
#undef GO
static void* findMenuDetachFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_menudetach_fct_##A == (uintptr_t)fct) return my_menudetach_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_menudetach_fct_##A == 0) {my_menudetach_fct_##A = (uintptr_t)fct; return my_menudetach_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GMenuDetachFunc callback\n");
    return NULL;
}

// GtkMenuPositionFunc
#define GO(A)   \
static uintptr_t my_menuposition_fct_##A = 0;                                                   \
static void my_menuposition_##A(void* menu, void* x, void* y, void* push_in, void* data)        \
{                                                                                               \
    RunFunctionFmt(my_menuposition_fct_##A, "ppppp", menu, x, y, push_in, data);          \
}
SUPER()
#undef GO
static void* findMenuPositionFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_menuposition_fct_##A == (uintptr_t)fct) return my_menuposition_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_menuposition_fct_##A == 0) {my_menuposition_fct_##A = (uintptr_t)fct; return my_menuposition_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GtkMenuPositionFunc callback\n");
    return NULL;
}

// GtkFunction
#define GO(A)   \
static uintptr_t my3_gtkfunction_fct_##A = 0;                               \
static int my3_gtkfunction_##A(void* data)                                  \
{                                                                           \
    return RunFunctionFmt(my3_gtkfunction_fct_##A, "p", data);        \
}
SUPER()
#undef GO
static void* findGtkFunctionFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my3_gtkfunction_fct_##A == (uintptr_t)fct) return my3_gtkfunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_gtkfunction_fct_##A == 0) {my3_gtkfunction_fct_##A = (uintptr_t)fct; return my3_gtkfunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GtkFunction callback\n");
    return NULL;
}

// GtkClipboardGetFunc
#define GO(A)   \
static uintptr_t my_clipboardget_fct_##A = 0;                                                       \
static void my_clipboardget_##A(void* clipboard, void* selection, uint32_t info, void* data)        \
{                                                                                                   \
    RunFunctionFmt(my_clipboardget_fct_##A, "ppup", clipboard, selection, info, data);        \
}
SUPER()
#undef GO
static void* findClipboadGetFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_clipboardget_fct_##A == (uintptr_t)fct) return my_clipboardget_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_clipboardget_fct_##A == 0) {my_clipboardget_fct_##A = (uintptr_t)fct; return my_clipboardget_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GtkClipboardGetFunc callback\n");
    return NULL;
}

// GtkClipboardClearFunc
#define GO(A)   \
static uintptr_t my_clipboardclear_fct_##A = 0;                                     \
static void my_clipboardclear_##A(void* clipboard, void* data)                      \
{                                                                                   \
    RunFunctionFmt(my_clipboardclear_fct_##A, "pp", clipboard, data);         \
}
SUPER()
#undef GO
static void* findClipboadClearFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_clipboardclear_fct_##A == (uintptr_t)fct) return my_clipboardclear_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_clipboardclear_fct_##A == 0) {my_clipboardclear_fct_##A = (uintptr_t)fct; return my_clipboardclear_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GtkClipboardClearFunc callback\n");
    return NULL;
}

// GtkCallback
#define GO(A)                                                                   \
static uintptr_t my3_gtkcallback_fct_##A = 0;                                   \
static void my3_gtkcallback_##A(void* widget, void* data)                       \
{                                                                               \
    RunFunctionFmt(my3_gtkcallback_fct_##A, "pp", widget, data);          \
}
SUPER()
#undef GO
static void* findGtkCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my3_gtkcallback_fct_##A == (uintptr_t)fct) return my3_gtkcallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_gtkcallback_fct_##A == 0) {my3_gtkcallback_fct_##A = (uintptr_t)fct; return my3_gtkcallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GtkCallback callback\n");
    return NULL;
}

// GtkTextCharPredicate
#define GO(A)   \
static uintptr_t my_textcharpredicate_fct_##A = 0;                                          \
static int my_textcharpredicate_##A(uint32_t ch, void* data)                                \
{                                                                                           \
    return (int)RunFunctionFmt(my_textcharpredicate_fct_##A, "up", ch, data);         \
}
SUPER()
#undef GO
static void* findGtkTextCharPredicateFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_textcharpredicate_fct_##A == (uintptr_t)fct) return my_textcharpredicate_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_textcharpredicate_fct_##A == 0) {my_textcharpredicate_fct_##A = (uintptr_t)fct; return my_textcharpredicate_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GtkTextCharPredicate callback\n");
    return NULL;
}

// Toolbar
#define GO(A)   \
static uintptr_t my_toolbar_fct_##A = 0;                                \
static void my_toolbar_##A(void* widget, void* data)                    \
{                                                                       \
    RunFunctionFmt(my_toolbar_fct_##A, "pp", widget, data);       \
}
SUPER()
#undef GO
static void* findToolbarFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_toolbar_fct_##A == (uintptr_t)fct) return my_toolbar_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_toolbar_fct_##A == 0) {my_toolbar_fct_##A = (uintptr_t)fct; return my_toolbar_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 Toolbar callback\n");
    return NULL;
}

// Builder
#define GO(A)   \
static uintptr_t my_builderconnect_fct_##A = 0;                                                                                     \
static void my_builderconnect_##A(void* builder, void* object, void* signal, void* handler, void* connect, int flags, void* data)   \
{                                                                                                                                   \
    RunFunctionFmt(my_builderconnect_fct_##A, "pppppip", builder, object, signal, handler, connect, flags, data);             \
}
SUPER()
#undef GO
static void* findBuilderConnectFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_builderconnect_fct_##A == (uintptr_t)fct) return my_builderconnect_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_builderconnect_fct_##A == 0) {my_builderconnect_fct_##A = (uintptr_t)fct; return my_builderconnect_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 BuilderConnect callback\n");
    return NULL;
}

// GtkTreeViewSearchEqualFunc
#define GO(A)   \
static uintptr_t my_GtkTreeViewSearchEqualFunc_fct_##A = 0;                                                             \
static int my_GtkTreeViewSearchEqualFunc_##A(void* model, int column, void* key, void* iter, void* data)                \
{                                                                                                                       \
    return RunFunctionFmt(my_GtkTreeViewSearchEqualFunc_fct_##A, "pippp", model, column, key, iter, data);        \
}
SUPER()
#undef GO
static void* findGtkTreeViewSearchEqualFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GtkTreeViewSearchEqualFunc_fct_##A == (uintptr_t)fct) return my_GtkTreeViewSearchEqualFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GtkTreeViewSearchEqualFunc_fct_##A == 0) {my_GtkTreeViewSearchEqualFunc_fct_##A = (uintptr_t)fct; return my_GtkTreeViewSearchEqualFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GtkTreeViewSearchEqualFunc callback\n");
    return NULL;
}
// GtkTreeCellDataFunc
#define GO(A)   \
static uintptr_t my_GtkTreeCellDataFunc_fct_##A = 0;                                                    \
static void my_GtkTreeCellDataFunc_##A(void* tree, void* cell, void* model, void* iter, void* data)     \
{                                                                                                       \
    RunFunctionFmt(my_GtkTreeCellDataFunc_fct_##A, "ppppp", tree, cell, model, iter, data);       \
}
SUPER()
#undef GO
static void* findGtkTreeCellDataFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GtkTreeCellDataFunc_fct_##A == (uintptr_t)fct) return my_GtkTreeCellDataFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GtkTreeCellDataFunc_fct_##A == 0) {my_GtkTreeCellDataFunc_fct_##A = (uintptr_t)fct; return my_GtkTreeCellDataFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GtkTreeCellDataFunc callback\n");
    return NULL;
}

// GDestroyNotify
#define GO(A)   \
static uintptr_t my_GDestroyNotify_fct_##A = 0;                         \
static void my_GDestroyNotify_##A(void* data)                           \
{                                                                       \
    RunFunctionFmt(my_GDestroyNotify_fct_##A, "p", data);         \
}
SUPER()
#undef GO
static void* findGDestroyNotifyFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GDestroyNotify_fct_##A == (uintptr_t)fct) return my_GDestroyNotify_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GDestroyNotify_fct_##A == 0) {my_GDestroyNotify_fct_##A = (uintptr_t)fct; return my_GDestroyNotify_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GDestroyNotify callback\n");
    return NULL;
}

// GTickCallback
#define GO(A)                                                                 \
    static uintptr_t my_GTickCallback_fct_##A = 0;                            \
    static void my_GTickCallback_##A(void* widget, void* clock, void* data)   \
    {                                                                         \
        RunFunctionFmt(my_GTickCallback_fct_##A, "ppp", widget, clock, data); \
    }
SUPER()
#undef GO
static void* findGTickCallbackFct(void* fct)
{
    if (!fct) return fct;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
#define GO(A) \
    if (my_GTickCallback_fct_##A == (uintptr_t)fct) return my_GTickCallback_##A;
    SUPER()
#undef GO
#define GO(A)                                      \
    if (my_GTickCallback_fct_##A == 0) {           \
        my_GTickCallback_fct_##A = (uintptr_t)fct; \
        return my_GTickCallback_##A;               \
    }
    SUPER()
#undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GTickCallback callback\n");
    return NULL;
}

// GtkTreeIterCompareFunc
#define GO(A)   \
static uintptr_t my_GtkTreeIterCompareFunc_fct_##A = 0;                                                 \
static int my_GtkTreeIterCompareFunc_##A(void* model, void* a, void* b, void* data)                     \
{                                                                                                       \
    return RunFunctionFmt(my_GtkTreeIterCompareFunc_fct_##A, "pppp", model, a, b, data);          \
}
SUPER()
#undef GO
static void* findGtkTreeIterCompareFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GtkTreeIterCompareFunc_fct_##A == (uintptr_t)fct) return my_GtkTreeIterCompareFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GtkTreeIterCompareFunc_fct_##A == 0) {my_GtkTreeIterCompareFunc_fct_##A = (uintptr_t)fct; return my_GtkTreeIterCompareFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GtkTreeIterCompareFunc callback\n");
    return NULL;
}

// GtkPrinterFunc
#define GO(A)   \
static uintptr_t my_GtkPrinterFunc_fct_##A = 0;                                         \
static int my_GtkPrinterFunc_##A(void* printer, void* data)                             \
{                                                                                       \
    return RunFunctionFmt(my_GtkPrinterFunc_fct_##A, "pp", printer, data);        \
}
SUPER()
#undef GO
static void* findGtkPrinterFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GtkPrinterFunc_fct_##A == (uintptr_t)fct) return my_GtkPrinterFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GtkPrinterFunc_fct_##A == 0) {my_GtkPrinterFunc_fct_##A = (uintptr_t)fct; return my_GtkPrinterFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GtkPrinterFunc callback\n");
    return NULL;
}

// GtkPrintJobCompleteHunc
#define GO(A)   \
static uintptr_t my_GtkPrintJobCompleteHunc_fct_##A = 0;                                        \
static void my_GtkPrintJobCompleteHunc_##A(void* job, void* data, void* error)                  \
{                                                                                               \
    RunFunctionFmt(my_GtkPrintJobCompleteHunc_fct_##A, "ppp", job, data, error);          \
}
SUPER()
#undef GO
static void* findGtkPrintJobCompleteHuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GtkPrintJobCompleteHunc_fct_##A == (uintptr_t)fct) return my_GtkPrintJobCompleteHunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GtkPrintJobCompleteHunc_fct_##A == 0) {my_GtkPrintJobCompleteHunc_fct_##A = (uintptr_t)fct; return my_GtkPrintJobCompleteHunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GtkPrintJobCompleteHunc callback\n");
    return NULL;
}


// GtkClipboardTextReceivedFunc
#define GO(A)   \
static uintptr_t my_GtkClipboardTextReceivedFunc_fct_##A = 0;                                           \
static void my_GtkClipboardTextReceivedFunc_##A(void* clipboard, void* text, void* data)                \
{                                                                                                       \
    RunFunctionFmt(my_GtkClipboardTextReceivedFunc_fct_##A, "ppp", clipboard, text, data);        \
}
SUPER()
#undef GO
static void* findGtkClipboardTextReceivedFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GtkClipboardTextReceivedFunc_fct_##A == (uintptr_t)fct) return my_GtkClipboardTextReceivedFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GtkClipboardTextReceivedFunc_fct_##A == 0) {my_GtkClipboardTextReceivedFunc_fct_##A = (uintptr_t)fct; return my_GtkClipboardTextReceivedFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GtkClipboardTextReceivedFunc callback\n");
    return NULL;
}

// GtkFileFilterFunc ...
#define GO(A)   \
static uintptr_t my_GtkFileFilterFunc_fct_##A = 0;                                      \
static int my_GtkFileFilterFunc_##A(void* a, void* b)                                   \
{                                                                                       \
    return (int)RunFunctionFmt(my_GtkFileFilterFunc_fct_##A, "pp", a, b);         \
}
SUPER()
#undef GO
static void* find_GtkFileFilterFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GtkFileFilterFunc_fct_##A == (uintptr_t)fct) return my_GtkFileFilterFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GtkFileFilterFunc_fct_##A == 0) {my_GtkFileFilterFunc_fct_##A = (uintptr_t)fct; return my_GtkFileFilterFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GtkFileFilterFunc callback\n");
    return NULL;
}

// GtkListBoxUpdateHeaderFunc
#define GO(A)   \
static uintptr_t my_GtkListBoxUpdateHeaderFunc_fct_##A = 0;                 \
static void my_GtkListBoxUpdateHeaderFunc_##A(void* a, void* b, void* c)    \
{                                                                           \
    RunFunctionFmt(my_GtkListBoxUpdateHeaderFunc_fct_##A, "ppp", a, b, c);  \
}
SUPER()
#undef GO
static void* findGtkListBoxUpdateHeaderFunc(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GtkListBoxUpdateHeaderFunc_fct_##A == (uintptr_t)fct) return my_GtkListBoxUpdateHeaderFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GtkListBoxUpdateHeaderFunc_fct_##A == 0) {my_GtkListBoxUpdateHeaderFunc_fct_##A = (uintptr_t)fct; return my_GtkListBoxUpdateHeaderFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GtkListBoxUpdateHeaderFunc callback\n");
    return NULL;
}
// TranslateEvent
#define GO(A)   \
static uintptr_t my_TranslateEvent_fct_##A = 0;             \
static void my_TranslateEvent_##A(void* a, void* b)         \
{                                                           \
    RunFunctionFmt(my_TranslateEvent_fct_##A, "pp", a, b);  \
}
SUPER()
#undef GO
static void* findTranslateEvent(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_TranslateEvent_fct_##A == (uintptr_t)fct) return my_TranslateEvent_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_TranslateEvent_fct_##A == 0) {my_TranslateEvent_fct_##A = (uintptr_t)fct; return my_TranslateEvent_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 TranslateEvent callback\n");
    return NULL;
}

// GCallback
#define GO(A)                                     \
    static uintptr_t my_GCallback_fct_##A = 0;    \
    static void my_GCallback_##A(void* data)      \
    {                                             \
        RunFunctionFmt(my_GCallback_fct_##A, ""); \
    }
SUPER()
#undef GO
static void* findGCallbackFct(void* fct)
{
    if (!fct) return fct;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
#define GO(A) \
    if (my_GCallback_fct_##A == (uintptr_t)fct) return my_GCallback_##A;
    SUPER()
#undef GO
#define GO(A)                                  \
    if (my_GCallback_fct_##A == 0) {           \
        my_GCallback_fct_##A = (uintptr_t)fct; \
        return my_GCallback_##A;               \
    }
    SUPER()
#undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GCallback callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my3_gtk_dialog_add_buttons(x64emu_t* emu, void* dialog, void* first, uintptr_t* b)
{
    void* btn = first;
    while(btn) {
        int id = (int)*(b++);
        my->gtk_dialog_add_button(dialog, btn, id);
        btn = (void*)*(b++);
    }
}

EXPORT void my3_gtk_message_dialog_format_secondary_text(x64emu_t* emu, void* dialog, void* fmt, void* b)
{
    char* buf = NULL;
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    int ret = vasprintf(&buf, fmt, VARARGS);
    (void)ret;
    // pre-bake the fmt/vaarg, because there is no "va_list" version of this function
    my->gtk_message_dialog_format_secondary_text(dialog, buf);
    free(buf);
}

EXPORT void my3_gtk_message_dialog_format_secondary_markup(x64emu_t* emu, void* dialog, void* fmt, void* b)
{
    char* buf = NULL;
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    int ret = vasprintf(&buf, fmt, VARARGS);
    (void)ret;
    // pre-bake the fmt/vaarg, because there is no "va_list" version of this function
    my->gtk_message_dialog_format_secondary_markup(dialog, buf);
    free(buf);
}

EXPORT void my3_gtk_file_filter_add_custom(x64emu_t* emu, void* filter, uint32_t needed, void* f, void* data, void* d)
{
    (void)emu;
    my->gtk_file_filter_add_custom(filter, needed, find_GtkFileFilterFunc_Fct(f), data, findGDestroyNotifyFct(d));
}

EXPORT void my3_gtk_init(x64emu_t* emu, void* argc, void* argv)
{
    my->gtk_init(argc, argv);
    my_checkGlobalGdkDisplay();
    AutoBridgeGtk(my->g_type_class_ref, my->g_type_class_unref);
}

EXPORT int my3_gtk_init_check(x64emu_t* emu, void* argc, void* argv)
{
    int ret = my->gtk_init_check(argc, argv);
    my_checkGlobalGdkDisplay();
    AutoBridgeGtk(my->g_type_class_ref, my->g_type_class_unref);
    return ret;
}

EXPORT int my3_gtk_init_with_args(x64emu_t* emu, void* argc, void* argv, void* param, void* entries, void* trans, void* error)
{
    int ret = my->gtk_init_with_args(argc, argv, param, entries, trans, error);
    my_checkGlobalGdkDisplay();
    AutoBridgeGtk(my->g_type_class_ref, my->g_type_class_unref);
    return ret;
}

EXPORT void my3_gtk_menu_attach_to_widget(x64emu_t* emu, void* menu, void* widget, void* f)
{
    my->gtk_menu_attach_to_widget(menu, widget, findMenuDetachFct(f));
}

EXPORT void my3_gtk_menu_popup(x64emu_t* emu, void* menu, void* shell, void* item, void* f, void* data, uint32_t button, uint32_t time_)
{
    my->gtk_menu_popup(menu, shell, item, findMenuPositionFct(f), data, button, time_);
}

EXPORT int my3_gtk_clipboard_set_with_data(x64emu_t* emu, void* clipboard, void* target, uint32_t n, void* f_get, void* f_clear, void* data)
{
    return my->gtk_clipboard_set_with_data(clipboard, target, n, findClipboadGetFct(f_get), findClipboadClearFct(f_clear), data);
}

EXPORT int my3_gtk_clipboard_set_with_owner(x64emu_t* emu, void* clipboard, void* target, uint32_t n, void* f_get, void* f_clear, void* data)
{
    return my->gtk_clipboard_set_with_owner(clipboard, target, n, findClipboadGetFct(f_get), findClipboadClearFct(f_clear), data);
}

EXPORT void my3_gtk_stock_set_translate_func(x64emu_t* emu, void* domain, void* f, void* data, void* notify)
{
    my->gtk_stock_set_translate_func(domain, findTranslateEvent(f), data, findGDestroyNotifyFct(notify));
}

EXPORT void my3_gtk_container_forall(x64emu_t* emu, void* container, void* f, void* data)
{
    my->gtk_container_forall(container, findGtkCallbackFct(f), data);
}

EXPORT void my3_gtk_tree_view_set_search_equal_func(x64emu_t* emu, void* tree_view, void* f, void* data, void* notify)
{
    my->gtk_tree_view_set_search_equal_func(tree_view, findGtkTreeViewSearchEqualFuncFct(f), data, findGDestroyNotifyFct(notify));
}

EXPORT int my3_gtk_text_iter_backward_find_char(x64emu_t* emu, void* iter, void* f, void* data, void* limit)
{
    return my->gtk_text_iter_backward_find_char(iter, findGtkTextCharPredicateFct(f), data, limit);
}

EXPORT int my3_gtk_text_iter_forward_find_char(x64emu_t* emu, void* iter, void* f, void* data, void* limit)
{
    return my->gtk_text_iter_forward_find_char(iter, findGtkTextCharPredicateFct(f), data, limit);
}

EXPORT void my3_gtk_tree_sortable_set_sort_func(x64emu_t* emu, void* sortable, int id, void* f, void* data, void* notify)
{
    my->gtk_tree_sortable_set_sort_func(sortable, id, findGtkTreeIterCompareFuncFct(f), data, findGDestroyNotifyFct(notify));
}

EXPORT void my3_gtk_tree_sortable_set_default_sort_func(x64emu_t* emu, void* sortable, void* f, void* data, void* notify)
{
    my->gtk_tree_sortable_set_default_sort_func(sortable, findGtkTreeIterCompareFuncFct(f), data, findGDestroyNotifyFct(notify));
}

EXPORT float my3_gtk_spin_button_get_value_as_float(x64emu_t* emu, void* spinner)
{
    return my->gtk_spin_button_get_value(spinner);
}

EXPORT void my3_gtk_builder_connect_signals_full(x64emu_t* emu, void* builder, void* f, void* data)
{
    my->gtk_builder_connect_signals_full(builder, findBuilderConnectFct(f), data);
}

typedef struct my_connectargs_s {
  void* module;
  void* data;
} my_connectargs_t;
//defined in gobject2...
uintptr_t my_g_signal_connect_object(x64emu_t* emu, void* instance, void* detailed, void* c_handler, void* object, uint32_t flags);
uintptr_t my_g_signal_connect_data(x64emu_t* emu, void* instance, void* detailed, void* c_handler, void* data, void* closure, uint32_t flags);
static void my3_gtk_builder_connect_signals_default(void* builder, void* object,
                                                    char* signal_name, char* handler_name,
                                                    void* connect_object, uint32_t flags, my_connectargs_t* args)
{
  void* func = my->gtk_builder_lookup_callback_symbol(builder, handler_name);
  if (!func && args && args->module) {
    my->g_module_symbol(args->module, handler_name, &func);
  }
  // Mixing Native and emulated code... the my_g_signal_* function will handle that (GetNativeFnc does)
  if(!func)
      func = (void*)FindGlobalSymbol(my_context->maplib, handler_name, 0, NULL, 0);

  if(!func) {
      my->g_log("Gtk", 1<<4, "Could not find signal handler '%s'.", handler_name);
      return;
  }

  if (connect_object)
    my_g_signal_connect_object(thread_get_emu(), object, signal_name, func, connect_object, flags);
  else
    my_g_signal_connect_data(thread_get_emu(), object, signal_name, func, args->data, NULL, flags);
}

EXPORT void my3_gtk_builder_connect_signals(x64emu_t* emu, void* builder, void* data)
{
    my_connectargs_t args = {0};
    args.data = data;
    if(my->g_module_open && my->g_module_close)
        args.module = my->g_module_open(NULL, 1);

    my->gtk_builder_connect_signals_full(builder, my3_gtk_builder_connect_signals_default, &args);

    if(args.module)
        my->g_module_close(args.module);
}

EXPORT void my3_gtk_tree_view_column_set_cell_data_func(x64emu_t* emu, void* tree, void* cell, void* f, void* data, void* destroy)
{
    my->gtk_tree_view_column_set_cell_data_func(tree, cell, findGtkTreeCellDataFuncFct(f), data, findGDestroyNotifyFct(destroy));
}

EXPORT void* my3_gtk_tree_store_new(x64emu_t* emu, uint32_t n, uintptr_t* b)
{
    uintptr_t c[n];
    for(uint32_t i=0; i<n; ++i)
        c[i] = getVArgs(emu, 1, b, i);
    return my->gtk_tree_store_newv(n, c);
}

EXPORT void my3_gtk_style_context_get_valist(x64emu_t* emu, void* context, uint32_t state, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    my->gtk_style_context_get_valist(context, state, VARARGS);
}

EXPORT void my3_gtk_style_context_get_style_valist(x64emu_t* emu, void* context, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    my->gtk_style_context_get_style_valist(context, VARARGS);
}

EXPORT void my3_gtk_style_context_get_style(x64emu_t* emu, void* context, uintptr_t* b)
{
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 1);
    my->gtk_style_context_get_style_valist(context, VARARGS);
}


EXPORT void my3_gtk_enumerate_printers(x64emu_t* emu, void* f, void* data, void* d, int i)
{
    my->gtk_enumerate_printers(findGtkPrinterFuncFct(f), data, findGDestroyNotifyFct(d), i);
}

EXPORT void my3_gtk_print_job_send(x64emu_t* emu, void *job, void* f, void* data, void* d)
{
    my->gtk_print_job_send(job, findGtkPrintJobCompleteHuncFct(f), data, findGDestroyNotifyFct(d));
}

EXPORT void my3_gtk_container_foreach(x64emu_t* emu, void* container, void* cb, void* data)
{
    my->gtk_container_foreach(container, findGtkCallbackFct(cb), data);
}

EXPORT void my3_gtk_clipboard_request_text(x64emu_t* emu, void* clipboard, void* f, void* data)
{
    my->gtk_clipboard_request_text(clipboard, findGtkClipboardTextReceivedFuncFct(f), data);
}

EXPORT void my3_gtk_style_context_get(x64emu_t* emu, void* context, uint32_t flags, uintptr_t* b)
{
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    my->gtk_style_context_get_valist(context, flags, VARARGS);
}

EXPORT void my3_gtk_widget_style_get(x64emu_t* emu, void* widget, void* first, uintptr_t* b)
{
// #ifdef PREFER_CONVERT_VAARG
#if 1
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    my->gtk_widget_style_get_valist(widget, first, VARARGS);
#else // breaks DeltaPatcher
    void* prop = first;
    int i=0;
    do {
        void* val = (void*)getVArgs(emu, 2, b, i++);
        my->gtk_widget_style_get_property(widget, prop, val);
        prop = (void*)getVArgs(emu, 2, b, i++);
    } while (prop);
#endif
}

EXPORT void my3_gtk_list_store_insert_with_values(x64emu_t* emu, void* list_store, void* iter, int position, uintptr_t* b)
{
    int n = 0;
    while((((int)getVArgs(emu, 3, b, n*2)))!=-1) n+=1;
    int columns[n];
    my_GValue_t values[n];
    for(int i=0; i<n; ++i) {
      columns[i] = (int)getVArgs(emu, 3, b, i*2+0);
      values[i] = *(my_GValue_t*)getVArgs(emu, 3, b, i*2+1);
    }
    my->gtk_list_store_insert_with_valuesv(list_store, iter, position, columns, values, n);
}

EXPORT void my3_gtk_tree_store_set_valist(x64emu_t* emu, void* tree, void* iter, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    my->gtk_tree_store_set_valist(tree, iter, VARARGS);
}

EXPORT void my3_gtk_tree_store_set(x64emu_t* emu, void* tree, void* iter, uintptr_t* b)
{
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    my->gtk_tree_store_set_valist(tree, iter, VARARGS);
}

EXPORT void my3_gtk_list_box_set_header_func(x64emu_t* emu, void* box, void* f, void* data, void* d)
{
    my->gtk_list_box_set_header_func(box, findGtkListBoxUpdateHeaderFunc(f), data, findGDestroyNotifyFct(d));
}

EXPORT void my3_gtk_tree_model_get(x64emu_t* emu, void* tree_model, void* iter, uintptr_t* b)
{
// #ifdef PREFER_CONVERT_VAARG
#if 1
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    my->gtk_tree_model_get_valist(tree_model, iter, VARARGS);
#else
    int i = 0;
    int col = (int)getVArgs(emu, 2, b, i++);
    while (col != -1) {
        void* val = (void*)getVArgs(emu, 2, b, i++);
        my->gtk_tree_model_get_value(tree_model, iter, col, val);
        col = (int)getVArgs(emu, 2, b, i++);
    }
#endif
}

EXPORT void my3_gtk_tree_model_get_valist(x64emu_t* emu, void* tree_model, void* iter, x64_va_list_t V)
{
#ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
#else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
#endif
    my->gtk_tree_model_get_valist(tree_model, iter, VARARGS);
}

EXPORT void my3_gtk_list_store_set(x64emu_t* emu, void* list_store, void* iter, uintptr_t* b)
{
// #ifdef PREFER_CONVERT_VAARG
#if 1
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    my->gtk_list_store_set_valist(list_store, iter, VARARGS);
#else
    int i = 0;
    int col = (int)getVArgs(emu, 2, b, i++);
    while (col != -1) {
        void* val = (void*)getVArgs(emu, 2, b, i++);
        my->gtk_list_store_set_value(list_store, iter, col, val);
        col = (int)getVArgs(emu, 2, b, i++);
    }
#endif
}

EXPORT void my3_gtk_list_store_set_valist(x64emu_t* emu, void* list_store, void* iter, x64_va_list_t V)
{
#ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
#else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
#endif
    my->gtk_list_store_set_valist(list_store, iter, VARARGS);
}

EXPORT void my3_gtk_widget_class_bind_template_callback_full(x64emu_t* emu, void* widget_class, void* cb_name, void* cb)
{
    my->gtk_widget_class_bind_template_callback_full(widget_class, cb_name, findGCallbackFct(cb));
}

EXPORT uint32_t my3_gtk_widget_add_tick_callback(x64emu_t* emu, void* widget, void* callback, void* data, void* d)
{
    return my->gtk_widget_add_tick_callback(widget, findGTickCallbackFct(callback), data, findGDestroyNotifyFct(d));
}

#define PRE_INIT \
    if (BOX64ENV(nogtk)) return -2;

#define ALTMY my3_

#define CUSTOM_INIT \
    SetGtkApplicationID(my->gtk_application_get_type());        \
    SetGtkWidget3ID(my->gtk_widget_get_type());                 \
    SetGtkFixed3ID(my->gtk_fixed_get_type());                   \
    SetGtkContainer3ID(my->gtk_container_get_type());           \
    SetGtkBin3ID(my->gtk_bin_get_type());                       \
    SetGtkButton3ID(my->gtk_button_get_type());                 \
    SetGtkDrawingArea3ID(my->gtk_drawing_area_get_type());      \
    SetGtkMenuButton3ID(my->gtk_menu_button_get_type());        \
    SetGtkWindow3ID(my->gtk_window_get_type());                 \
    SetGtkApplicationWindowID(my->gtk_application_window_get_type());\
    SetGtkListBoxID(my->gtk_list_box_get_type());               \
    SetGtkListBoxRowID(my->gtk_list_box_row_get_type());        \
    SetGtkTextView3ID(my->gtk_text_view_get_type());            \
    SetGtkGrid3ID(my->gtk_grid_get_type());                     \
    SetGtkMisc3ID(my->gtk_misc_get_type());                     \
    SetGtkLabel3ID(my->gtk_label_get_type());                   \
    SetGtkImage3ID(my->gtk_image_get_type());                   \
    SetGtkEventControllerID(my->gtk_event_controller_get_type());\
    SetGtkGestureID(my->gtk_gesture_get_type());                \
    SetGtkGestureSingleID(my->gtk_gesture_single_get_type());   \
    SetGtkGestureLongPressID(my->gtk_gesture_long_press_get_type());\
    SetGtkActionID(my->gtk_action_get_type());

#define NEEDED_LIBS "libgdk-3.so.0", "libpangocairo-1.0.so.0", "libgio-2.0.so.0", "libcairo.so.2"

#include "wrappedlib_init.h"
