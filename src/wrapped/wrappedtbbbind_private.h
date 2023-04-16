#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GOM(__TBB_internal_allocate_binding_handler, pFiiii)    //%%noE
GOM(__TBB_internal_apply_affinity, vFpi)   //%%noE
GOM(__TBB_internal_deallocate_binding_handler, vFp) //%%noE
GOM(__TBB_internal_destroy_system_topology, vFv)    //%%noE
GOM(__TBB_internal_get_default_concurrency, iFiii)  //%%noE
GOM(__TBB_internal_initialize_system_topology, vFLpppp) //%%noE
GOM(__TBB_internal_restore_affinity, vFpi)  //%%noE
//GO(_ZN3tbb6detail2r115runtime_warningEPKcz, 
//DATA(_ZN3tbb6detail2r115system_topology12instance_ptrE, 
//GO(_ZN3tbb6detail2r117assertion_failureEPKciS3_S3_, 
