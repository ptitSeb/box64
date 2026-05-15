#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#if defined(DYNAREC) && !defined(WIN32)
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#endif

#include "os.h"
#include "env.h"
#include "custommem.h"
#include "khash.h"
#include "debug.h"
#include "fileutils.h"
#include "dynacache.h"
#include "dynacache_compress.h"
#include "env_private.h"
#ifdef DYNAREC
#include "dynacache_hashes.h"
#endif

#ifdef _WIN32
#define PATHSEP "\\"
#define HOME    "USERPROFILE"
#else
#define PATHSEP "/"
#define HOME    "HOME"
#endif

#ifdef DYNAREC
int nUnalignedRange(uintptr_t start, size_t size);
void getUnalignedRange(uintptr_t start, size_t size, uintptr_t addrs[]);
void add_unaligned_address(uintptr_t addr);

const char* GetDynacacheFolder(mapping_t* mapping)
{
    static char folder[4096] = { 0 };
    if(mapping && mapping->env && mapping->env->is_dynacache_folder_overridden && mapping->env->dynacache_folder) {
        if (FileExist(mapping->env->dynacache_folder, 0) || MakeDir(mapping->env->dynacache_folder)) {
            strcpy(folder, mapping->env->dynacache_folder);
            goto done;
        }
    } else if (box64env.dynacache_folder) {
        if (FileExist(box64env.dynacache_folder, 0) || MakeDir(box64env.dynacache_folder)) {
            strcpy(folder, box64env.dynacache_folder);
            goto done;
        }
    }

    if(GetEnv("XDG_CACHE_HOME"))
        strcpy(folder, GetEnv("XDG_CACHE_HOME"));
    else if(GetEnv(HOME)) {
        strcpy(folder, GetEnv(HOME));
        strcat(folder, PATHSEP);
        strcat(folder, ".cache");
        if (!FileExist(folder, 0) && !MakeDir(folder))
            return NULL;

    } else
        return NULL;
    strcat(folder, PATHSEP);
    strcat(folder, "box64");
    if (!FileExist(folder, 0) && !MakeDir(folder))
        return NULL;

done:
    if (folder[strlen(folder) - 1] != PATHSEP[0]) {
        strcat(folder, PATHSEP);
    }
    return folder;
}

/*
    There are 3 compatibility values for DynaCache:
    1. FILE_VERSION for the DynaCache infrastructure and file format
    2. DYNAREC_VERSION as the generated hash of common dynarec sources
    3. ARCH_VERSION as the generated hash of the active backend sources

    Dynacache will ignore any DynaCache file not exactly matching those 3 version.
    `box64 --dynacache-clean` can be used from command line to purge obsolete DyaCache files
*/

#define FILE_VERSION 3
#define HEADER_SIGN  "DynaCache"

typedef struct DynaCacheHeader_s {
    char sign[10];  //"DynaCache\0"
    uint64_t    file_version:16;
    uint64_t    dynarec_version:24;
    uint64_t    arch_version:24;
    uint64_t    cpuext;
    uint64_t    dynarec_settings;
    size_t      pagesize;
    size_t      codesize;
    uintptr_t   map_addr;
    size_t      map_len;
    size_t      file_length;
    uint32_t    filename_length;
    uint32_t    nblocks;
    uint32_t    nLockAddresses;
    uint32_t    nUnalignedAddresses;
    char        filename[];
} DynaCacheHeader_t;

