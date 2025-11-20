#include <stdint.h>
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
#include "myalign32.h"

typedef struct Visuals_s {
    my_Visual_t* _64;
    my_Visual_32_t* _32;
    int ref;    // 0 is 64, 1 is 32
} Visuals_t;
KHASH_MAP_INIT_INT(visuals, Visuals_t);

#define N_DISPLAY 16
my_XDisplay_t* my32_Displays_64[N_DISPLAY] = {0};
struct my_XFreeFuncs_32 my32_free_funcs_32[N_DISPLAY] = {0};
struct my_XLockPtrs_32 my32_lock_fns_32[N_DISPLAY] = {0};
my_XDisplay_32_t my32_Displays_32[N_DISPLAY] = {0};
kh_visuals_t* my32_Displays_Visuals[N_DISPLAY] = {0};
void* my32_XCB_Display[N_DISPLAY] = {0};

void* getDisplay(void* d)
{
    if(!d) return d;
    for(int i=0; i<N_DISPLAY; ++i)
        if(((&my32_Displays_32[i])==d) || (my32_Displays_64[i]==d)) {
            return my32_Displays_64[i];
        }
    printf_log(LOG_INFO, "BOX32: Warning, 32bits Display %p not found\n", d);
    return d;
}

void* FindDisplay(void* d)
{
    if(!d) return d;
    for(int i=0; i<N_DISPLAY; ++i)
        if((my32_Displays_64[i]==d) || ((&my32_Displays_32[i])==d)) {
            return &my32_Displays_32[i];
        }
    return d;
}

void convert_Screen_to_32(void* d, void* s)
{
    my_Screen_t* src = s;
    my_Screen_32_t* dst = d;
    dst->ext_data = to_ptrv(src->ext_data);
    dst->display = to_ptrv(FindDisplay(src->display));
    dst->root = to_ulong(src->root);
    dst->width = src->width;
    dst->height = src->height;
    dst->mwidth = src->mwidth;
    dst->mheight = src->mheight;
    dst->ndepths = src->ndepths;
    dst->depths = to_ptrv(src->depths);
    dst->root_depth = src->root_depth;
    dst->root_visual = to_ptrv(src->root_visual);
    dst->default_gc = to_ptrv(src->default_gc);
    dst->cmap = to_ulong(src->cmap);
    dst->white_pixel = to_ulong(src->white_pixel);
    dst->black_pixel = to_ulong(src->black_pixel);
    dst->max_maps = src->max_maps;
    dst->min_maps = src->min_maps;
    dst->backing_store = src->backing_store;
    dst->save_unders = src->save_unders;
    dst->root_input_mask = to_long(src->root_input_mask);
}
void convert_Screen_to_64(void* d, void* s)
{
    my_Screen_t* dst = d;
    my_Screen_32_t* src = s;
    dst->root_input_mask = from_long(src->root_input_mask);
    dst->save_unders = src->save_unders;
    dst->backing_store = src->backing_store;
    dst->min_maps = src->min_maps;
    dst->max_maps = src->max_maps;
    dst->black_pixel = from_ulong(src->black_pixel);
    dst->white_pixel = from_ulong(src->white_pixel);
    dst->cmap = from_ulong(src->cmap);
    dst->default_gc = from_ptrv(src->default_gc);
    dst->root_visual = from_ptrv(src->root_visual);
    dst->root_depth = src->root_depth;
    dst->depths = from_ptrv(src->depths);
    dst->ndepths = src->ndepths;
    dst->mheight = src->mheight;
    dst->mwidth = src->mwidth;
    dst->height = src->height;
    dst->width = src->width;
    dst->root = from_ulong(src->root);
    dst->display = getDisplay(from_ptrv(src->display));
    dst->ext_data = from_ptrv(src->ext_data);
}

void internal_convert_Visual_to_32(void* d, void* s)
{
    my_Visual_t* src = s;
    my_Visual_32_t* dst = d;
    dst->ext_data = to_ptrv(src->ext_data);
    dst->visualid = to_ulong(src->visualid);
    dst->c_class = src->c_class;
    dst->red_mask = to_ulong(src->red_mask);
    dst->green_mask = to_ulong(src->green_mask);
    dst->blue_mask = to_ulong(src->blue_mask);
    dst->bits_per_rgb = src->bits_per_rgb;
    dst->map_entries = src->map_entries;
}
void internal_convert_Visual_to_64(void* d, void* s)
{
    my_Visual_32_t* src = s;
    my_Visual_t* dst = d;
    dst->map_entries = src->map_entries;
    dst->bits_per_rgb = src->bits_per_rgb;
    dst->blue_mask = from_ulong(src->blue_mask);
    dst->green_mask = from_ulong(src->green_mask);
    dst->red_mask = from_ulong(src->red_mask);
    dst->c_class = src->c_class;
    dst->visualid = from_ulong(src->visualid);
    dst->ext_data = from_ptrv(src->ext_data);
}

my_Visual_32_t* getVisual32(int N, my_Visual_t* a)
{
    if(!a) return NULL;
    uint32_t key = a->visualid;
    khint_t k = kh_get(visuals, my32_Displays_Visuals[N], key);
    Visuals_t* ret = NULL;
    if(k==kh_end(my32_Displays_Visuals[N])) {
        int r;
        k = kh_put(visuals, my32_Displays_Visuals[N], key, &r);
        ret = &kh_value(my32_Displays_Visuals[N], k);
        ret->_32 = calloc(1, sizeof(my_Visual_32_t));
        ret->_64 = a;
        ret->ref = 0;
        internal_convert_Visual_to_32(ret->_32, ret->_64);
    } else
        ret = &kh_value(my32_Displays_Visuals[N], k);
    return ret->_32;
}
my_Visual_t* getVisual64(int N, my_Visual_32_t* a)
{
    if(!a) return NULL;
    uint32_t key = a->visualid;
    khint_t k = kh_get(visuals, my32_Displays_Visuals[N], key);
    Visuals_t* ret = NULL;
    if(k==kh_end(my32_Displays_Visuals[N])) {
        // workaround for already "64" Visual
        // needed for retrocityrampge (probably other)
        // TODO: fix the underlying issue
        if(!a->ext_data && !a->visualid)
            return (my_Visual_t*)a;
        int r;
        k = kh_put(visuals, my32_Displays_Visuals[N], key, &r);
        ret = &kh_value(my32_Displays_Visuals[N], k);
        ret->_64 = calloc(1, sizeof(my_Visual_t));
        ret->_32 = a;
        ret->ref = 1;
        internal_convert_Visual_to_64(ret->_64, ret->_32);
    } else
        ret = &kh_value(my32_Displays_Visuals[N], k);
    return ret->_64;
}

void* get_display_screen(int idx, void* a)
{
    my_XDisplay_t* dpy = my32_Displays_64[idx];
    my_XDisplay_32_t* dpy32 = &my32_Displays_32[idx];
    my_Screen_32_t* screens32 = from_ptrv(dpy32->screens);
    for(int i=0; i<dpy->nscreens; ++i)
        if(&screens32[i] == a)
            return &dpy->screens[i];
    return a;
}

void* convert_Visual_to_32(void* dpy, void* a)
{
    if(!dpy) return a;
    for(int i=0; i<N_DISPLAY; ++i)
        if(((&my32_Displays_32[i])==dpy) || (my32_Displays_64[i]==dpy)) {
            return getVisual32(i, a);
        }
    return a;
}
void* convert_Visual_to_64(void* dpy, void* a)
{
    if(!dpy) return a;
    for(int i=0; i<N_DISPLAY; ++i)
        if(((&my32_Displays_32[i])==dpy) || (my32_Displays_64[i]==dpy)) {
            return getVisual64(i, a);
        }
    return a;
}

