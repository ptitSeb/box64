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

const char* gtkx112Name = "libgtk-x11-2.0.so.0";
#define ALTNAME "libgtk-x11-2.0.so"

#define LIBNAME gtkx112

typedef size_t        (*LFv_t)(void);
typedef int           (*iFp_t)(void*);
typedef void          (*vFp_t)(void*);
typedef void*         (*pFp_t)(void*);
typedef double        (*dFp_t)(void*);
typedef void*         (*pFL_t)(size_t);
typedef void*         (*pFip_t)(int, void*);
typedef void*         (*pFpL_t)(void*, size_t);
typedef void          (*vFppi_t)(void*, void*, int);
typedef int32_t       (*iFppp_t)(void*, void*, void*);
typedef void*         (*pFppi_t)(void*, void*, int32_t);
typedef void          (*vFpipV_t)(void*, int, void*, ...);
typedef unsigned long (*LFppppi_t)(void*, void*, void*, void*, int);
typedef void          (*vFpuipp_t)(void*, uint32_t, int, void*, void*);
typedef void          (*vFppippi_t)(void*, void*, int, void* , void*, int);
typedef unsigned long (*LFpppppi_t)(void*, void*, void*, void*, void*, int);

#define ADDED_FUNCTIONS() \
    GO(gtk_object_get_type, LFv_t)              \
    GO(g_type_check_instance_cast, pFpL_t)      \
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
    GO(gtk_text_view_get_type, LFv_t)           \
    GO(gtk_action_get_type, LFv_t)              \
    GO(g_type_class_ref, pFL_t)                 \
    GO(g_type_class_unref, vFp_t)               \
    GO(gtk_button_get_label, pFp_t)             \
    GO(gtk_spin_button_get_value, dFp_t)        \
    GO(gtk_binding_entry_add_signall, vFpuipp_t)\
    GO(g_signal_connect_object, LFppppi_t)      \
    GO(gtk_dialog_add_button, pFppi_t)          \
    GO(g_signal_connect_data, LFpppppi_t)       \
    GO(gtk_list_store_newv, pFip_t)             \
    GO(gtk_tree_store_newv, pFip_t)             \
    GO(gtk_list_store_insert, vFppi_t)          \
    GO(g_module_close, iFp_t)                   \
    GO(g_module_open, pFpi_t)                   \
    GO(g_module_supported, LFv_t)               \
    GO(g_module_symbol, iFppp_t)                \
    GO(g_log, vFpipV_t)                         \
    GO(gtk_notebook_get_type, LFv_t)            \
    GO(gtk_cell_renderer_get_type, LFv_t)       \
    GO(gtk_list_store_insert_with_valuesv, vFppippi_t)

#include "generated/wrappedgtkx112types.h"

#define ADDED_STRUCT() \
    int  gtk1;

#define ADDED_INIT()                            \
    if(strcmp(lib->name, "libgtk-1.2.so.0")==0) \
        my->gtk1 = 1;

#include "wrappercallback.h"

// this is quite ineficient, but GCallback is often used, so create a large pool here...
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
GO(13)  \
GO(14)  \
GO(15)  \
GO(16)  \
GO(17)  \
GO(18)  \
GO(19)  \
GO(20)  \
GO(21)  \
GO(22)  \
GO(23)  \
GO(24)  \
GO(25)  \
GO(26)  \
GO(27)  \
GO(28)  \
GO(29)  \
GO(30)  \
GO(31)  \
GO(32)  \
GO(33)  \
GO(34)  \
GO(35)  \
GO(36)  \
GO(37)  \
GO(38)  \
GO(39)  \

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
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GMenuDetachFunc callback\n");
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
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GtkMenuPositionFunc callback\n");
    return NULL;
}

// GtkFunction
#define GO(A)   \
static uintptr_t my_gtkfunction_fct_##A = 0;                                \
static int my_gtkfunction_##A(void* data)                                   \
{                                                                           \
    return RunFunctionFmt(my_gtkfunction_fct_##A, "p", data);         \
}
SUPER()
#undef GO
static void* findGtkFunctionFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_gtkfunction_fct_##A == (uintptr_t)fct) return my_gtkfunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_gtkfunction_fct_##A == 0) {my_gtkfunction_fct_##A = (uintptr_t)fct; return my_gtkfunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GtkFunction callback\n");
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
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GtkClipboardGetFunc callback\n");
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
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GtkClipboardClearFunc callback\n");
    return NULL;
}

// GtkCallback
#define GO(A)   \
static uintptr_t my_gtkcallback_fct_##A = 0;                                \
static void my_gtkcallback_##A(void* widget, void* data)                    \
{                                                                           \
    RunFunctionFmt(my_gtkcallback_fct_##A, "pp", widget, data);       \
}
SUPER()
#undef GO
static void* findGtkCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_gtkcallback_fct_##A == (uintptr_t)fct) return my_gtkcallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_gtkcallback_fct_##A == 0) {my_gtkcallback_fct_##A = (uintptr_t)fct; return my_gtkcallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GtkCallback callback\n");
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
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GtkTextCharPredicate callback\n");
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
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 Toolbar callback\n");
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
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 BuilderConnect callback\n");
    return NULL;
}

