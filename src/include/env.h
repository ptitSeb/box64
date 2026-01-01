#ifndef __ENV_H
#define __ENV_H

#include <stdint.h>
#include <unistd.h>

#define BOX64ENV(name)            (box64env.name)
#define BOX64DRENV(name)          ((dyn->env && dyn->env->is_##name##_overridden)?dyn->env->name:box64env.name)
#define SET_BOX64ENV(name, value)            \
    do {                                     \
        box64env.name = (value);             \
        box64env.is_any_overridden = 1;      \
        box64env.is_##name##_overridden = 1; \
    } while (0)
#define SET_BOX64ENV_IF_EMPTY(name, value)      \
    do {                                        \
        if (!box64env.is_##name##_overridden) { \
            SET_BOX64ENV(name, value);          \
        }                                       \
    } while (0)

/*
    INTEGER(NAME, name, default, min, max, wine)
    INTEGER64(NAME, name, default, wine)
    BOOLEAN(NAME, name, default, wine)
    ADDRESS(NAME, name, wine)
    STRING(NAME, name, wine)
*/

#ifdef _WIN32
#define DEFAULT_LOG_LEVEL (LOG_INFO)
#define BOX64_NOBANNER_DEFAULT (0)
#else
extern char* ftrace_name;
#define DEFAULT_LOG_LEVEL (ftrace_name ? LOG_INFO : (isatty(fileno(stdout)) ? LOG_INFO : LOG_NONE))
#define BOX64_NOBANNER_DEFAULT (isatty(fileno(stdout)) ? 0 : 1)
#endif

