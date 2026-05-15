#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <fcntl.h>
#include <string.h>
#include <inttypes.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

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
#include "dynacache.h"
#include "env_private.h"

box64env_t box64env = { 0 };

KHASH_MAP_INIT_STR(box64env_entry, box64env_t)
static kh_box64env_entry_t* box64env_entries = NULL;
static kh_box64env_entry_t* box64env_entries_wildcard = NULL;

#ifdef DYNAREC
void DelMmaplist(mmaplist_t* list);
int MmaplistHasNew(mmaplist_t* list, int clear);
int MmaplistIsDirty(mmaplist_t* list);
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

extern int box64_cycle_log_initialized;

static void applyCustomRules()
{
    if (BOX64ENV(log) == LOG_NEVER) {
        SET_BOX64ENV(log, BOX64ENV(log) - 1);
        SET_BOX64ENV(dump, 1);
    }

#ifndef _WIN32
    if (box64env.is_cycle_log_overridden) {
        box64env.rolling_log = BOX64ENV(cycle_log);

        if (BOX64ENV(rolling_log) == 1) {
            box64env.rolling_log = 16;
        }
        if (BOX64ENV(rolling_log) && BOX64ENV(log) > LOG_INFO) {
            box64env.rolling_log = 0;
        }
        if (!box64_cycle_log_initialized) initCycleLog(my_context);
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
        } else if (!strcasecmp(box64env.profile, "default")) {
        } else if (!strcasecmp(box64env.profile, "fast")) {
            SET_BOX64ENV_IF_EMPTY(dynarec_callret, 1);
            SET_BOX64ENV_IF_EMPTY(dynarec_sep, 1);
            SET_BOX64ENV_IF_EMPTY(dynarec_bigblock, 3);
            SET_BOX64ENV_IF_EMPTY(dynarec_safeflags, 0);
            SET_BOX64ENV_IF_EMPTY(dynarec_strongmem, 1);
            SET_BOX64ENV_IF_EMPTY(dynarec_dirty, 1);
            SET_BOX64ENV_IF_EMPTY(dynarec_forward, 1024);
        } else if (!strcasecmp(box64env.profile, "fastest")) {
            SET_BOX64ENV_IF_EMPTY(dynarec_callret, 1);
            SET_BOX64ENV_IF_EMPTY(dynarec_sep, 2);
            SET_BOX64ENV_IF_EMPTY(dynarec_bigblock, 3);
            SET_BOX64ENV_IF_EMPTY(dynarec_safeflags, 0);
            SET_BOX64ENV_IF_EMPTY(dynarec_strongmem, 0);
            SET_BOX64ENV_IF_EMPTY(dynarec_dirty, 1);
            SET_BOX64ENV_IF_EMPTY(dynarec_forward, 1024);
        } else {
            static int warned = 0;
            if (!warned) {
                printf_log(LOG_INFO, "Warning, unknown choice for BOX64_PROFILE: %s, choices are: safest,safe,default,fast,fastest.\n", box64env.profile);
                warned = 1;
            }
        }
    }

    if (box64env.maxcpu == 0 || (box64env.new_maxcpu < box64env.maxcpu)) {
        box64env.maxcpu = box64env.new_maxcpu;
        if (box64env.maxcpu && box64_sysinfo.ncpu > (uint64_t)box64env.maxcpu) {
            box64_sysinfo.box64_ncpu = (uint64_t)box64env.maxcpu;
        }

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
#elif defined(PPC64LE)
#define ENV_ARCH "ppc64le"
#elif defined(X86_64)
#define ENV_ARCH "x86_64"
#else
#warning "Unknown architecture for ENV_ARCH"
#define ENV_ARCH "unknown"
#endif

static void pushNewEntry(const char* name, box64env_t* env, int wildcard)
{
    if (env->is_arch_overridden && *env->arch == '\0') env->is_arch_overridden = 0;

    // Arch specific but not match, ignore
    if (env->is_arch_overridden && strcasecmp(env->arch, ENV_ARCH)) {
        freeEnv(env);
        return;
    }
    khint_t k;
    kh_box64env_entry_t* khp = wildcard ? box64env_entries_wildcard : box64env_entries;
    k = kh_get(box64env_entry, khp, name);
    // No entry exist, add a new one
    if (k == kh_end(khp)) {
        int ret;
        k = kh_put(box64env_entry, khp, box_strdup(name), &ret);
        box64env_t* p = &kh_value(khp, k);
        memcpy(p, env, sizeof(box64env_t));
        return;
    }

    // Entry exists, replace it if the new one is arch specific or has higher priority
    if (env->is_arch_overridden && !strcasecmp(env->arch, ENV_ARCH) || env->priority > kh_value(khp, k).priority) {
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
    } else {
        freeEnv(env);
    }
}

static void internalApplyEnvFileEntry(const char* entryname, const box64env_t* env);

#ifdef ANDROID
static int shm_open(const char *name, int oflag, mode_t mode) {
    return -1;
}
static int shm_unlink(const char *name) {
    return -1;
}
#endif

static void initializeEnvFile(const char* filename, int priority)
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
    if (!box64env_entries_wildcard)
        box64env_entries_wildcard = kh_init(box64env_entry);

    box64env_t current_env = { 0 };
    current_env.priority = priority;
    size_t linesize = 0, len = 0;
    char* current_name = NULL;
    int current_is_wildcard = 0;
    int current_is_shared = 0;
    char line[1024];
    while ((box_fgets(line, 1024, f)) != NULL) {
        // remove comments
        char* p = strchr(line, '#');
        if (p) *p = '\0';
        trimStringInplace(line);
        len = strlen(line);
        if (line[0] == '[' && strchr(line, ']')) {
            // new entry, push the previous one
            if (current_name) {
                if (current_is_shared) {
                    internalApplyEnvFileEntry("*", &current_env);
                } else {
                    pushNewEntry(current_name, &current_env, current_is_wildcard);
                }
            }
            char* end = strchr(line, ']');
            memset(&current_env, 0, sizeof(current_env));
            current_env.priority = priority;
            box_free(current_name);
            current_is_shared = (line[1] == '*' && end == line + 2);
            current_is_wildcard = (!current_is_shared && line[1] == '*' && end > line + 2 && end[-1] == '*');
            size_t start = current_is_wildcard ? 2 : 1;
            size_t skip_end = current_is_wildcard ? 1 : 0;
            size_t name_len = (uintptr_t)end - (uintptr_t)line - start - skip_end;
            current_name = box_calloc(1, name_len + 1);
            memcpy(current_name, line + start, name_len);
            trimStringInplace(current_name);
            char* lowercase_name = LowerCase(current_name);
            box_free(current_name);
            current_name = lowercase_name;
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
        if (current_is_shared) {
            internalApplyEnvFileEntry("*", &current_env);
        } else {
            pushNewEntry(current_name, &current_env, current_is_wildcard);
        }
        box_free(current_name);
    }
    box_fclose(f);
}


void InitializeEnvFiles()
{
    int priority = 0;
#ifndef _WIN32 // FIXME: this needs some consideration on Windows, so for now, only do it on Linux
    if (BOX64ENV(envfile) && FileExist(BOX64ENV(envfile), IS_FILE))
        initializeEnvFile(BOX64ENV(envfile), priority++);
#ifndef TERMUX
    else if (FileExist("/etc/box64.box64rc", IS_FILE))
        initializeEnvFile("/etc/box64.box64rc", priority++);
    else if (FileExist("/data/data/com.termux/files/usr/glibc/etc/box64.box64rc", IS_FILE))
        initializeEnvFile("/data/data/com.termux/files/usr/glibc/etc/box64.box64rc", priority++);
#else
    else if (FileExist("/data/data/com.termux/files/usr/etc/box64.box64rc", IS_FILE))
        initializeEnvFile("/data/data/com.termux/files/usr/etc/box64.box64rc", priority++);
#endif
    else
        initializeEnvFile(NULL, priority++); // load default rcfile
#endif

    char* p = GetEnv(HOME);
    if (p) {
        static char tmp[4096];
        strncpy(tmp, p, 4095);
        strncat(tmp, PATHSEP ".box64rc", 4095);
        if (FileExist(tmp, IS_FILE)) {
            initializeEnvFile(tmp, priority++);
        }
    }

    // in case there is a shared entry [*]
    applyCustomRules();
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
int ApplyEnvFileEntry(const char* entryname)
{
    if (!entryname || !box64env_entries) return 0;
    if (!strcasecmp(entryname, old_entryname)) return 0;

    int ret = 0;
    strncpy(old_entryname, entryname, 255);
    khint_t k1;
    {
        char* lowercase_entryname = LowerCase(entryname);
        k1 = kh_get(box64env_entry, box64env_entries, lowercase_entryname);
        box64env_t* env;
        const char* k2;
        // clang-format off
        kh_foreach_ref(box64env_entries_wildcard, k2, env,
            if (strstr(lowercase_entryname, k2)) {
                internalApplyEnvFileEntry(entryname, env);
                applyCustomRules();
                ret = 1;
            }
        )
        box_free(lowercase_entryname);
        // clang-format on
    }
    if (k1 == kh_end(box64env_entries)) return ret;

    box64env_t* env = &kh_value(box64env_entries, k1);
    internalApplyEnvFileEntry(entryname, env);
    applyCustomRules();
    return 1;
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
    if(strstr(lowercase_filename, "/memfd:")==lowercase_filename) {
        // memfd, first remove the (deleted) at the end
        char* p = strstr(lowercase_filename, " (deleted)");
        if(p=lowercase_filename+strlen(lowercase_filename)-strlen(" (deleted)"))
            *p = 0;
        // add the "/fd" at the end to differenciate between memfd
        char* new_name = box_calloc(1, strlen(lowercase_filename)+100);
        sprintf(new_name, "%s/%d", lowercase_filename, fd);
        box_free(lowercase_filename);
        lowercase_filename = new_name;
    }
    mutex_lock(&my_context->mutex_dyndump);
    int ret;
    mapping_t* mapping = NULL;
    int new_mapping = 0;
    khint_t k = kh_get(mapping_entry, mapping_entries, fullname);
    if(k == kh_end(mapping_entries)) {
        new_mapping = 1;
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
        dynarec_log(LOG_INFO, "Mapping of fd:%d %s (%s) in %p-%p\n", fd, fullname, lowercase_filename, (void*)addr, (void*)(addr+length));
    } else
        mapping = kh_value(mapping_entries, k);

    if(mapping && mapping->start>addr) { 
        dynarec_log(LOG_INFO, "Ignoring Mapping of fd:%d %s (%s) adjusted start: %p from %p\n", fd, fullname, lowercase_filename, (void*)addr, (void*)(mapping->start)); 
        box_free(lowercase_filename);
        mutex_unlock(&my_context->mutex_dyndump);
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
    mutex_unlock(&my_context->mutex_dyndump);
    if(new_mapping) {
        // First time we see this file
        if (box64_wine && BOX64ENV(unityplayer)) DetectUnityPlayer(lowercase_filename+1);
        if (box64_wine && !box64_is32bits && BOX64ENV(dynarec_volatile_metadata)) ParseVolatileMetadata(fullname, (void*)addr);
        #if defined(DYNAREC) && !defined(WIN32)
        int dynacache = box64env.dynacache;
        if(mapping->env && mapping->env->is_dynacache_overridden)
            dynacache = mapping->env->dynacache;
        if(dynacache)
            MmapDynaCache(mapping);
        #endif
    }
    if(mapping->env) {
        printf_log(LOG_DEBUG, "Applied [%s] of range %p:%p\n", filename, addr, addr + length);
        PrintEnvVariables(mapping->env, LOG_DEBUG);
    }
    box_free(lowercase_filename);
#endif
}

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
    #ifdef DYNAREC
    mmaplist_t* list = NULL;
    #endif
    mutex_lock(&my_context->mutex_dyndump);
    mapping_t* mapping = (mapping_t*)rb_get_64(envmap, addr);
    rb_unset(envmap, addr, addr+length);
    // quick check at next address
    if(mapping) {
        if(mapping == (mapping_t*)rb_get_64(envmap, addr+length)) {
            mutex_unlock(&my_context->mutex_dyndump);
            return; // still present, don't purge mapping
        }
        // Will traverse the tree to find any left over
        uintptr_t start = rb_get_leftmost(envmap);
        uintptr_t end;
        uint64_t val;
        do {
            rb_get_end_64(envmap, start, &val, &end);
            if((mapping_t*)val==mapping) {
                mutex_unlock(&my_context->mutex_dyndump);
                return; // found more occurance, exiting
            }
            start = end;
        } while(end!=UINTPTR_MAX);
        // no occurence found, delete mapping
        dynarec_log(LOG_INFO, "Delete Mapping %s (%s) in %p(%p)-%p\n", mapping->fullname, mapping->filename, (void*)addr, (void*)mapping->start, (void*)(addr+length));
        khint_t k = kh_get(mapping_entry, mapping_entries, mapping->fullname);
        if(k!=kh_end(mapping_entries))
            kh_del(mapping_entry, mapping_entries, k);
	#ifdef DYNAREC
        if(mapping->mmaplist) list = mapping->mmaplist; // deferring the deletion because of
	#endif
        box_free(mapping->filename);
        box_free(mapping->fullname);
        box_free(mapping);
    }
    mutex_unlock(&my_context->mutex_dyndump);
    #ifdef DYNAREC
    if(list) DelMmaplist(list);
    #endif
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

int IsAddrFileMappedNoMemFD(uintptr_t addr)
{
    const char* filename = NULL;
    if(!IsAddrFileMapped(addr, &filename, NULL))
        return 0;
    if(!filename)
        return 0;
    if(strstr(filename, "/memfd:")==filename)
        return 0;
    return 1;
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
