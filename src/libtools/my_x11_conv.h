#ifndef MY_X11_CONV
#define MY_X11_CONV
#include <stdint.h>

#include "box32.h"
#include "converter32.h"
#include "my_x11_defs.h"
#include "my_x11_defs_32.h"

void convertXEvent(my_XEvent_32_t* dst, my_XEvent_t* src);
void unconvertXEvent(my_XEvent_t* dst, my_XEvent_32_t* src);
void inplace_XEventData_shring(my_XEvent_t* evt);
void inplace_XEventData_enlarge(my_XEvent_t* evt);
void convert_XErrorEvent_to_32(void* d, void* s);
void convert_XErrorEvent_to_64(void* d, void* s);

// Add a new Native Display*, return a 32bits one
void* addDisplay(void* d);
// Find a Native Diplay* and return the 32bits one
void* FindDisplay(void* d);
// return the Native Display from a 32bits one
void* getDisplay(void* d);
// removed a 32bits Display and associated ressources
void delDisplay(void* d);
// refresh the 32bits from the 64bits version
void refreshDisplay(void* dpy);
// register an xcb for a display
void regXCBDisplay(void* d, void* xcb);

void convert_Screen_to_32(void* d, void* s);
void* getScreen64(void* dpy, void* a);

void* convert_Visual_to_32(void* dpy, void* a);
void* convert_Visual_to_64(void* dpy, void* a);

void convert_XWMints_to_64(void* d, void* s);
void inplace_enlarge_wmhints(void* hints);
void inplace_shrink_wmhints(void* hints);
void convert_XSizeHints_to_64(void* d, void *s);
void inplace_enlarge_wmsizehints(void* hints);
void inplace_shrink_wmsizehints(void* hints);

void convert_XWindowAttributes_to_32(void* dpy, void* d, void* s);

void inplace_XModifierKeymap_shrink(void* a);
void inplace_XModifierKeymap_enlarge(void* a);

void convert_XVisualInfo_to_32(void* dpy, my_XVisualInfo_32_t* dst, my_XVisualInfo_t* src);
void convert_XVisualInfo_to_64(void* dpy, my_XVisualInfo_t* dst, my_XVisualInfo_32_t* src);
void convert_XVisualInfo_to_64_novisual(void* dpy, my_XVisualInfo_t* dst, my_XVisualInfo_32_t* src);
void inplace_XVisualInfo_shrink(void* dpy, void *a);
void inplace_XVisualInfo_enlarge(void* dpy, void *a);

void inplace_XdbeVisualInfo_shrink(void* a);
void inplace_XdbeScreenVisualInfo_shrink(void* a);
void inplace_XdbeVisualInfo_enlarge(void* a);
void inplace_XdbeScreenVisualInfo_enlarge(void* a);

void inplace_XExtDisplayInfo_shrink(void* a);
void inplace_XExtDisplayInfo_enlarge(void* a);
void convert_XExtensionInfo_to_32(void* d, void* s);
void convert_XExtensionInfo_to_64(void* d, void* s);
void* inplace_XExtensionInfo_shrink(void* a);
void* inplace_XExtensionInfo_enlarge(void* a);

void convert_XFontProp_to_32(my_XFontProp_32_t* dst, my_XFontProp_t* src);
void convert_XFontProp_to_64(my_XFontProp_t* dst, my_XFontProp_32_t* src);
void inplace_XFontProp_shrink(void* a);
void inplace_XFontProp_enlarge(void* a);
void inplace_XFontStruct_shrink(void* a);
void inplace_XFontStruct_enlarge(void* a);

void convert_XSetWindowAttributes_to_64(my_XSetWindowAttributes_t* dst, my_XSetWindowAttributes_32_t* src);

void WrapXImage(void* d, void* s);  //define in wrappedx11.c because it contains callbacks
void UnwrapXImage(void* d, void* s);
void* inplace_XImage_shrink(void* a);
void* inplace_XImage_enlarge(void* a);

void convert_XRRModeInfo_to_32(void* d, const void* s);
void convert_XRRModeInfo_to_64(void* d, const void* s);
void inplace_XRRScreenResources_shrink(void* s);
void inplace_XRRScreenResources_enlarge(void* s);
void inplace_XRRCrtcInfo_shrink(void* s);
void inplace_XRROutputInfo_shrink(void* s);
void inplace_XRRProviderInfo_shrink(void* a);
void inplace_XRRProviderInfo_enlarge(void* a);
void inplace_XRRProviderResources_shrink(void* a);
void inplace_XRRProviderResources_enlarge(void* a);
void* inplace_XRRPropertyInfo_shrink(void* a);

void inplace_XIDeviceInfo_shrink(void* a, int n);
int inplace_XIDeviceInfo_enlarge(void* a);

void* inplace_XDevice_shrink(void* a);
void* inplace_XDevice_enlarge(void* a);
void* inplace_XDeviceState_shrink(void* a);
void* inplace_XDeviceState_enlarge(void* a);

void register_XDevice_events(my_XDevice_t* a);
void unregister_XDevice_events(my_XDevice_t* a);
void register_XFixes_events(int event_base);
void unregister_XFixes_events();
void register_XRandR_events(int event_base);
void unregister_XRandR_events();
void register_Xkb_events(int event_base);
void unregister_Xkb_events();

void convert_XShmSegmentInfo_to_32(void* d, void* s);
void convert_XShmSegmentInfo_to_64(void* d, void* s);

void* inplace_XkbDescRec_shrink(void* a);
void* inplace_XkbDescRec_enlarge(void* a);

void convert_XAnyClassInfo_to_32(void* d, void* s);
void convert_XAnyClassInfo_to_64(void* d, void* s);
void* inplace_XAnyClassInfo_shrink(void* a);
void* inplace_XAnyClassInfo_enlarge(void* a);
void* inplace_XDeviceInfo_shrink(void* a, int n);
void* inplace_XDeviceInfo_enlarge(void* a);

void* inplace_XFilters_shrink(void* a);
void* inplace_XFilters_enlarge(void* a);

void* inplace_XRRMonitorInfo_shrink(void* a, int n);
void* inplace_XRRMonitorInfo_enlarge(void* a, int n);

#endif//MY_X11_CONV