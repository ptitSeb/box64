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
#include "myalign.h"

const char* freetypeName = "libfreetype.so.6";
#define LIBNAME freetype

typedef void  (*vFp_t)(void*);

typedef union  FT_StreamDesc_s
{
    long   value;
    void*  pointer;
} FT_StreamDesc_t;

typedef struct  FT_StreamRec_s
{
    unsigned char*       base;
    unsigned long        size;
    unsigned long        pos;

    FT_StreamDesc_t      descriptor;
    FT_StreamDesc_t      pathname;
    void*                read;
    void*                close;

    void*                memory;
    unsigned char*       cursor;
    unsigned char*       limit;

} FT_StreamRec_t;

typedef struct  FT_Open_Args_s
{
uint32_t        flags;
const uint8_t*  memory_base;
intptr_t        memory_size;
char*           pathname;
FT_StreamRec_t* stream;
void*           driver;
int32_t         num_params;
void*   params;

} FT_Open_Args_t;
typedef struct  FT_BBox_s
{
    signed long     xMin, yMin;
    signed long     xMax, yMax;
} FT_BBox_t;
typedef struct  FT_Generic_s
{
    void*           data;
    vFp_t           finalizer;
} FT_Generic_t;
typedef struct  FT_ListRec_s
{
  void*             head;
  void*             tail;
} FT_ListRec_t;
typedef struct  FT_FaceRec_s
{
    signed long     num_faces;
    signed long     face_index;
    signed long     face_flags;
    signed long     style_flags;
    signed long     num_glyphs;
    char*           family_name;
    char*           style_name;
    int             num_fixed_sizes;
    void*           available_sizes;
    int             num_charmaps;
    void*           charmaps;
    FT_Generic_t    generic;
    FT_BBox_t       bbox;
    uint16_t        units_per_EM;
    int16_t         ascender;
    int16_t         descender;
    int16_t         height;
    int16_t         max_advance_width;
    int16_t         max_advance_height;
    int16_t         underline_position;
    int16_t         underline_thickness;
    void*           glyph;
    void*           size;
    void*           charmap;
    /*@private begin */
    void*           driver;
    void*           memory;
    FT_StreamDesc_t* stream;
    FT_ListRec_t    sizes_list;
    FT_Generic_t    autohint;   /* face-specific auto-hinter data */
    void*           extensions; /* unused                         */
    void*           internal;
} FT_FaceRec_t;

typedef int (*iFpplp_t)     (void*, void*, long, void*);
typedef int (*iFpuuLppp_t)  (void*, uint32_t, uint32_t, uintptr_t, void*, void*, void*);

#define SUPER() \
    GO(FT_Open_Face, iFpplp_t)      \
    GO(FTC_Manager_New, iFpuuLppp_t)

#include "wrappercallback.h"

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// FT_Generic_Finalizer
#define GO(A)   \
static uintptr_t my_FT_Generic_Finalizer_fct_##A = 0;                       \
static void my_FT_Generic_Finalizer_##A(void* object)                       \
{                                                                           \
    RunFunction(my_context, my_FT_Generic_Finalizer_fct_##A, 1, object);    \
}
SUPER()
#undef GO
static void* find_FT_Generic_Finalizer_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_FT_Generic_Finalizer_fct_##A == (uintptr_t)fct) return my_FT_Generic_Finalizer_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FT_Generic_Finalizer_fct_##A == 0) {my_FT_Generic_Finalizer_fct_##A = (uintptr_t)fct; return my_FT_Generic_Finalizer_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libfreetype FT_Generic_Finalizer callback\n");
    return NULL;
}
// FTC_Face_Requester
#define GO(A)   \
static uintptr_t my_FTC_Face_Requester_fct_##A = 0;                                                     \
static int my_FTC_Face_Requester_##A(void* face_id, void* lib, void* req, void* aface)                  \
{                                                                                                       \
    int ret = (int)RunFunction(my_context, my_FTC_Face_Requester_fct_##A, 4, face_id, lib, req, aface); \
    if(aface && *(void**)aface) {                                                                       \
        FT_FaceRec_t *f = *(FT_FaceRec_t**)aface;                                                       \
        f->generic.finalizer = find_FT_Generic_Finalizer_Fct(f->generic.finalizer);                     \
    }                                                                                                   \
    return ret;                                                                                         \
}
SUPER()
#undef GO
static void* find_FTC_Face_Requester_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_FTC_Face_Requester_fct_##A == (uintptr_t)fct) return my_FTC_Face_Requester_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FTC_Face_Requester_fct_##A == 0) {my_FTC_Face_Requester_fct_##A = (uintptr_t)fct; return my_FTC_Face_Requester_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libfreetype FTC_Face_Requester callback\n");
    return NULL;
}


static uintptr_t my_iofunc = 0;
static unsigned long my_FT_Stream_IoFunc(FT_StreamRec_t* stream, unsigned long offset, unsigned char* buffer, unsigned long count )
{
    return (unsigned long)RunFunction(my_context, my_iofunc, 4, stream, offset, buffer, count);
}

static uintptr_t my_closefunc = 0;
static void my_FT_Stream_CloseFunc(FT_StreamRec_t* stream)
{
    RunFunction(my_context, my_closefunc, 1, stream);
}

EXPORT int my_FT_Open_Face(x64emu_t* emu, void* library, FT_Open_Args_t* args, long face_index, void* aface)
{
    (void)emu;
    int wrapstream = (args->flags&0x02)?1:0;
    if(wrapstream) {
        my_iofunc = (uintptr_t)args->stream->read;
        if(my_iofunc)
            args->stream->read = my_FT_Stream_IoFunc;
        my_closefunc = (uintptr_t)args->stream->close;
        if(my_closefunc)
            args->stream->close = my_FT_Stream_CloseFunc;
    }
    int ret = my->FT_Open_Face(library, args, face_index, aface);
    /*if(wrapstream) {
        args->stream->read = (void*)my_iofunc;
        args->stream->close = (void*)my_closefunc;
    }*/
    return ret;
}

EXPORT int my_FTC_Manager_New(x64emu_t* emu, void* l, uint32_t max_faces, uint32_t max_sizes, uintptr_t max_bytes, void* req, void* data, void* aman)
{
    (void)emu;
    return my->FTC_Manager_New(l, max_faces, max_sizes, max_bytes, find_FTC_Face_Requester_Fct(req), data, aman);
}

#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
