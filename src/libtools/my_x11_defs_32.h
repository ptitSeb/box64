#ifndef MY_X11_DEFS_32
#define MY_X11_DEFS_32
#include <stdint.h>
#include "box32.h"

typedef ulong_t XID_32;

typedef struct ximage_32_s {
    ptr_t create_image;
    ptr_t destroy_image;
    ptr_t get_pixel;
    ptr_t put_pixel;
    ptr_t sub_image;
    ptr_t add_pixel;
} ximage_32_t;

typedef struct _XImage_32 {
    int32_t width, height;
    int32_t xoffset;
    int32_t format;
    ptr_t   data;
    int     byte_order;
    int     bitmap_unit;
    int     bitmap_bit_order;
    int     bitmap_pad;
    int     depth;
    int     bytes_per_line;
    int     bits_per_pixel;
    ulong_t red_mask;
    ulong_t green_mask;
    ulong_t blue_mask;
    ptr_t   obdata;
    ximage_32_t f;
} XImage_32;

struct my_XFreeFuncs_32 {
    ptr_t atoms;    // void*
    ptr_t modifiermap;  // void*
    ptr_t key_bindings; // void*
    ptr_t context_db;   // void*
    ptr_t defaultCCCs;  // void*
    ptr_t clientCmaps;  // void*
    ptr_t intensityMaps;    // void*
    ptr_t im_filters;   // void*
    ptr_t xkb;  // void*
};

struct my_XExten_32 {
    ptr_t next; //struct my_XExten *
    ptr_t codes;    // XExtCodes
    ptr_t create_GC;    // CreateGCType
    ptr_t copy_GC;  // CopyGCType
    ptr_t flush_GC; // FlushGCType
    ptr_t free_GC;  // FreeGCType
    ptr_t create_Font;  // CreateFontType
    ptr_t free_Font;    // FreeFontType
    ptr_t close_display;    // CloseDisplayType
    ptr_t error;    // ErrorType
    ptr_t error_string; // ErrorStringType
    ptr_t name; //char*
    ptr_t error_values; // PrintErrorType
    ptr_t before_flush; // BeforeFlushType
    ptr_t next_flush;   //struct my_XExten *
};

struct my_XInternalAsync_32 {
    ptr_t next; //struct my_XInternalAsync_32 *
    ptr_t handler; //int (*handler)(void*, void*, char*, int, void*);
    ptr_t data; //void*
};

struct my_XLockPtrs_32 {
    ptr_t lock_display;// void (*lock_display)(void* dpy);
    ptr_t unlock_display;// void (*unlock_display)(void *dpy);
};

struct my_XConnectionInfo_32 {
    int fd;
    ptr_t read_callback;    // _XInternalConnectionProc
    ptr_t call_data;
    ptr_t watch_data;   // void**
    struct my_XConnectionInfo *next;
};

struct my_XConnWatchInfo_32 {
    ptr_t fn;   // XConnectionWatchProc
    ptr_t client_data;
    ptr_t next; //struct _XConnWatchInfo *
};

typedef struct my_Visual_32_s {
    ptr_t   ext_data;   //XExtData*
    XID_32 visualid;
    int c_class;
    ulong_t red_mask, green_mask, blue_mask;
    int bits_per_rgb;
    int map_entries;
} my_Visual_32_t;

typedef struct my_Screen_32_s {
    ptr_t ext_data;         //XExtData *
    ptr_t display;      //struct my_XDisplay_s *
    XID_32 root;
    int width, height;
    int mwidth, mheight;
    int ndepths;
    ptr_t depths;   //Depth *
    int root_depth;         /* bits per pixel */
    ptr_t root_visual;  //Visual *
    ptr_t default_gc;   //GC == struct _XGC*
    XID_32 cmap;
    ulong_t white_pixel;
    ulong_t black_pixel;
    int max_maps, min_maps;
    int backing_store;
    int save_unders;
    long_t root_input_mask;
} my_Screen_32_t;

typedef struct my_XDisplay_32_s
{
    ptr_t ext_data; //void *                            //offset = 0x00
    ptr_t free_funcs;   //struct my_XFreeFuncs_32 *
    int fd;
    int conn_checker;
    int proto_major_version;                            //offset = 0x10
    int proto_minor_version;
    ptr_t vendor;   //char *
    XID_32 resource_base;
    XID_32 resource_mask;                               // offset = 0x20
    XID_32 resource_id;
    int resource_shift;
    ptr_t resource_alloc;//XID_32 (*resource_alloc)(void*);
    int byte_order;                                     // offset = 0x30
    int bitmap_unit;
    int bitmap_pad;
    int bitmap_bit_order;
    int nformats;                                       //offset = 0x40
    ptr_t pixmap_format;    //void *
    int vnumber;
    int release;
    ptr_t head, tail;                                   //ofsset = 0x50
    int qlen;
    ulong_t last_request_read;
    ulong_t request;                                    //offset = 0x60
    ptr_t last_req; //char *
    ptr_t buffer;   //char *
    ptr_t bufptr;   //char *
    ptr_t bufmax;   //char *                            //offset = 0x70
    unsigned max_request_size;
    ptr_t db;   //void* *
    ptr_t synchandler; //int (*synchandler)(void*);
    ptr_t display_name;//char *                         //offset = 0x80
    int default_screen;
    int nscreens;
    ptr_t screens;//void *
    ulong_t motion_buffer;                              //offset = 0x90
    volatile ulong_t flags;
    int min_keycode;
    int max_keycode;
    ptr_t keysyms;  //void *                            //offset = 0xA0
    ptr_t modifiermap;  //void *
    int keysyms_per_keycode;
    ptr_t xdefaults;    //char *
    ptr_t scratch_buffer;   //char *                    //offset = 0xB0
    ulong_t scratch_length;
    int ext_number;
    ptr_t ext_procs;    //struct my_XExten *
    ptr_t event_vec[128];    //int (*event_vec[128])(void *, void *, void *);   // start at 0xC0
    ptr_t wire_vec[128]; //int (*wire_vec[128])(void *, void *, void *);
    XID_32 lock_meaning;                                //offset = 0x4C0
    ptr_t lock; //void*
    ptr_t async_handlers;   //struct my_XInternalAsync *
    ulong_t bigreq_size;
    ptr_t lock_fns; //struct my_XLockPtrs *             //offset = 0x4D0
    ptr_t idlist_alloc; //void (*idlist_alloc)(void *, void *, int);
    ptr_t key_bindings; //void* 
    XID_32 cursor_font;
    ptr_t atoms;    //void* *
    unsigned int mode_switch;
    unsigned int num_lock;
    ptr_t context_db;   //void*
    ptr_t error_vec;    //int (**error_vec)(void*, void*, void*);
    struct {
        ptr_t defaultCCCs;   //void*
        ptr_t clientCmaps;   //void*
        ptr_t perVisualIntensityMaps;    //void*
    } cms;
    ptr_t im_filters;   //void*
    ptr_t qfree;    //void*
    ulong_t next_event_32_serial_num;
    ptr_t flushes;  //struct my_XExten *
    ptr_t im_fd_info;   //struct my_XConnectionInfo *
    int im_fd_length;
    ptr_t conn_watchers;    //struct my_XConnWatchInfo *
    int watcher_count;
    ptr_t filedes;  //void*
    ptr_t savedsynchandler;  //int (*savedsynchandler)(void *);
    XID_32 resource_max;
    int xcmisc_opcode;
    ptr_t xkb_info; //void* *
    ptr_t trans_conn;   //void* *
    ptr_t xcb;  //void* *
    unsigned int next_cookie;
    ptr_t generic_event_vec[128]; //int (*generic_event_vec[128])(void*, void*, void*);
    ptr_t generic_event_copy_vec[128]; //int (*generic_event_copy_vec[128])(void*, void*, void*);
    ptr_t cookiejar;    //void *
    ptr_t error_threads;    //void *
    ptr_t exit_handler; //void *
    ptr_t exit_handler_data;    //void *
} my_XDisplay_32_t;

