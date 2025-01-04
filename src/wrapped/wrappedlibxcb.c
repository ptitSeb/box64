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
#include "callback.h"
#include "librarian.h"
#include "box64context.h"
#include "emu/x64emu_private.h"
#include "myalign.h"

const char* libxcbName = "libxcb.so.1";
#define ALTNAME "libxcb.so"

#define LIBNAME libxcb

#define ADDED_FUNCTIONS()       \

#include "generated/wrappedlibxcbtypes.h"

#include "wrappercallback.h"

EXPORT void* my_xcb_connect(x64emu_t* emu, void* dispname, void* screen)
{
	return add_xcb_connection(my->xcb_connect(dispname, screen));
}

EXPORT void* my_xcb_connect_to_display_with_auth_info(x64emu_t* emu, void* dispname, void* auth, void* screen)
{
	return add_xcb_connection(my->xcb_connect_to_display_with_auth_info(dispname, auth, screen));
}

EXPORT void* my_xcb_connect_to_fd(x64emu_t* emu, int fd, void* auth)
{
	return add_xcb_connection(my->xcb_connect_to_fd(fd, auth));
}

EXPORT void my_xcb_disconnect(x64emu_t* emu, void* conn)
{
	my->xcb_disconnect(align_xcb_connection(conn));
	del_xcb_connection(conn);
}

#define NEEDED_LIBS "libXau.so.6", "libXdmcp.so.6"

#include "wrappedlib_init.h"
