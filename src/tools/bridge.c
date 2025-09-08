#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/mman.h>
#include <errno.h>

#include "os.h"
#include "wrappedlibs.h"
#include "custommem.h"
#include "bridge.h"
#include "bridge_private.h"
#include "khash.h"
#include "debug.h"
#include "x64emu.h"
#include "box64context.h"
#include "elfloader.h"
#include "alternate.h"
#ifdef DYNAREC
#include "dynablock.h"
#endif
#ifdef BOX32
#include "box32context.h"
#endif

KHASH_MAP_INIT_INT64(bridgemap, uintptr_t)

typedef struct brick_s brick_t;
typedef struct brick_s {
    onebridge_t *b;
    int         sz;
    brick_t     *next;
} brick_t;

#define NBRICK (box64_pagesize/sizeof(onebridge_t))

typedef struct bridge_s {
    brick_t         *head;
    brick_t         *last;      // to speed up
    kh_bridgemap_t  *bridgemap;
} bridge_t;

brick_t* NewBrick(void* old)
{
    brick_t* ret = (brick_t*)box_calloc(1, sizeof(brick_t));
    static void* load_addr_32bits = NULL;
    if(box64_is32bits)
        old = load_addr_32bits;
    else {
        if(old)
            old = old + NBRICK * sizeof(onebridge_t);
    }
    void* ptr = box_mmap(old, NBRICK * sizeof(onebridge_t), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | ((!box64_is32bits && box64_wine)?0:0x40) | MAP_ANONYMOUS, -1, 0); // 0x40 is MAP_32BIT
    if(ptr == MAP_FAILED)
        ptr = box_mmap(NULL, NBRICK * sizeof(onebridge_t), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | ((!box64_is32bits && box64_wine)?0:0x40) | MAP_ANONYMOUS, -1, 0);
    if(ptr == MAP_FAILED) {
        printf_log(LOG_NONE, "Warning, cannot allocate 0x%lx aligned bytes for bridge, will probably crash later\n", NBRICK*sizeof(onebridge_t));
    }
    setProtection_box((uintptr_t)ptr, NBRICK * sizeof(onebridge_t), PROT_READ | PROT_WRITE | PROT_EXEC | PROT_NOPROT);
    dynarec_log(LOG_INFO, "New Bridge brick at %p (size 0x%zx)\n", ptr, NBRICK*sizeof(onebridge_t));
    if(box64_is32bits) load_addr_32bits = ptr + NBRICK*sizeof(onebridge_t);
    ret->b = ptr;
    return ret;
}

bridge_t *NewBridge()
{
    bridge_t *b = (bridge_t*)box_calloc(1, sizeof(bridge_t));
    // before enable seccomp and bpf fileter, proton check if "syscall" address (from libc) is > 0x700000000000
    // it also test if an internal symbol "sc_seccomp" address's is also > 0x700000000000 before enabling seccomp syscall filter.
    // This hack  allow the test to pass, but only if the system has at least 47bits address space.
    // it will not work on 39bits address space and will need more hacks there
    void* load_addr = NULL;
    if((!box64_is32bits && box64_wine && my_context->exit_bridge))  // a first bridge is create for system use, before box64_is32bits can be computed, so use exit_bridge to detect that
        load_addr = (void*)0x700000000000LL;
    b->head = NewBrick(load_addr);
    b->last = b->head;
    b->bridgemap = kh_init(bridgemap);

    return b;
}
void FreeBridge(bridge_t** bridge)
{
    if(!bridge || !*bridge)
        return;
    brick_t *b = (*bridge)->head;
    while(b) {
        brick_t *n = b->next;
        dynarec_log(LOG_INFO, "FreeBridge brick at %p (size 0x%zx)\n", b->b, NBRICK*sizeof(onebridge_t));
        box_munmap(b->b, NBRICK*sizeof(onebridge_t));
        freeProtection((uintptr_t)b->b, NBRICK*sizeof(onebridge_t));
        box_free(b);
        b = n;
    }
    kh_destroy(bridgemap, (*bridge)->bridgemap);
    box_free(*bridge);
    *bridge = NULL;
}

#ifdef HAVE_TRACE
void addBridgeName(void* addr, const char* name);
#endif

