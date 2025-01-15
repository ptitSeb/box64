
Usage
----

There are multiple environment variables to control Box64 behaviour.

### Configuration files

Env. vars below marked with * can also be put inside box64rc file, with priority over command line environment.
Box64 looks for 2 places for rcfile: `/etc/box64.box64rc` and `~/.box64rc`.
The second takes precedence over the first per each application (when `[MYAPP]` appears in both files, only the settings in `~/.box64rc` will be applied).
Asterisks may be used to denote a substring in the application name, like `[*setup*]` that will be applied to every program containing "setup" in its name. Note that this is not a complete set of regex rules.
Settings priority: `~/.box64rc` > `/etc/box64.box64rc` > Command line.

----

### Debugging

#### BOX64_LOG *

Set the level of box64 logs.
 * 0: NONE : No messages (except for a few fatal errors). (default)
 * 1: INFO : Minimum log (Example: missing libraries)
 * 2: DEBUG : Verbose log (Example: relocations or functions called).
 * 3: DUMP : All DEBUG plus DUMP of all ELF Info.

#### BOX64_ROLLING_LOG *

Show last few wrapped function call when a Signal is caught.
 * 0: No last function call printed (default)
 * 1: Print last 16 wrapped functions calls when a signal is caught. Incompatible with BOX64_LOG>1 (may need BOX64_SHOWSEGV=1)
 * N: Print last N wrapped functions calls when a signal is caught. Incompatible with BOX64_LOG>1 (may need BOX64_SHOWSEGV=1)

#### BOX64_NOBANNER

Disables printing box64 version and build.
 * 0 : Print the banner. (default)
 * 1 : Do not print the banner. 

#### BOX64_DLSYM_ERROR *

Enable logging of `dlsym` errors.
 * 0 : Do not print `dlsym` errors. (default)
 * 1 : Print dlsym errors.

#### BOX64_TRACE_FILE *

Send all log and trace to a file instead of `stdout`.
File name syntax:
 * `%pid` in the name is replaced with the actual PID of box64 instance.
 * Use `+` in the end of the name to append instead of overwriting.
 * `stderr` can be used instead of default `stdout` 

#### BOX64_SHOWSEGV *

Always show Segfault signal.
 * 0 : Do not force show the SIGSEGV if a signal handler is present. (default)
 * 1 : Show SIGSEGV detail, even if a signal handler is present.

#### BOX64_SHOWBT *

Show some backtrace (native and emulated) when a signal (SEGV, ILL or BUS) is caught.
 * 0 : Do not show backtraces. (default)
 * 1 : Show Backtrace details (for native, box64 is renamed as the x86_64 binary runs).

#### BOX64_NOSIGSEGV *

Disable handling of SigSEGV for debugging.
 * 0 : Let the x86 program set sighandler for SEGV. (default)
 * 1 : Disable the handling of SigSEGV.

#### BOX64_NOSIGILL *

Disable handling of SigILL for debugging.
 * 0 : Let x86 program set sighandler for Illegal Instruction.
 * 1 : Disable the handling of SigILL.

#### BOX64_LOAD_ADDR *

Try to load the binary at 0xXXXXXX. (if the binary is a PIE)
 * 0xXXXXXXXX : The load address . (Only active on PIE programs.)

#### BOX64_JITGDB *

 * 0 : Just print the Segfault message when the signal is caught. (default)
 * 1 : Launch `gdb` when a segfault, bus error or illegal instruction signal is trapped, attached to the offending process and go in an endless loop, waiting.
 When in gdb, you need to find the correct thread yourself (the one with `my_box64signalhandler` in is stack)
 then probably need to `finish` 1 or 2 functions (inside `usleep(..)`) and then you'll be in `my_box64signalhandler`, 
 just before the printf of the Segfault message. Then simply 
 `set waiting=0` to exit the infinite loop.
 * 2 : Launch `gdbserver` when a segfault, bus error or illegal instruction signal is trapped, attached to the offending process, and go in an endless loop, waiting.
 Use `gdb /PATH/TO/box64` and then `target remote 127.0.0.1:1234` to connect to the gdbserver (or use actual IP if not on the machine). After that, the procedure is the same as with ` BOX64_JITGDB=1`.
 This mode can be usefull when programs redirect all console output to a file (like Unity3D Games)
 * 3 : Launch `lldb` when a segfault, bus error or illegal instruction signal is trapped, attached to the offending process and go in an endless loop, waiting.

#### BOX64_TRACE *

