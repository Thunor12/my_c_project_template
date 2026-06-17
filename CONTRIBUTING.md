# Contributing to the dependency catalog

This repo is a **communal C + nob.h dependency playbook**. When you adopt a new library in a project, add it here once so future projects can reuse the bundle.

## Two build scripts

| File | Who uses it |
|------|-------------|
| `nob.c` | Generated projects — skeleton (~main, test, hello example, optional dep hooks) |
| `nob.template.c` | This template repo — dogfoods every bundle under `config/features/` |

`./nob.sh` builds `nob.template.c` when present, otherwise `nob.c`. Copier excludes `nob.template.c` from generated output.

## Adding a dependency

1. **Create a feature bundle** under `config/features/<name>/`:
   - `<name>.h` — `NOB_CONFIG_HAS_<NAME>`, paths, version pins
   - `<name>.build.h` — `<name>_setup_deps()` + `<name>_build_lib()`
   - `dep.toml` — kind (`vendor` / `submodule`), url, checksum or commit

2. **Add a demo** at `examples/<name>_demo.c` (optional but recommended).

3. **Wire shared hooks** (one-time per dep, not per project):
   - `config/nob_dep_setup.h` — call `<name>_setup_deps()` in `nob_run_setup_all()`
   - `config/nob_dep_examples.h` — build the demo in `nob_build_dep_examples()`
   - `nob.template.c` — add `setup <name>` CLI branch

4. **Enable path** for existing projects:
   - `scripts/enable-<name>.sh` (+ `.ps1`) — copy bundle → `config/enabled/`, fetch sources, rebuild nob

5. **Copier** (`copier.yml`):
   - `include_<name>` question
   - `_exclude` unused bundle/scripts/examples
   - `_tasks` copy headers to `config/enabled/`, append `.gitmodules` if submodule

6. **Dogfood** in this repo:
   - `./scripts/enable-<name>.sh`
   - `./nob examples` (via `nob.template.c`)

7. **CI** — add a job or extend `build-template-*` to exercise the new bundle.

## Layout reference

```
config/features/<name>/
  <name>.h
  <name>.build.h
  dep.toml
config/enabled/          # activated copies (gitignored)
config/nob_dep_setup.h   # shared: ./nob setup
config/nob_dep_examples.h # shared: ./nob examples (dep demos)
nob.c                    # skeleton — do not add dep-specific code here
nob.template.c           # maintainer CLI — add setup <name> here
```

Generated projects stay thin: they inherit hooks via `nob_dep_*.h` when a feature is enabled in `config/enabled/`.
