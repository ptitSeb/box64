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

typedef struct thread_area_s
{
             int  entry_number;
        uintptr_t base_addr;
    unsigned int  limit;
    unsigned int  seg_32bit:1;
    unsigned int  contents:2;
    unsigned int  read_exec_only:1;
    unsigned int  limit_in_pages:1;
    unsigned int  seg_not_present:1;
    unsigned int  useable:1;
} thread_area_t;

static pthread_once_t thread_key_once0 = PTHREAD_ONCE_INIT;
static pthread_once_t thread_key_once1 = PTHREAD_ONCE_INIT;
static pthread_once_t thread_key_once2 = PTHREAD_ONCE_INIT;
static pthread_once_t thread_key_once3 = PTHREAD_ONCE_INIT;

static void thread_key_alloc0() {
	pthread_key_create(&my_context->segtls[0].key, NULL);
}
static void thread_key_alloc1() {
	pthread_key_create(&my_context->segtls[1].key, NULL);
}
static void thread_key_alloc2() {
	pthread_key_create(&my_context->segtls[2].key, NULL);
}
static void thread_key_alloc3() {
	pthread_key_create(&my_context->segtls[3].key, NULL);
}

uint32_t my_set_thread_area(thread_area_t* td)
{
    printf_log(LOG_DEBUG, "set_thread_area(%p[%d/base=%p/limit=%u/32bits:%u/%u/%u...])\n", td, td->entry_number, (void*)td->base_addr, td->limit_in_pages, td->seg_32bit, td->contents, td->read_exec_only);

    int isempty = 0;
    // first, check if the "user_desc", here td, is "empty"
    if(td->read_exec_only==1 && td->seg_not_present==1)
        if( !td->base_addr 
         && !td->limit
         && !td->seg_32bit 
         && !td->contents 
         && !td->limit_in_pages 
         && !td->useable)
            isempty = 1;
    int idx = td->entry_number;
    if(idx==-1) {
        // find a free one
        for (int i=0; i<3 && idx==-1; ++i)
            if(!my_context->segtls[i].present)
                idx=i;
        if(idx==-1) {
            errno = ESRCH;
            return (uint32_t)-1;
        }
        idx+=7;
        td->entry_number = idx;
    }
    if(isempty && (td->entry_number<7 || td->entry_number>7+2)) {
        errno = EINVAL;
        return (uint32_t)-1;
    }
    if(isempty) {
        memset(&my_context->segtls[td->entry_number-7], 0, sizeof(base_segment_t));
        return 0;
    }
    if((idx<7 || idx>7+2)) {
        errno = EINVAL;
        return (uint32_t)-1;
    }

    my_context->segtls[idx-7].base = td->base_addr;
    my_context->segtls[idx-7].limit = td->limit;
    my_context->segtls[idx-7].present = 1;
    switch (idx-7) {
        case 0:	pthread_once(&thread_key_once0, thread_key_alloc0); break;
        case 1:	pthread_once(&thread_key_once1, thread_key_alloc1); break;
        case 2:	pthread_once(&thread_key_once2, thread_key_alloc2); break;
    }

    pthread_setspecific(my_context->segtls[idx-7].key, (void*)my_context->segtls[idx-7].base);

    ResetSegmentsCache(thread_get_emu());

    return 0;
}

