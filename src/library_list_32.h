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
GO("libGLU.so.1", libglu)
GO("libSDL-1.2.so.0", sdl1)
GO("libasound.so.2", libasound)
GO("libasound.so", libasound)
GO("libX11.so.6", libx11)
GO("libXfixes.so.3", libxfixes)
GO("libXrender.so.1", libxrender)
GO("libXcursor.so.1", libxcursor)
GO("libXrandr.so.2", libxrandr)
GO("libxrandr.so.2", libxrandr)

GO("crashhandler.so", crashhandler)
