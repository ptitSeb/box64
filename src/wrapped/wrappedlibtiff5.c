#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"

const char* libtiff5Name = "libtiff.so.5";
#define LIBNAME libtiff5

#include "generated/wrappedlibtiff5types.h"

// Insert code here

#include "wrappedlib_init.h"