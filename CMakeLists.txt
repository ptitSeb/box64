cmake_minimum_required(VERSION 3.0)

option(RPI4ARM64 "Set to ON if targeting an RaspberryPI4 device with multiarch arm64 and armhf" ${RPI4ARM64})
option(RK3326 "Set to ON if targeting an Rockchip RK3326 based device" ${RK3326})
option(RK3399 "Set to ON if targeting an Rockchip RK3399 based device" ${RK3399})
option(USE_CCACHE "Set to ON to use ccache if present in the system" ${USE_CCACHE})
option(HAVE_TRACE "Set to ON to have Trace ability (needs ZydisInfo library)" ${HAVE_TRACE})
option(NOGIT "Set to ON if not building from a git clone repo (like when building from a zip download from github)" ${NOGIT})
if(RK3399 OR RPI4ARM64 OR RK3326)
    set(LD80BITS OFF CACHE BOOL "")
    set(NOALIGN OFF CACHE BOOL "")
    set(ARM_DYNAREC ON CACHE BOOL "")
endif()
option(LD80BITS "Set to ON if host device have 80bits long double (i.e. i386)" ${LD80BITS})
option(NOALIGN "Set to ON if host device doesn't need re-align (i.e. i386)" ${NOALIGN})
option(ARM_DYNAREC "Set to ON to use ARM Dynamic Recompilation" ${ARM_DYNAREC})

if(${CMAKE_VERSION} VERSION_LESS "3.12.2")
    find_package(PythonInterp 3)
    if(NOT PYTHONINTERP_FOUND)
        message( FATAL_ERROR "You need a Python interpretor, CMake will exit." )
    endif()
    if(${PYTHON_VERSION_MAJOR} LESS 3)
        message( FATAL_ERROR "You need a Python 3 interpretor, CMake will exit." )
    endif()
else()
    find_package(Python3)
    if(NOT Python3_Interpreter_FOUND)
        message( FATAL_ERROR "You need a Python interpretor, CMake will exit." )
    endif()
    set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE} CACHE INTERNAL "The Python3 executable" FORCE)
endif()

project(box64 C ASM)

enable_testing()

set(default_build_type "RelwithDebInfo")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(RPI4ARM64)
    add_definitions(-DRPI)
    add_definitions(-DRPI4ARM64)
    add_definitions(-pipe -march=armv8-a+crc -mtune=cortex-a72 -mfpu=neon-fp-armv8)
    set(CMAKE_ASM_FLAGS  "-pipe -march=armv8-a+crc -mtune=cortex-a72 -mfpu=neon-fp-armv8")
endif()

