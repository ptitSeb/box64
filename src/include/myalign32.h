#ifndef __MY_ALIGN32__H_
#define __MY_ALIGN32__H_
#include <stdint.h>
#include <net/if.h>
#include "box32.h"

#define X64_VA_MAX_REG  (6*8)
#define X64_VA_MAX_XMM  ((6*8)+(8*16))

#define ALIGN64_16(A) (uint64_t*)((((uintptr_t)A)+15)&~15LL)

#ifdef __x86_64__
// x86_64, 6 64bits general regs and 16 or 8? 128bits float regs
/*
For reference, here is the x86_64 va_list structure
typedef struct {
   unsigned int gp_offset;
   unsigned int fp_offset;
   void *overflow_arg_area;
   void *reg_save_area;
} va_list[1];
*/
#define CREATE_SYSV_VALIST_32(A)          \
  va_list sysv_varargs;                   \
  sysv_varargs->gp_offset=X64_VA_MAX_REG; \
  sysv_varargs->fp_offset=X64_VA_MAX_XMM; \
  sysv_varargs->reg_save_area=(A);        \
  sysv_varargs->overflow_arg_area=A

#define CONVERT_VALIST_32(A)              \
  va_list sysv_varargs;                   \
  sysv_varargs->gp_offset=X64_VA_MAX_REG; \
  sysv_varargs->fp_offset=X64_VA_MAX_XMM; \
  sysv_varargs->reg_save_area=(A);        \
  sysv_varargs->overflow_arg_area=A



#elif defined(__aarch64__)
// aarch64: 8 64bits general regs and 8 128bits float regs
/*
va_list declared as
typedef struct  va_list {
    void * stack; // next stack param
    void * gr_top; // end of GP arg reg save area
    void * vr_top; // end of FP/SIMD arg reg save area
    int gr_offs; // offset from  gr_top to next GP register arg
    int vr_offs; // offset from  vr_top to next FP/SIMD register arg
} va_list;
*/
#define CREATE_SYSV_VALIST_32(A) \
  va_list sysv_varargs; \
  sysv_varargs.__gr_offs=(8*8); \
  sysv_varargs.__vr_offs=(8*16); \
  sysv_varargs.__stack=(A);

#define CONVERT_VALIST_32(A)                                     \
  va_list sysv_varargs;                                          \
  sysv_varargs.__gr_offs=(8*8);                                  \
  sysv_varargs.__vr_offs=(8*16);                                 \
  sysv_varargs.__stack=(A);


#elif defined(__loongarch64) || defined(__powerpc64__) || defined(__riscv)
#define CREATE_SYSV_VALIST_32(A) \
  va_list sysv_varargs = (va_list)A

#define CREATE_VALIST_FROM_VALIST_32(VA, SCRATCH)                          \
  va_list sysv_varargs = (va_list)A

#else
#error Unknown architecture!
#endif

#define VARARGS_32 sysv_varargs
#define PREPARE_VALIST_32 CREATE_SYSV_VALIST_32(emu->scratch)
#define VARARGS_32_(A) sysv_varargs
#define PREPARE_VALIST_32_(A) CREATE_SYSV_VALIST_32(A)

void myStackAlign32(const char* fmt, uint32_t* st, uint64_t* mystack);
size_t myStackAlignScanf32(const char* fmt, uint32_t* st, uint64_t* mystack, size_t nb_elem); // return the number of long/ptr_t conversion pending
void myStackAlignScanf32_final(const char* fmt, uint32_t* st, uint64_t* mystack, size_t nb_elem, int n); // convert the long/ptr_t scanf results
size_t myStackAlignScanfW32(const char* fmt, uint32_t* st, uint64_t* mystack, size_t nb_elem);
void myStackAlignScanfW32_final(const char* fmt, uint32_t* st, uint64_t* mystack, size_t nb_elem, int n);
void myStackAlignW32(const char* fmt, uint32_t* st, uint64_t* mystack);

void* align_xcb_connection32(void* src);
void unalign_xcb_connection32(void* src, void* dst);
void* add_xcb_connection32(void* src);
void del_xcb_connection32(void* src);

void UnalignStat64_32(const void* source, void* dest);
void UnalignStat64_32_t64(const void* source, void* dest);

