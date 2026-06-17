# c_project_template

A **GitHub + Copier** template for C projects using [nob.h](https://github.com/tsoding/nob.h).

**Primary goal:** centralize how optional C dependencies are fetched, built, and enabled — add each dep once here, reuse in every future project.

## Two build scripts

| File | Purpose |
|------|---------|
| **`nob.c`** | Skeleton shipped to generated projects — `main`, smoke test, `hello` example, hooks for optional deps |
| **`nob.template.c`** | Template-repo maintainer build — dogfoods the full `config/features/` catalog |

```bash
./nob.sh test          # uses nob.template.c in this repo, nob.c in generated projects
NOB_SRC=nob.c ./nob.sh # force skeleton
```

Copier **excludes** `nob.template.c` from generated output.

## Two ways to start a project

### A) GitHub template

```bash
git clone --recurse-submodules https://github.com/<you>/<repo>.git
cd <repo>
gcc -o nob nob.c && ./nob test
```

You get the skeleton `nob.c`. Add features with `scripts/enable-*.sh`.

### B) Copier (pick features at generation)

```bash
copier copy https://github.com/<you>/c_project_template.git my_app --trust --defaults
```

**Windows:** run Copier from **Git Bash** or **WSL** (`_tasks` use `mkdir`, `cp`, `grep`). After generation with teapot:

```bash
git init
git submodule update --init third_party/teapot
```

| Question | Effect |
|----------|--------|
| `include_sqlite` | SQLite bundle, demo, enable scripts → `config/enabled/` |
| `include_teapot` | teapot bundle, demo, `.gitmodules` + submodule init → `config/enabled/` |

## Dependency catalog

```
config/features/<name>/     self-contained bundle (*.h, *.build.h, dep.toml)
config/enabled/             activated copies (gitignored)
config/nob_dep_setup.h      shared ./nob setup logic
config/nob_dep_examples.h   shared optional demo builds
```

```bash
./nob setup           # fetch all enabled optional deps (skeleton + template)
./nob examples        # hello + enabled dep demos
```

Per-target setup (`./nob setup sqlite`, etc.) is available in **`nob.template.c`** (template repo only).

See [CONTRIBUTING.md](CONTRIBUTING.md) for adding a new dependency to the catalog.

## Required dependency

Only **`third_party/nob.h`** (git submodule):

```bash
git submodule update --init third_party/nob.h
```

## Bootstrap

```bash
gcc -o nob nob.c           # generated project
gcc -o nob nob.template.c  # template repo maintainer
./nob test
./nob                      # build main
./nob examples             # hello + optional dep demos
```

## CI

- **Skeleton** — `gcc -o nob nob.c`, test, `hello` example (no `nob.template.c` required)
- **Template + SQLite** — `enable-sqlite.sh`, catalog examples
- **Template + teapot** — `enable-teapot.sh`, builds `teapot_hello`, HTTP probe on `/hello`

Runs on all branch pushes and pull requests.

## License

MIT — [LICENSE](LICENSE).
