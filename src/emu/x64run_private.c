#define _GNU_SOURCE /* See feature_test_macros(7) */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_TRACE
#include <unistd.h>
#include <sys/syscall.h>
#endif

#include "os.h"
#include "debug.h"
#include "box64stack.h"
#include "x64emu.h"
#include "box64cpu.h"
#include "box64cpu_util.h"
#include "x64emu_private.h"
#include "x64run_private.h"
#include "box64context.h"
#include "librarian.h"
#include "symbolfuncs.h"
#ifdef HAVE_TRACE
#include "x64trace.h"
#endif
#include "x64tls.h"
#include "bridge.h"
#ifdef BOX32
#include "box32.h"
#else
#define from_ptrv(A) ((void*)(uintptr_t)(A))
#endif


#ifdef HAVE_TRACE
#define PK(a)     (*(uint8_t*)(ip+a))
#define PKS(a)    (*(int8_t*)(ip+a))
#define PK32(a)   (*(int32_t*)((uint8_t*)(ip+a)))
#define PK64(a)   (*(int64_t*)((uint8_t*)(ip+a)))

void PrintTrace(x64emu_t* emu, uintptr_t ip, int dynarec)
{
    int is32bits = (emu->segs[_CS]==0x23);
    if(BOX64ENV(start_cnt)) SET_BOX64ENV(start_cnt, BOX64ENV(start_cnt)-1);
    if(!BOX64ENV(start_cnt) && my_context->dec && (
            (trace_end == 0)
            || ((ip >= trace_start) && (ip < trace_end))) ) {
        int tid = GetTID();
        mutex_lock(&my_context->mutex_trace);
#ifdef DYNAREC
        if((my_context->trace_tid != tid) || (my_context->trace_dynarec!=dynarec)) {
            printf_log(LOG_NONE, "Thread %04d| (%s) ", tid, dynarec?"dyn":"int");
            PrintFunctionAddr(ip, "here: ");
            printf_log_prefix(0, LOG_NONE, "\n");
            my_context->trace_tid = tid;
            my_context->trace_dynarec = dynarec;
        }
#else
        (void)dynarec;
        if(my_context->trace_tid != tid) {
            printf_log(LOG_NONE, "Thread %04d|\n", tid);
            my_context->trace_tid = tid;
        }
#endif
        printf_log(LOG_NONE, "%s", DumpCPURegs(emu, ip, is32bits));
        if(R_RIP==0) {
            printf_log_prefix(0, LOG_NONE, "Running at NULL address\n");
            mutex_unlock(&my_context->mutex_trace);
            return;
        }
        if (PK(0) == 0xcc && IsBridgeSignature(PK(1), PK(2))) {
            uint64_t a = *(uint64_t*)(ip+3);
            if(a==0) {
                printf_log_prefix(0, LOG_NONE, "%p: Exit x86emu\n", (void*)ip);
            } else {
                printf_log_prefix(0, LOG_NONE, "%p: Native call to %p => %s\n", (void*)ip, (void*)a, GetNativeName(*(void**)(ip+11)));
            }
        } else {
            printf_log_prefix(0, LOG_NONE, "%s", DecodeX64Trace(is32bits ? my_context->dec32 : my_context->dec, ip, 1));
            uint8_t peek = PK(0);
            rex_t rex = {0};
            if(!is32bits && peek>=0x40 && peek<=0x4f) {
                rex.rex = peek;
                ip++;
                peek = PK(0);
            }
            if(peek==0xC3 || peek==0xC2 || (peek==0xF3 && PK(1)==0xC3)) {
                if(is32bits) {
                    printf_log_prefix(0, LOG_NONE, " => %p", (void*)(uintptr_t)*(uint32_t*)(R_RSP));
                    PrintFunctionAddr(*(uint32_t*)(R_RSP), "=> ");
                } else {
                    printf_log_prefix(0, LOG_NONE, " => %p", *(void**)(R_RSP));
                    PrintFunctionAddr(*(uintptr_t*)(R_RSP), "=> ");
                }
            } else if(peek==0x57 && rex.b) {
                printf_log_prefix(0, LOG_NONE, " => STACK_TOP: %p", *(void**)(R_RSP));
                PrintFunctionAddr(ip, "here: ");
            } else if((peek==0x55 /*|| peek==0x53*/) && !is32bits) {
                if(!PrintFunctionAddr(*(uintptr_t*)(R_RSP), " STACK_TOP: "))
                    printf_log_prefix(0, LOG_NONE, " STACK_TOP: %p ", (void*)*(uintptr_t*)(R_RSP));
            } else if((peek==0x55 || peek==0x56 || peek==0x53 || peek==0x57) && is32bits) {
                if(!PrintFunctionAddr(*(uint32_t*)(R_RSP), " STACK_TOP: "))
                    printf_log_prefix(0, LOG_NONE, " STACK_TOP: %p ", (void*)(uintptr_t)*(uint32_t*)(R_RSP));
            } else if(peek==0xF3 && PK(1)==0x0F && PK(2)==0x1E && PK(3)==0xFA && !is32bits) {
                uintptr_t nextaddr = *(uintptr_t*)(R_RSP);
                if(!PrintFunctionAddr(nextaddr, "=> "))
                    printf_log_prefix(0, LOG_NONE, " => %p", (void*)nextaddr);
            } else if((peek==0x81 || peek==0x83) && PK(1)==0xEC && is32bits) {
                uintptr_t nextaddr = *(ptr_t*)from_ptrv(R_ESP);
                if(!PrintFunctionAddr(nextaddr, "STACK_TOP: "))
                    printf_log_prefix(0, LOG_NONE, " STACK_TOP: %p", (void*)nextaddr);
            } else if(peek==0xE8 || peek==0xE9) { // Call & Jmp
                uintptr_t nextaddr = ip + 5 + PK32(1);
                PrintFunctionAddr(nextaddr, "=> ");
            } else if(peek==0xFF) {
                if(PK(1)==0x25) {
                    uintptr_t nextaddr = is32bits?(*(uint32_t*)(uintptr_t)PK32(2)):(*(uintptr_t*)(ip + 6 + PK32(2)));
                    if(!PrintFunctionAddr(nextaddr, "=> "))
                        printf_log_prefix(0, LOG_NONE, " => %p", (void*)nextaddr);
                } else if(PK(1)==0x15) {
                    uintptr_t nextaddr = is32bits?(*(uint32_t*)(uintptr_t)PK32(2)):(*(uintptr_t*)(ip + 6 + PK32(2)));
                    if(!PrintFunctionAddr(nextaddr, "=> "))
                        printf_log_prefix(0, LOG_NONE, " => %p", (void*)nextaddr);
                } else if(PK(1)==0x60) {
                    uintptr_t nextaddr = *(uintptr_t*)(R_RAX+PK(2));
                    if(!PrintFunctionAddr(nextaddr, "=> "))
                        printf_log_prefix(0, LOG_NONE, " => %p", (void*)nextaddr);
                } else if(PK(1)==0xE0) {
                    uintptr_t nextaddr = R_RAX;
                    if(is32bits) nextaddr &= 0xffffffff;
                    if(!PrintFunctionAddr(nextaddr, "=> "))
                        printf_log_prefix(0, LOG_NONE, " => %p", (void*)nextaddr);
                } else if((PK(1)==0x14) && (PK(2)==0x25)) {
                    uintptr_t nextaddr = is32bits?(*(uint32_t*)(uintptr_t)PK32(3)):(*(uintptr_t*)(uintptr_t)PK32(3));
                    printf_log_prefix(0, LOG_NONE, " => %p", (void*)nextaddr);
                    PrintFunctionAddr(nextaddr, "=> ");
                } else if((PK(1)==0x14) && (PK(2)==0xC2) && rex.rex==0x41) {
                    uintptr_t nextaddr = *(uintptr_t*)(R_R10 + R_RAX*8);
                    printf_log_prefix(0, LOG_NONE, " => %p", (void*)nextaddr);
                    PrintFunctionAddr(nextaddr, "=> ");
                } else if(PK(1)==0xE1 && rex.rex==0x41) {
                    uintptr_t nextaddr = R_R9;
                    printf_log_prefix(0, LOG_NONE, " => %p", (void*)nextaddr);
                    PrintFunctionAddr(nextaddr, "=> ");
                } else if(is32bits && PK(1)==0xA3) {
                    uintptr_t nextaddr = *(ptr_t*)from_ptrv(R_EBX + PK32(2));
                    printf_log_prefix(0, LOG_NONE, " => %p", (void*)nextaddr);
                    PrintFunctionAddr(nextaddr, "=> ");
                } else if(PK(1)==0x92) {
                    uintptr_t nextaddr = is32bits?(*(ptr_t*)from_ptrv(R_EDX + PK32(2))):(*(uintptr_t*)(R_RDX + PK32(2)));
                    printf_log_prefix(0, LOG_NONE, " => %p", (void*)nextaddr);
                    PrintFunctionAddr(nextaddr, "=> ");
                } else if(PK(1)==0x50) {
                    uintptr_t nextaddr = is32bits?(*(ptr_t*)from_ptrv(R_EAX + PK(2))):(*(uintptr_t*)(R_RAX + PK(2)));
                    printf_log_prefix(0, LOG_NONE, " => %p", (void*)nextaddr);
                    PrintFunctionAddr(nextaddr, "=> ");
                } else if(PK(1)==0x52) {
                    uintptr_t nextaddr = is32bits?(*(ptr_t*)from_ptrv(R_EDX + PK(2))):(*(uintptr_t*)(R_RDX + PK(2)));
                    printf_log_prefix(0, LOG_NONE, " => %p", (void*)nextaddr);
                    PrintFunctionAddr(nextaddr, "=> ");
                } else if(is32bits && PK(1)==0x10) {
                    uintptr_t nextaddr = *(ptr_t*)from_ptrv(R_EAX);
                    printf_log_prefix(0, LOG_NONE, " => %p", (void*)nextaddr);
                    PrintFunctionAddr(nextaddr, "=> ");
                }

            }
            printf_log_prefix(0, LOG_NONE, "\n");
        }
        mutex_unlock(&my_context->mutex_trace);
    }
}

