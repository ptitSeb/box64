#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

//clockid_t is uint32?
// pid_t is uint32 too? (for clock_getcpuclockid)
// time_t is uint32?

// aio_cancel
// aio_cancel64
// aio_error
GO(aio_error64, iFp)
// aio_fsync
// aio_fsync64
// aio_init
// aio_read
GO(aio_read64, iFp)
// aio_return
GO(aio_return64, lFp)
// aio_suspend
GO(aio_suspend64, iFip)
// aio_write
GO(aio_write64, iFp)
GO(clock_getcpuclockid, iFup)
GO(clock_getres, iFup)
GO(clock_gettime, iFup)
GO(clock_nanosleep, iFuipp)
GO(clock_settime, iFup)
// lio_listio
// lio_listio64
// mq_close
GO(mq_getattr, iFip)
// mq_notify
// mq_open
// __mq_open_2
// mq_receive
// mq_send
// mq_setattr
// mq_timedreceive
// mq_timedsend
// mq_unlink
GO(shm_open, iFpiu)
GO(shm_unlink, iFp)
GOM(timer_create, iFEupp)
GO(timer_delete, iFu)
GO(timer_getoverrun, iFu)
GO(timer_gettime, iFup)
GO(timer_settime, iFuipp)
