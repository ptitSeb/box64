Compiling/Installing
----

If you don't want to compile box64 yourself and prefer to use third-party pre-build version, go to the [end of the document](#pre-built-packages) for alternatives.

You can also generate your own package using the [instructions below](https://github.com/ptitSeb/box64/blob/main/docs/COMPILE.md#debian-packaging). 

Additional installation steps may be necessary when copying only the box64 executable file without running make install in cross-build environments. See [Cross-Compiling](https://github.com/ptitSeb/box64/blob/main/docs/COMPILE.md#Cross-Compiling)

## Per-platform compiling instructions

### The general approach

```
git clone https://github.com/ptitSeb/box64
cd box64
mkdir build; cd build; cmake .. ${OPTIONS}
make -j4  
sudo make install
```
If it's the first install, you also need:
```
sudo systemctl restart systemd-binfmt
```
- You can use `make -j1`, `make -j2` with less jobs to prevent running out of memory
- You can also add `-DBAD_SIGNAL=ON` to the cmake command if you are on a Linux Kernel mixed with Android, like on RK3588.

### Note about Box32

If you want to build Box64 with the Box32 option, you will need to add `-DBOX32=ON` to the cmake command. That will enable 32bits process to be run with Box64.
If you also want binfmt integration on 32bits binaries, you also need to add `-DBOX32_BINFMT=ON` to the cmake command.

### Note about WowBox64

A highly experimantal subproject named WowBox64 has been added as a build option, add `-DWOW64=ON` to the cmake command to enable it.
It will build alongside the regular Box64, and produce a `wowbox64.dll` file in `build/wowbox64-prefix/src/wowbox64-build/` directory.

### Note about binfmt_misc on Android

[binfmt_misc](https://en.wikipedia.org/wiki/Binfmt_misc) is a capability of the Linux kernel which allows arbitrary executable file formats to be recognized and passed to certain user space applications such as Box64 usually used with systemd but systems like Android can't run/use systemd so you will need to register Box64 manually, this will also work inside a CHRoot.
```
sudo mount -t binfmt_misc none /proc/sys/fs/binfmt_misc
sudo echo ':box64:M::\x7fELF\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x3e\x00:\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xfe\xff\xff\xff:/usr/local/bin/box64:' | sudo tee /proc/sys/fs/binfmt_misc/register
```


#### Example of generic ARM64 build for linux platforms

```
git clone https://github.com/ptitSeb/box64
cd box64
mkdir build; cd build; cmake .. -D ARM_DYNAREC=ON -D CMAKE_BUILD_TYPE=RelWithDebInfo
make -j4
sudo make install
sudo systemctl restart systemd-binfmt
```

#### Example of generic ARM64 build for linux platforms with full box32 integration

```
git clone https://github.com/ptitSeb/box64
cd box64
mkdir build; cd build; cmake .. -D ARM_DYNAREC=ON -D CMAKE_BUILD_TYPE=RelWithDebInfo -D BOX32=ON -D BOX32_BINFMT=ON
make -j4
sudo make install
sudo systemctl restart systemd-binfmt
```

----

#### for RK3399

On a 64bit OS:
```
-D RK3399=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for RK3588 / RK3588S

On a 64bit OS:
```
-D RK3588=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for Raspberry Pi 3

On a 64bit OS:

If building on the Pi, you will also need a large swap (3 GB+)
[optionally reduce GPU memory to a minimum (e.g. 16 MB) using `raspi-config`
(and reboot) before starting the build]:

You can use e.g. '`make -j4`' to speed up the build, but on a Pi 3 with 1GB memory you will likely
run out of memory at some point and need to run the build again.
Still, this can be faster if your build is attended.

```
-D RPI3ARM64=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for Raspberry Pi 4

On a 64bit OS:

```
-D RPI4ARM64=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for Raspberry Pi 5

```
-D RPI5ARM64=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for TEGRA X1

On a 64bit OS:

```
-D TEGRAX1=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for Jetson Xavier/T194

On a 64bit OS:

```
-D TEGRA_T194=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for Jetson Orin/T234

On a 64bit OS:

Note: use gcc-11 or higher, older gcc doesn't know cortex-a78ae
```
-D TEGRA_T234=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for ODROID N2/N2+

On a 64bit OS:

```
-D ODROIDN2=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for Snapdragon

On a 64bit OS:

```
-D SD845=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

or

```
-D SD888=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

Depending on how recent your Snapdragon is

#### for Phytium

On a 64bit OS:
```
-D PHYTIUM=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for ADLink machines

On a 64bit OS:
```
-D ADLINK=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for M1

Only test on Asahi with Fedora, using the default "16K page" kernel

```
-D M1=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for LoongArch

On a 64bit OS:

```
-D LARCH64=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for RISC-V

On a 64bit OS:

```
-D RV64=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for PowerPC 64 LE

On a 64bit OS:

```
-D PPC64LE=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for LX2160A

On a 64bit OS:

```
-D LX2160A=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for Termux

You must have ARM64 machine to build box64.

##### in CHRoot/PRoot

```
-D ARM64=1 -DCMAKE_C_COMPILER=gcc -DBAD_SIGNAL=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

##### in Termux (Native)

Note: Box64 in native termux is experimental and won't run linux binaries!

You also need `libandroid-sysv-semaphore` and `libandroid-spawn` libraries.

```
-D TERMUX=1 -DCMAKE_C_COMPILER=clang -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for x86_64 Linux

```
-D LD80BITS=1 -D NOALIGN=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```
If you encounter some linking errors, try using `NOLOADADDR=ON` (`cmake -D NOLOADADDR=ON; make -j$(nproc)`).

----

### Use ccmake

Alternatively, you can **use the curses-based ccmake (or any other gui frontend for cmake)** to select which platform to use interactively.

### Customize your build

#### Use ccache when present

Add `-DUSE_CCACHE=1` option if you have ccache and plan to touch the sources.

#### Include debug information

Add `-DCMAKE_BUILD_TYPE=RelWithDebInfo` option for an optimized build with debug information embedded. That way, if you want to debug a crash or analyze performance, you have symbols.

#### Build with trace

To have a trace enabled build (***the interpreter will be slightly slower***), add `-DHAVE_TRACE=1`. You will need the [Zydis library](https://github.com/zyantific/zydis) devel package installed (`apt install libzydis-dev` on Debian for example).

#### Build DynaRec

Add `-D ARM_DYNAREC=ON` option to enable DynaRec on ARM machines.  
Add `-D RV64_DYNAREC=ON` option to enable DynaRec on RV64 machines.  
Add `-D LARCH64_DYNAREC=ON` option to enable DynaRec on LARCH64 machines.  

#### Save memory at run time

You can use `-DSAVE_MEM` to have a build that will try to save some memory. For now, it only increases the jumptable from 4 levels to 5. The added granularity avoids wasting space, but adds one more read from memory when jumping between blocks.

#### Build outside of a git repo

Box64 uses git SHA1 to show last commit in version number, use `-DNOGIT=1` option when building outside of a git repo (for example, downloading a release source code zip from github).

#### Use faster linker

Add `-DWITH_MOLD=1` option when GNU ld is extremely slow. Then run `mold -run make -j4` to build (make sure [Mold](https://github.com/rui314/mold) is installed).

#### Build a statically linked box64

You can now build box64 statically linked, with `-DSTATICBUILD` to use inside of a x86_64 chroot. Note that this version of box64 will only have the minimal wrapped libs, such as libc, libm and libpthread. Other libs (like libGL or libvulkan, SDL2, etc...) will use x86_64 versions. A static build is intended to be used in a docker image, or in a headless server. It is highly experimental, but feedback is always welcome.

----

## Testing

A few tests are included with box64.

They can be launched using the `ctest` command.

The tests are very basic and only test some functionality for now.

----

## Debian Packaging

Box64 can also be packaged into a .deb file ***using the source code zip from the releases page*** with `DEB_BUILD_OPTIONS=nostrip dpkg-buildpackage -us -uc -nc`. Configure any additional cmake options you want in `debian/rules`.

## Pre-built packages

### Debian-based Linux 

You can use the [Pi-Apps-Coders apt repository](https://github.com/Pi-Apps-Coders/box64-debs) to install precompiled box64 debs, updated every 24 hours. 

```
# check if .list file already exists
if [ -f /etc/apt/sources.list.d/box64.list ]; then
  sudo rm -f /etc/apt/sources.list.d/box64.list || exit 1
fi

# check if .sources file already exists
if [ -f /etc/apt/sources.list.d/box64.sources ]; then
  sudo rm -f /etc/apt/sources.list.d/box64.sources || exit 1
fi

# download gpg key from specified url
if [ -f /usr/share/keyrings/box64-archive-keyring.gpg ]; then
  sudo rm -f /usr/share/keyrings/box64-archive-keyring.gpg
fi
sudo mkdir -p /usr/share/keyrings
wget -qO- "https://pi-apps-coders.github.io/box64-debs/KEY.gpg" | sudo gpg --dearmor -o /usr/share/keyrings/box64-archive-keyring.gpg

# create .sources file
echo "Types: deb
URIs: https://Pi-Apps-Coders.github.io/box64-debs/debian
Suites: ./
Signed-By: /usr/share/keyrings/box64-archive-keyring.gpg" | sudo tee /etc/apt/sources.list.d/box64.sources >/dev/null

sudo apt update
sudo apt install box64-generic-arm -y
```

## Cross-Compiling 
----
### Set Up the Cross-Compiler
For example, to compile Box64 for RISC-V on an x86 machine, you can get prebuilt GNU toolchain from the [riscv-gnu-toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain/releases) 

### Run CMake with Cross-Compilation Options 
Follow the per-platform compilation instructions to configure the CMake options for your target architecture.
In particular, you must specify the cross-compiler. For example:
```
-DCMAKE_C_COMPILER=riscv64-unknown-linux-gnu-gcc  # Or whichever cross-compiler you use
```

Be aware of potential glibc capability errors when choosing a cross-compiler.
To avoid this, run `ldd --version` on your target machine to check its glibc version.

### Running tests with QEMU (optional)
To do a quick check, run:
```
qemu-riscv64 -L path/to/your/riscv64/sysroot box64 --help
```
You can run `dirname $(find -name libc.so.6)` to determine whether sysroot is provided by the prebuilt GNU toolchain or not.

To run CTest-based tests under QEMU:
```
ctest -j$(nproc)
```

### Installing on the Target Machine
After successfully cross-compiling, copy the box64 executable to your RISC-V device. Note that simply copying the binary does not automatically install Box64’s shared libraries. Because `make install` does not run on the target during cross-compilation, libraries required for emulation may be missing.

To resolve this, copy the shared libraries folder from the Box64 repository (`x64lib` or `x86lib`) to your target device, then rename it and place it in the appropriate library search path (e.g., `/usr/lib`).

For `x86_64`:
```shell
$ mv x64lib box64-x86_64-linux-gnu
$ mv box64-x86_64-linux-gnu /usr/lib
```
For `i386`:
```shell
$ mv x86lib box64-i386-linux-gnu
$ mv box64-i386-linux-gnu /usr/lib
```

