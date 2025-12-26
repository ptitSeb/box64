#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <wchar.h>
#include <sys/epoll.h>
#include <fts.h>
#include <sys/socket.h>
#include <obstack.h>

#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "myalign32.h"
#include "debug.h"
#include "box32.h"

void myStackAlign32(const char* fmt, uint32_t* st, uint64_t* mystack)
{
    if(!fmt)
        return;
    // loop...
    const char* p = fmt;
    int state = 0;
    double d;
    long double ld;
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
                switch(*p) {
                    case '%': state = 0;  ++p; break; //%% = back to 0
                    case 'l': ++state; if (state>3) state=3; ++p; break;
                    case 'z': state = 2; ++p; break;
                    case 'L': state = 4; ++p; break;
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
                    case 'n':
                    case 'o': state += 20; break;   // int
                    case 'x':
                    case 'X':
                    case 'u': state += 40; break;   // uint
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
                    case 'p':
                    case 'S':
                    case 's': state = 30; break; // pointers
                    case '$': ++p; break; // should issue a warning, it's not handled...
                    case '*': *(mystack++) = *(st++); ++p; break; // fetch an int in the stack....
                    case ' ': state=0; ++p; break;
                    default:
                        state=20; // other stuff, put an int...
                }
                break;
            case 11:    //double
            case 12:    //%lg, still double
            case 13:    //%llg, still double
            case 23:    // 64bits int
            case 43:    // 64bits uint
                *(uint64_t*)mystack = *(uint64_t*)st;
                st+=2; mystack+=1;
                state = 0;
                ++p;
                break;
            case 22:    // long int
                *(int64_t*)mystack = from_long(*(long_t*)st);
                st+=1; mystack+=1;
                state = 0;
                ++p;
                break;
            case 42:    // long uint
                *(uint64_t*)mystack = from_ulong(*(ulong_t*)st);
                st+=1; mystack+=1;
                state = 0;
                ++p;
                break;
            case 14:    //%LG long double
                #ifdef HAVE_LD80BITS
                memcpy(mystack, st, 10);
                st+=3; mystack+=2;
                #else
                LD2D((void*)st, &d);
                ld = d;
                if(((uintptr_t)mystack)&0xf)    // align the long double
                    mystack++;
                memcpy(mystack, &ld, 16);
                st+=3; mystack+=2;
                #endif
                state = 0;
                ++p;
                break;
            case 30:    //pointer
                *(uintptr_t*)mystack = from_ptr(*st);
                st++; mystack+=1;
                state = 0;
                ++p;
                break;
            case 20:    // fallback
            case 21:
            case 24:    // normal int / pointer
            case 40:
            case 41:
                *mystack = *st;
                ++mystack;
                ++st;
                state = 0;
                ++p;
                break;
            default:
                // whattt?
                state = 0;
        }
    }
}

size_t myStackAlignScanf32(const char* fmt, uint32_t* st, uint64_t* mystack, size_t nb_elem)
{
    
    if(!fmt)
        return 0;
    // loop...
    const char* p = fmt;
    size_t conv = 0;
    int state = 0;
    int ign = 0;
    uint64_t* saved = mystack;
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
                    case 'n':
                    case 'o': state += 20; break;   // int
                    case 'u':
                    case 'x':
                    case 'X': state += 40; break;   // usigned int
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
                    case '[': state += 60; ++p; break;
                    case 'm': state = 0; ++p; break; // no argument
                    case 'p': state = 30; break; // pointers
                    case 'S':
                    case 's': state = 50; break; // string
                    case '$': ++p; break; // should issue a warning, it's not handled...
                    case '*': ign=1; ++p; break; // ignore arg
                    case ' ': state=0; ++p; break;
                    default:
                        state=20; // other stuff, put an int...
                }
                break;
            case 22:    // long uint
            case 25:    // size_t int
            case 42:    // long uint
            case 45:
            case 30:    // pointer
                if(!ign) {
                    ++conv;
                    saved[nb_elem-conv] = 0;
                    *mystack = (uintptr_t)&saved[nb_elem-conv];
                    ++st;
                    ++mystack;
                }
                state = 0;
                ++p;
                break;
            
            case 11:    //double
            case 12:    //%lg, still double
            case 13:    //%llg, still double
            case 14:    //%Lg long double
            case 15:    //%zg
            case 20:    // fallback
            case 21:
            case 23:    // 64bits int
            case 24:    // normal int / pointer
            case 40:
            case 41:
            case 43:
            case 44:
            case 50:
                if(!ign) {
                    *mystack = *st;
                    ++st;
                    ++mystack;
                }
                state = 0;
                ++p;
                break;
            case 61:
                switch(*p) {
                    case ']': state = 50; break;
                    case '\\': ++p; if(*p) ++p; break;
                    default: ++p; break;
                }
                break;
            default:
                // whaaaat?
                state = 0;
        }
    }
    return conv;
}

