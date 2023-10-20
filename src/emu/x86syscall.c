#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/syscall.h>   /* For SYS_xxx definitions */
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
#ifndef __NR_socketcall
#include <linux/net.h>
#include <sys/socket.h>
#endif
#include <sys/resource.h>
#include <poll.h>

#include "debug.h"
#include "box64stack.h"
#include "x64emu.h"
#include "x64run.h"
#include "x64emu_private.h"
#include "x64trace.h"
#include "myalign.h"
#include "box64context.h"
#include "callback.h"
#include "signals.h"
#include "x64tls.h"


// Syscall table for x86_64 can be found 
typedef struct scwrap_s {
    uint32_t x86s;
    int nats;
    int nbpars;
} scwrap_t;

static const scwrap_t syscallwrap[] = {
    //{ 2, __NR_fork, 1 },  
    //{ 3, __NR_read, 3 },  // wrapped so SA_RESTART can be handled by libc
    //{ 4, __NR_write, 3 }, // same
    //{ 5, __NR_open, 3 },  // flags need transformation
    //{ 6, __NR_close, 1 },   // wrapped so SA_RESTART can be handled by libc
    //{ 7, __NR_waitpid, 3 },
    //{ 10, __NR_unlink, 1 },
    //{ 12, __NR_chdir, 1 },
    //{ 13, __NR_time, 1 },
    //{ 15, __NR_chmod, 2 },
    //{ 19, __NR_lseek, 3 },
    //{ 20, __NR_getpid, 0 },
    //{ 24, __NR_getuid, 0 },
    //{ 33, __NR_access, 2 },
    //{ 37, __NR_kill, 2 },
    //{ 38, __NR_rename, 2 },
    //{ 39, __NR_mkdir, 2 },
    //{ 40, __NR_rmdir, 1 },
    //{ 41, __NR_dup, 1 },
    //{ 42, __NR_pipe, 1 },
    //{ 45, __NR_brk, 1 },
    //{ 47, __NR_getgid, 0 },
    //{ 49, __NR_geteuid, 0 },
    //{ 50, __NR_getegid, 0 },
    //{ 54, __NR_ioctl, 3 },    // should be wrapped to allow SA_RESTART handling by libc, but syscall is only 3 arguments, ioctl can be 5
    //{ 55, __NR_fcntl, 3 },    // wrapped to allow filter of F_SETFD
    //{ 60, __NR_umask, 1 },
    //{ 63, __NR_dup2, 2 },
    //{ 64, __NR_getppid, 0 },
    //{ 66, __NR_setsid, 0 },
    //{ 75, __NR_setrlimit, 2 },
    //{ 76, __NR_getrlimit, 2 },
    //{ 77, __NR_getrusage, 2 },
    //{ 78, __NR_gettimeofday, 2 },
    //{ 83, __NR_symlink, 2 },
    //{ 82, __NR_select, 5 },
    //{ 85, __NR_readlink, 3 },
    //{ 91, __NR_munmap, 2 },
    //{ 94, __NR_fchmod, 2 },
    //{ 99, __NR_statfs, 2 },
    //{ 102, __NR_socketcall, 2 },
    //{ 104, __NR_setitimer, 3 },
    //{ 105, __NR_getitimer, 2 },
    //{ 106, __NR_newstat, 2 },
    //{ 106, __NR_stat, 2 },
    //{ 107, __NR_newlstat, 2 },
    //{ 107, __NR_lstat, 2 },
    //{ 108, __NR_newfstat, 2 },
    //{ 108, __NR_fstat, 2 },
    //{ 109, __NR_olduname, 1 },
    //{ 110, __NR_iopl, 1 },
    //{ 114, __NR_wait4, 4 }, //TODO: check struct rusage alignment
    //{ 117, __NR_ipc, 6 },
    //{ 119, __NR_sigreturn, 0},
    //{ 120, __NR_clone, 5 },    // need works
    //{ 122, __NR_uname, 1 },
    //{ 123, __NR_modify_ldt },
    //{ 125, __NR_mprotect, 3 },
    //{ 136, __NR_personality, 1 },
    //{ 140, __NR__llseek, 5 },
    //{ 141, __NR_getdents, 3 },
    //{ 142, __NR__newselect, 5 },
    //{ 143, __NR_flock,  2 },
    //{ 144, __NR_msync, 3 },
    //{ 145, __NR_readv, 3 },
    //{ 146, __NR_writev, 3 },
    //{ 148, __NR_fdatasync, 1 },
    //{ 149, __NR__sysctl, 1 },    // need wrapping?
    //{ 156, __NR_sched_setscheduler, 3 },
    //{ 157, __NR_sched_getscheduler, 1 },
    //{ 158, __NR_sched_yield, 0 },
    //{ 162, __NR_nanosleep, 2 },
    //{ 164, __NR_setresuid, 3 },
    //{ 168, __NR_poll, 3 },    // wrapped to allow SA_RESTART wrapping by libc
    //{ 172, __NR_prctl, 5 },
    //{ 173, __NR_rt_sigreturn, 0 },
    //{ 175, __NR_rt_sigprocmask, 4 },
    //{ 179, __NR_rt_sigsuspend, 2 },
    //{ 183, __NR_getcwd, 2 },
    //{ 184, __NR_capget, 2},
    //{ 185, __NR_capset, 2},
    //{ 186, __NR_sigaltstack, 2 },    // neeed wrap or something?
    //{ 191, __NR_ugetrlimit, 2 },
//    { 192, __NR_mmap2, 6},
    //{ 195, __NR_stat64, 2 },  // need proprer wrap because of structure size change
    //{ 196, __NR_lstat64, 2 }, // need proprer wrap because of structure size change
    //{ 197, __NR_fstat64, 2 },  // need proprer wrap because of structure size change
    //{ 199, __NR_getuid32, 0 },
    //{ 200, __NR_getgid32, 0 },
    //{ 201, __NR_geteuid32, 0 },
    //{ 202, __NR_getegid32, 0 },
    //{ 208, __NR_setresuid32, 3 },
    //{ 209, __NR_getresuid32, 3 },
    //{ 210, __NR_setresgid32, 3 },
    //{ 211, __NR_getresgid32, 3 },
    //{ 220, __NR_getdents64, 3 },
    //{ 221, __NR_fcntl64, 3 },
    { 224, __NR_gettid, 0 },
    //{ 240, __NR_futex, 6 },
    //{ 241, __NR_sched_setaffinity, 3 },
    //{ 242, __NR_sched_getaffinity, 3 },
    //{ 252, __NR_exit_group, 1 },
    //{ 254, __NR_epoll_create, 1 },
    //{ 255, __NR_epoll_ctl, 4 },
    //{ 256, __NR_epoll_wait, 4 },
    //{ 265, __NR_clock_gettime, 2 },
    //{ 266, __NR_clock_getres, 2 },
    //{ 270, __NR_tgkill, 3 },
    //{ 271, __NR_utimes, 2 },
    //{ 291, __NR_inotify_init, 0},
    //{ 292, __NR_inotify_add_watch, 3},
    //{ 293, __NR_inotify_rm_watch, 2},
    //{ 311, __NR_set_robust_list, 2 },
    //{ 312, __NR_get_robust_list, 4 },
    //{ 318, __NR_getcpu, 3},
    //{ 328, __NR_eventfd2, 2},
    //{ 329, __NR_epoll_create1, 1 },
    //{ 331, __NR_pipe2, 2},
    //{ 332, __NR_inotify_init1, 1},
    //{ 355, __NR_getrandom, 3 },
    //{ 356, __NR_memfd_create, 2},
    //{ 449, __NR_futex_waitv, 5},
};

