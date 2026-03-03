#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "box32context.h"
#include "myalign32.h"

static const char* liboggName = "libogg.so.0";
#define ALTNAME "libogg.so"

#define LIBNAME libogg

#define ADDED_FUNCTIONS()                   \

#include "generated/wrappedliboggtypes32.h"

#include "wrappercallback32.h"

// ============================================================================
// Native (64-bit) struct definitions matching <ogg/ogg.h>
// ============================================================================

typedef struct {
    unsigned char *body_data;
    long    body_storage;
    long    body_fill;
    long    body_returned;
    int     *lacing_vals;
    int64_t *granule_vals;
    long    lacing_storage;
    long    lacing_fill;
    long    lacing_packet;
    long    lacing_returned;
    unsigned char header[282];
    int     header_fill;
    int     e_o_s;
    int     b_o_s;
    long    serialno;
    long    pageno;
    int64_t packetno;
    int64_t granulepos;
} my_ogg_stream_state_t;

typedef struct {
    unsigned char *data;
    int     storage;
    int     fill;
    int     returned;
    int     unsynced;
    int     headerbytes;
    int     bodybytes;
} my_ogg_sync_state_t;

typedef struct {
    unsigned char *header;
    long    header_len;
    unsigned char *body;
    long    body_len;
} my_ogg_page_t;

typedef struct {
    unsigned char *packet;
    long    bytes;
    long    b_o_s;
    long    e_o_s;
    int64_t granulepos;
    int64_t packetno;
} my_ogg_packet_t;

typedef struct {
    long    endbyte;
    int     endbit;
    unsigned char *buffer;
    unsigned char *ptr;
    long    storage;
} my_oggpack_buffer_t;

// ============================================================================
// 32-bit struct definitions (i386 layout)
// ============================================================================

typedef struct {
    ptr_t   body_data;          // unsigned char*
    long_t  body_storage;
    long_t  body_fill;
    long_t  body_returned;
    ptr_t   lacing_vals;        // int*
    ptr_t   granule_vals;       // int64_t*
    long_t  lacing_storage;
    long_t  lacing_fill;
    long_t  lacing_packet;
    long_t  lacing_returned;
    unsigned char header[282];
    int     header_fill;
    int     e_o_s;
    int     b_o_s;
    long_t  serialno;
    long_t  pageno;
    int64_t packetno;
    int64_t granulepos;
} my_ogg_stream_state_32_t;

typedef struct {
    ptr_t   data;               // unsigned char*
    int     storage;
    int     fill;
    int     returned;
    int     unsynced;
    int     headerbytes;
    int     bodybytes;
} my_ogg_sync_state_32_t;

typedef struct {
    ptr_t   header;             // unsigned char*
    long_t  header_len;
    ptr_t   body;               // unsigned char*
    long_t  body_len;
} my_ogg_page_32_t;

typedef struct {
    ptr_t   packet;             // unsigned char*
    long_t  bytes;
    long_t  b_o_s;
    long_t  e_o_s;
    int64_t granulepos;
    int64_t packetno;
} my_ogg_packet_32_t;

typedef struct {
    long_t  endbyte;
    int     endbit;
    ptr_t   buffer;             // unsigned char*
    ptr_t   ptr;                // unsigned char*
    long_t  storage;
} my_oggpack_buffer_32_t;

// ============================================================================
// Shadow struct management for ogg_stream_state using khash
// ============================================================================

KHASH_MAP_INIT_INT(streamstate, my_ogg_stream_state_t*);
static kh_streamstate_t* stream_shadows = NULL;

KHASH_MAP_INIT_INT(syncstate, my_ogg_sync_state_t*);
static kh_syncstate_t* sync_shadows = NULL;

// Get or create a native shadow for a 32-bit ogg_stream_state
static my_ogg_stream_state_t* getStreamShadow(void* a)
{
    if(!a) return NULL;
    ptr_t key = to_ptrv(a);
    khint_t k = kh_get(streamstate, stream_shadows, key);
    if(k == kh_end(stream_shadows)) {
        int r;
        k = kh_put(streamstate, stream_shadows, key, &r);
        kh_value(stream_shadows, k) = (my_ogg_stream_state_t*)calloc(1, sizeof(my_ogg_stream_state_t));
    }
    return kh_value(stream_shadows, k);
}

