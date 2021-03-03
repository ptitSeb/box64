#ifndef __X64_TLS_H__
#define __X64_TLS_H__

typedef struct thread_area_s thread_area_t;

uint32_t my_set_thread_area(thread_area_t* td);
uint32_t my_modify_ldt(x64emu_t* emu, int op, thread_area_t* td, int size);

void* fillTLSData(box64context_t *context);
void* resizeTLSData(box64context_t *context, void* oldptr);
void* GetSegmentBase(uint32_t desc);

#endif //__X64_TLS_H__