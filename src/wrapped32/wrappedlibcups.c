#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box32context.h"
#include "emu/x64emu_private.h"
#include "myalign.h"

static const char* libcupsName = "libcups.so.2";
#define ALTNAME "libcups.so"

#define LIBNAME libcups

//------- 64 bits structures
typedef struct my_cups_option_s
{
    char*     name;
    char*     value;
} my_cups_option_t;

typedef struct my_cups_dest_s
{
    char*               name;
    char*               instance;
    int                 is_default;
    int                 num_options;
    my_cups_option_t*   options;
} my_cups_dest_t;

//------- 32 bits structures
typedef struct my_cups_option_32_s
{
    ptr_t     name; //char*
    ptr_t     value;    //char*
} my_cups_option_32_t;

typedef struct my_cups_dest_32_s
{
    ptr_t               name;   //char*
    ptr_t               instance;   //char*
    int                 is_default;
    int                 num_options;
    ptr_t               options;    //my_cups_option_t*
} my_cups_dest_32_t;

//------- Conversions

void* inplace_cups_option_shrink(void* a)
{
    if(a) {
        my_cups_option_t* src = a;
        my_cups_option_32_t* dst = a;
        dst->name = to_ptrv(src->name);
        dst->value = to_ptrv(src->value);
    }
    return a;
}
void* inplace_cups_option_enlarge(void* a)
{
    if(a) {
        my_cups_option_32_t* src = a;
        my_cups_option_t* dst = a;
        dst->value = from_ptrv(src->value);
        dst->name = from_ptrv(src->name);
    }
    return a;
}

void convert_cups_dest_to_32(void* s, void* d)
{
    if(!s || !d) return;
    my_cups_dest_t* src = s;
    my_cups_dest_32_t* dst = d;
    dst->name = to_ptrv(src->name);
    dst->instance = to_ptrv(src->instance);
    dst->is_default = src->is_default;
    dst->num_options = src->num_options;
    dst->options = to_ptrv(inplace_cups_option_shrink(src->options));
}
void convert_cups_dest_to_64(void* s, void* d)
{
    if(!s || !d) return;
    my_cups_dest_32_t* src = s;
    my_cups_dest_t*dst = d;
    dst->options = inplace_cups_option_enlarge(from_ptrv(src->options));
    dst->num_options = src->num_options;
    dst->is_default = src->is_default;
    dst->instance = from_ptrv(src->instance);
    dst->name = from_ptrv(src->name);
}
void* inplace_cups_dest_shrink(void* a)
{
    if(a) convert_cups_dest_to_32(a, a);
    return a;
}
void* inplace_cups_dest_enlarge(void* a)
{
    if(a) convert_cups_dest_to_64(a, a);
    return a;
}

// Wrappers

#define ADDED_FUNCTIONS()                   \

#include "generated/wrappedlibcupstypes32.h"

#include "wrappercallback32.h"

#if 0
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// cups_dest_cb_t ...
#define GO(A)   \
static uintptr_t my_cups_dest_cb_t_fct_##A = 0;                                         \
static int my_cups_dest_cb_t_##A(void* a, uint32_t b, void* c)                          \
{                                                                                       \
    return (int)RunFunctionFmt(my_cups_dest_cb_t_fct_##A, "pup", a, b, c);        \
}
SUPER()
#undef GO
static void* find_cups_dest_cb_t_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_cups_dest_cb_t_fct_##A == (uintptr_t)fct) return my_cups_dest_cb_t_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_cups_dest_cb_t_fct_##A == 0) {my_cups_dest_cb_t_fct_##A = (uintptr_t)fct; return my_cups_dest_cb_t_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcups cups_dest_cb_t callback\n");
    return NULL;
}
// cups_password_cb2_t ...
#define GO(A)   \
static uintptr_t my_cups_password_cb2_t_fct_##A = 0;                                        \
static void* my_cups_password_cb2_t_##A(void* a, void* b, void* c, void* d, void* e)        \
{                                                                                           \
    return (void*)RunFunctionFmt(my_cups_password_cb2_t_fct_##A, "ppppp", a, b, c, d, e);   \
}
SUPER()
#undef GO
static void* find_cups_password_cb2_t_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_cups_password_cb2_t_fct_##A == (uintptr_t)fct) return my_cups_password_cb2_t_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_cups_password_cb2_t_fct_##A == 0) {my_cups_password_cb2_t_fct_##A = (uintptr_t)fct; return my_cups_password_cb2_t_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcups cups_password_cb2_t callback\n");
    return NULL;
}

#undef SUPER

EXPORT int my_cupsEnumDests(x64emu_t* emu, uint32_t flags, int msec, int* cancel, uint32_t type, uint32_t mask, void* f, void* data)
{
    return my->cupsEnumDests(flags, msec, cancel, type, mask, find_cups_dest_cb_t_Fct(f), data);
}

EXPORT void my_cupsSetPasswordCB2(x64emu_t* emu, void* f, void* data)
{
    my->cupsSetPasswordCB2(find_cups_password_cb2_t_Fct(f), data);
}
#endif

EXPORT void my32_cupsFreeDests(x64emu_t* emu, int n, void* p)
{
    my_cups_dest_32_t* src = p;
    my_cups_dest_t* dst= p;
    for(int i=n-1; i>=0; --i)
        convert_cups_dest_to_64(&dst[i], &src[i]);
    my->cupsFreeDests(n, p);
}

EXPORT int my32_cupsGetDests(x64emu_t* emu, ptr_t* p)
{
    void* p_l = from_ptrv(*p);
    int ret = my->cupsGetDests(&p_l);
    *p = to_ptrv(p_l);
    my_cups_dest_t* src = p_l;
    my_cups_dest_32_t* dst = p_l;
    for(int i=0; i<ret; ++i)
        convert_cups_dest_to_32(&dst[i], &src[i]);
    return ret;
}

EXPORT void* my32_cupsGetOption(x64emu_t* emu, void* name, int num_options, my_cups_option_32_t* options)
{
    my_cups_option_t options_l[num_options];
    for(int i=0; i<num_options; ++i) {
        options_l[i].name = from_ptrv(options[i].name);
        options_l[i].value = from_ptrv(options[i].value);
    }
    return my->cupsGetOption(name, num_options, options_l);
}

#include "wrappedlib_init32.h"
