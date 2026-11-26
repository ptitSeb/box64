# box64 CI image, set up by contrib/ci.sh (build context: contrib/)
FROM alpine:latest
ARG ARCH LIBC TOOLCHAIN

RUN apk add --no-cache bash
COPY ci.sh /tmp/
RUN bash /tmp/ci.sh alpine "$ARCH" "$LIBC" "$TOOLCHAIN" install && rm /tmp/ci.sh

ENV CMAKE_TOOLCHAIN_FILE=/usr/share/cmake/box64-ci.cmake
WORKDIR /src
CMD ["/bin/bash"]