void* getScreen64(void* dpy, void* a)
{
    if(!dpy) return a;
    for(int i=0; i<N_DISPLAY; ++i)
        if(((&my32_Displays_32[i])==dpy) || (my32_Displays_64[i]==dpy)) {
            return get_display_screen(i, a);
        }
    return a;
}

void* my_dlopen(x64emu_t* emu, void *filename, int flag);
void* addDisplay(void* d)
{
    if(!my_context->libx11) {
        // the lib has not be loaded directly... need to open it! leaking the lib handle...
        #ifdef ANDROID
        my_dlopen(thread_get_emu(), "libX11.so", RTLD_NOW);
        #else
        my_dlopen(thread_get_emu(), "libX11.so.6", RTLD_NOW);
        #endif
    }
    my_XDisplay_t* dpy = (my_XDisplay_t*)d;
    // look for a free slot, or a display already there
    my_XDisplay_32_t* ret = NULL;
    struct my_XFreeFuncs_32 *free_funcs = NULL;
    struct my_XLockPtrs_32 *lock_fns = NULL;
    for(int i=0; i<N_DISPLAY && !ret; ++i)
        if(my32_Displays_64[i]==dpy)
            return &my32_Displays_32[i];
    for(int i=0; i<N_DISPLAY && !ret; ++i) {
        if(!my32_Displays_64[i]) {
            my32_Displays_64[i] = dpy;
            ret = &my32_Displays_32[i];
            free_funcs = &my32_free_funcs_32[i];
            ret->free_funcs = (dpy->free_funcs)?to_ptrv(free_funcs):0;
            lock_fns = &my32_lock_fns_32[i];
            ret->lock_fns = (dpy->lock_fns)?to_ptrv(lock_fns):0;
            my32_Displays_Visuals[i] = kh_init(visuals);
        }
    }
    if(!ret) {
        printf_log(LOG_INFO, "BOX32: No more slot available for libX11 Display!");
        return d;
    }

    bridge_t* system = my_context->libx11->w.bridge;

    // partial copy...

    #define GO(A, W)\
    if(dpy->A)      \
        ret->A = AddCheckBridge(system, W, dpy->A, 0, #A); \

    #define GO2(A, B, W) \
    if(dpy->A && dpy->A->B)  \
        A->B = AddCheckBridge(system, W, dpy->A->B, 0, #B "_" #A); \

    ret->vendor = to_cstring(dpy->vendor);
    ret->fd = dpy->fd;
    ret->conn_checker = dpy->conn_checker;
    ret->proto_major_version = dpy->proto_major_version;
    ret->proto_minor_version = dpy->proto_minor_version;
    ret->resource_base = to_ulong(dpy->resource_base);
    ret->resource_mask = to_ulong(dpy->resource_mask);
    ret->resource_id = to_ulong(dpy->resource_id);
    ret->xdefaults = to_cstring(dpy->xdefaults);
    ret->resource_shift = dpy->resource_shift;
    ret->byte_order = dpy->byte_order;
    ret->bitmap_unit = dpy->bitmap_unit;
    ret->bitmap_pad = dpy->bitmap_pad;
    ret->bitmap_bit_order = dpy->bitmap_bit_order;
    ret->display_name = to_cstring(dpy->display_name);
    ret->default_screen = dpy->default_screen;
    ret->nscreens = dpy->nscreens;
    if(dpy->screens) {
        my_Screen_32_t* screens = calloc(dpy->nscreens, sizeof(my_Screen_32_t));
        ret->screens = to_ptrv(screens);
        for(int i=0; i<dpy->nscreens; ++i) {
            convert_Screen_to_32(screens+i, dpy->screens+i);
            screens[i].root_visual = to_ptrv(getVisual32(i, dpy->screens[i].root_visual));
        }
    } else
        ret->screens = 0;

    GO2(free_funcs, atoms, vFp_32)
    GO2(free_funcs, modifiermap, iFp_32)
    GO2(free_funcs, key_bindings, vFp_32)
    GO2(free_funcs, context_db, vFp_32)
    GO2(free_funcs, defaultCCCs, vFp_32)
    GO2(free_funcs, clientCmaps, vFp_32)
    GO2(free_funcs, intensityMaps, vFp_32)
    GO2(free_funcs, im_filters, vFp_32)
    GO2(free_funcs, xkb, vFp_32)
    GO(resource_alloc, LFp_32)
    GO(synchandler, iFp_32)
    //TODO: ext_procs?
    //TODO: event_vec?
    //TODO: wire_vec?
    //TODO: async_handlers?
    GO2(lock_fns, lock_display, vFX_32)
    GO2(lock_fns, unlock_display, vFX_32)
    GO(idlist_alloc, vFppi_32)
    //TODO: error_vec?
    //TODO: flushes
    //TODO: im_fd_info?
    //TODO: conn_watchers
    GO(savedsynchandler, iFp_32)
    //TODO: generic_event_vec?
    //TODO: generic_event_copy_vec?


    #undef GO
    #undef GO2

    return ret;
}

void delDisplay(void* d)
{
    for(int i=0; i<N_DISPLAY; ++i) {
        // crude free of ressources... not perfect
        if(my32_Displays_64[i]==d) {
            my32_Displays_64[i] = NULL;
            free(from_ptrv(my32_Displays_32[i].screens));
            my32_Displays_32[i].screens = 0;
            Visuals_t* v;
            uint32_t k;
            kh_foreach_ref(my32_Displays_Visuals[i], k, v, if(v->ref) free(v->_64); else free(v->_32));
            kh_destroy(visuals, my32_Displays_Visuals[i]);
            my32_Displays_Visuals[i] = NULL;
            if(my32_XCB_Display[i]) {
                del_xcb_connection32(my32_XCB_Display[i]);
                my32_XCB_Display[i] = NULL;
            }
            return;
        }
    }
}

void regXCBDisplay(void* d, void* xcb)
{    if(!my_context->libx11) {
        // the lib has not be loaded directly... need to open it! leaking the lib handle...
        #ifdef ANDROID
        my_dlopen(thread_get_emu(), "libX11.so", RTLD_NOW);
        #else
        my_dlopen(thread_get_emu(), "libX11.so.6", RTLD_NOW);
        #endif
    }
    my_XDisplay_t* dpy = (my_XDisplay_t*)d;
    for(int i=0; i<N_DISPLAY; ++i)
        if(my32_Displays_64[i]==dpy) {
            my32_XCB_Display[i] = xcb;
            return;
        }
}

void refreshDisplay(void* dpy)
{
    // update some of the values now that the screen is locked
    my_XDisplay_t* src = dpy;
    my_XDisplay_32_t* dst = FindDisplay(dpy);
    // sync last request
    dst->request = src->request;
    // num lock
    dst->num_lock = src->num_lock;
    // sync screens
    if(dst->nscreens!=src->nscreens) {
        my_Screen_32_t* screens = from_ptrv(dst->screens);
        dst->nscreens = src->nscreens;
        screens = realloc(screens, dst->nscreens*sizeof(my_Screen_32_t));
        dst->screens = to_ptrv(screens);
        for(int i=0; i<dst->nscreens; ++i)
            convert_Screen_to_32(screens+i, src->screens+i);
    }
    // functions
    bridge_t* system = my_context->libx11->w.bridge;
    int N = -1;
    for(int i=0; i<N_DISPLAY && (N==-1); ++i) {
        if(my32_Displays_64[i]==dpy)
            N = i;
    }
    struct my_XFreeFuncs_32 *free_funcs = &my32_free_funcs_32[N];
    dst->free_funcs = (src->free_funcs)?to_ptrv(free_funcs):0;
    struct my_XLockPtrs_32 *lock_fns = &my32_lock_fns_32[N];
    dst->lock_fns = (src->lock_fns)?to_ptrv(lock_fns):0;
    #define GO(A, W)\
    if(src->A)      \
        dst->A = AddCheckBridge(system, W, src->A, 0, #A); \

    #define GO2(A, B, W) \
    if(src->A && src->A->B)  \
        A->B = AddCheckBridge(system, W, src->A->B, 0, #B "_" #A); \

    GO2(free_funcs, atoms, vFp_32)
    GO2(free_funcs, modifiermap, iFp_32)
    GO2(free_funcs, key_bindings, vFp_32)
    GO2(free_funcs, context_db, vFp_32)
    GO2(free_funcs, defaultCCCs, vFp_32)
    GO2(free_funcs, clientCmaps, vFp_32)
    GO2(free_funcs, intensityMaps, vFp_32)
    GO2(free_funcs, im_filters, vFp_32)
    GO2(free_funcs, xkb, vFp_32)
    GO(resource_alloc, LFp_32)
    GO(synchandler, iFp_32)
    //TODO: ext_procs?
    //TODO: event_vec?
    //TODO: wire_vec?
    //TODO: async_handlers?
    GO2(lock_fns, lock_display, vFX_32)
    GO2(lock_fns, unlock_display, vFX_32)
    GO(idlist_alloc, vFppi_32)
    //TODO: error_vec?
    //TODO: flushes
    //TODO: im_fd_info?
    //TODO: conn_watchers
    GO(savedsynchandler, iFp_32)
    //TODO: generic_event_vec?
    //TODO: generic_event_copy_vec?

    #undef GO
    #undef GO2
}

void convert_XWMints_to_64(void* d, void* s)
{
    my_XWMHints_t* dst = d;
    my_XWMHints_32_t* src = s;
    long flags = from_long(src->flags);
    // reverse order
    if(flags&XWMHint_WindowGroupHint)   dst->window_group = from_ulong(src->window_group);
    if(flags&XWMHint_IconMaskHint)      dst->icon_mask = from_ulong(src->icon_mask);
    if(flags&XWMHint_IconPositionHint)  {dst->icon_y = src->icon_y; dst->icon_x = src->icon_x;}
    if(flags&XWMHint_IconWindowHint)    dst->icon_window = from_ulong(src->icon_window);
    if(flags&XWMHint_IconPixmapHint)    dst->icon_pixmap = from_ulong(src->icon_pixmap);
    if(flags&XWMHint_StateHint)         dst->initial_state = src->initial_state;
    if(flags&XWMHint_InputHint)         dst->input = src->input;

    dst->flags = flags;
}
void inplace_enlarge_wmhints(void* hints)
{
    if(!hints) return;
    my_XWMHints_32_t* src = hints;
    my_XWMHints_t* dst = hints;
    long flags = from_long(src->flags);
    // reverse order
    if(flags&XWMHint_WindowGroupHint)   dst->window_group = from_ulong(src->window_group);
    if(flags&XWMHint_IconMaskHint)      dst->icon_mask = from_ulong(src->icon_mask);
    if(flags&XWMHint_IconPositionHint)  {dst->icon_y = src->icon_y; dst->icon_x = src->icon_x;}
    if(flags&XWMHint_IconWindowHint)    dst->icon_window = from_ulong(src->icon_window);
    if(flags&XWMHint_IconPixmapHint)    dst->icon_pixmap = from_ulong(src->icon_pixmap);
    if(flags&XWMHint_StateHint)         dst->initial_state = src->initial_state;
    if(flags&XWMHint_InputHint)         dst->input = src->input;

    dst->flags = flags;
}
void inplace_shrink_wmhints(void* hints)
{
    if(!hints) return;
    my_XWMHints_t* src = hints;
    my_XWMHints_32_t* dst = hints;
    long_t flags = to_long(src->flags);
    // forward order
    if(flags&XWMHint_InputHint)         dst->input = src->input;
    if(flags&XWMHint_StateHint)         dst->initial_state = src->initial_state;
    if(flags&XWMHint_IconPixmapHint)    dst->icon_pixmap = to_ulong(src->icon_pixmap);
    if(flags&XWMHint_IconWindowHint)    dst->icon_window = to_ulong(src->icon_window);
    if(flags&XWMHint_IconPositionHint)  {dst->icon_y = src->icon_y; dst->icon_x = src->icon_x;}
    if(flags&XWMHint_IconMaskHint)      dst->icon_mask = to_ulong(src->icon_mask);
    if(flags&XWMHint_WindowGroupHint)   dst->window_group = to_ulong(src->window_group);

    dst->flags = flags;
}

void convert_XSizeHints_to_64(void* d, void *s)
{
    //XSizeHints is a long flag and 17*int...
    long flags = to_long(*(long_t*)s);
    memcpy(d+8, s+4, 17*4);
    *(long*)d = flags;
}
void inplace_enlarge_wmsizehints(void* hints)
{
    //XSizeHints is a long flag and 17*int...
    long flags = to_long(*(long_t*)hints);
    memmove(hints+8, hints+4, 17*4);
    *(long*)hints = flags;
}
void inplace_shrink_wmsizehints(void* hints)
{
    //XSizeHints is a long flag and 17*int...
    long_t flags = from_long(*(long*)hints);
    memmove(hints+4, hints+8, 17*4);
    *(long_t*)hints = flags;
}

void convert_XWindowAttributes_to_32(void* dpy, void* d, void* s)
{
    my_XWindowAttributes_t* src = s;
    my_XWindowAttributes_32_t* dst = d;
    dst->x = src->x;
    dst->y = src->y;
    dst->width = src->width;
    dst->height = src->height;
    dst->border_width = src->border_width;
    dst->depth = src->depth;
    dst->visual = to_ptrv(convert_Visual_to_32(dpy, src->visual));
    dst->root = to_ulong(src->root);
    dst->c_class = src->c_class;
    dst->bit_gravity = src->bit_gravity;
    dst->win_gravity = src->win_gravity;
    dst->backing_store = src->backing_store;
    dst->backing_planes = to_ulong(src->backing_planes);
    dst->backing_pixel = to_ulong(src->backing_pixel);
    dst->save_under = src->save_under;
    dst->colormap = to_ulong(src->colormap);
    dst->map_installed = src->map_installed;
    dst->map_state = src->map_state;
    dst->all_event_masks = to_long(src->all_event_masks);
    dst->your_event_mask = to_long(src->your_event_mask);
    dst->do_not_propagate_mask = to_long(src->do_not_propagate_mask);
    dst->override_redirect = src->override_redirect;
    dst->screen = to_ptrv(src->screen);
}

void inplace_XModifierKeymap_shrink(void* a)
{
    my_XModifierKeymap_32_t *d = a;
    my_XModifierKeymap_t* s = a;

    d->max_keypermod = s->max_keypermod;
    d->modifiermap = to_ptrv(s->modifiermap);
}
void inplace_XModifierKeymap_enlarge(void* a)
{
    my_XModifierKeymap_t *d = a;
    my_XModifierKeymap_32_t* s = a;

    d->modifiermap = from_ptrv(s->modifiermap);
    d->max_keypermod = s->max_keypermod;
}

void convert_XVisualInfo_to_32(void* dpy, my_XVisualInfo_32_t* dst, my_XVisualInfo_t* src)
{
    dst->visual = to_ptrv(convert_Visual_to_32(dpy, src->visual));
    dst->visualid = to_ulong(src->visualid);
    dst->screen = src->screen;
    dst->depth = src->depth;
    dst->c_class = src->c_class;
    dst->red_mask = to_ulong(src->red_mask);
    dst->green_mask = to_ulong(src->green_mask);
    dst->blue_mask = to_ulong(src->blue_mask);
    dst->colormap_size = src->colormap_size;
    dst->bits_per_rgb = src->bits_per_rgb;
}
void convert_XVisualInfo_to_64_novisual(void* dpy, my_XVisualInfo_t* dst, my_XVisualInfo_32_t* src)
{
    dst->bits_per_rgb = src->bits_per_rgb;
    dst->colormap_size = src->colormap_size;
    dst->blue_mask = from_ulong(src->blue_mask);
    dst->green_mask = from_ulong(src->green_mask);
    dst->red_mask = from_ulong(src->red_mask);
    dst->c_class = src->c_class;
    dst->depth = src->depth;
    dst->screen = src->screen;
    dst->visualid = from_ulong(src->visualid);
    dst->visual = NULL;
}
void convert_XVisualInfo_to_64(void* dpy, my_XVisualInfo_t* dst, my_XVisualInfo_32_t* src)
{
    dst->bits_per_rgb = src->bits_per_rgb;
    dst->colormap_size = src->colormap_size;
    dst->blue_mask = from_ulong(src->blue_mask);
    dst->green_mask = from_ulong(src->green_mask);
    dst->red_mask = from_ulong(src->red_mask);
    dst->c_class = src->c_class;
    dst->depth = src->depth;
    dst->screen = src->screen;
    dst->visualid = from_ulong(src->visualid);
    dst->visual = convert_Visual_to_64(dpy, from_ptrv(src->visual));
}
void inplace_XVisualInfo_shrink(void* dpy, void *a)
{
    if(!a) return;
    my_XVisualInfo_t *src = a;
    my_XVisualInfo_32_t* dst = a;

    convert_XVisualInfo_to_32(dpy, dst, src);
}
void inplace_XVisualInfo_enlarge(void* dpy, void *a)
{
    if(!a) return;
    my_XVisualInfo_32_t *src = a;
    my_XVisualInfo_t* dst = a;

    convert_XVisualInfo_to_64(dpy, dst, src);
}

void inplace_XdbeVisualInfo_shrink(void* a)
{
    if(!a) return;
    my_XdbeVisualInfo_t *src = a;
    my_XdbeVisualInfo_32_t* dst = a;

    dst->visual = to_ulong(src->visual);
    dst->depth = src->depth;
    dst->perflevel = src->perflevel;
}
void inplace_XdbeScreenVisualInfo_shrink(void* a)
{
    if(!a) return;
    my_XdbeScreenVisualInfo_t *src = a;
    my_XdbeScreenVisualInfo_32_t* dst = a;

    for(int i=0; i<src->count; ++i)
        inplace_XdbeVisualInfo_shrink(src->visinfo+i);
    dst->count = src->count;
    dst->visinfo = to_ptrv(src->visinfo);
}
void inplace_XdbeVisualInfo_enlarge(void* a)
{
    if(!a) return;
    my_XdbeVisualInfo_32_t *src = a;
    my_XdbeVisualInfo_t* dst = a;

    dst->perflevel = src->perflevel;
    dst->depth = src->depth;
    dst->visual = from_ulong(src->visual);
}
void inplace_XdbeScreenVisualInfo_enlarge(void* a)
{
    if(!a) return;
    my_XdbeScreenVisualInfo_32_t *src = a;
    my_XdbeScreenVisualInfo_t* dst = a;

    dst->visinfo = from_ptrv(src->visinfo);
    dst->count = src->count;
    for(int i=dst->count-1; i>=0; --i)
        inplace_XdbeVisualInfo_enlarge(dst->visinfo+i);
}

void inplace_XExtDisplayInfo_shrink(void* a)
{
    if(!a) return;
    my_XExtDisplayInfo_t* src = a;
    my_XExtDisplayInfo_32_t* dst = a;

    dst->next = to_ptrv(src->next);
    dst->display = to_ptrv(FindDisplay(src->display));
    dst->codes = to_ptrv(src->codes);
    dst->data = to_ptrv(src->data);
}
void inplace_XExtDisplayInfo_enlarge(void* a)
{
    if(!a) return;
    my_XExtDisplayInfo_32_t* src = a;
    my_XExtDisplayInfo_t* dst = a;

    dst->data = from_ptrv(src->data);
    dst->codes = from_ptrv(src->codes);
    dst->display = getDisplay(from_ptrv(src->display));
    dst->next = from_ptrv(src->next);
}

void convert_XExtensionInfo_to_32(void* d, void* s)
{
    if(!s || !d) return;
    my_XExtensionInfo_t* src = s;
    my_XExtensionInfo_32_t* dst = d;

    my_XExtDisplayInfo_t* head = src->head;
    while(head) {
        my_XExtDisplayInfo_t* next = head->next;
        inplace_XExtDisplayInfo_shrink(head);
        head = next;
    }
    dst->head = to_ptrv(src->head);
    dst->cur = to_ptrv(src->cur);
    dst->ndisplays = src->ndisplays;
}
void convert_XExtensionInfo_to_64(void* d, void* s)
{
    if(!d || !s) return;
    my_XExtensionInfo_32_t* src = s;
    my_XExtensionInfo_t* dst = d;

    dst->ndisplays = src->ndisplays;
    dst->cur = from_ptrv(src->cur);
    dst->head = from_ptrv(src->head);
    my_XExtDisplayInfo_t* head = dst->head;
    while(head) {
        inplace_XExtDisplayInfo_enlarge(head);
        head = head->next;
    }
}
void* inplace_XExtensionInfo_shrink(void* a)
{
    if(a)
        convert_XExtensionInfo_to_32(a, a);
    return a;
}
void* inplace_XExtensionInfo_enlarge(void* a)
{
    if(a)
        convert_XExtensionInfo_to_64(a, a);
    return a;
}

void convert_XFontProp_to_32(my_XFontProp_32_t* dst, my_XFontProp_t* src)
{
    dst->name = to_ulong(src->name);
    dst->card32 = to_ulong(src->card32);
}
void inplace_XFontProp_shrink(void* a)
{
    if(!a) return;
    my_XFontProp_t* src = a;
    my_XFontProp_32_t* dst = a;
    convert_XFontProp_to_32(dst, src);
}
void convert_XFontProp_to_64(my_XFontProp_t* dst, my_XFontProp_32_t* src)
{
    dst->card32 = from_ulong(src->card32);
    dst->name = from_ulong(src->name);
}
void inplace_XFontProp_enlarge(void* a)
{
    if(!a) return;
    my_XFontProp_32_t* src = a;
    my_XFontProp_t* dst = a;
    convert_XFontProp_to_64(dst, src);
}
void inplace_XFontStruct_shrink(void* a)
{
    if(!a) return;
    my_XFontStruct_t* src = a;
    my_XFontStruct_32_t* dst = a;

    my_XFontProp_32_t* properties_s = (my_XFontProp_32_t*)src->properties;
    for(int i=0; i<src->n_properties; ++i)
        convert_XFontProp_to_32(properties_s+i, src->properties+i);

    dst->ext_data = to_ptrv(src->ext_data);
    dst->fid = to_ulong(src->fid);
    dst->direction = src->direction;
    dst->min_char_or_byte2 = src->min_char_or_byte2;
    dst->max_char_or_byte2 = src->max_char_or_byte2;
    dst->min_byte1 = src->min_byte1;
    dst->max_byte1 = src->max_byte1;
    dst->all_chars_exist = src->all_chars_exist;
    dst->default_char = src->default_char;
    dst->n_properties = src->n_properties;
    dst->properties = to_ptrv(src->properties);
    memmove(&dst->min_bounds, &src->min_bounds, sizeof(dst->min_bounds));
    memmove(&dst->max_bounds, &src->max_bounds, sizeof(dst->max_bounds));
    dst->per_char = to_ptrv(src->per_char);
    dst->ascent = src->ascent;
    dst->descent = src->descent;
}
void inplace_XFontStruct_enlarge(void* a)
{
    if(!a) return;
    my_XFontStruct_32_t* src = a;
    my_XFontStruct_t* dst = a;

    dst->descent = src->descent;
    dst->ascent = src->ascent;
    dst->per_char = from_ptrv(src->per_char);
    memmove(&dst->max_bounds, &src->max_bounds, sizeof(dst->max_bounds));
    memmove(&dst->min_bounds, &src->min_bounds, sizeof(dst->min_bounds));
    dst->properties = from_ptrv(src->properties);
    dst->n_properties = src->n_properties;
    dst->default_char = src->default_char;
    dst->all_chars_exist = src->all_chars_exist;
    dst->max_byte1 = src->max_byte1;
    dst->min_byte1 = src->min_byte1;
    dst->max_char_or_byte2 = src->max_char_or_byte2;
    dst->min_char_or_byte2 = src->min_char_or_byte2;
    dst->direction = src->direction;
    dst->fid = from_ulong(src->fid);
    dst->ext_data = from_ptrv(src->ext_data);

    my_XFontProp_32_t* properties_s = (my_XFontProp_32_t*)dst->properties;
    for(int i=dst->n_properties-1; i>=0;  --i)
        convert_XFontProp_to_64(dst->properties+i, properties_s+i);
}

void convert_XSetWindowAttributes_to_64(my_XSetWindowAttributes_t* dst, my_XSetWindowAttributes_32_t* src)
{
    dst->background_pixmap = from_ulong(src->background_pixmap);
    dst->background_pixel = from_ulong(src->background_pixel);
    dst->border_pixmap = from_ulong(src->border_pixmap);
    dst->border_pixel = from_ulong(src->border_pixel);
    dst->bit_gravity = src->bit_gravity;
    dst->win_gravity = src->win_gravity;
    dst->backing_store = from_ulong(src->backing_store);
    dst->backing_planes = from_ulong(src->backing_planes);
    dst->backing_pixel = src->backing_pixel;
    dst->save_under = src->save_under;
    dst->event_mask = from_long(src->event_mask);
    dst->do_not_propagate_mask = from_long(src->do_not_propagate_mask);
    dst->override_redirect = src->override_redirect;
    dst->colormap = from_ulong(src->colormap);
    dst->cursor = from_ulong(src->cursor);
}

void* inplace_XImage_shrink(void* a)
{
    if(!a) return NULL;
    WrapXImage(a, a);
    return a;
}
void* inplace_XImage_enlarge(void* a)
{
    if(!a) return NULL;
    UnwrapXImage(a, a);
    return a;
}

void convert_XRRModeInfo_to_32(void* d, const void* s)
{
    my_XRRModeInfo_32_t *dst = d;
    const my_XRRModeInfo_t *src = s;
    dst->id = to_ulong(src->id);
    dst->width = src->width;
    dst->height = src->height;
    dst->dotClock = to_ulong(src->dotClock);
    dst->hSyncStart = src->hSyncStart;
    dst->hSyncEnd = src->hSyncEnd;
    dst->hTotal = src->hTotal;
    dst->hSkew = src->hSkew;
    dst->vSyncStart = src->vSyncStart;
    dst->vSyncEnd = src->vSyncEnd;
    dst->vTotal = src->vTotal;
    dst->name = to_ptrv(src->name);
    dst->nameLength = src->nameLength;
    dst->modeFlags = to_ulong(src->modeFlags);
}

void convert_XRRModeInfo_to_64(void* d, const void* s)
{
    my_XRRModeInfo_t *dst = d;
    const my_XRRModeInfo_32_t *src = s;
    dst->modeFlags = from_ulong(src->modeFlags);
    dst->nameLength = src->nameLength;
    dst->name = from_ptrv(src->name);
    dst->vTotal = src->vTotal;
    dst->vSyncEnd = src->vSyncEnd;
    dst->vSyncStart = src->vSyncStart;
    dst->hSkew = src->hSkew;
    dst->hTotal = src->hTotal;
    dst->hSyncEnd = src->hSyncEnd;
    dst->hSyncStart = src->hSyncStart;
    dst->dotClock = from_ulong(src->dotClock);
    dst->height = src->height;
    dst->width = src->width;
    dst->id = from_ulong(src->id);
}

void inplace_XRRScreenResources_shrink(void* s)
{
    if(!s) return;
    my_XRRScreenResources_32_t *dst = s;
    my_XRRScreenResources_t *src = s;
    // shrinking, so forward...
    for(int i=0; i<src->ncrtc; ++i)
        ((XID_32*)src->crtcs)[i] = to_ulong(src->crtcs[i]);
    for(int i=0; i<src->noutput; ++i)
        ((XID_32*)src->outputs)[i] = to_ulong(src->outputs[i]);
    for(int i=0; i<src->nmode; ++i)
        convert_XRRModeInfo_to_32(&((my_XRRModeInfo_32_t*)src->modes)[i], &src->modes[i]);
    dst->timestamp = to_ulong(src->timestamp);
    dst->configTimestamp = to_ulong(src->configTimestamp);
    dst->ncrtc = src->ncrtc;
    dst->crtcs = to_ptrv(src->crtcs);
    dst->noutput = src->noutput;
    dst->outputs = to_ptrv(src->outputs);
    dst->nmode = src->nmode;
    dst->modes = to_ptrv(src->modes);
}

void inplace_XRRScreenResources_enlarge(void* s)
{
    if(!s) return;
    my_XRRScreenResources_t *dst = s;
    my_XRRScreenResources_32_t *src = s;
    // enlarge, so backward...
    int nmode = src->nmode;
    int noutput = src->noutput;
    int ncrtc = src->ncrtc;
    dst->modes = from_ptrv(src->modes);
    dst->nmode = src->nmode;
    dst->outputs = from_ptrv(src->outputs);
    dst->noutput = src->noutput;
    dst->crtcs = from_ptrv(src->crtcs);
    dst->ncrtc = src->ncrtc;
    dst->configTimestamp = to_ulong(src->configTimestamp);
    dst->timestamp = to_ulong(src->timestamp);
    for(int i=nmode-1; i>=0; --i)
        convert_XRRModeInfo_to_64(&dst->modes[i], &((my_XRRModeInfo_32_t*)dst->modes)[i]);
    for(int i=noutput-1; i>=0; --i)
        dst->outputs[i] = from_ulong(((XID_32*)dst->outputs)[i]);
    for(int i=ncrtc-1; i>=0; --i)
        dst->crtcs[i] = from_ulong(((XID_32*)dst->crtcs)[i]);
}

void inplace_XRRCrtcInfo_shrink(void* s)
{
    if(!s) return;
    my_XRRCrtcInfo_32_t *dst = s;
    my_XRRCrtcInfo_t *src = s;
    for(int i=0; i<src->noutput; ++i)
        ((XID_32*)src->outputs)[i] = to_ulong(src->outputs[i]);
    for(int i=0; i<src->npossible; ++i)
        ((XID_32*)src->possible)[i] = to_ulong(src->possible[i]);
    dst->timestamp = to_ulong(src->timestamp);
    dst->x = src->x;
    dst->y = src->y;
    dst->width = src->width;
    dst->height = src->height;
    dst->mode = to_ulong(src->mode);
    dst->rotation = src->rotation;
    dst->noutput = src->noutput;
    dst->outputs = to_ptrv(src->outputs);
    dst->rotations = src->rotations;
    dst->npossible = src->npossible;
    dst->possible = to_ptrv(src->possible);
}

void inplace_XRROutputInfo_shrink(void* s)
{
    if(!s) return;
    my_XRROutputInfo_32_t *dst = s;
    my_XRROutputInfo_t *src = s;
    for(int i=0; i<src->ncrtc; ++i)
        ((XID_32*)src->crtcs)[i] = to_ulong(src->crtcs[i]);
    for(int i=0; i<src->nclone; ++i)
        ((XID_32*)src->clones)[i] = to_ulong(src->clones[i]);
    for(int i=0; i<src->nmode; ++i)
        ((XID_32*)src->modes)[i] = to_ulong(src->modes[i]);
    dst->timestamp = to_ulong(src->timestamp);
    dst->crtc = src->crtc;
    dst->name = to_ptrv(src->name);
    dst->nameLen = src->nameLen;
    dst->mm_width = to_ulong(src->mm_width);
    dst->mm_height = to_ulong(src->mm_height);
    dst->connection = src->connection;
    dst->subpixel_order = src->subpixel_order;
    dst->ncrtc = src->ncrtc;
    dst->crtcs = to_ptrv(src->crtcs);
    dst->nclone = src->nclone;
    dst->clones = to_ptrv(src->clones);
    dst->nmode = src->nmode;
    dst->npreferred = src->npreferred;
    dst->modes = to_ptrv(src->modes);
}

void inplace_XRRProviderInfo_shrink(void* a)
{
    if(!a) return;
    my_XRRProviderInfo_32_t *dst = a;
    my_XRRProviderInfo_t* src = a;

    for(int i=0; i<src->ncrtcs; ++i)
        ((ulong_t*)src->crtcs)[i] = to_ulong(src->crtcs[i]);
    for(int i=0; i<src->noutputs; ++i)
        ((ulong_t*)src->outputs)[i] = to_ulong(src->outputs[i]);
    for(int i=0; i<src->nassociatedproviders; ++i)
        ((ulong_t*)src->associated_providers)[i] = to_ulong(src->associated_providers[i]);
    dst->capabilities = src->capabilities;
    dst->ncrtcs = src->ncrtcs;
    dst->crtcs = to_ptrv(src->crtcs);
    dst->noutputs = src->noutputs;
    dst->outputs = to_ptrv(src->outputs);
    dst->name = to_ptrv(src->name);
    dst->nassociatedproviders = src->nassociatedproviders;
    dst->associated_providers = to_ptrv(src->associated_providers);
    dst->associated_capability = to_ptrv(src->associated_capability);
    dst->nameLen = src->nameLen;
}
void inplace_XRRProviderInfo_enlarge(void* a)
{
    if(!a) return;
    my_XRRProviderInfo_t *dst = a;
    my_XRRProviderInfo_32_t* src = a;

    dst->nameLen = src->nameLen;
    dst->associated_capability = from_ptrv(src->associated_capability);
    dst->associated_providers = from_ptrv(src->associated_providers);
    dst->nassociatedproviders = src->nassociatedproviders;
    dst->name = from_ptrv(src->name);
    dst->outputs = from_ptrv(src->outputs);
    dst->noutputs = src->noutputs;
    dst->crtcs = from_ptrv(src->crtcs);
    dst->ncrtcs = src->ncrtcs;
    dst->capabilities = src->capabilities;
    for(int i=dst->ncrtcs-1; i>=0; --i)
        dst->crtcs[i] = from_ulong(((ulong_t*)dst->crtcs)[i]);
    for(int i=dst->noutputs-1; i>=0; --i)
        dst->outputs[i] = from_ulong(((ulong_t*)dst->outputs)[i]);
    for(int i=dst->nassociatedproviders-1; i>=0; --i)
        dst->associated_providers[i] = from_ulong(((ulong_t*)dst->associated_providers)[i]);
}

void inplace_XRRProviderResources_shrink(void* a)
{
    if(!a) return;
    my_XRRProviderResources_32_t* dst = a;
    my_XRRProviderResources_t* src = a;

    for(int i=0; i<src->nproviders; ++i)
        ((ulong_t*)src->providers)[i] = to_ulong(src->providers[i]);
    dst->timestamp = to_long(src->timestamp);
    dst->nproviders = src->nproviders;
    dst->providers = to_ptrv(src->providers);
}
void inplace_XRRProviderResources_enlarge(void* a)
{
    if(!a) return;
    my_XRRProviderResources_t* dst = a;
    my_XRRProviderResources_32_t* src = a;

    dst->timestamp = from_long(src->timestamp);
    dst->nproviders = src->nproviders;
    dst->providers = from_ptrv(src->providers);
    for(int i=dst->nproviders-1; i>=0; --i)
        dst->providers[i] = from_ulong(((ulong_t*)dst->providers)[i]);
}

void* inplace_XRRPropertyInfo_shrink(void* a)
{
    if(!a) return NULL;
    my_XRRPropertyInfo_t* src = a;
    my_XRRPropertyInfo_32_t* dst = a;

    for(int i=0; i<src->num_values; ++i)
        ((long_t*)src->values)[i] = to_long(src->values[i]);
    dst->pending = src->pending;
    dst->range = src->range;
    dst->immutable = src->immutable;
    dst->num_values = src->num_values;
    dst->values = to_ptrv(src->values);

    return a;
}

void inplace_XIDeviceInfo_shrink(void* a, int n)
{
    if(!a || !n) return;
    my_XIDeviceInfo_32_t* dst = a;
    my_XIDeviceInfo_t* src = a;

    for(int i=0; i<n; ++i, ++src, ++dst) {
        dst->deviceid = src->deviceid;
        dst->name = to_ptrv(src->name);
        dst->use = src->use;
        dst->attachment = src->attachment;
        dst->enabled = src->enabled;
        for(int j=0; j<src->num_classes; ++j)
            ((ptr_t*)src->classes)[j] = to_ptrv(src->classes[j]);
        dst->num_classes = src->num_classes;
        dst->classes = to_ptrv(src->classes);
    }
    // mark last record, even on only 1 record, thos last 2 uint32_t are free
    dst->deviceid = 0;
    dst->name = 0;
}

int inplace_XIDeviceInfo_enlarge(void* a)
{
    if(!a) return 0;
    my_XIDeviceInfo_t* dst = a;
    my_XIDeviceInfo_32_t* src = a;
    int n = 0;
    while(src[n].deviceid && src[n].name) ++n;
    src+=n-1;
    dst+=n-1;
    for(int i=n-1; i>=0; --i, --src, --dst) {
        dst->classes = from_ptrv(src->classes);
        dst->num_classes = src->num_classes;
        dst->enabled = src->enabled;
        dst->attachment = src->attachment;
        dst->use = src->use;
        dst->name = from_ptrv(src->name);
        dst->deviceid = src->deviceid;
        for(int j=dst->num_classes-1; j>=0; --j)
            dst->classes[j] = from_ptrv(((ptr_t*)dst->classes)[j]);
    }
    return n;
}

void* inplace_XDevice_shrink(void* a)
{
    if(!a) return a;
    my_XDevice_t* src = a;
    my_XDevice_32_t* dst = a;

    dst->device_id = to_ulong(src->device_id);
    dst->num_classes = src->num_classes;
    dst->classes = to_ptrv(src->classes);

    return a;
}
void* inplace_XDevice_enlarge(void* a)
{
    if(!a) return a;
    my_XDevice_32_t* src = a;
    my_XDevice_t* dst = a;

    dst->classes = from_ptrv(src->classes);
    dst->num_classes = src->num_classes;
    dst->device_id = from_ulong(src->device_id);

    return a;
}
void* inplace_XDeviceState_shrink(void* a)
{
    if(!a) return a;
    my_XDeviceState_t* src = a;
    my_XDeviceState_32_t* dst = a;

    dst->device_id = to_ulong(src->device_id);
    dst->num_classes = src->num_classes;
    dst->data = to_ptrv(src->data);

    return a;
}
void* inplace_XDeviceState_enlarge(void* a)
{
    if(!a) return a;
    my_XDeviceState_32_t* src = a;
    my_XDeviceState_t* dst = a;

    dst->data = from_ptrv(src->data);
    dst->num_classes = src->num_classes;
    dst->device_id = from_ulong(src->device_id);

    return a;
}

void convert_XShmSegmentInfo_to_32(void* d, void* s)
{
    my_XShmSegmentInfo_t* src = s;
    my_XShmSegmentInfo_32_t* dst = d;
    dst->shmseg = to_ulong(src->shmseg);
    dst->shmid = src->shmid;
    dst->shmaddr = to_ptrv(src->shmaddr);
    dst->readOnly = src->readOnly;
}
void convert_XShmSegmentInfo_to_64(void* d, void* s)
{
    my_XShmSegmentInfo_32_t* src = s;
    my_XShmSegmentInfo_t* dst = d;
    dst->readOnly = src->readOnly;
    dst->shmaddr = from_ptrv(src->shmaddr);
    dst->shmid = src->shmid;
    dst->shmseg = from_ulong(src->shmseg);
}
void* inplace_XkbNamesRec_shrink(void* a)
{
    if(a) {
        my_XkbNamesRec_t* src = a;
        my_XkbNamesRec_32_t* dst = a;

        for(int i=0; i<src->num_rg; ++i)
            src->radio_groups[i] = to_ulong(src->radio_groups[i]);
        dst->keycodes = to_ulong(src->keycodes);
        dst->geometry = to_ulong(src->geometry);
        dst->symbols = to_ulong(src->symbols);
        dst->types = to_ulong(src->types);
        dst->compat = to_ulong(src->compat);
        for(int i=0; i<16; ++i)
            dst->vmods[i] = to_ulong(src->vmods[i]);
        for(int i=0; i<32; ++i)
            dst->indicators[i] = to_ulong(src->indicators[i]);
        for(int i=0; i<4; ++i)
            dst->groups[i] = to_ulong(src->groups[i]);
        dst->keys = to_ptrv(src->keys);
        dst->key_aliases = to_ptrv(src->key_aliases);
        dst->radio_groups = to_ptrv(src->radio_groups);
        dst->phys_symbols = to_ulong(src->phys_symbols);
        dst->num_keys = src->num_keys;
        dst->num_key_aliases = src->num_key_aliases;
        dst->num_rg = src->num_rg;
    }
    return a;
}
void* inplace_XkbNamesRec_enlarge(void* a)
{
    if(a) {
        my_XkbNamesRec_32_t* src = a;
        my_XkbNamesRec_t* dst = a;

        dst->num_rg = src->num_rg;
        dst->num_key_aliases = src->num_key_aliases;
        dst->num_keys = src->num_keys;
        dst->phys_symbols = from_ulong(src->phys_symbols);
        dst->radio_groups = from_ptrv(src->radio_groups);
        dst->key_aliases = from_ptrv(src->key_aliases);
        for(int i=4-1; i>=0; --i)
            dst->groups[i] = from_ulong(src->groups[i]);
        for(int i=32-1; i>=0; --i)
            dst->indicators[i] = from_ulong(src->indicators[i]);
        for(int i=16-1; i>=0; --i)
            dst->vmods[i] = from_ulong(src->vmods[i]);
        dst->compat = from_ulong(src->compat);
        dst->types = from_ulong(src->types);
        dst->symbols = from_ulong(src->symbols);
        dst->geometry = from_ulong(src->geometry);
        dst->keys = from_ptrv(src->keys);
        dst->keycodes = from_ulong(src->keycodes);

        for(int i=src->num_rg-1; i>=0; --i)
            dst->radio_groups[i] = from_ulong(dst->radio_groups[i]);
    }
    return a;
}

void* inplace_XkbClientMapRec_shrink(void* a)
{
    if(a) {
        my_XkbClientMapRec_t* src = a;
        my_XkbClientMapRec_32_t* dst = a;
        dst->size_types = src->size_types;
        dst->num_types = src->num_types;
        dst->types = to_ptrv(src->types);
        dst->size_syms = src->size_syms;
        dst->num_syms = src->num_syms;
        dst->syms = to_ptrv(src->syms);
        dst->key_sym_map = to_ptrv(src->key_sym_map);
        dst->modmap = to_ptrv(src->modmap);
    }
    return a;
}
void* inplace_XkbClientMapRec_enlarge(void* a)
{
    if(a) {
        my_XkbClientMapRec_32_t* src = a;
        my_XkbClientMapRec_t* dst = a;
        dst->modmap = from_ptrv(src->modmap);
        dst->key_sym_map = from_ptrv(src->key_sym_map);
        dst->syms = from_ptrv(src->syms);
        dst->num_syms = src->num_syms;
        dst->size_syms = src->size_syms;
        dst->types = from_ptrv(src->types);
        dst->num_types = src->num_types;
        dst->size_types = src->size_types;
    }
    return a;
}

void* inplace_XkbDescRec_shrink(void* a)
{
    if(a) {
        my_XkbDescRec_t* src = a;
        my_XkbDescRec_32_t* dst = a;

        dst->display = to_ptrv(FindDisplay(src->display));
        dst->flags = src->flags;
        dst->device_spec = src->device_spec;
        dst->min_key_code = src->min_key_code;
        dst->max_key_code = src->max_key_code;
        dst->ctrls = to_ptrv(src->ctrls);
        dst->server = to_ptrv(src->server);
        dst->map = to_ptrv(inplace_XkbClientMapRec_shrink(src->map));
        dst->indicators = to_ptrv(src->indicators);
        dst->names = to_ptrv(inplace_XkbNamesRec_shrink(src->names));
        dst->compat = to_ptrv(src->compat);
        dst->geom = to_ptrv(src->geom);
    }
    return a;
}
void* inplace_XkbDescRec_enlarge(void* a)
{
    if(a) {
        my_XkbDescRec_32_t* src = a;
        my_XkbDescRec_t* dst = a;

        dst->geom = from_ptrv(src->geom);
        dst->compat = from_ptrv(src->compat);
        dst->names = inplace_XkbNamesRec_enlarge(from_ptrv(src->names));
        dst->indicators = from_ptrv(src->indicators);
        dst->map = inplace_XkbClientMapRec_enlarge(from_ptrv(src->map));
        dst->server = from_ptrv(src->server);
        dst->ctrls = from_ptrv(src->ctrls);
        dst->max_key_code = src->max_key_code;
        dst->min_key_code = src->min_key_code;
        dst->device_spec = src->device_spec;
        dst->flags = src->flags;
        dst->display = getDisplay(from_ptrv(src->display));
    }
    return a;
}

void convert_XAnyClassInfo_to_32(void* d, void* s)
{
    if(!d || !s) return;
    my_XAnyClassInfo_t* src = s;
    my_XAnyClassInfo_32_t* dst = d;
    int len = src->length;
    switch(src->c_class) {
        case 2: //ValuatorClass
            {
                my_XValuatorInfo_t* src = s;
                my_XValuatorInfo_32_t* dst = d;
                dst->c_class = to_ulong(src->c_class);
                dst->length = src->length;
                dst->num_axes = src->num_axes;
                dst->mode = src->mode;
                dst->motion_buffer = to_ulong(src->motion_buffer);
                dst->axes = to_ptrv(src->axes);
            }
            break;
        case 0: //KeyClass
        case 1: //ButtonClass
        case 3: //FeedbackClass
        case 4: //ProximityClass
        case 5: //FocusClass
        case 6: //OtherClass
        case 7: //AttachClass
        default:
            dst->c_class = to_ulong(src->c_class);
            memmove(d+4, s+8, len-4);
    }

}
void convert_XAnyClassInfo_to_64(void* d, void* s)
{
    if(!d || !s) return;
    my_XAnyClassInfo_32_t* src = s;
    my_XAnyClassInfo_t* dst = d;

    int len = src->length;
    switch(src->c_class) {
        case 2: //ValuatorClass
            {
                my_XValuatorInfo_32_t* src = s;
                my_XValuatorInfo_t* dst = d;
                dst->axes = from_ptrv(src->axes);
                dst->motion_buffer = from_ulong(src->motion_buffer);
                dst->mode = src->mode;
                dst->num_axes = src->num_axes;
                dst->length = src->length;
                dst->c_class = from_ulong(src->c_class);
            }
            break;
        case 0: //KeyClass
        case 1: //ButtonClass
        case 3: //FeedbackClass
        case 4: //ProximityClass
        case 5: //FocusClass
        case 6: //OtherClass
        case 7: //AttachClass
        default:
            memmove(d+8, s+4, len-4);
            dst->c_class = from_ulong(src->c_class);
    }
}
void* inplace_XAnyClassInfo_shrink(void* a)
{
    if(a)
        convert_XAnyClassInfo_to_32(a, a);
    return a;
}
void* inplace_XAnyClassInfo_enlarge(void* a)
{
    if(a)
        convert_XAnyClassInfo_to_64(a, a);
    return a;
}

void* inplace_XDeviceInfo_shrink(void* a, int n)
{
    if(a) {
        my_XDeviceInfo_t* src = a;
        my_XDeviceInfo_32_t* dst = a;

        for(int i=0; i<n; ++i, ++src, ++dst) {
            void* p = src->inputclassinfo;
            for(int j=0; j<src->num_classes; ++j) {
                my_XAnyClassInfo_32_t* dst_c = p;
                my_XAnyClassInfo_t* src_c = p;
                int len = src_c->length;
                convert_XAnyClassInfo_to_32(dst_c, src_c);
                p += len;
            }
            dst->id = to_ulong(src->id);
            dst->type = to_ulong(src->type);
            dst->name = to_ptrv(src->name);
            dst->num_classes = src->num_classes;
            dst->use = src->use;
            dst->inputclassinfo = to_ptrv(src->inputclassinfo);
        }
        // mark last record, even on only 1 record, thos last 2 uint32_t are free
        dst->inputclassinfo = 0;
        dst->name = 0;

    }
    return a;
}
void* inplace_XDeviceInfo_enlarge(void* a)
{
    if(a) {
        my_XDeviceInfo_32_t* src = a;
        my_XDeviceInfo_t* dst = a;
        int n = 0;
        while(src[n].inputclassinfo && src[n].name) ++n;
        src+=n-1;
        dst+=n-1;
        for(int i=n-1; i>=0; --i, --src, --dst) {
            dst->inputclassinfo = from_ptrv(src->inputclassinfo);
            dst->use = src->use;
            dst->num_classes = src->num_classes;
            dst->name = from_ptrv(src->name);
            dst->type = from_ulong(src->type);
            dst->id = from_ulong(src->id);
            for(int j=dst->num_classes-1; j>=0; --j) {
                void* p = dst->inputclassinfo;
                my_XAnyClassInfo_t* dst_c = p;
                my_XAnyClassInfo_32_t* src_c = p;
                for(int k=0; k<j; ++k) {
                    p+=src_c->length;
                    dst_c = p;
                    src_c = p;
                }
                convert_XAnyClassInfo_to_64(dst_c, src_c);
            }
        }
    }
    return a;
}


void* inplace_XFilters_shrink(void* a)
{
    if(a) {
        my_XFilters_t* src = a;
        my_XFilters_32_t* dst = a;
        dst->nfilter = src->nfilter;
        char** filter = src->filter;
        dst->filter = to_ptrv(filter);
        dst->nalias = src->nalias;
        dst->alias = to_ptrv(src->alias);
        for(int i=0; i<dst->nfilter; ++i)
            ((ptr_t*)(filter))[i] = to_ptrv(filter[i]);
    }
    return a;
}
void* inplace_XFilters_enlarge(void* a)
{
    if(a) {
        my_XFilters_t* dst = a;
        my_XFilters_32_t* src = a;
        dst->alias = from_ptrv(src->alias);
        dst->nalias = src->nalias;
        dst->filter = from_ptrv(src->filter);
        dst->nfilter = src->nfilter;
        char** filter = dst->filter;
        for(int i=dst->nfilter-1; i>=0; --i)
            filter[i] = from_ptrv(((ptr_t*)(filter))[i]);
    }
    return a;
}

void* inplace_XRRMonitorInfo_shrink(void* a, int n)
{
    if(a) {
        my_XRRMonitorInfo_t* src = a;
        my_XRRMonitorInfo_32_t* dst = a;
        for(int i=0; i<n; ++i) {
            for(int j=0; j<src->noutput; ++j)
                ((ulong_t*)src->outputs)[j] = to_ulong(src->outputs[j]);
            dst->name = to_ulong(src->name);
            dst->primary = src->primary;
            dst->automatic = src->automatic;
            dst->noutput = src->noutput;
            dst->x = src->x;
            dst->y = src->y;
            dst->width = src->width;
            dst->height = src->height;
            dst->mwidth = src->mwidth;
            dst->mheight = src->mheight;
            dst->outputs = to_ptrv(src->outputs);
            src++;
            dst++;
        }
        src->name = 0;  // mark the last record...
    }
    return a;
}

void* inplace_XRRMonitorInfo_enlarge(void* a, int n)
{
    if(a) {
        my_XRRMonitorInfo_32_t* src = a;
        my_XRRMonitorInfo_t* dst = a;
        for(int i=0; i<n; ++i) {
            dst->name = from_ulong(src->name);
            dst->primary = src->primary;
            dst->automatic = src->automatic;
            dst->noutput = src->noutput;
            dst->x = src->x;
            dst->y = src->y;
            dst->width = src->width;
            dst->height = src->height;
            dst->mwidth = src->mwidth;
            dst->mheight = src->mheight;
            dst->outputs = from_ptrv(src->outputs);
            for(int j=dst->noutput-1; j>=0; --j)
                ((unsigned long*)dst->outputs)[j] = from_ulong(dst->outputs[j]);
            src++;
            dst++;
        }
    }
    return a;
}

void* inplace_XRRCrtcTransformAttributes_shrink(void* a)
{
    if(a) {
        my_XRRCrtcTransformAttributes_t* src = a;
        my_XRRCrtcTransformAttributes_32_t* dst = a;
        dst->pendingTransform = src->pendingTransform;
        dst->pendingFilter = to_ptrv(src->pendingFilter);
        dst->pendingNparams = src->pendingNparams;
        dst->pendingParams = to_ptrv(src->pendingParams);
        dst->currentTransform = src->currentTransform;
        dst->currentFilter = to_ptrv(src->currentFilter);
        dst->currentNparams = src->currentNparams;
        dst->currentParams = to_ptrv(src->currentParams);
    }
    return a;
}