#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

// typedef PRUint32 PRIntervalTime;

//GO(GetExecutionEnvironment, 
//GO(libVersionPoint, 
//GO(LL_MaxInt, 
//GO(LL_MaxUint, 
//GO(LL_MinInt, 
GO(LL_Zero, lFv)
//GO(PR_Abort, 
//GO(PR_Accept, 
//GO(PR_AcceptRead, 
GO(PR_Access, iFpu)
//GO(PR_AddToCounter, 
//GO(PR_AddWaitFileDesc, 
//GO(PR_AllocFileDesc, 
GO(PR_Assert, vFppi)
GO(PR_AssertCurrentThreadInMonitor, vFp)
GO(PR_AssertCurrentThreadOwnsLock, vFp)
GO(PR_AtomicAdd, iFpi)
GO(PR_AtomicDecrement, iFp)
//GO(PR_AtomicIncrement, 
//GO(PR_AtomicSet, 
GO(PR_AttachSharedMemory, pFpi)
//GO(PR_AttachThread, 
//GO(PR_Available, 
//GO(PR_Available64, 
//GO(PR_Bind, 
//GO(PR_BlockClockInterrupts, 
//GO(PR_BlockInterrupt, 
GO(PR_Calloc, pFuu)
GOM(PR_CallOnce, iFEpp)
GOM(PR_CallOnceWithArg, iFEppp)
//GO(PR_CancelJob, 
//GO(PR_CancelWaitFileDesc, 
//GO(PR_CancelWaitGroup, 
//GO(PR_CeilingLog2, 
GO(PR_CEnterMonitor, pFp)
//GO(PR_CExitMonitor, 
//GO(PR_ChangeFileDescNativeHandle, 
GO(PR_Cleanup, iFv)
//GO(PR_ClearInterrupt, 
//GO(PR_ClearThreadGCAble, 
GO(PR_Close, iFp)
GO(PR_CloseDir, iFp)
GO(PR_CloseFileMap, iFp)
GO(PR_CloseSemaphore, iFp)
GO(PR_CloseSharedMemory, iFp)
//GO(PR_CNotify, 
GO(PR_CNotifyAll, iFp)
GO(PR_cnvtf, vFpiid)
//GO(PR_Connect, 
//GO(PR_ConnectContinue, 
GO(PR_ConvertIPv4AddrToIPv6, vFup)
//GO(PR_CreateAlarm, 
//GO(PR_CreateCounter, 
//GO(PR_CreateFileMap, 
//GO(PR_CreateIOLayer, 
GO(PR_CreateIOLayerStub, pFip)
GO(PR_CreateMWaitEnumerator, pFp)
//GO(PR_CreateOrderedLock, 
GO(PR_CreatePipe, iFpp)
GO(PR_CreateProcess, pFpppp)
GO(PR_CreateProcessDetached, iFpppp)
//GO(PR_CreateSocketPollFd, 
GO(PR_CreateStack, pFp)
//GO(PR_CreateThread, 
//GO(PR_CreateThreadGCAble, 
GO(PR_CreateThreadPool, pFiiu)
GO(PR_CreateTrace, pFppp)
GO(PR_CreateWaitGroup, pFi)
//GO(PR_CSetOnMonitorRecycle, 
GO(PR_CWait, iFpu)
//GO(PR_DecrementCounter, 
//GO(PR_Delete, 
//GO(PR_DeleteSemaphore, 
GO(PR_DeleteSharedMemory, iFp)
//GO(PR_DestroyAlarm, 
GO(PR_DestroyCondVar, vFp)
//GO(PR_DestroyCounter, 
GO(PR_DestroyLock, vFp)
GO(PR_DestroyMonitor, vFp)
GO(PR_DestroyMWaitEnumerator, iFp)
//GO(PR_DestroyOrderedLock, 
//GO(PR_DestroyPollableEvent, 
GO(PR_DestroyProcessAttr, vFp)
//GO(PR_DestroyRWLock, 
//GO(PR_DestroySem, 
//GO(PR_DestroySocketPollFd, 
GO(PR_DestroyStack, iFp)
GO(PR_DestroyTrace, vFp)
GO(PR_DestroyWaitGroup, iFp)
//GO(PR_DetachProcess, 
GO(PR_DetachSharedMemory, iFpp)
GO(PR_DetachThread, vFv)
//GO(PR_DisableClockInterrupts, 
GO(PR_dtoa, iFdiippppL)
GO(PR_DuplicateEnvironment, pFv)
GO(PR_EmulateAcceptRead, iFppppiu)
GO(PR_EmulateSendFile, iFppiu)
//GO(PR_EnableClockInterrupts, 
GO(PR_EnterMonitor, vFp)
GO(PR_EnumerateAddrInfo, pFppWp)
GO(PR_EnumerateHostEnt, iFipWp)
//GO(PR_EnumerateThreads, 
//GO(PR_EnumerateWaitGroup, 
//GO(PR_ErrorInstallCallback, 
GO(PR_ErrorInstallTable, iFp)
GO(PR_ErrorLanguages, pFv)
GO(PR_ErrorToName, pFi)
GO(PR_ErrorToString, pFiu)
GO(PR_ExitMonitor, iFp)
//GO(PR_ExplodeTime, 
GO(PR_ExportFileMapAsString, iFpLp)
//GO(PR_FD_CLR, 
//GO(PR_FD_ISSET, 
//GO(PR_FD_NCLR, 
//GO(PR_FD_NISSET, 
//GO(PR_FD_NSET, 
//GO(PR_FD_SET, 
//GO(PR_FD_ZERO, 
//GO(PR_FileDesc2NativeHandle, 
GO(PR_FindFunctionSymbol, pFpp)
GO(PR_FindFunctionSymbolAndLibrary, pFpp)
//GO(PR_FindNextCounterQname, 
//GO(PR_FindNextCounterRname, 
GO(PR_FindNextTraceQname, pFp)
GO(PR_FindNextTraceRname, pFpp)
GO(PR_FindSymbol, pFpp)
GO(PR_FindSymbolAndLibrary, pFpp)
GO(PR_FloorLog2, iFu)
GO(PR_FormatTime, uFpipp)
GO(PR_FormatTimeUSEnglish, uFpupp)
//GO(PR_fprintf, 
//GO(PR_FPrintZoneStats, 
GO(PR_Free, vFp)
GO(PR_FreeAddrInfo, vFp)
//GO(PR_FreeLibraryName, 
GO(PR_GetAddrInfoByName, pFpWi)
GO(PR_GetCanonNameFromAddrInfo, pFp)
//GO(PR_GetConnectStatus, 
//GO(PR_GetCounter, 
//GO(PR_GetCounterHandleFromName, 
//GO(PR_GetCounterNameFromHandle, 
GO(PR_GetCurrentThread, pFv)
GO(PR_GetDefaultIOMethods, pFv)
//GO(PR_GetDescType, 
//GO(PR_GetDirectorySeparator, 
//GO(PR_GetDirectorySepartor, 
//GO(PR_GetEnv, 
//GO(PR_GetEnvSecure, 
GO(PR_GetError, iFv)
GO(PR_GetErrorText, iFp)
GO(PR_GetErrorTextLength, iFv)
GO(PR_GetFileInfo, iFpp)
GO(PR_GetFileInfo64, iFpp)
//GO(PR_GetFileMethods, 
//GO(PR_GetGCRegisters, 
GO(PR_GetHostByAddr, iFppip)
GO(PR_GetHostByName, iFppip)
GO(PR_GetIdentitiesLayer, pFpi)
//GO(PR_GetInheritedFD, 
//GO(PR_GetInheritedFileMap, 
GO(PR_GetIPNodeByName, iFpWipip)
//GO(PR_GetLayersIdentity, 
//GO(PR_GetLibraryFilePathname, 
GO(PR_GetLibraryName, pFpp)
GO(PR_GetLibraryPath, pFv)
//GO(PR_GetMemMapAlignment, 
//GO(PR_GetMonitorEntryCount, 
GO(PR_GetNameForIdentity, pFi)
GO(PR_GetNumberOfProcessors, iFv)
//GO(PR_GetOpenFileInfo, 
//GO(PR_GetOpenFileInfo64, 
GO(PR_GetOSError, iFv)
//GO(PR_GetPageShift, 
//GO(PR_GetPageSize, 
GO(PR_GetPathSeparator, cFv)
//GO(PR_GetPeerName, 
GO(PR_GetPhysicalMemorySize, LFv)
//GO(PR_GetPipeMethods, 
GO(PR_GetProtoByName, iFppip)
GO(PR_GetProtoByNumber, iFipip)
GO(PR_GetRandomNoise, LFpL)
GO(PR_GetSocketOption, iFpp)
//GO(PR_GetSockName, 
//GO(PR_GetSP, 
//GO(PR_GetSpecialFD, 
//GO(PR_GetStackSpaceLeft, 
//GO(PR_GetSysfdTableMax, 
GO(PR_GetSystemInfo, iFupu)
//GO(PR_GetTCPMethods, 
//GO(PR_GetThreadAffinityMask, 
//GO(PR_GetThreadID, 
GO(PR_GetThreadName, pFp)
GO(PR_GetThreadPriority, uFp)
GO(PR_GetThreadPrivate, pFu)
GO(PR_GetThreadScope, uFp)
GO(PR_GetThreadState, uFp)
GO(PR_GetThreadType, uFp)
GO(PR_GetTraceEntries, iFpip)
GO(PR_GetTraceHandleFromName, pFpp)
GO(PR_GetTraceNameFromHandle, vFpppp)
GO(PR_GetTraceOption, vFup)
//GO(PR_GetUDPMethods, 
GO(PR_GetUniqueIdentity, iFp)
GO(PR_GetVersion, pFv)
//GO(PR_GMTParameters, 
GO(PR_htonl, uFu)
GO(PR_htonll, LFL)
GO(PR_htons, WFW)
GO(PR_ImplodeTime, lFp)
//GO(PR_ImportFile, 
GO(PR_ImportFileMapFromString, pFp)
//GO(PR_ImportPipe, 
GO(PR_ImportTCPSocket, pFi)
//GO(PR_ImportUDPSocket, 
//GO(PR_IncrementCounter, 
GO(PR_Init, vFuuu)
//GO(PR_Initialize, 
GO(PR_Initialized, iFv)
GO(PR_InitializeNetAddr, iFuWp)
GO(PR_Interrupt, iFp)
GO(PR_IntervalNow, uFv)
GO(PR_IntervalToMicroseconds, uFu)
GO(PR_IntervalToMilliseconds, uFu)
//GO(PR_IntervalToSeconds, 
GO(PR_IsNetAddrType, iFpu)
GO(PR_JoinJob, iFp)
//GO(PR_JoinThread, 
GO(PR_JoinThreadPool, iFp)
GO(PR_KillProcess, iFp)
//GO(PR_Listen, 
//GO(PR_LoadLibrary, pFp)   // needs wrapping
//GO(PR_LoadLibraryWithFlags, 
//GO(PR_LoadStaticLibrary, 
//GO(PR_LocalTimeParameters, 
GO(PR_Lock, vFp)
//GO(PR_LockFile, 
//GO(PR_LockOrderedLock, 
//GO(PR_LogFlush, 
//GO(PR_LogPrint, 
GO(PR_MakeDir, iFpi)
GO(PR_Malloc, pFu)
//GO(PR_MemMap, pFplu)  // needs wrapping
//GO(PR_MemUnmap, iFpu) // needs wrapping
GO(PR_MicrosecondsToInterval, uFu)
GO(PR_MillisecondsToInterval, uFu)
//GO(PR_MkDir, 
GO(PR_NetAddrToString, iFppu)
GO(PR_NewCondVar, pFp)
GO(PR_NewLock, pFv)
GO(PR_NewLogModule, pFp)
GO(PR_NewMonitor, pFv)
//GO(PR_NewNamedMonitor, 
//GO(PR_NewPollableEvent, 
GO(PR_NewProcessAttr, pFv)
GO(PR_NewRWLock, pFup)
//GO(PR_NewSem, 
//GO(PR_NewTCPSocket, 
GO(PR_NewTCPSocketPair, iFp)
//GO(PR_NewThreadPrivateIndex, 
//GO(PR_NewUDPSocket, 
//GO(PR_NormalizeTime, 
//GO(PR_Notify, 
GO(PR_NotifyAll, iFp)
GO(PR_NotifyAllCondVar, iFp)
//GO(PR_NotifyCondVar, 
GO(PR_Now, lFv)
//GO(PR_ntohl, 
//GO(PR_ntohll, 
//GO(PR_ntohs, 
//GO(PR_Open, 
GO(PR_OpenAnonFileMap, pFpLu)
GO(PR_OpenDir, pFp)
//GO(PR_OpenFile, 
GO(PR_OpenSemaphore, pFpiiu)
GO(PR_OpenSharedMemory, pFpLii)
//GO(PR_OpenTCPSocket, 
//GO(PR_OpenUDPSocket, 
GO(PR_ParseTimeString, iFpip)
GO(PR_ParseTimeStringToExplodedTime, iFpip)
//GO(PRP_DestroyNakedCondVar, 
//GO(PRP_NakedBroadcast, 
//GO(PRP_NakedNotify, 
//GO(PRP_NakedWait, 
//GO(PRP_NewNakedCondVar, 
//GO(PR_Poll, 
GO(PR_PopIOLayer, pFpi)
//GO(PR_PostSem, 
//GO(PR_PostSemaphore, 
GO(PR_ProcessAttrSetCurrentDirectory, iFpp)
//GO(PR_ProcessAttrSetInheritableFD, 
GO(PR_ProcessAttrSetInheritableFileMap, iFppp)
//GO(PR_ProcessAttrSetStdioRedirect, 
//GO(PR_ProcessExit, 
//GO(PRP_TryLock, 
GO(PR_PushIOLayer, iFpip)
//GO(_pr_push_ipv6toipv4_layer, 
//GO(PR_QueueJob, 
//GO(PR_QueueJob_Accept, 
//GO(PR_QueueJob_Connect, 
//GO(PR_QueueJob_Read, 
//GO(PR_QueueJob_Timer, 
//GO(PR_QueueJob_Write, 
GO(PR_Read, iFppi)
GO(PR_ReadDir, pFpu)
GO(PR_Realloc, pFpu)
GO(PR_RecordTraceEntries, vFv)
//GO(PR_Recv, 
//GO(PR_RecvFrom, 
GO(PR_Rename, iFpp)
//GO(PR_ResetAlarm, 
//GO(PR_ResetProcessAttr, 
//GO(PR_ResumeAll, 
//GO(PR_RmDir, 
//GO(PR_RWLock_Rlock, 
GO(PR_RWLock_Unlock, vFp)
//GO(PR_RWLock_Wlock, 
//GO(PR_ScanStackPointers, 
//GO(PR_SecondsToInterval, 
//GO(PR_Seek, 
//GO(PR_Seek64, 
//GO(PR_Select, 
//GO(PR_Send, 
//GO(PR_SendFile, 
//GO(PR_SendTo, 
//GO(PR_SetAlarm, 
GO(PR_SetConcurrency, vFu)
//GO(PR_SetCounter, 
//GO(PR_SetCurrentThreadName, 
//GO(PR_SetEnv, 
GO(PR_SetError, vFii)
GO(PR_SetErrorText, vFip)
GO(PR_SetFDCacheSize, iFii)
//GO(PR_SetFDInheritable, 
//GO(PR_SetLibraryPath, 
GO(PR_SetLogBuffering, vFi)
GO(PR_SetLogFile, iFp)
GO(PR_SetNetAddr, iFuWWp)
//GO(PR_SetPollableEvent, 
GO(PR_SetSocketOption, iFpp)
//GO(PR_SetStdioRedirect, 
//GO(PR_SetSysfdTableSize, 
//GO(PR_SetThreadAffinityMask, 
//GO(PR_SetThreadDumpProc, 
//GO(PR_SetThreadGCAble, 
GO(PR_SetThreadPriority, vFpu)
GO(PR_SetThreadPrivate, iFup)
//GO(PR_SetThreadRecycleMode, 
//GO(PR_SetTraceOption, 
//GO(PR_Shutdown, 
//GO(PR_ShutdownThreadPool, 
GO(PR_Sleep, iFu)
//GO(PR_smprintf, 
GO(PR_smprintf_free, vFp)
//GO(PR_snprintf, 
//GO(PR_Socket, 
//GO(PR_sprintf_append, 
//GO(PR_sscanf, 
GO(PR_StackPop, pFp)
GO(PR_StackPush, vFpp)
//GO(PR_Stat, 
GO(PR_StringToNetAddr, iFpp)
GO(PR_strtod, dFpp)
//GO(PR_SubtractFromCounter, 
//GO(PR_SuspendAll, 
//GO(PR_sxprintf, 
//GO(PR_Sync, 
//GO(PR_SyncMemMap, 
//GO(_pr_test_ipv6_socket, 
//GO(PR_ThreadScanStackPointers, 
GO(PR_TicksPerSecond, uFv)
//GO(PR_TLockFile, 
GO(PR_Trace, vFpuuuuuuuu)
//GO(PR_TransmitFile, 
//GO(PR_UnblockClockInterrupts, 
//GO(PR_UnblockInterrupt, 
GO(PR_UnloadLibrary, iFp)
GO(PR_Unlock, iFp)
//GO(PR_UnlockFile, 
//GO(PR_UnlockOrderedLock, 
//GO(PR_USPacificTimeParameters, 
//GO(PR_VersionCheck, 
//GO(PR_vfprintf, 
GO(PR_vsmprintf, pFpp)
GO(PR_vsnprintf, uFpupp)
GO(PR_vsprintf_append, pFppp)
//GO(PR_vsxprintf, 
GO(PR_Wait, iFpu)
GO(PR_WaitCondVar, iFpu)
//GO(PR_WaitForPollableEvent, 
GO(PR_WaitProcess, iFpp)
//GO(PR_WaitRecvReady, 
//GO(PR_WaitSem, 
//GO(PR_WaitSemaphore, 
GO(PR_Write, iFppi)
//GO(PR_Writev, 
//GO(PR_Yield, 
//GO(PT_FPrintStats, 
//GO(SetExecutionEnvironment, 
