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
#include "sdl1align32.h"


void inplace_SDL_Palette_to_64(void* a) {
    if(!a) return;
    my_SDL_Palette_32_t* src = a;
    my_SDL_Palette_t* dst = a;
    dst->colors = from_ptrv(src->colors);
}
void inplace_SDL_Palette_to_32(void* a) {
    if(!a) return;
    my_SDL_Palette_t* src = a;
    my_SDL_Palette_32_t* dst = a;
    dst->colors = to_ptrv(src->colors);
}
void inplace_SDL_PixelFormat_to_64_nopalette(void* a) {
    if(!a) return;
    my_SDL_PixelFormat_32_t* src = a;
    my_SDL_PixelFormat_t* dst = a;
    memmove(&dst->BitsPerPixel, &src->BitsPerPixel, sizeof(my_SDL_PixelFormat_t)-offsetof(my_SDL_PixelFormat_t, BitsPerPixel));
    dst->palette = from_ptrv(src->palette);
}
void inplace_SDL_PixelFormat_to_64(void* a) {
    if(!a) return;
    my_SDL_PixelFormat_32_t* src = a;
    my_SDL_PixelFormat_t* dst = a;
    memmove(&dst->BitsPerPixel, &src->BitsPerPixel, sizeof(my_SDL_PixelFormat_t)-offsetof(my_SDL_PixelFormat_t, BitsPerPixel));
    dst->palette = from_ptrv(src->palette);
    inplace_SDL_Palette_to_64(dst->palette);
}
void inplace_SDL_PixelFormat_to_32(void* a) {
    if(!a) return;
    my_SDL_PixelFormat_t* src = a;
    my_SDL_PixelFormat_32_t* dst = a;
    inplace_SDL_Palette_to_32(src->palette);
    dst->palette = to_ptrv(src->palette);
    memmove(&dst->BitsPerPixel, &src->BitsPerPixel, sizeof(my_SDL_PixelFormat_32_t)-offsetof(my_SDL_PixelFormat_32_t, BitsPerPixel));
}

void inplace_SDL_Surface_to_64(void* a) {
    if(!a) return;
    my_SDL_Surface_32_t* src = a;
    my_SDL_Surface_t* dst = a;
    dst->refcount = src->refcount;
    dst->format_version = src->format_version;
    dst->map = from_ptrv(src->map);
    dst->locked = src->locked;
    dst->unused1 = src->unused1;
    memmove(&dst->clip_rect, &src->clip_rect, sizeof(dst->clip_rect));
    dst->hwdata = from_ptrv(src->hwdata);
    dst->offset = src->offset;
    dst->pixels = from_ptrv(src->pixels);
    dst->pitch = src->pitch;
    dst->h = src->h;
    dst->w = src->w;
    dst->format = from_ptrv(src->format);
    inplace_SDL_PixelFormat_to_64(dst->format);
}
void inplace_SDL_Surface_to_32(void* a) {
    if(!a) return;
    my_SDL_Surface_t* src = a;
    my_SDL_Surface_32_t* dst = a;
    inplace_SDL_PixelFormat_to_32(src->format);
    dst->format = to_ptrv(src->format);
    dst->w = src->w;
    dst->h = src->h;
    dst->pitch = src->pitch;
    dst->pixels = to_ptrv(src->pixels);
    dst->offset = src->offset;
    dst->hwdata = to_ptrv(src->hwdata);
    memmove(&dst->clip_rect, &src->clip_rect, sizeof(dst->clip_rect));
    dst->unused1 = src->unused1;
    dst->locked = src->locked;
    dst->map = to_ptrv(src->map);
    dst->format_version = src->format_version;
    dst->refcount = src->refcount;
}


