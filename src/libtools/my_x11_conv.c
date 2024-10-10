#include <stdint.h>
#include <dlfcn.h>

#include "debug.h"
#include "box32context.h"
#include "wrapper32.h"
#include "library.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "converter32.h"
#include "my_x11_defs.h"
#include "my_x11_defs_32.h"
#include "my_x11_conv.h"

void convertXEvent(my_XEvent_32_t* dst, my_XEvent_t* src)
{
    if(!src->type) {
        // This is an XErrorEvent, and it's different!
        dst->xerror.type = src->xerror.type;
        dst->xerror.display = to_ptrv(FindDisplay(src->xerror.display));
        dst->xerror.resourceid = to_ulong(src->xerror.resourceid);
        dst->xerror.serial = to_ulong(src->xerror.serial);
        dst->xerror.error_code = src->xerror.error_code;
        dst->xerror.request_code = src->xerror.request_code;
        dst->xerror.minor_code = src->xerror.minor_code;
        return;
    }
    // convert the XAnyEvent first, as it's a common set
    dst->type = src->type;
    dst->xany.display = to_ptrv(FindDisplay(src->xany.display));
    dst->xany.window = to_ulong(src->xany.window);
    dst->xany.send_event = src->xany.serial;
    dst->xany.serial = to_ulong(src->xany.serial);
    switch(src->type) {
        case XEVT_KeyPress:
        case XEVT_KeyRelease:
            dst->xkey.root = to_ulong(src->xkey.root);
            dst->xkey.subwindow = to_ulong(src->xkey.subwindow);
            dst->xkey.time = to_ulong(src->xkey.time);
            dst->xkey.x = src->xkey.x;
            dst->xkey.y = src->xkey.y;
            dst->xkey.x_root = src->xkey.x_root;
            dst->xkey.y_root = src->xkey.y_root;
            dst->xkey.state = src->xkey.state;
            dst->xkey.keycode = src->xkey.keycode;
            dst->xkey.same_screen = src->xkey.same_screen;
            break;
        case XEVT_ButtonPress:
        case XEVT_ButtonRelease:
            dst->xbutton.root = to_ulong(src->xbutton.root);
            dst->xbutton.subwindow = to_ulong(src->xbutton.subwindow);
            dst->xbutton.time = to_ulong(src->xbutton.time);
            dst->xbutton.x = src->xbutton.x;
            dst->xbutton.y = src->xbutton.y;
            dst->xbutton.x_root = src->xbutton.x_root;
            dst->xbutton.y_root = src->xbutton.y_root;
            dst->xbutton.state = src->xbutton.state;
            dst->xbutton.button = src->xbutton.button;
            dst->xbutton.same_screen = src->xbutton.same_screen;
            break;
        case XEVT_MotionNotify:
            dst->xmotion.root = to_ulong(src->xmotion.root);
            dst->xmotion.subwindow = to_ulong(src->xmotion.subwindow);
            dst->xmotion.time = to_ulong(src->xmotion.time);
            dst->xmotion.x = src->xmotion.x;
            dst->xmotion.y = src->xmotion.y;
            dst->xmotion.x_root = src->xmotion.x_root;
            dst->xmotion.y_root = src->xmotion.y_root;
            dst->xmotion.state = src->xmotion.state;
            dst->xmotion.is_hint = src->xmotion.is_hint;
            dst->xmotion.same_screen = src->xmotion.same_screen;
            break;
        case XEVT_EnterNotify:
        case XEVT_LeaveNotify:
            dst->xcrossing.root = to_ulong(src->xcrossing.root);
            dst->xcrossing.subwindow = to_ulong(src->xcrossing.subwindow);
            dst->xcrossing.time = to_ulong(src->xcrossing.time);
            dst->xcrossing.x = src->xcrossing.x;
            dst->xcrossing.y = src->xcrossing.y;
            dst->xcrossing.x_root = src->xcrossing.x_root;
            dst->xcrossing.y_root = src->xcrossing.y_root;
            dst->xcrossing.mode = src->xcrossing.mode;
            dst->xcrossing.detail = src->xcrossing.detail;
            dst->xcrossing.same_screen = src->xcrossing.same_screen;
            dst->xcrossing.focus = src->xcrossing.focus;
            dst->xcrossing.state = src->xcrossing.state;
            break;
        case XEVT_FocusIn:
        case XEVT_FocusOut:
            dst->xfocus.mode = src->xfocus.mode;
            dst->xfocus.detail = src->xfocus.detail;
            break;
        case XEVT_KeymapNotify:
            memcpy(dst->xkeymap.key_vector, src->xkeymap.key_vector, 32);
            break;
        case XEVT_Expose:
            dst->xexpose.x = src->xexpose.x;
            dst->xexpose.y = src->xexpose.y;
            dst->xexpose.width = src->xexpose.width;
            dst->xexpose.height = src->xexpose.height;
            dst->xexpose.count = src->xexpose.count;
            break;
        case XEVT_GraphicsExpose:
            dst->xgraphicsexpose.x = src->xgraphicsexpose.x;
            dst->xgraphicsexpose.y = src->xgraphicsexpose.y;
            dst->xgraphicsexpose.width = src->xgraphicsexpose.width;
            dst->xgraphicsexpose.height = src->xgraphicsexpose.height;
            dst->xgraphicsexpose.count = src->xgraphicsexpose.count;
            dst->xgraphicsexpose.major_code = src->xgraphicsexpose.major_code;
            dst->xgraphicsexpose.minor_code = src->xgraphicsexpose.minor_code;
            break;
        case XEVT_NoExpose:
            dst->xnoexpose.major_code = src->xnoexpose.major_code;
            dst->xnoexpose.minor_code = src->xnoexpose.minor_code;
            break;
        case XEVT_VisibilityNotify:
            dst->xvisibility.state = src->xvisibility.state;
            break;
        case XEVT_CreateNotify:
            dst->xcreatewindow.window = to_ulong(src->xcreatewindow.window);
            dst->xcreatewindow.x = src->xcreatewindow.x;
            dst->xcreatewindow.y = src->xcreatewindow.y;
            dst->xcreatewindow.width = src->xcreatewindow.width;
            dst->xcreatewindow.height = src->xcreatewindow.height;
            dst->xcreatewindow.border_width = src->xcreatewindow.border_width;
            dst->xcreatewindow.override_redirect = src->xcreatewindow.override_redirect;
            break;
        case XEVT_DestroyNotify:
            dst->xdestroywindow.window = to_ulong(src->xdestroywindow.window);
            break;
        case XEVT_UnmapNotify:
            dst->xunmap.window = to_ulong(src->xunmap.window);
            dst->xunmap.from_configure = src->xunmap.from_configure;
            break;
        case XEVT_MapNotify:
            dst->xmap.window = to_ulong(src->xmap.window);
            dst->xmap.override_redirect = src->xmap.override_redirect;
            break;
        case XEVT_MapRequest:
            dst->xmaprequest.window = to_ulong(src->xmaprequest.window);
            break;
        case XEVT_ReparentNotify:
            dst->xreparent.window = to_ulong(src->xreparent.window);
            dst->xreparent.parent = to_ulong(src->xreparent.parent);
            dst->xreparent.x = src->xreparent.x;
            dst->xreparent.y = src->xreparent.y;
            dst->xreparent.override_redirect = src->xreparent.override_redirect;
            break;
        case XEVT_ConfigureNotify:
            dst->xconfigure.window = to_ulong(src->xconfigure.window);
            dst->xconfigure.x = src->xconfigure.x;
            dst->xconfigure.y = src->xconfigure.y;
            dst->xconfigure.width = src->xconfigure.width;
            dst->xconfigure.height = src->xconfigure.height;
            dst->xconfigure.border_width = src->xconfigure.border_width;
            dst->xconfigure.above = to_ulong(src->xconfigure.above);
            dst->xconfigure.override_redirect = src->xconfigure.override_redirect;
            break;
        case XEVT_ConfigureRequest:
            dst->xconfigurerequest.window = to_ulong(src->xconfigurerequest.window);
            dst->xconfigurerequest.x = src->xconfigurerequest.x;
            dst->xconfigurerequest.y = src->xconfigurerequest.y;
            dst->xconfigurerequest.width = src->xconfigurerequest.width;
            dst->xconfigurerequest.height = src->xconfigurerequest.height;
            dst->xconfigurerequest.border_width = src->xconfigurerequest.border_width;
            dst->xconfigurerequest.above = to_ulong(src->xconfigurerequest.above);
            dst->xconfigurerequest.detail = src->xconfigurerequest.detail;
            dst->xconfigurerequest.value_mask = to_ulong(src->xconfigurerequest.value_mask);
            break;
        case XEVT_GravityNotify:
            dst->xgravity.window = to_ulong(src->xgravity.window);
            dst->xgravity.x = src->xgravity.x;
            dst->xgravity.y = src->xgravity.y;
            break;
        case XEVT_ResizeRequest:
            dst->xresizerequest.width = src->xresizerequest.width;
            dst->xresizerequest.height = src->xresizerequest.height;
            break;
        case XEVT_CirculateNotify:
            dst->xcirculate.window = to_ulong(src->xcirculate.window);
            dst->xcirculate.place = src->xcirculate.place;
            break;
        case XEVT_CirculateRequest:
            dst->xcirculaterequest.window = to_ulong(src->xcirculaterequest.window);
            dst->xcirculaterequest.place = src->xcirculaterequest.place;
            break;
        case XEVT_PropertyNotify:
            dst->xproperty.atom = to_ulong(src->xproperty.atom);
            dst->xproperty.time = to_ulong(src->xproperty.time);
            dst->xproperty.state = src->xproperty.state;
            break;
        case XEVT_SelectionClear:
            dst->xselectionclear.selection = to_ulong(src->xselectionclear.selection);
            dst->xselectionclear.time = to_ulong(src->xselectionclear.time);
            break;
        case XEVT_SelectionRequest:
            dst->xselectionrequest.requestor = to_ulong(src->xselectionrequest.requestor);
            dst->xselectionrequest.selection = to_ulong(src->xselectionrequest.selection);
            dst->xselectionrequest.target = to_ulong(src->xselectionrequest.target);
            dst->xselectionrequest.property = to_ulong(src->xselectionrequest.property);
            dst->xselectionrequest.time = to_ulong(src->xselectionrequest.time);
            break;
        case XEVT_SelectionNotify:
            dst->xselection.selection = to_ulong(src->xselection.selection);
            dst->xselection.target = to_ulong(src->xselection.target);
            dst->xselection.property = to_ulong(src->xselection.property);
            dst->xselection.time = to_ulong(src->xselection.time);
            break;
        case XEVT_ColormapNotify:
            dst->xcolormap.colormap = to_ulong(src->xcolormap.colormap);
            dst->xcolormap.c_new = src->xcolormap.c_new;
            dst->xcolormap.state = src->xcolormap.state;
            break;
        case XEVT_ClientMessage:
            dst->xclient.message_type = to_ulong(src->xclient.message_type);
            dst->xclient.format = src->xclient.format;
            if(src->xclient.format==32)
                for(int i=0; i<5; ++i) {
                    if(((src->xclient.data.l[i]&0xffffffff80000000LL))==0xffffffff80000000LL)
                        dst->xclient.data.l[i] = to_ulong(src->xclient.data.l[i]&0xffffffff);   // negative value...
                    else
                        dst->xclient.data.l[i] = to_ulong(src->xclient.data.l[i]);
                }
            else
                memcpy(dst->xclient.data.b, src->xclient.data.b, 20);
            break;
        case XEVT_MappingNotify:
            dst->xmapping.request = src->xmapping.request;
            dst->xmapping.first_keycode = src->xmapping.first_keycode;
            dst->xmapping.count = src->xmapping.count;
            break;
        case XEVT_GenericEvent:
            dst->xgeneric.extension = src->xgeneric.extension;
            dst->xgeneric.evtype = src->xgeneric.evtype;
            break;
        default:
            printf_log(LOG_INFO, "Warning, unsupported 32bits XEvent type=%d\n", src->type);
    }
}
void unconvertXEvent(my_XEvent_t* dst, my_XEvent_32_t* src)
{
    if(!src->type) {
        // This is an XErrorEvent, and it's different!
        dst->xerror.type = src->xerror.type;
        dst->xerror.display = getDisplay(from_ptrv(src->xerror.display));
        dst->xerror.resourceid = from_ulong(src->xerror.resourceid);
        dst->xerror.serial = from_ulong(src->xerror.serial);
        dst->xerror.error_code = src->xerror.error_code;
        dst->xerror.request_code = src->xerror.request_code;
        dst->xerror.minor_code = src->xerror.minor_code;
        return;
    }
    // convert the XAnyEvent first, as it's a common set
    dst->type = src->type;
    dst->xany.display = getDisplay(from_ptrv(src->xany.display));
    dst->xany.window = from_ulong(src->xany.window);
    dst->xany.send_event = src->xany.serial;
    dst->xany.serial = from_ulong(src->xany.serial);
    switch(src->type) {
        case XEVT_KeyPress:
        case XEVT_KeyRelease:
            dst->xkey.root = from_ulong(src->xkey.root);
            dst->xkey.subwindow = from_ulong(src->xkey.subwindow);
            dst->xkey.time = from_ulong(src->xkey.time);
            dst->xkey.x = src->xkey.x;
            dst->xkey.y = src->xkey.y;
            dst->xkey.x_root = src->xkey.x_root;
            dst->xkey.y_root = src->xkey.y_root;
            dst->xkey.state = src->xkey.state;
            dst->xkey.keycode = src->xkey.keycode;
            dst->xkey.same_screen = src->xkey.same_screen;
            break;
        case XEVT_ButtonPress:
        case XEVT_ButtonRelease:
            dst->xbutton.root = from_ulong(src->xbutton.root);
            dst->xbutton.subwindow = from_ulong(src->xbutton.subwindow);
            dst->xbutton.time = from_ulong(src->xbutton.time);
            dst->xbutton.x = src->xbutton.x;
            dst->xbutton.y = src->xbutton.y;
            dst->xbutton.x_root = src->xbutton.x_root;
            dst->xbutton.y_root = src->xbutton.y_root;
            dst->xbutton.state = src->xbutton.state;
            dst->xbutton.button = src->xbutton.button;
            dst->xbutton.same_screen = src->xbutton.same_screen;
            break;
        case XEVT_MotionNotify:
            dst->xmotion.root = from_ulong(src->xmotion.root);
            dst->xmotion.subwindow = from_ulong(src->xmotion.subwindow);
            dst->xmotion.time = from_ulong(src->xmotion.time);
            dst->xmotion.x = src->xmotion.x;
            dst->xmotion.y = src->xmotion.y;
            dst->xmotion.x_root = src->xmotion.x_root;
            dst->xmotion.y_root = src->xmotion.y_root;
            dst->xmotion.state = src->xmotion.state;
            dst->xmotion.is_hint = src->xmotion.is_hint;
            dst->xmotion.same_screen = src->xmotion.same_screen;
            break;
        case XEVT_EnterNotify:
        case XEVT_LeaveNotify:
            dst->xcrossing.root = from_ulong(src->xcrossing.root);
            dst->xcrossing.subwindow = from_ulong(src->xcrossing.subwindow);
            dst->xcrossing.time = from_ulong(src->xcrossing.time);
            dst->xcrossing.x = src->xcrossing.x;
            dst->xcrossing.y = src->xcrossing.y;
            dst->xcrossing.x_root = src->xcrossing.x_root;
            dst->xcrossing.y_root = src->xcrossing.y_root;
            dst->xcrossing.mode = src->xcrossing.mode;
            dst->xcrossing.detail = src->xcrossing.detail;
            dst->xcrossing.same_screen = src->xcrossing.same_screen;
            dst->xcrossing.focus = src->xcrossing.focus;
            dst->xcrossing.state = src->xcrossing.state;
            break;
        case XEVT_FocusIn:
        case XEVT_FocusOut:
            dst->xfocus.mode = src->xfocus.mode;
            dst->xfocus.detail = src->xfocus.detail;
            break;
        case XEVT_KeymapNotify:
            memcpy(dst->xkeymap.key_vector, src->xkeymap.key_vector, 32);
            break;
        case XEVT_Expose:
            dst->xexpose.x = src->xexpose.x;
            dst->xexpose.y = src->xexpose.y;
            dst->xexpose.width = src->xexpose.width;
            dst->xexpose.height = src->xexpose.height;
            dst->xexpose.count = src->xexpose.count;
            break;
        case XEVT_GraphicsExpose:
            dst->xgraphicsexpose.x = src->xgraphicsexpose.x;
            dst->xgraphicsexpose.y = src->xgraphicsexpose.y;
            dst->xgraphicsexpose.width = src->xgraphicsexpose.width;
            dst->xgraphicsexpose.height = src->xgraphicsexpose.height;
            dst->xgraphicsexpose.count = src->xgraphicsexpose.count;
            dst->xgraphicsexpose.major_code = src->xgraphicsexpose.major_code;
            dst->xgraphicsexpose.minor_code = src->xgraphicsexpose.minor_code;
            break;
        case XEVT_NoExpose:
            dst->xnoexpose.major_code = src->xnoexpose.major_code;
            dst->xnoexpose.minor_code = src->xnoexpose.minor_code;
            break;
        case XEVT_VisibilityNotify:
            dst->xvisibility.state = src->xvisibility.state;
            break;
        case XEVT_CreateNotify:
            dst->xcreatewindow.window = from_ulong(src->xcreatewindow.window);
            dst->xcreatewindow.x = src->xcreatewindow.x;
            dst->xcreatewindow.y = src->xcreatewindow.y;
            dst->xcreatewindow.width = src->xcreatewindow.width;
            dst->xcreatewindow.height = src->xcreatewindow.height;
            dst->xcreatewindow.border_width = src->xcreatewindow.border_width;
            dst->xcreatewindow.override_redirect = src->xcreatewindow.override_redirect;
            break;
        case XEVT_DestroyNotify:
            dst->xdestroywindow.window = from_ulong(src->xdestroywindow.window);
            break;
        case XEVT_UnmapNotify:
            dst->xunmap.window = from_ulong(src->xunmap.window);
            dst->xunmap.from_configure = src->xunmap.from_configure;
            break;
        case XEVT_MapNotify:
            dst->xmap.window = from_ulong(src->xmap.window);
            dst->xmap.override_redirect = src->xmap.override_redirect;
            break;
        case XEVT_MapRequest:
            dst->xmaprequest.window = from_ulong(src->xmaprequest.window);
            break;
        case XEVT_ReparentNotify:
            dst->xreparent.window = from_ulong(src->xreparent.window);
            dst->xreparent.parent = from_ulong(src->xreparent.parent);
            dst->xreparent.x = src->xreparent.x;
            dst->xreparent.y = src->xreparent.y;
            dst->xreparent.override_redirect = src->xreparent.override_redirect;
            break;
        case XEVT_ConfigureNotify:
            dst->xconfigure.window = from_ulong(src->xconfigure.window);
            dst->xconfigure.x = src->xconfigure.x;
            dst->xconfigure.y = src->xconfigure.y;
            dst->xconfigure.width = src->xconfigure.width;
            dst->xconfigure.height = src->xconfigure.height;
            dst->xconfigure.border_width = src->xconfigure.border_width;
            dst->xconfigure.above = from_ulong(src->xconfigure.above);
            dst->xconfigure.override_redirect = src->xconfigure.override_redirect;
            break;
        case XEVT_ConfigureRequest:
            dst->xconfigurerequest.window = from_ulong(src->xconfigurerequest.window);
            dst->xconfigurerequest.x = src->xconfigurerequest.x;
            dst->xconfigurerequest.y = src->xconfigurerequest.y;
            dst->xconfigurerequest.width = src->xconfigurerequest.width;
            dst->xconfigurerequest.height = src->xconfigurerequest.height;
            dst->xconfigurerequest.border_width = src->xconfigurerequest.border_width;
            dst->xconfigurerequest.above = from_ulong(src->xconfigurerequest.above);
            dst->xconfigurerequest.detail = src->xconfigurerequest.detail;
            dst->xconfigurerequest.value_mask = from_ulong(src->xconfigurerequest.value_mask);
            break;
        case XEVT_GravityNotify:
            dst->xgravity.window = from_ulong(src->xgravity.window);
            dst->xgravity.x = src->xgravity.x;
            dst->xgravity.y = src->xgravity.y;
            break;
        case XEVT_ResizeRequest:
            dst->xresizerequest.width = src->xresizerequest.width;
            dst->xresizerequest.height = src->xresizerequest.height;
            break;
        case XEVT_CirculateNotify:
            dst->xcirculate.window = from_ulong(src->xcirculate.window);
            dst->xcirculate.place = src->xcirculate.place;
            break;
        case XEVT_CirculateRequest:
            dst->xcirculaterequest.window = from_ulong(src->xcirculaterequest.window);
            dst->xcirculaterequest.place = src->xcirculaterequest.place;
            break;
        case XEVT_PropertyNotify:
            dst->xproperty.atom = from_ulong(src->xproperty.atom);
            dst->xproperty.time = from_ulong(src->xproperty.time);
            dst->xproperty.state = src->xproperty.state;
            break;
        case XEVT_SelectionClear:
            dst->xselectionclear.selection = from_ulong(src->xselectionclear.selection);
            dst->xselectionclear.time = from_ulong(src->xselectionclear.time);
            break;
        case XEVT_SelectionRequest:
            dst->xselectionrequest.requestor = from_ulong(src->xselectionrequest.requestor);
            dst->xselectionrequest.selection = from_ulong(src->xselectionrequest.selection);
            dst->xselectionrequest.target = from_ulong(src->xselectionrequest.target);
            dst->xselectionrequest.property = from_ulong(src->xselectionrequest.property);
            dst->xselectionrequest.time = from_ulong(src->xselectionrequest.time);
            break;
        case XEVT_SelectionNotify:
            dst->xselection.selection = from_ulong(src->xselection.selection);
            dst->xselection.target = from_ulong(src->xselection.target);
            dst->xselection.property = from_ulong(src->xselection.property);
            dst->xselection.time = from_ulong(src->xselection.time);
            break;
        case XEVT_ColormapNotify:
            dst->xcolormap.colormap = from_ulong(src->xcolormap.colormap);
            dst->xcolormap.c_new = src->xcolormap.c_new;
            dst->xcolormap.state = src->xcolormap.state;
            break;
        case XEVT_ClientMessage:
            dst->xclient.message_type = from_ulong(src->xclient.message_type);
            dst->xclient.format = src->xclient.format;
            if(src->xclient.format==32)
                for(int i=0; i<5; ++i) dst->xclient.data.l[i] = from_ulong(src->xclient.data.l[i]);
            else
                memcpy(dst->xclient.data.b, src->xclient.data.b, 20);
            break;
        case XEVT_MappingNotify:
            dst->xmapping.request = src->xmapping.request;
            dst->xmapping.first_keycode = src->xmapping.first_keycode;
            dst->xmapping.count = src->xmapping.count;
            break;
        case XEVT_GenericEvent:
            dst->xgeneric.extension = src->xgeneric.extension;
            dst->xgeneric.evtype = src->xgeneric.evtype;
            break;

        default:
            printf_log(LOG_INFO, "Warning, unsupported 32bits (un)XEvent type=%d\n", src->type);
    }
}

