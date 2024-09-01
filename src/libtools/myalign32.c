#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <wchar.h>
#include <sys/epoll.h>
#include <fts.h>
#include <sys/socket.h>

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
                *(long double*)mystack = (long double)d;
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

void myStackAlignScanf32(const char* fmt, uint32_t* st, uint64_t* mystack)
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
}

void myStackAlignScanfW32(const char* fmt, uint32_t* st, uint64_t* mystack)
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
}

void myStackAlignGVariantNew32(const char* fmt, uint32_t* st, uint64_t* mystack)
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
                        *(uint64_t*)mystack = *(uint64_t*)st;
                        st+=2; mystack+=1;
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

void myStackAlignW32(const char* fmt, uint32_t* st, uint64_t* mystack)
{
    // loop...
    const wchar_t* p = (const wchar_t*)fmt;
    int state = 0;
    double d;
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
                *(long double*)mystack = (long double)d;
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

typedef struct __attribute__((packed)) {
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

typedef struct __attribute__((packed)) vorbis_dsp_state_x64 {
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

typedef struct __attribute__((packed)) {
  long endbyte;
  int  endbit;

  unsigned char *buffer;
  unsigned char *ptr;
  long storage;
} oggpack_buffer_x64;

typedef struct __attribute__((packed)) vorbis_block_x64 {

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

typedef struct __attribute__((packed)) OggVorbis_x64  {
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
typedef union __attribute__((packed)) i386_epoll_data {
    ptr_t    ptr;   //void*
    int      fd;
    uint32_t u32;
    uint64_t u64;
} i386_epoll_data_t;

struct __attribute__((packed)) i386_epoll_event {
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
typedef struct __attribute__((packed)) x64_SMPEG_Info_s {
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


// x86 -> Native
void AlignMsgHdr_32(void* dest, void* dest_iov, void* source)
{
    struct iovec* iov = dest_iov;
    struct msghdr* d = dest;
    struct i386_msghdr* s = source;
    struct i386_iovec* s_iov = from_ptrv(s->msg_iov);

    d->msg_name = from_ptrv(s->msg_name);
    d->msg_namelen = s->msg_namelen;
    d->msg_iov = iov;
    // TODO: check if iovlen is too big
    for(int i=0; i<s->msg_iovlen; ++i) {
        AlignIOV_32(d->msg_iov+i, s_iov+i);
    }
    d->msg_iovlen = s->msg_iovlen;
    d->msg_control = from_ptrv(s->msg_control);
    d->msg_controllen = s->msg_controllen;
    d->msg_flags = s->msg_flags;
}

