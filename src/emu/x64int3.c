#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>

#include "debug.h"
#include "box64stack.h"
#include "x64emu.h"
#include "x64run.h"
#include "x64emu_private.h"
#include "x64run_private.h"
#include "x87emu_private.h"
#include "x64primop.h"
#include "x64trace.h"
#include "wrapper.h"
#include "box64context.h"
#include "librarian.h"

#include <elf.h>
#include "elfloader.h"
#include "elfs/elfloader_private.h"

typedef int32_t (*iFpppp_t)(void*, void*, void*, void*);

x64emu_t* x64emu_fork(x64emu_t* emu, int forktype)
{
    // execute atforks prepare functions, in reverse order
    for (int i=my_context->atfork_sz-1; i>=0; --i)
        if(my_context->atforks[i].prepare)
            EmuCall(emu, my_context->atforks[i].prepare);
    int type = emu->type;
    int v;
    if(forktype==2) {
        iFpppp_t forkpty = (iFpppp_t)emu->forkpty_info->f;
        v = forkpty(emu->forkpty_info->amaster, emu->forkpty_info->name, emu->forkpty_info->termp, emu->forkpty_info->winp);
        emu->forkpty_info = NULL;
    } else
        v = fork();
    if(type == EMUTYPE_MAIN)
        thread_set_emu(emu);
    if(v==EAGAIN || v==ENOMEM) {
        // error...
    } else if(v!=0) {  
        // execute atforks parent functions
        for (int i=0; i<my_context->atfork_sz; --i)
            if(my_context->atforks[i].parent)
                EmuCall(emu, my_context->atforks[i].parent);

    } else if(v==0) {
        // execute atforks child functions
        for (int i=0; i<my_context->atfork_sz; --i)
            if(my_context->atforks[i].child)
                EmuCall(emu, my_context->atforks[i].child);
    }
    R_EAX = v;
    return emu;
}

extern int errno;
void x64Int3(x64emu_t* emu)
{
    if(Peek(emu, 0)=='S' && Peek(emu, 1)=='C') // Signature for "Out of x86 door"
    {
        R_RIP += 2;
        uintptr_t addr = Fetch64(emu);
        if(addr==0) {
            //printf_log(LOG_INFO, "%p:Exit x86 emu (emu=%p)\n", *(void**)(R_ESP), emu);
            emu->quit=1; // normal quit
        } else {
            RESET_FLAGS(emu);
            wrapper_t w = (wrapper_t)addr;
            addr = Fetch64(emu);
            /* This party can be used to trace only 1 specific lib (but it is quite slow)
            elfheader_t *h = FindElfAddress(my_context, *(uintptr_t*)(R_ESP));
            int have_trace = 0;
            if(h && strstr(ElfName(h), "libMiles")) have_trace = 1;*/
            if(box64_log>=LOG_DEBUG /*|| have_trace*/) {
                pthread_mutex_lock(&emu->context->mutex_trace);
                int tid = GetTID();
                char buff[256] = "\0";
                char buff2[64] = "\0";
                char buff3[64] = "\0";
                char *tmp;
                int post = 0;
                int perr = 0;
                uint32_t *pu32 = NULL;
                const char *s = NULL;
                s = GetNativeName((void*)addr);
                if(addr==(uintptr_t)PltResolver) {
                    snprintf(buff, 256, "%s", " ... ");
                } else  if(strstr(s, "__open")==s || !strcmp(s, "open") || !strcmp(s, "open ")) {
                    tmp = (char*)(R_RDI);
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", %d (,%d))", tid, *(void**)(R_RSP), s, (tmp)?tmp:"(nil)", (int)(R_ESI), (int)(R_EDX));
                    perr = 1;
                } else  if(strstr(s, "my___printf_chk")) {
                    tmp = (char*)(R_RSI);
                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, \"%s\" (,%p))", tid, *(void**)(R_RSP), s, R_EDI, (tmp)?tmp:"(nil)", (void*)(R_RDX));
                } else {
                    snprintf(buff, 255, "%04d|%p: Calling %s (0x%lX, 0x%lX, 0x%lX, ...)", tid, *(void**)(R_RSP), s, R_RDI, R_RSI, R_RDX);
                }
                printf_log(LOG_NONE, "%s =>", buff);
                pthread_mutex_unlock(&emu->context->mutex_trace);
                w(emu, addr);   // some function never come back, so unlock the mutex first!
                pthread_mutex_lock(&emu->context->mutex_trace);
                if(post)
                    switch(post) {
                    case 1: snprintf(buff2, 63, " [%d sec %d nsec]", pu32?pu32[0]:-1, pu32?pu32[1]:-1);
                            break;
                    case 2: snprintf(buff2, 63, "(%s)", R_RAX?((char*)R_RAX):"nil");
                            break;
                    case 3: snprintf(buff2, 63, "(%s)", pu32?((char*)pu32):"nil");
                            break;
                    case 4: snprintf(buff2, 63, " (%f)", ST0.d);
                            break;
                    case 5: {
                            uint32_t* p = (uint32_t*)R_RAX;
                            if(p)
                                snprintf(buff2, 63, " size=%dx%d, pitch=%d, pixels=%p", p[2], p[3], p[4], p+5);
                            else
                                snprintf(buff2, 63, "NULL Surface");
                            }
                            break;
                }
                if(perr==1 && ((int)R_EAX)<0)
                    snprintf(buff3, 63, " (errno=%d:\"%s\")", errno, strerror(errno));
                else if(perr==2 && R_EAX==0)
                    snprintf(buff3, 63, " (errno=%d:\"%s\")", errno, strerror(errno));
                printf_log(LOG_NONE, " return 0x%lX%s%s\n", R_RAX, buff2, buff3);
                pthread_mutex_unlock(&emu->context->mutex_trace);
            } else
                w(emu, addr);
        }
        return;
    }
    if(my_context->signals[SIGTRAP])
        raise(SIGTRAP);
    else
        printf_log(LOG_INFO, "%04d|Warning, ignoring unsupported Int 3 call @%p\n", GetTID(), (void*)R_RIP);
    //emu->quit = 1;
}

int GetTID()
{
    return syscall(SYS_gettid);
}