#define DYNAREC_SETTINGS()                                              \
    DS_GO(BOX64_DYNAREC_ALIGNED_ATOMICS, dynarec_aligned_atomics, 1)    \
    DS_GO(BOX64_DYNAREC_BIGBLOCK, dynarec_bigblock, 2)                  \
    DS_GO(BOX64_DYNAREC_CALLRET, dynarec_callret, 2)                    \
    DS_GO(BOX64_DYNAREC_DF, dynarec_df, 1)                              \
    DS_GO(BOX64_DYNAREC_DIRTY, dynarec_dirty, 2)                        \
    DS_GO(BOX64_DYNAREC_DIV0, dynarec_div0, 1)                          \
    DS_GO(BOX64_DYNAREC_FASTNAN, dynarec_fastnan, 1)                    \
    DS_GO(BOX64_DYNAREC_FASTROUND, dynarec_fastround, 2)                \
    DS_GO(BOX64_DYNAREC_FORWARD, dynarec_forward, 10)                   \
    DS_GO(BOX64_DYNAREC_NATIVEFLAGS, dynarec_nativeflags, 1)            \
    DS_GO(BOX64_DYNAREC_SAFEFLAGS, dynarec_safeflags, 2)                \
    DS_GO(BOX64_DYNAREC_STRONGMEM, dynarec_strongmem, 2)                \
    DS_GO(BOX64_DYNAREC_VOLATILE_METADATA, dynarec_volatile_metadata, 1)\
    DS_GO(BOX64_DYNAREC_WEAKBARRIER, dynarec_weakbarrier, 2)            \
    DS_GO(BOX64_DYNAREC_X87DOUBLE, dynarec_x87double, 2)                \
    DS_GO(BOX64_DYNAREC_NOARCH, dynarec_noarch, 2)                      \
    DS_GO(BOX64_AES, aes, 1)                                            \
    DS_GO(BOX64_PCLMULQDQ, pclmulqdq, 1)                                \
    DS_GO(BOX64_SHAEXT, shaext, 1)                                      \
    DS_GO(BOX64_SSE42, sse42, 1)                                        \
    DS_GO(BOX64_AVX, avx, 2)                                            \
    DS_GO(BOX64_X87_NO80BITS, x87_no80bits, 1)                          \
    DS_GO(BOX64_RDTSC_1GHZ, rdtsc_1ghz, 1)                              \
    DS_GO(BOX64_SSE_FLUSHTO0, sse_flushto0, 1)                          \

#define DS_GO(A, B, C) uint64_t B:C;
typedef union dynarec_settings_s {
    struct {
        DYNAREC_SETTINGS()
    };
    uint64_t    x;
} dynarec_settings_t;
#undef DS_GO
uint64_t GetDynSetting(mapping_t* mapping)
{
    dynarec_settings_t settings = {0};
    #define DS_GO(A, B, C)  settings.B = (mapping->env && mapping->env->is_##B##_overridden)?mapping->env->B:box64env.B;
    DYNAREC_SETTINGS()
    #undef DS_GO
    return settings.x;
}
void PrintDynfSettings(int level, uint64_t s)
{
    dynarec_settings_t settings = {0};
    settings.x = s;
    #define DS_GO(A, B, C) if(settings.B) printf_log_prefix(0, level, "\t\t" #A "=%d\n", settings.B);
    DYNAREC_SETTINGS()
    #undef DS_GO
}
#undef DYNAREC_SETTINGS

char* MmaplistName(const char* filename, uint64_t dynarec_settings, const char* fullname)
{
    // names are FOLDER/filename-YYYYY-XXXXX.box64
    // Where XXXXX is the hash of the full name
    // and YYYY is the Dynarec optim (in hex)
    static char mapname[4096];
    snprintf(mapname, 4095-6, "%s-%llx-%u", filename, dynarec_settings, __ac_X31_hash_string(fullname));
    strcat(mapname, ".box64");
    return mapname;
}

char* GetMmaplistName(mapping_t* mapping)
{
    return MmaplistName(mapping->filename+1, GetDynSetting(mapping), mapping->fullname);
}

const char* NicePrintSize(size_t sz)
{
    static char buf[256];
    const char* units[] = {"", "kb", "Mb", "Gb"};
    int idx = 0;
    size_t ratio = 0;
    while(idx<sizeof(units)/sizeof(units[0]) && (1<<(ratio+10))<sz) {
        ratio+=10;
        ++idx;
    }
    if(ratio && (sz>>ratio)<50) {
        snprintf(buf, 255, "%zd.%zd%s", sz>>ratio, (sz>>(ratio-1))%10, units[idx]);
    } else {
        snprintf(buf, 255, "%zd%s", sz>>ratio, units[idx]);
    }
    return buf;
}

