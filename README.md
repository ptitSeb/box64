# box64

![Official logo](Box64Logo.png "Official Logo")
Linux Userspace x86_64 Emulator with a twist

----

Box64 lets you run x86_64 Linux programs (such as games) on non-x86_64 Linux systems, like ARM (host system needs to be 64bit little-endian).

You can find many more Box64 video on Youtube in the [MicroLinux](https://www.youtube.com/channel/UCwFQAEj1lp3out4n7BeBatQ), [Pi Labs](https://www.youtube.com/channel/UCgfQjdc5RceRlTGfuthBs7g) or [The Byteman](https://www.youtube.com/channel/UCEr8lpIJ3B5Ctc5BvcOHSnA) Channels

Because Box64 uses the native versions of some "system" libraries, like libc, libm, SDL, and OpenGL, it's easy to integrate and use with most applications, and performance can be surprisingly high in many cases. Take a look at thoses bench analysis for an example [here](https://box86.org/index.php/2021/06/game-performances/).

Box64 integrates a DynaRec (dynamic recompiler) for the ARM64 platform, providing a speed boost between 5 to 10 times faster than only using the interpreter. Some high level information on how the Dynarec work can be found [here](https://box86.org/2021/07/inner-workings-a-high%e2%80%91level-view-of-box86-and-a-low%e2%80%91level-view-of-the-dynarec/).

Some x64 internal opcodes use parts of "Realmode X86 Emulator Library", see [x64primop.c](src/emu/x64primop.c) for copyright details

<img src="Box64Icon.png" width="96" height="96">

Logo and Icon made by @grayduck, thanks!

----

Usage
----

There are a few environment variables to control the behaviour of Box64.

See [here](USAGE.md) for all environment variables and what they do.

Note: Box64's Dynarec uses a mechanism with Memory Protection and a SegFault signal handler to handle JIT code. In simpler terms, if you want to use GDB to debug a running program that use JIT'd code (like mono/Unity3D), you will still have many "normal" segfaults triggering. It is suggested to use something like `handle SIGSEGV nostop` in GDB to not stop at each segfault, and maybe put a breakpoint inside `my_memprotectionhandler` in `signals.c` if you want to trap SegFaults.

----

Compiling/Installation
----
> Compilation instructions can be found [here](COMPILE.md)  

----

Version history
----

The change log is available [here](CHANGELOG.md)

----

Final words
----

(If you use Box64 in your project, please don't forget to mention Box64!)
