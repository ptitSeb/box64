#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA) && defined(GOS))
#error Meh...
#endif

//socklen_t is u32
// typedef unsigned long int nfds_t;
// key_t is S32
// uid_t is u32

//GO(a64l, 
GO(abort, vFv)
//DATAB(__abort_msg, 
//GO(abs, 
//GO(accept, 
//GO(accept4, 
GOW(access, iFpi)
//GO(acct, 
//GOW(addmntent, 
//GOW(addseverity, 
//GOW(adjtime, 
//GO(__adjtimex, 
//GOW(adjtimex, 
//GOW(advance, 
//GO(alarm, 
GOW(aligned_alloc, pFLL)
GOW(alphasort, iFpp)
GOW(alphasort64, iFpp)
//GO(__arch_prctl, 
//GOW(arch_prctl, 
//DATA(argp_err_exit_status, 
//GOW(argp_error, 
//GOW(argp_failure, 
//GOW(argp_help, 
//GOW(argp_parse, 
//DATAB(argp_program_bug_address, 
//DATAB(argp_program_version, 
//DATAB(argp_program_version_hook, 
//GOW(argp_state_help, 
//GOW(argp_usage, 
//GOW(argz_add, 
//GOW(argz_add_sep, 
//GOW(argz_append, 
//GO(__argz_count, 
//GOW(argz_count, 
//GOW(argz_create, 
//GOW(argz_create_sep, 
//GO(argz_delete, 
//GOW(argz_extract, 
//GOW(argz_insert, 
//GO(__argz_next, 
//GOW(argz_next, 
//GOW(argz_replace, 
//GO(__argz_stringify, 
//GOW(argz_stringify, 
GO(asctime, pFp)
GOW(asctime_r, pFpp)
//GO(__asprintf, 
//GOW(asprintf, 
//GO(__asprintf_chk, 
//GO(__assert, 
GO(__assert_fail, vFppup)
GO(__assert_perror_fail, vFipup)
GO(atof, dFp)
GO(atoi, iFp)
GO(atol, lFp)
GO(atoll, IFp)
//GO(authdes_create, 
//GO(authdes_getucred, 
//GO(authdes_pk_create, 
//GO(_authenticate, 
//GO(authnone_create, 
//GO(authunix_create, 
//GO(authunix_create_default, 
//GO(__backtrace, 
GOM(backtrace, iFEpi)   //weak
//GO(__backtrace_symbols, 
GOM(backtrace_symbols, pFEpi)  //weak
//GO(__backtrace_symbols_fd, 
GOM(backtrace_symbols_fd, vFEpii)   //weak
//GOW(basename, 
GO(bcmp, iFppL)
GO(bcopy, vFppL)
//GO(bdflush, 
GOW(bind, iFipu)
//GO(bindresvport, 
GOW(bindtextdomain, pFpp)
GOW(bind_textdomain_codeset, pFpp)
GOW(brk, iFp)
//GO(__bsd_getpgrp, 
//GOW(bsd_signal, 
GOM(bsearch, pFEppLLp)
GOW(btowc, iFi)
GO(__bzero, vFpL)
GOW(bzero, vFpL)
//GO(c16rtomb, 
//GOW(c32rtomb, 
GOW(calloc, pFLL)
//GO(callrpc, 
//GO(__call_tls_dtors, 
//GOW(canonicalize_file_name, 
//GO(capget, 
//GO(capset, 
GO(catclose, iFp)
GO(catgets, pFpiip)
GO(catopen, pFpi)
//GO(cbc_crypt, 
GO(cfgetispeed, uFp)
GO(cfgetospeed, uFp)
GO(cfmakeraw, vFp)
GO2(cfree, vFp, free)
GO(cfsetispeed, iFpu)
GO(cfsetospeed, iFpu)
GO(cfsetspeed, iFpi)
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
//GO(clnt_broadcast, 
//GO(clnt_create, 
//GO(clnt_pcreateerror, 
//GO(clnt_perrno, 
//GO(clnt_perror, 
//GO(clntraw_create, 
//GO(clnt_spcreateerror, 
//GO(clnt_sperrno, 
//GO(clnt_sperror, 
//GO(clnttcp_create, 
//GO(clntudp_bufcreate, 
//GO(clntudp_create, 
//GO(clntunix_create, 
GO(clock, LFv)
//GO(clock_adjtime, 
//GO(__clock_getcpuclockid, 
//GOW(clock_getcpuclockid, 
//GO(__clock_getres, 
//GOW(clock_getres, 
//GO(__clock_gettime, 
//GOW(clock_gettime, 
//GO(__clock_nanosleep, 
//GOW(clock_nanosleep, 
//GO(__clock_settime, 
//GOW(clock_settime, 
//GO(__clone, 
//GOW(clone, 
//GO(__close, 
GOW(close, iFi)
GOW(closedir, iFp)
GO(closelog, vFv)
//GO(__close_nocancel, 
//GO(__cmsg_nxthdr, 
GO(confstr, LFipL)
//GO(__confstr_chk, 
GOW(__connect, iFipu)
GOW(connect, iFipu)
//GO(copy_file_range, 
//GO(__copy_grp, 
//GOW(copysign, 
//GOW(copysignf, 
//GOW(copysignl, 
//GOW(creat, 
//GOW(creat64, 
//GO(create_module, 
//GO(ctermid, 
GO(ctime, pFp)
GO(ctime_r, pFpp)
//DATA(__ctype32_b, 
//DATA(__ctype32_tolower, 
//DATA(__ctype32_toupper, 
//DATA(__ctype_b, 
GO(__ctype_b_loc, pFv)
GOW(__ctype_get_mb_cur_max, LFv)
//GO(__ctype_init, 
DATAM(__ctype_tolower, sizeof(void*))
GO(__ctype_tolower_loc, pFv)
DATAM(__ctype_toupper, sizeof(void*))
GO(__ctype_toupper_loc, pFv)
//DATAB(__curbrk, 
GO(cuserid, pFp)
GOM(__cxa_atexit, iFEppp)
//GO(__cxa_at_quick_exit, 
GOM(__cxa_finalize, vFEp)
//GO(__cxa_thread_atexit_impl, 
//GO(__cyg_profile_func_enter, 
//GO(__cyg_profile_func_exit, 
//GO(daemon, 
//DATAB(__daylight, 
//GO(__dcgettext, 
//GOW(dcgettext, 
//GOW(dcngettext, 
//GO(__default_morecore, 
//GO(delete_module, 
//GO(des_setparity, 
//GO(__dgettext, 
//GOW(dgettext, 
GO(difftime, dFLL)
GO(dirfd, iFp)
GO(dirname, pFp)
GO(div, UFii)
//GO(_dl_addr, 
//GO(_dl_catch_error, 
//GO(_dl_catch_exception, 
GOM(dl_iterate_phdr, iFEpp) //Weak
//GO(_dl_mcount_wrapper, 
//GO(_dl_mcount_wrapper_check, 
//DATAB(_dl_open_hook, 
//DATAB(_dl_open_hook2, 
//GO(_dl_signal_error, 
//GO(_dl_signal_exception, 
//GO(_dl_sym, 
//GO(_dl_vsym, 
//GOW(dngettext, 
//GO(dprintf, 
//GO(__dprintf_chk, 
//GO(drand48, 
//GO(drand48_r, 
GOW(dup, iFi)
GO(__dup2, iFii)
GOW(dup2, iFii)
GOW(dup3, iFiiO)
GO(__duplocale, pFp)
GOW(duplocale, pFp)
//GO(dysize, 
//GOW(eaccess, 
//GO(ecb_crypt, 
//GO(ecvt, 
//GOW(ecvt_r, 
//GO(endaliasent, 
//GO(endfsent, 
//GO(endgrent, 
//GO(endhostent, 
//GO(__endmntent, 
//GOW(endmntent, 
//GO(endnetent, 
//GO(endnetgrent, 
GO(endprotoent, vFv)
//GO(endpwent, 
//GO(endrpcent, 
//GO(endservent, 
//GO(endsgent, 
//GO(endspent, 
//GOW(endttyent, 
//GO(endusershell, 
//GOW(endutent, 
//GO(endutxent, 
DATA(environ, sizeof(void*))
DATAB(__environ, sizeof(void*))
//GO(envz_add, 
//GO(envz_entry, 
//GO(envz_get, 
//GO(envz_merge, 
//GO(envz_remove, 
//GO(envz_strip, 
GO(epoll_create, iFi)
GO(epoll_create1, iFO)
GO(epoll_ctl, iFiiip)   // need to check about alignment
//GO(epoll_pwait, 
GO(epoll_wait, iFipii)
//GO(erand48, 
//GOW(erand48_r, 
//GO(err, 
//DATAB(errno, 
GO(__errno_location, pFv)
//GOW(error, 
//GOW(error_at_line, 
//DATAB(error_message_count, 
//DATAB(error_one_per_line, 
//DATAB(error_print_progname, 
//GO(errx, 
//GO(ether_aton, 
//GO(ether_aton_r, 
//GO(ether_hostton, 
//GO(ether_line, 
//GO(ether_ntoa, 
//GO(ether_ntoa_r, 
//GO(ether_ntohost, 
//GOW(euidaccess, 
GO(eventfd, iFui)
GO(eventfd_read, iFip)
GO(eventfd_write, iFiU)
//GO(execl, 
//GO(execle, 
GOM(execlp, iFEpV)
//GO(execv, 
//GOW(execve, 
GOM(execvp, iFEpp)
//GOW(execvpe, 
GO(_exit, vFi)
GO(exit, vFi)
GOW(_Exit, vFi)
//GO(explicit_bzero, 
//GO(__explicit_bzero_chk, 
//GO(faccessat, 
//GOW(fallocate, 
//GO(fallocate64, 
//GO(fanotify_init, 
//GO(fanotify_mark, 
//GO(fattach, 
//GO(__fbufsize, 
GOW(fchdir, iFi)
//GO(fchflags, 
GOW(fchmod, iFiu)
GO(fchmodat, iFipui)
GOW(fchown, iFiuu)
//GO(fchownat, 
GO(fclose, iFp)
GOW(fcloseall, iFv)
GOM(__fcntl, iFEiiN)    //Weak
GOM(fcntl, iFEiiN)      //Weak
GOM(fcntl64, iFEiiN)    //Weak
//GO(fcvt, 
//GOW(fcvt_r, 
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
//GO(fexecve, 
GOW(fflush, iFp)
GO(fflush_unlocked, iFp)
//GO(__ffs, 
//GO(ffs, 
//GOW(ffsl, 
//GO(ffsll, 
GOW(fgetc, iFp)
//GOW(fgetc_unlocked, 
//GO(fgetgrent, 
//GOW(fgetgrent_r, 
GO(fgetpos, lFpp)
GO(fgetpos64, lFpp)
//GO(fgetpwent, 
//GOW(fgetpwent_r, 
GOW(fgets, pFpip)
GO(__fgets_chk, pFpLip)
//GO(fgetsgent, 
//GOW(fgetsgent_r, 
//GO(fgetspent, 
//GOW(fgetspent_r, 
//GOW(fgets_unlocked, 
//GO(__fgets_unlocked_chk, 
GOW(fgetwc, uFp)
//GOW(fgetwc_unlocked, 
//GO(fgetws, 
//GO(__fgetws_chk, 
//GO(fgetws_unlocked, 
//GO(__fgetws_unlocked_chk, 
//GO(fgetxattr, 
GOW(fileno, iFp)
GOW(fileno_unlocked, iFp)
GO(__finite, iFd)
GOW(finite, iFd)
GO(__finitef, iFf)
GOW(finitef, iFf)
//GO(__finitel, 
//GOW(finitel, 
//GO(__flbf, 
//GO(flistxattr, 
GOW(flock, iFii)
GOW(flockfile, vFp)
//GOW(_flushlbf, 
//GO(fmemopen, 
//GO(fmemopen, 
//GO(fmtmsg, 
GO(fnmatch, iFppi)
GOM(fopen, pFEpp)
GOM(fopen64, pFEpp)  // Weak
//GO(fopencookie, 
//GO(__fork, 
GOM(fork, lFEv) // Weak
//GO(__fortify_fail, 
//GOW(fpathconf, 
//GO(__fpending, 
GOM(fprintf, iFEppV)
GOM(__fprintf_chk, iFEpipV)
//DATA(__fpu_control, 
//GO(__fpurge, 
GO(fputc, iFip)
GO(fputc_unlocked, iFip)
GOW(fputs, iFpp)
GOW(fputs_unlocked, iFpp)
GO(fputwc, iFip)
GO(fputwc_unlocked, iFip)
GO(fputws, iFpp)
GO(fputws_unlocked, iFpp)
GOW(fread, LFpLLp)
//GO(__freadable, 
//GO(__fread_chk, 
//GO(__freading, 
GOW(fread_unlocked, LFpLLp)
GO(__fread_unlocked_chk, LFpLLLp)
GO(free, vFp)
GO(freeaddrinfo, vFp)
GOW(freeifaddrs, vFp)
GO(__freelocale, vFp)
GOW(freelocale, vFp)
GO(fremovexattr, iFip)
GO(freopen, pFppp)
GO(freopen64, pFppp)
//GOW(frexp, 
//GOW(frexpf, 
//GOW(frexpl, 
GOM(fscanf, iFEppV)
GO(fseek, iFpli)
GOW(fseeko, iFpli)
GO(__fseeko64, iFpIi)
GOW(fseeko64, iFpIi)
GO(__fsetlocking, iFpi)
GO(fsetpos, iFpp)
GO(fsetpos64, iFpp)
//GO(fsetxattr, 
//GOW(fstatfs, 
GOW(fstatfs64, iFip)
//GOW(fstatvfs, 
//GOW(fstatvfs64, 
//GO(fsync, 
GOW(ftell, lFp)
GOW(ftello, lFp)
GO(__ftello64, IFp)
GOW(ftello64, IFp)
//GO(ftime, 
GO(ftok, iFpi)
GOW(ftruncate, iFil)
GOW(ftruncate64, iFiI)
//GOW(ftrylockfile, 
//GOW(fts64_children, 
//GOW(fts64_close, 
//GOW(fts64_open, 
//GOW(fts64_read, 
//GOW(fts64_set, 
GO(fts_children, pFpi)
GO(fts_close, iFp)
GO(fts_open, pFEpip)
GO(fts_read, pFp)
GO(fts_set, iFppi)
GOM(ftw, iFEppi)
GOM(ftw64, iFEppi)
//GOW(funlockfile, 
//GO(futimens, 
//GOW(futimes, 
//GO(futimesat, 
//GO(fwide, 
//GOW(fwprintf, 
//GO(__fwprintf_chk, 
//GO(__fwritable, 
GOW(fwrite, LFpLLp)
//GO(fwrite_unlocked, 
//GO(__fwriting, 
//GO(fwscanf, 
GOM(__fxstat, iFEiip)
GOM(__fxstat64, iFEiip)
GOM(__fxstatat, iFEiippi)
GOM(__fxstatat64, iFEiippi)
//GO(__gai_sigqueue, 
GO(gai_strerror, pFi)
//GO(__gconv_get_alias_db, 
//GO(__gconv_get_cache, 
//GO(__gconv_get_modules_db, 
//GO(__gconv_transliterate, 
//GO(gcvt, 
GO(getaddrinfo, iFpppp)
//GO(getaliasbyname, 
//GO(getaliasbyname_r, 
//GO(getaliasent, 
//GO(getaliasent_r, 
//GO(__getauxval, 
//GOW(getauxval, 
//GOW(get_avphys_pages, 
GOW(getc, iFp)
GO(getchar, iFv)
GO(getchar_unlocked, iFv)
GOM(getcontext, iFEp)   //Weak
GOW(getc_unlocked, iFp)
GO(get_current_dir_name, pFv)
GOW(getcwd, pFpL)
GO(__getcwd_chk, pFpLL)
GO(getdate, pFp)
//DATAB(getdate_err, 
//GOW(getdate_r, 
GOW(__getdelim, iFppip)
GOW(getdelim, iFppip)
//GOW(getdirentries, 
//GO(getdirentries64, 
//GO(getdomainname, 
//GO(__getdomainname_chk, 
GOW(getdtablesize, iFv)
GOW(getegid, iFv)
GO(getentropy, iFpL)
GO(getenv, pFp)
GOW(geteuid, pFv)
//GO(getfsent, 
//GO(getfsfile, 
//GO(getfsspec, 
GOW(getgid, iFv)
GO(getgrent, pFv)
//GO(getgrent_r, 
//GO(getgrgid, 
//GO(getgrgid_r, 
//GO(getgrnam, 
//GO(getgrnam_r, 
//GO(getgrouplist, 
//GOW(getgroups, 
//GO(__getgroups_chk, 
//GO(gethostbyaddr, 
//GO(gethostbyaddr_r, 
//GO(gethostbyname, 
//GO(gethostbyname2, 
//GO(gethostbyname2_r, 
//GO(gethostbyname_r, 
//GO(gethostent, 
//GO(gethostent_r, 
//GO(gethostid, 
GOW(gethostname, iFpL)
//GO(__gethostname_chk, 
GOW(getifaddrs, iFp)
//GO(getipv4sourcefilter, 
//GOW(getitimer, 
//GO(get_kernel_syms, 
//GOW(getline, 
//GO(getloadavg, 
//GO(getlogin, 
//GOW(getlogin_r, 
//GO(__getlogin_r_chk, 
//GO(getmntent, 
//GO(__getmntent_r, 
//GOW(getmntent_r, 
//GO(getmsg, 
//GO(get_myaddress, 
//GO(getnameinfo, 
//GO(getnetbyaddr, 
//GO(getnetbyaddr_r, 
//GO(getnetbyname, 
//GO(getnetbyname_r, 
//GO(getnetent, 
//GO(getnetent_r, 
//GO(getnetgrent, 
//GOW(getnetgrent_r, 
//GO(getnetname, 
GOW(get_nprocs, iFv)
GOW(get_nprocs_conf, iFv)
GO(getopt, iFipp)
//GO(getopt_long, 
//GO(getopt_long_only, 
GO(__getpagesize, iFv)
GOW(getpagesize, iFv)
//GO(getpass, 
//GOW(getpeername, 
//GO(__getpgid, 
//GOW(getpgid, 
//GO(getpgrp, 
//GOW(get_phys_pages, 
GO(__getpid, uFv)
GO(getpid, uFv)
//GO(getpmsg, 
//GOW(getppid, 
GOW(getpriority, iFii)
GO(getprotobyname, pFp)
//GO(getprotobyname_r, 
GO(getprotobynumber, pFp)
//GO(getprotobynumber_r, 
GO(getprotoent, pFv)
//GO(getprotoent_r, 
//GOW(getpt, 
//GO(getpublickey, 
//GOW(getpw, 
//GO(getpwent, 
//GO(getpwent_r, 
GO(getpwnam, pFp)
GO(getpwnam_r, iFpppLp)
GO(getpwuid, pFu)
GO(getpwuid_r, iFuppLp)
//GO(getrandom, 
//GO(getresgid, 
//GO(getresuid, 
GO(__getrlimit, iFip)
GOW(getrlimit, iFip)
GOW(getrlimit64, iFip)
//GO(getrpcbyname, 
//GO(getrpcbyname_r, 
//GO(getrpcbynumber, 
//GO(getrpcbynumber_r, 
//GO(getrpcent, 
//GO(getrpcent_r, 
//GO(getrpcport, 
GOW(getrusage, iFip)
//GOW(gets, 
//GO(__gets_chk, 
//GO(getsecretkey, 
//GO(getservbyname, 
//GO(getservbyname_r, 
//GO(getservbyport, 
//GO(getservbyport_r, 
//GO(getservent, 
//GO(getservent_r, 
//GO(getsgent, 
//GO(getsgent_r, 
//GO(getsgnam, 
//GO(getsgnam_r, 
//GO(getsid, 
GOW(getsockname, iFipp)
GOW(getsockopt, iFiiipp)
//GO(getsourcefilter, 
//GO(getspent, 
//GO(getspent_r, 
//GO(getspnam, 
//GO(getspnam_r, 
//GO(getsubopt, 
GOW(gettext, pFp)
GO(__gettimeofday, iFpp)
GO(gettimeofday, iFpp)
//GOW(getttyent, 
//GOW(getttynam, 
GOW(getuid, uFv)
//GO(getusershell, 
//GOW(getutent, 
//GOW(getutent_r, 
//GOW(getutid, 
//GOW(getutid_r, 
//GOW(getutline, 
//GOW(getutline_r, 
//GO(getutmp, 
//GO(getutmpx, 
//GO(getutxent, 
//GO(getutxid, 
//GO(getutxline, 
//GO(getw, 
GOW(getwc, uFp)
//GO(getwchar, 
//GO(getwchar_unlocked, 
//GOW(getwc_unlocked, 
//GO(getwd, 
//GO(__getwd_chk, 
//GO(getxattr, 
//GO(glob, 
GO(glob64, iFEpipp)
//GO(globfree, 
GOW(globfree64, vFp)
//GOW(glob_pattern_p, 
GO(gmtime, pFp)
GO(__gmtime_r, pFpp)
GOW(gmtime_r, pFpp)
//GOW(gnu_dev_major, 
//GOW(gnu_dev_makedev, 
//GOW(gnu_dev_minor, 
//GOW(gnu_get_libc_release, 
//GOW(gnu_get_libc_version, 
//GO(grantpt, 
//GOW(group_member, 
//GOW(gsignal, 
//GO(gtty, 
//GOW(hasmntopt, 
//GO(hcreate, 
//GOW(hcreate_r, 
//GOW(hdestroy, 
//GOW(hdestroy_r, 
//DATA(h_errlist, 
//DATAB(__h_errno, 
//GO(__h_errno_location, 
//GO(herror, 
//GO(host2netname, 
//GO(hsearch, 
//GOW(hsearch_r, 
GO(hstrerror, pFi)
GO(htonl, uFu)
GO(htons, uFu)
GO(iconv, LFLpppp)
GO(iconv_close, iFL)
GO(iconv_open, LFpp)
//GO(__idna_from_dns_encoding, 
//GO(__idna_to_dns_encoding, 
//GOW(if_freenameindex, 
//GOW(if_indextoname, 
//GOW(if_nameindex, 
//GOW(if_nametoindex, 
//GOW(imaxabs, 
//GOW(imaxdiv, 
DATA(in6addr_any, 16)  // type V
DATA(in6addr_loopback, 16)  //type V
//GOI(index, 
//GO(inet6_opt_append, 
//GO(inet6_opt_find, 
//GO(inet6_opt_finish, 
//GO(inet6_opt_get_val, 
//GO(inet6_opt_init, 
//GO(inet6_option_alloc, 
//GO(inet6_option_append, 
//GO(inet6_option_find, 
//GO(inet6_option_init, 
//GO(inet6_option_next, 
//GO(inet6_option_space, 
//GO(inet6_opt_next, 
//GO(inet6_opt_set_val, 
//GO(inet6_rth_add, 
//GO(inet6_rth_getaddr, 
//GO(inet6_rth_init, 
//GO(inet6_rth_reverse, 
//GO(inet6_rth_segments, 
//GO(inet6_rth_space, 
//GO(__inet6_scopeid_pton, 
GOW(inet_addr, uFp)
GOW(inet_aton, iFpp)
//GO(__inet_aton_exact, 
//GO(inet_lnaof, 
//GOW(inet_makeaddr, 
//GO(inet_netof, 
GO(inet_network, iFp)
//GO(inet_nsap_addr, 
GO(inet_nsap_ntoa, pFipp)
GO(inet_ntoa, pFu)
GO(inet_ntop, iFippu)
GOW(inet_pton, iFipp)
//GO(__inet_pton_length, 
//GO(initgroups, 
//GO(init_module, 
GOW(initstate, pFupL)
//GOW(initstate_r, 
//GO(innetgr, 
GO(inotify_add_watch, iFipu)
GO(inotify_init, iFv)
GO(inotify_init1, iFi)
GO(inotify_rm_watch, iFii)
//GO(insque, 
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
//GO(ioperm, 
//GO(iopl, 
//GO(_IO_popen, 
//GO(_IO_printf, 
//GO(_IO_proc_close, 
//GO(_IO_proc_open, 
//GO(_IO_putc, 
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
//GO(iruserok, 
//GO(iruserok_af, 
GO(isalnum, iFi)
//GO(__isalnum_l, 
//GOW(isalnum_l, 
GO(isalpha, iFi)
//GO(__isalpha_l, 
GOW(isalpha_l, iFip)
GO(isascii, iFi)
//GOW(__isascii_l, 
//GO(isastream, 
GOW(isatty, iFi)
GO(isblank, iFi)
//GO(__isblank_l, 
//GOW(isblank_l, 
GO(iscntrl, iFi)
//GO(__iscntrl_l, 
//GOW(iscntrl_l, 
//GO(__isctype, 
//GOW(isctype, 
GO(isdigit, iFi)
//GO(__isdigit_l, 
//GOW(isdigit_l, 
//GO(isfdtype, 
GO(isgraph, iFi)
//GO(__isgraph_l, 
//GOW(isgraph_l, 
GO(__isinf, iFd)
GOW(isinf, iFd)
GO(__isinff, iFf)
GOW(isinff, iFf)
//GO(__isinfl, 
//GOW(isinfl, 
GO(islower, iFi)
//GO(__islower_l, 
//GOW(islower_l, 
GO(__isnan, iFd)
GOW(isnan, iFd)
GO(__isnanf, iFf)
GOW(isnanf, iFf)
//GO(__isnanl, 
//GOW(isnanl, 
GOM(__isoc99_fscanf, iFEppV)
//GO(__isoc99_fwscanf, 
//GO(__isoc99_scanf, 
GOM(__isoc99_sscanf, iFEppV)
//GO(__isoc99_swscanf, 
//GO(__isoc99_vfscanf, 
//GO(__isoc99_vfwscanf, 
//GO(__isoc99_vscanf, 
GO(__isoc99_vsscanf, iFEppp)
//GO(__isoc99_vswscanf, 
//GO(__isoc99_vwscanf, 
//GO(__isoc99_wscanf, 
GO(isprint, iFi)
//GO(__isprint_l, 
//GOW(isprint_l, 
GO(ispunct, iFi)
//GO(__ispunct_l, 
//GOW(ispunct_l, 
GO(isspace, iFi)
//GO(__isspace_l, 
//GOW(isspace_l, 
GO(isupper, iFi)
//GO(__isupper_l, 
//GOW(isupper_l, 
GOW(iswalnum, iFi)
//GO(__iswalnum_l, 
GOW(iswalnum_l, iFip)
GOW(iswalpha, iFi)
//GO(__iswalpha_l, 
GOW(iswalpha_l, iFip)
GOW(iswblank, iFi)
//GO(__iswblank_l, 
GOW(iswblank_l, iFip)
GOW(iswcntrl, iFi)
//GO(__iswcntrl_l, 
GOW(iswcntrl_l, iFip)
//GO(__iswctype, 
//GOW(iswctype, 
GO(__iswctype_l, iFuLL)
//GOW(iswctype_l, 
GOW(iswdigit, iFi)
//GO(__iswdigit_l, 
GOW(iswdigit_l, iFip)
GOW(iswgraph, iFi)
//GO(__iswgraph_l, 
GOW(iswgraph_l, iFip)
GOW(iswlower, iFi)
//GO(__iswlower_l, 
GOW(iswlower_l, iFip)
GOW(iswprint, iFi)
//GO(__iswprint_l, 
GOW(iswprint_l, iFip)
GOW(iswpunct, iFi)
//GO(__iswpunct_l, 
GOW(iswpunct_l, iFip)
GOW(iswspace, iFi)
//GO(__iswspace_l, 
GOW(iswspace_l, iFip)
GOW(iswupper, iFi)
//GO(__iswupper_l, 
GOW(iswupper_l, iFip)
GOW(iswxdigit, iFi)
//GO(__iswxdigit_l, 
GOW(iswxdigit_l, iFip)
GO(isxdigit, iFi)
//GO(__isxdigit_l, 
GOW(isxdigit_l, iFip)
//GO(__ivaliduser, 
GO(jrand48, iFp)
//GOW(jrand48_r, 
//GO(key_decryptsession, 
//GO(key_decryptsession_pk, 
//DATAB(__key_decryptsession_pk_LOCAL, 
//GO(key_encryptsession, 
//GO(key_encryptsession_pk, 
//DATAB(__key_encryptsession_pk_LOCAL, 
//GO(key_gendes, 
//DATAB(__key_gendes_LOCAL, 
//GO(key_get_conv, 
//GO(key_secretkey_is_set, 
//GO(key_setnet, 
//GO(key_setsecret, 
GOW(kill, iFli)
GO(killpg, iFii)
//GO(klogctl, 
//GO(l64a, 
//GO(labs, 
//GO(lchmod, 
//GOW(lchown, 
//GOW(lckpwdf, 
//GO(lcong48, 
//GOW(lcong48_r, 
//GOW(ldexp, 
//GOW(ldexpf, 
//GOW(ldexpl, 
//GO(ldiv, 
GOM(lfind, pFEpppLp)
//GO(lgetxattr, 
//GO(__libc_alloca_cutoff, 
//GO(__libc_allocate_once_slow, 
//GO(__libc_allocate_rtsig, 
//GO(__libc_allocate_rtsig_private, 
//GO(__libc_alloc_buffer_alloc_array, 
//GO(__libc_alloc_buffer_allocate, 
//GO(__libc_alloc_buffer_copy_bytes, 
//GO(__libc_alloc_buffer_copy_string, 
//GO(__libc_alloc_buffer_create_failure, 
//GO(__libc_calloc, 
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
//GO(__libc_free, 
//GO(__libc_freeres, 
//GO(__libc_ifunc_impl_list, 
//GO(__libc_init_first, 
//GO(__libc_longjmp, 
//GO(__libc_mallinfo, 
//GO(__libc_malloc, 
//GO(__libc_mallopt, 
//GO(__libc_memalign, 
//GO(__libc_msgrcv, 
//GO(__libc_msgsnd, 
//GO(__libc_pread, 
//GO(__libc_pthread_init, 
//GO(__libc_pvalloc, 
//GO(__libc_pwrite, 
//GO(__libc_readline_unlocked, 
//GO(__libc_realloc, 
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
//GO(__libc_valloc, 
//GO(__libc_vfork, 
//GOW(link, 
//GO(linkat, 
//GOW(listen, 
//GO(listxattr, 
//GO(llabs, 
//GO(lldiv, 
//GO(llistxattr, 
//GO(llseek, 
//DATAB(loc1, 
//DATAB(loc2, 
GO(localeconv, pFv)
GO(localtime, pFp)
GOW(localtime_r, pFpp)
//GO(lockf, 
//GOW(lockf64, 
//DATAB(locs, 
GOM(_longjmp, vFEpi)    //Weak
GOM(longjmp, vFEpi) //weak
GOM(__longjmp_chk, vFEpi)
//GO(lrand48, 
//GO(lrand48_r, 
//GO(lremovexattr, 
GOM(lsearch, pFEpppLp)
GO(__lseek, IFiIi)
GOW(lseek, IFiIi)
GOW(lseek64, IFiIi)
//GO(lsetxattr, 
//GO(lutimes, 
GOM(__lxstat, iFEipp)
GOM(__lxstat64, iFEipp)
GO(__madvise, iFpLi)
GOW(madvise, iFpLi)
GOM(makecontext, iFEppiV)   //weak
//GOW(mallinfo, 
GOM(malloc, pFL) // need to wrap to clear allocated memory?
//GO(malloc_get_state, 
//GOW(malloc_info, 
//GO(malloc_set_state, 
//GOW(malloc_stats, 
GOW(malloc_trim, iFL)
GOW(malloc_usable_size, LFp)
//GOW(mallopt, 
//DATAB(mallwatch, 
GO(mblen, iFpL)
GO(__mbrlen, LFpLp)
GOW(mbrlen, LFpLp)
//GO(mbrtoc16, 
//GOW(mbrtoc32, 
GO(__mbrtowc, LFppLp)
GOW(mbrtowc, LFppLp)
GOW(mbsinit, iFp)
GOW(mbsnrtowcs, LFppLLp)
//GO(__mbsnrtowcs_chk, 
GOW(mbsrtowcs, LFppLp)
//GO(__mbsrtowcs_chk, 
GO(mbstowcs, LFppL)
//GO(__mbstowcs_chk, 
GO(mbtowc, iFppL)
//GO(mcheck, 
//GO(mcheck_check_all, 
//GO(mcheck_pedantic, 
//GO(_mcleanup, 
GO(_mcount, vFpp)
GOW(mcount, vFpp)
GOW(memalign, pFLL)
GOW(memccpy, pFppiL)
GO(memchr, pFpiL)
GO(memcmp, iFppL)
GO(memcpy, pFppL)
GO(__memcpy_chk, pFppuL)
//GO(memfd_create, 
//GO(memfrob, 
//GOW(memmem, 
GO(memmove, pFppL)
GO(__memmove_chk, pFppLL)
//GOI(__mempcpy, 
//GOI(mempcpy, 
//GOI(__mempcpy_chk, 
GOW(memrchr, pFpiL)
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
GO(mkostemp, iFpi)
GOW(mkostemp64, iFpi)
//GO(mkostemps, 
//GOW(mkostemps64, 
GO(mkstemp, iFp)
GOW(mkstemp64, iFp)
//GO(mkstemps, 
//GOW(mkstemps64, 
GO(__mktemp, pFp)
GOW(mktemp, pFp)
GO(mktime, LFp)
GO(mlock, iFpL)
//GO(mlock2, 
GO(mlockall, iFi)
//GO(__mmap, 
GOM(mmap, pFEpLiiiI)    //weak
GOM(mmap64, pFEpLiiiI)  //weak
//GOW(modf, 
//GOW(modff, 
//GOW(modfl, 
//GOW(modify_ldt, 
//GOW(moncontrol, 
//GO(__monstartup, 
//GOW(monstartup, 
//DATA(__morecore, 
//GOW(mount, 
//GO(mprobe, 
//GO(__mprotect, 
GOM(mprotect, iFEpLi) //weak
//GO(mrand48, 
//GO(mrand48_r, 
GOM(mremap, pFEpLLiN) //weal
//GO(msgctl, 
//GO(msgget, 
//GOW(msgrcv, 
//GOW(msgsnd, 
GO(msync, iFpLi)
//GO(mtrace, 
//GO(munlock, 
//GO(munlockall, 
//GO(__munmap, 
GOM(munmap, iFEpL)  //weak
//GO(muntrace, 
GO(name_to_handle_at, iFipppi)
GO(__nanosleep, iFpp)
GOW(nanosleep, iFpp)
//GO(__nanosleep_nocancel, 
//GO(__netlink_assert_response, 
//GO(netname2host, 
//GO(netname2user, 
GO(__newlocale, pFipp)
GOW(newlocale, pFipp)
//GO(nfsservctl, 
//GO(nftw, 
//GO(nftw, 
GOM(nftw64, iFEppii)
//GO(nftw64, 
//GOW(ngettext, 
//GO(nice, 
//DATAB(_nl_domain_bindings, 
GO(nl_langinfo, pFu)
GO(__nl_langinfo_l, pFup)
GOW(nl_langinfo_l, pFup)
//DATAB(_nl_msg_cat_cntr, 
//GO(nrand48, 
//GOW(nrand48_r, 
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
//GOW(ntohl, 
//GOW(ntohs, 
//GOW(ntp_adjtime, 
//GO(ntp_gettime, 
//GO(ntp_gettimex, 
//DATAB(_null_auth, 
//DATAB(_obstack, 
//GO(_obstack_allocated_p, 
//DATA(obstack_alloc_failed_handler, 
//GO(_obstack_begin, 
//GO(_obstack_begin_1, 
//DATA(obstack_exit_failure, 
//GO(_obstack_free, 
//GO(obstack_free, 
//GO(_obstack_memory_used, 
//GO(_obstack_newchunk, 
//GOW(obstack_printf, 
//GO(__obstack_printf_chk, 
//GOW(obstack_vprintf, 
//GO(__obstack_vprintf_chk, 
//GOW(on_exit, 
GOM(__open, iFEpOu) //Weak
GOM(open, iFEpOu)    //Weak
GO(__open_2, iFpO)
//GOW(__open64, 
GOM(open64, iFEpOu) //Weak
//GO(__open64_2, 
//GO(__open64_nocancel, 
//GOW(openat, 
//GO(__openat_2, 
//GOW(openat64, 
//GO(__openat64_2, 
//GO(open_by_handle_at, 
//GO(__open_catalog, 
GOW(opendir, pFp)
GO(openlog, vFpii)
//GOW(open_memstream, 
//GO(__open_nocancel, 
//GO(open_wmemstream, 
//DATAB(optarg, 
//DATA(opterr, 
//DATA(optind, 
//DATA(optopt, 
//GO(__overflow, 
//GO(parse_printf_format, 
//GO(passwd2des, 
GOW(pathconf, iFpi)
GOW(pause, iFv)
//GO(__pause_nocancel, 
GO(pclose, iFp)
GO(perror, vFp)
//GOW(personality, 
GO(__pipe, iFp)
GOW(pipe, iFp)
GOW(pipe2, iFpO)
//GO(pivot_root, 
//GO(pkey_alloc, 
//GO(pkey_free, 
//GO(pkey_get, 
//GO(pkey_mprotect, 
//GO(pkey_set, 
//GO(pmap_getmaps, 
//GO(pmap_getport, 
//GO(pmap_rmtcall, 
//GO(pmap_set, 
//GO(pmap_unset, 
GO(__poll, iFpLi)
GOW(poll, iFpLi)
//GO(__poll_chk, 
GO(popen, pFpp)
GO(posix_fadvise, iFilli)
//GOW(posix_fadvise64, 
//GO(posix_fallocate, 
//GO(posix_fallocate64, 
//GO(__posix_getopt, 
//GO(posix_madvise, 
GOW(posix_memalign, iFpLL)
//GOW(posix_openpt, 
//GO(posix_spawn, 
//GO(posix_spawn, 
//GO(posix_spawnattr_destroy, 
//GO(posix_spawnattr_getflags, 
//GO(posix_spawnattr_getpgroup, 
//GO(posix_spawnattr_getschedparam, 
//GO(posix_spawnattr_getschedpolicy, 
//GO(posix_spawnattr_getsigdefault, 
//GO(posix_spawnattr_getsigmask, 
//GO(posix_spawnattr_init, 
//GO(posix_spawnattr_setflags, 
//GO(posix_spawnattr_setpgroup, 
//GO(posix_spawnattr_setschedparam, 
//GO(posix_spawnattr_setschedpolicy, 
//GO(posix_spawnattr_setsigdefault, 
//GO(posix_spawnattr_setsigmask, 
//GO(posix_spawn_file_actions_addclose, 
//GO(posix_spawn_file_actions_adddup2, 
//GO(posix_spawn_file_actions_addopen, 
//GO(posix_spawn_file_actions_destroy, 
//GO(posix_spawn_file_actions_init, 
//GO(posix_spawnp, 
//GO(posix_spawnp, 
GO(ppoll, iFpLpp)
//GO(__ppoll_chk, 
GOW(prctl, iFiLLLL)
GOW(pread, lFipLl)
GOW(__pread64, lFipLI)
GOW(pread64, lFipLI)
//GO(__pread64_chk, 
//GO(__pread_chk, 
//GO(preadv, 
//GO(preadv2, 
GO(preadv64, lFipiI)
//GO(preadv64v2, 
GOM(printf, iFEpV)
GOM(__printf_chk, iFEipV)
//GO(__printf_fp, 
//GO(printf_size, 
//GO(printf_size_info, 
//GO(prlimit, 
//GOW(prlimit64, 
//GO(process_vm_readv, 
//GO(process_vm_writev, 
//GOW(profil, 
//GO(__profile_frequency, 
DATA(__progname, sizeof(void*))
DATA(__progname_full, sizeof(void))
//GOW(pselect, 
//GO(psiginfo, 
//GO(psignal, 
//GO(pthread_attr_destroy, 
//GO(pthread_attr_getdetachstate, 
//GO(pthread_attr_getinheritsched, 
//GO(pthread_attr_getschedparam, 
//GO(pthread_attr_getschedpolicy, 
//GO(pthread_attr_getscope, 
//GO(pthread_attr_init, 
//GO(pthread_attr_setdetachstate, 
//GO(pthread_attr_setinheritsched, 
//GO(pthread_attr_setschedparam, 
//GO(pthread_attr_setschedpolicy, 
//GO(pthread_attr_setscope, 
//GO(pthread_condattr_destroy, 
//GO(pthread_condattr_init, 
//GO(pthread_cond_broadcast, 
//GO(pthread_cond_broadcast, 
//GO(pthread_cond_destroy, 
//GO(pthread_cond_destroy, 
//GO(pthread_cond_init, 
//GO(pthread_cond_init, 
//GO(pthread_cond_signal, 
//GO(pthread_cond_signal, 
//GO(pthread_cond_timedwait, 
//GO(pthread_cond_timedwait, 
//GO(pthread_cond_wait, 
//GO(pthread_cond_wait, 
//GO(pthread_equal, 
//GO(pthread_exit, 
//GO(pthread_getschedparam, 
//GO(pthread_mutex_destroy, 
//GO(pthread_mutex_init, 
//GO(pthread_mutex_lock, 
//GO(pthread_mutex_unlock, 
//GO(pthread_self, 
//GO(pthread_setcancelstate, 
//GO(pthread_setcanceltype, 
//GO(pthread_setschedparam, 
//GO(ptrace, 
//GO(ptsname, 
//GOW(ptsname_r, 
//GO(__ptsname_r_chk, 
GOW(putc, iFip)
GO(putchar, iFi)
GO(putchar_unlocked, iFi)
GO(putc_unlocked, iFip)
GO(putenv, iFp)
//GO(putgrent, 
//GO(putmsg, 
//GO(putpmsg, 
//GO(putpwent, 
GOW(puts, iFp)
//GO(putsgent, 
//GO(putspent, 
GOW(pututline, pFp)
//GO(pututxline, 
//GO(putw, 
GO(putwc, iFip)
//GO(putwchar, 
GO(putwchar_unlocked, iFi)
GO(putwc_unlocked, iFip)
GOW(pvalloc, pFL)
GOW(pwrite, lFipLl)
//GOW(__pwrite64, 
GOW(pwrite64, lFipLI)
//GO(pwritev, 
//GO(pwritev2, 
GO(pwritev64, lFipiI)
//GO(pwritev64v2, 
//GO(qecvt, 
//GOW(qecvt_r, 
//GO(qfcvt, 
//GOW(qfcvt_r, 
//GO(qgcvt, 
GOM(qsort, vFEpLLp)
GOM(qsort_r, vFEpLLpp)  //Weak
//GO(query_module, 
//GO(quick_exit, 
//GO(quick_exit, 
//GO(quotactl, 
//GO(raise, 
GO(rand, iFv)
GOW(random, lFv)
//GOW(random_r, 
GO(rand_r, iFp)
GO(__rawmemchr, pFpi)
GO(rawmemchr, pFpi)
//GO(rcmd, 
//GO(rcmd_af, 
//DATAB(__rcmd_errstr, 
GOW(__read, lFipL)
GO(read, lFipL)
//GOW(readahead, 
//GO(__read_chk, 
GOW(readdir, pFp) // struct direct is 280 byts on x86_64 and ARM64
GOW(readdir64, pFp)
GOW(readdir64_r, iFppp)
GOW(readdir_r, iFppp)
GOM(readlink, lFEppL)   //weak
GO(readlinkat, pFippL)
//GO(__readlinkat_chk, 
//GO(__readlink_chk, 
//GO(__read_nocancel, 
GOW(readv, lFipi)
GO(realloc, pFpL)
//GOW(reallocarray, 
GOM(realpath, pFEpp)
//GO(realpath, 
GO2(__realpath_chk, pFEppv, my_realpath)
//GO(reboot, 
//GOW(re_comp, 
//GOW(re_compile_fastmap, 
//GOW(re_compile_pattern, 
GOW(__recv, lFipLi)
GOW(recv, lFipLi)
//GO(__recv_chk, 
GOW(recvfrom, lFipLipp)
//GO(__recvfrom_chk, 
GO(recvmmsg, iFipuip)
GOW(recvmsg, lFipi)
//GOW(re_exec, 
//GOW(regcomp, 
//GOW(regerror, 
GO(regexec, iFppLpi)
//GOW(regfree, 
GOM(__register_atfork, iFEpppp)
//GOW(register_printf_function, 
//GOW(register_printf_modifier, 
//GOW(register_printf_specifier, 
//GOW(register_printf_type, 
//GO(registerrpc, 
//GOW(remap_file_pages, 
//GOW(re_match, 
//GOW(re_match_2, 
//DATA(re_max_failures, 
GO(remove, iFp)
//GO(removexattr, 
//GO(remque, 
GO(rename, iFpp)
//GOW(renameat, 
//GOW(renameat2, 
//DATAB(_res, 
//GOW(re_search, 
//GOW(re_search_2, 
//GOW(re_set_registers, 
//GOW(re_set_syntax, 
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
//GO(__res_state, 
//DATAB(re_syntax_options, 
//GOW(revoke, 
GO(rewind, vFp)
GOW(rewinddir, vFp)
//GO(rexec, 
//GO(rexec_af, 
//DATAB(rexecoptions, 
//GOI(rindex, 
GOW(rmdir, iFp)
//DATAB(rpc_createerr, 
//GO(_rpc_dtablesize, 
//GO(__rpc_thread_createerr, 
//GO(__rpc_thread_svc_fdset, 
//GO(__rpc_thread_svc_max_pollfd, 
//GO(__rpc_thread_svc_pollfd, 
//GO(rpmatch, 
//GO(rresvport, 
//GO(rresvport_af, 
//GO(rtime, 
//GO(ruserok, 
//GO(ruserok_af, 
//GO(ruserpass, 
GO(__sbrk, pFl)
GOW(sbrk, pFl)
//GOW(scalbn, 
//GOW(scalbnf, 
//GOW(scalbnl, 
//GOW(scandir, 
GOM(scandir64, iFEpppp) //weak
//GOW(scandirat, 
//GO(scandirat64, 
//GO(scanf, 
//GO(__sched_cpualloc, 
GO(__sched_cpucount, iFLp)
//GO(__sched_cpufree, 
GO(sched_getaffinity, iFLLp)
//GO(sched_getcpu, 
GO(__sched_getparam, iFLp)
GOW(sched_getparam, iFLp)
GO(__sched_get_priority_max, iFi)
GOW(sched_get_priority_max, iFi)
GO(__sched_get_priority_min, iFi)
GOW(sched_get_priority_min, iFi)
//GO(__sched_getscheduler, 
//GOW(sched_getscheduler, 
//GOW(sched_rr_get_interval, 
GO(sched_setaffinity, iFLLp)
//GOW(sched_setparam, 
//GO(__sched_setscheduler, 
//GOW(sched_setscheduler, 
GO(__sched_yield, iFv)
GOW(sched_yield, iFv)
GO2(__secure_getenv, pFp, getenv)   // not always present
GOW(secure_getenv, pFp)
//GO(seed48, 
//GOW(seed48_r, 
GO(seekdir, vFpi)
GO(__select, iFipppp)
GOW(select, iFipppp)
GO(semctl, iFiiiN)
GO(semget, iFiii)
GO(semop, iFipL)
GO(semtimedop, iFipLp)
GO(__send, lFipLi)
GOW(send, lFipLi)
GO(sendfile, lFiipL)
GOW(sendfile64, lFiipL)
GO(__sendmmsg, iFipui)
GOW(sendmmsg, iFipui)
GOW(sendmsg, lFipi)
GOW(sendto, lFipLipu)
//GO(setaliasent, 
GO(setbuf, vFpp)
GOW(setbuffer, vFppL)
//GOW(setcontext, 
//GO(setdomainname, 
//GO(setegid, 
GOW(setenv, iFppi)
//GO(_seterr_reply, 
//GO(seteuid, 
//GO(setfsent, 
//GO(setfsgid, 
//GO(setfsuid, 
//GOW(setgid, 
//GO(setgrent, 
//GO(setgroups, 
//GO(sethostent, 
//GO(sethostid, 
GO(sethostname, iFpL)
//GO(setipv4sourcefilter, 
//GOW(setitimer, 
GOM(_setjmp, iFEp)
GOM(setjmp, iFEp)
GO(setlinebuf, vFp)
GO(setlocale, pFip)
//GO(setlogin, 
//GO(setlogmask, 
//GO(__setmntent, 
//GOW(setmntent, 
//GO(setnetent, 
//GO(setnetgrent, 
//GO(setns, 
//GO(__setpgid, 
//GOW(setpgid, 
//GO(setpgrp, 
GOW(setpriority, iFiii)
GO(setprotoent, vFi)
//GO(setpwent, 
//GOW(setregid, 
//GOW(setresgid, 
//GOW(setresuid, 
//GOW(setreuid, 
GOW(setrlimit, iFip)
GOW(setrlimit64, iFip)
//GO(setrpcent, 
//GO(setservent, 
//GO(setsgent, 
//GOW(setsid, 
GOW(setsockopt, iFiiipu)
//GO(setsourcefilter, 
//GO(setspent, 
GOW(setstate, pFp)
//GOW(setstate_r, 
//GOW(settimeofday, 
//GOW(setttyent, 
GOW(setuid, iFu)
//GO(setusershell, 
//GOW(setutent, 
//GO(setutxent, 
GOW(setvbuf, iFppiL)
//GO(setxattr, 
//GO(sgetsgent, 
//GOW(sgetsgent_r, 
//GO(sgetspent, 
//GOW(sgetspent_r, 
//GO(shmat, 
//GO(shmctl, 
//GO(shmdt, 
//GO(shmget, 
GOW(shutdown, iFii)
GOM(__sigaction, iFEipp)  // Weak
GOM(sigaction, iFEipp)    // Weak
GO(__sigaddset, iFpi)
GO(sigaddset, iFpi)
GOM(sigaltstack, iFEpp) // Weak
//GO(sigandset, 
GOW(sigblock, iFi)
//GO(__sigdelset, 
GO(sigdelset, iFpi)
GO(sigemptyset, iFp)
GO(sigfillset, iFp)
GO(siggetmask, iFv)
//GO(sighold, 
//GO(sigignore, 
//GO(siginterrupt, 
//GO(sigisemptyset, 
//GO(__sigismember, 
GO(sigismember, iFpi)
//GOW(siglongjmp, 
GOM(signal, pFEip)  //Weak
//GO(signalfd, 
GO(__signbit, iFd)
GO(__signbitf, iFf)
//GO(__signbitl, 
//GO(sigorset, 
//GO(__sigpause, 
//GOW(sigpause, 
GO(sigpending, iFp)
GOW(sigprocmask, iFipp)
//GOW(sigqueue, 
//GO(sigrelse, 
//GOW(sigreturn, 
GOM(sigset, pFEip)
GOM(__sigsetjmp, iFEp)
//GOW(sigsetmask, 
//GO(sigstack, 
GO(__sigsuspend, iFp)
GOW(sigsuspend, iFp)
GO(__sigtimedwait, iFppp)
GOW(sigtimedwait, iFppp)
//GO(sigvec, 
GOW(sigwait, iFpp)
GOW(sigwaitinfo, iFpp)
GOW(sleep, uFu)
GOM(__snprintf, iFEpLpV)
GOM(snprintf, iFEpLpV)  //Weak
GOM(__snprintf_chk, iFEpLiLpV)  //Weak
//GO(sockatmark, 
GO(__socket, iFiii)
GOW(socket, iFiii)
GOW(socketpair, iFiiip)
//GO(splice, 
GOM(sprintf, iFEppV)
GOM(__sprintf_chk, iFEpilpV)
//GOW(sprofil, 
GOW(srand, vFu)
//GO(srand48, 
//GOW(srand48_r, 
GOW(srandom, vFu)
//GOW(srandom_r, 
GOM(sscanf, iFEppV)
//GOW(ssignal, 
//GO(sstk, 
GOM(__stack_chk_fail, vFE)
//GO(__statfs, 
//GOW(statfs, 
GOW(statfs64, iFpp)
GOW(statvfs, iFpp)
GOW(statvfs64, iFpp)
//GO(statx, 
DATA(stderr, sizeof(void*))
DATA(stdin, sizeof(void*))
DATA(stdout, sizeof(void*))
//GOW(step, 
//GO(stime, 
//GOI(__stpcpy, 
GO(stpcpy, pFpp)
GO(__stpcpy_chk, pFppL)
//GO(__stpcpy_small, 
GO(__stpncpy, pFppL)
GOW(stpncpy, pFppL)
GO(__stpncpy_chk, pFppLL)
GO(__strcasecmp, iFpp)
GO(strcasecmp, iFpp)
//GOI(__strcasecmp_l, 
//GOI(strcasecmp_l, 
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
GOW(strerror_r, pFipL)
//GO(strfmon, 
//GO(__strfmon_l, 
//GOW(strfmon_l, 
//GO(strfromd, 
//GO(strfromf, 
//GO(strfromf128, 
//GOW(strfromf32, 
//GOW(strfromf32x, 
//GOW(strfromf64, 
//GOW(strfromf64x, 
//GO(strfroml, 
//GO(strfry, 
GO(strftime, LFpLpp)
GO(__strftime_l, LFpLppL)
GOW(strftime_l, LFpLppL)
GO(strlen, LFp)
GO(strncasecmp, iFppL)
//GOI(__strncasecmp_l, 
//GOI(strncasecmp_l, 
GO(strncat, pFppL)
GO(__strncat_chk, pFppLL)
GO(strncmp, iFppL)
GO(strncpy, pFppL)
GO(__strncpy_chk, pFppLL)
GO(__strndup, pFpL)
GOW(strndup, pFpL)
GO(strnlen, LFpL)
GO(strpbrk, pFpp)
//GO(__strpbrk_c2, 
//GO(__strpbrk_c3, 
GO(strptime, pFppp)
//GOW(strptime_l, 
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
//GO(strtof128, 
//GO(__strtof128_internal, 
//GOW(strtof128_l, 
//GO(__strtof128_nan, 
//GOW(strtof32, 
//GOW(strtof32_l, 
//GOW(strtof32x, 
//GOW(strtof32x_l, 
//GOW(strtof64, 
//GOW(strtof64_l, 
//GOW(strtof64x, 
//GOW(strtof64x_l, 
//GO(__strtof_internal, 
GO(__strtof_l, fFppL)
GOW(strtof_l, fFppL)
//GO(__strtof_nan, 
GO(strtoimax, IFppi)
GO(strtok, pFpp)
GO(__strtok_r, pFppp)
GOW(strtok_r, pFppp)
//GO(__strtok_r_1c, 
GOW(strtol, lFppi)
//GO(strtold, 
//GO(__strtold_internal, 
GO(__strtold_l, dFppiL)
GOW(strtold_l, dFppiL)
//GO(__strtold_nan, 
GO(__strtol_internal, lFppii)
GO(__strtol_l, lFppi)
GOW(strtol_l, lFppi)
GOW(strtoll, lFppi)
//GO(__strtoll_internal, 
GOW(__strtoll_l, IFppip)
GOW(strtoll_l, IFppip)
//GOW(strtoq, 
GOW(strtoul, LFppi)
GO(__strtoul_internal, LFppii)
//GO(__strtoul_l, 
//GOW(strtoul_l, 
GOW(strtoull, LFppi)
//GO(__strtoull_internal, 
//GOW(__strtoull_l, 
//GOW(strtoull_l, 
//GO(strtoumax, 
//GOW(strtouq, 
//GO(__strverscmp, 
GOW(strverscmp, iFpp)
GO(strxfrm, LFppL)
GO(__strxfrm_l, LFppLL)
GOW(strxfrm_l, LFppLL)
//GO(stty, 
//DATAB(svcauthdes_stats, 
//GO(svcerr_auth, 
//GO(svcerr_decode, 
//GO(svcerr_noproc, 
//GO(svcerr_noprog, 
//GO(svcerr_progvers, 
//GO(svcerr_systemerr, 
//GO(svcerr_weakauth, 
//GO(svc_exit, 
//GO(svcfd_create, 
//DATAB(svc_fdset, 
//GO(svc_getreq, 
//GO(svc_getreq_common, 
//GO(svc_getreq_poll, 
//GO(svc_getreqset, 
//DATAB(svc_max_pollfd, 
//DATAB(svc_pollfd, 
//GO(svcraw_create, 
//GO(svc_register, 
//GO(svc_run, 
//GO(svc_sendreply, 
//GO(svctcp_create, 
//GO(svcudp_bufcreate, 
//GO(svcudp_create, 
//GO(svcudp_enablecache, 
//GO(svcunix_create, 
//GO(svcunixfd_create, 
//GO(svc_unregister, 
//GO(swab, 
GOM(swapcontext, iFEpp) //Weak
//GOW(swapoff, 
//GOW(swapon, 
GOM(swprintf, iFEpLpV)
GOM(__swprintf_chk, iFEpLiLpV)
GOM(swscanf, iFEppV)
GOW(symlink, iFpp)
GO(symlinkat, iFpip)
GO(sync, vFv)
//GO(sync_file_range, 
GO(syncfs, iFi)
GOM(syscall, LFEv)
GO(__sysconf, lFi)
GOW(sysconf, lFi)
//GO(__sysctl, 
//GOW(sysctl, 
DATA(_sys_errlist, sizeof(void*))
DATA(sys_errlist, sizeof(void*))
//GOW(sysinfo, 
GOM(syslog, vFEipV)
GOM(__syslog_chk, vFEiipV)
//DATA(sys_sigabbrev, 
//DATA(sys_sigabbrev, 
//DATA(_sys_siglist, 
//DATA(_sys_siglist, 
//DATA(sys_siglist, 
//DATA(sys_siglist, 
GOW(system, iFp)
GOM(__sysv_signal, pFEip)
GOM(sysv_signal, pFEip)  // Weak
GOW(tcdrain, iFi)
GO(tcflow, iFii)
GO(tcflush, iFii)
GOW(tcgetattr, iFip)
GO(tcgetpgrp, iFi)
//GO(tcgetsid, 
GO(tcsendbreak, iFii)
GO(tcsetattr, iFiip)
GO(tcsetpgrp, iFii)
//GO(__tdelete, 
//GOW(tdelete, 
//GOW(tdestroy, 
//GO(tee, 
GO(telldir, iFp)
GO(tempnam, pFpp)
GOW(textdomain, pFp)
//GO(__tfind, 
//GOW(tfind, 
//GO(thrd_current, 
//GO(thrd_equal, 
//GO(thrd_sleep, 
//GO(thrd_yield, 
GO(time, LFp)
GO(timegm, LFp)
//GOW(timelocal, 
GO(timerfd_create, iFii)
GO(timerfd_gettime, iFip)
GO(timerfd_settime, iFiipp)
//GOW(times, 
//GO(timespec_get, 
DATAB(__timezone, sizeof(void*))
DATAV(timezone, sizeof(void*))  //type V
GO(tmpfile, pFv)
GOW(tmpfile64, pFv)
GO(tmpnam, pFp)
GO(tmpnam_r, pFp)
GO(toascii, iFi)
//GOW(__toascii_l, 
GO(_tolower, iFi)
GO(tolower, iFi)
//GO(__tolower_l, 
GOW(tolower_l, iFip)
//GO(_toupper, 
GO(toupper, iFi)
//GO(__toupper_l, 
GOW(toupper_l, iFip)
//GO(__towctrans, 
//GOW(towctrans, 
//GO(__towctrans_l, 
//GOW(towctrans_l, 
GOW(towlower, iFi)
GO(__towlower_l, iFip)
GOW(towlower_l, iFip)
GOW(towupper, iFi)
GO(__towupper_l, iFip)
GOW(towupper_l, iFip)
//GO(tr_break, 
GOW(truncate, iFpl)
GOW(truncate64, iFpI)
//GO(__tsearch, 
//GOW(tsearch, 
//GO(ttyname, 
//GOW(ttyname_r, 
//GO(__ttyname_r_chk, 
//GO(ttyslot, 
//GO(__twalk, 
//GOW(twalk, 
DATA(__tzname, sizeof(void*))
DATA(tzname, sizeof(void*)) //type V
GOW(tzset, vFv)
//GO(ualarm, 
//GO(__uflow, 
//GOW(ulckpwdf, 
//GOW(ulimit, 
GOW(umask, uFu)
GOW(umount, iFp)
GOW(umount2, iFpi)
GOM(uname, iFp) //Weak
GO(__underflow, iFp)
GOW(ungetc, iFip)
GO(ungetwc, iFip)
GOW(unlink, iFp)
//GO(unlinkat, 
//GO(unlockpt, 
GOW(unsetenv, iFp)
//GO(unshare, 
//GOW(updwtmp, 
//GO(updwtmpx, 
//GO(uselib, 
GO(__uselocale, pFp)
GOW(uselocale, pFp)
//GO(user2netname, 
GO(usleep, iFu)
//GO(ustat, 
//GO(utime, 
//GO(utimensat, 
//GOW(utimes, 
//GOW(utmpname, 
//GO(utmpxname, 
GOW(valloc, pFL)
GOM(vasprintf, iFEppp)   //weak
GOM(__vasprintf_chk, iFEpipp)
//GOW(vdprintf, 
//GO(__vdprintf_chk, 
//GO(verr, 
//GO(verrx, 
//GOW(versionsort, 
//GOW(versionsort64, 
//GO(__vfork, 
//GOW(vfork, 
GOM(vfprintf, iFEppp)
GOM(__vfprintf_chk, iFEpvpp)
//GO(__vfscanf, 
//GOW(vfscanf, 
//GOW(vfwprintf, 
//GO(__vfwprintf_chk, 
//GOW(vfwscanf, 
//GO(vhangup, 
//GO(vlimit, 
//GO(vmsplice, 
GOM(vprintf, iFEpp)
GOM(__vprintf_chk, iFEvpp)
//GOW(vscanf, 
GOM(__vsnprintf, iFEpLpp)  // Weak
GOM(vsnprintf, iFEpLpp)    // Weak
GOM(__vsnprintf_chk, iFEpLvvpp)
GOM(vsprintf, iFEpppp) //Weak
GOM(__vsprintf_chk, iFEpvvppp)
GOM(__vsscanf, iFEppp)  //Weak
GOM(vsscanf, iFEppp)    //Weak
GOM(vswprintf, iFEpuppp)    // Weak
GOM(__vswprintf_chk, iFEpuvvppp)    // Weak
//GO(vswscanf, 
//GO(vsyslog, 
//GO(__vsyslog_chk, 
//GO(vtimes, 
//GO(vwarn, 
//GO(vwarnx, 
//GO(vwprintf, 
//GO(__vwprintf_chk, 
//GO(vwscanf, 
//GOW(__wait, 
//GOW(wait, 
//GOW(wait3, 
//GOW(wait4, 
//GOW(waitid, 
GO(__waitpid, lFlpi)
GOW(waitpid, lFlpi)
//GO(warn, 
//GO(warnx, 
//GOW(wcpcpy, 
//GO(__wcpcpy_chk, 
//GOW(wcpncpy, 
//GO(__wcpncpy_chk, 
GOW(wcrtomb, LFpup)
//GO(__wcrtomb_chk, 
//GOW(wcscasecmp, 
//GO(__wcscasecmp_l, 
//GOW(wcscasecmp_l, 
//GOW(wcscat, 
//GO(__wcscat_chk, 
//GOI(wcschr, 
//GOW(wcschrnul, 
GO(wcscmp, iFpp)
GOW(wcscoll, iFpp)
GO(__wcscoll_l, iFppp)
GOW(wcscoll_l, iFppp)
GO(wcscpy, pFpp)
GO(__wcscpy_chk, pFppL)
//GO(wcscspn, 
GO(wcsdup, pFp)
GO(wcsftime, LFpLpp)
GO(__wcsftime_l, LFpLppp)
GOW(wcsftime_l, LFpLppp)
GO(wcslen, LFp)
//GOW(wcsncasecmp, 
//GO(__wcsncasecmp_l, 
//GOW(wcsncasecmp_l, 
//GO(wcsncat, 
//GO(__wcsncat_chk, 
//GOI(wcsncmp, 
//GOW(wcsncpy, 
//GO(__wcsncpy_chk, 
GOW(wcsnrtombs, LFppLLp)
//GO(__wcsnrtombs_chk, 
//GOI(wcsnlen, 
//GO(wcspbrk, 
//GOI(wcsrchr, 
//GOW(wcsrtombs, 
//GO(__wcsrtombs_chk, 
//GO(wcsspn, 
//GO(wcsstr, 
//GO(wcstod, 
//GO(__wcstod_internal, 
//GO(__wcstod_l, 
//GOW(wcstod_l, 
//GO(wcstof, 
//GO(wcstof128, 
//GO(__wcstof128_internal, 
//GOW(wcstof128_l, 
//GOW(wcstof32, 
//GOW(wcstof32_l, 
//GOW(wcstof32x, 
//GOW(wcstof32x_l, 
//GOW(wcstof64, 
//GOW(wcstof64_l, 
//GOW(wcstof64x, 
//GOW(wcstof64x_l, 
//GO(__wcstof_internal, 
//GO(__wcstof_l, 
//GOW(wcstof_l, 
//GO(wcstoimax, 
//GO(wcstok, 
//GOW(wcstol, 
//GO(wcstold, 
//GO(__wcstold_internal, 
//GO(__wcstold_l, 
//GOW(wcstold_l, 
//GO(__wcstol_internal, 
//GO(__wcstol_l, 
//GOW(wcstol_l, 
//GOW(wcstoll, 
//GO(__wcstoll_internal, 
//GOW(__wcstoll_l, 
//GOW(wcstoll_l, 
GO(wcstombs, LFppL)
//GO(__wcstombs_chk, 
//GOW(wcstoq, 
//GOW(wcstoul, 
//GO(__wcstoul_internal, 
//GO(__wcstoul_l, 
//GOW(wcstoul_l, 
//GOW(wcstoull, 
//GO(__wcstoull_internal, 
//GOW(__wcstoull_l, 
//GOW(wcstoull_l, 
//GO(wcstoumax, 
//GOW(wcstouq, 
//GOW(wcswcs, 
//GO(wcswidth, 
GO(wcsxfrm, LFppL)
GO(__wcsxfrm_l, LFppLL)
GOW(wcsxfrm_l, LFppLL)
GO(wctob, iFi)
GO(wctomb, iFpi)
GO(__wctomb_chk, iFpuL)
//GOW(wctrans, 
//GO(__wctrans_l, 
//GOW(wctrans_l, 
GOW(wctype, LFp)
GOW(wctype_l, LFpp)
GO(__wctype_l, LFpp)
GO(wcwidth, iFu)
GO(wmemchr, pFpuL)
GO(wmemcmp, iFppL)
GOW(wmemcpy, pFppL)
GO(__wmemcpy_chk, pFppLL)
GOW(wmemmove, pFppL)
//GO(__wmemmove_chk, 
//GOW(wmempcpy, 
//GO(__wmempcpy_chk, 
GO(wmemset, pFpuL)
//GOI(__wmemset_chk, 
//GO(wordexp, 
//GO(wordfree, 
//GO(__woverflow, 
//GO(wprintf, 
//GO(__wprintf_chk, 
GOW(__write, lFipL)
GOW(write, lFipL)
//GO(__write_nocancel, 
GOW(writev, lFipi)
//GO(wscanf, 
//GO(__wuflow, 
//GO(__wunderflow, 
//GO(xdecrypt, 
//GO(xdr_accepted_reply, 
//GO(xdr_array, 
//GO(xdr_authdes_cred, 
//GO(xdr_authdes_verf, 
//GO(xdr_authunix_parms, 
//GO(xdr_bool, 
//GO(xdr_bytes, 
//GO(xdr_callhdr, 
//GO(xdr_callmsg, 
//GO(xdr_char, 
//GO(xdr_cryptkeyarg, 
//GO(xdr_cryptkeyarg2, 
//GO(xdr_cryptkeyres, 
//GO(xdr_des_block, 
//GO(xdr_double, 
//GO(xdr_enum, 
//GO(xdr_float, 
//GO(xdr_free, 
//GO(xdr_getcredres, 
//GO(xdr_hyper, 
//GO(xdr_int, 
//GO(xdr_int16_t, 
//GO(xdr_int32_t, 
//GO(xdr_int64_t, 
//GO(xdr_int8_t, 
//GO(xdr_keybuf, 
//GO(xdr_key_netstarg, 
//GO(xdr_key_netstres, 
//GO(xdr_keystatus, 
//GO(xdr_long, 
//GO(xdr_longlong_t, 
//GO(xdrmem_create, 
//GO(xdr_netnamestr, 
//GO(xdr_netobj, 
//GO(xdr_opaque, 
//GO(xdr_opaque_auth, 
//GO(xdr_pmap, 
//GO(xdr_pmaplist, 
//GO(xdr_pointer, 
//GO(xdr_quad_t, 
//GO(xdrrec_create, 
//GO(xdrrec_endofrecord, 
//GO(xdrrec_eof, 
//GO(xdrrec_skiprecord, 
//GO(xdr_reference, 
//GO(xdr_rejected_reply, 
//GO(xdr_replymsg, 
//GO(xdr_rmtcall_args, 
//GO(xdr_rmtcallres, 
//GO(xdr_short, 
//GO(xdr_sizeof, 
//GO(xdrstdio_create, 
//GO(xdr_string, 
//GO(xdr_u_char, 
//GO(xdr_u_hyper, 
//GO(xdr_u_int, 
//GO(xdr_uint16_t, 
//GO(xdr_uint32_t, 
//GO(xdr_uint64_t, 
//GO(xdr_uint8_t, 
//GO(xdr_u_long, 
//GO(xdr_u_longlong_t, 
//GO(xdr_union, 
//GO(xdr_unixcred, 
//GO(xdr_u_quad_t, 
//GO(xdr_u_short, 
//GO(xdr_vector, 
//GO(xdr_void, 
//GO(xdr_wrapstring, 
//GO(xencrypt, 
//GO(__xmknod, 
//GO(__xmknodat, 
GO(__xpg_basename, pFp)
//GOW(__xpg_sigpause, 
GO(__xpg_strerror_r, pFipL)
//GO(xprt_register, 
//GO(xprt_unregister, 
GOM(__xstat, iFEipp)
GOM(__xstat64, iFEipp)

GOM(_Jv_RegisterClasses, vFv)   // dummy

GOM(_ITM_addUserCommitAction, vFEpup)
GOM(_ITM_registerTMCloneTable, vFEpu)
GOM(_ITM_deregisterTMCloneTable, vFEp)

GOM(__register_frame_info, vFpp)    // faked function
GOM(__deregister_frame_info, pFp)
