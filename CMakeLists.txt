cmake_minimum_required(VERSION 3.4)

cmake_policy(SET CMP0065 NEW)

SET(CMAKE_C_FLAGS_DEBUG  "-O0 -g")
SET(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")

option(RPI3ARM64 "Set to ON if targeting an RaspberryPI3 device with multiarch arm64 and armhf" ${RPI3ARM64})
option(RPI4ARM64 "Set to ON if targeting an RaspberryPI4 device with multiarch arm64 and armhf" ${RPI4ARM64})
option(RK3326 "Set to ON if targeting an Rockchip RK3326 based device" ${RK3326})
option(RK3399 "Set to ON if targeting an Rockchip RK3399 based device" ${RK3399})
option(RK3588 "Set to ON if targeting an Rockchip RK3588(S) based device" ${RK3588})
option(ODROIDN2 "Set to ON if targeting an Odroid-N2 device" ${ODROIDN2})
option(TEGRAX1 "Set to ON if targeting an Tegra X1 based device" ${TEGRAX1})
option(PHYTIUM "Set to ON if targeting an Phytium (D2000 or FT2000/4) based device" ${PHYTIUM})
option(SD845 "Set to ON if targeting a Snapragon 845 based device" ${SD845})
option(M1 "Set to ON if targeting a AppleM1 running on Asahi computer" ${M1})
option(LARCH64 "Set to ON if targeting an Loongarch64 based device" ${LARCH64})
option(RV64 "Set to ON if targeting an RISC-V RV64GC based device" ${RV64})
option(PPC64LE "Set to ON if targeting an PowerPC 64 LE based device" ${PPC64LE})
option(LX2160A "Set to ON if targeting an LX2160A based device" ${LX2160A})
option(USE_CCACHE "Set to ON to use ccache if present in the system" ${USE_CCACHE})
option(HAVE_TRACE "Set to ON to have Trace ability (needs ZydisInfo library)" ${HAVE_TRACE})
option(NOLOADADDR "Set to ON to avoid fixing the load address of Box64" OFF)
option(NOGIT "Set to ON if not building from a git clone repo (like when building from a zip download from github)" ${NOGIT})
option(BAD_SIGNAL "Set to ON to activate the workaround for incoherent si_info on SIGSEGV" ${BAD_SIGNAL})
option(SW64 "Set ON if targeting an SW64 based device" ${SW64})

if(LARCH64)
    set(LD80BITS OFF CACHE BOOL "")
    set(NOALIGN OFF CACHE BOOL "")
    set(ARM_DYNAREC OFF CACHE BOOL "")
    set(RV64_DYNAREC OFF CACHE BOOL "")
endif()
if(RV64)
    set(LD80BITS OFF CACHE BOOL "")
    set(NOALIGN OFF CACHE BOOL "")
    set(ARM_DYNAREC OFF CACHE BOOL "")
    set(RV64_DYNAREC ON CACHE BOOL "")
    set(PAGE16K OFF CACHE BOOL "")
endif()
if(PPC64LE)
    set(LD80BITS OFF CACHE BOOL "")
    set(NOALIGN OFF CACHE BOOL "")
    set(ARM_DYNAREC OFF CACHE BOOL "")
    set(RV64_DYNAREC OFF CACHE BOOL "")
    set(PAGE16K OFF CACHE BOOL "")
endif()
if(RK3399 OR RK3588 OR ODROIDN2 OR RPI3ARM64 OR RPI4ARM64 OR RK3326 OR TEGRAX1 OR PHYTIUM OR SD845 OR LX2160A OR M1)
    set(LD80BITS OFF CACHE BOOL "")
    set(NOALIGN OFF CACHE BOOL "")
    set(ARM_DYNAREC ON CACHE BOOL "")
    set(RV64_DYNAREC OFF CACHE BOOL "")
endif()
if(RK3399 OR RK3588 OR ODROIDN2 OR RPI3ARM64 OR RPI4ARM64 OR RK3326 OR TEGRAX1 OR PHYTIUM OR SD845 OR LX2160A)
    set(PAGE16K OFF CACHE BOOL "")
endif()
if(RK3588)
    set(BAD_SIGNAL ON CACHE BOOL "")
endif()
if(M1 OR LARCH64)
    set(PAGE16K ON CACHE BOOL "")
endif()
if(SW64)
    set(LD80BITS OFF CACHE BOOL "")
    set(NOALIGN OFF CACHE BOOL "")
    set(PAGE8K ON CACHE BOOL "")
endif()

option(LD80BITS "Set to ON if host device have 80bits long double (i.e. i386)" ${LD80BITS})
option(NOALIGN "Set to ON if host device doesn't need re-align (i.e. i386)" ${NOALIGN})
option(ARM_DYNAREC "Set to ON to use ARM Dynamic Recompilation" ${ARM_DYNAREC})
option(RV64_DYNAREC "Set to ON to use RISC-V Dynamic Recompilation" ${RV64_DYNAREC})
option(PAGE8K "Set to ON if host device have PageSize of 8K (instead of 4K)" ${PAGE8K})
option(PAGE16K "Set to ON if host device have PageSize of 16K (instead of 4K)" ${PAGE16K})
option(PAGE64K "Set to ON if host device have PageSize of 64K (instead of 4K)" ${PAGE64K})
option(STATICBUILD "Set to ON to have a static build (Warning, not working)" ${STATICBUILD})

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

set(default_build_type "RelWithDebInfo")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(RPI3ARM64)
    add_definitions(-DRPI)
    add_definitions(-DRPI3ARM64)
    add_definitions(-pipe -march=armv8-a+crc -mtune=cortex-a53)
    set(CMAKE_ASM_FLAGS  "-pipe -march=armv8-a+crc -mtune=cortex-a53")
elseif(RPI4ARM64)
    add_definitions(-DRPI)
    add_definitions(-DRPI4ARM64)
    add_definitions(-pipe -march=armv8-a+crc -mtune=cortex-a72)
    set(CMAKE_ASM_FLAGS  "-pipe -march=armv8-a+crc -mtune=cortex-a72")
elseif(RK3326)
    add_definitions(-DRK3326)
    add_definitions(-pipe -march=armv8-a+crc+simd+crypto -mcpu=cortex-a35+crypto)
    set(CMAKE_ASM_FLAGS  "-pipe -march=armv8-a+crc+simd+crypto -mcpu=cortex-a35+crypto")
elseif(RK3399)
    add_definitions(-DRK3399)
    add_definitions(-pipe -march=armv8-a+crc+simd+crypto -mcpu=cortex-a72.cortex-a53+crypto)
    set(CMAKE_ASM_FLAGS  "-pipe -march=armv8-a+crc+simd+crypto -mcpu=cortex-a72.cortex-a53+crypto")
elseif(RK3588)
    add_definitions(-DRK3588)
    add_definitions(-pipe -mcpu=cortex-a76.cortex-a55+crypto)
    add_definitions(-DBAD_SIGNAL)
    set(CMAKE_ASM_FLAGS  "-pipe -mcpu=cortex-a76.cortex-a55+crypto")
elseif(ODROIDN2)
    add_definitions(-DODROIDN2)
    add_definitions(-march=armv8-a+crc+simd+crypto -mcpu=cortex-a73.cortex-a53+crypto)
    set(CMAKE_ASM_FLAGS  "-pipe -march=armv8-a+crc+simd+crypto -mcpu=cortex-a73.cortex-a53+crypto")
elseif(TEGRAX1)
    add_definitions(-DTEGRAX1)
    add_definitions(-pipe -march=armv8-a+crc+simd+crypto -mcpu=cortex-a57+crypto)
    set(CMAKE_ASM_FLAGS  "-pipe -march=armv8-a+crc+simd+crypto -mcpu=cortex-a57+crypto")
elseif(PHYTIUM)
    add_definitions(-DPHYSIUM)
    add_definitions(-pipe -march=armv8.1-a+crc+simd+crypto)
    set(CMAKE_ASM_FLAGS  "-pipe -march=armv8.1-a+crc+simd+crypto+aes")
elseif(SD845)
    add_definitions(-DSD845)
    add_definitions(-pipe -march=armv8.2-a+simd+crypto -mtune=cortex-a75.cortex-a55)
    set(CMAKE_ASM_FLAGS  "-pipe -march=armv8.2-a+simd+crypto -mtune=cortex-a75.cortex-a55")
elseif(M1)
    add_definitions(-DM1)
    add_definitions(-pipe -march=armv8.5-a+simd+crypto)
    set(CMAKE_ASM_FLAGS  "-pipe -march=armv8.5-a+simd+crypto")
elseif(LARCH64)
    add_definitions(-DLA464)
    add_definitions(-pipe -march=loongarch64)
    set(CMAKE_ASM_FLAGS  "-pipe -march=loongarch64")
elseif(RV64)
    add_definitions(-DRV64)
    add_definitions(-pipe -march=rv64gc)
    set(CMAKE_ASM_FLAGS  "-pipe -march=rv64gc")
elseif(PPC64LE)
    add_definitions(-DPPC64LE)
elseif(LX2160A)
    add_definitions(-DLX2160A)
    add_definitions(-pipe -march=armv8-a+crypto+crc -mcpu=cortex-a72+crypto)
    set(CMAKE_ASM_FLAGS  "-pipe -march=armv8-a+crypto+crc -mcpu=cortex-a72+crypto")
elseif(SW64)
    add_definitions(-DSW64)
elseif(ARM_DYNAREC)
    set(CMAKE_ASM_FLAGS  "-pipe -march=armv8-a+crc+simd+crypto")
endif()

if(BAD_SIGNAL)
    add_definitions(-DBAD_SIGNAL)
endif()

if(PAGE8K)
    add_definitions(-DPAGE8K)
endif()

if(PAGE16K)
    add_definitions(-DPAGE16K)
endif()

if(PAGE64K)
    add_definitions(-DPAGE64K)
endif()

if(NOGIT)
    add_definitions(-DNOGIT)
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
    "${BOX64_ROOT}/src/mallochook.c"
    "${BOX64_ROOT}/src/steam.c"
    "${BOX64_ROOT}/src/dynarec/dynarec.c"
    "${BOX64_ROOT}/src/elfs/elfdwarf_private.c"
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
    "${BOX64_ROOT}/src/emu/x64run66f0.c"
    "${BOX64_ROOT}/src/emu/x64run67.c"
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
    "${BOX64_ROOT}/src/emu/x64run_private.c"
    "${BOX64_ROOT}/src/emu/x64syscall.c"
    "${BOX64_ROOT}/src/emu/x64tls.c"
    "${BOX64_ROOT}/src/emu/x64trace.c"
    "${BOX64_ROOT}/src/librarian/librarian.c"
    "${BOX64_ROOT}/src/librarian/library.c"
    "${BOX64_ROOT}/src/librarian/dictionnary.c"
    "${BOX64_ROOT}/src/librarian/globalsymbols.c"
    "${BOX64_ROOT}/src/librarian/symbols.c"
    "${BOX64_ROOT}/src/libtools/auxval.c"
    "${BOX64_ROOT}/src/libtools/obstack.c"
    "${BOX64_ROOT}/src/libtools/myalign.c"
    "${BOX64_ROOT}/src/libtools/sdl1rwops.c"
    "${BOX64_ROOT}/src/libtools/sdl2rwops.c"
    "${BOX64_ROOT}/src/libtools/signals.c"
    "${BOX64_ROOT}/src/libtools/threads.c"
    "${BOX64_ROOT}/src/tools/bitutils.c"
    "${BOX64_ROOT}/src/tools/box64stack.c"
    "${BOX64_ROOT}/src/tools/bridge.c"
    "${BOX64_ROOT}/src/tools/callback.c"
    "${BOX64_ROOT}/src/tools/my_cpuid.c"
    "${BOX64_ROOT}/src/tools/gtkclass.c"
    "${BOX64_ROOT}/src/tools/pathcoll.c"
    "${BOX64_ROOT}/src/tools/fileutils.c"
    "${BOX64_ROOT}/src/tools/rcfile.c"
    "${BOX64_ROOT}/src/tools/wine_tools.c"
    "${BOX64_ROOT}/src/wrapped/generated/wrapper.c"
)

