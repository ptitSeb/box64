#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

// Image functions
GO(vpx_img_alloc, pFpiuuu)
GO(vpx_img_wrap, pFpiuuup)
GO(vpx_img_set_rect, iFpuuuu)
GO(vpx_img_flip, vFp)
GO(vpx_img_free, vFp)

// Codec info functions
GO(vpx_codec_version, iFv)
GO(vpx_codec_version_str, pFv)
GO(vpx_codec_version_extra_str, pFv)
GO(vpx_codec_build_config, pFv)
GO(vpx_codec_iface_name, pFp)
GO(vpx_codec_err_to_string, pFi)
GO(vpx_codec_error, pFp)
GO(vpx_codec_error_detail, pFp)
GO(vpx_codec_destroy, iFp)
GO(vpx_codec_get_caps, lFp)
GO(vpx_codec_control_, iFpiN)

// Decoder functions
GO(vpx_codec_dec_init_ver, iFpppli)
GO(vpx_codec_peek_stream_info, iFppup)
GO(vpx_codec_get_stream_info, iFpp)
GO(vpx_codec_decode, iFppupl)
GO(vpx_codec_get_frame, pFpp)
//GOM(vpx_codec_register_put_frame_cb, iFEppp)    // callback
//GOM(vpx_codec_register_put_slice_cb, iFEppp)     // callback
GOM(vpx_codec_set_frame_buffer_functions, iFEpppp)

// Encoder functions
GO(vpx_codec_enc_init_ver, iFpppli)
GO(vpx_codec_enc_init_multi_ver, iFpppilpi)
GO(vpx_codec_enc_config_default, iFppu)
GO(vpx_codec_enc_config_set, iFpp)
GO(vpx_codec_get_global_headers, pFp)
GO(vpx_codec_encode, iFppILlL)
GO(vpx_codec_set_cx_data_buf, iFppuu)
GO(vpx_codec_get_cx_data, pFpp)
GO(vpx_codec_get_preview_frame, pFp)

// Codec interface accessors
GO(vpx_codec_vp8_cx, pFv)
GO(vpx_codec_vp9_cx, pFv)
GO(vpx_codec_vp8_dx, pFv)
GO(vpx_codec_vp9_dx, pFv)

// Internal algo data (accessed via accessor functions above)
//DATA(vpx_codec_vp8_cx_algo, 4)
//DATA(vpx_codec_vp9_cx_algo, 4)
//DATA(vpx_codec_vp8_dx_algo, 4)
//DATA(vpx_codec_vp9_dx_algo, 4)
