# box64

![Official logo](docs/img/Box64Logo.png "Official Logo")
Linux 上用户空间的 x86_64 应用模拟器（支持运行原生库）

----
![box64 build status](https://app.travis-ci.com/ptitSeb/box64.svg?branch=main)

[English](docs/README.md) | [Українська](https://github.com/ptitSeb/box64/blob/main/README_UK.md) 

Box64 可以在非 x86_64 Linux 系统（比如 ARM64）上运行 x86_64 Linux 程序（比如游戏），注意主机系统需要是 64 位小端。

您可以在 [MicroLinux](https://www.youtube.com/channel/UCwFQAEj1lp3out4n7BeBatQ)、[Pi Labs](https://www.youtube.com/channel/UCgfQjdc5RceRlTGfuthBs7g) 和 [The Byteman](https://www.youtube.com/channel/UCEr8lpIJ3B5Ctc5BvcOHSnA) YouTube 频道上找到许多 Box64 视频。

由于 Box64 使用一些“系统”库的原生版本，如 libc、libm、SDL 和 OpenGL 等，因此很容易与大多数应用程序集成和使用，并且在许多情况下性能会相当不错。可以在[这里](https://box86.org/index.php/2021/06/game-performances/)查看一些性能测试的样例。

Box64 集成了适用于 ARM64 和 RV64 平台的 DynaRec（动态重编译器），速度可以比纯解释模式快 5 到 10 倍。可以在[这里](https://box86.org/2021/07/inner-workings-a-high%E2%80%91level-view-of-box86-and-a-low%E2%80%91level-view-of-the-dynarec/)找到有关 DynaRec 工作原理的一些信息。

一些 x64 内部操作码使用 “Realmode X86 Emulator Library” 的部分内容，有关版权详细信息，请参见 [x64primop.c](../src/emu/x64primop.c)。

<img src="docs/img/Box64Icon.png" width="96" height="96">

LOGO 由 @grayduck 制作，感谢！

----

使用方法
----

有若干环境变量可以控制 Box64 的行为。

可在[这里](docs/USAGE.md)查看所有环境变量及其作用。

注意：Box64 的 Dynarec 使用具有内存保护和段错误信号处理的机制来执行 JIT 代码。所以，如果想使用 GDB 调试使用 JIT 代码的程序（如 Mono/Unity3D），这会触发许多“正常”的段错误。建议在 GDB 中使用类似 `handle SIGSEGV nostop` 来防止它每个段错误处停止。如果你想捕获段错误，可以在 `signals.c` 的 `my_memprotectionhandler` 中设置断点。

----

编译/安装
----

编译说明可以在[这里](docs/COMPILE.md)查看。

----

版本历史
----

版本日志在[这里](docs/CHANGELOG.md)。

----

32位平台的注意事项
----

因为 Box64 的工作原理是直接将函数调用从 x86_64 转换为主机系统，所以主机系统（运行 Box64 的系统）需要有 64 位库。Box64 不包含任何 64 位 <-> 32 位的转换。

所以 box64 只能运行 64 位的 Linux 二进制。对于 32 位二进制则需要使用 box86 来运行（它在 64 位操作系统上使用了 multiarch 和 proot 等技巧来实现运行）。请注意，许多（基于 mojo 的）安装程序在检测到 ARM64 操作系统时将回退到 “x86”，因此即使存在 x86_64 版本，也会尝试使用 box86。这时你可以使用一个假的 `uname` ，并使它在运行参数为 `-m` 时返回 `x86_64`。

----

关于 Unity 游戏模拟的注意事项
----

运行 Unity 游戏应该没什么问题，但还应该注意，许多 Unity3D 游戏需要 OpenGL 3+，这在 ARM SBC 上可能会比较棘手。同时许多较新的 Unity3D游戏（如 KSP）也使用 BC7 压缩纹理，很多 ARM 的集成显卡并不支持。

> 提示：如果游戏开始后没有显示任何东西就退出了，在 Pi4 上可以使用 `MESA_GL_VERSION_OVERRIDE=3.2`，在 Panfrost 上则可以使用 `PAN_MESA_DEBUG=gl3` 来使用更高的配置。

----

关于GTK程序的注意事项
----

box64 封装了 GTK，包括 gtk2 和 gtk3。

----

关于 Steam 的注意事项
----

请注意，Steam 是 32/64 位混合的应用，所以你需要 box86 才能运行，因为客户端应用程序是 32 位的。它还使用 64 位本地服务器，它的 steamwebhelper 无法被关闭（即使是在最小模式）而且会吃掉大量的内存。对于内存小于 6 GB 的机型，你将会需要创建 swapfile 来运行 Steam。

----

关于 Wine 的注意事项
----

box64 支持 Wine64，Proton 应该也能运行。请注意，64 位 Wine 包含有 32 位组件，以便能够运行 32 位 Windows 程序。32 位应用程序需要 box86，否则无法运行。在 box64 和 box86 都存在并工作的系统上，64 位的 Wine 可以同时运行 32 位和 64 位 Windows 程序（分别使用 `wine` 和 `wine64`）。请注意，目前在 Wine 7.+ 中实现的 Wine 时间在 64 位进程中的新 32bit PE 尚不支持。我测试了 Wine 7.5 64 位可以正常工作，但是更新的版本可能还不行。

----

关于 Vulkan 注意事项
----

Box64 封装了 Vulkan 库，但请注意，它仅在 RX550 显卡上进行过测试，因此根据您的显卡，某些扩展可能会丢失。

----

结语
----

我要感谢所有为 box64 开发做出贡献的人。
贡献方式有很多种：代码贡献、财务帮助、捐赠捐赠和广告合作！
以下感谢不分先后： 

 * 代码贡献：rajdakin, mogery, ksco, xctan
 * 财务帮助：[Playtron](https://playtron.one), FlyingFathead, stormchaser3000, dennis1248, sll00, [libre-computer-project](https://libre.computer/)
 * 硬件捐赠和 LoongArch 迁移：[xiaoji](https://www.linuxgame.cn/), Deepin Beijing Develop Team
 * 硬件捐赠：[ADLINK](https://www.adlinktech.com/Products/Computer_on_Modules/COM-HPC-Server-Carrier-and-Starter-Kit/Ampere_Altra_Developer_Platform?lang=en) with [Ampere](https://amperecomputing.com/home/edge), [SOPHGO](https://www.sophon.ai/), [Radxa](https://rockpi.org/), [StarFive](https://rvspace.org/), [Pine64](https://www.pine64.org/), [AYN](https://www.ayntec.com/), [AYANEO](https://ayaneo.com/), [jiangcuo](https://github.com/jiangcuo)
 * 为本项目持续的宣传：salva ([microLinux](https://www.youtube.com/channel/UCwFQAEj1lp3out4n7BeBatQ)), [PILab](https://www.youtube.com/channel/UCgfQjdc5RceRlTGfuthBs7g)/[TwisterOS](https://twisteros.com/) team, [The Byteman](https://www.youtube.com/channel/UCEr8lpIJ3B5Ctc5BvcOHSnA), [NicoD](https://www.youtube.com/channel/UCpv7NFr0-9AB5xoklh3Snhg), ekianjo ([Boilingsteam](https://boilingsteam.com/))

我还要感谢很多人，即使他们只参与了一次本项目。

(如果你的项目中使用了Box64，请不要忘记提及！)