void UnalignStatFS_32(const void* source, void* dest);
void UnalignStatFS64_32(const void* source, void* dest);
#if 0
void UnalignOggVorbis(void* dest, void* source); // Arm -> x86
void AlignOggVorbis(void* dest, void* source);   // x86 -> Arm

void UnalignVorbisDspState(void* dest, void* source); // Arm -> x86
void AlignVorbisDspState(void* dest, void* source);   // x86 -> Arm

void UnalignVorbisBlock(void* dest, void* source); // Arm -> x86
void AlignVorbisBlock(void* dest, void* source);   // x86 -> Arm
#endif
void UnalignEpollEvent32(void* dest, void* source, int nbr); // Arm -> x86
void AlignEpollEvent32(void* dest, void* source, int nbr); // x86 -> Arm
#if 0
void UnalignSmpegInfo(void* dest, void* source); // Arm -> x86
void AlignSmpegInfo(void* dest, void* source);   // x86 -> Arm
#endif
// stat64 is packed on i386, not on arm (and possibly other structures)
#undef st_atime
#undef st_atime_nsec
#undef st_mtime
#undef st_mtime_nsec
#undef st_ctime
#undef st_ctime_nsec
struct i386_stat64 {
	uint64_t	st_dev;
	uint8_t		__pad0[4];
	uint32_t		__st_ino;
	uint32_t		st_mode;
	uint32_t		st_nlink;
	uint32_t		st_uid;
	uint32_t		st_gid;
	uint64_t	st_rdev;
	uint8_t		__pad3[4];
	int64_t		st_size;
	uint32_t		st_blksize;
	uint64_t		st_blocks;
	uint32_t	st_atime;
	uint32_t	st_atime_nsec;
	uint32_t	st_mtime;
	uint32_t	st_mtime_nsec;
	uint32_t	st_ctime;
	uint32_t	st_ctime_nsec;
	uint64_t	st_ino;
} __attribute__((packed, aligned(4)));  // important for this one

struct i386_stat64_t64 {
	uint64_t	  st_dev;
	uint64_t	  st_ino;
	uint32_t		st_mode;
	uint32_t		st_nlink;
	uint32_t		st_uid;
	uint32_t		st_gid;
	uint64_t	  st_rdev;
	int64_t		  st_size;
	uint32_t		st_blksize;
	int64_t		  st_blocks;
	uint64_t	  st_atime;
	uint64_t	  st_atime_nsec;
	uint64_t	  st_mtime;
	uint64_t	  st_mtime_nsec;
	uint64_t	  st_ctime;
	uint64_t	  st_ctime_nsec;
	uint32_t	  __reserved4;
  uint32_t	  __reserved5;
} __attribute__((packed, aligned(4)));  // important for this one

struct i386_fsid {
  int     val[2];
};

struct i386_statfs {
  uint32_t    f_type;
  uint32_t    f_bsize;
  uint32_t    f_blocks;
  uint32_t    f_bfree;
  uint32_t    f_bavail;
  uint32_t    f_files;
  uint32_t    f_ffree;
  struct i386_fsid f_fsid;
  uint32_t    f_namelen;
  uint32_t    f_frsize;
  uint32_t    f_flags;
  uint32_t    f_spare[4];
};

struct i386_statfs64 {
  long_t      f_type;
  long_t      f_bsize;
  uint64_t    f_blocks;
  uint64_t    f_bfree;
  uint64_t    f_bavail;
  uint64_t    f_files;
  uint64_t    f_ffree;
  struct i386_fsid f_fsid;
  long_t      f_namelen;
  long_t      f_frsize;
  long_t      f_flags;
  long_t      f_spare[4];
};

struct i386_statvfs64 {
  ulong_t f_bsize;
  ulong_t f_frsize;
  uint64_t f_blocks;
  uint64_t f_bfree;
  uint64_t f_bavail;
  uint64_t f_files;
  uint64_t f_ffree;
  uint64_t f_favail;
  ulong_t f_fsid;
  int __f_unused;
  ulong_t f_flag;
  ulong_t f_namemax;
  unsigned int f_type;
  int __f_spare[5];
};

struct i386_statvfs {
  ulong_t f_bsize;
  ulong_t f_frsize;
  uint32_t f_blocks;
  uint32_t f_bfree;
  uint32_t f_bavail;
  uint32_t f_files;
  uint32_t f_ffree;
  uint32_t f_favail;
  ulong_t f_fsid;
  int __f_unused;
  ulong_t f_flag;
  ulong_t f_namemax;
  unsigned int f_type;
  int __f_spare[5];
};

