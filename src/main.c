#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <stdarg.h>
#ifdef DYNAREC
#ifdef ARM64
#include <linux/auxvec.h>
#include <asm/hwcap.h>
#endif
#endif

#include "build_info.h"
#include "debug.h"
#include "fileutils.h"
#include "box64context.h"
#include "wine_tools.h"
#include "elfloader.h"
#include "custommem.h"
#include "box64stack.h"
#include "auxval.h"
#include "x64emu.h"
#include "threads.h"
#include "x64trace.h"
#include "librarian.h"
#include "x64run.h"
#include "symbols.h"
#include "rcfile.h"
#include "emu/x64run_private.h"

box64context_t *my_context = NULL;
int box64_quit = 0;
int box64_log = LOG_INFO; //LOG_NONE;
int box64_dump = 0;
int box64_nobanner = 0;
int box64_dynarec_log = LOG_NONE;
uintptr_t box64_pagesize;
uintptr_t box64_load_addr = 0;
int box64_nosandbox = 0;
int box64_malloc_hack = 0;
#ifdef DYNAREC
int box64_dynarec = 1;
int box64_dynarec_dump = 0;
int box64_dynarec_forced = 0;
int box64_dynarec_bigblock = 1;
int box64_dynarec_forward = 128;
int box64_dynarec_strongmem = 0;
int box64_dynarec_x87double = 0;
int box64_dynarec_fastnan = 1;
int box64_dynarec_fastround = 1;
int box64_dynarec_safeflags = 1;
int box64_dynarec_callret = 0;
int box64_dynarec_hotpage = 0;
int box64_dynarec_fastpage = 0;
int box64_dynarec_bleeding_edge = 1;
int box64_dynarec_jvm = 1;
int box64_dynarec_wait = 1;
int box64_dynarec_test = 0;
int box64_dynarec_missing = 0;
uintptr_t box64_nodynarec_start = 0;
uintptr_t box64_nodynarec_end = 0;
#ifdef ARM64
int arm64_asimd = 0;
int arm64_aes = 0;
int arm64_pmull = 0;
int arm64_crc32 = 0;
int arm64_atomics = 0;
#elif defined(RV64)
int rv64_zba = 0;
int rv64_zbb = 0;
int rv64_zbc = 0;
int rv64_zbs = 0;
int rv64_xtheadba = 0;
int rv64_xtheadbb = 0;
int rv64_xtheadbs = 0;
int rv64_xtheadcondmov = 0;
int rv64_xtheadmemidx = 0;
int rv64_xtheadmempair = 0;
int rv64_xtheadfmemidx = 0;
int rv64_xtheadmac = 0;
int rv64_xtheadfmv = 0;
#endif
#else   //DYNAREC
int box64_dynarec = 0;
#endif
int box64_libcef = 1;
int dlsym_error = 0;
int cycle_log = 0;
#ifdef HAVE_TRACE
int trace_xmm = 0;
int trace_emm = 0;
int trace_regsdiff = 0;
uint64_t start_cnt = 0;
uintptr_t trace_start = 0, trace_end = 0;
char* trace_func = NULL;
char* trace_init = NULL;
char* box64_trace = NULL;
#ifdef DYNAREC
int box64_dynarec_trace = 0;
#endif
#endif
int box64_x11threads = 0;
int box64_x11glx = 1;
int allow_missing_libs = 0;
int box64_prefer_emulated = 0;
int box64_prefer_wrapped = 0;
int box64_sse_flushto0 = 0;
int box64_x87_no80bits = 0;
int fix_64bit_inodes = 0;
int box64_dummy_crashhandler = 1;
int box64_mapclean = 0;
int box64_zoom = 0;
int box64_steam = 0;
int box64_wine = 0;
int box64_musl = 0;
int box64_nopulse = 0;
int box64_nogtk = 0;
int box64_novulkan = 0;
int box64_showsegv = 0;
int box64_showbt = 0;
int box64_isglibc234 = 0;
char* box64_libGL = NULL;
char* box64_custom_gstreamer = NULL;
uintptr_t fmod_smc_start = 0;
uintptr_t fmod_smc_end = 0;
uint32_t default_gs = 0x53;
int jit_gdb = 0;
int box64_tcmalloc_minimal = 0;

FILE* ftrace = NULL;
char* ftrace_name = NULL;
int ftrace_has_pid = 0;

void openFTrace(const char* newtrace)
{
    const char* t = newtrace?newtrace:getenv("BOX64_TRACE_FILE");
    char tmp[500];
    char tmp2[500];
    const char* p = t;
    int append=0;
    if(p && strlen(p) && p[strlen(p)-1]=='+') {
        strncat(tmp2, t, 499);
        tmp2[strlen(p)-1]='\0';
        p = tmp2;
        append=1;
    }
    if(p && strstr(t, "%pid")) {
        int next = 0;
        if(!append) do {
            strcpy(tmp, p);
            char* c = strstr(tmp, "%pid");
            *c = 0; // cut
            char pid[16];
            if(next)
                sprintf(pid, "%d-%d", getpid(), next);
            else
                sprintf(pid, "%d", getpid());
            strcat(tmp, pid);
            c = strstr(p, "%pid") + strlen("%pid");
            strcat(tmp, c);
            ++next;
        } while (FileExist(tmp, IS_FILE));
        p = tmp;
        ftrace_has_pid = 1;
    }
    if(ftrace_name)
        free(ftrace_name);
    ftrace_name = NULL;
    if(p) {
        if(!strcmp(p, "stderr"))
            ftrace = stderr;
        else {
            if(append)
                ftrace = fopen(p, "w+");
            else
                ftrace = fopen(p, "w");
            if(!ftrace) {
                ftrace = stdout;
                printf_log(LOG_INFO, "Cannot open trace file \"%s\" for writing (error=%s)\n", p, strerror(errno));
            } else {
                ftrace_name = strdup(p);
                /*fclose(ftrace);
                ftrace = NULL;*/
                if(!box64_nobanner)
                    printf("BOX64 Trace %s to \"%s\"\n", append?"appended":"redirected", p);
            }
        }
    }
}

