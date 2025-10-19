#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>

#include "debug.h"
#include "box32.h"
#include "custommem.h"
#include "converter32.h"

#include "khash.h"

KHASH_MAP_INIT_INT64(to, ulong_t);
KHASH_MAP_INIT_INT(from, uintptr_t);
KHASH_MAP_INIT_STR(strings, ptr_t);

static kh_from_t*   hash_from = NULL;
static kh_to_t*     hash_to = NULL;
#define HASH_MSK    0xf000000f
#define HASH_VAL    0x007fffff
#define HASH_SHIFT  4
static uint32_t     hash_cnt = 1;
static pthread_rwlock_t hash_lock = {0};
static int          hash_running = 0;
// locale
static kh_from_t*   locale_from = NULL;
static kh_to_t*     locale_to = NULL;
static kh_strings_t* const_strings = NULL;


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
    if(!hash_running)
        return;
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
    if((p>0x100000000LL) && ((p&HASH_MSK)!=HASH_MSK))
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
        ret = HASH_MSK | (((hash_cnt++)&HASH_VAL)<<HASH_SHIFT);
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
    if((p>0x100000000LL) && ((p&HASH_MSK)!=HASH_MSK))
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
	void* p0[13];   // struct __locale_data *
	void* p1;   //const unsigned short int *__ctype_b;
	void* p2;   //const int *__ctype_tolower;
	void* p3;   //const int *__ctype_toupper;
	void* p4[13];   //const char*
} struct_locale_t;
// not a real structure
#define LOCALE_SIGN 0x54abcd845412LL
typedef struct struct_locale_32_s {
    ptr_t p0[13];
    ptr_t p1;
    ptr_t p2;
    ptr_t p3;
    ptr_t p4[13];
    uint64_t sign;
    struct_locale_t* org;   // save the orginal locale
    // copy of the array, in case some apps use them
    unsigned short int type_b[384];
    int tolower[384];
    int tohigher[384];
} struct_locale_32_t;

void to_struct_locale(ptr_t d, const struct_locale_t *src) {
	if (!src) return;
	struct_locale_32_t* dest = from_ptrv(d);
    // save the original locale
    dest->sign = LOCALE_SIGN;
    dest->org = (void*)src;
    //copy the 2 arrays
    for(int i=0; i<13; ++i) {
	    dest->p0[i] = to_hashv(src->p0[i]);
    }
    for(int i=0; i<13; ++i) {
	    dest->p4[i] = to_cstring(src->p4[i]);
    }
    // copy the 3 ctype int (1st is short int, but int will do)
    memcpy(dest->type_b, src->p1-128*sizeof(short), 384*sizeof(short));
    memcpy(dest->tolower, src->p2-128*sizeof(int), 384*sizeof(int));
    memcpy(dest->tohigher, src->p3-128*sizeof(int), 384*sizeof(int));
    dest->p1 = to_ptrv(dest->type_b)+128*sizeof(short);
    dest->p2 = to_ptrv(dest->tolower)+128*sizeof(int);
    dest->p3 = to_ptrv(dest->tohigher)+128*sizeof(int);
}
void free_struct_locale(const struct_locale_t *src) {
    for(int i=0; i<13; ++i) {
	    to_hash_d((uintptr_t)src->p0[i]);
    }
    /*for(int i=0; i<13; ++i) {
	    to_hash_d((uintptr_t)src->p4[i]);
    }*/
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
    struct_locale_32_t* loc = from_ptrv(l);
    if(loc->sign==LOCALE_SIGN)
        return loc->org;
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
        void* m = calloc(1, sizeof(struct_locale_32_t)); // the 3 ctype value are also inside the locale struct
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
        ret = to_ptrv(box32_strdup(p));
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
        box32_free(from_ptrv(ret));
    }
    pthread_rwlock_unlock(&hash_lock);
    return ret;
}