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

#ifdef ANDROID
	static const char* libxcbresName = "libxcb-res.so";
#else
	static const char* libxcbresName = "libxcb-res.so.0";
#endif

#define LIBNAME libxcbres

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

typedef my_xcb_cookie_t (*zFpup)(void*, uint32_t, void*);
typedef my_xcb_XXX_iterator_t (*ZFp_t)(void*);

#define ADDED_FUNCTIONS()       				\
GO(xcb_res_query_client_ids, zFpup)           	\
GO(xcb_res_query_client_ids_ids_iterator, ZFp_t)\

#include "generated/wrappedlibxcbrestypes32.h"

#include "wrappercallback32.h"

#define SUPER(F, P, ...)           \
    EXPORT void* my32_##F P        \
    {                              \
        (void)emu;                 \
        *ret = my->F(__VA_ARGS__); \
        return ret;                \
    }

SUPER(xcb_res_query_client_ids, (x64emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t count, void* specs), c, count, specs)

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

SUPER(xcb_res_query_client_ids_ids_iterator)

#undef SUPER

#include "wrappedlib_init32.h"