static void delStreamShadow(void* a)
{
    if(!a) return;
    ptr_t key = to_ptrv(a);
    khint_t k = kh_get(streamstate, stream_shadows, key);
    if(k != kh_end(stream_shadows)) {
        free(kh_value(stream_shadows, k));
        kh_del(streamstate, stream_shadows, k);
    }
}

// Get or create a native shadow for a 32-bit ogg_sync_state
static my_ogg_sync_state_t* getSyncShadow(void* a)
{
    if(!a) return NULL;
    ptr_t key = to_ptrv(a);
    khint_t k = kh_get(syncstate, sync_shadows, key);
    if(k == kh_end(sync_shadows)) {
        int r;
        k = kh_put(syncstate, sync_shadows, key, &r);
        kh_value(sync_shadows, k) = (my_ogg_sync_state_t*)calloc(1, sizeof(my_ogg_sync_state_t));
    }
    return kh_value(sync_shadows, k);
}

static void delSyncShadow(void* a)
{
    if(!a) return;
    ptr_t key = to_ptrv(a);
    khint_t k = kh_get(syncstate, sync_shadows, key);
    if(k != kh_end(sync_shadows)) {
        free(kh_value(sync_shadows, k));
        kh_del(syncstate, sync_shadows, k);
    }
}

// ============================================================================
// Conversion: ogg_stream_state native <-> 32-bit
// The native shadow is the authoritative copy. We sync back to 32-bit
// after each call so the app can read fields like serialno, pageno, etc.
// ============================================================================

static void ogg_stream_state_to_32(my_ogg_stream_state_32_t* dst, const my_ogg_stream_state_t* src)
{
    dst->body_data = to_ptrv(src->body_data);
    dst->body_storage = to_long(src->body_storage);
    dst->body_fill = to_long(src->body_fill);
    dst->body_returned = to_long(src->body_returned);
    dst->lacing_vals = to_ptrv(src->lacing_vals);
    dst->granule_vals = to_ptrv(src->granule_vals);
    dst->lacing_storage = to_long(src->lacing_storage);
    dst->lacing_fill = to_long(src->lacing_fill);
    dst->lacing_packet = to_long(src->lacing_packet);
    dst->lacing_returned = to_long(src->lacing_returned);
    memcpy(dst->header, src->header, 282);
    dst->header_fill = src->header_fill;
    dst->e_o_s = src->e_o_s;
    dst->b_o_s = src->b_o_s;
    dst->serialno = to_long(src->serialno);
    dst->pageno = to_long(src->pageno);
    dst->packetno = src->packetno;
    dst->granulepos = src->granulepos;
}

// ============================================================================
// Conversion: ogg_sync_state native <-> 32-bit
// ============================================================================

static void ogg_sync_state_to_32(my_ogg_sync_state_32_t* dst, const my_ogg_sync_state_t* src)
{
    dst->data = to_ptrv(src->data);
    dst->storage = src->storage;
    dst->fill = src->fill;
    dst->returned = src->returned;
    dst->unsynced = src->unsynced;
    dst->headerbytes = src->headerbytes;
    dst->bodybytes = src->bodybytes;
}

// ============================================================================
// Conversion: ogg_page native -> 32-bit and 32-bit -> native
// ============================================================================

static void ogg_page_to_native(my_ogg_page_t* dst, const my_ogg_page_32_t* src)
{
    dst->header = from_ptrv(src->header);
    dst->header_len = from_long(src->header_len);
    dst->body = from_ptrv(src->body);
    dst->body_len = from_long(src->body_len);
}

static void ogg_page_to_32(my_ogg_page_32_t* dst, const my_ogg_page_t* src)
{
    dst->header = to_ptrv(src->header);
    dst->header_len = to_long(src->header_len);
    dst->body = to_ptrv(src->body);
    dst->body_len = to_long(src->body_len);
}

// ============================================================================
// Conversion: ogg_packet native <-> 32-bit
// ============================================================================

static void ogg_packet_to_native(my_ogg_packet_t* dst, const my_ogg_packet_32_t* src)
{
    dst->packet = from_ptrv(src->packet);
    dst->bytes = from_long(src->bytes);
    dst->b_o_s = from_long(src->b_o_s);
    dst->e_o_s = from_long(src->e_o_s);
    dst->granulepos = src->granulepos;
    dst->packetno = src->packetno;
}

