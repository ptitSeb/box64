#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <wchar.h>
#include <sys/epoll.h>
#include <fts.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <pthread.h>

#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "myalign.h"
#include "debug.h"

static const int regs_abi[] = {_DI, _SI, _DX, _CX, _R8, _R9};

void myStackAlign(x64emu_t* emu, const char* fmt, uint64_t* st, uint64_t* mystack, int xmm, int pos)
{
    
    if(!fmt)
        return;
    // loop...
    const char* p = fmt;
    int state = 0;
    #ifndef HAVE_LD80BITS
    double d;
    long double ld;
    #endif
    int x = 0;
    while(*p)
    {
        switch(state) {
            case 0:
                switch(*p) {
                    case '%': state = 1; ++p; break;
                    default:
                        ++p;
                }
                break;
            case 1: // normal
            case 2: // l
            case 3: // ll
            case 4: // L
            case 5: // z
                switch(*p) {
                    case '%': state = 0;  ++p; break; //%% = back to 0
                    case 'l': ++state; if (state>3) state=3; ++p; break;
                    case 'L': state = 4; ++p; break;
                    case 'z': state = 5; ++p; break;
                    case 'a':
                    case 'A':
                    case 'e':
                    case 'E':
                    case 'g':
                    case 'G':
                    case 'F':
                    case 'f': state += 10; break;    //  float
                    case 'd':
                    case 'i':
                    case 'o':
                    case 'u':
                    case 'x':
                    case 'X': state += 20; break;   // int
                    case 'h': ++p; break;  // ignored...
                    case '\'':
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                    case '.': 
                    case '#':
                    case '+': 
                    case '-': ++p; break; // formating, ignored
                    case 'm': state = 0; ++p; break; // no argument
                    case 'n':
                    case 'p':
                    case 'S':
                    case 's': state = 30; break; // pointers
                    case '$': ++p; break; // should issue a warning, it's not handled...
                    case '*': 
                        if(pos<6)
                            *mystack = emu->regs[regs_abi[pos++]].q[0];
                        else {
                            *mystack = *st;
                            ++st;
                        }
                        ++mystack;
                        ++p; 
                        break; // fetch an int in the stack....
                    case ' ': state=0; ++p; break;
                    default:
                        state=20; // other stuff, put an int...
                }
                break;
            case 11:    //double
            case 12:    //%lg, still double
            case 13:    //%llg, still double
            case 15:    //%zg, meh.. double?
                if(xmm) {
                    *mystack = emu->xmm[x++].q[0];
                    --xmm;
                    mystack++;
                } else {
                    *mystack = *st;
                    st++; mystack++;
                }
                state = 0;
                ++p;
                break;
            case 14:    //%LG long double
                if((((uintptr_t)st)&0xf)!=0)
                    st++;
                #ifdef HAVE_LD80BITS
                if((((uintptr_t)mystack)&0xf)!=0)
                    mystack++;
                memcpy(mystack, st, 16);
                st+=2; mystack+=2;
                #elif defined(ANDROID)
                // there is 128bits long double on native and x64 side
                // need to align stacks to 128bits boundaries
                if((((uintptr_t)mystack)&0xf)!=0)
                    mystack++;
                if(xmm) {
                    memcpy(mystack, &emu->xmm[x++], 16);
                    st+=2; mystack+=2;
                    --xmm;
                } else {
                    if((((uintptr_t)st)&0xf)!=0)
                        st++;
                    memcpy(mystack, st, 16);
                    st+=2; mystack+=2;
                }
                #else
                // there is 128bits long double on ARM64, but they need 128bit alignment
                if((((uintptr_t)mystack)&0xf)!=0)
                    mystack++;
                LD2D((void*)st, &d);
                ld = d ;
                memcpy(mystack, &ld, 16);
                st+=2; mystack+=2;
                #endif
                state = 0;
                ++p;
                break;
            case 20:    // fallback
            case 21:
            case 22:
            case 23:    // 64bits int
            case 24:    // normal int / pointer
            case 25:    // size_t int
            case 30:
                if(pos<6)
                    *mystack = emu->regs[regs_abi[pos++]].q[0];
                else {
                    *mystack = *st;
                    ++st;
                }
                ++mystack;
                state = 0;
                ++p;
                break;
            default:
                // whaaaat?
                state = 0;
        }
    }
}

void myStackAlignScanf(x64emu_t* emu, const char* fmt, uint64_t* st, uint64_t* mystack, int pos)
{
    
    if(!fmt)
        return;
    // loop...
    const char* p = fmt;
    int state = 0;
    int ign = 0;
    while(*p)
    {
        switch(state) {
            case 0:
                ign = 0;
                switch(*p) {
                    case '%': state = 1; ++p; break;
                    default:
                        ++p;
                }
                break;
            case 1: // normal
            case 2: // l
            case 3: // ll
            case 4: // L
            case 5: // z
                switch(*p) {
                    case '%': state = 0;  ++p; break; //%% = back to 0
                    case 'l': ++state; if (state>3) state=3; ++p; break;
                    case 'L': state = 4; ++p; break;
                    case 'z': state = 5; ++p; break;
                    case 'a':
                    case 'A':
                    case 'e':
                    case 'E':
                    case 'g':
                    case 'G':
                    case 'F':
                    case 'f': state += 10; break;    //  float
                    case 'd':
                    case 'i':
                    case 'o':
                    case 'u':
                    case 'x':
                    case 'X': state += 20; break;   // int
                    case 'h': ++p; break;  // ignored...
                    case '\'':
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                    case '.': 
                    case '#':
                    case '+': 
                    case '-': ++p; break; // formating, ignored
                    case 'm': state = 0; ++p; break; // no argument
                    case 'n':
                    case 'p':
                    case 'S':
                    case 's': state = 30; break; // pointers
                    case '$': ++p; break; // should issue a warning, it's not handled...
                    case '*': ign=1; ++p; break; // ignore arg
                    case ' ': state=0; ++p; break;
                    default:
                        state=20; // other stuff, put an int...
                }
                break;
            case 11:    //double
            case 12:    //%lg, still double
            case 13:    //%llg, still double
            case 14:    //%Lg long double
            case 15:    //%zg
            case 20:    // fallback
            case 21:
            case 22:
            case 23:    // 64bits int
            case 24:    // normal int / pointer
            case 25:    // size_t int
            case 30:
                if(!ign) {
                    if(pos<6)
                        *mystack = emu->regs[regs_abi[pos++]].q[0];
                    else {
                        *mystack = *st;
                        ++st;
                    }
                    ++mystack;
                }
                state = 0;
                ++p;
                break;
            default:
                // whaaaat?
                state = 0;
        }
    }
}

