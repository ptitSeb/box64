#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "bridge.h"
#include "alternate.h"
#include "debug.h"
#include "callback.h"
#include "librarian.h"
#include "gtkclass.h"
#include "library.h"

// This file takes ages to compile, and yet wrapper.h changed pretty often, remove
// the dependency and declare the functions and macros here to prevent the recompilation as much as possible.
// #include "wrapper.h"
/*
To generate this content again, you can use the following Python code:

import re
file_path = 'path/to/box64/src/tools/gtkclass.c'
with open(file_path, 'r') as file:
    content = file.read()
pattern = r' ([a-zA-Z]F[a-zA-Z]+)'
matches = re.findall(pattern, content)
unique_sorted_matches = sorted(set(matches))
for match in unique_sorted_matches:
    print(f'void {match}(x64emu_t *emu, uintptr_t fnc);')
*/

void UFp(x64emu_t* emu, uintptr_t fnc);
void iFL(x64emu_t* emu, uintptr_t fnc);
void iFp(x64emu_t* emu, uintptr_t fnc);
void iFpUU(x64emu_t* emu, uintptr_t fnc);
void iFpUup(x64emu_t* emu, uintptr_t fnc);
void iFpi(x64emu_t* emu, uintptr_t fnc);
void iFpii(x64emu_t* emu, uintptr_t fnc);
void iFpiii(x64emu_t* emu, uintptr_t fnc);
void iFpiiip(x64emu_t* emu, uintptr_t fnc);
void iFpip(x64emu_t* emu, uintptr_t fnc);
void iFpipLpp(x64emu_t* emu, uintptr_t fnc);
void iFpippp(x64emu_t* emu, uintptr_t fnc);
void iFpp(x64emu_t* emu, uintptr_t fnc);
void iFppLp(x64emu_t* emu, uintptr_t fnc);
void iFppU(x64emu_t* emu, uintptr_t fnc);
void iFppUup(x64emu_t* emu, uintptr_t fnc);
void iFppdd(x64emu_t* emu, uintptr_t fnc);
void iFppi(x64emu_t* emu, uintptr_t fnc);
void iFppii(x64emu_t* emu, uintptr_t fnc);
void iFppiiu(x64emu_t* emu, uintptr_t fnc);
void iFppp(x64emu_t* emu, uintptr_t fnc);
void iFpppi(x64emu_t* emu, uintptr_t fnc);
void iFpppp(x64emu_t* emu, uintptr_t fnc);
void iFppppp(x64emu_t* emu, uintptr_t fnc);
void iFpu(x64emu_t* emu, uintptr_t fnc);
void iFpupp(x64emu_t* emu, uintptr_t fnc);
void iFppppi(x64emu_t* emu, uintptr_t fnc);
void iFppppppi(x64emu_t* emu, uintptr_t fnc);
void pFL(x64emu_t* emu, uintptr_t fnc);
void pFLup(x64emu_t* emu, uintptr_t fnc);
void pFp(x64emu_t* emu, uintptr_t fnc);
void pFpLi(x64emu_t* emu, uintptr_t fnc);
void pFpLp(x64emu_t* emu, uintptr_t fnc);
void pFpi(x64emu_t* emu, uintptr_t fnc);
void pFpipp(x64emu_t* emu, uintptr_t fnc);
void pFpll(x64emu_t* emu, uintptr_t fnc);
void pFpp(x64emu_t* emu, uintptr_t fnc);
void pFppL(x64emu_t* emu, uintptr_t fnc);
void pFppp(x64emu_t* emu, uintptr_t fnc);
void pFppii(x64emu_t* emu, uintptr_t fnc);
void pFpppp(x64emu_t* emu, uintptr_t fnc);
void pFppppp(x64emu_t* emu, uintptr_t fnc);
void pFpppppp(x64emu_t* emu, uintptr_t fnc);
void pFppppppi(x64emu_t* emu, uintptr_t fnc);
void pFv(x64emu_t* emu, uintptr_t fnc);
void uFpp(x64emu_t* emu, uintptr_t fnc);
void vFLp(x64emu_t* emu, uintptr_t fnc);
void vFLup(x64emu_t* emu, uintptr_t fnc);
void vFp(x64emu_t* emu, uintptr_t fnc);
void vFpdd(x64emu_t* emu, uintptr_t fnc);
void vFpi(x64emu_t* emu, uintptr_t fnc);
void vFpii(x64emu_t* emu, uintptr_t fnc);
void vFpiii(x64emu_t* emu, uintptr_t fnc);
void vFpip(x64emu_t* emu, uintptr_t fnc);
void vFpipp(x64emu_t* emu, uintptr_t fnc);
void vFpipppp(x64emu_t* emu, uintptr_t fnc);
void vFpp(x64emu_t* emu, uintptr_t fnc);
void vFppdd(x64emu_t* emu, uintptr_t fnc);
void vFppi(x64emu_t* emu, uintptr_t fnc);
void vFppiipuu(x64emu_t* emu, uintptr_t fnc);
void vFppip(x64emu_t* emu, uintptr_t fnc);
void vFppp(x64emu_t* emu, uintptr_t fnc);
void vFpppp(x64emu_t* emu, uintptr_t fnc);
void vFppppp(x64emu_t* emu, uintptr_t fnc);
void vFpppppp(x64emu_t* emu, uintptr_t fnc);
void vFpppuu(x64emu_t* emu, uintptr_t fnc);
void vFppu(x64emu_t* emu, uintptr_t fnc);
void vFppupp(x64emu_t* emu, uintptr_t fnc);
void vFppuu(x64emu_t* emu, uintptr_t fnc);
void vFpu(x64emu_t* emu, uintptr_t fnc);
void vFpup(x64emu_t* emu, uintptr_t fnc);
void vFpupp(x64emu_t* emu, uintptr_t fnc);
void vFu(x64emu_t* emu, uintptr_t fnc);
void vFppppppp(x64emu_t* emu, uintptr_t fnc);
void vFppppppi(x64emu_t* emu, uintptr_t fnc);