void printf_ftrace(const char* fmt, ...)
{
    if(ftrace_name) {
        int fd = fileno(ftrace);
        if(fd<0 || lseek(fd, 0, SEEK_CUR)==(off_t)-1)
            ftrace=fopen(ftrace_name, "w+");
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(ftrace, fmt, args);

    fflush(ftrace);

    va_end(args);
}

void my_child_fork()
{
    if(ftrace_has_pid) {
        // open a new ftrace...
        if(!ftrace_name)
            fclose(ftrace);
        openFTrace(NULL);
    }
}

const char* getCpuName();
int getNCpu();
#ifdef DYNAREC
void GatherDynarecExtensions()
{
    if(box64_dynarec==0)    // no need to check if no dynarec
        return;
#ifdef ARM64
/*
HWCAP_FP
    Functionality implied by ID_AA64PFR0_EL1.FP == 0b0000.
HWCAP_ASIMD
    Functionality implied by ID_AA64PFR0_EL1.AdvSIMD == 0b0000.
HWCAP_EVTSTRM
    The generic timer is configured to generate events at a frequency of
    approximately 10KHz.
HWCAP_AES
    Functionality implied by ID_AA64ISAR0_EL1.AES == 0b0001.
HWCAP_PMULL
    Functionality implied by ID_AA64ISAR0_EL1.AES == 0b0010.
HWCAP_SHA1
    Functionality implied by ID_AA64ISAR0_EL1.SHA1 == 0b0001.
HWCAP_SHA2
    Functionality implied by ID_AA64ISAR0_EL1.SHA2 == 0b0001.
HWCAP_CRC32
    Functionality implied by ID_AA64ISAR0_EL1.CRC32 == 0b0001.
HWCAP_ATOMICS
    Functionality implied by ID_AA64ISAR0_EL1.Atomic == 0b0010.
HWCAP_FPHP
    Functionality implied by ID_AA64PFR0_EL1.FP == 0b0001.
HWCAP_ASIMDHP
    Functionality implied by ID_AA64PFR0_EL1.AdvSIMD == 0b0001.
HWCAP_CPUID
    EL0 access to certain ID registers is available.
    These ID registers may imply the availability of features.
HWCAP_ASIMDRDM
    Functionality implied by ID_AA64ISAR0_EL1.RDM == 0b0001.
HWCAP_JSCVT
    Functionality implied by ID_AA64ISAR1_EL1.JSCVT == 0b0001.
HWCAP_FCMA
    Functionality implied by ID_AA64ISAR1_EL1.FCMA == 0b0001.
HWCAP_LRCPC
    Functionality implied by ID_AA64ISAR1_EL1.LRCPC == 0b0001.
HWCAP_DCPOP
    Functionality implied by ID_AA64ISAR1_EL1.DPB == 0b0001.
HWCAP_SHA3
    Functionality implied by ID_AA64ISAR0_EL1.SHA3 == 0b0001.
HWCAP_SM3
    Functionality implied by ID_AA64ISAR0_EL1.SM3 == 0b0001.
HWCAP_SM4
    Functionality implied by ID_AA64ISAR0_EL1.SM4 == 0b0001.
HWCAP_ASIMDDP
    Functionality implied by ID_AA64ISAR0_EL1.DP == 0b0001.
HWCAP_SHA512
    Functionality implied by ID_AA64ISAR0_EL1.SHA2 == 0b0010.
HWCAP_SVE
    Functionality implied by ID_AA64PFR0_EL1.SVE == 0b0001.
HWCAP_ASIMDFHM
   Functionality implied by ID_AA64ISAR0_EL1.FHM == 0b0001.
HWCAP_DIT
    Functionality implied by ID_AA64PFR0_EL1.DIT == 0b0001.
HWCAP_USCAT
    Functionality implied by ID_AA64MMFR2_EL1.AT == 0b0001.
HWCAP_ILRCPC
    Functionality implied by ID_AA64ISAR1_EL1.LRCPC == 0b0010.
HWCAP_FLAGM
    Functionality implied by ID_AA64ISAR0_EL1.TS == 0b0001.
HWCAP_SSBS
    Functionality implied by ID_AA64PFR1_EL1.SSBS == 0b0010.
HWCAP_SB
    Functionality implied by ID_AA64ISAR1_EL1.SB == 0b0001.
HWCAP_PACA
    Functionality implied by ID_AA64ISAR1_EL1.APA == 0b0001 or
    ID_AA64ISAR1_EL1.API == 0b0001.
HWCAP_PACG
    Functionality implied by ID_AA64ISAR1_EL1.GPA == 0b0001 or
    ID_AA64ISAR1_EL1.GPI == 0b0001.
HWCAP2_DCPODP
    Functionality implied by ID_AA64ISAR1_EL1.DPB == 0b0010.
HWCAP2_SVE2
    Functionality implied by ID_AA64ZFR0_EL1.SVEVer == 0b0001.
HWCAP2_SVEAES
    Functionality implied by ID_AA64ZFR0_EL1.AES == 0b0001.
HWCAP2_SVEPMULL
    Functionality implied by ID_AA64ZFR0_EL1.AES == 0b0010.
HWCAP2_SVEBITPERM
    Functionality implied by ID_AA64ZFR0_EL1.BitPerm == 0b0001.
HWCAP2_SVESHA3
    Functionality implied by ID_AA64ZFR0_EL1.SHA3 == 0b0001.
HWCAP2_SVESM4
    Functionality implied by ID_AA64ZFR0_EL1.SM4 == 0b0001.
HWCAP2_FLAGM2
    Functionality implied by ID_AA64ISAR0_EL1.TS == 0b0010.
HWCAP2_FRINT
    Functionality implied by ID_AA64ISAR1_EL1.FRINTTS == 0b0001.
HWCAP2_SVEI8MM
    Functionality implied by ID_AA64ZFR0_EL1.I8MM == 0b0001.
HWCAP2_SVEF32MM
    Functionality implied by ID_AA64ZFR0_EL1.F32MM == 0b0001.
HWCAP2_SVEF64MM
    Functionality implied by ID_AA64ZFR0_EL1.F64MM == 0b0001.
HWCAP2_SVEBF16
    Functionality implied by ID_AA64ZFR0_EL1.BF16 == 0b0001.
HWCAP2_I8MM
    Functionality implied by ID_AA64ISAR1_EL1.I8MM == 0b0001.
HWCAP2_BF16
    Functionality implied by ID_AA64ISAR1_EL1.BF16 == 0b0001.
HWCAP2_DGH
    Functionality implied by ID_AA64ISAR1_EL1.DGH == 0b0001.
HWCAP2_RNG
    Functionality implied by ID_AA64ISAR0_EL1.RNDR == 0b0001.
HWCAP2_BTI
    Functionality implied by ID_AA64PFR0_EL1.BT == 0b0001.
HWCAP2_MTE
    Functionality implied by ID_AA64PFR1_EL1.MTE == 0b0010.
HWCAP2_ECV
    Functionality implied by ID_AA64MMFR0_EL1.ECV == 0b0001.
*/
    unsigned long hwcap = real_getauxval(AT_HWCAP);
    if(!hwcap)  // no HWCap: provide a default...
        hwcap = HWCAP_ASIMD;
    // first, check all needed extensions, lif half, edsp and fastmult
    if((hwcap&HWCAP_ASIMD) == 0) {
        printf_log(LOG_INFO, "Missing ASMID cpu support, disabling Dynarec\n");
        box64_dynarec=0;
        return;
    }
    if(hwcap&HWCAP_CRC32)
        arm64_crc32 = 1;
    if(hwcap&HWCAP_PMULL)
        arm64_pmull = 1;
    if(hwcap&HWCAP_AES)
        arm64_aes = 1;
    if(hwcap&HWCAP_ATOMICS)
        arm64_atomics = 1;
    printf_log(LOG_INFO, "Dynarec for ARM64, with extension: ASIMD");
    if(arm64_aes)
        printf_log(LOG_INFO, " AES");
    if(arm64_crc32)
        printf_log(LOG_INFO, " CRC32");
    if(arm64_pmull)
        printf_log(LOG_INFO, " PMULL");
    if(arm64_atomics)
        printf_log(LOG_INFO, " ATOMICS");
    printf_log(LOG_INFO, " PageSize:%zd ", box64_pagesize);
#elif defined(LA464)
    printf_log(LOG_INFO, "Dynarec for LoongArch");
    printf_log(LOG_INFO, " PageSize:%zd ", box64_pagesize);
#elif defined(RV64)
    void RV64_Detect_Function();
    if(!getenv("BOX64_DYNAREC_RV64NOEXT"))
        RV64_Detect_Function();
    printf_log(LOG_INFO, "Dynarec for RISC-V ");
    printf_log(LOG_INFO, "With extension: I M A F D C");
    if(rv64_zba) printf_log(LOG_INFO, " Zba");
    if(rv64_zbb) printf_log(LOG_INFO, " Zbb");
    if(rv64_zbc) printf_log(LOG_INFO, " Zbc");
    if(rv64_zbs) printf_log(LOG_INFO, " Zbs");
    if(rv64_xtheadba) printf_log(LOG_INFO, " XTheadBa");
    if(rv64_xtheadbb) printf_log(LOG_INFO, " XTheadBb");
    if(rv64_xtheadbs) printf_log(LOG_INFO, " XTheadBs");
    if(rv64_xtheadcondmov) printf_log(LOG_INFO, " XTheadCondMov");
    if(rv64_xtheadmemidx) printf_log(LOG_INFO, " XTheadMemIdx");
    if(rv64_xtheadmempair) printf_log(LOG_INFO, " XTheadMemPair");
    if(rv64_xtheadfmemidx) printf_log(LOG_INFO, " XTheadFMemIdx");
    if(rv64_xtheadmac) printf_log(LOG_INFO, " XTheadMac");
    if(rv64_xtheadfmv) printf_log(LOG_INFO, " XTheadFmv");

    printf_log(LOG_INFO, " PageSize:%zd ", box64_pagesize);
#else
#error Unsupported architecture
#endif
}
#endif


EXPORTDYN
void LoadLogEnv()
{
    ftrace = stdout;
    box64_nobanner = isatty(fileno(stdout))?0:1;
    const char *p = getenv("BOX64_NOBANNER");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box64_nobanner = p[0]-'0';
        }
    }
    // grab BOX64_TRACE_FILE envvar, and change %pid to actual pid is present in the name
    openFTrace(NULL);
    box64_log = ftrace_name?LOG_INFO:(isatty(fileno(stdout))?LOG_INFO:LOG_NONE); //default LOG value different if stdout is redirected or not
    p = getenv("BOX64_LOG");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0'+LOG_NONE && p[0]<='0'+LOG_NEVER) {
                box64_log = p[0]-'0';
                if(box64_log == LOG_NEVER) {
                    --box64_log;
                    box64_dump = 1;
                }
            }
        } else {
            if(!strcasecmp(p, "NONE"))
                box64_log = LOG_NONE;
            else if(!strcasecmp(p, "INFO"))
                box64_log = LOG_INFO;
            else if(!strcasecmp(p, "DEBUG"))
                box64_log = LOG_DEBUG;
            else if(!strcasecmp(p, "DUMP")) {
                box64_log = LOG_DEBUG;
                box64_dump = 1;
            }
        }
        if(!box64_nobanner)
            printf_log(LOG_INFO, "Debug level is %d\n", box64_log);
    }
    p = getenv("BOX64_ROLLING_LOG");
    if(p) {
        int cycle = 0;
        if(sscanf(p, "%d", &cycle)==1)
                cycle_log = cycle;
        if(cycle_log==1)
            cycle_log = 16;
        if(cycle_log<0)
            cycle_log = 0;
        if(cycle_log && box64_log>LOG_INFO) {
            cycle_log = 0;
            printf_log(LOG_NONE, "Incompatible Rolling log and Debug Log, disabling Rolling log\n");
        }
    }
    if(!box64_nobanner && cycle_log)
        printf_log(LOG_INFO, "Rolling log, showing last %d function call on signals\n", cycle_log);
    p = getenv("BOX64_DUMP");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box64_dump = p[0]-'0';
        }
    }
    if(!box64_nobanner && box64_dump)
        printf_log(LOG_INFO, "Elf Dump if ON\n");
