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
#include "callback.h"
#include "librarian.h"
#include "box32context.h"
#include "emu/x64emu_private.h"
#include "myalign32.h"

static const char* freetypeName = 
#ifdef ANDROID
    "libfreetype.so"
#else
    "libfreetype.so.6"
#endif
    ;
#define LIBNAME freetype

typedef void  (*vFp_t)(void*);


// 64bits FreeType structures
typedef union  FT_StreamDesc_s
{
    long   value;
    void*  pointer;
} FT_StreamDesc_t;

typedef struct  FT_StreamRec_s
{
    unsigned char*       base;
    unsigned long        size;
    unsigned long        pos;

    FT_StreamDesc_t      descriptor;
    FT_StreamDesc_t      pathname;
    void*                read;
    void*                close;

    void*                memory;
    unsigned char*       cursor;
    unsigned char*       limit;

} FT_StreamRec_t;

typedef struct  FT_Open_Args_s
{
    uint32_t        flags;
    const uint8_t*  memory_base;
    intptr_t        memory_size;
    char*           pathname;
    FT_StreamRec_t* stream;
    void*           driver;
    int32_t         num_params;
    void*           params;
} FT_Open_Args_t;

typedef struct  FT_BBox_s
{
    signed long     xMin, yMin;
    signed long     xMax, yMax;
} FT_BBox_t;
typedef struct  FT_Generic_s
{
    void*           data;
    vFp_t           finalizer;
} FT_Generic_t;
typedef struct  FT_ListRec_s
{
  void*             head;
  void*             tail;
} FT_ListRec_t;

typedef struct  FT_Vector_s
{
    long  x;
    long  y;
} FT_Vector_t;

typedef struct  FT_Bitmap_Size_s
{
    short  height;
    short  width;
    long   size;
    long   x_ppem;
    long   y_ppem;
} FT_Bitmap_Size_t;

typedef struct  FT_Glyph_Metrics_s
{
    long        width;
    long        height;
    long        horiBearingX;
    long        horiBearingY;
    long        horiAdvance;
    long        vertBearingX;
    long        vertBearingY;
    long        vertAdvance;
} FT_Glyph_Metrics_t;

typedef struct  FT_Outline_s
{
    unsigned short   n_contours;
    unsigned short   n_points;
    FT_Vector_t*     points;
    unsigned char*   tags;
    unsigned short*  contours;
    int              flags;
} FT_Outline_t;

typedef struct  FT_Bitmap_s
{
    unsigned int    rows;
    unsigned int    width;
    int             pitch;
    unsigned char*  buffer;
    unsigned short  num_grays;
    unsigned char   pixel_mode;
    unsigned char   palette_mode;
    void*           palette;
} FT_Bitmap_t;

typedef struct  FT_GlyphSlotRec_s
{
    void*                       library;  //FT_Library
    void*                       face; //FT_Face
    struct FT_GlyphSlotRec_s*   next;
    uint32_t                    glyph_index;
    FT_Generic_t                generic;
    FT_Glyph_Metrics_t          metrics;
    long                        linearHoriAdvance;
    long                        linearVertAdvance;
    FT_Vector_t                 advance;
    int                         format;
    FT_Bitmap_t                 bitmap;
    int                         bitmap_left;
    int                         bitmap_top;
    FT_Outline_t                outline;
    uint32_t                    num_subglyphs;
    void*                       subglyphs;  //FT_SubGlyph
    void*                       control_data;
    long                        control_len;
    long                        lsb_delta;
    long                        rsb_delta;
    void*                       other;
    void*                       internal;
} FT_GlyphSlotRec_t;

typedef struct  FT_FaceRec_s
{
    signed long         num_faces;
    signed long         face_index;
    signed long         face_flags;
    signed long         style_flags;
    signed long         num_glyphs;
    char*               family_name;
    char*               style_name;
    int                 num_fixed_sizes;
    FT_Bitmap_Size_t*   available_sizes;
    int                 num_charmaps;
    void*               charmaps;
    FT_Generic_t        generic;
    FT_BBox_t           bbox;
    uint16_t            units_per_EM;
    int16_t             ascender;
    int16_t             descender;
    int16_t             height;
    int16_t             max_advance_width;
    int16_t             max_advance_height;
    int16_t             underline_position;
    int16_t             underline_thickness;
    FT_GlyphSlotRec_t*  glyph;
    void*               size;
    void*               charmap;
    /*@private begin */
    void*               driver;
    void*               memory;
    FT_StreamDesc_t*    stream;
    FT_ListRec_t        sizes_list;
    FT_Generic_t        autohint;   /* face-specific auto-hinter data */
    void*               extensions; /* unused                         */
    void*               internal;
} FT_FaceRec_t;

typedef struct  FT_MemoryRec_s
{
    void*           user;
    void*           alloc;
    void*           free;
    void*           realloc;
} FT_MemoryRec_t;

typedef struct  PS_PrivateRec_s
{
    int        unique_id;
    int        lenIV;
    uint8_t    num_blue_values;
    uint8_t    num_other_blues;
    uint8_t    num_family_blues;
    uint8_t    num_family_other_blues;
    int16_t    blue_values[14];
    int16_t    other_blues[10];
    int16_t    family_blues      [14];
    int16_t    family_other_blues[10];
    long       blue_scale;
    int        blue_shift;
    int        blue_fuzz;
    uint16_t   standard_width[1];
    uint16_t   standard_height[1];
    uint8_t    num_snap_widths;
    uint8_t    num_snap_heights;
    uint8_t    force_bold;
    uint8_t    round_stem_up;
    int16_t    snap_widths [13];
    int16_t    snap_heights[13];
    long       expansion_factor;
    long       language_group;
    long       password;
    int16_t    min_feature[2];
} PS_PrivateRec_t;

typedef struct  BDF_PropertyRec_s
{
    int  type;
    union {
        void*     atom; //const char*
        int       integer;
        uint32_t  cardinal;

    } u;
} BDF_PropertyRec_t;

