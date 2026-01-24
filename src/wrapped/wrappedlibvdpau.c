#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "callback.h"
#include "librarian.h"
#include "box64context.h"
#include "emu/x64emu_private.h"
#include "myalign.h"

const char* libvdpauName = "libvdpau.so.1";
#define LIBNAME libvdpau

typedef int (*iFpip_t) (void*, int, void*);
typedef int (*iFupp_t) (uint32_t, void*, void*);

#define ADDED_FUNCTIONS()       \

#include "generated/wrappedlibvdpautypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)


// VdpPreemptionCallback
#define GO(A)   \
static uintptr_t my_VdpPreemptionCallback_fct_##A = 0;              \
static void my_VdpPreemptionCallback_##A(uint32_t a, void* b)       \
{                                                                   \
    RunFunctionFmt(my_VdpPreemptionCallback_fct_##A, "up", a, b);   \
}
SUPER()
#undef GO
static void* findVdpPreemptionCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_VdpPreemptionCallback_fct_##A == (uintptr_t)fct) return my_VdpPreemptionCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_VdpPreemptionCallback_fct_##A == 0) {my_VdpPreemptionCallback_fct_##A = (uintptr_t)fct; return my_VdpPreemptionCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libvdpau VdpPreemptionCallback callback\n");
    return NULL;
}

#undef SUPER

