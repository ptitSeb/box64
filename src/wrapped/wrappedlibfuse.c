#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <sys/stat.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "callback.h"
#include "myalign.h"
#include "debug.h"

static library_t *my_lib = NULL;
const char* libfuseName = "libfuse.so.2";
#define LIBNAME libfuse

#include "generated/wrappedlibfusetypes.h"

typedef struct fuse_lowlevel_ops_s {
    void (*init) (void *userdata, void *conn);
    void (*destroy) (void *userdata);
    void (*lookup) (void* req, unsigned long parent, const char *name);
    void (*forget) (void* req, unsigned long ino, unsigned long nlookup);
    void (*getattr) (void* req, unsigned long ino, void *fi);
    void (*setattr) (void* req, unsigned long ino, struct stat *attr, int to_set, void *fi);
    void (*readlink) (void* req, unsigned long ino);
    void (*mknod) (void* req, unsigned long parent, const char *name, mode_t mode, dev_t rdev);
    void (*mkdir) (void* req, unsigned long parent, const char *name, mode_t mode);
    void (*unlink) (void* req, unsigned long parent, const char *name);
    void (*rmdir) (void* req, unsigned long parent, const char *name);
    void (*symlink) (void* req, const char *link, unsigned long parent, const char *name);
    void (*rename) (void* req, unsigned long parent, const char *name, unsigned long newparent, const char *newname);
    void (*link) (void* req, unsigned long ino, unsigned long newparent, const char *newname);
    void (*open) (void* req, unsigned long ino, void *fi);
    void (*read) (void* req, unsigned long ino, size_t size, off_t off, void *fi);
    void (*write) (void* req, unsigned long ino, const char *buf, size_t size, off_t off, void *fi);
    void (*flush) (void* req, unsigned long ino, void *fi);
    void (*release) (void* req, unsigned long ino, void *fi);
    void (*fsync) (void* req, unsigned long ino, int datasync, void *fi);
    void (*opendir) (void* req, unsigned long ino, void *fi);
    void (*readdir) (void* req, unsigned long ino, size_t size, off_t off, void *fi);
    void (*releasedir) (void* req, unsigned long ino, void *fi);
    void (*fsyncdir) (void* req, unsigned long ino, int datasync, void *fi);
    void (*statfs) (void* req, unsigned long ino);
    void (*setxattr) (void* req, unsigned long ino, const char *name, const char *value, size_t size, int flags);
    void (*getxattr) (void* req, unsigned long ino, const char *name, size_t size);
    void (*listxattr) (void* req, unsigned long ino, size_t size);
    void (*removexattr) (void* req, unsigned long ino, const char *name);
    void (*access) (void* req, unsigned long ino, int mask);
    void (*create) (void* req, unsigned long parent, const char *name, mode_t mode, void *fi);
    void (*getlk) (void* req, unsigned long ino, void *fi, void *lock);
    void (*setlk) (void* req, unsigned long ino, void *fi, void *lock, int sleep);
    void (*bmap) (void* req, unsigned long ino, size_t blocksize, uint64_t idx);
} fuse_lowlevel_ops_t;

typedef struct libfuse_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} libfuse_my_t;

