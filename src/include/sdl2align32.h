#ifndef __MY_SDL2ALIGN32_H_
#define __MY_SDL2ALIGN32_H_
#include <stdint.h>
#include "box32.h"

typedef struct my_SDL2_DisplayMode_s {
    uint32_t format;
    int w;
    int h;
    int refresh_rate;
    void* driverdata;
} my_SDL2_DisplayMode_t;

typedef struct my_SDL2_DisplayMode_32_s {
    uint32_t format;
    int w;
    int h;
    int refresh_rate;
    ptr_t driverdata;
} my_SDL2_DisplayMode_32_t;

typedef struct SDL2_CommonEvent_s {
    uint32_t type;
    uint32_t timestamp;
} my_SDL2_CommonEvent_t;

typedef struct SDL2_DisplayEvent_s {
    uint32_t type;
    uint32_t timestamp;
    uint32_t display;
    uint8_t event;
    uint8_t padding1;
    uint8_t padding2;
    uint8_t padding3;
    int32_t data1;
} my_SDL2_DisplayEvent_t;

typedef struct SDL2_WindowEvent_s {
    uint32_t type;
    uint32_t timestamp;
    uint32_t windowID;
    uint8_t event;
    uint8_t padding1;
    uint8_t padding2;
    uint8_t padding3;
    int32_t data1;
    int32_t data2;
} my_SDL2_WindowEvent_t;

typedef struct SDL2_Keysym_s {
    int32_t scancode;
    int32_t sym;
    uint16_t mod;
    uint32_t unused;
} my_SDL2_Keysym_t;

typedef struct SDL2_KeyboardEvent_s {
    uint32_t type;
    uint32_t timestamp;
    uint32_t windowID;
    uint8_t state;
    uint8_t repeat;
    uint8_t padding2;
    uint8_t padding3;
    my_SDL2_Keysym_t keysym;
} my_SDL2_KeyboardEvent_t;

typedef struct SDL2_TextEditingEvent_s {
    uint32_t type;
    uint32_t timestamp;
    uint32_t windowID;
    char text[32];
    int32_t start;
    int32_t length;
} my_SDL2_TextEditingEvent_t;


typedef struct SDL2_TextEditingExtEvent_s {
    uint32_t type;
    uint32_t timestamp;
    uint32_t windowID;
    char* text;
    int32_t start;
    int32_t length;
} my_SDL2_TextEditingExtEvent_t;

typedef struct SDL2_TextInputEvent_s {
    uint32_t type;
    uint32_t timestamp;
    uint32_t windowID;
    char text[32];
} my_SDL2_TextInputEvent_t;

typedef struct SDL2_MouseMotionEvent_s {
    uint32_t type;
    uint32_t timestamp;
    uint32_t windowID;
    uint32_t which;
    uint32_t state;
    int32_t x;
    int32_t y;
    int32_t xrel;
    int32_t yrel;
} my_SDL2_MouseMotionEvent_t;

typedef struct SDL2_MouseButtonEvent_s {
    uint32_t type;
    uint32_t timestamp;
    uint32_t windowID;
    uint32_t which;
    uint8_t button;
    uint8_t state;
    uint8_t clicks;
    uint8_t padding1;
    int32_t x;
    int32_t y;
} my_SDL2_MouseButtonEvent_t;

typedef struct SDL2_MouseWheelEvent_s {
    uint32_t type;
    uint32_t timestamp;
    uint32_t windowID;
    uint32_t which;
    int32_t x;
    int32_t y;
    uint32_t direction;
    float preciseX;
    float preciseY;
    int32_t mouseX;
    int32_t mouseY;
} my_SDL2_MouseWheelEvent_t;

typedef struct SDL2_JoyAxisEvent_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
    uint8_t axis;
    uint8_t padding1;
    uint8_t padding2;
    uint8_t padding3;
    int16_t value;
    uint16_t padding4;
} my_SDL2_JoyAxisEvent_t;


typedef struct SDL2_JoyBallEvent_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
    uint8_t ball;
    uint8_t padding1;
    uint8_t padding2;
    uint8_t padding3;
    int16_t xrel;
    int16_t yrel;
} my_SDL2_JoyBallEvent_t;

typedef struct SDL2_JoyHatEvent_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
    uint8_t hat;
    uint8_t value;
    uint8_t padding1;
    uint8_t padding2;
} my_SDL2_JoyHatEvent_t;

