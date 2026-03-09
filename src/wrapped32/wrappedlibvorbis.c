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

static const char* libvorbisName = "libvorbis.so.0";
#define ALTNAME "libvorbis.so"

#define LIBNAME libvorbis

#define ADDED_FUNCTIONS()                   \

#include "generated/wrappedlibvorbistypes32.h"

#include "wrappercallback32.h"

// ============================================================================
// Native (64-bit) struct definitions matching <vorbis/codec.h>
// ============================================================================

typedef struct {
    int      version;
    int      channels;
    long     rate;
    long     bitrate_upper;
    long     bitrate_nominal;
    long     bitrate_lower;
    long     bitrate_window;
    void    *codec_setup;
} my_vorbis_info_t;

typedef struct {
    char   **user_comments;
    int     *comment_lengths;
    int      comments;
    char    *vendor;
} my_vorbis_comment_t;

typedef struct {
    int      analysisp;
    void    *vi;            // vorbis_info*
    float  **pcm;
    float  **pcmret;
    int      pcm_storage;
    int      pcm_current;
    int      pcm_returned;
    int      preextrapolate;
    int      eofflag;
    long     lW;
    long     W;
    long     nW;
    long     centerW;
    int64_t  granulepos;
    int64_t  sequence;
    int64_t  glue_bits;
    int64_t  time_bits;
    int64_t  floor_bits;
    int64_t  res_bits;
    void    *backend_state;
    ptr_t    pcm32[256];    // buffer for converting native float** to ptr_t[] for 32-bit code
} my_vorbis_dsp_state_t;

typedef struct {
    long             endbyte;
    int              endbit;
    unsigned char   *buffer;
    unsigned char   *ptr;
    long             storage;
} my_oggpack_buffer_t;

typedef struct {
    float          **pcm;
    my_oggpack_buffer_t opb;
    long             lW;
    long             W;
    long             nW;
    int              pcmend;
    int              mode;
    int              eofflag;
    int64_t          granulepos;
    int64_t          sequence;
    void            *vd;            // vorbis_dsp_state*
    void            *localstore;
    long             localtop;
    long             localalloc;
    long             totaluse;
    void            *reap;          // struct alloc_chain*
    long             glue_bits;
    long             time_bits;
    long             floor_bits;
    long             res_bits;
    void            *internal;
} my_vorbis_block_t;

typedef struct {
    unsigned char   *packet;
    long             bytes;
    long             b_o_s;
    long             e_o_s;
    int64_t          granulepos;
    int64_t          packetno;
} my_ogg_packet_t;

// ============================================================================
// 32-bit struct definitions (i386 layout)
// ============================================================================

typedef struct {
    int      version;
    int      channels;
    long_t   rate;
    long_t   bitrate_upper;
    long_t   bitrate_nominal;
    long_t   bitrate_lower;
    long_t   bitrate_window;
    ptr_t    codec_setup;       // void*
} my_vorbis_info_32_t;

typedef struct {
    ptr_t    user_comments;     // char**
    ptr_t    comment_lengths;   // int*
    int      comments;
    ptr_t    vendor;            // char*
} my_vorbis_comment_32_t;

typedef struct {
    int      analysisp;
    ptr_t    vi;                // vorbis_info*
    ptr_t    pcm;               // float**
    ptr_t    pcmret;            // float**
    int      pcm_storage;
    int      pcm_current;
    int      pcm_returned;
    int      preextrapolate;
    int      eofflag;
    long_t   lW;
    long_t   W;
    long_t   nW;
    long_t   centerW;
    int64_t  granulepos;
    int64_t  sequence;
    int64_t  glue_bits;
    int64_t  time_bits;
    int64_t  floor_bits;
    int64_t  res_bits;
    ptr_t    backend_state;     // void*
} my_vorbis_dsp_state_32_t;

typedef struct {
    long_t           endbyte;
    int              endbit;
    ptr_t            buffer;        // unsigned char*
    ptr_t            ptr;           // unsigned char*
    long_t           storage;
} my_oggpack_buffer_32_t;