void convert_XErrorEvent_to_32(void* d, void* s)
{
    my_XErrorEvent_t* src = s;
    my_XErrorEvent_32_t* dst = d;
    dst->type = src->type;
    dst->display = to_ptrv(FindDisplay(src->display));
    dst->resourceid = to_ulong(src->resourceid);
    dst->serial = to_ulong(src->serial);
    dst->error_code = src->error_code;
    dst->request_code = src->request_code;
    dst->minor_code = src->minor_code;
}
void convert_XErrorEvent_to_64(void* d, void* s)
{
    my_XErrorEvent_32_t* src = s;
    my_XErrorEvent_t* dst = d;
    dst->minor_code = src->minor_code;
    dst->request_code = src->request_code;
    dst->error_code = src->error_code;
    dst->serial = from_ulong(src->serial);
    dst->resourceid = from_ulong(src->resourceid);
    dst->display = getDisplay(from_ptrv(src->display));
    dst->type = src->type;
}

#define N_DISPLAY 4
#define N_SCREENS 16
my_XDisplay_t* my32_Displays_64[N_DISPLAY] = {0};
struct my_XFreeFuncs_32 my32_free_funcs_32[N_DISPLAY] = {0};
struct my_XLockPtrs_32 my32_lock_fns_32[N_DISPLAY] = {0};
my_Screen_32_t my32_screens[N_DISPLAY*N_SCREENS] = {0};
int n_screeens = 0;
my_XDisplay_32_t my32_Displays_32[N_DISPLAY] = {0};

