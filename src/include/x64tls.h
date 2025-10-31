#ifndef __X64_TLS_H__
#define __X64_TLS_H__

typedef struct thread_area_s thread_area_t;
typedef struct thread_area_32_s thread_area_32_t;

uint32_t my_set_thread_area_32(x64emu_t* emu, thread_area_32_t* td);
uint32_t my_modify_ldt(x64emu_t* emu, int op, thread_area_t* td, int size);

void refreshTLSData(x64emu_t* emu);

int my_arch_prctl(x64emu_t *emu, int code, void* addr);

#endif //__X64_TLS_H__