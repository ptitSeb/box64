#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

//vkDeviceSize == uint64_t
//VkImageLayout == enum
//VK_DEFINE_NON_DISPATCHABLE_HANDLE == uint64_t
// VkAccelerationStructureNV  = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkBuffer = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkBufferView = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkCommandPool = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkDeferredOperationKHR = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkDescriptorPool = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkDescriptorSet = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkDescriptorSetLayout = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkDescriptorUpdateTemplate = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkDeviceMemory = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkDisplayKHR = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkDisplayModeKHR = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkEvent = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkFence = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkFramebuffer = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkImage = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkImageView = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkPipeline = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkPipelineCache = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkPipelineLayout = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkQueryPool = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkRenderPass = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkSampler = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkSamplerYcbcrConversion = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkSemaphore = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkShaderModule = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkSurfaceKHR = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkSwapchainKHR = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkPrivateDataSlot = VK_DEFINE_NON_DISPATCHABLE_HANDLE

// VkDeviceAddress = uint64_t

// VK_VERSION_1_0
GO(vkAllocateCommandBuffers, iFppp)
GO(vkAllocateDescriptorSets, iFppp)
GOM(vkAllocateMemory, iFEpppp)
GO(vkBeginCommandBuffer, iFpp)
GO(vkBindBufferMemory, iFpppU)
GO(vkBindImageMemory, iFpppU)
GO(vkCmdBeginQuery, vFppuu)
GO(vkCmdBeginRenderPass, vFppu)
GO(vkCmdBindDescriptorSets, vFpupuupup)
GO(vkCmdBindIndexBuffer, vFppUu)
GO(vkCmdBindPipeline, vFpup)
GO(vkCmdBindVertexBuffers, vFpuupp)
GO(vkCmdBlitImage, vFppupuupu)
GO(vkCmdClearAttachments, vFpupup)
GO(vkCmdClearColorImage, vFppupup)
GO(vkCmdClearDepthStencilImage, vFppupup)
GO(vkCmdCopyBuffer, vFpppup)
GO(vkCmdCopyBufferToImage, vFpppuup)
GO(vkCmdCopyImage, vFppupuup)
GO(vkCmdCopyImageToBuffer, vFppupup)
GO(vkCmdCopyQueryPoolResults, vFppuupUUu)
GO(vkCmdDispatch, vFpuuu)
GO(vkCmdDispatchIndirect, vFppU)
GO(vkCmdDraw, vFpuuuu)
GO(vkCmdDrawIndexed, vFpuuuiu)
GO(vkCmdDrawIndexedIndirect, vFppUuu)
GO(vkCmdDrawIndirect, vFppUuu)
GO(vkCmdEndQuery, vFppu)
GO(vkCmdEndRenderPass, vFp)
GO(vkCmdExecuteCommands, vFpup)
GO(vkCmdFillBuffer, vFppUUu)
GO(vkCmdNextSubpass, vFpu)
GO(vkCmdPipelineBarrier, vFpuuuupupup)
GO(vkCmdPushConstants, vFppuuup)
GO(vkCmdResetEvent, vFppu)
GO(vkCmdResetQueryPool, vFppuu)
GO(vkCmdResolveImage, vFppupuup)
GO(vkCmdSetBlendConstants, vFpp)
GO(vkCmdSetDepthBias, vFpfff)
GO(vkCmdSetDepthBounds, vFpff)
GO(vkCmdSetEvent, vFppu)
GO(vkCmdSetLineWidth, vFpf)
GO(vkCmdSetScissor, vFpuup)
GO(vkCmdSetStencilCompareMask, vFpuu)
GO(vkCmdSetStencilReference, vFpuu)
GO(vkCmdSetStencilWriteMask, vFpuu)
GO(vkCmdSetViewport, vFpuup)
GO(vkCmdUpdateBuffer, vFppUUp)
GO(vkCmdWaitEvents, vFpupuuupupup)
GO(vkCmdWriteTimestamp, vFpupu)
GOM(vkCreateBuffer, iFEpppp)
GOM(vkCreateBufferView, iFEpppp)
GOM(vkCreateCommandPool, iFEpppp)
GOM(vkCreateComputePipelines, iFEppuppp)
GOM(vkCreateDescriptorPool, iFEpppp)
GOM(vkCreateDescriptorSetLayout, iFEpppp)
GOM(vkCreateDevice, iFEpppp)
GOM(vkCreateEvent, iFEpppp)
GOM(vkCreateFence, iFEpppp)
GOM(vkCreateFramebuffer, iFEpppp)
GOM(vkCreateGraphicsPipelines, iFEppuppp)
GOM(vkCreateImage, iFEpppp)
GOM(vkCreateImageView, iFEpppp)
GOM(vkCreateInstance, iFEppp)
GOM(vkCreatePipelineCache, iFEpppp)
GOM(vkCreatePipelineLayout, iFEpppp)
GOM(vkCreateQueryPool, iFEpppp)
GOM(vkCreateRenderPass, iFEpppp)
GOM(vkCreateSampler, iFEpppp)
GOM(vkCreateSemaphore, iFEpppp)
GOM(vkCreateShaderModule, iFEpppp)
GOM(vkDestroyBuffer, vFEppp)
GOM(vkDestroyBufferView, vFEppp)
GOM(vkDestroyCommandPool, vFEppp)
GOM(vkDestroyDescriptorPool, vFEppp)
GOM(vkDestroyDescriptorSetLayout, vFEppp)
GOM(vkDestroyDevice, vFEpp)
GOM(vkDestroyEvent, vFEppp)
GOM(vkDestroyFence, vFEppp)
GOM(vkDestroyFramebuffer, vFEppp)
GOM(vkDestroyImage, vFEppp)
GOM(vkDestroyImageView, vFEppp)
GOM(vkDestroyInstance, vFEpp)
GOM(vkDestroyPipeline, vFEppp)
GOM(vkDestroyPipelineCache, vFEppp)
GOM(vkDestroyPipelineLayout, vFEppp)
GOM(vkDestroyQueryPool, vFEppp)
GOM(vkDestroyRenderPass, vFEppp)
GOM(vkDestroySampler, vFEppp)
GOM(vkDestroySemaphore, vFEppp)
GOM(vkDestroyShaderModule, vFEppp)
GO(vkDeviceWaitIdle, iFp)
GO(vkEndCommandBuffer, iFp)
GO(vkEnumerateDeviceExtensionProperties, iFpppp)
GO(vkEnumerateDeviceLayerProperties, iFppp)
GO(vkEnumerateInstanceExtensionProperties, iFppp)
GO(vkEnumerateInstanceLayerProperties, iFpp)
GO(vkEnumeratePhysicalDevices, iFppp)
GO(vkFlushMappedMemoryRanges, iFpup)
GO(vkFreeCommandBuffers, vFppup)
GO(vkFreeDescriptorSets, iFppup)
GOM(vkFreeMemory, vFEppp)
GO(vkGetBufferMemoryRequirements, vFppp)
GO(vkGetDeviceMemoryCommitment, vFppp)
GOM(vkGetDeviceProcAddr, pFEpp)
GO(vkGetDeviceQueue, vFpuup)
GO(vkGetEventStatus, iFpp)
GO(vkGetFenceStatus, iFpp)
GO(vkGetImageMemoryRequirements, vFppp)
GO(vkGetImageSparseMemoryRequirements, vFpppp)
GO(vkGetImageSubresourceLayout, vFpppp)
GOM(vkGetInstanceProcAddr, pFEpp)
GO(vkGetPhysicalDeviceFeatures, vFpp)
GO(vkGetPhysicalDeviceFormatProperties, vFpup)
GO(vkGetPhysicalDeviceImageFormatProperties, iFpuuuuup)
GO(vkGetPhysicalDeviceMemoryProperties, vFpp)
GO(vkGetPhysicalDeviceProperties, vFpp)
GO(vkGetPhysicalDeviceQueueFamilyProperties, vFppp)
GO(vkGetPhysicalDeviceSparseImageFormatProperties, vFpuuuuupp)
GO(vkGetPipelineCacheData, iFpppp)
GO(vkGetQueryPoolResults, iFppuuLpUu)
GO(vkGetRenderAreaGranularity, vFppp)
GO(vkInvalidateMappedMemoryRanges, iFpup)
GO(vkMapMemory, iFppUUup)
GO(vkMergePipelineCaches, iFppup)
GO(vkQueueBindSparse, iFpupp)
GO(vkQueueSubmit, iFpupp)
GO(vkQueueWaitIdle, iFp)
GO(vkResetCommandBuffer, iFpu)
GO(vkResetCommandPool, iFppu)
GO(vkResetDescriptorPool, iFppu)
GO(vkResetEvent, iFpp)
GO(vkResetFences, iFpup)
GO(vkSetEvent, iFpp)
GO(vkUnmapMemory, vFpp)
GO(vkUpdateDescriptorSets, vFpupup)
GO(vkWaitForFences, iFpupuU)