#endif

static uint8_t F8(uintptr_t* addr) {
    uint8_t ret = *(uint8_t*)*addr;
    *addr+=1;
    return ret;
}
static int8_t F8S(uintptr_t* addr) {
    int8_t ret = *(int8_t*)*addr;
    *addr+=1;
    return ret;
}
static uint16_t F16(uintptr_t* addr) {
    uint16_t ret = *(uint16_t*)*addr;
    *addr+=2;
    return ret;
}
static int16_t F16S(uintptr_t* addr) {
    int16_t ret = *(int16_t*)*addr;
    *addr+=2;
    return ret;
}
static uint32_t F32(uintptr_t* addr) {
    uint32_t ret = *(uint32_t*)*addr;
    *addr+=4;
    return ret;
}
static int32_t F32S(uintptr_t* addr) {
    int32_t ret = *(int32_t*)*addr;
    *addr+=4;
    return ret;
}
static uint64_t F64(uintptr_t* addr) {
    uint64_t ret = *(uint64_t*)*addr;
    *addr+=8;
    return ret;
}

reg64_t* GetECommon_32(x64emu_t* emu, uintptr_t* addr, uint8_t m, uint32_t base)
{
    if (m<=7) {
        if(m==0x4) {
            uint8_t sib = F8(addr);
            base += ((sib&0x7)==5)?((uint32_t)F32S(addr)):(emu->regs[(sib&0x7)].dword[0]); // base
            base += (emu->sbiidx[((sib>>3)&7)]->sdword[0] << (sib>>6));
            return (reg64_t*)(uintptr_t)base;
        } else if (m==0x5) { //disp32
            base += F32S(addr);
            return (reg64_t*)(uintptr_t)base;
        }
        return (reg64_t*)(uintptr_t)(base + emu->regs[m].dword[0]);
    } else {
        if((m&7)==4) {
            uint8_t sib = F8(addr);
            base += emu->regs[(sib&0x7)].dword[0]; // base
            base += (emu->sbiidx[((sib>>3)&7)]->sdword[0] << (sib>>6));
        } else {
            base += emu->regs[(m&0x7)].dword[0];
        }
        base+=(m&0x80)?F32S(addr):F8S(addr);
        return (reg64_t*)(uintptr_t)base;
    }
}
reg64_t* GetECommon_16(x64emu_t *emu, uintptr_t* addr, uint8_t m, uint32_t base)
{
    switch(m&7) {
        case 0: base+= R_BX+R_SI; break;
        case 1: base+= R_BX+R_DI; break;
        case 2: base+= R_BP+R_SI; break;
        case 3: base+= R_BP+R_DI; break;
        case 4: base+=      R_SI; break;
        case 5: base+=      R_DI; break;
        case 6: base+=      R_BP; break;
        case 7: base+=      R_BX; break;
    }
    switch((m>>6)&3) {
        case 0: if((m&7)==6) base= F16S(addr); break;
        case 1: base += F8S(addr); break;
        case 2: base += F16S(addr); break;
        // case 3 is C0..C7, already dealt with
    }
    return (reg64_t*)(uintptr_t)base;
}

