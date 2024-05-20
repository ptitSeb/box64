Compiling/Installing
----

### Debian-based Linux 
You can use [@ryanfortner](https://github.com/ryanfortner)'s apt repository to install precompiled box64 debs, updated every 24 hours. 

```
sudo wget https://ryanfortner.github.io/box64-debs/box64.list -O /etc/apt/sources.list.d/box64.list
wget -qO- https://ryanfortner.github.io/box64-debs/KEY.gpg | sudo gpg --dearmor -o /etc/apt/trusted.gpg.d/box64-debs-archive-keyring.gpg
sudo apt update && sudo apt install box64-arm64 -y
```

Alternatively, you can generate your own package using the [instructions below](https://github.com/ptitSeb/box64/blob/main/docs/COMPILE.md#debian-packaging). 

----

### The general approach is:
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
- You can use `make -j1`, `make -j2` to prevent running out of memory
- You can also add `-DBAD_SIGNAL=ON` to the cmake command if you are on Linux Kernel mixed with Android, like on RK3588.

#### For Instance, if you want to build box64 for Generic ARM64 Linux platforms, it would look like this:
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

Using a 64bit OS:
```
-D RK3399=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for RK3588 / RK3588S

Using a 64bit OS:
```
-D RK3588=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for Raspberry Pi 3

Warning, you need a 64bit OS:

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

Warning, you need a 64bit OS:

```
-D RPI4ARM64=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for Raspberry Pi 5

```
-D RPI5ARM64=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for TEGRA X1

Using a 64bit OS:

```
-D TEGRAX1=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for Jetson Xavier/T194

Using a 64bit OS:

```
-D TEGRA_T194=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for Jetson Orin/T234

Using a 64bit OS:

Caution: please use gcc-11 or higher, older gcc dosen't know cortex-a78ae
```
-D TEGRA_T234=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for ODROID N2/N2+

Using a 64bit OS:

```
-D ODROIDN2=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for Snapdragon

Using a 64bit OS:

```
-D SD845=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

or

```
-D SD888=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

Depending how recent your Snapdragon is

#### for Phytium

Using a 64bit OS:
```
-D PHYTIUM=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for ADLink machines

Using a 64bit OS:
```
-D ADLINK=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for M1

Only test on Asahi with Fedora, using the default "16K page" kernel

```
-D M1=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for LoongArch

Using a 64bit OS:

```
-D LARCH64=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for RISC-V

Using a 64bit OS:

```
-D RV64=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for PowerPC 64 LE

Using a 64bit OS:

```
-D PPC64LE=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for LX2160A

Using a 64bit OS:

```
-D LX2160A=1 -D CMAKE_BUILD_TYPE=RelWithDebInfo
```

#### for Termux

You must have ARM64 Device for build box64.

##### in CHRoot/PRoot

```
-D ARM64=1 -DCMAKE_C_COMPILER=gcc -DBAD_SIGNAL=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

##### in Termux (Native)

NOTE: BUILDING BOX64 IN TERMUX NATIVE IS EXPERIMENTAL AND DOESN'T GONNA RUN LINUX BINARIES IN NATIVE TERMUX BOX64!!!

You also needed have `libandroid-sysv-semaphore` and `libandroid-spawn` libraries:

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

Alternatively, you can **use the curses-bases ccmake (or any other gui frontend for cmake)** to select wich platform to use interactively.

### Customize your build

#### Use ccache if you have it

Add `-DUSE_CCACHE=1` if you have ccache (it's better if you plan to touch the sources)

#### To have some debug info

The `-DCMAKE_BUILD_TYPE=RelWithDebInfo` argument makes a build that is both optimized for speed, and has debug information embedded. That way, if you have a crash or try to analyse performance, you'll have some symbols.

#### To have a Trace Enabled build 

To have a trace enabled build (***the interpreter will be slightly slower***), add `-DHAVE_TRACE=1`. But you will need to have the [Zydis library](https://github.com/zyantific/zydis) in your `LD_LIBRARY_PATH` or in the system library folders at runtime. Use version v3.2.1, as later version changed the API and will no longer work with box64

#### To have ARM Dynarec

Dynarec is only available on ARM (for the meantime), Activate it by using `-DARM_DYNAREC=1`.

#### To have a build using less memory

You can use `-DSAVE_MEM` to have a build that will try to save some memory. It's, for now, only increasing the jumptable from 4 level to 5 levels. The added granularity avoid wasting space, but the 1 level more to the jumptable means there is on read from memory more when jumping between blocks.

#### Not building from a git clone

If you are not building from a git clone (for example, downloading a release source code zip from github), you need to use `-DNOGIT=1` from cmake to be able to build (box64 uses git SHA1 to show last commit in version number).

#### Use faster linker

You need to add `-DWITH_MOLD=1` if GNU ld is extremely slow. Then run `mold -run make -j4` to build (make sure [Mold](https://github.com/rui314/mold) is installed).

#### Build a statically linked box64

You can now build box64 staticaly linked, with `-DSTATICBUILD`. This is to use inside an x86_64 chroot. Note that this version of box64 will have just the minimum of wrapped libs. So only libc, libm and libpthread basically are wrapped. Other libs (like libGL or libvulkan, SDL2, etc...) will not be wrapped and x86_64 version will be used. It's designed to be used in docker image, or in headless server.
Also, the Static Build is highly experimental, but feedback are always welcomed.

----

Testing
----
A few tests are included with box64.

They can be launched using the `ctest` command.

The tests are very basic and only tests some functionality for now.

----

Debian Packaging
----
Box64 can also be packaged into a .deb file ***using the source code zip from the releases page*** with `DEB_BUILD_OPTIONS=nostrip dpkg-buildpackage -us -uc -nc`. Configure any additional cmake options you might want in `debian/rules`.
