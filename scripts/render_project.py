#!/usr/bin/env python3
"""Generate a project tree from config/catalog.yaml and a list of optional deps."""

from __future__ import annotations

import argparse
import os
import shutil
import sys
from pathlib import Path

try:
    import yaml
    from jinja2 import Environment, FileSystemLoader, select_autoescape
except ImportError as exc:  # pragma: no cover
    print(
        "render_project.py requires PyYAML and Jinja2: pip install pyyaml jinja2",
        file=sys.stderr,
    )
    raise SystemExit(1) from exc

ROOT = Path(__file__).resolve().parent.parent
CATALOG_PATH = ROOT / "config" / "catalog.yaml"

SKIP_DIR_NAMES = {
    ".git",
    "output_project",
    "build",
    "__pycache__",
    ".vscode",
    ".cursor",
}

MAINTAINER_ONLY = {
    "nob.c",
    "nob_config.h",
    "config/nob_dep_setup.h",
    "config/nob_dep_examples.h",
}

TEMPLATE_META = {
    "copier.yml",
    "CONTRIBUTING.md",
    ".github/workflows/ci.yml",
    "scripts/render_project.py",
    "scripts/render-output-project.sh",
    "scripts/render-output-project.ps1",
    "scripts/requirements-render.txt",
}

JINJA_OUTPUTS = (
    ("nob.c.jinja", "nob.c"),
    ("nob_config.h.jinja", "nob_config.h"),
    ("config/nob_dep_setup.h.jinja", "config/nob_dep_setup.h"),
    ("config/nob_dep_examples.h.jinja", "config/nob_dep_examples.h"),
)


def load_catalog() -> dict:
    with CATALOG_PATH.open(encoding="utf-8") as handle:
        data = yaml.safe_load(handle)
    if not data or "deps" not in data:
        raise SystemExit(f"Invalid catalog: {CATALOG_PATH}")
    return data


def parse_deps_arg(raw: str | None, catalog: dict) -> list[str]:
    if not raw or not raw.strip():
        return []
    names = [part.strip() for part in raw.split(",") if part.strip()]
    known = set(catalog["deps"])
    unknown = [name for name in names if name not in known]
    if unknown:
        raise SystemExit(
            f"Unknown dep(s): {', '.join(unknown)} — known: {', '.join(sorted(known))}"
        )
    return names


def dep_excluded_paths(catalog: dict, optional_deps: list[str]) -> set[str]:
    excluded: set[str] = set()
    for name, meta in catalog["deps"].items():
        if name in optional_deps:
            continue
        paths = meta.get("paths", {})
        for key in ("feature_dir", "example_file", "enable_sh", "enable_ps1"):
            value = paths.get(key)
            if value:
                excluded.add(value.replace("\\", "/"))
        submodule = meta.get("submodule")
        if submodule and submodule.get("path"):
            excluded.add(submodule["path"].replace("\\", "/"))
    return excluded


def should_copy(rel_posix: str, excluded_paths: set[str]) -> bool:
    if rel_posix in MAINTAINER_ONLY or rel_posix in TEMPLATE_META:
        return False
    if rel_posix.startswith("config/enabled/") and rel_posix != "config/enabled/.gitkeep":
        return False
    if rel_posix.endswith(".jinja"):
        return False
    if rel_posix == "config/catalog.yaml":
        return True
    for excluded in excluded_paths:
        if rel_posix == excluded or rel_posix.startswith(excluded + "/"):
            return False
    return True


def copy_tree(template_root: Path, output: Path, excluded_paths: set[str]) -> None:
    for dirpath, dirnames, filenames in os.walk(template_root):
        dirnames[:] = [name for name in dirnames if name not in SKIP_DIR_NAMES]
        for filename in filenames:
            src = Path(dirpath) / filename
            rel = src.relative_to(template_root).as_posix()
            if not should_copy(rel, excluded_paths):
                continue
            dst = output / rel
            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(src, dst)


def catalog_dep_names(catalog: dict) -> list[str]:
    return list(catalog["deps"].keys())


def all_deps_csv(catalog: dict) -> str:
    return ",".join(catalog_dep_names(catalog))


def make_context(catalog: dict, optional_deps: list[str], project_name: str) -> dict:
    return {
        "optional_deps": optional_deps,
        "catalog": catalog,
        "project_name": project_name,
    }


def render_jinja_files(
    template_root: Path, output: Path, context: dict, outputs: tuple | None = None
) -> None:
    env = Environment(
        loader=FileSystemLoader(str(template_root)),
        autoescape=select_autoescape(disabled_extensions=("jinja",)),
        keep_trailing_newline=True,
        trim_blocks=False,
        lstrip_blocks=False,
    )
    for jinja_rel, out_rel in outputs or JINJA_OUTPUTS:
        template = env.get_template(jinja_rel)
        target = output / out_rel
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(template.render(**context), encoding="utf-8", newline="\n")