reg64_t* GetECommon_64(x64emu_t* emu, uintptr_t* addr, rex_t rex, uint8_t m, uint8_t delta, uintptr_t base)
{
    if (m<=7) {
        if(m==0x4) {
            uint8_t sib = F8(addr);
            base += ((sib&0x7)==5)?((uint64_t)(int64_t)F32S(addr)):(emu->regs[(sib&0x7)+(rex.b<<3)].q[0]); // base
            base += (emu->sbiidx[((sib>>3)&7)+(rex.x<<3)]->sq[0] << (sib>>6));
            return (reg64_t*)base;
        } else if (m==0x5) { //disp32
            base += F32S(addr);
            return (reg64_t*)(base+*addr+delta);
        }
        return (reg64_t*)(base + emu->regs[m+(rex.b<<3)].q[0]);
    } else {
        if((m&7)==4) {
            uint8_t sib = F8(addr);
            base += emu->regs[(sib&0x7)+(rex.b<<3)].q[0]; // base
            base += (emu->sbiidx[((sib>>3)&7)+(rex.x<<3)]->sq[0] << (sib>>6));
        } else {
            base += emu->regs[(m&0x7)+(rex.b<<3)].q[0];
        }
        base+=(m&0x80)?F32S(addr):F8S(addr);
        return (reg64_t*)base;
    }
}

