#include <dirent.h>
#include <stdio.h>
#include <limits.h>
#include <fileutils.h>
#include <dlfcn.h>

#include "elfloader.h"
#include "box64.h"
#include "core.h"

/**
 * box64.c contains all APIs.
*/

extern char** environ;

static const char* dummyX64Library() {
    // Box64 has not been initialized yet, so we cannot use printf_log(LOG_NONE, ...) now.
    // Instead using fprintf(stderr, ...) to print error messages.

    // Read the env BOX64_LD_LIBRARY_PATH.
    const char* ld_library_path = getenv("BOX64_LD_LIBRARY_PATH");
    if (!ld_library_path) {
        fprintf(stderr, "Error: BOX64_LD_LIBRARY_PATH environment variable is not set.\n");
        return NULL;
    }

    DIR* dir = opendir(ld_library_path);
    if (!dir) {
        fprintf(stderr, "Error: Failed to open directory %s\n", ld_library_path);
        return NULL;
    }

    struct dirent* entry;
    char selected_file[PATH_MAX] = {0};
    int selected_file_found = 0;

    // Loop through directory entries to find an x86_64 format .so* file
    while ((entry = readdir(dir)) != NULL) {
        snprintf(selected_file, sizeof(selected_file), "%s/%s", ld_library_path, entry->d_name);
        if (strstr(entry->d_name, ".so") && FileExist(selected_file, IS_FILE | IS_EXECUTABLE) && 0 == strcmp(entry->d_name, "libdummy.so")) {
            selected_file_found = 1;
            break;
        }
    }

    closedir(dir);

    if (!selected_file_found) {
        fprintf(stderr, "Error: No x86_64 format .so* files found in %s\n", ld_library_path);
        return NULL;
    }

    // Allocate memory for the full path and copy the selected file path
    char* full_path = (char*)malloc(strlen(selected_file) + 1);
    if (!full_path) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }

    strcpy(full_path, selected_file);
    return full_path;
}

EXPORT int Initialize() 
{
    const char* dummy_x64_library = dummyX64Library();
    if (!dummy_x64_library) {
        printf_log(LOG_NONE, "Dummy x64 library not found !");
        abort();
    }

    int argc = 2;
    const char* argv[] = {"box64", dummy_x64_library};

    x64emu_t* dummy_emu = NULL;
    elfheader_t* dummy_elf_header = NULL;
    if (initialize(argc, argv, environ, &dummy_emu, &dummy_elf_header, 0)) {
        return -1;
    }

    printf_log(LOG_DEBUG, "libbox64.so initialzied.\n");
    return 0;
}

EXPORT uintptr_t RunX64Function(const char *x64_libname, const char *funcname, int nargs, ...)
{
    // 1. Load the library.
    // Note: We choose binding all symbols when opening the library,
    // which might output some warnings like "Warning: Weak Symbol 
    // xxx not found ...", so we could know ahead which libraries 
    // need to wrap. If there are too many noisy noise warnings, 
    // we could choose to bind symbol on demand by using RTLD_LAZY 
    // flag when opening the library.
    void* lib_handle = my_dlopen(thread_get_emu(), x64_libname, RTLD_NOW);
    if (!lib_handle) {
        printf_log(LOG_NONE, "Load x64 library fail! x64_libname: %s. \n", x64_libname);
        abort();
    }

    // 2. Find the elf header of the library.
    library_t* lib = GetLibInternal(x64_libname);
    if (!lib) {
        printf_log(LOG_NONE, "X64 library %s not found in box64 context !\n", x64_libname);
        abort();
    }
    elfheader_t* elf_header = GetElf(lib);
    if (!elf_header) {
        printf_log(LOG_NONE, "Elf header of library %s not found in box64 context !\n", x64_libname);
        abort();
    }

    // 3. Call func in emulator.
    uintptr_t x64_symbol_addr = 0;
    int ver = -1, veropt = 0;
    const char* vername = NULL;
    if (!ElfGetGlobalSymbolStartEnd(elf_header, &x64_symbol_addr, NULL, funcname, &ver, &vername, 1, &veropt)) {
        printf_log(LOG_NONE, "Symbol %s not found in library %s !\n", funcname, x64_libname);
        abort();
    }

    va_list va;
    va_start(va, nargs);
    uint64_t ret = VRunFunctionWithEmu(thread_get_emu(), 0, x64_symbol_addr, nargs, va);
    va_end(va);
    return ret;
}