void myStackAlignScanf32_final(const char* fmt, uint32_t* st, uint64_t* mystack, size_t nb_elem, int n)
{
    
    if(!fmt || n<=0)
        return;
    // loop...
    const char* p = fmt;
    size_t conv = 0;
    int state = 0;
    int ign = 0;
    uint64_t* saved = mystack;
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
                    case 'n':
                    case 'o': state += 20; break;   // int
                    case 'u':
                    case 'x':
                    case 'X': state += 40; break;   // usigned int
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
                    case '[': state += 60; ++p; break;
                    case 'm': state = 0; ++p; break; // no argument
                    case 'p': state = 30; break; // pointers
                    case 'S':
                    case 's': state = 50; break; // strings
                    case '$': ++p; break; // should issue a warning, it's not handled...
                    case '*': ign=1; ++p; break; // ignore arg
                    case ' ': state=0; ++p; break;
                    default:
                        state=20; // other stuff, put an int...
                }
                break;
            case 22:    // long int
            case 25:    // size_t int
            case 42:    // long uint
            case 45:
            case 30:    // pointer
                if(!ign) {
                    ++conv;
                    if((state==22) || (state==25)) {
                        long_t* dst = from_ptrv(*st);
                        *dst = saved[nb_elem-conv]; // not using to_long on purpose
                    } else if(state==30) {
                        ptr_t* dst = from_ptrv(*st);
                        *dst = to_ptr(saved[nb_elem-conv]);
                    } else {
                        ulong_t* dst = from_ptrv(*st);
                        *dst = saved[nb_elem-conv]; //not using to to_ulong on pupose
                    }
                    ++st;
                    ++mystack;
                    if(!--n) return;
                }
                state = 0;
                ++p;
                break;
            
            case 11:    //double
            case 12:    //%lg, still double
            case 13:    //%llg, still double
            case 14:    //%Lg long double
            case 15:    //%zg
            case 20:    // fallback
            case 21:
            case 23:    // 64bits int
            case 24:    // normal int / pointer
            case 40:
            case 41:
            case 43:
            case 44:
            case 50:
                if(!ign) {
                    *mystack = *st;
                    ++st;
                    ++mystack;
                }
                state = 0;
                ++p;
                if(!--n) return;
                break;
            case 61:
                switch(*p) {
                    case ']': state = 50; break;
                    case '\\': ++p; if(*p) ++p; break;
                    default: ++p; break;
                }
                break;
            default:
                // whaaaat?
                state = 0;
        }
    }
}

size_t myStackAlignScanfW32(const char* fmt, uint32_t* st, uint64_t* mystack, size_t nb_elem)
{
    
    if(!fmt)
        return 0;
    // loop...
    const wchar_t* p = (const wchar_t*)fmt;
    int state = 0;
    size_t conv = 0;
    int ign = 0;
    uint64_t* saved = mystack;
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
                    case 'n':
                    case 'o': state += 20; break;   // int
                    case 'u':
                    case 'x':
                    case 'X': state += 40; break;   // usigned int
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
                    case 'p': state = 30; break; // pointers
                    case 'S':
                    case 's': state = 50; break; // strings
                    case '$': ++p; break; // should issue a warning, it's not handled...
                    case '*': ign=1; ++p; break; // ignore arg
                    case ' ': state=0; ++p; break;
                    default:
                        state=20; // other stuff, put an int...
                }
                break;
            case 22:    // long uint
            case 25:    // size_t int
            case 42:    // long uint
            case 45:
            case 30:    // pointer
                if(!ign) {
                    ++conv;
                    saved[nb_elem-conv] = 0;
                    *mystack = (uintptr_t)&saved[nb_elem-conv];
                    ++st;
                    ++mystack;
                }
                state = 0;
                ++p;
                break;
            
            case 11:    //double
            case 12:    //%lg, still double
            case 13:    //%llg, still double
            case 14:    //%Lg long double
            case 15:    //%zg
            case 20:    // fallback
            case 21:
            case 23:    // 64bits int
            case 24:    // normal int / pointer
            case 40:
            case 41:
            case 43:
            case 44:
            case 50:
                if(!ign) {
                    *mystack = *st;
                    ++st;
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
    return conv;
}

