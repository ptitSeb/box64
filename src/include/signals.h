#ifndef __SIGNALS_H__
#define __SIGNALS_H__
#include <signal.h>

typedef void (*sighandler_t)(int);

typedef struct x64_sigaction_s {
	union {
	  sighandler_t _sa_handler;
	  void (*_sa_sigaction)(int, siginfo_t *, void *);
	} _u;
	sigset_t sa_mask;
	uint32_t sa_flags;
	void (*sa_restorer)(void);
} x64_sigaction_t;

typedef struct x64_sigaction_restorer_s {
	union {
	  sighandler_t _sa_handler;
	  void (*_sa_sigaction)(int, siginfo_t *, void *);
	} _u;
	uint32_t sa_flags;
	void (*sa_restorer)(void);
	sigset_t sa_mask;
} x64_sigaction_restorer_t;

sighandler_t my_signal(x64emu_t* emu, int signum, sighandler_t handler);
sighandler_t my___sysv_signal(x64emu_t* emu, int signum, sighandler_t handler);
sighandler_t my_sysv_signal(x64emu_t* emu, int signum, sighandler_t handler);

int my_sigaction(x64emu_t* emu, int signum, const x64_sigaction_t *act, x64_sigaction_t *oldact);
int my___sigaction(x64emu_t* emu, int signum, const x64_sigaction_t *act, x64_sigaction_t *oldact);

int my_syscall_rt_sigaction(x64emu_t* emu, int signum, const x64_sigaction_restorer_t *act, x64_sigaction_restorer_t *oldact, int sigsetsize);

void init_signal_helper(box64context_t* context);
void fini_signal_helper(void);

void emit_signal(x64emu_t* emu, int sig, void* addr, int code);
void emit_interruption(x64emu_t* emu, int num, void* addr);
void emit_div0(x64emu_t* emu, void* addr, int code);

#endif //__SIGNALS_H__
