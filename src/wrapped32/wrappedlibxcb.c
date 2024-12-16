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
#include "callback.h"
#include "librarian.h"
#include "box32context.h"
#include "emu/x64emu_private.h"
#include "myalign32.h"
#include "converter32.h"

static const char* libxcbName = "libxcb.so.1";
#define ALTNAME "libxcb.so"

#define LIBNAME libxcb

typedef struct my_xcb_XXX_iterator_s {
    void*             data;
    int               rem;
    int               index;
} my_xcb_XXX_iterator_t;

typedef struct my_xcb_XXX_iterator_32_s {
    ptr_t             data;
    int               rem;
    int               index;
} my_xcb_XXX_iterator_32_t;

typedef struct my_xcb_cookie_s {
    uint32_t        data;
} my_xcb_cookie_t;

typedef my_xcb_cookie_t (*zFppp_t)(void*, void*, void*);
typedef my_xcb_cookie_t (*zFpCWp_t)(void*, uint8_t, uint16_t, void*);
typedef my_xcb_cookie_t (*zFpCuuuuu_t)(void*, uint8_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef my_xcb_cookie_t (*zFpCuuuCup_t)(void*, uint8_t, uint32_t, uint32_t, uint32_t, uint8_t, uint32_t, void*);
typedef my_xcb_XXX_iterator_t (*ZFp_t)(void*);

#define ADDED_FUNCTIONS()       				\
GO(xcb_change_property, zFpCuuuCup_t)           \
GO(xcb_delete_property, zFppp_t)                \
GO(xcb_get_property, zFpCuuuuu_t)               \
GO(xcb_intern_atom, zFpCWp_t)                   \
GO(xcb_setup_roots_iterator, ZFp_t)             \

#include "generated/wrappedlibxcbtypes32.h"

#include "wrappercallback32.h"

EXPORT void* my32_xcb_connect(x64emu_t* emu, void* dispname, void* screen)
{
	return add_xcb_connection32(my->xcb_connect(dispname, screen));
}

EXPORT void my32_xcb_disconnect(x64emu_t* emu, void* conn)
{
	my->xcb_disconnect(align_xcb_connection32(conn));
	del_xcb_connection32(conn);
}

#define SUPER(F, P, ...)           \
    EXPORT void* my32_##F P        \
    {                              \
        (void)emu;                 \
        *ret = my->F(__VA_ARGS__); \
        return ret;                \
    }

SUPER(xcb_change_property, (x64emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t mode, uint32_t w, uint32_t prop, uint32_t type, uint8_t f, uint32_t len, void* data), c, mode, w, prop, type, f, len, data)
SUPER(xcb_delete_property, (x64emu_t* emu, my_xcb_cookie_t* ret, void* c, void* w, void* p), c, w, p)
SUPER(xcb_get_property, (x64emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t _del, uint32_t w, uint32_t p, uint32_t type, uint32_t off, uint32_t len), c, _del, w, p, type, off, len)
SUPER(xcb_intern_atom, (x64emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t only, uint16_t len, void* name), c, only, len, name)

#undef SUPER

#define SUPER(F)           		   												\
    EXPORT void* my32_##F(x64emu_t* emu, my_xcb_XXX_iterator_32_t* ret, void* R)\
    {                              												\
        (void)emu;       														\
		my_xcb_XXX_iterator_t ret_l = my->F(R);									\
        ret->data = to_ptrv(ret_l.data); 										\
        ret->rem = ret_l.rem; 													\
        ret->index = ret_l.index; 												\
        return ret;      														\
    }

SUPER(xcb_setup_roots_iterator)

#undef SUPER

#if 0
#ifdef ANDROID
#define NEEDED_LIBS "libXau.so", "libXdmcp.so"
#else
#define NEEDED_LIBS "libXau.so.6", "libXdmcp.so.6"
#endif
#endif

#include "wrappedlib_init32.h"
