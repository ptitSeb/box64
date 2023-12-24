#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(copy_bitmask_to_bitmask, vFpp)
GO(copy_bitmask_to_nodemask, vFpp)
GO(copy_nodemask_to_bitmask, vFpp)
GOW(get_mempolicy, lFppLpL)
GOW(mbind, iFpLipLu)
//GOW(migrate_pages, 
//GOW(move_pages, 
DATAB(numa_all_cpus_ptr, sizeof(void*))
//DATAB(numa_all_nodes, 
DATAB(numa_all_nodes_ptr, sizeof(void*))
GO(numa_alloc, pFL)
GO(numa_allocate_cpumask, pFv)
GO(numa_allocate_nodemask, pFv)
GO(numa_alloc_interleaved, pFL)
GO(numa_alloc_interleaved_subset, pFLp)
//GO2(numa_alloc_interleaved_subset@libnuma_1.1, 
GO(numa_alloc_local, pFL)
GO(numa_alloc_onnode, pFLi)
GO(numa_available, iFv)
GO(numa_bind, vFp)
//GO2(numa_bind@libnuma_1.1, 
GO(numa_bitmask_alloc, pFu)
GO(numa_bitmask_clearall, pFp)
GO(numa_bitmask_clearbit, pFpu)
GO(numa_bitmask_equal, iFpp)
GO(numa_bitmask_free, vFp)
GO(numa_bitmask_isbitset, iFpu)
GO(numa_bitmask_nbytes, uFp)
GO(numa_bitmask_setall, pFp)
GO(numa_bitmask_setbit, pFpu)
GO(numa_bitmask_weight, uFp)
GO(numa_distance, iFii)
GOW(numa_error, vFp)
DATAB(numa_exit_on_error, sizeof(int))
DATAB(numa_exit_on_warn, sizeof(int))
GO(numa_free, vFpL)
//GO2(numa_get_interleave_mask@libnuma_1.1, 
GO(numa_get_interleave_mask, pFv)
GO(numa_get_interleave_node, iFv)
GO(numa_get_membind, pFv)
//GO2(numa_get_membind@libnuma_1.1, 
GO(numa_get_mems_allowed, pFv)
//GO2(numa_get_run_node_mask@libnuma_1.1, 
GO(numa_get_run_node_mask, pFv)
//GO(numa_has_preferred_many, 
//GO2(numa_interleave_memory@libnuma_1.1, 
GO(numa_interleave_memory, vFpLp)
GO(numa_max_node, iFv)
GO(numa_max_possible_node, iFv)
GO(numa_migrate_pages, iFipp)
GO(numa_move_pages, iFiLpppi)
GO(numa_node_of_cpu, iFi)
GO(numa_node_size, lFip)
GO(numa_node_size64, IFip)
//DATAB(numa_nodes_ptr, 
//GO2(numa_node_to_cpus@libnuma_1.1, 
GO(numa_node_to_cpus, iFip)
//GO(numa_node_to_cpu_update, 
//DATAB(numa_no_nodes, 
DATAB(numa_no_nodes_ptr, sizeof(void*))
GO(numa_num_configured_cpus, iFv)
GO(numa_num_configured_nodes, iFv)
//GO(numa_num_possible_cpus, 
GO(numa_num_possible_nodes, iFv)
GO(numa_num_task_cpus, iFv)
GO(numa_num_task_nodes, iFv)
//GO(numa_num_thread_cpus, 
//GO(numa_num_thread_nodes, 
//GO(numa_pagesize, 
GO(numa_parse_bitmap, iFpp)
//GO2(numa_parse_bitmap@libnuma_1.1, 
GO(numa_parse_cpustring, pFp)
//GO(numa_parse_cpustring_all, 
GO(numa_parse_nodestring, pFp)
//GO(numa_parse_nodestring_all, 
GO(numa_police_memory, vFpL)
GO(numa_preferred, iFv)
//GO(numa_preferred_many, 
GO(numa_realloc, pFpLL)
GO(numa_run_on_node, iFi)
GO(numa_run_on_node_mask, iFp)
//GO2(numa_run_on_node_mask@libnuma_1.1, 
//GO(numa_run_on_node_mask_all, 
GO(numa_sched_getaffinity, iFip)
//GO2(numa_sched_getaffinity@libnuma_1.1, 
GO(numa_sched_setaffinity, iFip)
//GO2(numa_sched_setaffinity@libnuma_1.1, 
GO(numa_set_bind_policy, vFi)
GO(numa_set_interleave_mask, vFp)
//GO2(numa_set_interleave_mask@libnuma_1.1, 
GO(numa_set_localalloc, vFv)
GO(numa_setlocal_memory, vFpL)
GO(numa_set_membind, vFp)
//GO2(numa_set_membind@libnuma_1.1, 
//GO(numa_set_membind_balancing, 
GO(numa_set_preferred, vFi)
//GO(numa_set_preferred_many, 
GO(numa_set_strict, vFi)
GO(numa_tonodemask_memory, vFpLp)
//GO2(numa_tonodemask_memory@libnuma_1.1, 
GO(numa_tonode_memory, vFpLi)
GOW(numa_warn, vFippppppppppppp)    //use VAARG with printf style, should wrap if used
//GOW(set_mempolicy, 
