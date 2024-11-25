#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GO(gst_dmabuf_allocator_alloc, pFpiL)
GO(gst_dmabuf_allocator_alloc_with_flags, pFpiLu)
GO(gst_dmabuf_allocator_get_type, LFv)
GO(gst_dmabuf_allocator_new, pFv)
GO(gst_dmabuf_memory_get_fd, iFp)
GO(gst_fd_allocator_alloc, pFpiLu)
GO(gst_fd_allocator_get_type, LFv)
GO(gst_fd_allocator_new, pFv)
GO(gst_fd_memory_get_fd, iFp)
GO(gst_is_dmabuf_memory, iFp)
GO(gst_is_fd_memory, iFp)
GO(gst_is_phys_memory, iFp)
GO(gst_phys_memory_allocator_get_type, LFv)
GO(gst_phys_memory_get_phys_addr, LFp)
