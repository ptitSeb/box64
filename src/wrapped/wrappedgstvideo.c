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

#ifdef ANDROID
    const char* gstvideoName = "libgstvideo-1.0.so";
#else
    const char* gstvideoName = "libgstvideo-1.0.so.0";
#endif

#define LIBNAME gstvideo

typedef size_t  (*LFv_t)();

#define ADDED_FUNCTIONS()                           \
    GO(gst_video_decoder_get_type, LFv_t)           \
    GO(gst_video_encoder_get_type, LFv_t)           \
    GO(gst_video_sink_get_type, LFv_t)              \
    GO(gst_video_aggregator_get_type, LFv_t)        \
    GO(gst_video_aggregator_pad_get_type, LFv_t)    \
    GO(gst_video_filter_get_type, LFv_t)            \

#include "generated/wrappedgstbasetypes.h"

#include "wrappercallback.h"

#define PRE_INIT    \
    if(box64_nogtk) \
        return -1;

#ifdef ANDROID
    #define CUSTOM_INIT \
        getMy(lib);     \
        SetGstVideoDecoderID(my->gst_video_decoder_get_type());\
        SetGstVideoEncoderID(my->gst_video_encoder_get_type());\
        SetGstVideoSinkID(my->gst_video_sink_get_type());\
        SetGstVideoAggregatorID(my->gst_video_aggregator_get_type());\
        SetGstVideoAggregatorPadID(my->gst_video_aggregator_pad_get_type());\
        SetGstVideoFilterID(my->gst_video_filter_get_type());\
        setNeededLibs(lib, 1, "libgstreamer-1.0.so");
#else
    #define CUSTOM_INIT \
        getMy(lib);     \
        SetGstVideoDecoderID(my->gst_video_decoder_get_type());\
        SetGstVideoEncoderID(my->gst_video_encoder_get_type());\
        SetGstVideoSinkID(my->gst_video_sink_get_type());\
        SetGstVideoAggregatorID(my->gst_video_aggregator_get_type());\
        SetGstVideoAggregatorPadID(my->gst_video_aggregator_pad_get_type());\
        SetGstVideoFilterID(my->gst_video_filter_get_type());\
        setNeededLibs(lib, 1, "libgstreamer-1.0.so.0");
#endif

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
