#!/bin/bash

# create necessary directories
mkdir -p ~/steam/tmp # makes both ~/steam and ~/steam/tmp in one cmd
cd ~/steam/tmp

# download latest deb and unpack
curl -LO https://cdn.cloudflare.steamstatic.com/client/installer/steam.deb
ar x steam.deb
tar xf data.tar.xz

# move deb contents to steam folder
mv ./usr/* ..
cd .. && rm -rf ./tmp

# create run script
echo '#!/bin/bash
export STEAMOS=1
export STEAM_RUNTIME=1
export PROTON_USE_WOW64=1
export DBUS_FATAL_WARNINGS=0
~/steam/bin/steam $@' > steam

# make script executable and move
chmod +x steam
sudo mv steam /usr/local/bin/

echo "Script complete."