// GtkCellLayoutDataFunc
#define GO(A)   \
static uintptr_t my_GtkCellLayoutDataFunc_fct_##A = 0;                                                      \
static void my_GtkCellLayoutDataFunc_##A(void* layout, void* cell, void* tree, void* iter, void* data)      \
{                                                                                                           \
    RunFunctionFmt(my_GtkCellLayoutDataFunc_fct_##A, "ppppp", layout, cell, tree, iter, data);        \
}
SUPER()
#undef GO
static void* findGtkCellLayoutDataFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GtkCellLayoutDataFunc_fct_##A == (uintptr_t)fct) return my_GtkCellLayoutDataFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GtkCellLayoutDataFunc_fct_##A == 0) {my_GtkCellLayoutDataFunc_fct_##A = (uintptr_t)fct; return my_GtkCellLayoutDataFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GtkCellLayoutDataFunc callback\n");
    return NULL;
}

// GtkTreeCellDataFunc
#define GO(A)   \
static uintptr_t my_GtkTreeCellDataFunc_fct_##A = 0;                                                        \
static void my_GtkTreeCellDataFunc_##A(void* column, void* cell, void* tree, void* iter, void* data)        \
{                                                                                                           \
    RunFunctionFmt(my_GtkTreeCellDataFunc_fct_##A, "ppppp", column, cell, tree, iter, data);          \
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
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GtkTreeCellDataFunc callback\n");
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
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GDestroyNotify callback\n");
    return NULL;
}

// GtkTreeModelForeachFunc
#define GO(A)   \
static uintptr_t my_GtkTreeModelForeachFunc_fct_##A = 0;                                                            \
static int my_GtkTreeModelForeachFunc_##A(void* model, void* path, void* iter, void* data)                          \
{                                                                                                                   \
    return (int)RunFunctionFmt(my_GtkTreeModelForeachFunc_fct_##A, "pppp", model, path, iter, data);          \
}
SUPER()
#undef GO
static void* findGtkTreeModelForeachFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GtkTreeModelForeachFunc_fct_##A == (uintptr_t)fct) return my_GtkTreeModelForeachFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GtkTreeModelForeachFunc_fct_##A == 0) {my_GtkTreeModelForeachFunc_fct_##A = (uintptr_t)fct; return my_GtkTreeModelForeachFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GtkTreeModelForeachFunc callback\n");
    return NULL;
}

// GtkTreeSelectionSelectedForeachFunc
#define GO(A)   \
static uintptr_t my_GtkTreeSelectionSelectedForeachFunc_fct_##A = 0;                                                                \
static int my_GtkTreeSelectionSelectedForeachFunc_##A(void* selection, void* path, void* iter, void* data)                          \
{                                                                                                                                   \
    return (int)RunFunctionFmt(my_GtkTreeSelectionSelectedForeachFunc_fct_##A, "pppp", selection, path, iter, data);          \
}
SUPER()
#undef GO
static void* findGtkTreeSelectionSelectedForeachFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GtkTreeSelectionSelectedForeachFunc_fct_##A == (uintptr_t)fct) return my_GtkTreeSelectionSelectedForeachFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GtkTreeSelectionSelectedForeachFunc_fct_##A == 0) {my_GtkTreeSelectionSelectedForeachFunc_fct_##A = (uintptr_t)fct; return my_GtkTreeSelectionSelectedForeachFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GtkTreeSelectionSelectedForeachFunc callback\n");
    return NULL;
}

// GtkClipboardReceivedFunc
#define GO(A)   \
static uintptr_t my_GtkClipboardReceivedFunc_fct_##A = 0;                                           \
static void my_GtkClipboardReceivedFunc_##A(void* clipboard, void* sel, void* data)                 \
{                                                                                                   \
    RunFunctionFmt(my_GtkClipboardReceivedFunc_fct_##A, "ppp", clipboard, sel, data);         \
}
SUPER()
#undef GO
static void* findGtkClipboardReceivedFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GtkClipboardReceivedFunc_fct_##A == (uintptr_t)fct) return my_GtkClipboardReceivedFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GtkClipboardReceivedFunc_fct_##A == 0) {my_GtkClipboardReceivedFunc_fct_##A = (uintptr_t)fct; return my_GtkClipboardReceivedFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GtkClipboardReceivedFunc callback\n");
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
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GtkClipboardTextReceivedFunc callback\n");
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
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GtkTreeViewSearchEqualFunc callback\n");
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
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GtkTreeIterCompareFunc callback\n");
    return NULL;
}

// GdkInputFunction
#define GO(A)   \
static uintptr_t my_GdkInputFunction_fct_##A = 0;                                       \
static void my_GdkInputFunction_##A(void* data, int source, int cond)                   \
{                                                                                       \
    RunFunctionFmt(my_GdkInputFunction_fct_##A, "pii", data, source, cond);       \
}
SUPER()
#undef GO
static void* findGdkInputFunctionFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GdkInputFunction_fct_##A == (uintptr_t)fct) return my_GdkInputFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GdkInputFunction_fct_##A == 0) {my_GdkInputFunction_fct_##A = (uintptr_t)fct; return my_GdkInputFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GdkInputFunction callback\n");
    return NULL;
}

// GtkCallbackMarshal
#define GO(A)   \
static uintptr_t my_GtkCallbackMarshal_fct_##A = 0;                                         \
static void my_GtkCallbackMarshal_##A(void* obj, void* data, uint32_t n, void* args)        \
{                                                                                           \
    RunFunctionFmt(my_GtkCallbackMarshal_fct_##A, "ppup", obj, data, n, args);        \
}
SUPER()
#undef GO
static void* findGtkCallbackMarshalFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GtkCallbackMarshal_fct_##A == (uintptr_t)fct) return my_GtkCallbackMarshal_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GtkCallbackMarshal_fct_##A == 0) {my_GtkCallbackMarshal_fct_##A = (uintptr_t)fct; return my_GtkCallbackMarshal_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GtkCallbackMarshal callback\n");
    return NULL;
}
// GtkPrinterFunc ...
#define GO(A)   \
static uintptr_t my_GtkPrinterFunc_fct_##A = 0;                                     \
static int my_GtkPrinterFunc_##A(void* a, void* b)                                  \
{                                                                                   \
    return (int)RunFunctionFmt(my_GtkPrinterFunc_fct_##A, "pp", a, b);        \
}
SUPER()
#undef GO
static void* find_GtkPrinterFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GtkPrinterFunc_fct_##A == (uintptr_t)fct) return my_GtkPrinterFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GtkPrinterFunc_fct_##A == 0) {my_GtkPrinterFunc_fct_##A = (uintptr_t)fct; return my_GtkPrinterFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GtkPrinterFunc callback\n");
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
// GtkPrintJobCompleteFunc ...
#define GO(A)   \
static uintptr_t my_GtkPrintJobCompleteFunc_fct_##A = 0;                            \
static void my_GtkPrintJobCompleteFunc_##A(void* a, void* b, void* c)               \
{                                                                                   \
    RunFunctionFmt(my_GtkPrintJobCompleteFunc_fct_##A, "ppp", a, b, c);       \
}
SUPER()
#undef GO
static void* find_GtkPrintJobCompleteFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GtkPrintJobCompleteFunc_fct_##A == (uintptr_t)fct) return my_GtkPrintJobCompleteFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GtkPrintJobCompleteFunc_fct_##A == 0) {my_GtkPrintJobCompleteFunc_fct_##A = (uintptr_t)fct; return my_GtkPrintJobCompleteFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GtkPrintJobCompleteFunc callback\n");
    return NULL;
}

// GtkLinkButtonUri ...
#define GO(A)   \
static uintptr_t my_GtkLinkButtonUri_fct_##A = 0;                               \
static void my_GtkLinkButtonUri_##A(void* a, void* b, void* c)                  \
{                                                                               \
    RunFunctionFmt(my_GtkLinkButtonUri_fct_##A, "ppp", a, b, c);          \
}
SUPER()
#undef GO
static void* find_GtkLinkButtonUri_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GtkLinkButtonUri_fct_##A == (uintptr_t)fct) return my_GtkLinkButtonUri_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GtkLinkButtonUri_fct_##A == 0) {my_GtkLinkButtonUri_fct_##A = (uintptr_t)fct; return my_GtkLinkButtonUri_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GtkLinkButtonUri callback\n");
    return NULL;
}
static void* reverse_GtkLinkButtonUri_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_GtkLinkButtonUri_##A == fct) return (void*)my_GtkLinkButtonUri_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFppp, fct, 0, NULL);
}

// GtkKeySnoopFunc ...
#define GO(A)   \
static uintptr_t my_GtkKeySnoopFunc_fct_##A = 0;                    \
static void my_GtkKeySnoopFunc_##A(void* a, void* b, void* c)       \
{                                                                   \
    RunFunctionFmt(my_GtkKeySnoopFunc_fct_##A, "ppp", a, b, c);     \
}
SUPER()
#undef GO
static void* find_GtkKeySnoopFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GtkKeySnoopFunc_fct_##A == (uintptr_t)fct) return my_GtkKeySnoopFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GtkKeySnoopFunc_fct_##A == 0) {my_GtkKeySnoopFunc_fct_##A = (uintptr_t)fct; return my_GtkKeySnoopFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GtkKeySnoopFunc callback\n");
    return NULL;
}
// Event
#define GO(A)   \
static uintptr_t my_Event_fct_##A = 0;                                                                              \
static void my_Event_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j)  \
{                                                                                                                   \
    RunFunctionFmt(my_Event_fct_##A, "pppppppppp", a, b, c, d, e, f, g, h, i, j);                                   \
}
SUPER()
#undef GO
static void* findEventFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_Event_fct_##A == (uintptr_t)fct) return my_Event_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Event_fct_##A == 0) {my_Event_fct_##A = (uintptr_t)fct; return my_Event_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk2 generic Event callback\n");
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
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 TranslateEvent callback\n");
    return NULL;
}

