#ifndef GO
#error Nope
#endif

GO("libc.musl-x86_64.so.1", libcmusl)
#ifdef ANDROID
GO("libpthread.so", libpthread)
GO("librt.so", librt)
GO("libbsd.so", libbsd)
GO("libc.so", libc)
GO("libm.so", libm)
GO("libdl.so", libdl)
GO("libresolv.so", libresolv)
#else
GO("libpthread.so.0", libpthread)
GO("librt.so.1", librt)
GO("libbsd.so.0", libbsd)
GO("libc.so.6", libc)
GO("libm.so.6", libm)
GO("libdl.so.2", libdl)
GO("libresolv.so.2", libresolv)
#endif
GO("libutil.so.1", util)
GO("ld-linux-x86-64.so.2", ldlinux)
