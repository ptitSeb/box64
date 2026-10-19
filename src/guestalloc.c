// SPDX-License-Identifier: MIT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <malloc.h>

#include "box64context.h"
#include "debug.h"
#include "callback.h"
#include "librarian.h"
#include "elfs/elfloader_private.h"
#include "elfloader.h"
#include "custommem.h"
#include "env.h"
#include "bridge.h"
#include "alternate.h"
#include "guestalloc.h"
#ifdef BOX32
#include "box32.h"
#endif

extern int box64_quit;

#if defined(BOX32) && defined(PPC64LE)
static void checkOggHookedSymbols32(elfheader_t* h);
#endif

size_t (*box_malloc_usable_size)(void*) = NULL;

#ifndef ANDROID

char* box_strdup(const char* s)
{
    char* ret = box_calloc(1, strlen(s)+1);
    memcpy(ret, s, strlen(s));
    return ret;
}

char* box_realpath(const char* path, char* ret)
{
    if(ret)
        return realpath(path, ret);
#ifdef PATH_MAX
    size_t path_max = PATH_MAX;
#else
    size_t path_max = pathconf(path, _PC_PATH_MAX);
    if (path_max <= 0)
        path_max = 4096;
#endif
    char tmp[path_max];
    char* p = realpath(path, tmp);
    if(!p)
        return NULL;
    return box_strdup(tmp);
}

static size_t pot(size_t l)
{
    size_t ret = 0;
    while (l>(1u<<ret)) ++ret;
    return 1u<<ret;
}

#ifdef BOX32
int isCustomAddr(void* p);
#define FITS_IN_32BIT(ptr, size) (((uintptr_t)(ptr) + (size)) <= 0x100000000ULL)
void* box32_calloc(size_t n, size_t s)
{
    void* ret = box_calloc(n, s);
    if(ret && FITS_IN_32BIT(ret, n * s)) return ret;
    box_free(ret);
#ifdef HAVE_MALLOC_TRIM
    malloc_trim(0);
#endif
    ret = box_calloc(n, s);
    if(ret && FITS_IN_32BIT(ret, n * s)) return ret;
    box_free(ret);
    return customCalloc32(n, s);
}
void* box32_malloc(size_t s)
{
    void* ret = box_malloc(s);
    if(ret && FITS_IN_32BIT(ret, s)) return ret;
    box_free(ret);
#ifdef HAVE_MALLOC_TRIM
    malloc_trim(0);
#endif
    ret = box_malloc(s);
    if(ret && FITS_IN_32BIT(ret, s)) return ret;
    box_free(ret);
    return customMalloc32(s);
}
void* box32_realloc(void* p, size_t s)
{
    if(isCustomAddr(p))
        return customRealloc32(p, s);
    void* ret = box_realloc(p, s);
    if(!ret) return NULL;
    if(FITS_IN_32BIT(ret, s)) return ret;
#ifdef HAVE_MALLOC_TRIM
    malloc_trim(0);
#endif
    void* newret = customMalloc32(s);
    memcpy(newret, ret, s);
    box_free(ret);
    return newret;
}
void box32_free(void* p)
{
    if(isCustomAddr(p))
        customFree32(p);
    else
        box_free(p);
}
void* box32_memalign(size_t align, size_t s)
{
    void* ret = box_memalign(align, s);
    if(ret && FITS_IN_32BIT(ret, s)) return ret;
    box_free(ret);
#ifdef HAVE_MALLOC_TRIM
    malloc_trim(0);
#endif
    return customMemAligned32(align, s);
}
size_t box32_malloc_usable_size(void* p)
{
    if(isCustomAddr(p))
        return customGetUsableSize(p);
    else
        return box_malloc_usable_size(p);
}
char* box32_strdup(const char* s)
{
    char* ret = box32_calloc(1, strlen(s)+1);
    memcpy(ret, s, strlen(s));
    return ret;
}
#endif

static int ga_active = 0;
static uintptr_t ga_malloc_f = 0;
static uintptr_t ga_free_f = 0;
static uintptr_t ga_calloc_f = 0;
static uintptr_t ga_realloc_f = 0;
static uintptr_t ga_memalign_f = 0;
static uintptr_t ga_posix_memalign_f = 0;
static uintptr_t ga_aligned_alloc_f = 0;
static uintptr_t ga_usable_f = 0;
static __thread int ga_in_guest = 0;