void UnalignStatVFS_32(const void* source, void* dest);
void UnalignStatVFS64_32(const void* source, void* dest);

struct i386_dirent
{
  ulong_t d_ino;
  ulong_t d_off;
  uint16_t d_reclen;
  uint8_t d_type;
  char d_name[256];
};
void UnalignDirent_32(const void* source, void* dest);

#if 0
typedef struct {
  unsigned char *data;
  int storage;
  int fill;
  int returned;

  int unsynced;
  int headerbytes;
  int bodybytes;
} ogg_sync_state;

typedef struct {
  unsigned char   *body_data;    /* bytes from packet bodies */
  long    body_storage;          /* storage elements allocated */
  long    body_fill;             /* elements stored; fill mark */
  long    body_returned;         /* elements of fill returned */


  int     *lacing_vals;      /* The values that will go to the segment table */
  int64_t *granule_vals; /* granulepos values for headers. Not compact
                                this way, but it is simple coupled to the
                                lacing fifo */
  long    lacing_storage;
  long    lacing_fill;
  long    lacing_packet;
  long    lacing_returned;

  unsigned char    header[282];      /* working space for header encode */
  int              header_fill;

  int     e_o_s;          /* set when we have buffered the last packet in the
                             logical bitstream */
  int     b_o_s;          /* set after we've written the initial page
                             of a logical bitstream */
  long    serialno;
  long    pageno;
  int64_t  packetno;  /* sequence number for decode; the framing
                             knows where there's a hole in the data,
                             but we need coupling so that the codec
                             (which is in a separate abstraction
                             layer) also knows about the gap */
  int64_t   granulepos;

} ogg_stream_state;

typedef struct vorbis_dsp_state {
  int analysisp;
  ptr_t vi; //vorbis_info

  ptr_t  pcm;  //float**
  ptr_t pcmret; // float**
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
} vorbis_dsp_state;

typedef struct {
  long endbyte;
  int  endbit;

  unsigned char *buffer;
  unsigned char *ptr;
  long storage;
} oggpack_buffer;

typedef struct vorbis_block {
  /* necessary stream state for linking to the framing abstraction */
  float  **pcm;       /* this is a pointer into local storage */
  oggpack_buffer opb;

  long  lW;
  long  W;
  long  nW;
  int   pcmend;
  int   mode;

  int         eofflag;
  int64_t granulepos;
  int64_t sequence;
  vorbis_dsp_state *vd; /* For read-only access of configuration */

  /* local storage to avoid remallocing; it's up to the mapping to
     structure it */
  void               *localstore;
  long                localtop;
  long                localalloc;
  long                totaluse;
  struct alloc_chain *reap;

  /* bitmetrics for the frame */
  long glue_bits;
  long time_bits;
  long floor_bits;
  long res_bits;

  void *internal;

} vorbis_block;

typedef struct {
  size_t (*read_func)  (void *ptr, size_t size, size_t nmemb, void *datasource);
  int    (*seek_func)  (void *datasource, int64_t offset, int whence);
  int    (*close_func) (void *datasource);
  long   (*tell_func)  (void *datasource);
} ov_callbacks;

typedef struct OggVorbis  {
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

  ogg_stream_state os; /* take physical pages, weld into a logical
                          stream of packets */
  vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
  vorbis_block     vb; /* local working space for packet->PCM decode */

  ov_callbacks callbacks;

} OggVorbis;

typedef struct my_SMPEG_Info_s {
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
} my_SMPEG_Info_t;

typedef struct  __attribute__((packed)) x86_ftsent_s {
        struct x86_ftsent_s *fts_cycle;
        struct x86_ftsent_s *fts_parent;
        struct x86_ftsent_s *fts_link;
        long fts_number;
        void *fts_pointer;
        char *fts_accpath;
        char *fts_path;
        int fts_errno;
        int fts_symfd;
        uint16_t fts_pathlen;
        uint16_t fts_namelen;
        uintptr_t fts_ino;
        uint64_t fts_dev;
        uint32_t fts_nlink;
        int16_t fts_level;
        uint16_t fts_info;
        uint16_t fts_flags;
        uint16_t fts_instr;
        struct stat *fts_statp;
        char fts_name[1];
} x86_ftsent_t;