if(RK3326)
    add_definitions(-DRK3326)
    add_definitions(-marm -mcpu=cortex-a35 -mfpu=neon-vfpv3 -march=armv8-a+crc+simd+crypto -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-marm -mcpu=cortex-a35 -mfpu=neon-vfpv3 -march=armv8-a+crc+simd+crypto -mfloat-abi=hard")
endif()

if(RK3399)
    add_definitions(-DRK3399)
    add_definitions(-pipe -march=armv8-a+crc+simd+crypto -mcpu=cortex-a72+crypto)
    set(CMAKE_ASM_FLAGS  "-pipe -march=armv8-a+crc+simd+crypto -mcpu=cortex-a72+crypto")
endif()

if(NOGIT)
    add_definitions(-DNOGIT)
endif()

if(HAVE_TRACE)
    set(BOX64 box64t)
else()
    set(BOX64 box64)
endif()

set(BOX64_ELF_ADDRESS "0x500062800000")  #random load address...

if(LD80BITS)
    add_definitions(-DHAVE_LD80BITS)
endif()

if(NOALIGN)
    add_definitions(-DNOALIGN)
endif()

if(HAVE_TRACE)
    add_definitions(-DHAVE_TRACE)
endif()

if(ARM_DYNAREC)
    add_definitions(-DDYNAREC)
    add_definitions(-DARM64)
    enable_language(ASM)
endif()

set(BOX64_ROOT ${CMAKE_SOURCE_DIR})

add_definitions(-std=gnu11 -funwind-tables -fvisibility=hidden)

if(USE_CCACHE)
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    endif()
endif()

include_directories(
    "${BOX64_ROOT}/src/include"
    "${BOX64_ROOT}/src"
    "${BOX64_ROOT}/src/wrapped/generated"
)

# git_head.h is a generated file
set_source_files_properties(
    "${BOX64_ROOT}/src/git_head.h"
    PROPERTIES GENERATED TRUE
    HEADER_FILE_ONLY TRUE)

set(ELFLOADER_SRC
    "${BOX64_ROOT}/src/main.c"
    "${BOX64_ROOT}/src/box64context.c"
    "${BOX64_ROOT}/src/build_info.c"
    "${BOX64_ROOT}/src/custommem.c"
    "${BOX64_ROOT}/src/dynarec/dynarec.c"
    "${BOX64_ROOT}/src/elfs/elfloader.c"
    "${BOX64_ROOT}/src/elfs/elfparser.c"
    "${BOX64_ROOT}/src/elfs/elfload_dump.c"
    "${BOX64_ROOT}/src/emu/x64emu.c"
    "${BOX64_ROOT}/src/emu/x64int3.c"
    "${BOX64_ROOT}/src/emu/x87emu_private.c"
    "${BOX64_ROOT}/src/emu/x64primop.c"
    "${BOX64_ROOT}/src/emu/x64run.c"
    "${BOX64_ROOT}/src/emu/x64run0f.c"
    "${BOX64_ROOT}/src/emu/x64run64.c"
    "${BOX64_ROOT}/src/emu/x64run66.c"
    "${BOX64_ROOT}/src/emu/x64run660f.c"
    "${BOX64_ROOT}/src/emu/x64run6664.c"
    "${BOX64_ROOT}/src/emu/x64run66d9.c"
    "${BOX64_ROOT}/src/emu/x64run66dd.c"
    "${BOX64_ROOT}/src/emu/x64run67.c"
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
    "${BOX64_ROOT}/src/emu/x64run_private.c"
    "${BOX64_ROOT}/src/emu/x64syscall.c"
    "${BOX64_ROOT}/src/emu/x64tls.c"
    "${BOX64_ROOT}/src/emu/x64trace.c"
    "${BOX64_ROOT}/src/librarian/librarian.c"
    "${BOX64_ROOT}/src/librarian/library.c"
    "${BOX64_ROOT}/src/libtools/auxval.c"
    "${BOX64_ROOT}/src/libtools/myalign.c"
    "${BOX64_ROOT}/src/libtools/sdl1rwops.c"
    "${BOX64_ROOT}/src/libtools/sdl2rwops.c"
    "${BOX64_ROOT}/src/libtools/signals.c"
    "${BOX64_ROOT}/src/libtools/threads.c"
    "${BOX64_ROOT}/src/tools/box64stack.c"
    "${BOX64_ROOT}/src/tools/bridge.c"
    "${BOX64_ROOT}/src/tools/callback.c"
    "${BOX64_ROOT}/src/tools/my_cpuid.c"
    "${BOX64_ROOT}/src/tools/pathcoll.c"
    "${BOX64_ROOT}/src/tools/fileutils.c"
    "${BOX64_ROOT}/src/tools/wine_tools.c"
    "${BOX64_ROOT}/src/wrapped/generated/wrapper.c"
)

set(WRAPPEDS
    "${BOX64_ROOT}/src/wrapped/wrappedalure.c"
    "${BOX64_ROOT}/src/wrapped/wrappedalut.c"
    "${BOX64_ROOT}/src/wrapped/wrappedbz2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedcairo.c"
    "${BOX64_ROOT}/src/wrapped/wrappedcrashhandler.c"
    "${BOX64_ROOT}/src/wrapped/wrappedcrypto.c"
    "${BOX64_ROOT}/src/wrapped/wrappedcurl.c"
    "${BOX64_ROOT}/src/wrapped/wrappeddbus.c"
    "${BOX64_ROOT}/src/wrapped/wrappedfontconfig.c"
    "${BOX64_ROOT}/src/wrapped/wrappedfreetype.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgnutls.c"
    "${BOX64_ROOT}/src/wrapped/wrappedldlinux.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibasound.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibc.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibcups.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibdl.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibgl.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibglu.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibice.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibm.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibogg.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibpthread.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibresolv.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibrt.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibsm.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibssl.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibuuid.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibvorbis.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibx11.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibx11xcb.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxau.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcb.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcomposite.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcursor.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxdamage.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxdmcp.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxext.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxfixes.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxi.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxmu.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxpm.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxrandr.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxrender.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxss.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxt.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxtst.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxxf86vm.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibz.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlzma.c"
    "${BOX64_ROOT}/src/wrapped/wrappedopenal.c"
    "${BOX64_ROOT}/src/wrapped/wrappedpango.c"
    "${BOX64_ROOT}/src/wrapped/wrappedpangocairo.c"
    "${BOX64_ROOT}/src/wrapped/wrappedpangoft2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedpulse.c"
    "${BOX64_ROOT}/src/wrapped/wrappedpulsesimple.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsdl1.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsdl1image.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsdl1mixer.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsdl1ttf.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsmpeg.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsdl2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsdl2image.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsdl2mixer.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsdl2ttf.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsmpeg2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedudev0.c"
    "${BOX64_ROOT}/src/wrapped/wrappedudev1.c"
    "${BOX64_ROOT}/src/wrapped/wrappedutil.c"
    "${BOX64_ROOT}/src/wrapped/wrappedvorbisfile.c"
    "${BOX64_ROOT}/src/wrapped/wrappedxinerama.c"
)

#set(WRAPPEDS_HEAD "${BOX64_ROOT}/src/wrapped/wrappedd3dadapter9_gen.h")
foreach(A ${WRAPPEDS})
    string(REPLACE ".c" "_private.h" B ${A})
    set(WRAPPEDS_HEAD ${WRAPPEDS_HEAD} ${B})
    set_source_files_properties(${A} PROPERTIES OBJECT_DEPENDS ${B})
endforeach()

set(WRAPPER "${BOX64_ROOT}/src/wrapped/generated/wrapper.c" "${BOX64_ROOT}/src/wrapped/generated/wrapper.h")

add_custom_command(
    OUTPUT "${BOX64_ROOT}/src/wrapped/generated/functions_list.txt"
    COMMAND "${PYTHON_EXECUTABLE}" "${BOX64_ROOT}/rebuild_wrappers.py" 
    "${BOX64_ROOT}" 
    "PANDORA" "HAVE_LD80BITS" "NOALIGN" "HAVE_TRACE" "--" 
    ${WRAPPEDS_HEAD}
    MAIN_DEPENDENCY "${BOX64_ROOT}/rebuild_wrappers.py"
    DEPENDS ${WRAPPEDS} ${WRAPPEDS_HEAD}
    BYPRODUCTS ${WRAPPER}
)

#add_custom_command(
#    OUTPUT "${BOX64_ROOT}/src/dynarec/last_run.txt"
#    COMMAND "${PYTHON_EXECUTABLE}" "${BOX64_ROOT}/rebuild_printer.py" "${BOX64_ROOT}"
#    MAIN_DEPENDENCY "${BOX64_ROOT}/rebuild_printer.py"
#    DEPENDS "${BOX64_ROOT}/src/dynarec/arm64_instructions.txt"
#    BYPRODUCTS "${BOX64_ROOT}/src/dynarec/arm64_printer.c"
#)

add_custom_target(WRAPPERS DEPENDS "${BOX64_ROOT}/src/wrapped/generated/functions_list.txt")
#add_custom_target(PRINTER DEPENDS "${BOX64_ROOT}/src/dynarec/last_run.txt")

if(ARM_DYNAREC)
    set(DYNAREC_SRC
        "${BOX64_ROOT}/src/dynarec/dynablock.c"

        "${BOX64_ROOT}/src/dynarec/dynarec_arm64.c"
        "${BOX64_ROOT}/src/dynarec/dynarec_arm64_functions.c"
        "${BOX64_ROOT}/src/dynarec/arm64_printer.c"

        "${BOX64_ROOT}/src/dynarec/arm64_prolog.S"
        "${BOX64_ROOT}/src/dynarec/arm64_epilog.S"
        "${BOX64_ROOT}/src/dynarec/arm64_next.S"
        "${BOX64_ROOT}/src/dynarec/arm64_lock.S"
    )

    set(DYNAREC_PASS
    "${BOX64_ROOT}/src/dynarec/dynarec_arm64_helper.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_arm64_emit_tests.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_arm64_emit_math.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_arm64_emit_logic.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_arm64_emit_shift.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_arm64_pass.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_arm64_00.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_arm64_0f.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_arm64_64.c"
    #"${BOX64_ROOT}/src/dynarec/dynarec_arm64_65.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_arm64_66.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_arm64_67.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_arm64_d8.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_arm64_d9.c"
    #"${BOX64_ROOT}/src/dynarec/dynarec_arm64_da.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_arm64_db.c"
    #"${BOX64_ROOT}/src/dynarec/dynarec_arm64_dc.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_arm64_dd.c"
    #"${BOX64_ROOT}/src/dynarec/dynarec_arm64_de.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_arm64_df.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_arm64_f0.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_arm64_660f.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_arm64_6664.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_arm64_f20f.c"
    "${BOX64_ROOT}/src/dynarec/dynarec_arm64_f30f.c"
    )

    set(DYNAREC_PASS "${BOX64_ROOT}/src/wrapped/generated/wrapper.h" ${DYNAREC_PASS})

    add_library(dynarec_arm OBJECT ${DYNAREC_SRC})

    add_library(arm64_pass0 OBJECT ${DYNAREC_PASS})
    set_target_properties(arm64_pass0 PROPERTIES COMPILE_FLAGS "-DSTEP=0")
    add_library(arm64_pass1 OBJECT ${DYNAREC_PASS})
    set_target_properties(arm64_pass1 PROPERTIES COMPILE_FLAGS "-DSTEP=1")
    add_library(arm64_pass2 OBJECT ${DYNAREC_PASS})
    set_target_properties(arm64_pass2 PROPERTIES COMPILE_FLAGS "-DSTEP=2")
    add_library(arm64_pass3 OBJECT ${DYNAREC_PASS})
    set_target_properties(arm64_pass3 PROPERTIES COMPILE_FLAGS "-DSTEP=3")
    add_dependencies(arm64_pass0 WRAPPERS)
    add_dependencies(arm64_pass1 WRAPPERS)
    add_dependencies(arm64_pass2 WRAPPERS)
    add_dependencies(arm64_pass3 WRAPPERS)

    add_library(dynarec STATIC 
        $<TARGET_OBJECTS:dynarec_arm> 
        $<TARGET_OBJECTS:arm64_pass0> 
        $<TARGET_OBJECTS:arm64_pass1>
        $<TARGET_OBJECTS:arm64_pass2>
        $<TARGET_OBJECTS:arm64_pass3>
    )

endif()

# creates git_head.h
if(ARM_DYNAREC)
    add_custom_command(
        OUTPUT "${BOX64_ROOT}/src/git_head.h"
        COMMAND bash -c "echo \\\#define GITREV \\\"$(git rev-parse --short HEAD)\\\">\"${BOX64_ROOT}/src/git_head.h\""
        DEPENDS dynarec ${ELFLOADER_SRC} ${WRAPPEDS}
        VERBATIM)
else()
    add_custom_command(
        OUTPUT "${BOX64_ROOT}/src/git_head.h"
        COMMAND bash -c "echo \\\#define GITREV \\\"$(git rev-parse --short HEAD)\\\">\"${BOX64_ROOT}/src/git_head.h\""
        DEPENDS ${ELFLOADER_SRC} ${WRAPPEDS}
        VERBATIM)
endif()

add_executable(${BOX64} ${ELFLOADER_SRC} ${WRAPPEDS} "${BOX64_ROOT}/src/git_head.h")
add_dependencies(${BOX64} WRAPPERS)
#add_dependencies(${BOX64} PRINTER)
target_link_libraries(${BOX64} m dl rt pthread)
if(ARM_DYNAREC)
    target_link_libraries(${BOX64} dynarec)
endif()

if(${CMAKE_VERSION} VERSION_LESS "3.13")
    set_target_properties(${BOX64} PROPERTIES LINK_FLAGS "-rdynamic -Wl,-Ttext-segment,${BOX64_ELF_ADDRESS}")
else()
    target_link_options(${BOX64} PUBLIC -rdynamic)
    target_link_options(${BOX64} PUBLIC -Wl,-Ttext-segment,${BOX64_ELF_ADDRESS})
endif()

string(COMPARE EQUAL "${CMAKE_HOST_SYSTEM_PROCESSOR}" "i686"  _x86)
string(COMPARE EQUAL "${CMAKE_HOST_SYSTEM_PROCESSOR}" "x86_64"  _x86_64)

if(NOT _x86 AND NOT _x86_64)
  install(TARGETS ${BOX64}
    RUNTIME DESTINATION bin)
  configure_file(system/box64.conf.cmake system/box64.conf)
  install(FILES ${CMAKE_BINARY_DIR}/system/box64.conf DESTINATION /etc/binfmt.d/)
  install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libstdc++.so.6 DESTINATION /usr/lib/x86_64-linux-gnu/)
  install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libgcc_s.so.1 DESTINATION /usr/lib/x86_64-linux-gnu/)
endif()

if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

add_test(test01 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test01 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref01.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test02 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test02 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref02.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test03 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test03 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref03.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test04 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test04 -D TEST_ARGS2=yeah -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref04.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test05 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test05 -D TEST_ARGS2=7 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref05.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test06 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test06 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref06.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test07 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test07 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref07.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test08 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test08 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref08.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test09 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test09 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref09.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test10 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test10 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref10.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test11 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test11 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref11.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test12 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test12 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref12.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test13 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test13 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref13.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

#add_test(test14 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64} 
#    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test14 -D TEST_OUTPUT=tmpfile.txt 
#    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref14.txt
#    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test15 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test15 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref15.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

file(GLOB extension_tests "${CMAKE_SOURCE_DIR}/tests/extensions/*.c")
foreach(file ${extension_tests})
    get_filename_component(testname "${file}" NAME_WE)
    add_test(NAME "${testname}" COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/extensions/${testname} -D TEST_OUTPUT=tmpfile.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/extensions/${testname}.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake)
endforeach()
