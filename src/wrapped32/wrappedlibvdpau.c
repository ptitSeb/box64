#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "callback.h"
#include "librarian.h"
#include "box32context.h"
#include "emu/x64emu_private.h"
#include "myalign32.h"

static const char* libvdpauName = "libvdpau.so.1";
#define LIBNAME libvdpau


#define ADDED_FUNCTIONS()       \

#include "generated/wrappedlibvdpautypes32.h"

#include "wrappercallback32.h"

EXPORT uint32_t my32_vdp_device_create_x11(x64emu_t* emu, void* display, int screen, void* device, ptr_t* get_proc_address)
{
    return 1;   // VDP_STATUS_NO_IMPLEMENTATION
}

#include "wrappedlib_init32.h"
