#ifndef MY_X11_DEFS
#define MY_X11_DEFS
#include <stdint.h>

typedef unsigned long XID;

typedef struct XImageSave_s {
    int   anyEmu;
    void* create;
    void* destroy;
    void* get;
    void* put;
    void* sub;
    void* add;
} XImageSave_t;

typedef struct ximage_s {
    void*(*create_image)(
            void*           /* display */,
            void*           /* visual */,
            uint32_t        /* depth */,
            int32_t         /* format */,
            int32_t         /* offset */,
            void*           /* data */,
            uint32_t        /* width */,
            uint32_t        /* height */,
            int32_t         /* bitmap_pad */,
            int32_t         /* bytes_per_line */);
    int32_t (*destroy_image)        (void*);
    uintptr_t (*get_pixel)           (void*, int32_t, int32_t);
    int32_t (*put_pixel)            (void*, int32_t, int32_t, uintptr_t);
    void*(*sub_image)    (void*, int32_t, int32_t, uint32_t, uint32_t); //sub_image return a new XImage that need bridging => custom wrapper
    int32_t (*add_pixel)            (void*, intptr_t);
} ximage_t;

typedef struct _XImage {
    int32_t width, height;          /* size of image */
    int32_t xoffset;                /* number of pixels offset in X direction */
    int32_t format;                 /* XYBitmap, XYPixmap, ZPixmap */
    void*   data;                   /* pointer to image data */
    int32_t byte_order;             /* data byte order, LSBFirst, MSBFirst */
    int32_t bitmap_unit;            /* quant. of scanline 8, 16, 32 */
    int32_t bitmap_bit_order;       /* LSBFirst, MSBFirst */
    int32_t bitmap_pad;             /* 8, 16, 32 either XY or ZPixmap */
    int32_t depth;                  /* depth of image */
    int32_t bytes_per_line;         /* accelarator to next line */
    int32_t bits_per_pixel;         /* bits per pixel (ZPixmap) */
    uintptr_t red_mask;             /* bits in z arrangment */
    uintptr_t green_mask;
    uintptr_t blue_mask;
    void*    obdata;                 /* hook for the object routines to hang on */
    ximage_t f;

} XImage;

struct my_XFreeFuncs {
    void* atoms;
    void* modifiermap;
    void* key_bindings;
    void* context_db;
    void* defaultCCCs;
    void* clientCmaps;
    void* intensityMaps;
    void* im_filters;
    void* xkb;
};

struct my_XExten {
    struct my_XExten *next;
    void* codes;    // XExtCodes
    void* create_GC;    // CreateGCType
    void* copy_GC;  // CopyGCType
    void* flush_GC; // FlushGCType
    void* free_GC;  // FreeGCType
    void* create_Font;  // CreateFontType
    void* free_Font;    // FreeFontType
    void* close_display;    // CloseDisplayType
    void* error;    // ErrorType
    void* error_string; // ErrorStringType
    char *name;
    void* error_values; // PrintErrorType
    void* before_flush; // BeforeFlushType
    struct my_XExten *next_flush;
};

struct my_XInternalAsync {
    struct my_XInternalAsync *next;
    int (*handler)(void*, void*, char*, int, void*);
    void* data;
};

struct my_XLockPtrs {
    void (*lock_display)(void* dpy);
    void (*unlock_display)(void *dpy);
};

struct my_XConnectionInfo {
    int fd;
    void* read_callback;    // _XInternalConnectionProc
    void* call_data;
    void* *watch_data;
    struct my_XConnectionInfo *next;
};

struct my_XConnWatchInfo {
    void* fn;   // XConnectionWatchProc
    void* client_data;
    struct _XConnWatchInfo *next;
};

typedef struct my_Visual_s {
    void*   ext_data;   //XExtData*
    XID visualid;
    int c_class;
    unsigned long red_mask, green_mask, blue_mask;
    int bits_per_rgb;
    int map_entries;
} my_Visual_t;

typedef struct my_Screen_s {
    void* ext_data;         //XExtData *
    struct my_XDisplay_s *display;
    XID root;
    int width, height;
    int mwidth, mheight;
    int ndepths;
    void* depths;   //Depth *
    int root_depth;         /* bits per pixel */
    my_Visual_t* root_visual;
    void* default_gc;   //GC == struct _XGC*
    XID cmap;
    unsigned long white_pixel;
    unsigned long black_pixel;
    int max_maps, min_maps;
    int backing_store;
    int save_unders;
    long root_input_mask;
} my_Screen_t;

typedef struct my_XDisplay_s
{
    void *ext_data;
    struct my_XFreeFuncs *free_funcs;
    int fd;
    int conn_checker;
    int proto_major_version;
    int proto_minor_version;
    char *vendor;
    XID resource_base;
    XID resource_mask;
    XID resource_id;
    int resource_shift;
    XID (*resource_alloc)(void*);
    int byte_order;
    int bitmap_unit;
    int bitmap_pad;
    int bitmap_bit_order;
    int nformats;
    void *pixmap_format;
    int vnumber;
    int release;
    void *head, *tail;
    int qlen;
    unsigned long last_request_read;
    unsigned long request;
    char *last_req;
    char *buffer;
    char *bufptr;
    char *bufmax;
    unsigned max_request_size;
    void* *db;
    int (*synchandler)(void*);
    char *display_name;
    int default_screen;
    int nscreens;
    my_Screen_t *screens;
    unsigned long motion_buffer;
    volatile unsigned long flags;
    int min_keycode;
    int max_keycode;
    void *keysyms;
    void *modifiermap;
    int keysyms_per_keycode;
    char *xdefaults;
    char *scratch_buffer;
    unsigned long scratch_length;
    int ext_number;
    struct my_XExten *ext_procs;
    int (*event_vec[128])(void *, void *, void *);
    int (*wire_vec[128])(void *, void *, void *);
    XID lock_meaning;
    void* lock;
    struct my_XInternalAsync *async_handlers;
    unsigned long bigreq_size;
    struct my_XLockPtrs *lock_fns;
    void (*idlist_alloc)(void *, void *, int);
    void* key_bindings;
    XID cursor_font;
    void* *atoms;
    unsigned int mode_switch;
    unsigned int num_lock;
    void* context_db;
    int (**error_vec)(void*, void*, void*);
    struct {
        void* defaultCCCs;
        void* clientCmaps;
        void* perVisualIntensityMaps;
    } cms;
    void* im_filters;
    void* qfree;
    unsigned long next_event_serial_num;
    struct my_XExten *flushes;
    struct my_XConnectionInfo *im_fd_info;
    int im_fd_length;
    struct my_XConnWatchInfo *conn_watchers;
    int watcher_count;
    void* filedes;
    int (*savedsynchandler)(void *);
    XID resource_max;
    int xcmisc_opcode;
    void* *xkb_info;
    void* *trans_conn;
    void* *xcb;
    unsigned int next_cookie;
    int (*generic_event_vec[128])(void*, void*, void*);
    int (*generic_event_copy_vec[128])(void*, void*, void*);
    void *cookiejar;
    unsigned long last_request_read_upper32bit; // 64bits only
    unsigned long request_upper32bit;   // 64bits only
    void* error_threads;
    void* exit_handler;
    void* exit_handler_data;
} my_XDisplay_t;

