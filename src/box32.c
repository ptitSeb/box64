#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>

#include "debug.h"
#include "box32.h"
#include "custommem.h"
#include "converter32.h"

#include "khash.h"

KHASH_MAP_INIT_INT64(to, ulong_t);
KHASH_MAP_INIT_INT(from, uintptr_t);
KHASH_MAP_INIT_STR(strings, ptr_t);

static kh_from_t*   hash_from;
static kh_to_t*     hash_to;
#define HASH_MSK    0xff000000
#define HASH_VAL    0x00ffffff
static uint32_t     hash_cnt = 1;
static pthread_rwlock_t hash_lock = {0};
static int          hash_running = 0;
// locale
static kh_from_t*   locale_from;
static kh_to_t*     locale_to;
static kh_strings_t* const_strings;


void init_hash_helper() {
    hash_from = kh_init(from);
    hash_to = kh_init(to);
    locale_from = kh_init(from);
    locale_to = kh_init(to);
    const_strings = kh_init(strings);
    pthread_rwlock_init(&hash_lock, NULL);
    hash_running = 1;
}
void fini_hash_helper() {
    hash_running = 0;
    kh_destroy(from, hash_from);
    hash_from = NULL;
    kh_destroy(to, hash_to);
    hash_to = NULL;
    hash_cnt = 1;
    kh_destroy(from, locale_from);
    locale_from = NULL;
    kh_destroy(to, locale_to);
    locale_to = NULL;
    kh_destroy(strings, const_strings); //TODO: does not free memory correctly
    const_strings = NULL;
    pthread_rwlock_destroy(&hash_lock);
}

// Convert from hash key to original 64bits value
uintptr_t from_hash(ulong_t l) {
    // easy case first
    if((l&HASH_MSK)!=HASH_MSK) {
        return (uintptr_t)l;
    }
    if(l==0xffffffff) {
        return 0xffffffffffffffffll;
    }
    // get value from hash table second
    uintptr_t ret = 0;
    if(!hash_running) {
        //printf_log(LOG_INFO, "Warning, from_hash used but hash not running\n");
        return ret;
    }
    pthread_rwlock_rdlock(&hash_lock);
    khint_t k = kh_get(from, hash_from, l);
    if (k==kh_end(hash_from)) {
        ret = (uintptr_t)l;
    } else {
        ret = kh_value(hash_from, k);
    }
    pthread_rwlock_unlock(&hash_lock);
    return ret;
}
// same as from_hash
uintptr_t from_hash_d(ulong_t l) {
    return from_hash(l);
}

// Convert from 64bits to hash key, creating it if needed
ulong_t to_hash(uintptr_t p) {
    if((p<0x100000000LL) && ((p&HASH_MSK)!=HASH_MSK))
        return (ulong_t)p;
    if(p==0xffffffffffffffffll) {
        return 0xffffffff;
    }
    ulong_t ret = 0;
    if(!hash_running) {
        //printf_log(LOG_INFO, "Warning, to_hash used but hash not running\n");
        return ret;
    }
    khint_t k;
    pthread_rwlock_rdlock(&hash_lock);
    k = kh_get(to, hash_to, p);
    if(k==kh_end(hash_to)) {
        // create a new key, but need write lock!
        pthread_rwlock_unlock(&hash_lock);
        pthread_rwlock_wrlock(&hash_lock);
        ret = HASH_MSK | hash_cnt++;
        if(hash_cnt==HASH_VAL) hash_cnt = 1;
        int r;
        k = kh_put(to, hash_to, p, &r);
        kh_value(hash_to, k) = ret;
        k = kh_put(from, hash_from, ret, &r);
        kh_value(hash_from, k) = p;
    } else {
        ret = kh_value(hash_to, k);
    }
    pthread_rwlock_unlock(&hash_lock);
    return ret;
}

// Convert from 64bits to hash key and delete the entry from both hash table
ulong_t to_hash_d(uintptr_t p) {
    if((p<0x100000000LL) && ((p&HASH_MSK)!=HASH_MSK))
        return (ulong_t)p;
    if(p==0xffffffffffffffffll)
        return 0xffffffff;
    ulong_t ret = 0;
    if(!hash_running) {
        //printf_log(LOG_INFO, "Warning, to_hash_d used but hash not running\n");
        return ret;
    }
    khint_t k;
    pthread_rwlock_wrlock(&hash_lock);
    k = kh_get(to, hash_to, p);
    if(k==kh_end(hash_to)) {
        /// should this be an assert?
    } else {
        ret = kh_value(hash_to, k);
        // delete both entries
        //k = kh_get(to, hash_to, p);
        kh_del(to, hash_to, k);
        k = kh_get(from, hash_from, ret);
        kh_del(from, hash_from, k);
    }
    pthread_rwlock_unlock(&hash_lock);
    return ret;
}

