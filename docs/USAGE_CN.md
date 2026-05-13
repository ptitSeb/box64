<!--- 此文件由 gen.py 自动生成，请勿直接编辑。 -->
使用说明
----

Box64 有许多环境变量可用于控制其行为，下面将按类别列出。

Box64 有两种构建类型：Wine WOW64 构建（WowBox64）和常规 Linux 构建。请注意，只有部分环境变量在 WowBox64 中可用。

### 配置文件

除了环境变量之外，如果你使用的是常规 Linux 构建，Box64 默认还会在两个位置查找 rcfile：系统范围的 `/etc/box64.box64rc` 和用户目录下的 `~/.box64rc`。
而在 WowBox64 中，配置文件仅在 `%USERPROFILE%/.box64rc` 处检查。
设置的优先级如下（从高到低）：`~/.box64rc` > `/etc/box64.box64rc` > 环境变量。

配置示例：

```
[factorio]
BOX64_DYNAREC_SAFEFLAGS=0
BOX64_DYNAREC_BIGBLOCK=2
BOX64_DYNAREC_FORWARD=1024
BOX64_DYNAREC_CALLRET=1
```

此配置将使所有名为 `factorio` 的可执行文件应用指定的设置。

共享选项也可以定义在特殊的 `[*]` 配置段中。这些设置会全局应用，而更具体的配置段只会覆盖它自己显式定义的选项。

### Linux 构建的高级用法

1. **通配符匹配**

   可以使用星号（`*`）对应用程序名称进行基本的模式匹配。例如，`[*setup*]` 将匹配名称中包含 "setup" 的任何程序。请注意，这里实现的是简单的通配符匹配，而非完整的正则表达式支持。
2. **共享设置**

   特殊的 `[*]` 配置段可为所有应用程序定义共享的 rcfile 选项。它不同于 `[*setup*]` 这类通配符配置段，是一个专用的全局配置段。
3. **自定义配置文件**

   `BOX64_RCFILE` 环境变量可以指定一个替代的配置文件，而不是默认的 `/etc/box64.box64rc`。
4. **按文件设置**

   以 `/` 开头的配置项适用于特定文件。例如：
   ```
   [/d3d9.dll]
   BOX64_DYNAREC_SAFEFLAGS=0
   ```
   这些设置将仅影响 `d3d9.dll` 文件。此语法同样适用于**模拟的** Linux 库，例如 `[/libstdc++.so.6]`。

----

## 性能

### BOX64_DYNAREC

启用或禁用动态重编译器（DynaRec）。如果在支持的架构上构建时开启了 DynaRec 支持，则默认为 1。 在 WowBox64 中可用。

 * 0: 禁用 DynaRec。
 * 1: 启用 DynaRec。

### BOX64_DYNAREC_ALIGNED_ATOMICS

仅生成对齐的原子操作（目前仅在 Arm64 上可用）。 在 WowBox64 中可用。

 * 0: 生成非对齐原子操作处理代码。 [默认值]
 * 1: 仅生成对齐的原子操作，代码更快更小，但在非对齐数据地址上执行 LOCK 前缀指令时会导致 SIGBUS。

### BOX64_DYNAREC_BIGBLOCK

尝试构建更大的代码块以提高性能。 在 WowBox64 中可用。

 * 0: 不尝试构建尽可能大的代码块，适用于使用大量线程和 JIT 的程序（如 Unity）。
 * 1: 尽可能构建大的代码块。
 * 2: 构建更大的代码块，代码块重叠时不停止，但仅针对 elf 内存中的代码块。 [默认值]
 * 3: 构建更大的代码块，代码块重叠时不停止，适用于所有类型的内存，因此也对 wine 程序有用。

### BOX64_DYNAREC_CALLRET

优化 CALL/RET 指令。 在 WowBox64 中可用。

 * 0: 不优化 CALL/RET，使用跳转表。 [默认值]
 * 1: 尝试优化 CALL/RET，在可能的情况下跳过跳转表。
 * 2: 尝试优化 CALL/RET，在可能的情况下跳过跳转表，同时增加对返回到脏/已修改代码块的代码的处理。在 WowBox64 上不可用。

### BOX64_DYNAREC_SEP

使用二级入口点（SEP）进一步优化 CALL/RET。如果 CALLRET 未启用则无效。 在 WowBox64 中可用。

 * 0: 不在 CALLRET 返回时添加 SEP。
 * 1: 在 CALLRET 返回时添加 SEP，仅针对来自已知二进制文件的内存。 [默认值]
 * 2: 在 CALLRET 返回时添加 SEP，适用于所有类型的内存。

### BOX64_DYNAREC_DF

启用或禁用延迟 FLAGS 的使用。 在 WowBox64 中可用。

 * 0: 禁用延迟 FLAGS 的使用。
 * 1: 启用延迟 FLAGS 的使用。 [默认值]

