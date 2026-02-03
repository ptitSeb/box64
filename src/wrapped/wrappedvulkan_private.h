#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
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
GO(vkBindBufferMemory, iFpUUU)
GO(vkBindImageMemory, iFpUUU)
GO(vkCmdBeginQuery, vFpUuu)
GO(vkCmdBeginRenderPass, vFppi)
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
GO(vkCmdCopyQueryPoolResults, vFpUuuUUUu)
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
GO(vkCmdPipelineBarrier, vFpuuuupupup)
GO(vkCmdPushConstants, vFpUuuup)
GO(vkCmdResetEvent, vFpUu)
GO(vkCmdResetQueryPool, vFpUuu)
GO(vkCmdResolveImage, vFpUiUiup)
GO(vkCmdSetBlendConstants, vFpp)
GO(vkCmdSetDepthBias, vFpfff)
GO(vkCmdSetDepthBounds, vFpff)
GO(vkCmdSetEvent, vFpUu)
GO(vkCmdSetLineWidth, vFpf)
GO(vkCmdSetScissor, vFpuup)
GO(vkCmdSetStencilCompareMask, vFpuu)
GO(vkCmdSetStencilReference, vFpuu)
GO(vkCmdSetStencilWriteMask, vFpuu)
GO(vkCmdSetViewport, vFpuup)
GO(vkCmdUpdateBuffer, vFpUUUp)
GO(vkCmdWaitEvents, vFpupuuupupup)
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
GOM(vkFreeMemory, vFEpUp)
GO(vkGetBufferMemoryRequirements, vFpUp)
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
GO(vkGetPhysicalDeviceImageFormatProperties, iFpiiiuup) // VkImageFormatProperties sems OK
GO(vkGetPhysicalDeviceMemoryProperties, vFpp)
GO(vkGetPhysicalDeviceProperties, vFpp)
GO(vkGetPhysicalDeviceQueueFamilyProperties, vFppp)   //VkQueueFamilyProperties is OK
GO(vkGetPhysicalDeviceSparseImageFormatProperties, vFpiiiuipp)
GO(vkGetPipelineCacheData, iFpUpp)
GO(vkGetQueryPoolResults, iFpUuuLpUu)
GO(vkGetRenderAreaGranularity, vFpUp)
GO(vkInvalidateMappedMemoryRanges, iFpup)   //VkMappedMemoryRange seems OK
GO(vkMapMemory, iFpUUUup)
GO(vkMergePipelineCaches, iFpUup)
GO(vkQueueBindSparse, iFpupU)
GO(vkQueueSubmit, iFpupU)
GO(vkQueueWaitIdle, iFp)
GO(vkResetCommandBuffer, iFpu)
GO(vkResetCommandPool, iFpUu)
GO(vkResetDescriptorPool, iFpUu)
GO(vkResetEvent, iFpU)
GO(vkResetFences, iFpup)
GO(vkSetEvent, iFpU)
GO(vkUnmapMemory, vFpU)
GO(vkUpdateDescriptorSets, vFpupup)
GO(vkWaitForFences, iFpupuU)

// VK_VERSION_1_1
GO(vkBindBufferMemory2, iFpup)
GO(vkBindImageMemory2, iFpup)
GO(vkCmdDispatchBase, vFpuuuuuu)
GO(vkCmdSetDeviceMask, vFpu)
GOM(vkCreateDescriptorUpdateTemplate, iFEpppp)
GOM(vkCreateSamplerYcbcrConversion, iFEpppp)
GOM(vkDestroyDescriptorUpdateTemplate, vFEpUp)
GOM(vkDestroySamplerYcbcrConversion, vFEpUp)
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
GO(vkGetPhysicalDeviceImageFormatProperties2, iFppp)
GO(vkGetPhysicalDeviceMemoryProperties2, vFpp)
GOM(vkGetPhysicalDeviceProperties2, vFEpp)
GO(vkGetPhysicalDeviceQueueFamilyProperties2, vFppp)    //VkQueueFamilyProperties2 seems OK
GO(vkGetPhysicalDeviceSparseImageFormatProperties2, vFpppp) //VkSparseImageFormatProperties2 seems OK
GO(vkGetPhysicalDeviceToolProperties, iFppp)
GO(vkUpdateDescriptorSetWithTemplate, vFpUUp)
GO(vkTrimCommandPool, vFpUu)

