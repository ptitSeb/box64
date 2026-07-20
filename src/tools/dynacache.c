#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>
#if defined(DYNAREC) && !defined(WIN32)
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <utime.h>
#endif

#include "os.h"
#include "env.h"
#include "custommem.h"
#include "khash.h"
#include "debug.h"
#include "fileutils.h"
#include "dynacache.h"
#include "dynacache_compress.h"
#include "dynablock.h"
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

#define FILE_VERSION 6
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
    size_t      cache_header_size;
    size_t      cache_file_size;
    uint64_t    header_hash;
    uint64_t    payload_hash;
    uint32_t    filename_length;
    uint32_t    nblocks;
    uint32_t    nLockAddresses;
    uint32_t    nUnalignedAddresses;
    char        filename[];
} DynaCacheHeader_t;

// Helpers for conditional code emission based on dynacache bit width.
#define DC_0(...)
#define DC_1(...)                __VA_ARGS__
#define DC_2(...)                __VA_ARGS__
#define DC_3(...)                __VA_ARGS__
#define DC_CONCAT(A, B)          A##B
#define DC_IF_WIDTH(WIDTH, CODE) DC_CONCAT(DC_, WIDTH)(CODE)

// Generate dynarec_settings_t bit-fields from ENVSUPER().
#define INTEGER(NAME, name, default, min, max, wine, dynacache) DC_IF_WIDTH(dynacache, uint64_t name : dynacache;)
#define INTEGER64(NAME, name, default, wine, dynacache)         DC_IF_WIDTH(dynacache, uint64_t name : dynacache;)
#define BOOLEAN(NAME, name, default, wine, dynacache)           DC_IF_WIDTH(dynacache, uint64_t name : dynacache;)
#define ADDRESS(NAME, name, wine, dynacache)                    DC_IF_WIDTH(dynacache, uint64_t name : dynacache;)
#define STRING(NAME, name, wine, dynacache)                     DC_IF_WIDTH(dynacache, uint64_t name : dynacache;)
typedef union dynarec_settings_s {
    struct {
        ENVSUPER()
    };
    uint64_t    x;
} dynarec_settings_t;
#undef INTEGER
#undef INTEGER64
#undef BOOLEAN
#undef ADDRESS
#undef STRING

static int displayToInteger(const char* name, int value)
{
    if (!strcmp(name, "BOX64_DYNAREC_FORWARD")) {
        switch (value) {
            case 0: return 0;
            case 128: return 1;
            case 256: return 2;
            case 512: return 3;
            case 1024: return 4;
            default: return 1; // default 128
        }
    }
    return value;
}

static int integerToDisplay(const char* name, int stored)
{
    if (!strcmp(name, "BOX64_DYNAREC_FORWARD")) {
        switch (stored) {
            case 0: return 0;
            case 1: return 128;
            case 2: return 256;
            case 3: return 512;
            case 4: return 1024;
            default: return 128;
        }
    }
    return stored;
}

uint64_t GetDynSetting(mapping_t* mapping)
{
    dynarec_settings_t settings = {0};
#define INTEGER(NAME, name, default, min, max, wine, dynacache) DC_IF_WIDTH(dynacache, settings.name = displayToInteger(#NAME, (mapping && mapping->env && mapping->env->is_##name##_overridden) ? mapping->env->name : box64env.name);)
#define INTEGER64(NAME, name, default, wine, dynacache)         DC_IF_WIDTH(dynacache, settings.name = (mapping && mapping->env && mapping->env->is_##name##_overridden) ? mapping->env->name : box64env.name;)
#define BOOLEAN(NAME, name, default, wine, dynacache)           DC_IF_WIDTH(dynacache, settings.name = (mapping && mapping->env && mapping->env->is_##name##_overridden) ? mapping->env->name : box64env.name;)
#define ADDRESS(NAME, name, wine, dynacache)                    DC_IF_WIDTH(dynacache, settings.name = (mapping && mapping->env && mapping->env->is_##name##_overridden) ? mapping->env->name : box64env.name;)
#define STRING(NAME, name, wine, dynacache)                     DC_IF_WIDTH(dynacache, settings.name = (mapping && mapping->env && mapping->env->is_##name##_overridden) ? mapping->env->name : box64env.name;)
    ENVSUPER()
#undef INTEGER
#undef INTEGER64
#undef BOOLEAN
#undef ADDRESS
#undef STRING
    return settings.x;
}

