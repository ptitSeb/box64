// ----- GstObjectClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstObject, deep_notify, void, (void* object, void* origin, void* pspec), "ppp", object, origin, pspec);

#define SUPERGO() \
    GO(deep_notify, vFppp); \

// wrap (so bridge all calls, just in case)
static void wrapGstObjectClass(my_GstObjectClass_t* class)
{
    wrapGInitiallyUnownedClass(&class->parent);
    #define GO(A, W) class->A = reverse_##A##_GstObject (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstObjectClass(my_GstObjectClass_t* class)
{
    unwrapGInitiallyUnownedClass(&class->parent);
    #define GO(A, W)   class->A = find_##A##_GstObject (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstObjectClass(my_GstObjectClass_t* class)
{
    bridgeGInitiallyUnownedClass(&class->parent);
    #define GO(A, W) autobridge_##A##_GstObject (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGstObjectInstance(my_GstObject_t* class)
{
    unwrapGInitiallyUnownedInstance(&class->parent);
}
// autobridge
static void bridgeGstObjectInstance(my_GstObject_t* class)
{
    bridgeGInitiallyUnownedInstance(&class->parent);
}

// ----- GstAllocatorClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstAllocator, alloc, void*, (void *allocator, size_t size, void *params), "pLp", allocator, size, params);
WRAPPER(GstAllocator,free, void,    (void *allocator, void *memory), "pp", allocator, memory);

#define SUPERGO() \
    GO(alloc, pFpLp);       \
    GO(free, vFpp);         \

// wrap (so bridge all calls, just in case)
static void wrapGstAllocatorClass(my_GstAllocatorClass_t* class)
{
    wrapGstObjectClass(&class->parent);
    #define GO(A, W) class->A = reverse_##A##_GstAllocator (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstAllocatorClass(my_GstAllocatorClass_t* class)
{
    unwrapGstObjectClass(&class->parent);
    #define GO(A, W)   class->A = find_##A##_GstAllocator (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstAllocatorClass(my_GstAllocatorClass_t* class)
{
    bridgeGstObjectClass(&class->parent);
    #define GO(A, W) autobridge_##A##_GstAllocator (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

WRAPPER(GstAllocatorInstance, mem_map, void*,       (void* mem, size_t maxsize, int flags), "pLi", mem, maxsize, flags);
WRAPPER(GstAllocatorInstance, mem_unmap, void ,     (void* mem), "p", mem);
WRAPPER(GstAllocatorInstance, mem_copy, void*,      (void* mem, ssize_t offset, ssize_t size), "pll", mem, offset, size);
WRAPPER(GstAllocatorInstance, mem_share, void*,     (void* mem, ssize_t offset, ssize_t size), "pll", mem, offset, size);
WRAPPER(GstAllocatorInstance, mem_is_span, int  ,   (void* mem1, void* mem2, void* offset), "ppp", mem1, mem2, offset);
WRAPPER(GstAllocatorInstance, mem_map_full, void*,  (void* mem, void* info, size_t maxsize), "ppL", mem, info, maxsize);
WRAPPER(GstAllocatorInstance, mem_unmap_full, void ,(void* mem, void* info), "pp", mem, info);    

#define SUPERGO()               \
    GO(mem_map, pFpLi);         \
    GO(mem_unmap, vFp);         \
    GO(mem_copy, pFpll);        \
    GO(mem_share, pFpll);       \
    GO(mem_is_span, iFppp);     \
    GO(mem_map_full, pFppL);    \
    GO(mem_unmap_full, vFpp);   \

static void unwrapGstAllocatorInstance(my_GstAllocator_t* class)
{
    unwrapGstObjectInstance(&class->parent);
    #define GO(A, W)   class->A = find_##A##_GstAllocatorInstance (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstAllocatorInstance(my_GstAllocator_t* class)
{
    bridgeGstObjectInstance(&class->parent);
    #define GO(A, W) autobridge_##A##_GstAllocatorInstance (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

// ----- GstTaskPoolClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstTaskPool, prepare, void,  (void* pool, void* error), "pp", pool, error);
WRAPPER(GstTaskPool, cleanup, void,  (void* pool), "p", pool);
WRAPPER(GstTaskPool, push, void*,    (void* pool, void* func, void* user_data, void* error), "pppp", pool, AddCheckBridge(my_bridge, vFp, func, 0, NULL), user_data, error);
WRAPPER(GstTaskPool, join, void,     (void* pool, void* id), "pp", pool, id);
WRAPPER(GstTaskPool, dispose_handle, void, (void* pool, void* id), "pp", pool, id);

#define SUPERGO()               \
    GO(prepare, vFpp);          \
    GO(cleanup, vFp);           \
    GO(push, pFpppp);           \
    GO(join, vFpp);             \
    GO(dispose_handle, vFpp);   \

// wrap (so bridge all calls, just in case)
static void wrapGstTaskPoolClass(my_GstTaskPoolClass_t* class)
{
    wrapGstObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstTaskPool (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstTaskPoolClass(my_GstTaskPoolClass_t* class)
{
    unwrapGstObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstTaskPool (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstTaskPoolClass(my_GstTaskPoolClass_t* class)
{
    bridgeGstObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstTaskPool (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGstTaskPoolInstance(my_GstTaskPool_t* class)
{
    unwrapGstObjectInstance(&class->parent);
}
// autobridge
static void bridgeGstTaskPoolInstance(my_GstTaskPool_t* class)
{
    bridgeGstObjectInstance(&class->parent);
}

// ----- GstElementClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstElement,pad_added, void, (void* element, void* pad), "pp", element, pad);
WRAPPER(GstElement,pad_removed, void, (void* element, void* pad), "pp", element, pad);
WRAPPER(GstElement,no_more_pads, void, (void* element), "p", element);
WRAPPER(GstElement,request_new_pad, void*, (void* element, void* templ, void* name, void* caps), "pppp", element, templ, name, caps);
WRAPPER(GstElement,release_pad, void, (void* element, void* pad), "pp", element, pad);
WRAPPER(GstElement,get_state, int, (void*  element, void* state, void* pending, uint64_t timeout), "pppU", element, state, pending, timeout);
WRAPPER(GstElement,set_state, int, (void* element, int state), "pi", element, state);
WRAPPER(GstElement,change_state, int, (void* element, int transition), "pi", element, transition);
WRAPPER(GstElement,state_changed, void, (void* element, int oldstate, int newstate, int pending), "piii", element, oldstate, newstate, pending);
WRAPPER(GstElement,set_bus, void, (void*  element, void* bus), "pp", element, bus);
WRAPPER(GstElement,provide_clock, void*, (void* element), "p", element);
WRAPPER(GstElement,set_clock, int, (void* element, void* clock), "pp", element, clock);
WRAPPER(GstElement,send_event, int, (void* element, void* event), "pp", element, event);
WRAPPER(GstElement,query, int, (void* element, void* query), "pp", element, query);
WRAPPER(GstElement,post_message, int, (void* element, void* message), "pp", element, message);
WRAPPER(GstElement,set_context, void, (void* element, void* context), "pp", element, context);

#define SUPERGO()               \
    GO(pad_added, vFpp);        \
    GO(pad_removed, vFpp);      \
    GO(no_more_pads, vFp);      \
    GO(request_new_pad, pFpppp);\
    GO(release_pad, vFpp);      \
    GO(get_state, iFppU);       \
    GO(set_state, iFpi);        \
    GO(change_state, iFpi);     \
    GO(state_changed, vFpiii);  \
    GO(set_bus, vFpp);          \
    GO(provide_clock, pFp);     \
    GO(set_clock, iFpp);        \
    GO(send_event, iFpp);       \
    GO(query, iFpp);            \
    GO(post_message, iFpp);     \
    GO(set_context, vFpp);      \

// wrap (so bridge all calls, just in case)
static void wrapGstElementClass(my_GstElementClass_t* class)
{
    wrapGstObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstElement (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstElementClass(my_GstElementClass_t* class)
{
    unwrapGstObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstElement (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstElementClass(my_GstElementClass_t* class)
{
    bridgeGstObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstElement (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGstElementInstance(my_GstElement_t* class)
{
    unwrapGstObjectInstance(&class->parent);
}
// autobridge
static void bridgeGstElementInstance(my_GstElement_t* class)
{
    bridgeGstObjectInstance(&class->parent);
}
#undef SUPERGO

// ----- GstBinClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstBin,element_added, void, (void* bin, void* child), "pp", bin, child);
WRAPPER(GstBin,element_removed, void, (void* bin, void* child), "pp", bin, child);
WRAPPER(GstBin,add_element, int, (void* bin, void* element), "pp", bin, element);
WRAPPER(GstBin,remove_element, int, (void* bin, void* element), "pp", bin, element);
WRAPPER(GstBin,handle_message, void, (void* bin, void* message), "pp", bin, message);
WRAPPER(GstBin,do_latency, int, (void* bin), "p", bin);
WRAPPER(GstBin,deep_element_added, void, (void* bin, void* sub_bin, void* child), "ppp", bin, sub_bin, child);
WRAPPER(GstBin,deep_element_removed, void, (void* bin, void* sub_bin, void* child), "ppp", bin, sub_bin, child);

#define SUPERGO()                   \
    GO(element_added, vFpp);        \
    GO(element_removed, vFpp);      \
    GO(add_element, iFpp);          \
    GO(remove_element, iFpp);       \
    GO(handle_message, vFpp);       \
    GO(do_latency, iFp);            \
    GO(deep_element_added, vFppp);  \
    GO(deep_element_removed, vFppp);\

// wrap (so bridge all calls, just in case)
static void wrapGstBinClass(my_GstBinClass_t* class)
{
    wrapGstElementClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstBin (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstBinClass(my_GstBinClass_t* class)
{
    unwrapGstElementClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstBin (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstBinClass(my_GstBinClass_t* class)
{
    bridgeGstElementClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstBin (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGstBinInstance(my_GstBin_t* class)
{
    unwrapGstElementInstance(&class->parent);
}
// autobridge
static void bridgeGstBinInstance(my_GstBin_t* class)
{
    bridgeGstElementInstance(&class->parent);
}

// ----- GstBaseTransformClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstBaseTransform, transform_caps, void*, (void* trans, int direction, void* caps, void* filter), "pipp", trans, direction, caps, filter);
WRAPPER(GstBaseTransform, fixate_caps, void*, (void* trans, int direction, void* caps, void* othercaps), "pipp", trans, direction, caps, othercaps);
WRAPPER(GstBaseTransform, accept_caps, int, (void* trans, int direction, void* caps), "pip", trans, direction, caps);
WRAPPER(GstBaseTransform, set_caps, int, (void* trans, void* incaps, void* outcaps), "ppp", trans, incaps, outcaps);
WRAPPER(GstBaseTransform, query, int, (void* trans, int direction, void* query), "pip", trans, direction, query);
WRAPPER(GstBaseTransform, decide_allocation, int, (void* trans, void* query), "pp", trans, query );
WRAPPER(GstBaseTransform, filter_meta, int, (void* trans, void* query, size_t api, void* params), "ppLp", trans, query, api, params);
WRAPPER(GstBaseTransform, propose_allocation, int, (void* trans, void* decide_query, void* query), "ppp", trans, decide_query, query );
WRAPPER(GstBaseTransform, transform_size, int, (void* trans, int direction, void* caps, size_t size, void* othercaps, void* othersize), "pipLpp", trans, direction, caps, size, othercaps, othersize);
WRAPPER(GstBaseTransform, get_unit_size, int, (void* trans, void* caps, void* size), "ppp", trans, caps, size);
WRAPPER(GstBaseTransform, start, int, (void* trans), "p", trans);
WRAPPER(GstBaseTransform, stop, int, (void* trans), "p", trans);
WRAPPER(GstBaseTransform, sink_event, int, (void* trans, void* event), "pp", trans, event );
WRAPPER(GstBaseTransform, src_event, int, (void* trans, void* event), "pp", trans, event );
WRAPPER(GstBaseTransform, prepare_output_buffer, int, (void*  trans, void* input, void* outbuf), "ppp", trans, input, outbuf );
WRAPPER(GstBaseTransform, copy_metadata, int, (void* trans, void* input, void* outbuf), "ppp", trans, input, outbuf );
WRAPPER(GstBaseTransform, transform_meta, int, (void* trans, void* outbuf, void* meta, void* inbuf), "pppp", trans, outbuf, meta, inbuf );
WRAPPER(GstBaseTransform, before_transform, void, (void* trans, void* buffer), "pp", trans, buffer );
WRAPPER(GstBaseTransform, transform, int, (void* trans, void* inbuf, void* outbuf), "ppp", trans, inbuf, outbuf );
WRAPPER(GstBaseTransform, transform_ip, int, (void* trans, void* buf), "pp", trans, buf );
WRAPPER(GstBaseTransform, submit_input_buffer, int, (void* trans, int is_discont, void* input), "pip", trans, is_discont, input );
WRAPPER(GstBaseTransform, generate_output, int, (void* trans, void* outbuf), "pp", trans, outbuf );

#define SUPERGO()                       \
    GO(transform_caps, pFpipp);         \
    GO(fixate_caps, pFpipp);            \
    GO(accept_caps, iFpip);             \
    GO(set_caps, iFppp);                \
    GO(query, iFpip);                   \
    GO(decide_allocation, iFpp);        \
    GO(filter_meta, iFppLp);            \
    GO(propose_allocation, iFppp);      \
    GO(transform_size, iFpipLpp);       \
    GO(get_unit_size, iFppp);           \
    GO(start, iFp);                     \
    GO(stop, iFp);                      \
    GO(sink_event, iFpp);               \
    GO(src_event, iFpp);                \
    GO(prepare_output_buffer, iFppp);   \
    GO(copy_metadata, iFppp);           \
    GO(transform_meta, iFpppp);         \
    GO(before_transform, vFpp);         \
    GO(transform, iFppp);               \
    GO(transform_ip, iFpp);             \
    GO(submit_input_buffer, iFpip);     \
    GO(generate_output, iFpp);          \

// wrap (so bridge all calls, just in case)
static void wrapGstBaseTransformClass(my_GstBaseTransformClass_t* class)
{
    wrapGstElementClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstBaseTransform (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstBaseTransformClass(my_GstBaseTransformClass_t* class)
{
    unwrapGstElementClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstBaseTransform (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstBaseTransformClass(my_GstBaseTransformClass_t* class)
{
    bridgeGstElementClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstBaseTransform (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGstBaseTransformInstance(my_GstBaseTransform_t* class)
{
    unwrapGstElementInstance(&class->parent);
}
// autobridge
static void bridgeGstBaseTransformInstance(my_GstBaseTransform_t* class)
{
    bridgeGstElementInstance(&class->parent);
}
// ----- GstVideoDecoderClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstVideoDecoder, open, int, (void* decoder), "p", decoder);
WRAPPER(GstVideoDecoder, close, int, (void* decoder), "p", decoder);
WRAPPER(GstVideoDecoder, start, int, (void* decoder), "p", decoder);
WRAPPER(GstVideoDecoder, stop, int, (void* decoder), "p", decoder);
WRAPPER(GstVideoDecoder, parse, int, (void* decoder, void* frame, void* adapter, int at_eos), "pppi", decoder, frame, adapter, at_eos);
WRAPPER(GstVideoDecoder, set_format, int, (void* decoder, void* state), "pp", decoder, state);
WRAPPER(GstVideoDecoder, reset, int, (void* decoder, int hard), "pi", decoder, hard);
WRAPPER(GstVideoDecoder, finish, int, (void* decoder), "p", decoder);
WRAPPER(GstVideoDecoder, handle_frame, int, (void* decoder, void* frame), "pp", decoder, frame);
WRAPPER(GstVideoDecoder, sink_event, int, (void* decoder, void* event), "pp", decoder, event);
WRAPPER(GstVideoDecoder, src_event, int, (void* decoder, void* event), "pp", decoder, event);
WRAPPER(GstVideoDecoder, negotiate, int, (void* decoder), "p", decoder);
WRAPPER(GstVideoDecoder, decide_allocation, int, (void* decoder, void* query), "pp", decoder, query);
WRAPPER(GstVideoDecoder, propose_allocation, int, (void* decoder, void*  query), "pp", decoder, query);
WRAPPER(GstVideoDecoder, flush, int, (void* decoder), "p", decoder);
WRAPPER(GstVideoDecoder, sink_query, int, (void* decoder, void* query), "pp", decoder, query);
WRAPPER(GstVideoDecoder, src_query, int, (void* decoder, void* query), "pp", decoder, query);
WRAPPER(GstVideoDecoder, getcaps, void*, (void* decoder, void* filter), "pp", decoder, filter);
WRAPPER(GstVideoDecoder, drain, int, (void* decoder), "p", decoder);
WRAPPER(GstVideoDecoder, transform_meta, int, (void* decoder, void* frame, void* meta), "ppp", decoder, frame, meta);
WRAPPER(GstVideoDecoder, handle_missing_data, int, (void* decoder, uint64_t timestamp, uint64_t duration), "pUU", decoder, timestamp, duration);

#define SUPERGO()                       \
    GO(open, iFp);                      \
    GO(close, iFp);                     \
    GO(start, iFp);                     \
    GO(stop, iFp);                      \
    GO(parse, iFpppi);                  \
    GO(set_format, iFpp);               \
    GO(reset, iFp);                     \
    GO(finish, iFp);                    \
    GO(handle_frame, iFpp);             \
    GO(sink_event, iFpp);               \
    GO(src_event, iFpp);                \
    GO(negotiate, iFp);                 \
    GO(decide_allocation, iFpp);        \
    GO(propose_allocation, iFpp);       \
    GO(flush, iFp);                     \
    GO(sink_query, iFpp);               \
    GO(src_query, iFpp);                \
    GO(getcaps, pFpp);                  \
    GO(drain, iFp);                     \
    GO(transform_meta, iFppp);          \
    GO(handle_missing_data, iFpUU);     \

// wrap (so bridge all calls, just in case)
static void wrapGstVideoDecoderClass(my_GstVideoDecoderClass_t* class)
{
    wrapGstElementClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstVideoDecoder (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstVideoDecoderClass(my_GstVideoDecoderClass_t* class)
{
    unwrapGstElementClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstVideoDecoder (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstVideoDecoderClass(my_GstVideoDecoderClass_t* class)
{
    bridgeGstElementClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstVideoDecoder (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGstVideoDecoderInstance(my_GstVideoDecoder_t* class)
{
    unwrapGstElementInstance(&class->parent);
}
// autobridge
static void bridgeGstVideoDecoderInstance(my_GstVideoDecoder_t* class)
{
    bridgeGstElementInstance(&class->parent);
}
// ----- GstVideoEncoderClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstVideoEncoder, open, int, (void* encoder), "p", encoder);
WRAPPER(GstVideoEncoder, close, int, (void* encoder), "p", encoder);
WRAPPER(GstVideoEncoder, start, int, (void* encoder), "p", encoder);
WRAPPER(GstVideoEncoder, stop, int, (void* encoder), "p", encoder);
WRAPPER(GstVideoEncoder, set_format, int, (void* encoder, void* state), "pp", encoder, state);
WRAPPER(GstVideoEncoder, handle_frame, int, (void* encoder, void* frame), "pp", encoder, frame);
WRAPPER(GstVideoEncoder, reset, int, (void* encoder, int hard), "pi", encoder, hard);
WRAPPER(GstVideoEncoder, finish, int, (void* encoder), "p", encoder);
WRAPPER(GstVideoEncoder, pre_push, int, (void* encoder, void* frame), "pp", encoder, frame);
WRAPPER(GstVideoEncoder, getcaps, void*, (void* encoder, void* filter), "pp", encoder, filter);
WRAPPER(GstVideoEncoder, sink_event, int, (void* encoder, void* event), "pp", encoder, event);
WRAPPER(GstVideoEncoder, src_event, int, (void* encoder, void* event), "pp", encoder, event);
WRAPPER(GstVideoEncoder, negotiate, int, (void* encoder), "p", encoder);
WRAPPER(GstVideoEncoder, decide_allocation, int, (void* encoder, void* query), "pp", encoder, query);
WRAPPER(GstVideoEncoder, propose_allocation, int, (void* encoder, void*  query), "pp", encoder, query);
WRAPPER(GstVideoEncoder, flush, int, (void* encoder), "p", encoder);
WRAPPER(GstVideoEncoder, sink_query, int, (void* encoder, void* query), "pp", encoder, query);
WRAPPER(GstVideoEncoder, src_query, int, (void* encoder, void* query), "pp", encoder, query);
WRAPPER(GstVideoEncoder, transform_meta, int, (void* encoder, void* frame, void* meta), "ppp", encoder, frame, meta);

#define SUPERGO()                       \
    GO(open, iFp);                      \
    GO(close, iFp);                     \
    GO(start, iFp);                     \
    GO(stop, iFp);                      \
    GO(set_format, iFpp);               \
    GO(handle_frame, iFpp);             \
    GO(reset, iFp);                     \
    GO(finish, iFp);                    \
    GO(pre_push, iFpp);                 \
    GO(getcaps, pFpp);                  \
    GO(sink_event, iFpp);               \
    GO(src_event, iFpp);                \
    GO(negotiate, iFp);                 \
    GO(decide_allocation, iFpp);        \
    GO(propose_allocation, iFpp);       \
    GO(flush, iFp);                     \
    GO(sink_query, iFpp);               \
    GO(src_query, iFpp);                \
    GO(transform_meta, iFppp);          \

// wrap (so bridge all calls, just in case)
static void wrapGstVideoEncoderClass(my_GstVideoEncoderClass_t* class)
{
    wrapGstElementClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstVideoEncoder (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstVideoEncoderClass(my_GstVideoEncoderClass_t* class)
{
    unwrapGstElementClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstVideoEncoder (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstVideoEncoderClass(my_GstVideoEncoderClass_t* class)
{
    bridgeGstElementClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstVideoEncoder (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGstVideoEncoderInstance(my_GstVideoEncoder_t* class)
{
    unwrapGstElementInstance(&class->parent);
}
// autobridge
static void bridgeGstVideoEncoderInstance(my_GstVideoEncoder_t* class)
{
    bridgeGstElementInstance(&class->parent);
}
// ----- GstBaseSinkClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstBaseSink, get_caps, void*, (void* sink, void* filter), "pp", sink, filter);
WRAPPER(GstBaseSink, set_caps, int, (void* sink, void* caps), "pp", sink, caps);
WRAPPER(GstBaseSink, fixate, void* , (void* sink, void* caps), "pp", sink, caps);
WRAPPER(GstBaseSink, activate_pull, int, (void* sink, int active), "pi", sink, active);
WRAPPER(GstBaseSink, get_times, void, (void* sink, void* buffer, void* start, void* end), "pppp", sink, buffer, start, end);
WRAPPER(GstBaseSink, propose_allocation, int, (void* sink, void* query), "pp", sink, query);
WRAPPER(GstBaseSink, start, int, (void* sink), "p", sink);
WRAPPER(GstBaseSink, stop, int, (void* sink), "p", sink);
WRAPPER(GstBaseSink, unlock, int, (void* sink), "p", sink);
WRAPPER(GstBaseSink, unlock_stop, int, (void* sink), "p", sink);
WRAPPER(GstBaseSink, query, int, (void* sink, void* query), "pp", sink, query);
WRAPPER(GstBaseSink, event, int, (void* sink, void* event), "pp", sink, event);
WRAPPER(GstBaseSink, wait_event, int, (void* sink, void* event), "pp", sink, event);
WRAPPER(GstBaseSink, prepare, int, (void* sink, void* buffer), "pp", sink, buffer);
WRAPPER(GstBaseSink, prepare_list, int, (void* sink, void* buffer_list), "pp", sink, buffer_list);
WRAPPER(GstBaseSink, preroll, int, (void* sink, void* buffer), "pp", sink, buffer);
WRAPPER(GstBaseSink, render, int, (void* sink, void* buffer), "pp", sink, buffer);
WRAPPER(GstBaseSink, render_list, int, (void* sink, void* buffer_list), "pp", sink, buffer_list);

#define SUPERGO()                       \
    GO(get_caps, pFpp);                 \
    GO(set_caps, iFpp);                 \
    GO(fixate, pFpp);                   \
    GO(activate_pull, iFpi);            \
    GO(get_times, vFpppp);              \
    GO(propose_allocation, iFpp);       \
    GO(start, iFp);                     \
    GO(stop, iFp);                      \
    GO(unlock, iFp);                    \
    GO(unlock_stop, iFp);               \
    GO(query, iFpp);                    \
    GO(event, iFpp);                    \
    GO(wait_event, iFpp);               \
    GO(prepare, iFpp);                  \
    GO(prepare_list, iFpp);             \
    GO(preroll, iFpp);                  \
    GO(render, iFpp);                   \
    GO(render_list, iFpp);              \

// wrap (so bridge all calls, just in case)
static void wrapGstBaseSinkClass(my_GstBaseSinkClass_t* class)
{
    wrapGstElementClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstBaseSink (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstBaseSinkClass(my_GstBaseSinkClass_t* class)
{
    unwrapGstElementClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstBaseSink (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstBaseSinkClass(my_GstBaseSinkClass_t* class)
{
    bridgeGstElementClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstBaseSink (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstBaseSinkInstance(my_GstBaseSink_t* class)
{
    unwrapGstElementInstance(&class->parent);
}
// autobridge
static void bridgeGstBaseSinkInstance(my_GstBaseSink_t* class)
{
    bridgeGstElementInstance(&class->parent);
}
// ----- GstVideoSinkClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstVideoSink, show_frame, int, (void* video_sink, void* buf), "pp", video_sink, buf);
WRAPPER(GstVideoSink, set_info, int, (void* video_sink, void* caps, void* info), "ppp", video_sink, caps, info);

#define SUPERGO()                       \
    GO(show_frame, iFpp);               \
    GO(set_info, iFppp);                \

// wrap (so bridge all calls, just in case)
static void wrapGstVideoSinkClass(my_GstVideoSinkClass_t* class)
{
    wrapGstBaseSinkClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstVideoSink (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstVideoSinkClass(my_GstVideoSinkClass_t* class)
{
    unwrapGstBaseSinkClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstVideoSink (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstVideoSinkClass(my_GstVideoSinkClass_t* class)
{
    bridgeGstBaseSinkClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstVideoSink (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstVideoSinkInstance(my_GstVideoSink_t* class)
{
    unwrapGstBaseSinkInstance(&class->parent);
}
// autobridge
static void bridgeGstVideoSinkInstance(my_GstVideoSink_t* class)
{
    bridgeGstBaseSinkInstance(&class->parent);
}
// ----- GstGLBaseFilterClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstGLBaseFilter, gl_start, int, (void* filter), "p", filter);
WRAPPER(GstGLBaseFilter, gl_stop, void, (void* filter), "p", filter);
WRAPPER(GstGLBaseFilter, gl_set_caps, int, (void* filter, void* incaps, void* outcaps), "ppp", filter, incaps, outcaps);

#define SUPERGO()                       \
    GO(gl_start, iFp);                  \
    GO(gl_stop, vFp);                   \
    GO(gl_set_caps, iFppp);             \

// wrap (so bridge all calls, just in case)
static void wrapGstGLBaseFilterClass(my_GstGLBaseFilterClass_t* class)
{
    wrapGstBaseTransformClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstGLBaseFilter (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstGLBaseFilterClass(my_GstGLBaseFilterClass_t* class)
{
    unwrapGstBaseTransformClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstGLBaseFilter (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstGLBaseFilterClass(my_GstGLBaseFilterClass_t* class)
{
    bridgeGstBaseTransformClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstGLBaseFilter (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstGLBaseFilterInstance(my_GstGLBaseFilter_t* class)
{
    unwrapGstBaseTransformInstance(&class->parent);
}
// autobridge
static void bridgeGstGLBaseFilterInstance(my_GstGLBaseFilter_t* class)
{
    bridgeGstBaseTransformInstance(&class->parent);
}
// ----- GstGLFilterClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstGLFilter, set_caps, int, (void* filter, void* incaps, void* outcaps), "ppp", filter, incaps, outcaps);
WRAPPER(GstGLFilter, filter, int, (void* filter, void* inbuf, void* outbuf), "ppp", filter, inbuf, outbuf);
WRAPPER(GstGLFilter, filter_texture, int, (void* filter, void* input, void* output), "ppp", filter, input, output);
WRAPPER(GstGLFilter, init_fbo, int, (void* filter), "p", filter);
WRAPPER(GstGLFilter, transform_internal_caps, void*, (void* filter, int direction, void* caps, void* filter_caps), "pipp", filter, direction, caps, filter_caps);

#define SUPERGO()                       \
    GO(set_caps, iFppp);                \
    GO(filter, iFppp);                  \
    GO(filter_texture, iFppp);          \
    GO(init_fbo, iFp);                  \
    GO(transform_internal_caps, pFpipp);\

// wrap (so bridge all calls, just in case)
static void wrapGstGLFilterClass(my_GstGLFilterClass_t* class)
{
    wrapGstGLBaseFilterClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstGLFilter (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstGLFilterClass(my_GstGLFilterClass_t* class)
{
    unwrapGstGLBaseFilterClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstGLFilter (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstGLFilterClass(my_GstGLFilterClass_t* class)
{
    bridgeGstGLBaseFilterClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstGLFilter (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstGLFilterInstance(my_GstGLFilter_t* class)
{
    unwrapGstGLBaseFilterInstance(&class->parent);
}
// autobridge
static void bridgeGstGLFilterInstance(my_GstGLFilter_t* class)
{
    bridgeGstGLBaseFilterInstance(&class->parent);
}
// ----- GstAggregatorClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstAggregator, flush, int, (void* self), "p", self);
WRAPPER(GstAggregator, clip, void*, (void* self, void* aggregator_pad, void* buf), "ppp", self, aggregator_pad, buf);
WRAPPER(GstAggregator, finish_buffer, int, (void* self, void* buffer), "pp", self, buffer);
WRAPPER(GstAggregator, sink_event, int, (void* self, void* aggregator_pad, void* event), "ppp", self, aggregator_pad, event);
WRAPPER(GstAggregator, sink_query, int, (void* self, void* aggregator_pad, void* query), "ppp", self, aggregator_pad, query);
WRAPPER(GstAggregator, src_event, int, (void* self, void* event), "pp", self, event);
WRAPPER(GstAggregator, src_query, int, (void* self, void* query), "pp", self, query);
WRAPPER(GstAggregator, src_activate, int, (void* self, int mode, int active), "pii", self, mode, active);
WRAPPER(GstAggregator, aggregate, int, (void* self, int timeout), "pi", self, timeout);
WRAPPER(GstAggregator, stop, int, (void* self), "p", self);
WRAPPER(GstAggregator, start, int, (void* self), "p", self);
WRAPPER(GstAggregator, get_next_time, uint64_t, (void* self), "p", self);
WRAPPER(GstAggregator, create_new_pad, void*, (void* self, void* templ, void* req_name, void* caps), "pppp", self, templ, req_name, caps);
WRAPPER(GstAggregator, update_src_caps, int, (void* self, void* caps, void* ret), "ppp", self, caps, ret);
WRAPPER(GstAggregator, fixate_src_caps, void*, (void* self, void* caps), "pp", self, caps);
WRAPPER(GstAggregator, negotiated_src_caps, int, (void* self, void* caps), "pp", self, caps);
WRAPPER(GstAggregator, decide_allocation, int, (void* self, void* query), "pp", self, query);
WRAPPER(GstAggregator, propose_allocation, int, (void* self, void* pad, void* decide_query, void* query), "pppp", self, pad, decide_query, query);
WRAPPER(GstAggregator, negotiate, int, (void* self), "p", self);
WRAPPER(GstAggregator, sink_event_pre_queue, int, (void* self, void* aggregator_pad, void* event), "ppp", self, aggregator_pad, event);
WRAPPER(GstAggregator, sink_query_pre_queue, int, (void* self, void* aggregator_pad, void* query), "ppp", self, aggregator_pad, query);
WRAPPER(GstAggregator, finish_buffer_list, int, (void* self, void* bufferlist), "pp", self, bufferlist);
WRAPPER(GstAggregator, peek_next_sample, void, (void* self, void* aggregator_pad), "pp", self, aggregator_pad);

#define SUPERGO()                       \
    GO(flush, iFp);                     \
    GO(clip, pFppp);                    \
    GO(finish_buffer, iFpp);            \
    GO(sink_event, iFppp);              \
    GO(sink_query, iFppp);              \
    GO(src_event, iFpp);                \
    GO(src_query, iFpp);                \
    GO(src_activate, iFpii);            \
    GO(aggregate, iFpi);                \
    GO(stop, iFp);                      \
    GO(start, iFp);                     \
    GO(get_next_time, UFp);             \
    GO(create_new_pad, pFpppp);         \
    GO(update_src_caps, iFppp);         \
    GO(fixate_src_caps, pFpp);          \
    GO(negotiated_src_caps, iFpp);      \
    GO(decide_allocation, iFpp);        \
    GO(propose_allocation, iFpppp);     \
    GO(negotiate, iFp);                 \
    GO(sink_event_pre_queue, iFppp);    \
    GO(sink_query_pre_queue, iFppp);    \
    GO(finish_buffer_list, iFpp);       \
    GO(peek_next_sample, vFpp);         \

// wrap (so bridge all calls, just in case)
static void wrapGstAggregatorClass(my_GstAggregatorClass_t* class)
{
    wrapGstElementClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstAggregator (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstAggregatorClass(my_GstAggregatorClass_t* class)
{
    unwrapGstElementClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstAggregator (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstAggregatorClass(my_GstAggregatorClass_t* class)
{
    bridgeGstElementClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstAggregator (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstAggregatorInstance(my_GstAggregator_t* class)
{
    unwrapGstElementInstance(&class->parent);
}
// autobridge
static void bridgeGstAggregatorInstance(my_GstAggregator_t* class)
{
    bridgeGstElementInstance(&class->parent);
}
// ----- GstVideoAggregatorClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstVideoAggregator, update_caps, void*, (void* vagg, void* caps), "pp", vagg, caps);
WRAPPER(GstVideoAggregator, aggregate_frames, int, (void* vagg, void* outbuffer), "pp", vagg, outbuffer);
WRAPPER(GstVideoAggregator, create_output_buffer, int, (void* vagg, void* outbuffer), "pp", vagg, outbuffer);
WRAPPER(GstVideoAggregator, find_best_format, void, (void* vagg, void* downstream_caps, void* best_info, void* at_least_one_alpha), "pppp", vagg, downstream_caps, best_info, at_least_one_alpha);

#define SUPERGO()                       \
    GO(update_caps, pFpp);              \
    GO(aggregate_frames, iFpp);         \
    GO(create_output_buffer, iFpp);     \
    GO(find_best_format, vFpppp);       \

// wrap (so bridge all calls, just in case)
static void wrapGstVideoAggregatorClass(my_GstVideoAggregatorClass_t* class)
{
    wrapGstAggregatorClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstVideoAggregator (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstVideoAggregatorClass(my_GstVideoAggregatorClass_t* class)
{
    unwrapGstAggregatorClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstVideoAggregator (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstVideoAggregatorClass(my_GstVideoAggregatorClass_t* class)
{
    bridgeGstAggregatorClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstVideoAggregator (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstVideoAggregatorInstance(my_GstVideoAggregator_t* class)
{
    unwrapGstAggregatorInstance(&class->aggregator);
}
// autobridge
static void bridgeGstVideoAggregatorInstance(my_GstVideoAggregator_t* class)
{
    bridgeGstAggregatorInstance(&class->aggregator);
}
// ----- GstPadClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstPad, linked, void, (void* pad, void* peer), "pp", pad, peer);
WRAPPER(GstPad, unlinked, void, (void* pad, void* peer), "pp", pad, peer);

#define SUPERGO()               \
    GO(linked, vFpp);           \
    GO(unlinked, vFpp);         \

// wrap (so bridge all calls, just in case)
static void wrapGstPadClass(my_GstPadClass_t* class)
{
    wrapGstObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstPad (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstPadClass(my_GstPadClass_t* class)
{
    unwrapGstObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstPad (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstPadClass(my_GstPadClass_t* class)
{
    bridgeGstObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstPad (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO
WRAPPER(GstPadInstance, activatefunc, int    , (void* pad, void* parent), "pp", pad, parent);
WRAPPER(GstPadInstance, activatenotify, void   , (void* a), "p", a);
WRAPPER(GstPadInstance, activatemodefunc, int    , (void* pad, void* parent, int mode, int active), "ppii", pad, parent, mode, active);
WRAPPER(GstPadInstance, activatemodenotify, void   , (void* a), "p", a);
WRAPPER(GstPadInstance, linkfunc, int    , (void* pad, void* parent, void* peer), "ppp", pad, parent, peer);
WRAPPER(GstPadInstance, linknotify, void   , (void* a), "p", a);
WRAPPER(GstPadInstance, unlinkfunc, void   , (void* pad, void* parent), "pp", pad, parent);
WRAPPER(GstPadInstance, unlinknotify, void   , (void* a), "p", a);
WRAPPER(GstPadInstance, chainfunc, int    , (void* pad, void* parent, void* buffer), "ppp", pad, parent, buffer);
WRAPPER(GstPadInstance, chainnotify, void   , (void* a), "p", a);
WRAPPER(GstPadInstance, chainlistfunc, int    , (void* pad, void* parent, void* list), "ppp", pad, parent, list);
WRAPPER(GstPadInstance, chainlistnotify, void   , (void* a), "p", a);
WRAPPER(GstPadInstance, getrangefunc, int    , (void* pad, void* parent, uint64_t offset, uint32_t length, void* buffer), "ppUup", pad, parent, offset, length, buffer);
WRAPPER(GstPadInstance, getrangenotify, void   , (void* a), "p", a);
WRAPPER(GstPadInstance, eventfunc, int    , (void* pad, void* parent, void* event), "ppp", pad, parent, event);
WRAPPER(GstPadInstance, eventnotify, void   , (void* a), "p", a);
WRAPPER(GstPadInstance, queryfunc, int    , (void* pad, void* parent, void* query), "ppp", pad, parent, query);
WRAPPER(GstPadInstance, querynotify, void   , (void* a), "p", a);
WRAPPER(GstPadInstance, iterintlinkfunc, void*  , (void* pad, void* parent), "pp", pad, parent);
WRAPPER(GstPadInstance, iterintlinknotify, void   , (void* a), "p", a);
WRAPPER(GstPadInstance, finalize_hook, void   , (void* hook_list, void* hook), "pp", hook_list, hook);
WRAPPER(GstPadInstance, eventfullfunc, int    , (void* pad, void* parent, void* event), "ppp", pad, parent, event);

#define SUPERGO()                       \
    GO(activatefunc, iFpp);             \
    GO(activatenotify, vFp);            \
    GO(activatemodefunc, iFppii);       \
    GO(activatemodenotify, vFp);        \
    GO(linkfunc, iFppp);                \
    GO(linknotify, vFp);                \
    GO(unlinkfunc, vFpp);               \
    GO(unlinknotify, vFp);              \
    GO(chainfunc, iFppp);               \
    GO(chainnotify, vFp);               \
    GO(chainlistfunc, iFppp);           \
    GO(chainlistnotify, vFp);           \
    GO(getrangefunc, iFppUup);          \
    GO(getrangenotify, vFp);            \
    GO(eventfunc, iFppp);               \
    GO(eventnotify, vFp);               \
    GO(queryfunc, iFppp);               \
    GO(querynotify, vFp);               \
    GO(iterintlinkfunc, pFpp);          \
    GO(iterintlinknotify, vFp);         \
    GO2(probes, finalize_hook, vFpp);   \
    GO2(ABI.abi, eventfullfunc, vFppp); \

static void unwrapGstPadInstance(my_GstPad_t* class)
{
    unwrapGstObjectInstance(&class->parent);
    #define GO(A, W)   class->A = find_##A##_GstPadInstance (W, class->A)
    #define GO2(B, A, W)   class->B.A = find_##A##_GstPadInstance (W, class->B.A)
    SUPERGO()
    #undef GO2
    #undef GO
}
// autobridge
static void bridgeGstPadInstance(my_GstPad_t* class)
{
    bridgeGstObjectInstance(&class->parent);
    #define GO(A, W) autobridge_##A##_GstPadInstance (W, class->A)
    #define GO2(B, A, W) autobridge_##A##_GstPadInstance (W, class->B.A)
    SUPERGO()
    #undef GO2
    #undef GO
}
#undef SUPERGO
// ----- GstAggregatorPadClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstAggregatorPad, flush, int, (void* aggpad, void* aggregator), "pp", aggpad, aggregator);
WRAPPER(GstAggregatorPad, skip_buffer, int, (void* aggpad, void* aggregator, void* buffer), "ppp", aggpad, aggregator, buffer);

#define SUPERGO()                       \
    GO(flush, iFpp);                    \
    GO(skip_buffer, iFppp);             \

// wrap (so bridge all calls, just in case)
static void wrapGstAggregatorPadClass(my_GstAggregatorPadClass_t* class)
{
    wrapGstPadClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstAggregatorPad (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstAggregatorPadClass(my_GstAggregatorPadClass_t* class)
{
    unwrapGstPadClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstAggregatorPad (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstAggregatorPadClass(my_GstAggregatorPadClass_t* class)
{
    bridgeGstPadClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstAggregatorPad (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstAggregatorPadInstance(my_GstAggregatorPad_t* class)
{
    unwrapGstPadInstance(&class->parent);
}
// autobridge
static void bridgeGstAggregatorPadInstance(my_GstAggregatorPad_t* class)
{
    bridgeGstPadInstance(&class->parent);
}
// ----- GstVideoAggregatorPadClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstVideoAggregatorPad, update_conversion_info, void, (void* pad), "p", pad);
WRAPPER(GstVideoAggregatorPad, prepare_frame, int, (void* pad, void* vagg, void* buffer, void* prepared_frame), "pppp", pad, vagg, buffer, prepared_frame);
WRAPPER(GstVideoAggregatorPad, clean_frame, void, (void* pad, void* vagg, void* prepared_frame), "ppp", pad, vagg, prepared_frame);
WRAPPER(GstVideoAggregatorPad, prepare_frame_start, void, (void* pad, void* vagg, void* buffer, void* prepared_frame), "pppp", pad, vagg, buffer, prepared_frame);
WRAPPER(GstVideoAggregatorPad, prepare_frame_finish, void, (void* pad, void* vagg, void* prepared_frame), "ppp", pad, vagg, prepared_frame);

#define SUPERGO()                       \
    GO(update_conversion_info, vFp);    \
    GO(prepare_frame, iFpppp);          \
    GO(clean_frame, vFppp);             \
    GO(prepare_frame_start, vFpppp);    \
    GO(prepare_frame_finish, vFppp);    \

// wrap (so bridge all calls, just in case)
static void wrapGstVideoAggregatorPadClass(my_GstVideoAggregatorPadClass_t* class)
{
    wrapGstAggregatorPadClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstVideoAggregatorPad (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstVideoAggregatorPadClass(my_GstVideoAggregatorPadClass_t* class)
{
    unwrapGstAggregatorPadClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstVideoAggregatorPad (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstVideoAggregatorPadClass(my_GstVideoAggregatorPadClass_t* class)
{
    bridgeGstAggregatorPadClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstVideoAggregatorPad (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstVideoAggregatorPadInstance(my_GstVideoAggregatorPad_t* class)
{
    unwrapGstAggregatorPadInstance(&class->parent);
}
// autobridge
static void bridgeGstVideoAggregatorPadInstance(my_GstVideoAggregatorPad_t* class)
{
    bridgeGstAggregatorPadInstance(&class->parent);
}
#undef SUPERGO
// ----- GstBaseSrcClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstBaseSrc, get_caps, void*, (void* src, void* filter), "pp", src, filter);
WRAPPER(GstBaseSrc, negotiate, int, (void* src), "p", src);
WRAPPER(GstBaseSrc, fixate, void*, (void* src, void* caps), "pp", src, caps);
WRAPPER(GstBaseSrc, set_caps, int, (void* src, void* caps), "pp", src, caps);
WRAPPER(GstBaseSrc, decide_allocation, int, (void* src, void* query), "pp", src, query);
WRAPPER(GstBaseSrc, start, int, (void* src), "p", src);
WRAPPER(GstBaseSrc, stop, int, (void* src), "p", src);
WRAPPER(GstBaseSrc, get_times, void , (void* src, void* buffer, void* start, void* end), "pppp", src, buffer, start, end);
WRAPPER(GstBaseSrc, get_size, int, (void* src, void* size), "pp", src, size);
WRAPPER(GstBaseSrc, is_seekable, int, (void* src), "p", src);
WRAPPER(GstBaseSrc, prepare_seek_segment, int, (void* src, void* seek, void* segment), "ppp", src, seek, segment);
WRAPPER(GstBaseSrc, do_seek, int, (void* src, void* segment), "pp", src, segment);
WRAPPER(GstBaseSrc, unlock, int, (void* src), "p", src);
WRAPPER(GstBaseSrc, unlock_stop, int, (void* src), "p", src);
WRAPPER(GstBaseSrc, query, int, (void* src, void* query), "pp", src, query);
WRAPPER(GstBaseSrc, event, int, (void* src, void* event), "pp", src, event);
WRAPPER(GstBaseSrc, create, int, (void* src, uint64_t offset, uint32_t size, void* buf), "pUup", src, offset, size, buf);
WRAPPER(GstBaseSrc, alloc, int, (void* src, uint64_t offset, uint32_t size, void* buf), "pUup", src, offset, size, buf);
WRAPPER(GstBaseSrc, fill, int, (void* src, uint64_t offset, uint32_t size, void* buf), "pUup", src, offset, size, buf);

#define SUPERGO()                       \
    GO(get_caps, pFpp);                 \
    GO(negotiate, iFp);                 \
    GO(fixate, pFpp);                   \
    GO(set_caps, iFpp);                 \
    GO(decide_allocation, iFpp);        \
    GO(start, iFp);                     \
    GO(stop, iFp);                      \
    GO(get_times, vFpppp);              \
    GO(get_size, iFpp);                 \
    GO(is_seekable, iFp);               \
    GO(prepare_seek_segment, iFppp);    \
    GO(do_seek, iFpp);                  \
    GO(unlock, iFp);                    \
    GO(unlock_stop, iFp);               \
    GO(query, iFpp);                    \
    GO(event, iFpp);                    \
    GO(create, iFpUup);                 \
    GO(alloc, iFpUup);                  \
    GO(fill, iFpUup);                   \

// wrap (so bridge all calls, just in case)
static void wrapGstBaseSrcClass(my_GstBaseSrcClass_t* class)
{
    wrapGstElementClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstBaseSrc (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstBaseSrcClass(my_GstBaseSrcClass_t* class)
{
    unwrapGstElementClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstBaseSrc (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstBaseSrcClass(my_GstBaseSrcClass_t* class)
{
    bridgeGstElementClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstBaseSrc (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstBaseSrcInstance(my_GstBaseSrc_t* class)
{
    unwrapGstElementInstance(&class->parent);
}
// autobridge
static void bridgeGstBaseSrcInstance(my_GstBaseSrc_t* class)
{
    bridgeGstElementInstance(&class->parent);
}
// ----- GstPushSrcClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstPushSrc, create, int, (void* src, void* buf), "pp", src, buf);
WRAPPER(GstPushSrc, alloc, int, (void* src, void* buf), "pp", src, buf);
WRAPPER(GstPushSrc, fill, int, (void* src, void* buf), "pp", src, buf);

#define SUPERGO()               \
    GO(create, iFpp);           \
    GO(alloc, iFpp);            \
    GO(fill, iFpp);             \

// wrap (so bridge all calls, just in case)
static void wrapGstPushSrcClass(my_GstPushSrcClass_t* class)
{
    wrapGstBaseSrcClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstPushSrc (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstPushSrcClass(my_GstPushSrcClass_t* class)
{
    unwrapGstBaseSrcClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstPushSrc (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstPushSrcClass(my_GstPushSrcClass_t* class)
{
    bridgeGstBaseSrcClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstPushSrc (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstPushSrcInstance(my_GstPushSrc_t* class)
{
    unwrapGstBaseSrcInstance(&class->parent);
}
// autobridge
static void bridgeGstPushSrcInstance(my_GstPushSrc_t* class)
{
    bridgeGstBaseSrcInstance(&class->parent);
}
// ----- GstGLBaseSrcClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstGLBaseSrc, gl_start, int, (void* src), "p", src);
WRAPPER(GstGLBaseSrc, gl_stop, void, (void* src), "p", src);
WRAPPER(GstGLBaseSrc, fill_gl_memory, int, (void* src, void* mem), "pp", src, mem);

#define SUPERGO()               \
    GO(gl_start, iFp);          \
    GO(gl_stop, vFp);           \
    GO(fill_gl_memory, iFpp);   \

// wrap (so bridge all calls, just in case)
static void wrapGstGLBaseSrcClass(my_GstGLBaseSrcClass_t* class)
{
    wrapGstPushSrcClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstGLBaseSrc (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstGLBaseSrcClass(my_GstGLBaseSrcClass_t* class)
{
    unwrapGstPushSrcClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstGLBaseSrc (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstGLBaseSrcClass(my_GstGLBaseSrcClass_t* class)
{
    bridgeGstPushSrcClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstGLBaseSrc (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstGLBaseSrcInstance(my_GstGLBaseSrc_t* class)
{
    unwrapGstPushSrcInstance(&class->parent);
}
// autobridge
static void bridgeGstGLBaseSrcInstance(my_GstGLBaseSrc_t* class)
{
    bridgeGstPushSrcInstance(&class->parent);
}
// ----- GstAudioDecoderClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstAudioDecoder, start, int,(void* dec), "p", dec);
WRAPPER(GstAudioDecoder, stop, int,(void* dec), "p", dec);
WRAPPER(GstAudioDecoder, set_format, int,(void* dec, void* caps), "pp", dec, caps);
WRAPPER(GstAudioDecoder, parse, int,(void* dec, void* adapter, void* offset, void* length), "pppp", dec, adapter, offset, length);
WRAPPER(GstAudioDecoder, handle_frame, int,(void* dec, void* buffer), "pp", dec, buffer);
WRAPPER(GstAudioDecoder, flush, void ,(void* dec, int hard), "pi", dec, hard);
WRAPPER(GstAudioDecoder, pre_push, int,(void* dec, void* buffer), "pp", dec, buffer);
WRAPPER(GstAudioDecoder, sink_event, int,(void* dec, void* event), "pp", dec, event);
WRAPPER(GstAudioDecoder, src_event, int,(void* dec, void* event), "pp", dec, event);
WRAPPER(GstAudioDecoder, open, int,(void* dec), "p", dec);
WRAPPER(GstAudioDecoder, close, int,(void* dec), "p", dec);
WRAPPER(GstAudioDecoder, negotiate, int,(void* dec), "p", dec);
WRAPPER(GstAudioDecoder, decide_allocation, int,(void* dec, void* query), "pp", dec, query);
WRAPPER(GstAudioDecoder, propose_allocation, int,(void* dec, void* query), "pp", dec, query);
WRAPPER(GstAudioDecoder, sink_query, int,(void* dec, void* query), "pp", dec, query);
WRAPPER(GstAudioDecoder, src_query, int,(void* dec, void* query), "pp", dec, query);
WRAPPER(GstAudioDecoder, getcaps, void*,(void* dec, void*  filter), "pp", dec, filter);
WRAPPER(GstAudioDecoder, transform_meta, int,(void* enc, void* outbuf, void* meta, void* inbuf), "pppp", enc, outbuf, meta, inbuf);

#define SUPERGO()                       \
    GO(start, iFp);                     \
    GO(stop, iFp);                      \
    GO(set_format, iFpp);               \
    GO(parse, iFpppp);                  \
    GO(handle_frame, iFpp);             \
    GO(flush, vFpi);                    \
    GO(pre_push, iFpp);                 \
    GO(sink_event, iFpp);               \
    GO(src_event, iFpp);                \
    GO(open, iFp);                      \
    GO(close, iFp);                     \
    GO(negotiate, iFp);                 \
    GO(decide_allocation, iFpp);        \
    GO(propose_allocation, iFpp);       \
    GO(sink_query, iFpp);               \
    GO(src_query, iFpp);                \
    GO(getcaps, vFpp);                  \
    GO(transform_meta, iFpppp);         \

// wrap (so bridge all calls, just in case)
static void wrapGstAudioDecoderClass(my_GstAudioDecoderClass_t* class)
{
    wrapGstElementClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstAudioDecoder (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstAudioDecoderClass(my_GstAudioDecoderClass_t* class)
{
    unwrapGstElementClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstAudioDecoder (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstAudioDecoderClass(my_GstAudioDecoderClass_t* class)
{
    bridgeGstElementClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstAudioDecoder (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstAudioDecoderInstance(my_GstAudioDecoder_t* class)
{
    unwrapGstElementInstance(&class->parent);
}
// autobridge
static void bridgeGstAudioDecoderInstance(my_GstAudioDecoder_t* class)
{
    bridgeGstElementInstance(&class->parent);
}
// ----- GstAudioEncoderClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstAudioEncoder, start, int, (void* enc), "p", enc);
WRAPPER(GstAudioEncoder, stop, int, (void* enc), "p", enc);
WRAPPER(GstAudioEncoder, set_format, int, (void* enc, void* info), "pp", enc, info);
WRAPPER(GstAudioEncoder, handle_frame, int, (void* enc, void* buffer), "pp", enc, buffer);
WRAPPER(GstAudioEncoder, flush, void, (void* enc), "p", enc);
WRAPPER(GstAudioEncoder, pre_push, int, (void* enc, void* *buffer), "pp", enc, buffer);
WRAPPER(GstAudioEncoder, sink_event, int, (void* enc, void* event), "pp", enc, event);
WRAPPER(GstAudioEncoder, src_event, int, (void* enc, void* event), "pp", enc, event);
WRAPPER(GstAudioEncoder, getcaps, void, (void* enc, void* filter), "pp", enc, filter);
WRAPPER(GstAudioEncoder, open, int, (void* enc), "p", enc);
WRAPPER(GstAudioEncoder, close, int, (void* enc), "p", enc);
WRAPPER(GstAudioEncoder, negotiate, int, (void* enc), "p", enc);
WRAPPER(GstAudioEncoder, decide_allocation, int, (void* enc, void* query), "pp", enc, query);
WRAPPER(GstAudioEncoder, propose_allocation, int, (void* enc, void*  query), "pp", enc, query);
WRAPPER(GstAudioEncoder, transform_meta, int, (void* enc, void* outbuf, void* meta, void* inbuf), "pppp", enc, outbuf, meta, inbuf);
WRAPPER(GstAudioEncoder, sink_query, int, (void* enc, void* query), "pp", enc, query);
WRAPPER(GstAudioEncoder, src_query, int, (void* enc, void* query), "pp", enc, query);

#define SUPERGO()                       \
    GO(start, iFp);                     \
    GO(stop, iFp);                      \
    GO(set_format, iFpp);               \
    GO(handle_frame, iFpp);             \
    GO(flush, vFpi);                    \
    GO(pre_push, iFpp);                 \
    GO(sink_event, iFpp);               \
    GO(src_event, iFpp);                \
    GO(getcaps, vFpp);                  \
    GO(open, iFp);                      \
    GO(close, iFp);                     \
    GO(negotiate, iFp);                 \
    GO(decide_allocation, iFpp);        \
    GO(propose_allocation, iFpp);       \
    GO(transform_meta, iFpppp);         \
    GO(sink_query, iFpp);               \
    GO(src_query, iFpp);                \

// wrap (so bridge all calls, just in case)
static void wrapGstAudioEncoderClass(my_GstAudioEncoderClass_t* class)
{
    wrapGstElementClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstAudioEncoder (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstAudioEncoderClass(my_GstAudioEncoderClass_t* class)
{
    unwrapGstElementClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstAudioEncoder (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstAudioEncoderClass(my_GstAudioEncoderClass_t* class)
{
    bridgeGstElementClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstAudioEncoder (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstAudioEncoderInstance(my_GstAudioEncoder_t* class)
{
    unwrapGstElementInstance(&class->parent);
}
// autobridge
static void bridgeGstAudioEncoderInstance(my_GstAudioEncoder_t* class)
{
    bridgeGstElementInstance(&class->parent);
}
// ----- GstVideoFilterClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstVideoFilter, set_info, int, (void* filter, void* incaps, void* in_info, void* outcaps, void* out_info), "ppppp", filter, incaps, in_info, outcaps, out_info);
WRAPPER(GstVideoFilter, transform_frame, int, (void* filter, void* inframe, void* outframe), "ppp", filter, inframe, outframe);
WRAPPER(GstVideoFilter, transform_frame_ip, int, (void* filter, void* frame), "pp", filter, frame);

#define SUPERGO()                       \
    GO(set_info, iFppppp);              \
    GO(transform_frame, iFppp);         \
    GO(transform_frame_ip, iFpp);       \

// wrap (so bridge all calls, just in case)
static void wrapGstVideoFilterClass(my_GstVideoFilterClass_t* class)
{
    wrapGstBaseTransformClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstVideoFilter (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstVideoFilterClass(my_GstVideoFilterClass_t* class)
{
    unwrapGstBaseTransformClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstVideoFilter (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstVideoFilterClass(my_GstVideoFilterClass_t* class)
{
    bridgeGstBaseTransformClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstVideoFilter (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstVideoFilterInstance(my_GstVideoFilter_t* class)
{
    unwrapGstBaseTransformInstance(&class->parent);
}
// autobridge
static void bridgeGstVideoFilterInstance(my_GstVideoFilter_t* class)
{
    bridgeGstBaseTransformInstance(&class->parent);
}
// ----- GstAudioFilterClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstAudioFilter, setup, int, (void* filter, void* info), "pp", filter, info);

#define SUPERGO()       \
    GO(setup, iFpp);    \

// wrap (so bridge all calls, just in case)
static void wrapGstAudioFilterClass(my_GstAudioFilterClass_t* class)
{
    wrapGstBaseTransformClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstAudioFilter (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstAudioFilterClass(my_GstAudioFilterClass_t* class)
{
    unwrapGstBaseTransformClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstAudioFilter (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstAudioFilterClass(my_GstAudioFilterClass_t* class)
{
    bridgeGstBaseTransformClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstAudioFilter (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstAudioFilterInstance(my_GstAudioFilter_t* class)
{
    unwrapGstBaseTransformInstance(&class->parent);
}
// autobridge
static void bridgeGstAudioFilterInstance(my_GstAudioFilter_t* class)
{
    bridgeGstBaseTransformInstance(&class->parent);
}
// ----- GstBufferPoolClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstBufferPool, get_options, void*,(void* pool), "p", pool);
WRAPPER(GstBufferPool, set_config, int ,(void* pool, void* config), "pp", pool, config);
WRAPPER(GstBufferPool, start, int ,(void* pool), "p", pool);
WRAPPER(GstBufferPool, stop, int ,(void* pool), "p", pool);
WRAPPER(GstBufferPool, acquire_buffer, int ,(void* pool, void* buffer, void* params), "ppp", pool, buffer, params);
WRAPPER(GstBufferPool, alloc_buffer, int ,(void* pool, void* buffer, void* params), "ppp", pool, buffer, params);
WRAPPER(GstBufferPool, reset_buffer, void ,(void* pool, void* buffer), "pp", pool, buffer);
WRAPPER(GstBufferPool, release_buffer, void ,(void* pool, void* buffer), "pp", pool, buffer);
WRAPPER(GstBufferPool, free_buffer, void ,(void* pool, void* buffer), "pp", pool, buffer);
WRAPPER(GstBufferPool, flush_start, void ,(void* pool), "p", pool);
WRAPPER(GstBufferPool, flush_stop, void ,(void* pool), "p", pool);

#define SUPERGO()               \
    GO(get_options, pFp);       \
    GO(set_config, iFpp);       \
    GO(start, iFp);             \
    GO(stop, iFp);              \
    GO(acquire_buffer, iFppp);  \
    GO(alloc_buffer, iFppp);    \
    GO(reset_buffer, vFpp);     \
    GO(release_buffer, vFpp);   \
    GO(free_buffer, vFpp);      \
    GO(flush_start, vFp);       \
    GO(flush_stop, vFp);        \

// wrap (so bridge all calls, just in case)
static void wrapGstBufferPoolClass(my_GstBufferPoolClass_t* class)
{
    wrapGstObjectClass(&class->object_class);
    #define GO(A, W) class->A = reverse_##A##_GstBufferPool (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstBufferPoolClass(my_GstBufferPoolClass_t* class)
{
    unwrapGstObjectClass(&class->object_class);
    #define GO(A, W)   class->A = find_##A##_GstBufferPool (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstBufferPoolClass(my_GstBufferPoolClass_t* class)
{
    bridgeGstObjectClass(&class->object_class);
    #define GO(A, W) autobridge_##A##_GstBufferPool (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstBufferPoolInstance(my_GstBufferPool_t* class)
{
    unwrapGstObjectInstance(&class->object);
}
// autobridge
static void bridgeGstBufferPoolInstance(my_GstBufferPool_t* class)
{
    bridgeGstObjectInstance(&class->object);
}
// ----- GstVideoBufferPoolClass ------
// wrapper x86 -> natives of callbacks

#define SUPERGO()               \

// wrap (so bridge all calls, just in case)
static void wrapGstVideoBufferPoolClass(my_GstVideoBufferPoolClass_t* class)
{
    wrapGstBufferPoolClass(&class->parent);
    #define GO(A, W) class->A = reverse_##A##_GstVideoBufferPool (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstVideoBufferPoolClass(my_GstVideoBufferPoolClass_t* class)
{
    unwrapGstBufferPoolClass(&class->parent);
    #define GO(A, W)   class->A = find_##A##_GstVideoBufferPool (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstVideoBufferPoolClass(my_GstVideoBufferPoolClass_t* class)
{
    bridgeGstBufferPoolClass(&class->parent);
    #define GO(A, W) autobridge_##A##_GstBufferPool (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstVideoBufferPoolInstance(my_GstVideoBufferPool_t* class)
{
    unwrapGstBufferPoolInstance(&class->bufferpool);
}
// autobridge
static void bridgeGstVideoBufferPoolInstance(my_GstVideoBufferPool_t* class)
{
    bridgeGstBufferPoolInstance(&class->bufferpool);
}
// ----- GDBusProxyClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GDBusProxy, g_properties_changed, void, (void* proxy, void* changed_properties, const char* const* invalidated_properties), "ppp", proxy, changed_properties, invalidated_properties);
WRAPPER(GDBusProxy, g_signal, void,             (void* proxy, const char* sender_name, const char* signal_name, void* parameters), "pppp", proxy, sender_name, signal_name, parameters);

#define SUPERGO()                   \
    GO(g_properties_changed, vFppp);\
    GO(g_signal, vFpppp);           \

// wrap (so bridge all calls, just in case)
static void wrapGDBusProxyClass(my_GDBusProxyClass_t* class)
{
    wrapGObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GDBusProxy (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGDBusProxyClass(my_GDBusProxyClass_t* class)
{
    unwrapGObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GDBusProxy (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGDBusProxyClass(my_GDBusProxyClass_t* class)
{
    bridgeGObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GDBusProxy (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGDBusProxyInstance(my_GDBusProxy_t* class)
{
    unwrapGObjectInstance(&class->parent);
}
// autobridge
static void bridgeGDBusProxyInstance(my_GDBusProxy_t* class)
{
    bridgeGObjectInstance(&class->parent);
}
// ----- GstURIHandlerInterface ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstURIHandler,get_type, int, (size_t type), "L", type);
WRAPPER(GstURIHandler,get_protocols, void*, (size_t type), "L", type);
WRAPPER(GstURIHandler,get_uri, void*, (void* handler), "p", handler);
WRAPPER(GstURIHandler,set_uri, int, (void* handler, void* uri, void* error), "ppp", handler, uri, error);

#define SUPERGO()                       \
    GO(get_type, iFL);                  \
    GO(get_protocols, pFL);             \
    GO(get_uri, pFp);                   \
    GO(set_uri, iFppp);                 \

// wrap (so bridge all calls, just in case)
static void wrapGstURIHandlerInterface(my_GstURIHandlerInterface_t* iface)
{
    // parent don't need wrazpping
    #define GO(A, W) iface->A = reverse_##A##_GstURIHandler (W, iface->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstURIHandlerInterface(my_GstURIHandlerInterface_t* iface)
{
    // parent don't need wrazpping
    #define GO(A, W)   iface->A = find_##A##_GstURIHandler (W, iface->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstURIHandlerInterface(my_GstURIHandlerInterface_t* iface)
{
    // parent don't need wrazpping
    #define GO(A, W) autobridge_##A##_GstURIHandler (W, iface->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO
