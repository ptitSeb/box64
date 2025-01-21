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
#define SUPER()                                        \
ENTRYSTRING_(BOX64_ARGS, new_args)                      \

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
static kh_params_t *params_gen = NULL;

static void clearParam(my_params_t* param)
{
    #define ENTRYBOOL(NAME, name)
    #define CENTRYBOOL(NAME, name)
    #define ENTRYINT(NAME, name, minval, maxval, bits)
    #define ENTRYINTPOS(NAME, name)
    #define ENTRYSTRING(NAME, name) box_free(param->name);
    #define ENTRYSTRING_(NAME, name) box_free(param->name);
    #define ENTRYDSTRING(NAME, name) box_free(param->name);
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

static void addParam(const char* name, my_params_t* param, int gen)
{
    khint_t k;
    kh_params_t* khp = gen?params_gen:params;
    k = kh_get(params, khp, name);
    if(k==kh_end(khp)) {
        int ret;
        k = kh_put(params, khp, box_strdup(name), &ret);
    } else {
        clearParam(&kh_value(khp, k));
    }
    my_params_t *p = &kh_value(khp, k);
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
    if(!params_gen)
        params_gen = kh_init(params);
    // prepare to parse the file
    char* line = NULL;
    size_t lsize = 0;
    my_params_t current_param = {0};
    char* current_name = NULL;
    int dummy;
    size_t len;
    char* p;
    int decor = 1;
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
                addParam(current_name, &current_param, (decor==2));
            if(line[1]=='*' && line[(intptr_t)(strchr(line, ']')-line)-1]=='*')
                decor = 2;
            else
                decor = 1;
            // prepare a new entry
            memset(&current_param, 0, sizeof(current_param));
            box_free(current_name);
            current_name = LowerCase(line+decor);
            *(strchr(current_name, ']')+1-decor) = '\0';
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
                    if(current_param.name) box_free(current_param.name);\
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
        addParam(current_name, &current_param, (decor==2));
        box_free(current_name);
    }
    box_free(line);
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
    kh_foreach_key(params, key, box_free((void*)key));
    // free the hash itself
    kh_destroy(params, params);
    params = NULL;
}

extern int ftrace_has_pid;
extern FILE* ftrace;
extern char* ftrace_name;
void openFTrace(const char* newtrace, int reopen);
void addNewEnvVar(const char* s);
void AddNewLibs(const char* libs);
void my_reserveHighMem();
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
void internal_ApplyParams(const char* name, const my_params_t* param);
void ApplyParams(const char* name)
{
    if(!name || !params)
        return;
    if(!strcasecmp(name, old_name)) {
        return;
    }
    strncpy(old_name, name, 255);
    khint_t k1;
    {
        char* lname = LowerCase(name);
        k1 = kh_get(params, params, lname);
        my_params_t* param;
        const char* k2;
        kh_foreach_ref(params_gen, k2, param,
            if(strstr(lname, k2))
                internal_ApplyParams(name, param);
        )
        box_free(lname);
    }
    if(k1 == kh_end(params))
        return;
    my_params_t* param = &kh_value(params, k1);
    internal_ApplyParams(name, param);
}

void internal_ApplyParams(const char* name, const my_params_t* param) {
    #ifdef DYNAREC
    int olddynarec = BOX64ENV(dynarec);
    #endif
    printf_log(LOG_INFO, "Apply RC params for %s\n", name);
    #define ENTRYINT(NAME, name, minval, maxval, bits) if(param->is_##name##_present) {printf_log(LOG_INFO, "Applying %s=%d\n", #NAME, param->name); name = param->name;}
    #define ENTRYBOOL(NAME, name) ENTRYINT(NAME, name, 0, 1, 1)
    #define CENTRYBOOL(NAME, name) if(param->is_##name##_present) {printf_log(LOG_INFO, "Applying %s=%d\n", #NAME, param->name); my_context->name = param->name;}
    #define ENTRYINTPOS(NAME, name) if(param->is_##name##_present) {printf_log(LOG_INFO, "Applying %s=%d\n", #NAME, param->name); name = param->name;}
    #define ENTRYSTRING(NAME, name) if(param->is_##name##_present) {printf_log(LOG_INFO, "Applying %s=%s\n", #NAME, param->name); name = param->name;}
    #define ENTRYSTRING_(NAME, name)
    #define ENTRYDSTRING(NAME, name) if(param->is_##name##_present) {printf_log(LOG_INFO, "Applying %s=%s\n", #NAME, param->name); if(name) box_free(name); name = box_strdup(param->name);}
    #define ENTRYADDR(NAME, name) if(param->is_##name##_present) {printf_log(LOG_INFO, "Applying %s=%zd\n", #NAME, param->name); name = param->name;}
    #define ENTRYULONG(NAME, name) if(param->is_##name##_present) {printf_log(LOG_INFO, "Applying %s=%lld\n", #NAME, param->name); name = param->name;}
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
    #ifdef DYNAREC
    if(!olddynarec && BOX64ENV(dynarec))
        GatherDynarecExtensions();
    #endif
}
