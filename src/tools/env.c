#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <fcntl.h>
#include <string.h>
#include <inttypes.h>

#include "os.h"
#include "env.h"
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
"[steam-runtime-check-requirements]\n"
"BOX64_EXIT=1\n"
"\n"
"[steam-runtime-launcher-service]\n"
"BOX64_EXIT=1\n"
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

    if (box64env.maxcpu == 0 || (!box64_wine && box64env.new_maxcpu < box64env.maxcpu)) {
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

static void pushNewEntry(const char* name, box64env_t* env, int gen)
{
    khint_t k;
    kh_box64env_entry_t* khp = gen ? box64env_entries_gen : box64env_entries;
    k = kh_get(box64env_entry, khp, name);
    if (k == kh_end(khp)) {
        int ret;
        k = kh_put(box64env_entry, khp, strdup(name), &ret);
    } else {
        freeEnv(&kh_value(khp, k));
    }
    box64env_t* p = &kh_value(khp, k);
    memcpy(p, env, sizeof(box64env_t));
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

static char old_entryname[256] = "";
const char* GetLastApplyEntryName()
{
    return old_entryname;
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
#define STRING(NAME, name, wine)             \
    p = GETENV(#NAME, wine);                 \
    if (p) {                                 \
        box64env.name = strdup(p);           \
        box64env.is_##name##_overridden = 1; \
        box64env.is_any_overridden = 1;      \
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
} mapping_t;

KHASH_MAP_INIT_STR(mapping_entry, mapping_t*);
static kh_mapping_entry_t* mapping_entries = NULL;

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
    } else
        mapping = kh_value(mapping_entries, k);

    if(mapping && mapping->start>addr) { 
        dynarec_log(LOG_INFO, "Mapping %s (%s) adjusted start: %p from %p\n", fullname, lowercase_filename, (void*)addr, (void*)(mapping->start)); 
        mapping->start = addr;
    }
    rb_set_64(envmap, addr, addr + length, (uint64_t)mapping);
    if(mapping->env) {
        printf_log(LOG_DEBUG, "Applied [%s] of range %p:%p\n", filename, addr, addr + length);
        PrintEnvVariables(mapping->env, LOG_DEBUG);
    }
    box_free(lowercase_filename);
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
        uintptr_t start = rb_get_lefter(envmap);
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
        box_free(mapping->filename);
        box_free(mapping->fullname);
        box_free(mapping);
    }
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
