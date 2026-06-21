# Build configuration

## `config/catalog.yaml`

Lists every optional dep and codegen metadata (`setup_fn`, `build_lib_fn`, example `link_mode`, paths). **`optional_deps` in Copier and `DEPS=` for local render must use names from this file.**

## `nob_config.h` (root)

Core paths and toolchain flags. Optional features are included from `config/enabled/*.h`.

- **Template repo:** committed `nob.c` / headers are synced via `--sync-maintainer` (all catalog deps).
- **Local render:** `DEPS=sqlite ./scripts/render-output-project.sh` or `./scripts/render-all-deps.sh`.

`nob_go_rebuild_urself_project()` watches every `config/enabled/*.h` (directory scan) so enabling a feature triggers a nob rebuild without hardcoding dep names.

## Feature module layout

Each optional dependency is a **bundle** under `config/features/<name>/`:

| File | Role |
|------|------|
| `feature.h` | Macros, paths, `NOB_CONFIG_HAS_*` |
| `feature.build.h` | `static` lib build + `*_setup_deps()` fetch logic |
| `dep.toml` | Human-readable manifest (kind, url, version, checksum) |

Example: `config/features/sqlite/`, `config/features/teapot/`.

## `config/build_common.h`

Shared types (`CompResult`, `FileList`, `NobBuildCtx`) used by `nob_helpers.h`, `nob.c`, and all `*.build.h` modules.

## `config/enabled/`

Activated copies of feature headers. Created by:

- `scripts/render_project.py` — copies from `config/features/`
- Catalog render (`DEPS=… ./scripts/render-output-project.sh`)

In this template repo, `config/enabled/*.h` is **committed** (matches all-deps `nob.c`). Generated projects gitignore `config/enabled/*` except what render activates locally.

## Dependency commands

```bash
./nob setup          # fetch all enabled optional deps
./nob setup sqlite   # download + verify SQLite amalgamation
./nob setup teapot   # git submodule update --init third_party/teapot
```

Available when the feature is enabled in `config/enabled/` and listed in the generated `nob.c`.

Required dependency only: `git submodule update --init third_party/nob.h`

## Shared nob hooks

| File | Role |
|------|------|
| `nob_macros.h` | `HANDLE_COM_RES`, `MARK_BUILT` |
| `nob_dep_setup.h` | `nob_run_setup_all()` — generated per `optional_deps` |
| `nob_dep_examples.h` | `nob_build_dep_examples()` — generated per `optional_deps` |

Copier and `--sync-maintainer` render `config/nob_dep_*.h` from `*.jinja`.

## Adding a feature

See [CONTRIBUTING.md](../CONTRIBUTING.md). Summary:

1. `config/features/myfeature/` — bundle (`*.h`, `*.build.h`, `dep.toml`)
2. Add entry to `config/catalog.yaml` + `copier.yml` choices
3. Run `--sync-maintainer` and dogfood with `./nob setup` / `./nob examples`
