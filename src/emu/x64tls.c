// Handling of TLS calls, include x86 specifi set_thread_area
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "box64context.h"
#include "x64emu.h"
#include "x64emu_private.h"
#include "x64tls.h"
#include "elfloader.h"

typedef struct thread_area_s {
    int entry_number;
    uint64_t base_addr;
    unsigned int limit;
    unsigned int seg_32bit : 1;
    unsigned int contents : 2;
    unsigned int read_exec_only : 1;
    unsigned int limit_in_pages : 1;
    unsigned int seg_not_present : 1;
    unsigned int useable : 1;
    unsigned int lm : 1;
} thread_area_t;
typedef struct thread_area_32_s {
    int entry_number;
    uint32_t base_addr;
    unsigned int limit;
    unsigned int seg_32bit : 1;
    unsigned int contents : 2;
    unsigned int read_exec_only : 1;
    unsigned int limit_in_pages : 1;
    unsigned int seg_not_present : 1;
    unsigned int useable : 1;
} thread_area_32_t;

int GetTID();

uint32_t my_set_thread_area_32(x64emu_t* emu, thread_area_32_t* td)
{
    printf_log(LOG_DEBUG, "%04d| set_thread_area_32(%p[%d/base=%p/limit=%u/32bits:%u/%u/%u...])\n", GetTID(), td, td->entry_number, (void*)(uintptr_t)td->base_addr, td->limit_in_pages, td->seg_32bit, td->contents, td->read_exec_only);

    int isempty = 0;
    // first, check if the "user_desc", here td, is "empty"
    if (td->read_exec_only == 1 && td->seg_not_present == 1)
        if (!td->base_addr
            && !td->limit
            && !td->seg_32bit
            && !td->contents
            && !td->limit_in_pages
            && !td->useable)
            isempty = 1;
    int idx = td->entry_number;
    if (idx == -1) {
        // find a free one
        for (int i = 9; i < 15 && idx == -1; ++i)
            if (!my_context->segtls[i].present)
                idx = i;
        if (idx == -1) {
            errno = ESRCH;
            return (uint32_t)-1;
        }
        td->entry_number = idx;
    }
    if (isempty && (td->entry_number < 9 || td->entry_number > 15)) {
        errno = EINVAL;
        return (uint32_t)-1;
    }
    if (isempty) {
        memset(&my_context->segtls[td->entry_number], 0, sizeof(base_segment_t));
        return 0;
    }
    if ((idx < 9 || idx > 15)) {
        errno = EINVAL;
        return (uint32_t)-1;
    }

    my_context->segtls[idx].base = td->base_addr;
    my_context->segtls[idx].limit = td->limit;
    my_context->segtls[idx].present = 1;
    my_context->segtls[idx].is32bits = 1;
    if (!my_context->segtls[idx].key_init) {
        pthread_key_create(&my_context->segtls[idx].key, NULL);
        my_context->segtls[idx].key_init = 1;
    }

    pthread_setspecific(my_context->segtls[idx].key, (void*)my_context->segtls[idx].base);

    ResetSegmentsCache(emu);

    return 0;
}

uint32_t my_modify_ldt(x64emu_t* emu, int op, thread_area_t* td, int size)
{
    printf_log(LOG_DEBUG, "%04d| modify_ldt(0x%x, %p[0x%x/base=%p/limit=%u/32bits:%u/%u/%u...], %d)\n", GetTID(), op, td, td->entry_number, (void*)td->base_addr, td->limit_in_pages, td->seg_32bit, td->contents, td->read_exec_only, size);
    if (!td) {
        errno = EFAULT;
        return (uint32_t)-1;
    }
    if (op != 0x11) {
        errno = ENOSYS;
        return (uint32_t)-1;
    }
    if (!td->seg_32bit) {
        // not handling 16bits segments for now
        errno = EINVAL;
        return (uint32_t)-1;
    }

    int idx = td->entry_number;
    if (idx < 9 || idx > 15) {
        errno = EINVAL;
        return (uint32_t)-1;
    }

    /*
    my_context->segtls[idx].base = td->base_addr;
    my_context->segtls[idx].limit = td->limit;
    pthread_setspecific(my_context->segtls[idx].key, (void*)my_context->segtls[idx].base);
    */

    ResetSegmentsCache(emu);

    return 0;
}