typedef struct  FT_Size_RequestRec_s
{
    int         type;
    long        width;
    long        height;
    uint32_t    horiResolution;
    uint32_t    vertResolution;
} FT_Size_RequestRec_t;

// 32bits FreeType structures
typedef union  FT_StreamDesc_32_s
{
    long_t value;
    ptr_t  pointer; //void*
} FT_StreamDesc_32_t;

typedef struct  FT_StreamRec_32_s
{
    ptr_t                base; //unsigned char*
    ulong_t              size;
    ulong_t              pos;

    FT_StreamDesc_32_t   descriptor;
    FT_StreamDesc_32_t   pathname;
    ptr_t                read;  //void*
    ptr_t                close; //void*

    ptr_t                memory;    //void*
    ptr_t                cursor; //unsigned char*
    ptr_t                limit; //unsigned char*

} FT_StreamRec_32_t;

typedef struct  FT_Open_Args_32_s
{
    uint32_t        flags;
    ptr_t           memory_base; //const uint8_t*
    long_t          memory_size; //intptr_t
    ptr_t           pathname; //char*
    ptr_t           stream; //FT_StreamRec_t*
    ptr_t           driver; //void*
    int32_t         num_params;
    ptr_t           params; //void*
} FT_Open_Args_32_t;

typedef struct  FT_BBox_32_s
{
    long_t          xMin, yMin;
    long_t          xMax, yMax;
} FT_BBox_32_t;
typedef struct  FT_Generic_32_s
{
    ptr_t           data; //void*
    ptr_t           finalizer; //vFp_t
} FT_Generic_32_t;
typedef struct  FT_ListRec_32_s
{
  ptr_t             head; //void*
  ptr_t             tail; //void*
} FT_ListRec_32_t;

typedef struct  FT_Vector_32_s
{
    long_t  x;
    long_t  y;
} FT_Vector_32_t;

typedef struct  FT_Bitmap_Size_32_s
{
    short  height;
    short  width;
    long_t size;
    long_t x_ppem;
    long_t y_ppem;
} FT_Bitmap_Size_32_t;

typedef struct  FT_Glyph_Metrics_32_s
{
    long_t      width;
    long_t      height;
    long_t      horiBearingX;
    long_t      horiBearingY;
    long_t      horiAdvance;
    long_t      vertBearingX;
    long_t      vertBearingY;
    long_t      vertAdvance;
} FT_Glyph_Metrics_32_t;

typedef struct  FT_Outline_32_s
{
    unsigned short   n_contours;
    unsigned short   n_points;
    ptr_t            points;   //FT_Vector_32_t*
    ptr_t            tags;  //unsigned char*
    ptr_t            contours;  //unsigned short*
    int              flags;
} FT_Outline_32_t;

typedef struct  FT_Bitmap_32_s
{
    unsigned int    rows;
    unsigned int    width;
    int             pitch;
    ptr_t           buffer; //unsigned char*
    unsigned short  num_grays;
    unsigned char   pixel_mode;
    unsigned char   palette_mode;
    ptr_t           palette;    //void*
} FT_Bitmap_32_t;

typedef struct  FT_GlyphSlotRec_32_s
{
    ptr_t                       library;  //FT_Library
    ptr_t                       face; //FT_Face
    ptr_t                       next;   //struct FT_GlyphSlotRec_s*
    uint32_t                    glyph_index;
    FT_Generic_32_t             generic;        //0x10
    FT_Glyph_Metrics_32_t       metrics;        //0x18
    long_t                      linearHoriAdvance;  //0x38
    long_t                      linearVertAdvance;
    FT_Vector_32_t              advance;    //0x40
    int                         format;
    FT_Bitmap_32_t              bitmap; //0x4c
    int                         bitmap_left;
    int                         bitmap_top;
    FT_Outline_32_t             outline;
    uint32_t                    num_subglyphs;
    ptr_t                       subglyphs;  //FT_SubGlyph
    ptr_t                       control_data;
    long_t                      control_len;
    long_t                      lsb_delta;
    long_t                      rsb_delta;
    ptr_t                       other;
    ptr_t                       internal;
} FT_GlyphSlotRec_32_t;

typedef struct  FT_FaceRec_32_s
{
    long_t          num_faces;
    long_t          face_index;
    long_t          face_flags;
    long_t          style_flags;
    long_t          num_glyphs; //0x10
    ptr_t           family_name;    //char*
    ptr_t           style_name; //char*
    int             num_fixed_sizes;
    ptr_t           available_sizes; //FT_Bitmap_32_t*  //0x20
    int             num_charmaps;
    ptr_t           charmaps; //void*
    FT_Generic_32_t generic;    //0x28
    FT_BBox_32_t    bbox;       //0x30
    uint16_t        units_per_EM;   //0x40
    int16_t         ascender;
    int16_t         descender;
    int16_t         height;
    int16_t         max_advance_width;
    int16_t         max_advance_height;
    int16_t         underline_position;
    int16_t         underline_thickness;
    ptr_t           glyph; //FT_GlyphSlotRec_t* //0x50
    ptr_t           size; //void*
    ptr_t           charmap; //void*
    /*@private begin */
    ptr_t           driver; //void*     //0x5c
    ptr_t           memory; //void*
    ptr_t           stream; //FT_StreamDesc_t*
    FT_ListRec_32_t sizes_list;
    FT_Generic_32_t autohint;   /* face-specific auto-hinter data */
    ptr_t           extensions; /* unused                         */ //void*
    ptr_t           internal; //void*
} FT_FaceRec_32_t;

typedef struct  FT_MemoryRec_32_s
{
    ptr_t           user;   //void*
    ptr_t           alloc;  //void*
    ptr_t           free;   //void*
    ptr_t           realloc;//void*
} FT_MemoryRec_32_t;