void PrintDynSettings(int level, uint64_t s)
{
    dynarec_settings_t settings = {0};
    settings.x = s;
#define INTEGER(NAME, name, default, min, max, wine, dynacache) DC_IF_WIDTH(dynacache, if (settings.name != displayToInteger(#NAME, default)) printf_log_prefix(0, level, "\t\t" #NAME "=%d\n", integerToDisplay(#NAME, settings.name));)
#define INTEGER64(NAME, name, default, wine, dynacache)         DC_IF_WIDTH(dynacache, if (settings.name != default) printf_log_prefix(0, level, "\t\t" #NAME "=%lld\n", settings.name);)
#define BOOLEAN(NAME, name, default, wine, dynacache)           DC_IF_WIDTH(dynacache, if (settings.name != default) printf_log_prefix(0, level, "\t\t" #NAME "=%d\n", settings.name);)
#define ADDRESS(NAME, name, wine, dynacache)                    DC_IF_WIDTH(dynacache, if (settings.name != default) printf_log_prefix(0, level, "\t\t" #NAME "=%p\n", (void*)settings.name);)
#define STRING(NAME, name, wine, dynacache)                     DC_IF_WIDTH(dynacache, if (settings.name != default) printf_log_prefix(0, level, "\t\t" #NAME "=%s\n", (char*)settings.name);)
    ENVSUPER()
#undef INTEGER
#undef INTEGER64
#undef BOOLEAN
#undef ADDRESS
#undef STRING
}

#undef DC_0
#undef DC_1
#undef DC_2
#undef DC_3
#undef DC_CONCAT
#undef DC_IF_WIDTH