Only available on box64 build with trace. Adds trace of all instructions executed, along with a register dump.
 * 0 : No trace. (default) 
 * 1 : Enable trace. Trace starts after all dependencies are initialized.
 * symbolname : Trace only `symbolname` (if the symbol was found).
 * 0xXXXXXXX-0xYYYYYYY : Trace only between the 2 addresses.

#### BOX64_TRACE_INIT *

Same as BOX64_TRACE but starts the trace immediately.
 * 0 : No trace. (default)
 * 1 : Trace enabled. The trace starts before the initialization of dependencies.

#### BOX64_TRACE_START *

Only available on box64 build with trace.
 * NNNNNNN : Start trace only after NNNNNNNN opcode execute (number is an `uint64_t`).

#### BOX64_TRACE_XMM *

Only available on box64 build with trace.
 * 0 : The XMM (i.e. SSE/SSE2) register will not be logged with the general and x86 registers. (default)
 * 1 : Dump the XMM registers.

#### BOX64_TRACE_EMM *

Only available on box64 build with trace.
 * 0 : The EMM (i.e. MMX) register will not be logged with the general and x86 registers. (default)
 * 1 : Dump the EMM registers.

#### BOX64_TRACE_COLOR *

Only available on box64 build with trace.
 * 0 : The general registers will always be the default white color. (default)
 * 1 : The general registers will change color in the dumps when they changed value.

#### BOX64_DYNAREC *

Enable Box64 DynaRec.
 * 0 : Disable Dynarec.
 * 1 : Enable Dynarec. (default)

#### BOX64_DYNAREC_LOG *

Set the level of DynaRec logs.
 * 0 : NONE : No logs. (default)
 * 1 : INFO : Minimal logs. (only unimplemented OpCodes).
 * 2 : DEBUG : Debug Logs for Dynarec (with details on block created / executed).
 * 3 : VERBOSE : All of the above plus more.

#### BOX64_DYNAREC_TRACE *

Enable the trace for generated code.
 * 0 : Disable trace for generated code. (default)
 * 1 : Enable trace for generated code (like regular trace, slows down the program a lot and generates huge logs).

#### BOX64_DYNAREC_DUMP *

Enable Box64 DynaRec dump.
 * 0 : Do not dump DynaRec blocks. (default)
 * 1 : Enable DynaRec blocks dump.
 * 2 : Enable DynaRec blocks dump with some colors.

#### BOX64_DYNAREC_MISSING *

Print missing opcodes.
 * 0 : Do not print the missing opcode. (default, unless DYNAREC_LOG>=1 or DYNAREC_DUMP>=1 is used)
 * 1 : Print missing opcodes.
 * 2 : Print the fallback to scalar opcodes. (only valid on RISC-V)

#### BOX64_NODYNAREC *

Forbid dynablock creation in the interval specified. (helpful for debugging behaviour difference between Dynarec and Interpreter)
 * 0xXXXXXXXX-0xYYYYYYYY : the interval where dynablock cannot start. (inclusive-exclusive)

#### BOX64_DYNAREC_TEST *

Enable DynaRec execution comparison with the interpreter. (super slow, only for testing)
 * 0 : No comparison. (default)
 * 1 : Each opcode runs on interpreter and on Dynarec, regs and memory are compared and printed when they differ.
 * 0xXXXXXXXX-0xYYYYYYYY : define the interval where dynarec is tested (inclusive-exclusive)

#### BOX64_DYNAREC_GDBJIT *

The GDBJIT debugging support, only available on build with `-DGDBJIT=ON`, enable it with gdb command: jit-reader-load /usr/local/lib/libbox64gdbjitreader.so.
 * 0 : Dynarec will not generate GDBJIT debuginfo. (default)
 * 1 : Dynarec will generate GDBJIT debuginfo.
 * 2 : Dynarec will generate detailed GDBJIT debuginfo with internal state.

#### BOX64_DYNAREC_PERFMAP *

Generate map file for Linux perf tool.
 * 0 : Dynarec will not generate perf map. (default)
 * 1 : Dynarec will generate perf map.

----

### Compatibility

#### BOX64_X11THREADS *

Call XInitThreads when loading X11. (This is mostly for old Loki games with the Loki_Compat library.)
 * 0 : Do not force call XInitThreads. (default)
 * 1 : Call XInitThreads as soon as libX11 is loaded.

#### BOX64_MMAP32 *

Use 32bit address in priority for external MMAP.
 * 0 : Use regular mmap (default, except for Snapdragron build)
 * 1 : Use 32bits address space mmap in priority for external mmap when 32bit process is detected (default for Snapdragon and TegraX1 build)

#### BOX64_IGNOREINT3 *

Controls behaviour when a CC INT3 opcode is encountered.
 * 0 : Trigger a TRAP signal if a handler is present. (default)
 * 1 : Just skip silently the opcode.

