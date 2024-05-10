#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "debug.h"
#include "rcfile.h"
#include "box64context.h"
#include "fileutils.h"
#include "pathcoll.h"
#include "x64emu.h"
#ifdef HAVE_TRACE
#include "x64trace.h"
#endif
#include "custommem.h"
#include "khash.h"

// This file handle the box64rc files
// file are basicaly ini file, with section [XXXX] defining the name of the process
// and BOX64_XXXX=YYYY entry like the env. var. variables

// default rcfile
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

// list of all entries
#define SUPER1()                                        \
ENTRYINTPOS(BOX64_ROLLING_LOG, new_cycle_log)           \
ENTRYSTRING_(BOX64_LD_LIBRARY_PATH, ld_library_path)    \
ENTRYSTRING_(BOX64_PATH, box64_path)                    \
ENTRYSTRING_(BOX64_TRACE_FILE, trace_file)              \
ENTRYADDR(BOX64_LOAD_ADDR, box64_load_addr)             \
ENTRYINT(BOX64_LOG, box64_log, 0, 3, 2)                 \
ENTRYBOOL(BOX64_DUMP, box64_dump)                       \
ENTRYBOOL(BOX64_DLSYM_ERROR, dlsym_error)               \
CENTRYBOOL(BOX64_NOSIGSEGV, no_sigsegv)                 \
CENTRYBOOL(BOX64_NOSIGILL, no_sigill)                   \
ENTRYBOOL(BOX64_SHOWSEGV, box64_showsegv)               \
ENTRYBOOL(BOX64_SHOWBT, box64_showbt)                   \
ENTRYBOOL(BOX64_MMAP32, box64_mmap32)                   \
ENTRYBOOL(BOX64_IGNOREINT3, box64_ignoreint3)           \
IGNORE(BOX64_RDTSC)                                     \
ENTRYBOOL(BOX64_X11THREADS, box64_x11threads)           \
ENTRYBOOL(BOX64_X11GLX, box64_x11glx)                   \
ENTRYDSTRING(BOX64_LIBGL, box64_libGL)                  \
ENTRYBOOL(BOX64_SSE_FLUSHTO0, box64_sse_flushto0)       \
ENTRYBOOL(BOX64_X87_NO80BITS, box64_x87_no80bits)       \
ENTRYBOOL(BOX64_SYNC_ROUNDING, box64_sync_rounding)     \
ENTRYSTRING_(BOX64_EMULATED_LIBS, emulated_libs)        \
ENTRYBOOL(BOX64_ALLOWMISSINGLIBS, allow_missing_libs)   \
ENTRYBOOL(BOX64_PREFER_WRAPPED, box64_prefer_wrapped)   \
ENTRYBOOL(BOX64_PREFER_EMULATED, box64_prefer_emulated) \
ENTRYBOOL(BOX64_CRASHHANDLER, box64_dummy_crashhandler) \
ENTRYBOOL(BOX64_NOPULSE, box64_nopulse)                 \
ENTRYBOOL(BOX64_NOGTK, box64_nogtk)                     \
ENTRYBOOL(BOX64_NOVULKAN, box64_novulkan)               \
ENTRYBOOL(BOX64_SSE42, box64_sse42)                     \
ENTRYBOOL(BOX64_FUTEX_WAITV, box64_futex_waitv)         \
ENTRYSTRING_(BOX64_BASH, bash)                          \
ENTRYINT(BOX64_JITGDB, jit_gdb, 0, 3, 2)                \
ENTRYBOOL(BOX64_NOSANDBOX, box64_nosandbox)             \
ENTRYBOOL(BOX64_INPROCESSGPU, box64_inprocessgpu)       \
ENTRYBOOL(BOX64_CEFDISABLEGPU, box64_cefdisablegpu)     \
ENTRYBOOL(BOX64_CEFDISABLEGPUCOMPOSITOR, box64_cefdisablegpucompositor)\
ENTRYBOOL(BOX64_EXIT, want_exit)                        \
ENTRYBOOL(BOX64_LIBCEF, box64_libcef)                   \
ENTRYBOOL(BOX64_JVM, box64_jvm)                         \
ENTRYBOOL(BOX64_SDL2_JGUID, box64_sdl2_jguid)           \
ENTRYINT(BOX64_MALLOC_HACK, box64_malloc_hack, 0, 2, 2) \
ENTRYINTPOS(BOX64_MAXCPU, new_maxcpu)                   \
ENTRYSTRING_(BOX64_ADDLIBS, new_addlibs)                \
ENTRYSTRING_(BOX64_ENV, new_env)                        \
ENTRYSTRING_(BOX64_ENV1, new_env1)                      \
ENTRYSTRING_(BOX64_ENV2, new_env2)                      \
ENTRYSTRING_(BOX64_ENV3, new_env3)                      \
ENTRYSTRING_(BOX64_ENV4, new_env4)                      \

