#include "debug.h"
#include "x64run_private.h"
#include "box64cpu.h"
#include "box64cpu_util.h"
#include "elfloader.h"
#include "box32.h"

#ifdef ANDROID
void EXPORT my___libc_init(x64emu_t* emu, void* raw_args , void (*onexit)(void) , int (*main)(int, char**, char**), void const * const structors )
{
    //TODO: register fini
    // let's cheat and set all args...
    SetRDX(emu, (uintptr_t)my_context->envv);
    SetRSI(emu, (uintptr_t)my_context->argv);
    SetRDI(emu, (uintptr_t)my_context->argc);

    printf_log(LOG_DEBUG, "Transfert to main(%d, %p, %p)=>%p from __libc_init\n", my_context->argc, my_context->argv, my_context->envv, main);
    // should call structors->preinit_array and structors->init_array!
    // call main and finish
    Push64(emu, GetRBP(emu));   // set frame pointer
    SetRBP(emu, GetRSP(emu));   // save RSP
    SetRSP(emu, GetRSP(emu)&~0xFLL);    // Align RSP
    PushExit(emu);
    R_RIP=(uintptr_t)main;

    DynaRun(emu);

    SetRSP(emu, GetRBP(emu));   // restore RSP
    SetRBP(emu, Pop64(emu));    // restore RBP
    emu->quit = 1; // finished!
}
#else
EXPORT int32_t my___libc_start_main(x64emu_t* emu, int (*main) (int, char * *, char * *), int argc, char * * ubp_av, void (*init) (void), void (*fini) (void), void (*rtld_fini) (void), void (* stack_end))
{
    (void)argc; (void)ubp_av; (void)fini; (void)rtld_fini; (void)stack_end;

    if(init) {
        uintptr_t old_rsp = GetRSP(emu);
        uintptr_t old_rbp = GetRBP(emu); // should not be needed, but seems to be without dynarec
        Push64(emu, GetRBP(emu));   // set frame pointer
        SetRBP(emu, GetRSP(emu));   // save RSP
        SetRSP(emu, GetRSP(emu)&~0xFLL);    // Align RSP
        PushExit(emu);
        SetRDX(emu, (uint64_t)my_context->envv);
        SetRSI(emu, (uint64_t)my_context->argv);
        SetRDI(emu, (uint64_t)my_context->argc);
        R_RIP=(uint64_t)*init;
        printf_dump(LOG_DEBUG, "Calling init(%p) from __libc_start_main\n", *init);
        DynaRun(emu);
        if(emu->error)  // any error, don't bother with more
            return 0;
        SetRSP(emu, GetRBP(emu));   // restore RSP
        SetRBP(emu, Pop64(emu));    // restore RBP
        SetRSP(emu, old_rsp);
        SetRBP(emu, old_rbp);
        emu->quit = 0;
    } else {
        if(my_context->elfs[0]) {
            printf_dump(LOG_DEBUG, "Calling init from main elf\n");
            RunElfInit(my_context->elfs[0], emu);
        }
    }
    if(my_context->elfs[0]) {
        MarkElfInitDone(my_context->elfs[0]);
    }
    printf_log(LOG_DEBUG, "Transfert to main(%d, %p, %p)=>%p from __libc_start_main\n", my_context->argc, my_context->argv, my_context->envv, main);
    // call main and finish
    Push64(emu, GetRBP(emu));   // set frame pointer
    SetRBP(emu, GetRSP(emu));   // save RSP
    SetRSP(emu, GetRSP(emu)&~0xFLL);    // Align RSP
    PushExit(emu);
    SetRDX(emu, (uint64_t)my_context->envv);
    SetRSI(emu, (uint64_t)my_context->argv);
    SetRDI(emu, (uint64_t)my_context->argc);
    R_RIP=(uint64_t)main;

    DynaRun(emu);

    if(!emu->quit) {
        SetRSP(emu, GetRBP(emu));   // restore RSP
        SetRBP(emu, Pop64(emu));         // restore RBP
        emu->quit = 1;  // finished!
    }
    return (int)GetEAX(emu);
}
#ifdef BOX32
#ifdef ANDROID
void EXPORT my32___libc_init(x64emu_t* emu, void* raw_args , void (*onexit)(void) , int (*main)(int, char**, char**), void const * const structors )
{
    //TODO: register fini
    // let's cheat and set all args...
    Push_32(emu, (uint32_t)my_context->envv32);
    Push_32(emu, (uint32_t)my_context->argv32);
    Push_32(emu, (uint32_t)my_context->argc);

    printf_log(LOG_DEBUG, "Transfert to main(%d, %p, %p)=>%p from __libc_init\n", my_context->argc, my_context->argv, my_context->envv, main);
    // should call structors->preinit_array and structors->init_array!
    // call main and finish
    PushExit_32(emu);
    R_EIP=to_ptrv(main);

    DynaRun(emu);

    emu->quit = 1; // finished!
}
#else
int32_t EXPORT my32___libc_start_main(x64emu_t* emu, int *(main) (int, char * *, char * *), int argc, char * * ubp_av, void (*init) (void), void (*fini) (void), void (*rtld_fini) (void), void (* stack_end))
{
    // let's cheat and set all args...
    Push_32(emu, my_context->envv32);
    Push_32(emu, my_context->argv32);
    Push_32(emu, my_context->argc);
    if(init) {
        PushExit_32(emu);
        R_EIP=to_ptrv(*init);
        printf_log(LOG_DEBUG, "Calling init(%p) from __libc_start_main\n", *init);
        DynaRun(emu);
        if(emu->error)  // any error, don't bother with more
            return 0;
        emu->quit = 0;
    } else {
        if(my_context->elfs[0]) {
            printf_dump(LOG_DEBUG, "Calling init from main elf\n");
            RunElfInit(my_context->elfs[0], emu);
        }
    }
    if(my_context->elfs[0]) {
        MarkElfInitDone(my_context->elfs[0]);
    }
    printf_log(LOG_DEBUG, "Transfert to main(%d, %p, %p)=>%p from __libc_start_main\n", my_context->argc, my_context->argv, my_context->envv, main);
    // call main and finish
    PushExit_32(emu);
    R_EIP=to_ptrv(main);

    DynaRun(emu);

    emu->quit = 1; // finished!
    return 0;
}
#endif
#endif
#endif
