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
    // in reverse order so it can be done inplace...
    if (!src_ || !dst_) return;
    const my_SDL2_Event_32_t* src = src_;
    my_SDL2_Event_t* dst = dst_;
    switch (src->type) {
        case SDL2_WINDOWEVENT:
            dst->window.data2 = src->window.data2;
            dst->window.data1 = src->window.data1;
            dst->window.padding3 = src->window.padding3;
            dst->window.padding2 = src->window.padding2;
            dst->window.padding1 = src->window.padding1;
            dst->window.event = src->window.event;
            dst->window.windowID = src->window.windowID;
            dst->window.timestamp = src->window.timestamp;
            dst->window.type = src->window.type;
            break;
        case SDL2_MOUSEMOTION:
            dst->motion.yrel = src->motion.yrel;
            dst->motion.xrel = src->motion.xrel;
            dst->motion.y = src->motion.y;
            dst->motion.x = src->motion.x;
            dst->motion.state = src->motion.state;
            dst->motion.which = src->motion.which;
            dst->motion.windowID = src->motion.windowID;
            dst->motion.timestamp = src->motion.timestamp;
            dst->motion.type = src->motion.type;
            break;
        case SDL2_MOUSEBUTTONDOWN:
        case SDL2_MOUSEBUTTONUP:
            dst->button.y = src->button.y;
            dst->button.x = src->button.x;
            dst->button.padding1 = src->button.padding1;
            dst->button.clicks = src->button.clicks;
            dst->button.state = src->button.state;
            dst->button.button = src->button.button;
            dst->button.which = src->button.which;
            dst->button.windowID = src->button.windowID;
            dst->button.timestamp = src->button.timestamp;
            dst->button.type = src->button.type;
            break;
        case SDL2_AUDIODEVICEADDED:
        case SDL2_AUDIODEVICEREMOVED:
            dst->adevice.padding3 = src->adevice.padding3;
            dst->adevice.padding2 = src->adevice.padding2;
            dst->adevice.padding1 = src->adevice.padding1;
            dst->adevice.iscapture = src->adevice.iscapture;
            dst->adevice.which = src->adevice.which;
            dst->adevice.timestamp = src->adevice.timestamp;
            dst->adevice.type = src->adevice.type;
            break;
        case SDL2_KEYDOWN:
        case SDL2_KEYUP:
            dst->key.keysym.unused = src->key.keysym.unused;
            dst->key.keysym.mod = src->key.keysym.mod;
            dst->key.keysym.sym = src->key.keysym.sym;
            dst->key.keysym.scancode = src->key.keysym.scancode;
            dst->key.padding3 = src->key.padding3;
            dst->key.padding2 = src->key.padding2;
            dst->key.repeat = src->key.repeat;
            dst->key.state = src->key.state;
            dst->key.windowID = src->key.windowID;
            dst->key.timestamp = src->key.timestamp;
            dst->key.type = src->key.type;
            break;
        case SDL2_TEXTEDITING:
            dst->edit.length = src->edit.length;
            dst->edit.start = src->edit.start;
            memcpy(dst->edit.text, src->edit.text, 32);
            dst->edit.windowID = src->edit.windowID;
            dst->edit.timestamp = src->edit.timestamp;
            dst->edit.type = src->edit.type;
            break;
        case SDL2_TEXTINPUT:
            memcpy(dst->text.text, src->text.text, 32);
            dst->text.windowID = src->text.windowID;
            dst->text.timestamp = src->text.timestamp;
            dst->text.type = src->text.type;
            break;
        case SDL2_MOUSEWHEEL:
            dst->wheel.mouseY = src->wheel.mouseY;
            dst->wheel.mouseX = src->wheel.mouseX;
            dst->wheel.preciseY = src->wheel.preciseY;
            dst->wheel.preciseX = src->wheel.preciseX;
            dst->wheel.direction = src->wheel.direction;
            dst->wheel.y = src->wheel.y;
            dst->wheel.x = src->wheel.x;
            dst->wheel.which = src->wheel.which;
            dst->wheel.windowID = src->wheel.windowID;
            dst->wheel.timestamp = src->wheel.timestamp;
            dst->wheel.type = src->wheel.type;
            break;
        case SDL2_POLLSENTINEL:
            dst->common.timestamp = src->common.timestamp;
            dst->common.type = src->common.type;
            break;
        case SDL2_DISPLAYEVENT:
            dst->display.data1 = src->display.data1;
            dst->display.padding3 = src->display.padding3;
            dst->display.padding2 = src->display.padding2;
            dst->display.padding1 = src->display.padding1;
            dst->display.event = src->display.event;
            dst->display.display = src->display.display;
            dst->display.timestamp = src->display.timestamp;
            dst->display.type = src->display.type;
            break;
        case SDL2_JOYDEVICEADDED:
            dst->jdevice.which = src->jdevice.which;
            dst->jdevice.timestamp = src->jdevice.timestamp;
            dst->jdevice.type = src->jdevice.type;
            break;
        case SDL2_CONTROLLERDEVICEADDED:
            dst->cdevice.which = src->cdevice.which;
            dst->cdevice.timestamp = src->cdevice.timestamp;
            dst->cdevice.type = src->cdevice.type;
            break;
        case SDL2_JOYAXISMOTION:
            dst->jaxis.padding4 = src->jaxis.padding4;
            dst->jaxis.value = src->jaxis.value;
            dst->jaxis.padding3 = src->jaxis.padding3;
            dst->jaxis.padding2 = src->jaxis.padding2;
            dst->jaxis.padding1 = src->jaxis.padding1;
            dst->jaxis.axis = src->jaxis.axis;
            dst->jaxis.which = src->jaxis.which;
            dst->jaxis.timestamp = src->jaxis.timestamp;
            dst->jaxis.type = src->jaxis.type;
            break;
        case SDL2_CONTROLLERAXISMOTION:
            dst->caxis.padding4 = src->caxis.padding4;
            dst->caxis.value = src->caxis.value;
            dst->caxis.padding3 = src->caxis.padding3;
            dst->caxis.padding2 = src->caxis.padding2;
            dst->caxis.padding1 = src->caxis.padding1;
            dst->caxis.axis = src->caxis.axis;
            dst->caxis.which = src->caxis.which;
            dst->caxis.timestamp = src->caxis.timestamp;
            dst->caxis.type = src->caxis.type;
            break;
        case SDL2_QUIT:
            dst->quit.timestamp = src->quit.timestamp;
            dst->quit.type = src->quit.type;
            break;
        case SDL2_CLIPBOARDUPDATE:
            dst->common.timestamp = src->common.timestamp;
            dst->common.type = src->common.type;
            break;
        case SDL2_JOYBUTTONDOWN:
        case SDL2_JOYBUTTONUP:
            dst->jbutton.type = src->jbutton.type;
            dst->jbutton.timestamp = src->jbutton.timestamp;
            dst->jbutton.which = src->jbutton.which;
            dst->jbutton.button = src->jbutton.button;
            dst->jbutton.state = src->jbutton.state;
            dst->jbutton.padding1 = src->jbutton.padding1;
            dst->jbutton.padding2 = src->jbutton.padding2;
            break;
        case SDL2_CONTROLLERBUTTONDOWN:
        case SDL2_CONTROLLERBUTTONUP:
            dst->cbutton.type = src->cbutton.type;
            dst->cbutton.timestamp = src->cbutton.timestamp;
            dst->cbutton.which = src->cbutton.which;
            dst->cbutton.button = src->cbutton.button;
            dst->cbutton.state = src->cbutton.state;
            dst->cbutton.padding1 = src->cbutton.padding1;
            dst->cbutton.padding2 = src->cbutton.padding2;
            break;
        case SDL2_SYSWMEVENT:
            dst->syswm.type = src->syswm.type;
            dst->syswm.timestamp = src->syswm.timestamp;
            dst->syswm.msg = from_ptrv(src->syswm.msg);
            break;
        default:
            printf_log(LOG_INFO, "Warning, unsuported SDL2 event %d\n", src->type);
            if(dst_!=src_)
                memmove(dst, src, sizeof(my_SDL2_Event_32_t));
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
        case SDL2_POLLSENTINEL:
            dst->common.type = src->common.type;
            dst->common.timestamp = src->common.timestamp;
            break;
        case SDL2_DISPLAYEVENT:
            dst->display.type = src->display.type;
            dst->display.timestamp = src->display.timestamp;
            dst->display.display = src->display.display;
            dst->display.event = src->display.event;
            dst->display.padding1 = src->display.padding1;
            dst->display.padding2 = src->display.padding2;
            dst->display.padding3 = src->display.padding3;
            dst->display.data1 = src->display.data1;
            break;
        case SDL2_JOYDEVICEADDED:
            dst->jdevice.type = src->jdevice.type;
            dst->jdevice.timestamp = src->jdevice.timestamp;
            dst->jdevice.which = src->jdevice.which;
            break;
        case SDL2_CONTROLLERDEVICEADDED:
            dst->cdevice.type = src->cdevice.type;
            dst->cdevice.timestamp = src->cdevice.timestamp;
            dst->cdevice.which = src->cdevice.which;
            break;
        case SDL2_JOYAXISMOTION:
            dst->jaxis.type = src->jaxis.type;
            dst->jaxis.timestamp = src->jaxis.timestamp;
            dst->jaxis.which = src->jaxis.which;
            dst->jaxis.axis = src->jaxis.axis;
            dst->jaxis.padding1 = src->jaxis.padding1;
            dst->jaxis.padding2 = src->jaxis.padding2;
            dst->jaxis.padding3 = src->jaxis.padding3;
            dst->jaxis.value = src->jaxis.value;
            dst->jaxis.padding4 = src->jaxis.padding4;
            break;
        case SDL2_CONTROLLERAXISMOTION:
            dst->caxis.type = src->caxis.type;
            dst->caxis.timestamp = src->caxis.timestamp;
            dst->caxis.which = src->caxis.which;
            dst->caxis.axis = src->caxis.axis;
            dst->caxis.padding1 = src->caxis.padding1;
            dst->caxis.padding2 = src->caxis.padding2;
            dst->caxis.padding3 = src->caxis.padding3;
            dst->caxis.value = src->caxis.value;
            dst->caxis.padding4 = src->caxis.padding4;
            break;
        case SDL2_QUIT:
            dst->quit.type = src->quit.type;
            dst->quit.timestamp = src->quit.timestamp;
            break;
        case SDL2_CLIPBOARDUPDATE:
            dst->common.type = src->common.type;
            dst->common.timestamp = src->common.timestamp;
            break;
        case SDL2_JOYBUTTONDOWN:
        case SDL2_JOYBUTTONUP:
            dst->jbutton.type = src->jbutton.type;
            dst->jbutton.timestamp = src->jbutton.timestamp;
            dst->jbutton.which = src->jbutton.which;
            dst->jbutton.button = src->jbutton.button;
            dst->jbutton.state = src->jbutton.state;
            dst->jbutton.padding1 = src->jbutton.padding1;
            dst->jbutton.padding2 = src->jbutton.padding2;
            break;
        case SDL2_CONTROLLERBUTTONDOWN:
        case SDL2_CONTROLLERBUTTONUP:
            dst->cbutton.type = src->cbutton.type;
            dst->cbutton.timestamp = src->cbutton.timestamp;
            dst->cbutton.which = src->cbutton.which;
            dst->cbutton.button = src->cbutton.button;
            dst->cbutton.state = src->cbutton.state;
            dst->cbutton.padding1 = src->cbutton.padding1;
            dst->cbutton.padding2 = src->cbutton.padding2;
            break;
        case SDL2_SYSWMEVENT:
            dst->syswm.type = src->syswm.type;
            dst->syswm.timestamp = src->syswm.timestamp;
            dst->syswm.msg = to_ptrv_silent(src->syswm.msg);    // msg would need translation probably!
            break;
        default:
            printf_log(LOG_INFO, "Warning, unsuported SDL2 event %d\n", src->type);
            if(dst_!=src_)
                memmove(dst, src, sizeof(my_SDL2_Event_32_t));
    }
}