static void* GetSeg43Base();
static const char* arch_prctl_param(int code)
{
    static char ret[10] = { 0 };
    switch (code) {
        case 0x1001: return "ARCH_SET_GS";
        case 0x1002: return "ARCH_SET_FS";
        case 0x1003: return "ARCH_GET_FS";
        case 0x1004: return "ARCH_GET_GS";
    }
    sprintf(ret, "0x%x", code);
    return ret;
}
int my_arch_prctl(x64emu_t* emu, int code, void* addr)
{
    printf_log(LOG_DEBUG, "%04d| arch_prctl(%s, %p) (RSP=%p, FS=0x%x, GS=0x%x)\n", GetTID(), arch_prctl_param(code), addr, (void*)R_RSP, emu->segs[_FS], emu->segs[_GS]);

#define ARCH_SET_GS 0x1001
#define ARCH_SET_FS 0x1002
#define ARCH_GET_FS 0x1003
#define ARCH_GET_GS 0x1004
    int seg = 0;
    int idx = 0;
    errno = 0;
    switch (code) {
        case ARCH_GET_GS:
            *(void**)addr = GetSegmentBase(emu->segs[_GS]);
            return 0;
        case ARCH_GET_FS:
            *(void**)addr = GetSegmentBase(emu->segs[_FS]);
            return 0;
        case ARCH_SET_FS:
        case ARCH_SET_GS:
            seg = (code == ARCH_SET_FS) ? _FS : _GS;
            int idx = -1;
            // search if it's a TLS base
            if (GetSeg43Base() == addr)
                idx = 0x43 >> 3;
            // Is this search only occurs when seg==0?
            for (int i = 9; i < 15 && idx == -1; ++i)
                if (my_context->segtls[i].present && my_context->segtls[i].base == (uintptr_t)addr)
                    idx = i;
            // found...
            if (idx != -1) {
                printf_log(LOG_DEBUG, "Changing segment selector from 0x%x to 0x%x\n", emu->segs[seg], (idx << 3) + 3);
                emu->segs[seg] = (idx << 3) + 3;
            }
            if (emu->segs[seg] == 0) {
                printf_log(LOG_DEBUG, "Warning, set seg, but it's 0!\n");
                errno = EINVAL;
                return -1;
            }
            idx = emu->segs[seg] >> 3;
            if (idx < 0 || idx > 15) {
                errno = EINVAL;
                return -1;
            }
            emu->segs_serial[seg] = 0;
            my_context->segtls[idx].base = (uintptr_t)addr;
            my_context->segtls[idx].limit = 0;
            my_context->segtls[idx].present = 1;
            if (idx > 8 && !my_context->segtls[idx].key_init) {
                pthread_key_create(&my_context->segtls[idx].key, NULL);
                my_context->segtls[idx].key_init = 1;
            }
            if (my_context->segtls[idx].key_init)
                pthread_setspecific(my_context->segtls[idx].key, addr);
            ResetSegmentsCache(emu);
            return 0;
    }
    // other are unsupported
    printf_log(LOG_INFO, "warning, call to unsupported arch_prctl(0x%x, %p)\n", code, addr);
    errno = ENOSYS;
    return -1;
}


#define POS_TLS 0x200
/*
 tls record should looks like:
 void*      tcb             0x00
 void*      dtv             0x08
 void*      self            0x10
 int        multiple        0x18
 int        gscope          0x1c
 void*      sysinfo         0x20
 uintptr_t  stack_guard     0x28
 uitnptr_t  pointer_guard   0x30
 uint64_t   vgetcpu[2]      0x38
 uint32_t   features        0x48
 int        unused          0x4c
 void*      private[4]      0x50
 void*      private_ss      0x70
 uintptr_t  ssp_base        0x78
 .... padding ....          0x200?
*/
#define NELFSIZE_MASK 0x3FF
#define NELFSIZE      (NELFSIZE_MASK + 1)
static int sizeDTS(box64context_t* context)
{
    return ((context->elfsize + NELFSIZE_MASK) & ~NELFSIZE_MASK) * 16;
}
static int sizeTLSData(int s)
{
    uint32_t mask = 0xffff /*box64_nogtk?0xffff:0x1fff*/; // x86_64 does the mapping per 64K blocks, so it makes sense to have it this large
    return (s + mask) & ~mask;
}

static tlsdatasize_t* setupTLSData(box64context_t* context)
{
    // Setup the GS segment:
    int dtssize = sizeDTS(context);
    int datasize = sizeTLSData(context->tlssize);
    void* ptr_oversized = (char*)box_malloc(dtssize + POS_TLS + datasize);
    void* ptr = (void*)((uintptr_t)ptr_oversized + datasize);
    memcpy((void*)((uintptr_t)ptr - context->tlssize), context->tlsdata, context->tlssize);
    tlsdatasize_t* data = (tlsdatasize_t*)box_calloc(1, sizeof(tlsdatasize_t));
    data->data = ptr;
    data->tlssize = context->tlssize;
    data->ptr = ptr_oversized;
    data->n_elfs = context->elfsize;
    pthread_setspecific(context->tlskey, data);
    // copy canary...
    memset((void*)((uintptr_t)ptr), 0, POS_TLS + dtssize);                  // set to 0 remining bytes
    memcpy((void*)((uintptr_t)ptr + 0x28), context->canary, sizeof(void*)); // put canary in place
    uintptr_t tlsptr = (uintptr_t)ptr;
    memcpy((void*)((uintptr_t)ptr + 0x0), &tlsptr, sizeof(void*));
    memcpy((void*)((uintptr_t)ptr + 0x10), &tlsptr, sizeof(void*)); // set tcb and self same address
    uintptr_t dtp = (uintptr_t)ptr + POS_TLS;
    memcpy((void*)(tlsptr + sizeof(void*)), &dtp, sizeof(void*));
    if (dtssize) {
        for (int i = 0; i < context->elfsize; ++i) {
            // set pointer
            dtp = (uintptr_t)ptr + GetTLSBase(context->elfs[i]);
            *(uint64_t*)((uintptr_t)ptr + POS_TLS + i * 16) = dtp;
            *(uint64_t*)((uintptr_t)ptr + POS_TLS + i * 16 + 8) = i; // index
        }
    }
    memcpy((void*)((uintptr_t)ptr + 0x20), &context->vsyscall, sizeof(void*)); // address of vsyscall
    return data;
}

