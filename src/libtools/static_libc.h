#include <inttypes.h>
#include <locale.h>
#include <netinet/in.h>
#include <aliases.h>
#include <arpa/inet.h>
#include <envz.h>
#include <fmtmsg.h>
#include <fnmatch.h>
#include <fstab.h>
#include <grp.h>
#include <gnu/libc-version.h>
#include <iconv.h>
#include <ifaddrs.h>
#include <langinfo.h>
#include <libgen.h>
#include <libintl.h>
#include <mntent.h>
#include <mcheck.h>
#include <netdb.h>
#include <net/if.h>
#include <printf.h>
#include <pwd.h>
#include <regex.h>
#include <semaphore.h>
#include <sys/eventfd.h>
#include <sys/fanotify.h>
#include <sys/file.h>
#include <sys/fsuid.h>
#include <sys/klog.h>
#include <sys/random.h>
#include <sys/inotify.h>
#include <sys/mount.h>
#include <sys/msg.h>
#include <sys/personality.h>
#include <sys/quota.h>
#include <sys/reboot.h>
#include <sys/shm.h>
#include <sys/sendfile.h>
#include <sys/signalfd.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/timeb.h>
#include <sys/times.h>
#include <sys/timex.h>
#include <sys/timerfd.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/xattr.h>
#include <shadow.h>
#include <termios.h>
#include <ttyent.h>
#include <uchar.h>
#include <utime.h>
#include <utmp.h>
#include <utmpx.h>
#include <wctype.h>
#include <wordexp.h>
#include <math.h>

#include "myalign.h"
#include "libtools/static_threads.h"

