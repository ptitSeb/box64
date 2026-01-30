#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <dlfcn.h>
#include <glob.h>

#include "libdl.h"
#include "box64context.h"
#include "debug.h"
#include "x64emu.h"
#include "elfloader.h"
#include "fileutils.h"
#include "emu/x64emu_private.h"
#include "json.h"
#include "callback.h"
#include "bridge.h"
#include "khash.h"
#include "wrapper.h"
#include "alternate.h"
#include "myalign.h"

#include "vulkanoverlay.h"

typedef struct my_x64wrapper_s {
    void*       f;
    uintptr_t*  x64;
} my_x64wrapper_t;

KHASH_MAP_INIT_STR(x64wrappers, my_x64wrapper_t)

typedef struct my_vulkanoverlay_s {
    void*               handle;   // internal my_dlopen handle
    const char*         lib_name;
    struct {
        char* name;
        char* f;
    } *                 functions;
    size_t              n_functions;
    kh_x64wrappers_t*   wrappers;
    int                 idx;
} my_vulkanoverlay_t;

void* my_dlopen(x64emu_t* emu, const char *filename, int flag);
int my_dlclose(x64emu_t* emu, void *handle);
void* my_dlsym(x64emu_t* emu, void *handle, const char *symbol);

void* wrapVulkanSymbol(my_vulkanoverlay_t* v, const char* name, uintptr_t x64)
{
    if(!x64) return NULL;
    if(!v) return NULL;
    if(!v->wrappers) {
        printf_log(LOG_INFO, "Warning: Wrapped Vulkan Overlay without Wrappers Hash\n");
        return NULL;
    }
    const char* rname = NULL;
    for(size_t i=0; i<v->n_functions && !rname; ++i)
        if(!strcmp(name, v->functions[i].f))
            rname = v->functions[i].name;
    if(!rname) rname = name;
    khint_t k = kh_get(x64wrappers, v->wrappers, name);
    if(k==kh_end(v->wrappers)) {
        printf_log(LOG_INFO, "Warning, unknown wrapper for %s in Vulkan Overlay\n", rname);
        return NULL;
    }
    *kh_value(v->wrappers, k).x64 = x64;
    return kh_value(v->wrappers, k).f;
}

#define MAXN    4
static my_vulkanoverlay_t overlays[MAXN] = {0};

my_vulkanoverlay_t* getNewVulkanOverlay()
{
    my_vulkanoverlay_t* v = NULL;
    for(int i=0; i<MAXN && !v; ++i) {
        if(!overlays[i].wrappers) {
            v = overlays+i;
            v->idx = i;
            v->wrappers = kh_init(x64wrappers);
        }
    }
    return v;
}

void freeVulkanOverlay(my_vulkanoverlay_t* v)
{
    if(!v) return;
    if(!v->wrappers) return;
    size_t idx = v->idx;
    my_x64wrapper_t* w;
    kh_cstr_t k;
    kh_foreach_ref(v->wrappers, k, w, *w->x64 = 0;);
    kh_destroy(x64wrappers, v->wrappers);
    box_free((void*)v->lib_name);
    v->lib_name = NULL;
    v->idx = 0;
}

#define MAPNAME3(N,M) N##M
#define MAPNAME2(N,M) MAPNAME3(N,M)
#define MAPNAME(N) MAPNAME2(GO_,N)

#define F1(A, B, N) RunFunctionFmt(my_##A##_fct_##N, B, a)
#define F2(A, B, N) RunFunctionFmt(my_##A##_fct_##N, B, a, b)
#define F3(A, B, N) RunFunctionFmt(my_##A##_fct_##N, B, a, b, c)
#define F4(A, B, N) RunFunctionFmt(my_##A##_fct_##N, B, a, b, c, d)
#define F5(A, B, N) RunFunctionFmt(my_##A##_fct_##N, B, a, b, c, d, e)
#define F6(A, B, N) RunFunctionFmt(my_##A##_fct_##N, B, a, b, c, d, e, f)
#define F7(A, B, N) RunFunctionFmt(my_##A##_fct_##N, B, a, b, c, d, e, f, g)
#define F8(A, B, N) RunFunctionFmt(my_##A##_fct_##N, B, a, b, c, d, e, f, g, h)
#define F9(A, B, N) RunFunctionFmt(my_##A##_fct_##N, B, a, b, c, d, e, f, g, h, i)
#define F10(A, B, N) RunFunctionFmt(my_##A##_fct_##N, B, a, b, c, d, e, f, g, h, i, j)
#define F11(A, B, N) RunFunctionFmt(my_##A##_fct_##N, B, a, b, c, d, e, f, g, h, i, j, k)
#define F15(A, B, N) RunFunctionFmt(my_##A##_fct_##N, B, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o)

