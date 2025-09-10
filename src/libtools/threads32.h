#ifndef __THREADS_32_H_
#define __THREADS_32_H_
#include "box32.h"

typedef struct fake_pthread_mutext_s {
	int __lock;
	unsigned int __count;
  	int __owner;
	int i386__kind;
	int __kind;
	ptr_t real_mutex;
} fake_phtread_mutex_t;
#define KIND_SIGN	0xbad001

// longjmp / setjmp
typedef struct jump_buff_i386_s {
 uint32_t save_ebx;
 uint32_t save_esi;
 uint32_t save_edi;
 uint32_t save_ebp;
 uint32_t save_esp;
 uint32_t save_eip;
} jump_buff_i386_t;

// sigset_t should have the same size on 32bits and 64bits machine (64bits)
typedef struct __attribute__((packed, aligned(4))) __jmp_buf_tag_i386_s {
    jump_buff_i386_t __jmpbuf;
    int              __mask_was_saved;
    sigset_t         __saved_mask;
} __jmp_buf_tag_i386_t;

typedef struct i386_unwind_buff_s {
	struct {
		jump_buff_i386_t	__cancel_jmp_buf;	
		int					__mask_was_saved;
	} __cancel_jmp_buf[1];
	ptr_t __pad[2];
	ptr_t __pad3;
} __attribute__((packed, aligned(4))) i386_unwind_buff_t;

#endif //__THREADS_32_H_