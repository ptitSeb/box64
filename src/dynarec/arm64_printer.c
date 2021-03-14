#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "arm64_printer.h"

const char* arm64_print(uint32_t opcode)
{
    static char buff[200];

    snprintf(buff, sizeof(buff), "0x%8X ???", opcode);
    return buff;
}