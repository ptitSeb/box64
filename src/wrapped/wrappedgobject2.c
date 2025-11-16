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
#include "gtkclass.h"
#include "myalign.h"

const char* gobject2Name = "libgobject-2.0.so.0";
#define ALTNAME "libgobject-2.0.so"

#define LIBNAME gobject2

typedef size_t(*LFv_t)(void);
typedef void*(*pFL_t)(size_t);

#define ADDED_FUNCTIONS()           \
 GO(g_object_get_type, LFv_t)       \
 GO(g_initially_unowned_get_type, LFv_t)    \
 GO(g_type_name, pFL_t)             \
 GO(g_type_parent, LFv_t)           \
 GO(g_type_class_peek, pFL_t)       \

#include "generated/wrappedgobject2types.h"
static void addGObject2Alternate(library_t* lib);

#define ADDED_INIT() \
    addGObject2Alternate(lib);   \
    InitGTKClass(lib->w.bridge);

#define ADDED_FINI() \
    FiniGTKClass();

#include "wrappercallback.h"

static void addGObject2Alternate(library_t* lib)
{
    #define GO(A, W) AddAutomaticBridge(lib->w.bridge, W, dlsym(lib->w.lib, #A), 0, #A)
    GO(g_cclosure_marshal_VOID__VOID,               vFppuppp);
    GO(g_cclosure_marshal_VOID__BOOLEAN,            vFppuppp);
    GO(g_cclosure_marshal_VOID__UCHAR,              vFppuppp);
    GO(g_cclosure_marshal_VOID__INT,                vFppuppp);
    GO(g_cclosure_marshal_VOID__UINT,               vFppuppp);
    GO(g_cclosure_marshal_VOID__LONG,               vFppuppp);
    GO(g_cclosure_marshal_VOID__ULONG,              vFppuppp);
    GO(g_cclosure_marshal_VOID__ENUM,               vFppuppp);
    GO(g_cclosure_marshal_VOID__FLAGS,              vFppuppp);
    GO(g_cclosure_marshal_VOID__FLOAT,              vFppuppp);
    GO(g_cclosure_marshal_VOID__DOUBLE,             vFppuppp);
    GO(g_cclosure_marshal_VOID__STRING,             vFppuppp);
    GO(g_cclosure_marshal_VOID__PARAM,              vFppuppp);
    GO(g_cclosure_marshal_VOID__BOXED,              vFppuppp);
    GO(g_cclosure_marshal_VOID__POINTER,            vFppuppp);
    GO(g_cclosure_marshal_VOID__OBJECT,             vFppuppp);
    GO(g_cclosure_marshal_VOID__VARIANT,            vFppuppp);
    GO(g_cclosure_marshal_STRING__OBJECT_POINTER,   vFppuppp);
    GO(g_cclosure_marshal_VOID__UINT_POINTER,       vFppuppp);
    GO(g_cclosure_marshal_BOOLEAN__FLAGS,           vFppuppp);
    GO(g_cclosure_marshal_BOOLEAN__BOXED_BOXED,     vFppuppp);
    GO(g_cclosure_marshal_generic_va,               vFpppppip);
    GO(g_cclosure_marshal_VOID__VOIDv,              vFpppppip);
    GO(g_cclosure_marshal_VOID__BOOLEANv,           vFpppppip);
    GO(g_cclosure_marshal_VOID__CHARv,              vFpppppip);
    GO(g_cclosure_marshal_VOID__UCHARv,             vFpppppip);
    GO(g_cclosure_marshal_VOID__INTv,               vFpppppip);
    GO(g_cclosure_marshal_VOID__UINTv,              vFpppppip);
    GO(g_cclosure_marshal_VOID__LONGv,              vFpppppip);
    GO(g_cclosure_marshal_VOID__ULONGv,             vFpppppip);
    GO(g_cclosure_marshal_VOID__ENUMv,              vFpppppip);
    GO(g_cclosure_marshal_VOID__FLAGSv,             vFpppppip);
    GO(g_cclosure_marshal_VOID__FLOATv,             vFpppppip);
    GO(g_cclosure_marshal_VOID__DOUBLEv,            vFpppppip);
    GO(g_cclosure_marshal_VOID__STRINGv,            vFpppppip);
    GO(g_cclosure_marshal_VOID__PARAMv,             vFpppppip);
    GO(g_cclosure_marshal_VOID__BOXEDv,             vFpppppip);
    GO(g_cclosure_marshal_VOID__POINTERv,           vFpppppip);
    GO(g_cclosure_marshal_VOID__OBJECTv,            vFpppppip);
    GO(g_cclosure_marshal_VOID__VARIANTv,           vFpppppip);
    GO(g_cclosure_marshal_STRING__OBJECT_POINTERv,  vFpppppip);
    GO(g_cclosure_marshal_VOID__UINT_POINTERv,      vFpppppip);
    GO(g_cclosure_marshal_BOOLEAN__FLAGSv,          vFpppppip);
    GO(g_cclosure_marshal_BOOLEAN__BOXED_BOXEDv,    vFpppppip);
    #undef GO
}

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