void* getFuseMy(library_t* lib)
{
    my_lib = lib;
    libfuse_my_t* my = (libfuse_my_t*)calloc(1, sizeof(libfuse_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}

void freeFuseMy(void* lib)
{
    (void)lib;
    //libfuse_my_t *my = (libfuse_my_t *)lib;
}
#undef SUPER

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// fuse_opt_proc
#define GO(A)   \
static uintptr_t my_fuse_opt_proc_fct_##A = 0;                                      \
static int my_fuse_opt_proc_##A(void* a, void* b, int c, void* d)                   \
{                                                                                   \
    return (int)RunFunction(my_context, my_fuse_opt_proc_fct_##A, 4, a, b, c, d);   \
}
SUPER()
#undef GO
static void* findfuse_opt_procFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_fuse_opt_proc_fct_##A == (uintptr_t)fct) return my_fuse_opt_proc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_fuse_opt_proc_fct_##A == 0) {my_fuse_opt_proc_fct_##A = (uintptr_t)fct; return my_fuse_opt_proc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse fuse_opt_proc callback\n");
    return NULL;
}

// init
#define GO(A)   \
static uintptr_t my_init_fct_##A = 0;                 \
static void my_init_##A(void* a, void* b)             \
{                                                     \
    RunFunction(my_context, my_init_fct_##A, 2, a, b);\
}
SUPER()
#undef GO
static void* find_init_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_init_fct_##A == (uintptr_t)fct) return my_init_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_init_fct_##A == 0) {my_init_fct_##A = (uintptr_t)fct; return my_init_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse init callback\n");
    return NULL;
}

// destroy
#define GO(A)   \
static uintptr_t my_destroy_fct_##A = 0;                \
static void my_destroy_##A(void* a)                     \
{                                                       \
    RunFunction(my_context, my_destroy_fct_##A, 1, a);  \
}
SUPER()
#undef GO
static void* find_destroy_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_destroy_fct_##A == (uintptr_t)fct) return my_destroy_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_destroy_fct_##A == 0) {my_destroy_fct_##A = (uintptr_t)fct; return my_destroy_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse destroy callback\n");
    return NULL;
}

// lookup
#define GO(A)   \
static uintptr_t my_lookup_fct_##A = 0;                     \
static void my_lookup_##A(void* a, unsigned long b, void* c)\
{                                                           \
    RunFunction(my_context, my_lookup_fct_##A, 3, a, b, c); \
}
SUPER()
#undef GO
static void* find_lookup_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_lookup_fct_##A == (uintptr_t)fct) return my_lookup_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_lookup_fct_##A == 0) {my_lookup_fct_##A = (uintptr_t)fct; return my_lookup_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse lookup callback\n");
    return NULL;
}

// forget
#define GO(A)   \
static uintptr_t my_forget_fct_##A = 0;                     \
static void my_forget_##A(void* a, unsigned long b, unsigned long c)\
{                                                           \
    RunFunction(my_context, my_forget_fct_##A, 3, a, b, c); \
}
SUPER()
#undef GO
static void* find_forget_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_forget_fct_##A == (uintptr_t)fct) return my_forget_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_forget_fct_##A == 0) {my_forget_fct_##A = (uintptr_t)fct; return my_forget_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse forget callback\n");
    return NULL;
}

// getattr
#define GO(A)   \
static uintptr_t my_getattr_fct_##A = 0;                        \
static void my_getattr_##A(void* a, unsigned long b, void* c)   \
{                                                               \
    RunFunction(my_context, my_getattr_fct_##A, 3, a, b, c);    \
}
SUPER()
#undef GO
static void* find_getattr_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_getattr_fct_##A == (uintptr_t)fct) return my_getattr_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_getattr_fct_##A == 0) {my_getattr_fct_##A = (uintptr_t)fct; return my_getattr_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse getattr callback\n");
    return NULL;
}

// setattr
#define GO(A)   \
static uintptr_t my_setattr_fct_##A = 0;                        \
static void my_setattr_##A(void* a, unsigned long b, struct stat* c, int d, void* e)   \
{                                                               \
    struct stat c_;                                             \
    AlignStat64(c, &c_);                                        \
    RunFunction(my_context, my_setattr_fct_##A, 5, a, b, &c_, d, e);    \
}
SUPER()
#undef GO
static void* find_setattr_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_setattr_fct_##A == (uintptr_t)fct) return my_setattr_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_setattr_fct_##A == 0) {my_setattr_fct_##A = (uintptr_t)fct; return my_setattr_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse setattr callback\n");
    return NULL;
}

// readlink
#define GO(A)   \
static uintptr_t my_readlink_fct_##A = 0;                   \
static void my_readlink_##A(void* a, unsigned long b)       \
{                                                           \
    RunFunction(my_context, my_readlink_fct_##A, 2, a, b);  \
}
SUPER()
#undef GO
static void* find_readlink_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_readlink_fct_##A == (uintptr_t)fct) return my_readlink_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_readlink_fct_##A == 0) {my_readlink_fct_##A = (uintptr_t)fct; return my_readlink_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse readlink callback\n");
    return NULL;
}

// mknod
#define GO(A)   \
static uintptr_t my_mknod_fct_##A = 0;                                                  \
static void my_mknod_##A(void* a, unsigned long b, const char* c, mode_t d, dev_t e)    \
{                                                                                       \
    RunFunction(my_context, my_mknod_fct_##A, 5, a, b, c, of_convert(d), e);            \
}
SUPER()
#undef GO
static void* find_mknod_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_mknod_fct_##A == (uintptr_t)fct) return my_mknod_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_mknod_fct_##A == 0) {my_mknod_fct_##A = (uintptr_t)fct; return my_mknod_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse mknod callback\n");
    return NULL;
}

// mkdir
#define GO(A)   \
static uintptr_t my_mkdir_fct_##A = 0;                                      \
static void my_mkdir_##A(void* a, unsigned long b, const char* c, mode_t d) \
{                                                                           \
    RunFunction(my_context, my_mkdir_fct_##A, 4, a, b, c, of_convert(d));   \
}
SUPER()
#undef GO
static void* find_mkdir_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_mkdir_fct_##A == (uintptr_t)fct) return my_mkdir_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_mkdir_fct_##A == 0) {my_mkdir_fct_##A = (uintptr_t)fct; return my_mkdir_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse mkdir callback\n");
    return NULL;
}

// unlink
#define GO(A)   \
static uintptr_t my_unlink_fct_##A = 0;                             \
static void my_unlink_##A(void* a, unsigned long b, const char* c)  \
{                                                                   \
    RunFunction(my_context, my_unlink_fct_##A, 3, a, b, c);         \
}
SUPER()
#undef GO
static void* find_unlink_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_unlink_fct_##A == (uintptr_t)fct) return my_unlink_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_unlink_fct_##A == 0) {my_unlink_fct_##A = (uintptr_t)fct; return my_unlink_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse unlink callback\n");
    return NULL;
}

// rmdir
#define GO(A)   \
static uintptr_t my_rmdir_fct_##A = 0;                             \
static void my_rmdir_##A(void* a, unsigned long b, const char* c)  \
{                                                                   \
    RunFunction(my_context, my_rmdir_fct_##A, 3, a, b, c);         \
}
SUPER()
#undef GO
static void* find_rmdir_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_rmdir_fct_##A == (uintptr_t)fct) return my_rmdir_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_rmdir_fct_##A == 0) {my_rmdir_fct_##A = (uintptr_t)fct; return my_rmdir_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse rmdir callback\n");
    return NULL;
}

// symlink
#define GO(A)   \
static uintptr_t my_symlink_fct_##A = 0;                                            \
static void my_symlink_##A(void* a, const char* b, unsigned long c, const char* d)  \
{                                                                                   \
    RunFunction(my_context, my_symlink_fct_##A, 4, a, b, c, d);                     \
}
SUPER()
#undef GO
static void* find_symlink_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_symlink_fct_##A == (uintptr_t)fct) return my_symlink_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_symlink_fct_##A == 0) {my_symlink_fct_##A = (uintptr_t)fct; return my_symlink_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse symlink callback\n");
    return NULL;
}

// rename
#define GO(A)   \
static uintptr_t my_rename_fct_##A = 0;                                                             \
static void my_rename_##A(void* a, unsigned long b, const char* c, unsigned long d, const char* e)  \
{                                                                                                   \
    RunFunction(my_context, my_rename_fct_##A, 5, a, b, c, d, e);                                   \
}
SUPER()
#undef GO
static void* find_rename_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_rename_fct_##A == (uintptr_t)fct) return my_rename_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_rename_fct_##A == 0) {my_rename_fct_##A = (uintptr_t)fct; return my_rename_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse rename callback\n");
    return NULL;
}

// link
#define GO(A)   \
static uintptr_t my_link_fct_##A = 0;                                               \
static void my_link_##A(void* a, unsigned long b, unsigned long c, const char* d)   \
{                                                                                   \
    RunFunction(my_context, my_link_fct_##A, 4, a, b, c, d);                        \
}
SUPER()
#undef GO
static void* find_link_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_link_fct_##A == (uintptr_t)fct) return my_link_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_link_fct_##A == 0) {my_link_fct_##A = (uintptr_t)fct; return my_link_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse link callback\n");
    return NULL;
}

// open
#define GO(A)   \
static uintptr_t my_open_fct_##A = 0;                           \
static void my_open_##A(void* a, unsigned long b, const char* c)\
{                                                               \
    RunFunction(my_context, my_open_fct_##A, 3, a, b, c);       \
}
SUPER()
#undef GO
static void* find_open_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_open_fct_##A == (uintptr_t)fct) return my_open_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_open_fct_##A == 0) {my_open_fct_##A = (uintptr_t)fct; return my_open_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse open callback\n");
    return NULL;
}

// read
#define GO(A)   \
static uintptr_t my_read_fct_##A = 0;                                                           \
static void my_read_##A(void* a, unsigned long b, const char* c, size_t d, off_t e, void* f)    \
{                                                                                               \
    RunFunction(my_context, my_read_fct_##A, 6, a, b, c, d, e, f);                              \
}
SUPER()
#undef GO
static void* find_read_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_read_fct_##A == (uintptr_t)fct) return my_read_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_read_fct_##A == 0) {my_read_fct_##A = (uintptr_t)fct; return my_read_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse read callback\n");
    return NULL;
}

// write
#define GO(A)   \
static uintptr_t my_write_fct_##A = 0;                                                          \
static void my_write_##A(void* a, unsigned long b, const char* c, size_t d, off_t e, void* f)   \
{                                                                                               \
    RunFunction(my_context, my_write_fct_##A, 6, a, b, c, d, e, f);                             \
}
SUPER()
#undef GO
static void* find_write_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_write_fct_##A == (uintptr_t)fct) return my_write_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_write_fct_##A == 0) {my_write_fct_##A = (uintptr_t)fct; return my_write_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse write callback\n");
    return NULL;
}

// flush
#define GO(A)   \
static uintptr_t my_flush_fct_##A = 0;                              \
static void my_flush_##A(void* a, unsigned long b, const char* c)   \
{                                                                   \
    RunFunction(my_context, my_flush_fct_##A, 3, a, b, c);          \
}
SUPER()
#undef GO
static void* find_flush_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_flush_fct_##A == (uintptr_t)fct) return my_flush_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_flush_fct_##A == 0) {my_flush_fct_##A = (uintptr_t)fct; return my_flush_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse flush callback\n");
    return NULL;
}

// release
#define GO(A)   \
static uintptr_t my_release_fct_##A = 0;                              \
static void my_release_##A(void* a, unsigned long b, const char* c)   \
{                                                                   \
    RunFunction(my_context, my_release_fct_##A, 3, a, b, c);          \
}
SUPER()
#undef GO
static void* find_release_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_release_fct_##A == (uintptr_t)fct) return my_release_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_release_fct_##A == 0) {my_release_fct_##A = (uintptr_t)fct; return my_release_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse release callback\n");
    return NULL;
}

// fsync
#define GO(A)   \
static uintptr_t my_fsync_fct_##A = 0;                              \
static void my_fsync_##A(void* a, unsigned long b, int c, void* d)  \
{                                                                   \
    RunFunction(my_context, my_fsync_fct_##A, 4, a, b, c, d);       \
}
SUPER()
#undef GO
static void* find_fsync_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_fsync_fct_##A == (uintptr_t)fct) return my_fsync_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_fsync_fct_##A == 0) {my_fsync_fct_##A = (uintptr_t)fct; return my_fsync_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse fsync callback\n");
    return NULL;
}

// opendir
#define GO(A)   \
static uintptr_t my_opendir_fct_##A = 0;                        \
static void my_opendir_##A(void* a, unsigned long b, void* c)   \
{                                                               \
    RunFunction(my_context, my_opendir_fct_##A, 3, a, b, c);    \
}
SUPER()
#undef GO
static void* find_opendir_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_opendir_fct_##A == (uintptr_t)fct) return my_opendir_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_opendir_fct_##A == 0) {my_opendir_fct_##A = (uintptr_t)fct; return my_opendir_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse opendir callback\n");
    return NULL;
}

// readdir
#define GO(A)   \
static uintptr_t my_readdir_fct_##A = 0;                                            \
static void my_readdir_##A(void* a, unsigned long b, size_t c, off_t d, void* e)    \
{                                                                                   \
    RunFunction(my_context, my_readdir_fct_##A, 5, a, b, c, d, e);                  \
}
SUPER()
#undef GO
static void* find_readdir_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_readdir_fct_##A == (uintptr_t)fct) return my_readdir_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_readdir_fct_##A == 0) {my_readdir_fct_##A = (uintptr_t)fct; return my_readdir_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse readdir callback\n");
    return NULL;
}

// releasedir
#define GO(A)   \
static uintptr_t my_releasedir_fct_##A = 0;                         \
static void my_releasedir_##A(void* a, unsigned long b, void* c)    \
{                                                                   \
    RunFunction(my_context, my_releasedir_fct_##A, 3, a, b, c);     \
}
SUPER()
#undef GO
static void* find_releasedir_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_releasedir_fct_##A == (uintptr_t)fct) return my_releasedir_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_releasedir_fct_##A == 0) {my_releasedir_fct_##A = (uintptr_t)fct; return my_releasedir_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse releasedir callback\n");
    return NULL;
}

// fsyncdir
#define GO(A)   \
static uintptr_t my_fsyncdir_fct_##A = 0;                               \
static void my_fsyncdir_##A(void* a, unsigned long b, int c, void* d)   \
{                                                                       \
    RunFunction(my_context, my_fsyncdir_fct_##A, 4, a, b, c, d);        \
}
SUPER()
#undef GO
static void* find_fsyncdir_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_fsyncdir_fct_##A == (uintptr_t)fct) return my_fsyncdir_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_fsyncdir_fct_##A == 0) {my_fsyncdir_fct_##A = (uintptr_t)fct; return my_fsyncdir_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse fsyncdir callback\n");
    return NULL;
}

// statfs
#define GO(A)   \
static uintptr_t my_statfs_fct_##A = 0;                     \
static void my_statfs_##A(void* a, unsigned long b)         \
{                                                           \
    RunFunction(my_context, my_statfs_fct_##A, 2, a, b);    \
}
SUPER()
#undef GO
static void* find_statfs_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_statfs_fct_##A == (uintptr_t)fct) return my_statfs_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_statfs_fct_##A == 0) {my_statfs_fct_##A = (uintptr_t)fct; return my_statfs_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse statfs callback\n");
    return NULL;
}

// setxattr
#define GO(A)   \
static uintptr_t my_setxattr_fct_##A = 0;                                                               \
static void my_setxattr_##A(void* a, unsigned long b, const char* c, const char* d, size_t e, int f)    \
{                                                                                                       \
    RunFunction(my_context, my_setxattr_fct_##A, 6, a, b, c, d, e, f);                                  \
}
SUPER()
#undef GO
static void* find_setxattr_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_setxattr_fct_##A == (uintptr_t)fct) return my_setxattr_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_setxattr_fct_##A == 0) {my_setxattr_fct_##A = (uintptr_t)fct; return my_setxattr_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse setxattr callback\n");
    return NULL;
}

// getxattr
#define GO(A)   \
static uintptr_t my_getxattr_fct_##A = 0;                                                       \
static void my_getxattr_##A(void* a, unsigned long b, const char* c, const char* d, size_t e)   \
{                                                                                               \
    RunFunction(my_context, my_getxattr_fct_##A, 5, a, b, c, d, e);                             \
}
SUPER()
#undef GO
static void* find_getxattr_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_getxattr_fct_##A == (uintptr_t)fct) return my_getxattr_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_getxattr_fct_##A == 0) {my_getxattr_fct_##A = (uintptr_t)fct; return my_getxattr_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse getxattr callback\n");
    return NULL;
}

// listxattr
#define GO(A)   \
static uintptr_t my_listxattr_fct_##A = 0;                      \
static void my_listxattr_##A(void* a, unsigned long b, size_t c)\
{                                                               \
    RunFunction(my_context, my_listxattr_fct_##A, 3, a, b, c);  \
}
SUPER()
#undef GO
static void* find_listxattr_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_listxattr_fct_##A == (uintptr_t)fct) return my_listxattr_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_listxattr_fct_##A == 0) {my_listxattr_fct_##A = (uintptr_t)fct; return my_listxattr_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse listxattr callback\n");
    return NULL;
}

// removexattr
#define GO(A)   \
static uintptr_t my_removexattr_fct_##A = 0;                            \
static void my_removexattr_##A(void* a, unsigned long b, const char* c) \
{                                                                       \
    RunFunction(my_context, my_removexattr_fct_##A, 3, a, b, c);        \
}
SUPER()
#undef GO
static void* find_removexattr_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_removexattr_fct_##A == (uintptr_t)fct) return my_removexattr_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_removexattr_fct_##A == 0) {my_removexattr_fct_##A = (uintptr_t)fct; return my_removexattr_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse removexattr callback\n");
    return NULL;
}

// access
#define GO(A)   \
static uintptr_t my_access_fct_##A = 0;                     \
static void my_access_##A(void* a, unsigned long b, int c)  \
{                                                           \
    RunFunction(my_context, my_access_fct_##A, 3, a, b, c); \
}
SUPER()
#undef GO
static void* find_access_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_access_fct_##A == (uintptr_t)fct) return my_access_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_access_fct_##A == 0) {my_access_fct_##A = (uintptr_t)fct; return my_access_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse access callback\n");
    return NULL;
}

// create
#define GO(A)   \
static uintptr_t my_create_fct_##A = 0;                                                 \
static void my_create_##A(void* a, unsigned long b, const char* c, mode_t d, void* e)   \
{                                                                                       \
    RunFunction(my_context, my_create_fct_##A, 5, a, b, c, of_convert(d), e);           \
}
SUPER()
#undef GO
static void* find_create_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_create_fct_##A == (uintptr_t)fct) return my_create_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_create_fct_##A == 0) {my_create_fct_##A = (uintptr_t)fct; return my_create_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse create callback\n");
    return NULL;
}

// getlk
#define GO(A)   \
static uintptr_t my_getlk_fct_##A = 0;                              \
static void my_getlk_##A(void* a, unsigned long b, void* c, void* d)\
{                                                                   \
    RunFunction(my_context, my_getlk_fct_##A, 4, a, b, c, d);       \
}
SUPER()
#undef GO
static void* find_getlk_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_getlk_fct_##A == (uintptr_t)fct) return my_getlk_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_getlk_fct_##A == 0) {my_getlk_fct_##A = (uintptr_t)fct; return my_getlk_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse getlk callback\n");
    return NULL;
}

// setlk
#define GO(A)   \
static uintptr_t my_setlk_fct_##A = 0;                                      \
static void my_setlk_##A(void* a, unsigned long b, void* c, void* d, int e) \
{                                                                           \
    RunFunction(my_context, my_setlk_fct_##A, 5, a, b, c, d, e);            \
}
SUPER()
#undef GO
static void* find_setlk_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_setlk_fct_##A == (uintptr_t)fct) return my_setlk_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_setlk_fct_##A == 0) {my_setlk_fct_##A = (uintptr_t)fct; return my_setlk_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse setlk callback\n");
    return NULL;
}

// bmap
#define GO(A)   \
static uintptr_t my_bmap_fct_##A = 0;                                   \
static void my_bmap_##A(void* a, unsigned long b, size_t c, uint64_t d) \
{                                                                       \
    RunFunction(my_context, my_bmap_fct_##A, 4, a, b, c, d);            \
}
SUPER()
#undef GO
static void* find_bmap_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_bmap_fct_##A == (uintptr_t)fct) return my_bmap_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_bmap_fct_##A == 0) {my_bmap_fct_##A = (uintptr_t)fct; return my_bmap_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse bmap callback\n");
    return NULL;
}

#undef SUPER

EXPORT int my_fuse_opt_parse(x64emu_t* emu, void* args, void* data, void* opts, void* f)
{
    libfuse_my_t *my = (libfuse_my_t*)my_lib->priv.w.p2;
    return my->fuse_opt_parse(args, data, opts, findfuse_opt_procFct(f));
}

EXPORT void* my_fuse_lowlevel_new(x64emu_t* emu, void* args, fuse_lowlevel_ops_t *o, size_t sz, void* data)
{
    libfuse_my_t *my = (libfuse_my_t*)my_lib->priv.w.p2;
    static fuse_lowlevel_ops_t o_;
    #define GO(A) o_.A = find_##A##_Fct(o->A); if(o_.A) printf_log(LOG_DEBUG, "fuse: %s is present\n", #A)
    GO(init);
    GO(destroy);
    GO(lookup);
    GO(forget);
    GO(getattr);
    GO(setattr);
    GO(readlink);
    GO(mknod);
    GO(mkdir);
    GO(unlink);
    GO(rmdir);
    GO(symlink);
    GO(rename);
    GO(link);
    GO(open);
    GO(read);
    GO(write);
    GO(flush);
    GO(release);
    GO(fsync);
    GO(opendir);
    GO(readdir);
    GO(releasedir);
    GO(fsyncdir);
    GO(statfs);
    GO(setxattr);
    GO(getxattr);
    GO(listxattr);
    GO(removexattr);
    GO(access);
    GO(create);
    GO(getlk);
    GO(setlk);
    GO(bmap);
    #undef GO
    return my->fuse_lowlevel_new(args, &o_, sz, data);
}

#define CUSTOM_INIT                 \
    lib->priv.w.p2 = getFuseMy(lib);

#define CUSTOM_FINI             \
    freeFuseMy(lib->priv.w.p2);  \
    free(lib->priv.w.p2);
    
#include "wrappedlib_init.h"
