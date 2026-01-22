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
#ifdef BOX32
#include "box32.h"
#endif

#include "bridge.h"

KHASH_MAP_IMPL_INT(mapoffsets, cstr_t);

lib_t *NewLibrarian(box64context_t* context)
{
    lib_t *maplib = (lib_t*)box_calloc(1, sizeof(lib_t));
    
    maplib->mapoffsets = kh_init(mapoffsets);
    maplib->globaldata = NewMapSymbols();

    return maplib;
}

void FreeLibrarian(lib_t **maplib, x64emu_t *emu)
{
    // should that be in reverse order?
    if(!maplib || !*maplib)
        return;

    library_t* owner = (*maplib)->owner;
    (*maplib)->owner = NULL;    // to avoid recursive free...
    /*for(int i=0; i<(*maplib)->libsz; ++i) {
        printf_dump(LOG_DEBUG, "Unloading %s\n", (*maplib)->libraries[i]->name);
        DecRefCount(&(*maplib)->libraries[i], emu);
    }*/
    box_free((*maplib)->libraries);
    (*maplib)->libraries = NULL;

    if((*maplib)->mapoffsets) {
        kh_destroy(mapoffsets, (*maplib)->mapoffsets);
    }
    FreeMapSymbols(&(*maplib)->globaldata);
    (*maplib)->libsz = (*maplib)->libcap = 0;

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

void MapLibPrependLib(lib_t* maplib, library_t* lib, library_t* ref)
{
    if(libraryInMapLib(maplib, lib))
        return;
    if (maplib->libsz == maplib->libcap) {
        maplib->libcap += 8;
        maplib->libraries = (library_t**)box_realloc(maplib->libraries, maplib->libcap*sizeof(library_t*));
    }
    // find insersion point
    int point = ref?maplib->libsz:0;
    if(ref)
        for(int i=0; i<maplib->libsz; ++i)
            if(maplib->libraries[i]==ref) {
                point = i;
                i = maplib->libsz;
            }
    if(point<maplib->libsz)
        memmove(&maplib->libraries[point+1], &maplib->libraries[point], sizeof(library_t*)*(maplib->libsz-point));
    maplib->libraries[point] = lib;
    ++maplib->libsz;
}

static void MapLibAddMapLib(lib_t* dest, library_t* lib_src, lib_t* src)
{
    if(!src)
        return;
    library_t *owner = src->owner;
    for(int i=0; i<src->libsz; ++i) {
        library_t* lib = src->libraries[i];
        if(!lib || libraryInMapLib(dest, lib)) continue;
        MapLibAddLib(dest, lib);
    }
    if(lib_src == owner)
        FreeLibrarian(&src, NULL);
}

void MapLibRemoveLib(lib_t* maplib, library_t* lib)
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

void promoteLocalLibGlobal(library_t* lib)
{
    if(!lib || !my_context)
        return;
    // promote lib from local to global...
    // for add the depending local libs...
    if(lib->maplib && !lib->maplib_ref) {
        lib_t* maplib = lib->maplib;
        lib->maplib = NULL;
        MapLibAddMapLib(my_context->maplib, lib, maplib);
    }
    lib->maplib = NULL;
    lib->maplib_ref = 0;
    if(!libraryInMapLib(my_context->maplib, lib))
        MapLibAddLib(my_context->maplib, lib);
    MapLibRemoveMapLib(my_context->local_maplib, my_context->maplib);
}

int isLibLocal(library_t* lib)
{
    if(!lib || !my_context)
        return 0;
    return libraryInMapLib(my_context->local_maplib, lib);
}

static int AddNeededLib_add(lib_t** maplib, int local, needed_libs_t* needed, int n, elfheader_t* verneeded, box64context_t* box64, x64emu_t* emu)
{
    const char* path = needed->names[n];
    printf_dump(LOG_DEBUG, "Trying to add \"%s\" to maplib%s\n", path, local?" (local)":"");
    // first check if lib is already loaded
    library_t *lib = getLib(my_context->maplib, path);
    if(lib) {
        IncRefCount(lib, emu);   // increment cntref
        needed->libs[n] = lib;
        if(local && *maplib)
            MapLibAddLib(*maplib, lib);
        printf_dump(LOG_DEBUG, "Already present in maplib => success\n");
        return 0;
    }
    // check also in the local loaded lib
    lib = getLib(my_context->local_maplib, path);
    if(lib) {
        printf_dump(LOG_DEBUG, "Already present in local_maplib => success\n");
        needed->libs[n] = lib;
        IncRefCount(lib, emu);   // increment cntref
        if(local) {
            // add lib to maplib...
            if(*maplib) {
                MapLibAddLib(*maplib, lib);
            }
        } else {
            // promote lib from local to global...
            promoteLocalLibGlobal(lib);
        }
        return 0;
    }
    // load a new one
    needed->libs[n] = lib = NewLibrary(path, box64, verneeded);
    if(!lib) {
        printf_dump(LOG_DEBUG, "Faillure to create lib => fail\n");
        return 1;   //Error
    }

    // add lib now
    if(local) {
        MapLibAddLib(my_context->local_maplib, lib);
        if(*maplib) {
            MapLibAddLib(*maplib, lib);
        } else {
            lib->maplib = NewLibrarian(box64);
            MapLibAddLib(lib->maplib, lib);
        }
    } else {
        MapLibAddLib(my_context->maplib, lib);
    }

    if(!*maplib)
        *maplib = (local)?lib->maplib:my_context->maplib;

    if(local && !lib->maplib) {
        lib->maplib = *maplib;
        lib->maplib_ref = 1;
    }

    if(AddSymbolsLibrary(*maplib, lib, emu)) {   // also add needed libs
        printf_dump(LOG_DEBUG, "Failure to Add lib => fail\n");
        return 1;
    }

    if (lib->type == LIB_EMULATED) {
        // Need to add library to the linkmap (put here so the link is ordered)
        #ifdef BOX32
        if(box64_is32bits) {
            linkmap32_t *lm = addLinkMapLib32(lib);
            if(!lm) {
                // Crashed already
                printf_dump(LOG_DEBUG, "Failure to add lib linkmap\n");
                return 1;
            }
            lm->l_addr = (Elf32_Addr)to_ptrv(GetElfDelta(lib->e.elf));
            lm->l_name = to_cstring(lib->name);
            lm->l_ld = to_ptrv(GetLoadedDynamicSection(lib->e.elf));
        } else
        #endif
        {
            linkmap_t *lm = addLinkMapLib(lib);
            if(!lm) {
                // Crashed already
                printf_dump(LOG_DEBUG, "Failure to add lib linkmap\n");
                return 1;
            }
            lm->l_addr = (Elf64_Addr)GetElfDelta(lib->e.elf);
            lm->l_name = lib->name;
            lm->l_ld = GetLoadedDynamicSection(lib->e.elf);
        }
        //TODO: it seems to never be removed!
    }
    IncRefCount(lib, emu);
    return 0;
}

int AddNeededLib_init(lib_t* maplib, int local, int bindnow, int deepbind, library_t* lib, elfheader_t* verneeded, box64context_t* box64, x64emu_t* emu)
{
    if(!lib)    // no lib, error is already detected, no need to return a new one
        return 0;
    if(!maplib)
        maplib = (local)?lib->maplib:my_context->maplib;

    elfheader_t* mainelf = GetElf(lib);

    if(!mainelf) {
        // It's a native libs, nothing else to do
    } else {
        // it's an emulated lib, 
        // load dependancies and launch init sequence
        if(LoadNeededLibs(mainelf, maplib, local, bindnow, deepbind, box64, emu)) {
            printf_dump(LOG_DEBUG, "Failure to Add dependant lib => fail\n");
            return 1;
        }

        // finalize the lib
        if(FinalizeLibrary(lib, local?maplib:NULL, bindnow, deepbind, emu)) {
            printf_dump(LOG_DEBUG, "Failure to finalizing lib => fail\n");
            return 1;
        }
    }
    // success
    printf_dump(LOG_DEBUG, "Created lib and added to maplib => success\n");
    
    return 0;
}
void AddNeededLib_remove(lib_t* maplib, int local, library_t** lib, box64context_t* box64, x64emu_t* emu)
{
    
    if(!lib || !*lib)    // no lib, error is already detected, no need to return a new one
        return;
    if(!maplib)
        maplib = (local)?(*lib)->maplib:my_context->maplib;

    DecRefCount(lib, emu);
}

EXPORTDYN
int AddNeededLib(lib_t* maplib, int local, int bindnow, int deepbind, needed_libs_t* needed, elfheader_t* verneeded, box64context_t* box64, x64emu_t* emu)
{
    if(!needed) // no needed libs, no problems
        return 0;
    box64_mapclean = 0;
    int ret = 0;
    // Add libs and symbol
    for(int i=0; i<needed->size; ++i) {
        if(AddNeededLib_add(&maplib, local, needed, i, verneeded, box64, emu)) {
            printf_log(strchr(needed->names[i],'/')?LOG_DEBUG:LOG_INFO, "Error loading needed lib %s\n", needed->names[i]);
            ret = 1;
        }
    }
    // error while loadind lib, unload...
    if(ret) {
        return ret;
    }
    
    // add dependant libs and init them
    int n = needed->size;
    for (int i=0; i<n; ++i)
        if(AddNeededLib_init(maplib, local, bindnow, deepbind, needed->libs[n-i-1], verneeded, box64, emu)) {
            printf_log(LOG_INFO, "Error initializing needed lib %s\n", needed->names[i]);
            ret = 1;
        }
    // all done
    if(BOX64ENV(allow_missing_libs)) return 0;
    return ret;
}
EXPORTDYN
void RemoveNeededLib(lib_t* maplib, int local, needed_libs_t* needed, box64context_t* box64, x64emu_t* emu)
{
    if(!needed) // no needed libs, no problems
        return;
    for(int i=0; i<needed->size; ++i) {
        if (BOX64ENV(log)>=LOG_DEBUG && needed->libs[i])
            printf_dump(LOG_DEBUG, "Will remove after failed init %s\n", needed->names[i]);
        AddNeededLib_remove(maplib, local, &needed->libs[i], box64, emu);
    }
}

library_t* GetLibMapLib(lib_t* maplib, const char* name)
{
    printf_dump(LOG_DEBUG, "Trying to Get \"%s\" to maplib\n", name);
    return getLib(maplib, name);
}

library_t* GetLibInternal(const char* name)
{
    printf_dump(LOG_DEBUG, "Trying to Get \"%s\" to maplib\n", name);
    library_t* lib = getLib(my_context->maplib, name);
    if(!lib) lib = getLib(my_context->local_maplib, name);
    return lib;
}

EXPORTDYN
uintptr_t FindGlobalSymbol(lib_t *maplib, const char* name, int version, const char* vername, int veropt)
{
    uintptr_t start = 0, end = 0;
    if(GetGlobalSymbolStartEnd(maplib, name, &start, &end, (void*)1, version, vername, veropt, NULL))
        return start;
    return 0;
}

static int isLocal(elfheader_t* self, library_t* l)
{
    if(GetElf(l)==self)
        return 1;
    return 0;
}

static void CheckNeededLibs(needed_libs_t* needed)
{
    while(needed->nb_done<needed->size) {
        library_t* lib = needed->libs[needed->nb_done++];
        if(lib) {
            int n = GetNeededLibsN(lib);
            char** names = GetNeededLibsNames(lib);
            for (int i=0; i<n; ++i) {
                add1lib_neededlib(needed, GetNeededLib(lib, i), names[i]);
            }
        }
    }
}

int GetNoSelfSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t* self, size_t size, int version, const char* vername, int veropt, void** elfsym)
{
    assert(self);   // need self for this one
    int weak = 0;
    void* sym;
    // search in needed libs from preloaded first, in order
    if(my_context->preload)
        for(int i=0; i<my_context->preload->size; ++i)
            if(self!=GetElf(my_context->preload->libs[i]) && GetLibGlobalSymbolStartEnd(my_context->preload->libs[i], name, start, end, size, &weak, &version, &vername, 0, &veropt, elfsym)) {
                return 1;
            }
    if(maplib==my_context->maplib) {
        // search non-weak symbol
        if(self!=my_context->elfs[0] && (sym = ElfGetGlobalSymbolStartEnd(my_context->elfs[0], start, end, name, &version, &vername, 0, &veropt))) {
            if(elfsym) *elfsym = sym;
            return 1;
        }
    }
    // search in global symbols
    if(maplib) {
        for(int i=0; i<maplib->libsz; ++i) {
            if(self!=GetElf(maplib->libraries[i]) && GetLibGlobalSymbolStartEnd(maplib->libraries[i], name, start, end, size, &weak, &version, &vername, 0, &veropt, elfsym))
                return 1;
        }
    }

    // GetSymbolStartEnd should not change start/end if symbol is not found
    if(self!=my_context->elfs[0] && (sym = ElfGetWeakSymbolStartEnd(my_context->elfs[0], start, end, name, &version, &vername, 0, &veropt))) {
        if(elfsym) *elfsym = sym;
        weak = 1;
    }

    if(maplib)
    for(int i=0; i<maplib->libsz; ++i) {
        if(self!=GetElf(maplib->libraries[i]) && GetLibWeakSymbolStartEnd(maplib->libraries[i], name, start, end, size, &weak, &version, &vername, 0, &veropt, elfsym)) {
            weak = 1;
        }
    }

    // nope, not found
    return weak;
}
static int isLibPreloaded(library_t* lib)
{
    if(my_context->preload)
        for(int i=0; i<my_context->preload->size; ++i)
            if(my_context->preload->libs[i] == lib)
                return 1;
    return 0;
}
int GetNextSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t* self, size_t size, int version, const char* vername, int veropt, void** elfsym)
{
    assert(self);   // need self for this one
    int weak = 0;
    int next = 0;
    void* sym;
    // search in needed libs from preloaded first, in order
    if(my_context->preload)
        for(int i=0; i<my_context->preload->size; ++i) {
            if(next) {
                if(GetLibGlobalSymbolStartEnd(my_context->preload->libs[i], name, start, end, size, &weak, &version, &vername, 0, &veropt, elfsym)) {
                    return 1;
                }
                if(GetLibWeakSymbolStartEnd(my_context->preload->libs[i], name, start, end, size, &weak, &version, &vername, 0, &veropt, elfsym)) {
                    return 1;
                }
            }
            if(self==GetElf(my_context->preload->libs[i])) {
                next = 1;
            }
        }
    if(maplib==my_context->maplib) {
        if(next) {
            if((sym = ElfGetGlobalSymbolStartEnd(my_context->elfs[0], start, end, name, &version, &vername, 0, &veropt))) {
                if(elfsym) *elfsym = sym;
                return 1;
            }
            if((sym = ElfGetWeakSymbolStartEnd(my_context->elfs[0], start, end, name, &version, &vername, 0, &veropt))) {
                if(elfsym) *elfsym = sym;
                return 1;
            }
        }
        if(self==my_context->elfs[0]) {
            next = 1;
        }
    }
    // build a lib list loaded after "self"
    needed_libs_t* libs = copy_neededlib(GetELfNeededLibs(self));
    if(!libs) libs = new_neededlib(0);
    int idx = 0;
    while(idx!=libs->size) {
        int cnt = GetNeededLibsN(libs->libs[idx]);
        for(int i=0; i<cnt; ++i) {
            library_t* lib = GetNeededLib(libs->libs[idx], i);
            if(lib && lib->name)
                add1lib_neededlib_name(libs, lib, lib->name);
        }
        ++idx;
    }
    // add remainling libs to the list
    if(maplib) {
        for(int i=0; i<maplib->libsz && !next; ++i) if(!isLibPreloaded(maplib->libraries[i])) {
            if(next && maplib->libraries[i]) {
                add1lib_neededlib_name(libs, maplib->libraries[i], maplib->libraries[i]->name);
            } else
            if(self==GetElf(maplib->libraries[i]))
                next = 1;
        }
    }
    // search the list
    for(int i=0; i<libs->size; ++i) {
        if(GetLibGlobalSymbolStartEnd(libs->libs[i], name, start, end, size, &weak, &version, &vername, 0, &veropt, elfsym)) {
            free_neededlib(libs);
            return 1;
        }
        if(GetLibWeakSymbolStartEnd(libs->libs[i], name, start, end, size, &weak, &version, &vername, 0, &veropt, elfsym)) {
            free_neededlib(libs);
            return 1;
        }
    }

    // search in global symbols
    if(maplib) {
        for(int i=0; i<maplib->libsz; ++i) if(!isLibPreloaded(maplib->libraries[i])) {
            if(next) {
                if(GetLibGlobalSymbolStartEnd(maplib->libraries[i], name, start, end, size, &weak, &version, &vername, 0, &veropt, elfsym)) {
                    return 1;
                }
                if(GetLibWeakSymbolStartEnd(maplib->libraries[i], name, start, end, size, &weak, &version, &vername, 0, &veropt, elfsym)) {
                    return 1;
                }
            }
            if(self==GetElf(maplib->libraries[i]))
                next = 1;
        }
    }
    return 0;
}
static int GetGlobalSymbolStartEnd_internal(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t* self, int* version, const char** vername, int* veropt, void** elfsym)
{
    int weak = 0;
    size_t size = 0;
    void* sym;
    // search in needed libs from preloaded first, in order
    if(my_context->preload)
        for(int i=0; i<my_context->preload->size; ++i)
            if(GetLibGlobalSymbolStartEnd(my_context->preload->libs[i], name, start, end, size, &weak, version, vername, isLocal(self, my_context->preload->libs[i]), veropt, elfsym)) {
                return 1;
            }
    if(maplib==my_context->maplib) {
        // search non-weak symbol, from older to newer (first GLOBAL object wins, starting with self)
        if((sym = ElfGetGlobalSymbolStartEnd(my_context->elfs[0], start, end, name, version, vername, (my_context->elfs[0]==self || !self)?1:0, veropt))) {
            if(elfsym) *elfsym = sym;
            return 1;
        }
    }
    // search in global symbols
    if(maplib) {
        for(int i=0; i<maplib->libsz; ++i) {
            if(GetLibGlobalSymbolStartEnd(maplib->libraries[i], name, start, end, size, &weak, version, vername, isLocal(self, maplib->libraries[i]), veropt, elfsym))
                return 1;
        }
    }

    // GetSymbolStartEnd should not change start/end if symbol is not found
    if((sym = ElfGetWeakSymbolStartEnd(my_context->elfs[0], start, end, name, version, vername, (my_context->elfs[0]==self || !self)?1:0, veropt))) {
        if(elfsym) *elfsym = sym;
        weak = 1;
    }
    if(maplib)
    for(int i=0; i<maplib->libsz; ++i) {
        if(GetLibWeakSymbolStartEnd(maplib->libraries[i], name, start, end, size, &weak, version, vername, isLocal(self, maplib->libraries[i]), veropt, elfsym)) {
            weak = 1;
        }
    }
    // nope, not found
    return weak;
}
#ifndef STATICBUILD
void** my_GetGTKDisplay();
void** my_GetGthreadsGotInitialized();
#endif
int GetGlobalSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t* self, int version, const char* vername, int veropt, void** elfsym)
{
    if(GetGlobalSymbolStartEnd_internal(maplib, name, start, end, self, &version, &vername, &veropt, elfsym)) {
        if(start && end && *end==*start) {  // object is of 0 sized, try to see an "_END" object of null size
            uintptr_t start2, end2;
            char* buff = (char*)box_malloc(strlen(name) + strlen("_END") + 1);
            strcpy(buff, name);
            strcat(buff, "_END");
            if(GetGlobalSymbolStartEnd_internal(maplib, buff, &start2, &end2, self, &version, &vername, &veropt, elfsym)) {
                if(end2>*end && start2==end2)
                    *end = end2;
            }
            box_free(buff);
        }
        return 1;
    }
    #ifndef STATICBUILD
    // some special case symbol, defined inside box64 itself
    if(!box64_is32bits &&!strcmp(name, "gdk_display") && !BOX64ENV(nogtk)) {
        *start = (uintptr_t)my_GetGTKDisplay();
        *end = *start+sizeof(void*);
        printf_log(LOG_INFO, "Using global gdk_display for gdk-x11 (%p:%p)\n", start, *(void**)start);
        return 1;
    }
    if(!box64_is32bits && !strcmp(name, "g_threads_got_initialized") && !BOX64ENV(nogtk)) {
        *start = (uintptr_t)my_GetGthreadsGotInitialized();
        *end = *start+sizeof(int);
        printf_log(LOG_INFO, "Using global g_threads_got_initialized for gthread2 (%p:%p)\n", start, *(void**)start);
        return 1;
    }
    #endif
    // not found...
    return 0;
}

