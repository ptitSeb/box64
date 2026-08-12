#ifndef __DYNAREC_LA64_NATIVE_H__
#define __DYNAREC_LA64_NATIVE_H__

#include <stdint.h>

typedef struct dynarec_la64_s dynarec_la64_t;

typedef enum la64_native_call_e {
    LA64_NATIVE_NONE = 0,
    LA64_NATIVE_MEMCMP,
    LA64_NATIVE_MEMCPY,
    LA64_NATIVE_MEMMOVE,
    LA64_NATIVE_MEMSET,
    LA64_NATIVE_MEMCHR,
    LA64_NATIVE_STRCMP,
    LA64_NATIVE_STRLEN,
    LA64_NATIVE_LAST,
} la64_native_call_t;

la64_native_call_t la64_get_native_call(uintptr_t addr);
int la64_native_call_writes_memory(la64_native_call_t call);
void* create_native_call(la64_native_call_t call);

#ifdef STEP
#define la64_emit_native_call STEPNAME(la64_emit_native_call)
void la64_emit_native_call(dynarec_la64_t* dyn, int ninst, la64_native_call_t call);
#define la64_native_call_pass STEPNAME(la64_native_call_pass)
void la64_native_call_pass(dynarec_la64_t* dyn, la64_native_call_t call);
#endif

#endif
