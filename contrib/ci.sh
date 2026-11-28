#!/usr/bin/env bash
set -euo pipefail

log() { local color=$1; shift; if [ -t 2 ]; then echo "$(tput setaf "$color")$*$(tput sgr0)" >&2; else echo "$*" >&2; fi; }
info() { log 2 "[info] $*"; }
error() { log 1 "[error] $*"; exit 2; }
usage() {
    cat >&2 <<EOF
Build and test box64 in a container, locally or in CI.

  $0 <distro> <arch> <libc> <toolchain> <command> [args...]

  distro libc toolchain  alpine musl gcc|llvm
                         gentoo glibc|musl gcc|llvm

  arch                   x86_64 aarch64 riscv64 loongarch64 ppc64le

  image         build the image from .github/docker/<distro>.Dockerfile
                (UPDATE=1 pulls the base image)

  build         configure and build box64 (BOX32=ON, CMAKE_ARGS, BUILD_DIR)

  test [args]   ctest with the dynarec, then with the interpreter (not on x86_64),
                args go to ctest, e.g. -R printf

  bash [cmd]    run cmd or run smoke test if none given

  run <cmd>     run a command in the container

  install       install the image's packages (used by the dockerfiles)

e.g.  $0 alpine riscv64 musl llvm image
      BOX32=ON $0 alpine riscv64 musl llvm build
EOF
    exit 2
}

# --- configuration ----------------------------------------------------------

[ $# -ge 5 ] || usage
distro=$1 arch=$2 libc=$3 toolchain=$4 command=$5
shift 5

case $distro/$libc/$toolchain in
    alpine/musl/gcc | alpine/musl/llvm) ;;
    gentoo/glibc/gcc)  stage3=latest ;;
    gentoo/glibc/llvm) stage3=llvm ;;
    gentoo/musl/gcc)   stage3=musl ;;
    gentoo/musl/llvm)  stage3=musl-llvm ;;
    *) error "unsupported target: $distro $libc $toolchain" ;;
esac

case $arch in
    x86_64)      cmake_flags="-DLD80BITS=1 -DNOALIGN=1" ;;
    aarch64)     cmake_flags="-DARM64=1" ;;
    riscv64)     cmake_flags="-DRV64=ON" ;;
    loongarch64) cmake_flags="-DLARCH64=ON" ;;
    ppc64le)     cmake_flags="-DPPC64LE=ON -DBAD_SIGNAL=ON" ;;
    *) error "unsupported arch: $arch" ;;
esac

# a foreign arch (in "install": foreign to the image) is cross-compiled
cross=false
if [ "$arch" != "$(uname -m)" ]; then
    cross=true
    case $distro/$libc/$toolchain in
        alpine/musl/llvm) ;;
        *) error "$distro $libc $toolchain cannot cross-compile for $arch, use alpine musl llvm" ;;
    esac
fi

repo=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
image=box64-$distro-$libc-$toolchain:$arch
build_dir=${BUILD_DIR:-build-$distro-$libc-$toolchain-$arch}
box32=${BOX32:-OFF}

# --- install, run by the Dockerfiles ----------------------------------------

toolchain_file=/usr/share/cmake/box64-ci.cmake
triple=${arch/ppc64le/powerpc64le}-alpine-linux-musl

install_alpine() {
    local tools=(cmake git make python3)
    local libs=(musl-dev linux-headers argp-standalone libucontext-dev musl-fts-dev
                musl-legacy-error musl-obstack-dev libncursesw)
    case $toolchain in
        gcc)  tools+=(gcc) ;;
        # compiler-rt: clang's builtins, used instead of libgcc
        llvm) tools+=(clang lld llvm) libs+=(compiler-rt) ;;
    esac

    apk add --no-cache "${tools[@]}"
    if $cross; then
        # apk installs the target's libraries into /sysroot, nothing is emulated
        apk add --no-cache "qemu-$arch"
        apk add --no-cache --initdb --root /sysroot --arch "$arch" \
            --keys-dir "/usr/share/apk/keys/$arch" \
            --repositories-file /etc/apk/repositories \
            "${libs[@]}"
        # clang looks for the target's builtins in its own resource dir
        local resource_dir
        resource_dir=$(clang -print-resource-dir)
        cp -a "/sysroot$resource_dir/lib/." "$resource_dir/lib/"
    else
        apk add --no-cache "${libs[@]}"
    fi

    if [ "$arch/$toolchain" = ppc64le/llvm ]; then rebuild_argp; fi
}