typedef struct  PS_PrivateRec_32_s
{
    int        unique_id;
    int        lenIV;
    uint8_t    num_blue_values;
    uint8_t    num_other_blues;
    uint8_t    num_family_blues;
    uint8_t    num_family_other_blues;
    int16_t    blue_values[14];
    int16_t    other_blues[10];
    int16_t    family_blues      [14];
    int16_t    family_other_blues[10];
    long_t     blue_scale;
    int        blue_shift;
    int        blue_fuzz;
    uint16_t   standard_width[1];
    uint16_t   standard_height[1];
    uint8_t    num_snap_widths;
    uint8_t    num_snap_heights;
    uint8_t    force_bold;
    uint8_t    round_stem_up;
    int16_t    snap_widths [13];
    int16_t    snap_heights[13];
    long_t     expansion_factor;
    long_t     language_group;
    long_t     password;
    int16_t    min_feature[2];
} PS_PrivateRec_32_t;

typedef struct  BDF_PropertyRec_32_s
{
    int  type;
    union {
        ptr_t     atom; //const char*
        int       integer;
        uint32_t  cardinal;

    } u;
} BDF_PropertyRec_32_t;

typedef struct  FT_Size_RequestRec_32_s
{
    int         type;
    long_t      width;
    long_t      height;
    uint32_t    horiResolution;
    uint32_t    vertResolution;
} FT_Size_RequestRec_32_t;


void inplace_FT_GlyphSlot_shrink(void* a)
{
    if(!a) return;
    FT_GlyphSlotRec_t* src = a;
    FT_GlyphSlotRec_32_t* dst = a;
    void* next = src->next;

    dst->library = to_ptrv(src->library);
    dst->face = to_ptrv(src->face);
    dst->next = to_ptrv(src->next);
    dst->glyph_index = src->glyph_index;
    dst->generic.data = to_ptrv(src->generic.data);
    dst->generic.finalizer = to_ptrv(src->generic.finalizer);
    dst->metrics.width = to_long(src->metrics.width);
    dst->metrics.height = to_long(src->metrics.height);
    dst->metrics.horiBearingX = to_long(src->metrics.horiBearingX);
    dst->metrics.horiBearingY = to_long(src->metrics.horiBearingY);
    dst->metrics.horiAdvance = to_long(src->metrics.horiAdvance);
    dst->metrics.vertBearingX = to_long(src->metrics.vertBearingX);
    dst->metrics.vertBearingY = to_long(src->metrics.vertBearingY);
    dst->metrics.vertAdvance = to_long(src->metrics.vertAdvance);
    dst->linearHoriAdvance = to_long(src->linearHoriAdvance);
    dst->linearVertAdvance = to_long(src->linearVertAdvance);
    dst->advance.x = to_long(src->advance.x);
    dst->advance.y = to_long(src->advance.y);
    dst->format = src->format;
    dst->bitmap.rows = src->bitmap.rows;
    dst->bitmap.width = src->bitmap.width;
    dst->bitmap.pitch = src->bitmap.pitch;
    dst->bitmap.buffer = to_ptrv(src->bitmap.buffer);
    dst->bitmap.num_grays = src->bitmap.num_grays;
    dst->bitmap.pixel_mode = src->bitmap.pixel_mode;
    dst->bitmap.palette_mode = src->bitmap.palette_mode;
    dst->bitmap.palette = to_ptrv(src->bitmap.palette);
    dst->bitmap_left = src->bitmap_left;
    dst->bitmap_top = src->bitmap_top;
    dst->outline.n_contours = src->outline.n_contours;
    for(int i=0; i<src->outline.n_points; ++i) {
        ((FT_Vector_32_t*)src->outline.points)[i].x = to_long(src->outline.points[i].x);
        ((FT_Vector_32_t*)src->outline.points)[i].y = to_long(src->outline.points[i].y);
    }
    dst->outline.n_points = src->outline.n_points;
    dst->outline.points = to_ptrv(src->outline.points);
    dst->outline.tags = to_ptrv(src->outline.tags);
    dst->outline.contours = to_ptrv(src->outline.contours);
    dst->outline.flags = src->outline.flags;
    dst->num_subglyphs = src->num_subglyphs;
    dst->subglyphs = to_ptrv(src->subglyphs);
    dst->control_data = to_ptrv(src->control_data);
    dst->control_len = to_long(src->control_len);
    dst->lsb_delta = to_long(src->lsb_delta);
    dst->rsb_delta = to_long(src->rsb_delta);
    dst->other = to_ptrv(src->other);
    dst->internal = to_ptrv(src->internal);

    inplace_FT_GlyphSlot_shrink(next);
}

void inplace_FT_GlyphSlot_enlarge(void* a)
{
    if(!a) return;
    FT_GlyphSlotRec_32_t* src = a;
    FT_GlyphSlotRec_t* dst = a;
    void* next = from_ptrv(src->next);

    dst->internal = from_ptrv(src->internal);
    dst->other = from_ptrv(src->other);
    dst->rsb_delta = from_long(src->rsb_delta);
    dst->lsb_delta = from_long(src->lsb_delta);
    dst->control_len = from_long(src->control_len);
    dst->control_data = from_ptrv(src->control_data);
    dst->subglyphs = from_ptrv(src->subglyphs);
    dst->num_subglyphs = src->num_subglyphs;
    dst->outline.flags = src->outline.flags;
    dst->outline.contours = from_ptrv(src->outline.contours);
    dst->outline.tags = from_ptrv(src->outline.tags);
    dst->outline.points = from_ptrv(src->outline.points);
    dst->outline.n_points = src->outline.n_points;
    for(int i=dst->outline.n_points-1; i>=0; --i) {
        dst->outline.points[i].y = from_long(((FT_Vector_32_t*)dst->outline.points)[i].y);
        dst->outline.points[i].x = from_long(((FT_Vector_32_t*)dst->outline.points)[i].x);
    }
    dst->outline.n_contours = src->outline.n_contours;
    dst->bitmap_top = src->bitmap_top;
    dst->bitmap_left = src->bitmap_left;
    dst->bitmap.palette = from_ptrv(src->bitmap.palette);
    dst->bitmap.palette_mode = src->bitmap.palette_mode;
    dst->bitmap.pixel_mode = src->bitmap.pixel_mode;
    dst->bitmap.num_grays = src->bitmap.num_grays;
    dst->bitmap.buffer = from_ptrv(src->bitmap.buffer);
    dst->bitmap.pitch = src->bitmap.pitch;
    dst->bitmap.width = src->bitmap.width;
    dst->bitmap.rows = src->bitmap.rows;
    dst->format = src->format;
    dst->advance.y = from_long(src->advance.y);
    dst->advance.x = from_long(src->advance.x);
    dst->linearVertAdvance = from_long(src->linearVertAdvance);
    dst->linearHoriAdvance = from_long(src->linearHoriAdvance);
    dst->metrics.vertAdvance = from_long(src->metrics.vertAdvance);
    dst->metrics.vertBearingY = from_long(src->metrics.vertBearingY);
    dst->metrics.vertBearingX = from_long(src->metrics.vertBearingX);
    dst->metrics.horiAdvance = from_long(src->metrics.horiAdvance);
    dst->metrics.horiBearingY = from_long(src->metrics.horiBearingY);
    dst->metrics.horiBearingX = from_long(src->metrics.horiBearingX);
    dst->metrics.height = from_long(src->metrics.height);
    dst->metrics.width = from_long(src->metrics.width);
    dst->generic.finalizer = from_ptrv(src->generic.finalizer);
    dst->generic.data = from_ptrv(src->generic.data);
    dst->glyph_index = src->glyph_index;
    dst->next = from_ptrv(src->next); // no shinking of the whole chain?
    dst->face = from_ptrv(src->face);
    dst->library = from_ptrv(src->library);

    inplace_FT_GlyphSlot_enlarge(next);
}

