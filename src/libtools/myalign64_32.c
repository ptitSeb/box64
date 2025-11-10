#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <asm/stat.h>
#include <sys/vfs.h>
#include <sys/statvfs.h>
#include <dirent.h>

#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "myalign32.h"
#include "box32.h"

void UnalignStat64_32(const void* source, void* dest)
{
    struct i386_stat64 *i386st = (struct i386_stat64*)dest;
    struct stat *st = (struct stat*) source;
    
    memset(i386st->__pad0, 0, sizeof(i386st->__pad0));
	memset(i386st->__pad3, 0, sizeof(i386st->__pad3));
    i386st->st_dev      = st->st_dev;
    i386st->__st_ino    = st->st_ino;
    i386st->st_mode     = st->st_mode;
    i386st->st_nlink    = st->st_nlink;
    i386st->st_uid      = st->st_uid;
    i386st->st_gid      = st->st_gid;
    i386st->st_rdev     = st->st_rdev;
    i386st->st_size     = st->st_size;
    i386st->st_blksize  = st->st_blksize;
    i386st->st_blocks   = st->st_blocks;
    i386st->st_atime    = st->st_atime;
    i386st->st_atime_nsec   = st->st_atime_nsec;
    i386st->st_mtime    = st->st_mtime;
    i386st->st_mtime_nsec   = st->st_mtime_nsec;
    i386st->st_ctime    = st->st_ctime;
    i386st->st_ctime_nsec   = st->st_ctime_nsec;
    i386st->st_ino      = st->st_ino;
}

void UnalignStat64_32_t64(const void* source, void* dest)
{
    struct i386_stat64_t64 *i386st = (struct i386_stat64_t64*)dest;
    struct stat *st = (struct stat*) source;
    
    i386st->st_dev      = st->st_dev;
    i386st->st_ino      = st->st_ino;
    i386st->st_mode     = st->st_mode;
    i386st->st_nlink    = st->st_nlink;
    i386st->st_uid      = st->st_uid;
    i386st->st_gid      = st->st_gid;
    i386st->st_rdev     = st->st_rdev;
    i386st->st_size     = st->st_size;
    i386st->st_blksize  = st->st_blksize;
    i386st->st_blocks   = st->st_blocks;
    i386st->st_atime    = st->st_atime;
    i386st->st_atime_nsec   = st->st_atime_nsec;
    i386st->st_mtime    = st->st_mtime;
    i386st->st_mtime_nsec   = st->st_mtime_nsec;
    i386st->st_ctime    = st->st_ctime;
    i386st->st_ctime_nsec   = st->st_ctime_nsec;
}

struct native_fsid {
  int     val[2];
};

struct native_statfs64 {
  long        f_type;
  long        f_bsize;
  uint64_t    f_blocks;
  uint64_t    f_bfree;
  uint64_t    f_bavail;
  uint64_t    f_files;
  uint64_t    f_ffree;
  struct native_fsid f_fsid;
  long        f_namelen;
  long        f_frsize;
  long        f_flags;
  long        f_spare[4];
};  // f_flags is not always defined, but then f_spare is [5] in that case

void UnalignStatFS64_32(const void* source, void* dest)
{
    struct i386_statfs64 *i386st = (struct i386_statfs64*)dest;
    struct native_statfs64 *st = (struct native_statfs64*) source;

    i386st->f_type      = to_long(st->f_type);
    i386st->f_bsize     = to_long(st->f_bsize);
    i386st->f_blocks    = st->f_blocks;
    i386st->f_bfree     = st->f_bfree;
    i386st->f_bavail    = st->f_bavail;
    i386st->f_files     = st->f_files;
    i386st->f_ffree     = st->f_ffree;
    memcpy(&i386st->f_fsid, &st->f_fsid, sizeof(i386st->f_fsid));
    i386st->f_namelen   = to_long(st->f_namelen);
    i386st->f_frsize    = to_long(st->f_frsize);
    i386st->f_flags     = to_long(st->f_flags);
    i386st->f_spare[0]  = to_long(st->f_spare[0]);
    i386st->f_spare[1]  = to_long(st->f_spare[1]);
    i386st->f_spare[2]  = to_long(st->f_spare[2]);
    i386st->f_spare[3]  = to_long(st->f_spare[3]);
}