set(WRAPPEDS
    "${BOX64_ROOT}/src/wrapped/wrappedalure.c"
    "${BOX64_ROOT}/src/wrapped/wrappedalut.c"
    "${BOX64_ROOT}/src/wrapped/wrappedatk.c"
    "${BOX64_ROOT}/src/wrapped/wrappedatkbridge.c"
    "${BOX64_ROOT}/src/wrapped/wrappedatomic.c"
    "${BOX64_ROOT}/src/wrapped/wrappedatspi.c"
    "${BOX64_ROOT}/src/wrapped/wrappedbz2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedcairo.c"
    "${BOX64_ROOT}/src/wrapped/wrappedcairogobject.c"
    "${BOX64_ROOT}/src/wrapped/wrappedcrashhandler.c"
    "${BOX64_ROOT}/src/wrapped/wrappedcrypto.c"
    "${BOX64_ROOT}/src/wrapped/wrappedcurl.c"
    "${BOX64_ROOT}/src/wrapped/wrappeddbus.c"
    "${BOX64_ROOT}/src/wrapped/wrappeddbusglib1.c"
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
    "${BOX64_ROOT}/src/wrapped/wrappedglib2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgmodule2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgmp.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgnutls.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgobject2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgssapi.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgssapikrb5.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgstapp.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgstaudio.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgstbase.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgstgl.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgstreamer.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgsttag.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgstvideo.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgthread2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgtk3.c"
    "${BOX64_ROOT}/src/wrapped/wrappedgtkx112.c"
    "${BOX64_ROOT}/src/wrapped/wrappedkrb5.c"
    "${BOX64_ROOT}/src/wrapped/wrappedldlinux.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibasound.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibc.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibcrypt.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibcups.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibdl.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibdrm.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibegl.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibform.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibformw.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibfuse.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibgl.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibglu.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibglx.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibibus.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibice.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibm.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibncurses.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibncurses6.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibncursesw.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibncursesw6.c"
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
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcb.c"
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
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbshape.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbshm.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbsync.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbutil.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlibxcbxinerama.c"
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
    "${BOX64_ROOT}/src/wrapped/wrappedopenal.c"
    "${BOX64_ROOT}/src/wrapped/wrappedpango.c"
    "${BOX64_ROOT}/src/wrapped/wrappedpangocairo.c"
    "${BOX64_ROOT}/src/wrapped/wrappedpangoft2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedplc4.c"
    "${BOX64_ROOT}/src/wrapped/wrappedplds4.c"
    "${BOX64_ROOT}/src/wrapped/wrappedpng16.c"
    "${BOX64_ROOT}/src/wrapped/wrappedpulse.c"
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
    "${BOX64_ROOT}/src/wrapped/wrappedxml2.c"
    "${BOX64_ROOT}/src/wrapped/wrappedxslt.c"
    "${BOX64_ROOT}/src/wrapped/wrappedldapr.c"
    "${BOX64_ROOT}/src/wrapped/wrappedlber.c"
    "${BOX64_ROOT}/src/wrapped/wrappedvulkan.c"
    "${BOX64_ROOT}/src/wrapped/wrappedxshmfence.c"
    "${BOX64_ROOT}/src/wrapped/wrappedd3dadapter9.c"
)

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

