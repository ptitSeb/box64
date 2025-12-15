#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <fcntl.h>
#include <string.h>
#include <inttypes.h>
#include <strings.h>
#if defined(DYNAREC) && !defined(WIN32)
#include <sys/types.h>
#include <dirent.h>
#endif

#include "os.h"
#include "env.h"
#include "custommem.h"
#include "khash.h"
#include "debug.h"
#include "fileutils.h"
#include "box64context.h"
#include "rbtree.h"
#include "wine_tools.h"
#include "pe_tools.h"

box64env_t box64env = { 0 };

KHASH_MAP_INIT_STR(box64env_entry, box64env_t)
static kh_box64env_entry_t* box64env_entries = NULL;
static kh_box64env_entry_t* box64env_entries_gen = NULL;

mmaplist_t* NewMmaplist();
void DelMmaplist(mmaplist_t* list);
#ifdef DYNAREC
int MmaplistHasNew(mmaplist_t* list, int clear);
int MmaplistIsDirty(mmaplist_t* list);
int MmaplistNBlocks(mmaplist_t* list);
size_t MmaplistTotalAlloc(mmaplist_t* list);
void MmaplistFillBlocks(mmaplist_t* list, DynaCacheBlock_t* blocks);
void MmaplistAddNBlocks(mmaplist_t* list, int nblocks);
int MmaplistAddBlock(mmaplist_t* list, int fd, off_t offset, void* orig, size_t size, intptr_t delta_map, uintptr_t mapping_start);
int nLockAddressRange(uintptr_t start, size_t size);
void getLockAddressRange(uintptr_t start, size_t size, uintptr_t addrs[]);
void addLockAddress(uintptr_t addr);
int nUnalignedRange(uintptr_t start, size_t size);
void getUnalignedRange(uintptr_t start, size_t size, uintptr_t addrs[]);
void add_unaligned_address(uintptr_t addr);
#endif

static rbtree_t* envmap = NULL;

static const char default_rcfile[] = 
"[bash]\n"
"BOX64_LOG=0\n"
"\n"
"[deadcells]\n"
"BOX64_PREFER_EMULATED=1\n"
"\n"
"[dontstarve]\n"
"BOX64_EMULATED_LIBS=libSDL2-2.0.so.0\n"
"\n"
"[dota2]\n"
"BOX64_CRASHHANDLER=1\n"
"BOX64_DYNAREC_STRONGMEM=1\n"
"\n"
"[factorio]\n"
"BOX64_DYNAREC_FASTROUND=0\n"
"\n"
"[heroic]\n"
"BOX64_NOSANDBOX=1\n"
"BOX64_MALLOC_HACK=2\n"
"\n"
"[LotCG.x86_64]\n"
"BOX64_DYNAREC_FASTROUND=0\n"
"\n"
"[Mini Metro]\n"
"BOX64_ADDLIBS=stdc++.so.6\n"
"\n"
"[pressure-vessel-wrap]\n"
"BOX64_NOGTK=1\n"
"\n"
"[ShovelKnight]\n"
"BOX64_SDL2_JGUID=1\n"
"\n"
"[Soma.bin.x86_64]\n"
"BOX64_DYNAREC_FASTROUND=0\n"
"\n"
"[streaming_client]\n"
"BOX64_EMULATED_LIBS=libSDL2-2.0.so.0:libSDL2_ttf-2.0.so.0\n"
"\n"
"[steam-runtime-launcher-service]\n"
"BOX64_NOGTK=1\n"
;

#ifdef _WIN32
#define PATHSEP "\\"
#define HOME    "USERPROFILE"
#else
#define PATHSEP "/"
#define HOME    "HOME"
#endif

char* LowerCase(const char* s)
{
    if (!s)
        return NULL;
    char* ret = box_calloc(1, strlen(s) + 1);
    size_t i = 0;
    while (*s) {
        ret[i++] = (*s >= 'A' && *s <= 'Z') ? (*s - 'A' + 'a') : (*s);
        ++s;
    }

    return ret;
}

static void addNewEnvVar(const char* s)
{
#ifndef _WIN32
    if (!s) return;
    char* p = box_strdup(s);
    char* e = strchr(p, '=');
    if (!e) {
        printf_log(LOG_INFO, "Invalid specific env. var. '%s'\n", s);
        box_free(p);
        return;
    }
    *e = '\0';
    ++e;
    setenv(p, e, 1);
    box_free(p);
#endif
}

static void parseRange(const char* s, uintptr_t* start, uintptr_t* end)
{
    if (!s) return;
    if (!strchr(s, '-')) return;
    if (sscanf(s, "%" PRId64 "-%" PRId64, start, end) == 2) return;
    if (sscanf(s, "0x%" PRIX64 "-0x%" PRIX64, start, end) == 2) return;
    if (sscanf(s, "0x%" PRIx64 "-0x%" PRIx64, start, end) == 2) return;
    sscanf(s, "%" PRIx64 "-%" PRIx64, start, end);
}

void AddNewLibs(const char* list);
int canNCpuBeChanged();

