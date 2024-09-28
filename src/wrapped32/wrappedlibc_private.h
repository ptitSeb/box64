#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA) && defined(GOS))
#error Meh...
#endif

// FILE* is h (because stdin/stdout/stderr might be 64bits, maybe other special file can be too)
// locale_t needs special handling, with to_locale / from_locale (and is a / A)
// struct utimbuf is: LL
// struct timespec is: LL
// struct timeval is: LL
// struct itimerval is LLLL
// struct tm is: iiiiiiiiilt
// struct rusage is LLLLLLLLLLLLLLLLLL (2 timeval + 14 longs)
// time_t is: L
// socklen_t is u
// struct sockaddr is fine, no wrap needed

// a64l
GO(abort, vFv)
GO(abs, iFi)
GOW(accept, iFipp)
GOW(accept4, iFippi)
GOW(access, iFpi)
// acct
//GOW(addmntent, iFpp)
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
GO(alarm, iFu)
//GO2(aligned_alloc, pFuu, memalign)
GO(alphasort, iFpp)
GOM(alphasort64, iFEpp)
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
//GOW(argz_create_sep, iFpipp)
// argz_delete
// argz_extract // Weak
//GOW(argz_insert, iFpppp)
// __argz_next
//GOW(argz_next, pFpLp)
// argz_replace // Weak
// __argz_stringify
//GOW(argz_stringify, vFpLi)
GOM(asctime, pFriiiiiiiiilt_)   //%noE
GOW(asctime_r, pFriiiiiiiiilt_p)
//GOWM(asprintf, iFEppV)        //%%
//GOM(__asprintf, iFEppV)      //%%
//GOM(__asprintf_chk, iFEpipV) //%%
// __assert
GO(__assert_fail, vFppip)
//GO(__assert_perror_fail, vFipup)
GO(atof, dFp)
GO(atoi, iFp)
GO2(atol, lFp, atoi)
GO(atoll, IFp)
// authdes_create
// authdes_getucred
// authdes_pk_create
// _authenticate
// authnone_create
// authunix_create
// authunix_create_default
GOWM(backtrace, iFEpi)
//GO(__backtrace, iFpi)
//GO(__backtrace_symbols, pFpi)
GOWM(backtrace_symbols, pFEpi)
//GO(__backtrace_symbols_fd, vFpii)
//GOW(backtrace_symbols_fd, vFpii)
//GO(basename, pFp)
//GOW(bcmp, iFppL)
//GO(bcopy, vFppL)
// bdflush
GOW(bind, iFipu)
// bindresvport
//GOW(bindtextdomain, pFpp)
GOW(bind_textdomain_codeset, pFpp)
//GOW(brk, iFp)
// __bsd_getpgrp
// bsd_signal   // Weak
GOM(bsearch, pFEppLLp) //%%
GOW(btowc, iFi)
GOW(bzero, vFpL)
GO(__bzero, vFpL)
GOW(calloc, pFLL)
// callrpc
//GOW(canonicalize_file_name, pFp)
// capget
// capset
//GO(catclose, iFp)
//GO(catgets, pFpiip)
//GO(catopen, pFpi)
// cbc_crypt
//GO(cfgetispeed, uFp)
//GO(cfgetospeed, uFp)
//GO(cfmakeraw, vFp)
//GOW(cfree, vFp)
//GO(cfsetispeed, iFpu)
//GO(cfsetospeed, iFpu)
//GO(cfsetspeed, iFpi)
GOW(chdir, iFp)
//DATA(__check_rhosts_file, 4)
// chflags
// __chk_fail
GOW(chmod, iFpu)
GOW(chown, iFpuu)
//GO(chroot, iFp)
//GOW(clearenv, iFv)
GO(clearerr, vFS)
//GO(clearerr_unlocked, vFp)
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
GO(clock, LFv)
// clone    // Weak
// __clone
GOW(close, iFi)
// __close  // Weak
GOW(closedir, iFp)
GO(closelog, vFv)
//GO(__cmsg_nxthdr, pFpp)
//GO(confstr, uFipu)
// __confstr_chk
GOW(connect, iFipu)
GOW(__connect, iFipu)
// copysign // Weak
// copysignf    // Weak
// copysignl    // Weak
//GOW(creat, iFpu)
//GO(creat64, iFpu)
// create_module    // Weak
//GO(ctermid, pFp)
GO(ctime, pFrL_)
GO(ctime_r, pFrL_p)
DATAM(__ctype_b, 4)
GOM(__ctype_b_loc, pFEv)
GOW(__ctype_get_mb_cur_max, LFv)
DATAM(__ctype_tolower, 4)
GOM(__ctype_tolower_loc, pFEv)
DATAM(__ctype_toupper, 4)
GOM(__ctype_toupper_loc, pFEv)
// __curbrk // type B
//GO(cuserid, pFp)
GOM(__cxa_atexit, iFEppp) //%%
GOM(atexit, iFEp)           //%% just in case
GOM(__cxa_finalize, vFEp) //%%
DATAM(__cpu_model, 16)
//GOM(__cxa_thread_atexit_impl, iFEppp) //%%
// __cyg_profile_func_enter
// __cyg_profile_func_exit
// daemon
DATAV(daylight, 4)
// __daylight   // type B
//GOW(dcgettext, pFppi)
//GO(__dcgettext, pFppi)
//GOW(dcngettext, pFpppui)
// __default_morecore
// __default_rt_sa_restorer_v1
// __default_rt_sa_restorer_v2
// __default_sa_restorer_v1
// __default_sa_restorer_v2
// delete_module
// des_setparity
GOW(dgettext, pFpp)
GO(__dgettext, pFpp)
GO(difftime, dFLL)
//GO(dirfd, iFp)
GO(dirname, pFp)
GOS(div, pFpii) //%%,noE
// _dl_addr
GO2(dl_iterate_phdr, iFEpp, my_dl_iterate_phdr) //%%
// _dl_mcount_wrapper
// _dl_mcount_wrapper_check
// _dl_open_hook    // type B
// _dl_starting_up // Weak
// _dl_sym
// _dl_vsym
//GOW(dngettext, pFpppu)
//GOM(dprintf, iFEipV)
GOM(__dprintf_chk, iFEivpV)	//%%
GO(drand48, dFv)
// drand48_r
GOW(dup, iFi)
GOW(dup2, iFii)
GO(__dup2, iFii)
GO(dup3, iFiiO)
//GOW(duplocale, pFp)
GO(__duplocale, aFa)
// dysize
//GOW(eaccess, iFpi)
// ecb_crypt
// ecvt
//GO(ecvt_r, iFdipppL)
// endaliasent
// endfsent
GO(endgrent, vFv)
GO(endhostent, vFv)
GOW(endmntent, iFS)
// __endmntent
// endnetent
// endnetgrent
GO(endprotoent, vFv)
GO(endpwent, vFv)
// endrpcent
GO(endservent, vFv)
GO(endspent, vFv)
// endttyent
// endusershell
GOW(endutent, vFv)
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
GOM(epoll_create, iFEi)     //%% not needed, but used in syscall
GOM(epoll_create1, iFEO)    //%%
GOM(epoll_ctl, iFEiiip)     //%% align epool_event structure
// epoll_pwait
GOM(epoll_wait, iFEipii)    //%% need realign of epoll_event structure
// erand48
// erand48_r    // Weak
//GO(err, vFippppppppp)
// errno    // type B
GOM(__errno_location, pFEv)
//GOW(error, vFiippppppppp)  // Simple attempt: there is a vararg, but the alignment will/may be off if it tries some Double in the "printf" part
// error_at_line    // Weak
// error_message_count  // type B
// error_one_per_line   // type B
// error_print_progname // type B
//GO(errx, vFippppppppp)
//GO(ether_aton, pFp)
//GO(ether_aton_r, pFpp)
//GO(ether_hostton, iFpp)
//GO(ether_line, iFppp)
//GO(ether_ntoa, pFp)
//GO(ether_ntoa_r, pFpp)
//GO(ether_ntohost, iFpp)
//GOW(euidaccess, iFpi)
GO(eventfd, iFui)
//GO(eventfd_read, iFip)
//GO(eventfd_write, iFiU)
GO2(execl, iFEpV, my32_execv)
//GO2(execle, iFEpV, my_32execve)  // Nope! This one needs wrapping, because is char*, char*, ..., char*[]
GO2(execlp, iFpV, my32_execvp)
GOWM(execv, iFEpp)     //%%
GOM(execve, iFEppp)   //%% and this one too...
GOWM(execvp, iFEpp)
GO(exit, vFi)
GO(_exit, vFi)
GOW(_Exit, vFi)
//GOM(__explicit_bzero_chk, vFEpuu)    //%% not always defined
GO(faccessat, iFipii)
// fattach
//GO(__fbufsize, uFp)
GOW(fchdir, iFi)
// fchflags
GOW(fchmod, iFiu)
//GO(fchmodat, iFipui)
GOW(fchown, iFiuu)
//GO(fchownat, iFipuii)
GO(fclose, iFS)
GOW(fcloseall, iFv)
GOM(fcntl, iFEiiN)   //%% this also use a vararg for 3rd argument
GOM(__fcntl, iFEiiN) //%%
GOM(fcntl64, iFEiiN) //%%
//GO(fcvt, pFdipp)
//GO(fcvt_r, iFdipppL)
GO(fdatasync, iFi)
// fdetach
GO(fdopen, SFip)
GOW(fdopendir, pFi)
GOW(feof, iFS)
//GO(feof_unlocked, iFp)
GOW(ferror, iFS)
//GO(ferror_unlocked, iFp)
//GO(fexecve, iFipp)  //TODO: Check if needed to be wrapped, and target checked for x86 / native?
GOW(fflush, iFS)
//GO(fflush_unlocked, iFS)
GO(ffs, iFi)
// __ffs
GOW(ffsl, iFi)
GO(ffsll, iFI)
GOW(fgetc, iFS)
GOW(fgetc_unlocked, iFS)
// fgetgrent
// fgetgrent_r  // Weak
//GO(fgetpos, iFSBLii_)
GO(fgetpos64, iFSp)
// fgetpwent
// fgetpwent_r  // Weak
GOW(fgets, pFpiS)
GO(__fgets_chk, pFpLiS)
// fgetspent
// fgetspent_r  // Weak
//GO(fgets_unlocked, pFpip)
// __fgets_unlocked_chk
//GOW(fgetwc, iFp)
//GOW(fgetwc_unlocked, iFp)
//GO(fgetws, pFpip)
// __fgetws_chk
//GO(fgetws_unlocked, pFpip)
// __fgetws_unlocked_chk
//GO(fgetxattr, iFippu)
GO(fileno, iFS)
GOW(fileno_unlocked, iFS)
GOW(finite, iFd)
GO(__finite, iFd)
GOW(finitef, iFf)
// __finitef
// finitel  // Weak
// __finitel
// __flbf
//GO(flistxattr, iFipu)
GOW(flock, iFii)
//GOW(flockfile, vFp)
GOW(_flushlbf, vFv)
//GO(fmemopen, pFpup)
// fmtmsg
GO(fnmatch, iFppi)
GOM(fopen, SFEpp)           //%%
GOWM(fopen64, SFEpp)         //%%
//GOM(fopencookie, pFEpppppp) //%% last 4p are a struct with 4 callbacks...
GOWM(fork, iFEv)             //%%
GOM(__fork, iFEv)           //%%
// __fortify_fail
GOW(fpathconf, iFii)
//GO(__fpending, uFp)
GOM(fprintf, iFESpV) //%%
GOM(__fprintf_chk, iFESvpV) //%%
// __fpu_control    // type B
//GO(__fpurge, vFp)
GOW(fputc, iFiS)
//GO(fputc_unlocked, iFip)
GOW(fputs, iFpS)    // Weak
//GO(fputs_unlocked, iFpp)
GO(fputwc, iFiS)
//GO(fputwc_unlocked, iFip)
//GO(fputws, iFpp)
//GO(fputws_unlocked, iFpp)
GOW(fread, LFpLLS)
//GO(__freadable, iFp)
GO(__fread_chk, uFpLLLS)
//GO(__freading, iFp)
//GO(fread_unlocked, uFpuup)
//GO(__fread_unlocked_chk, uFpuuup)
GO(free, vFp)
GOM(freeaddrinfo, vFEp)
//DATAV(__free_hook, 4)
GO(freeifaddrs, vFp)
GOW(freelocale, vFA)
GO(__freelocale, vFA)
//GO(fremovexattr, iFip)
GO(freopen, SFppS)
GO(freopen64, SFppS)
// frexp    // Weak
// frexpf   // Weak
// frexpl   // Weak
GOM(fscanf, iFESpV)
GO(fseek, iFSli)
GO(fseeko, iFSli)
GO(fseeko64, iFSIi)
//GO(__fsetlocking, iFpi)
//GO(fsetpos, iFpp)
//GO(fsetpos64, iFpp)
//GO(fsetxattr, iFippui)
GOWM(fstatfs, iFip) //%%,noE
GOWM(fstatfs64, iFip)    //%%,noE
//GO(fstatvfs, iFip)
//GOW(fstatvfs64, iFip)   // alignment?
GOW(fsync, iFi)
GOWM(ftell, lFES)
GO(ftello, lFS)
GO(ftello64, IFS)
//GO(ftime, iFp)
GO(ftok, iFpi)
GOW(ftruncate, iFiu)
GOW(ftruncate64, iFiI)
//GOW(ftrylockfile, iFp)
//GOM(fts_children, pFEpi) //%%
//GOM(fts_close, iFEp)     //%%
//GOM(fts_open, pFEpip)    //%%
//GOM(fts_read, pFEp)      //%%
// fts_set
//GOM(ftw, iFEppi)         //%%
//GOM(ftw64, iFEppi)       //%%
//GOW(funlockfile, vFp)
//GO(futimens, iFip)
GOWM(futimes, iFEip)
//GO(futimesat, iFippp)
// fwide
//GOWM(fwprintf, iFEppV)   //%%
//GOM(__fwprintf_chk, iFEpvpV) //%%
//GO(__fwritable, iFp)
GOW(fwrite, LFpLLS)
//GO(fwrite_unlocked, uFpuup)
//GO(__fwriting, iFp)
// fwscanf
GOM(__fxstat, iFEiip)       //%%
GOM(__fxstat64, iFEiip)     //%% need reaalign of struct stat64
//GOM(__fxstatat, iFEiippi)   //%%
//GOM(__fxstatat64, iFEiippi) //%% struct stat64 again
// __gai_sigqueue
GO(gai_strerror, tFi)
// __gconv_get_alias_db
// __gconv_get_cache
// __gconv_get_modules_db
// gcvt
GOM(getaddrinfo, iFEpppp)
// getaliasbyname
// getaliasbyname_r
// getaliasent
// getaliasent_r
// get_avphys_pages // Weak
GOW(getc, iFS)
GOW(getchar, iFv)
GO(getchar_unlocked, iFv)
GOM(getcontext, iFEp)         //%%
//GOW(getc_unlocked, iFp)
//GO(get_current_dir_name, pFv)
GOW(getcwd, tFpL)
//GO(__getcwd_chk, pFpLL)
//GO(getdate, pFp)
// getdate_err  // type B
// getdate_r    // Weak
//GOW(getdelim, iFppip)
//GOW(__getdelim, iFppip)
// getdirentries
// getdirentries64
//GO(getdomainname, iFpu)
// __getdomainname_chk
GOW(getdtablesize, iFv)
GOW(getegid, iFv)
GO(getenv, tFp)
GOW(geteuid, pFv)
// getfsent
// getfsfile
// getfsspec
GOW(getgid, iFv)
//GO(getgrent, pFv)
// getgrent_r
//GO(getgrgid, pFu)
GOM(getgrgid_r, iFEuppup)
//GO(getgrnam, pFp)
GOM(getgrnam_r, iFEpppup)
//GO(getgrouplist, iFpipp)
GOW(getgroups, iFiu)
// __getgroups_chk
//GO(gethostbyaddr, pFpui)
//GO(gethostbyaddr_r, iFpuippupp)
GOM(gethostbyname, pFEp)
//GO(gethostbyname2, pFpi)
//GO(gethostbyname2_r, iFpippupp)
//GO(gethostbyname_r, iFpppupp)
//GO(gethostent, pFv)
//GO(gethostent_r, iFppupp)
// gethostid
GOW(gethostname, iFpL)
// __gethostname_chk
GOM(getifaddrs, iFEbp_)
// getipv4sourcefilter
//GOW(getitimer, iFip)
// get_kernel_syms  // Weak
//GOW(getline, iFppp)
GO(getloadavg, iFpi)
GO(getlogin, pFv)
GO(getlogin_r, iFpu)
// __getlogin_r_chk
GOM(getmntent, pFES)
// __getmntent_r
//GOW(getmntent_r, pFpppi)
// getmsg
// get_myaddress
GO(getnameinfo, iFpupLpLi)
// getnetbyaddr
// getnetbyaddr_r
// getnetbyname
// getnetbyname_r
// getnetent
// getnetent_r
// getnetgrent
// getnetgrent_r    // Weak
// getnetname
//GOW(get_nprocs, iFv)
//GOW(get_nprocs_conf, iFv)
//GOM(getopt, iFipp)             //%noE
//GOM(getopt_long, iFipppp)      //%noE
//GOM(getopt_long_only, iFipppp) //%noE
GOW(getpagesize, iFv)
GO(__getpagesize, iFv)
//GO(getpass, pFp)
GOW(getpeername, iFipp)
GOW(getpgid, uFu)
// __getpgid
GO(getpgrp, iFv)
// get_phys_pages   // Weak
GO(getpid, uFv)
GO(__getpid, uFv)
// getpmsg
GOW(getppid, uFv)
GO(getpriority, iFii)
//GOM(getrandom, iFEpuu)          //%%
GOM(getprotobyname, pFEp)
//GO(getprotobyname_r, iFpppup)
//GO(getprotobynumber, pFi)
//GO(getprotobynumber_r, iFippup)
//GO(getprotoent, pFv)
//GO(getprotoent_r, iFppup)
GOW(getpt, iFv)
// getpublickey
// getpw    // Weak
//GO(getpwent, pFv)
// getpwent_r
GOM(getpwnam, pFEp)
GOM(getpwnam_r, iFEpppup)
GOM(getpwuid, pFEu)
GOM(getpwuid_r, iFEuppLp)
//GOW(getresgid, iFppp)
GOW(getresuid, iFppp)
GOM(getrlimit, iFEip)
GO(getrlimit64, iFip)
// getrpcbyname
// getrpcbyname_r
// getrpcbynumber
// getrpcbynumber_r
// getrpcent
// getrpcent_r
// getrpcport
GOW(getrusage, iFiBLLLLLLLLLLLLLLLLLL_)
//GOW(gets, pFp)
// __gets_chk
// getsecretkey
//GO(getservbyname, pFpp)
//GO(getservbyname_r, iFppppup)
//GO(getservbyport, pFip)
//GO(getservbyport_r, iFipppup)
// getservent
//GO(getservent_r, iFppup)
GO(getsid, uFu)
GOW(getsockname, iFipp)
GOW(getsockopt, iFiiipp)
// getsourcefilter
//GO(getspent, pFv)
// getspent_r
//GO(getspnam, pFp)
// getspnam_r
// getsubopt
GOW(gettext, pFp)
GOW(gettimeofday, iFBLL_p)
//GO(__gettimeofday, iFpp)
// getttyent
// getttynam
GOW(getuid, uFv)
// getusershell
//GOW(getutent, pFv)
//GOW(getutent_r, iFpp)
//GOW(getutid, pFp)
//GOW(getutid_r, iFppp)
//GOW(getutline, pFp)
//GOW(getutline_r, iFppp)
// getutmp
// getutmpx
// getutxent
// getutxid
// getutxline
// getw
GO2(getwc, iFh, fgetwc)
GO(getwchar, iFv)
GO(getwchar_unlocked, iFv)
//GOW(getwc_unlocked, iFp)
//GO(getwd, pFp)
// __getwd_chk
//GO(getxattr, iFpppu)
//GOM(glob, iFEpipp)             //%%
//GOM(glob64, iFEpipp)           //%%
//GO(globfree, vFp)
//GO(globfree64, vFp)
// glob_pattern_p   // Weak
GOM(gmtime, pFEp)
//GO(__gmtime_r, pFpp)
GOWM(gmtime_r, pFEpp)
GO(gnu_dev_major, uFU)
GO(gnu_dev_makedev, UFii)       // dev_t seems to be a u64
GO(gnu_dev_minor, uFU)
GOW(gnu_get_libc_release, pFv)
GOW(gnu_get_libc_version, pFv)
// __gnu_mcount_nc
// __gnu_Unwind_Find_exidx
GO(grantpt, iFi)
// group_member // Weak
// gsignal  // Weak
// gtty
//GOW(hasmntopt, pFpp)
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
GO(htonl, uFu)
GO(htons, uFu)
GO(iconv, LFLbp_bL_Bp_BL_)
//GO(iconv_canonicalize, pFp)
GO(iconv_close, iFL)
GO(iconv_open, pFpp)
//GO(if_freenameindex, vFp)
//GO(if_indextoname, pFup)
//GO(if_nameindex, pFv)
//GO(if_nametoindex, uFp)
// imaxabs  // Weak
GOW(imaxdiv, IFII)
//DATA(in6addr_any, 16)  // type R
//DATA(in6addr_loopback, 16) // type R
// inb  // Weak
//GOW(index, pFpi)
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
GO(inet_addr, uFp)
GOW(inet_aton, iFpp)
// inet_lnaof
// inet_makeaddr
// inet_netof
//GO(inet_network, iFp)
// inet_nsap_addr
//GO(inet_nsap_ntoa, pFipp)
GO(inet_ntoa, tFu)
GO(inet_ntop, pFippu)
GO(inet_pton, iFipp)
//GO(initgroups, iFpi)
// init_module
// initstate    // Weak
//GOW(initstate_r, iFupup)
// inl  // Weak
// innetgr
GO(inotify_add_watch, iFipu)
GO(inotify_init, iFv)
GO(inotify_init1, iFi)
GO(inotify_rm_watch, iFii)
// insque
// __internal_endnetgrent
// __internal_getnetgrent_r
// __internal_setnetgrent
// inw  // Weak
//DATA(_IO_2_1_stderr_, 152)  //sizeof(struct _IO_FILE_plus)
//DATA(_IO_2_1_stdin_, 152)
//DATA(_IO_2_1_stdout_, 152)
//GO(_IO_adjust_column, uFupi)
// _IO_adjust_wcolumn
GO(ioctl, iFiLN)   //the vararg is just to have optional arg of various type, but only 1 arg
//GO(_IO_default_doallocate, iFS)
//GO(_IO_default_finish, vFSi)
//GO(_IO_default_pbackfail, iFSi)
//GO(_IO_default_uflow, iFS)
//GO(_IO_default_xsgetn, LFSpL)
//GO(_IO_default_xsputn, LFSpL)
//GO(_IO_doallocbuf, vFS)
//GO(_IO_do_write, iFSpL)
// _IO_fclose
// _IO_fdopen
// _IO_feof
// _IO_ferror
// _IO_fflush
// _IO_fgetpos
// _IO_fgetpos64
// _IO_fgets
//GO(_IO_file_attach, pFSi)
//GO(_IO_file_close, iFS)
//GO(_IO_file_close_it, iFS)
//GO(_IO_file_doallocate, iFS)
// _IO_file_finish
//GO(_IO_file_fopen, pFSppi)
//GO(_IO_file_init, vFS)
//DATA(_IO_file_jumps, 4)
//GO(_IO_file_open, pFSpiiii)
//GO(_IO_file_overflow, iFSi)
//GO(_IO_file_read, lFSpl)
//GO(_IO_file_seek, IFSIi)
//GO(_IO_file_seekoff, IFSIii)
//GO(_IO_file_setbuf, pFSpl)
//GOM(_IO_file_stat, iFESp)
//GO(_IO_file_sync, iFS)
//GO(_IO_file_underflow, iFS)
//GO(_IO_file_write, lFSpl)
//GO(_IO_file_xsputn, LFSpL)
//GO(_IO_flockfile, vFS)
//GO(_IO_flush_all, iFv)
//GO(_IO_flush_all_linebuffered, vFv)
// _IO_fopen
// _IO_fprintf  // Weak
// _IO_fputs
// _IO_fread
//GO(_IO_free_backup_area, vFS)
// _IO_free_wbackup_area
// _IO_fsetpos
// _IO_fsetpos64
// _IO_ftell
// _IO_ftrylockfile
//GO(_IO_funlockfile, vFS)
// _IO_fwrite
GO(_IO_getc, iFS)
// _IO_getline
//GO(_IO_getline_info, LFSpLiip)
// _IO_gets
//GO(_IO_init, vFSi)
//GO(_IO_init_marker, vFpS)
// _IO_init_wmarker
// _IO_iter_begin
// _IO_iter_end
// _IO_iter_file
// _IO_iter_next
// _IO_least_wmarker
//GO(_IO_link_in, vFp)
//DATA(_IO_list_all, 4)
// _IO_list_lock
// _IO_list_resetlock
// _IO_list_unlock
//GO(_IO_marker_delta, iFp)
//GO(_IO_marker_difference, iFpp)
//GO(_IO_padn, iFpii)
//GO(_IO_peekc_locked, iFp)
GOW(ioperm, iFuui)
GOW(iopl, iFi)
// _IO_popen
// _IO_printf
//GO(_IO_proc_close, iFS)
//GO(_IO_proc_open, pFSpp)
GO(_IO_putc, iFiS)
// _IO_puts
//GO(_IO_remove_marker, vFp)
//GO(_IO_seekmark, iFSpi)
//GO(_IO_seekoff, IFSIii)
//GO(_IO_seekpos, IFSIi)
// _IO_seekwmark
//GO(_IO_setb, vFSppi)
// _IO_setbuffer
// _IO_setvbuf
//GO(_IO_sgetn, uFppu)
// _IO_sprintf
//GO(_IO_sputbackc, iFSi)
// _IO_sputbackwc
// _IO_sscanf
//GO(_IO_str_init_readonly, vFppi)
//GO(_IO_str_init_static, vFppup)
//GO(_IO_str_overflow, iFSi)
//GO(_IO_str_pbackfail, iFSi)
//GO(_IO_str_seekoff, UFSUii)
//GO(_IO_str_underflow, iFS)
//GO(_IO_sungetc, iFp)
// _IO_sungetwc
//GO(_IO_switch_to_get_mode, iFp)
// _IO_switch_to_main_wget_area
// _IO_switch_to_wbackup_area
// _IO_switch_to_wget_mode
// _IO_ungetc
//GO(_IO_un_link, vFp)
//GO(_IO_unsave_markers, vFp)
// _IO_unsave_wmarkers
//GOM(_IO_vfprintf, iFEpppp) //%%
//GOM(_IO_vfscanf, iFEppp)   //%%
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
GO(isalnum, iFi)
// __isalnum_l
// isalnum_l    // Weak
GO(isalpha, iFi)
// __isalpha_l
// isalpha_l    // Weak
GO(isascii, iFi)
// __isascii_l  // Weak
// isastream
GOW(isatty, iFi)
GO(isblank, iFi)
// __isblank_l
// isblank_l    // Weak
GO(iscntrl, iFi)
// __iscntrl_l
// iscntrl_l    // Weak
// isctype  // Weak
// __isctype
GO(isdigit, iFi)
// __isdigit_l
// isdigit_l    // Weak
// isfdtype
GO(isgraph, iFi)
// __isgraph_l
// isgraph_l    // Weak
GOW(isinf, iFd)
GO(__isinf, iFd)
GOW(isinff, iFf)
GO(__isinff, iFf)
// isinfl   // Weak
// __isinfl
GO(islower, iFi)
// __islower_l
// islower_l    // Weak
GOW(isnan, iFd)
GO(__isnan, iFd)
GOW(isnanf, iFf)
GO(__isnanf, iFf)
// isnanl   // Weak
// __isnanl
//GOM(__isoc99_fscanf, iFEppV)  //%%
// __isoc99_fwscanf
// __isoc99_scanf
GOM(__isoc99_sscanf, iFEppV)  //%%
// __isoc99_swscanf
//GOM(__isoc99_vfscanf, iFEppp) //%%
// __isoc99_vfwscanf
// __isoc99_vscanf
//GOM(__isoc99_vsscanf, iFEppp) //%% TODO: check if ok
// __isoc99_vswscanf
// __isoc99_vwscanf
// __isoc99_wscanf
GO(isprint, iFi)
// __isprint_l
// isprint_l    // Weak
GO(ispunct, iFi)
// __ispunct_l
// ispunct_l    // Weak
GO(isspace, iFi)
// __isspace_l
// isspace_l    // Weak
GO(isupper, iFi)
// __isupper_l
// isupper_l    // Weak
GOW(iswalnum, iFi)
// __iswalnum_l
//GOW(iswalnum_l, iFip)
GOW(iswalpha, iFi)
// __iswalpha_l
//GOW(iswalpha_l, iFip)
GOW(iswblank, iFi)
// __iswblank_l
GOW(iswblank_l, iFip)
GOW(iswcntrl, iFi)
// __iswcntrl_l
GOW(iswcntrl_l, iFip)
GOW(iswctype, iFiu)
// __iswctype
GO(__iswctype_l, iFuLp)
// iswctype_l   // Weak
GOW(iswdigit, iFi)
// __iswdigit_l
//GOW(iswdigit_l, iFip)
GOW(iswgraph, iFi)
// __iswgraph_l
//GOW(iswgraph_l, iFip)
GOW(iswlower, iFi)
// __iswlower_l
//GOW(iswlower_l, iFip)
GOW(iswprint, iFi)
// __iswprint_l
//GOW(iswprint_l, iFip)
GOW(iswpunct, iFi)
// __iswpunct_l
//GOW(iswpunct_l, iFip)
GOW(iswspace, iFi)
// __iswspace_l
//GOW(iswspace_l, iFip)
GOW(iswupper, iFi)
// __iswupper_l
//GOW(iswupper_l, iFip)
GOW(iswxdigit, iFi)
// __iswxdigit_l
//GOW(iswxdigit_l, iFip)
GO(isxdigit, iFi)
// __isxdigit_l
// isxdigit_l   // Weak
// _itoa_lower_digits   // type R
// __ivaliduser
GO(jrand48, lFp)
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
GOW(kill, iFli)
GO(killpg, iFii)
// klogctl
// l64a
GO(labs, iFi)
// lchmod
GOW(lchown, iFpuu)
// lckpwdf  // Weak
// lcong48
// lcong48_r    // Weak
// ldexp    // Weak
// ldexpf   // Weak
// ldexpl   // Weak
//GOS(ldiv, pFEpii)               //%% return a struct, so address of stuct is on the stack, as a shadow 1st element
//GOM(lfind, pFEpppLp)            //%%
//GO(lgetxattr, iFpppu)
GOM(__libc_alloca_cutoff, iFEL) //%%
// __libc_allocate_rtsig
// __libc_allocate_rtsig_private
//GO(__libc_calloc, pFLL)
// __libc_clntudp_bufcreate
GO(__libc_current_sigrtmax, iFv)
// __libc_current_sigrtmax_private
GO(__libc_current_sigrtmin, iFv)
// __libc_current_sigrtmin_private
//GOM(__libc_dlclose, iFEp)       //%%
// __libc_dl_error_tsd
//GOM(__libc_dlopen_mode, pFEpi)  //%%
//GOM(__libc_dlsym, pFEpp)        //%%
// __libc_fatal
// __libc_fork
//GO(__libc_free, vFp)
// __libc_freeres
GOM(__libc_init_first, vFEipV)  //%%
DATAM(_libc_intl_domainname, 4)    // type R
//GO2(__libc_open, iFEpOu, my_open)
// __libc_longjmp
// __libc_mallinfo
//GO(__libc_malloc, pFL)
// __libc_mallopt
//GO(__libc_memalign, pFLL)
// __libc_pthread_init
//GO(__libc_pvalloc, pFL)
// __libc_pwrite
//GO2(__libc_read, lFipL, my_read) //%%,noE
//GO(__libc_realloc, pFpL)
// __libc_sa_len
// __libc_siglongjmp
GOM(__libc_start_main, iFEpippppp) //%%
GO2(__libc_sigaction, iFEipp, my32_sigaction) //%%
// __libc_system
// __libc_thread_freeres
//GO(__libc_valloc, pFL)
GOW(link, iFpp)
//GO(linkat, iFipipi)
GOW(listen, iFii)
//GO(listxattr, iFppu)
// llabs
// lldiv
//GO(llistxattr, iFppu)
// llseek   // Weak
// loc1 // type B
// loc2 // type B
GOWM(localeconv, pFEv)
GOM(localtime, pFEp)
GOWM(localtime_r, pFEpp)
GO(lockf, iFiiu)
GO(lockf64, iFiiI)
// locs // type B
GOM(longjmp, vFEpi)         //%%
GOM(_longjmp, vFEpi)        //%%
GOM(__longjmp_chk, vFEpi)   //%%
GO(lrand48, iFv)
// lrand48_r
//GO(lremovexattr, iFpp)
//GOM(lsearch, pFEpppLp)      //%%
GOW(lseek, iFiii)
// __lseek  // Weak
GOW(lseek64, IFiIi)
//GO(lsetxattr, iFpppui)
//GO(lutimes, iFpp)
GOM(__lxstat, iFEipp)       //%%
GOM(__lxstat64, iFEipp)     //%%
GO(madvise, iFpLi)
GOM(makecontext, iFEppiV)   //%%
//GOW(mallinfo, pFv)
GOM(malloc, pFL)            //%%,noE
// malloc_get_state // Weak
//DATAV(__malloc_hook, 4)
//DATAV(__malloc_initialize_hook, 4)
// malloc_set_state // Weak
// malloc_stats // Weak
GOW(malloc_trim, iFu)
GOW(malloc_usable_size, LFp)
GOW(mallopt, iFii)  // Weak
// mallwatch    // type B
//GO(mblen, iFpL)
//GOW(mbrlen, LFpLp)
//GO(__mbrlen, LFpLp)
GOW(mbrtowc, LFppLp)
GO(__mbrtowc, LFppLp)
//GOW(mbsinit, iFp)
GOW(mbsnrtowcs, LFpbp_LLp)
// __mbsnrtowcs_chk
GOW(mbsrtowcs, LFpbp_Lp)
// __mbsrtowcs_chk
GO(mbstowcs, LFppL)
// __mbstowcs_chk
//GO(mbtowc, iFppL)
// mcheck
// mcheck_check_all
// mcheck_pedantic
// _mcleanup
//GOWM(mcount, vFpp)   //%%,noE
// _mcount
GOW(memalign, pFLL)
//DATAV(__memalign_hook, 4)
//GOW(memccpy, pFppiL)
GO(memchr, pFpiL)
GO(memcmp, iFppL)
GO(memcpy, pFppL)
GO(__memcpy_chk, pFppLL)
// memfrob
//GO(memmem, pFpupu)
GO(memmove, pFppL)
GO(__memmove_chk, pFppLL)
GO(mempcpy, pFppL)
GO(__mempcpy, pFppL)
// __mempcpy_chk
// __mempcpy_small
GOW(memrchr, pFpiL)
GO(memset, pFpiL)
GO(__memset_chk, pFpiLL)
GO(mincore, iFpLp)
GOW(mkdir, iFpu)
//GO(mkdirat, iFipu)
GO(mkdtemp, pFp)
GO(mkfifo, iFpu)
//GO(mkfifoat, iFipu)
//GO(mkostemp, iFpi)
//GO(mkostemp64, iFpi)
GO(mkstemp, iFp)
GO(mkstemp64, iFp)
//GO(mktemp, pFp)
GO(mktime, LFriiiiiiiiilt_)
GO(mlock, iFpL)
//GO(mlockall, iFi)
GOM(mmap, pFEpLiiii)    //%%
GOM(mmap64, pFEpLiiiI)  //%%
// modf // Weak
// modff    // Weak
// modfl    // Weak
// moncontrol   // Weak
// monstartup   // Weak
// __monstartup
//DATA(__morecore, 4)
GOW(mount, iFpppup)
// mprobe
GOM(mprotect, iFEpLi)   //%%
// mrand48
// mrand48_r
GOWM(mremap, pFEpLLiN)	//%% 5th hidden paramerer "void* new_addr" if flags is MREMAP_FIXED
//GO(msgctl, iFiip)
//GOW(msgget, iFpi)
//GOW(msgrcv, lFipLli)
//GOW(msgsnd, iFipLi)
GOW(msync, iFpLi)
// mtrace
GO(munlock, iFpL)
//GO(munlockall, iFv)
GOM(munmap, iFEpL)       //%%
// muntrace
GOWM(nanosleep, iFrLL_BLL_)	 //%%,noE
// __nanosleep  // Weak
// netname2host
// netname2user
GOW(newlocale, aFipa)
GO(__newlocale, aFipa)
// nfsservctl
//GOM(nftw, iFEppii)       //%%
//GOM(nftw64, iFEppii)     //%%
//GOW(ngettext, pFppu)
GO(nice, iFi)
// _nl_default_dirname   // type R
// _nl_domain_bindings   // type B
GO(nl_langinfo, pFi)
GO(__nl_langinfo_l, pFia)
//GOW(nl_langinfo_l, pFup)
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
GOW(ntohl, uFu)
GOW(ntohs, uFu)
// ntp_adjtime  // Weak
// ntp_gettime
// _null_auth   // type B
// _obstack_allocated_p
//DATAM(obstack_alloc_failed_handler, 4)
//GOM(_obstack_begin, iFpLLpp) //%%,noE
// _obstack_begin_1
//DATA(obstack_exit_failure, 4)
//GOM(_obstack_free, vFpp)     //%%,noE
//GOM(obstack_free, vFpp)      //%%,noE
// _obstack_memory_used
//GOM(_obstack_newchunk, vFpi) //%%,noE
// obstack_printf   // Weak
// __obstack_printf_chk
//GOWM(obstack_vprintf, iFEpppp)  //%%
// __obstack_vprintf_chk
//GOWM(on_exit, iFEpp)  //%%
//GO2(__on_exit, iFEpp, my_on_exit)   //%%
GOWM(open, iFEpOu)    //%%
//GOWM(__open, iFEpOu)  //%%
//GO(__open_2, iFpO)
GOWM(open64, iFEpOu)  //%%
// __open64 // Weak
//GO(__open64_2, iFpO)
//GOW(openat, iFipON)
// __openat_2
//GOW(openat64, iFipON)
//GO(__openat64_2, iFipON)
// __open_catalog
GOW(opendir, pFp)
GO(openlog, vFpii)
// open_memstream
// open_wmemstream
//DATAB(optarg, 4)
//DATA(opterr, 4)
//DATA(optind, 4)
//DATA(optopt, 4)
// outb // Weak
// outl // Weak
// outw // Weak
//GO(__overflow, iFpi)
//GO(parse_printf_format, uFpup)
// passwd2des
GOW(pathconf, iFpi)
GOW(pause, iFv)
GO(pclose, iFS)
GO(perror, vFp)
// personality  // Weak
GOW(pipe, iFp)
// __pipe
//GOW(pipe2, iFpO) // assuming this works the same as pipe, so pointer for array of 2 int
// pivot_root
// pmap_getmaps
// pmap_getport
// pmap_rmtcall
// pmap_set
// pmap_unset
GOW(poll, iFpLi)    // poll have an array of struct as 1st argument
GO(__poll, iFpLi)
GO(popen, SFpp)
GO(posix_fadvise, iFiuui)
GO(posix_fadvise64, iFiuui)
GO(posix_fallocate, iFiii)
GO(posix_fallocate64, iFiII)
// posix_madvise
GOW(posix_memalign, iFBp_LL)
// posix_openpt // Weak
//GO(posix_spawn, iFpppppp)
// posix_spawnattr_destroy
// posix_spawnattr_getflags
// posix_spawnattr_getpgroup
// posix_spawnattr_getschedparam
// posix_spawnattr_getschedpolicy
// posix_spawnattr_getsigdefault
// posix_spawnattr_getsigmask
// posix_spawnattr_init
// posix_spawnattr_setflags
// posix_spawnattr_setpgroup
// posix_spawnattr_setschedparam
// posix_spawnattr_setschedpolicy
// posix_spawnattr_setsigdefault
// posix_spawnattr_setsigmask
// posix_spawn_file_actions_addclose
//GO(posix_spawn_file_actions_adddup2, iFpii)
//GO(posix_spawn_file_actions_addopen, iFpipii)
//GO(posix_spawn_file_actions_destroy, iFp)
//GO(posix_spawn_file_actions_init, iFp)
//GOM(posix_spawnp, iFEpppppp) //%%
//GO(ppoll, iFpupp)
GOW(prctl, iFiLLLL)
GOW(pread, lFipLl)
//GOW(pread64, lFipLI)
// __pread64    // Weak
// __pread64_chk
//GOM(preadv64, lFEipiI)  //%% not always present
// __pread_chk
GOM(printf, iFEpV) //%%
GOM(__printf_chk, iFEvpV) //%%
//GO(__printf_fp, iFppp)  // does this needs aligment?
// printf_size
// printf_size_info
// profil   // Weak
// __profile_frequency
//DATAM(__progname, 4)
//DATAM(__progname_full, 4)
DATAM(program_invocation_name, 4)
DATAM(program_invocation_short_name, 4)
//GOW(pselect, iFippppp)
// psignal
//GO(ptrace, iFiupp)  // will that work???
//GO(ptsname, pFi)
//GOW(ptsname_r, iFipu)
// __ptsname_r_chk
GOW(putc, iFip)
GO(putchar, iFi)
GO(putchar_unlocked, iFi)
//GO(putc_unlocked, iFip)
GO(putenv, iFp)
// putgrent
// putmsg
// putpmsg
// putpwent
GOW(puts, iFp)
// putspent
//GOW(pututline, pFp)
// pututxline
// putw
GO(putwc, iFiS)
// putwchar
GO(putwchar_unlocked, iFi)
//GO(putwc_unlocked, iFip)
// pvalloc  // Weak
// pwrite   // Weak
//GOW(pwrite64, lFipLI)
// __pwrite64   // Weak
//GOM(pwritev64, lFEipiI)  //%% not always present
// qecvt
#ifdef HAVE_LD80BITS
//GO(qecvt_r, iFDipppL)
#else
//GO(qecvt_r, iFKipppL)
#endif
// qfcvt
#ifdef HAVE_LD80BITS
//GO(qfcvt_r, iFDipppL)
#else
//GO(qfcvt_r, iFKipppL)
#endif
// qgcvt
GOM(qsort, vFEpLLp) //%%
//GOM(qsort_r, vFEpLLpp) //%%
// query_module // Weak
//GO(quotactl, iFipip)
GO(raise, iFi)
GO(rand, iFv)
GOW(random, iFv)
//GOW(random_r, iFpp)
//GO(rand_r, iFp)
//GOW(rawmemchr, pFpi)
//GO(__rawmemchr, pFpi)
// rcmd
// rcmd_af
// __rcmd_errstr    // type B
GOM(read, lFipL) //%%,noE
//GOW(__read, lFipL)
// readahead    // Weak
//GO(__read_chk, lFipLL)
GOWM(readdir, pFEp)  //%%
GO(readdir64, pFp)  // check if alignement is correct
// readdir64_r
//GOM(readdir_r, iFEppp)  //%% should also be weak
GOM(readlink, iFEppL) //%%
//GOM(readlinkat, iFEippL)
// __readlinkat_chk
// __readlink_chk
//GO(readv, lFipi)
GO(realloc, pFpL)
//DATAV(__realloc_hook, 4)
GOM(realpath, pFEpp) //%%
GO2(__realpath_chk, pFEppv, my32_realpath)
// reboot
// re_comp  // Weak
// re_compile_fastmap   // Weak
//GOW(re_compile_pattern, pFpup)
GO(recv, lFipLi)
GO(__recv_chk, iFipLLi)
GOW(recvfrom, lFipLipp)
// __recvfrom_chk
//GOM(recvmmsg, iFEipuup)    //%% actual recvmmsg is glibc 2.12+. The syscall is Linux 2.6.33+, so use syscall...
GOWM(recvmsg, lFEipi)
// re_exec  // Weak
//GOW(regcomp, iFppi)
//GOW(regerror, uFippu)
//GO(regexec, iFppupi)
//GOW(regfree, vFp)
GOM(__register_atfork, iFEpppp) //%%
// register_printf_function // Weak
// registerrpc
// remap_file_pages // Weak
//GOW(re_match, iFppiip)
// re_match_2   // Weak
GO(remove, iFp)
//GO(removexattr, iFpp)
// remque
GO(rename, iFpp)
//GO(renameat, iFipip)
//GO(renameat2, iFipipu)
// _res // type B
//GOW(re_search, iFppiiip)
//GOW(re_search_2, iFppipiiipi)
// re_set_registers // Weak
GOW(re_set_syntax, uFu)
// _res_hconf   // type B
//GO(__res_iclose, vFpi)
GO(__res_init, iFv)
//GO(__res_maybe_init, iFpi)
//GO(__res_nclose, vFp)
//GO(__res_ninit, iFp)
//DATA(__resp, 4)
// __res_randomid
//GO(__res_state, pFv)
//DATA(re_syntax_options, 4)    // type B
// revoke
GO(rewind, vFS)
//GO(rewinddir, vFp)
// rexec
// rexec_af
// rexecoptions // type B
//GOW(rindex, pFpi)
GOW(rmdir, iFp)
//GO(readdir64_r, iFppp)  // is this present?
// rpc_createerr    // type B
// _rpc_dtablesize
// __rpc_thread_createerr
// __rpc_thread_svc_fdset
// __rpc_thread_svc_max_pollfd
// __rpc_thread_svc_pollfd
//GO(rpmatch, iFp)
// rresvport
// rresvport_af
// rtime
// ruserok
// ruserok_af
// ruserpass
GOW(sbrk, pFl)
GO(__sbrk, pFl)
// scalbn   // Weak
// scalbnf  // Weak
// scalbnl  // Weak
GOM(scandir, iFEpppp) //%%
GOM(scandir64, iFEpppp) //%%
//GO2(scanf, iFpp, vscanf)
//GO(__sched_cpualloc, pFu)   //TODO: check, return cpu_set_t* : should this be aligned/changed?
GO(__sched_cpucount, iFup)
//GO(__sched_cpufree, vFp)
GO(sched_getaffinity, iFiup)
GO(sched_getcpu, iFv)
//GO(__sched_getparam, iFip)
//GOW(sched_getparam, iFip)
GO(__sched_get_priority_max, iFi)
GOW(sched_get_priority_max, iFi)
GO(__sched_get_priority_min, iFi)
GOW(sched_get_priority_min, iFi)
GO(__sched_getscheduler, iFi)
GOW(sched_getscheduler, iFi)
//GOW(sched_rr_get_interval, iFip)
GO(sched_setaffinity, iFiup)
//GOW(sched_setparam, iFip)
GO(__sched_setscheduler, iFiip)
GOW(sched_setscheduler, iFiip)
GO(__sched_yield, iFv)
GOW(sched_yield, iFv)
GO(__secure_getenv, pFp)
GO(secure_getenv, pFp)
// seed48
// seed48_r // Weak
//GO(seekdir, vFpi)
GOW(select, iFippprLL_)
GO(__select, iFippprLL_)
GO(semctl, iFiiiN)
GOW(semget, iFuii)
GOW(semop, iFipL)
//GO(semtimedop, iFipup)
GOW(send, lFipLi)
// __send   // Weak
GO(sendfile, lFiibp_L)
GO(sendfile64, lFiipL)
GOWM(sendmsg, lFEipi)
//GOM(__sendmmsg, iFEipuu)    //%% actual __sendmmsg is glibc 2.14+. The syscall is Linux 3.0+, so use syscall...
GOW(sendto, lFipLipu)
// setaliasent
GOW(setbuf, vFSp)
//GOW(setbuffer, vFppL)
GOM(setcontext, iFEp) //%%
// setdomainname
GO(setegid, iFu)
GOW(setenv, iFppi)
// _seterr_reply
GO(seteuid, iFu)
// setfsent
// setfsgid
// setfsuid
GOW(setgid, iFu)
GO(setgrent, vFv)
//GO(setgroups, iFup)
GO(sethostent, vFi)
// sethostid
//GO(sethostname, iFpu)
// setipv4sourcefilter
GOW(setitimer, iFirLLLL_BLLLL_)
GOM(setjmp, iFEp) //%%
GOM(_setjmp, iFEp) //%%
//GO(setlinebuf, vFp)
GO(setlocale, tFip)
// setlogin
GO(setlogmask, iFi)
GOW(setmntent, SFpp)
// __setmntent
// setnetent
// setnetgrent
GOW(setpgid, iFuu)
// __setpgid
GO(setpgrp, iFv)
GO(setpriority, iFiii)
GO(setprotoent, vFi)
GO(setpwent, vFv)
GOW(setregid, iFuu)
GOW(setresgid, iFuuu)
GOW(setresuid, iFuuu)
GOW(setreuid, iFuu)
GOM(setrlimit, iFEip)
GO(setrlimit64, iFip)
// setrpcent
// setservent
GOW(setsid, iFv)
GOW(setsockopt, iFiiipu)
// setsourcefilter
GO(setspent, vFv)
// setstate // Weak
//GOW(setstate_r, iFpp)
//GOW(settimeofday, iFpp)
// setttyent
GOW(setuid, iFu)
// setusershell
GOW(setutent, vFv)
// setutxent
GOW(setvbuf, iFhpiL)
//GO(setxattr, iFpppui)
// sgetspent
// sgetspent_r  // Weak
GOW(shmat, pFipi)
//GOW(shmctl, iFiip)
GOW(shmdt, iFp)
GOW(shmget, iFuui)
GOW(shutdown, iFii)
GOWM(sigaction, iFEipp)    //%%
//GOWM(__sigaction, iFEipp)  //%%
GO(sigaddset, iFpi)
// __sigaddset
GOWM(sigaltstack, iFEpp)   //%%
// sigandset
GOW(sigblock, iFi)
GO(sigdelset, iFpi)
// __sigdelset
GO(sigemptyset, iFp)
GO(sigfillset, iFp)
GO(siggetmask, iFv)
// sighold
// sigignore
GO(siginterrupt, iFii)  // no need to wrap this one?
// sigisemptyset
//GO(sigismember, iFpi)
// __sigismember
GOM(siglongjmp, vFEip) //%%
GOW2(signal, pFEip, my_signal)
// signalfd
GO(__signbit, iFd)
GO(__signbitf, iFf)
// sigorset
// sigpause // Weak
// __sigpause
//GO(sigpending, iFp)
GOW(sigprocmask, iFipp)
// sigqueue // Weak
// sigrelse
// sigreturn    // Weak
GOM(sigset, pFEip) //%%
GOM(__sigsetjmp, iFEp) //%%
GOW(sigsetmask, iFi)
// sigstack
GOW(sigsuspend, iFp)
// __sigsuspend
//GOW(sigtimedwait, iFppp)
//GOW(sigvec, iFipp)
//GOW(sigwait, iFpp)
//GOW(sigwaitinfo, iFpp)
GOW(sleep, uFu)
GOM(snprintf, iFEpLpV) //%%
GOM(__snprintf_chk, iFEpLiipV) //%%
//GOM(__snprintf, iFEpLpV) //%%
// sockatmark
GOW(socket, iFiii)
GOW(socketpair, iFiiip)
//GO(splice, iFipipuu)
GOM(sprintf, iFEppV) //%%
GOM(__sprintf_chk, iFEpvvpV) //%%
// sprofil  // Weak
GOW(srand, vFu)
GO(srand48, vFi)
// srand48_r    // Weak
GOW(srandom, vFu)
//GOW(srandom_r, iFup)
GOM(sscanf, iFEppV) //%%
// ssignal  // Weak
// sstk
GOM(__stack_chk_fail, vFEv) //%%
//GOM(lstat64, iFpp)	//%%,noE
//GOM(stat64, iFpp)	//%%,noE
GOWM(statfs, iFpp)  //%%,noE
// __statfs
GOWM(statfs64, iFpp)     //%%,noE
GOM(statvfs, iFEpp)
GOWM(statvfs64, iFESp)
DATAM(stderr, 4)
DATAM(stdin, 4)
DATAM(stdout, 4)
// step // Weak
// stime
GO(stpcpy, pFpp)
// __stpcpy
GO(__stpcpy_chk, pFppL)
// __stpcpy_small
//GOW(stpncpy, pFppL)
//GO(__stpncpy, pFppL)
//GO(__stpncpy_chk, pFppLL)
GOW(strcasecmp, iFpp)
//GO(__strcasecmp, iFpp)
// __strcasecmp_l
// strcasecmp_l // Weak
GOW(strcasestr, pFpp)
GO(__strcasestr, pFpp)
GO(strcat, pFpp)
GO(__strcat_chk, pFppL)
GO(strchr, pFpi)
//GOW(strchrnul, pFpi)
GO(strcmp, iFpp)
GO(strcoll, iFpp)
GO(__strcoll_l, iFppa)
//GOW(strcoll_l, iFppp)
GO(strcpy, pFpp)
GO(__strcpy_chk, pFppL)
// __strcpy_small
GO(strcspn, LFpp)
// __strcspn_c1
// __strcspn_c2
// __strcspn_c3
GOW(strdup, pFp)
GO(__strdup, pFp)
GO(strerror, tFi)
//GO(strerror_l, pFip)
GO(__strerror_r, pFipL)
GOW(strerror_r, pFipL)
//GO(strfmon, lFpLpppppppppp) //vaarg, probably needs align, there are just double...
// __strfmon_l
// strfmon_l    // Weak
// strfry
GO(strftime, LFpLpriiiiiiiiilt_)
GO(__strftime_l, LFpLpriiiiiiiiilt_a)
GOW(strftime_l, LFpLpriiiiiiiiilt_a)
GO(strlen, LFp)
GOW(strncasecmp, iFppL)
// __strncasecmp_l
// strncasecmp_l    // Weak
GO(strncat, pFppL)
GO(__strncat_chk, pFppLL)
GO(strncmp, iFppL)
GO(strncpy, pFppL)
GO(__strncpy_chk, pFppLL)
GOW(strndup, pFpL)
GO(__strndup, pFpL)
GO(strnlen, LFpL)
GO(strpbrk, pFpp)
// __strpbrk_c2
// __strpbrk_c3
GO(strptime, pFppriiiiiiiiilt_)
// strptime_l   // Weak
GO(strrchr, pFpi)
//GOW(strsep, pFpp)
// __strsep_1c
// __strsep_2c
// __strsep_3c
// __strsep_g
GO(strsignal, tFi)
GO(strspn, LFpp)
// __strspn_c1
// __strspn_c2
// __strspn_c3
GO(strstr, pFpp)
GO(strtod, dFpBp_)
GO(__strtod_internal, dFpBp_i)
GO(__strtod_l, dFpBp_a)
GOW(strtod_l, dFpBp_a)
GO(strtof, fFpBp_)
GO(__strtof_internal, fFpBp_p)
GO(__strtof_l, fFpBp_a)
//GOW(strtof_l, fFppu)
//GO(strtoimax, IFppi)
GO(strtok, pFpp)
GO(__strtok_r, pFppbp_)
GOW(strtok_r, pFppbp_)
// __strtok_r_1c
GOM(strtol, lFpBp_i)    //%%,noE
#ifdef HAVE_LD80BITS
//GO(strtold, DFpp)
//GO(__strtold_internal, DFppi)
GO(__strtold_l, DFpBp_a)
GOW(strtold_l, DFpBp_a)
#else
//GO(strtold, KFpp)
//GO2(__strtold_internal, KFppi, __strtod_internal)
GO2(__strtold_l, KFpBp_a, __strtod_l)
GOW2(strtold_l, KFpBp_a, strtod_l)
#endif
GO2(__strtol_internal, lFpBp_i, my32_strtol)    //%%,noE
GO(strtoll, IFpBp_i)
//GO(__strtol_l, lFppiip)
//GOW(strtol_l, lFppiip)
GO(__strtoll_internal, IFpBp_ii)
//GO(__strtoll_l, IFppip)
//GOW(strtoll_l, IFppip)
//GOW(strtoq, IFppi)  // is that ok?
GOM(strtoul, LFpBp_i)   //%%,noE
GO2(__strtoul_internal, LFpBp_iv, my32_strtoul) //%%,noE
GO(strtoull, UFpBp_i)
//GO(__strtoul_l, uFppip)
//GOW(strtoul_l, LFppip)
GO(__strtoull_internal, UFpBp_ii)
//GO(__strtoull_l, UFppip)
//GOW(strtoull_l, UFppip)
//GO(strtoumax, UFppi)
//GOW(strtouq, UFppi) // ok?
//GOW(strverscmp, iFpp)
// __strverscmp
//GO(strxfrm, uFppu)
GO(__strxfrm_l, LFppLa)
//GO(strxfrm_l, uFppup)
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
//GO(swab, vFppi)
GOM(swapcontext, iFEpp) //%%
// swapoff  // Weak
// swapon   // Weak
GOM(swprintf, iFEpupV) //%%
GOM(__swprintf_chk, iFEpuiupV) //%%
GOM(swscanf, iFEppV)
GOW(symlink, iFpp)
//GO(symlinkat, iFpip)
GO(sync, vFv)
GO(syncfs, iFi)
// sync_file_range
GOM(syscall, uFEV) //%%
GOW(sysconf, lFi)
GO(__sysconf, lFi)
// sysctl   // Weak
//GO(__sysctl, iFp)
//DATA(_sys_errlist, 4)
//DATA(sys_errlist, 4)
GOM(sysinfo, iFp)   //%noE
//GO2(syslog, vFipV, vsyslog)
//GO2(__syslog_chk, vFiipV, __vsyslog_chk)
//DATA(_sys_nerr, 4)    // type R
//DATA(sys_nerr, 4) // type R
//DATA(sys_sigabbrev, 4)
//DATA(_sys_siglist, 4)
//DATA(sys_siglist, 4)
GOW(system, iFp)          // Need to wrap to use box86 if needed?
GO2(__sysv_signal, pFEip, my___sysv_signal) //%%
//GOWM(sysv_signal, pFEip)  //%%
GOW(tcdrain, iFi)
GO(tcflow, iFii)
GO(tcflush, iFii)
GOW(tcgetattr, iFip)
GO(tcgetpgrp, iFi)
// tcgetsid
GO(tcsendbreak, iFii)
GO(tcsetattr, iFiip)
GO(tcsetpgrp, iFii)
// tdelete  // Weak
// tdestroy // Weak
// tee
//GO(telldir, iFp)
//GO(tempnam, pFpp)
//GOW(textdomain, pFp)
// tfind    // Weak
GO(time, LFrL_)
GO(timegm, LFriiiiiiiiilt_)
// timelocal    // Weak
GO(timerfd_create, iFii)
//GO(timerfd_gettime, iFip)
//GO(timerfd_settime, iFiipp)
//GOW(times, iFp)
DATAM(timezone, 4)
//DATAB(__timezone, 4)   // type B
GO(tmpfile, pFv)
//GO(tmpfile64, pFv)
GO(tmpnam, pFp)
//GO(tmpnam_r, pFp)
GO(toascii, iFi)
// __toascii_l  // Weak
GO(tolower, iFi)
// _tolower
// __tolower_l
//GOW(tolower_l, iFip)
GO(toupper, iFi)
// _toupper
// __toupper_l
//GOW(toupper_l, iFip)
// towctrans    // Weak
// __towctrans
// __towctrans_l
// towctrans_l  // Weak
GO(towlower, iFi)
GO(__towlower_l, iFia)
GOW(towlower_l, iFia)
GO(towupper, iFi)
GO(__towupper_l, iFia)
GOW(towupper_l, iFia)
// tr_break
//GOW(truncate, iFpu)
//GO(truncate64, iFpU)
// tsearch  // Weak
//GO(ttyname, pFi)
//GOW(ttyname_r, iFipu)
// __ttyname_r_chk
// ttyslot
// twalk    // Weak
//DATAV(tzname, 4)
//DATA(__tzname, 4)
GOWM(tzset, vFv)    //%%,noE
// ualarm
//GO(__uflow, iFp)
// ulckpwdf // Weak
// ulimit   // Weak
GOW(umask, uFu)
//GOW(umount, iFp)
//GOW(umount2, iFpi)
GOWM(uname, iFp) //%%,noE
//GO(__underflow, iFp)
GOW(ungetc, iFiS)
GO(ungetwc, iFiS)
GOW(unlink, iFp)
//GO(unlinkat, iFipi)
GO(unlockpt, iFi)
GOW(unsetenv, iFp)
// unshare
//GOW(updwtmp, vFpp)
// updwtmpx
// uselib
GOW(uselocale, aFa)
GO(__uselocale, aFa)
// user2netname
GO(usleep, iFu)
// ustat
GO(utime, iFprLL_)
//GO(utimensat, iFippi)
GOWM(utimes, iFEpp)
//GOW(utmpname, iFp)
// utmpxname
//GOW(valloc, pFu)
GOM(vasprintf, iFEppp) //%%
GOM(__vasprintf_chk, iFEpipp) //%%
// vdprintf // Weak
// __vdprintf_chk
//GOM(verr, vFEpV) //%%
// verrx
//GO(versionsort, iFpp)
//GO(versionsort64, iFpp) //need to align dirent64?
GOWM(vfork, iFEv) //%%
// __vfork
GOM(vfprintf, iFESpp) //%%
GOM(__vfprintf_chk, iFESvpp) //%%
//GOWM(vfscanf, iFEppp)  //%%
// __vfscanf
//GOWM(vfwprintf, iFEppp)    //%%
//GO2(__vfwprintf_chk, iFEpvpp, my_vfwprintf)
//GOW(vfwscanf, iFppp)
// vhangup
// vlimit
// vmsplice
GOM(vprintf, iFEpp)               //%%
GOM(__vprintf_chk, iFEvpp)        //%%
// vscanf   // Weak
GOWM(vsnprintf, iFEpLpp)         //%%
GOWM(__vsnprintf, iFEpLpp)       //%%
GOM(__vsnprintf_chk, iFEpLvvpp)  //%%
GOWM(vsprintf, iFEppp)            //%%
GOM(__vsprintf_chk, iFEpiLpp)     //%% 
//GOM(vsscanf, iFEppp) //%%
// __vsscanf    // Weak
GOWM(vswprintf, iFEpLpp)         //%%
GOWM(__vswprintf, iFEpLpp)         //%%
GOWM(__vswprintf_chk, iFEpLiLppp) //%%
GOM(vswscanf, iFEppp)
//GO(vsyslog, vFipp)
//GO(__vsyslog_chk, vFiipp)
// vtimes
//GOM(vwarn, vFEppp) //%%
// vwarnx
//GOM(vwprintf, iFEpp) //%%
//GO2(__vwprintf_chk, iFEvpp, my_vwprintf)
//GO(vwscanf, iFpp)
GOW(wait, iFp)
//GOW(__wait, iFp)
//GOW(wait3, iFpip)
//GOW(wait4, iFipip)
GOW(waitid, iFuupi) // might need to wrap "p", it's a siginfo_t *
GOW(waitpid, lFlpi)
GOW(__waitpid, lFlpi)
//GO(warn, vFppppppppp)
//GO(warnx, vFppppppppp)
//GOW(wcpcpy, pFpp)
// __wcpcpy_chk
//GOW(wcpncpy, pFpp)
// __wcpncpy_chk
GOW(wcrtomb, LFpip)
// __wcrtomb_chk
GOW(wcscasecmp, iFpp)
// __wcscasecmp_l
//GOW(wcscasecmp_l, iFppp)
GOW(wcscat, pFpp)
GO(__wcscat_chk, pFppL)
GO(wcschr, pFpi)
// wcschrnul    // Weak
GO(wcscmp, iFpp)
GOW(wcscoll, iFpp)
GO(__wcscoll_l, iFppa)
GOW(wcscoll_l, iFppa)
GO(wcscpy, pFpp)
GO(__wcscpy_chk, pFppL)
//GO(wcscspn, uFpp)
GO(wcsdup, pFp)
//GO(wcsftime, LFpLpp)
GO(__wcsftime_l, LFpLppa)
GOW(wcsftime_l, LFpLppa)
GOW(wcslen, LFp)
GOW(wcsncasecmp, iFppL)
// __wcsncasecmp_l
//GOW(wcsncasecmp_l, iFppup)
GO(wcsncat, pFppL)
GO(__wcsncat_chk, pFppLL)
GO(wcsncmp, iFppL)
GOW(wcsncpy, pFppL)
GO(__wcsncpy_chk, pFppLL)
GOW(wcsnlen, LFpL)
GOW(wcsnrtombs, LFpBp_LLp)
// __wcsnrtombs_chk
//GO(wcspbrk, pFpp)
GO(wcsrchr, pFpi)
//GOW(wcsrtombs, uFppup)
// __wcsrtombs_chk
//GO(wcsspn, uFpp)
GO(wcsstr, pFpp)
GO(wcstod, dFpBp_)
//GO(__wcstod_internal, dFppi)
// __wcstod_l
// wcstod_l // Weak
GO(wcstof, fFpBp_)
// __wcstof_internal
// __wcstof_l
// wcstof_l // Weak
// wcstoimax
//GO(wcstok, pFppp)
GOM(wcstol, iFpBp_i)    //%noE
//GO(wcstold, DFpp)
// __wcstold_internal
// __wcstold_l
// wcstold_l    // Weak
//GO(__wcstol_internal, iFppii)
GO(wcstoll, IFpBp_i)
// __wcstol_l
// wcstol_l // Weak
// __wcstoll_internal
// __wcstoll_l
// wcstoll_l    // Weak
GO(wcstombs, LFppL)
// __wcstombs_chk
// wcstoq   // Weak
GOM(wcstoul, iFpBp_i)    //%noE
//GO(__wcstoul_internal, LFppii)
GO(wcstoull, UFpBp_i)
// __wcstoul_l
// wcstoul_l    // Weak
// __wcstoull_internal
// __wcstoull_l
// wcstoull_l   // Weak
// wcstoumax
// wcstouq  // Weak
// wcswcs   // Weak
//GO(wcswidth, iFpu)
//GO(wcsxfrm, uFppu)
//GOW(wcsxfrm_l, uFppup)
GO(__wcsxfrm_l, LFppLa)
GO(wctob, iFi)
//GO(wctomb, iFpi)
//GO(__wctomb_chk, iFpuL)
// wctrans  // Weak
// __wctrans_l
// wctrans_l    // Weak
//GOW(wctype, uFp)
GO(__wctype_l, uFpa)
GOW(wctype_l, uFpa)
//GO(wcwidth, iFu)
GOW(wmemchr, pFpuL)
GO(wmemcmp, iFppL)
GOW(wmemcpy, pFppL)
GO(__wmemcpy_chk, pFppLL)
GOW(wmemmove, pFppL)
// __wmemmove_chk
// wmempcpy // Weak
// __wmempcpy_chk
GO(wmemset, pFpuL)
// __wmemset_chk
//GO(wordexp, iFppi)
//GO(wordfree, vFp)
// __woverflow
GOM(wprintf, iFEpV) //%%
//GOM(__wprintf_chk, iFEipV) //%%
GOW(write, lFipL)
//GOW(__write, lFipL)
GOWM(writev, lFEipi)
// wscanf
// __wuflow
// __wunderflow
// xdecrypt
// xdr_accepted_reply
// xdr_array
// xdr_authdes_cred
// xdr_authdes_verf
// xdr_authunix_parms
GO(xdr_bool, iFpp)
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
//GO(xdr_int, iFpp)
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
GO(xdr_netobj, iFpbup_)
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
GO(xdr_u_int, iFpp)
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
GO(xdr_void, iFv)
// xdr_wrapstring
// xencrypt
GO(__xmknod, iFipup)
//GO(__xmknodat, iFiipip)
//GO(__xpg_basename, pFp)
// __xpg_sigpause   // Weak
GO(__xpg_strerror_r, tFipu)
// xprt_register
// xprt_unregister
GOM(__xstat, iFEipp) //%%
GOM(__xstat64, iFEipp) //%%