reg64_t* GetECommon32O(x64emu_t* emu, uintptr_t* addr, rex_t rex, uint8_t m, uint8_t delta, uintptr_t base)
{
    if (m<=7) {
        if(m==0x4) {
            uint8_t sib = F8(addr);
            base += ((sib&0x7)==5)?((uint64_t)(int64_t)F32S(addr)):(emu->regs[(sib&0x7)+(rex.b<<3)].dword[0]); // base
            base += (emu->sbiidx[((sib>>3)&7)+(rex.x<<3)]->sdword[0] << (sib>>6));
            return (reg64_t*)base;
        } else if (m==0x5) { //disp32
            base += F32S(addr);
            return (reg64_t*)(base+(*addr)+delta);
        }
        return (reg64_t*)(uintptr_t)(base + emu->regs[m+(rex.b<<3)].dword[0]);
    } else {
        if((m&7)==4) {
            uint8_t sib = F8(addr);
            base += emu->regs[(sib&0x7)+(rex.b<<3)].dword[0]; // base
            base += (emu->sbiidx[((sib>>3)&7)+(rex.x<<3)]->sdword[0] << (sib>>6));
        } else {
            base += emu->regs[(m&0x7)+(rex.b<<3)].dword[0];
        }
        base+=(m&0x80)?F32S(addr):F8S(addr);
        return (reg64_t*)base;
    }
}

reg64_t* GetECommon(x64emu_t* emu, uintptr_t* addr, rex_t rex, uint8_t m, uint8_t delta)
{
    if(rex.is32bits && rex.is67)
        return GetECommon_16(emu, addr, m, rex.offset);
    if(rex.is32bits)
        return GetECommon_32(emu, addr, m, rex.offset);
    if(rex.is67)
        return GetECommon32O(emu, addr, rex, m, delta, rex.offset);
    if (m<=7) {
        if(m==0x4) {
            uint8_t sib = F8(addr);
            uintptr_t base = ((sib&0x7)==5)?((uint64_t)(int64_t)F32S(addr)):(emu->regs[(sib&0x7)+(rex.b<<3)].q[0]); // base
            base += (emu->sbiidx[((sib>>3)&7)+(rex.x<<3)]->sq[0] << (sib>>6));
            base += rex.offset;
            return (reg64_t*)base;
        } else if (m==0x5) { //disp32
            int32_t base = F32S(addr);
            base += rex.offset;
            return (reg64_t*)(base+*addr+delta);
        }
        return (reg64_t*)(emu->regs[m+(rex.b<<3)].q[0]);
    } else {
        uintptr_t base;
        if((m&7)==4) {
            uint8_t sib = F8(addr);
            base = emu->regs[(sib&0x7)+(rex.b<<3)].q[0]; // base
            base += (emu->sbiidx[((sib>>3)&7)+(rex.x<<3)]->sq[0] << (sib>>6));
        } else {
            base = emu->regs[(m&0x7)+(rex.b<<3)].q[0];
        }
        base+=(m&0x80)?F32S(addr):F8S(addr);
        base += rex.offset;
        return (reg64_t*)base;
    }
}

