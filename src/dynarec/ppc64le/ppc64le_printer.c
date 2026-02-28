#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "ppc64le_printer.h"

// TODO: Full PPC64LE instruction disassembler
static char buff[200];

const char* ppc64le_print(uint32_t opcode, uint64_t addr)
{
    snprintf(buff, sizeof(buff), "%08X ???", opcode);
    return buff;
}
