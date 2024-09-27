#!/bin/sh

set -e # exit on error

TOP="$(dirname "$(readlink -f "$0")")"
OPTIONS="-std=c99 -O0 -g -Wall -Wextra -Wpedantic -fsanitize=address,undefined"

mkdir -p "$TOP/build"

# TODO(CMHJ): work out why asan is cracking the shits about global constants
gcc -fPIC -shared -std=c99 -g -o "$TOP"/build/libasciiroids.so "$TOP"/src/asciiroids.c

gcc \
    $OPTIONS \
    -o "$TOP"/build/asciiroids \
    "$TOP"/src/main.c
