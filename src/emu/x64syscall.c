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
#include <sys/resource.h>
#include <poll.h>

#include "debug.h"
#include "box64stack.h"
#include "x64emu.h"
#include "x64run.h"
#include "x64emu_private.h"
#include "x64run_private.h"
//#include "x64primop.h"
#include "x64trace.h"
//#include "myalign.h"
#include "box64context.h"
#include "callback.h"
//#include "signals.h"
#include "x64tls.h"

typedef struct x64_sigaction_s x64_sigaction_t;
typedef struct x64_stack_s x64_stack_t;


//int32_t my_getrandom(x64emu_t* emu, void* buf, uint32_t buflen, uint32_t flags);
int of_convert(int flag);
int32_t my_open(x64emu_t* emu, void* pathname, int32_t flags, uint32_t mode);

int my_sigaction(x64emu_t* emu, int signum, const x64_sigaction_t *act, x64_sigaction_t *oldact);
int my_sigaltstack(x64emu_t* emu, const x64_stack_t* ss, x64_stack_t* oss);
//int32_t my_execve(x64emu_t* emu, const char* path, char* const argv[], char* const envp[]);
void* my_mmap64(x64emu_t* emu, void *addr, unsigned long length, int prot, int flags, int fd, int64_t offset);
int my_munmap(x64emu_t* emu, void* addr, unsigned long length);
int my_mprotect(x64emu_t* emu, void *addr, unsigned long len, int prot);

// cannot include <fcntl.h>, it conflict with some asm includes...
#ifndef O_NONBLOCK
#define O_NONBLOCK 04000
#endif
#undef fcntl
int fcntl(int fd, int cmd, ... /* arg */ );

// Syscall table for x86_64 can be found 
typedef struct scwrap_s {
    int x64s;
    int nats;
    int nbpars;
} scwrap_t;

scwrap_t syscallwrap[] = {
    //{ 0, __NR_read, 3 },      // wrapped so SA_RESTART can be handled by libc
    //{ 1, __NR_write, 3 },     // same
    //{ 2, __NR_open, 3 },      // flags need transformation
    //{ 3, __NR_close, 1 },     // wrapped so SA_RESTART can be handled by libc
    //{ 9, __NR_mmap, 6},       // wrapped to track mmap
    //{ 10, __NR_mprotect, 3},  // same
    //{ 11, __NR_munmap, 2},    // same
    { 5, __NR_fstat, 2},
    //{ 13, __NR_rt_sigaction, 4}   // wrapped to use my_ version
    #ifdef __NR_pipe
    { 22, __NR_pipe, 1},
    #endif
    #ifdef __NR_fork
    { 57, __NR_fork, 0 },    // should wrap this one, because of the struct pt_regs (the only arg)?
    #endif
    //{ 131, __NR_sigaltstack, 2},  // wrapped to use my_sigaltstack
    { 157, __NR_prctl, 5 },     // needs wrapping?
    { 186, __NR_gettid, 0 },    //0xBA
    { 202, __NR_futex, 6},
    { 217, __NR_getdents64, 3},
    #ifdef __NR_inotify_init
    { 253, __NR_inotify_init, 0},   //0xFD
    #endif
    { 254, __NR_inotify_add_watch, 3},
    { 255, __NR_inotify_rm_watch, 2},
    { 294, __NR_inotify_init1, 1},
    { 318, __NR_getrandom, 3},
};

struct mmap_arg_struct {
    unsigned long addr;
    unsigned long len;
    unsigned long prot;
    unsigned long flags;
    unsigned long fd;
    unsigned long offset;
};

//struct x86_pt_regs {
//	long ebx;
//	long ecx;
//	long edx;
//	long esi;
//	long edi;
//	long ebp;
//	long eax;
//	int  xds;
//	int  xes;
//	int  xfs;
//	int  xgs;
//	long orig_eax;
//	long eip;
//	int  xcs;
//	long eflags;
//	long esp;
//	int  xss;
//};

//int clone_fn(void* arg)
//{
//    x64emu_t *emu = (x64emu_t*)arg;
//    R_RAX = 0;
//    DynaRun(emu);
//    int ret = R_EAX;
//    FreeX64Emu(&emu);
//    return ret;
//}