static uintptr_t ga_resolve_guest(const char* name)
{
    uintptr_t start = 0, end = 0;
    void* elfsym = NULL;
    int found;
    if(!my_context || !my_context->maplib)
        return 0;
    lockLibLoader();
    found = GetGlobalSymbolStartEnd(my_context->maplib, name, &start, &end, (void*)1, -1, NULL, 0, &elfsym);
    unlockLibLoader();
    if(!found || !start || !elfsym)
        return 0;
    return start;
}

void guest_alloc_refresh(void)
{
    if(box64_is32bits || box64_quit)
        return;
    if(!my_context || !my_context->maplib)
        return;
    uintptr_t m = ga_resolve_guest("malloc");
    uintptr_t f = ga_resolve_guest("free");
    if(!m || !f) {
        if(ga_active)
            printf_log(LOG_INFO, "Guest allocator: no globally bound override anymore, using host allocator\n");
        ga_active = 0;
        ga_malloc_f = ga_free_f = ga_calloc_f = ga_realloc_f = 0;
        ga_memalign_f = ga_posix_memalign_f = ga_aligned_alloc_f = ga_usable_f = 0;
        return;
    }
    int changed = (!ga_active || ga_malloc_f!=m || ga_free_f!=f);
    ga_malloc_f = m;
    ga_free_f = f;
    ga_calloc_f = ga_resolve_guest("calloc");
    ga_realloc_f = ga_resolve_guest("realloc");
    ga_memalign_f = ga_resolve_guest("memalign");
    ga_posix_memalign_f = ga_resolve_guest("posix_memalign");
    ga_aligned_alloc_f = ga_resolve_guest("aligned_alloc");
    ga_usable_f = ga_resolve_guest("malloc_usable_size");
    ga_active = 1;
    if(changed)
        printf_log(LOG_INFO, "Guest allocator detected (malloc=%p, free=%p), guest heap enabled\n",
                   (void*)m, (void*)f);
}

int guest_alloc_is_active(void)
{
    return ga_active;
}

int guest_alloc_owns(const void* p)
{
    return ga_active && p && isGuestHeap((uintptr_t)p);
}

static int ga_enter(void)
{
    if(ga_in_guest) {
        printf_log(LOG_INFO, "Guest allocator: recursive allocation call, refusing\n");
        return 0;
    }
    ga_in_guest = 1;
    return 1;
}

static void ga_leave(void)
{
    ga_in_guest = 0;
}

static void* ga_call_malloc(size_t n)
{
    if(!ga_enter())
        return NULL;
    void* ret = (void*)RunFunctionFmtNoAlt(ga_malloc_f, "L", (uint64_t)n);
    ga_leave();
    return ret;
}

static void* ga_call_calloc(size_t n, size_t s)
{
    if(!ga_enter())
        return NULL;
    void* ret = (void*)RunFunctionFmtNoAlt(ga_calloc_f, "LL", (uint64_t)n, (uint64_t)s);
    ga_leave();
    return ret;
}

static void* ga_call_realloc(void* p, size_t n)
{
    if(!ga_enter())
        return NULL;
    void* ret = (void*)RunFunctionFmtNoAlt(ga_realloc_f, "pL", p, (uint64_t)n);
    ga_leave();
    return ret;
}

static void ga_call_free(void* p)
{
    if(!ga_enter())
        return;
    RunFunctionFmtNoAlt(ga_free_f, "p", p);
    ga_leave();
}

static void* ga_call_memalign(size_t align, size_t size)
{
    if(!ga_enter())
        return NULL;
    void* ret = (void*)RunFunctionFmtNoAlt(ga_memalign_f, "LL", (uint64_t)align, (uint64_t)size);
    ga_leave();
    return ret;
}

static int ga_call_posix_memalign(void** memptr, size_t align, size_t size)
{
    if(!ga_enter())
        return ENOMEM;
    int ret = (int)RunFunctionFmtNoAlt(ga_posix_memalign_f, "pLL", memptr, (uint64_t)align, (uint64_t)size);
    ga_leave();
    return ret;
}

