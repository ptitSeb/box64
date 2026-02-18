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
#include "gtkclass.h"
#include "fileutils.h"

const char* gstaudioName = "libgstaudio-1.0.so.0";
#define ALTNAME "libgstaudio-1.0.so"

#define LIBNAME gstaudio

typedef size_t  (*LFv_t)();

#define ADDED_FUNCTIONS()                   \
    GO(gst_audio_decoder_get_type, LFv_t)   \
    GO(gst_audio_filter_get_type, LFv_t)    \
    GO(gst_audio_encoder_get_type, LFv_t)   \

#include "generated/wrappedgstaudiotypes.h"

#include "wrappercallback.h"

#define PRE_INIT \
    if (BOX64ENV(nogtk)) return -2;

#define CUSTOM_INIT \
    SetGstAudioDecoderID(my->gst_audio_decoder_get_type()); \
    SetGstAudioFilterID(my->gst_audio_filter_get_type());   \
    SetGstAudioEncoderID(my->gst_audio_encoder_get_type());

#define NEEDED_LIBS "libgstreamer-1.0.so.0"

#include "wrappedlib_init.h"
