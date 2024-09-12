#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <unistd.h>
#include <stdint.h>

#ifndef MAP_FAILED
#define MAP_FAILED ((void *) -1)
#endif

#define EXPORT __attribute__((visibility("default")))
#ifdef BUILD_DYNAMIC
#define EXPORTDYN __attribute__((visibility("default")))
#else
#define EXPORTDYN
#endif

#ifndef MAP_32BIT
#define MAP_32BIT   0x40
#endif

typedef void x64emu_t;
extern void* mapallmem;
extern int box64_is32bits;
void setProtection(uintptr_t addr, size_t size, uint32_t prot);
void freeProtection(uintptr_t addr, size_t size);
void* internal_mmap(void *addr, unsigned long length, int prot, int flags, int fd, ssize_t offset);
int internal_munmap(void* addr, unsigned long length);

void* my_mmap64(x64emu_t* emu, void *addr, unsigned long length, int prot, int flags, int fd, ssize_t offset);

extern int running32bits;
extern int box64_mmap32;

EXPORT void* mmap64(void *addr, unsigned long length, int prot, int flags, int fd, ssize_t offset)
{
    void* ret;
    if(!addr && ((running32bits && box64_mmap32) || (flags&MAP_32BIT) || box64_is32bits))
        ret = my_mmap64(NULL, addr, length, prot, flags | MAP_32BIT, fd, offset);
    else
        ret = internal_mmap(addr, length, prot, flags, fd, offset);
    if(ret!=MAP_FAILED && mapallmem)
        setProtection((uintptr_t)ret, length, prot);
    return ret;
}
EXPORT void* mmap(void *addr, unsigned long length, int prot, int flags, int fd, ssize_t offset) __attribute__((alias("mmap64")));

EXPORT int munmap(void* addr, unsigned long length)
{
    int ret = internal_munmap(addr, length);
    if(!ret && mapallmem) {
        freeProtection((uintptr_t)addr, length);
    }
    return ret;
}
