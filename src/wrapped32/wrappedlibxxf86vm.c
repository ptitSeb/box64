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
#include "emu/x64emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box32context.h"
#include "emu/x64emu_private.h"

#ifdef ANDROID
    static const char* libxxf86vmName = "libXxf86vm.so";
#else
    static const char* libxxf86vmName = "libXxf86vm.so.1";
#endif

#define LIBNAME libxxf86vm

#ifdef ANDROID
#define NEEDED_LIBS "libX11.so", "libXext.so"
#else
#define NEEDED_LIBS "libX11.so.6", "libXext.so.6"
#endif


#include "libtools/my_x11_defs.h"
#include "libtools/my_x11_defs_32.h"

#include "generated/wrappedlibxxf86vmtypes32.h"

#include "wrappercallback32.h"

EXPORT int my32_XF86VidModeGetAllModeLines(x64emu_t* emu, void* dpy, int screen, int* num, ptr_t* infos)
{
    my_XF86VidModeModeInfo_t** infos_l;
    int ret = my->XF86VidModeGetAllModeLines(dpy, screen, num, &infos_l);
    if(*num>0) {
        *infos = to_ptrv(infos_l);
        ptr_t* small = from_ptrv(*infos);
        // shrink the array, the structure is fine
        for(int i=0; i<*num; ++i)
            small[i] = to_ptrv(infos_l[i]);
        // no need to mark the end
        //small[*num] = 0;
        if(*num==1)
            small[1] = small[0];    // duplicate the mode if only 1 exit (for Hercules6)
    }
    return ret;
}

#include "wrappedlib_init32.h"