void myStackAlignScanfW32_final(const char* fmt, uint32_t* st, uint64_t* mystack, size_t nb_elem, int n)
{
    
    if(!fmt || n<=0)
        return;
    // loop...
    const wchar_t* p = (const wchar_t*)fmt;
    int state = 0;
    size_t conv = 0;
    int ign = 0;
    uint64_t* saved = mystack;
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
                    case 'n':
                    case 'o': state += 20; break;   // int
                    case 'u':
                    case 'x':
                    case 'X': state += 40; break;   // usigned int
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
                    case 'p': state = 30; break; // pointers
                    case 'S':
                    case 's': state = 50; break; // string
                    case '$': ++p; break; // should issue a warning, it's not handled...
                    case '*': ign=1; ++p; break; // ignore arg
                    case ' ': state=0; ++p; break;
                    default:
                        state=20; // other stuff, put an int...
                }
                break;
            case 22:    // long uint
            case 25:    // size_t int
            case 42:    // long uint
            case 45:
            case 30:    // pointer
                if(!ign) {
                    ++conv;
                    if(state==22 || state==0x25) {
                        int32_t* dst = from_ptrv(*st);
                        *dst = to_long(saved[nb_elem-conv]);
                    } else if(state==30) {
                        ptr_t* dst = from_ptrv(*st);
                        *dst = to_ptr(saved[nb_elem-conv]);
                    } else {
                        uint32_t* dst = from_ptrv(*st);
                        *dst = to_ulong(saved[nb_elem-conv]);
                    }
                    ++st;
                    ++mystack;
                }
                state = 0;
                ++p;
                if(!--n) return;
                break;
            
            case 11:    //double
            case 12:    //%lg, still double
            case 13:    //%llg, still double
            case 14:    //%Lg long double
            case 15:    //%zg
            case 20:    // fallback
            case 21:
            case 23:    // 64bits int
            case 24:    // normal int / pointer
            case 40:
            case 41:
            case 43:
            case 44:
            case 50:
                if(!ign) {
                    *mystack = *st;
                    ++st;
                    ++mystack;
                }
                state = 0;
                ++p;
                if(!--n) return;
                break;
            default:
                // whaaaat?
                state = 0;
        }
    }
}

void myStackAlignW32(const char* fmt, uint32_t* st, uint64_t* mystack)
{
    // loop...
    const wchar_t* p = (const wchar_t*)fmt;
    int state = 0;
    double d;
    long double ld;
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
                switch(*p) {
                    case '%': state = 0;  ++p; break; //%% = back to 0
                    case 'l': ++state; if (state>3) state=3; ++p; break;
                    case 'z': state = 2; ++p; break;
                    case 'L': state = 4; ++p; break;
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
                    case 'o': state += 20; break;   // int
                    case 'x':
                    case 'X':
                    case 'u': state += 40; break;   // uint
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
                    case '*': *(mystack++) = *(st++); ++p; break; // fetch an int in the stack....
                    case ' ': state=0; ++p; break;
                    default:
                        state=20; // other stuff, put an int...
                }
                break;
            case 11:    //double
            case 12:    //%lg, still double
            case 13:    //%llg, still double
            case 23:    // 64bits int
            case 43:    // 64bits uint
                *(uint64_t*)mystack = *(uint64_t*)st;
                st+=2; mystack+=1;
                state = 0;
                ++p;
                break;
            case 22:    // long int
                *(int64_t*)mystack = from_long(*(long_t*)st);
                st+=1; mystack+=1;
                state = 0;
                ++p;
                break;
            case 42:    // long uint
                *(uint64_t*)mystack = from_ulong(*(ulong_t*)st);
                st+=1; mystack+=1;
                state = 0;
                ++p;
                break;
            case 14:    //%LG long double
                #ifdef HAVE_LD80BITS
                memcpy(mystack, st, 10);
                st+=3; mystack+=2;
                #else
                LD2D((void*)st, &d);
                if(((uintptr_t)mystack)&0xf)    // align the long double
                    mystack++;
                ld = d;
                memcpy(mystack, &ld, 16);
                st+=3; mystack+=2;
                #endif
                state = 0;
                ++p;
                break;
            case 30:    //pointer
                *(uintptr_t*)mystack = from_ptr(*st);
                st++; mystack+=1;
                state = 0;
                ++p;
                break;
            case 20:    // fallback
            case 21:
            case 24:    // normal int / pointer
            case 40:
            case 41:
                *mystack = *st;
                ++mystack;
                ++st;
                state = 0;
                ++p;
                break;
            default:
                // whattt?
                state = 0;
        }
    }
}


#if 0

typedef struct __attribute__((packed, aligned(4))) {
  unsigned char   *body_data;
  long    body_storage;
  long    body_fill;
  long    body_returned;


  int     *lacing_vals;
  int64_t *granule_vals;
  long    lacing_storage;
  long    lacing_fill;
  long    lacing_packet;
  long    lacing_returned;

  unsigned char    header[282];
  int              header_fill __attribute__ ((aligned (4)));

  int     e_o_s;
  int     b_o_s;
  long    serialno;
  long    pageno;
  int64_t  packetno;
  int64_t   granulepos;

} ogg_stream_state_x64;

