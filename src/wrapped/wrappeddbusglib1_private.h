#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

GO(dbus_connection_get_g_connection, pFp)
GO(dbus_connection_get_g_type, iFv)
GO(dbus_connection_setup_with_g_main, vFpp)
GO(dbus_g_bus_get, pFip)
GO(dbus_g_bus_get_private, pFipp)
GO(dbus_g_connection_flush, vFp)
GO(dbus_g_connection_get_connection, pFp)
GO(dbus_g_connection_get_g_type, iFv)
GO(dbus_g_connection_lookup_g_object, pFpp)
GO(dbus_g_connection_open, pFpp)
GO(dbus_g_connection_ref, pFp)
GO(dbus_g_connection_register_g_object, vFppp)
GO(dbus_g_connection_unref, vFp)
GO(dbus_g_connection_unregister_g_object, vFpp)
GO(dbus_g_error_domain_register, vFppi)
GO(dbus_g_error_get_name, pFp)
GO(dbus_g_error_has_name, iFpp)
GO(dbus_g_error_quark, pFv)
//GO(dbus_glib_internal_do_not_use_run_tests, 
GO(dbus_g_message_get_g_type, iFv)
//GO(dbus_g_message_get_message, 
//GO(dbus_g_message_ref, 
//GO(dbus_g_message_unref, 
GO(dbus_g_method_get_reply, pFp)
GO(dbus_g_method_get_sender, pFp)
//GO(dbus_g_method_return, vFppppppppppp) //vaarg
GO(dbus_g_method_return_error, vFpp)
GO(dbus_g_method_send_reply, vFpp)
GO(dbus_g_object_path_get_g_type, iFv)
//GO(dbus_g_object_register_marshaller, vFpipppppppp)   //vaarg, should remap with dbus_g_object_register_marshaller_array
GO(dbus_g_object_register_marshaller_array, vFpiup)
GO(dbus_g_object_type_install_info, vFip)   // p is DBusGObjectInfo *, that struct contains a GCallback... should wrap?
GO(dbus_g_object_type_register_shadow_property, vFipp)
//GO(dbus_g_proxy_add_signal, vFppipppppppp)  //vaarg
GOM(dbus_g_proxy_begin_call, pFEpppppiV)
GOM(dbus_g_proxy_begin_call_with_timeout, pFEpppppiiV)
//GO(dbus_g_proxy_call, iFpppippppppppppppp)  //vaarg
//GO(dbus_g_proxy_call_no_reply, vFppipppppppppppp)   //vaarg
//GO(dbus_g_proxy_call_with_timeout, iFppipipppppppppppp)   //vaarg
GO(dbus_g_proxy_cancel_call, vFpp)
GOM(dbus_g_proxy_connect_signal, vFEppppp)
GOM(dbus_g_proxy_disconnect_signal, vFEpppp)
//GO(dbus_g_proxy_end_call, iFpppippppppppppppppp)  //vaarg
GO(dbus_g_proxy_get_bus_name, pFp)
GO(dbus_g_proxy_get_interface, pFp)
GO(dbus_g_proxy_get_path, pFp)
GO(dbus_g_proxy_get_type, iFv)
GO(dbus_g_proxy_new_for_name, pFpppp)
GO(dbus_g_proxy_new_for_name_owner, pFppppp)
GO(dbus_g_proxy_new_for_peer, pFppp)
GO(dbus_g_proxy_new_from_proxy, pFppp)
GO(dbus_g_proxy_send, vFppp)
GO(dbus_g_proxy_set_default_timeout, vFpi)
GO(dbus_g_proxy_set_interface, vFpp)
GO(dbus_g_signature_get_g_type, iFv)
GO(dbus_g_thread_init, vFv)
GO(dbus_g_type_collection_get_fixed, iFppp)
//GO(dbus_g_type_collection_peek_vtable, 
GOM(dbus_g_type_collection_value_iterate, vFEppp)
GO(dbus_g_type_get_collection, iFpi)
GO(dbus_g_type_get_collection_specialization, iFi)
GO(dbus_g_type_get_map, iFpii)
GO(dbus_g_type_get_map_key_specialization, iFi)
GO(dbus_g_type_get_map_value_specialization, iFi)
//GO(dbus_g_type_get_struct, iFpippppppppppp) //vaarg, should wrap and use dbus_g_type_get_structv
GO(dbus_g_type_get_struct_member_type, iFiu)
GO(dbus_g_type_get_struct_size, uFi)
GO(dbus_g_type_get_structv, iFpup)
GO(dbus_g_type_is_collection, iFi)
GO(dbus_g_type_is_map, iFi)
GO(dbus_g_type_is_struct, iFi)
//GO(dbus_g_type_map_peek_vtable, 
GOM(dbus_g_type_map_value_iterate, vFEppp)
//GO(dbus_g_type_register_collection, 
//GO(dbus_g_type_register_map, 
//GO(dbus_g_type_register_struct, 
GO(dbus_g_type_specialized_collection_append, vFpp)
GO(dbus_g_type_specialized_collection_end_append, vFp)
GO(dbus_g_type_specialized_construct, pFi)
GO(dbus_g_type_specialized_init, vFv)
GO(dbus_g_type_specialized_init_append, vFpp)
GO(dbus_g_type_specialized_map_append, vFppp)
//GO(dbus_g_type_struct_get, iFpuppppppppppp) //vaarg
GO(dbus_g_type_struct_get_member, iFpup)
//GO(dbus_g_type_struct_peek_vtable, 
//GO(dbus_g_type_struct_set, iFpupppppppppppp)  //vaarg
GO(dbus_g_type_struct_set_member, iFpup)
GO(dbus_message_get_g_type, iFv)
GO(dbus_server_setup_with_g_main, vFpp)
GO(dbus_set_g_error, vFpp)
