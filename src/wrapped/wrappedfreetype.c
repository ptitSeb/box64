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

typedef int (*iFpplp_t)     (void*, void*, long, void*);
typedef int (*iFpuuLppp_t)  (void*, uint32_t, uint32_t, uintptr_t, void*, void*, void*);

#define SUPER() \
    GO(FT_Open_Face, iFpplp_t)      \
    GO(FTC_Manager_New, iFpuuLppp_t)

typedef struct freetype_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} freetype_my_t;

void* getFreeTypeMy(library_t* lib)
{
    freetype_my_t* my = (freetype_my_t*)calloc(1, sizeof(freetype_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeFreeTypeMy(void* lib)
{
    //freetype_my_t *my = (freetype_my_t *)lib;
}

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// FTC_Face_Requester
#define GO(A)   \
static uintptr_t my_FTC_Face_Requester_fct_##A = 0;                                                     \
static int my_FTC_Face_Requester_##A(void* face_id, void* lib, void* req, void* aface)                  \
{                                                                                                       \
    return (int)RunFunction(my_context, my_FTC_Face_Requester_fct_##A, 4, face_id, lib, req, aface);    \
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
    library_t* lib = GetLibInternal(freetypeName);
    freetype_my_t* my = (freetype_my_t*)lib->priv.w.p2;

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
    library_t* lib = GetLibInternal(freetypeName);
    freetype_my_t* my = (freetype_my_t*)lib->priv.w.p2;

    return my->FTC_Manager_New(l, max_faces, max_sizes, max_bytes, find_FTC_Face_Requester_Fct(req), data, aman);
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getFreeTypeMy(lib);

#define CUSTOM_FINI \
    freeFreeTypeMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"