typedef struct {
    ptr_t            pcm;           // float**
    my_oggpack_buffer_32_t opb;
    long_t           lW;
    long_t           W;
    long_t           nW;
    int              pcmend;
    int              mode;
    int              eofflag;
    int64_t          granulepos;
    int64_t          sequence;
    ptr_t            vd;            // vorbis_dsp_state*
    ptr_t            localstore;    // void*
    long_t           localtop;
    long_t           localalloc;
    long_t           totaluse;
    ptr_t            reap;          // struct alloc_chain*
    long_t           glue_bits;
    long_t           time_bits;
    long_t           floor_bits;
    long_t           res_bits;
    ptr_t            internal;      // void*
} my_vorbis_block_32_t;

typedef struct {
    ptr_t            packet;        // unsigned char*
    long_t           bytes;
    long_t           b_o_s;
    long_t           e_o_s;
    int64_t          granulepos;
    int64_t          packetno;
} my_ogg_packet_32_t;

// ============================================================================
// Shadow struct management using khash
// ============================================================================

KHASH_MAP_INIT_INT(dspstate, my_vorbis_dsp_state_t*);
static kh_dspstate_t* dsp_shadows = NULL;

KHASH_MAP_INIT_INT(blockstate, my_vorbis_block_t*);
static kh_blockstate_t* block_shadows = NULL;

KHASH_MAP_INIT_INT(infostate, my_vorbis_info_t*);
static kh_infostate_t* info_shadows = NULL;

KHASH_MAP_INIT_INT(commentstate, my_vorbis_comment_t*);
static kh_commentstate_t* comment_shadows = NULL;

// Get or create a native shadow for a 32-bit vorbis_info
static my_vorbis_info_t* getInfoShadow(void* a)
{
    if(!a) return NULL;
    ptr_t key = to_ptrv(a);
    khint_t k = kh_get(infostate, info_shadows, key);
    if(k == kh_end(info_shadows)) {
        int r;
        k = kh_put(infostate, info_shadows, key, &r);
        kh_value(info_shadows, k) = (my_vorbis_info_t*)calloc(1, sizeof(my_vorbis_info_t));
    }
    return kh_value(info_shadows, k);
}

static void delInfoShadow(void* a)
{
    if(!a) return;
    ptr_t key = to_ptrv(a);
    khint_t k = kh_get(infostate, info_shadows, key);
    if(k != kh_end(info_shadows)) {
        free(kh_value(info_shadows, k));
        kh_del(infostate, info_shadows, k);
    }
}

// Get or create a native shadow for a 32-bit vorbis_comment
static my_vorbis_comment_t* getCommentShadow(void* a)
{
    if(!a) return NULL;
    ptr_t key = to_ptrv(a);
    khint_t k = kh_get(commentstate, comment_shadows, key);
    if(k == kh_end(comment_shadows)) {
        int r;
        k = kh_put(commentstate, comment_shadows, key, &r);
        kh_value(comment_shadows, k) = (my_vorbis_comment_t*)calloc(1, sizeof(my_vorbis_comment_t));
    }
    return kh_value(comment_shadows, k);
}

static void delCommentShadow(void* a)
{
    if(!a) return;
    ptr_t key = to_ptrv(a);
    khint_t k = kh_get(commentstate, comment_shadows, key);
    if(k != kh_end(comment_shadows)) {
        free(kh_value(comment_shadows, k));
        kh_del(commentstate, comment_shadows, k);
    }
}

// Get or create a native shadow for a 32-bit vorbis_dsp_state
static my_vorbis_dsp_state_t* getDspShadow(void* a)
{
    if(!a) return NULL;
    ptr_t key = to_ptrv(a);
    khint_t k = kh_get(dspstate, dsp_shadows, key);
    if(k == kh_end(dsp_shadows)) {
        int r;
        k = kh_put(dspstate, dsp_shadows, key, &r);
        kh_value(dsp_shadows, k) = (my_vorbis_dsp_state_t*)calloc(1, sizeof(my_vorbis_dsp_state_t));
    }
    return kh_value(dsp_shadows, k);
}

static void delDspShadow(void* a)
{
    if(!a) return;
    ptr_t key = to_ptrv(a);
    khint_t k = kh_get(dspstate, dsp_shadows, key);
    if(k != kh_end(dsp_shadows)) {
        free(kh_value(dsp_shadows, k));
        kh_del(dspstate, dsp_shadows, k);
    }
}

