#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(pcre_assign_jit_stack, vFpp)
//DATAM(pcre_callout,   //int   (*pcre_callout)(pcre_callout_block *);
GO(pcre_compile, pFpippp)
GO(pcre_compile2, pFpipppp)
GO(pcre_config, iFip)
GO(pcre_copy_named_substring, iFpppippi)
GO(pcre_copy_substring, iFppiipi)
GO(pcre_dfa_exec, iFpppiiipipi)
GO(pcre_exec, iFpppiiipi)
DATAM(pcre_free, sizeof(void*))
GO(pcre_free_study, vFp)
GO(pcre_free_substring, vFp)
GO(pcre_free_substring_list, vFp)
GO(pcre_fullinfo, iFppip)
GO(pcre_get_named_substring, iFpppipp)
GO(pcre_get_stringnumber, iFpp)
GO(pcre_get_stringtable_entries, iFpppp)
GO(pcre_get_substring, iFppiip)
GO(pcre_get_substring_list, iFppip)
GO(pcre_info, iFppp)
GO(pcre_jit_exec, iFpppiiipip)
GO(pcre_jit_free_unused_memory, vFv)
GO(pcre_jit_stack_alloc, pFii)
GO(pcre_jit_stack_free, vFp)
GO(pcre_maketables, pFv)
//DATAM(pcre_malloc, //void *(*pcre_malloc)(size_t);
GO(pcre_pattern_to_host_byte_order, iFppp)
GO(pcre_refcount, iFpi)
//DATAM(pcre_stack_free, //void  (*pcre_stack_free)(void *);
//DATAM(pcre_stack_guard, //int   (*pcre_stack_guard)(void);
//DATAM(pcre_stack_malloc, //void *(*pcre_stack_malloc)(size_t);
GO(pcre_study, pFpip)
GO(pcre_version, pFv)
