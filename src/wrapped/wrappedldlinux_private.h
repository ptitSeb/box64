#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

// _dl_allocate_tls
// _dl_allocate_tls_init
// _dl_argv //type B
// _dl_cache_libcmp
// _dl_deallocate_tls
// _dl_debug_state
// _dl_get_tls_static_info
// _dl_make_stack_executable
// _dl_mcount
// _dl_rtld_di_serinfo
// _dl_tls_setup
DATA(__libc_enable_secure, sizeof(void*))
DATAM(__libc_stack_end, sizeof(void*))
#ifdef STATICBUILD
//DATAB(_r_debug, 40)
//DATA(__pointer_chk_guard, sizeof(void*))
//DATA(_rtld_global, sizeof(void*))
//DATA(_rtld_global_ro, sizeof(void*))
#else
DATAB(_r_debug, 40)
DATA(__pointer_chk_guard, sizeof(void*))
DATA(_rtld_global, sizeof(void*))
DATA(_rtld_global_ro, sizeof(void*))
#endif
DATA(__stack_chk_guard, sizeof(void*))
GOM(__tls_get_addr, pFEp)
