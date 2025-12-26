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
#include "converter32.h"

#include "khash.h"

static const char* freetypeName = 
#ifdef ANDROID
    "libfreetype.so"
#else
    "libfreetype.so.6"
#endif
    ;
#define LIBNAME freetype

typedef void  (*vFp_t)(void*);

#define FT_GLYPH_FORMAT_OUTLINE (('o'<<24)|('u'<<16)|('t'<<8)|'l')
#define FT_GLYPH_FORMAT_BITMAP  (('b'<<24)|('i'<<16)|('t'<<8)|'s')

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

typedef struct  FT_CharMapRec_s
{
    void*       face;   //FT_FaceRec_t*
    int         encoding;
    uint16_t    platform_id;
    uint16_t    encoding_id;
} FT_CharMapRec_t;

typedef struct  FT_Size_Metrics_s
{
    uint16_t    x_ppem;
    uint16_t    y_ppem;
    long        x_scale;
    long        y_scale;
    long        ascender;
    long        descender;
    long        height;
    long        max_advance;
} FT_Size_Metrics_t;

typedef struct  FT_SizeRec_s
{
    void*               face;   //FT_FaceRec_t*
    FT_Generic_t        generic;
    FT_Size_Metrics_t   metrics;
    void*               internal;   //FT_Size_Internal
} FT_SizeRec_t;

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
    FT_CharMapRec_t**   charmaps;
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
    FT_SizeRec_t*       size;
    void*               charmap;    //FT_CharMapRec_t*
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

typedef struct  FT_Parameter_s
{
    unsigned long   tag;
    void*           data;
} FT_Parameter_t;

typedef struct  FT_Open_Args_s
{
    uint32_t        flags;
    uint8_t*        memory_base;
    long            memory_size;
    char*           pathname;
    FT_StreamRec_t* stream;
    void*           driver; //FT_ModuleRec
    int             num_params;
    FT_Parameter_t* params;
} FT_Open_Args_t;

typedef struct  FT_WinFNT_HeaderRec_s
{
    uint16_t        version;
    unsigned long   file_size;
    uint8_t         copyright[60];
    uint16_t        file_type;
    uint16_t        nominal_point_size;
    uint16_t        vertical_resolution;
    uint16_t        horizontal_resolution;
    uint16_t        ascent;
    uint16_t        internal_leading;
    uint16_t        external_leading;
    uint8_t         italic;
    uint8_t         underline;
    uint8_t         strike_out;
    uint16_t        weight;
    uint8_t         charset;
    uint16_t        pixel_width;
    uint16_t        pixel_height;
    uint8_t         pitch_and_family;
    uint16_t        avg_width;
    uint16_t        max_width;
    uint8_t         first_char;
    uint8_t         last_char;
    uint8_t         default_char;
    uint8_t         break_char;
    uint16_t        bytes_per_row;
    unsigned long   device_offset;
    unsigned long   face_name_offset;
    unsigned long   bits_pointer;
    unsigned long   bits_offset;
    uint8_t         reserved;
    unsigned long   flags;
    uint16_t        A_space;
    uint16_t        B_space;
    uint16_t        C_space;
    uint16_t        color_table_offset;
    unsigned long   reserved1[4];
} FT_WinFNT_HeaderRec_t;

typedef struct  FT_Matrix_s
{
    long    xx, xy;
    long    yx, yy;
} FT_Matrix_t;

typedef struct  FT_GlyphRec_s
{
    void*           library;  //FT_Library
    void*           clazz;  //const FT_Glyph_Class*
    uint32_t        format;
    FT_Vector_t     advance;
} FT_GlyphRec_t;

typedef struct  FT_BitmapGlyphRec_s
{
    FT_GlyphRec_t   root;
    int             left;
    int             top;
    FT_Bitmap_t     bitmap;
} FT_BitmapGlyphRec_t;

typedef struct  TT_Header_s
{
    signed long     Table_Version;
    signed long     Font_Revision;
    long            CheckSum_Adjust;
    long            Magic_Number;
    uint16_t        Flags;
    uint16_t        Units_Per_EM;
    unsigned long   Created [2];
    unsigned long   Modified[2];
    int16_t         xMin;
    int16_t         yMin;
    int16_t         xMax;
    int16_t         yMax;
    uint16_t        Mac_Style;
    uint16_t        Lowest_Rec_PPEM;
    int16_t         Font_Direction;
    int16_t         Index_To_Loc_Format;
    int16_t         Glyph_Data_Format;
} TT_Header_t;
typedef struct  TT_MaxProfile_s
{
    signed long version;
    uint16_t    numGlyphs;
    uint16_t    maxPoints;
    uint16_t    maxContours;
    uint16_t    maxCompositePoints;
    uint16_t    maxCompositeContours;
    uint16_t    maxZones;
    uint16_t    maxTwilightPoints;
    uint16_t    maxStorage;
    uint16_t    maxFunctionDefs;
    uint16_t    maxInstructionDefs;
    uint16_t    maxStackElements;
    uint16_t    maxSizeOfInstructions;
    uint16_t    maxComponentElements;
    uint16_t    maxComponentDepth;
} TT_MaxProfile_t;
typedef struct  TT_OS2_s
{
    uint16_t        version;
    int16_t         xAvgCharWidth;
    uint16_t        usWeightClass;
    uint16_t        usWidthClass;
    uint16_t        fsType;
    int16_t         ySubscriptXSize;
    int16_t         ySubscriptYSize;
    int16_t         ySubscriptXOffset;
    int16_t         ySubscriptYOffset;
    int16_t         ySuperscriptXSize;
    int16_t         ySuperscriptYSize;
    int16_t         ySuperscriptXOffset;
    int16_t         ySuperscriptYOffset;
    int16_t         yStrikeoutSize;
    int16_t         yStrikeoutPosition;
    int16_t         sFamilyClass;
    uint8_t         panose[10];
    unsigned long   ulUnicodeRange1;
    unsigned long   ulUnicodeRange2;
    unsigned long   ulUnicodeRange3;
    unsigned long   ulUnicodeRange4;
    signed char     achVendID[4];
    uint16_t        fsSelection;
    uint16_t        usFirstCharIndex;
    uint16_t        usLastCharIndex;
    int16_t         sTypoAscender;
    int16_t         sTypoDescender;
    int16_t         sTypoLineGap;
    uint16_t        usWinAscent;
    uint16_t        usWinDescent;
    /* only version 1 and higher: */
    unsigned long   ulCodePageRange1;
    unsigned long   ulCodePageRange2;
    /* only version 2 and higher: */
    int16_t         sxHeight;
    int16_t         sCapHeight;
    uint16_t        usDefaultChar;
    uint16_t        usBreakChar;
    uint16_t        usMaxContext;
    /* only version 5 and higher: */
    uint16_t        usLowerOpticalPointSize;
    uint16_t        usUpperOpticalPointSize;
} TT_OS2_t;
typedef struct  TT_HoriHeader_s
{
    signed long Version;
    int16_t     Ascender;
    int16_t     Descender;
    int16_t     Line_Gap;
    uint16_t    advance_Width_Max;
    int16_t     min_Left_Side_Bearing;
    int16_t     min_Right_Side_Bearing;
    int16_t     xMax_Extent;
    int16_t     caret_Slope_Rise;
    int16_t     caret_Slope_Run;
    int16_t     caret_Offset;
    int16_t     Reserved[4];
    int16_t     metric_Data_Format;
    uint16_t    number_Of_HMetrics;
    void*       long_metrics;
    void*       short_metrics;
} TT_HoriHeader_t;
typedef struct  TT_VertHeader_s
{
    signed long Version;
    int16_t     Ascender;
    int16_t     Descender;
    int16_t     Line_Gap;
    uint16_t    advance_Height_Max;
    int16_t     min_Top_Side_Bearing;
    int16_t     min_Bottom_Side_Bearing;
    int16_t     yMax_Extent;
    int16_t     caret_Slope_Rise;
    int16_t     caret_Slope_Run;
    int16_t     caret_Offset;
    int16_t     Reserved[4];
    int16_t     metric_Data_Format;
    uint16_t    number_Of_VMetrics;
    void*       long_metrics;
    void*       short_metrics;
} TT_VertHeader_t;
typedef struct  TT_Postscript_s
{
    signed long     FormatType;
    signed long     italicAngle;
    int16_t         underlinePosition;
    int16_t         underlineThickness;
    unsigned long   isFixedPitch;
    unsigned long   minMemType42;
    unsigned long   maxMemType42;
    unsigned long   minMemType1;
    unsigned long   maxMemType1;
} TT_Postscript_t;
typedef struct  TT_PCLT_s
{
    signed long     Version;
    unsigned long   FontNumber;
    uint16_t        Pitch;
    uint16_t        xHeight;
    uint16_t        Style;
    uint16_t        TypeFamily;
    uint16_t        CapHeight;
    uint16_t        SymbolSet;
    signed char     TypeFace[16];
    signed char     CharacterComplement[8];
    signed char     FileName[6];
    signed char     StrokeWeight;
    signed char     WidthType;
    uint8_t         SerifStyle;
    uint8_t         Reserved;
} TT_PCLT_t;

