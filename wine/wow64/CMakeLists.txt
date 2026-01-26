cmake_minimum_required(VERSION 3.13)
project(wowbox64 C ASM)

if(${CMAKE_VERSION} VERSION_LESS "3.12.2")
    find_package(PythonInterp 3)
    if(NOT PYTHONINTERP_FOUND)
        message( FATAL_ERROR "You need a Python interpreter, CMake will exit." )
    endif()
    if(${PYTHON_VERSION_MAJOR} LESS 3)
        message( FATAL_ERROR "You need a Python 3 interpreter, CMake will exit." )
    endif()
else()
    find_package(Python3)
    if(NOT Python3_Interpreter_FOUND)
        message( FATAL_ERROR "You need a Python interpreter, CMake will exit." )
    endif()
    set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE} CACHE INTERNAL "The Python3 executable" FORCE)
endif()

set(BOX64_ROOT "${CMAKE_SOURCE_DIR}/../..")

string(REPLACE "," ";" DYNAREC_ASM "${DYNAREC_ASM_STR}")
string(REPLACE "," ";" DYNAREC_PASS "${DYNAREC_PASS_STR}")
string(REPLACE "," ";" INTERPRETER "${INTERPRETER_STR}")

set(WOW64_MAIN_SRC
    "${BOX64_ROOT}/wine/common/crt.c"
    "${BOX64_ROOT}/wine/common/wrt.c"
    "${BOX64_ROOT}/wine/wow64/wowbox64.c"
)

set(MUSL_SRC
    "${BOX64_ROOT}/external/musl/__cos.c"
    "${BOX64_ROOT}/external/musl/__math_divzero.c"
    "${BOX64_ROOT}/external/musl/__math_invalid.c"
    "${BOX64_ROOT}/external/musl/__rem_pio2_large.c"
    "${BOX64_ROOT}/external/musl/__rem_pio2.c"
    "${BOX64_ROOT}/external/musl/__sin.c"
    "${BOX64_ROOT}/external/musl/cos.c"
    "${BOX64_ROOT}/external/musl/exp_data.c"
    "${BOX64_ROOT}/external/musl/exp2.c"
    "${BOX64_ROOT}/external/musl/expm1.c"
    "${BOX64_ROOT}/external/musl/frexp.c"
    "${BOX64_ROOT}/external/musl/ldexp.c"
    "${BOX64_ROOT}/external/musl/log1p.c"
    "${BOX64_ROOT}/external/musl/log2_data.c"
    "${BOX64_ROOT}/external/musl/log2.c"
    "${BOX64_ROOT}/external/musl/scalbn.c"
    "${BOX64_ROOT}/external/musl/sin.c"
    "${BOX64_ROOT}/external/musl/sincos.c"
    "${BOX64_ROOT}/external/musl/sqrt_data.c"
    "${BOX64_ROOT}/external/musl/sqrt.c"
    "${BOX64_ROOT}/external/musl/sqrtf.c"
)

set_source_files_properties(${DYNAREC_ASM} PROPERTIES COMPILE_OPTIONS "-mcpu=cortex-a76")

foreach(STEP_VALUE RANGE 3)
    add_library(wow64_dynarec_pass${STEP_VALUE} OBJECT ${DYNAREC_PASS})
    target_compile_definitions(wow64_dynarec_pass${STEP_VALUE} PRIVATE STEP=${STEP_VALUE})
endforeach()

add_library(wow64_test_interpreter OBJECT ${INTERPRETER})
set_target_properties(wow64_test_interpreter PROPERTIES COMPILE_DEFINITIONS "TEST_INTERPRETER")

