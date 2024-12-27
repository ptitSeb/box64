#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "myalign32.h"

static const char* libdrmName = "libdrm.so.2";
#define LIBNAME libdrm

#define ADDED_FUNCTIONS()                   \

#include "generated/wrappedlibdrmtypes32.h"

#include "wrappercallback32.h"

//EXPORT void my32_drmMsg(x64emu_t* emu, void* fmt, void* b) {
//    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
//    PREPARE_VALIST;
//    char* buf = NULL;
//    int dummy = vasprintf(&buf, (const char*)fmt, VARARGS);
//    (void)dummy;
//    my->drmMsg(buf);
//    free(buf);
//}

typedef struct my_drmVersion_s {
    int     version_major;
    int     version_minor;
    int     version_patchlevel;
    int     name_len;
    char*   name;
    int     date_len;
    char*   date;
    int     desc_len;
    char*   desc;
} my_drmVersion_t;
typedef struct my_drmVersion_32_s {
    int     version_major;
    int     version_minor;
    int     version_patchlevel;
    int     name_len;
    ptr_t   name;   //char*
    int     date_len;
    ptr_t   date;   //char*
    int     desc_len;
    ptr_t   desc;   //char*
} my_drmVersion_32_t;
void* inplace_drmVersion_shrink(void* a)
{
    if(a) {
        my_drmVersion_t* src = a;
        my_drmVersion_32_t* dst = a;
        dst->version_major = src->version_major;
        dst->version_minor = src->version_minor;
        dst->version_patchlevel = src->version_patchlevel;
        dst->name_len = src->name_len;
        dst->name = to_ptrv(src->name);
        dst->date_len = src->date_len;
        dst->date = to_ptrv(src->date);
        dst->desc_len = src->desc_len;
        dst->desc = to_ptrv(src->desc);
    }
    return a;
}
void* inplace_drmVersion_enlarge(void* a)
{
    if(a) {
        my_drmVersion_32_t* src = a;
        my_drmVersion_t* dst = a;
        dst->desc = from_ptrv(src->desc);
        dst->desc_len = src->desc_len;
        dst->date = from_ptrv(src->date);
        dst->date_len = src->date_len;
        dst->name = from_ptrv(src->name);
        dst->name_len = src->name_len;
        dst->version_patchlevel = src->version_patchlevel;
        dst->version_minor = src->version_minor;
        dst->version_major = src->version_major;
    }
    return a;
}

EXPORT void* my32_drmGetVersion(x64emu_t* emu, int fd)
{
    return inplace_drmVersion_shrink(my->drmGetVersion(fd));
}

EXPORT void my32_drmFreeVersion(x64emu_t* emu, void* v)
{
    my->drmFreeVersion(inplace_drmVersion_enlarge(v));
}

#include "wrappedlib_init32.h"
