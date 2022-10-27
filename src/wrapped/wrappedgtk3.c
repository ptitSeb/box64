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
static char* libname = NULL;
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
typedef void          (*vFpipV_t)(void*, int, void*, ...);

#define ADDED_FUNCTIONS()                   \
GO(g_type_class_ref, pFL_t)                 \
GO(g_type_class_unref, vFp_t)               \
GO(g_initially_unowned_get_type, LFv_t)     \
GO(gtk_bin_get_type, LFv_t)                 \
GO(gtk_widget_get_type, LFv_t)              \
GO(gtk_button_get_type, LFv_t)              \
GO(gtk_container_get_type, LFv_t)           \
GO(gtk_misc_get_type, LFv_t)                \
GO(gtk_label_get_type, LFv_t)               \
GO(gtk_tree_view_get_type, LFv_t)           \
GO(gtk_window_get_type, LFv_t)              \
GO(gtk_table_get_type, LFv_t)               \
GO(gtk_fixed_get_type, LFv_t)               \
GO(gtk_combo_box_get_type, LFv_t)           \
GO(gtk_toggle_button_get_type, LFv_t)       \
GO(gtk_check_button_get_type, LFv_t)        \
GO(gtk_frame_get_type, LFv_t)               \
GO(gtk_entry_get_type, LFv_t)               \
GO(gtk_spin_button_get_type, LFv_t)         \
GO(gtk_progress_get_type, LFv_t)            \
GO(gtk_progress_bar_get_type, LFv_t)        \
GO(gtk_menu_shell_get_type, LFv_t)          \
GO(gtk_menu_bar_get_type, LFv_t)            \
GO(gtk_action_get_type, LFv_t)              \
GO(gtk_dialog_add_button, pFppi_t)          \
GO(gtk_spin_button_get_value, dFp_t)        \
GO(gtk_builder_lookup_callback_symbol, pFpp_t)  \
GO(g_module_symbol, iFppp_t)                \
GO(g_log, vFpipV_t)                         \
GO(g_module_open, pFpu_t)                   \
GO(g_module_close, vFp_t)                   \
GO(gtk_tree_store_newv, pFup_t)             \

#include "generated/wrappedgtk3types.h"


#include "wrappercallback.h"