uint32_t my_modify_ldt(x64emu_t* emu, int op, thread_area_t* td, int size)
{
    printf_log(/*LOG_DEBUG*/LOG_INFO, "modify_ldt(0x%x, %p[0x%x/base=%p/limit=%u/32bits:%u/%u/%u...], %d)\n", op, td, td->entry_number, (void*)td->base_addr, td->limit_in_pages, td->seg_32bit, td->contents, td->read_exec_only, size);
    if(!td) {
        errno = EFAULT;
        return (uint32_t)-1;
    }
    if(op!=0x11) {
        errno = ENOSYS;
        return (uint32_t)-1;
    }
    if(!td->seg_32bit) {
        // not handling 16bits segments for now
        errno = EINVAL;
        return (uint32_t)-1;
    }

    int idx = td->entry_number - 7;
    if(idx<0 || idx>2) {
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

int my_arch_prctl(x64emu_t *emu, int code, void* addr)
{
    #define ARCH_SET_GS          0x1001
    #define ARCH_SET_FS          0x1002
    #define ARCH_GET_FS          0x1003
    #define ARCH_GET_GS          0x1004
    switch(code) {
        case ARCH_GET_GS:
            *(void**)addr = GetSegmentBase(emu->segs[_GS]);
            return 0;
        case ARCH_SET_GS:
            pthread_once(&thread_key_once3, thread_key_alloc3);
            if(emu->segs[_GS]!=(0xa<<3))
                emu->segs[_GS] = 0xa<<3;    // should not move!
            emu->segs_serial[_GS] = 0;
            my_context->segtls[3].base = (uintptr_t)addr;
            my_context->segtls[3].limit = 0;
            my_context->segtls[3].present = 1;
            pthread_setspecific(my_context->segtls[3].key, (void*)my_context->segtls[3].base);
            ResetSegmentsCache(emu);
            return 0;
        case ARCH_GET_FS:
            *(void**)addr = GetSegmentBase(emu->segs[_FS]);
            return 0;
    }
    // other are unsupported
    printf_log(LOG_INFO, "warning, call to unsupported arch_prctl(0x%x, %p)\n", code, addr);
    return -1;
}


#define POS_TLS     0x200
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
static int sizeDTS(box64context_t* context)
{
    return ((context->elfsize+0xff)&~0xff)*16;
}
static int sizeTLSData(int s)
{
    uint32_t mask = 0xffff/*box64_nogtk?0xffff:0x1fff*/;    // x86_64 does the mapping per 64K blocks, so it makes sense to have it this large
    return (s+mask)&~mask;
}

static tlsdatasize_t* setupTLSData(box64context_t* context)
{
    // Setup the GS segment:
    int dtssize = sizeDTS(context);
    int datasize = sizeTLSData(context->tlssize);
    void *ptr_oversized = (char*)box_malloc(dtssize+POS_TLS+datasize);
    void *ptr = (void*)((uintptr_t)ptr_oversized + datasize);
    memcpy((void*)((uintptr_t)ptr-context->tlssize), context->tlsdata, context->tlssize);
    tlsdatasize_t *data = (tlsdatasize_t*)box_calloc(1, sizeof(tlsdatasize_t));
    data->data = ptr;
    data->tlssize = context->tlssize;
    data->ptr = ptr_oversized;
    data->n_elfs = context->elfsize;
    pthread_setspecific(context->tlskey, data);
    // copy canary...
    memset((void*)((uintptr_t)ptr), 0, POS_TLS+dtssize);            // set to 0 remining bytes
    memcpy((void*)((uintptr_t)ptr+0x28), context->canary, sizeof(void*));      // put canary in place
    uintptr_t tlsptr = (uintptr_t)ptr;
    memcpy((void*)((uintptr_t)ptr+0x0), &tlsptr, sizeof(void*));
    memcpy((void*)((uintptr_t)ptr+0x10), &tlsptr, sizeof(void*));  // set tcb and self same address
    uintptr_t dtp = (uintptr_t)ptr+POS_TLS;
    memcpy((void*)(tlsptr+sizeof(void*)), &dtp, sizeof(void*));
    if(dtssize) {
        for (int i=0; i<context->elfsize; ++i) {
            // set pointer
            dtp = (uintptr_t)ptr + GetTLSBase(context->elfs[i]);
            *(uint64_t*)((uintptr_t)ptr+POS_TLS+i*16) = dtp;
            *(uint64_t*)((uintptr_t)ptr+POS_TLS+i*16+8) = i; // index
        }
    }
    memcpy((void*)((uintptr_t)ptr+0x20), &context->vsyscall, sizeof(void*));  // address of vsyscall
    return data;
}

void* fillTLSData(box64context_t *context)
{
        pthread_mutex_lock(&context->mutex_tls);
        tlsdatasize_t *data = setupTLSData(context);
        pthread_mutex_unlock(&context->mutex_tls);
        return data;
}

void* resizeTLSData(box64context_t *context, void* oldptr)
{
        pthread_mutex_lock(&context->mutex_tls);
        tlsdatasize_t* oldata = (tlsdatasize_t*)oldptr;
        if(sizeTLSData(oldata->tlssize)!=sizeTLSData(context->tlssize) || (oldata->n_elfs/0xff)!=(context->elfsize/0xff)) {
            printf_log(LOG_INFO, "Warning, resizing of TLS occured! size: %d->%d / n_elfs: %d->%d\n", sizeTLSData(oldata->tlssize), sizeTLSData(context->tlssize), 1+(oldata->n_elfs/0xff), 1+(context->elfsize/0xff));
            tlsdatasize_t *data = setupTLSData(context);
            // copy the relevent old part, in case something changed
            memcpy((void*)((uintptr_t)data->data-oldata->tlssize), (void*)((uintptr_t)oldata->data-oldata->tlssize), oldata->tlssize);
            // all done, update new size, free old pointer and exit
            pthread_mutex_unlock(&context->mutex_tls);
            free_tlsdatasize(oldptr);
            return data;
        } else {
            // keep the same tlsdata, but fill in the blanks
            // adjust tlsdata
            void *ptr = oldata->data;
            if(context->tlssize!=oldata->tlssize) {
                memcpy((void*)((uintptr_t)ptr-context->tlssize), context->tlsdata, context->tlssize-oldata->tlssize);
                oldata->tlssize = context->tlssize;
            }
            // adjust DTS
            if(oldata->n_elfs!=context->elfsize) {
                uintptr_t dtp = (uintptr_t)ptr+POS_TLS;
                for (int i=oldata->n_elfs; i<context->elfsize; ++i) {
                    // set pointer
                    dtp = (uintptr_t)ptr + GetTLSBase(context->elfs[i]);
                    *(uint64_t*)((uintptr_t)ptr+POS_TLS+i*16) = dtp;
                    *(uint64_t*)((uintptr_t)ptr+POS_TLS+i*16+8) = i; // index
                }
                oldata->n_elfs = context->elfsize;
            }
            pthread_mutex_unlock(&context->mutex_tls);
            return oldata;
        }
}

static void* GetSeg33Base()
{
    tlsdatasize_t* ptr;
    if ((ptr = (tlsdatasize_t*)pthread_getspecific(my_context->tlskey)) == NULL) {
        ptr = (tlsdatasize_t*)fillTLSData(my_context);
    }
    if(ptr->tlssize != my_context->tlssize)
        ptr = (tlsdatasize_t*)resizeTLSData(my_context, ptr);
    return ptr->data;
}

void* GetSegmentBase(uint32_t desc)
{
    if(!desc) {
        printf_log(LOG_NONE, "Warning, accessing segment NULL\n");
        return NULL;
    }
    int base = desc>>3;
    if(base==0xe || base==0xf)
        return NULL;    // regular value...
    if(base==0x6)
        return GetSeg33Base();

    if(base>6 && base<11 && my_context->segtls[base-7].present) {
        void* ptr = pthread_getspecific(my_context->segtls[base-7].key);
        return ptr;
    }

    printf_log(LOG_NONE, "Warning, accessing segment unknown 0x%x or unset\n", desc);
    return NULL;
}
