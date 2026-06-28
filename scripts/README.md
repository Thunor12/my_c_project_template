# Scripts

## Project generation (`output_project/`)

| Script | Purpose |
|--------|---------|
| `render_project.py` | Engine — reads `config/catalog.yaml`, renders `*.jinja` |
| `render-output-project.sh` | Write `output_project/` (set `DEPS=…` or omit for minimal) |
| `render-all-deps.sh` | Write `output_project/` with every catalog dep |
| `test-template-system.sh` | End-to-end render + build tests |
| `render-output-project.ps1` | Windows wrapper |

```bash
pip install -r scripts/requirements-render.txt
DEPS=sqlite ./scripts/render-output-project.sh
cd output_project && gcc -o nob nob.c && ./nob test
```

## Post-hoc enable (template clone / existing project)

Each feature bundle lives in `config/features/<name>/`.

| Script | Copies to `config/enabled/` | Also does |
|--------|------------------------------|-----------|
| `enable-sqlite.sh` | `sqlite.h`, `sqlite.build.h` | Rebuilds `nob`, runs `./nob setup sqlite` |
| `enable-teapot.sh` | `teapot.h`, `teapot.build.h` | Adds `third_party/teapot` submodule, rebuilds `nob` |

After enabling: `./nob examples` (or `./nob setup` to fetch deps without building examples).
