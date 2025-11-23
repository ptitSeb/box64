#ifndef __FILEUTILS_H_
#define __FILEUTILS_H_

#include "pathcoll.h"

// find a file, using Path if needed, resolving symlinks
char* ResolveFile(const char* filename, path_collection_t* paths);
// find a file, using Path if needed, NOT resolving symlinks
char* ResolveFileSoft(const char* filename, path_collection_t* paths);

// 1: if file is an x86 elf, 0: if not (or not found)
int FileIsX86ELF(const char* filename);
int FileIsX64ELF(const char* filename);
int FileIsShell(const char* filename);
int FileIsPython(const char* filename);
size_t FileSize(const char* filename);

// return temp folder (will return /tmp if nothing is correct)
const char* GetTmpDir(void);

#if defined(RPI) || defined(RK3399) || defined(RK3326)
void sanitize_mojosetup_gtk_background(void);
#endif

#endif //__FILEUTILS_H_