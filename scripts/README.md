# Optional feature scripts

Each feature bundle lives in `config/features/<name>/` (`*.h`, `*.build.h`, `dep.toml`).

| Script | Copies to `config/enabled/` | Also does |
|--------|------------------------------|-----------|
| `enable-sqlite.sh` | `sqlite.h`, `sqlite.build.h` | Rebuilds `nob`, runs `./nob setup sqlite` |
| `enable-teapot.sh` | `teapot.h`, `teapot.build.h` | Adds `third_party/teapot` submodule, rebuilds `nob` |

After enabling: `./nob examples` (or `./nob setup` to fetch deps without building examples).