static void* ga_call_aligned_alloc(size_t align, size_t size)
{
    if(!ga_enter())
        return NULL;
    void* ret = (void*)RunFunctionFmtNoAlt(ga_aligned_alloc_f, "LL", (uint64_t)align, (uint64_t)size);
    ga_leave();
    return ret;
}

static size_t ga_call_usable(void* p)
{
    if(!ga_enter())
        return 0;
    size_t ret = (size_t)RunFunctionFmtNoAlt(ga_usable_f, "p", p);
    ga_leave();
    return ret;
}

static size_t ga_ptr_size(void* p)
{
    if(ga_usable_f && thread_emu_ready())
        return ga_call_usable(p);
    uintptr_t a = (uintptr_t)p;
    size_t s = 0;
    while(s < ((size_t)1<<30) && (getProtection(a+s) & PROT_READ))
        s += box64_pagesize;
    return s;
}

void* guest_malloc(size_t size)
{
    if(!ga_active)
        return box_malloc(size);
    if(!thread_emu_ready()) {
        printf_log(LOG_INFO, "Guest allocator: malloc(%zu) requested before guest TLS is ready, failing\n", size);
        return NULL;
    }
    return ga_call_malloc(size);
}

void* guest_calloc(size_t n, size_t size)
{
    if(!ga_active)
        return box_calloc(n, size);
    if(n && size > (size_t)-1 / n) {
        errno = ENOMEM;
        return NULL;
    }
    if(!thread_emu_ready()) {
        printf_log(LOG_INFO, "Guest allocator: calloc(%zu,%zu) requested before guest TLS is ready, failing\n", n, size);
        return NULL;
    }
    if(ga_calloc_f)
        return ga_call_calloc(n, size);
    void* p = ga_call_malloc(n*size);
    if(p)
        memset(p, 0, n*size);
    return p;
}

void* guest_realloc(void* p, size_t size)
{
    if(!ga_active)
        return box_realloc(p, size);
    if(!p)
        return guest_malloc(size);
    if(!isGuestHeap((uintptr_t)p))
        return box_realloc(p, size);
    if(!thread_emu_ready()) {
        printf_log(LOG_INFO, "Guest allocator: realloc(%p) requested before guest TLS is ready, failing\n", p);
        return NULL;
    }
    if(ga_realloc_f)
        return ga_call_realloc(p, size);
    size_t old = ga_ptr_size(p);
    if(old > size)
        old = size;
    void* ret = guest_malloc(size);
    if(!ret)
        return NULL;
    if(old)
        memcpy(ret, p, old);
    guest_free(p);
    return ret;
}

void guest_free(void* p)
{
    if(!p)
        return;
    if(!ga_active || !isGuestHeap((uintptr_t)p)) {
        box_free(p);
        return;
    }
    if(!thread_emu_ready() || !ga_free_f) {
        printf_log(LOG_INFO, "Guest allocator: cannot free %p before guest TLS is ready, leaking\n", p);
        return;
    }
    ga_call_free(p);
}

void* guest_memalign(size_t align, size_t size)
{
    if(!ga_active)
        return box_memalign(align, size);
    if(!thread_emu_ready()) {
        printf_log(LOG_INFO, "Guest allocator: aligned allocation requested before guest TLS is ready, failing\n");
        return NULL;
    }
    if(ga_memalign_f)
        return ga_call_memalign(align, size);
    if(ga_posix_memalign_f) {
        void* ret = NULL;
        if(ga_call_posix_memalign(&ret, align, size)==0)
            return ret;
        return NULL;
    }
    if(ga_aligned_alloc_f)
        return ga_call_aligned_alloc(align, size);
    printf_log(LOG_INFO, "Guest allocator: no aligned allocation primitive for align=%zu size=%zu\n", align, size);
    errno = ENOMEM;
    return NULL;
}