void myStackAlignW(x64emu_t* emu, const char* fmt, uint64_t* st, uint64_t* mystack, int xmm, int pos)
{
    // loop...
    const wchar_t* p = (const wchar_t*)fmt;
    int state = 0;
    #ifndef HAVE_LD80BITS
    double d;
    long double ld;
    #endif
    int x = 0;
    while(*p)
    {
        switch(state) {
            case 0:
                switch(*p) {
                    case '%': state = 1; ++p; break;
                    default:
                        ++p;
                }
                break;
            case 1: // normal
            case 2: // l
            case 3: // ll
            case 4: // L
            case 5: // z
                switch(*p) {
                    case '%': state = 0;  ++p; break; //%% = back to 0
                    case 'l': ++state; if (state>3) state=3; ++p; break;
                    case 'L': state = 4; ++p; break;
                    case 'z': state = 5; ++p; break;
                    case 'a':
                    case 'A':
                    case 'e':
                    case 'E':
                    case 'g':
                    case 'G':
                    case 'F':
                    case 'f': state += 10; break;    //  float
                    case 'd':
                    case 'i':
                    case 'o':
                    case 'u':
                    case 'x':
                    case 'X': state += 20; break;   // int
                    case 'h': ++p; break;  // ignored...
                    case '\'':
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                    case '.': 
                    case '#':
                    case '+': 
                    case '-': ++p; break; // formating, ignored
                    case 'm': state = 0; ++p; break; // no argument
                    case 'n':
                    case 'p':
                    case 'S':
                    case 's': state = 30; break; // pointers
                    case '$': ++p; break; // should issue a warning, it's not handled...
                    case '*': 
                        if(pos<6)
                            *mystack = emu->regs[regs_abi[pos++]].q[0];
                        else {
                            *mystack = *st;
                            ++st;
                        }
                        ++mystack;
                        ++p; 
                        break; // fetch an int in the stack....
                    case ' ': state=0; ++p; break;
                    default:
                        state=20; // other stuff, put an int...
                }
                break;
            case 11:    //double
            case 12:    //%lg, still double
            case 13:    //%llg, still double
            case 15:    //%zg, meh .. double
                if(xmm) {
                    *mystack = emu->xmm[x++].q[0];
                    --xmm;
                    mystack++;
                } else {
                    *mystack = *st;
                    st++; mystack++;
                }
                state = 0;
                ++p;
                break;
            case 14:    //%LG long double
                if((((uintptr_t)st)&0xf)!=0)
                    st++;
                #ifdef HAVE_LD80BITS
                if((((uintptr_t)mystack)&0xf)!=0)
                    mystack++;
                memcpy(mystack, st, 16);
                st+=2; mystack+=2;
                #elif defined(ANDROID)
                // there is 128bits long double on native and x64 side
                // need to align stacks to 128bits boundaries
                if((((uintptr_t)mystack)&0xf)!=0)
                    mystack++;
                if(xmm) {
                    memcpy(mystack, &emu->xmm[x++], 16);
                    st+=2; mystack+=2;
                    --xmm;
                } else {
                    if((((uintptr_t)st)&0xf)!=0)
                        st++;
                    memcpy(mystack, st, 16);
                    st+=2; mystack+=2;
                }
                #else
                // there is 128bits long double on ARM64, but they need 128bit alignment
                if((((uintptr_t)mystack)&0xf)!=0)
                    mystack++;
                LD2D((void*)st, &d);
                ld = d ;
                memcpy(mystack, &ld, 16);
                st+=2; mystack+=2;
                #endif
                state = 0;
                ++p;
                break;
            case 20:    // fallback
            case 21:
            case 22:
            case 23:    // 64bits int
            case 24:    // normal int / pointer
            case 25:    // size_t int
            case 30:
                if(pos<6)
                    *mystack = emu->regs[regs_abi[pos++]].q[0];
                else {
                    *mystack = *st;
                    ++st;
                }
                ++mystack;
                state = 0;
                ++p;
                break;
            default:
                // whaaaattt?
                state = 0;
        }
    }
}

void myStackAlignScanfW(x64emu_t* emu, const char* fmt, uint64_t* st, uint64_t* mystack, int pos)
{
    
    if(!fmt)
        return;
    // loop...
    const wchar_t* p = (const wchar_t*)fmt;
    int state = 0;
    int ign = 0;
    while(*p)
    {
        switch(state) {
            case 0:
                ign = 0;
                switch(*p) {
                    case '%': state = 1; ++p; break;
                    default:
                        ++p;
                }
                break;
            case 1: // normal
            case 2: // l
            case 3: // ll
            case 4: // L
            case 5: // z
                switch(*p) {
                    case '%': state = 0;  ++p; break; //%% = back to 0
                    case 'l': ++state; if (state>3) state=3; ++p; break;
                    case 'L': state = 4; ++p; break;
                    case 'z': state = 5; ++p; break;
                    case 'a':
                    case 'A':
                    case 'e':
                    case 'E':
                    case 'g':
                    case 'G':
                    case 'F':
                    case 'f': state += 10; break;    //  float
                    case 'd':
                    case 'i':
                    case 'o':
                    case 'u':
                    case 'x':
                    case 'X': state += 20; break;   // int
                    case 'h': ++p; break;  // ignored...
                    case '\'':
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                    case '.': 
                    case '#':
                    case '+': 
                    case '-': ++p; break; // formating, ignored
                    case 'm': state = 0; ++p; break; // no argument
                    case 'n':
                    case 'p':
                    case 'S':
                    case 's': state = 30; break; // pointers
                    case '$': ++p; break; // should issue a warning, it's not handled...
                    case '*': ign = 1; ++p; break; // ignore arg
                    case ' ': state=0; ++p; break;
                    default:
                        state=20; // other stuff, put an int...
                }
                break;
            case 11:    //double
            case 12:    //%lg, still double
            case 13:    //%llg, still double
            case 14:    //%Lg long double
            case 15:    //%zg
            case 20:    // fallback
            case 21:
            case 22:
            case 23:    // 64bits int
            case 24:    // normal int / pointer
            case 25:    // size_t int
            case 30:
                if(!ign) {
                    if(pos<6)
                        *mystack = emu->regs[regs_abi[pos++]].q[0];
                    else {
                        *mystack = *st;
                        ++st;
                    }
                    ++mystack;
                }
                state = 0;
                ++p;
                break;
            default:
                // whaaaat?
                state = 0;
        }
    }
}


