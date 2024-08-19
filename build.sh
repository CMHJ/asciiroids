#!/bin/sh

TOP="$(dirname "$(readlink -f "$0")")"

mkdir -p "$TOP/build"
gcc -std=c99 -o "$TOP"/build/asciiroids "$TOP"/src/main.c
