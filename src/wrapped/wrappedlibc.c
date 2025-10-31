#define _LARGEFILE_SOURCE 1
#define _FILE_OFFSET_BITS 64
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <wchar.h>
#include <dlfcn.h>
#include <signal.h>
#include <errno.h>
#include <err.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <glob.h>
#include <ctype.h>
#include <dirent.h>
#include <search.h>
#include <sys/types.h>
#include <poll.h>
#include <sys/epoll.h>
#include <ftw.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <setjmp.h>
#include <sys/vfs.h>
#include <spawn.h>
#include <fts.h>
#include <syslog.h>
#include <malloc.h>
#include <getopt.h>
#include <sys/resource.h>
#include <sys/prctl.h>
#include <sys/ptrace.h>
#include <error.h>
#undef LOG_INFO
#undef LOG_DEBUG

#include "wrappedlibs.h"

#include "os.h"
#include "box64stack.h"
#include "x64emu.h"
#include "box64cpu.h"
#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "callback.h"
#include "librarian.h"
#include "librarian/library_private.h"
#include "emu/x64emu_private.h"
#include "box64context.h"
#include "myalign.h"
#include "signals.h"
#include "fileutils.h"
#include "auxval.h"
#include "elfloader.h"
#include "bridge.h"
#include "globalsymbols.h"
#include "env.h"
#include "wine_tools.h"
#include "pe_tools.h"
#include "cleanup.h"
#ifndef LOG_INFO
#define LOG_INFO 1
#endif
#ifndef LOG_DEBUG
#define LOG_DEBUG 2
#endif


#define LIBNAME libc
const char* libcName = "libc.so.6";
#define ALTNAME "libc.so"

typedef int (*iFi_t)(int);
typedef int (*iFp_t)(void*);
typedef int (*iFL_t)(unsigned long);
typedef void (*vFpp_t)(void*, void*);
typedef void (*vFipp_t)(int32_t, void*, void*);
typedef int32_t (*iFpi_t)(void*, int32_t);
typedef int32_t (*iFpp_t)(void*, void*);
typedef int32_t (*iFpL_t)(void*, size_t);
typedef int32_t (*iFiip_t)(int32_t, int32_t, void*);
typedef int32_t (*iFipp_t)(int32_t, void*, void*);
typedef int32_t (*iFppi_t)(void*, void*, int32_t);
typedef int32_t (*iFpup_t)(void*, uint32_t, void*);
typedef int32_t (*iFpuu_t)(void*, uint32_t, uint32_t);
typedef int32_t (*iFiiII_t)(int, int, int64_t, int64_t);
typedef int32_t (*iFiiiV_t)(int, int, int, ...);
typedef int32_t (*iFippi_t)(int32_t, void*, void*, int32_t);
typedef int32_t (*iFpppp_t)(void*, void*, void*, void*);
typedef int32_t (*iFpipp_t)(void*, int32_t, void*, void*);
typedef int32_t (*iFppii_t)(void*, void*, int32_t, int32_t);
typedef int32_t (*iFipiup_t)(int, void*, int, uint32_t, void*);
typedef int32_t (*iFipiI_t)(int32_t, void*, int32_t, int64_t);
typedef int32_t (*iFipuu_t)(int32_t, void*, uint32_t, uint32_t);
typedef int32_t (*iFipuup_t)(int32_t, void*, uint32_t, uint32_t, void*);
typedef int32_t (*iFiiV_t)(int32_t, int32_t, ...);
typedef void* (*pFp_t)(void*);
typedef void* (*pFpip_t)(void*, int, void*);

#define ADDED_FUNCTIONS() \

#include "generated/wrappedlibctypes.h"

#include "wrappercallback.h"

static int regs_abi[] = {_DI, _SI, _DX, _CX, _R8, _R9};
void* getVargN(x64emu_t *emu, int n)
{
    if(n<6)
        return (void*)emu->regs[regs_abi[n]].q[0];
    return ((void**)R_RSP)[1+n-6];
}

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)   \
GO(5)   \
GO(6)   \
GO(7)   \
GO(8)   \
GO(9)   \
GO(10)  \
GO(11)  \
GO(12)  \
GO(13)  \
GO(14)  \
GO(15)

// compare
#define GO(A)   \
static uintptr_t my_compare_fct_##A = 0;                                    \
static int my_compare_##A(void* a, void* b)                                 \
{                                                                           \
    return (int)RunFunctionFmt(my_compare_fct_##A, "pp", a, b);       \
}
SUPER()
#undef GO
static void* findcompareFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_compare_fct_##A == (uintptr_t)fct) return my_compare_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_compare_fct_##A == 0) {my_compare_fct_##A = (uintptr_t)fct; return my_compare_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc compare callback\n");
    return NULL;
}
// action
#define GO(A)   \
static uintptr_t my_action_fct_##A = 0;                 \
static void my_action_##A(void* a, uint32_t b, int c)   \
{                                                       \
    RunFunctionFmt(my_action_fct_##A, "pui", a, b, c);  \
}
SUPER()
#undef GO
static void* findactionFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_action_fct_##A == (uintptr_t)fct) return my_action_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_action_fct_##A == 0) {my_action_fct_##A = (uintptr_t)fct; return my_action_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc action callback\n");
    return NULL;
}

// ftw64
#define GO(A)   \
static uintptr_t my_ftw64_fct_##A = 0;                      \
static int my_ftw64_##A(void* fpath, void* sb, int flag)    \
{                                                           \
    struct x64_stat64 x64st;                                \
    UnalignStat64(sb, &x64st);                              \
    return (int)RunFunctionFmt(my_ftw64_fct_##A, "ppi", fpath, &x64st, flag);         \
}
SUPER()
#undef GO
static void* findftw64Fct(void* fct)
{
    if(!fct) return NULL;
    #define GO(A) if(my_ftw64_fct_##A == (uintptr_t)fct) return my_ftw64_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ftw64_fct_##A == 0) {my_ftw64_fct_##A = (uintptr_t)fct; return my_ftw64_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc ftw64 callback\n");
    return NULL;
}

// nftw64
#define GO(A)   \
static uintptr_t my_nftw64_fct_##A = 0;                                     \
static int my_nftw64_##A(void* fpath, void* sb, int flag, void* ftwbuff)    \
{                                                                           \
    struct x64_stat64 x64st;                                                \
    UnalignStat64(sb, &x64st);                                              \
    return (int)RunFunctionFmt(my_nftw64_fct_##A, "ppip", fpath, &x64st, flag, ftwbuff);          \
}
SUPER()
#undef GO
static void* findnftw64Fct(void* fct)
{
    if(!fct) return NULL;
    #define GO(A) if(my_nftw64_fct_##A == (uintptr_t)fct) return my_nftw64_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_nftw64_fct_##A == 0) {my_nftw64_fct_##A = (uintptr_t)fct; return my_nftw64_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc nftw64 callback\n");
    return NULL;
}
// globerr
#define GO(A)   \
static uintptr_t my_globerr_fct_##A = 0;                                                \
static int my_globerr_##A(void* epath, int eerrno)                                      \
{                                                                                       \
    return (int)RunFunctionFmt(my_globerr_fct_##A, "pi", epath, eerrno);          \
}
SUPER()
#undef GO
static void* findgloberrFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_globerr_fct_##A == (uintptr_t)fct) return my_globerr_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_globerr_fct_##A == 0) {my_globerr_fct_##A = (uintptr_t)fct; return my_globerr_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc globerr callback\n");
    return NULL;
}
// free
#define GO(A)   \
static uintptr_t my_free_fct_##A = 0;                       \
static void my_free_##A(void* p)                            \
{                                                           \
    RunFunctionFmt(my_free_fct_##A, "p", p);          \
}
SUPER()
#undef GO
static void* findfreeFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_free_fct_##A == (uintptr_t)fct) return my_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_fct_##A == 0) {my_free_fct_##A = (uintptr_t)fct; return my_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc free callback\n");
    return NULL;
}

#if 0
#undef dirent
// filter_dir
#define GO(A)   \
static uintptr_t my_filter_dir_fct_##A = 0;                                 \
static int my_filter_dir_##A(const struct dirent* a)                        \
{                                                                           \
    return (int)RunFunctionFmt(my_filter_dir_fct_##A, "p", a);        \
}
SUPER()
#undef GO
static void* findfilter_dirFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_filter_dir_fct_##A == (uintptr_t)fct) return my_filter_dir_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_filter_dir_fct_##A == 0) {my_filter_dir_fct_##A = (uintptr_t)fct; return my_filter_dir_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc filter_dir callback\n");
    return NULL;
}
// compare_dir
#define GO(A)   \
static uintptr_t my_compare_dir_fct_##A = 0;                                    \
static int my_compare_dir_##A(const struct dirent* a, const struct dirent* b)   \
{                                                                               \
    return (int)RunFunctionFmt(my_compare_dir_fct_##A, "pp", a, b);       \
}
SUPER()
#undef GO
static void* findcompare_dirFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_compare_dir_fct_##A == (uintptr_t)fct) return my_compare_dir_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_compare_dir_fct_##A == 0) {my_compare_dir_fct_##A = (uintptr_t)fct; return my_compare_dir_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc compare_dir callback\n");
    return NULL;
}
#endif

// filter64
#define GO(A)   \
static uintptr_t my_filter64_fct_##A = 0;                                   \
static int my_filter64_##A(const struct dirent64* a)                        \
{                                                                           \
    return (int)RunFunctionFmt(my_filter64_fct_##A, "p", a);          \
}
SUPER()
#undef GO
static void* findfilter64Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_filter64_fct_##A == (uintptr_t)fct) return my_filter64_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_filter64_fct_##A == 0) {my_filter64_fct_##A = (uintptr_t)fct; return my_filter64_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc filter64 callback\n");
    return NULL;
}
// compare64
#define GO(A)   \
static uintptr_t my_compare64_fct_##A = 0;                                      \
static int my_compare64_##A(const struct dirent64* a, const struct dirent64* b) \
{                                                                               \
    return (int)RunFunctionFmt(my_compare64_fct_##A, "pp", a, b);         \
}
SUPER()
#undef GO
static void* findcompare64Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_compare64_fct_##A == (uintptr_t)fct) return my_compare64_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_compare64_fct_##A == 0) {my_compare64_fct_##A = (uintptr_t)fct; return my_compare64_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc compare64 callback\n");
    return NULL;
}
// printf_output
#define GO(A)   \
static uintptr_t my_printf_output_fct_##A = 0;                                          \
static int my_printf_output_##A(void* a, void* b, void* c)                              \
{                                                                                       \
    return (int)RunFunctionFmt(my_printf_output_fct_##A, "ppp", a, b, c);         \
}
SUPER()
#undef GO
static void* findprintf_outputFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_printf_output_fct_##A == (uintptr_t)fct) return my_printf_output_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_printf_output_fct_##A == 0) {my_printf_output_fct_##A = (uintptr_t)fct; return my_printf_output_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc printf_output callback\n");
    return NULL;
}
// printf_arginfo
#define GO(A)   \
static uintptr_t my_printf_arginfo_fct_##A = 0;                                             \
static int my_printf_arginfo_##A(void* a, size_t b, void* c, void* d)                       \
{                                                                                           \
    return (int)RunFunctionFmt(my_printf_arginfo_fct_##A, "pLpp", a, b, c, d);        \
}
SUPER()
#undef GO
static void* findprintf_arginfoFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_printf_arginfo_fct_##A == (uintptr_t)fct) return my_printf_arginfo_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_printf_arginfo_fct_##A == 0) {my_printf_arginfo_fct_##A = (uintptr_t)fct; return my_printf_arginfo_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc printf_arginfo callback\n");
    return NULL;
}
// printf_type
#define GO(A)   \
static uintptr_t my_printf_type_fct_##A = 0;                        \
static void my_printf_type_##A(void* a, va_list* b)                 \
{                                                                   \
    RunFunctionFmt(my_printf_type_fct_##A, "pp", a, b);       \
}
SUPER()
#undef GO
static void* findprintf_typeFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_printf_type_fct_##A == (uintptr_t)fct) return my_printf_type_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_printf_type_fct_##A == 0) {my_printf_type_fct_##A = (uintptr_t)fct; return my_printf_type_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc printf_type callback\n");
    return NULL;
}

#undef SUPER

// some my_XXX declare and defines
int32_t my___libc_start_main(x64emu_t* emu, int (*main) (int, char * *, char * *),
    int argc, char * * ubp_av, void (*init) (void), void (*fini) (void),
    void (*rtld_fini) (void), void (* stack_end)); // implemented in x64run_private.c
EXPORT void my___libc_init_first(x64emu_t* emu, int argc, char* arg0, char** b)
{
    // do nothing specific for now
    (void)emu; (void)argc; (void)arg0; (void)b;
    return;
}
uintptr_t my_syscall(x64emu_t *emu); // implemented in x64syscall.c
void EXPORT my___stack_chk_fail(x64emu_t* emu)
{
    char buff[200];
    #ifdef HAVE_TRACE
    sprintf(buff, "%p: Stack is corrupted, aborting (prev IP=%p)\n", (void*)emu->old_ip, (void*)emu->prev2_ip);
    #else
    sprintf(buff, "%p: Stack is corrupted, aborting\n", (void*)emu->old_ip);
    #endif
    if(BOX64ENV(rolling_log)) {
        print_rolling_log(LOG_INFO);
    }
    StopEmu(emu, buff, emu->segs[_CS]==0x23);
}
void EXPORT my___gmon_start__(x64emu_t *emu)
{
    (void)emu;
    printf_log(LOG_DEBUG, "__gmon_start__ called (dummy call)\n");
}

int EXPORT my___cxa_atexit(x64emu_t* emu, void* p, void* a, void* dso_handle)
{
    AddCleanup1Arg(emu, p, a, FindElfAddress(my_context, (uintptr_t)dso_handle));
    return 0;
}
void EXPORT my___cxa_finalize(x64emu_t* emu, void* p)
{
    if(!p) {
        // p is null, call (and remove) all Cleanup functions
        CallAllCleanup(emu);
        return;
    }
    CallCleanup(emu, FindElfAddress(my_context, (uintptr_t)p));
}
int EXPORT my_atexit(x64emu_t* emu, void *p)
{
    AddCleanup(emu, p);
    return 0;
}

int my_getcontext(x64emu_t* emu, void* ucp);
int my_setcontext(x64emu_t* emu, void* ucp);
int my_makecontext(x64emu_t* emu, void* ucp, void* fnc, int32_t argc, void* argv);
int my_swapcontext(x64emu_t* emu, void* ucp1, void* ucp2);

// All signal and context functions defined in signals.c

// All fts function defined in myfts.c

// getauxval implemented in auxval.c


// this one is defined in elfloader.c
int my_dl_iterate_phdr(x64emu_t *emu, void* F, void *data);

pid_t EXPORT my_fork(x64emu_t* emu)
{
    #if 1
    emu->quit = 1;
    emu->fork = 1;  // use regular fork...
    return 0;
    #else
    // execute atforks prepare functions, in reverse order
    for (int i=my_context->atfork_sz-1; i>=0; --i)
        if(my_context->atforks[i].prepare)
            RunFunctionWithEmu(emu, 0, my_context->atforks[i].prepare, 0);
    int type = emu->type;
    pid_t v;
    v = fork();
    if(type == EMUTYPE_MAIN)
        thread_set_emu(emu);
    if(v<0) {
        printf_log(LOG_NONE, "Warning, fork errored... (%d)\n", v);
        // error...
    } else if(v>0) {
        // execute atforks parent functions
        for (int i=0; i<my_context->atfork_sz; --i)
            if(my_context->atforks[i].parent)
                RunFunctionWithEmu(emu, 0, my_context->atforks[i].parent, 0);

    } else /*if(v==0)*/ {
        // execute atforks child functions
        for (int i=0; i<my_context->atfork_sz; --i)
            if(my_context->atforks[i].child)
                RunFunctionWithEmu(emu, 0, my_context->atforks[i].child, 0);
    }
    return v;
    #endif
}
pid_t EXPORT my___fork(x64emu_t* emu) __attribute__((alias("my_fork")));
pid_t EXPORT my_vfork(x64emu_t* emu)
{
    #if 1
    emu->quit = 1;
    emu->fork = 3;  // use regular fork...
    return 0;
    #else
    return 0;
    #endif
}

int EXPORT my_uname(struct utsname *buf)
{
    //TODO: check sizeof(struct utsname) == 390
    int ret = uname(buf);
    strcpy(buf->machine, "x86_64");
    return ret;
}

// X86_O_RDONLY 0x00
#define X86_O_WRONLY       0x01     // octal     01
#define X86_O_RDWR         0x02     // octal     02
#define X86_FMODE_EXEC     0x20
#define X86_O_CREAT        0x40     // octal     0100
#define X86_O_EXCL         0x80     // octal     0200
#define X86_O_NOCTTY       0x100    // octal     0400
#define X86_O_TRUNC        0x200    // octal    01000
#define X86_O_APPEND       0x400    // octal    02000
#define X86_O_NONBLOCK     0x800    // octal    04000
#define X86_O_SYNC         0x101000 // octal 04010000
#define X86_O_DSYNC        0x1000   // octal   010000
#define X86_O_RSYNC        X86_O_SYNC
#define X86_FASYNC         020000
#define X86_O_DIRECT       040000
#define X86_O_LARGEFILE    0100000
#define X86_O_DIRECTORY    0200000
#define X86_O_NOFOLLOW     0400000
#define X86_O_NOATIME      01000000
#define X86_O_CLOEXEC      02000000
#define X86_O_PATH         010000000
#define X86_O_TMPFILE      020200000
#define X86_FMODE_NONOTIFY 0x4000000

#ifndef O_TMPFILE
#define O_TMPFILE (020000000 | O_DIRECTORY)
#endif

#ifndef FMODE_EXEC
#define FMODE_EXEC 0x20
#endif
#ifndef FMODE_NONOTIFY
#define FMODE_NONOTIFY 0x4000000
#endif

#define SUPER()     \
    GO(O_WRONLY)    \
    GO(O_RDWR)      \
    GO(FMODE_EXEC)  \
    GO(O_CREAT)     \
    GO(O_EXCL)      \
    GO(O_NOCTTY)    \
    GO(O_TRUNC)     \
    GO(O_APPEND)    \
    GO(O_NONBLOCK)  \
    GO(O_SYNC)      \
    GO(O_DSYNC)     \
    GO(O_RSYNC)     \
    GO(FASYNC)      \
    GO(O_DIRECT)    \
    GO(O_LARGEFILE) \
    GO(O_TMPFILE)   \
    GO(O_DIRECTORY) \
    GO(O_NOFOLLOW)  \
    GO(O_NOATIME)   \
    GO(O_CLOEXEC)   \
    GO(O_PATH)      \
    GO(FMODE_NONOTIFY)

// x86->arm
int of_convert(int a)
{
    if(!a || a==-1) return a;
    int b=0;
    #define GO(A) if((a&(X86_##A))==(X86_##A)) {a&=~(X86_##A); b|=(A);}
    SUPER();
    #undef GO
    if(a) {
        printf_log(LOG_NONE, "Warning, of_convert(...) left over 0x%x, converted 0x%x\n", a, b);
    }
    return a|b;
}

// arm->x86
int of_unconvert(int a)
{
    if(!a || a==-1) return a;
    int b=0;
    #define GO(A) if((a&(A))==(A)) {a&=~(A); b|=(X86_##A);}
    SUPER();
    #undef GO
    int missing = 0;
    #ifdef ARM64
    if(!O_LARGEFILE) {
        if((a&(0400000))==(0400000)) {a&=~(0400000); b|=(X86_O_LARGEFILE);}
    }
    #else
    if(!O_LARGEFILE) missing |= X86_O_LARGEFILE;
    #endif
    if(a && (a&~missing)) {
        printf_log(LOG_NONE, "Warning, of_unconvert(...) left over 0x%x, converted 0x%x\n", a, b);
    }
    return a|b;
}
#undef SUPER

EXPORT void* my__ZGTtnaX (size_t a) { (void)a; printf("warning _ZGTtnaX called\n"); return NULL; }
EXPORT void* my__ZGTtnam (size_t a) { (void)a; printf("warning _ZGTtnam called\n"); return NULL; }
EXPORT void my__ZGTtdlPv (void* a) { (void)a; printf("warning _ZGTtdlPv called\n"); }
EXPORT uint8_t my__ITM_RU1(const uint8_t * a) { (void)a; printf("warning _ITM_RU1 called\n"); return 0; }
EXPORT uint32_t my__ITM_RU4(const uint32_t * a) { (void)a; printf("warning _ITM_RU4 called\n"); return 0; }
EXPORT uint64_t my__ITM_RU8(const uint64_t * a) { (void)a; printf("warning _ITM_RU8 called\n"); return 0; }
EXPORT void my__ITM_memcpyRtWn(void * a, const void * b, size_t c) { (void)a; (void)b; (void)c; printf("warning _ITM_memcpyRtWn called\n"); }
EXPORT void my__ITM_memcpyRnWt(void * a, const void * b, size_t c) { (void)a; (void)b; (void)c; printf("warning _ITM_memcpyRnWt called\n"); }

EXPORT void my_longjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val);
EXPORT void my__longjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val) __attribute__((alias("my_longjmp")));
EXPORT void my_siglongjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val) __attribute__((alias("my_longjmp")));
EXPORT void my___longjmp_chk(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val) __attribute__((alias("my_longjmp")));