static void applyCustomRules()
{
    if (BOX64ENV(log) == LOG_NEVER) {
        SET_BOX64ENV(log, BOX64ENV(log) - 1);
        SET_BOX64ENV(dump, 1);
    }

#ifndef _WIN32
    if(box64env.is_cycle_log_overridden) {
        freeCycleLog(my_context);
        box64env.rolling_log = BOX64ENV(cycle_log);

        if (BOX64ENV(rolling_log) == 1) {
            box64env.rolling_log = 16;
        }
        if (BOX64ENV(rolling_log) && BOX64ENV(log) > LOG_INFO) {
            box64env.rolling_log = 0;
        }
        initCycleLog(my_context);
    }

    if (box64env.is_dynarec_gdbjit_str_overridden) {
        if (strlen(box64env.dynarec_gdbjit_str) == 1) {
            if (box64env.dynarec_gdbjit_str[0] >= '0' && box64env.dynarec_gdbjit_str[0] <= '3')
                box64env.dynarec_gdbjit = box64env.dynarec_gdbjit_str[0] - '0';

            box64env.dynarec_gdbjit_start = 0x0;
            box64env.dynarec_gdbjit_end = 0x0;
        } else if (strchr(box64env.dynarec_gdbjit_str, '-')) {
            parseRange(box64env.dynarec_gdbjit_str, &box64env.dynarec_gdbjit_start, &box64env.dynarec_gdbjit_end);
            if (box64env.dynarec_gdbjit_end > box64env.dynarec_gdbjit_start) {
                box64env.dynarec_gdbjit = 2;
            } else {
                box64env.dynarec_gdbjit = 0;
            }
        }
    }
#endif

    if (box64env.is_dynarec_test_str_overridden) {
        if (strlen(box64env.dynarec_test_str) == 1) {
            if (box64env.dynarec_test_str[0] >= '0' && box64env.dynarec_test_str[0] <= '1')
                box64env.dynarec_test = box64env.dynarec_test_str[0] - '0';

            box64env.dynarec_test_start = 0x0;
            box64env.dynarec_test_end = 0x0;
        } else if (strchr(box64env.dynarec_test_str, '-')) {
            parseRange(box64env.dynarec_test_str, &box64env.dynarec_test_start, &box64env.dynarec_test_end);
            if (box64env.dynarec_test_end > box64env.dynarec_test_start) {
                box64env.dynarec_test = 1;
            } else {
                box64env.dynarec_test = 0;
            }
        }
    }

    if (box64env.is_nodynarec_overridden)
        parseRange(box64env.nodynarec, &box64env.nodynarec_start, &box64env.nodynarec_end);

    if (box64env.is_dynarec_dump_range_overridden)
        parseRange(box64env.dynarec_dump_range, &box64env.dynarec_dump_range_start, &box64env.dynarec_dump_range_end);

    if (box64env.dynarec_test) {
        SET_BOX64ENV(dynarec_fastnan, 0);
        SET_BOX64ENV(dynarec_fastround, 0);
        if (BOX64ENV(dynarec_x87double) == 0)
            SET_BOX64ENV(dynarec_x87double, 1);
        SET_BOX64ENV(dynarec_div0, 1);
        SET_BOX64ENV(dynarec_callret, 0);
#if defined(RV64) || defined(LA64)
        SET_BOX64ENV(dynarec_nativeflags, 0);
#endif
    }

    if (box64env.is_profile_overridden) {
        if (!strcasecmp(box64env.profile, "safest")) {
            SET_BOX64ENV_IF_EMPTY(dynarec_fastnan, 0);
            SET_BOX64ENV_IF_EMPTY(dynarec_fastround, 0);
            SET_BOX64ENV_IF_EMPTY(dynarec_bigblock, 0);
            SET_BOX64ENV_IF_EMPTY(dynarec_safeflags, 2);
            SET_BOX64ENV_IF_EMPTY(dynarec_strongmem, 2);
        } else if (!strcasecmp(box64env.profile, "safe")) {
            SET_BOX64ENV_IF_EMPTY(dynarec_bigblock, 0);
            SET_BOX64ENV_IF_EMPTY(dynarec_safeflags, 2);
            SET_BOX64ENV_IF_EMPTY(dynarec_strongmem, 1);
        } else if (!strcasecmp(box64env.profile, "fast")) {
            SET_BOX64ENV_IF_EMPTY(dynarec_callret, 1);
            SET_BOX64ENV_IF_EMPTY(dynarec_bigblock, 3);
            SET_BOX64ENV_IF_EMPTY(dynarec_safeflags, 0);
            SET_BOX64ENV_IF_EMPTY(dynarec_strongmem, 1);
            SET_BOX64ENV_IF_EMPTY(dynarec_dirty, 1);
            SET_BOX64ENV_IF_EMPTY(dynarec_forward, 1024);
        } else if (!strcasecmp(box64env.profile, "fastest")) {
            SET_BOX64ENV_IF_EMPTY(dynarec_callret, 1);
            SET_BOX64ENV_IF_EMPTY(dynarec_bigblock, 3);
            SET_BOX64ENV_IF_EMPTY(dynarec_safeflags, 0);
            SET_BOX64ENV_IF_EMPTY(dynarec_strongmem, 0);
            SET_BOX64ENV_IF_EMPTY(dynarec_dirty, 1);
            SET_BOX64ENV_IF_EMPTY(dynarec_forward, 1024);
        }
    }

    if (box64env.maxcpu == 0 || (box64env.new_maxcpu < box64env.maxcpu)) {
        if(canNCpuBeChanged())
            box64env.maxcpu = box64env.new_maxcpu;
    }

#ifndef _WIN32
    if (box64env.dynarec_perf_map) {
        char pathname[32];
        snprintf(pathname, sizeof(pathname), "/tmp/perf-%d.map", getpid());
        SET_BOX64ENV(dynarec_perf_map_fd, open(pathname, O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR));
    }
    if (box64env.emulated_libs && my_context) {
        AppendList(&my_context->box64_emulated_libs, box64env.emulated_libs, 0);
    }
    if (!box64env.libgl) {
        const char *p = getenv("SDL_VIDEO_GL_DRIVER");
        if(p) SET_BOX64ENV(libgl, box_strdup(p));
    }
#endif

    if (box64env.avx == 2) {
        box64env.avx = 1;
        box64env.avx2 = 1;
    }

#ifndef _WIN32
    if (box64env.exit) exit(0);
#endif

    if (box64env.env) addNewEnvVar(box64env.env);
    if (box64env.env1) addNewEnvVar(box64env.env1);
    if (box64env.env2) addNewEnvVar(box64env.env2);
    if (box64env.env3) addNewEnvVar(box64env.env3);
    if (box64env.env4) addNewEnvVar(box64env.env4);
    if (box64env.env5) addNewEnvVar(box64env.env5);

#ifndef _WIN32
    if (box64env.addlibs) AddNewLibs(box64env.addlibs);
#endif
}

