#!/bin/sh

set -ex

trap cleanup_exit INT TERM EXIT

cleanup_exit()
{
    echo "Running cleanup_exit..."
    rm -rf /tmp/box64-bundle.*
}

# we must have few tools
awk=$(which awk) || { echo "E: You must have awk" && exit 1; }
cpio=$(which cpio) || { echo "E: You must have cpio" && exit 1; }
curl=$(which curl) || { echo "E: You must have curl" && exit 1; }
curl_cmd="${curl} --connect-timeout 5 --retry 5 --retry-delay 1 --create-dirs -fsSLo"
rpm2cpio=$(which rpm2cpio) || { echo "E: You must have rpm2cpio" && exit 1; }
sha256sum=$(which sha256sum) || { echo "E: You must have sha256sum" && exit 1; }
unzip=$(which unzip) || { echo "E: You must have unzip" && exit 1; }

current_dir=$(pwd)
dir_tmp="$(mktemp -d /tmp/box64-bundle.XXXXXX)"

# download the packages
while IFS= read -r line; do
    # shellcheck disable=SC2016
    pkg_url=$(echo "${line}" | ${awk} -F',' '{print$(NF-1)}')
    # shellcheck disable=SC2016
    pkg_checksum=$(echo "${line}" | ${awk} -F',' '{print$(NF)}')
    # shellcheck disable=SC2016
    pkg_name=$(echo "${pkg_url}" | ${awk} -F'/' '{print$(NF)}')
    ${curl_cmd} "${dir_tmp}/bundle-pkgs/${pkg_name}" "${pkg_url}"
    checksum="$(${sha256sum} "${dir_tmp}/bundle-pkgs/${pkg_name}" | awk '{print $1}')"
    if [ "${pkg_checksum}" != "${checksum}" ]; then
        echo "E: Invalid checksum for ${pkg_name}"
        echo "Expected: ${pkg_checksum}"
        echo "Computed: ${checksum}"
        exit 1
    fi
done < box64-bundle-x86-libs.csv

# generate the bundle packages archive
tar -C "${dir_tmp}"/bundle-pkgs -czvf "${current_dir}"/box64-bundle-x86-pkgs.tar.gz .

# extract the packages
cd "${dir_tmp}"/bundle-pkgs
for file in *.deb *.eopkg *.rpm *.xbps; do
    # handle the case of no files, e.g. xbps
    [ -e "${dir_tmp}"/bundle-pkgs/"${file}" ] || break
    # shellcheck disable=SC2016
    extension=$(echo "${file}" | ${awk} -F'.' '{print$(NF)}')
    mkdir -p "${dir_tmp}"/bundle-libs
    cd "${dir_tmp}"/bundle-libs
    case "${extension}" in
        deb)
            echo "I: DEB (Debian) package detected (${file})"
            ar x "${dir_tmp}"/bundle-pkgs/"${file}"
            tar -xf data.tar*
            ;;
        eopkg)
            echo "I: EOPKG (Solus Linux) package detected (${file})"
            ${unzip} -o "${dir_tmp}"/bundle-pkgs/"${file}"
            tar -xf install.tar.xz
            ;;
        rpm)
            echo "I: RPM (Fedora) detected (${file})"
            ${rpm2cpio} "${dir_tmp}"/bundle-pkgs/"${file}" | ${cpio} -idmv
            ;;
        xbps)
            echo "I: XBPS (Void Linux) package detected (${file})"
            tar -xf "${dir_tmp}"/bundle-pkgs/"${file}"
            ;;
    esac
done

# reorganize the files
cd "${dir_tmp}"/bundle-libs
mkdir -p "${dir_tmp}"/bundle-libs/box64-i386-linux-gnu
mv lib/*.so* usr/lib/*.so* usr/lib/i386-linux-gnu/*.so* usr/lib32/*.so* \
    "${dir_tmp}"/bundle-libs/box64-i386-linux-gnu
mkdir -p "${dir_tmp}"/bundle-libs/box64-x86_64-linux-gnu
mv lib64/*.so* usr/lib/x86_64-linux-gnu/*.so* usr/lib64/*.so* \
    "${dir_tmp}"/bundle-libs/box64-x86_64-linux-gnu
rm -f ./*.tar.* debian-binary files.xml metadata.xml
rm -rf ./lib* etc run sbin usr var
mkdir -p usr/lib
mv box64-*-linux-gnu usr/lib

# remove libraries that cannot be emulated
# https://github.com/ptitSeb/box64/blob/v0.3.8/src/librarian/library.c#L433
set -- libc libpthread librt libGL libX11 libasound libdl libm libbsd libutil \
  libresolv libXxf86vm libXinerama libXrandr libXext libXfixes libXcursor \
  libXrender libXft libXi libXss libXpm libXau libXdmcp libX11-xcb libxcb \
  libxcb-xfixes libxcb-shape libxcb-shm libxcb-randr libxcb-image \
  libxcb-keysyms libxcb-xtest libxcb-glx libxcb-dri2 libxcb-dri3 libXtst libXt \
  libXcomposite libXdamage libXmu libxkbcommon libxkbcommon-x11 \
  libpulse-simple libpulse libvulkan ld-linux-x86-64 crashhandler \
  libtcmalloc_minimal libanl ld-linux libthread_db
for file in "$@"; do
    rm -f "${dir_tmp}"/bundle-libs/usr/lib/box64-*-linux-gnu/"${file}".so*
done

# check broken symlinks
if find "${dir_tmp}"/bundle-libs -type l ! -exec test -e {} \; -print | grep bundle-libs; then
    echo "E: Broken symlinks found"
    exit 1
fi

# create shared libraries (.so) symlinks
# if multiple libraries of the same name exist, it will symlink the oldest version
# e.g. libcurl-gnutls soname with versioned symbols (CURL_GNUTLS_3, CURL_GNUTLS_4)
# libcurl-gnutls.so -> libcurl-gnutls.so.3
# libcurl-gnutls.so.3 -> libcurl-gnutls.so.4
# libcurl-gnutls.so.4 -> libcurl-gnutls.so.4.7.0
# libcurl-gnutls.so.4.7.0
for dir_lib in "${dir_tmp}"/bundle-libs/usr/lib/box64-*-linux-gnu; do
    cd "${dir_lib}"
    for lib in lib*.so*; do
        lib_base="$(echo "${lib}" | awk -F'.' '{print $1"."$2}')"
        if ! ls "${dir_lib}/${lib_base}" 2> /dev/null; then
            ln -s "${lib}" "${lib_base}"
        fi
    done
done

# generate the bundle libraries archive
tar -C "${dir_tmp}"/bundle-libs -czvf "${current_dir}"/box64-bundle-x86-libs.tar.gz .
