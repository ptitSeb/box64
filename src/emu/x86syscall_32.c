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
#include <linux/futex.h>

#include "os.h"
#include "debug.h"
#include "box64stack.h"
#include "x64emu.h"
#include "box64cpu.h"
#include "x64emu_private.h"
#include "x64trace.h"
#include "myalign32.h"
#include "box64context.h"
#include "callback.h"
#include "signals.h"
#include "x64tls.h"
#include "box32.h"
#include "converter32.h"
#include "custommem.h"


// Syscall table for x86_64 can be found 
typedef struct scwrap_s {
    uint32_t x86s;
    int nats;
    int nbpars;
} scwrap_t;

static const scwrap_t syscallwrap[] = {
    #ifdef __NR_fork
    { 2, __NR_fork, 1 },  
    #endif
    //{ 3, __NR_read, 3 },  // wrapped so SA_RESTART can be handled by libc
    //{ 4, __NR_write, 3 }, // same
    //{ 5, __NR_open, 3 },  // flags need transformation
    //{ 6, __NR_close, 1 },   // wrapped so SA_RESTART can be handled by libc
    #ifdef __NR_waitpid
    { 7, __NR_waitpid, 3 },
    #endif
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
    #ifdef __NR_pipe
    { 42, __NR_pipe, 1 },
    #endif
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
    //{ 90, __NR_old_mmap, 1 },
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
    { 172, __NR_prctl, 5 },
    //{ 173, __NR_rt_sigreturn, 0 },
    //{ 175, __NR_rt_sigprocmask, 4 },
    //{ 179, __NR_rt_sigsuspend, 2 },
    //{ 183, __NR_getcwd, 2 },
    //{ 184, __NR_capget, 2},
    //{ 185, __NR_capset, 2},
    //{ 186, __NR_sigaltstack, 2 },    // neeed wrap or something?
    //{ 191, __NR_ugetrlimit, 2 },
    //{ 192, __NR_mmap2, 6},
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
    //{ 240, __NR_futex, 6 },   // needs wrapping for the optionnal timespec part
    //{ 241, __NR_sched_setaffinity, 3 },
    //{ 242, __NR_sched_getaffinity, 3 },
    //{ 243, __NR_set_thread_area, 1 },
    //{ 252, __NR_exit_group, 1 },
    //{ 254, __NR_epoll_create, 1 },
    //{ 255, __NR_epoll_ctl, 4 },
    //{ 256, __NR_epoll_wait, 4 },
    //{ 265, __NR_clock_gettime, 2 },
    //{ 266, __NR_clock_getres, 2 },
    { 270, __NR_tgkill, 3 },
    //{ 271, __NR_utimes, 2 },
    //{ 291, __NR_inotify_init, 0},
    //{ 292, __NR_inotify_add_watch, 3},
    //{ 293, __NR_inotify_rm_watch, 2},
    //{ 311, __NR_set_robust_list, 2 },
    //{ 312, __NR_get_robust_list, 4 }, // need wrapping of 3rd arg
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
    ulong_t addr;
    ulong_t len;
    ulong_t prot;
    ulong_t flags;
    ulong_t fd;
    ulong_t offset;
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

struct i386_robust_list {
        ptr_t next; // struct i386_robust_list *
};
struct i386_robust_list_head {
        struct i386_robust_list list;
        long_t futex_offset;
        ptr_t  list_op_pending; // struct robust_list *
};

typedef struct i386_stack_s i386_stack_t;

typedef struct i386_linux_dirent_s {
    uint32_t d_ino;
    uint32_t d_off;
    uint16_t d_reclen;
    char d_name[];
} i386_linux_dirent_t;

typedef struct native_linux_dirent {
    unsigned long d_ino;
    unsigned long d_off;
    unsigned short d_reclen;
    char d_name[];
} native_linux_dirent_t;

int32_t my_open(x64emu_t* emu, void* pathname, int32_t flags, uint32_t mode);
int32_t my32_execve(x64emu_t* emu, const char* path, ptr_t argv[], ptr_t envp[]);
ssize_t my32_read(int fd, void* buf, size_t count);
void* my32_mmap64(x64emu_t* emu, void *addr, size_t length, int prot, int flags, int fd, int64_t offset);
int my32_munmap(x64emu_t* emu, void* addr, unsigned long length);
int my32_sigaltstack(x64emu_t* emu, const i386_stack_t* ss, i386_stack_t* oss);
pid_t my_vfork(x64emu_t* emu);

#ifndef FUTEX_LOCK_PI2
#define FUTEX_LOCK_PI2 13
#endif

typedef struct clone_s {
    x64emu_t* emu;
    void* stack2free;
} clone_t;

static int clone32_fn(void* arg)
{
    clone_t* args = arg;
    x64emu_t *emu = args->emu;
    printf_log(LOG_DEBUG, "%04d|New clone32_fn starting with emu=%p (R_ESP=%p)\n", GetTID(), arg, from_ptrv(R_ESP));
    thread_forget_emu();
    thread_set_emu(emu);
    R_EAX = 0;
    DynaRun(emu);
    int ret = S_EAX;
    printf_log(LOG_DEBUG, "%04d|clone32_fn ending with ret=%d (emu=%p)\n", GetTID(), ret, arg);
    FreeX64Emu(&emu);
    void* stack2free = args->stack2free;
    box_free(args);
    if(my_context->stack_clone_used && !stack2free)
        my_context->stack_clone_used = 0;
    if(stack2free)
        box_free(stack2free);   // this free the stack, so it will crash very soon!
    _exit(ret);
}


void EXPORT x86Syscall(x64emu_t *emu)
{
    uint32_t s = R_EAX;
    printf_log(LOG_DEBUG, "%04d|%p: Calling 32bits syscall 0x%02X (%d) %p %p %p %p %p", GetTID(), (void*)R_RIP, s, s, (void*)(uintptr_t)R_EBX, (void*)(uintptr_t)R_ECX, (void*)(uintptr_t)R_EDX, (void*)(uintptr_t)R_ESI, (void*)(uintptr_t)R_EDI); 
    // check wrapper first
    int cnt = sizeof(syscallwrap) / sizeof(scwrap_t);
    void* tmp;
    size_t tmps;
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
        #ifndef __NR_fork
        case 2:
            S_EAX = fork();
            if(S_EAX==-1)
                S_EAX = -errno;
            break;
        #endif
        case 3:  // sys_read
            S_EAX = to_long(my32_read((int)R_EBX, from_ptrv(R_ECX), from_ulong(R_EDX)));
            break;
        case 4:  // sys_write
            S_EAX = to_long(write((int)R_EBX, from_ptrv(R_ECX), from_ulong(R_EDX)));
            break;
        case 5: // sys_open
            if(s==5) {printf_log(LOG_DEBUG, " => sys_open(\"%s\", %d, %d)", (char*)from_ptrv(R_EBX), of_convert32(R_ECX), R_EDX);}; 
            //S_EAX = open((void*)R_EBX, of_convert32(R_ECX), R_EDX);
            S_EAX = my_open(emu, from_ptrv(R_EBX), of_convert32(R_ECX), R_EDX);
            break;
        case 6:  // sys_close
            S_EAX = close((int)R_EBX);
            break;
#ifndef __NR_waitpid
        case 7: //sys_waitpid
            S_EAX = waitpid((pid_t)R_EBX, (int*)from_ptrv(R_ECX), S_EDX);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
#endif
        #ifndef __NR_fork
        case 42:
            S_EAX = pipe(from_ptrv(R_EBX));
            if(S_EAX==-1)
                S_EAX = -errno;
            break;
        #endif
        case 90:    // old_mmap
            {
                struct mmap_arg_struct *st = from_ptrv(R_EBX);
                R_EAX = to_ptrv(my32_mmap64(emu, from_ptrv(st->addr), st->len, st->prot, st->flags, st->fd, st->offset));
            }
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
        case 91:   // munmap
            S_EAX = my32_munmap(emu, from_ptrv(R_EBX), (unsigned long)R_ECX);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
        case 120: // sys_clone
            // x86 raw syscall is long clone(unsigned long flags, void *stack, int *parent_tid, unsigned long tls, int *child_tid);
            // so flags=R_EBX, stack=R_ECX, parent_tid=R_EDX, child_tid=R_ESI, tls=R_EDI
            if((R_EBX&~0xff)==0x4100) {
                // this is a case of vfork...
                S_RAX = my_vfork(emu);
                if(S_RAX==-1)
                    S_RAX = -errno;
            } else {
                if(R_ECX)
                {
                    void* stack_base = from_ptrv(R_ECX);
                    int stack_size = 0;
                    uintptr_t sp = R_ECX;
                    x64emu_t * newemu = NewX64Emu(emu->context, R_EIP, (uintptr_t)stack_base, stack_size, 0);
                    SetupX64Emu(newemu, emu);
                    CloneEmu(newemu, emu);
                    newemu->regs[_SP].q[0] = sp;  // setup new stack pointer
                    void* mystack = NULL;
                    clone_t* args = box_calloc(1, sizeof(clone_t));
                    args->emu = newemu;
                    if(my_context->stack_clone_used) {
                        args->stack2free = mystack = box_malloc(1024*1024);  // stack for own process...
                    } else {
                        if(!my_context->stack_clone)
                            my_context->stack_clone = box_malloc(1024*1024);
                        mystack = my_context->stack_clone;
                        my_context->stack_clone_used = 1;
                    }
                    int64_t ret = clone(clone32_fn, (void*)((uintptr_t)mystack+1024*1024), R_EBX, args, R_EDX, R_EDI, R_ESI);
                    S_RAX = ret;
                }
                else
                    #ifdef NOALIGN
                    S_RAX = syscall(__NR_clone, R_EBX, R_ECX, R_EDX, R_ESI, R_EDI);
                    #else
                    S_RAX = syscall(__NR_clone, R_EBX, R_ECX, R_EDX, R_EDI, R_ESI);    // invert R_ESI/R_EDI on Aarch64 and most other
                    #endif
            }
            break;        
        /*case 123:   // SYS_modify_ldt
            R_EAX = my32_modify_ldt(emu, R_EBX, (thread_area_t*)(uintptr_t)R_ECX, R_EDX);
            if(R_EAX==0xffffffff && errno>0)
                R_EAX = (uint32_t)-errno;
            break;*/
        case 141: { // getdents
            native_linux_dirent_t dirent_buffer[R_EDX];
            memset(dirent_buffer, 0, sizeof(dirent_buffer));
            S_EAX = syscall(__NR_getdents64, (unsigned)R_EBX, dirent_buffer, (unsigned int)R_EDX);
            if (S_EAX == -1) {
                if (errno > 0) S_EAX = -errno;
            } else {
                size_t total = 0;
                size_t off = 0;
                while (off < (size_t)S_EAX) {
                    native_linux_dirent_t* d = (native_linux_dirent_t*)((char*)dirent_buffer + off);
                    size_t reclen = sizeof(i386_linux_dirent_t) + strlen(d->d_name) + 1;
                    reclen = (reclen + 3) & ~3; // align to 4 bytes
                    if (total + reclen > (size_t)R_EDX)
                        break; // no more space
                    i386_linux_dirent_t* d32 = (i386_linux_dirent_t*)((char*)from_ptrv(R_ECX) + total);
                    d32->d_ino = (uint32_t)d->d_ino;
                    d32->d_off = (uint32_t)d->d_off;
                    d32->d_reclen = (uint16_t)reclen;
                    strcpy(d32->d_name, d->d_name);
                    total += reclen;
                    off += d->d_reclen;
                }
                S_EAX = total;
            }
            break;
        }
        case 186:   // sigaltstack
            S_EAX = my32_sigaltstack(emu, from_ptrv(R_EBX), from_ptrv(R_ECX));
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
        case 192:   // mmap2
            R_EAX = to_ptrv(my32_mmap64(emu, from_ptrv(R_EBX), (unsigned long)R_ECX, R_EDX, R_ESI, R_EDI, R_EBP));
            break;
        case 240: // futex
            {
                struct_LL_t tspec;
                int need_tspec = 1;
                switch(R_ECX&FUTEX_CMD_MASK) {
                    case FUTEX_WAIT:
                    case FUTEX_WAIT_BITSET:
                    case FUTEX_LOCK_PI:
                    case FUTEX_LOCK_PI2:
                    case FUTEX_WAIT_REQUEUE_PI:
                        need_tspec = 1;
                        break;
                    case FUTEX_CMP_REQUEUE_PI:
                    case FUTEX_UNLOCK_PI:
                    case FUTEX_TRYLOCK_PI:
                    case FUTEX_WAKE_BITSET:
                    case FUTEX_WAKE_OP:
                    case FUTEX_REQUEUE:
                    case FUTEX_CMP_REQUEUE:
                    case FUTEX_FD:
                    case FUTEX_WAKE:
                    default: need_tspec = 0;
                }
                if(need_tspec && R_ESI)
                    from_struct_LL(&tspec, R_ESI);
                else
                    need_tspec = 0;
                S_EAX = syscall(__NR_futex, R_EBX, R_ECX, R_EDX, need_tspec?&tspec:from_ptrv(R_ESI), R_EDI, R_EBP);
                if(S_EAX==-1 && errno>0)
                    R_EAX = (uint32_t)-errno;
            }
            break;
        case 243: // set_thread_area
            R_EAX = my_set_thread_area_32(emu, (thread_area_32_t*)(uintptr_t)R_EBX);
            if(R_EAX==0xffffffff && errno>0)
                R_EAX = (uint32_t)-errno;
            break;
        case 312: // get_robust_list
            {
                static struct i386_robust_list_head h;
                ulong_t *arg2 = from_ptrv(R_EDX);
                ptr_t* arg1 = from_ptrv(R_ECX);
                tmp = arg1?(from_ptrv(*arg1)):NULL;
                tmps = arg2?(from_ulong(*arg2)):0;
                tmp = R_ECX?from_ptrv(*(ptr_t*)from_ptrv(R_ECX)):NULL;
                tmps = R_EDX?from_ulong(*(ulong_t*)from_ptrv(R_EDX)):0;
                R_EAX = syscall(__NR_get_robust_list, S_EBX, arg1?(&tmp):NULL, arg2?(&tmps):NULL);
                if(!R_EAX) {
                    if(arg1) {
                        h.list.next = (*(void**)tmp==tmp)?to_ptrv(&h):to_ptrv(*(void**)tmp); // set head
                        h.futex_offset = to_long(((long*)tmp)[1]);
                        h.list_op_pending = to_ptrv(((void**)tmp)[2]);
                        *arg1 = to_ptrv(&h);   // should wrap all the structures, and keep it alive...
                    }
                    if(arg2) *arg2 = to_ulong(tmps/2);    // it's 2 times smaller in x86
                } else if(R_EAX==0xffffffff && errno>0)
                    R_EAX = (uint32_t)-errno;
            }
            break;
        case 449:
            #if defined(__NR_futex_waitv) && !defined(BAD_SIGNAL)
            S_RAX = syscall(__NR_futex_waitv, R_EBX, R_ECX, R_EDX, R_ESI, R_EDI);
            #else
            S_RAX = -ENOSYS;
            #endif
            break;
        default:
            printf_log(LOG_INFO, "Warning: Unsupported Syscall 0x%02Xh (%d)\n", s, s);
            R_EAX = (uint32_t)-ENOSYS;
            return;
    }
    printf_log(LOG_DEBUG, " => 0x%x\n", R_EAX);
}

