// SPDX-License-Identifier: MIT
// A signal handler that edits the saved fp state through ucontext->uc_mcontext.fpregs must have those edits applied when it returns,
// the same way Linux rt_sigreturn does.

#define _GNU_SOURCE
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>

#define FXSAVE_XMM(n)   (0xa0 + 16 * (n))
#define FXSAVE_YMM(n)   (0x240 + 16 * (n))

static int mode;
static int failures;
static volatile int alarm_done;

static void handler(int sig, siginfo_t* si, void* uc_)
{
    (void)sig; (void)si;
    ucontext_t* uc = (ucontext_t*)uc_;
    unsigned char* fp = (unsigned char*)uc->uc_mcontext.fpregs;

    switch (mode) {
    case 0:     // edit XMM0
        memset(fp + FXSAVE_XMM(0), 0xAB, 16);
        break;
    case 1:     // edit the upper half of YMM0
        memset(fp + FXSAVE_YMM(0), 0xCD, 16);
        break;
    case 2:     // async case: edit XMM2 while spinning in a dynablock
        memset(fp + FXSAVE_XMM(2), 0xEF, 16);
        alarm_done = 1;
        break;
    }
}

static void check(const char* name, int ok)
{
    printf("%s: %s\n", name, ok ? "ok" : "FAIL");
    if (!ok) failures++;
}

int main(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = handler;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);

    unsigned char in[32] __attribute__((aligned(32)));
    for (int i = 0; i < 32; i++) in[i] = i + 1;
    __asm__ volatile("vmovdqu %0, %%ymm0" :: "m"(in) : "ymm0");
    __asm__ volatile("vmovdqu %0, %%ymm2" :: "m"(in) : "ymm2");

    unsigned char out[32] __attribute__((aligned(32)));

    mode = 0;
    raise(SIGUSR1);
    __asm__ volatile("vmovdqu %%ymm0, %0" : "=m"(out) :: "ymm0");
    check("xmm0 edit", out[0] == 0xAB && out[15] == 0xAB);

    mode = 1;
    raise(SIGUSR1);
    __asm__ volatile("vmovdqu %%ymm0, %0" : "=m"(out) :: "ymm0");
    check("ymm0 edit", out[16] == 0xCD && out[31] == 0xCD);

    // async signal delivered while the guest is spinning in a dynablock,
    // dynarec's live SIMD cache must be reloaded from the edited frame state.
    mode = 2;
    alarm_done = 0;
    ualarm(100000, 0);
    volatile unsigned long spin = 0;
    while (!alarm_done && spin < 0x100000000UL) spin++;
    ualarm(0, 0);
    __asm__ volatile("vmovdqu %%ymm2, %0" : "=m"(out) :: "ymm2");
    check("async xmm2 edit", out[0] == 0xEF && out[15] == 0xEF);

    printf("failures=%d\n", failures);
    return failures ? 1 : 0;
}
