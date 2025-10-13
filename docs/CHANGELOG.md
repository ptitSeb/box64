v0.3.8
======

Highlights:

* This version introduce DynaCache, a disk cache for the generated native code. It's not enabled by default for now (but it will use cache file if present by default). It can dramaticaly speedup the load of program (try with linux factorio for example). This version also improve in the volatile metadata handling. There is a massive compatiblity improvment for box32 and on all dynarec in general too. And few more function wrapping...

    => DynaCache for faster loading of program, and avoid runtime hicups
    => Batter box32 compatibility (much less "Out of Memory" error)
    => Unity3D games on Wayland are working now
    => Volatile Metadata fully supported and handled, making games like Starfield to just work.
    => More work on AVX support for RV64 & LA64. LA64 almost there, while RV64 has Scalar only support (no hardware vector support yet for AVX)

Version summary:

* Improved and fixed memory tracking
* Improved volatile metadata handling, and some light refactor on StrongMem emulation
* Improved handling of TF flag
* DynaCache: Introducing a way to cache on disk native generated code for faster startup time on subsequent release
* DynaCache: Support for all 3 backends ARM64, RV64 & LA64
* Interpreter: refactor opcodes decoding to have prefixes handled in a generic way
* Interpreter: many small fixes
* Dynarec: some optimisations in the jump table, to remove one memory fetch on each jump
* Dynarec: create infrastructure to be able to change easily the reg mapping
* Dynarec: optimised defered flag handling to limit cases where UpdateFlags is needed
* Dynarec: Introduced BOX64_DYNAREC_NOHOTPAGE to disable detection of hotpages
* Dynarec: ARM64: A few more opcode added
* Dynarec: ARM64: Some small fixes and better handling of undefined behavior in opcodes, as well as undefined opcodes
* Dynarec: ARM64: Implemented a Dynarec version of UpdateFlags, for improved performances
* Dynarec; ARM64: Fixes/Improved SSE_FLUSHTO0 support
* Dynarec: LA64: Added support for AVX (& friends) opcodes. Still need some debugging so AVX still disabled by default on LA64
* Dynarec: LA64: Changed register mapping to be closer to the x86_64 ABI
* Dynarec: LA64: RV64: Added limited support to SSE_FLUSHTO0
* Dynarec: RV64: Many fixes and improvments for MMX opcodes
* Dynarec: RV64: many Scalar version of AVX opcodes added
* Dynarec: Many fixes to various OpCodes, on all 3 backends
* WowBox64: Added minimal runtime, improved overall stability
* BOX32: Wrapper: many new functions added, plus many fixes
* BOX32: Improved memory hadnling, using 32bits personality and other tunning to get more reliable 32bits memory allocations
* BOX64: Wrapper: some more wrapped functions added, including wayland (for Unity games on Wayland) and vulkan (more extensions)
* BOX64: Wrapper: improved handling of some VarArg wrapped functions on non-ARM64 backend
* RCFILE: Fixed MAXCPU not working properly with Wine programs
* RCFILE: Generic Setup profile refined, and more games profile added
* Trace: Added support for current libZydis
    
v0.3.6
======

Highlights:

* This version introduces wowbox64.dll for Hangover; the ability to use the Volatile Metadata of Windows executables for x64 wine; and some better x87/SSE/AVX handling

    => WowBox64 can be built directly from Box64 repo, and can be found in the CI artifacts also
    => it also supports a subset of BOX64_XXXX settings, which can be set from command line or via a setting file ".box64rc" in the Wine prefix home
    => Volatile Metadata, when present, allows only applying strong memory ordering when needed, as marked by the compiler, giving overall better performances
    => The precision control bit of x87 is now handled, allowing some older games to run correctly
    => More work has been done in the handling of precise NaN and Round handling in SSE & AVX opcodes

Version summary:
    