#undef st_atime
#undef st_mtime
#undef st_ctime

void UnalignStat64(const void* source, void* dest)
{
    struct x64_stat64 *x64st = (struct x64_stat64*)dest;
    struct stat *st = (struct stat*) source;
    
    x64st->__pad0 = 0;
    memset(x64st->__glibc_reserved, 0, sizeof(x64st->__glibc_reserved));
    x64st->st_dev      = st->st_dev;
    x64st->st_ino      = st->st_ino;
    x64st->st_mode     = st->st_mode;
    x64st->st_nlink    = st->st_nlink;
    x64st->st_uid      = st->st_uid;
    x64st->st_gid      = st->st_gid;
    x64st->st_rdev     = st->st_rdev;
    x64st->st_size     = st->st_size;
    x64st->st_blksize  = st->st_blksize;
    x64st->st_blocks   = st->st_blocks;
    x64st->st_atim     = st->st_atim;
    x64st->st_mtim     = st->st_mtim;
    x64st->st_ctim     = st->st_ctim;
}

void AlignStat64(const void* source, void* dest)
{
    struct stat *st = (struct stat*) dest;
    struct x64_stat64 *x64st = (struct x64_stat64*)source;
    
    st->st_dev      = x64st->st_dev;
    st->st_ino      = x64st->st_ino;
    st->st_mode     = x64st->st_mode;
    st->st_nlink    = x64st->st_nlink;
    st->st_uid      = x64st->st_uid;
    st->st_gid      = x64st->st_gid;
    st->st_rdev     = x64st->st_rdev;
    st->st_size     = x64st->st_size;
    st->st_blksize  = x64st->st_blksize;
    st->st_blocks   = x64st->st_blocks;
    st->st_atim     = x64st->st_atim;
    st->st_mtim     = x64st->st_mtim;
    st->st_ctim     = x64st->st_ctim;
}

struct __attribute__((packed)) x64_epoll_event {
    uint32_t            events;
    uint64_t            data;
};
// Arm -> x64
void UnalignEpollEvent(void* dest, void* source, int nbr)
{
    struct x64_epoll_event *x64_struct = (struct x64_epoll_event*)dest;
    struct epoll_event *arm_struct = (struct epoll_event*)source;
    while(nbr) {
        x64_struct->events = arm_struct->events;
        x64_struct->data = arm_struct->data.u64;
        ++x64_struct;
        ++arm_struct;
        --nbr;
    }
}

// x64 -> Arm
void AlignEpollEvent(void* dest, void* source, int nbr)
{
    struct x64_epoll_event *x64_struct = (struct x64_epoll_event*)source;
    struct epoll_event *arm_struct = (struct epoll_event*)dest;
    while(nbr) {
        arm_struct->events = x64_struct->events;
        arm_struct->data.u64 = x64_struct->data;
        ++x64_struct;
        ++arm_struct;
        --nbr;
    }
}

struct __attribute__((packed)) x64_semid_ds {
    struct ipc_perm sem_perm;
    time_t sem_otime;
    unsigned long _reserved1;
    time_t sem_ctime;
    unsigned long _reserved2;
    unsigned long sem_nsems;
    unsigned long _reserved3;
    unsigned long _reserved4;
};

void UnalignSemidDs(void *dest, const void* source)
{
    struct x64_semid_ds *x64_struct = (struct x64_semid_ds*)dest;
    const struct semid_ds *arm_struct = (const struct semid_ds*)source;

    x64_struct->sem_perm = arm_struct->sem_perm;
    x64_struct->sem_otime = arm_struct->sem_otime;
    x64_struct->sem_ctime = arm_struct->sem_ctime;
    x64_struct->sem_nsems = arm_struct->sem_nsems;
}

void AlignSemidDs(void *dest, const void* source)
{
    const struct x64_semid_ds *x64_struct = (const struct x64_semid_ds*)source;
    struct semid_ds *arm_struct = (struct semid_ds*)dest;

    arm_struct->sem_perm = x64_struct->sem_perm;
    arm_struct->sem_otime = x64_struct->sem_otime;
    arm_struct->sem_ctime = x64_struct->sem_ctime;
    arm_struct->sem_nsems = x64_struct->sem_nsems;
}

uintptr_t getVArgs(x64emu_t* emu, int pos, uintptr_t* b, int N)
{
    if((pos+N)>5)
        return b[pos+N-6];
    return emu->regs[regs_abi[pos+N]].q[0];
}

void setVArgs(x64emu_t* emu, int pos, uintptr_t* b, int N, uintptr_t a)
{
    if((pos+N)>5)
        b[pos+N-6] = a;
    else
        emu->regs[regs_abi[pos+N]].q[0] = a;
}

