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

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// vblank_handler
#define GO(A)                                                                           \
static uintptr_t my32_vblank_handler_fct_##A = 0;                                       \
static void my32_vblank_handler_##A(int a, uint32_t b, uint32_t c, uint32_t d, void* e) \
{                                                                                       \
    RunFunctionFmt(my32_vblank_handler_fct_##A, "iuuup", a, b, c, d, e);                \
}
SUPER()
#undef GO
static void* find_vblank_handler_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_vblank_handler_fct_##A == (uintptr_t)fct) return my32_vblank_handler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_vblank_handler_fct_##A == 0) {my32_vblank_handler_fct_##A = (uintptr_t)fct; return my32_vblank_handler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libdrm 32bits vblank_handler callback\n");
    return NULL;
}

// page_flip_handler
#define GO(A)                                                                               \
static uintptr_t my32_page_flip_handler_fct_##A = 0;                                        \
static void my32_page_flip_handler_##A(int a, uint32_t b, uint32_t c, uint32_t d, void* e)  \
{                                                                                           \
    RunFunctionFmt(my32_page_flip_handler_fct_##A, "iuuup", a, b, c, d, e);                 \
}
SUPER()
#undef GO
static void* find_page_flip_handler_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_page_flip_handler_fct_##A == (uintptr_t)fct) return my32_page_flip_handler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_page_flip_handler_fct_##A == 0) {my32_page_flip_handler_fct_##A = (uintptr_t)fct; return my32_page_flip_handler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libdrm 32bits page_flip_handler callback\n");
    return NULL;
}

// page_flip_handler2
#define GO(A)                                                                                           \
static uintptr_t my32_page_flip_handler2_fct_##A = 0;                                                   \
static void my32_page_flip_handler2_##A(int a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, void* f) \
{                                                                                                       \
    RunFunctionFmt(my32_page_flip_handler2_fct_##A, "iuuuup", a, b, c, d, e, f);                        \
}
SUPER()
#undef GO
static void* find_page_flip_handler2_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_page_flip_handler2_fct_##A == (uintptr_t)fct) return my32_page_flip_handler2_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_page_flip_handler2_fct_##A == 0) {my32_page_flip_handler2_fct_##A = (uintptr_t)fct; return my32_page_flip_handler2_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libdrm 32bits page_flip_handler2 callback\n");
    return NULL;
}

// sequence_handler
#define GO(A)                                                                       \
static uintptr_t my32_sequence_handler_fct_##A = 0;                                 \
static void my32_sequence_handler_##A(int a, uint64_t b, uint64_t c, uint64_t d)    \
{                                                                                   \
    RunFunctionFmt(my32_sequence_handler_fct_##A, "iUUU", a, b, c, d);              \
}
SUPER()
#undef GO
static void* find_sequence_handler_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_sequence_handler_fct_##A == (uintptr_t)fct) return my32_sequence_handler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_sequence_handler_fct_##A == 0) {my32_sequence_handler_fct_##A = (uintptr_t)fct; return my32_sequence_handler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libdrm 32bits sequence_handler callback\n");
    return NULL;
}
#undef SUPER

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

typedef struct my_drmModeConnector_s {
    uint32_t connector_id;
    uint32_t encoder_id;
    uint32_t connector_type;
    uint32_t connector_type_id;
    uint32_t connection;
    uint32_t mmWidth, mmHeight;
    uint32_t subpixel;
    int count_modes;
    void* modes;    //drmModeModeInfoPtr doesn't need conversion
    int count_props;
    uint32_t *props;
    uint64_t *prop_values;
    int count_encoders;
    uint32_t *encoders;
} my_drmModeConnector_t;
typedef struct my_drmModeConnector_32_s {
    uint32_t connector_id;
    uint32_t encoder_id;
    uint32_t connector_type;
    uint32_t connector_type_id;
    uint32_t connection;
    uint32_t mmWidth, mmHeight;
    uint32_t subpixel;
    int count_modes;
    ptr_t modes;    //drmModeModeInfoPtr doesn't need conversion
    int count_props;
    ptr_t props;    //uint32_t *
    ptr_t prop_values;  //uint64_t *
    int count_encoders;
    ptr_t encoders; //uint32_t *
} my_drmModeConnector_32_t;

void* inplace_drmModeConnector_shrink(void* a)
{
    if(a) {
        my_drmModeConnector_t* src = a;
        my_drmModeConnector_32_t* dst = a;
        dst->connector_id = src->connector_id;
        dst->encoder_id = src->encoder_id;
        dst->connector_type = src->connector_type;
        dst->connector_type_id = src->connector_type_id;
        dst->connection = src->connection;
        dst->mmWidth = src->mmWidth;
        dst->mmHeight = src->mmHeight;
        dst->subpixel = src->subpixel;
        dst->count_modes = src->count_modes;
        dst->modes = to_ptrv(src->modes);
        dst->count_props = src->count_props;
        dst->props = to_ptrv(src->props);
        dst->prop_values = to_ptrv(src->prop_values);
        dst->count_encoders = src->count_encoders;
        dst->encoders = to_ptrv(src->encoders);
    }
    return a;
}
void* inplace_drmModeConnector_enlarge(void* a)
{
    if(a) {
        my_drmModeConnector_t* dst = a;
        my_drmModeConnector_32_t* src = a;
        dst->encoders = from_ptrv(src->encoders);
        dst->count_encoders = src->count_encoders;
        dst->prop_values = from_ptrv(src->prop_values);
        dst->props = from_ptrv(src->props);
        dst->count_props = src->count_props;
        dst->modes = from_ptrv(src->modes);
        dst->count_modes = src->count_modes;
        dst->subpixel = src->subpixel;
        dst->mmHeight = src->mmHeight;
        dst->mmWidth = src->mmWidth;
        dst->connection = src->connection;
        dst->connector_type_id = src->connector_type_id;
        dst->connector_type = src->connector_type;
        dst->encoder_id = src->encoder_id;
        dst->connector_id = src->connector_id;
    }
    return a;
}

