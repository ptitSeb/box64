#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <assert.h>

#include "debug.h"
#include "librarian.h"
#include "librarian_private.h"
#include "library.h"
#include "library_private.h"
#include "x64emu.h"
#include "box64context.h"
#include "elfloader.h"

#include "bridge.h"

KHASH_MAP_IMPL_INT(mapoffsets, cstr_t);

lib_t *NewLibrarian(box64context_t* context, int ownlibs)
{
    lib_t *maplib = (lib_t*)box_calloc(1, sizeof(lib_t));
    
    maplib->mapoffsets = kh_init(mapoffsets);
    maplib->globaldata = NewMapSymbols();
    maplib->bridge = NewBridge();

    maplib->ownlibs = ownlibs;

    return maplib;
}
static void freeLibraryRecurse(lib_t *maplib, x64emu_t *emu, int idx, char *freed, library_t* owner) {
    if (freed[idx]) return; // Already freed
    
    library_t *lib = maplib->libraries[idx];
    if(lib==owner) return;  // don't free owner of maplib
    freed[idx] = 1; // Avoid infinite loops
    printf_log(LOG_DEBUG, "Unloading %s\n", lib->name);
    for (int i = lib->dependedby.size - 1; i >= 0; --i) {
        int j;
        for (j = 0; j < maplib->libsz; ++j) {
            if (lib->dependedby.libs[i] == maplib->libraries[j]) break;
        }
        if (j == maplib->libsz) {
            // dependant lib already freed
            // library as been freed already
            continue;
        }
        if (freed[j] == 1) {
            printf_log(LOG_DEBUG, "Cyclic dependancy detected (cycle is between %s and %s)\n", lib->name, lib->dependedby.libs[i]->name);
            continue;
        }
        freeLibraryRecurse(maplib, emu, j, freed, owner);
        if (freed[idx] != 1) {
            printf_log(LOG_DEBUG, "Note: library already freed (cyclic dependancy break)\n");
            return;
        }
    }
    
    library_t *ptr = maplib->libraries[idx];
    if(maplib->ownlibs/* && (!maplib->owner || ptr->maplib==maplib)*/)
        Free1Library(&ptr, emu);
    freed[idx] = 2;
}
void FreeLibrarian(lib_t **maplib, x64emu_t *emu)
{
    // should that be in reverse order?
    if(!maplib || !*maplib)
        return;

    library_t* owner = (*maplib)->owner;
    (*maplib)->owner = NULL;    // to avoid recursive free...
    
    if((*maplib)->ownlibs && (*maplib)->libsz) {
        printf_log(LOG_DEBUG, "Closing %d libs from maplib %p\n", (*maplib)->libsz, *maplib);
        char *freed = (char*)box_calloc((*maplib)->libsz, sizeof(char));
        if (!freed) {
            printf_log(LOG_INFO, "Failed to malloc freed table, using old algorithm (a crash is likely)\n");
            for (int i=(*maplib)->libsz-1; i>=0; --i) 
                if((*maplib)->libraries[i]!=owner) {
                    printf_log(LOG_DEBUG, "Unloading %s\n", (*maplib)->libraries[i]->name);
                    Free1Library(&(*maplib)->libraries[i], emu);
                }
        } else {
            for (int i=(*maplib)->libsz-1; i>=0; --i) {
                freeLibraryRecurse(*maplib, emu, i, freed, owner);
            }
            memset((*maplib)->libraries, 0, (*maplib)->libsz*sizeof(library_t*)); // NULL = 0 anyway
            (*maplib)->libsz = 0;
            box_free(freed);
        }
    }
    box_free((*maplib)->libraries);
    (*maplib)->libraries = NULL;

    if((*maplib)->mapoffsets) {
        kh_destroy(mapoffsets, (*maplib)->mapoffsets);
    }
    FreeMapSymbols(&(*maplib)->globaldata);
    (*maplib)->libsz = (*maplib)->libcap = 0;

    if((*maplib)->bridge)
        FreeBridge(&(*maplib)->bridge);

    box_free(*maplib);
    *maplib = NULL;

}

box64context_t* GetLibrarianContext(lib_t* maplib)
{
    return my_context;
}

kh_mapsymbols_t* GetGlobalData(lib_t* maplib)
{
    return maplib->globaldata;
}

library_t* getLib(lib_t* maplib, const char* path)
{
    for(int i=0; i<maplib->libsz; ++i) {
        library_t *lib = maplib->libraries[i];
        if(IsSameLib(lib, path)) {
            return lib;
        }
    }
    return NULL;
}