typedef struct  FT_Var_Axis_s
{
    void*           name;    //string
    long            minimum;
    long            def;
    long            maximum;
    unsigned long   tag;
    uint32_t        strid;
} FT_Var_Axis_t;

typedef struct  FT_Var_Named_Style_s
{
    long*      coords;  // one entry per axis
    uint32_t   strid;
    uint32_t   psid;
} FT_Var_Named_Style_t;

typedef struct  FT_MM_Var_s
{
    uint32_t                num_axis;
    uint32_t                num_designs;
    uint32_t                num_namedstyles;
    FT_Var_Axis_t*          axis;
    FT_Var_Named_Style_t*   namedstyle;
} FT_MM_Var_t;
// ===============================================
// 32bits FreeType structures
// ===============================================

typedef union  __attribute__((packed, aligned(4))) FT_StreamDesc_32_s
{
    long_t value;
    ptr_t  pointer; //void*
} FT_StreamDesc_32_t;

typedef struct __attribute__((packed, aligned(4))) FT_StreamRec_32_s
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

typedef struct __attribute__((packed, aligned(4))) FT_BBox_32_s
{
    long_t          xMin, yMin;
    long_t          xMax, yMax;
} FT_BBox_32_t;
typedef struct __attribute__((packed, aligned(4))) FT_Generic_32_s
{
    ptr_t           data; //void*
    ptr_t           finalizer; //vFp_t
} FT_Generic_32_t;
typedef struct __attribute__((packed, aligned(4))) FT_ListRec_32_s
{
  ptr_t             head; //void*
  ptr_t             tail; //void*
} FT_ListRec_32_t;

typedef struct __attribute__((packed, aligned(4))) FT_Vector_32_s
{
    long_t  x;
    long_t  y;
} FT_Vector_32_t;

typedef struct __attribute__((packed, aligned(4))) FT_Bitmap_Size_32_s
{
    short  height;
    short  width;
    long_t size;
    long_t x_ppem;
    long_t y_ppem;
} FT_Bitmap_Size_32_t;

typedef struct __attribute__((packed, aligned(4))) FT_Glyph_Metrics_32_s
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

typedef struct __attribute__((packed, aligned(4))) FT_Outline_32_s
{
    unsigned short   n_contours;
    unsigned short   n_points;
    ptr_t            points;   //FT_Vector_32_t*
    ptr_t            tags;  //unsigned char*
    ptr_t            contours;  //unsigned short*
    int              flags;
} FT_Outline_32_t;

typedef struct __attribute__((packed, aligned(4))) FT_Bitmap_32_s
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

typedef struct __attribute__((packed, aligned(4))) FT_GlyphSlotRec_32_s
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

typedef struct __attribute__((packed, aligned(4))) FT_CharMapRec_32_s
{
    ptr_t       face;   //FT_FaceRec_t*
    int         encoding;
    uint16_t    platform_id;
    uint16_t    encoding_id;
} FT_CharMapRec_32_t;

typedef struct __attribute__((packed, aligned(4))) FT_Size_Metrics_32_s
{
    uint16_t    x_ppem;
    uint16_t    y_ppem;
    long_t      x_scale;
    long_t      y_scale;
    long_t      ascender;
    long_t      descender;
    long_t      height;
    long_t      max_advance;
} FT_Size_Metrics_32_t;

typedef struct __attribute__((packed, aligned(4))) FT_SizeRec_32_s
{
    ptr_t                   face;   //FT_FaceRec_t*
    FT_Generic_32_t         generic;
    FT_Size_Metrics_32_t    metrics;
    ptr_t                   internal;   //FT_Size_Internal
} FT_SizeRec_32_t;

typedef struct __attribute__((packed, aligned(4))) FT_FaceRec_32_s
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
    ptr_t           charmaps; //FT_CharMapRec_32_t**
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
    ptr_t           size; //FT_SizeRec_32_t*
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

typedef struct __attribute__((packed, aligned(4))) FT_MemoryRec_32_s
{
    ptr_t           user;   //void*
    ptr_t           alloc;  //void*
    ptr_t           free;   //void*
    ptr_t           realloc;//void*
} FT_MemoryRec_32_t;

typedef struct __attribute__((packed, aligned(4))) PS_PrivateRec_32_s
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

typedef struct __attribute__((packed, aligned(4))) BDF_PropertyRec_32_s
{
    int  type;
    union {
        ptr_t     atom; //const char*
        int       integer;
        uint32_t  cardinal;

    } u;
} BDF_PropertyRec_32_t;

typedef struct __attribute__((packed, aligned(4))) FT_Size_RequestRec_32_s
{
    int         type;
    long_t      width;
    long_t      height;
    uint32_t    horiResolution;
    uint32_t    vertResolution;
} FT_Size_RequestRec_32_t;

typedef struct __attribute__((packed, aligned(4))) FT_Parameter_32_s
{
    ulong_t   tag;
    ptr_t     data; //void*
} FT_Parameter_32_t;

typedef struct __attribute__((packed, aligned(4))) FT_Open_Args_32_s
{
    uint32_t        flags;
    ptr_t           memory_base; //uint8_t*
    long_t          memory_size;
    ptr_t           pathname;   //char*
    ptr_t           stream; //FT_StreamRec_t*
    ptr_t           driver; //FT_ModuleRec
    int             num_params;
    ptr_t           params; //FT_Parameter_t*
} FT_Open_Args_32_t;

typedef struct FT_WinFNT_HeaderRec_32_s // removed the packed attribute
{
    uint16_t        version;
    ulong_t         file_size;
    uint8_t         copyright[60];
    uint16_t        file_type;
    uint16_t        nominal_point_size;
    uint16_t        vertical_resolution;
    uint16_t        horizontal_resolution;
    uint16_t        ascent;
    uint16_t        internal_leading;
    uint16_t        external_leading;
    uint8_t         italic;
    uint8_t         underline;
    uint8_t         strike_out;
    uint16_t        weight;
    uint8_t         charset;
    uint16_t        pixel_width;
    uint16_t        pixel_height;
    uint8_t         pitch_and_family;
    uint16_t        avg_width;
    uint16_t        max_width;
    uint8_t         first_char;
    uint8_t         last_char;
    uint8_t         default_char;
    uint8_t         break_char;
    uint16_t        bytes_per_row;
    ulong_t         device_offset;
    ulong_t         face_name_offset;
    ulong_t         bits_pointer;
    ulong_t         bits_offset;
    uint8_t         reserved;
    ulong_t         flags;
    uint16_t        A_space;
    uint16_t        B_space;
    uint16_t        C_space;
    uint16_t        color_table_offset;
    ulong_t         reserved1[4];
} FT_WinFNT_HeaderRec_32_t;

typedef struct __attribute__((packed, aligned(4))) FT_Matrix_32_s
{
    long_t  xx, xy;
    long_t  yx, yy;
} FT_Matrix_32_t;

typedef struct  __attribute__((packed, aligned(4))) FT_GlyphRec_32_s
{
    ptr_t           library;  //FT_Library
    ptr_t           clazz;  //const FT_Glyph_Class*
    uint32_t        format;
    FT_Vector_32_t  advance;
} FT_GlyphRec_32_t;

