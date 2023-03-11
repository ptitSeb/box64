#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "rv64_printer.h"
#include "debug.h"

const char* rv64_print(uint32_t opcode, uintptr_t addr)
{
    static char buff[200];

    snprintf(buff, sizeof(buff), "%08X ???", __builtin_bswap32(opcode));
    return buff;
}