reg64_t* GetEb(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta)
{
    if(rex.is67 && rex.is32bits) {
        printf_log(LOG_NONE, "Need 32bits 67 prefix GetEb\n");
        emu->quit = 1;
        return NULL;
    }
    uint8_t m = v&0xC7;    // filter Eb
    if(m>=0xC0) {
        if(rex.rex) {
            return &emu->regs[(m&0x07)+(rex.b<<3)];
        } else {
            int lowhigh = (m&4)>>2;
            return (reg64_t *)(((char*)(&emu->regs[(m&0x03)]))+lowhigh);  //?
        }
    } else return GetECommon(emu, addr, rex, m, delta);
}

reg64_t* TestEb(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta)
{
    if(rex.is67 && rex.is32bits) {
        printf_log(LOG_NONE, "Need 32bits 67 prefix TestEb\n");
        test->emu->quit = 1;
        return NULL;
    }
    uint8_t m = v&0xC7;    // filter Eb
    if(m>=0xC0) {
        if(rex.rex) {
            return &test->emu->regs[(m&0x07)+(rex.b<<3)];
        } else {
            int lowhigh = (m&4)>>2;
            return (reg64_t *)(((char*)(&test->emu->regs[(m&0x03)]))+lowhigh);  //?
        }
    } else {
        reg64_t* ret = GetECommon(test->emu, addr, rex, m, delta);
        test->memsize = 1;
        test->memaddr = (uintptr_t)ret;
        test->mem[0] = ret->byte[0];
        return (reg64_t*)test->mem;
    }
}

reg64_t* GetEd(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta)
{
    if(rex.is67 && rex.is32bits) {
        printf_log(LOG_NONE, "Need 32bits 67 prefix GetEd\n");
        emu->quit = 1;
        return NULL;
    }
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return &emu->regs[(m&0x07)+(rex.b<<3)];
    } else return GetECommon(emu, addr, rex, m, delta);
}

