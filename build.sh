#!/bin/sh

set -e # exit on error

TOP="$(dirname "$(readlink -f "$0")")"
OPTIONS="-std=c99 -O0 -g -Wall -Wextra -Wpedantic -fsanitize=address,undefined"

case "$1" in
release)
    mkdir -p "$TOP/build/release"
    gcc -std=c99 -DRELEASE -g -o "$TOP"/build/release/asciiroids \
        "$TOP"/src/main.c \
        -lm
    strip "$TOP"/build/release/asciiroids
    ;;
*)
    mkdir -p "$TOP/build/debug"

    # TODO(CMHJ): work out why asan is cracking the shits about global constants
    gcc -fPIC -shared -std=c99 -g -o "$TOP"/build/debug/libasciiroids.so "$TOP"/src/asciiroids.c

    gcc \
        $OPTIONS \
        -o "$TOP"/build/debug/asciiroids \
        "$TOP"/src/main.c
    ;;
esac