// VK_VERSION_1_1
GO(vkBindBufferMemory2, iFpup)
GO(vkBindImageMemory2, iFpup)
GO(vkCmdDispatchBase, vFpuuuuuu)
GO(vkCmdSetDeviceMask, vFpu)
GOM(vkCreateDescriptorUpdateTemplate, iFEpppp)
GOM(vkCreateSamplerYcbcrConversion, iFEpppp)
GOM(vkDestroyDescriptorUpdateTemplate, vFEppp)
GOM(vkDestroySamplerYcbcrConversion, vFEppp)
GO(vkEnumerateInstanceVersion, iFp)
GO(vkEnumeratePhysicalDeviceGroups, iFppp)
GO(vkGetBufferMemoryRequirements2, vFppp)
GO(vkGetImageMemoryRequirements2, vFppp)
GO(vkGetImageSparseMemoryRequirements2, vFpppp)
GO(vkGetDescriptorSetLayoutSupport, vFppp)
GO(vkGetDeviceGroupPeerMemoryFeatures, vFpuuup)
GO(vkGetDeviceQueue2, vFppp)
GO(vkGetPhysicalDeviceExternalBufferProperties, vFppp)
GO(vkGetPhysicalDeviceExternalFenceProperties, vFppp)
GO(vkGetPhysicalDeviceExternalSemaphoreProperties, vFppp)
GO(vkGetPhysicalDeviceFeatures2, vFpp)
GO(vkGetPhysicalDeviceFormatProperties2, vFpup)
GO(vkGetPhysicalDeviceImageFormatProperties2, iFppp)
GO(vkGetPhysicalDeviceMemoryProperties2, vFpp)
GOM(vkGetPhysicalDeviceProperties2, vFEpp)
GO(vkGetPhysicalDeviceQueueFamilyProperties2, vFppp)
GO(vkGetPhysicalDeviceSparseImageFormatProperties2, vFpppp)
GO(vkGetPhysicalDeviceToolProperties, iFppp)
GO(vkUpdateDescriptorSetWithTemplate, vFpppp)
GO(vkTrimCommandPool, vFppu)

// VK_VERSION_1_2
GO(vkResetQueryPool, vFppuu)
GO(vkCmdBeginRenderPass2, vFppp)
GO(vkCmdEndRenderPass2, vFpp)
GO(vkCmdNextSubpass2, vFppp)
GO(vkCmdDrawIndexedIndirectCount, vFppUpUuu)
GO(vkCmdDrawIndirectCount, vFppUpUuu)
GOM(vkCreateRenderPass2, iFEpppp)
GO(vkGetBufferDeviceAddress, UFpp)
GO(vkGetBufferOpaqueCaptureAddress, UFpp)
GO(vkGetDeviceMemoryOpaqueCaptureAddress, UFpp)
GO(vkGetSemaphoreCounterValue, iFppp)
GO(vkSignalSemaphore, iFpp)
GO(vkWaitSemaphores, iFppU)

// VK_VERSION_1_3
GO(vkCmdBeginRendering, vFpp)
GO(vkCmdEndRendering, vFp)
GO(vkCmdBlitImage2, vFpp)
GO(vkCmdCopyBuffer2, vFpp)
GO(vkCmdCopyBufferToImage2, vFpp)
GO(vkCmdCopyImage2, vFpp)
GO(vkCmdCopyImageToBuffer2, vFpp)
GO(vkCmdResolveImage2, vFpp)
GO(vkCmdSetDepthBiasEnable, vFpu)
GO(vkCmdSetLogicOp, vFpi) // Warning: failed to confirm
GO(vkCmdSetPatchControlPoints, vFpu) // Warning: failed to confirm
GO(vkCmdSetPrimitiveRestartEnable, vFpu)
GO(vkCmdSetRasterizerDiscardEnable, vFpu)
GOM(vkCreatePrivateDataSlot, iFEpppp)
GOM(vkDestroyPrivateDataSlot, vFEppp)
GO(vkGetPrivateData, vFpuUpp)
GO(vkSetPrivateData, iFpuUpU)
GO(vkGetDeviceBufferMemoryRequirements, vFppp)
GO(vkGetDeviceImageMemoryRequirements, vFppp)
GO(vkGetDeviceImageSparseMemoryRequirements, vFpppp)
GO(vkCmdPipelineBarrier2, vFpp)
GO(vkCmdResetEvent2, vFppU)
GO(vkCmdSetEvent2, vFppp)
GO(vkCmdWaitEvents2, vFpupp)
GO(vkCmdWriteTimestamp2, vFpUpu)
GO(vkQueueSubmit2, iFpupp)
GO(vkCmdBindVertexBuffers2, vFpuupppp)
GO(vkCmdSetCullMode, vFpu)
GO(vkCmdSetDepthBoundsTestEnable, vFpu)
GO(vkCmdSetDepthCompareOp, vFpu)
GO(vkCmdSetDepthTestEnable, vFpu)
GO(vkCmdSetDepthWriteEnable, vFpu)
GO(vkCmdSetFrontFace, vFpu)
GO(vkCmdSetPrimitiveTopology, vFpu)
GO(vkCmdSetScissorWithCount, vFpup)
GO(vkCmdSetStencilOp, vFpuuuuu)
GO(vkCmdSetStencilTestEnable, vFpu)
GO(vkCmdSetViewportWithCount, vFpup)

