// SPDX-License-Identifier: MIT
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
#include "librarian.h"
#include "box64context.h"
#include "emu/x64emu_private.h"

const char* pangoName = "libpango-1.0.so.0";
#define ALTNAME "libpango-1.0.so"
#define LIBNAME pango

#include "generated/wrappedpangotypes.h"

#include "wrappercallback.h"

typedef struct my_PangoAttrClass_s {
  int                type;
  void*            (*copy) (void *attr);
  void             (*destroy) (void *attr);
  int              (*equal) (void *attr1, void *attr2);
} my_PangoAttrClass_t;

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// PangoAttrClass
#define GO(A)   \
static my_PangoAttrClass_t* my_PangoAttrClass_used_##A = NULL;  \
static my_PangoAttrClass_t my_PangoAttrClass_struct_##A = {0};  \
static uintptr_t my_PangoAttrClass_copy_##A = 0;                \
static void* my_PangoAttrClass_copyfct##A(void* attr)           \
{                                                               \
    return (void*)RunFunctionFmt(my_PangoAttrClass_copy_##A, "p", attr);  \
}                                                               \
static uintptr_t my_PangoAttrClass_del_##A = 0;                 \
static void my_PangoAttrClass_delfct##A(void* attr)             \
{                                                               \
    RunFunctionFmt(my_PangoAttrClass_del_##A, "p", attr);\
}                                                               \
static uintptr_t my_PangoAttrClass_equal_##A = 0;               \
static int my_PangoAttrClass_equalfct##A(void* a, void* b)      \
{                                                               \
    return (int)RunFunctionFmt(my_PangoAttrClass_equal_##A, "pp", a, b);\
}
SUPER()
#undef GO
static void* find_PangoAttrClass_Fct(my_PangoAttrClass_t* klass)
{
    if(!klass) return NULL;
    #define GO(A) if(my_PangoAttrClass_used_##A == klass) return &my_PangoAttrClass_struct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_PangoAttrClass_used_##A == 0) {         \
        my_PangoAttrClass_used_##A = klass;                     \
        my_PangoAttrClass_t*p=&my_PangoAttrClass_struct_##A;    \
        p->type = klass->type;                                  \
        p->copy = my_PangoAttrClass_copyfct##A;                 \
        my_PangoAttrClass_copy_##A = (uintptr_t)klass->copy;    \
        p->destroy = my_PangoAttrClass_delfct##A;               \
        my_PangoAttrClass_del_##A = (uintptr_t)klass->destroy;  \
        p->equal = my_PangoAttrClass_equalfct##A;               \
        my_PangoAttrClass_equal_##A = (uintptr_t)klass->equal;  \
        return p;                                               \
    }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pango PangoAttrClass klass\n");
    return NULL;
}

#undef SUPER

EXPORT void my_pango_attribute_init(x64emu_t* emu, void* attr, my_PangoAttrClass_t* klass)
{
    (void)emu;
    my->pango_attribute_init(attr, find_PangoAttrClass_Fct(klass));
}

#define PRE_INIT \
    if (BOX64ENV(nogtk)) return -2;

#define NEEDED_LIBS "libgobject-2.0.so.0", "libglib-2.0.so.0"

#undef SUPER

// PangoAttrFilterFunc
#define SUPER() GO(0) GO(1) GO(2) GO(3) GO(4)
#define GO(A)   \
static uintptr_t my_filter_fct_##A = 0;                                 \
static int my_filter_##A(void* attr, void* data)                        \
{                                                                       \
    return (int)RunFunctionFmt(my_filter_fct_##A, "pp", attr, data);    \
}
SUPER()
#undef GO
static void* find_pango_filter_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_filter_fct_##A == (uintptr_t)fct) return my_filter_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_filter_fct_##A == 0) {my_filter_fct_##A = (uintptr_t)fct; return my_filter_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pango filter callback\n");
    return NULL;
}
#undef SUPER

// PangoAttrDataCopyFunc
#define SUPER() GO(0) GO(1) GO(2) GO(3) GO(4)
#define GO(A)   \
static uintptr_t my_datacopy_fct_##A = 0;                               \
static void* my_datacopy_##A(void* data)                                \
{                                                                       \
    return (void*)RunFunctionFmt(my_datacopy_fct_##A, "p", data);       \
}
SUPER()
#undef GO
static void* find_pango_datacopy_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_datacopy_fct_##A == (uintptr_t)fct) return my_datacopy_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_datacopy_fct_##A == 0) {my_datacopy_fct_##A = (uintptr_t)fct; return my_datacopy_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pango datacopy callback\n");
    return NULL;
}
#undef SUPER

// GDestroyNotify for pango attribute data
#define SUPER() GO(0) GO(1) GO(2) GO(3) GO(4)
#define GO(A)   \
static uintptr_t my_datadestroy_fct_##A = 0;                            \
static void my_datadestroy_##A(void* data)                              \
{                                                                       \
    RunFunctionFmt(my_datadestroy_fct_##A, "p", data);                  \
}
SUPER()
#undef GO
static void* find_pango_datadestroy_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_datadestroy_fct_##A == (uintptr_t)fct) return my_datadestroy_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_datadestroy_fct_##A == 0) {my_datadestroy_fct_##A = (uintptr_t)fct; return my_datadestroy_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pango datadestroy callback\n");
    return NULL;
}
#undef SUPER

EXPORT void* my_pango_attr_list_filter(x64emu_t* emu, void* list, void* func, void* data)
{
    return my->pango_attr_list_filter(list, find_pango_filter_Fct(func), data);
}

EXPORT void* my_pango_attr_shape_new_with_data(x64emu_t* emu, void* ink_rect, void* logical_rect, void* data, void* copy_func, void* destroy_func)
{
    return my->pango_attr_shape_new_with_data(ink_rect, logical_rect, data, find_pango_datacopy_Fct(copy_func), find_pango_datadestroy_Fct(destroy_func));
}

#include "wrappedlib_init.h"
