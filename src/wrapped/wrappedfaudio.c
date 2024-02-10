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

const char* faudioName = "libFAudio.so.0";
#define LIBNAME faudio

#include "generated/wrappedfaudiotypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// FAudioMalloc ...
#define GO(A)   \
static uintptr_t my_FAudioMalloc_fct_##A = 0;                                  \
static void* my_FAudioMalloc_##A(size_t a)                                     \
{                                                                              \
    return (void*)RunFunctionFmt(my_FAudioMalloc_fct_##A, "L", a);        \
}
SUPER()
#undef GO
static void* find_FAudioMalloc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_FAudioMalloc_fct_##A == (uintptr_t)fct) return my_FAudioMalloc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FAudioMalloc_fct_##A == 0) {my_FAudioMalloc_fct_##A = (uintptr_t)fct; return my_FAudioMalloc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FAudio FAudioMalloc callback\n");
    return NULL;
}
// FAudioFree ...
#define GO(A)   \
static uintptr_t my_FAudioFree_fct_##A = 0;                    \
static void my_FAudioFree_##A(void* a)                         \
{                                                              \
    RunFunctionFmt(my_FAudioFree_fct_##A, "p", a);        \
}
SUPER()
#undef GO
static void* find_FAudioFree_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_FAudioFree_fct_##A == (uintptr_t)fct) return my_FAudioFree_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FAudioFree_fct_##A == 0) {my_FAudioFree_fct_##A = (uintptr_t)fct; return my_FAudioFree_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FAudio FAudioFree callback\n");
    return NULL;
}
// FAudioRealloc ...
#define GO(A)   \
static uintptr_t my_FAudioRealloc_fct_##A = 0;                                      \
static void* my_FAudioRealloc_##A(void* a, size_t b)                                \
{                                                                                   \
    return (void*)RunFunctionFmt(my_FAudioRealloc_fct_##A, "pL", a, b);       \
}
SUPER()
#undef GO
static void* find_FAudioRealloc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_FAudioRealloc_fct_##A == (uintptr_t)fct) return my_FAudioRealloc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FAudioRealloc_fct_##A == 0) {my_FAudioRealloc_fct_##A = (uintptr_t)fct; return my_FAudioRealloc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FAudio FAudioRealloc callback\n");
    return NULL;
}
// OnCriticalErrorFunc ...
#define GO(A)   \
static uintptr_t my_OnCriticalErrorFunc_fct_##A = 0;                        \
static void my_OnCriticalErrorFunc_##A(void* a, uint32_t b)                 \
{                                                                           \
    RunFunctionFmt(my_OnCriticalErrorFunc_fct_##A, "pu", a, b);       \
}
SUPER()
#undef GO
static void* find_OnCriticalErrorFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_OnCriticalErrorFunc_fct_##A == (uintptr_t)fct) return my_OnCriticalErrorFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_OnCriticalErrorFunc_fct_##A == 0) {my_OnCriticalErrorFunc_fct_##A = (uintptr_t)fct; return my_OnCriticalErrorFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FAudio OnCriticalErrorFunc callback\n");
    return NULL;
}
// OnProcessingPassEndFunc ...
#define GO(A)   \
static uintptr_t my_OnProcessingPassEndFunc_fct_##A = 0;                    \
static void my_OnProcessingPassEndFunc_##A(void* a)                         \
{                                                                           \
    RunFunctionFmt(my_OnProcessingPassEndFunc_fct_##A, "p", a);       \
}
SUPER()
#undef GO
static void* find_OnProcessingPassEndFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_OnProcessingPassEndFunc_fct_##A == (uintptr_t)fct) return my_OnProcessingPassEndFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_OnProcessingPassEndFunc_fct_##A == 0) {my_OnProcessingPassEndFunc_fct_##A = (uintptr_t)fct; return my_OnProcessingPassEndFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FAudio OnProcessingPassEndFunc callback\n");
    return NULL;
}
// OnProcessingPassStartFunc ...
#define GO(A)   \
static uintptr_t my_OnProcessingPassStartFunc_fct_##A = 0;                    \
static void my_OnProcessingPassStartFunc_##A(void* a)                         \
{                                                                             \
    RunFunctionFmt(my_OnProcessingPassStartFunc_fct_##A, "p", a);         \
}
SUPER()
#undef GO
static void* find_OnProcessingPassStartFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_OnProcessingPassStartFunc_fct_##A == (uintptr_t)fct) return my_OnProcessingPassStartFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_OnProcessingPassStartFunc_fct_##A == 0) {my_OnProcessingPassStartFunc_fct_##A = (uintptr_t)fct; return my_OnProcessingPassStartFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FAudio OnProcessingPassStartFunc callback\n");
    return NULL;
}
typedef struct my_FAudioEngineCallback_s
{
	void* OnCriticalError;
	void* OnProcessingPassEnd;
	void* OnProcessingPassStart;
} my_FAudioEngineCallback_t;
// OnBufferEndFunc ...
#define GO(A)   \
static uintptr_t my_OnBufferEndFunc_fct_##A = 0;                        \
static void my_OnBufferEndFunc_##A(void* a, void* b)                    \
{                                                                       \
    RunFunctionFmt(my_OnBufferEndFunc_fct_##A, "pp", a, b);       \
}
SUPER()
#undef GO
static void* find_OnBufferEndFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_OnBufferEndFunc_fct_##A == (uintptr_t)fct) return my_OnBufferEndFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_OnBufferEndFunc_fct_##A == 0) {my_OnBufferEndFunc_fct_##A = (uintptr_t)fct; return my_OnBufferEndFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FAudio OnBufferEndFunc callback\n");
    return NULL;
}
// OnBufferStartFunc ...
#define GO(A)   \
static uintptr_t my_OnBufferStartFunc_fct_##A = 0;                        \
static void my_OnBufferStartFunc_##A(void* a, void* b)                    \
{                                                                         \
    RunFunctionFmt(my_OnBufferStartFunc_fct_##A, "pp", a, b);         \
}
SUPER()
#undef GO
static void* find_OnBufferStartFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_OnBufferStartFunc_fct_##A == (uintptr_t)fct) return my_OnBufferStartFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_OnBufferStartFunc_fct_##A == 0) {my_OnBufferStartFunc_fct_##A = (uintptr_t)fct; return my_OnBufferStartFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FAudio OnBufferStartFunc callback\n");
    return NULL;
}
// OnLoopEndFunc ...
#define GO(A)   \
static uintptr_t my_OnLoopEndFunc_fct_##A = 0;                        \
static void my_OnLoopEndFunc_##A(void* a, void* b)                    \
{                                                                     \
    RunFunctionFmt(my_OnLoopEndFunc_fct_##A, "pp", a, b);         \
}
SUPER()
#undef GO
static void* find_OnLoopEndFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_OnLoopEndFunc_fct_##A == (uintptr_t)fct) return my_OnLoopEndFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_OnLoopEndFunc_fct_##A == 0) {my_OnLoopEndFunc_fct_##A = (uintptr_t)fct; return my_OnLoopEndFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FAudio OnLoopEndFunc callback\n");
    return NULL;
}
// OnStreamEndFunc ...
#define GO(A)   \
static uintptr_t my_OnStreamEndFunc_fct_##A = 0;                    \
static void my_OnStreamEndFunc_##A(void* a)                         \
{                                                                   \
    RunFunctionFmt(my_OnStreamEndFunc_fct_##A, "p", a);       \
}
SUPER()
#undef GO
static void* find_OnStreamEndFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_OnStreamEndFunc_fct_##A == (uintptr_t)fct) return my_OnStreamEndFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_OnStreamEndFunc_fct_##A == 0) {my_OnStreamEndFunc_fct_##A = (uintptr_t)fct; return my_OnStreamEndFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FAudio OnStreamEndFunc callback\n");
    return NULL;
}
// OnVoiceErrorFunc ...
#define GO(A)   \
static uintptr_t my_OnVoiceErrorFunc_fct_##A = 0;                            \
static void my_OnVoiceErrorFunc_##A(void* a, void* b, uint32_t c)            \
{                                                                            \
    RunFunctionFmt(my_OnVoiceErrorFunc_fct_##A, "ppu", a, b, c);      \
}
SUPER()
#undef GO
static void* find_OnVoiceErrorFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_OnVoiceErrorFunc_fct_##A == (uintptr_t)fct) return my_OnVoiceErrorFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_OnVoiceErrorFunc_fct_##A == 0) {my_OnVoiceErrorFunc_fct_##A = (uintptr_t)fct; return my_OnVoiceErrorFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FAudio OnVoiceErrorFunc callback\n");
    return NULL;
}
// OnVoiceProcessingPassEndFunc ...
#define GO(A)   \
static uintptr_t my_OnVoiceProcessingPassEndFunc_fct_##A = 0;                    \
static void my_OnVoiceProcessingPassEndFunc_##A(void* a)                         \
{                                                                                \
    RunFunctionFmt(my_OnVoiceProcessingPassEndFunc_fct_##A, "p", a);      \
}
SUPER()
#undef GO
static void* find_OnVoiceProcessingPassEndFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_OnVoiceProcessingPassEndFunc_fct_##A == (uintptr_t)fct) return my_OnVoiceProcessingPassEndFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_OnVoiceProcessingPassEndFunc_fct_##A == 0) {my_OnVoiceProcessingPassEndFunc_fct_##A = (uintptr_t)fct; return my_OnVoiceProcessingPassEndFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FAudio OnVoiceProcessingPassEndFunc callback\n");
    return NULL;
}
// OnVoiceProcessingPassStartFunc ...
#define GO(A)   \
static uintptr_t my_OnVoiceProcessingPassStartFunc_fct_##A = 0;                        \
static void my_OnVoiceProcessingPassStartFunc_##A(void* a, uint32_t b)                 \
{                                                                                      \
    RunFunctionFmt(my_OnVoiceProcessingPassStartFunc_fct_##A, "pu", a, b);        \
}
SUPER()
#undef GO
static void* find_OnVoiceProcessingPassStartFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_OnVoiceProcessingPassStartFunc_fct_##A == (uintptr_t)fct) return my_OnVoiceProcessingPassStartFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_OnVoiceProcessingPassStartFunc_fct_##A == 0) {my_OnVoiceProcessingPassStartFunc_fct_##A = (uintptr_t)fct; return my_OnVoiceProcessingPassStartFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FAudio OnVoiceProcessingPassStartFunc callback\n");
    return NULL;
}
typedef struct my_FAudioVoiceCallback_s
{
	void* OnBufferEnd;
	void* OnBufferStart;
	void* OnLoopEnd;
	void* OnStreamEnd;
	void* OnVoiceError;
	void* OnVoiceProcessingPassEnd;
	void* OnVoiceProcessingPassStart;
} my_FAudioVoiceCallback_t;
// FAudioEngineCallEXT ...
#define GO(A)   \
static uintptr_t my_FAudioEngineCallEXT_fct_##A = 0;                        \
static void my_FAudioEngineCallEXT_##A(void* a, void* b)                    \
{                                                                           \
    RunFunctionFmt(my_FAudioEngineCallEXT_fct_##A, "pp", a, b);       \
}
SUPER()
#undef GO
static void* find_FAudioEngineCallEXT_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_FAudioEngineCallEXT_fct_##A == (uintptr_t)fct) return my_FAudioEngineCallEXT_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FAudioEngineCallEXT_fct_##A == 0) {my_FAudioEngineCallEXT_fct_##A = (uintptr_t)fct; return my_FAudioEngineCallEXT_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FAudio FAudioEngineCallEXT callback\n");
    return NULL;
}
// FAudioEngineProcedureEXT ...
#define GO(A)   \
static uintptr_t my_FAudioEngineProcedureEXT_fct_##A = 0;                                                              \
static void my_FAudioEngineProcedureEXT_##A(void* a, void* b, void* c, void* d)                                        \
{                                                                                                                      \
    RunFunctionFmt(my_FAudioEngineProcedureEXT_fct_##A, "pppp", find_FAudioEngineCallEXT_Fct(a), b, c, d);        \
}
SUPER()
#undef GO
static void* find_FAudioEngineProcedureEXT_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_FAudioEngineProcedureEXT_fct_##A == (uintptr_t)fct) return my_FAudioEngineProcedureEXT_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FAudioEngineProcedureEXT_fct_##A == 0) {my_FAudioEngineProcedureEXT_fct_##A = (uintptr_t)fct; return my_FAudioEngineProcedureEXT_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for FAudio FAudioEngineProcedureEXT callback\n");
    return NULL;
}
#undef SUPER