#define GO(A)   \
static uintptr_t my_copy_fct_##A = 0;                                     \
static void* my_copy_##A(void* data)                                      \
{                                                                         \
    return (void*)RunFunctionFmt(my_copy_fct_##A, "p", data); \
}
SUPER()
#undef GO
static void* findCopyFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_copy_fct_##A == (uintptr_t)fct) return my_copy_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_copy_fct_##A == 0) {my_copy_fct_##A = (uintptr_t)fct; return my_copy_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject Boxed Copy callback\n");
    return NULL;
}

#define GO(A)   \
static uintptr_t my_free_fct_##A = 0;                       \
static void my_free_##A(void* data)                         \
{                                                           \
    RunFunctionFmt(my_free_fct_##A, "p", data); \
}
SUPER()
#undef GO
static void* findFreeFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_free_fct_##A == (uintptr_t)fct) return my_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_fct_##A == 0) {my_free_fct_##A = (uintptr_t)fct; return my_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject Boxed Free callback\n");
    return NULL;
}
// GSignalAccumulator
#define GO(A)   \
static uintptr_t my_accumulator_fct_##A = 0;                                                                     \
static int my_accumulator_##A(void* ihint, void* return_accu, void* handler_return, void* data)                  \
{                                                                                                                \
    return RunFunctionFmt(my_accumulator_fct_##A, "pppp", ihint, return_accu, handler_return, data); \
}
SUPER()
#undef GO
static void* findAccumulatorFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_accumulator_fct_##A == (uintptr_t)fct) return my_accumulator_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_accumulator_fct_##A == 0) {my_accumulator_fct_##A = (uintptr_t)fct; return my_accumulator_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject Signal Accumulator callback\n");
    return NULL;
}

// GClosureMarshal
#define GO(A)   \
static uintptr_t my_marshal_fct_##A = 0;                                                                        \
static void my_marshal_##A(void* closure, void* return_value, uint32_t n, void* values, void* hint, void* data) \
{                                                                                                               \
    RunFunctionFmt(my_marshal_fct_##A, "ppuppp", closure, return_value, n, values, hint, data);     \
}
SUPER()
#undef GO
static void* findMarshalFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_marshal_fct_##A == (uintptr_t)fct) return my_marshal_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_marshal_fct_##A == 0) {AddAutomaticBridge(my_lib->w.bridge, vFppuppp, my_marshal_##A, 0, #A); my_marshal_fct_##A = (uintptr_t)fct; return my_marshal_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject Closure Marshal callback\n");
    return NULL;
}

// GClosureNotify
#define GO(A)   \
static uintptr_t my_GClosureNotify_fct_##A = 0;                               \
static int my_GClosureNotify_func_##A(void* a, void* b)                       \
{                                                                             \
    return RunFunctionFmt(my_GClosureNotify_fct_##A, "pp", a, b); \
}
SUPER()
#undef GO
static void* findGClosureNotify_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GClosureNotify_fct_##A == (uintptr_t)fct) return my_GClosureNotify_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GClosureNotify_fct_##A == 0) {my_GClosureNotify_fct_##A = (uintptr_t)fct; return my_GClosureNotify_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject GClosureNotify callback\n");
    return NULL;
}

// GValueTransform
#define GO(A)   \
static uintptr_t my_valuetransform_fct_##A = 0;                            \
static void my_valuetransform_##A(void* src, void* dst)                    \
{                                                                          \
    RunFunctionFmt(my_valuetransform_fct_##A, "pp", src, dst); \
}
SUPER()
#undef GO
static void* findValueTransformFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_valuetransform_fct_##A == (uintptr_t)fct) return my_valuetransform_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_valuetransform_fct_##A == 0) {my_valuetransform_fct_##A = (uintptr_t)fct; return my_valuetransform_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject Value Transform callback\n");
    return NULL;
}

// GDestroyFunc ...
#define GO(A)   \
static uintptr_t my_destroyfunc_fct_##A = 0;                               \
static int my_destroyfunc_##A(void* a, void* b)                            \
{                                                                          \
    return RunFunctionFmt(my_destroyfunc_fct_##A, "pp", a, b); \
}
SUPER()
#undef GO
static void* findDestroyFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_destroyfunc_fct_##A == (uintptr_t)fct) return my_destroyfunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_destroyfunc_fct_##A == 0) {my_destroyfunc_fct_##A = (uintptr_t)fct; return my_destroyfunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject GDestroyNotify callback\n");
    return NULL;
}

// GWeakNotify
#define GO(A)   \
static uintptr_t my_weaknotifyfunc_fct_##A = 0;                               \
static int my_weaknotifyfunc_##A(void* a, void* b)                            \
{                                                                             \
    return RunFunctionFmt(my_weaknotifyfunc_fct_##A, "pp", a, b); \
}
SUPER()
#undef GO
static void* findWeakNotifyFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_weaknotifyfunc_fct_##A == (uintptr_t)fct) return my_weaknotifyfunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_weaknotifyfunc_fct_##A == 0) {my_weaknotifyfunc_fct_##A = (uintptr_t)fct; return my_weaknotifyfunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject GWeakNotify callback\n");
    return NULL;
}

