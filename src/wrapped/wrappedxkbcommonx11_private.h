#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(xkb_x11_get_core_keyboard_device_id, iFp)
GO(xkb_x11_keymap_new_from_device, pFppii)
GO(xkb_x11_setup_xkb_extension, iFpWWipppp)
GO(xkb_x11_state_new_from_device, pFppi)
