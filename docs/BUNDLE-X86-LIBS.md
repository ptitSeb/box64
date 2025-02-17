# Bundling x86 Libraries

The script `box64-bundle-x86-libs.sh` is provided to download, extract, and re-bundle x86_64 and x86 libraries for use with Box64/Box32 and Box86. These libraries improve Box emulation. This bundle provides a much smaller size compared to using a full root file system of an operating system.

## Usage

### Administrator

Install required dependencies for the bundle script.

- Debian
    ```
    sudo apt-get update
    sudo apt-get install coreutils curl binutils rpm2cpio tar zstd
    ```

Run `box64-bundle-x86-libs.sh`. This will create two archives:
- `box64-bundle-x86-libs.tar.gz` = All of the extracted library files in the directory structure of `usr/lib/box64-i386-linux-gnu` and `usr/lib/box64-x86_64-linux-gnu`.
- `box64-bundle-x86-pkgs.tar.gz` = All of the Linux distribution packages used to extract the library files. This is only created for preservation purposes and is otherwise unused.

Extract the library archive. Box will automatically search these paths for library files to emulate.

```
sudo tar --extract --file box64-bundle-x86-libs.tar.gz --directory /
```

### Developer

Add new packages to the `box64-bundle-x86-libs.csv` file in the format of `<PACKAGE_URL>,<SHA256_CHECKSUM>`.

Supported packages:
- DEB (Debian)
- EOPKG (Solus)
- RPM (Fedora)
- XBPS (Void Linux)

Preference of operating system packages that provide a library:
1. RPMs from AlmaLinux 9 provide 10 years of updates.
2. DEBs from Debian 11 provide 5 years of updates.
3. EOPKG from Solus provide old variants of obscure 32-bit libraries.
4. XBPS from Void Linux provide updated variants of obscure 32-bit libraries.
5. Other (whereever a library is packaged)

For finding package names that contian a specific library file, use the [pkgs.org](https://pkgs.org/) website. Otherwise, use one of these package manager commands on a x86_64 Linux distribution:
- DEB
    - `apt-file <LIBRARY_FILE>`
- EOPKG
    - Does not support file search. Use pkgs.org instead.
- RPM
    - `dnf provides <LIBRARY_FILE>`
- XBPS
    - `xbps-query --repository --ownedby <LIBRARY_FILE>`

Once the package name is found, find the exact URL for downloading it.
- DEB = Explore "main" and "contrib". Search for packages with the name "deb11" in it. Use the "amd64.deb" and "i386.deb" packages.
    - https://archive.debian.org/debian/pool/
- EOPKG = Use the "`<VERSION>-<RELEASE>-x86_64.eopkg`" and "`32bit-<VERSION>-<RELEASE>-x86_64.eopkg`" packages.
    - https://cdn.getsol.us/repo/shannon/
- RPM = Navigate to the "`<MAJOR>.<MINOR>`" version directory first. Then explore "AppStream" and "BaseOS". Use the "x86_64.rpm" and "i686.rpm" packages.
    - https://repo.almalinux.org/almalinux/
- XBPS = There are too many files in the repository to display on a web browser. Use pkgs.org instead to find the exact URL for downloading it.
    - https://repo-default.voidlinux.org/current/

Download the package and then find the checksum with the `sha256sum` command.

Major Linux distributions such as AlmaLinux and Debian provide archives of packages. Rolling distributions such as Arch Linux, Solus, and Void Linux do not. In those cases, use the [Internet Archive](https://web.archive.org/) to archive any rolling release package used by Box by using the "Save Page Now" feature. If the error "Save Page Now browser crashed" appears, it can be safely ignored. The archive will still have been saved.