static int libraryInMapLib(lib_t* maplib, library_t* lib)
{
    if(!maplib)
        return 0;
    for(int i=0; i<maplib->libsz; ++i)
        if(maplib->libraries[i]==lib)
            return 1;
    return 0;
}

void MapLibAddLib(lib_t* maplib, library_t* lib)
{
    if(libraryInMapLib(maplib, lib))
        return;
    if (maplib->libsz == maplib->libcap) {
        maplib->libcap += 8;
        maplib->libraries = (library_t**)box_realloc(maplib->libraries, maplib->libcap*sizeof(library_t*));
    }
    maplib->libraries[maplib->libsz] = lib;
    ++maplib->libsz;
}

static void MapLibRemoveLib(lib_t* maplib, library_t* lib);

static void MapLibAddMapLib(lib_t* dest, library_t* lib_src, lib_t* src)
{
    if(!src)
        return;
    library_t *owner = src->owner;
    for(int i=0; i<src->libsz; ++i) {
        library_t* lib = src->libraries[i];
        if(!lib || libraryInMapLib(dest, lib)) continue;
        MapLibAddLib(dest, lib);
        if(lib->maplib && src!=lib->maplib && dest!=lib->maplib) {
            MapLibAddMapLib(dest, lib, lib->maplib);
            MapLibRemoveLib(src, lib);
            if(lib->maplib)
                lib->maplib = (dest==my_context->maplib)?NULL:dest;
        }
    }
    if(lib_src == owner)
        FreeLibrarian(&src, NULL);
}

static void MapLibRemoveLib(lib_t* maplib, library_t* lib)
{
    if(!maplib || !lib)
        return;
    int idx = 0;
    while(idx<maplib->libsz && maplib->libraries[idx]!=lib) ++idx;
    if(idx==maplib->libsz)  //not found
        return;
    --maplib->libsz;
    if(idx!=(maplib->libsz))
        memmove(&maplib->libraries[idx], &maplib->libraries[idx+1], sizeof(library_t*)*(maplib->libsz-idx));
    maplib->libraries[maplib->libsz] = NULL;
}

static void MapLibRemoveMapLib(lib_t* dest, lib_t* src)
{
    if(!src)
        return;
    //library_t *owner = src->owner;
    for(int i=0; i<src->libsz; ++i) {
        library_t* lib = src->libraries[i];
        if(!lib || !libraryInMapLib(dest, lib)) continue;
        MapLibRemoveLib(dest, lib);
        if(lib->maplib && src!=lib->maplib && dest!=lib->maplib) {
            MapLibRemoveMapLib(dest, lib->maplib);
            if(lib->maplib && src==my_context->local_maplib)
                lib->maplib = NULL;
        }
    }
}

