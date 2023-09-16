#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

// ogg_stream_state doesn't need align :)
// oggpack_buffer probably doesn't need align

GO(oggpack_adv, vFpi)
//GO(oggpack_adv1, 
//GO(oggpackB_adv, 
//GO(oggpackB_adv1, 
//GO(oggpackB_bits, 
GO(oggpackB_bytes, iFp)
GO(oggpackB_get_buffer, pFp)
//GO(oggpack_bits, 
//GO(oggpackB_look, 
//GO(oggpackB_look1, 
GO(oggpackB_read, lFpi)
//GO(oggpackB_read1, 
GO(oggpackB_readinit, vFppi)
GO(oggpackB_reset, vFp)
GO(oggpackB_write, vFpui)
//GO(oggpackB_writealign, 
//GO(oggpackB_writecheck, 
GO(oggpackB_writeclear, vFp)
//GO(oggpackB_writecopy, 
GO(oggpackB_writeinit, vFp)
//GO(oggpackB_writetrunc, 
GO(oggpack_bytes, lFp)
//GO(ogg_packet_clear, 
GO(oggpack_get_buffer, pFp)
GO(oggpack_look, lFpi)
//GO(oggpack_look1, 
GO(oggpack_read, lFpi)
//GO(oggpack_read1, 
GO(oggpack_readinit, vFppi)
GO(oggpack_reset, vFp)
GO(oggpack_write, vFpui)
//GO(oggpack_writealign, 
//GO(oggpack_writecheck, 
GO(oggpack_writeclear, vFp)
//GO(oggpack_writecopy, 
GO(oggpack_writeinit, vFp)
GO(oggpack_writetrunc, vFpl)
GO(ogg_page_bos, iFp)
GO(ogg_page_checksum_set, iFp)
GO(ogg_page_continued, iFp)
GO(ogg_page_eos, iFp)
GO(ogg_page_granulepos, IFp)
GO(ogg_page_packets, iFp)
GO(ogg_page_pageno, lFp)
GO(ogg_page_serialno, iFp)
//GO(ogg_page_version, 
//GO(ogg_stream_check, 
GO(ogg_stream_clear, iFp)
//GO(ogg_stream_destroy, 
//GO(ogg_stream_eos, 
GO(ogg_stream_flush, iFpp)
GO(ogg_stream_flush_fill, iFppi)
GO(ogg_stream_init, iFpi)
//GO(ogg_stream_iovecin, 
GO(ogg_stream_packetin, iFpp)
GO(ogg_stream_packetout, iFpp)
GO(ogg_stream_packetpeek, iFpp)
GO(ogg_stream_pagein, iFpp)
GO(ogg_stream_pageout, iFpp)
//GO(ogg_stream_pageout_fill, 
GO(ogg_stream_reset, iFp)
GO(ogg_stream_reset_serialno, iFpi)
GO(ogg_sync_buffer, pFpi)
//GO(ogg_sync_check, 
GO(ogg_sync_clear, iFp)
//GO(ogg_sync_destroy, 
GO(ogg_sync_init, iFp)
GO(ogg_sync_pageout, iFpp)
GO(ogg_sync_pageseek, iFpp)
GO(ogg_sync_reset, iFp)
GO(ogg_sync_wrote, iFpi)