#ifndef WIN32
typedef struct DynaCacheFileEntry_s {
    char* filename;
    uint64_t size;
    time_t mtime;
    int valid;
} DynaCacheFileEntry_t;

int ReadDynaCache(const char* folder, const char* name, mapping_t* mapping, int verbose);

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

#define DYNACACHE_TMP_STALE_SECONDS 60

static void SyncDynaCacheFolder(const char* folder)
{
#ifdef O_DIRECTORY
    int fd = open(folder, O_RDONLY|O_DIRECTORY|O_CLOEXEC);
#else
    int fd = open(folder, O_RDONLY|O_CLOEXEC);
#endif
    if(fd>=0) {
        fsync(fd);
        close(fd);
    }
}

static int DynaCacheOpenTempFile(const char* mapname, char* tmpname, size_t tmpname_size)
{
    for(int i=0; i<100; ++i) {
        snprintf(tmpname, tmpname_size, "%s.tmp.%ld.%d", mapname, (long)getpid(), i);
        int fd = open(tmpname, O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC, 0644);
        if(fd>=0)
            return fd;
        if(errno!=EEXIST)
            return -1;
    }
    errno = EEXIST;
    return -1;
}

static int RemoveStaleDynaCacheTempFile(const char* folder, const char* name)
{
    if(!strstr(name, ".box64.tmp."))
        return 0;

    char filename[strlen(folder)+strlen(name)+1];
    strcpy(filename, folder);
    strcat(filename, name);

    struct stat st = {0};
    if(lstat(filename, &st) || !S_ISREG(st.st_mode) || st.st_size<0)
        return 0;

    time_t now = time(NULL);
    if(now==(time_t)-1 || st.st_mtime > now-DYNACACHE_TMP_STALE_SECONDS)
        return 0;

    size_t size = st.st_size;
    if(unlink(filename))
        return 0;

    dynarec_log(LOG_INFO, "DynaCache removed stale temporary cache file %s (%s)\n", filename, NicePrintSize(size));
    return 1;
}

static int DynaCacheFileEntryCmp(const void* a, const void* b)
{
    const DynaCacheFileEntry_t* fa = a;
    const DynaCacheFileEntry_t* fb = b;
    if(fa->mtime < fb->mtime)
        return -1;
    if(fa->mtime > fb->mtime)
        return 1;
    return strcmp(fa->filename, fb->filename);
}

static void PruneDynaCacheFolder(const char* folder, uint64_t max_size)
{
    if(!max_size)
        return;
    DIR* dir = opendir(folder);
    if(!dir) return;

    DynaCacheFileEntry_t* files = NULL;
    size_t count = 0;
    size_t capacity = 0;
    uint64_t total = 0;

    // collect all dynacache files in the folder
    struct dirent* d = NULL;
    while((d = readdir(dir))) {
        size_t l = strlen(d->d_name);
        if(RemoveStaleDynaCacheTempFile(folder, d->d_name))
            continue;
        if(l<=6 || strcmp(d->d_name+l-6, ".box64")) continue;

        char* filename = box_malloc(strlen(folder)+l+1);
        strcpy(filename, folder);
        strcat(filename, d->d_name);

        struct stat st = {0};
        if(lstat(filename, &st) || !S_ISREG(st.st_mode) || st.st_size<0) {
            box_free(filename);
            continue;
        }

        if(count == capacity) {
            size_t new_capacity = capacity ? capacity*2 : 16;
            DynaCacheFileEntry_t* new_files = box_realloc(files, new_capacity*sizeof(*files));
            files = new_files;
            capacity = new_capacity;
        }

        files[count].filename = filename;
        files[count].size = st.st_size;
        files[count].mtime = st.st_mtime;
        files[count].valid = -1;
        total += files[count].size;
        ++count;
    }
    closedir(dir);

    // start pruning, invalid files first, then older ones until we are under 80% of set max size
    if(total > max_size && count) {
        uint64_t target_size = (max_size/5) * 4;
        size_t folder_len = strlen(folder);
        for(size_t i=0; i<count && total>target_size; ++i) {
            files[i].valid = (ReadDynaCache(folder, files[i].filename+folder_len, NULL, 0)==0);
            if(files[i].valid) continue;
            uint64_t size = files[i].size;
            if(!unlink(files[i].filename)) {
                dynarec_log(LOG_INFO, "DynaCache removed invalid cache file %s (%s)\n", files[i].filename, NicePrintSize(size));
                files[i].size = 0;
                total = (size>total)?0:total-size;
            }
        }
        qsort(files, count, sizeof(*files), DynaCacheFileEntryCmp);
        for(size_t i=0; i<count && total>target_size; ++i) {
            if(files[i].valid!=1 || !files[i].size)
                continue;
            uint64_t size = files[i].size;
            if(!unlink(files[i].filename)) {
                dynarec_log(LOG_INFO, "DynaCache removed old cache file %s (%s)\n", files[i].filename, NicePrintSize(size));
                total = (size>total)?0:total-size;
            }
        }
    }

    for(size_t i=0; i<count; ++i) box_free(files[i].filename);
    box_free(files);
}
#endif

