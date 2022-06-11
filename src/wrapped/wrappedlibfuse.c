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

const char* libfuseName = "libfuse.so.2";
#define LIBNAME libfuse

#include "generated/wrappedlibfusetypes.h"

#include "wrappercallback.h"

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
    void (*ioctl) (void* req, unsigned long ino, int cmd, void *arg, void* fi, unsigned flags, void* in_buf, size_t in_bufsz, size_t out_bufsz);
    void (*poll) (void* req, unsigned long ino, void* fi, void* ph);
    void (*write_buf) (void* req, unsigned long ino, void* bufv, off_t off, void* fi);
    void (*retrieve_reply) (void* req, void *cookie, unsigned long ino, off_t offset, void* bufv);
    void (*forget_multi) (void* req, size_t count, void* forgets);
    void (*flock) (void* req, unsigned long ino, void* fi, int op);
    void (*fallocate) (void* req, unsigned long ino, int mode, off_t offset, off_t length, void* fi);
} fuse_lowlevel_ops_t;

typedef struct fuse_entry_param_s {
        unsigned long ino;
        unsigned long generation;
        struct stat attr;
        double attr_timeout;
        double entry_timeout;
} fuse_entry_param_t;

typedef struct x64_entry_param_s {
        unsigned long ino;
        unsigned long generation;
        struct x64_stat64 attr;
        double attr_timeout;
        double entry_timeout;
} x64_entry_param_t;

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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "init");               \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "destroy");                \
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
static uintptr_t my_lookup_fct_##A = 0;                                         \
static void my_lookup_##A(void* a, unsigned long b, const char* c)              \
{                                                                               \
    printf_log(LOG_DEBUG, "fuse: call %s(%p, %lu, %s)\n", "lookup", a, b, c);   \
    RunFunction(my_context, my_lookup_fct_##A, 3, a, b, c);                     \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "forget");             \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "getattr");        \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "setattr");        \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "readlink");               \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "mknod");              \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "mkdir");              \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "unlink");             \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "rmdir");              \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "symlink");                \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "rename");             \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "link");               \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "open");               \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "read");               \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "write");              \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "flush");              \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "release");                \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "fsync");              \
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
static uintptr_t my_opendir_fct_##A = 0;                                        \
static void my_opendir_##A(void* a, unsigned long b, void* c)                   \
{                                                                               \
    printf_log(LOG_DEBUG, "fuse: call %s(%p, %lu, %p)\n", "opendir", a, b, c);  \
    RunFunction(my_context, my_opendir_fct_##A, 3, a, b, c);                    \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "readdir");                \
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
static uintptr_t my_releasedir_fct_##A = 0;                                         \
static void my_releasedir_##A(void* a, unsigned long b, void* c)                    \
{                                                                                   \
    printf_log(LOG_DEBUG, "fuse: call %s(%p, %lu, %p)\n", "releasedir", a, b, c);   \
    RunFunction(my_context, my_releasedir_fct_##A, 3, a, b, c);                     \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "fsyncdir");               \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "statfs");             \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "setxattr");               \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "getxattr");               \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "listxattr");              \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "removexattr");                \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "access");             \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "create");             \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "getlk");              \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "setlk");              \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "bmap");               \
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

// ioctl
#define GO(A)   \
static uintptr_t my_ioctl_fct_##A = 0;                                                                              \
static void my_ioctl_##A(void* a, unsigned long b, int c, void* d, void* e, unsigned f, void* g, size_t h, size_t i)\
{                                                                                                                   \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "ioctl");                                                              \
    RunFunction(my_context, my_ioctl_fct_##A, 9, a, b, c, d, e, f, g, h, i);                                        \
}
SUPER()
#undef GO
static void* find_ioctl_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_ioctl_fct_##A == (uintptr_t)fct) return my_ioctl_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ioctl_fct_##A == 0) {my_ioctl_fct_##A = (uintptr_t)fct; return my_ioctl_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse ioctl callback\n");
    return NULL;
}

// poll
#define GO(A)   \
static uintptr_t my_poll_fct_##A = 0;                                  \
static void my_poll_##A(void* a, unsigned long b, void* c, void* d)    \
{                                                                       \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "poll");                  \
    RunFunction(my_context, my_poll_fct_##A, 4, a, b, c, d);           \
}
SUPER()
#undef GO
static void* find_poll_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_poll_fct_##A == (uintptr_t)fct) return my_poll_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_poll_fct_##A == 0) {my_poll_fct_##A = (uintptr_t)fct; return my_poll_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse poll callback\n");
    return NULL;
}

// write_buf
#define GO(A)   \
static uintptr_t my_write_buf_fct_##A = 0;                                          \
static void my_write_buf_##A(void* a, unsigned long b, void* c, off_t d, void* e)   \
{                                                                                   \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "write_buf");                          \
    RunFunction(my_context, my_write_buf_fct_##A, 5, a, b, c, d, e);                \
}
SUPER()
#undef GO
static void* find_write_buf_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_write_buf_fct_##A == (uintptr_t)fct) return my_write_buf_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_write_buf_fct_##A == 0) {my_write_buf_fct_##A = (uintptr_t)fct; return my_write_buf_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse write_buf callback\n");
    return NULL;
}

// retrieve_reply
#define GO(A)   \
static uintptr_t my_retrieve_reply_fct_##A = 0;                                         \
static void my_retrieve_reply_##A(void* a, void* b, unsigned long c, off_t d, void* e)  \
{                                                                                       \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "retrieve_reply");                         \
    RunFunction(my_context, my_retrieve_reply_fct_##A, 5, a, b, c, d, e);               \
}
SUPER()
#undef GO
static void* find_retrieve_reply_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_retrieve_reply_fct_##A == (uintptr_t)fct) return my_retrieve_reply_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_retrieve_reply_fct_##A == 0) {my_retrieve_reply_fct_##A = (uintptr_t)fct; return my_retrieve_reply_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse retrieve_reply callback\n");
    return NULL;
}

// forget_multi
#define GO(A)   \
static uintptr_t my_forget_multi_fct_##A = 0;                       \
static void my_forget_multi_##A(void* a, size_t b, void* c)         \
{                                                                   \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "forget_multi");       \
    RunFunction(my_context, my_forget_multi_fct_##A, 3, a, b, c);   \
}
SUPER()
#undef GO
static void* find_forget_multi_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_forget_multi_fct_##A == (uintptr_t)fct) return my_forget_multi_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_forget_multi_fct_##A == 0) {my_forget_multi_fct_##A = (uintptr_t)fct; return my_forget_multi_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse forget_multi callback\n");
    return NULL;
}

// flock
#define GO(A)   \
static uintptr_t my_flock_fct_##A = 0;                              \
static void my_flock_##A(void* a, unsigned long b, void* c, int d)  \
{                                                                   \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "flock");              \
    RunFunction(my_context, my_flock_fct_##A, 4, a, b, c, d);       \
}
SUPER()
#undef GO
static void* find_flock_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_flock_fct_##A == (uintptr_t)fct) return my_flock_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_flock_fct_##A == 0) {my_flock_fct_##A = (uintptr_t)fct; return my_flock_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse flock callback\n");
    return NULL;
}