// Convertion function
void inplace_FT_FaceRec_shrink(void* a)
{
    if(!a) return;
    FT_FaceRec_t* src = a;
    FT_FaceRec_32_t* dst = a;
    void* glyphslot = src->glyph;

    dst->num_faces = to_long(src->num_faces);
    dst->face_index = to_long(src->face_index);
    dst->face_flags = to_long(src->face_flags);
    dst->style_flags = to_long(src->style_flags);
    dst->num_glyphs = to_long(src->num_glyphs);
    dst->family_name = to_ptrv(src->family_name);
    dst->style_name = to_ptrv(src->style_name);
    {
        FT_Bitmap_Size_32_t* dst_sizes = (void*)src->available_sizes;
        for(int i=0; i<src->num_fixed_sizes; ++i) {
            dst_sizes[i].height = src->available_sizes[i].height;
            dst_sizes[i].width = src->available_sizes[i].width;
            dst_sizes[i].size = to_long(src->available_sizes[i].size);
            dst_sizes[i].x_ppem = to_long(src->available_sizes[i].x_ppem);
            dst_sizes[i].y_ppem = to_long(src->available_sizes[i].y_ppem);
        }
    }
    dst->num_fixed_sizes = src->num_fixed_sizes;
    dst->available_sizes = to_ptrv(src->available_sizes);
    dst->num_charmaps = src->num_charmaps;
    dst->charmaps = to_ptrv(src->charmaps);
    dst->generic.data = to_ptrv(src->generic.data);
    dst->generic.finalizer = to_ptrv(src->generic.finalizer);
    dst->bbox.xMin = to_long(src->bbox.xMin);
    dst->bbox.yMin = to_long(src->bbox.yMin);
    dst->bbox.xMax = to_long(src->bbox.xMax);
    dst->bbox.yMax = to_long(src->bbox.yMax);
    dst->units_per_EM = src->units_per_EM;
    dst->ascender = src->ascender;
    dst->descender = src->descender;
    dst->height = src->height;
    dst->max_advance_width = src->max_advance_width;
    dst->max_advance_height = src->max_advance_height;
    dst->underline_position = src->underline_position;
    dst->underline_thickness = src->underline_thickness;
    dst->glyph = to_ptrv(src->glyph);
    dst->size = to_ptrv(src->size);
    dst->charmap = to_ptrv(src->charmap);
    dst->driver = to_ptrv(src->driver);
    dst->memory = to_ptrv(src->memory);
    ((FT_StreamDesc_32_t*)src->stream)->value = to_long(src->stream->value);
    ((FT_StreamDesc_32_t*)src->stream)->pointer = to_ptrv(src->stream->pointer);
    dst->stream = to_ptrv(src->stream);
    dst->sizes_list.head = to_ptrv(src->sizes_list.head);
    dst->sizes_list.tail = to_ptrv(src->sizes_list.tail);
    dst->autohint.data = to_ptrv(src->autohint.data);
    dst->autohint.finalizer = to_ptrv(src->autohint.finalizer);
    dst->extensions = to_ptrv(src->extensions);
    dst->internal = to_ptrv(src->internal);

    inplace_FT_GlyphSlot_shrink(glyphslot);
}