typedef struct __attribute__((packed, aligned(4))) vorbis_dsp_state_x64 {
  int analysisp;
  void *vi; //vorbis_info

  float **pcm;
  float **pcmret;
  int      pcm_storage;
  int      pcm_current;
  int      pcm_returned;

  int  preextrapolate;
  int  eofflag;

  long lW;
  long W;
  long nW;
  long centerW;

  int64_t granulepos;
  int64_t sequence;

  int64_t glue_bits;
  int64_t time_bits;
  int64_t floor_bits;
  int64_t res_bits;

  void       *backend_state;
} vorbis_dsp_state_x64;

typedef struct __attribute__((packed, aligned(4))) {
  long endbyte;
  int  endbit;

  unsigned char *buffer;
  unsigned char *ptr;
  long storage;
} oggpack_buffer_x64;

typedef struct __attribute__((packed, aligned(4))) vorbis_block_x64 {

  float  **pcm;
  oggpack_buffer_x64 opb;

  long  lW;
  long  W;
  long  nW;
  int   pcmend;
  int   mode;

  int         eofflag;
  int64_t granulepos;
  int64_t sequence;
  void *vd;
  
  void               *localstore;
  long                localtop;
  long                localalloc;
  long                totaluse;
  void *reap;

  long glue_bits;
  long time_bits;
  long floor_bits;
  long res_bits;

  void *internal;

} vorbis_block_x64;

typedef struct __attribute__((packed, aligned(4))) OggVorbis_x64  {
  void            *datasource; /* Pointer to a FILE *, etc. */
  int              seekable;
  int64_t      offset;
  int64_t      end;
  ogg_sync_state   oy;

  /* If the FILE handle isn't seekable (eg, a pipe), only the current
     stream appears */
  int              links;
  int64_t     *offsets;
  int64_t     *dataoffsets;
  long            *serialnos;
  int64_t     *pcmlengths; /* overloaded to maintain binary
                                  compatibility; x2 size, stores both
                                  beginning and end values */
  void     *vi; //vorbis_info
  void  *vc;    //vorbis_comment

  /* Decoding working state local storage */
  int64_t      pcm_offset;
  int              ready_state;
  long             current_serialno;
  int              current_link;

  double           bittrack;
  double           samptrack;

  ogg_stream_state_x64 os; /* take physical pages, weld into a logical
                          stream of packets */
  vorbis_dsp_state_x64 vd; /* central working state for the packet->PCM decoder */
  vorbis_block_x64     vb; /* local working space for packet->PCM decode */

  ov_callbacks callbacks;

} OggVorbis_x64;

#define TRANSFERT \
GO(datasource) \
GO(seekable) \
GO(offset) \
GO(end) \
GOM(oy, sizeof(ogg_sync_state)) \
GO(links) \
GO(offsets) \
GO(dataoffsets) \
GO(serialnos) \
GO(pcmlengths) \
GO(vi) \
GO(vc) \
GO(pcm_offset) \
GO(ready_state) \
GO(current_serialno) \
GO(current_link) \
GOM(bittrack, 16) \
GO(os.body_data) \
GO(os.body_storage) \
GO(os.body_fill) \
GO(os.body_returned) \
GO(os.lacing_vals) \
GO(os.granule_vals) \
GO(os.lacing_storage) \
GO(os.lacing_fill) \
GO(os.lacing_packet) \
GO(os.lacing_returned) \
GOM(os.header, 282) \
GO(os.header_fill) \
GO(os.e_o_s) \
GO(os.b_o_s) \
GO(os.serialno) \
GO(os.pageno) \
GO(os.packetno) \
GO(os.granulepos) \
GO(vd.analysisp) \
GO(vd.vi) \
GO(vd.pcm) \
GO(vd.pcmret) \
GO(vd.pcm_storage) \
GO(vd.pcm_current) \
GO(vd.pcm_returned) \
GO(vd.preextrapolate) \
GO(vd.eofflag) \
GO(vd.lW) \
GO(vd.W) \
GO(vd.nW) \
GO(vd.centerW) \
GO(vd.granulepos) \
GO(vd.sequence) \
GO(vd.glue_bits) \
GO(vd.time_bits) \
GO(vd.floor_bits) \
GO(vd.res_bits) \
GO(vd.backend_state) \
GO(vb.pcm) \
GO(vb.opb.endbyte) \
GO(vb.opb.endbit) \
GO(vb.opb.buffer) \
GO(vb.opb.ptr) \
GO(vb.opb.storage) \
GO(vb.lW) \
GO(vb.W) \
GO(vb.nW) \
GO(vb.pcmend) \
GO(vb.mode) \
GO(vb.eofflag) \
GO(vb.granulepos) \
GO(vb.sequence) \
GO(vb.localstore) \
GO(vb.localtop) \
GO(vb.localalloc) \
GO(vb.totaluse) \
GO(vb.reap) \
GO(vb.glue_bits) \
GO(vb.time_bits) \
GO(vb.floor_bits) \
GO(vb.res_bits) \
GO(vb.internal) \
GOM(callbacks, sizeof(ov_callbacks))

