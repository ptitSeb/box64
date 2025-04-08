cmake_minimum_required(VERSION 3.13)

set(WINE_INCLUDE_DIR "" CACHE PATH "Path to wine's include directory")

if(WINE_INCLUDE_DIR STREQUAL "")
    message(FATAL_ERROR "Use -DWINE_INCLUDE_DIR= to set wine's include directory")
endif()

if(NOT IS_DIRECTORY ${WINE_INCLUDE_DIR})
    message(FATAL_ERROR "No such directory: ${WINE_INCLUDE_DIR}")
endif()

set(CMAKE_C_COMPILER aarch64-w64-mingw32-clang)
set(CMAKE_ASM_COMPILER aarch64-w64-mingw32-as)

project(wowbox64 C ASM)

set(BOX64_ROOT "${CMAKE_SOURCE_DIR}/..")

set(MAIN_SRC
    "${BOX64_ROOT}/wow64/wowbox64.c"
)

set(DYNAREC_ASM_SRC
    "${BOX64_ROOT}/src/dynarec/arm64/arm64_prolog.S"
    "${BOX64_ROOT}/src/dynarec/arm64/arm64_epilog.S"
    # "${BOX64_ROOT}/src/dynarec/arm64/arm64_next.S"
    # "${BOX64_ROOT}/src/dynarec/arm64/arm64_lock.S"
)

set_source_files_properties(${DYNAREC_ASM_SRC} PROPERTIES COMPILE_OPTIONS "-mcpu=cortex-a76")

set(BOX64CPU_SRC
    "${BOX64_ROOT}/src/os/os_wine.c"
    # "${BOX64_ROOT}/src/custommem.c"
    # "${BOX64_ROOT}/src/tools/rbtree.c"
    # "${BOX64_ROOT}/src/dynarec/dynablock.c"
)

add_library(wowbox64 SHARED ${MAIN_SRC} ${BOX64CPU_SRC} ${DYNAREC_ASM_SRC})

target_include_directories(wowbox64 PRIVATE
    "${BOX64_ROOT}/src/include"
    "${BOX64_ROOT}/src"
)

# always enable DynaRec, only supports ARM64 for now.
target_compile_definitions(wowbox64 PRIVATE DYNAREC ARM64)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)