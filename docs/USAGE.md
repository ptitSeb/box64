
Usage
----

There are many environment variables to control Box64 behaviour. 
Env. var with * can also be put inside box64rc files. 
Box64 look for 2 places for rcfile: `/etc/box64.box64rc` and `~/.box64rc`
The second takes precedence to the first, on an APP level 
(that means if an [MYAPP] my appears in both file, only the settings in `~/.box64rc` will be applied)

#### BOX64_LOG *
Controls the Verbosity level of the logs
 * 0: NONE : No message (except some fatal error). (Default.)
 * 1: INFO : Show some minimum log (Example: librairies not found)
 * 2: DEBUG : Details a lot of stuff (Example: relocations or functions called).
 * 3: DUMP : All DEBUG plus DUMP of all ELF Info.

#### BOX64_ROLLING_LOG *
Show last few wrapped function call when a Signal is caught
 * 0: No last function call printed (Default.)
 * 1: Last 16 wrapped functions calls printed when a signal is printed. Incompatible with BOX64_LOG>1 (may need BOX64_SHOWSEGV=1 also)
 * N: Last N wrapped functions calls printed when a signal is printed. Incompatible with BOX64_LOG>1 (may need BOX64_SHOWSEGV=1 also)

#### BOX64_NOBANNER
Disables Box64 printing its version and build
 * 0 : Enable printing its banner. (Default.)
 * 1 : Disable printing its banner. 

#### BOX64_LD_LIBRARY_PATH *
Path to look for x86_64 libraries. Default is current folder and `lib` in current folder.
Also, `/usr/lib/x86_64-linux-gnu` and `/lib/x86_64-linux-gnu` are added if they exist.

#### BOX64_PATH *
Path to look for x86_64 executable. Default is current folder and `bin` in current folder.

#### BOX64_DLSYM_ERROR *
Enables/Disables the logging of `dlsym` errors.
 * 0 : Don't log `dlsym` errors. (Default.)
 * 1 : Log dlsym errors.

#### BOX64_TRACE_FILE *
Send all log and trace to a file instead of `stdout`
Also, if name contains `%pid` then this is replaced by the actual PID of box64 instance
End the filename with `+` to have thetrace appended instead of overwritten
Use `stderr` to use this instead of default `stdout` 

#### BOX64_TRACE *
Only on build with trace enabled. Trace allow the logging of all instruction executed, along with register dump
 * 0 : No trace. (Default.) 
 * 1 : Trace enabled. Trace start after the initialisation of all depending libraries is done.
 * symbolname : Trace only `symbolname` (trace is disable if the symbol is not found).
 * 0xXXXXXXX-0xYYYYYYY : Trace only between the 2 addresses.

#### BOX64_TRACE_INIT *
Use BOX64_TRACE_INIT instead of BOX64_TRACE to start trace before the initialisation of libraries and the running program
 * 0 : No trace. (Default.)
 * 1 : Trace enabled. The trace start with the initialisation of all depending libraries is done.

#### BOX64_TRACE_START *
Only on builds with trace enabled.
 * NNNNNNN : Start trace only after NNNNNNNN opcode execute (number is an `uint64_t`).

#### BOX64_TRACE_XMM *
Only on builds with trace enabled.
 * 0 : The XMM (i.e. SSE/SSE2) register will not be logged with the general and x86 registers. (Default.)
 * 1 : Dump the XMM registers.

#### BOX64_TRACE_EMM *
Only on builds with trace enabled.
 * 0 : The EMM (i.e. MMX) register will not be logged with the general and x86 registers. (Default.)
 * 1 : Dump the EMM registers.

#### BOX64_TRACE_COLOR *
Only on builds with trace enabled.
 * 0 : The general registers will always be the default white color. (Default.)
 * 1 : The general registers will change color in the dumps when they changed value.

#### BOX64_LOAD_ADDR *
Try to load at 0xXXXXXX main binary (if binary is a PIE)
 * 0xXXXXXXXX : The load address . (Only active on PIE programs.)