#ifdef DYNAREC
    p = getenv("BOX64_DYNAREC_DUMP");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='2')
                box64_dynarec_dump = p[0]-'0';
        }
        if (box64_dynarec_dump) printf_log(LOG_INFO, "Dynarec blocks are dumped%s\n", (box64_dynarec_dump>1)?" in color":"");
    }
    p = getenv("BOX64_DYNAREC_LOG");
    if(p) {
        if(strlen(p)==1) {
            if((p[0]>='0'+LOG_NONE) && (p[0]<='0'+LOG_NEVER))
                box64_dynarec_log = p[0]-'0';
        } else {
            if(!strcasecmp(p, "NONE"))
                box64_dynarec_log = LOG_NONE;
            else if(!strcasecmp(p, "INFO"))
                box64_dynarec_log = LOG_INFO;
            else if(!strcasecmp(p, "DEBUG"))
                box64_dynarec_log = LOG_DEBUG;
            else if(!strcasecmp(p, "VERBOSE"))
                box64_dynarec_log = LOG_VERBOSE;
        }
        printf_log(LOG_INFO, "Dynarec log level is %d\n", box64_dynarec_log);
    }
    p = getenv("BOX64_DYNAREC");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box64_dynarec = p[0]-'0';
        }
        printf_log(LOG_INFO, "Dynarec is %s\n", box64_dynarec?"on":"off");
    }
    p = getenv("BOX64_DYNAREC_FORCED");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box64_dynarec_forced = p[0]-'0';
        }
        if(box64_dynarec_forced)
            printf_log(LOG_INFO, "Dynarec is forced on all addresses\n");
    }
    p = getenv("BOX64_DYNAREC_BIGBLOCK");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='3')
                box64_dynarec_bigblock = p[0]-'0';
        }
        if(!box64_dynarec_bigblock)
            printf_log(LOG_INFO, "Dynarec will not try to make big block\n");
        else if (box64_dynarec_bigblock>1)
            printf_log(LOG_INFO, "Dynarec will try to make bigger blocks%s\n", (box64_dynarec_bigblock>2)?" even on non-elf memory":"");

    }
    p = getenv("BOX64_DYNAREC_FORWARD");
    if(p) {
        int val = -1;
        if(sscanf(p, "%d", &val)==1) {
            if(val>=0)
                box64_dynarec_forward = val;
        }
        if(box64_dynarec_forward)
            printf_log(LOG_INFO, "Dynarec will continue block for %d bytes on forward jump\n", box64_dynarec_forward);
        else
            printf_log(LOG_INFO, "Dynarec will not continue block on forward jump\n");
    }
    p = getenv("BOX64_DYNAREC_STRONGMEM");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='2')
                box64_dynarec_strongmem = p[0]-'0';
        }
        if(box64_dynarec_strongmem)
            printf_log(LOG_INFO, "Dynarec will try to emulate a strong memory model%s\n", (box64_dynarec_strongmem==1)?" with limited performance loss":"");
    }
    p = getenv("BOX64_DYNAREC_X87DOUBLE");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box64_dynarec_x87double = p[0]-'0';
        }
        if(box64_dynarec_x87double)
            printf_log(LOG_INFO, "Dynarec will use only double for x87 emulation\n");
    }
    p = getenv("BOX64_DYNAREC_FASTNAN");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box64_dynarec_fastnan = p[0]-'0';
        }
        if(!box64_dynarec_fastnan)
            printf_log(LOG_INFO, "Dynarec will try to normalize generated NAN\n");
    }
    p = getenv("BOX64_DYNAREC_FASTROUND");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box64_dynarec_fastround = p[0]-'0';
        }
        if(!box64_dynarec_fastround)
            printf_log(LOG_INFO, "Dynarec will try tp generate x86 precise IEEE->int rounding\n");
    }
    p = getenv("BOX64_DYNAREC_SAFEFLAGS");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='2')
                box64_dynarec_safeflags = p[0]-'0';
        }
        if(!box64_dynarec_safeflags)
            printf_log(LOG_INFO, "Dynarec will not play it safe with x64 flags\n");
        else
            printf_log(LOG_INFO, "Dynarec will play %s safe with x64 flags\n", (box64_dynarec_safeflags==1)?"moderatly":"it");
    }
    p = getenv("BOX64_DYNAREC_CALLRET");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box64_dynarec_callret = p[0]-'0';
        }
        if(box64_dynarec_callret)
            printf_log(LOG_INFO, "Dynarec will optimize CALL/RET\n");
    }
    p = getenv("BOX64_DYNAREC_BLEEDING_EDGE");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box64_dynarec_bleeding_edge = p[0]-'0';
        }
        if(!box64_dynarec_bleeding_edge)
            printf_log(LOG_INFO, "Dynarec will not detect MonoBleedingEdge\n");
    }
    p = getenv("BOX64_DYNAREC_JVM");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box64_dynarec_jvm = p[0]-'0';
        }
        if(!box64_dynarec_jvm)
            printf_log(LOG_INFO, "Dynarec will not detect libjvm\n");
    }
    p = getenv("BOX64_DYNAREC_WAIT");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box64_dynarec_wait = p[0]-'0';
        }
        if(!box64_dynarec_wait)
            printf_log(LOG_INFO, "Dynarec will not wait for FillBlock to ready and use Interpreter instead\n");
    }
    p = getenv("BOX64_DYNAREC_HOTPAGE");
    if(p) {
        int val = -1;
        if(sscanf(p, "%d", &val)==1) {
            if(val>=0)
                box64_dynarec_hotpage = val;
        }
        if(box64_dynarec_hotpage)
            printf_log(LOG_INFO, "Dynarec will have HotPage tagged for %d attempts\n", box64_dynarec_hotpage);
        else
            printf_log(LOG_INFO, "Dynarec will not tag HotPage\n");
    }
    p = getenv("BOX64_DYNAREC_FASTPAGE");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box64_dynarec_fastpage = p[0]-'0';
        }
        if(box64_dynarec_fastpage)
            printf_log(LOG_INFO, "Dynarec will use Fast HotPage\n");
    }
    p = getenv("BOX64_DYNAREC_MISSING");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box64_dynarec_missing = p[0]-'0';
        }
        if(box64_dynarec_missing)
            printf_log(LOG_INFO, "Dynarec will print missing opcodes\n");
    }
    p = getenv("BOX64_NODYNAREC");
    if(p) {
        if (strchr(p,'-')) {
            if(sscanf(p, "%ld-%ld", &box64_nodynarec_start, &box64_nodynarec_end)!=2) {
                if(sscanf(p, "0x%lX-0x%lX", &box64_nodynarec_start, &box64_nodynarec_end)!=2)
                    sscanf(p, "%lx-%lx", &box64_nodynarec_start, &box64_nodynarec_end);
            }
            printf_log(LOG_INFO, "No dynablock creation that start in the range %p - %p\n", (void*)box64_nodynarec_start, (void*)box64_nodynarec_end);
        }
    }
    p = getenv("BOX64_DYNAREC_TEST");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box64_dynarec_test = p[0]-'0';
        }
        if(box64_dynarec_test) {
            box64_dynarec_fastnan = 0;
            box64_dynarec_fastround = 0;
            box64_dynarec_callret = 0;
            printf_log(LOG_INFO, "Dynarec will compare it's execution with the interpreter (super slow, only for testing)\n");
        }
    }

