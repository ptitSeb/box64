#ifndef __PERFMAP_H__
#define __PERFMAP_H__

void writePerfMap(uintptr_t func_addr, uintptr_t code_addr, size_t code_size, const char* inst_name);

#endif // __PERFMAP_H__