int guest_posix_memalign(void** memptr, size_t align, size_t size)
{
    if(!ga_active) {
        if(!memptr)
            return EINVAL;
        if((align%sizeof(void*)) || pot(align)!=align)
            return EINVAL;
        void* ret = box_memalign(align, size);
        if(!ret)
            return ENOMEM;
        *memptr = ret;
        return 0;
    }
    if(!memptr)
        return EINVAL;
    if(!thread_emu_ready()) {
        printf_log(LOG_INFO, "Guest allocator: posix_memalign requested before guest TLS is ready, failing\n");
        return ENOMEM;
    }
    if(ga_posix_memalign_f)
        return ga_call_posix_memalign(memptr, align, size);
    if(ga_memalign_f) {
        void* ret = ga_call_memalign(align, size);
        if(!ret)
            return ENOMEM;
        *memptr = ret;
        return 0;
    }
    if(ga_aligned_alloc_f) {
        void* ret = ga_call_aligned_alloc(align, size);
        if(!ret)
            return ENOMEM;
        *memptr = ret;
        return 0;
    }
    return ENOMEM;
}

size_t guest_usable_size(void* p)
{
    if(!p)
        return 0;
    if(!ga_active || !isGuestHeap((uintptr_t)p))
        return box_malloc_usable_size ? box_malloc_usable_size(p) : 0;
    if(ga_usable_f && thread_emu_ready())
        return ga_call_usable(p);
    return 0;
}

EXPORT void* malloc(size_t l)
{
    return actual_calloc(1, l);
}

EXPORT void free(void* p)
{
    if(p && ga_active && isGuestHeap((uintptr_t)p)) {
        guest_free(p);
        return;
    }
    actual_free(p);
}

EXPORT void* calloc(size_t n, size_t s)
{
    return actual_calloc(n, s);
}

EXPORT void* realloc(void* p, size_t s)
{
    if(p && ga_active && isGuestHeap((uintptr_t)p))
        return guest_realloc(p, s);
    return actual_realloc(p, s);
}

EXPORT void* memalign(size_t align, size_t size)
{
    if(box64_is32bits && align==4)
        align = sizeof(void*);
    return actual_memalign(align, size);
}

EXPORT void* aligned_alloc(size_t align, size_t size)
{
    if(box64_is32bits && align==4)
        return memalign(align, size);
    return actual_memalign(align, size);
}

EXPORT int posix_memalign(void** p, size_t align, size_t size)
{
    if(box64_is32bits && align==4)
        align = sizeof(void*);
    if((align%sizeof(void*)) || (pot(align)!=align))
        return EINVAL;
    void* ret = actual_memalign(align, size);
    if(!ret)
        return ENOMEM;
    *p = ret;
    return 0;
}

EXPORT void* valloc(size_t size)
{
    return actual_memalign(box64_pagesize, size);
}

EXPORT void* pvalloc(size_t size)
{
    return actual_memalign(box64_pagesize, ALIGN(size));
}

EXPORT void cfree(void* p)
{
    if(p && ga_active && isGuestHeap((uintptr_t)p)) {
        guest_free(p);
        return;
    }
    actual_free(p);
}

EXPORT size_t malloc_usable_size(void* p)
{
    if(p && ga_active && isGuestHeap((uintptr_t)p))
        return guest_usable_size(p);
    return actual_malloc_usable_size(p);
}

static int elf_defines_allocator(elfheader_t* h)
{
    if(!h || !h->DynSym._64 || !h->DynStr)
        return 0;
    for(size_t i=0; i<h->numDynSym; ++i) {
        int type = ELF64_ST_TYPE(h->DynSym._64[i].st_info);
        int bind = ELF64_ST_BIND(h->DynSym._64[i].st_info);
        int vis = h->DynSym._64[i].st_other & 0x3;
        if(type != STT_FUNC)
            continue;
        if(h->DynSym._64[i].st_shndx == 0 || h->DynSym._64[i].st_shndx > 65521)
            continue;
        if(bind == STB_LOCAL)
            continue;
        if(vis != STV_DEFAULT && vis != STV_PROTECTED)
            continue;
        const char* n = h->DynStr + h->DynSym._64[i].st_name;
        if(!strcmp(n, "malloc") || !strcmp(n, "free") || !strcmp(n, "calloc") || !strcmp(n, "realloc"))
            return 1;
    }
    return 0;
}