void inplace_FT_FaceRec_enlarge(void* a)
{
    if(!a) return;
    FT_FaceRec_32_t* src = a;
    FT_FaceRec_t* dst = a;
    void* glyphslot = from_ptrv(src->glyph);

    dst->internal = from_ptrv(src->internal);
    dst->extensions = from_ptrv(src->extensions);
    dst->autohint.finalizer = from_ptrv(src->autohint.finalizer);
    dst->autohint.data = from_ptrv(src->autohint.data);
    dst->sizes_list.tail = from_ptrv(src->sizes_list.tail);
    dst->sizes_list.head = from_ptrv(src->sizes_list.head);
    dst->stream = from_ptrv(src->stream);
    dst->stream->pointer = from_ptrv(((FT_StreamDesc_32_t*)dst->stream)->pointer);
    dst->stream->value = from_long(((FT_StreamDesc_32_t*)dst->stream)->value);
    dst->memory = from_ptrv(src->memory);
    dst->driver = from_ptrv(src->driver);
    dst->charmap = from_ptrv(src->charmap);
    dst->size = from_ptrv(src->size);
    dst->glyph = from_ptrv(src->glyph);
    dst->underline_thickness = src->underline_thickness;
    dst->underline_position = src->underline_position;
    dst->max_advance_height = src->max_advance_height;
    dst->max_advance_width = src->max_advance_width;
    dst->height = src->height;
    dst->descender = src->descender;
    dst->ascender = src->ascender;
    dst->units_per_EM = src->units_per_EM;
    dst->bbox.yMax = from_long(src->bbox.yMax);
    dst->bbox.xMax = from_long(src->bbox.xMax);
    dst->bbox.yMin = from_long(src->bbox.yMin);
    dst->bbox.xMin = from_long(src->bbox.xMin);
    dst->generic.finalizer = from_ptrv(src->generic.finalizer);
    dst->generic.data = from_ptrv(src->generic.data);
    dst->charmaps = from_ptrv(src->charmaps);
    dst->num_charmaps = src->num_charmaps;
    dst->available_sizes = from_ptrv(src->available_sizes);
    dst->num_fixed_sizes = src->num_fixed_sizes;
    {
        FT_Bitmap_Size_32_t* src_sizes = (void*)dst->available_sizes;
        for(int i=dst->num_fixed_sizes-1; i>=0; --i) {
            dst->available_sizes[i].height = src_sizes[i].height;
            dst->available_sizes[i].width = src_sizes[i].width;
            dst->available_sizes[i].size = from_long(src_sizes[i].size);
            dst->available_sizes[i].x_ppem = from_long(src_sizes[i].x_ppem);
            dst->available_sizes[i].y_ppem = from_long(src_sizes[i].y_ppem);
        }
    }
    dst->style_name = from_ptrv(src->style_name);
    dst->family_name = from_ptrv(src->family_name);
    dst->num_glyphs = from_long(src->num_glyphs);
    dst->style_flags = from_long(src->style_flags);
    dst->face_flags = from_long(src->face_flags);
    dst->face_index = from_long(src->face_index);
    dst->num_faces = from_long(src->num_faces);

    inplace_FT_GlyphSlot_enlarge(glyphslot);
}

void convert_PS_PrivateRec_to_32(void* d, void* s)
{
    PS_PrivateRec_t* src = s;
    PS_PrivateRec_32_t* dst = d;

    dst->unique_id = src->unique_id;
    dst->lenIV = src->lenIV;
    dst->num_blue_values = src->num_blue_values;
    dst->num_other_blues = src->num_other_blues;
    dst->num_family_blues = src->num_family_blues;
    dst->num_family_other_blues = src->num_family_other_blues;
    memcpy(dst->blue_values, src->blue_values, sizeof(dst->blue_values));
    memcpy(dst->other_blues, src->other_blues, sizeof(dst->other_blues));
    memcpy(dst->family_blues, src->family_blues, sizeof(dst->family_blues));
    memcpy(dst->family_other_blues, src->family_other_blues, sizeof(dst->family_other_blues));
    dst->blue_scale = src->blue_scale;
    dst->blue_shift = src->blue_shift;
    dst->blue_fuzz = src->blue_fuzz;
    dst->standard_width[0] = src->standard_width[0];
    dst->standard_height[0] = src->standard_height[0];
    dst->num_snap_widths = src->num_snap_widths;
    dst->num_snap_heights = src->num_snap_heights;
    dst->force_bold = src->force_bold;
    dst->round_stem_up = src->round_stem_up;
    memcpy(dst->snap_widths, src->snap_widths, sizeof(dst->snap_widths));
    memcpy(dst->snap_heights, src->snap_heights, sizeof(dst->snap_heights));
    dst->expansion_factor = src->expansion_factor;
    dst->language_group = src->language_group;
    dst->password = src->password;
    dst->min_feature[0] = src->min_feature[0];
    dst->min_feature[1] = src->min_feature[1];
}

void convert_BDF_PropertyRec_to_32(void* d, void* s)
{
    BDF_PropertyRec_t* src = s;
    BDF_PropertyRec_32_t* dst = d;

    dst->type = src->type;
    switch(dst->type) {
        case 0: break;
        case 1: dst->u.atom = to_ptrv(src->u.atom); break;
        case 2: dst->u.integer = src->u.integer; break;
        case 3: dst->u.cardinal = src->u.cardinal; break;
    }
}

void convert_FT_Size_RequestRec_to_64(void* d, void* s)
{
    FT_Size_RequestRec_t* src = s;
    FT_Size_RequestRec_32_t* dst = d;

    dst->type = src->type;
    dst->width = from_long(src->width);
    dst->height = from_long(src->height);
    dst->horiResolution = src->horiResolution;
    dst->vertResolution = src->vertResolution;
}

#define ADDED_FUNCTIONS()                   \

#include "generated/wrappedfreetypetypes32.h"

