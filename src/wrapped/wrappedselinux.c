#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "callback.h"

const char* selinuxName = "libselinux.so.1";
#define LIBNAME selinux

typedef unsigned short security_class_t;

union selinux_callback {
    int (*func_log)(int type, const char* fmt, ...);
    int (*func_audit)(void* auditdata, security_class_t cls, char* msgbuf, size_t msgbufsize);
    int (*func_validate)(char** ctx);
    int (*func_setenforce)(int enforcing);
    int (*func_policyload)(int seqno);
};

#define SELINUX_CB_LOG        0
#define SELINUX_CB_AUDIT      1
#define SELINUX_CB_VALIDATE   2
#define SELINUX_CB_SETENFORCE 3
#define SELINUX_CB_POLICYLOAD 4

#include "generated/wrappedselinuxtypes.h"
#include "wrappercallback.h"

#define SUPER() \
GO(0)          \
GO(1)          \
GO(2)          \
GO(3)          \
GO(4)

// log callback: int (*func_log)(int type, const char* fmt, ...)
#define GO(A)   \
static uintptr_t my_log_fct_##A = 0;                               \
static int my_log_##A(int type, const char* fmt, ...)              \
{                                                                  \
    va_list args;                                                  \
    char buff[1024];                                               \
    va_start(args, fmt);                                           \
    vsnprintf(buff, sizeof(buff), fmt, args);                      \
    va_end(args);                                                  \
    return (int)RunFunction(my_log_fct_##A, 3, type, "%s", buff); \
}
SUPER()
#undef GO
static void* find_log_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_log_fct_##A == (uintptr_t)fct) return my_log_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_log_fct_##A == 0) {my_log_fct_##A = (uintptr_t)fct; return my_log_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for selinux log callback\n");
    return NULL;
}

// audit callback: int (*func_audit)(void*, security_class_t, char*, size_t)
#define GO(A)   \
static uintptr_t my_audit_fct_##A = 0;                                               \
static int my_audit_##A(void* auditdata, security_class_t cls, char* msgbuf, size_t msgbufsize) \
{                                                                                   \
    return (int)RunFunctionFmt(my_audit_fct_##A, "pWpL", auditdata, cls, msgbuf, msgbufsize); \
}
SUPER()
#undef GO
static void* find_audit_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_audit_fct_##A == (uintptr_t)fct) return my_audit_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_audit_fct_##A == 0) {my_audit_fct_##A = (uintptr_t)fct; return my_audit_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for selinux audit callback\n");
    return NULL;
}

// validate callback: int (*func_validate)(char**)
#define GO(A)   \
static uintptr_t my_validate_fct_##A = 0;            \
static int my_validate_##A(char** ctx)              \
{                                                   \
    return (int)RunFunctionFmt(my_validate_fct_##A, "p", ctx); \
}
SUPER()
#undef GO
static void* find_validate_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_validate_fct_##A == (uintptr_t)fct) return my_validate_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_validate_fct_##A == 0) {my_validate_fct_##A = (uintptr_t)fct; return my_validate_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for selinux validate callback\n");
    return NULL;
}

// setenforce callback: int (*func_setenforce)(int)
#define GO(A)   \
static uintptr_t my_setenforce_fct_##A = 0;          \
static int my_setenforce_##A(int enforcing)          \
{                                                   \
    return (int)RunFunctionFmt(my_setenforce_fct_##A, "i", enforcing); \
}
SUPER()
#undef GO
static void* find_setenforce_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_setenforce_fct_##A == (uintptr_t)fct) return my_setenforce_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_setenforce_fct_##A == 0) {my_setenforce_fct_##A = (uintptr_t)fct; return my_setenforce_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for selinux setenforce callback\n");
    return NULL;
}

// policyload callback: int (*func_policyload)(int)
#define GO(A)   \
static uintptr_t my_policyload_fct_##A = 0;          \
static int my_policyload_##A(int seqno)              \
{                                                   \
    return (int)RunFunctionFmt(my_policyload_fct_##A, "i", seqno); \
}
SUPER()
#undef GO
static void* find_policyload_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_policyload_fct_##A == (uintptr_t)fct) return my_policyload_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_policyload_fct_##A == 0) {my_policyload_fct_##A = (uintptr_t)fct; return my_policyload_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for selinux policyload callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my_selinux_set_callback(x64emu_t* emu, int type, union selinux_callback cb)
{
    (void)emu;
    void* cbp = NULL;
    switch(type) {
        case SELINUX_CB_LOG:
            cbp = find_log_Fct((void*)cb.func_log);
            break;
        case SELINUX_CB_AUDIT:
            cbp = find_audit_Fct((void*)cb.func_audit);
            break;
        case SELINUX_CB_VALIDATE:
            cbp = find_validate_Fct((void*)cb.func_validate);
            break;
        case SELINUX_CB_SETENFORCE:
            cbp = find_setenforce_Fct((void*)cb.func_setenforce);
            break;
        case SELINUX_CB_POLICYLOAD:
            cbp = find_policyload_Fct((void*)cb.func_policyload);
            break;
        default:
            break;
    }
    my->selinux_set_callback(type, cbp);
}

#include "wrappedlib_init.h"
