# c_project_template

A **GitHub + Copier** template for C projects using [nob.h](https://github.com/tsoding/nob.h).

**Primary goal:** centralize how optional C dependencies are fetched, built, and enabled — add each dep once in `config/catalog.yaml`, reuse in every future project.

## Build script

`nob.c` is **generated** from `config/catalog.yaml` and the selected `optional_deps`. In this template repo it is kept in sync with **all catalog deps** via `--sync-maintainer`.

```bash
./nob.sh test
```

## Generate a project locally (`output_project/`)

```bash
pip install -r scripts/requirements-render.txt
./scripts/render-output-project.sh                    # minimal (no optional deps)
DEPS=sqlite ./scripts/render-output-project.sh        # sqlite only
./scripts/render-all-deps.sh                          # every dep in catalog.yaml
python scripts/render_project.py --sync-maintainer    # refresh repo-root codegen (all deps)
```

Output lands in **`output_project/`** (gitignored). Dep list comes from **`config/catalog.yaml`**.

## Two ways to start a project

### A) GitHub template

```bash
git clone --recurse-submodules https://github.com/<you>/<repo>.git
cd <repo>
gcc -o nob nob.c && ./nob test
```

Add features with `scripts/enable-*.sh` (when the feature bundle is present), or regenerate with `render_project.py`.

### B) Copier (pick deps at generation)

```bash
pip install copier pyyaml jinja2
copier copy https://github.com/<you>/c_project_template.git my_app --trust
```

Select **`optional_deps`** from the catalog (`config/catalog.yaml`). Copier runs `render_project.py` to generate `nob.c`, `nob_config.h`, and `config/nob_dep_*.h`.

After generation with a submodule dep (e.g. teapot):

```bash
git init
git submodule update --init third_party/teapot
```

## Dependency catalog

```
config/catalog.yaml         dep list + codegen metadata (single source of truth)
config/features/<name>/     self-contained bundle (*.h, *.build.h, dep.toml)
config/enabled/             activated copies (committed in template repo; gitignored in generated projects)
```

```bash
./nob setup           # fetch all enabled optional deps
./nob setup <name>    # fetch one enabled dep (when present in generated nob.c)
./nob examples        # hello + enabled dep demos
```

See [CONTRIBUTING.md](CONTRIBUTING.md) for adding a new dependency.

## Required dependency

Only **`third_party/nob.h`** (git submodule):

```bash
git submodule update --init third_party/nob.h
```

## Bootstrap

```bash
gcc -o nob nob.c
./nob test
./nob                      # build main
./nob examples             # hello + optional dep demos
```

## CI

- **test-template-system** — `scripts/test-template-system.sh` (minimal + sqlite renders, copier validation, all-deps sync)
- **Generated Windows** — minimal `output_project/` on MSYS2
- **Template + SQLite / teapot** — all-deps `nob.c`, catalog examples

Runs on pull requests and on pushes to `main`.

## License

MIT — [LICENSE](LICENSE).