#include "wrappercallback32.h"

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// FT_Generic_Finalizer
#define GO(A)   \
static uintptr_t my_FT_Generic_Finalizer_fct_##A = 0;                         \
static void my_FT_Generic_Finalizer_##A(void* object)                         \
{                                                                             \
    RunFunctionFmt(my_FT_Generic_Finalizer_fct_##A, "p", object);         \
}
SUPER()
#undef GO
static void* find_FT_Generic_Finalizer_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_FT_Generic_Finalizer_fct_##A == (uintptr_t)fct) return my_FT_Generic_Finalizer_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FT_Generic_Finalizer_fct_##A == 0) {my_FT_Generic_Finalizer_fct_##A = (uintptr_t)fct; return my_FT_Generic_Finalizer_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libfreetype FT_Generic_Finalizer callback\n");
    return NULL;
}
// FTC_Face_Requester
#define GO(A)   \
static uintptr_t my_FTC_Face_Requester_fct_##A = 0;                                                             \
static int my_FTC_Face_Requester_##A(void* face_id, void* lib, void* req, void* aface)                          \
{                                                                                                               \
    int ret = (int)RunFunctionFmt(my_FTC_Face_Requester_fct_##A, "pppp", face_id, lib, req, aface);       \
    if(aface && *(void**)aface) {                                                                               \
        FT_FaceRec_t *f = *(FT_FaceRec_t**)aface;                                                               \
        f->generic.finalizer = find_FT_Generic_Finalizer_Fct(f->generic.finalizer);                             \
    }                                                                                                           \
    return ret;                                                                                                 \
}
SUPER()
#undef GO
static void* find_FTC_Face_Requester_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_FTC_Face_Requester_fct_##A == (uintptr_t)fct) return my_FTC_Face_Requester_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FTC_Face_Requester_fct_##A == 0) {my_FTC_Face_Requester_fct_##A = (uintptr_t)fct; return my_FTC_Face_Requester_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libfreetype FTC_Face_Requester callback\n");
    return NULL;
}
// FT_Alloc
#define GO(A)   \
static uintptr_t my_FT_Alloc_fct_##A = 0;                                              \
static void* my_FT_Alloc_##A(void* memory, long size)                                  \
{                                                                                      \
    return (void*)RunFunctionFmt(my_FT_Alloc_fct_##A, "pl", memory, size);        \
}
SUPER()
#undef GO
static void* find_FT_Alloc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_FT_Alloc_fct_##A == (uintptr_t)fct) return my_FT_Alloc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FT_Alloc_fct_##A == 0) {my_FT_Alloc_fct_##A = (uintptr_t)fct; return my_FT_Alloc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libfreetype FT_Alloc callback\n");
    return NULL;
}
// FT_Free
#define GO(A)   \
static uintptr_t my_FT_Free_fct_##A = 0;                             \
static void my_FT_Free_##A(void* memory, void* p)                    \
{                                                                    \
    RunFunctionFmt(my_FT_Free_fct_##A, "pp", memory, p);      \
}
SUPER()
#undef GO
static void* find_FT_Free_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_FT_Free_fct_##A == (uintptr_t)fct) return my_FT_Free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FT_Free_fct_##A == 0) {my_FT_Free_fct_##A = (uintptr_t)fct; return my_FT_Free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libfreetype FT_Free callback\n");
    return NULL;
}
// FT_Realloc
#define GO(A)   \
static uintptr_t my_FT_Realloc_fct_##A = 0;                                                        \
static void* my_FT_Realloc_##A(void* memory, long cur, long size, void* p)                         \
{                                                                                                  \
    return (void*)RunFunctionFmt(my_FT_Realloc_fct_##A, "pllp", memory, cur, size, p);        \
}
SUPER()
#undef GO
static void* find_FT_Realloc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_FT_Realloc_fct_##A == (uintptr_t)fct) return my_FT_Realloc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FT_Realloc_fct_##A == 0) {my_FT_Realloc_fct_##A = (uintptr_t)fct; return my_FT_Realloc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libfreetype FT_Realloc callback\n");
    return NULL;
}
// FT_Outline_MoveToFunc
#define GO(A)   \
static uintptr_t my_FT_Outline_MoveToFunc_fct_##A = 0;                                        \
static int my_FT_Outline_MoveToFunc_##A(void* to, void* user)                                 \
{                                                                                             \
    return (int)RunFunctionFmt(my_FT_Outline_MoveToFunc_fct_##A, "pp", to, user);         \
}
SUPER()
#undef GO
static void* find_FT_Outline_MoveToFunc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_FT_Outline_MoveToFunc_fct_##A == (uintptr_t)fct) return my_FT_Outline_MoveToFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FT_Outline_MoveToFunc_fct_##A == 0) {my_FT_Outline_MoveToFunc_fct_##A = (uintptr_t)fct; return my_FT_Outline_MoveToFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libfreetype FT_Outline_MoveToFunc callback\n");
    return NULL;
}

// FT_Outline_LineToFunc
#define GO(A)   \
static uintptr_t my_FT_Outline_LineToFunc_fct_##A = 0;                                        \
static int my_FT_Outline_LineToFunc_##A(void* to, void* user)                                 \
{                                                                                             \
    return (int)RunFunctionFmt(my_FT_Outline_LineToFunc_fct_##A, "pp", to, user);         \
}
SUPER()
#undef GO
static void* find_FT_Outline_LineToFunc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_FT_Outline_LineToFunc_fct_##A == (uintptr_t)fct) return my_FT_Outline_LineToFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FT_Outline_LineToFunc_fct_##A == 0) {my_FT_Outline_LineToFunc_fct_##A = (uintptr_t)fct; return my_FT_Outline_LineToFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libfreetype FT_Outline_LineToFunc callback\n");
    return NULL;
}

// FT_Outline_ConicToFunc
#define GO(A)   \
static uintptr_t my_FT_Outline_ConicToFunc_fct_##A = 0;                                              \
static int my_FT_Outline_ConicToFunc_##A(void* ctl, void* to, void* user)                            \
{                                                                                                    \
    return (int)RunFunctionFmt(my_FT_Outline_ConicToFunc_fct_##A, "ppp", ctl, to, user);      \
}
SUPER()
#undef GO
static void* find_FT_Outline_ConicToFunc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_FT_Outline_ConicToFunc_fct_##A == (uintptr_t)fct) return my_FT_Outline_ConicToFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FT_Outline_ConicToFunc_fct_##A == 0) {my_FT_Outline_ConicToFunc_fct_##A = (uintptr_t)fct; return my_FT_Outline_ConicToFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libfreetype FT_Outline_ConicToFunc callback\n");
    return NULL;
}

// FT_Outline_CubicToFunc
#define GO(A)   \
static uintptr_t my_FT_Outline_CubicToFunc_fct_##A = 0;                                                      \
static int my_FT_Outline_CubicToFunc_##A(void* ctl1, void* ctl2, void* to, void* user)                       \
{                                                                                                            \
    return (int)RunFunctionFmt(my_FT_Outline_CubicToFunc_fct_##A, "pppp", ctl1, ctl2, to, user);      \
}
SUPER()
#undef GO
static void* find_FT_Outline_CubicToFunc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_FT_Outline_CubicToFunc_fct_##A == (uintptr_t)fct) return my_FT_Outline_CubicToFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_FT_Outline_CubicToFunc_fct_##A == 0) {my_FT_Outline_CubicToFunc_fct_##A = (uintptr_t)fct; return my_FT_Outline_CubicToFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libfreetype FT_Outline_CubicToFunc callback\n");
    return NULL;
}

