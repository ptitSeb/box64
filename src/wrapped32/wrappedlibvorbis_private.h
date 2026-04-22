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
GOM(vorbis_analysis, iFEpp)
GOM(vorbis_analysis_blockout, iFEpp)
GOM(vorbis_analysis_buffer, pFEpi)
GOM(vorbis_analysis_headerout, iFEppppp)
GOM(vorbis_analysis_init, iFEpp)
GOM(vorbis_analysis_wrote, iFEpi)
//GO(_vorbis_apply_window, 
GOM(vorbis_bitrate_addblock, iFEp)
//GO(vorbis_bitrate_clear, 
GOM(vorbis_bitrate_flushpacket, iFEpp)
//GO(vorbis_bitrate_init, 
//GO(vorbis_bitrate_managed, 
//GO(_vorbis_block_alloc, 
GOM(vorbis_block_clear, iFEp)
GOM(vorbis_block_init, iFEpp)
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
GOM(vorbis_comment_add, vFEpp)
GOM(vorbis_comment_add_tag, vFEppp)
GOM(vorbis_comment_clear, vFEp)
GOM(vorbis_commentheader_out, iFEpp)
GOM(vorbis_comment_init, vFEp)
GOM(vorbis_comment_query, pFEppi)
GOM(vorbis_comment_query_count, iFEpp)
GOM(vorbis_dsp_clear, vFEp)
//GO(vorbis_granule_time, 
GOM(vorbis_info_blocksize, iFEpi)
GOM(vorbis_info_clear, vFEp)
GOM(vorbis_info_init, vFEp)
//GO(vorbis_lpc_from_data, 
//GO(vorbis_lpc_predict, 
//GO(vorbis_lpc_to_lsp, 
//GO(vorbis_lsp_to_curve, 
GOM(vorbis_packet_blocksize, lFEpp)
//GO(vorbis_staticbook_destroy, 
//GO(vorbis_staticbook_pack, 
//GO(vorbis_staticbook_unpack, 
GOM(vorbis_synthesis, iFEpp)
GOM(vorbis_synthesis_blockin, iFEpp)
GOM(vorbis_synthesis_halfrate, iFEpi)
GOM(vorbis_synthesis_halfrate_p, iFEp)
GOM(vorbis_synthesis_headerin, iFEppp)
GOM(vorbis_synthesis_idheader, iFEp)
GOM(vorbis_synthesis_init, iFEpp)
GOM(vorbis_synthesis_lapout, iFEpp)
GOM(vorbis_synthesis_pcmout, iFEpp)
GOM(vorbis_synthesis_read, iFEpi)
GOM(vorbis_synthesis_restart, iFEp)
GOM(vorbis_synthesis_trackonly, iFEpp)
GO(vorbis_version_string, pFv)
GOM(vorbis_window, pFEpi)
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