#### BOX64_X11GLX *

Force Xorg GLX extension to be present.
 * 0 : Do not force GLX extension to be present.
 * 1 : Require Xorg GLX extension when using XQueryExtension. (default)

#### BOX64_SSE_FLUSHTO0 *

Behaviour of SSE Flush to 0 flags.
 * 0 : Just track the flag. (default)
 * 1 : Apply SSE Flush to 0 flag directly.

#### BOX64_X87_NO80BITS *

Behavoiur of x87 80bits long double.
 * 0 : Try to handle 80bits long double as precise as possible. (default)
 * 1 : Handle them as double.

#### BOX64_MAXCPU

Maximum CPU Cores exposed.
 * 0 : Do not cap the number of cpu core exposed. (default)
 * XXX : Cap the maximum CPU Core exposed to XXX. (usefull with wine64 or GridAutosport for example)

#### BOX64_RDTSC_1GHZ

Use hardware counter for rdtsc if available.
* 0 : Use hardware counter for rdtsc opcode if available (default)
* 1 : Use hardware counter for rdtsc if available and only if precision is at least 1GHz

#### BOX64_SYNC_ROUNDING *

Sync rounding mode with fesetround/fegetround.
 * 0 : Disable rounding mode syncing. (default)
 * 1 : Enable rounding mode syncing.

#### BOX64_SDL2_JGUID *

Use a workaround for SDL_GetJoystickGUIDInfo function for wrapped SDL2.
 * 0 : Don't do anything special.
 * 1 : Use a workaround for program that use the private SDL_GetJoystickGUIDInfo function with 1 missing argument.

#### BOX64_MALLOC_HACK *

Behaviour when hooking malloc operators.
 * 0 : Don't allow malloc operator to be redirected, rewriting code to use regular function. (default)
 * 1 : Allow malloc operator to be redirected. (not advised)
 * 2 : Like 0, but track special mmap / free (some redirected functions are inlined and cannot be redirected).

#### BOX64_SHAEXT *

Expose SHAEXT (a.k.a. SHA_NI) capabilities.
 * 0 : Do not expose SHAEXT capabilities.
 * 1 : Expose SHAEXT capabilities. (default)

#### BOX64_SSE42 *

Expose SSE 4.2 capabilities.
 * 0 : Do not expose SSE 4.2 capabilities. (default when libjvm is detected)
 * 1 : Expose SSE 4.2 capabilities. (default)

#### BOX64_FUTEX_WAITV *

Expose futex_waitv syscall.
 * 0 : Report as unsupported. (default for BAD_SIGNAL build)
 * 1 : Let program use futex_waitv. (default)

#### BOX64_RESERVE_HIGH *

* 0 : Do not try to pre-reserve high memory (beyond 47bits) (default)
* 1 : Try to reserve (without allocating it) memory beyond 47bits (seems unstable)

#### BOX64_WRAP_EGL *

Prefer wrapped libs for EGL and GLESv2.
 * 0 : Emulated libs are preferred. (default)
 * 1 : Native libs are preferred.

#### BOX64_CRASHHANDLER *

Use a dummy crashhandler.so library.
 * 0 : Use emulated crashhandler.so library if needed.
 * 1 : Use an internal dummy (completely empty) crashhandler.so library. (default)

#### BOX64_NOPULSE *

Do not load pulseaudio libraries.
 * 0 : Load pulseaudio libraries if found. (default)
 * 1 : Disables the load of pulse audio libraries, both native and x86_64. (libpulse and libpulse-simple)

#### BOX64_NOGTK *

Do not load wrapped GTK libraries.
 * 0 : Load wrapped GTK libraries. (default)
 * 1 : Disable loading of wrapped GTK libraries.

#### BOX64_NOVULKAN *

Do not load vulkan libraries.
 * 0 : Load vulkan libraries if found. (default)
 * 1 : Disables loading of vulkan libraries, both native and x86_64. (can be useful on Pi4, where the vulkan driver is not quite there yet.)

#### BOX64_ARG0
Define argv[0] of the executable.
 * yyyy
Will rewrite argv[0] instead of using program path.

----

### DynaRec optimizations

#### BOX64_DYNAREC_BIGBLOCK *