static void trimStringInplace(char* s)
{
    if (!s) return;
    // trim right space/tab
    size_t len = strlen(s);
    while (len && (s[len - 1] == ' ' || s[len - 1] == '\t' || s[len - 1] == '\n'))
        s[--len] = '\0';
    // trim left space/tab
    while (s[0] == ' ' || s[0] == '\t')
        memmove(s, s + 1, strlen(s));
}

static void freeEnv(box64env_t* env)
{
#define INTEGER(NAME, name, default, min, max, wine)
#define INTEGER64(NAME, name, default, wine)
#define BOOLEAN(NAME, name, default, wine)
#define ADDRESS(NAME, name, wine)
#define STRING(NAME, name, wine) box_free(env->name);
    ENVSUPER()
#undef INTEGER
#undef INTEGER64
#undef BOOLEAN
#undef ADDRESS
#undef STRING
}

#ifdef ARM64
#define ENV_ARCH "arm64"
#elif defined(RV64)
#define ENV_ARCH "rv64"
#elif defined(LA64)
#define ENV_ARCH "la64"
#else
#warning "Unknown architecture for ENV_ARCH"
#define ENV_ARCH "unknown"
#endif

static void pushNewEntry(const char* name, box64env_t* env, int gen)
{
    if (env->is_arch_overridden && *env->arch == '\0') env->is_arch_overridden = 0;

    // Arch specific but not match, ignore
    if (env->is_arch_overridden && strcasecmp(env->arch, ENV_ARCH)) {
        freeEnv(env);
        return;
    }
    khint_t k;
    kh_box64env_entry_t* khp = gen ? box64env_entries_gen : box64env_entries;
    k = kh_get(box64env_entry, khp, name);
    // No entry exist, add a new one
    if (k == kh_end(khp)) {
        int ret;
        k = kh_put(box64env_entry, khp, box_strdup(name), &ret);
        box64env_t* p = &kh_value(khp, k);
        memcpy(p, env, sizeof(box64env_t));
        return;
    }

    // Entry exists, replace it if the new one is arch specific
    if (env->is_arch_overridden && !strcasecmp(env->arch, ENV_ARCH)) {
        freeEnv(&kh_value(khp, k));
        box64env_t* p = &kh_value(khp, k);
        memcpy(p, env, sizeof(box64env_t));
        return;
    }

    // Entry exists, the new one is generic, replace it only if existing one is also generic
    box64env_t* p = &kh_value(khp, k);
    if (!p->is_arch_overridden) {
        freeEnv(p);
        box64env_t* p = &kh_value(khp, k);
        memcpy(p, env, sizeof(box64env_t));
    }
}

#ifdef ANDROID
static int shm_open(const char *name, int oflag, mode_t mode) {
    return -1;
}
static int shm_unlink(const char *name) {
    return -1;
}
#endif

