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
#define LIBNAME gobject2
library_t *my_lib = NULL;

typedef size_t(*LFv_t)(void);
typedef void*(*pFL_t)(size_t);

#define ADDED_FUNCTIONS()           \
 GO(g_object_get_type, LFv_t)       \
 GO(g_type_name, pFL_t)

#include "generated/wrappedgobject2types.h"

typedef struct gobject2_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} gobject2_my_t;

static void addGObject2Alternate(library_t* lib);

void* getGobject2My(library_t* lib)
{
    gobject2_my_t* my = (gobject2_my_t*)calloc(1, sizeof(gobject2_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    addGObject2Alternate(lib);
    return my;
}
#undef SUPER

void freeGobject2My(void* lib)
{
    //gobject2_my_t *my = (gobject2_my_t *)lib;
}

static int signal_cb(void* a, void* b, void* c, void* d)
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
    if(!sig && my_signal_is_valid(c))  {
        sig = (my_signal_t*)c;
        i = 3;
    }
    if(!sig && my_signal_is_valid(d)) {
        sig = (my_signal_t*)d;
        i = 4;
    }
    printf_log(LOG_DEBUG, "gobject2 Signal called, sig=%p, handler=%p, NArgs=%d\n", sig, (void*)sig->c_handler, i);
    switch(i) {
        case 1: return (int)RunFunction(my_context, sig->c_handler, 1, sig->data);
        case 2: return (int)RunFunction(my_context, sig->c_handler, 2, a, sig->data);
        case 3: return (int)RunFunction(my_context, sig->c_handler, 3, a, b, sig->data);
        case 4: return (int)RunFunction(my_context, sig->c_handler, 4, a, b, c, sig->data);
    }
    printf_log(LOG_NONE, "Warning, GObject2 signal callback but no data found!");
    return 0;
}
static int signal_cb_swapped(my_signal_t* sig, void* b, void* c, void* d)
{
    // data is in front here...
    printf_log(LOG_DEBUG, "gobject2 swaped4 Signal called, sig=%p\n", sig);
    return (int)RunFunction(my_context, sig->c_handler, 4, sig->data, b, c, d);
}
static int signal_cb_5(void* a, void* b, void* c, void* d, my_signal_t* sig)
{
    printf_log(LOG_DEBUG, "gobject2 5 Signal called, sig=%p\n", sig);
    return (int)RunFunction(my_context, sig->c_handler, 5, a, b, c, d, sig->data);
}
static int signal_cb_swapped_5(my_signal_t* sig, void* b, void* c, void* d, void* e)
{
    // data is in front here...
    printf_log(LOG_DEBUG, "gobject2 swaped5 Signal called, sig=%p\n", sig);
    return (int)RunFunction(my_context, sig->c_handler, 5, sig->data, b, c, d, e);
}
static int signal_cb_6(void* a, void* b, void* c, void* d, void* e, my_signal_t* sig)
{
    printf_log(LOG_DEBUG, "gobject2 6 Signal called, sig=%p\n", sig);
    return (int)RunFunction(my_context, sig->c_handler, 6, a, b, c, d, e, sig->data);
}
static int signal_cb_swapped_6(my_signal_t* sig, void* b, void* c, void* d, void* e, void* f)
{
    // data is in front here...
    printf_log(LOG_DEBUG, "gobject2 swaped6 Signal called, sig=%p\n", sig);
    return (int)RunFunction(my_context, sig->c_handler, 6, sig->data, b, c, d, e, f);
}
static int signal_cb_8(void* a, void* b, void* c, void* d, void* e, void* f, void* g, my_signal_t* sig)
{
    printf_log(LOG_DEBUG, "gobject2 8 Signal called, sig=%p\n", sig);
    return (int)RunFunction(my_context, sig->c_handler, 8, a, b, c, d, e, f, g, sig->data);
}
static int signal_cb_swapped_8(my_signal_t* sig, void* b, void* c, void* d, void* e, void* f, void* g, void* h)
{
    // data is in front here...
    printf_log(LOG_DEBUG, "gobject2 swaped8 Signal called, sig=%p\n", sig);
    return (int)RunFunction(my_context, sig->c_handler, 8, sig->data, b, c, d, e, f, g, h);
}

static void signal_delete(my_signal_t* sig, void* b)
{
    uintptr_t d = sig->destroy;
    if(d) {
        RunFunction(my_context, d, 2, sig->data, b);
    }
    printf_log(LOG_DEBUG, "gobject2 Signal deleted, sig=%p, destroy=%p\n", sig, (void*)d);
    free(sig);
}

static void addGObject2Alternate(library_t* lib)
{
    #define GO(A, W) AddAutomaticBridge(thread_get_emu(), lib->priv.w.bridge, W, dlsym(lib->priv.w.lib, #A), 0)
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
    #define GO(A, W) AddAutomaticBridge(thread_get_emu(), lib->priv.w.bridge, W, A, 0)
    GO(signal_cb, iFpppp);
    GO(signal_cb_swapped, iFpppp);
    GO(signal_cb_5, iFppppp);
    GO(signal_cb_swapped_5, iFppppp);
    GO(signal_cb_6, iFpppppp);
    GO(signal_cb_swapped_6, iFpppppp);
    GO(signal_cb_8, iFpppppppp);
    GO(signal_cb_swapped_8, iFpppppppp);
    GO(signal_delete, vFpp);
    #undef GO
}

EXPORT uintptr_t my_g_signal_connect_data(x64emu_t* emu, void* instance, void* detailed, void* c_handler, void* data, void* closure, uint32_t flags)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

    //TODO: get the type of instance to be more precise below

    my_signal_t *sig = new_mysignal(c_handler, data, closure);
    uintptr_t ret = 0;
    #define GO(A, B) if(strcmp((const char*)detailed, A)==0) ret = my->g_signal_connect_data(instance, detailed, (flags&2)?((void*)signal_cb_swapped_##B):((void*)signal_cb_##B), sig, signal_delete, flags);
    GO("query-tooltip", 6)  // GtkWidget
    else GO("selection-get", 5) //GtkWidget
    else GO("drag-data-get", 5) //GtkWidget
    else GO("drag-data-received", 8)    //GtkWidget
    else GO("drag-drop", 6) //GtkWidget
    else GO("drag-motion", 6)   //GtkWidget
    else GO("expand-collapse-cursor-row", 5)    //GtkTreeView
    else
        ret = my->g_signal_connect_data(instance, detailed, (flags&2)?((void*)signal_cb_swapped):((void*)signal_cb), sig, signal_delete, flags);
    #undef GO
    printf_log(LOG_DEBUG, "Connecting gobject2 %p signal \"%s\" with sig=%p to %p, flags=%d\n", instance, (char*)detailed, sig, c_handler, flags);
    return ret;
}


