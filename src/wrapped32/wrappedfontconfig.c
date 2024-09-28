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

typedef struct FcValue_32_s {
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

//#define ADDED_FUNCTIONS()                   \

//#include "generated/wrappedfontconfigtypes32.h"

typedef int (*iFppSi_t)(void*, void*, FcValue_t, int);

#define SUPER() \
    GO(FcPatternAdd, iFppSi_t)              \

#include "wrappercallback32.h"

//EXPORT void* my_FcObjectSetVaBuild(x64emu_t* emu, void* first, x64_va_list_t V)
//{
//    #ifdef CONVERT_VALIST
//    CONVERT_VALIST(V);
//    #else
//    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
//    #endif
//    return my->FcObjectSetVaBuild(first, VARARGS);
//}
//EXPORT void* my_FcObjectSetBuild(x64emu_t* emu, void* first, uint64_t* b)
//{
//    if(!first)    
//        return my->FcObjectSetBuild(first, NULL);
//    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 1);
//    return my->FcObjectSetVaBuild(first, VARARGS);
//}

//EXPORT void* my_FcPatternVaBuild(x64emu_t* emu, void* pattern, x64_va_list_t V)
//{
//    #ifdef CONVERT_VALIST
//    CONVERT_VALIST(V);
//    #else
//    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
//    #endif
//    return my->FcPatternVaBuild(pattern, VARARGS);
//}
//EXPORT void* my_FcPatternBuild(x64emu_t* emu, void* pattern, uint64_t* b)
//{
//    if(!pattern)    
//        return my->FcPatternBuild(pattern, NULL);
//    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 1);
//    return my->FcPatternVaBuild(pattern, VARARGS);
//}

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
    return my->FcPatternAdd(p, object, val, append);
}

#define NEEDED_LIBS "libexpat.so.1", "libfreetype.so.6"

#include "wrappedlib_init32.h"