static void ogg_packet_to_32(my_ogg_packet_32_t* dst, const my_ogg_packet_t* src)
{
    dst->packet = to_ptrv(src->packet);
    dst->bytes = to_long(src->bytes);
    dst->b_o_s = to_long(src->b_o_s);
    dst->e_o_s = to_long(src->e_o_s);
    dst->granulepos = src->granulepos;
    dst->packetno = src->packetno;
}

// ============================================================================
// Conversion: oggpack_buffer native <-> 32-bit
// ============================================================================

static void oggpack_buffer_to_native(my_oggpack_buffer_t* dst, const my_oggpack_buffer_32_t* src)
{
    dst->endbyte = from_long(src->endbyte);
    dst->endbit = src->endbit;
    dst->buffer = from_ptrv(src->buffer);
    dst->ptr = from_ptrv(src->ptr);
    dst->storage = from_long(src->storage);
}

static void oggpack_buffer_to_32(my_oggpack_buffer_32_t* dst, const my_oggpack_buffer_t* src)
{
    dst->endbyte = to_long(src->endbyte);
    dst->endbit = src->endbit;
    dst->buffer = to_ptrv(src->buffer);
    dst->ptr = to_ptrv(src->ptr);
    dst->storage = to_long(src->storage);
}

// ============================================================================
// ogg_stream_* wrappers
// ============================================================================

EXPORT int my32_ogg_stream_init(x64emu_t* emu, void* os, int serialno)
{
    my_ogg_stream_state_t* shadow = getStreamShadow(os);
    int ret = my->ogg_stream_init(shadow, serialno);
    ogg_stream_state_to_32(os, shadow);
    return ret;
}

EXPORT int my32_ogg_stream_clear(x64emu_t* emu, void* os)
{
    my_ogg_stream_state_t* shadow = getStreamShadow(os);
    int ret = my->ogg_stream_clear(shadow);
    // After clear, the struct is zeroed; sync back and remove shadow
    ogg_stream_state_to_32(os, shadow);
    delStreamShadow(os);
    return ret;
}

EXPORT int my32_ogg_stream_reset(x64emu_t* emu, void* os)
{
    my_ogg_stream_state_t* shadow = getStreamShadow(os);
    int ret = my->ogg_stream_reset(shadow);
    ogg_stream_state_to_32(os, shadow);
    return ret;
}

EXPORT int my32_ogg_stream_reset_serialno(x64emu_t* emu, void* os, int serialno)
{
    my_ogg_stream_state_t* shadow = getStreamShadow(os);
    int ret = my->ogg_stream_reset_serialno(shadow, serialno);
    ogg_stream_state_to_32(os, shadow);
    return ret;
}

EXPORT int my32_ogg_stream_packetin(x64emu_t* emu, void* os, void* op)
{
    my_ogg_stream_state_t* shadow = getStreamShadow(os);
    my_ogg_packet_t op_l;
    ogg_packet_to_native(&op_l, op);
    int ret = my->ogg_stream_packetin(shadow, &op_l);
    ogg_stream_state_to_32(os, shadow);
    // ogg_stream_packetin doesn't modify the packet
    return ret;
}

EXPORT int my32_ogg_stream_pageout(x64emu_t* emu, void* os, void* og)
{
    my_ogg_stream_state_t* shadow = getStreamShadow(os);
    my_ogg_page_t og_l = {0};
    int ret = my->ogg_stream_pageout(shadow, &og_l);
    ogg_stream_state_to_32(os, shadow);
    if(og) ogg_page_to_32(og, &og_l);
    return ret;
}

EXPORT int my32_ogg_stream_pageout_fill(x64emu_t* emu, void* os, void* og, int nfill)
{
    my_ogg_stream_state_t* shadow = getStreamShadow(os);
    my_ogg_page_t og_l = {0};
    int ret = my->ogg_stream_pageout_fill(shadow, &og_l, nfill);
    ogg_stream_state_to_32(os, shadow);
    if(og) ogg_page_to_32(og, &og_l);
    return ret;
}

EXPORT int my32_ogg_stream_flush(x64emu_t* emu, void* os, void* og)
{
    my_ogg_stream_state_t* shadow = getStreamShadow(os);
    my_ogg_page_t og_l = {0};
    int ret = my->ogg_stream_flush(shadow, &og_l);
    ogg_stream_state_to_32(os, shadow);
    if(og) ogg_page_to_32(og, &og_l);
    return ret;
}

