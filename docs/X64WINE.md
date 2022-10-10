# Installing Wine64
Running Wine64/Wine with Box64/Box86 allows x64/x86 Windows programs to run on 64-bit ARM Linux devices.
 - Box64 needs `wine-amd64` to be installed **manually** on ARM64 (aarch64) devices.  This will run 64-bit Windows programs (aka x86_64).
 - Box86 needs `wine-i386` to be installed **manually** on ARM32 (armhf) devices (or on ARM64 with multiarch or chroot).  This will run 32-bit Windows programs (aka x86).
 - _Note that `wine:arm64` and `wine:armhf` will not work with Box64/Box86. Your device's package manager might try to install those if you ask it to install wine._

See installation steps below (in the [Examples](#examples) section).

## Overview
The general procedure for installing Wine64 & Wine for Box64 & Box86 is to...
 - Download all the install files for the version of Wine you wish to install
 - Unzip or dpkg the install files into one folder
 - Move that folder to the directory that you wish Wine to run from (often `~/wine/` by default)
 - Go to `/usr/local/bin` and make symlinks or scripts that will point to your main wine binaries.
 - Boot wine to create a new wineprefix.
 - Download winetricks (which is just a complicated bash script), make it executable, then copy it to `/usr/local/bin`.

## Examples
### Installing Wine64 & Wine for Box64 & Box86 on Raspberry Pi OS from WineHQ .deb files
_Links from the [WineHQ repo](https://dl.winehq.org/wine-builds/debian/dists/)_

This install method allows you to install different versions of Wine64/Wine.  You may install any version/branch of Wine that you wish.
```
	# NOTE: Can only run on aarch64 (since box64 can only run on aarch64)
	# box64 runs wine-amd64, box86 runs wine-i386.

	### User-defined Wine version variables ################
	# - Replace the variables below with your system's info.
	# - Note that we need the amd64 version for Box64 even though we're installing it on our ARM processor.
	# - Note that we need the i386 version for Box86 even though we're installing it on our ARM processor.
	# - Wine download links from WineHQ: https://dl.winehq.org/wine-builds/
  
	local branch="devel" #example: devel, staging, or stable (wine-staging 4.5+ requires libfaudio0:i386)
	local version="7.1" #example: "7.1"
	local id="debian" #example: debian, ubuntu
	local dist="bullseye" #example (for debian): bullseye, buster, jessie, wheezy, ${VERSION_CODENAME}, etc 
	local tag="-1" #example: -1 (some wine .deb files have -1 tag on the end and some don't)

  ########################################################

	# Clean up any old wine instances
	wineserver -k # stop any old wine installations from running
	rm -rf ~/.cache/wine # remove any old wine-mono/wine-gecko install files
	rm -rf ~/.local/share/applications/wine # remove any old program shortcuts

	# Backup any old wine installs
	rm -rf ~/wine-old 2>/dev/null; mv ~/wine ~/wine-old 2>/dev/null
	rm -rf ~/.wine-old 2>/dev/null; mv ~/.wine ~/.wine-old 2>/dev/null
	sudo mv /usr/local/bin/wine /usr/local/bin/wine-old 2>/dev/null
	sudo mv /usr/local/bin/wine64 /usr/local/bin/wine-old 2>/dev/null
	sudo mv /usr/local/bin/wineboot /usr/local/bin/wineboot-old 2>/dev/null
	sudo mv /usr/local/bin/winecfg /usr/local/bin/winecfg-old 2>/dev/null
	sudo mv /usr/local/bin/wineserver /usr/local/bin/wineserver-old 2>/dev/null

	# Wine download links from WineHQ: https://dl.winehq.org/wine-builds/
	LNKA="https://dl.winehq.org/wine-builds/${id}/dists/${dist}/main/binary-amd64/" #amd64-wine links
	DEB_A1="wine-${branch}-amd64_${version}~${dist}${tag}_amd64.deb" #wine64 main bin
	DEB_A2="wine-${branch}_${version}~${dist}${tag}_amd64.deb" #wine64 support files (required for wine64 / can work alongside wine_i386 main bin)
		#DEB_A3="winehq-${branch}_${version}~${dist}${tag}_amd64.deb" #shortcuts & docs
	LNKB="https://dl.winehq.org/wine-builds/${id}/dists/${dist}/main/binary-i386/" #i386-wine links
	DEB_B1="wine-${branch}-i386_${version}~${dist}${tag}_i386.deb" #wine_i386 main bin
	DEB_B2="wine-${branch}_${version}~${dist}${tag}_i386.deb" #wine_i386 support files (required for wine_i386 if no wine64 / CONFLICTS WITH wine64 support files)
		#DEB_B3="winehq-${branch}_${version}~${dist}${tag}_i386.deb" #shortcuts & docs

	# Install amd64-wine (64-bit) alongside i386-wine (32-bit)
	echo -e "Downloading wine . . ."
	wget -q ${LNKA}${DEB_A1} 
	wget -q ${LNKA}${DEB_A2} 
	wget -q ${LNKB}${DEB_B1} 
	echo -e "Extracting wine . . ."
	dpkg-deb -x ${DEB_A1} wine-installer
	dpkg-deb -x ${DEB_A2} wine-installer
	dpkg-deb -x ${DEB_B1} wine-installer
	echo -e "Installing wine . . ."
	mv wine-installer/opt/wine* ~/wine
	
	# Download wine dependencies
	# - these packages are needed for running box86/wine-i386 on a 64-bit RPiOS via multiarch
	sudo dpkg --add-architecture armhf && sudo apt-get update # enable multi-arch
	sudo apt-get install -y libasound2:armhf libc6:armhf libglib2.0-0:armhf libgphoto2-6:armhf libgphoto2-port12:armhf \
		libgstreamer-plugins-base1.0-0:armhf libgstreamer1.0-0:armhf libldap-2.4-2:armhf libopenal1:armhf libpcap0.8:armhf \
		libpulse0:armhf libsane1:armhf libudev1:armhf libusb-1.0-0:armhf libvkd3d1:armhf libx11-6:armhf libxext6:armhf \
		libasound2-plugins:armhf ocl-icd-libopencl1:armhf libncurses6:armhf libncurses5:armhf libcap2-bin:armhf libcups2:armhf \
		libdbus-1-3:armhf libfontconfig1:armhf libfreetype6:armhf libglu1-mesa:armhf libglu1:armhf libgnutls30:armhf \
		libgssapi-krb5-2:armhf libkrb5-3:armhf libodbc1:armhf libosmesa6:armhf libsdl2-2.0-0:armhf libv4l-0:armhf \
		libxcomposite1:armhf libxcursor1:armhf libxfixes3:armhf libxi6:armhf libxinerama1:armhf libxrandr2:armhf \
		libxrender1:armhf libxxf86vm1 libc6:armhf libcap2-bin:armhf # to run wine-i386 through box86:armhf on aarch64
		# This list found by downloading...
		#	wget https://dl.winehq.org/wine-builds/debian/dists/bullseye/main/binary-i386/wine-devel-i386_7.1~bullseye-1_i386.deb
		#	wget https://dl.winehq.org/wine-builds/debian/dists/bullseye/main/binary-i386/winehq-devel_7.1~bullseye-1_i386.deb
		#	wget https://dl.winehq.org/wine-builds/debian/dists/bullseye/main/binary-i386/wine-devel_7.1~bullseye-1_i386.deb
		# then `dpkg-deb -I package.deb`. Read output, add `:armhf` to packages in dep list, then try installing them on Pi aarch64.
		
	# - these packages are needed for running box64/wine-amd64 on RPiOS (box64 only runs on 64-bit OS's)
	sudo apt-get install -y libasound2:arm64 libc6:arm64 libglib2.0-0:arm64 libgphoto2-6:arm64 libgphoto2-port12:arm64 \
		libgstreamer-plugins-base1.0-0:arm64 libgstreamer1.0-0:arm64 libldap-2.4-2:arm64 libopenal1:arm64 libpcap0.8:arm64 \
		libpulse0:arm64 libsane1:arm64 libudev1:arm64 libunwind8:arm64 libusb-1.0-0:arm64 libvkd3d1:arm64 libx11-6:arm64 libxext6:arm64 \
		ocl-icd-libopencl1:arm64 libasound2-plugins:arm64 libncurses6:arm64 libncurses5:arm64 libcups2:arm64 \
		libdbus-1-3:arm64 libfontconfig1:arm64 libfreetype6:arm64 libglu1-mesa:arm64 libgnutls30:arm64 \
		libgssapi-krb5-2:arm64 libjpeg62-turbo:arm64 libkrb5-3:arm64 libodbc1:arm64 libosmesa6:arm64 libsdl2-2.0-0:arm64 libv4l-0:arm64 \
		libxcomposite1:arm64 libxcursor1:arm64 libxfixes3:arm64 libxi6:arm64 libxinerama1:arm64 libxrandr2:arm64 \
		libxrender1:arm64 libxxf86vm1:arm64 libc6:arm64 libcap2-bin:arm64
		# This list found by downloading...
		#	wget https://dl.winehq.org/wine-builds/debian/dists/bullseye/main/binary-amd64/wine-devel_7.1~bullseye-1_amd64.deb
		#	wget https://dl.winehq.org/wine-builds/debian/dists/bullseye/main/binary-amd64/wine-devel-amd64_7.1~bullseye-1_amd64.deb
		# then `dpkg-deb -I package.deb`. Read output, add `:arm64` to packages in dep list, then try installing them on Pi aarch64.	

	# These packages are needed for running wine-staging on RPiOS (Credits: chills340)
	sudo apt install libstb0 -y
	cd ~/Downloads
	wget -r -l1 -np -nd -A "libfaudio0_*~bpo10+1_i386.deb" http://ftp.us.debian.org/debian/pool/main/f/faudio/ # Download libfaudio i386 no matter its version number
	dpkg-deb -xv libfaudio0_*~bpo10+1_i386.deb libfaudio
	sudo cp -TRv libfaudio/usr/ /usr/
	rm libfaudio0_*~bpo10+1_i386.deb # clean up
	rm -rf libfaudio # clean up

	# Install symlinks
	sudo ln -s ~/wine/bin/wine /usr/local/bin/wine
	sudo ln -s ~/wine/bin/wine64 /usr/local/bin/wine64
	sudo ln -s ~/wine/bin/wineboot /usr/local/bin/wineboot
	sudo ln -s ~/wine/bin/winecfg /usr/local/bin/winecfg
	sudo ln -s ~/wine/bin/wineserver /usr/local/bin/wineserver
	sudo chmod +x /usr/local/bin/wine /usr/local/bin/wine64 /usr/local/bin/wineboot /usr/local/bin/winecfg /usr/local/bin/wineserver
```

## Installing winetricks
Winetricks is a bash script which makes it easier to install & configure any desired Windows core system software packages which may be dependencies for certain Windows programs.  You can either install it with `apt`, or manually using the steps below.

```
sudo apt-get install cabextract -y                                                                   # winetricks needs this installed
sudo mv /usr/local/bin/winetricks /usr/local/bin/winetricks-old                                      # Backup old winetricks
cd ~/Downloads && wget https://raw.githubusercontent.com/Winetricks/winetricks/master/src/winetricks # Download
sudo chmod +x winetricks && sudo mv winetricks /usr/local/bin/                                       # Install
```

Whenever we run winetricks, we must tell Box86 to supress its info banner, otherwise winetricks will crash.  Box86's info banner can be suppressed using the `BOX86_NOBANNER=1` environment variable before invoking winetricks (example: `BOX86_NOBANNER=1 winetricks`).

If `apt` installed a desktop menu shortcut for winetricks (or if you make your own desktop shortcut for winetricks) then you may need to edit that shortcut to include Box86's BOX86_NOBANNER=1 env var.  With your favorite text editor, edit `/usr/share/applications/winetricks.desktop` and change `Exec=winetricks --gui` to `Exec=env BOX86_NOBANNER=1 winetricks --gui`.

## Example commands
Creating a 64-bit wineprefix:
`wine64 wineboot` or `wine wineboot` (wineprefix is created in `~/.wine/`)
`WINEPREFIX="$HOME/prefix64" wine wineboot` (wineprefix is created in `~/prefix64/`) _Note: You will need to invoke `WINEPREFIX="$HOME/prefix64"` before every command to use this wineprefix._

Creating a 32-bit wineprefix:
`WINEARCH=win32 wine wineboot` (wineprefix is created in `~/.wine/`)

Fore-quitting wine:
`wineserver -k`

Running Wine Configuration:
`winecfg`

Using winetricks:
`winetricks -q corefonts vcrun2010 dotnet20sp1`  
_This command will silently install three packages in series: Windows core fonts, VC++ 2010 Runtimes, and .NET 2.0 SP1.  `-q` is the "install silent/quiet" command._

Invoking Box86's logging features (with `BOX86_LOG=1` or similar) will cause winetricks to crash.

For a list of all the different Windows packages & libraries that winetricks can help you install, run `winetricks list-all`  

## Other notes

### Wineprefixes (and Wine initialization)
When you first run or boot Wine (`wine wineboot`), Wine will create a new user environment within which to install Windows software.  This user environment is called a "wineprefix" (or "wine bottle") and is located (by default) in `~/.wine` (note that Linux folders with a `.` in front of them are "hidden" folders).  Think of a wineprefix as Wine's virtual 'harddrive' where it installs software and saves settings.  Wineprefixes are portable and deletable.  For more Wine documentation, see [WineHQ](https://www.winehq.org/documentation).

If you at any point corrupt something inside your default wineprefix, you can start "fresh" by deleting your `~/.wine` directory (with the `rm -rf ~/.wine` command) and boot wine again to create a new default wineprefix.

### Transplanting wineprefixes
If software isn't installing in Wine with Box86, but is installing for you in Wine on a regular x86 Linux computer, you can copy a wineprefix from your x86 Linux computer to the device you're running Box86 on. This is most easily done by tarring the `~/.wine` folder on your x86 Linux computer (`tar -cvf winebottle.tar ~/.wine`), transferring the tar file to your device, then un-tarring the tar file (`tar -xf winebottle.tar`) on your device running Box86 & Wine. Tarring the wineprefix preserves any symlinks that are inside it.

### Swapping out different versions of Wine
Some versions of Wine work better with certain software. It is best to install a version of Wine that is known to work with the software you would like to run. There are three main development branches of Wine you can pick from, referred to as wine-stable, wine-devel, and wine-staging. _Note that the wine-staging branch requires extra installation steps on Raspberry Pi._

Your entire Wine installation can reside within a single folder on your Linux computer. TwisterOS assumes that your Wine installation is located inside the `~/wine/` directory. The actual directory where you put your `wine` folder doesn't matter as long as you have symlinks within the `/usr/local/bin/` directory which point to the `wine` folder so that Linux can find Wine when you type `wine` into the terminal).

You can change which version of Wine you are running simply by renaming your old `wine` and `.wine` folders to something else, then putting a new `wine` folder (containing your new version of Wine) in its place. Running `wine wineboot` again will migrate a wineprefix from an older version of Wine to be usable by the newer version of Wine you just installed.  You can check which version of Wine you're running with the `wine --version` command.