# Workaround: Alpine's libargp.a is built with gcc -Os, which on ppc64 calls
# out-of-line FPR save/restore routines (_restfpr_*) that GNU ld synthesizes
# but lld does not. Rebuild the installed version with clang, like Alpine's
# APKBUILD does. Drop this once lld or Alpine handles it.
rebuild_argp() {
    local root=/ version src
    if $cross; then root=/sysroot; fi
    version=$(apk list --root "$root" --installed argp-standalone |
              sed -n 's/^argp-standalone-\([^-]*\)-r.*/\1/p')
    src=/tmp/argp-standalone-$version
    info "rebuilding argp-standalone $version with clang"

    apk add --no-cache --virtual .argp-build autoconf automake
    wget -q -O- "https://github.com/argp-standalone/argp-standalone/archive/refs/tags/$version.tar.gz" |
        tar -xz -C /tmp
    (
        cd "$src"
        autoreconf -if
        # -fgnu89-inline: what Alpine's gnu89-inline.patch adds
        ./configure --host="$triple" \
            CC="clang --target=$triple --sysroot=$root" \
            CFLAGS="-O2 -fPIC -fgnu89-inline" \
            LDFLAGS="-fuse-ld=lld -rtlib=compiler-rt"
        make -j"$(nproc)" AR=llvm-ar RANLIB=llvm-ranlib libargp.a
        install -m644 libargp.a "$root/usr/lib/libargp.a"
    )
    rm -r "$src"
    apk del --no-cache .argp-build
}

