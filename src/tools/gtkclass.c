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

// ----- GObjectClass ------
// wrapper x64 -> natives of callbacks
WRAPPER(GObject, constructor, void*, (size_t type, uint32_t n_construct_properties, void* construct_properties), "Lup", type, n_construct_properties, construct_properties);
WRAPPER(GObject, set_property, void, (void* object, uint32_t property_id, void* value, void* pspec), "pupp", object, property_id, value, pspec);
WRAPPER(GObject, get_property, void, (void* object, uint32_t property_id, void* value, void* pspec), "pupp", object, property_id, value, pspec);
WRAPPER(GObject, dispose, void, (void* object), "p", object);
WRAPPER(GObject, finalize, void, (void* object), "p", object);
WRAPPER(GObject, dispatch_properties_changed, void*, (size_t type, uint32_t n_pspecs, void* pspecs), "Lup", type, n_pspecs, pspecs);
WRAPPER(GObject, notify, void*, (size_t type, void* pspecs), "Lp", type, pspecs);
WRAPPER(GObject, constructed, void, (void* object), "p", object);

#define SUPERGO() \
    GO(constructor, pFLup);                 \
    GO(set_property, vFpupp);               \
    GO(get_property, vFpupp);               \
    GO(dispose, vFp);                       \
    GO(finalize, vFp);                      \
    GO(dispatch_properties_changed, vFLup); \
    GO(notify, vFpp);                       \
    GO(constructed, vFp);