void SerializeMmaplist(mapping_t* mapping)
{
    if(!DYNAREC_VERSION)
        return;
    if(mapping->env && mapping->env->is_dynacache_overridden && (mapping->env->dynacache!=1))
        return;
    if((!mapping->env || !mapping->env->is_dynacache_overridden) && box64env.dynacache!=1)
        return;
    // don't do serialize for program that needs purge=1
    if(mapping->env && mapping->env->is_dynarec_purge_overridden && mapping->env->dynarec_purge)
        return;
    if((!mapping->env || !mapping->env->is_dynarec_purge_overridden) && box64env.dynarec_purge)
        return;
    // don't do serialize for program that needs dirty=1
    if(mapping->env && mapping->env->is_dynarec_dirty_overridden && mapping->env->dynarec_dirty)
        return;
    if((!mapping->env || !mapping->env->is_dynarec_dirty_overridden) && box64env.dynarec_dirty)
        return;
    const char* folder = GetDynacacheFolder(mapping);
    if(!folder) return; // no folder, no serialize...
    const char* name = GetMmaplistName(mapping);
    if(!name) return;
    char mapname[strlen(folder)+strlen(name)+1];
    strcpy(mapname, folder);
    strcat(mapname, name);
    size_t filesize = FileSize(mapping->fullname);
    if(!filesize) {
        dynarec_log(LOG_INFO, "DynaCache will not serialize cache for %s because filesize is 0\n", mapping->fullname);
        return;   // mmaped file as a 0 size...
    }
    // prepare header
    int nblocks = MmaplistNBlocks(mapping->mmaplist);
    if(!nblocks) {
        dynarec_log(LOG_INFO, "DynaCache will not serialize cache for %s because nblocks is 0\n", mapping->fullname);
        return; //How???
    }
    size_t map_len = SizeFileMapped(mapping->start);
    size_t nLockAddresses = nLockAddressRange(mapping->start, map_len);
    size_t nUnaligned = nUnalignedRange(mapping->start, map_len);
    size_t total = sizeof(DynaCacheHeader_t) + strlen(mapping->fullname) + 1 + nblocks*sizeof(CompressedDynaCacheBlock_t) + nLockAddresses*sizeof(uintptr_t) + nUnaligned*sizeof(uintptr_t);;
    total = ALIGN(total); // align on pagesize
    uint8_t all_header[total];
    memset(all_header, 0, total);
    void* p = all_header;
    DynaCacheHeader_t* header = p;
    strcpy(header->sign, HEADER_SIGN);
    header->file_version = FILE_VERSION;
    header->dynarec_version = DYNAREC_VERSION;
    header->arch_version = ARCH_VERSION;
    header->dynarec_settings = GetDynSetting(mapping);
    header->cpuext = cpuext.x;
    header->pagesize = box64_pagesize;
    header->codesize = MmaplistTotalAlloc(mapping->mmaplist);
    header->map_addr = mapping->start;
    header->file_length = filesize;
    header->filename_length = strlen(mapping->fullname);
    header->nblocks = MmaplistNBlocks(mapping->mmaplist);
    header->map_len = map_len;
    header->nLockAddresses = nLockAddresses;
    header->nUnalignedAddresses = nUnaligned;
    size_t dynacache_min = box64env.dynacache_min;
    if(mapping->env && mapping->env->is_dynacache_min_overridden)
        dynacache_min = mapping->env->dynacache_min;
    if(dynacache_min*1024>header->codesize) {
        dynarec_log(LOG_INFO, "DynaCache will not serialize cache for %s because there is not enough usefull code (%s)\n", mapping->fullname, NicePrintSize(header->codesize));
        return; // not enugh code, do no write
    }
    p += sizeof(DynaCacheHeader_t); // fullname
    strcpy(p, mapping->fullname);
    p += strlen(p) + 1; // blocks
    CompressedDynaCacheBlock_t* blocks = p;
    MmaplistFillBlocks(mapping->mmaplist, blocks);
    p += nblocks*sizeof(CompressedDynaCacheBlock_t);
    uintptr_t* lockAddresses = p;
    p += nLockAddresses*sizeof(uintptr_t);
    uintptr_t* unalignedAddresses = p;
    if(nLockAddresses)
        getLockAddressRange(mapping->start, map_len, lockAddresses);
    if(nUnaligned)
        getUnalignedRange(mapping->start, map_len, unalignedAddresses);
    // all done, now just create the file and write all this down...
    #ifndef WIN32
    char tmpname[strlen(mapname)+64];
    int tmpfd = DynaCacheOpenTempFile(mapname, tmpname, sizeof(tmpname));
    if(tmpfd<0) {
        dynarec_log(LOG_INFO, "Cannot create cache file %s\n", mapname);
        return;
    }
    FILE* f = fdopen(tmpfd, "wb");
    if(!f) {
        close(tmpfd);
        unlink(tmpname);
        dynarec_log(LOG_INFO, "Cannot create cache file %s\n", mapname);
        return;
    }
    int write_error = 0;
    if(fwrite(all_header, total, 1, f)!=1)
        write_error = 1;
    int rewrite_header = 0;
    int type = BOX64ENV(dynacache_compress);
    if(type!=COMP_NONE) {
        for(int i=0; i<nblocks && !write_error; ++i) {
            size_t sz = 0;
            void* dest = dc_compress(blocks[i].block.block, blocks[i].block.size, type, &sz);
            if(dest) {
                if(fwrite(dest, sz, 1, f)!=1)
                    write_error = 1;
                else {
                    blocks[i].type = type;
                    blocks[i].compsize = sz;
                    rewrite_header = 1;
                    // align to pagesize...
                    if(sz&(box64_pagesize-1)) {
                        size_t align = box64_pagesize - (sz&(box64_pagesize-1));
                        char tmp[align];
                        memset(tmp, 0, align);
                        if(fwrite(tmp, align, 1, f)!=1)
                            write_error = 1;
                    }
                }
                box_free(dest);
            } else {
                if(fwrite(blocks[i].block.block, blocks[i].block.size, 1, f)!=1)
                    write_error = 1;
            }
        }
    } else
        for(int i=0; i<nblocks && !write_error; ++i) {
            if(fwrite(blocks[i].block.block, blocks[i].block.size, 1, f)!=1) {
                write_error = 1;
            }
        }
    if(!write_error && rewrite_header) {
        fseek(f, 0, SEEK_SET);
        if(fwrite(all_header, total, 1, f)!=1)
            write_error = 1;
    }
    if(!write_error && fflush(f))
        write_error = 1;
    if(!write_error && fsync(tmpfd))
        write_error = 1;
    if(fclose(f))
        write_error = 1;
    if(write_error) {
        dynarec_log(LOG_INFO, "Error writing Cache file (disk full?)\n");
        unlink(tmpname);
        return;
    }
    int dynacache_limit = box64env.dynacache_limit;
    if(mapping->env && mapping->env->is_dynacache_limit_overridden)
        dynacache_limit = mapping->env->dynacache_limit;

    if(rename(tmpname, mapname)) {
        dynarec_log(LOG_INFO, "Cannot publish cache file %s\n", mapname);
        unlink(tmpname);
        return;
    }
    SyncDynaCacheFolder(folder);
    if(dynacache_limit>0) {
        PruneDynaCacheFolder(folder, (uint64_t)dynacache_limit*1024*1024);
        SyncDynaCacheFolder(folder);
    }
    #else
    // TODO?
    #endif
}

