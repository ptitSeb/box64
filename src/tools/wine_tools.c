#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <errno.h>

#include "wine_tools.h"
#include "debug.h"
#include "box64context.h"
#include "custommem.h"

typedef struct wine_prereserve_s
{
    void*   addr;
    size_t  size;
} wine_prereserve_t;
#ifdef BOX32
typedef struct wine_prereserve_32_s
{
    ptr_t   addr;
    ulong_t  size;
} wine_prereserve_32_t;
#include "box32.h"
#endif
// only the prereseve argument is reserved, not the other zone that wine-preloader reserve
static wine_prereserve_t my_wine_reserve[] = {{(void*)0x00010000, 0x00008000}, {(void*)0x00110000, 0x30000000}, {(void*)0x7f000000, 0x03000000}, {0, 0}, {0, 0}};

int wine_preloaded = 0;

static int get_prereserve(const char* reserve, void** addr, size_t* size)
{
    if(!reserve)
        return 0;
    uintptr_t r = 0;
    int first = 1;
    while(*reserve) {
        // numbers reading
        if(*reserve>='0' && *reserve<='9')  r=r*16+(*reserve)-'0';
        else if(*reserve>='A' && *reserve<='F')  r=r*16+(*reserve)-'A'+10;
        else if(*reserve>='a' && *reserve<='f')  r=r*16+(*reserve)-'a'+10;
        else if(*reserve=='-') {if(first) {*addr=(void*)(r&~(box64_pagesize-1)); r=0; first=0;} else {printf_log(LOG_NONE, "Warning, Wine prereserve badly formatted\n"); return 0;}}
        else {printf_log(LOG_INFO, "Warning, Wine prereserve badly formatted\n"); return 0;}
        ++reserve;
    }
    *size = r;
    return 1;
}

static void add_no_overlap(void* addr, size_t size)
{
    int idx = 0;
    while(my_wine_reserve[idx].addr && my_wine_reserve[idx].size) {
        if(addr>=my_wine_reserve[idx].addr && addr<my_wine_reserve[idx].addr+my_wine_reserve[idx].size) {
            // overlap
            if (addr+size > my_wine_reserve[idx].addr+my_wine_reserve[idx].size)
                // need adjust size
                my_wine_reserve[idx].size = (intptr_t)addr-(intptr_t)my_wine_reserve[idx].addr+size;
            return;
        }
        ++idx;
    }
    my_wine_reserve[idx].addr = addr;
    my_wine_reserve[idx].size = size;
}

static void remove_prereserve(int idx)
{
    while(my_wine_reserve[idx].size) {
        my_wine_reserve[idx].addr = my_wine_reserve[idx+1].addr;
        my_wine_reserve[idx].size = my_wine_reserve[idx+1].size;
        ++idx;
    }
}

void preserve_highest32();
void wine_prereserve(const char* reserve)
{
    init_custommem_helper(my_context);
    void* addr = NULL;
    size_t size = 0;

    if(get_prereserve(reserve, &addr, &size)) {
        add_no_overlap(addr, size);
    }

    int idx = 0;
    while(my_wine_reserve[idx].addr && my_wine_reserve[idx].size) {
        void* ret = NULL;
        int isfree = isBlockFree(my_wine_reserve[idx].addr, my_wine_reserve[idx].size);
        if(isfree) ret=mmap(my_wine_reserve[idx].addr, my_wine_reserve[idx].size, 0, MAP_FIXED|MAP_PRIVATE|MAP_ANON|MAP_NORESERVE, -1, 0); else ret = NULL;
        if(!isfree || (ret!=my_wine_reserve[idx].addr)) {
            if(addr>=(void*)0x10000LL)
                printf_log(LOG_INFO, "Warning, prereserve of %p:0x%lx is not free\n", my_wine_reserve[idx].addr, my_wine_reserve[idx].size);
            if(ret)
                munmap(ret, my_wine_reserve[idx].size);
            remove_prereserve(idx);
        } else {
            setProtection_mmap((uintptr_t)my_wine_reserve[idx].addr, my_wine_reserve[idx].size, 0);
            printf_log(/*LOG_DEBUG*/LOG_INFO, "WINE prereserve of %p:0x%lx done\n", my_wine_reserve[idx].addr, my_wine_reserve[idx].size);
            ++idx;
        }
    }

    preserve_highest32();

    wine_preloaded = 1;
}

void* get_wine_prereserve()
{
    if(!wine_preloaded)
        wine_prereserve(NULL);
    #ifdef BOX32
    if(box64_is32bits) {
        static wine_prereserve_32_t my_wine_reserve_32[5];
        for(int i=0; i<5; ++i) {
            my_wine_reserve_32[i].addr = to_ptrv(my_wine_reserve[i].addr);
            my_wine_reserve_32[i].size = to_ulong(my_wine_reserve[i].size);
        }
        return &my_wine_reserve_32;
    } else
    #endif
        return &my_wine_reserve;
}

#ifdef DYNAREC
void dynarec_wine_prereserve()
{
    #if 0
    // disable for now, as it break some installer
    if(!wine_preloaded)
        wine_prereserve(NULL);
    // don't reserve the initial arbritrary block as "with linker", it's not true
    for(int i=1; i<sizeof(my_wine_reserve)/sizeof(my_wine_reserve[0]); ++i)
        if(my_wine_reserve[i].addr && my_wine_reserve[i].size)
            addDBFromAddressRange((uintptr_t)my_wine_reserve[i].addr, my_wine_reserve[i].size);  // prepare the prereserved area for exec, with linker
    #endif
}
#endif

void DetectUnityPlayer(char* filename)
{
    static int unityplayer_detected = 0;
    if (filename && BOX64ENV(unityplayer) && !unityplayer_detected && !strcmp(filename, "unityplayer.dll")) {
        printf_log(LOG_INFO, "Detected UnityPlayer.dll\n");
#ifdef DYNAREC
        if (!BOX64ENV(dynarec_strongmem)) {
            SET_BOX64ENV(dynarec_strongmem, 1);
            PrintEnvVariables(&box64env, LOG_INFO);
        }
#endif
        unityplayer_detected = 1;
    }
}