typedef struct my_XSetWindowAttributes_32_s {
    XID_32 background_pixmap;
    ulong_t background_pixel;
    XID_32 border_pixmap;
    ulong_t border_pixel;
    int bit_gravity;
    int win_gravity;
    int backing_store;
    ulong_t backing_planes;
    ulong_t backing_pixel;
    int save_under;
    long_t event_mask;
    long_t do_not_propagate_mask;
    int override_redirect;
    XID_32 colormap;
    XID_32 cursor;
} my_XSetWindowAttributes_32_t;

// Events

typedef struct my_XKeyEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 window;
    XID_32 root;
    XID_32 subwindow;
    ulong_t       time;
    int x, y;
    int x_root, y_root;
    unsigned int state;
    unsigned int keycode;
    int same_screen;
} my_XKeyEvent_32_t;
typedef my_XKeyEvent_32_t my_XKeyPressedEvent_32_t;
typedef my_XKeyEvent_32_t my_XKeyReleasedEvent_32_t;

typedef struct my_XButtonEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 window;
    XID_32 root;
    XID_32 subwindow;
    ulong_t       time;
    int x, y;
    int x_root, y_root;
    unsigned int state;
    unsigned int button;
    int same_screen;
} my_XButtonEvent_32_t;
typedef my_XButtonEvent_32_t my_XButtonPressedEvent_32_t;
typedef my_XButtonEvent_32_t my_XButtonReleasedEvent_32_t;

typedef struct my_XMotionEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 window;
    XID_32 root;
    XID_32 subwindow;
    ulong_t       time;
    int x, y;
    int x_root, y_root;
    unsigned int state;
    char is_hint;
    int same_screen;
} my_XMotionEvent_32_t;
typedef my_XMotionEvent_32_t my_XPointerMovedEvent_32_t;

typedef struct my_XCrossingEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 window;
    XID_32 root;
    XID_32 subwindow;
    ulong_t       time;
    int x, y;
    int x_root, y_root;
    int mode;
    int detail;
    int same_screen;
    int focus;
    unsigned int state;
} my_XCrossingEvent_32_t;

typedef my_XCrossingEvent_32_t my_XEnterWindowEvent_32_t;
typedef my_XCrossingEvent_32_t my_XLeaveWindowEvent_32_t;

typedef struct my_XFocusChangeEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 window;
    int mode;
    int detail;
} my_XFocusChangeEvent_32_t;
typedef my_XFocusChangeEvent_32_t my_XFocusInEvent_32_t;
typedef my_XFocusChangeEvent_32_t my_XFocusOutEvent_32_t;

typedef struct my_XKeymapEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 window;
    char key_vector[32];
} my_XKeymapEvent_32_t;

typedef struct my_XExposeEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 window;
    int x, y;
    int width, height;
    int count;
} my_XExposeEvent_32_t;

typedef struct my_XGraphicsExposeEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 drawable;
    int x, y;
    int width, height;
    int count;
    int major_code;
    int minor_code;
} my_XGraphicsExposeEvent_32_t;

typedef struct my_XNoExposeEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 drawable;
    int major_code;
    int minor_code;
} my_XNoExposeEvent_32_t;

typedef struct my_XVisibilityEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 window;
    int state;
} my_XVisibilityEvent_32_t;

typedef struct my_XCreateWindowEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 parent;
    XID_32 window;
    int x, y;
    int width, height;
    int border_width;
    int override_redirect;
} my_XCreateWindowEvent_32_t;

typedef struct my_XDestroyWindowEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 event;
    XID_32 window;
} my_XDestroyWindowEvent_32_t;

typedef struct my_XUnmapEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 event;
    XID_32 window;
    int from_configure;
} my_XUnmapEvent_32_t;

typedef struct my_XMapEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 event;
    XID_32 window;
    int override_redirect;
} my_XMapEvent_32_t;

typedef struct my_XMapRequestEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 parent;
    XID_32 window;
} my_XMapRequestEvent_32_t;

typedef struct my_XReparentEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 event;
    XID_32 window;
    XID_32 parent;
    int x, y;
    int override_redirect;
} my_XReparentEvent_32_t;

typedef struct my_XConfigureEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 event;
    XID_32 window;
    int x, y;
    int width, height;
    int border_width;
    XID_32 above;
    int override_redirect;
} my_XConfigureEvent_32_t;

typedef struct my_XGravityEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 event;
    XID_32 window;
    int x, y;
} my_XGravityEvent_32_t;

typedef struct my_XResizeRequestEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 window;
    int width, height;
} my_XResizeRequestEvent_32_t;

typedef struct my_XConfigureRequestEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 parent;
    XID_32 window;
    int x, y;
    int width, height;
    int border_width;
    XID_32 above;
    int detail;
    ulong_t       value_mask;
} my_XConfigureRequestEvent_32_t;

typedef struct my_XCirculateEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 event;
    XID_32 window;
    int place;
} my_XCirculateEvent_32_t;

typedef struct my_XCirculateRequestEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 parent;
    XID_32 window;
    int place;
} my_XCirculateRequestEvent_32_t;

typedef struct my_XPropertyEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 window;
    XID_32 atom;
    ulong_t       time;
    int state;
} my_XPropertyEvent_32_t;

typedef struct my_XSelectionClearEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 window;
    XID_32 selection;
    ulong_t       time;
} my_XSelectionClearEvent_32_t;

typedef struct my_XSelectionRequestEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 owner;
    XID_32 requestor;
    XID_32 selection;
    XID_32 target;
    XID_32 property;
    ulong_t       time;
} my_XSelectionRequestEvent_32_t;

