/*******************************************************************
 * File automatically generated by rebuild_wrappers.py (v2.5.0.24) *
 *******************************************************************/
#ifndef __wrappedgstreamerTYPES_H_
#define __wrappedgstreamerTYPES_H_

#ifndef LIBNAME
#error You should only #include this file inside a wrapped*.c file
#endif
#ifndef ADDED_FUNCTIONS
#define ADDED_FUNCTIONS() 
#endif

typedef void* (*pFA_t)(va_list);
typedef void (*vFpp_t)(void*, void*);
typedef void* (*pFuV_t)(uint32_t, ...);
typedef void* (*pFuA_t)(uint32_t, va_list);
typedef void* (*pFpp_t)(void*, void*);
typedef void* (*pFpV_t)(void*, ...);
typedef void* (*pFpA_t)(void*, va_list);
typedef void (*vFppp_t)(void*, void*, void*);
typedef void (*vFppV_t)(void*, void*, ...);
typedef void (*vFppA_t)(void*, void*, va_list);
typedef int32_t (*iFppp_t)(void*, void*, void*);
typedef int32_t (*iFppV_t)(void*, void*, ...);
typedef int32_t (*iFppA_t)(void*, void*, va_list);
typedef uint32_t (*uFppp_t)(void*, void*, void*);
typedef void* (*pFppp_t)(void*, void*, void*);
typedef void* (*pFppV_t)(void*, void*, ...);
typedef void* (*pFppA_t)(void*, void*, va_list);
typedef void (*vFpupp_t)(void*, uint32_t, void*, void*);
typedef void (*vFpupV_t)(void*, uint32_t, void*, ...);
typedef void (*vFpupA_t)(void*, uint32_t, void*, va_list);
typedef void (*vFpppp_t)(void*, void*, void*, void*);
typedef int32_t (*iFpppp_t)(void*, void*, void*, void*);
typedef uint32_t (*uFpppp_t)(void*, void*, void*, void*);
typedef void* (*pFppip_t)(void*, void*, int32_t, void*);
typedef void* (*pFpppV_t)(void*, void*, void*, ...);
typedef void* (*pFpppA_t)(void*, void*, void*, va_list);
typedef uint32_t (*uFpippp_t)(void*, int32_t, void*, void*, void*);
typedef uintptr_t (*LFpuppp_t)(void*, uint32_t, void*, void*, void*);
typedef void (*vFpuLppp_t)(void*, uint32_t, uintptr_t, void*, void*, void*);
typedef void* (*pFupLLLpp_t)(uint32_t, void*, uintptr_t, uintptr_t, uintptr_t, void*, void*);
typedef void* (*pFpuLpupp_t)(void*, uint32_t, uintptr_t, void*, uint32_t, void*, void*);
typedef void (*vFpuppippV_t)(void*, uint32_t, void*, void*, int32_t, void*, void*, ...);
typedef void (*vFpuppippA_t)(void*, uint32_t, void*, void*, int32_t, void*, void*, va_list);
typedef int32_t (*iFppuppppp_t)(void*, void*, uint32_t, void*, void*, void*, void*, void*);
typedef int32_t (*iFiipppppppp_t)(int32_t, int32_t, void*, void*, void*, void*, void*, void*, void*, void*);

#define SUPER() ADDED_FUNCTIONS() \
	GO(gst_tag_list_new_valist, pFA_t) \
	GO(gst_init, vFpp_t) \
	GO(gst_caps_features_new_id, pFuV_t) \
	GO(gst_caps_features_new_id_valist, pFuA_t) \
	GO(gst_plugin_load_file, pFpp_t) \
	GO(_gst_element_error_printf, pFpV_t) \
	GO(gst_caps_features_new, pFpV_t) \
	GO(gst_make_element_message_details, pFpV_t) \
	GO(gst_tag_list_new, pFpV_t) \
	GO(gst_caps_features_new_valist, pFpA_t) \
	GO(gst_pad_sticky_events_foreach, vFppp_t) \
	GO(gst_structure_filter_and_map_in_place, vFppp_t) \
	GO(gst_tag_list_foreach, vFppp_t) \
	GO(gst_bin_add_many, vFppV_t) \
	GO(gst_caps_set_simple, vFppV_t) \
	GO(gst_structure_remove_fields, vFppV_t) \
	GO(gst_structure_set, vFppV_t) \
	GO(gst_caps_set_simple_valist, vFppA_t) \
	GO(gst_structure_remove_fields_valist, vFppA_t) \
	GO(gst_structure_set_valist, vFppA_t) \
	GO(gst_buffer_foreach_meta, iFppp_t) \
	GO(gst_caps_foreach, iFppp_t) \
	GO(gst_element_foreach_sink_pad, iFppp_t) \
	GO(gst_init_check, iFppp_t) \
	GO(gst_structure_foreach, iFppp_t) \
	GO(gst_element_link_many, iFppV_t) \
	GO(gst_structure_get, iFppV_t) \
	GO(gst_info_vasprintf, iFppA_t) \
	GO(gst_structure_get_valist, iFppA_t) \
	GO(gst_bus_add_watch, uFppp_t) \
	GO(gst_task_new, pFppp_t) \
	GO(gst_caps_new_simple, pFppV_t) \
	GO(gst_structure_new, pFppV_t) \
	GO(gst_structure_new_valist, pFppA_t) \
	GO(gst_mini_object_set_qdata, vFpupp_t) \
	GO(gst_tag_list_add, vFpupV_t) \
	GO(gst_tag_list_add_values, vFpupV_t) \
	GO(gst_tag_list_add_valist, vFpupA_t) \
	GO(gst_tag_list_add_valist_values, vFpupA_t) \
	GO(gst_bus_set_sync_handler, vFpppp_t) \
	GO(gst_pad_set_activate_function_full, vFpppp_t) \
	GO(gst_pad_set_activatemode_function_full, vFpppp_t) \
	GO(gst_pad_set_chain_function_full, vFpppp_t) \
	GO(gst_pad_set_event_function_full, vFpppp_t) \
	GO(gst_pad_set_getrange_function_full, vFpppp_t) \
	GO(gst_pad_set_iterate_internal_links_function_full, vFpppp_t) \
	GO(gst_pad_set_link_function_full, vFpppp_t) \
	GO(gst_pad_set_query_function_full, vFpppp_t) \
	GO(gst_iterator_find_custom, iFpppp_t) \
	GO(gst_pad_start_task, iFpppp_t) \
	GO(gst_iterator_fold, uFpppp_t) \
	GO(gst_registry_feature_filter, pFppip_t) \
	GO(gst_pad_create_stream_id_printf, pFpppV_t) \
	GO(gst_pad_create_stream_id_printf_valist, pFpppA_t) \
	GO(gst_bus_add_watch_full, uFpippp_t) \
	GO(gst_pad_add_probe, LFpuppp_t) \
	GO(gst_mini_object_init, vFpuLppp_t) \
	GO(gst_buffer_new_wrapped_full, pFupLLLpp_t) \
	GO(gst_memory_new_wrapped, pFupLLLpp_t) \
	GO(gst_util_array_binary_search, pFpuLpupp_t) \
	GO(gst_debug_log, vFpuppippV_t) \
	GO(gst_debug_log_valist, vFpuppippA_t) \
	GO(gst_type_find_register, iFppuppppp_t) \
	GO(gst_plugin_register_static, iFiipppppppp_t)

#endif // __wrappedgstreamerTYPES_H_