char* MmaplistName(const char* filename, uint64_t dynarec_settings, const char* fullname)
{
    // names are FOLDER/filename-YYYYY-XXXXX.box64
    // Where XXXXX is the hash of the full name
    // and YYYY is the Dynarec optim (in hex)
    static char mapname[4096];
    snprintf(mapname, 4095-6, "%s-%" PRIx64 "-%u", filename, dynarec_settings, __ac_X31_hash_string(fullname));
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
    const char* units[] = {"", "KiB", "MiB", "GiB"};
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
#define DYNACACHE_HASH_INIT UINT64_C(0)

static uint64_t DynaCacheHash(uint64_t hash, const void* data, size_t size)
{
    const uint8_t* p = data;
    while(size) {
        int len = (size>INT_MAX)?INT_MAX:(int)size;
        uint32_t crc = X31_hash_code((void*)p, len);
        hash ^= (uint64_t)crc + UINT64_C(0x9e3779b97f4a7c15) + (hash << 6) + (hash >> 2) + (uint64_t)len;
        p += len;
        size -= len;
    }
    return hash;
}

static size_t DynaCacheHeaderSize(const DynaCacheHeader_t* header)
{
    size_t total = sizeof(DynaCacheHeader_t);
    total += (size_t)header->filename_length + 1;
    total += header->nblocks*sizeof(CompressedDynaCacheBlock_t);
    total += header->nLockAddresses*sizeof(uintptr_t);
    total += header->nUnalignedAddresses*sizeof(uintptr_t);
    return ALIGN(total);
}

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
    #ifdef WIN32
    (void)mapping;
    return;
    #else
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
    DynaCacheHeader_t header_info = {0};
    header_info.filename_length = strlen(mapping->fullname);
    header_info.nblocks = nblocks;
    header_info.nLockAddresses = nLockAddresses;
    header_info.nUnalignedAddresses = nUnaligned;
    size_t total = DynaCacheHeaderSize(&header_info);
    uint8_t* all_header = box_calloc(1, total);
    if(!all_header) return;
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
    header->cache_header_size = total;
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
        box_free(all_header);
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
        box_free(all_header);
        return;
    }
    FILE* f = fdopen(tmpfd, "wb");
    if(!f) {
        close(tmpfd);
        unlink(tmpname);
        dynarec_log(LOG_INFO, "Cannot create cache file %s\n", mapname);
        box_free(all_header);
        return;
    }
    int write_error = 0;
    uint64_t payload_hash = DYNACACHE_HASH_INIT;
    if(fwrite(all_header, total, 1, f)!=1)
        write_error = 1;
    int type = BOX64ENV(dynacache_compress);
    if(type!=COMP_NONE) {
        for(int i=0; i<nblocks && !write_error; ++i) {
            size_t sz = 0;
            void* dest = dc_compress(blocks[i].block.block, blocks[i].block.size, type, &sz);
            if(dest) {
                payload_hash = DynaCacheHash(payload_hash, dest, sz);
                if(fwrite(dest, sz, 1, f)!=1)
                    write_error = 1;
                else {
                    blocks[i].type = type;
                    blocks[i].compsize = sz;
                    // align to pagesize...
                    if(sz&(box64_pagesize-1)) {
                        size_t align = box64_pagesize - (sz&(box64_pagesize-1));
                        char tmp[align];
                        memset(tmp, 0, align);
                        payload_hash = DynaCacheHash(payload_hash, tmp, align);
                        if(fwrite(tmp, align, 1, f)!=1)
                            write_error = 1;
                    }
                }
                box_free(dest);
            } else {
                payload_hash = DynaCacheHash(payload_hash, blocks[i].block.block, blocks[i].block.size);
                if(fwrite(blocks[i].block.block, blocks[i].block.size, 1, f)!=1)
                    write_error = 1;
            }
        }
    } else
        for(int i=0; i<nblocks && !write_error; ++i) {
            payload_hash = DynaCacheHash(payload_hash, blocks[i].block.block, blocks[i].block.size);
            if(fwrite(blocks[i].block.block, blocks[i].block.size, 1, f)!=1) {
                write_error = 1;
            }
        }
    if(!write_error) {
        off_t cache_file_size = ftello(f);
        if(cache_file_size<0) {
            write_error = 1;
        } else {
            header->cache_file_size = cache_file_size;
            header->payload_hash = payload_hash;
            header->header_hash = 0;
            header->header_hash = DynaCacheHash(DYNACACHE_HASH_INIT, all_header, total);
            if(fseeko(f, 0, SEEK_SET) || fwrite(all_header, total, 1, f)!=1)
                write_error = 1;
        }
    }
    if(!write_error && fflush(f))
        write_error = 1;
    if(!write_error && fsync(tmpfd))
        write_error = 1;
    if(fclose(f))
        write_error = 1;
    tmpfd = -1;
    if(write_error) {
        dynarec_log(LOG_INFO, "Error writing Cache file (disk full?)\n");
        unlink(tmpname);
        box_free(all_header);
        return;
    }
    int dynacache_limit = box64env.dynacache_limit;
    if(mapping->env && mapping->env->is_dynacache_limit_overridden)
        dynacache_limit = mapping->env->dynacache_limit;

    if(rename(tmpname, mapname)) {
        dynarec_log(LOG_INFO, "Cannot publish cache file %s\n", mapname);
        unlink(tmpname);
        box_free(all_header);
        return;
    }
    SyncDynaCacheFolder(folder);
    if(dynacache_limit>0) {
        PruneDynaCacheFolder(folder, (uint64_t)dynacache_limit*1024*1024);
        SyncDynaCacheFolder(folder);
    }
    box_free(all_header);
    #else
    box_free(all_header);
    // TODO?
    #endif
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
#define DCERR_CHECKSUM      14