typedef struct my_XSelectionEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 requestor;
    XID_32 selection;
    XID_32 target;
    XID_32 property;
    ulong_t       time;
} my_XSelectionEvent_32_t;

typedef struct my_XColormapEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 window;
    XID_32 colormap;
    int c_new;
    int state;
} my_XColormapEvent_32_t;

typedef struct my_XClientMessageEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 window;
    XID_32 message_type;
    int format;
    union {
            char b[20];
            short s[10];
            long_t l[5];
            } data;
} my_XClientMessageEvent_32_t;

typedef struct my_XMappingEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 window;
    int request;
    int first_keycode;
    int count;
} my_XMappingEvent_32_t;

typedef struct my_XErrorEvent_32_s
{
    int type;
    ptr_t display;  //Display*
    XID_32 resourceid;
    ulong_t       serial;
    unsigned char error_code;
    unsigned char request_code;
    unsigned char minor_code;
} my_XErrorEvent_32_t;

typedef struct my_XAnyEvent_32_s
{
    int type;
    ulong_t       serial;
    int send_event;
    ptr_t display;  //Display*
    XID_32 window;
} my_XAnyEvent_32_t;

typedef struct my_XGenericEvent_32_s
{
    int            type;
    ulong_t        serial;
    int            send_event;
    ptr_t          display; //Display*
    int            extension;
    int            evtype;
} my_XGenericEvent_32_t;

typedef struct my_XGenericEventCookie_32_s
{
    int            type;
    ulong_t        serial;
    int            send_event;
    ptr_t          display; //Display*
    int            extension;
    int            evtype;
    unsigned int   cookie;
    ptr_t          data;    //void*
} my_XGenericEventCookie_32_t;

typedef union my_XEvent_32_s {
    int type;
    my_XAnyEvent_32_t xany;
    my_XKeyEvent_32_t xkey;
    my_XButtonEvent_32_t xbutton;
    my_XMotionEvent_32_t xmotion;
    my_XCrossingEvent_32_t xcrossing;
    my_XFocusChangeEvent_32_t xfocus;
    my_XExposeEvent_32_t xexpose;
    my_XGraphicsExposeEvent_32_t xgraphicsexpose;
    my_XNoExposeEvent_32_t xnoexpose;
    my_XVisibilityEvent_32_t xvisibility;
    my_XCreateWindowEvent_32_t xcreatewindow;
    my_XDestroyWindowEvent_32_t xdestroywindow;
    my_XUnmapEvent_32_t xunmap;
    my_XMapEvent_32_t xmap;
    my_XMapRequestEvent_32_t xmaprequest;
    my_XReparentEvent_32_t xreparent;
    my_XConfigureEvent_32_t xconfigure;
    my_XGravityEvent_32_t xgravity;
    my_XResizeRequestEvent_32_t xresizerequest;
    my_XConfigureRequestEvent_32_t xconfigurerequest;
    my_XCirculateEvent_32_t xcirculate;
    my_XCirculateRequestEvent_32_t xcirculaterequest;
    my_XPropertyEvent_32_t xproperty;
    my_XSelectionClearEvent_32_t xselectionclear;
    my_XSelectionRequestEvent_32_t xselectionrequest;
    my_XSelectionEvent_32_t xselection;
    my_XColormapEvent_32_t xcolormap;
    my_XClientMessageEvent_32_t xclient;
    my_XMappingEvent_32_t xmapping;
    my_XErrorEvent_32_t xerror;
    my_XKeymapEvent_32_t xkeymap;
    my_XGenericEvent_32_t xgeneric;
    my_XGenericEventCookie_32_t xcookie;
    long_t pad[24];
} my_XEvent_32_t;

// WMHints 
typedef struct my_XWMHints_32_s {
    long_t flags;
    int input;
    int initial_state;
    XID_32 icon_pixmap;
    XID_32 icon_window;
    int icon_x;
    int icon_y;
    XID_32 icon_mask;
    XID_32 window_group;
} my_XWMHints_32_t;

typedef struct my_XRRModeInfo_32_s {
    XID_32              id;
    unsigned int        width;
    unsigned int        height;
    ulong_t             dotClock;
    unsigned int        hSyncStart;
    unsigned int        hSyncEnd;
    unsigned int        hTotal;
    unsigned int        hSkew;
    unsigned int        vSyncStart;
    unsigned int        vSyncEnd;
    unsigned int        vTotal;
    ptr_t               name;   //char*
    unsigned int        nameLength;
    ulong_t             modeFlags;
} my_XRRModeInfo_32_t;


typedef struct my_XRRScreenResources_32_s {
    ulong_t     timestamp;
    ulong_t     configTimestamp;
    int         ncrtc;
    ptr_t       crtcs;  //XID_32*
    int         noutput;
    ptr_t       outputs;    //XID_32*
    int         nmode;
    ptr_t       modes;  //my_XRRModeInfo_32_t *
} my_XRRScreenResources_32_t;

typedef struct my_XRRCrtcInfo_32_s {
    ulong_t         timestamp;
    int             x, y;
    unsigned int    width, height;
    XID_32          mode;
    uint16_t        rotation;
    int             noutput;
    ptr_t           outputs;    //XID_32*
    uint16_t        rotations;
    int             npossible;
    ptr_t           possible;   //XID_32*
} my_XRRCrtcInfo_32_t;

typedef struct my_XRROutputInfo_32_s {
    ulong_t         timestamp;
    XID_32          crtc;
    ptr_t           name;   //char*
    int             nameLen;
    ulong_t         mm_width;
    ulong_t         mm_height;
    uint16_t        connection;
    uint16_t        subpixel_order;
    int             ncrtc;
    ptr_t           crtcs; //XID_32*
    int             nclone;
    ptr_t           clones; //XID_32*
    int             nmode;
    int             npreferred;
    ptr_t           modes; //XID_32*
} my_XRROutputInfo_32_t;

typedef struct my_XWindowAttributes_32_s {
    int x, y;
    int width, height;
    int border_width;
    int depth;
    ptr_t visual;   //Visual*
    XID_32 root;
    int c_class;
    int bit_gravity;
    int win_gravity;
    int backing_store;
    ulong_t backing_planes;
    ulong_t backing_pixel;
    int save_under;
    XID_32 colormap;
    int map_installed;
    int map_state;
    long_t all_event_masks;
    long_t your_event_mask;
    long_t do_not_propagate_mask;
    int override_redirect;
    ptr_t screen;   //Screen*
} my_XWindowAttributes_32_t;

typedef struct my_XVisualInfo_32_s {
  ptr_t visual; //Visual*
  ulong_t visualid;
  int screen;
  int depth;
  int c_class;
  ulong_t red_mask;
  ulong_t green_mask;
  ulong_t blue_mask;
  int colormap_size;
  int bits_per_rgb;
} my_XVisualInfo_32_t;

