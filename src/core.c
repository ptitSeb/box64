#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <ctype.h>
#ifdef BOX32
#include <sys/personality.h>
#endif
#ifdef DYNAREC
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include "os.h"
#include "build_info.h"
#include "debug.h"
#include "fileutils.h"
#include "box64context.h"
#include "box64cpu.h"
#include "box64cpu_util.h"
#include "wine_tools.h"
#include "elfloader.h"
#include "custommem.h"
#include "box64stack.h"
#include "auxval.h"
#include "threads.h"
#include "x64trace.h"
#include "librarian.h"
#include "symbols.h"
#include "emu/x64run_private.h"
#include "elfs/elfloader_private.h"
#include "x64emu.h"
#include "library.h"
#include "core.h"
#include "env.h"
#include "cleanup.h"
#include "freq.h"
#include "hostext.h"

box64context_t *my_context = NULL;
extern box64env_t box64env;

int box64_quit = 0;
int box64_exit_code = 0;
int box64_stdout_no_w = 0;
uintptr_t box64_pagesize;
path_collection_t box64_addlibs = {0};
int box64_is32bits = 0;
int box64_isAddressSpace32 = 0;
int box64_rdtsc = 0;
uint8_t box64_rdtsc_shift = 0;
int box64_mapclean = 0;
int box64_zoom = 0;
int box64_steam = 0;
int box64_steamcmd = 0;
int box64_musl = 0;
int box64_nolibs = 0;
char* box64_custom_gstreamer = NULL;
int box64_tcmalloc_minimal = 0;
uintptr_t fmod_smc_start = 0;
uintptr_t fmod_smc_end = 0;
uint32_t default_gs = 0x53;
uint32_t default_fs = 0;
int box64_isglibc234 = 0;
int box64_unittest_mode = 0;

#ifdef DYNAREC
cpu_ext_t cpuext = {0};
#endif

int box64_wine = 0;
const char* box64_guest_name = NULL;
const char* box64_wine_guest_name = NULL;

#ifdef HAVE_TRACE
uintptr_t trace_start = 0, trace_end = 0;
char* trace_func = NULL;
#endif

FILE* ftrace = NULL;
char* ftrace_name = NULL;
int ftrace_opened = 0;


static void openFTrace(void)
{
    const char* p = BOX64ENV(trace_file);
    #ifndef MAX_PATH
    #define MAX_PATH 4096
    #endif
    char tmp[MAX_PATH];
    char tmp2[MAX_PATH];
    int append = 0;

    if (ftrace_name) box_free(ftrace_name);
    ftrace_name = NULL;

    if (p && strlen(p) && p[strlen(p) - 1] == '+') {
        strncpy(tmp2, p, sizeof(tmp2));
        tmp2[strlen(p)-1]='\0';
        p = tmp2;
        append = 1;
    }

    if (!p || ftrace_opened) return;
    ftrace_opened = 1;

    if (strstr(p, "\%pid")) {
        int next = 0;
        do {
            strcpy(tmp, p);
            char* c = strstr(tmp, "%pid");
            *c = 0; // cut
            char pid[16];
            if (next)
                sprintf(pid, "%d-%d", GetTID(), next);
            else
                sprintf(pid, "%d", GetTID());
            strcat(tmp, pid);
            c = strstr(p, "\%pid") + strlen("\%pid");
            strcat(tmp, c);
            ++next;
        } while (FileExist(tmp, IS_FILE) && !append);
        p = tmp;
    }

    if (!strcmp(p, "stdout"))
        ftrace = stdout;
    else if (!strcmp(p, "stderr"))
        ftrace = stderr;
    else {
        if (append)
            ftrace = fopen(p, "a");
        else
            ftrace = fopen(p, "w");
        if (!ftrace) {
            ftrace = stderr;
            printf_log(LOG_INFO, "Cannot open trace file \"%s\" for writing (error=%s), fallback to stderr\n", p, strerror(errno));
        } else {
            ftrace_name = box_strdup(p);
            if (!BOX64ENV(nobanner)) {
                fprintf(stderr, "[BOX64] Trace %s to \"%s\" (set BOX64_NOBANNER=1 to suppress this log)\n", append ? "appended" : "redirected", p);
            }
            PrintBox64Version(0);
        }
    }
}

const char* getCpuName();
int getNCpuUnmasked();

void computeRDTSC()
{
    int hardware  = 0;
    box64_rdtsc_shift = 0;
    #if defined(ARM64) || defined(RV64) || defined(LA64)
    hardware = 1;
    box64_rdtsc = 0;    // allow hardware counter
    #else
    box64_rdtsc = 1;
    printf_log(LOG_INFO, "Will use time-based emulation for RDTSC, even if hardware counters are available\n");
    #endif
    uint64_t freq = ReadTSCFrequency(NULL);
    if(freq<((BOX64ENV(rdtsc_1ghz))?1000000000LL:1000000)) {
        box64_rdtsc = 1;
        if(hardware) printf_log(LOG_INFO, "Hardware counter is too slow (%d kHz), not using it\n", freq/1000);
        hardware = 0;
        freq = ReadTSCFrequency(NULL);
    }
    uint64_t efreq = freq;
    while(efreq<2000000000 && box64_rdtsc_shift<31) {    // minimum 2GHz, but not too much shift
        ++box64_rdtsc_shift;
        efreq = freq<<box64_rdtsc_shift;
    }
    printf_log(LOG_INFO, "Will use %s counter measured at ", box64_rdtsc?"software":"hardware");
    int ghz = freq>=1000000000LL;
    if(ghz) freq/=100000000LL; else freq/=100000;
    if (ghz) printf_log_prefix(0, LOG_INFO, "%d.%d GHz", freq / 10, freq % 10);
    if (!ghz && (freq >= 1000)) printf_log_prefix(0, LOG_INFO, "%d MHz", freq / 10);
    if (!ghz && (freq < 1000)) printf_log_prefix(0, LOG_INFO, "%d.%d MHz", freq / 10, freq % 10);
    if(box64_rdtsc_shift) {
        printf_log_prefix(0, LOG_INFO, " emulating ");
        ghz = efreq>=1000000000LL;
        if(ghz) efreq/=100000000LL; else efreq/=100000;
        if (ghz) printf_log_prefix(0, LOG_INFO, "%d.%d GHz", efreq / 10, efreq % 10);
        if (!ghz && (efreq >= 1000)) printf_log_prefix(0, LOG_INFO, "%d MHz", efreq / 10);
        if (!ghz && (efreq < 1000)) printf_log_prefix(0, LOG_INFO, "%d.%d MHz", efreq / 10, efreq % 10);
    }
    printf_log_prefix(0, LOG_INFO, "\n");
}

static void displayMiscInfo()
{
    openFTrace();

    if ((BOX64ENV(nobanner) || BOX64ENV(log)) && ftrace == stdout)
        box64_stdout_no_w = 1;

#if !defined(DYNAREC) && (defined(ARM64) || defined(RV64) || defined(LA64))
    printf_log(LOG_INFO, "Warning: DynaRec is available on this host architecture, an interpreter-only build is probably not intended.\n");
#endif

    char* p;

#ifdef DYNAREC
    if (DetectHostCpuFeatures())
        PrintHostCpuFeatures();
    else {
        printf_log(LOG_INFO, "Minimum CPU requirements not met, disabling DynaRec\n");
        SET_BOX64ENV(dynarec, 0);
    }
#endif

    // grab ncpu and cpu name
    int ncpu = getNCpuUnmasked();
    const char* cpuname = getCpuName();

    printf_log(LOG_INFO, "Running on %s with %d core%s, pagesize: %zd\n", cpuname, ncpu, ncpu > 1 ? "s" : "", box64_pagesize);

    // grab and calibrate hardware counter
    computeRDTSC();
}