#define GO_vFp(A, B, N)        static void my_##A##_##N(void* a) {F1(A, #B +2, N);}
#define GO_vFpi(A, B, N)       static void my_##A##_##N(void* a, int b) {F2(A, #B +2, N);}
#define GO_vFpp(A, B, N)       static void my_##A##_##N(void* a, void* b) {F2(A, #B +2, N);}
#define GO_vFpf(A, B, N)       static void my_##A##_##N(void* a, float b) {F2(A, #B +2, N);}
#define GO_vFpu(A, B, N)       static void my_##A##_##N(void* a, uint32_t b) {F2(A, #B +2, N);}
#define GO_vFpU(A, B, N)       static void my_##A##_##N(void* a, uint64_t b) {F2(A, #B +2, N);}
#define GO_vFpip(A, B, N)      static void my_##A##_##N(void* a, int b, void* c) {F3(A, #B +2, N);}
#define GO_vFppi(A, B, N)      static void my_##A##_##N(void* a, void* b, int c) {F3(A, #B +2, N);}
#define GO_vFppp(A, B, N)      static void my_##A##_##N(void* a, void* b, void* c) {F3(A, #B +2, N);}
#define GO_vFpff(A, B, N)      static void my_##A##_##N(void* a, float b, float c) {F3(A, #B +2, N);}
#define GO_vFpiU(A, B, N)      static void my_##A##_##N(void* a, int b, uint64_t c) {F3(A, #B +2, N);}
#define GO_vFpUi(A, B, N)      static void my_##A##_##N(void* a, uint64_t b, int c) {F3(A, #B +2, N);}
#define GO_vFppU(A, B, N)      static void my_##A##_##N(void* a, void* b, uint64_t c) {F3(A, #B +2, N);}
#define GO_vFpup(A, B, N)      static void my_##A##_##N(void* a, uint32_t b, void* c) {F3(A, #B +2, N);}
#define GO_vFpUp(A, B, N)      static void my_##A##_##N(void* a, uint64_t b, void* c) {F3(A, #B +2, N);}
#define GO_vFpUf(A, B, N)      static void my_##A##_##N(void* a, uint64_t b, float c) {F3(A, #B +2, N);}
#define GO_vFpUU(A, B, N)      static void my_##A##_##N(void* a, uint64_t b, uint64_t c) {F3(A, #B +2, N);}
#define GO_vFpUu(A, B, N)      static void my_##A##_##N(void* a, uint64_t b, uint32_t c) {F3(A, #B +2, N);}
#define GO_vFpuu(A, B, N)      static void my_##A##_##N(void* a, uint32_t b, uint32_t c) {F3(A, #B +2, N);}
#define GO_vFpuW(A, B, N)      static void my_##A##_##N(void* a, uint32_t b, uint16_t c) {F3(A, #B +2, N);}
#define GO_vFpipp(A, B, N)     static void my_##A##_##N(void* a, int b, void* c, void* d) {F4(A, #B +2, N);}
#define GO_vFpppp(A, B, N)     static void my_##A##_##N(void* a, void* b, void* c, void* d) {F4(A, #B +2, N);}
#define GO_vFpfff(A, B, N)     static void my_##A##_##N(void* a, float b, float c, float d) {F4(A, #B +2, N);}
#define GO_vFpiup(A, B, N)     static void my_##A##_##N(void* a, int b, uint32_t c, void* d) {F4(A, #B +2, N);}
#define GO_vFppLp(A, B, N)     static void my_##A##_##N(void* a, void* b, size_t c, void* d) {F4(A, #B +2, N);}
#define GO_vFpupp(A, B, N)     static void my_##A##_##N(void* a, uint32_t b, void* c, void* d) {F4(A, #B +2, N);}
#define GO_vFpUpp(A, B, N)     static void my_##A##_##N(void* a, uint64_t b, void* c, void* d) {F4(A, #B +2, N);}
#define GO_vFpiUu(A, B, N)     static void my_##A##_##N(void* a, int b, uint64_t c, uint32_t d) {F4(A, #B +2, N);}
#define GO_vFpUUi(A, B, N)     static void my_##A##_##N(void* a, uint64_t b, uint64_t c, int d) {F4(A, #B +2, N);}
#define GO_vFpuup(A, B, N)     static void my_##A##_##N(void* a, uint32_t b, uint32_t c, void* d) {F4(A, #B +2, N);}
#define GO_vFpUup(A, B, N)     static void my_##A##_##N(void* a, uint64_t b, uint32_t c, void* d) {F4(A, #B +2, N);}
#define GO_vFpUUp(A, B, N)     static void my_##A##_##N(void* a, uint64_t b, uint64_t c, void* d) {F4(A, #B +2, N);}
#define GO_vFpUUu(A, B, N)     static void my_##A##_##N(void* a, uint64_t b, uint64_t c, uint32_t d) {F4(A, #B +2, N);}
#define GO_vFpUuu(A, B, N)     static void my_##A##_##N(void* a, uint64_t b, uint32_t c, uint32_t d) {F4(A, #B +2, N);}
#define GO_vFpuuu(A, B, N)     static void my_##A##_##N(void* a, uint32_t b, uint32_t c, uint32_t d) {F4(A, #B +2, N);}
#define GO_vFpippp(A, B, N)    static void my_##A##_##N(void* a, int b, void* c, void* d, void* e) {F5(A, #B +2, N);}
#define GO_iFpUiUi(A, B, N)    static void my_##A##_##N(void* a, uint64_t b, int c, uint64_t d, int e) {F5(A, #B +2, N);}
#define GO_vFpiUUp(A, B, N)    static void my_##A##_##N(void* a, int b, uint64_t c, uint64_t d, void* e) {F5(A, #B +2, N);}
#define GO_vFpiUUu(A, B, N)    static void my_##A##_##N(void* a, int b, uint64_t c, uint64_t d, uint32_t e) {F5(A, #B +2, N);}
#define GO_vFpupup(A, B, N)    static void my_##A##_##N(void* a, uint32_t b, void* c, uint32_t d, void* e) {F5(A, #B +2, N);}
#define GO_vFpuupp(A, B, N)    static void my_##A##_##N(void* a, uint32_t b, uint32_t c, void* d, void* e) {F5(A, #B +2, N);}
#define GO_vFpUUUi(A, B, N)    static void my_##A##_##N(void* a, uint64_t b, uint64_t c, uint64_t d, int e) {F5(A, #B +2, N);}
#define GO_vFpUUUp(A, B, N)    static void my_##A##_##N(void* a, uint64_t b, uint64_t c, uint64_t d, void* e) {F5(A, #B +2, N);}
#define GO_vFpuuup(A, B, N)    static void my_##A##_##N(void* a, uint32_t b, uint32_t c, uint32_t d, void* e) {F5(A, #B +2, N);}
#define GO_vFpUUup(A, B, N)    static void my_##A##_##N(void* a, uint64_t b, uint64_t c, uint32_t d, void* e) {F5(A, #B +2, N);}
#define GO_vFpuuuu(A, B, N)    static void my_##A##_##N(void* a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {F5(A, #B +2, N);}
#define GO_vFpUuuu(A, B, N)    static void my_##A##_##N(void* a, uint64_t b, uint32_t c, uint32_t d, uint32_t e) {F5(A, #B +2, N);}
#define GO_vFpUUUu(A, B, N)    static void my_##A##_##N(void* a, uint64_t b, uint64_t c, uint64_t d, uint32_t e) {F5(A, #B +2, N);}
#define GO_vFpUUuu(A, B, N)    static void my_##A##_##N(void* a, uint64_t b, uint64_t c, uint32_t d, uint32_t e) {F5(A, #B +2, N);}
#define GO_vFpuiiii(A, B, N)   static void my_##A##_##N(void* a, uint32_t b, int c, int d, int e, int f) {F6(A, #B +2, N);}
#define GO_vFpupppp(A, B, N)   static void my_##A##_##N(void* a, uint32_t b, void* c, void* d, void* e, void* f) {F6(A, #B +2, N);}
#define GO_vFpppppU(A, B, N)   static void my_##A##_##N(void* a, void* b, void* c, void* d, void* e, uint64_t f) {F6(A, #B +2, N);}
#define GO_vFpUipup(A, B, N)   static void my_##A##_##N(void* a, uint64_t b, int c, void* d, uint32_t e, void* f) {F6(A, #B +2, N);}
#define GO_vFpUiUup(A, B, N)   static void my_##A##_##N(void* a, uint64_t b, int c, uint64_t d, uint32_t e, void* f) {F6(A, #B +2, N);}
#define GO_vFpuuppp(A, B, N)   static void my_##A##_##N(void* a, uint32_t b, uint32_t c, void* d, void* e, void* f) {F6(A, #B +2, N);}
#define GO_vFpiUuup(A, B, N)   static void my_##A##_##N(void* a, int b, uint64_t c, uint32_t d, uint32_t e, void* f) {F6(A, #B +2, N);}
#define GO_vFpUUiup(A, B, N)   static void my_##A##_##N(void* a, uint64_t b, uint64_t c, int d, uint32_t e, void* f) {F6(A, #B +2, N);}
#define GO_vFpupiUu(A, B, N)   static void my_##A##_##N(void* a, uint32_t b, void* c, int d, uint64_t e, uint32_t f) {F6(A, #B +2, N);}
#define GO_vFpuuuiu(A, B, N)   static void my_##A##_##N(void* a, uint32_t b, uint32_t c, uint32_t d, int e, uint32_t f) {F6(A, #B +2, N);}
#define GO_vFpUuuup(A, B, N)   static void my_##A##_##N(void* a, uint64_t b, uint32_t c, uint32_t d, uint32_t e, void* f) {F6(A, #B +2, N);}
#define GO_vFpupuuu(A, B, N)   static void my_##A##_##N(void* a, uint32_t b, void* c, uint32_t d, uint32_t e, uint32_t f) {F6(A, #B +2, N);}
#define GO_vFpUiUiup(A, B, N)  static void my_##A##_##N(void* a, uint64_t b, int c, uint64_t d, int e, uint32_t f, void* g) {F7(A, #B +2, N);}
#define GO_vFpuupppp(A, B, N)  static void my_##A##_##N(void* a, uint32_t b, uint32_t c, void* d, void* e, void* f, void* g) {F7(A, #B +2, N);}
#define GO_vFppUuupp(A, B, N)  static void my_##A##_##N(void* a, void* b, uint64_t c, uint32_t d, uint32_t e, void* f, void* g) {F7(A, #B +2, N);}
#define GO_vFpUuuUip(A, B, N)  static void my_##A##_##N(void* a, uint64_t b, uint32_t c, uint32_t d, uint64_t e, int f, void* g) {F7(A, #B +2, N);}
#define GO_vFpupuuup(A, B, N)  static void my_##A##_##N(void* a, uint32_t b, void* c, uint32_t d, uint32_t e, uint32_t f, void* g) {F7(A, #B +2, N);}
#define GO_vFpuuuuuu(A, B, N)  static void my_##A##_##N(void* a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f, uint32_t g) {F7(A, #B +2, N);}
#define GO_vFpuuUUuu(A, B, N)  static void my_##A##_##N(void* a, uint32_t b, uint32_t c, uint64_t d, uint64_t e, uint32_t f, uint32_t g) {F7(A, #B +2, N);}
#define GO_vFpUUUUuu(A, B, N)  static void my_##A##_##N(void* a, uint64_t b, uint64_t c, uint64_t d, uint64_t e, uint32_t f, uint32_t g) {F7(A, #B +2, N);}
#define GO_vFpiiiuipp(A, B, N) static void my_##A##_##N(void* a, int b, int c, int d, uint32_t e, int f, void* g, void* h) {F8(A, #B +2, N);}
#define GO_vFpUiUiupi(A, B, N) static void my_##A##_##N(void* a, uint64_t b, int c, uint64_t d, int e, uint32_t f, void* g, int h) {F8(A, #B +2, N);}
#define GO_vFpiUuupup(A, B, N) static void my_##A##_##N(void* a, int b, uint64_t c, uint32_t d, uint32_t e, void* f, uint32_t g, void* h) {F8(A, #B +2, N);}
#define GO_vFpuiULipp(A, B, N) static void my_##A##_##N(void* a, uint32_t b, int c, uint64_t d, size_t e, int f, void* g, void* h) {F8(A, #B +2, N);}
#define GO_vFpppppuuu(A, B, N) static void my_##A##_##N(void* a, void* b, void* c, void* d, void* e, uint32_t f, uint32_t g, uint32_t h) {F8(A, #B +2, N);}
#define GO_vFpUuuUUUu(A, B, N) static void my_##A##_##N(void* a, uint64_t b, uint32_t c, uint32_t d, uint64_t e, uint64_t f, uint64_t g, uint32_t h) {F8(A, #B +2, N);}
#define GO_vFppUUuUUUU(A, B, N)         static void my_##A##_##N(void* a, void* b, uint64_t c, uint64_t d, uint32_t e, uint64_t f, uint64_t g, uint64_t h, uint64_t i) {F9(A, #B +2, N);}
#define GO_vFpuuuupupup(A, B, N)        static void my_##A##_##N(void* a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, void* f, uint32_t g, void* h, uint32_t i, void* j) {F10(A, #B +2, N);}
#define GO_vFpupuuupupup(A, B, N)       static void my_##A##_##N(void* a, uint32_t b, void* c, uint32_t d, uint32_t e, uint32_t f, void* g, uint32_t h, void* i, uint32_t j, void* k) {F11(A, #B +2, N);}
#define GO_vFpUUUUUUUUUUUuuu(A, B, N)   static void my_##A##_##N(void* a, uint64_t b, uint64_t c, uint64_t d, uint64_t e, uint64_t f, uint64_t g, uint64_t h, uint64_t i, uint64_t j, uint64_t k, uint64_t l, uint32_t m, uint32_t n, uint32_t o) {F15(A, #B +2, N);}
#define GO_iFp(A, B, N)        static int my_##A##_##N(void* a) {return (int)F1(A, #B +2, N);}
#define GO_iFpp(A, B, N)       static int my_##A##_##N(void* a, void* b) {return (int)F2(A, #B +2, N);}
#define GO_iFpu(A, B, N)       static int my_##A##_##N(void* a, uint32_t b) {return (int)F2(A, #B +2, N);}
#define GO_iFpU(A, B, N)       static int my_##A##_##N(void* a, uint64_t b) {return (int)F2(A, #B +2, N);}
#define GO_iFpip(A, B, N)      static int my_##A##_##N(void* a, int b, void* c) {return (int)F3(A, #B +2, N);}
#define GO_iFppp(A, B, N)      static int my_##A##_##N(void* a, void* b, void* c) {return (int)F3(A, #B +2, N);}
#define GO_iFpiU(A, B, N)      static int my_##A##_##N(void* a, int b, uint64_t c) {return (int)F3(A, #B +2, N);}
#define GO_iFppU(A, B, N)      static int my_##A##_##N(void* a, void* b, uint64_t c) {return (int)F3(A, #B +2, N);}
#define GO_iFpup(A, B, N)      static int my_##A##_##N(void* a, uint32_t b, void* c) {return (int)F3(A, #B +2, N);}
#define GO_iFpUp(A, B, N)      static int my_##A##_##N(void* a, uint64_t b, void* c) {return (int)F3(A, #B +2, N);}
#define GO_iFpUu(A, B, N)      static int my_##A##_##N(void* a, uint64_t b, uint32_t c) {return (int)F3(A, #B +2, N);}
#define GO_iFpUU(A, B, N)      static int my_##A##_##N(void* a, uint64_t b, uint64_t c) {return (int)F3(A, #B +2, N);}
#define GO_iFpiip(A, B, N)     static int my_##A##_##N(void* a, int b, int c, void* d) {return (int)F4(A, #B +2, N);}
#define GO_iFpipp(A, B, N)     static int my_##A##_##N(void* a, int b, void* c, void* d) {return (int)F4(A, #B +2, N);}
#define GO_iFpppp(A, B, N)     static int my_##A##_##N(void* a, void* b, void* c, void* d) {return (int)F4(A, #B +2, N);}
#define GO_iFppip(A, B, N)     static int my_##A##_##N(void* a, void* b, int c, void* d) {return (int)F4(A, #B +2, N);}
#define GO_iFpiup(A, B, N)     static int my_##A##_##N(void* a, int b, uint32_t c, void* d) {return (int)F4(A, #B +2, N);}
#define GO_iFpUip(A, B, N)     static int my_##A##_##N(void* a, uint64_t b, int c, void* d) {return (int)F4(A, #B +2, N);}
#define GO_iFppup(A, B, N)     static int my_##A##_##N(void* a, void* b, uint32_t c, void* d) {return (int)F4(A, #B +2, N);}
#define GO_iFpupp(A, B, N)     static int my_##A##_##N(void* a, uint32_t b, void* c, void* d) {return (int)F4(A, #B +2, N);}
#define GO_iFpUpp(A, B, N)     static int my_##A##_##N(void* a, uint64_t b, void* c, void* d) {return (int)F4(A, #B +2, N);}
#define GO_iFpULp(A, B, N)     static int my_##A##_##N(void* a, uint64_t b, size_t c, void* d) {return (int)F4(A, #B +2, N);}
#define GO_iFpuUp(A, B, N)     static int my_##A##_##N(void* a, uint32_t b, uint64_t c, void* d) {return (int)F4(A, #B +2, N);}
#define GO_iFpupU(A, B, N)     static int my_##A##_##N(void* a, uint32_t b, void* c, uint64_t d) {return (int)F4(A, #B +2, N);}
#define GO_iFpUup(A, B, N)     static int my_##A##_##N(void* a, uint64_t b, uint32_t c, void* d) {return (int)F4(A, #B +2, N);}
#define GO_iFpUUu(A, B, N)     static int my_##A##_##N(void* a, uint64_t b, uint64_t c, uint32_t d) {return (int)F4(A, #B +2, N);}
#define GO_iFpUUU(A, B, N)     static int my_##A##_##N(void* a, uint64_t b, uint64_t c, uint64_t d) {return (int)F4(A, #B +2, N);}
#define GO_iFppppp(A, B, N)    static int my_##A##_##N(void* a, void* b, void* c, void* d, void* e) {return (int)F5(A, #B +2, N);}
#define GO_iFpuppp(A, B, N)    static int my_##A##_##N(void* a, uint32_t b, void* c, void* d, void* e) {return (int)F5(A, #B +2, N);}
#define GO_iFpUupp(A, B, N)    static int my_##A##_##N(void* a, uint64_t b, uint32_t c, void* d, void* e) {return (int)F5(A, #B +2, N);}
#define GO_iFpiUUU(A, B, N)    static int my_##A##_##N(void* a, int b, uint64_t c, uint64_t d, uint64_t e) {return (int)F5(A, #B +2, N);}
#define GO_iFpupuU(A, B, N)    static int my_##A##_##N(void* a, uint32_t b, void* c, uint32_t d, uint64_t e) {return (int)F5(A, #B +2, N);}
#define GO_iFpUiipp(A, B, N)   static int my_##A##_##N(void* a, uint64_t b, int c, int d, void* e, void* f) {return (int)F6(A, #B +2, N);}
#define GO_iFpUuuLp(A, B, N)   static int my_##A##_##N(void* a, uint64_t b, uint32_t c, uint32_t d, size_t e, void* f) {return (int)F6(A, #B +2, N);}
#define GO_iFpUUUup(A, B, N)   static int my_##A##_##N(void* a, uint64_t b, uint64_t c, uint64_t d, uint32_t e, void* f) {return (int)F6(A, #B +2, N);}
#define GO_iFpUUUUp(A, B, N)   static int my_##A##_##N(void* a, uint64_t b, uint64_t c, uint64_t d, uint64_t e, void* f) {return (int)F6(A, #B +2, N);}
#define GO_iFpiiiuup(A, B, N)  static int my_##A##_##N(void* a, int b, int c, int d, uint32_t e, uint32_t f, void* g) {return (int)F7(A, #B +2, N);}
#define GO_iFpupiLpL(A, B, N)  static int my_##A##_##N(void* a, uint32_t b, void* c, int d, size_t e, void* f, size_t g) {return (int)F7(A, #B +2, N);}
#define GO_iFpiiiuuup(A, B, N) static int my_##A##_##N(void* a, int b, int c, int d, uint32_t e, uint32_t f, uint32_t g, void* h) {return (int)F8(A, #B +2, N);}
#define GO_iFpUuuLpUu(A, B, N) static int my_##A##_##N(void* a, uint64_t b, uint32_t c, uint32_t d, size_t e, void* f, uint64_t g, uint32_t h) {return (int)F8(A, #B +2, N);}
#define GO_uFpp(A, B, N)       static uint32_t my_##A##_##N(void* a, void* b) {return (uint32_t)F2(A, #B +2, N);}
#define GO_uFpU(A, B, N)       static uint32_t my_##A##_##N(void* a, uint64_t b) {return (uint32_t)F2(A, #B +2, N);}
#define GO_uFpup(A, B, N)      static uint32_t my_##A##_##N(void* a, uint32_t b, void* c) {return (uint32_t)F3(A, #B +2, N);}
#define GO_uFpupp(A, B, N)     static uint32_t my_##A##_##N(void* a, uint32_t b, void* c, void* d) {return (uint32_t)F4(A, #B +2, N);}
#define GO_uFpubp(A, B, N)     static uint32_t my_##A##_##N(void* a, uint32_t b, void* c, void* d) {return (uint32_t)F4(A, #B +2, N);}
#define GO_UFpp(A, B, N)       static uint64_t my_##A##_##N(void* a, void* b) {return (uint64_t)F2(A, #B +2, N);}
#define GO_UFpu(A, B, N)       static uint64_t my_##A##_##N(void* a, uint32_t b) {return (uint64_t)F2(A, #B +2, N);}
#define GO_UFpUui(A, B, N)     static uint64_t my_##A##_##N(void* a, uint64_t b, uint32_t c, int d) {return (uint64_t)F4(A, #B +2, N);}