#define VDP_FUNC_ID_GET_ERROR_STRING                                            (0)
#define VDP_FUNC_ID_GET_PROC_ADDRESS                                            (1)
#define VDP_FUNC_ID_GET_API_VERSION                                             (2)
#define VDP_FUNC_ID_GET_INFORMATION_STRING                                      (4)
#define VDP_FUNC_ID_DEVICE_DESTROY                                              (5)
#define VDP_FUNC_ID_GENERATE_CSC_MATRIX                                         (6)
#define VDP_FUNC_ID_VIDEO_SURFACE_QUERY_CAPABILITIES                            (7)
#define VDP_FUNC_ID_VIDEO_SURFACE_QUERY_GET_PUT_BITS_Y_CB_CR_CAPABILITIES       (8)
#define VDP_FUNC_ID_VIDEO_SURFACE_CREATE                                        (9)
#define VDP_FUNC_ID_VIDEO_SURFACE_DESTROY                                       (10)
#define VDP_FUNC_ID_VIDEO_SURFACE_GET_PARAMETERS                                (11)
#define VDP_FUNC_ID_VIDEO_SURFACE_GET_BITS_Y_CB_CR                              (12)
#define VDP_FUNC_ID_VIDEO_SURFACE_PUT_BITS_Y_CB_CR                              (13)
#define VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_CAPABILITIES                           (14)
#define VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_GET_PUT_BITS_NATIVE_CAPABILITIES       (15)
#define VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_PUT_BITS_INDEXED_CAPABILITIES          (16)
#define VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_PUT_BITS_Y_CB_CR_CAPABILITIES          (17)
#define VDP_FUNC_ID_OUTPUT_SURFACE_CREATE                                       (18)
#define VDP_FUNC_ID_OUTPUT_SURFACE_DESTROY                                      (19)
#define VDP_FUNC_ID_OUTPUT_SURFACE_GET_PARAMETERS                               (20)
#define VDP_FUNC_ID_OUTPUT_SURFACE_GET_BITS_NATIVE                              (21)
#define VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_NATIVE                              (22)
#define VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_INDEXED                             (23)
#define VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_Y_CB_CR                             (24)
#define VDP_FUNC_ID_BITMAP_SURFACE_QUERY_CAPABILITIES                           (25)
#define VDP_FUNC_ID_BITMAP_SURFACE_CREATE                                       (26)
#define VDP_FUNC_ID_BITMAP_SURFACE_DESTROY                                      (27)
#define VDP_FUNC_ID_BITMAP_SURFACE_GET_PARAMETERS                               (28)
#define VDP_FUNC_ID_BITMAP_SURFACE_PUT_BITS_NATIVE                              (29)
#define VDP_FUNC_ID_OUTPUT_SURFACE_RENDER_OUTPUT_SURFACE                        (33)
#define VDP_FUNC_ID_OUTPUT_SURFACE_RENDER_BITMAP_SURFACE                        (34)
#define VDP_FUNC_ID_OUTPUT_SURFACE_RENDER_VIDEO_SURFACE_LUMA                    (35)
#define VDP_FUNC_ID_DECODER_QUERY_CAPABILITIES                                  (36)
#define VDP_FUNC_ID_DECODER_CREATE                                              (37)
#define VDP_FUNC_ID_DECODER_DESTROY                                             (38)
#define VDP_FUNC_ID_DECODER_GET_PARAMETERS                                      (39)
#define VDP_FUNC_ID_DECODER_RENDER                                              (40)
#define VDP_FUNC_ID_VIDEO_MIXER_QUERY_FEATURE_SUPPORT                           (41)
#define VDP_FUNC_ID_VIDEO_MIXER_QUERY_PARAMETER_SUPPORT                         (42)
#define VDP_FUNC_ID_VIDEO_MIXER_QUERY_ATTRIBUTE_SUPPORT                         (43)
#define VDP_FUNC_ID_VIDEO_MIXER_QUERY_PARAMETER_VALUE_RANGE                     (44)
#define VDP_FUNC_ID_VIDEO_MIXER_QUERY_ATTRIBUTE_VALUE_RANGE                     (45)
#define VDP_FUNC_ID_VIDEO_MIXER_CREATE                                          (46)
#define VDP_FUNC_ID_VIDEO_MIXER_SET_FEATURE_ENABLES                             (47)
#define VDP_FUNC_ID_VIDEO_MIXER_SET_ATTRIBUTE_VALUES                            (48)
#define VDP_FUNC_ID_VIDEO_MIXER_GET_FEATURE_SUPPORT                             (49)
#define VDP_FUNC_ID_VIDEO_MIXER_GET_FEATURE_ENABLES                             (50)
#define VDP_FUNC_ID_VIDEO_MIXER_GET_PARAMETER_VALUES                            (51)
#define VDP_FUNC_ID_VIDEO_MIXER_GET_ATTRIBUTE_VALUES                            (52)
#define VDP_FUNC_ID_VIDEO_MIXER_DESTROY                                         (53)
#define VDP_FUNC_ID_VIDEO_MIXER_RENDER                                          (54)
#define VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_DESTROY                           (55)
#define VDP_FUNC_ID_PRESENTATION_QUEUE_CREATE                                   (56)
#define VDP_FUNC_ID_PRESENTATION_QUEUE_DESTROY                                  (57)
#define VDP_FUNC_ID_PRESENTATION_QUEUE_SET_BACKGROUND_COLOR                     (58)
#define VDP_FUNC_ID_PRESENTATION_QUEUE_GET_BACKGROUND_COLOR                     (59)
#define VDP_FUNC_ID_PRESENTATION_QUEUE_GET_TIME                                 (62)
#define VDP_FUNC_ID_PRESENTATION_QUEUE_DISPLAY                                  (63)
#define VDP_FUNC_ID_PRESENTATION_QUEUE_BLOCK_UNTIL_SURFACE_IDLE                 (64)
#define VDP_FUNC_ID_PRESENTATION_QUEUE_QUERY_SURFACE_STATUS                     (65)
#define VDP_FUNC_ID_PREEMPTION_CALLBACK_REGISTER                                (66)
#define VDP_FUNC_ID_DECODER_QUERY_CAPABILITY                                    (67)
#define VDP_FUNC_ID_BASE_WINSYS 0x1000
#define VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_CREATE_X11 (VDP_FUNC_ID_BASE_WINSYS + 0)

void my_vdpau_wrap_function(int id, void** f);

int my_VdpGetProcAddress(x64emu_t* emu, void* a, int b, void* c)
{
    iFpip_t f = getBridgeFnc2((void*)R_RIP);
    if(!f) return 1;
    int ret = f(a, b, c);
    if(!ret) my_vdpau_wrap_function(b, c);
    return ret;
}

int my_VdpPreemptionCallbackRegister(x64emu_t* emu, uint32_t device, void* cb, void* ctx)
{
    iFupp_t f = getBridgeFnc2((void*)R_RIP);
    if(!f) return 1;
    return f(device, findVdpPreemptionCallbackFct(cb), ctx);
}

