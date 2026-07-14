# 在 Box64 上使用 Wine 的指南

Wine 不是一个模拟器，但 Box64 是。Box64 以多种方式支持 Wine。在继续阅读之前，你应该对 Wine 有基本的了解。

## Wine 的变体

Wine 以多种形式发布：


1. **x86 Wine**

    **x86 Wine** 的可执行文件架构是 x86，它运行 x32 Windows 可执行文件。
    
    x86 Wine 需要 32 位环境（读取 32 位 Linux 库），因为该架构天然的特点，这在 x86_64 机器上通常可用。

    但是，当你尝试在 RISC 架构上通过模拟器运行它时，情况则有些不同。对于 Arm64，存在 armhf 这样的 32 位环境，也有为 armhf 设计的 Box86，因此你可以使用 Box86 运行 **x86 Wine**。
    
    然而，对于 RISC-V 64、LoongArch64 以及许多较新的 Arm64 核心，32 位环境并不可用。Box64 引入了一种名为 Box32 的新模式，可以在编译时通过 CMake 选项 `BOX32` 启用。它本质上利用 64 位环境模拟 32 位环境，因此可以用来运行 x86 Wine。但请注意，Box32 仍是高度实验性且不稳定的。

2. **x86_64 Wine**

    **x86_64 Wine** 的可执行文件架构是 x86_64，它运行 x64 Windows 可执行文件。
    
    它可以与 Box64 配合使用。注意，**x86_64 Wine** 通常自带 **x86 Wine**。因此你可以结合使用 Box86 + Box64，或者仅使用启用了 Box32 的 Box64，来无缝运行 32 位和 64 位可执行文件。

3. **x86_64 Wine WOW64**

    **x86_64 Wine WOW64** 的可执行文件架构是 x86_64，它同时能运行 x32 和 x64 Windows 可执行文件。
    
    **x86_64 Wine WOW64** 的优点在于它运行 32 位 Windows 可执行文件时不需要 32 位环境，因为 32 位环境模拟已由 Wine WOW64 自身处理。
    
    这意味着你可以只使用 Box64（无需 Box32）来运行 32 位 Windows 可执行文件。然而缺点是，对于 32 位程序，WineD3D 的性能相当差（请使用 DXVK！）。

4. **Arm64 Wine WOW64**

    **Arm64 Wine WOW64** 的可执行文件架构是 Arm64，它能运行 x32、x64、Arm32 和 Arm64 的 Windows 可执行文件。
    
    但是，要运行 x32 和 x64 的程序，需要一个 cpu.dll，也就是所谓的 WOW64（用于 32 位）和 Arm64EC（用于 64 位）。
    
    Box64 现已支持 32 位 WOW64 DLL。你可以通过编译时启用 `WOW64` CMake 选项来启用它。我们计划在未来支持 Arm64EC。

## Wine 的安装

对于 x86 Wine，有很多下载渠道，略举几例：

- https://github.com/Kron4ek/Wine-Builds （Wine \[WOW64\]）
- https://github.com/GloriousEggroll/proton-ge-custom （Proton，即 Valve 优化过的 Wine）

下载后，只需以你喜欢的方式将 `bin` 文件夹添加到 `PATH` 环境变量中；Wine 应该能很好地与 Box64 配合工作。

要安装 DXVK、d3dcompiler_xx 或 cjkfonts 等组件，推荐使用 Winetricks：https://github.com/Winetricks/winetricks

对于 Arm64 Wine，推荐使用 Hangover：https://github.com/AndreRH/hangover
