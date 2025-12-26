#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <sys/signalfd.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <asm/stat.h>
#include <errno.h>
#include <sched.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/resource.h>
#include <poll.h>
#include <sys/epoll.h>

#include "x64_signals.h"
#include "os.h"
#include "debug.h"
#include "box64stack.h"
#include "x64emu.h"
#include "box64cpu.h"
#include "x64emu_private.h"
#include "x64run_private.h"
//#include "x64primop.h"
#include "x64trace.h"
//#include "myalign.h"
#include "box64context.h"
#include "callback.h"
#include "signals.h"
#include "emit_signals.h"
#include "x64tls.h"
#include "elfloader.h"
#include "x64int_private.h"

typedef struct x64_sigaction_s x64_sigaction_t;
typedef struct x64_stack_s x64_stack_t;

extern int mkdir(const char *path, mode_t mode);
extern int mknod(const char *path, mode_t mode, dev_t dev);
extern int fchmodat (int __fd, const char *__file, mode_t __mode, int __flag);

//int32_t my_getrandom(x64emu_t* emu, void* buf, uint32_t buflen, uint32_t flags);
int of_convert(int flag);
int32_t my_open(x64emu_t* emu, void* pathname, int32_t flags, uint32_t mode);
ssize_t my_readlink(x64emu_t* emu, void* path, void* buf, size_t sz);
int my_readlinkat(x64emu_t* emu, int fd, void* path, void* buf, size_t bufsize);
int my_stat(x64emu_t *emu, void* filename, void* buf);
int my_lstat(x64emu_t *emu, void* filename, void* buf);
int my_fstat(x64emu_t *emu, int fd, void* buf);
int my_fstatat(x64emu_t *emu, int fd, const char* path, void* buf, int flags);

int my_sigaction(x64emu_t* emu, int signum, const x64_sigaction_t *act, x64_sigaction_t *oldact);
int my_sigaltstack(x64emu_t* emu, const x64_stack_t* ss, x64_stack_t* oss);
//int32_t my_execve(x64emu_t* emu, const char* path, char* const argv[], char* const envp[]);
void* my_mmap64(x64emu_t* emu, void *addr, unsigned long length, int prot, int flags, int fd, int64_t offset);
int my_munmap(x64emu_t* emu, void* addr, unsigned long length);
int my_mprotect(x64emu_t* emu, void *addr, unsigned long len, int prot);
void* my_mremap(x64emu_t* emu, void* old_addr, size_t old_size, size_t new_size, int flags, void* new_addr);
#ifndef NOALIGN
int32_t my_epoll_ctl(x64emu_t* emu, int32_t epfd, int32_t op, int32_t fd, void* event);
int32_t my_epoll_wait(x64emu_t* emu, int32_t epfd, void* events, int32_t maxevents, int32_t timeout);
int32_t my_epoll_pwait(x64emu_t* emu, int32_t epfd, void* events, int32_t maxevents, int32_t timeout, const sigset_t *sigmask);
int32_t my_epoll_pwait2(x64emu_t* emu, int epfd, void* events, int maxevents, struct timespec *timeout, sigset_t * sigmask);
#endif
pid_t my_vfork(x64emu_t* emu);
int32_t my_fcntl(x64emu_t* emu, int32_t a, int32_t b, void* c);
int32_t my_execve(x64emu_t* emu, const char* path, char* const argv[], char* const envp[]);

// cannot include <fcntl.h>, it conflict with some asm includes...
#ifndef O_NONBLOCK
#define O_NONBLOCK 04000
#endif
#undef fcntl
int fcntl(int fd, int cmd, ... /* arg */ );

// Syscall table for x86_64 can be found
typedef struct scwrap_s {
    int nats;
    int nbpars;
} scwrap_t;