### BOX64_DYNAREC_DIRTY

是否允许继续运行未受保护且可能已脏的代码块。

 * 0: 不允许继续运行未受保护且可能已脏的代码块。 [默认值]
 * 1: 允许继续运行在与代码相同页面中写入数据的代码块，可以加快某些游戏的加载时间，但也可能导致意外崩溃。
 * 2: 除了 1 之外，当检测到热页时，将该页面标记为永不写保护，但基于该页面构建的代码块将始终被测试。这样可以更快（但某些 SMC 情况可能无法捕获）。

### BOX64_DYNAREC_NOHOTPAGE

是否禁用热页检测（即代码执行和数据写入同时发生的页面）。

 * 0: 检测热页。 [默认值]
 * 1: 不检测热页。

### BOX64_DYNAREC_FASTNAN

启用或禁用快速 NaN 处理。 在 WowBox64 中可用。

 * 0: 精确模拟 x86 上的 -NaN 生成。
 * 1: 不对 -NaN 生成做任何特殊处理，速度更快。 [默认值]

### BOX64_DYNAREC_FASTROUND

启用或禁用快速舍入。 在 WowBox64 中可用。

 * 0: 生成 float/double -> int 舍入，并使用当前舍入模式进行 float/double 计算，类似 x86。
 * 1: 不对边缘情况的舍入做任何特殊处理，速度更快。 [默认值]
 * 2: 使用当前舍入模式生成 float/double -> int 舍入（类似 x86），但使用快速的 int -> float/double 转换。

### BOX64_DYNAREC_FORWARD

定义构建代码块时允许的最大前向跳转值。 在 WowBox64 中可用。

 * 0: 无前向跳转值。当前代码块结束时，即使存在之前的前向跳转也不尝试继续前进。
 * 128: 允许代码块结束与下一个前向跳转之间最多 128 字节的间隙。 [默认值]
 * XXXX: 允许代码块结束与下一个前向跳转之间最多 XXXX 字节的间隙。

### BOX64_DYNAREC_NATIVEFLAGS

启用或禁用原生FLAGS的使用。 在 WowBox64 中可用。

 * 0: 不使用原生FLAGS。
 * 1: 在可能的情况下使用原生FLAGS。 [默认值]

### BOX64_DYNAREC_PAUSE

启用 x86 PAUSE 模拟，可能有助于自旋锁的性能。 在 WowBox64 中可用。

 * 0: 忽略 x86 PAUSE 指令。 [默认值]
 * 1: 使用 YIELD 模拟 x86 PAUSE 指令。
 * 2: 使用 WFI 模拟 x86 PAUSE 指令。
 * 3: 使用 SEVL+WFE 模拟 x86 PAUSE 指令。

### BOX64_DYNAREC_SAFEFLAGS

在 CALL/RET 指令和其他边缘情况下 FLAGS 模拟的行为。 在 WowBox64 中可用。

 * 0: 将 CALL/RET 标记为从不需要任何 FLAGS。
 * 1: 将大多数 RET 标记为需要 FLAGS，将大多数 CALL 标记为不需要。 [默认值]
 * 2: 将所有 CALL/RET 都标记为需要 FLAGS。同时也处理所有边缘情况。

### BOX64_DYNAREC_STRONGMEM

开启或关闭 x86 强内存模型的模拟。 在 WowBox64 中可用。

 * 0: 不做任何特殊处理。 [默认值]
 * 1: 在写入内存时启用以有限方式模拟 x86 强内存模型的内存屏障。
 * 2: 包含 1 的全部内容，外加 SIMD 指令的内存屏障。
 * 3: 包含 2 的全部内容，外加定期添加更多内存屏障。
 * 4: 模仿 x86 TSO，类似 QEMU 的方法，可用于评估目的。

### BOX64_DYNAREC_VOLATILE_METADATA

使用从 PE 文件解析的 volatile 元数据，仅对 64 位 Windows 游戏有效。

 * 0: 不使用 volatile 元数据。
 * 1: 使用 volatile 元数据，将指导 Box64 进行更好的强内存模拟。 [默认值]

### BOX64_DYNAREC_WAIT

是否等待代码块构建完成。 在 WowBox64 中可用。

 * 0: 不等待，改用解释器，可能在大量多线程或 JIT 程序上略微加速。
 * 1: 等待代码块构建完成。 [默认值]

### BOX64_MMAP32

在运行 32 位代码的 64 位程序（如 Wine WOW64）上强制使用 32 位兼容的内存映射，可提高性能。

 * 0: 不强制使用 32 位内存映射。
 * 1: 强制使用 32 位内存映射。 [默认值]

### BOX64_DYNAREC_NOARCH