reg64_t* TestEd(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta)
{
    if(rex.is67 && rex.is32bits) {
        printf_log(LOG_NONE, "Need 32bits 67 prefix TestEd\n");
        test->emu->quit = 1;
        return NULL;
    }
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
        return &test->emu->regs[(m&0x07)+(rex.b<<3)];
    } else {
        reg64_t* ret =  GetECommon(test->emu, addr, rex, m, delta);
        test->memsize = 4<<rex.w;
        test->memaddr = (uintptr_t)ret;
        if(rex.w)
            *(uint64_t*)test->mem = ret->q[0];
        else
            *(uint32_t*)test->mem = ret->dword[0];
        return (reg64_t*)test->mem;
    }
}
reg64_t* TestEd4(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta)
{
    if(rex.is67 && rex.is32bits) {
        printf_log(LOG_NONE, "Need 32bits 67 prefix TestEd4x\n");
        test->emu->quit = 1;
        return NULL;
    }
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
        return &test->emu->regs[(m&0x07)+(rex.b<<3)];
    } else {
        reg64_t* ret =  GetECommon(test->emu, addr, rex, m, delta);
        test->memsize = 4;
        test->memaddr = (uintptr_t)ret;
        *(uint32_t*)test->mem = ret->dword[0];
        return (reg64_t*)test->mem;
    }
}
reg64_t* TestEd8(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta)
{
    if(rex.is67 && rex.is32bits) {
        printf_log(LOG_NONE, "Need 32bits 67 prefix TestEd8\n");
        test->emu->quit = 1;
        return NULL;
    }
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
        return &test->emu->regs[(m&0x07)+(rex.b<<3)];
    } else {
        reg64_t* ret =  GetECommon(test->emu, addr, rex, m, delta);
        test->memsize = 8;
        test->memaddr = (uintptr_t)ret;
        *(uint64_t*)test->mem = ret->q[0];
        return (reg64_t*)test->mem;
    }
}
reg64_t* TestEdt(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta)
{
    if(rex.is67 && rex.is32bits) {
        printf_log(LOG_NONE, "Need 32bits 67 prefix TestEdt\n");
        test->emu->quit = 1;
        return NULL;
    }
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
        return &test->emu->regs[(m&0x07)+(rex.b<<3)];
    } else {
        reg64_t* ret =  GetECommon(test->emu, addr, rex, m, delta);
        test->memsize = 4;
        test->memaddr = (uintptr_t)ret;
        memcpy(test->mem, ret, 10);
        return (reg64_t*)test->mem;
    }
}
reg64_t* TestEd8xw(x64test_t *test, int w, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta)
{
    if(rex.is67 && rex.is32bits) {
        printf_log(LOG_NONE, "Need 32bits 67 prefix TestEd4xw\n");
        test->emu->quit = 1;
        return NULL;
    }
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
        return &test->emu->regs[(m&0x07)+(rex.b<<3)];
    } else {
        reg64_t* ret =  GetECommon(test->emu, addr, rex, m, delta);
        test->memsize = 8<<w;
        test->memaddr = (uintptr_t)ret;
        *(uint64_t*)test->mem = ret->q[0];
        if(w)
            ((uint64_t*)test->mem)[1] = ret->q[1];
        return (reg64_t*)test->mem;
    }
}
reg64_t* TestEw(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta)
{
    if(rex.is67 && rex.is32bits) {
        printf_log(LOG_NONE, "Need 32bits 67 prefix TestEw\n");
        test->emu->quit = 1;
        return NULL;
    }
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
        return &test->emu->regs[(m&0x07)+(rex.b<<3)];
    } else {
        reg64_t* ret =  GetECommon(test->emu, addr, rex, m, delta);
        test->memsize = rex.w?8:2;
        test->memaddr = (uintptr_t)ret;
        if(rex.w)
            *(uint64_t*)test->mem = ret->q[0];
        else
            *(uint16_t*)test->mem = ret->word[0];
        return (reg64_t*)test->mem;
    }
}

uintptr_t GetEA32(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta)
{
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return (uintptr_t)&emu->regs[(m&0x07)+(rex.b<<3)];
    } else return (uintptr_t)GetECommon32O(emu, addr, rex, m, delta, 0);
}

uintptr_t GetEA32_16(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta)
{
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return (uintptr_t)&emu->regs[(m&0x07)+(rex.b<<3)];
    } else return (uintptr_t)GetECommon_16(emu, addr, m, rex.offset);
}

uintptr_t GetEA(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta)
{
    if(rex.is67 && rex.is32bits)
        return GetEA32_16(emu, addr, rex, v, delta);
    if(rex.is67)
        return GetEA32(emu, addr, rex, v, delta);
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return (uintptr_t)&emu->regs[(m&0x07)+(rex.b<<3)];
    } else return (uintptr_t)GetECommon(emu, addr, rex, m, delta);
}

#define GetEw GetEd
#define TestEw TestEd

reg64_t* GetEw16(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v)
{
    (void)rex;

    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return &emu->regs[(m&0x07)];
    } else {
        uintptr_t base = 0;
        switch(m&7) {
            case 0: base = R_BX+R_SI; break;
            case 1: base = R_BX+R_DI; break;
            case 2: base = R_BP+R_SI; break;
            case 3: base = R_BP+R_DI; break;
            case 4: base =      R_SI; break;
            case 5: base =      R_DI; break;
            case 6: base =      R_BP; break;
            case 7: base =      R_BX; break;
        }
        switch((m>>6)&3) {
            case 0: if((m&7)==6) base = F16S(addr); break;
            case 1: base += F8S(addr); break;
            case 2: base += F16S(addr); break;
            // case 3 is C0..C7, already dealt with
        }
        return (reg64_t*)(base+rex.offset);
    }
}