static const scwrap_t syscallwrap[] = {
    //{ 0, __NR_read, 3 },      // wrapped so SA_RESTART can be handled by libc
    //{ 1, __NR_write, 3 },     // same
    //{ 2, __NR_open, 3 },      // flags need transformation
    //{ 3, __NR_close, 1 },     // wrapped so SA_RESTART can be handled by libc
    //{ 4, __NR_stat, 2 },     // Need to align struct stat
    //{ 5, __NR_fstat, 2},
    //{ 6, __NR_lstat, 2},
    #ifdef __NR_poll
    [7] = {__NR_poll, 3},
    #endif
    [8] = {__NR_lseek, 3},
    //[9] = {__NR_mmap, 6},       // wrapped to track mmap
    //[10] = {__NR_mprotect, 3},  // same
    //[11] = {__NR_munmap, 2},    // same
    [12] = {__NR_brk, 1},
    //[13] = {__NR_rt_sigaction, 4},   // wrapped to use my_ version
    [14] = {__NR_rt_sigprocmask, 4},
    [16] = {__NR_ioctl, 3},
    [17] = {__NR_pread64, 4},
    [18] = {__NR_pwrite64, 4},
    [19] = {__NR_readv, 3},
    [20] = {__NR_writev, 3},
    #ifdef __NR_access
    [21] = {__NR_access, 2},
    #endif
    #ifdef __NR_pipe
    [22] = {__NR_pipe, 1},
    #endif
    [24] = {__NR_sched_yield, 0},
    #ifdef __NR_select
    [23] = {__NR_select, 5},
    #endif
    //[25] = {__NR_mremap, 5},    // wrapped to track protection
    [27] = {__NR_mincore, 3},
    [28] = {__NR_madvise, 3},
    #ifdef __NR_dup2
    [33] = {__NR_dup2, 2},
    #endif
    [35] = {__NR_nanosleep, 2},
    [39] = {__NR_getpid, 0},
    [41] = {__NR_socket, 3},
    [42] = {__NR_connect, 3},
    [43] = {__NR_accept, 3},
    [44] = {__NR_sendto, 6},
    [45] = {__NR_recvfrom, 6},
    [46] = {__NR_sendmsg, 3},
    [47] = {__NR_recvmsg, 3},
    [49] = {__NR_bind, 3},
    [50] = {__NR_listen, 2},
    [51] = {__NR_getsockname, 3},
    [52] = {__NR_getpeername, 3},
    [53] = {__NR_socketpair, 4},
    [54] = {__NR_setsockopt, 5},
    [55] = {__NR_getsockopt, 5},
    //[56] = {__NR_clone, 5},
    #ifdef __NR_fork
    [57] = {__NR_fork, 0 },    // should wrap this one, because of the struct pt_regs (the only arg)?
    #endif
    //[58] = {__NR_vfork, 0},
    //[59] = {__NR_execve, 3},
    [60] = {__NR_exit, 1},    // Needs wrapping?
    [61] = {__NR_wait4, 4},
    [62] = {__NR_kill, 2 },
    //[63] = {__NR_uname, 1}, // Needs wrapping, use old_utsname
    [66] = {__NR_semctl, 4},
    //[72] = {__NR_fnctl, 3}, // Needs wrapping, and not always defined anyway
    [73] = {__NR_flock, 2},
    [74] = {__NR_fsync, 1},
    [77] = {__NR_ftruncate, 2},
    #ifdef __NR_getdents
    [78] = {__NR_getdents, 3},
    #endif
    [79] = {__NR_getcwd, 2},
    [80] = {__NR_chdir, 1},
    #ifdef __NR_rename
    [82] = {__NR_rename, 2},
    #endif
    #ifdef __NR_mkdir
    [83] = {__NR_mkdir, 2},
    #endif
    #ifdef __NR_unlink
    [87] = {__NR_unlink, 1},
    #endif
    //[89] = {__NR_readlink, 3},  // not always existing, better use the wrapped version anyway
    [96] = {__NR_gettimeofday, 2},
    #ifdef __NR_getrlimit
    [97] = {__NR_getrlimit, 2},
    #endif
    [101] = {__NR_ptrace, 4},
    [102] = {__NR_getuid, 0},
    [104] = {__NR_getgid, 0},
    [105] = {__NR_setuid, 1},
    [106] = {__NR_setgid, 1},
    [107] = {__NR_geteuid, 0},
    [108] = {__NR_getegid, 0},
    [109] = {__NR_setpgid, 2},
    [110] = {__NR_getppid, 0},
    //[111] = {__NR_getpgrp, 0},
    [112] = {__NR_setsid, 0},
    [113] = {__NR_setreuid, 2},
    [114] = {__NR_setregid, 2},
    [118] = {__NR_getresuid, 3},
    [120] = {__NR_getresgid, 3},
    [121] = {__NR_getpgid, 1},
    [122] = {__NR_setfsuid, 1},
    [123] = {__NR_setfsgid, 1},
    [124] = {__NR_getsid, 1},
    [125] = {__NR_capget, 2},
    [126] = {__NR_capset, 2},
    [127] = {__NR_rt_sigpending, 2},
    [128] = {__NR_rt_sigtimedwait, 4},
    //[131] = {__NR_sigaltstack, 2},  // wrapped to use my_sigaltstack*
    #ifdef __NR_mknod
    [133] = {__NR_mknod, 3},
    #endif
    [140] = {__NR_getpriority, 2},
    [145] = {__NR_sched_getscheduler, 1},
    [148] = {__NR_sched_rr_get_interval, 2},
    [149] = {__NR_mlock, 2},
    [150] = {__NR_munlock, 2},
    [155] = {__NR_pivot_root, 2},
    [157] = {__NR_prctl, 5 },     // needs wrapping?
    //[158] = {__NR_arch_prctl, 2},   //need wrapping
    #ifdef __NR_setrlimit
    [160] = {__NR_setrlimit, 2},
    #endif
    [161] = {__NR_chroot, 1},
    [165] = {__NR_mount, 5},
    [186] = {__NR_gettid, 0 },    //0xBA
    [194] = {__NR_listxattr, 3},
    [195] = {__NR_llistxattr, 3},
    [196] = {__NR_flistxattr, 3},
    [200] = {__NR_tkill, 2 },
    #ifdef __NR_time
    [201] = {__NR_time, 1},
    #endif
    [202] = {__NR_futex, 6},
    [203] = {__NR_sched_setaffinity, 3},
    [204] = {__NR_sched_getaffinity, 3},
    [206] = {__NR_io_setup, 2},
    [207] = {__NR_io_destroy, 1},
    [208] = {__NR_io_getevents, 4},
    [209] = {__NR_io_submit, 3},
    [210] = {__NR_io_cancel, 3},
    [212] = {__NR_lookup_dcookie, 3},
    #ifdef __NR_epoll_create
    [213] = {__NR_epoll_create, 1},
    #endif
    [217] = {__NR_getdents64, 3},
    [218] = {__NR_set_tid_address, 1},
    [220] = {__NR_semtimedop, 4},
    [228] = {__NR_clock_gettime, 2},
    [229] = {__NR_clock_getres, 2},
    [230] = {__NR_clock_nanosleep, 4},
    [231] = {__NR_exit_group, 1},
    #if defined(__NR_epoll_wait) && defined(NOALIGN)
    [232] = {__NR_epoll_wait, 4},
    #endif
    #if defined(__NR_epoll_ctl) && defined(NOALIGN)
    [233] = {__NR_epoll_ctl, 4},
    #endif
    [234] = {__NR_tgkill, 3},
    [237] = {__NR_mbind, 6},
    [238] = {__NR_set_mempolicy, 3},
    [239] = {__NR_get_mempolicy, 5},
    [247] = {__NR_waitid, 5},
    #ifdef __NR_inotify_init
    [253] = {__NR_inotify_init, 0},   //0xFD
    #endif
    [254] = {__NR_inotify_add_watch, 3},
    [255] = {__NR_inotify_rm_watch, 2},
    #ifdef NOALIGN
    [257] = {__NR_openat, 4},
    #endif
    [258] = {__NR_mkdirat, 3},
    [260] = {__NR_fchownat, 5},
    //[262] = {__NR_fstatat, 4},
    [263] = {__NR_unlinkat, 3},
    #ifdef __NR_renameat
    [264] = {__NR_renameat, 4},
    #endif
    [266] = {__NR_symlinkat, 3},
    //[267] = {__NR_readlinkat, 4},
    [268] = {__NR_fchmodat, 3},
    [270] = {__NR_pselect6, 6},
    [272] = {__NR_unshare, 1},
    [273] = {__NR_set_robust_list, 2},
    [274] = {__NR_get_robust_list, 3},
    [280] = {__NR_utimensat, 4},
    #ifdef NOALIGN
    [281] = {__NR_epoll_pwait, 5},
    #endif
    //[282] = {__NR__signalfd, 3},
    #ifdef _NR_eventfd
    [284] = {__NR_eventfd, 1},
    #endif
    [285] = {__NR_fallocate, 4},
    [288] = {__NR_accept4, 4},
    [289] = {__NR_signalfd4, 4},    // this one might need some wrapping
    [290] = {__NR_eventfd2, 2},
    #ifdef NOALIGN
    [291] = {__NR_epoll_create1, 1},
    #endif
    [292] = {__NR_dup3, 3},
    [293] = {__NR_pipe2, 2},
    [294] = {__NR_inotify_init1, 1},
    [297] = {__NR_rt_tgsigqueueinfo, 4},
    [298] = {__NR_perf_event_open, 5},
    [302] = {__NR_prlimit64, 4},
    [309] = {__NR_getcpu, 3}, // need wrapping?
    [314] = {__NR_sched_setattr, 3},
    [315] = {__NR_sched_getattr, 4},
    [316] = {__NR_renameat2, 5},
    //[317] = {__NR_seccomp, 3},
    [318] = {__NR_getrandom, 3},
    [319] = {__NR_memfd_create, 2},
    //[323] = {__NR_userfaultfd, 1}, //disable for now
    [324] = {__NR_membarrier, 2},
    #ifdef __NR_copy_file_range
    // TODO: call back if unavailable?
    [326] = {__NR_copy_file_range, 6},
    #endif
    #ifdef __NR_statx
    // TODO: implement fallback if __NR_statx is not defined
    [332] = {__NR_statx, 5},
    #endif
    #ifdef __NR_io_uring_setup
    [425] = {__NR_io_uring_setup, 2},
    #endif
    #ifdef __NR_io_uring_enter
    [426] = {__NR_io_uring_enter, 6},
    #endif
    #ifdef __NR_io_uring_register
    [427] = {__NR_io_uring_register, 4},
    #endif
    #ifdef __NR_fchmodat4
    [434] = {__NR_fchmodat4, 4},
    #endif
    #ifdef __NR_faccessat2
    [439] = {__NR_faccessat2, 4},
    #endif
    #if defined(__NR_epoll_pwait2) && defined(NOALIGN)
    [441] = {__NR_epoll_pwait2, 5},
    #endif
    #ifdef __NR_landlock_create_ruleset	
    [444] = {__NR_landlock_create_ruleset, 3},
    #endif
    #ifdef __NR_landlock_add_rule
    [445] = {__NR_landlock_add_rule, 4},
    #endif
    #ifdef __NR_landlock_restrict_self
    [446] = {__NR_landlock_restrict_self, 2},
    #endif
    //[449] = {__NR_futex_waitv, 5},
};