typedef struct SDL2_JoyButtonEvent_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
    uint8_t button;
    uint8_t state;
    uint8_t padding1;
    uint8_t padding2;
} my_SDL2_JoyButtonEvent_t;

typedef struct SDL2_JoyDeviceEvent_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
} my_SDL2_JoyDeviceEvent_t;


typedef struct SDL2_JoyBatteryEvent_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
    int32_t level;
} my_SDL2_JoyBatteryEvent_t;

typedef struct SDL2_ControllerAxisEvent_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
    uint8_t axis;
    uint8_t padding1;
    uint8_t padding2;
    uint8_t padding3;
    int16_t value;
    uint16_t padding4;
} my_SDL2_ControllerAxisEvent_t;


typedef struct SDL2_ControllerButtonEvent_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
    uint8_t button;
    uint8_t state;
    uint8_t padding1;
    uint8_t padding2;
} my_SDL2_ControllerButtonEvent_t;


typedef struct SDL2_ControllerDeviceEvent_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
} my_SDL2_ControllerDeviceEvent_t;

typedef struct SDL2_ControllerTouchpadEvent_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
    int32_t touchpad;
    int32_t finger;
    float x;
    float y;
    float pressure;
} my_SDL2_ControllerTouchpadEvent_t;

typedef struct SDL2_ControllerSensorEvent_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
    int32_t sensor;
    float data[3];
    uint64_t timestamp_us;
} my_SDL2_ControllerSensorEvent_t;

typedef struct SDL2_AudioDeviceEvent_s {
    uint32_t type;
    uint32_t timestamp;
    uint32_t which;
    uint8_t iscapture;
    uint8_t padding1;
    uint8_t padding2;
    uint8_t padding3;
} my_SDL2_AudioDeviceEvent_t;

typedef struct SDL2_TouchFingerEvent_s {
    uint32_t type;
    uint32_t timestamp;
    int64_t touchId;
    int64_t fingerId;
    float x;
    float y;
    float dx;
    float dy;
    float pressure;
    uint32_t windowID;
} my_SDL2_TouchFingerEvent_t;

typedef struct SDL2_MultiGestureEvent_s {
    uint32_t type;
    uint32_t timestamp;
    int64_t touchId;
    float dTheta;
    float dDist;
    float x;
    float y;
    uint16_t numFingers;
    uint16_t padding;
} my_SDL2_MultiGestureEvent_t;


typedef struct SDL2_DollarGestureEvent_s {
    uint32_t type;
    uint32_t timestamp;
    int64_t touchId;
    int64_t gestureId;
    uint32_t numFingers;
    float error;
    float x;
    float y;
} my_SDL2_DollarGestureEvent_t;

typedef struct SDL2_DropEvent_s {
    uint32_t type;
    uint32_t timestamp;
    char* file;
    uint32_t windowID;
} my_SDL2_DropEvent_t;

typedef struct SDL2_SensorEvent_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
    float data[6];
    uint64_t timestamp_us;
} my_SDL2_SensorEvent_t;


typedef struct SDL2_QuitEvent_s {
    uint32_t type;
    uint32_t timestamp;
} my_SDL2_QuitEvent_t;


typedef struct SDL2_UserEvent_s {
    uint32_t type;
    uint32_t timestamp;
    uint32_t windowID;
    int32_t code;
    void* data1;
    void* data2;
} my_SDL2_UserEvent_t;

typedef struct SDL2_SysWMEvent_s {
    uint32_t type;
    uint32_t timestamp;
    void* msg;
} my_SDL2_SysWMEvent_t;


