#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GO(gst_riff_create_audio_caps, pFWpppppp)
GO(gst_riff_create_audio_template_caps, pFv)
GO(gst_riff_create_iavs_caps, pFuppppp)
GO(gst_riff_create_iavs_template_caps, pFv)
GO(gst_riff_create_video_caps, pFuppppp)
GO(gst_riff_create_video_template_caps, pFv)
GO(gst_riff_init, vFv)
GO(gst_riff_parse_chunk, iFppppp)
GO(gst_riff_parse_file_header, iFppp)
GO(gst_riff_parse_info, vFppp)
GO(gst_riff_parse_strf_auds, iFpppp)
GO(gst_riff_parse_strf_iavs, iFpppp)
GO(gst_riff_parse_strf_vids, iFpppp)
GO(gst_riff_parse_strh, iFppp)
GO(gst_riff_read_chunk, iFppppp)
