// SPDX-License-Identifier: MIT
#ifndef __GUESTALLOC_H__
#define __GUESTALLOC_H__

#include <stddef.h>
#include <stdint.h>

typedef struct elfheader_s elfheader_t;

#ifdef STATICBUILD
#include <stdlib.h>
#include <malloc.h>
static inline void init_guestalloc(void) {}
static inline void guest_alloc_refresh(void) {}
static inline void guest_alloc_run(void) {}
static inline void guest_alloc_fini(void) {}
static inline int guest_alloc_is_active(void) { return 0; }
static inline int guest_alloc_owns(const void* p) { (void)p; return 0; }
static inline void* guest_malloc(size_t size) { return malloc(size); }
static inline void* guest_calloc(size_t n, size_t size) { return calloc(n, size); }
static inline void* guest_realloc(void* p, size_t size) { return realloc(p, size); }
static inline void guest_free(void* p) { free(p); }
static inline void* guest_memalign(size_t align, size_t size) { return memalign(align, size); }
static inline int guest_posix_memalign(void** p, size_t align, size_t size) { return posix_memalign(p, align, size); }
static inline size_t guest_usable_size(void* p) { return malloc_usable_size(p); }
static inline void checkHookedSymbols(elfheader_t* h) { (void)h; }
#else

void init_guestalloc(void);
void guest_alloc_refresh(void);
void guest_alloc_run(void);
void guest_alloc_fini(void);
int guest_alloc_is_active(void);
int guest_alloc_owns(const void* p);
void* guest_malloc(size_t size);
void* guest_calloc(size_t n, size_t size);
void* guest_realloc(void* p, size_t size);
void guest_free(void* p);
void* guest_memalign(size_t align, size_t size);
int guest_posix_memalign(void** memptr, size_t align, size_t size);
size_t guest_usable_size(void* p);

void checkHookedSymbols(elfheader_t* h);

#endif

#endif