typedef struct  FT_BitmapGlyphRec_32_s
{
    FT_GlyphRec_32_t    root;
    int                 left;
    int                 top;
    FT_Bitmap_32_t      bitmap;
} FT_BitmapGlyphRec_32_t;
typedef struct  TT_Header_32_s  //no align
{
    long_t          Table_Version;
    long_t          Font_Revision;
    long_t          CheckSum_Adjust;
    long_t          Magic_Number;
    uint16_t        Flags;
    uint16_t        Units_Per_EM;
    ulong_t         Created [2];
    ulong_t         Modified[2];
    int16_t         xMin;
    int16_t         yMin;
    int16_t         xMax;
    int16_t         yMax;
    uint16_t        Mac_Style;
    uint16_t        Lowest_Rec_PPEM;
    int16_t         Font_Direction;
    int16_t         Index_To_Loc_Format;
    int16_t         Glyph_Data_Format;
} TT_Header_32_t;
typedef struct  TT_MaxProfile_32_s  //no align
{
    long_t      version;
    uint16_t    numGlyphs;
    uint16_t    maxPoints;
    uint16_t    maxContours;
    uint16_t    maxCompositePoints;
    uint16_t    maxCompositeContours;
    uint16_t    maxZones;
    uint16_t    maxTwilightPoints;
    uint16_t    maxStorage;
    uint16_t    maxFunctionDefs;
    uint16_t    maxInstructionDefs;
    uint16_t    maxStackElements;
    uint16_t    maxSizeOfInstructions;
    uint16_t    maxComponentElements;
    uint16_t    maxComponentDepth;
} TT_MaxProfile_32_t;
typedef struct TT_OS2_32_s  //no align
{
    uint16_t        version;
    int16_t         xAvgCharWidth;
    uint16_t        usWeightClass;
    uint16_t        usWidthClass;
    uint16_t        fsType;
    int16_t         ySubscriptXSize;
    int16_t         ySubscriptYSize;
    int16_t         ySubscriptXOffset;
    int16_t         ySubscriptYOffset;
    int16_t         ySuperscriptXSize;
    int16_t         ySuperscriptYSize;
    int16_t         ySuperscriptXOffset;
    int16_t         ySuperscriptYOffset;
    int16_t         yStrikeoutSize;
    int16_t         yStrikeoutPosition;
    int16_t         sFamilyClass;
    uint8_t         panose[10];
    ulong_t         ulUnicodeRange1;
    ulong_t         ulUnicodeRange2;
    ulong_t         ulUnicodeRange3;
    ulong_t         ulUnicodeRange4;
    signed char     achVendID[4];
    uint16_t        fsSelection;
    uint16_t        usFirstCharIndex;
    uint16_t        usLastCharIndex;
    int16_t         sTypoAscender;
    int16_t         sTypoDescender;
    int16_t         sTypoLineGap;
    uint16_t        usWinAscent;
    uint16_t        usWinDescent;
    /* only version 1 and higher: */
    ulong_t         ulCodePageRange1;
    ulong_t         ulCodePageRange2;
    /* only version 2 and higher: */
    int16_t         sxHeight;
    int16_t         sCapHeight;
    uint16_t        usDefaultChar;
    uint16_t        usBreakChar;
    uint16_t        usMaxContext;
    /* only version 5 and higher: */
    uint16_t        usLowerOpticalPointSize;
    uint16_t        usUpperOpticalPointSize;
} TT_OS2_32_t;
typedef struct TT_HoriHeader_32_s   //no align
{
    long_t      Version;
    int16_t     Ascender;
    int16_t     Descender;
    int16_t     Line_Gap;
    uint16_t    advance_Width_Max;
    int16_t     min_Left_Side_Bearing;
    int16_t     min_Right_Side_Bearing;
    int16_t     xMax_Extent;
    int16_t     caret_Slope_Rise;
    int16_t     caret_Slope_Run;
    int16_t     caret_Offset;
    int16_t     Reserved[4];
    int16_t     metric_Data_Format;
    uint16_t    number_Of_HMetrics;
    ptr_t       long_metrics;
    ptr_t       short_metrics;
} TT_HoriHeader_32_t;
typedef struct  TT_VertHeader_32_s  //no align
{
    long_t      Version;
    int16_t     Ascender;
    int16_t     Descender;
    int16_t     Line_Gap;
    uint16_t    advance_Height_Max;
    int16_t     min_Top_Side_Bearing;
    int16_t     min_Bottom_Side_Bearing;
    int16_t     yMax_Extent;
    int16_t     caret_Slope_Rise;
    int16_t     caret_Slope_Run;
    int16_t     caret_Offset;
    int16_t     Reserved[4];
    int16_t     metric_Data_Format;
    uint16_t    number_Of_VMetrics;
    ptr_t       long_metrics;
    ptr_t       short_metrics;
} TT_VertHeader_32_t;
typedef struct  TT_Postscript_32_s  //no align
{
    long_t          FormatType;
    long_t          italicAngle;
    int16_t         underlinePosition;
    int16_t         underlineThickness;
    ulong_t         isFixedPitch;
    ulong_t         minMemType42;
    ulong_t         maxMemType42;
    ulong_t         minMemType1;
    ulong_t         maxMemType1;
} TT_Postscript_32_t;
typedef struct  TT_PCLT_32_s    //no align
{
    long_t          Version;
    ulong_t         FontNumber;
    uint16_t        Pitch;
    uint16_t        xHeight;
    uint16_t        Style;
    uint16_t        TypeFamily;
    uint16_t        CapHeight;
    uint16_t        SymbolSet;
    signed char     TypeFace[16];
    signed char     CharacterComplement[8];
    signed char     FileName[6];
    signed char     StrokeWeight;
    signed char     WidthType;
    uint8_t         SerifStyle;
    uint8_t         Reserved;
} TT_PCLT_32_t;

typedef struct  FT_Var_Axis_32_s
{
    ptr_t      name;    //string
    long_t     minimum;
    long_t     def;
    long_t     maximum;
    ulong_t    tag;
    uint32_t   strid;
} FT_Var_Axis_32_t;

typedef struct  FT_Var_Named_Style_32_s
{
    ptr_t      coords;  // long_t*
    uint32_t   strid;
    uint32_t   psid;
} FT_Var_Named_Style_32_t;

typedef struct  FT_MM_Var_32_s
{
    uint32_t    num_axis;
    uint32_t    num_designs;
    uint32_t    num_namedstyles;
    ptr_t       axis;    //FT_Var_Axis_32_t*
    ptr_t       namedstyle; //FT_Var_Named_Style_32_t*
} FT_MM_Var_32_t;

// ==================================
// Convertions
// ==================================

void convert_FT_StreamRec_to_32(void* d, void* s)
{
    FT_StreamRec_t* src = s;
    FT_StreamRec_32_t* dst = d;

    dst-> base = to_ptrv(src->base);
    dst-> size = to_ulong(src->size);
    dst-> pos = to_ulong(src->pos);
    dst-> descriptor.value = to_long(src->descriptor.value);
    dst-> descriptor.pointer = to_ptrv(src->descriptor.pointer);
    dst-> pathname.value = to_long(src->pathname.value);
    dst-> pathname.pointer = to_ptrv(src->pathname.pointer);
    dst-> read = to_ptrv(src->read);
    dst-> close = to_ptrv(src->close);
    dst-> memory = to_ptrv(src->memory);
    dst-> cursor = to_ptrv(src->cursor);
    dst-> limit = to_ptrv(src->limit);
}
void convert_FT_StreamRec_to_64(void* d, void* s)
{
    FT_StreamRec_32_t* src = s;
    FT_StreamRec_t* dst = d;

    dst-> limit = from_ptrv(src->limit);
    dst-> cursor = from_ptrv(src->cursor);
    dst-> memory = from_ptrv(src->memory);
    dst-> close = from_ptrv(src->close);
    dst-> read = from_ptrv(src->read);
    dst-> pathname.pointer = from_ptrv(src->pathname.pointer);
    dst-> pathname.value = from_long(src->pathname.value);
    dst-> descriptor.pointer = from_ptrv(src->descriptor.pointer);
    dst-> descriptor.value = from_long(src->descriptor.value);
    dst-> pos = from_ulong(src->pos);
    dst-> size = from_ulong(src->size);
    dst-> base = from_ptrv(src->base);
}

void convert_FT_GlyphSlot_to_32(void* d, void* s)
{
    FT_GlyphSlotRec_t* src = s;
    FT_GlyphSlotRec_32_t* dst = d;

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
}

void convert_FT_GlyphSlot_to_64(void* d, void* s)
{
    FT_GlyphSlotRec_32_t* src = s;
    FT_GlyphSlotRec_t* dst = d;

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
}

void inplace_FT_GlyphSlot_shrink(void* a)
{
    if(!a) return;
    FT_GlyphSlotRec_t* src = a;
    FT_GlyphSlotRec_32_t* dst = a;
    void* next = src->next;

    convert_FT_GlyphSlot_to_32(dst, src);

    inplace_FT_GlyphSlot_shrink(next);
}

void inplace_FT_GlyphSlot_enlarge(void* a)
{
    if(!a) return;
    FT_GlyphSlotRec_32_t* src = a;
    FT_GlyphSlotRec_t* dst = a;
    void* next = from_ptrv(src->next);

    convert_FT_GlyphSlot_to_64(dst, src);

    inplace_FT_GlyphSlot_enlarge(next);
}

void convert_FT_CharMapRec_to_32(void* d, void* s)
{
    FT_CharMapRec_t* src = s;
    FT_CharMapRec_32_t* dst = d;

    dst->face = to_ptrv(src->face);
    dst->encoding = src->encoding;
    dst->platform_id = src->platform_id;
    dst->encoding_id = src->encoding_id;
}
void convert_FT_CharMapRec_to_64(void* d, void* s)
{
    FT_CharMapRec_32_t* src = s;
    FT_CharMapRec_t* dst = d;

    dst->encoding_id = src->encoding_id;
    dst->platform_id = src->platform_id;
    dst->encoding = src->encoding;
    dst->face = from_ptrv(src->face);
}