static int GetGlobalWeakSymbolStartEnd_internal(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t* self, int* version, const char** vername, int* veropt, void** elfsym)
{
    int weak = 0;
    size_t size = 0;
    void* sym;
    // search in needed libs from preloaded first, in order
    if(my_context->preload)
        for(int i=0; i<my_context->preload->size; ++i)
            if(GetLibGlobalSymbolStartEnd(my_context->preload->libs[i], name, start, end, size, &weak, version, vername, isLocal(self, my_context->preload->libs[i]), veropt, elfsym))
                return 1;
    // search non-weak symbol, from older to newer (first GLOBAL object wins)
    if((sym = ElfGetGlobalSymbolStartEnd(my_context->elfs[0], start, end, name, version, vername, (my_context->elfs[0]==self || !self)?1:0, veropt))) {
        if(elfsym) *elfsym = sym;
        return 1;
    }
    if(maplib)
    for(int i=0; i<maplib->libsz; ++i) {
        if(GetLibGlobalSymbolStartEnd(maplib->libraries[i], name, start, end, size, &weak, version, vername, isLocal(self, maplib->libraries[i]), veropt, elfsym))
            return 1;
    }

    // check with default version...
    int ok = 0;
    // GetSymbolStartEnd should not change start/end if symbol is not found
    if(( sym = ElfGetWeakSymbolStartEnd(my_context->elfs[0], start, end, name, version, vername, (my_context->elfs[0]==self || !self)?1:0, veropt))) {
        if(elfsym) *elfsym = sym;
        ok = 1;
    }

    if(maplib)
    for(int i=0; i<maplib->libsz; ++i) {
        if(GetLibWeakSymbolStartEnd(maplib->libraries[i], name, start, end, size, &weak, version, vername, isLocal(self, maplib->libraries[i]), veropt, elfsym))
            ok = 1;
    }
    // nope, not found
    return ok;
}

int GetGlobalWeakSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t* self, int version, const char* vername, int veropt, void** elfsym)
{
    if(GetGlobalWeakSymbolStartEnd_internal(maplib, name, start, end, self, &version, &vername, &veropt, elfsym)) {
        if(start && end && *end==*start) {  // object is of 0 sized, try to see an "_END" object of null size
            uintptr_t start2, end2;
            char* buff = (char*)box_malloc(strlen(name) + strlen("_END") + 1);
            strcpy(buff, name);
            strcat(buff, "_END");
            if(GetGlobalWeakSymbolStartEnd_internal(maplib, buff, &start2, &end2, self, &version, &vername, &veropt, elfsym)) {
                if(end2>*end && start2==end2)
                    *end = end2;
            }
            box_free(buff);
        }
        return 1;
    }
    #ifndef STATICBUILD
    // some special case symbol, defined inside box64 itself
    if(!box64_is32bits && !strcmp(name, "gdk_display") && !BOX64ENV(nogtk)) {
        *start = (uintptr_t)my_GetGTKDisplay();
        *end = *start+sizeof(void*);
        if(elfsym) *elfsym = NULL;
        printf_log(LOG_INFO, "Using global gdk_display for gdk-x11 (%p:%p)\n", start, *(void**)start);
        return 1;
    }
    if(!box64_is32bits && !strcmp(name, "g_threads_got_initialized") && !BOX64ENV(nogtk)) {
        *start = (uintptr_t)my_GetGthreadsGotInitialized();
        *end = *start+sizeof(int);
        if(elfsym) *elfsym = NULL;
        printf_log(LOG_INFO, "Using global g_threads_got_initialized for gthread2 (%p:%p)\n", start, *(void**)start);
        return 1;
    }
    #endif
    // not found...
    return 0;
}

