#include <winternl.h>
#include <stdio.h>

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

    // Read from the registry to get the rest, consider as a success even if it fails
    HKEY key;
    ULONGLONG value;
    DWORD size = sizeof(value);
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &key))
        return 1;

    if (RegQueryValueExA(key, "CP 4030" /* ID_AA64ISAR0_EL1 */, NULL, NULL, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
        // TS, bits[55:52]
        cpuext.flagm = !!((value >> 52) & 0x1);
        cpuext.flagm2 = !!((value >> 53) & 0x1);
        // PRINTTS, bits[35:32]
        cpuext.frintts = !!((value >> 32) & 0x1);
        // RND, bits[63:60]
        cpuext.rndr = !!((value >> 60) & 0x1);
    }
    size = sizeof(value);
    if (RegQueryValueExA(key, "CP 4039" /* ID_AA64MMFR1_EL1 */, NULL, NULL, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
        // AFP, bits[47:44]
        cpuext.afp = !!((value >> 44) & 0x1);
    }
    size = sizeof(value);
    if (RegQueryValueExA(key, "CP 403A" /* ID_AA64MMFR2_EL1 */, NULL, NULL, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
        // AT, bits[35:32]
        cpuext.uscat = !!((value >> 32) & 0x1);
    }

    RegCloseKey(key);
    return 1;
}
