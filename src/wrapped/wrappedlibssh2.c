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

const char* libssh2Name = "libssh2.so.1";
#define LIBNAME libssh2

#define NEEDED_LIBS "libgcrypt.so.20"

#include "wrappedlib_init.h"
