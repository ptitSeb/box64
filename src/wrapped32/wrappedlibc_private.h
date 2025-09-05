#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA) && defined(GOS))
#error Meh...
#endif

//DATA not M cannot work on libc wbecause it's loaded in high memory

// FILE* is S
// locale_t needs special handling, with to_locale / from_locale (and is a / A)
// struct utimbuf is: LL
// struct timespec is: LL
// struct timeval is: LL
// struct itimerval is LLLL
// struct tm is: iiiiiiiiilt
// struct tms is: llll
// struct rusage is LLLLLLLLLLLLLLLLLL (2 timeval + 14 longs)
// time_t is: L
// socklen_t is u
// struct sockaddr is fine, no wrap needed
// wctype_t is h (used with locale_t)

// a64l
GO(abort, vEv)
GO(abs, iEi)
GOW(accept, iEipp)
GOW(accept4, iEippi)
GOW(access, iEpi)
// acct
//GOW(addmntent, iEpp)
// addseverity
// adjtime  // Weak
// adjtimex // Weak
// __adjtimex
// advance  // Weak
// __aeabi_assert
// __aeabi_atexit
// __aeabi_errno_addr
// __aeabi_localeconv
// __aeabi_MB_CUR_MAX
// __aeabi_memclr
// __aeabi_memclr4
// __aeabi_memclr8
// __aeabi_memcpy
// __aeabi_memcpy4
// __aeabi_memcpy8
// __aeabi_memmove
// __aeabi_memmove4
// __aeabi_memmove8
// __aeabi_memset
// __aeabi_memset4
// __aeabi_memset8
//DATAV(__after_morecore_hook, 4)
GO(alarm, uEu)
GO2(aligned_alloc, pELL, memalign)
GO(alphasort, iEpp)
GOM(alphasort64, iEEpp)
//DATA(argp_err_exit_status, 4)
// argp_error   // Weak
// argp_failure // Weak
// argp_help    // Weak
// argp_parse   // Weak
// argp_program_bug_address // type B
// argp_program_version // type B
// argp_program_version_hook    // type B
// argp_state_help  // Weak
// argp_usage   // Weak
// argz_add // Weak
// argz_add_sep // Weak
// argz_append  // Weak
// __argz_count
// argz_count   // Weak
// argz_create  // Weak
//GOW(argz_create_sep, iEpipp)
// argz_delete
// argz_extract // Weak
//GOW(argz_insert, iEpppp)
// __argz_next
//GOW(argz_next, pEpLp)
// argz_replace // Weak
// __argz_stringify
//GOW(argz_stringify, vEpLi)
GOM(asctime, pEriiiiiiiiilt_)   //%noE
GOW(asctime_r, pEriiiiiiiiilt_p)
GOWM(asprintf, iEEppV)        //%%
//GOM(__asprintf, iEEppV)      //%%
GOM(__asprintf_chk, iEEpipV) //%%
// __assert
GO(__assert_fail, vEppip)
//GO(__assert_perror_fail, vEipup)
GO(atof, dEp)
GO(atoi, iEp)
GO2(atol, lEp, atoi)
GO(atoll, IEp)
// authdes_create
// authdes_getucred
// authdes_pk_create
// _authenticate
// authnone_create
// authunix_create
// authunix_create_default
GOWM(backtrace, iEEpi)
//GO(__backtrace, iEpi)
//GO(__backtrace_symbols, pEpi)
GOWM(backtrace_symbols, pEEpi)
//GO(__backtrace_symbols_fd, vEpii)
//GOW(backtrace_symbols_fd, vEpii)
GO(basename, pEp)
GOW(bcmp, iEppL)
//GO(bcopy, vEppL)
// bdflush
GOW(bind, iEipu)
// bindresvport
GOW(bindtextdomain, tEpp)
GOW(bind_textdomain_codeset, tEpp)
//GOW(brk, iEp)
// __bsd_getpgrp
// bsd_signal   // Weak
GOM(bsearch, pEEppLLp) //%%
GOW(btowc, uEi)
GOW(bzero, vEpL)
GO(__bzero, vEpL)
GOW(calloc, pELL)
// callrpc
//GOW(canonicalize_file_name, pEp)
// capget
// capset
//GO(catclose, iEp)
//GO(catgets, pEpiip)
//GO(catopen, pEpi)
// cbc_crypt
GO(cfgetispeed, uEp)
GO(cfgetospeed, uEp)
//GO(cfmakeraw, vEp)
//GOW(cfree, vEp)
GO(cfsetispeed, iEpu)
GO(cfsetospeed, iEpu)
GO(cfsetspeed, iEpi)
GOW(chdir, iEp)
//DATA(__check_rhosts_file, 4)
// chflags
// __chk_fail
GOW(chmod, iEpu)
GOW(chown, iEpuu)
GO(chroot, iEp)
GOW(clearenv, iEv)
GO(clearerr, vES)
//GO(clearerr_unlocked, vEp)
// clnt_broadcast
// clnt_create
// clnt_pcreateerror
// clnt_perrno
// clnt_perror
// clntraw_create
// clnt_spcreateerror
// clnt_sperrno
// clnt_sperror
// clnttcp_create
// clntudp_bufcreate
// clntudp_create
// clntunix_create
GO(clock, lEv)
// clone    // Weak
// __clone
GOW(close, iEi)
// __close  // Weak
GOW(closedir, iEp)
GO(closelog, vEv)
GOM(__cmsg_nxthdr, pEpp)    //%noE
//GO(confstr, uEipu)
// __confstr_chk
GOW(connect, iEipu)
GOW(__connect, iEipu)
// copysign // Weak
// copysignf    // Weak
// copysignl    // Weak
GOW(creat, iEpu)
GO(creat64, iEpu)
// create_module    // Weak
GO(ctermid, tEp)
GOM(ctime, pErl_)   //%noE
GO2(__ctime64, pEp, ctime)
GO(ctime_r, pErl_p)
DATAM(__ctype_b, 4)
GOM(__ctype_b_loc, pEEv)
GOW(__ctype_get_mb_cur_max, LEv)
DATAM(__ctype_tolower, 4)
GOM(__ctype_tolower_loc, pEEv)
DATAM(__ctype_toupper, 4)
GOM(__ctype_toupper_loc, pEEv)
// __curbrk // type B
GO(cuserid, tEp)
GOM(__cxa_atexit, iEEppp) //%%
GOM(atexit, iEEp)           //%% just in case
GOM(__cxa_finalize, vEEp) //%%
DATAM(__cpu_model, 16)
GOM(__cxa_thread_atexit_impl, iEEppp) //%%
// __cyg_profile_func_enter
// __cyg_profile_func_exit
// daemon
DATAV(daylight, 4)
// __daylight   // type B
GOW(dcgettext, tEppi)
GO(__dcgettext, tEppi)
GOW(dcngettext, tEpppLi)
// __default_morecore
// __default_rt_sa_restorer_v1
// __default_rt_sa_restorer_v2
// __default_sa_restorer_v1
// __default_sa_restorer_v2
// delete_module
// des_setparity
GOW(dgettext, pEpp)
GO(__dgettext, pEpp)
GO(difftime, dEll)
GO(dirfd, iEp)
GO(dirname, tEp)
GOS(div, pEpii) //%%,noE
// _dl_addr
GOW2(dl_iterate_phdr, iEEpp, my_dl_iterate_phdr) //%%
// _dl_mcount_wrapper
// _dl_mcount_wrapper_check
// _dl_open_hook    // type B
// _dl_starting_up // Weak
// _dl_sym
// _dl_vsym
//GOW(dngettext, pEpppu)
//GOM(dprintf, iEEipV)
GOM(__dprintf_chk, iEEiipV)	//%%
GO(drand48, dEv)
// drand48_r
GOW(dup, iEi)
GOW(dup2, iEii)
GO(__dup2, iEii)
GO(dup3, iEiiO)
//GOW(duplocale, pEp)
GO(__duplocale, aEa)
// dysize
//GOW(eaccess, iEpi)
// ecb_crypt
// ecvt
//GO(ecvt_r, iEdipppL)
// endaliasent
// endfsent
GO(endgrent, vEv)
GO(endhostent, vFv)
GOW(endmntent, iES)
// __endmntent
// endnetent
// endnetgrent
GO(endprotoent, vEv)
GO(endpwent, vEv)
// endrpcent
GO(endservent, vEv)
GO(endspent, vEv)
// endttyent
// endusershell
GOW(endutent, vEv)
// endutxent
DATAM(environ, 4)
DATAM(_environ, 4)
DATAM(__environ, 4)    // type B
// envz_add
// envz_entry
// envz_get
// envz_merge
// envz_remove
// envz_strip
GOM(epoll_create, iEEi)     //%% not needed, but used in syscall
GOM(epoll_create1, iEEO)    //%%
GOM(epoll_ctl, iEEiiip)     //%% align epool_event structure
// epoll_pwait
GOM(epoll_wait, iEEipii)    //%% need realign of epoll_event structure
// erand48
// erand48_r    // Weak
//GO(err, vEippppppppp)
// errno    // type B
GOM(__errno_location, pEEv)
//GOW(error, vEiippppppppp)  // Simple attempt: there is a vararg, but the alignment will/may be off if it tries some Double in the "printf" part
// error_at_line    // Weak
// error_message_count  // type B
// error_one_per_line   // type B
// error_print_progname // type B
//GO(errx, vEippppppppp)
//GO(ether_aton, pEp)
//GO(ether_aton_r, pEpp)
//GO(ether_hostton, iEpp)
//GO(ether_line, iEppp)
//GO(ether_ntoa, pEp)
//GO(ether_ntoa_r, pEpp)
//GO(ether_ntohost, iEpp)
//GOW(euidaccess, iEpi)
GO(eventfd, iEui)
//GO(eventfd_read, iEip)
//GO(eventfd_write, iEiU)
GO2(execl, iEEpV, my32_execv)
//GO2(execle, iEEpV, my32_execve)  // Nope! This one needs wrapping, because is char*, char*, ..., char*[]
GO2(execlp, iEpV, my32_execvp)
GOWM(execv, iEEpp)      //%%
GOM(execve, iEEppp)     //%% and this one too...
GOWM(execvp, iEEpp)     //%%
GOWM(execvpe, iEEppp)   //%%
GO2(exit, vFEi, my_exit)
GO(_exit, vEi)
GOW(_Exit, vEi)
GOM(__explicit_bzero_chk, vEEpuu)    //%% not always defined
GO(faccessat, iEipii)
// fattach
//GO(__fbufsize, uEp)
GOW(fchdir, iEi)
// fchflags
GOW(fchmod, iEiu)
GO(fchmodat, iEipui)
GOW(fchown, iEiuu)
//GO(fchownat, iEipuii)
GO(fclose, iES)
GOW(fcloseall, iEv)
GOM(fcntl, iEEiiN)   //%% this also use a vararg for 3rd argument
GOM(__fcntl, iEEiiN) //%%
GO2(__fcntl_time64, iEiiN, my_fcntl)
GOM(fcntl64, iEEiiN) //%%
//GO(fcvt, pEdipp)
//GO(fcvt_r, iEdipppL)
GO(fdatasync, iEi)
// fdetach
GO(fdopen, SEip)
GOW(fdopendir, pEi)
GOW(feof, iES)
//GO(feof_unlocked, iEp)
GOW(ferror, iES)
//GO(ferror_unlocked, iEp)
//GO(fexecve, iEipp)  //TODO: Check if needed to be wrapped, and target checked for x86 / native?
GOW(fflush, iES)
//GO(fflush_unlocked, iES)
GO(ffs, iEi)
// __ffs
GOW(ffsl, iEl)
GO(ffsll, iEI)
GOW(fgetc, iES)
GOW(fgetc_unlocked, iES)
// fgetgrent
// fgetgrent_r  // Weak
GO(fgetpos, iESBliu_)
GO(fgetpos64, iESp)
// fgetpwent
// fgetpwent_r  // Weak
GOW(fgets, pEpiS)
GO(__fgets_chk, pEpLiS)
// fgetspent
// fgetspent_r  // Weak
//GO(fgets_unlocked, pEpip)
// __fgets_unlocked_chk
//GOW(fgetwc, iEp)
//GOW(fgetwc_unlocked, iEp)
//GO(fgetws, pEpip)
// __fgetws_chk
//GO(fgetws_unlocked, pEpip)
// __fgetws_unlocked_chk
GO(fgetxattr, iEippL)
GO(fileno, iES)
GOW(fileno_unlocked, iES)
GOW(finite, iEd)
GO(__finite, iEd)
GOW(finitef, iEf)
// __finitef
// finitel  // Weak
// __finitel
// __flbf
GO(flistxattr, iEipL)
GOW(flock, iEii)
GOW(flockfile, vFS)
GOW(_flushlbf, vEv)
//GO(fmemopen, pEpup)
// fmtmsg
GO(fnmatch, iEppi)
GO2(fopen, SEEpp, my_fopen)           //%%
GOW2(fopen64, SEEpp, my_fopen64)         //%%
//GOM(fopencookie, pEEpppppp) //%% last 4p are a struct with 4 callbacks...
GOWM(fork, iEEv)             //%%
GOM(__fork, iEEv)           //%%
// __fortify_fail
GOW(fpathconf, lEii)
//GO(__fpending, uEp)
GOM(fprintf, iEESpV) //%%
GOM(__fprintf_chk, iEESipV) //%%
// __fpu_control    // type B
//GO(__fpurge, vEp)
GOW(fputc, iEiS)
//GO(fputc_unlocked, iEip)
GOW(fputs, iEpS)    // Weak
//GO(fputs_unlocked, iEpp)
GO(fputwc, uEiS)
//GO(fputwc_unlocked, iEip)
//GO(fputws, iEpp)
//GO(fputws_unlocked, iEpp)
GOW(fread, LEpLLS)
//GO(__freadable, iEp)
GO(__fread_chk, uEpLLLS)
//GO(__freading, iEp)
//GO(fread_unlocked, uEpuup)
//GO(__fread_unlocked_chk, uEpuuup)
GO(free, vEp)
GOM(freeaddrinfo, vEEp)
//DATAV(__free_hook, 4)
GO(freeifaddrs, vEp)
GOW(freelocale, vEA)
GO(__freelocale, vEA)
GO(fremovexattr, iEip)
GO(freopen, SEppS)
GO(freopen64, SEppS)
GOW(frexp, dFdp)
// frexpf   // Weak
#ifdef HAVE_LD80BITS
GOW(frexpl, DFDp)
#else
GOW2(frexpl, KFKp, frexp)
#endif
GOM(fscanf, iEESpV)
GO(fseek, iESli)
GO(fseeko, iESli)
GO(fseeko64, iESIi)
GO(__fsetlocking, iESi)
GO(fsetpos, iEpBlii_)
//GO(fsetpos64, iEpp)
GO(fsetxattr, iEippLi)
GOM(fstat, iFip)    //%%,noE
GO2(__fstat64_time64, iFip, fstat)
GO2(__fstatat64_time64, iEippi, fstatat)
GOWM(fstatfs, iEip) //%%,noE
GOWM(fstatfs64, iEip)    //%%,noE
GOM(fstatvfs, iEEip)
GOWM(fstatvfs64, iEEip)
GOW(fsync, iEi)
GOWM(ftell, lEES)
GO(ftello, lES)
GO(ftello64, IES)
GO(ftime, iErLWww_)
GO(ftok, iEpi)
GOW(ftruncate, iEil)
GOW(ftruncate64, iEiI)
//GOW(ftrylockfile, iEp)
//GOM(fts_children, pEEpi) //%%
//GOM(fts_close, iEEp)     //%%
//GOM(fts_open, pEEpip)    //%%
//GOM(fts_read, pEEp)      //%%
// fts_set
GOM(ftw, iEEppi)         //%%
GOM(ftw64, iEEppi)       //%%
GOW(funlockfile, vFS)
GOM(futimens, iEEip)
GO2(__futimens64, iEip, futimens)
GOWM(futimes, iEEip)
//GO(futimesat, iEippp)
// fwide
//GOWM(fwprintf, iEEppV)   //%%
//GOM(__fwprintf_chk, iEEpvpV) //%%
//GO(__fwritable, iEp)
GOW(fwrite, LEpLLS)
//GO(fwrite_unlocked, uEpuup)
//GO(__fwriting, iEp)
// fwscanf
GOM(__fxstat, iEEiip)       //%%
GOM(__fxstat64, iEEiip)     //%% need reaalign of struct stat64
//GOM(__fxstatat, iEEiippi)   //%%
GOM(__fxstatat64, iEEiippi) //%% struct stat64 again
// __gai_sigqueue
GO(gai_strerror, tEi)
// __gconv_get_alias_db
// __gconv_get_cache
// __gconv_get_modules_db
// gcvt
GOM(getaddrinfo, iEEpppp)
// getaliasbyname
// getaliasbyname_r
// getaliasent
// getaliasent_r
// get_avphys_pages // Weak
GOW(getc, iES)
GOW(getchar, iEv)
GO(getchar_unlocked, iEv)
GOM(getcontext, iEEp)         //%%
//GOW(getc_unlocked, iEp)
GO(get_current_dir_name, pEv)
GOW(getcwd, tEpL)
//GO(__getcwd_chk, pEpLL)
//GO(getdate, pEp)
// getdate_err  // type B
// getdate_r    // Weak
//GOW(getdelim, iEbp_bL_iS)
GOW(__getdelim, iEbp_bL_iS)
// getdirentries
// getdirentries64
//GO(getdomainname, iEpu)
// __getdomainname_chk
GOW(getdtablesize, iEv)
GOW(getegid, uEv)
GO(getenv, tEp)
GOW(geteuid, uEv)
// getfsent
// getfsfile
// getfsspec
GOW(getgid, uEv)
//GO(getgrent, pEv)
// getgrent_r
//GO(getgrgid, pEu)
GOM(getgrgid_r, iEEuppLp)
GOM(getgrnam, pEEp)
GOM(getgrnam_r, iEEpppLp)
GO(getgrouplist, iEpipp)
GOW(getgroups, iEip)
// __getgroups_chk
GOM(gethostbyaddr, pFEpui)
GOM(gethostbyaddr_r, iFEpuippupp)
GOM(gethostbyname, pFEp)
//GO(gethostbyname2, pFpi)
//GO(gethostbyname2_r, iFpippupp)
GOM(gethostbyname_r, iFEpppupp)
//GO(gethostent, pFv)
//GO(gethostent_r, iFppupp)
GO(gethostid, LFv)
GOW(gethostname, iEpL)
// __gethostname_chk
GOM(getifaddrs, iEEbp_)
// getipv4sourcefilter
//GOW(getitimer, iEip)
// get_kernel_syms  // Weak
//GOW(getline, iEppp)
GO(getloadavg, iEpi)
GO(getlogin, pEv)
GO(getlogin_r, iEpL)
// __getlogin_r_chk
GOM(getmntent, pEES)
// __getmntent_r
GOW(getmntent_r, pESBppppii_pi)
// getmsg
// get_myaddress
GO(getnameinfo, iEpupupui)
// getnetbyaddr
// getnetbyaddr_r
// getnetbyname
// getnetbyname_r
// getnetent
// getnetent_r
// getnetgrent
// getnetgrent_r    // Weak
// getnetname
GOW(get_nprocs, iEv)
//GOW(get_nprocs_conf, iEv)
//GOM(getopt, iEipp)             //%noE
GOM(getopt_long, iEipppp)      //%noE
//GOM(getopt_long_only, iEipppp) //%noE
GOW(getpagesize, iEv)
GO(__getpagesize, iEv)
GO(getpass, tEp)
GOW(getpeername, iEipp)
GOW(getpgid, iEi)
// __getpgid
GO(getpgrp, iEv)
// get_phys_pages   // Weak
GO(getpid, iEv)
GO(__getpid, uEv)
// getpmsg
GOW(getppid, iEv)
GO(getpriority, iEuu)
GO(getrandom, iEpLu)
GOM(getprotobyname, pEEp)
//GO(getprotobyname_r, iEpppup)
//GO(getprotobynumber, pEi)
//GO(getprotobynumber_r, iEippup)
//GO(getprotoent, pEv)
//GO(getprotoent_r, iEppup)
GOW(getpt, iEv)
// getpublickey
// getpw    // Weak
GOM(getpwent, pEEv)
// getpwent_r
GOM(getpwnam, pEEp)
GOM(getpwnam_r, iEEpppLp)
GOM(getpwuid, pEEu)
GOM(getpwuid_r, iEEuppLp)
GOW(getresgid, iEppp)
GOW(getresuid, iEppp)
GOM(getrlimit, iEEup)
GO(getrlimit64, iEup)
// getrpcbyname
// getrpcbyname_r
// getrpcbynumber
// getrpcbynumber_r
// getrpcent
// getrpcent_r
// getrpcport
GOW(getrusage, iEiBLLLLLLLLLLLLLLLLLL_)
//GOW(gets, pEp)
// __gets_chk
// getsecretkey
GOM(getservbyname, pEEpp)
//GO(getservbyname_r, iEppppup)
//GO(getservbyport, pEip)
//GO(getservbyport_r, iEipppup)
// getservent
//GO(getservent_r, iEppup)
GO(getsid, iEi)
GOW(getsockname, iEipp)
GOW(getsockopt, iEiiipp)    // might need wrapping!
GO2(__getsockopt64, iEiiipp, getsockopt)
// getsourcefilter
//GO(getspent, pEv)
// getspent_r
//GO(getspnam, pEp)
// getspnam_r
// getsubopt
GOW(gettext, pEp)
GOW(gettid, iEv)
GOW(gettimeofday, iEBll_p)
//GO(__gettimeofday, iEpp)
GO2(__gettimeofday64, iEpp, gettimeofday)
// getttyent
// getttynam
GOW(getuid, uEv)
// getusershell
//GOW(getutent, pEv)
//GOW(getutent_r, iEpp)
//GOW(getutid, pEp)
//GOW(getutid_r, iEppp)
//GOW(getutline, pEp)
//GOW(getutline_r, iEppp)
// getutmp
// getutmpx
// getutxent
// getutxid
// getutxline
// getw
GO2(getwc, uES, fgetwc)
GO(getwchar, uEv)
GO(getwchar_unlocked, uEv)
//GOW(getwc_unlocked, iEp)
//GO(getwd, pEp)
// __getwd_chk
GO(getxattr, iEpppL)
GOM(glob, iEEpipp)             //%%
//GOM(glob64, iEEpipp)           //%%
GO(globfree, vEp)
//GO(globfree64, vEp)
// glob_pattern_p   // Weak
GOM(gmtime, pEEp)
GOM(__gmtime64, pEEp)
GO2(__gmtime_r, pEpp, my32_gmtime_r)
GOWM(gmtime_r, pEEpp)
GO(gnu_dev_major, uEU)
GO(gnu_dev_makedev, UEuu)
GO(gnu_dev_minor, uEU)
GOW(gnu_get_libc_release, tEv)
GOW(gnu_get_libc_version, tEv)
// __gnu_mcount_nc
// __gnu_Unwind_Find_exidx
GO(grantpt, iEi)
// group_member // Weak
// gsignal  // Weak
// gtty
GOW(hasmntopt, pErppppii_p)
// hcreate
// hcreate_r
// hdestroy // Weak
// hdestroy_r
//DATA(h_errlist, 4)
// h_errno  // type B
GOM(__h_errno_location, pFEv)
//GO(herror, vFp)
// h_nerr   // type R
// host2netname
// hsearch
// hsearch_r
//GO(hstrerror, pFi)
GO(htonl, uEu)
GO(htons, WEW)
GO(iconv, LELbp_bL_bp_bL_)
//GO(iconv_canonicalize, pEp)
GO(iconv_close, iEp)
GO(iconv_open, pEpp)
//GO(if_freenameindex, vEp)
GOW(if_indextoname, pEup)
//GO(if_nameindex, pEv)
GO(if_nametoindex, uEp)
// imaxabs  // Weak
GOWS(imaxdiv, pEpII) //%%
DATAM(in6addr_any, 16)  // type R
DATAM(in6addr_loopback, 16) // type R
// inb  // Weak
//GOW(index, pEpi)
// inet6_opt_append
// inet6_opt_find
// inet6_opt_finish
// inet6_opt_get_val
// inet6_opt_init
// inet6_option_alloc
// inet6_option_append
// inet6_option_find
// inet6_option_init
// inet6_option_next
// inet6_option_space
// inet6_opt_next
// inet6_opt_set_val
// inet6_rth_add
// inet6_rth_getaddr
// inet6_rth_init
// inet6_rth_reverse
// inet6_rth_segments
// inet6_rth_space
GO(inet_addr, uEp)
GOW(inet_aton, iEpp)
// inet_lnaof
// inet_makeaddr
// inet_netof
//GO(inet_network, iEp)
// inet_nsap_addr
//GO(inet_nsap_ntoa, pEipp)
GO(inet_ntoa, tEu)
GO(inet_ntop, pEippu)
GO(inet_pton, iEipp)
//GO(initgroups, iEpi)
// init_module
// initstate    // Weak
GOW(initstate_r, iEupuBpppiiip_)
// inl  // Weak
GO(innetgr, iEpppp)
GO(inotify_add_watch, iEipu)
GO(inotify_init, iEv)
GO(inotify_init1, iEi)
GO(inotify_rm_watch, iEii)
// insque
// __internal_endnetgrent
// __internal_getnetgrent_r
// __internal_setnetgrent
// inw  // Weak
//DATA(_IO_2_1_stderr_, 152)  //sizeof(struct _IO_FILE_plus)
//DATA(_IO_2_1_stdin_, 152)
//DATA(_IO_2_1_stdout_, 152)
//GO(_IO_adjust_column, uEupi)
// _IO_adjust_wcolumn
GO(ioctl, iEiLp)   //the vararg is just to have optional arg of various type, but only 1 arg
GO2(__ioctl_time64, iEiLp, ioctl)
//GO(_IO_default_doallocate, iES)
//GO(_IO_default_finish, vESi)
//GO(_IO_default_pbackfail, iESi)
//GO(_IO_default_uflow, iES)
//GO(_IO_default_xsgetn, LESpL)
//GO(_IO_default_xsputn, LESpL)
//GO(_IO_doallocbuf, vES)
//GO(_IO_do_write, iESpL)
// _IO_fclose
// _IO_fdopen
// _IO_feof
// _IO_ferror
// _IO_fflush
// _IO_fgetpos
// _IO_fgetpos64
// _IO_fgets
//GO(_IO_file_attach, pESi)
//GO(_IO_file_close, iES)
//GO(_IO_file_close_it, iES)
//GO(_IO_file_doallocate, iES)
// _IO_file_finish
//GO(_IO_file_fopen, pESppi)
//GO(_IO_file_init, vES)
//DATA(_IO_file_jumps, 4)
//GO(_IO_file_open, pESpiiii)
//GO(_IO_file_overflow, iESi)
//GO(_IO_file_read, lESpl)
//GO(_IO_file_seek, IESIi)
//GO(_IO_file_seekoff, IESIii)
//GO(_IO_file_setbuf, pESpl)
//GOM(_IO_file_stat, iEESp)
//GO(_IO_file_sync, iES)
//GO(_IO_file_underflow, iES)
//GO(_IO_file_write, lESpl)
//GO(_IO_file_xsputn, LESpL)
//GO(_IO_flockfile, vES)
//GO(_IO_flush_all, iEv)
//GO(_IO_flush_all_linebuffered, vEv)
// _IO_fopen
// _IO_fprintf  // Weak
// _IO_fputs
// _IO_fread
//GO(_IO_free_backup_area, vES)
// _IO_free_wbackup_area
// _IO_fsetpos
// _IO_fsetpos64
// _IO_ftell
// _IO_ftrylockfile
//GO(_IO_funlockfile, vES)
// _IO_fwrite
GO(_IO_getc, iES)
// _IO_getline
//GO(_IO_getline_info, LESpLiip)
// _IO_gets
//GO(_IO_init, vESi)
//GO(_IO_init_marker, vEpS)
// _IO_init_wmarker
// _IO_iter_begin
// _IO_iter_end
// _IO_iter_file
// _IO_iter_next
// _IO_least_wmarker
//GO(_IO_link_in, vEp)
//DATA(_IO_list_all, 4)
// _IO_list_lock
// _IO_list_resetlock
// _IO_list_unlock
//GO(_IO_marker_delta, iEp)
//GO(_IO_marker_difference, iEpp)
//GO(_IO_padn, iEpii)
//GO(_IO_peekc_locked, iEp)
GOW(ioperm, iELLi)
GOW(iopl, iEi)
// _IO_popen
// _IO_printf
//GO(_IO_proc_close, iES)
//GO(_IO_proc_open, pESpp)
GO(_IO_putc, iEiS)
// _IO_puts
//GO(_IO_remove_marker, vEp)
//GO(_IO_seekmark, iESpi)
//GO(_IO_seekoff, IESIii)
//GO(_IO_seekpos, IESIi)
// _IO_seekwmark
//GO(_IO_setb, vESppi)
// _IO_setbuffer
// _IO_setvbuf
//GO(_IO_sgetn, uEppu)
// _IO_sprintf
//GO(_IO_sputbackc, iESi)
// _IO_sputbackwc
// _IO_sscanf
//GO(_IO_str_init_readonly, vEppi)
//GO(_IO_str_init_static, vEppup)
//GO(_IO_str_overflow, iESi)
//GO(_IO_str_pbackfail, iESi)
//GO(_IO_str_seekoff, UESUii)
//GO(_IO_str_underflow, iES)
//GO(_IO_sungetc, iEp)
// _IO_sungetwc
//GO(_IO_switch_to_get_mode, iEp)
// _IO_switch_to_main_wget_area
// _IO_switch_to_wbackup_area
// _IO_switch_to_wget_mode
// _IO_ungetc
//GO(_IO_un_link, vEp)
//GO(_IO_unsave_markers, vEp)
// _IO_unsave_wmarkers
//GOM(_IO_vfprintf, iEEpppp) //%%
//GOM(_IO_vfscanf, iEEppp)   //%%
// _IO_vsprintf
// _IO_wdefault_doallocate
// _IO_wdefault_finish
// _IO_wdefault_pbackfail
// _IO_wdefault_uflow
// _IO_wdefault_xsgetn
// _IO_wdefault_xsputn
// _IO_wdoallocbuf
// _IO_wdo_write
//DATA(_IO_wfile_jumps, 4)
// _IO_wfile_overflow
// _IO_wfile_seekoff
// _IO_wfile_sync
// _IO_wfile_underflow
// _IO_wfile_xsputn
// _IO_wmarker_delta
// _IO_wsetb
// iruserok
// iruserok_af
GO(isalnum, iEi)
// __isalnum_l
// isalnum_l    // Weak
GO(isalpha, iEi)
// __isalpha_l
// isalpha_l    // Weak
GO(isascii, iEi)
// __isascii_l  // Weak
// isastream
GOW(isatty, iEi)
GO(isblank, iEi)
// __isblank_l
// isblank_l    // Weak
GO(iscntrl, iEi)
// __iscntrl_l
// iscntrl_l    // Weak
// isctype  // Weak
// __isctype
GO(isdigit, iEi)
// __isdigit_l
// isdigit_l    // Weak
// isfdtype
GO(isgraph, iEi)
// __isgraph_l
// isgraph_l    // Weak
GOW(isinf, iEd)
GO(__isinf, iEd)
GOW(isinff, iEf)
GO(__isinff, iEf)
// isinfl   // Weak
// __isinfl
GO(islower, iEi)
// __islower_l
// islower_l    // Weak
GOW(isnan, iEd)
GO(__isnan, iEd)
GOW(isnanf, iEf)
GO(__isnanf, iEf)
// isnanl   // Weak
// __isnanl
GO2(__isoc99_fscanf, iEESpV, my32_fscanf)
// __isoc99_fwscanf
// __isoc99_scanf
GOM(__isoc99_sscanf, iEEppV)  //%%
// __isoc99_swscanf
//GOM(__isoc99_vfscanf, iEEppp) //%%
// __isoc99_vfwscanf
// __isoc99_vscanf
//GOM(__isoc99_vsscanf, iEEppp) //%% TODO: check if ok
// __isoc99_vswscanf
// __isoc99_vwscanf
// __isoc99_wscanf
GO2(__isoc23_fscanf, iEESpV, my32_fscanf)
GO2(__isoc23_sscanf, iEEppV, my32_sscanf)
GO2(__isoc23_vsscanf, iEEppp, my32_vsscanf)
GO2(__isoc23_wcstol, lEpBp_i, my32_wcstol)
GO2(__isoc23_strtoll, IEpBp_i, strtoll)
GO2(__isoc23_strtoull, UEpBp_i, strtoull)
GO2(__isoc23_strtol, lEpBp_i, my32_strtol)
GO2(__isoc23_strtoul, LEpBp_i, my32_strtoul)
GO(isprint, iEi)
// __isprint_l
// isprint_l    // Weak
GO(ispunct, iEi)
// __ispunct_l
// ispunct_l    // Weak
GO(isspace, iEi)
// __isspace_l
// isspace_l    // Weak
GO(isupper, iEi)
// __isupper_l
// isupper_l    // Weak
GOW(iswalnum, iEu)
// __iswalnum_l
//GOW(iswalnum_l, iEua)
GOW(iswalpha, iEu)
// __iswalpha_l
//GOW(iswalpha_l, iEua)
GOW(iswblank, iEu)
// __iswblank_l
GOW(iswblank_l, iEua)
GOW(iswcntrl, iEu)
// __iswcntrl_l
GOW(iswcntrl_l, iEua)
GOW(iswctype, iEuL)
// __iswctype
GO(__iswctype_l, iEuLa)
// iswctype_l   // Weak
GOW(iswdigit, iEu)
// __iswdigit_l
//GOW(iswdigit_l, iEua)
GOW(iswgraph, iEu)
// __iswgraph_l
//GOW(iswgraph_l, iEua)
GOW(iswlower, iEu)
// __iswlower_l
//GOW(iswlower_l, iEua)
GOW(iswprint, iEu)
// __iswprint_l
//GOW(iswprint_l, iEua)
GOW(iswpunct, iEu)
// __iswpunct_l
//GOW(iswpunct_l, iEua)
GOW(iswspace, iEu)
// __iswspace_l
//GOW(iswspace_l, iEua)
GOW(iswupper, iEu)
// __iswupper_l
//GOW(iswupper_l, iEua)
GOW(iswxdigit, iEu)
// __iswxdigit_l
//GOW(iswxdigit_l, iEua)
GO(isxdigit, iEi)
// __isxdigit_l
// isxdigit_l   // Weak
// _itoa_lower_digits   // type R
// __ivaliduser
GO(jrand48, lEp)
// jrand48_r    // Weak
// key_decryptsession
// key_decryptsession_pk
// __key_decryptsession_pk_LOCAL    // type B
// key_encryptsession
// key_encryptsession_pk
// __key_encryptsession_pk_LOCAL    // type B
// key_gendes
// __key_gendes_LOCAL   // type B
// key_get_conv
// key_secretkey_is_set
// key_setnet
// key_setsecret
GOW(kill, iEii)
GO(killpg, iEii)
// klogctl
// l64a
GO(labs, lEl)
// lchmod
GOW(lchown, iEpuu)
// lckpwdf  // Weak
// lcong48
// lcong48_r    // Weak
GOW(ldexp, dFdi)
GOW(ldexpf, fFfi)
#ifdef HAVE_LD80BITS
GOW(ldexpl, DFDi)
#else
GOW2(ldexpl, KFKi, ldexp)
#endif
GOS(ldiv, pEEpii)               //%% return a struct, so address of stuct is on the stack, as a shadow 1st element
GOM(lfind, pEEppbL_Lp)            //%%
GO(lgetxattr, iEpppL)
GOM(__libc_alloca_cutoff, iEEL) //%%
// __libc_allocate_rtsig
// __libc_allocate_rtsig_private
//GO(__libc_calloc, pELL)
// __libc_clntudp_bufcreate
GO(__libc_current_sigrtmax, iEv)
// __libc_current_sigrtmax_private
GO(__libc_current_sigrtmin, iEv)
// __libc_current_sigrtmin_private
//GOM(__libc_dlclose, iEEp)       //%%
// __libc_dl_error_tsd
//GOM(__libc_dlopen_mode, pEEpi)  //%%
//GOM(__libc_dlsym, pEEpp)        //%%
// __libc_fatal
// __libc_fork
//GO(__libc_free, vEp)
// __libc_freeres
GOM(__libc_init_first, vEEipV)  //%%
DATAM(_libc_intl_domainname, 4)    // type R
//GO2(__libc_open, iEEpOu, my_open)
// __libc_longjmp
// __libc_mallinfo
//GO(__libc_malloc, pEL)
// __libc_mallopt
//GO(__libc_memalign, pELL)
// __libc_pthread_init
//GO(__libc_pvalloc, pEL)
// __libc_pwrite
//GO2(__libc_read, lEipL, my_read) //%%,noE
//GO(__libc_realloc, pEpL)
// __libc_sa_len
// __libc_siglongjmp
GOM(__libc_start_main, iEEpippppp) //%%
GO2(__libc_sigaction, iEEipp, my32_sigaction) //%%
// __libc_system
// __libc_thread_freeres
//GO(__libc_valloc, pEL)
GOW(link, iEpp)
//GO(linkat, iEipipi)
GOW(listen, iEii)
GO(listxattr, iEppL)
// llabs
// lldiv
GO(llistxattr, iEppL)
// llseek   // Weak
// loc1 // type B
// loc2 // type B
GOWM(localeconv, pEEv)
GOM(localtime, pEEp)
GOM(__localtime64, pEEp)
GOWM(localtime_r, pEEpp)
GO(lockf, iEiil)
GO(lockf64, iEiiI)
// locs // type B
GOM(longjmp, vEEpi)         //%%
GOM(_longjmp, vEEpi)        //%%
GOM(__longjmp_chk, vEEpi)   //%%
GO(lrand48, lEv)
// lrand48_r
//GO(lremovexattr, iEpp)
GOM(lsearch, pEEppbL_Lp)      //%%
GOW(lseek, lEili)
// __lseek  // Weak
GOW(lseek64, IEiIi)
GO(lsetxattr, iEpppLi)
//GO(lutimes, iEpp)
GOM(__lxstat, iEEipp)       //%%
GOM(__lxstat64, iEEipp)     //%%
GO(madvise, iEpLi)
GOWM(makecontext, vEEppiV)   //%%
GOW(mallinfo, pEv)
GOM(malloc, pEL)            //%%,noE
// malloc_get_state // Weak
//DATAV(__malloc_hook, 4)
//DATAV(__malloc_initialize_hook, 4)
// malloc_set_state // Weak
// malloc_stats // Weak
GOW(malloc_trim, iEL)
GOW(malloc_usable_size, LEp)
GOW(mallopt, iEii)  // Weak
// mallwatch    // type B
//GO(mblen, iEpL)
//GOW(mbrlen, LEpLp)
//GO(__mbrlen, LEpLp)
GOW(mbrtowc, LEppLp)
GO(__mbrtowc, LEppLp)
GOW(mbsinit, iEp)
GOW(mbsnrtowcs, LEpbp_LLp)
// __mbsnrtowcs_chk
GOW(mbsrtowcs, LEpbp_Lp)
// __mbsrtowcs_chk
GO(mbstowcs, LEppL)
// __mbstowcs_chk
GO(mbtowc, iEppL)
// mcheck
// mcheck_check_all
// mcheck_pedantic
// _mcleanup
//GOWM(mcount, vEpp)   //%%,noE
// _mcount
GOW(memalign, pELL)
//DATAV(__memalign_hook, 4)
//GOW(memccpy, pEppiL)
GO(memchr, pEpiL)
GO(memcmp, iEppL)
GO(memcpy, pEppL)
GO(__memcpy_chk, pEppLL)
// memfrob
GO(memfd_create, iEpu)
GO(memmem, pEpLpL)
GO(memmove, pEppL)
GO(__memmove_chk, pEppLL)
GO(mempcpy, pEppL)
GO(__mempcpy, pEppL)
// __mempcpy_chk
// __mempcpy_small
GOW(memrchr, pEpiL)
GO(memset, pEpiL)
GO(__memset_chk, pEpiLL)
GO(mincore, iEpLp)
GOW(mkdir, iEpu)
GO(mkdirat, iEipu)
GO(mkdtemp, pEp)
GO(mkfifo, iEpu)
//GO(mkfifoat, iEipu)
GO(mkostemp, iEpi)
GO(mkostemp64, iEpi)
GO(mkstemp, iEp)
GO(mkstemp64, iEp)
//GO(mktemp, pEp)
GO(mktime, LEbiiiiiiiiilt_)
GO2(__mktime64, LEp, mktime)
GO(mlock, iEpL)
GO(mlockall, iEi)
GOM(mmap, pEEpLiiil)    //%%
GOM(mmap64, pEEpLiiiI)  //%%
// modf // Weak
// modff    // Weak
// modfl    // Weak
// moncontrol   // Weak
// monstartup   // Weak
// __monstartup
//DATA(__morecore, 4)
GOW(mount, iEpppLp)
// mprobe
GOM(mprotect, iEEpLi)   //%%
// mrand48
// mrand48_r
GOWM(mremap, pEEpLLiN)	//%% 5th hidden paramerer "void* new_addr" if flags is MREMAP_FIXED
//GO(msgctl, iEiip)
//GOW(msgget, iEpi)
//GOW(msgrcv, lEipLli)
//GOW(msgsnd, iEipLi)
GOW(msync, iEpLi)
GO(mtrace, vFv)
GO(munlock, iEpL)
GO(munlockall, iEv)
GOM(munmap, iEEpL)       //%%
GO(muntrace, vFv)
GOWM(nanosleep, iErLL_BLL_)	 //%%,noE
// __nanosleep  // Weak
// netname2host
// netname2user
GOW(newlocale, aEipa)
GO(__newlocale, aEipa)
// nfsservctl
GOM(nftw, iEEppii)       //%%
GOM(nftw64, iEEppii)     //%%
//GOW(ngettext, pEppu)
GO(nice, iEi)
// _nl_default_dirname   // type R
// _nl_domain_bindings   // type B
GO(nl_langinfo, tEi)
GO(__nl_langinfo_l, tEia)
//GOW(nl_langinfo_l, pEup)
//DATAB(_nl_msg_cat_cntr, 4) // type B
// nrand48
// nrand48_r    // Weak
// __nss_configure_lookup
// __nss_database_lookup
// __nss_disable_nscd
// _nss_files_parse_grent
// _nss_files_parse_pwent
// _nss_files_parse_spent
// __nss_group_lookup
// __nss_group_lookup2
// __nss_hostname_digits_dots
// __nss_hosts_lookup
// __nss_hosts_lookup2
// __nss_lookup_function
// __nss_next
// __nss_next2
// __nss_passwd_lookup
// __nss_passwd_lookup2
// __nss_services_lookup2
GOW(ntohl, uEu)
GOW(ntohs, WEW)
// ntp_adjtime  // Weak
// ntp_gettime
// _null_auth   // type B
// _obstack_allocated_p
//DATAM(obstack_alloc_failed_handler, 4)
//GOM(_obstack_begin, iEpLLpp) //%%,noE
// _obstack_begin_1
//DATA(obstack_exit_failure, 4)
//GOM(_obstack_free, vEpp)     //%%,noE
//GOM(obstack_free, vEpp)      //%%,noE
// _obstack_memory_used
GOM(_obstack_newchunk, vEEpi)
// obstack_printf   // Weak
// __obstack_printf_chk
GOWM(obstack_vprintf, iEEpppp)  //%%
// __obstack_vprintf_chk
//GOWM(on_exit, iEEpp)  //%%
//GO2(__on_exit, iEEpp, my_on_exit)   //%%
GOW2(open, iEEpON, my_open)    //%%
GOW2(__open, iEEpON, my_open)  //%%
GO(__open_2, iEpO)
GOW2(open64, iEEpON, my_open64)  //%%
// __open64 // Weak
GO(__open64_2, iEpO)
//GOW(openat, iEipON)
// __openat_2
GOW(openat64, iEipON)
//GO(__openat64_2, iEipON)
// __open_catalog
GOW(opendir, pEp)
GO(openlog, vEpii)
GOW(open_memstream, SEpp)
// open_wmemstream
//DATAB(optarg, 4)
//DATA(opterr, 4)
//DATA(optind, 4)
//DATA(optopt, 4)
// outb // Weak
// outl // Weak
// outw // Weak
//GO(__overflow, iEpi)
GO(parse_printf_format, uEpup)
// passwd2des
GOW(pathconf, lEpi)
GOW(pause, iEv)
GO(pclose, iES)
GO(perror, vEp)
GOW(personality, iFL)
GOW(pipe, iEp)
// __pipe
GOW(pipe2, iEpO) // assuming this works the same as pipe, so pointer for array of 2 int
// pivot_root
// pmap_getmaps
// pmap_getport
// pmap_rmtcall
// pmap_set
// pmap_unset
GOW(poll, iEpLi)    // poll have an array of struct as 1st argument
GO(__poll, iEpLi)
GO(popen, SEpp)
GO(posix_fadvise, iEilli)
GO(posix_fadvise64, iEiIIi)
GO(posix_fallocate, iEill)
GO(posix_fallocate64, iEiII)
GO(posix_madvise, iEpLi)
GOW(posix_memalign, iEBp_LL)
// posix_openpt // Weak
GOM(posix_spawn, iEEpppppp) //%%
GO(posix_spawnattr_destroy, iFp)
// posix_spawnattr_getflags
// posix_spawnattr_getpgroup
// posix_spawnattr_getschedparam
// posix_spawnattr_getschedpolicy
// posix_spawnattr_getsigdefault
// posix_spawnattr_getsigmask
GO(posix_spawnattr_init, iFp)
GOW(posix_spawnattr_setflags, iFpw)
// posix_spawnattr_setpgroup
// posix_spawnattr_setschedparam
// posix_spawnattr_setschedpolicy
GOW(posix_spawnattr_setsigdefault, iFpp)
// posix_spawnattr_setsigmask
GOM(posix_spawn_file_actions_addclose, iEEpi)   //%%
GOM(posix_spawn_file_actions_adddup2, iEEpii)   //%%
GOM(posix_spawn_file_actions_addopen, iEEpipOi) //%%
GOM(posix_spawn_file_actions_destroy, iEEp) //%%
GOM(posix_spawn_file_actions_init, iEEp)    //%%
GOM(posix_spawnp, iEEpppppp) //%%
GO(ppoll, iEpurLL_p)
GO(__ppoll_chk, iEpurLL_pL)
GOWM(prctl, iEEiLLLL)
GOWM(__prctl_time64, iEEiLLLL)
GOW(pread, lEipLl)
GOW(pread64, lEipLI)
// __pread64    // Weak
// __pread64_chk
//GOM(preadv64, lEEipiI)  //%% not always present
// __pread_chk
GOM(printf, iEEpV) //%%
GOM(__printf_chk, iEEipV) //%%
//GO(__printf_fp, iEppp)  // does this needs aligment?
// printf_size
// printf_size_info
// profil   // Weak
// __profile_frequency
DATAM(__progname, 4)
DATAM(__progname_full, 4)
DATAM(program_invocation_name, 4)
DATAM(program_invocation_short_name, 4)
GOW(pselect, iEippprLL_p)
// psignal
//GO(ptrace, iEiupp)  // will that work???
//GO(ptsname, pEi)
//GOW(ptsname_r, iEipu)
// __ptsname_r_chk
GOW(putc, iEiS)
GO(putchar, iEi)
GO(putchar_unlocked, iEi)
//GO(putc_unlocked, iEip)
GO(putenv, iEp)
// putgrent
// putmsg
// putpmsg
// putpwent
GOW(puts, iEp)
// putspent
//GOW(pututline, pEp)
// pututxline
// putw
GO(putwc, uEiS)
// putwchar
GO(putwchar_unlocked, uEi)
//GO(putwc_unlocked, iEip)
// pvalloc  // Weak
// pwrite   // Weak
GOW(pwrite64, lEipLI)
// __pwrite64   // Weak
//GOM(pwritev64, lEEipiI)  //%% not always present
// qecvt
#ifdef HAVE_LD80BITS
//GO(qecvt_r, iEDipppL)
#else
//GO(qecvt_r, iEKipppL)
#endif
// qfcvt
#ifdef HAVE_LD80BITS
//GO(qfcvt_r, iEDipppL)
#else
//GO(qfcvt_r, iEKipppL)
#endif
// qgcvt
GOM(qsort, vEEpLLp) //%%
//GOM(qsort_r, vEEpLLpp) //%%
// query_module // Weak
//GO(quotactl, iEipip)
GO(raise, iEi)
GO(rand, iEv)
GOW(random, lEv)
GOW(random_r, iEbpppiiip_p)
GO(rand_r, iEp)
GOW(rawmemchr, pEpi)
GO(__rawmemchr, pEpi)
// rcmd
// rcmd_af
// __rcmd_errstr    // type B
GOM(read, lEipL) //%%,noE
//GOW(__read, lEipL)
// readahead    // Weak
GO(__read_chk, lEipLL)
GOWM(readdir, pEEp)  //%%
GO(readdir64, pEp)  // check if alignement is correct
//GOM(readdir_r, iEEppp)  //%% should also be weak
GO2(readlink, lEEppL, my_readlink) //%%
GOM(readlinkat, iEEippL)
// __readlinkat_chk
// __readlink_chk
GOM(readv, lEEipi)
GO(realloc, pEpL)
//DATAV(__realloc_hook, 4)
GOM(realpath, pEEpp) //%%
GO2(__realpath_chk, pEEppi, my32_realpath)
// reboot
// re_comp  // Weak
// re_compile_fastmap   // Weak
//GOW(re_compile_pattern, pEpup)
GO(recv, lEipLi)
GO(__recv_chk, iEipLLi)
GOW(recvfrom, lEipLipp)
// __recvfrom_chk
GOM(recvmmsg, iEEipuurLL_)
GOWM(recvmsg, lEEipi)
GO2(__recvmsg64, lEipi, recvmsg)
// re_exec  // Weak
GOWM(regcomp, iEEppi)
GOWM(regerror, uEEippu)
GOM(regexec, iEEppupi)
GOWM(regfree, vEEp)
GOM(__register_atfork, iEEpppp) //%%
// register_printf_function // Weak
// registerrpc
// remap_file_pages // Weak
//GOW(re_match, iEppiip)
// re_match_2   // Weak
GO(remove, iEp)
//GO(removexattr, iEpp)
// remque
GO(rename, iEpp)
GO(renameat, iEipip)
//GO(renameat2, iEipipu)
// _res // type B
//GOW(re_search, iEppiiip)
//GOW(re_search_2, iEppipiiipi)
// re_set_registers // Weak
GOW(re_set_syntax, LEL)
// _res_hconf   // type B
GO(__res_iclose, vEpi)
GO(__res_init, iEv)
//GO(__res_maybe_init, iEpi)
GO(__res_nclose, vEp)
GO(__res_ninit, iEp)
//DATA(__resp, 4)
// __res_randomid
GOM(__res_state, pEEv)
//DATA(re_syntax_options, 4)    // type B
// revoke
GO(rewind, vES)
GO(rewinddir, vEp)
// rexec
// rexec_af
// rexecoptions // type B
GOW(rindex, pEpi)
GOW(rmdir, iEp)
GO(readdir64_r, iEppBp_)
// rpc_createerr    // type B
// _rpc_dtablesize
// __rpc_thread_createerr
// __rpc_thread_svc_fdset
// __rpc_thread_svc_max_pollfd
// __rpc_thread_svc_pollfd
//GO(rpmatch, iEp)
// rresvport
// rresvport_af
// rtime
// ruserok
// ruserok_af
// ruserpass
GOW(sbrk, pEl)
GO(__sbrk, pEl)
// scalbn   // Weak
// scalbnf  // Weak
// scalbnl  // Weak
GOM(scandir, iEEpppp) //%%
GOM(scandir64, iEEpppp) //%%
//GO2(scanf, iEpp, vscanf)
//GO(__sched_cpualloc, pEu)   //TODO: check, return cpu_set_t* : should this be aligned/changed?
GO(__sched_cpucount, iEup)
//GO(__sched_cpufree, vEp)
GO(sched_getaffinity, iEiup)
GO(sched_getcpu, iEv)
GO(__sched_getparam, iEip)
GOW(sched_getparam, iEip)
GO(__sched_get_priority_max, iEi)
GOW(sched_get_priority_max, iEi)
GO(__sched_get_priority_min, iEi)
GOW(sched_get_priority_min, iEi)
GO(__sched_getscheduler, iEi)
GOW(sched_getscheduler, iEi)
//GOW(sched_rr_get_interval, iEip)
GO(sched_setaffinity, iEiup)
//GOW(sched_setparam, iEip)
GO(__sched_setscheduler, iEiip)
GOW(sched_setscheduler, iEiip)
GO(__sched_yield, iEv)
GOW(sched_yield, iEv)
GO2(__secure_getenv, tEp, secure_getenv)
GO(secure_getenv, tEp)
// seed48
// seed48_r // Weak
//GO(seekdir, vEpi)
GOW(select, iEippprLL_)
GO(__select, iEippprLL_)
GO2(__select64, iEipppp, select)
GO(semctl, iEiiiN)
GOW(semget, iEiii)
GOW(semop, iEipL)
//GO(semtimedop, iEipup)
GOW(send, lEipLi)
// __send   // Weak
GO(sendfile, lEiibl_L)
GO(sendfile64, lEiipL)
GOWM(sendmsg, lEEipi)
GO2(__sendmsg64, lEipi, sendmsg)
GOM(sendmmsg, iEEipuu)
GOW(sendto, lEipLipu)
// setaliasent
GOW(setbuf, vESp)
//GOW(setbuffer, vEppL)
GOM(setcontext, iEEp) //%%
// setdomainname
GO(setegid, iEu)
GOW(setenv, iEppi)
// _seterr_reply
GO(seteuid, iEu)
// setfsent
// setfsgid
// setfsuid
GOW(setgid, iEu)
GO(setgrent, vEv)
GO(setgroups, iEup)
GO(sethostent, vFi)
// sethostid
//GO(sethostname, iEpu)
// setipv4sourcefilter
GOW(setitimer, iEirLLLL_BLLLL_)
GOM(setjmp, iEEp) //%%
GOM(_setjmp, iEEp) //%%
GO(setlinebuf, vES)
GO(setlocale, tEip)
// setlogin
GO(setlogmask, iEi)
GOW(setmntent, SEpp)
// __setmntent
// setnetent
// setnetgrent
GO(setns, iEii)
GOW(setpgid, iEii)
// __setpgid
GO(setpgrp, iEv)
GO(setpriority, iEuui)
GO(setprotoent, vEi)
GO(setpwent, vEv)
GOW(setregid, iEuu)
GOW(setresgid, iEuuu)
GOW(setresuid, iEuuu)
GOW(setreuid, iEuu)
GOM(setrlimit, iEEup)
GO(setrlimit64, iEup)
// setrpcent
// setservent
GOW(setsid, iEv)
GOW(setsockopt, iEiiipu)
GO2(__setsockopt64, iEiiipu, setsockopt)
// setsourcefilter
GO(setspent, vEv)
// setstate // Weak
//GOW(setstate_r, iEpp)
//GOW(settimeofday, iEpp)
// setttyent
GOW(setuid, iEu)
// setusershell
GOW(setutent, vEv)
// setutxent
GOW(setvbuf, iESpiL)
GO(setxattr, iEpppLi)
// sgetspent
// sgetspent_r  // Weak
#ifdef WINLATOR_GLIBC
GOW(shmat, pEipi)
GOW(shmdt, iEp)
#else
GOWM(shmat, pEEipi)
GOWM(shmdt, iEEp)
#endif
GOW(shmctl, iEiip)
GOW(shmget, iEiLi)
GOW(shutdown, iEii)
GOWM(sigaction, iEEipp)    //%%
//GOWM(__sigaction, iEEipp)  //%%
GO(sigaddset, iEpi)
// __sigaddset
GOWM(sigaltstack, iEEpp)   //%%
// sigandset
GOW(sigblock, iEi)
GO(sigdelset, iEpi)
// __sigdelset
GO(sigemptyset, iEp)
GO(sigfillset, iEp)
GO(siggetmask, iEv)
// sighold
// sigignore
GO(siginterrupt, iEii)  // no need to wrap this one?
GO(sigisemptyset, iFp)
GO(sigismember, iEpi)
// __sigismember
GOM(siglongjmp, vEEip) //%%
GOW2(signal, pEEip, my_signal)
GO(signalfd, iFipi)
GO(__signbit, iEd)
GO(__signbitf, iEf)
// sigorset
// sigpause // Weak
// __sigpause
//GO(sigpending, iEp)
GOW(sigprocmask, iEipp)
// sigqueue // Weak
// sigrelse
// sigreturn    // Weak
GOM(sigset, pEEip) //%%
GOM(__sigsetjmp, iEEp) //%%
GOW(sigsetmask, iEi)
// sigstack
GOW(sigsuspend, iEp)
// __sigsuspend
GOW(sigtimedwait, iEpprLL_)
//GOW(sigvec, iEipp)
//GOW(sigwait, iEpp)
//GOW(sigwaitinfo, iEpp)
GOW(sleep, uEu)
GOM(snprintf, iEEpLpV) //%%
GOM(__snprintf_chk, iEEpLiipV) //%%
//GOM(__snprintf, iEEpLpV) //%%
// sockatmark
GOW(socket, iEiii)
GOW(socketpair, iEiiip)
GO(splice, iEipipLu)
GOM(sprintf, iEEppV) //%%
GOM(__sprintf_chk, iEEpiipV) //%%
// sprofil  // Weak
GOW(srand, vEu)
GO(srand48, vEl)
// srand48_r    // Weak
GOW(srandom, vEu)
//GOW(srandom_r, iEup)
GOM(sscanf, iEEppV) //%%
// ssignal  // Weak
// sstk
GOM(__stack_chk_fail, vEEv) //%%
//GOM(lstat64, iEpp)	//%%,noE
GO2(__lstat64_time64, iEEpp, my_lstat64)
//GOM(stat64, iEpp)	//%%,noE
GO2(__stat64_time64, iEEpp, my_stat64)
GOM(stat, iFpp) //%%,noE
GOWM(statfs, iEpp)  //%%,noE
// __statfs
GOWM(statfs64, iEpp)     //%%,noE
GOM(statvfs, iEEpp)
GOWM(statvfs64, iEEpp)
GOM(statx, iEEipiup)
DATAM(stderr, 4)
DATAM(stdin, 4)
DATAM(stdout, 4)
// step // Weak
// stime
GO(stpcpy, pEpp)
// __stpcpy
GO(__stpcpy_chk, pEppL)
// __stpcpy_small
GOW(stpncpy, pEppL)
//GO(__stpncpy, pEppL)
//GO(__stpncpy_chk, pEppLL)
GOW(strcasecmp, iEpp)
//GO(__strcasecmp, iEpp)
// __strcasecmp_l
// strcasecmp_l // Weak
GOW(strcasestr, pEpp)
GO(__strcasestr, pEpp)
GO(strcat, pEpp)
GO(__strcat_chk, pEppL)
GO(strchr, pEpi)
GOW(strchrnul, pEpi)
GO(strcmp, iEpp)
GO(strcoll, iEpp)
GO(__strcoll_l, iEppa)
//GOW(strcoll_l, iEppp)
GO(strcpy, pEpp)
GO(__strcpy_chk, pEppL)
// __strcpy_small
GO(strcspn, LEpp)
// __strcspn_c1
// __strcspn_c2
// __strcspn_c3
GOW(strdup, pEp)
GO(__strdup, pEp)
GO(strerror, tEi)
GO(strerror_l, pEia)
GO(__strerror_r, tEipL)
GOW(strerror_r, tEipL)
//GO(strfmon, lEpLpppppppppp) //vaarg, probably needs align, there are just double...
// __strfmon_l
// strfmon_l    // Weak
// strfry
GO(strftime, LEpLpriiiiiiiiilt_)
GO(__strftime_l, LEpLpriiiiiiiiilt_a)
GOW(strftime_l, LEpLpriiiiiiiiilt_a)
GO(strlen, LEp)
GOW(strncasecmp, iEppL)
// __strncasecmp_l
// strncasecmp_l    // Weak
GO(strlcat, pEppL)
GO(strlcpy, pEppL)
GO(strncat, pEppL)
GO(__strncat_chk, pEppLL)
GO(strncmp, iEppL)
GO(strncpy, pEppL)
GO(__strncpy_chk, pEppLL)
GOW(strndup, pEpL)
GO(__strndup, pEpL)
GO(strnlen, LEpL)
GO(strpbrk, pEpp)
// __strpbrk_c2
// __strpbrk_c3
GO(strptime, pEppriiiiiiiiilt_)
// strptime_l   // Weak
GO(strrchr, pEpi)
//GOW(strsep, pEpp)
// __strsep_1c
// __strsep_2c
// __strsep_3c
// __strsep_g
GO(strsignal, tEi)
GO(strspn, LEpp)
// __strspn_c1
// __strspn_c2
// __strspn_c3
GO(strstr, pEpp)
GO(strtod, dEpBp_)
GO(__strtod_internal, dEpBp_i)
GO(__strtod_l, dEpBp_a)
GOW(strtod_l, dEpBp_a)
GO(strtof, fEpBp_)
GO(__strtof_internal, fEpBp_p)
GO(__strtof_l, fEpBp_a)
//GOW(strtof_l, fEppu)
//GO(strtoimax, IEppi)
GO(strtok, pEpp)
GO(__strtok_r, pEppbp_)
GOW(strtok_r, pEppbp_)
// __strtok_r_1c
GOM(strtol, lEpBp_i)    //%%,noE
#ifdef HAVE_LD80BITS
//GO(strtold, DEpp)
//GO(__strtold_internal, DEppi)
GO(__strtold_l, DEpBp_a)
GOW(strtold_l, DEpBp_a)
#else
//GO(strtold, KEpp)
//GO2(__strtold_internal, KEppi, __strtod_internal)
GO2(__strtold_l, KEpBp_a, __strtod_l)
GOW2(strtold_l, KEpBp_a, strtod_l)
#endif
GO2(__strtol_internal, lEpBp_i, my32_strtol)    //%%,noE
GO(strtoll, IEpBp_i)
//GO(__strtol_l, lEppiip)
//GOW(strtol_l, lEppiip)
GO(__strtoll_internal, IEpBp_ii)
GO(__strtoll_l, IEpBp_ia)
GOW(strtoll_l, IEpBp_ia)
GOW(strtoq, IEppi)  // is that ok?
GOM(strtoul, LEpBp_i)   //%%,noE
GO2(__strtoul_internal, LEpBp_ii, my32_strtoul) //%%,noE
GO(strtoull, UEpBp_i)
//GO(__strtoul_l, uEppip)
//GOW(strtoul_l, LEppip)
GO(__strtoull_internal, UEpBp_ii)
GO(__strtoull_l, UEpBp_ia)
GOW(strtoull_l, UEpBp_ia)
//GO(strtoumax, UEppi)
GOW(strtouq, UEppi) // ok?
GOW(strverscmp, iEpp)
// __strverscmp
GO(strxfrm, uEppL)
GO(__strxfrm_l, LEppLa)
//GO(strxfrm_l, uEppup)
// stty
// svcauthdes_stats // type B
// svcerr_auth
// svcerr_decode
// svcerr_noproc
// svcerr_noprog
// svcerr_progvers
// svcerr_systemerr
// svcerr_weakauth
// svc_exit
// svcfd_create
// svc_fdset    // type B
// svc_getreq
// svc_getreq_common
// svc_getreq_poll
// svc_getreqset
// svc_max_pollfd   // type B
// svc_pollfd   // type B
// svcraw_create
// svc_register
// svc_run
// svc_sendreply
// svctcp_create
// svcudp_bufcreate
// svcudp_create
// svcudp_enablecache
// svcunix_create
// svcunixfd_create
// svc_unregister
//GO(swab, vEppi)
GOM(swapcontext, iEEpp) //%%
// swapoff  // Weak
// swapon   // Weak
GOM(swprintf, iEEpLpV) //%%
GOM(__swprintf_chk, iEEpLiLpV) //%%
GOM(swscanf, iEEppV)
GOW(symlink, iEpp)
GO(symlinkat, iEpip)
GO(sync, vEv)
GO(syncfs, iEi)
// sync_file_range
GOM(syscall, lEEuV) //%%
GOW2(sysconf, lEEi, my_sysconf)
GO2(__sysconf, lEEi, my_sysconf)
// sysctl   // Weak
//GO(__sysctl, iEp)
//DATA(_sys_errlist, 4)
//DATA(sys_errlist, 4)
GOM(sysinfo, iEp)   //%noE
GOM(syslog, vEEipV)
GOM(__syslog_chk, vEEiipV)
//DATA(_sys_nerr, 4)    // type R
//DATA(sys_nerr, 4) // type R
//DATA(sys_sigabbrev, 4)
//DATA(_sys_siglist, 4)
//DATA(sys_siglist, 4)
GOW(system, iEp)          // Need to wrap to use box86 if needed?
GO2(__sysv_signal, pEEip, my___sysv_signal) //%%
//GOWM(sysv_signal, pEEip)  //%%
GOW(tcdrain, iEi)
GO(tcflow, iEii)
GO(tcflush, iEii)
GOW(tcgetattr, iEip)
GO(tcgetpgrp, iEi)
// tcgetsid
GO(tcsendbreak, iEii)
GO(tcsetattr, iEiip)
GO(tcsetpgrp, iEii)
// tdelete  // Weak
// tdestroy // Weak
// tee
//GO(telldir, iEp)
GO(tempnam, pEpp)
GOW(textdomain, tEp)
// tfind    // Weak
GO(time, LEBL_)
GO2(__time64, IEp, time)
GOM(timegm, lEEriiiiiiiiilt_)   //%%
// timelocal    // Weak
GO(timerfd_create, iEii)
//GO(timerfd_gettime, iEip)
GO(timerfd_settime, iEiirLL_BLL_)
GOW(times, iEBllll_)
DATAM(timezone, 4)
DATAM(__timezone, 4)   // type B
GO(tmpfile, pEv)
GO(tmpfile64, pEv)
GO(tmpnam, pEp)
//GO(tmpnam_r, pEp)
GO(toascii, iEi)
// __toascii_l  // Weak
GO(tolower, iEi)
// _tolower
// __tolower_l
//GOW(tolower_l, iEip)
GO(toupper, iEi)
// _toupper
// __toupper_l
//GOW(toupper_l, iEip)
// towctrans    // Weak
// __towctrans
// __towctrans_l
// towctrans_l  // Weak
GO(towlower, uEu)
GO(__towlower_l, uEua)
GOW(towlower_l, uEua)
GO(towupper, uEu)
GO(__towupper_l, uEua)
GOW(towupper_l, uEua)
// tr_break
//GOW(truncate, iEpu)
GO(truncate64, iESU)
// tsearch  // Weak
//GO(ttyname, pEi)
//GOW(ttyname_r, iEipu)
// __ttyname_r_chk
// ttyslot
// twalk    // Weak
DATAM(tzname, 4)
//DATA(__tzname, 4)
GOWM(tzset, vEv)    //%%,noE
// ualarm
GO(__uflow, iES)
// ulckpwdf // Weak
// ulimit   // Weak
GOW(umask, uEu)
//GOW(umount, iEp)
//GOW(umount2, iEpi)
GOWM(uname, iEp) //%%,noE
//GO(__underflow, iEp)
GOW(ungetc, iEiS)
GO(ungetwc, uEuS)
GOW(unlink, iEp)
GO(unlinkat, iEipi)
GO(unlockpt, iEi)
GOW(unsetenv, iEp)
// unshare
//GOW(updwtmp, vEpp)
// updwtmpx
// uselib
GOW(uselocale, aEa)
GO(__uselocale, aEa)
// user2netname
GO(usleep, iEu)
// ustat
GO(utime, iEprll_)
GOM(utimensat, iEippi)  //%noE
GOWM(utimes, iEEpp)
//GOW(utmpname, iEp)
// utmpxname
GOW(valloc, pEL)
GOM(vasprintf, iEEppp) //%%
GOM(__vasprintf_chk, iEEpipp) //%%
// vdprintf // Weak
// __vdprintf_chk
//GOM(verr, vEEpV) //%%
// verrx
//GO(versionsort, iEpp)
//GO(versionsort64, iEpp) //need to align dirent64?
GOWM(vfork, iEEv) //%%
// __vfork
GOM(vfprintf, iEESpp) //%%
GOM(__vfprintf_chk, iEESipp) //%%
//GOWM(vfscanf, iEEppp)  //%%
// __vfscanf
//GOWM(vfwprintf, iEEppp)    //%%
//GO2(__vfwprintf_chk, iEEpvpp, my_vfwprintf)
//GOW(vfwscanf, iEppp)
// vhangup
// vlimit
// vmsplice
GOM(vprintf, iEEpp)               //%%
GOM(__vprintf_chk, iEEipp)        //%%
// vscanf   // Weak
GOWM(vsnprintf, iEEpLpp)         //%%
GOWM(__vsnprintf, iEEpLpp)       //%%
GOM(__vsnprintf_chk, iEEpLiipp)  //%%
GOWM(vsprintf, iEEppp)            //%%
GOM(__vsprintf_chk, iEEpiLpp)     //%% 
GOM(vsscanf, iEEppp) //%%
// __vsscanf    // Weak
GOWM(vswprintf, iEEpLpp)         //%%
GOWM(__vswprintf, iEEpLpp)         //%%
GOWM(__vswprintf_chk, iEEpLiLppp) //%%
GOM(vswscanf, iEEppp)
GOM(vsyslog, vEEipp)
GOM(__vsyslog_chk, vEEiipp)
// vtimes
//GOM(vwarn, vEEppp) //%%
// vwarnx
//GOM(vwprintf, iEEpp) //%%
//GO2(__vwprintf_chk, iEEvpp, my_vwprintf)
//GO(vwscanf, iEpp)
GOW(wait, iEp)
//GOW(__wait, iEp)
//GOW(wait3, iEpip)
//GOW(wait4, iEipip)
GOWM(waitid, iEEuupi)
GOW(waitpid, iEipi)
GOW(__waitpid, lElpi)
//GO(warn, vEppppppppp)
//GO(warnx, vEppppppppp)
//GOW(wcpcpy, pEpp)
// __wcpcpy_chk
//GOW(wcpncpy, pEpp)
// __wcpncpy_chk
GOW(wcrtomb, LEpip)
// __wcrtomb_chk
GOW(wcscasecmp, iEpp)
// __wcscasecmp_l
//GOW(wcscasecmp_l, iEppp)
GOW(wcscat, pEpp)
GO(__wcscat_chk, pEppL)
GO(wcschr, pEpi)
// wcschrnul    // Weak
GO(wcscmp, iEpp)
GOW(wcscoll, iEpp)
GO(__wcscoll_l, iEppa)
GOW(wcscoll_l, iEppa)
GO(wcscpy, pEpp)
GO(__wcscpy_chk, pEppL)
//GO(wcscspn, uEpp)
GO(wcsdup, pEp)
GO(wcsftime, LEpLpriiiiiiiiilt_)
GO(__wcsftime_l, LEpLppa)
GOW(wcsftime_l, LEpLppa)
GOW(wcslen, LEp)
GOW(wcsncasecmp, iEppL)
// __wcsncasecmp_l
//GOW(wcsncasecmp_l, iEppup)
GO(wcsncat, pEppL)
GO(__wcsncat_chk, pEppLL)
GO(wcsncmp, iEppL)
GOW(wcsncpy, pEppL)
GO(__wcsncpy_chk, pEppLL)
GOW(wcsnlen, LEpL)
GOW(wcsnrtombs, LEpBp_LLp)
// __wcsnrtombs_chk
GO(wcspbrk, pEpp)
GO(wcsrchr, pEpi)
//GOW(wcsrtombs, uEppup)
// __wcsrtombs_chk
//GO(wcsspn, uEpp)
GO(wcsstr, pEpp)
GO(wcstod, dEpBp_)
//GO(__wcstod_internal, dEppi)
// __wcstod_l
// wcstod_l // Weak
GO(wcstof, fEpBp_)
// __wcstof_internal
// __wcstof_l
// wcstof_l // Weak
// wcstoimax
GO(wcstok, pEppBp_)
GOM(wcstol, lEpBp_i)    //%noE
//GO(wcstold, DEpp)
// __wcstold_internal
// __wcstold_l
// wcstold_l    // Weak
//GO(__wcstol_internal, iEppii)
GO(wcstoll, IEpBp_i)
// __wcstol_l
// wcstol_l // Weak
// __wcstoll_internal
// __wcstoll_l
// wcstoll_l    // Weak
GO(wcstombs, LEppL)
// __wcstombs_chk
// wcstoq   // Weak
GOM(wcstoul, LEpBp_i)    //%noE
//GO(__wcstoul_internal, LEppii)
GO(wcstoull, UEpBp_i)
// __wcstoul_l
// wcstoul_l    // Weak
// __wcstoull_internal
// __wcstoull_l
// wcstoull_l   // Weak
// wcstoumax
// wcstouq  // Weak
// wcswcs   // Weak
//GO(wcswidth, iEpu)
GO(wcsxfrm, LEppL)
//GOW(wcsxfrm_l, uEppup)
GO(__wcsxfrm_l, LEppLa)
GO(wctob, iEu)
GO(wctomb, iEpi)
//GO(__wctomb_chk, iEpuL)
// wctrans  // Weak
// __wctrans_l
// wctrans_l    // Weak
GOW(wctype, uEp)
GO(__wctype_l, hEpa)
GOW(wctype_l, hEpa)
GO(wcwidth, iEu)
GO(wcslcat, LFppL)
GO(wcslcpy, LFppL)
GOW(wmemchr, pEpiL)
GO(wmemcmp, iEppL)
GOW(wmemcpy, pEppL)
GO(__wmemcpy_chk, pEppLL)
GOW(wmemmove, pEppL)
// __wmemmove_chk
// wmempcpy // Weak
// __wmempcpy_chk
GO(wmemset, pEpiL)
// __wmemset_chk
//GO(wordexp, iEppi)
//GO(wordfree, vEp)
// __woverflow
GOM(wprintf, iEEpV) //%%
//GOM(__wprintf_chk, iEEipV) //%%
GOW(write, lEipL)
//GOW(__write, lEipL)
GOWM(writev, lEEipi)
// wscanf
// __wuflow
// __wunderflow
// xdecrypt
// xdr_accepted_reply
// xdr_array
// xdr_authdes_cred
// xdr_authdes_verf
// xdr_authunix_parms
GO(xdr_bool, iEpp)
// xdr_bytes
// xdr_callhdr
// xdr_callmsg
// xdr_char
// xdr_cryptkeyarg
// xdr_cryptkeyarg2
// xdr_cryptkeyres
// xdr_des_block
// xdr_double
// xdr_enum
// xdr_float
// xdr_free
// xdr_getcredres
// xdr_hyper
GO(xdr_int, iEpp)
// xdr_int16_t
// xdr_int32_t
// xdr_int64_t
// xdr_int8_t
// xdr_keybuf
// xdr_key_netstarg
// xdr_key_netstres
// xdr_keystatus
// xdr_long
// xdr_longlong_t
// xdrmem_create
// xdr_netnamestr
GO(xdr_netobj, iEpbup_)
// xdr_opaque
// xdr_opaque_auth
// xdr_pmap
// xdr_pmaplist
// xdr_pointer
// xdr_quad_t
// xdrrec_create
// xdrrec_endofrecord
// xdrrec_eof
// xdrrec_skiprecord
// xdr_reference
// xdr_rejected_reply
// xdr_replymsg
// xdr_rmtcall_args
// xdr_rmtcallres
// xdr_short
// xdr_sizeof
// xdrstdio_create
// xdr_string
// xdr_u_char
// xdr_u_hyper
GO(xdr_u_int, iEpp)
// xdr_uint16_t
// xdr_uint32_t
// xdr_uint64_t
// xdr_uint8_t
// xdr_u_long
// xdr_u_longlong_t
// xdr_union
// xdr_unixcred
// xdr_u_quad_t
// xdr_u_short
// xdr_vector
GO(xdr_void, iEv)
// xdr_wrapstring
// xencrypt
GOM(__xmknod, iEEipup)
//GO(__xmknodat, iEiipip)
GO(__xpg_basename, tEp)
// __xpg_sigpause   // Weak
GO(__xpg_strerror_r, tEipu)
// xprt_register
// xprt_unregister
GOM(__xstat, iEEipp) //%%
GOM(__xstat64, iEEipp) //%%

