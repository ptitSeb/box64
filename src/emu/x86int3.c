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
#include <inttypes.h>

#include "x64_signals.h"
#include "os.h"
#include "debug.h"
#include "box64stack.h"
#include "x64emu.h"
#include "x64emu_private.h"
#include "x64run_private.h"
#include "x87emu_private.h"
#include "x64primop.h"
#include "x64trace.h"
#include "wrapper32.h"
#include "box32context.h"
#include "librarian.h"
#include "emit_signals.h"
#include "tools/bridge_private.h"

#include <elf.h>
#include "elfloader.h"
#include "elfload_dump.h"
#include "elfs/elfloader_private.h"

typedef int32_t (*iFpppp_t)(void*, void*, void*, void*);

static uint64_t F64(uintptr_t* addr) {
    uint64_t ret = *(uint64_t*)*addr;
    *addr+=8;
    return ret;
}
static uint8_t Peek8(uintptr_t addr, uintptr_t offset)
{
    return *(uint8_t*)(addr+offset);
}

extern int errno;
void x86Int3(x64emu_t* emu, uintptr_t* addr)
{
    onebridge_t* bridge = (onebridge_t*)(*addr-1);
    if (IsBridgeSignature(Peek8(*addr, 0), Peek8(*addr, 1))) { // Signature for "Out of x86 door"
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
            /* This party can be used to trace only 1 specific lib (but it is quite slow)
            elfheader_t *h = FindElfAddress(my_context, *(uintptr_t*)(R_ESP));
            int have_trace = 0;
            if(h && strstr(ElfName(h), "libMiles")) have_trace = 1;*/
            if(BOX64ENV(log)>=LOG_DEBUG || BOX64ENV(rolling_log)) {
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
                int ret_fmt = 0;
                uint64_t *pu64 = NULL;
                uint32_t *pu32 = NULL;
                uint8_t *pu8 = NULL;
                const char *s = (bridge->func)?GetNativeName(bridge->name_or_func):bridge->name_or_func;
                if(!s)
                    s = GetNativeName((void*)a);
                if(a==(uintptr_t)PltResolver32) {
                    if(BOX64ENV(rolling_log)) {
                        ptr_t addr = *((uint32_t*)from_ptrv(R_ESP));
                        int slot = *((uint32_t*)from_ptrv(R_ESP+4));
                        elfheader_t *h = (elfheader_t*)from_ptrv(addr);
                        Elf32_Rel * rel = (Elf32_Rel *)from_ptrv(h->jmprel + h->delta + slot);
                        Elf32_Sym *sym = &h->DynSym._32[ELF32_R_SYM(rel->r_info)];
                        const char* symname = SymName32(h, sym);
                        snprintf(buff, 256, "%04d|PltResolver \"%s\"", tid, symname?symname:"???");
                    } else {
                        snprintf(buff, 256, "%s", " ... ");
                    }
                } else
                if(strstr(s, "SDL_PollEvent")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4));
                    pu8 = from_ptriv(R_ESP+4);
                    post = 10;
                } else if(strstr(s, "SDL_RWFromFile")==s || strstr(s, "SDL_RWFromFile")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\")", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8));
                } else if(strstr(s, "SDL_WarpMouse")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%hd, %hd)", tid, from_ptriv(R_ESP), (char *)s, *(uint16_t*)from_ptr(R_ESP+4), *(uint16_t*)from_ptr(R_ESP+8));
                } else  if(strstr(s, "glColor4f")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%f, %f, %f, %f)", tid, from_ptriv(R_ESP), (char *)s, *(float*)from_ptr(R_ESP+4), *(float*)from_ptr(R_ESP+8), *(float*)from_ptr(R_ESP+12), *(float*)from_ptr(R_ESP+16));
                } else  if(strstr(s, "glTexCoord2f")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%f, %f)", tid, from_ptriv(R_ESP), (char *)s, *(float*)from_ptr(R_ESP+4), *(float*)from_ptr(R_ESP+8));
                } else  if(strstr(s, "glVertex2f")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%f, %f)", tid, from_ptriv(R_ESP), (char *)s, *(float*)from_ptr(R_ESP+4), *(float*)from_ptr(R_ESP+8));
                } else  if(strstr(s, "glVertex3f")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%f, %f, %f)", tid, from_ptriv(R_ESP), (char *)s, *(float*)from_ptr(R_ESP+4), *(float*)from_ptr(R_ESP+8), *(float*)from_ptr(R_ESP+12));
                } else  if(strstr(s, "__open64")==s || strcmp(s, "open64")==0) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", %d, %d)", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), *(int*)from_ptr(R_ESP+8), *(int*)from_ptr(R_ESP+12));
                    perr = 1;
                } else  if(!strcmp(s, "opendir")) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\")", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4));
                    perr = 2;
                } else  if(strstr(s, "__open")==s || !strcmp(s, "open") || !strcmp(s, "my_open64")) {
                    tmp = from_ptriv(R_ESP+4);
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", %d (,%d))", tid, from_ptriv(R_ESP), (char *)s, (tmp)?tmp:"(nil)", *(int*)from_ptr(R_ESP+8), *(int*)from_ptr(R_ESP+12));
                    perr = 1;
                } else  if(!strcmp(s, "shm_open")) {
                    tmp = from_ptriv(R_ESP+4);
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", %d, %d)", tid, from_ptriv(R_ESP), (char *)s, (tmp)?tmp:"(nil)", *(int*)from_ptr(R_ESP+8), *(int*)from_ptr(R_ESP+12));
                    perr = 1;
                } else  if(strcmp(s, "mkdir")==0) {
                    tmp = from_ptriv(R_ESP+4);
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", %d)", tid, from_ptriv(R_ESP), (char *)s, (tmp)?tmp:"(nil)", *(int*)from_ptr(R_ESP+8));
                    perr = 1;
                } else  if(!strcmp(s, "fopen")) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\")", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8));
                    perr = 2;
                } else  if(!strcmp(s, "freopen")) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\", %p)", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8), from_ptriv(R_ESP+12));
                    perr = 2;
                } else  if(!strcmp(s, "fopen64")) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\")", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8));
                    perr = 2;
                } else  if(!strcmp(s, "chdir")) {
                    pu32=(uint32_t*)from_ptrv(R_ESP+4);
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\")", tid, from_ptriv(R_ESP), (char *)s, pu32?((pu32==(uint32_t*)1)?"/1/":(char*)(uintptr_t)(*pu32)):"/0/");
                } else  if(!strcmp(s, "poll")) {
                    pu32=from_ptrv(*(ptr_t*)from_ptrv(R_ESP+4));
                    char tmp[50];
                    char tmp2[50] = {0};
                    uint32_t n = from_ptri(uint32_t, R_ESP+8);
                    for(int ii=0; ii<n; ++ii) {
                        snprintf(tmp, 49, "%s%d/0x%hx", ii?" ,":"", ((int*)pu32)[ii*2], ((uint16_t*)pu32)[ii*4+2]);
                        strncat(tmp2, tmp, 49);
                    }
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p[%s], %u, %d)", tid, from_ptriv(R_ESP), (char *)s, from_ptrv(R_ESP+4), tmp2, from_ptri(uint32_t, R_ESP+8), from_ptri(int, R_ESP+12));
                    post = 12;
                    perr = 1;
                } else  if(strstr(s, "getenv")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\")", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4));
                    post = 2;
                } else  if(strstr(s, "putenv")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\")", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4));
                } else  if(strstr(s, "pread")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p, %u, %d)", tid, from_ptriv(R_ESP), (char *)s, from_ptri(int32_t, R_ESP+4), from_ptriv(R_ESP+8), from_ptri(uint32_t, R_ESP+12), from_ptri(int32_t, R_ESP+16));
                    perr = 1;
                } else  if(!strcmp(s, "read")) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p, %u)", tid, from_ptriv(R_ESP), (char *)s, from_ptri(int32_t, R_ESP+4), from_ptriv(R_ESP+8), from_ptri(uint32_t, R_ESP+12));
                    perr = 1;
                } else  if(strstr(s, "ioctl")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, 0x%x, %p)", tid, from_ptriv(R_ESP), (char *)s, from_ptri(int32_t, R_ESP+4), from_ptri(int32_t, R_ESP+8), from_ptriv(R_ESP+12));
                    perr = 1;
                } else  if(strstr(s, "setsockopt")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %d, %d, %p, %d)", tid, from_ptriv(R_ESP), (char *)s, from_ptri(int32_t, R_ESP+4), from_ptri(int32_t, R_ESP+8), from_ptri(int32_t, R_ESP+12), from_ptriv(R_ESP+16), from_ptri(int32_t, R_ESP+20));
                    perr = 1;
                } else  if(strstr(s, "connect")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p, %d)", tid, from_ptriv(R_ESP), (char *)s, from_ptri(int32_t, R_ESP+4), from_ptriv(R_ESP+8), from_ptri(int32_t, R_ESP+12));
                    perr = 1;
                } else  if(strstr(s, "__errno_location")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s()", tid, from_ptriv(R_ESP), (char *)s);
                    post = 9;
                } else  if(strstr(s, "statvfs64")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p(\"%s\"), %p)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8));
                } else  if(strstr(s, "index")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p(\"%s\"), %i(%c))", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+4), from_ptri(int32_t, R_ESP+8), from_ptri(int32_t, R_ESP+8));
                } else  if(strstr(s, "rindex")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p(\"%s\"), %i(%c))", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+4), from_ptri(int32_t, R_ESP+8), from_ptri(int32_t, R_ESP+8));
                } else  if(strstr(s, "__xstat64")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p(\"%s\"), %p)", tid, from_ptriv(R_ESP), (char *)s, from_ptri(int32_t, R_ESP+4), from_ptriv(R_ESP+8), (char *)from_ptriv(R_ESP+8), from_ptriv(R_ESP+12));
                    perr = 1;
                } else  if(strcmp(s, "__xstat")==0) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p(\"%s\"), %p)", tid, from_ptriv(R_ESP), (char *)s, from_ptri(int32_t, R_ESP+4), from_ptriv(R_ESP+8), (char *)from_ptriv(R_ESP+8), from_ptriv(R_ESP+12));
                    perr = 1;
                } else  if(strstr(s, "__lxstat64")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p(\"%s\"), %p)", tid, from_ptriv(R_ESP), (char *)s, from_ptri(int32_t, R_ESP+4), from_ptriv(R_ESP+8), (char *)from_ptriv(R_ESP+8), from_ptriv(R_ESP+12));
                    perr = 1;
                } else  if(strstr(s, "sem_timedwait")==s) {
                    pu32 = (uint32_t*)from_ptr(*(ptr_t*)from_ptr(R_ESP+8));
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, %p[%d sec %d ns])", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8), pu32?pu32[0]:-1, pu32?pu32[1]:-1);
                    perr = 1;
                } else  if(strstr(s, "waitpid")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p, 0x%x)", tid, from_ptriv(R_ESP), (char *)s, from_ptri(int32_t, R_ESP+4), from_ptriv(R_ESP+8), from_ptri(uint32_t, R_ESP+12));
                    perr = 1;
                } else  if(!strcmp(s, "wait")) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4));
                    pu32 = from_ptriv(R_ESP+4);
                    perr = 1;
                    if(pu32) post = 14;
                } else  if(strstr(s, "clock_gettime")==s || strstr(s, "__clock_gettime")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p)", tid, from_ptriv(R_ESP), (char *)s, from_ptri(uint32_t, R_ESP+4), from_ptriv(R_ESP+8));
                    post = 1;
                    pu32 = (uint32_t*)from_ptr(*(ptr_t*)from_ptr(R_ESP+8));
                } else  if(strstr(s, "semop")==s) {
                    int16_t* p16 = *(int16_t**)from_ptr(R_ESP+8);
                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p, %d)", tid, from_ptriv(R_ESP), (char *)s, *(int*)from_ptr(R_ESP+4), p16, *(int*)from_ptr(R_ESP+12));
                    perr = 1;
                } else  if(!strcmp(s, "mmap64")) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, 0x%x, %d, 0x%x, %d, %ld)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), from_ptri(ulong_t, R_ESP+8), from_ptri(int, R_ESP+12), from_ptri(int, R_ESP+16), from_ptri(int, R_ESP+20),from_ptri(int64_t, R_ESP+24));
                    perr = 3;
                } else  if(!strcmp(s, "mmap")) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, 0x%x, %d, 0x%x, %d, %d)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), from_ptri(ulong_t, R_ESP+8), from_ptri(int, R_ESP+12), from_ptri(int, R_ESP+16), from_ptri(int, R_ESP+20), from_ptri(int, R_ESP+24));
                    perr = 3;
                } else  if(strstr(s, "strcasecmp")==s || strstr(s, "__strcasecmp")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\")", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8));
                    ret_fmt = 1;
                } else  if(strstr(s, "wcsncasecmp")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\", %d)", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8), *(int*)from_ptr(R_ESP+12));
                    ret_fmt = 1;
                } else  if(strstr(s, "gtk_signal_connect_full")) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, \"%s\", %p, %p, %p, %p, %d, %d)", tid, from_ptriv(R_ESP), "gtk_signal_connect_full", (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8), from_ptriv(R_ESP+12), *(void**)from_ptr(R_ESP+16), *(void**)from_ptr(R_ESP+20), *(void**)from_ptr(R_ESP+24), from_ptri(int32_t, R_ESP+28), from_ptri(int32_t, R_ESP+32));
                } else  if(strstr(s, "strcmp")==s || strstr(s, "__strcmp")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\")", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8));
                    ret_fmt = 1;
                } else  if(strstr(s, "strncmp")==s || strstr(s, "__strncmp")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\", %" PRIu32 ")", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8), from_ptri(ulong_t, R_ESP+12));
                    ret_fmt = 1;
                } else  if(strstr(s, "strncasecmp")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\", %" PRIu32 ")", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8), from_ptri(ulong_t, R_ESP+12));
                    ret_fmt = 1;
                } else  if(strstr(s, "strtol")==s || strstr(s, "strtoul")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", %p, %d)", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8), *(int*)from_ptr(R_ESP+12));
                    ret_fmt = 1;
                } else  if(strstr(s, "memcmp")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, %p, %" PRIu32 ")", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8), from_ptri(ulong_t, R_ESP+12));
                    ret_fmt = 1;
                } else  if(strstr(s, "strstr")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p\"%.127s\", \"%.127s\")", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8));
                } else  if(strstr(s, "strlen")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p(\"%s\"))", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), ((R_ESP+4))?((char*)from_ptriv(R_ESP+4)):"nil");
                } else  if(strstr(s, "strchr")==s || strstr(s, "strrchr")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p\"%.127s\", 0x%x'%c')", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), (char*)from_ptriv(R_ESP+4), *(char*)from_ptr(R_ESP+8), *(char*)from_ptr(R_ESP+8));
                } else  if(strstr(s, "vsnprintf")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%08X, %u, %08X...)", tid, from_ptriv(R_ESP), (char *)s, from_ptri(uint32_t, R_ESP+4), from_ptri(uint32_t, R_ESP+8), from_ptri(uint32_t, R_ESP+12));
                    pu32 = (uint32_t*)from_ptr(*(ptr_t*)from_ptr(R_ESP+4));
                    post = 3;
                } else  if(strstr(s, "__vsnprintf_chk")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, %u, %d, %d, %p...)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), from_ptri(uint32_t, R_ESP+8), *(int*)from_ptr(R_ESP+12), *(int*)from_ptr(R_ESP+16),from_ptriv(R_ESP+20));
                    pu32 = (uint32_t*)from_ptr(*(ptr_t*)from_ptr(R_ESP+4));
                    post = pu32?3:0;
                } else  if(strstr(s, "vsprintf")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, \"%s\", %p)", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8), from_ptriv(R_ESP+12));
                    pu32 = (uint32_t*)from_ptr(*(ptr_t*)from_ptr(R_ESP+4));
                    post = 3;
                } else  if(strstr(s, "__vsprintf_chk")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, %d, %zu, \"%s\", %p)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), *(int*)from_ptr(R_ESP+8), *(size_t*)from_ptr(R_ESP+12), (char*)from_ptriv(R_ESP+16), *(void**)from_ptr(R_ESP+20));
                    pu32 = (uint32_t*)from_ptr(*(ptr_t*)from_ptr(R_ESP+4));
                    post = 3;
                } else  if(strstr(s, "__snprintf_chk")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, %zu, %d, %d, \"%s\", %p)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), from_ptri(long_t, R_ESP+8), *(int*)from_ptr(R_ESP+12), *(int*)from_ptr(R_ESP+16), (char*)from_ptriv(R_ESP+20), *(void**)from_ptr(R_ESP+24));
                    pu32 = (uint32_t*)from_ptr(*(ptr_t*)from_ptr(R_ESP+4));
                    post = 3;
                } else  if(strstr(s, "snprintf")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, %zu, \"%s\", ...)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), from_ptri(long_t, R_ESP+8), (char*)from_ptriv(R_ESP+12));
                    pu32 = (uint32_t*)from_ptr(*(ptr_t*)from_ptr(R_ESP+4));
                    post = 3;
                } else  if(strstr(s, "sprintf")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%08X, %08X...)", tid, from_ptriv(R_ESP), (char *)s, from_ptri(uint32_t, R_ESP+4), from_ptri(uint32_t, R_ESP+8));
                    pu32 = (uint32_t*)from_ptr(*(ptr_t*)from_ptr(R_ESP+4));
                    post = 3;
                } else  if(strstr(s, "printf")==s) {
                    pu32 = (uint32_t*)from_ptr(*(ptr_t*)from_ptr(R_ESP+4));
                    if(((uintptr_t)pu32)<0x5) // probably a _chk function
                        pu32 = (uint32_t*)from_ptr(*(ptr_t*)from_ptr(R_ESP+8));
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\"...)", tid, from_ptriv(R_ESP), (char *)s, pu32?((char*)(pu32)):"nil");
                } else  if(strstr(s, "__printf_chk")==s) {
                    pu32 = (uint32_t*)from_ptr(*(ptr_t*)from_ptr(R_ESP+8));
                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, \"%s\", ...)", tid, from_ptriv(R_ESP), (char *)s, from_ptri(int, R_ESP+4), pu32?((char*)(pu32)):"nil");
                } else  if(strstr(s, "wprintf")==s) {
                    pu32 = (uint32_t*)from_ptr(*(ptr_t*)from_ptr(R_ESP+4));
                    if(((uintptr_t)pu32)<0x5) // probably a _chk function
                        pu32 = (uint32_t*)from_ptr(*(ptr_t*)from_ptr(R_ESP+8));
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%S\"...)", tid, from_ptriv(R_ESP), (char *)s, pu32?((wchar_t*)(pu32)):L"nil");
                } else  if(!strcmp(s, "__vswprintf_chk")) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, %u, %d, %u, \"%S\"...)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), from_ptri(ulong_t, R_ESP+8), *(int*)from_ptr(R_ESP+12), from_ptri(ulong_t, R_ESP+16), (wchar_t*)from_ptriv(R_ESP+20));
                    pu32 = (uint32_t*)from_ptr(*(ptr_t*)from_ptr(R_ESP+4));
                    post = 6;
                } else  if(strstr(s, "__vswprintf")==s) {
                    if(*(size_t*)from_ptr(R_ESP+12)<2) {
                        snprintf(buff, 255, "%04d|%p: Calling %s(%p, %u, %p, %p, ...)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), from_ptri(ulong_t, R_ESP+8), from_ptriv(R_ESP+12), *(void**)from_ptr(R_ESP+16));
                    } else {
                        snprintf(buff, 255, "%04d|%p: Calling %s(%p, %u, \"%S\", ...)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), from_ptri(ulong_t, R_ESP+8), *(wchar_t**)from_ptr(R_ESP+12));
                        pu32 = (uint32_t*)from_ptr(*(ptr_t*)from_ptr(R_ESP+4));
                        post = 6;
                    }
                } else  if(strstr(s, "puts")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\"...)", tid, from_ptriv(R_ESP), (char *)s, (char*)from_ptriv(R_ESP+4));
                } else  if(strstr(s, "fputs")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", %p...)", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8));
                } else  if(strstr(s, "fgets")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, %d, %p)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), *(int*)from_ptr(R_ESP+8), from_ptriv(R_ESP+8));
                    post = 2;
                    perr = 2;
                } else  if(strstr(s, "__uselocale")==s || strstr(s, "uselocale")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4));
                    perr = 3;
                } else  if(strstr(s, "fprintf")==s) {
                    pu32 = (uint32_t*)from_ptr(*(ptr_t*)from_ptr(R_ESP+8));
                    if(((uintptr_t)pu32)<0x5) // probably a __fprint_chk
                        pu32 = (uint32_t*)from_ptr(*(ptr_t*)from_ptr(R_ESP+12));
                    snprintf(buff, 255, "%04d|%p: Calling %s(%08X, \"%s\", ...)", tid, from_ptriv(R_ESP), (char *)s, from_ptri(uint32_t, R_ESP+4), pu32?((char*)(pu32)):"nil");
                } else  if(strstr(s, "vfprintf")==s) {
                    pu32 = (uint32_t*)from_ptr(*(ptr_t*)from_ptr(R_ESP+8));
                    if(((uintptr_t)pu32)<0x5) // probably a _chk function
                        pu32 = (uint32_t*)from_ptr(*(ptr_t*)from_ptr(R_ESP+12));
                    snprintf(buff, 255, "%04d|%p: Calling %s(%08X, \"%s\", ...)", tid, from_ptriv(R_ESP), (char *)s, from_ptri(uint32_t, R_ESP+4), pu32?((char*)(pu32)):"nil");
                } else  if(strstr(s, "vkGetInstanceProcAddr")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, \"%s\")", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8));
                } else  if(strstr(s, "vkGetDeviceProcAddr")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, \"%s\")", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8));
                } else  if(strstr(s, "glXGetProcAddress")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\")", tid, from_ptriv(R_ESP), (char *)s, (char*)from_ptriv(R_ESP+4));
                } else if (!strcmp(s, "glTexImage2D")) {
                    snprintf(buff, 256, "%04d|%p: Calling %s(0x%x, %d, 0x%x, %d, %d, %d, 0x%x, 0x%x, %p)", tid, from_ptrv(*(ptr_t*)from_ptrv(R_ESP)), (char *)s, *(uint32_t*)from_ptrv(R_ESP+4), *(int*)from_ptrv(R_ESP+8), *(int*)from_ptrv(R_ESP+12), *(int*)from_ptrv(R_ESP+16), *(int*)from_ptrv(R_ESP+20), *(int*)from_ptrv(R_ESP+24), *(uint32_t*)from_ptrv(R_ESP+28), *(uint32_t*)from_ptrv(R_ESP+32), from_ptrv(*(ptr_t*)from_ptrv(R_ESP+36)));
                } else  if(strstr(s, "fscanf")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, \"%s\", ...)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8));
                } else  if(strstr(s, "sscanf")==s || !strcmp(s, "__isoc99_sscanf")) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\", ...)", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8));
                } else  if(!strcmp(s, "vsscanf")) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\", ...)", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8));
                } else if(strstr(s, "XCreateWindow")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, %p, %d, %d, %u, %u, %u, %d, %u, %p, 0x%x, %p)", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8), *(int*)from_ptr(R_ESP+12), *(int*)from_ptr(R_ESP+16), from_ptri(uint32_t, R_ESP+20), from_ptri(uint32_t, R_ESP+24), from_ptri(uint32_t, R_ESP+28), from_ptri(int32_t, R_ESP+32), from_ptri(uint32_t, R_ESP+36), from_ptriv(R_ESP+40), from_ptri(uint32_t, R_ESP+44), from_ptriv(R_ESP+48));
                } else if(strstr(s, "XLoadQueryFont")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, \"%s\")", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8));
                } else if(strstr(s, "XRRSetScreenSize")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, %u, %d, %d, %d, %d)", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), from_ptri(ulong_t, R_ESP+8), from_ptri(int, R_ESP+12), from_ptri(int, R_ESP+16), from_ptri(int, R_ESP+20), from_ptri(int, R_ESP+24));
                } else if(strstr(s, "XextAddDisplay")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, %p, \'%s\', %p, %d, %p)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), from_ptriv(R_ESP+8), (char *)from_ptriv(R_ESP+12), from_ptriv(R_ESP+16), from_ptri(int, R_ESP+20), from_ptriv(R_ESP+24));
                } else if(strstr(s, "_XGetRequest")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, %hhu, %u)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), from_ptri(uint8_t, R_ESP+8), from_ptri(ulong_t, R_ESP+12));
                } else if(strstr(s, "_XReply")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, %p, %d, %d)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), from_ptriv(R_ESP+8), from_ptri(int, R_ESP+12), from_ptri(int, R_ESP+16));
                } else if(strstr(s, "pthread_mutex_init")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, %p)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), from_ptriv(R_ESP+8));
                } else if(strstr(s, "pthread_mutex_lock")==s || strstr(s, "pthread_mutex_unlock")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4));
                } else if(strstr(s, "pthread_setname_np")==s) {
                    post = 7;
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, \"%s\")", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), (char*)from_ptriv(R_ESP+8));
                } else if(strstr(s, "pthread_create")==s) {
                    post = 7;
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, %p, %p, %p)", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8), from_ptriv(R_ESP+12), from_ptriv(R_ESP+16));
                } else if(!strcmp(s, "fmodf")) {
                    post = 4;
                    snprintf(buff, 255, "%04d|%p: Calling %s(%f, %f)", tid, from_ptriv(R_ESP), (char *)s, *(float*)from_ptr(R_ESP+4), *(float*)from_ptr(R_ESP+8));
                } else if(!strcmp(s, "fmod")) {
                    post = 4;
                    snprintf(buff, 255, "%04d|%p: Calling %s(%f, %f)", tid, from_ptriv(R_ESP), (char *)s, *(double*)from_ptr(R_ESP+4), *(double*)from_ptr(R_ESP+12));
                } else if(strstr(s, "pthread_once")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p[%d], %p)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), *(int*)from_ptriv(R_ESP+4), from_ptriv(R_ESP+8));
                } else if(!strcmp(s, "posix_memalign")) {
                    post = 8;
                    pu32 = (uint32_t*)from_ptr(*(ptr_t*)from_ptr(R_ESP+4));
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, 0x%x, 0x%x)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), from_ptri(uint32_t, R_ESP+8), from_ptri(uint32_t, R_ESP+12));
                } else  if(strstr(s, "SDL_GL_LoadLibrary")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\")", tid, from_ptriv(R_ESP), (char *)s, (char*)from_ptriv(R_ESP+4));
                } else if(strstr(s, "SDL_GetWindowSurface")==s) {
                    post = 5;
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4));
                } else  if(strstr(s, "___tls_get_addr")) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p[%d, %d])", tid, from_ptriv(R_ESP), (char *)s, from_ptrv(R_EAX), ((int*)from_ptrv(R_EAX))[0], ((int*)from_ptrv(R_EAX))[1]);
                } else if (!strcmp(s, "FT_Outline_Get_CBox")) {
                    pu32 = *(uint32_t**)from_ptrv(R_ESP+8);
                    snprintf(buff, 256, "%04d|%p: Calling %s(%p, %p)", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8));
                    post = 11;
                } else if(strstr(s, "udev_monitor_new_from_netlink")==s) {
                    post = 5;
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, \"%s\")", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8));
                } else if(!strcmp(s, "recv")) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p, 0x%x, %d)", tid, from_ptriv(R_ESP), (char *)s, from_ptri(int, R_ESP+4), from_ptriv(R_ESP+8), from_ptri(uint32_t, R_ESP+12), from_ptri(int, R_ESP+16));
                    post = 13;
                } else if(!strcmp(s, "fnmatch")) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p\"%s\", %p\"%s\", %d)", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+4), from_ptriv(R_ESP+8), (char *)from_ptriv(R_ESP+8), from_ptri(int, R_ESP+12));
                    post = 13;
                } else  if(!strcmp(s, "syscall")) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p, %p, %p...)", tid, from_ptriv(R_ESP), (char *)s, from_ptri(int32_t, R_ESP+4), from_ptriv(R_ESP+8), from_ptriv(R_ESP+12), from_ptriv(R_ESP+16));
                    perr = 1;
                } else  if(!strcmp(s, "snd_device_name_get_hint")) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, \"%s\")", tid, from_ptriv(R_ESP), (char *)s, from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8));
                    post = 2;
                } else  if(!strcmp(s, "getaddrinfo")) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\", %p, %p)", tid, from_ptriv(R_ESP), (char *)s, (char *)from_ptriv(R_ESP+4), (char *)from_ptriv(R_ESP+8), from_ptriv(R_ESP+12), from_ptriv(R_ESP+16));
                } else  if(!strcmp(s, "__errno_location")) {
                    snprintf(buff, 255, "%04d|%p: Calling %s()", tid, from_ptriv(R_ESP), (char *)s);
                    perr = 4;
                } else if (!strcmp(s, "nanosleep")) {
                    pu32 = (uint32_t*)from_ptrv(*(uint32_t*)from_ptrv(R_ESP+4));
                    snprintf(buff, 256, "%04d|%p: Calling %s(%p[%d, %d], %p)", tid, from_ptriv(R_ESP), (char *)s, (void*)from_ptriv(R_ESP+4), pu32?pu32[0]:0, pu32?pu32[1]:0, (void*)from_ptriv(R_ESP+8));
                } else {
                    snprintf(buff, 255, "%04d|%p: Calling %s (%08X, %08X, %08X...)", tid, from_ptriv(R_ESP), (char *)s, from_ptri(uint32_t, R_ESP+4), from_ptri(uint32_t, R_ESP+8), from_ptri(uint32_t, R_ESP+12));
                }
                if(!BOX64ENV(rolling_log)) {
                    mutex_lock(&emu->context->mutex_trace);
                    printf_log(LOG_NONE, "%s =>", buff);
                    mutex_unlock(&emu->context->mutex_trace);
                }
                w(emu, a);   // some function never come back, so unlock the mutex first!
                if(post)
                    switch(post) {
                    case 1: snprintf(buff2, 63, " [%d sec %d nsec]", pu32?pu32[0]:-1, pu32?pu32[1]:-1);
                            break;
                    case 2: snprintf(buff2, 63, "(%s)", R_EAX?((char*)from_ptr(R_EAX)):"nil");
                            break;
                    case 3: snprintf(buff2, 63, "(%s)", pu32?((char*)pu32):"nil");
                            break;
                    case 4: snprintf(buff2, 63, " (%f)", ST0.d);
                            break;
                    case 5: {
                            uint32_t* p = (uint32_t*)from_ptrv(R_EAX);
                            if(p)
                                snprintf(buff2, 63, " size=%dx%d, pitch=%d, pixels=%p", p[2], p[3], p[4], p+5);
                            else
                                snprintf(buff2, 63, "NULL Surface");
                            }
                            break;
                    case 6: snprintf(buff2, 63, "(%S)", pu32?((wchar_t*)pu32):L"nil");
                            break;
                    case 7: if(R_EAX) snprintf(buff2, 63, " (error=\"%s\")", strerror(R_EAX)); break;
                    case 8: if(!R_EAX) snprintf(buff2, 63, " [%p]", from_ptrv(*pu32)); break;
                    case 9: if(errno) snprintf(buff2, 63, " (errno=%d/\"%s\")", errno, strerror(errno)); else snprintf(buff2, 63, " (errno=0)"); break;
                            break;
                    case 10: if(R_EAX)
                            switch(*pu8) {
                                case 4:
                                    snprintf(buff2, 63, " [type=%hhd, x=%hd, y=%hd, relx=%+hd, rely=%+hd]", *pu8, *(uint16_t*)(pu8+4), *(uint16_t*)(pu8+6), *(int16_t*)(pu8+8), *(int16_t*)(pu8+10)); 
                                    break;
                                default:
                                    snprintf(buff2, 63, " [type=%hhd]", *pu8); 
                            }
                            break;
                    case 11: snprintf(buff2, 63, " [%d / %d / %d /%d]", pu32[0], pu32[1], pu32[2], pu32[3]);
                            break;
                    case 12: if(R_EAX>0) {
                        char tmp[50];
                        char tmp2[50] = {0};
                        uint32_t n = from_ptri(uint32_t, R_ESP+8);
                        for(int ii=0; ii<n; ++ii) {
                            snprintf(tmp, 49, "%s%d/0x%hx", ii?" ,":"", pu32[ii*2], pu32[ii*2+1]>>16);
                            strncat(tmp2, tmp, 49);
                        }
                        snprintf(buff2, 63, "[%s]", tmp2);
                    }
                    case 13: if(R_EAX==0x25E)
                                snprintf(buff2, 63, "%s", "here");
                            break;
                    case 14: snprintf(buff2, 63, " [%d]", *(int*)pu32);
                            break;
                }
                if(perr==1 && (S_EAX)<0)
                    snprintf(buff3, 63, " (errno=%d:\"%s\")", errno, strerror(errno));
                else if(perr==2 && R_EAX==0)
                    snprintf(buff3, 63, " (errno=%d:\"%s\")", errno, strerror(errno));
                else if(perr==3 && (S_EAX)==-1)
                    snprintf(buff3, 63, " (errno=%d:\"%s\")", errno, strerror(errno));
                else if(perr==4)
                    snprintf(buff3, 63, " (errno=%d:\"%s\")", errno, strerror(errno));
                if(BOX64ENV(rolling_log)) {
                    if(ret_fmt==1)
                        snprintf(buffret, 127, "%d%s%s", S_EAX, buff2, buff3);
                    else
                        snprintf(buffret, 127, "0x%X%s%s", R_EAX, buff2, buff3);
                } else {
                    mutex_lock(&emu->context->mutex_trace);
                    if(ret_fmt==1)
                        printf_log_prefix(0, LOG_NONE, " return %d%s%s\n", S_EAX, buff2, buff3);
                    else
                        printf_log_prefix(0, LOG_NONE, " return 0x%X%s%s\n", R_EAX, buff2, buff3);
                    mutex_unlock(&emu->context->mutex_trace);
                }
            } else
                w(emu, a);
        }
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
