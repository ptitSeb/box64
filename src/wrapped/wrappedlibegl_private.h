#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(eglBindAPI, iFi)
GO(eglBindTexImage, iFppi)
GO(eglChooseConfig, iFpppip)
//GO(eglClientWaitSync, 
GO(eglCopyBuffers, iFppp)
GO(eglCreateContext, pFpppp)
//GO(eglCreateImage, 
GO(eglCreatePbufferFromClientBuffer, pFpippp)
GO(eglCreatePbufferSurface, pFppp)
GO(eglCreatePixmapSurface, pFpppp)
//GO(eglCreatePlatformPixmapSurface, 
//GO(eglCreatePlatformWindowSurface, 
//GO(eglCreateSync, 
GO(eglCreateWindowSurface, pFpppp)
GO(eglDestroyContext, iFpp)
//GO(eglDestroyImage, 
GO(eglDestroySurface, iFpp)
//GO(eglDestroySync, 
GO(eglGetConfigAttrib, iFppip)
GO(eglGetConfigs, iFppip)
GO(eglGetCurrentContext, pFv)
GO(eglGetCurrentDisplay, pFv)
GO(eglGetCurrentSurface, pFi)
GO(eglGetDisplay, pFp)
GO(eglGetError, iFv)
//GO(eglGetPlatformDisplay, 
GOM(eglGetProcAddress, pFEp)
//GO(eglGetSyncAttrib, 
GO(eglInitialize, iFppp)
GO(eglMakeCurrent, iFpppp)
GO(eglQueryAPI, iFv)
GO(eglQueryContext, iFppip)
GO(eglQueryString, pFpi)
GO(eglQuerySurface, iFppip)
GO(eglReleaseTexImage, iFppi)
GO(eglReleaseThread, iFv)
GO(eglSurfaceAttrib, iFppii)
GO(eglSwapBuffers, iFpp)
GO(eglSwapInterval, iFpi)
GO(eglTerminate, iFp)
GO(eglWaitClient, iFv)
GO(eglWaitGL, iFv)
GO(eglWaitNative, iFi)
//GO(eglWaitSync, 