typedef struct my_XModifierKeymap_32_s {
    int             max_keypermod;
    ptr_t           modifiermap;    //uint8_t*
} my_XModifierKeymap_32_t;

typedef struct my_XdbeVisualInfo_32_s
{
    XID_32      visual;
    int         depth;
    int         perflevel;
} my_XdbeVisualInfo_32_t;

typedef struct my_XdbeScreenVisualInfo_32_s
{
    int         count;
    ptr_t       visinfo;    //my_XdbeVisualInfo_t*
} my_XdbeScreenVisualInfo_32_t;

typedef struct my_XF86VidModeModeInfo_32_s
{
    unsigned int        dotclock;
    unsigned short      hdisplay;
    unsigned short      hsyncstart;
    unsigned short      hsyncend;
    unsigned short      htotal;
    unsigned short      hskew;
    unsigned short      vdisplay;
    unsigned short      vsyncstart;
    unsigned short      vsyncend;
    unsigned short      vtotal;
    unsigned int        flags;
    int                 privsize;
    ptr_t               tc_private;
} my_XF86VidModeModeInfo_32_t;

typedef struct my_XColor_32_s {
	ulong_t pixel;
	unsigned short red, green, blue;
	char flags;
	char pad;
} my_XColor_32_t;

typedef struct my_XRRProviderInfo_32_s {
    unsigned int    capabilities;
    int             ncrtcs;
    ptr_t           crtcs;  //XID*
    int             noutputs;
    ptr_t           outputs;    //XID*
    ptr_t           name;   //char*
    int             nassociatedproviders;
    ptr_t           associated_providers;   //XID*
    ptr_t           associated_capability;  //unsigned int*
    int             nameLen;
} my_XRRProviderInfo_32_t;

typedef struct my_XRRProviderResources_32_t {
    ulong_t   timestamp;
    int       nproviders;
    ptr_t     providers;    //XID*
} my_XRRProviderResources_32_t;

typedef struct my_XIAnyClassInfo_32_s
{
    int         type;
    int         sourceid;
} my_XIAnyClassInfo_32_t;

typedef struct my_XIDeviceInfo_32_s
{
    int        deviceid;
    ptr_t      name;   //char*
    int        use;
    int        attachment;
    int        enabled;
    int        num_classes;
    ptr_t      classes;    //my_XIAnyClassInfo_t**
} my_XIDeviceInfo_32_t;

typedef struct my_XIEventMask_32_s {
    int deviceid;
    int mask_len;
    ptr_t mask; //unsigned char*
} my_XIEventMask_32_t;

typedef struct my_XInputClassInfo_32_s {
     unsigned char input_class;
     unsigned char event_type_base;
} my_XInputClassInfo_32_t;

typedef struct my_XDevice_32_s {
     XID_32 device_id;
     int    num_classes;
     ptr_t  classes;    //my_XInputClassInfo_t*
} my_XDevice_32_t;

typedef struct my_XDeviceKeyEvent_32_s
{
    int            type;
    ulong_t        serial;
    int            send_event;
    ptr_t          display; //Display*
    XID_32         window;
    XID_32         deviceid;
    XID_32         root;
    XID_32         subwindow;
    ulong_t        time;
    int            x, y;
    int            x_root;
    int            y_root;
    unsigned int   state;
    unsigned int   keycode;
    int            same_screen;
    unsigned int   device_state;
    unsigned char  axes_count;
    unsigned char  first_axis;
    int            axis_data[6];
} my_XDeviceKeyEvent_32_t;
typedef my_XDeviceKeyEvent_32_t my_XDeviceButtonEvent_32_t;
typedef struct my_XDeviceMotionEvent_32_s
{
    int            type;
    ulong_t        serial;
    int            send_event;
    ptr_t          display; //Display*
    XID_32         window;
    XID_32         deviceid;
    XID_32         root;
    XID_32         subwindow;
    ulong_t        time;
    int            x, y;
    int            x_root;
    int            y_root;
    unsigned int   state;
    char           is_hint;
    int            same_screen;
    unsigned int   device_state;
    unsigned char  axes_count;
    unsigned char  first_axis;
    int            axis_data[6];
} my_XDeviceMotionEvent_32_t;
typedef struct my_XDeviceFocusChangeEvent_32_s
{
    int           type;
    ulong_t       serial;
    int           send_event;
    ptr_t         display;  //Display*
    XID_32        window;
    XID_32        deviceid;
    int           mode;
    int           detail;
    ulong_t       time;
} my_XDeviceFocusChangeEvent_32_t;
typedef struct my_XProximityNotifyEvent_32_s
{
    int            type;
    ulong_t        serial;
    int            send_event;
    ptr_t          display; //Display*
    XID_32         window;
    XID_32         deviceid;
    XID_32         root;
    XID_32         subwindow;
    ulong_t        time;
    int            x, y;
    int            x_root;
    int            y_root;
    unsigned int   state;
    int            same_screen;
    unsigned int   device_state;
    unsigned char  axes_count;
    unsigned char  first_axis;
    int            axis_data[6];
} my_XProximityNotifyEvent_32_t;
typedef struct my_XDeviceStateNotifyEvent_32_s
{
    int           type;
    ulong_t       serial;
    int           send_event;
    ptr_t         display;  //Display*
    XID_32        window;
    XID_32        deviceid;
    ulong_t       time;
    int           num_classes;
    char	      data[64];
} my_XDeviceStateNotifyEvent_32_t;
typedef struct my_XDeviceMappingEvent_32_s
{
    int           type;
    ulong_t       serial;
    int           send_event;
    ptr_t         display;
    XID_32        window;
    XID_32        deviceid;
    ulong_t       time;
    int           request;
    int           first_keycode;
    int           count;
} my_XDeviceMappingEvent_32_t;
typedef struct my_XChangeDeviceNotifyEvent_32_s
{
    int           type;
    ulong_t       serial;
    int           send_event;
    ptr_t         display;
    XID_32        window;
    XID_32        deviceid;
    ulong_t       time;
    int           request;
} my_XChangeDeviceNotifyEvent_32_t;
typedef struct my_XDevicePresenceNotifyEvent_32_s
{
    int           type;
    ulong_t       serial;
    int           send_event;
    ptr_t         display;
    XID_32        window;
    ulong_t       time;
    int           devchange;
    XID_32        deviceid;
    XID_32        control;
} my_XDevicePresenceNotifyEvent_32_t;
typedef struct my_XDevicePropertyNotifyEvent_32_s
{
    int           type;
    ulong_t       serial;
    int           send_event;
    ptr_t         display;
    XID_32        window;
    ulong_t       time;
    XID_32        deviceid;
    XID_32        atom;
    int           state;
} my_XDevicePropertyNotifyEvent_32_t;
typedef struct my_XInputClass_32_s {
     unsigned char class;
     unsigned char length;
} my_XInputClass_32_t;
typedef struct my_XDeviceState_32_s {
     XID_32 device_id;
     int num_classes;
     ptr_t data;    //my_XInputClass_32_t*
} my_XDeviceState_32_t;