// VK_VERSION_1_2
GO(vkResetQueryPool, vFpUuu)
GO(vkCmdBeginRenderPass2, vFppp)
GO(vkCmdEndRenderPass2, vFpp)
GO(vkCmdNextSubpass2, vFppp)
GO(vkCmdDrawIndexedIndirectCount, vFpUUUUuu)
GO(vkCmdDrawIndirectCount, vFpUUUUuu)
GOM(vkCreateRenderPass2, iFEpppp)
GO(vkGetBufferDeviceAddress, UFpp)
GO(vkGetBufferOpaqueCaptureAddress, UFpp)
GO(vkGetDeviceMemoryOpaqueCaptureAddress, UFpp)
GO(vkGetSemaphoreCounterValue, iFpUp)
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
GO(vkCmdSetLogicOp, vFpi)
GO(vkCmdSetPatchControlPoints, vFpu)
GO(vkCmdSetPrimitiveRestartEnable, vFpu)
GO(vkCmdSetRasterizerDiscardEnable, vFpu)
GOM(vkCreatePrivateDataSlot, iFEpppp)
GOM(vkDestroyPrivateDataSlot, vFEpUp)
GO(vkGetPrivateData, vFpiUUp)
GO(vkSetPrivateData, iFpiUUU)
GO(vkGetDeviceBufferMemoryRequirements, vFppp)
GO(vkGetDeviceImageMemoryRequirements, vFppp)
GO(vkGetDeviceImageSparseMemoryRequirements, vFpppp)
GO(vkCmdPipelineBarrier2, vFpp)
GO(vkCmdResetEvent2, vFpUU)
GO(vkCmdSetEvent2, vFpUp)
GO(vkCmdWaitEvents2, vFpupp)
GO(vkCmdWriteTimestamp2, vFpUUu)
GO(vkQueueSubmit2, iFpupU)
GO(vkCmdBindVertexBuffers2, vFpuupppp)
GO(vkCmdSetCullMode, vFpu)
GO(vkCmdSetDepthBoundsTestEnable, vFpu)
GO(vkCmdSetDepthCompareOp, vFpu)
GO(vkCmdSetDepthTestEnable, vFpu)
GO(vkCmdSetDepthWriteEnable, vFpu)
GO(vkCmdSetFrontFace, vFpi)
GO(vkCmdSetPrimitiveTopology, vFpi)
GO(vkCmdSetScissorWithCount, vFpup)
GO(vkCmdSetStencilOp, vFpuiiii)
GO(vkCmdSetStencilTestEnable, vFpu)
GO(vkCmdSetViewportWithCount, vFpup)

// VK_VERSION_1_4
GO(vkCmdBindDescriptorSets2, vFpp)
GO(vkCmdBindIndexBuffer2, vFpUUUi)
GO(vkCmdPushConstants2, vFpp)
GO(vkCmdPushDescriptorSet, vFpiUuup)
GO(vkCmdPushDescriptorSet2,vFpp)
GO(vkCmdPushDescriptorSetWithTemplate, vFpUUup)
GO(vkCmdPushDescriptorSetWithTemplate2, vFpp)
GO(vkCmdSetLineStipple, vFpuW)
GO(vkCmdSetRenderingAttachmentLocations, vFpp)
GO(vkCmdSetRenderingInputAttachmentIndices, vFpp)
GO(vkCopyImageToImage, iFpp)
GO(vkCopyImageToMemory, iFpp)
GO(vkCopyMemoryToImage, iFpp)
GO(vkGetDeviceImageSubresourceLayout, vFppp)
GO(vkGetImageSubresourceLayout2, vFpUpp)
GO(vkGetRenderingAreaGranularity, vFppp)
GO(vkMapMemory2, iFppp)
GO(vkTransitionImageLayout, iFpup)
GO(vkUnmapMemory2, iFpp)

// VK_EXT_debug_report
GOM(vkCreateDebugReportCallbackEXT, iFEpppp)
GO(vkDebugReportMessageEXT, vFpuiULipp)
GOM(vkDestroyDebugReportCallbackEXT, vFEppp)

//VK_EXT_debug_utils
GO(vkCmdBeginDebugUtilsLabelEXT, vFpp)  //TODO: Check alignement of this extension
GO(vkCmdEndDebugUtilsLabelEXT, vFp)
GO(vkCmdInsertDebugUtilsLabelEXT, vFpp)
GOM(vkCreateDebugUtilsMessengerEXT, iFEpppp)
GOM(vkDestroyDebugUtilsMessengerEXT, vFEppp)
GO(vkQueueBeginDebugUtilsLabelEXT, vFpp)
GO(vkQueueEndDebugUtilsLabelEXT, vFp)
GO(vkQueueInsertDebugUtilsLabelEXT, vFpp)
GO(vkSetDebugUtilsObjectNameEXT, iFpp)
GO(vkSetDebugUtilsObjectTagEXT, iFpp)
GO(vkSubmitDebugUtilsMessageEXT, vFpiup)

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
GO(vkGetPhysicalDeviceXcbPresentationSupportKHR, uFpubp)

