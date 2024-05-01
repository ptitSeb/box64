#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//GO(wl_array_add, 
//GO(wl_array_copy, 
//GO(wl_array_init, 
//GO(wl_array_release, 
//DATA(wl_buffer_interface, 
//DATA(wl_callback_interface, 
DATA(wl_compositor_interface, sizeof(void*)*3+sizeof(int)*4)
//DATA(wl_data_device_interface, 
//DATA(wl_data_device_manager_interface, 
//DATA(wl_data_offer_interface, 
//DATA(wl_data_source_interface, 
GO(wl_display_cancel_read, vFp)
GO(wl_display_connect, pFp)
GO(wl_display_connect_to_fd, pFi)
GO(wl_display_create_queue, pFp)
GO(wl_display_disconnect, vFp)
GO(wl_display_dispatch, iFp)
GO(wl_display_dispatch_pending, iFp)
GO(wl_display_dispatch_queue, iFpp)
GO(wl_display_dispatch_queue_pending, iFpp)
GO(wl_display_flush, iFp)
GO(wl_display_get_error, iFp)
GO(wl_display_get_fd, iFp)
//GO(wl_display_get_protocol_error, 
DATA(wl_display_interface, sizeof(void*)*3+sizeof(int)*4)
GO(wl_display_prepare_read, iFp)
GO(wl_display_prepare_read_queue, iFpp)
GO(wl_display_read_events, iFp)
GO(wl_display_roundtrip, iFp)
GO(wl_display_roundtrip_queue, iFpp)
GO(wl_event_queue_destroy, vFp)
DATA(wl_keyboard_interface, sizeof(void*)*3+sizeof(int)*4)
//GO(wl_list_empty, 
//GO(wl_list_init, 
//GO(wl_list_insert, 
//GO(wl_list_insert_list, 
//GO(wl_list_length, 
//GO(wl_list_remove, 
//GO(wl_log_set_handler_client, 
DATA(wl_output_interface, sizeof(void*)*3+sizeof(int)*4)
DATA(wl_pointer_interface, sizeof(void*)*3+sizeof(int)*4)
//GO(wl_proxy_add_dispatcher, 
GO(wl_proxy_add_listener, iFppp)
GO(wl_proxy_create, pFpp)
GO(wl_proxy_create_wrapper, pFp)
GO(wl_proxy_destroy, vFp)
//GO(wl_proxy_get_class, 
GO(wl_proxy_get_id, uFp)
GO(wl_proxy_get_listener, pFp)
//GO(wl_proxy_get_tag, 
GO(wl_proxy_get_user_data, pFp)
GO(wl_proxy_get_version, uFp)
GO(wl_proxy_marshal, vFpupppppppppp)    //VAARGS
GO(wl_proxy_marshal_array, vFpup)
GO(wl_proxy_marshal_array_constructor, pFpupp)
GO(wl_proxy_marshal_array_constructor_versioned, pFpuppu)
//GO(wl_proxy_marshal_array_flags, 
GO(wl_proxy_marshal_constructor, pFpupppppppppppppppp)  //VAARGS
GO(wl_proxy_marshal_constructor_versioned, pFpupupppppppppppppppp)  //VAARGS
GO(wl_proxy_marshal_flags, pFpupuupppppppppppppppppppp) //VAARGS
GO(wl_proxy_set_queue, vFpp)
//GO(wl_proxy_set_tag, 
GO(wl_proxy_set_user_data, vFpp)
GO(wl_proxy_wrapper_destroy, vFp)
DATA(wl_region_interface, 40)
DATA(wl_registry_interface, 40)
DATA(wl_seat_interface, 40)
DATA(wl_shell_interface, 40)
DATA(wl_shell_surface_interface, 40)
DATA(wl_shm_interface, 40)
DATA(wl_shm_pool_interface, 40)
DATA(wl_subcompositor_interface, 40)
DATA(wl_subsurface_interface, 40)
DATA(wl_surface_interface, 40)
DATA(wl_touch_interface, 40)
