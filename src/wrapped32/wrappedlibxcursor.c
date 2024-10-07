#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box32context.h"
#include "emu/x64emu_private.h"

#ifdef ANDROID
	static const char* libxcursorName = "libXcursor.so";
#else
	static const char* libxcursorName = "libXcursor.so.1";
#endif

#define LIBNAME libxcursor

#ifdef ANDROID
#define NEEDED_LIBS "libX11.so", "libXfixes.so", "libXrender.so"
#else
#define NEEDED_LIBS "libX11.so.6", "libXfixes.so.3", "libXrender.so.1"
#endif

#include "libtools/my_x11_defs.h"
#include "libtools/my_x11_defs_32.h"

#include "generated/wrappedlibxcursortypes32.h"

#include "wrappercallback32.h"

void* getDisplay(void*);
void* FindDisplay(void*);

void inplace_XcursorCursors_shrink(void* a)
{
	if(!a) return;
	my_XcursorCursors_t* src = a;
	my_XcursorCursors_32_t* dst = a;

	dst->dpy = to_ptrv(getDisplay(src->dpy));
	dst->ref = src->ref;
	dst->ncursor = src->ncursor;
	dst->cursors = to_ptrv(src->cursors);
}
void inplace_XcursorCursors_enlarge(void* a)
{
	if(!a) return;
	my_XcursorCursors_32_t* src = a;
	my_XcursorCursors_t* dst = a;

	dst->cursors = from_ptrv(src->cursors);
	dst->ncursor = src->ncursor;
	dst->ref = src->ref;
	dst->dpy = FindDisplay(from_ptrv(src->dpy));
}

EXPORT void* my32_XcursorCursorsCreate(x64emu_t* emu, void* dpy, int n)
{
	void* ret = my->XcursorCursorsCreate(dpy, n);
	inplace_XcursorCursors_shrink(ret);
	return ret;
}

EXPORT void my32_XcursorCursorsDestroy(x64emu_t* emu, void* a)
{
	inplace_XcursorCursors_enlarge(a);
	my->XcursorCursorsDestroy(a);
}

#include "wrappedlib_init32.h"