//EXPORT int32_t my_setjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p);
//EXPORT int32_t my__setjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p) __attribute__((alias("my_setjmp")));
//EXPORT int32_t my___sigsetjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p) __attribute__((alias("my_setjmp")));

EXPORT int my_printf(x64emu_t *emu, void* fmt, void* b) {
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    return vprintf((const char*)fmt, VARARGS);
}
EXPORT int my___printf_chk(x64emu_t *emu, int chk, void* fmt, void* b)
{
    (void)chk;
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    return vprintf((const char*)fmt, VARARGS);
}
extern int box64_stdout_no_w;
EXPORT int my_wprintf(x64emu_t *emu, void* fmt, void* b) {
    myStackAlignW(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    if(box64_stdout_no_w) {
        wchar_t buff[2048];
        int ret = vswprintf(buff, 2047, fmt, VARARGS);
        printf("%S", buff);
        return ret;
    }
    return vwprintf((const wchar_t*)fmt, VARARGS);
}
EXPORT int my___wprintf_chk(x64emu_t *emu, int chk, void* fmt, void* b)
{
    (void)chk;
    myStackAlignW(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    return vwprintf((const wchar_t*)fmt, VARARGS);
}

EXPORT int my_vprintf(x64emu_t *emu, void* fmt, x64_va_list_t b) {
    (void)emu;
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return vprintf(fmt, VARARGS);
}
EXPORT int my___vprintf_chk(x64emu_t *emu, void* fmt, x64_va_list_t b) __attribute__((alias("my_vprintf")));

EXPORT int my_vfprintf(x64emu_t *emu, void* F, void* fmt, x64_va_list_t b) {
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return vfprintf(F, fmt, VARARGS);
}
EXPORT int my___vfprintf_chk(x64emu_t *emu, void* F, void* fmt, x64_va_list_t b) __attribute__((alias("my_vfprintf")));
EXPORT int my__IO_vfprintf(x64emu_t *emu, void* F, void* fmt, x64_va_list_t b) __attribute__((alias("my_vfprintf")));

EXPORT int my_fprintf(x64emu_t *emu, void* F, void* fmt, void* b)  {
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    return vfprintf(F, fmt, VARARGS);
}
EXPORT int my___fprintf_chk(x64emu_t *emu, void* F, int flag, void* fmt, void* b)  {
    (void)flag;
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    return vfprintf(F, fmt, VARARGS);
}

EXPORT int my_vwprintf(x64emu_t *emu, void* fmt, x64_va_list_t b) {
    (void)emu;
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignWValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    int r = vwprintf(fmt, VARARGS);
    return r;
}

EXPORT int my_fwprintf(x64emu_t *emu, void* F, void* fmt, void* b)  {
    myStackAlignW(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    return vfwprintf(F, fmt, VARARGS);
}

EXPORT int my___fwprintf_chk(x64emu_t *emu, void* F, int flag, void* fmt, void* b) {
    myStackAlignW(emu, (const char*)fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    return vfwprintf(F, fmt, VARARGS);
}

EXPORT int my_vfwprintf(x64emu_t *emu, void* F, void* fmt, x64_va_list_t  b) {
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignWValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return vfwprintf(F, fmt, VARARGS);
}
EXPORT int my___vfwprintf_chk(x64emu_t *emu, void* F, int flag, void* fmt, x64_va_list_t b)  {
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignWValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return vfwprintf(F, fmt, VARARGS);
}

EXPORT int my_dprintf(x64emu_t *emu, int d, void* fmt, void* b) {
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    return vdprintf(d, fmt, VARARGS);
}

EXPORT int my___dprintf_chk(x64emu_t *emu, int d, int flag, void* fmt, void* b)  {
    (void)flag;
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    return vdprintf(d, fmt, VARARGS);
}


EXPORT int my_vdprintf(x64emu_t *emu, int d, void* fmt, x64_va_list_t b) {
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return vdprintf(d, fmt, VARARGS);
}

EXPORT int my___vdprintf_chk(x64emu_t *emu, int d, int flag, void* fmt, x64_va_list_t b)  {
    (void)flag;
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return vdprintf(d, fmt, VARARGS);
}

#if 0
EXPORT void *my_div(void *result, int numerator, int denominator) {
    *(div_t *)result = div(numerator, denominator);
    return result;
}
#endif

EXPORT int my_snprintf(x64emu_t* emu, void* buff, size_t s, void * fmt, uint64_t * b) {
    #ifdef PREFER_CONVERT_VAARG
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 3);
    #else
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    #endif
    int r = vsnprintf(buff, s, fmt, VARARGS);
    return r;
}
EXPORT int my___snprintf(x64emu_t* emu, void* buff, size_t s, void * fmt, uint64_t * b) __attribute__((alias("my_snprintf")));
EXPORT int my___snprintf_chk(x64emu_t* emu, void* buff, size_t s, int flags, size_t maxlen, void * fmt, uint64_t * b)
{
    (void)flags; (void)maxlen;
    #ifdef PREFER_CONVERT_VAARG
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 5);
    #else
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 5);
    PREPARE_VALIST;
    #endif
    int r = vsnprintf(buff, s, fmt, VARARGS);
    return r;
}

EXPORT int my_sprintf(x64emu_t* emu, void* buff, void * fmt, void * b) {
    #ifdef PREFER_CONVERT_VAARG
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    #else
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    #endif
    return vsprintf(buff, (const char*)fmt, VARARGS);
}
EXPORT int my___sprintf_chk(x64emu_t* emu, void* buff, int flag, size_t l, void * fmt, void * b) {
    (void)flag; (void)l;
    #ifdef PREFER_CONVERT_VAARG
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 4);
    #else
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 4);
    PREPARE_VALIST;
    #endif
    return vsprintf(buff, (const char*)fmt, VARARGS);
}

EXPORT int my_asprintf(x64emu_t* emu, void** buff, void * fmt, uint64_t * b) {
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    return vasprintf((char**)buff, (char*)fmt, VARARGS);
}
EXPORT int my___asprintf(x64emu_t* emu, void** buff, void * fmt, uint64_t * b) __attribute__((alias("my_asprintf")));

EXPORT int my_vasprintf(x64emu_t* emu, char** buff, void* fmt, x64_va_list_t b) {
    (void)emu;
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return vasprintf(buff, fmt, VARARGS);
}

EXPORT int my_vsprintf(x64emu_t* emu, void* buff,  void * fmt, x64_va_list_t b) {
    (void)emu;
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return vsprintf(buff, fmt, VARARGS);
}
EXPORT int my___vsprintf_chk(x64emu_t* emu, void* buff, void * fmt, x64_va_list_t b) __attribute__((alias("my_vsprintf")));

EXPORT int my_scanf(x64emu_t* emu, void* fmt, uint64_t* b)
{
    myStackAlignScanf(emu, (const char*)fmt, b, emu->scratch, 1);
    PREPARE_VALIST;

    return vscanf(fmt, VARARGS);
}

EXPORT int my_vfscanf(x64emu_t* emu, void* stream, void* fmt, x64_va_list_t b)
{
    (void)emu;
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignScanfValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return vfscanf(stream, fmt, VARARGS);
}

EXPORT int my_vsscanf(x64emu_t* emu, void* stream, void* fmt, x64_va_list_t b)
{
    (void)emu;
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignScanfValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return vsscanf(stream, fmt, VARARGS);
}

EXPORT int my___vsscanf(x64emu_t* emu, void* stream, void* fmt, void* b) __attribute__((alias("my_vsscanf")));

EXPORT int my_vfwscanf(x64emu_t* emu, void* F, void* fmt, x64_va_list_t b)
{
    (void)emu;
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignScanfWValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return vfwscanf(F, fmt, VARARGS);
}

EXPORT int my_vwscanf(x64emu_t* emu, void* fmt, x64_va_list_t b)
{
    (void)emu;
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignScanfWValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return vwscanf(fmt, VARARGS);
}

EXPORT int my_wscanf(x64emu_t* emu, void* fmt, uint64_t* b)
{
    myStackAlignScanfW(emu, (const char*)fmt, b, emu->scratch, 1);
    PREPARE_VALIST;

    return vwscanf(fmt, VARARGS);
}

EXPORT int my_vswscanf(x64emu_t* emu, void* stream, void* fmt, x64_va_list_t b)
{
    (void)emu;
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignScanfWValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return vswscanf(stream, fmt, VARARGS);
}

EXPORT int my_sscanf(x64emu_t* emu, void* stream, void* fmt, uint64_t* b)
{
    myStackAlignScanf(emu, (const char*)fmt, b, emu->scratch, 2);
    PREPARE_VALIST;

    return vsscanf(stream, fmt, VARARGS);
}
EXPORT int my_vscanf(x64emu_t* emu, void* fmt, x64_va_list_t b)
{
    (void)emu;
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignScanfValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return vscanf(fmt, VARARGS);
}

EXPORT int my__IO_vfscanf(x64emu_t* emu, void* stream, void* fmt, void* b) __attribute__((alias("my_vfscanf")));
EXPORT int my___isoc99_vsscanf(x64emu_t* emu, void* stream, void* fmt, void* b) __attribute__((alias("my_vsscanf")));
EXPORT int my___isoc99_vscanf(x64emu_t* emu, void* fmt, void* b) __attribute__((alias("my_vscanf")));
EXPORT int my___isoc99_vswscanf(x64emu_t* emu, void* stream, void* fmt, void* b) __attribute__((alias("my_vswscanf")));
EXPORT int my___isoc99_vfscanf(x64emu_t* emu, void* stream, void* fmt, void* b) __attribute__((alias("my_vfscanf")));

EXPORT int my___isoc99_fscanf(x64emu_t* emu, void* stream, void* fmt, uint64_t* b)
{
  myStackAlignScanf(emu, (const char*)fmt, b, emu->scratch, 2);
  PREPARE_VALIST;

  return vfscanf(stream, fmt, VARARGS);
}
EXPORT int my_fscanf(x64emu_t* emu, void* stream, void* fmt, uint64_t* b) __attribute__((alias("my___isoc99_fscanf")));

EXPORT int my___isoc99_scanf(x64emu_t* emu, void* fmt, uint64_t* b)
{
  myStackAlignScanf(emu, (const char*)fmt, b, emu->scratch, 1);
  PREPARE_VALIST;

  return vscanf(fmt, VARARGS);
}

EXPORT int my___isoc99_sscanf(x64emu_t* emu, void* stream, void* fmt, uint64_t* b)
{
  myStackAlignScanf(emu, (const char*)fmt, b, emu->scratch, 2);
  PREPARE_VALIST;

  return vsscanf(stream, fmt, VARARGS);
}

EXPORT int my___isoc99_swscanf(x64emu_t* emu, void* stream, void* fmt, uint64_t* b)
{
  myStackAlignScanfW(emu, (const char*)fmt, b, emu->scratch, 2);
  PREPARE_VALIST;

  return vswscanf(stream, fmt, VARARGS);
}

EXPORT int my_vsnprintf(x64emu_t* emu, void* buff, size_t s, void * fmt, x64_va_list_t b) {
    (void)emu;
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    int r = vsnprintf(buff, s, fmt, VARARGS);
    return r;
}
EXPORT int my___vsnprintf(x64emu_t* emu, void* buff, size_t s, void * fmt, x64_va_list_t b) __attribute__((alias("my_vsnprintf")));
EXPORT int my___vsnprintf_chk(x64emu_t* emu, void* buff, size_t s, int flags, size_t slen, void * fmt, x64_va_list_t b) {
    (void)emu;
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    int r = vsnprintf(buff, s, fmt, VARARGS);
    return r;
}
#if 0
EXPORT int my_vasprintf(x64emu_t* emu, void* strp, void* fmt, void* b, va_list V)
{
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, (uint32_t*)b, emu->scratch);
    PREPARE_VALIST;
    void* f = vasprintf;
    int r = ((iFppp_t)f)(strp, fmt, VARARGS);
    return r;
    #else
    void* f = vasprintf;
    int r = ((iFppp_t)f)(strp, fmt, (uint32_t*)b);
    return r;
    #endif
}
#endif
EXPORT int my___vasprintf_chk(x64emu_t* emu, void* buff, int flags, void* fmt, x64_va_list_t b)
{
    (void)emu; (void)flags;
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    int r = vasprintf(buff, fmt, VARARGS);
    return r;
}
EXPORT int my___asprintf_chk(x64emu_t* emu, void* result_ptr, int flags, void* fmt, void* b)
{
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    return vasprintf((char**)result_ptr, (char*)fmt, VARARGS);
}
EXPORT int my_vswprintf(x64emu_t* emu, void* buff, size_t s, void * fmt, x64_va_list_t b) {
    (void)emu;
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignWValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    int r = vswprintf(buff, s, fmt, VARARGS);
    return r;
}
EXPORT int my___vswprintf(x64emu_t* emu, void* buff, size_t s, void * fmt, x64_va_list_t b) __attribute__((alias("my_vswprintf")));
EXPORT int my___vswprintf_chk(x64emu_t* emu, void* buff, size_t s, void * fmt, x64_va_list_t b) __attribute__((alias("my_vswprintf")));

EXPORT int my_swscanf(x64emu_t* emu, void* stream, void* fmt, uint64_t* b)
{
    myStackAlignScanfW(emu, (const char*)fmt, b, emu->scratch, 2);
    PREPARE_VALIST;

    return vswscanf(stream, fmt, VARARGS);
}

EXPORT void my_error(x64emu_t *emu, int status, int errnum, void* fmt, void* b) {
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    char buf[512];
    vsnprintf(buf, 512, (const char*)fmt, VARARGS);
    error(status, errnum, "%s", buf);
}
EXPORT void my_error_at_line(x64emu_t *emu, int status, int errnum, void* filename, uint32_t linenum, void* fmt, void* b) {
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 5);
    PREPARE_VALIST;
    char buf[512];
    vsnprintf(buf, 512, (const char*)fmt, VARARGS);
    error_at_line(status, errnum, filename, linenum, "%s", buf);
}

EXPORT void my_verr(x64emu_t* emu, int eval, void* fmt, x64_va_list_t b) {
    if (!fmt)
        return err(eval, NULL);
    #ifdef CONVERT_VALIST
    (void)emu;
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return verr(eval, fmt, VARARGS);
}
EXPORT void my_verrx(x64emu_t* emu, int eval, void* fmt, x64_va_list_t b) {
    #ifdef CONVERT_VALIST
    (void)emu;
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return verrx(eval, fmt, VARARGS);
}
EXPORT void my_err(x64emu_t *emu, int eval, void* fmt, void* b) {
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    verr(eval, (const char*)fmt, VARARGS);
}
EXPORT void my_errx(x64emu_t *emu, int eval, void* fmt, void* b) {
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    verrx(eval, (const char*)fmt, VARARGS);
}
EXPORT void my_vwarn(x64emu_t* emu, void* fmt, x64_va_list_t b) {
    if (!fmt)
        return warn(NULL);
    #ifdef CONVERT_VALIST
    (void)emu;
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return vwarn(fmt, VARARGS);
}
EXPORT void my_vwarnx(x64emu_t* emu, void* fmt, x64_va_list_t b) {
    if (!fmt)
        return warnx(NULL);
    #ifdef CONVERT_VALIST
    (void)emu;
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return vwarnx(fmt, VARARGS);
}
EXPORT void my_warn(x64emu_t *emu, void* fmt, void* b) {
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    vwarn((const char*)fmt, VARARGS);
}
EXPORT void my_warnx(x64emu_t *emu, void* fmt, void* b) {
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    vwarnx((const char*)fmt, VARARGS);
}

