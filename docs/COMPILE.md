Compiling/Installing
----

#### Debian-based Linux 
You can use [@ryanfortner](https://github.com/ryanfortner)'s apt repository to install precompiled box64 debs, updated every 24 hours. 

```
sudo wget https://ryanfortner.github.io/box64-debs/box64.list -O /etc/apt/sources.list.d/box64.list
wget -O- https://ryanfortner.github.io/box64-debs/KEY.gpg | gpg --dearmor | sudo tee /usr/share/keyrings/box64-debs-archive-keyring.gpg 
sudo apt update && sudo apt install box64 -y
```

Alternatively, you can generate your own package using the [instructions below](https://github.com/ptitSeb/box64/blob/main/docs/COMPILE.md#debian-packaging). 

#### for RK3399

Using a 64bit OS:
```
git clone https://github.com/ptitSeb/box64
cd box64
mkdir build; cd build; cmake .. -DRK3399=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j4
sudo make install
```
If it's the first install, you also need:
```
sudo systemctl restart systemd-binfmt
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
git clone https://github.com/ptitSeb/box64
cd box64
mkdir build; cd build; cmake .. -DRPI3ARM64=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo
make 
# or e.g. make -j4
sudo make install
```
If it's the first install, you also need:
```
sudo systemctl restart systemd-binfmt
```

#### for Raspberry Pi 4

Warning, you need a 64bit OS:

```
git clone https://github.com/ptitSeb/box64
cd box64
mkdir build; cd build; cmake .. -DRPI4ARM64=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j4
sudo make install
```
If it's the first install, you also need:
```
sudo systemctl restart systemd-binfmt
```

#### for TEGRA X1

Using a 64bit OS:

```
git clone https://github.com/ptitSeb/box64
cd box64
mkdir build; cd build; cmake .. -DTEGRAX1=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j4
sudo make install
```
If it's the first install, you also need:
```
sudo systemctl restart systemd-binfmt
```

#### for ODROID N2/N2+

Using a 64bit OS:

```
git clone https://github.com/ptitSeb/box64
cd box64
mkdir build; cd build; cmake .. -DODROIDN2=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j4
sudo make install
```
If it's the first install, you also need:
```
sudo systemctl restart systemd-binfmt
```

#### for Snapdragon 845

Using a 64bit OS:

```
git clone https://github.com/ptitSeb/box64
cd box64
mkdir build; cd build; cmake .. -DSD845=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j4
sudo make install
```
If it's the first install, you also need:
```
sudo systemctl restart systemd-binfmt
```


----
#### for Phytium

Using a 64bit OS:
```
git clone https://github.com/ptitSeb/box64
cd box64
mkdir build; cd build; cmake .. -DPHYTIUM=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j4
sudo make install
```
If it's the first install, you also need:
```
sudo systemctl restart systemd-binfmt
```

#### for M1

Only test on Asahi for now, using the default "16K page" kernel

```
git clone https://github.com/ptitSeb/box64
cd box64
mkdir build; cd build; cmake .. -DM1=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j4
sudo make install
```
If it's the first install, you also need:
```
sudo systemctl restart systemd-binfmt
```


#### for Other ARM64 Linux platforms

 `mkdir build; cd build; cmake .. -DARM_DYNAREC=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo; make -j$(nproc)`

#### for LoongArch

Using a 64bit OS:

```
git clone https://github.com/ptitSeb/box64
cd box64
mkdir build; cd build; cmake .. -DLARCH64=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j4
sudo make install
```
If it's the first install, you also need:
```
sudo systemctl restart systemd-binfmt
```

#### for RISC-V

Using a 64bit OS:

```
git clone https://github.com/ptitSeb/box64
cd box64
mkdir build; cd build; cmake .. -DRV64=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j`nproc`
sudo make install
```
If it's the first install, you also need:
```
sudo systemctl restart systemd-binfmt
```

#### for PowerPC 64 LE

Using a 64bit OS:

```
git clone https://github.com/ptitSeb/box64
cd box64
mkdir build; cd build; cmake .. -DPPC64LE=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j4
sudo make install
```
If it's the first install, you also need:
```
sudo systemctl restart systemd-binfmt
```

#### for LX2160A

Using a 64bit OS:

```
git clone https://github.com/ptitSeb/box64
cd box64
mkdir build; cd build; cmake .. -DLX2160A=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j$(nproc)
sudo make install
```
If it's the first install, you also need:
```
sudo systemctl restart systemd-binfmt
```

#### for x86_64 Linux

 `mkdir build; cd build; cmake .. -DLD80BITS=1 -DNOALIGN=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make -j$(nproc)`

If you encounter some linking errors, try using `NOLOADADDR=ON` (`cmake -DNOLOADADDR=ON; make -j$(nproc)`).

### use ccmake

Alternatively, you can **use the curses-bases ccmake (or any other gui frontend for cmake)** to select wich platform to use interactively.

### Customize your build

#### Use ccache if you have it

Add `-DUSE_CCACHE=1` if you have ccache (it's better if you plan to touch the sources)

#### To have some debug info

The `-DCMAKE_BUILD_TYPE=RelWithDebInfo` argument makes a build that is both optimized for speed, and has debug information embedded. That way, if you have a crash or try to analyse performance, you'll have some symbols.

#### To have a Trace Enabled build 

To have a trace enabled build (***the interpreter will be slightly slower***), add `-DHAVE_TRACE=1`. But you will need to have the [Zydis library](https://github.com/zyantific/zydis) in your `LD_LIBRARY_PATH` or in the system library folders at runtime.

#### To have ARM Dynarec

Dynarec is only available on ARM (for the meantime), Activate it by using `-DARM_DYNAREC=1`.
###### *Note: VFPv3 and NEON are required for Dynarec.*

#### Not building from a git clone

If you are not building from a git clone (for example, downloading a release source code zip from github), you need to use `-DNOGIT=1` from cmake to be able to build (box64 uses git SHA1 to show last commit in version number).

----

Testing
----
A few tests are included with box64.

They can be launched using the `ctest` command.

The tests are very basic and only tests some functionality for now.

----

Debian Packaging
----
Box64 can also be packaged into a .deb file with `DEB_BUILD_OPTIONS=nostrip dpkg-buildpackage -us -uc -nc`.