void inplace_SDL2_Event_shrink(void* e)
{
    if(!e) return;
    convert_SDL2_Event_to_32(e, e);
}
void inplace_SDL2_Event_enlarge(void* e)
{
    if(!e) return;
    convert_SDL2_Event_to_64(e, e);
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
#define SDL2_PIXELFORMAT_SIGN 0xBADC0FEE
void inplace_SDL2_PixelFormat_to_32(void* a)
{
    while (a) {
        my_SDL2_PixelFormat_t* src = a;
        my_SDL2_PixelFormat_32_t* dst = a;
        if(*(uint32_t*)(dst+1) != SDL2_PIXELFORMAT_SIGN) {
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
            // put a signature to avoid muultiple unpack because of the next handling
            *(uint32_t*)(dst+1) = SDL2_PIXELFORMAT_SIGN;
        } else a = from_ptrv(dst->next);
    }
}

// used to mirror the pixelformat on x86 side, as the one in native side is a global static used by SDL2 and so cannot be inplace shrinked
// TODO: the pixelformats are leaking. There is no mecanism to remove them
// test of refcount should be done when surface are removed to delete the format, in sync with SDL2
static my_SDL2_PixelFormat_32_ext_t* pixelformat_head = NULL;

static my_SDL2_PixelFormat_32_ext_t* find_pixelformat_ext(my_SDL2_PixelFormat_t* a)
{
    my_SDL2_PixelFormat_32_ext_t* head = pixelformat_head;
    while(head) {
        if(head->ref == a)
            return head;
        head = head->next;
    }
    return NULL;
}

void* replace_SDL2_PixelFormat_to_32_ext(void* a)
{
    if(!a) return NULL;
    my_SDL2_PixelFormat_t* src = a;
    my_SDL2_PixelFormat_32_ext_t* dst = find_pixelformat_ext(src);
    if(dst) return dst;
    // create a new one
    dst = calloc(1, sizeof(my_SDL2_PixelFormat_32_ext_t)+(src->palette?sizeof(my_SDL2_Palette_32_t):0));  // not using box_calloc as we want this to be 32bits
    // copye the values
    dst->ref = src;
    dst->fmt.format = src->format;
    if(src->palette) {
        my_SDL2_Palette_32_t* pal = (void*)((uintptr_t)dst)+sizeof(my_SDL2_PixelFormat_32_ext_t); 
        dst->fmt.palette = to_ptrv(pal);
        pal->ncolors = src->palette->ncolors;
        pal->colors = to_ptrv(src->palette->colors);
        pal->version = src->palette->version;
        pal->refcount = src->palette->refcount;
    } else dst->fmt.palette = 0;
    dst->fmt.BitsPerPixel = src->BitsPerPixel;
    dst->fmt.BytesPerPixel = src->BytesPerPixel;
    dst->fmt.Rmask = src->Rmask;
    dst->fmt.Gmask = src->Gmask;
    dst->fmt.Bmask = src->Bmask;
    dst->fmt.Amask = src->Amask;
    dst->fmt.Rloss = src->Rloss;
    dst->fmt.Gloss = src->Gloss;
    dst->fmt.Bloss = src->Bloss;
    dst->fmt.Aloss = src->Aloss;
    dst->fmt.Rshift = src->Rshift;
    dst->fmt.Gshift = src->Gshift;
    dst->fmt.Bshift = src->Bshift;
    dst->fmt.Ashift = src->Ashift;
    dst->fmt.refcount = src->refcount;
    dst->fmt.next = to_ptrv(src->next);
    // insert at head and return the value
    dst->next = pixelformat_head;
    pixelformat_head = dst;
    return dst;
}

void* replace_SDL2_PixelFormat_to_64_ext(void* a)
{
    if(!a) return a;
    my_SDL2_PixelFormat_32_ext_t* dst = a;
    return dst->ref;
}

void inplace_SDL2_Surface_to_32(void* a)
{
    if (!a) return;
    my_SDL2_Surface_t* src = a;
    my_SDL2_Surface_32_t* dst = a;
    dst->flags = src->flags;
    #if 0
    inplace_SDL2_PixelFormat_to_32(src->format);
    dst->format = to_ptrv(src->format);
    #else
    dst->format = to_ptrv(replace_SDL2_PixelFormat_to_32_ext(src->format));
    #endif
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
        // check signatue
        if(*(uint32_t*)(src+1) == SDL2_PIXELFORMAT_SIGN) {
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
        } else a = dst->next;   // already 64bits!
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
    #if 0
    uintptr_t p = (uintptr_t)(src->format);
    inplace_SDL2_PixelFormat_to_64((void*)p);
    dst->format = from_ptrv(src->format);
    #else
    dst->format = replace_SDL2_PixelFormat_to_64_ext(from_ptrv(src->format));
    #endif
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