void my_vdpau_wrap_function(int id, void** f)
{
    if(!f) return;
    printf_dlsym(LOG_DEBUG, "vdpau:getprocaddress of id = %d (0x%x): %p\n", id, id, *f);
    if(!*f) return;
    switch(id) {
#define GO(A, B)    case A: *f = (void*)AddCheckBridge(my_lib->w.bridge, B, *f, 0, "vdpau_" #A); break
#define GO2(A, B, C)    case A: *f = (void*)AddCheckBridge2(my_lib->w.bridge, B, C, *f, 0, "vdpau_" #A); break;

        GO(VDP_FUNC_ID_GET_ERROR_STRING, pFi);
        GO2(VDP_FUNC_ID_GET_PROC_ADDRESS, iFEpip, my_VdpGetProcAddress);
        GO(VDP_FUNC_ID_GET_API_VERSION, iFp);
        GO(VDP_FUNC_ID_GET_INFORMATION_STRING, iFp);
        GO(VDP_FUNC_ID_DEVICE_DESTROY, iFu);
        GO(VDP_FUNC_ID_GENERATE_CSC_MATRIX, iFpup);
        GO(VDP_FUNC_ID_VIDEO_SURFACE_QUERY_CAPABILITIES, iFuuppp);
        GO(VDP_FUNC_ID_VIDEO_SURFACE_QUERY_GET_PUT_BITS_Y_CB_CR_CAPABILITIES, iFuuup);
        GO(VDP_FUNC_ID_VIDEO_SURFACE_CREATE, iFuuuup);
        GO(VDP_FUNC_ID_VIDEO_SURFACE_DESTROY, iFu);
        GO(VDP_FUNC_ID_VIDEO_SURFACE_GET_PARAMETERS, iFuppp);
        GO(VDP_FUNC_ID_VIDEO_SURFACE_GET_BITS_Y_CB_CR, iFuupp);
        GO(VDP_FUNC_ID_VIDEO_SURFACE_PUT_BITS_Y_CB_CR, iFuupp);
        GO(VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_CAPABILITIES, iFuuppp);
        GO(VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_GET_PUT_BITS_NATIVE_CAPABILITIES, iFuup);
        GO(VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_PUT_BITS_INDEXED_CAPABILITIES, iFuuuup);
        GO(VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_PUT_BITS_Y_CB_CR_CAPABILITIES, iFuuup);
        GO(VDP_FUNC_ID_OUTPUT_SURFACE_CREATE, iFuuuup);
        GO(VDP_FUNC_ID_OUTPUT_SURFACE_DESTROY, iFu);
        GO(VDP_FUNC_ID_OUTPUT_SURFACE_GET_PARAMETERS, iFuppp);
        GO(VDP_FUNC_ID_OUTPUT_SURFACE_GET_BITS_NATIVE, iFuppp);
        GO(VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_NATIVE, iFuppp);
        GO(VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_INDEXED, iFuupppup);
        GO(VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_Y_CB_CR, iFuupppp);
        GO(VDP_FUNC_ID_BITMAP_SURFACE_QUERY_CAPABILITIES, iFuuppp);
        GO(VDP_FUNC_ID_BITMAP_SURFACE_CREATE, iFuuuuip);
        GO(VDP_FUNC_ID_BITMAP_SURFACE_DESTROY, iFu);
        GO(VDP_FUNC_ID_BITMAP_SURFACE_GET_PARAMETERS, iFupppp);
        GO(VDP_FUNC_ID_BITMAP_SURFACE_PUT_BITS_NATIVE, iFuppp);
        GO(VDP_FUNC_ID_OUTPUT_SURFACE_RENDER_OUTPUT_SURFACE, iFupupppu);
        GO(VDP_FUNC_ID_OUTPUT_SURFACE_RENDER_BITMAP_SURFACE, iFupupppu);
        //GO(VDP_FUNC_ID_OUTPUT_SURFACE_RENDER_VIDEO_SURFACE_LUMA, 
        GO(VDP_FUNC_ID_DECODER_QUERY_CAPABILITY, iFuuup);
        GO(VDP_FUNC_ID_DECODER_QUERY_CAPABILITIES, iFuuppppp);
        GO(VDP_FUNC_ID_DECODER_CREATE, iFuuuuup);
        GO(VDP_FUNC_ID_DECODER_DESTROY, iFu);
        GO(VDP_FUNC_ID_DECODER_GET_PARAMETERS, iFuppp);
        GO(VDP_FUNC_ID_DECODER_RENDER, iFuupup);
        GO(VDP_FUNC_ID_VIDEO_MIXER_QUERY_FEATURE_SUPPORT, iFuup);
        GO(VDP_FUNC_ID_VIDEO_MIXER_QUERY_PARAMETER_SUPPORT, iFuup);
        GO(VDP_FUNC_ID_VIDEO_MIXER_QUERY_ATTRIBUTE_SUPPORT, iFuup);
        GO(VDP_FUNC_ID_VIDEO_MIXER_QUERY_PARAMETER_VALUE_RANGE, iFuupp);
        GO(VDP_FUNC_ID_VIDEO_MIXER_QUERY_ATTRIBUTE_VALUE_RANGE, iFuupp);
        GO(VDP_FUNC_ID_VIDEO_MIXER_CREATE, iFuupuppp);
        GO(VDP_FUNC_ID_VIDEO_MIXER_SET_FEATURE_ENABLES, iFuupp);
        GO(VDP_FUNC_ID_VIDEO_MIXER_SET_ATTRIBUTE_VALUES, iFuupp);
        GO(VDP_FUNC_ID_VIDEO_MIXER_GET_FEATURE_SUPPORT, iFuupp);
        GO(VDP_FUNC_ID_VIDEO_MIXER_GET_FEATURE_ENABLES, iFuupp);
        GO(VDP_FUNC_ID_VIDEO_MIXER_GET_PARAMETER_VALUES, iFuupp);
        GO(VDP_FUNC_ID_VIDEO_MIXER_GET_ATTRIBUTE_VALUES, iFuupp);
        GO(VDP_FUNC_ID_VIDEO_MIXER_DESTROY, iFu);
        GO(VDP_FUNC_ID_VIDEO_MIXER_RENDER, iFuupiupuuppuppup);
        GO(VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_DESTROY, iFu);
        GO(VDP_FUNC_ID_PRESENTATION_QUEUE_CREATE, iFuup);
        GO(VDP_FUNC_ID_PRESENTATION_QUEUE_DESTROY, iFu);
        GO(VDP_FUNC_ID_PRESENTATION_QUEUE_SET_BACKGROUND_COLOR, iFup);
        GO(VDP_FUNC_ID_PRESENTATION_QUEUE_GET_BACKGROUND_COLOR, iFup);
        GO(VDP_FUNC_ID_PRESENTATION_QUEUE_GET_TIME, iFup);
        GO(VDP_FUNC_ID_PRESENTATION_QUEUE_DISPLAY, iFuuuuU);
        GO(VDP_FUNC_ID_PRESENTATION_QUEUE_BLOCK_UNTIL_SURFACE_IDLE, iFuup);
        GO(VDP_FUNC_ID_PRESENTATION_QUEUE_QUERY_SURFACE_STATUS, iFuupp);
        GO2(VDP_FUNC_ID_PREEMPTION_CALLBACK_REGISTER, iFEupp, my_VdpPreemptionCallbackRegister);

        GO(VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_CREATE_X11, iFupp);
#undef GO
#undef GO2
        default: printf_log(LOG_NONE, "Error: vdpau function id=%d(0x%x) unsupported\n", id, id);
    }
}

EXPORT uint32_t my_vdp_device_create_x11(x64emu_t* emu, void* display, int screen, void* device, void** get_proc_address)
{
    int ret = my->vdp_device_create_x11(display, screen, device, get_proc_address);
    if(!ret && get_proc_address)
        *get_proc_address = (void*)AddCheckBridge2(my_lib->w.bridge, iFEpip, my_VdpGetProcAddress, *get_proc_address, 0, "vdpau_my_VdpGetProcAddress");;
    return ret;
}

#include "wrappedlib_init.h"