if(DYNAREC)
    set(DYNAREC_SRC
        "${BOX64_ROOT}/src/dynarec/dynablock.c"
        "${BOX64_ROOT}/src/dynarec/dynarec_native.c"
    )
endif()

if(ARM_DYNAREC)
    set(DYNAREC_SRC
        ${DYNAREC_SRC}

        "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_functions.c"
        "${BOX64_ROOT}/src/dynarec/arm64/arm64_printer.c"
        "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_jmpnext.c"

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
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_64.c"
    #"${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_65.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_66.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_67.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_d8.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_d9.c"
    #"${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_da.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_db.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_dc.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_dd.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_de.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_df.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_f0.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_660f.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_6664.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_66f0.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_f20f.c"
    "${BOX64_ROOT}/src/dynarec/arm64/dynarec_arm64_f30f.c"
    )
endif()

if(RV64_DYNAREC)
    set(DYNAREC_SRC
        ${DYNAREC_SRC}

        "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_functions.c"
        "${BOX64_ROOT}/src/dynarec/rv64/rv64_printer.c"
        "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_jmpnext.c"

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
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_0f.c"
    #"${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_64.c"
    #"${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_65.c"
    #"${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_66.c"
    #"${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_67.c"
    #"${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_d8.c"
    #"${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_d9.c"
    #"${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_da.c"
    #"${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_db.c"
    #"${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_dc.c"
    #"${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_dd.c"
    #"${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_de.c"
    #"${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_df.c"
    #"${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_f0.c"
    #"${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_660f.c"
    #"${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_6664.c"
    #"${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_66f0.c"
    #"${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_f20f.c"
    "${BOX64_ROOT}/src/dynarec/rv64/dynarec_rv64_f30f.c"
    )
