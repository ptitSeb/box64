#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

GO(udev_get_dev_path, pFp)
GO(udev_get_run_path, pFp)
GO(udev_get_sys_path, pFp)
GO(udev_monitor_new_from_socket, pFpp)
GO(udev_queue_get_failed_list_entry, pFp)
