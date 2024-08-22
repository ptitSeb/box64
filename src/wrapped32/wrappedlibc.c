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
#include <getopt.h>
#include <pwd.h>
#include <locale.h>

#include "wrappedlibs.h"

#include "box64stack.h"
#include "x64emu.h"
#include "debug.h"
#include "wrapper32.h"
#include "bridge.h"
#include "callback.h"
#include "librarian.h"
#include "librarian/library_private.h"
#include "emu/x64emu_private.h"
#include "box32context.h"
#include "myalign32.h"
#include "signals.h"
#include "fileutils.h"
#include "auxval.h"
#include "elfloader.h"
#include "bridge.h"
#include "globalsymbols.h"
#include "box32.h"

#ifdef PANDORA
#ifndef __NR_preadv
#define __NR_preadv                     (__NR_SYSCALL_BASE+361)
#endif
#ifndef __NR_pwritev
#define __NR_pwritev                    (__NR_SYSCALL_BASE+362)
#endif
#ifndef __NR_accept4
#define __NR_accept4                    (__NR_SYSCALL_BASE+366)
#endif
#ifndef __NR_sendmmsg
#define __NR_sendmmsg			        (__NR_SYSCALL_BASE+374)
#endif
#ifndef __NR_prlimit64
#define __NR_prlimit64                  (__NR_SYSCALL_BASE+369)
#endif
#ifndef __NR_recvmmsg
#define __NR_recvmmsg                   (__NR_SYSCALL_BASE+365)
#endif
#elif defined(__arm__)
#ifndef __NR_accept4
#define __NR_accept4                    (__NR_SYSCALL_BASE+366)
#endif
#endif

// need to undef all read / read64 stuffs!
#undef pread
#undef pwrite
#undef lseek
#undef fseeko
#undef ftello
#undef fseekpos
#undef fsetpos
#undef fgetpos
#undef fopen
#undef statfs
#undef fstatfs
#undef freopen
#undef truncate
#undef ftruncate
#undef tmpfile
#undef lockf
#undef fscanf
#undef scanf
#undef sscanf
#undef vfscanf
#undef vscanf
#undef vsscanf
#undef getc
#undef putc
#undef mkstemp
#undef mkstemps
#undef mkostemp
#undef mkostemps
#undef open
#undef openat
#undef read
#undef write
#undef creat
#undef scandir
#undef mmap
#undef fcntl
#undef stat
#undef __xstat
#undef xstat
#undef scandir
#undef ftw
#undef nftw
#undef glob

#define LIBNAME libc

static const char* libcName =
#ifdef ANDROID
    "libc.so"
#else
    "libc.so.6"
#endif
    ;

static library_t* my_lib = NULL;

extern int fix_64bit_inodes;
#if 0
typedef int (*iFL_t)(unsigned long);
typedef void (*vFpp_t)(void*, void*);
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
typedef int32_t (*iFpLpp_t)(void*, size_t, void*, void*);
typedef int32_t (*iFppii_t)(void*, void*, int32_t, int32_t);
typedef int32_t (*iFipuu_t)(int32_t, void*, uint32_t, uint32_t);
typedef int32_t (*iFipiI_t)(int32_t, void*, int32_t, int64_t);
typedef int32_t (*iFipuup_t)(int32_t, void*, uint32_t, uint32_t, void*);
typedef int32_t (*iFiiV_t)(int32_t, int32_t, ...);
typedef void* (*pFp_t)(void*);
typedef void* (*pFu_t)(uint32_t);
#define SUPER() \
    GO(_ITM_addUserCommitAction, iFpup_t)   \
    GO(_IO_file_stat, iFpp_t)

#endif

//#include "wrappercallback.h"

#if 0
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
static uintptr_t my32_compare_fct_##A = 0;        \
static int my32_compare_##A(void* a, void* b)     \
{                                               \
    return (int)RunFunction(my_context, my32_compare_fct_##A, 2, a, b);\
}
SUPER()
#undef GO
static void* findcompareFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my32_compare_fct_##A == (uintptr_t)fct) return my32_compare_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_compare_fct_##A == 0) {my32_compare_fct_##A = (uintptr_t)fct; return my32_compare_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc compare callback\n");
    return NULL;
}

// ftw
#define GO(A)   \
static uintptr_t my32_ftw_fct_##A = 0;                                      \
static int my32_ftw_##A(void* fpath, void* sb, int flag)                       \
{                                                                               \
    return (int)RunFunction(my_context, my32_ftw_fct_##A, 3, fpath, sb, flag);   \
}
SUPER()
#undef GO
static void* findftwFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my32_ftw_fct_##A == (uintptr_t)fct) return my32_ftw_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_ftw_fct_##A == 0) {my32_ftw_fct_##A = (uintptr_t)fct; return my32_ftw_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc ftw callback\n");
    return NULL;
}

// ftw64
#define GO(A)   \
static uintptr_t my32_ftw64_fct_##A = 0;                      \
static int my32_ftw64_##A(void* fpath, void* sb, int flag)    \
{                                                           \
    struct i386_stat64 i386st;                              \
    UnalignStat64(sb, &i386st);                             \
    return (int)RunFunction(my_context, my32_ftw64_fct_##A, 3, fpath, &i386st, flag);  \
}
SUPER()
#undef GO
static void* findftw64Fct(void* fct)
{
    if(!fct) return NULL;
    #define GO(A) if(my32_ftw64_fct_##A == (uintptr_t)fct) return my32_ftw64_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_ftw64_fct_##A == 0) {my32_ftw64_fct_##A = (uintptr_t)fct; return my32_ftw64_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc ftw64 callback\n");
    return NULL;
}

// nftw
#define GO(A)   \
static uintptr_t my32_nftw_fct_##A = 0;                                   \
static int my32_nftw_##A(void* fpath, void* sb, int flag, void* ftwbuff)  \
{                                                                       \
    return (int)RunFunction(my_context, my32_nftw_fct_##A, 4, fpath, sb, flag, ftwbuff);   \
}
SUPER()
#undef GO
static void* findnftwFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my32_nftw_fct_##A == (uintptr_t)fct) return my32_nftw_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_nftw_fct_##A == 0) {my32_nftw_fct_##A = (uintptr_t)fct; return my32_nftw_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc nftw callback\n");
    return NULL;
}

// nftw64
#define GO(A)   \
static uintptr_t my32_nftw64_fct_##A = 0;                                     \
static int my32_nftw64_##A(void* fpath, void* sb, int flag, void* ftwbuff)    \
{                                                                           \
    struct i386_stat64 i386st;                                              \
    UnalignStat64(sb, &i386st);                                             \
    return (int)RunFunction(my_context, my32_nftw64_fct_##A, 4, fpath, &i386st, flag, ftwbuff);   \
}
SUPER()
#undef GO
static void* findnftw64Fct(void* fct)
{
    if(!fct) return NULL;
    #define GO(A) if(my32_nftw64_fct_##A == (uintptr_t)fct) return my32_nftw64_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_nftw64_fct_##A == 0) {my32_nftw64_fct_##A = (uintptr_t)fct; return my32_nftw64_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc nftw64 callback\n");
    return NULL;
}

// globerr
#define GO(A)   \
static uintptr_t my32_globerr_fct_##A = 0;                                        \
static int my32_globerr_##A(void* epath, int eerrno)                              \
{                                                                               \
    return (int)RunFunction(my_context, my32_globerr_fct_##A, 2, epath, eerrno);  \
}
SUPER()
#undef GO
static void* findgloberrFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my32_globerr_fct_##A == (uintptr_t)fct) return my32_globerr_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_globerr_fct_##A == 0) {my32_globerr_fct_##A = (uintptr_t)fct; return my32_globerr_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc globerr callback\n");
    return NULL;
}
#undef dirent
// filter_dir
#define GO(A)   \
static uintptr_t my32_filter_dir_fct_##A = 0;                               \
static int my32_filter_dir_##A(const struct dirent* a)                    \
{                                                                       \
    return (int)RunFunction(my_context, my32_filter_dir_fct_##A, 1, a);     \
}
SUPER()
#undef GO
static void* findfilter_dirFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my32_filter_dir_fct_##A == (uintptr_t)fct) return my32_filter_dir_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_filter_dir_fct_##A == 0) {my32_filter_dir_fct_##A = (uintptr_t)fct; return my32_filter_dir_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc filter_dir callback\n");
    return NULL;
}
// compare_dir
#define GO(A)   \
static uintptr_t my32_compare_dir_fct_##A = 0;                                  \
static int my32_compare_dir_##A(const struct dirent* a, const struct dirent* b)    \
{                                                                           \
    return (int)RunFunction(my_context, my32_compare_dir_fct_##A, 2, a, b);     \
}
SUPER()
#undef GO
static void* findcompare_dirFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my32_compare_dir_fct_##A == (uintptr_t)fct) return my32_compare_dir_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_compare_dir_fct_##A == 0) {my32_compare_dir_fct_##A = (uintptr_t)fct; return my32_compare_dir_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc compare_dir callback\n");
    return NULL;
}

// filter64
#define GO(A)   \
static uintptr_t my32_filter64_fct_##A = 0;                               \
static int my32_filter64_##A(const struct dirent64* a)                    \
{                                                                       \
    return (int)RunFunction(my_context, my32_filter64_fct_##A, 1, a);     \
}
SUPER()
#undef GO
static void* findfilter64Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my32_filter64_fct_##A == (uintptr_t)fct) return my32_filter64_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_filter64_fct_##A == 0) {my32_filter64_fct_##A = (uintptr_t)fct; return my32_filter64_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc filter64 callback\n");
    return NULL;
}
// compare64
#define GO(A)   \
static uintptr_t my32_compare64_fct_##A = 0;                                      \
static int my32_compare64_##A(const struct dirent64* a, const struct dirent64* b) \
{                                                                               \
    return (int)RunFunction(my_context, my32_compare64_fct_##A, 2, a, b);         \
}
SUPER()
#undef GO
static void* findcompare64Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my32_compare64_fct_##A == (uintptr_t)fct) return my32_compare64_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_compare64_fct_##A == 0) {my32_compare64_fct_##A = (uintptr_t)fct; return my32_compare64_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc compare64 callback\n");
    return NULL;
}
// on_exit
#define GO(A)   \
static uintptr_t my32_on_exit_fct_##A = 0;                    \
static void my32_on_exit_##A(int a, const void* b)            \
{                                                           \
    RunFunction(my_context, my32_on_exit_fct_##A, 2, a, b);   \
}
SUPER()
#undef GO
static void* findon_exitFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my32_on_exit_fct_##A == (uintptr_t)fct) return my32_on_exit_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_on_exit_fct_##A == 0) {my32_on_exit_fct_##A = (uintptr_t)fct; return my32_on_exit_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc on_exit callback\n");
    return NULL;
}
#undef SUPER
#endif

// some my32_XXX declare and defines
int32_t my32___libc_start_main(x64emu_t* emu, int *(main) (int, char * *, char * *), 
    int argc, char * * ubp_av, void (*init) (void), void (*fini) (void), 
    void (*rtld_fini) (void), void (* stack_end)); // implemented in x64run_private.c
EXPORT void my32___libc_init_first(x64emu_t* emu, int argc, char* arg0, char** b)
{
    // do nothing specific for now
    return;
}
uint32_t my32_syscall(x64emu_t *emu); // implemented in x64syscall.c
void EXPORT my32___stack_chk_fail(x64emu_t* emu)
{
    char buff[200];
    #ifdef HAVE_TRACE
    sprintf(buff, "%p: Stack is corrupted, aborting (prev IP=%p)\n", (void*)emu->old_ip, (void*)emu->prev2_ip);
    #else
    sprintf(buff, "%p: Stack is corrupted, aborting\n", (void*)emu->old_ip);
    #endif
    StopEmu(emu, buff, 1);
}
void EXPORT my32___gmon_start__(x64emu_t *emu)
{
    printf_log(LOG_DEBUG, "__gmon_start__ called (dummy call)\n");
}
int EXPORT my32___cxa_atexit(x64emu_t* emu, void* p, void* a, void* d)
{
    AddCleanup1Arg(emu, p, a,FindElfAddress(my_context, (uintptr_t)d));
    return 0;
}
void EXPORT my32___cxa_finalize(x64emu_t* emu, void* p)
{
    if(!p) {
        // p is null, call (and remove) all Cleanup functions
        CallAllCleanup(emu);
        return;
    }
        CallCleanup(emu, p);
}
int EXPORT my32_atexit(x64emu_t* emu, void *p)
{
    AddCleanup(emu, p);
    return 0;
}


int my32_getcontext(x64emu_t* emu, void* ucp);
int my32_setcontext(x64emu_t* emu, void* ucp);
int my32_makecontext(x64emu_t* emu, void* ucp, void* fnc, int32_t argc, void* argv);
int my32_swapcontext(x64emu_t* emu, void* ucp1, void* ucp2);

// All signal and context functions defined in signals.c

// All fts function defined in myfts.c

// getauxval implemented in auxval.c


// this one is defined in elfloader.c
int my32_dl_iterate_phdr(x64emu_t *emu, void* F, void *data);

pid_t EXPORT my32_fork(x64emu_t* emu)
{
/*    #if 1
    emu->quit = 1;
    emu->fork = 1;
    return 0;
    #else
    return 0;
    #endif*/
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
        printf_log(LOG_NONE, "BOX32: Warning, fork errored... (%d)\n", v);
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
}
pid_t EXPORT my32___fork(x64emu_t* emu) __attribute__((alias("my32_fork")));
pid_t EXPORT my32_vfork(x64emu_t* emu)
{
    #if 1
    emu->quit = 1;
    emu->fork = 1;  // use regular fork...
    return 0;
    #else
    return 0;
    #endif
}

int EXPORT my32_uname(struct utsname *buf)
{
    //TODO: check sizeof(struct utsname) == 390
    int ret = uname(buf);
    strcpy(buf->machine, "x86_64");
    return ret;
}

// X86_O_RDONLY 0x00
#define X86_O_WRONLY       0x01     // octal     01
#define X86_O_RDWR         0x02     // octal     02
#define X86_O_CREAT        0x40     // octal     0100
#define X86_O_EXCL         0x80     // octal     0200
#define X86_O_NOCTTY       0x100    // octal     0400
#define X86_O_TRUNC        0x200    // octal    01000
#define X86_O_APPEND       0x400    // octal    02000
#define X86_O_NONBLOCK     0x800    // octal    04000
#define X86_O_SYNC         0x101000 // octal 04010000
#define X86_O_DSYNC        0x1000   // octal   010000
#define X86_O_RSYNC        O_SYNC
#define X86_FASYNC         020000
#define X86_O_DIRECT       040000
#define X86_O_LARGEFILE    0100000
#define X86_O_DIRECTORY    0200000
#define X86_O_NOFOLLOW     0400000
#define X86_O_NOATIME      01000000
#define X86_O_CLOEXEC      02000000
#define X86_O_PATH         010000000
#define X86_O_TMPFILE      020200000

#ifndef O_TMPFILE
#define O_TMPFILE (020000000 | O_DIRECTORY)
#endif
#ifndef O_PATH
#define O_PATH     010000000
#endif

#define SUPER()     \
    GO(O_WRONLY)    \
    GO(O_RDWR)      \
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

// x86->arm
int of_convert32(int a)
{
    if(!a || a==-1) return a;
    int b=0;
    #define GO(A) if((a&X86_##A)==X86_##A) {a&=~X86_##A; b|=A;}
    SUPER();
    #undef GO
    if(a) {
        printf_log(LOG_NONE, "Warning, of_convert32(...) left over 0x%x, converted 0x%x\n", a, b);
    }
    return a|b;
}

// arm->x86
int of_unconvert32(int a)
{
    if(!a || a==-1) return a;
    int b=0;
    #define GO(A) if((a&A)==A) {a&=~A; b|=X86_##A;}
    SUPER();
    #undef GO
    if(a) {
        printf_log(LOG_NONE, "Warning, of_unconvert32(...) left over 0x%x, converted 0x%x\n", a, b);
    }
    return a|b;
}
#undef SUPER

#if 0
EXPORT void* my32__ZGTtnaX (size_t a) { printf("warning _ZGTtnaX called\n"); return NULL; }
EXPORT void my32__ZGTtdlPv (void* a) { printf("warning _ZGTtdlPv called\n"); }
EXPORT uint8_t my32__ITM_RU1(const uint8_t * a) { printf("warning _ITM_RU1 called\n"); return 0; }
EXPORT uint32_t my32__ITM_RU4(const uint32_t * a) { printf("warning _ITM_RU4 called\n"); return 0; }
EXPORT uint64_t my32__ITM_RU8(const uint64_t * a) { printf("warning _ITM_RU8 called\n"); return 0; }
EXPORT void my32__ITM_memcpyRtWn(void * a, const void * b, size_t c) {printf("warning _ITM_memcpyRtWn called\n");  }
EXPORT void my32__ITM_memcpyRnWt(void * a, const void * b, size_t c) {printf("warning _ITM_memcpyRtWn called\n"); }
#endif
EXPORT void my32_longjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val);
EXPORT void my32__longjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val) __attribute__((alias("my32_longjmp")));
EXPORT void my32_siglongjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val) __attribute__((alias("my32_longjmp")));
EXPORT void my32___longjmp_chk(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val) __attribute__((alias("my32_longjmp")));

