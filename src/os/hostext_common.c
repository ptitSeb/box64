#include "debug.h"

#ifdef DYNAREC
void PrintHostCpuFeatures(void)
{
#ifdef ARM64
    printf_log(LOG_INFO, "Dynarec for ARM64, with extension: ASIMD");
    if(cpuext.aes)
        printf_log_prefix(0, LOG_INFO, " AES");
    if(cpuext.crc32)
        printf_log_prefix(0, LOG_INFO, " CRC32");
    if(cpuext.pmull)
        printf_log_prefix(0, LOG_INFO, " PMULL");
    if(cpuext.atomics)
        printf_log_prefix(0, LOG_INFO, " ATOMICS");
    if(cpuext.sha1)
        printf_log_prefix(0, LOG_INFO, " SHA1");
    if(cpuext.sha2)
        printf_log_prefix(0, LOG_INFO, " SHA2");
    if(cpuext.uscat)
        printf_log_prefix(0, LOG_INFO, " USCAT");
    if(cpuext.flagm)
        printf_log_prefix(0, LOG_INFO, " FLAGM");
    if(cpuext.flagm2)
        printf_log_prefix(0, LOG_INFO, " FLAGM2");
    if(cpuext.frintts)
        printf_log_prefix(0, LOG_INFO, " FRINT");
    if(cpuext.afp)
        printf_log_prefix(0, LOG_INFO, " AFP");
    if(cpuext.rndr)
        printf_log_prefix(0, LOG_INFO, " RNDR");
    printf_log_prefix(0, LOG_INFO, "\n");
#elif defined(LA64)
    printf_log(LOG_INFO, "Dynarec for LoongArch with extension LSX");
    if (cpuext.lasx)
        printf_log_prefix(0, LOG_INFO, " LASX");
    if (cpuext.lbt)
        printf_log_prefix(0, LOG_INFO, " LBT_X86");
    if (cpuext.lam_bh)
        printf_log_prefix(0, LOG_INFO, " LAM_BH");
    if (cpuext.lamcas)
        printf_log_prefix(0, LOG_INFO, " LAMCAS");
    if (cpuext.scq)
        printf_log_prefix(0, LOG_INFO, " SCQ");
    if (cpuext.frecipe)
        printf_log_prefix(0, LOG_INFO, " FRECIP");
    printf_log_prefix(0, LOG_INFO, "\n");
#elif defined(RV64)
    printf_log(LOG_INFO, "Dynarec for rv64g");
    if (cpuext.vector && !cpuext.xtheadvector) printf_log_prefix(0, LOG_INFO, "v");
    if (cpuext.zba) printf_log_prefix(0, LOG_INFO, "_zba");
    if (cpuext.zbb) printf_log_prefix(0, LOG_INFO, "_zbb");
    if (cpuext.zbc) printf_log_prefix(0, LOG_INFO, "_zbc");
    if (cpuext.zbs) printf_log_prefix(0, LOG_INFO, "_zbs");
    if (cpuext.vector && !cpuext.xtheadvector) printf_log_prefix(0, LOG_INFO, "_zvl%d", cpuext.vlen * 8);
    if (cpuext.xtheadba) printf_log_prefix(0, LOG_INFO, "_xtheadba");
    if (cpuext.xtheadbb) printf_log_prefix(0, LOG_INFO, "_xtheadbb");
    if (cpuext.xtheadbs) printf_log_prefix(0, LOG_INFO, "_xtheadbs");
    if (cpuext.xtheadmempair) printf_log_prefix(0, LOG_INFO, "_xtheadmempair");
    if (cpuext.xtheadcondmov) printf_log_prefix(0, LOG_INFO, "_xtheadcondmov");
    if (cpuext.xtheadmemidx) printf_log_prefix(0, LOG_INFO, "_xtheadmemidx");
    // Disable the display since these are only detected but never used.
    // if(cpuext.xtheadfmemidx) printf_log_prefix(0, LOG_INFO, " xtheadfmemidx");
    // if(cpuext.xtheadmac) printf_log_prefix(0, LOG_INFO, " xtheadmac");
    // if(cpuext.xtheadfmv) printf_log_prefix(0, LOG_INFO, " xtheadfmv");
    if (cpuext.xtheadvector) printf_log_prefix(0, LOG_INFO, "_xthvector");
    printf_log_prefix(0, LOG_INFO, "\n");
#endif
}
#endif