void EXPORT x64Syscall(x64emu_t *emu)
{
    RESET_FLAGS(emu);
    uint32_t s = R_EAX;
    printf_log(LOG_DEBUG, "%p: Calling syscall 0x%02X (%d) %p %p %p %p %p %p", (void*)R_RIP, s, s, (void*)R_RDI, (void*)R_RSI, (void*)R_RDX, (void*)R_R10, (void*)R_R8, (void*)R_R9); 
    // check wrapper first
    int cnt = sizeof(syscallwrap) / sizeof(scwrap_t);
    for (int i=0; i<cnt; i++) {
        if(syscallwrap[i].x64s == s) {
            int sc = syscallwrap[i].nats;
            switch(syscallwrap[i].nbpars) {
                case 0: *(int64_t*)&R_RAX = syscall(sc); break;
                case 1: *(int64_t*)&R_RAX = syscall(sc, R_RDI); break;
                case 2: if(s==33) {printf_log(LOG_DUMP, " => sys_access(\"%s\", %ld)\n", (char*)R_RDI, R_RSI);}; *(int64_t*)&R_RAX = syscall(sc, R_RDI, R_RSI); break;
                case 3: *(int64_t*)&R_RAX = syscall(sc, R_RDI, R_RSI, R_RDX); break;
                case 4: *(int64_t*)&R_RAX = syscall(sc, R_RDI, R_RSI, R_RDX, R_R10); break;
                case 5: *(int64_t*)&R_RAX = syscall(sc, R_RDI, R_RSI, R_RDX, R_R10, R_R8); break;
                case 6: *(int64_t*)&R_RAX = syscall(sc, R_RDI, R_RSI, R_RDX, R_R10, R_R8, R_R9); break;
                default:
                   printf_log(LOG_NONE, "ERROR, Unimplemented syscall wrapper (%d, %d)\n", s, syscallwrap[i].nbpars); 
                   emu->quit = 1;
                   return;
            }
            printf_log(LOG_DEBUG, " => 0x%x\n", R_EAX);
            return;
        }
    }
    switch (s) {
        case 0:  // sys_read
            R_EAX = (uint32_t)read((int)R_EDI, (void*)R_RSI, (size_t)R_RDX);
            break;
        case 1:  // sys_write
            R_EAX = (uint32_t)write((int)R_EDI, (void*)R_RSI, (size_t)R_RDX);
            break;
        case 2: // sys_open
            if(s==5) {printf_log(LOG_DEBUG, " => sys_open(\"%s\", %d, %d)", (char*)R_RDI, of_convert(R_ESI), R_EDX);}; 
            //R_EAX = (uint32_t)open((void*)R_EDI, of_convert(R_ESI), R_EDX);
            R_EAX = (uint32_t)my_open(emu, (void*)R_RDI, of_convert(R_ESI), R_EDX);
            break;
        case 3:  // sys_close
            R_EAX = (uint32_t)close((int)R_EDI);
            break;
        case 9: // sys_mmap
            R_RAX = (uintptr_t)my_mmap64(emu, (void*)R_RDI, R_RSI, (int)R_EDX, (int)R_R10d, (int)R_R8d, R_R9);
            break;
        case 10: // sys_mprotect
            R_EAX = (uint32_t)my_mprotect(emu, (void*)R_RDI, R_RSI, (int)R_EDX);
            break;
        case 11: // sys_munmap
            R_EAX = (uint32_t)my_munmap(emu, (void*)R_RDI, R_RSI);
            break;
        case 13: // sys_rt_sigaction
            R_EAX = (uint32_t)my_sigaction(emu, (int)R_EDI, (const x64_sigaction_t *)R_RSI, (x64_sigaction_t *)R_RDX/*, (size_t)R_R10*/);
            break;
        #ifndef __NR_pipe
        case 22:
            R_EAX = (uint32_t)pipe((void*)R_RDI);
            break;
        #endif
        #ifndef __NR_fork
        case 57: 
            R_RAX = fork();
            break;
        #endif
        case 131: // sys_sigaltstack
            R_EAX = (uint32_t)my_sigaltstack(emu, (void*)R_RDI, (void*)R_RSI);
            break;
        #ifndef __NR_inotify_init
        case 253:
            R_EAX = (int)syscall(__NR_inotify_init1, 0);
            break;
        #endif
        default:
            printf_log(LOG_INFO, "Error: Unsupported Syscall 0x%02Xh (%d)\n", s, s);
            emu->quit = 1;
            emu->error |= ERR_UNIMPL;
            return;
    }
    printf_log(LOG_DEBUG, " => 0x%x\n", R_EAX);
}