// GToggleNotify
#define GO(A)   \
static uintptr_t my_togglenotifyfunc_fct_##A = 0;                       \
static int my_togglenotifyfunc_##A(void* a, void* b, int c)             \
{                                                                       \
    return RunFunctionFmt(my_togglenotifyfunc_fct_##A, "ppi", a, b, c); \
}
SUPER()
#undef GO
static void* findToggleNotifyFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_togglenotifyfunc_fct_##A == (uintptr_t)fct) return my_togglenotifyfunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_togglenotifyfunc_fct_##A == 0) {my_togglenotifyfunc_fct_##A = (uintptr_t)fct; return my_togglenotifyfunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject GToggleNotify callback\n");
    return NULL;
}

// GParamSpecTypeInfo....
// First the structure GParamSpecTypeInfo statics, with paired x64 source pointer
typedef struct my_GParamSpecTypeInfo_s {
  /* type system portion */
  uint16_t  instance_size;
  uint16_t  n_preallocs;
  void      (*instance_init)    (void* pspec);
  size_t    value_type;
  void      (*finalize)         (void* pspec);
  void      (*value_set_default)(void* pspec, void* value);
  int       (*value_validate)   (void* pspec, void* value);
  int       (*values_cmp)       (void* pspec, void* value1, void* value2);
} my_GParamSpecTypeInfo_t;

#define GO(A)   \
static my_GParamSpecTypeInfo_t     my_GParamSpecTypeInfo_##A = {0};   \
static my_GParamSpecTypeInfo_t   *ref_GParamSpecTypeInfo_##A = NULL;
SUPER()
#undef GO
// then the static functions callback that may be used with the structure, but dispatch also have a callback...
#define GO(A)   \
static uintptr_t fct_funcs_instance_init_##A = 0;                           \
static void my_funcs_instance_init_##A(void* pspec) {                       \
    RunFunctionFmt(fct_funcs_instance_init_##A, "p", pspec);    \
}   \
static uintptr_t fct_funcs_finalize_##A = 0;                                \
static void my_funcs_finalize_##A(void* pspec) {                            \
    RunFunctionFmt(fct_funcs_finalize_##A, "p", pspec);         \
}   \
static uintptr_t fct_funcs_value_set_default_##A = 0;                   \
static void my_funcs_value_set_default_##A(void* pspec, void* value) {  \
    RunFunctionFmt(fct_funcs_value_set_default_##A, "pp", pspec, value);    \
}   \
static uintptr_t fct_funcs_value_validate_##A = 0;                      \
static int my_funcs_value_validate_##A(void* pspec, void* value) {      \
    return (int)RunFunctionFmt(fct_funcs_value_validate_##A, "pp", pspec, value); \
}   \
static uintptr_t fct_funcs_values_cmp_##A = 0;                          \
static int my_funcs_values_cmp_##A(void* pspec, void* value1, void* value2) {   \
    return (int)RunFunctionFmt(fct_funcs_values_cmp_##A, "ppp", pspec, value1, value2); \
}

SUPER()
#undef GO
// and now the get slot / assign... Taking into account that the desired callback may already be a wrapped one (so unwrapping it)
static my_GParamSpecTypeInfo_t* findFreeGParamSpecTypeInfo(my_GParamSpecTypeInfo_t* fcts)
{
    if(!fcts) return fcts;
    #define GO(A) if(ref_GParamSpecTypeInfo_##A == fcts) return &my_GParamSpecTypeInfo_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_GParamSpecTypeInfo_##A == 0) {   \
        ref_GParamSpecTypeInfo_##A = fcts;                 \
        my_GParamSpecTypeInfo_##A.instance_init = (fcts->instance_init)?((GetNativeFnc((uintptr_t)fcts->instance_init))?(void (*)(void *))GetNativeFnc((uintptr_t)fcts->instance_init):my_funcs_instance_init_##A):NULL;    \
        fct_funcs_instance_init_##A = (uintptr_t)fcts->instance_init;                            \
        my_GParamSpecTypeInfo_##A.finalize = (fcts->finalize)?((GetNativeFnc((uintptr_t)fcts->finalize))?(void (*)(void *))GetNativeFnc((uintptr_t)fcts->finalize):my_funcs_finalize_##A):NULL;    \
        fct_funcs_finalize_##A = (uintptr_t)fcts->finalize;                            \
        my_GParamSpecTypeInfo_##A.value_set_default = (fcts->value_set_default)?((GetNativeFnc((uintptr_t)fcts->value_set_default))?(void (*)(void *, void *))GetNativeFnc((uintptr_t)fcts->value_set_default):my_funcs_value_set_default_##A):NULL;    \
        fct_funcs_value_set_default_##A = (uintptr_t)fcts->value_set_default;                            \
        my_GParamSpecTypeInfo_##A.value_validate = (fcts->value_validate)?((GetNativeFnc((uintptr_t)fcts->value_validate))?(int (*)(void *, void *))GetNativeFnc((uintptr_t)fcts->value_validate):my_funcs_value_validate_##A):NULL;    \
        fct_funcs_value_validate_##A = (uintptr_t)fcts->value_validate;                            \
        my_GParamSpecTypeInfo_##A.values_cmp = (fcts->values_cmp)?((GetNativeFnc((uintptr_t)fcts->values_cmp))?(int (*)(void *, void *, void *))GetNativeFnc((uintptr_t)fcts->values_cmp):my_funcs_values_cmp_##A):NULL;    \
        fct_funcs_values_cmp_##A = (uintptr_t)fcts->values_cmp;                            \
        return &my_GParamSpecTypeInfo_##A;                \
    }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject2 GParamSpecTypeInfo callback\n");
    return NULL;
}