// structures
#define GO(A)   \
static FT_MemoryRec_t my_FT_MemoryRec_struct_##A = {0}; \
static FT_MemoryRec_t* my_FT_MemoryRec_ref_##A = NULL;
SUPER()
#undef GO
static void wrap_FT_MemoryRec(FT_MemoryRec_t* dst, FT_MemoryRec_t* src) {
    dst->user = src->user;
    dst->alloc = find_FT_Alloc_Fct(src->alloc);
    dst->free = find_FT_Free_Fct(src->free);
    dst->realloc = find_FT_Realloc_Fct(src->realloc);
}
static FT_MemoryRec_t* find_FT_MemoryRec_Struct(FT_MemoryRec_t* s)
{
    if(!s)  return NULL;
    #define GO(A) if(my_FT_MemoryRec_ref_##A == s) return &my_FT_MemoryRec_struct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(!my_FT_MemoryRec_ref_##A) {wrap_FT_MemoryRec(&my_FT_MemoryRec_struct_##A, s); my_FT_MemoryRec_ref_##A = s; return &my_FT_MemoryRec_struct_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libfreetype Struct FT_MemoryRec wrapping\n");
    return s;
}


#undef SUPER

//static uintptr_t my_iofunc = 0;
//static unsigned long my_FT_Stream_IoFunc(FT_StreamRec_t* stream, unsigned long offset, unsigned char* buffer, unsigned long count )
//{
//    return (unsigned long)RunFunctionFmt(my_iofunc, "pLpL", stream, offset, buffer, count)        ;
//}

//static uintptr_t my_closefunc = 0;
//static void my_FT_Stream_CloseFunc(FT_StreamRec_t* stream)
//{
//    RunFunctionFmt(my_closefunc, "p", stream)     ;
//}

//EXPORT int my_FT_Open_Face(x64emu_t* emu, void* library, FT_Open_Args_t* args, long face_index, void* aface)
//{
//    (void)emu;
//    int wrapstream = (args->flags&0x02)?1:0;
//    if(wrapstream) {
//        my_iofunc = (uintptr_t)args->stream->read;
//        if(my_iofunc)
//            args->stream->read = my_FT_Stream_IoFunc;
//        my_closefunc = (uintptr_t)args->stream->close;
//        if(my_closefunc)
//            args->stream->close = my_FT_Stream_CloseFunc;
//    }
//    int ret = my->FT_Open_Face(library, args, face_index, aface);
//    /*if(wrapstream) {
//        args->stream->read = (void*)my_iofunc;
//        args->stream->close = (void*)my_closefunc;
//    }*/
//    return ret;
//}

//EXPORT int my_FTC_Manager_New(x64emu_t* emu, void* l, uint32_t max_faces, uint32_t max_sizes, uintptr_t max_bytes, void* req, void* data, void* aman)
//{
//    (void)emu;
//    return my->FTC_Manager_New(l, max_faces, max_sizes, max_bytes, find_FTC_Face_Requester_Fct(req), data, aman);
//}

//EXPORT int my_FT_New_Library(x64emu_t* emu, FT_MemoryRec_t* memory, void* p)
//{
//    return my->FT_New_Library(find_FT_MemoryRec_Struct(memory), p);
//}

typedef struct  my_FT_Outline_Funcs_s
{
    void*       move_to;
    void*       line_to;
    void*       conic_to;
    void*       cubic_to;
    int         shift;
    signed long delta;
} my_FT_Outline_Funcs_t;

//EXPORT int my32_FT_Outline_Decompose(x64emu_t* emu, void* outline, my_FT_Outline_Funcs_t* tbl, void* data)
//{
//    my_FT_Outline_Funcs_t f = {0};
//    if(tbl) {
//        f.move_to = find_FT_Outline_MoveToFunc_Fct(tbl->move_to);
//        f.line_to = find_FT_Outline_LineToFunc_Fct(tbl->line_to);
//        f.conic_to = find_FT_Outline_ConicToFunc_Fct(tbl->conic_to);
//        f.cubic_to = find_FT_Outline_CubicToFunc_Fct(tbl->cubic_to);
//        f.shift = tbl->shift;
//        f.delta = tbl->delta;
//    }
//    return my->FT_Outline_Decompose(outline, tbl?(&f):tbl, data);
//}

EXPORT int my32_FT_New_Face(x64emu_t* emu, void* lib, void* name, long index, ptr_t* face)
{
    FT_FaceRec_t* res = NULL;
    int ret = my->FT_New_Face(lib, name, index, &res);
    if(ret) return ret;
    *face = to_ptrv(res);
    inplace_FT_FaceRec_shrink(res);
    return ret;
}

EXPORT int my32_FT_New_Memory_Face(x64emu_t* emu, void* lib, void* base, long size, long index, ptr_t* face)
{
    FT_FaceRec_t* res = NULL;
    int ret = my->FT_New_Memory_Face(lib, base, size, index, &res);
    if(ret) return ret;
    *face = to_ptrv(res);
    inplace_FT_FaceRec_shrink(res);
    return ret;
}