// VK_KHR_xlib_surface
GOM(vkCreateXlibSurfaceKHR, iFEpppp)
GO(vkGetPhysicalDeviceXlibPresentationSupportKHR, uFpupp)

// VK_KHR_android_surface
GOM(vkCreateAndroidSurfaceKHR, iFEpppp)

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
GO(vkGetDisplayPlaneCapabilitiesKHR, iFpUup)
GO(vkGetDisplayPlaneSupportedDisplaysKHR, iFpupp)
GO(vkGetPhysicalDeviceDisplayPlanePropertiesKHR, iFppp) //VkDisplayPlanePropertiesKHR is OK
GO(vkGetPhysicalDeviceDisplayPropertiesKHR, iFppp)

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
GO(vkGetPhysicalDeviceWaylandPresentationSupportKHR, uFpup)

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
GO(vkCmdBeginQueryIndexedEXT, vFpUuuu)
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
GO(vkCmdSetDepthCompareOpEXT, vFpi)
GO(vkCmdSetDepthTestEnableEXT, vFpu)
GO(vkCmdSetDepthWriteEnableEXT, vFpu)
GO(vkCmdSetFrontFaceEXT, vFpu)
GO(vkCmdSetPrimitiveTopologyEXT, vFpi)
GO(vkCmdSetScissorWithCountEXT, vFpup)
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
GO(vkCmdWriteBufferMarkerAMD, vFpiUUu)

// VK_AMD_shader_info
GO(vkGetShaderInfoAMD, iFpUiipp)

// VK_EXT_debug_marker
GO(vkCmdDebugMarkerBeginEXT, vFpp)
GO(vkCmdDebugMarkerEndEXT, vFp)
GO(vkCmdDebugMarkerInsertEXT, vFpp)
GO(vkDebugMarkerSetObjectNameEXT, iFpp)
GO(vkDebugMarkerSetObjectTagEXT, iFpp)

// VK_EXT_discard_rectangles
GO(vkCmdSetDiscardRectangleEXT, vFpuup)
GO(vkCmdSetDiscardRectangleEnableEXT, vFpu)
GO(vkCmdSetDiscardRectangleModeEXT, vFpi)

// VK_EXT_display_control
GO(vkDisplayPowerControlEXT, iFpUp)
GO(vkGetSwapchainCounterEXT, iFpUip)
GOM(vkRegisterDeviceEventEXT, iFEpppp)
GOM(vkRegisterDisplayEventEXT, iFEpUppp)

// VK_EXT_external_memory_host
GO(vkGetMemoryHostPointerPropertiesEXT, iFpipp)

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
GO(vkCmdSetExclusiveScissorEnableNV, vFpuup)
GO(vkCmdSetExclusiveScissorNV, vFpuup)

// VK_NV_shading_rate_image
GO(vkCmdBindShadingRateImageNV, vFpUi)
GO(vkCmdSetCoarseSampleOrderNV, vFpiup)
GO(vkCmdSetViewportShadingRatePaletteNV, vFpuup)

// VK_KHR_video_queue
GO(vkBindVideoSessionMemoryKHR, iFpUup)
GO(vkCmdBeginVideoCodingKHR, vFpp)
GO(vkCmdControlVideoCodingKHR, vFpp)
GO(vkCmdEndVideoCodingKHR, vFpp)
GOM(vkCreateVideoSessionKHR, iFEpppp)
GOM(vkCreateVideoSessionParametersKHR, iFEpppp)
GOM(vkDestroyVideoSessionKHR, vFEpUp)
GOM(vkDestroyVideoSessionParametersKHR, vFEpUp)
GO(vkGetPhysicalDeviceVideoCapabilitiesKHR, iFppp)
GO(vkGetPhysicalDeviceVideoFormatPropertiesKHR, iFpppp)
GO(vkGetVideoSessionMemoryRequirementsKHR, iFpppp)
GO(vkUpdateVideoSessionParametersKHR, iFpUp)

// VK_KHR_buffer_device_address
GO(vkGetBufferDeviceAddressKHR, UFpp)
GO(vkGetBufferOpaqueCaptureAddressKHR, UFpp)
GO(vkGetDeviceMemoryOpaqueCaptureAddressKHR, UFpp)

// VK_EXT_buffer_device_address
GO(vkGetBufferDeviceAddressEXT, UFpp)

// VK_KHR_timeline_semaphore
GO(vkGetSemaphoreCounterValueKHR, iFpUp)
GO(vkSignalSemaphoreKHR, iFpp)
GO(vkWaitSemaphoresKHR, iFppU)

