#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

// VK_VERSION_1_0
GO(vkAllocateCommandBuffers, iFppp)
GO(vkAllocateDescriptorSets, iFppp)
GOM(vkAllocateMemory, iFEpppp)
GO(vkBeginCommandBuffer, iFpp)
GO(vkBindBufferMemory, iFpUUU)
GO(vkBindImageMemory, iFpUUU)
GO(vkCmdBeginQuery, vFpUui)
GO(vkCmdBeginRenderPass, vFppp)
GO(vkCmdBindDescriptorSets, vFpiUuupup)
GO(vkCmdBindIndexBuffer, vFpUUi)
GO(vkCmdBindPipeline, vFppU)
GO(vkCmdBindVertexBuffers, vFpuupp)
GO(vkCmdBlitImage, vFpUiUiupi)
GO(vkCmdClearAttachments, vFpupup)
GO(vkCmdClearColorImage, vFpUipup)
GO(vkCmdClearDepthStencilImage, vFpUipup)
GO(vkCmdCopyBuffer, vFpUUup)
GO(vkCmdCopyBufferToImage, vFpUUiup)
GO(vkCmdCopyImage, vFpUiUiup)
GO(vkCmdCopyImageToBuffer, vFpUiUup)
GO(vkCmdCopyQueryPoolResults, vFpUuuUUUi)
GO(vkCmdDispatch, vFpuuu)
GO(vkCmdDispatchIndirect, vFpUU)
GO(vkCmdDraw, vFpuuuu)
GO(vkCmdDrawIndexed, vFpuuuiu)
GO(vkCmdDrawIndexedIndirect, vFpUUuu)
GO(vkCmdDrawIndirect, vFpUUuu)
GO(vkCmdEndQuery, vFpUu)
GO(vkCmdEndRenderPass, vFp)
GO(vkCmdExecuteCommands, vFpup)
GO(vkCmdFillBuffer, vFpUUUu)
GO(vkCmdNextSubpass, vFpi)
GOM(vkCmdPipelineBarrier, vFEpiiiupupup)
GO(vkCmdPushConstants, vFpUiuup)
GO(vkCmdResetEvent, vFpUi)
GO(vkCmdResetQueryPool, vFpUuu)
GO(vkCmdResolveImage, vFpUiUiup)
GO(vkCmdSetBlendConstants, vFpp)
GO(vkCmdSetDepthBias, vFpfff)
GO(vkCmdSetDepthBounds, vFpff)
GO(vkCmdSetEvent, vFpUi)
GO(vkCmdSetLineWidth, vFpf)
GO(vkCmdSetScissor, vFpuup)
GO(vkCmdSetStencilCompareMask, vFpiu)
GO(vkCmdSetStencilReference, vFpiu)
GO(vkCmdSetStencilWriteMask, vFpiu)
GO(vkCmdSetViewport, vFpuup)
GO(vkCmdUpdateBuffer, vFpUUUp)
GO(vkCmdWaitEvents, vFpupiiupupup)
GO(vkCmdWriteTimestamp, vFpiUu)
GOM(vkCreateBuffer, iFEpppp)
GOM(vkCreateBufferView, iFEpppp)
GOM(vkCreateCommandPool, iFEpppp)
GOM(vkCreateComputePipelines, iFEpUuppp)
GOM(vkCreateDescriptorPool, iFEpppp)
GOM(vkCreateDescriptorSetLayout, iFEpppp)
GOM(vkCreateDevice, iFEpppp)
GOM(vkCreateEvent, iFEpppp)
GOM(vkCreateFence, iFEpppp)
GOM(vkCreateFramebuffer, iFEpppp)
GOM(vkCreateGraphicsPipelines, iFEpUuppp)
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
GOM(vkDestroyBuffer, vFEpUp)
GOM(vkDestroyBufferView, vFEpUp)
GOM(vkDestroyCommandPool, vFEpUp)
GOM(vkDestroyDescriptorPool, vFEpUp)
GOM(vkDestroyDescriptorSetLayout, vFEpUp)
GOM(vkDestroyDevice, vFEpp)
GOM(vkDestroyEvent, vFEpUp)
GOM(vkDestroyFence, vFEpUp)
GOM(vkDestroyFramebuffer, vFEpUp)
GOM(vkDestroyImage, vFEpUp)
GOM(vkDestroyImageView, vFEpUp)
GOM(vkDestroyInstance, vFEpp)
GOM(vkDestroyPipeline, vFEpUp)
GOM(vkDestroyPipelineCache, vFEpUp)
GOM(vkDestroyPipelineLayout, vFEpUp)
GOM(vkDestroyQueryPool, vFEpUp)
GOM(vkDestroyRenderPass, vFEpUp)
GOM(vkDestroySampler, vFEpUp)
GOM(vkDestroySemaphore, vFEpUp)
GOM(vkDestroyShaderModule, vFEpUp)
GO(vkDeviceWaitIdle, iFp)
GO(vkEndCommandBuffer, iFp)
GO(vkEnumerateDeviceExtensionProperties, iFpppp)
GO(vkEnumerateDeviceLayerProperties, iFppp)
GO(vkEnumerateInstanceExtensionProperties, iFppp)
GO(vkEnumerateInstanceLayerProperties, iFpp)
GO(vkEnumeratePhysicalDevices, iFppp)
GO(vkFlushMappedMemoryRanges, iFpup)    // should wrap the array of VkMappedMemoryRange
GO(vkFreeCommandBuffers, vFpUup)
GO(vkFreeDescriptorSets, iFpUup)
GOM(vkFreeMemory, iFEpUp)
GO(vkGetBufferMemoryRequirements, iFpUp)
GO(vkGetDeviceMemoryCommitment, vFpUp)
GOM(vkGetDeviceProcAddr, pFEpp)
GO(vkGetDeviceQueue, vFpuup)
GO(vkGetEventStatus, iFpU)
GO(vkGetFenceStatus, iFpU)
GO(vkGetImageMemoryRequirements, vFpUp)
GO(vkGetImageSparseMemoryRequirements, vFpUpp)
GO(vkGetImageSubresourceLayout, vFpUpp)
GOM(vkGetInstanceProcAddr, pFEpp)
GO(vkGetPhysicalDeviceFeatures, vFpp)
GO(vkGetPhysicalDeviceFormatProperties, vFpip)
GO(vkGetPhysicalDeviceImageFormatProperties, iFpiiiiip) //VkImageFormatProperties sems OK
GOM(vkGetPhysicalDeviceMemoryProperties, vFEpp)
GOM(vkGetPhysicalDeviceProperties, vFEpp)
GO(vkGetPhysicalDeviceQueueFamilyProperties, vFppp)   //VkQueueFamilyProperties is OK
GOM(vkGetPhysicalDeviceSparseImageFormatProperties, vFEpiiiiipp)
GO(vkGetPipelineCacheData, iFpUpp)
GO(vkGetQueryPoolResults, iFpUuuLpUi)
GO(vkGetRenderAreaGranularity, vFpUp)
GO(vkInvalidateMappedMemoryRanges, iFpup)   //VkMappedMemoryRange seems OK
GO(vkMapMemory, iFpUUUip)
GO(vkMergePipelineCaches, iFpUup)
GO(vkQueueBindSparse, iFpupU)
GO(vkQueueSubmit, iFpupU)
GO(vkQueueWaitIdle, iFp)
GO(vkResetCommandBuffer, iFpi)
GO(vkResetCommandPool, iFpUi)
GO(vkResetDescriptorPool, iFpUi)
GO(vkResetEvent, iFpU)
GO(vkResetFences, iFpup)
GO(vkSetEvent, iFpU)
GO(vkUnmapMemory, vFpU)
GOM(vkUpdateDescriptorSets, vFEpupup)
GO(vkWaitForFences, iFpupiU)

