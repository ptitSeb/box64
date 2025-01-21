#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "env.h"
#include "khash.h"
#include "debug.h"
#include "fileutils.h"
#include "box64context.h"

box64env_t box64env = { 0 };

KHASH_MAP_INIT_STR(box64env_entry, box64env_t)
static kh_box64env_entry_t* box64env_entries = NULL;
static kh_box64env_entry_t* box64env_entries_gen = NULL;


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

static void applyCustomRules()
{
    if (BOX64ENV(log) == LOG_NEVER) {
        SET_BOX64ENV(log, BOX64ENV(log) - 1);
        SET_BOX64ENV(dump, 1);
    }

    if (BOX64ENV(rolling_log) == 1) {
        SET_BOX64ENV(rolling_log, 16);
    }
    if (BOX64ENV(rolling_log) && BOX64ENV(log) > LOG_INFO) {
        SET_BOX64ENV(rolling_log, 0);
    }

    if (box64env.is_dynarec_test_str_overridden) {
        if (strlen(box64env.dynarec_test_str) == 1) {
            if (box64env.dynarec_test_str[0] >= '0' && box64env.dynarec_test_str[0] <= '1')
                box64env.dynarec_test = box64env.dynarec_test_str[0] - '0';

            box64env.dynarec_test_start = 0x0;
            box64env.dynarec_test_end = 0x0;
        } else if (strchr(box64env.dynarec_test_str, '-')) {
            if (sscanf(box64env.dynarec_test_str, "%ld-%ld", &box64env.dynarec_test_start, &box64env.dynarec_test_end) != 2) {
                if (sscanf(box64env.dynarec_test_str, "0x%lX-0x%lX", &box64env.dynarec_test_start, &box64env.dynarec_test_end) != 2)
                    sscanf(box64env.dynarec_test_str, "%lx-%lx", &box64env.dynarec_test_start, &box64env.dynarec_test_end);
            }
            if (box64env.dynarec_test_end > box64env.dynarec_test_start) {
                box64env.dynarec_test = 1;
            } else {
                box64env.dynarec_test = 0;
            }
        }
    }

    if (box64env.is_nodynarec_overridden) {
        if(box64env.nodynarec) {
            if (strchr(box64env.nodynarec,'-')) {
                if(sscanf(box64env.nodynarec, "%ld-%ld", &box64env.nodynarec_start, &box64env.nodynarec_end)!=2) {
                    if(sscanf(box64env.nodynarec, "0x%lX-0x%lX", &box64env.nodynarec_start, &box64env.nodynarec_end)!=2)
                        sscanf(box64env.nodynarec, "%lx-%lx", &box64env.nodynarec_start, &box64env.nodynarec_end);
                }
            }
        }
    }

    if (box64env.dynarec_test) {
        SET_BOX64ENV(dynarec_fastnan, 0);
        SET_BOX64ENV(dynarec_fastround, 0);
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

    if (box64env.dynarec_perf_map) {
        char pathname[32];
        snprintf(pathname, sizeof(pathname), "/tmp/perf-%d.map", getpid());
        SET_BOX64ENV(dynarec_perf_map_fd, open(pathname, O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR));
    }
    if (!box64env.libgl) {
        const char *p = getenv("SDL_VIDEO_GL_DRIVER");
        if(p) SET_BOX64ENV(libgl, box_strdup(p));
    }
    if (box64env.avx == 2) {
        box64env.avx = 1;
        box64env.avx2 = 1;
    }

    if (box64env.exit) exit(0);

    if (box64env.env) setenv("BOX64_ENV", "1", 1);
    if (box64env.env1) setenv("BOX64_ENV1", "1", 1);
    if (box64env.env2) setenv("BOX64_ENV2", "1", 1);
    if (box64env.env3) setenv("BOX64_ENV3", "1", 1);
    if (box64env.env4) setenv("BOX64_ENV4", "1", 1);
    if (box64env.env5) setenv("BOX64_ENV5", "1", 1);
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
#define INTEGER(NAME, name, default, min, max)
#define INTEGER64(NAME, name, default)
#define BOOLEAN(NAME, name, default)
#define ADDRESS(NAME, name)
#define STRING(NAME, name) box_free(env->name);
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

    FILE* f = NULL;
    if (filename) 
        f = fopen(filename, "r");
    else {
        #define TMP_MEMRCFILE  "/box64_rcfile"
        int tmp = shm_open(TMP_MEMRCFILE, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return; // error, bye bye
        shm_unlink(TMP_MEMRCFILE);    // remove the shm file, but it will still exist because it's currently in use
        int dummy = write(tmp, default_rcfile, sizeof(default_rcfile));
        (void)dummy;
        lseek(tmp, 0, SEEK_SET);
        f = fdopen(tmp, "r");
    }
    if (!f) {
        printf("Error: Cannot open env file %s\n", filename);
        return;
    }

    if (!box64env_entries)
        box64env_entries = kh_init(box64env_entry);
    if (!box64env_entries_gen)
        box64env_entries_gen = kh_init(box64env_entry);

    box64env_t current_env = { 0 };
    size_t linesize = 0, len = 0;
    char *line = NULL, *current_name = NULL;
    int ret;
    bool is_wildcard_name = false;
    while ((ret = getline(&line, &linesize, f)) != -1) {
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
#define INTEGER(NAME, name, default, min, max)      \
    else if (!strcmp(key, #NAME))                   \
    {                                               \
        int v = strtol(val, &p, 0);                 \
        if (p != val && v >= min && v <= max) {     \
            current_env.is_##name##_overridden = 1; \
            current_env.name = v;                   \
        }                                           \
    }
#define INTEGER64(NAME, name, default)              \
    else if (!strcmp(key, #NAME))                   \
    {                                               \
        int64_t v = strtoll(val, &p, 0);            \
        if (p != val) {                             \
            current_env.is_##name##_overridden = 1; \
            current_env.name = v;                   \
        }                                           \
    }
#define BOOLEAN(NAME, name, default)                \
    else if (!strcmp(key, #NAME))                   \
    {                                               \
        if (strcmp(val, "0")) {                     \
            current_env.is_##name##_overridden = 1; \
            current_env.name = 1;                   \
        } else {                                    \
            current_env.is_##name##_overridden = 1; \
            current_env.name = 0;                   \
        }                                           \
    }
#define ADDRESS(NAME, name)                           \
    else if (!strcmp(key, #NAME))                     \
    {                                                 \
        uintptr_t v = (uintptr_t)strtoll(val, &p, 0); \
        if (p != val) {                               \
            current_env.is_##name##_overridden = 1;   \
            current_env.name = v;                     \
        }                                             \
    }
#define STRING(NAME, name)                                \
    else if (!strcmp(key, #NAME))                         \
    {                                                     \
        current_env.is_##name##_overridden = 1;           \
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
        }
    }
    // push the last entry
    if (current_name) {
        pushNewEntry(current_name, &current_env, is_wildcard_name);
        box_free(current_name);
    }
    box_free(line);
    fclose(f);
}


void InitializeEnvFiles()
{
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

    char* p = getenv("HOME");
    if (p) {
        static char tmp[4096];
        strncpy(tmp, p, 4095);
        strncat(tmp, "/.box64rc", 4095);
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
static void internalEnvFileEntry(const char* entryname, const box64env_t* env)
{
#define INTEGER(NAME, name, default, min, max) \
    if (env->is_##name##_overridden) {         \
        box64env.name = env->name;             \
        box64env.is_##name##_overridden = 1;   \
    }
#define INTEGER64(NAME, name, default)       \
    if (env->is_##name##_overridden) {       \
        box64env.name = env->name;           \
        box64env.is_##name##_overridden = 1; \
    }
#define BOOLEAN(NAME, name, default)         \
    if (env->is_##name##_overridden) {       \
        box64env.name = env->name;           \
        box64env.is_##name##_overridden = 1; \
    }
#define ADDRESS(NAME, name)                  \
    if (env->is_##name##_overridden) {       \
        box64env.name = env->name;           \
        box64env.is_##name##_overridden = 1; \
    }
#define STRING(NAME, name)                   \
    if (env->is_##name##_overridden) {       \
        box64env.name = env->name;           \
        box64env.is_##name##_overridden = 1; \
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
                internalEnvFileEntry(entryname, env);)
            box_free(lowercase_entryname);
    }
    if (k1 == kh_end(box64env_entries)) return;

    box64env_t* env = &kh_value(box64env_entries, k1);
    internalEnvFileEntry(entryname, env);
    applyCustomRules();
}

void LoadEnvVariables()
{
#define INTEGER(NAME, name, default, min, max) box64env.name = default;
#define INTEGER64(NAME, name, default)         box64env.name = default;
#define BOOLEAN(NAME, name, default)           box64env.name = default;
#define ADDRESS(NAME, name)                    box64env.name = 0;
#define STRING(NAME, name)                     box64env.name = NULL;
    ENVSUPER()
#undef INTEGER
#undef INTEGER64
#undef BOOLEAN
#undef ADDRESS
#undef STRING

    char* p;
    // load env vars from getenv()
#define INTEGER(NAME, name, default, min, max)            \
    p = getenv(#NAME);                                    \
    if (p) {                                              \
        box64env.name = atoi(p);                          \
        if (box64env.name < min || box64env.name > max) { \
            box64env.name = default;                      \
        } else {                                          \
            box64env.is_##name##_overridden = 1;          \
        }                                                 \
    }
#define INTEGER64(NAME, name, default)       \
    p = getenv(#NAME);                       \
    if (p) {                                 \
        box64env.name = atoll(p);            \
        box64env.is_##name##_overridden = 1; \
    }
#define BOOLEAN(NAME, name, default)         \
    p = getenv(#NAME);                       \
    if (p) {                                 \
        box64env.name = p[0] != '0';         \
        box64env.is_##name##_overridden = 1; \
    }
#define ADDRESS(NAME, name)                  \
    p = getenv(#NAME);                       \
    if (p) {                                 \
        box64env.name = (uintptr_t)atoll(p); \
        box64env.is_##name##_overridden = 1; \
    }
#define STRING(NAME, name)                   \
    p = getenv(#NAME);                       \
    if (p) {                                 \
        box64env.name = strdup(p);           \
        box64env.is_##name##_overridden = 1; \
    }
    ENVSUPER()
#undef INTEGER
#undef INTEGER64
#undef BOOLEAN
#undef ADDRESS
#undef STRING
    applyCustomRules();
}

void PrintEnvVariables()
{
    printf_log(LOG_INFO, "BOX64ENV: Variables overridden via env and/or RC file:\n");
#define INTEGER(NAME, name, default, min, max) \
    if (box64env.is_##name##_overridden)       \
        printf_log(LOG_INFO, "\t%s=%d\n", #NAME, box64env.name);
#define INTEGER64(NAME, name, default)   \
    if (box64env.is_##name##_overridden) \
        printf_log(LOG_INFO, "\t%s=%lld\n", #NAME, box64env.name);
#define BOOLEAN(NAME, name, default)     \
    if (box64env.is_##name##_overridden) \
        printf_log(LOG_INFO, "\t%s=%d\n", #NAME, box64env.name);
#define ADDRESS(NAME, name)              \
    if (box64env.is_##name##_overridden) \
        printf_log(LOG_INFO, "\t%s=%p\n", #NAME, (void*)box64env.name);
#define STRING(NAME, name)               \
    if (box64env.is_##name##_overridden) \
        printf_log(LOG_INFO, "\t%s=%s\n", #NAME, box64env.name);
    ENVSUPER()
#undef INTEGER
#undef INTEGER64
#undef BOOLEAN
#undef ADDRESS
#undef STRING
}