void AlignOggVorbis(void* dest, void* source)
{
     // Arm -> x64
     OggVorbis_x64* src = (OggVorbis_x64*)source;
     OggVorbis*     dst = (OggVorbis*)dest;

     #define GO(A) dst->A = src->A;
     #define GOM(A, S) memcpy(&dst->A, &src->A, S);
     TRANSFERT
     #undef GO
     #undef GOM
     dst->vb.vd = (src->vb.vd == &src->vd)?&dst->vd:(vorbis_dsp_state*)src->vb.vd;
}
void UnalignOggVorbis(void* dest, void* source)
{
    // x64 -> Arm
     OggVorbis_x64* dst = (OggVorbis_x64*)dest;
     OggVorbis*     src = (OggVorbis*)source;

     #define GO(A) dst->A = src->A;
     #define GOM(A, S) memcpy(&dst->A, &src->A, S);
     TRANSFERT
     #undef GO
     #undef GOM
     dst->vb.vd = (src->vb.vd == &src->vd)?&dst->vd:(vorbis_dsp_state_x64*)src->vb.vd;
}
#undef TRANSFERT

#define TRANSFERT \
GO(analysisp) \
GO(vi) \
GO(pcm) \
GO(pcmret) \
GO(pcm_storage) \
GO(pcm_current) \
GO(pcm_returned) \
GO(preextrapolate) \
GO(eofflag) \
GO(lW) \
GO(W) \
GO(nW) \
GO(centerW) \
GO(granulepos) \
GO(sequence) \
GO(glue_bits) \
GO(time_bits) \
GO(floor_bits) \
GO(res_bits) \
GO(backend_state)

void UnalignVorbisDspState(void* dest, void* source)
{
    // Arm -> x64
     #define GO(A) ((vorbis_dsp_state_x64*)dest)->A = ((vorbis_dsp_state*)source)->A;
     #define GOM(A, S) memcpy(&((vorbis_dsp_state_x64*)dest)->A, &((vorbis_dsp_state*)source)->A, S);
     TRANSFERT
     #undef GO
     #undef GOM
}
void AlignVorbisDspState(void* dest, void* source)
{
    // x64 -> Arm
     #define GO(A) ((vorbis_dsp_state*)dest)->A = ((vorbis_dsp_state_x64*)source)->A;
     #define GOM(A, S) memcpy(&((vorbis_dsp_state*)dest)->A, &((vorbis_dsp_state_x64*)source)->A, S);
     TRANSFERT
     #undef GO
     #undef GOM
}
#undef TRANSFERT

#define TRANSFERT \
GO(pcm) \
GO(opb.endbyte) \
GO(opb.endbit) \
GO(opb.buffer) \
GO(opb.ptr) \
GO(opb.storage) \
GO(lW) \
GO(W) \
GO(nW) \
GO(pcmend) \
GO(mode) \
GO(eofflag) \
GO(granulepos) \
GO(sequence) \
GO(vd) \
GO(localstore) \
GO(localtop) \
GO(localalloc) \
GO(totaluse) \
GO(reap) \
GO(glue_bits) \
GO(time_bits) \
GO(floor_bits) \
GO(res_bits) \
GO(internal)

void UnalignVorbisBlock(void* dest, void* source)
{
    // Arm -> x64
     #define GO(A) ((vorbis_block_x64*)dest)->A = ((vorbis_block*)source)->A;
     #define GOM(A, S) memcpy(&((vorbis_block_x64*)dest)->A, &((vorbis_block*)source)->A, S);
     TRANSFERT
     #undef GO
     #undef GOM
}
void AlignVorbisBlock(void* dest, void* source)
{
    // x64 -> Arm
     #define GO(A) ((vorbis_block*)dest)->A = ((vorbis_block_x64*)source)->A;
     #define GOM(A, S) memcpy(&((vorbis_block*)dest)->A, &((vorbis_block_x64*)source)->A, S);
     TRANSFERT
     #undef GO
     #undef GOM
}

#undef TRANSFERT
#endif
typedef union __attribute__((packed, aligned(4))) i386_epoll_data {
    ptr_t    ptr;   //void*
    int      fd;
    uint32_t u32;
    uint64_t u64;
} i386_epoll_data_t;

