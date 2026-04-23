#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GO(crypt, pFpp)
GOW(crypt_r, pFppp)
GO(encrypt, vFpi)
GOW(encrypt_r, vFpip)
//GOW(fcrypt, 
GO(setkey, vFp)
GOW(setkey_r, vFpp)
