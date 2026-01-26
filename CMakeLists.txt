cmake_minimum_required(VERSION 3.13)

cmake_policy(SET CMP0065 NEW)

SET(CMAKE_C_FLAGS_DEBUG  "-O0 -g")
SET(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")

option(RPI3ARM64 "Set to ON if targeting an RaspberryPI3 device with multiarch arm64 and armhf" ${RPI3ARM64})
option(RPI4ARM64 "Set to ON if targeting an RaspberryPI4 device with multiarch arm64 and armhf" ${RPI4ARM64})
option(RPI5ARM64 "Set to ON if targeting an RaspberryPi5 device with multiarch arm64 and armhf" ${RPI5ARM64})
option(RK3326 "Set to ON if targeting an Rockchip RK3326 based device" ${RK3326})
option(RK3399 "Set to ON if targeting an Rockchip RK3399 based device" ${RK3399})
option(RK3588 "Set to ON if targeting an Rockchip RK3588(S) based device" ${RK3588})
option(ODROIDN2 "Set to ON if targeting an Odroid-N2 device" ${ODROIDN2})
option(TEGRAX1 "Set to ON if targeting an Tegra X1 based device" ${TEGRAX1})
option(TEGRA_T194 "Set to ON if targeting an Tegra Xavier based device" ${TEGRA_T194})
option(TEGRA_T234 "Set to ON if targeting an Tegra Orin based device" ${TEGRA_T234})
option(NVIDIA_GB10 "Set to ON if targeting a DGX Spark(GB10) based device" ${NVIDIA_GB10})
option(PHYTIUM "Set to ON if targeting an Phytium (D2000 or FT2000/4) based device" ${PHYTIUM})
option(SD845 "Set to ON if targeting a Snapdragon 845 based device" ${SD845})
option(SD865 "Set to ON if targeting a Snapdragon 865 based device" ${SD865})
option(SD888 "Set to ON if targeting a Snapdragon 888 based device" ${SD888})
option(SD8G2 "Set to ON if targeting a Snapdragon 8 Gen 2 based device" ${SD8G2})
option(SDORYON1 "Set to ON if targeting a Snapdragon Oryon 1 (X1E80100/X1E78100) based device" ${SDORYON1})
option(ADLINK "Set to ON if targeting an ADLink AmpereAltra based device" ${ADLINK})
option(M1 "Set to ON if targeting a AppleM1 running on Asahi computer" ${M1})
option(LARCH64 "Set to ON if targeting an Loongarch64 based device" ${LARCH64})
option(RV64 "Set to ON if targeting an RISC-V RV64GC based device" ${RV64})
option(PPC64LE "Set to ON if targeting an PowerPC 64 LE based device" ${PPC64LE})
option(LX2160A "Set to ON if targeting an LX2160A based device" ${LX2160A})
option(ARM64 "Set to ON if targeting a generic ARM64 based device" ${ARM64})
option(ANDROID "Set to ON if targeting an Android device" ${ANDROID})
option(TERMUX "Set to ON if targeting an Android device with Termux" ${TERMUX})
option(WINLATOR_GLIBC "Set to ON if targeting an Android device with Winlator Glibc" ${WINLATOR_GLIBC})
option(USE_CCACHE "Set to ON to use ccache if present in the system" ${USE_CCACHE})
option(HAVE_TRACE "Set to ON to have Trace ability (needs libzydis-dev)" ${HAVE_TRACE})
option(ZYDIS3 "Set to ON if you have trace and an old libzydis v3.x" ${ZYDIS3})
option(SAVE_MEM "Set to ON to build dynarec with some slower memory saving optimizations" ${SAVE_MEM})
option(NOLOADADDR "Set to ON to avoid fixing the load address of Box64" OFF)
option(NOGIT "Set to ON if not building from a git clone repo (like when building from a zip download from github)" ${NOGIT})
option(BAD_SIGNAL "Set to ON to activate the workaround for incoherent si_info on SIGSEGV" ${BAD_SIGNAL})
option(BAD_PKILL "Set to ON to activate the workaround for incoherent pthread_kill" ${BAD_PKILL})
option(SW64 "Set ON if targeting an SW64 based device" ${SW64})
option(CI "Set to ON if running in CI" ${CI})
option(WITH_MOLD "Set to ON to use with mold" ${WITH_MOLD})
option(BOX32 "Set to ON to add Linux 32bits support (experimental, do not use)" ${BOX32})
option(BOX32_BINFMT "Also setup binfmt integration for box32" ${BOX32_BINFMT})
option(LARCH64_ABI_1 "Set to ON if building for Loongarch64 ABI 1.0 system" ${LARCH64_ABI_1})
option(GDBJIT "Set to ON to enable GDB JIT support" ${GDBJIT})
option(WOW64 "Set to ON if you want a WOW64 PE build in addition")

if(TERMUX)
    set(TERMUX_PATH "/data/data/com.termux/files")
    set(ANDROID ON CACHE BOOL "")
endif()

# Hack: limit the optimization level to -O1 for some platforms
if(TERMUX OR ANDROID)
    set(CMAKE_C_FLAGS_RELEASE "-O1")
    set(CMAKE_CXX_FLAGS_RELEASE "-O1")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O1 -g")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O1 -g")
endif()

if(LARCH64)
    set(LD80BITS OFF CACHE BOOL "")
    set(NOALIGN OFF CACHE BOOL "")
    set(ARM_DYNAREC OFF CACHE BOOL "")
    set(RV64_DYNAREC OFF CACHE BOOL "")
    set(LARCH64_DYNAREC ON CACHE BOOL "")
    set(BAD_PKILL ON CACHE BOOL "")
    if(LARCH64_ABI_1)
        message(STATUS "Build for Loongarch64 ABI 1.0 system")
        add_definitions(-DLA64_ABI_1)
    endif()
endif()
if(RV64)
    set(LD80BITS OFF CACHE BOOL "")
    set(NOALIGN OFF CACHE BOOL "")
    set(ARM_DYNAREC OFF CACHE BOOL "")
    set(RV64_DYNAREC ON CACHE BOOL "")
    set(LARCH64_DYNAREC OFF CACHE BOOL "")
endif()
if(PPC64LE)
    set(LD80BITS OFF CACHE BOOL "")
    set(NOALIGN OFF CACHE BOOL "")
    set(ARM_DYNAREC OFF CACHE BOOL "")
    set(RV64_DYNAREC OFF CACHE BOOL "")
    set(LARCH64_DYNAREC OFF CACHE BOOL "")
endif()
if(RK3399 OR RK3588 OR ODROIDN2 OR RPI3ARM64 OR RPI4ARM64 OR RPI5ARM64 OR RK3326 OR TEGRAX1 OR TEGRA_T194 OR TEGRA_T234 OR NVIDIA_GB10 OR PHYTIUM OR SD845 OR SD865 OR SD888 OR SD8G2 OR SDORYON1 OR LX2160A OR M1 OR ARM64 OR ADLINK)
    set(LD80BITS OFF CACHE BOOL "")
    set(NOALIGN OFF CACHE BOOL "")
    set(ARM_DYNAREC ON CACHE BOOL "")
    set(RV64_DYNAREC OFF CACHE BOOL "")
    set(LARCH64_DYNAREC OFF CACHE BOOL "")
endif()
if(RK3399 OR ODROIDN2 OR RPI3ARM64 OR RPI4ARM64 OR RPI5ARM64)
    set(SAVE_MEM ON CACHE BOOL "")
endif()
if(RK3588)
    set(BAD_SIGNAL ON CACHE BOOL "")
endif()
if(SW64)
    set(LD80BITS OFF CACHE BOOL "")
    set(NOALIGN OFF CACHE BOOL "")
endif()
if(ANDROID)
    set(TERMUX_PATH "/data/data/com.termux/files")
    #set(NOLOADADDR ON CACHE BOOL "")  #no need to force this setting it seems now
    set(BAD_SIGNAL ON CACHE BOOL "")
endif()

if(GDBJIT)
    message(STATUS "Build with GDBJIT support")
    add_definitions(-DGDBJIT)
endif()

option(LD80BITS "Set to ON if host device have 80bits long double (i.e. i386)" ${LD80BITS})
option(NOALIGN "Set to ON if host device doesn't need re-align (i.e. i386)" ${NOALIGN})
option(ARM_DYNAREC "Set to ON to use ARM Dynamic Recompilation" ${ARM_DYNAREC})
option(RV64_DYNAREC "Set to ON to use RISC-V Dynamic Recompilation" ${RV64_DYNAREC})
option(LARCH64_DYNAREC "Set to ON to use LOONGARCH64 Dynamic Recompilation" ${LARCH64_DYNAREC})
option(STATICBUILD "Set to ON to have a static build (Warning, not working)" ${STATICBUILD})
option(NO_LIB_INSTALL "Set ON to not install a few x86_64 libs that are used by many program" ${NO_LIB_INSTALL})
option(NO_CONF_INSTALL "Set ON to not install config files" ${NO_CONF_INSTALL})

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

project(box64 C ASM)

enable_testing()

set(default_build_type "RelWithDebInfo")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(RPI3ARM64)
    add_definitions(-DRPI)
    add_definitions(-DRPI3ARM64)
    set(CFLAGS -pipe -march=armv8-a+crc -mtune=cortex-a53)
elseif(RPI4ARM64)
    add_definitions(-DRPI)
    add_definitions(-DRPI4ARM64)
    set(CFLAGS -pipe -march=armv8-a+crc -mtune=cortex-a72)
elseif(RPI5ARM64)
    add_definitions(-DRPI)
    add_definitions(-DRPI5ARM64)
    set(CFLAGS -pipe -march=armv8.2-a+crc+crypto+fp16+rcpc+dotprod -mtune=cortex-a76)