struct __attribute__((packed, aligned(4))) i386_epoll_event {
    uint32_t            events;
    i386_epoll_data_t    data;
};
// Arm -> i386
void UnalignEpollEvent32(void* dest, void* source, int nbr)
{
    struct i386_epoll_event *i386_struct = (struct i386_epoll_event*)dest;
    struct epoll_event *arm_struct = (struct epoll_event*)source;
    while(nbr) {
        i386_struct->events = arm_struct->events;
        i386_struct->data.u64 = arm_struct->data.u64;
        ++i386_struct;
        ++arm_struct;
        --nbr;
    }
}

// i386 -> Arm
void AlignEpollEvent32(void* dest, void* source, int nbr)
{
    struct i386_epoll_event *i386_struct = (struct i386_epoll_event*)source;
    struct epoll_event *arm_struct = (struct epoll_event*)dest;
    while(nbr) {
        arm_struct->events = i386_struct->events;
        arm_struct->data.u64 = i386_struct->data.u64;
        ++i386_struct;
        ++arm_struct;
        --nbr;
    }
}
#if 0
typedef struct __attribute__((packed, aligned(4))) x64_SMPEG_Info_s {
    int has_audio;
    int has_video;
    int width;
    int height;
    int current_frame;
    double current_fps;
    char audio_string[80];
    int  audio_current_frame;
    uint32_t current_offset;
    uint32_t total_size;
    double current_time;
    double total_time;
} x64_SMPEG_Info_t;

#define TRANSFERT \
GO(has_audio) \
GO(has_video) \
GO(width) \
GO(height) \
GO(current_frame) \
GO(current_fps) \
GOM(audio_string, 80) \
GO(audio_current_frame) \
GO(current_offset) \
GO(total_size) \
GO(current_time) \
GO(total_time)


// Arm -> x64
void UnalignSmpegInfo(void* dest, void* source)
{
    #define GO(A) ((x64_SMPEG_Info_t*)dest)->A = ((my_SMPEG_Info_t*)source)->A;
    #define GOM(A, S) memcpy(&((x64_SMPEG_Info_t*)dest)->A, &((my_SMPEG_Info_t*)source)->A, S);
    TRANSFERT
    #undef GO
    #undef GOM
}
// x64 -> Arm
void AlignSmpegInfo(void* dest, void* source)
{
    #define GO(A) ((my_SMPEG_Info_t*)dest)->A = ((x64_SMPEG_Info_t*)source)->A;
    #define GOM(A, S) memcpy(&((my_SMPEG_Info_t*)dest)->A, &((x64_SMPEG_Info_t*)source)->A, S);
    TRANSFERT
    #undef GO
    #undef GOM
}
#undef TRANSFERT

#define TRANSFERT   \
GOV(fts_cycle)      \
GOV(fts_parent)     \
GOV(fts_link)       \
GO(fts_number)      \
GO(fts_pointer)     \
GO(fts_accpath)     \
GO(fts_path)        \
GO(fts_errno)       \
GO(fts_symfd)       \
GO(fts_pathlen)     \
GO(fts_namelen)     \
GO(fts_ino)         \
GO(fts_dev)         \
GO(fts_nlink)       \
GO(fts_level)       \
GO(fts_info)        \
GO(fts_flags)       \
GO(fts_instr)       \
GO(fts_statp)       \
GOM(fts_name, sizeof(void*))

// Arm -> x64
void UnalignFTSENT(void* dest, void* source)
{
    #define GO(A) ((x64_ftsent_t*)dest)->A = ((FTSENT*)source)->A;
    #define GOV(A) ((x64_ftsent_t*)dest)->A = (void*)((FTSENT*)source)->A;
    #define GOM(A, S) memcpy(&((x64_ftsent_t*)dest)->A, &((FTSENT*)source)->A, S);
    TRANSFERT
    #undef GO
    #undef GOV
    #undef GOM
}
// x64 -> Arm
void AlignFTSENT(void* dest, void* source)
{
    #define GO(A) ((FTSENT*)dest)->A = ((x64_ftsent_t*)source)->A;
    #define GOV(A) ((FTSENT*)dest)->A = (void*)((x64_ftsent_t*)source)->A;
    #define GOM(A, S) memcpy(&((FTSENT*)dest)->A, &((x64_ftsent_t*)source)->A, S);
    TRANSFERT
    #undef GO
    #undef GOV
    #undef GOM
}
#undef TRANSFERT

void alignNGValue(my_GValue_t* v, void* value, int n)
{
    while(n) {
        v->g_type = *(int*)value;
        memcpy(v->data, value+4, 2*sizeof(double));
        ++v;
        value+=4+2*sizeof(double);
        --n;
    }
}
void unalignNGValue(void* value, my_GValue_t* v, int n)
{
    while(n) {
        *(int*)value = v->g_type;
        memcpy(value+4, v->data, 2*sizeof(double));
        ++v;
        value+=4+2*sizeof(double);
        --n;
    }
}
#endif