#ifndef CONVERT_VALIST
void myStackAlignValist(x64emu_t* emu, const char* fmt, uint64_t* mystack, x64_va_list_t va)
{
    if(!fmt)
        return;
    // loop...
    const char* p = fmt;
    int state = 0;
    #ifndef HAVE_LD80BITS
    double d;
    long double ld;
    #endif
    int x = 0;
    uintptr_t *area = (uintptr_t*)va->reg_save_area;    // the direct registers copy
    uintptr_t *st = (uintptr_t*)va->overflow_arg_area;  // the stack arguments
    uintptr_t gprs = va->gp_offset;
    uintptr_t fprs = va->fp_offset;
    while(*p)
    {
        switch(state) {
            case 0:
                switch(*p) {
                    case '%': state = 1; ++p; break;
                    default:
                        ++p;
                }
                break;
            case 1: // normal
            case 2: // l
            case 3: // ll
            case 4: // L
            case 5: // z
                switch(*p) {
                    case '%': state = 0;  ++p; break; //%% = back to 0
                    case 'l': ++state; if (state>3) state=3; ++p; break;
                    case 'L': state = 4; ++p; break;
                    case 'z': state = 5; ++p; break;
                    case 'a':
                    case 'A':
                    case 'e':
                    case 'E':
                    case 'g':
                    case 'G':
                    case 'F':
                    case 'f': state += 10; break;    //  float
                    case 'd':
                    case 'i':
                    case 'o':
                    case 'u':
                    case 'x':
                    case 'X': state += 20; break;   // int
                    case 'h': ++p; break;  // ignored...
                    case '\'':
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                    case '.': 
                    case '+': 
                    case '-': ++p; break; // formating, ignored
                    case 'm': state = 0; ++p; break; // no argument
                    case 'n':
                    case 'p':
                    case 'S':
                    case 's': state = 30; break; // pointers
                    case '$': ++p; break; // should issue a warning, it's not handled...
                    case '*': 
                        if(gprs<X64_VA_MAX_REG) {
                            *mystack = area[gprs/8];
                            gprs+=8;
                        } else {
                            *mystack = *st;
                            ++st;
                        }
                        ++mystack;
                        ++p; 
                        break; // fetch an int in the stack....
                    case ' ': state=0; ++p; break;
                    default:
                        state=20; // other stuff, put an int...
                }
                break;
            case 11:    //double
            case 12:    //%lg, still double
            case 13:    //%llg, still double
            case 15:    //%zg, meh.. double?
                if(fprs<X64_VA_MAX_XMM) {
                    *mystack = area[fprs/8];
                    fprs+=16;
                    mystack++;
                } else {
                    *mystack = *st;
                    st++; mystack++;
                }
                state = 0;
                ++p;
                break;
            case 14:    //%LG long double
                if((((uintptr_t)st)&0xf)!=0)
                    st++;
                #ifdef HAVE_LD80BITS
                if((((uintptr_t)mystack)&0xf)!=0)
                    mystack++;
                memcpy(mystack, st, 16);
                st+=2; mystack+=2;
                #elif defined(ANDROID)
                // there is 128bits long double on native and x64 side
                // need to align stacks to 128bits boundaries
                if((((uintptr_t)mystack)&0xf)!=0)
                    mystack++;
                if(fprs<X64_VA_MAX_XMM) {
                    memcpy(mystack, &area[fprs/8], 16);
                    fprs+=16; mystack+=2;
                } else {
                    if((((uintptr_t)st)&0xf)!=0)
                        st++;
                    memcpy(mystack, st, 16);
                    st+=2; mystack+=2;
                }
                #else
                // there is 128bits long double on ARM64, but they need 128bit alignment
                if((((uintptr_t)mystack)&0xf)!=0)
                    mystack++;
                LD2D((void*)st, &d);
                ld = d ;
                memcpy(mystack, &ld, 16);
                st+=2; mystack+=2;
                #endif
                state = 0;
                ++p;
                break;
            case 20:    // fallback
            case 21:
            case 22:
            case 23:    // 64bits int
            case 24:    // normal int / pointer
            case 25:    // size_t int
            case 30:
                if(gprs<X64_VA_MAX_REG) {
                    *mystack = area[gprs/8];
                    gprs+=8;
                } else {
                    *mystack = *st;
                    ++st;
                }
                ++mystack;
                state = 0;
                ++p;
                break;
            default:
                // whaaaat?
                state = 0;
        }
    }
}

void myStackAlignWValist(x64emu_t* emu, const char* fmt, uint64_t* mystack, x64_va_list_t va)
{
    // loop...
    const wchar_t* p = (const wchar_t*)fmt;
    int state = 0;
    #ifndef HAVE_LD80BITS
    double d;
    long double ld;
    #endif
    int x = 0;
    uintptr_t *area = (uintptr_t*)va->reg_save_area;    // the direct registers copy
    uintptr_t *st = (uintptr_t*)va->overflow_arg_area;  // the stack arguments
    uintptr_t gprs = va->gp_offset;
    uintptr_t fprs = va->fp_offset;
    while(*p)
    {
        switch(state) {
            case 0:
                switch(*p) {
                    case '%': state = 1; ++p; break;
                    default:
                        ++p;
                }
                break;
            case 1: // normal
            case 2: // l
            case 3: // ll
            case 4: // L
            case 5: // z
                switch(*p) {
                    case '%': state = 0;  ++p; break; //%% = back to 0
                    case 'l': ++state; if (state>3) state=3; ++p; break;
                    case 'L': state = 4; ++p; break;
                    case 'z': state = 5; ++p; break;
                    case 'a':
                    case 'A':
                    case 'e':
                    case 'E':
                    case 'g':
                    case 'G':
                    case 'F':
                    case 'f': state += 10; break;    //  float
                    case 'd':
                    case 'i':
                    case 'o':
                    case 'u':
                    case 'x':
                    case 'X': state += 20; break;   // int
                    case 'h': ++p; break;  // ignored...
                    case '\'':
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                    case '.': 
                    case '+': 
                    case '-': ++p; break; // formating, ignored
                    case 'm': state = 0; ++p; break; // no argument
                    case 'n':
                    case 'p':
                    case 'S':
                    case 's': state = 30; break; // pointers
                    case '$': ++p; break; // should issue a warning, it's not handled...
                    case '*': 
                        if(gprs<X64_VA_MAX_REG) {
                            *mystack = area[gprs/8];
                            gprs+=8;
                        } else {
                            *mystack = *st;
                            ++st;
                        }
                        ++mystack;
                        ++p; 
                        break; // fetch an int in the stack....
                    case ' ': state=0; ++p; break;
                    default:
                        state=20; // other stuff, put an int...
                }
                break;
            case 11:    //double
            case 12:    //%lg, still double
            case 13:    //%llg, still double
            case 15:    //%zg, meh .. double
                if(fprs<X64_VA_MAX_XMM) {
                    *mystack = area[fprs/8];
                    fprs+=16;
                    mystack++;
                } else {
                    *mystack = *st;
                    st++; mystack++;
                }
                state = 0;
                ++p;
                break;
            case 14:    //%LG long double
                if((((uintptr_t)st)&0xf)!=0)
                    st++;
                #ifdef HAVE_LD80BITS
                if((((uintptr_t)mystack)&0xf)!=0)
                    mystack++;
                memcpy(mystack, st, 16);
                st+=2; mystack+=2;
                #elif defined(ANDROID)
                // there is 128bits long double on native and x64 side
                // need to align stacks to 128bits boundaries
                if((((uintptr_t)mystack)&0xf)!=0)
                    mystack++;
                if(fprs<X64_VA_MAX_XMM) {
                    memcpy(mystack, &area[fprs/8], 16);
                    fprs+=16; mystack+=2;
                } else {
                    if((((uintptr_t)st)&0xf)!=0)
                        st++;
                    memcpy(mystack, st, 16);
                    st+=2; mystack+=2;
                }
                #else
                // there is 128bits long double on ARM64, but they need 128bit alignment
                if((((uintptr_t)mystack)&0xf)!=0)
                    mystack++;
                LD2D((void*)st, &d);
                ld = d ;
                memcpy(mystack, &ld, 16);
                st+=2; mystack+=2;
                #endif
                state = 0;
                ++p;
                break;
            case 20:    // fallback
            case 21:
            case 22:
            case 23:    // 64bits int
            case 24:    // normal int / pointer
            case 25:    // size_t int
            case 30:
                if(gprs<X64_VA_MAX_REG) {
                    *mystack = area[gprs/8];
                    gprs+=8;
                } else {
                    *mystack = *st;
                    ++st;
                }
                ++mystack;
                state = 0;
                ++p;
                break;
            default:
                // whaaaattt?
                state = 0;
        }
    }
}

