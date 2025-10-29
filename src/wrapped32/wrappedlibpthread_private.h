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
GOM(pthread_atfork, iEEppp)
GOM(__pthread_atfork, iEEppp)
GOM(pthread_attr_destroy, iEEp)
// pthread_attr_getaffinity_np
GOM(pthread_attr_getdetachstate, iEEpp)
GOM(pthread_attr_getguardsize, iEEpp)
GOM(pthread_attr_getinheritsched, iEEpp)
GOM(pthread_attr_getschedparam, iEEpp)
GOM(pthread_attr_getschedpolicy, iEEpp)
GOM(pthread_attr_getscope, iEEpp)
GOM(pthread_attr_getstack, iEEppp)
GOM(pthread_attr_getstackaddr, iEEpp)
GOM(pthread_attr_getstacksize, iEEpp)
GOM(pthread_attr_init, iEEp)
GOM(pthread_attr_setaffinity_np, iEEpup)
GOM(pthread_attr_setdetachstate, iEEpi)
GOM(pthread_attr_setguardsize, iEEpL)
GOM(pthread_attr_setinheritsched, iEEpi)
GOM(pthread_attr_setschedparam, iEEpp)
GOM(pthread_attr_setschedpolicy, iEEpi)
GOM(pthread_attr_setscope, iEEpi)
GOM(pthread_attr_setstack, iEEppL)
GOM(pthread_attr_setstackaddr, iEEpp)
GOM(pthread_attr_setstacksize, iEEpL)
GO(pthread_barrierattr_destroy, iEp)
GO(pthread_barrierattr_getpshared, iEpp)
GO(pthread_barrierattr_init, iEp)
GO(pthread_barrierattr_setpshared, iEpi)
GO(pthread_barrier_destroy, iEp)
GO(pthread_barrier_init, iEppu)
GO(pthread_barrier_wait, iEp)
GO(pthread_cancel, iEh)
GOM(_pthread_cleanup_pop, vEEpi)
GOM(_pthread_cleanup_pop_restore, vEEpi)
GOM(_pthread_cleanup_push, vEEppp)
GOM(_pthread_cleanup_push_defer, vEEppp)
// __pthread_cleanup_routine
GO(pthread_condattr_destroy, iEp)
GO(pthread_condattr_getclock, iEpp)
GO(pthread_condattr_getpshared, iEpp)
GO(pthread_condattr_init, iEp)
GO(pthread_condattr_setclock, iEpp)
GO(pthread_condattr_setpshared, iEpi)
GO2(pthread_cond_broadcast@GLIBC_2.0, iEEp, my32_pthread_cond_broadcast_old)
GO2(pthread_cond_destroy@GLIBC_2.0, iEEp, my32_pthread_cond_destroy_old)
GO2(pthread_cond_init@GLIBC_2.0, iEEpp, my32_pthread_cond_init_old)
GO2(pthread_cond_signal@GLIBC_2.0, iEEp, my32_pthread_cond_signal_old)
GO2(pthread_cond_timedwait@GLIBC_2.0, iEEpprLL_, my32_pthread_cond_timedwait_old)
GO2(pthread_cond_wait@GLIBC_2.0, iEEpp, my32_pthread_cond_wait_old)
GOM(pthread_cond_broadcast, iEEp)
GOM(pthread_cond_destroy, iEEp)
GOM(pthread_cond_init, iEEpp)
GOM(pthread_cond_signal, iEEp)
GOM(pthread_cond_timedwait, iEEpprLL_)
GOM(pthread_cond_wait, iEEpp)
GOM(pthread_create, iEEBh_ppp)
GOM(pthread_detach, iEEh)
GO(pthread_equal, iEhh)
GO(pthread_exit, vEp)
GOM(pthread_getaffinity_np, iEEhup)
GOM(pthread_getattr_np, iEEhp)
GO(pthread_getconcurrency, iEv)
GO(pthread_getcpuclockid, iEhp)
GO(pthread_getschedparam, iEhpp)
GO(__pthread_getspecific, pEu)
GO(pthread_getspecific, pEu)
GO(pthread_getname_np, iEhpL)
GOM(__pthread_initialize, vEv)  //%noE doesn't exist anymore...
// __pthread_initialize_minimal
GO(pthread_join, iEHBp_)
GOM(__pthread_key_create, iEEpp)
GOM(pthread_key_create, iEEpp)
GO(pthread_key_delete, iEu)
GOM(pthread_kill, iEEhi)
GO2(pthread_kill@GLIBC_2.0, iEEhi, my32_pthread_kill_old)
// pthread_kill_other_threads_np
GO(__pthread_mutexattr_destroy, iEp)
GO(pthread_mutexattr_destroy, iEp)
GO(pthread_mutexattr_getprioceiling, iEpp)
GO(pthread_mutexattr_getprotocol, iEpp)
// pthread_mutexattr_getpshared
// pthread_mutexattr_getrobust_np
GO(pthread_mutexattr_gettype, iEpp)
GO2(__pthread_mutexattr_init, iEp, my32_pthread_mutexattr_init)
GOM(pthread_mutexattr_init, iEEp)
GO(pthread_mutexattr_setprioceiling, iEpi)
GO(pthread_mutexattr_setprotocol, iEpp)
GO(pthread_mutexattr_setpshared, iEpi)
// pthread_mutexattr_setrobust_np
GO(__pthread_mutexattr_settype, iEpi)
GO(pthread_mutexattr_settype, iEpi)
// pthread_mutex_consistent_np
GOM(__pthread_mutex_destroy, iEp)  //%noE
GOM(pthread_mutex_destroy, iEp)    //%noE
// pthread_mutex_getprioceiling
GOM(__pthread_mutex_init, iEpp)    //%noE
GOM(pthread_mutex_init, iEpp)      //%noE
GOM(__pthread_mutex_lock, iEp)     //%noE
GOM(pthread_mutex_lock, iEp)       //%noE
// pthread_mutex_setprioceiling
GOM(pthread_mutex_timedlock, iEpp) //%noE
GOM(__pthread_mutex_trylock, iEp)  //%noE
GOM(pthread_mutex_trylock, iEp)    //%noE
GOM(__pthread_mutex_unlock, iEp)   //%noE
GOM(pthread_mutex_unlock, iEp)     //%noE
GOM(pthread_once, iEEpp)
GOM(__pthread_once, iEEpp)
GOM(__pthread_register_cancel, vEEp)
// __pthread_register_cancel_defer
GO(pthread_rwlockattr_destroy, vEp)
GO(pthread_rwlockattr_getkind_np, iEpp)
// pthread_rwlockattr_getpshared
GO(pthread_rwlockattr_init, iEp)
GO(pthread_rwlockattr_setkind_np, iEpi)
// pthread_rwlockattr_setpshared
// __pthread_rwlock_destroy
GOM(pthread_rwlock_destroy, iEp)    //%noE
GOM(__pthread_rwlock_init, iEpp)    //%noE
GOM(pthread_rwlock_init, iEpp)  //%noE
GO(__pthread_rwlock_rdlock, iEp)
GO(pthread_rwlock_rdlock, iEp)
// pthread_rwlock_timedrdlock
// pthread_rwlock_timedwrlock
// __pthread_rwlock_tryrdlock
GO(pthread_rwlock_tryrdlock, iEp)
// __pthread_rwlock_trywrlock
GO(pthread_rwlock_trywrlock, iEp)
GO(__pthread_rwlock_unlock, iEp)
GOM(pthread_rwlock_unlock, iEp) //%noE
GO(__pthread_rwlock_wrlock, iEp)
GOM(pthread_rwlock_wrlock, iEp) //%noE
GO(pthread_self, hEv)
GOM(pthread_setaffinity_np, iEEhup)
GO(pthread_setcancelstate, iEip)
GO(pthread_setcanceltype, iEip)
GO(pthread_setconcurrency, iEi)
GO(pthread_setname_np, iEhp)
GO(pthread_setschedparam, iEhip)
GO(pthread_setschedprio, iEhi)
GO(__pthread_setspecific, iEup)
GO(pthread_setspecific, iEup)
GO(pthread_sigmask, iEipp)
GO(pthread_spin_destroy, iEp)
GO(pthread_spin_init, iEpi)
GO(pthread_spin_lock, iEp)
GO(pthread_spin_trylock, iEp)
GO(pthread_spin_unlock, iEp)
GO(pthread_testcancel, vEv)
GO(pthread_timedjoin_np, iEhpp) // will leak handle
GO(pthread_tryjoin_np, iEhp)    // will leak handle
GOM(__pthread_unregister_cancel, vEEp)
// __pthread_unregister_cancel_restore
// __pthread_unwind
GOM(__pthread_unwind_next, vEEp)
GO(pthread_yield, iEv)
// raise
// __res_state
GOM(sem_close, iEp) //%noE
GOM(sem_destroy, iEp)   //%noE
GOM(sem_getvalue, iEpp) //%noE
GOM(sem_init, iEpiu)    //%noE
GOM(sem_open, pEpOM)    //%noE
GOM(sem_post, iEp)  //%noE
GOM(sem_timedwait, iEprLL_) //%noE
GOM(sem_trywait, iEp)   //%noE
GO(sem_unlink, iEp)    // no need for M here?
GOM(sem_wait, iEp)  //%noE
// __sigaction
// system
// __vfork

GOM(pthread_mutexattr_setkind_np, iEEpi)    // deprecated
