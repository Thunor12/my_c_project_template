# Contributing to the dependency catalog

This repo is a **communal C + nob.h dependency playbook**. When you adopt a new library in a project, add it here once so future projects can reuse the bundle.

## Template repo workflow

Committed `nob.c`, `nob_config.h`, and `config/nob_dep_*.h` are generated from `*.jinja` with **every dep in `config/catalog.yaml`**:

```bash
python scripts/render_project.py --sync-maintainer
```

Same codegen path as Copier and `DEPS=… ./scripts/render-output-project.sh` — no special maintainer mode or dual build scripts.

## Adding a dependency

1. **Create a feature bundle** under `config/features/<name>/`:
   - `<name>.h` — `NOB_CONFIG_HAS_<NAME>`, paths, version pins
   - `<name>.build.h` — `<name>_setup_deps()` + `<name>_build_lib()`
   - `dep.toml` — kind (`vendor` / `submodule`), url, checksum or commit

2. **Add a demo** at `examples/<name>_demo.c` (optional but recommended).

3. **Wire shared hooks** (one-time per dep, not per project):
   - `config/catalog.yaml` — dep metadata for codegen
   - `*.jinja` — loop over `optional_deps` (no dep names in templates)
   - Run `python scripts/render_project.py --sync-maintainer` after changing jinja/catalog

4. **Enable path** for existing projects:
   - `scripts/enable-<name>.sh` (+ `.ps1`) — copy bundle → `config/enabled/`, fetch sources, rebuild nob

5. **Copier** (`copier.yml`):
   - Add name to `optional_deps.choices` (must match `config/catalog.yaml`)
   - `_tasks` run `scripts/render_project.py` — no per-dep edits in `*.jinja`

6. **Codegen** (`config/catalog.yaml` + `*.jinja`):
   - Add dep entry to `catalog.yaml` (setup/build fn, example link mode, paths)
   - Templates loop `{% for dep in optional_deps %}` — never hardcode dep names in jinja
   - Preview: `DEPS=<name> ./scripts/render-output-project.sh` → `output_project/`

7. **Dogfood** in this repo:
   - `./scripts/render-all-deps.sh` or `--sync-maintainer`
   - `./nob setup` and `./nob examples`

8. **CI** — extend `scripts/test-template-system.sh` or add a `build-template-*` job.

## Layout reference

```
config/catalog.yaml      # dep list + codegen metadata
config/features/<name>/  # bundle
*.jinja                  # generic templates (loop optional_deps)
nob.c                    # synced via --sync-maintainer (all catalog deps)
nob_config.h             # synced via --sync-maintainer
config/nob_dep_*.h       # synced via --sync-maintainer
config/enabled/*.h       # synced via --sync-maintainer (committed in template repo)
```

Generated projects stay thin: they inherit hooks via `nob_dep_*.h` when a feature is enabled in `config/enabled/`.