从代码块中移除架构元数据（在信号处理时使用）。 在 WowBox64 中可用。

 * 0: 为每个代码块生成所有架构元数据，包括 x87/mmx/SSE/AVX 寄存器追踪。会使用更多内存。 [默认值]
 * 1: 仅生成每个代码块的必要元数据（FLAGS、对齐），节省一些内存，但某些加密软件可能会失败。
 * 2: 不为代码块生成元数据，进一步节省内存，但在信号处理时可能会失败。

### BOX64_RDTSC_1GHZ

如果可用，使用硬件计数器实现 rdtsc。

 * 0: 如果可用，使用硬件计数器实现 rdtsc 指令。 [默认值]
 * 1: 仅当精度至少为 1GHz 时，使用硬件计数器实现 rdtsc。

## 不稳定或遗留选项

### BOX64_DYNAREC_PURGE

根据年龄阈值（BOX64_DYNAREC_PURGE_AGE）清除最近未执行的代码块。

 * 0: DynaRec 不会清除旧的未使用代码块。 [默认值]
 * 1: DynaRec 将根据年龄阈值清除旧的未使用代码块。

### BOX64_DYNAREC_PURGE_AGE

代码块清除的年龄阈值（需要 BOX64_DYNAREC_PURGE=1）。

 * 4096: 默认年龄阈值。 [默认值]
 * XXXX: 自定义年龄阈值（范围：10-65536）。

### BOX64_DYNAREC_WEAKBARRIER

调整内存屏障以减少强内存模拟的性能影响。 在 WowBox64 中可用。

 * 0: 使用常规安全屏障。
 * 1: 使用弱屏障以获得更多性能提升。 [默认值]
 * 2: 包含 1 的全部内容，外加禁用最后的写屏障。

### BOX64_DYNACACHE

启用或禁用动态重编译器缓存（DynaCache）。此选项默认为 2（读取已有缓存但不生成新的）。DynaCache 默认将文件写入 home 文件夹，生成新缓存文件时会根据 BOX64_DYNACACHE_LIMIT 控制文件夹大小。

 * 0: 禁用 DynaCache。
 * 1: 启用 DynaCache。
 * 2: 使用已有的 DynaCache 文件，但不生成新的。 [默认值]

### BOX64_DYNACACHE_FOLDER

设置 DynaCache 文件的文件夹。默认为 $XDG_CACHE_HOME/box64，如果 $XDG_CACHE_HOME 未设置则为 $HOME/.cache/box64。

 * XXXX: 使用文件夹 XXXX 存储 DynaCache 文件。

### BOX64_DYNACACHE_LIMIT

DynaCache 文件夹的最大大小（MiB）。超过限制时，会先删除无效的 DynaCache 文件，如仍超限再删除最旧的有效文件。默认大小为 2048MiB。

 * 0: 不限制 DynaCache 文件夹大小。
 * XXXX: 先删除无效缓存文件，如仍超限再删除最旧的有效缓存文件，将 DynaCache 文件夹保持在 XXXX MiB 以下。
 * 2048: 默认值为 2048 MiB。 [默认值]

### BOX64_DYNACACHE_MIN

DynaCache 写入磁盘的最小大小（KB）。默认大小为 350KB。

 * XXXX: 设置将缓存写入磁盘的最小 DynaRec 代码大小（XXXX KB），小于此值将不会保存到磁盘。
 * 350: 默认值为 350 KB。 [默认值]

## 兼容性

### BOX64_AES

是否暴露 AES 能力。 在 WowBox64 中可用。

 * 0: 不暴露 AES 能力。
 * 1: 暴露 AES 能力。 [默认值]

### BOX64_ARCH

在 rcfile 中指定当前条目适用的架构。此选项仅在 rcfile 中使用时生效。

 * `arm64`: 仅对 AArch64 应用当前条目。
 * `la64`: 仅对 LoongArch64 应用当前条目。
 * `rv64`: 仅对 RISC-V 应用当前条目。
 * `<empty>`: 对所有架构应用当前条目，但如果存在更具体的条目，当前条目将被替换。 [默认值]

### BOX64_AVX

向 CPUID 和 cpuinfo 文件暴露 AVX 扩展。在 Arm64 上默认值为 2，因为 DynaRec 已完全实现，其他架构默认为 0。 在 WowBox64 中可用。

 * 0: 不暴露 AVX 能力。
 * 1: 向 CPUID 和 cpuinfo 文件暴露 AVX、BMI1、F16C 和 VAES 扩展。
 * 2: 包含 1 的全部内容，外加 AVX2、BMI2、FMA、ADX、VPCLMULQDQ 和 RDRAND 扩展。

### BOX64_BASH

用于指定 bash 可执行文件的路径。

 * XXXX: 使用路径为 XXXX 的 bash 可执行文件。

### BOX64_CPUTYPE