#endif
#ifdef HAVE_TRACE
    p = getenv("BOX64_TRACE_XMM");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                trace_xmm = p[0]-'0';
        }
    }
    p = getenv("BOX64_TRACE_EMM");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                trace_emm = p[0]-'0';
        }
    }
    p = getenv("BOX64_TRACE_COLOR");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                trace_regsdiff = p[0]-'0';
        }
    }
    p = getenv("BOX64_TRACE_START");
    if(p) {
        char* p2;
        start_cnt = strtoll(p, &p2, 10);
        printf_log(LOG_INFO, "Will start trace only after %lu instructions\n", start_cnt);
    }
#ifdef DYNAREC
    p = getenv("BOX64_DYNAREC_TRACE");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                box64_dynarec_trace = p[0]-'0';
            if(box64_dynarec_trace)
                printf_log(LOG_INFO, "Dynarec generated code will also print a trace\n");
        }
    }
#endif
#endif
    // Other BOX64 env. var.
    p = getenv("BOX64_LIBCEF");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box64_libcef = p[0]-'0';
        }
        if(!box64_libcef)
            printf_log(LOG_INFO, "Dynarec will not detect libcef\n");
    }
    p = getenv("BOX64_LOAD_ADDR");
    if(p) {
        if(sscanf(p, "0x%zx", &box64_load_addr)!=1)
            box64_load_addr = 0;
        if(box64_load_addr)
            printf_log(LOG_INFO, "Use a starting load address of %p\n", (void*)box64_load_addr);
    }
    p = getenv("BOX64_DLSYM_ERROR");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                dlsym_error = p[0]-'0';
        }
    }
    p = getenv("BOX64_X11THREADS");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                box64_x11threads = p[0]-'0';
        }
        if(box64_x11threads)
            printf_log(LOG_INFO, "Try to Call XInitThreads if libX11 is loaded\n");
    }
    p = getenv("BOX64_X11GLX");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                box64_x11glx = p[0]-'0';
        }
        if(box64_x11glx)
            printf_log(LOG_INFO, "Hack to force libX11 GLX extension present\n");
        else
            printf_log(LOG_INFO, "Disabled Hack to force libX11 GLX extension present\n");
    }
    p = getenv("BOX64_LIBGL");
    if(p)
        box64_libGL = box_strdup(p);
    if(!box64_libGL) {
        p = getenv("SDL_VIDEO_GL_DRIVER");
        if(p)
            box64_libGL = box_strdup(p);
    }
    if(box64_libGL) {
        printf_log(LOG_INFO, "BOX64 using \"%s\" as libGL.so.1\n", p);
    }
    p = getenv("BOX64_ALLOWMISSINGLIBS");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                allow_missing_libs = p[0]-'0';
        }
        if(allow_missing_libs)
            printf_log(LOG_INFO, "Allow missing needed libs\n");
    }
    p = getenv("BOX64_CRASHHANDLER");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                box64_dummy_crashhandler = p[0]-'0';
        }
        if(!box64_dummy_crashhandler)
            printf_log(LOG_INFO, "Don't use dummy crashhandler lib\n");
    }
    p = getenv("BOX64_MALLOC_HACK");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+2)
                box64_malloc_hack = p[0]-'0';
        }
        if(!box64_malloc_hack) {
            if(box64_malloc_hack==1) {
                printf_log(LOG_INFO, "Malloc hook will not be redirected\n");
            } else
                printf_log(LOG_INFO, "Malloc hook will check for mmap/free occurrences\n");
        }
    }
    p = getenv("BOX64_NOPULSE");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                box64_nopulse = p[0]-'0';
        }
        if(box64_nopulse)
            printf_log(LOG_INFO, "Disable the use of pulseaudio libs\n");
    }
    p = getenv("BOX64_NOGTK");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                box64_nogtk = p[0]-'0';
        }
        if(box64_nogtk)
            printf_log(LOG_INFO, "Disable the use of wrapped gtk libs\n");
    }
    p = getenv("BOX64_NOVULKAN");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                box64_novulkan = p[0]-'0';
        }
        if(box64_novulkan)
            printf_log(LOG_INFO, "Disable the use of wrapped vulkan libs\n");
    }
    p = getenv("BOX64_FIX_64BIT_INODES");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                fix_64bit_inodes = p[0]-'0';
        }
        if(fix_64bit_inodes)
            printf_log(LOG_INFO, "Fix 64bit inodes\n");
    }
    p = getenv("BOX64_JITGDB");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+2)
                jit_gdb = p[0]-'0';
        }
        if(jit_gdb)
            printf_log(LOG_INFO, "Launch %s on segfault\n", (jit_gdb==2)?"gdbserver":"gdb");
    }
    p = getenv("BOX64_SHOWSEGV");
        if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                box64_showsegv = p[0]-'0';
        }
        if(box64_showsegv)
            printf_log(LOG_INFO, "Show Segfault signal even if a signal handler is present\n");
    }
    p = getenv("BOX64_SHOWBT");
        if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                box64_showbt = p[0]-'0';
        }
        if(box64_showbt)
            printf_log(LOG_INFO, "Show a Backtrace when a Segfault signal is caught\n");
    }
    box64_pagesize = sysconf(_SC_PAGESIZE);
    if(!box64_pagesize)
        box64_pagesize = 4096;
#ifdef DYNAREC
    GatherDynarecExtensions();
#endif
    int ncpu = getNCpu();
    const char* cpuname = getCpuName();
    printf_log(LOG_INFO, "Running on %s with %d Cores\n", cpuname, ncpu);
}

EXPORTDYN
void LoadEnvPath(path_collection_t *col, const char* defpath, const char* env)
{
    const char* p = getenv(env);
    if(p) {
        ParseList(p, col, 1);
    } else {
        ParseList(defpath, col, 1);
    }
}

void PrintCollection(path_collection_t* col, const char* env)
{
    if(LOG_INFO<=box64_log) {
        printf_log(LOG_INFO, "%s: ", env);
        for(int i=0; i<col->size; i++)
            printf_log(LOG_INFO, "%s%s", col->paths[i], (i==col->size-1)?"\n":":");
    }
}

EXPORTDYN
int CountEnv(char** env)
{
    // count, but remove all BOX64_* environnement
    // also remove PATH and LD_LIBRARY_PATH
    // but add 2 for default BOX64_PATH and BOX64_LD_LIBRARY_PATH
    char** p = env;
    int c = 0;
    while(*p) {
        if(strncmp(*p, "BOX64_", 6)!=0)
            //if(!(strncmp(*p, "PATH=", 5)==0 || strncmp(*p, "LD_LIBRARY_PATH=", 16)==0))
                ++c;
        ++p;
    }
    return c+2;
}
EXPORTDYN
int GatherEnv(char*** dest, char** env, char* prog)
{
    // Add all but BOX64_* environnement
    // but add 2 for default BOX64_PATH and BOX64_LD_LIBRARY_PATH
    char** p = env;
    int idx = 0;
    int path = 0;
    int ld_path = 0;
    while(*p) {
        if(strncmp(*p, "BOX64_PATH=", 11)==0) {
            (*dest)[idx++] = box_strdup(*p+6);
            path = 1;
        } else if(strncmp(*p, "BOX64_LD_LIBRARY_PATH=", 22)==0) {
            (*dest)[idx++] = box_strdup(*p+6);
            ld_path = 1;
        } else if(strncmp(*p, "_=", 2)==0) {
            /*int l = strlen(prog);
            char tmp[l+3];
            strcpy(tmp, "_=");
            strcat(tmp, prog);
            (*dest)[idx++] = box_strdup(tmp);*/
        } else if(strncmp(*p, "BOX64_", 6)!=0) {
            (*dest)[idx++] = box_strdup(*p);
            /*if(!(strncmp(*p, "PATH=", 5)==0 || strncmp(*p, "LD_LIBRARY_PATH=", 16)==0)) {
            }*/
        }
        ++p;
    }
    // update the calloc of envv when adding new variables here
    if(!path) {
        (*dest)[idx++] = box_strdup("BOX64_PATH=.:bin");
    }
    if(!ld_path) {
        (*dest)[idx++] = box_strdup("BOX64_LD_LIBRARY_PATH=.:lib:lib64:x86_64:bin64:libs64");
    }
    // add "_=prog" at the end...
    if(prog) {
        int l = strlen(prog);
        char tmp[l+3];
        strcpy(tmp, "_=");
        strcat(tmp, prog);
        (*dest)[idx++] = box_strdup(tmp);
    }
    // and a final NULL
    (*dest)[idx++] = 0;
    return 0;
}