#### BOX64_NOSIGSEGV *
Disable handling of SigSEGV. (Very useful for debugging.)
 * 0 : Let the x86 program set sighandler for SEGV (Default.)
 * 1 : Disable the handling of SigSEGV.

#### BOX64_NOSIGILL *
Disable handling of SigILL (to ease debugging mainly).
 * 0 : Let x86 program set sighandler for Illegal Instruction
 * 1 : Disables the handling of SigILL 

#### BOX64_SHOWSEGV *
Show Segfault signal even if a signal handler is present
 * 0 : Don"t force show the SIGSEGV analysis (Default.)
 * 1 : Show SIGSEGV detail, even if a signal handler is present

#### BOX64_SHOWBT *
Show some Backtrace (Nativ e and Emulated) whgen a signal (SEGV, ILL or BUS) is caught
 * 0 : Don"t show backtraces (Default.)
 * 1 : Show Backtrace detail (for native, box64 is rename as the x86_64 binary run)

#### BOX64_X11THREADS *
Call XInitThreads when loading X11. (This is mostly for old Loki games with the Loki_Compat library.)
 * 0 : Don't force call XInitThreads. (Default.)
 * 1 : Call XInitThreads as soon as libX11 is loaded.

#### BOX64_MMAP32 *
Will use 32bits address in priority for external MMAP (when 32bits process are detected)
 * 0 : Use regular mmap (default, except for Snapdragron build)
 * 1 : Use 32bits address space mmap in priority for external mmap as soon a 32bits process are detected (default for SnapDragon and TegraX1 build)

#### BOX64_IGNOREINT3 *
What to do when a CC INT3 opcode is encounter in the code being run
 * 0 : Trigger a TRAP signal if a handler is present
 * 1 : Just skip silently the opcode

#### BOX64_X11GLX *
Force libX11's GLX extension to be present.
* 0 : Do not force libX11's GLX extension to be present. 
* 1 : GLX will always be present when using XQueryExtension. (Default.)

#### BOX64_DYNAREC_DUMP *
Enables/disables Box64's Dynarec's dump.
 * 0 : Disable Dynarec's blocks dump. (Default.)
 * 1 : Enable Dynarec's blocks dump.
 * 2 : Enable Dynarec's blocks dump with some colors.

#### BOX64_DYNAREC_LOG *
Set the level of DynaRec's logs.
 * 0 : NONE : No Logs for DynaRec. (Default.)
 * 1 :INFO : Minimum Dynarec Logs (only unimplemented OpCode).
 * 2 : DEBUG : Debug Logs for Dynarec (with details on block created / executed).
 * 3 : VERBOSE : All of the above plus more.

#### BOX64_DYNAREC *
Enables/Disables Box64's Dynarec.
 * 0 : Disables Dynarec.
 * 1 : Enable Dynarec. (Default.)

#### BOX64_DYNAREC_TRACE *
Enables/Disables trace for generated code.
 * 0 : Disable trace for generated code. (Default.)
 * 1 : Enable trace for generated code (like regular Trace, this will slow down the program a lot and generate huge logs).

#### BOX64_NODYNAREC  *
Forbid dynablock creation in the interval specified (helpfull for debugging behaviour difference between Dynarec and Interpreter)
 * 0xXXXXXXXX-0xYYYYYYYY : define the interval where dynablock cannot start (inclusive-exclusive)

#### BOX64_DYNAREC_TEST *
Dynarec will compare it's execution with the interpreter (super slow, only for testing)
 * 0 : No comparison. (Default.)
 * 1 : Each opcode runs on interepter and on Dynarec, and regs and memory are compared and print if different.
 * 2 : Thread-safe tests, extremely slow.

