#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

//DATA(_nettle_aeads, 
//DATA(nettle_aes128, 
GO(nettle_aes128_decrypt, vFpLpp)
//GO(_nettle_aes128_decrypt_c, 
GO(nettle_aes128_encrypt, vFpipp)
//GO(_nettle_aes128_encrypt_c, 
GO(nettle_aes128_invert_key, vFpp)
GO(nettle_aes128_keyunwrap, iFppLpp)
GO(nettle_aes128_keywrap, vFppLpp)
GO(nettle_aes128_set_decrypt_key, vFpp)
GO(nettle_aes128_set_encrypt_key, vFpp)
//DATA(nettle_aes192, 
GO(nettle_aes192_decrypt, vFpLpp)
//GO(_nettle_aes192_decrypt_c, 
GO(nettle_aes192_encrypt, vFpipp)
//GO(_nettle_aes192_encrypt_c, 
GO(nettle_aes192_invert_key, vFpp)
GO(nettle_aes192_keyunwrap, iFppLpp)
GO(nettle_aes192_keywrap, vFppLpp)
GO(nettle_aes192_set_decrypt_key, vFpp)
GO(nettle_aes192_set_encrypt_key, vFpp)
//DATA(nettle_aes256, 
GO(nettle_aes256_decrypt, vFpLpp)
//GO(_nettle_aes256_decrypt_c, 
GO(nettle_aes256_encrypt, vFpipp)
//GO(_nettle_aes256_encrypt_c, 
GO(nettle_aes256_invert_key, vFpp)
GO(nettle_aes256_keyunwrap, iFppLpp)
GO(nettle_aes256_keywrap, vFppLpp)
GO(nettle_aes256_set_decrypt_key, vFpp)
GO(nettle_aes256_set_encrypt_key, vFpp)
//GO(_nettle_aes_decrypt, 
GO(nettle_aes_decrypt, vFpLpp)
//GO(_nettle_aes_encrypt, 
//GO(nettle_aes_encrypt, 
//GO(_nettle_aes_invert, 
GO(nettle_aes_invert_key, vFpp)
GO(nettle_aes_set_decrypt_key, vFpLp)
//GO(nettle_aes_set_encrypt_key, 
//GO(_nettle_aes_set_key, 
GO(nettle_arcfour_crypt, vFpLpp)
GO(nettle_arcfour_set_key, vFpLp)
//DATA(nettle_arctwo128, 
GO(nettle_arcfour128_set_key, vFpp)
GO(nettle_arctwo_decrypt, vFpLpp)
GO(nettle_arctwo_encrypt, vFpipp)
//GO(nettle_arctwo_set_key, 
GO(nettle_arctwo_set_key_ekb, vFpLpu)
GO(nettle_arctwo_set_key_gutmann, vFpLp)
//DATA(nettle_arctwo_gutmann128, 
//GO(nettle_arctwo128_set_key, 
GO(nettle_arctwo128_set_key_gutmann, vFpp)
//DATA(nettle_arctwo40, 
GO(nettle_arctwo40_set_key, vFpp)
//DATA(nettle_arctwo64, 
//GO(nettle_arctwo64_set_key, 
//DATA(_nettle_armors, 
//GO(nettle_balloon, 
GO(nettle_balloon_itch, LFLL)
//GO(nettle_balloon_sha1, 
//GO(nettle_balloon_sha256, 
//GO(nettle_balloon_sha384, 
GO(nettle_balloon_sha512, vFLLLpLppp)
//DATA(nettle_base16, 
GO(nettle_base16_decode_final, iFp)
GO(nettle_base16_decode_init, vFp)
GO(nettle_base16_decode_single, iFppc)
GO(nettle_base16_decode_update, iFpppLp)
GO(nettle_base16_encode_single, vFpC)
//GO(nettle_base16_encode_update, 
//DATA(nettle_base64, 
GO(nettle_base64_decode_final, iFp)
GO(nettle_base64_decode_init, vFp)
GO(nettle_base64_decode_single, iFppc)
GO(nettle_base64_decode_update, iFpppLp)
GO(nettle_base64_encode_final, LFpp)
GO(nettle_base64_encode_group, vFpu)
//GO(nettle_base64_encode_init, 
GO(nettle_base64_encode_raw, vFpLp)
GO(nettle_base64_encode_single, LFppC)
GO(nettle_base64_encode_update, LFppLp)
//DATA(nettle_base64url, 
GO(nettle_base64url_decode_init, vFp)
GO(nettle_base64url_encode_init, vFp)
GO(nettle_blowfish128_set_key, iFpp)
GO(nettle_blowfish_bcrypt_hash, iFpLpLpip)
GO(nettle_blowfish_bcrypt_verify, iFLpLp)
GO(nettle_blowfish_decrypt, vFpLpp)
//GO(_nettle_blowfish_encround, 
//GO(nettle_blowfish_encrypt, 
GO(nettle_blowfish_set_key, iFpLp)
//GO(nettle_buffer_clear, 
//GO(nettle_buffer_copy, 
//GO(nettle_buffer_grow, 
//GO(nettle_buffer_init, 
//GO(nettle_buffer_init_realloc, 
//GO(nettle_buffer_init_size, 
//GO(nettle_buffer_reset, 
//GO(nettle_buffer_space, 
//GO(nettle_buffer_write, 
//GO(_nettle_camellia_absorb, 
//GO(_nettle_camellia_crypt, 
//GO(_nettle_camellia_invert_key, 
GO(nettle_camellia_set_decrypt_key, vFpp)
//DATA(nettle_camellia128, 
GO(nettle_camellia128_crypt, vFpLpp)
GO(nettle_camellia128_invert_key, vFpp)
GO(nettle_camellia128_set_decrypt_key, vFpp)
GO(nettle_camellia128_set_encrypt_key, vFpp)
//DATA(nettle_camellia192, 
GO(nettle_camellia192_set_decrypt_key, vFpp)
GO(nettle_camellia192_set_encrypt_key, vFpp)
//DATA(nettle_camellia256, 
GO(nettle_camellia256_crypt, vFpLpp)
GO(nettle_camellia256_invert_key, vFpp)
GO(nettle_camellia256_set_decrypt_key, vFpp)
GO(nettle_camellia256_set_encrypt_key, vFpp)
//DATA(nettle_cast128, 
GO(nettle_cast128_decrypt, vFpLpp)
//GO(nettle_cast128_encrypt, 
GO(nettle_cast128_set_key, vFpp)
GO(nettle_cast5_set_key, vFpLp)
GO(nettle_cbc_aes128_encrypt, vFppLpp)
GO(nettle_cbc_aes192_encrypt, vFppLpp)
GO(nettle_cbc_aes256_encrypt, vFppLpp)
GOM(nettle_cbc_decrypt, vFEppLpLpp)
GOM(nettle_cbc_encrypt, vFEppLpLpp)
GO(nettle_ccm_aes128_decrypt, vFpLpp)
GO(nettle_ccm_aes128_decrypt_message, iFpLpLpLLpp)
GO(nettle_ccm_aes128_digest, vFpLp)
//GO(nettle_ccm_aes128_encrypt, 
GO(nettle_ccm_aes128_encrypt_message, vFpLpLpLLpp)
GO(nettle_ccm_aes128_set_key, vFpp)
GO(nettle_ccm_aes128_set_nonce, vFpLpLLL)
GO(nettle_ccm_aes128_update, vFpLp)
GO(nettle_ccm_aes192_decrypt, vFpLpp)
GO(nettle_ccm_aes192_decrypt_message, iFpLpLpLLpp)
GO(nettle_ccm_aes192_digest, vFpLp)
//GO(nettle_ccm_aes192_encrypt, 
GO(nettle_ccm_aes192_encrypt_message, vFpLpLpLLpp)
GO(nettle_ccm_aes192_set_key, vFpp)
GO(nettle_ccm_aes192_set_nonce, vFpLpLLL)
GO(nettle_ccm_aes192_update, vFpLp)
GO(nettle_ccm_aes256_decrypt, vFpLpp)
GO(nettle_ccm_aes256_decrypt_message, iFpLpLpLLpp)
GO(nettle_ccm_aes256_digest, vFpLp)
//GO(nettle_ccm_aes256_encrypt, 
GO(nettle_ccm_aes256_encrypt_message, vFpLpLpLLpp)
GO(nettle_ccm_aes256_set_key, vFpp)
GO(nettle_ccm_aes256_set_nonce, vFpLpLLL)
GO(nettle_ccm_aes256_update, vFpLp)
//GO(nettle_ccm_decrypt, 
GOM(nettle_ccm_decrypt_message, iFEppLpLpLLpp)
//GO(nettle_ccm_digest, 
//GO(nettle_ccm_encrypt, 
GOM(nettle_ccm_encrypt_message, vFEppLpLpLLpp)
//GO(nettle_ccm_set_nonce, 
//GO(nettle_ccm_update, 
GOM(nettle_cfb8_decrypt, vFEppLpLpp)
GOM(nettle_cfb8_encrypt, vFEppLpLpp)
GOM(nettle_cfb_decrypt, vFEppLpLpp)
GOM(nettle_cfb_encrypt, vFEppLpLpp)
//GO(_nettle_chacha_2core, 
//GO(_nettle_chacha_2core32, 
//GO(_nettle_chacha_4core, 
//GO(_nettle_chacha_4core32, 
//GO(_nettle_chacha_core, 
GO(nettle_chacha_crypt, vFpipp)
GO(nettle_chacha_crypt32, vFpLpp)
//DATA(nettle_chacha_poly1305, 
GO(nettle_chacha_poly1305_decrypt, vFpLpp)
GO(nettle_chacha_poly1305_digest, vFpLp)
GO(nettle_chacha_poly1305_encrypt, vFpipp)
GO(nettle_chacha_poly1305_set_key, vFpp)
GO(nettle_chacha_poly1305_set_nonce, vFpp)
GO(nettle_chacha_poly1305_update, vFpLp)
GO(nettle_chacha_set_counter, vFpp)
GO(nettle_chacha_set_counter32, vFpp)
GO(nettle_chacha_set_key, vFpp)
GO(nettle_chacha_set_nonce, vFpp)
GO(nettle_chacha_set_nonce96, vFpp)
//DATA(_nettle_ciphers, 
GOM(nettle_cmac128_digest, vFEppppup)
GO(nettle_cmac128_init, vFp)
GOM(nettle_cmac128_set_key, vFEppp)
GOM(nettle_cmac128_update, vFEpppLp)
GOM(nettle_cmac64_digest, vFEppppup)
GO(nettle_cmac64_init, vFp)
GOM(nettle_cmac64_set_key, vFEppp)
GOM(nettle_cmac64_update, vFEpppLp)
//DATA(nettle_cmac_aes128, 
GO(nettle_cmac_aes128_digest, vFpLp)
GO(nettle_cmac_aes128_set_key, vFpp)
GO(nettle_cmac_aes128_update, vFpLp)
//DATA(nettle_cmac_aes256, 
GO(nettle_cmac_aes256_digest, vFpLp)
GO(nettle_cmac_aes256_set_key, vFpp)
GO(nettle_cmac_aes256_update, vFpLp)
//DATA(nettle_cmac_des3, 
GO(nettle_cmac_des3_digest, vFpLp)
GO(nettle_cmac_des3_set_key, vFpp)
GO(nettle_cmac_des3_update, vFpLp)
GO(nettle_cnd_memcpy, vFippL)
GOM(nettle_ctr_crypt, vFEppLpLpp)
//GO(_nettle_ctr_crypt16, 
GO(nettle_des_check_parity, iFLp)
GO(nettle_des_decrypt, vFpLpp)
GO(nettle_des_encrypt, vFpipp)
GO(nettle_des_fix_parity, vFLpp)
GO(nettle_des_set_key, iFpp)
GO(nettle_des3_decrypt, vFpLpp)
GO(nettle_des3_encrypt, vFpipp)
GO(nettle_des3_set_key, iFpp)
GO(nettle_drbg_ctr_aes256_init, vFpp)
GO(nettle_drbg_ctr_aes256_random, vFpLp)
//DATA(nettle_eax_aes128, 
GO(nettle_eax_aes128_decrypt, vFpLpp)
GO(nettle_eax_aes128_digest, vFpLp)
//GO(nettle_eax_aes128_encrypt, 
GO(nettle_eax_aes128_set_key, vFpp)
//GO(nettle_eax_aes128_set_nonce, 
GO(nettle_eax_aes128_update, vFpLp)
GOM(nettle_eax_decrypt, vFEppppLpp)
GOM(nettle_eax_digest, vFEppppLp)
GOM(nettle_eax_encrypt, vFEppppLpp)
GOM(nettle_eax_set_key, vFEppp)
GOM(nettle_eax_set_nonce, vFEppppLp)
GOM(nettle_eax_update, vFEppppLp)
//DATA(nettle_gcm_aes128, 
GO(nettle_gcm_aes128_decrypt, vFpLpp)
GO(nettle_gcm_aes128_digest, vFpLp)
//GO(nettle_gcm_aes128_encrypt, 
GO(nettle_gcm_aes128_set_iv, vFpLp)
GO(nettle_gcm_aes128_set_key, vFpp)
GO(nettle_gcm_aes128_update, vFpip)
//DATA(nettle_gcm_aes192, 
GO(nettle_gcm_aes192_decrypt, vFpLpp)
GO(nettle_gcm_aes192_digest, vFpLp)
//GO(nettle_gcm_aes192_encrypt, 
GO(nettle_gcm_aes192_set_iv, vFpLp)
GO(nettle_gcm_aes192_set_key, vFpp)
GO(nettle_gcm_aes192_update, vFpip)
//DATA(nettle_gcm_aes256, 
GO(nettle_gcm_aes256_decrypt, vFpLpp)
GO(nettle_gcm_aes256_digest, vFpLp)
//GO(nettle_gcm_aes256_encrypt, 
GO(nettle_gcm_aes256_set_iv, vFpLp)
GO(nettle_gcm_aes256_set_key, vFpp)
GO(nettle_gcm_aes256_update, vFpip)
GO(nettle_gcm_aes_decrypt, vFpLpp)
GO(nettle_gcm_aes_digest, vFpLp)
//GO(nettle_gcm_aes_encrypt, 
//GO(nettle_gcm_aes_set_iv, 
//GO(nettle_gcm_aes_set_key, 
GO(nettle_gcm_aes_update, vFpLp)
//DATA(nettle_gcm_camellia128, 
GO(nettle_gcm_camellia128_decrypt, vFpLpp)
GO(nettle_gcm_camellia128_digest, vFpLp)
//GO(nettle_gcm_camellia128_encrypt, 
GO(nettle_gcm_camellia128_set_iv, vFpip)
GO(nettle_gcm_camellia128_set_key, vFpp)
GO(nettle_gcm_camellia128_update, vFpLp)
//DATA(nettle_gcm_camellia256, 
GO(nettle_gcm_camellia256_decrypt, vFpLpp)
GO(nettle_gcm_camellia256_digest, vFpLp)
//GO(nettle_gcm_camellia256_encrypt, 
GO(nettle_gcm_camellia256_set_iv, vFpip)
GO(nettle_gcm_camellia256_set_key, vFpp)
GO(nettle_gcm_camellia256_update, vFpLp)
GOM(nettle_gcm_decrypt, vFEppppLpp)
GOM(nettle_gcm_digest, vFEppppLp)
GOM(nettle_gcm_encrypt, vFEppppLpp)
GO(nettle_gcm_set_iv, vFppLp)
GOM(nettle_gcm_set_key, vFEppp)
//DATA(nettle_gcm_sm4, 
GO(nettle_gcm_sm4_decrypt, vFpLpp)
GO(nettle_gcm_sm4_digest, vFpLp)
//GO(nettle_gcm_sm4_encrypt, 
//GO(nettle_gcm_sm4_set_iv, 
GO(nettle_gcm_sm4_set_key, vFpp)
GO(nettle_gcm_sm4_update, vFpLp)
GO(nettle_gcm_update, vFppLp)
GO(nettle_get_aeads, pFv)
GO(nettle_get_armors, pFv)
GO(nettle_get_ciphers, pFv)
GO(nettle_get_hashes, pFv)
GO(nettle_get_macs, pFv)
//GO(_nettle_ghash_set_key, 
//GO(_nettle_ghash_set_key_c, 
//GO(_nettle_ghash_update, 
//GO(_nettle_ghash_update_c, 
//GO(_nettle_gost28147_encrypt_block, 
//DATA(nettle_gosthash94, 
GO(nettle_gosthash94cp_digest, vFpLp)
GO(nettle_gosthash94cp_update, vFpLp)
//GO(nettle_gosthash94_digest, 
GO(nettle_gosthash94_init, vFp)
//GO(nettle_gosthash94_update, 
//DATA(_nettle_hashes, 
GOM(nettle_hkdf_expand, vFEpppLLpLp)
GOM(nettle_hkdf_extract, vFEpppLLpp)
GOM(nettle_hmac_digest, vFEppppLp)
GO(nettle_hmac_gosthash94cp_digest, vFpLp)
GO(nettle_hmac_gosthash94cp_set_key, vFpip)
GO(nettle_hmac_gosthash94cp_update, vFpLp)
GO(nettle_hmac_gosthash94_digest, vFpLp)
//GO(nettle_hmac_gosthash94_set_key, 
GO(nettle_hmac_gosthash94_update, vFpLp)
//DATA(nettle_hmac_md5, 
GO(nettle_hmac_md5_digest, vFpLp)
GO(nettle_hmac_md5_set_key, vFpip)
GO(nettle_hmac_md5_update, vFpLp)
//DATA(nettle_hmac_ripemd160, 
GO(nettle_hmac_ripemd160_digest, vFpLp)
//GO(nettle_hmac_ripemd160_set_key, 
GO(nettle_hmac_ripemd160_update, vFpLp)
GOM(nettle_hmac_set_key, vFEppppLp)
//DATA(nettle_hmac_sha1, 
GO(nettle_hmac_sha1_digest, vFpLp)
GO(nettle_hmac_sha1_set_key, vFpip)
GO(nettle_hmac_sha1_update, vFpLp)
//DATA(nettle_hmac_sha224, 
GO(nettle_hmac_sha224_digest, vFpLp)
GO(nettle_hmac_sha224_set_key, vFpLp)
//DATA(nettle_hmac_sha256, 
GO(nettle_hmac_sha256_digest, vFpip)
GO(nettle_hmac_sha256_set_key, vFpip)
GO(nettle_hmac_sha256_update, vFpip)
//DATA(nettle_hmac_sha384, 
GO(nettle_hmac_sha384_digest, vFpLp)
GO(nettle_hmac_sha384_set_key, vFpLp)
//DATA(nettle_hmac_sha512, 
GO(nettle_hmac_sha512_digest, vFpip)
GO(nettle_hmac_sha512_set_key, vFpip)
GO(nettle_hmac_sha512_update, vFpip)
//DATA(nettle_hmac_sm3, 
GO(nettle_hmac_sm3_digest, vFpLp)
//GO(nettle_hmac_sm3_set_key, 
GO(nettle_hmac_sm3_update, vFpLp)
//DATA(nettle_hmac_streebog256, 
GO(nettle_hmac_streebog256_digest, vFpLp)
GO(nettle_hmac_streebog256_set_key, vFpLp)
//DATA(nettle_hmac_streebog512, 
GO(nettle_hmac_streebog512_digest, vFpip)
GO(nettle_hmac_streebog512_set_key, vFpip)
GO(nettle_hmac_streebog512_update, vFpip)
GOM(nettle_hmac_update, vFEppLp)
GO(nettle_knuth_lfib_get, uFp)
GO(nettle_knuth_lfib_get_array, vFpLp)
GO(nettle_knuth_lfib_init, vFpu)
GO(nettle_knuth_lfib_random, vFpLp)
//GO(nettle_lookup_hash, 
//DATA(_nettle_macs, 
//DATA(nettle_md2, 
GO(nettle_md2_digest, vFpLp)
GO(nettle_md2_init, vFp)
GO(nettle_md2_update, vFpLp)
//DATA(nettle_md4, 
GO(nettle_md4_digest, vFpLp)
GO(nettle_md4_init, vFp)
GO(nettle_md4_update, vFpLp)
//DATA(nettle_md5, 
//GO(nettle_md5_compress, 
GO(nettle_md5_digest, vFpLp)
GO(nettle_MD5Final, vFpp)
GO(nettle_md5_init, vFp)
GO(nettle_MD5Init, vFp)
GO(nettle_md5_update, vFpLp)
GO(nettle_MD5Update, vFppu)
GO(nettle_memeql_sec, iFppL)
GO(nettle_memxor, pFppL)
GO(nettle_memxor3, pFpppL)
//GO(nettle_nist_keyunwrap16, 
//GO(nettle_nist_keywrap16, 
GO(nettle_ocb_aes128_decrypt, vFpppLpp)
GO(nettle_ocb_aes128_decrypt_message, iFppLpLpLLpp)
GO(nettle_ocb_aes128_digest, vFppLp)
GO(nettle_ocb_aes128_encrypt, vFppLpp)
GO(nettle_ocb_aes128_encrypt_message, vFpLpLpLLpp)
GO(nettle_ocb_aes128_set_decrypt_key, vFppp)
GO(nettle_ocb_aes128_set_encrypt_key, vFpp)
GO(nettle_ocb_aes128_set_nonce, vFppLLp)
GO(nettle_ocb_aes128_update, vFppLp)
GOM(nettle_ocb_decrypt, vFEppppppLpp)
//GO(nettle_ocb_decrypt_message, 
GOM(nettle_ocb_digest, vFEppppLp)
GOM(nettle_ocb_encrypt, vFEppppLpp)
//GO(nettle_ocb_encrypt_message, 
GOM(nettle_ocb_set_key, vFEppp)
GOM(nettle_ocb_set_nonce, vFEpppLLp)
GOM(nettle_ocb_update, vFEppppLp)
GOM(nettle_pbkdf2, vFEpppLuLpLp)
GO(nettle_pbkdf2_hmac_gosthash94cp, vFLpuLpLp)
//GO(nettle_pbkdf2_hmac_sha1, 
//GO(nettle_pbkdf2_hmac_sha256, 
//GO(nettle_pbkdf2_hmac_sha384, 
//GO(nettle_pbkdf2_hmac_sha512, 
GO(nettle_poly1305_aes_digest, vFpLp)
//GO(nettle_poly1305_aes_set_key, 
GO(nettle_poly1305_aes_set_nonce, vFpp)
GO(nettle_poly1305_aes_update, vFpLp)
//GO(_nettle_poly1305_block, 
//GO(_nettle_poly1305_digest, 
//GO(_nettle_poly1305_set_key, 
//GO(_nettle_poly1305_update, 
//GO(nettle_realloc, 
//DATA(nettle_ripemd160, 
//GO(_nettle_ripemd160_compress, 
GO(nettle_ripemd160_digest, vFpLp)
GO(nettle_ripemd160_init, vFp)
GO(nettle_ripemd160_update, vFpLp)
//GO(nettle_salsa20_128_set_key, 
GO(nettle_salsa20_256_set_key, vFpp)
//GO(_nettle_salsa20_core, 
//GO(_nettle_salsa20_crypt, 
GO(nettle_salsa20_crypt, vFpipp)
GO(nettle_salsa20r12_crypt, vFpLpp)
GO(nettle_salsa20_set_key, vFpLp)
GO(nettle_salsa20_set_nonce, vFpp)
//DATA(nettle_serpent128, 
//GO(nettle_serpent128_set_key, 
//DATA(nettle_serpent192, 
//GO(nettle_serpent192_set_key, 
//DATA(nettle_serpent256, 
GO(nettle_serpent256_set_key, vFpp)
GO(nettle_serpent_decrypt, vFpLpp)
//GO(nettle_serpent_encrypt, 
GO(nettle_serpent_set_key, vFpLp)
//DATA(nettle_sha1, 
GO(nettle_sha1_compress, vFpp)
//GO(_nettle_sha1_compress_c, 
GO(nettle_sha1_digest, vFpLp)
GO(nettle_sha1_init, vFp)
GO(nettle_sha1_update, vFpLp)
//DATA(nettle_sha224, 
GO(nettle_sha224_digest, vFpLp)
GO(nettle_sha224_init, vFp)
//DATA(nettle_sha256, 
//GO(nettle_sha256_compress, 
//GO(_nettle_sha256_compress_n, 
//GO(_nettle_sha256_compress_n_c, 
GO(nettle_sha256_digest, vFpip)
GO(nettle_sha256_init, vFp)
GO(nettle_sha256_update, vFpLp)
GO(nettle_sha3_128_init, vFp)
//GO(nettle_sha3_128_shake, 
GO(nettle_sha3_128_shake_output, vFpLp)
GO(nettle_sha3_128_update, vFpLp)
//DATA(nettle_sha3_224, 
GO(nettle_sha3_224_digest, vFpLp)
GO(nettle_sha3_224_init, vFp)
GO(nettle_sha3_224_update, vFpLp)
//DATA(nettle_sha3_256, 
GO(nettle_sha3_256_digest, vFpip)
GO(nettle_sha3_256_init, vFp)
//GO(nettle_sha3_256_shake, 
GO(nettle_sha3_256_shake_output, vFpLp)
GO(nettle_sha3_256_update, vFpLp)
//DATA(nettle_sha3_384, 
GO(nettle_sha3_384_digest, vFpLp)
GO(nettle_sha3_384_init, vFp)
GO(nettle_sha3_384_update, vFpLp)
//DATA(nettle_sha3_512, 
GO(nettle_sha3_512_digest, vFpLp)
GO(nettle_sha3_512_init, vFp)
GO(nettle_sha3_512_update, vFpLp)
//DATA(nettle_sha384, 
GO(nettle_sha384_digest, vFpip)
GO(nettle_sha384_init, vFp)
//GO(_nettle_sha3_pad, 
GO(nettle_sha3_permute, vFp)
//GO(_nettle_sha3_shake, 
//GO(_nettle_sha3_shake_output, 
//GO(_nettle_sha3_update, 
//DATA(nettle_sha512, 
//DATA(nettle_sha512_224, 
//GO(nettle_sha512_224_digest, 
//GO(nettle_sha512_224_init, 
//DATA(nettle_sha512_256, 
GO(nettle_sha512_256_digest, vFpLp)
GO(nettle_sha512_256_init, vFp)
//GO(_nettle_sha512_compress, 
GO(nettle_sha512_compress, vFpp)
GO(nettle_sha512_digest, vFpip)
GO(nettle_sha512_init, vFp)
GO(nettle_sha512_update, vFpLp)
GO(nettle_siv_cmac_aes128_decrypt_message, iFpLpLpLpp)
GO(nettle_siv_cmac_aes128_encrypt_message, vFpLpLpLpp)
GO(nettle_siv_cmac_aes128_set_key, vFpp)
GO(nettle_siv_cmac_aes256_decrypt_message, iFpLpLpLpp)
GO(nettle_siv_cmac_aes256_encrypt_message, vFpLpLpLpp)
GO(nettle_siv_cmac_aes256_set_key, vFpp)
//GOM(nettle_siv_cmac_decrypt_message, 
//GOM(nettle_siv_cmac_encrypt_message, 
//GOM(nettle_siv_cmac_set_key, 
GO(nettle_siv_gcm_aes128_decrypt_message, iFpLpLpLpp)
GO(nettle_siv_gcm_aes128_encrypt_message, vFpLpLpLpp)
GO(nettle_siv_gcm_aes256_decrypt_message, iFpLpLpLpp)
GO(nettle_siv_gcm_aes256_encrypt_message, vFpLpLpLpp)
//GOM(nettle_siv_gcm_decrypt_message, 
//GOM(nettle_siv_gcm_encrypt_message, 
//GO(_nettle_siv_ghash_set_key, 
//GO(_nettle_siv_ghash_update, 
//DATA(nettle_sm3, 
GO(nettle_sm3_digest, vFpLp)
GO(nettle_sm3_init, vFp)
GO(nettle_sm3_update, vFpLp)
//DATA(nettle_sm4, 
GO(nettle_sm4_crypt, vFpLpp)
GO(nettle_sm4_set_decrypt_key, vFpp)
//GO(nettle_sm4_set_encrypt_key, 
//DATA(nettle_streebog256, 
GO(nettle_streebog256_digest, vFpLp)
GO(nettle_streebog256_init, vFp)
//DATA(nettle_streebog512, 
GO(nettle_streebog512_digest, vFpip)
GO(nettle_streebog512_init, vFp)
GO(nettle_streebog512_update, vFpLp)
//DATA(nettle_twofish128, 
//GO(nettle_twofish128_set_key, 
//DATA(nettle_twofish192, 
//GO(nettle_twofish192_set_key, 
//DATA(nettle_twofish256, 
GO(nettle_twofish256_set_key, vFpp)
GO(nettle_twofish_decrypt, vFpLpp)
GO(nettle_twofish_encrypt, vFpLpp)
GO(nettle_twofish_set_key, vFpLp)
GO(nettle_umac128_digest, vFpLp)
GO(nettle_umac128_set_key, vFpp)
GO(nettle_umac128_set_nonce, vFpp)
GO(nettle_umac128_update, vFpLp)
GO(nettle_umac32_digest, vFpLp)
GO(nettle_umac32_set_key, vFpp)
//GO(nettle_umac32_set_nonce, 
GO(nettle_umac32_update, vFpLp)
GO(nettle_umac64_digest, vFpLp)
GO(nettle_umac64_set_key, vFpp)
//GO(nettle_umac64_set_nonce, 
GO(nettle_umac64_update, vFpLp)
GO(nettle_umac96_digest, vFpLp)
GO(nettle_umac96_set_key, vFpp)
GO(nettle_umac96_set_nonce, vFpp)
GO(nettle_umac96_update, vFpLp)
//GO(_nettle_umac_l2, 
//GO(_nettle_umac_l2_final, 
//GO(_nettle_umac_l2_init, 
//GO(_nettle_umac_l3, 
//GO(_nettle_umac_l3_init, 
//GO(_nettle_umac_nh, 
//GO(_nettle_umac_nh_n, 
//GO(_nettle_umac_poly128, 
//GO(_nettle_umac_poly64, 
//GO(_nettle_umac_set_key, 
GO(nettle_version_major, iFv)
GO(nettle_version_minor, iFv)
//GO(_nettle_write_be32, 
//GO(_nettle_write_le32, 
//GO(_nettle_write_le64, 
GO(nettle_xrealloc, pFppL)
GO(nettle_xts_aes128_decrypt_message, vFppLpp)
GO(nettle_xts_aes128_encrypt_message, vFppLpp)
GO(nettle_xts_aes128_set_decrypt_key, vFpp)
GO(nettle_xts_aes128_set_encrypt_key, vFpp)
GO(nettle_xts_aes256_decrypt_message, vFppLpp)
GO(nettle_xts_aes256_encrypt_message, vFppLpp)
GO(nettle_xts_aes256_set_decrypt_key, vFpp)
GO(nettle_xts_aes256_set_encrypt_key, vFpp)
//GO(nettle_xts_decrypt_message, 
//GO(nettle_xts_encrypt_message, 
//GO(nettle_yarrow256_fast_reseed, 
GO(nettle_yarrow256_init, vFpup)
GO(nettle_yarrow256_is_seeded, iFp)
GO(nettle_yarrow256_needed_sources, uFp)
GO(nettle_yarrow256_random, vFpLp)
GO(nettle_yarrow256_seed, vFpLp)
GO(nettle_yarrow256_slow_reseed, vFp)
GO(nettle_yarrow256_update, iFpuuLp)
GO(nettle_yarrow_key_event_estimate, uFpuu)
GO(nettle_yarrow_key_event_init, vFp)