// VK_VERSION_1_1
GO(vkBindBufferMemory2, iFpup)
GO(vkBindImageMemory2, iFpup)
GO(vkCmdDispatchBase, vFpuuuuuu)
GO(vkCmdSetDeviceMask, vFpu)
GOM(vkCreateDescriptorUpdateTemplate, iFEpppp)
GOM(vkCreateSamplerYcbcrConversion, iFEpppp)
GOM(vkDestroyDescriptorUpdateTemplate, vFEpUp)
GO(vkEnumerateInstanceVersion, iFp)
GO(vkEnumeratePhysicalDeviceGroups, iFppp)  //VkPhysicalDeviceGroupProperties seems OK
GO(vkGetBufferMemoryRequirements2, iFppp)
GO(vkGetImageMemoryRequirements2, vFppp)
GO(vkGetImageSparseMemoryRequirements2, vFpppp)
GO(vkGetDescriptorSetLayoutSupport, vFppp)
GO(vkGetDeviceGroupPeerMemoryFeatures, vFpuuup)
GO(vkGetDeviceQueue2, vFppp)
GO(vkGetPhysicalDeviceExternalBufferProperties, vFppp)
GO(vkGetPhysicalDeviceExternalFenceProperties, vFppp)
GO(vkGetPhysicalDeviceExternalSemaphoreProperties, vFppp)
GO(vkGetPhysicalDeviceFeatures2, vFpp)
GO(vkGetPhysicalDeviceFormatProperties2, vFpip)
GO(vkGetPhysicalDeviceImageFormatProperties2, vFppp)
GO(vkGetPhysicalDeviceMemoryProperties2, vFpp)
GO(vkGetPhysicalDeviceProperties2, vFpp)
GO(vkGetPhysicalDeviceQueueFamilyProperties2, vFppp)    //VkQueueFamilyProperties2 seems OK
GO(vkGetPhysicalDeviceSparseImageFormatProperties2, vFpppp) //VkSparseImageFormatProperties2 seems OK
GO(vkGetPhysicalDeviceToolProperties, iFppp)
GO(vkUpdateDescriptorSetWithTemplate, vFpUUp)
GO(vkTrimCommandPool, vFpUi)

