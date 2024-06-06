![Official logo](docs/img/Box64Logo.png "Official Logo")

Linux Userspace x86_64 Emulator with a twist

[View changelog](https://github.com/ptitSeb/box64/blob/main/docs/CHANGELOG.md) | [中文](https://github.com/ptitSeb/box64/blob/main/README_CN.md) | [Українська](https://github.com/ptitSeb/box64/blob/main/README_UK.md) | [Report an error](https://github.com/ptitSeb/box64/issues/new)

![build](https://app.travis-ci.com/ptitSeb/box64.svg?branch=main) ![stars](https://img.shields.io/github/stars/ptitSeb/box64) ![forks](https://img.shields.io/github/forks/ptitSeb/box64) ![contributors](https://img.shields.io/github/contributors/ptitSeb/box64) ![prs](https://img.shields.io/github/issues-pr/ptitSeb/box64) ![issues](https://img.shields.io/github/issues/ptitSeb/box64)

----

Box64 lets you run x86_64 Linux programs (such as games) on non-x86_64 Linux systems, like ARM (host system needs to be 64-bit little-endian).

You can find many Box64 videos on the [MicroLinux](https://www.youtube.com/channel/UCwFQAEj1lp3out4n7BeBatQ), [Pi Labs](https://www.youtube.com/channel/UCgfQjdc5RceRlTGfuthBs7g) or [The Byteman](https://www.youtube.com/channel/UCEr8lpIJ3B5Ctc5BvcOHSnA) YouTube channels.

Since Box64 uses the native versions of some "system" libraries, like libc, libm, SDL, and OpenGL, it's easy to integrate and use with most applications, and performance can be surprisingly high in many cases. Take a look at thoses bench analysis for an example [here](https://box86.org/index.php/2021/06/game-performances/).

Box64 integrates with DynaRec (dynamic recompiler) for the ARM64 and RV64 platform, providing a speed boost between 5 to 10 times faster than using only the interpreter. Some high level information on how DynaRec works can be found [here](https://box86.org/2021/07/inner-workings-a-high%e2%80%91level-view-of-box86-and-a-low%e2%80%91level-view-of-the-dynarec/).

Some x64 internal opcodes use parts of "Realmode X86 Emulator Library", see [x64primop.c](src/emu/x64primop.c) for copyright details

<img src="docs/img/Box64Icon.png" width="96" height="96">

Logo and Icon made by @grayduck, thanks!

----

Usage
----

There are a few environment variables to control the behaviour of Box64.

See [here](docs/USAGE.md) for all the environment variables and what they do.

Note: Box64's Dynarec uses a mechanism with Memory Protection and a SegFault signal handler to handle JIT code. In simpler terms, if you want to use GDB to debug a running program that use JIT'd code (like mono/Unity3D), you will still have many "normal" segfaults triggering. It is suggested to use something like `handle SIGSEGV nostop` in GDB to not stop at each segfault, and maybe put a breakpoint inside `my_memprotectionhandler` in `signals.c` if you want to trap SegFaults.

----

Compiling/Installation
----
> Compilation instructions can be found [here](https://github.com/ptitSeb/box64/blob/main/docs/COMPILE.md) \
> Instructions for installing Wine for Box64 can be found [here](https://github.com/ptitSeb/box64/blob/main/docs/X64WINE.md)

----

Version history/Change log
----

The change log is available [here](docs/CHANGELOG.md).

----

Notes about 32-bit platforms
----

Because Box64 works by directly translating function calls from x86_64 to host system, the host system (the one Box64 is running on) needs to have 64-bit libraries. Box64 doesn't include any 64-bit <-> 32-bit translation. 

So understand that box64 will only run 64-bit linux binaries. For 32-bit binaries, you need box86 (with all the multiarch or proot trickery it implies on a 64-bit OS).
Note that many installer (mojo setup based) will fall back to "x86" when detecting ARM64 OS, and so will try to use box86 for the setup, even if an x86_64 version exist. You can hack your way around with a fake "uname" that return "x86_64" when the argument is "-m"

An alternative solution is to use Wine with new WoW64, but you will be limited to 32bits windows apps and games.

----

Notes about Box64 configuration
----

Box64 now have configurations files. There are 2 files loaded. `/etc/box64.box64rc` and `~/.box64rc`. Both files have the same syntax, and is basicaly an ini files. Section in square brakets define the process name, and the rest is the env. var. to set. Looke at [Usage](USAGE.md) for detail on what parameters can be put. Box64 comes with a default file that should be installed for better stability. The file in in `system/box64.box64rc` and should be installed to `/etc/box64.box64rc` If, for some reason, you don't want to install that file here, at least copy it to `~/.box64rc` or some game may not function correctly.
Note that the priority is: `~/.box64rc` > `/etc/box64.box64rc` > command line
So, your settings in `~/.box64rc` may override the setting from your command line...

----

Notes about Unity game emulation
----

Running Unity games should just work, but you should also note that many Unity3D games require OpenGL 3+ which can be tricky to provide on ARM SBC (single-board computers). Also many newer Unity3D (like KSP) games use the BC7 compressed textures, which is not supported on many ARM integrated GPU.
Hint: on Pi4, use `MESA_GL_VERSION_OVERRIDE=3.2` and with Panfrost use `PAN_MESA_DEBUG=gl3` to use higher profile if the game starts then quits before showing anything.

----

Notes about GTK programs
----

GTK libraries are now wrapped on box64, both gtk2 and gtk3.

----

Notes about Steam
----

Note that Steam is a hybrid 32-bit / 64-bit. You NEED box86 to run Steam, as the client app is a 32-bit binary. It also uses a 64-bit local server binaries, and that steamwebhelper process is now mandatory, even on the "small mode". And that process will eat lots of memory. So machine with less the 6Gb of RAM will need a swapfile to use Steam.

----

Notes about Wine
----

Wine64 is supported on box64. Proton too. Be aware that 64-bit Wine also includes 32-bit components, to be able to run 32-bit Windows programs. The 32-bit apps will need box86 and will not run without it. On a system where both box64 and box86 are present and working, a wine 64-bit setup can run both 32-bit and 64-bit Windows programs (just use `wine` and `wine64` respectively).
Note that the new 32-bit PE in 64-bit processes that the Wine team is currently implementing in Wine 7.+ is now supported. It's called "new WoW64" and it's working fine on box64, allowing the use of 32bits windows program on 64bits only system.

----

Notes about Vulkan
----

Box64 wraps Vulkan libraries, but note that it as mostly been tested with an AMD RX550 card and on the Freedreno driver, so some extensions may be missing depending on your graphics card.

----

Final word
----

I want to thank everyone who has contributed to box64 development.
There are many ways to contribute: code contribution, financial, hardware and advertisement!
So, in no particular order, I want to thank:
 * For their major code contribution: rajdakin, mogery, ksco, xctan
 * For their major financial contribution: [Playtron](https://playtron.one), FlyingFathead, stormchaser3000, dennis1248, sll00, [libre-computer-project](https://libre.computer/), [CubeCoders Limited](http://cubecoders.com/)
 * For hardware contribution and LoongArch migration: [xiaoji](https://www.linuxgame.cn/), Deepin Beijing Develop Team
 * For their hardware contribution: [ADLINK](https://www.adlinktech.com/Products/Computer_on_Modules/COM-HPC-Server-Carrier-and-Starter-Kit/Ampere_Altra_Developer_Platform?lang=en) with [Ampere](https://amperecomputing.com/home/edge), [SOPHGO](https://www.sophon.ai/), [Radxa](https://rockpi.org/), [StarFive](https://rvspace.org/), [Pine64](https://www.pine64.org/), [AYN](https://www.ayntec.com/), [AYANEO](https://ayaneo.com/), [jiangcuo](https://github.com/jiangcuo)
 * For their continuous advertisements for the box64 project: salva ([microLinux](https://www.youtube.com/channel/UCwFQAEj1lp3out4n7BeBatQ)), [PILab](https://www.youtube.com/channel/UCgfQjdc5RceRlTGfuthBs7g)/[TwisterOS](https://twisteros.com/) team, [The Byteman](https://www.youtube.com/channel/UCEr8lpIJ3B5Ctc5BvcOHSnA), [NicoD](https://www.youtube.com/channel/UCpv7NFr0-9AB5xoklh3Snhg), ekianjo ([Boilingsteam](https://boilingsteam.com/))

And I also thank the many other people who participated even once in this project.

(If you use Box64 in your project, please don't forget to mention it!)
