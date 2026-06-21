#!/usr/bin/env sh
# Generate output_project/ with every dep from config/catalog.yaml.
set -eu

ROOT="$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

DEPS="$(python3 -c "import yaml; print(','.join(yaml.safe_load(open('config/catalog.yaml'))['deps']))")"
exec python3 scripts/render_project.py --deps "$DEPS" --output output_project