// VK_VERSION_1_2
GO(vkResetQueryPool, vFpUuu)
GO(vkCmdBeginRenderPass2, vFppp)
GO(vkCmdEndRenderPass2, vFpp)
GO(vkCmdNextSubpass2, vFppp)
GOM(vkCreateRenderPass2, iFEpppp)
GO(vkCmdDrawIndexedIndirectCount, vFpUUUUuu)
GO(vkCmdDrawIndirectCount, vFpUUUUuu)

// VK_EXT_debug_report
GOM(vkCreateDebugReportCallbackEXT, iFEpppp)
GO(vkDebugReportMessageEXT, vFpiiULipp)
GOM(vkDestroyDebugReportCallbackEXT, iFEppp)

//VK_EXT_debug_utils
GO(vkCmdBeginDebugUtilsLabelEXT, vFpp)  //TODO: Cehck alignement of this extension
GO(vkCmdEndDebugUtilsLabelEXT, vFp)
GO(vkCmdInsertDebugUtilsLabelEXT, vFpp)
GOM(vkCreateDebugUtilsMessengerEXT, iFEpppp)
GOM(vkDestroyDebugUtilsMessengerEXT, vFEppp)
GO(vkQueueBeginDebugUtilsLabelEXT, vFpp)
GO(vkQueueEndDebugUtilsLabelEXT, vFp)
GO(vkQueueInsertDebugUtilsLabelEXT, vFpp)
GO(vkSetDebugUtilsObjectNameEXT, iFpp)
GO(vkSetDebugUtilsObjectTagEXT, iFpp)
//GOM(vkSubmitDebugUtilsMessageEXT, vFEpppp)    // callback in last arguments

// VK_KHR_external_memory_capabilities
GO(vkGetPhysicalDeviceExternalBufferPropertiesKHR, vFppp)