EXPORT uint32_t my32_FT_Get_Sfnt_Name_Count(x64emu_t* emu, void* face)
{
    inplace_FT_FaceRec_enlarge(face);
    uint32_t ret = my->FT_Get_Sfnt_Name_Count(face);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT uint32_t my32_FT_Get_Sfnt_Name(x64emu_t* emu, void* face, uint32_t idx, void* name)
{
    inplace_FT_FaceRec_enlarge(face);
    uint32_t ret = my->FT_Get_Sfnt_Name(face, idx, name);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT int my32_FT_Get_Glyph_Name(x64emu_t* emu, void* face, uint32_t index, void* buff, uint32_t size)
{
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Get_Glyph_Name(face, index, buff, size);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT void* my32_FT_Get_X11_Font_Format(x64emu_t* emu, void* face)
{
    inplace_FT_FaceRec_enlarge(face);
    void* ret = my->FT_Get_X11_Font_Format(face);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT void* my32_FT_Get_Postscript_Name(x64emu_t* emu, void* face)
{
    inplace_FT_FaceRec_enlarge(face);
    void* ret = my->FT_Get_Postscript_Name(face);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT int my32_FT_Load_Sfnt_Table(x64emu_t* emu, void* face, unsigned long tag, long offset, void*  buff, ulong_t* length)
{
    unsigned long len_l = 0;
    if(length) len_l = from_ulong(*length);
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Load_Sfnt_Table(face, tag, offset, buff, length?(&len_l):NULL);
    inplace_FT_FaceRec_shrink(face);
    if(length) *length = to_ulong(len_l);
    return ret;
}

EXPORT unsigned long my32_FT_Get_First_Char(x64emu_t* emu, void* face, uint32_t* index)
{
    inplace_FT_FaceRec_enlarge(face);
    unsigned long ret = my->FT_Get_First_Char(face, index);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT int my32_FT_Get_PS_Font_Info(x64emu_t* emu, void* face, PS_PrivateRec_32_t* info)
{
    PS_PrivateRec_t info_l = {0};
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Get_PS_Font_Info(face, &info_l);
    inplace_FT_FaceRec_shrink(face);
    convert_PS_PrivateRec_to_32(info, &info_l);
    return ret;
}

EXPORT int my32_FT_Select_Charmap(x64emu_t* emu, void* face, int encoding)
{
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Select_Charmap(face, encoding);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT void* my32_FT_Get_Sfnt_Table(x64emu_t* emu, void* face, int tag)
{
    inplace_FT_FaceRec_enlarge(face);
    void* ret = my->FT_Get_Sfnt_Table(face, tag);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT int my32_FT_Select_Size(x64emu_t* emu, void* face, int index)
{
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Select_Size(face, index);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT unsigned long my32_FT_Get_Next_Char(x64emu_t* emu, void* face, unsigned long code, void* buff)
{
    inplace_FT_FaceRec_enlarge(face);
    unsigned long ret = my->FT_Get_Next_Char(face, code, buff);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT int my32_FT_Has_PS_Glyph_Names(x64emu_t* emu, void* face)
{
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Has_PS_Glyph_Names(face);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT int my32_FT_Get_BDF_Property(x64emu_t* emu, void* face, void* name, BDF_PropertyRec_32_t* prop)
{
    BDF_PropertyRec_t prop_l = {0};
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Get_BDF_Property(face, name, &prop_l);
    inplace_FT_FaceRec_shrink(face);
    convert_BDF_PropertyRec_to_32(prop, &prop_l);
    return ret;
}

EXPORT int my32_FT_Done_Face(x64emu_t* emu, void* face)
{
    inplace_FT_FaceRec_enlarge(face);
    return my->FT_Done_Face(face);
}

EXPORT int my32_FT_Get_Kerning(x64emu_t* emu, void* face, uint32_t left, uint32_t right, uint32_t kern, FT_Vector_32_t* kerning)
{
    FT_Vector_t kerning_l = {0};
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Get_Kerning(face, left, right, kern, &kerning_l);
    inplace_FT_FaceRec_shrink(face);
    kerning->x = kerning_l.x;
    kerning->y = kerning_l.y;
    return ret;
}

EXPORT int my32_FT_Set_Char_Size(x64emu_t* emu, void* face, long char_width, long char_height, uint32_t horz, uint32_t vert)
{
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Set_Char_Size(face, char_width, char_height, horz, vert);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT int my32_FT_Set_Pixel_Sizes(x64emu_t* emu, void* face, uint32_t width, uint32_t height)
{
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Set_Pixel_Sizes(face, width, height);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT int my32_FT_Request_Size(x64emu_t* emu, void* face, FT_Size_RequestRec_32_t* req)
{
    FT_Size_RequestRec_t req_l = {0};
    convert_FT_Size_RequestRec_to_64(&req_l, req);
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Request_Size(face, &req_l);
    inplace_FT_FaceRec_enlarge(face);
    return ret;
}

EXPORT uint32_t my32_FT_Get_Char_Index(x64emu_t* emu, void* face, unsigned long code)
{
    inplace_FT_FaceRec_enlarge(face);
    uint32_t ret = my->FT_Get_Char_Index(face, code);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT int my32_FT_Load_Char(x64emu_t* emu, void* face, unsigned long code, int flags)
{
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Load_Char(face, code, flags);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT int my32_FT_Load_Glyph(x64emu_t* emu, void* face, uint32_t index, int flags)
{
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Load_Glyph(face, index, flags);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT void my32_FT_Outline_Get_CBox(x64emu_t* emu, FT_Outline_32_t* outline, FT_BBox_32_t* bbox)
{
    int n = outline->n_points;
    FT_Outline_t outline_l;
    FT_Vector_t vector[n];
    outline_l.n_contours = outline->n_contours;
    outline_l.n_points = outline->n_points;
    outline_l.points = vector;
    FT_Vector_32_t* vec = from_ptrv(outline->points);
    for(int i=0; i<n; ++i) {
        vector[i].x = from_long(vec[i].x);
        vector[i].y = from_long(vec[i].y);
    }
    outline_l.tags = from_ptrv(outline->tags);
    outline_l.contours = from_ptrv(outline->contours);
    outline_l.flags = outline->flags;
    FT_BBox_t res = {0};
    my->FT_Outline_Get_CBox(&outline_l, &res);
    bbox->xMin = to_long(res.xMin);
    bbox->yMin = to_long(res.yMin);
    bbox->xMax = to_long(res.xMax);
    bbox->yMax = to_long(res.yMax);
}

EXPORT int my32_FT_Render_Glyph(x64emu_t* emu, FT_GlyphSlotRec_32_t* glyph, uint32_t mode)
{
    inplace_FT_GlyphSlot_enlarge(glyph);
    int ret = my->FT_Render_Glyph(glyph, mode);
    inplace_FT_GlyphSlot_shrink(glyph);
    return ret;
}

#include "wrappedlib_init32.h"