EXPORT void* my_g_object_connect(x64emu_t* emu, void* object, void* signal_spec, void** b)
{
        //gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

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


#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)   \
GO(5)   \
GO(6)   \
GO(7)

#define GO(A)   \
static uintptr_t my_copy_fct_##A = 0;   \
static void* my_copy_##A(void* data)     \
{                                       \
    return (void*)RunFunction(my_context, my_copy_fct_##A, 1, data);\
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
static uintptr_t my_free_fct_##A = 0;   \
static void my_free_##A(void* data)     \
{                                       \
    RunFunction(my_context, my_free_fct_##A, 1, data);\
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
static uintptr_t my_accumulator_fct_##A = 0;   \
static int my_accumulator_##A(void* ihint, void* return_accu, void* handler_return, void* data)     \
{                                       \
    return RunFunction(my_context, my_accumulator_fct_##A, 4, ihint, return_accu, handler_return, data);\
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
static uintptr_t my_marshal_fct_##A = 0;   \
static void my_marshal_##A(void* closure, void* return_value, uint32_t n, void* values, void* hint, void* data) \
{                                                                                                               \
    RunFunction(my_context, my_marshal_fct_##A, 6, closure, return_value, n, values, hint, data);               \
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
    #define GO(A) if(my_marshal_fct_##A == 0) {AddAutomaticBridge(thread_get_emu(), my_lib->priv.w.bridge, vFppuppp, my_marshal_##A, 0); my_marshal_fct_##A = (uintptr_t)fct; return my_marshal_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject Closure Marshal callback\n");
    return NULL;
}

// GValueTransform
#define GO(A)   \
static uintptr_t my_valuetransform_fct_##A = 0;                     \
static void my_valuetransform_##A(void* src, void* dst)            \
{                                                                   \
    RunFunction(my_context, my_valuetransform_fct_##A, 2, src, dst);\
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
// GCallback  (generic function with 6 arguments, hopefully it's enough)
#define GO(A)   \
static uintptr_t my_GCallback_fct_##A = 0;                                             \
static void* my_GCallback_##A(void* a, void* b, void* c, void* d, void* e, void* f)    \
{                                                                                           \
    return (void*)RunFunction(my_context, my_GCallback_fct_##A, 6, a, b, c, d, e, f);  \
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
    printf_log(LOG_NONE, "Warning, no more slot for gobject Value Transform callback\n");
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

#define GO(A) \
static my_GParamSpecTypeInfo_t     my_GParamSpecTypeInfo_##A = {0};   \
static my_GParamSpecTypeInfo_t   *ref_GParamSpecTypeInfo_##A = NULL;
SUPER()
#undef GO
// then the static functions callback that may be used with the structure, but dispatch also have a callback...
#define GO(A)   \
static uintptr_t fct_funcs_instance_init_##A = 0;                   \
static void my_funcs_instance_init_##A(void* pspec) {               \
    RunFunction(my_context, fct_funcs_instance_init_##A, 1, pspec); \
}   \
static uintptr_t fct_funcs_finalize_##A = 0;                        \
static void my_funcs_finalize_##A(void* pspec) {                    \
    RunFunction(my_context, fct_funcs_finalize_##A, 1, pspec);      \
}   \
static uintptr_t fct_funcs_value_set_default_##A = 0;               \
static void my_funcs_value_set_default_##A(void* pspec, void* value) {          \
    RunFunction(my_context, fct_funcs_value_set_default_##A, 2, pspec, value);  \
}   \
static uintptr_t fct_funcs_value_validate_##A = 0;                  \
static int my_funcs_value_validate_##A(void* pspec, void* value) {  \
    return (int)RunFunction(my_context, fct_funcs_value_validate_##A, 2, pspec, value); \
}   \
static uintptr_t fct_funcs_values_cmp_##A = 0;                      \
static int my_funcs_values_cmp_##A(void* pspec, void* value1, void* value2) {   \
    return (int)RunFunction(my_context, fct_funcs_values_cmp_##A, 3, pspec, value1, value2);    \
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
        my_GParamSpecTypeInfo_##A.instance_init = (fcts->instance_init)?((GetNativeFnc((uintptr_t)fcts->instance_init))?GetNativeFnc((uintptr_t)fcts->instance_init):my_funcs_instance_init_##A):NULL;    \
        fct_funcs_instance_init_##A = (uintptr_t)fcts->instance_init;                            \
        my_GParamSpecTypeInfo_##A.finalize = (fcts->finalize)?((GetNativeFnc((uintptr_t)fcts->finalize))?GetNativeFnc((uintptr_t)fcts->finalize):my_funcs_finalize_##A):NULL;    \
        fct_funcs_finalize_##A = (uintptr_t)fcts->finalize;                            \
        my_GParamSpecTypeInfo_##A.value_set_default = (fcts->value_set_default)?((GetNativeFnc((uintptr_t)fcts->value_set_default))?GetNativeFnc((uintptr_t)fcts->value_set_default):my_funcs_value_set_default_##A):NULL;    \
        fct_funcs_value_set_default_##A = (uintptr_t)fcts->value_set_default;                            \
        my_GParamSpecTypeInfo_##A.value_validate = (fcts->value_validate)?((GetNativeFnc((uintptr_t)fcts->value_validate))?GetNativeFnc((uintptr_t)fcts->value_validate):my_funcs_value_validate_##A):NULL;    \
        fct_funcs_value_validate_##A = (uintptr_t)fcts->value_validate;                            \
        my_GParamSpecTypeInfo_##A.values_cmp = (fcts->values_cmp)?((GetNativeFnc((uintptr_t)fcts->values_cmp))?GetNativeFnc((uintptr_t)fcts->values_cmp):my_funcs_values_cmp_##A):NULL;    \
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
static uintptr_t my_GInterfaceInitFunc_fct_##A = 0;                     \
static void my_GInterfaceInitFunc_##A(void* src, void* dst)            \
{                                                                   \
    RunFunction(my_context, my_GInterfaceInitFunc_fct_##A, 2, src, dst);\
}
SUPER()
#undef GO
static void* findGInterfaceInitFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GInterfaceInitFunc_fct_##A == (uintptr_t)fct) return my_GInterfaceInitFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GInterfaceInitFunc_fct_##A == 0) {my_GInterfaceInitFunc_fct_##A = (uintptr_t)fct; return my_GInterfaceInitFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject GInterfaceInitFunc callback\n");
    return NULL;
}
// GInterfaceFinalizeFunc
#define GO(A)   \
static uintptr_t my_GInterfaceFinalizeFunc_fct_##A = 0;                     \
static void my_GInterfaceFinalizeFunc_##A(void* src, void* dst)            \
{                                                                   \
    RunFunction(my_context, my_GInterfaceFinalizeFunc_fct_##A, 2, src, dst);\
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
static uintptr_t my_compare_fct_##A = 0;                                \
static int my_compare_##A(void* a, void* b, void* data)                 \
{                                                                       \
    return RunFunction(my_context, my_compare_fct_##A, 3, a, b, data);  \
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
#undef SUPER

EXPORT uintptr_t my_g_signal_connect_object(x64emu_t* emu, void* instance, void* detailed, void* c_handler, void* object, uint32_t flags)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

    //TODO: get the type of instance to be more precise below

    return my->g_signal_connect_object(instance, detailed, findGCallbackFct(c_handler), object, flags);
}

EXPORT int my_g_boxed_type_register_static(x64emu_t* emu, void* name, void* boxed_copy, void* boxed_free)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;
    void* bc = findCopyFct(boxed_copy);
    void* bf = findFreeFct(boxed_free);
    return my->g_boxed_type_register_static(name, bc, bf);
}

EXPORT uint32_t my_g_signal_new(x64emu_t* emu, void* name, size_t itype, int flags, uint32_t offset, void* acc, void* accu_data, void* marsh, size_t rtype, uint32_t n, void** b)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

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

EXPORT uint32_t my_g_signal_newv(x64emu_t* emu, void* name, size_t itype, int flags, void* closure, void* acc, void* accu_data, void* marsh, size_t rtype, uint32_t n, void* types)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

    printf_log(LOG_DEBUG, "g_signal_newv for \"%s\", with %d args\n", (const char*)name, n);
    
    return my->g_signal_newv(name, itype, flags, closure, findAccumulatorFct(acc), accu_data, findMarshalFct(marsh), rtype, n, types);
}

EXPORT uint32_t my_g_signal_new_valist(x64emu_t* emu, void* name, size_t itype, int flags, void* closure, void* acc, void* accu_data, void* marsh, size_t rtype, uint32_t n, x64_va_list_t b)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

    printf_log(LOG_DEBUG, "g_signal_new_valist for \"%s\", with %d args\n", (const char*)name, n);
    CONVERT_VALIST(b);
    return my->g_signal_new_valist(name, itype, flags, closure, findAccumulatorFct(acc), accu_data, findMarshalFct(marsh), rtype, n, VARARGS);
}

EXPORT uint32_t my_g_signal_handlers_block_matched(x64emu_t* emu, void* instance, int mask, uint32_t signal, void* detail, void* closure, void* fnc, void* data)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

    // NOTE that I have no idea of the fnc signature!...
    if (fnc) printf_log(LOG_INFO, "Warning, gobject g_signal_handlers_block_matched called with non null function \n");
    fnc = findMarshalFct(fnc);  //... just in case
    return my->g_signal_handlers_block_matched(instance, mask, signal, detail, closure, fnc, data);
}

EXPORT uint32_t my_g_signal_handlers_unblock_matched(x64emu_t* emu, void* instance, int mask, uint32_t signal, void* detail, void* closure, void* fnc, void* data)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

    // NOTE that I have no idea of the fnc signature!...
    if (fnc) printf_log(LOG_INFO, "Warning, gobject g_signal_handlers_unblock_matched called with non null function \n");
    fnc = findMarshalFct(fnc);  //... just in case
    return my->g_signal_handlers_unblock_matched(instance, mask, signal, detail, closure, fnc, data);
}

EXPORT uint32_t my_g_signal_handlers_disconnect_matched(x64emu_t* emu, void* instance, int mask, uint32_t signal, void* detail, void* closure, void* fnc, void* data)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

    // NOTE that I have no idea of the fnc signature!...
    if (fnc) printf_log(LOG_INFO, "Warning, gobject g_signal_handlers_disconnect_matched called with non null function \n");
    fnc = findMarshalFct(fnc);  //... just in case
    return my->g_signal_handlers_disconnect_matched(instance, mask, signal, detail, closure, fnc, data);
}

EXPORT unsigned long my_g_signal_handler_find(x64emu_t* emu, void* instance, int mask, uint32_t signal, void* detail, void* closure, void* fnc, void* data)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

    // NOTE that I have no idea of the fnc signature!...
    if (fnc) printf_log(LOG_INFO, "Warning, gobject g_signal_handler_find called with non null function \n");
    fnc = findMarshalFct(fnc);  //... just in case
    return my->g_signal_handler_find(instance, mask, signal, detail, closure, fnc, data);
}

EXPORT void* my_g_object_new(x64emu_t* emu, size_t type, void* first, void* b)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

    if(first) {
        CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
        return my->g_object_new_valist(type, first, VARARGS);
    }
    return my->g_object_new(type, first);
}

EXPORT void* my_g_object_new_valist(x64emu_t* emu, size_t type, void* first, x64_va_list_t b)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;
    CONVERT_VALIST(b);
    return my->g_object_new_valist(type, first, VARARGS);
}

EXPORT size_t my_g_type_register_static(x64emu_t* emu, size_t parent, void* name, my_GTypeInfo_t* info, int flags)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

    return my->g_type_register_static(parent, name, findFreeGTypeInfo(info, parent), flags);
}

EXPORT size_t my_g_type_register_fundamental(x64emu_t* emu, size_t parent, void* name, my_GTypeInfo_t* info, void* finfo, int flags)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

    return my->g_type_register_fundamental(parent, name, findFreeGTypeInfo(info, parent), finfo, flags);
}

EXPORT void my_g_value_register_transform_func(x64emu_t* emu, size_t src, size_t dst, void* f)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

    my->g_value_register_transform_func(src, dst, findValueTransformFct(f));
}

static int my_signal_emission_hook(void* ihint, uint32_t n, void* values, my_signal_t* sig)
{
    printf_log(LOG_DEBUG, "gobject2 Signal Emission Hook called, sig=%p\n", sig);
    return (int)RunFunction(my_context, sig->c_handler, 4, ihint, n, values, sig->data);
}
EXPORT unsigned long my_g_signal_add_emission_hook(x64emu_t* emu, uint32_t signal, void* detail, void* f, void* data, void* notify)
{
    // there can be many signals connected, so something "light" is needed here
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

    if(!f)
        return my->g_signal_add_emission_hook(signal, detail, f, data, notify);
    my_signal_t* sig = new_mysignal(f, data, notify);
    printf_log(LOG_DEBUG, "gobject2 Signal Emission Hook for signal %d created for %p, sig=%p\n", signal, f, sig);
    return my->g_signal_add_emission_hook(signal, detail, my_signal_emission_hook, sig, my_signal_delete);
}

EXPORT size_t my_g_type_register_static_simple(x64emu_t* emu, size_t parent, void* name, size_t class_size, void* class_init, size_t instance_size, void* instance_init, int flags)
{
        //gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

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
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

    my_GInterfaceInfo_t i = {0};
    i.interface_init = findGInterfaceInitFuncFct(info->interface_init);
    i.interface_finalize = findGInterfaceFinalizeFuncFct(info->interface_finalize);
    i.data = info->data;
    my->g_type_add_interface_static(instance_type, interface_type, &i);
}

EXPORT void my_g_param_spec_set_qdata_full(x64emu_t* emu, void* pspec, uint32_t quark, void* data, void* notify)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

    my->g_param_spec_set_qdata_full(pspec, quark, data, findFreeFct(notify));
}

EXPORT int my_g_param_type_register_static(x64emu_t* emu, void* name, void* pspec_info)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

    return my->g_param_type_register_static(name, findFreeGParamSpecTypeInfo(pspec_info));
}

EXPORT void* my_g_value_array_sort(x64emu_t* emu, void* array, void* comp)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

    return my->g_value_array_sort(array, findcompareFct(comp));
}

EXPORT void* my_g_value_array_sort_with_data(x64emu_t* emu, void* array, void* comp, void* data)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

    return my->g_value_array_sort_with_data(array, findcompareFct(comp), data);
}