void UnalignFTSENT(void* dest, void* source); // Arm -> x86
void AlignFTSENT(void* dest, void* source);   // x86 -> Arm
#endif
typedef struct my_flock64_s {
	uint16_t  l_type;
	uint16_t  l_whence;
	int64_t   l_start;
	int64_t   l_len;
	int       l_pid;
} my_flock64_t;

typedef struct __attribute__((packed, aligned(4))) x86_flock64_s {  // alignment needed here to avoid 8bytes align before the start
	uint16_t  l_type;
	uint16_t  l_whence;
	int64_t   l_start;
	int64_t   l_len;
	int       l_pid;
} x86_flock64_t;

void UnalignFlock64_32(void* dest, void* source); // Arm -> x86
void AlignFlock64_32(void* dest, void* source);   // x86 -> Arm

typedef struct __attribute__((packed, aligned(4))) i386_flock_s
{
    short int l_type;
    short int l_whence;
    long_t l_start;
    long_t l_len;
    int l_pid;
} i386_flock_t;

void UnalignFlock_32(void* dest, void* source); // Arm -> x86
void AlignFlock_32(void* dest, void* source);   // x86 -> Arm

#if 0
// defined in wrapperlibc.c
int of_convert(int);    // x86->arm
int of_unconvert(int);  // arm->x86

typedef struct my_GValue_s
{
  int         g_type;
  union {
    int        v_int;
    int64_t    v_int64;
    uint64_t   v_uint64;
    float      v_float;
    double     v_double;
    void*      v_pointer;
  } data[2];
} my_GValue_t;

void alignNGValue(my_GValue_t* v, void* value, int n);
void unalignNGValue(void* value, my_GValue_t* v, int n);
#endif

int of_convert32(int a);
int of_unconvert32(int a);

struct i386_addrinfo
{
  int ai_flags;
  int ai_family;
  int ai_socktype;
  int ai_protocol;
  uint32_t ai_addrlen;
  ptr_t ai_addr;	    // struct sockaddr *
  ptr_t ai_canonname;   // char *
  ptr_t ai_next;        // struct addrinfo *
};

struct i386_hostent {
    ptr_t  h_name;     // char  *
    ptr_t  h_aliases;  // char **
    int    h_addrtype;
    int    h_length;
    ptr_t  h_addr_list;// char **
};

struct i386_servent {
    ptr_t  s_name;     // char  *
    ptr_t  s_aliases;  // char **
    int    s_port;
    ptr_t  s_proto;     // char *
};

struct i386_iovec
{
  ptr_t     iov_base; // void *
  ulong_t   iov_len;
};

struct i386_msghdr
{
  ptr_t     msg_name;		// void *
  uint32_t  msg_namelen;
  ptr_t     msg_iov;	// struct i386_iovec *
  uint32_t  msg_iovlen;
  ptr_t     msg_control;  // void *
  ulong_t   msg_controllen;
  int msg_flags;
};

struct i386_mmsghdr {
    struct i386_msghdr msg_hdr;
    unsigned int       msg_len;
};

// Some docs show cmsg_len as a socklen_t (so uint32_t), but thsi not true, it's a size_t (kernel_size_t)
struct i386_cmsghdr
{
  ulong_t cmsg_len;
  int cmsg_level;
  int cmsg_type;
};

void AlignIOV_32(void* dest, void* source);   // x86 -> Native
void UnalignIOV_32(void* dest, void* source); // Native -> x86

void AlignMsgHdr_32(void* dest, void* dest_iov, void* dest_cmsg, void* source, int convert_control);    // x86 -> Native
void UnalignMsgHdr_32(void* dest, void* source); // back to Native -> x86

struct i386_passwd
{
  ptr_t pw_name; // char*
  ptr_t pw_passwd; // char*
  __uid_t pw_uid;
  __gid_t pw_gid;
  ptr_t pw_gecos; // char*
  ptr_t pw_dir; // char*
  ptr_t pw_shell; // char*
} __attribute__((packed, aligned(4)));

struct i386_group
{
  ptr_t gr_name; // char *
  ptr_t gr_passwd; // char *
  __gid_t gr_gid;
  ptr_t gr_mem; // char **
} __attribute__((packed, aligned(4)));

