#define _GNU_SOURCE 1
#define __USE_MISC 1
#define PORTMAP

#include <stdint.h>

#include <aliases.h>
#include <argp.h>
#include <argz.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <asm/prctl.h>
#include <complex.h>
#include <ctype.h>
#include <dirent.h>
#include <envz.h>
#include <err.h>
#include <error.h>
#include <execinfo.h>
#include <event.h>
#include <fcntl.h>
#include <fmtmsg.h>
#include <fnmatch.h>
#include <fstab.h>
#include <fts.h>
#include <ftw.h>
#include <gconv.h>
#include <glob.h>
#include <gnu/libc-version.h>
#include <grp.h>
#include <gshadow.h>
#include <iconv.h>
#include <ifaddrs.h>
#include <inttypes.h>
#include <langinfo.h>
#include <libgen.h>
#include <libintl.h>
#include <link.h>
#include <linux/module.h>
#include <locale.h>
#include <math.h>
#include <malloc.h>
#include <mcheck.h>
#include <mntent.h>
#include <monetary.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <nl_types.h>
#include <obstack.h>
#include <poll.h>
#include <printf.h>
#include <pwd.h>
#include <regex.h>
#include <resolv.h>
#include <rpc/auth.h>
#include <rpc/des_crypt.h>
#include <rpc/key_prot.h>
#include <rpc/rpc.h>
#include <sched.h>
#include <search.h>
#include <setjmp.h>
#include <shadow.h>
#include <signal.h>
#include <spawn.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/auxv.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/fanotify.h>
#include <sys/file.h>
#include <sys/fsuid.h>
#include <sys/inotify.h>
#include <sys/io.h>
#include <sys/ipc.h>
#include <sys/klog.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/msg.h>
#include <sys/personality.h>
#include <sys/prctl.h>
#include <sys/profil.h>
#include <sys/ptrace.h>
#include <sys/quota.h>
#include <sys/random.h>
#include <sys/reboot.h>
#include <sys/resource.h>
#include <sys/sem.h>
#include <sys/sendfile.h>
#include <sys/shm.h>
#include <sys/signal.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <sys/swap.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <sys/syslog.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/timerfd.h>
#include <sys/times.h>
#include <sys/timex.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/vfs.h>
#include <sys/xattr.h>
#include <syslog.h>
#include <termios.h>
#include <time.h>
#include <threads.h>
#include <ttyent.h>
#include <uchar.h>
#include <ucontext.h>
#include <ulimit.h>
#include <unistd.h>
#include <unistdio.h>
#include <utime.h>
#include <utmp.h>
#include <utmpx.h>
#include <wait.h>
#include <wchar.h>
#include <wctype.h>
#include <wordexp.h>

#pragma wrappers type_letters S FILE*
#pragma wrappers type_letters S const FILE*
#pragma wrappers type_letters S FILE* restrict
#pragma wrappers type_letters S const FILE* restrict
#pragma wrappers type_letters p FTS*
#pragma wrappers type_letters p const FTS*
#pragma wrappers type_letters p FTS64*
#pragma wrappers type_letters p const FTS64*
#pragma wrappers type_letters p glob_t*
#pragma wrappers type_letters p const glob_t*
#pragma wrappers type_letters p glob64_t*
#pragma wrappers type_letters p const glob64_t*
