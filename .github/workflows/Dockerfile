ARG BASE_IMAGE=linuxdeepin/deepin:crimson
FROM ${BASE_IMAGE}

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        debhelper \
        fakeroot \
        pkg-config \
        qt6-base-dev \
        qt6-l10n-tools \
        libdtk6widget-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY . /build

RUN dpkg-buildpackage -us -uc -b && \
    mkdir -p /output && \
    cp ../*.deb /output/

CMD ["/bin/bash"]