static void initializeEnvFile(const char* filename)
{
    if (box64env.noenvfiles) return;

    BOXFILE* f = NULL;
    if (filename)
        f = box_fopen(filename, "r");
#ifndef _WIN32
    else {
#define TMP_MEMRCFILE "/box64_rcfile"
        int tmp = shm_open(TMP_MEMRCFILE, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return; // error, bye bye
        shm_unlink(TMP_MEMRCFILE);    // remove the shm file, but it will still exist because it's currently in use
        int dummy = write(tmp, default_rcfile, sizeof(default_rcfile));
        (void)dummy;
        lseek(tmp, 0, SEEK_SET);
        f = fdopen(tmp, "r");
    }
#endif

    if (!f) return;

    if (!box64env_entries)
        box64env_entries = kh_init(box64env_entry);
    if (!box64env_entries_gen)
        box64env_entries_gen = kh_init(box64env_entry);

    box64env_t current_env = { 0 };
    size_t linesize = 0, len = 0;
    char* current_name = NULL;
    bool is_wildcard_name = false;
    char line[1024];
    while ((box_fgets(line, 1024, f)) != NULL) {
        // remove comments
        char* p = strchr(line, '#');
        if (p) *p = '\0';
        trimStringInplace(line);
        len = strlen(line);
        if (line[0] == '[' && strchr(line, ']')) {
            // new entry, push the previous one
            if (current_name)
                pushNewEntry(current_name, &current_env, is_wildcard_name);
            is_wildcard_name = (line[1] == '*' && line[(intptr_t)(strchr(line, ']') - line) - 1] == '*');
            memset(&current_env, 0, sizeof(current_env));
            box_free(current_name);
            current_name = LowerCase(line + (is_wildcard_name ? 2 : 1));
            *(strchr(current_name, ']') + 1 - (is_wildcard_name ? 2 : 1)) = '\0';
            trimStringInplace(current_name);
        } else if (strchr(line, '=')) {
            char* key = line;
            char* val = strchr(key, '=') + 1;
            *strchr(key, '=') = '\0';
            trimStringInplace(key);
            trimStringInplace(val);
#ifdef _WIN32
#define VALID(a) a
#else
#define VALID(a) 1
#endif

#define INTEGER(NAME, name, default, min, max, wine) \
    else if (!strcmp(key, #NAME) && VALID(wine))     \
    {                                                \
        int v = strtol(val, &p, 0);                  \
        if (p != val && v >= min && v <= max) {      \
            current_env.is_##name##_overridden = 1;  \
            current_env.is_any_overridden = 1;       \
            current_env.name = v;                    \
        }                                            \
    }
#define INTEGER64(NAME, name, default, wine)        \
    else if (!strcmp(key, #NAME) && VALID(wine))    \
    {                                               \
        int64_t v = strtoll(val, &p, 0);            \
        if (p != val) {                             \
            current_env.is_##name##_overridden = 1; \
            current_env.is_any_overridden = 1;      \
            current_env.name = v;                   \
        }                                           \
    }
#define BOOLEAN(NAME, name, default, wine)          \
    else if (!strcmp(key, #NAME) && VALID(wine))    \
    {                                               \
        if (strcmp(val, "0")) {                     \
            current_env.is_##name##_overridden = 1; \
            current_env.is_any_overridden = 1;      \
            current_env.name = 1;                   \
        } else {                                    \
            current_env.is_##name##_overridden = 1; \
            current_env.is_any_overridden = 1;      \
            current_env.name = 0;                   \
        }                                           \
    }
#define ADDRESS(NAME, name, wine)                     \
    else if (!strcmp(key, #NAME) && VALID(wine))      \
    {                                                 \
        uintptr_t v = (uintptr_t)strtoll(val, &p, 0); \
        if (p != val) {                               \
            current_env.is_##name##_overridden = 1;   \
            current_env.is_any_overridden = 1;        \
            current_env.name = v;                     \
        }                                             \
    }
#define STRING(NAME, name, wine)                          \
    else if (!strcmp(key, #NAME) && VALID(wine))          \
    {                                                     \
        current_env.is_##name##_overridden = 1;           \
        current_env.is_any_overridden = 1;                \
        if (current_env.name) box_free(current_env.name); \
        current_env.name = strdup(val);                   \
    }
            if (0)
                ;
            ENVSUPER()
            else if (len && current_name)
            {
                printf_log(LOG_INFO, "BOX64ENV: Warning, unsupported %s=%s for [%s] in %s\n", key, val, current_name, filename);
            }
#undef INTEGER
#undef INTEGER64
#undef BOOLEAN
#undef ADDRESS
#undef STRING
#undef VALID
        }
    }
    // push the last entry
    if (current_name) {
        pushNewEntry(current_name, &current_env, is_wildcard_name);
        box_free(current_name);
    }
    box_fclose(f);
}


void InitializeEnvFiles()
{
#ifndef _WIN32 // FIXME: this needs some consideration on Windows, so for now, only do it on Linux
    if (BOX64ENV(envfile) && FileExist(BOX64ENV(envfile), IS_FILE))
        initializeEnvFile(BOX64ENV(envfile));
#ifndef TERMUX
    else if (FileExist("/etc/box64.box64rc", IS_FILE))
        initializeEnvFile("/etc/box64.box64rc");
    else if (FileExist("/data/data/com.termux/files/usr/glibc/etc/box64.box64rc", IS_FILE))
        initializeEnvFile("/data/data/com.termux/files/usr/glibc/etc/box64.box64rc");
#else
    else if (FileExist("/data/data/com.termux/files/usr/etc/box64.box64rc", IS_FILE))
        initializeEnvFile("/data/data/com.termux/files/usr/etc/box64.box64rc");
#endif
    else
        initializeEnvFile(NULL); // load default rcfile
#endif

    char* p = GetEnv(HOME);
    if (p) {
        static char tmp[4096];
        strncpy(tmp, p, 4095);
        strncat(tmp, PATHSEP ".box64rc", 4095);
        if (FileExist(tmp, IS_FILE)) {
            initializeEnvFile(tmp);
        }
    }
}

static void internalApplyEnvFileEntry(const char* entryname, const box64env_t* env)
{
#define INTEGER(NAME, name, default, min, max, wine) \
    if (env->is_##name##_overridden) {               \
        box64env.name = env->name;                   \
        box64env.is_##name##_overridden = 1;         \
        box64env.is_any_overridden = 1;              \
    }
#define INTEGER64(NAME, name, default, wine) \
    if (env->is_##name##_overridden) {       \
        box64env.name = env->name;           \
        box64env.is_##name##_overridden = 1; \
        box64env.is_any_overridden = 1;      \
    }
#define BOOLEAN(NAME, name, default, wine)   \
    if (env->is_##name##_overridden) {       \
        box64env.name = env->name;           \
        box64env.is_##name##_overridden = 1; \
        box64env.is_any_overridden = 1;      \
    }
#define ADDRESS(NAME, name, wine)            \
    if (env->is_##name##_overridden) {       \
        box64env.name = env->name;           \
        box64env.is_##name##_overridden = 1; \
        box64env.is_any_overridden = 1;      \
    }
#define STRING(NAME, name, wine)             \
    if (env->is_##name##_overridden) {       \
        box64env.name = env->name;           \
        box64env.is_##name##_overridden = 1; \
        box64env.is_any_overridden = 1;      \
    }
    ENVSUPER()
#undef INTEGER
#undef INTEGER64
#undef BOOLEAN
#undef ADDRESS
#undef STRING
}

static char old_entryname[256] = "";
void ApplyEnvFileEntry(const char* entryname)
{
    if (!entryname || !box64env_entries) return;
    if (!strcasecmp(entryname, old_entryname)) return;

    strncpy(old_entryname, entryname, 255);
    khint_t k1;
    {
        char* lowercase_entryname = LowerCase(entryname);
        k1 = kh_get(box64env_entry, box64env_entries, lowercase_entryname);
        box64env_t* env;
        const char* k2;
        kh_foreach_ref(box64env_entries_gen, k2, env,
            if (strstr(lowercase_entryname, k2))
                internalApplyEnvFileEntry(entryname, env);
            applyCustomRules();
        )
        box_free(lowercase_entryname);
    }
    if (k1 == kh_end(box64env_entries)) return;

    box64env_t* env = &kh_value(box64env_entries, k1);
    internalApplyEnvFileEntry(entryname, env);
    applyCustomRules();
}

void LoadEnvVariables()
{
#ifdef _WIN32
#define INTEGER(NAME, name, default, min, max, wine) box64env.name = wine ? default : 0;
#define INTEGER64(NAME, name, default, wine)         box64env.name = wine ? default : 0;
#define BOOLEAN(NAME, name, default, wine)           box64env.name = wine ? default : 0;
#define ADDRESS(NAME, name, wine)                    box64env.name = 0;
#define STRING(NAME, name, wine)                     box64env.name = NULL;
    ENVSUPER()
#undef INTEGER
#undef INTEGER64
#undef BOOLEAN
#undef ADDRESS
#undef STRING
#else
#define INTEGER(NAME, name, default, min, max, wine) box64env.name = default;
#define INTEGER64(NAME, name, default, wine)         box64env.name = default;
#define BOOLEAN(NAME, name, default, wine)           box64env.name = default;
#define ADDRESS(NAME, name, wine)                    box64env.name = 0;
#define STRING(NAME, name, wine)                     box64env.name = NULL;
    ENVSUPER()
#undef INTEGER
#undef INTEGER64
#undef BOOLEAN
#undef ADDRESS
#undef STRING
#endif
    char* p;
    // load env vars from GetEnv()

#ifdef _WIN32
#define GETENV(name, wine) (wine ? GetEnv(name) : NULL);
#else
#define GETENV(name, wine) GetEnv(name)
#endif

#define INTEGER(NAME, name, default, min, max, wine)      \
    p = GETENV(#NAME, wine);                              \
    if (p) {                                              \
        box64env.name = atoi(p);                          \
        if (box64env.name < min || box64env.name > max) { \
            box64env.name = default;                      \
        } else {                                          \
            box64env.is_##name##_overridden = 1;          \
            box64env.is_any_overridden = 1;               \
        }                                                 \
    }
#define INTEGER64(NAME, name, default, wine) \
    p = GETENV(#NAME, wine);                 \
    if (p) {                                 \
        box64env.name = atoll(p);            \
        box64env.is_##name##_overridden = 1; \
        box64env.is_any_overridden = 1;      \
    }
#define BOOLEAN(NAME, name, default, wine)   \
    p = GETENV(#NAME, wine);                 \
    if (p) {                                 \
        box64env.name = p[0] != '0';         \
        box64env.is_##name##_overridden = 1; \
        box64env.is_any_overridden = 1;      \
    }
#define ADDRESS(NAME, name, wine)                          \
    p = GETENV(#NAME, wine);                               \
    if (p) {                                               \
        char* endptr;                                      \
        box64env.name = (uintptr_t)strtoll(p, &endptr, 0); \
        box64env.is_##name##_overridden = 1;               \
        box64env.is_any_overridden = 1;                    \
    }
#define STRING(NAME, name, wine)                \
    p = GETENV(#NAME, wine);                    \
    if (p && strcasecmp(#NAME, "BOX64_ARCH")) { \
        box64env.name = strdup(p);              \
        box64env.is_##name##_overridden = 1;    \
        box64env.is_any_overridden = 1;         \
    }
    ENVSUPER()
#undef INTEGER
#undef INTEGER64
#undef BOOLEAN
#undef ADDRESS
#undef STRING
    applyCustomRules();
}

void PrintEnvVariables(box64env_t* env, int level)
{
    if (env->is_any_overridden) printf_log(level, "BOX64ENV: Variables overridden:\n");
#define INTEGER(NAME, name, default, min, max, wine) \
    if (env->is_##name##_overridden)                 \
        printf_log_prefix(0, level, "\t%s=%d\n", #NAME, env->name);
#define INTEGER64(NAME, name, default, wine) \
    if (env->is_##name##_overridden)         \
        printf_log_prefix(0, level, "\t%s=%lld\n", #NAME, env->name);
#define BOOLEAN(NAME, name, default, wine) \
    if (env->is_##name##_overridden)       \
        printf_log_prefix(0, level, "\t%s=%d\n", #NAME, env->name);
#define ADDRESS(NAME, name, wine)    \
    if (env->is_##name##_overridden) \
        printf_log_prefix(0, level, "\t%s=%p\n", #NAME, (void*)env->name);
#define STRING(NAME, name, wine)     \
    if (env->is_##name##_overridden) \
        printf_log_prefix(0, level, "\t%s=%s\n", #NAME, env->name);
    ENVSUPER()
#undef INTEGER
#undef INTEGER64
#undef BOOLEAN
#undef ADDRESS
#undef STRING
    if(env->is_dynarec_test_str_overridden && env->dynarec_test) {
        if(env->dynarec_test_end)
            printf_log(level, "\tBOX64_DYNAREC_TEST on range %p-%p\n", (void*)box64env.dynarec_test_start, (void*)box64env.dynarec_test_end);
        else
            printf_log(level, "\tBOX64_DYNAREC_TEST activated\n");
    }
    if(env->is_nodynarec_overridden && env->nodynarec_end)
        printf_log(level, "\tBOX64_NODYNAREC on range %p-%p\n", (void*)box64env.nodynarec_start, (void*)box64env.nodynarec_end);

}

typedef struct mapping_s {
    char*       filename;
    char*       fullname;
    box64env_t* env;
    uintptr_t   start;  //lower address of the map for this file
    mmaplist_t* mmaplist;
} mapping_t;

KHASH_MAP_INIT_STR(mapping_entry, mapping_t*);
static kh_mapping_entry_t* mapping_entries = NULL;

#if defined(DYNAREC) && !defined(WIN32)
void MmapDynaCache(mapping_t* mapping);
#endif


void RecordEnvMappings(uintptr_t addr, size_t length, int fd)
{
#ifndef _WIN32
    if (!envmap) { envmap = rbtree_init("envmap"); }
    if(!mapping_entries) mapping_entries = kh_init(mapping_entry);

    char* filename = NULL;
    static char fullname[4096];
    if (fd > 0) {
        static char buf[128];
        sprintf(buf, "/proc/self/fd/%d", fd);
        ssize_t r = readlink(buf, fullname, sizeof(fullname) - 1);
        if (r != -1) fullname[r] = 0;

        filename = strrchr(fullname, '/');
    }
    if (!filename) return;

    char* lowercase_filename = LowerCase(filename);

    int ret;
    mapping_t* mapping = NULL;
    khint_t k = kh_get(mapping_entry, mapping_entries, fullname);
    if(k == kh_end(mapping_entries)) {
        // First time we see this file
        if (box64_wine && BOX64ENV(unityplayer)) DetectUnityPlayer(lowercase_filename+1);
        if (box64_wine && BOX64ENV(dynarec_volatile_metadata)) ParseVolatileMetadata(fullname, (void*)addr);

        mapping = box_calloc(1, sizeof(mapping_t));
        mapping->filename = box_strdup(lowercase_filename);
        mapping->fullname = box_strdup(fullname);
        mapping->start = addr;
        k = kh_put(mapping_entry, mapping_entries, mapping->fullname, &ret);
        kh_value(mapping_entries, k) = mapping;
        if (box64env_entries) {
            khint_t k = kh_get(box64env_entry, box64env_entries, mapping->filename);
            if (k != kh_end(box64env_entries))
                mapping->env = &kh_value(box64env_entries, k);
        }
        dynarec_log(LOG_INFO, "Mapping %s (%s) in %p-%p\n", fullname, lowercase_filename, (void*)addr, (void*)(addr+length));
        #if defined(DYNAREC) && !defined(WIN32)
        int dynacache = box64env.dynacache;
        if(mapping->env && mapping->env->is_dynacache_overridden)
            dynacache = mapping->env->dynacache;
        if(dynacache)
            MmapDynaCache(mapping);
        #endif
    } else
        mapping = kh_value(mapping_entries, k);

    if(mapping && mapping->start>addr) { 
        dynarec_log(LOG_INFO, "Ignoring Mapping %s (%s) adjusted start: %p from %p\n", fullname, lowercase_filename, (void*)addr, (void*)(mapping->start)); 
        box_free(lowercase_filename);
        return;
    }
    if(BOX64ENV(dynarec_log)) {
        uintptr_t end; uintptr_t val;
        if(rb_get_end_64(envmap, addr, &val, &end))
            if(end<addr+length) {
                dynarec_log(LOG_INFO, "Enlarging Mapping %s (%s) %p-%p from %p\n", fullname, lowercase_filename, (void*)(mapping->start), (void*)(addr+length), (void*)end);
            }
    }
    rb_set_64(envmap, addr, addr + length, (uint64_t)mapping);
    if(mapping->env) {
        printf_log(LOG_DEBUG, "Applied [%s] of range %p:%p\n", filename, addr, addr + length);
        PrintEnvVariables(mapping->env, LOG_DEBUG);
    }
    box_free(lowercase_filename);
#endif
}

#ifdef DYNAREC
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
    There is 3 version to change when evoling things, depending on what is changed:
    1. FILE_VERSION for the DynaCache infrastructure
    2. DYNAREC_VERSION for dynablock_t changes and other global dynarec change
    3. ARCH_VERSION for the architecture specific changes (and there is one per arch)

    An ARCH_VERSION of 0 means Unsupported and disable DynaCache.
    Dynacache will ignore any DynaCache file not exactly matching those 3 version.
    `box64 --dynacache-clean` can be used from command line to purge obsolete DyaCache files
*/

#define FILE_VERSION               2
#define HEADER_SIGN "DynaCache"
#define SET_VERSION(MAJ, MIN, REV) (((MAJ)<<24)|((MIN)<<16)|(REV))
#ifdef ARM64
#define ARCH_VERSION SET_VERSION(0, 0, 10)
#elif defined(RV64)
#define ARCH_VERSION SET_VERSION(0, 0, 4)
#elif defined(LA64)
#define ARCH_VERSION SET_VERSION(0, 0, 4)
#else
#error meh!
#endif
#define DYNAREC_VERSION SET_VERSION(0, 0, 9)

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
    DS_GO(BOX64_aes, aes, 1)                                            \
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
    size_t u, d;
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
    size_t total = sizeof(DynaCacheHeader_t) + strlen(mapping->fullname) + 1 + nblocks*sizeof(DynaCacheBlock_t) + nLockAddresses*sizeof(uintptr_t) + nUnaligned*sizeof(uintptr_t);;
    total = (total + box64_pagesize-1)&~(box64_pagesize-1); // align on pagesize
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
    DynaCacheBlock_t* blocks = p;
    MmaplistFillBlocks(mapping->mmaplist, blocks);
    p += nblocks*sizeof(DynaCacheBlock_t);
    uintptr_t* lockAddresses = p;
    p += nLockAddresses*sizeof(uintptr_t);
    uintptr_t* unalignedAddresses = p;
    if(nLockAddresses)
        getLockAddressRange(mapping->start, map_len, lockAddresses);
    if(nUnaligned)
        getUnalignedRange(mapping->start, map_len, unalignedAddresses);
    // all done, now just create the file and write all this down...
    #ifndef WIN32
    unlink(mapname);
    FILE* f = fopen(mapname, "wbx");
    if(!f) {
        dynarec_log(LOG_INFO, "Cannot create cache file %s\n", mapname);
        return;
    }
    if(fwrite(all_header, total, 1, f)!=1) {
        dynarec_log(LOG_INFO, "Error writing Cache file (disk full?)\n");
        return;
    }
    for(int i=0; i<nblocks; ++i) {
        if(fwrite(blocks[i].block, blocks[i].size, 1, f)!=1) {
            dynarec_log(LOG_INFO, "Error writing Cache file (disk full?)\n");
            return;
        }
    }
    fclose(f);
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

#ifndef WIN32
int ReadDynaCache(const char* folder, const char* name, mapping_t* mapping, int verbose)
{
    char filename[strlen(folder)+strlen(name)+1];
    strcpy(filename, folder);
    strcat(filename, name);
    if(verbose) printf_log(LOG_NONE, "File %s:\t", name);
    if(!FileExist(filename, IS_FILE)) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Invalid file\n");
        return DCERR_NEXIST;
    }
    size_t filesize = FileSize(filename);
    if(filesize<sizeof(DynaCacheHeader_t)) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Invalid side: %zd\n", filesize);
        return DCERR_TOOSMALL;
    }
    FILE *f = fopen(filename, "rb");
    if(!f) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Cannot open file\n");
        return DCERR_FERROR;
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
    if(header.arch_version!=ARCH_VERSION) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Incompatible Dynarec Arch Version\n");
        fclose(f);
        return DCERR_DYNVER;
    }
    if(header.pagesize!=box64_pagesize) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Bad pagesize\n");
        fclose(f);
        return DCERR_PAGESIZE;
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
    DynaCacheBlock_t blocks[header.nblocks];
    if(fread(blocks, sizeof(DynaCacheBlock_t), header.nblocks, f)!=header.nblocks) {
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
    p = (p+box64_pagesize-1)&~(box64_pagesize-1);
    if(fseek(f, p, SEEK_SET)<0) {
        if(verbose) printf_log_prefix(0, LOG_NONE, "Error reading a block\n");
        fclose(f);
        return DCERR_FERROR;
    }
    if(!mapping) {
        // check the blocks can be read without reading...
        for(int i=0; i<header.nblocks; ++i) {
            p+=blocks[i].size;
            if(fseek(f, blocks[i].size, SEEK_CUR)<0 || ftell(f)!=p) {
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
            return DCERR_BADNAME;
        }
        if(verbose) {
            // check if name is coherent
            // file is valid, gives informations:
            printf_log_prefix(0, LOG_NONE, "%s (%s)\n", map_filename, NicePrintSize(filesize));
            printf_log_prefix(0, LOG_NONE, "\tDynarec Settings:\n");
            PrintDynfSettings(LOG_NONE, header.dynarec_settings);
            size_t total_blocks = 0, total_free = 0;
            size_t total_code = header.codesize;
            for(int i=0; i<header.nblocks; ++i) {
                total_blocks += blocks[i].size;
                total_free += blocks[i].free_size;
            }
            printf_log_prefix(0, LOG_NONE, "\tHas %d blocks for a total of %s", header.nblocks, NicePrintSize(total_blocks));
            printf_log_prefix(0, LOG_NONE, " with %s still free", NicePrintSize(total_free));
            printf_log_prefix(0, LOG_NONE, " and %s non-canceled blocks (mapped at %p-%p, with %zu lock and %zu unaligned addresses)\n", NicePrintSize(total_code), (void*)header.map_addr, (void*)header.map_addr+header.map_len, header.nLockAddresses, header.nUnalignedAddresses);
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
            if(MmaplistAddBlock(mapping->mmaplist, fd, p, blocks[i].block, blocks[i].size, delta_map, mapping->start)) {
                printf_log(LOG_NONE, "Error while doing relocation on a DynaCache (block %d)\n", i);
                fclose(f);
                return DCERR_RELOC;
            }
            p+=blocks[i].size;
        }
        for(size_t i=0; i<header.nLockAddresses; ++i)
            addLockAddress(lockAddresses[i]+delta_map);
        for(size_t i=0; i<header.nUnalignedAddresses; ++i)
            add_unaligned_address(lockAddresses[i]+delta_map);
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
    }
    struct dirent* d = NULL;
    int need_filter = (filter && strlen(filter));
    while(d = readdir(dir)) {
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
    }
    struct dirent* d = NULL;
    while(d = readdir(dir)) {
        size_t l = strlen(d->d_name);
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
                    printf_log(LOG_NONE, "Could not remove %d\n", d->d_name);
                }
            }
        }
    }
    closedir(dir);
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

void WillRemoveMapping(uintptr_t addr, size_t length)
{
    #ifdef DYNAREC
    if(!envmap) return;
    mapping_t* mapping = (mapping_t*)rb_get_64(envmap, addr);
    if(mapping) {
        if(MmaplistHasNew(mapping->mmaplist, 1)) {
            mutex_lock(&my_context->mutex_dyndump);
            SerializeMmaplist(mapping);
            mutex_unlock(&my_context->mutex_dyndump);
        }
    }
    #endif
}

void RemoveMapping(uintptr_t addr, size_t length)
{
    if(!envmap) return;
    mapping_t* mapping = (mapping_t*)rb_get_64(envmap, addr);
    rb_unset(envmap, addr, addr+length);
    // quick check at next address
    if(mapping) {
        if(mapping == (mapping_t*)rb_get_64(envmap, addr+length))
            return; // still present, don't purge mapping
        // Will traverse the tree to find any left over
        uintptr_t start = rb_get_leftmost(envmap);
        uintptr_t end;
        uint64_t val;
        do {
            rb_get_end_64(envmap, start, &val, &end);
            if((mapping_t*)val==mapping)
                return; // found more occurance, exiting
            start = end;
        } while(end!=UINTPTR_MAX);
        // no occurence found, delete mapping
        dynarec_log(LOG_INFO, "Delete Mapping %s (%s) in %p(%p)-%p\n", mapping->fullname, mapping->filename, (void*)addr, (void*)mapping->start, (void*)(addr+length));
        khint_t k = kh_get(mapping_entry, mapping_entries, mapping->fullname);
        if(k!=kh_end(mapping_entries))
            kh_del(mapping_entry, mapping_entries, k);
	#ifdef DYNAREC
        if(mapping->mmaplist)
            DelMmaplist(mapping->mmaplist);
	#endif
        box_free(mapping->filename);
        box_free(mapping->fullname);
        box_free(mapping);
    }
}

void SerializeAllMapping()
{
#ifdef DYNAREC
    mapping_t* mapping;
    mutex_lock(&my_context->mutex_dyndump);
    kh_foreach_value(mapping_entries, mapping, 
        if(MmaplistHasNew(mapping->mmaplist, 1))
            SerializeMmaplist(mapping);
    );
    mutex_unlock(&my_context->mutex_dyndump);
#endif
}

box64env_t* GetCurEnvByAddr(uintptr_t addr)
{
    if (!envmap) return &box64env;
    mapping_t* mapping = ((mapping_t*)rb_get_64(envmap, addr));
    if(!mapping) return &box64env;
    box64env_t* env = mapping->env;
    if(!env) return &box64env;
    return env;
}

mmaplist_t* GetMmaplistByAddr(uintptr_t addr)
{
    #ifdef DYNAREC
    if (!envmap) return NULL;
    mapping_t* mapping = ((mapping_t*)rb_get_64(envmap, addr));
    if(!mapping) return NULL;
    mmaplist_t* list = mapping->mmaplist;
    if(!list)
        list = mapping->mmaplist = NewMmaplist();
    return list;
    #else
    return NULL;
    #endif
}


int IsAddrFileMapped(uintptr_t addr, const char** filename, uintptr_t* start)
{
    if(!envmap) return 0;
    mapping_t* mapping = ((mapping_t*)rb_get_64(envmap, addr));
    if(!mapping) return 0;
    if(mapping->fullname) {
        if(filename) *filename = mapping->fullname;
        if(start) *start = mapping->start;
        return 1;
    }
    return 0;
}

size_t SizeFileMapped(uintptr_t addr)
{
    if(!envmap) return 0;
    uint64_t val = 0;
    uintptr_t end = 0;
    if(rb_get_end_64(envmap, addr, &val, &end)) {
        mapping_t* mapping = (mapping_t*)val;
        if(mapping && (mapping->start<end))
            return end - mapping->start;
    }
    return 0;
}

int IsAddrNeedReloc(uintptr_t addr)
{
    box64env_t* env = GetCurEnvByAddr(addr);
    // TODO: this seems quite wrong and should be refactored
    int test = env->is_dynacache_overridden?env->dynacache:box64env.dynacache;
    if(test!=1)
        return 0;
    uintptr_t end = env->nodynarec_end?env->nodynarec_end:box64env.nodynarec_end;
    uintptr_t start = env->nodynarec_start?env->nodynarec_start:box64env.nodynarec_start;
    if(end && addr>=start && addr<end)
        return 0;
     // don't do serialize for program that needs dirty=1 or 2 (maybe 1 is ok?)
    if(env && env->is_dynarec_dirty_overridden && env->dynarec_dirty)
        return 0;
    if((!env || !env->is_dynarec_dirty_overridden) && box64env.dynarec_dirty)
        return 0;
   #ifdef HAVE_TRACE
    end = env->dynarec_test_end?env->dynarec_test_end:box64env.dynarec_test_end;
    start = env->dynarec_test_start?env->dynarec_test_start:box64env.dynarec_test_start;
    if(end && addr>=start && addr<end)
        return 0;
    test = env->is_dynarec_trace_overridden?env->dynarec_trace:box64env.dynarec_trace;
    if(test && trace_end && addr>=trace_start && addr<trace_end)
        return 0;
    #endif
    return 1;
}

int IsAddrMappingLoadAndClean(uintptr_t addr)
{
    #ifdef DYNAREC
    if(!envmap) return 0;
    mapping_t* mapping = ((mapping_t*)rb_get_64(envmap, addr));
    if(!mapping) return 0;
    if(!mapping->mmaplist) return 0;
    if(MmaplistIsDirty(mapping->mmaplist)) return 0;
    return 1;
    #else
    return 0;
    #endif
}