void inplace_FT_CharMapRec_shrink(void* a)
{
    if(!a) return;

    convert_FT_CharMapRec_to_32(a, a);
}
void inplace_FT_CharMapRec_enlarge(void* a)
{
    if(!a) return;

    convert_FT_CharMapRec_to_64(a, a);
}

void convert_FT_SizeRec_to_32(void* d, void* s)
{
    FT_SizeRec_t* src = s;
    FT_SizeRec_32_t* dst = d;

    dst->face = to_ptrv(src->face);
    dst->generic.data = to_ptrv(src->generic.data);
    dst->generic.finalizer = to_ptrv(src->generic.finalizer);
    dst->metrics.x_ppem = src->metrics.x_ppem;
    dst->metrics.y_ppem = src->metrics.y_ppem;
    dst->metrics.x_scale = to_long(src->metrics.x_scale);
    dst->metrics.y_scale = to_long(src->metrics.y_scale);
    dst->metrics.ascender = to_long(src->metrics.ascender);
    dst->metrics.descender = to_long(src->metrics.descender);
    dst->metrics.height = to_long(src->metrics.height);
    dst->metrics.max_advance = to_long(src->metrics.max_advance);
    dst->internal = to_ptrv(src->internal);
}
void convert_FT_SizeRec_to_64(void* d, void* s)
{
    FT_SizeRec_32_t* src = s;
    FT_SizeRec_t* dst = d;

    dst->internal = from_ptrv(src->internal);
    dst->metrics.max_advance = from_long(src->metrics.max_advance);
    dst->metrics.height = from_long(src->metrics.height);
    dst->metrics.descender = from_long(src->metrics.descender);
    dst->metrics.ascender = from_long(src->metrics.ascender);
    dst->metrics.y_scale = from_long(src->metrics.y_scale);
    dst->metrics.x_scale = from_long(src->metrics.x_scale);
    dst->metrics.y_ppem = src->metrics.y_ppem;
    dst->metrics.x_ppem = src->metrics.x_ppem;
    dst->generic.finalizer = from_ptrv(src->generic.finalizer);
    dst->generic.data = from_ptrv(src->generic.data);
    dst->face = from_ptrv(src->face);
}

void inplace_FT_SizeRec_shrink(void* a)
{
    if(!a) return;

    convert_FT_SizeRec_to_32(a, a);
}
void inplace_FT_SizeRec_enlarge(void* a)
{
    if(!a) return;

    convert_FT_SizeRec_to_64(a, a);
}


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
    //convert charmaps content then pointers array
    for(int i=0; i<src->num_charmaps; ++i)
        inplace_FT_CharMapRec_shrink(src->charmaps[i]);
    for(int i=0; i<src->num_charmaps; ++i)
        ((ptr_t*)src->charmaps)[i] = to_ptrv(src->charmaps[i]);
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
    inplace_FT_SizeRec_shrink(src->size);
    dst->glyph = to_ptrv(src->glyph);
    dst->size = to_ptrv(src->size);
    dst->charmap = to_ptrv(src->charmap);
    dst->driver = to_ptrv(src->driver);
    dst->memory = to_ptrv(src->memory);
    //((FT_StreamDesc_32_t*)src->stream)->value = to_long(src->stream->value);
    //((FT_StreamDesc_32_t*)src->stream)->pointer = to_ptrv(src->stream->pointer);
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
    //dst->stream->pointer = from_ptrv(((FT_StreamDesc_32_t*)dst->stream)->pointer);
    //dst->stream->value = from_long(((FT_StreamDesc_32_t*)dst->stream)->value);
    dst->memory = from_ptrv(src->memory);
    dst->driver = from_ptrv(src->driver);
    dst->charmap = from_ptrv(src->charmap);
    dst->size = from_ptrv(src->size);
    inplace_FT_SizeRec_enlarge(dst->size);
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
    //convert charmaps pointer array then content
    for(int i=dst->num_charmaps-1; i>=0; --i)
        dst->charmaps[i] = from_ptrv(((ptr_t*)dst->charmaps)[i]);
    for(int i=dst->num_charmaps-1; i>=0; --i)
        inplace_FT_CharMapRec_enlarge(dst->charmaps[i]);
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

void convert_FT_WinFNT_HeaderRec_to_32(void* d, void* s)
{
    FT_WinFNT_HeaderRec_t* src = s;
    FT_WinFNT_HeaderRec_32_t* dst = d;

    dst->version = src->version;
    dst->file_size = to_ulong(src->file_size);
    memcpy(dst->copyright, src->copyright, sizeof(dst->copyright));
    dst->file_type = src->file_type;
    dst->nominal_point_size = src->nominal_point_size;
    dst->vertical_resolution = src->vertical_resolution;
    dst->horizontal_resolution = src->horizontal_resolution;
    dst->ascent = src->ascent;
    dst->internal_leading = src->internal_leading;
    dst->external_leading = src->external_leading;
    dst->italic = src->italic;
    dst->underline = src->underline;
    dst->strike_out = src->strike_out;
    dst->weight = src->weight;
    dst->charset = src->charset;
    dst->pixel_width = src->pixel_width;
    dst->pixel_height = src->pixel_height;
    dst->pitch_and_family = src->pitch_and_family;
    dst->avg_width = src->avg_width;
    dst->max_width = src->max_width;
    dst->first_char = src->first_char;
    dst->last_char = src->last_char;
    dst->default_char = src->default_char;
    dst->break_char = src->break_char;
    dst->bytes_per_row = src->bytes_per_row;
    dst->device_offset = to_ulong(src->device_offset);
    dst->face_name_offset = to_ulong(src->face_name_offset);
    dst->bits_pointer = to_ulong(src->bits_pointer);
    dst->bits_offset = to_ulong(src->bits_offset);
    dst->reserved = src->reserved;
    dst->flags = to_ulong(src->flags);
    dst->A_space = src->A_space;
    dst->B_space = src->B_space;
    dst->C_space = src->C_space;
    dst->color_table_offset = src->color_table_offset;
    dst->reserved1[0] = to_ulong(src->reserved1[0]);
    dst->reserved1[1] = to_ulong(src->reserved1[1]);
    dst->reserved1[2] = to_ulong(src->reserved1[2]);
    dst->reserved1[3] = to_ulong(src->reserved1[3]);
}

void convert_FT_Matrix_to_32(void* d, void* s)
{
    FT_Matrix_t* src = s;
    FT_Matrix_32_t* dst = d;

    dst->xx = to_long(src->xx);
    dst->xy = to_long(src->xy);
    dst->yx = to_long(src->yx);
    dst->yy = to_long(src->yy);
}
void convert_FT_Matrix_to_64(void* d, void* s)
{
    FT_Matrix_32_t* src = s;
    FT_Matrix_t* dst = d;

    dst->yy = from_long(src->yy);
    dst->yx = from_long(src->yx);
    dst->xy = from_long(src->xy);
    dst->xx = from_long(src->xx);
}

void convert_FT_Bitmap_to_32(void* d, void* s)
{
    FT_Bitmap_t* src = s;
    FT_Bitmap_32_t* dst = d;

    dst->rows = src->rows;
    dst->width = src->width;
    dst->pitch = src->pitch;
    dst->buffer = to_ptrv(src->buffer);
    dst->num_grays = src->num_grays;
    dst->pixel_mode = src->pixel_mode;
    dst->palette_mode = src->palette_mode;
    dst->palette = to_ptrv(src->palette);
}
void convert_FT_Bitmap_to_64(void* d, void* s)
{
    FT_Bitmap_32_t* src = s;
    FT_Bitmap_t* dst = d;

    dst->palette = from_ptrv(src->palette);
    dst->palette_mode = src->palette_mode;
    dst->pixel_mode = src->pixel_mode;
    dst->num_grays = src->num_grays;
    dst->buffer = from_ptrv(src->buffer);
    dst->pitch = src->pitch;
    dst->width = src->width;
    dst->rows = src->rows;
}

void convert_FT_Outline_to_32(void* d, void* s)
{
    if(!s || !d) return;
    FT_Outline_t* src = s;
    FT_Outline_32_t* dst = d;

    dst->n_contours = src->n_contours;
    dst->n_points = src->n_points;
    dst->points = to_ptrv(src->points);
    dst->tags = to_ptrv(src->tags);
    dst->contours = to_ptrv(src->contours);
    dst->flags = src->flags;
    int n = dst->n_points;
    FT_Vector_32_t* vec = from_ptrv(dst->points);
    for(int i=0; i<n; ++i) {
        vec[i].x = to_long(src->points[i].x);
        vec[i].y = to_long(src->points[i].y);
    }
}