EXPORT void my_g_object_set_data_full(x64emu_t* emu, void* object, void* key, void* data, void* notify)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

    my->g_object_set_data_full(object, key, data, findFreeFct(notify));
}

EXPORT void* my_g_type_class_peek_parent(x64emu_t* emu, void* object)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;

    void* klass = my->g_type_class_peek_parent(object);
    size_t type = klass?*(size_t*)klass:0;
    return wrapCopyGTKClass(klass, type);
}

EXPORT void my_g_signal_emit_valist(x64emu_t* emu, void* inst, uint32_t id, void* quark, x64_va_list_t b)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;
    CONVERT_VALIST(b);
    my->g_signal_emit_valist(inst, id, quark, VARARGS);
}

EXPORT void my_g_signal_emit(x64emu_t* emu, void* inst, uint32_t id, void* quark, x64_va_list_t b)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 3);
    my->g_signal_emit_valist(inst, id, quark, VARARGS);
}

EXPORT void my_g_object_get_valist(x64emu_t* emu, void* a1, void* a2, x64_va_list_t b)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;
    CONVERT_VALIST(b);
    my->g_object_get_valist(a1, a2, VARARGS);
}

EXPORT void my_g_object_get(x64emu_t* emu, void* a1, void* a2, uintptr_t* b)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    my->g_object_get_valist(a1, a2, VARARGS);
}


