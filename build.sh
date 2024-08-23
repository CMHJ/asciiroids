#!/bin/sh

set -e # exit on error

# if not root rerun as root
if [ "$(whoami)" != "root" ]; then
    exec sudo "$0" "$@"
fi

TOP="$(dirname "$(readlink -f "$0")")"

mkdir -p "$TOP/build"
gcc \
    -std=c99 \
    -g \
    -Wall -Wextra -Wpedantic \
    -o "$TOP"/build/asciiroids \
    "$TOP"/src/main.c

chown root:$(id -g) "$TOP"/build/asciiroids
chmod u+s "$TOP"/build/asciiroids