endif()

if(DYNAREC)
    set(DYNAREC_PASS
        "${BOX64_ROOT}/src/wrapped/generated/wrapper.h"
        ${DYNAREC_PASS}
        "${BOX64_ROOT}/src/dynarec/dynarec_native_pass.c"
    )

    add_library(dynarec_native OBJECT ${DYNAREC_SRC})

    add_library(native_pass0 OBJECT ${DYNAREC_PASS})
    set_target_properties(native_pass0 PROPERTIES COMPILE_FLAGS "-DSTEP=0")
    add_library(native_pass1 OBJECT ${DYNAREC_PASS})
    set_target_properties(native_pass1 PROPERTIES COMPILE_FLAGS "-DSTEP=1")
    add_library(native_pass2 OBJECT ${DYNAREC_PASS})
    set_target_properties(native_pass2 PROPERTIES COMPILE_FLAGS "-DSTEP=2")
    add_library(native_pass3 OBJECT ${DYNAREC_PASS})
    set_target_properties(native_pass3 PROPERTIES COMPILE_FLAGS "-DSTEP=3")
    add_dependencies(native_pass0 WRAPPERS)
    add_dependencies(native_pass1 WRAPPERS)
    add_dependencies(native_pass2 WRAPPERS)
    add_dependencies(native_pass3 WRAPPERS)

    add_library(dynarec STATIC
        $<TARGET_OBJECTS:dynarec_native>
        $<TARGET_OBJECTS:native_pass0>
        $<TARGET_OBJECTS:native_pass1>
        $<TARGET_OBJECTS:native_pass2>
        $<TARGET_OBJECTS:native_pass3>
    )
