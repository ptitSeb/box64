#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GO(BIO_f_ssl, pFv)
GO(BIO_ADDR_free, vFp)
GO(BIO_ADDR_new, pFv)
GO(BIO_get_data, pFp)
GO(BIO_get_shutdown, iFp)
GO(BIO_new_buffer_ssl_connect, pFp)
GO(BIO_new_ssl, pFpi)
GO(BIO_new_ssl_connect, pFp)
GO(BIO_meth_free, vFp)
GO(BIO_meth_new, pFip)
GOM(BIO_meth_set_create, iFEpp)
GOM(BIO_meth_set_ctrl, iFEpp)
GOM(BIO_meth_set_destroy, iFEpp)
GOM(BIO_meth_set_puts, iFEpp)
GOM(BIO_meth_set_read, iFEpp)
GOM(BIO_meth_set_write, iFEpp)
GO(BIO_set_data, vFpp)
GO(BIO_set_init, vFpi)
GO(BIO_set_shutdown, vFpi)
GO(BIO_ssl_copy_session_id, iFpp)
GO(BIO_ssl_shutdown, vFp)
//GO(custom_ext_add, 
//GO(custom_ext_init, 
//GO(custom_ext_parse, 
//GO(custom_exts_copy, 
//GO(custom_exts_free, 
GO(d2i_SSL_SESSION, pFppl)
//GO(do_dtls1_write, 
//GO(dtls1_accept, 
//GO(dtls1_buffer_message, 
//GO(dtls1_check_timeout_num, 
//GO(dtls1_clear, 
//GO(dtls1_clear_received_buffer, 
//GO(dtls1_clear_sent_buffer, 
//GO(dtls1_connect, 
//GO(dtls1_ctrl, 
//GO(dtls1_default_timeout, 
//GO(dtls1_dispatch_alert, 
//GO(dtls1_double_timeout, 
//GO(dtls1_do_write, 
//GO(dtls1_free, 
//GO(dtls1_get_ccs_header, 
//GO(dtls1_get_cipher, 
//GO(dtls1_get_message, 
//GO(dtls1_get_message_header, 
//GO(dtls1_get_queue_priority, 
//GO(dtls1_get_record, 
//GO(dtls1_get_timeout, 
//GO(dtls1_handle_timeout, 
//GO(dtls1_heartbeat, 
//GO(dtls1_hm_fragment_free, 
//GO(dtls1_is_timer_expired, 
//GO(dtls1_link_min_mtu, 
//GO(dtls1_listen, 
//GO(dtls1_min_mtu, 
//GO(dtls1_new, 
//GO(dtls1_process_heartbeat, 
//GO(dtls1_read_bytes, 
//GO(dtls1_read_failed, 
//GO(dtls1_reset_seq_numbers, 
//GO(dtls1_retransmit_buffered_messages, 
//GO(dtls1_retransmit_message, 
//GO(dtls1_send_change_cipher_spec, 
//GO(dtls1_set_message_header, 
//GO(dtls1_shutdown, 
//GO(dtls1_start_timer, 
//GO(dtls1_stop_timer, 
//GO(dtls1_write_app_data_bytes, 
//GO(dtls1_write_bytes, 
GO(DTLS_client_method, pFv)
GO(DTLS_method, pFv)
GO(DTLS_server_method, pFv)
GO(DTLSv1_2_client_method, pFv)
GO(DTLSv1_2_method, pFv)
GO(DTLSv1_2_server_method, pFv)
GO(DTLSv1_client_method, pFv)
GO(DTLSv1_listen, iFpp)
GO(DTLSv1_method, pFv)
GO(DTLSv1_server_method, pFv)
GO(ERR_load_SSL_strings, iFv)
//GO(_fini, 
GO(i2d_SSL_SESSION, iFpp)
//GO(_init, 
//GO(n_ssl3_mac, 
//GO(OBJ_bsearch_ssl_cipher_id, 
GO(OPENSSL_init_ssl, iFUp)
//GOM(PEM_read_bio_SSL_SESSION, pFEpppp)
//GOM(PEM_read_SSL_SESSION, pFESppp)
GO(PEM_write_bio_SSL_SESSION, iFpp)
GO(PEM_write_SSL_SESSION, iFSp)
GO(SRP_Calc_A_param, iFp)
//GO(SRP_generate_client_master_secret, 
//GO(SRP_generate_server_master_secret, 
//GO(srp_verify_server_param, 
//GO(ssl23_accept, 
//GO(ssl23_connect, 
//GO(ssl23_default_timeout, 
//GO(ssl23_get_cipher, 
//GO(ssl23_get_cipher_by_char, 
//GO(ssl23_get_client_hello, 
//GO(ssl23_num_ciphers, 
//GO(ssl23_peek, 
//GO(ssl23_put_cipher_by_char, 
//GO(ssl23_read, 
//GO(ssl23_read_bytes, 
//GO(ssl23_write, 
//GO(ssl23_write_bytes, 
//GO(ssl3_accept, 
//GO(ssl3_alert_code, 
//GO(ssl3_callback_ctrl, 
//GO(ssl3_cbc_copy_mac, 
//GO(ssl3_cbc_digest_record, 
//GO(ssl3_cbc_record_digest_supported, 
//GO(ssl3_cbc_remove_padding, 
//GO(ssl3_cert_verify_mac, 
//GO(ssl3_change_cipher_state, 
//GO(ssl3_check_cert_and_algorithm, 
//GO(ssl3_choose_cipher, 
//GO(ssl3_cleanup_key_block, 
//GO(ssl3_clear, 
//GO(ssl3_client_hello, 
//GO(ssl3_comp_find, 
//GO(ssl3_connect, 
//GO(ssl3_ctrl, 
//GO(ssl3_ctx_callback_ctrl, 
//GO(ssl3_ctx_ctrl, 
//GO(ssl3_default_timeout, 
//GO(ssl3_digest_cached_records, 
//GO(ssl3_dispatch_alert, 
//GO(ssl3_do_change_cipher_spec, 
//GO(ssl3_do_compress, 
//GO(ssl3_do_uncompress, 
//GO(ssl3_do_write, 
//GO(ssl3_enc, 
//GO(ssl3_final_finish_mac, 
//GO(ssl3_finish_mac, 
//GO(ssl3_free, 
//GO(ssl3_free_digest_list, 
//GO(ssl3_generate_master_secret, 
//GO(ssl3_get_certificate_request, 
//GO(ssl3_get_cert_status, 
//GO(ssl3_get_cert_verify, 
//GO(ssl3_get_cipher, 
//GO(ssl3_get_cipher_by_char, 
//GO(ssl3_get_client_certificate, 
//GO(ssl3_get_client_hello, 
//GO(ssl3_get_client_key_exchange, 
//GO(ssl3_get_finished, 
//GO(ssl3_get_key_exchange, 
//GO(ssl3_get_message, 
//GO(ssl3_get_new_session_ticket, 
//GO(ssl3_get_next_proto, 
//GO(ssl3_get_req_cert_type, 
//GO(ssl3_get_server_certificate, 
//GO(ssl3_get_server_done, 
//GO(ssl3_get_server_hello, 
//GO(ssl3_handshake_write, 
//GO(ssl3_init_finished_mac, 
//GO(ssl3_new, 
//GO(ssl3_num_ciphers, 
//GO(ssl3_output_cert_chain, 
//GO(ssl3_peek, 
//GO(ssl3_pending, 
//GO(ssl3_put_cipher_by_char, 
//GO(ssl3_read, 
//GO(ssl3_read_bytes, 
//GO(ssl3_read_n, 
//GO(ssl3_record_sequence_update, 
//GO(ssl3_release_read_buffer, 
//GO(ssl3_release_write_buffer, 
//GO(ssl3_renegotiate, 
//GO(ssl3_renegotiate_check, 
//GO(ssl3_send_alert, 
//GO(ssl3_send_certificate_request, 
//GO(ssl3_send_cert_status, 
//GO(ssl3_send_change_cipher_spec, 
//GO(ssl3_send_client_certificate, 
//GO(ssl3_send_client_key_exchange, 
//GO(ssl3_send_client_verify, 
//GO(ssl3_send_finished, 
//GO(ssl3_send_hello_request, 
//GO(ssl3_send_newsession_ticket, 
//GO(ssl3_send_next_proto, 
//GO(ssl3_send_server_certificate, 
//GO(ssl3_send_server_done, 
//GO(ssl3_send_server_hello, 
//GO(ssl3_send_server_key_exchange, 
//GO(ssl3_set_handshake_header, 
//GO(ssl3_setup_buffers, 
//GO(ssl3_setup_key_block, 
//GO(ssl3_setup_read_buffer, 
//GO(ssl3_setup_write_buffer, 
//GO(ssl3_shutdown, 
//GO(ssl3_write, 
//GO(ssl3_write_bytes, 
//GO(ssl3_write_pending, 
GO(SSL_accept, iFp)
GOW(SSL_CTX_load_verify_file, iFpp)
GOW(SSL_has_pending, iFp)
GOW(SSL_set0_rbio, vFpp)
GOW(SSL_set0_wbio, vFpp)
GOW(SSL_set1_host, iFpp)
//GO(ssl_add_cert_chain, 
GO(SSL_add_client_CA, iFpp)
//GO(ssl_add_clienthello_renegotiate_ext, 
//GO(ssl_add_clienthello_tlsext, 
//GO(ssl_add_clienthello_use_srtp_ext, 
GO(SSL_add_dir_cert_subjects_to_stack, iFpp)
GO(SSL_add_file_cert_subjects_to_stack, iFpp)
//GO(ssl_add_serverhello_renegotiate_ext, 
//GO(ssl_add_serverhello_tlsext, 
//GO(ssl_add_serverhello_use_srtp_ext, 
GO(SSL_alert_desc_string, pFi)
GO(SSL_alert_desc_string_long, pFi)
GO(SSL_alert_type_string, pFi)
GO(SSL_alert_type_string_long, pFi)
//GO(ssl_bad_method, 
//GO(ssl_build_cert_chain, 
//GO(ssl_bytes_to_cipher_list, 
//GO(SSL_cache_hit, 
GOM(SSL_callback_ctrl, lFEpip)
//GO(ssl_cert_add0_chain_cert, 
//GO(ssl_cert_add1_chain_cert, 
//GO(ssl_cert_clear_certs, 
//GO(ssl_cert_dup, 
//GO(ssl_cert_free, 
//GO(ssl_cert_inst, 
//GO(ssl_cert_new, 
GO(SSL_certs_clear, vFp)
//GO(ssl_cert_select_current, 
//GO(ssl_cert_set0_chain, 
//GO(ssl_cert_set1_chain, 
//GO(ssl_cert_set_cert_cb, 
//GO(ssl_cert_set_cert_store, 
//GO(ssl_cert_set_current, 
//GO(ssl_cert_set_default_md, 
//GO(ssl_cert_type, 
GO(SSL_check_chain, iFpppp)
//GO(ssl_check_clienthello_tlsext_late, 
GO(SSL_check_private_key, iFp)
//GO(ssl_check_serverhello_tlsext, 
//GO(ssl_check_srvr_ecc_cert_and_alg, 
GO(SSL_CIPHER_description, pFppi)
GO(SSL_CIPHER_find, pFpp)
GO(SSL_CIPHER_get_auth_nid, iFp)
GO(SSL_CIPHER_get_bits, iFpp)
//GO(ssl_cipher_get_cert_index, 
GO(SSL_CIPHER_get_cipher_nid, iFp)
GO(SSL_CIPHER_get_digest_nid, iFp)
//GO(ssl_cipher_get_evp, 
GO(SSL_CIPHER_get_id, uFp)
GO(SSL_CIPHER_get_kx_nid, iFp)
GO(SSL_CIPHER_get_name, pFp)
GO(SSL_CIPHER_get_version, pFp)
//GO(ssl_cipher_id_cmp, 
GO(SSL_CIPHER_is_aead, iFp)
//GO(ssl_cipher_list_to_bytes, 
//GO(ssl_cipher_ptr_id_cmp, 
GO(SSL_clear, iFp)
//GO(ssl_clear_bad_session, 
//GO(ssl_clear_cipher_ctx, 
//GO(ssl_clear_hash_ctx, 
GO(SSL_COMP_add_compression_method, iFip)
//GO(SSL_COMP_free_compression_methods, 
GO(SSL_COMP_get_compression_methods, pFv)
GO(SSL_COMP_get_name, pFp)
GO(SSL_COMP_set0_compression_methods, pFp)
GO(SSL_CONF_cmd, iFppp)
GO(SSL_CONF_cmd_argv, iFppp)
GO(SSL_CONF_cmd_value_type, iFpp)
GO(SSL_CONF_CTX_clear_flags, uFpu)
GO(SSL_CONF_CTX_finish, iFp)
GO(SSL_CONF_CTX_free, vFp)
GO(SSL_CONF_CTX_new, pFv)
GO(SSL_CONF_CTX_set1_prefix, iFpp)
GO(SSL_CONF_CTX_set_flags, uFpu)
GO(SSL_CONF_CTX_set_ssl, vFpp)
GO(SSL_CONF_CTX_set_ssl_ctx, vFpp)
GO(SSL_connect, iFp)
GO(SSL_copy_session_id, iFpp)
//GO(ssl_create_cipher_list, 
GO(SSL_ctrl, lFpilp)
GO(SSL_CTX_add_client_CA, iFpp)
//GOM(SSL_CTX_add_client_custom_ext, iFEpuppppp)
//GOM(SSL_CTX_add_server_custom_ext, iFEpuppppp)
GO(SSL_CTX_add_session, iFpp)
GOM(SSL_CTX_callback_ctrl, lFEpip)
GO(SSL_CTX_check_private_key, iFp)
GO(SSL_CTX_clear_options, UFpU)
GO(SSL_CTX_config, iFpp)
GO(SSL_CTX_ctrl, lFpilp)
GO(SSL_CTX_flush_sessions, vFpl)
GO(SSL_CTX_free, vFp)
GO(SSL_CTX_get0_certificate, pFp)
GO(SSL_CTX_get0_param, pFp)
GO(SSL_CTX_get0_privatekey, pFp)
GO(SSL_CTX_get_cert_store, pFp)
GO(SSL_CTX_get_client_CA_list, pFp)
//GOM(SSL_CTX_get_client_cert_cb, pFEp)
GOM(SSL_CTX_get_default_passwd_cb, pFEp)
GO(SSL_CTX_get_default_passwd_cb_userdata, pFp)
GO(SSL_CTX_get_ex_data, pFpi)
//GO(SSL_CTX_get_ex_new_index, 
//GOM(SSL_CTX_get_info_callback, pFEp)
GO(SSL_CTX_get_num_tickets, LFp)
GO(SSL_CTX_get_options, UFp)
GO(SSL_CTX_get_quiet_shutdown, iFp)
GO(SSL_CTX_get_security_level, iFp)
GO(SSL_CTX_get_ssl_method, pFp)
GO(SSL_CTX_get_timeout, lFp)
GOM(SSL_CTX_get_verify_callback, pFEp)
GO(SSL_CTX_get_verify_depth, iFp)
GO(SSL_CTX_get_verify_mode, iFp)
GO(SSL_CTX_load_verify_dir, iFpp)
GO(SSL_CTX_load_verify_locations, iFppp)
GO(SSL_CTX_new, pFp)
GO(SSL_CTX_new_ex, pFppp)
GO(SSL_CTX_up_ref, iFp)
GO(SSL_CTX_remove_session, iFpp)
//GOM(SSL_CTX_sess_get_get_cb, pFEp)
//GOM(SSL_CTX_sess_get_new_cb, pFEp)
//GOM(SSL_CTX_sess_get_remove_cb, pFEp)
GO(SSL_CTX_sessions, pFp)
//GOM(SSL_CTX_sess_set_get_cb, vFEpp)
GOM(SSL_CTX_sess_set_new_cb, vFEpp)
//GOM(SSL_CTX_sess_set_remove_cb, vFEpp)
GO(SSL_CTX_set1_param, iFpp)
GO(SSL_CTX_set_alpn_protos, iFppu)
GOM(SSL_CTX_set_alpn_select_cb, vFEppp)
//GOM(SSL_CTX_set_cert_cb, vFEppp)
GO(SSL_CTX_set_cert_store, vFpp)
GOM(SSL_CTX_set_cert_verify_callback, vFEppp)
GO(SSL_CTX_set_ciphersuites, iFpp)
GO(SSL_CTX_set_cipher_list, iFpp)
GO(SSL_CTX_set_client_CA_list, vFpp)
GOM(SSL_CTX_set_client_cert_cb, vFEpp)
GO(SSL_CTX_set_client_cert_engine, iFpp)
GOM(SSL_CTX_set_cookie_generate_cb, vFEpp)
GOM(SSL_CTX_set_cookie_verify_cb, vFEpp)
GOM(SSL_CTX_set_default_passwd_cb, vFEpp)
GO(SSL_CTX_set_default_passwd_cb_userdata, vFpp)
GO(SSL_CTX_set_default_verify_paths, iFp)
GO(SSL_CTX_set_default_read_buffer_len, vFpL)
GO(SSL_CTX_set_ex_data, iFpip)
//GOM(SSL_CTX_set_generate_session_id, iFEpp)
GOM(SSL_CTX_set_info_callback, vFEpp)
GOM(SSL_CTX_set_keylog_callback, vFEpp)
GOM(SSL_CTX_set_msg_callback, vFEpp)
//GOM(SSL_CTX_set_next_protos_advertised_cb, vFEppp)
GOM(SSL_CTX_set_next_proto_select_cb, vFEppp)
GO(SSL_CTX_set_num_tickets, iFpL)
GO(SSL_CTX_set_options, UFpU)
GO(SSL_CTX_set_post_handshake_auth, vFpi)
GOM(SSL_CTX_set_psk_client_callback, vFEpp)
GOM(SSL_CTX_set_psk_server_callback, vFEpp)
GO(SSL_CTX_set_purpose, iFpi)
GO(SSL_CTX_set_quiet_shutdown, vFpi)
GO(SSL_CTX_set_security_level, vFpi)
GO(SSL_CTX_set_session_id_context, iFppu)
GO(SSL_CTX_set_srp_cb_arg, iFpp)
//GOM(SSL_CTX_set_srp_client_pwd_callback, iFEpp)
GO(SSL_CTX_set_srp_password, iFpp)
GO(SSL_CTX_set_srp_strength, iFpi)
GO(SSL_CTX_set_srp_username, iFpp)
//GOM(SSL_CTX_set_srp_username_callback, iFEpp)
//GOM(SSL_CTX_set_srp_verify_param_callback, iFEpp)
GO(SSL_CTX_set_ssl_version, iFpp)
GO(SSL_CTX_set_timeout, lFpl)
GO(SSL_CTX_set_tlsext_use_srtp, iFpp)
//GOM(SSL_CTX_set_tmp_dh_callback, vFEpp)
GO(SSL_CTX_set0_tmp_dh_pkey, iFpp)
//GO(SSL_CTX_set_tmp_ecdh_callback, 
//GO(SSL_CTX_set_tmp_rsa_callback, 
GO(SSL_CTX_set_trust, iFpi)
GOM(SSL_CTX_set_verify, vFEpip)
GO(SSL_CTX_set_verify_depth, vFpi)
GO(SSL_CTX_SRP_CTX_free, iFp)
GO(SSL_CTX_SRP_CTX_init, iFp)
GO(SSL_CTX_use_certificate, iFpp)
GO(SSL_CTX_use_certificate_ASN1, iFpip)
GO(SSL_CTX_use_certificate_chain_file, iFpp)
GO(SSL_CTX_use_certificate_file, iFppi)
GO(SSL_CTX_use_PrivateKey, iFpp)
GO(SSL_CTX_use_PrivateKey_ASN1, iFippl)
GO(SSL_CTX_use_PrivateKey_file, iFppi)
GO(SSL_CTX_use_psk_identity_hint, iFpp)
GO(SSL_CTX_use_RSAPrivateKey, iFpp)
GO(SSL_CTX_use_RSAPrivateKey_ASN1, iFppl)
GO(SSL_CTX_use_RSAPrivateKey_file, iFppi)
GO(SSL_CTX_use_serverinfo, iFppL)
GO(SSL_CTX_use_serverinfo_file, iFpp)
//GO(ssl_do_client_cert_cb, 
GO(SSL_do_handshake, iFp)
GO(SSL_dup, pFp)
GO(SSL_dup_CA_list, pFp)
GO(SSL_export_keying_material, iFppLpLpLi)
GO(SSL_extension_supported, iFu)
//GO(ssl_fill_hello_random, 
GO(SSL_free, vFp)
//GO(ssl_free_wbio_buffer, 
GO(SSL_get0_alpn_selected, vFppp)
GO(SSL_get0_next_proto_negotiated, vFppp)
GO(SSL_get0_param, pFp)
GO(SSL_get0_group_name, pFp)
GO(SSL_get0_peer_certificate, pFp)
GO(SSL_get0_session, pFp) // Warning: failed to confirm
GO(SSL_get0_verified_chain, pFp)
GO(SSL_get1_peer_certificate, pFp)
GO(SSL_get1_session, pFp)
//GO(ssl_get_algorithm2, 
GO(SSL_get_certificate, pFp)
//GO(ssl_get_cipher_by_char, 
GO(SSL_get_cipher_list, pFpi)
GO(SSL_get_ciphers, pFp)
//GO(ssl_get_ciphers_by_id, 
GO(SSL_get_client_CA_list, pFp)
GO(SSL_get_client_ciphers, pFp)
GO(SSL_get_client_random, LFppL)
GO(SSL_get_current_cipher, pFp)
GO(SSL_get_current_compression, pFp)
GO(SSL_get_current_expansion, pFp)
GO(SSL_get_default_timeout, lFp)
GO(SSL_get_error, iFpi)
GO(SSL_get_early_data_status, iFp)
GO(SSL_get_ex_data, pFpi)
GO(SSL_get_ex_data_X509_STORE_CTX_idx, iFv)
GOM(SSL_get_ex_new_index, iFElpppp) // Warning: failed to confirm
GO(SSL_get_fd, iFp)
GO(SSL_get_finished, LFppL)
//GO(ssl_get_handshake_digest, 
//GOM(SSL_get_info_callback, pFEp)
//GO(ssl_get_new_session, 
GO(SSL_get_peer_cert_chain, pFp)
GO(SSL_get_peer_signature_type_nid, iFp)
GO(SSL_get_peer_finished, LFppL)
//GO(ssl_get_prev_session, 
GO(SSL_get_privatekey, pFp)
GO(SSL_get_psk_identity, pFp)
GO(SSL_get_psk_identity_hint, pFp)
GO(SSL_get_quiet_shutdown, iFp)
GO(SSL_get_rbio, pFp)
GO(SSL_get_read_ahead, iFp)
GO(SSL_get_rfd, iFp)
GO(SSL_get_selected_srtp_profile, pFp)
//GO(ssl_get_server_cert_serverinfo, 
GO(SSL_get_servername, pFpi)
GO(SSL_get_servername_type, iFp)
//GO(ssl_get_server_send_pkey, 
GO(SSL_get_session, pFp)
GO(SSL_get_shared_ciphers, pFppi)
GO(SSL_get_shared_sigalgs, iFpippppp)
GO(SSL_get_shutdown, iFp)
GO(SSL_get_sigalgs, iFpippppp)
//GO(ssl_get_sign_pkey, 
GO(SSL_get_srp_g, pFp)
GO(SSL_get_srp_N, pFp)
GO(SSL_get_srp_userinfo, pFp)
GO(SSL_get_srp_username, pFp)
GO(SSL_get_srtp_profiles, pFp)
GO(SSL_get_SSL_CTX, pFp)
GO(SSL_get_ssl_method, pFp)
GOM(SSL_get_verify_callback, pFEp)
GO(SSL_get_verify_depth, iFp)
GO(SSL_get_verify_mode, iFp)
GO(SSL_get_verify_result, lFp)
GO(SSL_get_version, pFp)
GO(SSL_get_wbio, pFp)
GO(SSL_get_wfd, iFp)
GO(SSL_has_matching_session_id, iFppu)
GO(SSL_in_init, iFp)
//GO(ssl_init_wbio_buffer, 
GO(SSL_is_init_finished, iFp)
GO(SSL_is_server, iFp)
GO(SSL_library_init, iFv)
//GO(ssl_load_ciphers, 
GO(SSL_load_client_CA_file, pFp)
GO(SSL_load_error_strings, vFv)
GO(SSL_new, pFp)
//GO(ssl_ok, 
//GO(ssl_parse_clienthello_renegotiate_ext, 
//GO(ssl_parse_clienthello_tlsext, 
//GO(ssl_parse_clienthello_use_srtp_ext, 
//GO(ssl_parse_serverhello_renegotiate_ext, 
//GO(ssl_parse_serverhello_tlsext, 
//GO(ssl_parse_serverhello_use_srtp_ext, 
GO(SSL_peek, iFppi)
GO(SSL_pending, iFp)
//GO(ssl_prepare_clienthello_tlsext, 
//GO(ssl_prepare_serverhello_tlsext, 
GO(SSL_read, iFppi)
GO(SSL_read_ex, iFppLp)
GO(SSL_renegotiate, iFp)
GO(SSL_renegotiate_abbreviated, iFp)
GO(SSL_renegotiate_pending, iFp)
//GO(ssl_replace_hash, 
GO(SSL_rstate_string, pFp)
GO(SSL_rstate_string_long, pFp)
GO(SSL_select_next_proto, iFpppupu)
//GO(ssl_sess_cert_free, 
//GO(ssl_sess_cert_new, 
GO(ssl_session_dup, pFp)
GO(SSL_SESSION_dup, pFp)
GO(SSL_SESSION_free, vFp)
GO(SSL_SESSION_get0_peer, pFp)
GO(SSL_SESSION_get_compress_id, uFp)
GO(SSL_SESSION_get_ex_data, pFpi)
//GO(SSL_SESSION_get_ex_new_index, 
GO(SSL_SESSION_get_id, pFpp)
GO(SSL_SESSION_get_master_key, LFppL)
GO(SSL_SESSION_get_max_early_data, uFp)
GO(SSL_SESSION_get_ticket_lifetime_hint, LFp)
GO(SSL_SESSION_get_time, lFp)
GO(SSL_SESSION_get_timeout, lFp)
GO(SSL_SESSION_get_time_ex, LFp)
GO(SSL_SESSION_has_ticket, iFp)
GO(SSL_SESSION_is_resumable, iFp)
GO(SSL_SESSION_new, pFv)
GO(SSL_SESSION_print, iFpp)
GO(SSL_SESSION_print_fp, iFSp)
GO(SSL_session_reused, iFp)
GO(SSL_SESSION_set1_id_context, iFppu)
GO(SSL_SESSION_set_ex_data, iFpip)
GO(SSL_SESSION_set_time, lFpl)
GO(SSL_SESSION_set_timeout, lFpl)
GO(SSL_SESSION_up_ref, iFp)
GO(SSL_set1_param, iFpp)
GO(SSL_set_accept_state, vFp)
GO(SSL_set_alpn_protos, iFppu)
GO(SSL_set_bio, vFppp)
//GOM(SSL_set_cert_cb, vFEppp)
//GO(ssl_set_cert_masks, 
GO(SSL_set_cipher_list, iFpp)
GO(SSL_set_ciphersuites, iFpp)
GO(SSL_set_client_CA_list, vFpp)
//GO(ssl_set_client_disabled, 
GO(SSL_set_connect_state, vFp)
GO(SSL_set_debug, vFpi)
GO(SSL_set_ex_data, iFpip)
GO(SSL_set_fd, iFpi)
//GOM(SSL_set_generate_session_id, iFEpp)
GOM(SSL_set_info_callback, vFEpp)
GOM(SSL_set_msg_callback, vFEpp)
GO(SSL_set_options, UFpU)
//GO(ssl_set_peer_cert_type, 
GO(SSL_set_post_handshake_auth, vFpi)
GOM(SSL_set_psk_client_callback, vFEpp)
GOM(SSL_set_psk_use_session_callback, vFEpp)
GOM(SSL_set_psk_server_callback, vFEpp)
GO(SSL_set_purpose, iFpi)
GO(SSL_set_quiet_shutdown, vFpi)
GO(SSL_set_read_ahead, vFpi)
GO(SSL_set_rfd, iFpi)
GO(SSL_set_session, iFpp)
GO(SSL_set_session_id_context, iFppu)
//GOM(SSL_set_session_secret_cb, iFEppp)
GO(SSL_set_session_ticket_ext, iFppi)
//GOM(SSL_set_session_ticket_ext_cb, iFEppp)
GO(SSL_set_shutdown, vFpi)
GO(SSL_set_srp_server_param, iFpppppp)
GO(SSL_set_srp_server_param_pw, iFpppp)
GO(SSL_set_SSL_CTX, pFpp)
GO(SSL_set_ssl_method, iFpp)
//GO(SSL_set_state, 
GO(SSL_set_tlsext_use_srtp, iFpp)
//GOM(SSL_set_tmp_dh_callback, vFEpp)
//GO(SSL_set_tmp_ecdh_callback, 
//GO(SSL_set_tmp_rsa_callback, 
GO(SSL_set_trust, iFpi)
GOM(SSL_set_verify, vFEpip)
GO(SSL_set_verify_depth, vFpi)
GO(SSL_set_verify_result, vFpl)
GO(SSL_set_wfd, iFpi)
GO(SSL_shutdown, iFp)
GO(SSL_SRP_CTX_free, iFp)
GO(SSL_SRP_CTX_init, iFp)
GO(SSL_srp_server_param_with_username, iFpp)
GO(SSL_state, iFp)
GO(SSL_state_string, pFp)
GO(SSL_state_string_long, pFp)
//GO(ssl_undefined_const_function, 
//GO(ssl_undefined_function, 
//GO(ssl_undefined_void_function, 
//GO(ssl_update_cache, 
GO(SSL_use_certificate, iFpp)
GO(SSL_use_certificate_ASN1, iFppi)
GO(SSL_use_certificate_file, iFppi)
GO(SSL_use_PrivateKey, iFpp)
GO(SSL_use_PrivateKey_ASN1, iFippl)
GO(SSL_use_PrivateKey_file, iFppi)
GO(SSL_use_psk_identity_hint, iFpp)
GO(SSL_use_RSAPrivateKey, iFpp)
GO(SSL_use_RSAPrivateKey_ASN1, iFppl)
GO(SSL_use_RSAPrivateKey_file, iFppi)
GO(SSL_verify_client_post_handshake, iFp)
GO(SSLv23_client_method, pFv)
GO(SSLv23_method, pFv)
GO(SSLv23_server_method, pFv)
GO(SSLv2_client_method, pFv)
GO(SSLv2_method, pFv)
GO(SSLv2_server_method, pFv)
GO(SSLv3_client_method, pFv)
GO(SSLv3_method, pFv)
GO(SSLv3_server_method, pFv)
//GO(ssl_verify_alarm_type, 
//GO(ssl_verify_cert_chain, 
GO(SSL_version, iFp)
GO(SSL_want, iFp)
GO(SSL_write, iFppi)
GO(SSL_write_ex, iFppLp)
GO(SSL_write_early_data, iFppLp)
//GO(tls12_check_peer_sigalg, 
//GO(tls12_get_hash, 
//GO(tls12_get_psigalgs, 
//GO(tls12_get_sigandhash, 
//GO(tls12_get_sigid, 
//GO(tls1_alert_code, 
//GO(tls1_cbc_remove_padding, 
//GO(tls1_cert_verify_mac, 
//GO(tls1_change_cipher_state, 
//GO(tls1_check_chain, 
//GO(tls1_check_curve, 
//GO(tls1_check_ec_tmp_key, 
//GO(tls1_clear, 
//GO(tls1_default_timeout, 
//GO(tls1_ec_curve_id2nid, 
//GO(tls1_ec_nid2curve_id, 
//GO(tls1_enc, 
//GO(tls1_export_keying_material, 
//GO(tls1_final_finish_mac, 
//GO(tls1_free, 
//GO(tls1_generate_master_secret, 
//GO(tls1_heartbeat, 
//GO(tls1_mac, 
//GO(tls1_new, 
//GO(tls1_process_heartbeat, 
//GO(tls1_process_sigalgs, 
//GO(tls1_process_ticket, 
//GO(tls1_save_sigalgs, 
//GO(tls1_set_cert_validity, 
//GO(tls1_set_curves, 
//GO(tls1_set_curves_list, 
//GO(tls1_set_server_sigalgs, 
//GO(tls1_set_sigalgs, 
//GO(tls1_set_sigalgs_list, 
//GO(tls1_setup_key_block, 
//GO(tls1_shared_curve, 
GO(TLS_method, pFv)
GO(TLS_server_method, pFv)
GO(TLS_client_method, pFv)
GO(TLSv1_1_client_method, pFv)
GO(TLSv1_1_method, pFv)
GO(TLSv1_1_server_method, pFv)
GO(TLSv1_2_client_method, pFv)
GO(TLSv1_2_method, pFv)
GO(TLSv1_2_server_method, pFv)
GO(TLSv1_client_method, pFv)
GO(TLSv1_method, pFv)
GO(verify_callback, iFip)

GO(TLSv1_server_method, pFv)    // not always present in lib