void UnalignStatFS_32(const void* source, void* dest)
{
    struct i386_statfs *i386st = (struct i386_statfs*)dest;
    struct native_statfs64 *st = (struct native_statfs64*) source;

    i386st->f_type      = st->f_type;
    i386st->f_bsize     = st->f_bsize;
    i386st->f_blocks    = st->f_blocks;
    i386st->f_bfree     = st->f_bfree;
    i386st->f_bavail    = st->f_bavail;
    i386st->f_files     = st->f_files;
    i386st->f_ffree     = st->f_ffree;
    memcpy(&i386st->f_fsid, &st->f_fsid, sizeof(i386st->f_fsid));
    i386st->f_namelen   = st->f_namelen;
    i386st->f_frsize    = st->f_frsize;
    i386st->f_flags     = st->f_flags;
    i386st->f_spare[0]  = st->f_spare[0];
    i386st->f_spare[1]  = st->f_spare[1];
    i386st->f_spare[2]  = st->f_spare[2];
    i386st->f_spare[3]  = st->f_spare[3];
}

struct native_statvfs {
    unsigned long int f_bsize;
    unsigned long int f_frsize;
#ifndef ANDROID
    __fsblkcnt64_t f_blocks;
    __fsblkcnt64_t f_bfree;
    __fsblkcnt64_t f_bavail;
    __fsfilcnt64_t f_files;
    __fsfilcnt64_t f_ffree;
    __fsfilcnt64_t f_favail;
#else
    fsblkcnt_t f_blocks;
    fsblkcnt_t f_bfree;
    fsblkcnt_t f_bavail;
    fsblkcnt_t f_files;
    fsblkcnt_t f_ffree;
    fsblkcnt_t f_favail;
#endif
    unsigned long int f_fsid;
#ifdef _STATVFSBUF_F_UNUSED
    int __f_unused;
#endif
    unsigned long int f_flag;
    unsigned long int f_namemax;
    unsigned int f_type;
    int __f_spare[5];
};

void UnalignStatVFS64_32(const void* source, void* dest)
{
    struct i386_statvfs64 *i386st = (struct i386_statvfs64*)dest;
    struct native_statvfs *st = (struct native_statvfs*) source;

    i386st->f_bsize     = st->f_bsize;
    i386st->f_frsize    = st->f_frsize;
    i386st->f_blocks    = st->f_blocks;
    i386st->f_bfree     = st->f_bfree;
    i386st->f_bavail    = st->f_bavail;
    i386st->f_files     = st->f_files;
    i386st->f_ffree     = st->f_ffree;
    i386st->f_favail    = st->f_favail;
    i386st->f_fsid      = st->f_fsid;
    i386st->f_flag      = st->f_flag;
    i386st->f_namemax   = st->f_namemax;
    i386st->f_type      = st->f_type;
}

void UnalignStatVFS_32(const void* source, void* dest)
{
    struct i386_statvfs *i386st = (struct i386_statvfs*)dest;
    struct native_statvfs *st = (struct native_statvfs*) source;

    i386st->f_bsize     = st->f_bsize;
    i386st->f_frsize    = st->f_frsize;
    i386st->f_blocks    = st->f_blocks;
    i386st->f_bfree     = st->f_bfree;
    i386st->f_bavail    = st->f_bavail;
    i386st->f_files     = st->f_files;
    i386st->f_ffree     = st->f_ffree;
    i386st->f_favail    = st->f_favail;
    i386st->f_fsid      = st->f_fsid;
    i386st->f_flag      = st->f_flag;
    i386st->f_namemax   = st->f_namemax;
    i386st->f_type      = st->f_type;
}

void UnalignDirent_32(const void* source, void* dest)
{
    struct i386_dirent* i386d = (struct i386_dirent*)dest;
    struct dirent* d = (struct dirent*)source;

    i386d->d_ino = d->d_ino ^ (d->d_ino >> 32);
    i386d->d_off = d->d_off;
    i386d->d_reclen = d->d_reclen - 8;
    i386d->d_type = d->d_type;
    memcpy(i386d->d_name, d->d_name, d->d_reclen - (8+2+1));
}

#define TRANSFERT   \
GO(l_type)          \
GO(l_whence)        \
GO(l_start)         \
GO(l_len)           \
GO(l_pid)

// Arm -> x86
void UnalignFlock64_32(void* dest, void* source)
{
    #define GO(A) ((x86_flock64_t*)dest)->A = ((my_flock64_t*)source)->A;
    TRANSFERT
    #undef GO
}

// x86 -> Arm
void AlignFlock64_32(void* dest, void* source)
{
    #define GO(A) ((my_flock64_t*)dest)->A = ((x86_flock64_t*)source)->A;
    TRANSFERT
    #undef GO
}
#undef TRANSFERT
