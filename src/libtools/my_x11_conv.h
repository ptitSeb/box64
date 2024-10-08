#ifndef MY_X11_CONV
#define MY_X11_CONV
#include <stdint.h>

#include "box32.h"
#include "converter32.h"
#include "my_x11_defs.h"
#include "my_x11_defs_32.h"

void convertXEvent(my_XEvent_32_t* dst, my_XEvent_t* src);
void unconvertXEvent(my_XEvent_t* dst, my_XEvent_32_t* src);
void* addDisplay(void* d);  // Adde new Native Display*, return a 32bits one
void* FindDisplay(void* d); // Find a Native Diplay* and return the 32bits one
void* getDisplay(void* d); // return the Native Display from a 32bits one
void delDisplay(void* d); // removed a 32bits Display and associated ressources

void convert_Screen_to_32(void* d, void* s);

void convert_XWMints_to_64(void* d, void* s);
void inplace_enlarge_wmhints(void* hints);
void inplace_shrink_wmhints(void* hints);
void convert_XSizeHints_to_64(void* d, void *s);
void inplace_enlarge_wmsizehints(void* hints);
void inplace_shrink_wmsizehints(void* hints);

void convert_XWindowAttributes_to_32(void* d, void* s);

void inplace_XModifierKeymap_shrink(void* a);
void inplace_XModifierKeymap_enlarge(void* a);

void convert_XVisualInfo_to_32(my_XVisualInfo_32_t* dst, my_XVisualInfo_t* src);
void convert_XVisualInfo_to_64(my_XVisualInfo_t* dst, my_XVisualInfo_32_t* src);
void inplace_XVisualInfo_shrink(void *a);
void inplace_XVisualInfo_enlarge(void *a);

void inplace_XdbeVisualInfo_shrink(void* a);
void inplace_XdbeScreenVisualInfo_shrink(void* a);
void inplace_XdbeVisualInfo_enlarge(void* a);
void inplace_XdbeScreenVisualInfo_enlarge(void* a);

void inplace_XExtDisplayInfo_shrink(void* a);
void inplace_XExtDisplayInfo_enlarge(void* a);
void* inplace_XExtensionInfo_shrink(void* a);
void* inplace_XExtensionInfo_enlarge(void* a);

void convert_XFontProp_to_32(my_XFontProp_32_t* dst, my_XFontProp_t* src);
void convert_XFontProp_to_64(my_XFontProp_t* dst, my_XFontProp_32_t* src);
void inplace_XFontProp_shrink(void* a);
void inplace_XFontProp_enlarge(void* a);
void inplace_XFontStruct_shrink(void* a);
void inplace_XFontStruct_enlarge(void* a);
#endif//MY_X11_CONV