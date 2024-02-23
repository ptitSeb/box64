#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

//GO(ber_alloc, 
GO(ber_alloc_t, pFi)
//GO(ber_bprint, 
GO(ber_bvarray_add, vFpp)
//GO(ber_bvarray_add_x, 
//GO(ber_bvarray_dup_x, 
GO(ber_bvarray_free, vFp)
//GO(ber_bvarray_free_x, 
GO(ber_bvdup, pFp)
GO(ber_bvecadd, vFpp)
//GO(ber_bvecadd_x, 
GO(ber_bvecfree, vFp)
//GO(ber_bvecfree_x, 
GO(ber_bvfree, vFp)
//GO(ber_bvfree_x, 
//GO(ber_bvreplace, 
//GO(ber_bvreplace_x, 
GO(ber_bvstr, pFp)
GO(ber_bvstrdup, pFp)
//GO(ber_decode_oid, 
//GO(ber_dump, 
//GO(ber_dup, 
GO(ber_dupbv, pFpp)
//GO(ber_dupbv_x, 
//GO(ber_encode_oid, 
//GO(ber_errno_addr, 
//GO(ber_error_print, 
GO(ber_first_element, LFppp)
GO(ber_flatten, iFpp)
//GO(ber_flatten2, 
GO(ber_flush, iFppi)
GO(ber_flush2, iFppi)
GO(ber_free, vFpi)
//GO(ber_free_buf, 
GO(ber_get_bitstringa, LFppp)
GO(ber_get_boolean, LFpp)
GO(ber_get_enum, iFpp)
GO(ber_get_int, iFpp)
GO(ber_get_next, LFppp)
GO(ber_get_null, LFp)
//GO(ber_get_option, 
GO(ber_get_stringa, LFpp)
GO(ber_get_stringal, LFpp)
//GO(ber_get_stringa_null, 
GO(ber_get_stringb, LFppp)
GO(ber_get_stringbv, LFppi)
//GO(ber_get_stringbv_null, 
//GO(ber_get_tag, 
GO(ber_init, pFp)
GO(ber_init2, vFppi)
//GO(ber_init_w_nullc, 
//DATAB(ber_int_errno_fn, 
//DATAB(ber_int_log_proc, 
//DATAB(ber_int_memory_fns, 
//DATAB(ber_int_options, 
//GO(ber_int_sb_close, 
//GO(ber_int_sb_destroy, 
//GO(ber_int_sb_init, 
//GO(ber_int_sb_read, 
//GO(ber_int_sb_write, 
//GO(ber_len, 
//GO(ber_log_bprint, 
//GO(ber_log_dump, 
//GO(ber_log_sos_dump, 
//GO(ber_mem2bv, 
//GO(ber_mem2bv_x, 
//GO(ber_memalloc, 
//GO(ber_memalloc_x, 
//GO(ber_memcalloc, 
//GO(ber_memcalloc_x, 
GO(ber_memfree, vFp)
//GO(ber_memfree_x, 
//GO(ber_memrealloc, 
//GO(ber_memrealloc_x, 
//GO(ber_memvfree, 
//GO(ber_memvfree_x, 
GO(ber_next_element, LFppp)
//GO(ber_peek_element, 
GO(ber_peek_tag, LFpp)
GO(ber_printf, iFpppppppppppppppppppppppppppppppppp)    //VAArg. May need some wrapping...
//GO(ber_ptrlen, 
//GO(ber_put_berval, 
GO(ber_put_bitstring, iFppLL)
GO(ber_put_boolean, iFpiL)
GO(ber_put_enum, iFpiL)
GO(ber_put_int, iFpiL)
GO(ber_put_null, iFpL)
GO(ber_put_ostring, iFppLL)
GO(ber_put_seq, iFp)
GO(ber_put_set, iFp)
GO(ber_put_string, iFppL)
//DATAB(ber_pvt_err_file, 
//GO(ber_pvt_log_output, 
//DATA(ber_pvt_log_print, 
//GO(ber_pvt_log_printf, 
//DATAB(ber_pvt_opt_on, 
//GO(ber_pvt_sb_buf_destroy, 
//GO(ber_pvt_sb_buf_init, 
//GO(ber_pvt_sb_copy_out, 
//GO(ber_pvt_sb_do_write, 
//GO(ber_pvt_sb_grow_buffer, 
//GO(ber_pvt_socket_set_nonblock, 
//GO(ber_read, 
//GO(ber_realloc, 
//GO(ber_remaining, 
//GO(ber_reset, 
//GO(ber_rewind, 
GO(ber_scanf, LFpppppppppppppppppp) // VAArg
//GO(ber_set_option, 
//GO(ber_skip_data, 
//GO(ber_skip_element, 
GO(ber_skip_tag, LFpp)
GOM(ber_sockbuf_add_io, iFEppip)
//GO(ber_sockbuf_alloc, 
//GO(ber_sockbuf_ctrl, 
//GO(ber_sockbuf_free, 
//DATA(ber_sockbuf_io_debug, 
//DATA(ber_sockbuf_io_fd, 
//DATA(ber_sockbuf_io_readahead, 
//DATA(ber_sockbuf_io_tcp, 
//GO(ber_sockbuf_remove_io, 
//GO(ber_sos_dump, 
//GO(ber_start, 
GO(ber_start_seq, iFpL)
GO(ber_start_set, iFpL)
GO(ber_str2bv, pFpLip)
//GO(ber_str2bv_x, 
//GO(ber_strdup, 
//GO(ber_strdup_x, 
//GO(ber_strndup, 
//GO(ber_strndup_x, 
//GO(ber_strnlen, 
//GO(ber_write, 
//GO(der_alloc, 
//GO(lutil_debug, 
//GO(lutil_debug_file, 
