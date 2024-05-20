#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA) && defined(GOS))
#error Meh...
#endif

//socklen_t is u32
// typedef unsigned long int nfds_t;
// pid_t is S32
// key_t is S32
// uid_t is u32
// gid_t is u32

GO(a64l, lFp)
GO(abort, vFv)
//DATAB(__abort_msg, 
GO(abs, iFi)
GO(accept, iFipp)
GO(accept4, iFippi)
GOW(access, iFpi)
GO(acct, iFp)
GOW(addmntent, iFpp)
GOW(addseverity, iFip)
GOW(adjtime, iFpp)
GO(__adjtimex, iFp)
GOW(adjtimex, iFp)
//GOW(advance, 
GO(alarm, uFu)
GO(aligned_alloc, pFUU)
GOW(alphasort, iFpp)
GOW(alphasort64, iFpp)
//GO(__arch_prctl, 
//GOW(arch_prctl, 
//DATA(argp_err_exit_status, 
//GOW(argp_error, vF!pV)
//GOW(argp_failure, vF!iipV)
//GOW(argp_help, vF!pup)
//GOW(argp_parse, iF!ipupp)
//DATAB(argp_program_bug_address, 
//DATAB(argp_program_version, 
//DATAB(argp_program_version_hook, 
//GOW(argp_state_help, vF!pu)
//GOW(argp_usage, vF!)
GOW(argz_add, iFppp)
GOW(argz_add_sep, iFpppi)
GOW(argz_append, iFpppL)
GO(__argz_count, LFpL)
GOW(argz_count, LFpL)
GOW(argz_create, iFppp)
GOW(argz_create_sep, iFpipp)
GO(argz_delete, vFppp)
GOW(argz_extract, vFpLp)
GOW(argz_insert, iFpppp)
GO(__argz_next, pFpLp)
GOW(argz_next, pFpLp)
GOW(argz_replace, iFppppp)
GO(__argz_stringify, vFpLi)
GOW(argz_stringify, vFpLi)
GO(asctime, pFp)
GOW(asctime_r, pFpp)
GOM(__asprintf, iFEppV)
GOWM(asprintf, iFEppV)
GOM(__asprintf_chk, iFEpipV)
GO(__assert, vFppi)
GO(__assert_fail, vFppup)
GO(__assert_perror_fail, vFipup)
GOM(atexit, iFEp)
GO(atof, dFp)
GO(atoi, iFp)
GO(atol, lFp)
GO(atoll, IFp)
//GO(authdes_create, !Fpup!)
//GO(authdes_getucred, iF!pppp)
//GO(authdes_pk_create, !Fp!up!)
//GO(_authenticate, uF!!)
//GO(authnone_create, !Fv)
//GO(authunix_create, !Fpuuip)
//GO(authunix_create_default, !Fv)
//GO(__backtrace, 
GOWM(backtrace, iFEpi)
//GO(__backtrace_symbols, 
GOWM(backtrace_symbols, pFEpi)
//GO(__backtrace_symbols_fd, 
GOWM(backtrace_symbols_fd, vFEpii)
GOW(basename, pFp)
GO(bcmp, iFppL)
GO(bcopy, vFppL)
//GO(bdflush, // Deprecated
GOW(bind, iFipu)
//GO(bindresvport, iFi!)
GOW(bindtextdomain, pFpp)
GOW(bind_textdomain_codeset, pFpp)
GOW(brk, iFp)
//GO(__bsd_getpgrp, 
GO2(bsd_signal, pFEip, my_signal)
GOM(bsearch, pFEppLLp)
GOW(btowc, uFi)
GO(__bzero, vFpL)
GO(bzero, vFpL)
//GO(c16rtomb, LFpW!)
GOW(c32rtomb, LFpup)
GOW(calloc, pFLL)
//GO(callrpc, iFpLLL@p@p)
//GO(__call_tls_dtors, vFv)
GOW(canonicalize_file_name, pFp)
GO(capget, iFpp)
GO(capset, iFpp)
GO(catclose, iFp)
GO(catgets, pFpiip)
GO(catopen, pFpi)
//GO(cbc_crypt, // Deprecated
GO(cfgetispeed, uFp)
GO(cfgetospeed, uFp)
GO(cfmakeraw, vFp)
GO(cfree, vFp)
GO(cfsetispeed, iFpu)
GO(cfsetospeed, iFpu)
GO(cfsetspeed, iFpu)
GOW(chdir, iFp)
DATA(__check_rhosts_file, sizeof(void*))
//GO(chflags, 
//GO(__chk_fail, 
GOW(chmod, iFpu)
GOW(chown, iFpuu)
GO(chroot, iFp)
GOW(clearenv, iFv)
GO(clearerr, vFp)
GO(clearerr_unlocked, vFp)
#ifdef STATICBUILD
//GO(clnt_broadcast, 
//GO(clnt_create, !FpLLp)
//GO(clnt_pcreateerror, vFp)
//GO(clnt_perrno, vFu)
//GO(clnt_perror, vF!p)
//GO(clntraw_create, !FLL)
//GO(clnt_spcreateerror, pFp)
//GO(clnt_sperrno, pFu)
//GO(clnt_sperror, pF!p)
//GO(clnttcp_create, !F!LLpuu)
//GO(clntudp_bufcreate, !F!LL?puu)
//GO(clntudp_create, !F!LL?p)
//GO(clntunix_create, !F!LLpuu)
#else
//GO(clnt_broadcast, 
//GO(clnt_create, !FpLLp)
GO(clnt_pcreateerror, vFp)
GO(clnt_perrno, vFu)
//GO(clnt_perror, vF!p)
//GO(clntraw_create, !FLL)
GO(clnt_spcreateerror, pFp)
GO(clnt_sperrno, pFu)
//GO(clnt_sperror, pF!p)
//GO(clnttcp_create, !F!LLpuu)
//GO(clntudp_bufcreate, !F!LL?puu)
//GO(clntudp_create, !F!LL?p)
//GO(clntunix_create, !F!LLpuu)
#endif
GO(clock, lFv)
GO(clock_adjtime, iFip)
//GO(__clock_getcpuclockid, 
GOW(clock_getcpuclockid, iFip)
//GO(__clock_getres, 
GOW(clock_getres, iFip)
//GO(__clock_gettime, 
GOW(clock_gettime, iFip)
//GO(__clock_nanosleep, 
GOW(clock_nanosleep, iFiipp)
//GO(__clock_settime, 
GOW(clock_settime, iFip)
//GO(__clone, 
GOWM(clone, iFEppipppp)
GO(__close, iFi)
GOW(close, iFi)
GOW(closedir, iFp)
GO(closelog, vFv)
//GO(__close_nocancel, 
GO(close_range, iFuuu)
GO(__cmsg_nxthdr, pFpp)
GO(confstr, LFipL)
//GO(__confstr_chk, 
GOW(__connect, iFipu)
GOW(connect, iFipu)
GO(copy_file_range, lFipipLu)
//GO(__copy_grp, 
GOW(copysign, dFdd)
GOW(copysignf, fFff)
GOW(copysignl, DFDD)
GOW(creat, iFpu)
GOW(creat64, iFpu)
//GO(create_module, 
GO(ctermid, pFp)
GO(ctime, pFp)
GO(ctime_r, pFpp)
//DATA(__ctype32_b, 
//DATA(__ctype32_tolower, 
//DATA(__ctype32_toupper, 
#ifdef STATICBUILD
//DATA(__ctype_b, sizeof(void*))
#else
DATA(__ctype_b, sizeof(void*))
#endif
GO(__ctype_b_loc, pFv)
GO(__ctype_get_mb_cur_max, LFv)
//GO(__ctype_init, 
DATAM(__ctype_tolower, sizeof(void*))
GO(__ctype_tolower_loc, pFv)
DATAM(__ctype_toupper, sizeof(void*))
GO(__ctype_toupper_loc, pFv)
//DATAB(__curbrk, 
GO(cuserid, pFp)
GOM(__cxa_atexit, iFEppp)
//GO(__cxa_at_quick_exit, /* at_quick_exit has signature iF@ -> */ iF@pp)
GOM(__cxa_finalize, vFEp)
GOM(__cxa_thread_atexit_impl, iFEppp)
//GO(__cyg_profile_func_enter, 
//GO(__cyg_profile_func_exit, 
GO(daemon, iFii)
DATAB(__daylight, 4)
DATA(daylight, 4)
GO(__dcgettext, pFppi)
GOW(dcgettext, pFppi)
GOW(dcngettext, pFpppLi)
//GO(__default_morecore, 
//GO(delete_module, // Deprecated
//GO(des_setparity, // Deprecated
GO(__dgettext, pFpp)
GOW(dgettext, pFpp)
GO(difftime, dFll)
GO(dirfd, iFp)
GO(dirname, pFp)
GO(div, LFii)
//GO(_dl_addr, 
//GO(_dl_catch_error, 
//GO(_dl_catch_exception, 
GOWM(dl_iterate_phdr, iFEpp)
//GO(_dl_mcount_wrapper, 
//GO(_dl_mcount_wrapper_check, 
//DATAB(_dl_open_hook, 
//DATAB(_dl_open_hook2, 
//GO(_dl_signal_error, 
//GO(_dl_signal_exception, 
//GO(_dl_sym, 
//GO(_dl_vsym, 
GOW(dngettext, pFpppL)
GOM(dprintf, iFEipV)
GOM(__dprintf_chk, iFEiipV)
GO(drand48, dFv)
GO(drand48_r, iFpp)
GOW(dup, iFi)
GO(__dup2, iFii)
GOW(dup2, iFii)
GOW(dup3, iFiiO)
GO(__duplocale, pFp)
GOW(duplocale, pFp)
GO(dysize, iFi)
GOW(eaccess, iFpi)
//GO(ecb_crypt, 
GO(ecvt, pFdipp)
GOW(ecvt_r, iFdipppL)
GO(endaliasent, vFv)
GO(endfsent, vFv)
GO(endgrent, vFv)
GO(endhostent, vFv)
GO(__endmntent, iFp)
GOW(endmntent, iFp)
GO(endnetent, vFv)
GO(endnetgrent, vFv)
GO(endprotoent, vFv)
GO(endpwent, vFv)
GO(endrpcent, vFv)
GO(endservent, vFv)
//GO(endsgent, 
GO(endspent, vFv)
GOW(endttyent, iFv)
GO(endusershell, vFv)
GOW(endutent, vFv)
GO(endutxent, vFv)
DATAM(environ, sizeof(void*))
DATAM(__environ, sizeof(void*))
GO(envz_add, iFpppp)
GO(envz_entry, pFpLp)
GO(envz_get, pFpLp)
GO(envz_merge, iFpppLi)
GO(envz_remove, vFppp)
GO(envz_strip, vFpp)
GO(epoll_create, iFi)
GO(epoll_create1, iFO)
#ifdef NOALIGN
GO(epoll_ctl, iFiiip)
GO(epoll_pwait, iFipiip)
GO(epoll_wait, iFipii)
#else
GOM(epoll_ctl, iFEiiip)   // struct epoll_event is 12byte on x86_64 and 16bytes (8bytes aligned) on arm64
GOM(epoll_pwait, iFEipiip)
GOM(epoll_wait, iFEipii)
#endif
GO(erand48, dFp)
GOW(erand48_r, iFppp)
GOM(err, vFEipV)
//DATAB(errno, 
GO(__errno_location, pFv)
//GOW(error, vFiipV)
//GOW(error_at_line, vFiipupV)
//DATAB(error_message_count, 
//DATAB(error_one_per_line, 
//DATAB(error_print_progname, 
GOM(errx, vFEipV)
//GO(ether_aton, !Fp)
//GO(ether_aton_r, !Fp!)
//GO(ether_hostton, iFp!)
//GO(ether_line, iFp!p)
//GO(ether_ntoa, pF!)
//GO(ether_ntoa_r, pF!p)
//GO(ether_ntohost, iFp!)
GOW(euidaccess, iFpi)
GO(eventfd, iFui)
GO(eventfd_read, iFip)
GO(eventfd_write, iFiL)
GOM(execl, iFEpV) // First argument is also part of the variadic
GOM(execle, iFEpV) // First argument is also part of the variadic
GOM(execlp, iFEpV) // First argument is also part of the variadic
GOM(execv, iFEpp)
GOM(execve, iFEppp)
GOM(execvp, iFEpp)
GOW(execvpe, iFppp)
GO(_exit, vFi)  // immediate exit
GOM(exit, vFEi)
GOW(_Exit, vFi)
GO(explicit_bzero, vFpL)
GO(__explicit_bzero_chk, vFpLL)
GO(faccessat, iFipii)
GOW(fallocate, iFiill)
GO(fallocate64, iFiill)
GO(fanotify_init, iFuu)
GO(fanotify_mark, iFiuLip)
//GO(fattach, 
GO(__fbufsize, LFp)
GOW(fchdir, iFi)
//GO(fchflags, 
GOW(fchmod, iFiu)
GO(fchmodat, iFipui)
GOW(fchown, iFiuu)
GO(fchownat, iFipuui)
GO(fclose, iFp)
GOW(fcloseall, iFv)
GOWM(__fcntl, iFEiiN)
GOWM(fcntl, iFEiiN)
GOWM(fcntl64, iFEiiN)
GO(fcvt, pFdipp)
GO(fcvt_r, iFdipppL)
GO(fdatasync, iFi)
GO(__fdelt_chk, LFL)
//GO(__fdelt_warn, 
//GO(fdetach, 
GO(fdopen, pFip)
GOW(fdopendir, pFi)
//GO(__fentry__, 
GOW(feof, iFp)
GOW(feof_unlocked, iFp)
GOW(ferror, iFp)
GOW(ferror_unlocked, iFp)
GO(fexecve, iFipp)
GOW(fflush, iFS)
GO(fflush_unlocked, iFS)
//GO(__ffs, 
GO(ffs, iFi)
GO(ffsl, iFI)
GO(ffsll, iFI)
GOW(fgetc, iFp)
GOW(fgetc_unlocked, iFp)
GO(fgetgrent, pFp)
GOW(fgetgrent_r, iFpppLp)
GO(fgetpos, iFpp)
GO(fgetpos64, iFpp)
GO(fgetpwent, pFp)
GOW(fgetpwent_r, iFpppLp)
GOW(fgets, pFpip)
GO(__fgets_chk, pFpLip)
//GO(fgetsgent, 
//GOW(fgetsgent_r, 
GO(fgetspent, pFp)
GOW(fgetspent_r, iFpppLp)
GOW(fgets_unlocked, pFpip)
//GO(__fgets_unlocked_chk, 
GOW(fgetwc, uFp)
GOW(fgetwc_unlocked, uFp)
GO(fgetws, pFpip)
//GO(__fgetws_chk, 
GO(fgetws_unlocked, pFpip)
//GO(__fgetws_unlocked_chk, 
GO(fgetxattr, lFippL)
GOW(fileno, iFS)
GOW(fileno_unlocked, iFS)
GO(__finite, iFd)
GOW(finite, iFd)
GO(__finitef, iFf)
GOW(finitef, iFf)
GO(__finitel, iFD)
GOW(finitel, iFD)
GO(__flbf, iFp)
GO(flistxattr, lFipL)
GOW(flock, iFii)
GOW(flockfile, vFp)
GOW(_flushlbf, vFv)
GO(fmemopen, pFpLp)
GO(fmtmsg, iFlpippp)
GO(fnmatch, iFppi)
GOM(fopen, pFEpp)
GOWM(fopen64, pFEpp)
GOM(fopencookie, pFEppV)
//GO(__fork, 
GOWM(fork, iFEv)
//GO(__fortify_fail, 
GO(fpathconf, lFii)
GO(__fpending, LFp)
GOM(fprintf, iFEppV)
GOM(__fprintf_chk, iFEpipV)
//DATA(__fpu_control, 
GO(__fpurge, vFp)
GO(fputc, iFip)
GO(fputc_unlocked, iFip)
GOW(fputs, iFpp)
GOW(fputs_unlocked, iFpp)
GO(fputwc, uFip)
GO(fputwc_unlocked, uFip)
GO(fputws, iFpp)
GO(fputws_unlocked, iFpp)
GO(fread, LFpLLp)
GO(__freadable, iFp)
GO(__fread_chk, LFpLLLp)
GO(__freading, iFp)
GO(fread_unlocked, LFpLLp)
GO(__fread_unlocked_chk, LFpLLLp)
GO(free, vFp)
GO(freeaddrinfo, vFp)
GOW(freeifaddrs, vFp)
GO(__freelocale, vFp)
GOW(freelocale, vFp)
GO(fremovexattr, iFip)
GO(freopen, pFppp)
GO(freopen64, pFppp)
GOW(frexp, dFdp)
GOW(frexpf, fFfp)
GOW(frexpl, DFDp)
GOM(fscanf, iFEppV)
GO(fseek, iFpli)
GO(fseeko, iFpIi)
GO(__fseeko64, iFpli)
GO(fseeko64, iFpli)
GO(__fsetlocking, iFpi)
GO(fsetpos, iFpp)
GO(fsetpos64, iFpp)
GO(fsetxattr, iFippLi)
GOM(fstat, iFEip)
GOM(fstat64, iFEip)
GOM(fstatat, iFEippi)
GOM(fstatat64, iFEippi)
GOW(fstatfs, iFip)
GOW(fstatfs64, iFip)
GOW(fstatvfs, iFip)
GOW(fstatvfs64, iFip)
GO(fsync, iFi)
GO(ftell, lFp)
GO(ftello, IFp)
GO(__ftello64, lFp)
GO(ftello64, lFp)
GO(ftime, iFp)
GO(ftok, iFpi)
GO(ftruncate, iFiI)
GO(ftruncate64, iFil)
GOW(ftrylockfile, iFp)
GOW(fts64_children, pFpi)
GOW(fts64_close, iFp)
GOWM(fts64_open, pFEpip)
GOW(fts64_read, pFp)
GOW(fts64_set, iFppi)
GO(fts_children, pFpi)
GO(fts_close, iFp)
GOM(fts_open, pFEpip)
GO(fts_read, pFp)
GO(fts_set, iFppi)
GOM(ftw, iFEppi)
GOM(ftw64, iFEppi)
GOW(funlockfile, vFp)
GO(futimens, iFip)
GOW(futimes, iFip)
GO(futimesat, iFipp)
GO(fwide, iFpi)
GOWM(fwprintf, iFEppV)
GOM(__fwprintf_chk, iFEpipV)
GO(__fwritable, iFp)
GO(fwrite, LFpLLp)
GO(fwrite_unlocked, LFpLLp)
GO(__fwriting, iFp)
//GO(fwscanf, iFppV)
GOM(__fxstat, iFEiip)
GOM(__fxstat64, iFEiip)
GOM(__fxstatat, iFEiippi)
GOM(__fxstatat64, iFEiippi)
//GO(__gai_sigqueue, 
GO(gai_cancel, iFp)
GO(gai_error, iFp)
GO(gai_strerror, pFi)
GO(gai_suspend, iFpip)
//GO(__gconv_get_alias_db, 
//GO(__gconv_get_cache, 
//GO(__gconv_get_modules_db, 
//GO(__gconv_transliterate, 
GO(gcvt, pFdip)
GO(getaddrinfo, iFpppp)
GO(getaddrinfo_a, iFipip)
GO(getaliasbyname, pFp)
GO(getaliasbyname_r, iFpppLp)
GO(getaliasent, pFv)
GO(getaliasent_r, iFppLp)
//GO(__getauxval, 
GOWM(getauxval, LFEL)
GOW(get_avphys_pages, lFv)
GOW(getc, iFp)
GO(getchar, iFv)
GO(getchar_unlocked, iFv)
GOWM(getcontext, iFEp)
GOW(getcpu, iFpp)
GOW(getc_unlocked, iFp)
GO(get_current_dir_name, pFv)
GO(getcwd, pFpL)
GO(__getcwd_chk, pFpLL)
GO(getdate, pFp)
//DATAB(getdate_err, 
GOW(getdate_r, iFpp)
GOW(__getdelim, lFppip)
GO(getdelim, lFppip)
GO(getdents64, lFipL)
GO(getdirentries, IFipUp)
GO(getdirentries64, lFipLp)
GO(getdomainname, iFpL)
//GO(__getdomainname_chk, 
GOW(getdtablesize, iFv)
GOW(getegid, uFv)
GO(getentropy, iFpL)
GO(getenv, pFp)
GOW(geteuid, uFv)
GO(getfsent, pFv)
GO(getfsfile, pFp)
GO(getfsspec, pFp)
GOW(getgid, uFv)
GO(getgrent, pFv)
GO(getgrent_r, iFppLp)
GO(getgrgid, pFu)
GO(getgrgid_r, iFuppLp)
GO(getgrnam, pFp)
GO(getgrnam_r, iFpppLp)
GO(getgrouplist, iFpupp)
GOW(getgroups, iFip)
GO(__getgroups_chk, iFipL)
GO(gethostbyaddr, pFpui)
GO(gethostbyaddr_r, iFpuippLpp)
GO(gethostbyname, pFp)
GO(gethostbyname2, pFpi)
GO(gethostbyname2_r, iFpippLpp)
GO(gethostbyname_r, iFpppLpp)
GO(gethostent, pFv)
GO(gethostent_r, iFppLpp)
GO(gethostid, lFv)
GOW(gethostname, iFpL)
//GO(__gethostname_chk, 
GOW(getifaddrs, iFp)
//GO(getipv4sourcefilter, iFi??pp!)
GOW(getitimer, iFup)
//GO(get_kernel_syms, // Deprecated
GO(getline, lFppp)
GO(getloadavg, iFpi)
GO(getlogin, pFv)
GOW(getlogin_r, iFpL)
//GO(__getlogin_r_chk, 
GO(getmntent, pFp)
GO(__getmntent_r, pFpppi)
GOW(getmntent_r, pFpppi)
//GO(getmsg, // Deprecated
//GO(get_myaddress, vF!)
GO(getnameinfo, iFpupupui)
//GO(getnetbyaddr, !Fui)
//GO(getnetbyaddr_r, iFui!pL!p)
//GO(getnetbyname, !Fp)
//GO(getnetbyname_r, iFp!pL!p)
//GO(getnetent, !Fv)
//GO(getnetent_r, iF!pL!p)
GO(getnetgrent, iFppp)
GOW(getnetgrent_r, iFppppL)
#ifdef STATICBUILD
//GO(getnetname, iFp)
#else
GO(getnetname, iFp)
#endif
GOW(get_nprocs, iFv)
GOW(get_nprocs_conf, iFv)
GOM(getopt, iFipp)
GOM(getopt_long, iFipppp)
GOM(getopt_long_only, iFipppp)
GO(__getpagesize, iFv)
GOW(getpagesize, iFv)
GO(getpass, pFp)
GOW(getpeername, iFipp)
GO(__getpgid, iFi)
GOW(getpgid, iFi)
GO(getpgrp, iFv)
GOW(get_phys_pages, lFv)
GO(__getpid, iFv)
GO(getpid, iFv)
//GO(getpmsg, // Deprecated
GOW(getppid, iFv)
GOW(getpriority, iFuu)
GO(getprotobyname, pFp)
GO(getprotobyname_r, iFpppLp)
GO(getprotobynumber, pFi)
GO(getprotobynumber_r, iFippLp)
GO(getprotoent, pFv)
GO(getprotoent_r, iFppLp)
GOW(getpt, iFv)
#ifdef STATICBUILD
//GO(getpublickey, iFpp)
#else
GO(getpublickey, iFpp)
#endif
GOW(getpw, iFup)
GO(getpwent, pFv)
GO(getpwent_r, iFppLp)
GO(getpwnam, pFp)
GO(getpwnam_r, iFpppLp)
GO(getpwuid, pFu)
GO(getpwuid_r, iFuppLp)
GO(getrandom, lFpLu)
GO(getresgid, iFppp)
GO(getresuid, iFppp)
GO(__getrlimit, iFip)
GOW(getrlimit, iFup)
GOW(getrlimit64, iFup)
GO(getrpcbyname, pFp)
GO(getrpcbyname_r, iFpppLp)
GO(getrpcbynumber, pFi)
GO(getrpcbynumber_r, iFippLp)
GO(getrpcent, pFv)
GO(getrpcent_r, iFppLp)
#ifdef STATICBUILD
//GO(getrpcport, 
#else
GO(getrpcport, iFpLLu)
#endif
GOW(getrusage, iFip)
//GOW(gets, // Deprecated
//GO(__gets_chk, 
#ifdef STATICBUILD
//GO(getsecretkey, 
#else
GO(getsecretkey, iFppp)
#endif
GO(getservbyname, pFpp)
GO(getservbyname_r, iFppppLp)
GO(getservbyport, pFip)
GO(getservbyport_r, iFipppLp)
GO(getservent, pFv)
//GO(getservent_r, iF!pL!)
//GO(getsgent, 
//GO(getsgent_r, 
//GO(getsgnam, 
//GO(getsgnam_r, 
GO(getsid, iFi)
GOW(getsockname, iFipp)
GOW(getsockopt, iFiiipp)
//GO(getsourcefilter, iFiupupp!)
GO(getspent, pFv)
GO(getspent_r, iFppLp)
GO(getspnam, pFp)
GO(getspnam_r, iFpppLp)
GO(getsubopt, iFppp)
GOW(gettext, pFp)
GOW(gettid, iFv)
GO(__gettimeofday, iFpp)
GO(gettimeofday, iFpp)
GOW(getttyent, pFv)
GOW(getttynam, pFp)
GOW(getuid, uFv)
GO(getusershell, pFv)
GOW(getutent, pFv)
GOW(getutent_r, iFpp)
GOW(getutid, pFp)
GOW(getutid_r, iFppp)
GOW(getutline, pFp)
GOW(getutline_r, iFppp)
GO(getutmp, vFpp)
GO(getutmpx, vFpp)
GO(getutxent, pFv)
GO(getutxid, pFp)
GO(getutxline, pFp)
GO(getw, iFp)
GOW(getwc, uFp)
GO(getwchar, uFv)
GO(getwchar_unlocked, uFv)
GOW(getwc_unlocked, uFp)
GO(getwd, pFp)
//GO(__getwd_chk, 
GO(getxattr, lFpppL)
GOM(glob, iFEpipp)
GOM(glob64, iFEpipp)
GO(globfree, vFp)
GOW(globfree64, vFp)
GOW(glob_pattern_p, iFpi)
GO(gmtime, pFp)
GO(__gmtime_r, pFpp)
GOW(gmtime_r, pFpp)
GOW(gnu_dev_major, uFL)
GOW(gnu_dev_makedev, LFuu)
GOW(gnu_dev_minor, uFL)
GOW(gnu_get_libc_release, pFv)
GOW(gnu_get_libc_version, pFv)
GO(grantpt, iFi)
GOW(group_member, iFu)
GOW(gsignal, iFi)
//GO(gtty, // Deprecated
GOW(hasmntopt, pFpp)
GO(hcreate, iFU)
//GOW(hcreate_r, iFL!)
GOW(hdestroy, vFv)
//GOW(hdestroy_r, vF!)
//DATA(h_errlist, 
//DATAB(__h_errno, 
GO(__h_errno_location, pFv)
GO(herror, vFp)
#ifdef STATICBUILD
//GO(host2netname, 
#else
GO(host2netname, iFppp)
#endif
//GO(hsearch, pF?u)
//GOW(hsearch_r, iF?up!)
GO(hstrerror, pFi)
GO(htonl, uFu)
GO(htons, WFW)
GO(iconv, LFppppp)
GO(iconv_close, iFp)
GO(iconv_open, pFpp)
//GO(__idna_from_dns_encoding, 
//GO(__idna_to_dns_encoding, 
GOW(if_freenameindex, vFp)
GOW(if_indextoname, pFup)
GOW(if_nameindex, pFv)
GOW(if_nametoindex, uFp)
GOW(imaxabs, lFl)
GOW(imaxdiv, IFII)
DATA(in6addr_any, 16)  // type V
DATA(in6addr_loopback, 16)  //type V
GO(index, pFpi)
GO(inet6_opt_append, iFpuiCuCp)
GO(inet6_opt_find, iFpuiCpp)
GO(inet6_opt_finish, iFpui)
GO(inet6_opt_get_val, iFpipu)
GO(inet6_opt_init, iFpu)
GO(inet6_option_alloc, pFpiii)
GO(inet6_option_append, iFppii)
GO(inet6_option_find, iFppi)
GO(inet6_option_init, iFppi)
GO(inet6_option_next, iFpp)
GO(inet6_option_space, iFi)
GO(inet6_opt_next, iFpuippp)
GO(inet6_opt_set_val, iFpipu)
//GO(inet6_rth_add, iFp!)
//GO(inet6_rth_getaddr, !Fpi)
GO(inet6_rth_init, pFpuii)
GO(inet6_rth_reverse, iFpp)
GO(inet6_rth_segments, iFp)
GO(inet6_rth_space, uFii)
//GO(__inet6_scopeid_pton, 
GOW(inet_addr, uFp)
GOW(inet_aton, iFpp)
//GO(__inet_aton_exact, 
//GO(inet_lnaof, uF?)
//GOW(inet_makeaddr, ?Fuu)
//GO(inet_netof, uF?)
GO(inet_network, uFp)
GO(inet_nsap_addr, uFppi)
GO(inet_nsap_ntoa, pFipp)
GO(inet_ntoa, pFu) // Really?
GO(inet_ntop, pFippu)
GOW(inet_pton, iFipp)
//GO(__inet_pton_length, 
GO(initgroups, iFpu)
//GO(init_module, // Deprecated
GO(initstate, pFupL)
GO(initstate_r, iFupLp)
GO(innetgr, iFpppp)
GO(inotify_add_watch, iFipu)
GO(inotify_init, iFv)
GO(inotify_init1, iFO)
GO(inotify_rm_watch, iFii)
GO(insque, vFpp)
//GO(__internal_endnetgrent, 
//GO(__internal_getnetgrent_r, 
//GO(__internal_setnetgrent, 
DATA(_IO_2_1_stderr_, 224)
DATA(_IO_2_1_stdin_, 224)
DATA(_IO_2_1_stdout_, 224)
//GO(_IO_adjust_column, 
//GO(_IO_adjust_wcolumn, 
GOW(ioctl, iFiLN)
GO(_IO_default_doallocate, iFS)
GO(_IO_default_finish, vFSi)
GO(_IO_default_pbackfail, iFSi)
GO(_IO_default_uflow, iFS)
GO(_IO_default_xsgetn, LFSpL)
GO(_IO_default_xsputn, LFSpL)
GO(_IO_doallocbuf, vFS)
GO(_IO_do_write, iFSpL)
//GO(_IO_enable_locks, 
//GO(_IO_fclose, 
//GO(_IO_fdopen, 
//GO(_IO_feof, 
//GO(_IO_ferror, 
//GO(_IO_fflush, 
//GO(_IO_fgetpos, 
//GO(_IO_fgetpos64, 
//GO(_IO_fgets, 
GO(_IO_file_attach, pFSi)
GO(_IO_file_close, iFS)
GO(_IO_file_close_it, iFS)
GO(_IO_file_doallocate, iFS)
//GO(_IO_file_finish, 
GO(_IO_file_fopen, pFSppi)
GO(_IO_file_init, vFS)
DATA(_IO_file_jumps, sizeof(void*))
GO(_IO_file_open, pFSpiiii)
GO(_IO_file_overflow, iFSi)
GO(_IO_file_read, lFSpl)
GO(_IO_file_seek, IFSIi)
GO(_IO_file_seekoff, IFSIii)
GO(_IO_file_setbuf, pFSpl)
GOM(_IO_file_stat, iFESp)
GO(_IO_file_sync, iFS)
GO(_IO_file_underflow, iFS)
GO(_IO_file_write, lFSpl)
GO(_IO_file_xsputn, LFSpL)
GO(_IO_flockfile, vFS)
GO(_IO_flush_all, iFv)
GO(_IO_flush_all_linebuffered, vFv)
//GO(_IO_fopen, 
//GOW(_IO_fprintf, 
//GO(_IO_fputs, 
//GO(_IO_fread, 
GO(_IO_free_backup_area, vFS)
//GO(_IO_free_wbackup_area, 
//GO(_IO_fsetpos, 
//GO(_IO_fsetpos64, 
//GO(_IO_ftell, 
//GO(_IO_ftrylockfile, 
GO(_IO_funlockfile, vFS)
//GO(_IO_fwrite, 
GO(_IO_getc, iFS)
//GO(_IO_getline, 
GO(_IO_getline_info, LFSpLiip)
//GO(_IO_gets, 
GO(_IO_init, vFSi)
GO(_IO_init_marker, vFpS)
//GO(_IO_init_wmarker, 
//GO(_IO_iter_begin, 
//GO(_IO_iter_end, 
//GO(_IO_iter_file, 
//GO(_IO_iter_next, 
//GO(_IO_least_wmarker, 
GO(_IO_link_in, vFp)
DATA(_IO_list_all, sizeof(void*))
//GO(_IO_list_lock, 
//GO(_IO_list_resetlock, 
//GO(_IO_list_unlock, 
//GO(_IO_marker_delta, 
//GO(_IO_marker_difference, 
//GO(_IO_padn, 
//GO(_IO_peekc_locked, 
#ifdef STATICBUILD
//GO(ioperm, 
#else
GO(ioperm, iFLLi)
#endif
GOM(iopl, iFEi)     //Not always present
//GO(_IO_popen, 
//GO(_IO_printf, 
//GO(_IO_proc_close, 
//GO(_IO_proc_open, 
GO(_IO_putc, iFiS)
//GO(_IO_puts, 
//GO(_IO_remove_marker, 
//GO(_IO_seekmark, 
//GO(_IO_seekoff, 
//GO(_IO_seekpos, 
//GO(_IO_seekwmark, 
//GO(_IO_setb, 
//GO(_IO_setbuffer, 
//GO(_IO_setvbuf, 
//GO(_IO_sgetn, 
//GO(_IO_sprintf, 
//GO(_IO_sputbackc, 
//GO(_IO_sputbackwc, 
//GO(_IO_sscanf, 
//GO(_IO_str_init_readonly, 
//GO(_IO_str_init_static, 
//GO(_IO_str_overflow, 
//GO(_IO_str_pbackfail, 
//GO(_IO_str_seekoff, 
//GO(_IO_str_underflow, 
//GO(_IO_sungetc, 
//GO(_IO_sungetwc, 
//GO(_IO_switch_to_get_mode, 
//GO(_IO_switch_to_main_wget_area, 
//GO(_IO_switch_to_wbackup_area, 
//GO(_IO_switch_to_wget_mode, 
//GO(_IO_ungetc, 
//GO(_IO_un_link, 
//GO(_IO_unsave_markers, 
//GO(_IO_unsave_wmarkers, 
//GO(_IO_vfprintf, 
//GO(_IO_vfscanf, 
//GO(_IO_vsprintf, 
//GO(_IO_wdefault_doallocate, 
//GO(_IO_wdefault_finish, 
//GO(_IO_wdefault_pbackfail, 
//GO(_IO_wdefault_uflow, 
//GO(_IO_wdefault_xsgetn, 
//GO(_IO_wdefault_xsputn, 
//GO(_IO_wdoallocbuf, 
//GO(_IO_wdo_write, 
//DATA(_IO_wfile_jumps, 
//GO(_IO_wfile_overflow, 
//GO(_IO_wfile_seekoff, 
//GO(_IO_wfile_sync, 
//GO(_IO_wfile_underflow, 
//GO(_IO_wfile_xsputn, 
//GO(_IO_wmarker_delta, 
//GO(_IO_wsetb, 
GO(iruserok, iFuipp)
GO(iruserok_af, iFpippW)
GO(isalnum, iFi)
//GO(__isalnum_l, 
//GOW(isalnum_l, iFi!)
GO(isalpha, iFi)
//GO(__isalpha_l, 
GOW(isalpha_l, iFip)
GO(isascii, iFi)
//GOW(__isascii_l, 
//GO(isastream, // Deprecated
GOW(isatty, iFi)
GO(isblank, iFi)
//GO(__isblank_l, 
//GOW(isblank_l, iFi!)
GO(iscntrl, iFi)
//GO(__iscntrl_l, 
//GOW(iscntrl_l, iFi!)
//GO(__isctype, 
GOW(isctype, iFii)
GO(isdigit, iFi)
//GO(__isdigit_l, 
//GOW(isdigit_l, iFi!)
GO(isfdtype, iFii)
GO(isgraph, iFi)
//GO(__isgraph_l, 
//GOW(isgraph_l, iFi!)
GO(__isinf, iFd)
GOW(isinf, iFd)
GO(__isinff, iFf)
GOW(isinff, iFf)
GO(__isinfl, iFD)
GOW(isinfl, iFD)
GO(islower, iFi)
//GO(__islower_l, 
//GOW(islower_l, iFi!)
GO(__isnan, iFd)
GOW(isnan, iFd)
GO(__isnanf, iFf)
GOW(isnanf, iFf)
GO(__isnanl, iFD)
GOW(isnanl, iFD)
GO2(__isoc23_fscanf, iFEppV, my___isoc99_fscanf)
GO2(__isoc23_sscanf, iFEppV, my___isoc99_sscanf)
GO2(__isoc23_strtol, lFppi, strtol)
GO2(__isoc23_strtoll, IFppi, strtoll)
GO2(__isoc23_strtoul, LFppi, strtoul)
GO2(__isoc23_strtoull, UFppi, strtoull)
GOM(__isoc99_fscanf, iFEppV)
//GO(__isoc99_fwscanf, iFppV)
GOM(__isoc99_scanf, iFEpV)
GOM(__isoc99_sscanf, iFEppV)
GOM(__isoc99_swscanf, iFEppV)
GOM(__isoc99_vfscanf, iFEppA)
//GO(__isoc99_vfwscanf, iFppA)
GOM(__isoc99_vscanf, iFEpA)
GOM(__isoc99_vsscanf, iFEppA)
GOM(__isoc99_vswscanf, iFEppA)
//GO(__isoc99_vwscanf, iFpA)
//GO(__isoc99_wscanf, iFpV)
GO(isprint, iFi)
//GO(__isprint_l, 
//GOW(isprint_l, iFi!)
GO(ispunct, iFi)
//GO(__ispunct_l, 
//GOW(ispunct_l, iFi!)
GO(isspace, iFi)
//GO(__isspace_l, 
//GOW(isspace_l, iFi!)
GO(isupper, iFi)
//GO(__isupper_l, 
//GOW(isupper_l, iFi!)
GOW(iswalnum, iFu)
//GO(__iswalnum_l, 
GOW(iswalnum_l, iFup)
GOW(iswalpha, iFu)
//GO(__iswalpha_l, 
GOW(iswalpha_l, iFup)
GOW(iswblank, iFu)
//GO(__iswblank_l, 
GOW(iswblank_l, iFup)
GOW(iswcntrl, iFu)
//GO(__iswcntrl_l, 
GOW(iswcntrl_l, iFup)
//GO(__iswctype, 
GOW(iswctype, iFuL)
GO(__iswctype_l, iFuLL)
GOW(iswctype_l, iFuLp)
GOW(iswdigit, iFu)
//GO(__iswdigit_l, 
GOW(iswdigit_l, iFup)
GOW(iswgraph, iFu)
//GO(__iswgraph_l, 
GOW(iswgraph_l, iFup)
GOW(iswlower, iFu)
//GO(__iswlower_l, 
GOW(iswlower_l, iFup)
GOW(iswprint, iFu)
//GO(__iswprint_l, 
GOW(iswprint_l, iFup)
GOW(iswpunct, iFu)
//GO(__iswpunct_l, 
GOW(iswpunct_l, iFup)
GOW(iswspace, iFu)
//GO(__iswspace_l, 
GOW(iswspace_l, iFup)
GOW(iswupper, iFu)
//GO(__iswupper_l, 
GOW(iswupper_l, iFup)
GOW(iswxdigit, iFu)
//GO(__iswxdigit_l, 
GOW(iswxdigit_l, iFup)
GO(isxdigit, iFi)
//GO(__isxdigit_l, 
GOW(isxdigit_l, iFip)
//GO(__ivaliduser, 
GO(jrand48, lFp)
GOW(jrand48_r, iFppp)
//GO(key_decryptsession, iFp!)
//GO(key_decryptsession_pk, iFp!!)
//DATAB(__key_decryptsession_pk_LOCAL, 
//GO(key_encryptsession, iFp!)
//GO(key_encryptsession_pk, iFp!!)
//DATAB(__key_encryptsession_pk_LOCAL, 
//GO(key_gendes, iF!)
//DATAB(__key_gendes_LOCAL, 
//GO(key_get_conv, iFp!)
#ifdef STATICBUILD
//GO(key_secretkey_is_set, 
//GO(key_setnet, 
//GO(key_setsecret, 
#else
GO(key_secretkey_is_set, iFv)
//GO(key_setnet, 
GO(key_setsecret, iFp)
#endif
GOW(kill, iFii)
GO(killpg, iFii)
GO(klogctl, iFipi)
GO(l64a, pFl)
GO(labs, lFl)
GO(lchmod, iFpu)
GOW(lchown, iFpuu)
GOW(lckpwdf, iFv)
GO(lcong48, vFp)
GOW(lcong48_r, iFpp)
GOW(ldexp, dFdi)
GOW(ldexpf, fFfi)
GOW(ldexpl, DFDi)
GO(ldiv, HFll)
GOM(lfind, pFEpppLp)
GO(lgetxattr, lFpppL)
//GO(__libc_alloca_cutoff, 
//GO(__libc_allocate_once_slow, 
//GO(__libc_allocate_rtsig, 
//GO(__libc_allocate_rtsig_private, 
//GO(__libc_alloc_buffer_alloc_array, 
//GO(__libc_alloc_buffer_allocate, 
//GO(__libc_alloc_buffer_copy_bytes, 
//GO(__libc_alloc_buffer_copy_string, 
//GO(__libc_alloc_buffer_create_failure, 
GO(__libc_calloc, pFLL)
//GO(__libc_clntudp_bufcreate, 
GO(__libc_current_sigrtmax, iFv)
//GO(__libc_current_sigrtmax_private, 
GO(__libc_current_sigrtmin, iFv)
//GO(__libc_current_sigrtmin_private, 
//GO(__libc_dlclose, 
//GO(__libc_dlopen_mode, 
//GO(__libc_dlsym, 
//GO(__libc_dlvsym, 
//GO(__libc_dynarray_at_failure, 
//GO(__libc_dynarray_emplace_enlarge, 
//GO(__libc_dynarray_finalize, 
//GO(__libc_dynarray_resize, 
//GO(__libc_dynarray_resize_clear, 
//GO(__libc_fatal, 
//GO(__libc_fcntl64, 
//GO(__libc_fork, 
GO(__libc_free, vFp)
//GO(__libc_freeres, 
//GO(__libc_ifunc_impl_list, 
//GO(__libc_init_first, 
//GO(__libc_longjmp, 
//GO(__libc_mallinfo, 
GO(__libc_malloc, pFL)
//GO(__libc_mallopt, 
GO(__libc_memalign, pFLL)
//GO(__libc_msgrcv, 
//GO(__libc_msgsnd, 
//GO(__libc_pread, 
//GO(__libc_pthread_init, 
GO(__libc_pvalloc, pFL)
//GO(__libc_pwrite, 
//GO(__libc_readline_unlocked, 
GO(__libc_realloc, pFpL)
//GO(__libc_reallocarray, 
//GO(__libc_rpc_getport, 
//GO(__libc_sa_len, 
//GO(__libc_scratch_buffer_grow, 
//GO(__libc_scratch_buffer_grow_preserve, 
//GO(__libc_scratch_buffer_set_array_size, 
//GOW(__libc_secure_getenv, 
//GO(__libc_siglongjmp, 
GOM(__libc_start_main, iFEpippppp)
//GO(__libc_system, 
//GO(__libc_thread_freeres, 
GO(__libc_valloc, pFL)
//GO(__libc_vfork, 
GOW(link, iFpp)
GO(linkat, iFipipi)
GOW(listen, iFii)
GO(listxattr, lFppL)
GO(llabs, IFI)
GO(lldiv, HFII)
GO(llistxattr, lFppL)
//GO(llseek, // Deprecated
//DATAB(loc1, 
//DATAB(loc2, 
GO(localeconv, pFv)
GO(localtime, pFp)
GO2(localtime64, pFp, localtime)
GOW(localtime_r, pFpp)
GO2(localtime64_r, pFpp, localtime_r)   //Weak
GO(lockf, iFiiI)
GO(lockf64, iFiil)
//DATAB(locs, 
GOWM(_longjmp, vFEpi)
GOWM(longjmp, vFEpi)
GOM(__longjmp_chk, vFEpi)
GO(lrand48, lFv)
GO(lrand48_r, iFpp)
GO(lremovexattr, iFpp)
GOM(lsearch, pFEpppLp)
GO(__lseek, IFiIi)
GO(lseek, IFiIi)
GO(lseek64, lFili)
GO(lsetxattr, iFpppLi)
GOM(lstat, iFEpp)
GOM(lstat64, iFEpp)
GO(lutimes, iFpp)
GOM(__lxstat, iFEipp)
GOM(__lxstat64, iFEipp)
GO(__madvise, iFpLi)
GOW(madvise, iFpLi)
GOWM(makecontext, iFEppiV)
GOWM(mallinfo, pFEp)
GO(malloc, pFL)
//GO(malloc_get_state, // Obsolete
GOW(malloc_info, iFip)
//GO(malloc_set_state, // Obsolete
GOW(malloc_stats, vFv)
GO(malloc_trim, iFL)
GO(malloc_usable_size, LFp)
GOW(mallopt, iFii)
//DATAB(mallwatch, 
GO(mblen, iFpU)
GO(__mbrlen, LFpLp)
GO(mbrlen, LFpLp)
//GO(mbrtoc16, LFppL!)
GOW(mbrtoc32, LFppLp)
GO(__mbrtowc, LFppLp)
GO(mbrtowc, LFppLp)
GOW(mbsinit, iFp)
GO(mbsnrtowcs, LFppLLp)
GO(__mbsnrtowcs_chk, LFppLLpL)
GO(mbsrtowcs, LFppLp)
GO(__mbsrtowcs_chk, LFppLpL)
GO(mbstowcs, LFppL)
GO(__mbstowcs_chk, LFppLL)
GO(mbtowc, iFppL)
//GO(mcheck, iF@)
GO(mcheck_check_all, vFv)
//GO(mcheck_pedantic, iF@)
//GO(_mcleanup, 
GO(_mcount, vFpp)
GO2(mcount, vFpp, _mcount)
GO(memalign, pFLL)
GO(memccpy, pFppiL)
GO(memchr, pFpiL)
GO(memcmp, iFppL)
GO(__memcmpeq, iFppL)
GO(memcpy, pFppL)
GO(__memcpy_chk, pFppuL)
GO(memfd_create, iFpu)
GO(memfrob, pFpL)
GO(memmem, pFpLpL)
GO(memmove, pFppU)
GO(__memmove_chk, pFppLL)
GO(__mempcpy, pFppL)
GO(mempcpy, pFppL)
GO(__mempcpy_chk, pFppLL)
GO(memrchr, pFpiL)
GO(memset, pFpiL)
GO(__memset_chk, pFpiLL)
//GO(__mempcpy_small, 
//GO(__merge_grp, 
GO(mincore, iFpLp)
GOW(mkdir, iFpu)
GO(mkdirat, iFipu)
GO(mkdtemp, pFp)
GO(mkfifo, iFpu)
GO(mkfifoat, iFipu)
GO(mknod, iFpuL)
GO(mknodat, iFipuL)
GO(mkostemp, iFpi)
GOW(mkostemp64, iFpi)
GO(mkostemps, iFpii)
GOW(mkostemps64, iFpii)
GO(mkstemp, iFp)
GOW(mkstemp64, iFp)
GO(mkstemps, iFpi)
GOW(mkstemps64, iFpi)
GO(__mktemp, pFp)
GOW(mktemp, pFp)
GO(mktime, lFp)
GO(mlock, iFpL)
GO(mlock2, iFpLu)
GO(mlockall, iFi)
//GO(__mmap, 
GOWM(mmap, pFEpLiiil)
GOWM(mmap64, pFEpLiiil)
GOW(modf, dFdp)
GOW(modff, fFfp)
GOW(modfl, DFDp)
//GOW(modify_ldt, // Deprecated
//GOW(moncontrol, 
//GO(__monstartup, 
//GOW(monstartup, 
//DATA(__morecore, // Compat-only
GOW(mount, iFpppLp)
GO(mprobe, iFp)
//GO(__mprotect, 
GOWM(mprotect, iFEpLi)
GO(mrand48, IFv)
GO(mrand48_r, iFpp)
GOM(mremap, pFEpLLiN) //weal
GO(msgctl, iFiip)
GO(msgget, iFii)
GOW(msgrcv, lFipLli)
GOW(msgsnd, iFipLi)
GO(msync, iFpLi)
GO(mtrace, vFv)
GO(munlock, iFpL)
GO(munlockall, iFv)
//GO(__munmap, 
GOWM(munmap, iFEpL)
GO(muntrace, vFv)
GO(name_to_handle_at, iFipppi)
GO(__nanosleep, iFpp)
GOW(nanosleep, iFpp)
//GO(__nanosleep_nocancel, 
//GO(__netlink_assert_response, 
#ifdef STATICBUILD
//GO(netname2host, iFppi)
//GO(netname2user, iFppppp)
#else
GO(netname2host, iFppi)
GO(netname2user, iFppppp)
#endif
GO(__newlocale, pFipp)
GOW(newlocale, pFipp)
//GO(nfsservctl, // Deprecated
//GO(nftw, iFp@ii)
GOM(nftw64, iFEppii)
GOW(ngettext, pFppL)
GO(nice, iFi)
//DATAB(_nl_domain_bindings, 
GO(nl_langinfo, pFi)
GO(__nl_langinfo_l, pFup)
GOW(nl_langinfo_l, pFip)
//DATAB(_nl_msg_cat_cntr, 
GO(nrand48, lFp)
GOW(nrand48_r, iFppp)
//GO(__nss_configure_lookup, 
//GO(__nss_database_lookup, 
//GO(__nss_disable_nscd, 
//GO(_nss_files_parse_grent, 
//GO(_nss_files_parse_pwent, 
//GO(_nss_files_parse_sgent, 
//GO(_nss_files_parse_spent, 
//GO(__nss_group_lookup, 
//GO(__nss_group_lookup2, 
//GO(__nss_hash, 
//GO(__nss_hostname_digits_dots, 
//GO(__nss_hosts_lookup, 
//GO(__nss_hosts_lookup2, 
//GO(__nss_lookup, 
//GO(__nss_lookup_function, 
//GO(__nss_next, 
//GO(__nss_next2, 
//GO(__nss_passwd_lookup, 
//GO(__nss_passwd_lookup2, 
//GO(__nss_services_lookup2, 
GOW(ntohl, uFu)
GOW(ntohs, WFW)
GOW(ntp_adjtime, iFp)
//GO(ntp_gettime, iF!)
//GO(ntp_gettimex, iF!)
//DATAB(_null_auth, 
//DATAB(_obstack, 
//GO(_obstack_allocated_p, 
//DATA(obstack_alloc_failed_handler, 
GOM(_obstack_begin, iFpLLpp)
//GO(_obstack_begin_1, 
//DATA(obstack_exit_failure, 
GOM(_obstack_free, vFpp)
GOM(obstack_free, vFpp)
//GO(_obstack_memory_used, 
GOM(_obstack_newchunk, vFEpi)
//GOW(obstack_printf, iFppV)
//GO(__obstack_printf_chk, 
GOWM(obstack_vprintf, iFEppA)
//GO(__obstack_vprintf_chk, 
//GOW(on_exit, iF@p)
GOWM(__open, iFEpOu)
GOWM(open, iFEpOu)
GO(__open_2, iFpO)
//GOW(__open64, 
GOWM(open64, iFEpOu)
GO(__open64_2, iFpO)
//GO(__open64_nocancel, 
GOW(openat, iFipON)
GO(__openat_2, iFipO)
GOW(openat64, iFipON)
GO(__openat64_2, iFipO)
GO(open_by_handle_at, iFipi)
//GO(__open_catalog, 
GOW(opendir, pFp)
GO(openlog, vFpii)
GOW(open_memstream, pFpp)
//GO(__open_nocancel, 
GO(open_wmemstream, pFpp)
DATA(optarg, sizeof(void*))
DATA(opterr, sizeof(int))
DATA(optind, sizeof(int))
DATA(optopt, sizeof(int))
GO(__overflow, iFpi)
GO(parse_printf_format, LFpLp)
//GO(passwd2des, // Deprecated
GO(pathconf, lFpi)
GOW(pause, iFv)
//GO(__pause_nocancel, 
GO(pclose, iFp)
GO(perror, vFp)
GOW(personality, iFL)
GO(__pipe, iFp)
GOW(pipe, iFp)
GO(pipe2, iFpO)
//GO(pivot_root, // Deprecated
GO(pkey_alloc, iFuu)
GO(pkey_free, iFi)
GO(pkey_get, iFi)
GO(pkey_mprotect, iFpLii)
GO(pkey_set, iFiu)
//GO(pmap_getmaps, pF!)
//GO(pmap_getport, WF!LLu)
//GO(pmap_rmtcall, uF!LLL@p@p?p)
#ifdef STATICBUILD
//GO(pmap_set, iFLLiW)
//GO(pmap_unset, iFLL)
#else
GO(pmap_set, iFLLiW)
GO(pmap_unset, iFLL)
#endif
GO(__poll, iFpLi)
GO(poll, iFpLi)
GO(__poll_chk, iFpuiL)
GO(popen, pFpp)
GO(posix_fadvise, iFilli)
GO(posix_fadvise64, iFilli)
GO(posix_fallocate, iFill)
GO(posix_fallocate64, iFill)
//GO(__posix_getopt, 
GO(posix_madvise, iFpLi)
GO(posix_memalign, iFpLL)
GOW(posix_openpt, iFi)
GOM(posix_spawn, iFEpppppp)
GOW(posix_spawnattr_destroy, iFp)
GO(posix_spawnattr_getflags, iFpp)
GO(posix_spawnattr_getpgroup, iFpp)
GO(posix_spawnattr_getschedparam, iFpp)
GO(posix_spawnattr_getschedpolicy, iFpp)
GO(posix_spawnattr_getsigdefault, iFpp)
GO(posix_spawnattr_getsigmask, iFpp)
GOW(posix_spawnattr_init, iFp)
GOW(posix_spawnattr_setflags, iFpw)
GO(posix_spawnattr_setpgroup, iFpi)
GO(posix_spawnattr_setschedparam, iFpp)
GO(posix_spawnattr_setschedpolicy, iFpi)
GOW(posix_spawnattr_setsigdefault, iFpp)
GOW(posix_spawnattr_setsigmask, iFpp)
GO(posix_spawn_file_actions_addchdir_np, iFpp)
GOW(posix_spawn_file_actions_addclose, iFpi)
GOW(posix_spawn_file_actions_adddup2, iFpii)
GO(posix_spawn_file_actions_addfchdir_np, iFpi)
GOW(posix_spawn_file_actions_addopen, iFpipiu)
GOW(posix_spawn_file_actions_destroy, iFp)
GOW(posix_spawn_file_actions_init, iFp)
GOM(posix_spawnp, iFEpppppp)
GO(ppoll, iFpLpp)
GO(__ppoll_chk, iFpuppL)
GOWM(prctl, iFEiLLLL)
GO(pread, IFipUI)
GOW(__pread64, lFipLI)
GO(pread64, lFipLl)
//GO(__pread64_chk, 
GO(__pread_chk, lFipLlL)
GO(preadv, lFipil)
GO(preadv2, lFipili)
GO(preadv64, lFipil)
GO(preadv64v2, lFipili)
GOM(printf, iFEpV)
GOM(__printf_chk, iFEipV)
//GO(__printf_fp, 
//GO(printf_size, 
//GO(printf_size_info, 
GO(prlimit, iFiupp)
GOW(prlimit64, iFiupp)
GO(process_vm_readv, lFipLpLL)
GO(process_vm_writev, lFipLpLL)
GO(profil, iFpLLu)
//GO(__profile_frequency, 
DATA(__progname, sizeof(void*))
DATA(__progname_full, sizeof(void))
GOW(pselect, iFippppp)
GO(psiginfo, vFpp)
GO(psignal, vFip)
#ifdef STATICBUILD
//GO(__sF, vFip)
//GO(__assert2, vFip)
#else
GO(__sF, vFip)
GO(__assert2, vFip)
#endif