void convert_FT_Outline_to_64(void* d, void* s)
{
    if(!s || !d) return;
    FT_Outline_32_t* src = s;
    FT_Outline_t* dst = d;

    dst->flags = src->flags;
    dst->contours = from_ptrv(src->contours);
    dst->tags = from_ptrv(src->tags);
    dst->points = from_ptrv(src->points);
    dst->n_points = src->n_points;
    dst->n_contours = src->n_contours;
    int n = dst->n_points;
    FT_Vector_32_t* vec = (FT_Vector_32_t*)dst->points;
    for(int i=n-1; i>=0; --i) {
        dst->points[i].x = from_long(vec[i].x);
        dst->points[i].y = from_long(vec[i].y);
    }
}

void inplace_FT_Glyph_shrink(void* a)
{
    if(!a)
        return;
    
    FT_GlyphRec_t* src = a;
    FT_GlyphRec_32_t* dst = a;

    dst->library = to_ptrv(src->library);
    dst->clazz = to_ptrv(src->clazz);
    dst->format = src->format;
    dst->advance.x = to_long(src->advance.x);
    dst->advance.y = to_long(src->advance.y);

    if(dst->format == FT_GLYPH_FORMAT_OUTLINE) {
        FT_Outline_t* src_o = (FT_Outline_t*)(src+1);
        FT_Outline_32_t* dst_o = (FT_Outline_32_t*)(dst+1);
        
        convert_FT_Outline_to_32(dst_o, src_o);
    } else if(dst->format == FT_GLYPH_FORMAT_BITMAP) {
        FT_BitmapGlyphRec_t* src_b = a;
        FT_BitmapGlyphRec_32_t* dst_b = a;

        dst_b->left = src_b->left;
        dst_b->top = src_b->top;
        convert_FT_Bitmap_to_32(&dst_b->bitmap, &src_b->bitmap);
    } else {
        printf_log(LOG_NONE, "BOX32: Warning, unsupported glyph format 0x%x (%c%c%c%c)\n", dst->format, dst->format>>24, (dst->format>>16)&0xff, (dst->format>>8)&0xff, dst->format&0xff);
    }
}

void inplace_FT_Glyph_enlarge(void* a)
{
    if(!a)
        return;
    
    FT_GlyphRec_32_t* src = a;
    FT_GlyphRec_t* dst = a;

    dst->advance.y = from_long(src->advance.y);
    dst->advance.x = from_long(src->advance.x);
    dst->format = src->format;
    dst->clazz = from_ptrv(src->clazz);
    dst->library = from_ptrv(src->library);

    if(dst->format == FT_GLYPH_FORMAT_OUTLINE) {
        FT_Outline_t* dst_o = (FT_Outline_t*)(dst+1);
        FT_Outline_32_t* src_o = (FT_Outline_32_t*)(src+1);

        convert_FT_Outline_to_64(dst_o, src_o);
    } else if(dst->format == FT_GLYPH_FORMAT_BITMAP) {
        FT_BitmapGlyphRec_32_t* src_b = a;
        FT_BitmapGlyphRec_t* dst_b = a;

        convert_FT_Bitmap_to_64(&dst_b->bitmap, &src_b->bitmap);
        dst_b->top = src_b->top;
        dst_b->left = src_b->left;
    }
}

void convert_TT_Header_to_32(void* d, void* s)
{
    if(!s || !d) return;
    TT_Header_t* src = s;
    TT_Header_32_t* dst = d;

    dst->Table_Version = to_long(src->Table_Version);
    dst->Font_Revision = to_long(src->Font_Revision);
    dst->CheckSum_Adjust = to_long(src->CheckSum_Adjust);
    dst->Magic_Number = to_long(src->Magic_Number);
    dst->Flags = src->Flags;
    dst->Units_Per_EM = src->Units_Per_EM;
    dst->Created [0] = to_ulong(src->Created[0]);
    dst->Created [1] = to_ulong(src->Created[1]);
    dst->Modified[0] = to_ulong(src->Modified[0]);
    dst->Modified[1] = to_ulong(src->Modified[1]);
    dst->xMin = src->xMin;
    dst->yMin = src->yMin;
    dst->xMax = src->xMax;
    dst->yMax = src->yMax;
    dst->Mac_Style = src->Mac_Style;
    dst->Lowest_Rec_PPEM = src->Lowest_Rec_PPEM;
    dst->Font_Direction = src->Font_Direction;
    dst->Index_To_Loc_Format = src->Index_To_Loc_Format;
    dst->Glyph_Data_Format = src->Glyph_Data_Format;
}
void convert_TT_MaxProfile_to_32(void* d, void* s)
{
    if(!s || !d) return;
    TT_MaxProfile_t* src = s;
    TT_MaxProfile_32_t* dst = d;

    dst->version = to_long(src->version);
    dst->numGlyphs = src->numGlyphs;
    dst->maxPoints = src->maxPoints;
    dst->maxContours = src->maxContours;
    dst->maxCompositePoints = src->maxCompositePoints;
    dst->maxCompositeContours = src->maxCompositeContours;
    dst->maxZones = src->maxZones;
    dst->maxTwilightPoints = src->maxTwilightPoints;
    dst->maxStorage = src->maxStorage;
    dst->maxFunctionDefs = src->maxFunctionDefs;
    dst->maxInstructionDefs = src->maxInstructionDefs;
    dst->maxStackElements = src->maxStackElements;
    dst->maxSizeOfInstructions = src->maxSizeOfInstructions;
    dst->maxComponentElements = src->maxComponentElements;
    dst->maxComponentDepth = src->maxComponentDepth;
}
void convert_TT_OS2_to_32(void* d, void* s)
{
    if(!s || !d) return;
    TT_OS2_t* src = s;
    TT_OS2_32_t* dst = d;

    dst->version = src->version;
    dst->xAvgCharWidth = src->xAvgCharWidth;
    dst->usWeightClass = src->usWeightClass;
    dst->usWidthClass = src->usWidthClass;
    dst->fsType = src->fsType;
    dst->ySubscriptXSize = src->ySubscriptXSize;
    dst->ySubscriptYSize = src->ySubscriptYSize;
    dst->ySubscriptXOffset = src->ySubscriptXOffset;
    dst->ySubscriptYOffset = src->ySubscriptYOffset;
    dst->ySuperscriptXSize = src->ySuperscriptXSize;
    dst->ySuperscriptYSize = src->ySuperscriptYSize;
    dst->ySuperscriptXOffset = src->ySuperscriptXOffset;
    dst->ySuperscriptYOffset = src->ySuperscriptYOffset;
    dst->yStrikeoutSize = src->yStrikeoutSize;
    dst->yStrikeoutPosition = src->yStrikeoutPosition;
    dst->sFamilyClass = src->sFamilyClass;
    for(int i=0; i<10; ++i)
        dst->panose[i] = src->panose[i];
    dst->ulUnicodeRange1 = src->ulUnicodeRange1;
    dst->ulUnicodeRange2 = src->ulUnicodeRange2;
    dst->ulUnicodeRange3 = src->ulUnicodeRange3;
    dst->ulUnicodeRange4 = src->ulUnicodeRange4;
    for(int i=0; i<4; ++i)
        dst->achVendID[i] = src->achVendID[i];
    dst->fsSelection = src->fsSelection;
    dst->usFirstCharIndex = src->usFirstCharIndex;
    dst->usLastCharIndex = src->usLastCharIndex;
    dst->sTypoAscender = src->sTypoAscender;
    dst->sTypoDescender = src->sTypoDescender;
    dst->sTypoLineGap = src->sTypoLineGap;
    dst->usWinAscent = src->usWinAscent;
    dst->usWinDescent = src->usWinDescent;
    if(dst->version<1) return;
    dst->ulCodePageRange1 = to_ulong(src->ulCodePageRange1);
    dst->ulCodePageRange2 = to_ulong(src->ulCodePageRange2);
    if(dst->version<2) return;
    dst->sxHeight = src->sxHeight;
    dst->sCapHeight = src->sCapHeight;
    dst->usDefaultChar = src->usDefaultChar;
    dst->usBreakChar = src->usBreakChar;
    dst->usMaxContext = src->usMaxContext;
    if(dst->version<5) return;
    dst->usLowerOpticalPointSize = src->usLowerOpticalPointSize;
    dst->usUpperOpticalPointSize = src->usUpperOpticalPointSize;
}
void convert_TT_HoriHeader_to_32(void* d, void* s)
{
    if(!s || !d) return;
    TT_HoriHeader_t* src = s;
    TT_HoriHeader_32_t* dst = d;

    dst->Version = to_long(src->Version);
    dst->Ascender = src->Ascender;
    dst->Descender = src->Descender;
    dst->Line_Gap = src->Line_Gap;
    dst->advance_Width_Max = src->advance_Width_Max;
    dst->min_Left_Side_Bearing = src->min_Left_Side_Bearing;
    dst->min_Right_Side_Bearing = src->min_Right_Side_Bearing;
    dst->xMax_Extent = src->xMax_Extent;
    dst->caret_Slope_Rise = src->caret_Slope_Rise;
    dst->caret_Slope_Run = src->caret_Slope_Run;
    dst->caret_Offset = src->caret_Offset;
    for(int i=0; i<4; ++i)
        dst->Reserved[i] = src->Reserved[i];
    dst->metric_Data_Format = src->metric_Data_Format;
    dst->number_Of_HMetrics = src->number_Of_HMetrics;
    dst->long_metrics = to_ptrv(src->long_metrics);
    dst->short_metrics = to_ptrv(src->short_metrics);
}
void convert_TT_VertHeader_to_32(void* d, void* s)
{
    if(!s || !d) return;
    TT_VertHeader_t* src = s;
    TT_VertHeader_32_t* dst = d;

    dst->Version = to_long(src->Version);
    dst->Ascender = src->Ascender;
    dst->Descender = src->Descender;
    dst->Line_Gap = src->Line_Gap;
    dst->advance_Height_Max = src->advance_Height_Max;
    dst->min_Top_Side_Bearing = src->min_Top_Side_Bearing;
    dst->min_Bottom_Side_Bearing = src->min_Bottom_Side_Bearing;
    dst->yMax_Extent = src->yMax_Extent;
    dst->caret_Slope_Rise = src->caret_Slope_Rise;
    dst->caret_Slope_Run = src->caret_Slope_Run;
    dst->caret_Offset = src->caret_Offset;
    for(int i=0; i<4; ++i)
        dst->Reserved[i] = src->Reserved[i];
    dst->metric_Data_Format = src->metric_Data_Format;
    dst->number_Of_VMetrics = src->number_Of_VMetrics;
    dst->long_metrics = to_ptrv(src->long_metrics);
    dst->short_metrics = to_ptrv(src->short_metrics);
}
void convert_TT_Postscript_to_32(void* d, void* s)
{
    if(!s || !d) return;
    TT_Postscript_t* src = s;
    TT_Postscript_32_t* dst = d;

    dst->FormatType = to_long(src->FormatType);
    dst->italicAngle = to_long(src->italicAngle);
    dst->underlinePosition = src->underlinePosition;
    dst->underlineThickness = src->underlineThickness;
    dst->isFixedPitch = to_ulong(src->isFixedPitch);
    dst->minMemType42 = to_ulong(src->minMemType42);
    dst->maxMemType42 = to_ulong(src->maxMemType42);
    dst->minMemType1 = to_ulong(src->minMemType1);
    dst->maxMemType1 = to_ulong(src->maxMemType1);
}
void convert_TT_PCLT_to_32(void* d, void* s)
{
    if(!s || !d) return;
    TT_PCLT_t* src = s;
    TT_PCLT_32_t* dst = d;

    dst->Version = to_long(src->Version);
    dst->FontNumber = to_ulong(src->FontNumber);
    dst->Pitch = src->Pitch;
    dst->xHeight = src->xHeight;
    dst->Style = src->Style;
    dst->TypeFamily = src->TypeFamily;
    dst->CapHeight = src->CapHeight;
    dst->SymbolSet = src->SymbolSet;
    for(int i=0; i<16; ++i)
        dst->TypeFace[i] = src->TypeFace[i];
    for(int i=0; i<8; ++i)
        dst->CharacterComplement[i] = src->CharacterComplement[i];
    for(int i=0; i<6; ++i)
        dst->FileName[i] = src->FileName[i];
    dst->StrokeWeight = src->StrokeWeight;
    dst->WidthType = src->WidthType;
    dst->SerifStyle = src->SerifStyle;
    dst->Reserved = src->Reserved;
}
// ==================================
// Wrapping
// ==================================

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