// VK_VERSION_1_4
GO(vkCmdBindDescriptorSets2, vFpp)
GO(vkCmdBindIndexBuffer2, vFppUUu)
GO(vkCmdPushConstants2, vFpp)
GO(vkCmdPushDescriptorSet, vFpupuup)
GO(vkCmdPushDescriptorSet2, vFpp)
GO(vkCmdPushDescriptorSetWithTemplate, vFpppup)
GO(vkCmdPushDescriptorSetWithTemplate2, vFpp)
GO(vkCmdSetLineStipple, vFpuW)
GO(vkCmdSetRenderingAttachmentLocations, vFpp)
GO(vkCmdSetRenderingInputAttachmentIndices, vFpp)
GO(vkCopyImageToImage, iFpp)
GO(vkCopyImageToMemory, iFpp)
GO(vkCopyMemoryToImage, iFpp)
GO(vkGetDeviceImageSubresourceLayout, vFppp)
GO(vkGetImageSubresourceLayout2, vFpppp)
GO(vkGetRenderingAreaGranularity, vFppp)
GO(vkMapMemory2, iFppp)
GO(vkTransitionImageLayout, iFpup)
GO(vkUnmapMemory2, iFpp)

// VK_EXT_debug_report
GOM(vkCreateDebugReportCallbackEXT, iFEpppp)
GO(vkDebugReportMessageEXT, vFpuuULipp)
GOM(vkDestroyDebugReportCallbackEXT, vFEppp)

//VK_EXT_debug_utils
GO(vkCmdBeginDebugUtilsLabelEXT, vFpp)
GO(vkCmdEndDebugUtilsLabelEXT, vFp)
GO(vkCmdInsertDebugUtilsLabelEXT, vFpp)
GOM(vkCreateDebugUtilsMessengerEXT, iFEpppp)
GOM(vkDestroyDebugUtilsMessengerEXT, vFEppp)
GO(vkQueueBeginDebugUtilsLabelEXT, vFpp)
GO(vkQueueEndDebugUtilsLabelEXT, vFp)
GO(vkQueueInsertDebugUtilsLabelEXT, vFpp)
GO(vkSetDebugUtilsObjectNameEXT, iFpp)
GO(vkSetDebugUtilsObjectTagEXT, iFpp)
GO(vkSubmitDebugUtilsMessageEXT, vFpuup)

// VK_KHR_external_memory_capabilities
GO(vkGetPhysicalDeviceExternalBufferPropertiesKHR, vFppp)

// VK_KHR_get_physical_device_properties2
GO(vkGetPhysicalDeviceFeatures2KHR, vFpp)
GO(vkGetPhysicalDeviceFormatProperties2KHR, vFpup)
GO(vkGetPhysicalDeviceImageFormatProperties2KHR, iFppp)
GO(vkGetPhysicalDeviceMemoryProperties2KHR, vFpp)
GO(vkGetPhysicalDeviceProperties2KHR, vFpp)
GO(vkGetPhysicalDeviceQueueFamilyProperties2KHR, vFppp)
GO(vkGetPhysicalDeviceSparseImageFormatProperties2KHR, vFpppp)

// VK_KHR_get_surface_capabilities2
GO(vkGetPhysicalDeviceSurfaceCapabilities2KHR, iFppp)
GO(vkGetPhysicalDeviceSurfaceFormats2KHR, iFpppp)

// VK_KHR_surface
GOM(vkDestroySurfaceKHR, vFEppp)
GO(vkGetPhysicalDeviceSurfaceCapabilitiesKHR, iFppp)
GO(vkGetPhysicalDeviceSurfaceFormatsKHR, iFpppp)
GO(vkGetPhysicalDeviceSurfacePresentModesKHR, iFpppp)
GO(vkGetPhysicalDeviceSurfaceSupportKHR, iFpupp)

// VK_KHR_xcb_surface
GOM(vkCreateXcbSurfaceKHR, iFEpppp) // Warning: failed to confirm
GO(vkGetPhysicalDeviceXcbPresentationSupportKHR, uFpubp) // Warning: failed to confirm

// VK_KHR_xlib_surface
GOM(vkCreateXlibSurfaceKHR, iFEpppp) // Warning: failed to confirm
GO(vkGetPhysicalDeviceXlibPresentationSupportKHR, uFpupp) // Warning: failed to confirm

// VK_KHR_android_surface
GOM(vkCreateAndroidSurfaceKHR, iFEpppp) // Warning: failed to confirm

// VK_KHR_swapchain
GO(vkAcquireNextImageKHR, iFppUppp)
GO(vkAcquireNextImage2KHR, iFppp)
GOM(vkCreateSwapchainKHR, iFEpppp)
GOM(vkDestroySwapchainKHR, vFEppp)
GO(vkGetDeviceGroupPresentCapabilitiesKHR, iFpp)
GO(vkGetDeviceGroupSurfacePresentModesKHR, iFppp)
GO(vkGetPhysicalDevicePresentRectanglesKHR, iFpppp)
GO(vkGetSwapchainImagesKHR, iFpppp)
GO(vkQueuePresentKHR, iFpp)

// VK_KHR_bind_memory2
GO(vkBindBufferMemory2KHR, iFpup)
GO(vkBindImageMemory2KHR, iFpup)

// VK_KHR_display
GOM(vkCreateDisplayModeKHR, iFEppppp)
GOM(vkCreateDisplayPlaneSurfaceKHR, iFEpppp)
GO(vkGetDisplayModePropertiesKHR, iFpppp)
GO(vkGetDisplayPlaneCapabilitiesKHR, iFppup)
GO(vkGetDisplayPlaneSupportedDisplaysKHR, iFpupp)
GO(vkGetPhysicalDeviceDisplayPlanePropertiesKHR, iFppp)
GO(vkGetPhysicalDeviceDisplayPropertiesKHR, iFppp)

// VK_KHR_descriptor_update_template
GOM(vkCreateDescriptorUpdateTemplateKHR, iFEpppp)
GOM(vkDestroyDescriptorUpdateTemplateKHR, vFEppp)
GO(vkUpdateDescriptorSetWithTemplateKHR, vFpppp)
GO(vkCmdPushDescriptorSetWithTemplateKHR, vFpppup)

// VK_EXT_display_surface_counter
GO(vkGetPhysicalDeviceSurfaceCapabilities2EXT, iFppp)

// VK_KHR_get_display_properties2
GO(vkGetDisplayModeProperties2KHR, iFpppp)
GO(vkGetDisplayPlaneCapabilities2KHR, iFppp)
GO(vkGetPhysicalDeviceDisplayPlaneProperties2KHR, iFppp)
GO(vkGetPhysicalDeviceDisplayProperties2KHR, iFppp)

// VK_KHR_device_group
GO(vkCmdDispatchBaseKHR, vFpuuuuuu)
GO(vkCmdSetDeviceMaskKHR, vFpu)
GO(vkGetDeviceGroupPeerMemoryFeaturesKHR, vFpuuup)

// VK_KHR_sampler_ycbcr_conversion
GOM(vkCreateSamplerYcbcrConversionKHR, iFEpppp)
GOM(vkDestroySamplerYcbcrConversionKHR, vFEppp)

// VK_KHR_display_swapchain
GOM(vkCreateSharedSwapchainsKHR, iFEpuppp)

// VK_KHR_wayland_surface
GOM(vkCreateWaylandSurfaceKHR, iFEpppp) // Warning: failed to confirm
GO(vkGetPhysicalDeviceWaylandPresentationSupportKHR, uFpup) // Warning: failed to confirm

// VK_KHR_device_group_creation
GO(vkEnumeratePhysicalDeviceGroupsKHR, iFppp)

// VK_KHR_get_memory_requirements2
GO(vkGetBufferMemoryRequirements2KHR, vFppp)
GO(vkGetImageMemoryRequirements2KHR, vFppp)
GO(vkGetImageSparseMemoryRequirements2KHR, vFpppp)

// VK_KHR_external_fence_capabilities
GO(vkGetPhysicalDeviceExternalFencePropertiesKHR, vFppp)