endif()

# creates git_head.h
if(DYNAREC)
    add_custom_command(
        OUTPUT "${BOX64_ROOT}/src/git_head.h"
        COMMAND sh -c "echo \\\#define GITREV \\\"$(git rev-parse --short HEAD)\\\">\"${BOX64_ROOT}/src/git_head.h\""
        DEPENDS dynarec ${ELFLOADER_SRC} ${WRAPPEDS}
        VERBATIM)
else()
    add_custom_command(
        OUTPUT "${BOX64_ROOT}/src/git_head.h"
        COMMAND sh -c "echo \\\#define GITREV \\\"$(git rev-parse --short HEAD)\\\">\"${BOX64_ROOT}/src/git_head.h\""
        DEPENDS ${ELFLOADER_SRC} ${WRAPPEDS}
        VERBATIM)
endif()

add_executable(${BOX64} ${ELFLOADER_SRC} ${WRAPPEDS} "${BOX64_ROOT}/src/git_head.h")
set_target_properties(${BOX64} PROPERTIES ENABLE_EXPORTS ON)
add_dependencies(${BOX64} WRAPPERS)
#add_dependencies(${BOX64} PRINTER)
#target_link_libraries(${BOX64} c m dl rt pthread resolv)
if(STATICBUILD)
    set_target_properties(${BOX64} PROPERTIES LINK_FLAGS "-static -Wl,--no-as-needed -Wl,--whole-archive -Wl,--allow-multiple-definition -lm -ldl -lrt -lpthread -lresolv -lc -Wl,-defsym,_DYNAMIC=0 -pthread")