typedef struct my_XSetWindowAttributes_s {
    XID background_pixmap;
    unsigned long background_pixel;
    XID border_pixmap;
    unsigned long border_pixel;
    int bit_gravity;
    int win_gravity;
    int backing_store;
    unsigned long backing_planes;
    unsigned long backing_pixel;
    int save_under;
    long event_mask;
    long do_not_propagate_mask;
    int override_redirect;
    XID colormap;
    XID cursor;
} my_XSetWindowAttributes_t;

typedef struct my_XKeyEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID window;
    XID root;
    XID subwindow;
    unsigned long time;
    int x, y;
    int x_root, y_root;
    unsigned int state;
    unsigned int keycode;
    int same_screen;
} my_XKeyEvent_t;
typedef my_XKeyEvent_t my_XKeyPressedEvent_t;
typedef my_XKeyEvent_t my_XKeyReleasedEvent_t;

typedef struct my_XButtonEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID window;
    XID root;
    XID subwindow;
    unsigned long time;
    int x, y;
    int x_root, y_root;
    unsigned int state;
    unsigned int button;
    int same_screen;
} my_XButtonEvent_t;
typedef my_XButtonEvent_t my_XButtonPressedEvent_t;
typedef my_XButtonEvent_t my_XButtonReleasedEvent_t;

typedef struct my_XMotionEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID window;
    XID root;
    XID subwindow;
    unsigned long time;
    int x, y;
    int x_root, y_root;
    unsigned int state;
    char is_hint;
    int same_screen;
} my_XMotionEvent_t;
typedef my_XMotionEvent_t my_XPointerMovedEvent_t;

typedef struct my_XCrossingEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID window;
    XID root;
    XID subwindow;
    unsigned long time;
    int x, y;
    int x_root, y_root;
    int mode;
    int detail;
    int same_screen;
    int focus;
    unsigned int state;
} my_XCrossingEvent_t;

typedef my_XCrossingEvent_t my_XEnterWindowEvent_t;
typedef my_XCrossingEvent_t my_XLeaveWindowEvent_t;

typedef struct my_XFocusChangeEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID window;
    int mode;
    int detail;
} my_XFocusChangeEvent_t;
typedef my_XFocusChangeEvent_t my_XFocusInEvent_t;
typedef my_XFocusChangeEvent_t my_XFocusOutEvent_t;

typedef struct my_XKeymapEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID window;
    char key_vector[32];
} my_XKeymapEvent_t;

typedef struct my_XExposeEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID window;
    int x, y;
    int width, height;
    int count;
} my_XExposeEvent_t;

typedef struct my_XGraphicsExposeEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID drawable;
    int x, y;
    int width, height;
    int count;
    int major_code;
    int minor_code;
} my_XGraphicsExposeEvent_t;

typedef struct my_XNoExposeEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID drawable;
    int major_code;
    int minor_code;
} my_XNoExposeEvent_t;

typedef struct my_XVisibilityEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID window;
    int state;
} my_XVisibilityEvent_t;

typedef struct my_XCreateWindowEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID parent;
    XID window;
    int x, y;
    int width, height;
    int border_width;
    int override_redirect;
} my_XCreateWindowEvent_t;

typedef struct my_XDestroyWindowEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID event;
    XID window;
} my_XDestroyWindowEvent_t;

typedef struct my_XUnmapEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID event;
    XID window;
    int from_configure;
} my_XUnmapEvent_t;

typedef struct my_XMapEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID event;
    XID window;
    int override_redirect;
} my_XMapEvent_t;

typedef struct my_XMapRequestEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID parent;
    XID window;
} my_XMapRequestEvent_t;

typedef struct my_XReparentEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID event;
    XID window;
    XID parent;
    int x, y;
    int override_redirect;
} my_XReparentEvent_t;

typedef struct my_XConfigureEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID event;
    XID window;
    int x, y;
    int width, height;
    int border_width;
    XID above;
    int override_redirect;
} my_XConfigureEvent_t;

typedef struct my_XGravityEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID event;
    XID window;
    int x, y;
} my_XGravityEvent_t;

typedef struct my_XResizeRequestEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID window;
    int width, height;
} my_XResizeRequestEvent_t;

typedef struct my_XConfigureRequestEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID parent;
    XID window;
    int x, y;
    int width, height;
    int border_width;
    XID above;
    int detail;
    unsigned long value_mask;
} my_XConfigureRequestEvent_t;

