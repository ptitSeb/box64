#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GO(a64l, lFp)
GO(abort, vFv)
//DATAB(__abort_msg, 
GO(abs, iFi)
GO(accept, iFipp)
GO(accept4, iFippi)
GOW(access, iFpi)
GO(acct, iFp)
GOW(addmntent, iFSp)
GOW(addseverity, iFip)
GOW(adjtime, iFpp)
GO(__adjtimex, iFp)
GOW(adjtimex, iFp)
//GOW(advance, 
GO(alarm, uFu)
GO(aligned_alloc, pFLL)
GOW(alphasort, iFpp)
GOW(alphasort64, iFpp)
//GO(__arch_prctl, 
//GOW(arch_prctl, 
//DATA(argp_err_exit_status, 4)
//GOWM(argp_error, vFppV)
//GOWM(argp_failure, vFpiipV)
//GOWM(argp_help, vFpSup)
//GOWM(argp_parse, iFpipupp)
//DATAB(argp_program_bug_address, 8)
//DATAB(argp_program_version, 8)
//DATAM(argp_program_version_hook, 8)
//GOWM(argp_state_help, vFpSu)
//GOWM(argp_usage, vFp)
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
//GOM(authdes_create, pFpupp)
//GO(authdes_getucred, "iF!pppp")
//GOM(authdes_pk_create, pFppupp)
//GOM(_authenticate, uFpp)
//GOM(authnone_create, pFv)
//GOM(authunix_create, pFpuuip)
//GOM(authunix_create_default, pFv)
//GO(__backtrace, 
GOWM(backtrace, iFEpi)
//GO(__backtrace_symbols, 
GOWM(backtrace_symbols, pFEpi)
//GO(__backtrace_symbols_fd, 
GOWM(backtrace_symbols_fd, vFEpii)
GOW(basename, pFp)
GO(bcmp, iFppL)
GO(bcopy, vFppL)
//GO(bdflush, 
GOW(bind, iFipu)
GO(bindresvport, iFip)
GOW(bindtextdomain, pFpp)
GOW(bind_textdomain_codeset, pFpp)
GOW(brk, iFp)
//GO(__bsd_getpgrp, 
GO2(bsd_signal, pFEip, my_signal)
GOM(bsearch, pFEppLLp)
GOW(btowc, uFi)
GO(__bzero, vFpL)
GO(bzero, vFpL)
GO(c16rtomb, LFpWp)
GOW(c32rtomb, LFpup)
GOW(calloc, pFLL)
//GOM(callrpc, iFpiiipppp)
//GO(__call_tls_dtors, vFv)
GOW(canonicalize_file_name, pFp)
GO(capget, iFpp)
GO(capset, iFpp)
GO(catclose, iFp)
GO(catgets, pFpiip)
GO(catopen, pFpi)
#ifdef STATICBUILD
//GO(cbc_crypt, iFppuup)
#else
GO(cbc_crypt, iFppuup)
#endif
GO(cfgetispeed, uFp)
GO(cfgetospeed, uFp)
GO(cfmakeraw, vFp)
GO(cfree, vFp)
GO(cfsetispeed, iFpu)
GO(cfsetospeed, iFpu)
GO(cfsetspeed, iFpu)
GOW(chdir, iFp)
DATA(__check_rhosts_file, 8)
//GO(chflags, 
//GO(__chk_fail, 
GOW(chmod, iFpu)
GOW(chown, iFpuu)
GO(chroot, iFp)
GOW(clearenv, iFv)
GO(clearerr, vFS)
GO(clearerr_unlocked, vFS)
#ifdef STATICBUILD
//GO(clnt_broadcast, 
//GO(clnt_create, "!FpLLp")
//GO(clnt_pcreateerror, vFp)
//GO(clnt_perrno, vFu)
//GO(clnt_perror, "vF!p")
//GO(clntraw_create, "!FLL")
//GO(clnt_spcreateerror, pFp)
//GO(clnt_sperrno, pFu)
//GO(clnt_sperror, "pF!p")
//GO(clnttcp_create, "!F!LLpuu")
//GO(clntudp_bufcreate, "!F!LL?puu")
//GO(clntudp_create, "!F!LL?p")
//GO(clntunix_create, "!F!LLpuu")
#else
//GOM(clnt_broadcast, uFLLLppppp)
//GOM(clnt_create, pFpuup)
GO(clnt_pcreateerror, vFp)
GO(clnt_perrno, vFu)
//GOM(clnt_perror, vFpp)
//GOM(clntraw_create, pFLL)
GO(clnt_spcreateerror, pFp)
GO(clnt_sperrno, pFu)
//GOM(clnt_sperror, pFpp)
//GOM(clnttcp_create, pFpLLpuu)
//GO(clntudp_bufcreate, "!F!LL?puu")
//GO(clntudp_create, "!F!LL?p")
//GOM(clntunix_create, pFpLLpuu)
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
GO(close_range, iFuui)
GO(__cmsg_nxthdr, pFpp)
GO(confstr, LFipL)
GO(__confstr_chk, LFipLL)
GOW(__connect, iFipu)
GOW(connect, iFipu)
GO(copy_file_range, lFipipLu)
//GO(__copy_grp, 
GOW(copysign, dFdd)
GOW(copysignf, fFff)
GOWD(copysignl, DFDD, copysign)
GOW(creat, iFpu)
GOW(creat64, iFpu)
//GO(create_module, 
GO(ctermid, pFp)
GO(ctime, pFp)
GO(ctime_r, pFpp)
#ifdef ANDROID
DATA(_ctype_, 8)
#else
//DATA(_ctype_, 8) //is it even used on Linux?
#endif
//DATA(__ctype32_b, 
//DATA(__ctype32_tolower, 
//DATA(__ctype32_toupper, 
#ifdef STATICBUILD
//DATA(__ctype_b, 8)
#else
DATA(__ctype_b, 8)
#endif
GO(__ctype_b_loc, pFv)
GO(__ctype_get_mb_cur_max, LFv)
//GO(__ctype_init, 
DATAM(__ctype_tolower, 8)
GO(__ctype_tolower_loc, pFv)
DATAM(__ctype_toupper, 8)
GO(__ctype_toupper_loc, pFv)
//DATAB(__curbrk, 
GO(cuserid, pFp)
GOM(__cxa_atexit, iFEppp)
//GO(__cxa_at_quick_exit, "/* at_quick_exit has signature iF@ -> */ iF@pp")
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
#ifdef STATICBUILD
//GO(delete_module, 
//GO(des_setparity, 
#else
GOM(delete_module, iFEpu)
GO(des_setparity, vFp)
#endif
GO(__dgettext, pFpp)
GOW(dgettext, pFpp)
GO(difftime, dFll)
GO(dirfd, iFp)
GO(dirname, pFp)
GO(div, UFii)
//GO(_dl_addr, 
//GO(_dl_catch_error, 
//GO(_dl_catch_exception, 
GOWM(dl_iterate_phdr, iFEpp)
//GO(_dl_mcount_wrapper, 
#ifdef STATICBUILD
//GO(_dl_mcount_wrapper_check, vFp)
#else
GO(_dl_mcount_wrapper_check, vFp)
#endif
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
#ifdef STATICBUILD
//GO(ecb_crypt, 
#else
GO(ecb_crypt, iFppuu)
#endif
GO(ecvt, pFdipp)
GOW(ecvt_r, iFdipppL)
GO(endaliasent, vFv)
GO(endfsent, vFv)
GO(endgrent, vFv)
GO(endhostent, vFv)
GO(__endmntent, iFp)
GOW(endmntent, iFS)
GO(endnetent, vFv)
GO(endnetgrent, vFv)
GO(endprotoent, vFv)
GO(endpwent, vFv)
GO(endrpcent, vFv)
GO(endservent, vFv)
#ifdef STATICBUILD
//GO(endsgent, 
#else
GO(endsgent, vFv)
#endif
GO(endspent, vFv)
GOW(endttyent, iFv)
GO(endusershell, vFv)
GOW(endutent, vFv)
GO(endutxent, vFv)
DATAM(environ, 8)
DATAM(__environ, 8)
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
GO(epoll_pwait2, iFipipp)
GO(epoll_wait, iFipii)
#else
GOM(epoll_ctl, iFEiiip)
GOM(epoll_pwait, iFEipiip)
GOM(epoll_pwait2, iFEipipp)
GOM(epoll_wait, iFEipii)
#endif
GO(erand48, dFp)
GOW(erand48_r, iFppp)
GOM(err, vFEipV)
//DATAB(errno, 
GO(__errno_location, pFv)
GOWM(error, vFiipV)
GOWM(error_at_line, vFiipupV)
//DATAB(error_message_count, 4)
//DATAB(error_one_per_line, 4)
//DATAM(error_print_progname, 8)
GOM(errx, vFEipV)
#ifdef STATICBUILD
//GO(ether_aton, pFp)
//GO(ether_aton_r, pFpp)
//GO(ether_hostton, iFpp)
//GO(ether_line, iFppp)
//GO(ether_ntoa, pFp)
//GO(ether_ntoa_r, pFpp)
//GO(ether_ntohost, iFpp)
#else
GO(ether_aton, pFp)
GO(ether_aton_r, pFpp)
GO(ether_hostton, iFpp)
GO(ether_line, iFppp)
GO(ether_ntoa, pFp)
GO(ether_ntoa_r, pFpp)
GO(ether_ntohost, iFpp)
#endif
GOW(euidaccess, iFpi)
GO(eventfd, iFui)
GO(eventfd_read, iFip)
GO(eventfd_write, iFiU)
GOM(execl, iFEpV) // First argument is also part of the variadic
GOM(execle, iFEpV) // First argument is also part of the variadic
GOM(execlp, iFEpV) // First argument is also part of the variadic
GOM(execv, iFEpp)
GOM(execve, iFEppp)
GOM(execvp, iFEpp)
GOWM(execvpe, iFEppp)
GOM(_exit, vFEi)
GOM(exit, vFEi)
GOW(_Exit, vFi)
GO(explicit_bzero, vFpL)
GO(__explicit_bzero_chk, vFpLL)
GO(faccessat, iFipii)
GOW(fallocate, iFiill)
GO(fallocate64, iFiiII)
GO(fanotify_init, iFuu)
GO(fanotify_mark, iFiuUip)
//GO(fattach, 
GO(__fbufsize, LFp)
GOW(fchdir, iFi)
//GO(fchflags, 
GOW(fchmod, iFiu)
GO(fchmodat, iFipui)
GOW(fchown, iFiuu)
GO(fchownat, iFipuui)
GO(fclose, iFS)
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
GO(fdopen, SFip)
GOW(fdopendir, pFi)
//GO(__fentry__, 
GOW(feof, iFS)
GOW(feof_unlocked, iFS)
GOW(ferror, iFS)
GOW(ferror_unlocked, iFS)
GO(fexecve, iFipp)
GOW(fflush, iFS)
GO(fflush_unlocked, iFS)
//GO(__ffs, 
GO(ffs, iFi)
GO(ffsl, iFl)
GO(ffsll, iFI)
GOW(fgetc, iFS)
GOW(fgetc_unlocked, iFS)
GO(fgetgrent, pFS)
GOW(fgetgrent_r, iFSppLp)
GO(fgetpos, iFSp)
GO(fgetpos64, iFSp)
GO(fgetpwent, pFS)
GOW(fgetpwent_r, iFSppLp)
GOW(fgets, pFpiS)
GO(__fgets_chk, pFpLip)
#ifdef STATICBUILD
//GO(fgetsgent, 
//GOW(fgetsgent_r, 
#else
GO(fgetsgent, pFS)
GOW(fgetsgent_r, iFSppLp)
#endif
GO(fgetspent, pFS)
GOW(fgetspent_r, iFSppLp)
GOW(fgets_unlocked, pFpiS)
//GO(__fgets_unlocked_chk, 
GOW(fgetwc, uFS)
GOW(fgetwc_unlocked, uFS)
GO(fgetws, pFpiS)
//GO(__fgetws_chk, 
GO(fgetws_unlocked, pFpiS)
//GO(__fgetws_unlocked_chk, 
GO(fgetxattr, lFippL)
GOW(fileno, iFS)
GOW(fileno_unlocked, iFS)
GO(__finite, iFd)
GOW(finite, iFd)
GO(__finitef, iFf)
GOW(finitef, iFf)
GOD(__finitel, iFD, __finite)
GOWD(finitel, iFD, finite)
GO(__flbf, iFp)
GO(flistxattr, lFipL)
GOW(flock, iFii)
GOW(flockfile, vFS)
GOW(_flushlbf, vFv)
GO(fmemopen, SFpLp)
GO(fmtmsg, iFlpippp)
GO(fnmatch, iFppi)
GOM(fopen, SFEpp)
GOWM(fopen64, SFEpp)
GOM(fopencookie, pFEppV)
//GO(__fork, 
GOWM(fork, iFEv)
//GO(__fortify_fail, 
GO(fpathconf, lFii)
GO(__fpending, LFp)
GOM(fprintf, iFESpV)
GOM(__fprintf_chk, iFEpipV)
//DATA(__fpu_control, 
GO(__fpurge, vFp)
GO(fputc, iFiS)
GO(fputc_unlocked, iFiS)
GOW(fputs, iFpS)
GOW(fputs_unlocked, iFpS)
GO(fputwc, uFiS)
GO(fputwc_unlocked, uFiS)
GO(fputws, iFpS)
GO(fputws_unlocked, iFpS)
GO(fread, LFpLLS)
GO(__freadable, iFp)
GO(__fread_chk, LFpLLLp)
GO(__freading, iFp)
GO(fread_unlocked, LFpLLS)
GO(__fread_unlocked_chk, LFpLLLp)
GO(free, vFp)
GO(freeaddrinfo, vFp)
GOW(freeifaddrs, vFp)
GO(__freelocale, vFp)
GOW(freelocale, vFp)
GO(fremovexattr, iFip)
GO(freopen, SFppS)
GO(freopen64, SFppS)
GOW(frexp, dFdp)
GOW(frexpf, fFfp)
GOWD(frexpl, DFDp, frexp)
GOM(fscanf, iFESpV)
GO(fseek, iFSli)
GO(fseeko, iFSli)
GO(__fseeko64, iFpli)
GO(fseeko64, iFSIi)
GO(__fsetlocking, iFpi)
GO(fsetpos, iFSp)
GO(fsetpos64, iFSp)
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
GO(ftell, lFS)
GO(ftello, lFS)
GO(__ftello64, lFp)
GO(ftello64, IFS)
GO(ftime, iFp)
GO(ftok, iFpi)
GO(ftruncate, iFil)
GO(ftruncate64, iFiI)
GOW(ftrylockfile, iFS)
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
GOW(funlockfile, vFS)
GO(futimens, iFip)
GOW(futimes, iFip)
GO(futimesat, iFipp)
GO(fwide, iFSi)
GOWM(fwprintf, iFESpV)
GOM(__fwprintf_chk, iFEpipV)
GO(__fwritable, iFp)
GO(fwrite, LFpLLS)
GO(fwrite_unlocked, LFpLLS)
GO(__fwriting, iFp)
//GOM(fwscanf, iFSpV)
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
//GOM(getaddrinfo_a, iFipip)
GO(getaliasbyname, pFp)
GO(getaliasbyname_r, iFpppLp)
GO(getaliasent, pFv)
GO(getaliasent_r, iFppLp)
//GO(__getauxval, 
GOWM(getauxval, LFEL)
GOW(get_avphys_pages, lFv)
GOW(getc, iFS)
GO(getchar, iFv)
GO(getchar_unlocked, iFv)
GOWM(getcontext, iFEp)
GOW(getcpu, iFpp)
GOW(getc_unlocked, iFS)
GO(get_current_dir_name, pFv)
GO(getcwd, pFpL)
GO(__getcwd_chk, pFpLL)
GO(getdate, pFp)
//DATAB(getdate_err, 4)
GOW(getdate_r, iFpp)
GOW(__getdelim, lFppiS)
GO(getdelim, lFppiS)
GO(getdents64, lFipL)
GO(getdirentries, lFipLp)
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
#ifdef STATICBUILD
//GO(__gethostname_chk, iFpLL)  // it's sometimes not defined, like in LA64 plateform, so lets no wrap it for now
#else
GO(__gethostname_chk, iFpLL)
#endif
GOW(getifaddrs, iFp)
GO(getipv4sourcefilter, iFiuuppp)
GOW(getitimer, iFup)
#ifdef STATICBUILD
//GO(get_kernel_syms, 
#else
//GO(get_kernel_syms, 
#endif
GO(getline, lFppS)
GO(getloadavg, iFpi)
GO(getlogin, pFv)
GOW(getlogin_r, iFpL)
//GO(__getlogin_r_chk, 
GO(getmntent, pFS)
GO(__getmntent_r, pFpppi)
GOW(getmntent_r, pFSppi)
#ifdef STATICBUILD
//GO(getmsg, 
//GO(get_myaddress, iFp)
#else
//GO(getmsg, 
GO(get_myaddress, iFp)
#endif
GO(getnameinfo, iFpupupui)
GO(getnetbyaddr, pFui)
GO(getnetbyaddr_r, iFuippLpp)
GO(getnetbyname, pFp)
GO(getnetbyname_r, iFpppLpp)
GO(getnetent, pFv)
GO(getnetent_r, iFppLpp)
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
//GO(getpmsg, 
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
GO(getrpcport, iFpiii)
#endif
GOW(getrusage, iFip)
//GOW(gets, 
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
GO(getservent_r, iFppLp)
#ifdef STATICBUILD
//GO(getsgent, 
//GO(getsgent_r, 
//GO(getsgnam, 
//GO(getsgnam_r, 
#else
GO(getsgent, pFv)
GO(getsgent_r, iFppLp)
GO(getsgnam, pFp)
GO(getsgnam_r, iFpppLp)
#endif
GO(getsid, iFi)
GOW(getsockname, iFipp)
GOW(getsockopt, iFiiipp)
GO(getsourcefilter, iFiupuppp)
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
GO(getw, iFS)
GOW(getwc, uFS)
GO(getwchar, uFv)
GO(getwchar_unlocked, uFv)
GOW(getwc_unlocked, uFS)
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
GOW(gnu_dev_major, uFU)
GOW(gnu_dev_makedev, UFuu)
GOW(gnu_dev_minor, uFU)
GOW(gnu_get_libc_release, pFv)
GOW(gnu_get_libc_version, pFv)
GO(grantpt, iFi)
GOW(group_member, iFu)
GOW(gsignal, iFi)
//GO(gtty, 
GOW(hasmntopt, pFpp)
GO(hcreate, iFL)
GOW(hcreate_r, iFLp)
GOW(hdestroy, vFv)
GOW(hdestroy_r, vFp)
//DATA(h_errlist, 
//DATAB(__h_errno, 
GO(__h_errno_location, pFv)
GO(herror, vFp)
#ifdef STATICBUILD
//GO(host2netname, 
#else
GO(host2netname, iFppp)
#endif
//GO(hsearch, "pF?u")
//GOW(hsearch_r, "iF?up!")
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
GOW(imaxabs, IFI)
GOW(imaxdiv, HFII)
DATAV(in6addr_any, 16)
DATAV(in6addr_loopback, 16)
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
GO(inet6_rth_add, iFpp)
GO(inet6_rth_getaddr, pFpi)
GO(inet6_rth_init, pFpuii)
GO(inet6_rth_reverse, iFpp)
GO(inet6_rth_segments, iFp)
GO(inet6_rth_space, uFii)
//GO(__inet6_scopeid_pton, 
GOW(inet_addr, uFp)
GOW(inet_aton, iFpp)
//GO(__inet_aton_exact, 
GO(inet_lnaof, uFu)
GOW(inet_makeaddr, UFuu)
GO(inet_netof, uFu)
GO(inet_network, uFp)
GO(inet_nsap_addr, uFppi)
GO(inet_nsap_ntoa, pFipp)
GO(inet_ntoa, pFu)
GO(inet_ntop, pFippu)
GOW(inet_pton, iFipp)
//GO(__inet_pton_length, 
GO(initgroups, iFpu)
GOM(init_module, iFEpLp)
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
DATA(_IO_file_jumps, 8)
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
DATA(_IO_list_all, 8)
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
GOW(isalnum_l, iFip)
GO(isalpha, iFi)
//GO(__isalpha_l, 
GOW(isalpha_l, iFip)
GO(isascii, iFi)
//GOW(__isascii_l, 
//GO(isastream, 
GOW(isatty, iFi)
GO(isblank, iFi)
//GO(__isblank_l, 
GOW(isblank_l, iFip)
GO(iscntrl, iFi)
//GO(__iscntrl_l, 
GOW(iscntrl_l, iFip)
//GO(__isctype, 
GOW(isctype, iFii)
GO(isdigit, iFi)
//GO(__isdigit_l, 
GOW(isdigit_l, iFip)
GO(isfdtype, iFii)
GO(isgraph, iFi)
//GO(__isgraph_l, 
GOW(isgraph_l, iFip)
GO(__isinf, iFd)
GOW(isinf, iFd)
GO(__isinff, iFf)
GOW(isinff, iFf)
GOD(__isinfl, iFD, __isinf)
GOWD(isinfl, iFD, isinf)
GO(islower, iFi)
//GO(__islower_l, 
GOW(islower_l, iFip)
GO(__isnan, iFd)
GOW(isnan, iFd)
GO(__isnanf, iFf)
GOW(isnanf, iFf)
GOD(__isnanl, iFD, __isnan)
GOWD(isnanl, iFD, isnan)
GO2(__isoc23_fscanf, iFESpV, my___isoc99_fscanf)
GO2(__isoc23_sscanf, iFEppV, my___isoc99_sscanf)
GO2(__isoc23_strtol, lFppi, strtol)
GO2(__isoc23_strtoll, IFppi, strtoll)
GO2(__isoc23_strtoul, LFppi, strtoul)
GO2(__isoc23_strtoull, UFppi, strtoull)
GO2(__isoc23_wcstol, lFppi, wcstol)
GOM(__isoc99_fscanf, iFEppV)
GOM(__isoc23_swscanf, iFEppV)
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
GOW(isprint_l, iFip)
GO(ispunct, iFi)
//GO(__ispunct_l, 
GOW(ispunct_l, iFip)
GO(isspace, iFi)
//GO(__isspace_l, 
GOW(isspace_l, iFip)
GO(isupper, iFi)
//GO(__isupper_l, 
GOW(isupper_l, iFip)
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
#ifdef STATICBUILD
//GO(key_decryptsession, iFpp)
//GO(key_decryptsession_pk, "iFp!!")
#else
GO(key_decryptsession, iFpp)
//GO(key_decryptsession_pk, "iFp!!")
#endif
//DATAB(__key_decryptsession_pk_LOCAL, 
#ifdef STATICBUILD
//GO(key_encryptsession, iFpp)
//GO(key_encryptsession_pk, "iFp!!")
#else
GO(key_encryptsession, iFpp)
//GO(key_encryptsession_pk, "iFp!!")
#endif
//DATAB(__key_encryptsession_pk_LOCAL, 
#ifdef STATICBUILD
//GO(key_gendes, iFp)
#else
GO(key_gendes, iFp)
#endif
//DATAB(__key_gendes_LOCAL, 
//GO(key_get_conv, "iFp!")
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
GOWD(ldexpl, DFDi, ldexp)
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
GOM(__libc_dlclose, iFEp)
GOM(__libc_dlopen_mode, pFEpi)
GOM(__libc_dlsym, pFEpp)
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
//GO(llseek, 
//DATAB(loc1, 
//DATAB(loc2, 
GO(localeconv, pFv)
GO(localtime, pFp)
GO2(localtime64, pFp, localtime)
GOW(localtime_r, pFpp)
GOW2(localtime64_r, pFpp, localtime_r)
GO(lockf, iFiil)
GO(lockf64, iFiiI)
//DATAB(locs, 
GOWM(_longjmp, vFEpi)
GOWM(longjmp, vFEpi)
GOM(__longjmp_chk, vFEpi)
GO(lrand48, lFv)
GO(lrand48_r, iFpp)
GO(lremovexattr, iFpp)
GOM(lsearch, pFEpppLp)
GO(__lseek, IFiIi)
GO(lseek, lFili)
GO(lseek64, IFiIi)
GO(lsetxattr, iFpppLi)
GOM(lstat, iFEpp)
GOM(lstat64, iFEpp)
GO(lutimes, iFpp)
GOM(__lxstat, iFEipp)
GOM(__lxstat64, iFEipp)
GO(__madvise, iFpLi)
GOW(madvise, iFpLi)
GOWM(makecontext, vFEppiV)
GOWM(mallinfo, pFEp)
GOWM(mallinfo2, pFEp)
GO(malloc, pFL)
//GO(malloc_get_state, 
GOW(malloc_info, iFiS)
//GO(malloc_set_state, 
GOW(malloc_stats, vFv)
GO(malloc_trim, iFL)
GO(malloc_usable_size, LFp)
GOW(mallopt, iFii)
//DATAB(mallwatch, 
GO(mblen, iFpL)
GO(__mbrlen, LFpLp)
GO(mbrlen, LFpLp)
GO(mbrtoc16, LFppLp)
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
//GOM(mcheck, iFp)
GO(mcheck_check_all, vFv)
//GOM(mcheck_pedantic, iFp)
GO(_mcleanup, vFv)
GO(_mcount, vFpp)
GO2(mcount, vFpp, _mcount)
GO(memalign, pFLL)
GO(memccpy, pFppiL)
GO(memchr, pFpiL)
GO(memcmp, iFppL)
GO(__memcmpeq, iFppL)
GO(memcpy, pFppL)
GO(__memcpy_chk, pFppLL)
GO(memfd_create, iFpu)
GO(memfrob, pFpL)
GO(memmem, pFpLpL)
GO(memmove, pFppL)
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
GO(mknod, iFpuU)
GO(mknodat, iFipuU)
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
GOWM(mmap64, pFEpLiiiI)
GOW(modf, dFdp)
GOW(modff, fFfp)
GOWD(modfl, DFDp, modf)
//GOW(modify_ldt, 
//GOW(moncontrol,
GO(__monstartup, vFLL)
#ifdef STATICBUILD
//GOW(monstartup, vFLL)
#else
GOW(monstartup, vFLL)
#endif
//DATA(__morecore, 
GOW(mount, iFpppLp)
GO(mprobe, iFp)
//GO(__mprotect, 
GOWM(mprotect, iFEpLi)
GO(mrand48, lFv)
GO(mrand48_r, iFpp)
GOWM(mremap, pFEpLLiN)
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
//GO(nfsservctl, 
GOM(nftw, iFEppii)
GOM(nftw64, iFEppii)
GOW(ngettext, pFppL)
GO(nice, iFi)
//DATAB(_nl_domain_bindings, 
GO(nl_langinfo, pFi)
GO(__nl_langinfo_l, pFip)
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
//GO(ntp_gettime, "iF!")
GO(ntp_gettimex, iFp)
//DATAB(_null_auth, 24)
//DATAB(_obstack, 
//GO(_obstack_allocated_p, 
//DATAM(obstack_alloc_failed_handler, 8)
GOM(_obstack_begin, iFpiipp)
//GOM(_obstack_begin_1, iFpiippp)
//DATA(obstack_exit_failure, 4)
GOM(_obstack_free, vFpp)
GOM(obstack_free, vFpp)
//GOM(_obstack_memory_used, iFp)
GOM(_obstack_newchunk, vFEpi)
//GOWM(obstack_printf, iFppV)
//GO(__obstack_printf_chk, 
GOWM(obstack_vprintf, iFEppA)
//GO(__obstack_vprintf_chk, 
//GOWM(on_exit, iFpp)
GOWM(__open, iFEpON)
GOWM(open, iFEpON)
GO(__open_2, iFpO)
//GOW(__open64, 
GOWM(open64, iFEpON)
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
GOW(open_memstream, SFpp)
//GO(__open_nocancel, 
GO(open_wmemstream, SFpp)
DATA(optarg, 8)
DATA(opterr, 4)
DATA(optind, 4)
DATA(optopt, 4)
GO(__overflow, iFSi)
GO(parse_printf_format, LFpLp)
#ifdef STATICBUILD
//GO(passwd2des, 
#else
GO(passwd2des, vFpp)
#endif
GO(pathconf, lFpi)
GOW(pause, iFv)
//GO(__pause_nocancel, 
GO(pclose, iFS)
GO(perror, vFp)
GOW(personality, iFL)
GO(__pipe, iFp)
GOW(pipe, iFp)
GO(pipe2, iFpO)
//GO(pivot_root, 
GO(pkey_alloc, iFuu)
GO(pkey_free, iFi)
GO(pkey_get, iFi)
GO(pkey_mprotect, iFpLii)
GO(pkey_set, iFiu)
#ifdef STATICBUILD
//GO(pmap_getmaps, pFp)
//GO(pmap_getport, WFpLLu)
//GO(pmap_rmtcall, "uF!LLL@p@p?p")
//GO(pmap_set, iFLLii)
//GO(pmap_unset, iFLL)
#else
GO(pmap_getmaps, pFp)
GO(pmap_getport, WFpLLu)
//GO(pmap_rmtcall, "uF!LLL@p@p?p")
GO(pmap_set, iFLLii)
GO(pmap_unset, iFLL)
#endif
GO(__poll, iFpLi)
GO(poll, iFpLi)
GO(__poll_chk, iFpuiL)
GO(popen, SFpp)
GO(posix_fadvise, iFilli)
GO(posix_fadvise64, iFiIIi)
GO(posix_fallocate, iFill)
GO(posix_fallocate64, iFiII)
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
GO(pread, lFipLl)
GOW(__pread64, lFipLI)
GO(pread64, lFipLI)
#ifdef LA64
GO2(__pread64_chk, lFipLlL, __pread_chk)
#else
GO(__pread64_chk, lFipLlL)
#endif
GO(__pread_chk, lFipLlL)
GO(preadv, lFipil)
GO(preadv2, lFipili)
GO(preadv64, lFipiI)
GO(preadv64v2, lFipiIi)
GOM(printf, iFEpV)
GOM(__printf_chk, iFEipV)
//GO(__printf_fp, 
GO(printf_size, iFSpp)
GO(printf_size_info, iFpLp)
GO(prlimit, iFiupp)
GOW(prlimit64, iFiupp)
GO(process_vm_readv, lFipLpLL)
GO(process_vm_writev, lFipLpLL)
GO(profil, iFpLLu)
//GO(__profile_frequency, 
DATA(__progname, 8)
DATA(__progname_full, 1)
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
GOW(putc, iFiS)
GO(putchar, iFi)
GO(putchar_unlocked, iFi)
GOW(putc_unlocked, iFiS)
GO(putenv, iFp)
GO(putgrent, iFpS)
//GO(putmsg, 
//GO(putpmsg, 
GO(putpwent, iFpS)
GOW(puts, iFp)
#ifdef STATICBUILD
//GO(putsgent, 
#else
GO(putsgent, iFpS)
#endif
GO(putspent, iFpS)
GOW(pututline, pFp)
GO(pututxline, pFp)
GO(putw, iFiS)
GO(putwc, uFiS)
GO(putwchar, uFi)
GO(putwchar_unlocked, uFi)
GO(putwc_unlocked, uFiS)
GO(pvalloc, pFL)
GO(pwrite, lFipLl)
//GOW(__pwrite64, 
GO(pwrite64, lFipLI)
GO(pwritev, lFipil)
GO(pwritev2, lFipili)
GO(pwritev64, lFipiI)
GO(pwritev64v2, lFipiIi)
GOD(qecvt, pFDipp, ecvt)
GOWD(qecvt_r, iFDipppL, ecvt_r)
GOD(qfcvt, pFDipp, fcvt)
GOD(qfcvt_r, iFDipppL, fcvt_r)
GOD(qgcvt, pFDip, gcvt)
GOM(qsort, vFEpLLp)
GOWM(qsort_r, vFEpLLpp)
//GO(query_module, 
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
GO(readahead, lFiIL)
GO(__read_chk, lFipLL)
GOW(readdir, pFp)
GOW(readdir64, pFp)
GOW(readdir64_r, iFppp)
GOW(readdir_r, iFppp)
GOWM(readlink, lFEppL)
GOM(readlinkat, lFEippL)
//GO(__readlinkat_chk, 
GOM(__readlink_chk, lFEppLL)
//GO(__read_nocancel, 
GOW(readv, lFipi)
GO(realloc, pFpL)
GO(reallocarray, pFpLL)
GOM(realpath, pFEpp)
GO2(__realpath_chk, pFEppv, my_realpath)
GO(reboot, iFi)
//GOW(re_comp, 
GOW(re_compile_fastmap, iFp)
GOW(re_compile_pattern, pFpLp)
GOW(__recv, lFipLi)
GO(recv, lFipLi)
GO(__recv_chk, lFipLLi)
GOW(recvfrom, lFipLipp)
//GO(__recvfrom_chk, 
GO(recvmmsg, iFipuip)
GO(recvmsg, lFipi)
//GOW(re_exec, 
GOW(regcomp, iFppi)
GOW(regerror, LFippL)
GO(regexec, iFppLpi)
GOW(regfree, vFp)
GOM(__register_atfork, iFEpppp)
//GOWM(register_printf_function, iFipp)
GOW(register_printf_modifier, iFp)
GOWM(register_printf_specifier, iFEipp)
GOWM(register_printf_type, iFEp)
//GOM(registerrpc, iFiiippp)
GO(remap_file_pages, iFpLiLi)
GOW(re_match, iFppiip)
GOW(re_match_2, iFppipiipi)
//DATA(re_max_failures, 
GO(remove, iFp)
GO(removexattr, iFpp)
GO(remque, vFp)
GO(rename, iFpp)
GOW(renameat, iFipip)
GOW(renameat2, iFipipu)
//DATAB(_res, 
GOW(re_search, iFppiiip)
GOW(re_search_2, iFppipiiipi)
GOW(re_set_registers, vFppupp)
GOW(re_set_syntax, LFL)
//DATAB(_res_hconf, 
GO(__res_iclose, vFpi)
GO(__res_init, iFv)
GO(__res_nclose, vFp)
GO(__res_ninit, iFp)
GO(res_nsearch, iFppiipi)
//GO(__resolv_context_get, 
//GO(__resolv_context_get_override, 
//GO(__resolv_context_get_preinit, 
//GO(__resolv_context_put, 
//DATA(__resp, 
#ifdef STATICBUILD
//GO(__res_randomid, 
#else
GO(__res_randomid, uFv)
#endif
GO(__res_state, pFv)
//DATAB(re_syntax_options, 8)
GOW(revoke, iFp)
GO(rewind, vFS)
GOW(rewinddir, vFp)
GO(rexec, iFpipppp)
GO(rexec_af, iFpippppW)
//DATAB(rexecoptions, 
GO(rindex, pFpi)
GOW(rmdir, iFp)
#ifdef STATICBUILD
//DATAB(rpc_createerr, 
//GO(_rpc_dtablesize, iFv)
//GO(__rpc_thread_createerr, "!Fv")
//GO(__rpc_thread_svc_fdset, "!Fv")
//GO(__rpc_thread_svc_max_pollfd, pFv)
//GO(__rpc_thread_svc_pollfd, pFv)
#else
//DATAB(rpc_createerr, 
GO(_rpc_dtablesize, iFv)
//GO(__rpc_thread_createerr, "!Fv")
//GO(__rpc_thread_svc_fdset, "!Fv")
GO(__rpc_thread_svc_max_pollfd, pFv)
GO(__rpc_thread_svc_pollfd, pFv)
#endif
GO(rpmatch, iFp)
GO(rresvport, iFp)
GO(rresvport_af, iFpW)
#ifdef STATICBUILD
//GO(rtime, 
#else
GO(rtime, iFppp)
#endif
GO(ruserok, iFpipp)
GO(ruserok_af, iFpippW)
//GO(ruserpass, 
GO(__sbrk, pFl)
GO(sbrk, pFl)
GOW(scalbn, dFdi)
GOW(scalbnf, fFfi)
GOWD(scalbnl, DFDi, scalbn)
GOWM(scandir, iFEpppp)
GOWM(scandir64, iFEpppp)
GOWM(scandirat, iFipppp)
//GOM(scandirat64, iFipppp)
GOM(scanf, iFpV)
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
GO(setbuf, vFSp)
GO(setbuffer, vFSpL)
GOWM(setcontext, iFEp)
GO(setdomainname, iFpL)
GO(setegid, iFu)
GOW(setenv, iFppi)
//GOM(_seterr_reply, vFpp)
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
GO(setipv4sourcefilter, iFiuuuup)
GOW(setitimer, iFupp)
GOM(_setjmp, iFEp)
GOM(setjmp, iFEp)
GO(setlinebuf, vFS)
GO(setlocale, pFip)
GO(setlogin, iFp)
GO(setlogmask, iFi)
GO(__setmntent, pFpp)
GOW(setmntent, SFpp)
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
#ifdef STATICBUILD
//GO(setsgent, 
#else
GO(setsgent, vFv)
#endif
GOW(setsid, iFv)
GOW(setsockopt, iFiiipu)
GO(setsourcefilter, iFiupuuup)
GO(setspent, vFv)
GOW(setstate, pFp)
GOW(setstate_r, iFpp)
GOW(settimeofday, iFpp)
GOW(setttyent, iFv)
GOW(setuid, iFu)
GO(setusershell, vFv)
GOW(setutent, vFv)
GO(setutxent, vFv)
GO(setvbuf, iFSpiL)
GO(setxattr, iFpppLi)
#ifdef STATICBUILD
//GO(sgetsgent, pFp)
//GOW(sgetsgent_r, iFpppLp)
#else
GO(sgetsgent, pFp)
GOW(sgetsgent_r, iFpppLp)
#endif
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
GO(sigandset, iFppp)
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
GOD(__signbitl, iFD, __signbit)
GO(sigorset, iFppp)
#ifdef STATICBUILD
//GO(__sigpause, 
//GOW(sigpause, 
#else
GO(__sigpause, iFii)
//GOW(sigpause, 
#endif
GO(sigpending, iFp)
GOW(sigprocmask, iFipp)
//GOW(sigqueue, "iFii?")
GO(sigrelse, iFi)
GOW(sigreturn, iFp)
GOM(sigset, pFEip)
GOM(__sigsetjmp, iFEpi)
GOM(sigsetjmp, iFEpi)
GOW(sigsetmask, iFi)
GO(sigstack, iFpp)
GO(__sigsuspend, iFp)
GOW(sigsuspend, iFp)
GO(__sigtimedwait, iFppp)
GOW(sigtimedwait, iFppp)
//GO(sigvec, 
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
#ifdef STATICBUILD
//GOW(sprofil, 
#else
GOW(sprofil, iFpipu)
#endif
GOW(srand, vFu)
GO(srand48, vFl)
GO(srand48_r, iFlp)
GOW(srandom, vFu)
GOW(srandom_r, iFup)
GOM(sscanf, iFEppV)
//GOWM(ssignal, pFip)
//GO(sstk, 
GOM(__stack_chk_fail, vFEv)
GOM(stat, iFEpp)
GOM(stat64, iFEpp)
//GO(__statfs, 
GOW(statfs, iFpp)
GOW(statfs64, iFpp)
GOW(statvfs, iFpp)
GOW(statvfs64, iFpp)
GOM(statx, iFEipiup)
DATA(stderr, 8)
DATA(stdin, 8)
DATA(stdout, 8)
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
#ifdef STATICBUILD
//GO(strfmon, lFpLpV)
//GO(__strfmon_l, 
#else
GO(strfmon, lFpLpdddddd)        // should be V, but only double are allowed...
//GO(__strfmon_l, 
#endif
GOW(strfmon_l, lFpLppdddddd)    // should be V, but only double are allowed...
GO(strfromd, iFpLpd)
GO(strfromf, iFpLpf)
GOD(strfromf128, iFpLpD, strfromf64)
GO(strfromf32, iFpLpf)
GO(strfromf32x, iFpLpd)
GO(strfromf64, iFpLpd)
GOD(strfromf64x, iFpLpD, strfromf32x)
GOD(strfroml, iFpLpD, strfromd)
GO(strfry, pFp)
GO(strftime, LFpLpp)
GO(__strftime_l, LFpLppL)
GOW(strftime_l, LFpLppp)
GO(strlen, LFp)
GO(__strlcpy_chk, LFppLL)
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
GOW(strptime_l, pFpppp)
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
GOD(strtof128, DFpp, strtof64)
//GO(__strtof128_internal, 
//GOW(strtof128_l, 
//GO(__strtof128_nan, 
GOW(strtof32, fFpp)
GOW(strtof32_l, fFppp)
GOW(strtof32x, dFpp)
GOW(strtof32x_l, dFppp)
GOW(strtof64, dFpp)
GOW(strtof64_l, dFppp)
GOWD(strtof64x, DFpp, strtof32x)
GOWD(strtof64x_l, DFppp, strtof32x_l)
GO(__strtof_internal, fFppi)
GO(__strtof_l, fFppL)
GOW(strtof_l, fFppp)
//GO(__strtof_nan, 
GO(strtoimax, IFppi)
GO2(__isoc23_strtoimax, IFppi, strtoimax)
GO2(__isoc23_strtoll_l, IFppip, strtoll_l)
GO2(__isoc23_strtoull_l, UFppip, strtoull_l)
GO2(__isoc23_vsscanf, iFEppA, my_vsscanf)
GO2(__isoc23_wcstoll, IFppi, wcstoll)
GO2(__isoc23_wcstoul, LFppi, wcstoul)
GO2(__isoc23_wcstoull, UFppi, wcstoull)
GO(strtok, pFpp)
GO(__strtok_r, pFppp)
GOW(strtok_r, pFppp)
//GO(__strtok_r_1c, 
GO(strtol, lFppi)
GOD(strtold, DFpp, strtod)
GOD(__strtold_internal, DFppi, __strtod_internal)
GOD(__strtold_l, DFppp, __strtod_l)
GOWD(strtold_l, DFppp, strtod_l)
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
GO(strtoumax, UFppi)
GO2(__isoc23_strtoumax, UFppi, strtoumax)
GOW(strtouq, UFppi)
//GO(__strverscmp, 
GOW(strverscmp, iFpp)
GO(strxfrm, LFppL)
GO(__strxfrm_l, LFppLL)
GO(strxfrm_l, LFppLp)
//GO(stty, 
#ifdef STATICBUILD
//DATAB(svcauthdes_stats, 
//GO(svcerr_auth, "vF!u")
//GO(svcerr_decode, "vF!")
//GO(svcerr_noproc, "vF!")
//GO(svcerr_noprog, "vF!")
//GO(svcerr_progvers, "vF!uu")
//GO(svcerr_systemerr, "vF!")
//GO(svcerr_weakauth, "vF!")
//GO(svc_exit, vFv)
//GO(svcfd_create, "!Fiuu")
//DATAB(svc_fdset, 
//GO(svc_getreq, vFi)
//GO(svc_getreq_common, vFi)
//GO(svc_getreq_poll, vFpi)
//GO(svc_getreqset, "vF!")
//DATAB(svc_max_pollfd, 
//DATAB(svc_pollfd, 
//GO(svcraw_create, "!Fv")
//GO(svc_register, "iF!LL@i")
//GO(svc_run, vFv)
//GO(svc_sendreply, "iF!@p")
//GO(svctcp_create, "!Fiuu")
//GO(svcudp_bufcreate, "!Fiuu")
//GO(svcudp_create, "!Fi")
//GO(svcudp_enablecache, 
//GO(svcunix_create, "!Fiuup")
//GO(svcunixfd_create, 
//GO(svc_unregister, vFLL)
#else
//DATAB(svcauthdes_stats, 
//GOM(svcerr_auth, vFpu)
//GOM(svcerr_decode, vFp)
//GOM(svcerr_noproc, vFp)
//GOM(svcerr_noprog, vFp)
//GOM(svcerr_progvers, vFpuu)
//GOM(svcerr_systemerr, vFp)
//GOM(svcerr_weakauth, vFp)
GO(svc_exit, vFv)
//GOM(svcfd_create, pFiuu)
//DATAB(svc_fdset, 128)
GO(svc_getreq, vFi)
GO(svc_getreq_common, vFi)
GO(svc_getreq_poll, vFpi)
GO(svc_getreqset, vFp)
//DATAB(svc_max_pollfd, 4)
//DATAB(svc_pollfd, 8)
//GOM(svcraw_create, pFv)
//GOM(svc_register, iFpLLpi)
GO(svc_run, vFv)
//GOM(svc_sendreply, iFppp)
//GOM(svctcp_create, pFiuu)
//GOM(svcudp_bufcreate, pFiuu)
//GOM(svcudp_create, pFi)
//GOM(svcudp_enablecache, iFpL)
//GOM(svcunix_create, pFiuup)
//GOM(svcunixfd_create, pFiuu)
GO(svc_unregister, vFLL)
#endif
GO(swab, vFppl)
GOWM(swapcontext, iFEpp)
#ifdef STATICBUILD
//GOW(swapoff, 
//GOW(swapon, 
#else
GOW(swapoff, iFp)
GOW(swapon, iFpi)
#endif
GOM(swprintf, iFEpLpV)
GOM(__swprintf_chk, iFEpLiLpV)
GOM(swscanf, iFEppV)
GOW(symlink, iFpp)
GO(symlinkat, iFpip)
GO(sync, vFv)
GO(sync_file_range, iFiIIu)
GO(syncfs, iFi)
GOM(syscall, lFEv)
GOM(__sysconf, lFEi)
GOM(sysconf, lFEi)
#ifdef STATICBUILD
//GO(__sysctl, 
//GOW(sysctl, 
//DATA(_sys_errlist, 8)
//DATA(sys_errlist, 8)
#else
GOM(__sysctl, iFEpipppL)
GOWM(sysctl, iFEpipppL)
DATA(_sys_errlist, 8)
DATA(sys_errlist, 8)
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
#ifdef STATICBUILD
//GO(thrd_current, 
#else
GO(thrd_current, LFv)
#endif
GO(thrd_exit, vFi)
#ifdef STATICBUILD
//GO(thrd_equal, 
//GO(thrd_sleep, 
//GO(thrd_yield, 
#else
GO(thrd_equal, iFLL)
GO(thrd_sleep, iFpp)
GO(thrd_yield, vFv)
#endif
GO(time, lFp)
GO(timegm, lFp)
GOW(timelocal, lFp)
GO(timerfd_create, iFii)
GO(timerfd_gettime, iFip)
GO(timerfd_settime, iFiipp)
GOW(times, lFp)
GO(timespec_get, iFpi)
DATAB(__timezone, 8)
DATAV(timezone, 8)
GO(tmpfile, SFv)
GOW(tmpfile64, SFv)
GO(tmpnam, pFp)
GO(tmpnam_r, pFp)
GO(toascii, iFi)
//GOW(__toascii_l, 
GO(_tolower, iFi)
GO(tolower, iFi)
GO(__tolower_l, iFip)
GOW(tolower_l, iFip)
GO(_toupper, iFi)
GO(toupper, iFi)
GO(__toupper_l, iFip)
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
GO(truncate, iFpl)
GO(truncate64, iFpI)
//GO(__tsearch, 
GOWM(tsearch, pFEppp)
GO(ttyname, pFi)
GO(ttyname_r, iFipL)
//GO(__ttyname_r_chk, 
GO(ttyslot, iFv)
//GO(__twalk, 
GOWM(twalk, vFEpp)
//GOWM(twalk_r, vFppp)
DATA(__tzname, 16)
DATAV(tzname, 16)
GOW(tzset, vFv)
GO(ualarm, uFuu)
GO(__uflow, iFS)
GOW(ulckpwdf, iFv)
#ifdef STATICBUILD
//GOW(ulimit, lFiV)
#else
GOW(ulimit, lFiN)
#endif
GOW(umask, uFu)
GOW(umount, iFp)
GOW(umount2, iFpi)
GOWM(uname, iFp)
GO(__underflow, iFp)
GOW(ungetc, iFiS)
GO(ungetwc, uFuS)
GOW(unlink, iFp)
GO(unlinkat, iFipi)
GO(unlockpt, iFi)
GOW(unsetenv, iFp)
GO(unshare, iFi)
GOW(updwtmp, vFpp)
GO(updwtmpx, vFpp)
//GO(uselib, 
GO(__uselocale, pFp)
GOW(uselocale, pFp)
#ifdef STATICBUILD
//GO(user2netname, 
#else
GO(user2netname, iFpup)
#endif
GO(usleep, iFu)
//GO(ustat, 
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
GOM(verr, vFEipA)
GOM(verrx, vFEipA)
GOW(versionsort, iFpp)
GOW(versionsort64, iFpp)
//GO(__vfork, 
GOWM(vfork, iFEv)
GOM(vfprintf, iFESpA)
GOM(__vfprintf_chk, iFEpvpp)
//GO(__vfscanf, 
GOWM(vfscanf, iFESpA)
GOWM(vfwprintf, iFESpA)
GOM(__vfwprintf_chk, iFEpipA)
GOWM(vfwscanf, iFSpA)
GO(vhangup, iFv)
//GO(vlimit, 
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
//GO(vtimes, 
GOM(vwarn, vFpA)
GOM(vwarnx, vFpA)
GOM(vwprintf, iFEpA)
//GO(__vwprintf_chk, 
GOM(vwscanf, iFpA)
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
GO(wcrtomb, LFpip)
GO(__wcrtomb_chk, LFpupL)
GOW(wcscasecmp, iFpp)
GO(__wcscasecmp_l, iFppp)
GOW(wcscasecmp_l, iFppp)
GOW(wcscat, pFpp)
GO(__wcscat_chk, pFppL)
GO(wcschr, pFpi)
GO(wcschrnul, pFpi)
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
GOW(wcsncasecmp_l, iFppLp)
GO(wcsncat, pFppL)
GO(__wcsncat_chk, pFppLL)
GO(wcsncmp, iFppL)
GOW(wcsncpy, pFppL)
GO(__wcsncpy_chk, pFppLL)
GO(wcsnlen, LFpL)
GO(wcsnrtombs, LFppLLp)
//GO(__wcsnrtombs_chk, 
GO(wcspbrk, pFpp)
GO(wcsrchr, pFpi)
GO(wcsrtombs, LFppLp)
GO(__wcsrtombs_chk, LFppLpL)
GO(wcsspn, LFpp)
GO(wcsstr, pFpp)
GO(wcstod, dFpp)
//GO(__wcstod_internal, 
//GO(__wcstod_l, 
GOW(wcstod_l, dFppp)
GO(wcstof, fFpp)
GOD(wcstof128, DFpp, wcstof64)
//GO(__wcstof128_internal, 
//GOW(wcstof128_l, 
GOW(wcstof32, fFpp)
GOW(wcstof32_l, fFppp)
GOW(wcstof32x, dFpp)
GOW(wcstof32x_l, dFppp)
GOW(wcstof64, dFpp)
GOW(wcstof64_l, dFppp)
GOWD(wcstof64x, DFpp, wcstof32x)
GOWD(wcstof64x_l, DFppp, wcstof32x_l)
//GO(__wcstof_internal, 
//GO(__wcstof_l, 
GOW(wcstof_l, fFppp)
GO(wcstoimax, IFppi)
GO(wcstok, pFppp)
GO(wcstol, lFppi)
GOD(wcstold, DFpp, wcstod)
//GO(__wcstold_internal, 
//GO(__wcstold_l, 
GOWD(wcstold_l, DFppp, wcstod_l)
//GO(__wcstol_internal, 
//GO(__wcstol_l, 
GOW(wcstol_l, lFppip)
GO2(__isoc23_wcstol_l, lFppip, wcstol_l)
GOW(wcstoll, IFppi)
//GO(__wcstoll_internal, 
//GOW(__wcstoll_l, 
GOW(wcstoll_l, IFppip)
GO(wcstombs, LFppL)
#ifdef STATICBUILD
//GO(__wcstombs_chk, LFppLL)    // it's sometimes not defined, like in LA64 plateform, so lets no wrap it for now
#else
GO(__wcstombs_chk, LFppLL)
#endif
GOW(wcstoq, IFppi)
GO(wcstoul, LFppi)
//GO(__wcstoul_internal, 
//GO(__wcstoul_l, 
GOW(wcstoul_l, LFppip)
GO2(__isoc23_wcstoul_l, LFppip, wcstoul_l)
GOW(wcstoull, UFppi)
//GO(__wcstoull_internal, 
//GOW(__wcstoull_l, 
GOW(wcstoull_l, UFppip)
GO(wcstoumax, UFppi)
GOW(wcstouq, UFppi)
GOW(wcswcs, pFpp)
GO(wcswidth, iFpL)
GO(wcsxfrm, LFppL)
GO(__wcsxfrm_l, LFppLL)
GO(wcsxfrm_l, LFppLp)
GO(wctob, iFu)
GO(wctomb, iFpi)
GO(__wctomb_chk, iFpuL)
GOW(wctrans, pFp)
//GO(__wctrans_l, 
GOW(wctrans_l, pFpp)
GOW(wctype, LFp)
GOW(wctype_l, LFpp)
GO(__wctype_l, LFpp)
GO(wcwidth, iFi)
GO(wmemchr, pFpiL)
GO(wmemcmp, iFppL)
GOW(wmemcpy, pFppL)
GO(__wmemcpy_chk, pFppLL)
GO(wmemmove, pFppL)
GO(__wmemmove_chk, pFppLL)
GOW(wmempcpy, pFppL)
//GO(__wmempcpy_chk, 
GO(wmemset, pFpiL)
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
GOM(wscanf, iFEpV)
//GO(__wuflow, 
//GO(__wunderflow, 
//GO(xdecrypt, 
#ifdef STATICBUILD
//GOM(xdr_accepted_reply, iFpp)
//GOM(xdr_array, iFpppuup)
//GO(xdr_authdes_cred, 
//GO(xdr_authdes_verf, 
//GOM(xdr_authunix_parms, iFpp)
//GOM(xdr_bool, iFpp)
//GOM(xdr_bytes, iFpppu)
//GOM(xdr_callhdr, iFpp)
//GOM(xdr_callmsg, iFpp)
//GOM(xdr_char, iFpp)
//GO(xdr_cryptkeyarg, 
//GO(xdr_cryptkeyarg2, 
//GO(xdr_cryptkeyres, 
//GOM(xdr_des_block, iFpp)
//GOM(xdr_double, iFpp)
//GOM(xdr_enum, iFpp)
//GOM(xdr_float, iFpp)
//GOM(xdr_free, vFpp)
//GO(xdr_getcredres, 
//GOM(xdr_hyper, iFpp)
//GOM(xdr_int, iFpp)
//GOM(xdr_int16_t, iFpp)
//GOM(xdr_int32_t, iFpp)
//GOM(xdr_int64_t, iFpp)
//GOM(xdr_int8_t, iFpp)
//GO(xdr_keybuf, 
//GO(xdr_key_netstarg, 
//GO(xdr_key_netstres, 
//GO(xdr_keystatus, 
//GOM(xdr_long, iFpp)
//GOM(xdr_longlong_t, iFpp)
//GOM(xdrmem_create, vFppuu)
//GO(xdr_netnamestr, 
//GOM(xdr_netobj, iFpp)
//GOM(xdr_opaque, iFppu)
//GOM(xdr_opaque_auth, iFpp)
//GOM(xdr_pmap, iFpp)
//GOM(xdr_pmaplist, iFpp)
//GOM(xdr_pointer, iFppup)
//GOM(xdr_quad_t, iFpp)
//GOM(xdrrec_create, vFpuuppp)
//GOM(xdrrec_endofrecord, iFpi)
//GOM(xdrrec_eof, iFp)
//GOM(xdrrec_skiprecord, iFp)
//GOM(xdr_reference, iFppup)
//GOM(xdr_rejected_reply, iFpp)
//GOM(xdr_replymsg, iFpp)
//GO(xdr_rmtcall_args, 
//GO(xdr_rmtcallres, 
//GOM(xdr_short, iFpp)
//GOM(xdr_sizeof, LFpp)
//GOM(xdrstdio_create, vFpSu)
//GOM(xdr_string, iFppu)
//GOM(xdr_u_char, iFpp)
//GOM(xdr_u_hyper, iFpp)
//GOM(xdr_u_int, iFpp)
//GOM(xdr_uint16_t, iFpp)
//GOM(xdr_uint32_t, iFpp)
//GOM(xdr_uint64_t, iFpp)
//GOM(xdr_uint8_t, iFpp)
//GOM(xdr_u_long, iFpp)
//GOM(xdr_u_longlong_t, iFpp)
//GOM(xdr_union, iFppppp)
//GO(xdr_unixcred, 
//GOM(xdr_u_quad_t, iFpp)
//GOM(xdr_u_short, iFpp)
//GOM(xdr_vector, iFppuup)
//GO(xdr_void, 
//GO(xdr_wrapstring, "iF!p")
#else
//GOM(xdr_accepted_reply, iFpp)
//GOM(xdr_array, iFpppuup)
//GO(xdr_authdes_cred, 
//GO(xdr_authdes_verf, 
//GOM(xdr_authunix_parms, iFpp)
//GOM(xdr_bool, iFpp)
//GOM(xdr_bytes, iFpppu)
//GOM(xdr_callhdr, iFpp)
//GOM(xdr_callmsg, iFpp)
//GOM(xdr_char, iFpp)
//GOM(xdr_cryptkeyarg, iFpp)
//GOM(xdr_cryptkeyarg2, iFpp)
//GOM(xdr_cryptkeyres, iFpp)
//GOM(xdr_des_block, iFpp)
//GOM(xdr_double, iFpp)
//GOM(xdr_enum, iFpp)
//GOM(xdr_float, iFpp)
GO(xdr_free, vFpp)
//GOM(xdr_getcredres, iFpp)
//GOM(xdr_hyper, iFpp)
//GOM(xdr_int, iFpp)
//GOM(xdr_int16_t, iFpp)
//GOM(xdr_int32_t, iFpp)
//GOM(xdr_int64_t, iFpp)
//GOM(xdr_int8_t, iFpp)
//GOM(xdr_keybuf, iFpp)
//GOM(xdr_key_netstarg, iFpp)
//GOM(xdr_key_netstres, iFpp)
//GOM(xdr_keystatus, iFpp)
//GOM(xdr_long, iFpp)
//GOM(xdr_longlong_t, iFpp)
//GOM(xdrmem_create, vFppuu)
//GOM(xdr_netnamestr, iFpp)
//GOM(xdr_netobj, iFpp)
//GOM(xdr_opaque, iFppu)
//GOM(xdr_opaque_auth, iFpp)
//GOM(xdr_pmap, iFpp)
//GOM(xdr_pmaplist, iFpp)
//GOM(xdr_pointer, iFppup)
//GOM(xdr_quad_t, iFpp)
//GOM(xdrrec_create, vFpuuppp)
//GOM(xdrrec_endofrecord, iFpi)
//GOM(xdrrec_eof, iFp)
//GOM(xdrrec_skiprecord, iFp)
//GOM(xdr_reference, iFppup)
//GOM(xdr_rejected_reply, iFpp)
//GOM(xdr_replymsg, iFpp)
//GO(xdr_rmtcall_args, 
//GO(xdr_rmtcallres, 
//GOM(xdr_short, iFpp)
//GOM(xdr_sizeof, LFpp)
//GOM(xdrstdio_create, vFpSu)
GO(xdr_string, iFppu)
//GOM(xdr_u_char, iFpp)
//GOM(xdr_u_hyper, iFpp)
//GOM(xdr_u_int, iFpp)
//GOM(xdr_uint16_t, iFpp)
//GOM(xdr_uint32_t, iFpp)
//GOM(xdr_uint64_t, iFpp)
//GOM(xdr_uint8_t, iFpp)
//GOM(xdr_u_long, iFpp)
//GOM(xdr_u_longlong_t, iFpp)
//GOM(xdr_union, iFppppp)
//GOM(xdr_unixcred, iFpp)
//GOM(xdr_u_quad_t, iFpp)
//GOM(xdr_u_short, iFpp)
//GOM(xdr_vector, iFppuup)
GO(xdr_void, iFv)
//GOM(xdr_wrapstring, iFpp)
#endif
//GO(xencrypt, 
#ifdef STATICBUILD
//GO(__xmknod, iFipup)
//GO(__xmknodat, iFiipup)
#else
GOM(__xmknod, iFEipup)
GOM(__xmknodat, iFEiipup)
#endif
GO(__xpg_basename, pFp)
GOW(__xpg_sigpause, iFi)
GO(__xpg_strerror_r, pFipL)
//GOM(xprt_register, vFp)
//GOM(xprt_unregister, vFp)
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