#ifdef HAVE_TRACE
#define SUPER2()                                        \
ENTRYSTRING_(BOX64_TRACE, trace)                        \
ENTRYULONG(BOX64_TRACE_START, start_cnt)                \
ENTRYSTRING_(BOX64_TRACE_INIT, trace_init)              \
ENTRYBOOL(BOX64_TRACE_XMM, trace_xmm)                   \
ENTRYBOOL(BOX64_TRACE_EMM, trace_emm)                   \
ENTRYBOOL(BOX64_TRACE_COLOR, trace_regsdiff)            \

#else
#define SUPER2()                                        \
IGNORE(BOX64_TRACE)                                     \
IGNORE(BOX64_TRACE_START)                               \
IGNORE(BOX64_TRACE_INIT)                                \
IGNORE(BOX64_TRACE_XMM)                                 \
IGNORE(BOX64_TRACE_EMM)                                 \
IGNORE(BOX64_TRACE_COLOR)                               \

#endif

#ifdef DYNAREC
#define SUPER3()                                                    \
ENTRYBOOL(BOX64_DYNAREC, box64_dynarec)                             \
ENTRYINT(BOX64_DYNAREC_DUMP, box64_dynarec_dump, 0, 2, 2)           \
ENTRYINT(BOX64_DYNAREC_LOG, box64_dynarec_log, 0, 3, 2)             \
ENTRYINT(BOX64_DYNAREC_BIGBLOCK, box64_dynarec_bigblock, 0, 3, 2)   \
ENTRYSTRING_(BOX64_DYNAREC_FORWARD, box64_dynarec_forward)          \
ENTRYINT(BOX64_DYNAREC_STRONGMEM, box64_dynarec_strongmem, 0, 4, 3) \
ENTRYBOOL(BOX64_DYNAREC_X87DOUBLE, box64_dynarec_x87double)         \
ENTRYBOOL(BOX64_DYNAREC_DIV0, box64_dynarec_div0)                   \
ENTRYBOOL(BOX64_DYNAREC_FASTNAN, box64_dynarec_fastnan)             \
ENTRYBOOL(BOX64_DYNAREC_FASTROUND, box64_dynarec_fastround)         \
ENTRYINT(BOX64_DYNAREC_SAFEFLAGS, box64_dynarec_safeflags, 0, 2, 2) \
ENTRYBOOL(BOX64_DYNAREC_CALLRET, box64_dynarec_callret)             \
ENTRYBOOL(BOX64_DYNAREC_BLEEDING_EDGE, box64_dynarec_bleeding_edge) \
ENTRYBOOL(BOX64_DYNAREC_JVM, box64_dynarec_jvm)                     \
ENTRYBOOL(BOX64_DYNAREC_TBB, box64_dynarec_tbb)                     \
IGNORE(BOX64_DYNAREC_HOTPAGE)                                       \
IGNORE(BOX64_DYNAREC_FASTPAGE)                                      \
ENTRYBOOL(BOX64_DYNAREC_ALIGNED_ATOMICS, box64_dynarec_aligned_atomics) \
ENTRYBOOL(BOX64_DYNAREC_WAIT, box64_dynarec_wait)                   \
ENTRYSTRING_(BOX64_NODYNAREC, box64_nodynarec)                      \
ENTRYBOOL(BOX64_DYNAREC_TEST, box64_dynarec_test)                   \
ENTRYBOOL(BOX64_DYNAREC_MISSING, box64_dynarec_missing)             \