#undef SUPER

EXPORT uintptr_t my_gtk_signal_connect_full(x64emu_t* emu, void* object, void* name, void* c_handler, void* unsupported, void* data, void* closure, uint32_t signal, int after)
{
    return my->gtk_signal_connect_full(object, name, findEventFct(c_handler), unsupported, data, findGDestroyNotifyFct(closure), signal, after);
}

EXPORT void my_gtk_dialog_add_buttons(x64emu_t* emu, void* dialog, void* first, uintptr_t* b)
{
    void* btn = first;
    int n = 0;
    while(btn) {
        uintptr_t id = getVArgs(emu, 2, b, n++);
        my->gtk_dialog_add_button(dialog, btn, id);
        btn = (void*)getVArgs(emu, 2, b, n++);
    }
}

EXPORT void my_gtk_message_dialog_format_secondary_text(x64emu_t* emu, void* dialog, void* fmt, void* b)
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

EXPORT void my_gtk_message_dialog_format_secondary_markup(x64emu_t* emu, void* dialog, void* fmt, void* b)
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

EXPORT void my_gtk_list_store_set_valist(x64emu_t* emu, void* list, void* iter, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    my->gtk_list_store_set_valist(list, iter, VARARGS);
}

EXPORT void my_gtk_list_store_set(x64emu_t* emu, void* list, void* iter, uintptr_t* b)
{
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    my->gtk_list_store_set_valist(list, iter, VARARGS);
}

