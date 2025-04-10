#include "symbolfuncs.h"
#include "elfloader.h"
#include "debug.h"
#include "box64context.h"

int PrintFunctionAddr(uintptr_t nextaddr, const char* text)
{
    uint64_t sz = 0;
    uintptr_t start = 0;
    const char* symbname = FindNearestSymbolName(FindElfAddress(my_context, nextaddr), (void*)nextaddr, &start, &sz);
    if(!sz) sz=0x100;   // arbitrary value...
    if(symbname && nextaddr>=start && (nextaddr<(start+sz) || !sz)) {
        if(nextaddr==start)
            printf_log_prefix(0, LOG_NONE, " (%s%s:%s)", text, ElfName(FindElfAddress(my_context, nextaddr)), symbname);
        else
            printf_log_prefix(0, LOG_NONE, " (%s%s:%s + 0x%lx)", text, ElfName(FindElfAddress(my_context, nextaddr)), symbname, nextaddr - start);
        return 1;
    }
    return 0;
}