#else
#define SUPER3()                                                    \
IGNORE(BOX64_DYNAREC)                                               \
IGNORE(BOX64_DYNAREC_DUMP)                                          \
IGNORE(BOX64_DYNAREC_LOG)                                           \
IGNORE(BOX64_DYNAREC_BIGBLOCK)                                      \
IGNORE(BOX64_DYNAREC_FORWARD)                                       \
IGNORE(BOX64_DYNAREC_STRONGMEM)                                     \
IGNORE(BOX64_DYNAREC_X87DOUBLE)                                     \
IGNORE(BOX64_DYNAREC_DIV0)                                          \
IGNORE(BOX64_DYNAREC_FASTNAN)                                       \
IGNORE(BOX64_DYNAREC_FASTROUND)                                     \
IGNORE(BOX64_DYNAREC_SAFEFLAGS)                                     \
IGNORE(BOX64_DYNAREC_CALLRET)                                       \
IGNORE(BOX64_DYNAREC_BLEEDING_EDGE)                                 \
IGNORE(BOX64_DYNAREC_JVM)                                           \
IGNORE(BOX64_DYNAREC_TBB)                                           \
IGNORE(BOX64_DYNAREC_HOTPAGE)                                       \
IGNORE(BOX64_DYNAREC_FASTPAGE)                                      \
IGNORE(BOX64_DYNAREC_ALIGNED_ATOMICS)                               \
IGNORE(BOX64_DYNAREC_WAIT)                                          \
IGNORE(BOX64_NODYNAREC)                                             \
IGNORE(BOX64_DYNAREC_TEST)                                          \
IGNORE(BOX64_DYNAREC_MISSING)                                       \

#endif

#if defined(HAVE_TRACE) && defined(DYNAREC)
#define SUPER4()                                                    \
ENTRYBOOL(BOX64_DYNAREC_TRACE, box64_dynarec_trace)                 \

#else
#define SUPER4()                                                    \
IGNORE(BOX64_DYNAREC_TRACE)                                         \

#endif

#define SUPER() \
SUPER1()        \
SUPER2()        \
SUPER3()        \
SUPER4()

typedef struct my_params_s {
// is present part
#define ENTRYBOOL(NAME, name) uint8_t is_##name##_present:1;
#define CENTRYBOOL(NAME, name) uint8_t is_##name##_present:1;
#define ENTRYINT(NAME, name, minval, maxval, bits) uint8_t is_##name##_present:1;
#define ENTRYINTPOS(NAME, name) uint8_t is_##name##_present:1;
#define ENTRYSTRING(NAME, name) uint8_t is_##name##_present:1;
#define ENTRYSTRING_(NAME, name) uint8_t is_##name##_present:1;
#define ENTRYDSTRING(NAME, name) uint8_t is_##name##_present:1;
#define ENTRYADDR(NAME, name) uint8_t is_##name##_present:1;
#define ENTRYULONG(NAME, name) uint8_t is_##name##_present:1;
#define IGNORE(NAME) 
SUPER()
// done
#undef ENTRYBOOL
#undef CENTRYBOOL
#undef ENTRYINT
#undef ENTRYINTPOS
#undef ENTRYSTRING
#undef ENTRYSTRING_
#undef ENTRYDSTRING
#undef ENTRYADDR
#undef ENTRYULONG
// the actual fields, in two steps to regroup bit fields together
#define ENTRYBOOL(NAME, name) uint8_t name:1;
#define CENTRYBOOL(NAME, name) uint8_t name:1;
#define ENTRYINT(NAME, name, minval, maxval, bits) uint8_t name:bits;
#define ENTRYINTPOS(NAME, name)
#define ENTRYSTRING(NAME, name)
#define ENTRYSTRING_(NAME, name)
#define ENTRYDSTRING(NAME, name)
#define ENTRYADDR(NAME, name)
#define ENTRYULONG(NAME, name)
SUPER()
// done
#undef ENTRYBOOL
#undef CENTRYBOOL
#undef ENTRYINT
#undef ENTRYINTPOS
#undef ENTRYSTRING
#undef ENTRYSTRING_
#undef ENTRYDSTRING
#undef ENTRYADDR
#undef ENTRYULONG
#define ENTRYBOOL(NAME, name)
#define CENTRYBOOL(NAME, name)
#define ENTRYINT(NAME, name, minval, maxval, bits)
#define ENTRYINTPOS(NAME, name) uint32_t name;
#define ENTRYSTRING(NAME, name) char* name;
#define ENTRYSTRING_(NAME, name) char* name;
#define ENTRYDSTRING(NAME, name) char* name;
#define ENTRYADDR(NAME, name) uintptr_t name;
#define ENTRYULONG(NAME, name) uint64_t name;
SUPER()
// done
#undef ENTRYBOOL
#undef CENTRYBOOL
#undef ENTRYINT
#undef ENTRYINTPOS
#undef ENTRYSTRING
#undef ENTRYSTRING_
#undef ENTRYDSTRING
#undef ENTRYADDR
#undef ENTRYULONG
} my_params_t;

