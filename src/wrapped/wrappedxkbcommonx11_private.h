#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(xkb_x11_get_core_keyboard_device_id, iFb)
GO(xkb_x11_keymap_new_from_device, pFpbii)
GO(xkb_x11_setup_xkb_extension, iFbWWipppp)
GO(xkb_x11_state_new_from_device, pFpbi)