void* getDisplay(void* d)
{
    if(!d) return d;
    for(int i=0; i<N_DISPLAY; ++i)
        if(((&my32_Displays_32[i])==d) || (my32_Displays_64[i]==d)) {
            return my32_Displays_64[i];
        }
        printf_log(LOG_INFO, "BOX32: Warning, 32bits Display %p not found\n", d);
    return d;
}

void* FindDisplay(void* d)
{
    if(!d) return d;
    for(int i=0; i<N_DISPLAY; ++i)
        if((my32_Displays_64[i]==d) || ((&my32_Displays_32[i])==d)) {
            return &my32_Displays_32[i];
        }
    return d;
}

void convert_Screen_to_32(void* d, void* s)
{
    my_Screen_t* src = s;
    my_Screen_32_t* dst = d;
    dst->ext_data = to_ptrv(src->ext_data);
    dst->display = to_ptrv(FindDisplay(src->display));
    dst->root = to_ulong(src->root);
    dst->width = src->width;
    dst->mwidth = src->mwidth;
    dst->ndepths = src->ndepths;
    dst->depths = to_ptrv(src->depths);
    dst->root_depth = src->root_depth;
    dst->root_visual = to_ptrv(src->root_visual);
    dst->default_gc = to_ptrv(src->default_gc);
    dst->cmap = to_ulong(src->cmap);
    dst->white_pixel = to_ulong(src->white_pixel);
    dst->black_pixel = to_ulong(src->black_pixel);
    dst->max_maps = src->max_maps;
    dst->backing_store = src->backing_store;
    dst->save_unders = src->save_unders;
    dst->root_input_mask = to_long(src->root_input_mask);
}

