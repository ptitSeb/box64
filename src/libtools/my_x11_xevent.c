#include <stdint.h>
#include <stddef.h>
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

typedef void(*convertXEvent_to_32)(my_XEvent_32_t* dst, my_XEvent_t* src);
typedef void(*convertXEvent_to_64)(my_XEvent_t* dst, my_XEvent_32_t* src);

typedef struct reg_event_s {
    uint32_t event;
    convertXEvent_to_32 to32;
    convertXEvent_to_64 to64;
} reg_event_t;
typedef struct register_events_s {
    void* id;
    uint32_t start_event;
    uint32_t end_event;
    size_t n;
    reg_event_t* events;
    struct register_events_s* next;
} register_events_t;

static register_events_t* register_events_head = NULL;

extern int my32_xinput_opcode;

void convertXEvent(my_XEvent_32_t* dst, my_XEvent_t* src)
{
    if(!src->type) {
        // This is an XErrorEvent, and it's different!
        dst->xerror.type = src->xerror.type;
        // use the silent version here because sometimes src->xerror points to some garbage (we don't know why!)
        dst->xerror.display = to_ptrv_silent(FindDisplay(src->xerror.display));
        dst->xerror.resourceid = to_ulong_silent(src->xerror.resourceid);
        dst->xerror.serial = to_ulong_silent(src->xerror.serial);
        dst->xerror.error_code = src->xerror.error_code;
        dst->xerror.request_code = src->xerror.request_code;
        dst->xerror.minor_code = src->xerror.minor_code;
        return;
    }
    // convert the XAnyEvent first, as it's a common set
    int type = src->type;
    dst->type = src->type;
    dst->xany.serial = to_ulong(src->xany.serial);
    dst->xany.send_event = src->xany.serial;
    dst->xany.display = to_ptrv(FindDisplay(src->xany.display));
    if(type!=XEVT_GenericEvent)
        dst->xany.window = to_ulong(src->xany.window);
    switch(type) {
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
            dst->xcookie.extension = src->xcookie.extension;
            dst->xcookie.evtype = src->xcookie.evtype;
            dst->xcookie.cookie = src->xcookie.cookie;
            dst->xcookie.data = to_ptrv_silent(src->xcookie.data);  // in case data are not initialized
            break;
        default: {
            register_events_t* head = register_events_head;
            while(head) {
                if(type>=head->start_event && type<=head->end_event) {
                    for(int i=0; i<head->n; ++i)
                        if(type==head->events[i].event) {
                            head->events[i].to32(dst, src);
                            return;
                        }
                }
                head = head->next;
            }
            printf_log(LOG_INFO, "Warning, unsupported 32bits XEvent type=%d\n", type);
        }    
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
    int type = src->type;
    dst->type = src->type;
    if(type!=XEVT_GenericEvent)
        dst->xany.window = from_ulong(src->xany.window);
    dst->xany.display = getDisplay(from_ptrv(src->xany.display));
    dst->xany.send_event = src->xany.serial;
    dst->xany.serial = from_ulong(src->xany.serial);
    switch(type) {
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
            dst->xcookie.extension = src->xcookie.extension;
            dst->xcookie.evtype = src->xcookie.evtype;
            dst->xcookie.cookie = src->xcookie.cookie;
            dst->xcookie.data = from_ptrv(src->xcookie.data);
            break;

        default: {
            register_events_t* head = register_events_head;
            while(head) {
                if(type>=head->start_event && type<=head->end_event) {
                    for(int i=0; i<head->n; ++i)
                        if(type==head->events[i].event) {
                            head->events[i].to64(dst, src);
                            return;
                        }
                }
                head = head->next;
            }
            printf_log(LOG_INFO, "Warning, unsupported 32bits (un)XEvent type=%d\n", type);
        }
    }
}

void inplace_XEventData_shring(my_XEvent_t* evt)
{
    if(!evt || evt->type!=XEVT_GenericEvent)
        return;
    if(my32_xinput_opcode && evt->xcookie.extension==my32_xinput_opcode) {
        //XInput2 event
        // convert Generic event 1st, as it's common
        my_XIEvent_t *s = evt->xcookie.data;
        my_XIEvent_32_t* d = evt->xcookie.data;
        d->type = s->type;
        d->serial = to_ulong(s->serial);
        d->send_event = s->send_event;
        d->display = to_ptrv(FindDisplay(s->display));
        d->extension = s->extension;
        d->evtype = s->evtype;
        d->time = to_ulong(s->time);
        switch(evt->xcookie.evtype) {
            case XI_DeviceChanged: {
                my_XIDeviceChangedEvent_t* s = evt->xcookie.data;
                my_XIDeviceChangedEvent_32_t* d = evt->xcookie.data;
                d->deviceid = s->deviceid;
                d->sourceid = s->sourceid;
                d->reason = s->reason;
                for(int j=0; j<s->num_classes; ++j)
                    ((ptr_t*)s->classes)[j] = to_ptrv(s->classes[j]);
                d->num_classes = s->num_classes;
                d->classes = to_ptrv(s->classes);
            }
            break;
            case XI_KeyPress:
            case XI_KeyRelease:
            case XI_ButtonPress:
            case XI_ButtonRelease:
            case XI_Motion:
            case XI_TouchBegin:
            case XI_TouchUpdate:
            case XI_TouchEnd:
            {
                my_XIDeviceEvent_t* s = evt->xcookie.data;
                my_XIDeviceEvent_32_t* d = evt->xcookie.data;
                d->deviceid = s->deviceid;
                d->sourceid = s->sourceid;
                d->detail = s->detail;
                d->root = to_ulong(s->root);
                d->event = to_ulong(s->event);
                d->child = to_ulong(s->child);
                d->root_x = s->root_x;
                d->root_y = s->root_y;
                d->event_x = s->event_x;
                d->event_y = s->event_y;
                d->flags = s->flags;
                d->buttons.mask_len = s->buttons.mask_len;
                d->buttons.mask = to_ptrv(s->buttons.mask);
                d->valuators.mask_len = s->valuators.mask_len;
                d->valuators.mask = to_ptrv(s->valuators.mask);
                d->valuators.values = to_ptrv(s->valuators.values);
                d->mods = s->mods;
                d->group = s->group;
            }
            break;
            case XI_Enter:
            case XI_Leave:
            case XI_FocusIn:
            case XI_FocusOut:
            {
                my_XIEnterEvent_t* s = evt->xcookie.data;
                my_XIEnterEvent_32_t* d = evt->xcookie.data;
                d->deviceid = s->deviceid;
                d->sourceid = s->sourceid;
                d->detail = s->detail;
                d->root = to_ulong(s->root);
                d->event = to_ulong(s->event);
                d->child = to_ulong(s->child);
                d->root_x = s->root_x;
                d->root_y = s->root_y;
                d->event_x = s->event_x;
                d->event_y = s->event_y;
                d->mode = s->mode;
                d->focus = s->focus;
                d->same_screen = s->same_screen;
                d->buttons.mask_len = s->buttons.mask_len;
                d->buttons.mask = to_ptrv(s->buttons.mask);
                d->mods = s->mods;
                d->group = s->group;
            }
            break;
            case XI_HierarchyChanged: {
                my_XIHierarchyEvent_t* s = evt->xcookie.data;
                my_XIHierarchyEvent_32_t* d = evt->xcookie.data;
                d->flags = s->flags;
                d->num_info = s->num_info;
                d->info = to_ptrv(s->info);
            }
            break;
            case XI_RawKeyPress:
            case XI_RawKeyRelease:
            case XI_RawButtonPress:
            case XI_RawButtonRelease:
            case XI_RawMotion:
            case XI_RawTouchBegin:
            case XI_RawTouchUpdate:
            case XI_RawTouchEnd:
            {
                my_XIRawEvent_t* s = evt->xcookie.data;
                my_XIRawEvent_32_t* d = evt->xcookie.data;
                d->deviceid = s->deviceid;
                d->sourceid = s->sourceid;
                d->detail = s->detail;
                d->flags = s->flags;
                d->valuators.mask_len = s->valuators.mask_len;
                d->valuators.mask = to_ptrv(s->valuators.mask);
                d->valuators.values = to_ptrv(s->valuators.values);
                d->raw_values = to_ptrv(s->raw_values);
            }
            break;
            case XI_TouchOwnership: {
                my_XITouchOwnershipEvent_t* s = evt->xcookie.data;
                my_XITouchOwnershipEvent_32_t* d = evt->xcookie.data;
                d->deviceid = s->deviceid;
                d->sourceid = s->sourceid;
                d->touchid = s->touchid;
                d->root = to_ulong(s->root);
                d->event = to_ulong(s->event);
                d->child = to_ulong(s->child);
                d->flags = s->flags;
            }
            break;
            case XI_BarrierHit:
            case XI_BarrierLeave:
            {
                my_XIBarrierEvent_t* s = evt->xcookie.data;
                my_XIBarrierEvent_32_t* d = evt->xcookie.data;
                d->deviceid = s->deviceid;
                d->sourceid = s->sourceid;
                d->event = to_ulong(s->event);
                d->root = to_ulong(s->root);
                d->root_x = s->root_x;
                d->root_y = s->root_y;
                d->dx = s->dx;
                d->dy = s->dy;
                d->dtime = s->dtime;
                d->flags = s->flags;
                d->barrier = to_ulong(s->barrier);
                d->eventid = s->eventid;
            }
            break;
            case XI_GesturePinchBegin:
            case XI_GesturePinchUpdate:
            case XI_GesturePinchEnd:
            {
                my_XIGesturePinchEvent_t* s = evt->xcookie.data;
                my_XIGesturePinchEvent_32_t* d = evt->xcookie.data;
                d->deviceid = s->deviceid;
                d->sourceid = s->sourceid;
                d->detail = s->detail;
                d->root = to_ulong(s->root);
                d->event = to_ulong(s->event);
                d->child = to_ulong(s->child);
                d->root_x = s->root_x;
                d->root_y = s->root_y;
                d->event_x = s->event_x;
                d->event_y = s->event_y;
                d->delta_x = s->delta_x;
                d->delta_y = s->delta_y;
                d->delta_unaccel_x = s->delta_unaccel_x;
                d->delta_unaccel_y = s->delta_unaccel_y;
                d->scale = s->scale;
                d->delta_angle = s->delta_angle;
                d->flags = s->flags;
                d->mods = s->mods;
                d->group = s->group;
            }
            break;
            case XI_GestureSwipeBegin:
            case XI_GestureSwipeUpdate:
            case XI_GestureSwipeEnd:
            {
                my_XIGestureSwipeEvent_t* s = evt->xcookie.data;
                my_XIGestureSwipeEvent_32_t* d = evt->xcookie.data;
                d->deviceid = s->deviceid;
                d->sourceid = s->sourceid;
                d->detail = s->detail;
                d->root = to_ulong(s->root);
                d->event = to_ulong(s->event);
                d->child = to_ulong(s->child);
                d->root_x = s->root_x;
                d->root_y = s->root_y;
                d->event_x = s->event_x;
                d->event_y = s->event_y;
                d->delta_x = s->delta_x;
                d->delta_y = s->delta_y;
                d->delta_unaccel_x = s->delta_unaccel_x;
                d->delta_unaccel_y = s->delta_unaccel_y;
                d->flags = s->flags;
                d->mods = s->mods;
                d->group = s->group;
            }
            break;
            default:
                printf_log(LOG_INFO, "Warning, unsupported 32bits XIEvent type=%d\n", evt->xcookie.evtype);
                break;
        }
    }
}
void inplace_XEventData_enlarge(my_XEvent_t* evt)
{
    if(!evt || evt->type!=XEVT_GenericEvent)
        return;
    if(my32_xinput_opcode && evt->xcookie.extension==my32_xinput_opcode) {
        switch(evt->xcookie.evtype) {
            case XI_DeviceChanged: {
                my_XIDeviceChangedEvent_32_t* s = evt->xcookie.data;
                my_XIDeviceChangedEvent_t* d = evt->xcookie.data;
                d->classes = from_ptrv(s->classes);
                d->num_classes = s->num_classes;
                d->sourceid = s->sourceid;
                d->deviceid = s->deviceid;
                d->reason = s->reason;
                for(int j=d->num_classes-1; j>=0; --j)
                    d->classes[j] = from_ptrv(((ptr_t*)d->classes)[j]);
            }
            break;
            case XI_KeyPress:
            case XI_KeyRelease:
            case XI_ButtonPress:
            case XI_ButtonRelease:
            case XI_Motion:
            case XI_TouchBegin:
            case XI_TouchUpdate:
            case XI_TouchEnd:
            {
                my_XIDeviceEvent_32_t* s = evt->xcookie.data;
                my_XIDeviceEvent_t* d = evt->xcookie.data;
                d->group = s->group;
                d->mods = s->mods;
                d->valuators.values = from_ptrv(s->valuators.values);
                d->valuators.mask = from_ptrv(s->valuators.mask);
                d->valuators.mask_len = s->valuators.mask_len;
                d->buttons.mask = from_ptrv(s->buttons.mask);
                d->buttons.mask_len = s->buttons.mask_len;
                d->flags = s->flags;
                d->event_y = s->event_y;
                d->event_x = s->event_x;
                d->root_y = s->root_y;
                d->root_x = s->root_x;
                d->child = from_ulong(s->child);
                d->event = from_ulong(s->event);
                d->root = from_ulong(s->root);
                d->detail = s->detail;
                d->sourceid = s->sourceid;
                d->deviceid = s->deviceid;
            }
            break;
            case XI_Enter:
            case XI_Leave:
            case XI_FocusIn:
            case XI_FocusOut:
            {
                my_XIEnterEvent_32_t* s = evt->xcookie.data;
                my_XIEnterEvent_t* d = evt->xcookie.data;
                d->group = s->group;
                d->mods = s->mods;
                d->buttons.mask = from_ptrv(s->buttons.mask);
                d->buttons.mask_len = s->buttons.mask_len;
                d->same_screen = s->same_screen;
                d->focus = s->focus;
                d->mode = s->mode;
                d->event_y = s->event_y;
                d->event_x = s->event_x;
                d->root_y = s->root_y;
                d->root_x = s->root_x;
                d->child = from_ulong(s->child);
                d->event = from_ulong(s->event);
                d->root = from_ulong(s->root);
                d->detail = s->detail;
                d->sourceid = s->sourceid;
                d->deviceid = s->deviceid;
            }
            break;
            case XI_HierarchyChanged: {
                my_XIHierarchyEvent_32_t* s = evt->xcookie.data;
                my_XIHierarchyEvent_t* d = evt->xcookie.data;
                d->info = from_ptrv(s->info);
                d->num_info = s->num_info;
                d->flags = s->flags;
            }
            break;
            case XI_RawKeyPress:
            case XI_RawKeyRelease:
            case XI_RawButtonPress:
            case XI_RawButtonRelease:
            case XI_RawMotion:
            case XI_RawTouchBegin:
            case XI_RawTouchUpdate:
            case XI_RawTouchEnd:
            {
                my_XIRawEvent_32_t* s = evt->xcookie.data;
                my_XIRawEvent_t* d = evt->xcookie.data;
                d->raw_values = from_ptrv(s->raw_values);
                d->valuators.values = from_ptrv(s->valuators.values);
                d->valuators.mask = from_ptrv(s->valuators.mask);
                d->valuators.mask_len = s->valuators.mask_len;
                d->flags = s->flags;
                d->detail = s->detail;
                d->sourceid = s->sourceid;
                d->deviceid = s->deviceid;
            }
            break;
            case XI_TouchOwnership: {
                my_XITouchOwnershipEvent_32_t* s = evt->xcookie.data;
                my_XITouchOwnershipEvent_t* d = evt->xcookie.data;
                d->flags = s->flags;
                d->child = from_ulong(s->child);
                d->event = from_ulong(s->event);
                d->root = from_ulong(s->root);
                d->touchid = s->touchid;
                d->sourceid = s->sourceid;
                d->deviceid = s->deviceid;
            }
            break;
            case XI_BarrierHit:
            case XI_BarrierLeave:
            {
                my_XIBarrierEvent_32_t* s = evt->xcookie.data;
                my_XIBarrierEvent_t* d = evt->xcookie.data;
                d->eventid = s->eventid;
                d->barrier = from_ulong(s->barrier);
                d->flags = s->flags;
                d->dtime = s->dtime;
                d->dy = s->dy;
                d->dx = s->dx;
                d->root_y = s->root_y;
                d->root_x = s->root_x;
                d->root = from_ulong(s->root);
                d->event = from_ulong(s->event);
                d->sourceid = s->sourceid;
                d->deviceid = s->deviceid;
            }
            break;
            case XI_GesturePinchBegin:
            case XI_GesturePinchUpdate:
            case XI_GesturePinchEnd:
            {
                my_XIGesturePinchEvent_32_t* s = evt->xcookie.data;
                my_XIGesturePinchEvent_t* d = evt->xcookie.data;
                d->group = s->group;
                d->mods = s->mods;
                d->flags = s->flags;
                d->delta_angle = s->delta_angle;
                d->scale = s->scale;
                d->delta_unaccel_y = s->delta_unaccel_y;
                d->delta_unaccel_x = s->delta_unaccel_x;
                d->delta_y = s->delta_y;
                d->delta_x = s->delta_x;
                d->event_y = s->event_y;
                d->event_x = s->event_x;
                d->root_y = s->root_y;
                d->root_x = s->root_x;
                d->child = from_ulong(s->child);
                d->event = from_ulong(s->event);
                d->root = from_ulong(s->root);
                d->detail = s->detail;
                d->sourceid = s->sourceid;
                d->deviceid = s->deviceid;
            }
            break;
            case XI_GestureSwipeBegin:
            case XI_GestureSwipeUpdate:
            case XI_GestureSwipeEnd:
            {
                my_XIGestureSwipeEvent_32_t* s = evt->xcookie.data;
                my_XIGestureSwipeEvent_t* d = evt->xcookie.data;
                d->group = s->group;
                d->mods = s->mods;
                d->flags = s->flags;
                d->delta_unaccel_y = s->delta_unaccel_y;
                d->delta_unaccel_x = s->delta_unaccel_x;
                d->delta_y = s->delta_y;
                d->delta_x = s->delta_x;
                d->event_y = s->event_y;
                d->event_x = s->event_x;
                d->root_y = s->root_y;
                d->root_x = s->root_x;
                d->child = from_ulong(s->child);
                d->event = from_ulong(s->event);
                d->root = from_ulong(s->root);
                d->detail = s->detail;
                d->sourceid = s->sourceid;
                d->deviceid = s->deviceid;
            }
            break;
            default:
                printf_log(LOG_INFO, "Warning, unsupported 32bits (un)XIEvent type=%d\n", evt->xcookie.evtype);
                break;
        }
        //XInput2 event
        // convert Generic event last, as it's common and on top of the structure
        my_XIEvent_32_t* s = evt->xcookie.data;
        my_XIEvent_t *d = evt->xcookie.data;
        d->time = from_ulong(s->time);
        d->evtype = s->evtype;
        d->extension = s->extension;
        d->display = getDisplay(from_ptrv(s->display));
        d->send_event = s->send_event;
        d->serial = from_ulong(s->serial);
        d->type = s->type;
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

void convert_XDeviceKeyEvent_to_32(my_XEvent_32_t* d, my_XEvent_t* s)
{
    my_XDeviceKeyEvent_t* src = (my_XDeviceKeyEvent_t*)s;
    my_XDeviceKeyEvent_32_t* dst = (my_XDeviceKeyEvent_32_t*)d;

    //dst->type = src->type;
    //dst->serial = src->serial;
    //dst->send_event = src->send_event;
    //dst->display = src->display;
    //dst->window = src->window;
    dst->deviceid = to_ulong(src->deviceid);
    dst->root = to_ulong(src->root);
    dst->subwindow = to_ulong(src->subwindow);
    dst->time = to_ulong(src->time);
    dst->x = src->x;
    dst->y = src->y;
    dst->x_root = src->x_root;
    dst->y_root = src->y_root;
    dst->state = src->state;
    dst->keycode = src->keycode;
    dst->same_screen = src->same_screen;
    dst->device_state = src->device_state;
    dst->axes_count = src->axes_count;
    dst->first_axis = src->first_axis;
    dst->axis_data[0] = src->axis_data[0];
    dst->axis_data[1] = src->axis_data[1];
    dst->axis_data[2] = src->axis_data[2];
    dst->axis_data[3] = src->axis_data[3];
    dst->axis_data[4] = src->axis_data[4];
    dst->axis_data[5] = src->axis_data[5];
}
void convert_XDeviceKeyEvent_to_64(my_XEvent_t* d, my_XEvent_32_t* s)
{
    my_XDeviceKeyEvent_32_t* src = (my_XDeviceKeyEvent_32_t*)s;
    my_XDeviceKeyEvent_t* dst = (my_XDeviceKeyEvent_t*)d;

    dst->axis_data[5] = src->axis_data[5];
    dst->axis_data[4] = src->axis_data[4];
    dst->axis_data[3] = src->axis_data[3];
    dst->axis_data[2] = src->axis_data[2];
    dst->axis_data[1] = src->axis_data[1];
    dst->axis_data[0] = src->axis_data[0];
    dst->first_axis = src->first_axis;
    dst->axes_count = src->axes_count;
    dst->device_state = src->device_state;
    dst->same_screen = src->same_screen;
    dst->keycode = src->keycode;
    dst->state = src->state;
    dst->y_root = src->y_root;
    dst->x_root = src->x_root;
    dst->y = src->y;
    dst->x = src->x;
    dst->time = from_ulong(src->time);
    dst->subwindow = from_ulong(src->subwindow);
    dst->root = from_ulong(src->root);
    dst->deviceid = from_ulong(src->deviceid);
    //dst->window = src->window;
    //dst->display = src->display;
    //dst->send_event = src->send_event;
    //dst->serial = src->serial;
    //dst->type = src->type;
}
void convert_XDeviceMotionEvent_to_32(my_XEvent_32_t* d, my_XEvent_t* s)
{
    my_XDeviceMotionEvent_t* src = (my_XDeviceMotionEvent_t*)s;
    my_XDeviceMotionEvent_32_t* dst = (my_XDeviceMotionEvent_32_t*)d;

    //dst->type = src->type;
    //dst->serial = src->serial;
    //dst->send_event = src->send_event;
    //dst->display = src->display;
    //dst->window = src->window;
    dst->deviceid = to_ulong(src->deviceid);
    dst->root = to_ulong(src->root);
    dst->subwindow = to_ulong(src->subwindow);
    dst->time = to_ulong(src->time);
    dst->x = src->x;
    dst->y = src->y;
    dst->x_root = src->x_root;
    dst->y_root = src->y_root;
    dst->state = src->state;
    dst->is_hint = src->is_hint;
    dst->same_screen = src->same_screen;
    dst->device_state = src->device_state;
    dst->axes_count = src->axes_count;
    dst->first_axis = src->first_axis;
    dst->axis_data[0] = src->axis_data[0];
    dst->axis_data[1] = src->axis_data[1];
    dst->axis_data[2] = src->axis_data[2];
    dst->axis_data[3] = src->axis_data[3];
    dst->axis_data[4] = src->axis_data[4];
    dst->axis_data[5] = src->axis_data[5];
}
void convert_XDeviceMotionEvent_to_64(my_XEvent_t* d, my_XEvent_32_t* s)
{
    my_XDeviceMotionEvent_32_t* src = (my_XDeviceMotionEvent_32_t*)s;
    my_XDeviceMotionEvent_t* dst = (my_XDeviceMotionEvent_t*)d;

    dst->axis_data[5] = src->axis_data[5];
    dst->axis_data[4] = src->axis_data[4];
    dst->axis_data[3] = src->axis_data[3];
    dst->axis_data[2] = src->axis_data[2];
    dst->axis_data[1] = src->axis_data[1];
    dst->axis_data[0] = src->axis_data[0];
    dst->first_axis = src->first_axis;
    dst->axes_count = src->axes_count;
    dst->device_state = src->device_state;
    dst->same_screen = src->same_screen;
    dst->is_hint = src->is_hint;
    dst->state = src->state;
    dst->y_root = src->y_root;
    dst->x_root = src->x_root;
    dst->y = src->y;
    dst->x = src->x;
    dst->time = from_ulong(src->time);
    dst->subwindow = from_ulong(src->subwindow);
    dst->root = from_ulong(src->root);
    dst->deviceid = from_ulong(src->deviceid);
    //dst->window = src->window;
    //dst->display = src->display;
    //dst->send_event = src->send_event;
    //dst->serial = src->serial;
    //dst->type = src->type;
}
void convert_XDeviceFocusEvent_to_32(my_XEvent_32_t* d, my_XEvent_t* s)
{
    my_XDeviceFocusChangeEvent_t* src = (my_XDeviceFocusChangeEvent_t*)s;
    my_XDeviceFocusChangeEvent_32_t* dst = (my_XDeviceFocusChangeEvent_32_t*)d;

    //dst->type = src->type;
    //dst->serial = src->serial;
    //dst->send_event = src->send_event;
    //dst->display = src->display;
    //dst->window = src->window;
    dst->deviceid = to_ulong(src->deviceid);
    dst->mode = src->mode;
    dst->detail = src->detail;
    dst->time = to_ulong(src->time);
}
void convert_XDeviceFocusEvent_to_64(my_XEvent_t* d, my_XEvent_32_t* s)
{
    my_XDeviceFocusChangeEvent_32_t* src = (my_XDeviceFocusChangeEvent_32_t*)s;
    my_XDeviceFocusChangeEvent_t* dst = (my_XDeviceFocusChangeEvent_t*)d;

    dst->time = from_ulong(src->time);
    dst->detail = src->detail;
    dst->mode = src->mode;
    dst->deviceid = from_ulong(src->deviceid);
    //dst->window = src->window;
    //dst->display = src->display;
    //dst->send_event = src->send_event;
    //dst->serial = src->serial;
    //dst->type = src->type;
}
void convert_XProximityNotifyEvent_to_32(my_XEvent_32_t* d, my_XEvent_t* s)
{
    my_XProximityNotifyEvent_t* src = (my_XProximityNotifyEvent_t*)s;
    my_XProximityNotifyEvent_32_t* dst = (my_XProximityNotifyEvent_32_t*)d;

    //dst->type = src->type;
    //dst->serial = src->serial;
    //dst->send_event = src->send_event;
    //dst->display = src->display;
    //dst->window = src->window;
    dst->deviceid = to_ulong(src->deviceid);
    dst->root = to_ulong(src->root);
    dst->subwindow = to_ulong(src->subwindow);
    dst->time = to_ulong(src->time);
    dst->x = src->x;
    dst->y = src->y;
    dst->x_root = src->x_root;
    dst->y_root = src->y_root;
    dst->state = src->state;
    dst->same_screen = src->same_screen;
    dst->device_state = src->device_state;
    dst->axes_count = src->axes_count;
    dst->first_axis = src->first_axis;
    dst->axis_data[0] = src->axis_data[0];
    dst->axis_data[1] = src->axis_data[1];
    dst->axis_data[2] = src->axis_data[2];
    dst->axis_data[3] = src->axis_data[3];
    dst->axis_data[4] = src->axis_data[4];
    dst->axis_data[5] = src->axis_data[5];
}
void convert_XProximityNotifyEvent_to_64(my_XEvent_t* d, my_XEvent_32_t* s)
{
    my_XProximityNotifyEvent_32_t* src = (my_XProximityNotifyEvent_32_t*)s;
    my_XProximityNotifyEvent_t* dst = (my_XProximityNotifyEvent_t*)d;

    dst->axis_data[5] = src->axis_data[5];
    dst->axis_data[4] = src->axis_data[4];
    dst->axis_data[3] = src->axis_data[3];
    dst->axis_data[2] = src->axis_data[2];
    dst->axis_data[1] = src->axis_data[1];
    dst->axis_data[0] = src->axis_data[0];
    dst->first_axis = src->first_axis;
    dst->axes_count = src->axes_count;
    dst->device_state = src->device_state;
    dst->same_screen = src->same_screen;
    dst->state = src->state;
    dst->y_root = src->y_root;
    dst->x_root = src->x_root;
    dst->y = src->y;
    dst->x = src->x;
    dst->time = from_ulong(src->time);
    dst->subwindow = from_ulong(src->subwindow);
    dst->root = from_ulong(src->root);
    dst->deviceid = from_ulong(src->deviceid);
    //dst->window = src->window;
    //dst->display = src->display;
    //dst->send_event = src->send_event;
    //dst->serial = src->serial;
    //dst->type = src->type;
}
void convert_XDeviceStateNotifyEvent_to_32(my_XEvent_32_t* d, my_XEvent_t* s)
{
    my_XDeviceStateNotifyEvent_t* src = (my_XDeviceStateNotifyEvent_t*)s;
    my_XDeviceStateNotifyEvent_32_t* dst = (my_XDeviceStateNotifyEvent_32_t*)d;

    //dst->type = src->type;
    //dst->serial = src->serial;
    //dst->send_event = src->send_event;
    //dst->display = src->display;
    //dst->window = src->window;
    dst->deviceid = to_ulong(src->deviceid);
    dst->time = to_ulong(src->time);
    memmove(dst->data, src->data, 64);
}
void convert_XDeviceStateNotifyEvent_to_64(my_XEvent_t* d, my_XEvent_32_t* s)
{
    my_XDeviceStateNotifyEvent_32_t* src = (my_XDeviceStateNotifyEvent_32_t*)s;
    my_XDeviceStateNotifyEvent_t* dst = (my_XDeviceStateNotifyEvent_t*)d;

    memmove(dst->data, src->data, 64);
    dst->time = from_ulong(src->time);
    dst->deviceid = from_ulong(src->deviceid);
    //dst->window = src->window;
    //dst->display = src->display;
    //dst->send_event = src->send_event;
    //dst->serial = src->serial;
    //dst->type = src->type;
}
void convert_XDeviceMappingEvent_to_32(my_XEvent_32_t* d, my_XEvent_t* s)
{
    my_XDeviceMappingEvent_t* src = (my_XDeviceMappingEvent_t*)s;
    my_XDeviceMappingEvent_32_t* dst = (my_XDeviceMappingEvent_32_t*)d;

    //dst->type = src->type;
    //dst->serial = src->serial;
    //dst->send_event = src->send_event;
    //dst->display = src->display;
    //dst->window = src->window;
    dst->deviceid = to_ulong(src->deviceid);
    dst->time = to_ulong(src->time);
    dst->request = src->request;
    dst->first_keycode = src->first_keycode;
    dst->count = src->count;
}
void convert_XDeviceMappingEvent_to_64(my_XEvent_t* d, my_XEvent_32_t* s)
{
    my_XDeviceMappingEvent_32_t* src = (my_XDeviceMappingEvent_32_t*)s;
    my_XDeviceMappingEvent_t* dst = (my_XDeviceMappingEvent_t*)d;

    dst->count = src->count;
    dst->first_keycode = src->first_keycode;
    dst->request = src->request;
    dst->time = from_ulong(src->time);
    dst->deviceid = from_ulong(src->deviceid);
    //dst->window = src->window;
    //dst->display = src->display;
    //dst->send_event = src->send_event;
    //dst->serial = src->serial;
    //dst->type = src->type;
}
void convert_XChangeDeviceNotifyEvent_to_32(my_XEvent_32_t* d, my_XEvent_t* s)
{
    my_XChangeDeviceNotifyEvent_t* src = (my_XChangeDeviceNotifyEvent_t*)s;
    my_XChangeDeviceNotifyEvent_32_t* dst = (my_XChangeDeviceNotifyEvent_32_t*)d;

    //dst->type = src->type;
    //dst->serial = src->serial;
    //dst->send_event = src->send_event;
    //dst->display = src->display;
    //dst->window = src->window;
    dst->deviceid = to_ulong(src->deviceid);
    dst->time = to_ulong(src->time);
    dst->request = src->request;
}
void convert_XChangeDeviceNotifyEvent_to_64(my_XEvent_t* d, my_XEvent_32_t* s)
{
    my_XChangeDeviceNotifyEvent_32_t* src = (my_XChangeDeviceNotifyEvent_32_t*)s;
    my_XChangeDeviceNotifyEvent_t* dst = (my_XChangeDeviceNotifyEvent_t*)d;

    dst->request = src->request;
    dst->time = from_ulong(src->time);
    dst->deviceid = from_ulong(src->deviceid);
    //dst->window = src->window;
    //dst->display = src->display;
    //dst->send_event = src->send_event;
    //dst->serial = src->serial;
    //dst->type = src->type;
}
void convert_XDevicePresenceNotifyEvent_to_32(my_XEvent_32_t* d, my_XEvent_t* s)
{
    my_XDevicePresenceNotifyEvent_t* src = (my_XDevicePresenceNotifyEvent_t*)s;
    my_XDevicePresenceNotifyEvent_32_t* dst = (my_XDevicePresenceNotifyEvent_32_t*)d;

    //dst->type = src->type;
    //dst->serial = src->serial;
    //dst->send_event = src->send_event;
    //dst->display = src->display;
    //dst->window = src->window;
    dst->time = to_ulong(src->time);
    dst->devchange = src->devchange;
    dst->deviceid = to_ulong(src->deviceid);
    dst->control = to_ulong(src->control);
}
void convert_XDevicePresenceNotifyEvent_to_64(my_XEvent_t* d, my_XEvent_32_t* s)
{
    my_XDevicePresenceNotifyEvent_32_t* src = (my_XDevicePresenceNotifyEvent_32_t*)s;
    my_XDevicePresenceNotifyEvent_t* dst = (my_XDevicePresenceNotifyEvent_t*)d;

    dst->control = from_ulong(src->control);
    dst->deviceid = from_ulong(src->deviceid);
    dst->devchange = src->devchange;
    dst->time = from_ulong(src->time);
    //dst->window = src->window;
    //dst->display = src->display;
    //dst->send_event = src->send_event;
    //dst->serial = src->serial;
    //dst->type = src->type;
}
void convert_XDevicePropertyNotifyEvent_to_32(my_XEvent_32_t* d, my_XEvent_t* s)
{
    my_XDevicePropertyNotifyEvent_t* src = (my_XDevicePropertyNotifyEvent_t*)s;
    my_XDevicePropertyNotifyEvent_32_t* dst = (my_XDevicePropertyNotifyEvent_32_t*)d;

    //dst->type = src->type;
    //dst->serial = src->serial;
    //dst->send_event = src->send_event;
    //dst->display = src->display;
    //dst->window = src->window;
    dst->time = to_ulong(src->time);
    dst->deviceid = to_ulong(src->deviceid);
    dst->atom = to_ulong(src->atom);
    dst->state = src->state;
}
void convert_XDevicePropertyNotifyEvent_to_64(my_XEvent_t* d, my_XEvent_32_t* s)
{
    my_XDevicePropertyNotifyEvent_32_t* src = (my_XDevicePropertyNotifyEvent_32_t*)s;
    my_XDevicePropertyNotifyEvent_t* dst = (my_XDevicePropertyNotifyEvent_t*)d;

    dst->state = src->state;
    dst->atom = from_ulong(src->atom);
    dst->deviceid = from_ulong(src->deviceid);
    dst->time = from_ulong(src->time);
    //dst->window = src->window;
    //dst->display = src->display;
    //dst->send_event = src->send_event;
    //dst->serial = src->serial;
    //dst->type = src->type;
}
void register_XDevice_events(my_XDevice_t* a)
{
    if(!a) return;
    // search if device is already in list
    register_events_t* head = register_events_head;
    while(head) {
        if(head->id == a)
            return; // found, nothing to do....
        head = head->next;
    }
    // counts events
    int n=0;
    for(int i=0; i<a->num_classes; ++i)
        switch(a->classes[i].input_class) {
            case 0: n+=2; break; //KeyClass
            case 1: n+=2; break; //ButtonClass
            case 2: n+=1; break; //ValuatorClass
            case 5: n+=2; break; //FocusClass
            case 4: n+=2; break; //ProximityClass
            case 6: n+=4; break; //OtherClass
        }
    // check if there is something to notify
    if(!n) return;
    // create a new event list
    register_events_t* events = box_malloc(sizeof(register_events_t)+n*sizeof(reg_event_t));
    events->id = a;
    events->n = n;
    events->events = (reg_event_t*)(events+1);
    n = 0;
    for(int i=0; i<a->num_classes; ++i) {
        switch(a->classes[i].input_class) {
            case 0: //KeyClass
                events->events[n].event = a->classes[i].event_type_base+0;
                events->events[n].to32 = convert_XDeviceKeyEvent_to_32;
                events->events[n].to64 = convert_XDeviceKeyEvent_to_64;
                ++n;
                events->events[n].event = a->classes[i].event_type_base+1;
                events->events[n].to32 = convert_XDeviceKeyEvent_to_32;
                events->events[n].to64 = convert_XDeviceKeyEvent_to_64;
                ++n;
                break;  
            case 1: //ButtonClass
                events->events[n].event = a->classes[i].event_type_base+0;
                events->events[n].to32 = convert_XDeviceKeyEvent_to_32;
                events->events[n].to64 = convert_XDeviceKeyEvent_to_64;
                ++n;
                events->events[n].event = a->classes[i].event_type_base+1;
                events->events[n].to32 = convert_XDeviceKeyEvent_to_32;
                events->events[n].to64 = convert_XDeviceKeyEvent_to_64;
                ++n;
                break;
            case 2: //ValuatorClass
                events->events[n].event = a->classes[i].event_type_base+0;
                events->events[n].to32 = convert_XDeviceMotionEvent_to_32;
                events->events[n].to64 = convert_XDeviceMotionEvent_to_64;
                ++n;
                break;
            case 5: //FocusClass
                events->events[n].event = a->classes[i].event_type_base+0;
                events->events[n].to32 = convert_XDeviceFocusEvent_to_32;
                events->events[n].to64 = convert_XDeviceFocusEvent_to_64;
                ++n;
                events->events[n].event = a->classes[i].event_type_base+1;
                events->events[n].to32 = convert_XDeviceFocusEvent_to_32;
                events->events[n].to64 = convert_XDeviceFocusEvent_to_64;
                ++n;
                break;
            case 4: //ProximityClass
                events->events[n].event = a->classes[i].event_type_base+0;
                events->events[n].to32 = convert_XProximityNotifyEvent_to_32;
                events->events[n].to64 = convert_XProximityNotifyEvent_to_64;
                ++n;
                events->events[n].event = a->classes[i].event_type_base+1;
                events->events[n].to32 = convert_XProximityNotifyEvent_to_32;
                events->events[n].to64 = convert_XProximityNotifyEvent_to_64;
                ++n;
                break;
            case 6: //OtherClass
                events->events[n].event = a->classes[i].event_type_base+0;
                events->events[n].to32 = convert_XDeviceStateNotifyEvent_to_32;
                events->events[n].to64 = convert_XDeviceStateNotifyEvent_to_64;
                ++n;
                events->events[n].event = a->classes[i].event_type_base+1;
                events->events[n].to32 = convert_XDeviceMappingEvent_to_32;
                events->events[n].to64 = convert_XDeviceMappingEvent_to_64;
                ++n;
                events->events[n].event = a->classes[i].event_type_base+2;
                events->events[n].to32 = convert_XChangeDeviceNotifyEvent_to_32;
                events->events[n].to64 = convert_XChangeDeviceNotifyEvent_to_64;
                ++n;
                //+3 would be DeviceKeyStateNotify
                //+4 wpuld be DeviceButtonStateNotify
                //+5 would be DevicePresenceNotify
                events->events[n].event = a->classes[i].event_type_base+6;
                events->events[n].to32 = convert_XDevicePropertyNotifyEvent_to_32;
                events->events[n].to64 = convert_XDevicePropertyNotifyEvent_to_64;
                ++n;
                break;
        }
    }
    events->start_event = events->end_event = events->events[0].event;
    for(int i=1; i<n; ++i) {
        if(events->start_event>events->events[i].event) events->start_event = events->events[i].event;
        if(events->end_event<events->events[i].event) events->end_event = events->events[i].event;
    }
    events->next = register_events_head;
    register_events_head = events;
}
void unregister_XDevice_events(my_XDevice_t* a)
{
    if(!a) return;
    register_events_t* prev = NULL;
    register_events_t* head = register_events_head;
    while(head) {
        if(head->id == a) {
            if(!prev)
                register_events_head = head->next;
            else
                prev->next = head->next;
            box_free(head);
            return;
        }
        prev = head;
        head = head->next;
    }
}

void convert_XFixesSelectionNotifyEvent_to_32(my_XEvent_32_t* d, my_XEvent_t* s)
{
    my_XFixesSelectionNotifyEvent_t* src = (my_XFixesSelectionNotifyEvent_t*)s;
    my_XFixesSelectionNotifyEvent_32_t* dst = (my_XFixesSelectionNotifyEvent_32_t*)d;

    //dst->type = src->type;
    //dst->serial = src->serial;
    //dst->send_event = src->send_event;
    //dst->display = src->display;
    //dst->window = src->window;
    dst->subtype = src->subtype;
    dst->owner = to_ulong(src->owner);
    dst->selection = to_ulong(src->selection);
    dst->timestamp = to_ulong(src->timestamp);
    dst->selection_timestamp = to_ulong(src->selection_timestamp);
}
void convert_XFixesSelectionNotifyEvent_to_64(my_XEvent_t* d, my_XEvent_32_t* s)
{
    my_XFixesSelectionNotifyEvent_32_t* src = (my_XFixesSelectionNotifyEvent_32_t*)s;
    my_XFixesSelectionNotifyEvent_t* dst = (my_XFixesSelectionNotifyEvent_t*)d;

    dst->selection_timestamp = from_ulong(src->selection_timestamp);
    dst->timestamp = from_ulong(src->timestamp);
    dst->selection = from_ulong(src->selection);
    dst->owner = from_ulong(src->owner);
    dst->subtype = src->subtype;
    //dst->window = src->window;
    //dst->display = src->display;
    //dst->send_event = src->send_event;
    //dst->serial = src->serial;
    //dst->type = src->type;
}
void convert_XFixesCursorNotifyEvent_to_32(my_XEvent_32_t* d, my_XEvent_t* s)
{
    my_XFixesCursorNotifyEvent_t* src = (my_XFixesCursorNotifyEvent_t*)s;
    my_XFixesCursorNotifyEvent_32_t* dst = (my_XFixesCursorNotifyEvent_32_t*)d;

    //dst->type = src->type;
    //dst->serial = src->serial;
    //dst->send_event = src->send_event;
    //dst->display = src->display;
    //dst->window = src->window;
    dst->subtype = src->subtype;
    dst->cursor_serial = to_ulong(src->cursor_serial);
    dst->timestamp = to_ulong(src->timestamp);
    dst->cursor_name = to_ulong(src->cursor_name);
}
void convert_XFixesCursorNotifyEvent_to_64(my_XEvent_t* d, my_XEvent_32_t* s)
{
    my_XFixesCursorNotifyEvent_32_t* src = (my_XFixesCursorNotifyEvent_32_t*)s;
    my_XFixesCursorNotifyEvent_t* dst = (my_XFixesCursorNotifyEvent_t*)d;

    dst->cursor_serial = from_ulong(src->cursor_serial);
    dst->timestamp = from_ulong(src->timestamp);
    dst->cursor_name = from_ulong(src->cursor_name);
    dst->subtype = src->subtype;
    //dst->window = src->window;
    //dst->display = src->display;
    //dst->send_event = src->send_event;
    //dst->serial = src->serial;
    //dst->type = src->type;
}

#define XFIXES (void*)1LL

void register_XFixes_events(int event_base)
{
    // search if device is already in list
    register_events_t* head = register_events_head;
    while(head) {
        if(head->id == XFIXES)
            return; // found, nothing to do....
        head = head->next;
    }
    int n = 2;  // 2 events to register!
    // create a new event list
    register_events_t* events = box_malloc(sizeof(register_events_t)+n*sizeof(reg_event_t));
    events->id = XFIXES;
    events->n = n;
    events->events = (reg_event_t*)(events+1);

    events->events[0].event = event_base+0;
    events->events[0].to32 = convert_XFixesSelectionNotifyEvent_to_32;
    events->events[0].to64 = convert_XFixesSelectionNotifyEvent_to_64;
    events->events[1].event = event_base+1;
    events->events[1].to32 = convert_XFixesCursorNotifyEvent_to_32;
    events->events[1].to64 = convert_XFixesCursorNotifyEvent_to_64;

    events->start_event = events->events[0].event;
    events->end_event = events->events[1].event;
    events->next = register_events_head;
    register_events_head = events;
}
void unregister_XFixes_events()
{
    register_events_t* prev = NULL;
    register_events_t* head = register_events_head;
    while(head) {
        if(head->id == XFIXES) {
            if(!prev)
                register_events_head = head->next;
            else
                prev->next = head->next;
            box_free(head);
            return;
        }
        prev = head;
        head = head->next;
    }
}

void convert_XRRScreenChangeNotifyEvent_to_32(my_XEvent_32_t* d, my_XEvent_t* s)
{
    my_XRRScreenChangeNotifyEvent_t* src = (my_XRRScreenChangeNotifyEvent_t*)s;
    my_XRRScreenChangeNotifyEvent_32_t* dst = (my_XRRScreenChangeNotifyEvent_32_t*)d;

    //dst->type = src->type;
    //dst->serial = src->serial;
    //dst->send_event = src->send_event;
    //dst->display = src->display;
    //dst->window = src->window;
    dst->root = to_ulong(src->root);
    dst->timestamp = to_ulong(src->timestamp);
    dst->config_timestamp = to_ulong(src->config_timestamp);
    dst->size_index = src->size_index;
    dst->subpixel_order = src->subpixel_order;
    dst->rotation = src->rotation;
    dst->width = src->width;
    dst->height = src->height;
    dst->mwidth = src->mwidth;
    dst->mheight = src->mheight;
}
void convert_XRRScreenChangeNotifyEvent_to_64(my_XEvent_t* d, my_XEvent_32_t* s)
{
    my_XRRScreenChangeNotifyEvent_32_t* src = (my_XRRScreenChangeNotifyEvent_32_t*)s;
    my_XRRScreenChangeNotifyEvent_t* dst = (my_XRRScreenChangeNotifyEvent_t*)d;

    dst->mheight = src->mheight;
    dst->mwidth = src->mwidth;
    dst->height = src->height;
    dst->width = src->width;
    dst->rotation = src->rotation;
    dst->subpixel_order = src->subpixel_order;
    dst->size_index = src->size_index;
    dst->config_timestamp = from_ulong(src->config_timestamp);
    dst->timestamp = from_ulong(src->timestamp);
    dst->root = from_ulong(src->root);
    //dst->window = src->window;
    //dst->display = src->display;
    //dst->send_event = src->send_event;
    //dst->serial = src->serial;
    //dst->type = src->type;
}
void convert_XRRNotifyEvent_to_32(my_XEvent_32_t* d, my_XEvent_t* s)
{
    my_XRRNotifyEvent_t* src = (my_XRRNotifyEvent_t*)s;
    my_XRRNotifyEvent_32_t* dst = (my_XRRNotifyEvent_32_t*)d;

    //dst->type = src->type;
    //dst->serial = src->serial;
    //dst->send_event = src->send_event;
    //dst->display = src->display;
    //dst->window = src->window;
    int subtype = src->subtype;
    dst->subtype = src->subtype;
    switch (subtype) {
        case 0: {
            my_XRROutputChangeNotifyEvent_t* src = (my_XRROutputChangeNotifyEvent_t*)s;
            my_XRROutputChangeNotifyEvent_32_t* dst = (my_XRROutputChangeNotifyEvent_32_t*)d;
            dst->output = to_long(src->output);
            dst->crtc = to_long(src->crtc);
            dst->mode = to_long(src->mode);
            dst->rotation = src->rotation;
            dst->connection = src->connection;
            dst->subpixel_order = src->subpixel_order;
        } break;
        case 1: {
            my_XRRCrtcChangeNotifyEvent_t* src = (my_XRRCrtcChangeNotifyEvent_t*)s;
            my_XRRCrtcChangeNotifyEvent_32_t* dst = (my_XRRCrtcChangeNotifyEvent_32_t*)d;
            dst->crtc = to_ulong(src->crtc);
            dst->mode = to_ulong(src->mode);
            dst->rotation = src->rotation;
            dst->x = src->x;
            dst->y = src->y;
            dst->width = src->width;
            dst->height = src->height;
        } break;
        case 2: {
            my_XRROutputPropertyNotifyEvent_t* src = (my_XRROutputPropertyNotifyEvent_t*)s;
            my_XRROutputPropertyNotifyEvent_32_t* dst = (my_XRROutputPropertyNotifyEvent_32_t*)d;
            dst->output = to_ulong(src->output);
            dst->property = to_ulong(src->property);
            dst->timestamp = to_ulong(src->timestamp);
            dst->state = src->state;
        } break;
        case 3: {
            my_XRRProviderChangeNotifyEvent_t* src = (my_XRRProviderChangeNotifyEvent_t*)s;
            my_XRRProviderChangeNotifyEvent_32_t* dst = (my_XRRProviderChangeNotifyEvent_32_t*)d;
            dst->provider = to_ulong(src->provider);
            dst->timestamp = to_ulong(src->timestamp);
            dst->current_role = src->current_role;
        } break;
        case 4: {
            my_XRRProviderPropertyNotifyEvent_t* src = (my_XRRProviderPropertyNotifyEvent_t*)s;
            my_XRRProviderPropertyNotifyEvent_32_t* dst = (my_XRRProviderPropertyNotifyEvent_32_t*)d;
            dst->provider = to_ulong(src->provider);
            dst->property = to_ulong(src->property);
            dst->timestamp = to_ulong(src->timestamp);
            dst->state = src->state;
        } break;
        case 5: {
            my_XRRResourceChangeNotifyEvent_t* src = (my_XRRResourceChangeNotifyEvent_t*)s;
            my_XRRResourceChangeNotifyEvent_32_t* dst = (my_XRRResourceChangeNotifyEvent_32_t*)d;
            dst->timestamp = to_ulong(src->timestamp);
        } break;
    }
}
void convert_XRRNotifyEvent_to_64(my_XEvent_t* d, my_XEvent_32_t* s)
{
    my_XRRNotifyEvent_32_t* src = (my_XRRNotifyEvent_32_t*)s;
    my_XRRNotifyEvent_t* dst = (my_XRRNotifyEvent_t*)d;
    int subtype = src->subtype;
    switch (subtype) {
        case 0: {
            my_XRROutputChangeNotifyEvent_32_t* src = (my_XRROutputChangeNotifyEvent_32_t*)s;
            my_XRROutputChangeNotifyEvent_t* dst = (my_XRROutputChangeNotifyEvent_t*)d;
            dst->output = from_long(src->output);
            dst->crtc = from_long(src->crtc);
            dst->mode = from_long(src->mode);
            dst->rotation = src->rotation;
            dst->connection = src->connection;
            dst->subpixel_order = src->subpixel_order;
        } break;
        case 1: {
            my_XRRCrtcChangeNotifyEvent_32_t* src = (my_XRRCrtcChangeNotifyEvent_32_t*)s;
            my_XRRCrtcChangeNotifyEvent_t* dst = (my_XRRCrtcChangeNotifyEvent_t*)d;
            dst->crtc = from_ulong(src->crtc);
            dst->mode = from_ulong(src->mode);
            dst->rotation = src->rotation;
            dst->x = src->x;
            dst->y = src->y;
            dst->width = src->width;
            dst->height = src->height;
        } break;
        case 2: {
            my_XRROutputPropertyNotifyEvent_32_t* src = (my_XRROutputPropertyNotifyEvent_32_t*)s;
            my_XRROutputPropertyNotifyEvent_t* dst = (my_XRROutputPropertyNotifyEvent_t*)d;
            dst->output = from_ulong(src->output);
            dst->property = from_ulong(src->property);
            dst->timestamp = from_ulong(src->timestamp);
            dst->state = src->state;
        } break;
        case 3: {
            my_XRRProviderChangeNotifyEvent_32_t* src = (my_XRRProviderChangeNotifyEvent_32_t*)s;
            my_XRRProviderChangeNotifyEvent_t* dst = (my_XRRProviderChangeNotifyEvent_t*)d;
            dst->provider = from_ulong(src->provider);
            dst->timestamp = from_ulong(src->timestamp);
            dst->current_role = src->current_role;
        } break;
        case 4: {
            my_XRRProviderPropertyNotifyEvent_32_t* src = (my_XRRProviderPropertyNotifyEvent_32_t*)s;
            my_XRRProviderPropertyNotifyEvent_t* dst = (my_XRRProviderPropertyNotifyEvent_t*)d;
            dst->provider = from_ulong(src->provider);
            dst->property = from_ulong(src->property);
            dst->timestamp = from_ulong(src->timestamp);
            dst->state = src->state;
        } break;
        case 5: {
            my_XRRResourceChangeNotifyEvent_32_t* src = (my_XRRResourceChangeNotifyEvent_32_t*)s;
            my_XRRResourceChangeNotifyEvent_t* dst = (my_XRRResourceChangeNotifyEvent_t*)d;
            dst->timestamp = from_ulong(src->timestamp);
        } break;
    }
    dst->subtype = src->subtype;
    //dst->window = src->window;
    //dst->display = src->display;
    //dst->send_event = src->send_event;
    //dst->serial = src->serial;
    //dst->type = src->type;
}

void register_XRandR_events(int event_base)
{
    void* a = (void*)2LL;
    // search if device is already in list
    register_events_t* head = register_events_head;
    while(head) {
        if(head->id == a)
            return; // found, nothing to do....
        head = head->next;
    }
    int n = 2;  // 2 events to register!, but second one as 6+1 subtypes
    // create a new event list
    register_events_t* events = box_malloc(sizeof(register_events_t)+n*sizeof(reg_event_t));
    events->id = a;
    events->n = n;
    events->events = (reg_event_t*)(events+1);

    events->events[0].event = event_base+0;
    events->events[0].to32 = convert_XRRScreenChangeNotifyEvent_to_32;
    events->events[0].to64 = convert_XRRScreenChangeNotifyEvent_to_64;
    events->events[1].event = event_base+1;
    events->events[1].to32 = convert_XRRNotifyEvent_to_32;
    events->events[1].to64 = convert_XRRNotifyEvent_to_64;

    events->start_event = events->events[0].event;
    events->end_event = events->events[1].event;
    events->next = register_events_head;
    register_events_head = events;
}

void unregister_XRandR_events()
{
    void* a = (void*)2LL;
    register_events_t* prev = NULL;
    register_events_t* head = register_events_head;
    while(head) {
        if(head->id == a) {
            if(!prev)
                register_events_head = head->next;
            else
                prev->next = head->next;
            box_free(head);
            return;
        }
        prev = head;
        head = head->next;
    }
}

void convert_XkbEvent_to_32(my_XEvent_32_t* d, my_XEvent_t* s)
{
    my_XkbEvent_t* src = (my_XkbEvent_t*)s;
    my_XkbEvent_32_t* dst = (my_XkbEvent_32_t*)d;
    // convert XkbAnyEvent first, as it's the common part
    int subtype = src->any.xkb_type;
    //dst->any.type = src->any.type;
    //dst->any.serial = src->any.serial;
    //dst->any.send_event = src->any.send_event;
    //dst->any.display = src->any.display;
    dst->any.time = to_ulong(src->any.time);
    dst->any.xkb_type = src->any.xkb_type;
    dst->any.device = src->any.device;
    // only XkbBellNotifyEvent need special conversion
    switch(subtype) {
        case 8:
            dst->bell.percent = src->bell.percent;
            dst->bell.pitch = src->bell.pitch;
            dst->bell.duration = src->bell.duration;
            dst->bell.bell_class = src->bell.bell_class;
            dst->bell.bell_id = src->bell.bell_id;
            dst->bell.name = to_ulong(src->bell.name);
            dst->bell.window = to_ulong(src->bell.window);
            dst->bell.event_only = src->bell.event_only;
            break;
        default:
            memcpy(&dst->any.device, &src->any.device, sizeof(my_XkbEvent_32_t)-offsetof(my_XkbEvent_32_t, any.device));
            break;
    }
}

void convert_XkbEvent_to_64(my_XEvent_t* d, my_XEvent_32_t* s)
{
    my_XkbEvent_32_t* src = (my_XkbEvent_32_t*)s;
    my_XkbEvent_t* dst = (my_XkbEvent_t*)d;
    // convert XkbAnyEvent first, as it's the common part
    int subtype = src->any.xkb_type;
    //dst->any.type = src->any.type;
    //dst->any.serial = src->any.serial;
    //dst->any.send_event = src->any.send_event;
    //dst->any.display = src->any.display;
    dst->any.time = from_ulong(src->any.time);
    dst->any.xkb_type = src->any.xkb_type;
    dst->any.device = src->any.device;
    // only XkbBellNotifyEvent need special conversion
    switch(subtype) {
        case 8:
            dst->bell.percent = src->bell.percent;
            dst->bell.pitch = src->bell.pitch;
            dst->bell.duration = src->bell.duration;
            dst->bell.bell_class = src->bell.bell_class;
            dst->bell.bell_id = src->bell.bell_id;
            dst->bell.name = from_ulong(src->bell.name);
            dst->bell.window = from_ulong(src->bell.window);
            dst->bell.event_only = src->bell.event_only;
            break;
        default:
            memcpy(&dst->any.device, &src->any.device, sizeof(my_XkbEvent_t)-offsetof(my_XkbEvent_t, any.device));
            break;
    }
}

#define XKB     (void*)3LL

void register_Xkb_events(int event_base)
{
    // search if device is already in list
    register_events_t* head = register_events_head;
    while(head) {
        if(head->id == XKB)
            return; // found, nothing to do....
        head = head->next;
    }
    int n = 1;  // 1 event to register!, but there are 12 subevent actualy
    // create a new event list
    register_events_t* events = box_malloc(sizeof(register_events_t)+n*sizeof(reg_event_t));
    events->id = XKB;
    events->n = n;
    events->events = (reg_event_t*)(events+1);

    events->events[0].event = event_base+0;
    events->events[0].to32 = convert_XkbEvent_to_32;
    events->events[0].to64 = convert_XkbEvent_to_64;

    events->start_event = events->events[0].event;
    events->end_event = events->events[0].event;
    events->next = register_events_head;
    register_events_head = events;
}

void unregister_Xkb_events()
{
    register_events_t* prev = NULL;
    register_events_t* head = register_events_head;
    while(head) {
        if(head->id == XKB) {
            if(!prev)
                register_events_head = head->next;
            else
                prev->next = head->next;
            box_free(head);
            return;
        }
        prev = head;
        head = head->next;
    }
}