typedef struct my_XFixesSelectionNotifyEvent_32_s
{
    int     type;
    ulong_t serial;
    int     send_event;
    ptr_t   display;
    XID_32  window;
    int     subtype;
    ulong_t selection_timestamp;
    XID_32  owner;
    XID_32  selection;
    ulong_t timestamp;
} my_XFixesSelectionNotifyEvent_32_t;
typedef struct my_XFixesCursorNotifyEvent_32_s
{
    int     type;
    ulong_t serial;
    int     send_event;
    ptr_t   display;
    XID_32  window;
    int     subtype;
    ulong_t cursor_serial;
    ulong_t timestamp;
    XID_32  cursor_name;
} my_XFixesCursorNotifyEvent_32_t;

typedef struct my_XRRScreenChangeNotifyEvent_32_s
{
    int         type;
    ulong_t     serial;
    int         send_event;
    ptr_t       display;
    XID_32      window;
    XID_32      root;
    ulong_t     timestamp;
    ulong_t     config_timestamp;
    uint16_t    size_index;
    uint16_t    subpixel_order;
    uint16_t    rotation;
    int         width;
    int         height;
    int         mwidth;
    int         mheight;
} my_XRRScreenChangeNotifyEvent_32_t;
typedef struct my_XRRNotifyEvent_32_s
{
    int     type;
    ulong_t serial;
    int     send_event;
    ptr_t   display;
    XID_32  window;
    int     subtype;
} my_XRRNotifyEvent_32_t;
typedef struct my_XRROutputChangeNotifyEvent_32_s
{
    int         type;
    ulong_t     serial;
    int         send_event;
    ptr_t       display;
    XID_32      window;
    int         subtype;
    XID_32      output;
    XID_32      crtc;
    XID_32      mode;
    uint16_t    rotation;
    uint16_t    connection;
    uint16_t    subpixel_order;
} my_XRROutputChangeNotifyEvent_32_t;
typedef struct my_XRRCrtcChangeNotifyEvent_32_s
{
    int             type;
    ulong_t         serial;
    int             send_event;
    ptr_t           display;
    XID_32          window;
    int             subtype;
    XID_32          crtc;
    XID_32          mode;
    uint16_t        rotation;
    int             x, y;
    unsigned int    width, height;
} my_XRRCrtcChangeNotifyEvent_32_t;
typedef struct my_XRROutputPropertyNotifyEvent_32_s
{
    int     type;
    ulong_t serial;
    int     send_event;
    ptr_t   display;
    XID_32  window;
    int     subtype;
    XID_32  output;
    XID_32  property;
    ulong_t timestamp;
    int     state;
} my_XRROutputPropertyNotifyEvent_32_t;
typedef struct my_XRRProviderChangeNotifyEvent_32_s
{
    int             type;
    ulong_t         serial;
    int             send_event;
    ptr_t           display;
    XID_32          window;
    int             subtype;
    XID_32          provider;
    ulong_t         timestamp;
    unsigned int    current_role;
} my_XRRProviderChangeNotifyEvent_32_t;
typedef struct my_XRRProviderPropertyNotifyEvent_32_s
{
    int     type;
    ulong_t serial;
    int     send_event;
    ptr_t   display;
    XID_32  window;
    int     subtype;
    XID_32  provider;
    XID_32  property;
    ulong_t timestamp;
    int     state;
} my_XRRProviderPropertyNotifyEvent_32_t;
typedef struct my_XRRResourceChangeNotifyEvent_32_s
{
    int     type;
    ulong_t serial;
    int     send_event;
    ptr_t   display;
    XID_32  window;
    int     subtype;
    ulong_t timestamp;
} my_XRRResourceChangeNotifyEvent_32_t;

typedef struct my_XcursorCursors_32_s {
    ptr_t      dpy;     //Display*
    int        ref;
    int        ncursor;
    ptr_t      cursors; //Cursor*
} my_XcursorCursors_32_t;

typedef struct my_XcursorImage_32_s {
    uint32_t        version;
    uint32_t        size;
    uint32_t        width;
    uint32_t        height;
    uint32_t        xhot;
    uint32_t        yhot;
    uint32_t        delay;
    ptr_t           pixels; //XcursorPixel*
} my_XcursorImage_32_t;

typedef struct my_XcursorImages_32_s {
    int             nimage;
    ptr_t           images; //my_XcursorImage_t**
    ptr_t           name; //char*
} my_XcursorImages_32_t;

typedef struct my_XExtCodes_32_s {
    int extension;
    int major_opcode;
    int first_event;
    int first_error;
} my_XExtCodes_32_t;

typedef struct my_XExtDisplayInfo_32_s {
    ptr_t   next; //struct my_XExtDisplayInfo_s*
    ptr_t   display;    //Didsplay*
    ptr_t   codes; //my_XExtCodes_t*
    ptr_t   data;   //void*
} my_XExtDisplayInfo_32_t;

typedef struct my_XExtensionInfo_32_s {
    ptr_t head; //my_XExtDisplayInfo_t*
    ptr_t cur;  //my_XExtDisplayInfo_t*
    int   ndisplays;
} my_XExtensionInfo_32_t;

typedef struct my_XCharStruct_32_t {
    short       lbearing;
    short       rbearing;
    short       width;
    short       ascent;
    short       descent;
    unsigned short attributes;
} my_XCharStruct_32_t;

typedef struct my_XFontProp_32_s {
    XID_32  name;
    ulong_t card32;
} my_XFontProp_32_t;

typedef struct my_XFontStruct_32_s {
    ptr_t                   ext_data; //XExtData*
    XID_32                  fid;
    unsigned                direction;
    unsigned                min_char_or_byte2;
    unsigned                max_char_or_byte2;
    unsigned                min_byte1;
    unsigned                max_byte1;
    int                     all_chars_exist;
    unsigned                default_char;
    int                     n_properties;
    ptr_t                   properties;   //my_XFontProp_t*
    my_XCharStruct_32_t     min_bounds;
    my_XCharStruct_32_t     max_bounds;
    ptr_t                   per_char; //my_XCharStruct_t*
    int                     ascent;
    int                     descent;
} my_XFontStruct_32_t;