// GInterfaceInitFunc
#define GO(A)   \
static uintptr_t my_GInterfaceInitFunc_fct_##A = 0;                             \
static size_t my_GInterfaceInitFunc_klass_##A = 0;                              \
static void my_GInterfaceInitFunc_##A(void* src, void* dst)                     \
{                                                                               \
    RunFunctionFmt(my_GInterfaceInitFunc_fct_##A, "pp", src, dst);              \
    unwrapGTKInterface(src, my_GInterfaceInitFunc_klass_##A);                   \
}
SUPER()
#undef GO
static void* findGInterfaceInitFuncFct(void* fct, size_t klass)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GInterfaceInitFunc_fct_##A == (uintptr_t)fct && my_GInterfaceInitFunc_klass_##A == klass) return my_GInterfaceInitFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GInterfaceInitFunc_fct_##A == 0) {my_GInterfaceInitFunc_fct_##A = (uintptr_t)fct; my_GInterfaceInitFunc_klass_##A = klass; return my_GInterfaceInitFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject GInterfaceInitFunc callback\n");
    return NULL;
}
// GInterfaceFinalizeFunc
#define GO(A)   \
static uintptr_t my_GInterfaceFinalizeFunc_fct_##A = 0;                             \
static void my_GInterfaceFinalizeFunc_##A(void* src, void* dst)                     \
{                                                                                   \
    RunFunctionFmt(my_GInterfaceFinalizeFunc_fct_##A, "pp", src, dst);  \
}
SUPER()
#undef GO
static void* findGInterfaceFinalizeFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GInterfaceFinalizeFunc_fct_##A == (uintptr_t)fct) return my_GInterfaceFinalizeFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GInterfaceFinalizeFunc_fct_##A == 0) {my_GInterfaceFinalizeFunc_fct_##A = (uintptr_t)fct; return my_GInterfaceFinalizeFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject GInterfaceFinalizeFunc callback\n");
    return NULL;
}
// compare
#define GO(A)   \
static uintptr_t my_compare_fct_##A = 0;                                        \
static int my_compare_##A(void* a, void* b, void* data)                         \
{                                                                               \
    return RunFunctionFmt(my_compare_fct_##A, "ppp", a, b, data);   \
}
SUPER()
#undef GO
static void* findcompareFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_compare_fct_##A == (uintptr_t)fct) return my_compare_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_compare_fct_##A == 0) {my_compare_fct_##A = (uintptr_t)fct; return my_compare_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject compare callback\n");
    return NULL;
}

// GTypeModuleClass
#define GO(A)   \
static uintptr_t my_load_fct_##A = 0;                         \
static int my_load_##A(my_GTypeModule_t* module)              \
{                                                             \
    return (int)RunFunctionFmt(my_load_fct_##A, "p", module); \
}
SUPER()
#undef GO
static void* findLoadFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_load_fct_##A == (uintptr_t)fct) return my_load_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_load_fct_##A == 0) {my_load_fct_##A = (uintptr_t)fct; return my_load_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject GTypeModuleClass load callback\n");
    return NULL;
}

#define GO(A)   \
static uintptr_t my_unload_fct_##A = 0;                         \
static int my_unload_##A(my_GTypeModule_t* module)              \
{                                                             \
    return (int)RunFunctionFmt(my_unload_fct_##A, "p", module); \
}
SUPER()
#undef GO
static void* findUnloadFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_unload_fct_##A == (uintptr_t)fct) return my_unload_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_unload_fct_##A == 0) {my_unload_fct_##A = (uintptr_t)fct; return my_unload_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject GTypeModuleClass unload callback\n");
    return NULL;
}

#undef SUPER

#include "super100.h"

// GCallback  (generic function with 6 arguments, hopefully it's enough)
#define GO(A)   \
static uintptr_t my_GCallback_fct_##A = 0;                                                      \
static void* my_GCallback_##A(void* a, void* b, void* c, void* d, void* e, void* f)             \
{                                                                                               \
    return (void*)RunFunctionFmt(my_GCallback_fct_##A, "pppppp", a, b, c, d, e, f); \
}
SUPER()
#undef GO
static void* findGCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GCallback_fct_##A == (uintptr_t)fct) return my_GCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GCallback_fct_##A == 0) {my_GCallback_fct_##A = (uintptr_t)fct; return my_GCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject generic GCallback\n");
    return NULL;
}
// EmissionHook
#define GO(A)   \
static uintptr_t my_EmissionHook_fct_##A = 0;                           \
static void my_EmissionHook_##A(void* a, uint32_t b, void* c, void* d)  \
{                                                                       \
    RunFunctionFmt(my_EmissionHook_fct_##A, "pupp", a, b, c, d);        \
}
SUPER()
#undef GO
static void* findEmissionHookFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_EmissionHook_fct_##A == (uintptr_t)fct) return my_EmissionHook_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_EmissionHook_fct_##A == 0) {my_EmissionHook_fct_##A = (uintptr_t)fct; return my_EmissionHook_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject2 generic EmissionHook callback\n");
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
    printf_log(LOG_NONE, "Warning, no more slot for gobject2 generic Event callback\n");
    return NULL;
}
#undef SUPER

EXPORT uintptr_t my_g_signal_connect_data(x64emu_t* emu, void* instance, void* detailed, void* c_handler, void* data, void* closure, uint32_t flags)
{
    return my->g_signal_connect_data(instance, detailed, findEventFct(c_handler), data, findGClosureNotify_Fct(closure), flags);
}


EXPORT void* my_g_object_connect(x64emu_t* emu, void* object, void* signal_spec, void** b)
{
        //gobject2_my_t *my = (gobject2_my_t*)my_lib->w.p2;

    char* spec = (char*)signal_spec;
    while(spec) {
        // loop on each triplet...
        if(strstr(spec, "signal::")==spec) {
            my_g_signal_connect_data(emu, object, spec+strlen("signal::"), b[0], b[1], NULL, 0);
            b+=2;
            spec = (char*)*(b++);
        } else if(strstr(spec, "swapped_signal::")==spec || strstr(spec, "swapped-signal::")==spec) {
            my_g_signal_connect_data(emu, object, spec+strlen("swapped_signal::"), b[0], b[1], NULL, 2);
            b+=2;
            spec = (char*)*(b++);
        } else if(strstr(spec, "signal_after::")==spec || strstr(spec, "signal-after::")==spec) {
            my_g_signal_connect_data(emu, object, spec+strlen("signal_after::"), b[0], b[1], NULL, 1);
            b+=2;
            spec = (char*)*(b++);
        } else if(strstr(spec, "swapped_signal_after::")==spec || strstr(spec, "swapped-signal-after::")==spec) {
            my_g_signal_connect_data(emu, object, spec+strlen("swapped_signal_after::"), b[0], b[1], NULL, 1|2);
            b+=2;
            spec = (char*)*(b++);
        } else {
            printf_log(LOG_NONE, "Warning, don't know how to handle signal spec \"%s\" in g_object_connect\n", spec);
            spec = NULL;
        }
    }
    return object;
}

EXPORT uintptr_t my_g_signal_connect_object(x64emu_t* emu, void* instance, void* detailed, void* c_handler, void* object, uint32_t flags)
{
    //TODO: get the type of instance to be more precise below

    return my->g_signal_connect_object(instance, detailed, findGCallbackFct(c_handler), object, flags);
}

EXPORT int my_g_boxed_type_register_static(x64emu_t* emu, void* name, void* boxed_copy, void* boxed_free)
{
    void* bc = findCopyFct(boxed_copy);
    void* bf = findFreeFct(boxed_free);
    return my->g_boxed_type_register_static(name, bc, bf);
}

EXPORT uint32_t my_g_signal_new(x64emu_t* emu, void* name, size_t itype, uint32_t flags, uint32_t offset, void* acc, void* accu_data, void* marsh, size_t rtype, uint32_t n, void** b)
{
    printf_log(LOG_DEBUG, "g_signal_new for \"%s\", with offset=%d and %d args\n", (const char*)name, offset, n);

    void* cb_acc = findAccumulatorFct(acc);
    void* cb_marsh = findMarshalFct(marsh);
    my_add_signal_offset(itype, offset, n); // register the signal for later use
    switch(n) {
        case 0: return my->g_signal_new(name, itype, flags, offset, cb_acc, accu_data, cb_marsh, rtype, n);
        case 1: return my->g_signal_new(name, itype, flags, offset, cb_acc, accu_data, cb_marsh, rtype, n, b[0]);
        case 2: return my->g_signal_new(name, itype, flags, offset, cb_acc, accu_data, cb_marsh, rtype, n, b[0], b[1]);
        case 3: return my->g_signal_new(name, itype, flags, offset, cb_acc, accu_data, cb_marsh, rtype, n, b[0], b[1], b[2]);
        case 4: return my->g_signal_new(name, itype, flags, offset, cb_acc, accu_data, cb_marsh, rtype, n, b[0], b[1], b[2], b[3]);
        case 5: return my->g_signal_new(name, itype, flags, offset, cb_acc, accu_data, cb_marsh, rtype, n, b[0], b[1], b[2], b[3], b[4]);
        case 6: return my->g_signal_new(name, itype, flags, offset, cb_acc, accu_data, cb_marsh, rtype, n, b[0], b[1], b[2], b[3], b[4], b[5]);
        case 15:return my->g_signal_new(name, itype, flags, offset, cb_acc, accu_data, cb_marsh, rtype, n, b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14]);
    }
    printf_log(LOG_NONE, "Warning, gobject g_signal_new called with too many parameters (%d)\n", n);
    return my->g_signal_new(name, itype, flags, offset, cb_acc, accu_data, cb_marsh, rtype, n, b[0], b[1], b[2]);
}