elfheader_t* GetGlobalSymbolElf(lib_t *maplib, const char* name, int version, const char* vername, int veropt)
{
    uintptr_t start = 0;
    uintptr_t end = 0;
    size_t size = 0;
    int weak = 0;
    elfheader_t* ret = NULL;
    // check with default version...
    if(ElfGetGlobalSymbolStartEnd(my_context->elfs[0], &start, &end, name, &version, &vername, 1, &veropt))
        return my_context->elfs[0];
    for(int i=0; i<maplib->libsz; ++i) {
        if(GetLibGlobalSymbolStartEnd(maplib->libraries[i], name, &start, &end, size, &weak, &version, &vername, 1, &veropt, NULL)) {
            elfheader_t* h = GetElf(maplib->libraries[i]);
            if(!h) {
                printf_log(LOG_NONE, "Warning, getting Elf info for a native symbol \"%s\" from lib \"%s\"\n", name, GetNameLib(maplib->libraries[i]));
                return NULL;
            }
            if(weak)
                ret = h;
            else
                return h;
        }
    }

    if(ElfGetWeakSymbolStartEnd(my_context->elfs[0], &start, &end, name, &version, &vername, 1, &veropt))
        ret = my_context->elfs[0];
    for(int i=0; i<maplib->libsz; ++i) {
        if(GetLibWeakSymbolStartEnd(maplib->libraries[i], name, &start, &end, size, &weak, &version, &vername, 1, &veropt, NULL)) {
            elfheader_t* h = GetElf(maplib->libraries[i]);
            if(!h) {
                printf_log(LOG_NONE, "Warning, getting Elf info for a native symbol \"%s\" from lib \"%s\"\n", name, GetNameLib(maplib->libraries[i]));
                return NULL;
            }
            if(weak)
                ret = h;
            else
                return h;
        }
    }
    // return what has been found (maybe nothing)
    return ret;
}

int GetGlobalNoWeakSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, int version, const char* vername, int veropt, void** elfsym)
{
    int weak = 0;
    size_t size = 0;
    void* sym;
    // check global GLOB_DAT kind of symbols
    if(GetSymbolStartEnd(GetGlobalData(maplib), name, start, end, version, vername, 1, veropt)) {
        if(elfsym) *elfsym = NULL;
        return 1;
    }
    // check with default version...
    if((sym = ElfGetGlobalSymbolStartEnd(my_context->elfs[0], start, end, name, &version, &vername, 1, &veropt))) {
        if(elfsym) *elfsym = sym;
        return 1;
    }
    for(int i=0; i<maplib->libsz; ++i)
        if(GetLibGlobalSymbolStartEnd(maplib->libraries[i], name, start, end, size, &weak, &version, &vername, 1, &veropt, elfsym))
            return 1;
    // nope, not found
    return 0;
}

int IsGlobalNoWeakSymbolInNative(lib_t *maplib, const char* name, int version, const char* vername, int veropt)
{
    uintptr_t start, end;
    size_t size = 0;
    int weak;
    // check with default version...
    for(int i=0; i<maplib->libsz; ++i)
        if(GetElfIndex(maplib->libraries[i])==-1)
            if(GetLibGlobalSymbolStartEnd(maplib->libraries[i], name, &start, &end, size, &weak, &version, &vername, 1, &veropt, NULL))
                return 1;
    // nope, not found
    return 0;

}

int GetLocalSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t *self, int version, const char* vername, int veropt, void** elfsym)
{
    // check with default version...
    int weak;
    size_t size = 0;
    void* sym;
    if(my_context->elfs[0]==self || !self) {
        if((sym = ElfGetLocalSymbolStartEnd(my_context->elfs[0], start, end, name, &version, &vername, 1, &veropt))) {
            if(elfsym) *elfsym = sym;
            return 1;
        }
        if(self)
            return 0;
    }
    for(int i=0; i<maplib->libsz; ++i) {
        if(GetElfIndex(maplib->libraries[i])!=-1 && (!self || GetElf(maplib->libraries[i])==self)) {
            if(GetLibLocalSymbolStartEnd(maplib->libraries[i], name, start, end, size, &weak, &version, &vername, 1, &veropt, elfsym))
                return 1;
            if(self)
                return 0;
        }
    }

    return 0;
}

int GetAnySymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, int version, const char* vername, int veropt)
{
    if(!maplib)
        return 0;
    for(int i=0; i<maplib->libsz; ++i) {
        elfheader_t* h = GetElf(maplib->libraries[i]);
        if(h && ElfGetSymbolStartEnd(h, start, end, name, &version, &vername, 1, &veropt))
            return 1;
    }
    return 0;

}

int GetSymTabStartEnd(lib_t* maplib, const char* name, uintptr_t* start, uintptr_t* end)
{
    if(!maplib)
        return 0;
    for(int i=0; i<maplib->libsz; ++i) {
        elfheader_t* h = GetElf(maplib->libraries[i]);
        if(box64_is32bits) {
            if(h && ElfGetSymTabStartEnd32(h, start, end, name))
                return 1;
        } else {
            if(h && ElfGetSymTabStartEnd64(h, start, end, name))
                return 1;
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
                    elfheader_t* head = GetElf(my_context->maplib->libraries[i]);
                    if((head) && (head==h)) {
                        *lib = my_context->maplib->libraries[i];
                        return ret;
                    }
                }
                for(int i=0; i<my_context->local_maplib->libsz; ++i) {
                    elfheader_t* head = GetElf(my_context->local_maplib->libraries[i]);
                    if((head) && (head==h)) {
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