void convert_SDL_Event_to_32(void* dst_, const void* src_)
{
    if(!src_|| !dst_) return;
    const my_SDL_Event_t *src = src_;
    my_SDL_Event_32_t* dst = dst_;
    dst->type = src->type;
    switch(src->type) {
        case SDL_ACTIVEEVENT:
            dst->active.gain = src->active.gain;
            dst->active.state = src->active.state;
            break;
        case SDL_KEYUP:
        case SDL_KEYDOWN:
            dst->key.keysym.mod = src->key.keysym.mod;
            dst->key.keysym.scancode = src->key.keysym.scancode;
            dst->key.keysym.sym = src->key.keysym.sym;
            dst->key.keysym.unicode = src->key.keysym.unicode;
            dst->key.state = src->key.state;
            dst->key.which = dst->key.which;
            break;
        case SDL_MOUSEMOTION:
            dst->motion.state = src->motion.state;
            dst->motion.which = src->motion.which;
            dst->motion.x = src->motion.x;
            dst->motion.y = src->motion.y;
            dst->motion.xrel = src->motion.xrel;
            dst->motion.yrel = src->motion.yrel;
            break;
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            dst->button.button = src->button.button;
            dst->button.state = src->button.state;
            dst->button.which = src->button.which;
            dst->button.x = src->button.x;
            dst->button.y = src->button.y;
            break;
        case SDL_JOYAXISMOTION:
            dst->jaxis.axis = src->jaxis.axis;
            dst->jaxis.value = src->jaxis.value;
            dst->jaxis.which = src->jaxis.which;
            break;
        case SDL_JOYBALLMOTION:
            dst->jball.ball = src->jball.ball;
            dst->jball.which = src->jball.which;
            dst->jball.xrel = src->jball.xrel;
            dst->jball.yrel = src->jball.yrel;
            break;
        case SDL_JOYHATMOTION:
            dst->jhat.hat = src->jhat.hat;
            dst->jhat.value = src->jhat.value;
            dst->jhat.which = src->jhat.which;
            break;
        case SDL_JOYBUTTONUP:
        case SDL_JOYBUTTONDOWN:
            dst->jbutton.button = src->jbutton.button;
            dst->jbutton.state = src->jbutton.state;
            dst->jbutton.which = src->jbutton.which;
            break;
        case SDL_VIDEORESIZE:
            dst->resize.h = src->resize.h;
            dst->resize.w = src->resize.w;
            break;
        case SDL_VIDEOEXPOSE:
        case SDL_QUIT:
            break;
        case SDL_USEREVENT:
            dst->user.code = src->user.code;
            dst->user.data1 = to_ptrv(src->user.data1);
            dst->user.data2 = to_ptrv(src->user.data2);
            break;
        case SDL_SYSWMEVENT:
            printf_log(LOG_NONE, "TODO: Convert SDL_SYSWMEVENT\n");
            abort();
            break;
        default:
            printf_log(LOG_INFO, "Warning, unsupported SDL1.2 event %d\n", src->type);
            memcpy(dst, src, sizeof(my_SDL_Event_32_t));
    }
}
void convert_SDL_Event_to_64(void* dst_, const void* src_)
{
    if(!src_|| !dst_) return;
    const my_SDL_Event_32_t *src = src_;
    my_SDL_Event_t* dst = dst_;
    dst->type = src->type;
    switch(src->type) {
        case SDL_ACTIVEEVENT:
            dst->active.gain = src->active.gain;
            dst->active.state = src->active.state;
            break;
        case SDL_KEYUP:
        case SDL_KEYDOWN:
            dst->key.keysym.mod = src->key.keysym.mod;
            dst->key.keysym.scancode = src->key.keysym.scancode;
            dst->key.keysym.sym = src->key.keysym.sym;
            dst->key.keysym.unicode = src->key.keysym.unicode;
            dst->key.state = src->key.state;
            dst->key.which = dst->key.which;
            break;
        case SDL_MOUSEMOTION:
            dst->motion.state = src->motion.state;
            dst->motion.which = src->motion.which;
            dst->motion.x = src->motion.x;
            dst->motion.y = src->motion.y;
            dst->motion.xrel = src->motion.xrel;
            dst->motion.yrel = src->motion.yrel;
            break;
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            dst->button.button = src->button.button;
            dst->button.state = src->button.state;
            dst->button.which = src->button.which;
            dst->button.x = src->button.x;
            dst->button.y = src->button.y;
            break;
        case SDL_JOYAXISMOTION:
            dst->jaxis.axis = src->jaxis.axis;
            dst->jaxis.value = src->jaxis.value;
            dst->jaxis.which = src->jaxis.which;
            break;
        case SDL_JOYBALLMOTION:
            dst->jball.ball = src->jball.ball;
            dst->jball.which = src->jball.which;
            dst->jball.xrel = src->jball.xrel;
            dst->jball.yrel = src->jball.yrel;
            break;
        case SDL_JOYHATMOTION:
            dst->jhat.hat = src->jhat.hat;
            dst->jhat.value = src->jhat.value;
            dst->jhat.which = src->jhat.which;
            break;
        case SDL_JOYBUTTONUP:
        case SDL_JOYBUTTONDOWN:
            dst->jbutton.button = src->jbutton.button;
            dst->jbutton.state = src->jbutton.state;
            dst->jbutton.which = src->jbutton.which;
            break;
        case SDL_VIDEORESIZE:
            dst->resize.h = src->resize.h;
            dst->resize.w = src->resize.w;
            break;
        case SDL_VIDEOEXPOSE:
        case SDL_QUIT:
            break;
        case SDL_USEREVENT:
            dst->user.code = src->user.code;
            dst->user.data1 = from_ptrv(src->user.data1);
            dst->user.data2 = from_ptrv(src->user.data2);
            break;
        case SDL_SYSWMEVENT:
            printf_log(LOG_NONE, "TODO: Convert SDL_SYSWMEVENT\n");
            abort();
            break;
        default:
            printf_log(LOG_INFO, "Warning, unsupported SDL1.2 (un)event %d\n", src->type);
            memcpy(dst, src, sizeof(my_SDL_Event_32_t));
    }
}

