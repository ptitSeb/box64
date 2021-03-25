#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//GO(_book_maptype1_quantvals, 
//GO(_book_unquantize, 
//GO(drft_backward, 
//GO(drft_clear, 
//GO(drft_forward, 
//GO(drft_init, 
//GO(_float32_pack, 
//GO(_float32_unpack, 
//GO(floor1_encode, 
//GO(floor1_fit, 
//GO(floor1_interpolate_fit, 
//GO(_make_words, 
//GO(mdct_backward, 
//GO(mdct_clear, 
//GO(mdct_forward, 
//GO(mdct_init, 
//GO(ov_ilog, 
//GO(res0_free_info, 
//GO(res0_free_look, 
//GO(res0_inverse, 
//GO(res0_look, 
//GO(res0_pack, 
//GO(res0_unpack, 
//GO(res1_class, 
//GO(res1_forward, 
//GO(res1_inverse, 
//GO(res2_class, 
//GO(res2_forward, 
//GO(res2_inverse, 
//GO(_ve_envelope_clear, 
//GO(_ve_envelope_init, 
//GO(_ve_envelope_mark, 
//GO(_ve_envelope_search, 
//GO(_ve_envelope_shift, 
//GO(_vi_gpsy_free, 
//GO(_vi_psy_free, 
GO(vorbis_analysis, iFpp)
GO(vorbis_analysis_blockout, iFpp)
GO(vorbis_analysis_buffer, pFpi)
GO(vorbis_analysis_headerout, iFppppp)
GO(vorbis_analysis_init, iFpp)
GO(vorbis_analysis_wrote, iFpi)
//GO(_vorbis_apply_window, 
GO(vorbis_bitrate_addblock, iFp)
//GO(vorbis_bitrate_clear, 
GO(vorbis_bitrate_flushpacket, iFpp)
//GO(vorbis_bitrate_init, 
//GO(vorbis_bitrate_managed, 
//GO(_vorbis_block_alloc, 
GO(vorbis_block_clear, iFp)
GO(vorbis_block_init, iFpp)
//GO(_vorbis_block_ripcord, 
//GO(vorbis_book_clear, 
//GO(vorbis_book_codelen, 
//GO(vorbis_book_codeword, 
//GO(vorbis_book_decode, 
//GO(vorbis_book_decodev_add, 
//GO(vorbis_book_decodevs_add, 
//GO(vorbis_book_decodev_set, 
//GO(vorbis_book_decodevv_add, 
//GO(vorbis_book_encode, 
//GO(vorbis_book_init_decode, 
//GO(vorbis_book_init_encode, 
GO(vorbis_comment_add, vFpp)
GO(vorbis_comment_add_tag, vFppp)
GO(vorbis_comment_clear, vFp)
GO(vorbis_commentheader_out, iFpp)
GO(vorbis_comment_init, vFp)
GO(vorbis_comment_query, pFppi)
GO(vorbis_comment_query_count, iFpp)
GO(vorbis_dsp_clear, vFp)
//GO(vorbis_granule_time, 
GO(vorbis_info_blocksize, iFpi)
GO(vorbis_info_clear, vFp)
GO(vorbis_info_init, vFp)
//GO(vorbis_lpc_from_data, 
//GO(vorbis_lpc_predict, 
//GO(vorbis_lpc_to_lsp, 
//GO(vorbis_lsp_to_curve, 
GO(vorbis_packet_blocksize, iFpp)
//GO(vorbis_staticbook_destroy, 
//GO(vorbis_staticbook_pack, 
//GO(vorbis_staticbook_unpack, 
GO(vorbis_synthesis, iFpp)
GO(vorbis_synthesis_blockin, iFpp)
GO(vorbis_synthesis_halfrate, iFpi)
GO(vorbis_synthesis_halfrate_p, iFp)
GO(vorbis_synthesis_headerin, iFppp)
GO(vorbis_synthesis_idheader, iFp)
GO(vorbis_synthesis_init, iFpp)
GO(vorbis_synthesis_lapout, iFpp)
GO(vorbis_synthesis_pcmout, iFpp)
GO(vorbis_synthesis_read, iFpi)
GO(vorbis_synthesis_restart, iFp)
GO(vorbis_synthesis_trackonly, iFpp)
GO(vorbis_version_string, pFv)
GO(vorbis_window, pFpi)
//GO(_vorbis_window_get, 
//GO(_vp_ampmax_decay, 
//GO(_vp_couple_quantize_normalize, 
//GO(_vp_global_free, 
//GO(_vp_global_look, 
//GO(_vp_noisemask, 
//GO(_vp_offset_and_mix, 
//GO(_vp_psy_clear, 
//GO(_vp_psy_init, 
//GO(_vp_tonemask, 