#define SUPER(A, B, C, N)    \
static uintptr_t my_##A##_fct_##N = 0;  \
B(A, C, N)

#define DATA(A, B)
#define GO2(A, B, C) error TODO
#define GOM(A, B)
#define GO(A, B)            \
SUPER(A, MAPNAME(B), B, 0)  \
SUPER(A, MAPNAME(B), B, 1)  \
SUPER(A, MAPNAME(B), B, 2)  \
SUPER(A, MAPNAME(B), B, 3)  \
static uintptr_t* my_##A##_fcts[MAXN] = {&my_##A##_fct_0, &my_##A##_fct_1, &my_##A##_fct_2,&my_##A##_fct_3}; \
static void* my_##A##_wrps[MAXN] = {my_##A##_0, my_##A##_1, my_##A##_2, my_##A##_3};

#include "wrapped/wrappedvulkan_private.h"

#undef GO
#undef GOM
#undef GO2
#undef DATA
#undef SUPER
#undef MAPNAME
#undef MAPNAME2
#undef MAPNAME3
#undef F14
#undef F11
#undef F10
#undef F9
#undef F8
#undef F7
#undef F6
#undef F5
#undef F4
#undef F3
#undef F2

typedef struct my_VkAllocationCallbacks_s {
    void*   pUserData;
    void*   pfnAllocation;
    void*   pfnReallocation;
    void*   pfnFree;
    void*   pfnInternalAllocation;
    void*   pfnInternalFree;
} my_VkAllocationCallbacks_t;

