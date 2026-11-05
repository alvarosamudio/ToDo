ARG BASE_IMAGE=linuxdeepin/deepin:crimson
FROM ${BASE_IMAGE} AS builder

ENV DEBIAN_FRONTEND=noninteractive

WORKDIR /build

# Las dependencias de compilacion se leen de debian/control, asi el CI
# siempre instala exactamente lo que declara Build-Depends.
COPY debian/ /build/debian/
RUN apt-get update && \
    apt-get install -y --no-install-recommends build-essential dpkg-dev fakeroot && \
    apt-get build-dep -y --no-install-recommends ./ && \
    rm -rf /var/lib/apt/lists/*

COPY . /build

RUN dpkg-buildpackage -us -uc -b && \
    mkdir -p /output && \
    cp ../*.deb /output/

FROM scratch AS export
COPY --from=builder /output/ /