EXPORT int my32_ogg_stream_flush_fill(x64emu_t* emu, void* os, void* og, int nfill)
{
    my_ogg_stream_state_t* shadow = getStreamShadow(os);
    my_ogg_page_t og_l = {0};
    int ret = my->ogg_stream_flush_fill(shadow, &og_l, nfill);
    ogg_stream_state_to_32(os, shadow);
    if(og) ogg_page_to_32(og, &og_l);
    return ret;
}

EXPORT int my32_ogg_stream_pagein(x64emu_t* emu, void* os, void* og)
{
    my_ogg_stream_state_t* shadow = getStreamShadow(os);
    my_ogg_page_t og_l;
    ogg_page_to_native(&og_l, og);
    int ret = my->ogg_stream_pagein(shadow, &og_l);
    ogg_stream_state_to_32(os, shadow);
    return ret;
}

EXPORT int my32_ogg_stream_packetout(x64emu_t* emu, void* os, void* op)
{
    my_ogg_stream_state_t* shadow = getStreamShadow(os);
    my_ogg_packet_t op_l = {0};
    int ret = my->ogg_stream_packetout(shadow, op?(&op_l):NULL);
    ogg_stream_state_to_32(os, shadow);
    if(op) ogg_packet_to_32(op, &op_l);
    return ret;
}

EXPORT int my32_ogg_stream_packetpeek(x64emu_t* emu, void* os, void* op)
{
    my_ogg_stream_state_t* shadow = getStreamShadow(os);
    my_ogg_packet_t op_l = {0};
    int ret = my->ogg_stream_packetpeek(shadow, op?(&op_l):NULL);
    ogg_stream_state_to_32(os, shadow);
    if(op) ogg_packet_to_32(op, &op_l);
    return ret;
}

// ============================================================================
// ogg_sync_* wrappers
// ============================================================================

EXPORT int my32_ogg_sync_init(x64emu_t* emu, void* oy)
{
    my_ogg_sync_state_t* shadow = getSyncShadow(oy);
    int ret = my->ogg_sync_init(shadow);
    ogg_sync_state_to_32(oy, shadow);
    return ret;
}

EXPORT int my32_ogg_sync_clear(x64emu_t* emu, void* oy)
{
    my_ogg_sync_state_t* shadow = getSyncShadow(oy);
    int ret = my->ogg_sync_clear(shadow);
    ogg_sync_state_to_32(oy, shadow);
    delSyncShadow(oy);
    return ret;
}

EXPORT int my32_ogg_sync_reset(x64emu_t* emu, void* oy)
{
    my_ogg_sync_state_t* shadow = getSyncShadow(oy);
    int ret = my->ogg_sync_reset(shadow);
    ogg_sync_state_to_32(oy, shadow);
    return ret;
}

EXPORT void* my32_ogg_sync_buffer(x64emu_t* emu, void* oy, int size)
{
    my_ogg_sync_state_t* shadow = getSyncShadow(oy);
    void* ret = my->ogg_sync_buffer(shadow, size);
    ogg_sync_state_to_32(oy, shadow);
    return ret;
}

EXPORT int my32_ogg_sync_wrote(x64emu_t* emu, void* oy, int bytes)
{
    my_ogg_sync_state_t* shadow = getSyncShadow(oy);
    int ret = my->ogg_sync_wrote(shadow, bytes);
    ogg_sync_state_to_32(oy, shadow);
    return ret;
}

EXPORT int my32_ogg_sync_pageout(x64emu_t* emu, void* oy, void* og)
{
    my_ogg_sync_state_t* shadow = getSyncShadow(oy);
    my_ogg_page_t og_l = {0};
    int ret = my->ogg_sync_pageout(shadow, &og_l);
    ogg_sync_state_to_32(oy, shadow);
    if(og) ogg_page_to_32(og, &og_l);
    return ret;
}

EXPORT int my32_ogg_sync_pageseek(x64emu_t* emu, void* oy, void* og)
{
    my_ogg_sync_state_t* shadow = getSyncShadow(oy);
    my_ogg_page_t og_l = {0};
    int ret = my->ogg_sync_pageseek(shadow, og?(&og_l):NULL);
    ogg_sync_state_to_32(oy, shadow);
    if(og) ogg_page_to_32(og, &og_l);
    return ret;
}

// ============================================================================
// ogg_page_* wrappers (read-only accessors, page is ephemeral)
// ============================================================================

EXPORT int my32_ogg_page_bos(x64emu_t* emu, void* og)
{
    my_ogg_page_t og_l;
    ogg_page_to_native(&og_l, og);
    return my->ogg_page_bos(&og_l);
}