static void bridge_VkAllocationCallbacks(my_VkAllocationCallbacks_t* src)
{
    if(!src) return;
    if(src->pfnAllocation)          AddAutomaticBridge(my_context->system, pFpLLi, src->pfnAllocation, 0, "vulkan_Allocation");
    if(src->pfnReallocation)        AddAutomaticBridge(my_context->system, pFppLLi, src->pfnReallocation, 0, "vulkan_Reallocation");
    if(src->pfnFree)                AddAutomaticBridge(my_context->system, vFpp, src->pfnFree, 0, "vulkan_Free");
    if(src->pfnInternalAllocation)  AddAutomaticBridge(my_context->system, pFpLii, src->pfnInternalAllocation, 0, "vulkan_InternalAllocation");
    if(src->pfnInternalFree)        AddAutomaticBridge(my_context->system, pFpLii, src->pfnInternalFree, 0, "vulkan_InternalFree");
}

typedef struct my_VkStruct_s {
    int         sType;
    struct my_VkStruct_s* pNext;
} my_VkStruct_t;

typedef struct my_VkDebugReportCallbackCreateInfoEXT_s {
    int         sType;
    const void* pNext;
    int         flags;
    void*       pfnCallback;
    void*       pUserData;
} my_VkDebugReportCallbackCreateInfoEXT_t;

