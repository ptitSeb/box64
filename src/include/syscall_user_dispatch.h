#ifndef __SYSCALL_USER_DISPATCH_H_
#define __SYSCALL_USER_DISPATCH_H_

#include <stdint.h>

typedef struct x64emu_s x64emu_t;

#ifndef PR_SET_SYSCALL_USER_DISPATCH
#define PR_SET_SYSCALL_USER_DISPATCH 59
#endif

#ifndef PR_SYS_DISPATCH_OFF
#define PR_SYS_DISPATCH_OFF 0
#endif

#ifndef PR_SYS_DISPATCH_ON
#define PR_SYS_DISPATCH_ON 1
#endif

#ifndef PR_SYS_DISPATCH_EXCLUSIVE_ON
#define PR_SYS_DISPATCH_EXCLUSIVE_ON PR_SYS_DISPATCH_ON
#endif

#ifndef PR_SYS_DISPATCH_INCLUSIVE_ON
#define PR_SYS_DISPATCH_INCLUSIVE_ON 2
#endif

#ifndef SYSCALL_DISPATCH_FILTER_ALLOW
#define SYSCALL_DISPATCH_FILTER_ALLOW 0
#endif

#ifndef SYSCALL_DISPATCH_FILTER_BLOCK
#define SYSCALL_DISPATCH_FILTER_BLOCK 1
#endif

int my_syscall_user_dispatch(x64emu_t* emu, uintptr_t call_addr, long syscall_nr, int is32bits);
long my_syscall_user_dispatch_prctl(x64emu_t* emu, unsigned long op, unsigned long offset, unsigned long len, void* selector);

#endif // __SYSCALL_USER_DISPATCH_H_