// Get or create a native shadow for a 32-bit vorbis_block
static my_vorbis_block_t* getBlockShadow(void* a)
{
    if(!a) return NULL;
    ptr_t key = to_ptrv(a);
    khint_t k = kh_get(blockstate, block_shadows, key);
    if(k == kh_end(block_shadows)) {
        int r;
        k = kh_put(blockstate, block_shadows, key, &r);
        kh_value(block_shadows, k) = (my_vorbis_block_t*)calloc(1, sizeof(my_vorbis_block_t));
    }
    return kh_value(block_shadows, k);
}

static void delBlockShadow(void* a)
{
    if(!a) return;
    ptr_t key = to_ptrv(a);
    khint_t k = kh_get(blockstate, block_shadows, key);
    if(k != kh_end(block_shadows)) {
        free(kh_value(block_shadows, k));
        kh_del(blockstate, block_shadows, k);
    }
}

// ============================================================================
// Conversion: vorbis_info native <-> 32-bit
// ============================================================================

static void vorbis_info_to_32(my_vorbis_info_32_t* dst, const my_vorbis_info_t* src)
{
    dst->version = src->version;
    dst->channels = src->channels;
    dst->rate = to_long(src->rate);
    dst->bitrate_upper = to_long(src->bitrate_upper);
    dst->bitrate_nominal = to_long(src->bitrate_nominal);
    dst->bitrate_lower = to_long(src->bitrate_lower);
    dst->bitrate_window = to_long(src->bitrate_window);
    dst->codec_setup = to_ptrv(src->codec_setup);
}

// ============================================================================
// Conversion: vorbis_comment native <-> 32-bit
// ============================================================================

static void vorbis_comment_to_32(my_vorbis_comment_32_t* dst, const my_vorbis_comment_t* src)
{
    dst->user_comments = to_ptrv(src->user_comments);
    dst->comment_lengths = to_ptrv(src->comment_lengths);
    dst->comments = src->comments;
    dst->vendor = to_ptrv(src->vendor);
}

// ============================================================================
// Conversion: vorbis_dsp_state native <-> 32-bit
// ============================================================================

static void vorbis_dsp_state_to_32(my_vorbis_dsp_state_32_t* dst, const my_vorbis_dsp_state_t* src)
{
    dst->analysisp = src->analysisp;
    dst->vi = to_ptrv(src->vi);
    dst->pcm = to_ptrv(src->pcm);
    dst->pcmret = to_ptrv(src->pcmret);
    dst->pcm_storage = src->pcm_storage;
    dst->pcm_current = src->pcm_current;
    dst->pcm_returned = src->pcm_returned;
    dst->preextrapolate = src->preextrapolate;
    dst->eofflag = src->eofflag;
    dst->lW = to_long(src->lW);
    dst->W = to_long(src->W);
    dst->nW = to_long(src->nW);
    dst->centerW = to_long(src->centerW);
    dst->granulepos = src->granulepos;
    dst->sequence = src->sequence;
    dst->glue_bits = src->glue_bits;
    dst->time_bits = src->time_bits;
    dst->floor_bits = src->floor_bits;
    dst->res_bits = src->res_bits;
    dst->backend_state = to_ptrv(src->backend_state);
}

// ============================================================================
// Conversion: oggpack_buffer native <-> 32-bit
// ============================================================================

static void oggpack_buffer_to_32(my_oggpack_buffer_32_t* dst, const my_oggpack_buffer_t* src)
{
    dst->endbyte = to_long(src->endbyte);
    dst->endbit = src->endbit;
    dst->buffer = to_ptrv(src->buffer);
    dst->ptr = to_ptrv(src->ptr);
    dst->storage = to_long(src->storage);
}

// ============================================================================
// Conversion: vorbis_block native <-> 32-bit
// ============================================================================