typedef struct my_XCirculateEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID event;
    XID window;
    int place;
} my_XCirculateEvent_t;

typedef struct my_XCirculateRequestEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID parent;
    XID window;
    int place;
} my_XCirculateRequestEvent_t;

typedef struct my_XPropertyEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID window;
    XID atom;
    unsigned long time;
    int state;
} my_XPropertyEvent_t;

typedef struct my_XSelectionClearEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID window;
    XID selection;
    unsigned long time;
} my_XSelectionClearEvent_t;

typedef struct my_XSelectionRequestEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID owner;
    XID requestor;
    XID selection;
    XID target;
    XID property;
    unsigned long time;
} my_XSelectionRequestEvent_t;

typedef struct my_XSelectionEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID requestor;
    XID selection;
    XID target;
    XID property;
    unsigned long time;
} my_XSelectionEvent_t;

typedef struct my_XColormapEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID window;
    XID colormap;
    int c_new;
    int state;
} my_XColormapEvent_t;

typedef struct my_XClientMessageEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID window;
    XID message_type;
    int format;
    union {
            char b[20];
            short s[10];
            long l[5];
            } data;
} my_XClientMessageEvent_t;

typedef struct my_XMappingEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID window;
    int request;
    int first_keycode;
    int count;
} my_XMappingEvent_t;

typedef struct my_XErrorEvent_s
{
    int type;
    my_XDisplay_t *display;
    XID           resourceid;
    unsigned long serial;
    unsigned char error_code;
    unsigned char request_code;
    unsigned char minor_code;
} my_XErrorEvent_t;

typedef struct my_XAnyEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    my_XDisplay_t *display;
    XID window;
} my_XAnyEvent_t;

typedef struct my_XGenericEvent_s
{
    int            type;
    unsigned long  serial;
    int            send_event;
    my_XDisplay_t  *display;
    int            extension;
    int            evtype;
} my_XGenericEvent_t;

typedef struct my_XGenericEventCookie_s
{
    int            type;
    unsigned long  serial;
    int            send_event;
    my_XDisplay_t  *display;
    int            extension;
    int            evtype;
    unsigned int   cookie;
    void           *data;
} my_XGenericEventCookie_t;

typedef union my_XEvent_s {
        int type;
        my_XAnyEvent_t xany;
        my_XKeyEvent_t xkey;
        my_XButtonEvent_t xbutton;
        my_XMotionEvent_t xmotion;
        my_XCrossingEvent_t xcrossing;
        my_XFocusChangeEvent_t xfocus;
        my_XExposeEvent_t xexpose;
        my_XGraphicsExposeEvent_t xgraphicsexpose;
        my_XNoExposeEvent_t xnoexpose;
        my_XVisibilityEvent_t xvisibility;
        my_XCreateWindowEvent_t xcreatewindow;
        my_XDestroyWindowEvent_t xdestroywindow;
        my_XUnmapEvent_t xunmap;
        my_XMapEvent_t xmap;
        my_XMapRequestEvent_t xmaprequest;
        my_XReparentEvent_t xreparent;
        my_XConfigureEvent_t xconfigure;
        my_XGravityEvent_t xgravity;
        my_XResizeRequestEvent_t xresizerequest;
        my_XConfigureRequestEvent_t xconfigurerequest;
        my_XCirculateEvent_t xcirculate;
        my_XCirculateRequestEvent_t xcirculaterequest;
        my_XPropertyEvent_t xproperty;
        my_XSelectionClearEvent_t xselectionclear;
        my_XSelectionRequestEvent_t xselectionrequest;
        my_XSelectionEvent_t xselection;
        my_XColormapEvent_t xcolormap;
        my_XClientMessageEvent_t xclient;
        my_XMappingEvent_t xmapping;
        my_XErrorEvent_t xerror;
        my_XKeymapEvent_t xkeymap;
        my_XGenericEvent_t xgeneric;
        my_XGenericEventCookie_t xcookie;
        long pad[24];
} my_XEvent_t;

#define XEVT_KeyPress                2
#define XEVT_KeyRelease              3
#define XEVT_ButtonPress             4
#define XEVT_ButtonRelease           5
#define XEVT_MotionNotify            6
#define XEVT_EnterNotify             7
#define XEVT_LeaveNotify             8
#define XEVT_FocusIn                 9
#define XEVT_FocusOut                10
#define XEVT_KeymapNotify            11
#define XEVT_Expose                  12
#define XEVT_GraphicsExpose          13
#define XEVT_NoExpose                14
#define XEVT_VisibilityNotify        15
#define XEVT_CreateNotify            16
#define XEVT_DestroyNotify           17
#define XEVT_UnmapNotify             18
#define XEVT_MapNotify               19
#define XEVT_MapRequest              20
#define XEVT_ReparentNotify          21
#define XEVT_ConfigureNotify         22
#define XEVT_ConfigureRequest        23
#define XEVT_GravityNotify           24
#define XEVT_ResizeRequest           25
#define XEVT_CirculateNotify         26
#define XEVT_CirculateRequest        27
#define XEVT_PropertyNotify          28
#define XEVT_SelectionClear          29
#define XEVT_SelectionRequest        30
#define XEVT_SelectionNotify         31
#define XEVT_ColormapNotify          32
#define XEVT_ClientMessage           33
#define XEVT_MappingNotify           34
#define XEVT_GenericEvent            35


// WMHints 
typedef struct my_XWMHints_s {
    long flags;
    int input;
    int initial_state;
    XID icon_pixmap;
    XID icon_window;
    int icon_x;
    int icon_y;
    XID icon_mask;
    XID window_group;
} my_XWMHints_t;

#define XWMHint_InputHint               (1L << 0)
#define XWMHint_StateHint               (1L << 1)
#define XWMHint_IconPixmapHint          (1L << 2)
#define XWMHint_IconWindowHint          (1L << 3)
#define XWMHint_IconPositionHint        (1L << 4)
#define XWMHint_IconMaskHint            (1L << 5)
#define XWMHint_WindowGroupHint         (1L << 6)
#define XWMHint_XUrgencyHint            (1L << 8)

