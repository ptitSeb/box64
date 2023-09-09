#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif
//typedef uint32_t xkb_keycode_t

GO(xkb_compose_state_feed, iFpu)
//GO(xkb_compose_state_get_compose_table, 
//GO(xkb_compose_state_get_one_sym, 
GO(xkb_compose_state_get_status, iFp)
//GO(xkb_compose_state_get_utf8, 
GO(xkb_compose_state_new, pFpi)
//GO(xkb_compose_state_ref, 
GO(xkb_compose_state_reset, vFp)
GO(xkb_compose_state_unref, vFp)
//GO(xkb_compose_table_new_from_buffer, 
//GO(xkb_compose_table_new_from_file, 
GO(xkb_compose_table_new_from_locale, pFppi)
//GO(xkb_compose_table_ref, 
GO(xkb_compose_table_unref, vFp)
//GO(xkb_context_get_log_level, 
//GO(xkb_context_get_log_verbosity, 
GO(xkb_context_get_user_data, pFp)
GO(xkb_context_include_path_append, iFpp)
//GO(xkb_context_include_path_append_default, 
//GO(xkb_context_include_path_clear, 
//GO(xkb_context_include_path_get, 
//GO(xkb_context_include_path_reset_defaults, 
GO(xkb_context_new, pFi)
//GO(xkb_context_num_include_paths, 
GO(xkb_context_ref, pFp)
//GOM(xkb_context_set_log_fn, vFEpp)
GO(xkb_context_set_log_level, vFpi)
GO(xkb_context_set_log_verbosity, vFpi)
GO(xkb_context_set_user_data, vFpp)
GO(xkb_context_unref, vFp)
GO(xkb_keymap_get_as_string, pFpi)
GO(xkb_keymap_key_by_name, uFpp)
//GOM(xkb_keymap_key_for_each, vFEppp)
GO(xkb_keymap_key_get_name, pFpu)
GO(xkb_keymap_key_get_syms_by_level, iFpuuup)
GO(xkb_keymap_key_repeats, iFpu)
GO(xkb_keymap_layout_get_index, uFpp)
GO(xkb_keymap_layout_get_name, pFpu)
GO(xkb_keymap_led_get_index, uFpp)
GO(xkb_keymap_led_get_name, pFpu)
GO(xkb_keymap_max_keycode, uFp)
GO(xkb_keymap_min_keycode, uFp)
GO(xkb_keymap_mod_get_index, uFpp)
GO(xkb_keymap_mod_get_name, pFpu)
GO(xkb_keymap_new_from_buffer, pFppLii)
//GO(xkb_keymap_new_from_file, 
GO(xkb_keymap_new_from_names, pFppi)
GO(xkb_keymap_new_from_string, pFppii)
GO(xkb_keymap_num_layouts, uFp)
GO(xkb_keymap_num_layouts_for_key, uFpu)
GO(xkb_keymap_num_leds, uFp)
GO(xkb_keymap_num_levels_for_key, uFpuu)
GO(xkb_keymap_num_mods, uFp)
GO(xkb_keymap_ref, pFp)
GO(xkb_keymap_unref, vFp)
GO(xkb_keysym_from_name, uFpi)
GO(xkb_keysym_get_name, iFupL)
GO(xkb_keysym_to_lower, uFu)
GO(xkb_keysym_to_upper, uFu)
GO(xkb_keysym_to_utf32, uFu)
GO(xkb_keysym_to_utf8, iFupL)
GO(xkb_state_get_keymap, pFp)
GO(xkb_state_key_get_consumed_mods, uFpu)
GO(xkb_state_key_get_consumed_mods2, uFpui)
GO(xkb_state_key_get_layout, iFpu)
GO(xkb_state_key_get_level, iFpui)
GO(xkb_state_key_get_one_sym, uFpu)
GO(xkb_state_key_get_syms, iFpip)
GO(xkb_state_key_get_utf32, uFpu)
GO(xkb_state_key_get_utf8, iFpupL)
GO(xkb_state_layout_index_is_active, iFpii)
GO(xkb_state_layout_name_is_active, iFppi)
GO(xkb_state_led_index_is_active, iFpi)
GO(xkb_state_led_name_is_active, iFpp)
GO(xkb_state_mod_index_is_active, iFpii)
GO(xkb_state_mod_index_is_consumed, iFpuu)
GO(xkb_state_mod_index_is_consumed2, iFpuui)
//GO(xkb_state_mod_indices_are_active, iFpiiV)
GO(xkb_state_mod_mask_remove_consumed, uFpuu)
GO(xkb_state_mod_name_is_active, iFppi)
//GO(xkb_state_mod_names_are_active, iFpiiV)
GO(xkb_state_new, pFp)
GO(xkb_state_ref, pFp)
GO(xkb_state_serialize_layout, uFpi)
GO(xkb_state_serialize_mods, uFpi)
GO(xkb_state_unref, vFp)
GO(xkb_state_update_key, iFpui)
GO(xkb_state_update_mask, iFpiiiiii)
