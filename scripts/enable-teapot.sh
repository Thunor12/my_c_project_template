#!/usr/bin/env sh
# Enable teapot: activate config/features/teapot and fetch the submodule.
set -eu

ROOT="$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

if [ ! -f config/features/teapot/teapot.h ]; then
    echo "config/features/teapot/ not found"
    exit 1
fi

mkdir -p config/enabled
cp config/features/teapot/teapot.h config/enabled/teapot.h
cp config/features/teapot/teapot.build.h config/enabled/teapot.build.h
echo "activated config/enabled/teapot.h + teapot.build.h"

if ! grep -q 'third_party/teapot' .gitmodules 2>/dev/null; then
    git submodule add https://github.com/Thunor12/teapot.git third_party/teapot
fi
git submodule update --init --depth 1 third_party/teapot

NOB_SRC=nob.c

if command -v gcc >/dev/null 2>&1; then
    gcc -o nob "$NOB_SRC"
elif command -v cc >/dev/null 2>&1; then
    cc -o nob "$NOB_SRC"
else
    echo "gcc/cc not found — rebuild nob manually after installing a compiler"
    exit 1
fi

echo "teapot enabled — run: ./nob examples"