typedef struct my_XRRModeInfo_s {
    XID                 id;
    unsigned int        width;
    unsigned int        height;
    unsigned long       dotClock;
    unsigned int        hSyncStart;
    unsigned int        hSyncEnd;
    unsigned int        hTotal;
    unsigned int        hSkew;
    unsigned int        vSyncStart;
    unsigned int        vSyncEnd;
    unsigned int        vTotal;
    char                *name;
    unsigned int        nameLength;
    unsigned long       modeFlags;
} my_XRRModeInfo_t;


typedef struct my_XRRScreenResources_s {
    unsigned long   timestamp;
    unsigned long   configTimestamp;
    int             ncrtc;
    XID             *crtcs;
    int             noutput;
    XID             *outputs;
    int             nmode;
    my_XRRModeInfo_t *modes;
} my_XRRScreenResources_t;

typedef struct my_XRRCrtcInfo_s {
    unsigned long   timestamp;
    int             x, y;
    unsigned int    width, height;
    XID             mode;
    uint16_t        rotation;
    int             noutput;
    XID             *outputs;
    uint16_t        rotations;
    int             npossible;
    XID             *possible;
} my_XRRCrtcInfo_t;

typedef struct my_XRROutputInfo_s {
    unsigned long   timestamp;
    XID             crtc;
    char            *name;
    int             nameLen;
    unsigned long   mm_width;
    unsigned long   mm_height;
    uint16_t        connection;
    uint16_t        subpixel_order;
    int             ncrtc;
    XID             *crtcs;
    int             nclone;
    XID             *clones;
    int             nmode;
    int             npreferred;
    XID             *modes;
} my_XRROutputInfo_t;

// Window Attribute
typedef struct my_XWindowAttributes_s {
    int x, y;
    int width, height;
    int border_width;
    int depth;
    void* visual;   //Visual*
    XID root;
    int c_class;
    int bit_gravity;
    int win_gravity;
    int backing_store;
    unsigned long backing_planes;
    unsigned long backing_pixel;
    int save_under;
    XID colormap;
    int map_installed;
    int map_state;
    long all_event_masks;
    long your_event_mask;
    long do_not_propagate_mask;
    int override_redirect;
    void* screen;   //Screen*
} my_XWindowAttributes_t;

typedef struct my_XVisualInfo_s {
  my_Visual_t* visual;
  unsigned long  visualid;
  int screen;
  int depth;
  int c_class;
  unsigned long red_mask;
  unsigned long green_mask;
  unsigned long blue_mask;
  int colormap_size;
  int bits_per_rgb;
} my_XVisualInfo_t;

typedef struct my_XModifierKeymap_s {
    int             max_keypermod;
    uint8_t*        modifiermap;
} my_XModifierKeymap_t;


typedef struct my_XdbeVisualInfo_s
{
    XID         visual;
    int         depth;
    int         perflevel;
} my_XdbeVisualInfo_t;

typedef struct my_XdbeScreenVisualInfo_s
{
    int                     count;
    my_XdbeVisualInfo_t*    visinfo;
} my_XdbeScreenVisualInfo_t;

typedef struct my_XF86VidModeModeInfo_s
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
    int*                tc_private;
} my_XF86VidModeModeInfo_t;

typedef struct my_XColor_s {
	unsigned long pixel;
	unsigned short red, green, blue;
	char flags;
	char pad;
} my_XColor_t;

typedef struct my_XRRProviderInfo_s {
    unsigned int    capabilities;
    int             ncrtcs;
    XID*            crtcs;
    int             noutputs;
    XID*            outputs;
    char*           name;
    int             nassociatedproviders;
    XID*            associated_providers;
    unsigned int*   associated_capability;
    int             nameLen;
} my_XRRProviderInfo_t;

typedef struct my_XRRProviderResources_t {
    unsigned long   timestamp;
    int             nproviders;
    XID*            providers;
} my_XRRProviderResources_t;

typedef struct my_XIAnyClassInfo_s
{
    int         type;
    int         sourceid;
} my_XIAnyClassInfo_t;

typedef struct my_XIDeviceInfo_s
{
    int                     deviceid;
    char*                   name;
    int                     use;
    int                     attachment;
    int                     enabled;
    int                     num_classes;
    my_XIAnyClassInfo_t**   classes;
} my_XIDeviceInfo_t;

typedef struct my_XIEventMask_s {
    int deviceid;
    int mask_len;
    unsigned char* mask;
} my_XIEventMask_t;

typedef struct my_XInputClassInfo_s {
     unsigned char input_class;
     unsigned char event_type_base;
} my_XInputClassInfo_t;

typedef struct my_XDevice_s {
     XID                    device_id;
     int                    num_classes;
     my_XInputClassInfo_t*  classes;
} my_XDevice_t;