EXPORT void my_gtk_widget_style_get_valist(x64emu_t* emu, void* widget, void* first, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    my->gtk_widget_style_get_valist(widget, first, VARARGS);
}

EXPORT void my_gtk_widget_style_get(x64emu_t* emu, void* widget, void* first, uintptr_t* b)
{
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    my->gtk_widget_style_get_valist(widget, first, VARARGS);
}

EXPORT void* my_gtk_type_class(x64emu_t* emu, size_t type)
{
    void* class = my->gtk_type_class(type);
    return wrapCopyGTKClass(class, type);
}

EXPORT void my_gtk_init(x64emu_t* emu, void* argc, void* argv)
{
    my->gtk_init(argc, argv);
    my_checkGlobalGdkDisplay();
    AutoBridgeGtk(my->g_type_class_ref, my->g_type_class_unref);
}

EXPORT int my_gtk_init_check(x64emu_t* emu, void* argc, void* argv)
{
    int ret = my->gtk_init_check(argc, argv);
    my_checkGlobalGdkDisplay();
    AutoBridgeGtk(my->g_type_class_ref, my->g_type_class_unref);
    return ret;
}

EXPORT int my_gtk_init_with_args(x64emu_t* emu, void* argc, void* argv, void* param, void* entries, void* trans, void* error)
{
    int ret = my->gtk_init_with_args(argc, argv, param, entries, trans, error);
    my_checkGlobalGdkDisplay();
    AutoBridgeGtk(my->g_type_class_ref, my->g_type_class_unref);
    return ret;
}

EXPORT void my_gtk_menu_attach_to_widget(x64emu_t* emu, void* menu, void* widget, void* f)
{
    my->gtk_menu_attach_to_widget(menu, widget, findMenuDetachFct(f));
}

EXPORT void my_gtk_menu_popup(x64emu_t* emu, void* menu, void* shell, void* item, void* f, void* data, uint32_t button, uint32_t time_)
{
    my->gtk_menu_popup(menu, shell, item, findMenuPositionFct(f), data, button, time_);
}

EXPORT uint32_t my_gtk_timeout_add(x64emu_t* emu, uint32_t interval, void* f, void* data)
{
    return my->gtk_timeout_add(interval, findGtkFunctionFct(f), data);
}

EXPORT int my_gtk_clipboard_set_with_data(x64emu_t* emu, void* clipboard, void* target, uint32_t n, void* f_get, void* f_clear, void* data)
{
    return my->gtk_clipboard_set_with_data(clipboard, target, n, findClipboadGetFct(f_get), findClipboadClearFct(f_clear), data);
}

EXPORT int my_gtk_clipboard_set_with_owner(x64emu_t* emu, void* clipboard, void* target, uint32_t n, void* f_get, void* f_clear, void* data)
{
    return my->gtk_clipboard_set_with_owner(clipboard, target, n, findClipboadGetFct(f_get), findClipboadClearFct(f_clear), data);
}

EXPORT void my_gtk_stock_set_translate_func(x64emu_t* emu, void* domain, void* f, void* data, void* notify)
{
    my->gtk_stock_set_translate_func(domain, findTranslateEvent(f), data, findGDestroyNotifyFct(notify));
}

EXPORT void my_gtk_container_forall(x64emu_t* emu, void* container, void* f, void* data)
{
    my->gtk_container_forall(container, findGtkCallbackFct(f), data);
}

EXPORT void my_gtk_tree_view_set_search_equal_func(x64emu_t* emu, void* tree_view, void* f, void* data, void* notify)
{
    my->gtk_tree_view_set_search_equal_func(tree_view, findGtkTreeViewSearchEqualFuncFct(f), data, findGDestroyNotifyFct(notify));
}