set(WOW64_BOX64CPU_SRC
    "${BOX64_ROOT}/src/build_info.c"
    "${BOX64_ROOT}/src/custommem.c"
    "${BOX64_ROOT}/src/dynarec/arm64/arm64_immenc.c"
    "${BOX64_ROOT}/src/dynarec/arm64/arm64_printer.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_arch.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_functions.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_jmpnext.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_consts.c"
    "${BOX64_ROOT}/src/dynarec/arm64/updateflags_arm64.c"
    "${BOX64_ROOT}/src/dynarec/dynablock.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_native_functions.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_native.c"
    "${BOX64_ROOT}/src/dynarec/dynarec.c"
    "${BOX64_ROOT}/src/dynarec/dynacache_reloc.c"
    "${BOX64_ROOT}/src/emu/x64compstrings.c"
    "${BOX64_ROOT}/src/emu/x64emu.c"
    "${BOX64_ROOT}/src/emu/x64primop.c"
    "${BOX64_ROOT}/src/emu/x64shaext.c"
    "${BOX64_ROOT}/src/emu/x64test.c"
    "${BOX64_ROOT}/src/emu/x64trace.c"
    "${BOX64_ROOT}/src/emu/x64run_private.c"
    "${BOX64_ROOT}/src/emu/x87emu_private.c"
    "${BOX64_ROOT}/src/os/backtrace.c"
    "${BOX64_ROOT}/src/os/os_wine.c"
    "${BOX64_ROOT}/src/os/hostext_common.c"
    "${BOX64_ROOT}/src/os/hostext_wine.c"
    "${BOX64_ROOT}/src/os/freq_wine.c"
    "${BOX64_ROOT}/src/os/symbolfuncs_wine.c"
    "${BOX64_ROOT}/src/os/emit_signal_wine.c"
    "${BOX64_ROOT}/src/os/perfmap.c"
    "${BOX64_ROOT}/src/os/random_wine.c"
    "${BOX64_ROOT}/src/os/my_cpuid.c"
    "${BOX64_ROOT}/src/os/sysinfo.c"
    "${BOX64_ROOT}/src/tools/alternate.c"
    "${BOX64_ROOT}/src/tools/bitutils.c"
    "${BOX64_ROOT}/src/tools/env.c"
    "${BOX64_ROOT}/src/tools/rbtree.c"
)

add_library(wowbox64 SHARED ${WOW64_MAIN_SRC} ${MUSL_SRC} ${WOW64_BOX64CPU_SRC} ${INTERPRETER} ${DYNAREC_ASM}
    $<TARGET_OBJECTS:wow64_dynarec_pass0>
    $<TARGET_OBJECTS:wow64_dynarec_pass1>
    $<TARGET_OBJECTS:wow64_dynarec_pass2>
    $<TARGET_OBJECTS:wow64_dynarec_pass3>
    $<TARGET_OBJECTS:wow64_test_interpreter>
    "${BOX64_ROOT}/wine/wow64/wowbox64.def")

include_directories(
    "${BOX64_ROOT}/src/include"
    "${BOX64_ROOT}/src"
    "${BOX64_ROOT}/wine/include"
    "${BOX64_ROOT}/external/musl"
    "${BOX64_ROOT}/external/musl/internal"
)

set(DLLTOOL aarch64-w64-mingw32-dlltool)

function(import_dll name)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${name}.a
        COMMAND ${DLLTOOL} 
            -D ${name}.dll
            -d ${BOX64_ROOT}/wine/${name}.def
            -l ${CMAKE_CURRENT_BINARY_DIR}/${name}.a
            -k
        DEPENDS ${BOX64_ROOT}/wine/${name}.def
        COMMENT "Generating import library ${name}.a from ${name}.def"
    )
    add_custom_target(generate_${name}_import_lib DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${name}.a)
    add_dependencies(wowbox64 generate_${name}_import_lib)
    target_link_libraries(wowbox64 PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/${name}.a)
endfunction()


import_dll(ntdll)
import_dll(wow64)

# always enable DynaRec, only supports ARM64 for now.
add_compile_definitions(DYNAREC ARM64)

target_compile_options(wowbox64 PRIVATE -Wno-inconsistent-dllimport)
target_link_options(wowbox64 PRIVATE -nostdlib -nodefaultlibs 
    -lclang_rt.builtins-aarch64
    -Wl,--image-base,0x004c0000
    -Wl,/ignore:4217 # LNK4217: locally defined symbol imported
)
set_target_properties(wowbox64 PROPERTIES
    PREFIX ""
    SUFFIX ".dll"
    OUTPUT_NAME "wowbox64"
)

add_custom_command(
    TARGET wowbox64 POST_BUILD
    COMMAND "${PYTHON_EXECUTABLE}" ${CMAKE_SOURCE_DIR}/../override_wine_builtin.py $<TARGET_FILE:wowbox64>
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
