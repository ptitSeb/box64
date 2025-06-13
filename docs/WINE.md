# The guide on using Wine with Box64

<ins>W</ins>ine <ins>I</ins>s <ins>N</ins>ot an <ins>E</ins>mulator, but Box64 is. Box64 supports Wine in multiple ways. Before continuing reading, you should have a basic understanding of Wine.

## Wine variants

Wine ships in multiple forms:


1. **x86 Wine**

    The executable architecture of **x86 Wine** is x86, and it runs x32 Windows executables.

    **x86 Wine** requires a 32-bit environment (read 32-bit Linux libraries), which is usually available on x86_64 machines because of the nature of the architecture.

    However, when you try to run it via an emulator on RISC architectures, the situation is a bit different. For Arm64, there is armhf, which is a 32-bit environment, and there is also Box86, which is made for armhf, so you can use Box86 to run **x86 Wine**.

    However, for RISC-V 64, LoongArch64, and many newer Arm64 cores, a 32-bit environment is not available. Box64 introduced a new mode called Box32, which can be enabled at compile time via a CMake option `BOX32`. It essentially emulates a 32-bit environment using the 64-bit environment, so it can be used to run **x86 Wine**. But be aware that Box32 is still highly experimental and unstable.

2. **x86_64 Wine**

    The executable architecture of **x86_64 Wine** is x86_64, and it runs x64 Windows executables.

    It can be used with Box64. Note that **x86_64 Wine** usually ships with **x86 Wine** included. So you can use a combination of Box86 + Box64 or Box64 alone with Box32 enabled to run both x32 and x64 executables seamlessly.

3. **x86_64 Wine WOW64**

    The executable architecture of **x86_64 Wine WOW64** is x86_64, and it runs both x32 and x64 Windows executables.

    What's great about **x86_64 Wine WOW64** is that it does not require a 32-bit environment for x32 Windows executable, as the 32-bit environment emulation is taken care of by Wine WOW64 itself.

    That means you can just use Box64 (without Box32) to run x32 Windows executables. However, the downside is that the performance of WineD3D is pretty bad for 32-bit (use DXVK!).

4. **Arm64 Wine WOW64**

    The executable architecture of **Arm64 Wine WOW64** is Arm64, and it runs
    x32, x64, Arm32, Arm64 Windows executables.

    However, to run x32 and x64 ones, a cpu.dll is required, known as the 
    WOW64 (for 32-bit) and Arm64EC (for 64-bit).

    Box64 now supports 32-bit WOW64 DLL. You can have it compiled by enabling the `WOW64` CMake option. And we plan to support Arm64EC in the future.

## Wine installation

For x86 Wines, there are plenty of places to download, to name a few:

- https://github.com/Kron4ek/Wine-Builds (Wine \[WOW64\])
- https://github.com/GloriousEggroll/proton-ge-custom (Proton, known as the Valve-optimized Wine)

After the download, just add the `bin` folder to your path in any way you like; Wine should work with Box64 just fine.

To install components like DXVK, d3dcompiler_xx, or cjkfonts, it's recommended to use Winetricks: https://github.com/Winetricks/winetricks

For Arm64 Wines, use of Hangover is recommended: https://github.com/AndreRH/hangover.