EXPORT int my32_ogg_page_eos(x64emu_t* emu, void* og)
{
    my_ogg_page_t og_l;
    ogg_page_to_native(&og_l, og);
    return my->ogg_page_eos(&og_l);
}

EXPORT int my32_ogg_page_continued(x64emu_t* emu, void* og)
{
    my_ogg_page_t og_l;
    ogg_page_to_native(&og_l, og);
    return my->ogg_page_continued(&og_l);
}

EXPORT int my32_ogg_page_checksum_set(x64emu_t* emu, void* og)
{
    my_ogg_page_t og_l;
    ogg_page_to_native(&og_l, og);
    // checksum_set modifies the page header in-place (the header data itself, not the struct)
    my->ogg_page_checksum_set(&og_l);
    // pointers haven't changed, no need to convert back
    return 0;
}

EXPORT int64_t my32_ogg_page_granulepos(x64emu_t* emu, void* og)
{
    my_ogg_page_t og_l;
    ogg_page_to_native(&og_l, og);
    return my->ogg_page_granulepos(&og_l);
}

EXPORT int my32_ogg_page_packets(x64emu_t* emu, void* og)
{
    my_ogg_page_t og_l;
    ogg_page_to_native(&og_l, og);
    return my->ogg_page_packets(&og_l);
}

EXPORT long my32_ogg_page_pageno(x64emu_t* emu, void* og)
{
    my_ogg_page_t og_l;
    ogg_page_to_native(&og_l, og);
    return my->ogg_page_pageno(&og_l);
}

EXPORT int my32_ogg_page_serialno(x64emu_t* emu, void* og)
{
    my_ogg_page_t og_l;
    ogg_page_to_native(&og_l, og);
    return my->ogg_page_serialno(&og_l);
}

// ============================================================================
// oggpack_* wrappers (bitpacking functions)
// ============================================================================

EXPORT void my32_oggpack_writeinit(x64emu_t* emu, void* b)
{
    my_oggpack_buffer_t b_l = {0};
    my->oggpack_writeinit(&b_l);
    oggpack_buffer_to_32(b, &b_l);
}

EXPORT int my32_oggpack_writecheck(x64emu_t* emu, void* b)
{
    my_oggpack_buffer_t b_l;
    oggpack_buffer_to_native(&b_l, b);
    int ret = my->oggpack_writecheck(&b_l);
    oggpack_buffer_to_32(b, &b_l);
    return ret;
}

EXPORT void my32_oggpack_reset(x64emu_t* emu, void* b)
{
    my_oggpack_buffer_t b_l;
    oggpack_buffer_to_native(&b_l, b);
    my->oggpack_reset(&b_l);
    oggpack_buffer_to_32(b, &b_l);
}

EXPORT void my32_oggpack_writeclear(x64emu_t* emu, void* b)
{
    my_oggpack_buffer_t b_l;
    oggpack_buffer_to_native(&b_l, b);
    my->oggpack_writeclear(&b_l);
    oggpack_buffer_to_32(b, &b_l);
}

EXPORT void my32_oggpack_readinit(x64emu_t* emu, void* b, void* buf, int bytes)
{
    my_oggpack_buffer_t b_l = {0};
    my->oggpack_readinit(&b_l, buf, bytes);
    oggpack_buffer_to_32(b, &b_l);
}

EXPORT void my32_oggpack_write(x64emu_t* emu, void* b, unsigned long value, int bits)
{
    my_oggpack_buffer_t b_l;
    oggpack_buffer_to_native(&b_l, b);
    my->oggpack_write(&b_l, value, bits);
    oggpack_buffer_to_32(b, &b_l);
}

EXPORT long my32_oggpack_look(x64emu_t* emu, void* b, int bits)
{
    my_oggpack_buffer_t b_l;
    oggpack_buffer_to_native(&b_l, b);
    return my->oggpack_look(&b_l, bits);
}

EXPORT long my32_oggpack_read(x64emu_t* emu, void* b, int bits)
{
    my_oggpack_buffer_t b_l;
    oggpack_buffer_to_native(&b_l, b);
    long ret = my->oggpack_read(&b_l, bits);
    oggpack_buffer_to_32(b, &b_l);
    return ret;
}

EXPORT void my32_oggpack_adv(x64emu_t* emu, void* b, int bits)
{
    my_oggpack_buffer_t b_l;
    oggpack_buffer_to_native(&b_l, b);
    my->oggpack_adv(&b_l, bits);
    oggpack_buffer_to_32(b, &b_l);
}

