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
#include "box64context.h"
#include "librarian.h"
#include "myalign.h"
#include "bridge.h"

#ifdef ANDROID
    const char* flacName = "libFLAC.so";
#else
    const char* flacName = "libFLAC.so.8";
#endif

#define LIBNAME flac

typedef struct {
  size_t (*read_func)  (void *ptr, size_t size, size_t nmemb, void *handle);
  size_t (*write_func)  (void *ptr, size_t size, size_t nmemb, void *handle);
  int    (*seek_func)  (void *handle, int64_t offset, int whence);
  long   (*tell_func)  (void *handle);
  int    (*eof_func) (void *handle);
  int    (*close_func) (void *handle);
} flac_callbacks;

typedef int (*iFppC_t)(void*, void*, flac_callbacks);
typedef int (*iFpppppppppp_t)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*);

#define SUPER() \
    GO(FLAC__metadata_chain_read_with_callbacks, iFppC_t)   \
    GO(FLAC__stream_decoder_init_stream, iFpppppppppp_t)    \

#include "wrappercallback.h"

#define SUPER() \
GO(0)           \
GO(1)           \
GO(2)           \
GO(3)           \
GO(4)           \
GO(5)           \
GO(6)           \
GO(7)

// read_write
#define GO(A)   \
static uintptr_t my_read_write_fct_##A = 0;                                                              \
static unsigned long my_read_write_##A(void* ptr, unsigned long size, unsigned long nmemb, void* handle) \
{                                                                                                        \
    return RunFunctionFmt(my_read_write_fct_##A, "pLLp", ptr, size, nmemb, handle);               \
}
SUPER()
#undef GO
static void* findread_writeFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_read_write_fct_##A == (uintptr_t)fct) return my_read_write_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_read_write_fct_##A == 0) {my_read_write_fct_##A = (uintptr_t)fct; return my_read_write_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FLAC read_write callback\n");
    return NULL;
}
// seek
#define GO(A)   \
static uintptr_t my_seek_fct_##A = 0;                                                    \
static int my_seek_##A(void* ptr, int64_t offset, int whence)                            \
{                                                                                        \
    return (int)RunFunctionFmt(my_seek_fct_##A, "pIi", ptr, offset, whence);      \
}
SUPER()
#undef GO
static void* findseekFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_seek_fct_##A == (uintptr_t)fct) return my_seek_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_seek_fct_##A == 0) {my_seek_fct_##A = (uintptr_t)fct; return my_seek_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FLAC seek callback\n");
    return NULL;
}
// close_eof
#define GO(A)   \
static uintptr_t my_close_eof_fct_##A = 0;                                  \
static int my_close_eof_##A(void* ptr)                                      \
{                                                                           \
    return (int)RunFunctionFmt(my_close_eof_fct_##A, "p", ptr);       \
}
SUPER()
#undef GO
static void* findclose_eofFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_close_eof_fct_##A == (uintptr_t)fct) return my_close_eof_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_close_eof_fct_##A == 0) {my_close_eof_fct_##A = (uintptr_t)fct; return my_close_eof_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FLAC close_eof callback\n");
    return NULL;
}
// tell
#define GO(A)   \
static uintptr_t my_tell_fct_##A = 0;                                   \
static long my_tell_##A(void* ptr)                                      \
{                                                                       \
    return (long)RunFunctionFmt(my_tell_fct_##A, "p", ptr);       \
}
SUPER()
#undef GO
static void* findtellFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_tell_fct_##A == (uintptr_t)fct) return my_tell_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_tell_fct_##A == 0) {my_tell_fct_##A = (uintptr_t)fct; return my_tell_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for vorbisfile tell callback\n");
    return NULL;
}
// Read
#define GO(A)   \
static uintptr_t my_Read_fct_##A = 0;                                                              \
static int my_Read_##A(void* decoder, void* buffer, size_t* bytes, void* data)                     \
{                                                                                                  \
    return (int)RunFunctionFmt(my_Read_fct_##A, "pppp", decoder, buffer, bytes, data);        \
}
SUPER()
#undef GO
static void* findReadFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_Read_fct_##A == (uintptr_t)fct) return my_Read_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Read_fct_##A == 0) {my_Read_fct_##A = (uintptr_t)fct; return my_Read_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FLAC Read callback\n");
    return NULL;
}
// Seek
#define GO(A)   \
static uintptr_t my_Seek_fct_##A = 0;                                                      \
static int my_Seek_##A(void* decoder, uint64_t offset, void* data)                         \
{                                                                                          \
    return (int)RunFunctionFmt(my_Seek_fct_##A, "pUp", decoder, offset, data);        \
}
SUPER()
#undef GO
static void* findSeekFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_Seek_fct_##A == (uintptr_t)fct) return my_Seek_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Seek_fct_##A == 0) {my_Seek_fct_##A = (uintptr_t)fct; return my_Seek_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FLAC Seek callback\n");
    return NULL;
}
// Tell
#define GO(A)   \
static uintptr_t my_Tell_fct_##A = 0;                                                      \
static int my_Tell_##A(void* decoder, uint64_t *offset, void* data)                        \
{                                                                                          \
    return (int)RunFunctionFmt(my_Tell_fct_##A, "ppp", decoder, offset, data);        \
}
SUPER()
#undef GO
static void* findTellFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_Tell_fct_##A == (uintptr_t)fct) return my_Tell_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Tell_fct_##A == 0) {my_Tell_fct_##A = (uintptr_t)fct; return my_Tell_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FLAC Tell callback\n");
    return NULL;
}
// Length
#define GO(A)                                                                                \
static uintptr_t my_Length_fct_##A = 0;                                                      \
static int my_Length_##A(void* decoder, uint64_t *length, void* data)                        \
{                                                                                            \
    return (int)RunFunctionFmt(my_Length_fct_##A, "ppp", decoder, length, data);      \
}
SUPER()
#undef GO
static void* findLengthFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_Length_fct_##A == (uintptr_t)fct) return my_Length_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Length_fct_##A == 0) {my_Length_fct_##A = (uintptr_t)fct; return my_Length_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FLAC Length callback\n");
    return NULL;
}
// Eof
#define GO(A)   \
static uintptr_t my_Eof_fct_##A = 0;                                             \
static int my_Eof_##A(void* decoder, void* data)                                 \
{                                                                                \
    return (int)RunFunctionFmt(my_Eof_fct_##A, "pp", decoder, data);      \
}
SUPER()
#undef GO
static void* findEofFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_Eof_fct_##A == (uintptr_t)fct) return my_Eof_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Eof_fct_##A == 0) {my_Eof_fct_##A = (uintptr_t)fct; return my_Eof_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FLAC Eof callback\n");
    return NULL;
}
// Write
#define GO(A)   \
static uintptr_t my_Write_fct_##A = 0;                                                              \
static int my_Write_##A(void* decoder, void* frame, void* buffer, void* data)                       \
{                                                                                                   \
    return (int)RunFunctionFmt(my_Write_fct_##A, "pppp", decoder, frame, buffer, data);       \
}
SUPER()
#undef GO
static void* findWriteFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_Write_fct_##A == (uintptr_t)fct) return my_Write_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Write_fct_##A == 0) {my_Write_fct_##A = (uintptr_t)fct; return my_Write_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FLAC Write callback\n");
    return NULL;
}
// Metadata
#define GO(A)   \
static uintptr_t my_Metadata_fct_##A = 0;                                                        \
static int my_Metadata_##A(void* decoder, void* metadata, void* data)                            \
{                                                                                                \
    return (int)RunFunctionFmt(my_Metadata_fct_##A, "ppp", decoder, metadata, data);      \
}
SUPER()
#undef GO
static void* findMetadataFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_Metadata_fct_##A == (uintptr_t)fct) return my_Metadata_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Metadata_fct_##A == 0) {my_Metadata_fct_##A = (uintptr_t)fct; return my_Metadata_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FLAC Metadata callback\n");
    return NULL;
}
// Error
#define GO(A)   \
static uintptr_t my_Error_fct_##A = 0;                                          \
static void my_Error_##A(void* decoder, int status, void* data)                 \
{                                                                               \
    RunFunctionFmt(my_Error_fct_##A, "pip", decoder, status, data);       \
}
SUPER()
#undef GO
static void* findErrorFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_Error_fct_##A == (uintptr_t)fct) return my_Error_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Error_fct_##A == 0) {my_Error_fct_##A = (uintptr_t)fct; return my_Error_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FLAC Error callback\n");
    return NULL;
}


