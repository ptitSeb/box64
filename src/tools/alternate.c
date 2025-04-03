#include "alternate.h"
#include "custommem.h"
#include "khash.h"

// Alternate address handling
KHASH_MAP_INIT_INT64(alternate, void*)
static kh_alternate_t *my_alternates = NULL;

int hasAlternate(void* addr) {
    if(!my_alternates)
        return 0;
    khint_t k = kh_get(alternate, my_alternates, (uintptr_t)addr);
    if(k==kh_end(my_alternates))
        return 0;
    return 1;
}

void* getAlternate(void* addr) {
    if(!my_alternates)
        return addr;
    khint_t k = kh_get(alternate, my_alternates, (uintptr_t)addr);
    if(k!=kh_end(my_alternates))
        return kh_value(my_alternates, k);
    return addr;
}
void addAlternate(void* addr, void* alt) {
    if(!my_alternates) {
        my_alternates = kh_init(alternate);
    }
    int ret;
    khint_t k = kh_put(alternate, my_alternates, (uintptr_t)addr, &ret);
    if(!ret)    // already there
        return;
    kh_value(my_alternates, k) = alt;
}

void addCheckAlternate(void* addr, void* alt) {
    if(!hasAlternate(addr))
        addAlternate(addr, alt);
}

void cleanAlternate() {
    if(my_alternates) {
        kh_destroy(alternate, my_alternates);
        my_alternates = NULL;
    }
}
