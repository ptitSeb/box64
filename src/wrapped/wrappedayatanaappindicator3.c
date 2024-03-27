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

const char* ayatanaappindicator3Name = "libayatana-appindicator3.so.1";

#define LIBNAME ayatanaappindicator3

#include "wrappedlib_init.h"