指定要模拟的 CPU 类型。 在 WowBox64 中可用。

 * 0: 模拟某个 Intel CPU 型号。 [默认值]
 * 1: 模拟某个 AMD CPU 型号。

### BOX64_CRASHHANDLER

是否使用模拟的 crashhandler.so 库。

 * 0: 在需要时使用模拟的 crashhandler.so 库。
 * 1: 使用内部定义的空 crashhandler.so 库。 [默认值]

### BOX64_DYNAREC_BLEEDING_EDGE

检测 MonoBleedingEdge 并应用保守设置（仅 Linux）。

 * 0: 不检测 MonoBleedingEdge。
 * 1: 检测 MonoBleedingEdge，并在检测到时应用 BOX64_DYNAREC_BIGBLOCK=0 和 BOX64_DYNAREC_强内存模型=1。 [默认值]

### BOX64_DYNAREC_DIV0

启用或禁用除零异常的生成。 在 WowBox64 中可用。

 * 0: 不生成除零异常。 [默认值]
 * 1: 生成除零异常。

### BOX64_DYNAREC_TBB

启用或禁用 libtbb 检测。

 * 0: 不检测 libtbb。
 * 1: 检测 libtbb 并应用保守设置。 [默认值]

### BOX64_DYNAREC_X87DOUBLE

强制使用 float/double 进行 x87 模拟。 在 WowBox64 中可用。

 * 0: 尝试在可能的情况下对 x87 模拟使用 float。 [默认值]
 * 1: 仅对 x87 模拟使用 Double。
 * 2: 在 x87 模拟中检查精度控制的低精度。

### BOX64_DYNAREC_INTERP_SIGNAL

允许在信号处理器中使用 DynaRec。

 * 0: 允许在信号处理器中使用 DynaRec。 [默认值]
 * 1: 仅在信号处理器中使用解释器（与 v0.3.8 及更早版本相同）。

### BOX64_EXIT

直接退出，不运行程序。

 * 0: 不执行任何操作。 [默认值]
 * 1: 直接退出。

### BOX64_FIX_64BIT_INODES

修复 64 位 inode。

 * 0: 不修复 64 位 inode。 [默认值]
 * 1: 修复 64 位 inode。

### BOX64_FORCE_LD_PRELOAD

是否强制 BOX64_LD_PRELOAD 库中的符号通过 RTLD_NEXT 解析。

 * 0: 不执行任何操作。 [默认值]
 * 1: 强制 BOX64_LD_PRELOAD 库中的符号通过 RTLD_NEXT 解析。

### BOX64_IGNOREINT3

忽略 INT3 指令。

 * 0: 如果存在处理器则触发 TRAP 信号。 [默认值]
 * 1: 静默跳过该指令。

### BOX64_JVM

检测 libjvm 并应用保守设置。

 * 0: 不执行任何操作。
 * 1: 检测 libjvm，并在检测到时应用 BOX64_DYNAREC_BIGBLOCK=0 BOX64_DYNAREC_强内存模型=1 BOX64_SSE42=0。 [默认值]

### BOX64_LIBCEF

检测 libcef 并应用 BOX64_MALLOC_HACK 设置。

 * 0: 不执行任何操作。 [默认值]
 * 1: 检测 libcef，并在检测到时应用 BOX64_MALLOC_HACK=2。

### BOX64_MALLOC_HACK

挂钩 malloc 操作符时的行为。

 * 0: 不允许重定向 malloc 操作符，重写代码使用常规函数。 [默认值]
 * 1: 允许重定向 malloc 操作符。
 * 2: 与 0 类似，但追踪特殊的 mmap/free（某些内联的重定向函数无法被重定向）。

### BOX64_MAXCPU

暴露的最大 CPU 核心数。 在 WowBox64 中可用。

 * 0: 使用实际的 CPU 核心数。 [默认值]
 * XXXX: 使用 XXXX 个 CPU 核心。

### BOX64_NORCFILES

不加载任何 rc 文件。

 * 0: 加载 rc 文件。 [默认值]
 * 1: 不加载任何 rc 文件。

### BOX64_PATH

查找 x86_64 二进制文件的路径。

 * XXXX: 将路径 XXXX 添加到二进制搜索路径。

### BOX64_PCLMULQDQ

暴露 PCLMULQDQ 能力。 在 WowBox64 中可用。

 * 0: 不暴露 PCLMULQDQ 能力。
 * 1: 暴露 PCLMULQDQ 能力。 [默认值]

### BOX64_PROFILE

预定义的环境变量集合，以兼容性或性能为导向。 在 WowBox64 中可用。

 * safest: 禁用所有不安全的 DynaRec 优化的配置文件。
 * safe: 比 safest 略不安全。
 * default: 大多数程序都能正常运行且性能适中的默认设置。 [默认值]
 * fast: 启用许多不安全的优化，但同时启用 强内存模型 模拟。
 * fastest: 启用许多不安全的优化以获得更好的性能。

