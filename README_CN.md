![图标](docs/img/Box64Logo.png "图标")

# Box64：Linux 用户空间 x86-64 模拟器（支持原生库）

[官网](https://box86.org/) | [更新日志](https://github.com/ptitSeb/box64/blob/main/docs/CHANGELOG.md) | [English](https://github.com/ptitSeb/box64/blob/main/README.md) | [Українська](https://github.com/ptitSeb/box64/blob/main/README_UK.md) | [Report an Error](https://github.com/ptitSeb/box64/issues/new)

![Build Status](https://app.travis-ci.com/ptitSeb/box64.svg?branch=main) ![Stars](https://img.shields.io/github/stars/ptitSeb/box64) ![Forks](https://img.shields.io/github/forks/ptitSeb/box64) ![Contributors](https://img.shields.io/github/contributors/ptitSeb/box64) ![Pull Requests](https://img.shields.io/github/issues-pr/ptitSeb/box64) ![Issues](https://img.shields.io/github/issues/ptitSeb/box64)

---

Box64 可以在非 x86_64 Linux 主机系统（比如 Arm）上运行 x86_64 Linux 程序（比如游戏），目前仅支持 64 位小端主机系统。

在 YouTube 上查看 Box64 的运行效果:
- [MicroLinux](https://www.youtube.com/channel/UCwFQAEj1lp3out4n7BeBatQ)
- [Pi Labs](https://www.youtube.com/channel/UCgfQjdc5RceRlTGfuthBs7g)
- [The Byteman](https://www.youtube.com/channel/UCEr8lpIJ3B5Ctc5BvcOHSnA)

Box64 可以使用本机原生的系统库（libc、libm、SDL、OpenGL），有着易于集成的特性和令人惊讶的性能。如需性能基准测试，请查看[此分析](https://box86.org/index.php/2021/06/game-performances/)。

借助适用于 ARM64，RV64 以及 LA64 平台的 DynaRec，Box64 的速度比单独使用解释器快 5-10 倍。如需深入了解 DynaRec，请参考[此文章](https://box86.org/2021/07/inner-workings-a-high%e2%80%91level-view-of-box86-and-a-low%e2%80%91level-view-of-the-dynarec/)。

<img src="docs/img/Box64Icon.png" width="96" height="96" alt="Box64 Icon">

_Logo 由 @grayduck 制作，感谢！_

---

## 📖 使用方法

- `$ box64 ./program [args]` 运行 Linux x86_64 程序。
- `$ box64 -k` 杀死所有模拟的进程。
- `$ box64-bash` 进入 x86_64 bash 环境。
- 想要使用 wine？请参阅 [Wine 使用说明](docs/WINE.md)。

Box64 还提供了环境变量和 Rc 文件来控制其行为。有关详细信息请参阅[使用文档](docs/USAGE.md)。

---

## 🚀 编译和安装

- [编译说明](https://github.com/ptitSeb/box64/blob/main/docs/COMPILE.md)
- [安装 Wine](https://github.com/ptitSeb/box64/blob/main/docs/WINE.md)

---

## 🔄 版本历史

请参阅[变更日志](docs/CHANGELOG.md)了解版本更新说明。

---

## 🖥️ 平台兼容性说明

Box64 直接转换 x86_64 函数调用到本机原生库，因此需要主机系统上的 64 位库。对于 32 位二进制文​​件，请使用 Box86 或 Box32。

**注意事项**

1. Box32 模式正在制作中，旨在支持 Box64 上运行 32 位二进制文​​件，敬请期待！
2. 某些安装程序可能会将 64 位的主机操作系统判定为 x86 导致兼容性问题。解决方法：使用假的 `uname` ，并使它在运行参数为 `-m` 时返回 `x86_64`。

---

## ⚙️ 配置

Box64 的配置文件位于 `/etc/box64.box64rc` 和 `~/.box64rc`，均为 `.ini` 格式的文件。如果您不想使用默认的 `/etc/box64.box64rc` 文件，请将其复制到 `~/.box64rc` 以确保兼容性。

配置项的优先级：`~/.box64rc` > `/etc/box64.box64rc` > 命令行。

更多信息，请参阅[使用文档](docs/USAGE.md)。

---

## 📄 特定平台的其他说明

### Unity 游戏
- 许多 Unity 游戏需要 OpenGL 3+，这对 ARM/RISC-V SBC 来说可能具有挑战性。
- **对 Pi4 和 Pi5 用户的提示**：设置 `MESA_GL_VERSION_OVERRIDE=3.2` 和 `BOX64_DYNAREC_STRONGMEM=1` 以防止卡死并启用强内存模式。
- **Panfrost**：为了在 ARM 上实现更好的兼容性，启用 `PAN_MESA_DEBUG=gl3` 以强制更高的 OpenGL Profile。如果游戏可以启动但在显示任何内容之前意外退出，这个设置可能会有所帮助。

### GTK 程序

Box64 包装了 GTK 库，支持 gtk2 和 gtk3。

### Steam

请参考[Steam 文档](docs/STEAM.md)。

### Wine

Box64 支持 Wine64 和 Proton。对于 32 位组件，需要 Box86。配备 Box64 和 Box86 的系统可以运行 32 位和 64 位的 Windows 程序。

**提示**：您可以使用 Wine WOW64 版本在仅限 Box64 的环境中运行 x86 Windows 程序，这项支持仍处于实验阶段，但在大多数情况下都可以工作。

有关更多信息，请参阅 [Wine 使用说明](docs/WINE.md)。

### Vulkan

Box64 包装了 Vulkan 库。

----

结语
----

我要感谢所有为 box64 开发做出贡献的人。
贡献方式有很多种：代码贡献、财务帮助、捐赠捐赠和广告合作！
以下感谢不分先后： 

 * 代码贡献：rajdakin, mogery, ksco, xctan
 * 财务支持：[Playtron](https://playtron.one), tohodakilla, FlyingFathead, stormchaser3000, dennis1248, sll00, [libre-computer-project](https://libre.computer/), [CubeCoders Limited](http://cubecoders.com/)
 * 硬件捐赠和 LoongArch 迁移：[xiaoji](https://www.linuxgame.cn/), Deepin Beijing Develop Team
 * 硬件捐赠：[ADLINK](https://www.adlinktech.com/Products/Computer_on_Modules/COM-HPC-Server-Carrier-and-Starter-Kit/Ampere_Altra_Developer_Platform?lang=en) with [Ampere](https://amperecomputing.com/home/edge), [SOPHGO](https://www.sophon.ai/), [Radxa](https://rockpi.org/), [StarFive](https://rvspace.org/), [Pine64](https://www.pine64.org/), [AYN](https://www.ayntec.com/), [AYANEO](https://ayaneo.com/), [jiangcuo](https://github.com/jiangcuo)
 * 为本项目持续的宣传：salva ([microLinux](https://www.youtube.com/channel/UCwFQAEj1lp3out4n7BeBatQ)), [PILab](https://www.youtube.com/channel/UCgfQjdc5RceRlTGfuthBs7g)/[TwisterOS](https://twisteros.com/) team, [The Byteman](https://www.youtube.com/channel/UCEr8lpIJ3B5Ctc5BvcOHSnA), [NicoD](https://www.youtube.com/channel/UCpv7NFr0-9AB5xoklh3Snhg), ekianjo ([Boilingsteam](https://boilingsteam.com/))

我还要感谢很多人，即使他们只参与了一次本项目。

(如果你的项目中使用了Box64，请不要忘记提及！)
