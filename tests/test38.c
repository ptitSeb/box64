// SPDX-License-Identifier: MIT
#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <pthread.h>
#include <execinfo.h>
#include <errno.h>

static void (*real_free)(void*);
static void* (*real_realloc)(void*, size_t);

#define MAGIC 0x6761616c6c6f6331ULL
typedef struct {
    uint64_t magic;
    size_t   size;
} hdr_t;

static char*  arena_brk;
static char*  arena_mmap;
static size_t used_brk;
static size_t used_mmap;
static size_t cap_brk = 2u << 20;
static size_t cap_mmap = 1u << 20;

static void init_allocator(void) __attribute__((constructor));
static void init_allocator(void)
{
    real_free = (void (*)(void*))dlsym(RTLD_NEXT, "free");
    real_realloc = (void* (*)(void*, size_t))dlsym(RTLD_NEXT, "realloc");
    arena_brk = (char*)sbrk(cap_brk);
    if(arena_brk==(void*)-1)
        arena_brk = NULL;
    arena_mmap = (char*)mmap(NULL, cap_mmap, PROT_READ|PROT_WRITE,
                             MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if(arena_mmap==MAP_FAILED)
        arena_mmap = NULL;
}

static int in_range(const void* p, const char* base, size_t used)
{
    return base && (const char*)p >= base && (const char*)p < base + used;
}

static int guest_pointer(const void* p)
{
    return in_range(p, arena_brk, used_brk) || in_range(p, arena_mmap, used_mmap);
}

static void* bump(size_t size)
{
    size_t total = (size + sizeof(hdr_t) + 15) & ~(size_t)15;
    size_t* used;
    char* base;
    size_t cap;
    if(arena_brk && used_brk + total <= cap_brk) {
        used = &used_brk;
        base = arena_brk;
        cap = cap_brk;
    } else if(arena_mmap && used_mmap + total <= cap_mmap) {
        used = &used_mmap;
        base = arena_mmap;
        cap = cap_mmap;
    } else {
        errno = ENOMEM;
        return NULL;
    }
    size_t off = __atomic_fetch_add(used, total, __ATOMIC_RELAXED);
    if(off + total > cap) {
        errno = ENOMEM;
        return NULL;
    }
    hdr_t* h = (hdr_t*)(base + off);
    h->magic = MAGIC;
    h->size = size;
    return h + 1;
}

static inline void* fast_alloc(size_t size) __attribute__((always_inline));
static inline void* fast_alloc(size_t size)
{
    return bump(size);
}

static inline void fast_free(void* p) __attribute__((always_inline));
static inline void fast_free(void* p)
{
    if(!p)
        return;
    if(guest_pointer(p)) {
        hdr_t* h = ((hdr_t*)p) - 1;
        if(h->magic != MAGIC) {
            printf("BAD_FREE\n");
            _exit(2);
        }
        h->magic = 0;
        return;
    }
    real_free(p);
}

void* malloc(size_t size)
{
    return bump(size);
}

void* calloc(size_t n, size_t size)
{
    if(n && size > (size_t)-1 / n) {
        errno = ENOMEM;
        return NULL;
    }
    void* p = bump(n * size);
    if(p)
        memset(p, 0, n * size);
    return p;
}

void free(void* p)
{
    fast_free(p);
}

void* realloc(void* p, size_t size)
{
    if(!p)
        return bump(size);
    if(!guest_pointer(p))
        return real_realloc(p, size);
    hdr_t* h = ((hdr_t*)p) - 1;
    if(h->magic != MAGIC) {
        printf("BAD_REALLOC\n");
        _exit(2);
    }
    size_t old = h->size;
    void* ret = bump(size);
    if(!ret)
        return NULL;
    memcpy(ret, p, old < size ? old : size);
    return ret;
}

size_t malloc_usable_size(void* p)
{
    if(!p)
        return 0;
    if(guest_pointer(p)) {
        hdr_t* h = ((hdr_t*)p) - 1;
        return h->magic == MAGIC ? h->size : 0;
    }
    return 0;
}

static int no_dots(const struct dirent* d)
{
    return d->d_name[0] != '.';
}

static void* thread_work(void* arg)
{
    (void)arg;
    for(int i=0; i<2000; ++i) {
        char* p = (i & 1) ? (char*)malloc(32) : (char*)fast_alloc(48);
        char* s = strdup("thread-string");
        if(!p || !s)
            return (void*)1;
        if(!guest_pointer(s))
            return (void*)1;
        if(i & 1)
            free(p);
        else
            fast_free(p);
        free(s);
    }
    return NULL;
}

int main(void)
{
    char* s = strdup("hello");
    printf("strdup guest=%d\n", guest_pointer(s));
    free(s);

    s = strndup("abcdef", 3);
    printf("strndup guest=%d val=%s\n", guest_pointer(s), s ? s : "(null)");
    free(s);

    wchar_t* ws = wcsdup(L"wide");
    printf("wcsdup guest=%d\n", guest_pointer(ws));
    free(ws);

    char* a = NULL;
    int r = asprintf(&a, "%s-%d", "asprintf", 42);
    printf("asprintf guest=%d val=%s\n", guest_pointer(a), (r >= 0 && a) ? a : "(null)");
    free(a);

    char* line = (char*)fast_alloc(4);
    size_t linecap = 4;
    FILE* f = fmemopen("abcdefghij\n", 11, "r");
    long l = getline(&line, &linecap, f);
    printf("getline guest=%d len=%ld val=%.*s\n", guest_pointer(line), l,
           (int)(l > 0 ? l - 1 : 0), line ? line : "");
    fclose(f);
    free(line);

    char cwd[512];
    char* gcw = getcwd(NULL, 0);
    int same = gcw && getcwd(cwd, sizeof(cwd)) && !strcmp(gcw, cwd);
    printf("getcwd guest=%d same=%d\n", guest_pointer(gcw), same);
    free(gcw);

    char* cdn = get_current_dir_name();
    printf("curdir guest=%d\n", guest_pointer(cdn));
    free(cdn);

    char* canon = canonicalize_file_name("/tmp");
    printf("canon guest=%d\n", guest_pointer(canon));
    free(canon);

    char* rp = realpath("/tmp", NULL);
    printf("realpath guest=%d\n", guest_pointer(rp));
    free(rp);

    char* tmp = tempnam("/tmp", "ga38");
    printf("tempnam guest=%d\n", guest_pointer(tmp));
    free(tmp);

    char* msbuf = (char*)fast_alloc(16);
    size_t mslen = 0;
    FILE* ms = open_memstream(&msbuf, &mslen);
    fputs("memstream", ms);
    fclose(ms);
    printf("memstream guest=%d len=%zu val=%s\n", guest_pointer(msbuf), mslen, msbuf);
    free(msbuf);

    char tmpl[] = "/tmp/ga38XXXXXX";
    char* dir = mkdtemp(tmpl);
    if(dir) {
        const char* names[] = {"b", "a", "c"};
        for(int i=0; i<3; ++i) {
            char path[256];
            snprintf(path, sizeof(path), "%s/%s", dir, names[i]);
            int fd = open(path, O_CREAT|O_WRONLY, 0600);
            if(fd>=0)
                close(fd);
        }
        struct dirent** list = NULL;
        int n = scandir(dir, &list, no_dots, alphasort);
        int g = (n==3) && guest_pointer(list);
        char joined[16] = {0};
        for(int i=0; i<n && g; ++i) {
            if(!guest_pointer(list[i]))
                g = 0;
            else
                strcat(joined, list[i]->d_name);
        }
        printf("scandir n=%d guest=%d names=%s\n", n, g, joined);
        for(int i=0; i<n; ++i)
            free(list[i]);
        free(list);
        for(int i=0; i<3; ++i) {
            char path[256];
            snprintf(path, sizeof(path), "%s/%s", dir, names[i]);
            unlink(path);
        }
        rmdir(dir);
    }

    void* bt[16];
    int nb = backtrace(bt, 16);
    char** syms = backtrace_symbols(bt, nb);
    printf("backtrace guest=%d\n", guest_pointer(syms));
    free(syms);

    char* brk0 = (char*)sbrk(0);
    char* brk1 = (char*)sbrk(4096);
    printf("sbrk ok=%d same=%d\n", brk0 != (void*)-1 && brk1 != (void*)-1, brk1 == brk0);

    void* p1 = fast_alloc(64);
    free(p1);
    printf("mix1=OK\n");

    void* p2 = malloc(64);
    fast_free(p2);
    printf("mix2=OK\n");

    pthread_t t[2];
    void* failed = NULL;
    for(int i=0; i<2; ++i)
        pthread_create(&t[i], NULL, thread_work, NULL);
    for(int i=0; i<2; ++i) {
        void* ret = NULL;
        pthread_join(t[i], &ret);
        if(ret)
            failed = ret;
    }
    printf("threads=%s\n", failed ? "FAIL" : "OK");

    return 0;
}