// fallocate
#define GO(A)   \
static uintptr_t my_fallocate_fct_##A = 0;                                                  \
static void my_fallocate_##A(void* a, unsigned long b, int c, off_t d, off_t e, void* f)    \
{                                                                                           \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "fallocate");                                  \
    RunFunction(my_context, my_fallocate_fct_##A, 6, a, b, c, d, e, f);                     \
}
SUPER()
#undef GO
static void* find_fallocate_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_fallocate_fct_##A == (uintptr_t)fct) return my_fallocate_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_fallocate_fct_##A == 0) {my_fallocate_fct_##A = (uintptr_t)fct; return my_fallocate_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse fallocate callback\n");
    return NULL;
}

#undef SUPER

EXPORT int my_fuse_opt_parse(x64emu_t* emu, void* args, void* data, void* opts, void* f)
{
    return my->fuse_opt_parse(args, data, opts, findfuse_opt_procFct(f));
}

EXPORT void* my_fuse_lowlevel_new(x64emu_t* emu, void* args, fuse_lowlevel_ops_t *o, size_t sz, void* data)
{
    static fuse_lowlevel_ops_t o_;
    size_t cvt = 0;
    #define GO(A) if(cvt<=sz) {o_.A = find_##A##_Fct(o->A); cvt+=sizeof(void*); if(o_.A) printf_log(LOG_DEBUG, "fuse: %s is present\n", #A);}
    GO(init)
    GO(destroy)
    GO(lookup)
    GO(forget)
    GO(getattr)
    GO(setattr)
    GO(readlink)
    GO(mknod)
    GO(mkdir)
    GO(unlink)
    GO(rmdir)
    GO(symlink)
    GO(rename)
    GO(link)
    GO(open)
    GO(read)
    GO(write)
    GO(flush)
    GO(release)
    GO(fsync)
    GO(opendir)
    GO(readdir)
    GO(releasedir)
    GO(fsyncdir)
    GO(statfs)
    GO(setxattr)
    GO(getxattr)
    GO(listxattr)
    GO(removexattr)
    GO(access)
    GO(create)
    GO(getlk)
    GO(setlk)
    GO(bmap)
    GO(ioctl)
    GO(poll)
    GO(write_buf)
    GO(retrieve_reply)
    GO(forget_multi)
    GO(flock)
    GO(fallocate)
    #undef GO
    printf_log(LOG_DEBUG, "fuse_lowlevel_new called with size_ops=%zd vs %zd\n", sz, cvt);
    return my->fuse_lowlevel_new(args, &o_, cvt, data);
}