int AddNeededLib_add(lib_t* maplib, needed_libs_t* neededlibs, library_t* deplib, int local, const char* path, box64context_t* box64, x64emu_t* emu)
{
    printf_log(LOG_DEBUG, "Trying to add \"%s\" to maplib%s\n", path, local?" (local)":"");
    // first check if lib is already loaded
    library_t *lib = getLib(my_context->maplib, path);
    if(lib) {
        add_neededlib(neededlibs, lib);
        if (lib && deplib) add_dependedbylib(&lib->dependedby, deplib);
        printf_log(LOG_DEBUG, "Already present in maplib => success\n");
        return 0;
    }
    // check also in the local loaded lib
    lib = getLib(my_context->local_maplib, path);
    if(lib) {
        printf_log(LOG_DEBUG, "Already present in local_maplib => success\n");
        if(local) {
            // add lib to maplib...
            if(maplib) {
                if(lib->maplib) {
                    MapLibAddMapLib(maplib, lib, lib->maplib);
                }
                if(!libraryInMapLib(maplib, lib))
                    MapLibAddLib(maplib, lib);
                if(maplib->ownlibs)
                    MapLibRemoveMapLib(my_context->local_maplib, maplib);
            }
        } else {
            // promote lib from local to global...
            // for add the depending local libs...
            if(lib->maplib) {
                MapLibAddMapLib(my_context->maplib, lib, lib->maplib);
            }
            if(!libraryInMapLib(my_context->maplib, lib))
                MapLibAddLib(my_context->maplib, lib);
            MapLibRemoveMapLib(my_context->local_maplib, my_context->maplib);
        }
        add_neededlib(neededlibs, lib);
        if (lib && deplib) add_dependedbylib(&lib->dependedby, deplib);
        return 0;
    }
    // load a new one
    lib = NewLibrary(path, box64);
    if(!lib) {
        printf_log(LOG_DEBUG, "Faillure to create lib => fail\n");
        return 1;   //Error
    }

    add_neededlib(neededlibs, lib);
    if (lib && deplib) add_dependedbylib(&lib->dependedby, deplib);

    // add lib now
    if(local) {
        MapLibAddLib(my_context->local_maplib, lib);
        if(maplib) {
            MapLibAddLib(maplib, lib);
            if(!lib->maplib)
                lib->maplib = maplib;
        } else {
            lib->maplib = NewLibrarian(box64, 0);
            MapLibAddLib(lib->maplib, lib);
        }
    } else {
        MapLibAddLib(my_context->maplib, lib);
    }

    if(!maplib)
        maplib = (local)?lib->maplib:my_context->maplib;

    if(AddSymbolsLibrary(maplib, lib, emu)) {   // also add needed libs
        printf_log(LOG_DEBUG, "Failure to Add lib => fail\n");
        return 1;
    }

    if (lib->type == LIB_EMULATED) {
        // Need to add library to the linkmap (put here so the link is ordered)
        linkmap_t *lm = addLinkMapLib(lib);
        if(!lm) {
            // Crashed already
            printf_log(LOG_DEBUG, "Failure to add lib linkmap\n");
            return 1;
        }
        lm->l_addr = (Elf64_Addr)GetElfDelta(my_context->elfs[lib->e.elf_index]);
        lm->l_name = lib->name;
        lm->l_ld = GetDynamicSection(my_context->elfs[lib->e.elf_index]);
    }
    return 0;
}

int AddNeededLib_init(lib_t* maplib, needed_libs_t* neededlibs, library_t* deplib, int local, int bindnow, library_t* lib, box64context_t* box64, x64emu_t* emu)
{
    if(!maplib)
        maplib = (local)?lib->maplib:my_context->maplib;

    int mainelf = GetElfIndex(lib);

    if(mainelf==-1) {
        // It's a native libs, nothing else to do
    } else {
        // it's an emulated lib, 
        // load dependancies and launch init sequence
        if(LoadNeededLibs(box64->elfs[mainelf], maplib, &lib->needed, lib, 0, bindnow, box64, emu)) {
            printf_log(LOG_DEBUG, "Failure to Add dependant lib => fail\n");
            return 1;
        }
        // some special case, where dependancies may not be correct
        if(!strcmp(GetNameLib(lib), "libCgGL.so")) {
            const char* libs[] = {"libGL.so.1"};
            AddNeededLib(maplib, &lib->needed, lib, 0, 0, libs, 1, box64, emu);
        }
        if(!strcmp(GetNameLib(lib), "libmss.so.6")) {
            const char* libs[] = {"libSDL-1.2.so.0", "libdl.so.2"};
            AddNeededLib(maplib, &lib->needed, lib, 0, 0, libs, 2, box64, emu);
        }

        // finalize the lib
        if(FinalizeLibrary(lib, local?maplib:NULL, bindnow, emu)) {
            printf_log(LOG_DEBUG, "Failure to finalizing lib => fail\n");
            return 1;
        }
    }
    // success
    printf_log(LOG_DEBUG, "Created lib and added to maplib => success\n");
    
    return 0;
}

EXPORTDYN
int AddNeededLib(lib_t* maplib, needed_libs_t* neededlibs, library_t* deplib, int local, int bindnow, const char** paths, int npath, box64context_t* box64, x64emu_t* emu)
{
    box64_mapclean = 0;
    if(!neededlibs) {
        neededlibs = box_calloc(1, sizeof(needed_libs_t));
    }
    int idx = neededlibs->size;
    // Add libs and symbol
    for(int i=0; i<npath; ++i) {
        if(AddNeededLib_add(maplib, neededlibs, deplib, local, paths[i], box64, emu)) {
            printf_log(strchr(paths[i],'/')?LOG_DEBUG:LOG_INFO, "Error loading needed lib %s\n", paths[i]);
            return 1;
        }
    }
    int idx_end = neededlibs->size;
    // add dependant libs and init them
    for (int i=idx; i<idx_end; ++i)
        if(AddNeededLib_init(maplib, neededlibs, deplib, local, bindnow, neededlibs->libs[i], box64, emu)) {
            printf_log(LOG_INFO, "Error initializing needed lib %s\n", neededlibs->libs[i]->name);
            if(!allow_missing_libs) return 1;
        }
    return 0;
}