// forcing a custom __gmon_start__ that does nothing
GOM(__gmon_start__, vFEv) //%%

GOM(_Jv_RegisterClasses, vFv)   //%%,noE dummy

GOM(__fdelt_chk, LFL) //%%,noE

GOM(getauxval, uFEu)  //%% implemented since glibc 2.16

//GOM(prlimit64, lFpupp)       //%%,noE
//GO(process_vm_readv, lFipLpLL)
//GO(process_vm_writev, lFipLpLL)
//GOM(reallocarray, pFpLL)     //%%,noE
//GOM(__open_nocancel, iFEpOV) //%%
//GO2(__read_nocancel, lFipL, read)
GO2(__close_nocancel, iFi, close)

//GOM(mkstemps64, iFEpi)   //%% not always implemented
//GOM(getentropy, iFEpL)   //%% starting from glibc 2.25

// not found (libitm???), but it seems OK to declare dummies:

GOM(_ITM_RU1, uFp)          //%%,noE
GOM(_ITM_RU4, uFp)          //%%,noE
GOM(_ITM_RU8, UFp)          //%%,noE
GOM(_ITM_memcpyRtWn, vFppu) //%%,noE register(2)
GOM(_ITM_memcpyRnWt, vFppu) //%%,noE register(2)
GOM(_ITM_addUserCommitAction, vFEpup)
GOM(_ITM_registerTMCloneTable, vFEpu)  //%%
GOM(_ITM_deregisterTMCloneTable, vFEp) //%%

GOM(__umoddi3, UFUU)        //%%,noE
GOM(__udivdi3, UFUU)        //%%,noE
GOM(__divdi3, IFII)         //%%,noE
//GOM(__poll_chk, iFpuii)     //%%,noE

GO(fallocate64, iFiill)

//DATAM(__libc_stack_end, sizeof(void*))

//DATAM(___brk_addr, 4)
//DATA(__libc_enable_secure, 4)

//GOM(__register_frame_info, vFpp)  //%%,noE faked function
//GOM(__deregister_frame_info, pFp) //%%,noE

//GO(name_to_handle_at, iFipppi) // only glibc 2.14+, so may not be present...

//GOM(modify_ldt, iFEipL) // there is suposedly no glibc wrapper for this one

#ifdef ANDROID
GOM(__libc_init, vFEpppp)
GO(__errno, pFv)
#else
// Those symbols don't exist in non-Android builds
//GOM(__libc_init,
//GO(__errno,
#endif

//GOM(stat,
//GOM(lstat,
//GOM(fstat,
//GO(setprogname,
//GO(getprogname,

DATAM(__libc_single_threaded, 4)	//B type
