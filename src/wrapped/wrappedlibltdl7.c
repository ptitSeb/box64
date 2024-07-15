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

const char* libltdl7Name = "libltdl.so.7";
#define LIBNAME libltdl7

//#include "generated/wrappedlibltdl7types.h"

// Insert code here

#include "wrappedlib_init.h"