// forcing a custom __gmon_start__ that does nothing
GOM(__gmon_start__, vEEv) //%%

GOM(_Jv_RegisterClasses, vEv)   //%%,noE dummy

GOM(__fdelt_chk, LEL) //%%,noE

GOM(getauxval, LEEL)  //%% implemented since glibc 2.16

//GOM(prlimit64, lEpupp)       //%%,noE
GOM(process_vm_readv, lEEipLpLL)
GOM(process_vm_writev, lEEipLpLL)
GOM(reallocarray, pEpLL)     //%%,noE
//GOM(__open_nocancel, iEEpOV) //%%
//GO2(__read_nocancel, lEipL, read)
GO2(__close_nocancel, iEi, close)

//GOM(mkstemps64, iEEpi)   //%% not always implemented
GO(getentropy, iEpL)

// not found (libitm???), but it seems OK to declare dummies:

GOWM(_ZGTtdlPv, vFp)    //%noE
GOWM(_ZGTtnaX, pFL) //%noE
GOWM(_ZGTtnam, pFL) //%noE
GOWM(_ZGTtnaj, pFu) //%noE
GOWM(_ITM_RU1, uEp)          //%%,noE
GOWM(_ITM_RU4, uEp)          //%%,noE
GOWM(_ITM_RU8, UEp)          //%%,noE
GOWM(_ITM_memcpyRtWn, vEppu) //%%,noE register(2)
GOWM(_ITM_memcpyRnWt, vEppu) //%%,noE register(2)
GOWM(_ITM_addUserCommitAction, vEEpup)
GOWM(_ITM_registerTMCloneTable, vEEpu)  //%%
GOWM(_ITM_deregisterTMCloneTable, vEEp) //%%

GOM(__umoddi3, UEUU)        //%%,noE
GOM(__udivdi3, UEUU)        //%%,noE
GOM(__divdi3, IEII)         //%%,noE
GOM(__poll_chk, iEpuiL)     //%%,noE

GO(fallocate64, iEiiII)

//DATAM(__libc_stack_end, 4)

//DATAM(___brk_addr, 4)
DATA(__libc_enable_secure, 4)

GOM(__register_frame_info, vEpp)  //%%,noE faked function
GOM(__deregister_frame_info, pEp) //%%,noE

GO(name_to_handle_at, iEipppi) // only glibc 2.14+, so may not be present...

//GOM(modify_ldt, iEEipL) // there is suposedly no glibc wrapper for this one

#ifdef ANDROID
//GOM(__libc_init, vEEpppp)
GO(__errno, pEv)
#else
// Those symbols don't exist in non-Android builds
//GOM(__libc_init,
//GO(__errno,
#endif

//GOM(lstat,
//GO(setprogname,
//GO(getprogname,

DATAM(__libc_single_threaded, 4)	//B type