# libc and toolchain come with the stage3 image
install_gentoo() {
    local packages=(dev-build/cmake dev-vcs/git)
    if [ "$libc" = musl ]; then
        # glibc extensions as standalone libraries on musl
        packages+=(sys-libs/argp-standalone sys-libs/error-standalone sys-libs/fts-standalone
                   sys-libs/libucontext sys-libs/obstack-standalone)
    fi

    eselect profile show
    echo 'FEATURES="getbinpkg -ipc-sandbox -network-sandbox -pid-sandbox"' >> /etc/portage/make.conf
    # git without rust, which would pull in rust-bin and gcc on llvm profiles
    echo 'dev-vcs/git -rust' > /etc/portage/package.use/box64
    MAKEOPTS="-j$(nproc)" emerge --oneshot --verbose --changed-use "${packages[@]}"
    rm -rf /var/cache/distfiles/* /var/cache/binpkgs/* /var/tmp/portage/*
}

# the Dockerfiles point CMAKE_TOOLCHAIN_FILE here
write_toolchain_file() {
    local cc=${toolchain/llvm/clang}
    {
        echo "set(CMAKE_C_COMPILER $cc)"
        echo "set(CMAKE_ASM_COMPILER $cc)"
        if [ "$toolchain" = llvm ]; then
            echo 'set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld -rtlib=compiler-rt")'
            echo 'set(CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=lld -rtlib=compiler-rt")'
        fi
        if $cross; then
            printf '%s\n' \
                "set(CMAKE_SYSTEM_NAME Linux)" \
                "set(CMAKE_SYSTEM_PROCESSOR $arch)" \
                "set(CMAKE_C_COMPILER_TARGET $triple)" \
                "set(CMAKE_ASM_COMPILER_TARGET $triple)" \
                "set(CMAKE_SYSROOT /sysroot)" \
                "set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)" \
                "set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)" \
                "set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)" \
                "set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)"
        fi
    } | tee "$toolchain_file"
}

cmd_install() {
    if $cross; then
        info "installing $distro $libc $toolchain, cross-compiling for $arch"
    else
        info "installing $distro $libc $toolchain for $arch"
    fi
    "install_$distro"
    write_toolchain_file
    git config --system --add safe.directory '*'
}

# --- commands, run on the host ----------------------------------------------

# docker run in the image, as the calling user, with the repository in /src
docker_run() {
    local args=(--rm --user "$(id -u):$(id -g)" -e HOME=/tmp -v "$repo:/src" -w /src)
    # rootless podman maps the user to a subordinate uid, which cannot write to the mounted tree
    if [ "$(docker info --format '{{.Host.Security.Rootless}}' 2>/dev/null)" = true ]; then
        args+=(--userns=keep-id)
    fi
    if [ -t 0 ] && [ -t 1 ]; then args+=(-it); fi
    # runTest.cmake runs the tests through $INTERPRETER
    if $cross; then args+=(-e INTERPRETER="qemu-$arch" -e QEMU_LD_PREFIX=/sysroot); fi
    docker run "${args[@]}" "$image" "$@"
}

cmd_image() {
    local args=(--build-arg ARCH="$arch" --build-arg LIBC="$libc" --build-arg TOOLCHAIN="$toolchain")
    if [ -n "${stage3:-}" ]; then args+=(--build-arg STAGE3="$stage3"); fi
    if [ -n "${UPDATE:-}" ]; then args+=(--pull); fi
    info "building $image"
    docker build "${args[@]}" -t "$image" -f "$repo/.github/docker/$distro.Dockerfile" "$repo/contrib"
}

cmd_build() {
    local cc=${toolchain/llvm/clang} ld=ld
    if [ "$toolchain" = llvm ]; then ld=ld.lld; fi
    # shellcheck disable=SC2016
    docker_run sh -c '. /etc/os-release && echo "$PRETTY_NAME"; for tool; do "$tool" --version | head -1; done' \
        sh "$cc" "$ld" cmake

    info "building box64 for $arch in $build_dir (Box32: $box32)"
    # shellcheck disable=SC2086
    docker_run cmake -S . -B "$build_dir" $cmake_flags \
        -DCMAKE_BUILD_TYPE=Release -DBOX32="$box32" -DCI=ON ${CMAKE_ARGS:-}
    docker_run cmake --build "$build_dir" -j"$(nproc)"
}

cmd_test() {
    # runTest.cmake forces BOX64_LOG=0, but segfaults and backtraces still go to
    # stderr, which it prints for failing tests
    local env=(BOX64_SHOWSEGV=1 BOX64_SHOWBT=1)
    local ctest=(ctest --test-dir "$build_dir" -j"$(nproc)" --timeout 300
                 --output-on-failure --no-tests=error)

    case $arch in loongarch64 | ppc64le) ctest+=(-E nocosim) ;; esac
    ctest+=("$@")

    info "ctest"
    docker_run env "${env[@]}" "${ctest[@]}"
    # x86_64 has no dynarec, the first run already used the interpreter
    if [ "$arch" != x86_64 ]; then
        info "ctest with the interpreter (BOX64_DYNAREC=0)"
        docker_run env "${env[@]}" BOX64_DYNAREC=0 "${ctest[@]}"
    fi
}

# cmd_bash [cmd...]: run cmd in the x86_64 bash, without cmd the smoke test
cmd_bash() {
    local qemu=()
    if $cross; then qemu=("qemu-$arch"); fi
    local bash=(env BOX64_NOBANNER=1 "${qemu[@]}" "$build_dir/box64" tests/box64-bash)
    if [ $# -gt 0 ]; then
        docker_run "${bash[@]}" -c "$*"
        return
    fi

    local out expected=$'hello from bash 5\n00042\n/tmp'
    # shellcheck disable=SC2016
    out=$(docker_run "${bash[@]}" -c 'echo "hello from bash ${BASH_VERSINFO[0]}"; printf "%05d\n" 42; cd /tmp && pwd')
    printf '%s\n' "$out"
    [ "$out" = "$expected" ] || error "bash smoke test: expected $(printf '%q' "$expected")"
}

case $command in
    install) cmd_install ;;
    image)   cmd_image ;;
    build)   cmd_build ;;
    test)    cmd_test "$@" ;;
    bash)    cmd_bash "$@" ;;
    run)     docker_run "$@" ;;
    *)       usage ;;
esac