// VK_AMD_display_native_hdr
GO(vkSetLocalDimmingAMD, vFpUu)

// VK_EXT_color_write_enable
GO(vkCmdSetColorWriteEnableEXT, vFpup)

// VK_EXT_extended_dynamic_state2
GO(vkCmdSetDepthBiasEnableEXT, vFpi)
GO(vkCmdSetLogicOpEXT, vFpi)
GO(vkCmdSetPatchControlPointsEXT, vFpu)
GO(vkCmdSetPrimitiveRestartEnableEXT, vFpi)
GO(vkCmdSetRasterizerDiscardEnableEXT, vFpi)

// VK_EXT_image_drm_format_modifier
GO(vkGetImageDrmFormatModifierPropertiesEXT, iFpUp)

// VK_EXT_line_rasterization
GO(vkCmdSetLineStippleEXT, vFpuW)

// VK_EXT_multi_draw
GO(vkCmdDrawMultiEXT, vFpupuuu)
GO(vkCmdDrawMultiIndexedEXT, vFpupuuup)

// VK_EXT_pageable_device_local_memory
GO(vkSetDeviceMemoryPriorityEXT, vFpUf)

// VK_EXT_private_data
GOM(vkCreatePrivateDataSlotEXT, iFEpppp)
GOM(vkDestroyPrivateDataSlotEXT, vFEpUp)
GO(vkGetPrivateDataEXT, vFpiUUp)
GO(vkSetPrivateDataEXT, iFpiUUU)

// VK_EXT_vertex_input_dynamic_state
GO(vkCmdSetVertexInputEXT, vFpupup)

// VK_HUAWEI_invocation_mask
GO(vkCmdBindInvocationMaskHUAWEI, vFpUi)

// VK_HUAWEI_subpass_shading
GO(vkCmdSubpassShadingHUAWEI, vFp)
GO(vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI, iFpUp)

// VK_INTEL_performance_query
GO(vkAcquirePerformanceConfigurationINTEL, iFppp)
GO(vkCmdSetPerformanceMarkerINTEL, iFpp)
GO(vkCmdSetPerformanceOverrideINTEL, iFpp)
GO(vkCmdSetPerformanceStreamMarkerINTEL, iFpp)
GO(vkGetPerformanceParameterINTEL, iFpip)
GO(vkInitializePerformanceApiINTEL, iFpp)
GO(vkQueueSetPerformanceConfigurationINTEL, iFpp)
GO(vkReleasePerformanceConfigurationINTEL, iFpU)
GO(vkUninitializePerformanceApiINTEL, vFp)

// VK_KHR_acceleration_structure
GO(vkBuildAccelerationStructuresKHR, iFpUupp)
GO(vkCmdBuildAccelerationStructuresIndirectKHR, vFpupppp)
GO(vkCmdBuildAccelerationStructuresKHR, vFpupp)
GO(vkCmdCopyAccelerationStructureKHR, vFpp)
GO(vkCmdCopyAccelerationStructureToMemoryKHR, vFpp)
GO(vkCmdCopyMemoryToAccelerationStructureKHR, vFpp)
GO(vkCmdWriteAccelerationStructuresPropertiesKHR, vFpupiUu)
GO(vkCopyAccelerationStructureKHR, iFpUp)
GO(vkCopyAccelerationStructureToMemoryKHR, iFpUp)
GO(vkCopyMemoryToAccelerationStructureKHR, iFpUp)
GOM(vkCreateAccelerationStructureKHR, iFEpppp)
GOM(vkDestroyAccelerationStructureKHR, vFEpUp)
GO(vkGetAccelerationStructureBuildSizesKHR, vFpippp)
GO(vkGetAccelerationStructureDeviceAddressKHR, UFpp)
GO(vkGetDeviceAccelerationStructureCompatibilityKHR, vFppp)
GO(vkWriteAccelerationStructuresPropertiesKHR, iFpupiLpL)

// VK_KHR_copy_commands2
GO(vkCmdBlitImage2KHR, vFpp)
GO(vkCmdCopyBuffer2KHR, vFpp)
GO(vkCmdCopyBufferToImage2KHR, vFpp)
GO(vkCmdCopyImage2KHR, vFpp)
GO(vkCmdCopyImageToBuffer2KHR, vFpp)
GO(vkCmdResolveImage2KHR, vFpp)

// VK_KHR_deferred_host_operations
GOM(vkCreateDeferredOperationKHR, iFEppp)
GO(vkDeferredOperationJoinKHR, iFpU)
GOM(vkDestroyDeferredOperationKHR, vFEpUp)
GO(vkGetDeferredOperationMaxConcurrencyKHR, uFpU)
GO(vkGetDeferredOperationResultKHR, iFpU)

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
GO(vkWaitForPresentKHR, iFpUUU)

