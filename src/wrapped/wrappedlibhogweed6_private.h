#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
error Meh...
#endif

GO(nettle_rsa_pss_sha512_verify_digest, iFpipp)
GO(nettle_rsa_pss_sha384_verify_digest, iFpipp)
GO(nettle_rsa_pss_sha256_verify_digest, iFpipp)
GO(nettle_rsa_pss_sha384_sign_digest_tr, iFpppipp)
GO(nettle_rsa_pss_sha512_sign_digest_tr, iFpppipp)
GO(nettle_rsa_pss_sha256_sign_digest_tr, iFpppipp)
GO(nettle_ecc_size_a, iFp)
GO(nettle_ecc_point_init, vFpp)
GO(nettle_rsa_sec_decrypt, iFpppppip)
GO(nettle_ecc_point_clear, vFp)
GO(nettle_dsa_params_clear, vFp)
GO(nettle_get_secp_192r1, pFv)
GO(nettle_get_gost_gc256b, pFv)
