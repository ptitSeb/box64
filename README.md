![Official logo](docs/img/Box64Logo.png "Official Logo")

# Box64: Linux Userspace x86_64 Emulator with a Twist

[Changelog](https://github.com/ptitSeb/box64/blob/main/docs/CHANGELOG.md) | [中文](https://github.com/ptitSeb/box64/blob/main/README_CN.md) | [Українська](https://github.com/ptitSeb/box64/blob/main/README_UK.md) | [Report an Error](https://github.com/ptitSeb/box64/issues/new)

![Build Status](https://app.travis-ci.com/ptitSeb/box64.svg?branch=main) ![Stars](https://img.shields.io/github/stars/ptitSeb/box64) ![Forks](https://img.shields.io/github/forks/ptitSeb/box64) ![Contributors](https://img.shields.io/github/contributors/ptitSeb/box64) ![Pull Requests](https://img.shields.io/github/issues-pr/ptitSeb/box64) ![Issues](https://img.shields.io/github/issues/ptitSeb/box64)

---

Box64 enables running x86_64 Linux programs, including games, on non-x86_64 Linux systems such as ARM (64-bit little-endian host system required).

Check out Box64 in action on YouTube:
- [MicroLinux](https://www.youtube.com/channel/UCwFQAEj1lp3out4n7BeBatQ)
- [Pi Labs](https://www.youtube.com/channel/UCgfQjdc5RceRlTGfuthBs7g)
- [The Byteman](https://www.youtube.com/channel/UCEr8lpIJ3B5Ctc5BvcOHSnA)

Box64 leverages native system libraries (libc, libm, SDL, OpenGL), offering ease of integration and surprising performance in many applications. For performance benchmarks, check [this analysis](https://box86.org/index.php/2021/06/game-performances/).

With DynaRec for ARM64 and RV64 platforms, Box64 achieves a speed boost 5-10x faster than the interpreter alone. For a deeper look at DynaRec, see [this article](https://box86.org/2021/07/inner-workings-a-high%e2%80%91level-view-of-box86-and-a-low%e2%80%91level-view-of-the-dynarec/).

<img src="docs/img/Box64Icon.png" width="96" height="96" alt="Box64 Icon">

_Logo and icon by @grayduck - Thank you!_

---

## 📖 Usage

Box64 offers environment variables to control its behavior. For details, see [Usage Documentation](docs/USAGE.md).

**Debugging Note:** Box64's Dynarec uses Memory Protection and a SegFault signal handler for JIT code handling. If debugging with GDB, set `handle SIGSEGV nostop` to avoid constant interruptions. Set a breakpoint inside `my_memprotectionhandler` in `signals.c` if you want to track SegFaults.

---

## 🚀 Compiling and Installation

- [Compilation Instructions](https://github.com/ptitSeb/box64/blob/main/docs/COMPILE.md)
- [Wine Installation for Box64](https://github.com/ptitSeb/box64/blob/main/docs/X64WINE.md)

---

## 🔄 Version History

See the [Changelog](docs/CHANGELOG.md) for version updates.

---

## 🖥️ Platform Compatibility Notes

Box64 requires 64-bit libraries on the host system, as it directly translates x86_64 function calls. For 32-bit binaries, use Box86.

**Note:** Some installers may default to x86 on ARM64 OS, causing compatibility issues. Workaround: Use a fake `uname` that returns "x86_64" for `-m`.

---

## ⚙️ Configuration

Box64 has configuration files located in `/etc/box64.box64rc` and `~/.box64rc`, both formatted as `.ini` files. If you don’t want to use the default `/etc/box64.box64rc` file, copy it to `~/.box64rc` to ensure compatibility.

Settings priority: `~/.box64rc` > `/etc/box64.box64rc` > Command line.

---

## 📄 Additional Platform-Specific Notes

### Unity Game Emulation
- Many Unity games require OpenGL 3+, which may be challenging on ARM SBCs.
- **Tip for Pi4 and Pi5 Users:** Set `MESA_GL_VERSION_OVERRIDE=3.2` with `BOX64_DYNAREC_STRONGMEM=1` to prevent freezes and enable strong memory mode.
- **Using Panfrost:** For better compatibility on ARM, enable `PAN_MESA_DEBUG=gl3` to force higher OpenGL profiles. This can help if a game starts but quits unexpectedly before showing any content.

Let me know if you’d like further adjustments!

### GTK Programs
- Box64 now wraps GTK libraries, supporting both gtk2 and gtk3.

### Steam
- Steam requires Box86 due to its 32-bit client app, but uses 64-bit local server binaries. Systems with less than 6GB RAM may need a swap file for optimal performance.

### Wine
- Box64 supports Wine64 and Proton. For 32-bit components, Box86 is required. Systems with both Box64 and Box86 can run 32- and 64-bit Windows programs.

### Vulkan
- Box64 wraps Vulkan libraries. Limited testing done with AMD RX550 and Freedreno drivers.

---

## ❤️ Acknowledgments

A huge thank you to everyone who has contributed to Box64 development! Special thanks to:

- **Code Contributors**: rajdakin, mogery, ksco, xctan
- **Financial Supporters**: [Playtron](https://playtron.one), tohodakilla, FlyingFathead, stormchaser3000, dennis1248, sll00, [libre-computer-project](https://libre.computer/), [CubeCoders Limited](http://cubecoders.com/)
- **Hardware Contributions**: [ADLINK](https://www.adlinktech.com), [SOPHGO](https://www.sophon.ai), [Radxa](https://rockpi.org), [StarFive](https://rvspace.org), [Pine64](https://www.pine64.org), [AYN](https://www.ayntec.com), [AYANEO](https://ayaneo.com)

---

If you use Box64 in your project, please don’t forget to mention it!

---