typedef struct my_drmModeRes_s {
    int count_fbs;
    uint32_t *fbs;
    int count_crtcs;
    uint32_t *crtcs;
    int count_connectors;
    uint32_t *connectors;
    int count_encoders;
    uint32_t *encoders;
    uint32_t min_width, max_width;
    uint32_t min_height, max_height;
} my_drmModeRes_t;
typedef struct my_drmModeRes_32_s {
    int count_fbs;
    ptr_t fbs;  //uint32_t *
    int count_crtcs;
    ptr_t crtcs;    //uint32_t *
    int count_connectors;
    ptr_t connectors;   //uint32_t *
    int count_encoders;
    ptr_t encoders; //uint32_t *
    uint32_t min_width, max_width;
    uint32_t min_height, max_height;
} my_drmModeRes_32_t;

void* inplace_drmModeRes_shrink(void* a)
{
    if(a) {
        my_drmModeRes_t* src = a;
        my_drmModeRes_32_t* dst = a;
        dst->count_fbs = src->count_fbs;
        dst->fbs = to_ptrv(src->fbs);
        dst->count_crtcs = src->count_crtcs;
        dst->crtcs = to_ptrv(src->crtcs);
        dst->count_connectors = src->count_connectors;
        dst->connectors = to_ptrv(src->connectors);
        dst->count_encoders = src->count_encoders;
        dst->encoders = to_ptrv(src->encoders);
        dst->min_width = src->min_width;
        dst->max_width = src->max_width;
        dst->min_height = src->min_height;
        dst->max_height = src->max_height;
    }
    return a;
}
void* inplace_drmModeRes_enlarge(void* a)
{
    if(a) {
        my_drmModeRes_t* dst = a;
        my_drmModeRes_32_t* src = a;
        dst->max_height = src->max_height;
        dst->min_height = src->min_height;
        dst->max_width = src->max_width;
        dst->min_width = src->min_width;
        dst->encoders = from_ptrv(src->encoders);
        dst->count_encoders = src->count_encoders;
        dst->connectors = from_ptrv(src->connectors);
        dst->count_connectors = src->count_connectors;
        dst->crtcs = from_ptrv(src->crtcs);
        dst->count_crtcs = src->count_crtcs;
        dst->fbs = from_ptrv(src->fbs);
        dst->count_fbs = src->count_fbs;
    }
    return a;
}

typedef struct my_drmEventContext_s
{
    int version;
    void (*vblank_handler)(int fd,
                           unsigned int sequence, 
                           unsigned int tv_sec,
                           unsigned int tv_usec,
                           void *user_data);
    void (*page_flip_handler)(int fd,
                              unsigned int sequence,
                              unsigned int tv_sec,
                              unsigned int tv_usec,
                              void *user_data);
    void (*page_flip_handler2)(int fd,
                               unsigned int sequence,
                               unsigned int tv_sec,
                               unsigned int tv_usec,
                               unsigned int crtc_id,
                               void *user_data);
    void (*sequence_handler)(int fd,
                             uint64_t sequence,
                             uint64_t ns,
                             uint64_t user_data);
} my_drmEventContext_t;
typedef struct my_drmEventContext_32_s
{
    int version;
    ptr_t vblank_handler;
    ptr_t page_flip_handler;
    ptr_t page_flip_handler2;
    ptr_t sequence_handler;
} my_drmEventContext_32_t;

EXPORT void* my32_drmGetVersion(x64emu_t* emu, int fd)
{
    return inplace_drmVersion_shrink(my->drmGetVersion(fd));
}

EXPORT void my32_drmFreeVersion(x64emu_t* emu, void* v)
{
    my->drmFreeVersion(inplace_drmVersion_enlarge(v));
}

EXPORT void* my32_drmModeGetConnector(x64emu_t* emu, int fd, uint32_t id)
{
    return inplace_drmModeConnector_shrink(my->drmModeGetConnector(fd, id));
}

EXPORT void* my32_drmModeGetConnectorCurrent(x64emu_t* emu, int fd, uint32_t id)
{
    return inplace_drmModeConnector_shrink(my->drmModeGetConnectorCurrent(fd, id));
}

EXPORT void my32_drmModeFreeConnector(void* v)
{
    my->drmModeFreeConnector(inplace_drmModeConnector_enlarge(v));
}

EXPORT void my32_drmModeFreeResources(void* v)
{
    my->drmModeFreeResources(inplace_drmModeRes_enlarge(v));
}

EXPORT void* my32_drmModeGetResources(int fd)
{
    return inplace_drmModeRes_shrink(my->drmModeGetResources(fd));
}

EXPORT int my32_drmHandleEvent(x64emu_t* emu, int fd, my_drmEventContext_32_t* event)
{
    my_drmEventContext_t ctx = {0};
    if(event) {
        ctx.version = event->version;
        ctx.vblank_handler = find_vblank_handler_Fct(from_ptrv(event->vblank_handler));
        ctx.page_flip_handler = find_page_flip_handler_Fct(from_ptrv(event->page_flip_handler));
        ctx.page_flip_handler2 = find_page_flip_handler2_Fct(from_ptrv(event->page_flip_handler2));
        ctx.sequence_handler = find_sequence_handler_Fct(from_ptrv(event->sequence_handler));
    }
    return my->drmHandleEvent(fd, event?(&ctx):NULL);
}

#include "wrappedlib_init32.h"