typedef struct my_VkDebugUtilsMessengerCreateInfoEXT_s {
    int          sType;
    const void*  pNext;
    int          flags;
    int          messageSeverity;
    int          messageType;
    void*        pfnUserCallback;
    void*        pUserData;
} my_VkDebugUtilsMessengerCreateInfoEXT_t;

typedef struct my_VkLayerInstanceLink_s {
    struct my_VkLayerInstanceLink_s*    pNext;
    void*                               pfnNextGetInstanceProcAddr;
    void*                               pfnNextGetPhysicalDeviceProcAddr;
} my_VkLayerInstanceLink_t;

typedef struct my_VkNegotiateLayerInterface_s {
    int         sType;
    void*       pNext;
    uint32_t    loaderLayerInterfaceVersion;
    void*       pfnGetInstanceProcAddr;
    void*       pfnGetDeviceProcAddr;
    void*       pfnGetPhysicalDeviceProcAddr;
} my_VkNegotiateLayerInterface_t;

typedef struct my_VkLayerInstanceCreateInfo_s {
    int             sType; // VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO
    const void*     pNext;
    int             function;
    union {
        my_VkLayerInstanceLink_t*   pLayerInfo;
        void*                       pfnSetInstanceLoaderData;   //iFpp
        struct {
                void*               pfnLayerCreateDevice;   //iFppppppp (many callback, but should be ok)
                void*               pfnLayerDestroyDevice;  //vFppp
            } layerDevice;
        uint32_t                    loaderFeatures;
    } u;
} my_VkLayerInstanceCreateInfo_t;

typedef struct my_VkLayerDeviceLink_s {
    struct my_VkLayerDeviceLink_s*  pNext;
    void*                           pfnNextGetInstanceProcAddr;
    void*                           pfnNextGetDeviceProcAddr;
} my_VkLayerDeviceLink_t;

typedef struct my_VkLayerDeviceCreateInfo_s {
    int         sType;  // VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO
    const void* pNext;
    int         function;
    union {
        my_VkLayerDeviceLink_t* pLayerInfo;
        void*                   pfnSetDeviceLoaderData; //iFpp
    } u;
} my_VkLayerDeviceCreateInfo_t;

void* my_GetVkProcAddr2(x64emu_t* emu, void* a, void* name, void*(*getaddr)(void* a, const char*));
#define SUPER() \
GO(0)           \
GO(1)           \
GO(2)           \
GO(3)           \
GO(4)           \
GO(5)           \
GO(6)           \
GO(7)

