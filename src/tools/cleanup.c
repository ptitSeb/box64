#include <string.h>

#include "cleanup.h"
#include "elfs/elfloader_private.h"
#include "box64context.h"
#include "debug.h"
#include "callback.h"

typedef struct cleanup_s {
    void*       f;
    int         arg;
    void*       a;
} cleanup_t;

void AddCleanup(x64emu_t *emu, void *p)
{
    (void)emu;

    if(my_context->clean_sz == my_context->clean_cap) {
        my_context->clean_cap += 32;
        my_context->cleanups = (cleanup_t*)box_realloc(my_context->cleanups, sizeof(cleanup_t)*my_context->clean_cap);
    }
    my_context->cleanups[my_context->clean_sz].arg = 0;
    my_context->cleanups[my_context->clean_sz].a = NULL;
    my_context->cleanups[my_context->clean_sz++].f = p;
}

void AddCleanup1Arg(x64emu_t *emu, void *p, void* a, elfheader_t* h)
{
    (void)emu;
    if (!h)
        h = my_context->elfs[0];

    if(h->clean_sz == h->clean_cap) {
        h->clean_cap += 32;
        h->cleanups = (cleanup_t*)box_realloc(h->cleanups, sizeof(cleanup_t)*h->clean_cap);
    }
    h->cleanups[h->clean_sz].arg = 1;
    h->cleanups[h->clean_sz].a = a;
    h->cleanups[h->clean_sz++].f = p;
}

void CallCleanup(x64emu_t *emu, elfheader_t* h)
{
    printf_log(LOG_DEBUG, "Calling atexit registered functions for elf: %p/%s\n", h, h?h->name:"(nil)");
    if(!h)
        return;
    for(int i=h->clean_sz-1; i>=0; --i) {
        printf_log(LOG_DEBUG, "Call cleanup #%d (args:%d, arg:%p)\n", i, h->cleanups[i].arg, h->cleanups[i].a);
        RunFunctionWithEmu(emu, 0, (uintptr_t)(h->cleanups[i].f), h->cleanups[i].arg, h->cleanups[i].a );
        // now remove the cleanup
        if(i!=h->clean_sz-1)
            memmove(h->cleanups+i, h->cleanups+i+1, (h->clean_sz-i-1)*sizeof(cleanup_t));
        --h->clean_sz;
    }
}

void CallAllCleanup(x64emu_t *emu)
{
    printf_log(LOG_DEBUG, "Calling atexit registered functions\n");
    for(int i=my_context->clean_sz-1; i>=0; --i) {
        printf_log(LOG_DEBUG, "Call cleanup #%d\n", i);
        --my_context->clean_sz;
        RunFunctionWithEmu(emu, 0, (uintptr_t)(my_context->cleanups[i].f), my_context->cleanups[i].arg, my_context->cleanups[i].a );
    }
    box_free(my_context->cleanups);
    my_context->cleanups = NULL;
}