// VK_KHR_external_semaphore_capabilities
GO(vkGetPhysicalDeviceExternalSemaphorePropertiesKHR, vFppp)

// VK_KHR_maintenance1
GO(vkTrimCommandPoolKHR, vFppu)

// VK_KHR_maintenance2
// no functions

// VK_KHR_maintenance3
GO(vkGetDescriptorSetLayoutSupportKHR, vFppp)

// VK_KHR_external_memory_fd
GO(vkGetMemoryFdKHR, iFppp)
GO(vkGetMemoryFdPropertiesKHR, iFpuip)

// VK_KHR_dedicated_allocation
// no functions

// VK_KHR_image_format_list
// no functions

// VK_KHR_shader_draw_parameters
// no functions

// VK_EXT_conditional_rendering
GO(vkCmdBeginConditionalRenderingEXT, vFpp)
GO(vkCmdEndConditionalRenderingEXT, vFp)

// VK_EXT_depth_clip_enable
// no functions

// VK_EXT_host_query_reset
GO(vkResetQueryPoolEXT, vFppuu)

// VK_EXT_memory_priority
// no functions

// VK_EXT_shader_demote_to_helper_invocation
// no functions

// VK_EXT_transform_feedback
GO(vkCmdBeginQueryIndexedEXT, vFppuuu)
GO(vkCmdBeginTransformFeedbackEXT, vFpuupp)
GO(vkCmdBindTransformFeedbackBuffersEXT, vFpuuppp)
GO(vkCmdDrawIndirectByteCountEXT, vFpuupUuu)
GO(vkCmdEndQueryIndexedEXT, vFppuu)
GO(vkCmdEndTransformFeedbackEXT, vFpuupp)

// VK_EXT_vertex_attribute_divisor
// no functions

// VK_EXT_full_screen_exclusive
GO(vkAcquireFullScreenExclusiveModeEXT, iFpU) // Warning: failed to confirm
GO(vkGetPhysicalDeviceSurfacePresentModes2EXT, iFpppp) // Warning: failed to confirm
GO(vkReleaseFullScreenExclusiveModeEXT, iFpU) // Warning: failed to confirm
GO(vkGetDeviceGroupSurfacePresentModes2EXT, iFppp) // Warning: failed to confirm

// VK_EXT_calibrated_timestamps
GO(vkGetCalibratedTimestampsEXT, iFpuppp)
GO(vkGetPhysicalDeviceCalibrateableTimeDomainsEXT, iFppp)

// VK_EXT_sample_locations
GO(vkCmdSetSampleLocationsEXT, vFpp)
GO(vkGetPhysicalDeviceMultisamplePropertiesEXT, vFpup)

// VK_EXT_headless_surface
GOM(vkCreateHeadlessSurfaceEXT, iFEpppp)

//VK_KHR_performance_query
GO(vkAcquireProfilingLockKHR, iFpp)
GO(vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR, iFpuppp)
GO(vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR, vFppp)
GO(vkReleaseProfilingLockKHR, vFp)

// VK_NV_cooperative_matrix
GO(vkGetPhysicalDeviceCooperativeMatrixPropertiesNV, iFppp)

// VK_NV_cooperative_vector
GO(vkCmdConvertCooperativeVectorMatrixNV, vFpup)
GO(vkConvertCooperativeVectorMatrixNV, iFpp)
GO(vkGetPhysicalDeviceCooperativeVectorPropertiesNV, iFppp)

// VK_KHR_fragment_shading_rate
GO(vkCmdSetFragmentShadingRateKHR, vFppp)
GO(vkGetPhysicalDeviceFragmentShadingRatesKHR, iFppp)

// VK_NV_coverage_reduction_mode
GO(vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV, iFppp)

// VK_EXT_tooling_info
GO(vkGetPhysicalDeviceToolPropertiesEXT, iFppp)

// VK_KHR_create_renderpass2
GO(vkCmdBeginRenderPass2KHR, vFppp)
GO(vkCmdEndRenderPass2KHR, vFpp)
GO(vkCmdNextSubpass2KHR, vFppp)
GOM(vkCreateRenderPass2KHR, iFEpppp)

// VK_EXT_extended_dynamic_state
GO(vkCmdBindVertexBuffers2EXT, vFpuupppp)
GO(vkCmdSetCullModeEXT, vFpu)
GO(vkCmdSetDepthBoundsTestEnableEXT, vFpu)
GO(vkCmdSetDepthCompareOpEXT, vFpu)
GO(vkCmdSetDepthTestEnableEXT, vFpu)
GO(vkCmdSetDepthWriteEnableEXT, vFpu)
GO(vkCmdSetFrontFaceEXT, vFpu)
GO(vkCmdSetPrimitiveTopologyEXT, vFpu)
GO(vkCmdSetScissorWithCountEXT, vFpup)
GO(vkCmdSetStencilOpEXT, vFpuuuuu)
GO(vkCmdSetStencilTestEnableEXT, vFpu)
GO(vkCmdSetViewportWithCountEXT, vFpup)

// VK_KHR_draw_indirect_count
GO(vkCmdDrawIndexedIndirectCountKHR, vFppUpUuu)
GO(vkCmdDrawIndirectCountKHR, vFppUpUuu)

// VK_AMD_draw_indirect_count
GO(vkCmdDrawIndexedIndirectCountAMD, vFppUpUuu)
GO(vkCmdDrawIndirectCountAMD, vFppUpUuu)

// VK_AMD_buffer_marker
GO(vkCmdWriteBufferMarkerAMD, vFpupUu)

// VK_AMD_shader_info
GO(vkGetShaderInfoAMD, iFppuupp)

// VK_EXT_debug_marker
GO(vkCmdDebugMarkerBeginEXT, vFpp)
GO(vkCmdDebugMarkerEndEXT, vFp)
GO(vkCmdDebugMarkerInsertEXT, vFpp)
GO(vkDebugMarkerSetObjectNameEXT, iFpp)
GO(vkDebugMarkerSetObjectTagEXT, iFpp)

// VK_EXT_discard_rectangles
GO(vkCmdSetDiscardRectangleEXT, vFpuup)
GO(vkCmdSetDiscardRectangleEnableEXT, vFpu)
GO(vkCmdSetDiscardRectangleModeEXT, vFpu)

// VK_EXT_display_control
GO(vkDisplayPowerControlEXT, iFppp)
GO(vkGetSwapchainCounterEXT, iFppup)
GOM(vkRegisterDeviceEventEXT, iFEpppp)
GOM(vkRegisterDisplayEventEXT, iFEppppp)

// VK_EXT_external_memory_host
GO(vkGetMemoryHostPointerPropertiesEXT, iFpupp)

// VK_EXT_hdr_metadata
GO(vkSetHdrMetadataEXT, vFpupp)

// VK_EXT_validation_cache
GOM(vkCreateValidationCacheEXT, iFEpppp)
GOM(vkDestroyValidationCacheEXT, vFEppp)
GO(vkGetValidationCacheDataEXT, iFpppp)
GO(vkMergeValidationCachesEXT, iFppup)

// VK_GOOGLE_display_timing
GO(vkGetPastPresentationTimingGOOGLE, iFpppp)
GO(vkGetRefreshCycleDurationGOOGLE, iFppp)

// VK_KHR_external_fence_fd
GO(vkGetFenceFdKHR, iFppp)
GO(vkImportFenceFdKHR, iFpp)