void PrintHelp() {
    printf("\n\nThis is Box64, the Linux x86_64 emulator with a twist\n");
    printf("\nUsage is box64 [options] path/to/software [args]\n");
    printf("to launch x86_64 software\n");
    printf(" options can be :\n");
    printf("    '-v'|'--version' to print box64 version and quit\n");
    printf("    '-h'|'--help'    to print box64 help and quit\n");
    printf("You can also set some environment variables:\n");
    printf(" BOX64_PATH is the box64 version of PATH (default is '.:bin')\n");
    printf(" BOX64_LD_LIBRARY_PATH is the box64 version LD_LIBRARY_PATH (default is '.:lib:lib64')\n");
    printf(" BOX64_LOG with 0/1/2/3 or NONE/INFO/DEBUG/DUMP to set the printed debug info (level 3 is level 2 + BOX64_DUMP)\n");
    printf(" BOX64_DUMP with 0/1 to dump elf infos\n");
    printf(" BOX64_NOBANNER with 0/1 to enable/disable the printing of box64 version and build at start\n");
#ifdef DYNAREC
    printf(" BOX64_DYNAREC_LOG with 0/1/2/3 or NONE/INFO/DEBUG/DUMP to set the printed dynarec info\n");
    printf(" BOX64_DYNAREC with 0/1 to disable or enable Dynarec (On by default)\n");
    printf(" BOX64_NODYNAREC with address interval (0x1234-0x4567) to forbid dynablock creation in the interval specified\n");
#endif
#ifdef HAVE_TRACE
    printf(" BOX64_TRACE with 1 to enable x86_64 execution trace\n");
    printf("    or with XXXXXX-YYYYYY to enable x86_64 execution trace only between address\n");
    printf("    or with FunctionName to enable x86_64 execution trace only in one specific function\n");
    printf("  use BOX64_TRACE_INIT instead of BOX_TRACE to start trace before init of Libs and main program\n\t (function name will probably not work then)\n");
    printf(" BOX64_TRACE_EMM with 1 to enable dump of MMX registers along with regular registers\n");
    printf(" BOX64_TRACE_XMM with 1 to enable dump of SSE registers along with regular registers\n");
    printf(" BOX64_TRACE_COLOR with 1 to enable detection of changed general register values\n");
    printf(" BOX64_TRACE_START with N to enable trace after N instructions\n");
#ifdef DYNAREC
    printf(" BOX64_DYNAREC_TRACE with 0/1 to disable or enable Trace on generated code too\n");
#endif
#endif
    printf(" BOX64_TRACE_FILE with FileName to redirect logs in a file (or stderr to use stderr instead of stdout)");
    printf(" BOX64_DLSYM_ERROR with 1 to log dlsym errors\n");
    printf(" BOX64_LOAD_ADDR=0xXXXXXX try to load at 0xXXXXXX main binary (if binary is a PIE)\n");
    printf(" BOX64_NOSIGSEGV=1 to disable handling of SigSEGV\n");
    printf(" BOX64_NOSIGILL=1  to disable handling of SigILL\n");
    printf(" BOX64_SHOWSEGV=1 to show Segfault signal even if a signal handler is present\n");
    printf(" BOX64_X11THREADS=1 to call XInitThreads when loading X11 (for old Loki games with Loki_Compat lib)");
    printf(" BOX64_LIBGL=libXXXX set the name (and optionnally full path) for libGL.so.1\n");
    printf(" BOX64_LD_PRELOAD=XXXX[:YYYYY] force loading XXXX (and YYYY...) libraries with the binary\n");
    printf(" BOX64_ALLOWMISSINGLIBS with 1 to allow one to continue even if a lib is missing (unadvised, will probably  crash later)\n");
    printf(" BOX64_PREFER_EMULATED=1 to prefer emulated libs first (execpt for glibc, alsa, pulse, GL, vulkan and X11\n");
    printf(" BOX64_PREFER_WRAPPED if box64 will use wrapped libs even if the lib is specified with absolute path\n");
    printf(" BOX64_CRASHHANDLER=0 to not use a dummy crashhandler lib\n");
    printf(" BOX64_NOPULSE=1 to disable the loading of pulseaudio libs\n");
    printf(" BOX64_NOGTK=1 to disable the loading of wrapped gtk libs\n");
    printf(" BOX64_NOVULKAN=1 to disable the loading of wrapped vulkan libs\n");
    printf(" BOX64_ENV='XXX=yyyy' will add XXX=yyyy env. var.\n");
    printf(" BOX64_ENV1='XXX=yyyy' will add XXX=yyyy env. var. and continue with BOX86_ENV2 ... until var doesn't exist\n");
    printf(" BOX64_JITGDB with 1 to launch \"gdb\" when a segfault is trapped, attached to the offending process\n");
}

void addNewEnvVar(const char* s)
{
    if(!s)
        return;
    char* p = box_strdup(s);
    char* e = strchr(p, '=');
    if(!e) {
        printf_log(LOG_INFO, "Invalid specific env. var. '%s'\n", s);
        box_free(p);
        return;
    }
    *e='\0';
    ++e;
    setenv(p, e, 1);
    box_free(p);
}

EXPORTDYN
void LoadEnvVars(box64context_t *context)
{
    // Check custom env. var. and add them if needed
    {
        char* p = getenv("BOX64_ENV");
        if(p)
            addNewEnvVar(p);
        int i = 1;
        char box64_env[50];
        do {
            sprintf(box64_env, "BOX64_ENV%d", i);
            p = getenv(box64_env);
            if(p) {
                addNewEnvVar(p);
                ++i;
            }
        } while(p);
    }
    // check BOX64_LD_LIBRARY_PATH and load it
    LoadEnvPath(&context->box64_ld_lib, ".:lib:lib64:x86_64:bin64:libs64", "BOX64_LD_LIBRARY_PATH");
    if(FileExist("/lib/x86_64-linux-gnu", 0))
        AddPath("/lib/x86_64-linux-gnu", &context->box64_ld_lib, 1);
    if(FileExist("/usr/lib/x86_64-linux-gnu", 0))
        AddPath("/usr/lib/x86_64-linux-gnu", &context->box64_ld_lib, 1);
    if(FileExist("/usr/x86_64-linux-gnu/lib", 0))
        AddPath("/usr/x86_64-linux-gnu/lib", &context->box64_ld_lib, 1);
    if(getenv("LD_LIBRARY_PATH"))
        PrependList(&context->box64_ld_lib, getenv("LD_LIBRARY_PATH"), 1);   // in case some of the path are for x86 world
    if(getenv("BOX64_EMULATED_LIBS")) {
        char* p = getenv("BOX64_EMULATED_LIBS");
        ParseList(p, &context->box64_emulated_libs, 0);
        if (my_context->box64_emulated_libs.size && box64_log) {
            printf_log(LOG_INFO, "BOX64 will force the used of emulated libs for ");
            for (int i=0; i<context->box64_emulated_libs.size; ++i)
                printf_log(LOG_INFO, "%s ", context->box64_emulated_libs.paths[i]);
            printf_log(LOG_INFO, "\n");
        }
    }
    // add libssl and libcrypto, prefer emulated version because of multiple version exist
    AddPath("libssl.so.1", &context->box64_emulated_libs, 0);
    AddPath("libssl.so.1.0.0", &context->box64_emulated_libs, 0);
    AddPath("libcrypto.so.1", &context->box64_emulated_libs, 0);
    AddPath("libcrypto.so.1.0.0", &context->box64_emulated_libs, 0);
    AddPath("libunwind.so.8", &context->box64_emulated_libs, 0);
    AddPath("libpng12.so.0", &context->box64_emulated_libs, 0);

    if(getenv("BOX64_SSE_FLUSHTO0")) {
        if (strcmp(getenv("BOX64_SSE_FLUSHTO0"), "1")==0) {
            box64_sse_flushto0 = 1;
            printf_log(LOG_INFO, "BOX64: Direct apply of SSE Flush to 0 flag\n");
    	}
    }
    if(getenv("BOX64_X87_NO80BITS")) {
        if (strcmp(getenv("BOX64_X87_NO80BITS"), "1")==0) {
            box64_x87_no80bits = 1;
            printf_log(LOG_INFO, "BOX64: all 80bits x87 long double will be handle as double\n");
    	}
    }
    if(getenv("BOX64_PREFER_WRAPPED")) {
        if (strcmp(getenv("BOX64_PREFER_WRAPPED"), "1")==0) {
            box64_prefer_wrapped = 1;
            printf_log(LOG_INFO, "BOX64: Prefer Wrapped libs\n");
    	}
    }
    if(getenv("BOX64_PREFER_EMULATED")) {
        if (strcmp(getenv("BOX64_PREFER_EMULATED"), "1")==0) {
            box64_prefer_emulated = 1;
            printf_log(LOG_INFO, "BOX64: Prefer Emulated libs\n");
    	}
    }

    if(getenv("BOX64_NOSIGSEGV")) {
        if (strcmp(getenv("BOX64_NOSIGSEGV"), "1")==0) {
            context->no_sigsegv = 1;
            printf_log(LOG_INFO, "BOX64: Disabling handling of SigSEGV\n");
        }
    }
    if(getenv("BOX64_NOSIGILL")) {
        if (strcmp(getenv("BOX64_NOSIGILL"), "1")==0) {
            context->no_sigill = 1;
            printf_log(LOG_INFO, "BOX64: Disabling handling of SigILL\n");
        }
    }
    // check BOX64_PATH and load it
    LoadEnvPath(&context->box64_path, ".:bin", "BOX64_PATH");
    if(getenv("PATH"))
        AppendList(&context->box64_path, getenv("PATH"), 1);   // in case some of the path are for x86 world
#ifdef HAVE_TRACE
    char* p = getenv("BOX64_TRACE");
    if(p) {
        if (strcmp(p, "0")) {
            context->x64trace = 1;
            box64_trace = p;
        }
    }
    p = getenv("BOX64_TRACE_INIT");
    if(p) {
        if (strcmp(p, "0")) {
            context->x64trace = 1;
            trace_init = p;
        }
    }
    if(my_context->x64trace) {
        printf_log(LOG_INFO, "Initializing Zydis lib\n");
        if(InitX64Trace(my_context)) {
            printf_log(LOG_INFO, "Zydis init failed, no x86 trace activated\n");
            context->x64trace = 0;
        }
    }
#endif
}