typedef struct my_XDeviceKeyEvent_s
{
    int            type;
    unsigned long  serial;
    int            send_event;
    void*          display; //Display*
    XID            window;
    XID            deviceid;
    XID            root;
    XID            subwindow;
    unsigned long  time;
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
} my_XDeviceKeyEvent_t;
typedef my_XDeviceKeyEvent_t my_XDeviceButtonEvent_t;
typedef struct my_XDeviceMotionEvent_s
{
    int            type;
    unsigned long  serial;
    int            send_event;
    void*          display; //Display*
    XID            window;
    XID            deviceid;
    XID            root;
    XID            subwindow;
    unsigned long  time;
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
} my_XDeviceMotionEvent_t;
typedef struct my_XDeviceFocusChangeEvent_s
{
    int           type;
    unsigned long serial;
    int           send_event;
    void*         display;  //Display*
    XID           window;
    XID           deviceid;
    int           mode;
    int           detail;
    unsigned long time;
} my_XDeviceFocusChangeEvent_t;
typedef struct my_XProximityNotifyEvent_s
{
    int            type;
    unsigned long  serial;
    int            send_event;
    void*          display; //Display*
    XID            window;
    XID            deviceid;
    XID            root;
    XID            subwindow;
    unsigned long  time;
    int            x, y;
    int            x_root;
    int            y_root;
    unsigned int   state;
    int            same_screen;
    unsigned int   device_state;
    unsigned char  axes_count;
    unsigned char  first_axis;
    int            axis_data[6];
} my_XProximityNotifyEvent_t;
typedef struct my_XDeviceStateNotifyEvent_s
{
    int           type;
    unsigned long serial;
    int           send_event;
    void*         display;  //Display*
    XID           window;
    XID           deviceid;
    unsigned long time;
    int           num_classes;
    char	      data[64];
} my_XDeviceStateNotifyEvent_t;
typedef struct my_XDeviceMappingEvent_s
{
    int           type;
    unsigned long serial;
    int           send_event;
    void*         display;
    XID           window;
    XID           deviceid;
    unsigned long time;
    int           request;
    int           first_keycode;
    int           count;
} my_XDeviceMappingEvent_t;
typedef struct my_XChangeDeviceNotifyEvent_s
{
    int           type;
    unsigned long serial;
    int           send_event;
    void*         display;
    XID           window;
    XID           deviceid;
    unsigned long time;
    int           request;
} my_XChangeDeviceNotifyEvent_t;
typedef struct my_XDevicePresenceNotifyEvent_s
{
    int           type;
    unsigned long serial;
    int           send_event;
    void*         display;
    XID           window;
    unsigned long time;
    int           devchange;
    XID           deviceid;
    XID           control;
} my_XDevicePresenceNotifyEvent_t;
typedef struct my_XDevicePropertyNotifyEvent_s
{
    int           type;
    unsigned long serial;
    int           send_event;
    void*         display;
    XID           window;
    unsigned long time;
    XID           deviceid;
    XID           atom;
    int           state;
} my_XDevicePropertyNotifyEvent_t;
typedef struct my_XInputClass_s {
     unsigned char class;
     unsigned char length;
} my_XInputClass_t;
typedef struct my_XDeviceState_s {
     XID device_id;
     int num_classes;
     my_XInputClass_t* data;
} my_XDeviceState_t;

typedef struct my_XFixesSelectionNotifyEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    void* display;
    XID window;
    int subtype;
    XID owner;
    XID selection;
    unsigned long timestamp;
    unsigned long selection_timestamp;
} my_XFixesSelectionNotifyEvent_t;
typedef struct my_XFixesCursorNotifyEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    void* display;
    XID window;
    int subtype;
    unsigned long cursor_serial;
    unsigned long timestamp;
    XID cursor_name;
} my_XFixesCursorNotifyEvent_t;

typedef struct my_XRRScreenChangeNotifyEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    void* display;
    XID window;
    XID root;
    unsigned long timestamp;
    unsigned long config_timestamp;
    uint16_t size_index;
    uint16_t subpixel_order;
    uint16_t rotation;
    int width;
    int height;
    int mwidth;
    int mheight;
} my_XRRScreenChangeNotifyEvent_t;
typedef struct my_XRRNotifyEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    void* display;
    XID window;
    int subtype;
} my_XRRNotifyEvent_t;
typedef struct my_XRROutputChangeNotifyEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    void* display;
    XID window;
    int subtype;
    XID output;
    XID crtc;
    XID mode;
    uint16_t rotation;
    uint16_t connection;
    uint16_t subpixel_order;
} my_XRROutputChangeNotifyEvent_t;
typedef struct my_XRRCrtcChangeNotifyEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    void* display;
    XID window;
    int subtype;
    XID crtc;
    XID mode;
    uint16_t rotation;
    int x, y;
    unsigned int width, height;
} my_XRRCrtcChangeNotifyEvent_t;
typedef struct my_XRROutputPropertyNotifyEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    void* display;
    XID window;
    int subtype;
    XID output;
    XID property;
    unsigned long timestamp;
    int state;
} my_XRROutputPropertyNotifyEvent_t;
typedef struct my_XRRProviderChangeNotifyEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    void* display;
    XID window;
    int subtype;
    XID provider;
    unsigned long timestamp;
    unsigned int current_role;
} my_XRRProviderChangeNotifyEvent_t;
typedef struct my_XRRProviderPropertyNotifyEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    void* display;
    XID window;
    int subtype;
    XID provider;
    XID property;
    unsigned long timestamp;
    int state;
} my_XRRProviderPropertyNotifyEvent_t;
typedef struct my_XRRResourceChangeNotifyEvent_s
{
    int type;
    unsigned long serial;
    int send_event;
    void* display;
    XID window;
    int subtype;
    unsigned long timestamp;
} my_XRRResourceChangeNotifyEvent_t;

typedef struct my_XcursorCursors_s {
    void*      dpy;     //Display*
    int        ref;
    int        ncursor;
    void*      cursors; //Cursor*
} my_XcursorCursors_t;

typedef struct my_XcursorImage_s {
    uint32_t        version;
    uint32_t        size;
    uint32_t        width;
    uint32_t        height;
    uint32_t        xhot;
    uint32_t        yhot;
    uint32_t        delay;
    void*           pixels;
} my_XcursorImage_t;

typedef struct my_XcursorImages_s {
    int                 nimage;
    my_XcursorImage_t** images;
    char*               name;
} my_XcursorImages_t;

typedef struct my_XExtCodes_s {
    int extension;
    int major_opcode;
    int first_event;
    int first_error;
} my_XExtCodes_t;

typedef struct my_XExtDisplayInfo_s {
    struct my_XExtDisplayInfo_s *next;
    void*   display;    //Didsplay*
    my_XExtCodes_t *codes;
    void* data;
} my_XExtDisplayInfo_t;

