#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>

#include "box64stack.h"
#include "box64context.h"
#include "elfloader.h"
#include "elfs/elfloader_private.h"
#include "debug.h"
#include "emu/x64emu_private.h"
#include "auxval.h"
#include "custommem.h"
#include "box32.h"

static void PushString32(x64emu_t *emu, const char* s)
{
    int sz = strlen(s) + 1;
    // round to 4 bytes boundary
    R_ESP -= sz;
    memcpy(from_ptrv(R_ESP), s, sz);
}

static void Push32_32(x64emu_t *emu, uint32_t v)
{
    R_ESP -= 4;
    *((uint32_t*)from_ptr(R_ESP)) = v;
}


EXPORTDYN
void SetupInitialStack32(x64emu_t *emu)
{
    // start with 0
    Push32_32(emu, 0);
    // push program executed
    PushString32(emu, emu->context->argv[0]);
    uintptr_t p_arg0 = from_ptr(R_ESP);
    // push envs
    uintptr_t p_envv[emu->context->envc+1];
    for (int i=emu->context->envc-1; i>=0; --i) {
        PushString32(emu, emu->context->envv[i]);
        p_envv[i] = from_ptr(R_ESP);
    }
    p_envv[emu->context->envc] = 0;
    // push args, also, free the argv[] string and point to the one in the main stack
    uintptr_t p_argv[emu->context->argc];
    for (int i=emu->context->argc-1; i>=0; --i) {
        PushString32(emu, emu->context->argv[i]);
        p_argv[i] = R_ESP;
        box_free(emu->context->argv[i]);
        emu->context->argv[i] = (char*)p_argv[i];
    }
    // align
    uintptr_t tmp = from_ptr(R_ESP)&~(emu->context->stackalign-1);
    memset((void*)tmp, 0, from_ptr(R_ESP)-tmp);
    R_ESP=to_ptr(tmp);

    // push some AuxVector stuffs
    PushString32(emu, "i686");
    uintptr_t p_i686 = from_ptr(R_ESP);
    uintptr_t p_random = real_getauxval(25);
    if(!p_random) {
        for (int i=0; i<4; ++i)
            Push32_32(emu, random());
        p_random = from_ptr(R_ESP);
    }
    // align
    tmp = (R_ESP)&~(emu->context->stackalign-1);
    memset((void*)tmp, 0, from_ptr(R_ESP)-tmp);
    R_ESP=tmp;

    // push the AuxVector themselves
    /*
    00: 00000000
    03: 08048034
    04: 00000020
    05: 0000000b
    06: 00001000
    07: f7fc0000
    08: 00000000
    09: 08049060
    11: 000003e8
    12: 000003e8
    13: 000003e8
    14: 000003e8
    15: ffd8aa5b/i686
    16: bfebfbff
    17: 00000064
    23: 00000000
    25: ffd8aa4b
    26: 00000000
    31: ffd8bfeb/./testAuxVec
    32: f7fbfb40
    33: f7fbf000
    */
   elfheader_t* main = my_context->elfs[0];
    Push32_32(emu, 0); Push32_32(emu, 0);                            //AT_NULL(0)=0
    Push32_32(emu, main->fileno); Push32_32(emu, 2);   //AT_EXECFD=file desciptor of program
    Push32_32(emu, (uintptr_t)main->PHEntries._32); Push32_32(emu, 3);  //AT_PHDR(3)=address of the PH of the executable
    Push32_32(emu, sizeof(Elf32_Phdr)); Push32_32(emu, 4);              //AT_PHENT(4)=size of PH entry
    Push32_32(emu, main->numPHEntries); Push32_32(emu, 5);              //AT_PHNUM(5)=number of elf headers
    Push32_32(emu, box64_pagesize); Push32_32(emu, 6);                  //AT_PAGESZ(6)
    //Push32_32(emu, real_getauxval(7)); Push32_32(emu, 7);             //AT_BASE(7)=ld-2.27.so start (in memory)
    Push32_32(emu, 0); Push32_32(emu, 8);                               //AT_FLAGS(8)=0
    Push32_32(emu, R_EIP); Push32_32(emu, 9);                           //AT_ENTRY(9)=entrypoint
    Push32_32(emu, from_ulong(real_getauxval(11))); Push32_32(emu, 11); //AT_UID(11)
    Push32_32(emu, from_ulong(real_getauxval(12))); Push32_32(emu, 12); //AT_EUID(12)
    Push32_32(emu, from_ulong(real_getauxval(13))); Push32_32(emu, 13); //AT_GID(13)
    Push32_32(emu, from_ulong(real_getauxval(14))); Push32_32(emu, 14); //AT_EGID(14)
    Push32_32(emu, p_i686); Push32_32(emu, 15);                         //AT_PLATFORM(15)=&"i686"
    // Push HWCAP:
    //  FPU: 1<<0 ; VME: 1<<1 ; DE : 1<<2 ; PSE: 1<<3 ; TSC: 1<<4 ; MSR: 1<<5 ; PAE: 1<<6 ; MCE: 1<<7
    //  CX8: 1<<8 ; APIC:1<<9 ;             SEP: 1<<11; MTRR:1<<12; PGE: 1<<13; MCA: 1<<14; CMOV:1<<15
    // FCMOV:1<<16;                                                                         MMX: 1<<23
    // OSFXR:1<<24; XMM: 1<<25;XMM2: 1<<26;                                                AMD3D:1<<31
    Push32_32(emu, (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<8)  | (1<<15) | (1<<16) | (1<<23) | (1<<25) | (1<<26));
    Push32_32(emu, 16);                                         //AT_HWCAP(16)=...
    //Push32_32(emu, sysconf(_SC_CLK_TCK)); Push32_32(emu, 17);        //AT_CLKTCK(17)=times() frequency
    Push32_32(emu, from_ulong(real_getauxval(23))); Push32_32(emu, 23);          //AT_SECURE(23)
    Push32_32(emu, p_random); Push32_32(emu, 25);                    //AT_RANDOM(25)=p_random
    Push32_32(emu, 0); Push32_32(emu, 26);                           //AT_HWCAP2(26)=0
    Push32_32(emu, p_arg0); Push32_32(emu, 31);                      //AT_EXECFN(31)=p_arg0
    Push32_32(emu, emu->context->vsyscall); Push32_32(emu, 32); //AT_SYSINFO(32)=vsyscall
    //Push32_32(emu, ); Push32_32(emu, 33);                            //AT_SYSINFO_EHDR(33)=address of vDSO
    if(!emu->context->auxval_start) // store auxval start if needed
        emu->context->auxval_start = (uintptr_t*)from_ptr(R_ESP);

    // push nil / envs / nil / args / argc
    Push32_32(emu, 0);
    for (int i=emu->context->envc-1; i>=0; --i)
        Push32_32(emu, to_ptr(p_envv[i]));
    emu->context->envv32 = R_ESP;
    Push32_32(emu, 0);
    for (int i=emu->context->argc-1; i>=0; --i)
        Push32_32(emu, to_ptr(p_argv[i]));
    emu->context->argv32 = R_ESP;
    Push32_32(emu, emu->context->argc);
}