// VK_KHR_external_semaphore_fd
GO(vkGetSemaphoreFdKHR, iFppp)
GO(vkImportSemaphoreFdKHR, iFpp)

// VK_KHR_push_descriptor
GO(vkCmdPushDescriptorSetKHR, vFpupuup)

// VK_KHR_shared_presentable_image
GO(vkGetSwapchainStatusKHR, iFpp)

// VK_NV_clip_space_w_scaling
GO(vkCmdSetViewportWScalingNV, vFpuup)

// VK_NV_device_diagnostic_checkpoints
GO(vkCmdSetCheckpointNV, vFpp)
GO(vkGetQueueCheckpointDataNV, vFppp)

// VK_NV_mesh_shader
GO(vkCmdDrawMeshTasksIndirectCountNV, vFppUpUuu)
GO(vkCmdDrawMeshTasksIndirectNV, vFppUuu)
GO(vkCmdDrawMeshTasksNV, vFpuu)

// VK_NV_scissor_exclusive
GO(vkCmdSetExclusiveScissorEnableNV, vFpuup)
GO(vkCmdSetExclusiveScissorNV, vFpuup)

// VK_NV_shading_rate_image
GO(vkCmdBindShadingRateImageNV, vFppu)
GO(vkCmdSetCoarseSampleOrderNV, vFpuup)
GO(vkCmdSetViewportShadingRatePaletteNV, vFpuup)

// VK_KHR_video_queue
GO(vkBindVideoSessionMemoryKHR, iFppup)
GO(vkCmdBeginVideoCodingKHR, vFpp)
GO(vkCmdControlVideoCodingKHR, vFpp)
GO(vkCmdEndVideoCodingKHR, vFpp)
GOM(vkCreateVideoSessionKHR, iFEpppp)
GOM(vkCreateVideoSessionParametersKHR, iFEpppp)
GOM(vkDestroyVideoSessionKHR, vFEppp)
GOM(vkDestroyVideoSessionParametersKHR, vFEppp)
GO(vkGetPhysicalDeviceVideoCapabilitiesKHR, iFppp)
GO(vkGetPhysicalDeviceVideoFormatPropertiesKHR, iFpppp)
GO(vkGetVideoSessionMemoryRequirementsKHR, iFpppp)
GO(vkUpdateVideoSessionParametersKHR, iFppp)

// VK_KHR_buffer_device_address
GO(vkGetBufferDeviceAddressKHR, UFpp)
GO(vkGetBufferOpaqueCaptureAddressKHR, UFpp)
GO(vkGetDeviceMemoryOpaqueCaptureAddressKHR, UFpp)

// VK_EXT_buffer_device_address
GO(vkGetBufferDeviceAddressEXT, UFpp)

// VK_KHR_timeline_semaphore
GO(vkGetSemaphoreCounterValueKHR, iFppp)
GO(vkSignalSemaphoreKHR, iFpp)
GO(vkWaitSemaphoresKHR, iFppU)

// VK_AMD_display_native_hdr
GO(vkSetLocalDimmingAMD, vFppu)

// VK_EXT_color_write_enable
GO(vkCmdSetColorWriteEnableEXT, vFpup)

// VK_EXT_extended_dynamic_state2
GO(vkCmdSetDepthBiasEnableEXT, vFpu)
GO(vkCmdSetLogicOpEXT, vFpu)
GO(vkCmdSetPatchControlPointsEXT, vFpu)
GO(vkCmdSetPrimitiveRestartEnableEXT, vFpu)
GO(vkCmdSetRasterizerDiscardEnableEXT, vFpu)

// VK_EXT_image_drm_format_modifier
GO(vkGetImageDrmFormatModifierPropertiesEXT, iFppp)

// VK_EXT_line_rasterization
GO(vkCmdSetLineStippleEXT, vFpuW)

// VK_EXT_multi_draw
GO(vkCmdDrawMultiEXT, vFpupuuu)
GO(vkCmdDrawMultiIndexedEXT, vFpupuuup)

// VK_EXT_pageable_device_local_memory
GO(vkSetDeviceMemoryPriorityEXT, vFppf)

// VK_EXT_private_data
GOM(vkCreatePrivateDataSlotEXT, iFEpppp)
GOM(vkDestroyPrivateDataSlotEXT, vFEppp)
GO(vkGetPrivateDataEXT, vFpuUpp)
GO(vkSetPrivateDataEXT, iFpuUpU)

// VK_EXT_vertex_input_dynamic_state
GO(vkCmdSetVertexInputEXT, vFpupup)

// VK_HUAWEI_invocation_mask
GO(vkCmdBindInvocationMaskHUAWEI, vFppu)

// VK_HUAWEI_subpass_shading
GO(vkCmdSubpassShadingHUAWEI, vFp)
GO(vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI, iFppp)

// VK_INTEL_performance_query
GO(vkAcquirePerformanceConfigurationINTEL, iFppp)
GO(vkCmdSetPerformanceMarkerINTEL, iFpp)
GO(vkCmdSetPerformanceOverrideINTEL, iFpp)
GO(vkCmdSetPerformanceStreamMarkerINTEL, iFpp)
GO(vkGetPerformanceParameterINTEL, iFpup)
GO(vkInitializePerformanceApiINTEL, iFpp)
GO(vkQueueSetPerformanceConfigurationINTEL, iFpp)
GO(vkReleasePerformanceConfigurationINTEL, iFpp)
GO(vkUninitializePerformanceApiINTEL, vFp)

// VK_KHR_acceleration_structure
GO(vkBuildAccelerationStructuresKHR, iFppupp)
GO(vkCmdBuildAccelerationStructuresIndirectKHR, vFpupppp)
GO(vkCmdBuildAccelerationStructuresKHR, vFpupp)
GO(vkCmdCopyAccelerationStructureKHR, vFpp)
GO(vkCmdCopyAccelerationStructureToMemoryKHR, vFpp)
GO(vkCmdCopyMemoryToAccelerationStructureKHR, vFpp)
GO(vkCmdWriteAccelerationStructuresPropertiesKHR, vFpupupu)
GO(vkCopyAccelerationStructureKHR, iFppp)
GO(vkCopyAccelerationStructureToMemoryKHR, iFppp)
GO(vkCopyMemoryToAccelerationStructureKHR, iFppp)
GOM(vkCreateAccelerationStructureKHR, iFEpppp)
GOM(vkDestroyAccelerationStructureKHR, vFEppp)
GO(vkGetAccelerationStructureBuildSizesKHR, vFpuppp)
GO(vkGetAccelerationStructureDeviceAddressKHR, UFpp)
GO(vkGetDeviceAccelerationStructureCompatibilityKHR, vFppp)
GO(vkWriteAccelerationStructuresPropertiesKHR, iFpupuLpL)

// VK_KHR_copy_commands2
GO(vkCmdBlitImage2KHR, vFpp)
GO(vkCmdCopyBuffer2KHR, vFpp)
GO(vkCmdCopyBufferToImage2KHR, vFpp)
GO(vkCmdCopyImage2KHR, vFpp)
GO(vkCmdCopyImageToBuffer2KHR, vFpp)
GO(vkCmdResolveImage2KHR, vFpp)

// VK_KHR_deferred_host_operations
GOM(vkCreateDeferredOperationKHR, iFEppp)
GO(vkDeferredOperationJoinKHR, iFpp)
GOM(vkDestroyDeferredOperationKHR, vFEppp)
GO(vkGetDeferredOperationMaxConcurrencyKHR, uFpp)
GO(vkGetDeferredOperationResultKHR, iFpp)

