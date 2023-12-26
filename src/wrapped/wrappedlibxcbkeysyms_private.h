#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(xcb_is_cursor_key, iFu)
GO(xcb_is_function_key, iFu)
GO(xcb_is_keypad_key, iFu)
GO(xcb_is_misc_function_key, iFu)
GO(xcb_is_modifier_key, iFu)
GO(xcb_is_pf_key, iFu)
GO(xcb_is_private_keypad_key, iFu)
GO(xcb_key_press_lookup_keysym, uFppi)
GO(xcb_key_release_lookup_keysym, uFppi)
GO(xcb_key_symbols_alloc, pFb)
GO(xcb_key_symbols_free, vFp)
GO(xcb_key_symbols_get_keycode, pFpu)
GO(xcb_key_symbols_get_keysym, uFpCi)
GO(xcb_refresh_keyboard_mapping, iFpp)