struct mmap_arg_struct {
    unsigned long addr;
    unsigned long len;
    unsigned long prot;
    unsigned long flags;
    unsigned long fd;
    unsigned long offset;
};

#ifndef __NR_getdents
typedef struct x86_linux_dirent_s {
    unsigned long  d_ino;
    unsigned long  d_off;
    unsigned short d_reclen;
    char           d_name[];
} x86_linux_dirent_t;

typedef struct nat_linux_dirent64_s {
    ino64_t        d_ino;
    off64_t        d_off;
    unsigned short d_reclen;
    unsigned char  d_type;
    char           d_name[];
} nat_linux_dirent64_t;

ssize_t DirentFromDirent64(void* dest, void* source, ssize_t count)
{
    nat_linux_dirent64_t *src = (nat_linux_dirent64_t*)source;
    x86_linux_dirent_t *dst = (x86_linux_dirent_t*)dest;
    x86_linux_dirent_t *old = NULL;
    ssize_t ret = 0;
    while(count>0) {
        ssize_t sz = src->d_reclen+sizeof(x86_linux_dirent_t)-sizeof(nat_linux_dirent64_t)+2;
        if(sz>=count) {
            dst->d_ino = src->d_ino;
            dst->d_reclen = sz;
            ret+=sz;
            strcpy(dst->d_name, src->d_name);
            dst->d_off = src->d_off?(src->d_off+sizeof(x86_linux_dirent_t)-sizeof(nat_linux_dirent64_t)+2):0;
            *(uint8_t*)((uintptr_t)dst + dst->d_reclen -2) = 0;
            *(uint8_t*)((uintptr_t)dst + dst->d_reclen -1) = src->d_type;

            count -= src->d_reclen;
            ret += 1;
            old = dst;
            src = (nat_linux_dirent64_t*)(((uintptr_t)src) + src->d_reclen);
            dst = (x86_linux_dirent_t*)(((uintptr_t)dst) + dst->d_reclen);
        } else {
            if(old)
                old->d_off = 0;
            count = 0;
        }
    }
    return (count<0)?count:ret;
}

#endif

typedef struct old_utsname_s {
    char sysname[65];
    char nodename[65];
    char release[65];
    char version[65];
    char machine[65];
} old_utsname_t;

//struct x86_pt_regs {
//    long ebx;
//    long ecx;
//    long edx;
//    long esi;
//    long edi;
//    long ebp;
//    long eax;
//    int  xds;
//    int  xes;
//    int  xfs;
//    int  xgs;
//    long orig_eax;
//    long eip;
//    int  xcs;
//    long eflags;
//    long esp;
//    int  xss;
//};

typedef struct clone_s {
    x64emu_t* emu;
    void* stack2free;
    void* tls;
} clone_t;

static int clone_fn_syscall(void* arg)
{
    clone_t* args = arg;
    x64emu_t *emu = args->emu;
    thread_forget_emu();
    thread_set_emu(emu);
    //TODO: do something with TLS. Refresh libc tls with that?
    R_RAX = 0;
    DynaRun(emu);
    int ret = R_EAX;
    FreeX64Emu(&emu);
    void* stack2free = args->stack2free;
    box_free(args);
    if(my_context->stack_clone_used && !stack2free)
        my_context->stack_clone_used = 0;
    if(stack2free)
        box_free(stack2free);   // this free the stack, so it will crash very soon!
    _exit(ret);
}

