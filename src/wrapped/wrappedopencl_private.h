#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GOM(clBuildProgram, iFEpupppp)
//GO(clCloneKernel, 
GOM(clCompileProgram, iFEpuppupppp)
GO(clCreateBuffer, pFpuLpp)
//GO(clCreateBufferWithProperties, 
GO(clCreateCommandQueue, pFppup)
//GO(clCreateCommandQueueWithProperties, 
GOM(clCreateContext, pFEpupppp)
GOM(clCreateContextFromType, pFEpuppp)
//GO(clCreateFromGLBuffer, 
//GO(clCreateFromGLRenderbuffer, 
//GO(clCreateFromGLTexture, 
//GO(clCreateFromGLTexture2D, 
//GO(clCreateFromGLTexture3D, 
GO(clCreateImage, pFpupppp)
GO(clCreateImage2D, pFpupLLLpp)
GO(clCreateImage3D, pFpupLLLLLpp)
//GO(clCreateImageWithProperties, 
GO(clCreateKernel, pFppp)
GO(clCreateKernelsInProgram, iFpupp)
//GO(clCreatePipe, 
GO(clCreateProgramWithBinary, pFpuppppp)
GO(clCreateProgramWithBuiltInKernels, pFpuppp)
//GO(clCreateProgramWithIL, 
GO(clCreateProgramWithSource, pFpuppp)
GO(clCreateSampler, pFpuuup)
//GO(clCreateSamplerWithProperties, 
GO(clCreateSubBuffer, pFpuupp)
GO(clCreateSubDevices, iFppupp)
GO(clCreateUserEvent, pFpp)
//GO(clEnqueueAcquireGLObjects, 
GO(clEnqueueBarrier, iFp)
GO(clEnqueueBarrierWithWaitList, iFpupp)
GO(clEnqueueCopyBuffer, iFpppLLLupp)
GO(clEnqueueCopyBufferRect, iFppppppLLLLupp)
GO(clEnqueueCopyBufferToImage, iFpppLppupp)
GO(clEnqueueCopyImage, iFppppppupp)
GO(clEnqueueCopyImageToBuffer, iFpppppLupp)
GO(clEnqueueFillBuffer, iFpppLLLupp)
GO(clEnqueueFillImage, iFpppppupp)
GO(clEnqueueMapBuffer, pFppuuLLuppp)
GO(clEnqueueMapImage, pFppuuppppuppp)
GO(clEnqueueMarker, iFpp)
GO(clEnqueueMarkerWithWaitList, iFpupp)
GO(clEnqueueMigrateMemObjects, iFpupuupp)
GOM(clEnqueueNativeKernel, iFEppLuppupp)
GO(clEnqueueNDRangeKernel, iFppupppupp)
GO(clEnqueueReadBuffer, iFppuLLpupp)
GO(clEnqueueReadBufferRect, iFppupppLLLLpupp)
GO(clEnqueueReadImage, iFppuppLLpupp)
//GO(clEnqueueReleaseGLObjects, 
//GO(clEnqueueSVMFree, 
//GO(clEnqueueSVMMap, 
//GO(clEnqueueSVMMemcpy, 
//GO(clEnqueueSVMMemFill, 
//GO(clEnqueueSVMMigrateMem, 
//GO(clEnqueueSVMUnmap, 
GO(clEnqueueTask, iFppupp)
GO(clEnqueueUnmapMemObject, iFpppupp)
GO(clEnqueueWaitForEvents, iFpup)
GO(clEnqueueWriteBuffer, iFppuLLpupp)
GO(clEnqueueWriteBufferRect, iFppupppLLLLpupp)
GO(clEnqueueWriteImage, iFppuppLLpupp)
GO(clFinish, iFp)
GO(clFlush, iFp)
GO(clGetCommandQueueInfo, iFpuLpp)
GO(clGetContextInfo, iFpuLpp)
//GO(clGetDeviceAndHostTimer, 
GO(clGetDeviceIDs, iFpuupp)
GO(clGetDeviceInfo, iFpuLpL)
GO(clGetEventInfo, iFpuLpp)
GO(clGetEventProfilingInfo, iFpuLpp)
//GO(clGetExtensionFunctionAddress, 
//GO(clGetExtensionFunctionAddressForPlatform, 
//GO(clGetGLObjectInfo, 
//GO(clGetGLTextureInfo, 
//GO(clGetHostTimer, 
GO(clGetImageInfo, pFpuLpp)
GO(clGetKernelArgInfo, iFpuuLpp)
GO(clGetKernelInfo, iFpuLpp)
//GO(clGetKernelSubGroupInfo, 
GO(clGetKernelWorkGroupInfo, iFppuLpp)
GO(clGetMemObjectInfo, iFpuLpp)
//GO(clGetPipeInfo, 
GO(clGetPlatformIDs, iFupp)
GO(clGetPlatformInfo, iFpuLpp)
GO(clGetProgramBuildInfo, iFppuLpp)
GO(clGetProgramInfo, iFpuLpL)
GO(clGetSamplerInfo, iFpuLpp)
GO(clGetSupportedImageFormats, iFpuuupp)
GOM(clLinkProgram, pFEpuppupppp)
GO(clReleaseCommandQueue, iFp)
GO(clReleaseContext, iFp)
GO(clReleaseDevice, iFp)
GO(clReleaseEvent, iFp)
GO(clReleaseKernel, iFp)
GO(clReleaseMemObject, iFp)
GO(clReleaseProgram, iFp)
GO(clReleaseSampler, iFp)
GO(clRetainCommandQueue, iFp)
GO(clRetainContext, iFp)
GO(clRetainDevice, iFp)
GO(clRetainEvent, iFp)
GO(clRetainKernel, iFp)
GO(clRetainMemObject, iFp)
GO(clRetainProgram, iFp)
GO(clRetainSampler, iFp)
//GO(clSetCommandQueueProperty, 
//GO(clSetContextDestructorCallback, 
//GO(clSetDefaultDeviceCommandQueue, 
GOM(clSetEventCallback, iFEppp)
GO(clSetKernelArg, iFpuLp)
//GO(clSetKernelArgSVMPointer, 
//GO(clSetKernelExecInfo, 
GOM(clSetMemObjectDestructorCallback, iFEppp)
//GO(clSetProgramReleaseCallback, 
//GO(clSetProgramSpecializationConstant, 
GO(clSetUserEventStatus, iFpi)
//GO(clSVMAlloc, 
//GO(clSVMFree, 
GO(clUnloadCompiler, iFv)
GO(clUnloadPlatformCompiler, iFp)
GO(clWaitForEvents, iFup)

//OPENCL_1.0
//OPENCL_1.1
//OPENCL_1.2
//OPENCL_2.0
//OPENCL_2.1
//OPENCL_2.2
//OPENCL_3.0
