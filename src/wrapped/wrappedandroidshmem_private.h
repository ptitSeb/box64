#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

GO(shmctl, iFiip)
GO(shmget, iFiLi)
GO(shmat, pFipi)
GO(shmdt, iFp)
