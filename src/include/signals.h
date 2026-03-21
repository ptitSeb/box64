#ifndef __SIGNALS_H__
#define __SIGNALS_H__
#include <stdint.h>

#include "x64_signals.h"
#include "box64context.h"

typedef void (*sighandler_t)(int);

#ifdef ANDROID
typedef struct x64_sigaction_s {
	int sa_flags;
	union {
	  sighandler_t _sa_handler;
	  void (*_sa_sigaction)(int, siginfo_t *, void *);
	} _u;
	sigset_t sa_mask;
	void (*sa_restorer)(void);
} x64_sigaction_t;
#else
typedef struct x64_sigaction_s {
	union {
	  sighandler_t _sa_handler;
	  void (*_sa_sigaction)(int, siginfo_t *, void *);
	} _u;
	sigset_t sa_mask;
	uint32_t sa_flags;
	void (*sa_restorer)(void);
} x64_sigaction_t;
#endif

typedef struct x64_sigaction_restorer_s {
	union {
	  sighandler_t _sa_handler;
	  void (*_sa_sigaction)(int, siginfo_t *, void *);
	} _u;
	uint32_t sa_flags;
	void (*sa_restorer)(void);
	sigset_t sa_mask;
} x64_sigaction_restorer_t;

#ifdef BOX32
typedef struct __attribute__((packed)) i386_sigaction_s {
	union {
	  ptr_t _sa_handler;	// sighandler_t
	  ptr_t _sa_sigaction; //void (*_sa_sigaction)(int, siginfo_t *, void *);
	} _u;
	sigset_t sa_mask;
	uint32_t sa_flags;
	ptr_t sa_restorer; //void (*sa_restorer)(void);
} i386_sigaction_t;

typedef struct __attribute__((packed)) i386_sigaction_restorer_s {
	union {
	  ptr_t _sa_handler;	//sighandler_t
	  ptr_t _sa_sigaction; //void (*_sa_sigaction)(int, siginfo_t *, void *);
	} _u;
	uint32_t sa_flags;
	ptr_t sa_restorer; //void (*sa_restorer)(void);
	sigset_t sa_mask;
} i386_sigaction_restorer_t;

#endif

sighandler_t my_signal(x64emu_t* emu, int signum, sighandler_t handler);
sighandler_t my___sysv_signal(x64emu_t* emu, int signum, sighandler_t handler);
sighandler_t my_sysv_signal(x64emu_t* emu, int signum, sighandler_t handler);

int my_sigaction(x64emu_t* emu, int signum, const x64_sigaction_t *act, x64_sigaction_t *oldact);
int my___sigaction(x64emu_t* emu, int signum, const x64_sigaction_t *act, x64_sigaction_t *oldact);

int my_syscall_rt_sigaction(x64emu_t* emu, int signum, const x64_sigaction_restorer_t *act, x64_sigaction_restorer_t *oldact, int sigsetsize);

void init_signal_helper(box64context_t* context);
void fini_signal_helper(void);
void setupNativeAltStack(void);

#endif //__SIGNALS_H__