// VK_KHR_present_wait2
GO(vkWaitForPresent2KHR, iFpUp)

// VK_KHR_ray_tracing_pipeline
GO(vkCmdSetRayTracingPipelineStackSizeKHR, vFpu)
GO(vkCmdTraceRaysIndirectKHR, vFpppppU)
GO(vkCmdTraceRaysKHR, vFpppppuuu)
GOM(vkCreateRayTracingPipelinesKHR, iFEpUUuppp)
GO(vkGetRayTracingCaptureReplayShaderGroupHandlesKHR, iFpUuuLp)
GO(vkGetRayTracingShaderGroupHandlesKHR, iFpUuuLp)
GO(vkGetRayTracingShaderGroupStackSizeKHR, UFpUui)

// VK_KHR_synchronization2
GO(vkCmdPipelineBarrier2KHR, vFpp)
GO(vkCmdResetEvent2KHR, vFpUU)
GO(vkCmdSetEvent2KHR, vFpUp)
GO(vkCmdWaitEvents2KHR, vFpupp)
GO(vkCmdWriteTimestamp2KHR, vFpUUu)
GO(vkQueueSubmit2KHR, iFpupU)
GO(vkCmdWriteBufferMarker2AMD, vFpUUUu)
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
GOM(vkDestroyCuFunctionNVX, vFEpUp)
GOM(vkDestroyCuModuleNVX, vFEpUp)

// VK_NVX_image_view_handle
GO(vkGetImageViewAddressNVX, iFpUp)
GO(vkGetImageViewHandle64NVX, UFpp)
GO(vkGetImageViewHandleNVX, uFpp)

// VK_NV_device_generated_commands
GO(vkCmdBindPipelineShaderGroupNV, vFpiUu)
GO(vkCmdExecuteGeneratedCommandsNV, vFpup)
GO(vkCmdPreprocessGeneratedCommandsNV, vFpp)
GOM(vkCreateIndirectCommandsLayoutNV, iFEpppp)
GOM(vkDestroyIndirectCommandsLayoutNV, vFEpUp)
GO(vkGetGeneratedCommandsMemoryRequirementsNV, vFppp)

// VK_NV_external_memory_rdma
GO(vkGetMemoryRemoteAddressNV, iFppp)

// VK_NV_fragment_shading_rate_enums
GO(vkCmdSetFragmentShadingRateEnumNV, vFpip)

// VK_NV_ray_tracing
GO(vkBindAccelerationStructureMemoryNV, iFpup)
GO(vkCmdBuildAccelerationStructureNV, vFppUUuUUUU)
GO(vkCmdCopyAccelerationStructureNV, vFpUUi)
GO(vkCmdTraceRaysNV, vFpUUUUUUUUUUUuuu)
GO(vkCmdWriteAccelerationStructuresPropertiesNV, vFpupiUu)
GO(vkCompileDeferredNV, iFpUu)
GOM(vkCreateAccelerationStructureNV, iFEpppp)
GOM(vkCreateRayTracingPipelinesNV, iFEpUuppp)
GOM(vkDestroyAccelerationStructureNV, vFEpUp)
GO(vkGetAccelerationStructureHandleNV, iFpULp)
GO(vkGetAccelerationStructureMemoryRequirementsNV, vFppp)
GO(vkGetRayTracingShaderGroupHandlesNV, iFpUuuLp)

// VK_EXT_acquire_xlib_display
GO(vkAcquireXlibDisplayEXT, iFppU)
GO(vkGetRandROutputDisplayEXT, iFpppp)

// VK_EXT_shader_module_identifier
GO(vkGetShaderModuleCreateInfoIdentifierEXT, vFppp)
GO(vkGetShaderModuleIdentifierEXT, vFpUp)

// VK_NV_optical_flow
GO(vkBindOpticalFlowSessionImageNV, iFpUiUi)
GO(vkCmdOpticalFlowExecuteNV, vFpUp)
GOM(vkCreateOpticalFlowSessionNV, iFEpppp)
GOM(vkDestroyOpticalFlowSessionNV, vFEpUp)
GO(vkGetPhysicalDeviceOpticalFlowImageFormatsNV, iFpppp)