// VK_KHR_get_physical_device_properties2
GO(vkGetPhysicalDeviceFeatures2KHR, vFpp)
GO(vkGetPhysicalDeviceFormatProperties2KHR, vFpip)
GO(vkGetPhysicalDeviceImageFormatProperties2KHR, vFppp)
GO(vkGetPhysicalDeviceMemoryProperties2KHR, vFpp)
GO(vkGetPhysicalDeviceProperties2KHR, vFpp)
GO(vkGetPhysicalDeviceQueueFamilyProperties2KHR, vFppp)
GO(vkGetPhysicalDeviceSparseImageFormatProperties2KHR, vFpppp)  //VkSparseImageFormatProperties2 seems OK

// VK_KHR_get_surface_capabilities2
GO(vkGetPhysicalDeviceSurfaceCapabilities2KHR, iFppp)
GO(vkGetPhysicalDeviceSurfaceFormats2KHR, iFpppp)   //VkSurfaceFormat2KHR seems OK (but array)

// VK_KHR_surface
GOM(vkDestroySurfaceKHR, vFEpUp)
GO(vkGetPhysicalDeviceSurfaceCapabilitiesKHR, iFpUp)    //VkSurfaceCapabilitiesKHR seems OK
GO(vkGetPhysicalDeviceSurfaceFormatsKHR, iFpUpp)
GO(vkGetPhysicalDeviceSurfacePresentModesKHR, iFpUpp)
GO(vkGetPhysicalDeviceSurfaceSupportKHR, iFpuUp)

// VK_KHR_xcb_surface
GOM(vkCreateXcbSurfaceKHR, iFEpppp)
GO(vkGetPhysicalDeviceXcbPresentationSupportKHR, iFpupp)

// VK_KHR_xlib_surface
GOM(vkCreateXlibSurfaceKHR, iFEpppp)
GO(vkGetPhysicalDeviceXlibPresentationSupportKHR, iFpupp)

// VK_KHR_swapchain
GO(vkAcquireNextImageKHR, iFpUUUUp)
GO(vkAcquireNextImage2KHR, iFppp)
GOM(vkCreateSwapchainKHR, iFEpppp)
GOM(vkDestroySwapchainKHR, vFEpUp)
GO(vkGetDeviceGroupPresentCapabilitiesKHR, iFpp)
GO(vkGetDeviceGroupSurfacePresentModesKHR, iFpUp)
GO(vkGetPhysicalDevicePresentRectanglesKHR, iFpUpp)
GO(vkGetSwapchainImagesKHR, iFpUpp)
GO(vkQueuePresentKHR, iFpp)

// VK_KHR_bind_memory2
GO(vkBindBufferMemory2KHR, iFpup)
GO(vkBindImageMemory2KHR, iFpup)

// VK_KHR_display
GOM(vkCreateDisplayModeKHR, iFEpUppp)
GOM(vkCreateDisplayPlaneSurfaceKHR, iFEpppp)
GO(vkGetDisplayModePropertiesKHR, iFpUpp)   //VkDisplayModePropertiesKHR seems OK
GOM(vkGetDisplayPlaneCapabilitiesKHR, iFEpUup)
GO(vkGetDisplayPlaneSupportedDisplaysKHR, iFpupp)
GO(vkGetPhysicalDeviceDisplayPlanePropertiesKHR, iFppp) //VkDisplayPlanePropertiesKHR is OK
GOM(vkGetPhysicalDeviceDisplayPropertiesKHR, iFEppp)

// VK_KHR_descriptor_update_template
GOM(vkCreateDescriptorUpdateTemplateKHR, iFEpppp)
GOM(vkDestroyDescriptorUpdateTemplateKHR, vFEpUp)
GO(vkUpdateDescriptorSetWithTemplateKHR, vFpUUp)
GO(vkCmdPushDescriptorSetWithTemplateKHR, vFpUUup)

// VK_EXT_display_surface_counter
GO(vkGetPhysicalDeviceSurfaceCapabilities2EXT, iFpUp)

// VK_KHR_get_display_properties2
GO(vkGetDisplayModeProperties2KHR, iFpUpp)
GO(vkGetDisplayPlaneCapabilities2KHR, iFppp)
GO(vkGetPhysicalDeviceDisplayPlaneProperties2KHR, iFppp)
GO(vkGetPhysicalDeviceDisplayProperties2KHR, iFppp)