#ifdef BOX32
#define stack(n) (b[(n)/4])
#define i32(n)  (int32_t)stack(n)
#define u32(n)  (uint32_t)stack(n)
#define p(n)    from_ptrv(stack(n))

uint32_t EXPORT my32_syscall(x64emu_t *emu, uint32_t s, ptr_t* b)
{
    static uint64_t warned[10] = {0};
    printf_log(LOG_DEBUG, "%p: Calling libc syscall 0x%02X (%d) %p %p %p %p %p\n", from_ptrv(R_EIP), s, s, from_ptrv(u32(0)), from_ptrv(u32(4)), from_ptrv(u32(8)), from_ptrv(u32(12)), from_ptrv(u32(16))); 
    // check wrapper first
    int cnt = sizeof(syscallwrap) / sizeof(scwrap_t);
    size_t tmps;
    void* tmp;
    int ret;
    for (int i=0; i<cnt; i++) {
        if(syscallwrap[i].x86s == s) {
            int sc = syscallwrap[i].nats;
            switch(syscallwrap[i].nbpars) {
                case 0: return syscall(sc);
                case 1: return syscall(sc, u32(0));
                case 2: return syscall(sc, u32(0), u32(4));
                case 3: return syscall(sc, u32(0), u32(4), u32(8));
                case 4: return syscall(sc, u32(0), u32(4), u32(8), u32(12));
                case 5: return syscall(sc, u32(0), u32(4), u32(8), u32(12), u32(16));
                case 6: return syscall(sc, u32(0), u32(4), u32(8), u32(12), u32(16), u32(20));
                default:
                   printf_log(LOG_NONE, "ERROR, Unimplemented syscall wrapper (%d, %d)\n", s, syscallwrap[i].nbpars); 
                   emu->quit = 1;
                   return 0;
            }
        }
    }
    switch (s) {
        case 1: // __NR_exit
            emu->quit = 1;
            return u32(0); // faking the syscall here, we don't want to really terminate the program now
        #ifndef __NR_fork
        case 2:
            return fork();
        #endif
        case 3:  // sys_read
            return (uint32_t)to_long(my32_read(i32(0), p(4), u32(8)));
        case 4:  // sys_write
            return (uint32_t)to_long(write(i32(0), p(4), u32(8)));
        case 5: // sys_open
            return my_open(emu, p(0), of_convert32(u32(4)), u32(8));
        case 6:  // sys_close
            return (uint32_t)close(i32(0));
        case 11: // execve
            return (uint32_t)my32_execve(emu, p(0), p(4), p(8));
        #ifndef __NR_fork
        case 42:
            S_EAX = pipe(p(0));
            if(S_EAX==-1)
                S_EAX = -errno;
            break;
        #endif
        case 91:   // munmap
            return (uint32_t)my32_munmap(emu, p(0), u32(4));
#if 0
        case 120:   // clone
            // x86 raw syscall is long clone(unsigned long flags, void *stack, int *parent_tid, unsigned long tls, int *child_tid);
            // so flags=u(0), stack=p(4), parent_tid=p(8), tls=p(12), child_tid=p(16)
            if(p(4))
            {
                void* stack_base = p(4);
                int stack_size = 0;
                if(!stack_base) {
                    // allocate a new stack...
                    int currstack = 0;
                    if((R_ESP>=(uintptr_t)emu->init_stack) && (R_ESP<=((uintptr_t)emu->init_stack+emu->size_stack)))
                        currstack = 1;
                    stack_size = (currstack)?emu->size_stack:(1024*1024);
                    stack_base = mmap(NULL, stack_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_GROWSDOWN, -1, 0);
                    // copy value from old stack to new stack
                    if(currstack)
                        memcpy(stack_base, emu->init_stack, stack_size);
                    else {
                        int size_to_copy = (uintptr_t)emu->init_stack + emu->size_stack - (R_ESP);
                        memcpy(stack_base+stack_size-size_to_copy, (void*)R_ESP, size_to_copy);
                    }
                }
                x64emu_t * newemu = NewX86Emu(emu->context, R_EIP, (uintptr_t)stack_base, stack_size, (p(4))?0:1);
                SetupX86Emu(newemu);
                CloneEmu(newemu, emu);
                Push32(newemu, 0);
                PushExit(newemu);
                void* mystack = NULL;
                if(my32_context->stack_clone_used) {
                    mystack = malloc(1024*1024);  // stack for own process... memory leak, but no practical way to remove it
                } else {
                    if(!my32_context->stack_clone)
                        my32_context->stack_clone = malloc(1024*1024);
                    mystack = my32_context->stack_clone;
                    my32_context->stack_clone_used = 1;
                }
                // x86_64 raw clone is long clone(unsigned long flags, void *stack, int *parent_tid, int *child_tid, unsigned long tls);
                long ret = clone(clone_fn, (void*)((uintptr_t)mystack+1024*1024), u32(0), newemu, p(8), p(12), p(16));
                return (uint32_t)ret;
            }
            else
                return (uint32_t)syscall(__NR_clone, u32(0), p(4), p(8), p(12), p(16));
            break;
        case 123:   // SYS_modify_ldt
            return my32_modify_ldt(emu, i32(0), (thread_area_t*)p(4), i32(8));
        case 125:   // mprotect
            return (uint32_t)my32_mprotect(emu, p(0), u32(4), i32(8));
        case 174:   // sys_rt_sigaction
            return (uint32_t)my32_sigaction(emu, i32(0), (x86_sigaction_t*)p(4), (x86_sigaction_t*)p(8));
#endif
        case 186:   // sigaltstack
            return my32_sigaltstack(emu, p(0), p(4));
        case 192:   // mmap2
            return to_ptrv(my32_mmap64(emu, p(0), u32(4), i32(8), i32(12), i32(16), u32(20)));
        case 240: // futex
            {
                struct_LL_t tspec;
                int need_tspec = 1;
                switch(u32(4)&FUTEX_CMD_MASK) {
                    case FUTEX_WAIT:
                    case FUTEX_WAIT_BITSET:
                    case FUTEX_LOCK_PI:
                    case FUTEX_LOCK_PI2:
                    case FUTEX_WAIT_REQUEUE_PI:
                        need_tspec = 1;
                        break;
                    case FUTEX_CMP_REQUEUE_PI:
                    case FUTEX_UNLOCK_PI:
                    case FUTEX_TRYLOCK_PI:
                    case FUTEX_WAKE_BITSET:
                    case FUTEX_WAKE_OP:
                    case FUTEX_REQUEUE:
                    case FUTEX_CMP_REQUEUE:
                    case FUTEX_FD:
                    case FUTEX_WAKE:
                    default: need_tspec = 0;
                }
                if(need_tspec && u32(12))
                    from_struct_LL(&tspec, u32(12));
                else
                    need_tspec = 0;
                return syscall(__NR_futex,  p(0), i32(4), u32(8), need_tspec?(&tspec):p(12), p(16), u32(20));
            }
            break;
        case 243: // set_thread_area
            return my_set_thread_area_32(emu, (thread_area_32_t*)p(0));
#if 0
        case 254: // epoll_create
            return my32_epoll_create(emu, i32(0));
        case 255: // epoll_ctl
            return my32_epoll_ctl(emu, i32(0), i32(4), i32(8), p(12));
        case 256: // epoll_wait
            return my32_epoll_wait(emu, i32(0), p(4), i32(8), i32(12));
        case 270: //_NR_tgkill
            /*if(!u32(8))*/ {
                //printf("tgkill(%u, %u, %u) => ", u32(0), u32(4), u32(8));
                uint32_t ret = (uint32_t)syscall(__NR_tgkill, u32(0), u32(4), u32(8));
                //printf("%u (errno=%d)\n", ret, (ret==(uint32_t)-1)?errno:0);
                return ret;
            }/* else {
                printf_log(LOG_INFO, "Warning: ignoring libc Syscall tgkill (%u, %u, %u)\n", u32(0), u32(4), u32(8));
            }*/
            return 0;
#endif
        case 312: // get_robust_list
            {
                // will wrap only head for now
                static uint8_t i386_nothing[0x14] = {0};  // for faking steamcmd use of get_robust_list
                static struct i386_robust_list_head h;
                ulong_t *arg2 = p(8);
                ptr_t* arg1 = p(4);
                tmp = arg1?(from_ptrv(*arg1)):NULL;
                tmps = arg2?(from_ulong(*arg2)):0;
                ret = syscall(__NR_get_robust_list, u32(0), arg1?(&tmp):NULL, arg2?(&tmps):NULL);
                if(!ret) {
                    if(box64_steamcmd || 1) {
                        h.list.next = to_ptrv(&h);
                        h.futex_offset = -0x14;
                        h.list_op_pending = 0;
                        *arg1 = to_ptrv(&h);
                        *arg2 = 12;
                    } else {
                        if(arg1) {
                            h.list.next = to_ptrv(((void**)tmp)[0]);//(*(void**)tmp==tmp)?to_ptrv(&h):to_ptrv(*(void**)tmp); // set head
                            h.futex_offset = to_long(((long*)tmp)[1]);
                            h.list_op_pending = to_ptrv(((void**)tmp)[2]);
                            *arg1 = to_ptrv(&h);   // should wrap all the structures, and keep it alive...
                        }
                        if(arg2) *arg2 = to_ulong(tmps/2);    // it's 2 times smaller in x86
                    }
                }
            }
            return ret;
#if 0
#ifndef NOALIGN
        case 329:   // epoll_create1
            return my32_epoll_create1(emu, of_convert32(i32(0)));
#endif
#ifndef __NR_getrandom
        case 355:  // getrandom
            return (uint32_t)my32_getrandom(emu, p(0), u32(4), u32(8));
#endif
#ifndef __NR_memfd_create
        case 356:  // memfd_create
            return (uint32_t)my32_memfd_create(emu, p(0), u32(4));
#endif
#endif
        case 449:
            //futex_waitv(struct futex_waitv *waiters, unsigned int nr_futexes, unsigned int flags, struct timespec *timeout, clockid_t clockid)
            //This syscall supports only 64bit sized timeout structs
            // the struct futex_waitv is also the same for 32bits and 64bits address space (with 64bits sized address)
            #if defined(__NR_futex_waitv) && !defined(BAD_SIGNAL)
            return syscall(__NR_futex_waitv, p(0), u32(4), u32(8), p(12), u32(16));
            #else
            errno = ENOSYS;
            return -1;
            #endif
        default:
            if((s>>6)<sizeof(warned)/sizeof(warned[0])) {
                if(!(warned[s>>6]&(1<<(s&0x3f)))) {
                    printf_log(LOG_INFO, "Warning: Unsupported libc Syscall 0x%02X (%d)\n", s, s);
                    warned[s>>6] |= (1<<(s&0x3f));
                }
            } else
                printf_log(LOG_INFO, "Warning: Unsupported libc Syscall 0x%02X (%d)\n", s, s);
            errno = ENOSYS;
            return -1;
    }
    return 0;
}
#endif //BOX32