#define DCERR_OK            0
#define DCERR_NEXIST        1
#define DCERR_TOOSMALL      2
#define DCERR_FERROR        3
#define DCERR_BADHEADER     4
#define DCERR_FILEVER       5
#define DCERR_DYNVER        6
#define DCERR_DYNARCHVER    7
#define DCERR_PAGESIZE      8
#define DCERR_MAPNEXIST     9
#define DCERR_MAPCHG        10
#define DCERR_RELOC         11
#define DCERR_BADNAME       12
#define DCERR_CPUEXT        13

#ifndef WIN32
int ReadDynaCache(const char* folder, const char* name, mapping_t* mapping, int verbose)
{
    char filename[strlen(folder)+strlen(name)+1];
    strcpy(filename, folder);
    strcat(filename, name);
    if(verbose) printf_log(LOG_NONE, "File %s:\t", name);
    FILE *f = fopen(filename, "rb");
    if(!f) {
        int exists = FileExist(filename, IS_FILE);
        if(verbose) printf_log_prefix(0, LOG_NONE, "%s\n", exists?"Cannot open file":"Invalid file");
        return exists?DCERR_FERROR:DCERR_NEXIST;
    }
    struct stat st = {0};
    if(fstat(fileno(f), &st) || !S_ISREG(st.st_mode) || st.st_size<0) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Cannot stat file\n");
        fclose(f);
        return DCERR_FERROR;
    }
    size_t filesize = st.st_size;
    if(filesize<sizeof(DynaCacheHeader_t)) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Invalid side: %zd\n", filesize);
        fclose(f);
        return DCERR_TOOSMALL;
    }
    DynaCacheHeader_t header = {0};
    if(fread(&header, sizeof(header), 1, f)!=1) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Cannot read header\n");
        fclose(f);
        return DCERR_FERROR;
    }
    if(strcmp(header.sign, HEADER_SIGN)) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Bad header\n");
        fclose(f);
        return DCERR_BADHEADER;
    }
    if (header.file_version != FILE_VERSION) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Incompatible File Version\n");
        fclose(f);
        return DCERR_FILEVER;
    }
    if(header.dynarec_version!=DYNAREC_VERSION) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Incompatible Dynarec Version\n");
        fclose(f);
        return DCERR_DYNVER;
    }
    if(header.arch_version!=ARCH_VERSION) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Incompatible Dynarec Arch Version\n");
        fclose(f);
        return DCERR_DYNARCHVER;
    }
    if(header.pagesize!=box64_pagesize) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Bad pagesize\n");
        fclose(f);
        return DCERR_PAGESIZE;
    }
    if(header.cpuext!=cpuext.x) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Incompatible CPU extensions\n");
        fclose(f);
        return DCERR_CPUEXT;
    }
    char map_filename[header.filename_length+1];
    if(fread(map_filename, header.filename_length+1, 1, f)!=1) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Cannot read filename\n");
        fclose(f);
        return DCERR_FERROR;
    }
    if(!FileExist(map_filename, IS_FILE)) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Mapfiled does not exists\n");
        fclose(f);
        return DCERR_MAPNEXIST;
    }
    if(FileSize(map_filename)!=header.file_length) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "File changed\n");
        fclose(f);
        return DCERR_MAPCHG;
    }
    CompressedDynaCacheBlock_t blocks[header.nblocks];
    if(fread(blocks, sizeof(CompressedDynaCacheBlock_t), header.nblocks, f)!=header.nblocks) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Cannot read blocks\n");
        fclose(f);
        return DCERR_FERROR;
    }
    uintptr_t lockAddresses[header.nLockAddresses];
    if(fread(lockAddresses, sizeof(uintptr_t), header.nLockAddresses, f)!=header.nLockAddresses) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Cannot read lockAddresses\n");
        fclose(f);
        return DCERR_FERROR;
    }
    uintptr_t unalignedAddresses[header.nUnalignedAddresses];
    if(fread(unalignedAddresses, sizeof(uintptr_t), header.nUnalignedAddresses, f)!=header.nUnalignedAddresses) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Cannot read unalignedAddresses\n");
        fclose(f);
        return DCERR_FERROR;
    }
    off_t p = ftell(f);
    p = ALIGN(p);;
    if(fseek(f, p, SEEK_SET)<0) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Error reading a block\n");
        fclose(f);
        return DCERR_FERROR;
    }
    if(!mapping) {
        // check the blocks can be read without reading...
        for(int i=0; i<header.nblocks; ++i) {
            p+=(blocks[i].type==COMP_NONE)?blocks[i].block.size:blocks[i].compsize;
            if(fseek(f, (blocks[i].type==COMP_NONE)?blocks[i].block.size:blocks[i].compsize, SEEK_CUR)<0 || ftell(f)!=p) {
                if(verbose) printf_log_prefix(0, LOG_NONE, "Error reading a block\n");
                fclose(f);
                return DCERR_FERROR;
            }
        }
        char* short_name = strrchr(map_filename, '/');
        if(short_name)
            ++short_name;
        else
            short_name = map_filename;
        short_name = LowerCase(short_name);
        const char* file_name = MmaplistName(short_name, header.dynarec_settings, map_filename);
        box_free(short_name);
        if(strcmp(file_name, name)) {
            if(verbose) printf_log_prefix(0, LOG_NONE, "Invalid cache name\n");
            fclose(f);
            return DCERR_BADNAME;
        }
        if(verbose) {
            // check if name is coherent
            // file is valid, gives informations:
            printf_log_prefix(0, LOG_NONE, "%s (%s)\n", map_filename, NicePrintSize(filesize));
            printf_log_prefix(0, LOG_NONE, "\tDynarec Settings:\n");
            PrintDynfSettings(LOG_NONE, header.dynarec_settings);
            size_t total_blocks = 0, total_free = 0, total_compressed = 0, total_uncompressed = 0;
            size_t total_code = header.codesize;
            for(int i=0; i<header.nblocks; ++i) {
                total_blocks += blocks[i].block.size;
                total_free += blocks[i].block.free_size;
                if(blocks[i].compsize)
                    total_compressed += blocks[i].compsize;
                else
                    total_uncompressed += blocks[i].block.size;
            }
            printf_log_prefix(0, LOG_NONE, "\tHas %d blocks for a total of %s", header.nblocks, NicePrintSize(total_blocks));
            printf_log_prefix(0, LOG_NONE, " with %s still free", NicePrintSize(total_free));
            printf_log_prefix(0, LOG_NONE, " and %s non-canceled blocks (mapped at %p-%p, with %zu lock and %zu unaligned addresses)", NicePrintSize(total_code), (void*)header.map_addr, (void*)header.map_addr+header.map_len, header.nLockAddresses, header.nUnalignedAddresses);
            if(total_compressed) {
                printf_log_prefix(0, LOG_NONE, " with %s compressed blocks", NicePrintSize(total_compressed));
                if(total_uncompressed)
                    printf_log_prefix(0, LOG_NONE, " and %s uncompressed block", NicePrintSize(total_uncompressed));
            }
            printf_log_prefix(0, LOG_NONE, "\n");
        }
    } else {
        // actually reading!
        int fd = fileno(f);
        intptr_t delta_map = mapping->start - header.map_addr;
        dynarec_log(LOG_INFO, "Trying to load DynaCache for %s, with a delta_map=%zx\n", mapping->fullname, delta_map);
        if(!mapping->mmaplist)
            mapping->mmaplist = NewMmaplist();
        MmaplistAddNBlocks(mapping->mmaplist, header.nblocks);
        for(int i=0; i<header.nblocks; ++i) {
            int ret = 0;
            if(blocks[i].type==COMP_NONE)
                ret = MmaplistAddBlock(mapping->mmaplist, fd, p, blocks[i].block.block, blocks[i].block.size, delta_map, mapping->start);
            else {
                void* src = box_malloc(blocks[i].compsize);
                fseek(f, p, SEEK_SET);
                if(!src || (fread(src, blocks[i].compsize, 1, f)!=1)) {
                    ret = -10;
                } else
                    ret = MmaplistAddCompressedBlock(mapping->mmaplist, blocks[i].type, src, blocks[i].compsize, blocks[i].block.block, blocks[i].block.size, delta_map, mapping->start);
                box_free(src);
            }
            if(ret) {
                printf_log(LOG_NONE, "Error while doing relocation on a DynaCache (block %d)\n", i);
                fclose(f);
                return DCERR_RELOC;
            }
            p+=(blocks[i].type==COMP_NONE)?blocks[i].block.size:ALIGN(blocks[i].compsize);
        }
        for(size_t i=0; i<header.nLockAddresses; ++i)
            addLockAddress(lockAddresses[i]+delta_map);
        for(size_t i=0; i<header.nUnalignedAddresses; ++i)
            add_unaligned_address(unalignedAddresses[i]+delta_map);
        dynarec_log(LOG_INFO, "Loaded DynaCache for %s, with %d blocks\n", mapping->fullname, header.nblocks);
    }
    fclose(f);
    return DCERR_OK;
}
#endif

