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
        else {printf_log(LOG_NONE, "Warning, Wine prereserve badly formatted\n"); return 0;}
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
        if(!isBlockFree(my_wine_reserve[idx].addr, my_wine_reserve[idx].size)) {
            printf_log(LOG_NONE, "Warning, prereserve of %p:0x%lx is not free\n", my_wine_reserve[idx].addr, my_wine_reserve[idx].size);
            my_wine_reserve[idx].addr = NULL;
            my_wine_reserve[idx].size = 0;
        } else {
            setProtection_mmap((uintptr_t)my_wine_reserve[idx].addr, my_wine_reserve[idx].size, 0);
            printf_log(LOG_DEBUG, "WINE prereserve of %p:0x%lx done\n", my_wine_reserve[idx].addr, my_wine_reserve[idx].size);
            ++idx;
        }
    }

    wine_preloaded = 1;
}

void* get_wine_prereserve()
{
    if(!wine_preloaded)
        wine_prereserve(NULL);
    return (void*)my_wine_reserve;
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