//static const char* default_bridge = "bridge???";
uintptr_t AddBridge(bridge_t* bridge, wrapper_t w, void* fnc, int N, const char* name)
{
    brick_t *b = NULL;
    int sz = -1;
    int ret;

    mutex_lock(&my_context->mutex_bridge);
    b = bridge->last;
    if(b->sz == (int)NBRICK) {
        b->next = NewBrick(b->b);
        b = b->next;
        bridge->last = b;
    }
    sz = b->sz;
    b->sz++;
    // add bridge to map, for fast recovery
    khint_t k = kh_put(bridgemap, bridge->bridgemap, (uintptr_t)fnc, &ret);
    kh_value(bridge->bridgemap, k) = (uintptr_t)&b->b[sz].CC;
    mutex_unlock(&my_context->mutex_bridge);

    b->b[sz].CC = 0xCC;
    b->b[sz].S = 'S'; b->b[sz].C='C';
    b->b[sz].w = w;
    b->b[sz].f = (uintptr_t)fnc;
    b->b[sz].C3 = N?0xC2:0xC3;
    b->b[sz].N = N;
    b->b[sz].name = name/*?name:default_bridge*/;

    return (uintptr_t)&b->b[sz].CC;
}

uintptr_t CheckBridged(bridge_t* bridge, void* fnc)
{
    // check if function alread have a bridge (the function wrapper will not be tested)
    khint_t k = kh_get(bridgemap, bridge->bridgemap, (uintptr_t)fnc);
    if(k==kh_end(bridge->bridgemap))
        return 0;
    return kh_value(bridge->bridgemap, k);
}

uintptr_t AddCheckBridge(bridge_t* bridge, wrapper_t w, void* fnc, int N, const char* name)
{
    if(!fnc && w)
        return 0;
    uintptr_t ret = CheckBridged(bridge, fnc);
    if(!ret)
        ret = AddBridge(bridge, w, fnc, N, name);
    return ret;
}

uintptr_t AddAutomaticBridge(bridge_t* bridge, wrapper_t w, void* fnc, int N, const char* name)
{
    if(!fnc)
        return 0;
    uintptr_t ret = CheckBridged(bridge, fnc);
    if(!ret)
        ret = AddBridge(bridge, w, fnc, N, name);
    if(!hasAlternate(fnc)) {
        printf_log(LOG_DEBUG, "Adding AutomaticBridge for %p to %p\n", fnc, (void*)ret);
        addAlternate(fnc, (void*)ret);
    }
    return ret;
}

uintptr_t AddAutomaticBridgeAlt(bridge_t* bridge, wrapper_t w, void* fnc, void* alt, int N, const char* name)
{
    if(!fnc)
        return 0;
    uintptr_t ret = CheckBridged(bridge, alt);
    if(!ret)
        ret = AddBridge(bridge, w, alt, N, name);
    if(!hasAlternate(fnc)) {
        printf_log(LOG_DEBUG, "Adding AutomaticBridge for %p to %p\n", fnc, (void*)ret);
        addAlternate(fnc, (void*)ret);
    }
    return ret;
}

void* GetNativeOrAlt(void* fnc, void* alt)
{
    if(!fnc) return NULL;
    // check if function exist in some loaded lib
    if(!FindElfAddress(my_context, (uintptr_t)fnc)) {
        Dl_info info;
        if(dladdr(fnc, &info))
            return fnc;
    }
    if(!getProtection((uintptr_t)fnc))
        return alt;
    // check if it's an indirect jump
    #define PK(a)       *(uint8_t*)(fnc+a)
    #define PK32(a)     *(uint32_t*)(fnc+a)
    if(PK(0)==0xff && PK(1)==0x25) {    // "absolute" jump, maybe the GOT (it's a RIP+relative in fact)
        uintptr_t a1 = (uintptr_t)fnc+6+(PK32(2)); // need to add a check to see if the address is from the GOT !
        a1 = *(uintptr_t*)a1;
        if(a1 && a1>0x10000) {
            a1 = (uintptr_t)GetNativeFnc(a1);
            if(a1)
                return (void*)a1;
        }
    }
    #undef PK
    #undef PK32
    // check if bridge exist
    onebridge_t *b = (onebridge_t*)fnc;
    if(b->CC != 0xCC || b->S!='S' || b->C!='C' || (b->C3!=0xC3 && b->C3!=0xC2))
        return alt;    // not a bridge?!
    return (void*)b->f;
}
void* GetNativeFnc(uintptr_t fnc)
{
    return GetNativeOrAlt((void*)fnc, NULL);
}
void* GetNativeFncOrFnc(uintptr_t fnc)
{
    onebridge_t *b = (onebridge_t*)fnc;
    if(b->CC != 0xCC || b->S!='S' || b->C!='C' || (b->C3!=0xC3 && b->C3!=0xC2))
        return (void*)fnc;    // not a bridge?!
    return (void*)b->f;
}

