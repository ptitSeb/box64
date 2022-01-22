#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

//GO(CreateFAPOBase, 
//GO(CreateFAPOBaseWithCustomAllocatorEXT, 
//DATA(DATAFORMAT_SUBTYPE_ADPCM, 
//DATA(DATAFORMAT_SUBTYPE_IEEE_FLOAT, 
//DATA(DATAFORMAT_SUBTYPE_PCM, 
//DATA(DATAFORMAT_SUBTYPE_WMAUDIO2, 
//DATA(DATAFORMAT_SUBTYPE_WMAUDIO3, 
//DATA(DATAFORMAT_SUBTYPE_WMAUDIO_LOSSLESS, 
//DATA(DATAFORMAT_SUBTYPE_XMAUDIO2, 
GO(F3DAudioCalculate, vFpppup)
//GO(F3DAudioCheckCalculateParams, 
//GO(F3DAudioCheckInitParams, 
GO(F3DAudioInitialize, vFufp)
GO(F3DAudioInitialize8, uFufp)
//GO(FACT3DApply, 
//GO(FACT3DCalculate, 
//GO(FACT3DInitialize, 
//GO(FACTAudioEngine_AddRef, 
//GO(FACTAudioEngine_CreateInMemoryWaveBank, 
//GO(FACTAudioEngine_CreateSoundBank, 
//GO(FACTAudioEngine_CreateStreamingWaveBank, 
//GO(FACTAudioEngine_DoWork, 
//GO(FACTAudioEngine_GetCategory, 
//GO(FACTAudioEngine_GetFinalMixFormat, 
//GO(FACTAudioEngine_GetGlobalVariable, 
//GO(FACTAudioEngine_GetGlobalVariableIndex, 
//GO(FACTAudioEngine_GetRendererCount, 
//GO(FACTAudioEngine_GetRendererDetails, 
//GO(FACTAudioEngine_Initialize, 
//GO(FACTAudioEngine_Pause, 
//GO(FACTAudioEngine_PrepareInMemoryWave, 
//GO(FACTAudioEngine_PrepareStreamingWave, 
//GO(FACTAudioEngine_PrepareWave, 
//GO(FACTAudioEngine_RegisterNotification, 
//GO(FACTAudioEngine_Release, 
//GO(FACTAudioEngine_SetGlobalVariable, 
//GO(FACTAudioEngine_SetVolume, 
//GO(FACTAudioEngine_ShutDown, 
//GO(FACTAudioEngine_Stop, 
//GO(FACTAudioEngine_UnRegisterNotification, 
//GO(FACTCreateEngine, 
//GO(FACTCreateEngineWithCustomAllocatorEXT, 
//GO(FACTCue_Destroy, 
//GO(FACTCue_GetProperties, 
//GO(FACTCue_GetState, 
//GO(FACTCue_GetVariable, 
//GO(FACTCue_GetVariableIndex, 
//GO(FACTCue_Pause, 
//GO(FACTCue_Play, 
//GO(FACTCue_SetMatrixCoefficients, 
//GO(FACTCue_SetOutputVoiceMatrix, 
//GO(FACTCue_SetOutputVoices, 
//GO(FACTCue_SetVariable, 
//GO(FACTCue_Stop, 
//GO(FACT_INTERNAL_ActivateEvent, 
//GO(FACT_INTERNAL_APIThread, 
//GO(FACT_INTERNAL_BeginFadeOut, 
//GO(FACT_INTERNAL_BeginReleaseRPC, 
//GO(FACT_INTERNAL_CalculateRPC, 
//GO(FACT_INTERNAL_CreateSound, 
//GO(FACT_INTERNAL_DefaultGetOverlappedResult, 
//GO(FACT_INTERNAL_DefaultReadFile, 
//GO(FACT_INTERNAL_DestroySound, 
//GO(FACT_INTERNAL_GetNextWave, 
//GO(FACT_INTERNAL_GetRPC, 
//GO(FACT_INTERNAL_IsInCategory, 
//GO(FACT_INTERNAL_OnBufferEnd, 
//GO(FACT_INTERNAL_OnStreamEnd, 
//GO(FACT_INTERNAL_ParseAudioEngine, 
//GO(FACT_INTERNAL_ParseSoundBank, 
//GO(FACT_INTERNAL_ParseTrackEvents, 
//GO(FACT_INTERNAL_ParseWaveBank, 
//GO(FACT_INTERNAL_UpdateCue, 
//GO(FACT_INTERNAL_UpdateEngine, 
//GO(FACT_INTERNAL_UpdateRPCs, 
//GO(FACT_INTERNAL_UpdateSound, 
//GO(FACTSoundBank_Destroy, 
//GO(FACTSoundBank_GetCueIndex, 
//GO(FACTSoundBank_GetCueProperties, 
//GO(FACTSoundBank_GetNumCues, 
//GO(FACTSoundBank_GetState, 
//GO(FACTSoundBank_Play, 
//GO(FACTSoundBank_Play3D, 
//GO(FACTSoundBank_Prepare, 
//GO(FACTSoundBank_Stop, 
//GO(FACTWaveBank_Destroy, 
//GO(FACTWaveBank_GetNumWaves, 
//GO(FACTWaveBank_GetState, 
//GO(FACTWaveBank_GetWaveIndex, 
//GO(FACTWaveBank_GetWaveProperties, 
//GO(FACTWaveBank_Play, 
//GO(FACTWaveBank_Prepare, 
//GO(FACTWaveBank_Stop, 
//GO(FACTWave_Destroy, 
//GO(FACTWave_GetProperties, 
//GO(FACTWave_GetState, 
//GO(FACTWave_Pause, 
//GO(FACTWave_Play, 
//GO(FACTWave_SetMatrixCoefficients, 
//GO(FACTWave_SetPitch, 
//GO(FACTWave_SetVolume, 
//GO(FACTWave_Stop, 
//GO(FAPOBase_AddRef, 
//GO(FAPOBase_BeginProcess, 
//GO(FAPOBase_CalcInputFrames, 
//GO(FAPOBase_CalcOutputFrames, 
//GO(FAPOBase_EndProcess, 
//GO(FAPOBase_GetParameters, 
//GO(FAPOBase_GetRegistrationProperties, 
//GO(FAPOBase_Initialize, 
//GO(FAPOBase_IsInputFormatSupported, 
//GO(FAPOBase_IsOutputFormatSupported, 
//GO(FAPOBase_LockForProcess, 
//GO(FAPOBase_OnSetParameters, 
//GO(FAPOBase_ParametersChanged, 
//GO(FAPOBase_ProcessThru, 
//GO(FAPOBase_Release, 
//GO(FAPOBase_Reset, 
//GO(FAPOBase_SetParameters, 
//GO(FAPOBase_UnlockForProcess, 
//GO(FAPOBase_ValidateFormatDefault, 
//GO(FAPOBase_ValidateFormatPair, 
//GO(FAPOFXCreateEcho, 
//GO(FAPOFXCreateEQ, 
GO(FAPOFX_CreateFX, uFpppu)
GOM(FAPOFX_CreateFXWithCustomAllocatorEXT, uFEpppuppp)
//GO(FAPOFXCreateMasteringLimiter, 
//GO(FAPOFXCreateReverb, 
//GO(FAPOFXEcho_Free, 
//GO(FAPOFXEcho_Initialize, 
//GO(FAPOFXEcho_Process, 
//GO(FAPOFXEQ_Free, 
//GO(FAPOFXEQ_Initialize, 
//GO(FAPOFXEQ_Process, 
//GO(FAPOFXMasteringLimiter_Free, 
//GO(FAPOFXMasteringLimiter_Initialize, 
//GO(FAPOFXMasteringLimiter_Process, 
//GO(FAPOFXReverb_Free, 
//GO(FAPOFXReverb_Initialize, 
//GO(FAPOFXReverb_Process, 
GO(FAudio_AddRef, uFp)
//GO(FAudio_close, 
GO(FAudioCOMConstructEXT, uFpuC)
GOM(FAudioCOMConstructWithCustomAllocatorEXT, uFEpCppp)
GO(FAudio_CommitChanges, uFp)
GO(FAudio_CommitOperationSet, uFpu)
GO(FAudioCreate, uFpuu)
GO(FAudio_CreateMasteringVoice, uFppuuuup)
GO(FAudio_CreateMasteringVoice8, uFppuuuppi)
GO(FAudioCreateReverb, uFpu)
GO(FAudioCreateReverb9, uFpu)
GOM(FAudioCreateReverb9WithCustomAllocatorEXT, uFEpuppp)
GOM(FAudioCreateReverbWithCustomAllocatorEXT, uFEpuppp)
GOM(FAudio_CreateSourceVoice, uFEpppufppp)
GO(FAudio_CreateSubmixVoice, uFppuuuupp)
GO(FAudioCreateVolumeMeter, uFpu)
GOM(FAudioCreateVolumeMeterWithCustomAllocatorEXT, uFEpuppp)
GOM(FAudioCreateWithCustomAllocatorEXT, uFEpuuppp)
//GO(FAudio_fopen, 
//GO(FAudioFXReverb_Free, 
//GO(FAudioFXReverb_Initialize, 
//GO(FAudioFXReverb_IsInputFormatSupported, 
//GO(FAudioFXReverb_IsOutputFormatSupported, 
//GO(FAudioFXReverb_LockForProcess, 
//GO(FAudioFXReverb_Process, 
//GO(FAudioFXReverb_Reset, 
//GO(FAudioFXVolumeMeter_Free, 
//GO(FAudioFXVolumeMeter_GetParameters, 
//GO(FAudioFXVolumeMeter_LockForProcess, 
//GO(FAudioFXVolumeMeter_Process, 
//GO(FAudioFXVolumeMeter_UnlockForProcess, 
GO(FAudio_GetDeviceCount, uFpp)
GO(FAudio_GetDeviceDetails, uFpup)
GO(FAudio_GetPerformanceData, vFpp)
GO(FAudio_GetProcessingQuantum, vFppp)
//GO(FAudio_GSTREAMER_end_buffer, 
//GO(FAudio_GSTREAMER_free, 
//GO(FAudio_GSTREAMER_init, 
GO(FAudio_Initialize, uFpuu)
//GO(FAudio_INTERNAL_AllocEffectChain, 
//DATAB(FAudio_INTERNAL_Amplify, 
//GO(FAudio_INTERNAL_Amplify_NEON, 
//DATAB(FAudio_INTERNAL_Convert_S16_To_F32, 
//GO(FAudio_INTERNAL_Convert_S16_To_F32_NEON, 
//DATAB(FAudio_INTERNAL_Convert_S32_To_F32, 
//GO(FAudio_INTERNAL_Convert_S32_To_F32_NEON, 
//DATAB(FAudio_INTERNAL_Convert_U8_To_F32, 
//GO(FAudio_INTERNAL_Convert_U8_To_F32_NEON, 
//GO(FAudio_INTERNAL_debug, 
//GO(FAudio_INTERNAL_debug_fmt, 
//GO(FAudio_INTERNAL_DecodeMonoMSADPCM, 
//GO(FAudio_INTERNAL_DecodePCM16, 
//GO(FAudio_INTERNAL_DecodePCM24, 
//GO(FAudio_INTERNAL_DecodePCM32, 
//GO(FAudio_INTERNAL_DecodePCM32F, 
//GO(FAudio_INTERNAL_DecodePCM8, 
//GO(FAudio_INTERNAL_DecodeStereoMSADPCM, 
//GO(FAudio_INTERNAL_DecodeWMAERROR, 
//GO(FAudio_INTERNAL_FreeEffectChain, 
//GO(FAudio_INTERNAL_InitSIMDFunctions, 
//GO(FAudio_INTERNAL_InsertSubmixSorted, 
//GO(FAudio_INTERNAL_Mix_1in_1out_Scalar, 
//GO(FAudio_INTERNAL_Mix_1in_2out_Scalar, 
//GO(FAudio_INTERNAL_Mix_1in_6out_Scalar, 
//GO(FAudio_INTERNAL_Mix_1in_8out_Scalar, 
//GO(FAudio_INTERNAL_Mix_2in_1out_Scalar, 
//GO(FAudio_INTERNAL_Mix_2in_2out_Scalar, 
//GO(FAudio_INTERNAL_Mix_2in_6out_Scalar, 
//GO(FAudio_INTERNAL_Mix_2in_8out_Scalar, 
//DATAB(FAudio_INTERNAL_Mix_Generic, 
//GO(FAudio_INTERNAL_Mix_Generic_Scalar, 
//GO(FAudio_INTERNAL_ResampleGeneric, 
//DATAB(FAudio_INTERNAL_ResampleMono, 
//GO(FAudio_INTERNAL_ResampleMono_NEON, 
//DATAB(FAudio_INTERNAL_ResampleStereo, 
//GO(FAudio_INTERNAL_ResampleStereo_NEON, 
//GO(FAudio_INTERNAL_ResizeDecodeCache, 
//GO(FAudio_INTERNAL_UpdateEngine, 
//GO(FAudio_INTERNAL_VoiceOutputFrequency, 
GO(FAudioLinkedVersion, uFv)
GO(FAudioMasteringVoice_GetChannelMask, uFpp)
//GO(FAudio_memopen, 
//GO(FAudio_memptr, 
//GO(FAudio_OPERATIONSET_ClearAll, 
//GO(FAudio_OPERATIONSET_ClearAllForVoice, 
//GO(FAudio_OPERATIONSET_Commit, 
//GO(FAudio_OPERATIONSET_CommitAll, 
//GO(FAudio_OPERATIONSET_Execute, 
//GO(FAudio_OPERATIONSET_QueueDisableEffect, 
//GO(FAudio_OPERATIONSET_QueueEnableEffect, 
//GO(FAudio_OPERATIONSET_QueueExitLoop, 
//GO(FAudio_OPERATIONSET_QueueSetChannelVolumes, 
//GO(FAudio_OPERATIONSET_QueueSetEffectParameters, 
//GO(FAudio_OPERATIONSET_QueueSetFilterParameters, 
//GO(FAudio_OPERATIONSET_QueueSetFrequencyRatio, 
//GO(FAudio_OPERATIONSET_QueueSetOutputFilterParameters, 
//GO(FAudio_OPERATIONSET_QueueSetOutputMatrix, 
//GO(FAudio_OPERATIONSET_QueueSetVolume, 
//GO(FAudio_OPERATIONSET_QueueStart, 
//GO(FAudio_OPERATIONSET_QueueStop, 
//GO(FAudio_PlatformAddRef, 
//GO(FAudio_PlatformCreateMutex, 
//GO(FAudio_PlatformCreateThread, 
//GO(FAudio_PlatformDestroyMutex, 
//GO(FAudio_PlatformGetDeviceCount, 
//GO(FAudio_PlatformGetDeviceDetails, 
//GO(FAudio_PlatformGetThreadID, 
//GO(FAudio_PlatformInit, 
//GO(FAudio_PlatformLockMutex, 
//GO(FAudio_PlatformQuit, 
//GO(FAudio_PlatformRelease, 
//GO(FAudio_PlatformThreadPriority, 
//GO(FAudio_PlatformUnlockMutex, 
//GO(FAudio_PlatformWaitThread, 
GOM(FAudio_RegisterForCallbacks, uFEpp)
GO(FAudio_Release, uFp)
GO(FAudio_SetDebugConfiguration, vFppp)
GOM(FAudio_SetEngineProcedureEXT, vFEppp)
//GO(FAudio_sleep, 
GO(FAudioSourceVoice_Discontinuity, uFp)
GO(FAudioSourceVoice_ExitLoop, uFpu)
GO(FAudioSourceVoice_FlushSourceBuffers, uFp)
GO(FAudioSourceVoice_GetFrequencyRatio, vFpp)
GO(FAudioSourceVoice_GetState, vFppu)
GO(FAudioSourceVoice_SetFrequencyRatio, uFpfu)
GO(FAudioSourceVoice_SetSourceSampleRate, uFpu)
GO(FAudioSourceVoice_Start, uFpuu)
GO(FAudioSourceVoice_Stop, uFpuu)
GO(FAudioSourceVoice_SubmitSourceBuffer, uFppp)
GO(FAudio_StartEngine, uFp)
GO(FAudio_StopEngine, vFp)
//GO(FAudio_timems, 
GOM(FAudio_UnregisterForCallbacks, vFEpp)
//GO(FAudio_UTF8_To_UTF16, 
GO(FAudioVoice_DestroyVoice, vFp)
GO(FAudioVoice_DisableEffect, uFpuu)
GO(FAudioVoice_EnableEffect, uFpuu)
GO(FAudioVoice_GetChannelVolumes, vFpup)
GO(FAudioVoice_GetEffectParameters, uFpupu)
GO(FAudioVoice_GetEffectState, vFpup)
GO(FAudioVoice_GetFilterParameters, vFpp)
GO(FAudioVoice_GetOutputFilterParameters, vFppp)
GO(FAudioVoice_GetOutputMatrix, vFppuup)
GO(FAudioVoice_GetVoiceDetails, vFpp)
GO(FAudioVoice_GetVolume, vFpp)
GO(FAudioVoice_SetChannelVolumes, uFpupu)
GO(FAudioVoice_SetEffectChain, uFpp)
GO(FAudioVoice_SetEffectParameters, uFpupuu)
GO(FAudioVoice_SetFilterParameters, uFppu)
GO(FAudioVoice_SetOutputFilterParameters, uFpppu)
GO(FAudioVoice_SetOutputMatrix, uFppuupu)
GO(FAudioVoice_SetOutputVoices, uFpp)
GO(FAudioVoice_SetVolume, uFpfu)
//DATA(kSpeakersConfigInfo, 
//GO(LinkedList_AddEntry, 
//GO(LinkedList_PrependEntry, 
//GO(LinkedList_RemoveEntry, 
GO(ReverbConvertI3DL2ToNative, vFpp)
GO(ReverbConvertI3DL2ToNative9, vFpp)
//GO(XNA_EnableVisualization, 
//GO(XNA_GetSongEnded, 
//GO(XNA_GetSongVisualizationData, 
//GO(XNA_PauseSong, 
//GO(XNA_PlaySong, 
//GO(XNA_ResumeSong, 
//GO(XNA_SetSongVolume, 
//GO(XNA_SongInit, 
//GO(XNA_SongQuit, 
//GO(XNA_StopSong, 
//GO(XNA_VisualizationEnabled, 