static void vorbis_block_to_32(my_vorbis_block_32_t* dst, const my_vorbis_block_t* src)
{
    dst->pcm = to_ptrv(src->pcm);
    oggpack_buffer_to_32(&dst->opb, &src->opb);
    dst->lW = to_long(src->lW);
    dst->W = to_long(src->W);
    dst->nW = to_long(src->nW);
    dst->pcmend = src->pcmend;
    dst->mode = src->mode;
    dst->eofflag = src->eofflag;
    dst->granulepos = src->granulepos;
    dst->sequence = src->sequence;
    dst->vd = to_ptrv(src->vd);
    dst->localstore = to_ptrv(src->localstore);
    dst->localtop = to_long(src->localtop);
    dst->localalloc = to_long(src->localalloc);
    dst->totaluse = to_long(src->totaluse);
    dst->reap = to_ptrv(src->reap);
    dst->glue_bits = to_long(src->glue_bits);
    dst->time_bits = to_long(src->time_bits);
    dst->floor_bits = to_long(src->floor_bits);
    dst->res_bits = to_long(src->res_bits);
    dst->internal = to_ptrv(src->internal);
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
// vorbis_info_* wrappers
// ============================================================================

EXPORT void my32_vorbis_info_init(x64emu_t* emu, void* vi)
{
    my_vorbis_info_t* shadow = getInfoShadow(vi);
    my->vorbis_info_init(shadow);
    vorbis_info_to_32(vi, shadow);
}

EXPORT void my32_vorbis_info_clear(x64emu_t* emu, void* vi)
{
    my_vorbis_info_t* shadow = getInfoShadow(vi);
    my->vorbis_info_clear(shadow);
    vorbis_info_to_32(vi, shadow);
    delInfoShadow(vi);
}

EXPORT int my32_vorbis_info_blocksize(x64emu_t* emu, void* vi, int zo)
{
    my_vorbis_info_t* shadow = getInfoShadow(vi);
    return my->vorbis_info_blocksize(shadow, zo);
}

// ============================================================================
// vorbis_comment_* wrappers
// ============================================================================

EXPORT void my32_vorbis_comment_init(x64emu_t* emu, void* vc)
{
    my_vorbis_comment_t* shadow = getCommentShadow(vc);
    my->vorbis_comment_init(shadow);
    vorbis_comment_to_32(vc, shadow);
}

EXPORT void my32_vorbis_comment_add(x64emu_t* emu, void* vc, void* comment)
{
    my_vorbis_comment_t* shadow = getCommentShadow(vc);
    my->vorbis_comment_add(shadow, comment);
    vorbis_comment_to_32(vc, shadow);
}

EXPORT void my32_vorbis_comment_add_tag(x64emu_t* emu, void* vc, void* tag, void* contents)
{
    my_vorbis_comment_t* shadow = getCommentShadow(vc);
    my->vorbis_comment_add_tag(shadow, tag, contents);
    vorbis_comment_to_32(vc, shadow);
}

EXPORT void* my32_vorbis_comment_query(x64emu_t* emu, void* vc, void* tag, int count)
{
    my_vorbis_comment_t* shadow = getCommentShadow(vc);
    return my->vorbis_comment_query(shadow, tag, count);
}

EXPORT int my32_vorbis_comment_query_count(x64emu_t* emu, void* vc, void* tag)
{
    my_vorbis_comment_t* shadow = getCommentShadow(vc);
    return my->vorbis_comment_query_count(shadow, tag);
}

EXPORT void my32_vorbis_comment_clear(x64emu_t* emu, void* vc)
{
    my_vorbis_comment_t* shadow = getCommentShadow(vc);
    my->vorbis_comment_clear(shadow);
    vorbis_comment_to_32(vc, shadow);
    delCommentShadow(vc);
}

EXPORT int my32_vorbis_commentheader_out(x64emu_t* emu, void* vc, void* op)
{
    my_vorbis_comment_t* vc_shadow = getCommentShadow(vc);
    my_ogg_packet_t op_l = {0};
    int ret = my->vorbis_commentheader_out(vc_shadow, &op_l);
    if(op) ogg_packet_to_32(op, &op_l);
    vorbis_comment_to_32(vc, vc_shadow);
    return ret;
}

// ============================================================================
// vorbis_dsp_* / vorbis_analysis_* / vorbis_synthesis_* wrappers
// ============================================================================

EXPORT int my32_vorbis_analysis_init(x64emu_t* emu, void* vd, void* vi)
{
    my_vorbis_dsp_state_t* dsp_shadow = getDspShadow(vd);
    my_vorbis_info_t* info_shadow = getInfoShadow(vi);
    int ret = my->vorbis_analysis_init(dsp_shadow, info_shadow);
    vorbis_dsp_state_to_32(vd, dsp_shadow);
    return ret;
}

EXPORT int my32_vorbis_synthesis_init(x64emu_t* emu, void* vd, void* vi)
{
    my_vorbis_dsp_state_t* dsp_shadow = getDspShadow(vd);
    my_vorbis_info_t* info_shadow = getInfoShadow(vi);
    int ret = my->vorbis_synthesis_init(dsp_shadow, info_shadow);
    vorbis_dsp_state_to_32(vd, dsp_shadow);
    return ret;
}

EXPORT ptr_t my32_vorbis_analysis_buffer(x64emu_t* emu, void* vd, int vals)
{
    my_vorbis_dsp_state_t* dsp_shadow = getDspShadow(vd);
    float** native_buf = my->vorbis_analysis_buffer(dsp_shadow, vals);
    vorbis_dsp_state_to_32(vd, dsp_shadow);
    if(native_buf) {
        my_vorbis_info_t* vi = (my_vorbis_info_t*)dsp_shadow->vi;
        int channels = vi ? vi->channels : 0;
        if(channels > 256) channels = 256;
        for(int i = 0; i < channels; i++)
            dsp_shadow->pcm32[i] = to_ptrv(native_buf[i]);
        return to_ptrv(dsp_shadow->pcm32);
    }
    return 0;
}

EXPORT int my32_vorbis_analysis_wrote(x64emu_t* emu, void* vd, int vals)
{
    my_vorbis_dsp_state_t* dsp_shadow = getDspShadow(vd);
    int ret = my->vorbis_analysis_wrote(dsp_shadow, vals);
    vorbis_dsp_state_to_32(vd, dsp_shadow);
    return ret;
}

EXPORT int my32_vorbis_analysis_blockout(x64emu_t* emu, void* vd, void* vb)
{
    my_vorbis_dsp_state_t* dsp_shadow = getDspShadow(vd);
    my_vorbis_block_t* blk_shadow = getBlockShadow(vb);
    int ret = my->vorbis_analysis_blockout(dsp_shadow, blk_shadow);
    vorbis_dsp_state_to_32(vd, dsp_shadow);
    vorbis_block_to_32(vb, blk_shadow);
    return ret;
}

EXPORT int my32_vorbis_analysis(x64emu_t* emu, void* vb, void* op)
{
    my_vorbis_block_t* blk_shadow = getBlockShadow(vb);
    my_ogg_packet_t op_l = {0};
    int ret = my->vorbis_analysis(blk_shadow, op ? &op_l : NULL);
    vorbis_block_to_32(vb, blk_shadow);
    if(op) ogg_packet_to_32(op, &op_l);
    return ret;
}

EXPORT int my32_vorbis_analysis_headerout(x64emu_t* emu, void* vd, void* vc, void* op, void* op_comm, void* op_code)
{
    my_vorbis_dsp_state_t* dsp_shadow = getDspShadow(vd);
    my_vorbis_comment_t* vc_shadow = getCommentShadow(vc);
    my_ogg_packet_t op_l = {0}, op_comm_l = {0}, op_code_l = {0};
    int ret = my->vorbis_analysis_headerout(dsp_shadow, vc_shadow, &op_l, &op_comm_l, &op_code_l);
    vorbis_dsp_state_to_32(vd, dsp_shadow);
    vorbis_comment_to_32(vc, vc_shadow);
    if(op) ogg_packet_to_32(op, &op_l);
    if(op_comm) ogg_packet_to_32(op_comm, &op_comm_l);
    if(op_code) ogg_packet_to_32(op_code, &op_code_l);
    return ret;
}

EXPORT int my32_vorbis_bitrate_addblock(x64emu_t* emu, void* vb)
{
    my_vorbis_block_t* blk_shadow = getBlockShadow(vb);
    int ret = my->vorbis_bitrate_addblock(blk_shadow);
    vorbis_block_to_32(vb, blk_shadow);
    return ret;
}

EXPORT int my32_vorbis_bitrate_flushpacket(x64emu_t* emu, void* vd, void* op)
{
    my_vorbis_dsp_state_t* dsp_shadow = getDspShadow(vd);
    my_ogg_packet_t op_l = {0};
    int ret = my->vorbis_bitrate_flushpacket(dsp_shadow, op ? &op_l : NULL);
    vorbis_dsp_state_to_32(vd, dsp_shadow);
    if(op) ogg_packet_to_32(op, &op_l);
    return ret;
}

EXPORT int my32_vorbis_block_init(x64emu_t* emu, void* vd, void* vb)
{
    my_vorbis_dsp_state_t* dsp_shadow = getDspShadow(vd);
    my_vorbis_block_t* blk_shadow = getBlockShadow(vb);
    int ret = my->vorbis_block_init(dsp_shadow, blk_shadow);
    vorbis_dsp_state_to_32(vd, dsp_shadow);
    vorbis_block_to_32(vb, blk_shadow);
    return ret;
}

EXPORT int my32_vorbis_block_clear(x64emu_t* emu, void* vb)
{
    my_vorbis_block_t* blk_shadow = getBlockShadow(vb);
    int ret = my->vorbis_block_clear(blk_shadow);
    vorbis_block_to_32(vb, blk_shadow);
    delBlockShadow(vb);
    return ret;
}

EXPORT void my32_vorbis_dsp_clear(x64emu_t* emu, void* vd)
{
    my_vorbis_dsp_state_t* dsp_shadow = getDspShadow(vd);
    my->vorbis_dsp_clear(dsp_shadow);
    vorbis_dsp_state_to_32(vd, dsp_shadow);
    delDspShadow(vd);
}

EXPORT int my32_vorbis_synthesis(x64emu_t* emu, void* vb, void* op)
{
    my_vorbis_block_t* blk_shadow = getBlockShadow(vb);
    my_ogg_packet_t op_l;
    ogg_packet_to_native(&op_l, op);
    int ret = my->vorbis_synthesis(blk_shadow, &op_l);
    vorbis_block_to_32(vb, blk_shadow);
    return ret;
}

EXPORT int my32_vorbis_synthesis_trackonly(x64emu_t* emu, void* vb, void* op)
{
    my_vorbis_block_t* blk_shadow = getBlockShadow(vb);
    my_ogg_packet_t op_l;
    ogg_packet_to_native(&op_l, op);
    int ret = my->vorbis_synthesis_trackonly(blk_shadow, &op_l);
    vorbis_block_to_32(vb, blk_shadow);
    return ret;
}

EXPORT int my32_vorbis_synthesis_blockin(x64emu_t* emu, void* vd, void* vb)
{
    my_vorbis_dsp_state_t* dsp_shadow = getDspShadow(vd);
    my_vorbis_block_t* blk_shadow = getBlockShadow(vb);
    int ret = my->vorbis_synthesis_blockin(dsp_shadow, blk_shadow);
    vorbis_dsp_state_to_32(vd, dsp_shadow);
    vorbis_block_to_32(vb, blk_shadow);
    return ret;
}

EXPORT int my32_vorbis_synthesis_pcmout(x64emu_t* emu, void* vd, ptr_t* pcm)
{
    my_vorbis_dsp_state_t* dsp_shadow = getDspShadow(vd);
    float** native_pcm = NULL;
    int ret = my->vorbis_synthesis_pcmout(dsp_shadow, pcm ? &native_pcm : NULL);
    vorbis_dsp_state_to_32(vd, dsp_shadow);
    if(pcm) {
        if(native_pcm) {
            // Convert the float** (array of native float* per channel) to ptr_t[]
            my_vorbis_info_t* vi = (my_vorbis_info_t*)dsp_shadow->vi;
            int channels = vi ? vi->channels : 0;
            if(channels > 256) channels = 256;
            for(int i = 0; i < channels; i++)
                dsp_shadow->pcm32[i] = to_ptrv(native_pcm[i]);
            *pcm = to_ptrv(dsp_shadow->pcm32);
        } else {
            *pcm = 0;
        }
    }
    return ret;
}

EXPORT int my32_vorbis_synthesis_lapout(x64emu_t* emu, void* vd, ptr_t* pcm)
{
    my_vorbis_dsp_state_t* dsp_shadow = getDspShadow(vd);
    float** native_pcm = NULL;
    int ret = my->vorbis_synthesis_lapout(dsp_shadow, pcm ? &native_pcm : NULL);
    vorbis_dsp_state_to_32(vd, dsp_shadow);
    if(pcm) {
        if(native_pcm) {
            my_vorbis_info_t* vi = (my_vorbis_info_t*)dsp_shadow->vi;
            int channels = vi ? vi->channels : 0;
            if(channels > 256) channels = 256;
            for(int i = 0; i < channels; i++)
                dsp_shadow->pcm32[i] = to_ptrv(native_pcm[i]);
            *pcm = to_ptrv(dsp_shadow->pcm32);
        } else {
            *pcm = 0;
        }
    }
    return ret;
}

EXPORT int my32_vorbis_synthesis_read(x64emu_t* emu, void* vd, int samples)
{
    my_vorbis_dsp_state_t* dsp_shadow = getDspShadow(vd);
    int ret = my->vorbis_synthesis_read(dsp_shadow, samples);
    vorbis_dsp_state_to_32(vd, dsp_shadow);
    return ret;
}

EXPORT int my32_vorbis_synthesis_headerin(x64emu_t* emu, void* vi, void* vc, void* op)
{
    my_vorbis_info_t* info_shadow = getInfoShadow(vi);
    my_vorbis_comment_t* vc_shadow = getCommentShadow(vc);
    my_ogg_packet_t op_l;
    ogg_packet_to_native(&op_l, op);
    int ret = my->vorbis_synthesis_headerin(info_shadow, vc_shadow, &op_l);
    vorbis_info_to_32(vi, info_shadow);
    vorbis_comment_to_32(vc, vc_shadow);
    return ret;
}

EXPORT int my32_vorbis_synthesis_idheader(x64emu_t* emu, void* op)
{
    my_ogg_packet_t op_l;
    ogg_packet_to_native(&op_l, op);
    return my->vorbis_synthesis_idheader(&op_l);
}

EXPORT int my32_vorbis_synthesis_halfrate(x64emu_t* emu, void* vi, int flag)
{
    my_vorbis_info_t* info_shadow = getInfoShadow(vi);
    int ret = my->vorbis_synthesis_halfrate(info_shadow, flag);
    vorbis_info_to_32(vi, info_shadow);
    return ret;
}

EXPORT int my32_vorbis_synthesis_halfrate_p(x64emu_t* emu, void* vi)
{
    my_vorbis_info_t* info_shadow = getInfoShadow(vi);
    return my->vorbis_synthesis_halfrate_p(info_shadow);
}

EXPORT int my32_vorbis_synthesis_restart(x64emu_t* emu, void* vd)
{
    my_vorbis_dsp_state_t* dsp_shadow = getDspShadow(vd);
    int ret = my->vorbis_synthesis_restart(dsp_shadow);
    vorbis_dsp_state_to_32(vd, dsp_shadow);
    return ret;
}

EXPORT int my32_vorbis_packet_blocksize(x64emu_t* emu, void* vi, void* op)
{
    my_vorbis_info_t* info_shadow = getInfoShadow(vi);
    my_ogg_packet_t op_l;
    ogg_packet_to_native(&op_l, op);
    return my->vorbis_packet_blocksize(info_shadow, &op_l);
}

EXPORT void* my32_vorbis_window(x64emu_t* emu, void* vd, int W)
{
    my_vorbis_dsp_state_t* dsp_shadow = getDspShadow(vd);
    return my->vorbis_window(dsp_shadow, W);
}

// ============================================================================
// Init / Fini
// ============================================================================

#define CUSTOM_INIT \
    dsp_shadows = kh_init(dspstate);            \
    block_shadows = kh_init(blockstate);         \
    info_shadows = kh_init(infostate);           \
    comment_shadows = kh_init(commentstate);

#define CUSTOM_FINI \
    my_vorbis_dsp_state_t* ds;                                  \
    kh_foreach_value(dsp_shadows, ds, free(ds));                \
    kh_destroy(dspstate, dsp_shadows);                          \
    dsp_shadows = NULL;                                         \
    my_vorbis_block_t* bs;                                      \
    kh_foreach_value(block_shadows, bs, free(bs));              \
    kh_destroy(blockstate, block_shadows);                      \
    block_shadows = NULL;                                       \
    my_vorbis_info_t* vi_s;                                     \
    kh_foreach_value(info_shadows, vi_s, free(vi_s));           \
    kh_destroy(infostate, info_shadows);                        \
    info_shadows = NULL;                                        \
    my_vorbis_comment_t* cs;                                    \
    kh_foreach_value(comment_shadows, cs, free(cs));            \
    kh_destroy(commentstate, comment_shadows);                  \
    comment_shadows = NULL;

#include "wrappedlib_init32.h"
