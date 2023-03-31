This tool is based on liblcangtooling, by parsing the AST of the library header files, generating the required structures of the wrapping library, including structure definitions, export function signatures, callback function wrapping, etc. Of course, this cannot completely automate, only as a reference. At the same time, this tool is also quite rough, and may even have errors.

Usage:
    helper <filename> <libname> [guest_triple] [host_triple] -- <clang_flags>
            <filename> : set the header file to be parsed
            <libname>  : set libname required for wrapping func
            [guest_triple]: set guest triple arm32/arm64/x86/x64, default is x64
            [host_triple]: set host tripe arm32/arm64/x86/x64, default is arm64
            -- : is necessary

Usage example:

./helper /usr/include/jpeglib.h libjpeg x64 arm64 --

You will see output similar to wrappedlibjpeg.c file and libjpeg_private.h file. If there are multiple header files to process, write them into a custom header file as input.

The output may like this:
```c
// libjpeg_private.h
GOM(jpeg_read_coefficients, pFEp)
GOM(jpeg_write_coefficients, vFEpp)
GOM(jpeg_copy_critical_parameters, vFEpp)
GOM(jpeg_read_icc_profile, uFEppp)
...
// wrappedlibjpeg.c
typedef void (*vFp_t)(struct jpeg_common_struct *  a0);
typedef void (*vFpu_t)(struct jpeg_common_struct *  a0, int  a1);
...
typedef struct jpeg_error_mgr {
  vFp_t error_exit;
  vFpu_t emit_message;
  vFp_t output_messag
...
EXPORT void my_jpeg_set_marker_processor(void* emu, struct jpeg_decompress_struct * cinfo, int  marker_code, void *  routine) {
  // WARN: This function's arg has structure ptr which is special, may be need wrap it for host
  libjpeg62_my_tmy = (libjpeg62_my_t)my_lib->priv.w.p2;
  my->jpeg_set_marker_processor(cinfo, marker_code, findjpeg_marker_parser_methodFct(routine))
}
...
```

