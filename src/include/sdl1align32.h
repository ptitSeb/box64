#ifndef __MY_SDL1ALIGN32_H_
#define __MY_SDL1ALIGN32_H_
#include <stdint.h>
#include "box32.h"

typedef struct my_SDL_Rect_s
{
    int16_t  x;
    int16_t  y;
    uint16_t w;
    uint16_t h;
} my_SDL_Rect_t;

typedef struct my_SDL_Color_s
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t unused;
} my_SDL_Color_t;

typedef struct my_SDL_Palette_s
{
    int ncolors;
    my_SDL_Color_t *colors;
} my_SDL_Palette_t;

typedef struct my_SDL_PixelFormat_s
{
    my_SDL_Palette_t *palette;
    uint8_t BitsPerPixel;
    uint8_t BytesPerPixel;
    uint8_t Rloss;
    uint8_t Gloss;
    uint8_t Bloss;
    uint8_t Aloss;
    uint8_t Rshift;
    uint8_t Gshift;
    uint8_t Bshift;
    uint8_t Ashift;
    uint32_t Rmask;
    uint32_t Gmask;
    uint32_t Bmask;
    uint32_t Amask;
    uint32_t colorkey;
    uint8_t alpha;
} my_SDL_PixelFormat_t;

typedef struct my_SDL_Surface_s
{
    uint32_t flags;
    my_SDL_PixelFormat_t *format;
    int w;
    int h;
    uint16_t pitch;
    void *pixels;
    int offset;
    void* hwdata; //struct private_hwdata
    my_SDL_Rect_t clip_rect;
    uint32_t unused1;
    uint32_t locked;
    void *map;
    unsigned int format_version;
    int refcount;
} my_SDL_Surface_t;

// x86 version (packed, 32bits pointers and long)

typedef struct my_SDL_Rect_32_s
{
    int16_t  x;
    int16_t  y;
    uint16_t w;
    uint16_t h;
} my_SDL_Rect_32_t;

typedef struct my_SDL_Color_32_s
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t unused;
} my_SDL_Color_32_t;

typedef struct my_SDL_Palette_32_s
{
    int ncolors;
    ptr_t colors;   // my_SDL_Color_t*
} my_SDL_Palette_32_t;

typedef struct my_SDL_PixelFormat_32_s
{
    ptr_t palette;  // my_SDL_Palette_t *
    uint8_t BitsPerPixel;
    uint8_t BytesPerPixel;
    uint8_t Rloss;
    uint8_t Gloss;
    uint8_t Bloss;
    uint8_t Aloss;
    uint8_t Rshift;
    uint8_t Gshift;
    uint8_t Bshift;
    uint8_t Ashift;
    uint32_t Rmask;
    uint32_t Gmask;
    uint32_t Bmask;
    uint32_t Amask;
    uint32_t colorkey;
    uint8_t alpha;
} my_SDL_PixelFormat_32_t;

typedef struct my_SDL_Surface_32_s
{
    uint32_t flags;
    ptr_t format;   // my_SDL_PixelFormat_t *
    int w;
    int h;
    uint16_t pitch;
    uint16_t dummy;
    ptr_t pixels;   // void *
    int offset;
    ptr_t hwdata; //struct private_hwdata*
    my_SDL_Rect_t clip_rect;
    uint32_t unused1;
    uint32_t locked;
    ptr_t map;  // void *
    unsigned int format_version;
    int refcount;
} my_SDL_Surface_32_t;

void inplace_SDL_Surface_to_64(void* a);
void inplace_SDL_Surface_to_32(void* a);
void inplace_SDL_PixelFormat_to_64(void* a);
void inplace_SDL_PixelFormat_to_64_nopalette(void* a);
void inplace_SDL_PixelFormat_to_32(void* a);
void inplace_SDL_Palette_to_64(void* a);
void inplace_SDL_Palette_to_32(void* a);

typedef struct my_SDL_keysym_s
{
    uint8_t scancode;
    int sym;
    int mod;
    uint16_t unicode;
} my_SDL_keysym_t;

typedef struct my_SDL_keysym_32_s
{
    uint8_t scancode;
    int sym;
    int mod;
    uint16_t unicode;
} my_SDL_keysym_32_t;