EXPORT void my_g_object_set_valist(x64emu_t* emu, void* a1, void* a2, x64_va_list_t b)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;
    CONVERT_VALIST(b);
    my->g_object_set_valist(a1, a2, VARARGS);
}

EXPORT void my_g_object_set(x64emu_t* emu, void* a1, void* a2, uintptr_t* b)
{
    gobject2_my_t *my = (gobject2_my_t*)my_lib->priv.w.p2;
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    my->g_object_set_valist(a1, a2, VARARGS);
}

#define PRE_INIT    \
    if(box64_nogtk) \
        return -1;

#define CUSTOM_INIT \
    InitGTKClass(lib->priv.w.bridge);           \
    lib->priv.w.p2 = getGobject2My(lib);        \
    my_lib = lib;                               \
    SetGObjectID(((gobject2_my_t*)lib->priv.w.p2)->g_object_get_type());        \
    SetGTypeName(((gobject2_my_t*)lib->priv.w.p2)->g_type_name);                \
    lib->priv.w.needed = 1;                     \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libglib-2.0.so.0");

#define CUSTOM_FINI \
    FiniGTKClass();                 \
    freeGobject2My(lib->priv.w.p2); \
    free(lib->priv.w.p2);           \
    my_lib = NULL;

#include "wrappedlib_init.h"

