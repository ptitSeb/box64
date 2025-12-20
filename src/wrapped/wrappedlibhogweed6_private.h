#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

// typedef __mpz_struct mpz_t[1]; so mpz_t is "p" when in function paramter (struct is 2 int + 1 pointer)
// typedef long int                mp_size_t;

GO(nettle_asn1_der_decode_bitstring, uFpp)
GO(nettle_asn1_der_decode_bitstring_last, uFp)
GO(nettle_asn1_der_decode_constructed, uFpp)
GO(nettle_asn1_der_decode_constructed_last, uFp)
GO(nettle_asn1_der_get_bignum, iFppu)
GO(nettle_asn1_der_get_uint32, iFpp)
GO(nettle_asn1_der_iterator_first, uFpLp)
GO(nettle_asn1_der_iterator_next, uFp)
//GO(_nettle_cnd_copy, 
//DATA(_nettle_curve25519, 
//GO(_nettle_curve25519_eh_to_x, 
GO(nettle_curve25519_mul, vFppp)
GO(nettle_curve25519_mul_g, vFpp)
//DATA(_nettle_curve448, 
//GO(_nettle_curve448_eh_to_x, 
GO(nettle_curve448_mul, vFppp)
GO(nettle_curve448_mul_g, vFpp)
//GOM(nettle_dsa_compat_generate_keypair, iFEppppppuu)
//GOM(nettle_dsa_generate_keypair, vFEppppp)
GOM(nettle_dsa_generate_params, iFEpppppuu)
//GO(_nettle_dsa_hash, 
GO(nettle_dsa_keypair_from_sexp_alist, iFpppuup)
GO(nettle_dsa_keypair_to_sexp, iFppppp)
GO(nettle_dsa_openssl_private_key_from_der_iterator, iFpppup)
GO(nettle_dsa_params_clear, vFp)
GO(nettle_dsa_params_from_der_iterator, iFpuup)
GO(nettle_dsa_params_init, vFp)
GO(nettle_dsa_private_key_clear, vFp)
GO(nettle_dsa_private_key_init, vFp)
GO(nettle_dsa_public_key_clear, vFp)
GO(nettle_dsa_public_key_from_der_iterator, iFppp)
GO(nettle_dsa_public_key_init, vFp)
GO(nettle_dsa_sha1_keypair_from_sexp, iFpppuLp)
//GOM(nettle_dsa_sha1_sign, iFEpppppp)
//GOM(nettle_dsa_sha1_sign_digest, iFEpppppp)
GO(nettle_dsa_sha1_verify, iFppp)
GO(nettle_dsa_sha1_verify_digest, iFppp)
GO(nettle_dsa_sha256_keypair_from_sexp, iFpppuLp)
//GOM(nettle_dsa_sha256_sign, iFEpppppp)
//GOM(nettle_dsa_sha256_sign_digest, iFEpppppp)
GO(nettle_dsa_sha256_verify, iFppp)
GO(nettle_dsa_sha256_verify_digest, iFppp)
GOM(nettle_dsa_sign, iFEppppLpp)
GO(nettle_dsa_signature_clear, vFp)
GO(nettle_dsa_signature_from_sexp, iFppu)
GO(nettle_dsa_signature_init, vFp)
GO(nettle_dsa_verify, iFppLpp)
//GO(_nettle_ecc_add_eh, 
//GO(_nettle_ecc_add_ehh, 
//GO(_nettle_ecc_add_jja, 
//GO(_nettle_ecc_add_jjj, 
//GO(_nettle_ecc_add_th, 
//GO(_nettle_ecc_add_thh, 
//GO(_nettle_ecc_a_to_j, 
GO(nettle_ecc_bit_size, uFp)
//GO(_nettle_ecc_dup_eh, 
//GO(_nettle_ecc_dup_jj, 
//GO(_nettle_ecc_dup_th, 
GO(nettle_ecc_ecdsa_sign, vFpppLpppp)
GO(nettle_ecc_ecdsa_sign_itch, lFp)
GO(nettle_ecc_ecdsa_verify, iFppLpppp)
GO(nettle_ecc_ecdsa_verify_itch, lFp)
//GO(_nettle_ecc_eh_to_a, 
GO(nettle_ecc_gostdsa_sign, vFpppLpppp)
GO(nettle_ecc_gostdsa_sign_itch, lFp)
GO(nettle_ecc_gostdsa_verify, iFppLpppp)
GO(nettle_ecc_gostdsa_verify_itch, lFp)
//GO(_nettle_ecc_j_to_a, 
//GO(_nettle_ecc_mod, 
//GO(_nettle_ecc_mod_add, 
//GO(_nettle_ecc_mod_addmul_1, 
//GO(_nettle_ecc_mod_equal_p, 
//GO(_nettle_ecc_mod_inv, 
//GO(_nettle_ecc_mod_mul, 
//GO(_nettle_ecc_mod_mul_1, 
//GO(_nettle_ecc_mod_mul_canonical, 
//GO(_nettle_ecc_mod_pow_2k, 
//GO(_nettle_ecc_mod_pow_2k_mul, 
//GO(_nettle_ecc_mod_random, 
//GO(_nettle_ecc_mod_sqr, 
//GO(_nettle_ecc_mod_sqr_canonical, 
//GO(_nettle_ecc_mod_sub, 
//GO(_nettle_ecc_mod_submul_1, 
//GO(_nettle_ecc_mod_zero_p, 
//GO(_nettle_ecc_mul_a, 
//GO(_nettle_ecc_mul_a_eh, 
//GO(_nettle_ecc_mul_g, 
//GO(_nettle_ecc_mul_g_eh, 
//GO(_nettle_ecc_mul_m, 
//GO(_nettle_ecc_nonsec_add_jjj, 
//GO(_nettle_ecc_pm1_redc, 
GO(nettle_ecc_point_clear, vFp)
GO(nettle_ecc_point_get, vFppp)
GO(nettle_ecc_point_init, vFpp)
GO(nettle_ecc_point_mul, vFppp)
GO(nettle_ecc_point_mul_g, vFpp)
GO(nettle_ecc_point_set, iFppp)
//GO(_nettle_ecc_pp1_redc, 
GO(nettle_ecc_scalar_clear, vFp)
GO(nettle_ecc_scalar_get, vFpp)
GO(nettle_ecc_scalar_init, vFpp)
//GOM(nettle_ecc_scalar_random, vFEppp)
GO(nettle_ecc_scalar_set, iFpp)
GO(nettle_ecc_size, lFp)
GO(nettle_ecc_size_a, lFp)
GO(nettle_ecc_size_j, lFp)
GOM(nettle_ecdsa_generate_keypair, vFEpppp)
GOM(nettle_ecdsa_sign, vFEpppLpp)
GO(nettle_ecdsa_verify, iFpLpp)
//DATA(_nettle_ed25519_sha512, 
GO(nettle_ed25519_sha512_public_key, vFpp)
GO(nettle_ed25519_sha512_sign, vFppLpp)
GO(nettle_ed25519_sha512_verify, iFpLpp)
//DATA(_nettle_ed448_shake256, 
GO(nettle_ed448_shake256_public_key, vFpp)
GO(nettle_ed448_shake256_sign, vFppLpp)
GO(nettle_ed448_shake256_verify, iFpLpp)
//GO(_nettle_eddsa_compress, 
//GO(_nettle_eddsa_compress_itch, 
//GO(_nettle_eddsa_decompress, 
//GO(_nettle_eddsa_decompress_itch, 
//GO(_nettle_eddsa_expand_key, 
//GO(_nettle_eddsa_hash, 
//GO(_nettle_eddsa_public_key, 
//GO(_nettle_eddsa_public_key_itch, 
//GO(_nettle_eddsa_sign, 
//GO(_nettle_eddsa_sign_itch, 
//GO(_nettle_eddsa_verify, 
//GO(_nettle_eddsa_verify_itch, 
//GO(_nettle_generate_pocklington_prime, 
GO(nettle_get_gost_gc256b, pFv)
GO(nettle_get_gost_gc512a, pFv)
GO(nettle_get_secp_192r1, pFv)
GO(nettle_get_secp_224r1, pFv)
GO(nettle_get_secp_256r1, pFv)
GO(nettle_get_secp_384r1, pFv)
GO(nettle_get_secp_521r1, pFv)
//GO(_nettle_gmp_alloc, 
//GO(_nettle_gmp_alloc_limbs, 
//GO(_nettle_gmp_free, 
//GO(_nettle_gmp_free_limbs, 
//GO(_nettle_gostdsa_hash, 
GOM(nettle_gostdsa_sign, vFEpppLpp)
GO(nettle_gostdsa_verify, iFpLpp)
GO(nettle_gostdsa_vko, vFppLpp)
//DATA(_nettle_gost_gc256b, 
//DATA(_nettle_gost_gc512a, 
//GO(_nettle_mpn_get_base256, 
//GO(_nettle_mpn_get_base256_le, 
//GO(_nettle_mpn_set_base256, 
//GO(_nettle_mpn_set_base256_le, 
GO(nettle_mpz_get_str_256, vFLpp)
GO(nettle_mpz_init_set_str_256_s, vFpLp)
GO(nettle_mpz_init_set_str_256_u, vFpLp)
//GO(_nettle_mpz_limbs_copy, 
GOM(nettle_mpz_random, vFEpppp)
GOM(nettle_mpz_random_size, vFEpppu)
//GO(_nettle_mpz_set_n, 
GO(nettle_mpz_set_sexp, iFpup)
GO(nettle_mpz_set_str_256_s, vFpLp)
GO(nettle_mpz_set_str_256_u, vFpLp)
GO(nettle_mpz_sizeinbase_256_s, LFp)
GO(nettle_mpz_sizeinbase_256_u, LFp)
//GO(_nettle_oaep_decode_mgf1, 
//GO(_nettle_oaep_encode_mgf1, 
GO(nettle_openssl_provate_key_from_der, iFpppuLp)
GO(nettle_pgp_armor, iFppup)
GO(nettle_pgp_crc24, uFup)
GO(nettle_pgp_put_header, iFpuu)
GO(nettle_pgp_put_header_length, vFpuu)
GO(nettle_pgp_put_length, iFpu)
GO(nettle_pgp_put_mpi, iFpp)
GO(nettle_pgp_put_public_rsa_key, iFppl)
GO(nettle_pgp_put_rsa_sha1_signature, iFpppup)
GO(nettle_pgp_put_string, iFpup)
GO(nettle_pgp_put_sub_packet, iFpuup)
GO(nettle_pgp_put_uint16, iFpu)
GO(nettle_pgp_put_uint32, iFpu)
GO(nettle_pgp_put_userid, iFpup)
GO(nettle_pgp_sub_packet_end, vFpu)
GO(nettle_pgp_sub_packet_start, uFp)
GO(nettle_pkcs1_decrypt, iFLppp)
//GOM(nettle_pkcs1_encrypt, iFELppLpp)
GO(nettle_pkcs1_rsa_digest_encode, iFpLLp)
GO(nettle_pkcs1_rsa_md5_encode, iFpLp)
GO(nettle_pkcs1_rsa_md5_encode_digest, iFpLp)
GO(nettle_pkcs1_rsa_sha1_encode, iFpLp)
GO(nettle_pkcs1_rsa_sha1_encode_digest, iFpLp)
GO(nettle_pkcs1_rsa_sha256_encode, iFpLp)
GO(nettle_pkcs1_rsa_sha256_encode_digest, iFpLp)
GO(nettle_pkcs1_rsa_sha512_encode, iFpLp)
GO(nettle_pkcs1_rsa_sha512_encode_digest, iFpLp)
//GO(_nettle_pkcs1_sec_decrypt, 
//GO(_nettle_pkcs1_sec_decrypt_variable, 
//GO(_nettle_pkcs1_signature_prefix, 
//GOM(nettle_pss_encode_mgf1, iFEpLpLpp)
//GOM(nettle_pss_mgf1, vFEppLp)
//GOM(nettle_pss_verify_mgf1, iFEpLpLp)
//GOM(nettle_random_prime, vFEpuipppp)
//GO(_nettle_rsa_blind, 
//GO(_nettle_rsa_check_size, 
GO(nettle_rsa_compute_root, vFppp)
//GOM(nettle_rsa_compute_root_tr, iFEpppppp)
GO(nettle_rsa_decrypt, iFpppp)
GOM(nettle_rsa_decrypt_tr, iFEppppppp)
GOM(nettle_rsa_encrypt, iFEpppLpp)
GOM(nettle_rsa_generate_keypair, iFEppppppuu)
GO(nettle_rsa_keypair_from_der, iFppuLp)
GO(nettle_rsa_keypair_from_sexp, iFppuLp)
GO(nettle_rsa_keypair_from_sexp_alist, iFppup)
GO(nettle_rsa_keypair_to_openpgp, iFpppp)
GO(nettle_rsa_keypair_to_sexp, iFpppp)
GO(nettle_rsa_md5_sign, iFppp)
GO(nettle_rsa_md5_sign_digest, iFppp)
//GOM(nettle_rsa_md5_sign_digest_tr, iFEpppppp)
//GOM(nettle_rsa_md5_sign_tr, iFEpppppp)
GO(nettle_rsa_md5_verify, iFppp)
GO(nettle_rsa_md5_verify_digest, iFppp)
//GO(_nettle_rsa_oaep_decrypt, 
//GO(_nettle_rsa_oaep_encrypt, 
//GOM(nettle_rsa_oaep_sha256_decrypt, iFEppppLpppp)
//GOM(nettle_rsa_oaep_sha256_encrypt, iFEpppLpLpp)
//GOM(nettle_rsa_oaep_sha384_decrypt, iFEppppLpppp)
//GOM(nettle_rsa_oaep_sha384_encrypt, iFEpppLpLpp)
//GOM(nettle_rsa_oaep_sha512_decrypt, iFEppppLpppp)
//GOM(nettle_rsa_oaep_sha512_encrypt, iFEpppLpLpp)
GO(nettle_rsa_pkcs1_sign, iFpLpp)
GOM(nettle_rsa_pkcs1_sign_tr, iFEppppLpp)
GO(nettle_rsa_pkcs1_verify, iFpLpp)
GO(nettle_rsa_private_key_clear, vFp)
GO(nettle_rsa_private_key_from_der_iterator, iFppup)
GO(nettle_rsa_private_key_init, vFp)
GO(nettle_rsa_private_key_prepare, iFp)
GOM(nettle_rsa_pss_sha256_sign_digest_tr, iFEppppLppp)
GO(nettle_rsa_pss_sha256_verify_digest, iFpLpp)
GOM(nettle_rsa_pss_sha384_sign_digest_tr, iFEppppLppp)
GO(nettle_rsa_pss_sha384_verify_digest, iFpLpp)
GOM(nettle_rsa_pss_sha512_sign_digest_tr, iFEppppLppp)
GO(nettle_rsa_pss_sha512_verify_digest, iFpLpp)
GO(nettle_rsa_public_key_clear, vFp)
GO(nettle_rsa_public_key_from_der_iterator, iFpup)
GO(nettle_rsa_public_key_init, vFp)
GO(nettle_rsa_public_key_prepare, iFp)
//GO(_nettle_rsa_sec_compute_root, 
//GO(_nettle_rsa_sec_compute_root_itch, 
//GO(_nettle_rsa_sec_compute_root_tr, 
GOM(nettle_rsa_sec_decrypt, iFEppppLpp)
GO(nettle_rsa_sha1_sign, iFppp)
GO(nettle_rsa_sha1_sign_digest, iFppp)
//GOM(nettle_rsa_sha1_sign_digest_tr, iFEpppppp)
//GOM(nettle_rsa_sha1_sign_tr, iFEpppppp)
GO(nettle_rsa_sha1_verify, iFppp)
GO(nettle_rsa_sha1_verify_digest, iFppp)
GO(nettle_rsa_sha256_sign, iFppp)
GO(nettle_rsa_sha256_sign_digest, iFppp)
//GOM(nettle_rsa_sha256_sign_digest_tr, iFEpppppp)
//GOM(nettle_rsa_sha256_sign_tr, iFEpppppp)
GO(nettle_rsa_sha256_verify, iFppp)
GO(nettle_rsa_sha256_verify_digest, iFppp)
GO(nettle_rsa_sha512_sign, iFppp)
GO(nettle_rsa_sha512_sign_digest, iFppp)
//GOM(nettle_rsa_sha512_sign_digest_tr, iFEpppppp)
//GOM(nettle_rsa_sha512_sign_tr, iFEpppppp)
GO(nettle_rsa_sha512_verify, iFppp)
GO(nettle_rsa_sha512_verify_digest, iFppp)
//GO(_nettle_rsa_unblind, 
//GO(_nettle_rsa_verify, 
//GO(_nettle_rsa_verify_recover, 
//GO(_nettle_sec_add_1, 
//DATA(_nettle_secp_192r1, 
//DATA(_nettle_secp_224r1, 
//DATA(_nettle_secp_256r1, 
//DATA(_nettle_secp_384r1, 
//DATA(_nettle_secp_521r1, 
//GO(_nettle_sec_sub_1, 
//GO(_nettle_sec_zero_p, 
//GOM(nettle_sexp_format, LFEppV)
GO(nettle_sexp_iterator_assoc, iFpupp)
GO(nettle_sexp_iterator_check_type, iFpp)
GO(nettle_sexp_iterator_check_types, pFpup)
GO(nettle_sexp_iterator_enter_list, iFp)
GO(nettle_sexp_iterator_exit_list, iFp)
GO(nettle_sexp_iterator_first, iFpLp)
GO(nettle_sexp_iterator_get_uint32, iFpp)
GO(nettle_sexp_iterator_next, iFp)
GO(nettle_sexp_iterator_subexpr, pFpp)
//GOM(nettle_sexp_transport_format, LFEppV)
GO(nettle_sexp_transport_iterator_first, iFpLp)
//GOM(nettle_sexp_transport_vformat, LFEppA)
//GOM(nettle_sexp_vformat, LFEppA)
