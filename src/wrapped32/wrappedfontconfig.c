#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

typedef struct FcValue_s {
    int	type;
    union {
        const char	*s;
        int		i;
        int		b;
        double		d;
        void	*m; //const FcMatrix
        void	*c; //const FcCharSet
        void		*f;
        void	*l; //const FcLangSet
    } u;
} FcValue_t;

typedef struct FcPattern_s {
    int		    num;
    int		    size;
    intptr_t	elts_offset;
    int		    ref;
} FcPattern_t;

typedef struct FcFontSet_s {
    int		nfont;
    int		sfont;
    FcPattern_t	**fonts;
} FcFontSet_t;

typedef struct FcStrSet_s {
    int		    ref;
    int		    num;
    int		    size;
    void	    **strs;
} FcStrSet_t;

typedef struct FcBlanks_s {
    int		nblank;
    int		sblank;
    void	*blanks;
} FcBlanks_t;

typedef struct FcConfig_s {
    FcStrSet_t	*configDirs;
    FcBlanks_t  *blanks;
    FcStrSet_t	*fontDirs;
    FcStrSet_t	*cacheDirs;
    FcStrSet_t	*configFiles;
    void	    *substPattern;  //FcSubst
    void	    *substFont; //FcSubst
    void	    *substScan; //FcSubst
    int		    maxObjects;
    FcStrSet_t	*acceptGlobs;
    FcStrSet_t	*rejectGlobs;
    FcFontSet_t	*acceptPatterns;
    FcFontSet_t	*rejectPatterns;
    FcFontSet_t	*fonts[2];
    time_t	    rescanTime;
    int		    rescanInterval;
    int		    ref;
    void        *expr_pool;    //FcExprPage
} FcConfig_t;

// 32bits structures

typedef struct __attribute__((packed, aligned(4))) FcValue_32_s {
    int	type;
    union {
        ptr_t   s;//const char	*
        int		i;
        int		b;
        double  d;
        ptr_t   m; //const FcMatrix*
        ptr_t   c; //const FcCharSet*
        ptr_t   f; //void*
        ptr_t   l; //const FcLangSet*
    } u;
} FcValue_32_t;

typedef struct FcPattern_32_s {
    int		    num;
    int		    size;
    long_t	    elts_offset;
    int		    ref;
} FcPattern_32_t;

typedef struct FcFontSet_32_s {
    int		nfont;
    int		sfont;
    ptr_t   fonts;//FcPattern_t	**
} FcFontSet_32_t;

typedef struct FcStrSet_32_s {
    int		    ref;
    int		    num;
    int		    size;
    ptr_t       strs;   //void	    **
} FcStrSet_32_t;

typedef struct FcBlanks_32_s {
    int		nblank;
    int		sblank;
    ptr_t   blanks; //void	*
} FcBlanks_32_t;

typedef struct FcConfig_32_s {
    ptr_t       configDirs;     //FcStrSet_t*
    ptr_t       blanks;         //FcBlanks_t*
    ptr_t       fontDirs;       //FcStrSet_t*
    ptr_t       cacheDirs;      //FcStrSet_t*
    ptr_t       configFiles;    //FcStrSet_t*
    ptr_t       substPattern;   //FcSubst*
    ptr_t       substFont;      //FcSubst*
    ptr_t       substScan;      //FcSubst*
    int		    maxObjects;
    ptr_t       acceptGlobs;    //FcStrSet_t*
    ptr_t       rejectGlobs;    //FcStrSet_t*
    ptr_t       acceptPatterns; //FcFontSet_t*
    ptr_t       rejectPatterns; //FcFontSet_t*
    ptr_t       fonts[2];       //FcFontSet_t*
    long_t	    rescanTime;
    int		    rescanInterval;
    int		    ref;
    ptr_t       expr_pool;    //FcExprPage*
} FcConfig_32_t;

#ifdef ANDROID
    static const char* fontconfigName = "libfontconfig.so";
#else
    static const char* fontconfigName = "libfontconfig.so.1";
#endif

#define LIBNAME fontconfig

#define ADDED_STRUCT()                          \
    iFppSi_t FcPatternAdd_;                     \

#define ADDED_INIT()                            \
    my->FcPatternAdd_ = (void*)my->FcPatternAdd;\

typedef int  (*iFppSi_t)(void*, void*, FcValue_t, int);
typedef void*(*pFpA_t)(void*, va_list);

#define ADDED_FUNCTIONS()                       \
    GO(FcObjectSetVaBuild, pFpA_t);             \
    GO(FcPatternVaBuild, pFpA_t);               \

#include "generated/wrappedfontconfigtypes32.h"

#include "wrappercallback32.h"

EXPORT void* my32_FcObjectSetVaBuild(x64emu_t* emu, void* first, void* V)
{
    PREPARE_VALIST_32_(V);
    return my->FcObjectSetVaBuild(first, VARARGS_32_(V));
}
EXPORT void* my32_FcObjectSetBuild(x64emu_t* emu, void* first, uint64_t* b)
{
    if(!first)    
        return my->FcObjectSetBuild(first, NULL);
    PREPARE_VALIST_32_(b);
    return my->FcObjectSetVaBuild(first, VARARGS_32_(b));
}