typedef struct my_regex_s
{
  void*             buffer;   //struct re_dfa_t
  size_t            allocated;
  size_t            used;
  unsigned long int syntax;
  char*             fastmap;
  unsigned char*    translate;
  size_t            re_nsub;
  unsigned          flags;
  /*
  unsigned          can_be_null : 1;
  unsigned          regs_allocated : 2;
  unsigned          fastmap_accurate : 1;
  unsigned          no_sub : 1;
  unsigned          not_bol : 1;
  unsigned          not_eol : 1;
  unsigned          newline_anchor : 1;
  */
} my_regex_t;

typedef struct my_regex_32_s
{
  ptr_t             buffer;   //struct re_dfa_t
  ulong_t           allocated;
  ulong_t           used;
  ulong_t           syntax;
  ptr_t             fastmap;  //char*
  ptr_t             translate;  //unsigned char*
  ulong_t           re_nsub;
  unsigned          flags;
  /*
  unsigned          can_be_null : 1;
  unsigned          regs_allocated : 2;
  unsigned          fastmap_accurate : 1;
  unsigned          no_sub : 1;
  unsigned          not_bol : 1;
  unsigned          not_eol : 1;
  unsigned          newline_anchor : 1;
  */
} my_regex_32_t;

void convert_regext_to_32(void* d, void* s);
void convert_regext_to_64(void* d, void* s);

typedef struct my_ns_msg_32_s {
	ptr_t     _msg; //const unsigned char	*_
  ptr_t     _eom; //const unsigned char	*_
	uint16_t	_id;
  uint16_t  _flags;
  uint16_t  _counts[4];
	ptr_t     _sections[4];//const unsigned char	*_
	uint32_t	_sect;
	int			  _rrnum;
	ptr_t     _msg_ptr; //const unsigned char	*
} my_ns_msg_32_t;

typedef	struct my_ns_rr_32_s {
	char			  name[1025];
	uint16_t		type;
	uint16_t		rr_class;
	uint32_t		ttl;
	uint16_t		rdlength;
	ptr_t	      rdata;  //const unsigned char *
} my_ns_rr_32_t;

struct my_obstack_chunk_32_t
{
  ptr_t  limit; //char *
  ptr_t  prev;  //struct _obstack_chunk *
  char contents[4];
};

struct my_obstack_32_t
{
  long_t chunk_size;
  ptr_t  chunk; //struct _obstack_chunk *
  ptr_t  object_base; //char*
  ptr_t  next_free; //char*
  ptr_t  chunk_limit; //char*
  union
  {
    int tempint;
    ptr_t tempptr;  //void*
  } temp;
  int alignment_mask;
  ptr_t chunkfun; //struct _obstack_chunk *(*chunkfun) (void *, long);
  ptr_t freefun; //void (*freefun) (void *, struct _obstack_chunk *);
  ptr_t extra_arg;//void *
  unsigned use_extra_arg : 1;
  unsigned maybe_empty_object : 1;
  unsigned alloc_failed : 1;
};

void convert_obstack_to_32(void* d, void* s);
void convert_obstack_to_64(void* d, void* s);

typedef struct i386_ifmap_s {
  ulong_t         mem_start;
  ulong_t         mem_end;
  unsigned short  base_addr;
  unsigned char   irq;
  unsigned char   dma;
  unsigned char   port;
} i386_ifmap_t;

typedef struct i386_ifreq_s {
  char i386_ifr_name[16]; /* Interface name */
  union {
    struct sockaddr i386_ifr_addr;
    struct sockaddr i386_ifr_dstaddr;
    struct sockaddr i386_ifr_broadaddr;
    struct sockaddr i386_ifr_netmask;
    struct sockaddr i386_ifr_hwaddr;
    short           i386_ifr_flags;
    int             i386_ifr_ifindex;
    int             i386_ifr_metric;
    int             i386_ifr_mtu;
    i386_ifmap_t    i386_ifr_map;
    char            i386_ifr_slave[16];
    char            i386_ifr_newname[16];
    ptr_t           i386_ifr_data; // char*
  };
} i386_ifreq_t;

typedef struct i386_ifconf_s {
  int ifc_len;
  union {
    ptr_t i386_ifc_buf; // char*
    ptr_t i386_ifc_req; // i386_ifreq_t*
  };
} i386_ifconf_t;

#endif//__MY_ALIGN32__H_