elseif(RK3326)
    add_definitions(-DRK3326)
    set(CFLAGS -pipe -march=armv8-a+crc+simd+crypto -mcpu=cortex-a35+crypto)
elseif(RK3399)
    add_definitions(-DRK3399)
    set(CFLAGS -pipe -march=armv8-a+crc+simd+crypto -mcpu=cortex-a72.cortex-a53+crypto)
elseif(RK3588)
    add_definitions(-DRK3588)
    add_definitions(-DBAD_SIGNAL)
    set(CFLAGS -pipe -mcpu=cortex-a76.cortex-a55+crypto)
elseif(ODROIDN2)
    add_definitions(-DODROIDN2)
    set(CFLAGS -march=armv8-a+crc+simd+crypto -mcpu=cortex-a73.cortex-a53+crypto)
elseif(TEGRAX1)
    add_definitions(-DTEGRAX1)
    set(CFLAGS -pipe -march=armv8-a+crc+simd+crypto -mcpu=cortex-a57+crypto)
elseif(TEGRA_T194)
    add_definitions(-DTEGRA_T194)
    set(CFLAGS -pipe -march=armv8.2-a+fp16+simd+crypto+predres -mcpu=cortex-a76+crypto -mtune=cortex-a76)
elseif(TEGRA_T234)
    add_definitions(-DTEGRA_T234)
    set(CFLAGS -pipe -march=armv8.2-a+fp16+simd+crypto+predres -mcpu=cortex-a78ae+crypto -mtune=cortex-a78ae)
elseif(NVIDIA_GB10)
    add_definitions(-DNVIDIA_GB10)
    set(CFLAGS -pipe -mcpu=gb10 -mtune=gb10)
elseif(PHYTIUM)
    add_definitions(-DPHYTIUM)
    set(CFLAGS -pipe -march=armv8.1-a+crc+simd+crypto)
elseif(SD845)
    add_definitions(-DSD845)
    set(CFLAGS -pipe -march=armv8.2-a+simd+crypto -mtune=cortex-a75.cortex-a55)
elseif(SD865)
    add_definitions(-DSD865)
    #note that cortex-a77.cortex-a55 is not supported, so fall back to a76 instead
    set(CFLAGS -pipe -march=armv8.2-a+simd+crypto -mtune=cortex-a76.cortex-a55)
elseif(SD888)
    add_definitions(-DSD888)
    set(CFLAGS -pipe -march=armv8.4-a+simd+crypto)
elseif(SD8G2)
    add_definitions(-DSD8G2)
    set(CFLAGS -pipe -march=armv9-a+i8mm+sm4+sha3+rcpc+crypto+nosve+nosve2)
elseif(SDORYON1)
    add_definitions(-DSDORYON1)
    # NOTE
    #
    # 1.
    # GCC 14 and Clang 19 support oryon-1 as a -mcpu/-mtune argument,
    # however it seems like fedora (the system I tested this on) does not work.
    # see https://gcc.gnu.org/onlinedocs/gcc-14.2.0/gcc/AArch64-Options.html if you're interested
    #
    # 2.
    # gcc/gcc/config/aarch64/aarch64-cores.def:157 defines oryon1 as
    # AARCH64_CORE("oryon-1", oryon1, cortexa57, V8_6A, (CRYPTO, SM4, SHA3, F16), cortexa72,   0x51, 0x001, -1)
    # where
    #  * the 3rd positional argument is represented as the "scheduler"
    #  * the 6th positional argument is represented as the "cost"
    #  I've chosen the a57 because I don't know if I should choose the a72
    set(CFLAGS -pipe -march=armv8.6-a+crypto+sm4+sha3+fp16 -mtune=cortex-a57)
elseif(ADLINK)
    add_definitions(-DADLINK)
    set(CFLAGS -pipe -mcpu=neoverse-n1 -fuse-ld=gold -fuse-linker-plugin)
elseif(M1)
    add_definitions(-DM1)
    set(CFLAGS -pipe -march=armv8.5-a+simd+crypto)
elseif(LARCH64)
    add_definitions(-DLA64)
    add_definitions(-DBAD_PKILL)
    set(CFLAGS -pipe -march=loongarch64)
    set(ASMFLAGS  -pipe -march=loongarch64)
elseif(RV64)
    add_definitions(-DRV64)
    set(CFLAGS -pipe -march=rv64gc)
    set(ASMFLAGS  -pipe -march=rv64gc)
elseif(PPC64LE)
    add_definitions(-DPPC64LE)
elseif(LX2160A)
    add_definitions(-DLX2160A)
    set(CFLAGS -pipe -march=armv8-a+crypto+crc -mcpu=cortex-a72+crypto)
elseif(SW64)
    add_definitions(-DSW64)
elseif(ARM64)
    #add_definitions(-pipe -march=native)
endif()
if(ARM_DYNAREC)
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU" AND CMAKE_C_COMPILER_VERSION VERSION_LESS "9.0")
        set(ASMFLAGS  -pipe -march=armv8.2-a+crc+simd+crypto)
    else()
        #putting a random cpu that have 8.2 architecture so assembly can be build. the use of the feature are conditionnal anyway
        set(ASMFLAGS  -pipe -mcpu=cortex-a76)
    endif()
endif()
if(ANDROID)
    add_definitions(-DANDROID)
endif()
if(TERMUX)
    add_definitions(-DTERMUX)
endif()
if(WINLATOR_GLIBC)
    add_definitions(-DWINLATOR_GLIBC)
endif()
if(BAD_SIGNAL)
    add_definitions(-DBAD_SIGNAL)
endif()
if(BAD_PKILL)
    add_definitions(-DBAD_PKILL)
endif()
if(SAVE_MEM)
    add_definitions(-DSAVE_MEM)
endif()
if(STATICBUILD)
    #-fno-pic -mcmodel=large
    add_definitions(-DSTATICBUILD)
    add_definitions(-Wno-deprecated-declarations)   #by-design, deprecated function can be used
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_EXE_LINKER_FLAGS -static)
    set(HAVE_TRACE OFF)
endif()
if(BOX32)
    add_definitions(-DBOX32)
    add_definitions(-Wno-address-of-packed-member)  #32bits might generated unaligned pointers
endif()

if(NOGIT)
    add_definitions(-DNOGIT)
endif()
if(CI)
    add_definitions(-Wno-pointer-type-mismatch)
endif()

if(HAVE_TRACE)
    set(BOX64 box64)
else()
    set(BOX64 box64)
endif()

#set(BOX64_ELF_ADDRESS "0x500062800000")  #random load address...
set(BOX64_ELF_ADDRESS "0x34800000")  #low address

if(LD80BITS)
    add_definitions(-DHAVE_LD80BITS)
endif()

if(NOALIGN)
    add_definitions(-DNOALIGN)
endif()

if(HAVE_TRACE)
    add_definitions(-DHAVE_TRACE)
endif()

if(ZYDIS3)
    add_definitions(-DZYDIS3)
endif()

if(ARM_DYNAREC)
    add_definitions(-DDYNAREC)
    add_definitions(-DARM64)
    enable_language(ASM)
    include_directories("${BOX64_ROOT}/src/dynarec/arm64")
    set(DYNAREC ON)
elseif(RV64_DYNAREC)
    add_definitions(-DDYNAREC)
    add_definitions(-DRV64)
    enable_language(ASM)
    include_directories("${BOX64_ROOT}/src/dynarec/rv64")
    set(DYNAREC ON)
elseif(LARCH64_DYNAREC)
    add_definitions(-DDYNAREC)
    add_definitions(-DLA64)
    enable_language(ASM)
    include_directories("${BOX64_ROOT}/src/dynarec/la64")
    set(DYNAREC ON)
else()
    set(DYNAREC OFF)
endif()

set(BOX64_ROOT ${CMAKE_SOURCE_DIR})

add_definitions(-std=gnu11 -funwind-tables -fvisibility=hidden)

if(USE_CCACHE)
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    else()
        message(SEND_ERROR "ccache not found!")
    endif()
endif()

include_directories(
    "${BOX64_ROOT}/src/include"
    "${BOX64_ROOT}/src"
    "${BOX64_ROOT}/src/wrapped/generated"
)
if(BOX32)
    include_directories(
        "${BOX64_ROOT}/src/wrapped32/generated"
    )
endif()

# git_head.h is a generated file
set_source_files_properties(
    "${BOX64_ROOT}/src/git_head.h"
    PROPERTIES GENERATED TRUE
    HEADER_FILE_ONLY TRUE)

set(OS_LINUX_SRC
    "${BOX64_ROOT}/src/os/backtrace.c"
    "${BOX64_ROOT}/src/os/emit_signals_linux.c"
    "${BOX64_ROOT}/src/os/hostext_common.c"
    "${BOX64_ROOT}/src/os/hostext_linux.c"
    "${BOX64_ROOT}/src/os/freq_linux.c"
    "${BOX64_ROOT}/src/os/os_linux.c"
    "${BOX64_ROOT}/src/os/perfmap.c"
    "${BOX64_ROOT}/src/os/symbolfuncs_linux.c"
    "${BOX64_ROOT}/src/os/random_linux.c"
    "${BOX64_ROOT}/src/os/my_cpuid.c"
    "${BOX64_ROOT}/src/os/sysinfo.c"
)