EXPORT void* my32_FcPatternVaBuild(x64emu_t* emu, void* pattern, void* V)
{
    PREPARE_VALIST_32_(V);
    return my->FcPatternVaBuild(pattern, VARARGS_32_(V));
}
EXPORT void* my32_FcPatternBuild(x64emu_t* emu, void* pattern, uint64_t* b)
{
    if(!pattern)    
        return my->FcPatternBuild(pattern, NULL);
    PREPARE_VALIST_32_(b);
    return my->FcPatternVaBuild(pattern, VARARGS_32_(b));
}

typedef union fcvalue_32s {
    ptr_t   p;
    double  d;
    int     i;
} fcvalue_32_t;
EXPORT int my32_FcPatternAdd(void* p, void* object, int type, fcvalue_32_t t, int append)
{
    // adjusting value "t" of the FcValue
    FcValue_t val = {0};
    val.type = type;
    switch(type) {
        case 3:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            val.u.c = from_ptrv(t.p);
            break;
        case 2:
            val.u.d = t.d;
        case 1:
        case 4: // just in case
            val.u.i = t.i;
            break;
    }
    return my->FcPatternAdd_(p, object, val, append);
}

void inplace_FT_FaceRec_shrink(void* face);
void inplace_FT_FaceRec_enlarge(void* face);
EXPORT uint32_t my32_FcFreeTypeCharIndex(x64emu_t* emu, void* face, uint32_t u)
{
    inplace_FT_FaceRec_enlarge(face);
    uint32_t ret = my->FcFreeTypeCharIndex(face, u);
    inplace_FT_FaceRec_shrink(face);
    return ret;
}

void* inplace_FcFontSet_shrink(void* set)
{
    if(!set) return set;
    FcFontSet_t* src = set;
    FcFontSet_32_t* dst = set;

    for(int i=0; i<src->nfont; ++i) {
        ((ptr_t*)src->fonts)[i] = to_ptrv(src->fonts[i]);
    }
    dst->nfont = src->nfont;
    dst->sfont = src->sfont;
    dst->fonts = to_ptrv(src->fonts);

    return set;
}
void* inplace_FcFontSet_enlarge(void* set)
{
    FcFontSet_32_t* src = set;
    FcFontSet_t* dst = set;

    dst->fonts = from_ptrv(src->fonts);
    dst->sfont = src->sfont;
    dst->nfont = src->nfont;
    for(int i=src->nfont-1; i>=0; --i) {
        dst->fonts[i] = from_ptrv(((ptr_t*)dst->fonts)[i]);
    }

    return set;
}

EXPORT void* my32_FcFontList(x64emu_t* emu, void* config, void* pattern, void* os)
{
    return inplace_FcFontSet_shrink(my->FcFontList(config, pattern, os));
}

EXPORT void* my32_FcFontSort(x64emu_t* emu, void* config, void* pattern, int trim, void* csp, int* result)
{
    return inplace_FcFontSet_shrink(my->FcFontSort(config, pattern, trim, csp, result));
}

EXPORT void* my32_FcCacheCopySet(x64emu_t* emu, void* cache)
{
    return inplace_FcFontSet_shrink(my->FcCacheCopySet(cache));
}

EXPORT void my32_FcFontSetDestroy(x64emu_t* emu, void* set)
{
    my->FcFontSetDestroy(inplace_FcFontSet_enlarge(set));
}

EXPORT void* my32_FcFontSetCreate(x64emu_t* emu)
{
    void* ret = my->FcFontSetCreate();
    return inplace_FcFontSet_shrink(ret);
}

EXPORT int my32_FcFontSetAdd(x64emu_t* emu, void* set, void* pattern)
{
    inplace_FcFontSet_enlarge(set);
    int ret = my->FcFontSetAdd(set, pattern);
    inplace_FcFontSet_shrink(set);
    return ret;
}

EXPORT void* my32_FcConfigGetFonts(x64emu_t* emu, void* config, uint32_t name)
{
    inplace_FcFontSet_shrink(my->FcConfigGetFonts(config, name));   // that's probably a bad idea, as the font is own by the config
}

EXPORT void* my32_FcFontSetSort(x64emu_t* emu, void* config, ptr_t* sets, int nsets, void* pattern, int trim, ptr_t* csp, void* result)
{
    void** sets_l[nsets];
    void* csp_l = csp?from_ptrv(*csp):NULL;
    for(int i=0; i<nsets; ++i)
        inplace_FcFontSet_enlarge((sets_l[i]=from_ptrv(sets[i])));

    void* ret = my->FcFontSetSort(config, sets_l, nsets, pattern, trim, csp?(&csp_l):NULL, result);

    for(int i=0; i<nsets; ++i)
        inplace_FcFontSet_shrink(from_ptrv(sets[i]));
    
    if(csp)
        *csp = to_ptrv(csp_l);
    
    return inplace_FcFontSet_shrink(ret);
}

#define NEEDED_LIBS "libexpat.so.1", "libfreetype.so.6"

#include "wrappedlib_init32.h"
