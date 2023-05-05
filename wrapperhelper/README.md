# Wrapper helper

**WARNING: There are still many problems with this tool. Please do NOT submit code generated directly by the tool, you should only use it as a preliminary reference.**


This tool is based on libclangtooling.

It parses the AST of the library header files, generating the required structures of the wrapping library, including:
- structure definitions,
- export function signatures,
- callback function wrapping,
etc. Of course, this cannot completely automate everything, it can only be used as a reference.

At the same time, this tool is also quite rough, and may even have errors.

## Build

```
sudo apt install libclang-14-dev
cd wrapperhelper
mkdir build; cd build; cmake ..
make
```

## Usage:

    helper <filename> <libname> [guest_triple] [host_triple] -- <clang_flags>
            <filename>    : set the header file to be parsed
            <libname>     : set libname required for wrapping func
            [guest_triple]: set guest triple: can be arm32/arm64/x86/x64, default is x64
            [host_triple] : set host triple: can be arm32/arm64/x86/x64, default is arm64
            --            : mandatory
            <clang_flags> : extra compiler flags

### Usage example:

`./helper /usr/include/jpeglib.h libjpeg x64 arm64 -- -I /usr/lib/gcc/x86_*/12.2.0/include --include /usr/lib/gcc/x86_*/12.2.0/include/stddef.h --include /usr/include/stdio.h`

You would see an output similar to the files `src/wrapped/wrappedlibjpeg.c` and `src/wrapped/wrappedlibjpeg_private.h`, should they exist.

If there are multiple header files to process, write them into a custom header file as input.

### Output sample

Using the command above, we get the following (trimmed) files:

In `wrappedlibjpeg_private.h`:
```c
...
GO(jpeg_quality_scaling, iFi)
...
GOM(jpeg_destroy, vFEp)
...
```

In `wrappedlibjpeg.c`:
```c
...
typedef struct jpeg_source_mgr {
    void *next_input_byte;
    unsigned long bytes_in_buffer;
    vFp_t init_source;
    iFp_t fill_input_buffer;
    vFpI_t skip_input_data;
    iFpi_t resync_to_restart;
    vFp_t term_source;
} jpeg_source_mgr, *jpeg_source_mgr_ptr;
...
#define GO(A) \
static uintptr_t my_term_source_fct_##A = 0; \
void  my_term_source_##A(struct jpeg_decompress_struct * a0) { \
    return RunFunction(my_context, my_term_source_fct_##A, 1, a0); \
}
SUPER()
#undef GO
static void* findterm_sourceFct(void* fct) {
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_term_source_fct_##A == (uintptr_t)fct) return my_term_source_##A;}
    SUPER()
    #undef GO
    #define GO(A) if(my_term_source_fct_##A == 0) {my_term_source_fct_##A = (uintptr_t)fct;return my_term_source_##A;}
    SUPER()
    #undef GO
    return NULL;
}
...
EXPORT int my_jpeg_quality_scaling(void *emu, int  quality) {
    libjpeg_my_t *my = (libjpeg_my_t*)my_lib->priv.w.p2;
    my->jpeg_quality_scaling(quality);
}
...
EXPORT void my_jpeg_destroy(void *emu, struct jpeg_common_struct * cinfo) {
    // WARN: This function's arg has a structure ptr which is special, may need to wrap it for the host
    libjpeg_my_t *my = (libjpeg_my_t*)my_lib->priv.w.p2;
    my->jpeg_destroy(cinfo);
}
...
```
