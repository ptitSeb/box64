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

const char* libxxf86vmName = "libXxf86vm.so.1";
#define LIBNAME libxxf86vm

#ifdef PANDORA
typedef struct my_XF86VidModeGamma_s {
    float red;
    float green;
    float blue;
} my_XF86VidModeGamma_t;

static my_XF86VidModeGamma_t current_gamma = {0};

EXPORT int my_XF86VidModeGetGamma(void* display, int screen, my_XF86VidModeGamma_t* gamma)
{
    memcpy(gamma, &current_gamma, sizeof(current_gamma));
    return 1;
}

EXPORT int my_XF86VidModeSetGamma(void* display, int screen, my_XF86VidModeGamma_t* gamma)
{
    memcpy(&current_gamma, gamma, sizeof(current_gamma));
    float mean = (current_gamma.red+current_gamma.green+current_gamma.blue)/3;
    char buf[50];
    if(mean==0.0f)
        sprintf(buf, "sudo /usr/pandora/scripts/op_gamma.sh 0");
    else
        sprintf(buf, "sudo /usr/pandora/scripts/op_gamma.sh %.2f", mean);
    system(buf);
    return 1;
}
#endif

#define CUSTOM_INIT \
    lib->priv.w.needed = 2; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libX11.so.6"); \
    lib->priv.w.neededlibs[1] = strdup("libXext.so.6");

#include "wrappedlib_init.h"