set(ELFLOADER_SRC
    "${BOX64_ROOT}/src/main.c"
    "${BOX64_ROOT}/src/test.c"
    "${BOX64_ROOT}/src/core.c"
    "${BOX64_ROOT}/src/box64context.c"
    "${BOX64_ROOT}/src/build_info.c"
    "${BOX64_ROOT}/src/custommem.c"
    "${BOX64_ROOT}/src/custommmap.c"
    "${BOX64_ROOT}/src/dynarec/dynarec.c"
    "${BOX64_ROOT}/src/elfs/elfloader.c"
    "${BOX64_ROOT}/src/elfs/elfparser.c"
    "${BOX64_ROOT}/src/elfs/elfhash.c"
    "${BOX64_ROOT}/src/elfs/elfload_dump.c"
    "${BOX64_ROOT}/src/elfs/elfdwarf_private.c"
    "${BOX64_ROOT}/src/emu/entrypoint.c"
    "${BOX64_ROOT}/src/emu/x64compstrings.c"
    "${BOX64_ROOT}/src/emu/x64emu.c"
    "${BOX64_ROOT}/src/emu/x64printer.c"
    "${BOX64_ROOT}/src/emu/x64int3.c"
    "${BOX64_ROOT}/src/emu/x87emu_private.c"
    "${BOX64_ROOT}/src/emu/x64primop.c"
    "${BOX64_ROOT}/src/emu/x64run_private.c"
    "${BOX64_ROOT}/src/emu/x64shaext.c"
    "${BOX64_ROOT}/src/emu/x64syscall.c"
    "${BOX64_ROOT}/src/emu/x64tls.c"
    "${BOX64_ROOT}/src/emu/x64trace.c"
    "${BOX64_ROOT}/src/librarian/librarian.c"
    "${BOX64_ROOT}/src/librarian/library.c"
    "${BOX64_ROOT}/src/librarian/dictionnary.c"
    "${BOX64_ROOT}/src/librarian/symbols.c"
    "${BOX64_ROOT}/src/libtools/auxval.c"
    "${BOX64_ROOT}/src/libtools/myalign.c"
    "${BOX64_ROOT}/src/libtools/signals.c"
    "${BOX64_ROOT}/src/libtools/decopcode.c"
    "${BOX64_ROOT}/src/libtools/threads.c"
    "${BOX64_ROOT}/src/tools/bitutils.c"
    "${BOX64_ROOT}/src/tools/box64stack.c"
    "${BOX64_ROOT}/src/tools/bridge.c"
    "${BOX64_ROOT}/src/tools/alternate.c"
    "${BOX64_ROOT}/src/tools/callback.c"
    "${BOX64_ROOT}/src/tools/cleanup.c"
    "${BOX64_ROOT}/src/tools/gdbjit.c"
    "${BOX64_ROOT}/src/tools/fileutils.c"
    "${BOX64_ROOT}/src/tools/pathcoll.c"
    "${BOX64_ROOT}/src/tools/rbtree.c"
    "${BOX64_ROOT}/src/tools/env.c"
    "${BOX64_ROOT}/src/tools/wine_tools.c"
    "${BOX64_ROOT}/src/tools/pe_tools.c"
    "${BOX64_ROOT}/src/wrapped/generated/wrapper.c"
)
if(NOT STATICBUILD)
    list(APPEND ELFLOADER_SRC
        "${BOX64_ROOT}/src/mallochook.c"
        "${BOX64_ROOT}/src/steam.c"
        "${BOX64_ROOT}/src/libtools/sdl1rwops.c"
        "${BOX64_ROOT}/src/libtools/sdl2rwops.c"
        "${BOX64_ROOT}/src/tools/gtkclass.c"
        "${BOX64_ROOT}/src/librarian/globalsymbols.c"
        )
endif()
if(BOX32)
    list(APPEND ELFLOADER_SRC
        "${BOX64_ROOT}/src/box32.c"
        "${BOX64_ROOT}/src/elfs/elfhash32.c"
        "${BOX64_ROOT}/src/elfs/elfloader32.c"
        "${BOX64_ROOT}/src/elfs/elfparser32.c"
        "${BOX64_ROOT}/src/elfs/elfload_dump32.c"
        "${BOX64_ROOT}/src/tools/box32stack.c"
        "${BOX64_ROOT}/src/emu/x86int3.c"
        "${BOX64_ROOT}/src/librarian/globalsymbols32.c"
        "${BOX64_ROOT}/src/libtools/box32_inputevent.c"
        "${BOX64_ROOT}/src/libtools/myalign32.c"
        "${BOX64_ROOT}/src/libtools/myalign64_32.c"
        "${BOX64_ROOT}/src/libtools/myalignxcb32.c"
        "${BOX64_ROOT}/src/libtools/signal32.c"
        "${BOX64_ROOT}/src/libtools/threads32.c"
        "${BOX64_ROOT}/src/libtools/libc_net32.c"
        "${BOX64_ROOT}/src/libtools/sdl1align32.c"
        "${BOX64_ROOT}/src/libtools/sdl2align32.c"
        "${BOX64_ROOT}/src/libtools/my_x11_conv.c"
        "${BOX64_ROOT}/src/libtools/my_x11_xevent.c"
        "${BOX64_ROOT}/src/emu/x86syscall_32.c"
        "${BOX64_ROOT}/src/wrapped32/generated/wrapper32.c"
        "${BOX64_ROOT}/src/wrapped32/generated/converter32.c"
    )
else()
    list(APPEND ELFLOADER_SRC
        "${BOX64_ROOT}/src/emu/x86syscall.c"
    )
endif()
if(NOT ANDROID)
    list(APPEND ELFLOADER_SRC
        "${BOX64_ROOT}/src/libtools/obstack.c"
    )
endif()