static void hookMangoHud()
{
    const char* config = getenv("MANGOHUD_CONFIG");
    const char* configfile = getenv("MANGOHUD_CONFIGFILE");
    if (config || configfile) return;
    if (FileExist("/etc/MangoHud.conf", IS_FILE)) return;
    const char* configdir = getenv("XDG_CONFIG_HOME");
    const char* homedir = getenv("HOME");
#ifndef ANDROID
    homedir = homedir ? homedir : getpwuid(getuid())->pw_dir;
#endif
    if (!homedir) return;

    static char config_base[256];
    snprintf(config_base, sizeof(config_base), "%s%s",
        configdir ? configdir : homedir,
        configdir ? "" : "/.config");

    const char* files[] = { "MangoHud.conf", "box64.conf" };
    for (int i = 0; i < 2; i++) {
        static char path[512];
        snprintf(path, sizeof(path), "%s/MangoHud/%s", config_base, files[i]);
        if (FileExist(path, IS_FILE)) return;
    }
    setenv("MANGOHUD_CONFIG", "legacy_layout=0,custom_text_center=" BOX64_BUILD_INFO_STRING ",gpu_stats=1,cpu_stats=1,fps=1,frame_timing=1", 0);
}

static void loadPath(path_collection_t *col, const char* defpath, const char* path)
{
    if(path) {
        ParseList(path, col, 1);
    } else {
        ParseList(defpath, col, 1);
    }
}

void PrintCollection(path_collection_t* col, const char* env)
{
    if (LOG_INFO<=BOX64ENV(log)) {
        printf_log(LOG_INFO, "%s: ", env);
        for(int i=0; i<col->size; i++)
            printf_log_prefix(0, LOG_INFO, "%s%s", col->paths[i], (i==col->size-1)?"":":");
        printf_log_prefix(0, LOG_INFO, "\n");
    }
}

EXPORTDYN
int CountEnv(char** env)
{
    char** p = env;
    int c = 0;
    while(*p) {
        ++c;
        ++p;
    }
    return c;
}
EXPORTDYN
int GatherEnv(char*** dest, char** env, char* prog)
{
    char** p = env;
    int idx = 0;
    int _prog = 0;
    while(*p) {
        if(strncmp(*p, "_=", 2)==0) {
            _prog = 1;
        } else {
            (*dest)[idx++] = box_strdup(*p);
        }
        ++p;
    }
    // add "_=prog" at the end...
    if(_prog && prog) {
        int l = strlen(prog);
        char tmp[l+3];
        strcpy(tmp, "_=");
        strcat(tmp, prog);
        (*dest)[idx++] = box_strdup(tmp);
    }
    // and a final NULL
    (*dest)[idx++] = NULL;
    return 0;
}

void AddNewLibs(const char* list)
{
    AppendList(&box64_addlibs, list, 0);
    printf_log(LOG_INFO, "Adding %s to the libs\n", list);
}

void PrintHelp() {
    PrintfFtrace(0, "%s\n", BOX64_BUILD_INFO_STRING);
    PrintfFtrace(0, "Linux userspace x86-64 emulator with a twist.\n");
    PrintfFtrace(0, "There are many environment variables to control Box64's behaviour, checkout the documentation here: https://github.com/ptitSeb/box64/blob/main/docs/USAGE.md\n\n");
    PrintfFtrace(0, "USAGE:\n");
    PrintfFtrace(0, "\tbox64 [options] path/to/x86_64/executable [args]\n");
    PrintfFtrace(0, "\tbox64-bash\n");
    PrintfFtrace(0, "OPTIONS:\n");
    PrintfFtrace(0, "\t-v, --version          print box64 version and quit\n");
    PrintfFtrace(0, "\t-h, --help             print this and quit\n");
    PrintfFtrace(0, "\t-k, --kill-all         kill all box64 instances\n");
    PrintfFtrace(0, "\t-t, --test             run a unit test\n");
    PrintfFtrace(0, "\t--dynacache-list       list of DynaCache file and their validity\n");
    PrintfFtrace(0, "\t--dynacache-clean      remove invalid DynaCache files\n");
}

void KillAllInstances()
{

    struct dirent* entry;
    ssize_t len;
    char proc_path[PATH_MAX];
    char exe_path[PATH_MAX];
    char exe_target[PATH_MAX];
    char self_name[PATH_MAX];
    char self_name_with_deleted[PATH_MAX];

    DIR* proc_dir = opendir("/proc");
    if (proc_dir == NULL) {
        perror("opendir(/proc)");
        return;
    }

    ssize_t self_len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (self_len == -1) {
        perror("readlink(/proc/self/exe)");
        closedir(proc_dir);
        return;
    }
    exe_path[self_len] = '\0';

    char* base_name_self = strrchr(exe_path, '/');
    base_name_self = base_name_self ? base_name_self + 1 : exe_path;
    strncpy(self_name, base_name_self, sizeof(self_name));
    self_name[sizeof(self_name) - 1] = '\0';

    snprintf(self_name_with_deleted, sizeof(self_name_with_deleted), "%s (deleted)", self_name);

    const pid_t self = getpid();
    while ((entry = readdir(proc_dir))) {
        const pid_t pid = atoi(entry->d_name);
        if (!pid || pid == self) continue;

        snprintf(proc_path, sizeof(proc_path), "/proc/%s", entry->d_name);
        struct stat statbuf;
        if (stat(proc_path, &statbuf) == -1 || !S_ISDIR(statbuf.st_mode)) continue;
        snprintf(exe_path, sizeof(exe_path), "%s/exe", proc_path);
        ssize_t len = readlink(exe_path, exe_target, sizeof(exe_target) - 1);
        if (len == -1) continue;
        exe_target[len] = '\0';
        char* base_name = strrchr(exe_target, '/');
        base_name = base_name ? base_name + 1 : exe_target;
        if (strcmp(base_name, self_name) != 0 && strcmp(base_name, self_name_with_deleted) != 0) continue;

        if (!kill(pid, SIGKILL)) {
            printf_log(LOG_INFO, "Killed box64 process %d\n", pid);
        } else {
            printf_log(LOG_INFO, "Failed to kill box64 process %d: %s\n", pid, strerror(errno));
        }
    }
    closedir(proc_dir);
}

static void addLibPaths(box64context_t* context)
{
    if(BOX64ENV(emulated_libs)) {
        char* p = BOX64ENV(emulated_libs);
        ParseList(p, &context->box64_emulated_libs, 0);
        if (my_context->box64_emulated_libs.size && BOX64ENV(log)) {
            printf_log(LOG_INFO, "BOX64 will force the used of emulated libs for ");
            for (int i=0; i<context->box64_emulated_libs.size; ++i)
                printf_log_prefix(0, LOG_INFO, "%s ", context->box64_emulated_libs.paths[i]);
            printf_log_prefix(0, LOG_INFO, "\n");
        }
    }

    // Add libssl and libcrypto (and a few others) to prefer the emulated version because multiple versions exist
    AddPath("libssl.so.1", &context->box64_emulated_libs, 0);
    AddPath("libssl.so.1.0.0", &context->box64_emulated_libs, 0);
    AddPath("libcrypto.so.1", &context->box64_emulated_libs, 0);
    AddPath("libcrypto.so.1.0.0", &context->box64_emulated_libs, 0);
    AddPath("libunwind.so.8", &context->box64_emulated_libs, 0);
    AddPath("libpng12.so.0", &context->box64_emulated_libs, 0);
    AddPath("libcurl.so.4", &context->box64_emulated_libs, 0);
    if(getenv("BOX64_PRESSURE_VESSEL_FILES"))   // use emulated gnutls in this case, it's safer
        AddPath("libgnutls.so.30", &context->box64_emulated_libs, 0);
    AddPath("libtbbmalloc.so.2", &context->box64_emulated_libs, 0);
    AddPath("libtbbmalloc_proxy.so.2", &context->box64_emulated_libs, 0);

    if(BOX64ENV(nosigsegv)) {
        context->no_sigsegv = 1;
    }
    if(BOX64ENV(nosigill)) {
        context->no_sigill = 1;
    }
    if(BOX64ENV(addlibs)) {
        AddNewLibs(BOX64ENV(addlibs));
    }
    loadPath(&context->box64_path, ".:bin", BOX64ENV(path));
    if(getenv("PATH"))
        AppendList(&context->box64_path, getenv("PATH"), 1);   // in case some of the path are for x86 world
}