typedef enum my_SDL_EventType_s
{
    SDL_NOEVENT,
    SDL_ACTIVEEVENT,
    SDL_KEYDOWN,
    SDL_KEYUP,
    SDL_MOUSEMOTION,
    SDL_MOUSEBUTTONDOWN,
    SDL_MOUSEBUTTONUP,
    SDL_JOYAXISMOTION,
    SDL_JOYBALLMOTION,
    SDL_JOYHATMOTION,
    SDL_JOYBUTTONDOWN,
    SDL_JOYBUTTONUP,
    SDL_QUIT,
    SDL_SYSWMEVENT,
    SDL_EVENT_RESERVEDA,
    SDL_EVENT_RESERVEDB,
    SDL_VIDEORESIZE,
    SDL_VIDEOEXPOSE,
    SDL_EVENT_RESERVED2,
    SDL_EVENT_RESERVED3,
    SDL_EVENT_RESERVED4,
    SDL_EVENT_RESERVED5,
    SDL_EVENT_RESERVED6,
    SDL_EVENT_RESERVED7,
    SDL_USEREVENT = 24,
    SDL_NUMEVENTS = 32
} my_SDL_EventType_t;

typedef struct my_SDL_ActiveEvent_s
{
    uint8_t type;
    uint8_t gain;
    uint8_t state;
} my_SDL_ActiveEvent_t;

typedef struct my_SDL_KeyboardEvent_s
{
    uint8_t type;
    uint8_t which;
    uint8_t state;
    my_SDL_keysym_t keysym;
} my_SDL_KeyboardEvent_t;

typedef struct my_SDL_MouseMotionEvent_s
{
    uint8_t type;
    uint8_t which;
    uint8_t state;
    uint16_t x;
    uint16_t y;
    int16_t xrel;
    int16_t yrel;
} my_SDL_MouseMotionEvent_t;

typedef struct my_SDL_MouseButtonEvent_s
{
    uint8_t type;
    uint8_t which;
    uint8_t button;
    uint8_t state;
    uint16_t x;
    uint16_t y;
} my_SDL_MouseButtonEvent_t;

typedef struct my_SDL_JoyAxisEvent_s
{
    uint8_t type;
    uint8_t which;
    uint8_t axis;
    int16_t value;
} my_SDL_JoyAxisEvent_t;

typedef struct my_SDL_JoyBallEvent_s
{
    uint8_t type;
    uint8_t which;
    uint8_t ball;
    int16_t xrel;
    int16_t yrel;
} my_SDL_JoyBallEvent_t;

typedef struct my_SDL_JoyHatEvent_s
{
    uint8_t type;
    uint8_t which;
    uint8_t hat;
    uint8_t value;
} my_SDL_JoyHatEvent_t;

typedef struct my_SDL_JoyButtonEvent_s
{
    uint8_t type;
    uint8_t which;
    uint8_t button;
    uint8_t state;
} my_SDL_JoyButtonEvent_t;

typedef struct my_SDL_ResizeEvent_s
{
    uint8_t type;
    int w;
    int h;
} my_SDL_ResizeEvent_t;

typedef struct my_SDL_ExposeEvent_s
{
    uint8_t type;
} my_SDL_ExposeEvent_t;

typedef struct my_SDL_QuitEvent_s
{
    uint8_t type;
} my_SDL_QuitEvent_t;

typedef struct my_SDL_UserEvent_s
{
    uint8_t type;
    int code;
    void *data1;
    void *data2;
} my_SDL_UserEvent_t;

typedef struct my_SDL_version_s {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} my_SDL_version_t;

typedef struct my_SDL_SysWMinfo_s {
  my_SDL_version_t version;
  int subsystem;
  union {
    struct {
      void* display;
      void* window;
      void (*lock_func)(void);
      void (*unlock_func)(void);
      void* fswindow;
      void* wmwindow;
      void* gfxdisplay;
    } x11;
  } info;
} my_SDL_SysWMinfo_t;

typedef union my_XEvent_s {
    int Type;
	long pad[24];
} my_XEvent_t;

typedef struct my_SDL_SysWMmsg_s
{
    my_SDL_version_t version;
    int subsystem;
    union {
        my_XEvent_t xevent;
    } event;
} my_SDL_SysWMmsg_t;


typedef struct my_SDL_SysWMEvent_s
{
    uint8_t type;
    my_SDL_SysWMmsg_t *msg;
} my_SDL_SysWMEvent_t;

