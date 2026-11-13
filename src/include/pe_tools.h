// SPDX-License-Identifier: MIT
#ifndef __PE_TOOLS_H__
#define __PE_TOOLS_H__

// Parse the PE file and register its volatile metadata, which contains volatile opcode entries and volatile ranges.
void ParseVolatileMetadata(char* filename, void* addr);

#endif // __PE_TOOLS_H__
