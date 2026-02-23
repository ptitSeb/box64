#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "callback.h"
#include "librarian.h"
#include "librarian/library_private.h"
#include "emu/x64emu_private.h"
#include "box64context.h"
#include "gltools.h"

#include "generated/wrappedsdl3defs.h"

const char* sdl3Name = "libSDL3.so.0";
#define LIBNAME sdl3

static void* my_glhandle = NULL;
// DL functions from wrappedlibdl.c
void* my_dlopen(x64emu_t* emu, void *filename, int flag);
int my_dlclose(x64emu_t* emu, void *handle);

typedef void  (*vFv_t)(void);
#define ADDED_FUNCTIONS()           \
    GO(SDL_Quit, vFv_t)
#include "generated/wrappedsdl3types.h"

#include "wrappercallback.h"

EXPORT void* my3_SDL_GL_GetProcAddress(x64emu_t* emu, void* name)
{
    const char* rname = (const char*)name;
    static int lib_checked = 0;
    if(!lib_checked) {
        lib_checked = 1;
        // check if libGL is loaded, load it if not
        if(!my_glhandle && !GetLibInternal(BOX64ENV(libgl)?BOX64ENV(libgl):"libGL.so.1"))
            my_glhandle = my_dlopen(emu, BOX64ENV(libgl)?BOX64ENV(libgl):"libGL.so.1", RTLD_LAZY|RTLD_GLOBAL);
    }
    return getGLProcAddress(emu, NULL, (glprocaddress_t)my->SDL_GL_GetProcAddress, rname);
}

#undef HAS_MY

#define ALTMY my3_

#define CUSTOM_INIT         \
    getMy(lib);

#define CUSTOM_FINI                                             \
    my->SDL_Quit();                                             \
    if(my_glhandle) my_dlclose(thread_get_emu(), my_glhandle);  \
    my_glhandle = NULL;                                         \
    freeMy();

#include "wrappedlib_init.h"
