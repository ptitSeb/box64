#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

// _fini
// _init
GOM(SDL_AddTimer, pFEupp)
GO(SDL_AllocRW, pFv)    // no need to use a my_ version here
//GO(SDL_CDClose, vFp)
//GO(SDL_CDEject, iFp)
//GO(SDL_CDName, pFi)
//GO(SDL_CDNumDrives, iFv)
//GO(SDL_CDOpen, pFi)
//GO(SDL_CDPause, iFp)
//GO(SDL_CDPlay, iFpii)
//GO(SDL_CDPlayTracks, iFpiiii)
//GO(SDL_CDResume, iFp)
//GO(SDL_CDStatus, iFp)
//GO(SDL_CDStop, iFp)
//GO(SDL_ClearError, vFv)
//GO(SDL_CondBroadcast, iFp)
//GO(SDL_CondSignal, iFp)
//GO(SDL_CondWait, iFpp)
//GO(SDL_CondWaitTimeout, iFppu)
//GO(SDL_ConvertSurface, pFppu)
//GO(SDL_CreateCond, pFv)
//GO(SDL_CreateCursor, pFppiiii)
//GO(SDL_CreateMutex, pFv)
//GO(SDL_CreateRGBSurface, pFuiiiuuuu)
GOM(SDL_CreateRGBSurfaceFrom, pFpiiiiuuuu)  //%noE
GO(SDL_CreateSemaphore, pFu)
GOM(SDL_CreateThread, pFEpp)
//GO(SDL_CreateYUVOverlay, pFiiup)
GO(SDL_Delay, vFu)
//GO(SDL_DestroyCond, vFp)
//GO(SDL_DestroyMutex, vFp)
//GO(SDL_DestroySemaphore, vFp)
//GO(SDL_DisplayFormat, pFp)
//GO(SDL_DisplayFormatAlpha, pFp)
//GO(SDL_DisplayYUVOverlay, iFpp)
GO(SDL_EnableKeyRepeat, iFii)
GO(SDL_EnableUNICODE, iFi)
GO(SDL_Error, vFu)
GO(SDL_EventState, CFCi)
//GO(SDL_FillRect, iFppu)
GOM(SDL_Flip, iFp)  //%noE
GO(SDL_FreeCursor, vFp)
GO(SDL_FreeRW, vFp)
GOM(SDL_FreeSurface, vFp)    //%noE
//GO(SDL_FreeYUVOverlay, vFp)
GO(SDL_GL_GetAttribute, iFup)
GOM(SDL_GL_GetProcAddress, pFEp)
GO(SDL_GL_LoadLibrary, iFp)
GO(SDL_GL_Lock, vFv)
GO(SDL_GL_SetAttribute, iFui)
GO(SDL_GL_SwapBuffers, vFv)
GO(SDL_GL_Unlock, vFv)
//GO(SDL_GL_UpdateRects, vFip)
GO(SDL_GetAppState, CFv)
//GO(SDL_GetClipRect, vFpp)
//GO(SDL_GetCursor, pFv)
GO(SDL_GetError, pFv)
//GOM(SDL_GetEventFilter, pFEv)
//GO(SDL_GetGammaRamp, iFppp)
GO(SDL_GetKeyName, pFu)
//GO(SDL_GetKeyRepeat, vFpp)
GO(SDL_GetKeyState, pFp)
//GO(SDL_GetModState, uFv)
GO(SDL_GetMouseState, uFpp)
GOM(SDL_GetRGB, vFupppp)    //%noE
//GO(SDL_GetRGBA, vFuppppp)
//GO(SDL_GetRelativeMouseState, CFpp)
//GO(SDL_GetThreadID, uFp)
GO(SDL_GetTicks, uFv)
GOM(SDL_GetVideoInfo, pFv)  //%noE
GOM(SDL_GetVideoSurface, pFv)    //%noE
//GOM(SDL_GetWMInfo, iFEp)
GOM(SDL_Has3DNow, iFv)  //%noE
GOM(SDL_Has3DNowExt, iFv)   //%noE
GOM(SDL_HasAltiVec, iFv)    //%noE
GOM(SDL_HasMMX, iFv)    //%noE
GOM(SDL_HasMMXExt, iFv) //%noE
GOM(SDL_HasRDTSC, iFv)  //%noE
GOM(SDL_HasSSE, iFv)    //%noE
GOM(SDL_HasSSE2, iFv)   //%noE
GO(SDL_Init, iFu)
//GO(SDL_InitQuickDraw, vFp)
GO(SDL_InitSubSystem, iFu)
GO(SDL_JoystickClose, vFp)
GO(SDL_JoystickEventState, iFi)
GO(SDL_JoystickGetAxis, wFpi)
GO(SDL_JoystickGetBall, iFpipp)
GO(SDL_JoystickGetButton, CFpi)
GO(SDL_JoystickGetHat, uFpi)
GO(SDL_JoystickIndex, iFp)
GO(SDL_JoystickName, pFi)
GO(SDL_JoystickNumAxes, iFp)
GO(SDL_JoystickNumBalls, iFp)
GO(SDL_JoystickNumButtons, iFp)
GO(SDL_JoystickNumHats, iFp)
GO(SDL_JoystickOpen, pFi)
GO(SDL_JoystickOpened, iFi)
GO(SDL_JoystickUpdate, vFv)
GOM(SDL_KillThread, vFEp)
GO(SDL_Linked_Version, pFv)
GOM(SDL_ListModes, pFpu)    //%noE
GOM(SDL_LoadBMP_RW, pFEpi)
GOM(SDL_LoadFunction, pFEpp)
GOM(SDL_LoadObject, pFEp)
GOM(SDL_LockSurface, iFp)   //%noE
//GO(SDL_LockYUVOverlay, iFp)
//GO(SDL_LowerBlit, iFpppp)
GOM(SDL_MapRGB, uFpCCC) //%noE
//GO(SDL_MapRGBA, uFpCCCC)
GO(SDL_NumJoysticks, iFv)
GOM(SDL_PeepEvents, iFpiuu) //%noE
GOM(SDL_PollEvent, iFp) //%noE
GO(SDL_PumpEvents, vFv)
GOM(SDL_PushEvent, iFp) //%noE
GOM(SDL_Quit, vFv)  //%noE
GO(SDL_QuitSubSystem, vFu)
//GOM(SDL_RWFromConstMem, pFEpi)
//GOM(SDL_RWFromFP, pFEpi)
GOM(SDL_RWFromFile, pFEpp)
GOM(SDL_RWFromMem, pFEpi)
//GOM(SDL_ReadBE16, uFEp)
//GOM(SDL_ReadBE32, uFEp)
//GOM(SDL_ReadBE64, UFEp)
//GOM(SDL_ReadLE16, uFEp)
GOM(SDL_ReadLE32, uFEp)
//GOM(SDL_ReadLE64, UFEp)
//GO(SDL_RegisterApp, iFpup)
GOM(SDL_RemoveTimer, iFEp)
//GOM(SDL_SaveBMP_RW, iFEppi)
GO(SDL_SemPost, iFp)
GO(SDL_SemTryWait, iFp)
GO(SDL_SemValue, uFp)
GO(SDL_SemWait, iFp)
GO(SDL_SemWaitTimeout, iFpu)
GO(SDL_SetAlpha, iFpuC)
//GO(SDL_SetClipRect, uFpp)
GOM(SDL_SetColorKey, iFpuu)  //%noE
//GO(SDL_SetColors, iFppii)
//GO(SDL_SetCursor, vFp)
GO(SDL_SetError, vFpppppp)      // use ..., so putting arbitrary number of arguments...
//GOM(SDL_SetEventFilter, vFEp)
GO(SDL_SetGamma, iFfff)
GO(SDL_SetGammaRamp, iFppp)
GO(SDL_SetModState, vFu)
//GO(SDL_SetModuleHandle, vFp)
//GO(SDL_SetPalette, iFpipii)
GOM(SDL_SetTimer, iFEup)
GOM(SDL_SetVideoMode, pFiiiu)   //%noE
GO(SDL_ShowCursor, iFi)
//GO(SDL_SoftStretch, iFpppp)
GO(SDL_ThreadID, uFv)
GOM(SDL_UnloadObject, vFEp)
GOM(SDL_UnlockSurface, vFp) //%noE
//GO(SDL_UnlockYUVOverlay, vFp)
GO(SDL_UnregisterApp, vFv)
//GO(SDL_UpdateRect, vFpiiuu)
//GO(SDL_UpdateRects, vFpip)
//GO(SDL_UpperBlit, iFpppp)
GO(SDL_VideoDriverName, pFpi)
//GO(SDL_VideoInit, iFpu)
GO(SDL_VideoModeOK, iFiiiu)
GO(SDL_VideoQuit, vFv)
//GO(SDL_WM_GetCaption, vFpp)
GO(SDL_WM_GrabInput, iFi)
GO(SDL_WM_IconifyWindow, iFv)
GO(SDL_WM_SetCaption, vFpp)
GOM(SDL_WM_SetIcon, vFpp)   //%noE
//GO(SDL_WM_ToggleFullScreen, iFp)
//GO(SDL_WaitEvent, iFp)
//GO(SDL_WaitThread, vFpp)
GO(SDL_WarpMouse, vFWW)
GO(SDL_WasInit, uFu)
//GOM(SDL_WriteBE16, uFEpW)
//GOM(SDL_WriteBE32, uFEpu)
//GOM(SDL_WriteBE64, uFEpU)
//GOM(SDL_WriteLE16, uFEpW)
//GOM(SDL_WriteLE32, uFEpu)
//GOM(SDL_WriteLE64, uFEpU)
//GO(SDL_framerateDelay, uFp)
//GO(SDL_getFramecount, iFp)
//GO(SDL_getFramerate, iFp)
//GO(SDL_iconv, LFppppp)
//GO(SDL_iconv_close, iFp)
//GO(SDL_iconv_open, pFpp)
//GO(SDL_iconv_string, pFpppL)
//GO(SDL_initFramerate, vFp)
//GO(SDL_lltoa, pFlpi)
//GO(SDL_ltoa, pFlpi)
//GO(SDL_main, iFip)
//GO(SDL_memcmp, iFppu)
//GO(SDL_mutexP, iFp)
//GO(SDL_mutexV, iFp)
//GO(SDL_revcpy, pFppL)
//GO(SDL_setFramerate, iFpu)
//GO(SDL_snprintf, iFpupppppp)    // use ...
//GO(SDL_sscanf, iFppppppp)       // use ...
//GO(SDL_strcasecmp, iFpp)
//GO(SDL_strchr, pFpi)
//GO(SDL_strcmp, iFpp)
//GO(SDL_strdup, pFp)
//GO(SDL_strlcat, LFppL)
//GO(SDL_strlcpy, uFppu)
//GO(SDL_strlen, uFp)
//GO(SDL_strlwr, pFp)
//GO(SDL_strncasecmp, iFppu)
//GO(SDL_strncmp, iFppu)
//GO(SDL_strrchr, pFpi)
//GO(SDL_strrev, pFp)
//GO(SDL_strstr, pFpp)
//GO(SDL_strtod, dFpp)
//GO(SDL_strtol, iFppi)
//GO(SDL_strtoll, IFppi)
//GO(SDL_strtoul, uFppi)
//GO(SDL_strtoull, UFppi)
//GO(SDL_strupr, pFp)
//GO(SDL_ulltoa, pFLpi)
//GO(SDL_ultoa, pFLpi)
//GOM(SDL_vsnprintf, iFpupV)
GOM(SDL_OpenAudio, iFEpp)
GO(SDL_LockAudio, vFv)
GO(SDL_UnlockAudio, vFv)
GO(SDL_PauseAudio, vFi)
GO(SDL_GetAudioStatus, uFv)
//GO(SDL_LoadWAV, pFpppp)
//GO(SDL_FreeWAV, vFp)
//GOM(SDL_LoadWAV_RW, pFEpippp)
GOM(SDL_BuildAudioCVT, iFpWCiWCi)   //%noE
GOM(SDL_ConvertAudio, iFp)  //%noE
//GO(SDL_AudioInit, iFp)
GO(SDL_AudioQuit, vFv)
GO(SDL_MixAudio, vFppui)
GO(SDL_CloseAudio, vFv)
//GO(SDL_AudioDriverName, pFpi)