static void* fillTLSData(box64context_t* context)
{
    mutex_lock(&context->mutex_tls);
    tlsdatasize_t* data = setupTLSData(context);
    mutex_unlock(&context->mutex_tls);
    return data;
}

static void* resizeTLSData(box64context_t* context, void* oldptr)
{
    mutex_lock(&context->mutex_tls);
    tlsdatasize_t* oldata = (tlsdatasize_t*)oldptr;
    if (sizeTLSData(oldata->tlssize) != sizeTLSData(context->tlssize) || (oldata->n_elfs / NELFSIZE) != (context->elfsize / NELFSIZE)) {
        if (sizeTLSData(oldata->tlssize)) {
            printf_log(LOG_INFO, "Warning, resizing of TLS occurred! size: %d->%d / n_elfs: %d->%d\n", sizeTLSData(oldata->tlssize), sizeTLSData(context->tlssize), 1 + (oldata->n_elfs / NELFSIZE), 1 + (context->elfsize / NELFSIZE));
        }
        tlsdatasize_t* data = setupTLSData(context);
        // copy the relevent old part, in case something changed
        memcpy((void*)((uintptr_t)data->data - oldata->tlssize), (void*)((uintptr_t)oldata->data - oldata->tlssize), oldata->tlssize);
        // all done, update new size, free old pointer and exit
        mutex_unlock(&context->mutex_tls);
        free_tlsdatasize(oldptr);
        return data;
    } else {
        // keep the same tlsdata, but fill in the blanks
        // adjust tlsdata
        void* ptr = oldata->data;
        if (context->tlssize != oldata->tlssize) {
            memcpy((void*)((uintptr_t)ptr - context->tlssize), context->tlsdata, context->tlssize - oldata->tlssize);
            oldata->tlssize = context->tlssize;
        }
        // adjust DTS
        if (oldata->n_elfs != context->elfsize) {
            uintptr_t dtp = (uintptr_t)ptr + POS_TLS;
            for (int i = oldata->n_elfs; i < context->elfsize; ++i) {
                // set pointer
                dtp = (uintptr_t)ptr + GetTLSBase(context->elfs[i]);
                *(uint64_t*)((uintptr_t)ptr + POS_TLS + i * 16) = dtp;
                *(uint64_t*)((uintptr_t)ptr + POS_TLS + i * 16 + 8) = i; // index
            }
            oldata->n_elfs = context->elfsize;
        }
        mutex_unlock(&context->mutex_tls);
        return oldata;
    }
}

tlsdatasize_t* getTLSData(box64context_t* context)
{
    tlsdatasize_t* ptr = NULL;
    if (!ptr)
        if ((ptr = (tlsdatasize_t*)pthread_getspecific(context->tlskey)) == NULL) {
            ptr = (tlsdatasize_t*)fillTLSData(context);
        }
    if (ptr->tlssize != context->tlssize)
        ptr = (tlsdatasize_t*)resizeTLSData(context, ptr);
    return ptr;
}

static void* GetSeg43Base()
{
    tlsdatasize_t* ptr = getTLSData(my_context);
    return ptr->data;
}

void* GetSegmentBase(uint32_t desc)
{
    if (!desc) {
        printf_log(LOG_NONE, "Warning, accessing segment NULL\n");
        return NULL;
    }
    int base = desc >> 3;
    if (base == 0x8 && !my_context->segtls[base].key_init)
        return GetSeg43Base();
    if (base > 15) {
        printf_log(LOG_NONE, "Warning, accessing segment unknown 0x%x or unset\n", desc);
        return NULL;
    }
    if (my_context->segtls[base].key_init) {
        void* ptr = pthread_getspecific(my_context->segtls[base].key);
        return ptr;
    }

    void* ptr = (void*)my_context->segtls[base].base;
    return ptr;
}