typedef struct my_XExtensionInfo_s {
    my_XExtDisplayInfo_t *head;
    my_XExtDisplayInfo_t *cur;
    int ndisplays;
} my_XExtensionInfo_t;

typedef struct my_XCharStruct_s {
    short       lbearing;
    short       rbearing;
    short       width;
    short       ascent;
    short       descent;
    unsigned short attributes;
} my_XCharStruct_t;

typedef struct my_XFontProp_s {
    XID name;
    unsigned long card32;
} my_XFontProp_t;

typedef struct my_XFontStruct_s {
    void*             ext_data; //XExtData*
    XID               fid;
    unsigned          direction;
    unsigned          min_char_or_byte2;
    unsigned          max_char_or_byte2;
    unsigned          min_byte1;
    unsigned          max_byte1;
    int               all_chars_exist;
    unsigned          default_char;
    int               n_properties;
    my_XFontProp_t*   properties;
    my_XCharStruct_t  min_bounds;
    my_XCharStruct_t  max_bounds;
    my_XCharStruct_t* per_char;
    int               ascent;
    int               descent;
} my_XFontStruct_t;

typedef struct my_XExtensionHooks_s {
    int (*create_gc)(void* dpy, void* gc, my_XExtCodes_t* e);
    int (*copy_gc)(void* dpy, void* gc, my_XExtCodes_t* e);
    int (*flush_gc)(void* dpy, void* gc, my_XExtCodes_t* e);
    int (*free_gc)(void* dpy, void* gc, my_XExtCodes_t* e);
    int (*create_font)(void* dpy, my_XFontStruct_t* f, my_XExtCodes_t* e);
    int (*free_font)(void* dpy, my_XFontStruct_t* f, my_XExtCodes_t* e);
    int (*close_display)(void* dpy, my_XExtCodes_t* e);
    int (*wire_to_event)(void* dpy, my_XEvent_t* evt, void* xEvent);
    int (*event_to_wire)(void* dpy, my_XEvent_t* evt, void* xEvent);
    int (*error)(void* dpy, void* xError, my_XExtCodes_t* e, int* n);
    char*(*error_string)(void* dpy, int, my_XExtCodes_t* e, char* s, int n);
} my_XExtensionHooks_t;

typedef struct my_XRRPropertyInfo_s {
    int     pending;
    int     range;
    int     immutable;
    int     num_values;
    long*   values;
} my_XRRPropertyInfo_t;

typedef struct my_XShmSegmentInfo_s {
    XID     shmseg;
    int     shmid;
    char*   shmaddr;
    int     readOnly;
} my_XShmSegmentInfo_t;

typedef struct my_XkbNamesRec_s {
    XID             keycodes;
    XID             geometry;
    XID             symbols;
    XID             types;
    XID             compat;
    XID             vmods[16];
    XID             indicators[32];
    XID             groups[4];
    void*           keys; //XkbKeyNameRec* as array[xkb->max_key_code + 1]
    void*           key_aliases; //XkbKeyAliasRec* as array[num_key_aliases]
    XID*            radio_groups;
    XID             phys_symbols;
    unsigned char   num_keys;
    unsigned char   num_key_aliases;
    unsigned short  num_rg;
} my_XkbNamesRec_t;

typedef struct my_XkbClientMapRec_s {
    unsigned char            size_types;
    unsigned char            num_types;
    void*                    types; //XkbKeyTypePtr
    unsigned short           size_syms;
    unsigned short           num_syms;
    void*                    syms;  //KeySym*
    void*                    key_sym_map;   //XkbSymMapPtr
    unsigned char*           modmap;
} my_XkbClientMapRec_t;

typedef struct my_XkbDescRec_s {
   my_XDisplay_t*           display;
   unsigned short           flags;
   unsigned short           device_spec;
   uint8_t                  min_key_code;
   uint8_t                  max_key_code;
   void*                    ctrls; //XkbControlsPtr
   void*                    server; //XkbServerMapPtr
   my_XkbClientMapRec_t*    map; //XkbClientMapPtr
   void*                    indicators; //XkbIndicatorPtr
   my_XkbNamesRec_t*        names;
   void*                    compat; //XkbCompatMapPtr
   void*                    geom; //XkbGeometryPtr
} my_XkbDescRec_t;

typedef struct my_XmbTextItem_s {
    void*       chars;  //char*
    int         nchars;
    int         delta;
    void*       font_set;   //XFontSet
} my_XmbTextItem_t;

typedef struct my_XwcTextItem_s {
    void*           chars;  //wchar_t*
    int             nchars;
    int             delta;
    void*           font_set;   //XFontSet
} my_XwcTextItem_t;

typedef struct my_XAnyClassinfo_s {
    XID         c_class;
    int         length;
} my_XAnyClassInfo_t;

typedef struct my_XKeyInfo_s
{
    XID                 c_class;
    int                 length;
    unsigned short      min_keycode;
    unsigned short      max_keycode;
    unsigned short      num_keys;
} my_XKeyInfo_t;
typedef struct my_XButtonInfo_s
{
    XID         c_class;
    int         length;
    short       num_buttons;
} my_XButtonInfo_t;
typedef struct  my_XValuatorInfo_s
{
    XID                 c_class;
    int                 length;
    unsigned char       num_axes;
    unsigned char       mode;
    unsigned long       motion_buffer;
    void*               axes;   //XAxisInfoPtr
} my_XValuatorInfo_t;

typedef struct my_XDeviceInfo_s
{
    XID                 id;        
    XID                 type;
    char*               name;
    int                 num_classes;
    int                 use;
    my_XAnyClassInfo_t* inputclassinfo;
} my_XDeviceInfo_t;

typedef struct my_XTimeCoord_s {
	unsigned long time;
	short x, y;
} my_XTimeCoord_t;

typedef struct my_XDeviceTimeCoord_s {
    unsigned long   time;
    int*            data;
} my_XDeviceTimeCoord_t;