set(INTERPRETER
    "${BOX64_ROOT}/src/emu/x64run.c"
    "${BOX64_ROOT}/src/emu/x64run0f.c"
    "${BOX64_ROOT}/src/emu/x64run66.c"
    "${BOX64_ROOT}/src/emu/x64run660f.c"
    "${BOX64_ROOT}/src/emu/x64run66f20f.c"
    "${BOX64_ROOT}/src/emu/x64run66f30f.c"
    "${BOX64_ROOT}/src/emu/x64run66d9.c"
    "${BOX64_ROOT}/src/emu/x64run66dd.c"
    "${BOX64_ROOT}/src/emu/x64run66f0.c"
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

if(STATICBUILD)
set(WRAPPEDS
    "${BOX64_ROOT}/src/wrapped/wrappedldlinux.c"
    #"${BOX64_ROOT}/src/wrapped/wrappedlibbsd.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibc.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibcmusl.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibdl.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibm.c"
    "${BOX64_ROOT}/src/wrapped/wrappedutil.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibresolv.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibrt.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibpthread.c"
)
else()
set(WRAPPEDS
    "${BOX64_ROOT}/src/wrapped/wrappedalure.c"
    "${BOX64_ROOT}/src/wrapped/wrappedalut.c"
    "${BOX64_ROOT}/src/wrapped/wrappedatk.c"
    "${BOX64_ROOT}/src/wrapped/wrappedatkbridge.c"
    "${BOX64_ROOT}/src/wrapped/wrappedatomic.c"
    "${BOX64_ROOT}/src/wrapped/wrappedatspi.c"
    "${BOX64_ROOT}/src/wrapped/wrappedayatanaappindicator3.c"
    "${BOX64_ROOT}/src/wrapped/wrappedbz2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedcap.c"
    "${BOX64_ROOT}/src/wrapped/wrappedcuda.c"
    "${BOX64_ROOT}/src/wrapped/wrappedcairo.c"
    "${BOX64_ROOT}/src/wrapped/wrappedcairogobject.c"
    "${BOX64_ROOT}/src/wrapped/wrappedcrashhandler.c"
    "${BOX64_ROOT}/src/wrapped/wrappedcrypto.c"
    "${BOX64_ROOT}/src/wrapped/wrappedcrypto3.c"
    "${BOX64_ROOT}/src/wrapped/wrappedcurl.c"
    "${BOX64_ROOT}/src/wrapped/wrappeddbus.c"
    "${BOX64_ROOT}/src/wrapped/wrappeddbusglib1.c"
    "${BOX64_ROOT}/src/wrapped/wrappeddbusmenuglib.c"
    "${BOX64_ROOT}/src/wrapped/wrappeddecor0.c"
    "${BOX64_ROOT}/src/wrapped/wrappedevent21.c"
    "${BOX64_ROOT}/src/wrapped/wrappedexpat.c"
    "${BOX64_ROOT}/src/wrapped/wrappedfaudio.c"
    "${BOX64_ROOT}/src/wrapped/wrappedflac.c"
    "${BOX64_ROOT}/src/wrapped/wrappedfontconfig.c"
    "${BOX64_ROOT}/src/wrapped/wrappedfreebl3.c"
    "${BOX64_ROOT}/src/wrapped/wrappedfreetype.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgbm.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgconf2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgcrypt.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgdk3.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgdkpixbuf2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgdkx112.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgio2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedglesv2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedglib2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgmodule2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgmp.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgnutls.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgobject2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgomp.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgssapi.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgssapikrb5.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgstallocators.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgstapp.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgstaudio.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgstbase.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgstcheck.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgstcontroller.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgstfft.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgstgl.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgstnet.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgstpbutils.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgstreamer.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgstriff.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgstrtp.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgstrtsp.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgstsdp.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgsttag.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgstvideo.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgthread2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgtk3.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgtkx112.c"
    "${BOX64_ROOT}/src/wrapped/wrappedkrb5.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlcms2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedldlinux.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibasound.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibavcodec58.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibavformat58.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibavutil56.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibblas.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibbsd.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibc.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibcmusl.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibcrypt.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibcups.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibcupsimage.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibdl.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibdrm.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibdrmamdgpu.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibegl.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibform.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibformw.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibformw6.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibfuse.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibgl.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibglu.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibglx.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibglxnvidia.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibharfbuzz.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibibus.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibice.c"
    "${BOX64_ROOT}/src/wrapped/wrappedliblapack.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibm.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibncurses.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibncurses6.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibncursesw.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibncursesw6.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibnuma.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibogg.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibpanel.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibpci.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibpcre.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibpthread.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibresolv.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibrt.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibsm.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibsndfile.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibssl.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibssl3.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibtinfo.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibtinfo6.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibusb1.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibuuid.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibva.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibvadrm.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibvawayland.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibvax11.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibvdpau.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibvorbis.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibx11.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibx11xcb.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxau.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxaw.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcb.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbcursor.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbdri2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbdri3.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbglx.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbkeysyms.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbicccm.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbimage.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbpresent.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbrandr.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbrender.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbrenderutil.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbres.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbshape.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbshm.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbsync.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbutil.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbxinerama.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbxinput.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbxfixes.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbxkb.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbxtest.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcomposite.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcursor.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxdamage.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxdmcp.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxext.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxfixes.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxft.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxi.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxmu.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxpm.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxpresent.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxrandr.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxrender.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxss.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxt.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxtst.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxxf86vm.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibz.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlzma.c"
    "${BOX64_ROOT}/src/wrapped/wrappedmpg123.c"
    "${BOX64_ROOT}/src/wrapped/wrappednotify.c"
    "${BOX64_ROOT}/src/wrapped/wrappednsl.c"
    "${BOX64_ROOT}/src/wrapped/wrappednspr4.c"
    "${BOX64_ROOT}/src/wrapped/wrappednss3.c"
    "${BOX64_ROOT}/src/wrapped/wrappednssutil3.c"
    "${BOX64_ROOT}/src/wrapped/wrappednvml.c"
    "${BOX64_ROOT}/src/wrapped/wrappedopenal.c"
    "${BOX64_ROOT}/src/wrapped/wrappedopencl.c"
    "${BOX64_ROOT}/src/wrapped/wrappedpango.c"
    "${BOX64_ROOT}/src/wrapped/wrappedpangocairo.c"
    "${BOX64_ROOT}/src/wrapped/wrappedpangoft2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedpam.c"
    "${BOX64_ROOT}/src/wrapped/wrappedpcap.c"
    "${BOX64_ROOT}/src/wrapped/wrappedplc4.c"
    "${BOX64_ROOT}/src/wrapped/wrappedplds4.c"
    "${BOX64_ROOT}/src/wrapped/wrappedpng16.c"
    "${BOX64_ROOT}/src/wrapped/wrappedpulse.c"
    "${BOX64_ROOT}/src/wrapped/wrappedpulsemainloopglib.c"
    "${BOX64_ROOT}/src/wrapped/wrappedpulsesimple.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsdl1.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsdl1image.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsdl1mixer.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsdl1net.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsdl1sound.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsdl1ttf.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsecret1.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsoftokn3.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsmpeg.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsdl2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsdl2image.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsdl2mixer.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsdl2net.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsdl2ttf.c"
    "${BOX64_ROOT}/src/wrapped/wrappedselinux.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsmime3.c"
    "${BOX64_ROOT}/src/wrapped/wrappedsmpeg2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedssl3.c"
    "${BOX64_ROOT}/src/wrapped/wrappedtbbbind.c"
    "${BOX64_ROOT}/src/wrapped/wrappedtbbmalloc.c"
    "${BOX64_ROOT}/src/wrapped/wrappedtbbmallocproxy.c"
    "${BOX64_ROOT}/src/wrapped/wrappedtcmallocminimal.c"
    "${BOX64_ROOT}/src/wrapped/wrappedudev0.c"
    "${BOX64_ROOT}/src/wrapped/wrappedudev1.c"
    "${BOX64_ROOT}/src/wrapped/wrappedunwind.c"
    "${BOX64_ROOT}/src/wrapped/wrappedutil.c"
    "${BOX64_ROOT}/src/wrapped/wrappedvorbisfile.c"
    "${BOX64_ROOT}/src/wrapped/wrappedwaylandclient.c"
    "${BOX64_ROOT}/src/wrapped/wrappedwaylandcursor.c"
    "${BOX64_ROOT}/src/wrapped/wrappedwaylandegl.c"
    "${BOX64_ROOT}/src/wrapped/wrappedxinerama.c"
    "${BOX64_ROOT}/src/wrapped/wrappedxkbcommon.c"
    "${BOX64_ROOT}/src/wrapped/wrappedxkbcommonx11.c"
    "${BOX64_ROOT}/src/wrapped/wrappedxkbregistry.c"
    "${BOX64_ROOT}/src/wrapped/wrappedxml2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedxslt.c"
    "${BOX64_ROOT}/src/wrapped/wrappedldapr.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlber.c"
    "${BOX64_ROOT}/src/wrapped/wrappedvulkan.c"
    "${BOX64_ROOT}/src/wrapped/wrappedxshmfence.c"
    "${BOX64_ROOT}/src/wrapped/wrappedd3dadapter9.c"
    "${BOX64_ROOT}/src/wrapped/wrappedicuuc64.c"
    "${BOX64_ROOT}/src/wrapped/wrappedicui18n64.c"
    "${BOX64_ROOT}/src/wrapped/wrappedicuuc66.c"
    "${BOX64_ROOT}/src/wrapped/wrappedicui18n66.c"
    "${BOX64_ROOT}/src/wrapped/wrappedicuuc67.c"
    "${BOX64_ROOT}/src/wrapped/wrappedicui18n67.c"
    "${BOX64_ROOT}/src/wrapped/wrappedicuuc72.c"
    "${BOX64_ROOT}/src/wrapped/wrappedicui18n72.c"
    "${BOX64_ROOT}/src/wrapped/wrappedicuuc73.c"
    "${BOX64_ROOT}/src/wrapped/wrappedicui18n73.c"
    "${BOX64_ROOT}/src/wrapped/wrappedicuuc74.c"
    "${BOX64_ROOT}/src/wrapped/wrappedicui18n74.c"
    "${BOX64_ROOT}/src/wrapped/wrappedicuuc75.c"
    "${BOX64_ROOT}/src/wrapped/wrappedicui18n75.c"
    "${BOX64_ROOT}/src/wrapped/wrappedicuuc76.c"
    "${BOX64_ROOT}/src/wrapped/wrappedicui18n76.c"
    "${BOX64_ROOT}/src/wrapped/wrappedidn2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedanl.c"
    "${BOX64_ROOT}/src/wrapped/wrappedpsl5.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibssh2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedp11kit.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibtasn1.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibnettle8.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibunistring2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibhogweed6.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibsqlite3.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibtiff5.c"
    "${BOX64_ROOT}/src/wrapped/wrappedbrotlidec.c"
    "${BOX64_ROOT}/src/wrapped/wrappedzstd.c"
)
endif()
if(ANDROID)
    list(APPEND WRAPPEDS
        "${BOX64_ROOT}/src/wrapped/wrappedandroidshmem.c"
    )
endif()
if(TERMUX)
    list(APPEND WRAPPEDS
        "${BOX64_ROOT}/src/wrapped/wrappediconv.c"
        "${BOX64_ROOT}/src/wrapped/wrappedtermuxexec.c"
        "${BOX64_ROOT}/src/wrapped/wrappedandroidsupport.c"
    )
endif()

# If BOX64_ROOT contains a ".c", the build breaks...
string(REPLACE ".c" "_private.h" MODROOT ${BOX64_ROOT})
set(WRAPPEDS_HEAD "${BOX64_ROOT}/src/wrapped/wrappedd3dadapter9_genvate.h")
foreach(A ${WRAPPEDS})
    string(REPLACE ".c" "_private.h" C ${A})
    string(REPLACE "${MODROOT}" "${BOX64_ROOT}" B ${C})
    set(WRAPPEDS_HEAD ${WRAPPEDS_HEAD} ${B})
    set_source_files_properties(${A} PROPERTIES OBJECT_DEPENDS ${B})
endforeach()


set(WRAPPER "${BOX64_ROOT}/src/wrapped/generated/wrapper.c" "${BOX64_ROOT}/src/wrapped/generated/wrapper.h")

if(NOT CI)
    add_custom_command(
        OUTPUT "${BOX64_ROOT}/src/wrapped/generated/functions_list.txt"
        COMMAND "${PYTHON_EXECUTABLE}" "${BOX64_ROOT}/rebuild_wrappers.py"
        "${BOX64_ROOT}"
        "PANDORA" "HAVE_LD80BITS" "NOALIGN" "HAVE_TRACE" "ANDROID" "TERMUX" "STATICBUILD" "LA64" "--"
        ${WRAPPEDS_HEAD}
        MAIN_DEPENDENCY "${BOX64_ROOT}/rebuild_wrappers.py"
        DEPENDS ${WRAPPEDS} ${WRAPPEDS_HEAD}
        BYPRODUCTS ${WRAPPER}
    )
endif ()

#add_custom_command(
#    OUTPUT "${BOX64_ROOT}/src/dynarec/last_run.txt"
#    COMMAND "${PYTHON_EXECUTABLE}" "${BOX64_ROOT}/rebuild_printer.py" "${BOX64_ROOT}"
#    MAIN_DEPENDENCY "${BOX64_ROOT}/rebuild_printer.py"
#    DEPENDS "${BOX64_ROOT}/src/dynarec/arm64_instructions.txt"
#    BYPRODUCTS "${BOX64_ROOT}/src/dynarec/arm64_printer.c"
#)

add_custom_target(WRAPPERS DEPENDS "${BOX64_ROOT}/src/wrapped/generated/functions_list.txt")
#add_custom_target(PRINTER DEPENDS "${BOX64_ROOT}/src/dynarec/last_run.txt")

if(BOX32)
    if(STATICBUILD)
    set(WRAPPEDS32
        "${BOX64_ROOT}/src/wrapped32/wrappedldlinux.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibc.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibdl.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibm.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibpthread.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibrt.c"
    )
    else()
    set(WRAPPEDS32
        "${BOX64_ROOT}/src/wrapped32/wrappedldlinux.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibc.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibdl.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibm.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibpthread.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibrt.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibresolv.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedcrashhandler.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibgl.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibglu.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibglxnvidia.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedsdl1.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedsdl2.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedsdl2image.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibasound.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedopenal.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedfreetype.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedfontconfig.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibx11.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibxdamage.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibxext.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibxfixes.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibxrender.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibxi.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibxcomposite.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibxcursor.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibxrandr.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibxss.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibxxf86vm.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedxinerama.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedexpat.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibx11xcb.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibxcb.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibxcbres.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedudev0.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedudev1.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibuuid.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedgnutls.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedtcmallocminimal.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedcairo.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibdrm.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibvdpau.c"
        "${BOX64_ROOT}/src/wrapped32/wrappednsl.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibcups.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedgbm.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibegl.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedlibglx.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedselinux.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedblkid.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedmount.c"
        "${BOX64_ROOT}/src/wrapped32/wrappeddbus.c"
        "${BOX64_ROOT}/src/wrapped32/wrappedcurl.c"
    )
    endif()
    string(REPLACE ".c" "_private.h" MODROOT ${BOX64_ROOT})
    #set(WRAPPEDS32_HEAD "${BOX64_ROOT}/src/wrapped/wrappedd3dadapter9_genvate.h")
    foreach(A ${WRAPPEDS32})
        string(REPLACE ".c" "_private.h" C ${A})
        string(REPLACE "${MODROOT}" "${BOX64_ROOT}" B ${C})
        set(WRAPPEDS32_HEAD ${WRAPPEDS32_HEAD} ${B})
        set_source_files_properties(${A} PROPERTIES OBJECT_DEPENDS ${B})
    endforeach()

    set(WRAPPER32 "${BOX64_ROOT}/src/wrapped32/generated/wrapper32.c" "${BOX64_ROOT}/src/wrapped32/generated/wrapper32.h")

    if(NOT CI)
        add_custom_command(
            OUTPUT "${BOX64_ROOT}/src/wrapped32/generated/functions_list.txt"
            COMMAND "${PYTHON_EXECUTABLE}" "${BOX64_ROOT}/rebuild_wrappers_32.py"
            "${BOX64_ROOT}"
            "PANDORA" "HAVE_LD80BITS" "NOALIGN" "HAVE_TRACE" "ANDROID" "TERMUX" "WINLATOR_GLIBC" "STATICBUILD" "--"
            ${WRAPPEDS32_HEAD}
            MAIN_DEPENDENCY "${BOX64_ROOT}/rebuild_wrappers_32.py"
            DEPENDS ${WRAPPEDS32} ${WRAPPEDS32_HEAD}
            BYPRODUCTS ${WRAPPER32}
        )
    endif()

    add_custom_target(WRAPPERS32 DEPENDS "${BOX64_ROOT}/src/wrapped32/generated/functions_list.txt")
else()
    set(WRAPPEDS32)
endif()


if(DYNAREC)
    set(DYNAREC_SRC
        "${BOX64_ROOT}/src/dynarec/dynablock.c"
        "${BOX64_ROOT}/src/dynarec/dynarec_native.c"
        "${BOX64_ROOT}/src/dynarec/dynarec_native_functions.c"
        "${BOX64_ROOT}/src/dynarec/dynacache_reloc.c"
        "${BOX64_ROOT}/src/emu/x64test.c"
    )
endif()

if(ARM_DYNAREC)
    set(DYNAREC_SRC
        ${DYNAREC_SRC}

        "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_functions.c"
        "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_arch.c"
        "${BOX64_ROOT}/src/dynarec/arm64/arm64_immenc.c"
        "${BOX64_ROOT}/src/dynarec/arm64/arm64_printer.c"
        "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_jmpnext.c"
        "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_consts.c"
        "${BOX64_ROOT}/src/dynarec/arm64/updateflags_arm64.c"
    )
    set(DYNAREC_ASM
        ${DYNAREC_ASM}
        "${BOX64_ROOT}/src/dynarec/arm64/arm64_prolog.S"
        "${BOX64_ROOT}/src/dynarec/arm64/arm64_epilog.S"
        "${BOX64_ROOT}/src/dynarec/arm64/arm64_next.S"
        "${BOX64_ROOT}/src/dynarec/arm64/arm64_lock.S"
    )

    set(DYNAREC_PASS
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_helper.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_emit_tests.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_emit_math.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_emit_logic.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_emit_shift.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_00.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_0f.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_66.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_d8.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_d9.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_da.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_db.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_dc.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_dd.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_de.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_df.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_f0.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_660f.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_66f20f.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_66f30f.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_66f0.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_f20f.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_f30f.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_avx.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_avx_0f.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_avx_0f38.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_avx_66_0f.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_avx_f2_0f.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_avx_f3_0f.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_avx_66_0f38.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_avx_66_0f3a.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_avx_f2_0f38.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_avx_f2_0f3a.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_avx_f3_0f38.c"

    "${BOX64_ROOT}/src/dynarec/arm64/updateflags_arm64_pass.c"
    )
endif()

if(RV64_DYNAREC)
    set(DYNAREC_SRC
        ${DYNAREC_SRC}

        "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_functions.c"
        "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_arch.c"
        "${BOX64_ROOT}/src/dynarec/rv64/rv64_printer.c"
        "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_jmpnext.c"
        "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_consts.c"
        )
    set(DYNAREC_ASM
        ${DYNAREC_ASM}
        "${BOX64_ROOT}/src/dynarec/rv64/rv64_prolog.S"
        "${BOX64_ROOT}/src/dynarec/rv64/rv64_epilog.S"
        "${BOX64_ROOT}/src/dynarec/rv64/rv64_next.S"
        "${BOX64_ROOT}/src/dynarec/rv64/rv64_lock.S"
    )

    set(DYNAREC_PASS
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_helper.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_emit_tests.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_emit_math.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_emit_logic.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_emit_shift.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_00.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_00_0.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_00_1.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_00_2.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_00_3.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_0f.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_0f_vector.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_66.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_d8.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_d9.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_da.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_db.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_dc.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_dd.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_de.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_df.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_f0.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_660f.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_660f38.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_660f_vector.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_66f20f.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_66f30f.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_66f0.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_f20f.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_f20f_vector.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_f30f.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_f30f_vector.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_avx.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_avx_0f.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_avx_66_0f.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_avx_66_0f38.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_avx_66_0f3a.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_avx_f2_0f.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_avx_f3_0f.c"
    )
endif()

if(LARCH64_DYNAREC)
    set(DYNAREC_SRC
        ${DYNAREC_SRC}

        "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_functions.c"
        "${BOX64_ROOT}/src/dynarec/la64/la64_printer.c"
        "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_jmpnext.c"
        "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_consts.c"
    )
    set(DYNAREC_ASM
        ${DYNAREC_ASM}
        "${BOX64_ROOT}/src/dynarec/la64/la64_prolog.S"
        "${BOX64_ROOT}/src/dynarec/la64/la64_epilog.S"
        "${BOX64_ROOT}/src/dynarec/la64/la64_next.S"
        "${BOX64_ROOT}/src/dynarec/la64/la64_lock.S"
    )

    set(DYNAREC_PASS
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_helper.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_emit_tests.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_emit_math.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_emit_shift.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_emit_logic.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_00.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_0f.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_66.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_660f.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_66f0.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_66f20f.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_66f30f.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_f0.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_f20f.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_f30f.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_avx.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_avx_0f.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_avx_0f38.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_avx_66_0f.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_avx_66_0f38.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_avx_66_0f3a.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_avx_f2_0f.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_avx_f2_0f38.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_avx_f2_0f3a.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_avx_f3_0f.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_avx_f3_0f38.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_d8.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_d9.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_da.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_db.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_dc.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_dd.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_de.c"
    "${BOX64_ROOT}/src/dynarec/la64/dynarec_la64_df.c"
    )
endif()


if(GDBJIT)
set(GDBJITREADER "${BOX64_ROOT}/gdbjit/reader.c")
endif()

if(DYNAREC)
    set(DYNAREC_PASS
        "${BOX64_ROOT}/src/wrapped/generated/wrapper.h"
        ${DYNAREC_PASS}
        "${BOX64_ROOT}/src/dynarec/dynarec_native_pass.c"
    )
    if(BOX32)
        list(APPEND DYNAREC_PASS
            "${BOX64_ROOT}/src/wrapped32/generated/wrapper32.h"
        )
    endif()

    set_source_files_properties(${DYNAREC_PASS} PROPERTIES COMPILE_OPTIONS "${CFLAGS}")
    set_source_files_properties(${DYNAREC_ASM} PROPERTIES COMPILE_OPTIONS "${ASMFLAGS}")
    set_source_files_properties(${INTERPRETER} PROPERTIES COMPILE_OPTIONS "${CFLAGS}")

    add_library(dynarec_native OBJECT ${DYNAREC_SRC} ${DYNAREC_ASM})

    add_library(native_pass0 OBJECT ${DYNAREC_PASS})
    set_target_properties(native_pass0 PROPERTIES COMPILE_DEFINITIONS "STEP=0")
    add_library(native_pass1 OBJECT ${DYNAREC_PASS})
    set_target_properties(native_pass1 PROPERTIES COMPILE_DEFINITIONS "STEP=1")
    add_library(native_pass2 OBJECT ${DYNAREC_PASS})
    set_target_properties(native_pass2 PROPERTIES COMPILE_DEFINITIONS "STEP=2")
    add_library(native_pass3 OBJECT ${DYNAREC_PASS})
    set_target_properties(native_pass3 PROPERTIES COMPILE_DEFINITIONS "STEP=3")
    add_library(test_interpreter OBJECT ${INTERPRETER})
    set_target_properties(test_interpreter PROPERTIES COMPILE_DEFINITIONS "TEST_INTERPRETER")
    add_dependencies(native_pass0 WRAPPERS)
    add_dependencies(native_pass1 WRAPPERS)
    add_dependencies(native_pass2 WRAPPERS)
    add_dependencies(native_pass3 WRAPPERS)
    if(BOX32)
        add_dependencies(native_pass0 WRAPPERS32)
        add_dependencies(native_pass1 WRAPPERS32)
        add_dependencies(native_pass2 WRAPPERS32)
        add_dependencies(native_pass3 WRAPPERS32)
    endif()

    add_library(dynarec STATIC
        $<TARGET_OBJECTS:dynarec_native>
        $<TARGET_OBJECTS:native_pass0>
        $<TARGET_OBJECTS:native_pass1>
        $<TARGET_OBJECTS:native_pass2>
        $<TARGET_OBJECTS:native_pass3>
        $<TARGET_OBJECTS:test_interpreter>
    )

    if(GDBJIT)
        add_library(box64gdbjitreader SHARED ${GDBJITREADER})
    endif()
endif()

set_source_files_properties(${OS_LINUX_SRC} ${ELFLOADER_SRC} ${INTERPRETER} ${WRAPPEDS} ${WRAPPEDS32} PROPERTIES COMPILE_OPTIONS "${CFLAGS}")

# creates git_head.h
if(DYNAREC)
    add_custom_command(
        OUTPUT "${BOX64_ROOT}/src/git_head.h"
        COMMAND sh -c "echo \\\#define GITREV \\\"$(git rev-parse --short HEAD)\\\">\"${BOX64_ROOT}/src/git_head.h\""
        DEPENDS ${DYNAREC_SRC} ${DYNAREC_ASM} ${DYNAREC_PASS} ${ELFLOADER_SRC} ${INTERPRETER} ${WRAPPEDS} ${WRAPPEDS32} ${WRAPPEDS_HEAD} ${WRAPPEDS32_HEAD}
        VERBATIM)
else()
    add_custom_command(
        OUTPUT "${BOX64_ROOT}/src/git_head.h"
        COMMAND sh -c "echo \\\#define GITREV \\\"$(git rev-parse --short HEAD)\\\">\"${BOX64_ROOT}/src/git_head.h\""
        DEPENDS ${ELFLOADER_SRC} ${INTERPRETER} ${WRAPPEDS} ${WRAPPEDS32} ${WRAPPEDS_HEAD} ${WRAPPEDS32_HEAD}
        VERBATIM)
endif()

add_custom_target(generate_git_head_target
  DEPENDS "${BOX64_ROOT}/src/git_head.h"
)
add_custom_target(functions_list DEPENDS "${BOX64_ROOT}/src/wrapped/generated/functions_list.txt" "${BOX64_ROOT}/src/wrapped32/generated/functions_list.txt")
add_dependencies(generate_git_head_target functions_list)

add_library(interpreter OBJECT ${INTERPRETER})

add_library(mainobj OBJECT ${OS_LINUX_SRC} ${ELFLOADER_SRC} ${WRAPPEDS} ${WRAPPEDS32})
add_dependencies(mainobj generate_git_head_target)

add_executable(${BOX64})
target_link_libraries(${BOX64} mainobj)
set_target_properties(${BOX64} PROPERTIES ENABLE_EXPORTS ON)
add_dependencies(${BOX64} WRAPPERS)
if(BOX32)
    add_dependencies(${BOX64} WRAPPERS32)
endif()
#add_dependencies(${BOX64} PRINTER)
#target_link_libraries(${BOX64} c m dl rt pthread resolv)
if(STATICBUILD)
    #set_target_properties(${BOX64} PROPERTIES LINK_FLAGS "-fuse-ld=gold -static -Wl,--no-relax -Wl,--allow-multiple-definition -ldl -lrt -lpthread -lresolv -lc -lutil -lm -Wl,-defsym,_DYNAMIC=0 -pthread")
    target_link_libraries(${BOX64} c m dl pthread resolv util c)
else()
    if(ANDROID)
        if(TERMUX)
        target_link_libraries(${BOX64} c m dl android-sysv-semaphore)
        else()
        target_link_libraries(${BOX64} c m dl)
        endif()
    else()
	    set_property(TARGET ${BOX64} APPEND_STRING PROPERTY LINK_FLAGS "-Wl,--no-as-needed -lc -lm -ldl -lrt -lpthread -lresolv -lutil -Wl,--as-needed -pthread ")
		target_link_libraries(${BOX64} c m dl pthread resolv util c)
    endif()
endif()
if(DYNAREC)
    target_link_libraries(${BOX64} dynarec)
endif()
target_link_libraries(${BOX64} interpreter)

if(${CMAKE_VERSION} VERSION_LESS "3.13")
    if(NOT NOLOADADDR)
        if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR WITH_MOLD)
            set_property(TARGET ${BOX64} APPEND_STRING PROPERTY LINK_FLAGS "-no-pie -Wl,--image-base=${BOX64_ELF_ADDRESS} ")
        else()
            set_property(TARGET ${BOX64} APPEND_STRING PROPERTY LINK_FLAGS "-Wl,-Ttext-segment,${BOX64_ELF_ADDRESS} ")
        endif()
    endif()