void myStackAlignScanfValist(x64emu_t* emu, const char* fmt, uint64_t* mystack, x64_va_list_t va)
{
    
    if(!fmt)
        return;
    // loop...
    const char* p = fmt;
    int state = 0;
    int ign = 0;
    uintptr_t *area = (uintptr_t*)va->reg_save_area;    // the direct registers copy
    uintptr_t *st = (uintptr_t*)va->overflow_arg_area;  // the stack arguments
    uintptr_t gprs = va->gp_offset;
    uintptr_t fprs = va->fp_offset;
    while(*p)
    {
        switch(state) {
            case 0:
                ign = 0;
                switch(*p) {
                    case '%': state = 1; ++p; break;
                    default:
                        ++p;
                }
                break;
            case 1: // normal
            case 2: // l
            case 3: // ll
            case 4: // L
            case 5: // z
                switch(*p) {
                    case '%': state = 0;  ++p; break; //%% = back to 0
                    case 'l': ++state; if (state>3) state=3; ++p; break;
                    case 'L': state = 4; ++p; break;
                    case 'z': state = 5; ++p; break;
                    case 'a':
                    case 'A':
                    case 'e':
                    case 'E':
                    case 'g':
                    case 'G':
                    case 'F':
                    case 'f': state += 10; break;    //  float
                    case 'd':
                    case 'i':
                    case 'o':
                    case 'u':
                    case 'x':
                    case 'X': state += 20; break;   // int
                    case 'h': ++p; break;  // ignored...
                    case '\'':
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                    case '.': 
                    case '+': 
                    case '-': ++p; break; // formating, ignored
                    case 'm': state = 0; ++p; break; // no argument
                    case 'n':
                    case 'p':
                    case 'S':
                    case 's': state = 30; break; // pointers
                    case '$': ++p; break; // should issue a warning, it's not handled...
                    case '*': ign=1; ++p; break; // ignore arg
                    case ' ': state=0; ++p; break;
                    default:
                        state=20; // other stuff, put an int...
                }
                break;
            case 11:    //double
            case 12:    //%lg, still double
            case 13:    //%llg, still double
            case 14:    //%Lg long double
            case 15:    //%zg
            case 20:    // fallback
            case 21:
            case 22:
            case 23:    // 64bits int
            case 24:    // normal int / pointer
            case 25:    // size_t int
            case 30:
                if(!ign) {
                    if(gprs<X64_VA_MAX_REG) {
                        *mystack = area[gprs/8];
                        gprs+=8;
                    } else {
                        *mystack = *st;
                        ++st;
                    }
                    ++mystack;
                }
                state = 0;
                ++p;
                break;
            default:
                // whaaaat?
                state = 0;
        }
    }
}

void myStackAlignScanfWValist(x64emu_t* emu, const char* fmt, uint64_t* mystack, x64_va_list_t va)
{
    
    if(!fmt)
        return;
    // loop...
    const wchar_t* p = (const wchar_t*)fmt;
    int state = 0;
    int ign = 0;
    uintptr_t *area = (uintptr_t*)va->reg_save_area;    // the direct registers copy
    uintptr_t *st = (uintptr_t*)va->overflow_arg_area;  // the stack arguments
    uintptr_t gprs = va->gp_offset;
    uintptr_t fprs = va->fp_offset;
    while(*p)
    {
        switch(state) {
            case 0:
                ign = 0;
                switch(*p) {
                    case '%': state = 1; ++p; break;
                    default:
                        ++p;
                }
                break;
            case 1: // normal
            case 2: // l
            case 3: // ll
            case 4: // L
            case 5: // z
                switch(*p) {
                    case '%': state = 0;  ++p; break; //%% = back to 0
                    case 'l': ++state; if (state>3) state=3; ++p; break;
                    case 'L': state = 4; ++p; break;
                    case 'z': state = 5; ++p; break;
                    case 'a':
                    case 'A':
                    case 'e':
                    case 'E':
                    case 'g':
                    case 'G':
                    case 'F':
                    case 'f': state += 10; break;    //  float
                    case 'd':
                    case 'i':
                    case 'o':
                    case 'u':
                    case 'x':
                    case 'X': state += 20; break;   // int
                    case 'h': ++p; break;  // ignored...
                    case '\'':
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                    case '.': 
                    case '+': 
                    case '-': ++p; break; // formating, ignored
                    case 'm': state = 0; ++p; break; // no argument
                    case 'n':
                    case 'p':
                    case 'S':
                    case 's': state = 30; break; // pointers
                    case '$': ++p; break; // should issue a warning, it's not handled...
                    case '*': ign = 1; ++p; break; // ignore arg
                    case ' ': state=0; ++p; break;
                    default:
                        state=20; // other stuff, put an int...
                }
                break;
            case 11:    //double
            case 12:    //%lg, still double
            case 13:    //%llg, still double
            case 14:    //%Lg long double
            case 15:    //%zg
            case 20:    // fallback
            case 21:
            case 22:
            case 23:    // 64bits int
            case 24:    // normal int / pointer
            case 25:    // size_t int
            case 30:
                if(!ign) {
                    if(gprs<X64_VA_MAX_REG) {
                        *mystack = area[gprs/8];
                        gprs+=8;
                    } else {
                        *mystack = *st;
                        ++st;
                    }
                    ++mystack;
                }
                state = 0;
                ++p;
                break;
            default:
                // whaaaat?
                state = 0;
        }
    }
}