### BOX64_PYTHON3

python3 可执行文件的路径。

 * XXXX: 使用路径为 XXXX 的 python3 可执行文件。

### BOX64_RCFILE

要加载的 rc 文件路径。

 * XXXX: 加载 rc 文件 XXXX，不会加载默认的 rc 文件。

### BOX64_RDTSC_INV

在 Intel CPU 类型的 CPUID 中报告 Invariant TSC 位。 在 WowBox64 中可用。

 * 0: 不为 Intel CPU 类型报告 Invariant TSC（默认）。 [默认值]
 * 1: 在 Intel CPU 类型的 CPUID leaf 0x80000007 中报告 Invariant TSC 位。

### BOX64_RESERVE_HIGH

为程序保留高内存区域，在 Box32 上始终启用。

 * 0: 不为程序保留高内存区域。 [默认值]
 * 1: 为程序保留高内存区域。

### BOX64_SDL2_JGUID

对包装的 SDL2 使用 SDL_GetJoystickGUIDInfo 函数的变通方案。

 * 0: 不执行任何操作。 [默认值]
 * 1: 对包装的 SDL2 使用 SDL_GetJoystickGUIDInfo 函数的变通方案。

### BOX64_SHAEXT

暴露 SHAEXT（即 SHA_NI）能力。 在 WowBox64 中可用。

 * 0: 不暴露 SHAEXT 能力。
 * 1: 暴露 SHAEXT 能力。 [默认值]

### BOX64_SSE_FLUSHTO0

SSE Flush to 0 FLAGS的行为，同时追踪 SSE 异常FLAGS。 在 WowBox64 中可用。

 * 0: 仅追踪FLAGS。 [默认值]
 * 1: 直接应用 SSE Flush to 0 FLAGS。同时在 DynaRec 中反映 SSE 异常FLAGS（如果可用）。

### BOX64_SSE42

暴露 SSE4.2 能力。 在 WowBox64 中可用。

 * 0: 不暴露 SSE4.2 能力。
 * 1: 暴露 SSE4.2 能力。 [默认值]

### BOX64_STEAM_VULKAN

为 Linux Steam UI 强制使用 Vulkan。

 * 0: 不执行任何操作。 [默认值]
 * 1: 为 Linux Steam UI 强制使用 Vulkan。

### BOX64_SYNC_ROUNDING

在 x86 和原生之间同步舍入模式。

 * 0: 不同步舍入模式。 [默认值]
 * 1: 在 x86 和原生之间同步舍入模式。

### BOX64_UNITYPLAYER

检测 UnityPlayer 并应用保守设置。

 * 0: 不执行任何操作。
 * 1: 检测 UnityPlayer（Windows 或 Linux），并在检测到时应用 BOX64_UNITY=1。 [默认值]

### BOX64_UNITY

告诉 Box64 这是一个 Unity 游戏。

 * 0: 不执行任何操作。
 * 1: 这是 Unity 游戏，对 Windows 使用特殊的检测代码，对 Linux 应用 BOX64_DYNAREC_强内存模型=1。 [默认值]

### BOX64_X11GLX

是否强制 Xorg GLX 扩展存在。

 * 0: 不强制 Xorg GLX 扩展存在。 [默认值]
 * 1: 在使用 XQueryExtension 时要求 Xorg GLX 扩展存在。

### BOX64_X11SYNC

是否强制 X11 显示同步操作。

 * 0: 不强制 X11 显示同步操作。 [默认值]
 * 1: 强制 X11 显示同步操作。

### BOX64_X11THREADS

加载 X11 时调用 XInitThreads。这主要用于使用 Loki_Compat 库的旧 Loki 游戏。

 * 0: 不调用 XInitThreads。 [默认值]
 * 1: 一旦加载 libX11 就调用 XInitThreads。

### BOX64_X87_NO80BITS

x87 80 位 long double 的行为。 在 WowBox64 中可用。

 * 0: 尝试尽可能精确地处理 80 位 long double。 [默认值]
 * 1: 对 x87 使用 64 位 double。

### BOX64_NOPERSONA32BITS

禁用 32 位二进制文件的自动 personality 切换（在 Box32 上始终启用）。

 * 0: 为 32 位二进制文件切换 personality（如果尚未尝试）。 [默认值]
 * 1: 不为 32 位二进制文件切换 personality。

## 参数

### BOX64_ARGS

传递给目标程序的参数，仅在没有任何已有参数时有效。

 * XXXX: 将 XXXX 参数传递给目标程序。
 * XXXX YYYY ZZZZ: 将 XXXX、YYYY 和 ZZZZ 参数传递给目标程序。

### BOX64_INSERT_ARGS