typedef union my_SDL2_Event_s {
    uint32_t type;
    my_SDL2_CommonEvent_t common;
    my_SDL2_DisplayEvent_t display;
    my_SDL2_WindowEvent_t window;
    my_SDL2_KeyboardEvent_t key;
    my_SDL2_TextEditingEvent_t edit;
    my_SDL2_TextEditingExtEvent_t editExt;
    my_SDL2_TextInputEvent_t text;
    my_SDL2_MouseMotionEvent_t motion;
    my_SDL2_MouseButtonEvent_t button;
    my_SDL2_MouseWheelEvent_t wheel;
    my_SDL2_JoyAxisEvent_t jaxis;
    my_SDL2_JoyBallEvent_t jball;
    my_SDL2_JoyHatEvent_t jhat;
    my_SDL2_JoyButtonEvent_t jbutton;
    my_SDL2_JoyDeviceEvent_t jdevice;
    my_SDL2_JoyBatteryEvent_t jbattery;
    my_SDL2_ControllerAxisEvent_t caxis;
    my_SDL2_ControllerButtonEvent_t cbutton;
    my_SDL2_ControllerDeviceEvent_t cdevice;
    my_SDL2_ControllerTouchpadEvent_t ctouchpad;
    my_SDL2_ControllerSensorEvent_t csensor;
    my_SDL2_AudioDeviceEvent_t adevice;
    my_SDL2_SensorEvent_t sensor;
    my_SDL2_QuitEvent_t quit;
    my_SDL2_UserEvent_t user;
    my_SDL2_SysWMEvent_t syswm;
    my_SDL2_TouchFingerEvent_t tfinger;
    my_SDL2_MultiGestureEvent_t mgesture;
    my_SDL2_DollarGestureEvent_t dgesture;
    my_SDL2_DropEvent_t drop;
} my_SDL2_Event_t;

typedef struct SDL2_CommonEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
} my_SDL2_CommonEvent_32_t;

typedef struct SDL2_DisplayEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    uint32_t display;
    uint8_t event;
    uint8_t padding1;
    uint8_t padding2;
    uint8_t padding3;
    int32_t data1;
} my_SDL2_DisplayEvent_32_t;

typedef struct SDL2_WindowEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    uint32_t windowID;
    uint8_t event;
    uint8_t padding1;
    uint8_t padding2;
    uint8_t padding3;
    int32_t data1;
    int32_t data2;
} my_SDL2_WindowEvent_32_t;

typedef struct SDL2_Keysym_32_s {
    int32_t scancode;
    int32_t sym;
    uint16_t mod;
    uint32_t unused;
} my_SDL2_Keysym_32_t;

typedef struct SDL2_KeyboardEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    uint32_t windowID;
    uint8_t state;
    uint8_t repeat;
    uint8_t padding2;
    uint8_t padding3;
    my_SDL2_Keysym_32_t keysym;
} my_SDL2_KeyboardEvent_32_t;

typedef struct SDL2_TextEditingEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    uint32_t windowID;
    char text[32];
    int32_t start;
    int32_t length;
} my_SDL2_TextEditingEvent_32_t;


typedef struct SDL2_TextEditingExtEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    uint32_t windowID;
    ptr_t text; //char*
    int32_t start;
    int32_t length;
} my_SDL2_TextEditingExtEvent_32_t;

typedef struct SDL2_TextInputEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    uint32_t windowID;
    char text[32];
} my_SDL2_TextInputEvent_32_t;

typedef struct SDL2_MouseMotionEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    uint32_t windowID;
    uint32_t which;
    uint32_t state;
    int32_t x;
    int32_t y;
    int32_t xrel;
    int32_t yrel;
} my_SDL2_MouseMotionEvent_32_t;

typedef struct SDL2_MouseButtonEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    uint32_t windowID;
    uint32_t which;
    uint8_t button;
    uint8_t state;
    uint8_t clicks;
    uint8_t padding1;
    int32_t x;
    int32_t y;
} my_SDL2_MouseButtonEvent_32_t;

typedef struct SDL2_MouseWheelEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    uint32_t windowID;
    uint32_t which;
    int32_t x;
    int32_t y;
    uint32_t direction;
    float preciseX;
    float preciseY;
    int32_t mouseX;
    int32_t mouseY;
} my_SDL2_MouseWheelEvent_32_t;

typedef struct SDL2_JoyAxisEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
    uint8_t axis;
    uint8_t padding1;
    uint8_t padding2;
    uint8_t padding3;
    int16_t value;
    uint16_t padding4;
} my_SDL2_JoyAxisEvent_32_t;


typedef struct SDL2_JoyBallEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
    uint8_t ball;
    uint8_t padding1;
    uint8_t padding2;
    uint8_t padding3;
    int16_t xrel;
    int16_t yrel;
} my_SDL2_JoyBallEvent_32_t;

typedef struct SDL2_JoyHatEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
    uint8_t hat;
    uint8_t value;
    uint8_t padding1;
    uint8_t padding2;
} my_SDL2_JoyHatEvent_32_t;

typedef struct SDL2_JoyButtonEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
    uint8_t button;
    uint8_t state;
    uint8_t padding1;
    uint8_t padding2;
} my_SDL2_JoyButtonEvent_32_t;