KHASH_MAP_INIT_STR(params, my_params_t)

static kh_params_t *params = NULL;

static void clearParam(my_params_t* param)
{
    #define ENTRYBOOL(NAME, name) 
    #define CENTRYBOOL(NAME, name) 
    #define ENTRYINT(NAME, name, minval, maxval, bits) 
    #define ENTRYINTPOS(NAME, name) 
    #define ENTRYSTRING(NAME, name) free(param->name); 
    #define ENTRYSTRING_(NAME, name) free(param->name); 
    #define ENTRYDSTRING(NAME, name) free(param->name); 
    #define ENTRYADDR(NAME, name) 
    #define ENTRYULONG(NAME, name) 
    SUPER()
    #undef ENTRYBOOL
    #undef CENTRYBOOL
    #undef ENTRYINT
    #undef ENTRYINTPOS
    #undef ENTRYSTRING
    #undef ENTRYSTRING_
    #undef ENTRYDSTRING
    #undef ENTRYADDR
    #undef ENTRYULONG
}

static void addParam(const char* name, my_params_t* param)
{
    khint_t k;
    k = kh_get(params, params, name);
    if(k==kh_end(params)) {
        int ret;
        k = kh_put(params, params, box_strdup(name), &ret);
    } else {
        clearParam(&kh_value(params, k));
    }
    my_params_t *p = &kh_value(params, k);
    memcpy(p, param, sizeof(my_params_t));
}

static void trimString(char* s)
{
    if(!s)
        return;
    // trim right space/tab
    size_t len = strlen(s);
    while(len && (s[len-1]==' ' || s[len-1]=='\t' || s[len-1]=='\n'))
        s[--len] = '\0';
    // trim left space/tab
    while(s[0]==' ' || s[0]=='\t')
        memmove(s, s+1, strlen(s));
}

#ifdef ANDROID
static int shm_open(const char *name, int oflag, mode_t mode) {
    return -1;
}
static int shm_unlink(const char *name) {
    return -1;
}
#endif