DATAM(program_invocation_name, 8)
DATAM(program_invocation_short_name, 8)

DATAM(__libc_single_threaded, 1)

#ifdef STATICBUILD
//GO(iconvctl, 
#else
GO(iconvctl, iFlip)
#endif
GO(dummy__ZnwmSt11align_val_tRKSt9nothrow_t, pFLLp) // for mallochook.c

GOWM(_ZGTtnaX, pFL) //%noE
GOWM(_ZGTtdlPv, vFp)    //%noE
GOWM(_ZGTtnam, pFL) //%noE
GOWM(_ITM_RU1, CFp) //%noE
GOWM(_ITM_RU4, uFp) //%noE
GOWM(_ITM_RU8, UFp) //%noE
GOWM(_ITM_memcpyRtWn, vFppL)  //%noE
GOWM(_ITM_memcpyRnWt, vFppL)  //%noE

GOM(__udivti3, HFHH)    //%noE

#ifdef ANDROID
GOM(__libc_init, vFEpppp)
GO(__errno, pFv)
GO(android_set_abort_message, vFp)
#else
// Those symbols don't exist in non-Android builds
//GOM(__libc_init, 
//GO(__errno, 
//GO(android_set_abort_message, vFp)
#endif
#ifdef STATICBUILD
GO(dummy_pFLp, pFLp)
GO(dummy_pFpLLp, pFpLLp)
#else
// not needed in no-static build
//GO(dummy_pFLp, pFLp)
//GO(dummy_pFpLLp, pFpLLp)
#endif

GO(arc4random, uFv)