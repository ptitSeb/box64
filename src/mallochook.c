#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "box64context.h"
#include "debug.h"

#ifdef ANDROID
void*(*__libc_malloc)(size_t) = NULL;
void*(*__libc_realloc)(size_t, void*) = NULL;
void*(*__libc_calloc)(size_t, size_t) = NULL;
void (*__libc_free*)(void*) = NULL;

void init_malloc_hook() {
    __libc_malloc = dlsym(RTLD_NEXT, "malloc");
    __libc_realloc = dlsym(RTLD_NEXT, "realloc");
    __libc_calloc = dlsym(RTLD_NEXT, "realloc");
    __libc_free = dlsym(RTLD_NEXT, "free");
}
#endif

char* box_strdup(const char* s) {
    char* ret = box_calloc(1, strlen(s)+1);
    memcpy(ret, s, strlen(s));
    return ret;
}