EXPORT uintptr_t my3_gtk_signal_connect_full(x64emu_t* emu, void* object, void* name, void* c_handler, void* unsupported, void* data, void* closure, uint32_t signal, int after)
{
    my_signal_t *sig = new_mysignal(c_handler, data, closure);
    uintptr_t ret = my->gtk_signal_connect_full(object, name, my_signal_cb, NULL, sig, my_signal_delete, signal, after);
    printf_log(LOG_DEBUG, "Connecting gtk signal \"%s\" with cb=%p\n", (char*)name, sig);
    return ret;
}

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// GtkMenuDetachFunc
#define GO(A)   \
static uintptr_t my_menudetach_fct_##A = 0;   \
static void my_menudetach_##A(void* widget, void* menu)     \
{                                       \
    RunFunction(my_context, my_menudetach_fct_##A, 2, widget, menu);\
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
static uintptr_t my_menuposition_fct_##A = 0;   \
static void my_menuposition_##A(void* menu, void* x, void* y, void* push_in, void* data)     \
{                                       \
    RunFunction(my_context, my_menuposition_fct_##A, 5, menu, x, y, push_in, data);\
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
static uintptr_t my3_gtkfunction_fct_##A = 0;   \
static int my3_gtkfunction_##A(void* data)     \
{                                       \
    return RunFunction(my_context, my3_gtkfunction_fct_##A, 1, data);\
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
static uintptr_t my_clipboardget_fct_##A = 0;   \
static void my_clipboardget_##A(void* clipboard, void* selection, uint32_t info, void* data)     \
{                                       \
    RunFunction(my_context, my_clipboardget_fct_##A, 4, clipboard, selection, info, data);\
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
static uintptr_t my_clipboardclear_fct_##A = 0;   \
static void my_clipboardclear_##A(void* clipboard, void* data)     \
{                                       \
    RunFunction(my_context, my_clipboardclear_fct_##A, 2, clipboard, data);\
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
#define GO(A)   \
static uintptr_t my3_gtkcallback_fct_##A = 0;   \
static void my3_gtkcallback_##A(void* widget, void* data)     \
{                                       \
    RunFunction(my_context, my3_gtkcallback_fct_##A, 2, widget, data);\
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
static uintptr_t my_textcharpredicate_fct_##A = 0;   \
static int my_textcharpredicate_##A(uint32_t ch, void* data)     \
{                                       \
    return (int)RunFunction(my_context, my_textcharpredicate_fct_##A, 2, ch, data);\
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
static uintptr_t my_toolbar_fct_##A = 0;   \
static void my_toolbar_##A(void* widget, void* data)     \
{                                       \
    RunFunction(my_context, my_toolbar_fct_##A, 2, widget, data);\
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
static uintptr_t my_builderconnect_fct_##A = 0;   \
static void my_builderconnect_##A(void* builder, void* object, void* signal, void* handler, void* connect, int flags, void* data)     \
{                                       \
    RunFunction(my_context, my_builderconnect_fct_##A, 7, builder, object, signal, handler, connect, flags, data);\
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
static uintptr_t my_GtkTreeViewSearchEqualFunc_fct_##A = 0;                                                     \
static int my_GtkTreeViewSearchEqualFunc_##A(void* model, int column, void* key, void* iter, void* data)        \
{                                                                                                               \
    return RunFunction(my_context, my_GtkTreeViewSearchEqualFunc_fct_##A, 5, model, column, key, iter, data);   \
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
static uintptr_t my_GtkTreeCellDataFunc_fct_##A = 0;                                                \
static void my_GtkTreeCellDataFunc_##A(void* tree, void* cell, void* model, void* iter, void* data) \
{                                                                                                   \
    RunFunction(my_context, my_GtkTreeCellDataFunc_fct_##A, 5, tree, cell, model, iter, data);      \
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
static uintptr_t my_GDestroyNotify_fct_##A = 0;                     \
static void my_GDestroyNotify_##A(void* data)                       \
{                                                                   \
    RunFunction(my_context, my_GDestroyNotify_fct_##A, 1, data);    \
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

// GtkTreeIterCompareFunc
#define GO(A)   \
static uintptr_t my_GtkTreeIterCompareFunc_fct_##A = 0;                                         \
static int my_GtkTreeIterCompareFunc_##A(void* model, void* a, void* b, void* data)             \
{                                                                                               \
    return RunFunction(my_context, my_GtkTreeIterCompareFunc_fct_##A, 4, model, a, b, data);    \
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
static uintptr_t my_GtkPrinterFunc_fct_##A = 0;                                     \
static int my_GtkPrinterFunc_##A(void* printer, void* data)                         \
{                                                                                   \
    return RunFunction(my_context, my_GtkPrinterFunc_fct_##A, 2, printer, data);    \
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
static uintptr_t my_GtkPrintJobCompleteHunc_fct_##A = 0;                                \
static void my_GtkPrintJobCompleteHunc_##A(void* job, void* data, void* error)          \
{                                                                                       \
    RunFunction(my_context, my_GtkPrintJobCompleteHunc_fct_##A, 3, job, data, error);   \
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

#undef SUPER
/*
EXPORT void my3_gtk_dialog_add_buttons(x64emu_t* emu, void* dialog, void* first, uintptr_t* b)
{
    void* btn = first;
    while(btn) {
        int id = (int)*(b++);
        my->gtk_dialog_add_button(dialog, btn, id);
        btn = (void*)*(b++);
    }
}
*/
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
/*
EXPORT void my3_gtk_message_dialog_format_secondary_markup(x64emu_t* emu, void* dialog, void* fmt, void* b)
{
    char* buf = NULL;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, VARARGS);
    #else
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, b);
    #endif
    // pre-bake the fmt/vaarg, because there is no "va_list" version of this function
    my->gtk_message_dialog_format_secondary_markup(dialog, buf);
    free(buf);
}
*/
EXPORT void* my3_gtk_type_class(x64emu_t* emu, size_t type)
{
    void* class = my->gtk_type_class(type);
    return wrapCopyGTKClass(class, type);
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

EXPORT uint32_t my3_gtk_timeout_add(x64emu_t* emu, uint32_t interval, void* f, void* data)
{
    return my->gtk_timeout_add(interval, findGtkFunctionFct(f), data);
}

EXPORT int my3_gtk_clipboard_set_with_data(x64emu_t* emu, void* clipboard, void* target, uint32_t n, void* f_get, void* f_clear, void* data)
{
    return my->gtk_clipboard_set_with_data(clipboard, target, n, findClipboadGetFct(f_get), findClipboadClearFct(f_clear), data);
}

EXPORT int my3_gtk_clipboard_set_with_owner(x64emu_t* emu, void* clipboard, void* target, uint32_t n, void* f_get, void* f_clear, void* data)
{
    return my->gtk_clipboard_set_with_owner(clipboard, target, n, findClipboadGetFct(f_get), findClipboadClearFct(f_clear), data);
}

static void* my_translate_func(void* path, my_signal_t* sig)
{
    return (void*)RunFunction(my_context, sig->c_handler, 2, path, sig->data);
}

EXPORT void my3_gtk_stock_set_translate_func(x64emu_t* emu, void* domain, void* f, void* data, void* notify)
{
    my_signal_t *sig = new_mysignal(f, data, notify);
    my->gtk_stock_set_translate_func(domain, my_translate_func, sig, my_signal_delete);
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

EXPORT void* my3_gtk_toolbar_append_item(x64emu_t* emu, void* toolbar, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data)
{
    return my->gtk_toolbar_append_item(toolbar, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data);
}

EXPORT void* my3_gtk_toolbar_prepend_item(x64emu_t* emu, void* toolbar, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data)
{
    return my->gtk_toolbar_prepend_item(toolbar, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data);
}

EXPORT void* my3_gtk_toolbar_insert_item(x64emu_t* emu, void* toolbar, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data, int position)
{
    return my->gtk_toolbar_insert_item(toolbar, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data, position);
}

EXPORT void* my3_gtk_toolbar_append_element(x64emu_t* emu, void* toolbar, size_t type, void* widget, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data)
{
    return my->gtk_toolbar_append_element(toolbar, type, widget, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data);
}

EXPORT void* my3_gtk_toolbar_prepend_element(x64emu_t* emu, void* toolbar, size_t type, void* widget, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data)
{
    return my->gtk_toolbar_prepend_element(toolbar, type, widget, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data);
}

EXPORT void* my3_gtk_toolbar_insert_element(x64emu_t* emu, void* toolbar, size_t type, void* widget, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data, int position)
{
    return my->gtk_toolbar_insert_element(toolbar, type, widget, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data, position);
}

EXPORT void* my3_gtk_toolbar_insert_stock(x64emu_t* emu, void* toolbar, void* stock_id, void* tooltip_text, void* tooltip_private, void* f, void* data, int position)
{
    return my->gtk_toolbar_insert_stock(toolbar, stock_id, tooltip_text, tooltip_private, findToolbarFct(f), data, position);
}

EXPORT void my3_gtk_tree_sortable_set_sort_func(x64emu_t* emu, void* sortable, int id, void* f, void* data, void* notify)
{
    my->gtk_tree_sortable_set_sort_func(sortable, id, findGtkTreeIterCompareFuncFct(f), data, findGDestroyNotifyFct(notify));
}

EXPORT void my3_gtk_tree_sortable_set_default_sort_func(x64emu_t* emu, void* sortable, void* f, void* data, void* notify)
{
    my->gtk_tree_sortable_set_default_sort_func(sortable, findGtkTreeIterCompareFuncFct(f), data, findGDestroyNotifyFct(notify));
}

EXPORT int my3_gtk_type_unique(x64emu_t* emu, size_t parent, my_GtkTypeInfo_t* gtkinfo)
{
    return my->gtk_type_unique(parent, findFreeGtkTypeInfo(gtkinfo, parent));
}

EXPORT unsigned long my3_gtk_signal_connect(x64emu_t* emu, void* object, void* name, void* func, void* data)
{
    return my3_gtk_signal_connect_full(emu, object, name, func, NULL, data, NULL, 0, 0);
}

EXPORT void my3_gtk_object_set_data_full(x64emu_t* emu, void* object, void* key, void* data, void* notify)
{
    my->gtk_object_set_data_full(object, key, data, findGDestroyNotifyFct(notify));
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
      func = (void*)FindGlobalSymbol(my_context->maplib, handler_name, 0, NULL);

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
    for(int i=0; i<n; ++i)
        c[i] = getVArgs(emu, 1, b, i);
    return my->gtk_tree_store_newv(n, c);
}

EXPORT void my3_gtk_style_context_get_valist(x64emu_t* emu, void* context, int state, x64_va_list_t V)
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

#define PRE_INIT    \
    if(box64_nogtk) \
        return -1;

#define CUSTOM_INIT \
    libname = lib->name;                                        \
    getMy(lib);                                                 \
    SETALT(my3_);                                               \
    SetGInitiallyUnownedID(my->g_initially_unowned_get_type()); \
    SetGtkWidget3ID(my->gtk_widget_get_type());                 \
    SetGtkActionID(my->gtk_action_get_type());                  \
    setNeededLibs(lib, 2, "libgdk-3.so.0", "libpangocairo-1.0.so.0");

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