// x86 -> Native
void AlignIOV_32(void* dest, void* source)
{
    struct iovec* d = dest;
    struct i386_iovec* s = source;

    d->iov_base = from_ptrv(s->iov_base);
    d->iov_len = s->iov_len;
}

// Native -> x86
void UnalignIOV_32(void* dest, void* source)
{
    struct iovec* s = source;
    struct i386_iovec* d = dest;

    d->iov_base = to_ptrv(s->iov_base);
    d->iov_len = s->iov_len;
}

void* my32___cmsg_nxthdr(struct i386_msghdr* mhdr, struct i386_cmsghdr* cmsg);

// x86 -> Native
void AlignMsgHdr_32(void* dest, void* dest_iov, void* dest_cmsg, void* source, int convert_control)
{
    struct iovec* iov = dest_iov;
    struct msghdr* d = dest;
    struct i386_msghdr* s = source;
    struct i386_iovec* s_iov = from_ptrv(s->msg_iov);
    d->msg_name = from_ptrv(s->msg_name);
    d->msg_namelen = s->msg_namelen;
    d->msg_iov = iov;
    // TODO: check if iovlen is too big
    for(uint32_t i=0; i<s->msg_iovlen; ++i) {
        AlignIOV_32(d->msg_iov+i, s_iov+i);
    }
    d->msg_iovlen = s->msg_iovlen;
    d->msg_controllen = s->msg_controllen;
    if(convert_control) {
        if(s->msg_control) {
            d->msg_control = dest_cmsg;
            struct i386_cmsghdr* cmsg = (s->msg_controllen)?from_ptrv(s->msg_control):NULL;
            struct cmsghdr* dcmsg = dest_cmsg;
            while(cmsg) {
                dcmsg->cmsg_len = from_ulong(cmsg->cmsg_len);
                dcmsg->cmsg_level = cmsg->cmsg_level;
                dcmsg->cmsg_type = cmsg->cmsg_type;
                if(cmsg->cmsg_len) {
                    dcmsg->cmsg_len += 4;
                    memcpy(CMSG_DATA(dcmsg), cmsg+1, cmsg->cmsg_len-sizeof(struct i386_cmsghdr));
                    d->msg_controllen += 4;
                }
                struct cmsghdr* next = (struct cmsghdr*)(((uintptr_t)dcmsg) + ((dcmsg->cmsg_len+7)&~7));
                cmsg = my32___cmsg_nxthdr(s, cmsg);
                uintptr_t next_diff = (uintptr_t)next-((uintptr_t)dcmsg+dcmsg->cmsg_len);
                if(cmsg)
                    d->msg_controllen+=next_diff;
                dcmsg = next;
                
            }
        } else 
            d->msg_control = NULL;
    } else {
        if(d->msg_controllen) d->msg_controllen+=4;
        d->msg_control = (s->msg_control)?dest_cmsg:NULL;
        if(d->msg_control) memset(d->msg_control, 0, d->msg_controllen);
    }
    d->msg_flags = s->msg_flags;
}

void UnalignMsgHdr_32(void* dest, void* source)
{
    struct msghdr* s = source;
    struct i386_msghdr* d = dest;
    struct iovec* s_iov = s->msg_iov;
    struct i386_iovec* d_iov = from_ptrv(d->msg_iov);
    d->msg_name = to_ptrv(s->msg_name);
    d->msg_namelen = s->msg_namelen;
    // TODO: check if iovlen is too big
    for(uint32_t i=0; i<s->msg_iovlen; ++i) {
        UnalignIOV_32(d_iov+i, s_iov+i);
    }
    d->msg_iovlen = s->msg_iovlen;
    d->msg_controllen = s->msg_controllen;
    if(s->msg_control) {
        struct i386_cmsghdr* dcmsg = from_ptrv(d->msg_control);
        struct cmsghdr* scmsg = (s->msg_controllen)?s->msg_control:NULL;
        while(scmsg) {
            dcmsg->cmsg_len = to_ulong(scmsg->cmsg_len);
            dcmsg->cmsg_level = scmsg->cmsg_level;
            dcmsg->cmsg_type = scmsg->cmsg_type;
            if(dcmsg->cmsg_len) {
                dcmsg->cmsg_len -= 4;
                memcpy(dcmsg+1, CMSG_DATA(scmsg), dcmsg->cmsg_len-sizeof(struct i386_cmsghdr));
                d->msg_controllen -= 4;
            }
            dcmsg = (struct i386_cmsghdr*)(((uintptr_t)dcmsg) + ((dcmsg->cmsg_len+3)&~3));
            scmsg = CMSG_NXTHDR(s, scmsg);
        }
    } else 
        d->msg_control = 0;
    d->msg_flags = s->msg_flags;
}