else()
    # If symbols are missing, try this: target_link_options(${BOX64} PUBLIC -rdynamic)
    if(NOT NOLOADADDR)
        if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR WITH_MOLD)
            if(WITH_MOLD) # --image-base requires -no-pie in mold!
                set_property(TARGET ${BOX64} APPEND_STRING PROPERTY LINK_FLAGS "-no-pie -lm ")
            endif()
            target_link_options(${BOX64} PUBLIC LINKER:--image-base=${BOX64_ELF_ADDRESS})
        else()
            target_link_options(${BOX64} PUBLIC LINKER:-Ttext-segment,${BOX64_ELF_ADDRESS})
        endif()
    endif()
endif()

string(COMPARE EQUAL "${CMAKE_SYSTEM_PROCESSOR}" "i686"  _x86)
string(COMPARE EQUAL "${CMAKE_SYSTEM_PROCESSOR}" "x86_64"  _x86_64)
string(COMPARE EQUAL "${CMAKE_SYSTEM_PROCESSOR}" "aarch64"  _aarch64)
string(COMPARE EQUAL "${CMAKE_SYSTEM_PROCESSOR}" "riscv64"  _riscv64)

if(_x86_64 OR _aarch64)
  add_definitions(-DCONFIG_64BIT)
endif()

if(NOT _x86 AND NOT _x86_64)
  if(NOT TERMUX)
   install(TARGETS ${BOX64} RUNTIME DESTINATION bin)
   if(NOT NO_LIB_INSTALL)
    install(PROGRAMS ${CMAKE_SOURCE_DIR}/tests/box64-bash DESTINATION bin)
   endif()
  else()
   install(TARGETS ${BOX64} RUNTIME DESTINATION ${TERMUX_PATH}/usr/bin)
   if(NOT NO_LIB_INSTALL)
    install(PROGRAMS ${CMAKE_SOURCE_DIR}/tests/box64-bash DESTINATION ${TERMUX_PATH}/usr/bin)
   endif()
  endif()
  if (GDBJIT)
    install(TARGETS box64gdbjitreader RUNTIME DESTINATION lib)
  endif()
  if(NOT NO_CONF_INSTALL)
    configure_file(system/box64.conf.cmake system/box64.conf)
	  if(NOT TERMUX)
        install(FILES ${CMAKE_BINARY_DIR}/system/box64.conf DESTINATION /etc/binfmt.d/)
        install(FILES ${CMAKE_SOURCE_DIR}/system/box64.box64rc DESTINATION /etc/)
        if(BOX32_BINFMT)
            configure_file(system/box32.conf.cmake system/box32.conf)
            install(FILES ${CMAKE_BINARY_DIR}/system/box32.conf DESTINATION /etc/binfmt.d/)
        endif()
      else()
        install(FILES ${CMAKE_BINARY_DIR}/system/box64.conf DESTINATION ${TERMUX_PATH}/usr/etc/binfmt.d/)
        install(FILES ${CMAKE_SOURCE_DIR}/system/box64.box64rc DESTINATION ${TERMUX_PATH}/usr/etc/)
        if(BOX32_BINFMT)
            configure_file(system/box32.conf.cmake system/box32.conf)
            install(FILES ${CMAKE_BINARY_DIR}/system/box32.conf DESTINATION ${TERMUX_PATH}/usr/etc/binfmt.d/)
        endif()
	  endif()

  endif()
  set(INSTALL_PATH "/usr/lib/box64-x86_64-linux-gnu/")
  if(NOT NO_LIB_INSTALL)
	if(NOT TERMUX AND NOT ANDROID)
        install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libstdc++.so.5 DESTINATION ${INSTALL_PATH})
        install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libstdc++.so.6 DESTINATION ${INSTALL_PATH})
        install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libgcc_s.so.1 DESTINATION ${INSTALL_PATH})
        install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libpng12.so.0 DESTINATION ${INSTALL_PATH})
        install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libcrypto.so.1.1 DESTINATION ${INSTALL_PATH})
        install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libcrypto.so.1.0.0 DESTINATION ${INSTALL_PATH})
        install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libssl.so.1.1 DESTINATION ${INSTALL_PATH})
        install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libssl.so.1.0.0 DESTINATION ${INSTALL_PATH})
        install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libunwind.so.8 DESTINATION ${INSTALL_PATH})
        install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libmbedx509.so.1 DESTINATION ${INSTALL_PATH})
        install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libmbedx509.so.0 DESTINATION ${INSTALL_PATH})
        install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libmbedtls.so.14 DESTINATION ${INSTALL_PATH})
        install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libmbedtls.so.12 DESTINATION ${INSTALL_PATH})
        install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libmbedcrypto.so.7 DESTINATION ${INSTALL_PATH})
        install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libmbedcrypto.so.3 DESTINATION ${INSTALL_PATH})
	elseif(TERMUX)
		install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libstdc++.so.5 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
		install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libstdc++.so.6 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
		install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libgcc_s.so.1 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
		install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libpng12.so.0 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
		install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libcrypto.so.1.1 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
        install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libcrypto.so.1.0.0 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
		install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libssl.so.1.1 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
        install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libssl.so.1.0.0 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
		install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libunwind.so.8 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
		install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libmbedx509.so.1 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
        install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libmbedx509.so.0 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
        install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libmbedtls.so.14 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
        install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libmbedtls.so.12 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
        install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libmbedcrypto.so.7 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
        install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libmbedcrypto.so.3 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
    elseif(ANDROID)
    install(FILES ${CMAKE_SOURCE_DIR}/x64android/libc++_shared.so DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
	endif()
   endif()
   if(BOX32)
    set(INSTALL_PATH "/usr/lib/box64-i386-linux-gnu/")
    if(NOT NO_LIB_INSTALL)
    	if(NOT TERMUX AND NOT ANDROID)
    		install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libstdc++.so.5 DESTINATION ${INSTALL_PATH})
    		install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libstdc++.so.6 DESTINATION ${INSTALL_PATH})
    		install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libgcc_s.so.1 DESTINATION ${INSTALL_PATH})
    		install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libpng12.so.0 DESTINATION ${INSTALL_PATH})
            install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libunwind.so.8 DESTINATION ${INSTALL_PATH})
            install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libcrypto.so.1.0.0 DESTINATION ${INSTALL_PATH})
            install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libssl.so.1.0.0 DESTINATION ${INSTALL_PATH})
            install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libz.so.1 DESTINATION ${INSTALL_PATH})
        elseif(TERMUX)
            install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libstdc++.so.5 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
            install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libstdc++.so.6 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
            install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libgcc_s.so.1 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
            install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libpng12.so.0 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
            install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libunwind.so.8 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
            install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libcrypto.so.1.0.0 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
            install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libssl.so.1.0.0 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
            install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libz.so.1 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
        elseif(ANDROID)
            install(FILES ${CMAKE_SOURCE_DIR}/x86android/libc++_shared.so DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
       endif()
    endif()
  endif()
endif()

if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

string(REPLACE ";" "," DYNAREC_ASM_STR "${DYNAREC_ASM}")
string(REPLACE ";" "," DYNAREC_PASS_STR "${DYNAREC_PASS}")
string(REPLACE ";" "," INTERPRETER_STR "${INTERPRETER}")

if(WOW64 AND ARM_DYNAREC)
    include(ExternalProject)
    ExternalProject_Add(
        wowbox64
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/wine/wow64"
        CMAKE_ARGS 
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_SOURCE_DIR}/wine/toolchain_mingw.cmake
            -DDYNAREC_PASS_STR=${DYNAREC_PASS_STR}
            -DDYNAREC_ASM_STR=${DYNAREC_ASM_STR}
            -DINTERPRETER_STR=${INTERPRETER_STR}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        INSTALL_COMMAND ""
        TEST_COMMAND ""
        BUILD_ALWAYS ON
    )
    add_dependencies(wowbox64 generate_git_head_target)