void checkHookedSymbols(elfheader_t* h)
{
    if(box64_is32bits) {
#if defined(BOX32) && defined(PPC64LE)
        checkOggHookedSymbols32(h);
#else
        (void)h;
#endif
        return;
    }
    if(ga_active || elf_defines_allocator(h))
        guest_alloc_refresh();
}

void init_guestalloc(void)
{
    box_malloc_usable_size = dlsym(RTLD_NEXT, "malloc_usable_size");
}

void guest_alloc_run(void)
{
    guest_alloc_refresh();
}

void guest_alloc_fini(void)
{
    ga_active = 0;
    ga_malloc_f = ga_free_f = ga_calloc_f = ga_realloc_f = 0;
    ga_memalign_f = ga_posix_memalign_f = ga_aligned_alloc_f = ga_usable_f = 0;
}

void* my_tbb_malloc_proxy(unsigned long size);

EXPORT void* my__Znwm(size_t size)
{
    return my_tbb_malloc_proxy(size);
}
EXPORT void* my__ZnwmRKSt9nothrow_t(size_t size, void* nt)
{
    (void)nt;
    return my_tbb_malloc_proxy(size);
}
EXPORT void* my__Znam(size_t size)
{
    return my_tbb_malloc_proxy(size);
}
EXPORT void* my__ZnamRKSt9nothrow_t(size_t size, void* nt)
{
    (void)nt;
    return my_tbb_malloc_proxy(size);
}
EXPORT void my__ZdlPv(void* p)
{
    free(p);
}
EXPORT void my__ZdlPvRKSt9nothrow_t(void* p, void* nt)
{
    (void)nt;
    free(p);
}
EXPORT void my__ZdaPv(void* p)
{
    free(p);
}
EXPORT void my__ZdaPvRKSt9nothrow_t(void* p, void* nt)
{
    (void)nt;
    free(p);
}

static void* ga_original_allocator(const char* name)
{
    if(!strcmp(name, "malloc") || !strcmp(name, "__libc_malloc")) return (void*)malloc;
    if(!strcmp(name, "free") || !strcmp(name, "__libc_free")) return (void*)free;
    if(!strcmp(name, "calloc") || !strcmp(name, "__libc_calloc")) return (void*)calloc;
    if(!strcmp(name, "realloc") || !strcmp(name, "__libc_realloc")) return (void*)realloc;
    if(!strcmp(name, "memalign") || !strcmp(name, "__libc_memalign")) return (void*)memalign;
    if(!strcmp(name, "posix_memalign")) return (void*)posix_memalign;
    if(!strcmp(name, "aligned_alloc")) return (void*)aligned_alloc;
    if(!strcmp(name, "valloc") || !strcmp(name, "__libc_valloc")) return (void*)valloc;
    if(!strcmp(name, "pvalloc") || !strcmp(name, "__libc_pvalloc")) return (void*)pvalloc;
    if(!strcmp(name, "cfree") || !strcmp(name, "__libc_cfree")) return (void*)cfree;
    if(!strcmp(name, "malloc_usable_size")) return (void*)malloc_usable_size;
    return NULL;
}

EXPORT int my___TBB_internal_find_original_malloc(int n, char* names[], void* ptr[])
{
    int ret = 1;
    for(int i=0; i<n; ++i) {
        void* f = ga_original_allocator(names[i]);
        if(f)
            ptr[i] = f;
        else
            ret = 0;
    }
    return ret;
}

EXPORT void my___TBB_call_with_my_server_info(void* cb, void* server)
{
    (void)cb; (void)server;
}

EXPORT int my___TBB_make_rml_server(void* factory, void* server, void* client)
{
    (void)factory; (void)server; (void)client;
    return 0;
}

EXPORT void my___RML_close_factory(void* server)
{
    (void)server;
}

EXPORT int my___RML_open_factory(void* factory, void* server_version, int client_version)
{
    (void)factory; (void)server_version; (void)client_version;
    return 0;
}