// using the brdige mecanism for the VSyscall
uintptr_t AddVSyscall(bridge_t* bridge, int num)
{
    brick_t *b = NULL;
    int sz = -1;
    mutex_lock(&my_context->mutex_bridge);
    b = bridge->last;
    if(b->sz == (int)NBRICK) {
        b->next = NewBrick(b->b);
        b = b->next;
        bridge->last = b;
    }
    sz = b->sz;
    b->sz++;
    mutex_unlock(&my_context->mutex_bridge);

    b->b[sz].B8 = 0xB8;
    b->b[sz].num = num;
    b->b[sz]._0F = 0x0F;
    b->b[sz]._05 = 0x05;
    b->b[sz]._C3 = 0xC3;

    return (uintptr_t)&b->b[sz].CC;
}

const char* getBridgeName(void* addr)
{
    if(!memExist((uintptr_t)addr))
        return NULL;
    if(!(getProtection((uintptr_t)addr)&PROT_READ))
        return NULL;
    onebridge_t* one = (onebridge_t*)(((uintptr_t)addr&~(sizeof(onebridge_t)-1)));   // align to start of bridge
    if (one->C3 == 0xC3 && IsBridgeSignature(one->S, one->C)) {
        if(one->w==NULL)
            return "ExitEmulation";
        else
            return one->name;
    }
    return NULL;
}

void init_bridge_helper()
{
}

void fini_bridge_helper()
{
    cleanAlternate();
}

#ifdef BOX32
int isNativeCall32(uintptr_t addr, uintptr_t* calladdress, uint16_t* retn)
{
#define PK(a)       *(uint8_t*)(addr+a)
#define PK32(a)     *(uint32_t*)(addr+a)

    if(!addr || !getProtection(addr))
        return 0;
    if(PK(0)==0xff && PK(1)==0x25) {  // absolute jump, maybe the GOT
        ptr_t a1 = (PK32(2));   // need to add a check to see if the address is from the GOT !
        addr = (uintptr_t)getAlternate(from_ptrv(a1)); 
    }
    if(addr<0x10000 || !getProtection(addr))    // too low, that is suspicious
        return 0;
    onebridge_t *b = (onebridge_t*)(addr);
    if(b->CC==0xCC && b->S=='S' && b->C=='C' && b->w!=(wrapper_t)0 && b->f!=(uintptr_t)PltResolver32) {
        // found !
        if(retn) *retn = (b->C3==0xC2)?b->N:0;
        if(calladdress) *calladdress = addr+1;
        return 1;
    }
    return 0;
#undef PK32
#undef PK
}
#else
int isNativeCall32(uintptr_t addr, uintptr_t* calladdress, uint16_t* retn)
{
    return 0;
}
#endif

int isNativeCallInternal(uintptr_t addr, int is32bits, uintptr_t* calladdress, uint16_t* retn)
{
    if (is32bits)
        return isNativeCall32(addr, calladdress, retn);

#define PK(a)   *(uint8_t*)(addr + a)
#define PK32(a) *(int32_t*)(addr + a)

    if (!addr || !getProtection(addr))
        return 0;
    if (PK(0) == 0xff && PK(1) == 0x25) {    // "absolute" jump, maybe the GOT (well, RIP relative in fact)
        uintptr_t a1 = addr + 6 + (PK32(2)); // need to add a check to see if the address is from the GOT !
        addr = (uintptr_t)getAlternate(*(void**)a1);
    }
    if (!addr || !getProtection(addr))
        return 0;
    onebridge_t* b = (onebridge_t*)(addr);
    if (b->CC == 0xCC && IsBridgeSignature(b->S, b->C) && b->w != (wrapper_t)0 && b->f != (uintptr_t)PltResolver64) {
        // found !
        if (retn) *retn = (b->C3 == 0xC2) ? b->N : 0;
        if (calladdress) *calladdress = addr + 1;
        return 1;
    }
    return 0;
#undef PK32
#undef PK
}