Enable building bigger DynaRec blocks for performance.
 * 0 : Do not try to build block as big as possible (can help program using lots of threads and JIT, like C#/Unity) (default when libmonobdwgc-2.0.so is loaded)
 * 1 : Build Dynarec block as big as possible. (default)
 * 2 : Build Dynarec block bigger (do not stop when block overlaps, but only for blocks in elf memory)
 * 3 : Build Dynarec block bigger (do not stop when block overlaps, for all type of memory)

#### BOX64_DYNAREC_FORWARD *

Define max allowed forward value when building block.
 * 0 : No forward value. When current block ends, do not try to go further even if there are previous forward jumps
 * XXX : Allow up to XXXX bytes of gap between end of the block and the next forward jump (default: 128)
 
#### BOX64_DYNAREC_STRONGMEM *

Enable simulation of Strong Memory model.
 * 0 : Do not try anything special (default)
 * 1 : Enable some memory barriers when writing to memory to simulate the Strong Memory Model in a limited way (default when libmonobdwgc-2.0.so is loaded)
 * 2 : All 1. plus memory barriers on SIMD instructions
 * 3 : All 2. plus more memory barriers on a regular basis

#### BOX64_DYNAREC_WEAKBARRIER *

Tweak the memory barriers to reduce the performance impact by STRONGMEM.
 * 0 : Use regular safe barrier. (default)
 * 1 : Use weak barriers to have more performance boost.
 * 2 : All 1. Plus disabled the last write barriers.

#### BOX64_DYNAREC_PAUSE *

Enable x86 PAUSE emulation, that helps the performance of spinlocks.
 * 0 : Ignore x86 PAUSE instruction. (default)
 * 1 : Use YIELD to emulate x86 PAUSE instruction.
 * 2 : Use WFI to emulate x86 PAUSE instruction.
 * 3 : Use SEVL+WFE to emulate x86 PAUSE instruction.

#### BOX64_DYNAREC_X87DOUBLE *

Force the use of Double for x87 emulation.
 * 0 : Try to use float when possible for x87 emulation. (default, faster)
 * 1 : Only use Double for x87 emulation. (slower, may be needed for some specific games, like Crysis)

#### BOX64_DYNAREC_FASTNAN *

Generate x86 -NAN.
 * 0 : Generate -NAN like on x86.
 * 1 : Do not do anything special with NAN, to go as fast as possible. (default, faster)

#### BOX64_DYNAREC_FASTROUND *

Generate precise x86 rounding.
 * 0 : Generate float/double -> int rounding and use current rounding mode for float/double computation like on x86.
 * 1 : Do not do anything special with edge case Rounding, to go as fast as possible (no INF/NAN/Overflow -> MIN_INT conversion, and no non-default rounding modes). (default, faster)
 * 2 : Everything from 1 plus also fast round of double->float (not taking into account current rounding mode).

#### BOX64_DYNAREC_SAFEFLAGS *

Behaviour of flags on CALL/RET opcodes.
 * 0 : Treat CALL/RET as if it never needs any flags (faster but not advised)
 * 1 : most of RET will need flags, most of CALLS will not (default)
 * 2 : All CALL/RET will need flags (slower, but might be needed. Automatically enabled for Vara.exe)

#### BOX64_DYNAREC_CALLRET *

Optimize CALL/RET opcodes.
 * 0 : Do not optimize CALL/RET, use Jump Table. (default)
 * 1 : Try to optimize CALL/RET, skipping the JumpTable when possible.

#### BOX64_DYNAREC_ALIGNED_ATOMICS *

Generate code for aligned atomics only.
 * 0 : Add handling of unaligned atomics. (default)
 * 1 : Generate code for aligned atomics only (faster and less code is generated, but will SEGBUS if LOCK prefix is unused on unaligned data).

#### BOX64_DYNAREC_NATIVEFLAGS *

Use native ARM flags when possible.
 * 0 : Do not use native flags.
 * 1 : Use native flags when possible. (default)

#### BOX64_DYNAREC_WAIT *

Wait for FillBlock to be ready (FillBlock builds Dynarec blocks and is not multithreaded).
 * 0 : Do not wait for FillBlock to ready and use Interpreter instead (might speedup a bit massive multithread or JIT programs).
 * 1 : Wait for FillBlock to be ready. (default, mostly faster)

#### BOX64_DYNAREC_DIRTY *

Allow continue running a block that is unprotected and potentially dirty
 * 0 : Do not allow conitnuing to run a dynablock that gets unprotected (default)
 * 1 : Allow continue to run a dynablock that write data in the same page the code is. It can get faster loading time of some game but can also get unexpected crash.

### Detection

#### BOX64_DYNAREC_BLEEDING_EDGE *

Detect MonoBleedingEdge and apply conservative settings.
 * 0 : Do not detect MonoBleedingEdge.
 * 1 : Detect MonoBleedingEdge and apply BIGBLOCK=0 STRONGMEM=1 when detected. (default)

#### BOX64_DYNAREC_JVM *

Detect libjvm and apply conservative settings. Obsolete, use BOX64_JVM instead.
 * 0 : Do not detect libjvm.
 * 1 : Detect libjvm and apply BIGBLOCK=0 STRONGMEM=1 SSE42=0 when detected. (default)

#### BOX64_LIBCEF *

Detect libcef and apply malloc_hack settings.
 * 0 : Don't do anything special.
 * 1 : Detect libcef, and apply MALLOC_HACK=2 if detected. (default)

#### BOX64_JVM *

Detect libjvm and apply conservative settings.
 * 0 : Don't do anything special.
 * 1 : Detect libjvm, and apply BIGBLOCK=0 STRONGMEM=1 SSE42=0 if detected. (default)

#### BOX64_UNITYPLAYER *

Detect UnityPlayer.dll and apply strongmem settings.
 * 0 : Don't do anything special.
 * 1 : Detect UnityPlayer.dll, and apply BOX64_DYNAREC_STRONGMEM=1 if detected. (default)

----

### Paths and libraries

#### BOX64_LD_LIBRARY_PATH *

Path to look for x86_64 libraries. Default is current folder and `lib` in current folder.
Also, `/usr/lib/x86_64-linux-gnu`, `/lib/x86_64-linux-gnu` and `/usr/lib/box64-x86_64-linux-gnu` are added if they exist.

#### BOX64_LD_PRELOAD

 * XXXX[:YYYYY] force loading XXXX (and YYYY...) libraries with the binary
 PreLoaded libs can be emulated or native, and are treated the same way as if they were coming from the binary

#### BOX64_PATH *

Path to look for x86_64 executable. Default is current folder and `bin` in current folder.

#### BOX64_BASH *

Specify path to x86_64 bash to launch scripts.
 * yyyy
 Will use yyyy as x86_64 bash to launch script. yyyy needs to be a full path to a valid x86_64 version of bash

#### BOX64_LIBGL *

 * libXXXX set the name for libGL. (defaults to libGL.so.1).
 * /PATH/TO/libGLXXX : Sets the name and path for libGL.
 Has similar action to SDL_VIDEO_GL_DRIVER.
 
#### BOX64_EMULATED_LIBS *

 * XXXX[:YYYYY] force lib XXXX (and YYYY...) to be emulated (and not wrapped)
Some games use old versions of libraries, with ABI incompatible with native version.
Note that LittleInferno for example is auto detected, and libvorbis.so.0 is automatically added to emulated libs, and same for Don't Starve (and Together / Server variant) that use an old SDL2 too

#### BOX64_ALLOWMISSINGLIBS *

Allow Box64 to continue even if a library is missing.
 * 0 : Box64 will stop if a library cannot be loaded. (default)
 * 1 : Continue even if a needed library cannot be loaded. Unadvised, this will, in most cases, crash later on.

#### BOX64_PREFER_WRAPPED *

Prefer wrapped libs first even if the lib is specified with absolute path.
 * 0 : Try to use emulated libs when they are defined with absolute path. (default)
 * 1 : Use Wrapped native libs even if path is absolute.

#### BOX64_PREFER_EMULATED *

Prefer emulated libs first (except for glibc, alsa, pulse, GL, vulkan and X11).
 * 0 : Native libs are preferred. (default)
 * 1 : Emulated libs are preferred. (default for program running inside pressure-vessel)

----

### Rcfiles

#### BOX64_ENV, BOX64_ENV1, BOX64_ENV2, ... *

Add XXX=yyyy env. var.
 * XXX=yyyy

#### BOX64_NORCFILES

Disable loading rcfiles.

#### BOX64_RCFILE

Specify custom rcfile path instead of default paths. (BOX64_RCFILE is loaded first, default paths are not loaded)

----

### Valid inside rcfiles only

#### BOX64_NOSANDBOX

Add `--no-sandbox` to the command line arguments (useful for chrome based programs).

#### BOX64_INPROCESSGPU

Add `--in-process-gpu` to the command line arguments (useful for chrome based programs).

#### BOX64_CEFDISABLEGPU

Add `-cef-disable-gpu` to the command line arguments (useful for steamwebhelper/cef based programs).

#### BOX64_CEFDISABLEGPUCOMPOSITOR

Add `-cef-disable-gpu-compositor` to the command line arguments (useful for steamwebhelper/cef based programs).

#### BOX64_ARGS

Append arguments to the command line if there is no current argument. Note that "" are supported, but not ''.

#### BOX64_INSERT_ARGS

Prepend arguments to the command line. Note that "" are supported, but not ''.

#### BOX64_EXIT

Just exit, do not try to run the program.
