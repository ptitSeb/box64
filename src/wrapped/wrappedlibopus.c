// SPDX-License-Identifier: MIT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#define _GNU_SOURCE
#include <dlfcn.h>

#include "wrappedlibs.h"
#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "myalign.h"

const char* libopusName = "libopus.so.0";
#define ALTNAME "libopus.so"
#define LIBNAME libopus

#include "generated/wrappedlibopustypes.h"
#include "wrappercallback.h"

#define OPUS_RESET_STATE 4028
#define OPUS_SET_DNN_BLOB_REQUEST 4052
#define OPUS_PROJECTION_GET_DEMIXING_MATRIX_REQUEST 6005
#define OPUS_MULTISTREAM_GET_ENCODER_STATE_REQUEST 5120
#define OPUS_MULTISTREAM_GET_DECODER_STATE_REQUEST 5122

static int my_opus_ctl(x64emu_t* emu, iFpiV_t fn, void* st, int request,
                       void* stack_args)
{
    CREATE_VALIST_FROM_VAARG(stack_args, emu->scratch, 2);

    if (request == OPUS_RESET_STATE)
        return fn(st, request);
    if (request == OPUS_SET_DNN_BLOB_REQUEST ||
        request == OPUS_PROJECTION_GET_DEMIXING_MATRIX_REQUEST) {
        void *data = va_arg(VARARGS, void*);
        int len = va_arg(VARARGS, int);
        return fn(st, request, data, len);
    }
    if (request == OPUS_MULTISTREAM_GET_ENCODER_STATE_REQUEST ||
        request == OPUS_MULTISTREAM_GET_DECODER_STATE_REQUEST) {
        int stream_id = va_arg(VARARGS, int);
        void *state = va_arg(VARARGS, void*);
        return fn(st, request, stream_id, state);
    }
    // GET request.
    if (request & 1)
        return fn(st, request, va_arg(VARARGS, void*));
    // Set request.
    return fn(st, request, va_arg(VARARGS, int));
}

EXPORT int my_opus_decoder_ctl(x64emu_t* emu, void* st, int request, void* stack_args)
{
    return my_opus_ctl(emu, my->opus_decoder_ctl, st, request, stack_args);
}

EXPORT int my_opus_encoder_ctl(x64emu_t* emu, void* st, int request, void* stack_args)
{
    return my_opus_ctl(emu, my->opus_encoder_ctl, st, request, stack_args);
}

EXPORT int my_opus_multistream_decoder_ctl(x64emu_t* emu, void* st, int request, void* stack_args)
{
    return my_opus_ctl(emu, my->opus_multistream_decoder_ctl, st, request, stack_args);
}

EXPORT int my_opus_multistream_encoder_ctl(x64emu_t* emu, void* st, int request, void* stack_args)
{
    return my_opus_ctl(emu, my->opus_multistream_encoder_ctl, st, request, stack_args);
}

EXPORT int my_opus_projection_decoder_ctl(x64emu_t* emu, void* st, int request, void* stack_args)
{
    return my_opus_ctl(emu, my->opus_projection_decoder_ctl, st, request, stack_args);
}

EXPORT int my_opus_projection_encoder_ctl(x64emu_t* emu, void* st, int request, void* stack_args)
{
    return my_opus_ctl(emu, my->opus_projection_encoder_ctl, st, request, stack_args);
}

#include "wrappedlib_init.h"