EXPORT long my32_oggpack_bytes(x64emu_t* emu, void* b)
{
    my_oggpack_buffer_t b_l;
    oggpack_buffer_to_native(&b_l, b);
    return my->oggpack_bytes(&b_l);
}

EXPORT void* my32_oggpack_get_buffer(x64emu_t* emu, void* b)
{
    my_oggpack_buffer_t b_l;
    oggpack_buffer_to_native(&b_l, b);
    return my->oggpack_get_buffer(&b_l);
}

EXPORT void my32_oggpack_writetrunc(x64emu_t* emu, void* b, long bits)
{
    my_oggpack_buffer_t b_l;
    oggpack_buffer_to_native(&b_l, b);
    my->oggpack_writetrunc(&b_l, bits);
    oggpack_buffer_to_32(b, &b_l);
}

// ============================================================================
// oggpackB_* wrappers (MSb bitpacking functions)
// ============================================================================

EXPORT void my32_oggpackB_writeinit(x64emu_t* emu, void* b)
{
    my_oggpack_buffer_t b_l = {0};
    my->oggpackB_writeinit(&b_l);
    oggpack_buffer_to_32(b, &b_l);
}

EXPORT int my32_oggpackB_writecheck(x64emu_t* emu, void* b)
{
    my_oggpack_buffer_t b_l;
    oggpack_buffer_to_native(&b_l, b);
    int ret = my->oggpackB_writecheck(&b_l);
    oggpack_buffer_to_32(b, &b_l);
    return ret;
}

EXPORT void my32_oggpackB_reset(x64emu_t* emu, void* b)
{
    my_oggpack_buffer_t b_l;
    oggpack_buffer_to_native(&b_l, b);
    my->oggpackB_reset(&b_l);
    oggpack_buffer_to_32(b, &b_l);
}

EXPORT void my32_oggpackB_writeclear(x64emu_t* emu, void* b)
{
    my_oggpack_buffer_t b_l;
    oggpack_buffer_to_native(&b_l, b);
    my->oggpackB_writeclear(&b_l);
    oggpack_buffer_to_32(b, &b_l);
}

EXPORT void my32_oggpackB_readinit(x64emu_t* emu, void* b, void* buf, int bytes)
{
    my_oggpack_buffer_t b_l = {0};
    my->oggpackB_readinit(&b_l, buf, bytes);
    oggpack_buffer_to_32(b, &b_l);
}

EXPORT void my32_oggpackB_write(x64emu_t* emu, void* b, unsigned long value, int bits)
{
    my_oggpack_buffer_t b_l;
    oggpack_buffer_to_native(&b_l, b);
    my->oggpackB_write(&b_l, value, bits);
    oggpack_buffer_to_32(b, &b_l);
}

EXPORT long my32_oggpackB_read(x64emu_t* emu, void* b, int bits)
{
    my_oggpack_buffer_t b_l;
    oggpack_buffer_to_native(&b_l, b);
    long ret = my->oggpackB_read(&b_l, bits);
    oggpack_buffer_to_32(b, &b_l);
    return ret;
}

EXPORT int my32_oggpackB_bytes(x64emu_t* emu, void* b)
{
    my_oggpack_buffer_t b_l;
    oggpack_buffer_to_native(&b_l, b);
    return my->oggpackB_bytes(&b_l);
}

EXPORT void* my32_oggpackB_get_buffer(x64emu_t* emu, void* b)
{
    my_oggpack_buffer_t b_l;
    oggpack_buffer_to_native(&b_l, b);
    return my->oggpackB_get_buffer(&b_l);
}

// ============================================================================
// Init / Fini
// ============================================================================

#define CUSTOM_INIT \
    stream_shadows = kh_init(streamstate);  \
    sync_shadows = kh_init(syncstate);

#define CUSTOM_FINI \
    my_ogg_stream_state_t* ss;                                  \
    kh_foreach_value(stream_shadows, ss, free(ss));             \
    kh_destroy(streamstate, stream_shadows);                    \
    stream_shadows = NULL;                                      \
    my_ogg_sync_state_t* sy;                                    \
    kh_foreach_value(sync_shadows, sy, free(sy));               \
    kh_destroy(syncstate, sync_shadows);                        \
    sync_shadows = NULL;

#include "wrappedlib_init32.h"
