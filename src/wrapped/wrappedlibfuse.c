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

typedef int(* fuse_fill_dir_t) (void *buf, const char *name, const struct stat *stbuf, off_t off, int flags);

typedef struct fuse_operations_s {
    int (*getattr_op) (const char *, struct stat *);
    int (*readlink_op) (const char *, char *, size_t);
    int (*getdir_op) (const char *, void*, void*);
    int (*mknod_op) (const char *, mode_t, dev_t);
    int (*mkdir_op) (const char *, mode_t);
    int (*unlink_op) (const char *);
    int (*rmdir_op) (const char *);
    int (*symlink_op) (const char *, const char *);
    int (*rename_op) (const char *, const char *);
    int (*link_op) (const char *, const char *);
    int (*chmod_op) (const char *, mode_t);
    int (*chown_op) (const char *, uid_t, gid_t);
    int (*truncate_op) (const char *, off_t);
    int (*utime_op) (const char *, void *);
    int (*open_op) (const char *, void* );
    int (*read_op) (const char *, char *, size_t, off_t, void* );
    int (*write_op) (const char *, const char *, size_t, off_t, void* );
    int (*statfs_op) (const char *, void*);
    int (*flush_op) (const char *, void* );
    int (*release_op) (const char *, void* );
    int (*fsync_op) (const char *, int, void* );
    int (*setxattr_op) (const char *, const char *, const char *, size_t, int);
    int (*getxattr_op) (const char *, const char *, char *, size_t);
    int (*listxattr_op) (const char *, char *, size_t);
    int (*removexattr_op) (const char *, const char *);
    int (*opendir_op) (const char *, void* );
    int (*readdir_op) (const char *, void *, fuse_fill_dir_t, off_t, void*);
    int (*releasedir_op) (const char *, void* );
    int (*fsyncdir_op) (const char *, int, void* );
    void *(*init_op) (void* conn);
    void (*destroy_op) (void *private_data);
    int (*access_op) (const char *, int);
    int (*create_op) (const char *, mode_t, void* );
    int (*ftruncate_op) (const char *, off_t, void*);
    int (*fgetattr_op) (const char *, struct stat*, void*);
    int (*lock_op) (const char *, void* , int cmd, void*);
    int (*utimens_op) (const char *, const struct timespec tv[2]);
    int (*bmap_op) (const char *, size_t blocksize, uint64_t *idx);
    uint32_t flags;
    int (*ioctl_op) (const char *, int cmd, void *arg, void* , unsigned int flags, void *data);
    int (*poll_op) (const char *, void* , void* ph, unsigned *reventsp);
    int (*write_buf_op) (const char *, void* buf, off_t off, void* );
    int (*read_buf_op) (const char *, void* bufp, size_t size, off_t off, void* );
    int (*flock_op) (const char *, void* , int op);
    int (*fallocate_op) (const char *, int, off_t, off_t, void* );
 } fuse_operations_t;

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
static uintptr_t my_fuse_opt_proc_fct_##A = 0;                                              \
static int my_fuse_opt_proc_##A(void* a, void* b, int c, void* d)                           \
{                                                                                           \
    return (int)RunFunctionFmt(my_fuse_opt_proc_fct_##A, "ppip", a, b, c, d);   \
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
static uintptr_t my_init_fct_##A = 0;                           \
static void my_init_##A(void* a, void* b)                       \
{                                                               \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "init");           \
    RunFunctionFmt(my_init_fct_##A, "pp", a, b);    \
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
static uintptr_t my_destroy_fct_##A = 0;                    \
static void my_destroy_##A(void* a)                         \
{                                                           \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "destroy");    \
    RunFunctionFmt(my_destroy_fct_##A, "p", a); \
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
    RunFunctionFmt(my_lookup_fct_##A, "pLp", a, b, c);              \
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
static uintptr_t my_forget_fct_##A = 0;                                 \
static void my_forget_##A(void* a, unsigned long b, unsigned long c)    \
{                                                                       \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "forget");                 \
    RunFunctionFmt(my_forget_fct_##A, "pLL", a, b, c);      \
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
static uintptr_t my_getattr_fct_##A = 0;                            \
static void my_getattr_##A(void* a, unsigned long b, void* c)       \
{                                                                   \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "getattr");            \
    RunFunctionFmt(my_getattr_fct_##A, "pLp", a, b, c); \
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
static uintptr_t my_setattr_fct_##A = 0;                                            \
static void my_setattr_##A(void* a, unsigned long b, struct stat* c, int d, void* e)\
{                                                                                   \
    struct stat c_;                                                                 \
    AlignStat64(c, &c_);                                                            \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "setattr");                            \
    RunFunctionFmt(my_setattr_fct_##A, "pLpip", a, b, &c_, d, e);       \
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
static uintptr_t my_readlink_fct_##A = 0;                           \
static void my_readlink_##A(void* a, unsigned long b)               \
{                                                                   \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "readlink");           \
    RunFunctionFmt(my_readlink_fct_##A, "pL", a, b);    \
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
    printf_log(LOG_DEBUG, "fuse: call %s\n", "mknod");                                  \
    RunFunctionFmt(my_mknod_fct_##A, "pLpuL", a, b, c, of_convert(d), e);   \
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
static uintptr_t my_mkdir_fct_##A = 0;                                              \
static void my_mkdir_##A(void* a, unsigned long b, const char* c, mode_t d)         \
{                                                                                   \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "mkdir");                              \
    RunFunctionFmt(my_mkdir_fct_##A, "pLpu", a, b, c, of_convert(d));   \
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
    RunFunctionFmt(my_unlink_fct_##A, "pLp", a, b, c);  \
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
static uintptr_t my_rmdir_fct_##A = 0;                              \
static void my_rmdir_##A(void* a, unsigned long b, const char* c)   \
{                                                                   \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "rmdir");              \
    RunFunctionFmt(my_rmdir_fct_##A, "pLp", a, b, c);   \
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
static uintptr_t my_symlink_fct_##A = 0;                                \
static void my_symlink_##A(void* a, const char* b, unsigned long c, const char* d)  \
{                                                                       \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "symlink");                \
    RunFunctionFmt(my_symlink_fct_##A, "ppLp", a, b, c, d); \
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
static uintptr_t my_rename_fct_##A = 0;                                     \
static void my_rename_##A(void* a, unsigned long b, const char* c, unsigned long d, const char* e)  \
{                                                                           \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "rename");                     \
    RunFunctionFmt(my_rename_fct_##A, "pLpLp", a, b, c, d, e);  \
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
static uintptr_t my_link_fct_##A = 0;                                   \
static void my_link_##A(void* a, unsigned long b, unsigned long c, const char* d)   \
{                                                                       \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "link");                   \
    RunFunctionFmt(my_link_fct_##A, "pLLp", a, b, c, d);    \
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
static uintptr_t my_open_fct_##A = 0;                               \
static void my_open_##A(void* a, unsigned long b, const char* c)    \
{                                                                   \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "open");               \
    RunFunctionFmt(my_open_fct_##A, "pLp", a, b, c);    \
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
static uintptr_t my_read_fct_##A = 0;                                           \
static void my_read_##A(void* a, unsigned long b, const char* c, size_t d, off_t e, void* f)    \
{                                                                               \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "read");                           \
    RunFunctionFmt(my_read_fct_##A, "pLpLlp", a, b, c, d, e, f);    \
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
static uintptr_t my_write_fct_##A = 0;                                          \
static void my_write_##A(void* a, unsigned long b, const char* c, size_t d, off_t e, void* f)   \
{                                                                               \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "write");                          \
    RunFunctionFmt(my_write_fct_##A, "pLpLlp", a, b, c, d, e, f);   \
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
    RunFunctionFmt(my_flush_fct_##A, "pLp", a, b, c);   \
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
static uintptr_t my_release_fct_##A = 0;                            \
static void my_release_##A(void* a, unsigned long b, const char* c) \
{                                                                   \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "release");            \
    RunFunctionFmt(my_release_fct_##A, "pLp", a, b, c); \
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
static uintptr_t my_fsync_fct_##A = 0;                                  \
static void my_fsync_##A(void* a, unsigned long b, int c, void* d)      \
{                                                                       \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "fsync");                  \
    RunFunctionFmt(my_fsync_fct_##A, "pLip", a, b, c, d);   \
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
    RunFunctionFmt(my_opendir_fct_##A, "pLp", a, b, c);                    \
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
    RunFunctionFmt(my_readdir_fct_##A, "pLLlp", a, b, c, d, e);                  \
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
    RunFunctionFmt(my_releasedir_fct_##A, "pLp", a, b, c);                     \
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
    RunFunctionFmt(my_fsyncdir_fct_##A, "pLip", a, b, c, d);        \
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
    RunFunctionFmt(my_statfs_fct_##A, "pL", a, b);    \
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
    RunFunctionFmt(my_setxattr_fct_##A, "pLppLi", a, b, c, d, e, f);                                  \
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
    RunFunctionFmt(my_getxattr_fct_##A, "pLppL", a, b, c, d, e);                             \
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
    RunFunctionFmt(my_listxattr_fct_##A, "pLL", a, b, c);  \
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
    RunFunctionFmt(my_removexattr_fct_##A, "pLp", a, b, c);        \
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
    RunFunctionFmt(my_access_fct_##A, "pLi", a, b, c); \
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
    RunFunctionFmt(my_create_fct_##A, "pLpup", a, b, c, of_convert(d), e);           \
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
    RunFunctionFmt(my_getlk_fct_##A, "pLpp", a, b, c, d);       \
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
    RunFunctionFmt(my_setlk_fct_##A, "pLppi", a, b, c, d, e);            \
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
    RunFunctionFmt(my_bmap_fct_##A, "pLLU", a, b, c, d);            \
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
    RunFunctionFmt(my_ioctl_fct_##A, "pLippupLL", a, b, c, d, e, f, g, h, i);                                        \
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
    RunFunctionFmt(my_poll_fct_##A, "pLpp", a, b, c, d);           \
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
    RunFunctionFmt(my_write_buf_fct_##A, "pLplp", a, b, c, d, e);                \
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
    RunFunctionFmt(my_retrieve_reply_fct_##A, "ppLlp", a, b, c, d, e);               \
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
    RunFunctionFmt(my_forget_multi_fct_##A, "pLp", a, b, c);   \
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
    RunFunctionFmt(my_flock_fct_##A, "pLpi", a, b, c, d);       \
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
    RunFunctionFmt(my_fallocate_fct_##A, "pLillp", a, b, c, d, e, f);                     \
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

// getattr_op
#define GO(A)   \
static uintptr_t my_getattr_op_fct_##A = 0;                                         \
static int my_getattr_op_##A(const char * a, struct stat * b)                       \
{                                                                                   \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "getattr_op");                         \
    struct x64_stat64 b_;                                                           \
    UnalignStat64(b, &b_);                                                          \
    int ret = (int)RunFunctionFmt(my_getattr_op_fct_##A, "pp", a, &b_);       \
    AlignStat64(&b_, b);                                                            \
    return ret;                                                                     \
}
SUPER()
#undef GO
static void* find_getattr_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_getattr_op_fct_##A == (uintptr_t)fct) return my_getattr_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_getattr_op_fct_##A == 0) {my_getattr_op_fct_##A = (uintptr_t)fct; return my_getattr_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse getattr_op callback\n");
    return NULL;
}

// readlink_op
#define GO(A)   \
static uintptr_t my_readlink_op_fct_##A = 0;                            \
static int my_readlink_op_##A(const char * a, char * b, size_t c)       \
{                                                                       \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "readlink_op");            \
    return (int)RunFunctionFmt(my_readlink_op_fct_##A, "ppL", a, b, c);\
}
SUPER()
#undef GO
static void* find_readlink_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_readlink_op_fct_##A == (uintptr_t)fct) return my_readlink_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_readlink_op_fct_##A == 0) {my_readlink_op_fct_##A = (uintptr_t)fct; return my_readlink_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse readlink_op callback\n");
    return NULL;
}

// getdir_op
#define GO(A)   \
static uintptr_t my_getdir_op_fct_##A = 0;                                                                                      \
static int my_getdir_op_##A(const char * a, void* b, void* c)                                                                   \
{                                                                                                                               \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "getdir_op");                                                                      \
    return (int)RunFunctionFmt(my_getdir_op_fct_##A, "ppp", a, b, AddCheckBridge(my_lib->w.bridge, iFppiU, c, 0, NULL));   \
}
SUPER()
#undef GO
static void* find_getdir_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_getdir_op_fct_##A == (uintptr_t)fct) return my_getdir_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_getdir_op_fct_##A == 0) {my_getdir_op_fct_##A = (uintptr_t)fct; return my_getdir_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse getdir_op callback\n");
    return NULL;
}

// mknod_op
#define GO(A)   \
static uintptr_t my_mknod_op_fct_##A = 0;                                   \
static int my_mknod_op_##A(const char * a, mode_t b, dev_t c)               \
{                                                                           \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "mknod_op");                   \
    return (int)RunFunctionFmt(my_mknod_op_fct_##A, "puU", a, b, c);   \
}
SUPER()
#undef GO
static void* find_mknod_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_mknod_op_fct_##A == (uintptr_t)fct) return my_mknod_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_mknod_op_fct_##A == 0) {my_mknod_op_fct_##A = (uintptr_t)fct; return my_mknod_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse mknod_op callback\n");
    return NULL;
}

// mkdir_op
#define GO(A)   \
static uintptr_t my_mkdir_op_fct_##A = 0;                               \
static int my_mkdir_op_##A(const char * a, mode_t b)                    \
{                                                                       \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "mkdir_op");               \
    return (int)RunFunctionFmt(my_mkdir_op_fct_##A, "pu", a, b);  \
}
SUPER()
#undef GO
static void* find_mkdir_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_mkdir_op_fct_##A == (uintptr_t)fct) return my_mkdir_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_mkdir_op_fct_##A == 0) {my_mkdir_op_fct_##A = (uintptr_t)fct; return my_mkdir_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse mkdir_op callback\n");
    return NULL;
}

// unlink_op
#define GO(A)   \
static uintptr_t my_unlink_op_fct_##A = 0;                              \
static int my_unlink_op_##A(const char * a)                             \
{                                                                       \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "unlink_op");              \
    return (int)RunFunctionFmt(my_unlink_op_fct_##A, "p", a);    \
}
SUPER()
#undef GO
static void* find_unlink_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_unlink_op_fct_##A == (uintptr_t)fct) return my_unlink_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_unlink_op_fct_##A == 0) {my_unlink_op_fct_##A = (uintptr_t)fct; return my_unlink_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse unlink_op callback\n");
    return NULL;
}

// rmdir_op
#define GO(A)   \
static uintptr_t my_rmdir_op_fct_##A = 0;                           \
static int my_rmdir_op_##A(const char * a)                          \
{                                                                   \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "rmdir_op");           \
    return (int)RunFunctionFmt(my_rmdir_op_fct_##A, "p", a); \
}
SUPER()
#undef GO
static void* find_rmdir_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_rmdir_op_fct_##A == (uintptr_t)fct) return my_rmdir_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_rmdir_op_fct_##A == 0) {my_rmdir_op_fct_##A = (uintptr_t)fct; return my_rmdir_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse rmdir_op callback\n");
    return NULL;
}

// symlink_op
#define GO(A)   \
static uintptr_t my_symlink_op_fct_##A = 0;                                 \
static int my_symlink_op_##A(const char * a, const char * b)                \
{                                                                           \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "symlink_op");                 \
    return (int)RunFunctionFmt(my_symlink_op_fct_##A, "pp", a, b);    \
}
SUPER()
#undef GO
static void* find_symlink_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_symlink_op_fct_##A == (uintptr_t)fct) return my_symlink_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_symlink_op_fct_##A == 0) {my_symlink_op_fct_##A = (uintptr_t)fct; return my_symlink_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse symlink_op callback\n");
    return NULL;
}

// rename_op
#define GO(A)   \
static uintptr_t my_rename_op_fct_##A = 0;                              \
static int my_rename_op_##A(const char * a, const char * b)             \
{                                                                       \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "rename_op");              \
    return (int)RunFunctionFmt(my_rename_op_fct_##A, "pp", a, b); \
}
SUPER()
#undef GO
static void* find_rename_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_rename_op_fct_##A == (uintptr_t)fct) return my_rename_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_rename_op_fct_##A == 0) {my_rename_op_fct_##A = (uintptr_t)fct; return my_rename_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse rename_op callback\n");
    return NULL;
}

// link_op
#define GO(A)   \
static uintptr_t my_link_op_fct_##A = 0;                                \
static int my_link_op_##A(const char * a, const char * b)               \
{                                                                       \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "link_op");                \
    return (int)RunFunctionFmt(my_link_op_fct_##A, "pp", a, b);   \
}
SUPER()
#undef GO
static void* find_link_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_link_op_fct_##A == (uintptr_t)fct) return my_link_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_link_op_fct_##A == 0) {my_link_op_fct_##A = (uintptr_t)fct; return my_link_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse link_op callback\n");
    return NULL;
}

// chmod_op
#define GO(A)   \
static uintptr_t my_chmod_op_fct_##A = 0;                               \
static int my_chmod_op_##A(const char * a, mode_t b)                    \
{                                                                       \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "chmod_op");               \
    return (int)RunFunctionFmt(my_chmod_op_fct_##A, "pu", a, b);  \
}
SUPER()
#undef GO
static void* find_chmod_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_chmod_op_fct_##A == (uintptr_t)fct) return my_chmod_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_chmod_op_fct_##A == 0) {my_chmod_op_fct_##A = (uintptr_t)fct; return my_chmod_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse chmod_op callback\n");
    return NULL;
}

// chown_op
#define GO(A)   \
static uintptr_t my_chown_op_fct_##A = 0;                                   \
static int my_chown_op_##A(const char * a, uid_t b, gid_t c)                \
{                                                                           \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "chown_op");                   \
    return (int)RunFunctionFmt(my_chown_op_fct_##A, "puu", a, b, c);   \
}
SUPER()
#undef GO
static void* find_chown_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_chown_op_fct_##A == (uintptr_t)fct) return my_chown_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_chown_op_fct_##A == 0) {my_chown_op_fct_##A = (uintptr_t)fct; return my_chown_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse chown_op callback\n");
    return NULL;
}

// truncate_op
#define GO(A)   \
static uintptr_t my_truncate_op_fct_##A = 0;                                \
static int my_truncate_op_##A(const char * a, off_t b)                      \
{                                                                           \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "truncate_op");                \
    return (int)RunFunctionFmt(my_truncate_op_fct_##A, "pl", a, b);   \
}
SUPER()
#undef GO
static void* find_truncate_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_truncate_op_fct_##A == (uintptr_t)fct) return my_truncate_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_truncate_op_fct_##A == 0) {my_truncate_op_fct_##A = (uintptr_t)fct; return my_truncate_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse truncate_op callback\n");
    return NULL;
}

// utime_op
#define GO(A)   \
static uintptr_t my_utime_op_fct_##A = 0;                               \
static int my_utime_op_##A(const char * a, void* b)                     \
{                                                                       \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "utime_op");               \
    return (int)RunFunctionFmt(my_utime_op_fct_##A, "pp", a, b);  \
}
SUPER()
#undef GO
static void* find_utime_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_utime_op_fct_##A == (uintptr_t)fct) return my_utime_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_utime_op_fct_##A == 0) {my_utime_op_fct_##A = (uintptr_t)fct; return my_utime_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse utime_op callback\n");
    return NULL;
}

// open_op
#define GO(A)   \
static uintptr_t my_open_op_fct_##A = 0;                                \
static int my_open_op_##A(const char * a, void* b)                      \
{                                                                       \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "open_op");                \
    return (int)RunFunctionFmt(my_open_op_fct_##A, "pp", a, b);   \
}
SUPER()
#undef GO
static void* find_open_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_open_op_fct_##A == (uintptr_t)fct) return my_open_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_open_op_fct_##A == 0) {my_open_op_fct_##A = (uintptr_t)fct; return my_open_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse open_op callback\n");
    return NULL;
}

// read_op
#define GO(A)   \
static uintptr_t my_read_op_fct_##A = 0;                                        \
static int my_read_op_##A(const char * a, char * b, size_t c, off_t d, void* e) \
{                                                                               \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "read_op");                        \
    return (int)RunFunctionFmt(my_read_op_fct_##A, "ppLlp", a, b, c, d, e);  \
}
SUPER()
#undef GO
static void* find_read_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_read_op_fct_##A == (uintptr_t)fct) return my_read_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_read_op_fct_##A == 0) {my_read_op_fct_##A = (uintptr_t)fct; return my_read_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse read_op callback\n");
    return NULL;
}

// write_op
#define GO(A)   \
static uintptr_t my_write_op_fct_##A = 0;                                               \
static int my_write_op_##A(const char * a, const char * b, size_t c, off_t d, void* e)  \
{                                                                                       \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "write_op");                               \
    return (int)RunFunctionFmt(my_write_op_fct_##A, "ppLlp", a, b, c, d, e);         \
}
SUPER()
#undef GO
static void* find_write_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_write_op_fct_##A == (uintptr_t)fct) return my_write_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_write_op_fct_##A == 0) {my_write_op_fct_##A = (uintptr_t)fct; return my_write_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse write_op callback\n");
    return NULL;
}

// statfs_op
#define GO(A)   \
static uintptr_t my_statfs_op_fct_##A = 0;                              \
static int my_statfs_op_##A(const char * a, void* b)         \
{                                                                       \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "statfs_op");              \
    return (int)RunFunctionFmt(my_statfs_op_fct_##A, "pp", a, b); \
}
SUPER()
#undef GO
static void* find_statfs_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_statfs_op_fct_##A == (uintptr_t)fct) return my_statfs_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_statfs_op_fct_##A == 0) {my_statfs_op_fct_##A = (uintptr_t)fct; return my_statfs_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse statfs_op callback\n");
    return NULL;
}

// flush_op
#define GO(A)   \
static uintptr_t my_flush_op_fct_##A = 0;                               \
static int my_flush_op_##A(const char * a, void* b)                     \
{                                                                       \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "flush_op");               \
    return (int)RunFunctionFmt(my_flush_op_fct_##A, "pp", a, b);  \
}
SUPER()
#undef GO
static void* find_flush_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_flush_op_fct_##A == (uintptr_t)fct) return my_flush_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_flush_op_fct_##A == 0) {my_flush_op_fct_##A = (uintptr_t)fct; return my_flush_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse flush_op callback\n");
    return NULL;
}

// release_op
#define GO(A)   \
static uintptr_t my_release_op_fct_##A = 0;                                 \
static int my_release_op_##A(const char * a, void* b)                       \
{                                                                           \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "release_op");                 \
    return (int)RunFunctionFmt(my_release_op_fct_##A, "pp", a, b);    \
}
SUPER()
#undef GO
static void* find_release_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_release_op_fct_##A == (uintptr_t)fct) return my_release_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_release_op_fct_##A == 0) {my_release_op_fct_##A = (uintptr_t)fct; return my_release_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse release_op callback\n");
    return NULL;
}

// fsync_op
#define GO(A)   \
static uintptr_t my_fsync_op_fct_##A = 0;                                   \
static int my_fsync_op_##A(const char * a, int b, void* c)                  \
{                                                                           \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "fsync_op");                   \
    return (int)RunFunctionFmt(my_fsync_op_fct_##A, "pip", a, b, c);   \
}
SUPER()
#undef GO
static void* find_fsync_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_fsync_op_fct_##A == (uintptr_t)fct) return my_fsync_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_fsync_op_fct_##A == 0) {my_fsync_op_fct_##A = (uintptr_t)fct; return my_fsync_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse fsync_op callback\n");
    return NULL;
}

// setxattr_op
#define GO(A)   \
static uintptr_t my_setxattr_op_fct_##A = 0;                                                    \
static int my_setxattr_op_##A(const char * a, const char * b, const char * c, size_t d, int e)  \
{                                                                                               \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "setxattr_op");                                    \
    return (int)RunFunctionFmt(my_setxattr_op_fct_##A, "pppLi", a, b, c, d, e);                  \
}
SUPER()
#undef GO
static void* find_setxattr_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_setxattr_op_fct_##A == (uintptr_t)fct) return my_setxattr_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_setxattr_op_fct_##A == 0) {my_setxattr_op_fct_##A = (uintptr_t)fct; return my_setxattr_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse setxattr_op callback\n");
    return NULL;
}

// getxattr_op
#define GO(A)   \
static uintptr_t my_getxattr_op_fct_##A = 0;                                        \
static int my_getxattr_op_##A(const char * a, const char * b, char * c, size_t d)   \
{                                                                                   \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "getxattr_op");                        \
    return (int)RunFunctionFmt(my_getxattr_op_fct_##A, "pppL", a, b, c, d);     \
}
SUPER()
#undef GO
static void* find_getxattr_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_getxattr_op_fct_##A == (uintptr_t)fct) return my_getxattr_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_getxattr_op_fct_##A == 0) {my_getxattr_op_fct_##A = (uintptr_t)fct; return my_getxattr_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse getxattr_op callback\n");
    return NULL;
}

// listxattr_op
#define GO(A)   \
static uintptr_t my_listxattr_op_fct_##A = 0;                                   \
static int my_listxattr_op_##A(const char * a, char * b, size_t c)              \
{                                                                               \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "listxattr_op");                   \
    return (int)RunFunctionFmt(my_listxattr_op_fct_##A, "ppL", a, b, c);   \
}
SUPER()
#undef GO
static void* find_listxattr_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_listxattr_op_fct_##A == (uintptr_t)fct) return my_listxattr_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_listxattr_op_fct_##A == 0) {my_listxattr_op_fct_##A = (uintptr_t)fct; return my_listxattr_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse listxattr_op callback\n");
    return NULL;
}

// removexattr_op
#define GO(A)   \
static uintptr_t my_removexattr_op_fct_##A = 0;                                 \
static int my_removexattr_op_##A(const char * a, const char * b)                \
{                                                                               \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "removexattr_op");                 \
    return (int)RunFunctionFmt(my_removexattr_op_fct_##A, "pp", a, b);    \
}
SUPER()
#undef GO
static void* find_removexattr_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_removexattr_op_fct_##A == (uintptr_t)fct) return my_removexattr_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_removexattr_op_fct_##A == 0) {my_removexattr_op_fct_##A = (uintptr_t)fct; return my_removexattr_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse removexattr_op callback\n");
    return NULL;
}

// opendir_op
#define GO(A)   \
static uintptr_t my_opendir_op_fct_##A = 0;                                 \
static int my_opendir_op_##A(const char * a, void* b)                       \
{                                                                           \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "opendir_op");                 \
    return (int)RunFunctionFmt(my_opendir_op_fct_##A, "pp", a, b);    \
}
SUPER()
#undef GO
static void* find_opendir_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_opendir_op_fct_##A == (uintptr_t)fct) return my_opendir_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_opendir_op_fct_##A == 0) {my_opendir_op_fct_##A = (uintptr_t)fct; return my_opendir_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse opendir_op callback\n");
    return NULL;
}

// readdir_op
#define GO(A)   \
static uintptr_t my_readdir_op_fct_##A = 0;                                                                                             \
static int my_readdir_op_##A(const char * a, void * b, fuse_fill_dir_t c, off_t d, void* e)                                             \
{                                                                                                                                       \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "readdir_op");                                                                             \
    return (int)RunFunctionFmt(my_readdir_op_fct_##A, "ppplp", a, b, AddCheckBridge(my_lib->w.bridge, iFpppUi, c, 0, NULL), d, e);   \
}
SUPER()
#undef GO
static void* find_readdir_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_readdir_op_fct_##A == (uintptr_t)fct) return my_readdir_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_readdir_op_fct_##A == 0) {my_readdir_op_fct_##A = (uintptr_t)fct; return my_readdir_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse readdir_op callback\n");
    return NULL;
}

// releasedir_op
#define GO(A)   \
static uintptr_t my_releasedir_op_fct_##A = 0;                              \
static int my_releasedir_op_##A(const char * a, void* b)                    \
{                                                                           \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "releasedir_op");              \
    return (int)RunFunctionFmt(my_releasedir_op_fct_##A, "pp", a, b);  \
}
SUPER()
#undef GO
static void* find_releasedir_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_releasedir_op_fct_##A == (uintptr_t)fct) return my_releasedir_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_releasedir_op_fct_##A == 0) {my_releasedir_op_fct_##A = (uintptr_t)fct; return my_releasedir_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse releasedir_op callback\n");
    return NULL;
}

// fsyncdir_op
#define GO(A)   \
static uintptr_t my_fsyncdir_op_fct_##A = 0;                                \
static int my_fsyncdir_op_##A(const char * a, int b, void* c)               \
{                                                                           \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "fsyncdir_op");                \
    return (int)RunFunctionFmt(my_fsyncdir_op_fct_##A, "pip", a, b, c);\
}
SUPER()
#undef GO
static void* find_fsyncdir_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_fsyncdir_op_fct_##A == (uintptr_t)fct) return my_fsyncdir_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_fsyncdir_op_fct_##A == 0) {my_fsyncdir_op_fct_##A = (uintptr_t)fct; return my_fsyncdir_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse fsyncdir_op callback\n");
    return NULL;
}

// init_op
#define GO(A)   \
static uintptr_t my_init_op_fct_##A = 0;                                \
static void* my_init_op_##A(void* a)                                    \
{                                                                       \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "init_op");                \
    return (void*)RunFunctionFmt(my_init_op_fct_##A, "p", a);    \
}
SUPER()
#undef GO
static void* find_init_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_init_op_fct_##A == (uintptr_t)fct) return my_init_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_init_op_fct_##A == 0) {my_init_op_fct_##A = (uintptr_t)fct; return my_init_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse init_op callback\n");
    return NULL;
}

// destroy_op
#define GO(A)   \
static uintptr_t my_destroy_op_fct_##A = 0;                 \
static void my_destroy_op_##A(void * a)                     \
{                                                           \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "destroy_op"); \
    RunFunctionFmt(my_destroy_op_fct_##A, "p", a);   \
}
SUPER()
#undef GO
static void* find_destroy_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_destroy_op_fct_##A == (uintptr_t)fct) return my_destroy_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_destroy_op_fct_##A == 0) {my_destroy_op_fct_##A = (uintptr_t)fct; return my_destroy_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse destroy_op callback\n");
    return NULL;
}

// access_op
#define GO(A)   \
static uintptr_t my_access_op_fct_##A = 0;                              \
static int my_access_op_##A(const char * a, int b)                      \
{                                                                       \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "access_op");              \
    return (int)RunFunctionFmt(my_access_op_fct_##A, "pi", a, b); \
}
SUPER()
#undef GO
static void* find_access_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_access_op_fct_##A == (uintptr_t)fct) return my_access_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_access_op_fct_##A == 0) {my_access_op_fct_##A = (uintptr_t)fct; return my_access_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse access_op callback\n");
    return NULL;
}

// create_op
#define GO(A)   \
static uintptr_t my_create_op_fct_##A = 0;                                  \
static int my_create_op_##A(const char * a, mode_t b, void* c)              \
{                                                                           \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "create_op");                  \
    return (int)RunFunctionFmt(my_create_op_fct_##A, "pup", a, b, c);  \
}
SUPER()
#undef GO
static void* find_create_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_create_op_fct_##A == (uintptr_t)fct) return my_create_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_create_op_fct_##A == 0) {my_create_op_fct_##A = (uintptr_t)fct; return my_create_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse create_op callback\n");
    return NULL;
}

// ftruncate_op
#define GO(A)   \
static uintptr_t my_ftruncate_op_fct_##A = 0;                                   \
static int my_ftruncate_op_##A(const char * a, off_t b, void* c)                \
{                                                                               \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "ftruncate_op");                   \
    return (int)RunFunctionFmt(my_ftruncate_op_fct_##A, "plp", a, b, c);   \
}
SUPER()
#undef GO
static void* find_ftruncate_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_ftruncate_op_fct_##A == (uintptr_t)fct) return my_ftruncate_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ftruncate_op_fct_##A == 0) {my_ftruncate_op_fct_##A = (uintptr_t)fct; return my_ftruncate_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse ftruncate_op callback\n");
    return NULL;
}

// fgetattr_op
#define GO(A)   \
static uintptr_t my_fgetattr_op_fct_##A = 0;                                        \
static int my_fgetattr_op_##A(const char * a, struct stat* b, void* c)              \
{                                                                                   \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "fgetattr_op");                        \
    struct x64_stat64 b_;                                                           \
    UnalignStat64(b, &b_);                                                          \
    int ret = (int)RunFunctionFmt(my_fgetattr_op_fct_##A, "ppp", a, &b_, c);   \
    AlignStat64(&b_, b);                                                            \
    return ret;                                                                     \
}
SUPER()
#undef GO
static void* find_fgetattr_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_fgetattr_op_fct_##A == (uintptr_t)fct) return my_fgetattr_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_fgetattr_op_fct_##A == 0) {my_fgetattr_op_fct_##A = (uintptr_t)fct; return my_fgetattr_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse fgetattr_op callback\n");
    return NULL;
}

// lock_op
#define GO(A)   \
static uintptr_t my_lock_op_fct_##A = 0;                                        \
static int my_lock_op_##A(const char * a, void* b, int c, void* d)              \
{                                                                               \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "lock_op");                        \
    return (int)RunFunctionFmt(my_lock_op_fct_##A, "ppip", a, b, c, d);     \
}
SUPER()
#undef GO
static void* find_lock_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_lock_op_fct_##A == (uintptr_t)fct) return my_lock_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_lock_op_fct_##A == 0) {my_lock_op_fct_##A = (uintptr_t)fct; return my_lock_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse lock_op callback\n");
    return NULL;
}

// utimens_op
#define GO(A)   \
static uintptr_t my_utimens_op_fct_##A = 0;                                 \
static int my_utimens_op_##A(const char * a, const struct timespec b[2])    \
{                                                                           \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "utimens_op");                 \
    return (int)RunFunctionFmt(my_utimens_op_fct_##A, "pp", a, b);    \
}
SUPER()
#undef GO
static void* find_utimens_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_utimens_op_fct_##A == (uintptr_t)fct) return my_utimens_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_utimens_op_fct_##A == 0) {my_utimens_op_fct_##A = (uintptr_t)fct; return my_utimens_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse utimens_op callback\n");
    return NULL;
}

// bmap_op
#define GO(A)   \
static uintptr_t my_bmap_op_fct_##A = 0;                                    \
static int my_bmap_op_##A(const char * a, size_t b, uint64_t *c)            \
{                                                                           \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "bmap_op");                    \
    return (int)RunFunctionFmt(my_bmap_op_fct_##A, "pLp", a, b, c);    \
}
SUPER()
#undef GO
static void* find_bmap_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_bmap_op_fct_##A == (uintptr_t)fct) return my_bmap_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_bmap_op_fct_##A == 0) {my_bmap_op_fct_##A = (uintptr_t)fct; return my_bmap_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse bmap_op callback\n");
    return NULL;
}

// ioctl_op
#define GO(A)   \
static uintptr_t my_ioctl_op_fct_##A = 0;                                                       \
static int my_ioctl_op_##A(const char * a, int b, void* c, void* d, unsigned int e, void* f)    \
{                                                                                               \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "ioctl_op");                                       \
    return (int)RunFunctionFmt(my_ioctl_op_fct_##A, "pippup", a, b, c, d, e, f);              \
}
SUPER()
#undef GO
static void* find_ioctl_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_ioctl_op_fct_##A == (uintptr_t)fct) return my_ioctl_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ioctl_op_fct_##A == 0) {my_ioctl_op_fct_##A = (uintptr_t)fct; return my_ioctl_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse ioctl_op callback\n");
    return NULL;
}

// poll_op
#define GO(A)   \
static uintptr_t my_poll_op_fct_##A = 0;                                    \
static int my_poll_op_##A(const char * a, void* b, void* c, unsigned * d)   \
{                                                                           \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "poll_op");                    \
    return (int)RunFunctionFmt(my_poll_op_fct_##A, "pppp", a, b, c, d); \
}
SUPER()
#undef GO
static void* find_poll_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_poll_op_fct_##A == (uintptr_t)fct) return my_poll_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_poll_op_fct_##A == 0) {my_poll_op_fct_##A = (uintptr_t)fct; return my_poll_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse poll_op callback\n");
    return NULL;
}

// write_buf_op
#define GO(A)   \
static uintptr_t my_write_buf_op_fct_##A = 0;                                       \
static int my_write_buf_op_##A(const char * a, void* b, off_t c, void* d)           \
{                                                                                   \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "write_buf_op");                       \
    return (int)RunFunctionFmt(my_write_buf_op_fct_##A, "pplp", a, b, c, d);    \
}
SUPER()
#undef GO
static void* find_write_buf_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_write_buf_op_fct_##A == (uintptr_t)fct) return my_write_buf_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_write_buf_op_fct_##A == 0) {my_write_buf_op_fct_##A = (uintptr_t)fct; return my_write_buf_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse write_buf_op callback\n");
    return NULL;
}

// read_buf_op
#define GO(A)   \
static uintptr_t my_read_buf_op_fct_##A = 0;                                        \
static int my_read_buf_op_##A(const char * a, void* b, size_t c, off_t d, void* e)  \
{                                                                                   \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "read_buf_op");                        \
    return (int)RunFunctionFmt(my_read_buf_op_fct_##A, "ppLlp", a, b, c, d, e);  \
}
SUPER()
#undef GO
static void* find_read_buf_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_read_buf_op_fct_##A == (uintptr_t)fct) return my_read_buf_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_read_buf_op_fct_##A == 0) {my_read_buf_op_fct_##A = (uintptr_t)fct; return my_read_buf_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse read_buf_op callback\n");
    return NULL;
}

// flock_op
#define GO(A)   \
static uintptr_t my_flock_op_fct_##A = 0;                                   \
static int my_flock_op_##A(const char * a, void* b, int c)                  \
{                                                                           \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "flock_op");                   \
    return (int)RunFunctionFmt(my_flock_op_fct_##A, "ppi", a, b, c);   \
}
SUPER()
#undef GO
static void* find_flock_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_flock_op_fct_##A == (uintptr_t)fct) return my_flock_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_flock_op_fct_##A == 0) {my_flock_op_fct_##A = (uintptr_t)fct; return my_flock_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse flock_op callback\n");
    return NULL;
}