EXPORT void my_syslog(x64emu_t* emu, int priority, const char* fmt, uint64_t* b)
{
    myStackAlign(emu, fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    return vsyslog(priority, fmt, VARARGS);
}
EXPORT void my___syslog_chk(x64emu_t* emu, int priority, int flags, const char* fmt, uint64_t* b)
{
    (void)flags;
    myStackAlign(emu, fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    return vsyslog(priority, fmt, VARARGS);
}
EXPORT void my_vsyslog(x64emu_t* emu, int priority, const char* fmt, x64_va_list_t b)
{
    (void)emu;
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return vsyslog(priority, fmt, VARARGS);
}
EXPORT void my___vsyslog_chk(x64emu_t* emu, int priority, int flag, const char* fmt, x64_va_list_t b)
{
    (void)emu;
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return vsyslog(priority, fmt, VARARGS);
}

EXPORT int my___swprintf_chk(x64emu_t* emu, void* s, size_t n, int32_t flag, size_t slen, void* fmt, uint64_t* b)
{
    (void)flag;
    (void)slen;
    myStackAlignW(emu, (const char*)fmt, b, emu->scratch, R_EAX, 5);
    PREPARE_VALIST;
    return vswprintf(s, n, (const wchar_t*)fmt, VARARGS);
}
EXPORT int my_swprintf(x64emu_t* emu, void* s, size_t n, void* fmt, uint64_t* b)
{
    myStackAlignW(emu, (const char*)fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    return vswprintf(s, n, (const wchar_t*)fmt, VARARGS);
}

EXPORT void my__ITM_addUserCommitAction(x64emu_t* emu, void* cb, uint32_t b, void* c)
{
    // disabled for now... Are all this _ITM_ stuff really mendatory?
    #if 0
    // quick and dirty... Should store the callback to be removed later....
    libc_my_t *my = (libc_my_t *)emu->context->libclib->w.p2;
    x64emu_t *cbemu = AddCallback(emu, (uintptr_t)cb, 1, c, NULL, NULL, NULL);
    my->_ITM_addUserCommitAction(libc1ArgCallback, b, cbemu);
    // should keep track of cbemu to remove at some point...
    #else
    (void)emu; (void)cb; (void)b; (void)c;
    printf("warning _ITM_addUserCommitAction called\n");
    #endif
}
EXPORT void my__ITM_registerTMCloneTable(x64emu_t* emu, void* p, uint32_t s) { (void)emu; (void)p; (void)s; }
EXPORT void my__ITM_deregisterTMCloneTable(x64emu_t* emu, void* p) { (void)emu; (void)p; }


EXPORT int my___fxstat(x64emu_t *emu, int vers, int fd, void* buf)
{
    (void)emu; (void)vers;
    struct stat64 st;
    int r = fstat64(fd, buf?&st:buf);
    if(buf && !r)
        UnalignStat64(&st, buf);
    return r;
}

EXPORT int my___fxstat64(x64emu_t *emu, int vers, int fd, void* buf)
{
    (void)emu; (void)vers;
    struct stat64 st;
    int r = fstat64(fd, buf?&st:buf);
    if(buf && !r)
        UnalignStat64(&st, buf);
    return r;
}

EXPORT int my_statx(x64emu_t* emu, int dirfd, void* path, int flags, uint32_t mask, void* buf)
{
    if(my->statx)
        return my->statx(dirfd, path, flags, mask, buf);
    #ifdef __NR_statx
    int ret = syscall(__NR_statx, dirfd, path, flags, mask, buf);
    if(ret<0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
    #else
    errno = ENOSYS;
    return -1;
    #endif
}

EXPORT int my___xmknod(x64emu_t* emu, int v, char* path, uint32_t mode, dev_t* dev)
{
    (void)emu;
    (void)v;
    return mknod((const char*)path, mode, *dev);
}

EXPORT int my___xmknodat(x64emu_t* emu, int v, int dirfd, char* path, uint32_t mode, dev_t* dev)
{
    (void)emu;
    (void)v;
    return mknodat(dirfd, (const char*)path, mode, *dev);
}

EXPORT int my___xstat(x64emu_t* emu, int v, void* path, void* buf)
{
    (void)emu; (void)v;
    struct stat64 st;
    int r = stat64((const char*)path, buf?&st:buf);
    if(buf && !r)
        UnalignStat64(&st, buf);
    return r;
}

EXPORT int my___xstat64(x64emu_t* emu, int v, void* path, void* buf)
{
    (void)emu; (void)v;
    struct stat64 st;
    int r = stat64((const char*)path, buf?&st:buf);
    if(buf && !r)
        UnalignStat64(&st, buf);
    return r;
}

EXPORT int my___lxstat(x64emu_t* emu, int v, void* name, void* buf)
{
    (void)emu; (void)v;
    struct stat64 st;
    int r = lstat64((const char*)name, buf?&st:buf);
    if(buf && !r)
        UnalignStat64(&st, buf);
    return r;
}

EXPORT int my___lxstat64(x64emu_t* emu, int v, void* name, void* buf)
{
    (void)emu; (void)v;
    struct stat64 st;
    int r = lstat64((const char*)name, buf?&st:buf);
    if(buf && !r)
        UnalignStat64(&st, buf);
    return r;
}

EXPORT int my___fxstatat(x64emu_t* emu, int v, int d, void* path, void* buf, int flags)
{
    (void)emu; (void)v;
    struct  stat64 st;
    int r = fstatat64(d, path, &st, flags);
    if(!r)
        UnalignStat64(&st, buf);
    return r;
}

EXPORT int my___fxstatat64(x64emu_t* emu, int v, int d, void* path, void* buf, int flags)
{
    (void)emu; (void)v;
    struct  stat64 st;
    int r = fstatat64(d, path, &st, flags);
    if(!r)
        UnalignStat64(&st, buf);
    return r;
}

EXPORT int my_stat(x64emu_t *emu, void* filename, void* buf)
{
    (void)emu;
    struct stat st;
    int r = stat(filename, &st);
    if(!r)
        UnalignStat64(&st, buf);
    return r;
}
EXPORT int my_stat64(x64emu_t *emu, void* filename, void* buf) __attribute__((alias("my_stat")));

EXPORT int my_lstat(x64emu_t *emu, void* filename, void* buf)
{
    (void)emu;
    struct stat st;
    int r = lstat(filename, &st);
    if(!r)
        UnalignStat64(&st, buf);
    return r;
}
EXPORT int my_lstat64(x64emu_t *emu, void* filename, void* buf) __attribute__((alias("my_lstat")));

EXPORT int my_fstat(x64emu_t *emu, int fd, void* buf)
{
    (void)emu;
    struct stat st;
    int r = fstat(fd, &st);
    if(!r)
        UnalignStat64(&st, buf);
    return r;
}
EXPORT int my_fstat64(x64emu_t* emu, int fd, void* buf) __attribute__((alias("my_fstat")));

EXPORT int my_fstatat(x64emu_t *emu, int fd, const char* path, void* buf, int flags)
{
    (void)emu;
    struct stat st;
    int r = fstatat(fd, path, &st, flags);
    if(!r)
        UnalignStat64(&st, buf);
    return r;
}
EXPORT int my_fstatat64(x64emu_t *emu, int fd, const char* path, void* buf, int flags) __attribute__((alias("my_fstatat")));

EXPORT int my__IO_file_stat(x64emu_t* emu, void* f, void* buf)
{
    struct stat64 st;
    int r = my->_IO_file_stat(f, &st);
    UnalignStat64(&st, buf);
    return r;
}

#if 0
EXPORT int my_fstatfs64(int fd, void* buf)
{
    struct statfs64 st;
    int r = fstatfs64(fd, &st);
    UnalignStatFS64(&st, buf);
    return r;
}

EXPORT int my_statfs64(const char* path, void* buf)
{
    struct statfs64 st;
    int r = statfs64(path, &st);
    UnalignStatFS64(&st, buf);
    return r;
}
#endif

#ifdef ANDROID
typedef int (*__compar_d_fn_t)(const void*, const void*, void*);

static size_t qsort_r_partition(void* base, size_t size, __compar_d_fn_t compar, void* arg, size_t lo, size_t hi)
{
    void* tmp = alloca(size);
    void* pivot = ((char*)base) + lo * size;
    size_t i = lo;
    for (size_t j = lo; j <= hi; j++)
    {
        void* base_i = ((char*)base) + i * size;
        void* base_j = ((char*)base) + j * size;
        if (compar(base_j, pivot, arg) < 0)
        {
            memcpy(tmp, base_i, size);
            memcpy(base_i, base_j, size);
            memcpy(base_j, tmp, size);
            i++;
        }
    }
    void* base_i = ((char *)base) + i * size;
    void* base_hi = ((char *)base) + hi * size;
    memcpy(tmp, base_i, size);
    memcpy(base_i, base_hi, size);
    memcpy(base_hi, tmp, size);
    return i;
}

static void qsort_r_helper(void* base, size_t size, __compar_d_fn_t compar, void* arg, ssize_t lo, ssize_t hi)
{
    if (lo < hi)
    {
        size_t p = qsort_r_partition(base, size, compar, arg, lo, hi);
        qsort_r_helper(base, size, compar, arg, lo, p - 1);
        qsort_r_helper(base, size, compar, arg, p + 1, hi);
    }
}

static void qsort_r(void* base, size_t nmemb, size_t size, __compar_d_fn_t compar, void* arg)
{
    return qsort_r_helper(base, size, compar, arg, 0, nmemb - 1);
}
#endif

typedef struct compare_r_s {
    x64emu_t* emu;
    uintptr_t f;
    void*     data;
    int       r;
} compare_r_t;

static int my_compare_r_cb(void* a, void* b, compare_r_t* arg)
{
    return (int)RunFunctionWithEmu(arg->emu, 0, arg->f, 2+arg->r, a, b, arg->data);
}
EXPORT void my_qsort(x64emu_t* emu, void* base, size_t nmemb, size_t size, void* fnc)
{
    compare_r_t args;
    args.emu = emu; args.f = (uintptr_t)fnc; args.r = 0; args.data = NULL;
    qsort_r(base, nmemb, size, (__compar_d_fn_t)my_compare_r_cb, &args);
}
EXPORT void my_qsort_r(x64emu_t* emu, void* base, size_t nmemb, size_t size, void* fnc, void* data)
{
    compare_r_t args;
    args.emu = emu; args.f = (uintptr_t)fnc; args.r = 1; args.data = data;
    qsort_r(base, nmemb, size, (__compar_d_fn_t)my_compare_r_cb, &args);
}
EXPORT void* my_bsearch(x64emu_t* emu, void* key, void* base, size_t nmemb, size_t size, void* fnc)
{
    (void)emu;
    return bsearch(key, base, nmemb, size, findcompareFct(fnc));
}

EXPORT void* my_lsearch(x64emu_t* emu, void* key, void* base, size_t* nmemb, size_t size, void* fnc)
{
    (void)emu;
    return lsearch(key, base, nmemb, size, findcompareFct(fnc));
}

EXPORT void* my_tsearch(x64emu_t* emu, void* key, void* root, void* fnc)
{
    (void)emu;
    return tsearch(key, root, findcompareFct(fnc));
}

EXPORT int my___sysctl(x64emu_t* emu, int* name, int nlen, void* oldval, size_t* oldlenp, void* newval, size_t newlen)
{
    return ENOSYS;
}

EXPORT int my_sysctl(x64emu_t* emu, int* name, int nlen, void* oldval, size_t* oldlenp, void* newval, size_t newlen)
{
    /* Glibc 2.32 Release note.
      The deprecated <sys/sysctl.h> header and the sysctl function have been
      removed.  To support old binaries, the sysctl function continues to
      exist as a compatibility symbol (on those architectures which had it),
      but always fails with ENOSYS.  This reflects the removal of the system
      call from all architectures, starting with Linux 5.5.
    */
    return ENOSYS;
}

EXPORT void my_tdestroy(x64emu_t* emu, void* root, void* fnc)
{
    (void)emu;
    tdestroy(root, findfreeFct(fnc));
}
EXPORT void* my_tdelete(x64emu_t* emu, void* key, void** root, void* fnc)
{
    (void)emu;
    return tdelete(key, root, findcompareFct(fnc));
}
EXPORT void* my_tfind(x64emu_t* emu, void* key, void** root, void* fnc)
{
    (void)emu;
    return tfind(key, root, findcompareFct(fnc));
}
EXPORT void my_twalk(x64emu_t* emu, void* root, void* fnc)
{
    (void)emu;
    twalk(root, findactionFct(fnc));
}
EXPORT void* my_lfind(x64emu_t* emu, void* key, void* base, size_t* nmemb, size_t size, void* fnc)
{
    (void)emu;
    return lfind(key, base, nmemb, size, findcompareFct(fnc));
}

EXPORT void* my_fts_open(x64emu_t* emu, void* path, int options, void* c)
{
    (void)emu;
    return fts_open(path, options, findcompareFct(c));
}

EXPORT void* my_fts64_open(x64emu_t* emu, void* path, int options, void* c)
{
    (void)emu;
    return my->fts64_open(path, options, findcompareFct(c));
}

#if 0
struct i386_dirent {
    uint32_t d_ino;
    int32_t  d_off;
    uint16_t d_reclen;
    uint8_t  d_type;
    char     d_name[256];
};

EXPORT void* my_readdir(x64emu_t* emu, void* dirp)
{
    if (BOX64ENV(fix_64bit_inodes))
    {
        struct dirent64 *dp64 = readdir64((DIR *)dirp);
        if (!dp64) return NULL;
        uint32_t ino32 = dp64->d_ino ^ (dp64->d_ino >> 32);
        int32_t off32 = dp64->d_off;
        struct i386_dirent *dp32 = (struct i386_dirent *)&(dp64->d_off);
        dp32->d_ino = ino32;
        dp32->d_off = off32;
        dp32->d_reclen -= 8;
        return dp32;
    }
    else
    {
        static pFp_t f = NULL;
        if(!f) {
            library_t* lib = my_lib;
            if(!lib) return NULL;
            f = (pFp_t)dlsym(lib->w.lib, "readdir");
        }

        return f(dirp);
    }
}

EXPORT int32_t my_readdir_r(x64emu_t* emu, void* dirp, void* entry, void** result)
{
    struct dirent64 d64, *dp64;
    if (BOX64ENV(fix_64bit_inodes) && (sizeof(d64.d_name) > 1))
    {
        static iFppp_t f = NULL;
        if(!f) {
            library_t* lib = my_lib;
            if(!lib)
            {
                *result = NULL;
                return 0;
            }
            f = (iFppp_t)dlsym(lib->w.lib, "readdir64_r");
        }

        int r = f(dirp, &d64, &dp64);
        if (r || !dp64 || !entry)
        {
            *result = NULL;
            return r;
        }

        struct i386_dirent *dp32 = (struct i386_dirent *)entry;
        int namelen = dp64->d_reclen - offsetof(struct dirent64, d_name);
        if (namelen > sizeof(dp32->d_name))
        {
            *result = NULL;
            return ENAMETOOLONG;
        }

        dp32->d_ino = dp64->d_ino ^ (dp64->d_ino >> 32);
        dp32->d_off = dp64->d_off;
        dp32->d_reclen = namelen + offsetof(struct i386_dirent, d_name);
        dp32->d_type = dp64->d_type;
        memcpy(dp32->d_name, dp64->d_name, namelen);
        *result = dp32;
        return 0;
    }
    else
    {
        static iFppp_t f = NULL;
        if(!f) {
            library_t* lib = my_lib;
            if(!lib)
            {
                *result = NULL;
                return 0;
            }
            f = (iFppp_t)dlsym(lib->w.lib, "readdir_r");
        }

        return f(dirp, entry, result);
    }
}
#endif

static int isProcSelf(const char *path, const char* w)
{
    if(strncmp(path, "/proc/", 6)==0) {
        char tmp[64];
        // check if self ....
        sprintf(tmp, "/proc/self/%s", w);
        if(strcmp((const char*)path, tmp)==0)
            return 1;
        // check if self PID ....
        pid_t pid = getpid();
        sprintf(tmp, "/proc/%d/%s", pid, w);
        if(strcmp((const char*)path, tmp)==0)
            return 1;
    }
    return 0;
}

static int isSysCpuCache(const char *path, const char* w, int* _cpu, int* _index)
{
    char tmp[128] = {0};
    int cpu=0, index=0;
    if(sscanf(path, "/sys/devices/system/cpu/cpu%d/cache/index%d/%s", &cpu, &index, tmp)!=3)
        return 0;
    if(strcmp(tmp, w))
        return 0;
    if(_cpu) * _cpu = cpu;
    if(_index) *_index = index;
    return 1;
}

static long isProcMem(const char* path)
{
    long pid;
    if(sscanf(path, "/proc/%ld/mem", &pid)==1)
        return pid;
    return 0;
}

EXPORT ssize_t my_readlink(x64emu_t* emu, void* path, void* buf, size_t sz)
{
    if(isProcSelf((const char*)path, "exe")) {
        // special case for self...
        return strlen(strncpy((char*)buf, emu->context->fullpath, sz));
    }
    return readlink((const char*)path, (char*)buf, sz);
}

EXPORT ssize_t my___readlink_chk(x64emu_t* emu, void* path, void* buf, size_t sz, size_t buflen)
{
    return my_readlink(emu, path, buf, sz);
}

int getNCpu();  // defined in my_cpuid.c
const char* getBoxCpuName();    // defined in my_cpuid.c
const char* getCpuName(); // defined in my_cpu_id.c
double getBogoMips(); // defined in my_cpu_id.c

#ifndef NOALIGN
void CreateCPUInfoFile(int fd)
{
    size_t dummy;
    char buff[600];
    double freq = 600.0; // default to 600 MHz
    // try to get actual ARM max speed:
    FILE *f = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", "r");
    if(f) {
        int r;
        if(1==fscanf(f, "%d", &r))
            freq = r/1000.;
        fclose(f);
    }
    int n = getNCpu();
    // generate fake CPUINFO
    #define P \
    dummy = write(fd, buff, strlen(buff))
    for (int i=0; i<n; ++i) {
        sprintf(buff, "processor\t: %d\n", i);
        P;
        sprintf(buff, "vendor_id\t: GenuineIntel\n");
        P;
        sprintf(buff, "cpu family\t: 6\n");
        P;
        sprintf(buff, "model\t\t: 1\n");
        P;
        sprintf(buff, "model name\t: %s\n", getBoxCpuName());
        P;
        sprintf(buff, "stepping\t: 1\nmicrocode\t: 0x10\n");
        P;
        sprintf(buff, "cpu MHz\t\t: %g\n", freq);
        P;
        sprintf(buff, "cache size\t: %d\n", 4096);
        P;
        sprintf(buff, "physical id\t: %d\nsiblings\t: %d\n", 0, n);
        P;
        sprintf(buff, "core id\t\t: %d\ncpu cores\t: %d\n", i, n);
        P;
        sprintf(buff, "bogomips\t: %g\n", getBogoMips());
        P;
        sprintf(buff, "flags\t\t: fpu cx8 sep ht cmov clflush mmx sse sse2 syscall tsc lahf_lm ssse3 ht tm lm fxsr cpuid"\
                      "%s cx16 %s movbe pni "\
                      "sse4_1%s%s%s lzcnt popcnt%s%s%s%s%s%s%s%s%s\n",
                      BOX64ENV(pclmulqdq)?" pclmulqdq":"",
                      BOX64ENV(aes)?" aes":"",
                      BOX64ENV(sse42)?" sse4_2":"", BOX64ENV(avx)?" avx":"", BOX64ENV(shaext)?"sha_ni":"",
                      BOX64ENV(avx)?" bmi1":"", BOX64ENV(avx2)?" avx2":"", BOX64ENV(avx)?" bmi2":"",
                      (BOX64ENV(avx2)&&BOX64ENV(aes))?" vaes":"", BOX64ENV(avx2)?" fma":"",
                      BOX64ENV(avx)?" xsave":"", BOX64ENV(avx)?" f16c":"", BOX64ENV(avx2)?" randr":"",
                      BOX64ENV(avx2)?" adx":""
                      );
        P;
        sprintf(buff, "address sizes\t: 48 bits physical, 48 bits virtual\n");
        P;
        sprintf(buff, "\n");
        P;
    }
    (void)dummy;
    #undef P
}
void CreateCPUPresentFile(int fd)
{
    size_t dummy;
    char buff[600];
    int n = getNCpu();
    // generate fake CPUINFO
    sprintf(buff, "0-%d\n", n-1);
    dummy = write(fd, buff, strlen(buff));
    (void)dummy;
}
void CreateClocksourceFile(int fd)
{
    size_t dummy;
    dummy = write(fd, "tsc\n", strlen("tsc\n"));
    (void)dummy;
}
void CreateCpuCacheAssoc(int fd, int cpu, int index)
{
    size_t dummy;
    char tmp[64];
    sprintf(tmp, "%d\n", (index>=3)?16:8);  // Random be coherent values...
    dummy = write(fd, tmp, strlen(tmp));
    (void)dummy;
}
void CreateCpuCacheCoher(int fd, int cpu, int index)
{
    size_t dummy;
    char tmp[64];
    sprintf(tmp, "%d\n", 64);  // Random be coherent values...
    dummy = write(fd, tmp, strlen(tmp));
    (void)dummy;
}
void CreateCpuCacheSize(int fd, int cpu, int index)
{
    size_t dummy;
    char tmp[64];
    int cachesize = 12288;
    switch(index) {
        case 0: cachesize = 32; break;
        case 1: cachesize = 32; break;
        case 2: cachesize = 256; break;
    }
    sprintf(tmp, "%dK\n", cachesize);  // Random be coherent values...
    dummy = write(fd, tmp, strlen(tmp));
    (void)dummy;
}

#ifdef ANDROID
static int shm_open(const char *name, int oflag, mode_t mode) {
    return -1;
}
static int shm_unlink(const char *name) {
    return -1;
}
#endif

#define TMP_CPUINFO "box64_tmpcpuinfo"
#define TMP_CPUTOPO "box64_tmpcputopo%d"
#define TMP_CLOCKSOURCE "box64_tmpclocksource"
#endif
#define TMP_MEMMAP  "box64_tmpmemmap"
#define TMP_CMDLINE "box64_tmpcmdline"
#define TMP_CPUPRESENT "box64_cpupresent"
#define TMP_CPUCACHE_ASSOC "box64_cpucacheassoc"
#define TMP_CPUCACHE_COHER "box64_cpucachecoher"
#define TMP_CPUCACHE_SIZE "box64_cpucachesize"
EXPORT int32_t my_open(x64emu_t* emu, void* pathname, int32_t flags, uint32_t mode)
{
    if(isProcSelf((const char*) pathname, "cmdline")) {
        // special case for self command line...
        #if 0
        char tmpcmdline[200] = {0};
        char tmpbuff[100] = {0};
        sprintf(tmpbuff, "%s/cmdlineXXXXXX", getenv("TMP")?getenv("TMP"):".");
        int tmp = mkstemp(tmpbuff);
        int dummy;
        if(tmp<0) return open(pathname, flags, mode);
        dummy = write(tmp, emu->context->fullpath, strlen(emu->context->fullpath)+1);
        for (int i=1; i<emu->context->argc; ++i)
            dummy = write(tmp, emu->context->argv[i], strlen(emu->context->argv[i])+1);
        lseek(tmp, 0, SEEK_SET);
        #else
        int tmp = shm_open(TMP_CMDLINE, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return open(pathname, flags, mode);
        shm_unlink(TMP_CMDLINE);    // remove the shm file, but it will still exist because it's currently in use
        int dummy = write(tmp, emu->context->fullpath, strlen(emu->context->fullpath)+1);
        (void)dummy;
        for (int i=1; i<emu->context->argc; ++i)
            if(emu->context->argv[i])
                dummy = write(tmp, emu->context->argv[i], strlen(emu->context->argv[i])+1);
        lseek(tmp, 0, SEEK_SET);
        #endif
        return tmp;
    }
    if(isProcSelf((const char*)pathname, "exe")) {
        return open(emu->context->fullpath, flags, mode);
    }
    #ifndef NOALIGN
    if(strcmp((const char*)pathname, "/proc/cpuinfo")==0) {
        // special case for cpuinfo
        int tmp = shm_open(TMP_CPUINFO, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return open(pathname, flags, mode); // error fallback
        shm_unlink(TMP_CPUINFO);    // remove the shm file, but it will still exist because it's currently in use
        CreateCPUInfoFile(tmp);
        lseek(tmp, 0, SEEK_SET);
        return tmp;
    }
    if(!strcmp((const char*)pathname, "/sys/bus/clocksource/devices/clocksource0/current_clocksource")) {
        // special case to say tsc as current clocksource
        int tmp = shm_open(TMP_CLOCKSOURCE, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return open(pathname, flags, mode); // error fallback
        shm_unlink(TMP_CLOCKSOURCE);    // remove the shm file, but it will still exist because it's currently in use
        CreateClocksourceFile(tmp);
        lseek(tmp, 0, SEEK_SET);
        return tmp;
    }
    int cpu, index;
    if(isSysCpuCache(pathname, "ways_of_associativity", &cpu, &index) && !FileExist(pathname, IS_FILE)) {
        // Create a dummy one
        int tmp = shm_open(TMP_CPUCACHE_ASSOC, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return open(pathname, flags, mode); // error fallback
        shm_unlink(TMP_CPUCACHE_ASSOC);    // remove the shm file, but it will still exist because it's currently in use
        CreateCpuCacheAssoc(tmp, cpu, index);
        lseek(tmp, 0, SEEK_SET);
        return tmp;
    }
    if(isSysCpuCache(pathname, "coherency_line_size", &cpu, &index) && !FileExist(pathname, IS_FILE)) {
        // Create a dummy one
        int tmp = shm_open(TMP_CPUCACHE_COHER, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return open(pathname, flags, mode); // error fallback
        shm_unlink(TMP_CPUCACHE_COHER);    // remove the shm file, but it will still exist because it's currently in use
        CreateCpuCacheCoher(tmp, cpu, index);
        lseek(tmp, 0, SEEK_SET);
        return tmp;
    }
    if(isSysCpuCache(pathname, "size", &cpu, &index) && !FileExist(pathname, IS_FILE)) {
        // Create a dummy one
        int tmp = shm_open(TMP_CPUCACHE_SIZE, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return open(pathname, flags, mode); // error fallback
        shm_unlink(TMP_CPUCACHE_SIZE);    // remove the shm file, but it will still exist because it's currently in use
        CreateCpuCacheAssoc(tmp, cpu, index);
        lseek(tmp, 0, SEEK_SET);
        return tmp;
    }
    if(box64_wine && isProcMem(pathname) && (mode&O_WRONLY)) {
        // deny using proc/XX/mem as it messes up with dynarec memory protection & tracking
        return -1;
    }
    #endif

    if (!strcmp((const char*)pathname, "box64-custom-bashrc-file")) {
        int tmp = shm_open("box64-custom-bashrc-file", O_RDWR | O_CREAT, S_IRWXU);
        if (tmp < 0) return open(pathname, flags, mode); // error fallback
        shm_unlink("box64-custom-bashrc-file");
        const char* content = "if [ -f ~/.bashrc ]\nthen\n. ~/.bashrc\nfi\nexport PS1=\"(box64) \"$PS1\nexport BOX64_NOBANNER=1\nexport BOX64_LOG=0\n";
        size_t dummy;
        dummy = write(tmp, content, strlen(content));
        (void)dummy;
        lseek(tmp, 0, SEEK_SET);
        return tmp;
    }

    int ret = open(pathname, flags, mode);
    return ret;
}
EXPORT int32_t my___open(x64emu_t* emu, void* pathname, int32_t flags, uint32_t mode) __attribute__((alias("my_open")));

//#ifdef DYNAREC
//static int hasDBFromAddress(uintptr_t addr)
//{
//    int idx = (addr>>DYNAMAP_SHIFT);
//    return getDB(idx)?1:0;
//}
//#endif

//EXPORT int32_t my_read(int fd, void* buf, uint32_t count)
//{
//    int ret = read(fd, buf, count);
//#ifdef DYNAREC
//    if(ret!=count && ret>0) {
//        // continue reading...
//        void* p = buf+ret;
//        if(hasDBFromAddress((uintptr_t)p)) {
//            // allow writing the whole block (this happens with HalfLife, libMiles load code directly from .mix and other file like that)
//            unprotectDB((uintptr_t)p, count-ret, 1);
//            int l;
//            do {
//                l = read(fd, p, count-ret);
//                if(l>0) {
//                    p+=l; ret+=l;
//                }
//            } while(l>0);
//        }
//    }
//#endif
//    return ret;
//}

EXPORT int32_t my_open64(x64emu_t* emu, void* pathname, int32_t flags, uint32_t mode)
{
    if(isProcSelf((const char*)pathname, "cmdline")) {
        // special case for self command line...
        #if 0
        char tmpcmdline[200] = {0};
        char tmpbuff[100] = {0};
        sprintf(tmpbuff, "%s/cmdlineXXXXXX", getenv("TMP")?getenv("TMP"):".");
        int tmp = mkstemp64(tmpbuff);
        int dummy;
        if(tmp<0) return open64(pathname, flags, mode);
        dummy = write(tmp, emu->context->fullpath, strlen(emu->context->fullpath)+1);
        for (int i=1; i<emu->context->argc; ++i)
            dummy = write(tmp, emu->context->argv[i], strlen(emu->context->argv[i])+1);
        lseek64(tmp, 0, SEEK_SET);
        #else
        int tmp = shm_open(TMP_CMDLINE, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return open64(pathname, flags, mode);
        shm_unlink(TMP_CMDLINE);    // remove the shm file, but it will still exist because it's currently in use
        int dummy = write(tmp, emu->context->fullpath, strlen(emu->context->fullpath)+1);
        (void)dummy;
        for (int i=1; i<emu->context->argc; ++i)
            if(emu->context->argv[i])
                dummy = write(tmp, emu->context->argv[i], strlen(emu->context->argv[i])+1);
        lseek(tmp, 0, SEEK_SET);
        #endif
        return tmp;
    }
    if(isProcSelf((const char*)pathname, "exe")) {
        return open64(emu->context->fullpath, flags, mode);
    }
    #ifndef NOALIGN
    if(strcmp((const char*)pathname, "/proc/cpuinfo")==0) {
        // special case for cpuinfo
        int tmp = shm_open(TMP_CPUINFO, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return open64(pathname, flags, mode); // error fallback
        shm_unlink(TMP_CPUINFO);    // remove the shm file, but it will still exist because it's currently in use
        CreateCPUInfoFile(tmp);
        lseek(tmp, 0, SEEK_SET);
        return tmp;
    }
    if(BOX64ENV(maxcpu) && (!strcmp(pathname, "/sys/devices/system/cpu/present") || !strcmp(pathname, "/sys/devices/system/cpu/online")) && (getNCpu()>=BOX64ENV(maxcpu))) {
        // special case for cpu present (to limit to 64 cores)
        int tmp = shm_open(TMP_CPUPRESENT, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return open64(pathname, mode); // error fallback
        shm_unlink(TMP_CPUPRESENT);    // remove the shm file, but it will still exist because it's currently in use
        CreateCPUPresentFile(tmp);
        lseek(tmp, 0, SEEK_SET);
        return tmp;
    }
    if(!strcmp((const char*)pathname, "/sys/bus/clocksource/devices/clocksource0/current_clocksource")) {
        // special case to say tsc as current clocksource
        int tmp = shm_open(TMP_CLOCKSOURCE, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return open64(pathname, flags, mode); // error fallback
        shm_unlink(TMP_CLOCKSOURCE);    // remove the shm file, but it will still exist because it's currently in use
        CreateClocksourceFile(tmp);
        lseek(tmp, 0, SEEK_SET);
        return tmp;
    }
    int cpu, index;
    if(isSysCpuCache(pathname, "ways_of_associativity", &cpu, &index) && !FileExist(pathname, IS_FILE)) {
        // Create a dummy one
        int tmp = shm_open(TMP_CPUCACHE_ASSOC, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return open(pathname, flags, mode); // error fallback
        shm_unlink(TMP_CPUCACHE_ASSOC);    // remove the shm file, but it will still exist because it's currently in use
        CreateCpuCacheAssoc(tmp, cpu, index);
        lseek(tmp, 0, SEEK_SET);
        return tmp;
    }
    if(isSysCpuCache(pathname, "coherency_line_size", &cpu, &index) && !FileExist(pathname, IS_FILE)) {
        // Create a dummy one
        int tmp = shm_open(TMP_CPUCACHE_COHER, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return open(pathname, flags, mode); // error fallback
        shm_unlink(TMP_CPUCACHE_COHER);    // remove the shm file, but it will still exist because it's currently in use
        CreateCpuCacheCoher(tmp, cpu, index);
        lseek(tmp, 0, SEEK_SET);
        return tmp;
    }
    if(isSysCpuCache(pathname, "size", &cpu, &index) && !FileExist(pathname, IS_FILE)) {
        // Create a dummy one
        int tmp = shm_open(TMP_CPUCACHE_SIZE, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return open(pathname, flags, mode); // error fallback
        shm_unlink(TMP_CPUCACHE_SIZE);    // remove the shm file, but it will still exist because it's currently in use
        CreateCpuCacheAssoc(tmp, cpu, index);
        lseek(tmp, 0, SEEK_SET);
        return tmp;
    }
    #endif
    return open64(pathname, flags, mode);
}

EXPORT FILE* my_fopen64(x64emu_t* emu, const char* path, const char* mode)
{
    if(isProcSelf(path, "maps")) {
        // special case for self memory map
        int tmp = shm_open(TMP_MEMMAP, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return fopen64(path, mode); // error fallback
        shm_unlink(TMP_MEMMAP);    // remove the shm file, but it will still exist because it's currently in use
        CreateMemorymapFile(emu->context, tmp);
        lseek(tmp, 0, SEEK_SET);
        return fdopen(tmp, mode);
    }
    #ifndef NOALIGN
    if(strcmp(path, "/proc/cpuinfo")==0) {
        // special case for cpuinfo
        int tmp = shm_open(TMP_CPUINFO, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return fopen64(path, mode); // error fallback
        shm_unlink(TMP_CPUINFO);    // remove the shm file, but it will still exist because it's currently in use
        CreateCPUInfoFile(tmp);
        lseek(tmp, 0, SEEK_SET);
        return fdopen(tmp, mode);
    }
    if(BOX64ENV(maxcpu) && (!strcmp(path, "/sys/devices/system/cpu/present") || !strcmp(path, "/sys/devices/system/cpu/online")) && (getNCpu()>=BOX64ENV(maxcpu))) {
        // special case for cpu present (to limit to 64 cores)
        int tmp = shm_open(TMP_CPUPRESENT, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return fopen64(path, mode); // error fallback
        shm_unlink(TMP_CPUPRESENT);    // remove the shm file, but it will still exist because it's currently in use
        CreateCPUPresentFile(tmp);
        lseek(tmp, 0, SEEK_SET);
        return fdopen(tmp, mode);
    }
    if(strcmp(path, "/sys/bus/clocksource/devices/clocksource0/current_clocksource")==0) {
        // special case to say tsc as current clocksource
        int tmp = shm_open(TMP_CLOCKSOURCE, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return fopen64(path, mode); // error fallback
        shm_unlink(TMP_CLOCKSOURCE);    // remove the shm file, but it will still exist because it's currently in use
        CreateClocksourceFile(tmp);
        lseek(tmp, 0, SEEK_SET);
        return fdopen(tmp, mode);
    }
    int cpu=0, index=0;
    if(isSysCpuCache(path, "ways_of_associativity", &cpu, &index) && !FileExist(path, IS_FILE)) {
        // Create a dummy one
        int tmp = shm_open(TMP_CPUCACHE_ASSOC, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return fopen64(path, mode); // error fallback
        shm_unlink(TMP_CPUCACHE_ASSOC);    // remove the shm file, but it will still exist because it's currently in use
        CreateCpuCacheAssoc(tmp, cpu, index);
        lseek(tmp, 0, SEEK_SET);
        return fdopen(tmp, mode);
    }
    if(isSysCpuCache(path, "coherency_line_size", &cpu, &index) && !FileExist(path, IS_FILE)) {
        // Create a dummy one
        int tmp = shm_open(TMP_CPUCACHE_COHER, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return fopen64(path, mode); // error fallback
        shm_unlink(TMP_CPUCACHE_COHER);    // remove the shm file, but it will still exist because it's currently in use
        CreateCpuCacheCoher(tmp, cpu, index);
        lseek(tmp, 0, SEEK_SET);
        return fdopen(tmp, mode);
    }
    if(isSysCpuCache(path, "size", &cpu, &index) && !FileExist(path, IS_FILE)) {
        // Create a dummy one
        int tmp = shm_open(TMP_CPUCACHE_SIZE, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return fopen64(path, mode); // error fallback
        shm_unlink(TMP_CPUCACHE_SIZE);    // remove the shm file, but it will still exist because it's currently in use
        CreateCpuCacheAssoc(tmp, cpu, index);
        lseek(tmp, 0, SEEK_SET);
        return fdopen(tmp, mode);
    }
    #endif
    if(isProcSelf(path, "exe")) {
        return fopen64(emu->context->fullpath, mode);
    }
    return fopen64(path, mode);
}
EXPORT FILE* my_fopen(x64emu_t* emu, const char* path, const char* mode) __attribute__((alias("my_fopen64")));

#if 0
EXPORT int32_t my_ftw(x64emu_t* emu, void* pathname, void* B, int32_t nopenfd)
{
    static iFppi_t f = NULL;
    if(!f) {
        library_t* lib = my_lib;
        if(!lib) return 0;
        f = (iFppi_t)dlsym(lib->w.lib, "ftw");
    }

    return f(pathname, findftwFct(B), nopenfd);
}

#endif

#ifndef NOALIGN
EXPORT int32_t my_epoll_ctl(x64emu_t* emu, int32_t epfd, int32_t op, int32_t fd, void* event)
{
    struct epoll_event _event[1] = {0};
    if(event && (op!=EPOLL_CTL_DEL))
        AlignEpollEvent(_event, event, 1);
    return epoll_ctl(epfd, op, fd, event?_event:NULL);
}
EXPORT int32_t my_epoll_wait(x64emu_t* emu, int32_t epfd, void* events, int32_t maxevents, int32_t timeout)
{
    struct epoll_event _events[maxevents];
    //AlignEpollEvent(_events, events, maxevents);
    int32_t ret = epoll_wait(epfd, events?_events:NULL, maxevents, timeout);
    if(ret>0)
        UnalignEpollEvent(events, _events, ret);
    return ret;
}
EXPORT int32_t my_epoll_pwait(x64emu_t* emu, int32_t epfd, void* events, int32_t maxevents, int32_t timeout, const sigset_t *sigmask)
{
    struct epoll_event _events[maxevents];
    //AlignEpollEvent(_events, events, maxevents);
    int32_t ret = epoll_pwait(epfd, events?_events:NULL, maxevents, timeout, sigmask);
    if(ret>0)
        UnalignEpollEvent(events, _events, ret);
    return ret;
}
EXPORT int my_epoll_pwait2(x64emu_t* emu, int epfd, void* events, int maxevents, struct timespec *timeout, sigset_t * sigmask)
{
    struct epoll_event _events[maxevents];
    //AlignEpollEvent(_events, events, maxevents);
    int ret = 0;
    if(!my->epoll_pwait2) {
        // epoll_pwait2 doesn't exist, to tranforming timeout to int, and from nanosecods to milliseconds...
        int tout = -1;
        if(timeout) {
            int64_t tmp = (timeout->tv_nsec + timeout->tv_sec*1000000000LL)/1000000LL;
            if(tmp>1<<31) tmp = 1<<31;
            tout = tmp;
        }
        ret = epoll_pwait(epfd, events?_events:NULL, maxevents, tout, sigmask);
    } else
        ret = my->epoll_pwait2(epfd, events?_events:NULL, maxevents, timeout, sigmask);
    if(ret>0)
        UnalignEpollEvent(events, _events, ret);
    return ret;
}
#endif

#ifndef ANDROID
EXPORT int32_t my_glob64(x64emu_t *emu, void* pat, int32_t flags, void* errfnc, void* pglob)
{
    (void)emu;
    return glob64(pat, flags, findgloberrFct(errfnc), pglob);
}
EXPORT int32_t my_glob(x64emu_t *emu, void* pat, int32_t flags, void* errfnc, void* pglob) __attribute__((alias("my_glob64")));
#endif

EXPORT int my_scandir64(x64emu_t *emu, void* dir, void* namelist, void* sel, void* comp)
{
    (void)emu;
    return scandir64(dir, namelist, findfilter64Fct(sel), findcompare64Fct(comp));
}
EXPORT int my_scandir(x64emu_t *emu, void* dir, void* namelist, void* sel, void* comp) __attribute__((alias("my_scandir64")));

EXPORT int my_scandirat(x64emu_t *emu, int dirfd, void* dirp, void* namelist, void* sel, void* comp)
{
    (void)emu;
    return scandirat(dirfd, dirp, namelist, findfilter64Fct(sel), findcompare64Fct(comp));
}

EXPORT int my_ftw64(x64emu_t* emu, void* filename, void* func, int descriptors)
{
    (void)emu;
    return ftw64(filename, findftw64Fct(func), descriptors);
}
EXPORT int my_ftw(x64emu_t* emu, void* filename, void* func, int descriptors) __attribute__((alias("my_ftw64")));

EXPORT int32_t my_nftw64(x64emu_t* emu, void* pathname, void* B, int32_t nopenfd, int32_t flags)
{
    (void)emu;
    return nftw64(pathname, findnftw64Fct(B), nopenfd, flags);
}
EXPORT int my_nftw(x64emu_t* emu, void* pathname, void* B, int32_t nopenfd, int32_t flags) __attribute__((alias("my_nftw64")));

EXPORT char** my_environ = NULL;
EXPORT char** my__environ = NULL;
EXPORT char** my___environ = NULL;  // all aliases

EXPORT int32_t my_execv(x64emu_t* emu, const char* path, char* const argv[])
{
    int self = isProcSelf(path, "exe");
    int x64 = FileIsX64ELF(path);
    int x86 = my_context->box86path?FileIsX86ELF(path):0;
    int script = (my_context->bashpath && FileIsShell(path))?1:0;
    printf_log(LOG_DEBUG, "execv(\"%s\", %p) is x64=%d x86=%d script=%d self=%d\n", path, argv, x64, x86, script, self);
    #if 1
    if (x64 || x86 || script || self) {
        int skip_first = 0;
        if(strlen(path)>=strlen("wine64-preloader") && strcmp(path+strlen(path)-strlen("wine64-preloader"), "wine64-preloader")==0)
            skip_first++;
        // count argv...
        int n=skip_first;
        while(argv[n]) ++n;
        int toadd = script?2:1;
        const char** newargv = (const char**)box_calloc(n+toadd+2, sizeof(char*));
        newargv[0] = x86?emu->context->box86path:emu->context->box64path;
        if(script) newargv[1] = emu->context->bashpath; // script needs to be launched with bash
        memcpy(newargv+toadd, argv+skip_first, sizeof(char*)*(n+1-skip_first));
        if(self)
            newargv[1] = emu->context->fullpath;
        else {
            // TODO check if envp is not environ and add the value on a copy
            if(strcmp(newargv[toadd], skip_first?argv[skip_first]:path))
                setenv(x86?"BOX86_ARG0":"BOX64_ARG0", newargv[toadd], 1);
            newargv[toadd] = skip_first?argv[skip_first]:path;
        }
        printf_log(LOG_DEBUG, " => execv(\"%s\", %p [\"%s\", \"%s\", \"%s\"...:%d])\n", newargv[0], newargv, newargv[0], n?newargv[1]:"", (n>1)?newargv[2]:"",n);
        char** envv = NULL;
        if(my_environ!=my_context->envv) envv = my_environ;
        if(my__environ!=my_context->envv) envv = my__environ;
        if(my___environ!=my_context->envv) envv = my___environ;

        int ret;
        if(envv)
            ret = execve(newargv[0], (char* const*)newargv, envv);
        else
            ret = execv(newargv[0], (char* const*)newargv);
        box_free(newargv);
        return ret;
    }
    #endif
    return execv(path, argv);
}

EXPORT int32_t my_execve(x64emu_t* emu, const char* path, char* const argv[], char* const envp[])
{
    int self = isProcSelf(path, "exe");
    int x64 = FileIsX64ELF(path);
    int x86 = my_context->box86path?FileIsX86ELF(path):0;
    int script = (my_context->bashpath && FileIsShell(path))?1:0;
    printf_log(LOG_DEBUG, "execve(\"%s\", %p[\"%s\", \"%s\", \"%s\"...], %p) is x64=%d x86=%d script=%d (my_context->envv=%p, environ=%p\n", path, argv, argv[0], argv[1]?argv[1]:"(nil)", argv[2]?argv[2]:"(nil)", envp, x64, x86, script, my_context->envv, environ);
    // hack to update the environ var if needed
    if(envp == my_context->envv && environ) {
        envp = environ;
    }
    #if 1
    if (x64 || x86 || self || script) {
        int skip_first = 0;
        if(strlen(path)>=strlen("wine64-preloader") && strcmp(path+strlen(path)-strlen("wine64-preloader"), "wine64-preloader")==0)
            skip_first++;
        // count argv...
        int n=skip_first;
        while(argv[n]) ++n;
        int toadd = script?2:1;
        const char** newargv = (const char**)alloca((n+1+toadd-skip_first)*sizeof(char*));
        memset(newargv, 0, (n+1+toadd)*sizeof(char*));
        newargv[0] = x86?emu->context->box86path:emu->context->box64path;
        if(script) newargv[1] = emu->context->bashpath; // script needs to be launched with bash
        memcpy(newargv+toadd, argv+skip_first, sizeof(char*)*(n+1-skip_first));
        if(self) newargv[toadd] = emu->context->fullpath;
        else {
            // TODO check if envp is not environ and add the value on a copy
            if(strcmp(newargv[toadd], path))
                setenv(x86?"BOX86_ARG0":"BOX64_ARG0", newargv[toadd], 1);
            newargv[toadd] = path;
        }
        printf_log(LOG_DEBUG, " => execve(\"%s\", %p [\"%s\", \"%s\", \"%s\"...:%d], %p)\n", newargv[0], newargv, newargv[0], (n+toadd-skip_first)?newargv[1]:"", ((n+toadd-skip_first)>1)?newargv[2]:"",n, envp);
        int ret = execve(newargv[0], (char* const*)newargv, envp);
        return ret;
    }
    #endif
    if(!strcmp(path + strlen(path) - strlen("/uname"), "/uname")
     && argv[1] && (!strcmp(argv[1], "-m") || !strcmp(argv[1], "-p") || !strcmp(argv[1], "-i"))
     && !argv[2]) {
        // uname -m is redirected to box64 -m
        path = my_context->box64path;
        char *argv2[3] = { my_context->box64path, argv[1], NULL };
        return execve(path, argv2, envp);
    }
    #ifndef NOALIGN
    if(!strcmp(path + strlen(path) - strlen("/grep"), "/grep")
    && argv[1] && argv[2] && (!strcmp(argv[2], "/proc/cpuinfo") || (argv[1][1]=='-' && argv[3] && !strcmp(argv[3], "/proc/cpuinfo")))) {
        // special case of a bash script shell running grep on cpuinfo to extract capacities...
        int cpuinfo = strcmp(argv[2], "/proc/cpuinfo")?3:2;
        int n=0;
        while(argv[n]) ++n;
        const char** newargv = (const char**)alloca((n+1)*sizeof(char*));
        memcpy(newargv, argv, sizeof(char*)*(n+1));
        // create a dummy cpuinfo in temp (that will stay there, sorry)
        const char* tmpdir = GetTmpDir();
        char template[100] = {0};
        sprintf(template, "%s/box64cpuinfoXXXXXX", tmpdir);
        int fd = mkstemp(template);
        CreateCPUInfoFile(fd);
        // get back the name
        char cpuinfo_file[100] = {0};
        sprintf(template, "/proc/self/fd/%d", fd);
        int rl = readlink(template, cpuinfo_file, sizeof(cpuinfo_file));
        close(fd);
        chmod(cpuinfo_file, 0666);
        newargv[cpuinfo] = cpuinfo_file;
        printf_log(LOG_DEBUG, " => execve(\"%s\", %p [\"%s\", \"%s\", \"%s\"...:%d], %p)\n", path, newargv, newargv[0], newargv[1], newargv[2],n, envp);
        int ret = execve(path, (char* const*)newargv, envp);
        return ret;
    }
    if(!strcmp(path + strlen(path) - strlen("/cat"), "/cat")
    && argv[1] && !strcmp(argv[1], "/proc/cpuinfo")) {
        // special case of a bash script shell running grep on cpuinfo to extract capacities...
        int cpuinfo = 1;
        int n=0;
        while(argv[n]) ++n;
        const char** newargv = (const char**)alloca((n+1)*sizeof(char*));
        memcpy(newargv, argv, sizeof(char*)*(n+1));
        // create a dummy cpuinfo in temp (that will stay there, sorry)
        const char* tmpdir = GetTmpDir();
        char template[100] = {0};
        sprintf(template, "%s/box64cpuinfoXXXXXX", tmpdir);
        int fd = mkstemp(template);
        CreateCPUInfoFile(fd);
        // get back the name
        char cpuinfo_file[100] = {0};
        sprintf(template, "/proc/self/fd/%d", fd);
        int rl = readlink(template, cpuinfo_file, sizeof(cpuinfo_file));
        close(fd);
        chmod(cpuinfo_file, 0666);
        newargv[cpuinfo] = cpuinfo_file;
        printf_log(LOG_DEBUG, " => execve(\"%s\", %p [\"%s\", \"%s\", \"%s\"...:%d], %p)\n", path, newargv, newargv[0], newargv[1], newargv[2],n, envp);
        int ret = execve(path, (char* const*)newargv, envp);
        return ret;
    }
    /*if(!strcmp(path + strlen(path) - strlen("/bwrap"), "/bwrap")) {
        printf_log(LOG_NONE, "\n\n*********\n\nCalling bwrap!\n\n**********\n\n");
    }*/
    #endif

    return execve(path, argv, envp);
}

// execvp should use PATH to search for the program first
EXPORT int32_t my_execvp(x64emu_t* emu, const char* path, char* const argv[])
{
    // need to use BOX64_PATH / PATH here...
    char* fullpath = ResolveFileSoft(path, &my_context->box64_path);
    // use fullpath...
    int self = isProcSelf(fullpath, "exe");
    int x64 = FileIsX64ELF(fullpath);
    int x86 = my_context->box86path?FileIsX86ELF(fullpath):0;
    int script = (my_context->bashpath && FileIsShell(fullpath))?1:0;
    printf_log(LOG_DEBUG, "execvp(\"%s\", %p), IsX86=%d / fullpath=\"%s\"\n", path, argv, x64, fullpath);
    if (x64 || x86 || script || self) {
        // count argv...
        int i=0;
        while(argv[i]) ++i;
        int toadd = script?2:1;
        char** newargv = (char**)alloca((i+toadd+1)*sizeof(char*));
        memset(newargv, 0, (i+toadd+1)*sizeof(char*));
        newargv[0] = x86?emu->context->box86path:emu->context->box64path;
        if(script) newargv[1] = emu->context->bashpath; // script needs to be launched with bash
        for (int j=0; j<i; ++j)
            newargv[j+toadd] = argv[j];
        if(self) newargv[1] = emu->context->fullpath;
        //else if(script) newargv[2] = fullpath;
        else {
            // TODO check if envp is not environ and add the value on a copy
            if(strcmp(newargv[toadd], path))
                setenv(x86?"BOX86_ARG0":"BOX64_ARG0", newargv[toadd], 1);
            newargv[toadd] = fullpath;
        }

        printf_log(LOG_DEBUG, " => execvp(\"%s\", %p [\"%s\", \"%s\"...:%d])\n", newargv[0], newargv, newargv[1], i?newargv[2]:"", i);
        char** envv = NULL;
        if(my_environ!=my_context->envv) envv = my_environ;
        if(my__environ!=my_context->envv) envv = my__environ;
        if(my___environ!=my_context->envv) envv = my___environ;
        int ret;
        if(envv)
            ret = execvpe(newargv[0], newargv, envv);
        else
            ret = execvp(newargv[0], newargv);
        box_free(fullpath);
        return ret;
    }
    if((!strcmp(path + strlen(path) - strlen("/uname"), "/uname") || !strcmp(path, "uname"))
     && argv[1] && (!strcmp(argv[1], "-m") || !strcmp(argv[1], "-p") || !strcmp(argv[1], "-i"))
     && !argv[2]) {
        // uname -m is redirected to box64 -m
        path = my_context->box64path;
        char *argv2[3] = { my_context->box64path, argv[1], NULL };

        return execvp(path, argv2);
    }

    // fullpath is gone, so the search will only be on PATH, not on BOX64_PATH (is that an issue?)
    return execvp(path, argv);
}
// execvp should use PATH to search for the program first
EXPORT int32_t my_execvpe(x64emu_t* emu, const char* path, char* const argv[], char* const envp[])
{
    // need to use BOX64_PATH / PATH here...
    char* fullpath = ResolveFileSoft(path, &my_context->box64_path);
    // use fullpath...
    int self = isProcSelf(fullpath, "exe");
    int x64 = FileIsX64ELF(fullpath);
    int x86 = my_context->box86path?FileIsX86ELF(fullpath):0;
    int script = (my_context->bashpath && FileIsShell(fullpath))?1:0;
    printf_log(LOG_DEBUG, "execvpe(\"%s\", %p, %p), IsX86=%d / fullpath=\"%s\"\n", path, argv, envp, x64, fullpath);
    // hack to update the environ var if needed
    if(envp == my_context->envv && environ) {
        envp = environ;
    }
    if (x64 || x86 || script || self) {
        // count argv...
        int i=0;
        while(argv[i]) ++i;
        int toadd = script?2:1;
        char** newargv = (char**)alloca((i+toadd+1)*sizeof(char*));
        memset(newargv, 0, (i+toadd+1)*sizeof(char*));
        newargv[0] = x86?emu->context->box86path:emu->context->box64path;
        if(script) newargv[1] = emu->context->bashpath; // script needs to be launched with bash
        for (int j=0; j<i; ++j)
            newargv[j+toadd] = argv[j];
        if(self) newargv[1] = emu->context->fullpath;
        //else if(script) newargv[2] = fullpath;
        else {
            // TODO check if envp is not environ and add the value on a copy
            if(strcmp(newargv[toadd], path))
                setenv(x86?"BOX86_ARG0":"BOX64_ARG0", newargv[toadd], 1);
            newargv[toadd] = fullpath;
        }

        printf_log(LOG_DEBUG, " => execvp(\"%s\", %p [\"%s\", \"%s\"...:%d])\n", newargv[0], newargv, newargv[1], i?newargv[2]:"", i);
        int ret;
        ret = execvpe(newargv[0], newargv, envp);
        box_free(fullpath);
        return ret;
    }
    if((!strcmp(path + strlen(path) - strlen("/uname"), "/uname") || !strcmp(path, "uname"))
     && argv[1] && (!strcmp(argv[1], "-m") || !strcmp(argv[1], "-p") || !strcmp(argv[1], "-i"))
     && !argv[2]) {
        // uname -m is redirected to box64 -m
        path = my_context->box64path;
        char *argv2[3] = { my_context->box64path, argv[1], NULL };

        return execvpe(path, argv2, envp);
    }

    // fullpath is gone, so the search will only be on PATH, not on BOX64_PATH (is that an issue?)
    return execvpe(path, argv, envp);
}

EXPORT int32_t my_execl(x64emu_t* emu, const char* path)
{
    int self = isProcSelf(path, "exe");
    int x64 = FileIsX64ELF(path);
    int x86 = my_context->box86path?FileIsX86ELF(path):0;
    int script = (my_context->bashpath && FileIsShell(path))?1:0;
    printf_log(LOG_DEBUG, "execl(\"%s\", ...), IsX86=%d, self=%d\n", path, x64, self);
    // count argv...
    int i=0;
    while(getVargN(emu, i+1)) ++i;
    int toadd = script?2:((x64||self)?1:0);
    char** newargv = (char**)box_calloc(i+toadd+1, sizeof(char*));
    int j=0;
    if ((x64 || x86 || script || self))
        newargv[j++] = x86?emu->context->box86path:emu->context->box64path;
    if(script) newargv[j++] = emu->context->bashpath;
    for (int k=0; k<i; ++k)
        newargv[j++] = getVargN(emu, k+1);
    if(self) newargv[1] = emu->context->fullpath;
    printf_log(LOG_DEBUG, " => execl(\"%s\", %p [\"%s\", \"%s\"...:%d])\n", newargv[0], newargv, newargv[1], i?newargv[2]:"", i);
    int ret = 0;
    if (!(x64 || x86 || script || self)) {
        ret = execv(path, newargv);
    } else {
        ret = execv(newargv[0], newargv);
    }
    box_free(newargv);
    return ret;
}

EXPORT int32_t my_execle(x64emu_t* emu, const char* path)
{
    int self = isProcSelf(path, "exe");
    int x64 = FileIsX64ELF(path);
    int x86 = my_context->box86path?FileIsX86ELF(path):0;
    int script = (my_context->bashpath && FileIsShell(path))?1:0;
    printf_log(LOG_DEBUG, "execle(\"%s\", ...), IsX86=%d, self=%d\n", path, x64, self);
    // hack to update the environ var if needed
    // count argv...
    int i=0;
    while(getVargN(emu, i+1)) ++i;
    int toadd = script?2:((x64||self)?1:0);
    char** newargv = (char**)box_calloc(i+toadd+1, sizeof(char*));
    char** envp = (char**)getVargN(emu, i+2);
    if(envp == my_context->envv && environ) {
        envp = environ;
    }
    int j=0;
    if ((x64 || x86 || script || self))
        newargv[j++] = x86?emu->context->box86path:emu->context->box64path;
    if(script) newargv[j++] = emu->context->bashpath;
    for (int k=0; k<i; ++k)
        newargv[j++] = getVargN(emu, k+1);
    if(self) newargv[1] = emu->context->fullpath;
    printf_log(LOG_DEBUG, " => execle(\"%s\", %p [\"%s\", \"%s\"...:%d], %p)\n", newargv[0], newargv, newargv[1], i?newargv[2]:"", i, envp);
    int ret = execve(newargv[0], newargv, envp);
    box_free(newargv);
    return ret;
}

EXPORT int32_t my_execlp(x64emu_t* emu, const char* path)
{
    // need to use BOX64_PATH / PATH here...
    char* fullpath = ResolveFileSoft(path, &my_context->box64_path);
    // use fullpath...
    int self = isProcSelf(fullpath, "exe");
    int x64 = FileIsX64ELF(fullpath);
    int x86 = my_context->box86path?FileIsX86ELF(fullpath):0;
    int script = (my_context->bashpath && FileIsShell(fullpath))?1:0;
    printf_log(LOG_DEBUG, "execlp(\"%s\", ...), IsX86=%d / fullpath=\"%s\"\n", path, x64, fullpath);
    // count argv...
    int i=0;
    while(getVargN(emu, i+1)) ++i;
    int toadd = script?2:((x64||self)?1:0);
    char** newargv = (char**)box_calloc(i+toadd+1, sizeof(char*));
    int j=0;
    if ((x64 || x86 || script || self))
        newargv[j++] = x86?emu->context->box86path:emu->context->box64path;
    if(script) newargv[j++] = emu->context->bashpath;
    for (int k=0; k<i; ++k)
        newargv[j++] = getVargN(emu, k+1);
    if(self) newargv[1] = emu->context->fullpath;
    if(script) newargv[2] = fullpath;
    printf_log(LOG_DEBUG, " => execlp(\"%s\", %p [\"%s\", \"%s\"...:%d])\n", newargv[0], newargv, newargv[1], i?newargv[2]:"", i);
    char** envv = NULL;
    if(my_environ!=my_context->envv) envv = my_environ;
    if(my__environ!=my_context->envv) envv = my__environ;
    if(my___environ!=my_context->envv) envv = my___environ;
    int ret;
    if(envv)
        ret = execvpe(newargv[0], newargv, envv);
    else
        ret = execvp(newargv[0], newargv);
    box_free(newargv);
    box_free(fullpath);
    return ret;
}

EXPORT int32_t my_posix_spawn(x64emu_t* emu, pid_t* pid, const char* fullpath,
    const posix_spawn_file_actions_t *actions, const posix_spawnattr_t* attrp,  char* const argv[], char* const envp[])
{
    int self = isProcSelf(fullpath, "exe");
    int x64 = FileIsX64ELF(fullpath);
    int x86 = my_context->box86path?FileIsX86ELF(fullpath):0;
    int script = (my_context->bashpath && FileIsShell(fullpath))?1:0;
    int ret;
    printf_log(/*LOG_DEBUG*/LOG_INFO, "posix_spawn(%p, \"%s\", %p, %p, %p[\"%s\", \"%s\", ...], %p), IsX64=%d, IsX86=%d IsScript=%d %s\n", pid, fullpath, actions, attrp, argv, argv[0], argv[1]?argv[1]:"", envp, x64, x86, script, (envp==my_context->envv)?"envp is context->envv":"");
    // hack to update the environ var if needed
    if(envp == my_context->envv && environ) {
        envp = environ;
    }
    if (x64 || x86 || script || self) {
        int n=1;
        while(argv[n]) ++n;
        int toadd = script?2:1;
        const char** newargv = (const char**)alloca((n+1+toadd)*sizeof(char*));
        memset(newargv, 0, (n+1+toadd)*sizeof(char*));
        newargv[0] = x86?emu->context->box86path:emu->context->box64path;
        if(script) newargv[1] = emu->context->bashpath; // script needs to be launched with bash
        memcpy(newargv+toadd, argv, (n+1)*sizeof(char*));
        if(self) newargv[toadd] = emu->context->fullpath;
        else {
            // TODO check if envp is not environ and add the value on a copy
            if(strcmp(newargv[toadd], fullpath))
                setenv(x86?"BOX86_ARG0":"BOX64_ARG0", newargv[toadd], 1);
            newargv[toadd] = fullpath;
        }
        printf_log(/*LOG_DEBUG*/LOG_INFO, " => posix_spawn(%p, \"%s\", %p, %p, %p [\"%s\", \"%s\", \"%s\"...:%d], %p)\n", pid, newargv[0], actions, attrp, newargv, newargv[0], newargv[1], newargv[2]?newargv[2]:"", n, envp);
        ret = posix_spawn(pid, newargv[0], actions, attrp, (char* const*)newargv, envp);
        printf_log(/*LOG_DEBUG*/LOG_INFO, "posix_spawn returned %d\n", ret);
        //box_free(newargv);
    } else
        ret = posix_spawn(pid, fullpath, actions, attrp, argv, envp);
    return ret;
}

// execvp should use PATH to search for the program first
EXPORT int32_t my_posix_spawnp(x64emu_t* emu, pid_t* pid, const char* path,
    const posix_spawn_file_actions_t *actions, const posix_spawnattr_t* attrp,  char* const argv[], char* const envp[])
{
    // need to use BOX64_PATH / PATH here...
    char* fullpath = ResolveFileSoft(path, &my_context->box64_path);
    // use fullpath...
    int self = isProcSelf(fullpath, "exe");
    int x64 = FileIsX64ELF(fullpath);
    int x86 = my_context->box86path?FileIsX86ELF(path):0;
    int script = (my_context->bashpath && FileIsShell(fullpath))?1:0;
    int ret;
    printf_log(/*LOG_DEBUG*/LOG_INFO, "posix_spawnp(%p, \"%s\", %p, %p, %p, %p), IsX86=%d / fullpath=\"%s\"\n", pid, path, actions, attrp, argv, envp, x64, fullpath);
    // hack to update the environ var if needed
    if(envp == my_context->envv && environ) {
        envp = environ;
    }
    if (x64 || x86 || script || self) {
        int n=1;
        while(argv[n]) ++n;
        int toadd = script?2:1;
        const char** newargv = (const char**)alloca((n+1+toadd)*sizeof(char*));
        memset(newargv, 0, (n+1+toadd)*sizeof(char*));
        newargv[0] = x86?emu->context->box86path:emu->context->box64path;
        if(script) newargv[1] = emu->context->bashpath; // script needs to be launched with bash
        memcpy(newargv+toadd, argv, (n+1)*sizeof(char*));
        if(self) newargv[toadd] = emu->context->fullpath;
        else {
            // TODO check if envp is not environ and add the value on a copy
            if(strcmp(newargv[toadd], fullpath))
                setenv(x86?"BOX86_ARG0":"BOX64_ARG0", newargv[toadd], 1);
            newargv[toadd] = fullpath;
        }
        printf_log(/*LOG_DEBUG*/LOG_INFO, " => posix_spawn(%p, \"%s\", %p, %p, %p [\"%s\", \"%s\", \"%s\"...:%d], %p)\n", pid, newargv[0], actions, attrp, newargv, newargv[0], newargv[1], newargv[2]?newargv[2]:"", n, envp);
        ret = posix_spawn(pid, newargv[0], actions, attrp, (char* const*)newargv, envp);
        printf_log(/*LOG_DEBUG*/LOG_INFO, "posix_spawn returned %d\n", ret);
        //box_free(newargv);
    } else
        ret = posix_spawnp(pid, path, actions, attrp, argv, envp);
    box_free(fullpath);
    return ret;
}

EXPORT void my__Jv_RegisterClasses() {}

EXPORT int32_t my___cxa_thread_atexit_impl(x64emu_t* emu, void* dtor, void* obj, void* dso)
{
    (void)emu;
    //printf_log(LOG_INFO, "Warning, call to __cxa_thread_atexit_impl(%p, %p, %p) ignored\n", dtor, obj, dso);
    AddCleanup1Arg(emu, dtor, obj, FindElfAddress(my_context, (uintptr_t)dso));
    return 0;
}

EXPORT int32_t my_delete_module(x64emu_t* emu, const char* name, unsigned int flags)
{
    (void)emu;
    return syscall(__NR_delete_module, name, flags);
}

EXPORT int32_t my_init_module(x64emu_t* emu, void* module_image, unsigned long size, const char* param_values)
{
    (void)emu;
    return syscall(__NR_init_module, module_image, size, param_values);
}

EXPORT int32_t my___register_atfork(x64emu_t *emu, void* prepare, void* parent, void* child, void* handle)
{
    (void)emu;
    // this is partly incorrect, because the emulated funcionts should be executed by actual fork and not by my_atfork...
    if(my_context->atfork_sz==my_context->atfork_cap) {
        my_context->atfork_cap += 4;
        my_context->atforks = (atfork_fnc_t*)box_realloc(my_context->atforks, my_context->atfork_cap*sizeof(atfork_fnc_t));
    }
    my_context->atforks[my_context->atfork_sz].prepare = (uintptr_t)prepare;
    my_context->atforks[my_context->atfork_sz].parent = (uintptr_t)parent;
    my_context->atforks[my_context->atfork_sz].child = (uintptr_t)child;
    my_context->atforks[my_context->atfork_sz].handle = handle;
    return 0;
}

#if 0
EXPORT uint64_t my___umoddi3(uint64_t a, uint64_t b)
{
    return a%b;
}
EXPORT uint64_t my___udivdi3(uint64_t a, uint64_t b)
{
    return a/b;
}
EXPORT int64_t my___divdi3(int64_t a, int64_t b)
{
    return a/b;
}

EXPORT int32_t my___poll_chk(void* a, uint32_t b, int c, int l)
{
    return poll(a, b, c);   // no check...
}
#endif

EXPORT int32_t my_fcntl64(x64emu_t* emu, int32_t a, int32_t b, void* c)
{
    (void)emu;
    if(b==F_SETFL)
        c = (void*)(uintptr_t)of_convert((intptr_t)c);
    #if 0
    if(b==F_GETLK64 || b==F_SETLK64 || b==F_SETLKW64)
    {
        my_flock64_t fl;
        AlignFlock64(&fl, c);
        int ret = fcntl(a, b, &fl);
        UnalignFlock64(c, &fl);
        return ret;
    }
    #endif
    int ret = fcntl(a, b, c);
    if(b==F_GETFL && ret!=-1)
        ret = of_unconvert(ret);

    return ret;
}

EXPORT int32_t my_fcntl(x64emu_t* emu, int32_t a, int32_t b, void* c)
{
    (void)emu;
    if(b==F_SETFL && (intptr_t)c==0xFFFFF7FF) {
        // special case for ~O_NONBLOCK...
        int flags = fcntl(a, F_GETFL);
        if(flags&O_NONBLOCK) {
            flags &= ~O_NONBLOCK;
            return fcntl(a, b, flags);
        }
        return 0;
    }
    if(b==F_SETFL)
        c = (void*)(uintptr_t)of_convert((intptr_t)c);
    #if 0
    if(b==F_GETLK64 || b==F_SETLK64 || b==F_SETLKW64)
    {
        my_flock64_t fl;
        AlignFlock64(&fl, c);
        int ret = fcntl(a, b, &fl);
        UnalignFlock64(c, &fl);
        return ret;
    }
    #endif
    int ret = fcntl(a, b, c);
    if(b==F_GETFL && ret!=-1)
        ret = of_unconvert(ret);

    return ret;
}
EXPORT int32_t my___fcntl(x64emu_t* emu, int32_t a, int32_t b, void* c) __attribute__((alias("my_fcntl")));

#if 0
EXPORT int32_t my_preadv64(x64emu_t* emu, int32_t fd, void* v, int32_t c, int64_t o)
{
    library_t* lib = my_lib;
    if(!lib) return 0;
    void* f = dlsym(lib->w.lib, "preadv64");
    if(f)
        return ((iFipiI_t)f)(fd, v, c, o);
    return syscall(__NR_preadv, fd, v, c,(uint32_t)(o&0xffffffff), (uint32_t)((o>>32)&0xffffffff));
}

EXPORT int32_t my_pwritev64(x64emu_t* emu, int32_t fd, void* v, int32_t c, int64_t o)
{
    library_t* lib = my_lib;
    if(!lib) return 0;
    void* f = dlsym(lib->w.lib, "pwritev64");
    if(f)
        return ((iFipiI_t)f)(fd, v, c, o);
    #ifdef __arm__
    return syscall(__NR_pwritev, fd, v, c, 0, (uint32_t)(o&0xffffffff), (uint32_t)((o>>32)&0xffffffff));
    // on arm, 64bits args needs to be on even/odd register, so need to put a 0 for aligment
    #else
    return syscall(__NR_pwritev, fd, v, c,(uint32_t)(o&0xffffffff), (uint32_t)((o>>32)&0xffffffff));
    #endif
}

EXPORT int32_t my_accept4(x64emu_t* emu, int32_t fd, void* a, void* l, int32_t flags)
{
    library_t* lib = my_lib;
    if(!lib) return 0;
    void* f = dlsym(lib->w.lib, "accept4");
    if(f)
        return ((iFippi_t)f)(fd, a, l, flags);
    if(!flags)
        return accept(fd, a, l);
    return syscall(__NR_accept4, fd, a, l, flags);
}

EXPORT  int32_t my_fallocate64(int fd, int mode, int64_t offs, int64_t len)
{
    iFiiII_t f = NULL;
    static int done = 0;
    if(!done) {
        library_t* lib = my_lib;
        f = (iFiiII_t)dlsym(lib->w.lib, "fallocate64");
        done = 1;
    }
    if(f)
        return f(fd, mode, offs, len);
    else
        return syscall(__NR_fallocate, fd, mode, (uint32_t)(offs&0xffffffff), (uint32_t)((offs>>32)&0xffffffff), (uint32_t)(len&0xffffffff), (uint32_t)((len>>32)&0xffffffff));
        //return posix_fallocate64(fd, offs, len);
}

EXPORT struct __processor_model
{
  unsigned int __cpu_vendor;
  unsigned int __cpu_type;
  unsigned int __cpu_subtype;
  unsigned int __cpu_features[1];
} my___cpu_model;

#include "cpu_info.h"
void InitCpuModel()
{
    // some pseudo random cpu info...
    my___cpu_model.__cpu_vendor = VENDOR_INTEL;
    my___cpu_model.__cpu_type = INTEL_PENTIUM_M;
    my___cpu_model.__cpu_subtype = 0; // N/A
    my___cpu_model.__cpu_features[0] = (1<<FEATURE_CMOV)
                                     | (1<<FEATURE_MMX)
                                     | (1<<FEATURE_SSE)
                                     | (1<<FEATURE_SSE2)
                                     | (1<<FEATURE_SSE3)
                                     | (1<<FEATURE_SSSE3)
                                     | (1<<FEATURE_MOVBE)
                                     | (1<<FEATURE_ADX);
}
#endif

#ifdef ANDROID
void ctSetup()
{
}
#else
EXPORT const unsigned short int *my___ctype_b;
EXPORT const int32_t *my___ctype_tolower;
EXPORT const int32_t *my___ctype_toupper;

void ctSetup()
{
    my___ctype_b = *(__ctype_b_loc());
    my___ctype_toupper = *(__ctype_toupper_loc());
    my___ctype_tolower = *(__ctype_tolower_loc());
}
#endif

EXPORT void my___register_frame_info(void* a, void* b)
{
    // nothing
    (void)a; (void)b;
}
EXPORT void* my___deregister_frame_info(void* a)
{
    (void)a;
    return NULL;
}

EXPORT void* my____brk_addr = NULL;

void EXPORT my_longjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val)
{
    jump_buff_x64_t *jpbuff = &((__jmp_buf_tag_t*)p)->__jmpbuf;
    //restore  regs
    R_RBX = jpbuff->save_rbx;
    R_RBP = jpbuff->save_rbp;
    R_R12 = jpbuff->save_r12;
    R_R13 = jpbuff->save_r13;
    R_R14 = jpbuff->save_r14;
    R_R15 = jpbuff->save_r15;
    R_RSP = jpbuff->save_rsp;
    // jmp to saved location, plus restore val to rax
    R_RAX = __val;
    R_RIP = jpbuff->save_rip;
    if(((__jmp_buf_tag_t*)p)->__mask_was_saved) {
        sigprocmask(SIG_SETMASK, &((__jmp_buf_tag_t*)p)->__saved_mask, NULL);
    }
    if(emu->flags.quitonlongjmp) {
        emu->flags.longjmp = 1;
        emu->quit = 1;
    }
}

EXPORT int32_t my___sigsetjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int savesigs)
{
    jump_buff_x64_t *jpbuff = &((__jmp_buf_tag_t*)p)->__jmpbuf;
    // save the buffer
    jpbuff->save_rbx = R_RBX;
    jpbuff->save_rbp = R_RBP;
    jpbuff->save_r12 = R_R12;
    jpbuff->save_r13 = R_R13;
    jpbuff->save_r14 = R_R14;
    jpbuff->save_r15 = R_R15;
    jpbuff->save_rsp = R_RSP+sizeof(uintptr_t); // include "return address"
    jpbuff->save_rip = *(uintptr_t*)(R_RSP);
    if(savesigs) {
        if(sigprocmask(SIG_SETMASK, NULL, &((__jmp_buf_tag_t*)p)->__saved_mask))
            ((__jmp_buf_tag_t*)p)->__mask_was_saved = 0;
        else
            ((__jmp_buf_tag_t*)p)->__mask_was_saved = 1;
    } else
        ((__jmp_buf_tag_t*)p)->__mask_was_saved = 0;
    // quit emulation loop and create a new jumpbuf if needed
    if(!emu->flags.jmpbuf_ready) {
        emu->flags.need_jmpbuf = 1;
        emu->quit = 1;
    }
    return 0;
}
EXPORT int32_t my_sigsetjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int savesigs)
{
    return my___sigsetjmp(emu, p, savesigs);
}
EXPORT int32_t my__setjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p)
{
    return  my___sigsetjmp(emu, p, 0);
}
EXPORT int32_t my_setjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p)
{
    return  my___sigsetjmp(emu, p, 1);
}

EXPORT void my___explicit_bzero_chk(x64emu_t* emu, void* dst, uint32_t len, uint32_t dstlen)
{
    (void)emu; (void)dstlen;
    memset(dst, 0, len);
}

EXPORT void* my_realpath(x64emu_t* emu, void* path, void* resolved_path)
{
    if(isProcSelf(path, "exe")) {
        return realpath(emu->context->fullpath, resolved_path);
    }
    return realpath(path, resolved_path);
}

EXPORT int my_readlinkat(x64emu_t* emu, int fd, void* path, void* buf, size_t bufsize)
{
    if((fd==AT_FDCWD) && isProcSelf(path, "exe")) {
        strncpy(buf, my_context->fullpath, bufsize);
        size_t l = strlen(my_context->fullpath);
        return (l>bufsize)?bufsize:(l+1);
    }
    return readlinkat(fd, path, buf, bufsize);
}
extern int have48bits;
void* last_mmap_addr[2] = {0};
size_t last_mmap_len[2] = {0};
int last_mmap_idx = 0;
EXPORT void* my_mmap64(x64emu_t* emu, void *addr, size_t length, int prot, int flags, int fd, ssize_t offset)
{
    (void)emu;
    if(BOX64ENV(dynarec_log)>=LOG_DEBUG) {printf_log(LOG_NONE, "mmap64(%p, 0x%zx, 0x%x, 0x%x, %d, %zd) ", addr, length, prot, flags, fd, offset);}
    void* ret = box_mmap(addr, length, prot, flags, fd, offset);
    int e = errno;
    if(emu && box64_is32bits && ret!=MAP_FAILED && ((ret>(void*)0xc0000000) || (ret+length>(void*)0xc0000000))) {
        // do not allow allocating memory that high for 32bits process
        box_munmap(ret, length);
        ret = MAP_FAILED;
        e = EEXIST;
    }
    if((ret==MAP_FAILED && (emu || box64_is32bits)) && (BOX64ENV(log)>=LOG_DEBUG || BOX64ENV(dynarec_log)>=LOG_DEBUG)) {printf_log(LOG_NONE, "%s (%d)\n", strerror(errno), errno);}
    if(((ret!=MAP_FAILED) && (emu || box64_is32bits)) && (BOX64ENV(log)>=LOG_DEBUG || BOX64ENV(dynarec_log)>=LOG_DEBUG)) {printf_log(LOG_NONE, "%p\n", ret);}
    #ifdef DYNAREC
    if(BOX64ENV(dynarec) && ret!=MAP_FAILED) {
        /*if(flags&0x100000 && addr!=ret)
        {
            // program used MAP_FIXED_NOREPLACE but the host linux didn't support it
            // and responded with a different address, so ignore it
        } else*/ {
            if(prot& PROT_EXEC)
                addDBFromAddressRange((uintptr_t)ret, length);
            else
                cleanDBFromAddressRange((uintptr_t)ret, length, prot?0:1);
        }
    }
    #endif
    if(ret!=MAP_FAILED) {
        if (emu && !(flags & MAP_ANONYMOUS) && (fd > 0)) {
            // the last_mmap will allow mmap created by wine, even those that have hole, to be fully tracked as one single mmap
            if((ret>=last_mmap_addr[0]) && ret+length<(last_mmap_addr[0]+last_mmap_len[0]))
                RecordEnvMappings((uintptr_t)last_mmap_addr[0], last_mmap_len[0], fd);
            else if((ret>=last_mmap_addr[1]) && ret+length<(last_mmap_addr[1]+last_mmap_len[1]))
                RecordEnvMappings((uintptr_t)last_mmap_addr[1], last_mmap_len[1], fd);
            else
                RecordEnvMappings((uintptr_t)ret, length, fd);
        }
        if((flags&MAP_SHARED) && (fd>0)) {
            uint32_t flags = fcntl(fd, F_GETFL);
            if((flags&O_ACCMODE)==O_RDWR) {
                if((BOX64ENV(log)>=LOG_DEBUG || BOX64ENV(dynarec_log)>=LOG_DEBUG)) {printf_log(LOG_NONE, "Note: Marking the region (%p-%p prot=%x) as NEVERCLEAN because fd have O_RDWR attribute\n", ret, ret+length, prot);}
                prot |= PROT_NEVERCLEAN;
            }
        }
        // hack to capture full size of the mmap done by wine
#if defined(ANDROID) || defined(WINLATOR_GLIBC)
        if(emu && (fd==-1) && (flags&(MAP_PRIVATE|MAP_ANON))==(MAP_PRIVATE|MAP_ANON) && !(flags&MAP_NORESERVE)) {
#else
        if(emu && (fd==-1) && (flags&(MAP_PRIVATE|MAP_ANON))==(MAP_PRIVATE|MAP_ANON)) {
#endif
            last_mmap_addr[last_mmap_idx] = ret;
            last_mmap_len[last_mmap_idx] = length;
        } else {
            last_mmap_addr[last_mmap_idx] = NULL;
            last_mmap_len[last_mmap_idx] = 0;
        }
        last_mmap_idx = 1-last_mmap_idx;
        if(emu)
            setProtection_mmap((uintptr_t)ret, length, prot);
        else
            setProtection_box((uintptr_t)ret, length, prot);
        if(addr && ret!=addr)
            e = EEXIST;
    }
    errno = e;  // preserve errno
    return ret;
}
EXPORT void* my_mmap(x64emu_t* emu, void *addr, size_t length, int prot, int flags, int fd, ssize_t offset) __attribute__((alias("my_mmap64")));

EXPORT void* my_mremap(x64emu_t* emu, void* old_addr, size_t old_size, size_t new_size, int flags, void* new_addr)
{
    (void)emu;
    if((emu || box64_is32bits) && (BOX64ENV(log)>=LOG_DEBUG || BOX64ENV(dynarec_log)>=LOG_DEBUG)) {printf_log(LOG_NONE, "mremap(%p, %lu, %lu, %d, %p)=>", old_addr, old_size, new_size, flags, new_addr);}
    void* ret = mremap(old_addr, old_size, new_size, flags, new_addr);
    if((emu || box64_is32bits) && (BOX64ENV(log)>=LOG_DEBUG || BOX64ENV(dynarec_log)>=LOG_DEBUG)) {printf_log(LOG_NONE, "%p\n", ret);}
    if(ret!=(void*)-1) {
        uint32_t prot = getProtection((uintptr_t)old_addr)&~PROT_CUSTOM;
        if(ret==old_addr) {
            if(old_size && old_size<new_size) {
                setProtection_mmap((uintptr_t)ret+old_size, new_size-old_size, prot);
                #ifdef DYNAREC
                if(BOX64ENV(dynarec))
                    addDBFromAddressRange((uintptr_t)ret+old_size, new_size-old_size);
                #endif
            } else if(old_size && new_size<old_size) {
                freeProtection((uintptr_t)ret+new_size, old_size-new_size);
                #ifdef DYNAREC
                if(BOX64ENV(dynarec))
                    cleanDBFromAddressRange((uintptr_t)ret+new_size, old_size-new_size, 1);
                #endif
            } else if(!old_size) {
                setProtection_mmap((uintptr_t)ret, new_size, prot);
                #ifdef DYNAREC
                if(BOX64ENV(dynarec))
                    addDBFromAddressRange((uintptr_t)ret, new_size);
                #endif
            }
        } else {
            if(old_size
            #ifdef MREMAP_DONTUNMAP
            && ((flags&MREMAP_DONTUNMAP)==0)
            #endif
            ) {
                freeProtection((uintptr_t)old_addr, old_size);
                #ifdef DYNAREC
                if(BOX64ENV(dynarec))
                    cleanDBFromAddressRange((uintptr_t)old_addr, old_size, 1);
                #endif
            }
            setProtection_mmap((uintptr_t)ret, new_size, prot); // should copy the protection from old block
            #ifdef DYNAREC
            if(BOX64ENV(dynarec))
                addDBFromAddressRange((uintptr_t)ret, new_size);
            #endif
        }
    }
    return ret;
}

EXPORT int my_munmap(x64emu_t* emu, void* addr, size_t length)
{
    (void)emu;
    if((emu || box64_is32bits) && (BOX64ENV(log)>=LOG_DEBUG || BOX64ENV(dynarec_log)>=LOG_DEBUG)) {printf_log(LOG_NONE, "munmap(%p, 0x%lx)\n", addr, length);}
    int ret = box_munmap(addr, length);
    int e = errno;
    #ifdef DYNAREC
    if(!ret) {
        WillRemoveMapping((uintptr_t)addr, length);
    }
    if(!ret && BOX64ENV(dynarec) && length) {
        cleanDBFromAddressRange((uintptr_t)addr, length, 1);
    }
    #endif
    if(!ret) {
        last_mmap_addr[1-last_mmap_idx] = NULL;
        last_mmap_len[1-last_mmap_idx] = 0;
        freeProtection((uintptr_t)addr, length);
        RemoveMapping((uintptr_t)addr, length);
    }
    errno = e;  // preseve errno
    return ret;
}

EXPORT int my_mprotect(x64emu_t* emu, void *addr, unsigned long len, int prot)
{
    (void)emu;
    if(emu && (BOX64ENV(log)>=LOG_DEBUG || BOX64ENV(dynarec_log)>=LOG_DEBUG)) {printf_log(LOG_NONE, "mprotect(%p, 0x%lx, 0x%x)\n", addr, len, prot);}
    if(prot&PROT_WRITE)
        prot|=PROT_READ;    // PROT_READ is implicit with PROT_WRITE on x86_64
    int ret = mprotect(addr, len, prot);
    #ifdef DYNAREC
    if(BOX64ENV(dynarec) && !ret && len) {
        if(prot& PROT_EXEC) {
            if(!IsAddrMappingLoadAndClean((uintptr_t)addr))
                addDBFromAddressRange((uintptr_t)addr, len);
        } else
            cleanDBFromAddressRange((uintptr_t)addr, len, (!prot)?1:0);
    }
    #endif
    if(!ret && len) {
        updateProtection((uintptr_t)addr, len, prot);
    }
    return ret;
}

typedef struct mallinfo (*mallinfo_fnc)(void);
EXPORT void* my_mallinfo(x64emu_t* emu, void* p)
{
    static mallinfo_fnc f = NULL;
    static int inited = 0;
    if(!inited) {
        inited = 1;
        f = (mallinfo_fnc)dlsym(my_lib->w.lib, "mallinfo");
    }
    if(f)
        *(struct mallinfo*)p=f();
    else
        memset(p, 0, sizeof(struct mallinfo));
    return p;
}

struct my_mallinfo2_s {
    size_t arena;
    size_t ordblks;
    size_t smblks;
    size_t hblks;
    size_t hblkhd;
    size_t usmblks;
    size_t fsmblks;
    size_t uordblks;
    size_t fordblks;
    size_t keepcost;
};

typedef struct my_mallinfo2_s (*mallinfo2_fnc)(void);
EXPORT void* my_mallinfo2(x64emu_t* emu, void* p)
{
    static mallinfo2_fnc f = NULL;
    static int inited = 0;
    if(!inited) {
        inited = 1;
        f = (mallinfo2_fnc)dlsym(my_lib->w.lib, "mallinfo2");
    }
    if(f)
        *(struct my_mallinfo2_s*)p = f();
    else
        memset(p, 0, sizeof(struct my_mallinfo2_s));
    return p;
}

#ifdef STATICBUILD
void my_updateGlobalOpt() {}
void my_checkGlobalOpt() {}
#endif

EXPORT int my_getopt(int argc, char* const argv[], const char *optstring)
{
    my_updateGlobalOpt();
    int ret = getopt(argc, argv, optstring);
    my_checkGlobalOpt();
    return ret;
}

EXPORT int my_getopt_long(int argc, char* const argv[], const char* optstring, const struct option *longopts, int *longindex)
{
    my_updateGlobalOpt();
    int ret = getopt_long(argc, argv, optstring, longopts, longindex);
    my_checkGlobalOpt();
    return ret;
}

EXPORT int my_getopt_long_only(int argc, char* const argv[], const char* optstring, const struct option *longopts, int *longindex)
{
    my_updateGlobalOpt();
    int ret = getopt_long_only(argc, argv, optstring, longopts, longindex);
    my_checkGlobalOpt();
    return ret;
}

#ifndef ANDROID
typedef struct {
   void  *read;
   void *write;
   void  *seek;
   void *close;
} my_cookie_io_functions_t;

typedef struct my_cookie_s {
    uintptr_t r, w, s, c;
    void* cookie;
} my_cookie_t;

static ssize_t my_cookie_read(void *p, char *buf, size_t size)
{
    my_cookie_t* cookie = (my_cookie_t*)p;
    return (ssize_t)RunFunctionFmt(cookie->r, "ppL", cookie->cookie, buf, size)       ;
}
static ssize_t my_cookie_write(void *p, const char *buf, size_t size)
{
    my_cookie_t* cookie = (my_cookie_t*)p;
    return (ssize_t)RunFunctionFmt(cookie->w, "ppL", cookie->cookie, buf, size)       ;
}
static int my_cookie_seek(void *p, off64_t *offset, int whence)
{
    my_cookie_t* cookie = (my_cookie_t*)p;
    return RunFunctionFmt(cookie->s, "ppi", cookie->cookie, offset, whence)       ;
}
static int my_cookie_close(void *p)
{
    my_cookie_t* cookie = (my_cookie_t*)p;
    int ret = 0;
    if(cookie->c)
        ret = RunFunctionFmt(cookie->c, "p", cookie->cookie)      ;
    box_free(cookie);
    return ret;
}
EXPORT void* my_fopencookie(x64emu_t* emu, void* cookie, void* mode, my_cookie_io_functions_t *s)
{
    cookie_io_functions_t io_funcs = {s->read?my_cookie_read:NULL, s->write?my_cookie_write:NULL, s->seek?my_cookie_seek:NULL, my_cookie_close};
    my_cookie_t *cb = (my_cookie_t*)box_calloc(1, sizeof(my_cookie_t));
    cb->r = (uintptr_t)s->read;
    cb->w = (uintptr_t)s->write;
    cb->s = (uintptr_t)s->seek;
    cb->c = (uintptr_t)s->close;
    cb->cookie = cookie;
    return fopencookie(cb, mode, io_funcs);
}
#endif

#if 0

EXPORT long my_prlimit64(void* pid, uint32_t res, void* new_rlim, void* old_rlim)
{
    return syscall(__NR_prlimit64, pid, res, new_rlim, old_rlim);
}

EXPORT void* my_reallocarray(void* ptr, size_t nmemb, size_t size)
{
    return realloc(ptr, nmemb*size);
}

#ifndef __OPEN_NEEDS_MODE
# define __OPEN_NEEDS_MODE(oflag) \
  (((oflag) & O_CREAT) != 0)
// || ((oflag) & __O_TMPFILE) == __O_TMPFILE)
#endif
EXPORT int my___open_nocancel(x64emu_t* emu, void* file, int oflag, int* b)
{
    int mode = 0;
    if (__OPEN_NEEDS_MODE (oflag))
        mode = b[0];
    return openat(AT_FDCWD, file, oflag, mode);
}

EXPORT int my___libc_alloca_cutoff(x64emu_t* emu, size_t size)
{
    // not always implemented on old linux version...
    library_t* lib = my_lib;
    if(!lib) return 0;
    void* f = dlsym(lib->w.lib, "__libc_alloca_cutoff");
    if(f)
        return ((iFL_t)f)(size);
    // approximate version but it's better than nothing....
    return (size<=(65536*4));
}

EXPORT int my_nanosleep(const struct timespec *req, struct timespec *rem)
{
    if(!req)
        return 0;   // workaround for some strange calls
    return nanosleep(req, rem);
}
#endif

// DL functions from wrappedlibdl.c
void* my_dlopen(x64emu_t* emu, void *filename, int flag);
int my_dlclose(x64emu_t* emu, void *handle);
void* my_dlsym(x64emu_t* emu, void *handle, void *symbol);
EXPORT int my___libc_dlclose(x64emu_t* emu, void* handle)
{
    return my_dlclose(emu, handle);
}
EXPORT void* my___libc_dlopen_mode(x64emu_t* emu, void* name, int mode)
{
    return my_dlopen(emu, name, mode);
}
EXPORT void* my___libc_dlsym(x64emu_t* emu, void* handle, void* name)
{
    return my_dlsym(emu, handle, name);
}

#ifdef ANDROID
void obstackSetup() {
}
#else
// all obstack function defined in obstack.c file
void obstackSetup();
#endif

EXPORT void* my_malloc(unsigned long size)
{
    return calloc(1, size);
}

EXPORT int my_setrlimit(x64emu_t* emu, int ressource, const struct rlimit *rlim)
{
    int ret = (ressource==RLIMIT_AS)?0:setrlimit(ressource, rlim);
    if(ressource==RLIMIT_AS) printf_log(LOG_DEBUG, " (ignored) RLIMIT_AS, cur=0x%lx, max=0x%lx ", rlim->rlim_cur, rlim->rlim_max);
    return ret;
}

#if 0
#ifdef PANDORA
#define RENAME_NOREPLACE    (1 << 0)
#define RENAME_EXCHANGE     (1 << 1)
#define RENAME_WHITEOUT     (1 << 2)
EXPORT int my_renameat2(int olddirfd, void* oldpath, int newdirfd, void* newpath, uint32_t flags)
{
    // simulate that function, but
    if(flags&RENAME_NOREPLACE) {
        if(FileExist(newpath, -1)) {
            errno = EEXIST;
            return -1;
        }
        flags &= ~RENAME_NOREPLACE;
    }
    if(!flags) return renameat(olddirfd, oldpath, newdirfd, newpath);
    if(flags&RENAME_WHITEOUT) {
        errno = EINVAL;
        return -1;  // not handling that
    }
    if((flags&RENAME_EXCHANGE) && (olddirfd==-1) && (newdirfd==-1)) {
        // cannot do atomically...
        char* tmp = (char*)box_malloc(strlen(oldpath)+10); // create a temp intermediary
        tmp = strcat(oldpath, ".tmp");
        int ret = renameat(-1, oldpath, -1, tmp);
        if(ret==-1) return -1;
        ret = renameat(-1, newpath, -1, oldpath);
        if(ret==-1) return -1;
        ret = renameat(-1, tmp, -1, newpath);
        box_free(tmp);
        return ret;
    }
    return -1; // unknown flags
}
#endif

#ifndef __NR_memfd_create
#define MFD_CLOEXEC         0x0001U
#define MFD_ALLOW_SEALING   0x0002U
EXPORT int my_memfd_create(x64emu_t* emu, void* name, uint32_t flags)
{
    // try to simulate that function
    uint32_t fl = O_RDWR | O_CREAT;
    if(flags&MFD_CLOEXEC)
        fl |= O_CLOEXEC;
    int tmp = shm_open(name, fl, S_IRWXU);
    if(tmp<0) return -1;
    shm_unlink(name);    // remove the shm file, but it will still exist because it's currently in use
    return tmp;
}
#endif

#ifndef GRND_RANDOM
#define GRND_RANDOM 0x0002
#endif
EXPORT int my_getentropy(x64emu_t* emu, void* buffer, size_t length)
{
    library_t* lib = my_lib;
    if(!lib) return 0;
    void* f = dlsym(lib->w.lib, "getentropy");
    if(f)
        return ((iFpL_t)f)(buffer, length);
    // custom implementation
    if(length>256) {
        errno = EIO;
        return -1;
    }
    int ret = my_getrandom(emu, buffer, length, GRND_RANDOM);
    if(ret!=length) {
        errno = EIO;
        return -1;
    }
    return 0;
}

EXPORT void my_mcount(void* frompc, void* selfpc)
{
    // stub doing nothing...
    return;
}
#endif

#ifndef ANDROID
union semun {
  int              val;    /* Value for SETVAL */
  struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
  unsigned short  *array;  /* Array for GETALL, SETALL */
  struct seminfo  *__buf;  /* Buffer for IPC_INFO
                              (Linux-specific) */
};
#endif
#ifndef SEM_STAT_ANY
#define SEM_STAT_ANY 20
#endif

EXPORT int my_semctl(int semid, int semnum, int cmd, union semun b)
{
    struct semid_ds semidds;
    void *backup = NULL;
    if ((cmd == IPC_STAT) || (cmd == IPC_SET) || (cmd == SEM_STAT) || (cmd == SEM_STAT_ANY)) {
        backup = b.buf;
        b.buf = &semidds;
        if (cmd == IPC_SET) {
            AlignSemidDs(&semidds, backup);
        }
    }
    int ret = semctl(semid, semnum, cmd, b);
    if ((cmd == IPC_STAT) || (cmd == IPC_SET) || (cmd == SEM_STAT) || (cmd == SEM_STAT_ANY)) {
        b.buf = backup;
        if (cmd == IPC_STAT) {
            UnalignSemidDs(backup, &semidds);
        }
    }
    return ret;
}

EXPORT int64_t userdata_sign = 0x1234598765ABCEF0;
EXPORT uint32_t userdata[1024];

EXPORT long my_ptrace(x64emu_t* emu, int request, pid_t pid, void* addr, uint32_t* data)
{
    if(request == PTRACE_POKEUSER) {
        if(ptrace(PTRACE_PEEKDATA, pid, &userdata_sign, NULL)==userdata_sign  && (uintptr_t)addr < sizeof(userdata)) {
            long ret = ptrace(PTRACE_POKEDATA, pid, addr+(uintptr_t)userdata, data);
            return ret;
        }
        // fallback to a generic local faking
        if((uintptr_t)addr < sizeof(userdata)) {
            *(uintptr_t*)(addr+(uintptr_t)userdata) = (uintptr_t)data;
        // lets just ignore this for now!
            errno = 0;
            return 0;
        }
        errno = EINVAL;
        return -1;
    }
    if(request == PTRACE_PEEKUSER) {
        if(ptrace(PTRACE_PEEKDATA, pid, &userdata_sign, NULL)==userdata_sign  && (uintptr_t)addr < sizeof(userdata)) {
            return ptrace(PTRACE_PEEKDATA, pid, addr+(uintptr_t)userdata, data);
        }
        // fallback to a generic local faking
        if((uintptr_t)addr < sizeof(userdata)) {
            errno = 0;
            return *(uintptr_t*)(addr+(uintptr_t)userdata);
        }
        errno = EINVAL;
        return -1;
    }
    long ret = ptrace(request, pid, addr, data);
    return ret;
}

// Backtrace stuff

#include "elfs/elfdwarf_private.h"
EXPORT int my_backtrace(x64emu_t* emu, void** buffer, int size)
{
    if (!size) return 0;
    dwarf_unwind_t *unwind = init_dwarf_unwind_registers(emu);
    int idx = 0;
    char success = 0;
    if(!(getProtection_fast(R_RSP)&PROT_READ))
        return 0;
    if(!(getProtection_fast((uintptr_t)buffer)&PROT_READ))
        return 0;
    uintptr_t addr = *(uintptr_t*)R_RSP;
    buffer[0] = (void*)addr;
    while (++idx < size) {
        uintptr_t ret_addr = get_parent_registers(unwind, FindElfAddress(my_context, addr), addr, &success);
        if (ret_addr == my_context->exit_bridge) {
            // TODO: do something to be able to get the function name
            buffer[idx] = (void*)ret_addr;
            success = 2;
            // See elfdwarf_private.c for the register mapping
            unwind->regs[7] = unwind->regs[6]; // mov rsp, rbp
            if(!(getProtection_fast(unwind->regs[7])&PROT_READ))
                return idx-1;
            unwind->regs[6] = *(uint64_t*)unwind->regs[7]; // pop rbp
            unwind->regs[7] += 8;
            ret_addr = *(uint64_t*)unwind->regs[7]; // ret
            unwind->regs[7] += 8;
            if (++idx < size) buffer[idx] = (void*)ret_addr;
        } else if (!success) break;
        else buffer[idx] = (void*)ret_addr;
        addr = ret_addr;
    }
    free_dwarf_unwind_registers(&unwind);
    return idx;
}

// special version, called in signal with SHOWBT
EXPORT int my_backtrace_ip(x64emu_t* emu, void** buffer, int size)
{
    if (!size) return 0;
    dwarf_unwind_t *unwind = init_dwarf_unwind_registers(emu);
    int idx = 0;
    char success = 1;
    uintptr_t addr = R_RIP;
    buffer[0] = (void*)addr;
    while ((++idx < size) && success) {
        uintptr_t ret_addr = get_parent_registers(unwind, FindElfAddress(my_context, addr), addr, &success);
        if (ret_addr == my_context->exit_bridge) {
            // TODO: do something to be able to get the function name
            buffer[idx] = (void*)ret_addr;
            success = 2;
            // See elfdwarf_private.c for the register mapping
            unwind->regs[7] = unwind->regs[6]; // mov rsp, rbp
            unwind->regs[6] = *(uint64_t*)unwind->regs[7]; // pop rbp
            unwind->regs[7] += 8;
            ret_addr = *(uint64_t*)unwind->regs[7]; // ret
            unwind->regs[7] += 8;
            if (++idx < size) buffer[idx] = (void*)ret_addr;
        } else if (!success) {
            if(getProtection((uintptr_t)addr)&(PROT_READ)) {
                if (getProtection((uintptr_t)addr - 19) && *(uint8_t*)(addr - 19) == 0xCC && IsBridgeSignature(*(uint8_t*)(addr - 19 + 1), *(uint8_t*)(addr - 19 + 2))) {
                    buffer[idx-1] = (void*)(addr-19);
                    success = 2;
                    if(idx==1)
                        unwind->regs[7] -= 8;
                    ret_addr = *(uint64_t*)unwind->regs[7]; // ret
                    unwind->regs[7] += 8;
                    buffer[idx] = (void*)ret_addr;
                } else {
                    // try a simple end of function epilog
                    unwind->regs[7] = unwind->regs[6]; // mov rsp, rbp
                    if(getProtection(unwind->regs[7])&(PROT_READ)) {
                        unwind->regs[6] = *(uint64_t*)unwind->regs[7]; // pop rbp
                        unwind->regs[7] += 8;
                        ret_addr = *(uint64_t*)unwind->regs[7]; // ret
                        unwind->regs[7] += 8;
                        buffer[idx] = (void*)ret_addr;
                        success = 2;
                    } else
                        break;
                }
            } else
                break;
        } else buffer[idx] = (void*)ret_addr;
        addr = ret_addr;
    }
    free_dwarf_unwind_registers(&unwind);
    return idx;
}

EXPORT char** my_backtrace_symbols(x64emu_t* emu, uintptr_t* buffer, int size)
{
    (void)emu;
    char** ret = (char**)calloc(1, size*sizeof(char*) + size*200);  // capping each strings to 200 chars, not using box_calloc (program space)
    char* s = (char*)(ret+size);
    for (int i=0; i<size; ++i) {
        uintptr_t start = 0;
        uint64_t sz = 0;
        elfheader_t *hdr = FindElfAddress(my_context, buffer[i]);
        const char* symbname = FindNearestSymbolName(hdr, (void*)buffer[i], &start, &sz);
        if(!sz) sz=0x100;   // arbitrary value...
        if (symbname && buffer[i]>=start && (buffer[i]<(start+sz) || !sz)) {
            snprintf(s, 200, "%s(%s+%lx) [%p]", ElfName(hdr), symbname, buffer[i] - start, (void*)buffer[i]);
        } else if (hdr) {
            snprintf(s, 200, "%s+%lx [%p]", ElfName(hdr), buffer[i] - (uintptr_t)GetBaseAddress(hdr), (void*)buffer[i]);
        } else {
            snprintf(s, 200, "??? [%p]", (void*)buffer[i]);
        }
        ret[i] = s;
        s += 200;
    }
    return ret;
}

EXPORT void my_backtrace_symbols_fd(x64emu_t* emu, uintptr_t* buffer, int size, int fd)
{
    (void)emu;
    char s[200];
    for (int i=0; i<size; ++i) {
        uintptr_t start = 0;
        uint64_t sz = 0;
        const char* symbname = FindNearestSymbolName(FindElfAddress(my_context, buffer[i]), (void*)buffer[i], &start, &sz);
        if(!sz) sz=0x100;   // arbitrary value...
        if(symbname && buffer[i]>=start && (buffer[i]<(start+sz) || !sz))
            snprintf(s, 200, "%s+%ld [%p]\n", symbname, buffer[i] - start, (void*)buffer[i]);
        else
            snprintf(s, 200, "??? [%p]\n", (void*)buffer[i]);
        int dummy = write(fd, s, strlen(s));
        (void)dummy;
    }
}

EXPORT int my_iopl(x64emu_t* emu, int level)
{
    // Set I/O permission (so access IN/OUT opcodes) Default is 0. Can Set to 0..3
    // set permission for all 65536 ports addresses
    // note ioperm can set individual permission
    /*static iFi_t real_iopl = NULL;
    static int searched = 0;
    if(!searched) {
        searched = 1;
        real_iopl = (iFi_t)dlsym(my_lib, "iopl");
    }
    if(real_iopl)
        return real_iopl(level);*/
    // For now, lets just return "unsupported"
    errno = ENOSYS;
    return -1;
}

EXPORT int my_stime(x64emu_t* emu, const time_t *t)
{
    // TODO?
    errno = EPERM;
    return -1;
}

int GetTID();
#ifdef ANDROID
void updateGlibcTidCache() {}
#else
struct glibc_pthread {
#if defined(NO_ALIGN)
    char header[704];
#else
    void* header[24];
#endif
  void* list[2];
  pid_t tid;
};
pid_t getGlibcCachedTid() {
  pthread_mutex_t lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
  pthread_mutex_lock(&lock);
  pid_t tid = lock.__data.__owner;
  pthread_mutex_unlock(&lock);
  pthread_mutex_destroy(&lock);
  return tid;
}
void updateGlibcTidCache() {
  pid_t real_tid = GetTID();
  pid_t cached_tid = getGlibcCachedTid();
  if (cached_tid != real_tid) {
    pid_t* cached_tid_location =
        &((struct glibc_pthread*)(pthread_self()))->tid;
    *cached_tid_location = real_tid;
  }
}
#endif
typedef struct clone_arg_s {
 uintptr_t stack;
 x64emu_t *emu;
 uintptr_t fnc;
 void* args;
 int stack_clone_used;
 int flags;
 void* tls;
} clone_arg_t;
void init_mutexes(box64context_t* context);
static int clone_fn(void* p)
{
    clone_arg_t* arg = (clone_arg_t*)p;
    updateGlibcTidCache();  // update cache tid if needed
    x64emu_t *emu = arg->emu;
    R_RSP = arg->stack;
    emu->flags.quitonexit = 1;
    thread_forget_emu();    //TODO: not all will flags needs this, probably just CLONE_VM?
    thread_set_emu(emu);
    if(arg->flags&CLONE_NEWUSER) {
        init_mutexes(my_context);
        ResetSegmentsCache(emu);
    }
    int ret = RunFunctionWithEmu(emu, 0, arg->fnc, 1, arg->args);
    int exited = (emu->flags.quitonexit==2);
    thread_set_emu(NULL);
    FreeX64Emu(&emu);
    if(arg->stack_clone_used)
        my_context->stack_clone_used = 0;
    box_free(arg);
    /*if(exited)
        exit(ret);*/
    return ret;
}

EXPORT int my_clone(x64emu_t* emu, void* fn, void* stack, int flags, void* args, void* parent, void* tls, void* child)
{
    printf_log(LOG_DEBUG, "my_clone(fn:%p(%s), stack:%p, 0x%x, args:%p, %p, %p, %p)", fn, getAddrFunctionName((uintptr_t)fn), stack, flags, args, parent, tls, child);
    void* mystack = NULL;
    clone_arg_t* arg = (clone_arg_t*)box_calloc(1, sizeof(clone_arg_t));
    x64emu_t * newemu = NewX64Emu(emu->context, R_RIP, (uintptr_t)stack, 0, 0);
    SetupX64Emu(newemu, emu);
    //CloneEmu(newemu, emu);
    if(my_context->stack_clone_used) {
        printf_log(LOG_DEBUG, " no free stack_clone ");
        mystack = box_malloc(1024*1024);  // stack for own process... memory leak, but no practical way to remove it
    } else {
        if(!my_context->stack_clone)
            my_context->stack_clone = box_malloc(1024*1024);
        mystack = my_context->stack_clone;
        printf_log(LOG_DEBUG, " using stack_clone ");
        my_context->stack_clone_used = 1;
        arg->stack_clone_used = 1;
    }
    arg->stack = (uintptr_t)stack &~7LL;
    arg->args = args;
    arg->fnc = (uintptr_t)fn;
    arg->tls = tls;
    arg->emu = newemu;
    arg->flags = flags;
    if((flags|(CLONE_VM|CLONE_VFORK|CLONE_SETTLS))==flags)   // that's difficult to setup, so lets ignore all those flags :S
        flags&=~(CLONE_VM|CLONE_VFORK|CLONE_SETTLS);
    int64_t ret = clone(clone_fn, (void*)((uintptr_t)mystack+1024*1024), flags, arg, parent, NULL, child);
    return (uintptr_t)ret;
}

EXPORT void my___cxa_pure_virtual(x64emu_t* emu)
{
    printf_log(LOG_NONE, "Pure virtual function called\n");
    emu->quit = 1;
    abort();
}

EXPORT size_t my_strlcpy(x64emu_t* emu, void* dst, void* src, size_t l)
{
    strncpy(dst, src, l-1);
    ((char*)dst)[l-1] = '\0';
    return strlen(src);
}
EXPORT size_t my_strlcat(x64emu_t* emu, void* dst, void* src, size_t l)
{
    size_t s = strlen(dst);
    if(s>=l)
        return l;
    strncat(dst, src, l-1);
    ((char*)dst)[l-1] = '\0';
    return s+strlen(src);
}

EXPORT int my_register_printf_specifier(x64emu_t* emu, int c, void* f1, void* f2)
{
    //TODO: defining a new sepcifier for printf, it should also be registered on myStackAlign/myStackAlignW, using f2 to get the type of arg
    return my->register_printf_specifier(c, findprintf_outputFct(f1), findprintf_arginfoFct(f2));
}

EXPORT int my_register_printf_type(x64emu_t* emu, void* f)
{
    //TODO: defining a new type, probably needs to also register that for myStackAlign stuffs
    return my->register_printf_type(findprintf_typeFct(f));
}

EXPORT __uint128_t my___udivti3(__uint128_t a, __uint128_t b)
{
    return a/b;
}

extern int box64_quit;
extern int box64_exit_code;
void endBox64();
static void* timed_exit_thread(void* a)
{
    // this is a workaround for some NVidia drivers on ARM64 that may freeze at exit
    // waiting on a pthread_cond_destroy
    usleep(5000000); // wait 5 seconds
    printf_log(LOG_DEBUG, "Too late, forced exit...\n");
    _exit(box64_exit_code); // force exit, something is wrong
}

void startTimedExit()
{
    static int started = 0;
    if(started)
        return;
    started = 1;
    pthread_t exit_thread;
    pthread_create(&exit_thread, NULL, timed_exit_thread, NULL);
}

EXPORT void my_exit(x64emu_t* emu, int code)
{
    if(emu->flags.quitonexit) {
        emu->quit = 1;
        R_EAX = code;
        emu->flags.quitonexit = 2;
        return;
    }
    emu->quit = 1;
    box64_exit_code = code;
    endBox64();
    startTimedExit();
    exit(code);
}

EXPORT void my__exit(x64emu_t* emu, int code)
{
    if(emu->flags.quitonexit || emu->quit) {
        _exit(code);
    }
    dynarec_log(LOG_INFO, "Fast _exit called\n");
    emu->quit = 1;
    box64_exit_code = code;
    SerializeAllMapping();   // just to be safe
    // then call all the fini
    
    _exit(code);
}

EXPORT int my_prctl(x64emu_t* emu, int option, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5)
{
    if(option==PR_SET_NAME) {
        printf_log(LOG_DEBUG, "set process name to \"%s\"\n", (char*)arg2);
        ApplyEnvFileEntry((char*)arg2);
        size_t l = strlen((char*)arg2);
        if(l>4 && !strcasecmp((char*)arg2+l-4, ".exe")) {
            printf_log(LOG_DEBUG, "hacking orig command line to \"%s\"\n", (char*)arg2);
            strcpy(my_context->orig_argv[0], (char*)arg2);
        }
    }
    if(option==PR_SET_SECCOMP) {
        printf_log(LOG_INFO, "ignoring prctl(PR_SET_SECCOMP, ...)\n");
        return 0;
    }
    return prctl(option, arg2, arg3, arg4, arg5);
}

#ifndef _SC_NPROCESSORS_ONLN
#define _SC_NPROCESSORS_ONLN    84
#endif
#ifndef _SC_NPROCESSORS_CONF
#define _SC_NPROCESSORS_CONF    83
#endif
EXPORT long my_sysconf(x64emu_t* emu, int what) {
    if(what==_SC_NPROCESSORS_ONLN) {
        return getNCpu();
    }
    if(what==_SC_NPROCESSORS_CONF) {
        return getNCpu();
    }
    return sysconf(what);
}
EXPORT long my___sysconf(x64emu_t* emu, int what) __attribute__((alias("my_sysconf")));

EXPORT char* my___progname = NULL;
EXPORT char* my___progname_full = NULL;
EXPORT char* my_program_invocation_name = NULL;
EXPORT char* my_program_invocation_short_name = NULL;

// ignoring this for now
EXPORT char my___libc_single_threaded = 0;

EXPORT char* secure_getenv(const char* name)
{
    // ignoring the "secure" part for now
    //TODO: better handling of user and process ID
    return getenv(name);
}

#ifdef STATICBUILD
uint32_t get_random32();
__attribute__((weak)) uint32_t arc4random()
{
    return get_random32();
}
#include "libtools/static_libc.h"
#endif

#ifndef STATICBUILD
#define PRE_INIT\
    if(1)                                                      \
        lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);    \
    else
#endif

#if defined(ANDROID)
#ifdef STATICBUILD
#define NEEDED_LIBS_DEF   3,\
    "libpthread.so",        \
    "libdl.so" ,            \
    "libm.so"
#define NEEDED_LIBS_234 3,  \
    "libpthread.so",        \
    "libdl.so" ,            \
    "libm.so"
#else
#define NEEDED_LIBS_DEF   4,\
    "libpthread.so",        \
    "libdl.so" ,            \
    "libm.so",              \
    "libbsd.so"
#define NEEDED_LIBS_234 4,  \
    "libpthread.so",        \
    "libdl.so" ,            \
    "libm.so",              \
    "libbsd.so"
#endif
#else
#ifdef STATICBUILD
#define NEEDED_LIBS_DEF   5,\
    "ld-linux-x86-64.so.2", \
    "libpthread.so.0",      \
    "libdl.so.2",           \
    "libutil.so.1",         \
    "librt.so.1"
#define NEEDED_LIBS_234 6,  \
    "ld-linux-x86-64.so.2", \
    "libpthread.so.0",      \
    "libdl.so.2",           \
    "libutil.so.1",         \
    "libresolv.so.2",       \
    "librt.so.1"
#else
#define NEEDED_LIBS_DEF   6,\
    "ld-linux-x86-64.so.2", \
    "libpthread.so.0",      \
    "libdl.so.2",           \
    "libutil.so.1",         \
    "librt.so.1",           \
    "libbsd.so.0"
#define NEEDED_LIBS_234 7,  \
    "ld-linux-x86-64.so.2", \
    "libpthread.so.0",      \
    "libdl.so.2",           \
    "libutil.so.1",         \
    "libresolv.so.2",       \
    "librt.so.1",           \
    "libbsd.so.0"
#endif
#endif

#undef HAS_MY

#define CUSTOM_INIT         \
    box64->libclib = lib;   \
    /*InitCpuModel();*/         \
    ctSetup();              \
    obstackSetup();         \
    my_environ = my__environ = my___environ = box64->envv;                      \
    my___progname_full = my_program_invocation_name = box64->argv[0];           \
    my___progname = my_program_invocation_short_name =                          \
        strrchr(box64->argv[0], '/') + 1;                                       \
    getMy(lib);                                                                 \
    if(box64_isglibc234)                                                        \
        setNeededLibs(lib, NEEDED_LIBS_234);                                    \
    else                                                                        \
        setNeededLibs(lib, NEEDED_LIBS_DEF);

#define CUSTOM_FINI \
    freeMy();       \
    return;     // do not unload...

#include "wrappedlib_init.h"