EXPORT uint32_t my_g_signal_newv(x64emu_t* emu, void* name, size_t itype, uint32_t flags, void* closure, void* acc, void* accu_data, void* marsh, size_t rtype, uint32_t n, void* types)
{
    printf_log(LOG_DEBUG, "g_signal_newv for \"%s\", with %d args\n", (const char*)name, n);

    return my->g_signal_newv(name, itype, flags, closure, findAccumulatorFct(acc), accu_data, findMarshalFct(marsh), rtype, n, types);
}

EXPORT uint32_t my_g_signal_new_valist(x64emu_t* emu, void* name, size_t itype, uint32_t flags, void* closure, void* acc, void* accu_data, void* marsh, size_t rtype, uint32_t n, x64_va_list_t b)
{
    printf_log(LOG_DEBUG, "g_signal_new_valist for \"%s\", with %d args\n", (const char*)name, n);
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    CREATE_VALIST_FROM_VALIST(b, emu->scratch);
    #endif
    return my->g_signal_new_valist(name, itype, flags, closure, findAccumulatorFct(acc), accu_data, findMarshalFct(marsh), rtype, n, VARARGS);
}

EXPORT uint32_t my_g_signal_handlers_block_matched(x64emu_t* emu, void* instance, uint32_t mask, uint32_t signal, uint32_t detail, void* closure, void* fnc, void* data)
{
    // NOTE that I have no idea of the fnc signature!...
    if (fnc) printf_log(LOG_DEBUG, "Warning, gobject g_signal_handlers_block_matched called with non null function \n");
    fnc = findMarshalFct(fnc);  //... just in case
    return my->g_signal_handlers_block_matched(instance, mask, signal, detail, closure, fnc, data);
}