library_t* GetLibMapLib(lib_t* maplib, const char* name)
{
    printf_log(LOG_DEBUG, "Trying to Get \"%s\" to maplib\n", name);
    return getLib(maplib, name);
}

library_t* GetLibInternal(const char* name)
{
    printf_log(LOG_DEBUG, "Trying to Get \"%s\" to maplib\n", name);
    library_t* lib = getLib(my_context->maplib, name);
    if(!lib) lib = getLib(my_context->local_maplib, name);
    return lib;
}

EXPORTDYN
uintptr_t FindGlobalSymbol(lib_t *maplib, const char* name, int version, const char* vername)
{
    uintptr_t start = 0, end = 0;
    if(GetGlobalSymbolStartEnd(maplib, name, &start, &end, (void*)1, version, vername))
        return start;
    return 0;
}

static int isLocal(elfheader_t* self, library_t* l)
{
    if(GetElf(l)==self)
        return 1;
    return 0;
}

int GetNoSelfSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t* self, int version, const char* vername)
{
    assert(self);   // need self for this one
    //search for the self, to start "next"
    int go = -1;
    int weak = 0;
    const char* defver = GetDefaultVersion(my_context->globaldefver, name);
    for(int i=0; i<maplib->libsz && (go==-1); ++i) {
        if(GetElfIndex(maplib->libraries[i])!=-1 && (GetElf(maplib->libraries[i])==self))
            go = i+1;
    }
    if(go<0)
        go = 0; // not found...
    for(int i=go; i<maplib->libsz; ++i) {
        if(GetElfIndex(maplib->libraries[i])==-1 || (my_context->elfs[GetElfIndex(maplib->libraries[i])]!=self))
            if(GetLibGlobalSymbolStartEnd(maplib->libraries[i], name, start, end, &weak, version, vername, 0))
                if(*start)
                    return 1;
    }
    for(int i=go; i<maplib->libsz; ++i)
        if(GetElfIndex(maplib->libraries[i])==-1 || (my_context->elfs[GetElfIndex(maplib->libraries[i])]!=self))
            GetLibWeakSymbolStartEnd(maplib->libraries[i], name, start, end, &weak, version, vername, 0);
    // loop done, weak symbol found
    if(weak && *start)
        return 1;
    #if 0
    // if self defined, give it another chance with self...
    defver = GetDefaultVersion(my_context->weakdefver, name);
    if(self) {
        if(my_context->elfs[0]==self) {
            if(GetSymbolStartEnd(GetMapSymbols(my_context->elfs[0]), name, start, end, version, vername, 1, defver))
                if(*start)
                    return 1;
            if(GetSymbolStartEnd(GetWeakSymbols(my_context->elfs[0]), name, start, end, version, vername, 1, defver))
                if(*start)
                    return 1;
        }
        // should check weak here?
        for(int i=0; i<go; ++i) {
            if(GetElfIndex(maplib->libraries[i])==-1 || (my_context->elfs[GetElfIndex(maplib->libraries[i])]==self))
                if(GetLibWeakSymbolStartEnd(maplib->libraries[i], name, start, end, &weak, version, vername, 1))
                    if(*start)
                        return 1;
        }
    }
    #endif
    // nope, not found
    return 0;
}
static int GetGlobalSymbolStartEnd_internal(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t* self, int version, const char* vername)
{
    int weak = 0;
    // check with default version...
    const char* defver = GetDefaultVersion(my_context->globaldefver, name);
    // search non-weak symbol, from older to newer (first GLOBAL object wins)
    if(GetSymbolStartEnd(GetMapSymbols(my_context->elfs[0]), name, start, end, version, vername, (my_context->elfs[0]==self || !self)?1:0, defver))
        if(*start)
            return 1;
    // TODO: create a temporary map to search lib only 1 time, and in order of needed...
    // search in needed libs from neededlibs first, in order
    for(int i=0; i<my_context->neededlibs.size; ++i)
        if(GetLibGlobalSymbolStartEnd(my_context->neededlibs.libs[i], name, start, end, &weak, version, vername, isLocal(self, my_context->neededlibs.libs[i])))
            if(*start)
                return 1;
    // search in global symbols
    for(int i=0; i<maplib->libsz; ++i) {
        if(GetLibGlobalSymbolStartEnd(maplib->libraries[i], name, start, end, &weak, version, vername, isLocal(self, maplib->libraries[i])))
            if(*start)
                return 1;
    }

    // check with default version...
    defver = GetDefaultVersion(my_context->weakdefver, name);
    int ok = 0;
    // GetSymbolStartEnd should not change start/end if symbol is not found
    if(GetSymbolStartEnd(GetWeakSymbols(my_context->elfs[0]), name, start, end, version, vername, (my_context->elfs[0]==self || !self)?1:0, defver))
        if(*start)
            ok = 1;

    for(int i=0; i<maplib->libsz; ++i) {
        if(GetLibWeakSymbolStartEnd(maplib->libraries[i], name, start, end, &weak, version, vername, isLocal(self, maplib->libraries[i])))
            if(*start)
                ok = 1;
    }
    // nope, not found
    return (ok && *start)?1:0;
}
void** my_GetGTKDisplay();
void** my_GetGthreadsGotInitialized();
int GetGlobalSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t* self, int version, const char* vername)
{
    if(GetGlobalSymbolStartEnd_internal(maplib, name, start, end, self, version, vername)) {
        if(start && end && *end==*start) {  // object is of 0 sized, try to see an "_END" object of null size
            uintptr_t start2, end2;
            char* buff = (char*)malloc(strlen(name) + strlen("_END") + 1);
            strcpy(buff, name);
            strcat(buff, "_END");
            if(GetGlobalSymbolStartEnd_internal(maplib, buff, &start2, &end2, self, version, vername)) {
                if(end2>*end && start2==end2)
                    *end = end2;
            }
            box_free(buff);
        }
        return 1;
    }
    // some special case symbol, defined inside box64 itself
    if(!strcmp(name, "gdk_display")) {
        *start = (uintptr_t)my_GetGTKDisplay();
        *end = *start+sizeof(void*);
        printf_log(LOG_INFO, "Using global gdk_display for gdk-x11 (%p:%p)\n", start, *(void**)start);
        return 1;
    }
    if(!strcmp(name, "g_threads_got_initialized")) {
        *start = (uintptr_t)my_GetGthreadsGotInitialized();
        *end = *start+sizeof(int);
        printf_log(LOG_INFO, "Using global g_threads_got_initialized for gthread2 (%p:%p)\n", start, *(void**)start);
        return 1;
    }
    // not found...
    return 0;
}