def activate_deps(output: Path, optional_deps: list[str], catalog: dict) -> None:
    enabled_dir = output / "config" / "enabled"
    if enabled_dir.exists():
        for child in enabled_dir.iterdir():
            if child.name == ".gitkeep":
                continue
            if child.is_dir():
                shutil.rmtree(child)
            else:
                child.unlink()

    if not optional_deps:
        enabled_dir.mkdir(parents=True, exist_ok=True)
        gitkeep = enabled_dir / ".gitkeep"
        if not gitkeep.exists():
            gitkeep.touch()
        return

    enabled_dir.mkdir(parents=True, exist_ok=True)

    for dep in optional_deps:
        feature_dir = output / catalog["deps"][dep]["paths"]["feature_dir"]
        shutil.copy2(feature_dir / f"{dep}.h", enabled_dir / f"{dep}.h")
        shutil.copy2(feature_dir / f"{dep}.build.h", enabled_dir / f"{dep}.build.h")

    for dep in optional_deps:
        submodule = catalog["deps"][dep].get("submodule")
        if not submodule:
            continue
        gitmodules = output / ".gitmodules"
        path = submodule["path"]
        block = (
            f'[submodule "{path}"]\n'
            f"\tpath = {path}\n"
            f"\turl = {submodule['url']}\n"
        )
        if gitmodules.exists():
            text = gitmodules.read_text(encoding="utf-8")
            if path not in text:
                gitmodules.write_text(text.rstrip() + "\n\n" + block, encoding="utf-8")
        else:
            gitmodules.write_text("\n" + block, encoding="utf-8")


def prune_excluded(output: Path, excluded_paths: set[str]) -> None:
    for rel in sorted(excluded_paths, key=len, reverse=True):
        target = output / rel
        if target.is_dir():
            shutil.rmtree(target, ignore_errors=True)
        elif target.is_file():
            target.unlink(missing_ok=True)


def generate(
    *,
    template_root: Path,
    output: Path,
    optional_deps: list[str],
    project_name: str,
    clean: bool,
) -> None:
    catalog = load_catalog()
    excluded_paths = dep_excluded_paths(catalog, optional_deps)

    if clean and output.exists():
        shutil.rmtree(output)
    output.mkdir(parents=True, exist_ok=True)

    if not clean:
        prune_excluded(output, excluded_paths)

    context = make_context(catalog, optional_deps, project_name)

    copy_tree(template_root, output, excluded_paths)
    render_jinja_files(template_root, output, context)
    activate_deps(output, optional_deps, catalog)


def sync_maintainer_codegen(template_root: Path) -> None:
    """Regenerate repo-root codegen with every dep from config/catalog.yaml."""
    catalog = load_catalog()
    all_deps = catalog_dep_names(catalog)
    context = make_context(catalog, all_deps, "maintainer")

    render_jinja_files(template_root, template_root, context)
    activate_deps(template_root, all_deps, catalog)


def validate_copier_choices(catalog: dict, copier_path: Path) -> None:
    text = copier_path.read_text(encoding="utf-8")
    catalog_names = set(catalog_dep_names(catalog))
    in_choices = False
    choices: set[str] = set()
    for line in text.splitlines():
        stripped = line.strip()
        if stripped == "choices:":
            in_choices = True
            continue
        if in_choices:
            if stripped.startswith("- "):
                choices.add(stripped[2:].strip())
            elif stripped and not stripped.startswith("#"):
                in_choices = False
    if choices != catalog_names:
        raise SystemExit(
            "copier.yml optional_deps.choices out of sync with config/catalog.yaml\n"
            f"  catalog: {sorted(catalog_names)}\n"
            f"  copier:  {sorted(choices)}"
        )


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--template",
        type=Path,
        default=ROOT,
        help="Template repository root (default: repo root)",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=ROOT / "output_project",
        help="Output directory (default: output_project)",
    )
    parser.add_argument(
        "--deps",
        default="",
        help="Comma-separated optional deps from config/catalog.yaml (default: none)",
    )
    parser.add_argument(
        "--project-name",
        default="",
        help="Project name for templates (default: output directory name)",
    )
    parser.add_argument(
        "--no-clean",
        action="store_true",
        help="Do not remove the output directory before generating",
    )
    parser.add_argument(
        "--sync-maintainer",
        action="store_true",
        help="Regenerate repo-root nob.c / nob_config.h / nob_dep_*.h with all catalog deps",
    )
    parser.add_argument(
        "--validate-copier",
        action="store_true",
        help="Ensure copier.yml optional_deps.choices match config/catalog.yaml",
    )
    args = parser.parse_args()

    catalog = load_catalog()

    if args.validate_copier:
        validate_copier_choices(catalog, ROOT / "copier.yml")
        print("copier.yml choices match config/catalog.yaml")
        return

    if args.sync_maintainer:
        sync_maintainer_codegen(args.template.resolve())
        print("Synced maintainer codegen files from *.jinja")
        return

    optional_deps = parse_deps_arg(args.deps, catalog)
    project_name = args.project_name or args.output.name

    generate(
        template_root=args.template.resolve(),
        output=args.output.resolve(),
        optional_deps=optional_deps,
        project_name=project_name,
        clean=not args.no_clean,
    )

    dep_summary = ", ".join(optional_deps) if optional_deps else "(none)"
    print(f"Generated {args.output.resolve()} with optional deps: {dep_summary}")


if __name__ == "__main__":
    main()
