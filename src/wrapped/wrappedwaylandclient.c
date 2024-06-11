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


const char* waylandclientName = "libwayland-client.so.0";
#define LIBNAME waylandclient

static void AutoBridgeInterfaces(void* lib, bridge_t* bridge)
{
    // creates bridges for the various interfaces of this libs...
    void** p = NULL;
    p = dlsym(lib, "wl_buffer_interface");
    if(p) {
        AddAutomaticBridge(bridge, vFpp, *p, 0, "wl_buffer_interface_destroy");
    }
    p = dlsym(lib, "wl_compositor_interface");
    if(p) {
        AddAutomaticBridge(bridge, vFppu, p[0], 0, "wl_compositor_interface_create_surface");
        AddAutomaticBridge(bridge, vFppu, p[1], 0, "wl_compositor_interface_create_region");
    }
    p = dlsym(lib, "wl_display_interface");
    if(p) {
        AddAutomaticBridge(bridge, vFppu, p[0], 0, "wl_display_interface_sync");
        AddAutomaticBridge(bridge, vFppu, p[1], 0, "wl_display_interface_get_registry");
    }
    p = dlsym(lib, "wl_keyboard_interface");
    if(p) {
        AddAutomaticBridge(bridge, vFpp, *p, 0, "wl_keyboard_interface_release");
    }
    p = dlsym(lib, "wl_output_interface");
    if(p) {
        AddAutomaticBridge(bridge, vFpp, *p, 0, "wl_output_interface_release");
    }
    p = dlsym(lib, "wl_pointer_interface");
    if(p) {
        AddAutomaticBridge(bridge, vFppupii, p[0], 0, "wl_pointer_interface_set_cursor");
        AddAutomaticBridge(bridge, vFppu, p[1], 0, "wl_pointer_interface_release");
    }
}

#define CUSTOM_INIT         \
    AutoBridgeInterfaces(lib->w.lib, lib->w.bridge);

#include "wrappedlib_init.h"