typedef struct my_XFixesCursorImage_s {
    short           x, y;
    unsigned short  width, height;
    unsigned short  xhot, yhot;
    unsigned long   cursor_serial;
    unsigned long*  pixels;
    XID             atom;                   /* Version >= 2 only */
    void*           name;                  /* Version >= 2 only */
} my_XFixesCursorImage_t;

typedef struct my_XFilters_s {
    int nfilter;
    char **filter;
    int nalias;
    short *alias;
} my_XFilters_t;

typedef struct my__XRRMonitorInfo_s {
    XID     name;
    int     primary;
    int     automatic;
    int     noutput;
    int     x;
    int     y;
    int     width;
    int     height;
    int     mwidth;
    int     mheight;
    XID*    outputs;
} my_XRRMonitorInfo_t;

typedef struct my_XkbAnyEvent_s {
    int             type;
    unsigned long   serial;
    int             send_event;
    my_XDisplay_t*  display;
    unsigned long   time;
    int             xkb_type;
    unsigned int    device;
} my_XkbAnyEvent_t;

typedef struct my_XkbNewKeyboardNotify_s {
    int             type;
    unsigned long   serial;
    int             send_event;
    my_XDisplay_t*  display;
    unsigned long   time;
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
} my_XkbNewKeyboardNotifyEvent_t;

typedef struct my_XkbMapNotifyEvent_s {
    int             type;
    unsigned long   serial;
    int             send_event;
    my_XDisplay_t*  display;
    unsigned long   time;
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
} my_XkbMapNotifyEvent_t;

typedef struct my_XkbStateNotifyEvent_s {
    int             type;
    unsigned long   serial;
    int             send_event;
    my_XDisplay_t*  display;
    unsigned long   time;
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
} my_XkbStateNotifyEvent_t;

typedef struct my_XkbControlsNotify_s {
    int             type;
    unsigned long   serial;
    int             send_event;
    my_XDisplay_t*  display;
    unsigned long   time;
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
} my_XkbControlsNotifyEvent_t;

typedef struct my_XkbIndicatorNotify_s {
    int             type;
    unsigned long   serial;
    int             send_event;
    my_XDisplay_t*  display;
    unsigned long   time;
    int             xkb_type;
    int             device;
    unsigned int    changed;
    unsigned int    state;
} my_XkbIndicatorNotifyEvent_t;

typedef struct my_XkbNamesNotify_s {
    int             type;
    unsigned long   serial;
    int             send_event;
    my_XDisplay_t*  display;
    unsigned long   time;
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
} my_XkbNamesNotifyEvent_t;

typedef struct my_XkbCompatMapNotify_s {
    int             type;
    unsigned long   serial;
    int             send_event;
    my_XDisplay_t*  display;
    unsigned long   time;
    int             xkb_type;
    int             device;
    unsigned int    changed_groups;
    int             first_si;
    int             num_si;
    int             num_total_si;
} my_XkbCompatMapNotifyEvent_t;

typedef struct my_XkbBellNotify_s {
    int             type;
    unsigned long   serial;
    int             send_event;
    my_XDisplay_t*  display;
    unsigned long   time;
    int             xkb_type;
    int             device;
    int             percent;
    int             pitch;
    int             duration;
    int             bell_class;
    int             bell_id;
    XID             name;
    XID             window;
    int            event_only;
} my_XkbBellNotifyEvent_t;

typedef struct my_XkbActionMessage_s {
    int             type;
    unsigned long   serial;
    int             send_event;
    my_XDisplay_t*  display;
    unsigned long   time;
    int             xkb_type;
    int             device;
    uint8_t         keycode;
    int             press;
    int             key_event_follows;
    int             group;
    unsigned int    mods;
    char            message[6+1];
} my_XkbActionMessageEvent_t;

typedef struct my_XkbAccessXNotify_s {
    int             type;
    unsigned long   serial;
    int             send_event;
    my_XDisplay_t*  display;
    unsigned long   time;
    int             xkb_type;
    int             device;
    int             detail;
    int             keycode;
    int             sk_delay;
    int             debounce_delay;
} my_XkbAccessXNotifyEvent_t;

typedef struct my_XkbExtensionDeviceNotify_s {
    int             type;
    unsigned long   serial;
    int             send_event;
    my_XDisplay_t*  display;
    unsigned long   time;
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
} my_XkbExtensionDeviceNotifyEvent_t;

typedef union my_XkbEvent_s {
    int                                 type;
    my_XkbAnyEvent_t                    any;
    my_XkbNewKeyboardNotifyEvent_t      new_kbd;
    my_XkbMapNotifyEvent_t              map;
    my_XkbStateNotifyEvent_t            state;
    my_XkbControlsNotifyEvent_t         ctrls;
    my_XkbIndicatorNotifyEvent_t        indicators;
    my_XkbNamesNotifyEvent_t            names;
    my_XkbCompatMapNotifyEvent_t        compat;
    my_XkbBellNotifyEvent_t             bell;
    my_XkbActionMessageEvent_t          message;
    my_XkbAccessXNotifyEvent_t          accessx;
    my_XkbExtensionDeviceNotifyEvent_t  device;
    my_XEvent_t                         core;
} my_XkbEvent_t;

typedef struct my_XIEvent_s {
    int           type;
    unsigned long serial;
    int           send_event;
    my_XDisplay_t*display;
    int           extension;
    int           evtype;
    unsigned long time;
} my_XIEvent_t;

typedef struct my_XIHierarchyInfo_s {
    int           deviceid;
    int           attachment;
    int           use;
    int           enabled;
    int           flags;
} my_XIHierarchyInfo_t;

typedef struct my_XIButtonState_s {
    int           mask_len;
    unsigned char *mask;
} my_XIButtonState_t;

typedef struct my_XIValuatorState_s {
    int           mask_len;
    unsigned char *mask;
    double        *values;
} my_XIValuatorState_t;

