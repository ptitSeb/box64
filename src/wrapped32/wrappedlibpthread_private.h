#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif
// General note: pthread_t is unsigned long int
// cpu_set_t is a struct with an array, default size is fixed for all architecture
// struct timespec is: LL

// __errno_location
// fork
// __fork
// __h_errno_location
// _IO_flockfile
// _IO_ftrylockfile
// _IO_funlockfile
// __libc_allocate_rtsig
// __libc_current_sigrtmax
// __libc_current_sigrtmin
GOM(pthread_atfork, iFEppp)
GOM(__pthread_atfork, iFEppp)
GOM(pthread_attr_destroy, iFEp)
// pthread_attr_getaffinity_np
GOM(pthread_attr_getdetachstate, iFEpp)
GOM(pthread_attr_getguardsize, iFEpp)
GOM(pthread_attr_getinheritsched, iFEpp)
GOM(pthread_attr_getschedparam, iFEpp)
GOM(pthread_attr_getschedpolicy, iFEpp)
GOM(pthread_attr_getscope, iFEpp)
GOM(pthread_attr_getstack, iFEppp)
GOM(pthread_attr_getstackaddr, iFEpp)
GOM(pthread_attr_getstacksize, iFEpp)
GOM(pthread_attr_init, iFEp)
GOM(pthread_attr_setaffinity_np, iFEpup)
GOM(pthread_attr_setdetachstate, iFEpi)
GOM(pthread_attr_setguardsize, iFEpL)
GOM(pthread_attr_setinheritsched, iFEpi)
GOM(pthread_attr_setschedparam, iFEpp)
GOM(pthread_attr_setschedpolicy, iFEpi)
GOM(pthread_attr_setscope, iFEpi)
GOM(pthread_attr_setstack, iFEppL)
GOM(pthread_attr_setstackaddr, iFEpp)
GOM(pthread_attr_setstacksize, iFEpL)
GO(pthread_barrierattr_destroy, iFp)
GO(pthread_barrierattr_getpshared, iFpp)
GO(pthread_barrierattr_init, iFp)
GO(pthread_barrierattr_setpshared, iFpi)
GO(pthread_barrier_destroy, iFp)
GO(pthread_barrier_init, iFppu)
GO(pthread_barrier_wait, iFp)
GO(pthread_cancel, iFh)
GOM(_pthread_cleanup_pop, vFEpi)
GOM(_pthread_cleanup_pop_restore, vFEpi)
GOM(_pthread_cleanup_push, vFEppp)
GOM(_pthread_cleanup_push_defer, vFEppp)
// __pthread_cleanup_routine
GO(pthread_condattr_destroy, iFp)
GO(pthread_condattr_getclock, iFpp)
GO(pthread_condattr_getpshared, iFpp)
GO(pthread_condattr_init, iFp)
GO(pthread_condattr_setclock, iFpp)
GO(pthread_condattr_setpshared, iFpi)
GO2(pthread_cond_broadcast@GLIBC_2.0, iFEp, my32_pthread_cond_broadcast_old)
GO2(pthread_cond_destroy@GLIBC_2.0, iFEp, my32_pthread_cond_destroy_old)
GO2(pthread_cond_init@GLIBC_2.0, iFEpp, my32_pthread_cond_init_old)
GO2(pthread_cond_signal@GLIBC_2.0, iFEp, my32_pthread_cond_signal_old)
GO2(pthread_cond_timedwait@GLIBC_2.0, iFEpprLL_, my32_pthread_cond_timedwait_old)
GO2(pthread_cond_wait@GLIBC_2.0, iFEpp, my32_pthread_cond_wait_old)
GOM(pthread_cond_broadcast, iFEp)
GOM(pthread_cond_destroy, iFEp)
GOM(pthread_cond_init, iFEpp)
GOM(pthread_cond_signal, iFEp)
GOM(pthread_cond_timedwait, iFEpprLL_)
GOM(pthread_cond_wait, iFEpp)
GOM(pthread_create, iFEBh_ppp)
GOM(pthread_detach, iFEh)
GO(pthread_equal, iFhh)
GO(pthread_exit, vFp)
GOM(pthread_getaffinity_np, iFEhup)
GOM(pthread_getattr_np, iFEhp)
GO(pthread_getconcurrency, iFv)
GO(pthread_getcpuclockid, iFhp)
GO(pthread_getschedparam, iFhpp)
GO(__pthread_getspecific, pFu)
GO(pthread_getspecific, pFu)
GO(pthread_getname_np, iFhpL)
GOM(__pthread_initialize, vFv)  //%noE doesn't exist anymore...
// __pthread_initialize_minimal
GO(pthread_join, iFHBp_)
GOM(__pthread_key_create, iFEpp)
GOM(pthread_key_create, iFEpp)
GO(pthread_key_delete, iFu)
GO(pthread_kill, iFhi)
GO2(pthread_kill@GLIBC_2.0, iFEhi, my32_pthread_kill_old)
// pthread_kill_other_threads_np
GO(__pthread_mutexattr_destroy, iFp)
GO(pthread_mutexattr_destroy, iFp)
GO(pthread_mutexattr_getprioceiling, iFpp)
GO(pthread_mutexattr_getprotocol, iFpp)
// pthread_mutexattr_getpshared
// pthread_mutexattr_getrobust_np
GO(pthread_mutexattr_gettype, iFpp)
GO2(__pthread_mutexattr_init, iFp, my32_pthread_mutexattr_init)
GOM(pthread_mutexattr_init, iFEp)
GO(pthread_mutexattr_setprioceiling, iFpi)
GO(pthread_mutexattr_setprotocol, iFpp)
GO(pthread_mutexattr_setpshared, iFpi)
// pthread_mutexattr_setrobust_np
GO(__pthread_mutexattr_settype, iFpi)
GO(pthread_mutexattr_settype, iFpi)
// pthread_mutex_consistent_np
GOM(__pthread_mutex_destroy, iFp)  //%noE
GOM(pthread_mutex_destroy, iFp)    //%noE
// pthread_mutex_getprioceiling
GOM(__pthread_mutex_init, iFpp)    //%noE
GOM(pthread_mutex_init, iFpp)      //%noE
GOM(__pthread_mutex_lock, iFp)     //%noE
GOM(pthread_mutex_lock, iFp)       //%noE
// pthread_mutex_setprioceiling
GOM(pthread_mutex_timedlock, iFpp) //%noE
GOM(__pthread_mutex_trylock, iFp)  //%noE
GOM(pthread_mutex_trylock, iFp)    //%noE
GOM(__pthread_mutex_unlock, iFp)   //%noE
GOM(pthread_mutex_unlock, iFp)     //%noE
GOM(pthread_once, iFEpp)
GOM(__pthread_once, iFEpp)
GOM(__pthread_register_cancel, vFEp)
// __pthread_register_cancel_defer
GO(pthread_rwlockattr_destroy, vFp)
GO(pthread_rwlockattr_getkind_np, iFpp)
// pthread_rwlockattr_getpshared
GO(pthread_rwlockattr_init, iFp)
GO(pthread_rwlockattr_setkind_np, iFpi)
// pthread_rwlockattr_setpshared
// __pthread_rwlock_destroy
GOM(pthread_rwlock_destroy, iFp)    //%noE
GOM(__pthread_rwlock_init, iFpp)    //%noE
GOM(pthread_rwlock_init, iFpp)  //%noE
GO(__pthread_rwlock_rdlock, iFp)
GO(pthread_rwlock_rdlock, iFp)
// pthread_rwlock_timedrdlock
// pthread_rwlock_timedwrlock
// __pthread_rwlock_tryrdlock
GO(pthread_rwlock_tryrdlock, iFp)
// __pthread_rwlock_trywrlock
GO(pthread_rwlock_trywrlock, iFp)
GO(__pthread_rwlock_unlock, iFp)
GOM(pthread_rwlock_unlock, iFp) //%noE
GO(__pthread_rwlock_wrlock, iFp)
GOM(pthread_rwlock_wrlock, iFp) //%noE
GO(pthread_self, hFv)
GOM(pthread_setaffinity_np, iFEhup)
GO(pthread_setcancelstate, iFip)
GO(pthread_setcanceltype, iFip)
GO(pthread_setconcurrency, iFi)
GO(pthread_setname_np, iFhp)
GO(pthread_setschedparam, iFhip)
GO(pthread_setschedprio, iFhi)
GO(__pthread_setspecific, iFup)
GO(pthread_setspecific, iFup)
GO(pthread_sigmask, iFipp)
GO(pthread_spin_destroy, iFp)
GO(pthread_spin_init, iFpi)
GO(pthread_spin_lock, iFp)
GO(pthread_spin_trylock, iFp)
GO(pthread_spin_unlock, iFp)
GO(pthread_testcancel, vFv)
GO(pthread_timedjoin_np, iFppp)
GO(pthread_tryjoin_np, iFpp)
GOM(__pthread_unregister_cancel, vFEp)
// __pthread_unregister_cancel_restore
// __pthread_unwind
GOM(__pthread_unwind_next, vFEp)
GO(pthread_yield, iFv)
// raise
// __res_state
GOM(sem_close, iFp) //%noE
GOM(sem_destroy, iFp)   //%noE
GOM(sem_getvalue, iFpp) //%noE
GOM(sem_init, iFpiu)    //%noE
GOM(sem_open, pFpOM)    //%noE
GOM(sem_post, iFp)  //%noE
GOM(sem_timedwait, iFprLL_) //%noE
GOM(sem_trywait, iFp)   //%noE
GO(sem_unlink, iFp)    // no need for M here?
GOM(sem_wait, iFp)  //%noE
// __sigaction
// system
// __vfork

GOM(pthread_mutexattr_setkind_np, iFEpi)    // deprecated