EXPORT uint32_t my_g_signal_handlers_unblock_matched(x64emu_t* emu, void* instance, uint32_t mask, uint32_t signal, uint32_t detail, void* closure, void* fnc, void* data)
{
    // NOTE that I have no idea of the fnc signature!...
    if (fnc) printf_log(LOG_DEBUG, "Warning, gobject g_signal_handlers_unblock_matched called with non null function \n");
    fnc = findMarshalFct(fnc);  //... just in case
    return my->g_signal_handlers_unblock_matched(instance, mask, signal, detail, closure, fnc, data);
}

EXPORT uint32_t my_g_signal_handlers_disconnect_matched(x64emu_t* emu, void* instance, uint32_t mask, uint32_t signal, uint32_t detail, void* closure, void* fnc, void* data)
{
    // NOTE that I have no idea of the fnc signature!...
    if (fnc) printf_log(LOG_DEBUG, "Warning, gobject g_signal_handlers_disconnect_matched called with non null function \n");
    fnc = findMarshalFct(fnc);  //... just in case
    return my->g_signal_handlers_disconnect_matched(instance, mask, signal, detail, closure, fnc, data);
}

EXPORT unsigned long my_g_signal_handler_find(x64emu_t* emu, void* instance, uint32_t mask, uint32_t signal, uint32_t detail, void* closure, void* fnc, void* data)
{
    // NOTE that I have no idea of the fnc signature!...
    if (fnc) printf_log(LOG_DEBUG, "Warning, gobject g_signal_handler_find called with non null function \n");
    fnc = findMarshalFct(fnc);  //... just in case
    return my->g_signal_handler_find(instance, mask, signal, detail, closure, fnc, data);
}

EXPORT void* my_g_object_new(x64emu_t* emu, size_t type, void* first, void* b)
{

    if(first) {
        CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
        return my->g_object_new_valist(type, first, VARARGS);
    }
    return my->g_object_new(type, first);
}

EXPORT void* my_g_object_new_valist(x64emu_t* emu, size_t type, void* first, x64_va_list_t b)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    CREATE_VALIST_FROM_VALIST(b, emu->scratch);
    #endif
    return my->g_object_new_valist(type, first, VARARGS);
}

EXPORT size_t my_g_type_register_static(x64emu_t* emu, size_t parent, void* name, my_GTypeInfo_t* info, int flags)
{
    size_t ret = my->g_type_register_static(parent, name, findFreeGTypeInfo(info, parent), flags);
    printf_log(LOG_DEBUG, "Registered %s as 0x%zx\n", name, ret);
    addRegisteredClass(ret, name);
    return ret;
}