typedef struct SDL2_JoyDeviceEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
} my_SDL2_JoyDeviceEvent_32_t;


typedef struct SDL2_JoyBatteryEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
    int32_t level;
} my_SDL2_JoyBatteryEvent_32_t;

typedef struct SDL2_ControllerAxisEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
    uint8_t axis;
    uint8_t padding1;
    uint8_t padding2;
    uint8_t padding3;
    int16_t value;
    uint16_t padding4;
} my_SDL2_ControllerAxisEvent_32_t;


typedef struct SDL2_ControllerButtonEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
    uint8_t button;
    uint8_t state;
    uint8_t padding1;
    uint8_t padding2;
} my_SDL2_ControllerButtonEvent_32_t;


typedef struct SDL2_ControllerDeviceEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
} my_SDL2_ControllerDeviceEvent_32_t;

typedef struct SDL2_ControllerTouchpadEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
    int32_t touchpad;
    int32_t finger;
    float x;
    float y;
    float pressure;
} my_SDL2_ControllerTouchpadEvent_32_t;

typedef struct SDL2_ControllerSensorEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
    int32_t sensor;
    float data[3];
    uint64_t timestamp_us;
} my_SDL2_ControllerSensorEvent_32_t;

typedef struct SDL2_AudioDeviceEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    uint32_t which;
    uint8_t iscapture;
    uint8_t padding1;
    uint8_t padding2;
    uint8_t padding3;
} my_SDL2_AudioDeviceEvent_32_t;

typedef struct SDL2_TouchFingerEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    int64_t touchId;
    int64_t fingerId;
    float x;
    float y;
    float dx;
    float dy;
    float pressure;
    uint32_t windowID;
} my_SDL2_TouchFingerEvent_32_t;

typedef struct SDL2_MultiGestureEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    int64_t touchId;
    float dTheta;
    float dDist;
    float x;
    float y;
    uint16_t numFingers;
    uint16_t padding;
} my_SDL2_MultiGestureEvent_32_t;


typedef struct SDL2_DollarGestureEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    int64_t touchId;
    int64_t gestureId;
    uint32_t numFingers;
    float error;
    float x;
    float y;
} my_SDL2_DollarGestureEvent_32_t;

typedef struct SDL2_DropEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    ptr_t file;
    uint32_t windowID;
} my_SDL2_DropEvent_32_t;

typedef struct SDL2_SensorEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    int32_t which;
    float data[6];
    uint64_t timestamp_us;
} my_SDL2_SensorEvent_32_t;


typedef struct SDL2_QuitEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
} my_SDL2_QuitEvent_32_t;


typedef struct SDL2_UserEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    uint32_t windowID;
    int32_t code;
    ptr_t data1;
    ptr_t data2;
} my_SDL2_UserEvent_32_t;

typedef struct SDL2_SysWMEvent_32_s {
    uint32_t type;
    uint32_t timestamp;
    ptr_t msg;
} my_SDL2_SysWMEvent_32_t;


typedef union my_SDL2_Event_32_s {
    uint32_t type;
    my_SDL2_CommonEvent_32_t common;
    my_SDL2_DisplayEvent_32_t display;
    my_SDL2_WindowEvent_32_t window;
    my_SDL2_KeyboardEvent_32_t key;
    my_SDL2_TextEditingEvent_32_t edit;
    my_SDL2_TextEditingExtEvent_32_t editExt;
    my_SDL2_TextInputEvent_32_t text;
    my_SDL2_MouseMotionEvent_32_t motion;
    my_SDL2_MouseButtonEvent_32_t button;
    my_SDL2_MouseWheelEvent_32_t wheel;
    my_SDL2_JoyAxisEvent_32_t jaxis;
    my_SDL2_JoyBallEvent_32_t jball;
    my_SDL2_JoyHatEvent_32_t jhat;
    my_SDL2_JoyButtonEvent_32_t jbutton;
    my_SDL2_JoyDeviceEvent_32_t jdevice;
    my_SDL2_JoyBatteryEvent_32_t jbattery;
    my_SDL2_ControllerAxisEvent_32_t caxis;
    my_SDL2_ControllerButtonEvent_32_t cbutton;
    my_SDL2_ControllerDeviceEvent_32_t cdevice;
    my_SDL2_ControllerTouchpadEvent_32_t ctouchpad;
    my_SDL2_ControllerSensorEvent_32_t csensor;
    my_SDL2_AudioDeviceEvent_32_t adevice;
    my_SDL2_SensorEvent_32_t sensor;
    my_SDL2_QuitEvent_32_t quit;
    my_SDL2_UserEvent_32_t user;
    my_SDL2_SysWMEvent_32_t syswm;
    my_SDL2_TouchFingerEvent_32_t tfinger;
    my_SDL2_MultiGestureEvent_32_t mgesture;
    my_SDL2_DollarGestureEvent_32_t dgesture;
    my_SDL2_DropEvent_32_t drop;
} my_SDL2_Event_32_t;