GOM(ptrace, lFEuipp)
GO(ptsname, pFi)
GOW(ptsname_r, iFipL)
//GO(__ptsname_r_chk, 
GOW(putc, iFip)
GO(putchar, iFi)
GO(putchar_unlocked, iFi)
GOW(putc_unlocked, iFip)
GO(putenv, iFp)
GO(putgrent, iFpp)
//GO(putmsg, // Deprecated
//GO(putpmsg, // Deprecated
GO(putpwent, iFpp)
GOW(puts, iFp)
//GO(putsgent, 
GO(putspent, iFpp)
GOW(pututline, pFp)
GO(pututxline, pFp)
GO(putw, iFip)
GO(putwc, uFup)
GO(putwchar, uFu)
GO(putwchar_unlocked, uFu)
GO(putwc_unlocked, uFup)
GO(pvalloc, pFL)
GO(pwrite, IFipUI)
//GOW(__pwrite64, 
GO(pwrite64, lFipLl)
GO(pwritev, lFipil)
GO(pwritev2, lFipili)
GO(pwritev64, lFipil)
GO(pwritev64v2, lFipili)
GO(qecvt, pFDipp)
GOW(qecvt_r, iFDipppL)
GO(qfcvt, pFDipp)
GO(qfcvt_r, iFDipppL)
GO(qgcvt, pFDip)
GOM(qsort, vFEpLLp)
GOWM(qsort_r, vFEpLLpp)
//GO(query_module, // Deprecated
GO(quick_exit, vFi)
GO(quotactl, iFipip)
GO(raise, iFi)
GO(rand, iFv)
GOW(random, lFv)
GOW(random_r, iFpp)
GO(rand_r, iFp)
GO(__rawmemchr, pFpi)
GO(rawmemchr, pFpi)
GO(rcmd, iFpWpppp)
GO(rcmd_af, iFpWppppW)
//DATAB(__rcmd_errstr, 
GOW(__read, lFipL)
GO(read, lFipL)
GO(readahead, lFilL)
GO(__read_chk, lFipLL)
GOW(readdir, pFp) // struct dirent is 280 bytes on x86_64 and ARM64
GOW(readdir64, pFp)
GOW(readdir64_r, iFppp)
GOW(readdir_r, iFppp)
GOWM(readlink, lFEppL)
GOM(readlinkat, iFEippL)
//GO(__readlinkat_chk, 
//GO(__readlink_chk, 
//GO(__read_nocancel, 
GOW(readv, lFipi)
GO(realloc, pFpL)
GO(reallocarray, pFpLL)
GOM(realpath, pFEpp)
GO2(__realpath_chk, pFEppv, my_realpath)
GO(reboot, iFi)
//GOW(re_comp, // Deprecated
GOW(re_compile_fastmap, iFp)
GOW(re_compile_pattern, pFpLp)
GOW(__recv, lFipLi)
GO(recv, lFipLi)
GO(__recv_chk, lFipLLi)
GOW(recvfrom, lFipLipp)
//GO(__recvfrom_chk, 
GO(recvmmsg, iFipuip)
GO(recvmsg, lFipi)
//GOW(re_exec, // Deprecated
GOW(regcomp, iFppi)
GOW(regerror, LFippL)
GO(regexec, iFppLpi)
GOW(regfree, vFp)
GOM(__register_atfork, iFEpppp)
//GOW(register_printf_function, 
GOW(register_printf_modifier, iFp)
GOWM(register_printf_specifier, iFEipp)
GOWM(register_printf_type, iFEp)
//GO(registerrpc, // Deprecated?
GO(remap_file_pages, iFpLiLi)
//GOW(re_match, iFppii!)
//GOW(re_match_2, iFppipii!i)
//DATA(re_max_failures, 
GO(remove, iFp)
GO(removexattr, iFpp)
GO(remque, vFp)
GO(rename, iFpp)
GOW(renameat, iFipip)
GOW(renameat2, iFipipu)
//DATAB(_res, 
//GOW(re_search, iFppiii!)
//GOW(re_search_2, iFppipiii!i)
//GOW(re_set_registers, vFp!upp)
GOW(re_set_syntax, LFL)
//DATAB(_res_hconf, 
GO(__res_iclose, vFpi)
GO(__res_init, iFv)
GO(__res_nclose, vFp)
GO(__res_ninit, iFp)
//GO(__resolv_context_get, 
//GO(__resolv_context_get_override, 
//GO(__resolv_context_get_preinit, 
//GO(__resolv_context_put, 
//DATA(__resp, 
//GO(__res_randomid, 
GO(__res_state, pFv)
//DATAB(re_syntax_options, 
GOW(revoke, iFp)
GO(rewind, vFp)
GOW(rewinddir, vFp)
GO(rexec, iFpipppp)
GO(rexec_af, iFpippppW)
//DATAB(rexecoptions, 
GO(rindex, pFpi)
GOW(rmdir, iFp)
#ifdef STATICBUILD
//DATAB(rpc_createerr, 
//GO(_rpc_dtablesize, iFv)
//GO(__rpc_thread_createerr, !Fv)
//GO(__rpc_thread_svc_fdset, !Fv)
//GO(__rpc_thread_svc_max_pollfd, pFv)
//GO(__rpc_thread_svc_pollfd, pFv)
#else
//DATAB(rpc_createerr, 
GO(_rpc_dtablesize, iFv)
//GO(__rpc_thread_createerr, !Fv)
//GO(__rpc_thread_svc_fdset, !Fv)
GO(__rpc_thread_svc_max_pollfd, pFv)
GO(__rpc_thread_svc_pollfd, pFv)
#endif
GO(rpmatch, iFp)
GO(rresvport, iFp)
GO(rresvport_af, iFpW)
//GO(rtime, iF!!!)
GO(ruserok, iFpipp)
GO(ruserok_af, iFpippW)
//GO(ruserpass, 
GO(__sbrk, pFl)
GO(sbrk, pFl)
GOW(scalbn, dFdi)
GOW(scalbnf, fFfi)
GOW(scalbnl, DFDi)
GOWM(scandir, iFEpppp)
GOWM(scandir64, iFEpppp)
GOWM(scandirat, iFipppp)
//GO(scandirat64, iFipp@@)
//GO(scanf, iFpV)
GO(__sched_cpualloc, pFL)
GO(__sched_cpucount, iFLp)
GO(__sched_cpufree, vFp)
GO(sched_getaffinity, iFiLp)
GO(sched_getcpu, iFv)
GO(__sched_getparam, iFip)
GOW(sched_getparam, iFip)
GO(__sched_get_priority_max, iFi)
GOW(sched_get_priority_max, iFi)
GO(__sched_get_priority_min, iFi)
GOW(sched_get_priority_min, iFi)
//GO(__sched_getscheduler, iFi)
GOW(sched_getscheduler, iFi)
GOW(sched_rr_get_interval, iFip)
GO(sched_setaffinity, iFiLp)
GOW(sched_setparam, iFip)
//GO(__sched_setscheduler, iFiip)
GOW(sched_setscheduler, iFiip)
GO(__sched_yield, iFv)
GOW(sched_yield, iFv)
GO2(__secure_getenv, pFp, getenv)   // not always present
GOW(secure_getenv, pFp)
GO(seed48, pFp)
GOW(seed48_r, iFpp)
GO(seekdir, vFpl)
GO(__select, iFipppp)
GOW(select, iFipppp)
GOM(semctl, iFiiiN)
GO(semget, iFiii)
GO(semop, iFipL)
GO(semtimedop, iFipLp)
GO(__send, lFipLi)
GO(send, lFipLi)
GO(sendfile, lFiipL)
GOW(sendfile64, lFiipL)
GO(__sendmmsg, iFipui)
GOW(sendmmsg, iFipui)
GO(sendmsg, lFipi)
GOW(sendto, lFipLipu)
GO(setaliasent, vFv)
GO(setbuf, vFpp)
GO(setbuffer, vFppL)
GOWM(setcontext, iFEp)
GO(setdomainname, iFpL)
GO(setegid, iFu)
GOW(setenv, iFppi)
//GO(_seterr_reply, vF!!)
GO(seteuid, iFu)
GO(setfsent, iFv)
GO(setfsgid, iFu)
GO(setfsuid, iFu)
GOW(setgid, iFu)
GO(setgrent, vFv)
GO(setgroups, iFLp)
GO(sethostent, vFi)
GO(sethostid, iFl)
GO(sethostname, iFpL)
//GO(setipv4sourcefilter, iFi??uu!)
GOW(setitimer, iFupp)
GOM(_setjmp, iFEp)
GOM(setjmp, iFEp)
GO(setlinebuf, vFp)
GO(setlocale, pFip)
GO(setlogin, iFp)
GO(setlogmask, iFi)
GO(__setmntent, pFpp)
GOW(setmntent, pFpp)
GO(setnetent, vFi)
GO(setnetgrent, iFp)
GO(setns, iFii)
GO(__setpgid, iFpp)
GOW(setpgid, iFii)
GO(setpgrp, iFv)
GOW(setpriority, iFuui)
GO(setprotoent, vFi)
GO(setpwent, vFv)
GOW(setregid, iFuu)
GOW(setresgid, iFuuu)
GOW(setresuid, iFuuu)
GOW(setreuid, iFuu)
GOWM(setrlimit, iFEup)
GOW(setrlimit64, iFup)
GO(setrpcent, vFi)
GO(setservent, vFi)
//GO(setsgent, 
GOW(setsid, iFv)
GOW(setsockopt, iFiiipu)
//GO(setsourcefilter, iFiupuuu!)
GO(setspent, vFv)
GOW(setstate, pFp)
GOW(setstate_r, iFpp)
GOW(settimeofday, iFpp)
GOW(setttyent, iFv)
GOW(setuid, iFu)
GO(setusershell, vFv)
GOW(setutent, vFv)
GO(setutxent, vFv)
GO(setvbuf, iFppiL)
GO(setxattr, iFpppLi)
//GO(sgetsgent, 
//GOW(sgetsgent_r, 
GO(sgetspent, pFp)
GOW(sgetspent_r, iFpppLp)
GO(shmat, pFipi)
GO(shmctl, iFiip)
GO(shmdt, iFp)
GO(shmget, iFiLi)
GOW(shutdown, iFii)
GOWM(__sigaction, iFEipp)
GOWM(sigaction, iFEipp)
GO(__sigaddset, iFpi)
GO(sigaddset, iFpi)
GOWM(sigaltstack, iFEpp)
//GO(sigandset, 
GOW(sigblock, iFi)
//GO(__sigdelset, 
GO(sigdelset, iFpi)
GO(sigemptyset, iFp)
GO(sigfillset, iFp)
GO(siggetmask, iFv)
GO(sighold, iFi)
GO(sigignore, iFi)
GO(siginterrupt, iFii)
GO(sigisemptyset, iFp)
//GO(__sigismember, 
GO(sigismember, iFpi)
GOM(siglongjmp, vFEpi)
GOWM(signal, pFEip)
GO(signalfd, iFipi)
GO(__signbit, iFd)
GO(__signbitf, iFf)
GO(__signbitl, iFD)
GO(sigorset, iFppp)
//GO(__sigpause, 
//GOW(sigpause, // Deprecated
GO(sigpending, iFp)
GOW(sigprocmask, iFipp)
//GOW(sigqueue, iFii?)
GO(sigrelse, iFi)
//GOW(sigreturn, iF!)
GOM(sigset, pFEip)
GOM(__sigsetjmp, iFEpi)
GOM(sigsetjmp, iFEpi)
GOW(sigsetmask, iFi)
GO(sigstack, iFpp)
GO(__sigsuspend, iFp)
GOW(sigsuspend, iFp)
GO(__sigtimedwait, iFppp)
GOW(sigtimedwait, iFppp)
//GO(sigvec, // Deprecated
GOW(sigwait, iFpp)
GOW(sigwaitinfo, iFpp)
GOW(sleep, uFu)
GOM(__snprintf, iFEpLpV)
GOWM(snprintf, iFEpLpV)
GOWM(__snprintf_chk, iFEpLiLpV)
GO(sockatmark, iFi)
GO(__socket, iFiii)
GOW(socket, iFiii)
GOW(socketpair, iFiiip)
GO(splice, lFipipLu)
GOM(sprintf, iFEppV)
GOM(__sprintf_chk, iFEpilpV)
//GOW(sprofil, 
GOW(srand, vFu)
GO(srand48, vFl)
GO(srand48_r, iFlp)
GOW(srandom, vFu)
GOW(srandom_r, iFup)
GOM(sscanf, iFEppV)
//GOW(ssignal, @Fi@)
//GO(sstk, 
GOM(__stack_chk_fail, vFEv)
GOM(stat, iFEpp)
GOM(stat64, iFEpp)
//GO(__statfs, 
GOW(statfs, iFpp)
GOW(statfs64, iFpp)
GOW(statvfs, iFpp)
GOW(statvfs64, iFpp)
GO(statx, iFipiup)
DATA(stderr, sizeof(void*))
DATA(stdin, sizeof(void*))
DATA(stdout, sizeof(void*))
//GOW(step, 
GOM(stime, iFEp)
GO(__stpcpy, pFpp)
GO(stpcpy, pFpp)
GO(__stpcpy_chk, pFppL)
//GO(__stpcpy_small, 
GO(__stpncpy, pFppL)
GO(stpncpy, pFppL)
GO(__stpncpy_chk, pFppLL)
GO(__strcasecmp, iFpp)
GO(strcasecmp, iFpp)
//GO(__strcasecmp_l, 
GO(strcasecmp_l, iFppp)
GO(__strcasestr, pFpp)
GOW(strcasestr, pFpp)
GO(strcat, pFpp)
GO(__strcat_chk, pFppL)
GO(strchr, pFpi)
GO(strchrnul, pFpi)
GO(strcmp, iFpp)
GO(strcoll, iFpp)
GO(__strcoll_l, iFppp)
GOW(strcoll_l, iFppp)
GO(strcpy, pFpp)
GO(__strcpy_chk, pFppL)
//GO(__strcpy_small, 
GO(strcspn, LFpp)
//GO(__strcspn_c1, 
//GO(__strcspn_c2, 
//GO(__strcspn_c3, 
GO(__strdup, pFp)
GOW(strdup, pFp)
GO(strerror, pFi)
GO(strerror_l, pFip)
GO(__strerror_r, pFipL)
GO(strerror_r, pFipL)
//GO(strfmon, lFpLpV)
//GO(__strfmon_l, 
GOW(strfmon_l, lFpLppdddddd)    // should be V, but only double are allowed...
GO(strfromd, iFpLpd)
GO(strfromf, iFpLpf)
GO(strfromf128, iFpLpD)
GO(strfromf32, iFpLpf)
GO(strfromf32x, iFpLpd)
GO(strfromf64, iFpLpd)
GO(strfromf64x, iFpLpD)
GO(strfroml, iFpLpD)
GO(strfry, pFp)
GO(strftime, LFpLpp)
GO(__strftime_l, LFpLppL)
GOW(strftime_l, LFpLppp)
GO(strlen, LFp)
GO(strncasecmp, iFppL)
//GO(__strncasecmp_l, 
GO(strncasecmp_l, iFppLp)
GO(strncat, pFppL)
GO(__strncat_chk, pFppLL)
GO(strncmp, iFppL)
GO(strncpy, pFppL)
GO(__strncpy_chk, pFppLL)
GO(__strndup, pFpL)
GO(strndup, pFpL)
GO(strnlen, LFpL)
GO(strpbrk, pFpp)
//GO(__strpbrk_c2, 
//GO(__strpbrk_c3, 
GO(strptime, pFppp)
//GOW(strptime_l, pFppp!)
GO(strrchr, pFpi)
GOW(strsep, pFpp)
//GO(__strsep_1c, 
//GO(__strsep_2c, 
//GO(__strsep_3c, 
//GO(__strsep_g, 
GO(strsignal, pFi)
GO(strspn, LFpp)
//GO(__strspn_c1, 
//GO(__strspn_c2, 
//GO(__strspn_c3, 
GO(strstr, pFpp)
GO(strtod, dFpp)
GO(__strtod_internal, dFppi)
GO(__strtod_l, dFppp)
GOW(strtod_l, dFppp)
//GO(__strtod_nan, 
GO(strtof, fFpp)
GO(strtof128, DFpp)
//GO(__strtof128_internal, 
//GOW(strtof128_l, 
//GO(__strtof128_nan, 
GOW(strtof32, fFpp)
//GOW(strtof32_l, fFpp!)
GOW(strtof32x, dFpp)
//GOW(strtof32x_l, dFpp!)
GOW(strtof64, dFpp)
//GOW(strtof64_l, dFpp!)
GOW(strtof64x, DFpp)
//GOW(strtof64x_l, DFpp!)
GO(__strtof_internal, fFppi)
GO(__strtof_l, fFppL)
GOW(strtof_l, fFppp)
//GO(__strtof_nan, 
GO(strtoimax, lFppi)
GO(strtok, pFpp)
GO(__strtok_r, pFppp)
GOW(strtok_r, pFppp)
//GO(__strtok_r_1c, 
GO(strtol, lFppi)
GO(strtold, DFpp)
GO(__strtold_internal, DFppi)
GO(__strtold_l, DFppp)
GOW(strtold_l, DFppp)
//GO(__strtold_nan, 
GO(__strtol_internal, lFppii)
GO(__strtol_l, lFppi)
GO(strtol_l, lFppip)
GOW(strtoll, IFppi)
GO(__strtoll_internal, IFppii)
GOW(__strtoll_l, IFppip)
GOW(strtoll_l, IFppip)
GOW(strtoq, IFppi)
GO(strtoul, LFppi)
GO(__strtoul_internal, LFppii)
//GO(__strtoul_l, 
GO(strtoul_l, LFppip)
GOW(strtoull, UFppi)
GO(__strtoull_internal, UFppii)
GOW(__strtoull_l, LFppip)
GOW(strtoull_l, UFppip)
GO(strtoumax, LFppi)
GOW(strtouq, UFppi)
//GO(__strverscmp, 
GOW(strverscmp, iFpp)
GO(strxfrm, LFppL)
GO(__strxfrm_l, LFppLL)
GO(strxfrm_l, LFppLp)
//GO(stty, // Deprecated
#ifdef STATICBUILD
//DATAB(svcauthdes_stats, 
//GO(svcerr_auth, vF!u)
//GO(svcerr_decode, vF!)
//GO(svcerr_noproc, vF!)
//GO(svcerr_noprog, vF!)
//GO(svcerr_progvers, vF!LL)
//GO(svcerr_systemerr, vF!)
//GO(svcerr_weakauth, vF!)
//GO(svc_exit, vFv)
//GO(svcfd_create, !Fiuu)
//DATAB(svc_fdset, 
//GO(svc_getreq, vFi)
//GO(svc_getreq_common, vFi)
//GO(svc_getreq_poll, vFpi)
//GO(svc_getreqset, vF!)
//DATAB(svc_max_pollfd, 
//DATAB(svc_pollfd, 
//GO(svcraw_create, !Fv)
//GO(svc_register, iF!LL@L)
//GO(svc_run, vFv)
//GO(svc_sendreply, iF!@p)
//GO(svctcp_create, !Fiuu)
//GO(svcudp_bufcreate, !Fiuu)
//GO(svcudp_create, !Fi)
//GO(svcudp_enablecache, 
//GO(svcunix_create, !Fiuup)
//GO(svcunixfd_create, 
//GO(svc_unregister, vFLL)
#else
//DATAB(svcauthdes_stats, 
//GO(svcerr_auth, vF!u)
//GO(svcerr_decode, vF!)
//GO(svcerr_noproc, vF!)
//GO(svcerr_noprog, vF!)
//GO(svcerr_progvers, vF!LL)
//GO(svcerr_systemerr, vF!)
//GO(svcerr_weakauth, vF!)
GO(svc_exit, vFv)
//GO(svcfd_create, !Fiuu)
//DATAB(svc_fdset, 
GO(svc_getreq, vFi)
GO(svc_getreq_common, vFi)
GO(svc_getreq_poll, vFpi)
//GO(svc_getreqset, vF!)
//DATAB(svc_max_pollfd, 
//DATAB(svc_pollfd, 
//GO(svcraw_create, !Fv)
//GO(svc_register, iF!LL@L)
GO(svc_run, vFv)
//GO(svc_sendreply, iF!@p)
//GO(svctcp_create, !Fiuu)
//GO(svcudp_bufcreate, !Fiuu)
//GO(svcudp_create, !Fi)
//GO(svcudp_enablecache, 
//GO(svcunix_create, !Fiuup)
//GO(svcunixfd_create, 
GO(svc_unregister, vFLL)
#endif
GO(swab, vFppl)
GOWM(swapcontext, iFEpp)
//GOW(swapoff, 
//GOW(swapon, 
GOM(swprintf, iFEpLpV)
GOM(__swprintf_chk, iFEpLiLpV)
GOM(swscanf, iFEppV)
GOW(symlink, iFpp)
GO(symlinkat, iFpip)
GO(sync, vFv)
GO(sync_file_range, iFillu)
GO(syncfs, iFi)
GOM(syscall, lFEv)
GOM(__sysconf, lFEi)
GOM(sysconf, IFEi)
#ifdef STATICBUILD
//GO(__sysctl, 
//GO(sysctl, 
#else
GO(__sysctl, iFpipppL)
GOW(sysctl, iFpipppL) // Deprecated
#endif
#ifdef STATICBUILD
//DATA(_sys_errlist, sizeof(void*))
//DATA(sys_errlist, sizeof(void*))
#else
DATA(_sys_errlist, sizeof(void*))
DATA(sys_errlist, sizeof(void*))
#endif
GOW(sysinfo, iFp)
GOM(syslog, vFEipV)
GOM(__syslog_chk, vFEiipV)
//DATA(sys_sigabbrev, 
//DATA(_sys_siglist, 
//DATA(sys_siglist, 
GOW(system, iFp)
GOM(__sysv_signal, pFEip)
GOWM(sysv_signal, pFEip)
GOW(tcdrain, iFi)
GO(tcflow, iFii)
GO(tcflush, iFii)
GOW(tcgetattr, iFip)
GO(tcgetpgrp, iFi)
GO(tcgetsid, iFi)
GO(tcsendbreak, iFii)
GO(tcsetattr, iFiip)
GO(tcsetpgrp, iFii)
//GO(__tdelete, 
GOWM(tdelete, pFEppp)
GOWM(tdestroy, vFEpp)
GO(tee, lFiiLu)
GO(telldir, lFp)
GO(tempnam, pFpp)
GOW(textdomain, pFp)
//GO(__tfind, 
GOWM(tfind, pFEppp)
GOW(tgkill, iFiii)
//GO(thrd_current, 
GO(thrd_exit, vFp)
//GO(thrd_equal, 
//GO(thrd_sleep, 
//GO(thrd_yield, 
GO(time, lFp)
GO(timegm, lFp)
GOW(timelocal, lFp)
GO(timerfd_create, iFii)
GO(timerfd_gettime, iFip)
GO(timerfd_settime, iFiipp)
GOW(times, lFp)
GO(timespec_get, iFpi)
DATAB(__timezone, sizeof(void*))
DATAV(timezone, sizeof(void*))
GO(tmpfile, pFv)
GOW(tmpfile64, pFv)
GO(tmpnam, pFp)
GO(tmpnam_r, pFp)
GO(toascii, iFi)
//GOW(__toascii_l, 
GO(_tolower, iFi)
GO(tolower, iFi)
//GO(__tolower_l, iFi!)
GOW(tolower_l, iFip)
GO(_toupper, iFi)
GO(toupper, iFi)
//GO(__toupper_l, iFi!)
GOW(toupper_l, iFip)
//GO(__towctrans, 
GOW(towctrans, uFup)
//GO(__towctrans_l, 
GOW(towctrans_l, uFupp)
GOW(towlower, uFu)
GO(__towlower_l, iFip)
GOW(towlower_l, uFup)
GOW(towupper, uFu)
GO(__towupper_l, iFip)
GOW(towupper_l, uFup)
//GO(tr_break, 
GO(truncate, iFpI)
GO(truncate64, iFpl)
//GO(__tsearch, 
GOWM(tsearch, pFEppp)
GO(ttyname, pFi)
GO(ttyname_r, iFipL)
//GO(__ttyname_r_chk, 
GO(ttyslot, iFv)
//GO(__twalk, 
GOWM(twalk, vFEpp)
//GOW(twalk_r, vFp@p)
DATA(__tzname, sizeof(void*))
DATA(tzname, sizeof(void*)) //type V
GOW(tzset, vFv)
GO(ualarm, uFuu)
GO(__uflow, iFp)
GOW(ulckpwdf, iFv)
//GOW(ulimit, lFiV) // Deprecated
GOW(umask, uFu)
GOW(umount, iFp)
GOW(umount2, iFpi)
GOWM(uname, iFp)
GO(__underflow, iFp)
GOW(ungetc, iFip)
GO(ungetwc, uFup)
GOW(unlink, iFp)
GO(unlinkat, iFipi)
GO(unlockpt, iFi)
GOW(unsetenv, iFp)
GO(unshare, iFi)
GOW(updwtmp, vFpp)
GO(updwtmpx, vFpp)
//GO(uselib, // Deprecated
GO(__uselocale, pFp)
GOW(uselocale, pFp)
#ifdef STATICBUILD
//GO(user2netname, 
#else
GO(user2netname, iFpup)
#endif
GO(usleep, iFu)
//GO(ustat, // Deprecated
GO(utime, iFpp)
GO(utimensat, iFippi)
GOW(utimes, iFpp)
GOW(utmpname, iFp)
GO(utmpxname, iFp)
GO(valloc, pFL)
GOWM(vasprintf, iFEppA)
GOM(__vasprintf_chk, iFEpipp)
GOM(vdprintf, iFEipA)
GOM(__vdprintf_chk, iFEiipA)
//GO(verr, vFipA)
//GO(verrx, vFipA)
GOW(versionsort, iFpp)
GOW(versionsort64, iFpp)
//GO(__vfork, 
GOWM(vfork, iFEv)
GOM(vfprintf, iFEppA)
GOM(__vfprintf_chk, iFEpvpp)
//GO(__vfscanf, 
GOWM(vfscanf, iFEppA)
GOWM(vfwprintf, iFEppA)
GOM(__vfwprintf_chk, iFEpipA)
//GOW(vfwscanf, iFppA)
GO(vhangup, iFv)
//GO(vlimit, // Deprecated
GO(vmsplice, lFipLu)
GOM(vprintf, iFEpA)
GOM(__vprintf_chk, iFEvpp)
GOWM(vscanf, iFEpA)
GOWM(__vsnprintf, iFEpLpA)
GOWM(vsnprintf, iFEpLpA)
GOM(__vsnprintf_chk, iFEpLiLpp)
GOWM(vsprintf, iFEppA)
GOM(__vsprintf_chk, iFEpvvppp)
GOWM(__vsscanf, iFEppp)
GOWM(vsscanf, iFEppA)
GOWM(vswprintf, iFEpLpA)
GOWM(__vswprintf_chk, iFEpuvvppp)
GOM(vswscanf, iFEppA)
GOM(vsyslog, vFEipA)
GOM(__vsyslog_chk, vFEiipA)
//GO(vtimes, // Deprecated
//GO(vwarn, vFpA)
//GO(vwarnx, vFpA)
GOM(vwprintf, iFEpA)
//GO(__vwprintf_chk, 
//GO(vwscanf, iFpA)
GOW(__wait, iFp)
GOW(wait, iFp)
GOW(wait3, iFpip)
GOW(wait4, iFipip)
GOW(waitid, iFuupi)
GO(__waitpid, iFipi)
GOW(waitpid, iFipi)
GOM(warn, vFEpV)
GOM(warnx, vFEpV)
GOW(wcpcpy, pFpp)
//GO(__wcpcpy_chk, 
GO(wcpncpy, pFppL)
//GO(__wcpncpy_chk, 
GO(wcrtomb, LFpup)
//GO(__wcrtomb_chk, 
GOW(wcscasecmp, iFpp)
GO(__wcscasecmp_l, iFppp)
GOW(wcscasecmp_l, iFppp)
GOW(wcscat, pFpp)
GO(__wcscat_chk, pFppL)
GO(wcschr, pFpu)
GO(wcschrnul, pFpu)
GO(wcscmp, iFpp)
GOW(wcscoll, iFpp)
GO(__wcscoll_l, iFppp)
GOW(wcscoll_l, iFppp)
GO(wcscpy, pFpp)
GO(__wcscpy_chk, pFppL)
GO(wcscspn, LFpp)
GO(wcsdup, pFp)
GO(wcsftime, LFpLpp)
GO(__wcsftime_l, LFpLppp)
GO(wcsftime_l, LFpLppp)
GO(wcslen, LFp)
GO(wcsncasecmp, iFppL)
//GO(__wcsncasecmp_l, 
//GOW(wcsncasecmp_l, iFppL!)
GO(wcsncat, pFppL)
GO(__wcsncat_chk, pFppLL)
GO(wcsncmp, iFppL)
GOW(wcsncpy, pFppL)
GO(__wcsncpy_chk, pFppLL)
GO(wcsnlen, LFpL)
GO(wcsnrtombs, LFppLLp)
//GO(__wcsnrtombs_chk, 
GO(wcspbrk, pFpp)
GO(wcsrchr, pFpu)
GO(wcsrtombs, LFppLp)
//GO(__wcsrtombs_chk, 
GO(wcsspn, LFpp)
GO(wcsstr, pFpp)
GO(wcstod, dFpp)
//GO(__wcstod_internal, 
//GO(__wcstod_l, 
GOW(wcstod_l, dFppp)
GO(wcstof, fFpp)
GO(wcstof128, DFpp)
//GO(__wcstof128_internal, 
//GOW(wcstof128_l, 
GOW(wcstof32, fFpp)
//GOW(wcstof32_l, fFpp!)
GOW(wcstof32x, dFpp)
//GOW(wcstof32x_l, dFpp!)
GOW(wcstof64, dFpp)
//GOW(wcstof64_l, dFpp!)
GOW(wcstof64x, DFpp)
//GOW(wcstof64x_l, DFpp!)
//GO(__wcstof_internal, 
//GO(__wcstof_l, 
//GOW(wcstof_l, fFpp!)
GO(wcstoimax, lFppi)
GO(wcstok, pFppp)
GO(wcstol, lFppi)
GO(wcstold, DFpp)
//GO(__wcstold_internal, 
//GO(__wcstold_l, 
//GOW(wcstold_l, DFpp!)
//GO(__wcstol_internal, 
//GO(__wcstol_l, 
GOW(wcstol_l, lFppip)
GOW(wcstoll, IFppi)
//GO(__wcstoll_internal, 
//GOW(__wcstoll_l, 
//GOW(wcstoll_l, IFppi!)
GO(wcstombs, LFppL)
//GO(__wcstombs_chk, 
GOW(wcstoq, IFppi)
GO(wcstoul, LFppi)
//GO(__wcstoul_internal, 
//GO(__wcstoul_l, 
GOW(wcstoul_l, LFppip)
GOW(wcstoull, UFppi)
//GO(__wcstoull_internal, 
//GOW(__wcstoull_l, 
//GOW(wcstoull_l, UFppi!)
GO(wcstoumax, LFppi)
GOW(wcstouq, UFppi)
GOW(wcswcs, pFpp)
GO(wcswidth, iFpL)
GO(wcsxfrm, LFppL)
GO(__wcsxfrm_l, LFppLL)
GO(wcsxfrm_l, LFppLp)
GO(wctob, iFu)
GO(wctomb, iFpu)
GO(__wctomb_chk, iFpuL)
GOW(wctrans, pFp)
//GO(__wctrans_l, 
GOW(wctrans_l, pFpp)
GOW(wctype, LFp)
GOW(wctype_l, LFpp)
GO(__wctype_l, LFpp)
GO(wcwidth, iFu)
GO(wmemchr, pFpuL)
GO(wmemcmp, iFppL)
GOW(wmemcpy, pFppL)
GO(__wmemcpy_chk, pFppLL)
GO(wmemmove, pFppL)
GO(__wmemmove_chk, pFppLL)
GOW(wmempcpy, pFppL)
//GO(__wmempcpy_chk, 
GO(wmemset, pFpuL)
GO(__wmemset_chk, pFpuLL)
GO(wordexp, iFppi)
GO(wordfree, vFp)
//GO(__woverflow, 
GOM(wprintf, iFEpV)
GOM(__wprintf_chk, lFEipV)
GOW(__write, lFipL)
GO(write, lFipL)
//GO(__write_nocancel, 
GOW(writev, lFipi)
//GO(wscanf, iFpV)
//GO(__wuflow, 
//GO(__wunderflow, 
//GO(xdecrypt, // Deprecated
//GO(xdr_accepted_reply, 
//GO(xdr_array, iF!ppuu@)
//GO(xdr_authdes_cred, 
//GO(xdr_authdes_verf, 
//GO(xdr_authunix_parms, iF!!)
//GO(xdr_bool, iF!p)
//GO(xdr_bytes, iF!ppu)
//GO(xdr_callhdr, iF!!)
//GO(xdr_callmsg, iF!!)
//GO(xdr_char, iF!p)
//GO(xdr_cryptkeyarg, 
//GO(xdr_cryptkeyarg2, 
//GO(xdr_cryptkeyres, 
//GO(xdr_des_block, iF!!)
//GO(xdr_double, iF!p)
//GO(xdr_enum, iF!p)
//GO(xdr_float, iF!p)
//GO(xdr_free, vF@p)
//GO(xdr_getcredres, 
//GO(xdr_hyper, iF!p)
//GO(xdr_int, iF!p)
//GO(xdr_int16_t, iF!p)
//GO(xdr_int32_t, iF!p)
//GO(xdr_int64_t, iF!p)
//GO(xdr_int8_t, iF!p)
//GO(xdr_keybuf, 
//GO(xdr_key_netstarg, 
//GO(xdr_key_netstres, 
//GO(xdr_keystatus, 
//GO(xdr_long, iF!p)
//GO(xdr_longlong_t, iF!p)
//GO(xdrmem_create, vF!puu)
//GO(xdr_netnamestr, 
//GO(xdr_netobj, iF!!)
//GO(xdr_opaque, iF!pu)
//GO(xdr_opaque_auth, iF!!)
//GO(xdr_pmap, 
//GO(xdr_pmaplist, 
//GO(xdr_pointer, iF!pu@)
//GO(xdr_quad_t, iF!p)
//GO(xdrrec_create, vF!uup@@)
//GO(xdrrec_endofrecord, iF!i)
//GO(xdrrec_eof, iF!)
//GO(xdrrec_skiprecord, iF!)
//GO(xdr_reference, iF!pu@)
//GO(xdr_rejected_reply, 
//GO(xdr_replymsg, iF!!)
//GO(xdr_rmtcall_args, 
//GO(xdr_rmtcallres, 
//GO(xdr_short, iF!p)
//GO(xdr_sizeof, LF@p)
//GO(xdrstdio_create, vF!pu)
//GO(xdr_string, iF!pu)
//GO(xdr_u_char, iF!p)
//GO(xdr_u_hyper, iF!p)
//GO(xdr_u_int, iF!p)
//GO(xdr_uint16_t, iF!p)
//GO(xdr_uint32_t, iF!p)
//GO(xdr_uint64_t, iF!p)
//GO(xdr_uint8_t, iF!p)
//GO(xdr_u_long, iF!p)
//GO(xdr_u_longlong_t, iF!p)
//GO(xdr_union, iF!pp!@)
//GO(xdr_unixcred, 
//GO(xdr_u_quad_t, iF!p)
//GO(xdr_u_short, iF!p)
//GO(xdr_vector, iF!puu@)
#ifdef STATICBUILD
//GO(xdr_void, 
//GO(xdr_wrapstring, iF!p)
#else
GO(xdr_void, iFv)
//GO(xdr_wrapstring, iF!p)
#endif
//GO(xencrypt, 
#ifdef STATICBUILD
//GO(__xmknod, iFipup)
//GO(__xmknodat, iFiipup)
#else
GO(__xmknod, iFipup)
GO(__xmknodat, iFiipup)
#endif
GO(__xpg_basename, pFp)
GOW(__xpg_sigpause, iFi)
GO(__xpg_strerror_r, pFipL)
//GO(xprt_register, vF!)
//GO(xprt_unregister, vF!)
GOM(__xstat, iFEipp)
GOM(__xstat64, iFEipp)

