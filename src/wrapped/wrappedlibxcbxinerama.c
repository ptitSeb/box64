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

#ifdef ANDROID
const char* libxcbxineramaName = "libxcb-xinerama.so";
#else
const char* libxcbxineramaName = "libxcb-xinerama.so.0";
#endif
#define LIBNAME libxcbxinerama

#include "wrappedlib_init.h"