#### BOX64_DYNAREC_BIGBLOCK *
Enables/Disables Box64's Dynarec building BigBlock.
 * 0 : Don't try to build block as big as possible (can help program using lots of thread and a JIT, like C#/Unity) (Default when libmonobdwgc-2.0.so is loaded)
 * 1 : Build Dynarec block as big as possible (Default.)
 * 2 : Build Dynarec block bigger (don't stop when block overlaps, but only for blocks in elf memory)
 * 3 : Build Dynarec block bigger (don't stop when block overlaps, for all type of memory)

#### BOX64_DYNAREC_FORWARD *
Define Box64's Dynarec max allowed forward value when building Block.
 * 0 : No forward value. When current block end, don't try to go further even if there are previous forward jumps
 * XXX : Allow up to XXXX bytes of gap when building a Block after the block end to next forward jump (Default: 128)
 
#### BOX64_DYNAREC_STRONGMEM *
Enable/Disable simulation of Strong Memory model
* 0 : Don't try anything special (Default.)
* 1 : Enable some Memory Barrier when writting to memory (on some MOV opcode) to simulate Strong Memory Model while trying to limit performance impact (Default when libmonobdwgc-2.0.so is loaded)
* 2 : All 1. plus a memory barrier on every write to memory using MOV
* 3 : All 2. plus Memory Barrier when reading from memory and on some SSE/SSE2 opcodes too

#### BOX64_DYNAREC_X87DOUBLE *
Force the use of Double for x87 emulation
* 0 : Try to use float when possible for x87 emulation (default, faster)
* 1 : Only use Double for x87 emulation (slower, may be needed for some specific games, like Crysis)

#### BOX64_DYNAREC_FASTNAN *
Enable/Disable generation of -NAN
* 0 : Generate -NAN like on x86
* 1 : Don't do anything special with NAN, to go as fast as possible (default, faster)

#### BOX64_DYNAREC_FASTROUND *
Enable/Disable generation of precise x86 rounding
* 0 : Generate float/double -> int rounding like on x86
* 1 : Don't do anything special with edge case Rounding, to go as fast as possible (no INF/NAN/Overflow -> MIN_INT conversion) (default, faster)

#### BOX64_DYNAREC_SAFEFLAGS *
Handling of flags on CALL/RET opcodes
* 0 : Treat CALL/RET as if it never needs any flags (faster but not advised)
* 1 : most of RET will need flags, most of CALLS will not (Default)
* 2 : All CALL/RET will need flags (slower, but might be needed. Automatically enabled for Vara.exe)

#### BOX64_DYNAREC_CALLRET *
Optimisation of CALL/RET opcodes (not compatible with jit/dynarec/smc)
* 0 : Don't optimize CALL/RET, use Jump Table for boths (Default)
* 1 : Try to optimized CALL/RET, skipping the use of the JumpTable when possible

#### BOX64_DYNAREC_ALIGNED_ATOMICS *
Generated code for aligned atomics only
* 0 : The code generated can handle unaligned atomics (Default)
* 1 : Generated code only for aligned atomics (faster and less code generated, but will SEGBUS if LOCK prefix is unsed on unaligned data)

#### BOX64_DYNAREC_BLEEDING_EDGE *
Detect MonoBleedingEdge and apply conservative settings
* 0 : Don't detect MonoBleedingEdge
* 1 : Detect MonoBleedingEdge, and apply BIGBLOCK=0 STRONGMEM=1 if detected (Default)

#### BOX64_DYNAREC_JVM *
Detect libjvm and apply conservative settings. Obsolete, use BOX64_JVM instead.
* 0 : Don't detect libjvm
* 1 : Detect libjvm, and apply BIGBLOCK=0 STRONGMEM=1 SSE42=0 if detected (Default)

#### BOX64_DYNAREC_WAIT *
Behavior with FillBlock is not availble (FillBlock build Dynarec blocks and is not multithreaded)
* 0 : Dynarec will not wait for FillBlock to ready and use Interpreter instead (might speedup a bit massive multithread or JIT programs)
* 1 : Dynarec will wait for FillBlock to be ready (Default)

#### BOX64_DYNAREC_MISSING *
Dynarec print the missing opcodes
* 0 : not print the missing opcode (Default, unless DYNAREC_LOG>=1 or DYNAREC_DUMP>=1 is used)
* 1 : Will print the missing opcodes

#### BOX64_SSE_FLUSHTO0 *
Handling of SSE Flush to 0 flags
* 0 : Just track the flag (Default)
* 1 : Direct apply of SSE Flush to 0 flag

#### BOX64_X87_NO80BITS *
Handling of x87 80bits long double
* 0 : Try to handle 80bits long double as precise as possible (Default)
* 1 : Handle them as double

#### BOX64_MAXCPU
Maximum CPU Core exposed
* 0 : Don't cap the number of cpu core exposed (Default)
* XXX : Cap the maximum CPU Core exposed to XXX (usefull with wine64 or GridAutosport for example)

#### BOX64_SYNC_ROUNDING *
Box64 will sync rounding mode with fesetround/fegetround.
* 0 : Disable rounding mode syncing. (Default.)
* 1 : Enable rounding mode syncing.

#### BOX64_LIBCEF *
Detect libcef and apply malloc_hack settings
* 0 : Don't detect libcef
* 1 : Detect libcef, and apply MALLOC_HACK=2 if detected (Default)

#### BOX64_JVM *
Detect libjvm and apply conservative settings
* 0 : Don't detect libjvm
* 1 : Detect libjvm, and apply BIGBLOCK=0 STRONGMEM=1 SSE42=0 if detected (Default)

#### BOX64_SDL2_JGUID *
Need a workaround for SDL_GetJoystickGUIDInfo function for wrapped SDL2
* 0 : Don't use any workaround
* 1 : Use a workaround for program that use the private SDL_GetJoystickGUIDInfo function with 1 missing argument

#### BOX64_LIBGL *
 * libXXXX set the name for libGL (defaults to libGL.so.1).
 * /PATH/TO/libGLXXX : Sets the name and path for libGL
 You can also use SDL_VIDEO_GL_DRIVER

#### BOX64_LD_PRELOAD
 * XXXX[:YYYYY] force loading XXXX (and YYYY...) libraries with the binary
 PreLoaded libs can be emulated or native, and are treated the same way as if they were comming from the binary
 
#### BOX64_EMULATED_LIBS *
 * XXXX[:YYYYY] force lib XXXX (and YYYY...) to be emulated (and not wrapped)
Some games uses an old version of some libraries, with an ABI incompatible with native version.
Note that LittleInferno for example is auto detected, and libvorbis.so.0 is automatical added to emulated libs, and same for Don't Starve (and Together / Server variant) that use an old SDL2 too

#### BOX64_ALLOWMISSINGLIBS *
Allow Box64 to continue even if a library is missing.
 * 0 : Box64 will stop if a library cannot be loaded. (Default.)
 * 1 : Continue even if a needed library cannot be loaded. Unadvised, this will, in most cases, crash later on.

#### BOX64_PREFER_WRAPPED *
Box64 will use wrapped libs even if the lib is specified with absolute path
 * 0 : Try to use emulated libs when they are defined with absolute path  (Default.)
 * 1 : Use Wrapped native libs even if path is absolute

#### BOX64_PREFER_EMULATED *
Box64 will prefer emulated libs first (execpt for glibc, alsa, pulse, GL, vulkan and X11
 * 0 : Native libs are prefered (Default.)
 * 1 : Emulated libs are prefered (Default for program running inside pressure-vessel)

#### BOX64_CRASHHANDLER *
Box64 will use a dummy crashhandler.so library
 * 0 : Use Emulated crashhandler.so library if needed
 * 1 : Use an internal dummy (completly empty) crashhandler.so library (defaut)

#### BOX64_MALLOC_HACK *
How Box64 will handle hooking of malloc operators
 * 0 : Don't allow malloc operator to be redirected, rewriting code to use regular function (Default)
 * 1 : Allow malloc operator to be redirected (not advised)
 * 2 : Like 0, but track special mmap / free (some redirected functions were inlined and cannot be redirected)

#### BOX64_NOPULSE *
Disables the load of pulseaudio libraries.
 * 0 : Load pulseaudio libraries if found. (Default.)
 * 1 : Disables the load of pulse audio libraries (libpulse and libpulse-simple), both the native library and the x86 library

#### BOX64_NOGTK *
Disables the loading of wrapped GTK libraries.
 * 0 : Load wrapped GTK libraries if found. (Default.)
 * 1 : Disables loading wrapped GTK libraries.

#### BOX64_NOVULKAN *
Disables the load of vulkan libraries.
 * 0 : Load vulkan libraries if found.
 * 1 : Disables the load of vulkan libraries, both the native and the i386 version (can be useful on Pi4, where the vulkan driver is not quite there yet.)

#### BOX64_SSE42 *
Expose or not SSE 4.2 capabilites
 * 0 : Do not expose SSE 4.2 capabilites (default when libjvm is detected)
 * 1 : Expose SSE 4.2 capabilites (Default.)

#### BOX64_FUTEX_WAITV *
Use of the new fuext_waitc syscall
 * 0 : Do not try to use it, return unsupported (Default for BAD_SIGNAL build)
 * 1 : let program use the syscall if the host system support it (Default for other build)

#### BOX64_BASH *
Define x86_64 bash to launch script
 * yyyy
 Will use yyyy as x86_64 bash to launch script. yyyy needs to be a full path to a valid x86_64 version of bash

#### BOX64_ENV *
 * XXX=yyyy
 will add XXX=yyyy env. var.

#### BOX64_ENV1 *
 * XXX=yyyy
 will add XXX=yyyy env. var. and continue with BOX86_ENV2 ... until var doesn't exist

#### BOX64_RESERVE_HIGH
* 0 : Don't try to pe-reserve high memory (beyond 47bits) (Default)
* 1 : Try to reserve (without allocating it) memory beyond 47bits (seems unstable)

#### BOX64_JITGDB *
 * 0 : Just print the Segfault message on segfault (default)
 * 1 : Launch `gdb` when a segfault, bus error or illegal instruction signal is trapped, attached to the offending process and go in an endless loop, waiting.
 When in gdb, you need to find the correct thread yourself (the one with `my_box64signalhandler` in is stack)
 then probably need to `finish` 1 or 2 functions (inside `usleep(..)`) and then you'll be in `my_box64signalhandler`, 
 just before the printf of the Segfault message. Then simply 
 `set waiting=0` to exit the infinite loop.
 * 2 : Launch `gdbserver` when a segfault, bus error or illegal instruction signal is trapped, attached to the offending process, and go in an endless loop, waiting.
 Use `gdb /PATH/TO/box64` and then `target remote 127.0.0.1:1234` to connect to the gdbserver (or use actual IP if not on the machine). After that, the procedure is the same as with ` BOX64_JITGDB=1`.
 This mode can be usefullwhen programs redirect all console output to a file (like Unity3D Games)
 * 3 : Launch `lldb` when a segfault, bus error or illegal instruction signal is trapped, attached to the offending process and go in an endless loop, waiting.

#### BOX64_NORCFILES
If the env var exist, no rc files (like /etc/box64.box64rc and ~/.box64rc) will be loaded

#### BOX64_RCFILE
If the env var is set and file exists, this variable will be used as path to the box64rc file instead of default paths (BOX64_RCFILE is loaded first, default paths are not loaded)

----

Those variables are only valid inside a rcfile:
----

#### BOX64_NOSANDBOX
 * 0 : Nothing special
 * 1 : Added "--no-sandbox" to command line arguments (usefull for chrome based programs)

#### BOX64_INPROCESSGPU
 * 0 : Nothing special
 * 1 : Added "--in-process-gpu" to command line arguments (usefull for chrome based programs)

#### BOX64_CEFDISABLEGPU
 * 0 : Nothing special
 * 1 : Added "-cef-disable-gpu" to command line arguments (usefull for steamwebhelper/cef based programs)

#### BOX64_CEFDISABLEGPUCOMPOSITOR
 * 0 : Nothing special
 * 1 : Added "-cef-disable-gpu-compositor" to command line arguments (usefull for steamwebhelper/cef based programs)

#### BOX64_EXIT
 * 0 : Nothing special
 * 1 : Just exit, don't try to run the program
