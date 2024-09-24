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
    ptr_t    obdata;
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
        ptr_t keysyms;  //void *
        ptr_t modifiermap;  //void *
        int keysyms_per_keycode;
        ptr_t xdefaults;    //char *
        ptr_t scratch_buffer;   //char *
        ulong_t scratch_length;
        int ext_number;
        ptr_t ext_procs;    //struct my_XExten *
        ptr_t event_vec;    //int (*event_vec[128])(void *, void *, void *);
        ptr_t wire_vec; //int (*wire_vec[128])(void *, void *, void *);
        XID_32 lock_meaning;
        ptr_t lock; //void* 
        ptr_t async_handlers;   //struct my_XInternalAsync *
        ulong_t bigreq_size;
        ptr_t lock_fns; //struct my_XLockPtrs *
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
    void           *data;
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


#endif//MY_X11_DEFS_32