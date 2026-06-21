#!/usr/bin/env sh
# Bootstrap and run the project build script.
set -eu
ROOT="$(CDPATH= cd -- "$(dirname "$0")" && pwd)"
cd "$ROOT"

NOB_SRC="${NOB_SRC:-nob.c}"

if [ ! -f ./nob ] && [ ! -f ./nob.exe ]; then
    if command -v gcc >/dev/null 2>&1; then
        gcc -o nob "$NOB_SRC"
    else
        cc -o nob "$NOB_SRC"
    fi
fi

if [ -f ./nob.exe ]; then
    exec ./nob.exe "$@"
fi
exec ./nob "$@"