typedef enum SDL2_EventType {
    SDL2_FIRSTEVENT = 0,
    SDL2_QUIT = 0x100,
    SDL2_APP_TERMINATING,
    SDL2_APP_LOWMEMORY,
    SDL2_APP_WILLENTERBACKGROUND,
    SDL2_APP_DIDENTERBACKGROUND,
    SDL2_APP_WILLENTERFOREGROUND,
    SDL2_APP_DIDENTERFOREGROUND,
    SDL2_LOCALECHANGED,
    SDL2_DISPLAYEVENT = 0x150,
    SDL2_WINDOWEVENT = 0x200,
    SDL2_SYSWMEVENT,
    SDL2_KEYDOWN = 0x300,
    SDL2_KEYUP,
    SDL2_TEXTEDITING,
    SDL2_TEXTINPUT,
    SDL2_KEYMAPCHANGED,
    SDL2_TEXTEDITING_EXT,
    SDL2_MOUSEMOTION = 0x400,
    SDL2_MOUSEBUTTONDOWN,
    SDL2_MOUSEBUTTONUP,
    SDL2_MOUSEWHEEL,
    SDL2_JOYAXISMOTION = 0x600,
    SDL2_JOYBALLMOTION,
    SDL2_JOYHATMOTION,
    SDL2_JOYBUTTONDOWN,
    SDL2_JOYBUTTONUP,
    SDL2_JOYDEVICEADDED,
    SDL2_JOYDEVICEREMOVED,
    SDL2_JOYBATTERYUPDATED,
    SDL2_CONTROLLERAXISMOTION = 0x650,
    SDL2_CONTROLLERBUTTONDOWN,
    SDL2_CONTROLLERBUTTONUP,
    SDL2_CONTROLLERDEVICEADDED,
    SDL2_CONTROLLERDEVICEREMOVED,
    SDL2_CONTROLLERDEVICEREMAPPED,
    SDL2_CONTROLLERTOUCHPADDOWN,
    SDL2_CONTROLLERTOUCHPADMOTION,
    SDL2_CONTROLLERTOUCHPADUP,
    SDL2_CONTROLLERSENSORUPDATE,
    SDL2_CONTROLLERUPDATECOMPLETE_RESERVED_FOR_SDL3,
    SDL2_CONTROLLERSTEAMHANDLEUPDATED,
    SDL2_FINGERDOWN = 0x700,
    SDL2_FINGERUP,
    SDL2_FINGERMOTION,
    SDL2_DOLLARGESTURE = 0x800,
    SDL2_DOLLARRECORD,
    SDL2_MULTIGESTURE,
    SDL2_CLIPBOARDUPDATE = 0x900,
    SDL2_DROPFILE = 0x1000,
    SDL2_DROPTEXT,
    SDL2_DROPBEGIN,
    SDL2_DROPCOMPLETE,
    SDL2_AUDIODEVICEADDED = 0x1100,
    SDL2_AUDIODEVICEREMOVED,
    SDL2_SENSORUPDATE = 0x1200,
    SDL2_RENDER_TARGETS_RESET = 0x2000,
    SDL2_RENDER_DEVICE_RESET,
    SDL2_POLLSENTINEL = 0x7F00,
    SDL2_USEREVENT = 0x8000,
    SDL2_LASTEVENT = 0xFFFF
} SDL2_EventType;

typedef struct my_SDL2_Palette_s {
    int ncolors;
    void* colors;
    uint32_t version;
    int refcount;
} my_SDL2_Palette_t;