EXPORT int my_gtk_text_iter_backward_find_char(x64emu_t* emu, void* iter, void* f, void* data, void* limit)
{
    return my->gtk_text_iter_backward_find_char(iter, findGtkTextCharPredicateFct(f), data, limit);
}

EXPORT int my_gtk_text_iter_forward_find_char(x64emu_t* emu, void* iter, void* f, void* data, void* limit)
{
    return my->gtk_text_iter_forward_find_char(iter, findGtkTextCharPredicateFct(f), data, limit);
}

EXPORT void* my_gtk_toolbar_append_item(x64emu_t* emu, void* toolbar, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data)
{
    return my->gtk_toolbar_append_item(toolbar, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data);
}

EXPORT void* my_gtk_toolbar_prepend_item(x64emu_t* emu, void* toolbar, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data)
{
    return my->gtk_toolbar_prepend_item(toolbar, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data);
}

EXPORT void* my_gtk_toolbar_insert_item(x64emu_t* emu, void* toolbar, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data, int position)
{
    return my->gtk_toolbar_insert_item(toolbar, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data, position);
}

EXPORT void* my_gtk_toolbar_append_element(x64emu_t* emu, void* toolbar, size_t type, void* widget, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data)
{
    return my->gtk_toolbar_append_element(toolbar, type, widget, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data);
}

EXPORT void* my_gtk_toolbar_prepend_element(x64emu_t* emu, void* toolbar, size_t type, void* widget, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data)
{
    return my->gtk_toolbar_prepend_element(toolbar, type, widget, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data);
}

EXPORT void* my_gtk_toolbar_insert_element(x64emu_t* emu, void* toolbar, size_t type, void* widget, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data, int position)
{
    return my->gtk_toolbar_insert_element(toolbar, type, widget, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data, position);
}

EXPORT void* my_gtk_toolbar_insert_stock(x64emu_t* emu, void* toolbar, void* stock_id, void* tooltip_text, void* tooltip_private, void* f, void* data, int position)
{
    return my->gtk_toolbar_insert_stock(toolbar, stock_id, tooltip_text, tooltip_private, findToolbarFct(f), data, position);
}

EXPORT void my_gtk_tree_sortable_set_sort_func(x64emu_t* emu, void* sortable, int id, void* f, void* data, void* notify)
{
    my->gtk_tree_sortable_set_sort_func(sortable, id, findGtkTreeIterCompareFuncFct(f), data, findGDestroyNotifyFct(notify));
}

EXPORT void my_gtk_tree_sortable_set_default_sort_func(x64emu_t* emu, void* sortable, void* f, void* data, void* notify)
{
    my->gtk_tree_sortable_set_default_sort_func(sortable, findGtkTreeIterCompareFuncFct(f), data, findGDestroyNotifyFct(notify));
}

EXPORT size_t my_gtk_type_unique(x64emu_t* emu, size_t parent, my_GtkTypeInfo_t* gtkinfo)
{
    return my->gtk_type_unique((void*)parent, findFreeGtkTypeInfo(gtkinfo, parent));
}

EXPORT unsigned long my_gtk_signal_connect(x64emu_t* emu, void* object, void* name, void* func, void* data)
{
    return my_gtk_signal_connect_full(emu, object, name, func, NULL, data, NULL, 0, 0);
}

EXPORT void my_gtk_object_set_data_full(x64emu_t* emu, void* object, void* key, void* data, void* notify)
{
    my->gtk_object_set_data_full(object, key, data, findGDestroyNotifyFct(notify));
}

EXPORT float my_gtk_spin_button_get_value_as_float(x64emu_t* emu, void* spinner)
{
    return my->gtk_spin_button_get_value(spinner);
}

static size_t gtk1Type(gtkx112_my_t *my, size_t type)
{
    if (type==21)
        return my->gtk_object_get_type();
    return type;    // other changes needed?
}

typedef struct dummy_gtk1_button_s {
    size_t type;
    int dummy[14];
    void* label;
} dummy_gtk1_button_t;

EXPORT void* my_gtk_type_check_object_cast(x64emu_t* emu, void* obj, size_t type)
{
    if(my->gtk1 && type==my->gtk_bin_get_type())
    {
        // check if obj is a button...
        if(my->g_type_check_instance_cast(obj, my->gtk_button_get_type()))
        {
            // make some bad casting of button from gtkv2 to button from gtkv1, the "bin" sized changed, and is shorter now!
            // shogo launcher acces label from button directly from the cast (+0x3c)
            static dummy_gtk1_button_t dummy = {0};
            dummy.type = my->gtk_button_get_type();
            void **p = my->g_type_check_instance_cast(obj, my->gtk_bin_get_type());
            // style is 0x06
            dummy.label = p[0x11];
            return &dummy;
        }
    }

    return my->g_type_check_instance_cast(obj, gtk1Type(my, type));
}

EXPORT void my_gtk_builder_connect_signals_full(x64emu_t* emu, void* builder, void* f, void* data)
{
    my->gtk_builder_connect_signals_full(builder, findBuilderConnectFct(f), data);
}

