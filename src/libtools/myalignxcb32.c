#include <stdint.h>
#include <pthread.h>
#include <string.h>

#include "debug.h"
#include "myalign32.h"
#include "threads32.h"
#include "my_xcb_defs.h"
  
typedef struct my_xcb_ext_32_s {
    fake_phtread_mutex_t lock;
    ptr_t extensions;   //struct lazyreply *
    int extensions_size;
} my_xcb_ext_32_t;

typedef struct my_xcb_xid_32_s {
    fake_phtread_mutex_t lock;
    uint32_t last;
    uint32_t base;
    uint32_t max;
    uint32_t inc;
} my_xcb_xid_32_t;

typedef struct my_xcb_out_32_s {
    pthread_cond_t cond;
    int writing;
    pthread_cond_t socket_cond;
    ptr_t return_socket;//void (*return_socket)(void *closure);
    ptr_t socket_closure;//void *
    int socket_moving;
    char queue[16384];
    int queue_len;
    uint64_t request;
    uint64_t request_written;
    fake_phtread_mutex_t reqlenlock;
    int maximum_request_length_tag;
    uint32_t maximum_request_length;
    my_xcb_fd_t out_fd;
} my_xcb_out_32_t;

typedef struct my_xcb_in_32_s {
    pthread_cond_t  event_cond;
    int             reading;
    char            queue[4096];
    int             queue_len;
    uint64_t        request_expected;
    uint64_t        request_read;
    uint64_t        request_completed;
    ptr_t           current_reply;//struct reply_list *
    ptr_t           current_reply_tail;//struct reply_list **
    ptr_t           replies;//void*
    ptr_t           events;//struct event_list *
    ptr_t           events_tail;//struct event_list **
    ptr_t           readers;//struct reader_list *
    ptr_t           special_waiters;//struct special_list *
    ptr_t           pending_replies;//struct pending_reply *
    ptr_t           pending_replies_tail;//struct pending_reply **
    my_xcb_fd_t     in_fd;
    ptr_t           special_events;//struct xcb_special_event *
} my_xcb_in_32_t;

typedef struct my_xcb_connection_32_s {
    int has_error;
    ptr_t setup;    //void *
    int fd;
    fake_phtread_mutex_t iolock;
    my_xcb_in_32_t in;
    my_xcb_out_32_t out;
    my_xcb_ext_32_t ext;
    my_xcb_xid_32_t xid;
} my_xcb_connection_32_t;

#define NXCB 32
static my_xcb_connection_t* my_xcb_connects[NXCB] = {0};
static my_xcb_connection_32_t i386_xcb_connects[NXCB] = {0};
static my_xcb_connection_t* tmp_xcb_connect = NULL;
static my_xcb_connection_32_t i386_tmp_xcb_connect = {0};

void* temp_xcb_connection32(void* src);

void* align_xcb_connection32(void* src)
{
    if(!src)
        return src;
    // find it
    my_xcb_connection_t * dest = NULL;
    for(int i=0; i<NXCB && !dest; ++i)
        if(src==&i386_xcb_connects[i])
            dest = my_xcb_connects[i];
    #if 1
    if(!dest) {
        dest = temp_xcb_connection32(src);
    }
    #else
    if(!dest) {
        printf_log(LOG_NONE, "Error, xcb_connect %p not found\n", src);
        abort();
    }
    #endif
    #if 1
    // do not update most values
    my_xcb_connection_32_t* source = src;
    dest->has_error = source->has_error;
    dest->setup = from_ptrv(source->setup);
    dest->fd = source->fd;
    //memcpy(&dest->iolock, source->iolock, MUTEX_SIZE_X64);
    //dest->in = source->in;
    //dest->out = source->out;
    //memcpy(&dest->ext.lock, source->ext.lock, MUTEX_SIZE_X64);
    dest->ext.extensions = from_ptrv(source->ext.extensions);
    dest->ext.extensions_size = source->ext.extensions_size;
    //memcpy(&dest->xid.lock, source->xid.lock, MUTEX_SIZE_X64);
    dest->xid.base = source->xid.base;
    dest->xid.inc = source->xid.inc;
    dest->xid.last = source->xid.last;
    dest->xid.max = source->xid.last;
    #endif
    return dest;
}

static void setup_mutex(fake_phtread_mutex_t* mutex, pthread_mutex_t* src)
{
    memset(mutex, 0, sizeof(*mutex));
    mutex->__kind = KIND_SIGN;
    mutex->real_mutex = to_ptrv(src);
}