static void setupZydis(box64context_t* context)
{
#ifdef HAVE_TRACE
    if ((BOX64ENV(trace_init) && strcmp(BOX64ENV(trace_init), "0")) || (BOX64ENV(trace) && strcmp(BOX64ENV(trace), "0"))) {
        context->x64trace = 1;
    }
    if (context->x64trace) {
        printf_log(LOG_INFO, "Initializing Zydis lib\n");
        if (InitX64Trace(context)) {
            printf_log(LOG_INFO, "Zydis init failed. No x86 trace activated\n");
            context->x64trace = 0;
        }
    }
#endif
}

EXPORTDYN
void LoadLDPath(box64context_t *context)
{
    #ifdef BOX32
    if(box64_is32bits)
        loadPath(&context->box64_ld_lib, ".:lib:i386:bin:libs", BOX64ENV(ld_library_path));
    else
    #endif
    loadPath(&context->box64_ld_lib, ".:lib:lib64:x86_64:bin64:libs64", BOX64ENV(ld_library_path));
    #ifndef TERMUX
    if(box64_is32bits) {
        #ifdef BOX32
        if(FileExist("/lib/i386-linux-gnu", 0))
            AddPath("/lib/i386-linux-gnu", &context->box64_ld_lib, 1);
        if(FileExist("/usr/lib/i386-linux-gnu", 0))
            AddPath("/usr/lib/i386-linux-gnu", &context->box64_ld_lib, 1);
        if(FileExist("/usr/i386-linux-gnu/lib", 0))
            AddPath("/usr/i386-linux-gnu/lib", &context->box64_ld_lib, 1);
        if(FileExist("/usr/lib/box64-i386-linux-gnu", 0))
            AddPath("/usr/lib/box64-i386-linux-gnu", &context->box64_ld_lib, 1);
        if(FileExist("/data/data/com.termux/files/usr/glibc/lib/i386-linux-gnu", 0))
            AddPath("/data/data/com.termux/files/usr/glibc/lib/i386-linux-gnu", &context->box64_ld_lib, 1);
        if(FileExist("/data/data/com.termux/files/usr/glibc/lib/box64-i386-linux-gnu", 0))
            AddPath("/data/data/com.termux/files/usr/glibc/lib/box64-i386-linux-gnu", &context->box64_ld_lib, 1);
        if(FileExist("/data/data/com.termux/files/usr/lib/box64-i386-linux-gnu", 0))
            AddPath("/data/data/com.termux/files/usr/lib/box64-i386-linux-gnu", &context->box64_ld_lib, 1);
        #endif
    } else {
        if(FileExist("/lib/x86_64-linux-gnu", 0))
            AddPath("/lib/x86_64-linux-gnu", &context->box64_ld_lib, 1);
        if(FileExist("/usr/lib/x86_64-linux-gnu", 0))
            AddPath("/usr/lib/x86_64-linux-gnu", &context->box64_ld_lib, 1);
        if(FileExist("/usr/x86_64-linux-gnu/lib", 0))
            AddPath("/usr/x86_64-linux-gnu/lib", &context->box64_ld_lib, 1);
        if(FileExist("/usr/lib/box64-x86_64-linux-gnu", 0))
            AddPath("/usr/lib/box64-x86_64-linux-gnu", &context->box64_ld_lib, 1);
        if(FileExist("/data/data/com.termux/files/usr/glibc/lib/x86_64-linux-gnu", 0))
            AddPath("/data/data/com.termux/files/usr/glibc/lib/x86_64-linux-gnu", &context->box64_ld_lib, 1);
        if(FileExist("/data/data/com.termux/files/usr/glibc/lib/box64-x86_64-linux-gnu", 0))
            AddPath("/data/data/com.termux/files/usr/glibc/lib/box64-x86_64-linux-gnu", &context->box64_ld_lib, 1);
        if(FileExist("/data/data/com.termux/files/usr/lib/box64-x86_64-linux-gnu", 0))
            AddPath("/data/data/com.termux/files/usr/lib/box64-x86_64-linux-gnu", &context->box64_ld_lib, 1);
    }
    #else
    //TODO: Add Termux Library Path - Lily
    if(box64_is32bits) {
        #ifdef BOX32
        if(FileExist("/data/data/com.termux/files/usr/lib/i386-linux-gnu", 0))
            AddPath("/data/data/com.termux/files/usr/lib/i386-linux-gnu", &context->box64_ld_lib, 1);
        #endif
    } else {
        if(FileExist("/data/data/com.termux/files/usr/lib/x86_64-linux-gnu", 0))
            AddPath("/data/data/com.termux/files/usr/lib/x86_64-linux-gnu", &context->box64_ld_lib, 1);
    }
    #endif
    if(getenv("LD_LIBRARY_PATH"))
        PrependList(&context->box64_ld_lib, getenv("LD_LIBRARY_PATH"), 1);   // in case some of the path are for x86 world
}

EXPORTDYN
void setupTraceInit()
{
#ifdef HAVE_TRACE
    char* p = BOX64ENV(trace_init);
    if(p) {
        setbuf(stdout, NULL);
        uintptr_t s_trace_start=0, s_trace_end=0;
        if (strcmp(p, "1")==0)
            SetTraceEmu(0, 0);
        else if (strcmp(p, "0")==0)
            ;
        else if (strchr(p,'-')) {
            if(sscanf(p, "%ld-%ld", &s_trace_start, &s_trace_end)!=2) {
                if(sscanf(p, "0x%lX-0x%lX", &s_trace_start, &s_trace_end)!=2)
                    sscanf(p, "%lx-%lx", &s_trace_start, &s_trace_end);
            }
            if(s_trace_start || s_trace_end)
                SetTraceEmu(s_trace_start, s_trace_end);
        } else {
            int veropt = 1;
            int ver = 0;
            const char* vername = NULL;
            int search = 0;
            for(int i=0; i<my_context->elfsize && !search; ++i) {
                search = ElfGetSymbolStartEnd(my_context->elfs[i], &s_trace_start, &s_trace_end, p, &ver, &vername, 1, &veropt)?1:0;
                if(!search)
                    search = ElfGetSymTabStartEnd(my_context->elfs[i], &s_trace_start, &s_trace_end, p);
            }
            if(search) {
                SetTraceEmu(s_trace_start, s_trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)s_trace_start, (void*)s_trace_end);
            } else {
                printf_log(LOG_NONE, "Warning, Symbol to trace (\"%s\") not found, Disabling trace\n", p);
                SetTraceEmu(0, 100);  // disabling trace, mostly
            }
        }
    } else {
        if(BOX64ENV(trace) && strcmp(BOX64ENV(trace), "0"))
            SetTraceEmu(0, 1);
    }
#endif
}

