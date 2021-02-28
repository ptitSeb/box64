#ifndef __FILEUTILS_H_
#define __FILEUTILS_H_

#include "pathcoll.h"

#define IS_EXECUTABLE   (1<<0)
#define IS_FILE         (1<<1)
       

// 0 : doesn't exist, 1: Does exist
int FileExist(const char* filename, int flags);

// find a file, using Path if needed
char* ResolveFile(const char* filename, path_collection_t* paths);

// 1: if file is an x86 elf, 0: if not (or not found)
int FileIsX64ELF(const char* filename);

#if defined(RPI) || defined(RK3399)
void sanitize_mojosetup_gtk_background();
#endif

#endif //__FILEUTILS_H_