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

void inplace_XcursorImage_shrink(void* a)
{
	if(!a) return;
	my_XcursorImage_t* src = a;
	my_XcursorImage_32_t* dst = a;

	dst->version = src->version;
	dst->size = src->size;
	dst->width = src->width;
	dst->height = src->height;
	dst->xhot = src->xhot;
	dst->yhot = src->yhot;
	dst->delay = src->delay;
	dst->pixels = to_ptrv(src->pixels);
}
void inplace_XcursorImage_enlarge(void* a)
{
	if(!a) return;
	my_XcursorImage_32_t* src = a;
	my_XcursorImage_t* dst = a;

	dst->pixels = from_ptrv(src->pixels);
	dst->delay = src->delay;
	dst->yhot = src->yhot;
	dst->xhot = src->xhot;
	dst->height = src->height;
	dst->width = src->width;
	dst->size = src->size;
	dst->version = src->version;
}

void inplace_XcursorImages_shrink(void* a)
{
	if(!a) return;
	my_XcursorImages_t* src = a;
	my_XcursorImages_32_t* dst = a;

	for(int i=0; i<src->nimage; ++i)
		inplace_XcursorCursors_shrink(src->images[i]);
	for(int i=0; i<src->nimage; ++i)
		((ptr_t*)src->images)[i] = to_ptrv(src->images[i]);
	dst->nimage = src->nimage;
	dst->images = to_ptrv(src->images);
	dst->name = to_ptrv(src->name);
}

void inplace_XcursorImages_enlarge(void* a)
{
	if(!a) return;
	my_XcursorImages_32_t* src = a;
	my_XcursorImages_t* dst = a;

	dst->name = from_ptrv(src->name);
	dst->images = from_ptrv(src->images);
	dst->nimage = src->nimage;
	for(int i=dst->nimage-1; i>=0; --i)
		dst->images[i] = from_ptrv(((ptr_t*)dst->images)[i]);
	for(int i=dst->nimage-1; i>=0; --i)
		inplace_XcursorCursors_enlarge(dst->images[i]);
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

EXPORT void* my32_XcursorImageCreate(x64emu_t* emu, int w, int h)
{
	void* ret = my->XcursorImageCreate(w, h);
	inplace_XcursorImage_shrink(ret);
	return ret;
}

EXPORT void my32_XcursorImageDestroy(x64emu_t* emu, void* image)
{
	inplace_XcursorImage_enlarge(image);
	my->XcursorImageDestroy(image);
}

EXPORT unsigned long my32_XcursorImageLoadCursor(x64emu_t* emu, void* dpy, void* image)
{
	inplace_XcursorImage_enlarge(image);
	unsigned long ret = my->XcursorImageLoadCursor(dpy, image);
	inplace_XcursorImage_shrink(image);
    return ret;
}

EXPORT void* my32_XcursorImagesCreate(x64emu_t* emu, int n)
{
	void* ret = my->XcursorImagesCreate(n);
	inplace_XcursorImages_shrink(ret);
	return ret;
}

EXPORT void my32_XcursorImagesDestroy(x64emu_t* emu, void* images)
{
	inplace_XcursorImages_enlarge(images);
	my->XcursorImagesDestroy(images);
}

EXPORT unsigned long my32_XcursorImagesLoadCursor(x64emu_t* emu, void* dpy, void* images)
{
	inplace_XcursorImages_enlarge(images);
	unsigned long ret = my->XcursorImagesLoadCursor(dpy, images);
	inplace_XcursorImages_shrink(images);
	return ret;
}

EXPORT void* my32_XcursorImagesLoadCursors(x64emu_t* emu, void* dpy, void* images)
{
	inplace_XcursorImages_enlarge(images);
	void* ret = my->XcursorImagesLoadCursors(dpy, images);
	inplace_XcursorImages_shrink(images);
	inplace_XcursorCursors_shrink(ret);
	return ret;
}

EXPORT void* my32_XcursorLibraryLoadImages(x64emu_t* emu, void* name, void* theme, int size)
{
	void* ret = my->XcursorLibraryLoadImages(name, theme, size);
	inplace_XcursorImages_shrink(ret);
	return ret;
}

EXPORT void* my32_XcursorShapeLoadImages(x64emu_t* emu, uint32_t shape, void* theme, int size)
{
	void* ret = my->XcursorShapeLoadImages(shape, theme, size);
	inplace_XcursorImages_shrink(ret);
	return ret;
}

#include "wrappedlib_init32.h"