typedef struct my_GSList_s {
  void*               data;
  struct my_GSList_s *next;
} my_GSList_t;

EXPORT void my_gtk_binding_entry_add_signal(x64emu_t* emu, void* binding, uint32_t keyval, int mod, void* name, uint32_t n, uintptr_t* b)
{
    if(!n) {
        my->gtk_binding_entry_add_signal(binding, keyval, mod, name, 0);
        return;
    }
    // build the list (using calloc and not box_calloc, it's program space...)
    my_GSList_t *list = calloc(n, sizeof(my_GSList_t));
    for(uint32_t i=0; i<n; ++i) {
        list[i].data = (void*)getVArgs(emu, 5, b, i);
        list[i].next = (i==(n-1))?NULL:&list[i+1];
    }

    my->gtk_binding_entry_add_signall(binding, keyval, mod, name, list);

    free(list);
}

EXPORT void my_gtk_container_foreach(x64emu_t* emu, void* container, void* f, void* data)
{
    my->gtk_container_foreach(container, findGtkCallbackFct(f), data);
}

EXPORT void my_gtk_cell_layout_set_cell_data_func(x64emu_t* emu, void* layout, void* cell, void* f, void* data, void* notify)
{
    my->gtk_cell_layout_set_cell_data_func(layout, cell, findGtkCellLayoutDataFuncFct(f), data, findGDestroyNotifyFct(notify));
}

EXPORT void my_gtk_tree_view_column_set_cell_data_func(x64emu_t* emu, void* column, void* cell, void* f, void* data, void* notify)
{
    my->gtk_tree_view_column_set_cell_data_func(column, cell, findGtkTreeCellDataFuncFct(f), data, findGDestroyNotifyFct(notify));
}

typedef struct my_ConnectArgs_s
{
    gtkx112_my_t *my;
    x64emu_t* emu;
    void* module;
    void* data;
} my_ConnectArgs_t;
// defined in wrappedgobject2.c
uintptr_t my_g_signal_connect_data(x64emu_t* emu, void* instance, void* detailed, void* c_handler, void* data, void* closure, uint32_t flags);

static void my_gtk_builder_connect_signals_custom(void* builder,
                                                  void* object,
                                                  char* signal_name,
                                                  char* handler_name,
                                                  void* connect_object,
                                                  int   flags,
                                                  my_ConnectArgs_t* args)
{
    /* Only error out for missing GModule support if we've not
    * found the symbols explicitly added with gtk_builder_add_callback_symbol()
    */
    void* func = NULL;
    printf_log(LOG_DEBUG, "signal \"%s\" from \"%s\" connection, connect_object=%p\n", signal_name, handler_name, connect_object);

    uintptr_t offs = 0;
    uintptr_t end = 0;
    GetGlobalSymbolStartEnd(my_context->maplib, handler_name, &offs, &end, NULL, -1, NULL, 0, NULL);
    if(!offs) {
        if (args->module == NULL)
            args->my->g_log("Gtk", 1<<2 ,"gtk_builder_connect_signals() requires working GModule");
        if (!args->my->g_module_symbol(args->module, handler_name, (void*)&func))
        {
            args->my->g_log("Gtk", 1<<4, "Could not find signal handler '%s'.  Did you compile with -rdynamic?", handler_name);
            return;
        }
        if (connect_object)
            args->my->g_signal_connect_object(object, signal_name, func, connect_object, flags);
        else
            args->my->g_signal_connect_data(object, signal_name, func, args->data, NULL, flags);
    } else {
        if(connect_object) {
            printf_log(LOG_NONE, "Error: connect custom signal to object unsupported\n");
        } else
            my_g_signal_connect_data(args->emu, object, signal_name, (void*)offs, args->data, NULL, flags);
    }
}

EXPORT void my_gtk_builder_connect_signals(x64emu_t* emu, void* builder, void* data)
{
    my_ConnectArgs_t args;

    args.my = my;
    args.emu = emu;
    args.data = data;
    if (my->g_module_supported())
        args.module = my->g_module_open(NULL, 1);
    my->gtk_builder_connect_signals_full(builder, my_gtk_builder_connect_signals_custom, &args);
    if (args.module)
        my->g_module_close(args.module);
}

typedef struct my_GtkActionEntry_s {
  const char* name;
  const char* stock_id;
  const char* label;
  const char* accelerator;
  const char* tooltip;
  void*       callback;
} my_GtkActionEntry_t;

EXPORT void my_gtk_action_group_add_actions(x64emu_t* emu, void* action_group, my_GtkActionEntry_t* entries, uint32_t n, void* data)
{
    my_GtkActionEntry_t myentries[n];
    memcpy(myentries, entries, n*sizeof(my_GtkActionEntry_t));
    for(uint32_t i=0; i<n; ++i)
        myentries[i].callback = findGtkCallbackFct(entries[i].callback);
    my->gtk_action_group_add_actions(action_group, myentries, n, data);
}

