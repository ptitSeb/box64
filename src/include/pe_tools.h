#ifndef __PE_TOOLS_H__
#define __PE_TOOLS_H__

// Parse the PE file and extract the volatile metadata, which contains volatile opcode entries and volatile ranges.
void ParseVolatileMetadata(char* filename, void* addr);

// Check if a given address is contained within the volatile ranges.
int VolatileRangesContains(uintptr_t addr);

// Check if a given address is contained within the volatile opcode entries.
int VolatileOpcodesHas(uintptr_t addr);

#endif // __PE_TOOLS_H__