#define stack(n) (R_RSP+8+n)
#define i32(n)  *(int32_t*)stack(n)
#define u32(n)  *(uint32_t*)stack(n)
#define i64(n)  *(int64_t*)stack(n)
#define u64(n)  *(uint64_t*)stack(n)
#define p(n)    *(void**)stack(n)

uintptr_t EXPORT my_syscall(x64emu_t *emu)
{
    uint32_t s = R_EDI;;
    printf_log(LOG_DUMP, "%p: Calling libc syscall 0x%02X (%d) %p %p %p %p %p\n", (void*)R_RIP, s, s, (void*)R_RSI, (void*)R_RDX, (void*)R_RCX, (void*)R_R8, (void*)R_R9); 
    // check wrapper first
    int cnt = sizeof(syscallwrap) / sizeof(scwrap_t);
    for (int i=0; i<cnt; i++) {
        if(syscallwrap[i].x64s == s) {
            int sc = syscallwrap[i].nats;
            switch(syscallwrap[i].nbpars) {
                case 0: return syscall(sc);
                case 1: return syscall(sc, R_RSI);
                case 2: return syscall(sc, R_RSI, R_RDX);
                case 3: return syscall(sc, R_RSI, R_RDX, R_RCX);
                case 4: return syscall(sc, R_RSI, R_RDX, R_RCX, R_R8);
                case 5: return syscall(sc, R_RSI, R_RDX, R_RCX, R_R8, R_R9);
                case 6: return syscall(sc, R_RSI, R_RDX, R_RCX, R_R8, R_R9, u64(0));
                default:
                   printf_log(LOG_NONE, "ERROR, Unimplemented syscall wrapper (%d, %d)\n", s, syscallwrap[i].nbpars); 
                   emu->quit = 1;
                   return 0;
            }
        }
    }
    switch (s) {
        case 0:  // sys_read
            return (uint32_t)read(R_ESI, (void*)R_RDX, R_ECX);
        case 1:  // sys_write
            return (uint32_t)write(R_ESI, (void*)R_RDX, R_ECX);
        case 2: // sys_open
            return my_open(emu, (char*)R_RSI, of_convert(R_EDX), R_ECX);
        case 3:  // sys_close
            return (uint32_t)close(R_ESI);
        case 9: // sys_mmap
            return (uintptr_t)my_mmap64(emu, (void*)R_RSI, R_RDX, (int)R_RCX, (int)R_R8d, (int)R_R9, i64(0));
        case 10: // sys_mprotect
            return (uint32_t)my_mprotect(emu, (void*)R_RSI, R_RDX, (int)R_ECX);
        case 11: // sys_munmap
            return (uint32_t)my_munmap(emu, (void*)R_RSI, R_RDX);
        case 13: // sys_rt_sigaction
            return (uint32_t)my_sigaction(emu, (int)R_ESI, (const x64_sigaction_t *)R_RDX, (x64_sigaction_t *)R_RCX/*, (size_t)R_R8*/);
        #ifndef __NR_pipe
        case 22:
            return (uint32_t)pipe((void*)R_RSI);
        #endif
        #ifndef __NR_fork
        case 57: 
            return fork();
        #endif
        case 131: // sys_sigaltstack
            return (uint32_t)my_sigaltstack(emu, (void*)R_RSI, (void*)R_RDX);
        #ifndef __NR_inotify_init
        case 253:
            return (int)syscall(__NR_inotify_init1, 0);
        #endif
        default:
            printf_log(LOG_INFO, "Error: Unsupported libc Syscall 0x%02X (%d)\n", s, s);
            emu->quit = 1;
            emu->error |= ERR_UNIMPL;
    }
    return 0;
}
