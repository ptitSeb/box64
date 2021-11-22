Compiling
----
#### for RK3399

Using a 64bits OS:
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

#### for PI4

Warning, you need a 64bits OS:

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

Using a 64bits OS:

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

#### for Snapdragon 845

Using a 64bits OS:

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

Using a 64bits OS:
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

#### for Other ARM64 Linux platforms

 `mkdir build; cd build; cmake .. -DARM_DYNAREC=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo; make -j$(nproc)`

#### for LoongArch

Using a 64bits OS:

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

#### for PowerPC 64 LE

Using a 64bits OS:

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

#### for x86_64 Linux

 `mkdir build; cd build; cmake .. -DLD80BITS=1 -DNOALIGN=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make -j$(nproc)`

If you encounter some linking errors, try activating `NOLOADADDR` (`cmake -DNOLOADADDR=ON; make -j$(nproc)`).

### use ccmake

Alternatively, you can **use the curses-bases ccmake (or any other gui frontend for cmake)** to select wich platform to use interactively.

### Customize your build

*Use ccache if you have it* 

Add `-DUSE_CCACHE=1` if you have ccache (it's better if you plan to touch the sources)

*To have some debug info* 

The `-DCMAKE_BUILD_TYPE=RelWithDebInfo` argument makes a build that is both optimized for speed, and has debug information embedded. That way, if you have a crash or try to analyse performance, you'll have some symbols.

*To have a Trace Enabled build* 

To have a trace enabled build ( ***the interpretor will be slightly slower***), add `-DHAVE_TRACE=1` but you will need, at runtime, to have the [Zydis library](https://github.com/zyantific/zydis) library in your `LD_LIBRARY_PATH` or in the system library folders.

*To have ARM Dynarec*

The Dynarec is only available on the ARM architecture(Right now, anyways.). Notes also that VFPv3 and NEON are required for the Dynarec. Activate it by using `-DARM_DYNAREC=1`.

*Not building from a git clone*

If you are not building from a git clone (for example, downloading a release source zipped from github), you need to activate `-DNOGIT=1` from cmake to be able to build (normal process include git sha1 of HEAD in the version that box64 print).

----

Testing
----
A few tests are included.
They can be launched with `ctest`
They are very basic and don't test much for now.