typedef struct my_XExtensionHooks_32_s {
    ptr_t create_gc; //int (*create_gc)(void* dpy, void* gc, my_XExtCodes_t* e);
    ptr_t copy_gc; //int (*copy_gc)(void* dpy, void* gc, my_XExtCodes_t* e);
    ptr_t flush_gc; //int (*flush_gc)(void* dpy, void* gc, my_XExtCodes_t* e);
    ptr_t free_gc; //int (*free_gc)(void* dpy, void* gc, my_XExtCodes_t* e);
    ptr_t create_font; //int (*create_font)(void* dpy, my_XFontStruct_t* f, my_XExtCodes_t* e);
    ptr_t free_font; //int (*free_font)(void* dpy, my_XFontStruct_t* f, my_XExtCodes_t* e);
    ptr_t close_display; //int (*close_display)(void* dpy, my_XExtCodes_t* e);
    ptr_t wire_to_event; //int (*wire_to_event)(void* dpy, my_XEvent_t* evt, void* xEvent);
    ptr_t event_to_wire; //int (*event_to_wire)(void* dpy, my_XEvent_t* evt, void* xEvent);
    ptr_t error; //int (*error)(void* dpy, void* xError, my_XExtCodes_t* e, int* n);
    ptr_t error_string; //char*(*error_string)(void* dpy, int, my_XExtCodes_t* e, char* s, int n);
} my_XExtensionHooks_32_t;

typedef struct my_XRRPropertyInfo_32_s {
    int     pending;
    int     range;
    int     immutable;
    int     num_values;
    ptr_t   values; //long*
} my_XRRPropertyInfo_32_t;

typedef struct my_XShmSegmentInfo_32_s {
    XID_32  shmseg;
    int     shmid;
    ptr_t   shmaddr;
    int     readOnly;
} my_XShmSegmentInfo_32_t;

typedef struct my_XkbNamesRec_32_s {
    XID_32          keycodes;
    XID_32          geometry;
    XID_32          symbols;
    XID_32          types;
    XID_32          compat;
    XID_32          vmods[16];
    XID_32          indicators[32];
    XID_32          groups[4];
    ptr_t           keys; //XkbKeyNameRec* as array[xkb->max_key_code + 1]
    ptr_t           key_aliases; //XkbKeyAliasRec* as array[num_key_aliases]
    ptr_t           radio_groups;   //XID*
    XID_32          phys_symbols;
    unsigned char   num_keys;
    unsigned char   num_key_aliases;
    unsigned short  num_rg;
} my_XkbNamesRec_32_t;

typedef struct my_XkbClientMapRec_32_s {
    unsigned char            size_types;
    unsigned char            num_types;
    ptr_t                    types; //XkbKeyTypePtr
    unsigned short           size_syms;
    unsigned short           num_syms;
    ptr_t                    syms;  //KeySym*
    ptr_t                    key_sym_map;   //XkbSymMapPtr
    ptr_t                    modmap;    //unsigned char*
} my_XkbClientMapRec_32_t;

typedef struct my_XkbDescRec_32_s {
   ptr_t              display; //my_XDisplay_32_t*
   unsigned short     flags;
   unsigned short     device_spec;
   uint8_t            min_key_code;
   uint8_t            max_key_code;
   ptr_t              ctrls; //XkbControlsPtr
   ptr_t              server; //XkbServerMapPtr
   ptr_t              map; //XkbClientMapPtr
   ptr_t              indicators; //XkbIndicatorPtr
   ptr_t              names; //my_XkbNamesRec_32_t*
   ptr_t              compat; //XkbCompatMapPtr
   ptr_t              geom; //XkbGeometryPtr
} my_XkbDescRec_32_t;

typedef struct my_XmbTextItem_32_s {
    ptr_t       chars;  //char*
    int         nchars;
    int         delta;
    ptr_t       font_set;   //XFontSet
} my_XmbTextItem_32_t;

typedef struct my_XwcTextItem_32_s {
    ptr_t           chars;  //wchar_t*
    int             nchars;
    int             delta;
    ptr_t           font_set;   //XFontSet
} my_XwcTextItem_32_t;

// virtual structure
typedef struct my_XFontSet_32_s {
    void*   fontset;    // the actual fontset
    ptr_t** names;      // arrays of array of names
    int names_size;     // size of names
    ptr_t** fonts;      // array of array of fonts
    int fonts_size;     // soze of fonts
} my_XFontSet_32_t;

typedef struct my_XAnyClassinfo_32_s {
    XID_32      c_class;
    int         length;
} my_XAnyClassInfo_32_t;

typedef struct my_XKeyInfo_32_s
{
    XID_32              c_class;
    int                 length;
    unsigned short      min_keycode;
    unsigned short      max_keycode;
    unsigned short      num_keys;
} my_XKeyInfo_32_t;
typedef struct my_XButtonInfo_32_s
{
    XID_32      c_class;
    int         length;
    short       num_buttons;
} my_XButtonInfo_32_t;
typedef struct  my_XValuatorInfo_32_s
{
    XID_32              c_class;
    int                 length;
    unsigned char       num_axes;
    unsigned char       mode;
    ulong_t             motion_buffer;
    ptr_t               axes;   //XAxisInfoPtr
} my_XValuatorInfo_32_t;

typedef struct my_XDeviceInfo_32_s
{
    XID_32              id;        
    XID_32              type;
    ptr_t               name;   //char*
    int                 num_classes;
    int                 use;
    ptr_t               inputclassinfo; //my_XAnyClassInfo_32_t*
} my_XDeviceInfo_32_t;

typedef struct my_XTimeCoord_32_s {
	ulong_t time;
	short x, y;
} my_XTimeCoord_32_t;

typedef struct my_XDeviceTimeCoord_32_s {
    ulong_t   time;
    ptr_t     data; //int*
} my_XDeviceTimeCoord_32_t;

typedef struct my_XFixesCursorImage_32_s {
    short           x, y;
    unsigned short  width, height;
    unsigned short  xhot, yhot;
    ulong_t         cursor_serial;
    ptr_t           pixels; //unsigned long*
    XID_32          atom;                   /* Version >= 2 only */
    ptr_t           name;                  /* Version >= 2 only */
} my_XFixesCursorImage_32_t;

typedef struct my_XFilters_32_s {
    int nfilter;
    ptr_t filter;   //char **
    int nalias;
    ptr_t alias;    //short *
} my_XFilters_32_t;

typedef struct my__XRRMonitorInfo_32_s {
    XID_32  name;
    int     primary;
    int     automatic;
    int     noutput;
    int     x;
    int     y;
    int     width;
    int     height;
    int     mwidth;
    int     mheight;
    ptr_t   outputs;   //XID*
} my_XRRMonitorInfo_32_t;

typedef struct my_XkbAnyEvent_32_s {
    int             type;
    ulong_t         serial;
    int             send_event;
    ptr_t           display;    //my_XDisplay_t*
    ulong_t         time;
    int             xkb_type;
    unsigned int    device;
} my_XkbAnyEvent_32_t;

