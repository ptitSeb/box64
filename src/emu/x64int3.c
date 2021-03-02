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
//#include "x64run.h"
#include "x64emu_private.h"
#include "x64run_private.h"
#include "x87emu_private.h"
//#include "x64primop.h"
//#include "x64trace.h"
//#include "wrapper.h"
#include "box64context.h"
//#include "librarian.h"

#include <elf.h>
#include "elfloader.h"
#include "elfs/elfloader_private.h"

typedef int32_t (*iFpppp_t)(void*, void*, void*, void*);

//x64emu_t* x64emu_fork(x64emu_t* emu, int forktype)
//{
//    // execute atforks prepare functions, in reverse order
//    for (int i=my_context->atfork_sz-1; i>=0; --i)
//        if(my_context->atforks[i].prepare)
//            EmuCall(emu, my_context->atforks[i].prepare);
//    int type = emu->type;
//    int v;
//    if(forktype==2) {
//        iFpppp_t forkpty = (iFpppp_t)emu->forkpty_info->f;
//        v = forkpty(emu->forkpty_info->amaster, emu->forkpty_info->name, emu->forkpty_info->termp, emu->forkpty_info->winp);
//        emu->forkpty_info = NULL;
//    } else
//        v = fork();
//    if(type == EMUTYPE_MAIN)
//        thread_set_emu(emu);
//    if(v==EAGAIN || v==ENOMEM) {
//        // error...
//    } else if(v!=0) {  
//        // execute atforks parent functions
//        for (int i=0; i<my_context->atfork_sz; --i)
//            if(my_context->atforks[i].parent)
//                EmuCall(emu, my_context->atforks[i].parent);
//
//    } else if(v==0) {
//        // execute atforks child functions
//        for (int i=0; i<my_context->atfork_sz; --i)
//            if(my_context->atforks[i].child)
//                EmuCall(emu, my_context->atforks[i].child);
//    }
//    R_EAX = v;
//    return emu;
//}

