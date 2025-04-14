cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_COMPILER aarch64-w64-mingw32-clang)
set(CMAKE_ASM_COMPILER aarch64-w64-mingw32-as)

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

project(wowbox64 C ASM)

set(BOX64_ROOT "${CMAKE_SOURCE_DIR}/..")

set(MAIN_SRC
    "${BOX64_ROOT}/wow64/wowbox64.c"
)

set(DYNAREC_ASM_SRC
    "${BOX64_ROOT}/src/dynarec/arm64/arm64_prolog.S"
    "${BOX64_ROOT}/src/dynarec/arm64/arm64_epilog.S"
    "${BOX64_ROOT}/src/dynarec/arm64/arm64_next.S"
    "${BOX64_ROOT}/src/dynarec/arm64/arm64_lock.S"
)

set_source_files_properties(${DYNAREC_ASM_SRC} PROPERTIES COMPILE_OPTIONS "-mcpu=cortex-a76")

set(DYNAREC_PASS_SRC
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_00.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_0f.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_64.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_66.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_660f.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_6664.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_66f0.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_66f20f.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_66f30f.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_67_32.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_67_avx.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_67.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_6764_32.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_avx_0f.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_avx_0f38.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_avx_66_0f.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_avx_66_0f38.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_avx_66_0f3a.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_avx_f2_0f.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_avx_f2_0f38.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_avx_f2_0f3a.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_avx_f3_0f.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_avx_f3_0f38.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_avx.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_d8.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_d9.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_da.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_db.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_dc.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_dd.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_de.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_df.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_emit_logic.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_emit_math.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_emit_shift.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_emit_tests.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_f0.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_f20f.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_f30f.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_helper.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_native_pass.c"
)

foreach(STEP_VALUE RANGE 3)
    add_library(dynarec_pass${STEP_VALUE} OBJECT ${DYNAREC_PASS_SRC})
    target_compile_definitions(dynarec_pass${STEP_VALUE} PRIVATE STEP=${STEP_VALUE})
endforeach()

set(INTERPRETER_SRC
    "${BOX64_ROOT}/src/emu/x64run.c"
    "${BOX64_ROOT}/src/emu/x64run0f.c"
    "${BOX64_ROOT}/src/emu/x64run64.c"
    "${BOX64_ROOT}/src/emu/x64run66.c"
    "${BOX64_ROOT}/src/emu/x64run660f.c"
    "${BOX64_ROOT}/src/emu/x64run66f20f.c"
    "${BOX64_ROOT}/src/emu/x64run66f30f.c"
    "${BOX64_ROOT}/src/emu/x64run6664.c"
    "${BOX64_ROOT}/src/emu/x64run66d9.c"
    "${BOX64_ROOT}/src/emu/x64run66dd.c"
    "${BOX64_ROOT}/src/emu/x64run66f0.c"
    "${BOX64_ROOT}/src/emu/x64run67.c"
    "${BOX64_ROOT}/src/emu/x64run67avx.c"
    "${BOX64_ROOT}/src/emu/x64run67_32.c"
    "${BOX64_ROOT}/src/emu/x64run6764_32.c"
    "${BOX64_ROOT}/src/emu/x64run670f.c"
    "${BOX64_ROOT}/src/emu/x64run6766.c"
    "${BOX64_ROOT}/src/emu/x64run67660f.c"
    "${BOX64_ROOT}/src/emu/x64rund8.c"
    "${BOX64_ROOT}/src/emu/x64rund9.c"
    "${BOX64_ROOT}/src/emu/x64runda.c"
    "${BOX64_ROOT}/src/emu/x64rundb.c"
    "${BOX64_ROOT}/src/emu/x64rundc.c"
    "${BOX64_ROOT}/src/emu/x64rundd.c"
    "${BOX64_ROOT}/src/emu/x64runde.c"
    "${BOX64_ROOT}/src/emu/x64rundf.c"
    "${BOX64_ROOT}/src/emu/x64runf0.c"
    "${BOX64_ROOT}/src/emu/x64runf20f.c"
    "${BOX64_ROOT}/src/emu/x64runf30f.c"
    "${BOX64_ROOT}/src/emu/x64runavx.c"
    "${BOX64_ROOT}/src/emu/x64runavx0f.c"
    "${BOX64_ROOT}/src/emu/x64runavx0f38.c"
    "${BOX64_ROOT}/src/emu/x64runavx660f.c"
    "${BOX64_ROOT}/src/emu/x64runavxf20f.c"
    "${BOX64_ROOT}/src/emu/x64runavxf30f.c"
    "${BOX64_ROOT}/src/emu/x64runavx660f38.c"
    "${BOX64_ROOT}/src/emu/x64runavx660f3a.c"
    "${BOX64_ROOT}/src/emu/x64runavxf20f38.c"
    "${BOX64_ROOT}/src/emu/x64runavxf30f38.c"
    "${BOX64_ROOT}/src/emu/x64runavxf20f3a.c"
    "${BOX64_ROOT}/src/emu/x64runavxf30f3a.c"
)

add_library(test_interpreter OBJECT ${INTERPRETER_SRC})
set_target_properties(test_interpreter PROPERTIES COMPILE_DEFINITIONS "TEST_INTERPRETER")

set(BOX64CPU_SRC
    "${BOX64_ROOT}/src/custommem.c"
    "${BOX64_ROOT}/src/dynarec/arm64/arm64_immenc.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_arch.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_functions.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_jmpnext.c"
    "${BOX64_ROOT}/src/dynarec/dynablock.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_native_functions.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_native.c"
    "${BOX64_ROOT}/src/dynarec/dynarec.c"
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
    "${BOX64_ROOT}/src/os/freq_wine.c"
    "${BOX64_ROOT}/src/os/symbolfuncs_wine.c"
    "${BOX64_ROOT}/src/os/emit_signal_wine.c"
    "${BOX64_ROOT}/src/os/perfmap.c"
    "${BOX64_ROOT}/src/os/my_cpuid_wine.c"
    "${BOX64_ROOT}/src/tools/alternate.c"
    "${BOX64_ROOT}/src/tools/rbtree.c"
)

add_library(wowbox64 SHARED ${MAIN_SRC} ${BOX64CPU_SRC} ${INTERPRETER_SRC} ${DYNAREC_ASM_SRC}
    $<TARGET_OBJECTS:dynarec_pass0>
    $<TARGET_OBJECTS:dynarec_pass1>
    $<TARGET_OBJECTS:dynarec_pass2>
    $<TARGET_OBJECTS:dynarec_pass3>
    $<TARGET_OBJECTS:test_interpreter>)

include_directories(
    "${BOX64_ROOT}/src/include"
    "${BOX64_ROOT}/src"
)

# always enable DynaRec, only supports ARM64 for now.
add_compile_definitions(DYNAREC ARM64)
target_link_libraries(wowbox64 PRIVATE winpthread)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)