void EXPORT x64Syscall(x64emu_t *emu)
{
    // check if it's a wine process, then filter the syscall (simulate SECCMP)
    if(box64_wine && !box64_is32bits) {
        //64bits only here...
        uintptr_t ret_addr = R_RIP-2;
        if(/*ret_addr<0x700000000000LL &&*/ (my_context->signals[X64_SIGSYS]>2) && !FindElfAddress(my_context, ret_addr)) {
            // not a linux elf, not a syscall to setup x86_64 arch. Signal SIGSYS
            EmitSignal(emu, X64_SIGSYS, (void*)ret_addr, R_EAX&0xffff);  // what are the parameters?
            return;
        }
    }
    return x64Syscall_linux(emu);
}
void EXPORT x64Syscall_linux(x64emu_t *emu)
{
    RESET_FLAGS(emu);
    uint32_t s = R_EAX; // EAX? (syscalls only go up to 547 anyways)
    int log = 0;
    char t_buff[256] = "\0";
    char t_buffret[128] = "\0";
    char buff2[128] = "\0";
    char* buff = NULL;
    char* buffret = NULL;
    if(BOX64ENV(log) >= LOG_DEBUG || BOX64ENV(rolling_log)) {
        log = 1;
        buff = BOX64ENV(rolling_log)?(my_context->log_call+256*my_context->current_line):t_buff;
        buffret = BOX64ENV(rolling_log)?(my_context->log_ret+128*my_context->current_line):t_buffret;
        if(BOX64ENV(rolling_log))
            my_context->current_line = (my_context->current_line+1)%BOX64ENV(rolling_log);
        snprintf(buff, 255, "%04d|%p: Calling syscall 0x%02X (%d) %p %p %p %p %p %p", GetTID(), (void*)R_RIP, s, s, (void*)R_RDI, (void*)R_RSI, (void*)R_RDX, (void*)R_R10, (void*)R_R8, (void*)R_R9);
        if(!BOX64ENV(rolling_log))
            printf_log(LOG_NONE, "%s", buff);
    }
    // check wrapper first
    uint32_t cnt = sizeof(syscallwrap) / sizeof(scwrap_t);
    if(s<cnt && syscallwrap[s].nats) {
        int sc = syscallwrap[s].nats;
        switch(syscallwrap[s].nbpars) {
            case 0: S_RAX = syscall(sc); break;
            case 1: 
                if(s==80) {if(log) snprintf(buff2, 127, " [sys_chdir(\"%s\")]", (char*)R_RDI);}; 
                S_RAX = syscall(sc, R_RDI); 
                break;
            case 2: 
                if(s==33) {if(log) snprintf(buff2, 127, " [sys_access(\"%s\", %ld)]", (char*)R_RDI, R_RSI);}; 
                S_RAX = syscall(sc, R_RDI, R_RSI); 
                break;
            case 3: 
                if(s==42) {if(log) snprintf(buff2, 127, " [sys_connect(%d, %p[type=%d], %d)]", R_EDI, (void*)R_RSI, *(unsigned short*)R_RSI, R_EDX);}; 
                if(s==258) {if(log) snprintf(buff2, 127, " [sys_mkdirat(%d, %s, 0x%x]", R_EDI, (char*)R_RSI, R_EDX);}; 
                S_RAX = syscall(sc, R_RDI, R_RSI, R_RDX); 
                break;
            case 4: S_RAX = syscall(sc, R_RDI, R_RSI, R_RDX, R_R10); break;
            case 5: 
                if(s==165) {if(log) snprintf(buff2, 127, " [sys_mount(%s, %s, %s, 0x%lx, %s]", (char*)R_RDI, (char*)R_RSI, (char*)R_RDX, R_R10, R_R8?(char*)R_R8:"(nil)");}; 
                S_RAX = syscall(sc, R_RDI, R_RSI, R_RDX, R_R10, R_R8); 
                break;
            case 6: S_RAX = syscall(sc, R_RDI, R_RSI, R_RDX, R_R10, R_R8, R_R9); break;
            default:
                printf_log(LOG_NONE, "ERROR, Unimplemented syscall wrapper (%d, %d)\n", s, syscallwrap[s].nbpars);
                emu->quit = 1;
                return;
        }
        if(S_EAX==-1 && errno>0)
            S_RAX = -errno;
        if(log) snprintf(buffret, 127, "0x%x%s", R_EAX, buff2);
        if(log && !BOX64ENV(rolling_log)) printf_log(LOG_NONE, "=> %s\n", buffret);
        return;
    }
    switch (s) {
        case 0:  // sys_read
            S_RAX = read(S_EDI, (void*)R_RSI, (size_t)R_RDX);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        case 1:  // sys_write
            S_RAX = write(S_EDI, (void*)R_RSI, (size_t)R_RDX);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        case 2: // sys_open
            if (log) snprintf(buff2, 127, "[sys_open \"%s\", 0x%x]", (char*)R_RDI, of_convert(R_ESI));
            //S_RAX = open((void*)R_EDI, of_convert(R_ESI), R_EDX);
            S_RAX = my_open(emu, (void*)R_RDI, of_convert(R_ESI), R_EDX);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        case 3:  // sys_close
            S_RAX = close(S_EDI);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        case 4: // sys_stat
            S_RAX = my_stat(emu, (void*)R_RDI, (void*)R_RSI);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        case 5: // sys_fstat
            S_RAX = my_fstat(emu, S_EDI, (void*)R_RSI);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        case 6: // sys_lstat
            S_RAX = my_lstat(emu, (void*)R_RDI, (void*)R_RSI);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #ifndef __NR_poll
        case 7: // sys_poll
            S_RAX = poll((struct pollfd*)R_RDI, (nfds_t)R_RSI, S_EDX);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #endif
        case 9: // sys_mmap
            R_RAX = (uintptr_t)my_mmap64(emu, (void*)R_RDI, R_RSI, S_EDX, S_R10d, S_R8d, R_R9);
            break;
        case 10: // sys_mprotect
            S_RAX = my_mprotect(emu, (void*)R_RDI, R_RSI, S_EDX);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        case 11: // sys_munmap
            S_RAX = my_munmap(emu, (void*)R_RDI, R_RSI);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        case 13: // sys_rt_sigaction
            #if 1
            S_RAX = my_syscall_rt_sigaction(emu, S_EDI, (const x64_sigaction_restorer_t *)R_RSI, (x64_sigaction_restorer_t *)R_RDX, (size_t)R_R10);
            #else
            {
                x64_sigaction_t n ={0};
                x64_sigaction_t o = {0};
                if(R_RSI) {
                    x64_sigaction_restorer_t *p = (x64_sigaction_restorer_t*)R_RSI;
                    n._u._sa_sigaction = p->_u._sa_sigaction;
                    n.sa_flags = p->sa_flags;
                    n.sa_restorer = p->sa_restorer;
                    memcpy(&n.sa_mask, &p->sa_mask, R_R10);
                }
                R_RAX = (int64_t)(int64_t)my_sigaction(emu, S_EDI, R_RSI?&n:NULL, R_RDX?&o:NULL/*, (size_t)R_R10*/);
                if(R_RAX>=0 && R_RDX) {
                    x64_sigaction_restorer_t *p = (x64_sigaction_restorer_t*)R_RDX;
                    p->_u._sa_sigaction = o._u._sa_sigaction;
                    p->sa_flags = o.sa_flags;
                    p->sa_restorer = o.sa_restorer;
                    memcpy(&p->sa_mask, &o.sa_mask, R_R10);
                }
            }
            #endif
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #ifndef __NR_access
        case 21: // sys_access
            S_RAX = access((void*)R_RDI, R_ESI);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #endif
        #ifndef __NR_pipe
        case 22:
            S_RAX = pipe((void*)R_RDI);
            if(S_RAX==-1)
                S_RAX = -errno;
            else if(log) printf_log(LOG_INFO, "[%d, %d]", ((int*)R_RDI)[0], ((int*)R_RDI)[1]);
            break;
        #endif
        #ifndef __NR_select
        case 23: // sys_select
            S_RAX = select(R_RDI, (void*)R_RSI, (void*)R_RDX, (void*)R_R10, (void*)R_R8);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #endif
        case 25: // sys_mremap
            R_RAX = (uintptr_t)my_mremap(emu, (void*)R_RDI, R_RSI, R_RDX, R_R10d, (void*)R_R8);
            break;
        #ifndef __NR_dup2
        case 33: // sys_dup2
            S_RAX = dup2(S_EDI, S_ESI);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #endif
        case 56: // sys_clone
            // x86_64 raw syscall is long clone(unsigned long flags, void *stack, int *parent_tid, int *child_tid, unsigned long tls);
            // so flags=R_RDI, stack=R_RSI, parent_tid=R_RDX, child_tid=R_R10, tls=R_R8
            if((R_EDI&~0xff)==0x4100) {
                // this is a case of vfork...
                S_RAX = my_vfork(emu);
                if(S_RAX==-1)
                    S_RAX = -errno;
            } else {
                if(R_RSI)
                {
                    uint64_t flags = R_RDI;
                    void* stack_base = (void*)R_RSI;
                    int stack_size = 0;
                    uintptr_t sp = R_RSI;
                    x64emu_t * newemu = NewX64Emu(emu->context, R_RIP, (uintptr_t)stack_base, stack_size, (R_RSI)?0:1);
                    SetupX64Emu(newemu, emu);
                    CloneEmu(newemu, emu);
                    clone_t* args = box_calloc(1, sizeof(clone_t));
                    newemu->regs[_SP].q[0] = sp;  // setup new stack pointer
                    args->emu = newemu;
                    if(flags&CLONE_SETTLS) args->tls = (void*)R_R9;
                    void* mystack = NULL;
                    if(my_context->stack_clone_used) {
                        args->stack2free = mystack = box_malloc(1024*1024);  // stack for own process...
                    } else {
                        if(!my_context->stack_clone)
                            my_context->stack_clone = box_malloc(1024*1024);
                        mystack = my_context->stack_clone;
                        my_context->stack_clone_used = 1;
                    }
                    flags&=~CLONE_SETTLS;   // to be handled differently
                    int64_t ret = clone(clone_fn_syscall, (void*)((uintptr_t)mystack+1024*1024), flags, args, R_RDX, R_R8, R_R10);
                    S_RAX = ret;
                }
                else
                    #ifdef NOALIGN
                    S_RAX = syscall(__NR_clone, R_RDI, R_RSI, R_RDX, R_R10, R_R8);
                    #else
                    S_RAX = syscall(__NR_clone, R_RDI, R_RSI, R_RDX, R_R8, R_R10);    // invert R_R8/R_R10 on Aarch64 and most other
                    #endif
            }
            break;
        #ifndef __NR_fork
        case 57:
            S_RAX = fork();
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #endif
        case 58:   // vfork
            S_RAX = my_vfork(emu);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        case 59:   // execve
            S_RAX = my_execve(emu, (const char*)R_RDI, (char* const*)R_RSI, (char* const*)R_RDX);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        case 63:    //uname
            {
                old_utsname_t *old = (old_utsname_t*)R_RDI;
                struct utsname uts;
                S_RAX = uname(&uts);
                if(S_RAX==-1)
                    S_RAX = -errno;
                memcpy(old, &uts, sizeof(*old)); // old_uts is just missing a field from new_uts
                strcpy(old->machine, "x86_64");
            }
            break;
        case 72:    //fcntl
            S_RAX = my_fcntl(emu, S_EDI, S_ESI, (void*)R_RDX);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #ifndef __NR_getdents
        case 78:
            {
                size_t count = R_RDX;
                nat_linux_dirent64_t *d64 = (nat_linux_dirent64_t*)alloca(count);
                ssize_t ret = syscall(__NR_getdents64, R_EDI, d64, count);
                ret = DirentFromDirent64((void*)R_RSI, d64, ret);
                R_RAX = (uint64_t)ret;
                if(ret==-1)
                    R_RAX = (uint64_t)-errno;
            }
            break;
        #endif
        #ifndef __NR_rename
        case 82: // sys_rename
            S_RAX = rename((void*)R_RDI, (void*)R_RSI);
            if(S_RAX==-1)
                S_RAX = -errno;
        break;
        #endif
        #ifndef __NR_mkdir
        case 83: // sys_mkdir
            S_RAX = mkdir((void*)R_RDI, R_ESI);
            if(S_RAX==-1)
                S_RAX = -errno;
        break;
        #endif
        #ifndef __NR_unlink
        case 87: //sys_unlink
            S_RAX = unlink((void*)R_RDI);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #endif
        case 89: // sys_readlink
            S_RAX = my_readlink(emu,(void*)R_RDI, (void*)R_RSI, (size_t)R_RDX);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #ifndef __NR_getrlimit
        case 97:
            S_RAX = getrlimit(S_EDI, (void*)R_RSI);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #endif
        case 111: // sys_getpgrp
            S_RAX = getpgrp();
            break;
        case 131: // sys_sigaltstack
            S_RAX = my_sigaltstack(emu, (void*)R_RDI, (void*)R_RSI);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #ifndef __NR_mknod
        case 133: // sys_mknod
            S_RAX = mknod((void*)R_RDI, R_ESI, R_RDX);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #endif
        case 158: // sys_arch_prctl
            S_RAX = my_arch_prctl(emu, S_EDI, (void*)R_RSI);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #ifndef __NR_setrlimit
        case 160:
            S_RAX = setrlimit(S_EDI, (void*)R_RSI);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #endif
        case 175: // sys_init_module
            // huh?
            S_RAX = -EPERM;
            break;
        #ifndef __NR_time
        case 201: // sys_time
            R_RAX = (uintptr_t)time((void*)R_RDI);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #endif
        #if !defined(__NR_epoll_wait) || !defined(NOALIGN)
        case 232:
            R_RAX = my_epoll_wait(emu, S_EDI, (void*)R_RSI, S_EDX, S_R10d);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #endif
        #if !defined(__NR_epoll_ctl) || !defined(NOALIGN)
        case 233:
            S_RAX = my_epoll_ctl(emu, S_EDI, S_ESI, S_EDX, (void*)R_R10);
            if(S_RAX==-1) {
                S_RAX = -errno;
                if(log) snprintf(buff2, 127, "[err=%d/%s]", errno, strerror(errno));
            }
            break;
        #endif
        #ifndef __NR_inotify_init
        case 253:
            S_RAX = syscall(__NR_inotify_init1, 0);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #endif
        #ifndef NOALIGN
        case 257:
            if (log) snprintf(buff2, 127, "[sys_openat %d, \"%s\", 0x%x]", S_EDI, (char*)R_RSI, of_convert(R_EDX));
            S_RAX = syscall(__NR_openat, S_EDI, (void*)R_RSI, of_convert(S_EDX), R_R10d);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #endif
        case 262:
            S_RAX = my_fstatat(emu, S_EDI, (char*)R_RSI, (void*)R_RDX, S_R10d);
            if (log) snprintf(buff2, 127, "[sys_fstatat %d, \"%s\", %p, 0x%x]", S_EDI, (char*)R_RSI, (void*)R_RDX, S_R10d);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #ifndef __NR_renameat
        case 264:
            R_EAX = (uint32_t)renameat(S_RDI, (const char*)R_RSI, S_EDX, (const char*)R_R10);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #endif
        case 267:   // sys_readlinkat
            if(log) snprintf(buff2, 127, " [sys_readlinkat(%d, \"%s\"...]", S_EDI, (char*)R_RSI);
            S_RAX = my_readlinkat(emu, S_EDI, (void*)R_RSI, (void*)R_RDX, R_R10); 
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #ifndef NOALIGN
        case 281:   // sys_epoll_pwait
            S_RAX = my_epoll_pwait(emu, S_EDI, (void*)R_RSI, S_EDX, S_R10d, (void*)R_R8);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #endif
        case 282:   // sys_signalfd
            // need to mask SIGSEGV
            {
                //TODO: convert the sigset from x64!
                sigset_t * set = (sigset_t *)R_RSI;
                if(sigismember(set, SIGSEGV)) {
                    sigdelset(set, SIGSEGV);
                    printf_log(LOG_INFO, "Warning, signalfd on SIGSEGV unsupported\n");
                }
                S_RAX = signalfd(S_EDI, set, 0);
                if(S_RAX==-1)
                    S_RAX = -errno;
            }
            break;
        #ifndef _NR_eventfd
        case 284:   // sys_eventfd
            S_RAX = eventfd(S_EDI, 0);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #endif
        #ifndef NOALIGN
        case 291:   // sys__epoll_create1
            S_RAX = epoll_create1(of_convert(S_EDI));
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #endif
        case 317:   // sys_seccomp
            R_RAX = 0;  // ignoring call
            break;
        case 334: // It is helpeful to run static binary
            R_RAX = -ENOSYS;
            break;
        #ifndef __NR_fchmodat4
        case 434:
            S_RAX = fchmodat(S_EDI, (void*)R_RSI, (mode_t)R_RDX, S_R10d);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #endif
        #ifndef __NR_faccessat2
        case 439:
            S_RAX = faccessat(S_EDI, (void*)R_RSI, (mode_t)R_RDX, S_R10d);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #endif
        #if !defined(__NR_epoll_pwait2) || !defined(NOALIGN)
        case 441:   // sys_epoll_pwait2
            S_RAX = my_epoll_pwait2(emu, S_EDI, (void*)R_RSI, S_EDX, (void*)S_R10, (void*)R_R8);
            if(S_RAX==-1)
                S_RAX = -errno;
            break;
        #endif
        case 449:
            #if defined(__NR_futex_waitv) && !defined(BAD_SIGNAL)
            S_RAX = syscall(__NR_futex_waitv, R_RDI, R_RSI, R_RDX, R_R10, R_R8);
            #else
            S_RAX = -ENOSYS;
            #endif
            break;
        default:
            printf_log(LOG_INFO, "Warning: Unsupported Syscall 0x%02Xh (%d)\n", s, s);
            S_RAX = -ENOSYS;
            break;
    }
    if(log) {
        if(BOX64ENV(rolling_log))
            snprintf(buffret, 127, "0x%lx%s", R_RAX, buff2);
        else
            printf_log_prefix(0, LOG_NONE, "=> 0x%lx%s\n", R_RAX, buff2);
    }
}