typedef struct struct_locale_s {
	void* p0[13];
	void* p1;
	void* p2;
	void* p3;
	void* p4[13];
} struct_locale_t;
void from_struct_locale(struct_locale_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
    for(int i=0; i<13; ++i) {
	    dest->p0[i] = (void*)from_hash(*(ptr_t*)src); src += 4;
    }
	dest->p1 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->p2 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->p3 = from_ptrv(*(ptr_t*)src); src += 4;
    for(int i=0; i<13; ++i) {
	    dest->p4[i] = (void*)from_hash(*(ptr_t*)src); src += 4;
    }
}
void to_struct_locale(ptr_t d, const struct_locale_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
    for(int i=0; i<13; ++i) {
	    *(ptr_t*)dest = to_hashv(src->p0[i]); dest += 4;
    }
    // copy the 3 ctype int (1st is short int, but int will do)
    *(unsigned short int*)(d+(13+3+13)*sizeof(ptr_t)) = *(unsigned short int*)src->p1;
	*(ptr_t*)dest = d+(13+3+13)*sizeof(ptr_t); dest += 4;
    *(int*)(d+(13+3+13+1)*sizeof(ptr_t)) = *(int*)src->p2;
	*(ptr_t*)dest = d+(13+3+13+1)*sizeof(ptr_t); dest += 4;
    *(int*)(d+(13+3+13+3)*sizeof(ptr_t)) = *(int*)src->p3;
	*(ptr_t*)dest = d+(13+3+13+2)*sizeof(ptr_t); dest += 4;
    for(int i=0; i<13; ++i) {
	    *(ptr_t*)dest = to_hashv(src->p4[i]); dest += 4;
    }
}
void free_struct_locale(const struct_locale_t *src) {
    for(int i=0; i<13; ++i) {
	    to_hash_d((uintptr_t)src->p0[i]);
    }
    for(int i=0; i<13; ++i) {
	    to_hash_d((uintptr_t)src->p4[i]);
    }
}

// Convert from locale key to original 64bits value
void* from_locale(ptr_t l) {
    // easy case first
    if(l < 0x100) {
        return from_ptrv(l);
    }
    if(l == 0xffffffff) {
        return (void*)-1;
    }
    // get value from hash table second
    void* ret = 0;
    if(!hash_running) {
        //printf_log(LOG_INFO, "Warning, from_locale used but hash not running\n");
        return ret;
    }
    pthread_rwlock_rdlock(&hash_lock);
    khint_t k = kh_get(from, locale_from, l);
    if (k==kh_end(locale_from)) {
        ret = from_ptrv(l);
    } else {
        ret = (void*)kh_value(locale_from, k);
    }
    pthread_rwlock_unlock(&hash_lock);
    //from_struct_locale((struct_locale_t*)ret, l);
    return ret;
}
// same as from_locale
void* from_locale_d(ptr_t l) {
    return from_locale(l);
}

// Convert from 64bits to locale key, creating it if needed
ptr_t to_locale(void* p) {
    if((uintptr_t)p < 0x100) {
        return to_ptrv(p);
    }
    if(p == (void*)-1) {
        return 0xffffffff;
    }
    ptr_t ret = 0;
    if(!hash_running) {
        //printf_log(LOG_INFO, "Warning, to_locale used but hash not running\n");
        return ret;
    }
    khint_t k;
    pthread_rwlock_rdlock(&hash_lock);
    k = kh_get(to, locale_to, (uintptr_t)p);
    int conv = 0;
    if(k==kh_end(locale_to)) {
        // create a new key, but need write lock!
        pthread_rwlock_unlock(&hash_lock);
        pthread_rwlock_wrlock(&hash_lock);
        // a locale_t is 5 pointer!
        void* m = calloc(13+3+13+4, sizeof(ptr_t)); // the 3 ctype value are also inside the locale struct
        ret = to_ptrv(m);
        // add to hash maps
        int r;
        k = kh_put(to, locale_to, (uintptr_t)p, &r);
        kh_value(locale_to, k) = ret;
        k = kh_put(from, locale_from, ret, &r);
        kh_value(locale_from, k) = (uintptr_t)p;
        conv = 1;
    } else {
        ret = kh_value(locale_to, k);
    }
    pthread_rwlock_unlock(&hash_lock);
    if(conv)
        to_struct_locale(ret, (struct_locale_t*)p);
    return ret;
}

// Convert from 64bits to hash key and delete the entry from both hash table
ptr_t to_locale_d(void* p) {
    if((uintptr_t)p < 0x100)
        return to_ptrv(p);
    ptr_t ret = 0;
    if(!hash_running)
        return ret;
    khint_t k;
    pthread_rwlock_wrlock(&hash_lock);
    k = kh_get(to, locale_to, (uintptr_t)p);
    if(k==kh_end(locale_to)) {
        /// should this be an assert?
    } else {
        ret = kh_value(locale_to, k);
        // free the memory
        free_struct_locale(p);
        free(from_ptrv(ret));
        // delete both entries
        k = kh_get(to, locale_to, (uintptr_t)p);
        kh_del(to, locale_to, k);
        k = kh_get(from, locale_from, ret);
        kh_del(from, locale_from, k);
    }
    pthread_rwlock_unlock(&hash_lock);
    return ret;
}

char* from_cstring(ptr_t p) {
    return (char*)from_ptrv(p);
}

ptr_t to_cstring(char* p) {
    if((uintptr_t)p<0x100000000LL)
        return to_ptrv(p);
    ptr_t ret = 0;
    pthread_rwlock_rdlock(&hash_lock);
    khint_t k = kh_get(strings, const_strings, p);
    if(k==kh_end(const_strings)) {
        // create a new key, but need write lock!
        pthread_rwlock_unlock(&hash_lock);
        pthread_rwlock_wrlock(&hash_lock);
        ret = to_ptrv(box_strdup(p));
        int r;
        k = kh_put(strings, const_strings, (char*)from_ptrv(ret), &r);
        kh_value(const_strings, k) = ret;
    } else
        ret = kh_value(const_strings, k);
    pthread_rwlock_unlock(&hash_lock);
    return ret;
}

ptr_t to_cstring_d(char* p) {
    if((uintptr_t)p<0x100000000LL)
        return to_ptrv(p);
    ptr_t ret = 0;
    pthread_rwlock_wrlock(&hash_lock);
    khint_t k = kh_get(strings, const_strings, p);
    if(k==kh_end(const_strings)) {
        // assert?
    } else {
        ret = kh_value(const_strings, k);
        kh_del(strings, const_strings, k);
        free(from_ptrv(ret));
    }
    pthread_rwlock_unlock(&hash_lock);
    return ret;
}