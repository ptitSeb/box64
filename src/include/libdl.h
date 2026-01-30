#ifndef __LIBDL_H__
#define __LIBDL_H__

typedef enum my_hookedlib_type_s {
    VulkanOverlay = 0
} my_hookedlib_type_t;

typedef struct my_hookedlib_s {
    void* self; // reference to itself
    const char* fullpath;   // to find repeted lib
    int ref;    // nb of reference;
    my_hookedlib_type_t type;
    void* priv;
} my_hookedlib_t;

#endif //__LIBDL_H__