// VK_KHR_dynamic_rendering
GO(vkCmdBeginRenderingKHR, vFpp)
GO(vkCmdEndRenderingKHR, vFp)

// VK_KHR_maintenance4
GO(vkGetDeviceBufferMemoryRequirementsKHR, vFppp)
GO(vkGetDeviceImageMemoryRequirementsKHR, vFppp)
GO(vkGetDeviceImageSparseMemoryRequirementsKHR, vFpppp)

// VK_KHR_pipeline_executable_properties
GO(vkGetPipelineExecutableInternalRepresentationsKHR, iFpppp)
GO(vkGetPipelineExecutablePropertiesKHR, iFpppp)
GO(vkGetPipelineExecutableStatisticsKHR, iFpppp)

// VK_KHR_present_wait
GO(vkWaitForPresentKHR, iFppUU)

// VK_KHR_present_wait2
GO(vkWaitForPresent2KHR, iFppp)

// VK_KHR_ray_tracing_pipeline
GO(vkCmdSetRayTracingPipelineStackSizeKHR, vFpu)
GO(vkCmdTraceRaysIndirectKHR, vFpppppU)
GO(vkCmdTraceRaysKHR, vFpppppuuu)
GOM(vkCreateRayTracingPipelinesKHR, iFEpppuppp)
GO(vkGetRayTracingCaptureReplayShaderGroupHandlesKHR, iFppuuLp)
GO(vkGetRayTracingShaderGroupHandlesKHR, iFppuuLp)
GO(vkGetRayTracingShaderGroupStackSizeKHR, UFppuu)

// VK_KHR_synchronization2
GO(vkCmdPipelineBarrier2KHR, vFpp)
GO(vkCmdResetEvent2KHR, vFppU)
GO(vkCmdSetEvent2KHR, vFppp)
GO(vkCmdWaitEvents2KHR, vFpupp)
GO(vkCmdWriteTimestamp2KHR, vFpUpu)
GO(vkQueueSubmit2KHR, iFpupp)
GO(vkCmdWriteBufferMarker2AMD, vFpUpUu)
GO(vkGetQueueCheckpointData2NV, vFppp)

// VK_KHR_video_decode_queue
GO(vkCmdDecodeVideoKHR, vFpp)

// VK_KHR_video_encode_queue
GO(vkCmdEncodeVideoKHR, vFpp)
GO(vkGetEncodedVideoSessionParametersKHR, iFppppp)
GO(vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR, iFppp)

// VK_NVX_binary_import
GO(vkCmdCuLaunchKernelNVX, vFpp)
GOM(vkCreateCuFunctionNVX, iFEpppp)
GOM(vkCreateCuModuleNVX, iFEpppp)
GOM(vkDestroyCuFunctionNVX, vFEppp)
GOM(vkDestroyCuModuleNVX, vFEppp)

// VK_NVX_image_view_handle
GO(vkGetImageViewAddressNVX, iFppp)
GO(vkGetImageViewHandle64NVX, UFpp)
GO(vkGetImageViewHandleNVX, uFpp)

// VK_NV_device_generated_commands
GO(vkCmdBindPipelineShaderGroupNV, vFpupu)
GO(vkCmdExecuteGeneratedCommandsNV, vFpup)
GO(vkCmdPreprocessGeneratedCommandsNV, vFpp)
GOM(vkCreateIndirectCommandsLayoutNV, iFEpppp)
GOM(vkDestroyIndirectCommandsLayoutNV, vFEppp)
GO(vkGetGeneratedCommandsMemoryRequirementsNV, vFppp)

// VK_NV_external_memory_rdma
GO(vkGetMemoryRemoteAddressNV, iFppp)

// VK_NV_fragment_shading_rate_enums
GO(vkCmdSetFragmentShadingRateEnumNV, vFpup)

// VK_NV_ray_tracing
GO(vkBindAccelerationStructureMemoryNV, iFpup)
GO(vkCmdBuildAccelerationStructureNV, vFpppUupppU)
GO(vkCmdCopyAccelerationStructureNV, vFpppu)
GO(vkCmdTraceRaysNV, vFppUpUUpUUpUUuuu)
GO(vkCmdWriteAccelerationStructuresPropertiesNV, vFpupupu)
GO(vkCompileDeferredNV, iFppu)
GOM(vkCreateAccelerationStructureNV, iFEpppp)
GOM(vkCreateRayTracingPipelinesNV, iFEppuppp)
GOM(vkDestroyAccelerationStructureNV, vFEppp)
GO(vkGetAccelerationStructureHandleNV, iFppLp)
GO(vkGetAccelerationStructureMemoryRequirementsNV, vFppp)
GO(vkGetRayTracingShaderGroupHandlesNV, iFppuuLp)

// VK_EXT_acquire_xlib_display
GO(vkAcquireXlibDisplayEXT, iFppU) // Warning: failed to confirm
GO(vkGetRandROutputDisplayEXT, iFpppp) // Warning: failed to confirm

// VK_EXT_shader_module_identifier
GO(vkGetShaderModuleCreateInfoIdentifierEXT, vFppp)
GO(vkGetShaderModuleIdentifierEXT, vFppp)

// VK_NV_optical_flow
GO(vkBindOpticalFlowSessionImageNV, iFppupu)
GO(vkCmdOpticalFlowExecuteNV, vFppp)
GOM(vkCreateOpticalFlowSessionNV, iFEpppp)
GOM(vkDestroyOpticalFlowSessionNV, vFEppp)
GO(vkGetPhysicalDeviceOpticalFlowImageFormatsNV, iFpppp)

// VK_EXT_extended_dynamic_state3
GO(vkCmdSetAlphaToCoverageEnableEXT, vFpu)
GO(vkCmdSetAlphaToOneEnableEXT, vFpu)
GO(vkCmdSetColorBlendAdvancedEXT, vFpuup)
GO(vkCmdSetColorBlendEnableEXT, vFpuup)
GO(vkCmdSetColorBlendEquationEXT, vFpuup)
GO(vkCmdSetColorWriteMaskEXT, vFpuup)
GO(vkCmdSetConservativeRasterizationModeEXT, vFpu)
GO(vkCmdSetCoverageModulationModeNV, vFpu)
GO(vkCmdSetCoverageModulationTableEnableNV, vFpu)
GO(vkCmdSetCoverageModulationTableNV, vFpup)
GO(vkCmdSetCoverageReductionModeNV, vFpu)
GO(vkCmdSetCoverageToColorEnableNV, vFpu)
GO(vkCmdSetCoverageToColorLocationNV, vFpu)
GO(vkCmdSetDepthClampEnableEXT, vFpu)
GO(vkCmdSetDepthClipEnableEXT, vFpu)
GO(vkCmdSetDepthClipNegativeOneToOneEXT, vFpu)
GO(vkCmdSetExtraPrimitiveOverestimationSizeEXT, vFpf)
GO(vkCmdSetLineRasterizationModeEXT, vFpu)
GO(vkCmdSetLineStippleEnableEXT, vFpu)
GO(vkCmdSetLogicOpEnableEXT, vFpu)
GO(vkCmdSetPolygonModeEXT, vFpu)
GO(vkCmdSetProvokingVertexModeEXT, vFpu)
GO(vkCmdSetRasterizationSamplesEXT, vFpu)
GO(vkCmdSetRasterizationStreamEXT, vFpu)
GO(vkCmdSetRepresentativeFragmentTestEnableNV, vFpu)
GO(vkCmdSetSampleLocationsEnableEXT, vFpu)
GO(vkCmdSetSampleMaskEXT, vFpup)
GO(vkCmdSetShadingRateImageEnableNV, vFpu)
GO(vkCmdSetTessellationDomainOriginEXT, vFpu)
GO(vkCmdSetViewportSwizzleNV, vFpuup)
GO(vkCmdSetViewportWScalingEnableNV, vFpu)

