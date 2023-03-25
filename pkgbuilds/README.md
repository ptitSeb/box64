# The directory containing all PKGBUILDs for BOX64
## FAQ
### 1. How to use it?
Just download the `PKGBUILD-<your-platform>` (no need to download the whole repository), go to directory where you downloaded and type:
```sh
mv ./PKGBUILD-<your-platform> ./PKGBUILD
# use the below line if you need a DEBUG build
# sed -i 's/RelWithDebInfo/Debug/g' ./PKGBUILD
cd ..
makepkg -si
```
This will copy the PKGBUILD for your platform (where you type your platform name instead of `<your-platform>`), build it and install the PKGBUILD on 64-bit Archlinux ARM.

In the future, I might try to do an unified PKGBUILD for all platforms that will recognize your platform automatically.

### 2. What platforms are currently supported.
All specified in [COMPILE.md](../docs/COMPILE.md) except for LARCH64, PPC64LE and SW64.

### 3. Any advantages with this solution?
This might give you some advantages like:
- automatic updates (with some AUR frontends like [`pamac-aur`](https://aur.archlinux.org/packages/pamac-aur/) or [`yay`](https://aur.archlinux.org/packages/yay));
- easily uninstall with `pacman -R` when no needed (no need other scripts nor Makefile to do this);
- your `box64` installation is recognized as a package (where it wasn't when installing with `make install`).


### Note: You can use [makedeb](https://github.com/makedeb/makedeb) (or) [pkgbuild2spec](https://github.com/prozum/pkgbuild2spec) to compile box64 to .deb or .rpm installer files.