EXPORTDYN
void setupTraceInit()
{
#ifdef HAVE_TRACE
    char* p = trace_init;
    if(p) {
        setbuf(stdout, NULL);
        uintptr_t s_trace_start=0, s_trace_end=0;
        if (strcmp(p, "1")==0)
            SetTraceEmu(0, 0);
        else if (strchr(p,'-')) {
            if(sscanf(p, "%ld-%ld", &s_trace_start, &s_trace_end)!=2) {
                if(sscanf(p, "0x%lX-0x%lX", &s_trace_start, &s_trace_end)!=2)
                    sscanf(p, "%lx-%lx", &s_trace_start, &s_trace_end);
            }
            if(s_trace_start || s_trace_end)
                SetTraceEmu(s_trace_start, s_trace_end);
        } else {
            if (GetGlobalSymbolStartEnd(my_context->maplib, p, &s_trace_start, &s_trace_end, NULL, -1, NULL, NULL, NULL)) {
                SetTraceEmu(s_trace_start, s_trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)s_trace_start, (void*)s_trace_end);
            } else if(GetLocalSymbolStartEnd(my_context->maplib, p, &s_trace_start, &s_trace_end, NULL, -1, NULL, NULL, NULL)) {
                SetTraceEmu(s_trace_start, s_trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)s_trace_start, (void*)s_trace_end);
            } else {
                printf_log(LOG_NONE, "Warning, symbol to trace (\"%s\") not found, disabling trace\n", p);
                SetTraceEmu(0, 100);  // disabling trace, mostly
            }
        }
    } else {
        p = box64_trace;
        if(p)
            if (strcmp(p, "0"))
                SetTraceEmu(0, 1);
    }
#endif
}

EXPORTDYN
void setupTrace()
{
#ifdef HAVE_TRACE
    char* p = box64_trace;
    if(p) {
        setbuf(stdout, NULL);
        uintptr_t s_trace_start=0, s_trace_end=0;
        if (strcmp(p, "1")==0)
            SetTraceEmu(0, 0);
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
            if (GetGlobalSymbolStartEnd(my_context->maplib, p, &s_trace_start, &s_trace_end, NULL, -1, NULL, NULL, NULL)) {
                SetTraceEmu(s_trace_start, s_trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)s_trace_start, (void*)s_trace_end);
            } else if(GetLocalSymbolStartEnd(my_context->maplib, p, &s_trace_start, &s_trace_end, NULL, -1, NULL, NULL, NULL)) {
                SetTraceEmu(s_trace_start, s_trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)s_trace_start, (void*)s_trace_end);
            } else {
                printf_log(LOG_NONE, "Warning, symbol to trace (\"%s\") not found, trying to set trace later\n", p);
                SetTraceEmu(0, 1);  // disabling trace, mostly
                if(trace_func)
                    box_free(trace_func);
                trace_func = box_strdup(p);
            }
        }
    }
#endif
}
void endMallocHook();