elfheader_t* GetGlobalSymbolElf(lib_t *maplib, const char* name, int version, const char* vername)
{
    uintptr_t start = 0;
    uintptr_t end = 0;
    int weak = 0;
    elfheader_t* ret = NULL;
    // check with default version...
    const char* defver = GetDefaultVersion(my_context->globaldefver, name);

    if(GetSymbolStartEnd(GetMapSymbols(my_context->elfs[0]), name, &start, &end, version, vername, 1, defver))
        return my_context->elfs[0];
    for(int i=0; i<maplib->libsz; ++i) {
        if(GetLibGlobalSymbolStartEnd(maplib->libraries[i], name, &start, &end, &weak, version, vername, 1)) {
            int idx = GetElfIndex(maplib->libraries[i]);
            if(idx==-1) {
                printf_log(LOG_NONE, "Warning, getting Elf info for a native symbol \"%s\" from lib \"%s\"\n", name, GetNameLib(maplib->libraries[i]));
                return NULL;
            }
            if(weak)
                ret = my_context->elfs[idx];
            else
                return my_context->elfs[idx];
        }
    }

    defver = GetDefaultVersion(my_context->weakdefver, name);
    if(GetSymbolStartEnd(GetWeakSymbols(my_context->elfs[0]), name, &start, &end, version, vername, 1, defver))
        ret = my_context->elfs[0];
    for(int i=0; i<maplib->libsz; ++i) {
        if(GetLibWeakSymbolStartEnd(maplib->libraries[i], name, &start, &end, &weak, version, vername, 1)) {
            int idx = GetElfIndex(maplib->libraries[i]);
            if(idx==-1) {
                printf_log(LOG_NONE, "Warning, getting Elf info for a native symbol \"%s\" from lib \"%s\"\n", name, GetNameLib(maplib->libraries[i]));
                return NULL;
            }
            if(weak)
                ret = my_context->elfs[idx];
            else
                return my_context->elfs[idx];
        }
    }
    // return what has been found (maybe nothing)
    return ret;
}

int GetGlobalNoWeakSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, int version, const char* vername)
{
    int weak = 0;
    // check with default version...
    const char* defver = GetDefaultVersion(my_context->globaldefver, name);

    if(GetSymbolStartEnd(GetMapSymbols(my_context->elfs[0]), name, start, end, version, vername, 1, defver))
        if(*start || *end)
            return 1;
    for(int i=0; i<maplib->libsz; ++i)
        if(GetLibGlobalSymbolStartEnd(maplib->libraries[i], name, start, end, &weak, version, vername, 1))
            if(*start || *end)
                return 1;
    // nope, not found
    return 0;
}

int IsGlobalNoWeakSymbolInNative(lib_t *maplib, const char* name, int version, const char* vername)
{
    uintptr_t start, end;
    int weak;
    // check with default version...
    for(int i=0; i<maplib->libsz; ++i)
        if(GetElfIndex(maplib->libraries[i])==-1)
            if(GetLibGlobalSymbolStartEnd(maplib->libraries[i], name, &start, &end, &weak, version, vername, 1))
                if(start || end)
                    return 1;
    // nope, not found
    return 0;

}

int GetLocalSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t *self, int version, const char* vername)
{
    // check with default version...
    int weak;

    if(my_context->elfs[0]==self || !self) {
        if(GetSymbolStartEnd(GetLocalSymbols(my_context->elfs[0]), name, start, end, version, vername, 1, NULL))
            if(*start || *end)
                return 1;
        if(self)
            return 0;
    }
    for(int i=0; i<maplib->libsz; ++i) {
        if(GetElfIndex(maplib->libraries[i])!=-1 && (!self || my_context->elfs[GetElfIndex(maplib->libraries[i])]==self)) {
            if(GetLibLocalSymbolStartEnd(maplib->libraries[i], name, start, end, &weak, version, vername, 1))
                if(*start)
                    return 1;
            if(self)
                return 0;
        }
    }

    return 0;
}


const char* FindSymbolName(lib_t *maplib, void* p, void** start, uint64_t* sz, const char** libname, void** base, library_t** lib)
{
    // first, search in self...
    const char* ret = NULL;
    uintptr_t offs = 0;
    uint64_t size = 0;
    elfheader_t* h = FindElfAddress(my_context, (uintptr_t)p);
    if(h) {
        ret = FindNearestSymbolName(h, p, &offs, &size);

        if(start)
            *start = (void*)offs;
        if(sz)
            *sz = size;
        if(libname)
            *libname = ElfName(h);
        if(base)
            *base = GetBaseAddress(h);
        if(lib) {
            if(h == my_context->elfs[0])
                *lib = NULL;    // main elf
            else {
                for(int i=0; i<my_context->maplib->libsz; ++i) {
                    int idx = GetElfIndex(my_context->maplib->libraries[i]);
                    if((idx!=-1) && (my_context->elfs[idx]==h)) {
                        *lib = my_context->maplib->libraries[i];
                        return ret;
                    }
                }
                for(int i=0; i<my_context->local_maplib->libsz; ++i) {
                    int idx = GetElfIndex(my_context->local_maplib->libraries[i]);
                    if((idx!=-1) && (my_context->elfs[idx]==h)) {
                        *lib = my_context->local_maplib->libraries[i];
                        return ret;
                    }
                }
            }
        }
        return ret;
    }
    // TODO: find if cyclic references exists (should also fix MapLibAddMapLib)
    /* for (int i = 0; i < maplib->libsz; ++i) {
        // if (maplib == maplib->libraries[i]->maplib) continue;
        const char *nameInLib = FindSymbolName(maplib->libraries[i]->maplib, p, start, sz, libname, base);
        if (nameInLib) return nameInLib;
    } */
    return NULL;
}

void AddOffsetSymbol(lib_t *maplib, void* offs, const char* name)
{
    int ret;
    khint_t k = kh_put(mapoffsets, maplib->mapoffsets, (uintptr_t)offs, &ret);
    kh_value(maplib->mapoffsets, k) = (cstr_t)name;
}

const char* GetNameOffset(lib_t *maplib, void* offs)
{
    if(!maplib || !maplib->mapoffsets)
        return NULL;
    khint_t k = kh_get(mapoffsets, maplib->mapoffsets, (uintptr_t)offs);
    if (k!=kh_end(maplib->mapoffsets))
        return kh_value(maplib->mapoffsets, k);
    return NULL;
}