#undef SUPER

EXPORT int my_FLAC__metadata_chain_read_with_callbacks(x64emu_t* emu, void* chain, void* handle,
    void* read_fnc, void* write_fnc, void* seek_fnc, void* tell_fnc, void* eof_fnc, void* close_fnc)
{
    flac_callbacks cbs = {0};
    cbs.read_func = findread_writeFct(read_fnc);
    cbs.write_func = findread_writeFct(write_fnc);
    cbs.seek_func = findseekFct(seek_fnc);
    cbs.tell_func = findtellFct(tell_fnc);
    cbs.eof_func = findclose_eofFct(eof_fnc);
    cbs.close_func = findclose_eofFct(close_fnc);
    int ret =  my->FLAC__metadata_chain_read_with_callbacks(chain, handle, cbs);
    return ret;
}

EXPORT int my_FLAC__stream_decoder_init_stream(x64emu_t* emu, void* decoder,
    void* read_fnc, void* seek_fnc, void* tell_fnc, void* length_fnc, void* eof_fnc,
    void* write_fnc, void* metadata_fnc, void* error_fnc, void* data)
{
    int ret =  my->FLAC__stream_decoder_init_stream(decoder,
        findReadFct(read_fnc), findSeekFct(seek_fnc), findTellFct(tell_fnc),
        findLengthFct(length_fnc), findEofFct(eof_fnc), findWriteFct(write_fnc),
        findMetadataFct(metadata_fnc), findErrorFct(error_fnc), data);
    return ret;
}

#include "wrappedlib_init.h"