static uintptr_t my_iofunc = 0;
static unsigned long my_FT_Stream_IoFunc(FT_StreamRec_t* stream, unsigned long offset, unsigned char* buffer, unsigned long count )
{
    static FT_StreamRec_32_t stream_s;
    convert_FT_StreamRec_to_32(&stream_s, stream);
    unsigned char* buffer_s = buffer;
    if((uintptr_t)buffer_s>=0x100000000LL) {
        buffer_s = malloc(count);
        memcpy(buffer_s, buffer, count);
    }
    unsigned long ret = (unsigned long)RunFunctionFmt(my_iofunc, "pLpL", &stream_s, offset, buffer_s, count);
    convert_FT_StreamRec_to_64(stream, &stream_s);
    if(buffer_s!=buffer) {
        memcpy(buffer, buffer_s, count);
        free(buffer_s);
    }
    return ret;
}

static uintptr_t my_closefunc = 0;
static void my_FT_Stream_CloseFunc(FT_StreamRec_t* stream)
{
    static FT_StreamRec_32_t stream_s;
    convert_FT_StreamRec_to_32(&stream_s, stream);
    RunFunctionFmt(my_closefunc, "p", &stream_s);
    convert_FT_StreamRec_to_64(stream, &stream_s);
}

EXPORT int my32_FT_Open_Face(x64emu_t* emu, void* library, FT_Open_Args_32_t* args, long face_index, ptr_t* aface)
{
    (void)emu;
    void* aface_l = NULL;
    FT_Open_Args_t args_l = {0};
    // keep streams alive and in low memory, as they are living until FT_Face_Done is done. A better way would be to dynamicaly allocate it
    // and free when done, but needs to detect when stream is actualy allocated on box side...
    static FT_StreamRec_t streams[16] = {0};
    static int streams_idx = 0;
    FT_StreamRec_t *stream = &streams[streams_idx];
    streams_idx = (streams_idx+1)&15;
    FT_Parameter_t params[50];
    args_l.flags = args->flags;
    args_l.memory_base = from_ptrv(args->memory_base);
    args_l.memory_size = from_long(args->memory_size);
    args_l.pathname = from_ptrv(args->pathname);
    args_l.stream = args->stream?stream:NULL;
    if(args->stream)
        convert_FT_StreamRec_to_64(stream, from_ptrv(args->stream));
    args_l.driver = from_ptrv(args->driver);
    args_l.num_params = args->num_params;
    args_l.params = args->params?params:NULL;
    for(int i=0; args_l.num_params; ++i) {
        params[i].tag = from_ulong(((FT_Parameter_32_t*)from_ptrv(args->params))[i].tag);
        params[i].data = from_ptrv(((FT_Parameter_32_t*)from_ptrv(args->params))[i].data);
    }
    int wrapstream = (args->flags&0x02)?1:0;
    if(wrapstream) {
        my_iofunc = (uintptr_t)args_l.stream->read;
        if(my_iofunc) {
            args_l.stream->read = GetNativeFnc(my_iofunc);
            if(!args_l.stream->read)
                args_l.stream->read = my_FT_Stream_IoFunc;
        }
        my_closefunc = (uintptr_t)args_l.stream->close;
        if(my_closefunc) {
            args_l.stream->close = GetNativeFnc(my_closefunc);
            if(!args_l.stream->close)
                args_l.stream->close = my_FT_Stream_CloseFunc;
        }
    }
    int ret = my->FT_Open_Face(library, &args_l, face_index, &aface_l);
    /*if(wrapstream) {
        args->stream->read = (void*)my_iofunc;
        args->stream->close = (void*)my_closefunc;
    }*/
    *aface = to_ptrv(aface_l);
    inplace_FT_FaceRec_shrink(aface_l);
    return ret;
}

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
    if(!ret) return ret;
    static TT_Header_32_t tt_head = {0};
    static TT_MaxProfile_32_t tt_max = {0};
    static TT_OS2_32_t tt_os2 = {0};
    static TT_HoriHeader_32_t tt_hori = {0};
    static TT_VertHeader_32_t tt_vert = {0};
    static TT_Postscript_32_t tt_post = {0};
    static TT_PCLT_32_t tt_pclt = {0};
    switch(tag) {
        case 0: convert_TT_Header_to_32(&tt_head, ret); return &tt_head;
        case 1: convert_TT_MaxProfile_to_32(&tt_max, ret); return &tt_max;
        case 2: convert_TT_OS2_to_32(&tt_os2, ret); return &tt_os2;
        case 3: convert_TT_HoriHeader_to_32(&tt_hori, ret); return &tt_hori;
        case 4: convert_TT_VertHeader_to_32(&tt_vert, ret); return &tt_vert;
        case 5: convert_TT_Postscript_to_32(&tt_post, ret); return &tt_post;
        case 6: convert_TT_PCLT_to_32(&tt_pclt, ret); return &tt_pclt;
    }
    printf_log(LOG_NONE, "BOX32: Warning, unsupported type %d for FT_Get_Sfnt_Table\n", tag);
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

KHASH_MAP_INIT_INT(face_ref, uintptr_t);

static kh_face_ref_t *face_ref = NULL;

