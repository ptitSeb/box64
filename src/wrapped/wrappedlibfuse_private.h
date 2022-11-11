#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

//GO(cuse_lowlevel_main, 
//GO(cuse_lowlevel_new, 
//GO(cuse_lowlevel_setup, 
//GO(cuse_lowlevel_teardown, 
//GO(fuse_add_dirent, 
GOM(fuse_add_direntry, LFEppLppU)
//GO(fuse_buf_copy, 
//GO(fuse_buf_size, 
//GO(fuse_chan_bufsize, 
//GO(fuse_chan_data, 
//GO(fuse_chan_destroy, 
//GO(fuse_chan_fd, 
//GO(fuse_chan_new, 
//GO(fuse_chan_new_compat24, 
//GO(fuse_chan_receive, 
//GO(fuse_chan_recv, 
//GO(fuse_chan_send, 
//GO(fuse_chan_session, 
//GO(fuse_clean_cache, 
GO(fuse_daemonize, iFi)
//GO(fuse_destroy, 
//GO(fuse_dirent_size, 
//GO(fuse_exit, 
//GO(__fuse_exited, 
//GO(fuse_exited, 
//GO(fuse_fs_access, 
//GO(fuse_fs_bmap, 
//GO(fuse_fs_chmod, 
//GO(fuse_fs_chown, 
//GO(fuse_fs_create, 
//GO(fuse_fs_destroy, 
//GO(fuse_fs_fallocate, 
//GO(fuse_fs_fgetattr, 
//GO(fuse_fs_flock, 
//GO(fuse_fs_flush, 
//GO(fuse_fs_fsync, 
//GO(fuse_fs_fsyncdir, 
//GO(fuse_fs_ftruncate, 
//GO(fuse_fs_getattr, 
//GO(fuse_fs_getxattr, 
//GO(fuse_fs_init, 
//GO(fuse_fs_ioctl, 
//GO(fuse_fs_link, 
//GO(fuse_fs_listxattr, 
//GO(fuse_fs_lock, 
//GO(fuse_fs_mkdir, 
//GO(fuse_fs_mknod, 
//GO(fuse_fs_new, 
//GO(fuse_fs_open, 
//GO(fuse_fs_opendir, 
//GO(fuse_fs_poll, 
//GO(fuse_fs_read, 
//GO(fuse_fs_read_buf, 
//GO(fuse_fs_readdir, 
//GO(fuse_fs_readlink, 
//GO(fuse_fs_release, 
//GO(fuse_fs_releasedir, 
//GO(fuse_fs_removexattr, 
//GO(fuse_fs_rename, 
//GO(fuse_fs_rmdir, 
//GO(fuse_fs_setxattr, 
//GO(fuse_fs_statfs, 
//GO(fuse_fs_symlink, 
//GO(fuse_fs_truncate, 
//GO(fuse_fs_unlink, 
//GO(fuse_fs_utimens, 
//GO(fuse_fs_write, 
//GO(fuse_fs_write_buf, 
//GO(fuse_get_context, 
//GO(fuse_getgroups, 
//GO(fuse_get_session, 
//GO(fuse_interrupted, 
//GO(fuse_invalidate, 
//GO(fuse_is_lib_option, 
//GO(fuse_kern_chan_new, 
//GO(fuse_loop, 
//GO(__fuse_loop_mt, 
//GO(fuse_loop_mt, 
//GO(fuse_loop_mt_proc, 
//GO(fuse_lowlevel_is_lib_option, 
GOM(fuse_lowlevel_new, pFEppLp)
//GO(fuse_lowlevel_new_compat, 
//GO(fuse_lowlevel_new_compat25, 
//GO(fuse_lowlevel_notify_delete, 
//GO(fuse_lowlevel_notify_inval_entry, 
//GO(fuse_lowlevel_notify_inval_inode, 
//GO(fuse_lowlevel_notify_poll, 
//GO(fuse_lowlevel_notify_retrieve, 
//GO(fuse_lowlevel_notify_store, 
//GO(fuse_main, 
//GO(fuse_main_compat1, 
//GO(fuse_main_compat2, 
GOM(fuse_main_real, iFEippLp)
//GO(fuse_main_real_compat22, 
//GO(fuse_main_real_compat25, 
GO(fuse_mount, pFpp)
//GO(fuse_mount_compat1, 
//GO(fuse_mount_compat22, 
//GO(fuse_mount_compat25, 
//GO(fuse_new, 
//GO(fuse_new_compat1, 
//GO(fuse_new_compat2, 
//GO(fuse_new_compat22, 
//GO(fuse_new_compat25, 
//GO(fuse_notify_poll, 
GO(fuse_opt_add_arg, iFpp)
GO(fuse_opt_add_opt, iFpp)
GO(fuse_opt_add_opt_escaped, iFpp)
GO(fuse_opt_free_args, vFp)
GO(fuse_opt_insert_arg, iFpip)
GO(fuse_opt_match, iFpp)
GOM(fuse_opt_parse, iFEpppp)
GO(fuse_parse_cmdline, iFpppp)
//GO(fuse_pollhandle_destroy, 
//GO(__fuse_process_cmd, 
//GO(fuse_process_cmd, 
//GO(__fuse_read_cmd, 
//GO(fuse_read_cmd, 
//GO(fuse_register_module, 
GO(fuse_remove_signal_handlers, vFp)
GOM(fuse_reply_attr, iFEppd)
GO(fuse_reply_bmap, iFpU)
GO(fuse_reply_buf, iFppL)
GOM(fuse_reply_create, iFEppp)
GO(fuse_reply_data, iFppi)
GOM(fuse_reply_entry, iFEpp)
GO(fuse_reply_err, iFpi)
GO(fuse_reply_ioctl, iFpipL)
GO(fuse_reply_ioctl_iov, iFpipi)
GO(fuse_reply_ioctl_retry, iFppLpL)
GO(fuse_reply_iov, iFppi)
GO(fuse_reply_lock, iFpp)
GO(fuse_reply_none, vFp)
GO(fuse_reply_open, iFpp)
GO(fuse_reply_open_compat, iFpp)
GO(fuse_reply_poll, iFpu)
GO(fuse_reply_readlink, iFpp)
GO(fuse_reply_statfs, iFpp)
//GO(fuse_reply_statfs_compat, 
GO(fuse_reply_write, iFpL)
GO(fuse_reply_xattr, iFpL)
//GO(fuse_req_ctx, 
//GO(fuse_req_getgroups, 
//GO(fuse_req_interrupted, 
//GO(fuse_req_interrupt_func, 
GO(fuse_req_userdata, pFp)
GO(fuse_session_add_chan, vFpp)
//GO(fuse_session_data, 
GO(fuse_session_destroy, vFp)
//GO(fuse_session_exit, 
//GO(fuse_session_exited, 
GO(fuse_session_loop, iFp)
GO(fuse_session_loop_mt, iFp)
//GO(fuse_session_new, 
//GO(fuse_session_next_chan, 
//GO(fuse_session_process, 
//GO(fuse_session_process_buf, 
GO(fuse_session_remove_chan, vFp)
//GO(fuse_session_reset, 
//GO(__fuse_set_getcontext_func, 
//GO(fuse_set_getcontext_func, 
GO(fuse_set_signal_handlers, iFp)
//GO(__fuse_setup, 
//GO(fuse_setup, 
//GO(fuse_setup_compat2, 
//GO(fuse_setup_compat22, 
//GO(fuse_setup_compat25, 
//GO(fuse_start_cleanup_thread, 
//GO(fuse_stop_cleanup_thread, 
//GO(__fuse_teardown, 
//GO(fuse_teardown, 
//GO(fuse_teardown_compat22, 
GO(fuse_unmount, vFpp)
//GO(fuse_unmount_compat22, 
//GO(fuse_version, 

GO(dummy_getdir_op, iFppiU)    // dummy functions for callbacks
GO(dummy_readdir_op, iFpppUi)