// fallocate_op
#define GO(A)   \
static uintptr_t my_fallocate_op_fct_##A = 0;                                       \
static int my_fallocate_op_##A(const char * a, int b, off_t c, off_t d, void* e)    \
{                                                                                   \
    printf_log(LOG_DEBUG, "fuse: call %s\n", "fallocate_op");                       \
    return (int)RunFunctionFmt(my_fallocate_op_fct_##A, "pillp", a, b, c, d, e); \
}
SUPER()
#undef GO
static void* find_fallocate_op_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_fallocate_op_fct_##A == (uintptr_t)fct) return my_fallocate_op_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_fallocate_op_fct_##A == 0) {my_fallocate_op_fct_##A = (uintptr_t)fct; return my_fallocate_op_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Fuse fallocate_op callback\n");
    return NULL;
}

#undef SUPER

EXPORT int my_fuse_opt_parse(x64emu_t* emu, void* args, void* data, void* opts, void* f)
{
    return my->fuse_opt_parse(args, data, opts, findfuse_opt_procFct(f));
}

EXPORT void* my_fuse_lowlevel_new(x64emu_t* emu, void* args, fuse_lowlevel_ops_t *o, size_t sz, void* data)
{
    static fuse_lowlevel_ops_t o_ = {0};
    size_t cvt = 0;
    #define GO(A) if(cvt<sz) {o_.A = find_##A##_Fct(o->A); cvt+=sizeof(void*); if(o_.A) printf_log(LOG_DEBUG, "fuse: %s is present\n", #A);}
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

EXPORT int my_fuse_main_real(x64emu_t* emu, int argc, void* argv, const fuse_operations_t* op, size_t op_size, void* data)
{
    static fuse_operations_t o_ = {0};
box64_log=2;
box64_showsegv=1;
    size_t cvt = 0;
    #define GO(A) if(cvt<op_size) {o_.A = find_##A##_Fct(op->A); cvt+=sizeof(void*); if(o_.A) printf_log(LOG_DEBUG, "fuse: %s is present\n", #A);}
    // size is aligned in GOS
    #define GOS(A) if(cvt<op_size) {o_.A = op->A; cvt+=sizeof(void*);}
    GO(getattr_op)
    GO(readlink_op)
    GO(getdir_op)
    GO(mknod_op)
    GO(mkdir_op)
    GO(unlink_op)
    GO(rmdir_op)
    GO(symlink_op)
    GO(rename_op)
    GO(link_op)
    GO(chmod_op)
    GO(chown_op)
    GO(truncate_op)
    GO(utime_op)
    GO(open_op)
    GO(read_op)
    GO(write_op)
    GO(statfs_op)
    GO(flush_op)
    GO(release_op)
    GO(fsync_op)
    GO(setxattr_op)
    GO(getxattr_op)
    GO(listxattr_op)
    GO(removexattr_op)
    GO(opendir_op)
    GO(readdir_op)
    GO(releasedir_op)
    GO(fsyncdir_op)
    GO(init_op)
    GO(destroy_op)
    GO(access_op)
    GO(create_op)
    GO(ftruncate_op)
    GO(fgetattr_op)
    GO(lock_op)
    GO(utimens_op)
    GO(bmap_op)
    GOS(flags)
    GO(ioctl_op)
    GO(poll_op)
    GO(write_buf_op)
    GO(read_buf_op)
    GO(flock_op)
    GO(fallocate_op)
    #undef GO
    #undef GOS
    printf_log(LOG_DEBUG, "fuse_main_real called for %s with size_ops=%zd vs %zd\n", my_context->fullpath, op_size, cvt);
    int ret = my->fuse_main_real(argc, argv, &o_, cvt, data);
    printf_log(LOG_DEBUG, "fuse_main_real returned %d\n", ret);
    return ret;
}

#include "wrappedlib_init.h"