EXPORT int my32_FT_Reference_Face(x64emu_t* emu, void* face)
{
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Reference_Face(face);
    inplace_FT_FaceRec_shrink(face);
    if(!ret) {
        if(!face_ref) face_ref = kh_init(face_ref);
    }
    khint_t k = kh_get(face_ref, face_ref, (uintptr_t)face);
    if(k==kh_end(face_ref)) {
        int ret;
        k = kh_put(face_ref, face_ref, (uintptr_t)face, &ret);
        kh_value(face_ref, k) = 0;
    }
    ++kh_value(face_ref, k);
    return ret;
}

EXPORT int my32_FT_Done_Face(x64emu_t* emu, void* face)
{
    int will_keep = 0;
    khint_t k;
    if(face_ref && ((k=kh_get(face_ref, face_ref, (uintptr_t)face)!=kh_end(face_ref))))
        will_keep = 1;
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Done_Face(face);
    if(will_keep) {
        inplace_FT_FaceRec_shrink(face);
        --kh_value(face_ref, k);
        if(!kh_value(face_ref, k))
            kh_del(face_ref, face_ref, k);
    }
    return ret;
}

EXPORT void my32_FT_Set_Transform(x64emu_t* emu, void* face, FT_Matrix_32_t* matrix, FT_Vector_32_t* delta)
{
    FT_Matrix_t matrix_l = {0};
    FT_Vector_t delta_l = {0};
    inplace_FT_FaceRec_enlarge(face);
    if(matrix)
        convert_FT_Matrix_to_64(&delta_l, matrix);
    if(delta) {
        delta_l.x = from_long(delta->x);
        delta_l.y = from_long(delta->y);
    }
    my->FT_Set_Transform(face, matrix?(&matrix_l):NULL, delta?(&delta_l):NULL);
    inplace_FT_FaceRec_shrink(face);
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

EXPORT int my32_FT_Set_Charmap(x64emu_t* emu, void* face, void* charmap)
{
    // do not enlarge charmap, as it's already part of the face and so is expanded already
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Set_Charmap(face, charmap);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT int my32_FT_Get_Advance(x64emu_t* emu, void* face, uint32_t gindex, int load_flags, long_t*padvance)
{
    signed long advance_l = 0;
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Get_Advance(face, gindex, load_flags, &advance_l);
    inplace_FT_FaceRec_shrink(face);
    *padvance = to_long(advance_l);
    return ret;
}

EXPORT int my32_FT_Get_MM_Var(x64emu_t* emu, void* face, FT_MM_Var_32_t* amaster)
{
    FT_MM_Var_t* amaster_l = NULL;
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Get_MM_Var(face, amaster_l);
    inplace_FT_FaceRec_shrink(face);
    if(!amaster_l) return ret;
    // create a 32bits structure...
    size_t sz = sizeof(FT_MM_Var_32_t)+sizeof(void*);
    if(amaster_l->axis)
        sz += amaster_l->num_axis*sizeof(FT_Var_Axis_32_t);
    if(amaster->namedstyle)
        sz += amaster_l->num_namedstyles*(sizeof(FT_Var_Named_Style_32_t) + amaster_l->num_axis*sizeof(long_t));
    void* p = box32_calloc(1, sz);
    amaster = p; p += sizeof(FT_MM_Var_32_t);
    *(void**)p = amaster_l; // save original value
    p += sizeof(void*);
    amaster->num_axis = amaster_l->num_axis;
    amaster->num_designs = amaster_l->num_designs;
    amaster->num_namedstyles = amaster_l->num_namedstyles;
    if(amaster_l->axis) {
        amaster->axis = to_ptrv(p);
        FT_Var_Axis_32_t* axis = p;
        p += amaster_l->num_axis*sizeof(FT_Var_Axis_32_t);
        for(uint32_t i=0; i<amaster_l->num_axis; ++i) {
            axis[i].name = to_cstring(amaster_l->axis[i].name);
            axis[i].minimum = to_long(amaster_l->axis[i].minimum);
            axis[i].def = to_long(amaster_l->axis[i].def);
            axis[i].maximum = to_long(amaster_l->axis[i].maximum);
            axis[i].tag = to_ulong(amaster_l->axis[i].tag);
            axis[i].strid = amaster_l->axis[i].strid;
        }
    } else
        amaster->axis = 0;
    if(amaster_l->namedstyle) {
        amaster->axis = to_ptrv(p);
        FT_Var_Named_Style_32_t* axis = p;
        p += amaster_l->num_axis*sizeof(FT_Var_Named_Style_32_t);
        for(uint32_t i=0; i<amaster_l->num_namedstyles; ++i) {
            axis[i].coords = to_ptrv(p);
            long_t* coords = p;
            p += sizeof(long_t)*amaster_l->num_axis;
            for(uint32_t j=0; j<amaster_l->num_axis; ++j)
                coords[j] = to_long(amaster_l->namedstyle[i].coords[j]);
        }
    } else
        amaster->namedstyle = 0;
    return ret;
}

EXPORT int my32_FT_Done_MM_Var(x64emu_t* emu, void* face, FT_MM_Var_32_t* amaster)
{
    inplace_FT_FaceRec_enlarge(face);
    void* amaster_l = amaster+1;
    int ret = my->FT_Done_MM_Var(face, amaster_l);
    inplace_FT_FaceRec_shrink(face);
    box32_free(amaster);    // should check result first?
    return ret;
}

EXPORT int my32_FT_Set_Var_Design_Coordinates(x64emu_t* emu, void* face, uint32_t num_coords, long_t* coords)
{
    long coords_l[num_coords];
    for(uint32_t i=0; i<num_coords; ++i)
        coords_l[i] = from_long(coords[i]);
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Set_Var_Design_Coordinates(face, num_coords, coords_l);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT int my32_FT_Set_Var_Blend_Coordinates(x64emu_t* emu, void* face, uint32_t num_coords, long_t* coords)
{
    long coords_l[num_coords];
    for(uint32_t i=0; i<num_coords; ++i)
        coords_l[i] = from_long(coords[i]);
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Set_Var_Blend_Coordinates(face, num_coords, coords_l);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT int my32_FT_Get_Var_Blend_Coordinates(x64emu_t* emu, void* face, uint32_t num_coords, long_t* coords)
{
    long coords_l[num_coords];
    memset(coords_l, 0, sizeof(coords_l));
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Get_Var_Blend_Coordinates(face, num_coords, coords_l);
    inplace_FT_FaceRec_shrink(face);
    for(uint32_t i=0; i<num_coords; ++i)
        coords[i] = to_long(coords_l[i]);
    return ret;
}

EXPORT uint32_t my32_FT_Face_GetCharVariantIndex(x64emu_t* emu, void* face, unsigned long charcode, unsigned long variantSel)
{
    inplace_FT_FaceRec_enlarge(face);
    uint32_t ret = my->FT_Face_GetCharVariantIndex(face, charcode, variantSel);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT uint32_t my32_FT_Get_Name_Index(x64emu_t* emu, void* face, void* name)
{
    inplace_FT_FaceRec_enlarge(face);
    uint32_t ret = my->FT_Get_Name_Index(face, name);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT uint16_t my32_FT_Get_FSType_Flags(x64emu_t* emu, void* face)
{
    inplace_FT_FaceRec_enlarge(face);
    uint16_t ret = my->FT_Get_FSType_Flags(face);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT int my32_FT_Get_Advances(x64emu_t* emu, void* face, uint32_t start, uint32_t count, int flags, long_t* padvances)
{
    signed long advances_l[count];
    memset(advances_l, 0, sizeof(advances_l));
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Get_Advances(face, start, count, flags, advances_l);
    inplace_FT_FaceRec_shrink(face);
    for(uint32_t i=0; i<count; ++i)
        padvances[i] = to_long(advances_l[i]);
    return ret;
}

EXPORT void my32_FT_Outline_Get_CBox(x64emu_t* emu, FT_Outline_32_t* outline, FT_BBox_32_t* bbox)
{
    FT_Outline_t outline_l = {0};
    FT_BBox_t res = {0};
    convert_FT_Outline_to_64(&outline_l, outline);
    my->FT_Outline_Get_CBox(&outline_l, &res);
    bbox->xMin = to_long(res.xMin);
    bbox->yMin = to_long(res.yMin);
    bbox->xMax = to_long(res.xMax);
    bbox->yMax = to_long(res.yMax);
}

EXPORT int my32_FT_Outline_Copy(x64emu_t* emu, FT_Outline_32_t* source, FT_Outline_32_t* target)
{
    FT_Outline_t source_l = {0}, target_l = {0};
    convert_FT_Outline_to_64(&source_l, source);
    convert_FT_Outline_to_64(&target_l, target);
    int ret = my->FT_Outline_Copy(source, target);
    convert_FT_Outline_to_32(target, &target_l);
    return ret;
}

EXPORT int my32_FT_Render_Glyph(x64emu_t* emu, FT_GlyphSlotRec_32_t* glyph, uint32_t mode)
{
    #if 1
    void* face = from_ptrv(glyph->face);
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Render_Glyph(glyph, mode);
    inplace_FT_FaceRec_shrink(face);
    #else
    FT_GlyphSlotRec_t slot = {0};
    convert_FT_GlyphSlot_to_64(&slot, glyph);
    int ret = my->FT_Render_Glyph(&slot, mode);
    convert_FT_GlyphSlot_to_32(glyph, &slot);
    #endif
    return ret;
}

EXPORT int my32_FT_Get_WinFNT_Header(x64emu_t* emu, void* face, FT_WinFNT_HeaderRec_32_t* aheader)
{
    FT_WinFNT_HeaderRec_t aheader_l = {0};
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Get_WinFNT_Header(face, &aheader_l);
    inplace_FT_FaceRec_shrink(face);
    if(!ret)
        convert_FT_WinFNT_HeaderRec_to_32(aheader, &aheader_l);
    return ret;
}

EXPORT void my32_FT_Matrix_Multiply(x64emu_t* emu, FT_Matrix_32_t* a, FT_Matrix_32_t* b)
{
    FT_Matrix_t a_l, b_l;
    convert_FT_Matrix_to_64(&a_l, a);
    convert_FT_Matrix_to_64(&b_l, b);
    my->FT_Matrix_Multiply(&a_l, &b_l);
    convert_FT_Matrix_to_32(b, &b_l);
}

EXPORT int my32_FT_Outline_Get_Bitmap(x64emu_t* emu, void* lib, FT_Outline_32_t* outline, FT_Bitmap_32_t* bitmap)
{
    FT_Bitmap_t bitmap_l = {0};
    FT_Outline_t outline_l = {0};
    convert_FT_Outline_to_64(&outline_l, outline);
    convert_FT_Bitmap_to_64(&bitmap_l, bitmap);
    int ret = my->FT_Outline_Get_Bitmap(lib, &outline_l, &bitmap_l);
    convert_FT_Bitmap_to_32(bitmap, &bitmap_l);
    convert_FT_Outline_to_32(outline, &outline_l);  //usefull?
    return ret;
}

EXPORT void my32_FT_Outline_Transform(x64emu_t* emu, FT_Outline_32_t* outline, FT_Matrix_32_t* matrix)
{
    FT_Matrix_t matrix_l = {0};
    FT_Outline_t outline_l = {0};
    convert_FT_Outline_to_64(&outline_l, outline);
    convert_FT_Matrix_to_64(&matrix_l, matrix);
    my->FT_Outline_Transform(&outline_l, &matrix_l);
    convert_FT_Outline_to_32(outline, &outline_l);
}

EXPORT void my32_FT_Outline_Translate(x64emu_t* emu, FT_Outline_32_t* outline, long x, long y)
{
    FT_Outline_t outline_l = {0};
    convert_FT_Outline_to_64(&outline_l, outline);
    my->FT_Outline_Translate(&outline_l, x, y);
    convert_FT_Outline_to_32(outline, &outline_l);
}

EXPORT void my32_FT_Outline_Embolden(x64emu_t* emu, FT_Outline_32_t* outline, long strength)
{
    FT_Outline_t outline_l = {0};
    convert_FT_Outline_to_64(&outline_l, outline);
    my->FT_Outline_Embolden(&outline_l, strength);
    convert_FT_Outline_to_32(outline, &outline_l);
}

EXPORT void my32_FT_Outline_EmboldenXY(x64emu_t* emu, FT_Outline_32_t* outline, long xstrength, long ystrength)
{
    FT_Outline_t outline_l = {0};
    convert_FT_Outline_to_64(&outline_l, outline);
    my->FT_Outline_EmboldenXY(&outline_l, xstrength, ystrength);
    convert_FT_Outline_to_32(outline, &outline_l);
}

EXPORT int my32_FT_Outline_Decompose(x64emu_t* emu, FT_Outline_32_t* outline, my_FT_Outline_Funcs_t* tbl, void* data)
{
    FT_Outline_t outline_l = {0};
    convert_FT_Outline_to_64(&outline_l, outline);
    //
    my_FT_Outline_Funcs_t f = {0};
    if(tbl) {
        f.move_to = find_FT_Outline_MoveToFunc_Fct(tbl->move_to);
        f.line_to = find_FT_Outline_LineToFunc_Fct(tbl->line_to);
        f.conic_to = find_FT_Outline_ConicToFunc_Fct(tbl->conic_to);
        f.cubic_to = find_FT_Outline_CubicToFunc_Fct(tbl->cubic_to);
        f.shift = tbl->shift;
        f.delta = tbl->delta;
    }
    int ret = my->FT_Outline_Decompose(&outline_l, tbl?(&f):tbl, data);
    convert_FT_Outline_to_32(outline, &outline_l);
    return ret;
}

EXPORT int my32_FT_Outline_Done(x64emu_t* emu, void* library, FT_Outline_32_t* outline)
{
    FT_Outline_t outline_l = {0};
    convert_FT_Outline_to_64(&outline_l, outline);
    int ret = my->FT_Outline_Done(library, &outline_l);
    return ret;
}

EXPORT int my32_FT_Outline_New(x64emu_t* emu, void* library, uint32_t numPoints, int numContours, FT_Outline_32_t* outline)
{
    FT_Outline_t outline_l = {0};
    int ret = my->FT_Outline_New(library, numPoints, numContours, &outline_l);
    convert_FT_Outline_to_32(outline, &outline_l);
    return ret;
}


EXPORT int my32_FT_Activate_Size(x64emu_t* emu, FT_SizeRec_32_t* size)
{
    //FT_SizeRec_t size_l = {0};
    //convert_FT_SizeRec_to_64(&size_l, size);
    FT_FaceRec_t* face = from_ptrv(size->face);
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Activate_Size(size);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}
EXPORT int my32_FT_Done_Size(x64emu_t* emu, FT_SizeRec_32_t* size)
{
    //FT_SizeRec_t size_l = {0};
    //convert_FT_SizeRec_to_64(&size_l, size);
    FT_FaceRec_t* face = from_ptrv(size->face);
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_Done_Size(size);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT int my32_FT_New_Size(x64emu_t* emu, FT_FaceRec_32_t* face, ptr_t* size)
{
    void* size_l = NULL;
    inplace_FT_FaceRec_enlarge(face);
    int ret = my->FT_New_Size(face, &size_l);
    *size = to_ptrv(size_l);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

EXPORT int my32_FT_Get_Glyph(x64emu_t* emu, FT_GlyphSlotRec_32_t* slot, ptr_t* glyph)
{
    FT_GlyphSlotRec_t slot_l = {0};
    convert_FT_GlyphSlot_to_64(&slot_l, slot);
    void* glyph_l;
    int ret = my->FT_Get_Glyph(&slot_l, &glyph_l);
    *glyph = to_ptrv(glyph_l);
    convert_FT_GlyphSlot_to_32(slot, &slot_l);
    inplace_FT_Glyph_shrink(glyph_l);
    return ret;
}

EXPORT int my32_FT_GlyphSlot_Own_Bitmap(x64emu_t* emu, FT_GlyphSlotRec_32_t* slot)
{
    FT_GlyphSlotRec_t slot_l = {0};
    convert_FT_GlyphSlot_to_64(&slot_l, slot);
    int ret = my->FT_GlyphSlot_Own_Bitmap(&slot_l);
    convert_FT_GlyphSlot_to_32(slot, &slot_l);  //usefull?
    return ret;
}

EXPORT int my32_FT_Glyph_Copy(x64emu_t* emu, void* src, void** dst)
{
    inplace_FT_Glyph_enlarge(src);
    int ret = my->FT_Glyph_Copy(src, (void*)dst);
    inplace_FT_Glyph_shrink(src);
    inplace_FT_Glyph_shrink(*dst);
    return ret;
}

EXPORT void my32_FT_Glyph_Get_CBox(x64emu_t* emu, void* glyph, uint32_t mode, void* bbox)
{
    inplace_FT_Glyph_enlarge(glyph);
    my->FT_Glyph_Get_CBox(glyph, mode, bbox);
    inplace_FT_Glyph_shrink(glyph);
}

EXPORT int my32_FT_Glyph_Transform(x64emu_t* emu, void* glyph, void* mat, void* vec)
{
    inplace_FT_Glyph_enlarge(glyph);
    int ret = my->FT_Glyph_Transform(glyph, mat, vec);
    inplace_FT_Glyph_shrink(glyph);
    return ret;
}

EXPORT void my32_FT_Done_Glyph(x64emu_t* emu, void* glyph)
{
    inplace_FT_FaceRec_enlarge(glyph);
    my->FT_Done_Glyph(glyph);
}

#include "wrappedlib_init32.h"