typedef struct my_XkbNewKeyboardNotify_32_s {
    int             type;
    ulong_t         serial;
    int             send_event;
    ptr_t           display;    //my_XDisplay_t*
    ulong_t         time;
    int             xkb_type;
    int             device;
    int             old_device;
    int             min_key_code;
    int             max_key_code;
    int             old_min_key_code;
    int             old_max_key_code;
    unsigned int    changed;
    char            req_major;
    char            req_minor;
} my_XkbNewKeyboardNotifyEvent_32_t;

typedef struct my_XkbMapNotifyEvent_32_s {
    int             type;
    ulong_t         serial;
    int             send_event;
    ptr_t           display;    //my_XDisplay_t*
    ulong_t         time;
    int             xkb_type;
    int             device;
    unsigned int    changed;
    unsigned int    flags;
    int             first_type;
    int             num_types;
    uint8_t         min_key_code;
    uint8_t         max_key_code;
    uint8_t         first_key_sym;
    uint8_t         first_key_act;
    uint8_t         first_key_behavior;
    uint8_t         first_key_explicit;
    uint8_t         first_modmap_key;
    uint8_t         first_vmodmap_key;
    int             num_key_syms;
    int             num_key_acts;
    int             num_key_behaviors;
    int             num_key_explicit;
    int             num_modmap_keys;
    int             num_vmodmap_keys;
    unsigned int    vmods;
} my_XkbMapNotifyEvent_32_t;

typedef struct my_XkbStateNotifyEvent_32_s {
    int             type;
    ulong_t         serial;
    int             send_event;
    ptr_t           display;    //my_XDisplay_t*
    ulong_t         time;
    int             xkb_type;
    int             device;
    unsigned int    changed;
    int             group;
    int             base_group;
    int             latched_group;
    int             locked_group;
    unsigned int    mods;
    unsigned int    base_mods;
    unsigned int    latched_mods;
    unsigned int    locked_mods;
    int             compat_state;
    unsigned char   grab_mods;
    unsigned char   compat_grab_mods;
    unsigned char   lookup_mods;
    unsigned char   compat_lookup_mods;
    int             ptr_buttons;
    uint8_t         keycode;
    char            event_type;
    char            req_major;
    char            req_minor;
} my_XkbStateNotifyEvent_32_t;

typedef struct my_XkbControlsNotify_32_s {
    int             type;
    ulong_t         serial;
    int             send_event;
    ptr_t           display;    //my_XDisplay_t*
    ulong_t         time;
    int             xkb_type;
    int             device;
    unsigned int    changed_ctrls;
    unsigned int    enabled_ctrls;
    unsigned int    enabled_ctrl_changes;
    int             num_groups;
    uint8_t         keycode;
    char            event_type;
    char            req_major;
    char            req_minor;
} my_XkbControlsNotifyEvent_32_t;

typedef struct my_XkbIndicatorNotify_32_s {
    int             type;
    ulong_t         serial;
    int             send_event;
    ptr_t           display;    //my_XDisplay_t*
    ulong_t         time;
    int             xkb_type;
    int             device;
    unsigned int    changed;
    unsigned int    state;
} my_XkbIndicatorNotifyEvent_32_t;

typedef struct my_XkbNamesNotify_32_s {
    int             type;
    ulong_t         serial;
    int             send_event;
    ptr_t           display;    //my_XDisplay_t*
    ulong_t         time;
    int             xkb_type;
    int             device;
    unsigned int    changed;
    int             first_type;
    int             num_types;
    int             first_lvl;
    int             num_lvls;
    int             num_aliases;
    int             num_radio_groups;
    unsigned int    changed_vmods;
    unsigned int    changed_groups;
    unsigned int    changed_indicators;
    int             first_key;
    int             num_keys;
} my_XkbNamesNotifyEvent_32_t;

typedef struct my_XkbCompatMapNotify_32_s {
    int             type;
    ulong_t         serial;
    int             send_event;
    ptr_t           display;    //my_XDisplay_t*
    ulong_t         time;
    int             xkb_type;
    int             device;
    unsigned int    changed_groups;
    int             first_si;
    int             num_si;
    int             num_total_si;
} my_XkbCompatMapNotifyEvent_32_t;

typedef struct my_XkbBellNotify_32_s {
    int             type;
    ulong_t         serial;
    int             send_event;
    ptr_t           display;    //my_XDisplay_t*
    ulong_t         time;
    int             xkb_type;
    int             device;
    int             percent;
    int             pitch;
    int             duration;
    int             bell_class;
    int             bell_id;
    XID_32          name;
    XID_32          window;
    int             event_only;
} my_XkbBellNotifyEvent_32_t;

typedef struct my_XkbActionMessage_32_s {
    int             type;
    ulong_t         serial;
    int             send_event;
    ptr_t           display;    //my_XDisplay_t*
    ulong_t         time;
    int             xkb_type;
    int             device;
    uint8_t         keycode;
    int             press;
    int             key_event_follows;
    int             group;
    unsigned int    mods;
    char            message[6+1];
} my_XkbActionMessageEvent_32_t;

typedef struct my_XkbAccessXNotify_32_s {
    int             type;
    ulong_t         serial;
    int             send_event;
    ptr_t           display;    //my_XDisplay_t*
    ulong_t         time;
    int             xkb_type;
    int             device;
    int             detail;
    int             keycode;
    int             sk_delay;
    int             debounce_delay;
} my_XkbAccessXNotifyEvent_32_t;

typedef struct my_XkbExtensionDeviceNotify_32_s {
    int             type;
    ulong_t         serial;
    int             send_event;
    ptr_t           display;    //my_XDisplay_t*
    ulong_t         time;
    int             xkb_type;
    int             device;
    unsigned int    reason;
    unsigned int    supported;
    unsigned int    unsupported;
    int             first_btn;
    int             num_btns;
    unsigned int    leds_defined;
    unsigned int    led_state;
    int             led_class;
    int             led_id;
} my_XkbExtensionDeviceNotifyEvent_32_t;

typedef union my_XkbEvent_32_s {
    int                                     type;
    my_XkbAnyEvent_32_t                     any;
    my_XkbNewKeyboardNotifyEvent_32_t       new_kbd;
    my_XkbMapNotifyEvent_32_t               map;
    my_XkbStateNotifyEvent_32_t             state;
    my_XkbControlsNotifyEvent_32_t          ctrls;
    my_XkbIndicatorNotifyEvent_32_t         indicators;
    my_XkbNamesNotifyEvent_32_t             names;
    my_XkbCompatMapNotifyEvent_32_t         compat;
    my_XkbBellNotifyEvent_32_t              bell;
    my_XkbActionMessageEvent_32_t           message;
    my_XkbAccessXNotifyEvent_32_t           accessx;
    my_XkbExtensionDeviceNotifyEvent_32_t   device;
    my_XEvent_32_t                          core;
} my_XkbEvent_32_t;

