#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE /* See feature_test_macros(7) */
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
#include "gtkclass.h"
#include "fileutils.h"

#ifdef ANDROID
const char* gstaudioName = "libgstaudio-1.0.so";
#else
const char* gstaudioName = "libgstaudio-1.0.so.0";
#endif

#define LIBNAME gstaudio

typedef size_t (*LFv_t)();

#define ADDED_FUNCTIONS() \
    GO(gst_audio_decoder_get_type, LFv_t)

#include "generated/wrappedgstaudiotypes.h"

#include "wrappercallback.h"

#define PRE_INIT     \
    if (box64_nogtk) \
        return -1;

#define CUSTOM_INIT \
    SetGstAudioDecoderID(my->gst_audio_decoder_get_type());

#ifdef ANDROID
#define NEEDED_LIBS "libgstreamer-1.0.so"
#else
#define NEEDED_LIBS "libgstreamer-1.0.so.0"
#endif

#include "wrappedlib_init.h"
