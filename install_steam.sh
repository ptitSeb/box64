#!/bin/bash

# create necessary directories
mkdir -p ~/steam
mkdir -p ~/steam/tmp
cd ~/steam/tmp

# download latest deb and unpack
wget https://cdn.cloudflare.steamstatic.com/client/installer/steam.deb
ar x steam.deb
tar xf data.tar.xz

# remove deb archives, not needed anymore
rm ./*.tar.xz ./steam.deb

# move deb contents to steam folder
mv ./usr/* ../
cd ../ && rm -rf ./tmp/

# create run script
echo '#!/bin/bash
export STEAMOS=1
export STEAM_RUNTIME=1
export DBUS_FATAL_WARNINGS=0

# Attempt to prioritize native aarch64 libraries for Box64/Box32 themselves
# and ensure native Vulkan ICDs are found.
export LD_LIBRARY_PATH="/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH}"
export VK_ICD_FILENAMES="/usr/share/vulkan/icd.d/freedreno_icd.aarch64.json:/usr/share/vulkan/icd.d/lvp_icd.aarch64.json${VK_ICD_FILENAMES:+:}${VK_ICD_FILENAMES}"

~/steam/bin/steam $@' > steam

# make script executable and move
chmod +x steam
sudo mv steam /usr/local/bin/

echo "Script complete."