extern int errno;
//void x64Int3(x64emu_t* emu)
//{
//    if(Peek(emu, 0)=='S' && Peek(emu, 1)=='C') // Signature for "Out of x86 door"
//    {
//        R_EIP += 2;
//        #ifdef RK3399
//        volatile    // to avoid addr to be put in an VFPU register
//        #endif
//        uint32_t addr = Fetch32(emu);
//        if(addr==0) {
//            //printf_log(LOG_INFO, "%p:Exit x86 emu (emu=%p)\n", *(void**)(R_ESP), emu);
//            emu->quit=1; // normal quit
//        } else {
//            RESET_FLAGS(emu);
//            wrapper_t w = (wrapper_t)addr;
//            addr = Fetch32(emu);
//            /* This party can be used to trace only 1 specific lib (but it is quite slow)
//            elfheader_t *h = FindElfAddress(my_context, *(uintptr_t*)(R_ESP));
//            int have_trace = 0;
//            if(h && strstr(ElfName(h), "libMiles")) have_trace = 1;*/
//            if(box86_log>=LOG_DEBUG /*|| have_trace*/) {
//                pthread_mutex_lock(&emu->context->mutex_trace);
//                int tid = GetTID();
//                char buff[256] = "\0";
//                char buff2[64] = "\0";
//                char buff3[64] = "\0";
//                char *tmp;
//                int post = 0;
//                int perr = 0;
//                uint32_t *pu32 = NULL;
//                const char *s = NULL;
//                {
//                    Dl_info info;
//                    if(dladdr((void*)addr, &info))
//                        s = info.dli_sname;
//                }
//                if(!s) s = GetNativeName((void*)addr);
//                if(addr==(uintptr_t)PltResolver) {
//                    snprintf(buff, 256, "%s", " ... ");
//                } else
//                if(strstr(s, "SDL_RWFromFile")==s || strstr(s, "SDL_RWFromFile")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%s, %s)", tid, *(void**)(R_ESP), s, *(char**)(R_ESP+4), *(char**)(R_ESP+8));
//                } else  if(strstr(s, "glColor4f")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%f, %f, %f, %f)", tid, *(void**)(R_ESP), s, *(float*)(R_ESP+4), *(float*)(R_ESP+8), *(float*)(R_ESP+12), *(float*)(R_ESP+16));
//                } else  if(strstr(s, "glTexCoord2f")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%f, %f)", tid, *(void**)(R_ESP), s, *(float*)(R_ESP+4), *(float*)(R_ESP+8));
//                } else  if(strstr(s, "glVertex2f")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%f, %f)", tid, *(void**)(R_ESP), s, *(float*)(R_ESP+4), *(float*)(R_ESP+8));
//                } else  if(strstr(s, "glVertex3f")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%f, %f, %f)", tid, *(void**)(R_ESP), s, *(float*)(R_ESP+4), *(float*)(R_ESP+8), *(float*)(R_ESP+12));
//                } else  if(strstr(s, "__open64")==s || strcmp(s, "open64")==0) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", %d, %d)", tid, *(void**)(R_ESP), s, *(char**)(R_ESP+4), *(int*)(R_ESP+8), *(int*)(R_ESP+12));
//                    perr = 1;
//                } else  if(!strcmp(s, "opendir")) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\")", tid, *(void**)(R_ESP), s, *(char**)(R_ESP+4));
//                    perr = 1;
//                } else  if(strstr(s, "__open")==s || strcmp(s, "open")==0) {
//                    tmp = *(char**)(R_ESP+4);
//                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", %d (,%d))", tid, *(void**)(R_ESP), s, (tmp)?tmp:"(nil)", *(int*)(R_ESP+8), *(int*)(R_ESP+12));
//                    perr = 1;
//                } else  if(strcmp(s, "mkdir")==0) {
//                    tmp = *(char**)(R_ESP+4);
//                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", %d)", tid, *(void**)(R_ESP), s, (tmp)?tmp:"(nil)", *(int*)(R_ESP+8));
//                    perr = 1;
//                } else  if(!strcmp(s, "fopen")) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\")", tid, *(void**)(R_ESP), s, *(char**)(R_ESP+4), *(char**)(R_ESP+8));
//                    perr = 1;
//                } else  if(!strcmp(s, "freopen")) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\", %p)", tid, *(void**)(R_ESP), s, *(char**)(R_ESP+4), *(char**)(R_ESP+8), *(void**)(R_ESP+12));
//                    perr = 1;
//                } else  if(!strcmp(s, "fopen64")) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\")", tid, *(void**)(R_ESP), s, *(char**)(R_ESP+4), *(char**)(R_ESP+8));
//                    perr = 2;
//                } else  if(!strcmp(s, "chdir")) {
//                    pu32=*(uint32_t**)(R_ESP+4);
//                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\")", tid, *(void**)(R_ESP), s, pu32?((pu32==(uint32_t*)1)?"/1/":(char*)pu32):"/0/");
//                } else  if(strstr(s, "getenv")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\")", tid, *(void**)(R_ESP), s, *(char**)(R_ESP+4));
//                    post = 2;
//                } else  if(strstr(s, "pread")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p, %u, %d)", tid, *(void**)(R_ESP), s, *(int32_t*)(R_ESP+4), *(void**)(R_ESP+8), *(uint32_t*)(R_ESP+12), *(int32_t*)(R_ESP+16));
//                    perr = 1;
//                } else  if(strstr(s, "ioctl")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, 0x%x, %p)", tid, *(void**)(R_ESP), s, *(int32_t*)(R_ESP+4), *(int32_t*)(R_ESP+8), *(void**)(R_ESP+12));
//                    perr = 1;
//                } else  if(strstr(s, "statvfs64")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%p(\"%s\"), %p)", tid, *(void**)(R_ESP), s, *(void**)(R_ESP+4), *(char**)(R_ESP+4), *(void**)(R_ESP+8));
//                } else  if(strstr(s, "index")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%p(\"%s\"), %i(%c))", tid, *(void**)(R_ESP), s, *(char**)(R_ESP+4), *(char**)(R_ESP+4), *(int32_t*)(R_ESP+8), *(int32_t*)(R_ESP+8));
//                } else  if(strstr(s, "rindex")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%p(\"%s\"), %i(%c))", tid, *(void**)(R_ESP), s, *(char**)(R_ESP+4), *(char**)(R_ESP+4), *(int32_t*)(R_ESP+8), *(int32_t*)(R_ESP+8));
//                } else  if(strstr(s, "my___xstat64")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p(\"%s\"), %p)", tid, *(void**)(R_ESP), s, *(int32_t*)(R_ESP+4), *(char**)(R_ESP+8), *(char**)(R_ESP+8), *(void**)(R_ESP+12));
//                    perr = 1;
//                } else  if(strcmp(s, "my___xstat")==0) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p(\"%s\"), %p)", tid, *(void**)(R_ESP), s, *(int32_t*)(R_ESP+4), *(char**)(R_ESP+8), *(char**)(R_ESP+8), *(void**)(R_ESP+12));
//                    perr = 1;
//                } else  if(strstr(s, "my___lxstat64")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p(\"%s\"), %p)", tid, *(void**)(R_ESP), s, *(int32_t*)(R_ESP+4), *(char**)(R_ESP+8), *(char**)(R_ESP+8), *(void**)(R_ESP+12));
//                    perr = 1;
//                } else  if(strstr(s, "sem_timedwait")==s) {
//                    pu32 = *(uint32_t**)(R_ESP+8);
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, %p[%d sec %d ns])", tid, *(void**)(R_ESP), s, *(void**)(R_ESP+4), *(void**)(R_ESP+8), pu32?pu32[0]:-1, pu32?pu32[1]:-1);
//                    perr = 1;
//                } else  if(strstr(s, "waitpid")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p, 0x%x)", tid, *(void**)(R_ESP), s, *(int32_t*)(R_ESP+4), *(void**)(R_ESP+8), *(uint32_t*)(R_ESP+12));
//                    perr = 1;
//                } else  if(strstr(s, "clock_gettime")==s || strstr(s, "__clock_gettime")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p)", tid, *(void**)(R_ESP), s, *(uint32_t*)(R_ESP+4), *(void**)(R_ESP+8));
//                    post = 1;
//                    pu32 = *(uint32_t**)(R_ESP+8);
//                } else  if(strstr(s, "semop")==s) {
//                    int16_t* p16 = *(int16_t**)(R_ESP+8);
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p[%u/%d/%d], %d)", tid, *(void**)(R_ESP), s, *(int*)(R_ESP+4), p16, p16[0], p16[1], p16[2], *(int*)(R_ESP+12));
//                    perr = 1;
//                } else  if(!strcmp(s, "my_mmap64")) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, 0x%x, %d, 0x%x, %d, %lld)", tid, *(void**)(R_ESP), s, *(void**)(R_ESP+4), *(size_t*)(R_ESP+8), *(int*)(R_ESP+12), *(int*)(R_ESP+16), *(int*)(R_ESP+20), *(int64_t*)(R_ESP+24));
//                    perr = 1;
//                } else  if(!strcmp(s, "my_mmap")) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, 0x%x, %d, 0x%x, %d, %d)", tid, *(void**)(R_ESP), s, *(void**)(R_ESP+4), *(size_t*)(R_ESP+8), *(int*)(R_ESP+12), *(int*)(R_ESP+16), *(int*)(R_ESP+20), *(int*)(R_ESP+24));
//                    perr = 1;
//                } else  if(strstr(s, "strcasecmp")==s || strstr(s, "__strcasecmp")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\")", tid, *(void**)(R_ESP), s, *(char**)(R_ESP+4), *(char**)(R_ESP+8));
//                } else  if(strstr(s, "gtk_signal_connect_full")) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, \"%s\", %p, %p, %p, %p, %d, %d)", tid, *(void**)(R_ESP), "gtk_signal_connect_full", *(void**)(R_ESP+4), *(char**)(R_ESP+8), *(void**)(R_ESP+12), *(void**)(R_ESP+16), *(void**)(R_ESP+20), *(void**)(R_ESP+24), *(int32_t*)(R_ESP+28), *(int32_t*)(R_ESP+32));
//                } else  if(strstr(s, "strcmp")==s || strstr(s, "__strcmp")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\")", tid, *(void**)(R_ESP), s, *(char**)(R_ESP+4), *(char**)(R_ESP+8));
//                } else  if(strstr(s, "strstr")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%.127s\", \"%.127s\")", tid, *(void**)(R_ESP), s, *(char**)(R_ESP+4), *(char**)(R_ESP+8));
//                } else  if(strstr(s, "strlen")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%p(\"%s\"))", tid, *(void**)(R_ESP), s, *(char**)(R_ESP+4), ((R_ESP+4))?(*(char**)(R_ESP+4)):"nil");
//                } else  if(strstr(s, "my_vsnprintf")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%08X, %u, %08X...)", tid, *(void**)(R_ESP), s, *(uint32_t*)(R_ESP+4), *(uint32_t*)(R_ESP+8), *(uint32_t*)(R_ESP+12));
//                    pu32 = *(uint32_t**)(R_ESP+4);
//                    post = 3;
//                } else  if(strstr(s, "my_vsprintf")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%08X, \"%s\"...)", tid, *(void**)(R_ESP), s, *(uint32_t*)(R_ESP+4), *(char**)(R_ESP+8));
//                    pu32 = *(uint32_t**)(R_ESP+4);
//                    post = 3;
//                } else  if(strstr(s, "my_snprintf")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%08X, %u, %08X...)", tid, *(void**)(R_ESP), s, *(uint32_t*)(R_ESP+4), *(uint32_t*)(R_ESP+8), *(uint32_t*)(R_ESP+12));
//                    pu32 = *(uint32_t**)(R_ESP+4);
//                    post = 3;
//                } else  if(strstr(s, "my_sprintf")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%08X, %08X...)", tid, *(void**)(R_ESP), s, *(uint32_t*)(R_ESP+4), *(uint32_t*)(R_ESP+8));
//                    pu32 = *(uint32_t**)(R_ESP+4);
//                    post = 3;
//                } else  if(strstr(s, "my_printf")==s) {
//                    pu32 = *(uint32_t**)(R_ESP+4);
//                    if(((uintptr_t)pu32)<0x5) // probably a _chk function
//                        pu32 = *(uint32_t**)(R_ESP+8);
//                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\"...)", tid, *(void**)(R_ESP), s, pu32?((char*)(pu32)):"nil");
//                } else  if(strstr(s, "puts")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\"...)", tid, *(void**)(R_ESP), s, *(char**)(R_ESP+4));
//                } else  if(strstr(s, "fputs")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", %p...)", tid, *(void**)(R_ESP), s, *(char**)(R_ESP+4), *(void**)(R_ESP+8));
//                } else  if(strstr(s, "my_fprintf")==s) {
//                    pu32 = *(uint32_t**)(R_ESP+8);
//                    if(((uintptr_t)pu32)<0x5) // probably a __fprint_chk
//                        pu32 = *(uint32_t**)(R_ESP+12);
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%08X, \"%s\", ...)", tid, *(void**)(R_ESP), s, *(uint32_t*)(R_ESP+4), pu32?((char*)(pu32)):"nil");
//                } else  if(strstr(s, "my_vfprintf")==s) {
//                    pu32 = *(uint32_t**)(R_ESP+8);
//                    if(((uintptr_t)pu32)<0x5) // probably a _chk function
//                        pu32 = *(uint32_t**)(R_ESP+12);
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%08X, \"%s\", ...)", tid, *(void**)(R_ESP), s, *(uint32_t*)(R_ESP+4), pu32?((char*)(pu32)):"nil");
//                } else  if(strstr(s, "vkGetInstanceProcAddr")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, \"%s\")", tid, *(void**)(R_ESP), s, *(void**)(R_ESP+4), *(char**)(R_ESP+8));
//                } else  if(strstr(s, "vkGetDeviceProcAddr")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, \"%s\")", tid, *(void**)(R_ESP), s, *(void**)(R_ESP+4), *(char**)(R_ESP+8));
//                } else  if(strstr(s, "my_glXGetProcAddress")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\")", tid, *(void**)(R_ESP), s, *(char**)(R_ESP+4));
//                } else  if(strstr(s, "my_sscanf")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\", ...)", tid, *(void**)(R_ESP), s, *(char**)(R_ESP+4), *(char**)(R_ESP+8));
//                } else  if(!strcmp(s, "vsscanf")) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\", ...)", tid, *(void**)(R_ESP), s, *(char**)(R_ESP+4), *(char**)(R_ESP+8));
//                } else if(strstr(s, "XCreateWindow")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, %p, %d, %d, %u, %u, %u, %d, %u, %p, %u, %p)", tid, *(void**)(R_ESP), s, *(void**)(R_ESP+4), *(void**)(R_ESP+8), *(int*)(R_ESP+12), *(int*)(R_ESP+16), *(uint32_t*)(R_ESP+20), *(uint32_t*)(R_ESP+24), *(uint32_t*)(R_ESP+28), *(int32_t*)(R_ESP+32), *(uint32_t*)(R_ESP+36), *(void**)(R_ESP+40), *(uint32_t*)(R_ESP+44), *(void**)(R_ESP+48));
//                } else if(strstr(s, "XLoadQueryFont")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, \"%s\")", tid, *(void**)(R_ESP), s, *(void**)(R_ESP+4), *(char**)(R_ESP+8));
//                } else if(strstr(s, "pthread_mutex_lock")==s) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%p)", tid, *(void**)(R_ESP), s, *(void**)(R_ESP+4));
//                } else if(!strcmp(s, "fmodf")) {
//                    post = 4;
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%f, %f)", tid, *(void**)(R_ESP), s, *(float*)(R_ESP+4), *(float*)(R_ESP+8));
//                } else if(!strcmp(s, "fmod")) {
//                    post = 4;
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%f, %f)", tid, *(void**)(R_ESP), s, *(double*)(R_ESP+4), *(double*)(R_ESP+12));
//                } else if(strstr(s, "SDL_GetWindowSurface")==s) {
//                    post = 5;
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%p)", tid, *(void**)(R_ESP), s, *(void**)(R_ESP+4));
//                } else if(strstr(s, "udev_monitor_new_from_netlink")==s) {
//                    post = 5;
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, \"%s\")", tid, *(void**)(R_ESP), s, *(void**)(R_ESP+4), *(char**)(R_ESP+8));
//                } else  if(!strcmp(s, "my_syscall")) {
//                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p, %p, %p...)", tid, *(void**)(R_ESP), s, *(int32_t*)(R_ESP+4), *(void**)(R_ESP+8), *(void**)(R_ESP+12), *(void**)(R_ESP+16));
//                    perr = 1;
//                } else {
//                    snprintf(buff, 255, "%04d|%p: Calling %s (%08X, %08X, %08X...)", tid, *(void**)(R_ESP), s, *(uint32_t*)(R_ESP+4), *(uint32_t*)(R_ESP+8), *(uint32_t*)(R_ESP+12));
//                }
//                printf_log(LOG_NONE, "%s =>", buff);
//                pthread_mutex_unlock(&emu->context->mutex_trace);
//                w(emu, addr);   // some function never come back, so unlock the mutex first!
//                pthread_mutex_lock(&emu->context->mutex_trace);
//                if(post)
//                    switch(post) {
//                    case 1: snprintf(buff2, 63, " [%d sec %d nsec]", pu32?pu32[0]:-1, pu32?pu32[1]:-1);
//                            break;
//                    case 2: snprintf(buff2, 63, "(%s)", R_EAX?((char*)R_EAX):"nil");
//                            break;
//                    case 3: snprintf(buff2, 63, "(%s)", pu32?((char*)pu32):"nil");
//                            break;
//                    case 4: snprintf(buff2, 63, " (%f)", ST0.d);
//                            break;
//                    case 5: {
//                            uint32_t* p = (uint32_t*)R_EAX;
//                            if(p)
//                                snprintf(buff2, 63, " size=%dx%d, pitch=%d, pixels=%p", p[2], p[3], p[4], p+5);
//                            else
//                                snprintf(buff2, 63, "NULL Surface");
//                            }
//                            break;
//                }
//                if(perr==1 && ((int)R_EAX)<0)
//                    snprintf(buff3, 63, " (errno=%d:\"%s\")", errno, strerror(errno));
//                else if(perr==2 && R_EAX==0)
//                    snprintf(buff3, 63, " (errno=%d:\"%s\")", errno, strerror(errno));
//                printf_log(LOG_NONE, " return 0x%08X%s%s\n", R_EAX, buff2, buff3);
//                pthread_mutex_unlock(&emu->context->mutex_trace);
//            } else
//                w(emu, addr);
//        }
//        return;
//    }
//    if(my_context->signals[SIGTRAP])
//        raise(SIGTRAP);
//    else
//        printf_log(LOG_INFO, "%04d|Warning, ignoring unsupported Int 3 call @%p\n", GetTID(), (void*)R_EIP);
//    //emu->quit = 1;
//}

int GetTID()
{
    return syscall(SYS_gettid);
}