endif()

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "ptitSeb")
set(CPACK_PACKAGE_CONTACT "ptitSeb@box86.org")
set(CPACK_PACKAGE_DESCRIPTION "Box64 - Linux Userspace x86_64 Emulator with a twist")
if(NOT TERMUX)
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6")
else()
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "libandroid-sysv-semaphore")
endif()
set(CPACK_PACKAGE_HOMEPAGE_URL, "https://box86.org")
file(STRINGS "${BOX64_ROOT}/src/box64version.h" TEMP_STRING REGEX "BOX64_MAJOR\\s*")
string(REGEX REPLACE "BOX64_MAJOR" "" TEMP_STRING ${TEMP_STRING})
string(REGEX MATCH "[0-9]" BOX64_MAJOR ${TEMP_STRING})
file(STRINGS "${BOX64_ROOT}/src/box64version.h" TEMP_STRING REGEX "BOX64_MINOR\\s*")
string(REGEX REPLACE "BOX64_MINOR" "" TEMP_STRING ${TEMP_STRING})
string(REGEX MATCH "[0-9]" BOX64_MINOR ${TEMP_STRING})
file(STRINGS "${BOX64_ROOT}/src/box64version.h" TEMP_STRING REGEX "BOX64_REVISION\\s*")
string(REGEX REPLACE "BOX64_REVISION" "" TEMP_STRING ${TEMP_STRING})
string(REGEX MATCH "[0-9]" BOX64_REVISION ${TEMP_STRING})
set(CPACK_PACKAGE_VERSION_MAJOR ${BOX64_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${BOX64_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${BOX64_REVISION})
if(NOT TERMUX)
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/postinst")
endif()
if(_aarch64)
	if(TERMUX)
		set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "aarch64")
	else()
    		set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
	endif()
elseif(_riscv64)
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "riscv64")
elseif(_x86_64)
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "x86_64")
else()
    #probably wrong...
    execute_process(COMMAND "dpkg --print-architecture" OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE)
