#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <wchar.h>
#include <sys/epoll.h>
#include <fts.h>
#include <sys/socket.h>

#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "myalign32.h"
#include "debug.h"
#include "box32.h"
#include "sdl2align32.h"

void inplace_SDL2_DisplayMode_to_64(void* a)
{
    if (!a) return;
    my_SDL2_DisplayMode_32_t* src = a;
    my_SDL2_DisplayMode_t* dst = a;
    dst->driverdata = from_ptrv(src->driverdata);
}

void inplace_SDL2_DisplayMode_to_32(void* a)
{
    if (!a) return;
    my_SDL2_DisplayMode_t* src = a;
    my_SDL2_DisplayMode_32_t* dst = a;
    dst->driverdata = to_ptrv(src->driverdata);
}

void convert_SDL2_DisplayMode_to_32(void* dst_, void* src_)
{
    if (!src_ || !dst_) return;
    my_SDL2_DisplayMode_t* src = src_;
    my_SDL2_DisplayMode_32_t* dst = dst_;
    dst->format = src->format;
    dst->w = src->w;
    dst->h = src->h;
    dst->refresh_rate = src->refresh_rate;
    dst->driverdata = to_ptrv(src->driverdata);
}

void convert_SDL2_DisplayMode_to_64(void* dst_, void* src_)
{
    if (!src_ || !dst_) return;
    my_SDL2_DisplayMode_32_t* src = src_;
    my_SDL2_DisplayMode_t* dst = dst_;
    dst->format = src->format;
    dst->w = src->w;
    dst->h = src->h;
    dst->refresh_rate = src->refresh_rate;
    dst->driverdata = from_ptrv(src->driverdata);
}

