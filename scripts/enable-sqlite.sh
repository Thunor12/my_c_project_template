#!/usr/bin/env sh
# Enable SQLite: activate config/features/sqlite and fetch the amalgamation.
set -eu

ROOT="$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

if [ ! -f config/features/sqlite/sqlite.h ]; then
    echo "config/features/sqlite/ not found"
    exit 1
fi

mkdir -p config/enabled
cp config/features/sqlite/sqlite.h config/enabled/sqlite.h
cp config/features/sqlite/sqlite.build.h config/enabled/sqlite.build.h
echo "activated config/enabled/sqlite.h + sqlite.build.h"

NOB_SRC=nob.c

if command -v gcc >/dev/null 2>&1; then
    gcc -o nob "$NOB_SRC"
elif command -v cc >/dev/null 2>&1; then
    cc -o nob "$NOB_SRC"
else
    echo "gcc/cc not found — rebuild nob manually after installing a compiler"
    exit 1
fi

if [ -f ./nob.exe ]; then
    ./nob.exe setup sqlite
else
    ./nob setup sqlite
fi

echo "sqlite enabled — run: ./nob examples"
