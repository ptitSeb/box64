# box64

![Official logo](img/Box64Logo.png "Official Logo")
Linux上用户空间的x86_64应用模拟器 

----
![box64 build status](https://app.travis-ci.com/ptitSeb/box64.svg?branch=main)

[English](README.md)

在ARM或其他非X86_64架构的Linux操作系统上，使用Box64可以运行x86_64 Linux程序（例如游戏），请注意，主机系统需要**64位小端**。

您可以在Youtube的[MicroLinux](https://www.youtube.com/channel/UCwFQAEj1lp3out4n7BeBatQ)、[Pi Labs](https://www.youtube.com/channel/UCgfQjdc5RceRlTGfuthBs7g)或[The Byteman](https://www.youtube.com/channel/UCEr8lpIJ3B5Ctc5BvcOHSnA)频道找到更多Box64视频。

Box64使用了一些“系统”库的原生版本，例如libc、libm、SDL和OpenGL，所以它很容易与大多数应用程序集成和使用，并且在很多情况下性能会出奇的高。点击[此处](https://box86.org/index.php/2021/06/game-performances/)查看示例的基准测试分析。

Box64为ARM64平台集成了DynaRec（动态重编译器），比仅使用解释器提升5到10倍性能。关于Dynarec如何工作的更多信息，可以在[这里](https://box86.org/2021/07/inner-workings-a-high%e2%80%91level-view-of-box86-and-a-low%e2%80%91level-view-of-the-dynarec/)找到。

注：DynaRec需要针对不同平台编写一部分汇编代码，目前仅支持ARM64。

一些x64内部操作码使用“Realmode X86 Emulator Library”的部分内容，请参阅[x64primop.c](../src/emu/x64primop.c)了解版权详情 

<img src="img/Box64Icon.png" width="96" height="96">

Logo和图标由grayduck制作，感谢!

----

使用方法
----

若干环境变量可以设置Box64的行为。

点击[这里](USAGE.md)了解所有的环境变量和它们的作用。

请注意：Box64的Dynarec提供了一种机制来处理JIT代码，包含内存保护和段错误信号处理程序。简单地说，如果您想用GDB来调试一个使用JIT代码的运行程序（比如mono/Unity3D），您仍然会触发很多“正常”的段错误。建议在GDB中使用类似于`handle SIGSEGV nostop`的东西，不要在每个段错误时都停止，如果您想捕获段错误，可以在`signals.c`中的`my_memprotectionhandler`中放置一个断点。

----

编译/安装
----
> 编译说明可以在[这里](COMPILE.md)查看。  

----

版本历史
----

版本日志在[这里](CHANGELOG.md)。

----

32位平台的注意事项
----

Box64的工作原理是直接将函数调用从x86_64转换到主机系统，主机系统（Box64运行的系统）需要有64位的库。Box64并不包括任何64位<->32位的转换。

所以请理解，Box64只能运行64位的Linux二进制文件。对于32位的二进制文件，需要Box86（在64位的操作系统上需要所有的multiarch库或一些proot技巧）。

请注意，许多(基于mojo的)安装程序，即使存在x86_64版本，但在检测到架构为ARM64时会退回到"x86"，这时会尝试使用（如果有）box86进行安装。这时你可以使用一个假的"uname"，当它的参数为"-m"时返回"x86_64"。

----

关于Unity游戏模拟的注意事项
----

当前Unity游戏模拟是一件碰运气的事情。Unity使用Mono（它的信号表示，并不总能100%成功模拟）。

您还应该注意，一些Unity3D游戏需要OpenGL 3+，目前在ARM SBC（单板计算机）上提供OpenGL 3+是很棘手的。

所以，并不是所有的Unity游戏都能工作，而且可能需要很高的OpenGL配置，但现在很多游戏都能工作。

提示：如果游戏开始后在显示任何东西之前退出，在Pi4上使用`MESA_GL_VERSION_OVERRIDE=3.2`，在Panfrost上使用`PAN_MESA_DEBUG=gl3`来使用更高的配置文件。

----

关于GTK程序的注意事项
----

GTK 庫現在包裝在 box64 上，但只有 gtk2，而不是 gtk3（與 box86 相反）。

----

关于Steam的注意事项
----

Steam并不是32位/64位混合的，而是32位的，因此您需要Box86才能运行Steam。Steam也使用64位的本地服务器程序，但像大多数使用libcef/chromium的程序一样，在box64上暂时无法正常工作。
所以：目前在box64上无法运行Steam。
----

关于Wine的注意事项
----

box64支持64位Wine，但请注意，64位的Wine还包含32位的组件，以便能够运行32位的Windws程序。32位程序需要Box86，没有它就不能运行。在Box64和Box86共存的系统上，设置64位Wine可以同时运行32位和64位windows程序（只需分别使用 `wine` 和 `wine64`）。

----

关于Vulkan注意事项
----

Box64 包裝 Vulkan 庫。但請注意，它僅使用 RX550 卡進行了測試，根據您的圖形卡，可能會缺少某些擴展。

----

结语
----

我要感谢所有为 box64 开发做出贡献的人。
贡献方式有很多种：代码贡献、财务帮助、捐赠捐赠和广告合作！
以下感谢不分先后： 
 * 代码贡献：rajdakin, mogery
 * 财务帮助：FlyingFathead, stormchaser3000, dennis1248
 * 硬件捐赠：[Radxa](https://rockpi.org/), [Pine64](https://www.pine64.org/), [StarFive](https://rvspace.org/)
 * 硬件捐赠和LoongArch迁移：[xiaoji](https://www.linuxgame.cn)
 * 为本项目持续的宣传：salva ([microLinux](https://www.youtube.com/channel/UCwFQAEj1lp3out4n7BeBatQ)), [PILab](https://www.youtube.com/channel/UCgfQjdc5RceRlTGfuthBs7g)/[TwisterOS](https://twisteros.com/) team, [The Byteman](https://www.youtube.com/channel/UCEr8lpIJ3B5Ctc5BvcOHSnA), [NicoD](https://www.youtube.com/channel/UCpv7NFr0-9AB5xoklh3Snhg), ekianjo ([Boilingsteam](https://boilingsteam.com/))

我还要感谢很多人，即使他们只参与了一次本项目。

(如果你的项目中使用了Box64，请不要忘记提及！)