// VK_KHR_external_memory_win32
GO(vkGetMemoryWin32HandleKHR, iFppp) // Warning: failed to confirm
GO(vkGetMemoryWin32HandlePropertiesKHR, iFpipp) // Warning: failed to confirm

// VK_EXT_swapchain_maintenance1
GO(vkReleaseSwapchainImagesEXT, iFpp)

// VK_VALVE_descriptor_set_host_mapping
GO(vkGetDescriptorSetHostMappingVALVE, vFppp)
GO(vkGetDescriptorSetLayoutHostMappingInfoVALVE, vFppp)

// VK_EXT_descriptor_buffer
GO(vkCmdBindDescriptorBufferEmbeddedSamplersEXT, vFpupu)
GO(vkCmdBindDescriptorBuffersEXT, vFpup)
GO(vkCmdSetDescriptorBufferOffsetsEXT, vFpupuupp)
GO(vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT, iFppp)
GO(vkGetBufferOpaqueCaptureDescriptorDataEXT, iFppp)
GO(vkGetDescriptorEXT, vFppLp)
GO(vkGetDescriptorSetLayoutBindingOffsetEXT, vFppup)
GO(vkGetDescriptorSetLayoutSizeEXT, vFppp)
GO(vkGetImageOpaqueCaptureDescriptorDataEXT, iFppp)
GO(vkGetImageViewOpaqueCaptureDescriptorDataEXT, iFppp)
GO(vkGetSamplerOpaqueCaptureDescriptorDataEXT, iFppp)

// VK_KHR_cooperative_matrix
GO(vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR, iFppp)

// VK_KHR_maintenance5
GO(vkCmdBindIndexBuffer2KHR, vFppUUu)
GO(vkGetDeviceImageSubresourceLayoutKHR, vFppp)
GO(vkGetImageSubresourceLayout2KHR, vFpppp)
GO(vkGetRenderingAreaGranularityKHR, vFppp)

// VK_NV_memory_decompression
GO(vkCmdDecompressMemoryIndirectCountNV, vFpUUu)
GO(vkCmdDecompressMemoryNV, vFpup)

// VK_EXT_merge_shader
GO(vkCmdDrawMeshTasksEXT, vFpuuu)
GO(vkCmdDrawMeshTasksIndirectCountEXT, vFppUpUuu)
GO(vkCmdDrawMeshTasksIndirectEXT, vFppUuu)

// VK_EXT_opacity_micromap
GO(vkBuildMicromapsEXT, iFppup)
GO(vkCmdBuildMicromapsEXT, vFpup)
GO(vkCmdCopyMemoryToMicromapEXT, vFpp)
GO(vkCmdCopyMicromapEXT, vFpp)
GO(vkCmdCopyMicromapToMemoryEXT, vFpp)
GO(vkCmdWriteMicromapsPropertiesEXT, vFpupupu)
GO(vkCopyMemoryToMicromapEXT, iFppp)
GO(vkCopyMicromapEXT, iFppp)
GO(vkCopyMicromapToMemoryEXT, iFppp)
GOM(vkCreateMicromapEXT, iFEpppp)
GOM(vkDestroyMicromapEXT, vFEppp)
GO(vkGetDeviceMicromapCompatibilityEXT, vFppp)
GO(vkGetMicromapBuildSizesEXT, vFpupp)
GO(vkWriteMicromapsPropertiesEXT, iFpupuLpL)

// VK_KHR_ray_tracing_maintenance1
GO(vkCmdTraceRaysIndirect2KHR, vFpU)

// VK_EXT_depth_bias_control
GO(vkCmdSetDepthBias2EXT, vFpp)

// VK_EXT_host_image_copy
GO(vkCopyImageToImageEXT, iFpp)
GO(vkCopyImageToMemoryEXT, iFpp)
GO(vkCopyMemoryToImageEXT, iFpp)
GO(vkGetImageSubresourceLayout2EXT, vFpppp)
GO(vkTransitionImageLayoutEXT, iFpup)

// VK_KHR_map_memory2
GO(vkMapMemory2KHR, iFppp)
GO(vkUnmapMemory2KHR, iFpp)

// VK_EXT_device_fault
GO(vkGetDeviceFaultInfoEXT, iFppp)

// VK_EXT_acquire_drm_display
GO(vkAcquireDrmDisplayEXT, iFpip)
GO(vkGetDrmDisplayEXT, iFpiup)

// VK_EXT_shader_object
GO(vkCmdBindShadersEXT, vFpupp)
GOM(vkCreateShadersEXT, iFEpuppp)
GOM(vkDestroyShaderEXT, vFEppp)
GO(vkGetShaderBinaryDataEXT, iFpppp)

// VK_NV_copy_memory_indirect
GO(vkCmdCopyMemoryIndirectNV, vFpUuu)
GO(vkCmdCopyMemoryToImageIndirectNV, vFpUuupup)

// VK_HUAWEI_cluster_culling_shader
GO(vkCmdDrawClusterHUAWEI, vFpuuu)
GO(vkCmdDrawClusterIndirectHUAWEI, vFppU)

// VK_EXT_attachment_feedback_loop_dynamic_state
GO(vkCmdSetAttachmentFeedbackLoopEnableEXT, vFpu)

// VK_NV_device_generated_commands_compute
GO(vkCmdUpdatePipelineIndirectBufferNV, vFpup)
GO(vkGetPipelineIndirectDeviceAddressNV, UFpp)
GO(vkGetPipelineIndirectMemoryRequirementsNV, vFppp)

// VK_QCOM_tile_properties
GO(vkGetDynamicRenderingTilePropertiesQCOM, iFppp)
GO(vkGetFramebufferTilePropertiesQCOM, iFpppp)

// VK_NV_external_memory_capabilities
GO(vkGetPhysicalDeviceExternalImageFormatPropertiesNV, iFpuuuuuup)

// VK_EXT_pipeline_properties
GO(vkGetPipelinePropertiesEXT, iFppp)

// VK_EXT_direct_mode_display
GO(vkReleaseDisplayEXT, iFpp)

// VK_KHR_calibrated_timestamps
GO(vkGetCalibratedTimestampsKHR, iFpuppp)
GO(vkGetPhysicalDeviceCalibrateableTimeDomainsKHR, iFppp)

// VK_KHR_video_encode_h264
// no functions

// VK_KHR_video_encode_h265
// no functions

// VK_KHR_video_maintenance1
// no functions

// VK_NV_cuda_kernel_launch
GO(vkCmdCudaLaunchKernelNV, vFpp) // Warning: failed to confirm
GOM(vkCreateCudaFunctionNV, iFEpppp) // Warning: failed to confirm
GOM(vkCreateCudaModuleNV, iFEpppp) // Warning: failed to confirm
GOM(vkDestroyCudaFunctionNV, vFEpUp) // Warning: failed to confirm
GOM(vkDestroyCudaModuleNV, vFEpUp) // Warning: failed to confirm
GO(vkGetCudaModuleCacheNV, iFpUpp) // Warning: failed to confirm

