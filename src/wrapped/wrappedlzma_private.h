#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GOM(lzma_alone_decoder, iFEpU)
GOM(lzma_alone_encoder, iFEpp)
GO(lzma_auto_decoder, iFpUu)
GO(lzma_block_buffer_bound, LFL)
//GO(lzma_block_buffer_decode, 
//GO(lzma_block_buffer_encode, 
GO(lzma_block_compressed_size, uFpL)
//GO(lzma_block_decoder, 
//GO(lzma_block_encoder, 
//GO(lzma_block_header_decode, 
GO(lzma_block_header_encode, uFpp)
GO(lzma_block_header_size, uFp)
GO(lzma_block_total_size, LFp)
GO(lzma_block_uncomp_encode, uFppLppL)
//GO(lzma_block_unpadded_size, 
GO(lzma_check_is_supported, CFu)
GO(lzma_check_size, uFu)
GOM(lzma_code, iFEpi)
GO(lzma_cputhreads, uFv)
GO(lzma_crc32, uFpLu)
GO(lzma_crc64, LFpLL)
//GO(lzma_easy_buffer_encode, 
GO(lzma_easy_decoder_memusage, LFu)
GOM(lzma_easy_encoder, iFEpui)
//GO(lzma_easy_encoder_memusage, 
GOM(lzma_end, vFEp)
GO(lzma_filter_decoder_is_supported, CFL)
//GO(lzma_filter_encoder_is_supported, 
//GO(lzma_filter_flags_decode, 
GO(lzma_filter_flags_encode, uFpppL)
GO(lzma_filter_flags_size, uFpp)
//GO(lzma_filters_copy, 
//GO(lzma_filters_update, 
GO(lzma_get_check, iFp)
//GO(lzma_get_progress, 
//GO(lzma_index_append, 
//GO(lzma_index_block_count, 
GOM(lzma_index_buffer_decode, iFEpppppL)
GO(lzma_index_buffer_encode, uFpppL)
//GO(lzma_index_cat, 
GO(lzma_index_checks, uFp)
//GO(lzma_index_decoder, 
//GO(lzma_index_dup, 
//GO(lzma_index_encoder, 
GOM(lzma_index_end, vFEpp)
//GO(lzma_index_file_size, 
GO(lzma_index_hash_append, uFpLL)
GO(lzma_index_hash_decode, uFpppL)
//GO(lzma_index_hash_end, 
//GO(lzma_index_hash_init, 
GO(lzma_index_hash_size, LFp)
//GO(lzma_index_init, 
GO(lzma_index_iter_init, vFpp)
GO(lzma_index_iter_locate, CFpL)
GO(lzma_index_iter_next, CFpu)
GO(lzma_index_iter_rewind, vFp)
GO(lzma_index_memusage, LFLL)
GO(lzma_index_memused, LFp)
GO(lzma_index_size, UFp)
//GO(lzma_index_stream_count, 
GO(lzma_index_stream_flags, uFpp)
GO(lzma_index_stream_padding, uFpL)
//GO(lzma_index_stream_size, 
//GO(lzma_index_total_size, 
GO(lzma_index_uncompressed_size, LFp)
GO(lzma_lzma_preset, CFpu)
GO(lzma_memlimit_get, UFp)
GO(lzma_memlimit_set, iFpU)
GO(lzma_memusage, UFp)
GO(lzma_mf_is_supported, CFu)
GO(lzma_mode_is_supported, CFu)
GO(lzma_physmem, LFv)
GOM(lzma_properties_decode, iFEpppL)
GO(lzma_properties_encode, uFpp)
GO(lzma_properties_size, iFpp)
//GO(lzma_raw_buffer_decode, 
//GO(lzma_raw_buffer_encode, 
GOM(lzma_raw_decoder, iFEpp)
GO(lzma_raw_decoder_memusage, LFp)
GOM(lzma_raw_encoder, iFEpp)
//GO(lzma_raw_encoder_memusage, 
//GO(lzma_stream_buffer_bound, 
GOM(lzma_stream_buffer_decode, iFEpupppLppL)
//GO(lzma_stream_buffer_encode, 
GOM(lzma_stream_decoder, iFEpUi)
GOM(lzma_stream_encoder, iFEppi)
GOM(lzma_stream_encoder_mt, iFEpp)
GOM(lzma_stream_decoder_mt, iFEpp)
GO(lzma_stream_encoder_mt_memusage, LFp)
GO(lzma_stream_flags_compare, uFpp)
GO(lzma_stream_footer_decode, uFpp)
GO(lzma_stream_footer_encode, uFpp)
//GO(lzma_stream_header_decode, 
//GO(lzma_stream_header_encode, 
//GO(lzma_version_number, 
GO(lzma_version_string, pFv)
GO(lzma_vli_decode, uFppppL)
GO(lzma_vli_encode, uFLpppL)
GO(lzma_vli_size, uFL)
