#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <wchar.h>
#include <sys/epoll.h>
#include <fts.h>
#include <sys/stat.h>
#include <sys/sem.h>

#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "myalign.h"

static int regs_abi[] = {_DI, _SI, _DX, _CX, _R8, _R9};

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

#if 0
void myStackAlignGVariantNew(const char* fmt, uint32_t* st, uint32_t* mystack)
{
    if (!fmt)
        return;
    
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
                        *mystack = *st;
                        ++mystack;
                        ++st;
                        break;
                    case 'x': // gint64
                    case 't': // guint64
                    case 'd': // gdouble
                        if ((((uint32_t)mystack)&0x7)!=0)
                            ++mystack;
                        *(uint64_t*)mystack = *(uint64_t*)st;
                        st+=2; mystack+=2;
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
            case 8: // ^&
                if (*p == 'a') state = 9;
                else state = 0; //???
            case 9: // ^&a
                if (*p == 'y') {
                    *mystack = *st;
                    ++mystack;
                    ++st;
                    state = 0;
                } else state = 0; //???
        }
        ++p;
    } while (*p && (inblocks || state));
}

#endif

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

typedef union __attribute__((packed)) x64_epoll_data {
    void    *ptr;
    int      fd;
    uint32_t u32;
    uint64_t u64;
} x64_epoll_data_t;

struct __attribute__((packed)) x64_epoll_event {
    uint32_t            events;
    x64_epoll_data_t    data;
};
// Arm -> x64
void UnalignEpollEvent(void* dest, void* source, int nbr)
{
    struct x64_epoll_event *x64_struct = (struct x64_epoll_event*)dest;
    struct epoll_event *arm_struct = (struct epoll_event*)source;
    while(nbr) {
        x64_struct->events = arm_struct->events;
        x64_struct->data.u64 = arm_struct->data.u64;
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
        arm_struct->data.u64 = x64_struct->data.u64;
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