EXPORT size_t my_g_type_register_fundamental(x64emu_t* emu, size_t parent, void* name, my_GTypeInfo_t* info, void* finfo, int flags)
{

    return my->g_type_register_fundamental(parent, name, findFreeGTypeInfo(info, parent), finfo, flags);
}

EXPORT void my_g_value_register_transform_func(x64emu_t* emu, size_t src, size_t dst, void* f)
{

    my->g_value_register_transform_func(src, dst, findValueTransformFct(f));
}

EXPORT unsigned long my_g_signal_add_emission_hook(x64emu_t* emu, uint32_t signal, uint32_t detail, void* f, void* data, void* notify)
{
    return my->g_signal_add_emission_hook(signal, detail, findEmissionHookFct(f), data, findDestroyFct(notify));
}

EXPORT size_t my_g_type_register_static_simple(x64emu_t* emu, size_t parent, void* name, uint32_t class_size, void* class_init, uint32_t instance_size, void* instance_init, uint32_t flags)
{
        //gobject2_my_t *my = (gobject2_my_t*)my_lib->w.p2;

    my_GTypeInfo_t info = {0};
    info.class_size = class_size;
    info.class_init = class_init;
    info.instance_size = instance_size;
    info.instance_init = instance_init;

    return my_g_type_register_static(emu, parent, name, &info, flags);
}

typedef struct my_GInterfaceInfo_s {
    void* interface_init;
    void* interface_finalize;
    void* data;
} my_GInterfaceInfo_t;

EXPORT void my_g_type_add_interface_static(x64emu_t* emu, size_t instance_type, size_t interface_type, my_GInterfaceInfo_t* info)
{
    my_GInterfaceInfo_t i = {0};
    i.interface_init = findGInterfaceInitFuncFct(info->interface_init, interface_type);
    i.interface_finalize = findGInterfaceFinalizeFuncFct(info->interface_finalize);
    i.data = info->data;
    my->g_type_add_interface_static(instance_type, interface_type, &i);
}

EXPORT void my_g_param_spec_set_qdata_full(x64emu_t* emu, void* pspec, uint32_t quark, void* data, void* notify)
{

    my->g_param_spec_set_qdata_full(pspec, quark, data, findFreeFct(notify));
}

EXPORT size_t my_g_param_type_register_static(x64emu_t* emu, void* name, void* pspec_info)
{

    return my->g_param_type_register_static(name, findFreeGParamSpecTypeInfo(pspec_info));
}

EXPORT void* my_g_value_array_sort(x64emu_t* emu, void* array, void* comp)
{

    return my->g_value_array_sort(array, findcompareFct(comp));
}

EXPORT void* my_g_value_array_sort_with_data(x64emu_t* emu, void* array, void* comp, void* data)
{

    return my->g_value_array_sort_with_data(array, findcompareFct(comp), data);
}

EXPORT void my_g_object_set_data_full(x64emu_t* emu, void* object, void* key, void* data, void* notify)
{

    my->g_object_set_data_full(object, key, data, findFreeFct(notify));
}

EXPORT void* my_g_type_class_peek_parent(x64emu_t* emu, void* object)
{

    void* klass = my->g_type_class_peek_parent(object);
    size_t type = klass?*(size_t*)klass:0;
    return wrapCopyGTKClass(klass, type);
}

EXPORT void my_g_signal_emit_valist(x64emu_t* emu, void* inst, uint32_t id, uint32_t quark, x64_va_list_t b)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    CREATE_VALIST_FROM_VALIST(b, emu->scratch);
    #endif
    my->g_signal_emit_valist(inst, id, quark, VARARGS);
}

EXPORT void my_g_signal_emit(x64emu_t* emu, void* inst, uint32_t id, uint32_t quark, uintptr_t* b)
{
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 3);
    my->g_signal_emit_valist(inst, id, quark, VARARGS);
}

EXPORT void my_g_object_get_valist(x64emu_t* emu, void* a1, void* a2, x64_va_list_t b)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    CREATE_VALIST_FROM_VALIST(b, emu->scratch);
    #endif
    my->g_object_get_valist(a1, a2, VARARGS);
}

EXPORT void my_g_object_get(x64emu_t* emu, void* a1, void* a2, uintptr_t* b)
{
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    my->g_object_get_valist(a1, a2, VARARGS);
}


EXPORT void my_g_object_set_valist(x64emu_t* emu, void* a1, void* a2, x64_va_list_t b)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    CREATE_VALIST_FROM_VALIST(b, emu->scratch);
    #endif
    my->g_object_set_valist(a1, a2, VARARGS);
}

EXPORT void my_g_object_set(x64emu_t* emu, void* a1, void* a2, uintptr_t* b)
{
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    my->g_object_set_valist(a1, a2, VARARGS);
}