static bridge_t*        my_bridge           = NULL;
static const char* (*g_type_name)(size_t)   = NULL;
static size_t (*g_type_parent)(size_t)      = NULL;
static void* (*g_type_class_peek)(size_t)   = NULL;
#define GTKCLASS(A) static size_t my_##A    = (size_t)-1;
#define GTKIFACE(A) GTKCLASS(A)
GTKCLASSES()
#undef GTKIFACE
#undef GTKCLASS

KHASH_SET_INIT_INT(signalmap)
static kh_signalmap_t *my_signalmap = NULL;

typedef struct sigoffset_s {
    uint32_t offset;
    int     n;
} sigoffset_t;
typedef struct sigoffset_array_s {
    sigoffset_t *a;
    int     cap;
    int     sz;
} sigoffset_array_t;

KHASH_MAP_INIT_INT64(sigoffset, sigoffset_array_t)
static kh_sigoffset_t *my_sigoffset = NULL;

KHASH_MAP_INIT_INT64(customclass, char*)
static kh_customclass_t *my_customclass = NULL;

// ---- Defining the multiple functions now -----
#include "super80.h"

#define WRAPPED_RET(A, NAME, RET, FRET, DEF, FMT, ...)  \
static uintptr_t my_##NAME##_fct_##A = 0;   \
static RET my_##NAME##_##A DEF              \
{                                           \
    printf_log(LOG_DEBUG, "Calling " #NAME "_" #A " wrapper\n");                \
    return FRET((RET)RunFunctionFmt(my_##NAME##_fct_##A, FMT, __VA_ARGS__));    \
}

#define WRAPPED(A, NAME, RET, DEF, FMT, ...)  \
static uintptr_t my_##NAME##_fct_##A = 0;   \
static RET my_##NAME##_##A DEF              \
{                                           \
    printf_log(LOG_DEBUG, "Calling " #NAME "_" #A " wrapper\n");             \
    return (RET)RunFunctionFmt(my_##NAME##_fct_##A, FMT, __VA_ARGS__);\
}

#define FIND(A, NAME) \
static void* find_##NAME##_##A(wrapper_t W, void* fct)                            \
{                                                                                 \
    if(!fct) return fct;                                                          \
    void* tmp = GetNativeFnc((uintptr_t)fct);                                     \
    if(tmp) {AddAutomaticBridge(my_bridge, W, fct, 0, #NAME "_" #A); return tmp;} \
    if(my_##NAME##_##A##_fct_0 == (uintptr_t)fct) return my_##NAME##_##A##_0;     \
    if(my_##NAME##_##A##_fct_1 == (uintptr_t)fct) return my_##NAME##_##A##_1;     \
    if(my_##NAME##_##A##_fct_2 == (uintptr_t)fct) return my_##NAME##_##A##_2;     \
    if(my_##NAME##_##A##_fct_3 == (uintptr_t)fct) return my_##NAME##_##A##_3;     \
    if(my_##NAME##_##A##_fct_4 == (uintptr_t)fct) return my_##NAME##_##A##_4;     \
    if(my_##NAME##_##A##_fct_5 == (uintptr_t)fct) return my_##NAME##_##A##_5;     \
    if(my_##NAME##_##A##_fct_6 == (uintptr_t)fct) return my_##NAME##_##A##_6;     \
    if(my_##NAME##_##A##_fct_7 == (uintptr_t)fct) return my_##NAME##_##A##_7;     \
    if(my_##NAME##_##A##_fct_8 == (uintptr_t)fct) return my_##NAME##_##A##_8;     \
    if(my_##NAME##_##A##_fct_9 == (uintptr_t)fct) return my_##NAME##_##A##_9;     \
    if(my_##NAME##_##A##_fct_10 == (uintptr_t)fct) return my_##NAME##_##A##_10;   \
    if(my_##NAME##_##A##_fct_11 == (uintptr_t)fct) return my_##NAME##_##A##_11;   \
    if(my_##NAME##_##A##_fct_12 == (uintptr_t)fct) return my_##NAME##_##A##_12;   \
    if(my_##NAME##_##A##_fct_13 == (uintptr_t)fct) return my_##NAME##_##A##_13;   \
    if(my_##NAME##_##A##_fct_14 == (uintptr_t)fct) return my_##NAME##_##A##_14;   \
    if(my_##NAME##_##A##_fct_15 == (uintptr_t)fct) return my_##NAME##_##A##_15;   \
    if(my_##NAME##_##A##_fct_16 == (uintptr_t)fct) return my_##NAME##_##A##_16;   \
    if(my_##NAME##_##A##_fct_17 == (uintptr_t)fct) return my_##NAME##_##A##_17;   \
    if(my_##NAME##_##A##_fct_18 == (uintptr_t)fct) return my_##NAME##_##A##_18;   \
    if(my_##NAME##_##A##_fct_19 == (uintptr_t)fct) return my_##NAME##_##A##_19;   \
    if(my_##NAME##_##A##_fct_20 == (uintptr_t)fct) return my_##NAME##_##A##_20;   \
    if(my_##NAME##_##A##_fct_21 == (uintptr_t)fct) return my_##NAME##_##A##_21;   \
    if(my_##NAME##_##A##_fct_22 == (uintptr_t)fct) return my_##NAME##_##A##_22;   \
    if(my_##NAME##_##A##_fct_23 == (uintptr_t)fct) return my_##NAME##_##A##_23;   \
    if(my_##NAME##_##A##_fct_24 == (uintptr_t)fct) return my_##NAME##_##A##_24;   \
    if(my_##NAME##_##A##_fct_25 == (uintptr_t)fct) return my_##NAME##_##A##_25;   \
    if(my_##NAME##_##A##_fct_26 == (uintptr_t)fct) return my_##NAME##_##A##_26;   \
    if(my_##NAME##_##A##_fct_27 == (uintptr_t)fct) return my_##NAME##_##A##_27;   \
    if(my_##NAME##_##A##_fct_28 == (uintptr_t)fct) return my_##NAME##_##A##_28;   \
    if(my_##NAME##_##A##_fct_29 == (uintptr_t)fct) return my_##NAME##_##A##_29;   \
    if(my_##NAME##_##A##_fct_30 == (uintptr_t)fct) return my_##NAME##_##A##_30;   \
    if(my_##NAME##_##A##_fct_31 == (uintptr_t)fct) return my_##NAME##_##A##_31;   \
    if(my_##NAME##_##A##_fct_32 == (uintptr_t)fct) return my_##NAME##_##A##_32;   \
    if(my_##NAME##_##A##_fct_33 == (uintptr_t)fct) return my_##NAME##_##A##_33;   \
    if(my_##NAME##_##A##_fct_34 == (uintptr_t)fct) return my_##NAME##_##A##_34;   \
    if(my_##NAME##_##A##_fct_35 == (uintptr_t)fct) return my_##NAME##_##A##_35;   \
    if(my_##NAME##_##A##_fct_36 == (uintptr_t)fct) return my_##NAME##_##A##_36;   \
    if(my_##NAME##_##A##_fct_37 == (uintptr_t)fct) return my_##NAME##_##A##_37;   \
    if(my_##NAME##_##A##_fct_38 == (uintptr_t)fct) return my_##NAME##_##A##_38;   \
    if(my_##NAME##_##A##_fct_39 == (uintptr_t)fct) return my_##NAME##_##A##_39;   \
    if(my_##NAME##_##A##_fct_0 == 0) {my_##NAME##_##A##_fct_0 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_0, fct); return my_##NAME##_##A##_0; } \
    if(my_##NAME##_##A##_fct_1 == 0) {my_##NAME##_##A##_fct_1 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_1, fct); return my_##NAME##_##A##_1; } \
    if(my_##NAME##_##A##_fct_2 == 0) {my_##NAME##_##A##_fct_2 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_2, fct); return my_##NAME##_##A##_2; } \
    if(my_##NAME##_##A##_fct_3 == 0) {my_##NAME##_##A##_fct_3 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_3, fct); return my_##NAME##_##A##_3; } \
    if(my_##NAME##_##A##_fct_4 == 0) {my_##NAME##_##A##_fct_4 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_4, fct); return my_##NAME##_##A##_4; } \
    if(my_##NAME##_##A##_fct_5 == 0) {my_##NAME##_##A##_fct_5 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_5, fct); return my_##NAME##_##A##_5; } \
    if(my_##NAME##_##A##_fct_6 == 0) {my_##NAME##_##A##_fct_6 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_6, fct); return my_##NAME##_##A##_6; } \
    if(my_##NAME##_##A##_fct_7 == 0) {my_##NAME##_##A##_fct_7 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_7, fct); return my_##NAME##_##A##_7; } \
    if(my_##NAME##_##A##_fct_8 == 0) {my_##NAME##_##A##_fct_8 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_8, fct); return my_##NAME##_##A##_8; } \
    if(my_##NAME##_##A##_fct_9 == 0) {my_##NAME##_##A##_fct_9 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_9, fct); return my_##NAME##_##A##_9; } \
    if(my_##NAME##_##A##_fct_10 == 0) {my_##NAME##_##A##_fct_10 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_10, fct); return my_##NAME##_##A##_10; } \
    if(my_##NAME##_##A##_fct_11 == 0) {my_##NAME##_##A##_fct_11 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_11, fct); return my_##NAME##_##A##_11; } \
    if(my_##NAME##_##A##_fct_12 == 0) {my_##NAME##_##A##_fct_12 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_12, fct); return my_##NAME##_##A##_12; } \
    if(my_##NAME##_##A##_fct_13 == 0) {my_##NAME##_##A##_fct_13 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_13, fct); return my_##NAME##_##A##_13; } \
    if(my_##NAME##_##A##_fct_14 == 0) {my_##NAME##_##A##_fct_14 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_14, fct); return my_##NAME##_##A##_14; } \
    if(my_##NAME##_##A##_fct_15 == 0) {my_##NAME##_##A##_fct_15 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_15, fct); return my_##NAME##_##A##_15; } \
    if(my_##NAME##_##A##_fct_16 == 0) {my_##NAME##_##A##_fct_16 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_16, fct); return my_##NAME##_##A##_16; } \
    if(my_##NAME##_##A##_fct_17 == 0) {my_##NAME##_##A##_fct_17 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_17, fct); return my_##NAME##_##A##_17; } \
    if(my_##NAME##_##A##_fct_18 == 0) {my_##NAME##_##A##_fct_18 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_18, fct); return my_##NAME##_##A##_18; } \
    if(my_##NAME##_##A##_fct_19 == 0) {my_##NAME##_##A##_fct_19 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_19, fct); return my_##NAME##_##A##_19; } \
    if(my_##NAME##_##A##_fct_20 == 0) {my_##NAME##_##A##_fct_20 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_20, fct); return my_##NAME##_##A##_20; } \
    if(my_##NAME##_##A##_fct_21 == 0) {my_##NAME##_##A##_fct_21 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_21, fct); return my_##NAME##_##A##_21; } \
    if(my_##NAME##_##A##_fct_22 == 0) {my_##NAME##_##A##_fct_22 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_22, fct); return my_##NAME##_##A##_22; } \
    if(my_##NAME##_##A##_fct_23 == 0) {my_##NAME##_##A##_fct_23 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_23, fct); return my_##NAME##_##A##_23; } \
    if(my_##NAME##_##A##_fct_24 == 0) {my_##NAME##_##A##_fct_24 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_24, fct); return my_##NAME##_##A##_24; } \
    if(my_##NAME##_##A##_fct_25 == 0) {my_##NAME##_##A##_fct_25 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_25, fct); return my_##NAME##_##A##_25; } \
    if(my_##NAME##_##A##_fct_26 == 0) {my_##NAME##_##A##_fct_26 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_26, fct); return my_##NAME##_##A##_26; } \
    if(my_##NAME##_##A##_fct_27 == 0) {my_##NAME##_##A##_fct_27 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_27, fct); return my_##NAME##_##A##_27; } \
    if(my_##NAME##_##A##_fct_28 == 0) {my_##NAME##_##A##_fct_28 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_28, fct); return my_##NAME##_##A##_28; } \
    if(my_##NAME##_##A##_fct_29 == 0) {my_##NAME##_##A##_fct_29 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_29, fct); return my_##NAME##_##A##_29; } \
    if(my_##NAME##_##A##_fct_30 == 0) {my_##NAME##_##A##_fct_30 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_30, fct); return my_##NAME##_##A##_30; } \
    if(my_##NAME##_##A##_fct_31 == 0) {my_##NAME##_##A##_fct_31 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_31, fct); return my_##NAME##_##A##_31; } \
    if(my_##NAME##_##A##_fct_32 == 0) {my_##NAME##_##A##_fct_32 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_32, fct); return my_##NAME##_##A##_32; } \
    if(my_##NAME##_##A##_fct_33 == 0) {my_##NAME##_##A##_fct_33 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_33, fct); return my_##NAME##_##A##_33; } \
    if(my_##NAME##_##A##_fct_34 == 0) {my_##NAME##_##A##_fct_34 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_34, fct); return my_##NAME##_##A##_34; } \
    if(my_##NAME##_##A##_fct_35 == 0) {my_##NAME##_##A##_fct_35 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_35, fct); return my_##NAME##_##A##_35; } \
    if(my_##NAME##_##A##_fct_36 == 0) {my_##NAME##_##A##_fct_36 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_36, fct); return my_##NAME##_##A##_36; } \
    if(my_##NAME##_##A##_fct_37 == 0) {my_##NAME##_##A##_fct_37 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_37, fct); return my_##NAME##_##A##_37; } \
    if(my_##NAME##_##A##_fct_38 == 0) {my_##NAME##_##A##_fct_38 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_38, fct); return my_##NAME##_##A##_38; } \
    if(my_##NAME##_##A##_fct_39 == 0) {my_##NAME##_##A##_fct_39 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_39, fct); return my_##NAME##_##A##_39; } \
    printf_log(LOG_NONE, "Warning, no more slot for " #A " " #NAME " gtkclass callback\n");    \
    return NULL;    \
}

#define REVERSE(A, NAME)   \
static void* reverse_##NAME##_##A(wrapper_t W, void* fct)                       \
{                                                                               \
    if(!fct) return fct;                                                        \
    if((void*)my_##NAME##_##A##_0 == fct) return (void*)my_##NAME##_##A##_fct_0;\
    if((void*)my_##NAME##_##A##_1 == fct) return (void*)my_##NAME##_##A##_fct_1;\
    if((void*)my_##NAME##_##A##_2 == fct) return (void*)my_##NAME##_##A##_fct_2;\
    if((void*)my_##NAME##_##A##_3 == fct) return (void*)my_##NAME##_##A##_fct_3;\
    if((void*)my_##NAME##_##A##_4 == fct) return (void*)my_##NAME##_##A##_fct_4;\
    if((void*)my_##NAME##_##A##_5 == fct) return (void*)my_##NAME##_##A##_fct_5;\
    if((void*)my_##NAME##_##A##_6 == fct) return (void*)my_##NAME##_##A##_fct_6;\
    if((void*)my_##NAME##_##A##_7 == fct) return (void*)my_##NAME##_##A##_fct_7;\
    if((void*)my_##NAME##_##A##_8 == fct) return (void*)my_##NAME##_##A##_fct_8;\
    if((void*)my_##NAME##_##A##_9 == fct) return (void*)my_##NAME##_##A##_fct_9;\
    if((void*)my_##NAME##_##A##_10 == fct) return (void*)my_##NAME##_##A##_fct_10;\
    if((void*)my_##NAME##_##A##_11 == fct) return (void*)my_##NAME##_##A##_fct_11;\
    if((void*)my_##NAME##_##A##_12 == fct) return (void*)my_##NAME##_##A##_fct_12;\
    if((void*)my_##NAME##_##A##_13 == fct) return (void*)my_##NAME##_##A##_fct_13;\
    if((void*)my_##NAME##_##A##_14 == fct) return (void*)my_##NAME##_##A##_fct_14;\
    if((void*)my_##NAME##_##A##_15 == fct) return (void*)my_##NAME##_##A##_fct_15;\
    if((void*)my_##NAME##_##A##_16 == fct) return (void*)my_##NAME##_##A##_fct_16;\
    if((void*)my_##NAME##_##A##_17 == fct) return (void*)my_##NAME##_##A##_fct_17;\
    if((void*)my_##NAME##_##A##_18 == fct) return (void*)my_##NAME##_##A##_fct_18;\
    if((void*)my_##NAME##_##A##_19 == fct) return (void*)my_##NAME##_##A##_fct_19;\
    if((void*)my_##NAME##_##A##_20 == fct) return (void*)my_##NAME##_##A##_fct_20;\
    if((void*)my_##NAME##_##A##_21 == fct) return (void*)my_##NAME##_##A##_fct_21;\
    if((void*)my_##NAME##_##A##_22 == fct) return (void*)my_##NAME##_##A##_fct_22;\
    if((void*)my_##NAME##_##A##_23 == fct) return (void*)my_##NAME##_##A##_fct_23;\
    if((void*)my_##NAME##_##A##_24 == fct) return (void*)my_##NAME##_##A##_fct_24;\
    if((void*)my_##NAME##_##A##_25 == fct) return (void*)my_##NAME##_##A##_fct_25;\
    if((void*)my_##NAME##_##A##_26 == fct) return (void*)my_##NAME##_##A##_fct_26;\
    if((void*)my_##NAME##_##A##_27 == fct) return (void*)my_##NAME##_##A##_fct_27;\
    if((void*)my_##NAME##_##A##_28 == fct) return (void*)my_##NAME##_##A##_fct_28;\
    if((void*)my_##NAME##_##A##_29 == fct) return (void*)my_##NAME##_##A##_fct_29;\
    if((void*)my_##NAME##_##A##_30 == fct) return (void*)my_##NAME##_##A##_fct_30;\
    if((void*)my_##NAME##_##A##_31 == fct) return (void*)my_##NAME##_##A##_fct_31;\
    if((void*)my_##NAME##_##A##_32 == fct) return (void*)my_##NAME##_##A##_fct_32;\
    if((void*)my_##NAME##_##A##_33 == fct) return (void*)my_##NAME##_##A##_fct_33;\
    if((void*)my_##NAME##_##A##_34 == fct) return (void*)my_##NAME##_##A##_fct_34;\
    if((void*)my_##NAME##_##A##_35 == fct) return (void*)my_##NAME##_##A##_fct_35;\
    if((void*)my_##NAME##_##A##_36 == fct) return (void*)my_##NAME##_##A##_fct_36;\
    if((void*)my_##NAME##_##A##_37 == fct) return (void*)my_##NAME##_##A##_fct_37;\
    if((void*)my_##NAME##_##A##_38 == fct) return (void*)my_##NAME##_##A##_fct_38;\
    if((void*)my_##NAME##_##A##_39 == fct) return (void*)my_##NAME##_##A##_fct_39;\
    Dl_info info;                                                               \
    if(dladdr(fct, &info))                                                      \
        return (void*)AddCheckBridge(my_bridge, W, fct, 0, NULL);               \
    return fct;                                                                 \
}

#define AUTOBRIDGE(A, NAME)   \
static void autobridge_##NAME##_##A(wrapper_t W, void* fct)         \
{                                                                   \
    if(!fct)                                                        \
        return;                                                     \
    Dl_info info;                                                   \
    if(dladdr(fct, &info))                                          \
        AddAutomaticBridge(my_bridge, W, fct, 0, #NAME "_" #A); \
}

#define WRAPPER(A, NAME, RET, DEF, FMT, ...)        \
WRAPPED( 0, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED( 1, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED( 2, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED( 3, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED( 4, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED( 5, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED( 6, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED( 7, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED( 8, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED( 9, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(10, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(11, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(12, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(13, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(14, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(15, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(16, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(17, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(18, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(19, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(20, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(21, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(22, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(23, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(24, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(25, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(26, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(27, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(28, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(29, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(30, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(31, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(32, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(33, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(34, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(35, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(36, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(37, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(38, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(39, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
FIND(A, NAME)                                       \
REVERSE(A, NAME)                                    \
AUTOBRIDGE(A, NAME)

#define WRAPPER_RET(A, NAME, RET, FRET, DEF, FMT, ...)        \
WRAPPED_RET( 0, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET( 1, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET( 2, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET( 3, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET( 4, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET( 5, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET( 6, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET( 7, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET( 8, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET( 9, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(10, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(11, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(12, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(13, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(14, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(15, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(16, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(17, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(18, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(19, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(20, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(21, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(22, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(23, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(24, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(25, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(26, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(27, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(28, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(29, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(30, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(31, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(32, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(33, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(34, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(35, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(36, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(37, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(38, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(39, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
FIND(A, NAME)                                       \
REVERSE(A, NAME)                                    \
AUTOBRIDGE(A, NAME)


// ---- Class callback implementations (split for readability) ----
// These files are #include'd to maintain a single translation unit.
// All static functions, macros, and variables remain in the same scope.
#include "gtk/callbacks_gobject.h"
#include "gtk/callbacks_gtk_widgets.h"
#include "gtk/callbacks_gtk_widgets2.h"
#include "gtk/callbacks_gtk_gestures.h"
#include "gtk/callbacks_atk.h"
#include "gtk/callbacks_gstreamer.h"

// No more wrap/unwrap
#undef WRAPPER
#undef FIND
#undef REVERSE
#undef WRAPPED


// ---- Dispatch, GTypeValueTable, signals, GTypeInfo, init/fini ----
#include "gtk/dispatch.h"
