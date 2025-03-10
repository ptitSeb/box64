#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

GO(libandroid_shmdt, iFp)
GO(libandroid_shmctl, iFiip)
GO(libandroid_shmat, pFipi)
GO(libandroid_shmget, iFiLi)
GO(shmctl, iFiip)
GO(shmget, iFiLi)
GO(shmat, pFipi)
GO(shmdt, iFp)