EXPORT int my_fuse_reply_entry(x64emu_t* emu, void* req, const x64_entry_param_t *e)
{
    static fuse_entry_param_t e_;
    if(e) {
        e_.ino = e->ino;
        e_.generation = e->generation;
        e_.attr_timeout = e->attr_timeout;
        e_.entry_timeout = e->entry_timeout;
        AlignStat64(&e->attr, &e_.attr);
    }
    return my->fuse_reply_entry(req, e?(void*)&e_:(void*)e);
}

EXPORT int my_fuse_reply_create(x64emu_t* emu, void* req, const x64_entry_param_t *e, void* fi)
{
    static fuse_entry_param_t e_;
    if(e) {
        e_.ino = e->ino;
        e_.generation = e->generation;
        e_.attr_timeout = e->attr_timeout;
        e_.entry_timeout = e->entry_timeout;
        AlignStat64(&e->attr, &e_.attr);
    }
    return my->fuse_reply_create(req, e?(void*)&e_:(void*)e, fi);
}

EXPORT int my_fuse_reply_attr(x64emu_t* emu, void* req, const struct x64_stat64 *attr, double attr_timeout)
{
    static struct stat attr_;
    if(attr) AlignStat64(attr, &attr_);
    return my->fuse_reply_attr(req, attr?(void*)&attr_:(void*)attr, attr_timeout);
}

EXPORT size_t my_fuse_add_direntry(x64emu_t* emu, void* req, char *buf, size_t bufsize, void* name, const struct x64_stat64 *stbuf, off_t off)
{
    static struct stat stbuf_;
    if(stbuf) AlignStat64(stbuf, &stbuf_);
    return my->fuse_add_direntry(req, buf, bufsize, name, stbuf?(void*)&stbuf_:(void*)stbuf, off);
}


#define CUSTOM_INIT                 \
    getMy(lib);

#define CUSTOM_FINI             \
    freeMy();
    
#include "wrappedlib_init.h"