在命令行前添加参数。

 * XXXX: 在程序前添加参数 XXXX。
 * XXXX YYYY ZZZZ: 在目标程序前添加参数 XXXX、YYYY 和 ZZZZ。

### BOX64_INPROCESSGPU

向目标程序添加 --in-process-gpu 参数。

 * 0: 不执行任何操作。 [默认值]
 * 1: 向目标程序添加 --in-process-gpu 参数。

### BOX64_NOSANDBOX

向目标程序添加 --no-sandbox 参数。

 * 0: 不执行任何操作。 [默认值]
 * 1: 向目标程序添加 --no-sandbox 参数。

## 库

### BOX64_ADDLIBS

添加额外需要的共享库（这个选项不太常用）。

 * XXXX: 将 XXXX 共享库添加到所需库列表。
 * XXXX:YYYY:ZZZZ: 将 XXXX、YYYY 和 ZZZZ 共享库添加到所需库列表。

### BOX64_ALLOWMISSINGLIBS

是否允许忽略缺失的共享库。

 * 0: 不允许忽略缺失的共享库。 [默认值]
 * 1: 允许忽略缺失的共享库。

### BOX64_EMULATED_LIBS

强制使用模拟库。

 * XXXX: 强制使用模拟库 XXXX。
 * XXXX:YYYY:ZZZZ: 强制使用模拟库 XXXX、YYYY 和 ZZZZ。

### BOX64_LD_LIBRARY_PATH

查找 x86_64 库的路径。

 * XXXX: 将路径 XXXX 添加到库搜索路径。

### BOX64_LD_PRELOAD

强制随二进制文件加载库。

 * XXXX: 强制加载库 XXXX。
 * XXXX:YYYY: 强制加载库 XXXX 和 YYYY。

### BOX64_LIBGL

设置 libGL 的名称。

 * libXXXX: 将 libGL 的名称设置为 libXXXX。
 * /path/to/libXXXX: 将 libGL 的名称和路径设置为 /path/to/libXXXX，你也可以使用 SDL_VIDEO_GL_DRIVER。

### BOX64_NOGTK

不加载包装的 GTK 库。

 * 0: 加载包装的 GTK 库。 [默认值]
 * 1: 不加载包装的 GTK 库。

### BOX64_NOPULSE

不加载 PulseAudio 库（原生库和模拟库均不加载）。

 * 0: 加载 PulseAudio 库。 [默认值]
 * 1: 不加载 PulseAudio 库。

### BOX64_NOVULKAN

不加载 Vulkan 库。

 * 0: 加载 Vulkan 库。 [默认值]
 * 1: 不加载 Vulkan 库，原生库和模拟库均不加载。

### BOX64_NOVULKANOVERLAY

不加载 x64 Vulkan 覆盖层。

 * 0: 尝试加载 x64 Vulkan 覆盖层。 [默认值]
 * 1: 不加载 x64 Vulkan 覆盖层。

### BOX64_PREFER_EMULATED

优先使用模拟库而非原生库。

 * 0: 优先使用原生库而非模拟库。 [默认值]
 * 1: 优先使用模拟库而非原生库。

### BOX64_PREFER_WRAPPED

即使库使用绝对路径指定，也优先使用包装库。

 * 0: 优先使用绝对路径指定的库而非包装库。 [默认值]
 * 1: 即使库使用绝对路径指定，也优先使用包装库。

### BOX64_WRAP_EGL

优先使用包装库处理 EGL 和 GLESv2。

 * 0: 优先使用模拟库处理 EGL 和 GLESv2。 [默认值]
 * 1: 优先使用包装库处理 EGL 和 GLESv2。

## 环境变量

### BOX64_ENV

添加环境变量。

 * XXXX=yyyy: 添加值为 yyyy 的环境变量 XXXX。

### BOX64_ENV[1-5]

添加环境变量。

 * XXXX=yyyy: 添加值为 yyyy 的环境变量 XXXX。

## 调试

### BOX64_DLSYM_ERROR

启用或禁用 dlsym 错误的日志记录。

 * 0: 禁用 dlsym 错误的日志记录。 [默认值]
 * 1: 启用 dlsym 错误的日志记录。

### BOX64_DUMP

是否输出 elfloader 调试信息。 在 WowBox64 中可用。

 * 0: 不输出 elfloader 调试信息。 [默认值]
 * 1: 输出 elfloader 调试信息。

### BOX64_DYNAREC_DUMP

启用 DynaRec 转储。 在 WowBox64 中可用。

 * 0: 不转储代码块。 [默认值]
 * 1: 转储代码块。
 * 2: 转储代码块并带有颜色。

### BOX64_DYNAREC_DUMP_RANGE

转储指定范围内的代码块。 在 WowBox64 中可用。

 * 0xXXXXXXXX-0xYYYYYYYY: 定义代码块 被转储的范围（左闭右开）。