// VK_KHR_device_group
GO(vkCmdDispatchBaseKHR, vFpuuuuuu)
GO(vkCmdSetDeviceMaskKHR, vFpu)
GO(vkGetDeviceGroupPeerMemoryFeaturesKHR, vFpuuup)

// VK_KHR_sampler_ycbcr_conversion
GOM(vkCreateSamplerYcbcrConversionKHR, iFEpppp)
GOM(vkDestroySamplerYcbcrConversionKHR, vFEpUp)

// VK_KHR_display_swapchain
GOM(vkCreateSharedSwapchainsKHR, iFEpuppp)

// VK_KHR_wayland_surface
GOM(vkCreateWaylandSurfaceKHR, iFEpppp)
GO(vkGetPhysicalDeviceWaylandPresentationSupportKHR, iFpup)

// VK_KHR_device_group_creation
GO(vkEnumeratePhysicalDeviceGroupsKHR, iFppp)

// VK_KHR_get_memory_requirements2
GO(vkGetBufferMemoryRequirements2KHR, iFppp)
GO(vkGetImageMemoryRequirements2KHR, vFppp)
GO(vkGetImageSparseMemoryRequirements2KHR, vFpppp)

// VK_KHR_external_fence_capabilities
GO(vkGetPhysicalDeviceExternalFencePropertiesKHR, vFppp)

// VK_KHR_external_semaphore_capabilities
GO(vkGetPhysicalDeviceExternalSemaphorePropertiesKHR, vFppp)

// VK_KHR_maintenance1
GO(vkTrimCommandPoolKHR, vFpUi)

// VK_KHR_maintenance2
// no functions

// VK_KHR_maintenance3
GO(vkGetDescriptorSetLayoutSupportKHR, vFppp)

// VK_KHR_external_memory_fd
GO(vkGetMemoryFdKHR, iFppp)
GO(vkGetMemoryFdPropertiesKHR, iFpiip)

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
GO(vkResetQueryPoolEXT, vFpUuu)

// VK_EXT_memory_priority
// no functions

// VK_EXT_shader_demote_to_helper_invocation
// no functions

// VK_EXT_transform_feedback
GO(vkCmdBeginQueryIndexedEXT, vFpUuiu)
GO(vkCmdBeginTransformFeedbackEXT, vFpuupp)
GO(vkCmdBindTransformFeedbackBuffersEXT, vFpuuppp)
GO(vkCmdDrawIndirectByteCountEXT, vFpuuUUuu)
GO(vkCmdEndQueryIndexedEXT, vFpUuu)
GO(vkCmdEndTransformFeedbackEXT, vFpuupp)

// VK_EXT_vertex_attribute_divisor
// no functions

// VK_EXT_full_screen_exclusive
GO(vkAcquireFullScreenExclusiveModeEXT, iFpU)
GO(vkGetPhysicalDeviceSurfacePresentModes2EXT, iFpppp)
GO(vkReleaseFullScreenExclusiveModeEXT, iFpU)
GO(vkGetDeviceGroupSurfacePresentModes2EXT, iFppp)

// VK_EXT_calibrated_timestamps
GO(vkGetCalibratedTimestampsEXT, iFpuppp)
GO(vkGetPhysicalDeviceCalibrateableTimeDomainsEXT, iFppp)

// VK_EXT_sample_locations
GO(vkCmdSetSampleLocationsEXT, vFpp)
GO(vkGetPhysicalDeviceMultisamplePropertiesEXT, vFpip)

// VK_EXT_headless_surface
GOM(vkCreateHeadlessSurfaceEXT, iFEpppp)

//VK_KHR_performance_query
GO(vkAcquireProfilingLockKHR, iFpp)
GO(vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR, iFpuppp)
GO(vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR, vFppp)
GO(vkReleaseProfilingLockKHR, vFp)