EXPORT void my_gtk_action_group_add_actions_full(x64emu_t* emu, void* action_group, my_GtkActionEntry_t* entries, uint32_t n, void* data, void* destroy)
{
    my_GtkActionEntry_t myentries[n];
    memcpy(myentries, entries, n*sizeof(my_GtkActionEntry_t));
    for(uint32_t i=0; i<n; ++i)
        myentries[i].callback = findGtkCallbackFct(entries[i].callback);
    my->gtk_action_group_add_actions_full(action_group, myentries, n, data, findGDestroyNotifyFct(destroy));
}

EXPORT void my_gtk_tree_model_foreach(x64emu_t* emu, void* model, void* f, void* data)
{
    my->gtk_tree_model_foreach(model, findGtkTreeModelForeachFuncFct(f), data);
}

EXPORT void my_gtk_tree_selection_selected_foreach(x64emu_t* emu, void* selection, void* f, void* data)
{
    my->gtk_tree_selection_selected_foreach(selection, findGtkTreeSelectionSelectedForeachFuncFct(f), data);
}

EXPORT void my_gtk_clipboard_request_contents(x64emu_t* emu, void* clipboard, void* target, void* f, void* data)
{
    my->gtk_clipboard_request_contents(clipboard, target, findGtkClipboardReceivedFuncFct(f), data);
}


EXPORT void my_gtk_clipboard_request_text(x64emu_t* emu, void* clipboard, void* f, void* data)
{
    my->gtk_clipboard_request_text(clipboard, findGtkClipboardTextReceivedFuncFct(f), data);
}

EXPORT uint32_t my_gtk_input_add_full(x64emu_t* emu, int source, int condition, void* func, void* marshal, void* data, void* destroy)
{
    return my->gtk_input_add_full(source, condition, findGdkInputFunctionFct(func), findGtkCallbackMarshalFct(marshal), data, findGDestroyNotifyFct(destroy));
}

EXPORT void my_gtk_list_store_insert_with_values(x64emu_t* emu, void* store, void* iter, int pos, uintptr_t* b)
{
    int n = 0;
    //sizeof(my_GValue_t) is 24, so 3 pointers, that will be directly in stack
    // so cannot use getVArg for this...
    int col;
    do {
        if(n==0) col = S_ECX; else col = (int)b[n*4-1];
        ++n;
    } while(col==-1);
    int columns[n];
    my_GValue_t values[n];
    for(int i=0; i<n; ++i) {
        if(i==0) col = S_ECX; else col = (int)b[i*4-1];
        columns[i] = col;
        values[i] = *(my_GValue_t*)&b[i*4];
    }
    my->gtk_list_store_insert_with_valuesv(store, iter, pos, columns, values, n);
}

EXPORT void* my_gtk_list_store_new(x64emu_t* emu, int n, uintptr_t* b)
{
    uintptr_t a[n];
    for(int i=0; i<n; ++i)
        a[i] = getVArgs(emu, 1, b, i);
    return my->gtk_list_store_newv(n, a);
}

EXPORT void* my_gtk_tree_store_new(x64emu_t* emu, int n, uintptr_t* b)
{
    uintptr_t a[n];
    for(int i=0; i<n; ++i)
        a[i] = getVArgs(emu, 1, b, i);
    return my->gtk_tree_store_newv(n, a);
}

EXPORT void my_gtk_tree_model_get_valist(x64emu_t* emu, void* tree, void* iter, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    my->gtk_tree_model_get_valist(tree, iter, VARARGS);
}

EXPORT void my_gtk_tree_model_get(x64emu_t* emu, void* tree, void* iter, uintptr_t* b)
{
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    my->gtk_tree_model_get_valist(tree, iter, VARARGS);
}

EXPORT void* my_gtk_link_button_set_uri_hook(x64emu_t* emu, void* f, void* data, void* d)
{
    void* ret = my->gtk_link_button_set_uri_hook(find_GtkLinkButtonUri_Fct(f), data, findGDestroyNotifyFct(d));
    return reverse_GtkLinkButtonUri_Fct(ret);
}

EXPORT void my_gtk_enumerate_printers(x64emu_t* emu, void* f, void* data, void* d, int wait)
{
    (void)emu;
    my->gtk_enumerate_printers(find_GtkPrinterFunc_Fct(f), data, findGDestroyNotifyFct(d), wait);
}

EXPORT void my_gtk_file_filter_add_custom(x64emu_t* emu, void* filter, uint32_t needed, void* f, void* data, void* d)
{
    (void)emu;
    my->gtk_file_filter_add_custom(filter, needed, find_GtkFileFilterFunc_Fct(f), data, findGDestroyNotifyFct(d));
}

EXPORT void my_gtk_print_job_send(x64emu_t* emu, void* job, void* f, void* data, void* d)
{
    (void)emu;
    my->gtk_print_job_send(job, find_GtkPrintJobCompleteFunc_Fct(f), data, findGDestroyNotifyFct(d));
}

EXPORT uint32_t my_gtk_key_snooper_install(x64emu_t* emu, void* f, void* data)
{
    (void)emu;
    return my->gtk_key_snooper_install(find_GtkKeySnoopFunc_Fct(f), data);
}

