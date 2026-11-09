# SPDX-License-Identifier: MIT
#
# Guest-side x86_64 compatibility shim for WeMeet.
#
# WeMeet's custom coroutine expects RDI/RSI to survive a call to pthread_mutex_unlock.
# The x86_64 glibc implementation happens to leave them alone, but box64 calls the host implementation,
# whose return value is written to $a0/$a1, which are mapped to guest RDI/RSI.
# Preserve the two guest registers around the real call.
#
# Build:
#   x86_64-linux-gnu-gcc -shared -nostdlib -fPIC -Wl,-soname,libwemeet_unlock_shim.so -o libwemeet_unlock_shim.so wemeet_unlock_shim.S
#
# The [wemeetapp] profile in system/box64.box64rc loads it through BOX64_LD_PRELOAD.

    .section .text
    .globl pthread_mutex_unlock
    .type pthread_mutex_unlock,@function
pthread_mutex_unlock:
    pushq %rdi
    pushq %rsi
    subq  $8, %rsp
    movq  real_unlock(%rip), %rax
    testq %rax, %rax
    jz    1f
    call  *%rax
    addq  $8, %rsp
    popq  %rsi
    popq  %rdi
    ret
1:
    leaq  name(%rip), %rsi
    movq  $-1, %rdi               # RTLD_NEXT
    call  dlsym@PLT
    movq  %rax, real_unlock(%rip)
    testq %rax, %rax
    jz    2f
    movq  16(%rsp), %rdi          # restore the mutex argument
    call  *%rax
    addq  $8, %rsp
    popq  %rsi
    popq  %rdi
    ret
2:
    addq  $8, %rsp
    popq  %rsi
    popq  %rdi
    xorl  %eax, %eax
    ret
    .size pthread_mutex_unlock,.-pthread_mutex_unlock

    .globl init_unlock_shim
    .type init_unlock_shim,@function
init_unlock_shim:
    pushq %rbp
    movq  %rsp, %rbp
    leaq  name(%rip), %rsi
    movq  $-1, %rdi               # RTLD_NEXT
    call  dlsym@PLT
    movq  %rax, real_unlock(%rip)
    popq  %rbp
    ret
    .size init_unlock_shim,.-init_unlock_shim

    .section .init_array,"aw"
    .align 8
    .quad init_unlock_shim

    .section .rodata
name:
    .asciz "pthread_mutex_unlock"

    .section .bss
    .align 8
real_unlock:
    .zero 8
    .section .note.GNU-stack,"",@progbits
