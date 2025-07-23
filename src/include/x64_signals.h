#ifndef __X64_SIGNALS_H__
#define __X64_SIGNALS_H__

#include <signal.h>

#define X64_SIGHUP           1
#define X64_SIGINT           2
#define X64_SIGQUIT          3
#define X64_SIGILL           4
#define X64_SIGTRAP          5
#define X64_SIGABRT          6
#define X64_SIGIOT           6
#define X64_SIGBUS           7
#define X64_SIGFPE           8
#define X64_SIGKILL          9
#define X64_SIGUSR1         10
#define X64_SIGSEGV         11
#define X64_SIGUSR2         12
#define X64_SIGPIPE         13
#define X64_SIGALRM         14
#define X64_SIGTERM         15
#define X64_SIGSTKFLT       16
#define X64_SIGCHLD         17
#define X64_SIGCONT         18
#define X64_SIGSTOP         19
#define X64_SIGTSTP         20
#define X64_SIGTTIN         21
#define X64_SIGTTOU         22
#define X64_SIGURG          23
#define X64_SIGXCPU         24
#define X64_SIGXFSZ         25
#define X64_SIGVTALRM       26
#define X64_SIGPROF         27
#define X64_SIGWINCH        28
#define X64_SIGIO           29
#define X64_SIGPWR          30
#define X64_SIGSYS          31

#if !defined(NEED_SIG_CONV) && X64_SIGHUP != SIGHUP
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGINT != SIGINT
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGQUIT != SIGQUIT
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGILL != SIGILL
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGTRAP != SIGTRAP
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGABRT != SIGABRT
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGIOT != SIGIOT
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGBUS != SIGBUS
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGFPE != SIGFPE
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGKILL != SIGKILL
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGUSR1 != SIGUSR1
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGSEGV != SIGSEGV
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGUSR2 != SIGUSR2
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGPIPE != SIGPIPE
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGALRM != SIGALRM
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGTERM != SIGTERM
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGSTKFLT != SIGSTKFLT
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGCHLD != SIGCHLD
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGCONT != SIGCONT
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGSTOP != SIGSTOP
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGTSTP != SIGTSTP
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGTTIN != SIGTTIN
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGTTOU != SIGTTOU
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGURG != SIGURG
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGXCPU != SIGXCPU
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGXFSZ != SIGXFSZ
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGVTALRM != SIGVTALRM
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGPROF != SIGPROF
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGWINCH != SIGWINCH
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGIO != SIGIO
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGPWR != SIGPWR
 #define NEED_SIG_CONV
#endif
#if !defined(NEED_SIG_CONV) && X64_SIGSYS != SIGSYS
 #define NEED_SIG_CONV
#endif

#ifdef NEED_SIG_CONV
int signal_to_x64(int sig);
int signal_from_x64(int sig);
#define SUPER_SIGNAL    \
GO(SIGHUP)      \
GO(SIGINT)      \
GO(SIGQUIT)     \
GO(SIGILL)      \
GO(SIGTRAP)     \
GO(SIGABRT)     \
GO(SIGIOT)      \
GO(SIGBUS)      \
GO(SIGFPE)      \
GO(SIGKILL)     \
GO(SIGUSR1)     \
GO(SIGSEGV)     \
GO(SIGUSR2)     \
GO(SIGPIPE)     \
GO(SIGALRM)     \
GO(SIGTERM)     \
GO(SIGSTKFLT)   \
GO(SIGCHLD)     \
GO(SIGCONT)     \
GO(SIGSTOP)     \
GO(SIGTSTP)     \
GO(SIGTTIN)     \
GO(SIGTTOU)     \
GO(SIGURG)      \
GO(SIGXCPU)     \
GO(SIGXFSZ)     \
GO(SIGVTALRM)   \
GO(SIGPROF)     \
GO(SIGWINCH)    \
GO(SIGIO)       \
GO(SIGPWR)      \
GO(SIGSYS)
#else
#define signal_to_x64(A)    A
#define signal_from_x64(A)  A
#endif

#endif //__X64_SIGNALS_H__