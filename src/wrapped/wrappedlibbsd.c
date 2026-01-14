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


const char* libbsdName = "libbsd.so.0";
#define ALTNAME "libbsd.so"
#define LIBNAME libbsd

#ifdef STATICBUILD
extern uint32_t arc4random(void);
extern void arc4random_addrandom(unsigned char *dat, int datlen);
extern void arc4random_buf(void *buf, size_t nbytes);
extern void arc4random_stir(void);
extern uint32_t arc4random_uniform(uint32_t upper_bound);
extern const char *getprogname(void);
extern void setprogname(const char *);
#endif

#ifndef STATICBUILD
#define PRE_INIT \
    if(1) { \
        lib->w.lib = dlopen(libbsdName, RTLD_LAZY | RTLD_GLOBAL); \
        if(!lib->w.lib) lib->w.lib = dlopen(ALTNAME, RTLD_LAZY | RTLD_GLOBAL); \
        if(!lib->w.lib) lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL); \
    } else
#endif

// define all standard library functions
#include "wrappedlib_init.h"

