#!/bin/sh

TOP="$(dirname "$(readlink -f "$0")")"

find "$TOP"/src -type f | xargs clang-format -i