// VK_EXT_extended_dynamic_state3
GO(vkCmdSetAlphaToCoverageEnableEXT, vFpu)
GO(vkCmdSetAlphaToOneEnableEXT, vFpu)
GO(vkCmdSetColorBlendAdvancedEXT, vFpuup)
GO(vkCmdSetColorBlendEnableEXT, vFpuup)
GO(vkCmdSetColorBlendEquationEXT, vFpuup)
GO(vkCmdSetColorWriteMaskEXT, vFpuup)
GO(vkCmdSetConservativeRasterizationModeEXT, vFpi)
GO(vkCmdSetCoverageModulationModeNV, vFpi)
GO(vkCmdSetCoverageModulationTableEnableNV, vFpu)
GO(vkCmdSetCoverageModulationTableNV, vFpup)
GO(vkCmdSetCoverageReductionModeNV, vFpi)
GO(vkCmdSetCoverageToColorEnableNV, vFpu)
GO(vkCmdSetCoverageToColorLocationNV, vFpu)
GO(vkCmdSetDepthClampEnableEXT, vFpu)
GO(vkCmdSetDepthClipEnableEXT, vFpu)
GO(vkCmdSetDepthClipNegativeOneToOneEXT, vFpu)
GO(vkCmdSetExtraPrimitiveOverestimationSizeEXT, vFpf)
GO(vkCmdSetLineRasterizationModeEXT, vFpi)
GO(vkCmdSetLineStippleEnableEXT, vFpu)
GO(vkCmdSetLogicOpEnableEXT, vFpu)
GO(vkCmdSetPolygonModeEXT, vFpi)
GO(vkCmdSetProvokingVertexModeEXT, vFpi)
GO(vkCmdSetRasterizationSamplesEXT, vFpi)
GO(vkCmdSetRasterizationStreamEXT, vFpu)
GO(vkCmdSetRepresentativeFragmentTestEnableNV, vFpu)
GO(vkCmdSetSampleLocationsEnableEXT, vFpu)
GO(vkCmdSetSampleMaskEXT, vFpip)
GO(vkCmdSetShadingRateImageEnableNV, vFpu)
GO(vkCmdSetTessellationDomainOriginEXT, vFpi)
GO(vkCmdSetViewportSwizzleNV, vFpuup)
GO(vkCmdSetViewportWScalingEnableNV, vFpu)

// VK_KHR_external_memory_win32
GO(vkGetMemoryWin32HandleKHR, iFppp)
GO(vkGetMemoryWin32HandlePropertiesKHR, iFpipp)

// VK_EXT_swapchain_maintenance1
GO(vkReleaseSwapchainImagesEXT, iFpp)

// VK_VALVE_descriptor_set_host_mapping
GO(vkGetDescriptorSetHostMappingVALVE, vFppp)
GO(vkGetDescriptorSetLayoutHostMappingInfoVALVE, vFppp)

// VK_EXT_descriptor_buffer
GO(vkCmdBindDescriptorBufferEmbeddedSamplersEXT, vFpiUu)
GO(vkCmdBindDescriptorBuffersEXT, vFpup)
GO(vkCmdSetDescriptorBufferOffsetsEXT, vFppUuupp)
GO(vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT, iFppp)
GO(vkGetBufferOpaqueCaptureDescriptorDataEXT, iFppp)
GO(vkGetDescriptorEXT, vFppLp)
GO(vkGetDescriptorSetLayoutBindingOffsetEXT, vFpUup)
GO(vkGetDescriptorSetLayoutSizeEXT, vFpUp)
GO(vkGetImageOpaqueCaptureDescriptorDataEXT, iFppp)
GO(vkGetImageViewOpaqueCaptureDescriptorDataEXT, iFppp)
GO(vkGetSamplerOpaqueCaptureDescriptorDataEXT, iFppp)

// VK_KHR_cooperative_matrix
GO(vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR, iFppp)

// VK_KHR_maintenance5
GO(vkCmdBindIndexBuffer2KHR, vFpUUUi)
GO(vkGetDeviceImageSubresourceLayoutKHR, vFppp)
GO(vkGetImageSubresourceLayout2KHR, vFpUpp)
GO(vkGetRenderingAreaGranularityKHR, vFppp)

// VK_NV_memory_decompression
GO(vkCmdDecompressMemoryIndirectCountNV, vFpUUu)
GO(vkCmdDecompressMemoryNV, vFpup)

// VK_EXT_merge_shader
GO(vkCmdDrawMeshTasksEXT, vFpuuu)
GO(vkCmdDrawMeshTasksIndirectCountEXT, vFpUUUUuu)
GO(vkCmdDrawMeshTasksIndirectEXT, vFpUUuu)