void* my_dlopen(x64emu_t* emu, void *filename, int flag);
void* addDisplay(void* d)
{
    if(!my_context->libx11) {
        // the lib has not be loaded directly... need to open it! leaking the lib handle...
        #ifdef ANDROID
        my_dlopen(thread_get_emu(), "libX11.so", RTLD_NOW);
        #else
        my_dlopen(thread_get_emu(), "libX11.so.6", RTLD_NOW);
        #endif
    }
    my_XDisplay_t* dpy = (my_XDisplay_t*)d;
    // look for a free slot, or a display already there
    my_XDisplay_32_t* ret = NULL;
    struct my_XFreeFuncs_32 *free_funcs = NULL;
    struct my_XLockPtrs_32 *lock_fns = NULL;
    for(int i=0; i<N_DISPLAY && !ret; ++i)
        if(my32_Displays_64[i]==dpy)
            return &my32_Displays_32[i];
    for(int i=0; i<N_DISPLAY && !ret; ++i) {
        if(!my32_Displays_64[i]) {
            my32_Displays_64[i] = dpy;
            ret = &my32_Displays_32[i];
            free_funcs = &my32_free_funcs_32[i];
            ret->free_funcs = to_ptrv(free_funcs);
            lock_fns = &my32_lock_fns_32[i];
            ret->lock_fns = to_ptrv(lock_fns);
        }
    }
    if(!ret) {
        printf_log(LOG_INFO, "BOX32: No more slot available for libX11 Display!");
        return d;
    }

    bridge_t* system = my_context->libx11->w.bridge;

    // partial copy...

    #define GO(A, W)\
    if(dpy->A)      \
        if(!CheckBridged(system, dpy->A)) \
            ret->A = AddCheckBridge(system, W, dpy->A, 0, #A); \

    #define GO2(A, B, W) \
    if(dpy->A && dpy->A->B)  \
        if(!CheckBridged(system, dpy->A->B)) \
            A->B = AddCheckBridge(system, W, dpy->A->B, 0, #B "_" #A); \

    ret->vendor = to_cstring(dpy->vendor);
    ret->fd = dpy->fd;
    ret->conn_checker = dpy->conn_checker;
    ret->proto_major_version = dpy->proto_major_version;
    ret->proto_minor_version = dpy->proto_minor_version;
    ret->xdefaults = to_cstring(dpy->xdefaults);
    ret->display_name = to_cstring(dpy->display_name);
    ret->default_screen = dpy->default_screen;
    ret->nscreens = dpy->nscreens;
    if(dpy->screens) {
        ret->screens = to_ptrv(&my32_screens[n_screeens]);
        for(int i=0; i<dpy->nscreens; ++i) {
            if(n_screeens==N_DISPLAY*N_SCREENS) {
                printf_log(LOG_INFO, "BOX32: Warning, no more libX11 Screen slots!");
                break;
            }
            convert_Screen_to_32(&my32_screens[n_screeens++], &dpy->screens[i]);
        }
    } else
        ret->screens = 0;

    GO2(free_funcs, atoms, vFp_32)
    GO2(free_funcs, modifiermap, iFp_32)
    GO2(free_funcs, key_bindings, vFp_32)
    GO2(free_funcs, context_db, vFp_32)
    GO2(free_funcs, defaultCCCs, vFp_32)
    GO2(free_funcs, clientCmaps, vFp_32)
    GO2(free_funcs, intensityMaps, vFp_32)
    GO2(free_funcs, im_filters, vFp_32)
    GO2(free_funcs, xkb, vFp_32)
    GO(resource_alloc, LFp_32)
    GO(synchandler, iFp_32)
    //TODO: ext_procs?
    //TODO: event_vec?
    //TODO: wire_vec?
    //TODO: async_handlers?
    GO2(lock_fns, lock_display, vFp_32)
    GO2(lock_fns, unlock_display, vFp_32)
    GO(idlist_alloc, vFppi_32)
    //TODO: error_vec?
    //TODO: flushes
    //TODO: im_fd_info?
    //TODO: conn_watchers
    GO(savedsynchandler, iFp_32)
    //TODO: generic_event_vec?
    //TODO: generic_event_copy_vec?


    #undef GO
    #undef GO2

    return ret;
}

void delDisplay(void* d)
{
    for(int i=0; i<N_DISPLAY; ++i) {
        // crude free of ressources... not perfect
        if(my32_Displays_64[i]==d) {
            my32_Displays_64[i] = NULL;
            return;
        }
    }
}

void convert_XWMints_to_64(void* d, void* s)
{
    my_XWMHints_t* dst = d;
    my_XWMHints_32_t* src = s;
    long flags = from_long(src->flags);
    // reverse order
    if(flags&XWMHint_WindowGroupHint)   dst->window_group = from_ulong(src->window_group);
    if(flags&XWMHint_IconMaskHint)      dst->icon_mask = from_ulong(src->icon_mask);
    if(flags&XWMHint_IconPositionHint)  {dst->icon_y = src->icon_y; dst->icon_x = src->icon_x;}
    if(flags&XWMHint_IconWindowHint)    dst->icon_window = from_ulong(src->icon_window);
    if(flags&XWMHint_IconPixmapHint)    dst->icon_pixmap = from_ulong(src->icon_pixmap);
    if(flags&XWMHint_StateHint)         dst->initial_state = src->initial_state;
    if(flags&XWMHint_InputHint)         dst->input = src->input;

    dst->flags = flags;
}
void inplace_enlarge_wmhints(void* hints)
{
    if(!hints) return;
    my_XWMHints_32_t* src = hints;
    my_XWMHints_t* dst = hints;
    long flags = from_long(src->flags);
    // reverse order
    if(flags&XWMHint_WindowGroupHint)   dst->window_group = from_ulong(src->window_group);
    if(flags&XWMHint_IconMaskHint)      dst->icon_mask = from_ulong(src->icon_mask);
    if(flags&XWMHint_IconPositionHint)  {dst->icon_y = src->icon_y; dst->icon_x = src->icon_x;}
    if(flags&XWMHint_IconWindowHint)    dst->icon_window = from_ulong(src->icon_window);
    if(flags&XWMHint_IconPixmapHint)    dst->icon_pixmap = from_ulong(src->icon_pixmap);
    if(flags&XWMHint_StateHint)         dst->initial_state = src->initial_state;
    if(flags&XWMHint_InputHint)         dst->input = src->input;

    dst->flags = flags;
}
void inplace_shrink_wmhints(void* hints)
{
    if(!hints) return;
    my_XWMHints_t* src = hints;
    my_XWMHints_32_t* dst = hints;
    long_t flags = to_long(src->flags);
    // forward order
    if(flags&XWMHint_InputHint)         dst->input = src->input;
    if(flags&XWMHint_StateHint)         dst->initial_state = src->initial_state;
    if(flags&XWMHint_IconPixmapHint)    dst->icon_pixmap = to_ulong(src->icon_pixmap);
    if(flags&XWMHint_IconWindowHint)    dst->icon_window = to_ulong(src->icon_window);
    if(flags&XWMHint_IconPositionHint)  {dst->icon_y = src->icon_y; dst->icon_x = src->icon_x;}
    if(flags&XWMHint_IconMaskHint)      dst->icon_mask = to_ulong(src->icon_mask);
    if(flags&XWMHint_WindowGroupHint)   dst->window_group = to_ulong(src->window_group);

    dst->flags = flags;
}

void convert_XSizeHints_to_64(void* d, void *s)
{
    //XSizeHints is a long flag and 17*int...
    long flags = to_long(*(long_t*)s);
    memcpy(d+8, s+4, 17*4);
    *(long*)d = flags;
}
void inplace_enlarge_wmsizehints(void* hints)
{
    //XSizeHints is a long flag and 17*int...
    long flags = to_long(*(long_t*)hints);
    memmove(hints+8, hints+4, 17*4);
    *(long*)hints = flags;
}
void inplace_shrink_wmsizehints(void* hints)
{
    //XSizeHints is a long flag and 17*int...
    long_t flags = from_long(*(long*)hints);
    memmove(hints+4, hints+8, 17*4);
    *(long_t*)hints = flags;
}

void convert_XWindowAttributes_to_32(void* d, void* s)
{
    my_XWindowAttributes_t* src = s;
    my_XWindowAttributes_32_t* dst = d;
    dst->x = src->x;
    dst->y = src->y;
    dst->width = src->width;
    dst->height = src->height;
    dst->border_width = src->border_width;
    dst->depth = src->depth;
    dst->visual = to_ptrv(src->visual);
    dst->root = to_ulong(src->root);
    dst->c_class = src->c_class;
    dst->bit_gravity = src->bit_gravity;
    dst->win_gravity = src->win_gravity;
    dst->backing_store = src->backing_store;
    dst->backing_planes = to_ulong(src->backing_planes);
    dst->backing_pixel = to_ulong(src->backing_pixel);
    dst->save_under = src->save_under;
    dst->colormap = to_ulong(src->colormap);
    dst->map_installed = src->map_installed;
    dst->map_state = src->map_state;
    dst->all_event_masks = to_long(src->all_event_masks);
    dst->your_event_mask = to_long(src->your_event_mask);
    dst->do_not_propagate_mask = to_long(src->do_not_propagate_mask);
    dst->override_redirect = src->override_redirect;
    dst->screen = to_ptrv(src->screen);
}

void inplace_XModifierKeymap_shrink(void* a)
{
    my_XModifierKeymap_32_t *d = a;
    my_XModifierKeymap_t* s = a;

    d->max_keypermod = s->max_keypermod;
    d->modifiermap = to_ptrv(s->modifiermap);
}
void inplace_XModifierKeymap_enlarge(void* a)
{
    my_XModifierKeymap_t *d = a;
    my_XModifierKeymap_32_t* s = a;

    d->modifiermap = from_ptrv(s->modifiermap);
    d->max_keypermod = s->max_keypermod;
}

void convert_XVisualInfo_to_32(my_XVisualInfo_32_t* dst, my_XVisualInfo_t* src)
{
    dst->visual = to_ptrv(src->visual);
    dst->visualid = to_ulong(src->visualid);
    dst->screen = src->screen;
    dst->depth = src->depth;
    dst->c_class = src->c_class;
    dst->red_mask = to_ulong(src->red_mask);
    dst->green_mask = to_ulong(src->green_mask);
    dst->blue_mask = to_ulong(src->blue_mask);
    dst->colormap_size = src->colormap_size;
    dst->bits_per_rgb = src->bits_per_rgb;
}
void convert_XVisualInfo_to_64(my_XVisualInfo_t* dst, my_XVisualInfo_32_t* src)
{
    dst->bits_per_rgb = src->bits_per_rgb;
    dst->colormap_size = src->colormap_size;
    dst->blue_mask = from_ulong(src->blue_mask);
    dst->green_mask = from_ulong(src->green_mask);
    dst->red_mask = from_ulong(src->red_mask);
    dst->c_class = src->c_class;
    dst->depth = src->depth;
    dst->screen = src->screen;
    dst->visualid = from_ulong(src->visualid);
    dst->visual = from_ptrv(src->visual);
}
void inplace_XVisualInfo_shrink(void *a)
{
    if(!a) return;
    my_XVisualInfo_t *src = a;
    my_XVisualInfo_32_t* dst = a;

    convert_XVisualInfo_to_32(dst, src);
}
void inplace_XVisualInfo_enlarge(void *a)
{
    if(!a) return;
    my_XVisualInfo_32_t *src = a;
    my_XVisualInfo_t* dst = a;

    convert_XVisualInfo_to_64(dst, src);
}

void inplace_XdbeVisualInfo_shrink(void* a)
{
    if(!a) return;
    my_XdbeVisualInfo_t *src = a;
    my_XdbeVisualInfo_32_t* dst = a;

    dst->visual = to_ulong(src->visual);
    dst->depth = src->depth;
    dst->perflevel = src->perflevel;
}
void inplace_XdbeScreenVisualInfo_shrink(void* a)
{
    if(!a) return;
    my_XdbeScreenVisualInfo_t *src = a;
    my_XdbeScreenVisualInfo_32_t* dst = a;

    for(int i=0; i<src->count; ++i)
        inplace_XdbeVisualInfo_shrink(src->visinfo+i);
    dst->count = src->count;
    dst->visinfo = to_ptrv(src->visinfo);
}
void inplace_XdbeVisualInfo_enlarge(void* a)
{
    if(!a) return;
    my_XdbeVisualInfo_32_t *src = a;
    my_XdbeVisualInfo_t* dst = a;

    dst->perflevel = src->perflevel;
    dst->depth = src->depth;
    dst->visual = from_ulong(src->visual);
}
void inplace_XdbeScreenVisualInfo_enlarge(void* a)
{
    if(!a) return;
    my_XdbeScreenVisualInfo_32_t *src = a;
    my_XdbeScreenVisualInfo_t* dst = a;

    dst->visinfo = from_ptrv(src->visinfo);
    dst->count = src->count;
    for(int i=dst->count-1; i>=0; --i)
        inplace_XdbeVisualInfo_enlarge(dst->visinfo+i);
}

void inplace_XExtDisplayInfo_shrink(void* a)
{
    if(!a) return;
    my_XExtDisplayInfo_t* src = a;
    my_XExtDisplayInfo_32_t* dst = a;

    dst->next = to_ptrv(src->next);
    dst->display = to_ptrv(getDisplay(src->display));
    dst->codes = to_ptrv(src->codes);
    dst->data = to_ptrv(src->data);
}
void inplace_XExtDisplayInfo_enlarge(void* a)
{
    if(!a) return;
    my_XExtDisplayInfo_32_t* src = a;
    my_XExtDisplayInfo_t* dst = a;

    dst->data = from_ptrv(src->data);
    dst->codes = from_ptrv(src->codes);
    dst->display = FindDisplay(from_ptrv(src->display));
    dst->next = from_ptrv(src->next);
}

void* inplace_XExtensionInfo_shrink(void* a)
{
    if(!a) return a;
    my_XExtensionInfo_t* src = a;
    my_XExtensionInfo_32_t* dst = a;

    my_XExtDisplayInfo_t* head = src->head;
    while(head) {
        my_XExtDisplayInfo_t* next = head->next;
        inplace_XExtDisplayInfo_shrink(head);
        head = next;
    }
    dst->head = to_ptrv(src->head);
    dst->cur = to_ptrv(src->cur);
    dst->ndisplays = src->ndisplays;
    return a;
}
void* inplace_XExtensionInfo_enlarge(void* a)
{
    if(!a) return a;
    my_XExtensionInfo_32_t* src = a;
    my_XExtensionInfo_t* dst = a;

    dst->ndisplays = src->ndisplays;
    dst->cur = from_ptrv(src->cur);
    dst->head = from_ptrv(src->head);
    my_XExtDisplayInfo_t* head = dst->head;
    while(head) {
        inplace_XExtDisplayInfo_enlarge(head);
        head = head->next;
    }
    return a;
}

void convert_XFontProp_to_32(my_XFontProp_32_t* dst, my_XFontProp_t* src)
{
    dst->name = to_ulong(src->name);
    dst->card32 = to_ulong(src->card32);
}
void inplace_XFontProp_shrink(void* a)
{
    if(!a) return;
    my_XFontProp_t* src = a;
    my_XFontProp_32_t* dst = a;
    convert_XFontProp_to_32(dst, src);
}
void convert_XFontProp_to_64(my_XFontProp_t* dst, my_XFontProp_32_t* src)
{
    dst->card32 = from_ulong(src->card32);
    dst->name = from_ulong(src->name);
}
void inplace_XFontProp_enlarge(void* a)
{
    if(!a) return;
    my_XFontProp_32_t* src = a;
    my_XFontProp_t* dst = a;
    convert_XFontProp_to_64(dst, src);
}
void inplace_XFontStruct_shrink(void* a)
{
    if(!a) return;
    my_XFontStruct_t* src = a;
    my_XFontStruct_32_t* dst = a;

    my_XFontProp_32_t* properties_s = (my_XFontProp_32_t*)src->properties;
    for(int i=0; i<src->n_properties; ++i)
        convert_XFontProp_to_32(properties_s+i, src->properties+i);

    dst->ext_data = to_ptrv(src->ext_data);
    dst->fid = to_ulong(src->fid);
    dst->direction = src->direction;
    dst->min_char_or_byte2 = src->min_char_or_byte2;
    dst->max_char_or_byte2 = src->max_char_or_byte2;
    dst->min_byte1 = src->min_byte1;
    dst->max_byte1 = src->max_byte1;
    dst->all_chars_exist = src->all_chars_exist;
    dst->default_char = src->default_char;
    dst->n_properties = src->n_properties;
    dst->properties = to_ptrv(src->properties);
    memmove(&dst->min_bounds, &src->min_bounds, sizeof(dst->min_bounds));
    memmove(&dst->max_bounds, &src->max_bounds, sizeof(dst->max_bounds));
    dst->per_char = to_ptrv(src->per_char);
    dst->ascent = src->ascent;
    dst->descent = src->descent;
}
void inplace_XFontStruct_enlarge(void* a)
{
    if(!a) return;
    my_XFontStruct_32_t* src = a;
    my_XFontStruct_t* dst = a;

    dst->descent = src->descent;
    dst->ascent = src->ascent;
    dst->per_char = from_ptrv(src->per_char);
    memmove(&dst->max_bounds, &src->max_bounds, sizeof(dst->max_bounds));
    memmove(&dst->min_bounds, &src->min_bounds, sizeof(dst->min_bounds));
    dst->properties = from_ptrv(src->properties);
    dst->n_properties = src->n_properties;
    dst->default_char = src->default_char;
    dst->all_chars_exist = src->all_chars_exist;
    dst->max_byte1 = src->max_byte1;
    dst->min_byte1 = src->min_byte1;
    dst->max_char_or_byte2 = src->max_char_or_byte2;
    dst->min_char_or_byte2 = src->min_char_or_byte2;
    dst->direction = src->direction;
    dst->fid = from_ulong(src->fid);
    dst->ext_data = from_ptrv(src->ext_data);

    my_XFontProp_32_t* properties_s = (my_XFontProp_32_t*)dst->properties;
    for(int i=dst->n_properties-1; i>=0;  --i)
        convert_XFontProp_to_64(dst->properties+i, properties_s+i);
}

void convert_XSetWindowAttributes_to_64(my_XSetWindowAttributes_t* dst, my_XSetWindowAttributes_32_t* src)
{
    dst->background_pixmap = from_ulong(src->background_pixmap);
    dst->background_pixel = from_ulong(src->background_pixel);
    dst->border_pixmap = from_ulong(src->border_pixmap);
    dst->border_pixel = from_ulong(src->border_pixel);
    dst->bit_gravity = src->bit_gravity;
    dst->win_gravity = src->win_gravity;
    dst->backing_store = from_ulong(src->backing_store);
    dst->backing_planes = from_ulong(src->backing_planes);
    dst->backing_pixel = src->backing_pixel;
    dst->save_under = src->save_under;
    dst->event_mask = from_long(src->event_mask);
    dst->do_not_propagate_mask = from_long(src->do_not_propagate_mask);
    dst->override_redirect = src->override_redirect;
    dst->colormap = from_ulong(src->colormap);
    dst->cursor = from_ulong(src->cursor);
}

void* inplace_XImage_shrink(void* a)
{
    if(!a) return NULL;
    WrapXImage(a, a);
    return a;
}
void* inplace_XImage_enlarge(void* a)
{
    if(!a) return NULL;
    UnwrapXImage(a, a);
    return a;
}

void convert_XRRModeInfo_to_32(void* d, const void* s)
{
    my_XRRModeInfo_32_t *dst = d;
    const my_XRRModeInfo_t *src = s;
    dst->id = to_ulong(src->id);
    dst->width = src->width;
    dst->height = src->height;
    dst->dotClock = to_ulong(src->dotClock);
    dst->hSyncStart = src->hSyncStart;
    dst->hSyncEnd = src->hSyncEnd;
    dst->hTotal = src->hTotal;
    dst->hSkew = src->hSkew;
    dst->vSyncStart = src->vSyncStart;
    dst->vSyncEnd = src->vSyncEnd;
    dst->vTotal = src->vTotal;
    dst->name = to_ptrv(src->name);
    dst->nameLength = src->nameLength;
    dst->modeFlags = to_ulong(src->modeFlags);
}

void convert_XRRModeInfo_to_64(void* d, const void* s)
{
    my_XRRModeInfo_t *dst = d;
    const my_XRRModeInfo_32_t *src = s;
    dst->modeFlags = from_ulong(src->modeFlags);
    dst->nameLength = src->nameLength;
    dst->name = from_ptrv(src->name);
    dst->vTotal = src->vTotal;
    dst->vSyncEnd = src->vSyncEnd;
    dst->vSyncStart = src->vSyncStart;
    dst->hSkew = src->hSkew;
    dst->hTotal = src->hTotal;
    dst->hSyncEnd = src->hSyncEnd;
    dst->hSyncStart = src->hSyncStart;
    dst->dotClock = from_ulong(src->dotClock);
    dst->height = src->height;
    dst->width = src->width;
    dst->id = from_ulong(src->id);
}

void inplace_XRRScreenResources_shrink(void* s)
{
    if(!s) return;
    my_XRRScreenResources_32_t *dst = s;
    my_XRRScreenResources_t *src = s;
    // shrinking, so forward...
    dst->timestamp = to_ulong(src->timestamp);
    dst->configTimestamp = to_ulong(src->configTimestamp);
    dst->ncrtc = src->ncrtc;
    for(int i=0; i<dst->ncrtc; ++i)
        ((XID_32*)src->crtcs)[i] = to_ulong(src->crtcs[i]);
    dst->crtcs = to_ptrv(src->crtcs);
    dst->noutput = src->noutput;
    for(int i=0; i<dst->noutput; ++i)
        ((XID_32*)src->outputs)[i] = to_ulong(src->outputs[i]);
    dst->outputs = to_ptrv(src->outputs);
    dst->nmode = src->nmode;
    for(int i=0; i<dst->noutput; ++i)
        convert_XRRModeInfo_to_32(&((my_XRRModeInfo_32_t*)src->modes)[i], &src->modes[i]);
    dst->modes = to_ptrv(src->modes);
}

void inplace_XRRScreenResources_enlarge(void* s)
{
    if(!s) return;
    my_XRRScreenResources_t *dst = s;
    my_XRRScreenResources_32_t *src = s;
    // enlarge, so backward...
    int nmode = src->nmode;
    int noutput = src->noutput;
    int ncrtc = src->ncrtc;
    dst->modes = from_ptrv(src->modes);
    for(int i=nmode-1; i>=0; --i)
        convert_XRRModeInfo_to_64(&dst->modes[i], &((my_XRRModeInfo_32_t*)dst->modes)[i]);
    dst->nmode = src->nmode;
    dst->outputs = from_ptrv(src->outputs);
    for(int i=noutput-1; i>=0; --i)
        dst->outputs[i] = from_ulong(((XID_32*)dst->outputs)[i]);
    dst->noutput = src->noutput;
    dst->crtcs = from_ptrv(src->crtcs);
    for(int i=ncrtc-1; i>=0; --i)
        dst->crtcs[i] = from_ulong(((XID_32*)dst->crtcs)[i]);
    dst->ncrtc = src->ncrtc;
    dst->configTimestamp = to_ulong(src->configTimestamp);
    dst->timestamp = to_ulong(src->timestamp);
}

void inplace_XRRCrtcInfo_shrink(void* s)
{
    if(!s) return;
    my_XRRCrtcInfo_32_t *dst = s;
    my_XRRCrtcInfo_t *src = s;
    dst->timestamp = to_ulong(src->timestamp);
    dst->x = src->x;
    dst->y = src->y;
    dst->width = src->width;
    dst->height = src->height;
    dst->mode = to_ulong(src->mode);
    dst->rotation = src->rotation;
    for(int i=0; i<src->noutput; ++i)
        ((XID_32*)src->outputs)[i] = to_ulong(src->outputs[i]);
    dst->noutput = src->noutput;
    dst->outputs = to_ptrv(src->outputs);
    dst->rotations = src->rotations;
    dst->npossible = src->npossible;
    for(int i=0; i<dst->npossible; ++i)
        ((XID_32*)src->possible)[i] = to_ulong(src->possible[i]);
    dst->possible = to_ptrv(src->possible);
}

void inplace_XRROutputInfo_shrink(void* s)
{
    if(!s) return;
    my_XRROutputInfo_32_t *dst = s;
    my_XRROutputInfo_t *src = s;
    dst->timestamp = to_ulong(src->timestamp);
    dst->crtc = src->crtc;
    dst->name = to_ptrv(src->name);
    dst->nameLen = src->nameLen;
    dst->mm_width = to_ulong(src->mm_width);
    dst->mm_height = to_ulong(src->mm_height);
    dst->connection = src->connection;
    dst->subpixel_order = src->subpixel_order;
    dst->ncrtc = src->ncrtc;
    for(int i=0; i<dst->ncrtc; ++i)
        ((XID_32*)src->crtcs)[i] = to_ulong(src->crtcs[i]);
    dst->crtcs = to_ptrv(src->crtcs);
    dst->nclone = src->nclone;
    for(int i=0; i<dst->nclone; ++i)
        ((XID_32*)src->clones)[i] = to_ulong(src->clones[i]);
    dst->clones = to_ptrv(src->clones);
    dst->nmode = src->nmode;
    dst->npreferred = src->npreferred;
    for(int i=0; i<dst->nmode; ++i)
        ((XID_32*)src->modes)[i] = to_ulong(src->modes[i]);
    dst->modes = to_ptrv(src->modes);
}

void inplace_XRRProviderInfo_shrink(void* a)
{
    if(!a) return;
    my_XRRProviderInfo_32_t *dst = a;
    my_XRRProviderInfo_t* src = a;

    for(int i=0; i<src->ncrtcs; ++i)
        ((ulong_t*)src->crtcs)[i] = to_ulong(src->crtcs[i]);
    for(int i=0; i<src->noutputs; ++i)
        ((ulong_t*)src->outputs)[i] = to_ulong(src->outputs[i]);
    for(int i=0; i<src->nassociatedproviders; ++i)
        ((ulong_t*)src->associated_providers)[i] = to_ulong(src->associated_providers[i]);
    dst->capabilities = src->capabilities;
    dst->ncrtcs = src->ncrtcs;
    dst->crtcs = to_ptrv(src->crtcs);
    dst->noutputs = src->noutputs;
    dst->outputs = to_ptrv(src->outputs);
    dst->name = to_ptrv(src->name);
    dst->nassociatedproviders = src->nassociatedproviders;
    dst->associated_providers = to_ptrv(src->associated_providers);
    dst->associated_capability = to_ptrv(src->associated_capability);
    dst->nameLen = src->nameLen;
}
void inplace_XRRProviderInfo_enlarge(void* a)
{
    if(!a) return;
    my_XRRProviderInfo_t *dst = a;
    my_XRRProviderInfo_32_t* src = a;

    dst->nameLen = src->nameLen;
    dst->associated_capability = from_ptrv(src->associated_capability);
    dst->associated_providers = from_ptrv(src->associated_providers);
    dst->nassociatedproviders = src->nassociatedproviders;
    dst->name = from_ptrv(src->name);
    dst->outputs = from_ptrv(src->outputs);
    dst->noutputs = src->noutputs;
    dst->crtcs = from_ptrv(src->crtcs);
    dst->ncrtcs = src->ncrtcs;
    dst->capabilities = src->capabilities;
    for(int i=dst->ncrtcs-1; i>=0; --i)
        dst->crtcs[i] = from_ulong(((ulong_t*)dst->crtcs)[i]);
    for(int i=dst->noutputs-1; i>=0; --i)
        dst->outputs[i] = from_ulong(((ulong_t*)dst->outputs)[i]);
    for(int i=dst->nassociatedproviders-1; i>=0; --i)
        dst->associated_providers[i] = from_ulong(((ulong_t*)dst->associated_providers)[i]);
}

void inplace_XRRProviderResources_shrink(void* a)
{
    if(!a) return;
    my_XRRProviderResources_32_t* dst = a;
    my_XRRProviderResources_t* src = a;

    for(int i=0; i<src->nproviders; ++i)
        ((ulong_t*)src->providers)[i] = to_ulong(src->providers[i]);
    dst->timestamp = to_long(src->timestamp);
    dst->nproviders = src->nproviders;
    dst->providers = to_ptrv(src->providers);
}
void inplace_XRRProviderResources_enlarge(void* a)
{
    if(!a) return;
    my_XRRProviderResources_t* dst = a;
    my_XRRProviderResources_32_t* src = a;

    dst->timestamp = from_long(src->timestamp);
    dst->nproviders = src->nproviders;
    dst->providers = from_ptrv(src->providers);
    for(int i=dst->nproviders-1; i>=0; --i)
        dst->providers[i] = from_ulong(((ulong_t*)dst->providers)[i]);
}

void* inplace_XRRPropertyInfo_shrink(void* a)
{
    if(!a) return NULL;
    my_XRRPropertyInfo_t* src = a;
    my_XRRPropertyInfo_32_t* dst = a;

    for(int i=0; i<src->num_values; ++i)
        ((long_t*)src->values)[i] = to_long(src->values[i]);
    dst->pending = src->pending;
    dst->range = src->range;
    dst->immutable = src->immutable;
    dst->num_values = src->num_values;
    dst->values = to_ptrv(src->values);

    return a;
}