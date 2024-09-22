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

typedef struct my_Screen_s {
        void* ext_data;         //XExtData *
        struct my_XDisplay_s *display;
        XID root;
        int width, height;
        int mwidth, mheight;
        int ndepths;
        void* depths;   //Depth *
        int root_depth;         /* bits per pixel */
        void* root_visual;  //Visual *
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
        XID resourceid;
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


#endif//MY_X11_DEFS