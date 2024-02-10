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

const char* freetypeName = 
#ifdef ANDROID
	"libfreetype.so"
#else
    "libfreetype.so.6"
#endif
    ;
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

typedef struct  FT_MemoryRec_s
{
    void*           user;
    void*           alloc;
    void*           free;
    void*           realloc;
} FT_MemoryRec_t;

#define ADDED_FUNCTIONS()                   \

#include "generated/wrappedfreetypetypes.h"

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
static uintptr_t my_FT_Generic_Finalizer_fct_##A = 0;                         \
static void my_FT_Generic_Finalizer_##A(void* object)                         \
{                                                                             \
    RunFunctionFmt(my_FT_Generic_Finalizer_fct_##A, "p", object);         \
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
static uintptr_t my_FTC_Face_Requester_fct_##A = 0;                                                             \
static int my_FTC_Face_Requester_##A(void* face_id, void* lib, void* req, void* aface)                          \
{                                                                                                               \
    int ret = (int)RunFunctionFmt(my_FTC_Face_Requester_fct_##A, "pppp", face_id, lib, req, aface);       \
    if(aface && *(void**)aface) {                                                                               \
        FT_FaceRec_t *f = *(FT_FaceRec_t**)aface;                                                               \
        f->generic.finalizer = find_FT_Generic_Finalizer_Fct(f->generic.finalizer);                             \
    }                                                                                                           \
    return ret;                                                                                                 \
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
// FT_Alloc
#define GO(A)   \
static uintptr_t my_FT_Alloc_fct_##A = 0;                                              \
static void* my_FT_Alloc_##A(void* memory, long size)                                  \
{                                                                                      \
    return (void*)RunFunctionFmt(my_FT_Alloc_fct_##A, "pl", memory, size);        \
}
SUPER()
#undef GO
static void* find_FT_Alloc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_FT_Alloc_fct_##A == (uintptr_t)fct) return my_FT_Alloc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FT_Alloc_fct_##A == 0) {my_FT_Alloc_fct_##A = (uintptr_t)fct; return my_FT_Alloc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libfreetype FT_Alloc callback\n");
    return NULL;
}
// FT_Free
#define GO(A)   \
static uintptr_t my_FT_Free_fct_##A = 0;                             \
static void my_FT_Free_##A(void* memory, void* p)                    \
{                                                                    \
    RunFunctionFmt(my_FT_Free_fct_##A, "pp", memory, p);      \
}
SUPER()
#undef GO
static void* find_FT_Free_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_FT_Free_fct_##A == (uintptr_t)fct) return my_FT_Free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FT_Free_fct_##A == 0) {my_FT_Free_fct_##A = (uintptr_t)fct; return my_FT_Free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libfreetype FT_Free callback\n");
    return NULL;
}
// FT_Realloc
#define GO(A)   \
static uintptr_t my_FT_Realloc_fct_##A = 0;                                                        \
static void* my_FT_Realloc_##A(void* memory, long cur, long size, void* p)                         \
{                                                                                                  \
    return (void*)RunFunctionFmt(my_FT_Realloc_fct_##A, "pllp", memory, cur, size, p);        \
}
SUPER()
#undef GO
static void* find_FT_Realloc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_FT_Realloc_fct_##A == (uintptr_t)fct) return my_FT_Realloc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FT_Realloc_fct_##A == 0) {my_FT_Realloc_fct_##A = (uintptr_t)fct; return my_FT_Realloc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libfreetype FT_Realloc callback\n");
    return NULL;
}
// FT_Outline_MoveToFunc
#define GO(A)   \
static uintptr_t my_FT_Outline_MoveToFunc_fct_##A = 0;                                        \
static int my_FT_Outline_MoveToFunc_##A(void* to, void* user)                                 \
{                                                                                             \
    return (int)RunFunctionFmt(my_FT_Outline_MoveToFunc_fct_##A, "pp", to, user);         \
}
SUPER()
#undef GO
static void* find_FT_Outline_MoveToFunc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_FT_Outline_MoveToFunc_fct_##A == (uintptr_t)fct) return my_FT_Outline_MoveToFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FT_Outline_MoveToFunc_fct_##A == 0) {my_FT_Outline_MoveToFunc_fct_##A = (uintptr_t)fct; return my_FT_Outline_MoveToFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libfreetype FT_Outline_MoveToFunc callback\n");
    return NULL;
}

// FT_Outline_LineToFunc
#define GO(A)   \
static uintptr_t my_FT_Outline_LineToFunc_fct_##A = 0;                                        \
static int my_FT_Outline_LineToFunc_##A(void* to, void* user)                                 \
{                                                                                             \
    return (int)RunFunctionFmt(my_FT_Outline_LineToFunc_fct_##A, "pp", to, user);         \
}
SUPER()
#undef GO
static void* find_FT_Outline_LineToFunc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_FT_Outline_LineToFunc_fct_##A == (uintptr_t)fct) return my_FT_Outline_LineToFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FT_Outline_LineToFunc_fct_##A == 0) {my_FT_Outline_LineToFunc_fct_##A = (uintptr_t)fct; return my_FT_Outline_LineToFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libfreetype FT_Outline_LineToFunc callback\n");
    return NULL;
}

// FT_Outline_ConicToFunc
#define GO(A)   \
static uintptr_t my_FT_Outline_ConicToFunc_fct_##A = 0;                                              \
static int my_FT_Outline_ConicToFunc_##A(void* ctl, void* to, void* user)                            \
{                                                                                                    \
    return (int)RunFunctionFmt(my_FT_Outline_ConicToFunc_fct_##A, "ppp", ctl, to, user);      \
}
SUPER()
#undef GO
static void* find_FT_Outline_ConicToFunc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_FT_Outline_ConicToFunc_fct_##A == (uintptr_t)fct) return my_FT_Outline_ConicToFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FT_Outline_ConicToFunc_fct_##A == 0) {my_FT_Outline_ConicToFunc_fct_##A = (uintptr_t)fct; return my_FT_Outline_ConicToFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libfreetype FT_Outline_ConicToFunc callback\n");
    return NULL;
}

// FT_Outline_CubicToFunc
#define GO(A)   \
static uintptr_t my_FT_Outline_CubicToFunc_fct_##A = 0;                                                      \
static int my_FT_Outline_CubicToFunc_##A(void* ctl1, void* ctl2, void* to, void* user)                       \
{                                                                                                            \
    return (int)RunFunctionFmt(my_FT_Outline_CubicToFunc_fct_##A, "pppp", ctl1, ctl2, to, user);      \
}
SUPER()
#undef GO
static void* find_FT_Outline_CubicToFunc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_FT_Outline_CubicToFunc_fct_##A == (uintptr_t)fct) return my_FT_Outline_CubicToFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FT_Outline_CubicToFunc_fct_##A == 0) {my_FT_Outline_CubicToFunc_fct_##A = (uintptr_t)fct; return my_FT_Outline_CubicToFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libfreetype FT_Outline_CubicToFunc callback\n");
    return NULL;
}

// structures
#define GO(A)   \
static FT_MemoryRec_t my_FT_MemoryRec_struct_##A = {0}; \
static FT_MemoryRec_t* my_FT_MemoryRec_ref_##A = NULL;
SUPER()
#undef GO
static void wrap_FT_MemoryRec(FT_MemoryRec_t* dst, FT_MemoryRec_t* src) {
    dst->user = src->user;
    dst->alloc = find_FT_Alloc_Fct(src->alloc);
    dst->free = find_FT_Free_Fct(src->free);
    dst->realloc = find_FT_Realloc_Fct(src->realloc);
}
static FT_MemoryRec_t* find_FT_MemoryRec_Struct(FT_MemoryRec_t* s)
{
    if(!s)  return NULL;
    #define GO(A) if(my_FT_MemoryRec_ref_##A == s) return &my_FT_MemoryRec_struct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(!my_FT_MemoryRec_ref_##A) {wrap_FT_MemoryRec(&my_FT_MemoryRec_struct_##A, s); my_FT_MemoryRec_ref_##A = s; return &my_FT_MemoryRec_struct_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libfreetype Struct FT_MemoryRec wrapping\n");
    return s;
}


#undef SUPER

static uintptr_t my_iofunc = 0;
static unsigned long my_FT_Stream_IoFunc(FT_StreamRec_t* stream, unsigned long offset, unsigned char* buffer, unsigned long count )
{
    return (unsigned long)RunFunctionFmt(my_iofunc, "pLpL", stream, offset, buffer, count)        ;
}

static uintptr_t my_closefunc = 0;
static void my_FT_Stream_CloseFunc(FT_StreamRec_t* stream)
{
    RunFunctionFmt(my_closefunc, "p", stream)     ;
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

EXPORT int my_FT_New_Library(x64emu_t* emu, FT_MemoryRec_t* memory, void* p)
{
    return my->FT_New_Library(find_FT_MemoryRec_Struct(memory), p);
}

typedef struct  my_FT_Outline_Funcs_s
{
    void*       move_to;
    void*       line_to;
    void*       conic_to;
    void*       cubic_to;
    int         shift;
    signed long delta;
} my_FT_Outline_Funcs_t;

EXPORT int my_FT_Outline_Decompose(x64emu_t* emu, void* outline, my_FT_Outline_Funcs_t* tbl, void* data)
{
    my_FT_Outline_Funcs_t f = {0};
    if(tbl) {
        f.move_to = find_FT_Outline_MoveToFunc_Fct(tbl->move_to);
        f.line_to = find_FT_Outline_LineToFunc_Fct(tbl->line_to);
        f.conic_to = find_FT_Outline_ConicToFunc_Fct(tbl->conic_to);
        f.cubic_to = find_FT_Outline_CubicToFunc_Fct(tbl->cubic_to);
        f.shift = tbl->shift;
        f.delta = tbl->delta;
    }
    return my->FT_Outline_Decompose(outline, tbl?(&f):tbl, data);
}

#include "wrappedlib_init.h"