void setupTraceMapLib(lib_t* maplib)
{
#ifdef HAVE_TRACE
    if(!trace_func)
        return;
    char* p = trace_func;
    uintptr_t s_trace_start=0, s_trace_end=0;
    void* search = NULL;
    if(GetAnySymbolStartEnd(maplib, p, &s_trace_start, &s_trace_end, 0, NULL, 1)) {
        SetTraceEmu(s_trace_start, s_trace_end);
        printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)s_trace_start, (void*)s_trace_end);
        box_free(trace_func);
        trace_func = NULL;
        return;
    } else if(GetSymTabStartEnd(maplib, p, &s_trace_start, &s_trace_end)) {
        SetTraceEmu(s_trace_start, s_trace_end);
        printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)s_trace_start, (void*)s_trace_end);
        box_free(trace_func);
        trace_func = NULL;
        return;
    } else {
        printf_log(LOG_NONE, "Warning, Symbol to trace (\"%s\") not found. Trying to set trace later\n", p);
        SetTraceEmu(0, 1);  // disabling trace, mostly
    }
#endif
}

EXPORTDYN
void setupTrace()
{
#ifdef HAVE_TRACE
    char* p = BOX64ENV(trace);
    if(p) {
        setbuf(stdout, NULL);
        uintptr_t s_trace_start=0, s_trace_end=0;
        if (strcmp(p, "1")==0)
            SetTraceEmu(0, 0);
        else if (strcmp(p, "0")==0)
            ;
        else if (strchr(p,'-')) {
            if(sscanf(p, "%ld-%ld", &s_trace_start, &s_trace_end)!=2) {
                if(sscanf(p, "0x%lX-0x%lX", &s_trace_start, &s_trace_end)!=2)
                    sscanf(p, "%lx-%lx", &s_trace_start, &s_trace_end);
            }
            if(s_trace_start || s_trace_end) {
                SetTraceEmu(s_trace_start, s_trace_end);
                if(!s_trace_start && s_trace_end==1) {
                    printf_log(LOG_INFO, "TRACE enabled but inactive\n");
                } else {
                    printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)s_trace_start, (void*)s_trace_end);
                }
            }
        } else {
            int search = 0;
            if (my_context->elfs) {
                int veropt = 1;
                int ver = 0;
                const char* vername = NULL;
                for(int i=0; i<my_context->elfsize && !search; ++i) {
                    search = ElfGetSymbolStartEnd(my_context->elfs[i], &s_trace_start, &s_trace_end, p, &ver, &vername, 1, &veropt)?1:0;
                    if(!search)
                        search = ElfGetSymTabStartEnd(my_context->elfs[i], &s_trace_start, &s_trace_end, p);
                }
            }
            if(search) {
                SetTraceEmu(s_trace_start, s_trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)s_trace_start, (void*)s_trace_end);
            } else {
                printf_log(LOG_NONE, "Warning, Symbol to trace (\"%s\") not found. Trying to set trace later\n", p);
                SetTraceEmu(0, 1);  // disabling trace, mostly
                if(trace_func)
                    box_free(trace_func);
                trace_func = box_strdup(p);
            }
        }
    }
#endif
}
#ifndef STATICBUILD
void endMallocHook();
#endif

void endBox64()
{
    if(!my_context || box64_quit)
        return;

    SerializeAllMapping();   // just to be safe
    // then call all the fini
    dynarec_log(LOG_DEBUG, "endBox64() called\n");
    box64_quit = 1;
    x64emu_t* emu = thread_get_emu();
    void startTimedExit();
    startTimedExit();
    // atexit first
    printf_log(LOG_DEBUG, "Calling atexit registered functions (exiting box64)\n");
    CallAllCleanup(emu);
    printf_log(LOG_DEBUG, "Calling fini for all loaded elfs and unload native libs\n");
    //void closeAllDLOpened();
    //closeAllDLOpened();    // close residual dlopened libs. Disabled, seems like a bad idea, better to unload with proper dependancies
    RunElfFini(my_context->elfs[0], emu);
    // unload needed libs
    needed_libs_t* needed = my_context->elfs[0]->needed;
    printf_log(LOG_DEBUG, "Unloaded main elf: Will Dec RefCount of %d libs\n", needed?needed->size:0);
    if(needed)
        for(int i=0; i<needed->size; ++i)
            DecRefCount(&needed->libs[i], emu);
    // all done, free context
    #ifndef STATICBUILD
    endMallocHook();
    #endif
    SerializeAllMapping();   // to be safe
    FreeBox64Context(&my_context);
    #ifdef DYNAREC
    // disable dynarec now
    SET_BOX64ENV(dynarec, 0);
    #endif
    if(BOX64ENV(libgl)) {
        box_free(BOX64ENV(libgl));
        SET_BOX64ENV(libgl, NULL);
    }
    if(box64_custom_gstreamer) {
        box_free(box64_custom_gstreamer);
        box64_custom_gstreamer = NULL;
    }
}


static void free_contextargv()
{
    for(int i=0; i<my_context->argc; ++i)
        box_free(my_context->argv[i]);
}

static void add_argv(const char* what) {
    int there = 0;
    for(int i=1; i<my_context->argc && !there; ++i)
        if(!strcmp(my_context->argv[i], what))
            there = 1;
    if(!there) {
        // try to prepend the arg, not appending
        static int where = 0;
        if(!where)
            where = (box64_wine)?2:1;
        printf_log(LOG_INFO, "Inserting \"%s\" to the argument %d\n", what, where);
        my_context->argv = (char**)box_realloc(my_context->argv, (my_context->argc+1+1)*sizeof(char*));
        memmove(my_context->argv+where+1, my_context->argv+where, (my_context->argc-where)*sizeof(char*));
        my_context->argv[where] = box_strdup(what);
        my_context->argc++;
        where++;
    }
}

#ifndef STATICBUILD
void pressure_vessel(int argc, const char** argv, int nextarg, const char* prog);
#endif
extern char** environ;