// VK_EXT_opacity_micromap
GO(vkBuildMicromapsEXT, iFppup)
GO(vkCmdBuildMicromapsEXT, vFpup)
GO(vkCmdCopyMemoryToMicromapEXT, vFpp)
GO(vkCmdCopyMicromapEXT, vFpp)
GO(vkCmdCopyMicromapToMemoryEXT, vFpp)
GO(vkCmdWriteMicromapsPropertiesEXT, vFpupiUu)
GO(vkCopyMemoryToMicromapEXT, iFpUp)
GO(vkCopyMicromapEXT, iFpUp)
GO(vkCopyMicromapToMemoryEXT, iFpUp)
GOM(vkCreateMicromapEXT, iFEpppp)
GOM(vkDestroyMicromapEXT, vFEpUp)
GO(vkGetDeviceMicromapCompatibilityEXT, vFppp)
GO(vkGetMicromapBuildSizesEXT, vFpipp)
GO(vkWriteMicromapsPropertiesEXT, iFpupiLpL)

// VK_KHR_ray_tracing_maintenance1
GO(vkCmdTraceRaysIndirect2KHR, vFpU)

// VK_EXT_depth_bias_control
GO(vkCmdSetDepthBias2EXT, vFpp)

// VK_EXT_host_image_copy
GO(vkCopyImageToImageEXT, iFpp)
GO(vkCopyImageToMemoryEXT, iFpp)
GO(vkCopyMemoryToImageEXT, iFpp)
GO(vkGetImageSubresourceLayout2EXT, vFpUpp)
GO(vkTransitionImageLayoutEXT, iFpup)

// VK_KHR_map_memory2
GO(vkMapMemory2KHR, iFppp)
GO(vkUnmapMemory2KHR, iFpp)

// VK_EXT_device_fault
GO(vkGetDeviceFaultInfoEXT, iFppp)

// VK_EXT_acquire_drm_display
GO(vkAcquireDrmDisplayEXT, iFpiU)
GO(vkGetDrmDisplayEXT, iFpiup)

// VK_EXT_shader_object
GO(vkCmdBindShadersEXT, vFpupp)
GOM(vkCreateShadersEXT, iFEpuppp)
GOM(vkDestroyShaderEXT, vFpUp)
GO(vkGetShaderBinaryDataEXT, iFpUpp)

// VK_NV_copy_memory_indirect
GO(vkCmdCopyMemoryIndirectNV, vFpUuu)
GO(vkCmdCopyMemoryToImageIndirectNV, vFpUuuUip)

// VK_HUAWEI_cluster_culling_shader
GO(vkCmdDrawClusterHUAWEI, vFpuuu)
GO(vkCmdDrawClusterIndirectHUAWEI, vFpUU)

// VK_EXT_attachment_feedback_loop_dynamic_state
GO(vkCmdSetAttachmentFeedbackLoopEnableEXT, vFpu)

// VK_NV_device_generated_commands_compute
GO(vkCmdUpdatePipelineIndirectBufferNV, vFpiU)
GO(vkGetPipelineIndirectDeviceAddressNV, UFpp)
GO(vkGetPipelineIndirectMemoryRequirementsNV, vFppp)

// VK_QCOM_tile_properties
GO(vkGetDynamicRenderingTilePropertiesQCOM, iFppp)
GO(vkGetFramebufferTilePropertiesQCOM, iFpUpp)

// VK_NV_external_memory_capabilities
GO(vkGetPhysicalDeviceExternalImageFormatPropertiesNV, iFpiiiuuup)

// VK_EXT_pipeline_properties
GO(vkGetPipelinePropertiesEXT, iFppp)

// VK_EXT_direct_mode_display
GO(vkReleaseDisplayEXT, iFpU)

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
GO(vkCmdCudaLaunchKernelNV, vFpp)
GOM(vkCreateCudaFunctionNV, iFEpppp)
GOM(vkCreateCudaModuleNV, iFEpppp)
GOM(vkDestroyCudaFunctionNV, vFEpUp)
GOM(vkDestroyCudaModuleNV, vFEpUp)
GO(vkGetCudaModuleCacheNV, iFpUpp)

// VK_NV_low_latency2
GO(vkGetLatencyTimingsNV, vFpUp)
GO(vkLatencySleepNV, iFpUp)
GO(vkQueueNotifyOutOfBandNV, vFpp)
GO(vkSetLatencyMarkerNV, vFpUp)
GO(vkSetLatencySleepModeNV, iFpUp)

// VK_AMDX_shader_enqueue
GO(vkCmdDispatchGraphAMDX, vFpUp)
GO(vkCmdDispatchGraphIndirectAMDX, vFpUp)
GO(vkCmdDispatchGraphIndirectCountAMDX, vFpUU)
GO(vkCmdInitializeGraphScratchMemoryAMDX, vFpU)
GOM(vkCreateExecutionGraphPipelinesAMDX, iFEpUuppp)
GO(vkGetExecutionGraphPipelineNodeIndexAMDX, iFpUpp)
GO(vkGetExecutionGraphPipelineScratchSizeAMDX, iFpUp)

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
GO(vkGetSemaphoreWin32HandleKHR, iFppp)
GO(vkImportSemaphoreWin32HandleKHR, iFpp)

