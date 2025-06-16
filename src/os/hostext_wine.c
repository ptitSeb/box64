#include <winternl.h>

#include "debug.h"
#include "wine/compiler.h"
#include "wine/debug.h"

int DetectHostCpuFeatures(void)
{
    cpuext.asimd = 1;
    if (IsProcessorFeaturePresent(PF_ARM_V8_CRYPTO_INSTRUCTIONS_AVAILABLE))
        cpuext.aes = cpuext.sha1 = cpuext.sha2 = cpuext.pmull = 1;
    if (IsProcessorFeaturePresent(PF_ARM_V8_CRC32_INSTRUCTIONS_AVAILABLE))
        cpuext.crc32 = 1;
    if (IsProcessorFeaturePresent(PF_ARM_V81_ATOMIC_INSTRUCTIONS_AVAILABLE))
        cpuext.atomics = 1;

    // TODO
    cpuext.uscat = cpuext.flagm = cpuext.flagm2 = cpuext.frintts = cpuext.afp = cpuext.rndr = 0;
    return 1;
}
