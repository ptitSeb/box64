// SPDX-License-Identifier: MIT
#include <windows.h>
#include <winternl.h>

#include "os.h"
#include "wine/compiler.h"

BOOLEAN WINAPI BTCpu64IsProcessorFeaturePresent(UINT feature)
{
    return Box64WineIsProcessorFeaturePresent(feature);
}

void WINAPI UpdateProcessorInformation(SYSTEM_CPU_INFORMATION* info)
{
    Box64WineUpdateProcessorInformation(info, PROCESSOR_ARCHITECTURE_AMD64);
}
