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
#include "emu/x64emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box64context.h"
#include "emu/x64emu_private.h"
#include "myalign.h"

const char* pamName = "libpam.so.0";
#define LIBNAME pam

#define ADDED_FUNCTIONS()

#include "generated/wrappedpamtypes.h"

#include "wrappercallback.h"

typedef struct my_pam_conv_s {
    int (*conv)(int num_msg, const void** msg, void** resp, void* appdata_ptr);
    void* appdata_ptr;
} my_pam_conv_t;

#define SUPER() \
    GO(0)       \
    GO(1)       \
    GO(2)       \
    GO(3)       \
    GO(4)

#define GO(A)                                                                                        \
    static uintptr_t my_pam_conv_conv_##A = 0;                                                       \
    static int my_pam_conv_convfct##A(int num_msg, const void** msg, void** resp, void* appdata_ptr) \
    {                                                                                                \
        return (int)RunFunctionFmt(my_pam_conv_conv_##A, "ippp", num_msg, msg, resp, appdata_ptr);  \
    }
SUPER()
#undef GO
static void* find_pam_conv_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
#define GO(A) \
    if (my_pam_conv_conv_##A == (uintptr_t)fct) return my_pam_conv_convfct##A;
    SUPER()
#undef GO
#define GO(A)                                  \
    if (my_pam_conv_conv_##A == 0) {           \
        my_pam_conv_conv_##A = (uintptr_t)fct; \
        return my_pam_conv_convfct##A;         \
    }
    SUPER()
#undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pam conv callback\n");
    return NULL;
}

EXPORT int my_pam_start(x64emu_t* emu, void* service_name, void* user, my_pam_conv_t* pam_conversation, void** pamh)
{
    (void)emu;
    void* conv = NULL;
    if (pam_conversation)
        conv = find_pam_conv_Fct(pam_conversation->conv);
    return my->pam_start(service_name, user, conv ? &(my_pam_conv_t) { conv, pam_conversation->appdata_ptr } : NULL, pamh);
}

#include "wrappedlib_init.h"