void unalign_xcb_connection32(void* src, void* dst)
{
    if(!src || !dst || src==dst)
        return;
    // update values
    my_xcb_connection_t* source = src;
    my_xcb_connection_32_t* dest = dst;
    dest->has_error = source->has_error;
    dest->setup = to_ptrv(source->setup);
    dest->fd = source->fd;
    setup_mutex(&dest->iolock, &source->iolock);
    dest->in.event_cond = source->in.event_cond;
    dest->in.reading = source->in.reading;
    memcpy(dest->in.queue, source->in.queue, sizeof(dest->in.queue));
    dest->in.queue_len = source->in.queue_len;
    dest->in.request_expected = source->in.request_expected;
    dest->in.request_read = source->in.request_read;
    dest->in.request_completed = source->in.request_completed;
    dest->in.current_reply = to_ptrv(source->in.current_reply);
    dest->in.current_reply_tail = to_ptrv(source->in.current_reply_tail);
    dest->in.replies = to_ptrv(source->in.replies);
    dest->in.events = to_ptrv(source->in.events);
    dest->in.events_tail = to_ptrv(source->in.events_tail);
    dest->in.readers = to_ptrv(source->in.readers);
    dest->in.special_waiters = to_ptrv(source->in.special_waiters);
    dest->in.pending_replies = to_ptrv(source->in.pending_replies);
    dest->in.pending_replies_tail = to_ptrv(source->in.pending_replies_tail);
    dest->in.in_fd = source->in.in_fd;
    dest->in.special_events = to_ptrv(source->in.special_events);
    setup_mutex(&dest->out.reqlenlock, &source->out.reqlenlock);
    dest->out.cond = source->out.cond;
    dest->out.maximum_request_length = source->out.maximum_request_length;
    dest->out.maximum_request_length_tag = source->out.maximum_request_length_tag;
    dest->out.out_fd = source->out.out_fd;
    memcpy(dest->out.queue, source->out.queue, sizeof(dest->out.queue));
    dest->out.queue_len = source->out.queue_len;
    dest->out.request = source->out.request;
    dest->out.request_written = source->out.request_written;
    dest->out.return_socket = to_ptrv(source->out.return_socket);
    dest->out.socket_closure = to_ptrv(source->out.socket_closure);
    dest->out.socket_cond = source->out.socket_cond;
    dest->out.socket_moving = source->out.socket_moving;
    dest->out.writing = source->out.writing;
    setup_mutex(&dest->ext.lock, &source->ext.lock);
    dest->ext.extensions = to_ptrv(source->ext.extensions);
    dest->ext.extensions_size = source->ext.extensions_size;
    setup_mutex(&dest->xid.lock, &source->xid.lock);
    dest->xid.base = source->xid.base;
    dest->xid.inc = source->xid.inc;
    dest->xid.last = source->xid.last;
    dest->xid.max = source->xid.last;
}

void* add_xcb_connection32(void* src)
{
    if(!src)
        return src;
    // check if already exist
    for(int i=0; i<NXCB; ++i)
        if(my_xcb_connects[i] == src) {
            unalign_xcb_connection32(src, &i386_xcb_connects[i]);
            return &i386_xcb_connects[i];
        }
    // find a free slot
    for(int i=0; i<NXCB; ++i)
        if(!my_xcb_connects[i]) {
            my_xcb_connects[i] = src;
            unalign_xcb_connection32(src, &i386_xcb_connects[i]);
            return &i386_xcb_connects[i];
        }
    printf_log(LOG_NONE, "Error, no more free xcb_connect 32bits slot for %p\n", src);
    return src;
}

void* temp_xcb_connection32(void* src)
{
    tmp_xcb_connect = src;
    unalign_xcb_connection32(src, &i386_tmp_xcb_connect);
    return &i386_tmp_xcb_connect;
}

void del_xcb_connection32(void* src)
{
    if(!src)
        return;
    // find it
    for(int i=0; i<NXCB; ++i)
        if(src==&i386_xcb_connects[i]) {
            my_xcb_connects[i] = NULL;
            memset(&i386_xcb_connects[i], 0, sizeof(my_xcb_connection_32_t));
            return;
        }
    printf_log(LOG_NONE, "Error, 32bits xcb_connect %p not found for deletion\n", src);
}