static void wrapFAudioEngineCallback(my_FAudioEngineCallback_t* src, my_FAudioEngineCallback_t* dst)
{
    dst->OnCriticalError = find_OnCriticalErrorFunc_Fct(src->OnCriticalError);
    dst->OnProcessingPassStart = find_OnProcessingPassStartFunc_Fct(src->OnProcessingPassStart);
    dst->OnProcessingPassEnd = find_OnProcessingPassEndFunc_Fct(src->OnProcessingPassEnd);
}

static void wrapFAudioVoiceCallback(my_FAudioVoiceCallback_t* src, my_FAudioVoiceCallback_t* dst)
{
    dst->OnBufferEnd = find_OnBufferEndFunc_Fct(src->OnBufferEnd);
    dst->OnBufferStart = find_OnBufferStartFunc_Fct(src->OnBufferStart);
    dst->OnLoopEnd = find_OnLoopEndFunc_Fct(src->OnLoopEnd);
    dst->OnStreamEnd = find_OnStreamEndFunc_Fct(src->OnStreamEnd);
    dst->OnVoiceError = find_OnVoiceErrorFunc_Fct(src->OnVoiceError);
    dst->OnVoiceProcessingPassEnd = find_OnVoiceProcessingPassEndFunc_Fct(src->OnVoiceProcessingPassEnd);
    dst->OnVoiceProcessingPassStart = find_OnVoiceProcessingPassStartFunc_Fct(src->OnVoiceProcessingPassStart);
}