void convert_SDL2_Event_to_64(void* dst_, const void* src_)
{
    if (!src_ || !dst_) return;
    const my_SDL2_Event_32_t* src = src_;
    my_SDL2_Event_t* dst = dst_;
    dst->type = src->type;
    switch (src->type) {
        case SDL2_WINDOWEVENT:
            dst->window.type = src->window.type;
            dst->window.timestamp = src->window.timestamp;
            dst->window.windowID = src->window.windowID;
            dst->window.event = src->window.event;
            dst->window.padding1 = src->window.padding1;
            dst->window.padding2 = src->window.padding2;
            dst->window.padding3 = src->window.padding3;
            dst->window.data1 = src->window.data1;
            dst->window.data2 = src->window.data2;
            break;
        case SDL2_MOUSEMOTION:
            dst->motion.type = src->motion.type;
            dst->motion.timestamp = src->motion.timestamp;
            dst->motion.windowID = src->motion.windowID;
            dst->motion.which = src->motion.which;
            dst->motion.state = src->motion.state;
            dst->motion.x = src->motion.x;
            dst->motion.y = src->motion.y;
            dst->motion.xrel = src->motion.xrel;
            dst->motion.yrel = src->motion.yrel;
            break;
        case SDL2_MOUSEBUTTONDOWN:
        case SDL2_MOUSEBUTTONUP:
            dst->button.type = src->button.type;
            dst->button.timestamp = src->button.timestamp;
            dst->button.windowID = src->button.windowID;
            dst->button.which = src->button.which;
            dst->button.button = src->button.button;
            dst->button.state = src->button.state;
            dst->button.clicks = src->button.clicks;
            dst->button.padding1 = src->button.padding1;
            dst->button.x = src->button.x;
            dst->button.y = src->button.y;
            break;
        case SDL2_AUDIODEVICEADDED:
        case SDL2_AUDIODEVICEREMOVED:
            dst->adevice.type = src->adevice.type;
            dst->adevice.timestamp = src->adevice.timestamp;
            dst->adevice.which = src->adevice.which;
            dst->adevice.iscapture = src->adevice.iscapture;
            dst->adevice.padding1 = src->adevice.padding1;
            dst->adevice.padding2 = src->adevice.padding2;
            dst->adevice.padding3 = src->adevice.padding3;
            break;
        case SDL2_KEYDOWN:
        case SDL2_KEYUP:
            dst->key.type = src->key.type;
            dst->key.timestamp = src->key.timestamp;
            dst->key.windowID = src->key.windowID;
            dst->key.state = src->key.state;
            dst->key.repeat = src->key.repeat;
            dst->key.padding2 = src->key.padding2;
            dst->key.padding3 = src->key.padding3;
            dst->key.keysym.scancode = src->key.keysym.scancode;
            dst->key.keysym.sym = src->key.keysym.sym;
            dst->key.keysym.mod = src->key.keysym.mod;
            dst->key.keysym.unused = src->key.keysym.unused;
            break;
        case SDL2_TEXTEDITING:
            dst->edit.type = src->edit.type;
            dst->edit.timestamp = src->edit.timestamp;
            dst->edit.windowID = src->edit.windowID;
            memcpy(dst->edit.text, src->edit.text, 32);
            dst->edit.start = src->edit.start;
            dst->edit.length = src->edit.length;
            break;
        case SDL2_TEXTINPUT:
            dst->text.type = src->text.type;
            dst->text.timestamp = src->text.timestamp;
            dst->text.windowID = src->text.windowID;
            memcpy(dst->text.text, src->text.text, 32);
            break;
        case SDL2_MOUSEWHEEL:
            dst->wheel.type = src->wheel.type;
            dst->wheel.timestamp = src->wheel.timestamp;
            dst->wheel.windowID = src->wheel.windowID;
            dst->wheel.which = src->wheel.which;
            dst->wheel.x = src->wheel.x;
            dst->wheel.y = src->wheel.y;
            dst->wheel.direction = src->wheel.direction;
            dst->wheel.preciseX = src->wheel.preciseX;
            dst->wheel.preciseY = src->wheel.preciseY;
            dst->wheel.mouseX = src->wheel.mouseX;
            dst->wheel.mouseY = src->wheel.mouseY;
            break;
        default:
            printf_log(LOG_INFO, "Warning, unsuported SDL2 event %d\n", src->type);
            memcpy(dst, src, sizeof(my_SDL2_Event_32_t));
    }
}
void convert_SDL2_Event_to_32(void* dst_, const void* src_)
{
    if (!src_ || !dst_) return;
    const my_SDL2_Event_t* src = src_;
    my_SDL2_Event_32_t* dst = dst_;
    dst->type = src->type;
    switch (src->type) {
        case SDL2_WINDOWEVENT:
            dst->window.type = src->window.type;
            dst->window.timestamp = src->window.timestamp;
            dst->window.windowID = src->window.windowID;
            dst->window.event = src->window.event;
            dst->window.padding1 = src->window.padding1;
            dst->window.padding2 = src->window.padding2;
            dst->window.padding3 = src->window.padding3;
            dst->window.data1 = src->window.data1;
            dst->window.data2 = src->window.data2;
            break;
        case SDL2_MOUSEMOTION:
            dst->motion.type = src->motion.type;
            dst->motion.timestamp = src->motion.timestamp;
            dst->motion.windowID = src->motion.windowID;
            dst->motion.which = src->motion.which;
            dst->motion.state = src->motion.state;
            dst->motion.x = src->motion.x;
            dst->motion.y = src->motion.y;
            dst->motion.xrel = src->motion.xrel;
            dst->motion.yrel = src->motion.yrel;
            break;
        case SDL2_MOUSEBUTTONDOWN:
        case SDL2_MOUSEBUTTONUP:
            dst->button.type = src->button.type;
            dst->button.timestamp = src->button.timestamp;
            dst->button.windowID = src->button.windowID;
            dst->button.which = src->button.which;
            dst->button.button = src->button.button;
            dst->button.state = src->button.state;
            dst->button.clicks = src->button.clicks;
            dst->button.padding1 = src->button.padding1;
            dst->button.x = src->button.x;
            dst->button.y = src->button.y;
            break;
        case SDL2_AUDIODEVICEADDED:
        case SDL2_AUDIODEVICEREMOVED:
            dst->adevice.type = src->adevice.type;
            dst->adevice.timestamp = src->adevice.timestamp;
            dst->adevice.which = src->adevice.which;
            dst->adevice.iscapture = src->adevice.iscapture;
            dst->adevice.padding1 = src->adevice.padding1;
            dst->adevice.padding2 = src->adevice.padding2;
            dst->adevice.padding3 = src->adevice.padding3;
            break;
        case SDL2_KEYDOWN:
        case SDL2_KEYUP:
            dst->key.type = src->key.type;
            dst->key.timestamp = src->key.timestamp;
            dst->key.windowID = src->key.windowID;
            dst->key.state = src->key.state;
            dst->key.repeat = src->key.repeat;
            dst->key.padding2 = src->key.padding2;
            dst->key.padding3 = src->key.padding3;
            dst->key.keysym.scancode = src->key.keysym.scancode;
            dst->key.keysym.sym = src->key.keysym.sym;
            dst->key.keysym.mod = src->key.keysym.mod;
            dst->key.keysym.unused = src->key.keysym.unused;
            break;
        case SDL2_TEXTEDITING:
            dst->edit.type = src->edit.type;
            dst->edit.timestamp = src->edit.timestamp;
            dst->edit.windowID = src->edit.windowID;
            memcpy(dst->edit.text, src->edit.text, 32);
            dst->edit.start = src->edit.start;
            dst->edit.length = src->edit.length;
            break;
        case SDL2_TEXTINPUT:
            dst->text.type = src->text.type;
            dst->text.timestamp = src->text.timestamp;
            dst->text.windowID = src->text.windowID;
            memcpy(dst->text.text, src->text.text, 32);
            break;
        case SDL2_MOUSEWHEEL:
            dst->wheel.type = src->wheel.type;
            dst->wheel.timestamp = src->wheel.timestamp;
            dst->wheel.windowID = src->wheel.windowID;
            dst->wheel.which = src->wheel.which;
            dst->wheel.x = src->wheel.x;
            dst->wheel.y = src->wheel.y;
            dst->wheel.direction = src->wheel.direction;
            dst->wheel.preciseX = src->wheel.preciseX;
            dst->wheel.preciseY = src->wheel.preciseY;
            dst->wheel.mouseX = src->wheel.mouseX;
            dst->wheel.mouseY = src->wheel.mouseY;
            break;
        default:
            printf_log(LOG_INFO, "Warning, unsuported SDL2 event %d\n", src->type);
            memcpy(dst, src, sizeof(my_SDL2_Event_32_t));
    }
}