else()
    set_target_properties(${BOX64} PROPERTIES LINK_FLAGS "-Wl,--no-as-needed -lc -lm -ldl -lrt -lpthread -lresolv -Wl,--as-needed -pthread")
endif()
if(DYNAREC)
    target_link_libraries(${BOX64} dynarec)
endif()

if(${CMAKE_VERSION} VERSION_LESS "3.13")
    if(NOT NOLOADADDR)
        if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
            set_target_properties(${BOX64} PROPERTIES LINK_FLAGS "-Wl,--image-base=${BOX64_ELF_ADDRESS}")
        else()
            set_target_properties(${BOX64} PROPERTIES LINK_FLAGS "-Wl,-Ttext-segment,${BOX64_ELF_ADDRESS}")
        endif()
    endif()
else()
    # If symbols are missing, try this: target_link_options(${BOX64} PUBLIC -rdynamic)
    if(NOT NOLOADADDR)
        if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
            target_link_options(${BOX64} PUBLIC LINKER:--image-base=${BOX64_ELF_ADDRESS})
        else()
            target_link_options(${BOX64} PUBLIC LINKER:-Ttext-segment,${BOX64_ELF_ADDRESS})
        endif()
    endif()
endif()

string(COMPARE EQUAL "${CMAKE_HOST_SYSTEM_PROCESSOR}" "i686"  _x86)
string(COMPARE EQUAL "${CMAKE_HOST_SYSTEM_PROCESSOR}" "x86_64"  _x86_64)
string(COMPARE EQUAL "${CMAKE_HOST_SYSTEM_PROCESSOR}" "aarch64"  _aarch64)

if(_x86_64 OR _aarch64)
  add_definitions(-DCONFIG_64BIT)
endif()

if(NOT _x86 AND NOT _x86_64)
  install(TARGETS ${BOX64}
    RUNTIME DESTINATION bin)
  configure_file(system/box64.conf.cmake system/box64.conf)
  install(FILES ${CMAKE_BINARY_DIR}/system/box64.conf DESTINATION /etc/binfmt.d/)
  install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libstdc++.so.5 DESTINATION /usr/lib/x86_64-linux-gnu/)
  install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libstdc++.so.6 DESTINATION /usr/lib/x86_64-linux-gnu/)
  install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libgcc_s.so.1 DESTINATION /usr/lib/x86_64-linux-gnu/)
  install(FILES ${CMAKE_SOURCE_DIR}/x64lib/libpng12.so.0 DESTINATION /usr/lib/x86_64-linux-gnu/)
  install(FILES ${CMAKE_SOURCE_DIR}/system/box64.box64rc DESTINATION /etc/)
endif()

if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "ptitSeb")
set(CPACK_PACKAGE_CONTACT "ptitSeb@box86.org")
set(CPACK_PACKAGE_DESCRIPTION "Box64 - Linux Userspace x86_64 Emulator with a twist")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6")
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
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/postinst")
if(_aarch64)
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
elseif(_x86_64)
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "x86_64")
else()
    #probably wrong...
    execute_process(COMMAND "dpkg --print-architecture" OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE)
endif()
set(CPACK_DEBIAN_FILE_NAME "${BOX64}-${BOX64_MAJOR}.${BOX64_MINOR}.${BOX64_REVISION}_Linux-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}.deb")
INCLUDE(CPack)

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

add_test(cppThreads ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test10 -D TEST_OUTPUT=tmpfile10.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref10.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

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

    add_test(longjumpInSignals ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test21 -D TEST_OUTPUT=tmpfile21.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref21.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    add_test(x87 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test22 -D TEST_OUTPUT=tmpfile22.txt
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref22.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    file(GLOB extension_tests "${CMAKE_SOURCE_DIR}/tests/extensions/*.c")
foreach(file ${extension_tests})
    get_filename_component(testname "${file}" NAME_WE)
    add_test(NAME "${testname}" COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/extensions/${testname} -D TEST_OUTPUT=tmpfile-${testname}.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/extensions/${testname}.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake)
endforeach()
