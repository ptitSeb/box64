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
#include "elfloader.h"
#include "box64context.h"
#include "x64tls.h"


#ifdef ANDROID
const char* libbsdName = "libbsd.so";
#else
const char* libbsdName = "libbsd.so.0";
#endif
#define LIBNAME libbsd

#ifdef STATICBUILD
void arc4random_addrandom(unsigned char *dat, int datlen);
void arc4random_stir(void);
const char *getprogname(void);
void setprogname(const char *);
#endif

#ifndef STATICBUILD
#define PRE_INIT\
    if(1)                                                           \
        lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);    \
    else
#endif

// define all standard library functions
#include "wrappedlib_init.h"