void myStackAlignGVariantNewVa(x64emu_t* emu, const char* fmt, uint64_t* scratch, x64_va_list_t* b)
{
    uint64_t* grp = (uint64_t*)((*b)->reg_save_area);
    uint64_t* frp = (uint64_t*)((*b)->reg_save_area + X64_VA_MAX_REG);

    int idx = 0;
    int gr_offs = ((*b)->gp_offset) / 8;
    int fr_offs = ((*b)->fp_offset - X64_VA_MAX_REG) / 8;

    int oa_offs = 0;
    const char *p = fmt;
    int state = 0;
    int inblocks = 0;
    int tmp;

    do {
        switch(state) {
            case 0: // Nothing
                switch(*p) {
                    case 'b': // gboolean
                    case 'y': // guchar
                    case 'n': // gint16
                    case 'q': // guint16
                    case 'i': // gint32
                    case 'u': // guint32
                    case 'h': // gint32
                    case 's': // const gchar*
                    case 'o':
                    case 'g':
                    case 'v': // GVariant*
                    case '*': // GVariant* of any type
                    case '?': // GVariant* of basic type
                    case 'r': // GVariant* of tuple type
                    case 'x': // gint64
                    case 't': // guint64
                        if (gr_offs >= (X64_VA_MAX_REG / 8))
                            scratch[idx++] = ((uint64_t*)((*b)->overflow_arg_area))[oa_offs++];
                        else
                            scratch[idx++] = grp[gr_offs++];
                        break;
                    case 'd': // gdouble
                        if (fr_offs >= ((X64_VA_MAX_XMM - X64_VA_MAX_REG) / 8)) {
                            scratch[idx++] = ((uint64_t*)((*b)->overflow_arg_area))[oa_offs++];
                        } else {
                            scratch[idx++] = frp[fr_offs];
                            fr_offs += 2;
                        }
                        break;
                    case '{':
                    case '(': ++inblocks; break;
                    case '}':
                    case ')': --inblocks; break;
                    case 'a': state = 1; break; // GVariantBuilder* or GVariantIter**
                    case 'm': state = 2; break; // maybe types
                    case '@': state = 3; break; // GVariant* of type [type]
                    case '^': state = 4; break; // pointer value
                    case '&': break; // pointer: do nothing
                }
                break;
            case 1: // Arrays
                switch(*p) {
                    case '{':
                    case '(': ++tmp; break;
                    case '}':
                    case ')': --tmp; break;
                }
                if (*p == 'a') break;
                if (tmp == 0) {
                    if (gr_offs >= (X64_VA_MAX_REG / 8))
                        scratch[idx++] = ((uint64_t*)((*b)->overflow_arg_area))[oa_offs++];
                    else
                        scratch[idx++] = grp[gr_offs++];
                    state = 0;
                }
                break;
            case 2: // Maybe-types
                switch(*p) {
                    case 'b': // gboolean
                    case 'y': // guchar
                    case 'n': // gint16
                    case 'q': // guint16
                    case 'i': // gint32
                    case 'u': // guint32
                    case 'h': // gint32
                    case 'x': // gint64
                    case 't': // guint64
                    case 'd': // gdouble
                    case '{':
                    case '}':
                    case '(':
                    case ')':
                        // Add a gboolean or gboolean*, no char increment
                        if (gr_offs >= (X64_VA_MAX_REG / 8))
                            scratch[idx++] = ((uint64_t*)((*b)->overflow_arg_area))[oa_offs++];
                        else
                            scratch[idx++] = grp[gr_offs++];
                        --p;
                        state = 0;
                        break;
                    case 'a': // GVariantBuilder* or GVariantIter**
                    case 's': // const gchar*
                    case 'o':
                    case 'g':
                    case 'v': // GVariant*
                    case '@': // GVariant* of type [type]
                    case '*': // GVariant* of any type
                    case '?': // GVariant* of basic type
                    case 'r': // GVariant* of tuple type
                    case '&': // pointer
                    case '^': // pointer value
                        // Just maybe-NULL
                        --p;
                        state = 0;
                        break;

                    default: // Default to add a gboolean & reinit state?
                        if (gr_offs >= (X64_VA_MAX_REG / 8))
                            scratch[idx++] = ((uint64_t*)((*b)->overflow_arg_area))[oa_offs++];
                        else
                            scratch[idx++] = grp[gr_offs++];
                        --p;
                        state = 0;
                }
                break;
            case 3: // GVariant*
                switch(*p) {
                    case '{':
                    case '(': ++tmp; break;
                    case '}':
                    case ')': --tmp; break;
                    case 'a': // GVariantBuilder* or GVariantIter**
                        do { ++p; } while(*p == 'a'); // Use next character which is not an array (array definition)
                        switch(*p) {
                            case '{':
                            case '(': ++tmp; break;
                            case '}':
                            case ')': --tmp; break;
                        }
                        break;
                }
                if (tmp == 0) {
                    if (gr_offs >= (X64_VA_MAX_REG / 8))
                        scratch[idx++] = ((uint64_t*)((*b)->overflow_arg_area))[oa_offs++];
                    else
                        scratch[idx++] = grp[gr_offs++];
                    state = 0;
                }
                break;
            case 4: // ^
                if (*p == 'a') state = 5;
                else if (*p == '&') state = 8;
                else state = 0; //???
                break;
            case 5: // ^a
                if ((*p == 's') || (*p == 'o') || (*p == 'y')) {
                    if (gr_offs >= (X64_VA_MAX_REG / 8))
                        scratch[idx++] = ((uint64_t*)((*b)->overflow_arg_area))[oa_offs++];
                    else
                        scratch[idx++] = grp[gr_offs++];
                    state = 0;
                } else if (*p == '&') state = 6;
                else if (*p == 'a') state = 7;
                else state = 0; //???
                break;
            case 6: // ^a&
                if ((*p == 's') || (*p == 'o')) {
                    if (gr_offs >= (X64_VA_MAX_REG / 8))
                        scratch[idx++] = ((uint64_t*)((*b)->overflow_arg_area))[oa_offs++];
                    else
                        scratch[idx++] = grp[gr_offs++];
                    state = 0;
                } else if (*p == 'a') state = 7;
                else state = 0; //???
                break;
            case 7: // ^aa / ^a&a
                if (*p == 'y') {
                    if (gr_offs >= (X64_VA_MAX_REG / 8))
                        scratch[idx++] = ((uint64_t*)((*b)->overflow_arg_area))[oa_offs++];
                    else
                        scratch[idx++] = grp[gr_offs++];
                    state = 0;
                } else state = 0; //???
                break;
            case 8: // ^&
                if (*p == 'a') state = 9;
                else state = 0; //???
                break;
            case 9: // ^&a
                if (*p == 'y') {
                    if (gr_offs >= (X64_VA_MAX_REG / 8))
                        scratch[idx++] = ((uint64_t*)((*b)->overflow_arg_area))[oa_offs++];
                    else
                        scratch[idx++] = grp[gr_offs++];
                    state = 0;
                } else state = 0; //???
                break;
        }
        ++p;
    } while (*p && (inblocks || state));                    
}