void endBox64()
{
    if(!my_context || box64_quit)
        return;

    endMallocHook();
    x64emu_t* emu = thread_get_emu();
    // atexit first
    printf_log(LOG_DEBUG, "Calling atexit registered functions (exiting box64)\n");
    CallAllCleanup(emu);
    // then call all the fini
    box64_quit = 1;
    printf_log(LOG_DEBUG, "Calling fini for all loaded elfs and unload native libs\n");
    RunElfFini(my_context->elfs[0], emu);
    FreeLibrarian(&my_context->local_maplib, emu);    // unload all libs
    FreeLibrarian(&my_context->maplib, emu);    // unload all libs
    #if 0
    // waiting for all thread except this one to finish
    int this_thread = GetTID();
    int pid = getpid();
    int running = 1;
    int attempt = 0;
    printf_log(LOG_DEBUG, "Waiting for all threads to finish before unloading box64context\n");
    while(running) {
        DIR *proc_dir;
        char dirname[100];
        snprintf(dirname, sizeof dirname, "/proc/self/task");
        proc_dir = opendir(dirname);
        running = 0;
        if (proc_dir)
        {
            struct dirent *entry;
            while ((entry = readdir(proc_dir)) != NULL && !running)
            {
                if(entry->d_name[0] == '.')
                    continue;

                int tid = atoi(entry->d_name);
                // tid != pthread_t, so no pthread functions are available here
                if(tid!=this_thread) {
                    if(attempt>4000) {
                        printf_log(LOG_INFO, "Stop waiting for remaining thread %04d\n", tid);
                        // enough wait, kill all thread!
                        syscall(__NR_tgkill, pid, tid, SIGABRT);
                    } else {
                        running = 1;
                        ++attempt;
                        sched_yield();
                    }
                }
            }
            closedir(proc_dir);
        }
    }
    #endif
    // all done, free context
    FreeBox64Context(&my_context);
    #ifdef DYNAREC
    // disable dynarec now
    box64_dynarec = 0;
    #endif
    if(box64_libGL) {
        box_free(box64_libGL);
        box64_libGL = NULL;
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

static void load_rcfiles()
{
    if(FileExist("/etc/box64.box64rc", IS_FILE))
        LoadRCFile("/etc/box64.box64rc");
    else
        LoadRCFile(NULL);   // load default rcfile
    char* p = getenv("HOME");
    if(p) {
        char tmp[4096];
        strncpy(tmp, p, 4095);
        strncat(tmp, "/.box64rc", 4095);
        if(FileExist(tmp, IS_FILE))
            LoadRCFile(tmp);
    }
}

void pressure_vessel(int argc, const char** argv, int nextarg, const char* prog);
extern char** environ;
int main(int argc, const char **argv, char **env) {
    init_malloc_hook();
    init_auxval(argc, argv, environ?environ:env);
    // trying to open and load 1st arg
    if(argc==1) {
        PrintBox64Version();
        PrintHelp();
        return 1;
    }
    if(argc>1 && !strcmp(argv[1], "/usr/bin/gdb") && getenv("BOX64_TRACE_FILE"))
        exit(0);
    // uname -m is redirected to box64 -m
    if(argc==2 && (!strcmp(argv[1], "-m") || !strcmp(argv[1], "-p") || !strcmp(argv[1], "-i")))
    {
        printf("x86_64\n");
        exit(0);
    }

    // check BOX64_LOG debug level
    LoadLogEnv();
    if(!getenv("BOX64_NORCFILES")) {
        load_rcfiles();
    }
    char* bashpath = NULL;
    {
        char* p = getenv("BOX64_BASH");
        if(p) {
            if(FileIsX64ELF(p)) {
                bashpath = p;
                printf_log(LOG_INFO, "Using bash \"%s\"\n", bashpath);
            } else {
                printf_log(LOG_INFO, "the x86_64 bash \"%s\" is not an x86_64 binary\n", p);
            }
        }
    }

    const char* prog = argv[1];
    int nextarg = 1;
    // check if some options are passed
    while(prog && prog[0]=='-') {
        if(!strcmp(prog, "-v") || !strcmp(prog, "--version")) {
            PrintBox64Version();
            exit(0);
        }
        if(!strcmp(prog, "-h") || !strcmp(prog, "--help")) {
            PrintHelp();
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
        printf("Box64: nothing to run\n");
        exit(0);
    }
    if(!box64_nobanner)
        PrintBox64Version();
    // precheck, for win-preload
    if(strstr(prog, "wine-preloader")==(prog+strlen(prog)-strlen("wine-preloader"))
     || strstr(prog, "wine64-preloader")==(prog+strlen(prog)-strlen("wine64-preloader"))) {
        // wine-preloader detecter, skipping it if next arg exist and is an x86 binary
        int x64 = (nextarg<argc)?FileIsX64ELF(argv[nextarg]):0;
        if(x64) {
            prog = argv[++nextarg];
            printf_log(LOG_INFO, "BOX64: Wine preloader detected, loading \"%s\" directly\n", prog);
            //wine_preloaded = 1;
        }
    }
    #if 1
    // pre-check for pressure-vessel-wrap
    if(strstr(prog, "pressure-vessel-wrap")==(prog+strlen(prog)-strlen("pressure-vessel-wrap"))) {
        printf_log(LOG_INFO, "BOX64: pressure-vessel-wrap detected\n");
        pressure_vessel(argc, argv, nextarg+1, prog);
    }
    #endif
    int ld_libs_args = -1;
    int is_custom_gstreamer = 0;
    // check if this is wine
    if(!strcmp(prog, "wine64")
     || !strcmp(prog, "wine64-development") 
     || !strcmp(prog, "wine") 
     || (strrchr(prog, '/') && !strcmp(strrchr(prog,'/'), "/wine"))
     || (strrchr(prog, '/') && !strcmp(strrchr(prog,'/'), "/wine64"))) {
        const char* prereserve = getenv("WINEPRELOADRESERVE");
        printf_log(LOG_INFO, "BOX64: Wine64 detected, WINEPRELOADRESERVE=\"%s\"\n", prereserve?prereserve:"");
        if(wine_preloaded)
            wine_prereserve(prereserve);
        // special case for winedbg, doesn't work anyway
        if(argv[nextarg+1] && strstr(argv[nextarg+1], "winedbg")==argv[nextarg+1]) {
            if(getenv("BOX64_WINEDBG")) {
                box64_nobanner = 1;
                box64_log = 0;
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
                //printf_log(LOG_INFO, "BOX64: Custom gstreamer detected, disable gtk wrapping\n");
                //box64_nogtk = 1;
                //is_custom_gstreamer = 1;
                box64_custom_gstreamer = box_strdup(tmp);
            }
        }
    } else if(strstr(prog, "ld-musl-x86_64.so.1")) {
    // check if ld-musl-x86_64.so.1 is used
        printf_log(LOG_INFO, "BOX64: ld-musl detected, trying to workaround and use system ld-linux\n");
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
    }
    // check if this is wineserver
    if(!strcmp(prog, "wineserver") || !strcmp(prog, "wineserver64") || (strlen(prog)>9 && !strcmp(prog+strlen(prog)-strlen("/wineserver"), "/wineserver"))) {
        box64_wine = 1;
    }
    if(box64_wine) {
        // disabling the use of futex_waitv for now
        setenv("WINEFSYNC", "0", 1);
    }
    // Create a new context
    my_context = NewBox64Context(argc - nextarg);

    // check BOX64_LD_LIBRARY_PATH and load it
    LoadEnvVars(my_context);
    // Append ld_list if it exist
    if(ld_libs_args!=-1)
        PrependList(&my_context->box64_ld_lib, argv[ld_libs_args], 1);
    if(is_custom_gstreamer)
        AddPath("libwayland-client.so.0", &my_context->box64_emulated_libs, 0);

    my_context->box64path = ResolveFile(argv[0], &my_context->box64_path);
    // prepare all other env. var
    my_context->envc = CountEnv(environ?environ:env);
    printf_log(LOG_INFO, "Counted %d Env var\n", my_context->envc);
    // allocate extra space for new environment variables such as BOX64_PATH
    my_context->envv = (char**)box_calloc(my_context->envc+4, sizeof(char*));
    GatherEnv(&my_context->envv, environ?environ:env, my_context->box64path);
    if(box64_dump || box64_log<=LOG_DEBUG) {
        for (int i=0; i<my_context->envc; ++i)
            printf_dump(LOG_DEBUG, " Env[%02d]: %s\n", i, my_context->envv[i]);
    }

    path_collection_t ld_preload = {0};
    if(getenv("BOX64_LD_PRELOAD")) {
        char* p = getenv("BOX64_LD_PRELOAD");
        ParseList(p, &ld_preload, 0);
        if (ld_preload.size && box64_log) {
            printf_log(LOG_INFO, "BOX64 try to Preload ");
            for (int i=0; i<ld_preload.size; ++i)
                printf_log(LOG_INFO, "%s ", ld_preload.paths[i]);
            printf_log(LOG_INFO, "\n");
        }
    } else {
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
            ParseList(p, &ld_preload, 0);
            if (ld_preload.size && box64_log) {
                printf_log(LOG_INFO, "BOX64 try to Preload ");
                for (int i=0; i<ld_preload.size; ++i)
                    printf_log(LOG_INFO, "%s ", ld_preload.paths[i]);
                printf_log(LOG_INFO, "\n");
            }
        }
    }
    // print PATH and LD_LIB used
    PrintCollection(&my_context->box64_ld_lib, "BOX64 LIB PATH");
    PrintCollection(&my_context->box64_path, "BOX64 BIN PATH");
    // lets build argc/argv stuff
    printf_log(LOG_INFO, "Looking for %s\n", prog);
    my_context->argv[0] = ResolveFile(prog, &my_context->box64_path);
    // check if box86 is present
    {
        my_context->box86path = box_strdup(my_context->box64path);
        char* p = strrchr(my_context->box86path, '6');  // get the 6 of box64
        p[0] = '8'; p[1] = '6'; // change 64 to 86
        if(!FileExist(my_context->box86path, IS_FILE)) {
            box_free(my_context->box86path);
            my_context->box86path = NULL;
        }
    }
    const char* prgname = strrchr(prog, '/');
    if(!prgname)
        prgname = prog;
    else
        ++prgname;
    if(box64_wine) {
        #ifdef ANDROID
            AddPath("libdl.so", &ld_preload, 0);
        #else
            AddPath("libdl.so.2", &ld_preload, 0);
        #endif
    }
    // special case for zoom
    if(strstr(prgname, "zoom")==prgname) {
        printf_log(LOG_INFO, "Zoom detected, trying to use system libturbojpeg if possible\n");
        box64_zoom = 1;
    }
    // special case for bash (add BOX86_NOBANNER=1 if not there)
    if(!strcmp(prgname, "bash")) {
        printf_log(LOG_INFO, "bash detected, disabling banner\n");
        if (!box64_nobanner) {
            setenv("BOX86_NOBANNER", "1", 0);
            setenv("BOX64_NOBANNER", "1", 0);
        }
        if (!bashpath) {
            bashpath = (char*)prog;
            setenv("BOX64_BASH", prog, 1);
        }
    }
    if(bashpath)
        my_context->bashpath = box_strdup(bashpath);

    /*if(strstr(prgname, "awesomium_process")==prgname) {
        printf_log(LOG_INFO, "awesomium_process detected, forcing emulated libpng12\n");
        AddPath("libpng12.so.0", &my_context->box64_emulated_libs, 0);
    }*/
    /*if(!strcmp(prgname, "gdb")) {
        exit(-1);
    }*/
    ApplyParams("*");   // [*] is a special setting for all process
    ApplyParams(prgname);

    for(int i=1; i<my_context->argc; ++i) {
        my_context->argv[i] = box_strdup(argv[i+nextarg]);
        printf_log(LOG_INFO, "argv[%i]=\"%s\"\n", i, my_context->argv[i]);
    }
    if(box64_nosandbox)
    {
        // check if sandbox is already there
        int there = 0;
        for(int i=1; i<my_context->argc && !there; ++i)
            if(!strcmp(my_context->argv[i], "--no-sandbox"))
                there = 1;
        if(!there) {
            my_context->argv = (char**)box_realloc(my_context->argv, (my_context->argc+1)*sizeof(char*));
            my_context->argv[my_context->argc] = box_strdup("--no-sandbox");
            my_context->argc++;
        }
    }

    // check if file exist
    if(!my_context->argv[0] || !FileExist(my_context->argv[0], IS_FILE)) {
        printf_log(LOG_NONE, "Error: file is not found (check BOX64_PATH)\n");
        free_contextargv();
        FreeBox64Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    if(!FileExist(my_context->argv[0], IS_FILE|IS_EXECUTABLE)) {
        printf_log(LOG_NONE, "Error: %s is not an executable file\n", my_context->argv[0]);
        free_contextargv();
        FreeBox64Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    if(!(my_context->fullpath = box_realpath(my_context->argv[0], NULL)))
        my_context->fullpath = box_strdup(my_context->argv[0]);
    if(getenv("BOX64_ARG0"))
        my_context->argv[0] = box_strdup(getenv("BOX64_ARG0"));
    FILE *f = fopen(my_context->fullpath, "rb");
    if(!f) {
        printf_log(LOG_NONE, "Error: Cannot open %s\n", my_context->fullpath);
        free_contextargv();
        FreeBox64Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    elfheader_t *elf_header = LoadAndCheckElfHeader(f, my_context->fullpath, 1);
    if(!elf_header) {
        int x86 = my_context->box86path?FileIsX86ELF(my_context->fullpath):0;
        int script = my_context->bashpath?FileIsShell(my_context->fullpath):0;
        printf_log(LOG_NONE, "Error: reading elf header of %s, try to launch %s instead\n", my_context->fullpath, x86?"using box86":(script?"using bash":"natively"));
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

    if(CalcLoadAddr(elf_header)) {
        printf_log(LOG_NONE, "Error: reading elf header of %s\n", my_context->fullpath);
        fclose(f);
        free_contextargv();
        FreeBox64Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    // allocate memory
    if(AllocElfMemory(my_context, elf_header, 1)) {
        printf_log(LOG_NONE, "Error: allocating memory for elf %s\n", my_context->fullpath);
        fclose(f);
        free_contextargv();
        FreeBox64Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    // Load elf into memory
    if(LoadElfMemory(f, my_context, elf_header)) {
        printf_log(LOG_NONE, "Error: loading in memory elf %s\n", my_context->fullpath);
        fclose(f);
        free_contextargv();
        FreeBox64Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    // can close the file now
    fclose(f);
    if(ElfCheckIfUseTCMallocMinimal(elf_header)) {
        if(!box64_tcmalloc_minimal) {
            // need to reload with tcmalloc_minimal as a LD_PRELOAD!
            printf_log(LOG_INFO, "BOX64: tcmalloc_minimal.so.4 used, reloading box64 with the lib preladed\n");
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
                printf_log(LOG_NONE, "Failed to relaunch, error is %d/%s\n", errno, strerror(errno));
        } else {
            printf_log(LOG_INFO, "BOX64: Using tcmalloc_minimal.so.4, and it's in the LD_PRELOAD command\n");
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
    }
    box64_isglibc234 = GetNeededVersionForLib(elf_header, "libc.so.6", "GLIBC_2.34");
    if(box64_isglibc234)
        printf_log(LOG_DEBUG, "Program linked with GLIBC 2.34+\n");
    // get and alloc stack size and align
    if(CalcStackSize(my_context)) {
        printf_log(LOG_NONE, "Error: allocating stack\n");
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
    SetRSI(emu, my_context->argc);
    SetRDX(emu, (uint64_t)my_context->argv);
    SetRCX(emu, (uint64_t)my_context->envv);
    SetRBP(emu, 0); // Frame pointer so to "No more frame pointer"

    // child fork to handle traces
    pthread_atfork(NULL, NULL, my_child_fork);

    thread_set_emu(emu);

    // export symbols
    AddSymbols(my_context->maplib, GetMapSymbols(elf_header), GetWeakSymbols(elf_header), GetLocalSymbols(elf_header), elf_header);
    if(wine_preloaded) {
        uintptr_t wineinfo = FindSymbol(GetMapSymbols(elf_header), "wine_main_preload_info", -1, NULL, 1, NULL);
        if(!wineinfo) wineinfo = FindSymbol(GetWeakSymbols(elf_header), "wine_main_preload_info", -1, NULL, 1, NULL);
        if(!wineinfo) wineinfo = FindSymbol(GetLocalSymbols(elf_header), "wine_main_preload_info", -1, NULL, 1, NULL);
        if(!wineinfo) {printf_log(LOG_NONE, "Warning, Symbol wine_main_preload_info not found\n");}
        else {
            *(void**)wineinfo = get_wine_prereserve();
            printf_log(LOG_DEBUG, "WINE wine_main_preload_info found and updated\n");
        }
        #ifdef DYNAREC
        dynarec_wine_prereserve();
        #endif
    }
    AddMainElfToLinkmap(elf_header);
    // pre-load lib if needed
    if(ld_preload.size) {
        my_context->preload = new_neededlib(0);
        for(int i=0; i<ld_preload.size; ++i) {
            needed_libs_t* tmp = new_neededlib(1);
            tmp->names[0] = ld_preload.paths[i];
            if(AddNeededLib(my_context->maplib, 0, 0, tmp, elf_header, my_context, emu)) {
                printf_log(LOG_INFO, "Warning, cannot pre-load of %s\n", tmp->names[0]);
                RemoveNeededLib(my_context->maplib, 0, tmp, my_context, emu);
            } else {
                for(int j=0; j<tmp->size; ++j)
                    add1lib_neededlib(my_context->preload, tmp->libs[j], tmp->names[j]);
                free_neededlib(tmp);
            }
        }
    }
    FreeCollection(&ld_preload);
    // Call librarian to load all dependant elf
    if(LoadNeededLibs(elf_header, my_context->maplib, 0, 0, my_context, emu)) {
        printf_log(LOG_NONE, "Error: loading needed libs in elf %s\n", my_context->argv[0]);
        FreeBox64Context(&my_context);
        return -1;
    }
    // reloc...
    printf_log(LOG_DEBUG, "And now export symbols / relocation for %s...\n", ElfName(elf_header));
    if(RelocateElf(my_context->maplib, NULL, 0, elf_header)) {
        printf_log(LOG_NONE, "Error: relocating symbols in elf %s\n", my_context->argv[0]);
        FreeBox64Context(&my_context);
        return -1;
    }
    // and handle PLT
    RelocateElfPlt(my_context->maplib, NULL, 0, elf_header);
    // deferred init
    setupTraceInit();
    RunDeferredElfInit(emu);
    // update TLS of main elf
    RefreshElfTLS(elf_header);
    // do some special case check, _IO_2_1_stderr_ and friends, that are setup by libc, but it's already done here, so need to do a copy
    ResetSpecialCaseMainElf(elf_header);
    // init...
    setupTrace();
    // get entrypoint
    my_context->ep = GetEntryPoint(my_context->maplib, elf_header);

    atexit(endBox64);
    loadProtectionFromMap();

    // emulate!
    printf_log(LOG_DEBUG, "Start x64emu on Main\n");
    // Stack is ready, with stacked: NULL env NULL argv argc
    SetRIP(emu, my_context->ep);
    ResetFlags(emu);
    Push64(emu, my_context->exit_bridge);  // push to pop it just after
    SetRDX(emu, Pop64(emu));    // RDX is exit function
    Run(emu, 0);
    // Get EAX
    int ret = GetEAX(emu);
    printf_log(LOG_DEBUG, "Emulation finished, EAX=%d\n", ret);

#ifdef HAVE_TRACE
    if(trace_func)  {
        box_free(trace_func);
        trace_func = NULL;
    }
#endif

    return ret;
}
