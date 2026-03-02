#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

// indicator-desktop-shortcuts.h
GO(indicator_desktop_shortcuts_get_nicks, pFp)
GO(indicator_desktop_shortcuts_get_type, pFv)
GO(indicator_desktop_shortcuts_new, pFpp)
GO(indicator_desktop_shortcuts_nick_exec, iFpp)
GO(indicator_desktop_shortcuts_nick_exec_with_context, iFppp)
GO(indicator_desktop_shortcuts_nick_get_name, pFpp)

// indicator-image-helper.h
GO(indicator_image_helper, pFp)
GO(indicator_image_helper_update, vFpp)
GO(indicator_image_helper_update_from_gicon, vFpp)

// indicator-ng.h
GO(indicator_ng_get_profile, pFp)
GO(indicator_ng_get_service_file, pFp)
GO(indicator_ng_get_type, pFv)
GO(indicator_ng_new, pFpp)
GO(indicator_ng_new_for_profile, pFppp)
GO(indicator_ng_secondary_activate, vFpu)

// indicator-object.h
GO(indicator_object_check_environment, iFpp)
GO(indicator_object_entry_activate, vFppu)
GO(indicator_object_entry_activate_window, vFppuu)
GO(indicator_object_entry_close, vFppu)
GO(indicator_object_entry_is_visible, iFpp)
GO(indicator_object_get_entries, pFp)
GO(indicator_object_get_environment, pFp)
GO(indicator_object_get_location, uFpp)
GO(indicator_object_get_position, iFp)
GO(indicator_object_get_show_now, uFpp)
GO(indicator_object_get_type, pFv)
GO(indicator_object_new_from_file, pFp)
GO(indicator_object_set_environment, vFpp)
GO(indicator_object_set_visible, vFpi)

// indicator-service.h
GO(indicator_service_get_type, pFv)
GO(indicator_service_new, pFp)
GO(indicator_service_new_version, pFpu)

// indicator-service-manager.h
GO(indicator_service_manager_connected, iFp)
GO(indicator_service_manager_get_type, pFv)
GO(indicator_service_manager_new, pFp)
GO(indicator_service_manager_new_version, pFpu)
GO(indicator_service_manager_set_refresh, vFpu)

// indicator.h enums
GO(indicator_scroll_direction_get_type, pFv)

// data symbols
DATA(ICON_SIZE, sizeof(int))
DATA(INDICATOR_NAMES_DATA, sizeof(void*))
