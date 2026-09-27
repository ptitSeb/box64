cmake_minimum_required(VERSION 3.13)
project(box64ec C ASM)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(BOX64_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")

string(REPLACE "," ";" INTERPRETER "${INTERPRETER_STR}")
if(NOT INTERPRETER)
    message(FATAL_ERROR "Build Box64EC from the parent project with -DARM64EC=ON")
endif()

add_library(box64ec_interpreter STATIC ${INTERPRETER})

add_library(box64ec SHARED
    box64ec.c box64ec_init.c box64ec_cpu.c box64ec_thread.c
    box64ec_context.c box64ec_runtime.c box64ec_transition.c
    box64ec_dispatch.c box64ec_syscalls.c box64ec_memory.c
    box64ec_exception.c module.S box64ec.def
    "${BOX64_ROOT}/wine/common/crt.c"
    "${BOX64_ROOT}/wine/common/wrt.c"
    "${BOX64_ROOT}/src/custommem.c"
    "${BOX64_ROOT}/src/emu/x64emu.c"
    "${BOX64_ROOT}/src/emu/x64compstrings.c"
    "${BOX64_ROOT}/src/emu/x64primop.c"
    "${BOX64_ROOT}/src/emu/x64shaext.c"
    "${BOX64_ROOT}/src/emu/x64run_private.c"
    "${BOX64_ROOT}/src/emu/x87emu_private.c"
    "${BOX64_ROOT}/src/os/backtrace.c"
    "${BOX64_ROOT}/src/os/emit_signal_wine.c"
    "${BOX64_ROOT}/src/os/my_cpuid.c"
    "${BOX64_ROOT}/src/os/random_wine.c"
    "${BOX64_ROOT}/src/os/os_wine.c"
    "${BOX64_ROOT}/src/os/sysinfo.c"
    "${BOX64_ROOT}/src/os/freq_wine.c"
    "${BOX64_ROOT}/src/os/hostext_wine.c"
    "${BOX64_ROOT}/src/tools/env.c"
    "${BOX64_ROOT}/src/tools/alternate.c"
    "${BOX64_ROOT}/src/tools/cpumask.c"
    "${BOX64_ROOT}/src/tools/bitutils.c"
    "${BOX64_ROOT}/src/tools/rbtree.c"
    "${BOX64_ROOT}/external/musl/__math_divzero.c"
    "${BOX64_ROOT}/external/musl/__math_invalid.c"
    "${BOX64_ROOT}/external/musl/expm1.c"
    "${BOX64_ROOT}/external/musl/frexp.c"
    "${BOX64_ROOT}/external/musl/ldexp.c"
    "${BOX64_ROOT}/external/musl/log1p.c"
    "${BOX64_ROOT}/external/musl/log2.c"
    "${BOX64_ROOT}/external/musl/log2_data.c"
    "${BOX64_ROOT}/external/musl/scalbn.c"
    "${BOX64_ROOT}/external/musl/sqrt_data.c"
    "${BOX64_ROOT}/external/musl/sqrtf.c"
)
foreach(target box64ec box64ec_interpreter)
    target_include_directories(${target} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${BOX64_ROOT}/src/include"
        "${BOX64_ROOT}/src"
        "${BOX64_ROOT}/wine/include"
        "${BOX64_ROOT}/external/musl"
        "${BOX64_ROOT}/external/musl/internal")
    target_compile_definitions(${target} PRIVATE ARM64 BOX64EC)

    # ARM64EC contexts cannot preserve scalable vector state.
    target_compile_options(${target} PRIVATE
        -Wno-inconsistent-dllimport
        "SHELL:-Xclang -target-feature -Xclang -sve2"
        "SHELL:-Xclang -target-feature -Xclang -sve")
endforeach()
target_link_libraries(box64ec PRIVATE box64ec_interpreter)

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
target_link_options(box64ec PRIVATE -nostdlib -nodefaultlibs
    -lclang_rt.builtins-aarch64 -Wl,/ignore:4217)

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