void LoadRCFile(const char* filename)
{
    FILE *f = NULL;
    if(filename)
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
    if(!f) {
        printf_log(LOG_INFO, "Cannot open RC file %s\n", filename);
        return;
    }
    // init the hash table if needed
    if(!params)
        params = kh_init(params);
    // prepare to parse the file
    char* line = NULL;
    size_t lsize = 0;
    my_params_t current_param = {0};
    char* current_name = NULL;
    int dummy;
    size_t len;
    char* p;
    // parsing
    while ((dummy = getline(&line, &lsize, f)) != -1) {
        // remove comments
        if((p=strchr(line, '#')))
            *p = '\0';
        trimString(line);
        len = strlen(line);
        // check the line content
        if(line[0]=='[' && strchr(line, ']')) {
            // new entry, will need to add current one
            if(current_name)
                addParam(current_name, &current_param);
            // prepare a new entry
            memset(&current_param, 0, sizeof(current_param));
            free(current_name);
            current_name = LowerCase(line+1);
            *strchr(current_name, ']') = '\0';
            trimString(current_name);
        } else if(strchr(line, '=')) {
            // actual parameters
            //get the key and val
            char* key = line;
            char* val = strchr(key, '=')+1;
            *strchr(key, '=') = '\0';
            trimString(key);
            trimString(val);
            // extract, check and set arg
            #define ENTRYINT(NAME, name, minval, maxval, bits)          \
                else if(!strcmp(key, #NAME)) {                          \
                    int v = strtol(val, &p, 0);                         \
                    if(p!=val && v>=minval && v<=maxval) {              \
                        current_param.is_##name##_present = 1;          \
                        current_param.name = v;                         \
                    }                                                   \
                }
            #define ENTRYBOOL(NAME, name) ENTRYINT(NAME, name, 0, 1, 1)
            #define CENTRYBOOL(NAME, name) ENTRYBOOL(NAME, name)
            #define ENTRYINTPOS(NAME, name)                             \
                else if(!strcmp(key, #NAME)) {                          \
                    int v = strtol(val, &p, 0);                         \
                    if(p!=val) {                                        \
                        current_param.is_##name##_present = 1;          \
                        current_param.name = v;                         \
                    }                                                   \
                }
            #define ENTRYSTRING(NAME, name)                             \
                else if(!strcmp(key, #NAME)) {                          \
                    current_param.is_##name##_present = 1;              \
                    if(current_param.name) free(current_param.name);    \
                    current_param.name = box_strdup(val);               \
                }
            #define ENTRYSTRING_(NAME, name) ENTRYSTRING(NAME, name)
            #define ENTRYDSTRING(NAME, name) ENTRYSTRING(NAME, name)
            #define ENTRYADDR(NAME, name)                               \
                else if(!strcmp(key, #NAME)) {                          \
                    uintptr_t v = strtoul(val, &p, 0);                  \
                    if(p!=val) {                                        \
                        current_param.is_##name##_present = 1;          \
                        current_param.name = v;                         \
                    }                                                   \
                }
            #define ENTRYULONG(NAME, name)                              \
                else if(!strcmp(key, #NAME)) {                          \
                    uint64_t v = strtoull(val, &p, 0);                  \
                    if(p!=val) {                                        \
                        current_param.is_##name##_present = 1;          \
                        current_param.name = v;                         \
                    }                                                   \
                }
            #undef IGNORE
            #define IGNORE(NAME) else if(!strcmp(key, #NAME)) ;
            if(0) ;
            SUPER()
            else if(len && current_name) {
                printf_log(LOG_INFO, "Warning, unsupported %s=%s for [%s] in %s\n", key, val, current_name, filename);
            }
            #undef ENTRYBOOL
            #undef CENTRYBOOL
            #undef ENTRYINT
            #undef ENTRYINTPOS
            #undef ENTRYSTRING
            #undef ENTRYSTRING_
            #undef ENTRYDSTRING
            #undef ENTRYADDR
            #undef ENTRYULONG
            #undef IGNORE
            #define IGNORE(NAME) 
        }
    }
    // last entry to be pushed too
    if(current_name) {
        addParam(current_name, &current_param);
        free(current_name);
    }
    free(line);
    fclose(f);
    printf_log(LOG_INFO, "Params database has %d entries\n", kh_size(params));
}

void DeleteParams()
{
    if(!params)
        return;
    
    // free strings
    my_params_t* p;
    // need to free duplicated strings
    kh_foreach_value_ref(params, p, clearParam(p));
    const char* key;
    kh_foreach_key(params, key, free((void*)key));
    // free the hash itself
    kh_destroy(params, params);
    params = NULL;
}

extern int ftrace_has_pid;
extern FILE* ftrace;
extern char* ftrace_name;
void openFTrace(const char* newtrace);
void addNewEnvVar(const char* s);
void AddNewLibs(const char* libs);
#ifdef DYNAREC
void GatherDynarecExtensions();
#endif
#ifdef HAVE_TRACE
void setupTraceInit();
void setupTrace();
#endif
static char old_name[256] = "";
const char* GetLastApplyName()
{
    return old_name;
}
void ApplyParams(const char* name)
{
    if(!name || !params)
        return;
    int new_cycle_log = cycle_log;
    int new_maxcpu = box64_maxcpu;
    int box64_dynarec_jvm = box64_jvm;
    if(!strcmp(name, old_name)) {
        return;
    }
    strncpy(old_name, name, 255);
    khint_t k;
    {
        char* lname = LowerCase(name);
        k = kh_get(params, params, lname);
        free(lname);
    }
    if(k == kh_end(params))
        return;
    my_params_t* param = &kh_value(params, k);
    int want_exit = 0;
    #ifdef DYNAREC
    int olddynarec = box64_dynarec;
    #endif
    printf_log(LOG_INFO, "Apply RC params for %s\n", name);
    #define ENTRYINT(NAME, name, minval, maxval, bits) if(param->is_##name##_present) {name = param->name; printf_log(LOG_INFO, "Applying %s=%d\n", #NAME, param->name);}
    #define ENTRYBOOL(NAME, name) ENTRYINT(NAME, name, 0, 1, 1)
    #define CENTRYBOOL(NAME, name) if(param->is_##name##_present) {my_context->name = param->name; printf_log(LOG_INFO, "Applying %s=%d\n", #NAME, param->name);}
    #define ENTRYINTPOS(NAME, name) if(param->is_##name##_present) {name = param->name; printf_log(LOG_INFO, "Applying %s=%d\n", #NAME, param->name);}
    #define ENTRYSTRING(NAME, name) if(param->is_##name##_present) {name = param->name; printf_log(LOG_INFO, "Applying %s=%s\n", #NAME, param->name);}
    #define ENTRYSTRING_(NAME, name)  
    #define ENTRYDSTRING(NAME, name) if(param->is_##name##_present) {if(name) free(name); name = box_strdup(param->name); printf_log(LOG_INFO, "Applying %s=%s\n", #NAME, param->name);}
    #define ENTRYADDR(NAME, name) if(param->is_##name##_present) {name = param->name; printf_log(LOG_INFO, "Applying %s=%zd\n", #NAME, param->name);}
    #define ENTRYULONG(NAME, name) if(param->is_##name##_present) {name = param->name; printf_log(LOG_INFO, "Applying %s=%lld\n", #NAME, param->name);}
    SUPER()
    #undef ENTRYBOOL
    #undef CENTRYBOOL
    #undef ENTRYINT
    #undef ENTRYINTPOS
    #undef ENTRYSTRING
    #undef ENTRYSTRING_
    #undef ENTRYDSTRING
    #undef ENTRYADDR
    #undef ENTRYULONG
    // now handle the manuel entry (the one with ending underscore)
    if(want_exit)
        exit(0);
    if(new_cycle_log==1)
        new_cycle_log = 16;
    if(new_cycle_log!=cycle_log) {
        freeCycleLog(my_context);
        cycle_log = new_cycle_log;
        initCycleLog(my_context);
    }
    #ifdef DYNAREC
    if(param->is_box64_dynarec_jvm_present && !param->is_box64_jvm_present)
        box64_jvm = box64_dynarec_jvm;
    #endif
    if(!box64_maxcpu_immutable) {
        if(new_maxcpu!=box64_maxcpu && box64_maxcpu && box64_maxcpu<new_maxcpu) {
        printf_log(LOG_INFO, "Not applying BOX64_MAXCPU=%d because a lesser value is already active: %d\n", new_maxcpu, box64_maxcpu);
        } else
            box64_maxcpu = new_maxcpu;
    } else if(new_maxcpu!=box64_maxcpu)
        printf_log(LOG_INFO, "Not applying BOX64_MAXCPU=%d because it's too late\n", new_maxcpu);
    if(param->is_ld_library_path_present) AppendList(&my_context->box64_ld_lib, param->ld_library_path, 1);
    if(param->is_box64_path_present) AppendList(&my_context->box64_path, param->box64_path, 1);
    if(param->is_trace_file_present) {
        // open a new ftrace...
        printf_log(LOG_INFO, "Applying %s=%s\n", "BOX64_TRACE_FILE", param->trace_file);
        if(ftrace_name) {
            fclose(ftrace);
        }
        openFTrace(param->trace_file);
    }
    if(param->is_emulated_libs_present) {
        AppendList(&my_context->box64_emulated_libs, param->emulated_libs, 0);
        printf_log(LOG_INFO, "Applying %s=%s\n", "BOX64_EMULATED_LIBS", param->emulated_libs);
    }
    if(param->is_new_addlibs_present) {
        AddNewLibs(param->new_addlibs);
    }
    if(param->is_new_env_present) {
        addNewEnvVar(param->new_env);
        printf_log(LOG_INFO, "Applying %s=%s\n", "BOX64_ENV", param->new_env);
    }
    if(param->is_new_env1_present) {
        addNewEnvVar(param->new_env1);
        printf_log(LOG_INFO, "Applying %s=%s\n", "BOX64_ENV1", param->new_env1);
    }
    if(param->is_new_env2_present) {
        addNewEnvVar(param->new_env2);
        printf_log(LOG_INFO, "Applying %s=%s\n", "BOX64_ENV2", param->new_env2);
    }
    if(param->is_new_env3_present) {
        addNewEnvVar(param->new_env3);
        printf_log(LOG_INFO, "Applying %s=%s\n", "BOX64_ENV3", param->new_env3);
    }
    if(param->is_new_env4_present) {
        addNewEnvVar(param->new_env4);
        printf_log(LOG_INFO, "Applying %s=%s\n", "BOX64_ENV4", param->new_env4);
    }
    if(param->is_bash_present && FileIsX64ELF(param->bash)) {
        if(my_context->bashpath)
            free(my_context->bashpath);
        my_context->bashpath = box_strdup(param->bash);
        printf_log(LOG_INFO, "Applying %s=%s\n", "BOX64_BASH", param->bash);
    }
    #ifdef HAVE_TRACE
    int old_x64trace = my_context->x64trace;
    if(param->is_trace_present) {
        char*p = param->trace;
        if (strcmp(p, "0")) {
            my_context->x64trace = 1;
            box64_trace = p;
        }
        printf_log(LOG_INFO, "Applying %s=%s\n", "BOX64_TRACE", param->trace);
    }
    if(param->is_trace_init_present) {
        char* p = param->trace_init;
        if (strcmp(p, "0")) {
            my_context->x64trace = 1;
            trace_init = p;
        }
        printf_log(LOG_INFO, "Applying %s=%s\n", "BOX64_TRACE_INIT", param->trace_init);
    }
    if(my_context->x64trace && !old_x64trace) {
        printf_log(LOG_INFO, "Initializing Zydis lib\n");
        if(InitX64Trace(my_context)) {
            printf_log(LOG_INFO, "Zydis init failed, no x86 trace activated\n");
            my_context->x64trace = 0;
        }
    }
    if(param->is_trace_init_present)
        setupTraceInit();
    if(param->is_trace_present)
        setupTrace();
    #endif
    #ifdef DYNAREC
    if(param->is_box64_nodynarec_present) {
        uintptr_t no_start = 0, no_end = 0;
        char* p;
        no_start = strtoul(param->box64_nodynarec, &p, 0);
        if(p!=param->box64_nodynarec && p[0]=='-') {
            char* p2;
            ++p;
            no_end = strtoul(p, &p2, 0);
            if(p2!=p && no_end>no_start) {
                box64_nodynarec_start = no_start;
                box64_nodynarec_end = no_end;
                printf_log(LOG_INFO, "Appling BOX64_NODYNAREC=%p-%p\n", (void*)box64_nodynarec_start, (void*)box64_nodynarec_end);
            }
        }
    }
    if(param->is_box64_dynarec_forward_present) {
        int forward = 0;
        if(sscanf(param->box64_dynarec_forward, "%d", &forward)==1) {
            box64_dynarec_forward = forward;
            printf_log(LOG_INFO, "Appling BOX64_DYNAREC_FORWARD=%d\n", box64_dynarec_forward);
        }
    }
    if(!olddynarec && box64_dynarec)
        GatherDynarecExtensions();
    if(param->is_box64_dynarec_test_present && box64_dynarec_test) {
        box64_dynarec_fastnan = 0;
        box64_dynarec_fastround = 0;
        box64_dynarec_x87double = 1;
        box64_dynarec_div0 = 1;
        box64_dynarec_callret = 0;
    }
    #endif
    if(box64_log==3) {
        box64_log = 2;
        box64_dump = 1;
    }
}
