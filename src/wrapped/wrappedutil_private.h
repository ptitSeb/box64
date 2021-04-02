#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

GOM(forkpty, iFEpppp)
GO(login, vFp)
GO(login_tty, iFi)
GO(logout, iFp)
GO(logwtmp, vFppp)
GO(openpty, iFppppp)