#if defined(BOX32) && defined(PPC64LE)
// ---- ogg_sync_pageout hook for 32-bit binaries ----
// Fixes use-after-free in games that statically link libogg.
// ogg_sync_pageout/pageseek populates ogg_page with pointers into the
// ogg_sync_state internal buffer. A subsequent ogg_sync_buffer call may
// realloc that buffer, invalidating the pointers. This can cause crashes
// on any platform where realloc moves the buffer rather than extending
// in-place. Currently gated behind PPC64LE where 64KB pages make this
// particularly likely, but may apply to other backends too.
// The fix: after calling the original ogg_sync_pageout, copy the page
// header and body to stable allocations.
//
// See https://github.com/ptitSeb/box64/issues/3587

#include "wrapper32.h"

typedef struct  __attribute__((packed)) {
    ptr_t   header;
    long_t  header_len;
    ptr_t   body;
    long_t  body_len;
} ogg_page_32_t;

static uintptr_t real_ogg_sync_pageout_32 = 0;
static void*     ogg_page_header_copy = NULL;
static void*     ogg_page_body_copy = NULL;

static int my_ogg_sync_pageout_32(void* oy, void* og)
{
    int ret = (int)RunFunctionFmtNoAlt(real_ogg_sync_pageout_32, "pp", oy, og);

    if(ret > 0 && og) {
        ogg_page_32_t* page = (ogg_page_32_t*)og;

        if(ogg_page_header_copy) { actual_free(ogg_page_header_copy); ogg_page_header_copy = NULL; }
        if(ogg_page_body_copy)   { actual_free(ogg_page_body_copy);   ogg_page_body_copy = NULL; }

        if(page->header && page->header_len > 0) {
            ogg_page_header_copy = actual_malloc(page->header_len);
            if(ogg_page_header_copy) {
                memcpy(ogg_page_header_copy, from_ptrv(page->header), page->header_len);
                page->header = to_ptrv(ogg_page_header_copy);
            }
        }

        if(page->body && page->body_len > 0) {
            ogg_page_body_copy = actual_malloc(page->body_len);
            if(ogg_page_body_copy) {
                memcpy(ogg_page_body_copy, from_ptrv(page->body), page->body_len);
                page->body = to_ptrv(ogg_page_body_copy);
            }
        }
    }

    return ret;
}

static void checkOggHookedSymbols32(elfheader_t* h)
{
    if(!h->numSymTab || !h->SymTab._32)
        return;

    for (size_t i = 0; i < h->numSymTab; ++i) {
        int type = ELF32_ST_TYPE(h->SymTab._32[i].st_info);
        if(type == STT_FUNC && h->SymTab._32[i].st_shndx != 0
            && h->SymTab._32[i].st_shndx <= 65521) {
            const char* symname = h->StrTab + h->SymTab._32[i].st_name;
            if(!strcmp(symname, "ogg_sync_pageout")) {
                uintptr_t offs = h->SymTab._32[i].st_value + h->delta;
                if(!real_ogg_sync_pageout_32) {
                    real_ogg_sync_pageout_32 = offs;
                    uintptr_t alt = AddCheckBridge(my_context->system, iFpp_32, (void*)my_ogg_sync_pageout_32, 0, "my_ogg_sync_pageout_32");
                    addAlternate((void*)offs, (void*)alt);
                    printf_log(LOG_INFO, "Hooked statically-linked ogg_sync_pageout at %p in %s (ogg realloc fix)\n", (void*)offs, ElfName(h));
                }
                break;
            }
        }
    }
}
#endif

#else

void init_guestalloc(void) {}
void guest_alloc_refresh(void) {}
void guest_alloc_run(void) {}
void guest_alloc_fini(void) {}
int guest_alloc_is_active(void) { return 0; }
int guest_alloc_owns(const void* p) { (void)p; return 0; }
void* guest_malloc(size_t size) { return malloc(size); }
void* guest_calloc(size_t n, size_t size) { return calloc(n, size); }
void* guest_realloc(void* p, size_t size) { return realloc(p, size); }
void guest_free(void* p) { free(p); }
void* guest_memalign(size_t align, size_t size) { return memalign(align, size); }
int guest_posix_memalign(void** memptr, size_t align, size_t size) { return posix_memalign(memptr, align, size); }
size_t guest_usable_size(void* p) { return malloc_usable_size(p); }
void checkHookedSymbols(elfheader_t* h) { (void)h; }

#endif
