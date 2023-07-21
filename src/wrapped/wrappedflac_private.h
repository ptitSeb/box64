#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//DATA(FLAC_API_SUPPORTS_OGG_FLAC, 
//DATA(FLAC__ChannelAssignmentString, 
//DATA(FLAC__EntropyCodingMethodTypeString, 
GO(FLAC__format_blocksize_is_subset, iFuu)
GO(FLAC__format_cuesheet_is_legal, iFpip)
GO(FLAC__format_picture_is_legal, iFpp)
GO(FLAC__format_sample_rate_is_subset, iFu)
//GO(FLAC__format_sample_rate_is_valid, 
GO(FLAC__format_seektable_is_legal, iFp)
GO(FLAC__format_seektable_sort, uFp)
GO(FLAC__format_vorbiscomment_entry_is_legal, iFpu)
GO(FLAC__format_vorbiscomment_entry_name_is_legal, iFp)
//GO(FLAC__format_vorbiscomment_entry_value_is_legal, 
//DATA(FLAC__FrameNumberTypeString, 
GO(FLAC__metadata_chain_check_if_tempfile_needed, iFpi)
//GO(FLAC__metadata_chain_delete, 
//GO(FLAC__metadata_chain_merge_padding, 
GO(FLAC__metadata_chain_new, pFv)
//GO(FLAC__metadata_chain_read, 
GO(FLAC__metadata_chain_read_ogg, iFpp)
//GO(FLAC__metadata_chain_read_ogg_with_callbacks, 
GOM(FLAC__metadata_chain_read_with_callbacks, iFEppPPPPPP)   // FLAC__IOCallbacks is a struct with all callbacks
GO(FLAC__metadata_chain_sort_padding, vFp)
GO(FLAC__metadata_chain_status, uFp)
DATA(FLAC__Metadata_ChainStatusString, sizeof(void*))
GO(FLAC__metadata_chain_write, iFpii)
//GO(FLAC__metadata_chain_write_with_callbacks, 
//GO(FLAC__metadata_chain_write_with_callbacks_and_tempfile, 
GO(FLAC__metadata_get_cuesheet, iFpp)
GO(FLAC__metadata_get_picture, iFppuppuuuu)
GO(FLAC__metadata_get_streaminfo, iFpp)
//GO(FLAC__metadata_get_tags, 
GO(FLAC__metadata_iterator_delete, vFp)
GO(FLAC__metadata_iterator_delete_block, iFpi)
GO(FLAC__metadata_iterator_get_block, pFp)
GO(FLAC__metadata_iterator_get_block_type, uFp)
GO(FLAC__metadata_iterator_init, vFpp)
GO(FLAC__metadata_iterator_insert_block_after, iFpp)
GO(FLAC__metadata_iterator_insert_block_before, iFpp)
GO(FLAC__metadata_iterator_new, pFv)
GO(FLAC__metadata_iterator_next, iFp)
GO(FLAC__metadata_iterator_prev, iFp)
GO(FLAC__metadata_iterator_set_block, iFpp)
GO(FLAC__metadata_object_application_set_data, iFppui)
GO(FLAC__metadata_object_clone, pFp)
GO(FLAC__metadata_object_cuesheet_calculate_cddb_id, uFp)
GO(FLAC__metadata_object_cuesheet_delete_track, iFpu)
//GO(FLAC__metadata_object_cuesheet_insert_blank_track, 
GO(FLAC__metadata_object_cuesheet_insert_track, iFpupi)
GO(FLAC__metadata_object_cuesheet_is_legal, iFpip)
//GO(FLAC__metadata_object_cuesheet_resize_tracks, 
//GO(FLAC__metadata_object_cuesheet_set_track, 
GO(FLAC__metadata_object_cuesheet_track_clone, pFp)
GO(FLAC__metadata_object_cuesheet_track_delete, vFp)
GO(FLAC__metadata_object_cuesheet_track_delete_index, iFpuu)
//GO(FLAC__metadata_object_cuesheet_track_insert_blank_index, 
//GO(FLAC__metadata_object_cuesheet_track_insert_index, 
GO(FLAC__metadata_object_cuesheet_track_new, pFv)
//GO(FLAC__metadata_object_cuesheet_track_resize_indices, 
GO(FLAC__metadata_object_delete, vFp)
GO(FLAC__metadata_object_is_equal, iFpp)
GO(FLAC__metadata_object_new, pFu)
GO(FLAC__metadata_object_picture_is_legal, iFpp)
GO(FLAC__metadata_object_picture_set_data, iFppui)
GO(FLAC__metadata_object_picture_set_description, iFppi)
GO(FLAC__metadata_object_picture_set_mime_type, iFppi)
//GO(FLAC__metadata_object_seektable_delete_point, 
//GO(FLAC__metadata_object_seektable_insert_point, 
GO(FLAC__metadata_object_seektable_is_legal, iFp)
//GO(FLAC__metadata_object_seektable_resize_points, 
//GO(FLAC__metadata_object_seektable_set_point, 
//GO(FLAC__metadata_object_seektable_template_append_placeholders, 
GO(FLAC__metadata_object_seektable_template_append_point, iFpL)
GO(FLAC__metadata_object_seektable_template_append_points, iFppu)
//GO(FLAC__metadata_object_seektable_template_append_spaced_points, 
GO(FLAC__metadata_object_seektable_template_append_spaced_points_by_samples, iFpuL)
GO(FLAC__metadata_object_seektable_template_sort, iFpi)
//GO(FLAC__metadata_object_vorbiscomment_append_comment, 
//GO(FLAC__metadata_object_vorbiscomment_delete_comment, 
GO(FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair, iFppp)
//GO(FLAC__metadata_object_vorbiscomment_entry_matches, 
//GO(FLAC__metadata_object_vorbiscomment_entry_to_name_value_pair, 
GO(FLAC__metadata_object_vorbiscomment_find_entry_from, iFpup)
//GO(FLAC__metadata_object_vorbiscomment_insert_comment, 
GO(FLAC__metadata_object_vorbiscomment_remove_entries_matching, iFpp)
//GO(FLAC__metadata_object_vorbiscomment_remove_entry_matching, 
//GO(FLAC__metadata_object_vorbiscomment_replace_comment, 
//GO(FLAC__metadata_object_vorbiscomment_resize_comments, 
//GO(FLAC__metadata_object_vorbiscomment_set_comment, 
//GO(FLAC__metadata_object_vorbiscomment_set_vendor_string, 
GO(FLAC__metadata_simple_iterator_delete, vFp)
GO(FLAC__metadata_simple_iterator_delete_block, iFpi)
GO(FLAC__metadata_simple_iterator_get_application_id, iFpp)
GO(FLAC__metadata_simple_iterator_get_block, pFp)
GO(FLAC__metadata_simple_iterator_get_block_length, uFp)
GO(FLAC__metadata_simple_iterator_get_block_offset, lFp)
GO(FLAC__metadata_simple_iterator_get_block_type, uFp)
GO(FLAC__metadata_simple_iterator_init, iFppii)
GO(FLAC__metadata_simple_iterator_insert_block_after, iFppi)
GO(FLAC__metadata_simple_iterator_is_last, iFp)
//GO(FLAC__metadata_simple_iterator_is_writable, 
GO(FLAC__metadata_simple_iterator_new, pFv)
//GO(FLAC__metadata_simple_iterator_next, 
GO(FLAC__metadata_simple_iterator_prev, iFp)
//GO(FLAC__metadata_simple_iterator_set_block, 
GO(FLAC__metadata_simple_iterator_status, uFp)
//DATA(FLAC__Metadata_SimpleIteratorStatusString, 
//DATA(FLAC__MetadataTypeString, 
GO(FLAC__stream_decoder_delete, vFp)
DATA(FLAC__StreamDecoderErrorStatusString, sizeof(void*))
GO(FLAC__stream_decoder_finish, iFp)
GO(FLAC__stream_decoder_flush, iFp)
//GO(FLAC__stream_decoder_get_bits_per_sample, 
GO(FLAC__stream_decoder_get_blocksize, uFp)
GO(FLAC__stream_decoder_get_channel_assignment, uFp)
//GO(FLAC__stream_decoder_get_channels, 
GO(FLAC__stream_decoder_get_decode_position, iFpp)
GO(FLAC__stream_decoder_get_md5_checking, iFp)
GO(FLAC__stream_decoder_get_resolved_state_string, pFp)
//GO(FLAC__stream_decoder_get_sample_rate, 
GO(FLAC__stream_decoder_get_state, uFp)
GO(FLAC__stream_decoder_get_total_samples, LFp)
//GO(FLAC__stream_decoder_init_file, 
//GO(FLAC__stream_decoder_init_FILE, 
//GO(FLAC__stream_decoder_init_ogg_file, 
//GO(FLAC__stream_decoder_init_ogg_FILE, 
//GO(FLAC__stream_decoder_init_ogg_stream, 
//DATA(FLAC__StreamDecoderInitStatusString, 
GOM(FLAC__stream_decoder_init_stream, iFEpppppppppp)
//DATA(FLAC__StreamDecoderLengthStatusString, 
GO(FLAC__stream_decoder_new, pFv)
GO(FLAC__stream_decoder_process_single, iFp)
GO(FLAC__stream_decoder_process_until_end_of_metadata, iFp)
GO(FLAC__stream_decoder_process_until_end_of_stream, iFp)
//DATA(FLAC__StreamDecoderReadStatusString, 
GO(FLAC__stream_decoder_reset, iFp)
GO(FLAC__stream_decoder_seek_absolute, iFpL)
//DATA(FLAC__StreamDecoderSeekStatusString, 
GO(FLAC__stream_decoder_set_md5_checking, iFpi)
GO(FLAC__stream_decoder_set_metadata_ignore, iFpu)
GO(FLAC__stream_decoder_set_metadata_ignore_all, iFp)
GO(FLAC__stream_decoder_set_metadata_ignore_application, iFpp)
//GO(FLAC__stream_decoder_set_metadata_respond, 
//GO(FLAC__stream_decoder_set_metadata_respond_all, 
//GO(FLAC__stream_decoder_set_metadata_respond_application, 
GO(FLAC__stream_decoder_set_ogg_serial_number, iFpl)
GO(FLAC__stream_decoder_skip_single_frame, iFp)
//DATA(FLAC__StreamDecoderStateString, 
//DATA(FLAC__StreamDecoderTellStatusString, 
//DATA(FLAC__StreamDecoderWriteStatusString, 
GO(FLAC__stream_encoder_delete, vFp)
//GO(FLAC__stream_encoder_disable_constant_subframes, 
//GO(FLAC__stream_encoder_disable_fixed_subframes, 
//GO(FLAC__stream_encoder_disable_verbatim_subframes, 
GO(FLAC__stream_encoder_finish, iFp)
//GO(FLAC__stream_encoder_get_bits_per_sample, 
//GO(FLAC__stream_encoder_get_blocksize, 
//GO(FLAC__stream_encoder_get_channels, 
//GO(FLAC__stream_encoder_get_do_escape_coding, 
//GO(FLAC__stream_encoder_get_do_exhaustive_model_search, 
//GO(FLAC__stream_encoder_get_do_md5, 
//GO(FLAC__stream_encoder_get_do_mid_side_stereo, 
//GO(FLAC__stream_encoder_get_do_qlp_coeff_prec_search, 
//GO(FLAC__stream_encoder_get_loose_mid_side_stereo, 
//GO(FLAC__stream_encoder_get_max_lpc_order, 
//GO(FLAC__stream_encoder_get_max_residual_partition_order, 
//GO(FLAC__stream_encoder_get_min_residual_partition_order, 
//GO(FLAC__stream_encoder_get_qlp_coeff_precision, 
GO(FLAC__stream_encoder_get_resolved_state_string, pFp)
GO(FLAC__stream_encoder_get_rice_parameter_search_dist, uFp)
//GO(FLAC__stream_encoder_get_sample_rate, 
GO(FLAC__stream_encoder_get_state, uFp)
//GO(FLAC__stream_encoder_get_streamable_subset, 
GO(FLAC__stream_encoder_get_total_samples_estimate, LFp)
//GO(FLAC__stream_encoder_get_verify, 
GO(FLAC__stream_encoder_get_verify_decoder_error_stats, vFppppppp)
GO(FLAC__stream_encoder_get_verify_decoder_state, uFp)
//GO(FLAC__stream_encoder_init_file, 
//GO(FLAC__stream_encoder_init_FILE, 
//GO(FLAC__stream_encoder_init_ogg_file, 
//GO(FLAC__stream_encoder_init_ogg_FILE, 
//GO(FLAC__stream_encoder_init_ogg_stream, 
//DATA(FLAC__StreamEncoderInitStatusString, 
//GO(FLAC__stream_encoder_init_stream, 
GO(FLAC__stream_encoder_new, pFv)
GO(FLAC__stream_encoder_process, iFppu)
GO(FLAC__stream_encoder_process_interleaved, iFppu)
//DATA(FLAC__StreamEncoderReadStatusString, 
//DATA(FLAC__StreamEncoderSeekStatusString, 
GO(FLAC__stream_encoder_set_apodization, iFpp)
//GO(FLAC__stream_encoder_set_bits_per_sample, 
//GO(FLAC__stream_encoder_set_blocksize, 
//GO(FLAC__stream_encoder_set_channels, 
//GO(FLAC__stream_encoder_set_compression_level, 
//GO(FLAC__stream_encoder_set_do_escape_coding, 
//GO(FLAC__stream_encoder_set_do_exhaustive_model_search, 
//GO(FLAC__stream_encoder_set_do_md5, 
//GO(FLAC__stream_encoder_set_do_mid_side_stereo, 
//GO(FLAC__stream_encoder_set_do_qlp_coeff_prec_search, 
//GO(FLAC__stream_encoder_set_loose_mid_side_stereo, 
//GO(FLAC__stream_encoder_set_max_lpc_order, 
//GO(FLAC__stream_encoder_set_max_residual_partition_order, 
GO(FLAC__stream_encoder_set_metadata, iFppu)
//GO(FLAC__stream_encoder_set_min_residual_partition_order, 
GO(FLAC__stream_encoder_set_ogg_serial_number, iFpl)
//GO(FLAC__stream_encoder_set_qlp_coeff_precision, 
GO(FLAC__stream_encoder_set_rice_parameter_search_dist, iFpu)
//GO(FLAC__stream_encoder_set_sample_rate, 
//GO(FLAC__stream_encoder_set_streamable_subset, 
GO(FLAC__stream_encoder_set_total_samples_estimate, iFpL)
//GO(FLAC__stream_encoder_set_verify, 
//DATA(FLAC__StreamEncoderStateString, 
//DATA(FLAC__StreamEncoderTellStatusString, 
//DATA(FLAC__StreamEncoderWriteStatusString, 
//DATA(FLAC__StreamMetadata_Picture_TypeString, 
//DATA(FLAC__SubframeTypeString, 
//DATA(FLAC__VENDOR_STRING, 
//DATA(FLAC__VERSION_STRING, 
//GO(get_client_data_from_decoder, 
