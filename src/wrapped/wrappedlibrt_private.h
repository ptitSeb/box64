#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

//clockid_t is uint32?
// pid_t is uint32 too? (for clock_getcpuclockid)
// time_t is uint32?
// mqd_t is int

GOM(aio_cancel, iFEip)
// aio_cancel64
GO(aio_error, iFp)
GO(aio_error64, iFp)
GO(aio_fsync, iFip)
GO(aio_fsync64, iFip)
GO(aio_init, vFp)
GOM(aio_read, iFEp)
GOM(aio_read64, iFEp)
GO(aio_return, lFp)
GO(aio_return64, lFp)
GO(aio_suspend, iFpip)
GO(aio_suspend64, iFpip)
GOM(aio_write, iFEp)
GOM(aio_write64, iFEp)
GO(clock_getcpuclockid, iFip)
GO(clock_getres, iFip)
GO(clock_gettime, iFip)
GO(clock_nanosleep, iFiipp)
GO(clock_settime, iFip)
GOM(lio_listio, iFEipip)
// lio_listio64
GO(mq_close, iFi)
GO(mq_getattr, iFip)
GO(mq_notify, iFip)
GO(mq_open, iFpONN)
GO(__mq_open_2, iFpO)
GO(mq_receive, lFipLp)
GO(mq_send, iFipLu)
GO(mq_setattr, iFipp)
GO(mq_timedreceive, lFipLpp)
GO(mq_timedsend, iFipLup)
GO(mq_unlink, iFp)
GO(shm_open, iFpOu)
GO(shm_unlink, iFp)
GOM(timer_create, iFEipp)
GO(timer_delete, iFp)
GO(timer_getoverrun, iFp)
GO(timer_gettime, iFpp)
GO(timer_settime, iFpipp)
