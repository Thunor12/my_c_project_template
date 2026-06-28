#!/usr/bin/env sh
# Generate output_project/ from config/catalog.yaml.
# Usage:
#   ./scripts/render-output-project.sh              # minimal project (no optional deps)
#   DEPS=sqlite ./scripts/render-output-project.sh
#   DEPS=sqlite,teapot ./scripts/render-output-project.sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

DEPS="${DEPS:-}"
if [ -n "$DEPS" ]; then
  exec python3 scripts/render_project.py --deps "$DEPS" --output output_project
fi
exec python3 scripts/render_project.py --output output_project
