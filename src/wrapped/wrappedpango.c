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

#ifdef ANDROID
    const char* pangoName = "libpango-1.0.so";
#else
    const char* pangoName = "libpango-1.0.so.0";
#endif
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

#define PRE_INIT    \
    if(box64_nogtk) \
        return -1;

#ifdef ANDROID
#define NEEDED_LIBS "libgobject-2.0.so", "libglib-2.0.so"
#else
#define NEEDED_LIBS "libgobject-2.0.so.0", "libglib-2.0.so.0"
#endif

#include "wrappedlib_init.h"