//extern void* sys_errlist;
//extern void* _sys_errlist;
extern void* __progname_full;
extern void* __progname;
extern void* _IO_list_all;
extern void* _IO_file_jumps;
extern FILE* _IO_2_1_stdout_;
extern FILE* _IO_2_1_stdin_;
extern FILE* _IO_2_1_stderr_;
//extern const unsigned short int *__ctype_b;
extern void* __check_rhosts_file;
typedef struct x64_stack_s x64_stack_t;
extern sighandler_t my_sigset(x64emu_t* emu, int signum, sighandler_t handler);
extern int my_sigaltstack(x64emu_t* emu, const x64_stack_t* ss, x64_stack_t* oss);
int my_obstack_vprintf(x64emu_t* emu, struct obstack* obstack, void* fmt, x64_va_list_t V);
void my__obstack_newchunk(x64emu_t* emu, struct obstack* obstack, int s);
void my_obstack_free(struct obstack * obstack, void* block);
void my__obstack_free(struct obstack * obstack, void* block);
int my__obstack_begin(struct obstack * obstack, size_t size, size_t alignment, void* chunkfun, void* freefun);
extern int __adjtimex(void*);
extern void __assert(void*, void*, int);
extern void __assert_fail(void*, void*, uint32_t, void*);
extern void __assert_perror_fail(int, void*, uint32_t, void*);
extern void __bzero(void*, size_t);
extern int capget(void*, void*);
extern int capset(void*, void*);
void cfree(void* p) {free(p);}
//extern void clnt_pcreateerror(void*);
//extern void clnt_perrno(uint32_t);
//extern void* clnt_spcreateerror(void*);
//extern char* clnt_sperrno(enum clnt_stat);
extern int __close(int);
extern int __connect(int, void*, uint32_t);
extern size_t __confstr_chk(int name, char * buf, size_t len, size_t buflen);
extern double copysign(double, double);
extern float copysignf(float, float);
extern long double copysignl(long double, long double);
extern int __dup2(int, int);
extern void* __duplocale(void*);
extern int __endmntent(void*);
extern void __explicit_bzero_chk(void*, size_t, size_t);
extern void error(int status, int errnum, const char *format, ...);
extern size_t __fbufsize(void*);
extern long int __fdelt_chk(long int);
extern char* __fgets_chk(char*, size_t, int, FILE*);
extern int __finite(double);
extern int finite(double);
extern int __finitef(float);
extern int finitef(float);
extern int __finitel(long double);
extern int finitel(long double);
extern int __flbf(void*);
extern void _flushlbf();
extern size_t __fpending(void*);
extern void __fpurge(void*);
extern int __freadable(void*);
extern size_t __fread_chk(void*, size_t, size_t, size_t, FILE*);
extern int __freading(void*);
extern size_t __fread_unlocked_chk(void*, size_t, size_t, size_t, FILE*);
extern void __freelocale(void*);
extern int __fseeko64(void*, ssize_t, int);
extern int __fsetlocking(void*, int);
extern ssize_t __ftello64(void*);
extern int __fwritable(void*);
extern int __fwriting(void*);
extern char* __getcwd_chk(char*, size_t, size_t);
extern int __getgroups_chk(int, __gid_t*, size_t);
extern void* __getmntent_r(void*, void*, void*, int);
//extern int getnetname(char *);
extern int __getpagesize();
extern int __getpid(void);
//extern int getpublickey (const char *, char *);
extern int __getrlimit(int, void*);
//extern int getrpcport(char *, int, int, int);
//extern int getsecretkey (char *, char *, char *);
extern int __gettimeofday(void*, void*);
extern void* __gmtime_r(void*, void*);
extern uint32_t gnu_dev_major(size_t);
extern size_t gnu_dev_makedev(uint32_t, uint32_t);
extern uint32_t gnu_dev_minor(size_t);
//extern int host2netname(char *, const char *, const char *);
extern int iconvctl(long, int, void*);
extern int _IO_default_doallocate(void*);
extern void _IO_default_finish(void*, int);
extern int _IO_default_pbackfail(void*, int);
extern int _IO_default_uflow(void*);
extern size_t _IO_default_xsgetn(void*, void*, size_t);
extern size_t _IO_default_xsputn(void*, void*, size_t);
extern void _IO_doallocbuf(void*);
extern int _IO_do_write(void*, void*, size_t);
extern void* _IO_file_attach(void*, int);
extern int _IO_file_close(void*);
extern int _IO_file_close_it(void*);
extern int _IO_file_doallocate(void*);
extern void* _IO_file_fopen(void*, void*, void*, int);
extern void _IO_file_init(void*);
extern void* _IO_file_open(void*, void*, int, int, int, int);
extern int _IO_file_overflow(void*, int);
extern ssize_t _IO_file_read(void*, void*, ssize_t);
extern int64_t _IO_file_seek(void*, int64_t, int);
extern int64_t _IO_file_seekoff(void*, int64_t, int, int);
extern void* _IO_file_setbuf(void*, void*, ssize_t);
extern int _IO_file_sync(void*);
extern int _IO_file_underflow(void*);
extern ssize_t _IO_file_write(void*, void*, ssize_t);
extern size_t _IO_file_xsputn(void*, void*, size_t);
extern void _IO_flockfile(void*);
extern int _IO_flush_all();
extern void _IO_flush_all_linebuffered();
extern void _IO_free_backup_area(void*);
extern void _IO_funlockfile(void*);
extern int _IO_getc(void*);
extern size_t _IO_getline_info(void*, void*, size_t, int, int, void*);
extern void _IO_init(void*, void*);
extern void _IO_init_marker(void*, void*);
extern void _IO_link_in(void*);
//extern int ioperm(size_t, size_t, int);
extern int _IO_putc(int, void*);
extern int __iswctype_l(uint32_t, size_t, size_t);
//extern int key_secretkey_is_set(void);
//extern int key_setsecret(const char *);
extern void* __libc_calloc(size_t, size_t);
extern void __libc_free(void*);
extern void* __libc_malloc(size_t);
extern void* __libc_memalign(size_t, size_t);
extern void* __libc_pvalloc(size_t);
extern void* __libc_realloc(void*, size_t);
extern void* __libc_valloc(size_t);
extern int64_t __lseek(int, int64_t, int);
extern int __madvise(void*, size_t, int);
extern size_t __mbrtowc(void*, void*, size_t, void*);
extern size_t __mbsnrtowcs_chk(wchar_t*, const char**, size_t, size_t, mbstate_t*, size_t);
extern size_t __mbsrtowcs_chk(wchar_t*, const char**, size_t, mbstate_t*, size_t);
extern size_t __mbstowcs_chk(wchar_t*, const char*, size_t, size_t);
extern void _mcount(void*, void*);
extern void _mcleanup();
extern void* __memcpy_chk(void*, void*, size_t, size_t);
extern void* __memmove_chk(void*, void*, size_t, size_t);
extern void* __mempcpy_chk(void*, void*, size_t, size_t);
extern void* __memset_chk(void*, int, size_t, size_t);
extern void* __mktemp(void*);
extern int __nanosleep(void*, void*);
//extern int netname2host(char *, char *, const int);
//extern int netname2user(char *, uid_t *, gid_t *, int *, gid_t *);
extern void* __newlocale(int, void*, void*);
extern void __nl_langinfo_l(uint32_t, void*);
extern int __open_2(const char*, int);
extern int __open64_2(const char*, int);
extern int __openat_2(int, const char*, int);
extern int __openat64_2(int, const char*, int);
extern int __pipe(void*);
//extern int pmap_set(size_t, size_t, int, int);
//extern int pmap_unset(size_t, size_t);
extern int __poll(void*, size_t, int);
extern int __poll_chk(struct pollfd*, size_t, int, size_t);
extern int __ppoll_chk(struct pollfd*, size_t, const struct timespec *, const __sigset_t *, size_t);
extern ssize_t __pread64(int, void*, size_t, int64_t);
extern ssize_t __pread_chk(int, void*, size_t, ssize_t, size_t);
#ifndef LA64
extern ssize_t __pread64_chk(int, void*, size_t, ssize_t, size_t);
#endif
//extern void __sF(int, void*);
//extern void __assert2(int, void*);
//extern void pthread_kill_other_threads_np();
extern void* __rawmemchr(void*, int);
extern ssize_t __read(int, void*, size_t);
extern ssize_t __read_chk(int, void*, size_t, size_t);
extern ssize_t __recv(int, void*, size_t, int);
extern ssize_t __recv_chk(int, void*, size_t, size_t, int);
extern void __res_iclose(void*, int);
extern int __res_init();
extern void __res_nclose(void*);
extern int __res_ninit(void*);
extern int res_nsearch(void* statp, const char *dname, int class_, int type, char* answer, int anslen);
extern void* __res_state();
//extern int _rpc_dtablesize();
//extern void* __rpc_thread_svc_max_pollfd();
//extern void* __rpc_thread_svc_pollfd();
extern void* __sbrk(long);
extern double scalbn(double, int);
extern float scalbnf(float, int);
extern long double scalbnl(long double, int);
extern int __sched_getparam(int, void*);
extern int __sched_get_priority_max(int);
extern int __sched_get_priority_min(int);
extern int __sched_yield();
extern int __select(int, void*, void*, void*, void*);
extern ssize_t __send(int, void*, size_t, int);
extern int __sendmmsg(int, void*, uint32_t, int);
extern void* __setmntent(void*, void*);
extern int __setpgid(void*, void*);
int __sigaddset(void* a, int b) {return sigaddset(a, b);}
extern int __signbit(double);
extern int __signbitf(float);
extern int __signbitl(long double);
extern int __sigsuspend(void*);
extern int __sigtimedwait(void*, void*, void*);
extern int __socket(int, int, int);
extern char* __stpcpy_chk(char *, const char *, long unsigned int);
extern char* __stpncpy_chk(char *, const char *, long unsigned int,  long unsigned int);
extern int __strcasecmp(void*, void*);
extern void* __strcasestr(void*, void*);
extern char* __strcat_chk(char *, const char *, long unsigned int);
extern int __strcoll_l(void*, void*, void*);
extern void* __strcpy_chk(char *, const char *, long unsigned int);
extern void* __strdup(void*);
extern void* __strerror_r(int, void*, size_t);
extern long strfmon_l(void*, size_t, void*, void*, ...);
extern size_t __strftime_l(void*, size_t, void*, void*, size_t);
extern size_t __strlcpy_chk(char*, const char*, size_t, size_t);
extern void* __strncat_chk(char *, const char *, long unsigned int,  long unsigned int);
extern void* __strncpy_chk(char *, const char *, long unsigned int,  long unsigned int);
extern void* __strndup(void*, size_t);
extern double __strtod_internal(void*, void*, int);
extern double __strtod_l(void*, void*, void*);
extern float __strtof_internal(void*, void*, int);
extern float __strtof_l(void*, void*, size_t);
extern long double __strtold_internal(void*, void*, int);
extern long double __strtold_l(void*, void*, void*);
extern long __strtol_internal(void*, void*, int, int);
extern long __strtol_l(void*, void*, int);
extern int64_t __strtoll_internal(void*, void*, int, int);
extern int64_t __strtoll_l(void*, void*, int, void*);
extern size_t __strtoul_internal(void*, void*, int, int);
extern uint64_t __strtoull_internal(void*, void*, int, int);
extern size_t __strtoull_l(void*, void*, int, void*);
extern size_t __strxfrm_l(void*, void*, size_t, size_t);
//extern void svc_exit(void);
//extern void svc_getreq(int);
//extern void svc_getreq_common(int);
//extern void svc_getreq_poll(struct pollfd *, int);
//extern void svc_run(void);
//extern void svc_unregister(u_long, u_long);
//extern int __sysctl(void*, int, void*, void*, void*, size_t);
//extern int sysctl(void*, int, void*, void*, void*, size_t);
extern void thrd_exit(void*);
extern int __towlower_l(int, void*);
extern int __towupper_l(int, void*);
extern int __underflow(void*);
extern void* __uselocale(void*);
extern int user2netname(char *, const uid_t, const char *);
extern int __wait(void*);
extern int __waitpid(int, void*, int);
extern size_t __wcrtomb_chk(char * s, wchar_t wchar, mbstate_t * ps, size_t buflen);
extern int __wcscasecmp_l(void*, void*, void*);
extern wchar_t* __wcscat_chk(wchar_t*, const wchar_t*, size_t);
extern int __wcscoll_l(void*, void*, void*);
extern wchar_t* __wcscpy_chk(wchar_t*, const wchar_t*, size_t);
extern size_t __wcsftime_l(void*, size_t, void*, void*, void*);
extern wchar_t* __wcsncat_chk(wchar_t*, const wchar_t*, size_t, size_t);
extern wchar_t*  __wcsncpy_chk(wchar_t*, const wchar_t*, size_t, size_t);
extern size_t __wcsrtombs_chk(char * dest, const wchar_t * * src, size_t len, mbstate_t * ps, size_t destlen);
extern size_t __wcsxfrm_l(void*, void*, size_t, size_t);
extern int __wctomb_chk(char*, wchar_t, size_t);
extern size_t __wctype_l(void*, void*);
extern wchar_t* __wmemcpy_chk(wchar_t*, const wchar_t*, size_t, size_t);
extern wchar_t* __wmemmove_chk(wchar_t*, const wchar_t*, size_t, size_t);
extern wchar_t* __wmemset_chk(wchar_t*, wchar_t, size_t, size_t);
extern ssize_t __write(int, void*, size_t);
extern int __xpg_strerror_r(int __errnum, char *__buf, size_t __buflen);
extern int __xpg_sigpause(int __sig);
extern char *__xpg_basename(char *__path);
//extern int __xmknodat(int, int, void*, uint32_t, void*);
//extern int __xmknod(int, void*, uint32_t, void*);
//extern int xdr_void();
void* dummy_pFLp(size_t a, void* b) {}
void* dummy_pFpLLp(void* a, size_t b, size_t c, void* d) {}
void* dummy__ZnwmSt11align_val_tRKSt9nothrow_t(size_t a, size_t b, void* c) {}
extern void __monstartup(unsigned long, unsigned long);
