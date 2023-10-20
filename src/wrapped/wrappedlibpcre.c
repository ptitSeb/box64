#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "callback.h"
#include "debug.h"

const char* libpcreName = "libpcre.so.3";
#define LIBNAME libpcre

typedef void  (*pcre_free_t)(void *);
EXPORT uintptr_t my_pcre_free;

void wrapped_pcre_free(void* p) {
    if(my_pcre_free)
        RunFunctionFmt(my_pcre_free, "p", p);
    else
        box_free(p);
}
EXPORT pcre_free_t pcre_free = wrapped_pcre_free;

#define CUSTOM_INIT \
    my_pcre_free = AddCheckBridge(lib->w.bridge, vFp, free, 0, "free");

#include "wrappedlib_init.h"