typedef struct my_SDL2_PixelFormat_s {
    uint32_t format;
    my_SDL2_Palette_t* palette;
    uint8_t BitsPerPixel;
    uint8_t BytesPerPixel;
    uint8_t padding[2];
    uint32_t Rmask;
    uint32_t Gmask;
    uint32_t Bmask;
    uint32_t Amask;
    uint8_t Rloss;
    uint8_t Gloss;
    uint8_t Bloss;
    uint8_t Aloss;
    uint8_t Rshift;
    uint8_t Gshift;
    uint8_t Bshift;
    uint8_t Ashift;
    int refcount;
    struct my_SDL2_PixelFormat_s* next;
} my_SDL2_PixelFormat_t;

typedef struct my_SDL2_Rect_s {
    int x, y;
    int w, h;
} my_SDL2_Rect_t;

typedef struct my_SDL2_Surface_s {
    uint32_t flags;
    my_SDL2_PixelFormat_t* format;
    int w, h;
    int pitch;
    void* pixels;
    void* userdata;
    int locked;
    void* list_blitmap;
    my_SDL2_Rect_t clip_rect;
    void* map;
    int refcount;
} my_SDL2_Surface_t;

typedef struct my_SDL2_Palette_32_s {
    int ncolors;
    ptr_t colors;
    uint32_t version;
    int refcount;
} my_SDL2_Palette_32_t;

typedef struct my_SDL2_PixelFormat_32_s {
    uint32_t format;
    ptr_t palette;
    uint8_t BitsPerPixel;
    uint8_t BytesPerPixel;
    uint8_t padding[2];
    uint32_t Rmask;
    uint32_t Gmask;
    uint32_t Bmask;
    uint32_t Amask;
    uint8_t Rloss;
    uint8_t Gloss;
    uint8_t Bloss;
    uint8_t Aloss;
    uint8_t Rshift;
    uint8_t Gshift;
    uint8_t Bshift;
    uint8_t Ashift;
    int refcount;
    ptr_t next;
} my_SDL2_PixelFormat_32_t;

// used to mirror the pixelformat on x86 side, as the one in native side is a global static used by SDL2 and so cannot be inplace shrinked
typedef struct my_SDL2_PixelFormat_32_ext_s {
    my_SDL2_PixelFormat_32_t fmt;
    my_SDL2_PixelFormat_t*   ref;
    struct my_SDL2_PixelFormat_32_ext_s* next;
} my_SDL2_PixelFormat_32_ext_t;

typedef struct my_SDL2_Rect_32_s {
    int x, y;
    int w, h;
} my_SDL2_Rect_32_t;

typedef struct my_SDL2_Surface_32_s {
    uint32_t flags;
    ptr_t format;
    int w, h;
    int pitch;
    ptr_t pixels;
    ptr_t userdata;
    int locked;
    ptr_t list_blitmap;
    my_SDL2_Rect_32_t clip_rect;
    ptr_t map;
    int refcount;
} my_SDL2_Surface_32_t;

// simplified RWops
typedef struct my_SDL2_RWops_s {
    void* size;
    void* seek;
    void* read;
    void* write;
    void* close;
    uint32_t type;
    void* hidden[3];
} my_SDL2_RWops_t;

typedef struct my_SDL2_RWops_32_s {
    ptr_t size;
    ptr_t seek;
    ptr_t read;
    ptr_t write;
    ptr_t close;
    uint32_t type;
    ptr_t hidden[3]; // not converting hidden, just moving it
} my_SDL2_RWops_32_t;

void inplace_SDL2_DisplayMode_to_64(void* a);
void inplace_SDL2_DisplayMode_to_32(void* a);
void convert_SDL2_DisplayMode_to_64(void* dst_, void* src_);
void convert_SDL2_DisplayMode_to_32(void* dst_, void* src_);

void convert_SDL2_Event_to_64(void* dst_, const void* src_);
void convert_SDL2_Event_to_32(void* dst_, const void* src_);
void inplace_SDL2_Event_shrink(void* e);
void inplace_SDL2_Event_enlarge(void* e);

void inplace_SDL2_Palette_to_32(void* a);
void inplace_SDL2_PixelFormat_to_32(void* a);
void* replace_SDL2_PixelFormat_to_32_ext(void* a);
void inplace_SDL2_Surface_to_32(void* a);
void inplace_SDL2_Palette_to_64(void* a);
void inplace_SDL2_PixelFormat_to_64(void* a);
void* replace_SDL2_PixelFormat_to_64_ext(void* a);
void inplace_SDL2_Surface_to_64(void* a);

void inplace_SDL2_RWops_to_32(void* a);
void inplace_SDL2_RWops_to_64(void* a);

#endif // __MY_SDL2ALIGN32_H_