static void addGtk2Alternate(library_t* lib)
{
    #define GO(A, W) AddAutomaticBridge(lib->w.bridge, W, dlsym(lib->w.lib, #A), 0, #A)
    GO(gtk_marshal_BOOLEAN__POINTER,                            vFppuppp);
    GO(gtk_marshal_BOOLEAN__POINTER_INT_INT,                    vFppuppp);
    GO(gtk_marshal_BOOLEAN__POINTER_INT_INT_UINT,               vFppuppp);
    GO(gtk_marshal_BOOLEAN__POINTER_POINTER_INT_INT,            vFppuppp);
    GO(gtk_marshal_BOOLEAN__POINTER_STRING_STRING_POINTER,      vFppuppp);
    GO(gtk_marshal_BOOLEAN__VOID,                               vFppuppp);
    GO(gtk_marshal_ENUM__ENUM,                                  vFppuppp);
    GO(gtk_marshal_INT__POINTER,                                vFppuppp);
    GO(gtk_marshal_INT__POINTER_CHAR_CHAR,                      vFppuppp);
    GO(gtk_marshal_VOID__ENUM_FLOAT,                            vFppuppp);
    GO(gtk_marshal_VOID__ENUM_FLOAT_BOOLEAN,                    vFppuppp);
    GO(gtk_marshal_VOID__INT_INT,                               vFppuppp);
    GO(gtk_marshal_VOID__INT_INT_POINTER,                       vFppuppp);
    GO(gtk_marshal_VOID__POINTER_INT,                           vFppuppp);
    GO(gtk_marshal_VOID__POINTER_INT_INT_POINTER_UINT_UINT,     vFppuppp);
    GO(gtk_marshal_VOID__POINTER_POINTER,                       vFppuppp);
    GO(gtk_marshal_VOID__POINTER_POINTER_POINTER,               vFppuppp);
    GO(gtk_marshal_VOID__POINTER_POINTER_UINT_UINT,             vFppuppp);
    GO(gtk_marshal_VOID__POINTER_STRING_STRING,                 vFppuppp);
    GO(gtk_marshal_VOID__POINTER_UINT,                          vFppuppp);
    GO(gtk_marshal_VOID__POINTER_UINT_ENUM,                     vFppuppp);
    GO(gtk_marshal_VOID__POINTER_UINT_UINT,                     vFppuppp);
    GO(gtk_marshal_VOID__STRING_INT_POINTER,                    vFppuppp);
    GO(gtk_marshal_VOID__UINT_POINTER_UINT_ENUM_ENUM_POINTER,   vFppuppp);
    GO(gtk_marshal_VOID__UINT_POINTER_UINT_UINT_ENUM,           vFppuppp);
    GO(gtk_marshal_VOID__UINT_STRING,                           vFppuppp);
    #undef GO
}

#define PRE_INIT \
    if (BOX64ENV(nogtk)) return -2;

#define CUSTOM_INIT \
    addGtk2Alternate(lib);                                                      \
    SetGtkObjectID(my->gtk_object_get_type());                                  \
    SetGtkWidget2ID(my->gtk_widget_get_type());                                 \
    SetGtkContainer2ID(my->gtk_container_get_type());                           \
    SetGtkActionID(my->gtk_action_get_type());                                  \
    SetGtkMisc2ID(my->gtk_misc_get_type());                                     \
    SetGtkLabel2ID(my->gtk_label_get_type());                                   \
    SetGtkTreeView2ID(my->gtk_tree_view_get_type());                            \
    SetGtkBin2ID(my->gtk_bin_get_type());                                       \
    SetGtkWindow2ID(my->gtk_window_get_type());                                 \
    SetGtkTable2ID(my->gtk_table_get_type());                                   \
    SetGtkFixed2ID(my->gtk_fixed_get_type());                                   \
    SetGtkButton2ID(my->gtk_button_get_type());                                 \
    SetGtkComboBox2ID(my->gtk_combo_box_get_type());                            \
    SetGtkToggleButton2ID(my->gtk_toggle_button_get_type());                    \
    SetGtkCheckButton2ID(my->gtk_check_button_get_type());                      \
    SetGtkEntry2ID(my->gtk_entry_get_type());                                   \
    SetGtkSpinButton2ID(my->gtk_spin_button_get_type());                        \
    SetGtkProgress2ID(my->gtk_progress_get_type());                             \
    SetGtkProgressBar2ID(my->gtk_progress_bar_get_type());                      \
    SetGtkFrame2ID(my->gtk_frame_get_type());                                   \
    SetGtkMenuShell2ID(my->gtk_menu_shell_get_type());                          \
    SetGtkMenuBar2ID(my->gtk_menu_bar_get_type());                              \
    SetGtkTextView2ID(my->gtk_text_view_get_type());                            \
    SetGtkNotebook2ID(my->gtk_notebook_get_type());                             \
    SetGtkCellRenderer2ID(my->gtk_cell_renderer_get_type());

#define NEEDED_LIBS "libgdk-x11-2.0.so.0", "libpangocairo-1.0.so.0"

#include "wrappedlib_init.h"
