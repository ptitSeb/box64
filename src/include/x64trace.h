#ifndef __X64TRACE_H_
#define __X64TRACE_H_
#include <stdint.h>

typedef struct box64context_s box64context_t;
typedef struct zydis_dec_s zydis_dec_t;

int InitX64Trace(box64context_t *context);
void DeleteX64Trace(box64context_t *context);

zydis_dec_t* InitX86TraceDecoder(box64context_t *context);
void DeleteX86TraceDecoder(zydis_dec_t **dec);
zydis_dec_t* InitX64TraceDecoder(box64context_t *context);
void DeleteX64TraceDecoder(zydis_dec_t **dec);

const char* DecodeX64Trace(zydis_dec_t *dec, uintptr_t p);

#endif //__X64TRACE_H_