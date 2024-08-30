#!/bin/sh

set -e # exit on error

# if not root rerun as root
if [ "$(whoami)" != "root" ]; then
    exec sudo "$0" "$@"
fi

TOP="$(dirname "$(readlink -f "$0")")"
OPTIONS="-std=c99 -O0 -g -Wall -Wextra -Wpedantic -fsanitize=address,undefined"

mkdir -p "$TOP/build"

# TODO(CMHJ): work out why asan is cracking the shits about global constants
gcc -shared -std=c99 -g -o "$TOP"/build/libasciiroids.so "$TOP"/src/asciiroids.c

gcc \
    $OPTIONS \
    -o "$TOP"/build/asciiroids \
    "$TOP"/src/main.c \
    -L "$TOP"/build -l asciiroids \
    -Wl,-rpath,"$TOP"/build

chown root:$(id -g) "$TOP"/build/asciiroids
chmod u+s "$TOP"/build/asciiroids
