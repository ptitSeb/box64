#!/bin/sh
# SPDX-License-Identifier: MIT

set -e

trap cleanup_exit INT TERM EXIT

cleanup_exit()
{
    log "Running cleanup_exit..."
    rm -rf /tmp/box64-bundle.*
}

# progress messages are only printed with --debug
log()
{
    if [ -n "${verbose}" ]; then
        echo "$@"
    fi
}

usage()
{
    cat << EOF
Usage: ${0} [OPTION]

Options:
  --build     Build the x86 library bundle archives from Linux distribution
              packages.
  --install   Install the x86 library bundle from the latest GitHub release.
              The bundle is only downloaded when at least one local file does
              not match the checksums of the latest release. It is extracted
              to the root of the file system or into DESTDIR when that
              environment variable is set.
  --debug     Log every command that is run and every file that is extracted
              or archived. Without it, --build is silent unless it fails. Can
              be combined with --build or --install.
  --help      Show this help message.
EOF
}

# parse the arguments before anything else so that --debug logs everything
mode=""
verbose=""
quiet="q"
for arg in "$@"; do
    case "${arg}" in
        --build|--install)
            mode="${arg}"
            ;;
        --debug)
            set -x
            verbose="v"
            quiet=""
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        *)
            echo "E: Unknown argument (${arg})"
            usage
            exit 1
            ;;
    esac
done
if [ -z "${mode}" ]; then
    echo "E: A --build or --install argument is required"
    usage
    exit 1
fi

repo_url="https://github.com/ptitSeb/box64"

# compare the local files to the latest release and, on any mismatch, install it
install_bundle()
{
    # find the latest release
    release=$(basename "$(${curl} -Ls -o /dev/null -w '%{url_effective}' "${repo_url}/releases/latest")")
    case "${release}" in
        v*) ;;
        *)
            echo "E: Unable to determine the latest release"
            exit 1
            ;;
    esac
    bundle_url="${repo_url}/releases/download/${release}/box64-bundle-x86-libs-${release}"
    bundle_checksums="${dir_tmp}/box64-bundle-x86-libs-${release}.sha256"
    bundle_archive="${dir_tmp}/box64-bundle-x86-libs-${release}.tar.gz"

    # the checksum paths are relative to the root of the destination
    ${curl_cmd} "${bundle_checksums}" "${bundle_url}.sha256"
    if [ ! -d "${dir_dest}" ]; then
        mkdir -p "${dir_dest}"
    fi
    cd "${dir_dest}"
    if ${sha256sum} --check --quiet "${bundle_checksums}" \
        > "${dir_tmp}/mismatches" 2> /dev/null; then
        echo "I: The x86 library bundle ${release} is already installed in ${dir_dest}"
        return 0
    fi

    # a single mismatch means the whole bundle is downloaded and extracted again
    echo "I: $(wc -l < "${dir_tmp}/mismatches") file(s) do not match the latest bundle"
    if [ ! -w "${dir_dest}" ]; then
        echo "E: ${dir_dest} is not writable, run as root to install the x86 library bundle ${release}"
        exit 1
    fi
    ${curl_cmd} "${bundle_archive}" "${bundle_url}.tar.gz"
    ${tar} --extract --no-same-owner --file "${bundle_archive}" --directory "${dir_dest}"

    # verify the installation
    cd "${dir_dest}"
    if ! ${sha256sum} --check --quiet "${bundle_checksums}"; then
        echo "E: Invalid checksums after installing the x86 library bundle ${release}"
        exit 1
    fi
    echo "I: Installed the x86 library bundle ${release} into ${dir_dest}"
}

# we must have few tools
awk=$(which awk) || { echo "E: You must have awk" && exit 1; }
curl=$(which curl) || { echo "E: You must have curl" && exit 1; }
curl_cmd="${curl} --connect-timeout 5 --retry 5 --retry-delay 1 --create-dirs -fsSLo"
sha256sum=$(which sha256sum) || { echo "E: You must have sha256sum" && exit 1; }
tar=$(which tar) || { echo "E: You must have tar" && exit 1; }