void DynaCacheList(const char* filter)
{
    #ifndef WIN32
    const char* folder = GetDynacacheFolder(NULL);
    if(!folder) {
        printf_log(LOG_NONE, "DynaCache folder not found\n");
        return;
    }
    DIR* dir = opendir(folder);
    if(!dir) {
        printf_log(LOG_NONE, "Cannot open DynaCache folder\n");
        return;
    }
    struct dirent* d = NULL;
    int need_filter = (filter && strlen(filter));
    while((d = readdir(dir))) {
        size_t l = strlen(d->d_name);
        if(l>6 && !strcmp(d->d_name+l-6, ".box64")) {
            if(need_filter && !strstr(d->d_name, filter))
                continue;
            ReadDynaCache(folder, d->d_name, NULL, 1);
            printf_log_prefix(0, LOG_NONE, "\n");
        }
    }
    closedir(dir);
    #endif
}
void DynaCacheClean()
{
    #ifndef WIN32
    const char* folder = GetDynacacheFolder(NULL);
    if(!folder) {
        printf_log(LOG_NONE, "DynaCache folder not found\n");
        return;
    }
    DIR* dir = opendir(folder);
    if(!dir) {
        printf_log(LOG_NONE, "Cannot open DynaCache folder\n");
        return;
    }
    struct dirent* d = NULL;
    while((d = readdir(dir))) {
        size_t l = strlen(d->d_name);
        if(RemoveStaleDynaCacheTempFile(folder, d->d_name))
            continue;
        if(l>6 && !strcmp(d->d_name+l-6, ".box64")) {
            int ret = ReadDynaCache(folder, d->d_name, NULL, 0);
            if(ret) {
                char filename[strlen(folder)+strlen(d->d_name)+1];
                strcpy(filename, folder);
                strcat(filename, d->d_name);
                size_t filesize = FileSize(filename);
                if(!unlink(filename)) {
                    printf_log(LOG_NONE, "Removed %s for %s\n", d->d_name, NicePrintSize(filesize));
                } else {
                    printf_log(LOG_NONE, "Could not remove %s\n", d->d_name);
                }
            }
        }
    }
    closedir(dir);
    SyncDynaCacheFolder(folder);
    #endif
}
#ifndef WIN32
void MmapDynaCache(mapping_t* mapping)
{
    if(!DYNAREC_VERSION)
        return;
    // no need to test dynacache enabled or not, it has already been done before this call
    const char* folder = GetDynacacheFolder(mapping);
    if(!folder) return;
    const char* name = GetMmaplistName(mapping);
    if(!name) return;
    dynarec_log(LOG_DEBUG, "Looking for DynaCache %s in %s\n", name, folder);
    ReadDynaCache(folder, name, mapping, 0);
}
#endif
#else
void SerializeMmaplist(mapping_t* mapping) {}
void DynaCacheList(const char* filter) { printf_log(LOG_NONE, "Dynarec not enable\n"); }
void DynaCacheClean() {}
#endif