typedef struct my_XIModifierState_s {
    int    base;
    int    latched;
    int    locked;
    int    effective;
} my_XIModifierState_t;

typedef struct my_XIHierarchyEvent_s {
    int                     type;
    unsigned long           serial;
    int                     send_event;
    my_XDisplay_t*          display;
    int                     extension;
    int                     evtype;
    unsigned long           time;
    int                     flags;
    int                     num_info;
    my_XIHierarchyInfo_t*   info;
} my_XIHierarchyEvent_t;

typedef struct my_XIDeviceChangedEvent_s {
    int                     type;
    unsigned long           serial;
    int                     send_event;
    my_XDisplay_t*          display;
    int                     extension;
    int                     evtype;
    unsigned long           time;
    int                     deviceid;
    int                     sourceid;
    int                     reason;
    int                     num_classes;
    my_XIAnyClassInfo_t**   classes;
} my_XIDeviceChangedEvent_t;

typedef struct my_XIDeviceEvent_s {
    int                     type;
    unsigned long           serial;
    int                     send_event;
    my_XDisplay_t*          display;
    int                     extension;
    int                     evtype;
    unsigned long           time;
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
    int                     flags;
    my_XIButtonState_t      buttons;
    my_XIValuatorState_t    valuators;
    my_XIModifierState_t    mods;
    my_XIModifierState_t    group;
} my_XIDeviceEvent_t;

typedef struct my_XIRawEvent_s {
    int                     type;
    unsigned long           serial;
    int                     send_event;
    my_XDisplay_t*          display;
    int                     extension;
    int                     evtype;
    unsigned long           time;
    int                     deviceid;
    int                     sourceid;
    int                     detail;
    int                     flags;
    my_XIValuatorState_t    valuators;
    double*                 raw_values;
} my_XIRawEvent_t;

typedef struct my_XIEnterEvent_s {
    int                     type;
    unsigned long           serial;
    int                     send_event;
    my_XDisplay_t*          display;
    int                     extension;
    int                     evtype;
    unsigned long           time;
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
    my_XIButtonState_t      buttons;
    my_XIModifierState_t    mods;
    my_XIModifierState_t    group;
} my_XIEnterEvent_t;
// Leave, FocusIn and FocusOut are like Enter

typedef struct my_XIPropertyEvent_s {
    int           type;
    unsigned long serial;
    int           send_event;
    my_XDisplay_t*display;
    int           extension;
    int           evtype;
    unsigned long time;
    int           deviceid;
    XID           property;
    int           what;
} my_XIPropertyEvent_t;

typedef struct my_XITouchOwnershipEvent_s {
    int           type;
    unsigned long serial;
    int           send_event;
    my_XDisplay_t*display;
    int           extension;
    int           evtype;
    unsigned long time;
    int           deviceid;
    int           sourceid;
    unsigned int  touchid;
    XID           root;
    XID           event;
    XID           child;
    int           flags;
} my_XITouchOwnershipEvent_t;

typedef struct my_XIBarrierEvent_s {
    int           type;
    unsigned long serial;
    int           send_event;
    my_XDisplay_t*display;
    int           extension;
    int           evtype;
    unsigned long time;
    int           deviceid;
    int           sourceid;
    XID           event;
    XID           root;
    double        root_x;
    double        root_y;
    double        dx;
    double        dy;
    int           dtime;
    int           flags;
    XID           barrier;
    uint32_t      eventid;
} my_XIBarrierEvent_t;

typedef struct my_XIGesturePinchEvent_s {
    int                     type;
    unsigned long           serial;
    int                     send_event;
    my_XDisplay_t*          display;
    int                     extension;
    int                     evtype;
    unsigned long           time;
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
    double                  delta_x;
    double                  delta_y;
    double                  delta_unaccel_x;
    double                  delta_unaccel_y;
    double                  scale;
    double                  delta_angle;
    int                     flags;
    my_XIModifierState_t    mods;
    my_XIModifierState_t    group;
} my_XIGesturePinchEvent_t;

typedef struct my_XIGestureSwipeEvent_s {
    int                     type;
    unsigned long           serial;
    int                     send_event;
    my_XDisplay_t*          display;
    int                     extension;
    int                     evtype;
    unsigned long           time;
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
    double                  delta_x;
    double                  delta_y;
    double                  delta_unaccel_x;
    double                  delta_unaccel_y;
    int                     flags;
    my_XIModifierState_t    mods;
    my_XIModifierState_t    group;
} my_XIGestureSwipeEvent_t;

#define XI_DeviceChanged                 1
#define XI_KeyPress                      2
#define XI_KeyRelease                    3
#define XI_ButtonPress                   4
#define XI_ButtonRelease                 5
#define XI_Motion                        6
#define XI_Enter                         7
#define XI_Leave                         8
#define XI_FocusIn                       9
#define XI_FocusOut                      10
#define XI_HierarchyChanged              11
#define XI_PropertyEvent                 12
#define XI_RawKeyPress                   13
#define XI_RawKeyRelease                 14
#define XI_RawButtonPress                15
#define XI_RawButtonRelease              16
#define XI_RawMotion                     17
#define XI_TouchBegin                    18 /* XI 2.2 */
#define XI_TouchUpdate                   19
#define XI_TouchEnd                      20
#define XI_TouchOwnership                21
#define XI_RawTouchBegin                 22
#define XI_RawTouchUpdate                23
#define XI_RawTouchEnd                   24
#define XI_BarrierHit                    25 /* XI 2.3 */
#define XI_BarrierLeave                  26
#define XI_GesturePinchBegin             27 /* XI 2.4 */
#define XI_GesturePinchUpdate            28
#define XI_GesturePinchEnd               29
#define XI_GestureSwipeBegin             30
#define XI_GestureSwipeUpdate            31
#define XI_GestureSwipeEnd               32

#endif//MY_X11_DEFS