int initialize(int argc, const char **argv, char** env, x64emu_t** emulator, elfheader_t** elfheader, int exec)
{
    #ifndef STATICBUILD
    init_malloc_hook();
    #endif
    init_auxval(argc, argv, environ?environ:env);
    // analogue to QEMU_VERSION in qemu-user-mode emulation
    if(getenv("BOX64_VERSION")) {
        PrintBox64Version(0);
        exit(0);
    }
    // trying to open and load 1st arg
    if(argc==1) {
        /*PrintBox64Version(1);
        PrintHelp();
        return 1;*/
        printf("[BOX64] Missing operand after 'box64'\n");
        printf("See 'box64 --help' for more information.\n");
        exit(0);
    }
    if(argc>1 && !strcmp(argv[1], "/usr/bin/gdb") && BOX64ENV(trace_file))
        exit(0);
    // uname -m is redirected to box64 -m
    if (argc == 2 && (!strcmp(argv[1], "-m") || !strcmp(argv[1], "-p") || !strcmp(argv[1], "-i"))) {
        printf("x86_64\n");
        exit(0);
    }

    if (argc >= 3 && (!strcmp(argv[1], "--test") || !strcmp(argv[1], "-t"))) {
        box64_unittest_mode = 1;
        exit(unittest(argc, argv));
    }

    ftrace = stderr;

    // grab pagesize
    box64_pagesize = sysconf(_SC_PAGESIZE);
    if(!box64_pagesize)
        box64_pagesize = 4096;

    LoadEnvVariables();
    InitializeEnvFiles();

    const char* prog = argv[1];
    int nextarg = 1;
    // check if some options are passed
    while(prog && prog[0]=='-') {
        if(!strcmp(prog, "-v") || !strcmp(prog, "--version")) {
            PrintBox64Version(0);
            exit(0);
        }
        if(!strcmp(prog, "-h") || !strcmp(prog, "--help")) {
            PrintHelp();
            exit(0);
        }
        if (!strcmp(prog, "-k") || !strcmp(prog, "--kill-all")) {
            KillAllInstances();
            exit(0);
        }
        if(!strcmp(prog, "--dynacache-list")) {
            DynaCacheList(argv[nextarg+1]);
            exit(0);
        }
        if(!strcmp(prog, "--dynacache-clean")) {
            DynaCacheClean();
            exit(0);
        }
        // other options?
        if(!strcmp(prog, "--")) {
            prog = argv[++nextarg];
            break;
        }
        printf("Warning, unrecognized option '%s'\n", prog);
        prog = argv[++nextarg];
    }
    if(!prog || nextarg==argc) {
        printf("[BOX64] Nothing to run\n");
        exit(0);
    }

    if (!BOX64ENV(nobanner)) PrintBox64Version(1);

    displayMiscInfo();

    hookMangoHud();

    char* bashpath = NULL;
    {
        char* p = BOX64ENV(bash);
        if(p) {
            if(FileIsX64ELF(p)) {
                bashpath = p;
                printf_log(LOG_INFO, "Using bash \"%s\"\n", bashpath);
            } else {
                printf_log(LOG_INFO, "The x86_64 bash \"%s\" is not an x86_64 binary.\n", p);
            }
        }
    }
    char* pythonpath = NULL;
    {
        char* p = BOX64ENV(python3);
        if(p) {
            if(FileIsX64ELF(p)) {
                pythonpath = p;
                printf_log(LOG_INFO, "Using python3 \"%s\"\n", pythonpath);
            } else {
                printf_log(LOG_INFO, "The x86_64 python3 \"%s\" is not an x86_64 binary.\n", p);
            }
        }
    }

    // precheck, for win-preload
    const char* prog_ = strrchr(prog, '/');
    if(!prog_) prog_ = prog; else ++prog_;
    if(!strcmp(prog_, "wine-preloader") || !strcmp(prog_, "wine64-preloader")) {
        // wine-preloader detecter, skipping it if next arg exist and is an x86 binary
        int x64 = (nextarg<argc)?FileIsX64ELF(argv[nextarg]):0;
        #ifdef BOX32
        int x86 = (nextarg<argc)?FileIsX86ELF(argv[nextarg]):0;
        #else
        int x86 = 0;
        #endif
        if(x64 || x86) {
            prog = argv[++nextarg];
            printf_log(LOG_INFO, "Wine preloader detected, loading \"%s\" directly\n", prog);
            wine_preloaded = 1;
            prog_ = strrchr(prog, '/');
            if(!prog_) prog_ = prog; else ++prog_;
        }
    }
    #ifndef STATICBUILD
    // pre-check for pressure-vessel-wrap
    if(!strcmp(prog_, "pressure-vessel-wrap")) {
        printf_log(LOG_INFO, "pressure-vessel-wrap detected\n");
        unsetenv("BOX64_ARG0");
        pressure_vessel(argc, argv, nextarg+1, prog);
    }
    #endif
    int ld_libs_args = -1;
    int is_custom_gstreamer = 0;
    // check if this is wine
    if(!strcmp(prog_, "wine64")
     || !strcmp(prog_, "wine64-development")
     || !strcmp(prog_, "wine")) {
        const char* prereserve = getenv("WINEPRELOADRESERVE");
        printf_log(LOG_INFO, "Wine64 detected, WINEPRELOADRESERVE=\"%s\"\n", prereserve?prereserve:"");
        if(wine_preloaded || 1) {
            wine_prereserve(prereserve);
        }
        // special case for winedbg, doesn't work anyway
        if(argv[nextarg+1] && strstr(argv[nextarg+1], "winedbg")==argv[nextarg+1]) {
            if(getenv("BOX64_WINEDBG")) {
                SET_BOX64ENV(nobanner, 1);
                BOX64ENV(log) = 0;
            } else {
                printf_log(LOG_NONE, "winedbg detected, not launching it!\n");
                exit(0);    // exiting, it doesn't work anyway
            }
        }
        box64_wine = 1;
        // check if it's proton, with it's custom gstreamer build, to disable gtk3 loading
        char tmp[strlen(prog)+100];
        strcpy(tmp, prog);
        char* pp = strrchr(tmp, '/');
        if(pp) {
            *pp = '\0'; // remove the wine binary call
            strcat(tmp, "/../lib64/gstreamer-1.0");
            // check if it exist
            if(FileExist(tmp, 0)) {
                box64_custom_gstreamer = box_strdup(tmp);
            } else {
                *pp = '\0';
                strcat(tmp, "/../lib/x86_64-linux-gnu/gstreamer-1.0");
                if(FileExist(tmp, 0)) {
                   box64_custom_gstreamer = box_strdup(tmp);
                }
            }
        }
        // Try to get the name of the exe being run, to ApplyEnvFileEntry laters
        if(argv[nextarg+1] && argv[nextarg+1][0]!='-' && strlen(argv[nextarg+1])>4 /*&& !strcasecmp(argv[nextarg+1]+strlen(argv[nextarg+1])-4, ".exe")*/) {
            const char* pp = strrchr(argv[nextarg+1], '/');
            if(pp)
                box64_wine_guest_name = pp + 1;
            else {
                pp = strrchr(argv[nextarg+1], '\\');
                if(pp)
                    box64_wine_guest_name = pp + 1;
                else
                    box64_wine_guest_name = argv[nextarg + 1];
            }
        }
        if (box64_wine_guest_name) printf_log(LOG_INFO, "Detected running wine with \"%s\"\n", box64_wine_guest_name);
    } else if(strstr(prog, "ld-musl-x86_64.so.1")) {
    // check if ld-musl-x86_64.so.1 is used
        printf_log(LOG_INFO, "ld-musl detected. Trying to workaround and use system ld-linux\n");
        box64_musl = 1;
        // skip ld-musl and go through args unti "--" is found, handling "--library-path" to add some libs to BOX64_LD_LIBRARY
        ++nextarg;
        while(strcmp(argv[nextarg], "--")) {
            if(!strcmp(argv[nextarg], "--library-path")) {
                ++nextarg;
                ld_libs_args = nextarg;
            }
            ++nextarg;
        }
        ++nextarg;
        prog = argv[nextarg];
    } else if(!strcmp(prog_, "steam") ) {
        printf_log(LOG_INFO, "steam detected\n");
        box64_steam = 1;
    } else if(!strcmp(prog_, "steamcmd")) {
        printf_log(LOG_INFO, "steamcmd detected\n");
        box64_steamcmd = 1;
    } else  if(!strcmp(prog_, "wineserver")) {
        // check if this is wineserver
        box64_wine = 1;
    }
    // Create a new context
    my_context = NewBox64Context(argc - nextarg);

    addLibPaths(my_context);

    // Append ld_list if it exist
    if(ld_libs_args!=-1)
        PrependList(&my_context->box64_ld_lib, argv[ld_libs_args], 1);
    if(is_custom_gstreamer) //TODO: is this still needed?
        AddPath("libwayland-client.so.0", &my_context->box64_emulated_libs, 0);

    my_context->box64path = ResolveFile(argv[0], &my_context->box64_path);
    // prepare all other env. var
    my_context->envc = CountEnv(environ?environ:env);
    printf_log(LOG_INFO, "Counted %d Env var\n", my_context->envc);
    // allocate extra space for new environment variables such as BOX64_PATH
    my_context->envv = (char**)box_calloc(my_context->envc+1, sizeof(char*));

    path_collection_t ld_preload = {0};
    if(getenv("BOX64_LD_PRELOAD")) {
        char* p = getenv("BOX64_LD_PRELOAD");
        ParseList(p, &ld_preload, 0);
        if (ld_preload.size && BOX64ENV(log)) {
            printf_log(LOG_INFO, "BOX64 trying to Preload ");
            for (int i=0; i<ld_preload.size; ++i)
                printf_log_prefix(0, LOG_INFO, "%s ", ld_preload.paths[i]);
            printf_log_prefix(0, LOG_INFO, "\n");
        }
    }
    if(getenv("LD_PRELOAD")) {
        char* p = getenv("LD_PRELOAD");
        if(strstr(p, "libtcmalloc_minimal.so.0"))
            box64_tcmalloc_minimal = 1;
        if(strstr(p, "libtcmalloc_minimal.so.4"))
            box64_tcmalloc_minimal = 1;
        if(strstr(p, "libtcmalloc_minimal_debug.so.4"))
            box64_tcmalloc_minimal = 1;
        if(strstr(p, "libasan.so"))
            box64_tcmalloc_minimal = 1; // it seems Address Sanitizer doesn't handle dlsym'd malloc very well
        AppendList(&ld_preload, p, 0);
        if (ld_preload.size && BOX64ENV(log)) {
            printf_log(LOG_INFO, "BOX64 trying to Preload ");
            for (int i=0; i<ld_preload.size; ++i)
                printf_log_prefix(0, LOG_INFO, "%s ", ld_preload.paths[i]);
            printf_log_prefix(0, LOG_INFO, "\n");
        }
    }
    // print PATH and LD_LIB used
    PrintCollection(&my_context->box64_ld_lib, "Library search path");
    PrintCollection(&my_context->box64_path, "Binary search path");
    // lets build argc/argv stuff
    printf_log(LOG_INFO, "Looking for %s\n", prog);
    if(strchr(prog, '/'))
        my_context->argv[0] = box_strdup(prog);
    else
        my_context->argv[0] = ResolveFileSoft(prog, &my_context->box64_path);
    //
    GatherEnv(&my_context->envv, environ?environ:env, my_context->argv[0]);
    if (BOX64ENV(dump) || BOX64ENV(log)<=LOG_DEBUG) {
        for (int i=0; i<my_context->envc; ++i)
            printf_dump(LOG_DEBUG, " Env[%02d]: %s\n", i, my_context->envv[i]);
    }
    // check if box86 is present
    {
        my_context->box86path = box_strdup(my_context->box64path);
        #ifndef BOX32
        if(strstr(my_context->box86path, "box64")) {
            char* p = strrchr(my_context->box86path, '6');  // get the 6 of box64
            p[0] = '8'; p[1] = '6'; // change 64 to 86
        } else {
            box_free(my_context->box86path);
            my_context->box86path = ResolveFileSoft("box86", &my_context->box64_path);
        }
        if(my_context->box86path && !FileExist(my_context->box86path, IS_FILE)) {
            box_free(my_context->box86path);
            my_context->box86path = NULL;
        }
        #endif
    }
    box64_guest_name = strrchr(prog, '/');
    if (!box64_guest_name)
        box64_guest_name = prog;
    else
        ++box64_guest_name;
    if(box64_wine) {
        #ifdef ANDROID
            AddPath("libdl.so", &ld_preload, 0);
        #else
            AddPath("libdl.so.2", &ld_preload, 0);
        #endif
    }
    // special case for zoom
    if (strstr(box64_guest_name, "zoom") == box64_guest_name) {
        printf_log(LOG_INFO, "Zoom detected, Trying to use system libturbojpeg if possible\n");
        box64_zoom = 1;
    }
    // special case for bash
    int setup_bash_rcfile = 0;
    if (!strcmp(box64_guest_name, "bash") || !strcmp(box64_guest_name, "box64-bash")) {
        printf_log(LOG_INFO, "Bash detected, disabling banner\n");
        if (!BOX64ENV(nobanner)) {
            setenv("BOX86_NOBANNER", "1", 0);
            setenv("BOX64_NOBANNER", "1", 0);
        }
        if (!bashpath) {
            bashpath = (char*)prog;
            SET_BOX64ENV(bash, (char*)prog);
            setenv("BOX64_BASH", prog, 1);
        }
        setup_bash_rcfile = 1;
    }
    if(!bashpath)
        bashpath = ResolveFile("box64-bash", &my_context->box64_path);
    if(bashpath)
        my_context->bashpath = box_strdup(bashpath);
    if(pythonpath)
        my_context->pythonpath = box_strdup(pythonpath);

    ApplyEnvFileEntry(box64_guest_name);
    if (box64_wine && box64_wine_guest_name) {
        ApplyEnvFileEntry(box64_wine_guest_name);
        box64_wine_guest_name = NULL;
    }
    // Try to open ftrace again after applying rcfile.
    openFTrace();
    setupZydis(my_context);
    PrintEnvVariables(&box64env, LOG_INFO);

    for(int i=1; i<my_context->argc; ++i) {
        my_context->argv[i] = box_strdup(argv[i+nextarg]);
        printf_log(LOG_INFO, "argv[%i]=\"%s\"\n", i, my_context->argv[i]);
    }

    // Setup custom bash rcfile iff no args are present
    if (setup_bash_rcfile && my_context->argc == 1) {
        add_argv("--rcfile");
        add_argv("box64-custom-bashrc-file"); // handled by my_open
    }

    if(BOX64ENV(nosandbox))
    {
        add_argv("--no-sandbox");
    }
    if(BOX64ENV(inprocessgpu))
    {
        add_argv("--in-process-gpu");
    }
    if(BOX64ENV(cefdisablegpu))
    {
        add_argv("-cef-disable-gpu");
    }
    if(BOX64ENV(cefdisablegpucompositing))
    {
        add_argv("-cef-disable-gpu-compositing");
    }
    // add new args only if there is no args already
    if(BOX64ENV(args)) {
        char tmp[256];
        char* p = BOX64ENV(args);
        int state = 0;
        char* p2 = p;
        if(my_context->argc==1 || (my_context->argc==2 && box64_wine))
            while(state>=0) {
                switch(*p2) {
                    case 0: // end of flux
                        if(state && (p2!=p)) add_argv(p);
                        state = -1;
                        break;
                    case '"': // start/end of quotes
                        if(state<2) {if(!state) p=p2; state=2;} else state=1;
                        break;
                    case ' ':
                        if(state==1) {strncpy(tmp, p, p2-p); tmp[p2-p]='\0'; add_argv(tmp); state=0;}
                        break;
                    default:
                        if(state==0) {state=1; p=p2;}
                        break;
                }
                ++p2;
            }
    }
    if(BOX64ENV(insert_args)) {
        char tmp[256];
        char* p = BOX64ENV(insert_args);
        int state = 0;
        char* p2 = p;
        while(state>=0) {
            switch(*p2) {
                case 0: // end of flux
                    if(state && (p2!=p)) add_argv(p);
                    state = -1;
                    break;
                case '"': // start/end of quotes
                    if(state<2) {if(!state) p=p2; state=2;} else state=1;
                    break;
                case ' ':
                    if(state==1) {strncpy(tmp, p, p2-p); tmp[p2-p]='\0'; add_argv(tmp); state=0;}
                    break;
                default:
                    if(state==0) {state=1; p=p2;}
                    break;
            }
            ++p2;
        }
    }
    // check if file exist
    if(!my_context->argv[0] || !FileExist(my_context->argv[0], IS_FILE)) {
        printf_log(LOG_NONE, "Error: File is not found. (%s)\n", my_context->argv[0]);
        free_contextargv();
        FreeBox64Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    if(!FileExist(my_context->argv[0], IS_FILE|IS_EXECUTABLE)) {
        printf_log(LOG_NONE, "Error: %s is not an executable file.\n", my_context->argv[0]);
        free_contextargv();
        FreeBox64Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    if(!(my_context->fullpath = box_realpath(my_context->argv[0], NULL)))
        my_context->fullpath = box_strdup(my_context->argv[0]);
    if (getenv("BOX64_ARG0")) {
        my_context->argv[0] = box_strdup(getenv("BOX64_ARG0"));
        unsetenv("BOX64_ARG0");
    }
    FILE *f = fopen(my_context->fullpath, "rb");
    if(!f) {
        printf_log(LOG_NONE, "Error: Cannot open %s\n", my_context->fullpath);
        free_contextargv();
        FreeBox64Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    #ifdef BOX32
    box64_is32bits = FileIsX86ELF(my_context->fullpath);
    // try to switch personality, but only if not already tried
    if(box64_is32bits) {
        int tried = getenv("BOX32_PERSONA32BITS")?1:0;
        if(tried) {
            unsetenv("BOX32_PERSONA32BITS");
            int p = personality(0xffffffff);
            if(p==ADDR_LIMIT_32BIT) {
                box64_isAddressSpace32 = 1;
                printf_log(LOG_INFO, "Personality set to 32bits\n");
            }
        } else {
            if(personality(ADDR_LIMIT_32BIT)!=-1) {
                int nenv = 0;
                while(env[nenv]) nenv++;
                // alloc + "LD_PRELOAD" if needd + last NULL ending
                char** newenv = (char**)box_calloc(nenv+1+1, sizeof(char*));
                // copy strings
                for (int i=0; i<nenv; ++i)
                    newenv[i] = box_strdup(env[i]);
                newenv[nenv] = "BOX32_PERSONA32BITS=1";
                // re-launch...
                if(execve(my_context->box64path, (void*)argv, newenv)<0)
                    printf_log(LOG_NONE, "Failed to relaunch. Error is %d/%s (argv[0]=\"%s\")\n", errno, strerror(errno), argv[0]);
            }
        }
    }
    if(box64_is32bits) {
        printf_log(LOG_INFO, "Using Box32 to load 32bits elf\n");
        loadProtectionFromMap();
        reserveHighMem();
        init_pthread_helper_32();
    }
    #endif
    LoadLDPath(my_context);
    elfheader_t *elf_header = LoadAndCheckElfHeader(f, my_context->fullpath, 1);
    if(!elf_header) {
        int x86 = my_context->box86path?FileIsX86ELF(my_context->fullpath):0;
        int script = my_context->bashpath?FileIsShell(my_context->fullpath):0;
        int python3 = my_context->pythonpath?FileIsPython(my_context->fullpath):0;
        printf_log(LOG_NONE, "Error: Reading elf header of %s, Try to launch %s instead\n", my_context->fullpath, x86?"using box86":(script?"using bash":"natively"));
        fclose(f);
        FreeCollection(&ld_preload);
        int ret;
        if(x86) {
            // duplicate the array and insert 1st arg as box86
            const char** newargv = (const char**)box_calloc(my_context->argc+2, sizeof(char*));
            newargv[0] = my_context->box86path;
            for(int i=0; i<my_context->argc; ++i)
                newargv[i+1] = my_context->argv[i];
            ret = execvp(newargv[0], (char * const*)newargv);
        } else if (script) {
            // duplicate the array and insert 1st arg as box64, 2nd is bash
            const char** newargv = (const char**)box_calloc(my_context->argc+3, sizeof(char*));
            newargv[0] = my_context->box64path;
            newargv[1] = my_context->bashpath;
            for(int i=0; i<my_context->argc; ++i)
                newargv[i+2] = my_context->argv[i];
            ret = execvp(newargv[0], (char * const*)newargv);
        } else if (python3) {
            // duplicate the array and insert 1st arg as box64, 2nd is python3
            const char** newargv = (const char**)box_calloc(my_context->argc+3, sizeof(char*));
            newargv[0] = my_context->box64path;
            newargv[1] = my_context->pythonpath;
            for(int i=0; i<my_context->argc; ++i)
                newargv[i+2] = my_context->argv[i];
            ret = execvp(newargv[0], (char * const*)newargv);
        } else {
            const char** newargv = (const char**)box_calloc(my_context->argc+1, sizeof(char*));
            for(int i=0; i<my_context->argc; ++i)
                newargv[i] = my_context->argv[i];
            ret = execvp(newargv[0], (char * const*)newargv);
        }
        free_contextargv();
        FreeBox64Context(&my_context);
        return ret;
    }
    AddElfHeader(my_context, elf_header);
    *elfheader = elf_header;

    if(CalcLoadAddr(elf_header)) {
        printf_log(LOG_NONE, "Error: Reading elf header of %s\n", my_context->fullpath);
        FreeElfHeader(&elf_header);
        free_contextargv();
        FreeBox64Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    // allocate memory and load elf
    if(AllocLoadElfMemory(my_context, elf_header, 1)) {
        printf_log(LOG_NONE, "Error: Loading elf %s\n", my_context->fullpath);
        FreeElfHeader(&elf_header);
        free_contextargv();
        FreeBox64Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    if (!strcmp(box64_guest_name, "heroic")) {
        // check if heroic needs patching (for the 2.15.1 version)
        uint8_t* address = GetBaseAddress(elf_header);
        if(address[0x422f6e1]==0x72 && address[0x422f6e2]==0x44 && address[0x422f6e0]==0xF8 && address[0x422f727]==0xcc) {
            printf_log(LOG_INFO, "Patched heroic!\n");
            uintptr_t page = ((uintptr_t)&address[0x422f6e1])&~(box64_pagesize-1);
            int prot = getProtection(page);
            mprotect((void*)page, box64_pagesize, PROT_READ|PROT_WRITE|PROT_EXEC);
            address[0x422f6e1]=0x90; address[0x422f6e2]=0x90;
            mprotect((void*)page, box64_pagesize, prot);
        }
    }
    if(ElfCheckIfUseTCMallocMinimal(elf_header)) {
        if(!box64_tcmalloc_minimal) {
            // need to reload with tcmalloc_minimal as a LD_PRELOAD!
            printf_log(LOG_INFO, "tcmalloc_minimal.so.4 used. Reloading box64 with the lib preladed\n");
            // need to get a new envv variable. so first count it and check if LD_PRELOAD is there
            int preload=(getenv("LD_PRELOAD"))?1:0;
            int nenv = 0;
            while(env[nenv]) nenv++;
            // alloc + "LD_PRELOAD" if needd + last NULL ending
            char** newenv = (char**)box_calloc(nenv+1+((preload)?0:1), sizeof(char*));
            // copy strings
            for (int i=0; i<nenv; ++i)
                newenv[i] = box_strdup(env[i]);
            // add ld_preload
            if(preload) {
                // find the line
                int l = 0;
                while(l<nenv) {
                    if(strstr(newenv[l], "LD_PRELOAD=")==newenv[l]) {
                        // found it!
                        char *old = newenv[l];
                        newenv[l] = (char*)box_calloc(strlen(old)+strlen("libtcmalloc_minimal.so.4:")+1, sizeof(char));
                        strcpy(newenv[l], "LD_PRELOAD=libtcmalloc_minimal.so.4:");
                        strcat(newenv[l], old + strlen("LD_PRELOAD="));
                        box_free(old);
                        // done, end loop
                        l = nenv;
                    } else ++l;
                }
            } else {
                //move last one
                newenv[nenv] = box_strdup(newenv[nenv-1]);
                box_free(newenv[nenv-1]);
                newenv[nenv-1] = box_strdup("LD_PRELOAD=libtcmalloc_minimal.so.4");
            }
            // duplicate argv too
            char** newargv = box_calloc(argc+1, sizeof(char*));
            int narg = 0;
            while(argv[narg]) {newargv[narg] = box_strdup(argv[narg]); narg++;}
            // launch with new env...
            if(execve(newargv[0], newargv, newenv)<0)
                printf_log(LOG_NONE, "Failed to relaunch. Error is %d/%s\n", errno, strerror(errno));
        } else {
            printf_log(LOG_INFO, "Using tcmalloc_minimal.so.4, and it's in the LD_PRELOAD command\n");
        }
    }
#if defined(RPI) || defined(RK3399) || defined(RK3326)
    // before launching emulation, let's check if this is a mojosetup from GOG
    if (((strstr(prog, "bin/linux/x86_64/mojosetup") && getenv("MOJOSETUP_BASE")) || strstr(prog, ".mojosetup/mojosetup"))
       && getenv("GTK2_RC_FILES")) {
        sanitize_mojosetup_gtk_background();
    }
#endif
    // change process name
    {
        char* p = strrchr(my_context->fullpath, '/');
        if(p)
            ++p;
        else
            p = my_context->fullpath;
        if(prctl(PR_SET_NAME, p)==-1)
            printf_log(LOG_NONE, "Error setting process name (%s)\n", strerror(errno));
        else
            printf_log(LOG_INFO, "Rename process to \"%s\"\n", p);
        if (strcmp(box64_guest_name, p)) {
            ApplyEnvFileEntry(p);
        }
        // and now all change the argv (so libs libs mesa find the correct program names)
        char* endp = (char*)argv[argc-1];
        while(*endp)
            ++endp;    // find last argv[] address
        uintptr_t diff = prog - argv[0]; // this is the difference we need to compensate
        for(p=(char*)prog; p<=endp; ++p)
            *(p - diff) = *p;  // copy all element at argv[nextarg] to argv[0]
        memset(endp - diff, 0, diff); // fill reminder with NULL
        for(int i=nextarg; i<argc; ++i)
            argv[i] -= diff;    // adjust strings
        my_context->orig_argc = argc;
        my_context->orig_argv = (char**)argv;
    }
    box64_nolibs = (NeededLibs(elf_header)==0);
    if(box64_nolibs) printf_log(LOG_INFO, "Warning, box64 is not really compatible with staticaly linked binaries. Expect crash!\n");
    box64_isglibc234 = GetNeededVersionForLib(elf_header, "libc.so.6", "GLIBC_2.34");
    if(box64_isglibc234)
        printf_log(LOG_DEBUG, "Program linked with GLIBC 2.34+\n");
    // get and alloc stack size and align
    if(CalcStackSize(my_context)) {
        printf_log(LOG_NONE, "Error: Allocating stack\n");
        free_contextargv();
        FreeBox64Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    // init x86_64 emu
    x64emu_t *emu = NewX64Emu(my_context, my_context->ep, (uintptr_t)my_context->stack, my_context->stacksz, 0);
    // stack setup is much more complicated then just that!
    SetupInitialStack(emu); // starting here, the argv[] don't need free anymore
    SetupX64Emu(emu, NULL);
    if(box64_is32bits) {
        SetEAX(emu, my_context->argc);
        SetEBX(emu, my_context->argv32);
    } else {
        SetRSI(emu, my_context->argc);
        SetRDX(emu, (uint64_t)my_context->argv);
        SetRCX(emu, (uint64_t)my_context->envv);
        SetRBP(emu, 0); // Frame pointer so to "No more frame pointer"
    }

    thread_set_emu(emu);

    // export symbols
    AddSymbols(my_context->maplib, elf_header);
    if(wine_preloaded) {
        uintptr_t wineinfo = 0;
        int ver = -1, veropt = 0;
        const char* vername = NULL;
        if(!ElfGetGlobalSymbolStartEnd(elf_header, &wineinfo, NULL, "wine_main_preload_info", &ver, &vername, 1, &veropt))
            if(!ElfGetWeakSymbolStartEnd(elf_header, &wineinfo, NULL, "wine_main_preload_info", &ver, &vername, 1, &veropt))
                ElfGetLocalSymbolStartEnd(elf_header, &wineinfo, NULL, "wine_main_preload_info", &ver, &vername, 1, &veropt);
        if(!wineinfo) {printf_log(LOG_DEBUG, "Warning, Symbol wine_main_preload_info not found\n");}
        else {
            *(void**)wineinfo = get_wine_prereserve();
            printf_log(LOG_DEBUG, "WINE wine_main_preload_info found and updated %p -> %p\n", get_wine_prereserve(), *(void**)wineinfo);
        }
        #ifdef DYNAREC
        dynarec_wine_prereserve();
        #endif
    }
    AddMainElfToLinkmap(elf_header);    //TODO: LinkMap seems incorect
    // pre-load lib if needed
    if(ld_preload.size) {
        my_context->preload = new_neededlib(0);
        for(int i=0; i<ld_preload.size; ++i) {
            needed_libs_t* tmp = new_neededlib(1);
            tmp->names[0] = ld_preload.paths[i];
            if(AddNeededLib(my_context->maplib, 0, 0, 0, tmp, elf_header, my_context, emu)) {
                printf_log(LOG_INFO, "Warning, cannot pre-load %s\n", tmp->names[0]);
                RemoveNeededLib(my_context->maplib, 0, tmp, my_context, emu);
            } else {
                for(int j=0; j<tmp->size; ++j)
                    add1lib_neededlib(my_context->preload, tmp->libs[j], tmp->names[j]);
            }
            free_neededlib(tmp);
        }
    }
    FreeCollection(&ld_preload);
    // Call librarian to load all dependant elf
    if(LoadNeededLibs(elf_header, my_context->maplib, 0, 0, 0, my_context, emu)) {
        printf_log(LOG_NONE, "Error: Loading needed libs in elf %s\n", my_context->argv[0]);
        FreeBox64Context(&my_context);
        return -1;
    }
    // reloc...
    printf_log(LOG_DEBUG, "And now export symbols / relocation for %s...\n", ElfName(elf_header));
    if(RelocateElf(my_context->maplib, NULL, 0, 0, elf_header)) {
        printf_log(LOG_NONE, "Error: Relocating symbols in elf %s\n", my_context->argv[0]);
        FreeBox64Context(&my_context);
        return -1;
    }
    // and handle PLT
    RelocateElfPlt(my_context->maplib, NULL, 0, 0, elf_header);
    // deferred init
    setupTraceInit();
    RunDeferredElfInit(emu);
    // update TLS of main elf
    RefreshElfTLS(elf_header, emu);
    // do some special case check, _IO_2_1_stderr_ and friends, that are setup by libc, but it's already done here, so need to do a copy
    ResetSpecialCaseMainElf(elf_header);
    // init...
    setupTrace();

    *emulator = emu;
    return 0;
}

int emulate(x64emu_t* emu, elfheader_t* elf_header)
{
    // get entrypoint
    my_context->ep = GetEntryPoint(my_context->maplib, elf_header);

    atexit(endBox64);
    loadProtectionFromMap();

    // emulate!
    printf_log(LOG_DEBUG, "Start x64emu on Main\n");
    // Stack is ready, with stacked: NULL env NULL argv argc
    ResetFlags(emu);
    #ifdef BOX32
    if(box64_is32bits) {
        SetEIP(emu, my_context->ep);
        Push32(emu, my_context->exit_bridge);  // push to pop it just after
        SetEDX(emu, Pop32(emu));    // RDX is exit function
    } else
    #endif
    {
        SetRIP(emu, my_context->ep);
        Push64(emu, my_context->exit_bridge);  // push to pop it just after
        SetRDX(emu, Pop64(emu));    // RDX is exit function
    }
    DynaRun(emu);
    // Get EAX
    int ret = GetEAX(emu);
    printf_log(LOG_DEBUG, "Emulation finished, EAX=%d\n", ret);
    endBox64();
#ifdef HAVE_TRACE
    if(trace_func)  {
        box_free(trace_func);
        trace_func = NULL;
    }
#endif

#ifdef DYNAREC
    if (BOX64ENV(dynarec_perf_map) && BOX64ENV(dynarec_perf_map_fd) != -1) {
        close(BOX64ENV(dynarec_perf_map_fd));
        SET_BOX64ENV(dynarec_perf_map_fd, -1);
    }
#endif
    return ret;
}