// wrap (so bridge all calls, just in case)
static void wrapGObjectClass(my_GObjectClass_t* class)
{
    #define GO(A, W) class->A = reverse_##A##_GObject (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGObjectClass(my_GObjectClass_t* class)
{
    #define GO(A, W)   class->A = find_##A##_GObject (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGObjectClass(my_GObjectClass_t* class)
{
    #define GO(A, W) autobridge_##A##_GObject (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

// unwrap (and use callback if not a native call anymore)
static void unwrapGObjectInstance(my_GObject_t* class)
{
}
// autobridge
static void bridgeGObjectInstance(my_GObject_t* class)
{
}
#undef SUPERGO
// ----- GInitiallyUnownedClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GInitiallyUnowned, constructor, void*, (size_t type, uint32_t n_construct_properties, void* construct_properties), "Lup", type, n_construct_properties, construct_properties);
WRAPPER(GInitiallyUnowned, set_property, void, (void* object, uint32_t property_id, void* value, void* pspec), "pupp", object, property_id, value, pspec);
WRAPPER(GInitiallyUnowned, get_property, void, (void* object, uint32_t property_id, void* value, void* pspec), "pupp", object, property_id, value, pspec);
WRAPPER(GInitiallyUnowned, dispose, void, (void* object), "p", object);
WRAPPER(GInitiallyUnowned, finalize, void, (void* object), "p", object);
WRAPPER(GInitiallyUnowned, dispatch_properties_changed, void*, (size_t type, uint32_t n_pspecs, void* pspecs), "Lup", type, n_pspecs, pspecs);
WRAPPER(GInitiallyUnowned, notify, void*, (size_t type, void* pspecs), "Lp", type, pspecs);
WRAPPER(GInitiallyUnowned, constructed, void, (void* object), "p", object);

#define SUPERGO() \
    GO(constructor, pFLup);                 \
    GO(set_property, vFpupp);               \
    GO(get_property, vFpupp);               \
    GO(dispose, vFp);                       \
    GO(finalize, vFp);                      \
    GO(dispatch_properties_changed, vFLup); \
    GO(notify, vFLp);                       \
    GO(constructed, vFp);

// wrap (so bridge all calls, just in case)
static void wrapGInitiallyUnownedClass(my_GInitiallyUnownedClass_t* class)
{
    #define GO(A, W) class->A = reverse_##A##_GInitiallyUnowned (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGInitiallyUnownedClass(my_GInitiallyUnownedClass_t* class)
{
    #define GO(A, W)   class->A = find_##A##_GInitiallyUnowned (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGInitiallyUnownedClass(my_GInitiallyUnownedClass_t* class)
{
    #define GO(A, W) autobridge_##A##_GInitiallyUnowned (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGInitiallyUnownedInstance(my_GInitiallyUnowned_t* class)
{
}
// autobridge
static void bridgeGInitiallyUnownedInstance(my_GInitiallyUnowned_t* class)
{
}

// ----- GApplicationClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GApplication, startup, void, (void* application), "p", application);
WRAPPER(GApplication, activate, void, (void* application), "p", application);
WRAPPER(GApplication, open, void, (void* application, void* files, int n_files, void* hint), "ppip", application, files, n_files, hint);
WRAPPER(GApplication, command_line, void, (void* application, void* command_line), "pp", application, command_line);
WRAPPER(GApplication, local_command_line, void, (void* application, void* arguments, void* exit_status), "ppp", application, arguments, exit_status);
WRAPPER(GApplication, before_emit, void*, (void* application, void* platform_data), "pp", application, platform_data);
WRAPPER(GApplication, after_emit, void, (void* application, void* platform_data), "pp", application, platform_data);
WRAPPER(GApplication, add_platform_data, void, (void* application, void* builder), "pp", application, builder);
WRAPPER(GApplication, quit_mainloop, void, (void* application), "p", application);
WRAPPER(GApplication, run_mainloop, void, (void* application), "p", application);
WRAPPER(GApplication, shutdown, void, (void* application), "p", application);
WRAPPER(GApplication, dbus_register, void, (void* application, void* connection, void* object_path, void* error), "pppp", application, connection, object_path, error);
WRAPPER(GApplication, dbus_unregister, void, (void* application, void* connection, void* object_path), "ppp", application, connection, object_path);
WRAPPER(GApplication, handle_local_options, void, (void* application, void* options), "pp", application, options);
WRAPPER(GApplication, name_lost, void, (void* application), "p", application);

#define SUPERGO()                     \
    GO(startup, vFp);                 \
    GO(activate, vFp);                \
    GO(open, vFppip);                 \
    GO(command_line, vFpp);           \
    GO(local_command_line, vFppp);    \
    GO(before_emit, vFpp);            \
    GO(after_emit, vFpp);             \
    GO(add_platform_data, vFpp);      \
    GO(quit_mainloop, vFp);           \
    GO(run_mainloop, vFp);            \
    GO(shutdown, vFp);                \
    GO(dbus_register, vFpppp);        \
    GO(dbus_unregister, vFppp);       \
    GO(handle_local_options, vFpp);   \
    GO(name_lost, vFp);

// wrap (so bridge all calls, just in case)
static void wrapGApplicationClass(my_GApplicationClass_t* class)
{
    wrapGObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GApplication (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGApplicationClass(my_GApplicationClass_t* class)
{
    unwrapGObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GApplication (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGApplicationClass(my_GApplicationClass_t* class)
{
    bridgeGObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GApplication (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGApplicationInstance(my_GApplication_t* class)
{
    unwrapGObjectInstance(&class->parent);
}
// autobridge
static void bridgeGApplicationInstance(my_GApplication_t* class)
{
    bridgeGObjectInstance(&class->parent);
}
#undef SUPERGO

// ----- GtkApplicationClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkApplication, window_added, void, (void* application, void* window), "pp", application, window);
WRAPPER(GtkApplication, window_removed, void, (void* application, void* window), "pp", application, window);

#define SUPERGO() \
    GO(window_added, pFpp);   \
    GO(window_removed, vFpp);

static void wrapGtkApplicationClass(my_GtkApplicationClass_t* class)
{
    wrapGApplicationClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkApplication (W, class->A)
    SUPERGO()
    #undef GO
}

static void unwrapGtkApplicationClass(my_GtkApplicationClass_t* class)
{
    unwrapGApplicationClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkApplication (W, class->A)
    SUPERGO()
    #undef GO
}

static void bridgeGtkApplicationClass(my_GtkApplicationClass_t* class)
{
    bridgeGApplicationClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkApplication (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkApplicationInstance(my_GtkApplication_t* class)
{
    unwrapGApplicationInstance(&class->parent);
}

static void bridgeGtkApplicationInstance(my_GtkApplication_t* class)
{
    bridgeGApplicationInstance(&class->parent);
}

// ----- GtkObjectClass ------
// wrapper x64 -> natives of callbacks
WRAPPER(GtkObject, set_arg, void, (void* object, void* arg, uint32_t arg_id), "ppu", object, arg, arg_id);
WRAPPER(GtkObject, get_arg, void, (void* object, void* arg, uint32_t arg_id), "ppu", object, arg, arg_id);
WRAPPER(GtkObject, destroy, void, (void* object), "p", object);

#define SUPERGO() \
    GO(set_arg, vFppu); \
    GO(get_arg, vFppu); \
    GO(destroy, vFp);
// wrap (so bridge all calls, just in case)
static void wrapGtkObjectClass(my_GtkObjectClass_t* class)
{
    wrapGInitiallyUnownedClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkObject (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkObjectClass(my_GtkObjectClass_t* class)
{
    unwrapGInitiallyUnownedClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkObject (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkObjectClass(my_GtkObjectClass_t* class)
{
    bridgeGInitiallyUnownedClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkObject (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkObjectInstance(my_GtkObject_t* class)
{
    unwrapGInitiallyUnownedInstance(&class->parent);
}
// autobridge
static void bridgeGtkObjectInstance(my_GtkObject_t* class)
{
    bridgeGInitiallyUnownedInstance(&class->parent);
}

// ----- GtkWidget2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkWidget2, dispatch_child_properties_changed, void, (void* widget, uint32_t n_pspecs, void* pspecs), "pup", widget, n_pspecs, pspecs);
WRAPPER(GtkWidget2, show,              void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, show_all,          void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, hide,              void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, hide_all,          void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, map,               void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, unmap,             void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, realize,           void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, unrealize,         void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, size_request,      void, (void* widget, void* requisition), "pp", widget, requisition);
WRAPPER(GtkWidget2, size_allocate,     void, (void* widget, void* allocation), "pp", widget, allocation);
WRAPPER(GtkWidget2, state_changed,     void, (void* widget, int previous_state), "pi", widget, previous_state);
WRAPPER(GtkWidget2, parent_set,        void, (void* widget, void* previous_parent), "pp", widget, previous_parent);
WRAPPER(GtkWidget2, hierarchy_changed, void, (void* widget, void* previous_toplevel), "pp", widget, previous_toplevel);
WRAPPER(GtkWidget2, style_set,         void, (void* widget, void* previous_style), "pp", widget, previous_style);
WRAPPER(GtkWidget2, direction_changed, void, (void* widget, int previous_direction), "pi", widget, previous_direction);
WRAPPER(GtkWidget2, grab_notify,       void, (void* widget, int was_grabbed), "pi", widget, was_grabbed);
WRAPPER(GtkWidget2, child_notify,      void, (void* widget, void* pspec), "pp", widget, pspec);
WRAPPER(GtkWidget2, mnemonic_activate, int, (void* widget, int group_cycling), "pi", widget, group_cycling);
WRAPPER(GtkWidget2, grab_focus,        void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, focus,             int, (void* widget, int direction), "pi", widget, direction);
WRAPPER(GtkWidget2, event,             int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, button_press_event,int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, button_release_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, scroll_event,      int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, motion_notify_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, delete_event,       int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, destroy_event,      int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, expose_event,       int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, key_press_event,    int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, key_release_event,  int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, enter_notify_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, leave_notify_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, configure_event,    int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, focus_in_event,     int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, focus_out_event,    int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, map_event,          int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, unmap_event,        int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, property_notify_event,  int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, selection_clear_event,  int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, selection_request_event,int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, selection_notify_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, proximity_in_event,  int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, proximity_out_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, visibility_notify_event, int, (void* widget, void* event), "p", widget, event);
WRAPPER(GtkWidget2, client_event,        int, (void* widget, void* event), "p", widget, event);
WRAPPER(GtkWidget2, no_expose_event,     int, (void* widget, void* event), "p", widget, event);
WRAPPER(GtkWidget2, window_state_event,  int, (void* widget, void* event), "p", widget, event);
WRAPPER(GtkWidget2, selection_get,       void, (void* widget, void* selection_data, uint32_t info, uint32_t time_), "ppuu", widget, selection_data, info, time_);
WRAPPER(GtkWidget2, selection_received,  void, (void* widget, void* selection_data, uint32_t time_), "ppu", widget, selection_data, time_);
WRAPPER(GtkWidget2, drag_begin,          void, (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget2, drag_end,            void, (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget2, drag_data_get,       void, (void* widget, void* context, void* selection_data, uint32_t info, uint32_t time_), "pppuu", widget, context, selection_data, info, time_);
WRAPPER(GtkWidget2, drag_data_delete,    void, (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget2, drag_leave,          void, (void* widget, void* context, uint32_t time_), "ppu", widget, context, time_);
WRAPPER(GtkWidget2, drag_motion,         int, (void* widget, void* context, int32_t x, int32_t y, uint32_t time_), "ppiiu", widget, context, x, y, time_);
WRAPPER(GtkWidget2, drag_drop,           int, (void* widget, void* context, int32_t x, int32_t y, uint32_t time_), "ppiiu", widget, context, x, y, time_);
WRAPPER(GtkWidget2, drag_data_received,  void, (void* widget, void* context, int32_t x, int32_t y, void* selection_data, uint32_t info, uint32_t time_), "ppiipuu", widget, context, x, y, selection_data, info, time_);
WRAPPER(GtkWidget2,  popup_menu,         int  , (void* widget), "p", widget);
WRAPPER(GtkWidget2,  show_help,          int  , (void* widget, int help_type), "pi", widget, help_type);
WRAPPER(GtkWidget2, get_accessible,      void*, (void* widget), "p", widget);
WRAPPER(GtkWidget2, screen_changed,      void , (void* widget, void* previous_screen), "pp", widget, previous_screen);
WRAPPER(GtkWidget2, can_activate_accel,  int  , (void* widget, uint32_t signal_id), "pu", widget, signal_id);
WRAPPER(GtkWidget2, grab_broken_event,   int  , (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2,  composited_changed, void , (void* widget), "p", widget);
WRAPPER(GtkWidget2,  query_tooltip,      int  , (void* widget, int32_t x, int32_t y, int keyboard_tooltip, void* tooltip), "piiip", widget, x, y, keyboard_tooltip, tooltip);

#define SUPERGO() \
    GO(dispatch_child_properties_changed, vFpup);   \
    GO(show, vFp);                                  \
    GO(show_all, vFp);                              \
    GO(hide, vFp);                                  \
    GO(hide_all, vFp);                              \
    GO(map, vFp);                                   \
    GO(unmap, vFp);                                 \
    GO(realize, vFp);                               \
    GO(unrealize, vFp);                             \
    GO(size_request, vFpp);                         \
    GO(size_allocate, vFpp);                        \
    GO(state_changed, vFpi);                        \
    GO(parent_set, vFpp);                           \
    GO(hierarchy_changed, vFpp);                    \
    GO(style_set, vFpp);                            \
    GO(direction_changed, vFpi);                    \
    GO(grab_notify, vFpi);                          \
    GO(child_notify, vFpp);                         \
    GO(mnemonic_activate, iFpi);                    \
    GO(grab_focus, vFp);                            \
    GO(focus, iFpi);                                \
    GO(event, iFpp);                                \
    GO(button_press_event, iFpp);                   \
    GO(button_release_event, iFpp);                 \
    GO(scroll_event, iFpp);                         \
    GO(motion_notify_event, iFpp);                  \
    GO(delete_event, iFpp);                         \
    GO(destroy_event, iFpp);                        \
    GO(expose_event, iFpp);                         \
    GO(key_press_event, iFpp);                      \
    GO(key_release_event, iFpp);                    \
    GO(enter_notify_event, iFpp);                   \
    GO(leave_notify_event, iFpp);                   \
    GO(configure_event, iFpp);                      \
    GO(focus_in_event, iFpp);                       \
    GO(focus_out_event, iFpp);                      \
    GO(map_event, iFpp);                            \
    GO(unmap_event, iFpp);                          \
    GO(property_notify_event, iFpp);                \
    GO(selection_clear_event, iFpp);                \
    GO(selection_request_event, iFpp);              \
    GO(selection_notify_event, iFpp);               \
    GO(proximity_in_event, iFpp);                   \
    GO(proximity_out_event, iFpp);                  \
    GO(visibility_notify_event, iFpp);              \
    GO(client_event, iFpp);                         \
    GO(no_expose_event, iFpp);                      \
    GO(window_state_event, iFpp);                   \
    GO(selection_get, vFppuu);                      \
    GO(selection_received, vFppu);                  \
    GO(drag_begin, vFpp);                           \
    GO(drag_end, vFpp);                             \
    GO(drag_data_get, vFpppuu);                     \
    GO(drag_data_delete, vFpp);                     \
    GO(drag_leave, vFppu);                          \
    GO(drag_motion, iFppiiu);                       \
    GO(drag_drop, iFppiiu);                         \
    GO(drag_data_received, vFppiipuu);              \
    GO(popup_menu, iFp);                            \
    GO(show_help, iFpi);                            \
    GO(get_accessible, pFp);                        \
    GO(screen_changed, vFpp);                       \
    GO(can_activate_accel, iFpu);                   \
    GO(grab_broken_event, iFpp);                    \
    GO(composited_changed, vFp);                    \
    GO(query_tooltip, iFpiiip);

// wrap (so bridge all calls, just in case)
static void wrapGtkWidget2Class(my_GtkWidget2Class_t* class)
{
    wrapGtkObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkWidget2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkWidget2Class(my_GtkWidget2Class_t* class)
{
    unwrapGtkObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkWidget2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkWidget2Class(my_GtkWidget2Class_t* class)
{
    bridgeGtkObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkWidget2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkWidget2Instance(my_GtkWidget2_t* class)
{
    unwrapGtkObjectInstance(&class->private);
}
// autobridge
static void bridgeGtkWidget2Instance(my_GtkWidget2_t* class)
{
    bridgeGtkObjectInstance(&class->private);
}

// ----- GtkWidget3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkWidget3, dispatch_child_properties_changed, void, (void* widget, uint32_t n_pspecs, void* pspecs), "pup", widget, n_pspecs, pspecs);
WRAPPER(GtkWidget3, destroy, void,              (void* widget), "p", widget);
WRAPPER(GtkWidget3, show, void,                 (void* widget), "p", widget);
WRAPPER(GtkWidget3, show_all, void,             (void* widget), "p", widget);
WRAPPER(GtkWidget3, hide, void,                 (void* widget), "p", widget);
WRAPPER(GtkWidget3, map, void,                  (void* widget), "p", widget);
WRAPPER(GtkWidget3, unmap, void,                (void* widget), "p", widget);
WRAPPER(GtkWidget3, realize, void,              (void* widget), "p", widget);
WRAPPER(GtkWidget3, unrealize, void,            (void* widget), "p", widget);
WRAPPER(GtkWidget3, size_allocate, void,        (void* widget, void* allocation), "pp", widget, allocation);
WRAPPER(GtkWidget3, state_changed, void,        (void* widget, int previous_state), "pi", widget, previous_state);
WRAPPER(GtkWidget3, state_flags_changed, void,  (void* widget, int previous_state_flags), "pi", widget, previous_state_flags);
WRAPPER(GtkWidget3, parent_set, void,           (void* widget, void* previous_parent), "pp", widget, previous_parent);
WRAPPER(GtkWidget3, hierarchy_changed, void,    (void* widget, void* previous_toplevel), "pp", widget, previous_toplevel);
WRAPPER(GtkWidget3, style_set, void,            (void* widget, void* previous_style), "pp", widget, previous_style);
WRAPPER(GtkWidget3, direction_changed, void,    (void* widget, int previous_direction), "pi", widget, previous_direction);
WRAPPER(GtkWidget3, grab_notify, void,          (void* widget, int was_grabbed), "pi", widget, was_grabbed);
WRAPPER(GtkWidget3, child_notify, void,         (void* widget, void* child_property), "pp", widget, child_property);
WRAPPER(GtkWidget3, draw, int,                  (void* widget, void* cr), "pp", widget, cr);
WRAPPER(GtkWidget3, get_request_mode, int,      (void* widget), "p", widget);
WRAPPER(GtkWidget3, get_preferred_height, void, (void* widget, void* minimum_height, void* natural_height), "ppp", widget, minimum_height, natural_height);
WRAPPER(GtkWidget3, get_preferred_width_for_height, void,  (void* widget, int height, void* minimum_width, void* natural_width), "pipp", widget, height, minimum_width, natural_width);
WRAPPER(GtkWidget3, get_preferred_width, void,  (void* widget, void* minimum_width, void* natural_width), "ppp", widget, minimum_width, natural_width);
WRAPPER(GtkWidget3, get_preferred_height_for_width, void,  (void* widget, int width, void* minimum_height, void* natural_height), "pipp", widget, width, minimum_height, natural_height);
WRAPPER(GtkWidget3, mnemonic_activate, int,     (void* widget, int group_cycling), "pi", widget, group_cycling);
WRAPPER(GtkWidget3, grab_focus, void,           (void* widget), "p", widget);
WRAPPER(GtkWidget3, focus, int,                 (void* widget, int direction), "pi", widget, direction);
WRAPPER(GtkWidget3, move_focus, void,           (void* widget, int direction), "pi", widget, direction);
WRAPPER(GtkWidget3, keynav_failed, int,         (void* widget, int direction), "pi", widget, direction);
WRAPPER(GtkWidget3, event, int,                 (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, button_press_event, int,    (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, button_release_event, int,  (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, scroll_event, int,          (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, motion_notify_event, int,   (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, delete_event, int,          (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, destroy_event, int,         (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, key_press_event, int,       (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, key_release_event, int,     (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, enter_notify_event, int,    (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, leave_notify_event, int,    (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, configure_event, int,       (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, focus_in_event, int,        (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, focus_out_event, int,       (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, map_event, int,             (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, unmap_event, int,           (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, property_notify_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, selection_clear_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, selection_request_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, selection_notify_event, int,(void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, proximity_in_event, int,    (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, proximity_out_event, int,   (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, visibility_notify_event, int,   (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, window_state_event, int,    (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, damage_event, int,          (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, grab_broken_event, int,     (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, selection_get, void,        (void* widget, void* selection_data, uint32_t info, uint32_t time_), "ppuu", widget, selection_data, info, time_);
WRAPPER(GtkWidget3, selection_received, void,   (void* widget, void* selection_data, uint32_t time_), "ppu", widget, selection_data, time_);
WRAPPER(GtkWidget3, drag_begin, void,           (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget3, drag_end, void,             (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget3, drag_data_get, void,        (void* widget, void* context, void* selection_data, uint32_t info, uint32_t time_), "pppuu", widget, context, selection_data, info, time_);
WRAPPER(GtkWidget3, drag_data_delete, void,     (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget3, drag_leave, void,           (void* widget, void* context, uint32_t time_), "ppu", widget, context, time_);
WRAPPER(GtkWidget3, drag_motion, int,           (void* widget, void* context, int x, int y, uint32_t time_), "ppiiu", widget, context, x, y, time_);
WRAPPER(GtkWidget3, drag_drop, int,             (void* widget, void* context, int x, int y, uint32_t time_), "ppiiu", widget, context, x, y, time_);
WRAPPER(GtkWidget3, drag_data_received, void,   (void* widget, void* context, int x, int y, void* selection_data, uint32_t info, uint32_t time_), "ppiipuu", widget, context, x, y, selection_data, info, time_);
WRAPPER(GtkWidget3, drag_failed, int,           (void* widget, void* context, int result), "ppi", widget, context, result);
WRAPPER(GtkWidget3, popup_menu, int,            (void* widget), "p", widget);
WRAPPER(GtkWidget3, show_help, int,             (void* widget, int help_type), "pi", widget, help_type);
WRAPPER(GtkWidget3, get_accessible, void*,      (void *widget), "p", widget);
WRAPPER(GtkWidget3, screen_changed, void,       (void* widget, void* previous_screen), "pp", widget, previous_screen);
WRAPPER(GtkWidget3, can_activate_accel, int,    (void* widget, uint32_t signal_id), "pu", widget, signal_id);
WRAPPER(GtkWidget3, composited_changed, void,   (void* widget), "p", widget);
WRAPPER(GtkWidget3, query_tooltip, int,         (void* widget, int x, int y, int keyboard_tooltip, void* tooltip), "piiip", widget, x, y, keyboard_tooltip, tooltip);
WRAPPER(GtkWidget3, compute_expand, void,       (void* widget, void* hexpand_p, void* vexpand_p), "ppp", widget, hexpand_p, vexpand_p);
WRAPPER(GtkWidget3, adjust_size_request, void,  (void* widget, int orientation, void* minimum_size, void* natural_size), "pipp", widget, orientation, minimum_size, natural_size);
WRAPPER(GtkWidget3, adjust_size_allocation, void, (void*widget, int orientation, void* minimum_size, void* natural_size, void* allocated_pos, void* allocated_size), "pipppp", widget, orientation, minimum_size, natural_size, allocated_pos, allocated_size);
WRAPPER(GtkWidget3, style_updated, void,        (void* widget), "p", widget);
WRAPPER(GtkWidget3, touch_event, int,           (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, get_preferred_height_and_baseline_for_width, void, (void* widget, int width, void* minimum_height, void* natural_height, void* minimum_baseline, void* natural_baseline), "pipppp", widget, width, minimum_height, natural_height, minimum_baseline, natural_baseline);
WRAPPER(GtkWidget3, adjust_baseline_request, void,  (void* widget, void* minimum_baseline, void* natural_baseline), "ppp", widget, minimum_baseline, natural_baseline);
WRAPPER(GtkWidget3, adjust_baseline_allocation, void,  (void* widget, void* baseline), "pp", widget, baseline);
WRAPPER(GtkWidget3, queue_draw_region, void,    (void* widget, void* region), "pp", widget, region);

#define SUPERGO() \
    GO(dispatch_child_properties_changed, vFpup);   \
    GO(destroy, vFp);                               \
    GO(show, vFp);                                  \
    GO(show_all, vFp);                              \
    GO(hide, vFp);                                  \
    GO(map, vFp);                                   \
    GO(unmap, vFp);                                 \
    GO(realize, vFp);                               \
    GO(unrealize, vFp);                             \
    GO(size_allocate, vFpp);                        \
    GO(state_changed, vFpi);                        \
    GO(state_flags_changed, vFpi);                  \
    GO(parent_set, vFpp);                           \
    GO(hierarchy_changed, vFpp);                    \
    GO(style_set, vFpp);                            \
    GO(direction_changed, vFpi);                    \
    GO(grab_notify, vFpi);                          \
    GO(child_notify, vFpp);                         \
    GO(draw, iFpp);                                 \
    GO(get_request_mode, iFp);                      \
    GO(get_preferred_height, vFppp);                \
    GO(get_preferred_width_for_height, vFpipp);     \
    GO(get_preferred_width, vFppp);                 \
    GO(get_preferred_height_for_width, vFpipp);     \
    GO(mnemonic_activate, iFpi);                    \
    GO(grab_focus, vFp);                            \
    GO(focus, iFpi);                                \
    GO(move_focus, vFpi);                           \
    GO(keynav_failed, iFpi);                        \
    GO(event, iFpp);                                \
    GO(button_press_event, iFpp);                   \
    GO(button_release_event, iFpp);                 \
    GO(scroll_event, iFpp);                         \
    GO(motion_notify_event, iFpp);                  \
    GO(delete_event, iFpp);                         \
    GO(destroy_event, iFpp);                        \
    GO(key_press_event, iFpp);                      \
    GO(key_release_event, iFpp);                    \
    GO(enter_notify_event, iFpp);                   \
    GO(leave_notify_event, iFpp);                   \
    GO(configure_event, iFpp);                      \
    GO(focus_in_event, iFpp);                       \
    GO(focus_out_event, iFpp);                      \
    GO(map_event, iFpp);                            \
    GO(unmap_event, iFpp);                          \
    GO(property_notify_event, iFpp);                \
    GO(selection_clear_event, iFpp);                \
    GO(selection_request_event, iFpp);              \
    GO(selection_notify_event, iFpp);               \
    GO(proximity_in_event, iFpp);                   \
    GO(proximity_out_event, iFpp);                  \
    GO(visibility_notify_event, iFpp);              \
    GO(window_state_event, iFpp);                   \
    GO(damage_event, iFpp);                         \
    GO(grab_broken_event, iFpp);                    \
    GO(selection_get, vFppuu);                      \
    GO(selection_received, vFppu);                  \
    GO(drag_begin, vFpp);                           \
    GO(drag_end, vFpp);                             \
    GO(drag_data_get, vFpppuu);                     \
    GO(drag_data_delete, vFpp);                     \
    GO(drag_leave, vFppu);                          \
    GO(drag_motion, iFppiiu);                       \
    GO(drag_drop, iFppiiu);                         \
    GO(drag_data_received, vFppiipuu);              \
    GO(drag_failed, iFppi);                         \
    GO(popup_menu, iFp);                            \
    GO(show_help, iFpi);                            \
    GO(get_accessible, pFp);                        \
    GO(screen_changed, vFpp);                       \
    GO(can_activate_accel, iFpu);                   \
    GO(grab_broken_event, iFpp);                    \
    GO(composited_changed, vFp);                    \
    GO(query_tooltip, iFpiiip);                     \
    GO(compute_expand, vFppp);                      \
    GO(adjust_size_request, vFpipp);                \
    GO(adjust_size_allocation, vFpipppp);           \
    GO(style_updated, vFp);                         \
    GO(touch_event, iFpp);                          \
    GO(get_preferred_height_and_baseline_for_width, vFpipppp);\
    GO(adjust_baseline_request, vFppp);             \
    GO(adjust_baseline_allocation, vFpp);           \
    GO(queue_draw_region, vFpp);                    \

// wrap (so bridge all calls, just in case)
static void wrapGtkWidget3Class(my_GtkWidget3Class_t* class)
{
    wrapGInitiallyUnownedClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkWidget3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkWidget3Class(my_GtkWidget3Class_t* class)
{
    unwrapGInitiallyUnownedClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkWidget3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkWidget3Class(my_GtkWidget3Class_t* class)
{
    bridgeGInitiallyUnownedClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkWidget3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkWidget3Instance(my_GtkWidget3_t* class)
{
    unwrapGInitiallyUnownedInstance(&class->parent);
}
// autobridge
static void bridgeGtkWidget3Instance(my_GtkWidget3_t* class)
{
    bridgeGInitiallyUnownedInstance(&class->parent);
}

// ----- GtkContainer2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkContainer2, add, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer2, remove, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer2, check_resize, void, (void* container), "p", container);
WRAPPER(GtkContainer2, forall, void, (void* container, int include_internals, void* callback, void* callback_data), "pipp", container, include_internals, AddCheckBridge(my_bridge, vFpp, callback, 0, NULL), callback_data);
WRAPPER(GtkContainer2, set_focus_child, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer2, child_type, int, (void* container), "p", container);
WRAPPER(GtkContainer2, composite_name, void*, (void* container, void* child), "pp", container, child);
WRAPPER(GtkContainer2, set_child_property, void, (void* container, void* child, uint32_t property_id, void* value, void* pspec), "ppupp", container, child, property_id, value, pspec);
WRAPPER(GtkContainer2, get_child_property, void, (void* container, void* child, uint32_t property_id, void* value, void* pspec), "ppupp", container, child, property_id, value, pspec);

#define SUPERGO() \
    GO(add, vFpp);                  \
    GO(remove, vFpp);               \
    GO(check_resize, vFp);          \
    GO(forall, vFpipp);             \
    GO(set_focus_child, vFpp);      \
    GO(child_type, iFp);            \
    GO(composite_name, pFpp);       \
    GO(set_child_property, vFppupp);\
    GO(get_child_property, vFppupp);

// wrap (so bridge all calls, just in case)
static void wrapGtkContainer2Class(my_GtkContainer2Class_t* class)
{
    wrapGtkWidget2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkContainer2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkContainer2Class(my_GtkContainer2Class_t* class)
{
    unwrapGtkWidget2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkContainer2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkContainer2Class(my_GtkContainer2Class_t* class)
{
    bridgeGtkWidget2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkContainer2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkContainer2Instance(my_GtkContainer2_t* class)
{
    unwrapGtkWidget2Instance(&class->parent);
}
// autobridge
static void bridgeGtkContainer2Instance(my_GtkContainer2_t* class)
{
    bridgeGtkWidget2Instance(&class->parent);
}

// ----- GtkContainer3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkContainer3, add, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer3, remove, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer3, check_resize, void, (void* container), "p", container);
WRAPPER(GtkContainer3, forall, void, (void* container, int include_internals, void* callback, void* callback_data), "pipp", container, include_internals, AddCheckBridge(my_bridge, vFpp, callback, 0, NULL), callback_data);
WRAPPER(GtkContainer3, set_focus_child, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer3, child_type, int, (void* container), "p", container);
WRAPPER(GtkContainer3, composite_name, void*, (void* container, void* child), "pp", container, child);
WRAPPER(GtkContainer3, set_child_property, void, (void* container, void* child, uint32_t property_id, void* value, void* pspec), "ppupp", container, child, property_id, value, pspec);
WRAPPER(GtkContainer3, get_child_property, void, (void* container, void* child, uint32_t property_id, void* value, void* pspec), "ppupp", container, child, property_id, value, pspec);
WRAPPER(GtkContainer3, get_path_for_child, void*, (void* container, void* child), "pp", container, child);

#define SUPERGO() \
    GO(add, vFpp);                  \
    GO(remove, vFpp);               \
    GO(check_resize, vFp);          \
    GO(forall, vFpipp);             \
    GO(set_focus_child, vFpp);      \
    GO(child_type, iFp);            \
    GO(composite_name, pFpp);       \
    GO(set_child_property, vFppupp);\
    GO(get_child_property, vFppupp);\
    GO(get_path_for_child, pFpp);   \

// wrap (so bridge all calls, just in case)
static void wrapGtkContainer3Class(my_GtkContainer3Class_t* class)
{
    wrapGtkWidget3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkContainer3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkContainer3Class(my_GtkContainer3Class_t* class)
{
    unwrapGtkWidget3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkContainer3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkContainer3Class(my_GtkContainer3Class_t* class)
{
    bridgeGtkWidget3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkContainer3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkContainer3Instance(my_GtkContainer3_t* class)
{
    unwrapGtkWidget3Instance(&class->parent);
}
// autobridge
static void bridgeGtkContainer3Instance(my_GtkContainer3_t* class)
{
    bridgeGtkWidget3Instance(&class->parent);
}

// ----- GtkActionClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkAction, activate, void, (void* action), "p", action);
WRAPPER(GtkAction, create_menu_item, void*, (void* action), "p", action);
WRAPPER(GtkAction, create_tool_item, void*, (void* action), "p", action);
WRAPPER(GtkAction, connect_proxy, void , (void* action, void* proxy), "pp", action, proxy);
WRAPPER(GtkAction, disconnect_proxy, void , (void* action, void* proxy), "pp", action, proxy);
WRAPPER(GtkAction, create_menu, void*, (void* action), "p", action);

#define SUPERGO() \
    GO(activate, vFp);          \
    GO(create_menu_item, pFp);  \
    GO(create_tool_item, pFp);  \
    GO(connect_proxy, vFpp);    \
    GO(disconnect_proxy, vFpp); \
    GO(create_menu, pFp);       \

// wrap (so bridge all calls, just in case)
static void wrapGtkActionClass(my_GtkActionClass_t* class)
{
    wrapGObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkAction (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkActionClass(my_GtkActionClass_t* class)
{
    unwrapGObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkAction (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkActionClass(my_GtkActionClass_t* class)
{
    bridgeGObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkAction (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGtkActionInstance(my_GtkAction_t* class)
{
    unwrapGObjectInstance(&class->parent);
}
// autobridge
static void bridgeGtkActionInstance(my_GtkAction_t* class)
{
    bridgeGObjectInstance(&class->parent);
}

// ----- GtkDrawingArea3Class ------

// wrap (so bridge all calls, just in case)
static void wrapGtkDrawingArea3Class(my_GtkDrawingArea3Class_t* class)
{
    wrapGtkWidget3Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkDrawingArea3Class(my_GtkDrawingArea3Class_t* class)
{
    unwrapGtkWidget3Class(&class->parent_class);
}
// autobridge
static void bridgeGtkDrawingArea3Class(my_GtkDrawingArea3Class_t* class)
{
    bridgeGtkWidget3Class(&class->parent_class);
}

static void unwrapGtkDrawingArea3Instance(my_GtkDrawingArea3_t* class)
{
    unwrapGtkWidget3Instance(&class->parent);
}
// autobridge
static void bridgeGtkDrawingArea3Instance(my_GtkDrawingArea3_t* class)
{
    bridgeGtkWidget3Instance(&class->parent);
}

// ----- GtkMisc2Class ------

// wrap (so bridge all calls, just in case)
static void wrapGtkMisc2Class(my_GtkMisc2Class_t* class)
{
    wrapGtkWidget2Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkMisc2Class(my_GtkMisc2Class_t* class)
{
    unwrapGtkWidget2Class(&class->parent_class);
}
// autobridge
static void bridgeGtkMisc2Class(my_GtkMisc2Class_t* class)
{
    bridgeGtkWidget2Class(&class->parent_class);
}

static void unwrapGtkMisc2Instance(my_GtkMisc2_t* class)
{
    unwrapGtkWidget2Instance(&class->parent);
}
// autobridge
static void bridgeGtkMisc2Instance(my_GtkMisc2_t* class)
{
    bridgeGtkWidget2Instance(&class->parent);
}

// ----- GtkMisc3Class ------
// no wrapper x86 -> natives of callbacks

#define SUPERGO()           \

// wrap (so bridge all calls, just in case)
static void wrapGtkMisc3Class(my_GtkMisc3Class_t* class)
{
    wrapGtkWidget3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkMisc3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkMisc3Class(my_GtkMisc3Class_t* class)
{
    unwrapGtkWidget3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkMisc3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkMisc3Class(my_GtkMisc3Class_t* class)
{
    bridgeGtkWidget3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkMisc3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkMisc3Instance(my_GtkMisc3_t* class)
{
    unwrapGtkWidget3Instance(&class->parent);
}
// autobridge
static void bridgeGtkMisc3Instance(my_GtkMisc3_t* class)
{
    bridgeGtkWidget3Instance(&class->parent);
}

// ----- GtkImage3Class ------

// wrap (so bridge all calls, just in case)
static void wrapGtkImage3Class(my_GtkImage3Class_t* class)
{
    wrapGtkMisc3Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkImage3Class(my_GtkImage3Class_t* class)
{
    unwrapGtkMisc3Class(&class->parent_class);
}
// autobridge
static void bridgeGtkImage3Class(my_GtkImage3Class_t* class)
{
    bridgeGtkMisc3Class(&class->parent_class);
}

static void unwrapGtkImage3Instance(my_GtkImage3_t* class)
{
    unwrapGtkMisc3Instance(&class->parent);
}
// autobridge
static void bridgeGtkImage3Instance(my_GtkImage3_t* class)
{
    bridgeGtkMisc3Instance(&class->parent);
}


// ----- GtkLabel2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkLabel2, move_cursor, void, (void* label, int step, int count, int extend_selection), "piii", label, step, count, extend_selection);
WRAPPER(GtkLabel2, copy_clipboard, void, (void* label), "p", label);
WRAPPER(GtkLabel2, populate_popup, void, (void* label, void* menu), "pp", label, menu);
WRAPPER(GtkLabel2, activate_link, int, (void* label, void* uri), "pp", label, uri);

#define SUPERGO() \
    GO(move_cursor, vFpiii);    \
    GO(copy_clipboard, vFp);    \
    GO(populate_popup, vFpp);   \
    GO(activate_link, iFpp);    \

// wrap (so bridge all calls, just in case)
static void wrapGtkLabel2Class(my_GtkLabel2Class_t* class)
{
    wrapGtkMisc2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkLabel2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkLabel2Class(my_GtkLabel2Class_t* class)
{
    unwrapGtkMisc2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkLabel2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkLabel2Class(my_GtkLabel2Class_t* class)
{
    bridgeGtkMisc2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkLabel2 (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGtkLabel2Instance(my_GtkLabel2_t* class)
{
    unwrapGtkMisc2Instance(&class->misc);
}
// autobridge
static void bridgeGtkLabel2Instance(my_GtkLabel2_t* class)
{
    bridgeGtkMisc2Instance(&class->misc);
}

// ----- GtkLabel3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkLabel3, move_cursor, void, (void* label, int step, int count, int extend_selection), "piii", label, step, count, extend_selection);
WRAPPER(GtkLabel3, copy_clipboard, void, (void* label), "p", label);
WRAPPER(GtkLabel3, populate_popup, void, (void* label, void* menu), "pp", label, menu);
WRAPPER(GtkLabel3, activate_link, int, (void* label, void* uri), "pp", label, uri);

#define SUPERGO() \
    GO(move_cursor, vFpiii);    \
    GO(copy_clipboard, vFp);    \
    GO(populate_popup, vFpp);   \
    GO(activate_link, iFpp);    \

// wrap (so bridge all calls, just in case)
static void wrapGtkLabel3Class(my_GtkLabel3Class_t* class)
{
    wrapGtkMisc3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkLabel3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkLabel3Class(my_GtkLabel3Class_t* class)
{
    unwrapGtkMisc3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkLabel3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkLabel3Class(my_GtkLabel3Class_t* class)
{
    bridgeGtkMisc3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkLabel3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkLabel3Instance(my_GtkLabel3_t* class)
{
    unwrapGtkMisc3Instance(&class->misc);
}
// autobridge
static void bridgeGtkLabel3Instance(my_GtkLabel3_t* class)
{
    bridgeGtkMisc3Instance(&class->misc);
}

// ----- GtkTreeView2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkTreeView2, set_scroll_adjustments, void, (void* tree_view, void* hadjustment, void* vadjustment), "ppp", tree_view, hadjustment, vadjustment);
WRAPPER(GtkTreeView2, row_activated, void, (void* tree_view, void* path, void* column), "ppp", tree_view, path, column);
WRAPPER(GtkTreeView2, test_expand_row, int, (void* tree_view, void* iter, void* path), "ppp", tree_view, iter, path);
WRAPPER(GtkTreeView2, test_collapse_row, int, (void* tree_view, void* iter, void* path), "ppp", tree_view, iter, path);
WRAPPER(GtkTreeView2, row_expanded, void, (void* tree_view, void* iter, void* path), "ppp", tree_view, iter, path);
WRAPPER(GtkTreeView2, row_collapsed, void, (void* tree_view, void* iter, void* path), "ppp", tree_view, iter, path);
WRAPPER(GtkTreeView2, columns_changed, void, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, cursor_changed, void, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, move_cursor, int, (void* tree_view, int step, int count), "pii", tree_view, step, count);
WRAPPER(GtkTreeView2, select_all, int, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, unselect_all, int, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, select_cursor_row, int, (void* tree_view, int start_editing), "pi", tree_view, start_editing);
WRAPPER(GtkTreeView2, toggle_cursor_row, int, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, expand_collapse_cursor_row, int, (void* tree_view, int logical, int expand, int open_all), "piii", tree_view, logical, expand, open_all);
WRAPPER(GtkTreeView2, select_cursor_parent, int, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, start_interactive_search, int, (void* tree_view), "p", tree_view);

#define SUPERGO() \
    GO(set_scroll_adjustments, vFppp);      \
    GO(row_activated, vFppp);               \
    GO(test_expand_row, iFppp);             \
    GO(test_collapse_row, iFppp);           \
    GO(row_expanded, vFppp);                \
    GO(row_collapsed, vFppp);               \
    GO(columns_changed, vFp);               \
    GO(cursor_changed, vFp);                \
    GO(move_cursor, iFppp);                 \
    GO(select_all, iFp);                    \
    GO(unselect_all, iFp);                  \
    GO(select_cursor_row, iFpi);            \
    GO(toggle_cursor_row, iFp);             \
    GO(expand_collapse_cursor_row, iFpiii); \
    GO(select_cursor_parent, iFp);          \
    GO(start_interactive_search, iFp);      \

// wrap (so bridge all calls, just in case)
static void wrapGtkTreeView2Class(my_GtkTreeView2Class_t* class)
{
    wrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkTreeView2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkTreeView2Class(my_GtkTreeView2Class_t* class)
{
    unwrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkTreeView2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkTreeView2Class(my_GtkTreeView2Class_t* class)
{
    bridgeGtkContainer2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkTreeView2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkTreeView2Instance(my_GtkTreeView2_t* class)
{
    unwrapGtkContainer2Instance(&class->parent);
}
// autobridge
static void bridgeGtkTreeView2Instance(my_GtkTreeView2_t* class)
{
    bridgeGtkContainer2Instance(&class->parent);
}

// ----- GtkBin2Class ------

// wrap (so bridge all calls, just in case)
static void wrapGtkBin2Class(my_GtkBin2Class_t* class)
{
    wrapGtkContainer2Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkBin2Class(my_GtkBin2Class_t* class)
{
    unwrapGtkContainer2Class(&class->parent_class);
}
// autobridge
static void bridgeGtkBin2Class(my_GtkBin2Class_t* class)
{
    bridgeGtkContainer2Class(&class->parent_class);
}

static void unwrapGtkBin2Instance(my_GtkBin2_t* class)
{
    unwrapGtkContainer2Instance(&class->container);
}
// autobridge
static void bridgeGtkBin2Instance(my_GtkBin2_t* class)
{
    bridgeGtkContainer2Instance(&class->container);
}

// ----- GtkBin3Class ------
static void wrapGtkBin3Class(my_GtkBin3Class_t* class)
{
    wrapGtkContainer3Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkBin3Class(my_GtkBin3Class_t* class)
{
    unwrapGtkContainer3Class(&class->parent_class);
}
// autobridge
static void bridgeGtkBin3Class(my_GtkBin3Class_t* class)
{
    bridgeGtkContainer3Class(&class->parent_class);
}

static void unwrapGtkBin3Instance(my_GtkBin3_t* class)
{
    unwrapGtkContainer3Instance(&class->container);
}
// autobridge
static void bridgeGtkBin3Instance(my_GtkBin3_t* class)
{
    bridgeGtkContainer3Instance(&class->container);
}
// ----- GtkWindow2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkWindow2, set_focus, void, (void* window, void* focus), "pp", window, focus);
WRAPPER(GtkWindow2, frame_event, int, (void* window, void* event), "pp", window, event);
WRAPPER(GtkWindow2, activate_focus, void, (void* window), "p", window);
WRAPPER(GtkWindow2, activate_default, void, (void* window), "p", window);
WRAPPER(GtkWindow2, move_focus, void, (void* window, int direction), "pi", window, direction);
WRAPPER(GtkWindow2, keys_changed, void, (void* window), "p", window);

#define SUPERGO()               \
    GO(set_focus, vFpp);        \
    GO(frame_event, iFpp);      \
    GO(activate_focus, vFp);    \
    GO(activate_default, vFp);  \
    GO(move_focus, vFpi);       \
    GO(keys_changed, vFp);      \


// wrap (so bridge all calls, just in case)
static void wrapGtkWindow2Class(my_GtkWindow2Class_t* class)
{
    wrapGtkBin2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkWindow2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkWindow2Class(my_GtkWindow2Class_t* class)
{
    unwrapGtkBin2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkWindow2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkWindow2Class(my_GtkWindow2Class_t* class)
{
    bridgeGtkBin2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkWindow2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkWindow2Instance(my_GtkWindow2_t* class)
{
    unwrapGtkBin2Instance(&class->parent);
}
// autobridge
static void bridgeGtkWindow2Instance(my_GtkWindow2_t* class)
{
    bridgeGtkBin2Instance(&class->parent);
}

// ----- GtkWindow3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkWindow3, set_focus, void, (void* window, void* focus), "pp", window, focus);
WRAPPER(GtkWindow3, activate_focus, void, (void* window), "p", window);
WRAPPER(GtkWindow3, activate_default, void, (void* window), "p", window);
WRAPPER(GtkWindow3, keys_changed, void, (void* window), "p", window);
WRAPPER(GtkWindow3, enable_debugging, int, (void* window, int toggle), "pi", window, toggle);

#define SUPERGO()               \
    GO(set_focus, vFpp);        \
    GO(activate_focus, vFp);    \
    GO(activate_default, vFp);  \
    GO(keys_changed, vFp);      \
    GO(enable_debugging, iFpi);


// wrap (so bridge all calls, just in case)
static void wrapGtkWindow3Class(my_GtkWindow3Class_t* class)
{
    wrapGtkBin3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkWindow3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkWindow3Class(my_GtkWindow3Class_t* class)
{
    unwrapGtkBin3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkWindow3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkWindow3Class(my_GtkWindow3Class_t* class)
{
    bridgeGtkBin3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkWindow3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkWindow3Instance(my_GtkWindow3_t* class)
{
    unwrapGtkBin3Instance(&class->bin);
}
// autobridge
static void bridgeGtkWindow3Instance(my_GtkWindow3_t* class)
{
    bridgeGtkBin3Instance(&class->bin);
}

// ----- GtkApplicationWindowClass ------
// wrap (so bridge all calls, just in case)
static void wrapGtkApplicationWindowClass(my_GtkApplicationWindowClass_t* class)
{
    wrapGtkWindow3Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkApplicationWindowClass(my_GtkApplicationWindowClass_t* class)
{
    unwrapGtkWindow3Class(&class->parent_class);
}
// autobridge
static void bridgeGtkApplicationWindowClass(my_GtkApplicationWindowClass_t* class)
{
    bridgeGtkWindow3Class(&class->parent_class);
}

static void unwrapGtkApplicationWindowInstance(my_GtkApplicationWindow_t* class)
{
    unwrapGtkWindow3Instance(&class->parent);
}
// autobridge
static void bridgeGtkApplicationWindowInstance(my_GtkApplicationWindow_t* class)
{
    bridgeGtkWindow3Instance(&class->parent);
}
// ----- GtkListBoxClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkListBoxClass,row_selected, void, (void *box, void *row), "pp", box, row);
WRAPPER(GtkListBoxClass,row_activated, void, (void *box, void *row), "pp", box, row);
WRAPPER(GtkListBoxClass,activate_cursor_row, void, (void *box), "p", box);
WRAPPER(GtkListBoxClass,toggle_cursor_row, void, (void *box), "p", box);
WRAPPER(GtkListBoxClass,move_cursor, void, (void *box, int step, int count), "pii", box, step, count);
WRAPPER(GtkListBoxClass,selected_rows_changed, void, (void *box), "p", box);
WRAPPER(GtkListBoxClass,select_all, void, (void *box), "p", box);
WRAPPER(GtkListBoxClass,unselect_all, void, (void *box), "p", box);

#define SUPERGO()                  \
    GO(row_selected, vFpp);        \
    GO(row_activated, vFpp);    \
    GO(activate_cursor_row, vFp);    \
    GO(toggle_cursor_row, vFp);    \
    GO(move_cursor, vFpii);    \
    GO(selected_rows_changed, vFp);    \
    GO(select_all, vFp);    \
    GO(unselect_all, vFp);

// wrap (so bridge all calls, just in case)
static void wrapGtkListBoxClass(my_GtkListBoxClass_t* class)
{
    wrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkListBoxClass (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkListBoxClass(my_GtkListBoxClass_t* class)
{
    unwrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkListBoxClass (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkListBoxClass(my_GtkListBoxClass_t* class)
{
    bridgeGtkContainer3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkListBoxClass (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkListBoxInstance(my_GtkListBox_t* class)
{
    unwrapGtkContainer3Instance(&class->parent);
}
// autobridge
static void bridgeGtkListBoxInstance(my_GtkListBox_t* class)
{
    bridgeGtkContainer3Instance(&class->parent);
}

// ----- GtkListBoxRowClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkListBoxRowClass, activate, void, (void *row), "p", row);

#define SUPERGO()       \
    GO(activate, vFpp);

// wrap (so bridge all calls, just in case)
static void wrapGtkListBoxRowClass(my_GtkListBoxRowClass_t* class)
{
    wrapGtkBin3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkListBoxRowClass (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkListBoxRowClass(my_GtkListBoxRowClass_t* class)
{
    unwrapGtkBin3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkListBoxRowClass (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkListBoxRowClass(my_GtkListBoxRowClass_t* class)
{
    bridgeGtkBin3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkListBoxRowClass (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkListBoxRowInstance(my_GtkListBoxRow_t* class)
{
    unwrapGtkBin3Instance(&class->parent);
}
// autobridge
static void bridgeGtkListBoxRowInstance(my_GtkListBoxRow_t* class)
{
    bridgeGtkBin3Instance(&class->parent);
}

// ----- GtkTable2Class ------
// wrap (so bridge all calls, just in case)
static void wrapGtkTable2Class(my_GtkTable2Class_t* class)
{
    wrapGtkContainer2Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkTable2Class(my_GtkTable2Class_t* class)
{
    unwrapGtkContainer2Class(&class->parent_class);
}
// autobridge
static void bridgeGtkTable2Class(my_GtkTable2Class_t* class)
{
    bridgeGtkContainer2Class(&class->parent_class);
}

static void unwrapGtkTable2Instance(my_GtkTable2_t* class)
{
    unwrapGtkContainer2Instance(&class->container);
}
// autobridge
static void bridgeGtkTable2Instance(my_GtkTable2_t* class)
{
    bridgeGtkContainer2Instance(&class->container);
}
// ----- GtkFixed2Class ------

// wrap (so bridge all calls, just in case)
static void wrapGtkFixed2Class(my_GtkFixed2Class_t* class)
{
    wrapGtkContainer2Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkFixed2Class(my_GtkFixed2Class_t* class)
{
    unwrapGtkContainer2Class(&class->parent_class);
}
// autobridge
static void bridgeGtkFixed2Class(my_GtkFixed2Class_t* class)
{
    bridgeGtkContainer2Class(&class->parent_class);
}

static void unwrapGtkFixed2Instance(my_GtkFixed2_t* class)
{
    unwrapGtkContainer2Instance(&class->parent);
}
// autobridge
static void bridgeGtkFixed2Instance(my_GtkFixed2_t* class)
{
    bridgeGtkContainer2Instance(&class->parent);
}

// ----- GtkFixed3Class ------

// wrap (so bridge all calls, just in case)
static void wrapGtkFixed3Class(my_GtkFixed3Class_t* class)
{
    wrapGtkContainer3Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkFixed3Class(my_GtkFixed3Class_t* class)
{
    unwrapGtkContainer3Class(&class->parent_class);
}
// autobridge
static void bridgeGtkFixed3Class(my_GtkFixed3Class_t* class)
{
    bridgeGtkContainer3Class(&class->parent_class);
}

static void unwrapGtkFixed3Instance(my_GtkFixed3_t* class)
{
    unwrapGtkContainer3Instance(&class->parent);
}
// autobridge
static void bridgeGtkFixed3Instance(my_GtkFixed3_t* class)
{
    bridgeGtkContainer3Instance(&class->parent);
}

// ----- MetaFrames2Class ------

// wrap (so bridge all calls, just in case)
static void wrapMetaFrames2Class(my_MetaFrames2Class_t* class)
{
    wrapGtkWindow2Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapMetaFrames2Class(my_MetaFrames2Class_t* class)
{
    unwrapGtkWindow2Class(&class->parent_class);
}
// autobridge
static void bridgeMetaFrames2Class(my_MetaFrames2Class_t* class)
{
    bridgeGtkWindow2Class(&class->parent_class);
}

static void unwrapMetaFrames2Instance(my_MetaFrames2_t* class)
{
    unwrapGtkWindow2Instance(&class->parent);
}
// autobridge
static void bridgeMetaFrames2Instance(my_MetaFrames2_t* class)
{
    bridgeGtkWindow2Instance(&class->parent);
}
// ----- GtkNotebook2Class ------
WRAPPER(GtkNotebook2Class, switch_page, void, (void* notebook, void* page, uint32_t page_num), "ppp", notebook, page, page_num);
WRAPPER(GtkNotebook2Class, select_page, int, (void* notebook, int move_focus), "pi", notebook, move_focus);
WRAPPER(GtkNotebook2Class, focus_tab, int, (void* notebook, int type), "pi", notebook, type);
WRAPPER(GtkNotebook2Class, change_current_page, int, (void* notebook, int offset), "pi", notebook, offset);
WRAPPER(GtkNotebook2Class, move_focus_out,void , (void* notebook, int direction), "pi", notebook, direction);
WRAPPER(GtkNotebook2Class, reorder_tab, int, (void* notebook, int direction, int move_to_last), "pii", notebook, direction, move_to_last);
WRAPPER(GtkNotebook2Class, insert_page, int, (void* notebook, void* child, void* tab_label, void* menu_label, int position), "ppppi", notebook, child, tab_label, menu_label, position);
WRAPPER(GtkNotebook2Class, create_window, void*, (void* notebook, void* page, int x, int y), "ppii", notebook, page, x, y);

#define SUPERGO()                   \
    GO(switch_page, vFppp);         \
    GO(select_page, iFpi);          \
    GO(focus_tab, iFpi);            \
    GO(change_current_page, iFpi);  \
    GO(move_focus_out, vFpi);       \
    GO(reorder_tab, iFpii);         \
    GO(insert_page, iFppppi);       \
    GO(create_window, pFppii);      \

// wrap (so bridge all calls, just in case)
static void wrapGtkNotebook2Class(my_GtkNotebook2Class_t* class)
{
    wrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkNotebook2Class (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkNotebook2Class(my_GtkNotebook2Class_t* class)
{
    unwrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkNotebook2Class (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkNotebook2Class(my_GtkNotebook2Class_t* class)
{
    bridgeGtkContainer2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkNotebook2Class (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkNotebook2Instance(my_GtkNotebook2_t* class)
{
    unwrapGtkContainer2Instance(&class->parent);
}
// autobridge
static void bridgeGtkNotebook2Instance(my_GtkNotebook2_t* class)
{
    bridgeGtkContainer2Instance(&class->parent);
}

// ----- GtkCellRenderer2Class ------
WRAPPER(GtkCellRenderer2Class, get_size, void, (void* cell, void* widget, void* cell_area, int* x_offset, int* y_offset, int* width, int* height), "ppppppp", cell, widget, cell_area, x_offset, y_offset, width, height);
WRAPPER(GtkCellRenderer2Class, render, void, (void* cell, void* window, void* widget, void* background_area, void* cell_area, void* expose_area, int flags), "ppppppi", cell, window, widget, background_area, cell_area, expose_area, flags);
WRAPPER(GtkCellRenderer2Class, activate, int, (void* cell, void* event, void* widget, void* path, void* background_area, void* cell_area, int flags), "ppppppi", cell, event, widget, path, background_area, cell_area, flags);
WRAPPER(GtkCellRenderer2Class, start_editing, void*, (void* cell, void* event, void* widget, void* path, void* background_area, void* cell_area, int flags), "ppppppi", cell, event, widget, path, background_area, cell_area, flags);
WRAPPER(GtkCellRenderer2Class, editing_canceled, void, (void* cell), "p", cell);
WRAPPER(GtkCellRenderer2Class, editing_started, void, (void* cell, void* editable, void* path), "ppp", cell, editable, path);
  
#define SUPERGO()                   \
    GO(get_size, vFppppppp);        \
    GO(render, vFppppppi);          \
    GO(activate, iFppppppi);        \
    GO(start_editing, pFppppppi);   \
    GO(editing_canceled, vFp);      \
    GO(editing_started, vFppp);     \

// wrap (so bridge all calls, just in case)
static void wrapGtkCellRenderer2Class(my_GtkCellRenderer2Class_t* class)
{
    wrapGtkObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkCellRenderer2Class (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkCellRenderer2Class(my_GtkCellRenderer2Class_t* class)
{
    unwrapGtkObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkCellRenderer2Class (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkCellRenderer2Class(my_GtkCellRenderer2Class_t* class)
{
    bridgeGtkObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkCellRenderer2Class (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkCellRenderer2Instance(my_GtkCellRenderer2_t* class)
{
    unwrapGtkObjectInstance(&class->parent);
}
// autobridge
static void bridgeGtkCellRenderer2Instance(my_GtkCellRenderer2_t* class)
{
    bridgeGtkObjectInstance(&class->parent);
}

// ----- GtkCellRendererText2Class ------
WRAPPER(GtkCellRendererText2Class, edited, void, (void* cell_renderer_text, void* path, void* new_text), "ppp", cell_renderer_text, path, new_text);
  
#define SUPERGO()                   \
    GO(edited, vFppp);              \

// wrap (so bridge all calls, just in case)
static void wrapGtkCellRendererText2Class(my_GtkCellRendererText2Class_t* class)
{
    wrapGtkCellRenderer2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkCellRendererText2Class (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkCellRendererText2Class(my_GtkCellRendererText2Class_t* class)
{
    unwrapGtkCellRenderer2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkCellRendererText2Class (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkCellRendererText2Class(my_GtkCellRendererText2Class_t* class)
{
    bridgeGtkCellRenderer2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkCellRendererText2Class (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkCellRendererText2Instance(my_GtkCellRendererText2_t* class)
{
    unwrapGtkCellRenderer2Instance(&class->parent);
}
// autobridge
static void bridgeGtkCellRendererText2Instance(my_GtkCellRendererText2_t* class)
{
    bridgeGtkCellRenderer2Instance(&class->parent);
}

// ----- GDBusObjectManagerClientClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GDBusObjectManagerClient,interface_proxy_signal, void, (void* manager, void* object_proxy, void* interface_proxy, void* sender_name, void* signal_name, void* parameters), "pppppp", manager, object_proxy, interface_proxy, sender_name, signal_name, parameters);
WRAPPER(GDBusObjectManagerClient,interface_proxy_properties_changed, void, (void* manager, void* object_proxy, void* interface_proxy, void* changed_properties, void* invalidated_properties), "ppppp", manager, object_proxy, interface_proxy, changed_properties, invalidated_properties);

#define SUPERGO()                                       \
    GO(interface_proxy_signal, vFpppppp);               \
    GO(interface_proxy_properties_changed, vFppppp);    \


// wrap (so bridge all calls, just in case)
static void wrapGDBusObjectManagerClientClass(my_GDBusObjectManagerClientClass_t* class)
{
    wrapGObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GDBusObjectManagerClient (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGDBusObjectManagerClientClass(my_GDBusObjectManagerClientClass_t* class)
{
    unwrapGObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GDBusObjectManagerClient (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGDBusObjectManagerClientClass(my_GDBusObjectManagerClientClass_t* class)
{
    bridgeGObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GDBusObjectManagerClient (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGDBusObjectManagerClientInstance(my_GDBusObjectManagerClient_t* class)
{
    unwrapGObjectInstance(&class->parent);
}
// autobridge
static void bridgeGDBusObjectManagerClientInstance(my_GDBusObjectManagerClient_t* class)
{
    bridgeGObjectInstance(&class->parent);
}

// ----- GDBusInterfaceSkeletonClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GDBusInterfaceSkeleton,get_info, void*, (void* interface_), "p", interface_);
WRAPPER_RET(GDBusInterfaceSkeleton,get_vtable, my_GDBusInterfaceVTable_t*, findFreeGDBusInterfaceVTable,(void* interface_), "p", interface_);
WRAPPER(GDBusInterfaceSkeleton,get_properties, void*, (void* interface_), "p", interface_);
WRAPPER(GDBusInterfaceSkeleton,flush, void, (void* interface_), "p", interface_);
WRAPPER(GDBusInterfaceSkeleton,g_authorize_method, int, (void* interface_, void* invocation), "pp", interface_, invocation);

#define SUPERGO()                   \
    GO(get_info, pFp);              \
    GO(get_vtable, pFp);            \
    GO(get_properties, pFp);        \
    GO(flush, vFp);                 \
    GO(g_authorize_method, iFpp);   \


// wrap (so bridge all calls, just in case)
static void wrapGDBusInterfaceSkeletonClass(my_GDBusInterfaceSkeletonClass_t* class)
{
    wrapGObjectClass(&class->parent);
    #define GO(A, W) class->A = reverse_##A##_GDBusInterfaceSkeleton (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGDBusInterfaceSkeletonClass(my_GDBusInterfaceSkeletonClass_t* class)
{
    unwrapGObjectClass(&class->parent);
    #define GO(A, W)   class->A = find_##A##_GDBusInterfaceSkeleton (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGDBusInterfaceSkeletonClass(my_GDBusInterfaceSkeletonClass_t* class)
{
    bridgeGObjectClass(&class->parent);
    #define GO(A, W) autobridge_##A##_GDBusInterfaceSkeleton (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGDBusInterfaceSkeletonInstance(my_GDBusInterfaceSkeleton_t* class)
{
    unwrapGObjectInstance(&class->parent);
}
// autobridge
static void bridgeGDBusInterfaceSkeletonInstance(my_GDBusInterfaceSkeleton_t* class)
{
    bridgeGObjectInstance(&class->parent);
}

// ----- GtkButton2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkButton2, pressed, void,  (void* button), "p", button);
WRAPPER(GtkButton2, released, void, (void* button), "p", button);
WRAPPER(GtkButton2, clicked, void,  (void* button), "p", button);
WRAPPER(GtkButton2, enter, void,    (void* button), "p", button);
WRAPPER(GtkButton2, leave, void,    (void* button), "p", button);
WRAPPER(GtkButton2, activate, void, (void* button), "p", button);

#define SUPERGO()               \
    GO(pressed, vFp);           \
    GO(released, vFp);          \
    GO(clicked, vFp);           \
    GO(enter, vFp);             \
    GO(leave, vFp);             \
    GO(activate, vFp);          \


// wrap (so bridge all calls, just in case)
static void wrapGtkButton2Class(my_GtkButton2Class_t* class)
{
    wrapGtkBin2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkButton2Class(my_GtkButton2Class_t* class)
{
    unwrapGtkBin2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkButton2Class(my_GtkButton2Class_t* class)
{
    bridgeGtkBin2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkButton2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkButton2Instance(my_GtkButton2_t* class)
{
    unwrapGtkBin2Instance(&class->bin);
}
// autobridge
static void bridgeGtkButton2Instance(my_GtkButton2_t* class)
{
    bridgeGtkBin2Instance(&class->bin);
}

// ----- GtkButton3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkButton3, pressed, void,  (void* button), "p", button);
WRAPPER(GtkButton3, released, void, (void* button), "p", button);
WRAPPER(GtkButton3, clicked, void,  (void* button), "p", button);
WRAPPER(GtkButton3, enter, void,    (void* button), "p", button);
WRAPPER(GtkButton3, leave, void,    (void* button), "p", button);
WRAPPER(GtkButton3, activate, void, (void* button), "p", button);

#define SUPERGO()               \
    GO(pressed, vFp);           \
    GO(released, vFp);          \
    GO(clicked, vFp);           \
    GO(enter, vFp);             \
    GO(leave, vFp);             \
    GO(activate, vFp);          \


// wrap (so bridge all calls, just in case)
static void wrapGtkButton3Class(my_GtkButton3Class_t* class)
{
    wrapGtkBin3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkButton3Class(my_GtkButton3Class_t* class)
{
    unwrapGtkBin3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkButton3Class(my_GtkButton3Class_t* class)
{
    bridgeGtkBin3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkButton3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkButton3Instance(my_GtkButton3_t* class)
{
    unwrapGtkBin3Instance(&class->bin);
}
// autobridge
static void bridgeGtkButton3Instance(my_GtkButton3_t* class)
{
    bridgeGtkBin3Instance(&class->bin);
}

// ----- GtkComboBox2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkComboBox2, changed, void, (void* combo_box), "p", combo_box);
WRAPPER(GtkComboBox2, get_active_text, void*, (void* combo_box), "p", combo_box);

#define SUPERGO()               \
    GO(changed, vFp);           \
    GO(get_active_text, pFp);   \


// wrap (so bridge all calls, just in case)
static void wrapGtkComboBox2Class(my_GtkComboBox2Class_t* class)
{
    wrapGtkBin2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkComboBox2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkComboBox2Class(my_GtkComboBox2Class_t* class)
{
    unwrapGtkBin2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkComboBox2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkComboBox2Class(my_GtkComboBox2Class_t* class)
{
    bridgeGtkBin2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkComboBox2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkComboBox2Instance(my_GtkComboBox2_t* class)
{
    unwrapGtkBin2Instance(&class->parent);
}
// autobridge
static void bridgeGtkComboBox2Instance(my_GtkComboBox2_t* class)
{
    bridgeGtkBin2Instance(&class->parent);
}

// ----- GtkToggleButton2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkToggleButton2, toggled, void, (void* toggle_button), "p", toggle_button);

#define SUPERGO()               \
    GO(toggled, vFp);           \


// wrap (so bridge all calls, just in case)
static void wrapGtkToggleButton2Class(my_GtkToggleButton2Class_t* class)
{
    wrapGtkButton2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkToggleButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkToggleButton2Class(my_GtkToggleButton2Class_t* class)
{
    unwrapGtkButton2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkToggleButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkToggleButton2Class(my_GtkToggleButton2Class_t* class)
{
    bridgeGtkButton2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkToggleButton2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkToggleButton2Instance(my_GtkToggleButton2_t* class)
{
    unwrapGtkButton2Instance(&class->button);
}
// autobridge
static void bridgeGtkToggleButton2Instance(my_GtkToggleButton2_t* class)
{
    bridgeGtkButton2Instance(&class->button);
}

// ----- GtkToggleButton3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkToggleButton3, toggled, void, (void* toggle_button), "p", toggle_button);

#define SUPERGO()               \
    GO(toggled, vFp);           \


// wrap (so bridge all calls, just in case)
static void wrapGtkToggleButton3Class(my_GtkToggleButton3Class_t* class)
{
    wrapGtkButton3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkToggleButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkToggleButton3Class(my_GtkToggleButton3Class_t* class)
{
    unwrapGtkButton3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkToggleButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkToggleButton3Class(my_GtkToggleButton3Class_t* class)
{
    bridgeGtkButton3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkToggleButton2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkToggleButton3Instance(my_GtkToggleButton3_t* class)
{
    unwrapGtkButton3Instance(&class->parent);
}
// autobridge
static void bridgeGtkToggleButton3Instance(my_GtkToggleButton3_t* class)
{
    bridgeGtkButton3Instance(&class->parent);
}

// ----- GtkCheckButton2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkCheckButton2, draw_indicator, void, (void* check_button, void* area), "pp", check_button, area);

#define SUPERGO()               \
    GO(draw_indicator, vFpp);   \


// wrap (so bridge all calls, just in case)
static void wrapGtkCheckButton2Class(my_GtkCheckButton2Class_t* class)
{
    wrapGtkToggleButton2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkCheckButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkCheckButton2Class(my_GtkCheckButton2Class_t* class)
{
    unwrapGtkToggleButton2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkCheckButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkCheckButton2Class(my_GtkCheckButton2Class_t* class)
{
    bridgeGtkToggleButton2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkCheckButton2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkCheckButton2Instance(my_GtkCheckButton2_t* class)
{
    unwrapGtkToggleButton2Instance(&class->parent);
}
// autobridge
static void bridgeGtkCheckButton2Instance(my_GtkCheckButton2_t* class)
{
    bridgeGtkToggleButton2Instance(&class->parent);
}

// ----- GtkCheckButton3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkCheckButton3, draw_indicator, void, (void* check_button, void* area), "pp", check_button, area);

#define SUPERGO()               \
    GO(draw_indicator, vFpp);   \


// wrap (so bridge all calls, just in case)
static void wrapGtkCheckButton3Class(my_GtkCheckButton3Class_t* class)
{
    wrapGtkToggleButton3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkCheckButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkCheckButton3Class(my_GtkCheckButton3Class_t* class)
{
    unwrapGtkToggleButton3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkCheckButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkCheckButton3Class(my_GtkCheckButton3Class_t* class)
{
    bridgeGtkToggleButton3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkCheckButton3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkCheckButton3Instance(my_GtkCheckButton3_t* class)
{
    unwrapGtkToggleButton3Instance(&class->parent);
}
// autobridge
static void bridgeGtkCheckButton3Instance(my_GtkCheckButton3_t* class)
{
    bridgeGtkToggleButton3Instance(&class->parent);
}
// ----- GtkGtkMenuButton3Class ------
// wrapper x86 -> natives of callbacks

#define SUPERGO() \

// wrap (so bridge all calls, just in case)
static void wrapGtkMenuButton3Class(my_GtkMenuButton3Class_t* class)
{
    wrapGtkToggleButton3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkMenuButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkMenuButton3Class(my_GtkMenuButton3Class_t* class)
{
    unwrapGtkToggleButton3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkMenuButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkMenuButton3Class(my_GtkMenuButton3Class_t* class)
{
    bridgeGtkToggleButton3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkMenuButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGtkMenuButton3Instance(my_GtkMenuButton3_t* class)
{
    unwrapGtkToggleButton3Instance(&class->parent);
}
// autobridge
static void bridgeGtkMenuButton3Instance(my_GtkMenuButton3_t* class)
{
    bridgeGtkToggleButton3Instance(&class->parent);
}
// ----- GtkEntry2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkEntry2, populate_popup, void,     (void* entry, void* menu), "pp", entry, menu);
WRAPPER(GtkEntry2, activate, void,           (void* entry), "p", entry);
WRAPPER(GtkEntry2, move_cursor, void,        (void* entry, int step, int count, int extend_selection), "piii", entry, step, count, extend_selection);
WRAPPER(GtkEntry2, insert_at_cursor, void,   (void* entry, void* str), "pp", entry, str);
WRAPPER(GtkEntry2, delete_from_cursor, void, (void* entry, size_t type, int count), "pLi", entry, type, count);
WRAPPER(GtkEntry2, backspace, void,          (void* entry), "p", entry);
WRAPPER(GtkEntry2, cut_clipboard, void,      (void* entry), "p", entry);
WRAPPER(GtkEntry2, copy_clipboard, void,     (void* entry), "p", entry);
WRAPPER(GtkEntry2, paste_clipboard, void,    (void* entry), "p", entry);
WRAPPER(GtkEntry2, toggle_overwrite, void,   (void* entry), "p", entry);
WRAPPER(GtkEntry2, get_text_area_size, void, (void* entry, void* x, void* y, void* width, void* height), "ppppp", entry, x, y, width, height);

#define SUPERGO()                   \
    GO(populate_popup, vFpp);       \
    GO(activate, vFp);              \
    GO(move_cursor, vFpiii);        \
    GO(insert_at_cursor, vFp);      \
    GO(delete_from_cursor, vFpii);  \
    GO(backspace, vFp);             \
    GO(cut_clipboard, vFp);         \
    GO(copy_clipboard, vFp);        \
    GO(paste_clipboard, vFp);       \
    GO(toggle_overwrite, vFp);      \
    GO(get_text_area_size, vFppppp);\

// wrap (so bridge all calls, just in case)
static void wrapGtkEntry2Class(my_GtkEntry2Class_t* class)
{
    wrapGtkWidget2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkEntry2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkEntry2Class(my_GtkEntry2Class_t* class)
{
    unwrapGtkWidget2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkEntry2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkEntry2Class(my_GtkEntry2Class_t* class)
{
    bridgeGtkWidget2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkEntry2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkEntry2Instance(my_GtkEntry2_t* class)
{
    unwrapGtkWidget2Instance(&class->parent);
}
// autobridge
static void bridgeGtkEntry2Instance(my_GtkEntry2_t* class)
{
    bridgeGtkWidget2Instance(&class->parent);
}

// ----- GtkSpinButton2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkSpinButton2, input, int,  (void* spin_button, void* new_value), "pp", spin_button, new_value);
WRAPPER(GtkSpinButton2, output, int, (void* spin_button), "p", spin_button);
WRAPPER(GtkSpinButton2, value_changed, void, (void* spin_button), "p", spin_button);
WRAPPER(GtkSpinButton2, change_value, void, (void* spin_button, int scroll), "pi", spin_button, scroll);
WRAPPER(GtkSpinButton2, wrapped, void, (void* spin_button), "p", spin_button);

#define SUPERGO()           \
    GO(input, iFpp);        \
    GO(output, iFp);        \
    GO(value_changed, vFp); \
    GO(change_value, vFpi); \
    GO(wrapped, vFp);       \

// wrap (so bridge all calls, just in case)
static void wrapGtkSpinButton2Class(my_GtkSpinButton2Class_t* class)
{
    wrapGtkEntry2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkSpinButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkSpinButton2Class(my_GtkSpinButton2Class_t* class)
{
    unwrapGtkEntry2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkSpinButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkSpinButton2Class(my_GtkSpinButton2Class_t* class)
{
    bridgeGtkEntry2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkSpinButton2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkSpinButton2Instance(my_GtkSpinButton2_t* class)
{
    unwrapGtkEntry2Instance(&class->entry);
}
// autobridge
static void bridgeGtkSpinButton2Instance(my_GtkSpinButton2_t* class)
{
    bridgeGtkEntry2Instance(&class->entry);
}

// ----- GtkProgress2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkProgress2, paint, void,          (void* progress), "p", progress);
WRAPPER(GtkProgress2, update, void,         (void* progress), "p", progress);
WRAPPER(GtkProgress2, act_mode_enter, void, (void* progress), "p", progress);

#define SUPERGO()           \
    GO(paint, vFp);         \
    GO(update, vFp);        \
    GO(act_mode_enter, vFp);\

// wrap (so bridge all calls, just in case)
static void wrapGtkProgress2Class(my_GtkProgress2Class_t* class)
{
    wrapGtkWidget2Class(&class->parent);
    #define GO(A, W) class->A = reverse_##A##_GtkProgress2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkProgress2Class(my_GtkProgress2Class_t* class)
{
    unwrapGtkWidget2Class(&class->parent);
    #define GO(A, W)   class->A = find_##A##_GtkProgress2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkProgress2Class(my_GtkProgress2Class_t* class)
{
    bridgeGtkWidget2Class(&class->parent);
    #define GO(A, W) autobridge_##A##_GtkProgress2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkProgress2Instance(my_GtkProgress2_t* class)
{
    unwrapGtkWidget2Instance(&class->widget);
}
// autobridge
static void bridgeGtkProgress2Instance(my_GtkProgress2_t* class)
{
    bridgeGtkWidget2Instance(&class->widget);
}

// ----- GtkProgressBar2Class ------
// no wrapper x86 -> natives of callbacks

#define SUPERGO()           \

// wrap (so bridge all calls, just in case)
static void wrapGtkProgressBar2Class(my_GtkProgressBar2Class_t* class)
{
    wrapGtkProgress2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkProgressBar2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkProgressBar2Class(my_GtkProgressBar2Class_t* class)
{
    unwrapGtkProgress2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkProgressBar2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkProgressBar2Class(my_GtkProgressBar2Class_t* class)
{
    bridgeGtkProgress2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkProgressBar2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkProgressBar2Instance(my_GtkProgressBar2_t* class)
{
    unwrapGtkProgress2Instance(&class->parent);
}
// autobridge
static void bridgeGtkProgressBar2Instance(my_GtkProgressBar2_t* class)
{
    bridgeGtkProgress2Instance(&class->parent);
}

// ----- GtkFrame2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkFrame2, compute_child_allocation, void, (void* frame, void* allocation), "pp", frame, allocation);

#define SUPERGO()                       \
    GO(compute_child_allocation, vFpp); \

// wrap (so bridge all calls, just in case)
static void wrapGtkFrame2Class(my_GtkFrame2Class_t* class)
{
    wrapGtkBin2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkFrame2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkFrame2Class(my_GtkFrame2Class_t* class)
{
    unwrapGtkBin2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkFrame2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkFrame2Class(my_GtkFrame2Class_t* class)
{
    bridgeGtkBin2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkFrame2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkFrame2Instance(my_GtkFrame2_t* class)
{
    unwrapGtkBin2Instance(&class->bin);
}
// autobridge
static void bridgeGtkFrame2Instance(my_GtkFrame2_t* class)
{
    bridgeGtkBin2Instance(&class->bin);
}

// ----- GtkMenuShell2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkMenuShell2,deactivate, void,      (void* menu_shell), "p", menu_shell);
WRAPPER(GtkMenuShell2,selection_done, void,  (void* menu_shell), "p", menu_shell);
WRAPPER(GtkMenuShell2,move_current, void,    (void* menu_shell, int direction),  "pi", menu_shell, direction);
WRAPPER(GtkMenuShell2,activate_current, void,(void* menu_shell, int force_hide), "pi", menu_shell, force_hide);
WRAPPER(GtkMenuShell2,cancel, void,          (void* menu_shell), "p", menu_shell);
WRAPPER(GtkMenuShell2,select_item, void,     (void* menu_shell, void* menu_item), "pp", menu_shell, menu_item);
WRAPPER(GtkMenuShell2,insert, void,          (void* menu_shell, void* child, int position), "ppi", menu_shell, child, position);
WRAPPER(GtkMenuShell2,get_popup_delay, int,  (void* menu_shell), "p", menu_shell);
WRAPPER(GtkMenuShell2,move_selected, int,    (void* menu_shell, int distance), "pi", menu_shell, distance);

#define SUPERGO()               \
    GO(deactivate, vFp);        \
    GO(selection_done, vFp);    \
    GO(move_current, vFpi);     \
    GO(activate_current, vFpi); \
    GO(cancel, vFp);            \
    GO(select_item, vFpp);      \
    GO(insert, vFppi);          \
    GO(get_popup_delay, iFp);   \
    GO(move_selected, iFpi);    \

// wrap (so bridge all calls, just in case)
static void wrapGtkMenuShell2Class(my_GtkMenuShell2Class_t* class)
{
    wrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkMenuShell2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkMenuShell2Class(my_GtkMenuShell2Class_t* class)
{
    unwrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkMenuShell2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkMenuShell2Class(my_GtkMenuShell2Class_t* class)
{
    bridgeGtkContainer2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkMenuShell2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkMenuShell2Instance(my_GtkMenuShell2_t* class)
{
    unwrapGtkContainer2Instance(&class->container);
}
// autobridge
static void bridgeGtkMenuShell2Instance(my_GtkMenuShell2_t* class)
{
    bridgeGtkContainer2Instance(&class->container);
}

// ----- GtkMenuBar2Class ------
// no wrapper x86 -> natives of callbacks

#define SUPERGO()           \

// wrap (so bridge all calls, just in case)
static void wrapGtkMenuBar2Class(my_GtkMenuBar2Class_t* class)
{
    wrapGtkMenuShell2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkMenuBar2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkMenuBar2Class(my_GtkMenuBar2Class_t* class)
{
    unwrapGtkMenuShell2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkMenuBar2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkMenuBar2Class(my_GtkMenuBar2Class_t* class)
{
    bridgeGtkMenuShell2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkMenuBar2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkMenuBar2Instance(my_GtkMenuBar2_t* class)
{
    unwrapGtkMenuShell2Instance(&class->parent);
}
// autobridge
static void bridgeGtkMenuBar2Instance(my_GtkMenuBar2_t* class)
{
    bridgeGtkMenuShell2Instance(&class->parent);
}

// ----- GtkTextView2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkTextView2, set_scroll_adjustments, void,   (void* text_view, void* hadjustment, void* vadjustment), "ppp", text_view, hadjustment, vadjustment);
WRAPPER(GtkTextView2, populate_popup, void,           (void* text_view, void* menu), "pp", text_view, menu);
WRAPPER(GtkTextView2, move_cursor, void,              (void* text_view, int step, int count, int extend_selection), "piii", text_view, step, count, extend_selection);
WRAPPER(GtkTextView2, page_horizontally, void,        (void* text_view, int count, int extend_selection), "pii", text_view, count, extend_selection);
WRAPPER(GtkTextView2, set_anchor, void,               (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, insert_at_cursor, void,         (void* text_view, void* str), "pp", text_view, str);
WRAPPER(GtkTextView2, delete_from_cursor, void,       (void* text_view, int type, int count), "pii", text_view, type, count);
WRAPPER(GtkTextView2, backspace, void,                (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, cut_clipboard, void,            (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, copy_clipboard, void,           (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, paste_clipboard, void,          (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, toggle_overwrite, void,         (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, move_focus, void,               (void* text_view, int direction), "pi", text_view, direction);

#define SUPERGO()                       \
    GO(set_scroll_adjustments, vFppp);  \
    GO(populate_popup, vFpp);           \
    GO(move_cursor, vFpiii);            \
    GO(page_horizontally, vFpii);       \
    GO(set_anchor, vFp);                \
    GO(insert_at_cursor, vFpp);         \
    GO(delete_from_cursor, vFpii);      \
    GO(backspace, vFp);                 \
    GO(cut_clipboard, vFp);             \
    GO(copy_clipboard, vFp);            \
    GO(paste_clipboard, vFp);           \
    GO(toggle_overwrite, vFp);          \
    GO(move_focus, vFpi);               \

// wrap (so bridge all calls, just in case)
static void wrapGtkTextView2Class(my_GtkTextView2Class_t* class)
{
    wrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkTextView2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkTextView2Class(my_GtkTextView2Class_t* class)
{
    unwrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkTextView2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkTextView2Class(my_GtkTextView2Class_t* class)
{
    bridgeGtkContainer2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkTextView2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkTextView2Instance(my_GtkTextView2_t* class)
{
    unwrapGtkContainer2Instance(&class->parent);
}
// autobridge
static void bridgeGtkTextView2Instance(my_GtkTextView2_t* class)
{
    bridgeGtkContainer2Instance(&class->parent);
}

// ----- GtkTextView3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkTextView3, populate_popup, void,           (void* text_view, void* menu), "pp", text_view, menu);
WRAPPER(GtkTextView3, move_cursor, void,              (void* text_view, int step, int count, int extend_selection), "piii", text_view, step, count, extend_selection);
WRAPPER(GtkTextView3, set_anchor, void,               (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, insert_at_cursor, void,         (void* text_view, void* str), "pp", text_view, str);
WRAPPER(GtkTextView3, delete_from_cursor, void,       (void* text_view, int type, int count), "pii", text_view, type, count);
WRAPPER(GtkTextView3, backspace, void,                (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, cut_clipboard, void,            (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, copy_clipboard, void,           (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, paste_clipboard, void,          (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, toggle_overwrite, void,         (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, create_buffer, void*,           (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, draw_layer, void,               (void* text_view, int layer, void* cr), "pip", text_view, layer, cr);
WRAPPER(GtkTextView3, extend_selection, int,          (void* text_view, int granularity, void* location, void* start, void* end), "pippp", text_view, granularity, location, start, end);
WRAPPER(GtkTextView3, insert_emoji, void,             (void* text_view), "p", text_view);

#define SUPERGO()                       \
    GO(populate_popup, vFpp);           \
    GO(move_cursor, vFpiii);            \
    GO(set_anchor, vFp);                \
    GO(insert_at_cursor, vFpp);         \
    GO(delete_from_cursor, vFpii);      \
    GO(backspace, vFp);                 \
    GO(cut_clipboard, vFp);             \
    GO(copy_clipboard, vFp);            \
    GO(paste_clipboard, vFp);           \
    GO(toggle_overwrite, vFp);          \
    GO(create_buffer, pFp);             \
    GO(draw_layer, vFpip);              \
    GO(extend_selection, iFpippp);      \
    GO(insert_emoji, vFp);              \

// wrap (so bridge all calls, just in case)
static void wrapGtkTextView3Class(my_GtkTextView3Class_t* class)
{
    wrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkTextView3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkTextView3Class(my_GtkTextView3Class_t* class)
{
    unwrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkTextView3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkTextView3Class(my_GtkTextView3Class_t* class)
{
    bridgeGtkContainer3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkTextView3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkTextView3Instance(my_GtkTextView3_t* class)
{
    unwrapGtkContainer3Instance(&class->parent);
}
// autobridge
static void bridgeGtkTextView3Instance(my_GtkTextView3_t* class)
{
    bridgeGtkContainer3Instance(&class->parent);
}

// ----- GtkGrid3Class ------
// no wrapper x86 -> natives of callbacks

#define SUPERGO()           \

// wrap (so bridge all calls, just in case)
static void wrapGtkGrid3Class(my_GtkGrid3Class_t* class)
{
    wrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkGrid3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkGrid3Class(my_GtkGrid3Class_t* class)
{
    unwrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkGrid3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkGrid3Class(my_GtkGrid3Class_t* class)
{
    bridgeGtkContainer3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkGrid3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkGrid3Instance(my_GtkGrid3_t* class)
{
    unwrapGtkContainer3Instance(&class->parent);
}
// autobridge
static void bridgeGtkGrid3Instance(my_GtkGrid3_t* class)
{
    bridgeGtkContainer3Instance(&class->parent);
}

// ----- GtkEventControllerClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkEventController, set_widget, void, (void* controller, void* widget), "pp", controller, widget);
WRAPPER(GtkEventController, unset_widget, void, (void* controller), "p", controller);
WRAPPER(GtkEventController, handle_event, int, (void *controller, void *event, double x, double y), "ppdd", controller, event, x, y);
WRAPPER(GtkEventController, reset, void, (void* controller), "p", controller);
WRAPPER(GtkEventController, handle_crossing, void, (void *controller, void *crossing, double x, double y), "ppdd", controller, crossing, x, y);
WRAPPER(GtkEventController, filter_event, void, (void *controller, void *event), "pp", controller, event);

#define SUPERGO()               \
    GO(set_widget, vFpp);       \
    GO(unset_widget, vFp);      \
    GO(handle_event, iFppdd);   \
    GO(reset, vFp);             \
    GO(handle_crossing, vFppdd);\
    GO(filter_event, vFpp);

// wrap (so bridge all calls, just in case)
static void wrapGtkEventControllerClass(my_GtkEventControllerClass_t* class)
{
    wrapGObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkEventController(W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkEventControllerClass(my_GtkEventControllerClass_t* class)
{
    unwrapGObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkEventController (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkEventControllerClass(my_GtkEventControllerClass_t* class)
{
    bridgeGObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkEventController (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkEventControllerInstance(my_GtkEventController_t* class)
{
    unwrapGObjectInstance(&class->parent);
}
// autobridge
static void bridgeGtkEventControllerInstance(my_GtkEventController_t* class)
{
    bridgeGObjectInstance(&class->parent);
}

// ----- GtkGestureClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkGesture, check, void, (void* gesture), "p", gesture);
WRAPPER(GtkGesture, begin, void, (void *gesture, void *sequence), "pp", gesture, sequence);
WRAPPER(GtkGesture, update, void, (void *gesture, void *sequence), "pp", gesture, sequence);
WRAPPER(GtkGesture, end, void, (void *gesture, void *sequence), "pp", gesture, sequence);
WRAPPER(GtkGesture, cancel, void, (void *gesture, void *sequence), "pp", gesture, sequence);
WRAPPER(GtkGesture, sequence_state_changed, void, (void *gesture, void *sequence, int state), "ppi", gesture, sequence, state);

#define SUPERGO()              \
    GO(check, vFp);            \
    GO(begin, vFpp);           \
    GO(update, vFpp);          \
    GO(end, vFpp);             \
    GO(cancel, vFpp);          \
    GO(sequence_state_changed, vFppi);

// wrap (so bridge all calls, just in case)
static void wrapGtkGestureClass(my_GtkGestureClass_t* class)
{
    wrapGtkEventControllerClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkGesture(W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkGestureClass(my_GtkGestureClass_t* class)
{
    unwrapGtkEventControllerClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkGesture (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkGestureClass(my_GtkGestureClass_t* class)
{
    bridgeGtkEventControllerClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkGesture (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkGestureInstance(my_GtkGesture_t* class)
{
    unwrapGtkEventControllerInstance(&class->parent);
}
// autobridge
static void bridgeGtkGestureInstance(my_GtkGesture_t* class)
{
    bridgeGtkEventControllerInstance(&class->parent);
}

// ----- GtkGestureSingleClass ------
// wrap (so bridge all calls, just in case)
static void wrapGtkGestureSingleClass(my_GtkGestureSingleClass_t* class)
{
    wrapGtkGestureClass(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkGestureSingleClass(my_GtkGestureSingleClass_t* class)
{
    unwrapGtkGestureClass(&class->parent_class);
}
// autobridge
static void bridgeGtkGestureSingleClass(my_GtkGestureSingleClass_t* class)
{
    bridgeGtkGestureClass(&class->parent_class);
}

static void unwrapGtkGestureSingleInstance(my_GtkGestureSingle_t* class)
{
    unwrapGtkGestureInstance(&class->parent);
}
// autobridge
static void bridgeGtkGestureSingleInstance(my_GtkGestureSingle_t* class)
{
    bridgeGtkGestureInstance(&class->parent);
}
// ----- GtkGestureClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkGestureLongPress, pressed, void, (void *gesture, double x, double y), "pdd", gesture, x, y);
WRAPPER(GtkGestureLongPress, cancelled, void, (void *cancelled), "p", cancelled);

#define SUPERGO()              \
    GO(pressed, vFpdd);          \
    GO(cancelled, vFp);

// wrap (so bridge all calls, just in case)
static void wrapGtkGestureLongPressClass(my_GtkGestureLongPressClass_t* class)
{
    wrapGtkGestureSingleClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkGestureLongPress(W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkGestureLongPressClass(my_GtkGestureLongPressClass_t* class)
{
    unwrapGtkGestureSingleClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkGestureLongPress (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkGestureLongPressClass(my_GtkGestureLongPressClass_t* class)
{
    bridgeGtkGestureSingleClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkGestureLongPress (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkGestureLongPressInstance(my_GtkGestureLongPress_t* class)
{
    unwrapGtkGestureSingleInstance(&class->parent);
}
// autobridge
static void bridgeGtkGestureLongPressInstance(my_GtkGestureLongPress_t* class)
{
    bridgeGtkGestureSingleInstance(&class->parent);
}

// ----- AtkObjectClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(AtkObject, get_name, void*, (void* accessible), "p", accessible);
WRAPPER(AtkObject, get_description, void*, (void* accessible), "p", accessible);
WRAPPER(AtkObject, get_parent, void*, (void* accessible), "p", accessible);
WRAPPER(AtkObject, get_n_children, int, (void* accessible), "p", accessible);
WRAPPER(AtkObject, ref_child, void*, (void* accessible, int i), "pi", accessible, i);
WRAPPER(AtkObject, get_index_in_parent, int, (void* accessible), "p", accessible);
WRAPPER(AtkObject, ref_relation_set, void*, (void* accessible), "p", accessible);
WRAPPER(AtkObject, get_role, int, (void* accessible), "p", accessible);
WRAPPER(AtkObject, get_layer, int, (void* accessible), "p", accessible);
WRAPPER(AtkObject, get_mdi_zorder, int, (void* accessible), "p", accessible);
WRAPPER(AtkObject, ref_state_set, void*, (void* accessible), "p", accessible);
WRAPPER(AtkObject, set_name, void, (void* accessible, void* name), "pp", accessible, name);
WRAPPER(AtkObject, set_description, void, (void* accessible, void* description), "pp", accessible, description);
WRAPPER(AtkObject, set_parent, void, (void* accessible, void* parent), "pp", accessible, parent);
WRAPPER(AtkObject, set_role, void, (void* accessible, int role), "pi", accessible, role);
WRAPPER(AtkObject, connect_property_change_handler, uint32_t, (void* accessible, void* handler), "pp", accessible, AddCheckBridge(my_bridge, vFpp, handler, 0, NULL));
WRAPPER(AtkObject, remove_property_change_handler, void, (void* accessible, uint32_t handler_id), "pu", accessible, handler_id);
WRAPPER(AtkObject, initialize, void, (void* accessible, void* data), "pp", accessible, data);
WRAPPER(AtkObject, children_changed, void, (void* accessible, uint32_t change_index, void* changed_child), "pup", accessible, change_index, changed_child);
WRAPPER(AtkObject, focus_event, void, (void* accessible, int focus_in), "pi", accessible, focus_in);
WRAPPER(AtkObject, property_change, void, (void* accessible, void* values), "pp", accessible, values);
WRAPPER(AtkObject, state_change, void, (void* accessible, void* name, int state_set), "ppi", accessible, name, state_set);
WRAPPER(AtkObject, visible_data_changed, void, (void* accessible), "p", accessible);
WRAPPER(AtkObject, active_descendant_changed, void, (void* accessible, void* child), "pp", accessible, child);
WRAPPER(AtkObject, get_attributes, void*, (void* accessible), "p", accessible);
WRAPPER(AtkObject, get_object_locale, void*, (void* accessible), "p", accessible);

#define SUPERGO() \
    GO(get_name, pFp);                          \
    GO(get_description, pFp);                   \
    GO(get_parent, pFp);                        \
    GO(get_n_children, iFp);                    \
    GO(ref_child, pFpi);                        \
    GO(get_index_in_parent, iFp);               \
    GO(ref_relation_set, pFp);                  \
    GO(get_role, iFp);                          \
    GO(get_layer, iFp);                         \
    GO(get_mdi_zorder, iFp);                    \
    GO(ref_state_set, pFp);                     \
    GO(set_name, vFpp);                         \
    GO(set_description, vFpp);                  \
    GO(set_parent, vFpp);                       \
    GO(set_role, vFpi);                         \
    GO(connect_property_change_handler, uFpp);  \
    GO(remove_property_change_handler, vFpu);   \
    GO(initialize, vFpp);                       \
    GO(children_changed, vFpup);                \
    GO(focus_event, vFpi);                      \
    GO(property_change, vFpp);                  \
    GO(state_change, vFppi);                    \
    GO(visible_data_changed, vFp);              \
    GO(active_descendant_changed, vFpp);        \
    GO(get_attributes, pFp);                    \
    GO(get_object_locale, pFp);                 \

// wrap (so bridge all calls, just in case)
static void wrapAtkObjectClass(my_AtkObjectClass_t* class)
{
    wrapGObjectClass(&class->parent);
    #define GO(A, W) class->A = reverse_##A##_AtkObject (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapAtkObjectClass(my_AtkObjectClass_t* class)
{
    unwrapGObjectClass(&class->parent);
    #define GO(A, W)   class->A = find_##A##_AtkObject (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeAtkObjectClass(my_AtkObjectClass_t* class)
{
    bridgeGObjectClass(&class->parent);
    #define GO(A, W) autobridge_##A##_AtkObject (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapAtkObjectInstance(my_AtkObject_t* class)
{
    unwrapGObjectInstance(&class->parent);
}
// autobridge
static void bridgeAtkObjectInstance(my_AtkObject_t* class)
{
    bridgeGObjectInstance(&class->parent);
}

// ----- AtkUtilClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(AtkUtil,add_global_event_listener, uint32_t,(void* listener, void* event_type), "pp", AddCheckBridge(my_bridge, iFpupp, listener, 0, NULL), event_type);
WRAPPER(AtkUtil,remove_global_event_listener, void ,(uint32_t listener_id), "u", listener_id);
WRAPPER(AtkUtil,add_key_event_listener, uint32_t   ,(void* listener, void* data), "pp", AddCheckBridge(my_bridge, iFpp, listener, 0, NULL), data);
WRAPPER(AtkUtil,remove_key_event_listener, void    ,(uint32_t listener_id), "u", listener_id);
WRAPPER(AtkUtil,get_root, void*                    ,(void), "", 0);
WRAPPER(AtkUtil,get_toolkit_name, void*            ,(void), "", 0);
WRAPPER(AtkUtil,get_toolkit_version, void*         ,(void), "", 0);

#define SUPERGO() \
    GO(add_global_event_listener, uFpp);        \
    GO(remove_global_event_listener, vFu);      \
    GO(add_key_event_listener, uFpp);           \
    GO(remove_key_event_listener, vFu);         \
    GO(get_root, pFv);                          \
    GO(get_toolkit_name, pFv);                  \
    GO(get_toolkit_version, pFv);               \

// wrap (so bridge all calls, just in case)
static void wrapAtkUtilClass(my_AtkUtilClass_t* class)
{
    wrapGObjectClass(&class->parent);
    #define GO(A, W) class->A = reverse_##A##_AtkUtil (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapAtkUtilClass(my_AtkUtilClass_t* class)
{
    unwrapGObjectClass(&class->parent);
    #define GO(A, W)   class->A = find_##A##_AtkUtil (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeAtkUtilClass(my_AtkUtilClass_t* class)
{
    bridgeGObjectClass(&class->parent);
    #define GO(A, W) autobridge_##A##_AtkUtil (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapAtkUtilInstance(my_AtkUtil_t* class)
{
    unwrapGObjectInstance(&class->parent);
}
// autobridge
static void bridgeAtkUtilInstance(my_AtkUtil_t* class)
{
    bridgeGObjectInstance(&class->parent);
}

// ----- GstObjectClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstObject, deep_notify, void, (void* object, void* origin, void* pspec), "ppp", object, origin, pspec);

#define SUPERGO() \
    GO(deep_notify, vFppp); \

// wrap (so bridge all calls, just in case)
static void wrapGstObjectClass(my_GstObjectClass_t* class)
{
    wrapGInitiallyUnownedClass(&class->parent);
    #define GO(A, W) class->A = reverse_##A##_GstObject (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstObjectClass(my_GstObjectClass_t* class)
{
    unwrapGInitiallyUnownedClass(&class->parent);
    #define GO(A, W)   class->A = find_##A##_GstObject (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstObjectClass(my_GstObjectClass_t* class)
{
    bridgeGInitiallyUnownedClass(&class->parent);
    #define GO(A, W) autobridge_##A##_GstObject (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGstObjectInstance(my_GstObject_t* class)
{
    unwrapGInitiallyUnownedInstance(&class->parent);
}
// autobridge
static void bridgeGstObjectInstance(my_GstObject_t* class)
{
    bridgeGInitiallyUnownedInstance(&class->parent);
}

// ----- GstAllocatorClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstAllocator, alloc, void*, (void *allocator, size_t size, void *params), "pLp", allocator, size, params);
WRAPPER(GstAllocator,free, void,    (void *allocator, void *memory), "pp", allocator, memory);

#define SUPERGO() \
    GO(alloc, pFpLp);       \
    GO(free, vFpp);         \

// wrap (so bridge all calls, just in case)
static void wrapGstAllocatorClass(my_GstAllocatorClass_t* class)
{
    wrapGstObjectClass(&class->parent);
    #define GO(A, W) class->A = reverse_##A##_GstAllocator (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstAllocatorClass(my_GstAllocatorClass_t* class)
{
    unwrapGstObjectClass(&class->parent);
    #define GO(A, W)   class->A = find_##A##_GstAllocator (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstAllocatorClass(my_GstAllocatorClass_t* class)
{
    bridgeGstObjectClass(&class->parent);
    #define GO(A, W) autobridge_##A##_GstAllocator (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

WRAPPER(GstAllocatorInstance, mem_map, void*,       (void* mem, size_t maxsize, int flags), "pLi", mem, maxsize, flags);
WRAPPER(GstAllocatorInstance, mem_unmap, void ,     (void* mem), "p", mem);
WRAPPER(GstAllocatorInstance, mem_copy, void*,      (void* mem, ssize_t offset, ssize_t size), "pll", mem, offset, size);
WRAPPER(GstAllocatorInstance, mem_share, void*,     (void* mem, ssize_t offset, ssize_t size), "pll", mem, offset, size);
WRAPPER(GstAllocatorInstance, mem_is_span, int  ,   (void* mem1, void* mem2, void* offset), "ppp", mem1, mem2, offset);
WRAPPER(GstAllocatorInstance, mem_map_full, void*,  (void* mem, void* info, size_t maxsize), "ppL", mem, info, maxsize);
WRAPPER(GstAllocatorInstance, mem_unmap_full, void ,(void* mem, void* info), "pp", mem, info);    

#define SUPERGO()               \
    GO(mem_map, pFpLi);         \
    GO(mem_unmap, vFp);         \
    GO(mem_copy, pFpll);        \
    GO(mem_share, pFpll);       \
    GO(mem_is_span, iFppp);     \
    GO(mem_map_full, pFppL);    \
    GO(mem_unmap_full, vFpp);   \

static void unwrapGstAllocatorInstance(my_GstAllocator_t* class)
{
    unwrapGstObjectInstance(&class->parent);
    #define GO(A, W)   class->A = find_##A##_GstAllocatorInstance (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstAllocatorInstance(my_GstAllocator_t* class)
{
    bridgeGstObjectInstance(&class->parent);
    #define GO(A, W) autobridge_##A##_GstAllocatorInstance (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

// ----- GstTaskPoolClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstTaskPool, prepare, void,  (void* pool, void* error), "pp", pool, error);
WRAPPER(GstTaskPool, cleanup, void,  (void* pool), "p", pool);
WRAPPER(GstTaskPool, push, void*,    (void* pool, void* func, void* user_data, void* error), "pppp", pool, AddCheckBridge(my_bridge, vFp, func, 0, NULL), user_data, error);
WRAPPER(GstTaskPool, join, void,     (void* pool, void* id), "pp", pool, id);
WRAPPER(GstTaskPool, dispose_handle, void, (void* pool, void* id), "pp", pool, id);

#define SUPERGO()               \
    GO(prepare, vFpp);          \
    GO(cleanup, vFp);           \
    GO(push, pFpppp);           \
    GO(join, vFpp);             \
    GO(dispose_handle, vFpp);   \

// wrap (so bridge all calls, just in case)
static void wrapGstTaskPoolClass(my_GstTaskPoolClass_t* class)
{
    wrapGstObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstTaskPool (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstTaskPoolClass(my_GstTaskPoolClass_t* class)
{
    unwrapGstObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstTaskPool (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstTaskPoolClass(my_GstTaskPoolClass_t* class)
{
    bridgeGstObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstTaskPool (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGstTaskPoolInstance(my_GstTaskPool_t* class)
{
    unwrapGstObjectInstance(&class->parent);
}
// autobridge
static void bridgeGstTaskPoolInstance(my_GstTaskPool_t* class)
{
    bridgeGstObjectInstance(&class->parent);
}

// ----- GstElementClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstElement,pad_added, void, (void* element, void* pad), "pp", element, pad);
WRAPPER(GstElement,pad_removed, void, (void* element, void* pad), "pp", element, pad);
WRAPPER(GstElement,no_more_pads, void, (void* element), "p", element);
WRAPPER(GstElement,request_new_pad, void*, (void* element, void* templ, void* name, void* caps), "pppp", element, templ, name, caps);
WRAPPER(GstElement,release_pad, void, (void* element, void* pad), "pp", element, pad);
WRAPPER(GstElement,get_state, int, (void*  element, void* state, void* pending, uint64_t timeout), "pppU", element, state, pending, timeout);
WRAPPER(GstElement,set_state, int, (void* element, int state), "pi", element, state);
WRAPPER(GstElement,change_state, int, (void* element, int transition), "pi", element, transition);
WRAPPER(GstElement,state_changed, void, (void* element, int oldstate, int newstate, int pending), "piii", element, oldstate, newstate, pending);
WRAPPER(GstElement,set_bus, void, (void*  element, void* bus), "pp", element, bus);
WRAPPER(GstElement,provide_clock, void*, (void* element), "p", element);
WRAPPER(GstElement,set_clock, int, (void* element, void* clock), "pp", element, clock);
WRAPPER(GstElement,send_event, int, (void* element, void* event), "pp", element, event);
WRAPPER(GstElement,query, int, (void* element, void* query), "pp", element, query);
WRAPPER(GstElement,post_message, int, (void* element, void* message), "pp", element, message);
WRAPPER(GstElement,set_context, void, (void* element, void* context), "pp", element, context);

#define SUPERGO()               \
    GO(pad_added, vFpp);        \
    GO(pad_removed, vFpp);      \
    GO(no_more_pads, vFp);      \
    GO(request_new_pad, pFpppp);\
    GO(release_pad, vFpp);      \
    GO(get_state, iFppU);       \
    GO(set_state, iFpi);        \
    GO(change_state, iFpi);     \
    GO(state_changed, vFpiii);  \
    GO(set_bus, vFpp);          \
    GO(provide_clock, pFp);     \
    GO(set_clock, iFpp);        \
    GO(send_event, iFpp);       \
    GO(query, iFpp);            \
    GO(post_message, iFpp);     \
    GO(set_context, vFpp);      \

// wrap (so bridge all calls, just in case)
static void wrapGstElementClass(my_GstElementClass_t* class)
{
    wrapGstObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstElement (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstElementClass(my_GstElementClass_t* class)
{
    unwrapGstObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstElement (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstElementClass(my_GstElementClass_t* class)
{
    bridgeGstObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstElement (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGstElementInstance(my_GstElement_t* class)
{
    unwrapGstObjectInstance(&class->parent);
}
// autobridge
static void bridgeGstElementInstance(my_GstElement_t* class)
{
    bridgeGstObjectInstance(&class->parent);
}
#undef SUPERGO

// ----- GstBinClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstBin,element_added, void, (void* bin, void* child), "pp", bin, child);
WRAPPER(GstBin,element_removed, void, (void* bin, void* child), "pp", bin, child);
WRAPPER(GstBin,add_element, int, (void* bin, void* element), "pp", bin, element);
WRAPPER(GstBin,remove_element, int, (void* bin, void* element), "pp", bin, element);
WRAPPER(GstBin,handle_message, void, (void* bin, void* message), "pp", bin, message);
WRAPPER(GstBin,do_latency, int, (void* bin), "p", bin);
WRAPPER(GstBin,deep_element_added, void, (void* bin, void* sub_bin, void* child), "ppp", bin, sub_bin, child);
WRAPPER(GstBin,deep_element_removed, void, (void* bin, void* sub_bin, void* child), "ppp", bin, sub_bin, child);

#define SUPERGO()                   \
    GO(element_added, vFpp);        \
    GO(element_removed, vFpp);      \
    GO(add_element, iFpp);          \
    GO(remove_element, iFpp);       \
    GO(handle_message, vFpp);       \
    GO(do_latency, iFp);            \
    GO(deep_element_added, vFppp);  \
    GO(deep_element_removed, vFppp);\

// wrap (so bridge all calls, just in case)
static void wrapGstBinClass(my_GstBinClass_t* class)
{
    wrapGstElementClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstBin (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstBinClass(my_GstBinClass_t* class)
{
    unwrapGstElementClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstBin (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstBinClass(my_GstBinClass_t* class)
{
    bridgeGstElementClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstBin (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGstBinInstance(my_GstBin_t* class)
{
    unwrapGstElementInstance(&class->parent);
}
// autobridge
static void bridgeGstBinInstance(my_GstBin_t* class)
{
    bridgeGstElementInstance(&class->parent);
}

// ----- GstBaseTransformClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstBaseTransform, transform_caps, void*, (void* trans, int direction, void* caps, void* filter), "pipp", trans, direction, caps, filter);
WRAPPER(GstBaseTransform, fixate_caps, void*, (void* trans, int direction, void* caps, void* othercaps), "pipp", trans, direction, caps, othercaps);
WRAPPER(GstBaseTransform, accept_caps, int, (void* trans, int direction, void* caps), "pip", trans, direction, caps);
WRAPPER(GstBaseTransform, set_caps, int, (void* trans, void* incaps, void* outcaps), "ppp", trans, incaps, outcaps);
WRAPPER(GstBaseTransform, query, int, (void* trans, int direction, void* query), "pip", trans, direction, query);
WRAPPER(GstBaseTransform, decide_allocation, int, (void* trans, void* query), "pp", trans, query );
WRAPPER(GstBaseTransform, filter_meta, int, (void* trans, void* query, size_t api, void* params), "ppLp", trans, query, api, params);
WRAPPER(GstBaseTransform, propose_allocation, int, (void* trans, void* decide_query, void* query), "ppp", trans, decide_query, query );
WRAPPER(GstBaseTransform, transform_size, int, (void* trans, int direction, void* caps, size_t size, void* othercaps, void* othersize), "pipLpp", trans, direction, caps, size, othercaps, othersize);
WRAPPER(GstBaseTransform, get_unit_size, int, (void* trans, void* caps, void* size), "ppp", trans, caps, size);
WRAPPER(GstBaseTransform, start, int, (void* trans), "p", trans);
WRAPPER(GstBaseTransform, stop, int, (void* trans), "p", trans);
WRAPPER(GstBaseTransform, sink_event, int, (void* trans, void* event), "pp", trans, event );
WRAPPER(GstBaseTransform, src_event, int, (void* trans, void* event), "pp", trans, event );
WRAPPER(GstBaseTransform, prepare_output_buffer, int, (void*  trans, void* input, void* outbuf), "ppp", trans, input, outbuf );
WRAPPER(GstBaseTransform, copy_metadata, int, (void* trans, void* input, void* outbuf), "ppp", trans, input, outbuf );
WRAPPER(GstBaseTransform, transform_meta, int, (void* trans, void* outbuf, void* meta, void* inbuf), "pppp", trans, outbuf, meta, inbuf );
WRAPPER(GstBaseTransform, before_transform, void, (void* trans, void* buffer), "pp", trans, buffer );
WRAPPER(GstBaseTransform, transform, int, (void* trans, void* inbuf, void* outbuf), "ppp", trans, inbuf, outbuf );
WRAPPER(GstBaseTransform, transform_ip, int, (void* trans, void* buf), "pp", trans, buf );
WRAPPER(GstBaseTransform, submit_input_buffer, int, (void* trans, int is_discont, void* input), "pip", trans, is_discont, input );
WRAPPER(GstBaseTransform, generate_output, int, (void* trans, void* outbuf), "pp", trans, outbuf );

#define SUPERGO()                       \
    GO(transform_caps, pFpipp);         \
    GO(fixate_caps, pFpipp);            \
    GO(accept_caps, iFpip);             \
    GO(set_caps, iFppp);                \
    GO(query, iFpip);                   \
    GO(decide_allocation, iFpp);        \
    GO(filter_meta, iFppLp);            \
    GO(propose_allocation, iFppp);      \
    GO(transform_size, iFpipLpp);       \
    GO(get_unit_size, iFppp);           \
    GO(start, iFp);                     \
    GO(stop, iFp);                      \
    GO(sink_event, iFpp);               \
    GO(src_event, iFpp);                \
    GO(prepare_output_buffer, iFppp);   \
    GO(copy_metadata, iFppp);           \
    GO(transform_meta, iFpppp);         \
    GO(before_transform, vFpp);         \
    GO(transform, iFppp);               \
    GO(transform_ip, iFpp);             \
    GO(submit_input_buffer, iFpip);     \
    GO(generate_output, iFpp);          \

// wrap (so bridge all calls, just in case)
static void wrapGstBaseTransformClass(my_GstBaseTransformClass_t* class)
{
    wrapGstElementClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstBaseTransform (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstBaseTransformClass(my_GstBaseTransformClass_t* class)
{
    unwrapGstElementClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstBaseTransform (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstBaseTransformClass(my_GstBaseTransformClass_t* class)
{
    bridgeGstElementClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstBaseTransform (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGstBaseTransformInstance(my_GstBaseTransform_t* class)
{
    unwrapGstElementInstance(&class->parent);
}
// autobridge
static void bridgeGstBaseTransformInstance(my_GstBaseTransform_t* class)
{
    bridgeGstElementInstance(&class->parent);
}
// ----- GstVideoDecoderClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstVideoDecoder, open, int, (void* decoder), "p", decoder);
WRAPPER(GstVideoDecoder, close, int, (void* decoder), "p", decoder);
WRAPPER(GstVideoDecoder, start, int, (void* decoder), "p", decoder);
WRAPPER(GstVideoDecoder, stop, int, (void* decoder), "p", decoder);
WRAPPER(GstVideoDecoder, parse, int, (void* decoder, void* frame, void* adapter, int at_eos), "pppi", decoder, frame, adapter, at_eos);
WRAPPER(GstVideoDecoder, set_format, int, (void* decoder, void* state), "pp", decoder, state);
WRAPPER(GstVideoDecoder, reset, int, (void* decoder, int hard), "pi", decoder, hard);
WRAPPER(GstVideoDecoder, finish, int, (void* decoder), "p", decoder);
WRAPPER(GstVideoDecoder, handle_frame, int, (void* decoder, void* frame), "pp", decoder, frame);
WRAPPER(GstVideoDecoder, sink_event, int, (void* decoder, void* event), "pp", decoder, event);
WRAPPER(GstVideoDecoder, src_event, int, (void* decoder, void* event), "pp", decoder, event);
WRAPPER(GstVideoDecoder, negotiate, int, (void* decoder), "p", decoder);
WRAPPER(GstVideoDecoder, decide_allocation, int, (void* decoder, void* query), "pp", decoder, query);
WRAPPER(GstVideoDecoder, propose_allocation, int, (void* decoder, void*  query), "pp", decoder, query);
WRAPPER(GstVideoDecoder, flush, int, (void* decoder), "p", decoder);
WRAPPER(GstVideoDecoder, sink_query, int, (void* decoder, void* query), "pp", decoder, query);
WRAPPER(GstVideoDecoder, src_query, int, (void* decoder, void* query), "pp", decoder, query);
WRAPPER(GstVideoDecoder, getcaps, void*, (void* decoder, void* filter), "pp", decoder, filter);
WRAPPER(GstVideoDecoder, drain, int, (void* decoder), "p", decoder);
WRAPPER(GstVideoDecoder, transform_meta, int, (void* decoder, void* frame, void* meta), "ppp", decoder, frame, meta);
WRAPPER(GstVideoDecoder, handle_missing_data, int, (void* decoder, uint64_t timestamp, uint64_t duration), "pUU", decoder, timestamp, duration);

#define SUPERGO()                       \
    GO(open, iFp);                      \
    GO(close, iFp);                     \
    GO(start, iFp);                     \
    GO(stop, iFp);                      \
    GO(parse, iFpppi);                  \
    GO(set_format, iFpp);               \
    GO(reset, iFp);                     \
    GO(finish, iFp);                    \
    GO(handle_frame, iFpp);             \
    GO(sink_event, iFpp);               \
    GO(src_event, iFpp);                \
    GO(negotiate, iFp);                 \
    GO(decide_allocation, iFpp);        \
    GO(propose_allocation, iFpp);       \
    GO(flush, iFp);                     \
    GO(sink_query, iFpp);               \
    GO(src_query, iFpp);                \
    GO(getcaps, pFpp);                  \
    GO(drain, iFp);                     \
    GO(transform_meta, iFppp);          \
    GO(handle_missing_data, iFpUU);     \

// wrap (so bridge all calls, just in case)
static void wrapGstVideoDecoderClass(my_GstVideoDecoderClass_t* class)
{
    wrapGstElementClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstVideoDecoder (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstVideoDecoderClass(my_GstVideoDecoderClass_t* class)
{
    unwrapGstElementClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstVideoDecoder (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstVideoDecoderClass(my_GstVideoDecoderClass_t* class)
{
    bridgeGstElementClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstVideoDecoder (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGstVideoDecoderInstance(my_GstVideoDecoder_t* class)
{
    unwrapGstElementInstance(&class->parent);
}
// autobridge
static void bridgeGstVideoDecoderInstance(my_GstVideoDecoder_t* class)
{
    bridgeGstElementInstance(&class->parent);
}
// ----- GstVideoEncoderClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstVideoEncoder, open, int, (void* encoder), "p", encoder);
WRAPPER(GstVideoEncoder, close, int, (void* encoder), "p", encoder);
WRAPPER(GstVideoEncoder, start, int, (void* encoder), "p", encoder);
WRAPPER(GstVideoEncoder, stop, int, (void* encoder), "p", encoder);
WRAPPER(GstVideoEncoder, set_format, int, (void* encoder, void* state), "pp", encoder, state);
WRAPPER(GstVideoEncoder, handle_frame, int, (void* encoder, void* frame), "pp", encoder, frame);
WRAPPER(GstVideoEncoder, reset, int, (void* encoder, int hard), "pi", encoder, hard);
WRAPPER(GstVideoEncoder, finish, int, (void* encoder), "p", encoder);
WRAPPER(GstVideoEncoder, pre_push, int, (void* encoder, void* frame), "pp", encoder, frame);
WRAPPER(GstVideoEncoder, getcaps, void*, (void* encoder, void* filter), "pp", encoder, filter);
WRAPPER(GstVideoEncoder, sink_event, int, (void* encoder, void* event), "pp", encoder, event);
WRAPPER(GstVideoEncoder, src_event, int, (void* encoder, void* event), "pp", encoder, event);
WRAPPER(GstVideoEncoder, negotiate, int, (void* encoder), "p", encoder);
WRAPPER(GstVideoEncoder, decide_allocation, int, (void* encoder, void* query), "pp", encoder, query);
WRAPPER(GstVideoEncoder, propose_allocation, int, (void* encoder, void*  query), "pp", encoder, query);
WRAPPER(GstVideoEncoder, flush, int, (void* encoder), "p", encoder);
WRAPPER(GstVideoEncoder, sink_query, int, (void* encoder, void* query), "pp", encoder, query);
WRAPPER(GstVideoEncoder, src_query, int, (void* encoder, void* query), "pp", encoder, query);
WRAPPER(GstVideoEncoder, transform_meta, int, (void* encoder, void* frame, void* meta), "ppp", encoder, frame, meta);

#define SUPERGO()                       \
    GO(open, iFp);                      \
    GO(close, iFp);                     \
    GO(start, iFp);                     \
    GO(stop, iFp);                      \
    GO(set_format, iFpp);               \
    GO(handle_frame, iFpp);             \
    GO(reset, iFp);                     \
    GO(finish, iFp);                    \
    GO(pre_push, iFpp);                 \
    GO(getcaps, pFpp);                  \
    GO(sink_event, iFpp);               \
    GO(src_event, iFpp);                \
    GO(negotiate, iFp);                 \
    GO(decide_allocation, iFpp);        \
    GO(propose_allocation, iFpp);       \
    GO(flush, iFp);                     \
    GO(sink_query, iFpp);               \
    GO(src_query, iFpp);                \
    GO(transform_meta, iFppp);          \

// wrap (so bridge all calls, just in case)
static void wrapGstVideoEncoderClass(my_GstVideoEncoderClass_t* class)
{
    wrapGstElementClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstVideoEncoder (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstVideoEncoderClass(my_GstVideoEncoderClass_t* class)
{
    unwrapGstElementClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstVideoEncoder (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstVideoEncoderClass(my_GstVideoEncoderClass_t* class)
{
    bridgeGstElementClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstVideoEncoder (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGstVideoEncoderInstance(my_GstVideoEncoder_t* class)
{
    unwrapGstElementInstance(&class->parent);
}
// autobridge
static void bridgeGstVideoEncoderInstance(my_GstVideoEncoder_t* class)
{
    bridgeGstElementInstance(&class->parent);
}
// ----- GstBaseSinkClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstBaseSink, get_caps, void*, (void* sink, void* filter), "pp", sink, filter);
WRAPPER(GstBaseSink, set_caps, int, (void* sink, void* caps), "pp", sink, caps);
WRAPPER(GstBaseSink, fixate, void* , (void* sink, void* caps), "pp", sink, caps);
WRAPPER(GstBaseSink, activate_pull, int, (void* sink, int active), "pi", sink, active);
WRAPPER(GstBaseSink, get_times, void, (void* sink, void* buffer, void* start, void* end), "pppp", sink, buffer, start, end);
WRAPPER(GstBaseSink, propose_allocation, int, (void* sink, void* query), "pp", sink, query);
WRAPPER(GstBaseSink, start, int, (void* sink), "p", sink);
WRAPPER(GstBaseSink, stop, int, (void* sink), "p", sink);
WRAPPER(GstBaseSink, unlock, int, (void* sink), "p", sink);
WRAPPER(GstBaseSink, unlock_stop, int, (void* sink), "p", sink);
WRAPPER(GstBaseSink, query, int, (void* sink, void* query), "pp", sink, query);
WRAPPER(GstBaseSink, event, int, (void* sink, void* event), "pp", sink, event);
WRAPPER(GstBaseSink, wait_event, int, (void* sink, void* event), "pp", sink, event);
WRAPPER(GstBaseSink, prepare, int, (void* sink, void* buffer), "pp", sink, buffer);
WRAPPER(GstBaseSink, prepare_list, int, (void* sink, void* buffer_list), "pp", sink, buffer_list);
WRAPPER(GstBaseSink, preroll, int, (void* sink, void* buffer), "pp", sink, buffer);
WRAPPER(GstBaseSink, render, int, (void* sink, void* buffer), "pp", sink, buffer);
WRAPPER(GstBaseSink, render_list, int, (void* sink, void* buffer_list), "pp", sink, buffer_list);

#define SUPERGO()                       \
    GO(get_caps, pFpp);                 \
    GO(set_caps, iFpp);                 \
    GO(fixate, pFpp);                   \
    GO(activate_pull, iFpi);            \
    GO(get_times, vFpppp);              \
    GO(propose_allocation, iFpp);       \
    GO(start, iFp);                     \
    GO(stop, iFp);                      \
    GO(unlock, iFp);                    \
    GO(unlock_stop, iFp);               \
    GO(query, iFpp);                    \
    GO(event, iFpp);                    \
    GO(wait_event, iFpp);               \
    GO(prepare, iFpp);                  \
    GO(prepare_list, iFpp);             \
    GO(preroll, iFpp);                  \
    GO(render, iFpp);                   \
    GO(render_list, iFpp);              \

// wrap (so bridge all calls, just in case)
static void wrapGstBaseSinkClass(my_GstBaseSinkClass_t* class)
{
    wrapGstElementClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstBaseSink (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstBaseSinkClass(my_GstBaseSinkClass_t* class)
{
    unwrapGstElementClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstBaseSink (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstBaseSinkClass(my_GstBaseSinkClass_t* class)
{
    bridgeGstElementClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstBaseSink (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstBaseSinkInstance(my_GstBaseSink_t* class)
{
    unwrapGstElementInstance(&class->parent);
}
// autobridge
static void bridgeGstBaseSinkInstance(my_GstBaseSink_t* class)
{
    bridgeGstElementInstance(&class->parent);
}
// ----- GstVideoSinkClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstVideoSink, show_frame, int, (void* video_sink, void* buf), "pp", video_sink, buf);
WRAPPER(GstVideoSink, set_info, int, (void* video_sink, void* caps, void* info), "ppp", video_sink, caps, info);

#define SUPERGO()                       \
    GO(show_frame, iFpp);               \
    GO(set_info, iFppp);                \

// wrap (so bridge all calls, just in case)
static void wrapGstVideoSinkClass(my_GstVideoSinkClass_t* class)
{
    wrapGstBaseSinkClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstVideoSink (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstVideoSinkClass(my_GstVideoSinkClass_t* class)
{
    unwrapGstBaseSinkClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstVideoSink (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstVideoSinkClass(my_GstVideoSinkClass_t* class)
{
    bridgeGstBaseSinkClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstVideoSink (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstVideoSinkInstance(my_GstVideoSink_t* class)
{
    unwrapGstBaseSinkInstance(&class->parent);
}
// autobridge
static void bridgeGstVideoSinkInstance(my_GstVideoSink_t* class)
{
    bridgeGstBaseSinkInstance(&class->parent);
}
// ----- GstGLBaseFilterClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstGLBaseFilter, gl_start, int, (void* filter), "p", filter);
WRAPPER(GstGLBaseFilter, gl_stop, void, (void* filter), "p", filter);
WRAPPER(GstGLBaseFilter, gl_set_caps, int, (void* filter, void* incaps, void* outcaps), "ppp", filter, incaps, outcaps);

#define SUPERGO()                       \
    GO(gl_start, iFp);                  \
    GO(gl_stop, vFp);                   \
    GO(gl_set_caps, iFppp);             \

// wrap (so bridge all calls, just in case)
static void wrapGstGLBaseFilterClass(my_GstGLBaseFilterClass_t* class)
{
    wrapGstBaseTransformClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstGLBaseFilter (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstGLBaseFilterClass(my_GstGLBaseFilterClass_t* class)
{
    unwrapGstBaseTransformClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstGLBaseFilter (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstGLBaseFilterClass(my_GstGLBaseFilterClass_t* class)
{
    bridgeGstBaseTransformClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstGLBaseFilter (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstGLBaseFilterInstance(my_GstGLBaseFilter_t* class)
{
    unwrapGstBaseTransformInstance(&class->parent);
}
// autobridge
static void bridgeGstGLBaseFilterInstance(my_GstGLBaseFilter_t* class)
{
    bridgeGstBaseTransformInstance(&class->parent);
}
// ----- GstGLFilterClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstGLFilter, set_caps, int, (void* filter, void* incaps, void* outcaps), "ppp", filter, incaps, outcaps);
WRAPPER(GstGLFilter, filter, int, (void* filter, void* inbuf, void* outbuf), "ppp", filter, inbuf, outbuf);
WRAPPER(GstGLFilter, filter_texture, int, (void* filter, void* input, void* output), "ppp", filter, input, output);
WRAPPER(GstGLFilter, init_fbo, int, (void* filter), "p", filter);
WRAPPER(GstGLFilter, transform_internal_caps, void*, (void* filter, int direction, void* caps, void* filter_caps), "pipp", filter, direction, caps, filter_caps);

#define SUPERGO()                       \
    GO(set_caps, iFppp);                \
    GO(filter, iFppp);                  \
    GO(filter_texture, iFppp);          \
    GO(init_fbo, iFp);                  \
    GO(transform_internal_caps, pFpipp);\

// wrap (so bridge all calls, just in case)
static void wrapGstGLFilterClass(my_GstGLFilterClass_t* class)
{
    wrapGstGLBaseFilterClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstGLFilter (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstGLFilterClass(my_GstGLFilterClass_t* class)
{
    unwrapGstGLBaseFilterClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstGLFilter (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstGLFilterClass(my_GstGLFilterClass_t* class)
{
    bridgeGstGLBaseFilterClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstGLFilter (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstGLFilterInstance(my_GstGLFilter_t* class)
{
    unwrapGstGLBaseFilterInstance(&class->parent);
}
// autobridge
static void bridgeGstGLFilterInstance(my_GstGLFilter_t* class)
{
    bridgeGstGLBaseFilterInstance(&class->parent);
}
// ----- GstAggregatorClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstAggregator, flush, int, (void* self), "p", self);
WRAPPER(GstAggregator, clip, void*, (void* self, void* aggregator_pad, void* buf), "ppp", self, aggregator_pad, buf);
WRAPPER(GstAggregator, finish_buffer, int, (void* self, void* buffer), "pp", self, buffer);
WRAPPER(GstAggregator, sink_event, int, (void* self, void* aggregator_pad, void* event), "ppp", self, aggregator_pad, event);
WRAPPER(GstAggregator, sink_query, int, (void* self, void* aggregator_pad, void* query), "ppp", self, aggregator_pad, query);
WRAPPER(GstAggregator, src_event, int, (void* self, void* event), "pp", self, event);
WRAPPER(GstAggregator, src_query, int, (void* self, void* query), "pp", self, query);
WRAPPER(GstAggregator, src_activate, int, (void* self, int mode, int active), "pii", self, mode, active);
WRAPPER(GstAggregator, aggregate, int, (void* self, int timeout), "pi", self, timeout);
WRAPPER(GstAggregator, stop, int, (void* self), "p", self);
WRAPPER(GstAggregator, start, int, (void* self), "p", self);
WRAPPER(GstAggregator, get_next_time, uint64_t, (void* self), "p", self);
WRAPPER(GstAggregator, create_new_pad, void*, (void* self, void* templ, void* req_name, void* caps), "pppp", self, templ, req_name, caps);
WRAPPER(GstAggregator, update_src_caps, int, (void* self, void* caps, void* ret), "ppp", self, caps, ret);
WRAPPER(GstAggregator, fixate_src_caps, void*, (void* self, void* caps), "pp", self, caps);
WRAPPER(GstAggregator, negotiated_src_caps, int, (void* self, void* caps), "pp", self, caps);
WRAPPER(GstAggregator, decide_allocation, int, (void* self, void* query), "pp", self, query);
WRAPPER(GstAggregator, propose_allocation, int, (void* self, void* pad, void* decide_query, void* query), "pppp", self, pad, decide_query, query);
WRAPPER(GstAggregator, negotiate, int, (void* self), "p", self);
WRAPPER(GstAggregator, sink_event_pre_queue, int, (void* self, void* aggregator_pad, void* event), "ppp", self, aggregator_pad, event);
WRAPPER(GstAggregator, sink_query_pre_queue, int, (void* self, void* aggregator_pad, void* query), "ppp", self, aggregator_pad, query);
WRAPPER(GstAggregator, finish_buffer_list, int, (void* self, void* bufferlist), "pp", self, bufferlist);
WRAPPER(GstAggregator, peek_next_sample, void, (void* self, void* aggregator_pad), "pp", self, aggregator_pad);

#define SUPERGO()                       \
    GO(flush, iFp);                     \
    GO(clip, pFppp);                    \
    GO(finish_buffer, iFpp);            \
    GO(sink_event, iFppp);              \
    GO(sink_query, iFppp);              \
    GO(src_event, iFpp);                \
    GO(src_query, iFpp);                \
    GO(src_activate, iFpii);            \
    GO(aggregate, iFpi);                \
    GO(stop, iFp);                      \
    GO(start, iFp);                     \
    GO(get_next_time, UFp);             \
    GO(create_new_pad, pFpppp);         \
    GO(update_src_caps, iFppp);         \
    GO(fixate_src_caps, pFpp);          \
    GO(negotiated_src_caps, iFpp);      \
    GO(decide_allocation, iFpp);        \
    GO(propose_allocation, iFpppp);     \
    GO(negotiate, iFp);                 \
    GO(sink_event_pre_queue, iFppp);    \
    GO(sink_query_pre_queue, iFppp);    \
    GO(finish_buffer_list, iFpp);       \
    GO(peek_next_sample, vFpp);         \

// wrap (so bridge all calls, just in case)
static void wrapGstAggregatorClass(my_GstAggregatorClass_t* class)
{
    wrapGstElementClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstAggregator (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstAggregatorClass(my_GstAggregatorClass_t* class)
{
    unwrapGstElementClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstAggregator (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstAggregatorClass(my_GstAggregatorClass_t* class)
{
    bridgeGstElementClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstAggregator (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstAggregatorInstance(my_GstAggregator_t* class)
{
    unwrapGstElementInstance(&class->parent);
}
// autobridge
static void bridgeGstAggregatorInstance(my_GstAggregator_t* class)
{
    bridgeGstElementInstance(&class->parent);
}
// ----- GstVideoAggregatorClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstVideoAggregator, update_caps, void*, (void* vagg, void* caps), "pp", vagg, caps);
WRAPPER(GstVideoAggregator, aggregate_frames, int, (void* vagg, void* outbuffer), "pp", vagg, outbuffer);
WRAPPER(GstVideoAggregator, create_output_buffer, int, (void* vagg, void* outbuffer), "pp", vagg, outbuffer);
WRAPPER(GstVideoAggregator, find_best_format, void, (void* vagg, void* downstream_caps, void* best_info, void* at_least_one_alpha), "pppp", vagg, downstream_caps, best_info, at_least_one_alpha);

#define SUPERGO()                       \
    GO(update_caps, pFpp);              \
    GO(aggregate_frames, iFpp);         \
    GO(create_output_buffer, iFpp);     \
    GO(find_best_format, vFpppp);       \

// wrap (so bridge all calls, just in case)
static void wrapGstVideoAggregatorClass(my_GstVideoAggregatorClass_t* class)
{
    wrapGstAggregatorClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstVideoAggregator (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstVideoAggregatorClass(my_GstVideoAggregatorClass_t* class)
{
    unwrapGstAggregatorClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstVideoAggregator (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstVideoAggregatorClass(my_GstVideoAggregatorClass_t* class)
{
    bridgeGstAggregatorClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstVideoAggregator (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstVideoAggregatorInstance(my_GstVideoAggregator_t* class)
{
    unwrapGstAggregatorInstance(&class->aggregator);
}
// autobridge
static void bridgeGstVideoAggregatorInstance(my_GstVideoAggregator_t* class)
{
    bridgeGstAggregatorInstance(&class->aggregator);
}
// ----- GstPadClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstPad, linked, void, (void* pad, void* peer), "pp", pad, peer);
WRAPPER(GstPad, unlinked, void, (void* pad, void* peer), "pp", pad, peer);

#define SUPERGO()               \
    GO(linked, vFpp);           \
    GO(unlinked, vFpp);         \

// wrap (so bridge all calls, just in case)
static void wrapGstPadClass(my_GstPadClass_t* class)
{
    wrapGstObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstPad (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstPadClass(my_GstPadClass_t* class)
{
    unwrapGstObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstPad (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstPadClass(my_GstPadClass_t* class)
{
    bridgeGstObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstPad (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO
WRAPPER(GstPadInstance, activatefunc, int    , (void* pad, void* parent), "pp", pad, parent);
WRAPPER(GstPadInstance, activatenotify, void   , (void* a), "p", a);
WRAPPER(GstPadInstance, activatemodefunc, int    , (void* pad, void* parent, int mode, int active), "ppii", pad, parent, mode, active);
WRAPPER(GstPadInstance, activatemodenotify, void   , (void* a), "p", a);
WRAPPER(GstPadInstance, linkfunc, int    , (void* pad, void* parent, void* peer), "ppp", pad, parent, peer);
WRAPPER(GstPadInstance, linknotify, void   , (void* a), "p", a);
WRAPPER(GstPadInstance, unlinkfunc, void   , (void* pad, void* parent), "pp", pad, parent);
WRAPPER(GstPadInstance, unlinknotify, void   , (void* a), "p", a);
WRAPPER(GstPadInstance, chainfunc, int    , (void* pad, void* parent, void* buffer), "ppp", pad, parent, buffer);
WRAPPER(GstPadInstance, chainnotify, void   , (void* a), "p", a);
WRAPPER(GstPadInstance, chainlistfunc, int    , (void* pad, void* parent, void* list), "ppp", pad, parent, list);
WRAPPER(GstPadInstance, chainlistnotify, void   , (void* a), "p", a);
WRAPPER(GstPadInstance, getrangefunc, int    , (void* pad, void* parent, uint64_t offset, uint32_t length, void* buffer), "ppUup", pad, parent, offset, length, buffer);
WRAPPER(GstPadInstance, getrangenotify, void   , (void* a), "p", a);
WRAPPER(GstPadInstance, eventfunc, int    , (void* pad, void* parent, void* event), "ppp", pad, parent, event);
WRAPPER(GstPadInstance, eventnotify, void   , (void* a), "p", a);
WRAPPER(GstPadInstance, queryfunc, int    , (void* pad, void* parent, void* query), "ppp", pad, parent, query);
WRAPPER(GstPadInstance, querynotify, void   , (void* a), "p", a);
WRAPPER(GstPadInstance, iterintlinkfunc, void*  , (void* pad, void* parent), "pp", pad, parent);
WRAPPER(GstPadInstance, iterintlinknotify, void   , (void* a), "p", a);
WRAPPER(GstPadInstance, finalize_hook, void   , (void* hook_list, void* hook), "pp", hook_list, hook);
WRAPPER(GstPadInstance, eventfullfunc, int    , (void* pad, void* parent, void* event), "ppp", pad, parent, event);

#define SUPERGO()                       \
    GO(activatefunc, iFpp);             \
    GO(activatenotify, vFp);            \
    GO(activatemodefunc, iFppii);       \
    GO(activatemodenotify, vFp);        \
    GO(linkfunc, iFppp);                \
    GO(linknotify, vFp);                \
    GO(unlinkfunc, vFpp);               \
    GO(unlinknotify, vFp);              \
    GO(chainfunc, iFppp);               \
    GO(chainnotify, vFp);               \
    GO(chainlistfunc, iFppp);           \
    GO(chainlistnotify, vFp);           \
    GO(getrangefunc, iFppUup);          \
    GO(getrangenotify, vFp);            \
    GO(eventfunc, iFppp);               \
    GO(eventnotify, vFp);               \
    GO(queryfunc, iFppp);               \
    GO(querynotify, vFp);               \
    GO(iterintlinkfunc, pFpp);          \
    GO(iterintlinknotify, vFp);         \
    GO2(probes, finalize_hook, vFpp);   \
    GO2(ABI.abi, eventfullfunc, vFppp); \

static void unwrapGstPadInstance(my_GstPad_t* class)
{
    unwrapGstObjectInstance(&class->parent);
    #define GO(A, W)   class->A = find_##A##_GstPadInstance (W, class->A)
    #define GO2(B, A, W)   class->B.A = find_##A##_GstPadInstance (W, class->B.A)
    SUPERGO()
    #undef GO2
    #undef GO
}
// autobridge
static void bridgeGstPadInstance(my_GstPad_t* class)
{
    bridgeGstObjectInstance(&class->parent);
    #define GO(A, W) autobridge_##A##_GstPadInstance (W, class->A)
    #define GO2(B, A, W) autobridge_##A##_GstPadInstance (W, class->B.A)
    SUPERGO()
    #undef GO2
    #undef GO
}
#undef SUPERGO
// ----- GstAggregatorPadClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstAggregatorPad, flush, int, (void* aggpad, void* aggregator), "pp", aggpad, aggregator);
WRAPPER(GstAggregatorPad, skip_buffer, int, (void* aggpad, void* aggregator, void* buffer), "ppp", aggpad, aggregator, buffer);

#define SUPERGO()                       \
    GO(flush, iFpp);                    \
    GO(skip_buffer, iFppp);             \

// wrap (so bridge all calls, just in case)
static void wrapGstAggregatorPadClass(my_GstAggregatorPadClass_t* class)
{
    wrapGstPadClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstAggregatorPad (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstAggregatorPadClass(my_GstAggregatorPadClass_t* class)
{
    unwrapGstPadClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstAggregatorPad (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstAggregatorPadClass(my_GstAggregatorPadClass_t* class)
{
    bridgeGstPadClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstAggregatorPad (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstAggregatorPadInstance(my_GstAggregatorPad_t* class)
{
    unwrapGstPadInstance(&class->parent);
}
// autobridge
static void bridgeGstAggregatorPadInstance(my_GstAggregatorPad_t* class)
{
    bridgeGstPadInstance(&class->parent);
}
// ----- GstVideoAggregatorPadClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstVideoAggregatorPad, update_conversion_info, void, (void* pad), "p", pad);
WRAPPER(GstVideoAggregatorPad, prepare_frame, int, (void* pad, void* vagg, void* buffer, void* prepared_frame), "pppp", pad, vagg, buffer, prepared_frame);
WRAPPER(GstVideoAggregatorPad, clean_frame, void, (void* pad, void* vagg, void* prepared_frame), "ppp", pad, vagg, prepared_frame);
WRAPPER(GstVideoAggregatorPad, prepare_frame_start, void, (void* pad, void* vagg, void* buffer, void* prepared_frame), "pppp", pad, vagg, buffer, prepared_frame);
WRAPPER(GstVideoAggregatorPad, prepare_frame_finish, void, (void* pad, void* vagg, void* prepared_frame), "ppp", pad, vagg, prepared_frame);

#define SUPERGO()                       \
    GO(update_conversion_info, vFp);    \
    GO(prepare_frame, iFpppp);          \
    GO(clean_frame, vFppp);             \
    GO(prepare_frame_start, vFpppp);    \
    GO(prepare_frame_finish, vFppp);    \

// wrap (so bridge all calls, just in case)
static void wrapGstVideoAggregatorPadClass(my_GstVideoAggregatorPadClass_t* class)
{
    wrapGstAggregatorPadClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstVideoAggregatorPad (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstVideoAggregatorPadClass(my_GstVideoAggregatorPadClass_t* class)
{
    unwrapGstAggregatorPadClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstVideoAggregatorPad (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstVideoAggregatorPadClass(my_GstVideoAggregatorPadClass_t* class)
{
    bridgeGstAggregatorPadClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstVideoAggregatorPad (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstVideoAggregatorPadInstance(my_GstVideoAggregatorPad_t* class)
{
    unwrapGstAggregatorPadInstance(&class->parent);
}
// autobridge
static void bridgeGstVideoAggregatorPadInstance(my_GstVideoAggregatorPad_t* class)
{
    bridgeGstAggregatorPadInstance(&class->parent);
}
#undef SUPERGO
// ----- GstBaseSrcClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstBaseSrc, get_caps, void*, (void* src, void* filter), "pp", src, filter);
WRAPPER(GstBaseSrc, negotiate, int, (void* src), "p", src);
WRAPPER(GstBaseSrc, fixate, void*, (void* src, void* caps), "pp", src, caps);
WRAPPER(GstBaseSrc, set_caps, int, (void* src, void* caps), "pp", src, caps);
WRAPPER(GstBaseSrc, decide_allocation, int, (void* src, void* query), "pp", src, query);
WRAPPER(GstBaseSrc, start, int, (void* src), "p", src);
WRAPPER(GstBaseSrc, stop, int, (void* src), "p", src);
WRAPPER(GstBaseSrc, get_times, void , (void* src, void* buffer, void* start, void* end), "pppp", src, buffer, start, end);
WRAPPER(GstBaseSrc, get_size, int, (void* src, void* size), "pp", src, size);
WRAPPER(GstBaseSrc, is_seekable, int, (void* src), "p", src);
WRAPPER(GstBaseSrc, prepare_seek_segment, int, (void* src, void* seek, void* segment), "ppp", src, seek, segment);
WRAPPER(GstBaseSrc, do_seek, int, (void* src, void* segment), "pp", src, segment);
WRAPPER(GstBaseSrc, unlock, int, (void* src), "p", src);
WRAPPER(GstBaseSrc, unlock_stop, int, (void* src), "p", src);
WRAPPER(GstBaseSrc, query, int, (void* src, void* query), "pp", src, query);
WRAPPER(GstBaseSrc, event, int, (void* src, void* event), "pp", src, event);
WRAPPER(GstBaseSrc, create, int, (void* src, uint64_t offset, uint32_t size, void* buf), "pUup", src, offset, size, buf);
WRAPPER(GstBaseSrc, alloc, int, (void* src, uint64_t offset, uint32_t size, void* buf), "pUup", src, offset, size, buf);
WRAPPER(GstBaseSrc, fill, int, (void* src, uint64_t offset, uint32_t size, void* buf), "pUup", src, offset, size, buf);

#define SUPERGO()                       \
    GO(get_caps, pFpp);                 \
    GO(negotiate, iFp);                 \
    GO(fixate, pFpp);                   \
    GO(set_caps, iFpp);                 \
    GO(decide_allocation, iFpp);        \
    GO(start, iFp);                     \
    GO(stop, iFp);                      \
    GO(get_times, vFpppp);              \
    GO(get_size, iFpp);                 \
    GO(is_seekable, iFp);               \
    GO(prepare_seek_segment, iFppp);    \
    GO(do_seek, iFpp);                  \
    GO(unlock, iFp);                    \
    GO(unlock_stop, iFp);               \
    GO(query, iFpp);                    \
    GO(event, iFpp);                    \
    GO(create, iFpUup);                 \
    GO(alloc, iFpUup);                  \
    GO(fill, iFpUup);                   \

// wrap (so bridge all calls, just in case)
static void wrapGstBaseSrcClass(my_GstBaseSrcClass_t* class)
{
    wrapGstElementClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstBaseSrc (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstBaseSrcClass(my_GstBaseSrcClass_t* class)
{
    unwrapGstElementClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstBaseSrc (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstBaseSrcClass(my_GstBaseSrcClass_t* class)
{
    bridgeGstElementClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstBaseSrc (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstBaseSrcInstance(my_GstBaseSrc_t* class)
{
    unwrapGstElementInstance(&class->parent);
}
// autobridge
static void bridgeGstBaseSrcInstance(my_GstBaseSrc_t* class)
{
    bridgeGstElementInstance(&class->parent);
}
// ----- GstPushSrcClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstPushSrc, create, int, (void* src, void* buf), "pp", src, buf);
WRAPPER(GstPushSrc, alloc, int, (void* src, void* buf), "pp", src, buf);
WRAPPER(GstPushSrc, fill, int, (void* src, void* buf), "pp", src, buf);

#define SUPERGO()               \
    GO(create, iFpp);           \
    GO(alloc, iFpp);            \
    GO(fill, iFpp);             \

// wrap (so bridge all calls, just in case)
static void wrapGstPushSrcClass(my_GstPushSrcClass_t* class)
{
    wrapGstBaseSrcClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstPushSrc (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstPushSrcClass(my_GstPushSrcClass_t* class)
{
    unwrapGstBaseSrcClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstPushSrc (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstPushSrcClass(my_GstPushSrcClass_t* class)
{
    bridgeGstBaseSrcClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstPushSrc (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstPushSrcInstance(my_GstPushSrc_t* class)
{
    unwrapGstBaseSrcInstance(&class->parent);
}
// autobridge
static void bridgeGstPushSrcInstance(my_GstPushSrc_t* class)
{
    bridgeGstBaseSrcInstance(&class->parent);
}
// ----- GstGLBaseSrcClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstGLBaseSrc, gl_start, int, (void* src), "p", src);
WRAPPER(GstGLBaseSrc, gl_stop, void, (void* src), "p", src);
WRAPPER(GstGLBaseSrc, fill_gl_memory, int, (void* src, void* mem), "pp", src, mem);

#define SUPERGO()               \
    GO(gl_start, iFp);          \
    GO(gl_stop, vFp);           \
    GO(fill_gl_memory, iFpp);   \

// wrap (so bridge all calls, just in case)
static void wrapGstGLBaseSrcClass(my_GstGLBaseSrcClass_t* class)
{
    wrapGstPushSrcClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstGLBaseSrc (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstGLBaseSrcClass(my_GstGLBaseSrcClass_t* class)
{
    unwrapGstPushSrcClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstGLBaseSrc (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstGLBaseSrcClass(my_GstGLBaseSrcClass_t* class)
{
    bridgeGstPushSrcClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstGLBaseSrc (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstGLBaseSrcInstance(my_GstGLBaseSrc_t* class)
{
    unwrapGstPushSrcInstance(&class->parent);
}
// autobridge
static void bridgeGstGLBaseSrcInstance(my_GstGLBaseSrc_t* class)
{
    bridgeGstPushSrcInstance(&class->parent);
}
// ----- GstAudioDecoderClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstAudioDecoder, start, int,(void* dec), "p", dec);
WRAPPER(GstAudioDecoder, stop, int,(void* dec), "p", dec);
WRAPPER(GstAudioDecoder, set_format, int,(void* dec, void* caps), "pp", dec, caps);
WRAPPER(GstAudioDecoder, parse, int,(void* dec, void* adapter, void* offset, void* length), "pppp", dec, adapter, offset, length);
WRAPPER(GstAudioDecoder, handle_frame, int,(void* dec, void* buffer), "pp", dec, buffer);
WRAPPER(GstAudioDecoder, flush, void ,(void* dec, int hard), "pi", dec, hard);
WRAPPER(GstAudioDecoder, pre_push, int,(void* dec, void* buffer), "pp", dec, buffer);
WRAPPER(GstAudioDecoder, sink_event, int,(void* dec, void* event), "pp", dec, event);
WRAPPER(GstAudioDecoder, src_event, int,(void* dec, void* event), "pp", dec, event);
WRAPPER(GstAudioDecoder, open, int,(void* dec), "p", dec);
WRAPPER(GstAudioDecoder, close, int,(void* dec), "p", dec);
WRAPPER(GstAudioDecoder, negotiate, int,(void* dec), "p", dec);
WRAPPER(GstAudioDecoder, decide_allocation, int,(void* dec, void* query), "pp", dec, query);
WRAPPER(GstAudioDecoder, propose_allocation, int,(void* dec, void* query), "pp", dec, query);
WRAPPER(GstAudioDecoder, sink_query, int,(void* dec, void* query), "pp", dec, query);
WRAPPER(GstAudioDecoder, src_query, int,(void* dec, void* query), "pp", dec, query);
WRAPPER(GstAudioDecoder, getcaps, void*,(void* dec, void*  filter), "pp", dec, filter);
WRAPPER(GstAudioDecoder, transform_meta, int,(void* enc, void* outbuf, void* meta, void* inbuf), "pppp", enc, outbuf, meta, inbuf);

#define SUPERGO()                       \
    GO(start, iFp);                     \
    GO(stop, iFp);                      \
    GO(set_format, iFpp);               \
    GO(parse, iFpppp);                  \
    GO(handle_frame, iFpp);             \
    GO(flush, vFpi);                    \
    GO(pre_push, iFpp);                 \
    GO(sink_event, iFpp);               \
    GO(src_event, iFpp);                \
    GO(open, iFp);                      \
    GO(close, iFp);                     \
    GO(negotiate, iFp);                 \
    GO(decide_allocation, iFpp);        \
    GO(propose_allocation, iFpp);       \
    GO(sink_query, iFpp);               \
    GO(src_query, iFpp);                \
    GO(getcaps, vFpp);                  \
    GO(transform_meta, iFpppp);         \

// wrap (so bridge all calls, just in case)
static void wrapGstAudioDecoderClass(my_GstAudioDecoderClass_t* class)
{
    wrapGstElementClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstAudioDecoder (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstAudioDecoderClass(my_GstAudioDecoderClass_t* class)
{
    unwrapGstElementClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstAudioDecoder (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstAudioDecoderClass(my_GstAudioDecoderClass_t* class)
{
    bridgeGstElementClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstAudioDecoder (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstAudioDecoderInstance(my_GstAudioDecoder_t* class)
{
    unwrapGstElementInstance(&class->parent);
}
// autobridge
static void bridgeGstAudioDecoderInstance(my_GstAudioDecoder_t* class)
{
    bridgeGstElementInstance(&class->parent);
}
// ----- GstVideoFilterClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstVideoFilter, set_info, int, (void* filter, void* incaps, void* in_info, void* outcaps, void* out_info), "ppppp", filter, incaps, in_info, outcaps, out_info);
WRAPPER(GstVideoFilter, transform_frame, int, (void* filter, void* inframe, void* outframe), "ppp", filter, inframe, outframe);
WRAPPER(GstVideoFilter, transform_frame_ip, int, (void* filter, void* frame), "pp", filter, frame);

#define SUPERGO()                       \
    GO(set_info, iFppppp);              \
    GO(transform_frame, iFppp);         \
    GO(transform_frame_ip, iFpp);       \

// wrap (so bridge all calls, just in case)
static void wrapGstVideoFilterClass(my_GstVideoFilterClass_t* class)
{
    wrapGstBaseTransformClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstVideoFilter (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstVideoFilterClass(my_GstVideoFilterClass_t* class)
{
    unwrapGstBaseTransformClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstVideoFilter (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstVideoFilterClass(my_GstVideoFilterClass_t* class)
{
    bridgeGstBaseTransformClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstVideoFilter (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstVideoFilterInstance(my_GstVideoFilter_t* class)
{
    unwrapGstBaseTransformInstance(&class->parent);
}
// autobridge
static void bridgeGstVideoFilterInstance(my_GstVideoFilter_t* class)
{
    bridgeGstBaseTransformInstance(&class->parent);
}
// ----- GstAudioFilterClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstAudioFilter, setup, int, (void* filter, void* info), "pp", filter, info);

#define SUPERGO()       \
    GO(setup, iFpp);    \

// wrap (so bridge all calls, just in case)
static void wrapGstAudioFilterClass(my_GstAudioFilterClass_t* class)
{
    wrapGstBaseTransformClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstAudioFilter (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstAudioFilterClass(my_GstAudioFilterClass_t* class)
{
    unwrapGstBaseTransformClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstAudioFilter (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstAudioFilterClass(my_GstAudioFilterClass_t* class)
{
    bridgeGstBaseTransformClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstAudioFilter (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstAudioFilterInstance(my_GstAudioFilter_t* class)
{
    unwrapGstBaseTransformInstance(&class->parent);
}
// autobridge
static void bridgeGstAudioFilterInstance(my_GstAudioFilter_t* class)
{
    bridgeGstBaseTransformInstance(&class->parent);
}
// ----- GstBufferPoolClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstBufferPool, get_options, void*,(void* pool), "p", pool);
WRAPPER(GstBufferPool, set_config, int ,(void* pool, void* config), "pp", pool, config);
WRAPPER(GstBufferPool, start, int ,(void* pool), "p", pool);
WRAPPER(GstBufferPool, stop, int ,(void* pool), "p", pool);
WRAPPER(GstBufferPool, acquire_buffer, int ,(void* pool, void* buffer, void* params), "ppp", pool, buffer, params);
WRAPPER(GstBufferPool, alloc_buffer, int ,(void* pool, void* buffer, void* params), "ppp", pool, buffer, params);
WRAPPER(GstBufferPool, reset_buffer, void ,(void* pool, void* buffer), "pp", pool, buffer);
WRAPPER(GstBufferPool, release_buffer, void ,(void* pool, void* buffer), "pp", pool, buffer);
WRAPPER(GstBufferPool, free_buffer, void ,(void* pool, void* buffer), "pp", pool, buffer);
WRAPPER(GstBufferPool, flush_start, void ,(void* pool), "p", pool);
WRAPPER(GstBufferPool, flush_stop, void ,(void* pool), "p", pool);

#define SUPERGO()               \
    GO(get_options, pFp);       \
    GO(set_config, iFpp);       \
    GO(start, iFp);             \
    GO(stop, iFp);              \
    GO(acquire_buffer, iFppp);  \
    GO(alloc_buffer, iFppp);    \
    GO(reset_buffer, vFpp);     \
    GO(release_buffer, vFpp);   \
    GO(free_buffer, vFpp);      \
    GO(flush_start, vFp);       \
    GO(flush_stop, vFp);        \

// wrap (so bridge all calls, just in case)
static void wrapGstBufferPoolClass(my_GstBufferPoolClass_t* class)
{
    wrapGstObjectClass(&class->object_class);
    #define GO(A, W) class->A = reverse_##A##_GstBufferPool (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstBufferPoolClass(my_GstBufferPoolClass_t* class)
{
    unwrapGstObjectClass(&class->object_class);
    #define GO(A, W)   class->A = find_##A##_GstBufferPool (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstBufferPoolClass(my_GstBufferPoolClass_t* class)
{
    bridgeGstObjectClass(&class->object_class);
    #define GO(A, W) autobridge_##A##_GstBufferPool (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstBufferPoolInstance(my_GstBufferPool_t* class)
{
    unwrapGstObjectInstance(&class->object);
}
// autobridge
static void bridgeGstBufferPoolInstance(my_GstBufferPool_t* class)
{
    bridgeGstObjectInstance(&class->object);
}
// ----- GstVideoBufferPoolClass ------
// wrapper x86 -> natives of callbacks

#define SUPERGO()               \

// wrap (so bridge all calls, just in case)
static void wrapGstVideoBufferPoolClass(my_GstVideoBufferPoolClass_t* class)
{
    wrapGstBufferPoolClass(&class->parent);
    #define GO(A, W) class->A = reverse_##A##_GstVideoBufferPool (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstVideoBufferPoolClass(my_GstVideoBufferPoolClass_t* class)
{
    unwrapGstBufferPoolClass(&class->parent);
    #define GO(A, W)   class->A = find_##A##_GstVideoBufferPool (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstVideoBufferPoolClass(my_GstVideoBufferPoolClass_t* class)
{
    bridgeGstBufferPoolClass(&class->parent);
    #define GO(A, W) autobridge_##A##_GstBufferPool (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGstVideoBufferPoolInstance(my_GstVideoBufferPool_t* class)
{
    unwrapGstBufferPoolInstance(&class->bufferpool);
}
// autobridge
static void bridgeGstVideoBufferPoolInstance(my_GstVideoBufferPool_t* class)
{
    bridgeGstBufferPoolInstance(&class->bufferpool);
}
// ----- GDBusProxyClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GDBusProxy, g_properties_changed, void, (void* proxy, void* changed_properties, const char* const* invalidated_properties), "ppp", proxy, changed_properties, invalidated_properties);
WRAPPER(GDBusProxy, g_signal, void,             (void* proxy, const char* sender_name, const char* signal_name, void* parameters), "pppp", proxy, sender_name, signal_name, parameters);

#define SUPERGO()                   \
    GO(g_properties_changed, vFppp);\
    GO(g_signal, vFpppp);           \

// wrap (so bridge all calls, just in case)
static void wrapGDBusProxyClass(my_GDBusProxyClass_t* class)
{
    wrapGObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GDBusProxy (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGDBusProxyClass(my_GDBusProxyClass_t* class)
{
    unwrapGObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GDBusProxy (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGDBusProxyClass(my_GDBusProxyClass_t* class)
{
    bridgeGObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GDBusProxy (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGDBusProxyInstance(my_GDBusProxy_t* class)
{
    unwrapGObjectInstance(&class->parent);
}
// autobridge
static void bridgeGDBusProxyInstance(my_GDBusProxy_t* class)
{
    bridgeGObjectInstance(&class->parent);
}
// ----- GstURIHandlerInterface ------
// wrapper x86 -> natives of callbacks
WRAPPER(GstURIHandler,get_type, int, (size_t type), "L", type);
WRAPPER(GstURIHandler,get_protocols, void*, (size_t type), "L", type);
WRAPPER(GstURIHandler,get_uri, void*, (void* handler), "p", handler);
WRAPPER(GstURIHandler,set_uri, int, (void* handler, void* uri, void* error), "ppp", handler, uri, error);

#define SUPERGO()                       \
    GO(get_type, iFL);                  \
    GO(get_protocols, pFL);             \
    GO(get_uri, pFp);                   \
    GO(set_uri, iFppp);                 \

// wrap (so bridge all calls, just in case)
static void wrapGstURIHandlerInterface(my_GstURIHandlerInterface_t* iface)
{
    // parent don't need wrazpping
    #define GO(A, W) iface->A = reverse_##A##_GstURIHandler (W, iface->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGstURIHandlerInterface(my_GstURIHandlerInterface_t* iface)
{
    // parent don't need wrazpping
    #define GO(A, W)   iface->A = find_##A##_GstURIHandler (W, iface->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGstURIHandlerInterface(my_GstURIHandlerInterface_t* iface)
{
    // parent don't need wrazpping
    #define GO(A, W) autobridge_##A##_GstURIHandler (W, iface->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO
// No more wrap/unwrap
#undef WRAPPER
#undef FIND
#undef REVERSE
#undef WRAPPED

// a class to collection of custom defined class...
void addRegisteredClass(size_t klass, char* name)
{
    if(!klass)
        return;
    if(!my_customclass) {
        my_customclass = kh_init(customclass);
    }
    khint_t k;
    int ret;
    k = kh_put(customclass, my_customclass, klass, &ret);
    kh_value(my_customclass, k) = strdup(name);
}

int checkRegisteredClass(size_t klass)
{
    if(!my_customclass)
        return 0;
    khint_t k = kh_get(customclass, my_customclass, klass);
    return (k==kh_end(my_customclass))?0:1;
}

// g_type_class_peek_parent
void wrapGTKClass(void* cl, size_t type)
{
    #define GTKIFACE(A)
    #define GTKCLASS(A)                             \
    if(type==my_##A)                                \
        wrap##A##Class((my_##A##Class_t*)cl);       \
    else

    printf_log(LOG_DEBUG, "wrapGTKClass(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple opbjects have no structure
    else {
        if(my_MetaFrames2==(size_t)-1 && !strcmp(g_type_name(type), "MetaFrames")) {
            my_MetaFrames2 = type;
            wrapMetaFrames2Class((my_MetaFrames2Class_t*)cl);
        } else
            printf_log(LOG_NONE, "Warning, Custom Class initializer with unknown class type 0w%zx (%s)\n", type, g_type_name(type));
    }
    #undef GTKCLASS
    #undef GTKIFACE
}

void unwrapGTKClass(void* cl, size_t type)
{
    #define GTKIFACE(A)
    #define GTKCLASS(A)                             \
    if(type==my_##A)                                \
        unwrap##A##Class((my_##A##Class_t*)cl);     \
    else

    printf_log(LOG_DEBUG, "...unwrapGTKClass(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple opbjects have no structure
    else
        printf_log(LOG_NONE, "Warning: fail to unwrapGTKClass for type %zx (%s)\n", type, g_type_name(type));
    #undef GTKCLASS
    #undef GTKIFACE
}

static void bridgeGTKClass(void* cl, size_t type)
{
    #define GTKIFACE(A)
    #define GTKCLASS(A)                             \
    if(type==my_##A)                                \
        bridge##A##Class((my_##A##Class_t*)cl);     \
    else

    printf_log(LOG_DEBUG, "bridgeGTKClass(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple opbjects have no structure
    else {
        printf_log(LOG_NONE, "Warning, AutoBridge GTK Class with unknown class type 0x%zx (%s)\n", type, g_type_name(type));
    }
    #undef GTKCLASS
    #undef GTKIFACE
}

static void wrapGTKInterface(void* cl, size_t type)
{
    #define GTKCLASS(A)
    #define GTKIFACE(A)                                 \
    if(type==my_##A)                                    \
        wrap##A##Interface((my_##A##Interface_t*)cl);   \
    else

    printf_log(LOG_DEBUG, "wrapGTKInterface(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple opbjects have no structure
    else {
        printf_log(LOG_NONE, "Warning, Custom Interface initializer with unknown class type 0x%zx (%s)\n", type, g_type_name(type));
    }
    #undef GTKIFACE
    #undef GTKCLASS
}

void unwrapGTKInterface(void* cl, size_t type)
{
    #define GTKCLASS(A)
    #define GTKIFACE(A)                                 \
    if(type==my_##A)                                    \
        unwrap##A##Interface((my_##A##Interface_t*)cl); \
    else

    printf_log(LOG_DEBUG, "unwrapGTKInterface(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple opbjects have no structure
    else
    {}  // else no warning, one is enough...
    #undef GTKIFACE
    #undef GTKCLASS
}

static void bridgeGTKInterface(void* cl, size_t type)
{
    #define GTKCLASS(A)
    #define GTKIFACE(A)                                 \
    if(type==my_##A)                                    \
        bridge##A##Interface((my_##A##Interface_t*)cl); \
    else

    printf_log(LOG_DEBUG, "bridgeGTKInterface(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple opbjects have no structure
    else {
        printf_log(LOG_NONE, "Warning, AutoBridge GTK Interface with unknown class type 0x%zx (%s)\n", type, g_type_name(type));
    }
    #undef GTKCLASS
    #undef GTKIFACE
}

void unwrapGTKInstance(void* cl, size_t type)
{
    #define GTKIFACE(A)
    #define GTKCLASS(A)                             \
    if(type==my_##A)                                \
        unwrap##A##Instance((my_##A##_t*)cl);     \
    else

    printf_log(LOG_DEBUG, "unwrapGTKInstance(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple opbjects have no structure
    else
    {}  // else no warning, one is enough...
    #undef GTKCLASS
    #undef GTKIFACE
}

void bridgeGTKInstance(void* cl, size_t type)
{
    #define GTKIFACE(A)
    #define GTKCLASS(A)                             \
    if(type==my_##A)                                \
        bridge##A##Instance((my_##A##_t*)cl);     \
    else

    printf_log(LOG_DEBUG, "bridgeGTKInstance(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple opbjects have no structure
    else {
        printf_log(LOG_NONE, "Warning, AutoBridge GTK Class with unknown class type 0w%zx (%s)\n", type, g_type_name(type));
    }
    #undef GTKCLASS
    #undef GTKIFACE
}

typedef union my_GClassAll_s {
    #define GTKCLASS(A) my_##A##Class_t A;
    #define GTKIFACE(A) my_##A##Interface_t A;
    GTKCLASSES()
    #undef GTKIFACE
    #undef GTKCLASS
} my_GClassAll_t;

#define GO(A) \
static void* my_gclassall_ref_##A = NULL;   \
static my_GClassAll_t my_gclassall_##A;

SUPER()
#undef GO

void* unwrapCopyGTKClass(void* klass, size_t type)
{
    if(!klass) return klass;
    if(checkRegisteredClass(type))
        return klass;
    #define GO(A) if(klass == my_gclassall_ref_##A) return &my_gclassall_##A;
    SUPER()
    #undef GO
    // check if class is the exact type we know
    size_t sz = 0;
    #define GTKIFACE(A)
    #define GTKCLASS(A) if(type==my_##A) sz = sizeof(my_##A##Class_t); else
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple opbjects have no structure
    else {
        printf_log(LOG_NONE, "Warning, unwrapCopyGTKClass called with unknown class type 0x%zx (%s)\n", type, g_type_name(type));
        return klass;
    }
    #undef GTKCLASS
    #undef GTKIFACE
    my_GClassAll_t *newklass = NULL;
    #define GO(A) if(!newklass && !my_gclassall_ref_##A) {my_gclassall_ref_##A = klass; newklass = &my_gclassall_##A;}
    SUPER()
    #undef GO
    if(!newklass) {
        printf_log(LOG_NONE, "Warning: no more slot for unwrapCopyGTKClass\n");
        return klass;
    }
    memcpy(newklass, klass, sz);
    unwrapGTKClass(newklass, type);
    return newklass;
}

void* unwrapCopyGTKInterface(void* iface, size_t type)
{
    if(!iface) return iface;
    if(checkRegisteredClass(type))
        return iface;
    #define GO(A) if(iface == my_gclassall_ref_##A) return &my_gclassall_##A;
    SUPER()
    #undef GO
    // check if class is the exact type we know
    size_t sz = 0;
    #define GTKIFACE(A) if(type==my_##A) sz = sizeof(my_##A##Interface_t); else
    #define GTKCLASS(A)
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple opbjects have no structure
    else {
        printf_log(LOG_NONE, "Warning, unwrapCopyGTKInterface called with unknown class type 0x%zx (%s)\n", type, g_type_name(type));
        return iface;
    }
    #undef GTKCLASS
    #undef GTKIFACE
    my_GClassAll_t *newiface = NULL;
    #define GO(A) if(!newiface && !my_gclassall_ref_##A) {my_gclassall_ref_##A = iface; newiface = &my_gclassall_##A;}
    SUPER()
    #undef GO
    if(!newiface) {
        printf_log(LOG_NONE, "Warning: no more slot for unwrapCopyGTKInterface\n");
        return iface;
    }
    memcpy(newiface, iface, sz);
    unwrapGTKInterface(newiface, type);
    return newiface;
}

// gtk_type_class

void* wrapCopyGTKClass(void* klass, size_t type)
{
    if(!klass) return klass;
    while(checkRegisteredClass(type))
        type = g_type_parent(type);
    printf_log(LOG_DEBUG, "wrapCopyGTKClass(%p, %zd (%s))\n", klass, type, g_type_name(type));
    bridgeGTKClass(klass, type);
    return klass;
}

void* wrapCopyGTKInterface(void* iface, size_t type)
{
    if(!iface) return iface;
    while(checkRegisteredClass(type))
        type = g_type_parent(type);
    printf_log(LOG_DEBUG, "wrapCopyGTKInterface(%p, %zd (%s))\n", iface, type, g_type_name(type));
    bridgeGTKInterface(iface, type);
    return iface;
}
// ---- GTypeValueTable ----

// First the structure GTypeInfo statics, with paired x64 source pointer
#define GO(A) \
static my_GTypeValueTable_t     my_gtypevaluetable_##A = {0};   \
static my_GTypeValueTable_t   *ref_gtypevaluetable_##A = NULL;
SUPER()
#undef GO
// Then the static functions callback that may be used with the structure
// value_init ...
#define GO(A)   \
static uintptr_t my_value_init_fct_##A = 0;                     \
static void my_value_init_##A(void* a)                          \
{                                                               \
    RunFunctionFmt(my_value_init_fct_##A, "p", a);  \
}
SUPER()
#undef GO
static void* find_value_init_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_value_init_fct_##A == (uintptr_t)fct) return my_value_init_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_value_init_fct_##A == 0) {my_value_init_fct_##A = (uintptr_t)fct; return my_value_init_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo value_init callback\n");
    return NULL;
}
// value_free ...
#define GO(A)   \
static uintptr_t my_value_free_fct_##A = 0;                     \
static void my_value_free_##A(void* a)                          \
{                                                               \
    RunFunctionFmt(my_value_free_fct_##A, "p", a);  \
}
SUPER()
#undef GO
static void* find_value_free_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_value_free_fct_##A == (uintptr_t)fct) return my_value_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_value_free_fct_##A == 0) {my_value_free_fct_##A = (uintptr_t)fct; return my_value_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo value_free callback\n");
    return NULL;
}
// value_copy ...
#define GO(A)   \
static uintptr_t my_value_copy_fct_##A = 0;                     \
static void my_value_copy_##A(void* a, void* b)                 \
{                                                               \
    RunFunctionFmt(my_value_copy_fct_##A, "pp", a, b); \
}
SUPER()
#undef GO
static void* find_value_copy_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_value_copy_fct_##A == (uintptr_t)fct) return my_value_copy_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_value_copy_fct_##A == 0) {my_value_copy_fct_##A = (uintptr_t)fct; return my_value_copy_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo value_copy callback\n");
    return NULL;
}
// value_peek_pointer ...
#define GO(A)   \
static uintptr_t my_value_peek_pointer_fct_##A = 0;                                 \
static void* my_value_peek_pointer_##A(void* a)                                     \
{                                                                                   \
    return (void*)RunFunctionFmt(my_value_peek_pointer_fct_##A, "p", a);\
}
SUPER()
#undef GO
static void* find_value_peek_pointer_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_value_peek_pointer_fct_##A == (uintptr_t)fct) return my_value_peek_pointer_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_value_peek_pointer_fct_##A == 0) {my_value_peek_pointer_fct_##A = (uintptr_t)fct; return my_value_peek_pointer_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo value_peek_pointer callback\n");
    return NULL;
}
// collect_value ...
#define GO(A)   \
static uintptr_t my_collect_value_fct_##A = 0;                                              \
static void* my_collect_value_##A(void* a, uint32_t b, void* c, uint32_t d)                 \
{                                                                                           \
    return (void*)RunFunctionFmt(my_collect_value_fct_##A, "pupu", a, b, c, d); \
}
SUPER()
#undef GO
static void* find_collect_value_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_collect_value_fct_##A == (uintptr_t)fct) return my_collect_value_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_collect_value_fct_##A == 0) {my_collect_value_fct_##A = (uintptr_t)fct; return my_collect_value_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo collect_value callback\n");
    return NULL;
}
// lcopy_value ...
#define GO(A)   \
static uintptr_t my_lcopy_value_fct_##A = 0;                                                \
static void* my_lcopy_value_##A(void* a, uint32_t b, void* c, uint32_t d)                   \
{                                                                                           \
    return (void*)RunFunctionFmt(my_lcopy_value_fct_##A, "pupu", a, b, c, d);   \
}
SUPER()
#undef GO
static void* find_lcopy_value_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_lcopy_value_fct_##A == (uintptr_t)fct) return my_lcopy_value_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_lcopy_value_fct_##A == 0) {my_lcopy_value_fct_##A = (uintptr_t)fct; return my_lcopy_value_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo lcopy_value callback\n");
    return NULL;
}
// And now the get slot / assign... Taking into account that the desired callback may already be a wrapped one (so unwrapping it)
my_GTypeValueTable_t* findFreeGTypeValueTable(my_GTypeValueTable_t* fcts)
{
    if(!fcts) return fcts;
    #define GO(A) if(ref_gtypevaluetable_##A == fcts) return &my_gtypevaluetable_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_gtypevaluetable_##A == 0) {                                                        \
        ref_gtypevaluetable_##A = fcts;                                                                     \
        my_gtypevaluetable_##A.value_init = find_value_init_Fct(fcts->value_init);                          \
        my_gtypevaluetable_##A.value_free = find_value_free_Fct(fcts->value_free);                          \
        my_gtypevaluetable_##A.value_copy = find_value_copy_Fct(fcts->value_copy);                          \
        my_gtypevaluetable_##A.value_peek_pointer = find_value_peek_pointer_Fct(fcts->value_peek_pointer);  \
        my_gtypevaluetable_##A.collect_format = fcts->collect_format;                                       \
        my_gtypevaluetable_##A.collect_value = find_collect_value_Fct(fcts->collect_value);                 \
        my_gtypevaluetable_##A.lcopy_format = fcts->lcopy_format;                                           \
        my_gtypevaluetable_##A.lcopy_value = find_lcopy_value_Fct(fcts->lcopy_value);                       \
        return &my_gtypevaluetable_##A;                                                                     \
    }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeValueTable callback\n");
    return NULL;
}

// signal2 ...
#define GO(A)   \
static uintptr_t my_signal2_fct_##A = 0;                                        \
static void* my_signal2_##A(void* a, void* b)                                   \
{                                                                               \
    return (void*)RunFunctionFmt(my_signal2_fct_##A, "pp", a, b);   \
}
SUPER()
#undef GO
static void* find_signal2_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal2_fct_##A == (uintptr_t)fct) return my_signal2_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal2_fct_##A == 0) {my_signal2_fct_##A = (uintptr_t)fct; return my_signal2_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal2 callback\n");
    return NULL;
}
// signal3 ...
#define GO(A)   \
static uintptr_t my_signal3_fct_##A = 0;                                            \
static void* my_signal3_##A(void* a, void* b, void* c)                              \
{                                                                                   \
    return (void*)RunFunctionFmt(my_signal3_fct_##A, "ppp", a, b, c);   \
}
SUPER()
#undef GO
static void* find_signal3_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal3_fct_##A == (uintptr_t)fct) return my_signal3_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal3_fct_##A == 0) {my_signal3_fct_##A = (uintptr_t)fct; return my_signal3_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal3 callback\n");
    return NULL;
}
// signal4 ...
#define GO(A)   \
static uintptr_t my_signal4_fct_##A = 0;                                                \
static void* my_signal4_##A(void* a, void* b, void* c, void* d)                         \
{                                                                                       \
    return (void*)RunFunctionFmt(my_signal4_fct_##A, "pppp", a, b, c, d);   \
}
SUPER()
#undef GO
static void* find_signal4_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal4_fct_##A == (uintptr_t)fct) return my_signal4_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal4_fct_##A == 0) {my_signal4_fct_##A = (uintptr_t)fct; return my_signal4_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal4 callback\n");
    return NULL;
}
// signal5 ...
#define GO(A)   \
static uintptr_t my_signal5_fct_##A = 0;                                                    \
static void* my_signal5_##A(void* a, void* b, void* c, void* d, void* e)                    \
{                                                                                           \
    return (void*)RunFunctionFmt(my_signal5_fct_##A, "ppppp", a, b, c, d, e);   \
}
SUPER()
#undef GO
static void* find_signal5_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal5_fct_##A == (uintptr_t)fct) return my_signal5_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal5_fct_##A == 0) {my_signal5_fct_##A = (uintptr_t)fct; return my_signal5_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal5 callback\n");
    return NULL;
}
// signal6 ...
#define GO(A)   \
static uintptr_t my_signal6_fct_##A = 0;                                                        \
static void* my_signal6_##A(void* a, void* b, void* c, void* d, void* e, void* f)               \
{                                                                                               \
    return (void*)RunFunctionFmt(my_signal6_fct_##A, "pppppp", a, b, c, d, e, f);   \
}
SUPER()
#undef GO
static void* find_signal6_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal6_fct_##A == (uintptr_t)fct) return my_signal6_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal6_fct_##A == 0) {my_signal6_fct_##A = (uintptr_t)fct; return my_signal6_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal6 callback\n");
    return NULL;
}
// signal7 ...
#define GO(A)   \
static uintptr_t my_signal7_fct_##A = 0;                                                            \
static void* my_signal7_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g)          \
{                                                                                                   \
    return (void*)RunFunctionFmt(my_signal7_fct_##A, "ppppppp", a, b, c, d, e, f, g);   \
}
SUPER()
#undef GO
static void* find_signal7_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal7_fct_##A == (uintptr_t)fct) return my_signal7_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal7_fct_##A == 0) {my_signal7_fct_##A = (uintptr_t)fct; return my_signal7_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal7 callback\n");
    return NULL;
}
// signal8 ...
#define GO(A)                                                                                  \
    static uintptr_t my_signal8_fct_##A = 0;                                                   \
    static void* my_signal8_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g) \
    {                                                                                          \
        return (void*)RunFunctionFmt(my_signal8_fct_##A, "ppppppp", a, b, c, d, e, f, g);      \
    }
SUPER()
#undef GO
static void* find_signal8_Fct(void* fct)
{
    if (!fct) return fct;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
#define GO(A) \
    if (my_signal8_fct_##A == (uintptr_t)fct) return my_signal8_##A;
    SUPER()
#undef GO
#define GO(A)                                \
    if (my_signal8_fct_##A == 0) {           \
        my_signal8_fct_##A = (uintptr_t)fct; \
        return my_signal8_##A;               \
    }
    SUPER()
#undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal8 callback\n");
    return NULL;
}
// signal9 ...
#define GO(A)                                                                                  \
    static uintptr_t my_signal9_fct_##A = 0;                                                   \
    static void* my_signal9_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g) \
    {                                                                                          \
        return (void*)RunFunctionFmt(my_signal9_fct_##A, "ppppppp", a, b, c, d, e, f, g);      \
    }
SUPER()
#undef GO
static void* find_signal9_Fct(void* fct)
{
    if (!fct) return fct;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
#define GO(A) \
    if (my_signal9_fct_##A == (uintptr_t)fct) return my_signal9_##A;
    SUPER()
#undef GO
#define GO(A)                                \
    if (my_signal9_fct_##A == 0) {           \
        my_signal9_fct_##A = (uintptr_t)fct; \
        return my_signal9_##A;               \
    }
    SUPER()
#undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal9 callback\n");
    return NULL;
}
// signal10 ...
#define GO(A)                                                                                   \
    static uintptr_t my_signal10_fct_##A = 0;                                                   \
    static void* my_signal10_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g) \
    {                                                                                           \
        return (void*)RunFunctionFmt(my_signal10_fct_##A, "ppppppp", a, b, c, d, e, f, g);      \
    }
SUPER()
#undef GO
static void* find_signal10_Fct(void* fct)
{
    if (!fct) return fct;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
#define GO(A) \
    if (my_signal10_fct_##A == (uintptr_t)fct) return my_signal10_##A;
    SUPER()
#undef GO
#define GO(A)                                 \
    if (my_signal10_fct_##A == 0) {           \
        my_signal10_fct_##A = (uintptr_t)fct; \
        return my_signal10_##A;               \
    }
    SUPER()
#undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal10 callback\n");
    return NULL;
}
typedef void* (*finder_t)(void*);
static const finder_t finders[] = { find_signal2_Fct, find_signal3_Fct, find_signal4_Fct, find_signal5_Fct, find_signal6_Fct, find_signal7_Fct, find_signal8_Fct, find_signal9_Fct, find_signal10_Fct };
#define MAX_SIGNAL_N (10 - 2)

// ---- GTypeInfo ----
// let's handle signal with offset, that are used to wrap custom signal function
void my_unwrap_signal_offset(void* klass);
void my_add_signal_offset(size_t itype, uint32_t offset, int n)
{
    printf_log(LOG_DEBUG, "my_add_signal_offset(0x%zx, %d, %d)\n", itype, offset, n);
    if(!offset || !itype) // no offset means no overload...
        return;
    if(n<0 || n>MAX_SIGNAL_N) {
        printf_log(LOG_NONE, "Warning, signal with too many args (%d) in my_add_signal_offset\n", n);
        return;
    }
    int ret;
    khint_t k = kh_put(sigoffset, my_sigoffset, itype, &ret);
    sigoffset_array_t *p = &kh_value(my_sigoffset, k);
    if(ret) {
        p->a = NULL; p->cap = 0; p->sz = 0;
    }
    // check if offset already there
    for(int i=0; i<p->sz; ++i)
        if(p->a[i].offset == offset) {
            printf_log(LOG_INFO, "Offset already there... Bye\n");
            return; // already there, bye
        }
    if(p->sz==p->cap) {
        p->cap+=4;
        p->a = (sigoffset_t*)box_realloc(p->a, sizeof(sigoffset_t)*p->cap);
    }
    p->a[p->sz].offset = offset;
    p->a[p->sz++].n = n;
}
void my_unwrap_signal_offset(void* klass)
{
    if(!klass)
        return;
    size_t itype = *(size_t*)klass;
    khint_t k = kh_get(sigoffset, my_sigoffset, itype);
    if(k==kh_end(my_sigoffset))
        return;
    sigoffset_array_t *p = &kh_value(my_sigoffset, k);
    printf_log(LOG_DEBUG, "my_unwrap_signal_offset(%p) type=0x%zx with %d signals with offset\n", klass, itype, p->sz);
    for(int i=0; i<p->sz; ++i) {
        void** f = (void**)((uintptr_t)klass + p->a[i].offset);
        void* new_f = GetNativeFnc((uintptr_t)*f);
        if(!new_f) {
            // Not a native function: autobridge it
            new_f = finders[p->a[i].n](f);
        }
        if(new_f != *f) {
            printf_log(LOG_DEBUG, "Unwrapping %p[%p: offset=0x%x, n=%d] -> %p (with alternate)\n", *f, f, p->a[i].offset, p->a[i].n, new_f);
            if(!hasAlternate(new_f))
                addAlternate(new_f, *f);
            *f = new_f;
        }
    }
}

// First the structure my_GTypeInfo_t statics, with paired x64 source pointer
#define GO(A) \
static my_GTypeInfo_t     my_gtypeinfo_##A = {0};   \
static my_GTypeInfo_t    ref_gtypeinfo_##A = {0};   \
static int              used_gtypeinfo_##A = 0;
SUPER()
#undef GO
// Then the static functions callback that may be used with the structure
// base_init ...
#define GO(A)   \
static uintptr_t my_base_init_fct_##A = 0;                          \
static int my_base_init_##A(void* a)                                \
{                                                                   \
    return RunFunctionFmt(my_base_init_fct_##A, "p", a);     \
}
SUPER()
#undef GO
static void* find_base_init_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_base_init_fct_##A == (uintptr_t)fct) return my_base_init_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_base_init_fct_##A == 0) {my_base_init_fct_##A = (uintptr_t)fct; return my_base_init_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo base_init callback\n");
    return NULL;
}
// base_finalize ...
#define GO(A)   \
static uintptr_t my_base_finalize_fct_##A = 0;                      \
static int my_base_finalize_##A(void* a)                            \
{                                                                   \
    return RunFunctionFmt(my_base_finalize_fct_##A, "p", a); \
}
SUPER()
#undef GO
static void* find_base_finalize_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_base_finalize_fct_##A == (uintptr_t)fct) return my_base_finalize_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_base_finalize_fct_##A == 0) {my_base_finalize_fct_##A = (uintptr_t)fct; return my_base_finalize_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo base_finalize callback\n");
    return NULL;
}
// class_init ...
#define GO(A)   \
static uintptr_t my_class_init_fct_##A = 0;                                 \
static size_t parent_class_init_##A = 0;                                    \
static int my_class_init_##A(void* a, void* b)                              \
{                                                                           \
    printf_log(LOG_DEBUG, "Custom Class init %d for class %p (parent=%p:%s)\n", A, a, (void*)parent_class_init_##A, g_type_name(parent_class_init_##A));\
    int ret = RunFunctionFmt(my_class_init_fct_##A, "pp", a, b);            \
    size_t type = parent_class_init_##A;                                    \
    while(checkRegisteredClass(type))                                       \
        type = g_type_parent(type);                                         \
    unwrapGTKClass(a, type);                                                \
    my_unwrap_signal_offset(a);                                             \
    if(!strcmp(g_type_name(type), "AtkUtil")) {                             \
        my_AtkUtilClass_t* p = (my_AtkUtilClass_t*)g_type_class_peek(type); \
        unwrapGTKClass(p, type);                                            \
    }                                                                       \
    return ret;                                                             \
}
SUPER()
#undef GO
void* find_class_init_Fct(void* fct, size_t parent)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_class_init_fct_##A == (uintptr_t)fct && parent_class_init_##A==parent) return my_class_init_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_class_init_fct_##A == 0) {my_class_init_fct_##A = (uintptr_t)fct; parent_class_init_##A=parent; return my_class_init_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo class_init callback\n");
    return NULL;
}
// class_finalize ...
#define GO(A)   \
static uintptr_t my_class_finalize_fct_##A = 0;                                 \
static int my_class_finalize_##A(void* a, void* b)                              \
{                                                                               \
    return RunFunctionFmt(my_class_finalize_fct_##A, "pp", a, b);               \
}
SUPER()
#undef GO
static void* find_class_finalize_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_class_finalize_fct_##A == (uintptr_t)fct) return my_class_finalize_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_class_finalize_fct_##A == 0) {my_class_finalize_fct_##A = (uintptr_t)fct; return my_class_finalize_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo class_finalize callback\n");
    return NULL;
}
// instance_init ...
#define GO(A)   \
static uintptr_t my_instance_init_fct_##A = 0;                              \
static size_t parent_instance_init_##A = 0;                                 \
static int my_instance_init_##A(void* a, void* b)                           \
{                                                                           \
    printf_log(LOG_DEBUG, "Custom Instance init %d for class %p (parent=%p:%s)\n", A, a, (void*)parent_instance_init_##A, g_type_name(parent_instance_init_##A));\
    int ret = RunFunctionFmt(my_instance_init_fct_##A, "pp", a, b);         \
    size_t type = parent_instance_init_##A;                                 \
    while(checkRegisteredClass(type))                                       \
        type = g_type_parent(type);                                         \
    unwrapGTKInstance(a, type);                                             \
    bridgeGTKInstance(a, type);                                             \
    return ret;                                                             \
}
SUPER()
#undef GO
static void* find_instance_init_Fct(void* fct, size_t parent)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_instance_init_fct_##A == (uintptr_t)fct && parent_instance_init_##A==parent) return my_instance_init_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_instance_init_fct_##A == 0) {my_instance_init_fct_##A = (uintptr_t)fct; parent_instance_init_##A=parent; return my_instance_init_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo instance_init callback\n");
    return NULL;
}
// And now the get slot / assign... Taking into account that the desired callback may already be a wrapped one (so unwrapping it)
my_GTypeInfo_t* findFreeGTypeInfo(my_GTypeInfo_t* fcts, size_t parent)
{
    if(!fcts) return NULL;
    #define GO(A) if(used_gtypeinfo_##A && memcmp(&ref_gtypeinfo_##A, fcts, sizeof(my_GTypeInfo_t))==0) return &my_gtypeinfo_##A;
    SUPER()
    #undef GO
    #define GO(A) if(used_gtypeinfo_##A == 0) {                                                 \
        used_gtypeinfo_##A = 1;                                                                 \
        memcpy(&ref_gtypeinfo_##A, fcts, sizeof(my_GTypeInfo_t));                               \
        my_gtypeinfo_##A.class_size = fcts->class_size;                                         \
        my_gtypeinfo_##A.base_init = find_base_init_Fct(fcts->base_init);                       \
        my_gtypeinfo_##A.base_finalize = find_base_finalize_Fct(fcts->base_finalize);           \
        my_gtypeinfo_##A.class_init = find_class_init_Fct(fcts->class_init, parent);            \
        my_gtypeinfo_##A.class_finalize = find_class_finalize_Fct(fcts->class_finalize);        \
        my_gtypeinfo_##A.class_data = fcts->class_data;                                         \
        my_gtypeinfo_##A.instance_size = fcts->instance_size;                                   \
        my_gtypeinfo_##A.n_preallocs = fcts->n_preallocs;                                       \
        my_gtypeinfo_##A.instance_init = find_instance_init_Fct(fcts->instance_init, parent);   \
        my_gtypeinfo_##A.value_table = findFreeGTypeValueTable(fcts->value_table);              \
        return &my_gtypeinfo_##A;                                                               \
    }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo callback\n");
    return NULL;
}

// ---- GtkTypeInfo ----

// First the structure my_GtkTypeInfo_t statics, with paired x64 source pointer
#define GO(A) \
static my_GtkTypeInfo_t     my_gtktypeinfo_##A = {0};   \
static my_GtkTypeInfo_t    ref_gtktypeinfo_##A = {0};  \
static int                used_gtktypeinfo_##A = 0;
SUPER()
#undef GO
// Then the static functions callback that may be used with the structure
#define GO(A)   \
static int fct_gtk_parent_##A = 0 ;                                                 \
static uintptr_t fct_gtk_class_init_##A = 0;                                        \
static int my_gtk_class_init_##A(void* g_class) {                                   \
    printf_log(LOG_DEBUG, "Calling fct_gtk_class_init_" #A " wrapper\n");           \
    int ret = (int)RunFunctionFmt(fct_gtk_class_init_##A, "p", g_class);            \
    unwrapGTKClass(g_class, fct_gtk_parent_##A);                                    \
    return ret;                                                                     \
}                                                                                   \
static uintptr_t fct_gtk_object_init_##A = 0;                                       \
static int my_gtk_object_init_##A(void* object, void* data) {                       \
    printf_log(LOG_DEBUG, "Calling fct_gtk_object_init_" #A " wrapper\n");          \
    return (int)RunFunctionFmt(fct_gtk_object_init_##A, "pp", object, data);        \
}                                                                                   \
static uintptr_t fct_gtk_base_class_init_##A = 0;                                   \
static int my_gtk_base_class_init_##A(void* instance, void* data) {                 \
    printf_log(LOG_DEBUG, "Calling fct_gtk_base_class_init_" #A " wrapper\n");      \
    return (int)RunFunctionFmt(fct_gtk_base_class_init_##A, "pp", instance, data);  \
}

SUPER()
#undef GO
// And now the get slot / assign... Taking into account that the desired callback may already be a wrapped one (so unwrapping it)
my_GtkTypeInfo_t* findFreeGtkTypeInfo(my_GtkTypeInfo_t* fcts, size_t parent)
{
    if(!fcts) return NULL;
    #define GO(A) if(used_gtktypeinfo_##A && memcmp(&ref_gtktypeinfo_##A, fcts, sizeof(my_GtkTypeInfo_t))==0) return &my_gtktypeinfo_##A;
    SUPER()
    #undef GO
    #define GO(A) if(used_gtktypeinfo_##A == 0) {          \
        memcpy(&ref_gtktypeinfo_##A, fcts, sizeof(my_GtkTypeInfo_t));        \
        fct_gtk_parent_##A = parent;                        \
        my_gtktypeinfo_##A.type_name = fcts->type_name; \
        my_gtktypeinfo_##A.object_size = fcts->object_size; \
        my_gtktypeinfo_##A.class_size = fcts->class_size; \
        my_gtktypeinfo_##A.class_init_func = (fcts->class_init_func)?((GetNativeFnc((uintptr_t)fcts->class_init_func))?GetNativeFnc((uintptr_t)fcts->class_init_func):(void*)my_gtk_class_init_##A):NULL;    \
        fct_gtk_class_init_##A = (uintptr_t)fcts->class_init_func;           \
        my_gtktypeinfo_##A.object_init_func = (fcts->object_init_func)?((GetNativeFnc((uintptr_t)fcts->object_init_func))?GetNativeFnc((uintptr_t)fcts->object_init_func):(void*)my_gtk_object_init_##A):NULL;    \
        fct_gtk_object_init_##A = (uintptr_t)fcts->object_init_func;         \
        my_gtktypeinfo_##A.reserved_1 = fcts->reserved_1;                 \
        my_gtktypeinfo_##A.reserved_2 = fcts->reserved_2;                 \
        my_gtktypeinfo_##A.base_class_init_func = (fcts->base_class_init_func)?((GetNativeFnc((uintptr_t)fcts->base_class_init_func))?GetNativeFnc((uintptr_t)fcts->base_class_init_func):(void*)my_gtk_base_class_init_##A):NULL;    \
        fct_gtk_base_class_init_##A = (uintptr_t)fcts->base_class_init_func;   \
        return &my_gtktypeinfo_##A;                       \
    }

    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GtkTypeInfo callback\n");
    return NULL;
}

#undef SUPER

void InitGTKClass(bridge_t *bridge)
{
    my_bridge  = bridge;
    my_signalmap = kh_init(signalmap);
    my_sigoffset = kh_init(sigoffset);
}

void FiniGTKClass()
{
    if(my_signalmap) {
        /*khint_t k;
        kh_foreach_key(my_signalmap, k,
            my_signal_t* p = (my_signal_t*)(uintptr_t)k;
            box_free(p);
        );*/ // lets assume all signals data is freed by gtk already
        kh_destroy(signalmap, my_signalmap);
        my_signalmap = NULL;
    }
    if(my_sigoffset) {
        sigoffset_array_t* p;
        kh_foreach_value_ref(my_sigoffset, p,
            box_free(p->a);
        );
        kh_destroy(sigoffset, my_sigoffset);
        my_sigoffset = NULL;
    }
}

#define GTKCLASS(A)             \
void Set##A##ID(size_t id)      \
{                               \
    my_##A = id;                \
}
#define GTKIFACE(A)  GTKCLASS(A)
GTKCLASSES()
#undef GTKIFACE
#undef GTKCLASS

void AutoBridgeGtk(void*(*ref)(size_t), void(*unref)(void*))
{
    void* p;
    #define GTKIFACE(A)
    #define GTKCLASS(A)                \
    if(my_##A && my_##A!=(size_t)-1) { \
        p = ref(my_##A);               \
        bridgeGTKClass(p, my_##A);     \
        unref(p);                      \
    }
    GTKCLASSES()
    #undef GTKIFACE
    #undef GTKCLASS
}

void SetGTypeName(void* f)
{
    g_type_name = f;
}

void SetGClassPeek(void* f)
{
    g_type_class_peek = f;
}

void SetGTypeParent(void* f)
{
    g_type_parent = f;
}