void myStackAlignGVariantNew(x64emu_t* emu, const char* fmt, uint64_t* st, uint64_t* mystack, int xmm)
{
    if (!fmt)
        return;

    int x = 0;
    const char *p = fmt;
    int state = 0;
    int inblocks = 0;
    int tmp;

    do {
        switch(state) {
            case 0: // Nothing
                switch(*p) {
                    case 'b': // gboolean
                    case 'y': // guchar
                    case 'n': // gint16
                    case 'q': // guint16
                    case 'i': // gint32
                    case 'u': // guint32
                    case 'h': // gint32
                    case 's': // const gchar*
                    case 'o':
                    case 'g':
                    case 'v': // GVariant*
                    case '*': // GVariant* of any type
                    case '?': // GVariant* of basic type
                    case 'r': // GVariant* of tuple type
                    case 'x': // gint64
                    case 't': // guint64
                        *mystack = *st;
                        ++mystack;
                        ++st;
                        break;
                    case 'd': // gdouble
                        if (xmm) {
                            *mystack = emu->xmm[x++].q[0];
                            --xmm;
                            mystack++;
                        } else {
                            *mystack = *st;
                            st++;
                            mystack++;
                        }
                        break;
                    case '{':
                    case '(': ++inblocks; break;
                    case '}':
                    case ')': --inblocks; break;
                    case 'a': state = 1; break; // GVariantBuilder* or GVariantIter**
                    case 'm': state = 2; break; // maybe types
                    case '@': state = 3; break; // GVariant* of type [type]
                    case '^': state = 4; break; // pointer value
                    case '&': break; // pointer: do nothing
                }
                break;
            case 1: // Arrays
                switch(*p) {
                    case '{':
                    case '(': ++tmp; break;
                    case '}':
                    case ')': --tmp; break;
                }
                if (*p == 'a') break;
                if (tmp == 0) {
                    *mystack = *st;
                    ++mystack;
                    ++st;
                    state = 0;
                }
                break;
            case 2: // Maybe-types
                switch(*p) {
                    case 'b': // gboolean
                    case 'y': // guchar
                    case 'n': // gint16
                    case 'q': // guint16
                    case 'i': // gint32
                    case 'u': // guint32
                    case 'h': // gint32
                    case 'x': // gint64
                    case 't': // guint64
                    case 'd': // gdouble
                    case '{':
                    case '}':
                    case '(':
                    case ')':
                        // Add a gboolean or gboolean*, no char increment
                        *mystack = *st;
                        ++mystack;
                        ++st;
                        --p;
                        state = 0;
                        break;
                    case 'a': // GVariantBuilder* or GVariantIter**
                    case 's': // const gchar*
                    case 'o':
                    case 'g':
                    case 'v': // GVariant*
                    case '@': // GVariant* of type [type]
                    case '*': // GVariant* of any type
                    case '?': // GVariant* of basic type
                    case 'r': // GVariant* of tuple type
                    case '&': // pointer
                    case '^': // pointer value
                        // Just maybe-NULL
                        --p;
                        state = 0;
                        break;

                    default: // Default to add a gboolean & reinit state?
                        *mystack = *st;
                        ++mystack;
                        ++st;
                        --p;
                        state = 0;
                }
                break;
            case 3: // GVariant*
                switch(*p) {
                    case '{':
                    case '(': ++tmp; break;
                    case '}':
                    case ')': --tmp; break;
                    case 'a': // GVariantBuilder* or GVariantIter**
                        do { ++p; } while(*p == 'a'); // Use next character which is not an array (array definition)
                        switch(*p) {
                            case '{':
                            case '(': ++tmp; break;
                            case '}':
                            case ')': --tmp; break;
                        }
                        break;
                }
                if (tmp == 0) {
                    *mystack = *st;
                    ++mystack;
                    ++st;
                    state = 0;
                }
                break;
            case 4: // ^
                if (*p == 'a') state = 5;
                else if (*p == '&') state = 8;
                else state = 0; //???
                break;
            case 5: // ^a
                if ((*p == 's') || (*p == 'o') || (*p == 'y')) {
                    *mystack = *st;
                    ++mystack;
                    ++st;
                    state = 0;
                } else if (*p == '&') state = 6;
                else if (*p == 'a') state = 7;
                else state = 0; //???
                break;
            case 6: // ^a&
                if ((*p == 's') || (*p == 'o')) {
                    *mystack = *st;
                    ++mystack;
                    ++st;
                    state = 0;
                } else if (*p == 'a') state = 7;
                else state = 0; //???
                break;
            case 7: // ^aa / ^a&a
                if (*p == 'y') {
                    *mystack = *st;
                    ++mystack;
                    ++st;
                    state = 0;
                } else state = 0; //???
                break;
            case 8: // ^&
                if (*p == 'a') state = 9;
                else state = 0; //???
                break;
            case 9: // ^&a
                if (*p == 'y') {
                    *mystack = *st;
                    ++mystack;
                    ++st;
                    state = 0;
                } else state = 0; //???
                break;
        }
        ++p;
    } while (*p && (inblocks || state));
}