* Added/fixed some Syscalls, helping Go programs
* Wrapper: more functions added, and some fixes too. Vulkan is 1.4 now, among other things
* Wrapper: Added a few more wrapped libs (like some avcodec and friends)
* Improve internal memory allocator (with a dedicated allocator for small memory allocations), and fixed some issues around it
* Improve internal mmaped file tracking
* Box32: Some small refactoring around memory management and internal memory allocations
* Box32: More functions wrapped
* Box32: More libraries wrapped
* ARM64: Added support for Atomic extension in many internal functions (not related to Dynarec).
* ARM64: Added support for CRC2 extension for internal CRC functions
* Interp: A few fixes to some opcodes, and better NAN handling on some selected SSE/AVX opcodes
* Interp: Added a few opcodes with exotic prefixes
* Dynarec: Introcude BOX64_DYNAREC_ROUND=2 to handle x87 precision control bit (ARM64 and RV64, LA64 still doesn't handle x87 yet)
* Dynarec: Some small optimizations on Strongmem emulation, making it more efficient
* Dynarec: Some speed optimizations on code that do SMC and waiting slot
* Dynarec: Added CALLRET=2 to improve call/ret optimization compatibility with SMC code
* Dynarec: ARM64: Various optimizations and fixes on some opcodes, especially AVX ones
* Dynarec: ARM64: Added missing FASTNAN=0 handling on some SSE/AVX opcodes
* Dynarec: ARM64: Added some exotic prefixed opcodes
* Dynarec: ARM64: Avoid purging XMM/YMM register on intra-block loop if possible
* Dynarec: RV64: Many fixes and improvments around x87 emulation
* Dynarec: RV64: Added some more missing opcodes, especially some LOCK prefixed ones and many MMX ones
* Dynarec: RV64: Various fixes on some existing opcodes
* Dynarec: RV64: Improved nativeflags handling, allowing for more cases to use it
* Dynarec: LA64: Added some more opcodes and MMX handling
* Dynarec: LA64: Various fixes to some existing opcodes
* Android: Improved signal handling, structure alignment, and transformation
* WowBox64: Created the dll, with RcFile, Env. Var. handling and log printout (both ARM64 Dynarec and Interpreter available)
* Cosim: Various fixes and improvements to limit false negatives and improve readability of logs
* RcFile: A few fixes for some values that would be ignored

v0.3.4
======
* This version is Faster and more compatible:
    => The RV64 backend got faster and more stable, with lots of RVV 1.0 / xThreadVector support to emulate SSE/SSE2+ opcode (no AVX yet)
    => On ARM64, Box64 now support more DRM types than before.
    => BOX32 can run steam now. At least on ARM64 backend, it's still not working on RV64 and LA64 for now.
* Added support to emulate Windows Syscall (needs Proton and a 48bits address space for now)
* Added BOX64_DYNAREC_DIRTY to have a faster (but less safe) way to handle code that write in it's current page
* Added a few rarely used opcodes
* Added experimental support for GDBJIT, that allow an x86/x86_64 program to be debugged on ARM/RV64/LA64 platform with x86 regs and opcode view (might generate a lot of file tho)
* Added support for Perf tools in a similar way if GDBJIT, to have an x86 fine view of the performances
* Reworked undefined flags for common operations, to be similar to real CPU
* Reworked cpuid handling, adding BOX64_CPUTYPE support to select Intel/AMD emulation (no 3DNow! support on AMD for now)
* Reworked ucontext link to be closer to the real thing (and make it more simple)
* Improved memory traking, file descriptor backed memory map, and detect wine loaded dll
* Introduced settings per library and dll (mostly for Dynarec)
* Wrapper: More functions addes, and some fixes too
* BOX32: Many fixes around pthreads wrapping
* BOX32: More functions wrapped.
* BOX32: More libraies wrapped.
* BOX32: Some reworks on how high memory (higher than 32bits) is masked, and can be used for Dynarec blocks.
* Dynarec: The usual batch of fixes, improvments and opcodes additions on all 3 supported backend
* Dynarec: Some fixes to NativeFlags handling, both on ARM64 and RV64
* Dynarec: Added optimized REP MOVSB (expected for default CPUTYPE=0, according to new cpuid return)
* Dynarec: LA64: more opcoded addes, using hardware extensions like AES
* Dynarec: RV64: More work on using RVV 1.0 and xTheadVector extension to emulate SSE/SSE2+ opcodes
* Dynarec: RV64: Added informations on flags when building blocks, to be used when a signal happens to rebuild a x86 context more accurate
* Dynarec: ARM64: Reworked undefined flags for common operations, to be similar to real CPU
* Dynarec: ARM64: Added informations on flags and xmm/ymm/x87 register when building blocks, to be used when a signal happens to rebuild a x86 context 100% accurate
* TRACE: Reworked how trace for wrapped function call is printed.
* TRACE: Reworked logs to prefix each line with a (optionaly colored) BOX32 or BOX64
* COSIM: Some rework on cosim to limit false negative, especialy when handling x87 operations
* LA64: Added limited support for ABI 1.0
* CI: github CI now also generate MiceWine .rat archive, along with Winlator .wcp archive and regular linux builds.
* RCFile: Added many new games profiles (both for speed improvment or for compatibilty)

v0.3.2
======
* Introduced Box32 to run 32bits apps on 64bits OS
    => Box32 is a build option for Box64, and is disabled by default
    => Optionnal binfmt integration available for Box32
    => Wrapped some basics libs, including graphics and sounds so a few games already works
    => Work In Progress, many issues remain to be fixed, but some Linux games are playable
* Introduced Native Flags. Can be controled with BOX64_DYNAREC_NATIVEFLAGS, but enabled by default
    => Available on the 3 Dynarec backend, with different implementations
    => ARM64 Dynarec build have all AVX/AVX2 extensions enabled by default
    +> Can lead to large speedup on certain cases
* Improved Strong Memory Model emulation, with better default options and a more efficiant emulation
* Dynarec: RV64: using RVV (and XThreadVector) to implement SSEx opcodes, with some massive speedup when used!
* Some bugfix and improvment in the Signal handler and internal memory tracking
* Improvment to TRAP signal generation and Handling
* Improved CPUID opcodes again, and the various virtual file in /proc that represent those data
* Improved some x87 operations, like long double handling and infinity comparisons...
* Dynarec: ARM64 fixes to a few opcodes
* Dynarec: RV64 fixes to many opcodes
* Dynarec: LA64 fixes to a few opcodes
* Dynarec: RV64 fixes and improvments on LOCK prefixes opcodes
* Dynarec: Many optimisation on some individual opcodes, an all 3 backends
* WRAPPING: More libs and functions wrapped. Some wrapping fixes too
* WREPPING: New wrapperhelper to help coding new wrapping & fixing/enhancing existing one
* ElfLoader: Improved object fetching
* New build profile for SD865 and ORYON
* Changed the installation folder of x86 libs to avoid conflict with linux distro
* Added a few syscalls
* Some more Vulkan extensions wrapped
* More work on build system, and github CI (generating WCP archive), with some fixes on the Android version (still not 100% operational)

v0.3.0
======
* AVX and AVX2 implemented, along with BMI1, BMI2, ADX, FMA, F16C and RDANDR extension!
* Introduced BOX64_AVX env. var. to enable/disable AVX extension:
    => BOX64_AVX=1 enable AVX, BMI1, F16C and VAES extension
    => BOX64_AVX=2 also enable AVX2, BMI2, FMA, ADX,VPCLMULQDQ and RDRAND
* Dynarec: Implemented most AVX/AVX2 and friends in ARM64 Dynarec (using only NEON, so available on all ARM64 CPU)
    => ARM64 Dynarec build have all AVX/AVX2 extensions enabled by default
* Improved CPUID opcodes, adding a few leaf, fixing same. Returnning an Haswell type of CPU now (was Pentium IV before)
* Dynarec: Small optim in the emit_pf helper, on all 3 backends
* Dynarec: ARM64 some fixes to a few opcodes
* Dynarec: LA64 many new opcodes added, and fixes.
* Dynarec: RV64 some new opcodes added, many fixes.
* Dynarec: RV64 Adding some infrastructure to use RVV (if available) for SIMD emulation.
* Dynarec: Many small changes and fixes around handling of x87, registers and internal jump.
* WRAPPING: Added a workaround on SDL_GetThreadID to fix a race condition that was happening mostly on LA64 and broke many SDL games
* WRAPPING: More libs and functions wrapped. Some wrapping fixes too (including a nasty error on some rare libGL functions)
* WRAPPING: Wayland is now supported. Wine-wayland at least works now
* Dynarec: LA64 improved LOCK CMPXCHG handling, and better 16Byte LOCK handling too (fixes many Unity3D games)
* Dynarec: RV64 improved LOCK CMPXCHG handling
* COSIM: Many fixes and improvment, to make BOX64_DYNAREC_TEST more usable
* Detect when a Wine program use UnityPlayer.dll to atumaticaly use BOX64_DYNAREC_STRONGMEM=1 (unless it's already set), so more games just work
* ElfLoader: Added a new type of relocation

v0.2.8
======
* Wrapping: More libs and function wrapping (gtk3, vulkan...)
* Reworked X11 Callback handling, for better stability
* Reworked exit and the load/unload of libs and symbol resolution
* Some more profiles in box64rc
* Better way to rename executable (including wine ones) so driver like mesa can apply the correct profile
* Reworked memory managment in the Dynarec, limiting amount of dynamic allocation
* Improved memory tracking and managment. Also introduce BOX64_MMAP32 to force 32bits compatible mmap for external libs (like mesa)
=> Allow Vulkan drvier on Wine/WOW64 games to allocate 32bits compatible memory, allowing dxvk on 32bits.
* Reworked memory managment in the Dynarec, limiting amount of dynamic allocation
* Lots of improvments in x87 handling (virtual stack, rounding, ffree, etc.)
=> should improve games stability
* Dynarec: ARM64 some opcodes optimisation, especialy shift and rotation ones
* Dynarec: RV64 lots of opcoded added.
=> Some games on linux now have a good speed. Game on Wine are still a bit slow, and dxvk still have issues
* Dynarec: Added Loogarch backend! Note that Loongarch are 16K pagesize platform by default.
=> Linux games are running, and with good speed, especialy on 3A6000 platforms. Games on Wine are not working yet due to 16k pagesize limitation
* Dynarec: Better handling of large block in the dynarec
* Dynarec: Lots of improvments and fixes to BOX86_DYNAREC_CALLRET optimisation.
=> It can be used on more software now, and combine with BOX64_DYNAREC_BOGBLOCKS 2 or 3 can gives some good speed boosts.

v0.2.6
======
* X64Libs: Updated libstdc++ & libgcc_s
* X64Libs: Use x86_64 vrsion of libunwind (it's needed by wine)
* Dynarec: More opcodes on ARM64 & RV64
* Dynarec: Fixed some flags propagation issues inside blocks
* Dynarec: Improved Strong Memory Model emulation, introducing a 3rd level
* Dynarec: ARM64: reworked shift and bit opcodes flags handling
* Dynarec: RV64: Many fixes, making Wow64 apps running
* Dynarec: ARM64: Added support for SSE4.2 (with CRC32 if supported by CPU)
* Dynarec: ARM64: Added support for SHA (with hardware support if CPU allows it)
* Dynarec: ARM64: The CALLRET optimisation has been improved and is now default
* CPU: Remoworked how INT, priviliged opcodes and teh Trace flags works
* CPU: Added full support for SSE4.2
* CPU: Added full support for SHA extension
* ElfLoader: rework how elf memory is handled
* Emulation: Added some more syscalls
* Emulation: Better handling of Signal (also handling SIGMAX correctly)
* Emulation: Better support for POKEUSER/PEEKUSER (helps Windows Unity games)
* Wrapping: More libs and some fixes in function wrapping (gtk3, vulkan...)
* Wrapping: Removed faked libunwind wrapping
* Hardware: Added more profile, with support ADLink Ampere Altra, Qualcomm 8Gen3, and Pi5 machines
* OS: Added profile to build for Android and Termux.
* OS: New option to use mold for a high speed link
=> More hadware support, more functions wrapped for an improve compatibity
=> More CPU extension, with ARM64 hardware support when possible, and improved CALLRET optimisation for an improved speed of emulation
=> Some work also to reduce the memory footprint

v0.2.4
======
* Added Dynarec for RISC-V
* Added partial 32 bits support (WoW64 works)
* Changed cpuid to display box64
* Fixed steam, again
* Improved hacked version for overridden malloc/free functions
* More wrapped libs
* More wrapped functions and syscalls
* Reduced memory footprint
* Added support for the BIND_NOW flags in ELF files
* Added RTLD_NOLOAD support
* Fixed some typos and general cleanup, again
* Added a cosimulation mode (to compare the behaviour of the dynarec with the interpreter)
* Added FASTNAN/FASTROUND handling for more opcodes
* Better float/double/long double handling
* Better dlopen/dlclose handling
* More games in the rc file
* Added BOX64_DYNAREC_MISSING to show only missing opcode, independantly of the log level
* Added BOX64_RESERVE_HIGH to reserve addresses above 47bits
* Improved multitreading support
* Ukrainian translation of the README
=> Dynarec now support RISC-V, large speedup on RISC-V 64, were games are now playable (like Stardew Valley on a Vision Five 2)
=> Improved compatibility with more fixes in elf handling, more wrapped libs and functiond, more fixed opcodes and refined profile

v0.2.2
======
* Added a hacked version for overridden malloc/free functions, enabling libcef (and derivative) to work.
    => Steam Large mode and new BigPicture are working
    => Heroic Launcher is working
* Added rcfile handling: box64 now read `/etc/box64.box64rc` and `~/.box64rc` and apply per process parameters
    => Every program can be finetuned, for speed or just to make them work
* Added BOX64_DYNAREC_CALLRET to optimized CALL/RET opcode using semi-direct native CALL/RET (get more than 10% speedup)
* Added BOX64_DYNAREC_FORWARD to allow bigger Dynarec block to be build (can get more then 30% of Speedup!)
* Improved memory protection tracking and signal handling, again
* Box64 now return a custom name for the CPU instal of P4, build with Box64 and the cpu name it's running on
* Simplified mutex handling (faster, cleaner)
* Reduce memory used by the memory tracking (memory almost divide by 4 is some critical use case)
* Added d3dadapter9 wrapping
    => gallium nine is now supported on box64 too
* More wrapped libs (especially on the gtk familly)
* Improved/fixed some Syscall handling
* Refactored Strong Memory Model emulation (faster and more accurate)
* Added some more opcode to ARM64 Dynarec, and fixed some other
* Some fixes to flags handling in the Dynarec on some special cases
* Refactor library loading/unloading, with refcount.
* Some special cases fixes for dlopen (fixes deadcells, maybe others too)
* Improved mmap mapping, espcially with MAP_32BITS flag
* More wrapped functions and syscalls
* Lots of improvment in the Rounding of float & double (Dynarec and Interpreter), and proper handling of denormals when converting to/from 80 bits long double
* Added specific suppport for RK3588 board (needed some hack in signal handling, probably due to the use of a non mainlined kernel)
* More support on the RV64 port
* More support to build under musl environment

v0.2.0
======
* Wrapped more vulkan function, dxvk 2.0 now works
* Added support for overridden malloc/free functions
* Refactor Dynarec memory management, reducing memory footprint (from 20% to 40% of Dynarec size)
* Improved elf loader
* Improved dlsym handling with RTLD_NEXT
* Added BOX64_DYNAREC_SAFEFLAGS to allow finetuning Dynarec generated code handling of flags accross function calls and returns.
* Added BOX64_BASH to setup x86_64 bash binary, so shell script can be run in x86_64 world 
* Added BOX64_ROLLING_LOG to have details log only on crash
* Work on alignment for some of pthread structures
* More wrapped libraries in the gtk family
* Some fixes on the Dynarec x87 code (fixing a bunch of games like ut2004 or Piczle Puzzle)
* Improved TLS Size handling (Unity3D games now works)
* More PageSize Compile options
* Improved `execv` family of function to have better box64 integration
* Added support for wrapped libfuse so AppImage works
* Adjusted the binfmt integration so AppImage are automaticaly picked
* Improved FAudio, SDL2, GL (and more) wrapping
* More wrapped libs, like libEGL.so, to helps emulated Qt4/5 libs to load
* Improved Signal Handling and memory protection traking
* Some signature fixes to some wrapped functions (in libc)
* Added more syscalls
* Added a lot of new opcodes to the ARM64 Dynarec
=> Lot's of new games stat working, to name a few: Surviving Mars, Piczle Cross Adventure (using Proton experimental), UT2004, Obduction, Machinarium, Silicon Zero, Hacknet, Absolute Drift, Art of Rally, Dota2
=> Some apps also runs now, like TeamSpeak 3 Client and Server or Discord
=> The bash integration is usefull for device with no binfmt integration or for script that check the machine it's running on

v0.1.8
======
* Added an option to get generated -NAN on SSE2 operation
* Many small fixes and improvement to get SteamPlay (i.e. proton) working (along with box86)
* Added some workaround for program calling uname to mimic x86_64 system
* Added some granularity to mmap, and generic memory protection tracking improvment, to mimic x86 system (for wine stagging and proton)
* Better clone syscall and libc implementation
* Improved wrapping of GTK libs
* More wrapped libs (more gtk libs, like gstreamer-1.0 among other non-gtk libs too)
* More wrapped functions(with some tricky one, like dladdr1)
* Some fixes and improvment to some wrapped functions
* Refactor Dynarec:
    * better handling of SSE/x87 registers
    * optimisation to limit float/double conversion
    * better handling of "internal" jump inside a dynablock
    * make adding new architecture easier, by sharing as much infrastructure as possible
* Improved elfloader
* Improved some tests
* Improved a bit the "Hotpage" handling and messages
* Added more dynarec opcodes
* A few fixes in some Dynarec opcodes
* A bit more optimisation in some Dynarec opcodes
* Don't quit on unsupported syscall, return -1 / ENOSYS error
* Added some more Syscall
* Wrapped Vulkan library. Vulkan and dxvk now supported!
* Support building on PPC64LE (interpreter only)
* Added support for Risc-V (interpreter only)

v0.1.6
======
* Introduce "HotPage", to temporarily disable Dynarec on a page were writing is also occuring (can help speed up C# code)
* Some work on Dynarec to limit the number of mutex use, and also allow smaller block to be built (for JIT'd programs)
* Introduce BOX64_DYNAREC_STRONGMEM for the (Arm) dynarec to emulate StrngMemory Model (needed by some games, like RimWorld)
* More functions wrapped
* Improve speed of the custom allocator used by dynarec and Hash (speedup loading)
* Added a workaround for streamwebhelper to not load it
* More opcodes added
* More wrapped libraries, including gtk2
* Added a fix for linking box64 on system using glibc 2.34+
* Some residual fixes on thread attribute handling
* Better handling of app that want 32bits jumps, like unity3d/mono (thanks mogery)
* More ARM hardware option in CMake (Phytium, NX...)
* Added loongarch64 support (interpreter only)
* Some fixes to elfloader
* Added a mecanism to cancel a Dynarec block construction if it triggers a segfault (instead of a crash)

v0.1.4
======
* Reworked README a bit
* Added some more wrapped libs
* Added some SSE4x opcodes
* Preliminary support for VSyscall (still missing VDSO)
* Added support for AES-NI cpu extension, also in Dynarec (and using ARM extension if present)
* Added some syscall
* Added some more wrapped functions
* Improvment to the handling of longjmp inside signal handlers
* Improved wrapper helpers (from rajdakin)
* Some fixes to opcodes (especially C2 flags for some x87 opcodes)
* Handling of some specific opcode as privileged instruction (instead of NOP)
* Fixed ELF signature detection
* Some fixes to a few env. var. handling

v0.1.2
======
* OpenSource!
* A few more opcode added to the Interpreter and the Dynarec
* Improved Native Function calling
* Added some function to limit allocated memory to 47bits space (for Wine)
* Improved and Fixed the functions to limit allocated memory to 32bits space
* Wine is starting now, but not everything and every version works

v0.1.0
======
* Improvement to ElfLoader
* More opcode added to the Dynarec
* More lib wrapping
* Some fixes to a few function signature
* Improvement to Signal handling, and internal mutex handling
* A few fixes to some dynarec opcodes
* A few fixes to some interpreter opcodes

v0.0.8
======
* Improvement to SSE/x87 handling
* More opcode added to the Dynarec
* More lib wrapping

v0.0.6
======
* Added Dynarec for ARM64
* Many general CPU opcode added to the Dynarec
* Many SSEx opcodes added to the Dynarec
* Added a few more symbols in libc
* A few changes to the "thread once" handling
* New Logo and Icon from @grayduck

v0.0.4
======
* Added a lot of opcodes
* Added SDL1 and SDL2 wrapping
* OpenGL/GLU wrapping done
* libasound / libpulse wrapping done
* libopenal / libalure / libalut wrapping done
* Many X11 libs wrapped
* Added "install" with binfmt integration
* A few games are actualy starting now (old WorldOfGoo, Into The Breach, VVVVVV and Thimbeweed Park)

v0.0.2
======
* Using box86 as a base, implemented most planned box64 part except dynarec
* Wrapped a few libs
* ELF loader a bit crude, but seems working
* Signal handling implemented
* MMX and x87 are sharing their registers
