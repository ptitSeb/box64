#include "mailbox.h"

#include <semaphore.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "custommem.h"
#include "debug.h"

//#define DEBUG_MAILBOX

// === REQUESTS ===

typedef enum available_bits_s {
    AVAILABLE_BITS_32 = 32,
    AVAILABLE_BITS_47 = 47,
    AVAILABLE_BITS_MAX = 0,
} available_bits_e;

typedef struct request_s {
    enum request_type_e {
        REQUEST_HALT,
        REQUEST_UPD_PROTECT,
        REQUEST_SET_PROTECT,
        REQUEST_FRE_PROTECT,
        REQUEST_GET_PROTECT,
        REQUEST_LOD_PROTECT,
#ifdef DYNAREC
        REQUEST_PROT_DB,
        REQUEST_UNPROT_DB,
        REQUEST_ISPROT_DB,
#endif
        REQUEST_FIND,
        REQUEST_FIND_HINT,
    } type;
    sem_t *answered;
    void *ansBuf;
    union {
        struct {
            uintptr_t addr;
        } protect;
        struct {
            uintptr_t addr;
            size_t    size;
        } size_protect;
        struct {
            uintptr_t addr;
            size_t    size;
            uint32_t  prot;
        } prot_protect;
        struct {
            available_bits_e availablebits;
            uintptr_t        size;
        } find;
        struct {
            available_bits_e availablebits;
            uintptr_t        size;
            void*            hint;
        } find_near;
    } data;
#ifdef DEBUG_MAILBOX
    int reqno;
#endif
} request_t;

request_t curHdr;
sem_t empty;
sem_t full;
sem_t finished;

static void sendRequest(request_t *rhdr, void *ans) {
#ifdef DEBUG_MAILBOX
    static int reqno = 0;
    rhdr->reqno = ++reqno;
    printf_log(LOG_DEBUG, "[REQ%05u] Sending request %02X (ans=%p)\n", rhdr->reqno, rhdr->type, ans);
#endif
    rhdr->ansBuf = ans;
    if (ans) {
        sem_t answered;
        sem_init(&answered, 0, 0);
        rhdr->answered = &answered;
#ifdef DEBUG_MAILBOX
        printf_log(LOG_DEBUG, "[REQ%05u] Will wait for answer\n", rhdr->reqno);
#endif
        sem_wait(&empty);
        curHdr = *rhdr;
        sem_post(&full);
        sem_wait(&answered);
    } else {
        sem_wait(&empty);
        curHdr = *rhdr;
        sem_post(&full);
    }
#ifdef DEBUG_MAILBOX
    printf_log(LOG_DEBUG, "[REQ%05u] Done, leaving\n", rhdr->reqno);
#endif
}

// A function per request (avoids exposing dangerous internal functions)
void updateProtection(uintptr_t addr, size_t size, uint32_t prot) {
    request_t req;
    req.type = REQUEST_UPD_PROTECT;
    req.data.prot_protect.addr = addr;
    req.data.prot_protect.size = size;
    req.data.prot_protect.prot = prot;
    void *none = NULL;
    sendRequest(&req, &none); // Need to wait for completion
}
void setProtection(uintptr_t addr, size_t size, uint32_t prot) {
    request_t req;
    req.type = REQUEST_SET_PROTECT;
    req.data.prot_protect.addr = addr;
    req.data.prot_protect.size = size;
    req.data.prot_protect.prot = prot;
    sendRequest(&req, NULL);
}
void freeProtection(uintptr_t addr, size_t size) {
    request_t req;
    req.type = REQUEST_FRE_PROTECT;
    req.data.size_protect.addr = addr;
    req.data.size_protect.size = size;
    void *none = NULL;
    sendRequest(&req, &none); // Need to wait for completion
}
uint32_t getProtection(uintptr_t addr) {
    request_t req;
    req.type = REQUEST_GET_PROTECT;
    req.data.protect.addr = addr;
    uint32_t ret;
    sendRequest(&req, &ret);
    return ret;
}
void loadProtectionFromMap() {
    request_t req;
    req.type = REQUEST_LOD_PROTECT;
    void *none = NULL;
    sendRequest(&req, &none); // Need to wait for completion
}
#ifdef DYNAREC
void protectDB(uintptr_t addr, size_t size) {
    request_t req;
    req.type = REQUEST_PROT_DB;
    req.data.size_protect.addr = addr;
    req.data.size_protect.size = size;
    void *none = NULL;
    sendRequest(&req, &none); // Need to wait for completion
}
void unprotectDB(uintptr_t addr, size_t size) {
    request_t req;
    req.type = REQUEST_UNPROT_DB;
    req.data.size_protect.addr = addr;
    req.data.size_protect.size = size;
    void *none = NULL;
    sendRequest(&req, &none); // Need to wait for completion
}
int isprotectedDB(uintptr_t addr, size_t size) {
    request_t req;
    req.type = REQUEST_ISPROT_DB;
    req.data.size_protect.addr = addr;
    req.data.size_protect.size = size;
    int ret;
    sendRequest(&req, &ret);
    return ret;
}
#endif
void* find32bitBlock(size_t size) {
    request_t req;
    req.type = REQUEST_FIND;
    req.data.find.size = size;
    req.data.find.availablebits = AVAILABLE_BITS_32;
    void* ret;
    sendRequest(&req, &ret);
    return ret;
}
void* findBlockNearHint(void* hint, size_t size) {
    request_t req;
    req.type = REQUEST_FIND_HINT;
    req.data.find_near.size = size;
    req.data.find_near.availablebits = AVAILABLE_BITS_MAX;
    req.data.find_near.hint = hint;
    void* ret;
    sendRequest(&req, &ret);
    return ret;
}
void* find47bitBlock(size_t size) {
    request_t req;
    req.type = REQUEST_FIND;
    req.data.find.size = size;
    req.data.find.availablebits = AVAILABLE_BITS_47;
    void* ret;
    sendRequest(&req, &ret);
    return ret;
}
void* find47bitBlockNearHint(void* hint, size_t size) {
    request_t req;
    req.type = REQUEST_FIND_HINT;
    req.data.find_near.size = size;
    req.data.find_near.availablebits = AVAILABLE_BITS_47;
    req.data.find_near.hint = hint;
    void* ret;
    sendRequest(&req, &ret);
    return ret;
}

