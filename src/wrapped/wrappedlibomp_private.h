// SPDX-License-Identifier: MIT
#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif
//GO(GOMP_alloc,
GO(GOMP_atomic_end, vFv)
GO(GOMP_atomic_start, vFv)
GO(GOMP_barrier, vFv)
GO(GOMP_barrier_cancel, iFv)
GO(GOMP_cancel, iFii)
//GO(GOMP_cancellation_point,
GO(GOMP_critical_end, vFv)
GO(GOMP_critical_name_end, vFp)
GO(GOMP_critical_name_start, vFp)
GO(GOMP_critical_start, vFv)
//GO(GOMP_doacross_post,
//GO(GOMP_doacross_ull_post,
//GO(GOMP_doacross_ull_wait,
//GO(GOMP_doacross_wait,
//GO(GOMP_free,
//GO(GOMP_loop_doacross_dynamic_start,
//GO(GOMP_loop_doacross_guided_start,
//GO(GOMP_loop_doacross_runtime_start,
//GO(GOMP_loop_doacross_start,
//GO(GOMP_loop_doacross_static_start,
GO(GOMP_loop_dynamic_next, iFpp)
GO(GOMP_loop_dynamic_start, iFllllpp)
GO(GOMP_loop_end, vFv)
//GO(GOMP_loop_end_cancel,
GO(GOMP_loop_end_nowait, vFv)
//GO(GOMP_loop_guided_next,
//GO(GOMP_loop_guided_start,
//GO(GOMP_loop_maybe_nonmonotonic_runtime_next,
//GO(GOMP_loop_maybe_nonmonotonic_runtime_start,
//GO(GOMP_loop_nonmonotonic_dynamic_next,
//GO(GOMP_loop_nonmonotonic_dynamic_start,
//GO(GOMP_loop_nonmonotonic_guided_next,
//GO(GOMP_loop_nonmonotonic_guided_start,
//GO(GOMP_loop_nonmonotonic_runtime_next,
//GO(GOMP_loop_nonmonotonic_runtime_start,
GO(GOMP_loop_ordered_dynamic_next, iFpp)
GO(GOMP_loop_ordered_dynamic_start, iFllllpp)
//GO(GOMP_loop_ordered_guided_next,
//GO(GOMP_loop_ordered_guided_start,
//GO(GOMP_loop_ordered_runtime_next,
//GO(GOMP_loop_ordered_runtime_start,
//GO(GOMP_loop_ordered_start,
//GO(GOMP_loop_ordered_static_next,
//GO(GOMP_loop_ordered_static_start,
//GO(GOMP_loop_runtime_next,
//GO(GOMP_loop_runtime_start,
//GO(GOMP_loop_start,
//GO(GOMP_loop_static_next,
//GO(GOMP_loop_static_start,
//GO(GOMP_loop_ull_doacross_dynamic_start,
//GO(GOMP_loop_ull_doacross_guided_start,
//GO(GOMP_loop_ull_doacross_runtime_start,
//GO(GOMP_loop_ull_doacross_start,
//GO(GOMP_loop_ull_doacross_static_start,
//GO(GOMP_loop_ull_dynamic_next,
//GO(GOMP_loop_ull_dynamic_start,
//GO(GOMP_loop_ull_guided_next,
//GO(GOMP_loop_ull_guided_start,
//GO(GOMP_loop_ull_maybe_nonmonotonic_runtime_next,
//GO(GOMP_loop_ull_maybe_nonmonotonic_runtime_start,
//GO(GOMP_loop_ull_nonmonotonic_dynamic_next,
//GO(GOMP_loop_ull_nonmonotonic_dynamic_start,
//GO(GOMP_loop_ull_nonmonotonic_guided_next,
//GO(GOMP_loop_ull_nonmonotonic_guided_start,
//GO(GOMP_loop_ull_nonmonotonic_runtime_next,
//GO(GOMP_loop_ull_nonmonotonic_runtime_start,
//GO(GOMP_loop_ull_ordered_dynamic_next,
//GO(GOMP_loop_ull_ordered_dynamic_start,
//GO(GOMP_loop_ull_ordered_guided_next,
//GO(GOMP_loop_ull_ordered_guided_start,
//GO(GOMP_loop_ull_ordered_runtime_next,
//GO(GOMP_loop_ull_ordered_runtime_start,
//GO(GOMP_loop_ull_ordered_start,
//GO(GOMP_loop_ull_ordered_static_next,
//GO(GOMP_loop_ull_ordered_static_start,
//GO(GOMP_loop_ull_runtime_next,
//GO(GOMP_loop_ull_runtime_start,
//GO(GOMP_loop_ull_start,
//GO(GOMP_loop_ull_static_next,
//GO(GOMP_loop_ull_static_start,
GO(GOMP_ordered_end, vFv)
GO(GOMP_ordered_start, vFv)
GOM(GOMP_parallel, vFEppuu)
//GO(GOMP_parallel_end,
//GO(GOMP_parallel_loop_dynamic,
//GO(GOMP_parallel_loop_dynamic_start,
//GO(GOMP_parallel_loop_guided,
//GO(GOMP_parallel_loop_guided_start,
//GO(GOMP_parallel_loop_maybe_nonmonotonic_runtime,
//GO(GOMP_parallel_loop_nonmonotonic_dynamic,
//GO(GOMP_parallel_loop_nonmonotonic_guided,
//GO(GOMP_parallel_loop_nonmonotonic_runtime,
//GO(GOMP_parallel_loop_runtime,
//GO(GOMP_parallel_loop_runtime_start,
//GO(GOMP_parallel_loop_static,
//GO(GOMP_parallel_loop_static_start,
//GO(GOMP_parallel_reductions,
//GO(GOMP_parallel_sections,
//GO(GOMP_parallel_sections_start,
//GO(GOMP_parallel_start,
//GO(GOMP_sections2_start,
//GO(GOMP_sections_end,
//GO(GOMP_sections_end_cancel,
GO(GOMP_sections_end_nowait, vFv)
GO(GOMP_sections_next, uFv)
GO(GOMP_sections_start, uFu)
//GO(GOMP_single_copy_end,
//GO(GOMP_single_copy_start,
GO(GOMP_single_start, iFv)
//GO(GOMP_target,
//GO(GOMP_target_data,
//GO(GOMP_target_end_data,
//GO(GOMP_target_update,
GOM(GOMP_task, vFEppplliupi)
//GO(GOMP_task_reduction_remap,
//GO(GOMP_taskgroup_end,
//GO(GOMP_taskgroup_reduction_register,
//GO(GOMP_taskgroup_reduction_unregister,
//GO(GOMP_taskgroup_start,
//GO(GOMP_taskloop,
//GO(GOMP_taskloop_ull,
//GO(GOMP_taskwait,
//GO(GOMP_taskwait_depend,
//GO(GOMP_taskyield,
//GO(GOMP_teams,
//GO(GOMP_teams_reg,
//GO(GOMP_workshare_task_reduction_unregister,
//GO(___kmp_allocate,
//GO(___kmp_free,
//GO(__kmp_acquire_drdpa_lock,
//GO(__kmp_acquire_nested_drdpa_lock,
//GO(__kmp_acquire_nested_queuing_lock,
//GO(__kmp_acquire_nested_tas_lock,
//GO(__kmp_acquire_nested_ticket_lock,
//GO(__kmp_acquire_queuing_lock,
//GO(__kmp_acquire_tas_lock,
//GO(__kmp_acquire_ticket_lock,
//GO(__kmp_fork_call,
//GO(__kmp_get_reduce_method,
//GO(__kmp_invoke_microtask,
//GO(__kmp_itt_fini_ittlib,
//GO(__kmp_itt_init_ittlib,
//GO(__kmp_reap_monitor,
//GO(__kmp_reap_worker,
//GO(__kmp_release_64,
//GO(__kmp_reset_stats,
//GO(__kmp_wait_4,
//GO(__kmp_wait_64,
//GO(__kmpc_aligned_alloc,
//GO(__kmpc_alloc,
//GO(__kmpc_atomic_1,
//GO(__kmpc_atomic_16,
//GO(__kmpc_atomic_2,
//GO(__kmpc_atomic_32,
//GO(__kmpc_atomic_4,
//GO(__kmpc_atomic_8,
//GO(__kmpc_atomic_cmplx10_add_cpt,
//GO(__kmpc_atomic_cmplx10_div_cpt,
//GO(__kmpc_atomic_cmplx10_mul_cpt,
//GO(__kmpc_atomic_cmplx10_rd,
//GO(__kmpc_atomic_cmplx10_sub_cpt,
//GO(__kmpc_atomic_cmplx10_wr,
//GO(__kmpc_atomic_cmplx4_add,
//GO(__kmpc_atomic_cmplx4_add_cmplx8,
//GO(__kmpc_atomic_cmplx4_add_cpt,
//GO(__kmpc_atomic_cmplx4_div,
//GO(__kmpc_atomic_cmplx4_div_cmplx8,
//GO(__kmpc_atomic_cmplx4_div_cpt,
//GO(__kmpc_atomic_cmplx4_mul,
//GO(__kmpc_atomic_cmplx4_mul_cmplx8,
//GO(__kmpc_atomic_cmplx4_mul_cpt,
//GO(__kmpc_atomic_cmplx4_rd,
//GO(__kmpc_atomic_cmplx4_sub,
//GO(__kmpc_atomic_cmplx4_sub_cmplx8,
//GO(__kmpc_atomic_cmplx4_sub_cpt,
//GO(__kmpc_atomic_cmplx4_wr,
//GO(__kmpc_atomic_cmplx8_add,
//GO(__kmpc_atomic_cmplx8_add_cpt,
//GO(__kmpc_atomic_cmplx8_div,
//GO(__kmpc_atomic_cmplx8_div_cpt,
//GO(__kmpc_atomic_cmplx8_mul,
//GO(__kmpc_atomic_cmplx8_mul_cpt,
//GO(__kmpc_atomic_cmplx8_rd,
//GO(__kmpc_atomic_cmplx8_sub,
//GO(__kmpc_atomic_cmplx8_sub_cpt,
//GO(__kmpc_atomic_cmplx8_wr,
//GO(__kmpc_atomic_end,
//GO(__kmpc_atomic_fixed1_add,
//GO(__kmpc_atomic_fixed1_add_cpt,
//GO(__kmpc_atomic_fixed1_andb,
//GO(__kmpc_atomic_fixed1_andb_cpt,
//GO(__kmpc_atomic_fixed1_andl,
//GO(__kmpc_atomic_fixed1_andl_cpt,
//GO(__kmpc_atomic_fixed1_div,
//GO(__kmpc_atomic_fixed1_div_cpt,
//GO(__kmpc_atomic_fixed1_div_float8,
//GO(__kmpc_atomic_fixed1_eqv,
//GO(__kmpc_atomic_fixed1_eqv_cpt,
//GO(__kmpc_atomic_fixed1_max,
//GO(__kmpc_atomic_fixed1_max_cpt,
//GO(__kmpc_atomic_fixed1_min,
//GO(__kmpc_atomic_fixed1_min_cpt,
//GO(__kmpc_atomic_fixed1_mul,
//GO(__kmpc_atomic_fixed1_mul_cpt,
//GO(__kmpc_atomic_fixed1_mul_float8,
//GO(__kmpc_atomic_fixed1_neqv,
//GO(__kmpc_atomic_fixed1_neqv_cpt,
//GO(__kmpc_atomic_fixed1_orb,
//GO(__kmpc_atomic_fixed1_orb_cpt,
//GO(__kmpc_atomic_fixed1_orl,
//GO(__kmpc_atomic_fixed1_orl_cpt,
//GO(__kmpc_atomic_fixed1_rd,
//GO(__kmpc_atomic_fixed1_shl,
//GO(__kmpc_atomic_fixed1_shl_cpt,
//GO(__kmpc_atomic_fixed1_shr,
//GO(__kmpc_atomic_fixed1_shr_cpt,
//GO(__kmpc_atomic_fixed1_sub,
//GO(__kmpc_atomic_fixed1_sub_cpt,
//GO(__kmpc_atomic_fixed1_wr,
//GO(__kmpc_atomic_fixed1_xor,
//GO(__kmpc_atomic_fixed1_xor_cpt,
//GO(__kmpc_atomic_fixed1u_div,
//GO(__kmpc_atomic_fixed1u_div_cpt,
//GO(__kmpc_atomic_fixed1u_shr,
//GO(__kmpc_atomic_fixed1u_shr_cpt,
//GO(__kmpc_atomic_fixed2_add,
//GO(__kmpc_atomic_fixed2_add_cpt,
//GO(__kmpc_atomic_fixed2_andb,
//GO(__kmpc_atomic_fixed2_andb_cpt,
//GO(__kmpc_atomic_fixed2_andl,
//GO(__kmpc_atomic_fixed2_andl_cpt,
//GO(__kmpc_atomic_fixed2_div,
//GO(__kmpc_atomic_fixed2_div_cpt,
//GO(__kmpc_atomic_fixed2_div_float8,
//GO(__kmpc_atomic_fixed2_eqv,
//GO(__kmpc_atomic_fixed2_eqv_cpt,
//GO(__kmpc_atomic_fixed2_max,
//GO(__kmpc_atomic_fixed2_max_cpt,
//GO(__kmpc_atomic_fixed2_min,
//GO(__kmpc_atomic_fixed2_min_cpt,
//GO(__kmpc_atomic_fixed2_mul,
//GO(__kmpc_atomic_fixed2_mul_cpt,
//GO(__kmpc_atomic_fixed2_mul_float8,
//GO(__kmpc_atomic_fixed2_neqv,
//GO(__kmpc_atomic_fixed2_neqv_cpt,
//GO(__kmpc_atomic_fixed2_orb,
//GO(__kmpc_atomic_fixed2_orb_cpt,
//GO(__kmpc_atomic_fixed2_orl,
//GO(__kmpc_atomic_fixed2_orl_cpt,
//GO(__kmpc_atomic_fixed2_rd,
//GO(__kmpc_atomic_fixed2_shl,
//GO(__kmpc_atomic_fixed2_shl_cpt,
//GO(__kmpc_atomic_fixed2_shr,
//GO(__kmpc_atomic_fixed2_shr_cpt,
//GO(__kmpc_atomic_fixed2_sub,
//GO(__kmpc_atomic_fixed2_sub_cpt,
//GO(__kmpc_atomic_fixed2_wr,
//GO(__kmpc_atomic_fixed2_xor,
//GO(__kmpc_atomic_fixed2_xor_cpt,
//GO(__kmpc_atomic_fixed2u_div,
//GO(__kmpc_atomic_fixed2u_div_cpt,
//GO(__kmpc_atomic_fixed2u_shr,
//GO(__kmpc_atomic_fixed2u_shr_cpt,
//GO(__kmpc_atomic_fixed4_add,
//GO(__kmpc_atomic_fixed4_add_cpt,
//GO(__kmpc_atomic_fixed4_andb,
//GO(__kmpc_atomic_fixed4_andb_cpt,
//GO(__kmpc_atomic_fixed4_andl,
//GO(__kmpc_atomic_fixed4_andl_cpt,
//GO(__kmpc_atomic_fixed4_div,
//GO(__kmpc_atomic_fixed4_div_cpt,
//GO(__kmpc_atomic_fixed4_div_float8,
//GO(__kmpc_atomic_fixed4_eqv,
//GO(__kmpc_atomic_fixed4_eqv_cpt,
//GO(__kmpc_atomic_fixed4_max,
//GO(__kmpc_atomic_fixed4_max_cpt,
//GO(__kmpc_atomic_fixed4_min,
//GO(__kmpc_atomic_fixed4_min_cpt,
//GO(__kmpc_atomic_fixed4_mul,
//GO(__kmpc_atomic_fixed4_mul_cpt,
//GO(__kmpc_atomic_fixed4_mul_float8,
//GO(__kmpc_atomic_fixed4_neqv,
//GO(__kmpc_atomic_fixed4_neqv_cpt,
//GO(__kmpc_atomic_fixed4_orb,
//GO(__kmpc_atomic_fixed4_orb_cpt,
//GO(__kmpc_atomic_fixed4_orl,
//GO(__kmpc_atomic_fixed4_orl_cpt,
//GO(__kmpc_atomic_fixed4_rd,
//GO(__kmpc_atomic_fixed4_shl,
//GO(__kmpc_atomic_fixed4_shl_cpt,
//GO(__kmpc_atomic_fixed4_shr,
//GO(__kmpc_atomic_fixed4_shr_cpt,
//GO(__kmpc_atomic_fixed4_sub,
//GO(__kmpc_atomic_fixed4_sub_cpt,
//GO(__kmpc_atomic_fixed4_wr,
//GO(__kmpc_atomic_fixed4_xor,
//GO(__kmpc_atomic_fixed4_xor_cpt,
//GO(__kmpc_atomic_fixed4u_div,
//GO(__kmpc_atomic_fixed4u_div_cpt,
//GO(__kmpc_atomic_fixed4u_shr,
//GO(__kmpc_atomic_fixed4u_shr_cpt,
//GO(__kmpc_atomic_fixed8_add,
//GO(__kmpc_atomic_fixed8_add_cpt,
//GO(__kmpc_atomic_fixed8_andb,
//GO(__kmpc_atomic_fixed8_andb_cpt,
//GO(__kmpc_atomic_fixed8_andl,
//GO(__kmpc_atomic_fixed8_andl_cpt,
//GO(__kmpc_atomic_fixed8_div,
//GO(__kmpc_atomic_fixed8_div_cpt,
//GO(__kmpc_atomic_fixed8_div_float8,
//GO(__kmpc_atomic_fixed8_eqv,
//GO(__kmpc_atomic_fixed8_eqv_cpt,
//GO(__kmpc_atomic_fixed8_max,
//GO(__kmpc_atomic_fixed8_max_cpt,
//GO(__kmpc_atomic_fixed8_min,
//GO(__kmpc_atomic_fixed8_min_cpt,
//GO(__kmpc_atomic_fixed8_mul,
//GO(__kmpc_atomic_fixed8_mul_cpt,
//GO(__kmpc_atomic_fixed8_mul_float8,
//GO(__kmpc_atomic_fixed8_neqv,
//GO(__kmpc_atomic_fixed8_neqv_cpt,
//GO(__kmpc_atomic_fixed8_orb,
//GO(__kmpc_atomic_fixed8_orb_cpt,
//GO(__kmpc_atomic_fixed8_orl,
//GO(__kmpc_atomic_fixed8_orl_cpt,
//GO(__kmpc_atomic_fixed8_rd,
//GO(__kmpc_atomic_fixed8_shl,
//GO(__kmpc_atomic_fixed8_shl_cpt,
//GO(__kmpc_atomic_fixed8_shr,
//GO(__kmpc_atomic_fixed8_shr_cpt,
//GO(__kmpc_atomic_fixed8_sub,
//GO(__kmpc_atomic_fixed8_sub_cpt,
//GO(__kmpc_atomic_fixed8_wr,
//GO(__kmpc_atomic_fixed8_xor,
//GO(__kmpc_atomic_fixed8_xor_cpt,
//GO(__kmpc_atomic_fixed8u_div,
//GO(__kmpc_atomic_fixed8u_div_cpt,
//GO(__kmpc_atomic_fixed8u_shr,
//GO(__kmpc_atomic_fixed8u_shr_cpt,
//GO(__kmpc_atomic_float10_add_cpt,
//GO(__kmpc_atomic_float10_div_cpt,
//GO(__kmpc_atomic_float10_max_cpt,
//GO(__kmpc_atomic_float10_min_cpt,
//GO(__kmpc_atomic_float10_mul_cpt,
//GO(__kmpc_atomic_float10_rd,
//GO(__kmpc_atomic_float10_sub_cpt,
//GO(__kmpc_atomic_float10_wr,
//GO(__kmpc_atomic_float4_add,
//GO(__kmpc_atomic_float4_add_cpt,
//GO(__kmpc_atomic_float4_add_float8,
//GO(__kmpc_atomic_float4_div,
//GO(__kmpc_atomic_float4_div_cpt,
//GO(__kmpc_atomic_float4_div_float8,
//GO(__kmpc_atomic_float4_max,
//GO(__kmpc_atomic_float4_max_cpt,
//GO(__kmpc_atomic_float4_min,
//GO(__kmpc_atomic_float4_min_cpt,
//GO(__kmpc_atomic_float4_mul,
//GO(__kmpc_atomic_float4_mul_cpt,
//GO(__kmpc_atomic_float4_mul_float8,
//GO(__kmpc_atomic_float4_rd,
//GO(__kmpc_atomic_float4_sub,
//GO(__kmpc_atomic_float4_sub_cpt,
//GO(__kmpc_atomic_float4_sub_float8,
//GO(__kmpc_atomic_float4_wr,
//GO(__kmpc_atomic_float8_add,
//GO(__kmpc_atomic_float8_add_cpt,
//GO(__kmpc_atomic_float8_div,
//GO(__kmpc_atomic_float8_div_cpt,
//GO(__kmpc_atomic_float8_max,
//GO(__kmpc_atomic_float8_max_cpt,
//GO(__kmpc_atomic_float8_min,
//GO(__kmpc_atomic_float8_min_cpt,
//GO(__kmpc_atomic_float8_mul,
//GO(__kmpc_atomic_float8_mul_cpt,
//GO(__kmpc_atomic_float8_rd,
//GO(__kmpc_atomic_float8_sub,
//GO(__kmpc_atomic_float8_sub_cpt,
//GO(__kmpc_atomic_float8_wr,
//GO(__kmpc_atomic_start,
//GO(__kmpc_barrier,
//GO(__kmpc_barrier_master,
//GO(__kmpc_barrier_master_nowait,
//GO(__kmpc_begin,
//GO(__kmpc_bound_num_threads,
//GO(__kmpc_bound_thread_num,
//GO(__kmpc_calc_original_ivs_rectang,
//GO(__kmpc_calloc,
//GO(__kmpc_cancel,
//GO(__kmpc_cancel_barrier,
//GO(__kmpc_cancellationpoint,
//GO(__kmpc_copyprivate,
//GO(__kmpc_copyprivate_light,
//GO(__kmpc_critical,
//GO(__kmpc_critical_with_hint,
//GO(__kmpc_destroy_allocator,
//GO(__kmpc_destroy_lock,
//GO(__kmpc_destroy_nest_lock,
//GO(__kmpc_dispatch_deinit,
//GO(__kmpc_dispatch_fini_4,
//GO(__kmpc_dispatch_fini_4u,
//GO(__kmpc_dispatch_fini_8,
//GO(__kmpc_dispatch_fini_8u,
//GO(__kmpc_dispatch_init_4,
//GO(__kmpc_dispatch_init_4u,
//GO(__kmpc_dispatch_init_8,
//GO(__kmpc_dispatch_init_8u,
//GO(__kmpc_dispatch_next_4,
//GO(__kmpc_dispatch_next_4u,
//GO(__kmpc_dispatch_next_8,
//GO(__kmpc_dispatch_next_8u,
//GO(__kmpc_dist_dispatch_init_4,
//GO(__kmpc_dist_dispatch_init_4u,
//GO(__kmpc_dist_dispatch_init_8,
//GO(__kmpc_dist_dispatch_init_8u,
//GO(__kmpc_dist_for_static_init_4,
//GO(__kmpc_dist_for_static_init_4u,
//GO(__kmpc_dist_for_static_init_8,
//GO(__kmpc_dist_for_static_init_8u,
//GO(__kmpc_doacross_fini,
//GO(__kmpc_doacross_init,
//GO(__kmpc_doacross_post,
//GO(__kmpc_doacross_wait,
//GO(__kmpc_end,
//GO(__kmpc_end_barrier_master,
//GO(__kmpc_end_critical,
//GO(__kmpc_end_masked,
//GO(__kmpc_end_master,
//GO(__kmpc_end_ordered,
//GO(__kmpc_end_reduce,
//GO(__kmpc_end_reduce_nowait,
//GO(__kmpc_end_scope,
//GO(__kmpc_end_sections,
//GO(__kmpc_end_serialized_parallel,
//GO(__kmpc_end_single,
//GO(__kmpc_end_taskgroup,
//GO(__kmpc_error,
//GO(__kmpc_flush,
//GO(__kmpc_for_collapsed_init,
//GO(__kmpc_for_static_fini,
//GO(__kmpc_for_static_init_4,
//GO(__kmpc_for_static_init_4u,
//GO(__kmpc_for_static_init_8,
//GO(__kmpc_for_static_init_8u,
//GO(__kmpc_fork_call,
//GO(__kmpc_fork_call_if,
//GO(__kmpc_fork_teams,
//GO(__kmpc_free,
//GO(__kmpc_get_default_allocator,
//GO(__kmpc_get_parent_taskid,
//GO(__kmpc_get_target_offload,
//GO(__kmpc_get_taskid,
//GO(__kmpc_global_num_threads,
//GO(__kmpc_global_thread_num,
//GO(__kmpc_in_parallel,
//GO(__kmpc_init_allocator,
//GO(__kmpc_init_lock,
//GO(__kmpc_init_lock_with_hint,
//GO(__kmpc_init_nest_lock,
//GO(__kmpc_init_nest_lock_with_hint,
//GO(__kmpc_invoke_task_func,
//GO(__kmpc_masked,
//GO(__kmpc_master,
//GO(__kmpc_next_section,
//GO(__kmpc_ok_to_fork,
//GO(__kmpc_omp_get_target_async_handle_ptr,
//GO(__kmpc_omp_has_task_team,
//GO(__kmpc_omp_reg_task_with_affinity,
//GO(__kmpc_omp_target_task_alloc,
//GO(__kmpc_omp_task,
//GO(__kmpc_omp_task_alloc,
//GO(__kmpc_omp_task_begin_if0,
//GO(__kmpc_omp_task_complete_if0,
//GO(__kmpc_omp_task_parts,
//GO(__kmpc_omp_task_with_deps,
//GO(__kmpc_omp_taskwait,
//GO(__kmpc_omp_taskwait_deps_51,
//GO(__kmpc_omp_taskyield,
//GO(__kmpc_omp_wait_deps,
//GO(__kmpc_ordered,
//GO(__kmpc_pause_resource,
//GO(__kmpc_pop_num_threads,
//GO(__kmpc_process_loop_nest_rectang,
//GO(__kmpc_proxy_task_completed,
//GO(__kmpc_proxy_task_completed_ooo,
//GO(__kmpc_push_num_teams,
//GO(__kmpc_push_num_teams_51,
//GO(__kmpc_push_num_threads,
//GO(__kmpc_push_num_threads_list,
//GO(__kmpc_push_num_threads_list_strict,
//GO(__kmpc_push_num_threads_strict,
//GO(__kmpc_push_proc_bind,
//GO(__kmpc_realloc,
//GO(__kmpc_reduce,
//GO(__kmpc_reduce_nowait,
//GO(__kmpc_scope,
//GO(__kmpc_sections_init,
//GO(__kmpc_serialized_parallel,
//GO(__kmpc_set_default_allocator,
//GO(__kmpc_set_lock,
//GO(__kmpc_set_nest_lock,
//GO(__kmpc_set_thread_limit,
//GO(__kmpc_single,
//GO(__kmpc_task_allow_completion_event,
//GO(__kmpc_task_get_depnode,
//GO(__kmpc_task_get_successors,
//GO(__kmpc_task_reduction_get_th_data,
//GO(__kmpc_task_reduction_init,
//GO(__kmpc_task_reduction_modifier_fini,
//GO(__kmpc_task_reduction_modifier_init,
//GO(__kmpc_taskgroup,
//GO(__kmpc_taskloop,
//GO(__kmpc_taskloop_5,
//GO(__kmpc_taskred_init,
//GO(__kmpc_taskred_modifier_init,
//GO(__kmpc_team_static_init_4,
//GO(__kmpc_team_static_init_4u,
//GO(__kmpc_team_static_init_8,
//GO(__kmpc_team_static_init_8u,
//GO(__kmpc_test_lock,
//GO(__kmpc_test_nest_lock,
//GO(__kmpc_threadprivate,
//GO(__kmpc_threadprivate_cached,
//GO(__kmpc_threadprivate_register,
//GO(__kmpc_threadprivate_register_vec,
//GO(__kmpc_unset_lock,
//GO(__kmpc_unset_nest_lock,
GO(kmp_aligned_malloc, pFLL)
GO(kmp_aligned_malloc_, vFLL)
GO(kmp_calloc, pFLL)
GO(kmp_calloc_, pFLL)
GO(kmp_create_affinity_mask, vFp)
GO(kmp_create_affinity_mask_, vFp)
GO(kmp_destroy_affinity_mask, vFp)
GO(kmp_destroy_affinity_mask_, vFp)
GO(kmp_free, vFp)
GO(kmp_free_, vFp)
GO(kmp_get_affinity, iFp)
GO(kmp_get_affinity_, iFp)
GO(kmp_get_affinity_mask_proc, iFip)
GO(kmp_get_affinity_mask_proc_, iFip)
GO(kmp_get_affinity_max_proc, iFv)
GO(kmp_get_affinity_max_proc_, iFv)
GO(kmp_get_blocktime, iFv)
GO(kmp_get_blocktime_, iFv)
GO(kmp_get_cancellation_status, iFv)
GO(kmp_get_cancellation_status_, iFv)
GO(kmp_get_library, iFv)
GO(kmp_get_library_, iFv)
GO(kmp_get_num_known_threads, iFv)
GO(kmp_get_num_known_threads_, iFv)
GO(kmp_get_stacksize, iFv)
GO(kmp_get_stacksize_, iFv)
GO(kmp_get_stacksize_s, LFv)
GO(kmp_get_stacksize_s_, LFv)
GO(kmp_malloc, pFL)
GO(kmp_malloc_, vFL)
GO(kmp_realloc, pFpL)
GO(kmp_realloc_, pFpL)
GO(kmp_set_affinity, iFp)
GO(kmp_set_affinity_, iFp)
GO(kmp_set_affinity_mask_proc, iFip)
GO(kmp_set_affinity_mask_proc_, iFip)
GO(kmp_set_blocktime, vFi)
GO(kmp_set_blocktime_, vFi)
GO(kmp_set_defaults, vFp)
GO(kmp_set_defaults_, vFp)
GO(kmp_set_disp_num_buffers, vFi)
GO(kmp_set_disp_num_buffers_, vFi)
GO(kmp_set_library, vFi)
GO(kmp_set_library_, vFi)
GO(kmp_set_library_serial, vFv)
GO(kmp_set_library_serial_, vFv)
GO(kmp_set_library_throughput, vFv)
GO(kmp_set_library_throughput_, vFv)
GO(kmp_set_library_turnaround, vFv)
GO(kmp_set_library_turnaround_, vFv)
GO(kmp_set_stacksize, vFi)
GO(kmp_set_stacksize_, vFi)
GO(kmp_set_stacksize_s, vFL)
GO(kmp_set_stacksize_s_, vFL)
//GO(kmp_set_thread_affinity_mask_initial,
GO(kmp_set_warnings_off, vFv)
GO(kmp_set_warnings_off_, vFv)
GO(kmp_set_warnings_on, vFv)
GO(kmp_set_warnings_on_, vFv)
//GO(kmp_threadprivate_insert,
//GO(kmp_threadprivate_insert_private_data,
GO(kmp_unset_affinity_mask_proc, iFip)
GO(kmp_unset_affinity_mask_proc_, iFip)
//GO(kmpc_aligned_malloc,
//GO(kmpc_calloc,
//GO(kmpc_free,
//GO(kmpc_get_affinity_mask_proc,
//GO(kmpc_malloc,
//GO(kmpc_realloc,
//GO(kmpc_set_affinity_mask_proc,
//GO(kmpc_set_blocktime,
//GO(kmpc_set_defaults,
//GO(kmpc_set_disp_num_buffers,
//GO(kmpc_set_library,
//GO(kmpc_set_stacksize,
//GO(kmpc_set_stacksize_s,
//GO(kmpc_unset_affinity_mask_proc,
GO(omp_aligned_alloc, pFLLL)
GO(omp_aligned_calloc, pFLLLL)
GO(omp_alloc, pFLL)
GO(omp_calloc, pFLLL)
GO(omp_capture_affinity, LFpLp)
GO(omp_capture_affinity_, LFpLp)
GO(omp_control_tool, iFiip)
GO(omp_control_tool_, iFiip)
GO(omp_destroy_allocator, vFL)
GO(omp_destroy_allocator_, vFL)
GO(omp_destroy_lock, vFp)
GO(omp_destroy_lock_, vFp)
GO(omp_destroy_nest_lock, vFp)
GO(omp_destroy_nest_lock_, vFp)
GO(omp_display_affinity, vFp)
GO(omp_display_affinity_, vFp)
GO(omp_display_env, vFi)
GO(omp_display_env_, vFi)
GO(omp_free, vFpL)
GO(omp_fulfill_event, vFL)
GO(omp_fulfill_event_, vFL)
GO(omp_get_active_level, iFv)
GO(omp_get_active_level_, iFv)
GO(omp_get_affinity_format, LFpL)
GO(omp_get_affinity_format_, LFpL)
GO(omp_get_ancestor_thread_num, iFi)
GO(omp_get_ancestor_thread_num_, iFi)
GO(omp_get_cancellation, iFv)
GO(omp_get_cancellation_, iFv)
GO(omp_get_default_allocator, LFv)
GO(omp_get_default_allocator_, LFv)
GO(omp_get_default_device, iFv)
GO(omp_get_default_device_, iFv)
GO(omp_get_device_allocator, LFiL)
GO(omp_get_device_allocator_, LFiL)
GO(omp_get_device_and_host_allocator, LFiL)
GO(omp_get_device_and_host_allocator_, LFiL)
GO(omp_get_device_and_host_memspace, LFiL)
GO(omp_get_device_and_host_memspace_, LFiL)
GO(omp_get_device_memspace, LFiL)
GO(omp_get_device_memspace_, LFiL)
GO(omp_get_device_num, iFv)
GO(omp_get_device_num_, iFv)
GO(omp_get_devices_all_allocator, LFL)
GO(omp_get_devices_all_allocator_, LFL)
GO(omp_get_devices_all_memspace, LFL)
GO(omp_get_devices_all_memspace_, LFL)
GO(omp_get_devices_allocator, LFipL)
GO(omp_get_devices_allocator_, LFipL)
GO(omp_get_devices_and_host_allocator, LFipL)
GO(omp_get_devices_and_host_allocator_, LFipL)
GO(omp_get_devices_and_host_memspace, LFipL)
GO(omp_get_devices_and_host_memspace_, LFipL)
GO(omp_get_devices_memspace, LFipL)
GO(omp_get_devices_memspace_, LFipL)
GO(omp_get_dynamic, iFv)
GO(omp_get_dynamic_, iFv)
GO(omp_get_initial_device, iFv)
GO(omp_get_initial_device_, iFv)
GO(omp_get_interop_int, lFpip)
GO(omp_get_interop_int_, lFpip)
GO(omp_get_interop_name, pFpi)
GO(omp_get_interop_name_, pFpi)
GO(omp_get_interop_ptr, pFpip)
GO(omp_get_interop_ptr_, pFpip)
GO(omp_get_interop_rc_desc, pFpi)
GO(omp_get_interop_rc_desc_, pFpi)
GO(omp_get_interop_str, pFpip)
GO(omp_get_interop_str_, pFpip)
GO(omp_get_interop_type_desc, pFpi)
GO(omp_get_interop_type_desc_, pFpi)
GO(omp_get_level, iFv)
GO(omp_get_level_, iFv)
GO(omp_get_max_active_levels, iFv)
GO(omp_get_max_active_levels_, iFv)
GO(omp_get_max_task_priority, iFv)
GO(omp_get_max_task_priority_, iFv)
GO(omp_get_max_teams, iFv)
GO(omp_get_max_teams_, iFv)
GO(omp_get_max_threads, iFv)
GO(omp_get_max_threads_, iFv)
GO(omp_get_memspace_num_resources, iFL)
GO(omp_get_memspace_num_resources_, iFL)
GO(omp_get_nested, iFv)
GO(omp_get_nested_, iFv)
GO(omp_get_num_devices, iFv)
GO(omp_get_num_devices_, iFv)
GO(omp_get_num_interop_properties, iFp)
GO(omp_get_num_interop_properties_, iFp)
GO(omp_get_num_places, iFv)
GO(omp_get_num_places_, iFv)
GO(omp_get_num_procs, iFv)
GO(omp_get_num_procs_, iFv)
GO(omp_get_num_teams, iFv)
GO(omp_get_num_teams_, iFv)
GO(omp_get_num_threads, iFv)
GO(omp_get_num_threads_, iFv)
GO(omp_get_partition_num_places, iFv)
GO(omp_get_partition_num_places_, iFv)
GO(omp_get_partition_place_nums, vFp)
GO(omp_get_partition_place_nums_, vFp)
GO(omp_get_place_num, iFv)
GO(omp_get_place_num_, iFv)
GO(omp_get_place_num_procs, iFi)
GO(omp_get_place_num_procs_, iFi)
GO(omp_get_place_proc_ids, vFip)
GO(omp_get_place_proc_ids_, vFip)
GO(omp_get_proc_bind, uFv)
GO(omp_get_proc_bind_, uFv)
GO(omp_get_schedule, vFpp)
GO(omp_get_schedule_, vFpp)
GO(omp_get_submemspace, LFLip)
GO(omp_get_submemspace_, LFLip)
GO(omp_get_supported_active_levels, iFv)
GO(omp_get_supported_active_levels_, iFv)
GO(omp_get_team_num, iFv)
GO(omp_get_team_num_, iFv)
GO(omp_get_team_size, iFi)
GO(omp_get_team_size_, iFi)
GO(omp_get_teams_thread_limit, iFv)
GO(omp_get_teams_thread_limit_, iFv)
GO(omp_get_thread_limit, iFv)
GO(omp_get_thread_limit_, iFv)
GO(omp_get_thread_num, iFv)
GO(omp_get_thread_num_, iFv)
GO(omp_get_wtick, dFv)
GO(omp_get_wtick_, dFv)
GO(omp_get_wtime, dFv)
GO(omp_get_wtime_, dFv)
GO(omp_in_explicit_task, iFv)
GO(omp_in_explicit_task_, iFv)
GO(omp_in_final, iFv)
GO(omp_in_final_, iFv)
GO(omp_in_parallel, iFv)
GO(omp_in_parallel_, iFv)
GO(omp_init_allocator, LFLip)
GO(omp_init_allocator_, LFLip)
GO(omp_init_lock, vFp)
GO(omp_init_lock_, vFp)
GO(omp_init_lock_with_hint, vFpi)
GO(omp_init_lock_with_hint_, vFpi)
GO(omp_init_nest_lock, vFp)
GO(omp_init_nest_lock_, vFp)
GO(omp_init_nest_lock_with_hint, vFpi)
GO(omp_init_nest_lock_with_hint_, vFpi)
GO(omp_is_initial_device, iFv)
GO(omp_is_initial_device_, iFv)
GO(omp_pause_resource, iFii)
GO(omp_pause_resource_, iFii)
GO(omp_pause_resource_all, iFi)
GO(omp_pause_resource_all_, iFi)
GO(omp_realloc, pFpLLL)
GO(omp_set_affinity_format, vFp)
GO(omp_set_affinity_format_, vFp)
GO(omp_set_default_allocator, vFL)
GO(omp_set_default_allocator_, vFL)
GO(omp_set_default_device, vFi)
GO(omp_set_default_device_, vFi)
GO(omp_set_dynamic, vFi)
GO(omp_set_dynamic_, vFi)
GO(omp_set_lock, vFp)
GO(omp_set_lock_, vFp)
GO(omp_set_max_active_levels, vFi)
GO(omp_set_max_active_levels_, vFi)
GO(omp_set_nest_lock, vFp)
GO(omp_set_nest_lock_, vFp)
GO(omp_set_nested, vFi)
GO(omp_set_nested_, vFi)
GO(omp_set_num_teams, vFi)
GO(omp_set_num_teams_, vFi)
GO(omp_set_num_threads, vFi)
GO(omp_set_num_threads_, vFi)
GO(omp_set_schedule, vFui)
GO(omp_set_schedule_, vFui)
GO(omp_set_teams_thread_limit, vFi)
GO(omp_set_teams_thread_limit_, vFi)
GO(omp_test_lock, iFp)
GO(omp_test_lock_, iFp)
GO(omp_test_nest_lock, iFp)
GO(omp_test_nest_lock_, iFp)
GO(omp_unset_lock, vFp)
GO(omp_unset_lock_, vFp)
GO(omp_unset_nest_lock, vFp)
GO(omp_unset_nest_lock_, vFp)
GO(ompc_capture_affinity, LFpLp)
GO(ompc_display_affinity, vFp)
GO(ompc_get_affinity_format, LFpL)
GO(ompc_get_ancestor_thread_num, iFi)
GO(ompc_get_team_size, iFi)
GO(ompc_set_affinity_format, vFp)
GO(ompc_set_dynamic, vFi)
GO(ompc_set_max_active_levels, vFi)
GO(ompc_set_nested, vFi)
GO(ompc_set_num_threads, vFi)
GO(ompc_set_schedule, vFui)
//GO(ompt_libomp_connect,
//GO(ompt_start_tool,
//DATA(__kmp_debugging,
//DATA(__kmp_omp_debug_struct_info,
//DATA(__kmp_thread_pool,
//DATA(kmp_a_debug,
//DATA(kmp_b_debug,
//DATA(kmp_c_debug,
//DATA(kmp_d_debug,
//DATA(kmp_diag,
//DATA(kmp_e_debug,
//DATA(kmp_f_debug,
//DATA(kmp_max_mem_alloc,
//DATA(kmp_max_mem_space,
//DATA(kmp_mutex_impl_info,
//DATA(omp_cgroup_mem_alloc,
//DATA(omp_const_mem_alloc,
//DATA(omp_const_mem_space,
//DATA(omp_default_mem_alloc,
//DATA(omp_default_mem_space,
//DATA(omp_high_bw_mem_alloc,
//DATA(omp_high_bw_mem_space,
//DATA(omp_large_cap_mem_alloc,
//DATA(omp_large_cap_mem_space,
//DATA(omp_low_lat_mem_alloc,
//DATA(omp_low_lat_mem_space,
//DATA(omp_null_allocator,
//DATA(omp_null_mem_space,
//DATA(omp_pteam_mem_alloc,
//DATA(omp_thread_mem_alloc,