#ifndef WIN32
int ReadDynaCache(const char* folder, const char* name, mapping_t* mapping, int verbose)
{
    int ret = DCERR_OK;
    uint8_t* all_header = NULL;
    char filename[strlen(folder)+strlen(name)+1];
    strcpy(filename, folder);
    strcat(filename, name);
    if (!FileExist(filename, IS_FILE)) return DCERR_NEXIST;
    if (verbose) printf_log(LOG_NONE, "File %s:\n\t", name);
    FILE *f = fopen(filename, "rb");
    if(!f) {
        if (verbose) printf_log_prefix(0, LOG_NONE, "Cannot open file\n");
        return DCERR_FERROR;
    }
    struct stat st = {0};
    int fd = fileno(f);
    if(fstat(fd, &st) || !S_ISREG(st.st_mode) || st.st_size<0) {
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
    if(memcmp(header.sign, HEADER_SIGN, sizeof(header.sign))) {
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
    if(!header.filename_length || !header.nblocks) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Invalid header\n");
        fclose(f);
        return DCERR_BADHEADER;
    }
    size_t header_size = DynaCacheHeaderSize(&header);
    if(header.cache_header_size!=header_size
       || header.cache_header_size>filesize
       || header.cache_file_size!=filesize) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Invalid cache size\n");
        fclose(f);
        return DCERR_BADHEADER;
    }
    all_header = box_malloc(header.cache_header_size);
    if(!all_header) {
        if (verbose) printf_log_prefix(0, LOG_NONE, "Cannot allocate memory for header\n");
        fclose(f);
        return DCERR_FERROR;
    }
    if(fseeko(f, 0, SEEK_SET) || fread(all_header, header.cache_header_size, 1, f)!=1) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Cannot read header\n");
        ret = DCERR_FERROR;
        goto done;
    }
    DynaCacheHeader_t* file_header = (DynaCacheHeader_t*)all_header;
    if(file_header->cache_header_size!=header.cache_header_size) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Invalid cache size\n");
        ret = DCERR_BADHEADER;
        goto done;
    }
    uint64_t expected_header_hash = file_header->header_hash;
    file_header->header_hash = 0;
    uint64_t actual_header_hash = DynaCacheHash(DYNACACHE_HASH_INIT, all_header, header.cache_header_size);
    file_header->header_hash = expected_header_hash;
    if(actual_header_hash!=expected_header_hash) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Bad header checksum\n");
        ret = DCERR_CHECKSUM;
        goto done;
    }
    char* map_filename = (char*)all_header + sizeof(DynaCacheHeader_t);
    if(map_filename[file_header->filename_length] || strlen(map_filename)!=file_header->filename_length) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Invalid filename\n");
        ret = DCERR_BADHEADER;
        goto done;
    }
    if(!FileExist(map_filename, IS_FILE)) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Mapfiled does not exists\n");
        ret = DCERR_MAPNEXIST;
        goto done;
    }
    if(FileSize(map_filename)!=file_header->file_length) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "File changed\n");
        ret = DCERR_MAPCHG;
        goto done;
    }
    if(mapping && strcmp(map_filename, mapping->fullname)) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Invalid cache name\n");
        ret = DCERR_BADNAME;
        goto done;
    }
    if(mapping && file_header->dynarec_settings!=GetDynSetting(mapping)) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Incompatible Dynarec Settings\n");
        ret = DCERR_BADNAME;
        goto done;
    }
    if (mapping && file_header->map_len != SizeFileMapped(mapping->start)) {
        if (verbose) printf_log_prefix(0, LOG_NONE, "Incompatible mapped size\n");
        ret = DCERR_MAPCHG;
        goto done;
    }

    CompressedDynaCacheBlock_t* blocks = (CompressedDynaCacheBlock_t*)(map_filename + file_header->filename_length + 1);
    uintptr_t* lockAddresses = (uintptr_t*)(blocks + file_header->nblocks);
    uintptr_t* unalignedAddresses = lockAddresses + file_header->nLockAddresses;

    off_t p = file_header->cache_header_size;
    uint64_t payload_hash = DYNACACHE_HASH_INIT;
    if(fseeko(f, p, SEEK_SET)) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Error reading a block\n");
        ret = DCERR_FERROR;
        goto done;
    }
    for(int i=0; i<file_header->nblocks; ++i) {
        size_t file_size = 0;
        size_t payload_size = 0;
        int valid_block = 0;
        if(blocks[i].block.block && blocks[i].block.size) {
            if(blocks[i].type==COMP_NONE) {
                valid_block = !blocks[i].compsize;
                payload_size = blocks[i].block.size;
                file_size = blocks[i].block.size;
            } else if((blocks[i].type==COMP_Z || blocks[i].type==COMP_Z_MAX)
                      && blocks[i].compsize) {
                valid_block = 1;
                payload_size = blocks[i].compsize;
                file_size = ALIGN(blocks[i].compsize);
            }
        }
        if(!valid_block || p<0 || (uintmax_t)p>filesize || file_size>filesize-(size_t)p) {
            if(verbose) printf_log_prefix(0, LOG_NONE, "Invalid block table\n");
            ret = DCERR_BADHEADER;
            goto done;
        }
        void* payload = box_malloc(payload_size);
        if(!payload || fread(payload, payload_size, 1, f)!=1) {
            box_free(payload);
            if(verbose) printf_log_prefix(0, LOG_NONE, "Error reading a block\n");
            ret = DCERR_FERROR;
            goto done;
        }
        payload_hash = DynaCacheHash(payload_hash, payload, payload_size);
        box_free(payload);
        if(file_size>payload_size) {
            size_t padding_size = file_size-payload_size;
            uint8_t padding[padding_size];
            if(fread(padding, padding_size, 1, f)!=1) {
                if(verbose) printf_log_prefix(0, LOG_NONE, "Error reading a block\n");
                ret = DCERR_FERROR;
                goto done;
            }
            payload_hash = DynaCacheHash(payload_hash, padding, padding_size);
        }
        p += file_size;
    }
    if(p<0 || (uintmax_t)p!=filesize) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Invalid cache size\n");
        ret = DCERR_BADHEADER;
        goto done;
    }
    if(payload_hash!=file_header->payload_hash) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Bad payload checksum\n");
        ret = DCERR_CHECKSUM;
        goto done;
    }
    if(!mapping) {
        char* short_name = strrchr(map_filename, '/');
        if(short_name)
            ++short_name;
        else
            short_name = map_filename;
        short_name = LowerCase(short_name);
        const char* file_name = MmaplistName(short_name, file_header->dynarec_settings, map_filename);
        box_free(short_name);
        if(strcmp(file_name, name)) {
            if(verbose) printf_log_prefix(0, LOG_NONE, "Invalid cache name\n");
            ret = DCERR_BADNAME;
            goto done;
        }
        if(verbose) {
            // check if name is coherent
            // file is valid, gives informations:
            printf_log_prefix(0, LOG_NONE, "%s (%s)\n", map_filename, NicePrintSize(filesize));
            printf_log_prefix(0, LOG_NONE, "\tDynarec Settings:\n");
            PrintDynSettings(LOG_NONE, file_header->dynarec_settings);
            size_t total_blocks = 0, total_free = 0, total_compressed = 0, total_uncompressed = 0;
            size_t total_code = file_header->codesize;
            for(int i=0; i<file_header->nblocks; ++i) {
                total_blocks += blocks[i].block.size;
                total_free += blocks[i].block.free_size;
                if(blocks[i].compsize)
                    total_compressed += blocks[i].compsize;
                else
                    total_uncompressed += blocks[i].block.size;
            }
            char buf[1024];
            int n = 0;
            n += snprintf(buf+n, sizeof(buf)-n, "\tHas %d blocks for a total of %s", file_header->nblocks, NicePrintSize(total_blocks));
            n += snprintf(buf+n, sizeof(buf)-n, ", with %s still free", NicePrintSize(total_free));
            n += snprintf(buf+n, sizeof(buf)-n, " and %s non-canceled blocks\n", NicePrintSize(total_code));
            n += snprintf(buf+n, sizeof(buf)-n, "\tMapped at %p-%p, with %zu lock and %zu unaligned addresses",
                (void*)file_header->map_addr, (void*)file_header->map_addr+file_header->map_len,
                file_header->nLockAddresses, file_header->nUnalignedAddresses);
            if(total_compressed && n>0 && n<(int)sizeof(buf)) {
                n += snprintf(buf+n, sizeof(buf)-n, "\n\tCompression: %d%% / %s compressed", 100ULL-total_compressed*100ULL/total_blocks ,NicePrintSize(total_compressed));
                if(total_uncompressed && n>0 && n<(int)sizeof(buf))
                    n += snprintf(buf+n, sizeof(buf)-n, ", %s uncompressed", NicePrintSize(total_uncompressed));
            }
            if(n>0 && n<(int)sizeof(buf))
                snprintf(buf+n, sizeof(buf)-n, "\n");
            printf_log_prefix(0, LOG_NONE, "%s", buf);
        }
    } else {
        // actually reading!
        intptr_t delta_map = mapping->start - file_header->map_addr;
        dynarec_log(LOG_INFO, "Trying to load DynaCache for %s, with a delta_map=%zx\n", mapping->fullname, delta_map);
        if(!mapping->mmaplist)
            mapping->mmaplist = NewMmaplist();
        MmaplistAddNBlocks(mapping->mmaplist, file_header->nblocks);
        p = file_header->cache_header_size;
        for(int i=0; i<file_header->nblocks; ++i) {
            int reloc_ret = 0;
            size_t file_size = (blocks[i].type==COMP_NONE)?blocks[i].block.size:ALIGN(blocks[i].compsize);
            if(blocks[i].type==COMP_NONE)
                reloc_ret = MmaplistAddBlock(mapping->mmaplist, fd, p, blocks[i].block.block, blocks[i].block.size, delta_map, mapping->start);
            else {
                void* src = box_malloc(blocks[i].compsize);
                if(!src || fseeko(f, p, SEEK_SET) || fread(src, blocks[i].compsize, 1, f)!=1) {
                    reloc_ret = -10;
                } else
                    reloc_ret = MmaplistAddCompressedBlock(mapping->mmaplist, blocks[i].type, src, blocks[i].compsize, blocks[i].block.block, blocks[i].block.size, delta_map, mapping->start);
                box_free(src);
            }
            if(reloc_ret) {
                printf_log(LOG_NONE, "Error while doing relocation on a DynaCache (block %d)\n", i);
                ret = DCERR_RELOC;
                goto done;
            }
            p += file_size;
        }
        for(size_t i=0; i<file_header->nLockAddresses; ++i)
            addLockAddress(lockAddresses[i]+delta_map);
        for(size_t i=0; i<file_header->nUnalignedAddresses; ++i)
            add_unaligned_address(unalignedAddresses[i]+delta_map);
        if (verbose) printf_log_prefix(0, LOG_NONE, "Cache loaded successfully\n");
        dynarec_log(LOG_INFO, "Loaded DynaCache for %s, with %d blocks\n", mapping->fullname, file_header->nblocks);
        // try to update mtime for used cache file, so that it is less likely to be pruned
        utime(filename, NULL);
    }
done:
    box_free(all_header);
    fclose(f);
    return ret;
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