// VK_KHR_win32_keyed_mutex
GO(wine_vkAcquireKeyedMutex, iFpUUu)
GO(wine_vkReleaseKeyedMutex, iFpUU)

// VK_NV_cooperative_matrix2
GO(vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV, iFppp)

// VK_AMD_anti_lag
GO(vkAntiLagUpdateAMD, vFpp)

// VK_EXT_device_generated_commands
GO(vkCmdExecuteGeneratedCommandsEXT, vFpup)
GO(vkCmdPreprocessGeneratedCommandsEXT, vFppp)
GOM(vkCreateIndirectCommandsLayoutEXT, iFEpppp)
GOM(vkCreateIndirectExecutionSetEXT, iFEpppp)
GOM(vkDestroyIndirectCommandsLayoutEXT, vFEpUp)
GOM(vkDestroyIndirectExecutionSetEXT, vFEpUp)
GO(vkGetGeneratedCommandsMemoryRequirementsEXT, vFppp)
GO(vkUpdateIndirectExecutionSetPipelineEXT, vFpUup)
GO(vkUpdateIndirectExecutionSetShaderEXT, vFpUup)

// VK_KHR_pipeline_binary
GOM(vkCreatePipelineBinariesKHR, iFEpppp)
GOM(vkDestroyPipelineBinaryKHR, vFEpUp)
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
GOM(vkDestroyTensorARM, vFEpUp)
GOM(vkDestroyTensorViewARM, vFEpUp)
GO(vkGetDeviceTensorMemoryRequirementsARM, vFppp)
GO(vkGetPhysicalDeviceExternalTensorPropertiesARM, vFppp)
GO(vkGetTensorMemoryRequirementsARM, vFppp)

// VK_EXT_descriptor_buffer + VK_ARM_tensors
GO(vkGetTensorOpaqueCaptureDescriptorDataARM, iFppp)
GO(vkGetTensorViewOpaqueCaptureDescriptorDataARM, iFppp)

// VK_ARM_data_graph
GO(vkBindDataGraphPipelineSessionMemoryARM, iFpup)
GO(vkCmdDispatchDataGraphARM, vFpUp)
GOM(vkCreateDataGraphPipelineSessionARM, iFEpppp)
GOM(vkCreateDataGraphPipelinesARM, iFEpUUuppp)
GOM(vkDestroyDataGraphPipelineSessionARM, vFEpUp)
GO(vkGetDataGraphPipelineAvailablePropertiesARM, iFpppp)
GO(vkGetDataGraphPipelinePropertiesARM, iFpppp)
GO(vkGetDataGraphPipelineSessionBindPointRequirementsARM, iFpppp)
GO(vkGetDataGraphPipelineSessionMemoryRequirementsARM, vFppp)
GO(vkGetPhysicalDeviceQueueFamilyDataGraphProcessingEnginePropertiesARM, vFppp)
GO(vkGetPhysicalDeviceQueueFamilyDataGraphPropertiesARM, iFpupp)

// VK_ARM_performance_counters_by_region
GO(vkEnumeratePhysicalDeviceQueueFamilyPerformanceCountersByRegionARM, iFpuppp)

// VK_EXT_descriptor_heap
GO(vkCmdBindResourceHeapEXT, iFpp)
GO(vkCmdBindSamplerHeapEXT, iFpp)
GO(vkCmdPushDataEXT, vFpp)
GO(vkGetImageOpaqueCaptureDataEXT, iFpupp)
GO(vkGetPhysicalDeviceDescriptorSizeEXT, UFpu)
GO(vkWriteResourceDescriptorsEXT, iFpupp)
GO(vkWriteSamplerDescriptorsEXT, iFpupp)

// VK_EXT_descriptor_heap + VK_ARM_tensors
GO(vkGetTensorOpaqueCaptureDataARM, iFpupp)

// VK_EXT_descriptor_heap + VK_EXT_custom_border_color
GO(vkRegisterCustomBorderColorEXT, iFppip)
GO(vkUnregisterCustomBorderColorEXT, vFpu)

// VK_EXT_custom_border_color
// no new functions

// VK_KHR_win32_surface
GOM(vkCreateWin32SurfaceKHR, iFEpppp)
GO(vkGetPhysicalDeviceWin32PresentationSupportKHR, iFpu)

// Layer stuffs
//GO(vkNegotiateLoaderLayerInterfaceVersion, iFp)   // structure needs wrapping