// VK_NV_cooperative_matrix
GO(vkGetPhysicalDeviceCooperativeMatrixPropertiesNV, iFppp)

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
GO(vkCmdSetPrimitiveTopologyEXT, vFpi)
GO(vkCmdSetScissorWithCountEXT, vFup)
GO(vkCmdSetStencilOpEXT, vFpuiiii)
GO(vkCmdSetStencilTestEnableEXT, vFpu)
GO(vkCmdSetViewportWithCountEXT, vFpup)

// VK_KHR_draw_indirect_count
GO(vkCmdDrawIndexedIndirectCountKHR, vFpUUUUuu)
GO(vkCmdDrawIndirectCountKHR, vFpUUUUuu)

// VK_AMD_draw_indirect_count
GO(vkCmdDrawIndexedIndirectCountAMD, vFpUUUUuu)
GO(vkCmdDrawIndirectCountAMD, vFpUUUUuu)

// VK_AMD_buffer_marker
GO(vkCmdWriteBufferMarkerAMD, vFpuUUu)

// VK_AMD_shader_info
GO(vkGetShaderInfoAMD, iFpUuupp)

// VK_EXT_debug_marker
GO(vkCmdDebugMarkerBeginEXT, vFpp)
GO(vkCmdDebugMarkerEndEXT, vFp)
GO(vkCmdDebugMarkerInsertEXT, vFpp)
GO(vkDebugMarkerSetObjectNameEXT, iFpp)
GO(vkDebugMarkerSetObjectTagEXT, iFpp)

// VK_EXT_discard_rectangles
GO(vkCmdSetDiscardRectangleEXT, vFpuup)

// VK_EXT_display_control
GO(vkDisplayPowerControlEXT, iFpUp)
GO(vkGetSwapchainCounterEXT, iFpUup)
GOM(vkRegisterDeviceEventEXT, iFEpppp)
GOM(vkRegisterDisplayEventEXT, iFEpUppp)

// VK_EXT_external_memory_host
GO(vkGetMemoryHostPointerPropertiesEXT, iFpupp)

// VK_EXT_hdr_metadata
GO(vkSetHdrMetadataEXT, vFpupp)

// VK_EXT_validation_cache
GOM(vkCreateValidationCacheEXT, iFEpppp)
GOM(vkDestroyValidationCacheEXT, vFEpUp)
GO(vkGetValidationCacheDataEXT, iFpUpp)
GO(vkMergeValidationCachesEXT, iFpUup)

// VK_GOOGLE_display_timing
GO(vkGetPastPresentationTimingGOOGLE, iFpUpp)
GO(vkGetRefreshCycleDurationGOOGLE, iFpUp)

// VK_KHR_external_fence_fd
GO(vkGetFenceFdKHR, iFppp)
GO(vkImportFenceFdKHR, iFpp)

// VK_KHR_external_semaphore_fd
GO(vkGetSemaphoreFdKHR, iFppp)
GO(vkImportSemaphoreFdKHR, iFpp)

// VK_KHR_push_descriptor
GO(vkCmdPushDescriptorSetKHR, vFpiUuup)

// VK_KHR_shared_presentable_image
GO(vkGetSwapchainStatusKHR, iFpU)

// VK_NV_clip_space_w_scaling
GO(vkCmdSetViewportWScalingNV, vFpuup)

// VK_NV_device_diagnostic_checkpoints
GO(vkCmdSetCheckpointNV, vFpp)
GO(vkGetQueueCheckpointDataNV, vFppp)

// VK_NV_mesh_shader
GO(vkCmdDrawMeshTasksIndirectCountNV, vFpUUUUuu)
GO(vkCmdDrawMeshTasksIndirectNV, vFpUUuu)
GO(vkCmdDrawMeshTasksNV, vFpuu)

// VK_NV_scissor_exclusive
GO(vkCmdSetExclusiveScissorNV, vFpuup)

// VK_NV_shading_rate_image
GO(vkCmdBindShadingRateImageNV, vFpUi)
GO(vkCmdSetCoarseSampleOrderNV, vFpiup)
GO(vkCmdSetViewportShadingRatePaletteNV, vFpuup)