endif()
if(TERMUX)
set(CPACK_PACKAGING_INSTALL_PREFIX "${TERMUX_PATH}/usr")
endif()
if(NOT TERMUX)
set(CPACK_DEBIAN_FILE_NAME "${BOX64}-${BOX64_MAJOR}.${BOX64_MINOR}.${BOX64_REVISION}_${CMAKE_SYSTEM_NAME}-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}.deb")
else()
set(CPACK_DEBIAN_FILE_NAME "${BOX64}-${BOX64_MAJOR}.${BOX64_MINOR}.${BOX64_REVISION}_Termux-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}.deb")
endif()
INCLUDE(CPack)

if(NOT ANDROID)
add_test(bootSyscall ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test01 -D TEST_OUTPUT=tmpfile01.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref01.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(bootSyscallC ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test02 -D TEST_OUTPUT=tmpfile02.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref02.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(printf ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test03 -D TEST_OUTPUT=tmpfile03.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref03.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(args ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test04 -D TEST_ARGS2=yeah -D TEST_OUTPUT=tmpfile04.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref04.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(maths1 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test05 -D TEST_ARGS2=7 -D TEST_OUTPUT=tmpfile05.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref05.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(threadsStart ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test06 -D TEST_OUTPUT=tmpfile06.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref06.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(trig ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test07 -D TEST_OUTPUT=tmpfile07.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref07.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(pi ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test08 -D TEST_OUTPUT=tmpfile08.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref08.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(fork ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test09 -D TEST_OUTPUT=tmpfile09.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref09.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(cppThreads_nocosim ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test10 -D TEST_OUTPUT=tmpfile10.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref10.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

set_tests_properties(cppThreads_nocosim PROPERTIES ENVIRONMENT "BOX64_LD_LIBRARY_PATH=${CMAKE_SOURCE_DIR}/x64lib")

add_test(tlsData ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test11 -D TEST_OUTPUT=tmpfile11.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref11.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(fpu ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test12 -D TEST_OUTPUT=tmpfile12.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref12.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(contexts ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test13 -D TEST_OUTPUT=tmpfile13.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref13.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

#add_test(conditionalThreads ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
#    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test14 -D TEST_OUTPUT=tmpfile14.txt
#    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref14.txt
#    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(linkingIndirectNoversion ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test15 -D TEST_OUTPUT=tmpfile15.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref15.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(sse_asm ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test16 -D TEST_OUTPUT=tmpfile16.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref16.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

set_tests_properties(sse_asm PROPERTIES ENVIRONMENT "BOX64_DYNAREC_FASTROUND=0")

add_test(sse_intrinsics ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test17 -D TEST_OUTPUT=tmpfile17.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref17.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

set_tests_properties(sse_intrinsics PROPERTIES ENVIRONMENT "BOX64_DYNAREC_FASTNAN=0;BOX64_DYNAREC_FASTROUND=0")

add_test(aes ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test18 -D TEST_OUTPUT=tmpfile18.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref18.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(backtrace ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test19 -D TEST_OUTPUT=tmpfile19.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref19.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(irelative_reloc ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test20 -D TEST_OUTPUT=tmpfile20.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref20.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(longjumpInSignals_nocosim ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test21 -D TEST_OUTPUT=tmpfile21.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref21.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(x87 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test22 -D TEST_OUTPUT=tmpfile22.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref22.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

set_tests_properties(x87 PROPERTIES ENVIRONMENT "BOX64_DYNAREC_FASTROUND=0")

add_test(pshufb ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test23 -D TEST_OUTPUT=tmpfile23.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref23.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(bswap ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test24 -D TEST_OUTPUT=tmpfile24.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref24.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(x87cache ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test25 -D TEST_OUTPUT=tmpfile25.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref25.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(feround ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test26 -D TEST_OUTPUT=tmpfile26.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref26.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )
set_tests_properties(feround PROPERTIES ENVIRONMENT "BOX64_SYNC_ROUNDING=1")

add_test(sse4_2 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test27 -D TEST_OUTPUT=tmpfile27.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref27.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(shaext ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test28 -D TEST_OUTPUT=tmpfile28.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref28.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(lock ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test29 -D TEST_OUTPUT=tmpfile29.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref29.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(avx_intrinsics ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test30 -D TEST_OUTPUT=tmpfile30.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref30.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

set_tests_properties(avx_intrinsics PROPERTIES ENVIRONMENT "BOX64_DYNAREC_FASTNAN=0;BOX64_DYNAREC_FASTROUND=0;BOX64_AVX=2")

add_test(fpu_rounding ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test31 -D TEST_OUTPUT=tmpfile31.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref31.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

set_tests_properties(fpu_rounding PROPERTIES ENVIRONMENT "BOX64_DYNAREC_FASTROUND=0")

add_test(x87pc ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test32 -D TEST_OUTPUT=tmpfile32.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref32.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

set_tests_properties(x87pc PROPERTIES ENVIRONMENT "BOX64_DYNAREC_X87DOUBLE=2")

add_test(Boundary ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test33 -D TEST_OUTPUT=tmpfile33.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref33.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

else()

add_test(bootSyscall ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test01_android -D TEST_OUTPUT=tmpfile01.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref01.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(bootSyscallC ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test02_android -D TEST_OUTPUT=tmpfile02.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref02.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(printf ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test03_android -D TEST_OUTPUT=tmpfile03.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref03.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(args ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test04_android -D TEST_ARGS2=yeah -D TEST_OUTPUT=tmpfile04.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref04.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(maths1 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test05_android -D TEST_ARGS2=7 -D TEST_OUTPUT=tmpfile05.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref05.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(threadsStart ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test06_android -D TEST_OUTPUT=tmpfile06.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref06.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(trig ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test07_android -D TEST_OUTPUT=tmpfile07.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref07.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(pi ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test08_android -D TEST_OUTPUT=tmpfile08.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref08.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(fork ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test09_android -D TEST_OUTPUT=tmpfile09.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref09.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(fpu ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test12_android -D TEST_OUTPUT=tmpfile12.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref12_android.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(linkingIndirectNoversion ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test15_android -D TEST_OUTPUT=tmpfile15.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref15.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

# Box64 for Android now have problems with test16, then disable it for now

#add_test(sse_asm ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
#    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test16_android -D TEST_OUTPUT=tmpfile16.txt
#    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref16.txt
#    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

# Box64 for Android now have problems with test17, then disable it for now

#set_tests_properties(sse_asm PROPERTIES ENVIRONMENT "BOX64_DYNAREC_FASTROUND=0")

#add_test(sse_intrinsics ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
#    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test17_android -D TEST_OUTPUT=tmpfile17.txt
#    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref17_android.txt
#    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

#set_tests_properties(sse_intrinsics PROPERTIES ENVIRONMENT "BOX64_DYNAREC_FASTNAN=0;BOX64_DYNAREC_FASTROUND=0")

add_test(aes ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test18_android -D TEST_OUTPUT=tmpfile18.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref18.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

# Android don't have backtrace symbols

add_test(irelative_reloc ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test20_android -D TEST_OUTPUT=tmpfile20.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref20.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

# add_test(longjumpInSignals_nocosim ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
#     -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test21_android -D TEST_OUTPUT=tmpfile21.txt
#     -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref21.txt
#     -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(x87 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test22_android -D TEST_OUTPUT=tmpfile22.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref22_android.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

set_tests_properties(x87 PROPERTIES ENVIRONMENT "BOX64_DYNAREC_FASTROUND=0")

add_test(pshufb ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test23_android -D TEST_OUTPUT=tmpfile23.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref23.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(bswap ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test24_android -D TEST_OUTPUT=tmpfile24.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref24.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(x87cache ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test25_android -D TEST_OUTPUT=tmpfile25.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref25.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(feround ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test26_android -D TEST_OUTPUT=tmpfile26.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref26.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )
set_tests_properties(feround PROPERTIES ENVIRONMENT "BOX64_SYNC_ROUNDING=1")

# TODO: need to regen test with latest test27.c sources

#add_test(sse4_2 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
#    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test27_android -D TEST_OUTPUT=tmpfile27.txt
#    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref27.txt
#    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(shaext ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test28_android -D TEST_OUTPUT=tmpfile28.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref28.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(lock ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test29_android -D TEST_OUTPUT=tmpfile29.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref29.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

#add_test(avx_intrinsics ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
#    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test30_android -D TEST_OUTPUT=tmpfile30.txt
#    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref30.txt
#    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )
#
#set_tests_properties(avx_intrinsics PROPERTIES ENVIRONMENT "BOX64_DYNAREC_FASTNAN=0;BOX64_DYNAREC_FASTROUND=0 BOX64_AVX=2")


endif()

if(NOT ANDROID)
file(GLOB extension_tests "${CMAKE_SOURCE_DIR}/tests/extensions/*.c")
foreach(file ${extension_tests})
    get_filename_component(testname "${file}" NAME_WE)
    add_test(NAME "${testname}" COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/extensions/${testname} -D TEST_OUTPUT=tmpfile-${testname}.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/extensions/${testname}.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake)
endforeach()
else()

file(GLOB extension_tests "${CMAKE_SOURCE_DIR}/tests/extensions/*.c")
foreach(file ${extension_tests})
    get_filename_component(testname "${file}" NAME_WE)
    add_test(NAME "${testname}" COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/extensions/${testname}_android -D TEST_OUTPUT=tmpfile-${testname}.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/extensions/${testname}.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake)
endforeach()
endif()

if(BOX32)
    add_test(NAME bootSyscall_32bits COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test01 -D TEST_OUTPUT=tmpfile32_01.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref01.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    add_test(NAME bootSyscallC_32bits COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test02 -D TEST_OUTPUT=tmpfile32_02.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref02.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    add_test(NAME printf_32bits COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test03 -D TEST_OUTPUT=tmpfile32_03.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref03.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    add_test(NAME args_32bits COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test04 -D TEST_ARGS2=yeah -D TEST_OUTPUT=tmpfile32_04.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref04.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    add_test(NAME maths1_32bits COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test05 -D TEST_ARGS2=7 -D TEST_OUTPUT=tmpfile32_05.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref05.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    add_test(NAME threadsStart_32bits COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test06 -D TEST_OUTPUT=tmpfile32_06.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref06.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    add_test(NAME trig_32bits COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test07 -D TEST_OUTPUT=tmpfile32_07.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref07.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    add_test(NAME pi_32bits COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test08 -D TEST_OUTPUT=tmpfile32_08.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref08.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    add_test(NAME fork_32bits COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test09 -D TEST_OUTPUT=tmpfile32_09.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref09.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    add_test(NAME cppThreads_32bits_nocosim COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test10 -D TEST_OUTPUT=tmpfile32_10.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref10.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )
    set_tests_properties(cppThreads_32bits_nocosim PROPERTIES ENVIRONMENT "BOX64_LD_LIBRARY_PATH=${CMAKE_SOURCE_DIR}/x86lib")

    add_test(NAME tlsData_32bits COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test11 -D TEST_OUTPUT=tmpfile32_11.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref11.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    add_test(NAME fpu_32bits COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test12 -D TEST_OUTPUT=tmpfile32_12.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref12.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    add_test(NAME contexts_32bits COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test13 -D TEST_OUTPUT=tmpfile32_13.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref13.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    if(NOT LD80BITS)
        add_test(NAME conditionalThreads_32bits_nocosim COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
            -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test14 -D TEST_OUTPUT=tmpfile32_14.txt
            -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref14.txt
            -P ${CMAKE_SOURCE_DIR}/runTest.cmake )
    endif()

    add_test(NAME linkingIndirectNoversion_32bits COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test15 -D TEST_OUTPUT=tmpfile32_15.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref15.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    add_test(NAME linkingIndirectVersion_32bits COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test16 -D TEST_OUTPUT=tmpfile32_16.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref16.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    add_test(NAME sse_32bits COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test17 -D TEST_OUTPUT=tmpfile32_17.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref17.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    set_tests_properties(sse_32bits PROPERTIES ENVIRONMENT "BOX64_DYNAREC_FASTNAN=0;BOX64_DYNAREC_FASTROUND=0")

    add_test(NAME longjumpInSignals_32bits_nocosim COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test18 -D TEST_OUTPUT=tmpfile32_18.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref18.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    add_test(NAME x87_32bits COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test19 -D TEST_OUTPUT=tmpfile32_19.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref19.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    set_tests_properties(x87_32bits PROPERTIES ENVIRONMENT "BOX64_DYNAREC_FASTROUND=0")

    add_test(NAME idiv_32bits COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test20 -D TEST_OUTPUT=tmpfile32_20.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref20.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    add_test(NAME multiple_dlopen_32bits COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test21 -D TEST_OUTPUT=tmpfile32_21.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref21.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    file(GLOB extension_tests "${CMAKE_SOURCE_DIR}/tests32/extensions/*.c")
    foreach(file ${extension_tests})
        get_filename_component(testname "${file}" NAME_WE)
        add_test(NAME "${testname}_32bits" COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
            -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/extensions/${testname} -D TEST_OUTPUT=tmpfile32_-${testname}.txt
            -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/extensions/${testname}.txt
            -P ${CMAKE_SOURCE_DIR}/runTest.cmake)
    endforeach()

    #add_test(NAME sse_optimized_32bits COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    #    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test17_o2 -D TEST_OUTPUT=tmpfile32_17_o2.txt
    #    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref17_o2.txt
    #    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    #set_tests_properties(sse_optimized_32bits PROPERTIES ENVIRONMENT "BOX64_DYNAREC_FASTNAN=0;BOX64_DYNAREC_FASTROUND=0")

    add_test(bswap_32bits ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
            -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests32/test23 -D TEST_OUTPUT=tmpfile32_23.txt
            -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests32/ref23.txt
            -P ${CMAKE_SOURCE_DIR}/runTest.cmake )
endif()
