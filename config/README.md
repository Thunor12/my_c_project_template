# Build configuration

## `nob_config.h` (root)

Core paths and toolchain flags. Optional features are included from `config/enabled/*.h`.

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

Activated copies (gitignored). Created by:

- `scripts/enable-*.sh` (copies from `config/features/`)
- Copier render (`include_sqlite` / `include_teapot`)

## Dependency commands

```bash
./nob setup          # fetch all enabled optional deps
./nob setup sqlite   # download + verify SQLite amalgamation
./nob setup teapot   # git submodule update --init third_party/teapot
```

Required dependency only: `git submodule update --init third_party/nob.h`

## Shared nob hooks

| File | Role |
|------|------|
| `nob_macros.h` | `HANDLE_COM_RES`, `MARK_BUILT` |
| `nob_dep_setup.h` | `nob_run_setup_all()` — extend when adding a dep |
| `nob_dep_examples.h` | `nob_build_dep_examples()` — extend when adding a demo |

Both `nob.c` (skeleton) and `nob.template.c` (maintainer) include these.

## Adding a feature

See [CONTRIBUTING.md](../CONTRIBUTING.md). Summary:

1. `config/features/myfeature/` — bundle (`*.h`, `*.build.h`, `dep.toml`)
2. Wire `nob_dep_setup.h` + `nob_dep_examples.h` + `nob.template.c` `setup` CLI
3. `scripts/enable-myfeature.sh` + `copier.yml`
4. Dogfood with `nob.template.c` in this repo