### BOX64_DYNAREC_GDBJIT

GDBJIT 调试支持，仅在使用 `-DGDBJIT=ON` 构建时可用，通过 gdb 命令启用：jit-reader-load /usr/local/lib/libbox64gdbjitreader.so。

 * 0: DynaRec 不会生成 GDBJIT 调试信息。 [默认值]
 * 1: DynaRec 将生成 GDBJIT 调试信息。
 * 2: DynaRec 将生成包含内部状态的详细 GDBJIT 调试信息。
 * 3: DynaRec 将仅对目标程序陷入的代码块 注册详细的 GDBJIT 调试信息，大幅提升性能。
 * 0xXXXXXXX-0xYYYYYYY: 定义 DynaRec 将生成包含内部状态的详细 GDBJIT 调试信息的范围。

### BOX64_DYNAREC_LOG

禁用或启用 DynaRec 日志。 在 WowBox64 中可用。

 * 0: 禁用 DynaRec 日志。 [默认值]
 * 1: 启用最小级别的 DynaRec 日志。
 * 2: 启用调试级别的 DynaRec 日志。
 * 3: 启用详细级别的 DynaRec 日志。

### BOX64_DYNAREC_MISSING

打印缺失的指令。 在 WowBox64 中可用。

 * 0: 不打印缺失的指令。 [默认值]
 * 1: 打印缺失的指令。
 * 2: 打印回退到标量指令的信息，仅对 RISC-V 有效。

### BOX64_DYNAREC_NOHOSTEXT

禁用可选的主机扩展。

 * 0: 不禁用任何扩展。 [默认值]
 * 1: 禁用所有可选扩展。
 * xxx,yyy,zzz: 禁用指定的可选扩展，可选项包括 Arm：`crc32,pmull,aes,atomics,sha1,sha2,uscat,flagm,flagm2,frintts,afp,rndr`，LoongArch：`lasx,lbt,frecipe,lam_bh,lamcas,scq`，以及 RISC-V：`zba,zbb,zbc,zbs,vector,xtheadba,xtheadbb,xtheadbs,xtheadmemidx,xtheadmempair,xtheadcondmov`。

### BOX64_DYNAREC_PERFMAP

为 Linux perf 工具生成映射文件。

 * 0: DynaRec 不会生成 perf 映射。 [默认值]
 * 1: DynaRec 将生成 perf 映射。

### BOX64_DYNAREC_TEST

启用 DynaRec 与解释器的执行对比，非常慢，仅用于测试。 在 WowBox64 中可用。

 * 0: 不进行对比。 [默认值]
 * 1: 每个指令都在解释器和 DynaRec 上运行，当寄存器或内存不同时进行对比并打印。
 * 0xXXXXXXXX-0xYYYYYYYY: 定义 DynaRec 测试的范围（左闭右开）。

### BOX64_DYNAREC_TEST_NODUP

启用 COSIM 发现差异的 IP 地址去重。 在 WowBox64 中可用。

 * 0: 显示所有出错的地址。 [默认值]
 * 1: 每个出错地址仅显示一次，即使转储可能不同。

### BOX64_DYNAREC_TEST_NODUMP

当 BOX64_DYNAREC_DUMP 启用时不转储测试代码。 在 WowBox64 中可用。

 * 0: 转储测试代码。
 * 1: 不转储测试代码。 [默认值]

### BOX64_DYNAREC_TRACE

启用或禁用 DynaRec 追踪。

 * 0: 不追踪代码块。 [默认值]
 * 1: 追踪代码块，会大幅降低程序速度并生成大量日志。

### BOX64_JITGDB

发生 SIGSEGV、SIGILL 和 SIGBUS 时是否启动 gdb。

 * 0: 捕获信号时仅打印消息。 [默认值]
 * 1: 当捕获到 SIGSEGV、SIGILL 或 SIGBUS 信号时启动 gdb，附加到有问题的进程并进入无限循环等待。在 gdb 中，你需要自己找到正确的线程（在其堆栈中有 `my_box64signalhandler` 的线程），然后可能需要 `finish` 1 或 2 个函数（在 `usleep(..)` 内部），之后你将进入 `my_box64signalhandler`，就在 Segfault 消息的 printf 之前。然后 `set waiting=0` 退出无限循环。
 * 2: 当捕获到 SIGSEGV、SIGILL 或 SIGBUS 信号时启动 gdbserver，附加到有问题的进程，并进入无限循环等待。使用 `gdb /PATH/TO/box64` 然后 `target remote 127.0.0.1:1234` 连接到 gdbserver（如果不在本机，则使用实际 IP）。之后的操作步骤与 `BOX64_JITGDB=1` 相同。
 * 3: 当捕获到 SIGSEGV、SIGILL 或 SIGBUS 信号时启动 lldb，附加到有问题的进程并进入无限循环等待。