EXPORT uint32_t my_FAudioCreateVolumeMeterWithCustomAllocatorEXT(x64emu_t* emu, void* ppApo, uint32_t flags, void* customMalloc, void* customFree, void* customRealloc) {
    return my->FAudioCreateVolumeMeterWithCustomAllocatorEXT(ppApo, flags, find_FAudioMalloc_Fct(customMalloc), find_FAudioFree_Fct(customFree), find_FAudioRealloc_Fct(customRealloc));
}

EXPORT uint32_t my_FAudioCreateReverbWithCustomAllocatorEXT(x64emu_t* emu, void* ppApo, uint32_t flags, void* customMalloc, void* customFree, void* customRealloc) {
    return my->FAudioCreateReverbWithCustomAllocatorEXT(ppApo, flags, find_FAudioMalloc_Fct(customMalloc), find_FAudioFree_Fct(customFree), find_FAudioRealloc_Fct(customRealloc));
}

EXPORT uint32_t my_FAudioCreateReverb9WithCustomAllocatorEXT(x64emu_t* emu, void* ppApo, uint32_t flags, void* customMalloc, void* customFree, void* customRealloc) {
    return my->FAudioCreateReverb9WithCustomAllocatorEXT(ppApo, flags, find_FAudioMalloc_Fct(customMalloc), find_FAudioFree_Fct(customFree), find_FAudioRealloc_Fct(customRealloc));
}