current_dir=$(pwd)
dir_tmp="$(mktemp -d /tmp/box64-bundle.XXXXXX)"
dir_dest="${DESTDIR:-/}"

if [ "${mode}" = "--install" ]; then
    install_bundle
    exit 0
fi

# we must have few more tools to build the bundle
cpio=$(which cpio) || { echo "E: You must have cpio" && exit 1; }
rpm2cpio=$(which rpm2cpio) || { echo "E: You must have rpm2cpio" && exit 1; }
unzip=$(which unzip) || { echo "E: You must have unzip" && exit 1; }

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
tar -C "${dir_tmp}"/bundle-pkgs -cz"${verbose}"f "${current_dir}"/box64-bundle-x86-pkgs.tar.gz .

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
            log "I: DEB (Debian) package detected (${file})"
            ar x "${dir_tmp}"/bundle-pkgs/"${file}"
            tar -xf data.tar*
            ;;
        eopkg)
            log "I: EOPKG (Solus Linux) package detected (${file})"
            ${unzip} -o"${quiet}" "${dir_tmp}"/bundle-pkgs/"${file}"
            tar -xf install.tar.xz
            ;;
        rpm)
            log "I: RPM (Fedora) detected (${file})"
            if [ -n "${verbose}" ]; then
                ${rpm2cpio} "${dir_tmp}"/bundle-pkgs/"${file}" | ${cpio} -idmv
            else
                ${rpm2cpio} "${dir_tmp}"/bundle-pkgs/"${file}" \
                    | ${cpio} -idm --quiet 2> /dev/null
            fi
            ;;
        xbps)
            log "I: XBPS (Void Linux) package detected (${file})"
            tar -xf "${dir_tmp}"/bundle-pkgs/"${file}"
            ;;
    esac
done

# reorganize the files
cd "${dir_tmp}"/bundle-libs
mkdir -p "${dir_tmp}"/bundle-libs/box64-i386-linux-gnu
mv lib/*.so* usr/lib/*.so* usr/lib/i386-linux-gnu/* usr/lib32/*.so* \
    "${dir_tmp}"/bundle-libs/box64-i386-linux-gnu
mkdir -p "${dir_tmp}"/bundle-libs/box64-x86_64-linux-gnu
mv lib64/*.so* usr/lib/x86_64-linux-gnu/* usr/lib64/*.so* \
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
  libpulse-simple libpulse libvulkan ld-linux-x86-64 \
  libtcmalloc_minimal libanl ld-linux libthread_db
for file in "$@"; do
    rm -f "${dir_tmp}"/bundle-libs/usr/lib/box64-*-linux-gnu/"${file}".so*
done

# check broken symlinks
if find "${dir_tmp}"/bundle-libs -type l ! -exec test -e {} \; -print | grep bundle-libs; then
    echo "E: Broken symlinks found"
    exit 1
fi

# Manually create additional symlinks that are useful to have.
cd "${dir_tmp}"/bundle-libs/usr/lib/box64-x86_64-linux-gnu/
ln -s libmbedcrypto.so.7 libmbedcrypto.so.3
ln -s libmbedtls.so.14 libmbedtls.so.12
ln -s libmbedx509.so.1 libmbedx509.so.0

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
        if ! ls "${dir_lib}/${lib_base}" > /dev/null 2>&1; then
            ln -s "${lib}" "${lib_base}"
        fi
    done
done

# generate a SHA256 checksum of every file and symlink in the bundle
cd "${dir_tmp}"/bundle-libs
find usr \( -type f -o -type l \) -exec "${sha256sum}" {} + \
    | LC_ALL=C sort --key 2 > "${current_dir}"/box64-bundle-x86-libs.sha256

# generate the bundle libraries archive
tar -C "${dir_tmp}"/bundle-libs -cz"${verbose}"f "${current_dir}"/box64-bundle-x86-libs.tar.gz .