### BOX64_LOAD_ADDR

设置程序加载的地址，仅对 PIE 目标程序有效。

 * 0xXXXXXXXX: 设置程序加载的地址。

### BOX64_LOG

启用或禁用 Box64 日志，如果 stdout 不是终端则默认为 0，否则为 1。 在 WowBox64 中可用。

 * 0: 禁用 Box64 日志。
 * 1: 启用最小级别的 Box64 日志。
 * 2: 启用调试级别的 Box64 日志。
 * 3: 启用详细级别的 Box64 日志。

### BOX64_NOBANNER

禁用 Box64 横幅。 在 WowBox64 中可用。

 * 0: 显示 Box64 横幅。
 * 1: 不显示 Box64 横幅。

### BOX64_NODYNAREC

在指定的地址范围内禁止创建代码块，有助于调试 DynaRec 和解释器之间的行为差异。 在 WowBox64 中可用。

 * 0xXXXXXXXX-0xYYYYYYYY: 定义禁止创建代码块 的范围（左闭右开）。

### BOX64_NODYNAREC_DELAY

延迟后移除 NoDynaRec 范围。 在 WowBox64 中可用。

 * 0: 不移除 NoDynaRec 范围。 [默认值]
 * 1: 创建 2 个线程后移除 NoDynaRec 范围。

### BOX64_NOSIGSEGV

禁用 SIGSEGV 的处理。

 * 0: 允许 x86 程序为 SIGSEGV 设置信号处理器。 [默认值]
 * 1: 禁用 SIGSEGV 的处理。

### BOX64_NOSIGILL

禁用 SIGILL 的处理。

 * 0: 允许 x86 程序为 SIGILL 设置信号处理器。 [默认值]
 * 1: 禁用 SIGILL 的处理。

### BOX64_ROLLING_LOG

当捕获信号时显示最近几个包装函数调用。

 * 0: 不执行任何操作。 [默认值]
 * 1: 捕获信号时显示最近 16 个包装函数调用。
 * XXXX: 捕获信号时显示最近 XXXX 个包装函数调用。

### BOX64_SHOWBT

捕获信号时显示原生和模拟的回溯。

 * 0: 不执行任何操作。 [默认值]
 * 1: 捕获信号时显示原生和模拟的回溯。

### BOX64_SHOWSEGV

始终显示 SIGSEGV 信号详情。

 * 0: 不执行任何操作。 [默认值]
 * 1: 始终显示 SIGSEGV 信号详情。

### BOX64_TRACE_COLOR

启用或禁用彩色追踪输出。

 * 0: 禁用彩色追踪输出。 [默认值]
 * 1: 启用彩色追踪输出。

### BOX64_TRACE_EMM

启用或禁用 EMM（即 MMX）追踪输出。

 * 0: 禁用 EMM 追踪输出。 [默认值]
 * 1: 启用 EMM 追踪输出。

### BOX64_TRACE_FILE

将所有日志和追踪发送到文件而非 `stdout`。

 * XXXX: 将所有日志和追踪发送到文件 XXXX。
 * XXXX%pid: 将所有日志和追踪发送到文件 XXXX，文件名中附加 pid。
 * stderr: 将所有日志和追踪发送到 `stderr`。 [默认值]
 * stdout: 将所有日志和追踪发送到 `stdout`。

### BOX64_TRACE_INIT

与 BOX64_TRACE 相同，但立即开始追踪。

 * 0: 禁用追踪输出。 [默认值]
 * 1: 启用追踪输出。追踪在依赖项初始化之前开始。
 * symbolname: 仅对 `symbolname` 启用追踪输出。追踪在依赖项初始化之前开始。
 * 0xXXXXXXX-0xYYYYYYY: 对指定地址范围（左闭右开）启用追踪输出。追踪在依赖项初始化之前开始。

### BOX64_TRACE_START

在 N 个指令执行后开始追踪。

 * 0: 立即开始追踪。 [默认值]
 * 1: 执行 1 个指令后开始追踪。
 * XXXX: 执行 XXXX 个指令后开始追踪。

### BOX64_TRACE_XMM

启用或禁用 XMM（即 SSE）追踪输出。

 * 0: 禁用 XMM 追踪输出。 [默认值]
 * 1: 启用 XMM 追踪输出。

### BOX64_TRACE

仅在带追踪功能的 box64 构建中可用。添加所有执行指令的追踪以及寄存器转储。

 * 0: 禁用追踪输出。 [默认值]
 * 1: 启用追踪输出。
 * symbolname: 仅对 `symbolname` 启用追踪输出。
 * 0xXXXXXXX-0xYYYYYYY: 对指定地址范围（左闭右开）启用追踪输出。

