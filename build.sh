#!/bin/sh

set -e # exit on error

# if not root rerun as root
if [ "$(whoami)" != "root" ]; then
    exec sudo "$0" "$@"
fi

TOP="$(dirname "$(readlink -f "$0")")"
OPTIONS="-std=c99 -g -Wall -Wextra -Wpedantic -fsanitize=address,undefined"

mkdir -p "$TOP/build"
gcc \
    $OPTIONS \
    -o "$TOP"/build/asciiroids \
    "$TOP"/src/main.c

chown root:$(id -g) "$TOP"/build/asciiroids
chmod u+s "$TOP"/build/asciiroids
