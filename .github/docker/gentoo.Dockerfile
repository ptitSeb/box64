# box64 CI image, set up by contrib/ci.sh (build context: contrib/)
# STAGE3: latest (glibc gcc), llvm (glibc llvm), musl (musl gcc), musl-llvm
ARG STAGE3=latest

# package index: architecture independent, but only published for amd64
FROM --platform=linux/amd64 docker.io/gentoo/portage:latest AS portage
FROM docker.io/gentoo/stage3:${STAGE3}
ARG ARCH LIBC TOOLCHAIN
COPY --from=portage /var/db/repos/gentoo /var/db/repos/gentoo

COPY ci.sh /tmp/
RUN bash /tmp/ci.sh gentoo "$ARCH" "$LIBC" "$TOOLCHAIN" install && rm /tmp/ci.sh

ENV CMAKE_TOOLCHAIN_FILE=/usr/share/cmake/box64-ci.cmake
# Gentoo sets up the toolchain environment in /etc/profile.env
ENTRYPOINT ["/bin/bash", "--login", "-c", "exec \"$@\"", "--"]
WORKDIR /src
CMD ["/bin/bash"]
