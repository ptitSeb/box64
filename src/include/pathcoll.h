#ifndef __PATHCOLL_H_
#define __PATHCOLL_H_

// utility to handle path collection (like BOX64_PATH or BOX64_LD_LIBRARY_PATH)

// paths can be resized with realloc, so don't take address as invariant
typedef struct path_collection_s
{
    int    size;
    int    cap;
    char** paths;
} path_collection_t;

void ParseList(const char* List, path_collection_t* collection, int folder);
void FreeCollection(path_collection_t* collection);
void CopyCollection(path_collection_t* to, path_collection_t* from);
void AddPath(const char* path, path_collection_t* collection, int folder);
void PrependPath(const char* path, path_collection_t* collection, int folder);
void AppendList(path_collection_t* collection, const char* List, int folder);
void PrependList(path_collection_t* collection, const char* List, int folder);
int FindInCollection(const char* path, path_collection_t* collection);

#endif //__PATHCOLL_H_
