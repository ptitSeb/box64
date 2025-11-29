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

const char* gstvideoName = "libgstvideo-1.0.so.0";
#define ALTNAME "libgstvideo-1.0.so"

#define LIBNAME gstvideo

typedef size_t  (*LFv_t)();

#define ADDED_FUNCTIONS()                           \
    GO(gst_video_decoder_get_type, LFv_t)           \
    GO(gst_video_encoder_get_type, LFv_t)           \
    GO(gst_video_sink_get_type, LFv_t)              \
    GO(gst_video_aggregator_get_type, LFv_t)        \
    GO(gst_video_aggregator_pad_get_type, LFv_t)    \
    GO(gst_video_filter_get_type, LFv_t)            \
    GO(gst_video_buffer_pool_get_type, LFv_t)       \

#include "generated/wrappedgstbasetypes.h"

#include "wrappercallback.h"

#define PRE_INIT \
    if (BOX64ENV(nogtk)) return -2;

#define CUSTOM_INIT \
    SetGstVideoDecoderID(my->gst_video_decoder_get_type());\
    SetGstVideoEncoderID(my->gst_video_encoder_get_type());\
    SetGstVideoSinkID(my->gst_video_sink_get_type());\
    SetGstVideoAggregatorID(my->gst_video_aggregator_get_type());\
    SetGstVideoAggregatorPadID(my->gst_video_aggregator_pad_get_type());\
    SetGstVideoFilterID(my->gst_video_filter_get_type());\
    SetGstVideoBufferPoolID(my->gst_video_buffer_pool_get_type());\

#define NEEDED_LIBS "libgstreamer-1.0.so.0"

#include "wrappedlib_init.h"