#define ENVSUPER1()                                                           \
    STRING(BOX64_ADDLIBS, addlibs, 0)                                         \
    BOOLEAN(BOX64_AES, aes, 1, 1)                                             \
    BOOLEAN(BOX64_ALLOWMISSINGLIBS, allow_missing_libs, 0, 0)                 \
    STRING(BOX64_ARCH, arch, 0)                                               \
    STRING(BOX64_ARGS, args, 0)                                               \
    STRING(BOX64_BASH, bash, 0)                                               \
    STRING(BOX64_PYTHON3, python3, 0)                                         \
    INTEGER(BOX64_CPUTYPE, cputype, 0, 0, 1, 1)                               \
    BOOLEAN(BOX64_CRASHHANDLER, dummy_crashhandler, 1, 0)                     \
    BOOLEAN(BOX64_DLSYM_ERROR, dlsym_error, 0, 0)                             \
    BOOLEAN(BOX64_DUMP, dump, 0, 1)                                           \
    BOOLEAN(BOX64_DYNAREC_ALIGNED_ATOMICS, dynarec_aligned_atomics, 0, 1)     \
    INTEGER(BOX64_DYNAREC_BIGBLOCK, dynarec_bigblock, 2, 0, 3, 1)             \
    BOOLEAN(BOX64_DYNAREC_BLEEDING_EDGE, dynarec_bleeding_edge, 1, 0)         \
    INTEGER(BOX64_DYNAREC_CALLRET, dynarec_callret, 0, 0, 2, 1)               \
    BOOLEAN(BOX64_DYNAREC_DF, dynarec_df, 1, 1)                               \
    INTEGER(BOX64_DYNAREC_DIRTY, dynarec_dirty, 0, 0, 2, 0)                   \
    BOOLEAN(BOX64_DYNAREC_HOTPAGE_ALT, dynarec_hotpage_alt, 1, 0)             \
    BOOLEAN(BOX64_DYNAREC_NOHOTPAGE, dynarec_nohotpage, 0, 0)                 \
    BOOLEAN(BOX64_DYNAREC_DIV0, dynarec_div0, 0, 1)                           \
    INTEGER(BOX64_DYNAREC_DUMP, dynarec_dump, 0, 0, 2, 1)                     \
    STRING(BOX64_DYNAREC_DUMP_RANGE, dynarec_dump_range, 1)                   \
    BOOLEAN(BOX64_DYNAREC_FASTNAN, dynarec_fastnan, 1, 1)                     \
    INTEGER(BOX64_DYNAREC_FASTROUND, dynarec_fastround, 1, 0, 2, 1)           \
    INTEGER(BOX64_DYNAREC_FORWARD, dynarec_forward, 128, 0, 1024, 1)          \
    STRING(BOX64_DYNAREC_GDBJIT, dynarec_gdbjit_str, 0)                       \
    INTEGER(BOX64_DYNAREC_LOG, dynarec_log, 0, 0, 3, 1)                       \
    INTEGER(BOX64_DYNAREC_MISSING, dynarec_missing, 0, 0, 2, 1)               \
    BOOLEAN(BOX64_DYNAREC_NATIVEFLAGS, dynarec_nativeflags, 1, 1)             \
    INTEGER(BOX64_DYNAREC_PAUSE, dynarec_pause, 0, 0, 3, 1)                   \
    BOOLEAN(BOX64_DYNAREC_PERFMAP, dynarec_perf_map, 0, 0)                    \
    INTEGER(BOX64_DYNAREC_SAFEFLAGS, dynarec_safeflags, 1, 0, 2, 1)           \
    INTEGER(BOX64_DYNAREC_STRONGMEM, dynarec_strongmem, 0, 0, 3, 1)           \
    BOOLEAN(BOX64_DYNAREC_TBB, dynarec_tbb, 1, 0)                             \
    STRING(BOX64_DYNAREC_TEST, dynarec_test_str, 1)                           \
    BOOLEAN(BOX64_DYNAREC_TEST_NODUP, dynarec_test_nodup, 0, 1)               \
    BOOLEAN(BOX64_DYNAREC_TRACE, dynarec_trace, 0, 0)                         \
    BOOLEAN(BOX64_DYNAREC_VOLATILE_METADATA, dynarec_volatile_metadata, 1, 0) \
    BOOLEAN(BOX64_DYNAREC_WAIT, dynarec_wait, 1, 1)                           \
    INTEGER(BOX64_DYNAREC_WEAKBARRIER, dynarec_weakbarrier, 1, 0, 2, 1)       \
    INTEGER(BOX64_DYNAREC_X87DOUBLE, dynarec_x87double, 0, 0, 2, 1)           \
    BOOLEAN(BOX64_DYNAREC_INTERP_SIGNAL, dynarec_interp_signal, 0, 0)         \
    BOOLEAN(BOX64_DYNAREC_PURGE, dynarec_purge, 0, 0)                         \
    INTEGER(BOX64_DYNAREC_PURGE_AGE, dynarec_purge_age, 4096, 10, 65536, 0)   \
    BOOLEAN(BOX64_NODYNAREC_DELAY, nodynarec_delay, 0, 1)                     \
    STRING(BOX64_EMULATED_LIBS, emulated_libs, 0)                             \
    INTEGER(BOX64_DYNAREC_NOARCH, dynarec_noarch, 0, 0, 2, 1)                 \
    STRING(BOX64_ENV, env, 0)                                                 \
    STRING(BOX64_ENV1, env1, 0)                                               \
    STRING(BOX64_ENV2, env2, 0)                                               \
    STRING(BOX64_ENV3, env3, 0)                                               \
    STRING(BOX64_ENV4, env4, 0)                                               \
    STRING(BOX64_ENV5, env5, 0)                                               \
    BOOLEAN(BOX64_EXIT, exit, 0, 0)                                           \
    BOOLEAN(BOX64_FIX_64BIT_INODES, fix_64bit_inodes, 0, 0)                   \
    BOOLEAN(BOX64_IGNOREINT3, ignoreint3, 0, 0)                               \
    STRING(BOX64_INSERT_ARGS, insert_args, 0)                                 \
    BOOLEAN(BOX64_INPROCESSGPU, inprocessgpu, 0, 0)                           \
    INTEGER(BOX64_JITGDB, jitgdb, 0, 0, 3, 0)                                 \
    BOOLEAN(BOX64_JVM, jvm, 1, 0)                                             \
    STRING(BOX64_LD_LIBRARY_PATH, ld_library_path, 0)                         \
    BOOLEAN(BOX64_LIBCEF, libcef, 0, 0)                                       \
    STRING(BOX64_LIBGL, libgl, 0)                                             \
    ADDRESS(BOX64_LOAD_ADDR, load_addr, 0)                                    \
    INTEGER(BOX64_LOG, log, DEFAULT_LOG_LEVEL, 0, 3, 1)                       \
    INTEGER(BOX64_MALLOC_HACK, malloc_hack, 0, 0, 2, 0)                       \
    INTEGER(BOX64_MAXCPU, new_maxcpu, 0, 0, 100, 1)                           \
    BOOLEAN(BOX64_MMAP32, mmap32, 1, 0)                                       \
    BOOLEAN(BOX64_NOBANNER, nobanner, BOX64_NOBANNER_DEFAULT, 1)              \
    STRING(BOX64_NODYNAREC, nodynarec, 1)                                     \
    BOOLEAN(BOX64_NOGTK, nogtk, 0, 0)                                         \
    BOOLEAN(BOX64_NOPULSE, nopulse, 0, 0)                                     \
    BOOLEAN(BOX64_NORCFILES, noenvfiles, 0, 0)                                \
    BOOLEAN(BOX64_NOSANDBOX, nosandbox, 0, 0)                                 \
    BOOLEAN(BOX64_NOSIGSEGV, nosigsegv, 0, 0)                                 \
    BOOLEAN(BOX64_NOSIGILL, nosigill, 0, 0)                                   \
    BOOLEAN(BOX64_NOVULKAN, novulkan, 0, 0)                                   \
    STRING(BOX64_PATH, path, 0)                                               \
    BOOLEAN(BOX64_PCLMULQDQ, pclmulqdq, 1, 1)                                 \
    BOOLEAN(BOX64_PREFER_EMULATED, prefer_emulated, 0, 0)                     \
    BOOLEAN(BOX64_PREFER_WRAPPED, prefer_wrapped, 0, 0)                       \
    STRING(BOX64_PROFILE, profile, 1)                                         \
    STRING(BOX64_RCFILE, envfile, 0)                                          \
    BOOLEAN(BOX64_RDTSC_1GHZ, rdtsc_1ghz, 0, 0)                               \
    BOOLEAN(BOX64_RESERVE_HIGH, reserve_high, 0, 0)                           \
    INTEGER(BOX64_ROLLING_LOG, cycle_log, 0, 0, 2048, 0)                      \
    BOOLEAN(BOX64_SDL2_JGUID, sdl2_jguid, 0, 0)                               \
    BOOLEAN(BOX64_SHAEXT, shaext, 1, 1)                                       \
    BOOLEAN(BOX64_SHOWBT, showbt, 0, 0)                                       \
    BOOLEAN(BOX64_SHOWSEGV, showsegv, 0, 0)                                   \
    BOOLEAN(BOX64_SSE_FLUSHTO0, sse_flushto0, 0, 1)                           \
    BOOLEAN(BOX64_SSE42, sse42, 1, 1)                                         \
    BOOLEAN(BOX64_STEAM_VULKAN, steam_vulkan, 0, 0)                           \
    BOOLEAN(BOX64_SYNC_ROUNDING, sync_rounding, 0, 0)                         \
    BOOLEAN(BOX64_TRACE_COLOR, trace_regsdiff, 0, 0)                          \
    BOOLEAN(BOX64_TRACE_EMM, trace_emm, 0, 0)                                 \
    STRING(BOX64_TRACE_FILE, trace_file, 0)                                   \
    STRING(BOX64_TRACE_INIT, trace_init, 0)                                   \
    INTEGER64(BOX64_TRACE_START, start_cnt, 0, 0)                             \
    BOOLEAN(BOX64_TRACE_XMM, trace_xmm, 0, 0)                                 \
    STRING(BOX64_TRACE, trace, 0)                                             \
    BOOLEAN(BOX64_UNITY, unity, 0, 0)                                         \
    BOOLEAN(BOX64_UNITYPLAYER, unityplayer, 1, 0)                             \
    BOOLEAN(BOX64_WRAP_EGL, wrap_egl, 0, 0)                                   \
    BOOLEAN(BOX64_X11GLX, x11glx, 1, 0)                                       \
    BOOLEAN(BOX64_X11SYNC, x11sync, 0, 0)                                     \
    BOOLEAN(BOX64_X11THREADS, x11threads, 0, 0)                               \
    BOOLEAN(BOX64_X87_NO80BITS, x87_no80bits, 0, 1)                           \
    BOOLEAN(BOX64_NOPERSONA32BITS, nopersona32bits, 0, 0)                     \
    INTEGER(BOX64_DYNACACHE, dynacache, 2, 0, 2, 0)                           \
    STRING(BOX64_DYNACACHE_FOLDER, dynacache_folder, 0)                       \
    INTEGER(BOX64_DYNACACHE_MIN, dynacache_min, 350, 0, 10240, 0)

