Current version
======
* Added an option to get generated -NAN on SSE2 operation
* Many small fixes and improvement to get SteamPlay (i.e. proton) working (along with box86)
* Added some workaround for program calling uname to mimic x86_64 system
* Added some granularity to mmap, and generic memory protection tracking improvment, to mimic x86 system (for wine stagging and proton)
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
* Added more dynarec opcodes
* A few fixes in some Dynarec opcodes
* A bit more optimisation in some Dynarec opcodes
* Don't quit on unsupported syscall, return -1 / ENOSYS error
* Added some more Syscall
* Wrapped Vulkan library. Vulkan and dxvk now supported!
* Support building on PPC64LE (interpretor only)
* Added support for Risc-V (interpretor only)

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
* Added loongarch64 support (interpretor only)
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
* A few more opcode added to the Interpretor and the Dynarec
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
* A few fixes to some interpretor opcodes

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
