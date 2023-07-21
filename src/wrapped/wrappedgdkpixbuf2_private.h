#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(gdk_colorspace_get_type, iFv)
GO(gdk_interp_type_get_type, iFv)
GO(gdk_pixbuf_add_alpha, pFpiCCC)
GO(gdk_pixbuf_alpha_mode_get_type, iFv)
GO(gdk_pixbuf_animation_get_height, iFp)
GO(gdk_pixbuf_animation_get_iter, pFpp)
GO(gdk_pixbuf_animation_get_static_image, pFp)
GO(gdk_pixbuf_animation_get_type, lFv)
GO(gdk_pixbuf_animation_get_width, iFp)
GO(gdk_pixbuf_animation_is_static_image, iFp)
GO(gdk_pixbuf_animation_iter_advance, iFpp)
GO(gdk_pixbuf_animation_iter_get_delay_time, iFp)
GO(gdk_pixbuf_animation_iter_get_pixbuf, pFp)
//GO(gdk_pixbuf_animation_iter_get_type, 
GO(gdk_pixbuf_animation_iter_on_currently_loading_frame, iFp)
GO(gdk_pixbuf_animation_new_from_file, pFpp)
//GO(gdk_pixbuf_animation_new_from_resource, 
//GO(gdk_pixbuf_animation_new_from_stream, 
//GO(gdk_pixbuf_animation_new_from_stream_async, 
//GO(gdk_pixbuf_animation_new_from_stream_finish, 
GO(gdk_pixbuf_animation_ref, pFp)
GO(gdk_pixbuf_animation_unref, vFp)
GO(gdk_pixbuf_apply_embedded_orientation, pFp)
GO(gdk_pixbuf_composite, vFppiiiiddddii)
GO(gdk_pixbuf_composite_color, vFppiiiiddddiiiiiuu)
GO(gdk_pixbuf_composite_color_simple, pFpiiiiiuu)
GO(gdk_pixbuf_copy, pFp)
GO(gdk_pixbuf_copy_area, vFpiiiipii)
GO(gdk_pixbuf_error_get_type, iFv)
GO(gdk_pixbuf_error_quark, pFv)
GO(gdk_pixbuf_fill, vFpu)
GO(gdk_pixbuf_flip, pFpi)
GO(gdk_pixbuf_format_copy, pFp)
GO(gdk_pixbuf_format_free, vFp)
GO(gdk_pixbuf_format_get_description, pFp)
GO(gdk_pixbuf_format_get_extensions, pFp)
GO(gdk_pixbuf_format_get_license, pFp)
GO(gdk_pixbuf_format_get_mime_types, pFp)
GO(gdk_pixbuf_format_get_name, pFp)
GO(gdk_pixbuf_format_get_type, iFv)
GO(gdk_pixbuf_format_is_disabled, iFp)
GO(gdk_pixbuf_format_is_scalable, iFp)
GO(gdk_pixbuf_format_is_writable, iFp)
GO(gdk_pixbuf_format_set_disabled, vFpi)
GO(gdk_pixbuf_from_pixdata, pFpip)
GO(gdk_pixbuf_get_bits_per_sample, iFp)
GO(gdk_pixbuf_get_byte_length, uFp)
GO(gdk_pixbuf_get_colorspace, uFp)
GO(gdk_pixbuf_get_file_info, pFppp)
GO(gdk_pixbuf_get_formats, pFv)
GO(gdk_pixbuf_get_has_alpha, iFp)
GO(gdk_pixbuf_get_height, iFp)
GO(gdk_pixbuf_get_n_channels, iFp)
GO(gdk_pixbuf_get_option, pFpp)
GO(gdk_pixbuf_get_pixels, pFp)
GO(gdk_pixbuf_get_pixels_with_length, pFpp)
GO(gdk_pixbuf_get_rowstride, iFp)
GO(gdk_pixbuf_gettext, pFp)
GO(gdk_pixbuf_get_type, iFv)
GO(gdk_pixbuf_get_width, iFp)
GO(gdk_pixbuf_loader_close, iFpp)
GO(gdk_pixbuf_loader_get_animation, pFp)
GO(gdk_pixbuf_loader_get_format, pFp)
GO(gdk_pixbuf_loader_get_pixbuf, pFp)
GO(gdk_pixbuf_loader_get_type, iFv)
GO(gdk_pixbuf_loader_new, pFv)
GO(gdk_pixbuf_loader_new_with_mime_type, pFpp)
GO(gdk_pixbuf_loader_new_with_type, pFpp)
GO(gdk_pixbuf_loader_set_size, vFpii)
GO(gdk_pixbuf_loader_write, iFppLp)
GO(gdk_pixbuf_loader_write_bytes, iFppp)
GO(gdk_pixbuf_new, pFiiiii)
GOM(gdk_pixbuf_new_from_data, pFEpiiiiiipp)
GO(gdk_pixbuf_new_from_file, pFpp)
GO(gdk_pixbuf_new_from_file_at_scale, pFpiiip)
GO(gdk_pixbuf_new_from_file_at_size, pFpiip)
GO(gdk_pixbuf_new_from_inline, pFipip)
GO(gdk_pixbuf_new_from_resource, pFpp)
GO(gdk_pixbuf_new_from_resource_at_scale, pFpiiip)
GO(gdk_pixbuf_new_from_stream, pFppp)
//GO(gdk_pixbuf_new_from_stream_async, 
//GO(gdk_pixbuf_new_from_stream_at_scale, 
//GO(gdk_pixbuf_new_from_stream_at_scale_async, 
//GO(gdk_pixbuf_new_from_stream_finish, 
GO(gdk_pixbuf_new_from_xpm_data, pFp)
GO(gdk_pixbuf_new_subpixbuf, pFpiiii)
//GO(gdk_pixbuf_non_anim_get_type, 
//GO(gdk_pixbuf_non_anim_new, 
GO(gdk_pixbuf_ref, pFp)
GO(gdk_pixbuf_rotate_simple, pFpi)
//GO(gdk_pixbuf_rotation_get_type, 
GO(gdk_pixbuf_saturate_and_pixelate, vFppfi)
//GO(gdk_pixbuf_save, iFpppppppppppppp)   //vaarg, wrap with gdk_pixbuf_save?
//GO(gdk_pixbuf_save_to_buffer, iFpppppppppppppp)   //vaarg, wrap with gdk_pixbuf_save_to_bufferv ?
GO(gdk_pixbuf_save_to_bufferv, iFppppppp)
//GOM(gdk_pixbuf_save_to_callback, iFEpppppV)   //wrap with gdk_pixbuf_save_to_callbackv
//GOM(gdk_pixbuf_save_to_callbackv, iFEppppppp)
//GO(gdk_pixbuf_save_to_stream, iFpppppppppppppp) //vaarg, but gdk_pixbuf_save_to_streamv is fairly recent
//GOM(gdk_pixbuf_save_to_stream_async, vFEppppppppppppppp)   //vaarg, and gdk_pixbuf_save_to_streamv_async is fairly recent
GO(gdk_pixbuf_save_to_stream_finish, iFpp)
GO(gdk_pixbuf_savev, iFpppppp)
GO(gdk_pixbuf_scale, vFppiiiiddddi)
GO(gdk_pixbuf_scaled_anim_get_type, iFv)
GO(gdk_pixbuf_scaled_anim_iter_get_type, iFv)
GO(gdk_pixbuf_scale_simple, pFpiii)
GO(gdk_pixbuf_set_option, pFppp)
//GO(gdk_pixbuf_simple_anim_add_frame, 
//GO(gdk_pixbuf_simple_anim_get_loop, 
//GO(gdk_pixbuf_simple_anim_get_type, 
//GO(gdk_pixbuf_simple_anim_iter_get_type, 
//GO(gdk_pixbuf_simple_anim_new, 
//GO(gdk_pixbuf_simple_anim_set_loop, 
GO(gdk_pixbuf_unref, vFp)
GO(gdk_pixdata_deserialize, iFpupp)
GO(gdk_pixdata_from_pixbuf, pFppi)
GO(gdk_pixdata_serialize, pFpp)
GO(gdk_pixdata_to_csource, pFppi)
