#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

GO(baudrate, iFv)
GO(cbreak, iFv)
GO(curses_version, pFv)
GO(curs_set, iFi)
GO(define_key, iFpi)
GO(def_prog_mode, iFv)
GO(def_shell_mode, iFv)
GO(delay_output, iFi)
GOM(del_curterm, iFEp)
GO(erasechar, cFv)
GO(flushinp, iFv)
GO(halfdelay, iFi)
GO(has_ic, iFv)
GO(has_il, iFv)
GO(has_key, iFi)
GO(idcok, vFpi)
GO(idlok, iFpi)
GO(intrflush, iFpi)
GO(keybound, pFii)
GO(key_defined, iFp)
GO(keyname, pFi)
GO(keyok, iFii)
GO(keypad, iFpi)
GO(killchar, cFv)
GO(longname, pFv)
GO(meta, iFpi)
GO(napms, iFi)
GO(_nc_basename, pFp)
GO(_nc_is_abs_path, iFp)
GO(_nc_rootname, pFp)
GOM(_nc_tiparm, pFEipV)
//GO(_nc_access, 
//GO(_nc_add_to_try, 
//GO(_nc_align_termtype, 
//GO(_nc_baudrate, 
//GO(_nc_copy_termtype, 
//GO(_nc_delink_entry, 
//GO(_nc_doalloc, 
//GO(_nc_err_abort, 
//GO(_nc_expand_try, 
//GO(_nc_fallback, 
//GO(_nc_find_entry, 
//GO(_nc_find_type_entry, 
//GO(_nc_first_db, 
//GO(_nc_first_name, 
//GO(_nc_flush, 
//GO(_nc_free_entries, 
//GO(_nc_free_entry, 
//GO(_nc_free_termtype, 
//GO(_nc_get_alias_table, 
//GO(_nc_getenv_num, 
//GO(_nc_get_hash_info, 
//GO(_nc_get_hash_table, 
//GO(_nc_get_locale, 
//GO(_nc_get_screensize, 
//GO(_nc_get_source, 
//GO(_nc_get_table, 
//GO(_nc_get_tty_mode, 
//GO(_nc_get_type, 
//GO(_nc_handle_sigwinch, 
//GO(_nc_hashed_db, 
//GO(_nc_home_terminfo, 
//GO(_nc_init_acs, 
//GO(_nc_init_keytry, 
//GO(_nc_is_dir_path, 
//GO(_nc_is_file_path, 
//GO(_nc_keep_tic_dir, 
//GO(_nc_keyname, 
//GO(_nc_keypad, 
//GO(_nc_last_db, 
//GO(_nc_leaks_tinfo, 
//GO(_nc_locale_breaks_acs, 
//GO(_nc_name_match, 
//GO(_nc_next_db, 
//GO(_nc_ospeed, 
//GO(_nc_outch, 
//GO(_nc_pathlast, 
//GO(_nc_putp, 
//GO(_nc_putp_flush, 
//GO(_nc_read_entry, 
//GO(_nc_read_file_entry, 
//GO(_nc_read_termcap, 
//GO(_nc_read_termtype, 
//GO(_nc_remove_key, 
//GO(_nc_remove_string, 
//GO(_nc_safe_strcat, 
//GO(_nc_safe_strcpy, 
//GO(_nc_screen_of, 
//GO(_nc_set_buffer, 
//GO(_nc_set_no_padding, 
//GO(_nc_set_source, 
//GO(_nc_set_tty_mode, 
//GO(_nc_set_type, 
//GO(_nc_setupterm, 
//GO(_nc_setup_tinfo, 
//GO(_nc_str_copy, 
//GO(_nc_str_init, 
//GO(_nc_str_null, 
//GO(_nc_syserr_abort, 
//GO(_nc_tic_dir, 
//GO(_nc_timed_wait, 
//GO(_nc_tinfo_cmdch, 
//GO(_nc_tparm_analyze, 
//GO(_nc_trim_sgr0, 
//GO(_nc_unctrl, 
//GO(_nc_unicode_locale, 
//GO(_nc_update_screensize, 
//GO(_nc_visbuf, 
//GO(_nc_visbuf2, 
//GO(_nc_visbufn, 
//GO(_nc_warning, 
GO(nocbreak, iFv)
GO(nodelay, iFpi)
GO(noqiflush, vFv)
GO(noraw, iFv)
GO(notimeout, iFpi)
GO(putp, iFp)
GO(qiflush, vFv)
GO(raw, iFv)
GO(reset_prog_mode, iFv)
GO(reset_shell_mode, iFv)
GO(resetty, iFv)
GO(savetty, iFv)
GOM(set_curterm, pFEp)
GO(set_tabsize, iFi)
GOM(setupterm, iFEpip)
GO(termname, pFv)
GO(tgetent, iFpp)
GO(tgetflag, iFp)
GO(tgetnum, iFp)
GO(tgetstr, pFpp)
GO(tgoto, pFpii)
GO(tigetflag, iFp)
GO(tigetnum, iFp)
GO(tigetstr, pFp)
//GO(tiparm, p(p,...)
GO(tparm, pFplllllllll) // va_arg or 9 long, depending on how old the .h is
GOM(tputs, iFEpip)
GO(typeahead, iFi)
GO(unctrl, pFL)
GO(use_env, vFi)
GO(use_extended_names, iFi)
GO(use_tioctl, vFi)
GO(wtimeout, vFpi)

DATA(COLS, sizeof(int))
DATA(LINES, sizeof(int))
DATA(TABSIZE, sizeof(int))
DATA(curscr, sizeof(void*))
DATA(newscr, sizeof(void*))
DATA(stdscr, sizeof(void*))
DATA(acs_map, 64*sizeof(void*))
DATA(UP, sizeof(void*))
DATA(BC, sizeof(void*))
DATA(PC, 1)
DATA(ospeed, 2)
DATA(ttytype, 32*sizeof(void*))
DATA(cur_term, sizeof(void*))
