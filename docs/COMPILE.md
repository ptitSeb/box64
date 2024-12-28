Compiling/Installing
----

If you don't want to compile box64 yourself and prefer to use third-party pre-build version, go to the [end of the document](#pre-build-packages) for alternatives.

You can also generate your own package using the [instructions below](https://github.com/ptitSeb/box64/blob/main/docs/COMPILE.md#debian-packaging). 

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

#### Example of generic ARM64 build for linux platforms

```
git clone https://github.com/ptitSeb/box64
cd box64
mkdir build; cd build; cmake .. -D ARM_DYNAREC=ON -D CMAKE_BUILD_TYPE=RelWithDebInfo
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

To have a trace enabled build (***the interpreter will be slightly slower***), add `-DHAVE_TRACE=1`. You will need the [Zydis library](https://github.com/zyantific/zydis) in your `LD_LIBRARY_PATH` or in the system library folders at runtime to get x86 trace. Use version v3.2.1, as later versions have changed the API and no longer work with box64.

#### Build ARM DynaRec

Add `-DARM_DYNAREC=1` option to enable DynaRec on ARM machines.

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
