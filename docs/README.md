# box64

![Official logo](img/Box64Logo.png "Official Logo")
Linux Userspace x86_64 Emulator with a twist

----

Box64 lets you run x86_64 Linux programs (such as games) on non-x86_64 Linux systems, like ARM (host system needs to be 64bit little-endian).

You can find many more Box64 video on Youtube in the [MicroLinux](https://www.youtube.com/channel/UCwFQAEj1lp3out4n7BeBatQ), [Pi Labs](https://www.youtube.com/channel/UCgfQjdc5RceRlTGfuthBs7g) or [The Byteman](https://www.youtube.com/channel/UCEr8lpIJ3B5Ctc5BvcOHSnA) Channels

Because Box64 uses the native versions of some "system" libraries, like libc, libm, SDL, and OpenGL, it's easy to integrate and use with most applications, and performance can be surprisingly high in many cases. Take a look at thoses bench analysis for an example [here](https://box86.org/index.php/2021/06/game-performances/).

Box64 integrates a DynaRec (dynamic recompiler) for the ARM64 platform, providing a speed boost between 5 to 10 times faster than only using the interpreter. Some high level information on how the Dynarec work can be found [here](https://box86.org/2021/07/inner-workings-a-high%e2%80%91level-view-of-box86-and-a-low%e2%80%91level-view-of-the-dynarec/).

Some x64 internal opcodes use parts of "Realmode X86 Emulator Library", see [x64primop.c](../src/emu/x64primop.c) for copyright details

<img src="img/Box64Icon.png" width="96" height="96">

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

Notes about 32-bit platforms
----

Because Box64 works by directly translating function calls from x86_64 to host system, the host system (the one Box64 is running on) needs to have 64-bits libraries. Box64 doesn't include any 64-bits <-> 32-bits translation. 

So understand that box64 will only runs 64bits linux binaries. For 32bits binaries, you need box86 (with all the multiarch or proot trickery it imply on 64bits OS).
Note that many installer (mojo setup based) will fall back to "x86" when detecting ARM64 OS, and so will try to use box86 for the setup, even if an x86_64 version exist. You can hack your way around with a fake "uname" that return "x86_64" when argument is "-m"

----

Notes about Unity game emulation
----

Running Unity games is a hit or miss for now. Unity uses Mono (which uses signals that are not always emulated in a 100% accurate way).
You should also note that some Unity3D games require OpenGL 3+ which can be tricky to provide on ARM SBC (single-board computers) for now.
So, not all Unity games work and can require a high OpenGL profile, but many actualy works now
Hint: on Pi4, use `MESA_GL_VERSION_OVERRIDE=3.2` and with Panfrost use `PAN_MESA_DEBUG=gl3` to use higher profile if the game start then quit before showing anything.

----

Notes about GTK programs
----

GTK libraries are not wrapped yet on box64 (contrary to box86). Many games use gtk (setup too), like Unity game for the Screen Selection dialog. While Unity game can run without that dialog (using default setup), many just can't. For now, you'll need o put the set of gtk x86_64 libraries in the game folder (or some other folder known by box64) to get gtk support. GTK Wrapping is planned on box64.

----

Notes about Steam
----

Not that Steam is an hybrid 32buts / 64bits. You NEED box86 to run steam, as the client app is a 32bits binaries. It also use a 64bits local server binaries, but like most stuff using libsef/chromium. it's not working correctly on box64 for now.
So: no Steam for now on box64.

----

Notes about Wine
----

Wine 64bits is supported on box64. Be aware that 64bits wine also include 32bits componant, to be able to run 32bits windws programs. The 32bits will need box86 and will not run without it. On a system where both box64 and box86 are present and working, a wine 64bits setup can run both 32bits and 64bits windows program (just use `wine` and `wine64` respectivelly)

----

Notes about Vulkan
----

Box64 Does not wrap Vulkan libraries yet. It's planned, but for now, vulkan program will not run on box64.

----

Final word
----

I want to thank everyone who has contributed to box64 developpement.
There are many ways to contribute: code contribution, financial, hardware and advertisement!
So, in no particular order, I want to thank:
 * For their major code contribution: rajdakin
 * For their major financial contribution: FlyingFathead, stormchaser3000
 * For their hardware contribution: [Radxa](https://rockpi.org/), [Pine64](https://www.pine64.org/)
 * For their continous advertisement of box64 project: salva ([microLinux](https://www.youtube.com/channel/UCwFQAEj1lp3out4n7BeBatQ)), [PILab](https://www.youtube.com/channel/UCgfQjdc5RceRlTGfuthBs7g)/[TwisterOS](https://twisteros.com/) team, [The Byteman](https://www.youtube.com/channel/UCEr8lpIJ3B5Ctc5BvcOHSnA), [NicoD](https://www.youtube.com/channel/UCpv7NFr0-9AB5xoklh3Snhg), ekianjo ([Boilingsteam](https://boilingsteam.com/))

And I also thank the many other people who participated even once in the project.

(If you use Box64 in your project, please don't forget to mention it!)
