#!/usr/bin/env sh
# End-to-end tests for config/catalog.yaml + render_project.py
set -eu

ROOT="$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

TMP="${TMPDIR:-/tmp}/c_project_template_render_test"
rm -rf "$TMP"
mkdir -p "$TMP"

pip install -q -r scripts/requirements-render.txt
python3 scripts/render_project.py --validate-copier

copy_nob_h() {
  mkdir -p "$1/third_party"
  cp -R "$ROOT/third_party/nob.h" "$1/third_party/nob.h"
}

echo "== minimal project =="
python3 scripts/render_project.py --deps "" --output "$TMP/minimal"
test ! -d "$TMP/minimal/config/features/sqlite"
test ! -d "$TMP/minimal/config/features/teapot"
! grep -q 'teapot\|sqlite' "$TMP/minimal/nob.c"
copy_nob_h "$TMP/minimal"
(
  cd "$TMP/minimal"
  gcc -o nob nob.c
  ./nob test
  ./nob
  ./nob examples
  test -f ./build/hello
)

echo "== sqlite project =="
python3 scripts/render_project.py --deps sqlite --output "$TMP/sqlite"
test -d "$TMP/sqlite/config/features/sqlite"
test ! -d "$TMP/sqlite/config/features/teapot"
test ! -e "$TMP/sqlite/third_party/teapot"
! grep -q teapot "$TMP/sqlite/nob.c"
copy_nob_h "$TMP/sqlite"
(
  cd "$TMP/sqlite"
  gcc -o nob nob.c
  ./nob test
  ./nob setup
  ./nob examples
  test -f ./build/sqlite_demo || test -f ./build/sqlite_demo.exe
)

echo "== output_project (default path) =="
./scripts/render-output-project.sh
copy_nob_h output_project
(
  cd output_project
  gcc -o nob nob.c
  ./nob test
)

echo "== template repo: all catalog deps =="
python3 scripts/render_project.py --sync-maintainer
copy_nob_h "$ROOT"
(
  cd "$ROOT"
  gcc -o /tmp/nob_maintainer nob.c
  ./nob test
)

echo "PASS: template system"