//EXPORT int32_t my32_setjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p);
//EXPORT int32_t my32__setjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p) __attribute__((alias("my32_setjmp")));
//EXPORT int32_t my32___sigsetjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p) __attribute__((alias("my32_setjmp")));
#if 0
EXPORT void my32_exit(x64emu_t *emu, int32_t status)
{
    R_EAX = (uint32_t)status;
    emu->quit = 1;
}
EXPORT void my32__exit(x64emu_t *emu, int32_t status) __attribute__((alias("my32_exit")));
EXPORT void my32__Exit(x64emu_t *emu, int32_t status) __attribute__((alias("my32_exit")));
#endif
void myStackAlign32(const char* fmt, uint32_t* st, uint64_t* mystack); // align st into mystack according to fmt (for v(f)printf(...))
typedef int (*iFpp_t)(void*, void*);
typedef int (*iFppp_t)(void*, void*, void*);
typedef int (*iFpupp_t)(void*, uint32_t, void*, void*);
EXPORT int my32_printf(x64emu_t *emu, void* fmt, void* b) {
    myStackAlign32((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST_32;
    return vprintf((const char*)fmt, VARARGS_32);
}
EXPORT int my32___printf_chk(x64emu_t *emu, void* fmt, void* b) __attribute__((alias("my32_printf")));
#if 0

EXPORT int my32_vprintf(x64emu_t *emu, void* fmt, void* b) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign32((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vprintf;
    return ((iFpp_t)f)(fmt, VARARGS_32);
    #else
    // other platform don't need that
    return vprintf(fmt, b);
    #endif
}
EXPORT int my32___vprintf_chk(x64emu_t *emu, void* fmt, void* b) __attribute__((alias("my32_vprintf")));

EXPORT int my32_vfprintf(x64emu_t *emu, void* F, void* fmt, void* b) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign32((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vfprintf;
    return ((iFppp_t)f)(F, fmt, VARARGS_32);
    #else
    // other platform don't need that
    return vfprintf(F, fmt, b);
    #endif
}
EXPORT int my32___vfprintf_chk(x64emu_t *emu, void* F, void* fmt, void* b) __attribute__((alias("my32_vfprintf")));
EXPORT int my32__IO_vfprintf(x64emu_t *emu, void* F, void* fmt, void* b) __attribute__((alias("my32_vfprintf")));

EXPORT int my32_dprintf(x64emu_t *emu, int fd, void* fmt, void* V)  {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign32((const char*)fmt, V, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vdprintf;
    return ((iFipp_t)f)(fd, fmt, VARARGS_32);
    #else
    return vdprintf(fd, (const char*)fmt, (va_list)V);
    #endif
}
EXPORT int my32___dprintf_chk(x64emu_t *emu, int fd, void* fmt, void* V) __attribute__((alias("my32_dprintf")));

EXPORT int my32_fprintf(x64emu_t *emu, void* F, void* fmt, void* V)  {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign32((const char*)fmt, V, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vfprintf;
    return ((iFppp_t)f)(F, fmt, VARARGS_32);
    #else
    return vfprintf((FILE*)F, (const char*)fmt, (va_list)V);
    #endif
}
EXPORT int my32___fprintf_chk(x64emu_t *emu, void* F, void* fmt, void* V) __attribute__((alias("my32_fprintf")));

EXPORT int my32_wprintf(x64emu_t *emu, void* fmt, void* V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlignW((const char*)fmt, V, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vwprintf;
    return ((iFpp_t)f)(fmt, VARARGS_32);
    #else
    // other platform don't need that
    return vwprintf((const wchar_t*)fmt, (va_list)V);
    #endif
}
EXPORT int my32___wprintf_chk(x64emu_t *emu, int flag, void* fmt, void* V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlignW((const char*)fmt, V, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vwprintf;
    return ((iFpp_t)f)(fmt, VARARGS_32);
    #else
    // other platform don't need that
    return vwprintf((const wchar_t*)fmt, (va_list)V);
    #endif
}
EXPORT int my32_fwprintf(x64emu_t *emu, void* F, void* fmt, void* V)  {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlignW((const char*)fmt, V, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vfwprintf;
    return ((iFppp_t)f)(F, fmt, VARARGS_32);
    #else
    // other platform don't need that
    return vfwprintf((FILE*)F, (const wchar_t*)fmt, V);
    #endif
}
EXPORT int my32___fwprintf_chk(x64emu_t *emu, void* F, void* fmt, void* V) __attribute__((alias("my32_fwprintf")));

EXPORT int my32_vfwprintf(x64emu_t *emu, void* F, void* fmt, void* b) {
    #ifndef NOALIGN
    myStackAlignW((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vfwprintf;
    return ((iFppp_t)f)(F, fmt, VARARGS_32);
    #else
    return vfwprintf(F, fmt, b);
    #endif
}

EXPORT int my32_vwprintf(x64emu_t *emu, void* fmt, void* b) {
    #ifndef NOALIGN
    myStackAlignW((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vwprintf;
    return ((iFpp_t)f)(fmt, VARARGS_32);
    #else
    void* f = vwprintf;
    return ((iFpp_t)f)(fmt, b);
    #endif
}

EXPORT void *my32_div(void *result, int numerator, int denominator) {
    *(div_t *)result = div(numerator, denominator);
    return result;
}

EXPORT int my32_snprintf(x64emu_t* emu, void* buff, size_t s, void * fmt, void * b) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign32((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vsnprintf;
    return ((iFpLpp_t)f)(buff, s, fmt, VARARGS_32);
    #else
    return vsnprintf((char*)buff, s, (char*)fmt, b);
    #endif
}
EXPORT int my32___snprintf(x64emu_t* emu, void* buff, size_t s, void * fmt, void * b) __attribute__((alias("my32_snprintf")));

EXPORT int my32___snprintf_chk(x64emu_t* emu, void* buff, size_t s, int f1, int f2, void * fmt, void * b) {
    (void)f1; (void)f2;
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign32((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vsnprintf;
    return ((iFpLpp_t)f)(buff, s, fmt, VARARGS_32);
    #else
    return vsnprintf((char*)buff, s, (char*)fmt, b);
    #endif
}


EXPORT int my32_sprintf(x64emu_t* emu, void* buff, void * fmt, void * b) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign32((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vsprintf;
    return ((iFppp_t)f)(buff, fmt, VARARGS_32);
    #else
    return vsprintf((char*)buff, (char*)fmt, b);
    #endif
}
EXPORT int my32___sprintf_chk(x64emu_t* emu, void* buff, void * fmt, void * b) __attribute__((alias("my32_sprintf")));

EXPORT int my32_asprintf(x64emu_t* emu, void** buff, void * fmt, void * b) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign32((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vasprintf;
    return ((iFppp_t)f)(buff, fmt, VARARGS_32);
    #else
    return vasprintf((char**)buff, (char*)fmt, b);
    #endif
}
EXPORT int my32___asprintf(x64emu_t* emu, void** buff, void * fmt, void * b) __attribute__((alias("my32_asprintf")));

EXPORT int my32_vsprintf(x64emu_t* emu, void* buff,  void * fmt, uint32_t * b) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign32((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vsprintf;
    int r = ((iFppp_t)f)(buff, fmt, VARARGS_32);
    return r;
    #else
    void* f = vsprintf;
    int r = ((iFppp_t)f)(buff, fmt, b);
    return r;
    #endif
}
EXPORT int my32___vsprintf_chk(x64emu_t* emu, void* buff, int flags, size_t len, void * fmt, uint32_t * b)  {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign32((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vsprintf;
    int r = ((iFppp_t)f)(buff, fmt, VARARGS_32);
    return r;
    #else
    void* f = vsprintf;
    int r = ((iFppp_t)f)(buff, fmt, b);
    return r;
    #endif
}

#ifdef POWERPCLE
EXPORT int my32_vfscanf(x64emu_t* emu, void* stream, void* fmt, void* b) // probably uneeded to do a GOM, a simple wrap should enough
{
    //myStackAlign32((const char*)fmt, (uint32_t*)b, emu->scratch);
    PREPARE_VALIST_32_(b);
    void* f = vfscanf;

    return ((iFppp_t)f)(stream, fmt, VARARGS_32_(b));
}



EXPORT int my32_vsscanf(x64emu_t* emu, void* stream, void* fmt, void* b)
{
    //myStackAlign32((const char*)fmt, (uint32_t*)b, emu->scratch);
    PREPARE_VALIST_32_(b);
    void* f = vsscanf;

    return ((iFppp_t)f)(stream, fmt, VARARGS_32_(b));
}

EXPORT int my32__vsscanf(x64emu_t* emu, void* stream, void* fmt, void* b) __attribute__((alias("my32_vsscanf")));
EXPORT int my32_sscanf(x64emu_t* emu, void* stream, void* fmt, void* b) __attribute__((alias("my32_vsscanf")));

EXPORT int my32__IO_vfscanf(x64emu_t* emu, void* stream, void* fmt, void* b) __attribute__((alias("my32_vfscanf")));
EXPORT int my32___isoc99_vsscanf(x64emu_t* emu, void* stream, void* fmt, void* b) __attribute__((alias("my32_vsscanf")));

EXPORT int my32___isoc99_vfscanf(x64emu_t* emu, void* stream, void* fmt, void* b) __attribute__((alias("my32_vfscanf")));
EXPORT int my32___isoc99_fscanf(x64emu_t* emu, void* stream, void* fmt, void* b) __attribute__((alias("my32_vfscanf")));

EXPORT int my32___isoc99_sscanf(x64emu_t* emu, void* stream, void* fmt, void* b)
{
  void* f = sscanf;
  PREPARE_VALIST_32;

  return ((iFppp_t)f)(stream, fmt, VARARGS_32);
}
#endif

EXPORT int my32_vsnprintf(x64emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign32((const char*)fmt, (uint32_t*)b, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vsnprintf;
    int r = ((iFpupp_t)f)(buff, s, fmt, VARARGS_32);
    return r;
    #else
    void* f = vsnprintf;
    int r = ((iFpupp_t)f)(buff, s, fmt, (uint32_t*)b);
    return r;
    #endif
}
EXPORT int my32___vsnprintf(x64emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) __attribute__((alias("my32_vsnprintf")));
EXPORT int my32___vsnprintf_chk(x64emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) __attribute__((alias("my32_vsnprintf")));

EXPORT int my32_vasprintf(x64emu_t* emu, void* strp, void* fmt, void* b, va_list V)
{
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign32((const char*)fmt, (uint32_t*)b, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vasprintf;
    int r = ((iFppp_t)f)(strp, fmt, VARARGS_32);
    return r;
    #else
    void* f = vasprintf;
    int r = ((iFppp_t)f)(strp, fmt, (uint32_t*)b);
    return r;
    #endif
}
EXPORT int my32___vasprintf_chk(x64emu_t* emu, void* strp, int flags, void* fmt, void* b, va_list V)
{
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign32((const char*)fmt, (uint32_t*)b, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vasprintf;
    int r = ((iFppp_t)f)(strp, fmt, VARARGS_32);
    return r;
    #else
    void* f = vasprintf;
    int r = ((iFppp_t)f)(strp, fmt, (uint32_t*)b);
    return r;
    #endif
}

EXPORT int my32___asprintf_chk(x64emu_t* emu, void* result_ptr, int flags, void* fmt, void* b)
{
    #ifndef NOALIGN
    myStackAlign32((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vasprintf;
    return ((iFppp_t)f)(result_ptr, fmt, VARARGS_32);
    #else
    return vasprintf((char**)result_ptr, (char*)fmt, b);
    #endif
}

EXPORT int my32_vswprintf(x64emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlignW((const char*)fmt, (uint32_t*)b, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vswprintf;
    int r = ((iFpupp_t)f)(buff, s, fmt, VARARGS_32);
    return r;
    #else
    void* f = vswprintf;
    int r = ((iFpupp_t)f)(buff, s, fmt, (uint32_t*)b);
    return r;
    #endif
}
EXPORT int my32___vswprintf(x64emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) __attribute__((alias("my32_vswprintf")));
EXPORT int my32___vswprintf_chk(x64emu_t* emu, void* buff, size_t s, int flags, size_t m, void * fmt, void * b, va_list V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlignW((const char*)fmt, (uint32_t*)b, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vswprintf;
    int r = ((iFpupp_t)f)(buff, s, fmt, VARARGS_32);
    return r;
    #else
    void* f = vswprintf;
    int r = ((iFpupp_t)f)(buff, s, fmt, (uint32_t*)b);
    return r;
    #endif
}

EXPORT void my32_verr(x64emu_t* emu, int eval, void* fmt, void* b) {
    #ifndef NOALIGN
    myStackAlignW((const char*)fmt, (uint32_t*)b, emu->scratch);
    PREPARE_VALIST_32;
    void* f = verr;
    ((vFipp_t)f)(eval, fmt, VARARGS_32);
    #else
    void* f = verr;
    ((vFipp_t)f)(eval, fmt, (uint32_t*)b);
    #endif
}

EXPORT void my32_vwarn(x64emu_t* emu, void* fmt, void* b) {
    #ifndef NOALIGN
    myStackAlignW((const char*)fmt, (uint32_t*)b, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vwarn;
    ((vFpp_t)f)(fmt, VARARGS_32);
    #else
    void* f = vwarn;
    ((vFpp_t)f)(fmt, (uint32_t*)b);
    #endif
}

EXPORT int my32___swprintf_chk(x64emu_t* emu, void* s, uint32_t n, int32_t flag, uint32_t slen, void* fmt, void * b)
{
    #ifndef NOALIGN
    myStackAlignW((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vswprintf;
    int r = ((iFpupp_t)f)(s, n, fmt, VARARGS_32);
    return r;
    #else
    void* f = vswprintf;
    int r = ((iFpupp_t)f)(s, n, fmt, b);
    return r;
    #endif
}
EXPORT int my32_swprintf(x64emu_t* emu, void* s, uint32_t n, void* fmt, void *b)
{
    #ifndef NOALIGN
    myStackAlignW((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST_32;
    void* f = vswprintf;
    int r = ((iFpupp_t)f)(s, n, fmt, VARARGS_32);
    return r;
    #else
    void* f = vswprintf;
    int r = ((iFpupp_t)f)(s, n, fmt, b);
    return r;
    #endif
}

EXPORT void my32__ITM_addUserCommitAction(x64emu_t* emu, void* cb, uint32_t b, void* c)
{
    // disabled for now... Are all this _ITM_ stuff really mendatory?
    #if 0
    // quick and dirty... Should store the callback to be removed later....
    libc_my32_t *my = (libc_my32_t *)emu->context->libclib->priv.w.p2;
    x64emu_t *cbemu = AddCallback(emu, (uintptr_t)cb, 1, c, NULL, NULL, NULL);
    my->_ITM_addUserCommitAction(libc1ArgCallback, b, cbemu);
    // should keep track of cbemu to remove at some point...
    #else
    printf("warning _ITM_addUserCommitAction called\n");
    #endif
}
#endif
EXPORT void my32__ITM_registerTMCloneTable(x64emu_t* emu, void* p, uint32_t s) {}
EXPORT void my32__ITM_deregisterTMCloneTable(x64emu_t* emu, void* p) {}


struct i386_stat {
	uint64_t  st_dev;
	uint32_t  __pad1;
	uint32_t  st_ino;
	uint32_t  st_mode;
	uint32_t  st_nlink;
	uint32_t  st_uid;
	uint32_t  st_gid;
	uint64_t  st_rdev;
	uint32_t  __pad2;
	int32_t   st_size;
	int32_t   st_blksize;
	int32_t   st_blocks;
	int32_t   st_atime_sec;
	uint32_t  st_atime_nsec;
	int32_t   st_mtime_sec;
	uint32_t  st_mtime_nsec;
	int32_t   st_ctime_sec;
	uint32_t  st_ctime_nsec;
	uint32_t  __unused4;
	uint32_t  __unused5;
} __attribute__((packed));

static int FillStatFromStat64(int vers, const struct stat64 *st64, void *st32)
{
    struct i386_stat *i386st = (struct i386_stat *)st32;

    if (vers != 3)
    {
        errno = EINVAL;
        return -1;
    }

    i386st->st_dev = st64->st_dev;
    i386st->__pad1 = 0;
    if (fix_64bit_inodes)
    {
        i386st->st_ino = st64->st_ino ^ (st64->st_ino >> 32);
    }
    else
    {
        i386st->st_ino = st64->st_ino;
        if ((st64->st_ino >> 32) != 0)
        {
            errno = EOVERFLOW;
            return -1;
        }
    }
    i386st->st_mode = st64->st_mode;
    i386st->st_nlink = st64->st_nlink;
    i386st->st_uid = st64->st_uid;
    i386st->st_gid = st64->st_gid;
    i386st->st_rdev = st64->st_rdev;
    i386st->__pad2 = 0;
    i386st->st_size = st64->st_size;
    if ((i386st->st_size >> 31) != (int32_t)(st64->st_size >> 32))
    {
        errno = EOVERFLOW;
        return -1;
    }
    i386st->st_blksize = st64->st_blksize;
    i386st->st_blocks = st64->st_blocks;
    if ((i386st->st_blocks >> 31) != (int32_t)(st64->st_blocks >> 32))
    {
        errno = EOVERFLOW;
        return -1;
    }
    i386st->st_atime_sec = st64->st_atim.tv_sec;
    i386st->st_atime_nsec = st64->st_atim.tv_nsec;
    i386st->st_mtime_sec = st64->st_mtim.tv_sec;
    i386st->st_mtime_nsec = st64->st_mtim.tv_nsec;
    i386st->st_ctime_sec = st64->st_ctim.tv_sec;
    i386st->st_ctime_nsec = st64->st_ctim.tv_nsec;
    i386st->__unused4 = 0;
    i386st->__unused5 = 0;
    return 0;
}
#if 0
#ifdef ANDROID
EXPORT int my32_stat(char* path, void* buf)
{
    struct stat64 st;
    int r = stat64(path, &st);
    UnalignStat64(&st, buf);
    return r;
}

EXPORT int my32_fstat(int fd, void* buf)
{
    struct stat64 st;
    int r = fstat64(fd, &st);
    UnalignStat64(&st, buf);
    return r;
}

EXPORT int my32_lstat(char* path, void* buf)
{
    struct stat64 st;
    int r = lstat64(path, &st);
    UnalignStat64(&st, buf);
    return r;
}
#endif

EXPORT int my32___fxstat(x64emu_t *emu, int vers, int fd, void* buf)
{
    if (vers == 1)
    {
        static iFiip_t f = NULL;
        if(!f) {
            library_t* lib = my_lib;
            if(!lib)
            {
                errno = EINVAL;
                return -1;
            }
            f = (iFiip_t)dlsym(lib->priv.w.lib, "__fxstat");
        }

        return f(vers, fd, buf);
    }
    struct stat64 st;
    int r = fstat64(fd, &st);
    if (r) return r;
    r = FillStatFromStat64(vers, &st, buf);
    return r;
}

EXPORT int my32___fxstat64(x64emu_t *emu, int vers, int fd, void* buf)
{
    struct stat64 st;
    int r = fstat64(fd, &st);
    //int r = syscall(__NR_stat64, fd, &st);
    UnalignStat64(&st, buf);
    return r;
}

EXPORT int my32_stat64(x64emu_t* emu, void* path, void* buf)
{
    struct stat64 st;
    int r = stat64(path, &st);
    UnalignStat64(&st, buf);
    return r;
}
EXPORT int my32_lstat64(x64emu_t* emu, void* path, void* buf)
{
    struct stat64 st;
    int r = lstat64(path, &st);
    UnalignStat64(&st, buf);
    return r;
}

EXPORT int my32___xstat(x64emu_t* emu, int v, void* path, void* buf)
{
    if (v == 1)
    {
        static iFipp_t f = NULL;
        if(!f) {
            library_t* lib = my_lib;
            if(!lib)
            {
                errno = EINVAL;
                return -1;
            }
            f = (iFipp_t)dlsym(lib->priv.w.lib, "__xstat");
        }

        return f(v, path, buf);
    }
    struct stat64 st;
    int r = stat64((const char*)path, &st);
    if (r) return r;
    r = FillStatFromStat64(v, &st, buf);
    return r;
}

EXPORT int my32___xstat64(x64emu_t* emu, int v, void* path, void* buf)
{
    struct stat64 st;
    int r = stat64((const char*)path, &st);
    UnalignStat64(&st, buf);
    return r;
}

EXPORT int my32___lxstat(x64emu_t* emu, int v, void* name, void* buf)
{
    if (v == 1)
    {
        static iFipp_t f = NULL;
        if(!f) {
            library_t* lib = my_lib;
            if(!lib)
            {
                errno = EINVAL;
                return -1;
            }
            f = (iFipp_t)dlsym(lib->priv.w.lib, "__lxstat");
        }

        return f(v, name, buf);
    }
    struct stat64 st;
    int r = lstat64((const char*)name, &st);
    if (r) return r;
    r = FillStatFromStat64(v, &st, buf);
    return r;
}

EXPORT int my32___lxstat64(x64emu_t* emu, int v, void* name, void* buf)
{
    struct stat64 st;
    int r = lstat64((const char*)name, &st);
    UnalignStat64(&st, buf);
    return r;
}

EXPORT int my32___fxstatat(x64emu_t* emu, int v, int d, void* path, void* buf, int flags)
{
    struct  stat64 st;
    int r = fstatat64(d, path, &st, flags);
    if (r) return r;
    r = FillStatFromStat64(v, &st, buf);
    return r;
}

EXPORT int my32___fxstatat64(x64emu_t* emu, int v, int d, void* path, void* buf, int flags)
{
    struct  stat64 st;
    int r = fstatat64(d, path, &st, flags);
    UnalignStat64(&st, buf);
    return r;
}

EXPORT int my32__IO_file_stat(x64emu_t* emu, void* f, void* buf)
{
    struct stat64 st;
    libc_my32_t *my = (libc_my32_t *)emu->context->libclib->priv.w.p2;
    int r = my->_IO_file_stat(f, &st);
    UnalignStat64(&st, buf);
    return r;
}

EXPORT int my32_fstatfs64(int fd, void* buf)
{
    struct statfs64 st;
    int r = fstatfs64(fd, &st);
    UnalignStatFS64(&st, buf);
    return r;
}

EXPORT int my32_statfs64(const char* path, void* buf)
{
    struct statfs64 st;
    int r = statfs64(path, &st);
    UnalignStatFS64(&st, buf);
    return r;
}


#ifdef ANDROID
typedef int (*__compar_d_fn_t)(const void*, const void*, void*);

static size_t qsort_r_partition(void* base, size_t size, __compar_d_fn_t compar, void* arg, size_t lo, size_t hi)
{
    void* tmp = malloc(size);
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
    free(tmp);
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

static int my32_compare_r_cb(void* a, void* b, compare_r_t* arg)
{
    return (int)RunFunctionWithEmu(arg->emu, 0, arg->f, 2+arg->r, a, b, arg->data);
}
EXPORT void my32_qsort(x64emu_t* emu, void* base, size_t nmemb, size_t size, void* fnc)
{
    compare_r_t args;
    args.emu = emu; args.f = (uintptr_t)fnc; args.r = 0; args.data = NULL;
    qsort_r(base, nmemb, size, (__compar_d_fn_t)my32_compare_r_cb, &args);
}
EXPORT void my32_qsort_r(x64emu_t* emu, void* base, size_t nmemb, size_t size, void* fnc, void* data)
{
    compare_r_t args;
    args.emu = emu; args.f = (uintptr_t)fnc; args.r = 1; args.data = data;
    qsort_r(base, nmemb, size, (__compar_d_fn_t)my32_compare_r_cb, &args);
}

EXPORT void* my32_bsearch(x64emu_t* emu, void* key, void* base, size_t nmemb, size_t size, void* fnc)
{
    return bsearch(key, base, nmemb, size, findcompareFct(fnc));
}

EXPORT void* my32_lsearch(x64emu_t* emu, void* key, void* base, size_t* nmemb, size_t size, void* fnc)
{
    return lsearch(key, base, nmemb, size, findcompareFct(fnc));
}
EXPORT void* my32_lfind(x64emu_t* emu, void* key, void* base, size_t* nmemb, size_t size, void* fnc)
{
    return lfind(key, base, nmemb, size, findcompareFct(fnc));
}


struct i386_dirent {
    uint32_t d_ino;
    int32_t  d_off;
    uint16_t d_reclen;
    uint8_t  d_type;
    char     d_name[256];
};

EXPORT void* my32_readdir(x64emu_t* emu, void* dirp)
{
    if (fix_64bit_inodes)
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
            f = (pFp_t)dlsym(lib->priv.w.lib, "readdir");
        }

        return f(dirp);
    }
}

EXPORT int32_t my32_readdir_r(x64emu_t* emu, void* dirp, void* entry, void** result)
{
    struct dirent64 d64, *dp64;
    if (fix_64bit_inodes && (sizeof(d64.d_name) > 1))
    {
        static iFppp_t f = NULL;
        if(!f) {
            library_t* lib = my_lib;
            if(!lib)
            {
                *result = NULL;
                return 0;
            }
            f = (iFppp_t)dlsym(lib->priv.w.lib, "readdir64_r");
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
            f = (iFppp_t)dlsym(lib->priv.w.lib, "readdir_r");
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

EXPORT int32_t my32_readlink(x64emu_t* emu, void* path, void* buf, uint32_t sz)
{
    if(isProcSelf((const char*)path, "exe")) {
        // special case for self...
        return strlen(strncpy((char*)buf, emu->context->fullpath, sz));
    }
    return readlink((const char*)path, (char*)buf, sz);
}
#ifndef NOALIGN

void CreateCPUInfoFile(int fd);
int getNCpu();
static int isCpuTopology(const char* p) {
    if(strstr(p, "/sys/devices/system/cpu/cpu")!=p)
        return -1;  //nope
    if( FileExist(p, -1))
        return -1;  //no need to fake it
    char buf[512];
    const char* p2 = p + strlen("/sys/devices/system/cpu/cpu");
    int n = 0;
    while(*p2>='0' && *p2<='9') {
        n = n*10+ *p2 - '0';
        ++p2;
    }
    if(n>=getNCpu()) // filter for non existing cpu
        return -1;
    snprintf(buf, 512, "/sys/devices/system/cpu/cpu%d/topology/core_id", n);
    if(!strcmp(p, buf))
        return n;
    return -1;
}
static void CreateCPUTopologyCoreID(int fd, int cpu)
{
    char buf[512];
    snprintf(buf, 512, "%d\n", cpu);
    size_t dummy = write(fd, buf, strlen(buf));
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
#endif
#define TMP_CPUINFO "box32_tmpcpuinfo"
#define TMP_CPUTOPO "box32_tmpcputopo%d"

#define TMP_MEMMAP  "box32_tmpmemmap"
#define TMP_CMDLINE "box32_tmpcmdline"
EXPORT int32_t my32_open(x64emu_t* emu, void* pathname, int32_t flags, uint32_t mode)
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
    if(isCpuTopology((const char*)pathname)!=-1) {
        int n = isCpuTopology((const char*)pathname);
        char buf[512];
        snprintf(buf, 512, TMP_CPUTOPO, n);
        int tmp = shm_open(buf, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return open(pathname, flags, mode); // error fallback
        shm_unlink(buf);    // remove the shm file, but it will still exist because it's currently in use
        CreateCPUTopologyCoreID(tmp, n);
        lseek(tmp, 0, SEEK_SET);
        return tmp;
    }
    #endif
    int ret = open(pathname, flags, mode);
    return ret;
}
EXPORT int32_t my32___open(x64emu_t* emu, void* pathname, int32_t flags, uint32_t mode) __attribute__((alias("my32_open")));

#ifdef DYNAREC
static int hasDBFromAddress(uintptr_t addr)
{
    int idx = (addr>>DYNAMAP_SHIFT);
    return getDB(idx)?1:0;
}
#endif

EXPORT int32_t my32_read(int fd, void* buf, uint32_t count)
{
    int ret = read(fd, buf, count);
#ifdef DYNAREC
    if(ret!=count && ret>0) {
        // continue reading...
        void* p = buf+ret;
        if(hasDBFromAddress((uintptr_t)p)) {
            // allow writing the whole block (this happens with HalfLife, libMiles load code directly from .mix and other file like that)
            unprotectDB((uintptr_t)p, count-ret, 1);
            int l;
            do {
                l = read(fd, p, count-ret); 
                if(l>0) {
                    p+=l; ret+=l;
                }
            } while(l>0);
        }
    }
#endif
    return ret;
}

EXPORT int32_t my32_open64(x64emu_t* emu, void* pathname, int32_t flags, uint32_t mode)
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
    if(isCpuTopology((const char*)pathname)!=-1) {
        int n = isCpuTopology((const char*)pathname);
        char buf[512];
        snprintf(buf, 512, TMP_CPUTOPO, n);
        int tmp = shm_open(buf, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return open64(pathname, flags, mode); // error fallback
        shm_unlink(buf);    // remove the shm file, but it will still exist because it's currently in use
        CreateCPUTopologyCoreID(tmp, n);
        lseek(tmp, 0, SEEK_SET);
        return tmp;
    }
    #endif
    return open64(pathname, flags, mode);
}

EXPORT FILE* my32_fopen(x64emu_t* emu, const char* path, const char* mode)
{
    if(isProcSelf(path, "maps")) {
        // special case for self memory map
        int tmp = shm_open(TMP_MEMMAP, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return fopen(path, mode); // error fallback
        shm_unlink(TMP_MEMMAP);    // remove the shm file, but it will still exist because it's currently in use
        CreateMemorymapFile(emu->context, tmp);
        lseek(tmp, 0, SEEK_SET);
        return fdopen(tmp, mode);
    }
    #ifndef NOALIGN
    if(strcmp(path, "/proc/cpuinfo")==0) {
        // special case for cpuinfo
        int tmp = shm_open(TMP_CPUINFO, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return fopen(path, mode); // error fallback
        shm_unlink(TMP_CPUINFO);    // remove the shm file, but it will still exist because it's currently in use
        CreateCPUInfoFile(tmp);
        lseek(tmp, 0, SEEK_SET);
        return fdopen(tmp, mode);
    }
    if(isCpuTopology(path)!=-1) {
        int n = isCpuTopology(path);
        char buf[512];
        snprintf(buf, 512, TMP_CPUTOPO, n);
        int tmp = shm_open(buf, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return fopen(path, mode); // error fallback
        shm_unlink(buf);    // remove the shm file, but it will still exist because it's currently in use
        CreateCPUTopologyCoreID(tmp, n);
        lseek(tmp, 0, SEEK_SET);
        return fdopen(tmp, mode);;
    }
    #endif
    if(isProcSelf(path, "exe")) {
        return fopen(emu->context->fullpath, mode);
    }
    return fopen(path, mode);
}

EXPORT FILE* my32_fopen64(x64emu_t* emu, const char* path, const char* mode)
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
    if(isCpuTopology(path)!=-1) {
        int n = isCpuTopology(path);
        char buf[512];
        snprintf(buf, 512, TMP_CPUTOPO, n);
        int tmp = shm_open(buf, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return fopen(path, mode); // error fallback
        shm_unlink(buf);    // remove the shm file, but it will still exist because it's currently in use
        CreateCPUTopologyCoreID(tmp, n);
        lseek(tmp, 0, SEEK_SET);
        return fdopen(tmp, mode);;
    }
    #endif
    if(isProcSelf(path, "exe")) {
        return fopen64(emu->context->fullpath, mode);
    }
    return fopen64(path, mode);
}

#if 0
EXPORT int my32_mkstemps64(x64emu_t* emu, char* template, int suffixlen)
{
    library_t* lib = my_lib;
    if(!lib) return 0;
    void* f = dlsym(lib->priv.w.lib, "mkstemps64");
    if(f)
        return ((iFpi_t)f)(template, suffixlen);
    // implement own version...
    // TODO: check size of template, and if really XXXXXX is there
    char* fname = strdup(template);
    do {
        strcpy(fname, template);
        char num[8];
        sprintf(num, "%06d", rand()%999999);
        memcpy(fname+strlen(fname)-suffixlen-6, num, 6);
    } while(!FileExist(fname, -1));
    int ret = open64(fname, O_EXCL);
    free(fname);
    return ret;
}

EXPORT int32_t my32_ftw(x64emu_t* emu, void* pathname, void* B, int32_t nopenfd)
{
    static iFppi_t f = NULL;
    if(!f) {
        library_t* lib = my_lib;
        if(!lib) return 0;
        f = (iFppi_t)dlsym(lib->priv.w.lib, "ftw");
    }

    return f(pathname, findftwFct(B), nopenfd);
}

EXPORT int32_t my32_nftw(x64emu_t* emu, void* pathname, void* B, int32_t nopenfd, int32_t flags)
{
    static iFppii_t f = NULL;
    if(!f) {
        library_t* lib = my_lib;
        if(!lib) return 0;
        f = (iFppii_t)dlsym(lib->priv.w.lib, "nftw");
    }

    return f(pathname, findnftwFct(B), nopenfd, flags);
}

EXPORT void* my32_ldiv(x64emu_t* emu, void* p, int32_t num, int32_t den)
{
    *((ldiv_t*)p) = ldiv(num, den);
    return p;
}

#ifndef NOALIGN
EXPORT int my32_epoll_create(x64emu_t* emu, int size)
{
    return epoll_create(size);
}
EXPORT int my32_epoll_create1(x64emu_t* emu, int flags)
{
    return epoll_create1(flags);
}
EXPORT int32_t my32_epoll_ctl(x64emu_t* emu, int32_t epfd, int32_t op, int32_t fd, void* event)
{
    struct epoll_event _event[1] = {0};
    if(event && (op!=EPOLL_CTL_DEL))
        AlignEpollEvent(_event, event, 1);
    return epoll_ctl(epfd, op, fd, event?_event:event);
}
EXPORT int32_t my32_epoll_wait(x64emu_t* emu, int32_t epfd, void* events, int32_t maxevents, int32_t timeout)
{
    struct epoll_event _events[maxevents];
    //AlignEpollEvent(_events, events, maxevents);
    int32_t ret = epoll_wait(epfd, events?_events:NULL, maxevents, timeout);
    if(ret>0)
        UnalignEpollEvent(events, _events, ret);
    return ret;
}
#endif

EXPORT int32_t my32_glob(x64emu_t *emu, void* pat, int32_t flags, void* errfnc, void* pglob)
{
    static iFpipp_t f = NULL;
    if(!f) {
        library_t* lib = my_lib;
        if(!lib) return 0;
        f = (iFpipp_t)dlsym(lib->priv.w.lib, "glob");
    }

    return f(pat, flags, findgloberrFct(errfnc), pglob);
}

#ifndef ANDROID
EXPORT int32_t my32_glob64(x64emu_t *emu, void* pat, int32_t flags, void* errfnc, void* pglob)
{
    return glob64(pat, flags, findgloberrFct(errfnc), pglob);
}
#endif

EXPORT int my32_scandir64(x64emu_t *emu, void* dir, void* namelist, void* sel, void* comp)
{
    return scandir64(dir, namelist, findfilter64Fct(sel), findcompare64Fct(comp));
}

EXPORT int my32_scandir(x64emu_t *emu, void* dir, void* namelist, void* sel, void* comp)
{
    static iFpppp_t f = NULL;
    if(!f) {
        library_t* lib = my_lib;
        if(!lib) return 0;
        f = (iFpppp_t)dlsym(lib->priv.w.lib, "scandir");
    }

    return f(dir, namelist, findfilter_dirFct(sel), findcompare_dirFct(comp));
}

EXPORT int my32_ftw64(x64emu_t* emu, void* filename, void* func, int descriptors)
{
    return ftw64(filename, findftw64Fct(func), descriptors);
}

EXPORT int32_t my32_nftw64(x64emu_t* emu, void* pathname, void* B, int32_t nopenfd, int32_t flags)
{
    return nftw64(pathname, findnftw64Fct(B), nopenfd, flags);
}

EXPORT int32_t my32_execv(x64emu_t* emu, const char* path, char* const argv[])
{
    int self = isProcSelf(path, "exe");
    int x86 = FileIsX86ELF(path);
    int x64 = my_context->box64path?FileIsX64ELF(path):0;
    printf_log(LOG_DEBUG, "execv(\"%s\", %p) is x86=%d\n", path, argv, x86);
    if (x86 || x64 || self) {
        int skip_first = 0;
        if(strlen(path)>=strlen("wine-preloader") && strcmp(path+strlen(path)-strlen("wine-preloader"), "wine-preloader")==0)
            skip_first++;
        // count argv...
        int n=skip_first;
        while(argv[n]) ++n;
        const char** newargv = (const char**)calloc(n+2, sizeof(char*));
        newargv[0] = x64?emu->context->box64path:emu->context->box64path;
        memcpy(newargv+1, argv+skip_first, sizeof(char*)*(n+1));
        if(self) newargv[1] = emu->context->fullpath;
        printf_log(LOG_DEBUG, " => execv(\"%s\", %p [\"%s\", \"%s\", \"%s\"...:%d])\n", emu->context->box64path, newargv, newargv[0], n?newargv[1]:"", (n>1)?newargv[2]:"",n);
        int ret = execv(newargv[0], (char* const*)newargv);
        free(newargv);
        return ret;
    }
    return execv(path, argv);
}
#endif
EXPORT int32_t my32_execve(x64emu_t* emu, const char* path, char* const argv[], char* const envp[])
{
    int self = isProcSelf(path, "exe");
    int x86 = FileIsX86ELF(path);
    int x64 = my_context->box64path?FileIsX64ELF(path):0;
    // hack to update the environ var if needed
    if(envp == my_context->envv && environ) {
        envp = environ;
    }
    printf_log(LOG_DEBUG, "execve(\"%s\", %p, %p) is x86=%d\n", path, argv, envp, x86);
    if (x86 || x64 || self) {
        int skip_first = 0;
        if(strlen(path)>=strlen("wine-preloader") && strcmp(path+strlen(path)-strlen("wine-preloader"), "wine-preloader")==0)
            skip_first++;
        // count argv...
        int n=skip_first;
        while(argv[n]) ++n;
        const char** newargv = (const char**)calloc(n+2, sizeof(char*));
        newargv[0] = x64?emu->context->box64path:emu->context->box64path;
        memcpy(newargv+1, argv+skip_first, sizeof(char*)*(n+1));
        if(self) newargv[1] = emu->context->fullpath;
        printf_log(LOG_DEBUG, " => execve(\"%s\", %p [\"%s\", \"%s\", \"%s\"...:%d])\n", emu->context->box64path, newargv, newargv[0], n?newargv[1]:"", (n>1)?newargv[2]:"",n);
        int ret = execve(newargv[0], (char* const*)newargv, envp);
        free(newargv);
        return ret;
    }
    if(!strcmp(path + strlen(path) - strlen("/uname"), "/uname")
     && argv[1] && (!strcmp(argv[1], "-m") || !strcmp(argv[1], "-p") || !strcmp(argv[1], "-i"))
     && !argv[2]) {
        // uname -m is redirected to box32 -m
        path = my_context->box64path;
        char *argv2[3] = { my_context->box64path, argv[1], NULL };
        return execve(path, argv2, envp);
    }

    return execve(path, argv, envp);
}
#if 0
// execvp should use PATH to search for the program first
EXPORT int32_t my32_execvp(x64emu_t* emu, const char* path, char* const argv[])
{
    // need to use BOX32_PATH / PATH here...
    char* fullpath = ResolveFile(path, &my_context->box32_path);
    // use fullpath...
    int self = isProcSelf(fullpath, "exe");
    int x86 = FileIsX86ELF(fullpath);
    int x64 = my_context->box64path?FileIsX64ELF(path):0;
    printf_log(LOG_DEBUG, "execvp(\"%s\", %p), IsX86=%d / fullpath=\"%s\"\n", path, argv, x86, fullpath);
    free(fullpath);
    if (x86 || self) {
        // count argv...
        int i=0;
        while(argv[i]) ++i;
        char** newargv = (char**)calloc(i+2, sizeof(char*));
        newargv[0] = x64?emu->context->box64path:emu->context->box64path;
        for (int j=0; j<i; ++j)
            newargv[j+1] = argv[j];
        if(self) newargv[1] = emu->context->fullpath;
        printf_log(LOG_DEBUG, " => execvp(\"%s\", %p [\"%s\", \"%s\"...:%d])\n", newargv[0], newargv, newargv[1], i?newargv[2]:"", i);
        int ret = execvp(newargv[0], newargv);
        free(newargv);
        return ret;
    }
    if((!strcmp(path + strlen(path) - strlen("/uname"), "/uname") || !strcmp(path, "uname"))
     && argv[1] && (!strcmp(argv[1], "-m") || !strcmp(argv[1], "-p") || !strcmp(argv[1], "-i"))
     && !argv[2]) {
        // uname -m is redirected to box32 -m
        path = my_context->box64path;
        char *argv2[3] = { my_context->box64path, argv[1], NULL };
        return execvp(path, argv2);
    }

    // fullpath is gone, so the search will only be on PATH, not on BOX32_PATH (is that an issue?)
    return execvp(path, argv);
}

// execvp should use PATH to search for the program first
EXPORT int32_t my32_posix_spawnp(x64emu_t* emu, pid_t* pid, const char* path, 
    const posix_spawn_file_actions_t *actions, const posix_spawnattr_t* attrp,  char* const argv[], char* const envp[])
{
    // need to use BOX32_PATH / PATH here...
    char* fullpath = ResolveFile(path, &my_context->box32_path);
    // use fullpath...
    int self = isProcSelf(fullpath, "exe");
    int x86 = FileIsX86ELF(fullpath);
    int x64 = my_context->box64path?FileIsX64ELF(path):0;
    printf_log(LOG_DEBUG, "posix_spawnp(%p, \"%s\", %p, %p, %p, %p), IsX86=%d / fullpath=\"%s\"\n", pid, path, actions, attrp, argv, envp, x86, fullpath);
    free(fullpath);
    if ((x86 || self)) {
        // count argv...
        int i=0;
        while(argv[i]) ++i;
        char** newargv = (char**)calloc(i+2, sizeof(char*));
        newargv[0] = x64?emu->context->box64path:emu->context->box64path;
        for (int j=0; j<i; ++j)
            newargv[j+1] = argv[j];
        if(self) newargv[1] = emu->context->fullpath;
        printf_log(LOG_DEBUG, " => posix_spawnp(%p, \"%s\", %p, %p, %p [\"%s\", \"%s\"...:%d], %p)\n", pid, newargv[0], actions, attrp, newargv, newargv[1], i?newargv[2]:"", i, envp);
        int ret = posix_spawnp(pid, newargv[0], actions, attrp, newargv, envp);
        printf_log(LOG_DEBUG, "posix_spawnp returned %d\n", ret);
        //free(newargv);
        return ret;
    }
    // fullpath is gone, so the search will only be on PATH, not on BOX32_PATH (is that an issue?)
    return posix_spawnp(pid, path, actions, attrp, argv, envp);
}
#endif
EXPORT void my32__Jv_RegisterClasses() {}

EXPORT int32_t my32___cxa_thread_atexit_impl(x64emu_t* emu, void* dtor, void* obj, void* dso)
{
    printf_log(LOG_INFO, "Warning, call to __cxa_thread_atexit_impl(%p, %p, %p) ignored\n", dtor, obj, dso);
    return 0;
}
#if 0
#ifndef ANDROID
extern void __chk_fail();
EXPORT unsigned long int my32___fdelt_chk (unsigned long int d)
{
  if (d >= FD_SETSIZE)
    __chk_fail ();

  return d / __NFDBITS;
}
#endif

EXPORT int32_t my32_getrandom(x64emu_t* emu, void* buf, uint32_t buflen, uint32_t flags)
{
    // not always implemented on old linux version...
    library_t* lib = my_lib;
    if(!lib) return 0;
    void* f = dlsym(lib->priv.w.lib, "getrandom");
    if(f)
        return ((iFpuu_t)f)(buf, buflen, flags);
    // do what should not be done, but it's better then nothing....
    FILE * rnd = fopen("/dev/urandom", "rb");
    uint32_t r = fread(buf, 1, buflen, rnd);
    fclose(rnd);
    return r;
}

static struct passwd fakepwd = {};
EXPORT void* my32_getpwuid(x64emu_t* emu, uint32_t uid)
{
    void *ret = NULL;
    library_t* lib = my_lib;
    if(!lib) return 0;
    void* f = dlsym(lib->priv.w.lib, "getpwuid");
    if(f)
        ret = ((pFu_t)f)(uid);
    
    // In case of failure, provide a fake one. Evil hack :/
    if (!ret && !fakepwd.pw_name) {
        fakepwd.pw_name = strdup("root");
        fakepwd.pw_passwd = strdup("fakehash");
        fakepwd.pw_uid = 0;
        fakepwd.pw_gid = 0;
        fakepwd.pw_gecos = strdup("root");
        fakepwd.pw_dir = getenv("HOME");
        fakepwd.pw_shell = strdup("/bin/bash");
    }

    return ret ? ret : (void*)&fakepwd;
}

EXPORT int32_t my32_recvmmsg(x64emu_t* emu, int32_t fd, void* msgvec, uint32_t vlen, uint32_t flags, void* timeout)
{
    // Implemented starting glibc 2.12+
    library_t* lib = my_lib;
    if(!lib) return 0;
    void* f = dlsym(lib->priv.w.lib, "recvmmsg");
    if(f)
        return ((iFipuup_t)f)(fd, msgvec, vlen, flags, timeout);
    // Use the syscall
    return syscall(__NR_recvmmsg, fd, msgvec, vlen, flags, timeout);
}

EXPORT int32_t my32___sendmmsg(x64emu_t* emu, int32_t fd, void* msgvec, uint32_t vlen, uint32_t flags)
{
    // Implemented starting glibc 2.14+
    library_t* lib = my_lib;
    if(!lib) return 0;
    void* f = dlsym(lib->priv.w.lib, "__sendmmsg");
    if(f)
        return ((iFipuu_t)f)(fd, msgvec, vlen, flags);
    // Use the syscall
    return syscall(__NR_sendmmsg, fd, msgvec, vlen, flags);
}

EXPORT int32_t my32___register_atfork(x64emu_t *emu, void* prepare, void* parent, void* child, void* handle)
{
    // this is partly incorrect, because the emulated funcionts should be executed by actual fork and not by my32_atfork...
    if(my_context->atfork_sz==my_context->atfork_cap) {
        my_context->atfork_cap += 4;
        my_context->atforks = (atfork_fnc_t*)realloc(my_context->atforks, my_context->atfork_cap*sizeof(atfork_fnc_t));
    }
    my_context->atforks[my_context->atfork_sz].prepare = (uintptr_t)prepare;
    my_context->atforks[my_context->atfork_sz].parent = (uintptr_t)parent;
    my_context->atforks[my_context->atfork_sz].child = (uintptr_t)child;
    my_context->atforks[my_context->atfork_sz].handle = handle;
    return 0;
}

EXPORT uint64_t my32___umoddi3(uint64_t a, uint64_t b)
{
    return a%b;
}
EXPORT uint64_t my32___udivdi3(uint64_t a, uint64_t b)
{
    return a/b;
}
EXPORT int64_t my32___divdi3(int64_t a, int64_t b)
{
    return a/b;
}

EXPORT int32_t my32___poll_chk(void* a, uint32_t b, int c, int l)
{
    return poll(a, b, c);   // no check...
}

EXPORT int32_t my32_fcntl64(x64emu_t* emu, int32_t a, int32_t b, uint32_t d1, uint32_t d2, uint32_t d3, uint32_t d4, uint32_t d5, uint32_t d6)
{
    // Implemented starting glibc 2.14+
    library_t* lib = my_lib;
    if(!lib) return 0;
    iFiiV_t f = dlsym(lib->priv.w.lib, "fcntl64");
    if(b==F_SETFL)
        d1 = of_convert32(d1);
    if(b==F_GETLK64 || b==F_SETLK64 || b==F_SETLKW64)
    {
        my32_flock64_t fl;
        AlignFlock64(&fl, (void*)d1);
        int ret = f?f(a, b, &fl):fcntl(a, b, &fl);
        UnalignFlock64((void*)d1, &fl);
        return ret;
    }
    //TODO: check if better to use the syscall or regular fcntl?
    //return syscall(__NR_fcntl64, a, b, d1);   // should be enough
    int ret = f?f(a, b, d1):fcntl(a, b, d1);

    if(b==F_GETFL && ret!=-1)
        ret = of_unconvert32(ret);

    return ret;
}

EXPORT int32_t my32_fcntl(x64emu_t* emu, int32_t a, int32_t b, uint32_t d1, uint32_t d2, uint32_t d3, uint32_t d4, uint32_t d5, uint32_t d6)
{
    if(b==F_SETFL && d1==0xFFFFF7FF) {
        // special case for ~O_NONBLOCK...
        int flags = fcntl(a, F_GETFL);
        if(flags&X86_O_NONBLOCK) {
            flags &= ~O_NONBLOCK;
            return fcntl(a, b, flags);
        }
        return 0;
    }
    if(b==F_SETFL)
        d1 = of_convert32(d1);
    if(b==F_GETLK64 || b==F_SETLK64 || b==F_SETLKW64)
    {
        my32_flock64_t fl;
        AlignFlock64(&fl, (void*)d1);
        int ret = fcntl(a, b, &fl);
        UnalignFlock64((void*)d1, &fl);
        return ret;
    }
    int ret = fcntl(a, b, d1);
    if(b==F_GETFL && ret!=-1)
        ret = of_unconvert32(ret);
    
    return ret;    
}
EXPORT int32_t my32___fcntl(x64emu_t* emu, int32_t a, int32_t b, uint32_t d1, uint32_t d2, uint32_t d3, uint32_t d4, uint32_t d5, uint32_t d6) __attribute__((alias("my32_fcntl")));

EXPORT int32_t my32_preadv64(x64emu_t* emu, int32_t fd, void* v, int32_t c, int64_t o)
{
    library_t* lib = my_lib;
    if(!lib) return 0;
    void* f = dlsym(lib->priv.w.lib, "preadv64");
    if(f)
        return ((iFipiI_t)f)(fd, v, c, o);
    return syscall(__NR_preadv, fd, v, c,(uint32_t)(o&0xffffffff), (uint32_t)((o>>32)&0xffffffff));
}

EXPORT int32_t my32_pwritev64(x64emu_t* emu, int32_t fd, void* v, int32_t c, int64_t o)
{
    library_t* lib = my_lib;
    if(!lib) return 0;
    void* f = dlsym(lib->priv.w.lib, "pwritev64");
    if(f)
        return ((iFipiI_t)f)(fd, v, c, o);
    #ifdef __arm__
    return syscall(__NR_pwritev, fd, v, c, 0, (uint32_t)(o&0xffffffff), (uint32_t)((o>>32)&0xffffffff));
    // on arm, 64bits args needs to be on even/odd register, so need to put a 0 for aligment
    #else
    return syscall(__NR_pwritev, fd, v, c,(uint32_t)(o&0xffffffff), (uint32_t)((o>>32)&0xffffffff));
    #endif
}

EXPORT int32_t my32_accept4(x64emu_t* emu, int32_t fd, void* a, void* l, int32_t flags)
{
    library_t* lib = my_lib;
    if(!lib) return 0;
    void* f = dlsym(lib->priv.w.lib, "accept4");
    if(f)
        return ((iFippi_t)f)(fd, a, l, flags);
    if(!flags)
        return accept(fd, a, l);
    return syscall(__NR_accept4, fd, a, l, flags);
}

EXPORT  int32_t my32_fallocate64(int fd, int mode, int64_t offs, int64_t len)
{
    iFiiII_t f = NULL;
    static int done = 0;
    if(!done) {
        library_t* lib = my_lib;
        f = (iFiiII_t)dlsym(lib->priv.w.lib, "fallocate64");
        done = 1;
    }
    if(f)
        return f(fd, mode, offs, len);
    else
        return syscall(__NR_fallocate, fd, mode, (uint32_t)(offs&0xffffffff), (uint32_t)((offs>>32)&0xffffffff), (uint32_t)(len&0xffffffff), (uint32_t)((len>>32)&0xffffffff));
        //return posix_fallocate64(fd, offs, len);
}

EXPORT int my32_getopt(int argc, char* const argv[], const char *optstring)
{
    int ret = getopt(argc, argv, optstring);
    my32_checkGlobalOpt();
    return ret;
}

EXPORT int my32_getopt_long(int argc, char* const argv[], const char* optstring, const struct option *longopts, int *longindex)
{
    int ret = getopt_long(argc, argv, optstring, longopts, longindex);
    my32_checkGlobalOpt();
    return ret;
}

EXPORT int my32_getopt_long_only(int argc, char* const argv[], const char* optstring, const struct option *longopts, int *longindex)
{
    int ret = getopt_long_only(argc, argv, optstring, longopts, longindex);
    my32_checkGlobalOpt();
    return ret;
}
#endif

EXPORT struct __processor_model
{
  unsigned int __cpu_vendor;
  unsigned int __cpu_type;
  unsigned int __cpu_subtype;
  unsigned int __cpu_features[1];
} my32___cpu_model;

#include "cpu_info.h"
void InitCpuModel()
{
    // some pseudo random cpu info...
    my32___cpu_model.__cpu_vendor = VENDOR_INTEL;
    my32___cpu_model.__cpu_type = INTEL_PENTIUM_M;
    my32___cpu_model.__cpu_subtype = 0; // N/A
    my32___cpu_model.__cpu_features[0] = (1<<FEATURE_CMOV) 
                                     | (1<<FEATURE_MMX) 
                                     | (1<<FEATURE_SSE) 
                                     | (1<<FEATURE_SSE2) 
                                     | (1<<FEATURE_SSE3)
                                     | (1<<FEATURE_SSSE3)
                                     | (1<<FEATURE_MOVBE)
                                     | (1<<FEATURE_ADX);
}

EXPORT const unsigned short int *my32___ctype_b;
EXPORT const int32_t *my32___ctype_tolower;
EXPORT const int32_t *my32___ctype_toupper;

static void ctSetup()
{
    my32___ctype_b = *(__ctype_b_loc());
    my32___ctype_toupper = *(__ctype_toupper_loc());
    my32___ctype_tolower = *(__ctype_tolower_loc());
}

#if 0
EXPORT void my32___register_frame_info(void* a, void* b)
{
    // nothing
}
EXPORT void* my32___deregister_frame_info(void* a)
{
    return NULL;
}

EXPORT void* my32____brk_addr = NULL;
#endif
// longjmp / setjmp
typedef struct jump_buff_i386_s {
 uint32_t save_ebx;
 uint32_t save_esi;
 uint32_t save_edi;
 uint32_t save_ebp;
 uint32_t save_esp;
 uint32_t save_eip;
} jump_buff_i386_t;

typedef struct __jmp_buf_tag_s {
    jump_buff_i386_t __jmpbuf;
    int              __mask_was_saved;
    sigset_t         __saved_mask;
} __jmp_buf_tag_t;

void EXPORT my32_longjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val)
{
    jump_buff_i386_t *jpbuff = &((__jmp_buf_tag_t*)p)->__jmpbuf;
    //restore  regs
    R_EBX = jpbuff->save_ebx;
    R_ESI = jpbuff->save_esi;
    R_EDI = jpbuff->save_edi;
    R_EBP = jpbuff->save_ebp;
    R_ESP = jpbuff->save_esp;
    // jmp to saved location, plus restore val to eax
    R_EAX = __val;
    R_EIP = jpbuff->save_eip;
    if(((__jmp_buf_tag_t*)p)->__mask_was_saved) {
        sigprocmask(SIG_SETMASK, &((__jmp_buf_tag_t*)p)->__saved_mask, NULL);
    }
    if(emu->flags.quitonlongjmp) {
        emu->flags.longjmp = 1;
        emu->quit = 1;
    }
}

EXPORT int32_t my32___sigsetjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int savesigs)
{
    jump_buff_i386_t *jpbuff = &((__jmp_buf_tag_t*)p)->__jmpbuf;
    // save the buffer
    jpbuff->save_ebx = R_EBX;
    jpbuff->save_esi = R_ESI;
    jpbuff->save_edi = R_EDI;
    jpbuff->save_ebp = R_EBP;
    jpbuff->save_esp = R_ESP+4; // include "return address"
    jpbuff->save_eip = *(uint32_t*)from_ptr(R_ESP);
    if(savesigs) {
        if(sigprocmask(SIG_SETMASK, NULL, &((__jmp_buf_tag_t*)p)->__saved_mask))
            ((__jmp_buf_tag_t*)p)->__mask_was_saved = 0;
        else
            ((__jmp_buf_tag_t*)p)->__mask_was_saved = 1;
    } else
        ((__jmp_buf_tag_t*)p)->__mask_was_saved = 0;
    return 0;
}

EXPORT int32_t my32__setjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p)
{
    return  my32___sigsetjmp(emu, p, 0);
}
EXPORT int32_t my32_setjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p)
{
    return  my32___sigsetjmp(emu, p, 1);
}
#if 0
EXPORT void my32___explicit_bzero_chk(x64emu_t* emu, void* dst, uint32_t len, uint32_t dstlen)
{
    memset(dst, 0, len);
}

EXPORT void* my32_realpath(x64emu_t* emu, void* path, void* resolved_path)
{

    if(isProcSelf(path, "exe")) {
        return realpath(emu->context->fullpath, resolved_path);
    }
        return realpath(path, resolved_path);
}

EXPORT int my32_readlinkat(x64emu_t* emu, int fd, void* path, void* buf, size_t bufsize)
{
    if(isProcSelf(path, "exe")) {
        strncpy(buf, emu->context->fullpath, bufsize);
        size_t l = strlen(emu->context->fullpath);
        return (l>bufsize)?bufsize:(l+1);
    }
    return readlinkat(fd, path, buf, bufsize);
}

#endif
EXPORT void* my32_mmap(x64emu_t* emu, void *addr, size_t length, int prot, int flags, int fd, int offset)
{
    if(prot&PROT_WRITE) 
        prot|=PROT_READ;    // PROT_READ is implicit with PROT_WRITE on i386
    #ifdef __x86_64__
    flags |= MAP_32BIT;
    #endif
    if(box64_log<LOG_DEBUG) {dynarec_log(LOG_DEBUG, "mmap(%p, %zu, 0x%x, 0x%x, %d, %d) =>", addr, length, prot, flags, fd, offset);}
    void* new_addr = addr?addr:find32bitBlock(length);
    void* ret = mmap(new_addr, length, prot, flags, fd, offset);
    if(!addr && ret!=new_addr && ret!=(void*)-1) {
        munmap(ret, length);
        loadProtectionFromMap();    // reload map, because something went wrong previously
        new_addr = find31bitBlockNearHint(addr, length, 0); // is this the best way?
        ret = mmap(new_addr, length, prot, flags, fd, offset);
    }
    if(box64_log<LOG_DEBUG) {dynarec_log(LOG_DEBUG, "%p\n", ret);}
    #ifdef DYNAREC
    if(box64_dynarec && ret!=(void*)-1) {
        if(flags&0x100000 && addr!=ret)
        {
            // program used MAP_FIXED_NOREPLACE but the host linux didn't support it
            // and responded with a different address, so ignore it
        } else {
            if(prot& PROT_EXEC)
                addDBFromAddressRange((uintptr_t)ret, length);
            else
                cleanDBFromAddressRange((uintptr_t)ret, length, prot?0:1);
        }
    } 
    #endif
    if(ret!=(void*)-1)
        setProtection((uintptr_t)ret, length, prot);
    return ret;
}

EXPORT void* my32_mmap64(x64emu_t* emu, void *addr, size_t length, int prot, int flags, int fd, int64_t offset)
{
    if(prot&PROT_WRITE) 
        prot|=PROT_READ;    // PROT_READ is implicit with PROT_WRITE on i386
    if(box64_log<LOG_DEBUG) {dynarec_log(LOG_DEBUG, "mmap64(%p, %zu, 0x%x, 0x%x, %d, %ld) =>", addr, length, prot, flags, fd, offset);}
    void* new_addr = (flags&MAP_FIXED)?addr:find31bitBlockNearHint(addr, length, 0);
    void* ret = mmap64(new_addr, length, prot, flags, fd, offset);
    if(!addr && ret!=new_addr && ret!=(void*)-1) {
        munmap(ret, length);
        loadProtectionFromMap();    // reload map, because something went wrong previously
        new_addr = find31bitBlockNearHint(addr, length, 0);
        ret = mmap64(new_addr, length, prot, flags, fd, offset);
    } else if(addr && ret!=(void*)-1 && ret!=new_addr && 
      ((uintptr_t)ret&0xffff) && !(flags&MAP_FIXED) && box64_wine) {
        munmap(ret, length);
        loadProtectionFromMap();    // reload map, because something went wrong previously
        new_addr = find31bitBlockNearHint(addr, length, 0);
        ret = mmap64(new_addr, length, prot, flags, fd, offset);
        if(ret!=(void*)-1 && ret!=addr && ((uintptr_t)ret&0xffff) && box64_wine) {
            // addr is probably too high, start again with a low address
            munmap(ret, length);
            loadProtectionFromMap();    // reload map, because something went wrong previously
            new_addr = find31bitBlockNearHint(NULL, length, 0); // is this the best way?
            ret = mmap64(new_addr, length, prot, flags, fd, offset);
            if(ret!=(void*)-1 && (uintptr_t)ret&0xffff) {
                munmap(ret, length);
                ret = (void*)-1;
            }
        }
    }
    if(box64_log<LOG_DEBUG) {dynarec_log(LOG_DEBUG, "%p\n", ret);}
    #ifdef DYNAREC
    if(box64_dynarec && ret!=(void*)-1) {
        if(flags&0x100000 && addr!=ret)
        {
            // program used MAP_FIXED_NOREPLACE but the host linux didn't support it
            // and responded with a different address, so ignore it
        } else {
            if(prot& PROT_EXEC)
                addDBFromAddressRange((uintptr_t)ret, length);
            else
                cleanDBFromAddressRange((uintptr_t)ret, length, prot?0:1);
        }
    }
    #endif
    if(ret!=(void*)-1)
        setProtection((uintptr_t)ret, length, prot);
    return ret;
}

EXPORT void* my32_mremap(x64emu_t* emu, void* old_addr, size_t old_size, size_t new_size, int flags, void* new_addr)
{
    dynarec_log(/*LOG_DEBUG*/LOG_NONE, "mremap(%p, %zu, %zu, %d, %p)=>", old_addr, old_size, new_size, flags, new_addr);
    void* ret = mremap(old_addr, old_size, new_size, flags, new_addr);
    dynarec_log(/*LOG_DEBUG*/LOG_NONE, "%p\n", ret);
    if(ret==(void*)-1)
        return ret; // failed...
    uint32_t prot = getProtection((uintptr_t)old_addr)&~PROT_CUSTOM;
    if(ret==old_addr) {
        if(old_size && old_size<new_size) {
            setProtection((uintptr_t)ret+old_size, new_size-old_size, prot);
            #ifdef DYNAREC
            if(box64_dynarec)
                addDBFromAddressRange((uintptr_t)ret+old_size, new_size-old_size);
            #endif
        } else if(old_size && new_size<old_size) {
            freeProtection((uintptr_t)ret+new_size, old_size-new_size);
            #ifdef DYNAREC
            if(box64_dynarec)
                cleanDBFromAddressRange((uintptr_t)ret+new_size, new_size-old_size, 1);
            #endif
        } else if(!old_size) {
            setProtection((uintptr_t)ret, new_size, prot);
            #ifdef DYNAREC
            if(box64_dynarec)
                addDBFromAddressRange((uintptr_t)ret, new_size);
            #endif
        }
    } else {
        if(old_size
        #ifdef MREMAP_DONTUNMAP
        && !(flags&MREMAP_DONTUNMAP)
        #endif
        ) {
            freeProtection((uintptr_t)old_addr, old_size);
            #ifdef DYNAREC
            if(box64_dynarec)
                cleanDBFromAddressRange((uintptr_t)old_addr, old_size, 1);
            #endif
        }
        setProtection((uintptr_t)ret, new_size, prot); // should copy the protection from old block
        #ifdef DYNAREC
        if(box64_dynarec)
            addDBFromAddressRange((uintptr_t)ret, new_size);
        #endif
    }
    return ret;
}

EXPORT int my32_munmap(x64emu_t* emu, void* addr, unsigned long length)
{
    dynarec_log(LOG_DEBUG, "munmap(%p, %lu)\n", addr, length);
    #ifdef DYNAREC
    if(box64_dynarec) {
        cleanDBFromAddressRange((uintptr_t)addr, length, 1);
    }
    #endif
    int ret = munmap(addr, length);
    if(!ret)
        freeProtection((uintptr_t)addr, length);
    return ret;
}

EXPORT int my32_mprotect(x64emu_t* emu, void *addr, unsigned long len, int prot)
{
    dynarec_log(LOG_DEBUG, "mprotect(%p, %lu, 0x%x)\n", addr, len, prot);
    if(prot&PROT_WRITE) 
        prot|=PROT_READ;    // PROT_READ is implicit with PROT_WRITE on i386
    int ret = mprotect(addr, len, prot);
    #ifdef DYNAREC
    if(box64_dynarec) {
        if(prot& PROT_EXEC)
            addDBFromAddressRange((uintptr_t)addr, len);
        else
            cleanDBFromAddressRange((uintptr_t)addr, len, 0);
    }
    #endif
    if(!ret)
        updateProtection((uintptr_t)addr, len, prot);
    return ret;
}
#if 0
#ifndef ANDROID
typedef struct my32_cookie_s {
    uintptr_t r, w, s, c;
    void* cookie;
} my32_cookie_t;

static ssize_t my32_cookie_read(void *p, char *buf, size_t size)
{
    my32_cookie_t* cookie = (my32_cookie_t*)p;
    return (ssize_t)RunFunction(my_context, cookie->r, 3, cookie->cookie, buf, size);
}
static ssize_t my32_cookie_write(void *p, const char *buf, size_t size)
{
    my32_cookie_t* cookie = (my32_cookie_t*)p;
    return (ssize_t)RunFunction(my_context, cookie->w, 3, cookie->cookie, buf, size);
}
static int my32_cookie_seek(void *p, off64_t *offset, int whence)
{
    my32_cookie_t* cookie = (my32_cookie_t*)p;
    return RunFunction(my_context, cookie->s, 3, cookie->cookie, offset, whence);
}
static int my32_cookie_close(void *p)
{
    my32_cookie_t* cookie = (my32_cookie_t*)p;
    int ret = 0;
    if(cookie->c)
        ret = RunFunction(my_context, cookie->c, 1, cookie->cookie);
    free(cookie);
    return ret;
}
EXPORT void* my32_fopencookie(x64emu_t* emu, void* cookie, void* mode, void* read, void* write, void* seek, void* close)
{
    cookie_io_functions_t io_funcs = {read?my32_cookie_read:NULL, write?my32_cookie_write:NULL, seek?my32_cookie_seek:NULL, my32_cookie_close};
    my32_cookie_t *cb = (my32_cookie_t*)calloc(1, sizeof(my32_cookie_t));
    cb->r = (uintptr_t)read;
    cb->w = (uintptr_t)write;
    cb->s = (uintptr_t)seek;
    cb->c = (uintptr_t)close;
    cb->cookie = cookie;
    return fopencookie(cb, mode, io_funcs);
}
#endif

EXPORT long my32_prlimit64(void* pid, uint32_t res, void* new_rlim, void* old_rlim)
{
    return syscall(__NR_prlimit64, pid, res, new_rlim, old_rlim);
}

EXPORT void* my32_reallocarray(void* ptr, size_t nmemb, size_t size)
{
    return realloc(ptr, nmemb*size);
}

#ifndef __OPEN_NEEDS_MODE
# define __OPEN_NEEDS_MODE(oflag) \
  (((oflag) & O_CREAT) != 0)
// || ((oflag) & __O_TMPFILE) == __O_TMPFILE)
#endif
EXPORT int my32___open_nocancel(x64emu_t* emu, void* file, int oflag, int* b)
{
    int mode = 0;
    if (__OPEN_NEEDS_MODE (oflag))
        mode = b[0];
    return openat(AT_FDCWD, file, oflag, mode);
}

EXPORT int my32___libc_alloca_cutoff(x64emu_t* emu, size_t size)
{
    // not always implemented on old linux version...
    library_t* lib = my_lib;
    if(!lib) return 0;
    void* f = dlsym(lib->priv.w.lib, "__libc_alloca_cutoff");
    if(f)
        return ((iFL_t)f)(size);
    // approximate version but it's better than nothing....
    return (size<=(65536*4));
}

// DL functions from wrappedlibdl.c
void* my32_dlopen(x64emu_t* emu, void *filename, int flag);
int my32_dlclose(x64emu_t* emu, void *handle);
void* my32_dlsym(x64emu_t* emu, void *handle, void *symbol);
EXPORT int my32___libc_dlclose(x64emu_t* emu, void* handle)
{
    return my32_dlclose(emu, handle);
}
EXPORT void* my32___libc_dlopen_mode(x64emu_t* emu, void* name, int mode)
{
    return my32_dlopen(emu, name, mode);
}
EXPORT void* my32___libc_dlsym(x64emu_t* emu, void* handle, void* name)
{
    return my32_dlsym(emu, handle, name);
}
#endif
// all obstack function defined in obstack.c file
void obstackSetup();
#if 0
EXPORT int my32_nanosleep(const struct timespec *req, struct timespec *rem)
{
    if(!req)
        return 0;   // workaround for some strange calls
    return nanosleep(req, rem);
}
#endif
// wrapped malloc using calloc, it seems x86 malloc set alloc'd block to zero somehow
EXPORT void* my32_malloc(unsigned long size)
{
    return calloc(1, size);
}
#if 0
#ifndef __NR_memfd_create
#define MFD_CLOEXEC		    0x0001U
#define MFD_ALLOW_SEALING	0x0002U
EXPORT int my32_memfd_create(x64emu_t* emu, void* name, uint32_t flags)
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
#define GRND_RANDOM	0x0002
#endif
EXPORT int my32_getentropy(x64emu_t* emu, void* buffer, size_t length)
{
    library_t* lib = my_lib;
    if(!lib) return 0;
    void* f = dlsym(lib->priv.w.lib, "getentropy");
    if(f)
        return ((iFpL_t)f)(buffer, length);
    // custom implementation
    if(length>256) {
        errno = EIO;
        return -1;
    }
    int ret = my32_getrandom(emu, buffer, length, GRND_RANDOM);
    if(ret!=length) {
        errno = EIO;
        return -1;
    }
    return 0;
}

EXPORT void my32_mcount(void* frompc, void* selfpc)
{
    // stub doing nothing...
    return;
}

#ifndef ANDROID
union semun {
  int              val;    /* Value for SETVAL */
  struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
  unsigned short  *array;  /* Array for GETALL, SETALL */
  struct seminfo  *__buf;  /* Buffer for IPC_INFO
                              (Linux-specific) */
};
#endif

EXPORT int my32_semctl(x64emu_t* emu, int semid, int semnum, int cmd, union semun b)
{
  iFiiiV_t f = semctl;
  return  ((iFiiiV_t)f)(semid, semnum, cmd, b);
}

#ifndef ANDROID
EXPORT int my32_on_exit(x64emu_t* emu, void* f, void* args)
{
    return on_exit(findon_exitFct(f), args);
}
#endif
#endif

EXPORT long my32_strtol(void* nptr, ptr_t* endptr, int base)
{
    void* endp;
    long ret = strtol(nptr, (char**)(endptr?(&endp):NULL), base);
    if(endptr)
        *endptr = to_ptrv(endp);
    return ret;
}

EXPORT char** my32_environ = NULL;
EXPORT char** my32__environ = NULL;
EXPORT char** my32___environ = NULL;  // all aliases

EXPORT char* my32___progname = NULL;
EXPORT char* my32___progname_full = NULL;
EXPORT char* my32_program_invocation_name = NULL;
EXPORT char* my32_program_invocation_short_name = NULL;

EXPORT ptr_t my32_stdin = 0;
EXPORT ptr_t my32_stdout = 0;
EXPORT ptr_t my32_stderr = 0;

EXPORT int my32___libc_single_threaded = 0;

#define PRE_INIT\
    if(1)                                                           \
        my_lib = lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);\
    else

#ifdef ANDROID
#define NEEDED_LIBS   0
#else
//#define NEEDED_LIBS   4,\
//    "ld-linux.so.2",    \
//    "libpthread.so.0",  \
//    "libdl.so.2",       \
//    "librt.so.1"
#define NEEDED_LIBS         \
    "ld-linux.so.2"
#endif

#define CUSTOM_INIT         \
    box64->libclib = lib;   \
    my_lib = lib;           \
    InitCpuModel();         \
    ctSetup();              \
    /*obstackSetup();*/     \
    my32_environ = my32__environ = my32___environ = box64->envv;            \
    my32___progname_full = my32_program_invocation_name = box64->argv[0];   \
    my32___progname = my32_program_invocation_short_name =                  \
        strrchr(box64->argv[0], '/');                                       \
    my32_stdin = to_hashv(stdin);  \
    my32_stdout = to_hashv(stdout);\
    my32_stderr = to_hashv(stderr);

#include "wrappedlib_init32.h"