#define GO(N)   \
static void* my_getVkXXXXProcAddr_fct_##N = NULL;                                           \
static void* my_getVkXXXXProcAddr_##N(void* a, const char* b)                               \
{                                                                                           \
    return my_GetVkProcAddr2(thread_get_emu(), a, (void*)b, my_getVkXXXXProcAddr_fct_##N);  \
}
SUPER()
#undef GO
static void addAutoBridgeVkProcAddr(void* f)
{
    if(!f) return;
    #define GO(N) if(my_getVkXXXXProcAddr_fct_##N==f) return;
    SUPER()
    #undef GO
    #define GO(N) if(!my_getVkXXXXProcAddr_fct_##N) {   \
        my_getVkXXXXProcAddr_fct_##N = f;               \
        AddAutomaticBridgeAlt(my_context->system, pFpp, f, my_getVkXXXXProcAddr_##N, 0, "vulkan_layer_getinstanceprocadr"); \
        return;                                         \
    }
    SUPER()
    #undef GO
    printf_log(LOG_INFO, "Warning, no more slot for addAutoBridgeVkProcAddr\n");
}
#undef SUPER

void addBridgeVkLayerDeviceLink(my_VkLayerDeviceLink_t* p) {
    while(p) {
        addAutoBridgeVkProcAddr(p->pfnNextGetInstanceProcAddr);
        addAutoBridgeVkProcAddr(p->pfnNextGetDeviceProcAddr);
        p = p->pNext;
    }
}
void addBridgeVkLayerInstanceLink(my_VkLayerInstanceLink_t* p) {
    while(p) {
        addAutoBridgeVkProcAddr(p->pfnNextGetInstanceProcAddr);
        addAutoBridgeVkProcAddr(p->pfnNextGetPhysicalDeviceProcAddr);
        p = p->pNext;
    }
}


#define VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT 1000011000
#define VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT 1000128004
#define VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO           47
#define VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO             48
static void addBridgeCreateInfo(my_VkStruct_t* p)
{
    while(p) {
        if(p->sType==VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT) {
            my_VkDebugReportCallbackCreateInfoEXT_t* vk = (my_VkDebugReportCallbackCreateInfoEXT_t*)p;
            AddAutomaticBridge(my_context->system, iFiiULippp, vk->pfnCallback, 0, "vulkan_debug");
        } else if(p->sType==VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT) {
            my_VkDebugUtilsMessengerCreateInfoEXT_t* vk = (my_VkDebugUtilsMessengerCreateInfoEXT_t*)p;
            AddAutomaticBridge(my_context->system, iFiipp, vk->pfnUserCallback, 0, "vulkan_debug");
        } else if(p->sType==VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO) {
            my_VkLayerDeviceCreateInfo_t* vk = (my_VkLayerDeviceCreateInfo_t*)p;
            switch(vk->function) {
                case 0:
                    addBridgeVkLayerDeviceLink(vk->u.pLayerInfo);
                    break;
                case 1:
                    AddAutomaticBridge(my_context->system, iFpp, vk->u.pfnSetDeviceLoaderData, 0, "vulkan_layer_setdeviceloaderdata");
                    break;
            }
        } else if(p->sType==VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO) {
            my_VkLayerInstanceCreateInfo_t* vk = (my_VkLayerInstanceCreateInfo_t*)p;
            switch(vk->function) {
                case 0: addBridgeVkLayerInstanceLink(vk->u.pLayerInfo); break;
                case 1: AddAutomaticBridge(my_context->system, iFpp, vk->u.pfnSetInstanceLoaderData, 0, "vulkan_layer_setinstanceloaderdata"); break;
                case 2: 
                    AddAutomaticBridge(my_context->system, iFppppppp, vk->u.layerDevice.pfnLayerCreateDevice, 0, "vulkan_layer_createdevice"); 
                    AddAutomaticBridge(my_context->system, vFppp, vk->u.layerDevice.pfnLayerDestroyDevice, 0, "vulkan_layer_destroydevice"); 
                    break;
            }
        }
        p = p->pNext;
    }
}

typedef struct my_VkXcbSurfaceCreateInfoKHR_s {
    int         sType;
    const void* pNext;
    uint32_t    flags;
    void**      connection;
    int         window;
} my_VkXcbSurfaceCreateInfoKHR_t;

#define SUPER(A)    \
GO(A, 0)            \
GO(A, 1)            \
GO(A, 2)            \
GO(A, 3)            \
static uintptr_t* my_##A##_fcts[MAXN] = {&my_##A##_fct_0, &my_##A##_fct_1, &my_##A##_fct_2,&my_##A##_fct_3}; \
static void* my_##A##_wrps[MAXN] = {my_##A##_0, my_##A##_1, my_##A##_2, my_##A##_3};

//vkGetInstanceProcAddr
#define GO(B, A)   \
static uintptr_t my_vkGetInstanceProcAddr_fct_##A = 0;                                                      \
static void* my_vkGetInstanceProcAddr_##A(void* a, void* b)                                                 \
{                                                                                                           \
    printf_dlsym(LOG_DEBUG, "my_vkGetInstanceProcAddr_%d(%p, %s) =>", A, a, b);                             \
    uintptr_t addr = RunFunctionFmt(my_vkGetInstanceProcAddr_fct_##A, "pp", a, b);                          \
    printf_dlsym_prefix(0, LOG_DEBUG, " %p =>", (void*)addr);                                               \
    void* ret = wrapVulkanSymbol(&overlays[A], b, addr);                                                    \
    printf_dlsym_prefix(0, LOG_DEBUG, " %p\n", ret);                                                        \
    return ret;                                                                                             \
}
SUPER(vkGetInstanceProcAddr)
#undef GO
//vkGetDeviceProcAddr
#define GO(B, A)   \
static uintptr_t my_vkGetDeviceProcAddr_fct_##A = 0;                                                        \
static void* my_vkGetDeviceProcAddr_##A(void* a, void* b)                                                   \
{                                                                                                           \
    printf_dlsym(LOG_DEBUG, "my_vkGetDeviceProcAddr_%d(%p, %s)\n", A, a, b);                                \
    return wrapVulkanSymbol(&overlays[A], b, RunFunctionFmt(my_vkGetDeviceProcAddr_fct_##A, "pp", a, b));   \
}
SUPER(vkGetDeviceProcAddr)
#undef GO
// All the CREATE
#define GO(A, N)   \
static uintptr_t my_##A##_fct_##N = 0;                                                      \
static void* my_##A##_##N(void* a, void* b, my_VkAllocationCallbacks_t* c, void* d)         \
{                                                                                           \
    bridge_VkAllocationCallbacks(c);                                                        \
    return (void*)RunFunctionFmt(my_##A##_fct_##N, "pppp", a, b, c, d);                     \
}
#define CREATE(A) SUPER(A)
CREATE(vkAllocateMemory)
CREATE(vkCreateBuffer)
CREATE(vkCreateBufferView)
CREATE(vkCreateCommandPool)
CREATE(vkCreateDescriptorPool)
CREATE(vkCreateDescriptorSetLayout)
CREATE(vkCreateDescriptorUpdateTemplate)
CREATE(vkCreateDescriptorUpdateTemplateKHR)
//CREATE(vkCreateDevice)
CREATE(vkCreateDisplayPlaneSurfaceKHR)
CREATE(vkCreateEvent)
CREATE(vkCreateFence)
CREATE(vkCreateFramebuffer)
CREATE(vkCreateImage)
CREATE(vkCreateImageView)
CREATE(vkCreatePipelineCache)
CREATE(vkCreatePipelineLayout)
CREATE(vkCreateQueryPool)
CREATE(vkCreateRenderPass)
CREATE(vkCreateSampler)
CREATE(vkCreateSamplerYcbcrConversion)
CREATE(vkCreateSemaphore)
CREATE(vkCreateShaderModule)
CREATE(vkCreateSwapchainKHR)
CREATE(vkCreateWaylandSurfaceKHR)
CREATE(vkCreateXlibSurfaceKHR)
CREATE(vkCreateAndroidSurfaceKHR)
CREATE(vkCreateRenderPass2)
CREATE(vkCreateRenderPass2KHR)
CREATE(vkCreateValidationCacheEXT)
CREATE(vkCreateSamplerYcbcrConversionKHR)
CREATE(vkCreateVideoSessionKHR)
CREATE(vkCreateVideoSessionParametersKHR)
CREATE(vkCreatePrivateDataSlot)
CREATE(vkCreatePrivateDataSlotEXT)
CREATE(vkCreateAccelerationStructureKHR)
CREATE(vkCreateCuFunctionNVX)
CREATE(vkCreateCuModuleNVX)
CREATE(vkCreateIndirectCommandsLayoutNV)
CREATE(vkCreateAccelerationStructureNV)
CREATE(vkCreateOpticalFlowSessionNV)
CREATE(vkCreateMicromapEXT)
CREATE(vkCreateCudaFunctionNV)
CREATE(vkCreateCudaModuleNV)
CREATE(vkCreateHeadlessSurfaceEXT)
CREATE(vkCreateIndirectCommandsLayoutEXT)
CREATE(vkCreateIndirectExecutionSetEXT)
CREATE(vkCreatePipelineBinariesKHR)
CREATE(vkCreateTensorARM)
CREATE(vkCreateTensorViewARM)
CREATE(vkCreateDataGraphPipelineSessionARM)
CREATE(vkCreateDebugReportCallbackEXT)  // needs better wrapping because of Debug callback?
CREATE(vkCreateDebugUtilsMessengerEXT)  // Needs better wrapping?
CREATE(vkRegisterDeviceEventEXT)
#undef CREATE
#undef GO
// DESTROY64
#define GO(A, N)    \
static uintptr_t my_##A##_fct_##N = 0;                                        \
static void my_##A##_##N(void* a, uint64_t b, my_VkAllocationCallbacks_t* c)  \
{                                                                             \
    bridge_VkAllocationCallbacks(c);                                          \
    RunFunctionFmt(my_##A##_fct_##N, "pUp", a, b, c);                         \
}
#define DESTROY64(A) SUPER(A)
DESTROY64(vkDestroyShaderEXT)
DESTROY64(vkDestroyBuffer)
DESTROY64(vkDestroyBufferView)
DESTROY64(vkDestroyCommandPool)
DESTROY64(vkDestroyDescriptorPool)
DESTROY64(vkDestroyDescriptorSetLayout)
DESTROY64(vkDestroyDescriptorUpdateTemplate)
DESTROY64(vkDestroyDescriptorUpdateTemplateKHR)
DESTROY64(vkDestroyEvent)
DESTROY64(vkDestroyFence)
DESTROY64(vkDestroyFramebuffer)
DESTROY64(vkDestroyImage)
DESTROY64(vkDestroyImageView)
DESTROY64(vkDestroyPipeline)
DESTROY64(vkDestroyPipelineCache)
DESTROY64(vkDestroyPipelineLayout)
DESTROY64(vkDestroyQueryPool)
DESTROY64(vkDestroyRenderPass)
DESTROY64(vkDestroySampler)
DESTROY64(vkDestroySamplerYcbcrConversion)
DESTROY64(vkDestroySemaphore)
DESTROY64(vkDestroyShaderModule)
DESTROY64(vkDestroySwapchainKHR)
DESTROY64(vkFreeMemory)
DESTROY64(vkDestroySurfaceKHR)
DESTROY64(vkDestroySamplerYcbcrConversionKHR)
DESTROY64(vkDestroyValidationCacheEXT)
DESTROY64(vkDestroyVideoSessionKHR)
DESTROY64(vkDestroyVideoSessionParametersKHR)
DESTROY64(vkDestroyPrivateDataSlot)
DESTROY64(vkDestroyPrivateDataSlotEXT)
DESTROY64(vkDestroyAccelerationStructureKHR)
DESTROY64(vkDestroyDeferredOperationKHR)
DESTROY64(vkDestroyCuFunctionNVX)
DESTROY64(vkDestroyCuModuleNVX)
DESTROY64(vkDestroyIndirectCommandsLayoutNV)
DESTROY64(vkDestroyAccelerationStructureNV)
DESTROY64(vkDestroyOpticalFlowSessionNV)
DESTROY64(vkDestroyMicromapEXT)
DESTROY64(vkDestroyCudaFunctionNV)
DESTROY64(vkDestroyCudaModuleNV)
DESTROY64(vkDestroyIndirectCommandsLayoutEXT)
DESTROY64(vkDestroyIndirectExecutionSetEXT)
DESTROY64(vkDestroyPipelineBinaryKHR)
DESTROY64(vkDestroyTensorARM)
DESTROY64(vkDestroyTensorViewARM)
DESTROY64(vkDestroyDataGraphPipelineSessionARM)
#undef DESTROY64
#undef GO
// DESTROY
#define GO(A, N)    \
static uintptr_t my_##A##_fct_##N = 0;                                        \
static void my_##A##_##N(void* a, void* b, my_VkAllocationCallbacks_t* c)     \
{                                                                             \
    bridge_VkAllocationCallbacks(c);                                          \
    RunFunctionFmt(my_##A##_fct_##N, "ppp", a, b, c);                         \
}
#define DESTROY(A) SUPER(A)
DESTROY(vkDestroyDebugUtilsMessengerEXT)
DESTROY(vkReleaseCapturedPipelineDataKHR)
DESTROY(vkDestroyDebugReportCallbackEXT)    // needs better wrapping?
#undef DESTROY
#undef GO
//vkCreate*Pipelines
#define GO(A, N)   \
static uintptr_t my_##A##_fct_##N = 0;                                                                      \
static int my_##A##_##N(void* a, uint64_t b, uint32_t c, void* d, my_VkAllocationCallbacks_t* e, void* f)   \
{                                                                                                           \
    bridge_VkAllocationCallbacks(e);                                                                        \
    return (int)RunFunctionFmt(my_##A##_fct_##N, "pUuppp", a, b, c, d, e, f);                               \
}
SUPER(vkCreateComputePipelines)
SUPER(vkCreateGraphicsPipelines)
SUPER(vkCreateRayTracingPipelinesNV)
SUPER(vkCreateExecutionGraphPipelinesAMDX)

#undef GO
// Simpler DESTROY
#define GO(A, N)    \
static uintptr_t my_##A##_fct_##N = 0;                              \
static void my_##A##_##N(void* a, my_VkAllocationCallbacks_t* b)    \
{                                                                   \
    bridge_VkAllocationCallbacks(b);                                \
    RunFunctionFmt(my_##A##_fct_##N, "pp", a, b);                   \
}
SUPER(vkDestroyDevice)
SUPER(vkDestroyInstance)
#undef GO
// vkGetPhysicalDeviceMemoryProperties
#define GO(A, N)    \
static uintptr_t my_##A##_fct_##N = 0;                              \
static void my_##A##_##N(void* a, void* b)                          \
{                                                                   \
    RunFunctionFmt(my_##A##_fct_##N, "pp", a, b);                   \
}
SUPER(vkGetPhysicalDeviceProperties2)
#undef GO
//vkCreateDisplayModeKHR
#define GO(A, N)   \
static uintptr_t my_##A##_fct_##N = 0;                                                          \
static int my_##A##_##N(void* a, uint64_t b, void* c, my_VkAllocationCallbacks_t* d, void* e)   \
{                                                                                               \
    bridge_VkAllocationCallbacks(d);                                                            \
    return (int)RunFunctionFmt(my_##A##_fct_##N, "pUppp", a, b, c, d, e);                       \
}
SUPER(vkCreateDisplayModeKHR)
SUPER(vkRegisterDisplayEventEXT)
#undef GO
//vkCreateSharedSwapchainsKHR
#define GO(A, N)   \
static uintptr_t my_##A##_fct_##N = 0;                                                          \
static int my_##A##_##N(void* a, uint32_t b, void* c, my_VkAllocationCallbacks_t* d, void* e)   \
{                                                                                               \
    bridge_VkAllocationCallbacks(d);                                                            \
    return (int)RunFunctionFmt(my_##A##_fct_##N, "puppp", a, b, c, d, e);                       \
}
SUPER(vkCreateSharedSwapchainsKHR)
SUPER(vkCreateShadersEXT)
#undef GO
//CREATE 3p
#define GO(A, N)   \
static uintptr_t my_##A##_fct_##N = 0;                                      \
static int my_##A##_##N(void* a, my_VkAllocationCallbacks_t* b, void* c)    \
{                                                                           \
    bridge_VkAllocationCallbacks(b);                                        \
    return (int)RunFunctionFmt(my_##A##_fct_##N, "ppp", a, b, c);           \
}
SUPER(vkCreateDeferredOperationKHR)
#undef GO
//vkCreateRayTracingPipelinesKHR
#define GO(A, N)   \
static uintptr_t my_##A##_fct_##N = 0;                                                                                  \
static int my_##A##_##N(void* a, uint64_t b, uint64_t c, uint32_t d, void* e, my_VkAllocationCallbacks_t* f, void* g)   \
{                                                                                                                       \
    bridge_VkAllocationCallbacks(f);                                                                                    \
    return (int)RunFunctionFmt(my_##A##_fct_##N, "pUUuppp", a, b, c, d, e, f, g);                                       \
}
SUPER(vkCreateRayTracingPipelinesKHR)
SUPER(vkCreateDataGraphPipelinesARM)
#undef GO
#define GO(A, N)   \
static uintptr_t my_##A##_fct_##N = 0;                                      \
static int my_##A##_##N(void* a, my_VkAllocationCallbacks_t* b, void* c)    \
{                                                                           \
    addBridgeCreateInfo(a);                                                 \
    bridge_VkAllocationCallbacks(b);                                        \
    return (int)RunFunctionFmt(my_##A##_fct_##N, "ppp", a, b, c);           \
}
SUPER(vkCreateInstance)
#undef GO
#define GO(A, N)   \
static uintptr_t my_##A##_fct_##N = 0;                                              \
static int my_##A##_##N(void* a, void* b, my_VkAllocationCallbacks_t* c, void* d)   \
{                                                                                   \
    addBridgeCreateInfo(b);                                                         \
    bridge_VkAllocationCallbacks(c);                                                \
    return (int)RunFunctionFmt(my_##A##_fct_##N, "pppp", a, b, c, d);               \
}
SUPER(vkCreateDevice)
#undef GO
// vkCreateXcbSurfaceKHR
#define GO(A, N)   \
static uintptr_t my_##A##_fct_##N = 0;                                                      \
static void* my_##A##_##N(void* a, void* b, my_VkAllocationCallbacks_t* c, void* d)         \
{                                                                                           \
    bridge_VkAllocationCallbacks(c);                                                        \
    my_VkXcbSurfaceCreateInfoKHR_t* surfaceinfo = b;                                        \
    void* old_conn = surfaceinfo->connection;                                               \
    surfaceinfo->connection = add_xcb_connection(old_conn);                                 \
    void* ret = (void*)RunFunctionFmt(my_##A##_fct_##N, "pppp", a, b, c, d);                \
    surfaceinfo->connection = old_conn;                                                     \
    return ret;                                                                             \
}
SUPER(vkCreateXcbSurfaceKHR)
#undef GO

#undef SUPER

static json_value_t* json_find(json_object_t* object, const char* key)
{
    json_object_element_t *element = object->start;
    while (element) {
        if (strcmp(element->name->string, key) == 0) {
            return element->value;
        }
        element = element->next;
    }
    return NULL;
}

static void addVulkanSymbol(my_vulkanoverlay_t* v, const char* name, uintptr_t* x64, void* f)
{
    int ret;
    khint_t k = kh_put(x64wrappers, v->wrappers, name, &ret);
    kh_value(v->wrappers, k).x64 = x64;
    kh_value(v->wrappers, k).f = f;
}

void* LoadVulkanOverlay(const char* path, int flags)
{
    // find the local vulkan/implicit_layer.d
    {
        // simplified research pattern...
        glob_t pglob = {0};
        char tmp[4096] = {0};
        if(getenv("XDG_CONFIG_HOME")) {
            snprintf(tmp, sizeof(tmp)-1, "%s/vulkan/implicit_layer.d/*.json", getenv("XDG_CONFIG_HOME"));
        } else if(getenv("HOME")) {
            snprintf(tmp, sizeof(tmp)-1, "%s/.config/vulkan/implicit_layer.d/*.json", getenv("HOME"));
        }
        int flags = GLOB_APPEND;
        if(strlen(tmp)) glob(tmp, 0, NULL, &pglob); else flags = 0;
        if(getenv("XDG_CONFIG_DIRS")) {
            snprintf(tmp, sizeof(tmp)-1, "%s/vulkan/implicit_layer.d/*.json", getenv("XDG_CONFIG_DIRS"));
        } else {
            snprintf(tmp, sizeof(tmp)-1, "/etc/xdg/vulkan/implicit_layer.d/*.json");
        }
        glob(tmp, flags, NULL, &pglob);
        flags = GLOB_APPEND;
        tmp[0] = 0;
        if(getenv("XDG_DATA_HOME")) {
            snprintf(tmp, sizeof(tmp)-1, "%s/vulkan/implicit_layer.d/*.json", getenv("XDG_DATA_HOME"));
        } else if(getenv("HOME")) {
            snprintf(tmp, sizeof(tmp)-1, "%s/.local/share/vulkan/implicit_layer.d/*.json", getenv("HOME"));
        }
        glob(tmp, flags, NULL, &pglob);
        // should also search XDG_DATA_DIRS, but it's a list, so a bit anoying to do...

        // now, look through all the json collected to find one that reference the lib
        json_value_t* json = NULL;
        for(size_t i=0; i<pglob.gl_pathc && !json; ++i) {
            char* src = NULL;
            size_t sz = FileSize(pglob.gl_pathv[i]);
            src = box_malloc(sz);
            FILE* f = fopen(pglob.gl_pathv[i], "r");
            if(f) {
                if(fread(src, sz, 1, f)==1) {
                    json = json_parse(src, sz);
                    int keep = 0;
                    if(json && (json->type==json_type_object)) {
                        json_value_t* layer = json_find(json->payload, "layer");
                        if(layer && (layer->type==json_type_object)) {
                            json_value_t* libpath = json_find(layer->payload, "library_path");
                            if(libpath && libpath->type==json_type_string) {
                                json_string_t* string = libpath->payload;
                                if(!strcmp(string->string, path))
                                    keep = 1;
                            }
                        }
                    }
                    if(!keep) {
                        free(json);
                        json = NULL;
                    }
                }
                fclose(f);
                box_free(src);
                src = NULL;
                f = NULL;
            }
        }
        globfree(&pglob);
        if(json) {
            printf_log(LOG_INFO, "Found a keeper: %s\n", path);
            my_vulkanoverlay_t* v = getNewVulkanOverlay();
            if(!v) {
                printf_log(LOG_INFO, "Warning, no more slot for a new Vulkan Overlay (%s)\n", path);
            } else {
                json_value_t* layer = json_find(json->payload, "layer");
                json_value_t* functions = json_find(layer->payload, "functions");
                if(functions && functions->type==json_type_object) {
                    json_object_t* funcs = functions->payload;
                    // there is an array on transformed functions
                    v->functions = calloc(funcs->length, sizeof(v->functions[0]));
                    json_object_element_t* e = funcs->start;
                    uint32_t idx = 0;
                    while(e) {
                        if(e->value->type==json_type_string) {
                            v->functions[idx].name = box_strdup(e->name->string);
                            v->functions[idx].f = box_strdup(((json_string_t*)e->value->payload)->string);
                            ++idx;
                        }
                        e = e->next;
                    }
                    v->n_functions = idx;
                }
                printf_log(LOG_INFO, "\twith %d functions name overrides\n", v->n_functions);
            }
            // all done with json, it can be freed now
            free(json);
            json = NULL;
            // my_dlopen it now
            if(v) {
                x64emu_t* emu = thread_get_emu();
                void* handle = my_dlopen(emu, path, flags);
                if(!handle) {
                    printf_log(LOG_INFO, "\tFailed to open the lib\n");
                    // there was an error, cancel...
                    freeVulkanOverlay(v);
                    return NULL;
                }
                v->handle = handle;
                v->lib_name = box_strdup(strrchr(path, '/')+1);
                #define DATA(A, B)
                #define GO2(A, B,C)
                #define GOM(A, B) addVulkanSymbol(v, #A, my_##A##_fcts[v->idx], my_##A##_wrps[v->idx]);
                #define GO(A, B) addVulkanSymbol(v, #A, my_##A##_fcts[v->idx], my_##A##_wrps[v->idx]);
                #include "wrapped/wrappedvulkan_private.h"
                #undef GO
                #undef GOM
                #undef GO2
                #undef DATA
                return v;
            }
        }
    }

    return NULL;
}
void* GetSymbolVulkanOverlay(void* g, const char* sym)
{
    my_vulkanoverlay_t* v = g;
    if(v) {
        x64emu_t* emu = thread_get_emu();
        const char* name = NULL;
        for(size_t i=0; i<v->n_functions && !name; ++i) {
            if(!strcmp(v->functions[i].f, sym))
                name = v->functions[i].name;
        }
        if(!name) name = sym;
        size_t idx = v->idx;
        printf_log(LOG_INFO, "Hooked dlsym on VulkanOverlay[%d/%s] with function %s (%s)\n", idx, v->lib_name, sym, name);
        if(!strcmp(name, "vkGetDeviceProcAddr")) {
            uintptr_t x64 = (uintptr_t)my_dlsym(emu, v->handle, sym);
            if(!x64) return NULL;
            *my_vkGetDeviceProcAddr_fcts[idx] = x64;
            return my_vkGetDeviceProcAddr_wrps[idx];
        }
        if(!strcmp(name, "vkGetInstanceProcAddr")) {
            uintptr_t x64 = (uintptr_t)my_dlsym(emu, v->handle, sym);
            if(!x64) return NULL;
            *my_vkGetInstanceProcAddr_fcts[idx] = x64;
            return my_vkGetInstanceProcAddr_wrps[idx];
        }
        
    }
    return NULL;
}
int FreeVulkanOverlay(void* g)
{
    my_vulkanoverlay_t* v = g;
    if(v) {
        if(v->n_functions) box_free(v->functions);
        x64emu_t* emu = thread_get_emu();
        my_dlclose(emu, v->handle);
        freeVulkanOverlay(v);
    }
    return 0;
}
