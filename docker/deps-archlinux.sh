#!/bin/bash
sh
set -evu
pacman -Sy --noconfirm \
    sudo \
    sed \
    grep \
    awk \
    fakeroot \
    wget \
    cmake \
    make \
    gcc \
    libffi \
    git \
    jsoncpp \
    libmicrohttpd \
    curl \
    hiredis \
    redis \
    argtable \
    p11-kit