void inplace_SDL_RWops_to_64(void* a)
{
    if(!a) return;
    my_SDL_RWops_32_t* src = a;
    my_SDL_RWops_t* dst = a;
    memmove(&dst->hidden, &src->hidden, sizeof(dst->hidden));
    dst->type = src->type;
    dst->close = from_ptrv(src->close);
    dst->write = from_ptrv(src->write);
    dst->read = from_ptrv(src->read);
    dst->seek = from_ptrv(src->seek);

}
void inplace_SDL_RWops_to_32(void* a)
{
    if(!a) return;
    my_SDL_RWops_t* src = a;
    my_SDL_RWops_32_t* dst = a;
    dst->seek = to_ptrv(src->seek);
    dst->read = to_ptrv(src->read);
    dst->write = to_ptrv(src->write);
    dst->close = to_ptrv(src->close);
    dst->type = src->type;
    memmove(&dst->hidden, &src->hidden, sizeof(dst->hidden));
}

void convert_AudioCVT_to_32(void* d, void* s)
{
    my_SDL_AudioCVT_32_t* dst = d;
    my_SDL_AudioCVT_t* src = s;
    dst->needed = src->needed;
    dst->src_format = src->src_format;
    dst->dest_format = src->dest_format;
    dst->rate_incr = src->rate_incr;
    dst->buf = to_ptrv(src->buf);
    dst->len = src->len;
    dst->len_cvt = src->len_cvt;
    dst->len_mult = src->len_mult;
    dst->len_ratio = src->len_ratio;
    for(int i=0; i<10; ++i)
        dst->filters[i] = to_ptrv(src->filters[i]);
    dst->filter_index = src->filter_index;
}
void convert_AudioCVT_to_64(void* d, void* s)
{
    my_SDL_AudioCVT_t* dst = d;
    my_SDL_AudioCVT_32_t* src = s;
    dst->needed = src->needed;
    dst->src_format = src->src_format;
    dst->dest_format = src->dest_format;
    dst->rate_incr = src->rate_incr;
    dst->buf = from_ptrv(src->buf);
    dst->len = src->len;
    dst->len_cvt = src->len_cvt;
    dst->len_mult = src->len_mult;
    dst->len_ratio = src->len_ratio;
    for(int i=0; i<10; ++i)
        dst->filters[i] = from_ptrv(src->filters[i]);
    dst->filter_index = src->filter_index;
}