EXPORT void my_g_object_set_qdata_full(x64emu_t* emu, void* o, uint32_t q, void* data, void* d)
{
    my->g_object_set_qdata_full(o, q, data, findDestroyFct(d));
}

EXPORT int my_g_object_replace_qdata(x64emu_t* emu, void* o, uint32_t q, void* ov, void* nv, void* d, void* od)
{
    return my->g_object_replace_qdata(o, q, ov, nv, findDestroyFct(d), findDestroyFct(od));
}

EXPORT void my_g_object_class_install_properties(x64emu_t* emu, void* klass, uint32_t n, void* specs)
{
    unwrapGTKClass(klass, my->g_object_get_type());
    my->g_object_class_install_properties(klass, n, specs);
    wrapGTKClass(klass, my->g_object_get_type());
}

EXPORT void my_g_object_weak_ref(x64emu_t* emu, void* object, void* notify, void* data)
{
    my->g_object_weak_ref(object, findWeakNotifyFct(notify), data);
}

EXPORT void my_g_object_weak_unref(x64emu_t* emu, void* object, void* notify, void* data)
{
    my->g_object_weak_unref(object, findWeakNotifyFct(notify), data);
}

EXPORT void my_g_object_add_toggle_ref(x64emu_t* emu, void* object, void* notify, void* data)
{
    my->g_object_add_toggle_ref(object, findToggleNotifyFct(notify), data);
}

EXPORT void my_g_object_remove_toggle_ref(x64emu_t* emu, void* object, void* notify, void* data)
{
    my->g_object_remove_toggle_ref(object, findToggleNotifyFct(notify), data);
}

EXPORT void my_g_signal_override_class_handler(x64emu_t* emu, char* name, size_t gtype, void* callback)
{
    my->g_signal_override_class_handler(name, gtype, findGCallbackFct(callback));
}

EXPORT void my_g_closure_set_marshal(x64emu_t* emu, void* closure, void* marshal)
{
    my->g_closure_set_marshal(closure, findMarshalFct(marshal));
}

EXPORT void* my_g_cclosure_new(x64emu_t* emu, void* callback, void* data, void* destroy)
{
    return my->g_cclosure_new(findGCallbackFct(callback), data, findGClosureNotify_Fct(destroy));
}

EXPORT void* my_g_cclosure_new_swap(x64emu_t* emu, void* callback, void* data, void* destroy)
{
    return my->g_cclosure_new_swap(findGCallbackFct(callback), data, findGClosureNotify_Fct(destroy));
}

EXPORT void my_g_closure_add_finalize_notifier(x64emu_t* emu, void* closure, void* data, void* f)
{
    my->g_closure_add_finalize_notifier(closure, data, findGClosureNotify_Fct(f));
}

EXPORT void my_g_closure_remove_finalize_notifier(x64emu_t* emu, void* closure, void* data, void* f)
{
    my->g_closure_remove_finalize_notifier(closure, data, findGClosureNotify_Fct(f));
}

EXPORT void* my_g_type_value_table_peek(x64emu_t* emu, size_t type)
{
    return findFreeGTypeValueTable(my->g_type_value_table_peek(type));
}

EXPORT int my_g_type_module_use(x64emu_t* emu, my_GTypeModule_t* module)
{
    my_GTypeModuleClass_t* module_class = (my_GTypeModuleClass_t*)(((my_GTypeInstance_t*)module)->g_class);
    if (module_class) {
        module_class->load = findLoadFct(module_class->load);
        module_class->unload = findUnloadFct(module_class->unload);
    }
    return my->g_type_module_use(module);
}

EXPORT void my_g_type_module_add_interface(x64emu_t* emu, my_GTypeModule_t* module, size_t instance_type, size_t interface_type, my_GInterfaceInfo_t* interface_info)
{
    if (interface_info) {
        interface_info->interface_init = findGInterfaceInitFuncFct(interface_info->interface_init, interface_type);
        interface_info->interface_finalize = findGInterfaceFinalizeFuncFct(interface_info->interface_finalize);
    }
    return my->g_type_module_add_interface(module, instance_type, interface_type, interface_info);
}

EXPORT size_t my_g_type_module_register_type(x64emu_t* emu, my_GTypeModule_t* module, size_t parent_type, char* type_name, my_GTypeInfo_t* type_info, uint32_t flags)
{
    if (type_info) {
        type_info->class_init = find_class_init_Fct(type_info->class_init, parent_type);
    }
    return my->g_type_module_register_type(module, parent_type, type_name, type_info, flags);
}

#define PRE_INIT    \
    if(BOX64ENV(nogtk)) \
        return -1;

#define CUSTOM_INIT \
    SetGObjectID(my->g_object_get_type());  \
    SetGInitiallyUnownedID(my->g_initially_unowned_get_type()); \
    SetGTypeName(my->g_type_name);          \
    SetGClassPeek(my->g_type_class_peek);   \
    SetGTypeParent(my->g_type_parent);

#define NEEDED_LIBS "libglib-2.0.so.0"

#include "wrappedlib_init.h"
