#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <poll.h>
#include <sys/wait.h>
#include <elf.h>

#include "x64_signals.h"
#include "os.h"
#include "debug.h"
#include "box64stack.h"
#include "x64emu.h"
#include "box64cpu.h"
#include "x64emu_private.h"
#include "x64run_private.h"
#include "x64int_private.h"
#include "x87emu_private.h"
#include "x64primop.h"
#include "x64trace.h"
#include "wrapper.h"
#include "box64context.h"
#include "librarian.h"
#include "emit_signals.h"
#include "tools/bridge_private.h"

#include <elf.h>
#include "elfloader.h"
#include "elfload_dump.h"
#include "elfs/elfloader_private.h"

typedef int32_t (*iFpppp_t)(void*, void*, void*, void*);

x64emu_t* x64emu_fork(x64emu_t* emu, int forktype)
{
    // execute atforks prepare functions, in reverse order
    for (int i=my_context->atfork_sz-1; i>=0; --i)
        if(my_context->atforks[i].prepare)
            EmuCall(emu, my_context->atforks[i].prepare);
    //int type = emu->type;
    int v;
    if(forktype==2) {
        iFpppp_t forkpty = (iFpppp_t)emu->forkpty_info->f;
        v = forkpty(emu->forkpty_info->amaster, emu->forkpty_info->name, emu->forkpty_info->termp, emu->forkpty_info->winp);
        emu->forkpty_info = NULL;
    } /*else if(forktype==3)
        v = vfork();*/
    else
        v = fork();
    /*if(type == EMUTYPE_MAIN)
        thread_set_emu(emu);*/
    if(v==EAGAIN || v==ENOMEM) {
        // error...
    } else if(v!=0) {  
        // execute atforks parent functions
        for (int i=0; i<my_context->atfork_sz; --i)
            if(my_context->atforks[i].parent)
                EmuCall(emu, my_context->atforks[i].parent);
        if(forktype==3) {
            // vfork, the parent wait the end or execve of the son
            waitpid(v, NULL, WEXITED);
        }
    } else if(v==0) {
        ResetSegmentsCache(emu);
        // execute atforks child functions
        for (int i=0; i<my_context->atfork_sz; --i)
            if(my_context->atforks[i].child)
                EmuCall(emu, my_context->atforks[i].child);
    }
    R_EAX = v;
    return emu;
}


void printf_function(int level, x64emu_t* emu)
{
    onebridge_t* bridge = (onebridge_t*)(R_RIP&~(sizeof(onebridge_t)-1));
    if (IsBridgeSignature(bridge->S, bridge->C)) {
        printf_log(level, "calling %s\n", bridge->name?bridge->name:"????");
    } else {
        printf_log(level, "Could found the function name for fnc=%p\n", bridge->f);
    }
}

static uint64_t F64(uintptr_t* addr) {
    uint64_t ret = *(uint64_t*)*addr;
    *addr+=8;
    return ret;
}
static uint8_t Peek8(uintptr_t addr, uintptr_t offset)
{
    return *(uint8_t*)(addr+offset);
}

void x64Print(x64emu_t* emu, char* buff, size_t buffsz, const char* func, int tid, wrapper_t w);