struct mmap_arg_struct {
    unsigned long addr;
    unsigned long len;
    unsigned long prot;
    unsigned long flags;
    unsigned long fd;
    unsigned long offset;
};

#undef st_atime
#undef st_ctime
#undef st_mtime

struct x64_pt_regs {
	long ebx;
	long ecx;
	long edx;
	long esi;
	long edi;
	long ebp;
	long eax;
	int  xds;
	int  xes;
	int  xfs;
	int  xgs;
	long orig_eax;
	long eip;
	int  xcs;
	long eflags;
	long esp;
	int  xss;
};

#ifndef __NR_olduname
struct oldold_utsname {
        char sysname[9];
        char nodename[9];
        char release[9];
        char version[9];
        char machine[9];
};
#endif
struct old_utsname {
        char sysname[65];
        char nodename[65];
        char release[65];
        char version[65];
        char machine[65];
};

struct i386_user_desc {
    unsigned int  entry_number;
    unsigned long base_addr;
    unsigned int  limit;
    unsigned int  seg_32bit:1;
    unsigned int  contents:2;
    unsigned int  read_exec_only:1;
    unsigned int  limit_in_pages:1;
    unsigned int  seg_not_present:1;
    unsigned int  useable:1;
};

void EXPORT x86Syscall(x64emu_t *emu)
{
    uint32_t s = R_EAX;
    printf_log(LOG_DEBUG, "%p: Calling 32bits syscall 0x%02X (%d) %p %p %p %p %p", (void*)R_RIP, s, s, (void*)(uintptr_t)R_EBX, (void*)(uintptr_t)R_ECX, (void*)(uintptr_t)R_EDX, (void*)(uintptr_t)R_ESI, (void*)(uintptr_t)R_EDI); 
    // check wrapper first
    int cnt = sizeof(syscallwrap) / sizeof(scwrap_t);
    for (int i=0; i<cnt; i++) {
        if(syscallwrap[i].x86s == s) {
            int sc = syscallwrap[i].nats;
            switch(syscallwrap[i].nbpars) {
                case 0: *(int32_t*)&R_EAX = syscall(sc); break;
                case 1: *(int32_t*)&R_EAX = syscall(sc, R_EBX); break;
                case 2: *(int32_t*)&R_EAX = syscall(sc, R_EBX, R_ECX); break;
                case 3: *(int32_t*)&R_EAX = syscall(sc, R_EBX, R_ECX, R_EDX); break;
                case 4: *(int32_t*)&R_EAX = syscall(sc, R_EBX, R_ECX, R_EDX, R_ESI); break;
                case 5: *(int32_t*)&R_EAX = syscall(sc, R_EBX, R_ECX, R_EDX, R_ESI, R_EDI); break;
                case 6: *(int32_t*)&R_EAX = syscall(sc, R_EBX, R_ECX, R_EDX, R_ESI, R_EDI, R_EBP); break;
                default:
                   printf_log(LOG_NONE, "ERROR, Unimplemented syscall wrapper (%d, %d)\n", s, syscallwrap[i].nbpars); 
                   emu->quit = 1;
                   return;
            }
            if(R_EAX==0xffffffff && errno>0)
                R_EAX = (uint32_t)-errno;
            printf_log(LOG_DEBUG, " => 0x%x\n", R_EAX);
            return;
        }
    }
    switch (s) {
        case 1: // sys_exit
            emu->quit = 1;
            emu->exit = 1;
            //R_EAX = syscall(__NR_exit, R_EBX);  // the syscall should exit only current thread
            R_EAX = R_EBX; // faking the syscall here, we don't want to really terminate the thread now
            break;
        /*case 123:   // SYS_modify_ldt
            R_EAX = my_modify_ldt(emu, R_EBX, (thread_area_t*)(uintptr_t)R_ECX, R_EDX);
            if(R_EAX==0xffffffff && errno>0)
                R_EAX = (uint32_t)-errno;
            break;*/
        case 243: // set_thread_area
            R_EAX = my_set_thread_area_32(emu, (thread_area_32_t*)(uintptr_t)R_EBX);
            if(R_EAX==0xffffffff && errno>0)
                R_EAX = (uint32_t)-errno;
            break;
        default:
            printf_log(LOG_INFO, "Warning: Unsupported Syscall 0x%02Xh (%d)\n", s, s);
            R_EAX = (uint32_t)-ENOSYS;
            return;
    }
    printf_log(LOG_DEBUG, " => 0x%x\n", R_EAX);
}