typedef union my_SDL_Event_s
{
    uint8_t type;
    my_SDL_ActiveEvent_t active;
    my_SDL_KeyboardEvent_t key;
    my_SDL_MouseMotionEvent_t motion;
    my_SDL_MouseButtonEvent_t button;
    my_SDL_JoyAxisEvent_t jaxis;
    my_SDL_JoyBallEvent_t jball;
    my_SDL_JoyHatEvent_t jhat;
    my_SDL_JoyButtonEvent_t jbutton;
    my_SDL_ResizeEvent_t resize;
    my_SDL_ExposeEvent_t expose;
    my_SDL_QuitEvent_t quit;
    my_SDL_UserEvent_t user;
    my_SDL_SysWMEvent_t syswm;
} my_SDL_Event_t;

typedef struct my_SDL_ActiveEvent_32_s
{
    uint8_t type;
    uint8_t gain;
    uint8_t state;
} my_SDL_ActiveEvent_32_t;

typedef struct my_SDL_KeyboardEvent_32_s
{
    uint8_t type;
    uint8_t which;
    uint8_t state;
    my_SDL_keysym_32_t keysym;
} my_SDL_KeyboardEvent_32_t;

typedef struct my_SDL_MouseMotionEvent_32_s
{
    uint8_t type;
    uint8_t which;
    uint8_t state;
    uint16_t x;
    uint16_t y;
    int16_t xrel;
    int16_t yrel;
} my_SDL_MouseMotionEvent_32_t;

typedef struct my_SDL_MouseButtonEvent_32_s
{
    uint8_t type;
    uint8_t which;
    uint8_t button;
    uint8_t state;
    uint16_t x;
    uint16_t y;
} my_SDL_MouseButtonEvent_32_t;

typedef struct my_SDL_JoyAxisEvent_32_s
{
    uint8_t type;
    uint8_t which;
    uint8_t axis;
    int16_t value;
} my_SDL_JoyAxisEvent_32_t;

typedef struct my_SDL_JoyBallEvent_32_s
{
    uint8_t type;
    uint8_t which;
    uint8_t ball;
    int16_t xrel;
    int16_t yrel;
} my_SDL_JoyBallEvent_32_t;

typedef struct my_SDL_JoyHatEvent_32_s
{
    uint8_t type;
    uint8_t which;
    uint8_t hat;
    uint8_t value;
} my_SDL_JoyHatEvent_32_t;

typedef struct my_SDL_JoyButtonEvent_32_s
{
    uint8_t type;
    uint8_t which;
    uint8_t button;
    uint8_t state;
} my_SDL_JoyButtonEvent_32_t;

typedef struct my_SDL_ResizeEvent_32_s
{
    uint8_t type;
    int w;
    int h;
} my_SDL_ResizeEvent_32_t;

typedef struct my_SDL_ExposeEvent_32_s
{
    uint8_t type;
} my_SDL_ExposeEvent_32_t;

typedef struct my_SDL_QuitEvent_32_s
{
    uint8_t type;
} my_SDL_QuitEvent_32_t;

typedef struct my_SDL_UserEvent_32_s
{
    uint8_t type;
    int code;
    ptr_t data1;    //void*
    ptr_t data2;    //void*
} my_SDL_UserEvent_32_t;

typedef struct my_SDL_version_32_s {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} my_SDL_version_32_t;

typedef struct my_SDL_SysWMinfo_32_s {
  my_SDL_version_32_t version;
  int subsystem;
  union {
    struct {
      ptr_t display;    //void*
      ptr_t window; //void*
      ptr_t lock_func;//void (*lock_func)(void);
      ptr_t unlock_func;//void (*unlock_func)(void);
      ptr_t fswindow;   //void*
      ptr_t wmwindow;   //void*
      ptr_t gfxdisplay; //void*
    } x11;
  } info;
} my_SDL_SysWMinfo_32_t;

typedef union my_XEvent_32_s {
    int Type;
	long_t pad[24];
} my_XEvent_32_t;

typedef struct my_SDL_SysWMmsg_32_s
{
    my_SDL_version_32_t version;
    int subsystem;
    union {
        my_XEvent_32_t xevent;
    } event;
} my_SDL_SysWMmsg_32_t;


typedef struct my_SDL_SysWMEvent_32_s
{
    uint8_t type;
    ptr_t msg; //my_SDL_SysWMmsg_t*
} my_SDL_SysWMEvent_32_t;

