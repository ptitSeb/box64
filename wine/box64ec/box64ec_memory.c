// SPDX-License-Identifier: MIT
#include <stdint.h>
#include <string.h>
#include "box64ec_private.h"
#include "wine/compiler.h"

int Box64EC_IsEcCode(uintptr_t address)
{
    uint64_t* map;
    uint64_t word;
    uintptr_t page;
    void* peb = *(void**)((char*)NtCurrentTeb() + 0x60);

    if (!peb || !address || address >= Box64EC_EcCodeBitmapLimit) {
        return 0;
    }
    map = *(uint64_t**)((char*)peb + 0x368);
    if (!map) {
        return 0;
    }
    page = address >> 12;
    word = map[page >> 6];
    {
        int result = (int)((word >> (page & 63)) & 1);
        return result;
    }
}