void x64Int3(x64emu_t* emu, uintptr_t* addr)
{
    if(box64_is32bits) {
        x86Int3(emu,addr);
        return;
    }
    onebridge_t* bridge = (onebridge_t*)(*addr-1);
    if (IsBridgeSignature(bridge->S, bridge->C)) { // Signature for "Out of x86 door"
        *addr += 2;
        uintptr_t a = F64(addr);
        if(a==0) {
            R_RIP = *addr;
            //printf_log(LOG_INFO, "%p:Exit x86 emu (emu=%p)\n", *(void**)(R_ESP), emu);
            emu->quit=1; // normal quit
        } else {
            RESET_FLAGS(emu);
            wrapper_t w = bridge->w;
            a = F64(addr);
            R_RIP = *addr;
            /* This part can be used to trace only 1 specific lib (but it is quite slow)
            elfheader_t *h = FindElfAddress(my_context, *(uintptr_t*)(R_ESP));
            int have_trace = 0;
            if(h && strstr(ElfName(h), "libMiles")) have_trace = 1;*/
            if(BOX64ENV(log)>=LOG_DEBUG || BOX64ENV(rolling_log)) {
                int e = errno;
                int tid = GetTID();
                char t_buff[256] = "\0";
                char buff2[64] = "\0";
                char buff3[64] = "\0";
                int cycle_line = my_context->current_line;
                if(BOX64ENV(rolling_log)) {
                    my_context->current_line = (my_context->current_line+1)%BOX64ENV(rolling_log);
                }
                char* buff = BOX64ENV(rolling_log)?(my_context->log_call+256*cycle_line):t_buff;
                char* buffret = BOX64ENV(rolling_log)?(my_context->log_ret+128*cycle_line):NULL;
                if(buffret) buffret[0] = '\0';
                char *tmp;
                int post = 0;
                int perr = 0;
                uint64_t *pu64 = NULL;
                uint32_t *pu32 = NULL;
                uint8_t *pu8 = NULL;
                const char *s = bridge->name;
                if(!s)
                    s = GetNativeName((void*)a);
                if(a==(uintptr_t)PltResolver64) {
                    if(BOX64ENV(rolling_log)) {
                        uintptr_t addr = *((uint64_t*)(R_RSP));
                        int slot = *((uint64_t*)(R_RSP+8));
                        elfheader_t *h = (elfheader_t*)addr;
                        Elf64_Rela * rel = (Elf64_Rela *)(h->jmprel + h->delta) + slot;
                        Elf64_Sym *sym = &h->DynSym._64[ELF64_R_SYM(rel->r_info)];
                        const char* symname = SymName64(h, sym);
                        snprintf(buff, 256, "%04d|PltResolver \"%s\"", tid, symname?symname:"???");
                    } else {
                        snprintf(buff, 256, "%s", " ... ");
                    }
                } else if (!strcmp(s, "__open") || !strcmp(s, "open") || !strcmp(s, "open ") || !strcmp(s, "open64") || !strcmp(s, "my_open")) {
                    tmp = (char*)(R_RDI);
                    snprintf(buff, 256, "%04d|%p: Calling %s(\"%s\", %d (,%d))", tid, *(void**)(R_RSP), s, (tmp)?tmp:"(nil)", (int)(R_ESI), (int)(R_EDX));
                    perr = 1;
                } else if (!strcmp(s, "shm_open")) {
                    tmp = (char*)(R_RDI);
                    snprintf(buff, 256, "%04d|%p: Calling %s(\"%s\", %d, %d)", tid, *(void**)(R_RSP), s, (tmp)?tmp:"(nil)", (int)(R_ESI), (int)(R_EDX));
                    perr = 1;
                } else if (!strcmp(s, "fopen") || !strcmp(s, "fopen64") || !strcmp(s, "my_fopen")) {
                    tmp = (char*)(R_RDI);
                    snprintf(buff, 256, "%04d|%p: Calling %s(\"%s\", \"%s\")", tid, *(void**)(R_RSP), s, (tmp)?tmp:"(nil)", (char*)(R_RSI));
                    perr = 2;
                } else if (!strcmp(s, "__openat64") || !strcmp(s, "openat64") || !strcmp(s, "__openat64_2")) {
                    tmp = (char*)(R_RSI);
                    snprintf(buff, 256, "%04d|%p: Calling %s(%d, \"%s\", %d (,%d))", tid, *(void**)(R_RSP), s, S_EDI, (tmp)?tmp:"(nil)", (int)(R_EDX), (int)(R_ECX));
                    perr = 1;
                } else if (!strcmp(s, "readlink")) {
                    tmp = (char*)(R_RDI);
                    snprintf(buff, 256, "%04d|%p: Calling %s(\"%s\", %p, %zd)", tid, *(void**)(R_RSP), s, (tmp)?tmp:"(nil)", (void*)(R_RSI), (size_t)R_RDX);
                    perr = 1;
                } else if (!strcmp(s, "execv") || !strcmp(s, "my_execv")) {
                    tmp = (char*)(R_RDI);
                    snprintf(buff, 256, "%04d|%p: Calling %s(\"%s\", %p)", tid, *(void**)(R_RSP), s, (tmp)?tmp:"(nil)", (void*)(R_RSI));
                    perr = 1;
                } else if (strstr(s, "mkdir")==s) {
                    tmp = (char*)(R_RDI);
                    snprintf(buff, 256, "%04d|%p: Calling %s(\"%s\", %d)", tid, *(void**)(R_RSP), s, (tmp)?tmp:"(nil)", (int)(R_ESI));
                    perr = 1;
                } else if (strstr(s, "opendir")==s) {
                    tmp = (char*)(R_RDI);
                    snprintf(buff, 256, "%04d|%p: Calling %s(\"%s\")", tid, *(void**)(R_RSP), s, (tmp)?tmp:"(nil)");
                    perr = 2;
                } else if (!strcmp(s, "read") || !strcmp(s, "my_read")) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(%d, %p, %zu)", tid, *(void**)(R_RSP), s, R_EDI, (void*)R_RSI, R_RDX);
                    pu8 = (uint8_t*)R_RSI;
                    post = 8;
                    perr = 1;
                } else if (!strcmp(s, "write") || !strcmp(s, "my_write")) {
                    if(R_EDI==2 || R_EDI==3)
                        snprintf(buff, 256, "%04d|%p: Calling %s(%d, %p\"%.*s\", %zu)", tid, *(void**)(R_RSP), s, R_EDI, (void*)R_RSI, R_EDX, (char*)R_RSI, R_RDX);
                    else
                        snprintf(buff, 256, "%04d|%p: Calling %s(%d, %p, %zu)", tid, *(void**)(R_RSP), s, R_EDI, (void*)R_RSI, R_RDX);
                    perr = 1;
                } else if (strstr(s, "access")==s) {
                    tmp = (char*)(R_RDI);
                    snprintf(buff, 256, "%04d|%p: Calling %s(\"%s\", 0x%x)", tid, *(void**)(R_RSP), s, (tmp)?tmp:"(nil)", R_ESI);
                    perr = 1;
                } else if (strstr(s, "waitpid")==s) {
                    pu32 = (uint32_t*)R_RSI;
                    snprintf(buff, 256, "%04d|%p: Calling %s(%d, %p, %d)", tid, *(void**)(R_RSP), s, R_EDI, pu32, R_EDX);
                    perr = 1;
                    post = 6;
                } else if (!strcmp(s, "ioctl")) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(%d, 0x%x, %p)", tid, *(void**)(R_RSP), s, S_EDI, R_ESI, (void*)R_RDX);
                    perr = 1;
                } else if (!strcmp(s, "lseek64")) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(%d, %ld, %d)", tid, *(void**)(R_RSP), s, S_EDI, (int64_t)R_RSI, S_EDX);
                    perr = 1;
                } else if (!strcmp(s, "lseek")) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(%d, %ld, %d)", tid, *(void**)(R_RSP), s, S_EDI, (int64_t)R_RSI, S_EDX);
                    perr = 1;
                } else if (!strcmp(s, "recvmsg")) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(%d, %p, 0x%x)", tid, *(void**)(R_RSP), s, R_EDI, (void*)R_RSI, R_EDX);
                    perr = 1;
                } else if (!strcmp(s, "getxattr")) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(\"%s\", \"%s\", %p, 0x%zx)", tid, *(void**)(R_RSP), s, (char*)R_RDI, (char*)R_RSI, (void*)R_RDX, R_RCX);
                    perr = 1;
                } else if (!strcmp(s, "fgetxattr")) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(%d, \"%s\", %p, 0x%zx)", tid, *(void**)(R_RSP), s, R_EDI, (char*)R_RSI, (void*)R_RDX, R_RCX);
                    perr = 1;
                } else if (!strcmp(s, "connect")) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(%d, %p, %d)", tid, *(void**)(R_RSP), s, R_EDI, (void*)R_RSI, R_EDX);
                    perr = 1;
                } else if (strstr(s, "puts")==s) {
                    tmp = (char*)(R_RDI);
                    snprintf(buff, 256, "%04d|%p: Calling %s(\"%s\")", tid, *(void**)(R_RSP), s, (tmp)?tmp:"(nil)");
                } else if (!strcmp(s, "syscall")) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(%d, %p, %p....)", tid, *(void**)(R_RSP), s, S_EDI, (void*)R_RSI, (void*)R_RDX);
                    perr = 1;
                } else if (strstr(s, "strlen")==s) {
                    tmp = (char*)(R_RDI);
                    snprintf(buff, 256, "%04d|%p: Calling %s(\"%s\")", tid, *(void**)(R_RSP), s, (tmp)?tmp:"(nil)");
                } else if (strstr(s, "strcmp")==s) {
                    tmp = (char*)(R_RDI);
                    snprintf(buff, 256, "%04d|%p: Calling %s(\"%s\", \"%s\")", tid, *(void**)(R_RSP), s, (tmp)?tmp:"(nil)", (char*)R_RSI);
                } else if (strstr(s, "getenv")==s) {
                    tmp = (char*)(R_RDI);
                    post = 2;
                    snprintf(buff, 256, "%04d|%p: Calling %s(\"%s\")", tid, *(void**)(R_RSP), s, (tmp)?tmp:"(nil)");
                } else if (strstr(s, "setenv")==s) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(\"%s\", \"%s\", %d)", tid, *(void**)(R_RSP), s, (char*)R_RDI, (char*)R_RSI, R_EDX);
                } else if (strstr(s, "unsetenv")==s) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(\"%s\")", tid, *(void**)(R_RSP), s, (char*)R_RDI);
                } else if (strstr(s, "putenv")==s) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(\"%s\")", tid, *(void**)(R_RSP), s, (char*)R_RDI);
                } else if (!strcmp(s, "poll")) {
                    struct pollfd* pfd = (struct pollfd*)(R_RDI);
                    snprintf(buff, 256, "%04d|%p: Calling %s(%p[%d/%d/%d, ...], %d, %d)", tid, *(void**)(R_RSP), s, pfd, pfd->fd, pfd->events, pfd->revents, R_ESI, R_EDX);
                } else if (!strcmp(s, "__printf_chk") || !strcmp(s, "my___printf_chk")) {
                    tmp = (char*)(R_RSI);
                    snprintf(buff, 256, "%04d|%p: Calling %s(%d, \"%s\" (,%p))", tid, *(void**)(R_RSP), s, R_EDI, (tmp)?tmp:"(nil)", (void*)(R_RDX));
                } else if (!strcmp(s, "__snprintf_chk") || !strcmp(s, "my___snprintf_chk")) {
                    tmp = (char*)(R_R8);
                    pu64 = (uint64_t*)R_RDI;
                    post = 3;
                    snprintf(buff, 256, "%04d|%p: Calling %s(%p, %zu, %d, %zu, \"%s\" (,%p))", tid, *(void**)(R_RSP), s, (void*)R_RDI, R_RSI, R_EDX, R_RCX, (tmp)?tmp:"(nil)", (void*)(R_R9));
                } else if (!strcmp(s, "__vfprintf_chk") || !strcmp(s, "my___vfprintf_chk")) {
                    tmp = (char*)(R_RDX);
                    pu64 = (uint64_t*)R_RDI;
                    snprintf(buff, 256, "%04d|%p: Calling %s(%p, %d, \"%s\", ... )", tid, *(void**)(R_RSP), s, (void*)R_RDI, R_ESI, (tmp)?tmp:"(nil)");
                } else if (!strcmp(s, "snprintf") || !strcmp(s, "my_snprintf")) {
                    tmp = (char*)(R_RDX);
                    pu64 = (uint64_t*)R_RDI;
                    post = 3;
                    snprintf(buff, 256, "%04d|%p: Calling %s(%p, %zu, \"%s\" (,%p))", tid, *(void**)(R_RSP), s, (void*)R_RDI, R_RSI, (tmp)?tmp:"(nil)", (void*)(R_RCX));
                } else if (!strcmp(s, "sprintf") || !strcmp(s, "my_sprintf")) {
                    tmp = (char*)(R_RSI);
                    pu64 = (uint64_t*)R_RDI;
                    post = 3;
                    snprintf(buff, 256, "%04d|%p: Calling %s(%p, \"%s\" (,%p))", tid, *(void**)(R_RSP), s, (void*)R_RDI, (tmp)?tmp:"(nil)", (void*)(R_RDX));
                } else if (!strcmp(s, "vfprintf") || !strcmp(s, "my_vfprintf")) {
                    tmp = (char*)((R_RSI>2)?R_RSI:R_RDX);
                    snprintf(buff, 256, "%04d|%p: Calling %s(%p, \"%s\", ...)", tid, *(void**)(R_RSP), s, (void*)R_RDI, (tmp)?tmp:"(nil)");
                } else if (!strcmp(s, "g_source_set_name")) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(%p, \"%s\")", tid, *(void**)(R_RSP), s, (void*)R_RDI, (char*)R_RSI);
                } else if (!strcmp(s, "getcwd")) {
                    post = 2;
                    snprintf(buff, 256, "%04d|%p: Calling %s(%p, %zu)", tid, *(void**)(R_RSP), s, (void*)R_RDI, R_RSI);
                } else if (!strcmp(s, "ftok")) {
                    tmp = (char*)(R_RDI);
                    perr = 1;
                    snprintf(buff, 256, "%04d|%p: Calling %s(\"%s\", %d)", tid, *(void**)(R_RSP), s, tmp?tmp:"nil", R_ESI);
                } else if (!strcmp(s, "xcb_wait_for_event") || !strcmp(s, "xcb_poll_for_queued_event") || !strcmp(s, "xcb_poll_for_event")) {
                    post = 9;
                    snprintf(buff, 256, "%04d|%p: Calling %s(%p)", tid, *(void**)(R_RSP), s, (void*)R_RDI);
                } else if (!strcmp(s, "glXGetProcAddress") || !strcmp(s, "SDL_GL_GetProcAddress") || !strcmp(s, "glXGetProcAddressARB")) {
                    tmp = (char*)(R_RDI);
                    snprintf(buff, 256, "%04d|%p: Calling %s(\"%s\")", tid, *(void**)(R_RSP), s, (tmp)?tmp:"(nil)");
                } else if (!strcmp(s, "glLabelObjectEXT")) {
                    tmp = (char*)(R_RCX);
                    snprintf(buff, 256, "%04d|%p: Calling %s(0x%x, %d, %d, \"%s\")", tid, *(void**)(R_RSP), s, R_EDI, R_ESI, R_ECX, (tmp)?tmp:"(nil)");
                } else if (!strcmp(s, "glGetStringi")) {
                    post = 2;
                    snprintf(buff, 256, "%04d|%p: Calling %s(0x%x, %d)", tid, *(void**)(R_RSP), s, R_EDI, R_ESI);
                } else if (!strcmp(s, "_dl_tlsdesc_undefweak")) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(RAX=%p)", tid, *(void**)(R_RSP), s, (void*)R_RAX);
                } else if (!strcmp(s, "glFramebufferTexture2D")) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(0x%x, 0x%x, 0x%x, %u, %d)", tid, *(void**)(R_RSP), s, R_EDI, R_ESI, R_EDX, R_ECX, R_R8d);
                } else if (!strcmp(s, "glTexSubImage2D")) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(0x%x, %d, %d, %d, %d, %d, 0x%x, 0x%x, %p)", tid, *(void**)(R_RSP), s, R_EDI, R_ESI, R_EDX, R_ECX, R_R8d, R_R9d, *(uint32_t*)(R_RSP+8), *(uint32_t*)(R_RSP+16), *(void**)(R_RSP+24));
                } else if (!strcmp(s, "glCompressedTexSubImage2D")) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(0x%x, %d, %d, %d, %d, %d, 0x%x, %d, %p)", tid, *(void**)(R_RSP), s, R_EDI, R_ESI, R_EDX, R_ECX, R_R8d, R_R9d, *(uint32_t*)(R_RSP+8), *(uint32_t*)(R_RSP+16), *(void**)(R_RSP+24));
                } else if (!strcmp(s, "glVertexAttribPointer")) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(%u, %d, 0x%x, %d, %d, %p)", tid, *(void**)(R_RSP), s, R_EDI, R_ESI, R_EDX, R_ECX, R_R8d, (void*)R_R9);
                } else if (!strcmp(s, "glDrawElements")) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(0x%x, %d, 0x%x, %p)", tid, *(void**)(R_RSP), s, R_EDI, R_ESI, R_EDX, (void*)R_RCX);
                } else if (!strcmp(s, "glUniform4fv")) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(%d, %d, %p[%g/%g/%g/%g...])", tid, *(void**)(R_RSP), s, R_EDI, R_ESI, (void*)R_RDX, ((float*)(R_RDX))[0], ((float*)(R_RDX))[1], ((float*)(R_RDX))[2], ((float*)(R_RDX))[3]);
                } else if (!strcmp(s, "ov_read")) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(%p, %p, %d, %d, %d, %d, %p)", tid, *(void**)(R_RSP), s, (void*)R_RDI, (void*)R_RSI, R_EDX, R_ECX, R_R8d, R_R9d, *(void**)(R_RSP+8));
                } else if (!strcmp(s, "mmap64") || !strcmp(s, "mmap")) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(%p, 0x%lx, 0x%x, 0x%x, %d, %ld)", tid, *(void**)(R_RSP), s,
                        (void*)R_RDI, R_RSI, (int)(R_RDX), (int)(S_RCX), (int)(S_R8), R_R9);
                    perr = 3;
                } else if (!strcmp(s, "sscanf")) {
                    tmp = (char*)(R_RSI);
                    snprintf(buff, 256, "%04d|%p: Calling %s(%p, \"%s\" (,%p))", tid, *(void**)(R_RSP), s, (void*)R_RDI, (tmp)?tmp:"(nil)", (void*)(R_RDX));
                } else if (!strcmp(s, "__isoc99_fscanf")) {
                    tmp = (char*)(R_RSI);
                    snprintf(buff, 256, "%04d|%p: Calling %s(%p, \"%s\" (,%p))", tid, *(void**)(R_RSP), s, (void*)R_RDI, (tmp)?tmp:"(nil)", (void*)(R_RDX));
                } else if (!strcmp(s, "inotify_add_watch")) {
                    tmp = (char*)(R_RSI);
                    snprintf(buff, 256, "%04d|%p: Calling %s(%d, \"%s\" , 0x%x)", tid, *(void**)(R_RSP), s, R_EDI, (tmp)?tmp:"(nil)", R_EDX);
                    perr = 1;
                } else if (!strcmp(s, "__xstat64")) {
                    tmp = (char*)(R_RSI);
                    snprintf(buff, 256, "%04d|%p: Calling %s(%d, \"%s\" , %p)", tid, *(void**)(R_RSP), s, R_EDI, (tmp)?tmp:"(nil)", (void*)R_RDX);
                    perr = 1;
                } else if (!strcmp(s, "XCreateWindow")) {
                    tmp = (char*)(R_RSI);
                    snprintf(buff, 256, "%04d|%p: Calling %s(%p, %p, %d, %d, %u, %u, %u, %d, %u, %p, 0x%lx, %p)", tid, *(void**)(R_RSP), s, 
                        (void*)R_RDI, (void*)R_RSI, S_EDX, S_ECX, R_R8d, R_R9d, 
                        (uint32_t)*(uint64_t*)(R_RSP+8), (int)*(uint64_t*)(R_RSP+16), 
                        (uint32_t)*(uint64_t*)(R_RSP+24), (void*)*(uint64_t*)(R_RSP+32), 
                        (unsigned long)*(uint64_t*)(R_RSP+40), (void*)*(uint64_t*)(R_RSP+48));
                } else if (!strcmp(s, "pipe2")) {
                    pu32 = (uint32_t*)(R_RDI);
                    post = 7;
                    snprintf(buff, 256, "%04d|%p: Calling %s(%p, 0x%X)", tid, *(void**)(R_RSP), s, (void*)R_RDI, R_ESI);
                } else  if(!strcmp(s, "__errno_location")) {
                    snprintf(buff, 255, "%04d|%p: Calling %s()", tid, *(void**)(R_RSP), s);
                    perr = 4;
                } else {
                    x64Print(emu, buff, 256, s, tid, w);
                }
                if(!BOX64ENV(rolling_log)) {
                    mutex_lock(&emu->context->mutex_trace);
                    printf_log_prefix(0, LOG_NONE, "%s =>", buff);
                    mutex_unlock(&emu->context->mutex_trace);
                }
                errno = e;
                w(emu, a);   // some function never come back, so unlock the mutex first!
                e = errno;
                if(post)
                    switch(post) { // Only ever 2 for now...
                    case 1: snprintf(buff2, 64, " [%llu sec %llu nsec]", pu64?pu64[0]:~0ull, pu64?pu64[1]:~0ull);
                            break;
                    case 2: snprintf(buff2, 64, "(%s)", R_RAX?((char*)R_RAX):"nil");
                            break;
                    case 3: snprintf(buff2, 64, "(%s)", pu64?((char*)pu64):"nil");
                            break;
                    case 4: snprintf(buff2, 64, " (%f)", ST0.d);
                            break;
                    case 5: {
                            uint32_t* p = (uint32_t*)R_RAX; // uint64_t? (case never used)
                            if(p)
                                snprintf(buff2, 64, " size=%ux%u, pitch=%u, pixels=%p", p[2], p[3], p[4], p+5);
                            else
                                snprintf(buff2, 64, "NULL Surface");
                        }
                        break;
                    case 6: if(pu32) snprintf(buff2, 64, " [0x%x] ", pu32[0]);
                        break;
                    case 7: if(pu32) snprintf(buff2, 64, " [%d, %d] ", pu32[0], pu32[1]);
                        break;
                    case 8: if(pu8 && S_RAX!=-1) {
                        char buff5[64] = "";
                        char buff6[10];
                        int n = S_EAX; if(n>64/(3*2)) n=64/(3*2);
                        for(int i=0; i<n; ++i) {
                            snprintf(buff6, sizeof(buff6), "%02X ", pu8[i]);
                            strcat(buff5, buff6);
                        }
                        snprintf(buff2, 64, "[%s%s] ", buff5, (n==S_EAX)?"":"...");
                    }
                    break;
                    case 9: if(R_RAX) {
                        uint8_t type = *(uint8_t*)(R_RAX);
                        snprintf(buff2, 64, "[type=%d]", type);
                    }
                    break;
                }
                if(perr==1 && (S_EAX)<0)
                    snprintf(buff3, 64, " (errno=%d:\"%s\")", e, strerror(e));
                else if(perr==2 && R_EAX==0)
                    snprintf(buff3, 64, " (errno=%d:\"%s\")", e, strerror(e));
                else if(perr==3 && (S_RAX)==-1)
                    snprintf(buff3, 64, " (errno=%d:\"%s\")", e, strerror(e));
                else if(perr==4)
                    snprintf(buff3, 64, " (errno=%d:\"%s\")", e, strerror(e));

                if(BOX64ENV(rolling_log))
                    snprintf(buffret, 127, "0x%lX%s%s", R_RAX, buff2, buff3);
                else {
                    mutex_lock(&emu->context->mutex_trace);
                    printf_log_prefix(0, LOG_NONE, " return 0x%lX%s%s\n", R_RAX, buff2, buff3);
                    mutex_unlock(&emu->context->mutex_trace);
                }
                errno = e;
            } else
                w(emu, a);
        }
        return;
    }
    if(Peek8(*addr, -1)!=0xCC) {
        // why this happens?!
        printf_log(LOG_DEBUG, "%04d|Warning, x64int3 with no CC opcode at %p?\n", GetTID(), (void*)R_RIP);
        return;
    }
    if(!BOX64ENV(ignoreint3) && my_context->signals[X64_SIGTRAP]) {
        R_RIP = *addr;  // update RIP
        EmitSignal(emu, X64_SIGTRAP, NULL, 3);
    } else {
        printf_log(LOG_DEBUG, "%04d|Warning, ignoring unsupported Int 3 call @%p\n", GetTID(), (void*)R_RIP);
        R_RIP = *addr;
    }
    //emu->quit = 1;
}

void print_rolling_log(int loglevel) {
    if(BOX64ENV(rolling_log)) {
        printf_log(loglevel, "Last calls\n");
        int j = (my_context->current_line+1)%BOX64ENV(rolling_log);
        for (int i=0; i<BOX64ENV(rolling_log); ++i) {
            int k = (i+j)%BOX64ENV(rolling_log);
            if(my_context->log_call[256*k+0]) {
                printf_log(loglevel, "%s => return %s\n", my_context->log_call+256*k, my_context->log_ret+128*k);
            }
        }
    }
}

#ifndef BOX32
void x86Int3(x64emu_t* emu, uintptr_t* addr)
{
    printf_log(LOG_NONE, "Error: Calling 32bits wrapped function without box32 support built in\n");
    abort();
}
#endif