#define stack(n) (R_RSP+8+n)
#define i32(n)  *(int32_t*)stack(n)
#define u32(n)  *(uint32_t*)stack(n)
#define i64(n)  *(int64_t*)stack(n)
#define u64(n)  *(uint64_t*)stack(n)
#define p(n)    *(void**)stack(n)

long EXPORT my_syscall(x64emu_t *emu)
{
    static uint32_t warned = 0;
    uint32_t s = R_EDI;
    printf_dump(LOG_DEBUG, "%04d| %p: Calling libc syscall 0x%02X (%d) %p %p %p %p %p\n", GetTID(), (void*)R_RIP, s, s, (void*)R_RSI, (void*)R_RDX, (void*)R_RCX, (void*)R_R8, (void*)R_R9);
    // check wrapper first
    uint32_t cnt = sizeof(syscallwrap) / sizeof(scwrap_t);
    if(s<cnt && syscallwrap[s].nats) {
        int sc = syscallwrap[s].nats;
        switch(syscallwrap[s].nbpars) {
            case 0: return syscall(sc);
            case 1: return syscall(sc, R_RSI);
            case 2: return syscall(sc, R_RSI, R_RDX);
            case 3: return syscall(sc, R_RSI, R_RDX, R_RCX);
            case 4: return syscall(sc, R_RSI, R_RDX, R_RCX, R_R8);
            case 5: return syscall(sc, R_RSI, R_RDX, R_RCX, R_R8, R_R9);
            case 6: return syscall(sc, R_RSI, R_RDX, R_RCX, R_R8, R_R9, u64(0));
            default:
                printf_log(LOG_NONE, "ERROR, Unimplemented syscall wrapper (%d, %d)\n", s, syscallwrap[s].nbpars);
                emu->quit = 1;
                return 0;
        }
    }
    switch (s) {
        case 0:  // sys_read
            return read(R_ESI, (void*)R_RDX, R_ECX);
        case 1:  // sys_write
            return write(R_ESI, (void*)R_RDX, R_ECX);
        case 2: // sys_open
            return my_open(emu, (char*)R_RSI, of_convert(R_EDX), R_ECX);
        case 3:  // sys_close
            return close(R_ESI);
        case 4: // sys_stat
            return my_stat(emu, (void*)R_RSI, (void*)R_RDX);
        case 5: // sys_fstat
            return my_fstat(emu, S_ESI, (void*)R_RDX);
        case 6: // sys_lstat
            return my_lstat(emu, (void*)R_RSI, (void*)R_RDX);
        #ifndef __NR_poll
        case 7: // sys_poll
            return poll((struct pollfd*)R_RSI, (nfds_t)R_RDX, S_ECX);
            break;
        #endif
        case 9: // sys_mmap
            return (intptr_t)my_mmap64(emu, (void*)R_RSI, R_RDX, S_RCX, S_R8d, S_R9, i64(0));
        case 10: // sys_mprotect
            return my_mprotect(emu, (void*)R_RSI, R_RDX, S_ECX);
        case 11: // sys_munmap
            return my_munmap(emu, (void*)R_RSI, R_RDX);
        case 13: // sys_rt_sigaction
            #if 1
            return my_syscall_rt_sigaction(emu, S_ESI, (const x64_sigaction_restorer_t *)R_RDX, (x64_sigaction_restorer_t *)R_RCX, (size_t)R_R8);
            #else
            {
                x64_sigaction_t n ={0};
                x64_sigaction_t o = {0};
                if(R_RDX) {
                    x64_sigaction_restorer_t *p = (x64_sigaction_restorer_t*)R_RDX;
                    n._u._sa_sigaction = p->_u._sa_sigaction;
                    n.sa_flags = p->sa_flags;
                    n.sa_restorer = p->sa_restorer;
                    memcpy(&n.sa_mask, &p->sa_mask, R_R8);
                }
                long ret = my_sigaction(emu, S_ESI, R_RDX?&n:NULL, R_RCX?&o:NULL/*, (size_t)R_R8*/);
                if(R_RCX) {
                    x64_sigaction_restorer_t *p = (x64_sigaction_restorer_t*)R_RCX;
                    p->_u._sa_sigaction = o._u._sa_sigaction;
                    p->sa_flags = o.sa_flags;
                    p->sa_restorer = o.sa_restorer;
                    memcpy(&p->sa_mask, &o.sa_mask, R_R8);
                }
                return ret;
            }
            #endif
        #ifndef __NR_access
        case 21: // sys_access
            return access((void*)R_RSI, R_EDX);
        #endif
        #ifndef __NR_pipe
        case 22:
            return pipe((void*)R_RSI);
        #endif
        #ifndef __NR_select
        case 23: // sys_select
            return select(R_RSI, (void*)R_RDX, (void*)R_RCX, (void*)R_R8, (void*)R_R9);
        #endif
        case 25: // sys_mremap
            return (intptr_t)my_mremap(emu, (void*)R_RSI, R_RDX, R_RCX, R_R8d, (void*)R_R9);
        case 56: // sys_clone
            // x86_64 raw syscall is long clone(unsigned long flags, void *stack, int *parent_tid, int *child_tid, unsigned long tls);
            // so flags=R_RSI, stack=R_RDX, parent_tid=R_RCX, child_tid=R_R8, tls=R_R9
            if(R_RDX)
            {
                void* stack_base = (void*)R_RDX;
                int stack_size = 0;
                uintptr_t sp = R_RDX;
                if(!stack_base) {
                    // allocate a new stack...
                    int currstack = 0;
                    if((R_RSP>=(uintptr_t)emu->init_stack) && (R_RSP<=((uintptr_t)emu->init_stack+emu->size_stack)))
                        currstack = 1;
                    stack_size = (currstack)?emu->size_stack:(1024*1024);
                    stack_base = mmap(NULL, stack_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_GROWSDOWN, -1, 0);
                    // copy value from old stack to new stack
                    if(currstack) {
                        memcpy(stack_base, emu->init_stack, stack_size);
                        sp = (uintptr_t)emu->init_stack + R_RSP - (uintptr_t)stack_base;
                    } else {
                        int size_to_copy = (uintptr_t)emu->init_stack + emu->size_stack - (R_RSP);
                        memcpy(stack_base+stack_size-size_to_copy, (void*)R_RSP, size_to_copy);
                        sp = (uintptr_t)stack_base+stack_size-size_to_copy;
                    }
                }
                x64emu_t * newemu = NewX64Emu(emu->context, R_RIP, (uintptr_t)stack_base, stack_size, (R_RDX)?0:1);
                SetupX64Emu(newemu, emu);
                CloneEmu(newemu, emu);
                newemu->regs[_SP].q[0] = sp;  // setup new stack pointer
                void* mystack = NULL;
                if(my_context->stack_clone_used) {
                    mystack = box_malloc(1024*1024);  // stack for own process... memory leak, but no practical way to remove it
                } else {
                    if(!my_context->stack_clone)
                        my_context->stack_clone = box_malloc(1024*1024);
                    mystack = my_context->stack_clone;
                    my_context->stack_clone_used = 1;
                }
                // x86_64 raw clone is long clone(unsigned long flags, void *stack, int *parent_tid, int *child_tid, unsigned long tls);
                long ret = clone(clone_fn_syscall, (void*)((uintptr_t)mystack+1024*1024), R_ESI, newemu, R_RCX, R_R9, R_R8);
                return ret;
            }
            else
                #ifdef NOALIGN
                return syscall(__NR_clone, R_RSI, R_RDX, R_RCX, R_R8, R_R9);
                #else
                return syscall(__NR_clone, R_RSI, R_RDX, R_RCX, R_R9, R_R8);    // invert R_R8/R_R9 on Aarch64 and most other
                #endif
            break;
        #ifndef __NR_dup2
        case 33:
            return  dup2(S_ESI, S_EDX);
        #endif
        #ifndef __NR_fork
        case 57:
            return fork();
        #endif
        case 58:   // vfork
            return my_vfork(emu);
        case 59:   // execve
            return my_execve(emu, (const char*)R_RSI, (char* const*)R_RDX, (char* const*)R_RCX);
        case 63:    //uname
            {
                old_utsname_t *old = (old_utsname_t*)R_RSI;
                struct utsname uts;
                long ret = uname(&uts);
                memcpy(old, &uts, sizeof(*old));
                strcpy(old->machine, "x86_64");
                return ret;
            }
        case 72:    //fcntl
            R_RAX = (uint64_t)my_fcntl(emu, S_ESI, S_EDX, (void*)R_RCX);
            break;
        #ifndef __NR_getdents
        case 78:
            {
                size_t count = R_RCX;
                nat_linux_dirent64_t *d64 = (nat_linux_dirent64_t*)alloca(count);
                ssize_t ret = syscall(__NR_getdents64, R_ESI, d64, count);
                ret = DirentFromDirent64((void*)R_RDX, d64, ret);
                return ret;
            }
        #endif
        #ifndef __NR_mkdir
        case 83: // sys_mkdir
            return mkdir((void*)R_RSI, R_EDX);
        #endif
        #ifndef __NR_unlink
        case 87: //sys_unlink
            return unlink((void*)R_RSI);
        #endif
        case 89: // sys_readlink
            return my_readlink(emu,(void*)R_RSI, (void*)R_RDX, (size_t)R_RCX);
        #ifndef __NR_getrlimit
        case 97:
            return getrlimit(S_ESI, (void*)R_RDX);
        #endif
        case 131: // sys_sigaltstack
            return my_sigaltstack(emu, (void*)R_RSI, (void*)R_RDX);
        #ifndef __NR_mknod
        case 133: // sys_mknod
            return mknod((void*)R_RSI, R_EDX, R_RCX);
        #endif
        case 158: // sys_arch_prctl
            return my_arch_prctl(emu, S_ESI, (void*)R_RDX);
        #ifndef __NR_setrlimit
        case 160:
            return setrlimit(S_ESI, (void*)R_RDX);
        #endif
        case 175: // sys_init_module
            // huh?
            errno = -EPERM;
            return -1;
        #ifndef __NR_time
        case 201: // sys_time
            return (intptr_t)time((void*)R_RSI);
        #endif
        #ifndef __NR_epoll_create
        case 213:
            return epoll_create(S_ESI);
        #endif
        #if !defined(__NR_epoll_wait) || !defined(NOALIGN)
        case 232:
            return my_epoll_wait(emu, S_ESI, (void*)R_RDX, S_ECX, S_R8d);
        #endif
        #if !defined(__NR_epoll_ctl) || !defined(NOALIGN)
        case 233:
            return my_epoll_ctl(emu, S_ESI, S_EDX, S_ECX, (void*)R_R8);
        #endif
        #ifndef __NR_inotify_init
        case 253:
            return syscall(__NR_inotify_init1, 0);
        #endif
        #ifndef NOALIGN
        case 257:
            syscall(__NR_openat, S_ESI, (void*)R_RDX, of_convert(S_ECX), R_R8d);
        #endif
        case 262:
            return my_fstatat(emu, S_RSI, (char*)R_RDX, (void*)R_RCX, S_R8d);
        #ifndef __NR_renameat
        case 264:
            return renameat(S_RSI, (const char*)R_RDX, S_ECX, (const char*)R_R8);
        #endif
        case 267:   // sys_readlinkat
            return my_readlinkat(emu, S_RSI, (void*)R_RDX, (void*)R_RCX, R_R8); 
        #ifndef NOALIGN
        case 281:   // sys_epoll_pwait
            return my_epoll_pwait(emu, S_ESI, (void*)R_RDX, S_ECX, S_R8d, (void*)R_R9);
        #endif
        case 282:   // sys_signalfd
            // need to mask SIGSEGV
            {
                //TODO: convert sigset from x64
                sigset_t * set = (sigset_t *)R_RDX;
                if(sigismember(set, SIGSEGV)) {
                    sigdelset(set, SIGSEGV);
                    printf_log(LOG_INFO, "Warning, signalfd on SIGSEGV unsupported\n");
                }
                return signalfd(S_ESI, set, 0);
            }
            break;
        #ifndef _NR_eventfd
        case 284:   // sys_eventfd
            return eventfd(S_ESI, 0);
        #endif
        #ifndef NOALIGN
        case 291:   // sys__epoll_create1
            return epoll_create1(of_convert(S_EDI));
        #endif
        case 317:   // sys_seccomp
            return 0;  // ignoring call
        #ifndef __NR_fchmodat4
        case 434:
            return fchmodat(S_ESI, (void*)R_RDX, (mode_t)R_RCX, S_R8d);
        #endif
        #ifndef __NR_faccessat2
        case 439:
            return faccessat(S_ESI, (void*)R_RDX, (mode_t)R_RCX, S_R8d);
        #endif
        #if !defined(__NR_epoll_pwait2) || !defined(NOALIGN)
        case 441:
            return my_epoll_pwait2(emu, S_ESI, (void*)R_RDX, S_ECX, (void*)S_R8, (void*)R_R9);
            break;
        #endif
        case 449:
            #if defined(__NR_futex_waitv) && !defined(BAD_SIGNAL)
            return syscall(__NR_futex_waitv, R_RSI, R_RDX, R_RCX, R_R8, R_R9);
            #else
            errno = ENOSYS;
            return -1;
            #endif
        default:
            if(!(warned&(1<<s))) {
                printf_log(LOG_INFO, "Warning: Unsupported libc Syscall 0x%02X (%d)\n", s, s);
                warned|=(1<<s);
            }
            errno = ENOSYS;
            return -1;
    }
    return 0;
}