#if defined(ARM64)
#define ENVSUPER2() \
    INTEGER(BOX64_AVX, avx, 2, 0, 2, 1)
#else
#define ENVSUPER2() \
    INTEGER(BOX64_AVX, avx, 0, 0, 2, 1)
#endif

#ifdef DYNAREC
#define ENVSUPER3() \
    BOOLEAN(BOX64_DYNAREC, dynarec, 1, 1)
#else
#define ENVSUPER3() \
    BOOLEAN(BOX64_DYNAREC, dynarec, 0, 1)
#endif

#define ENVSUPER() \
    ENVSUPER1()    \
    ENVSUPER2()    \
    ENVSUPER3()

typedef struct box64env_s {
#define INTEGER(NAME, name, default, min, max, wine) int name;
#define INTEGER64(NAME, name, default, wine)         int64_t name;
#define BOOLEAN(NAME, name, default, wine)           int name;
#define ADDRESS(NAME, name, wine)                    uintptr_t name;
#define STRING(NAME, name, wine)                     char* name;
    ENVSUPER()
#undef INTEGER
#undef INTEGER64
#undef BOOLEAN
#undef ADDRESS
#undef STRING

#define INTEGER(NAME, name, default, min, max, wine) uint64_t is_##name##_overridden : 1;
#define INTEGER64(NAME, name, default, wine)         uint64_t is_##name##_overridden : 1;
#define BOOLEAN(NAME, name, default, wine)           uint64_t is_##name##_overridden : 1;
#define ADDRESS(NAME, name, wine)                    uint64_t is_##name##_overridden : 1;
#define STRING(NAME, name, wine)                     uint64_t is_##name##_overridden : 1;
    ENVSUPER()
#undef INTEGER
#undef INTEGER64
#undef BOOLEAN
#undef ADDRESS
#undef STRING

    /******** Custom ones ********/
    int maxcpu;
    int dynarec_test;
    int avx2;
    int rolling_log;
    int dynarec_perf_map_fd;
    int dynarec_gdbjit;
    uintptr_t dynarec_test_start;
    uintptr_t dynarec_test_end;
    uintptr_t nodynarec_start;
    uintptr_t nodynarec_end;
    uintptr_t dynarec_gdbjit_start;
    uintptr_t dynarec_gdbjit_end;
    uintptr_t dynarec_dump_range_start;
    uintptr_t dynarec_dump_range_end;

    uint64_t is_any_overridden : 1;
    uint64_t is_dynarec_perf_map_fd_overridden : 1;
} box64env_t;

typedef struct mmaplist_s mmaplist_t;
#ifdef DYNAREC
typedef struct blocklist_s blocklist_t;

typedef struct DynaCacheBlock_s {
    blocklist_t*    block;
    size_t          size;
    size_t          free_size;
} DynaCacheBlock_t;
#endif

void InitializeEnvFiles();
void ApplyEnvFileEntry(const char* name);
const char* GetLastApplyEntryName();
void InitializeEnv();
void LoadEnvVariables();
void PrintEnvVariables(box64env_t* env, int level);
void RecordEnvMappings(uintptr_t addr, size_t length, int fd);
void WillRemoveMapping(uintptr_t addr, size_t length);
void RemoveMapping(uintptr_t addr, size_t length);
box64env_t* GetCurEnvByAddr(uintptr_t addr);
int IsAddrFileMapped(uintptr_t addr, const char** filename, uintptr_t* start);
size_t SizeFileMapped(uintptr_t addr);
mmaplist_t* GetMmaplistByAddr(uintptr_t addr);
int IsAddrNeedReloc(uintptr_t addr);
void SerializeAllMapping();
void DynaCacheList(const char* name);
void DynaCacheClean();
int IsAddrMappingLoadAndClean(uintptr_t addr);

#endif // __ENV_H
