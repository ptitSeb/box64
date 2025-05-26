#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

//clockid_t is uint32?
// pid_t is uint32 too? (for clock_getcpuclockid)
// time_t is long
// timespec is struct LL
// itimerspec is struct 2 timespec (so LLLL)

// aio_cancel
// aio_cancel64
// aio_error
GO(aio_error64, iEp)
GO(aio_fsync, iEip)
GO(aio_fsync64, iEip)
// aio_init
// aio_read
GO(aio_read64, iEp)
// aio_return
GO(aio_return64, lEp)
GO(aio_suspend, iEpip)
GO(aio_suspend64, iEpip)
// aio_write
GO(aio_write64, iEp)
GO(clock_getcpuclockid, iEup)
GO(clock_getres, iEuBLL_)
GO2(__clock_getres64, iEup, clock_getres)
GO(clock_gettime, iEuBLL_)   // *timespec
GO2(__clock_gettime, iEuBLL_, clock_gettime)
GO2(__clock_gettime64, iEup, clock_gettime)
GO(clock_nanosleep, iEuirLL_BLL_)
//GO(clock_settime, iEup)
// lio_listio
// lio_listio64
// mq_close
GO(mq_getattr, iEip)
// mq_notify
// mq_open
// __mq_open_2
// mq_receive
// mq_send
// mq_setattr
// mq_timedreceive
// mq_timedsend
// mq_unlink
GO(shm_open, iEpOu)
GO(shm_unlink, iEp)
//GOM(timer_create, iEEupp)
GO(timer_delete, iEu)
GO(timer_getoverrun, iEu)
//GO(timer_gettime, iEup)   //time_t is L, timespec is struct LL, itimerspec (the p) is 2 timespec
//GO(timer_settime, iEuipp) //same, p are itimerspec