#define TRANSFERT   \
GO(l_type)          \
GO(l_whence)        \
GO(l_start)         \
GO(l_len)           \
GO(l_pid)

// Arm -> x86
void UnalignFlock_32(void* dest, void* source)
{
    #define GO(A) ((i386_flock_t*)dest)->A = ((my_flock64_t*)source)->A;
    TRANSFERT
    #undef GO
}

// x86 -> Arm
void AlignFlock_32(void* dest, void* source)
{
    #define GO(A) ((my_flock64_t*)dest)->A = ((i386_flock_t*)source)->A;
    TRANSFERT
    #undef GO
}
#undef TRANSFERT

void convert_regext_to_32(void* d, void* s)
{
    my_regex_32_t* dst = d;
    my_regex_t* src = s;

    dst->buffer = to_ptrv(src->buffer);
    dst->allocated = to_ulong(src->allocated);
    dst->used = to_ulong(src->used);
    dst->syntax = to_ulong(src->syntax);
    dst->fastmap = to_ptrv(src->fastmap);
    dst->translate = to_ptrv(src->translate);
    dst->re_nsub = to_ulong(src->re_nsub);
    dst->flags = src->flags;
}

void convert_regext_to_64(void* d, void* s)
{
    my_regex_t* dst = d;
    my_regex_32_t* src = s;

    dst->buffer = from_ptrv(src->buffer);
    dst->allocated = from_ulong(src->allocated);
    dst->used = from_ulong(src->used);
    dst->syntax = from_ulong(src->syntax);
    dst->fastmap = from_ptrv(src->fastmap);
    dst->translate = from_ptrv(src->translate);
    dst->re_nsub = from_ulong(src->re_nsub);
    dst->flags = src->flags;
}

void* inplace_obstack_chunk_shrink(void* a)
{
    if(a) {
        struct my_obstack_chunk_32_t* dst = a;
        struct _obstack_chunk* src = a;
        dst->limit = to_ptrv(src->limit);
        dst->prev = to_ptrv(src->prev);
        memcpy(dst->contents, src->contents, sizeof(dst->contents));
    }
    return a;
}
void* inplace_obstack_chunk_enlarge(void* a)
{
    if(a) {
        struct my_obstack_chunk_32_t* src = a;
        struct _obstack_chunk* dst = a;
        memcpy(dst->contents, src->contents, sizeof(dst->contents));
        dst->prev = from_ptrv(src->prev);
        dst->limit = from_ptrv(src->limit);
    }
    return a;
}

void convert_obstack_to_32(void* d, void* s)
{
    if(!d || !s) return;
    struct my_obstack_32_t* dst = d;
    struct obstack *src = s;
    // chunks
    struct _obstack_chunk* chunk = src->chunk;
    while(chunk) {
        struct _obstack_chunk* prev = chunk->prev;
        inplace_obstack_chunk_shrink(chunk);
        chunk = prev;
    }
    // struture
    dst->chunk_size = to_long(src->chunk_size);
    dst->chunk = to_ptrv(src->chunk);
    dst->object_base = to_ptrv(src->object_base);
    dst->next_free = to_ptrv(src->next_free);
    dst->chunk_limit = to_ptrv(src->chunk_limit);
    dst->temp.tempptr = to_ptrv(src->temp.tempptr);
    dst->chunkfun = to_ptrv(src->chunkfun);
    dst->freefun = to_ptrv(src->freefun);
    dst->extra_arg = to_ptrv(src->extra_arg);
    dst->use_extra_arg = src->use_extra_arg;
    dst->maybe_empty_object = src->maybe_empty_object;
    dst->alloc_failed = src->alloc_failed;
}
void convert_obstack_to_64(void* d, void* s)
{
    if(!d || !s) return;
    struct my_obstack_32_t* src = s;
    struct obstack *dst = d;
    // struture
    dst->alloc_failed = src->alloc_failed;
    dst->maybe_empty_object = src->maybe_empty_object;
    dst->use_extra_arg = src->use_extra_arg;
    dst->extra_arg = from_ptrv(src->extra_arg);
    dst->freefun = from_ptrv(src->freefun);
    dst->chunkfun = from_ptrv(src->chunkfun);
    dst->temp.tempptr = from_ptrv(src->temp.tempptr);
    dst->chunk_limit = from_ptrv(src->chunk_limit);
    dst->next_free = from_ptrv(src->next_free);
    dst->object_base = from_ptrv(src->object_base);
    dst->chunk = from_ptrv(src->chunk);
    dst->chunk_size = from_long(src->chunk_size);
    // chunks
    struct _obstack_chunk* chunk = dst->chunk;
    while(chunk) {
        struct _obstack_chunk* prev = chunk->prev;
        inplace_obstack_chunk_enlarge(chunk);
        chunk = prev;
    }
}