typedef union my_SDL_Event_32_s
{
    uint8_t type;
    my_SDL_ActiveEvent_32_t active;
    my_SDL_KeyboardEvent_32_t key;
    my_SDL_MouseMotionEvent_32_t motion;
    my_SDL_MouseButtonEvent_32_t button;
    my_SDL_JoyAxisEvent_32_t jaxis;
    my_SDL_JoyBallEvent_32_t jball;
    my_SDL_JoyHatEvent_32_t jhat;
    my_SDL_JoyButtonEvent_32_t jbutton;
    my_SDL_ResizeEvent_32_t resize;
    my_SDL_ExposeEvent_32_t expose;
    my_SDL_QuitEvent_32_t quit;
    my_SDL_UserEvent_32_t user;
    my_SDL_SysWMEvent_32_t syswm;
} my_SDL_Event_32_t;


void convert_SDL_Event_to_32(void* dst, const void* src);
void convert_SDL_Event_to_64(void* dst, const void* src);

// simplified RWops
typedef struct my_SDL_RWops_s {
    void*  seek;    //sdl1_seek
    void*  read;    //sdl1_read
    void* write;    //sdl1_write
    void* close;    //sdl1_close
    uint32_t type;
    void* hidden[3]; // not converting hidden, just moving it
} my_SDL_RWops_t;

typedef struct my_SDL_RWops_32_s {
    ptr_t  seek;    //sdl1_seek
    ptr_t  read;    //sdl1_read
    ptr_t write;    //sdl1_write
    ptr_t close;    //sdl1_close
    uint32_t type;
    void* hidden[3]; // not converting hidden, just moving it
} my_SDL_RWops_32_t;

void inplace_SDL_RWops_to_64(void* a);
void inplace_SDL_RWops_to_32(void* a);

typedef struct my_SDL_VideoInfo_s {
  uint32_t hw_available:1;
  uint32_t wm_available:1;
  uint32_t UnusedBits1:6;
  uint32_t UnusedBits2:1;
  uint32_t blit_hw:1;
  uint32_t blit_hw_CC:1;
  uint32_t blit_hw_A:1;
  uint32_t blit_sw:1;
  uint32_t blit_sw_CC:1;
  uint32_t blit_sw_A:1;
  uint32_t blit_fill:1;
  uint32_t UnusedBits3:16;
  uint32_t video_mem;
  my_SDL_PixelFormat_t *vfmt;
  int current_w;
  int current_h;
} my_SDL_VideoInfo_t;

typedef struct my_SDL_VideoInfo_32_s {
  uint32_t hw_available:1;
  uint32_t wm_available:1;
  uint32_t UnusedBits1:6;
  uint32_t UnusedBits2:1;
  uint32_t blit_hw:1;
  uint32_t blit_hw_CC:1;
  uint32_t blit_hw_A:1;
  uint32_t blit_sw:1;
  uint32_t blit_sw_CC:1;
  uint32_t blit_sw_A:1;
  uint32_t blit_fill:1;
  uint32_t UnusedBits3:16;
  uint32_t video_mem;
  ptr_t vfmt;   // my_SDL_PixelFormat_t *
  int current_w;
  int current_h;
} my_SDL_VideoInfo_32_t;

typedef struct my_SDL_AudioCVT_s {
  int needed;
  uint16_t src_format;
  uint16_t dest_format;
  double rate_incr;
  uint8_t *buf;
  int len;
  int len_cvt;
  int len_mult;
  double len_ratio;
  void (*filters[10])(struct my_SDL_AudioCVT_s *cvt, uint16_t format);
  int filter_index;
} my_SDL_AudioCVT_t;

typedef struct my_SDL_AudioCVT_32_s {
  int needed;
  uint16_t src_format;
  uint16_t dest_format;
  double rate_incr;
  ptr_t buf;    //uint8_t *
  int len;
  int len_cvt;
  int len_mult;
  double len_ratio;
  ptr_t filters[10]; //void (*filters[10])(struct my_SDL_AudioCVT_s *cvt, uint16_t format);
  int filter_index;
} my_SDL_AudioCVT_32_t;

void convert_AudioCVT_to_32(void* d, void* s);
void convert_AudioCVT_to_64(void* d, void* s);

#endif//__MY_SDL1ALIGN32_H_