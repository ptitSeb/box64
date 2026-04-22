#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//oggpack functions - all take oggpack_buffer*
GOM(oggpack_adv, vFEpi)
//GO(oggpack_adv1,
//GO(oggpackB_adv,
//GO(oggpackB_adv1,
//GO(oggpackB_bits,
GOM(oggpackB_bytes, iFEp)
GOM(oggpackB_get_buffer, pFEp)
//GO(oggpack_bits,
//GO(oggpackB_look,
//GO(oggpackB_look1,
GOM(oggpackB_read, lFEpi)
//GO(oggpackB_read1,
GOM(oggpackB_readinit, vFEppi)
GOM(oggpackB_reset, vFEp)
GOM(oggpackB_write, vFEpui)
//GO(oggpackB_writealign,
GOM(oggpackB_writecheck, iFEp)
GOM(oggpackB_writeclear, vFEp)
//GO(oggpackB_writecopy,
GOM(oggpackB_writeinit, vFEp)
//GO(oggpackB_writetrunc,
GOM(oggpack_bytes, lFEp)
//GO(ogg_packet_clear,
GOM(oggpack_get_buffer, pFEp)
GOM(oggpack_look, lFEpi)
//GO(oggpack_look1,
GOM(oggpack_read, lFEpi)
//GO(oggpack_read1,
GOM(oggpack_readinit, vFEppi)
GOM(oggpack_reset, vFEp)
GOM(oggpack_write, vFEpLi)
//GO(oggpack_writealign,
GOM(oggpack_writecheck, iFEp)
GOM(oggpack_writeclear, vFEp)
//GO(oggpack_writecopy,
GOM(oggpack_writeinit, vFEp)
GOM(oggpack_writetrunc, vFEpl)
//ogg_page functions - all take ogg_page*
GOM(ogg_page_bos, iFEp)
GOM(ogg_page_checksum_set, iFEp)
GOM(ogg_page_continued, iFEp)
GOM(ogg_page_eos, iFEp)
GOM(ogg_page_granulepos, IFEp)
GOM(ogg_page_packets, iFEp)
GOM(ogg_page_pageno, lFEp)
GOM(ogg_page_serialno, iFEp)
//GO(ogg_page_version,
//ogg_stream functions - take ogg_stream_state* and optionally ogg_page*/ogg_packet*
//GO(ogg_stream_check,
GOM(ogg_stream_clear, iFEp)
//GO(ogg_stream_destroy,
//GO(ogg_stream_eos,
GOM(ogg_stream_flush, iFEpp)
GOM(ogg_stream_flush_fill, iFEppi)
GOM(ogg_stream_init, iFEpi)
//GO(ogg_stream_iovecin,
GOM(ogg_stream_packetin, iFEpp)
GOM(ogg_stream_packetout, iFEpp)
GOM(ogg_stream_packetpeek, iFEpp)
GOM(ogg_stream_pagein, iFEpp)
GOM(ogg_stream_pageout, iFEpp)
GOM(ogg_stream_pageout_fill, iFEppi)
GOM(ogg_stream_reset, iFEp)
GOM(ogg_stream_reset_serialno, iFEpi)
//ogg_sync functions - take ogg_sync_state* and optionally ogg_page*
GOM(ogg_sync_buffer, pFEpi)
//GO(ogg_sync_check,
GOM(ogg_sync_clear, iFEp)
//GO(ogg_sync_destroy,
GOM(ogg_sync_init, iFEp)
GOM(ogg_sync_pageout, iFEpp)
GOM(ogg_sync_pageseek, iFEpp)
GOM(ogg_sync_reset, iFEp)
GOM(ogg_sync_wrote, iFEpi)
