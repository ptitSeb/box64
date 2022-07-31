#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#ifndef MAX_PATH
#define MAX_PATH 4096
#endif

#include "debug.h"
#include "pathcoll.h"

void FreeCollection(path_collection_t* collection)
{
    if(!collection)
        return;
    if(collection->cap) {
        for(int i=0; i<collection->size; ++i)
            box_free(collection->paths[i]);
        box_free(collection->paths);
    }
    collection->paths = NULL;
    collection->size = 0;
    collection->cap = 0;
    return;
}

void ParseList(const char* List, path_collection_t* collection, int folder)
{
    if(collection->cap)
        FreeCollection(collection);
    // count the stings (basically count(':') + 1) to define cap
    int cnt = 1;
    {
        const char* p = List;
        while(p) {
            p = strchr(p, ':');
            ++cnt;
            if(p) ++p;
        }
    }
    // alloc empty strings
    collection->cap = cnt;
    collection->paths = (char**)box_calloc(cnt, sizeof(char*));
    // and now split the paths...
    char tmp[MAX_PATH];
    const char *p = List;
    int idx = 0;
    while(p) {
        const char *p2 = strchr(p, ':');
        if(!p2) {
            strncpy(tmp, p, MAX_PATH - 1);
            p=NULL;
        } else {
            int l = (uintptr_t)p2 - (uintptr_t)p;
            strncpy(tmp, p, l);
            tmp[l]='\0';
            p=p2+1;
        }
        // check if there is terminal '/', add it if not
        int l = strlen(tmp);
        // skip empty strings
        if(l) {
            if(folder && tmp[l-1]!='/')
                strcat(tmp, "/");
            collection->paths[idx]  =box_strdup(tmp);
            collection->size=++idx;
        }
    }
}

void CopyCollection(path_collection_t* to, path_collection_t* from)
{
    to->cap = from->cap;
    to->paths = (char**)box_calloc(to->cap, sizeof(char*));
    to->size = from->size;
    for (int i=0; i<to->size; ++i)
        to->paths[i] = box_strdup(from->paths[i]);
}

void AddPath(const char* path, path_collection_t* collection, int folder)
{
    char tmp[MAX_PATH];
    strcpy(tmp, path);
    int l = strlen(tmp);
    // skip empty strings
    if(l) {
        if(folder && tmp[l-1]!='/')
            strcat(tmp, "/");
        if(collection->size==collection->cap) {
            collection->cap += 4;
            collection->paths = (char**)box_realloc(collection->paths, collection->cap*sizeof(char*));
        }
        collection->paths[collection->size++]=box_strdup(tmp);
    }
}
void PrependPath(const char* path, path_collection_t* collection, int folder)
{
    char tmp[MAX_PATH];
    strcpy(tmp, path);
    int l = strlen(tmp);
    // skip empty strings
    if(l) {
        if(folder && tmp[l-1]!='/')
            strcat(tmp, "/");
        if(collection->size==collection->cap) {
            collection->cap += 4;
            collection->paths = (char**)box_realloc(collection->paths, collection->cap*sizeof(char*));
        }
        memmove(collection->paths+1, collection->paths, sizeof(char*)*collection->size);
        collection->paths[0]=box_strdup(tmp);
        ++collection->size;
    }
}

void AppendList(path_collection_t* collection, const char* List, int folder)
{
    if(!List)
        return;
        // and now split the paths...
    char tmp[MAX_PATH];
    const char *p = List;
    while(p) {
        const char *p2 = strchr(p, ':');
        if(!p2) {
            strncpy(tmp, p, MAX_PATH - 1);
            p=NULL;
        } else {
            int l = (uintptr_t)p2 - (uintptr_t)p;
            strncpy(tmp, p, l);
            tmp[l]='\0';
            p=p2+1;
        }
        // check if there is terminal '/', add it if not
        int l = strlen(tmp);
        // skip empty strings
        if(l) {
            if(folder && tmp[l-1]!='/')
                strcat(tmp, "/");
            AddPath(tmp, collection, folder);
        }
    }

}
void PrependList(path_collection_t* collection, const char* List, int folder)
{
    if(!List)
        return;
        // and now split the paths...
    char tmp[MAX_PATH];
    char *p = box_strdup(List);
    while(p) {
        char *p2 = strrchr(p, ':');
        if(!p2) {
            strncpy(tmp, p, MAX_PATH - 1);
            box_free(p);
            p=NULL;
        } else {
            strncpy(tmp, p2+1, MAX_PATH - 1);
            tmp[MAX_PATH - 1]='\0';
            *p2 = '\0';
        }
        // check if there is terminal '/', add it if not
        int l = strlen(tmp);
        // skip empty strings
        if(l) {
            if(folder && tmp[l-1]!='/')
                strcat(tmp, "/");
            PrependPath(tmp, collection, folder);
        }
    }

}

int FindInCollection(const char* path, path_collection_t* collection)
{
    for (int i=0; i<collection->size; ++i)
        if(strcmp(path, collection->paths[i])==0)
            return 1;
    return 0;
}
