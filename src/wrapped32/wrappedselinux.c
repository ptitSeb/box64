// SPDX-License-Identifier: MIT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "box32context.h"

static const char* selinuxName = "libselinux.so.1";
#define LIBNAME selinux

#include "wrappedlib_init32.h"