// VK_NV_low_latency2
GO(vkGetLatencyTimingsNV, vFppp)
GO(vkLatencySleepNV, iFppp)
GO(vkQueueNotifyOutOfBandNV, vFpp)
GO(vkSetLatencyMarkerNV, vFppp)
GO(vkSetLatencySleepModeNV, iFppp)

// VK_AMDX_shader_enqueue
GO(vkCmdDispatchGraphAMDX, vFpUp) // Warning: failed to confirm
GO(vkCmdDispatchGraphIndirectAMDX, vFpUp) // Warning: failed to confirm
GO(vkCmdDispatchGraphIndirectCountAMDX, vFpUU) // Warning: failed to confirm
GO(vkCmdInitializeGraphScratchMemoryAMDX, vFpU) // Warning: failed to confirm
GOM(vkCreateExecutionGraphPipelinesAMDX, iFEpUuppp) // Warning: failed to confirm
GO(vkGetExecutionGraphPipelineNodeIndexAMDX, iFpUpp) // Warning: failed to confirm
GO(vkGetExecutionGraphPipelineScratchSizeAMDX, iFpUp) // Warning: failed to confirm

// VK_KHR_maintenance6
GO(vkCmdBindDescriptorSets2KHR, vFpp)
GO(vkCmdPushConstants2KHR, vFpp)
GO(vkCmdBindDescriptorBufferEmbeddedSamplers2EXT, vFpp)
GO(vkCmdSetDescriptorBufferOffsets2EXT, vFpp)
GO(vkCmdPushDescriptorSet2KHR, vFpp)
GO(vkCmdPushDescriptorSetWithTemplate2KHR, vFpp)

// VK_KHR_dynamic_rendering_local_read
GO(vkCmdSetRenderingAttachmentLocationsKHR, vFpp)
GO(vkCmdSetRenderingInputAttachmentIndicesKHR, vFpp)

// VK_KHR_line_rasterization
GO(vkCmdSetLineStippleKHR, vFpuW)

// VK_KHR_external_semaphore_win32
GO(vkGetSemaphoreWin32HandleKHR, iFppp) // Warning: failed to confirm
GO(vkImportSemaphoreWin32HandleKHR, iFpp) // Warning: failed to confirm

// VK_KHR_win32_keyed_mutex
GO(wine_vkAcquireKeyedMutex, iFpUUu) // Warning: failed to confirm
GO(wine_vkReleaseKeyedMutex, iFpUU) // Warning: failed to confirm

// VK_NV_cooperative_matrix2
GO(vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV, iFppp)

// VK_AMD_anti_lag
GO(vkAntiLagUpdateAMD, vFpp)

// VK_EXT_device_generated_commands
GO(vkCmdExecuteGeneratedCommandsEXT, vFpup)
GO(vkCmdPreprocessGeneratedCommandsEXT, vFppp)
GOM(vkCreateIndirectCommandsLayoutEXT, iFEpppp)
GOM(vkCreateIndirectExecutionSetEXT, iFEpppp)
GOM(vkDestroyIndirectCommandsLayoutEXT, vFEppp)
GOM(vkDestroyIndirectExecutionSetEXT, vFEppp)
GO(vkGetGeneratedCommandsMemoryRequirementsEXT, vFppp)
GO(vkUpdateIndirectExecutionSetPipelineEXT, vFppup)
GO(vkUpdateIndirectExecutionSetShaderEXT, vFppup)

// VK_KHR_pipeline_binary
GOM(vkCreatePipelineBinariesKHR, iFEpppp)
GOM(vkDestroyPipelineBinaryKHR, vFEppp)
GO(vkGetPipelineBinaryDataKHR, iFppppp)
GO(vkGetPipelineKeyKHR, iFppp)
GOM(vkReleaseCapturedPipelineDataKHR, iFEppp)

// VK_EXT_depth_clamp_control
GO(vkCmdSetDepthClampRangeEXT, vFpup)

// VK_ARM_tensors
GO(vkBindTensorMemoryARM, iFpup)
GO(vkCmdCopyTensorARM, vFpp)
GOM(vkCreateTensorARM, iFEpppp)
GOM(vkCreateTensorViewARM, iFEpppp)
GOM(vkDestroyTensorARM, vFEppp)
GOM(vkDestroyTensorViewARM, vFEppp)
GO(vkGetDeviceTensorMemoryRequirementsARM, vFppp)
GO(vkGetPhysicalDeviceExternalTensorPropertiesARM, vFppp)
GO(vkGetTensorMemoryRequirementsARM, vFppp)

// VK_EXT_descriptor_buffer + VK_ARM_tensors
GO(vkGetTensorOpaqueCaptureDescriptorDataARM, iFppp)
GO(vkGetTensorViewOpaqueCaptureDescriptorDataARM, iFppp)

// VK_ARM_data_graph
GO(vkBindDataGraphPipelineSessionMemoryARM, iFpup)
GO(vkCmdDispatchDataGraphARM, vFppp)
GOM(vkCreateDataGraphPipelineSessionARM, iFEpppp)
GOM(vkCreateDataGraphPipelinesARM, iFEpppuppp)
GOM(vkDestroyDataGraphPipelineSessionARM, vFEppp)
GO(vkGetDataGraphPipelineAvailablePropertiesARM, iFpppp)
GO(vkGetDataGraphPipelinePropertiesARM, iFppup)
GO(vkGetDataGraphPipelineSessionBindPointRequirementsARM, iFpppp)
GO(vkGetDataGraphPipelineSessionMemoryRequirementsARM, vFppp)
GO(vkGetPhysicalDeviceQueueFamilyDataGraphProcessingEnginePropertiesARM, vFppp)
GO(vkGetPhysicalDeviceQueueFamilyDataGraphPropertiesARM, iFpupp)

// VK_ARM_performance_counters_by_region
GO(vkEnumeratePhysicalDeviceQueueFamilyPerformanceCountersByRegionARM, iFpuppp)

// VK_EXT_descriptor_heap
GO(vkCmdBindResourceHeapEXT, vFpp)
GO(vkCmdBindSamplerHeapEXT, vFpp)
GO(vkCmdPushDataEXT, vFpp)
GO(vkGetImageOpaqueCaptureDataEXT, iFpupp)
GO(vkGetPhysicalDeviceDescriptorSizeEXT, UFpu)
GO(vkWriteResourceDescriptorsEXT, iFpupp)
GO(vkWriteSamplerDescriptorsEXT, iFpupp)

// VK_EXT_descriptor_heap + VK_ARM_tensors
GO(vkGetTensorOpaqueCaptureDataARM, iFpupp)

// VK_EXT_descriptor_heap + VK_EXT_custom_border_color
GO(vkRegisterCustomBorderColorEXT, iFppup)
GO(vkUnregisterCustomBorderColorEXT, vFpu)

// VK_EXT_custom_border_color
// no new functions

// VK_KHR_win32_surface
GOM(vkCreateWin32SurfaceKHR, iFEpppp) // Warning: failed to confirm
GO(vkGetPhysicalDeviceWin32PresentationSupportKHR, iFpu) // Warning: failed to confirm

// VK_KHR_swapchain_maintenance1
GO(vkReleaseSwapchainImagesKHR, iFpp)

// Layer stuffs
GO(vk_icdNegotiateLoaderICDInterfaceVersion, iFp) // Warning: failed to confirm
//GO(vkNegotiateLoaderLayerInterfaceVersion, iFp)