void inplace_SDL2_Palette_to_32(void* a)
{
    if (!a) return;
    my_SDL2_Palette_t* src = a;
    my_SDL2_Palette_32_t* dst = a;
    dst->ncolors = src->ncolors;
    dst->colors = to_ptrv(src->colors);
    dst->version = src->version;
    dst->refcount = src->refcount;
}

void inplace_SDL2_PixelFormat_to_32(void* a)
{
    while (a) {
        my_SDL2_PixelFormat_t* src = a;
        my_SDL2_PixelFormat_32_t* dst = a;
        dst->format = src->format;
        inplace_SDL2_Palette_to_32(src->palette);
        dst->palette = to_ptrv(src->palette);
        dst->BitsPerPixel = src->BitsPerPixel;
        dst->BytesPerPixel = src->BytesPerPixel;
        dst->Rmask = src->Rmask;
        dst->Gmask = src->Gmask;
        dst->Bmask = src->Bmask;
        dst->Amask = src->Amask;
        dst->Rloss = src->Rloss;
        dst->Gloss = src->Gloss;
        dst->Bloss = src->Bloss;
        dst->Aloss = src->Aloss;
        dst->Rshift = src->Rshift;
        dst->Gshift = src->Gshift;
        dst->Bshift = src->Bshift;
        dst->Ashift = src->Ashift;
        dst->refcount = src->refcount;
        a = (void*)src->next;
        dst->next = to_ptrv(src->next);
    }
}