GOM(_Jv_RegisterClasses, vFv)   // dummy

GOM(_ITM_addUserCommitAction, vFEpup)
GOM(_ITM_registerTMCloneTable, vFEpu)
GOM(_ITM_deregisterTMCloneTable, vFEp)

GOM(__register_frame_info, vFpp)    // faked function
GOM(__deregister_frame_info, pFp)

GOM(strlcpy, LFEppL)
GOM(strlcat, LFEppL)

GOWM(__cxa_pure_virtual, vFEv)     // create a function to trap pure virtual call

DATAM(program_invocation_name, sizeof(void*))
DATAM(program_invocation_short_name, sizeof(void*))

DATAM(__libc_single_threaded, 1)

#ifdef STATICBUILD
//GO(iconvctl, 
#else
GO(iconvctl, iFlip)
#endif
GO(dummy__ZnwmSt11align_val_tRKSt9nothrow_t, pFLLp) // for mallochook.c

#ifdef ANDROID
GOM(__libc_init, vFEpppp)
GO(__errno, pFv)
#else
// Those symbols don't exist in non-Android builds
//GOM(__libc_init,
//GO(__errno,
#endif
#ifdef STATICBUILD
GO(dummy_pFLp, pFLp)
GO(dummy_pFpLLp, pFpLLp)
#else
// not needed in no-static build
//GO(dummy_pFLp, pFLp)
//GO(dummy_pFpLLp, pFpLLp)
#endif