typedef struct my_XIEvent_32_s {
    int           type;
    ulong_t       serial;
    int           send_event;
    ptr_t         display;
    int           extension;
    int           evtype;
    ulong_t       time;
} my_XIEvent_32_t;

typedef struct my_XIButtonState_32_s {
    int           mask_len;
    ptr_t         mask; //unsigned char*
} my_XIButtonState_32_t;

typedef struct my_XIValuatorState_32_s {
    int           mask_len;
    ptr_t         mask; //unsigned char*
    ptr_t         values; //double*
} my_XIValuatorState_32_t;

typedef struct my_XIHierarchyEvent_32_s {
    int         type;
    ulong_t     serial;
    int         send_event;
    ptr_t       display;
    int         extension;
    int         evtype;
    ulong_t     time;
    int         flags;
    int         num_info;
    ptr_t       info;   //my_XIHierarchyInfo_t*
} my_XIHierarchyEvent_32_t;

typedef struct my_XIDeviceChangedEvent_32_s {
    int                     type;
    ulong_t                 serial;
    int                     send_event;
    ptr_t                   display;
    int                     extension;
    int                     evtype;
    ulong_t                 time;
    int                     deviceid;
    int                     sourceid;
    int                     reason;
    int                     num_classes;
    ptr_t                   classes;    //my_XIAnyClassInfo_t**
} my_XIDeviceChangedEvent_32_t;

typedef struct my_XIDeviceEvent_32_s {
    int                     type;
    ulong_t                 serial;
    int                     send_event;
    ptr_t                   display;
    int                     extension;
    int                     evtype;
    ulong_t                 time;
    int                     deviceid;
    int                     sourceid;
    int                     detail;
    XID_32                  root;
    XID_32                  event;
    XID_32                  child;
    double                  root_x;
    double                  root_y;
    double                  event_x;
    double                  event_y;
    int                     flags;
    my_XIButtonState_32_t   buttons;
    my_XIValuatorState_32_t valuators;
    my_XIModifierState_t    mods;
    my_XIModifierState_t    group;
} __attribute__((packed)) my_XIDeviceEvent_32_t;

typedef struct my_XIRawEvent_32_s {
    int                     type;
    ulong_t                 serial;
    int                     send_event;
    ptr_t                   display;
    int                     extension;
    int                     evtype;
    ulong_t                 time;
    int                     deviceid;
    int                     sourceid;
    int                     detail;
    int                     flags;
    my_XIValuatorState_32_t valuators;
    ptr_t                   raw_values; //double*
} my_XIRawEvent_32_t;

typedef struct my_XIEnterEvent_32_s {
    int                     type;
    ulong_t                 serial;
    int                     send_event;
    ptr_t                   display;
    int                     extension;
    int                     evtype;
    ulong_t                 time;
    int                     deviceid;
    int                     sourceid;
    int                     detail;
    XID                     root;
    XID                     event;
    XID                     child;
    double                  root_x;
    double                  root_y;
    double                  event_x;
    double                  event_y;
    int                     mode;
    int                     focus;
    int                     same_screen;
    my_XIButtonState_32_t   buttons;
    my_XIModifierState_t    mods;
    my_XIModifierState_t    group;
} __attribute__((packed)) my_XIEnterEvent_32_t;
// Leave, FocusIn and FocusOut are like Enter

typedef struct my_XIPropertyEvent_32_s {
    int           type;
    ulong_t       serial;
    int           send_event;
    ptr_t         display;
    int           extension;
    int           evtype;
    ulong_t       time;
    int           deviceid;
    XID_32        property;
    int           what;
} my_XIPropertyEvent_32_t;

typedef struct my_XITouchOwnershipEvent_32_s {
    int           type;
    ulong_t       serial;
    int           send_event;
    ptr_t         display;
    int           extension;
    int           evtype;
    ulong_t       time;
    int           deviceid;
    int           sourceid;
    unsigned int  touchid;
    XID_32        root;
    XID_32        event;
    XID_32        child;
    int           flags;
} my_XITouchOwnershipEvent_32_t;

typedef struct my_XIBarrierEvent_32_s {
    int           type;
    ulong_t       serial;
    int           send_event;
    ptr_t         display;
    int           extension;
    int           evtype;
    ulong_t       time;
    int           deviceid;
    int           sourceid;
    XID_32        event;
    XID_32        root;
    double        root_x;
    double        root_y;
    double        dx;
    double        dy;
    int           dtime;
    int           flags;
    XID_32        barrier;
    uint32_t      eventid;
} __attribute__((packed)) my_XIBarrierEvent_32_t;

typedef struct my_XIGesturePinchEvent_32_s {
    int                     type;
    ulong_t                 serial;
    int                     send_event;
    ptr_t                   display;
    int                     extension;
    int                     evtype;
    ulong_t                 time;
    int                     deviceid;
    int                     sourceid;
    int                     detail;
    XID_32                  root;
    XID_32                  event;
    XID_32                  child;
    double                  root_x;
    double                  root_y;
    double                  event_x;
    double                  event_y;
    double                  delta_x;
    double                  delta_y;
    double                  delta_unaccel_x;
    double                  delta_unaccel_y;
    double                  scale;
    double                  delta_angle;
    int                     flags;
    my_XIModifierState_t    mods;
    my_XIModifierState_t    group;
} __attribute__((packed)) my_XIGesturePinchEvent_32_t;

typedef struct my_XIGestureSwipeEvent_32_s {
    int                     type;
    ulong_t                 serial;
    int                     send_event;
    ptr_t                   display;
    int                     extension;
    int                     evtype;
    ulong_t                 time;
    int                     deviceid;
    int                     sourceid;
    int                     detail;
    XID_32                  root;
    XID_32                  event;
    XID_32                  child;
    double                  root_x;
    double                  root_y;
    double                  event_x;
    double                  event_y;
    double                  delta_x;
    double                  delta_y;
    double                  delta_unaccel_x;
    double                  delta_unaccel_y;
    int                     flags;
    my_XIModifierState_t    mods;
    my_XIModifierState_t    group;
} __attribute__((packed)) my_XIGestureSwipeEvent_32_t;

typedef struct my_XRRCrtcTransformAttributes_32_s {
    my_XTransform_t     pendingTransform;
    ptr_t               pendingFilter;  //char*
    int                 pendingNparams;
    ptr_t               pendingParams;  //int*
    my_XTransform_t     currentTransform;
    ptr_t               currentFilter;  //char*
    int                 currentNparams;
    ptr_t               currentParams;  //int*
} my_XRRCrtcTransformAttributes_32_t;

#endif//MY_X11_DEFS_32