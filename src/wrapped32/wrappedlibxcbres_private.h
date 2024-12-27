#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

//%S z my_xcb_cookie_t u
//%S Z my_xcb_XXX_iterator_t pii

//GO(xcb_res_client_end, 
//GO(xcb_res_client_id_spec_end, 
GO(xcb_res_client_id_spec_next, vFbpii_)
//GO(xcb_res_client_id_value_end, 
GO(xcb_res_client_id_value_next, vFbpii_)
GO(xcb_res_client_id_value_sizeof, iFp)
GO(xcb_res_client_id_value_value, pFp)
//GOS(xcb_res_client_id_value_value_end, pFpp)
GO(xcb_res_client_id_value_value_length, iFp)
GO(xcb_res_client_next, vFbpii_)
//DATA(xcb_res_id, 
GOS(xcb_res_query_client_ids, zFEnup)  //%%
GOS(xcb_res_query_client_ids_ids_iterator, ZFEp)   //%%
GO(xcb_res_query_client_ids_ids_length, iFp)
GO(xcb_res_query_client_ids_reply, pFnubp_)
GO(xcb_res_query_client_ids_sizeof, iFp)
//GOS(xcb_res_query_client_ids_unchecked, pFpnup)
//GOS(xcb_res_query_client_pixmap_bytes, pFpnu)
GO(xcb_res_query_client_pixmap_bytes_reply, pFnubp_)
//GOS(xcb_res_query_client_pixmap_bytes_unchecked, pFpnu)
//GOS(xcb_res_query_client_resources, pFpnu)
GO(xcb_res_query_client_resources_reply, pFnubp_)
GO(xcb_res_query_client_resources_sizeof, iFp)
GO(xcb_res_query_client_resources_types, pFp)
//GOS(xcb_res_query_client_resources_types_iterator, pFpp)
GO(xcb_res_query_client_resources_types_length, iFp)
//GOS(xcb_res_query_client_resources_unchecked, pFpnu)
//GOS(xcb_res_query_clients, pFpn)
GO(xcb_res_query_clients_clients, pFp)
//GOS(xcb_res_query_clients_clients_iterator, pFpp)
GO(xcb_res_query_clients_clients_length, iFp)
GO(xcb_res_query_clients_reply, pFnubp_)
GO(xcb_res_query_clients_sizeof, iFp)
//GOS(xcb_res_query_clients_unchecked, pFpn)
//GOS(xcb_res_query_resource_bytes, pFpnuup)
GO(xcb_res_query_resource_bytes_reply, pFnubp_)
GO(xcb_res_query_resource_bytes_sizeof, iFp)
//GOS(xcb_res_query_resource_bytes_sizes_iterator, pFpp)
GO(xcb_res_query_resource_bytes_sizes_length, iFp)
//GOS(xcb_res_query_resource_bytes_unchecked, pFpnuup)
//GOS(xcb_res_query_version, pFpnCC)
GO(xcb_res_query_version_reply, pFnubp_)
//GOS(xcb_res_query_version_unchecked, pFpnCC)
//GO(xcb_res_resource_id_spec_end, 
GO(xcb_res_resource_id_spec_next, vFbpii_)
//GO(xcb_res_resource_size_spec_end, 
GO(xcb_res_resource_size_spec_next, vFbpii_)
GO(xcb_res_resource_size_value_cross_references, pFp)
//GOS(xcb_res_resource_size_value_cross_references_iterator, pFpp)
GO(xcb_res_resource_size_value_cross_references_length, iFp)
//GO(xcb_res_resource_size_value_end, 
GO(xcb_res_resource_size_value_next, vFbpii_)
GO(xcb_res_resource_size_value_sizeof, iFp)
//GO(xcb_res_type_end, 
GO(xcb_res_type_next, vFbpii_)
