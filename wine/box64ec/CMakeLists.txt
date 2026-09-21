cmake_minimum_required(VERSION 3.13)
project(box64ec C ASM)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(BOX64_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")

add_library(box64ec SHARED box64ec.c box64ec.def)

# ARM64EC contexts cannot preserve scalable vector state.
target_compile_options(box64ec PRIVATE
    "SHELL:-Xclang -target-feature -Xclang -sve2"
    "SHELL:-Xclang -target-feature -Xclang -sve")

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ntdll.a"
    COMMAND arm64ec-w64-mingw32-dlltool
        -D ntdll.dll
        -d "${BOX64_ROOT}/wine/ntdll.def"
        -l "${CMAKE_CURRENT_BINARY_DIR}/ntdll.a"
        -k
    DEPENDS "${BOX64_ROOT}/wine/ntdll.def"
    VERBATIM
)
add_custom_target(generate_ntdll_import_lib_ec
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/ntdll.a")
add_dependencies(box64ec generate_ntdll_import_lib_ec)
target_link_libraries(box64ec PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/ntdll.a" mingwex)
target_link_options(box64ec PRIVATE -nostdlib -nodefaultlibs)

set_target_properties(box64ec PROPERTIES
    PREFIX ""
    SUFFIX ".dll"
    OUTPUT_NAME "box64ec"
)

add_custom_command(TARGET box64ec POST_BUILD
    COMMAND "${Python3_EXECUTABLE}" "${BOX64_ROOT}/wine/override_wine_builtin.py"
        $<TARGET_FILE:box64ec>
    VERBATIM
)