EXPORT uint32_t my_FAudio_RegisterForCallbacks(x64emu_t* emu, void* audio, my_FAudioEngineCallback_t* p) {
    my_FAudioEngineCallback_t cb = {0};
    wrapFAudioEngineCallback(p, &cb);
    return my->FAudio_RegisterForCallbacks(audio, &cb);
}
EXPORT void my_FAudio_UnregisterForCallbacks(x64emu_t* emu, void* audio, my_FAudioEngineCallback_t* p) {
    my_FAudioEngineCallback_t cb = {0};
    wrapFAudioEngineCallback(p, &cb);
    my->FAudio_UnregisterForCallbacks(audio, &cb);
}

EXPORT uint32_t my_FAudio_CreateSourceVoice(x64emu_t* emu, void* audio, void* voices, void* format, uint32_t flags, float ratio, my_FAudioVoiceCallback_t* p, void* send, void* effect) {
    my_FAudioVoiceCallback_t cb = {0};
    if(p)
        wrapFAudioVoiceCallback(p, &cb);
    return my->FAudio_CreateSourceVoice(audio, voices, format, flags, ratio, p?(&cb):NULL, send, effect);
}

EXPORT uint32_t my_FAudioCreateWithCustomAllocatorEXT(x64emu_t* emu, void* pp, uint32_t flags, uint32_t proc, void* customMalloc, void* customFree, void* customRealloc) {
    return my->FAudioCreateWithCustomAllocatorEXT(pp, flags, proc, find_FAudioMalloc_Fct(customMalloc), find_FAudioFree_Fct(customFree), find_FAudioRealloc_Fct(customRealloc));
}

EXPORT uint32_t my_FAudioCOMConstructWithCustomAllocatorEXT(x64emu_t* emu, void* pp, uint8_t ver,void* customMalloc, void* customFree, void* customRealloc) {
    return my->FAudioCOMConstructWithCustomAllocatorEXT(pp, ver, find_FAudioMalloc_Fct(customMalloc), find_FAudioFree_Fct(customFree), find_FAudioRealloc_Fct(customRealloc));
}

EXPORT void my_FAudio_SetEngineProcedureEXT(x64emu_t* emu, void* audio, void* p, void* user) {
    my->FAudio_SetEngineProcedureEXT(audio, find_FAudioEngineProcedureEXT_Fct(p), user);
}

EXPORT uint32_t my_FAPOFX_CreateFXWithCustomAllocatorEXT(x64emu_t* emu, void* clsid, void* p, void* data, uint32_t size, void* customMalloc, void* customFree, void* customRealloc) {
    return my->FAPOFX_CreateFXWithCustomAllocatorEXT(clsid, p, data, size, find_FAudioMalloc_Fct(customMalloc), find_FAudioFree_Fct(customFree), find_FAudioRealloc_Fct(customRealloc));
}

#include "wrappedlib_init.h"