reg64_t* TestEw16(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v)
{
    (void)rex;
    x64emu_t* emu = test->emu;

    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
        return &emu->regs[(m&0x07)];
    } else {
        uintptr_t base = 0;
        switch(m&7) {
            case 0: base = R_BX+R_SI; break;
            case 1: base = R_BX+R_DI; break;
            case 2: base = R_BP+R_SI; break;
            case 3: base = R_BP+R_DI; break;
            case 4: base =      R_SI; break;
            case 5: base =      R_DI; break;
            case 6: base =      R_BP; break;
            case 7: base =      R_BX; break;
        }
        switch((m>>6)&3) {
            case 0: if((m&7)==6) base = F16S(addr); break;
            case 1: base += F8S(addr); break;
            case 2: base += F16S(addr); break;
            // case 3 is C0..C7, already dealt with
        }
        test->memsize = 2;
        *(uint16_t*)test->mem = *(uint16_t*)(base+rex.offset);
        test->memaddr = (uintptr_t)(base+rex.offset);
        return (reg64_t*)test->mem;
    }
}

mmx87_regs_t* GetEm(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta)
{
    if(rex.is67 && rex.is32bits) {
        printf_log(LOG_INFO, "Needed is67 32bits GetEm\n");
        emu->quit = 1;
        return NULL;
    }
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return &emu->mmx[m&0x07];
    } else return (mmx87_regs_t*)GetECommon(emu, addr, rex, m, delta);
}

mmx87_regs_t* TestEm(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta)
{
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
        return &test->emu->mmx[m&0x07];
    } else {
        mmx87_regs_t* ret = (mmx87_regs_t*)GetECommon(test->emu, addr, rex, m, delta);
        test->memsize = 8;
        *(uint64_t*)test->mem = ret->q;
        test->memaddr = (uintptr_t)ret;
        return (mmx87_regs_t*)test->mem;
    }
}

sse_regs_t* GetEx(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta)
{
    if(rex.is67 && rex.is32bits) {
        printf_log(LOG_INFO, "Needed is67 32bits GetEx\n");
        emu->quit = 1;
        return NULL;
    }
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return &emu->xmm[(m&0x07)+(rex.b<<3)];
    } else return (sse_regs_t*)GetECommon(emu, addr, rex, m, delta);
}

sse_regs_t* TestEx(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta, int sz)
{
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
        test->memsize=0;
        return &test->emu->xmm[(m&0x07)+(rex.b<<3)];
    } else {
        sse_regs_t* ret = (sse_regs_t*)GetECommon(test->emu, addr, rex, m, delta);
        test->memsize = sz;
        memcpy(test->mem, ret, sz);
        test->memaddr = (uintptr_t)ret;
        return (sse_regs_t*)test->mem;
    }
}
sse_regs_t* TestEy(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v)
{
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
        test->memsize=0;
        return &test->emu->ymm[(m&0x07)+(rex.b<<3)];
    } else {
        sse_regs_t* ret = (sse_regs_t*)(test->memaddr+16);
        test->memsize += 16;
        ((uint64_t*)test->mem)[2] = ret->q[0];
        ((uint64_t*)test->mem)[3] = ret->q[1];
        return (sse_regs_t*)&test->mem[16];
    }
}

reg64_t* GetGd(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v)
{
    return &emu->regs[((v&0x38)>>3)+(rex.r<<3)];
}

reg64_t* GetGb(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v)
{
    uint8_t m = (v&0x38)>>3;
    if(rex.rex)
        return &emu->regs[(m&7)+(rex.r<<3)];
    else
        return (reg64_t*)&emu->regs[m&3].byte[m>>2];
}

mmx87_regs_t* GetGm(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v)
{
    (void)rex;

    uint8_t m = (v&0x38)>>3;
    return &emu->mmx[m&7];
}

sse_regs_t* GetGx(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v)
{
    uint8_t m = (v&0x38)>>3;
    return &emu->xmm[(m&7)+(rex.r<<3)];
}

sse_regs_t* GetGy(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v)
{
    uint8_t m = (v&0x38)>>3;
    return &emu->ymm[(m&7)+(rex.r<<3)];
}
