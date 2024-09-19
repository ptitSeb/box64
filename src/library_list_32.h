#ifndef GO
#error Nope
#endif

GO("libpthread.so.0", libpthread)
GO("librt.so.1", librt)
GO("libc.so.6", libc)
GO("libm.so.6", libm)
GO("libdl.so.2", libdl)
GO("ld-linux.so.2", ldlinux)

GO("libGL.so", libgl)
GO("libGL.so.1", libgl)
GO("libSDL-1.2.so.0", sdl1)
GO("libasound.so.2", libasound)
GO("libasound.so", libasound)

GO("crashhandler.so", crashhandler)