// === MAILBOX ===

void internalUpdateProtection(uintptr_t addr, size_t size, uint32_t prot);
void internalSetProtection(uintptr_t addr, size_t size, uint32_t prot);
void internalFreeProtection(uintptr_t addr, size_t size);
uint32_t internalGetProtection(uintptr_t addr);
void internalLoadProtectionFromMap();
#ifdef DYNAREC
void internalProtectDB(uintptr_t addr, size_t size);
void internalUnprotectDB(uintptr_t addr, size_t size);
int internalIsprotectedDB(uintptr_t addr, size_t size);
#endif
void* internalFind32bitBlock(size_t size);
void* internalFindBlockNearHint(void* hint, size_t size);
void* internalFind47bitBlock(size_t size);
void* internalFind47bitBlockNearHint(void* hint, size_t size);

void *parse_find_req(request_t *req) {
    switch (req->data.find.availablebits) {
    case AVAILABLE_BITS_32:
        return internalFind32bitBlock(req->data.find.size);
    case AVAILABLE_BITS_47:
        return internalFind47bitBlock(req->data.find.size);
    }
    __builtin_unreachable();
}
void *parse_find_near_req(request_t *req) {
    switch (req->data.find_near.availablebits) {
    case AVAILABLE_BITS_MAX:
        return internalFindBlockNearHint(req->data.find_near.hint, req->data.find_near.size);
    case AVAILABLE_BITS_47:
        return internalFind47bitBlockNearHint(req->data.find_near.hint, req->data.find_near.size);
    }
    __builtin_unreachable();
}

static void runMailbox() {
    sem_init(&empty, 0, 1);
    sem_init(&full, 0, 0);

    while (1) {
        sem_wait(&full);
#ifdef DEBUG_MAILBOX
        printf_log(LOG_DEBUG, "[ WORKER ] Received request %05u (%02X) (ans=%p)\n", curHdr.reqno, curHdr.type, curHdr.ansBuf);
#endif
        switch (curHdr.type) {
        case REQUEST_HALT:
            return;

        case REQUEST_UPD_PROTECT:
            internalUpdateProtection(curHdr.data.prot_protect.addr, curHdr.data.prot_protect.size, curHdr.data.prot_protect.prot);
            break;
        case REQUEST_SET_PROTECT:
            internalSetProtection(curHdr.data.prot_protect.addr, curHdr.data.prot_protect.size, curHdr.data.prot_protect.prot);
            break;
        case REQUEST_FRE_PROTECT:
            internalFreeProtection(curHdr.data.size_protect.addr, curHdr.data.size_protect.size);
            break;
        case REQUEST_GET_PROTECT:
            *(uintptr_t*)curHdr.ansBuf = internalGetProtection(curHdr.data.protect.addr);
            break;
        case REQUEST_LOD_PROTECT:
            internalLoadProtectionFromMap();
            break;
#ifdef DYNAREC
        case REQUEST_PROT_DB:
            internalProtectDB(curHdr.data.size_protect.addr, curHdr.data.size_protect.size);
            break;
        case REQUEST_UNPROT_DB:
            internalUnprotectDB(curHdr.data.size_protect.addr, curHdr.data.size_protect.size);
            break;
        case REQUEST_ISPROT_DB:
            *(intptr_t*)curHdr.ansBuf = internalIsprotectedDB(curHdr.data.size_protect.addr, curHdr.data.size_protect.size);
            break;
#endif
        case REQUEST_FIND:
            *(void**)curHdr.ansBuf = parse_find_req(&curHdr);
            break;
        case REQUEST_FIND_HINT:
            *(void**)curHdr.ansBuf = parse_find_near_req(&curHdr);
            break;
#ifdef DEBUG_MAILBOX
        default:
            printf_log(LOG_DEBUG, "[ WORKER ] AAAAAAA %X\n", curHdr.type);
#endif
        }
        if (curHdr.ansBuf) {
#ifdef DEBUG_MAILBOX
            printf_log(LOG_DEBUG, "[ WORKER ] Answering %X to REQ%05u\n", *(uintptr_t*)curHdr.ansBuf, curHdr.reqno);
#endif
            sem_post(curHdr.answered);
        }
        sem_post(&empty);
#ifdef DEBUG_MAILBOX
        printf_log(LOG_DEBUG, "[ WORKER ] Waiting for next request\n");
#endif
    }

    sem_destroy(&empty);
    sem_destroy(&full);
}

#include <pthread.h>
#include <signal.h>
pthread_t thread;
static void *mailboxRoutine(void *arg) {
    sigset_t sigset;
    sigfillset(&sigset);
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);
    runMailbox();
    return NULL;
}

// Declared in main.c and threads.c
void startMailbox(void) {
    pthread_create(&thread, NULL, mailboxRoutine, NULL);
}
void haltMailbox(void) {
    request_t hltreq;
    hltreq.type = REQUEST_HALT;
    sendRequest(&hltreq, NULL);
    pthread_join(thread, NULL);
}