#endif

#include "my_xcb_defs.h"

#define NXCB 8
static my_xcb_connection_t* my_xcb_connects[NXCB] = {0};
static x64_xcb_connection_t x64_xcb_connects[NXCB] = {0};
static void* xcb_display[NXCB] = {0};

void* align_xcb_connection(void* src)
{
    if(!src)
        return src;
    // find it
    my_xcb_connection_t * dest = NULL;
    for(int i=0; i<NXCB && !dest; ++i)
        if(src==&x64_xcb_connects[i])
            dest = my_xcb_connects[i];
    #if 1
    if(!dest)
        dest = add_xcb_connection(src);
    #else
    if(!dest) {
        printf_log(LOG_NONE, "Error, xcb_connect %p not found\n", src);
        abort();
    }
    #endif
    #if 1
    // do not update most values
    x64_xcb_connection_t* source = src;
    dest->has_error = source->has_error;
    dest->setup = source->setup;
    dest->fd = source->fd;
    //memcpy(&dest->iolock, source->iolock, MUTEX_SIZE_X64);
    //dest->in = source->in;
    //dest->out = source->out;
    //memcpy(&dest->ext.lock, source->ext.lock, MUTEX_SIZE_X64);
    dest->ext.extensions = source->ext.extensions;
    dest->ext.extensions_size = source->ext.extensions_size;
    //memcpy(&dest->xid.lock, source->xid.lock, MUTEX_SIZE_X64);
    dest->xid.base = source->xid.base;
    dest->xid.inc = source->xid.inc;
    dest->xid.last = source->xid.last;
    dest->xid.max = source->xid.last;
    #endif
    return dest;
}

void unalign_xcb_connection(void* src, void* dst)
{
    if(!src || !dst || src==dst)
        return;
    // update values
    my_xcb_connection_t* source = src;
    x64_xcb_connection_t* dest = dst;
    dest->has_error = source->has_error;
    dest->setup = source->setup;
    dest->fd = source->fd;
    memcpy(dest->iolock, &source->iolock, MUTEX_SIZE_X64);
    dest->in = source->in;
    memcpy(dest->out.reqlenlock, &source->out.reqlenlock, MUTEX_SIZE_X64);
    dest->out.cond = source->out.cond;
    dest->out.maximum_request_length = source->out.maximum_request_length;
    dest->out.maximum_request_length_tag = source->out.maximum_request_length_tag;
    dest->out.out_fd = source->out.out_fd;
    memcpy(dest->out.queue, source->out.queue, sizeof(dest->out.queue));
    dest->out.queue_len = source->out.queue_len;
    dest->out.request = source->out.request;
    dest->out.request_written = source->out.request_written;
    dest->out.return_socket = source->out.return_socket;
    dest->out.socket_closure = source->out.socket_closure;
    dest->out.socket_cond = source->out.socket_cond;
    dest->out.socket_moving = source->out.socket_moving;
    dest->out.writing = source->out.writing;
    memcpy(dest->ext.lock, &source->ext.lock, MUTEX_SIZE_X64);
    dest->ext.extensions = source->ext.extensions;
    dest->ext.extensions_size = source->ext.extensions_size;
    memcpy(dest->xid.lock, &source->xid.lock, MUTEX_SIZE_X64);
    dest->xid.base = source->xid.base;
    dest->xid.inc = source->xid.inc;
    dest->xid.last = source->xid.last;
    dest->xid.max = source->xid.last;
}

void* add_xcb_connection(void* src)
{
    if(!src)
        return src;
    // check if already exist
    for(int i=0; i<NXCB; ++i)
        if(my_xcb_connects[i] == src) {
            unalign_xcb_connection(src, &x64_xcb_connects[i]);
            return &x64_xcb_connects[i];
        }
    // find a free slot
    for(int i=0; i<NXCB; ++i)
        if(!my_xcb_connects[i]) {
            my_xcb_connects[i] = src;
            xcb_display[i] = NULL;
            unalign_xcb_connection(src, &x64_xcb_connects[i]);
            return &x64_xcb_connects[i];
        }
    printf_log(LOG_NONE, "Error, no more free xcb_connect slot for %p\n", src);
    return src;
}

void del_xcb_connection(void* src)
{
    if(!src)
        return;
    // find it
    for(int i=0; i<NXCB; ++i)
        if(src==&x64_xcb_connects[i]) {
            my_xcb_connects[i] = NULL;
            xcb_display[i] = NULL;
            memset(&x64_xcb_connects[i], 0, sizeof(x64_xcb_connection_t));
            return;
        }
    printf_log(LOG_NONE, "Error, xcb_connect %p not found for deletion\n", src);
}

void register_xcb_display(void* d, void* xcb)
{
    for(int i=0; i<NXCB; ++i)
        if(&x64_xcb_connects[i] == xcb) {
            xcb_display[i] = d;
            return;
        }
}

void unregister_xcb_display(void* d)
{
    for(int i=0; i<NXCB; ++i)
        if(&xcb_display[i] == d) {
            my_xcb_connects[i] = NULL;
            memset(&x64_xcb_connects[i], 0, sizeof(x64_xcb_connection_t));
            return;
        }
}