void inplace_SDL2_Surface_to_32(void* a)
{
    if (!a) return;
    my_SDL2_Surface_t* src = a;
    my_SDL2_Surface_32_t* dst = a;
    dst->flags = src->flags;
    inplace_SDL2_PixelFormat_to_32(src->format);
    dst->format = to_ptrv(src->format);
    dst->w = src->w;
    dst->h = src->h;
    dst->pitch = src->pitch;
    dst->pixels = to_ptrv(src->pixels);
    dst->userdata = to_ptrv(src->userdata);
    dst->locked = src->locked;
    dst->list_blitmap = to_ptrv(src->list_blitmap);
    dst->clip_rect.x = src->clip_rect.x;
    dst->clip_rect.y = src->clip_rect.y;
    dst->clip_rect.w = src->clip_rect.w;
    dst->clip_rect.h = src->clip_rect.h;
    dst->map = to_ptrv(src->map);
    dst->refcount = src->refcount;
}


void inplace_SDL2_Palette_to_64(void* a)
{
    if (!a) return;
    my_SDL2_Palette_32_t* src = a;
    my_SDL2_Palette_t* dst = a;
    dst->refcount = src->refcount;
    dst->version = src->version;
    dst->colors = from_ptrv(src->colors);
    dst->ncolors = src->ncolors;
}

void inplace_SDL2_PixelFormat_to_64(void* a)
{
    while (a) {
        my_SDL2_PixelFormat_32_t* src = a;
        my_SDL2_PixelFormat_t* dst = a;
        uintptr_t p = (uintptr_t)(src->next);
        a = (void*)p;
        dst->next = from_ptrv(src->next);
        dst->refcount = src->refcount;
        dst->Ashift = src->Ashift;
        dst->Bshift = src->Bshift;
        dst->Gshift = src->Gshift;
        dst->Rshift = src->Rshift;
        dst->Aloss = src->Aloss;
        dst->Bloss = src->Bloss;
        dst->Gloss = src->Gloss;
        dst->Rloss = src->Rloss;
        dst->Amask = src->Amask;
        dst->Bmask = src->Bmask;
        dst->Gmask = src->Gmask;
        dst->Rmask = src->Rmask;
        dst->BytesPerPixel = src->BytesPerPixel;
        dst->BitsPerPixel = src->BitsPerPixel;
        p = (uintptr_t)(src->palette);
        inplace_SDL2_Palette_to_64((void*)p);
        dst->palette = from_ptrv(src->palette);
        dst->format = src->format;
    }
}

void inplace_SDL2_Surface_to_64(void* a)
{
    if (!a) return;
    my_SDL2_Surface_32_t* src = a;
    my_SDL2_Surface_t* dst = a;
    dst->refcount = src->refcount;
    dst->map = from_ptrv(src->map);
    dst->clip_rect.h = src->clip_rect.h;
    dst->clip_rect.w = src->clip_rect.w;
    dst->clip_rect.y = src->clip_rect.y;
    dst->clip_rect.x = src->clip_rect.x;
    dst->list_blitmap = from_ptrv(src->list_blitmap);
    dst->locked = src->locked;
    dst->userdata = from_ptrv(src->userdata);
    dst->pixels = from_ptrv(src->pixels);
    dst->pitch = src->pitch;
    dst->h = src->h;
    dst->w = src->w;
    uintptr_t p = (uintptr_t)(src->format);
    inplace_SDL2_PixelFormat_to_64((void*)p);
    dst->format = from_ptrv(src->format);
    dst->flags = src->flags;
}

void inplace_SDL2_RWops_to_32(void* a)
{
    if (!a) return;
    my_SDL2_RWops_t* src = a;
    my_SDL2_RWops_32_t* dst = a;
    dst->size = to_ptrv(src->size);
    dst->seek = to_ptrv(src->seek);
    dst->read = to_ptrv(src->read);
    dst->write = to_ptrv(src->write);
    dst->close = to_ptrv(src->close);
    dst->type = src->type;
    memmove(&dst->hidden, &src->hidden, sizeof(dst->hidden));
}

void inplace_SDL2_RWops_to_64(void* a)
{
    if(!a) return;
    my_SDL2_RWops_32_t* src = a;
    my_SDL2_RWops_t* dst = a;
    memmove(&dst->hidden, &src->hidden, sizeof(dst->hidden));
    dst->type = src->type;
    dst->close = from_ptrv(src->close);
    dst->write = from_ptrv(src->write);
    dst->read = from_ptrv(src->read);
    dst->seek = from_ptrv(src->seek);
    dst->size = from_ptrv(src->size);
}